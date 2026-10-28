// RainbowVision (mods/RainbowVision.mod): colours for colour-blind players.
//
// The whole picture goes through a colour matrix on its way to the window (render.cpp, the window's draws only):
// "daltonization" (Fidaner, Lin and Ozguven, 2005). For each kind of colour blindness, the colours a player with it
// sees are simulated (the 3x3 matrices of Machado, Oliveira and Fernandes, 2009, full severity); what is lost (the
// original minus the simulation) is moved into the channels the player does see:
//   protanopia / deuteranopia (red / green): the lost difference goes into green and blue;
//   tritanopia (blue / yellow): into red and green.
// So reds and greens that looked the same come apart (hearts, pills, enemies' tints...). Option strength (0-100)
// mixes it with the original colours. Only the picture changes: the game itself is untouched.
#include "mods.h"
#include "core.h"
#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <string>

namespace fl {
std::atomic<bool> g_colorMatOn{false}; float g_colorMat[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};   // (read by render.cpp)

namespace {
void mul(const float* a, const float* b, float* out) {   // 3x3, row-major
    for (int r = 0; r < 3; r++) for (int c = 0; c < 3; c++) { float s = 0; for (int k = 0; k < 3; k++) s += a[r * 3 + k] * b[k * 3 + c]; out[r * 3 + c] = s; }
}
bool matrixFor(const std::string& mode, float strength, float* m) {
    static const float kProt[9] = {0.152286f, 1.052583f, -0.204868f, 0.114503f, 0.786281f, 0.099216f, -0.003882f, -0.048116f, 1.051998f};
    static const float kDeut[9] = {0.367322f, 0.860646f, -0.227968f, 0.280085f, 0.672501f, 0.047413f, -0.011820f, 0.042940f, 0.968881f};
    static const float kTrit[9] = {1.255528f, -0.076749f, -0.178779f, -0.078411f, 0.930809f, 0.147602f, 0.004733f, 0.691367f, 0.303900f};
    static const float kShiftRG[9] = {0, 0, 0, 0.7f, 1, 0, 0.7f, 0, 1};   // red's loss into green and blue
    static const float kShiftB[9] = {1, 0, 0.7f, 0, 1, 0.7f, 0, 0, 0};    // blue's loss into red and green
    const float* sim; const float* shift;
    if (mode == "protanopia") { sim = kProt; shift = kShiftRG; }
    else if (mode == "deuteranopia") { sim = kDeut; shift = kShiftRG; }
    else if (mode == "tritanopia") { sim = kTrit; shift = kShiftB; }
    else return false;
    float err[9], fix[9];
    for (int i = 0; i < 9; i++) err[i] = (i % 4 == 0 ? 1.f : 0.f) - sim[i];   // I - S
    mul(shift, err, fix);                                                      // Shift (I - S)
    for (int i = 0; i < 9; i++) m[i] = (i % 4 == 0 ? 1.f : 0.f) + fix[i] * strength;   // I + k Shift (I - S)
    return true;
}
}  // namespace

void rainbowTick(const ModInfo& mod) {
    static std::string key;
    std::string mode = mod.opt("type", "deuteranopia"), str = mod.opt("strength", "100");
    if (mode + "|" + str == key) return;
    key = mode + "|" + str;
    float k = (float)std::max(0.0, std::min(100.0, atof(str.c_str()))) / 100.f, m[9];
    if (!matrixFor(mode, k, m)) { g_colorMatOn = false; logf("rainbowvision: unknown type \"%s\"\n", mode.c_str()); return; }
    memcpy(g_colorMat, m, sizeof m);
    g_colorMatOn.store(true, std::memory_order_release);
    logf("rainbowvision: %s, strength %d%%\n", mode.c_str(), (int)(k * 100 + 0.5f));
}
}  // namespace fl
