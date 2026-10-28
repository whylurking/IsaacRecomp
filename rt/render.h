#pragma once
#include <array>
#include "display.h"
#include "snapshot.h"
#include <string>
#include <vector>
#include <functional>
#include <cstdint>
namespace fl {
struct RenderConfig {
    bool smooth = true; bool filters = true; int cacheMB = 768; int rasterPerFrame = 64;
    // maximum time (ms) a frame waits for new graphics being rasterized in the background; anything
    // not ready in time is drawn at another already-rasterized scale or appears one frame later
    double rasterWaitMs = 1e9;
    double rasterWaitNoFallbackMs = 1e9;   // same, when there is no other scale to show in the meantime
    // blur filters whose contents changed: how many may be recomputed per frame (0 = all); the rest show their
    // previous result, refreshed at least every filterMaxAge frames. Mobile GPUs pay a lot for each render-target
    // switch (the Eternal title screen has 17 blurs = ~50 switches per frame).
    int filterBudget = 0, filterMaxAge = 3;
    bool filterLowRes = false;   // blur at lower resolution (mobile)
};   // cacheMB: main.cpp sets the platform's value
extern RenderConfig g_renderCfg;
struct RenderStats {
    int rasters = 0; double rasterMs = 0; double uploadMB = 0, uploadMs = 0; int quads = 0; int draws = 0; int filters = 0; int texCreates = 0;
    // render thread time breakdown (ms): adopting snapshots, compiling commands, rasterizing what's missing,
    // generating quads + GL calls, and the frame's node/command counts
    double adoptMs = 0, compileMs = 0, prerasterMs = 0, execMs = 0; int nodes = 0, cmds = 0, asyncPending = 0, filterHits = 0;
    int rasterDeferred = 0;   // (PSP) graphics left for a later frame (rasterization budget)
};
extern RenderStats g_rstats;
extern Mat g_stageToWindow;
bool renderInit();
// adopts a new snapshot (applies its pixel uploads); the previous one becomes the interpolation source
void renderAdopt(const SnapPtr& s);
// OpenGL ES 3.0 instead of desktop OpenGL 3.3 (set before renderInit; Android, or ISAAC_GLES=1 for testing)
extern bool g_glES;
// with a logic thread (g_retireSnapshots = true) replaced snapshots are freed by it, calling renderFreeRetired()
extern bool g_retireSnapshots;
void renderFreeRetired();
const Snapshot* renderCurrent();
// draws lerp(previous, current, alpha)
void renderFrame(int winW, int winH, double alpha);
Mat stageToWindowFor(int winW, int winH);
bool renderScreenshot(const char* path);
void renderOverlay(const std::vector<std::string>& lines);
void renderHint(const std::string& text);   // small text in the bottom-right corner
// touch joysticks drawn over the frame (window pixels): base centre/radius and knob position
struct TouchStick { bool active = false; double cx = 0, cy = 0, r = 0, kx = 0, ky = 0; };
void renderTouchSticks(const TouchStick* sticks, int n);
void renderPauseButton(double x0, double y0, double x1, double y1);   // window pixels
// game selection screen when several are available (rects: area of each option, for mouse and touch)
void renderChooser(int winW, int winH, const std::string& title, const std::vector<std::string>& items, int sel,
                   const std::string& hint, std::vector<std::array<int, 4>>* rects);
void renderEndFrame();   // after the frame's last draw (before presenting it)
// ---- graphics preloading
// A rasterized graphic: shape (SWF id), kind, variant (morph frame) and scale in quarter-octaves
// (q100 = ×100); relative to the screen height (720 px), except abs (drawings into a bitmap's texture).
struct PreKey { uint16_t id = 0; uint8_t kind = 0; bool abs = false; uint16_t variant = 0; int16_t q100 = 0; };
// queues the list for background rasterization (while the game starts)
void renderPreload(const std::vector<PreKey>& keys, int winW, int winH, const std::function<void(float)>& progress);
void renderLoadingScreen(int winW, int winH, float frac);   // draws the loading screen (without presenting it)
size_t renderPreloadPending();   // preload graphics still pending (and stores a bit of the finished work in the atlas)
std::vector<PreKey> renderTakeLearned();                     // new graphics since the last call
void renderMarkKnown(const std::vector<PreKey>& keys);       // (already in the list: don't record them again)   // diagnostic text (F3) in the top-left corner
}
