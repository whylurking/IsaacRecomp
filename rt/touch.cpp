// Touch controls (see touch.h).
#include "touch.h"
#include <algorithm>
#include <cmath>

namespace fl {

static const double kMinHold = 0.09;
static bool g_swap = false, g_follow = false; static double g_sizeK = 0.12;
void TouchControls::setLayout(bool swapSides, int size, int type) { g_swap = swapSides; g_sizeK = size <= 0 ? 0.09 : size >= 2 ? 0.15 : 0.12; g_follow = type == 1; }
static const double kDead = 0.18;      // dead zone (fraction of the radius)   // s: a quick tap must still be seen by the game's per-tick key polling (30 Hz)

bool TouchControls::toStage(const TouchCtx& c, double px, double py, double& sx, double& sy) {
    Mat inv; if (!c.stageToWin.invert(inv)) return false;
    inv.apply(px, py, sx, sy);
    return true;
}

void TouchControls::press(int code, bool down) {
    if (code <= 0 || code >= 256) return;
    if (down) { if (keyRef_[code]++ == 0) out_.key(code, true); }
    else if (keyRef_[code] > 0 && --keyRef_[code] == 0) out_.key(code, false);
}

// joystick direction -> keys. Movement: 8 directions (WASD). Shooting: 4 directions (the game shoots along one
// axis; the dominant one wins, with some hysteresis so it doesn't flicker on the diagonal).
void TouchControls::steer(Stick& s, int which, double px, double py) {
    s.fx = px; s.fy = py;
    double dx = px - s.cx, dy = py - s.cy, d = std::hypot(dx, dy);
    if (g_follow && d > s.r) {   // joystick type 2: the base follows the finger (it never gets further than the edge)
        s.cx = px - dx / d * s.r; s.cy = py - dy / d * s.r;
        dx = px - s.cx; dy = py - s.cy; d = s.r;
    }
    // type 1: the base stays where the finger first landed; the knob stops at its edge
    double kd = std::min(d, s.r), k = d > 0 ? kd / d : 0;
    s.kx = s.cx + dx * k; s.ky = s.cy + dy * k;
    int want[4] = {0, 0, 0, 0};   // left, up, right, down
    s.mag = (float)(kd / s.r);
    if (d > s.r * kDead && lastPlay_) {   // (outside gameplay a held stick is only tracked, see update)
        double ux = dx / d, uy = dy / d;
        if (which == 0) {
            const double t = 0.383;   // sin 22.5 deg: 8 sectors
            want[0] = ux < -t; want[2] = ux > t; want[1] = uy < -t; want[3] = uy > t;
        } else {
            bool wasX = s.keys[0] || s.keys[2], wasY = s.keys[1] || s.keys[3];
            double bias = 1.15;
            bool horiz = wasX ? std::fabs(ux) * bias >= std::fabs(uy) : wasY ? std::fabs(ux) > std::fabs(uy) * bias : std::fabs(ux) >= std::fabs(uy);
            if (horiz) { want[0] = ux < 0; want[2] = ux > 0; } else { want[1] = uy < 0; want[3] = uy > 0; }
        }
    }
    static const int kMove[4] = {'A', 'W', 'D', 'S'}, kShoot[4] = {37, 38, 39, 40};
    const int* codes = which == 0 ? kMove : kShoot;
    for (int i = 0; i < 4; i++) if (want[i] != s.keys[i]) { s.keys[i] = want[i]; press(codes[i], want[i] != 0); }
}

void TouchControls::down(int64_t id, double px, double py, const TouchCtx& c, double now) {
    double sx, sy; if (!toStage(c, px, py, sx, sy)) return;
    if (c.gameplay) {
        // HUD icons first. Tested in window pixels: on a wide phone the bomb row is only ~2 mm tall, so every
        // target is grown to a minimum finger-sized square (13% of the screen height) around its center; where
        // grown targets overlap, the nearest center wins.
        static const int kCodes[4] = {'E', 32, 'Q', 'P'};
        const double minSz = std::min(c.winW, c.winH) * 0.13;
        int best = -1; double bestD = 1e30;
        for (int i = 0; i < 4; i++) {
            if (!(c.mask & (1 << i))) continue;
            const float* r = c.rects[i];
            double x0, y0, x1, y1;
            c.stageToWin.apply(r[0], r[1], x0, y0); c.stageToWin.apply(r[2], r[3], x1, y1);
            if (x0 > x1) std::swap(x0, x1);
            if (y0 > y1) std::swap(y0, y1);
            double mx = i == 3 ? (x1 - x0) * 0.4 : 0;   // (the pause button is small: a more forgiving touch area)
            x0 -= mx; x1 += mx; y0 -= mx; y1 += mx;
            double cx = (x0 + x1) / 2, cy = (y0 + y1) / 2;
            double hw = std::max((x1 - x0) / 2, minSz / 2), hh = std::max((y1 - y0) / 2, minSz / 2);
            if (std::fabs(px - cx) > hw || std::fabs(py - cy) > hh) continue;
            double d = std::hypot((px - cx) / hw, (py - cy) / hh);
            if (d < bestD) { bestD = d; best = i; }
        }
        if (best >= 0) {
            for (auto& h : hold_) if (h.id == -1 && !h.code) { h.id = id; h.code = kCodes[best]; h.since = now; h.up = false; press(h.code, true); return; }
            return;
        }
        int which = (px < c.winW * 0.5) != g_swap ? 0 : 1;   // 0 = movement stick
        Stick& s = st_[which];
        if (s.id != -1) return;   // that side already has a finger
        s.id = id; s.cx = s.kx = px; s.cy = s.ky = py;
        s.r = std::max(40.0, std::min(330.0, std::min(c.winW, c.winH) * g_sizeK));   // (normal = the first Android build's size)
        steer(s, which, px, py);
        return;
    }
    if (menuId_ != -1) return;   // menus: one finger = the mouse
    menuId_ = id; menuX0_ = px; menuY0_ = py; menuT0_ = now; menuMoved_ = false;
    out_.mouse(0, sx, sy); out_.mouse(1, sx, sy);
}

void TouchControls::move(int64_t id, double px, double py, const TouchCtx& c, double now) {
    (void)now;
    for (int w = 0; w < 2; w++) if (st_[w].id == id) { steer(st_[w], w, px, py); return; }
    if (id == menuId_) {
        double sx, sy; if (!toStage(c, px, py, sx, sy)) return;
        if (std::hypot(px - menuX0_, py - menuY0_) > std::min(c.winW, c.winH) * 0.03) menuMoved_ = true;
        out_.mouse(0, sx, sy);
    }
}

void TouchControls::up(int64_t id, double px, double py, const TouchCtx& c, double now) {
    for (int w = 0; w < 2; w++) if (st_[w].id == id) {
        Stick& s = st_[w];
        static const int kMove[4] = {'A', 'W', 'D', 'S'}, kShoot[4] = {37, 38, 39, 40};
        for (int i = 0; i < 4; i++) if (s.keys[i]) { s.keys[i] = 0; press(w == 0 ? kMove[i] : kShoot[i], false); }
        s.id = -1; return;
    }
    for (auto& h : hold_) if (h.id == id && h.code) { h.up = true; h.id = -2; update(c, now); return; }
    if (id == menuId_) {
        menuId_ = -1;
        double sx, sy; if (!toStage(c, px, py, sx, sy)) return;
        out_.mouse(0, sx, sy); out_.mouse(2, sx, sy);
        if (!menuMoved_ && now - menuT0_ < 0.6) out_.tap(sx, sy);
    }
}

void TouchControls::releaseAll(double now) {
    (void)now;
    static const int kMove[4] = {'A', 'W', 'D', 'S'}, kShoot[4] = {37, 38, 39, 40};
    for (int w = 0; w < 2; w++) {
        for (int i = 0; i < 4; i++) if (st_[w].keys[i]) { st_[w].keys[i] = 0; press(w == 0 ? kMove[i] : kShoot[i], false); }
        st_[w].id = -1;
    }
    for (auto& h : hold_) if (h.code) { press(h.code, false); h = Hold(); }
    menuId_ = -1;
}

void TouchControls::prune(const std::vector<int64_t>& live, double now) {
    (void)now;
    auto on = [&](int64_t id) { for (int64_t l : live) if (l == id) return true; return false; };
    static const int kMove[4] = {'A', 'W', 'D', 'S'}, kShoot[4] = {37, 38, 39, 40};
    for (int w = 0; w < 2; w++) {
        Stick& s = st_[w];
        if (s.id == -1 || on(s.id)) continue;
        for (int i = 0; i < 4; i++) if (s.keys[i]) { s.keys[i] = 0; press(w == 0 ? kMove[i] : kShoot[i], false); }
        s.id = -1;
    }
    for (auto& h : hold_) if (h.code && h.id >= 0 && !on(h.id)) { h.up = true; h.id = -2; }
}

void TouchControls::update(const TouchCtx& c, double now) {
    if (c.gameplay != lastPlay_) {
        // pause, boss "VS" screen, death...: the HUD buttons and the menu finger are let go; a finger still on a
        // joystick keeps it - its keys are released and pressed again when play resumes (before, the joystick
        // vanished and Isaac stood still after the boss intro until the finger was lifted and put down again)
        lastPlay_ = c.gameplay;
        for (int w = 0; w < 2; w++) if (st_[w].id != -1) steer(st_[w], w, st_[w].fx, st_[w].fy);
        if (c.gameplay) {   // (play goes on: every stick key no finger holds is sent up again, whatever the game kept)
            static const int kMv[4] = {'A', 'W', 'D', 'S'}, kSh[4] = {37, 38, 39, 40};
            for (int w = 0; w < 2; w++) for (int i = 0; i < 4; i++) {
                int code = w == 0 ? kMv[i] : kSh[i];
                if (!(st_[w].id != -1 && st_[w].keys[i])) { keyRef_[code] = 0; out_.key(code, false); }
            }
        }
        for (auto& h : hold_) if (h.code) { press(h.code, false); h = Hold(); }
        menuId_ = -1;
    }
    for (auto& h : hold_) if (h.code && h.up && now - h.since >= kMinHold) { press(h.code, false); h = Hold(); }
    // (a safety net: every movement / shooting key the touch controls hold must belong to a finger on a stick; one
    // that doesn't - a release lost somewhere - is let go, so Isaac never keeps walking or shooting on his own)
    static const int kMove[4] = {'A', 'W', 'D', 'S'}, kShoot[4] = {37, 38, 39, 40};
    for (int w = 0; w < 2; w++)
        for (int i = 0; i < 4; i++) {
            int code = w == 0 ? kMove[i] : kShoot[i];
            bool want = st_[w].id != -1 && st_[w].keys[i];
            bool held = false; for (auto& h : hold_) held |= h.code == code;
            if (!want && !held && keyRef_[code] > 0) { keyRef_[code] = 0; out_.key(code, false); }
        }
}

float TouchControls::moveAmount() const {
    const Stick& s = st_[0];
    return s.id == -1 || s.mag <= kDead ? -1.f : s.mag;
}
float TouchControls::deadZone() { return (float)kDead; }

int TouchControls::sticks(TouchStick out[2]) const {
    int n = 0;
    for (int w = 0; w < 2; w++) {
        out[w] = TouchStick();
        if (st_[w].id == -1) continue;
        out[w].active = true; out[w].cx = st_[w].cx; out[w].cy = st_[w].cy; out[w].r = st_[w].r; out[w].kx = st_[w].kx; out[w].ky = st_[w].ky;
        n++;
    }
    return n;
}

}  // namespace fl
