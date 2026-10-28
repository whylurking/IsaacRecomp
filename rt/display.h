// Display list and timeline engine (AVM1 / Flash Player 8 semantics).
#pragma once
#include "core.h"
#include "swfdata.h"
#include <deque>
#include <array>
#include <atomic>

namespace fl {

constexpr int DEPTH_BIAS = 16384;

struct BitmapData;
inline uint32_t nextUid() { static std::atomic<uint32_t> c{0}; return ++c; }   // stable identity for the renderer

struct DisplayObject : Object {
    uint32_t uid = nextUid();
    MovieClip* parent = nullptr;
    int depth = 0;
    uint16_t charId = 0;
    Str* name = nullptr;
    Mat m;                                  // local matrix (px)
    // _xscale/_yscale (percent), _rotation (degrees) and skew (radians) as Ruffle keeps them: the values last set by
    // script are returned as they were set; after a matrix change they are derived from it on demand
    double sxP = 100, syP = 100, rotD = 0, skew = 0;
    bool srCached = true;
    CX cx;
    bool visible = true, scriptXform = false, scriptColor = false, removed = false, placedByScript = false, timelinePlaced = false;
    int clipDepth = 0;
    uint16_t ratio = 0;
    int placeFrame = 0;
    std::vector<Filter> filters;
    uint8_t blend = 0;
    Str* path = nullptr;         // path cached on removal (dead references)

    void setMatrix(const Mat& mm);
    void cacheSR();
    void setRotation(double deg);
    void setScaleX(double pct);
    void setScaleY(double pct);
    void trace(Tracer& tr) override;
};

struct ShapeObj final : DisplayObject { CharDef* def = nullptr; };   // shape, morph or static text

struct BitmapObj final : DisplayObject {
    BitmapData* bd = nullptr; bool smoothing = false;
    void trace(Tracer& tr) override;
};

struct EditTextObj final : DisplayObject {
    EditTextDef* def = nullptr;
    std::string text; uint32_t argb = 0xff000000;
    uint32_t version = 1;      // unique content identifier (invalidates the render cache)
    // version: identifies the content for the render cache. It's a hash of the text and colour (the rest is in the
    // definition), so a field showing a text it showed before (a counter, a timer's digit) reuses that raster
    // instead of rasterizing and uploading it again.
    void setText(const std::string& t) {
        uint32_t h = 2166136261u;
        for (unsigned char c : t) { h ^= c; h *= 16777619u; }
        for (int i = 0; i < 4; i++) { h ^= (argb >> (i * 8)) & 0xff; h *= 16777619u; }
        if (h <= 1) h += 2;
        if (t == text && h == version) return;
        text = t; version = h;
    }
};

struct ButtonObj final : DisplayObject {
    ButtonDef* def = nullptr;
    int state = 0;             // 0 up, 1 over, 2 down
    std::vector<DisplayObject*> kids;   // children of the current state
    std::vector<DisplayObject*> hitKids;
    bool enabled = true;
    void trace(Tracer& tr) override;
};

struct MovieClip final : DisplayObject {
    SpriteDef* def = nullptr;
    int cur = 0;                 // current frame (1-based; 0 = uninitialized)
    bool playing = true, initialized = false, inExec = false;
    std::vector<DisplayObject*> kids;          // sorted by depth
    std::unordered_map<Str*, DisplayObject*> byName;
    Scope* tscope = nullptr;
    MovieClip* execNext = nullptr;
    int streamChan = -1;          // audio channel of the timeline stream
    int lastStreamFrame = -1;
    void trace(Tracer& tr) override;

