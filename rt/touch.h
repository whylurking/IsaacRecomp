// Touch controls for phones/tablets (Android). Main thread only.
// In gameplay: two joysticks that appear where a finger lands and stay there until it lifts (left half = move
// with WASD, right half = shoot with the arrows) and taps on the HUD icons (bomb = E, active item = Space,
// pill/card = Q, and our pause button next to -LIFE- = P). In menus: a finger acts as the mouse (the game's
// buttons respond to it); a tap on a screen without buttons (cutscenes, "press space" screens) is turned into
// Space by the logic thread.
#pragma once
#include <cstdint>
#include <functional>
#include <vector>
#include "render.h"
#include "swfdata.h"

namespace fl {

struct TouchCtx {
    bool gameplay = false, inGame = false;
    int winW = 0, winH = 0;          // drawable size (pixels)
    Mat stageToWin;                  // stage coordinates -> drawable pixels
    int mask = 0;                    // HUD targets on screen (bit 0 bomb, 1 active item, 2 pill/card, 3 pause button)
    float rects[4][4] = {};          // their areas, stage coordinates
};

struct TouchSink {
    std::function<void(int code, bool down)> key;              // Flash key code
    std::function<void(int type, double sx, double sy)> mouse; // 0 move, 1 down, 2 up (stage coordinates)
    std::function<void(double sx, double sy)> tap;             // short tap in menus (stage coordinates)
};

class TouchControls {
public:
    explicit TouchControls(TouchSink sink) : out_(std::move(sink)) {}
    void down(int64_t id, double px, double py, const TouchCtx& c, double now);
    void move(int64_t id, double px, double py, const TouchCtx& c, double now);
    void up(int64_t id, double px, double py, const TouchCtx& c, double now);
    void update(const TouchCtx& c, double now);   // every frame: timed releases, mode changes
    void releaseAll(double now);
    void prune(const std::vector<int64_t>& live, double now);   // every frame: fingers not in `live` (on the screen) let go
    int sticks(TouchStick out[2]) const;          // what to draw
    float moveAmount() const;                     // movement stick: how far it is pushed (0..1), -1 = not moving
    static float deadZone();
    static void setLayout(bool swapSides, int size, int type);   // movement stick on the right half; size 0..2; type 0 fixed base, 1 follows the finger
private:
    struct Stick { int64_t id = -1; double cx = 0, cy = 0, kx = 0, ky = 0, r = 0, fx = 0, fy = 0; float mag = 0; int keys[4] = {0, 0, 0, 0}; };
    struct Hold { int64_t id = -1; int code = 0; double since = 0; bool up = false; };
    TouchSink out_;
    Stick st_[2];                 // 0 = move (left), 1 = shoot (right)
    Hold hold_[4];                // HUD buttons held by a finger (or waiting for the minimum hold time)
    int64_t menuId_ = -1; double menuX0_ = 0, menuY0_ = 0, menuT0_ = 0; bool menuMoved_ = false;
    bool lastPlay_ = false;
    int keyRef_[256] = {};
    void press(int code, bool down);
    void steer(Stick& s, int which, double px, double py);
    static bool toStage(const TouchCtx& c, double px, double py, double& sx, double& sy);
};

}  // namespace fl
