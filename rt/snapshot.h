// Snapshot of the display list at the end of each logic tick.
// The logic (AVM1) runs on its own thread at exactly 30 Hz and publishes one snapshot per tick; the
// renderer (main thread, at the monitor's refresh rate) interpolates between the last two.
// The snapshot is self-contained: the renderer never touches live game objects.
#pragma once
#include "display.h"
#include "raster.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <atomic>

namespace fl {

enum SnapKind : uint8_t { SN_CONTAINER, SN_CONTENT, SN_BITMAP };

struct SnapNode {
    uint32_t uid = 0;            // stable object identity (to match against the previous tick)
    uint32_t end = 0;            // index just past the subtree (children go from i+1 to end, in order)
    int32_t parent = -1;
    SnapKind kind = SN_CONTAINER;
    uint8_t objKind = 0;         // OK_CLIP, OK_SHAPE...
    bool visible = true;
    int clipDepth = 0, depth = 0;
    Mat m;                        // local matrix
    CX cx;
    // content (SN_CONTENT)
    const CharDef* def = nullptr; uint16_t ratio = 0; uint32_t textVersion = 0; int32_t text = -1;   // index into Snapshot::texts
    uint32_t argb = 0;
    RectF bounds; bool hasBounds = false;   // local bounds of the content
    // bitmap (SN_BITMAP)
    uint32_t bmUid = 0; int bmW = 0, bmH = 0; bool smoothing = false;
    const uint32_t* bmPx = nullptr;   // (PSP) the BitmapData's pixels, drawn in place (valid until the next tick)
    // filters (blur only)
    bool blur = false; float blurX = 0, blurY = 0; int blurPasses = 1;
};

struct BitmapUpload {
    uint32_t uid = 0; int w = 0, h = 0;          // full size of the BitmapData
    int x0 = 0, y0 = 0, x1 = 0, y1 = 0;          // copied rectangle
    std::vector<uint32_t> px;                    // (x1-x0)*(y1-y0) premultiplied pixels
};

struct Snapshot;
// drawing operation on a BitmapData that lives on the GPU (executed in order when the snapshot is adopted)
struct GpuOp {
    enum Kind : uint8_t { UPLOAD, FILL, COPY, DRAW } kind = FILL;
    uint32_t uid = 0; int w = 0, h = 0;      // destination BitmapData
    int x0 = 0, y0 = 0, x1 = 0, y1 = 0;      // FILL/COPY: destination rectangle (bitmap px)
    uint32_t color = 0;                      // FILL: premultiplied ARGB
    bool merge = false;                      // COPY: alpha-blend (mergeAlpha) or replace
    std::vector<uint32_t> px;                // UPLOAD: the whole bitmap; COPY: the source pixels
    Mat m; CX cx;                            // DRAW: matrix and color transform
    std::shared_ptr<Snapshot> scene;         // DRAW: what gets drawn (subtree; nodes[0] = the source)
};

struct Snapshot {
    uint32_t tick = 0;
    uint32_t serial = 0;          // unique snapshot number (GpuOp scenes have their own too)
    bool main = true;             // screen snapshot (not a draw scene)
    std::vector<GpuOp> gpuOps;    // draws onto GPU layers made during the tick (before the uploads)
    double time = 0;              // nominal time of the tick (s, logic clock)
    std::vector<SnapNode> nodes;  // preorder; nodes[0] = _root (if any)
    std::vector<std::string> texts;
    std::vector<BitmapUpload> uploads;
    std::vector<uint32_t> freedBitmaps;
    uint32_t bgColor = 0xffffff;
    bool mouseHidden = false, quit = false;
    bool handCursor = false;   // the mouse is over something clickable (menuui.cpp)
    bool inGame = false;          // a game is in progress (even if paused)
    bool gameplay = false;        // actively playing (not in menus or paused): decides the button mapping on Switch
    double logicMs = 0;           // how long this tick took (diagnostics)
    double pubDelay = 0;          // publication delay relative to the nominal time (s)
    int entities = 0;
    float touchRects[4][4] = {};  // touch screens: bomb / active item / pill-card / pause HUD areas (stage coordinates)
    int touchMask = 0;            // which of them are on screen
    bool focusOn = false;         // controller in a menu: outline around the selected option (stage coordinates)
    float focusRect[4] = {};
    uint32_t focusUid = 0;        // its display object: its ink drawn pink (render.cpp)
    bool focusStrong = false;     // ... deep red instead (the main menu's Start)
    int rebindRequests = 0;       // grows when the player asks for the rebinding screen (menuui.cpp)
    bool frog = false, lefty = false;   // the game's AZERTY / lefty control options
    // the part of the stage to show (WideSaac): fitted to the window, centered; viewClip = false also shows what lies
    // around it (the window's whole area). Unset: the whole stage, letterboxed (the default)
    bool viewSet = false, viewClip = true; float view[4] = {};
    bool viewCut = false; float cut[4] = {};   // (WideSaac's boss intros) only this part of the stage is drawn, black around it
    // computed by the renderer on adoption: uid → node index (open table, power of 2) and, per node, the
    // displacement relative to the previous tick (to detect jumps)
    std::vector<uint64_t> uidTable; uint32_t uidMask = 0;
    std::vector<float> jump;
    std::vector<uint8_t> fresh;   // node did not exist in the tick before (renderer)
    bool indexed = false;
    int findUid(uint32_t uid) const {
        if (uidTable.empty()) return -1;
        for (uint32_t h = (uid * 2654435761u) & uidMask;; h = (h + 1) & uidMask) {
            uint64_t e = uidTable[h];
            if (!e) return -1;
            if ((uint32_t)e == uid) return (int)(e >> 32);
        }
    }
};
using SnapPtr = std::shared_ptr<Snapshot>;

// logic thread
SnapPtr buildSnapshot();
// set by the logic thread (mods) and copied into each snapshot
struct ViewReq { bool set = false, clip = true, cut = false; float r[4] = {}, c[4] = {}; };   // cut: only c (stage px) is drawn, black around it
extern ViewReq g_viewReq;
// per display object, for the snapshot only (the game never sees it; WideSaac's Hyper Zoom): hidden, and / or drawn
// with another local matrix. Rebuilt by the logic thread every tick before buildSnapshot.
struct SnapOv { bool hide = false, hasM = false, hasCx = false; Mat m; float alpha = -1; CX cx; };   // alpha >= 0: multiplies the node's alpha; hasCx: cx applied over the node's own color transform
extern std::unordered_map<DisplayObject*, SnapOv> g_snapOv;
extern std::atomic<float> g_windowAspect;   // the window's width / height (main thread), 0 if unknown
void bitmapGone(BitmapData* b);   // a BitmapData was destroyed or released (dispose)
// GPU layers (see BitmapData::gpu). gpuBitmapsEnabled: only with a renderer (in renderer-less tests, everything is on the CPU)
extern bool g_gpuBitmaps;
bool bdUseGpu(BitmapData* b);     // drawn on the GPU? (moves it to the GPU the first time if possible)
void bdNeedCpu(BitmapData* b);    // the game is about to read or modify its pixels on the CPU
void gpuFill(BitmapData* b, int x0, int y0, int x1, int y1, uint32_t premulColor);
void gpuCopy(BitmapData* d, BitmapData* s, int sx, int sy, int w, int h, int dx, int dy, bool merge);
void gpuDraw(BitmapData* b, Object* src, const Mat& m, const CX* cx);

}  // namespace fl