    DisplayObject* childAtDepth(int d) const;
    void insertChild(DisplayObject* c);
    void eraseChild(DisplayObject* c);
    void renameChild(DisplayObject* c, Str* newName);
    void reindexName(Str* n);
    DisplayObject* childByName(Str* n) const { auto it = byName.find(n); return it == byName.end() ? nullptr : it->second; }
};

// ---- BitmapData (CPU, premultiplied ARGB)
struct BitmapData final : Object {
    int w = 0, h = 0; bool transparent = true; bool disposed = false;
    std::vector<uint32_t> px;
    uint32_t version = 1;                    // changes when pixels are modified
    int dx0 = 0, dy0 = 0, dx1 = 0, dy1 = 0;  // dirty rectangle pending upload to the renderer
    uint32_t uid = nextUid();
    int sentW = 0, sentH = 0;                // size of the texture held by the renderer (0 = never uploaded)
    uint32_t snapTick = 0;                   // last snapshot that included it
    uint32_t snapSerial = 0;                 // (avoids including it twice in the same snapshot)
    // Display-only layers (attached with attachBitmap and never read): whatever is drawn into them is done by
    // the GPU (snapshot.cpp, render.cpp) and the logic rasterizes nothing. If the game ever reads their pixels,
    // it falls back to the CPU (bdNeedCpu).
    bool displayed = false, gpu = false, cpuRead = false;
    // Lazy fill: a freshly created BitmapData is a single color and does not allocate its pixels until the CPU
    // needs them (ensurePx). Each room's layers (6.7 MB) that are drawn on the GPU never allocate them.
    bool lazy = false; uint32_t lazyFill = 0;
    void ensurePx();
    BitmapData() { kind = OK_BITMAPDATA; }
    void dirty(int x0, int y0, int x1, int y1) {
        if (x0 < 0) x0 = 0; if (y0 < 0) y0 = 0; if (x1 > w) x1 = w; if (y1 > h) y1 = h;
        if (x1 <= x0 || y1 <= y0) return;
        if (dx1 <= dx0) { dx0 = x0; dy0 = y0; dx1 = x1; dy1 = y1; }
        else { if (x0 < dx0) dx0 = x0; if (y0 < dy0) dy0 = y0; if (x1 > dx1) dx1 = x1; if (y1 > dy1) dy1 = y1; }
        if (fillKnown) for (int y = y0; y < y1; y++) {
            uint32_t& sp = rowSpan[y]; int a = sp & 0xffff, b = sp >> 16;
            if (b <= a) { a = x0; b = x1; } else { if (x0 < a) a = x0; if (x1 > b) b = x1; }
            sp = (uint32_t)a | (uint32_t)b << 16;
        }
        version++;
    }
    // (PSP) after a fillRect of the whole bitmap: its colour, and per row the span changed since (fillRect then only
    // has to clear those: every tick the game clears a 300x300 bitmap used as a table of colliding pairs, and sets a
    // few hundred pixels in it)
    bool fillKnown = false; uint32_t fillVal = 0; std::vector<uint32_t> rowSpan;
    ~BitmapData() override;
};

void bdPoolTake(std::vector<uint32_t>& v, size_t n);   // reuses a freed pixel buffer
void bdPoolGive(std::vector<uint32_t>& v);
inline void BitmapData::ensurePx() {
    if (!lazy) return;
    lazy = false; size_t n = (size_t)w * h;
    bdPoolTake(px, n); px.assign(n, lazyFill);
}

// ---- Player
struct Player {
    uint32_t tick = 0;
    double timeMs = 0;            // getTimer()
    MovieClip* execHead = nullptr;
    struct Queued { DisplayObject* clip; FrameScript fn; Str* method; };
    std::deque<Queued> queue;
    // input
    bool keys[256] = {};
    int lastKey = 0, lastAscii = 0;
    double mouseX = 0, mouseY = 0;   // in stage coordinates (px)
    bool mouseDown = false;
    std::vector<Object*> keyListeners, mouseListeners;
    ButtonObj* hoverButton = nullptr; ButtonObj* pressButton = nullptr;
    int quality = 2;                  // _quality
    bool quit = false;
    std::vector<EditTextObj*> boundTexts;
};
extern Player g_player;

void playerInit();
void runFrame();                       // one logic tick (1/30 s)
void runActions();
void queueScript(DisplayObject* clip, FrameScript fn);
void queueMethod(DisplayObject* clip, Str* method);
void beginTimeline(Act& a, DisplayObject* clip);

DisplayObject* instantiate(MovieClip* parent, uint16_t id, int depth, const PlaceCmd* pc, bool byTimeline);
void applyPlace(DisplayObject* d, const PlaceCmd& pc);
MovieClip* newClip(SpriteDef* def);
void addToExec(MovieClip* c);
void runFrameClip(MovieClip* c);       // onEnterFrame + advance
void gotoFrame(MovieClip* c, int frame, bool stop);   // frame 1-based
void removeObject(DisplayObject* d);   // remove from parent + unload
void markRemoved(DisplayObject* d);
bool removeMovieClip(DisplayObject* d);
bool swapDepths(DisplayObject* d, const Value& target);
MovieClip* attachMovie(MovieClip* parent, const std::string& linkName, Str* newName, int depth, Object* init);
MovieClip* createEmptyMovieClip(MovieClip* parent, Str* name, int depth);
int frameFromValue(MovieClip* c, const Value& v, MovieClip** target);
bool hitTestPoint(DisplayObject* d, double gx, double gy, bool shapeFlag);
Mat worldMatrix(DisplayObject* d);
RectF localBounds(DisplayObject* d);   // in the object's parent space (includes its matrix)
RectF stageBounds(DisplayObject* d);   // in stage coordinates
RectF hitStageBounds(ButtonObj* b);   // its hit area, in stage coordinates
bool touchButtonAt(double x, double y);   // (touch screens) a button under this stage point
bool touchAnyButton();                    // (touch screens) any enabled, visible button on screen
void keyEvent(int flashCode, int ascii, bool down);
void mouseEvent(double x, double y, int type);   // 0 move, 1 down, 2 up
void updateBoundTexts();
Str* targetPath(DisplayObject* d);
void fscommand(const std::string& cmd, const std::string& arg);

// Sound (audio.cpp)
int soundStart(SoundDef* s, double offsetSec, int loops, Object* owner, float vol, float pan,
               const std::vector<std::array<uint32_t, 3>>* envelope = nullptr);   // envelope: SoundInfo points (44.1 kHz pos, l, r)
void soundStopOwner(Object* owner);
void soundStopAll();
void soundStopChan(int ch);
void soundSetVolumeOwner(Object* owner, float vol, float pan);
double soundPositionMs(Object* owner, bool* active = nullptr);   // active: a channel of this owner is still playing
bool soundPlayingDef(SoundDef* s);
int streamStart(StreamInfo* st, int frame0, Object* owner);
void streamSync(int ch, StreamInfo* st, int frame0);
bool chanAlive(int ch);
void audioInit(bool device = true);   // false: test mode (mixed per tick)
void audioDevicesChanged(double now);   // main thread: SDL_AUDIODEVICEADDED / REMOVED
void audioDeviceTick(double now);
void audioLock(); void audioUnlock();

}  // namespace fl
