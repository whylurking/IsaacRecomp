// Entry point: SDL2 + OpenGL, 30 Hz loop with rendering at the display refresh rate.
#include "display.h"
#include "render.h"
#include "platform.h"
#ifdef __SWITCH__
#include <switch.h>
#include <sys/stat.h>
#endif
#include "snapshot.h"
#include "touch.h"
#include "controls.h"
#include "menuui.h"
#include "mods.h"
#ifdef __ANDROID__
#include <jni.h>
#include <sys/stat.h>
#endif
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <SDL.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cerrno>
#include <algorithm>
#include <array>
#include <dirent.h>
#include <sstream>
#include <fstream>
#include <thread>
#include <exception>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <deque>
#include <functional>
#include <cmath>

namespace fl {
extern bool g_rngSeedFixed;
extern bool g_autoAim;
int autoAimKey();
extern std::atomic<bool> g_restartRequest;
void initGlobals();
void runIntervals();
void sharedObjectFlushAll();
void debugEval(const std::string&);
extern bool g_gcLog;
extern double g_gcMs;
void rasterMemStats();
int rasterThreads();
void audioDumpTick(const char*);
void audioDumpClose();
extern std::string g_savePath;
extern bool g_mouseHidden;
int recompTouchTargets(float out[4][4]);
void applyLoadPatches(); void recompPatchesTick(); void recompFullscreenChanged(bool full); bool recompInGameplay(bool* inGame);
extern std::atomic<int> g_fullscreenRequest; extern std::atomic<bool> g_isFullscreen;
void renderFinish();
void recompMusicTick();
void renderPaceAfterSwap();
bool renderPaceWait(uint64_t timeoutNs);
int renderAtlasPages();
extern float g_audioVolume;
extern std::atomic<float> g_moveScale; extern bool g_joyAccel;
}
// joystick_acceleration: how far the stick is pushed (0..1) -> movement factor. Below the dead zone nothing moves;
// just past it the player moves at 40% of its speed, rising linearly to 100% at 75% of the stick's travel.
static float moveCurve(float m, float dead) {
    if (m <= dead) return 0.f;
    float t = (m - dead) / (0.75f - dead);
    return 0.4f + 0.6f * std::max(0.f, std::min(1.f, t));
}
using namespace fl;

static int sdlToFlash(SDL_Keycode k) {
    if (k >= SDLK_a && k <= SDLK_z) return 'A' + (k - SDLK_a);
    if (k >= SDLK_0 && k <= SDLK_9) return '0' + (k - SDLK_0);
    if (k >= SDLK_F1 && k <= SDLK_F12) return 112 + (k - SDLK_F1);
    if (k >= SDLK_KP_1 && k <= SDLK_KP_9) return 97 + (k - SDLK_KP_1);
    switch (k) {
        case SDLK_KP_0: return 96; case SDLK_SPACE: return 32; case SDLK_RETURN: case SDLK_KP_ENTER: return 13; case SDLK_ESCAPE: return 27;
        case SDLK_BACKSPACE: return 8; case SDLK_TAB: return 9; case SDLK_LSHIFT: case SDLK_RSHIFT: return 16;
        case SDLK_LCTRL: case SDLK_RCTRL: return 17; case SDLK_LALT: case SDLK_RALT: return 18;
        case SDLK_LEFT: return 37; case SDLK_UP: return 38; case SDLK_RIGHT: return 39; case SDLK_DOWN: return 40;
        case SDLK_PAGEUP: return 33; case SDLK_PAGEDOWN: return 34; case SDLK_END: return 35; case SDLK_HOME: return 36;
        case SDLK_INSERT: return 45; case SDLK_DELETE: return 46; case SDLK_CAPSLOCK: return 20;
        case SDLK_SEMICOLON: return 186; case SDLK_EQUALS: return 187; case SDLK_COMMA: return 188; case SDLK_MINUS: return 189;
        case SDLK_PERIOD: return 190; case SDLK_SLASH: return 191; case SDLK_BACKQUOTE: return 192; case SDLK_LEFTBRACKET: return 219;
        case SDLK_BACKSLASH: return 220; case SDLK_RIGHTBRACKET: return 221; case SDLK_QUOTE: return 222;
        default: return 0;
    }
}
static int asciiOf(SDL_Keycode k, bool shift) {
    if (k >= SDLK_a && k <= SDLK_z) return shift ? 'A' + (k - SDLK_a) : (int)k;
    if (k >= 32 && k < 127) return (int)k;
    if (k == SDLK_RETURN) return 13; if (k == SDLK_ESCAPE) return 27; if (k == SDLK_BACKSPACE) return 8; if (k == SDLK_TAB) return 9;
    return 0;
}

// ---- configuration
struct Config {
    std::string swf, game; bool fullscreen = true; bool vsync = true; int width = 1440, height = 1080; int volume = 100;
    bool smooth = true; bool filters = true; int cacheMB = fl::g_lim.defaultCacheMB;
    bool gpuSync = !fl::kIsSwitch && !fl::kIsAndroid;   // glFinish after presenting: on Switch/Android it prevents CPU/GPU overlap
    bool lowLatency = true;          // (without gpu_sync) never leave more than one frame queued on the GPU
    double interpLead = 0;           // (experimental) draw this fraction of a tick ahead: less visual delay, slight overshoot
#if defined(__ANDROID__) || defined(__SWITCH__)
    bool touchControls = true;       // on-screen joysticks + taps on the HUD (see touch.h; Switch: handheld touch screen)
#else
    bool touchControls = false;      // (PC: for testing, with a touch screen or mouse-generated touches)
#endif
    bool performance = false;        // performance diagnostics on from the start (overlay + performance.txt)
    bool joystickAcceleration = true;   // analog sticks / touch: movement speed follows how far the stick is pushed
    int renderHeight = 0;            // (Android) render resolution (height in pixels); 0 = automatic
    int autoHeight = 0;              // (Android) the resolution automatic mode settled on for this device (written by the game)
    bool gcAfterTick = true;         // collect garbage after the tick has been handed to the renderer (not inside it)
    int maxMarginMs = fl::kIsSwitch ? 6 : 0;   // cap of the adaptive display delay (ms); 0 = automatic (4..30 ms)
    std::string mods;                // mods turned on (ids of files in the mods folder), comma-separated
};
// settings added after the first release: written into an existing isaac.ini (with their explanation) when missing,
// so they can be found and changed
static const char* kLateSettings[][3] = {
    {"gc_after_tick", "; gc_after_tick = 1 frees memory after each game tick has been sent to the screen, so it doesn't delay the\n;   image; 0 = inside the tick, as in 1.3", nullptr},
    {"max_margin", "; max_margin = how far behind the game the image may be shown, in ms (it follows how long the game's ticks take,\n;   to keep motion smooth); a cap keeps the delay constant (late ticks are extrapolated); 0 = automatic (4-30 ms)", nullptr},
    {"mods", "; mods = the mods that are on: ids of the files in the mods folder, separated by commas (empty: none). The easy\n;   way to change it: open tools/mod_manager.html in a browser", ""},
};
static std::string trim(std::string s) { while (!s.empty() && isspace((unsigned char)s.back())) s.pop_back(); size_t i = 0; while (i < s.size() && isspace((unsigned char)s[i])) i++; return s.substr(i); }
static void loadConfig(const std::string& path, Config& c) {
    std::ifstream f(path); std::string line;
    if (!f) {
        std::ofstream o(path);
        o << "; The Binding of Isaac configuration (native recomp)\n"
             "; swf = path to the Steam Isaac.exe (or to the .swf); if empty, it is searched for next to the executable\n"
             "swf =\n; game = og, wotl or eternal: the last one chosen (if there are several games, you are asked at startup)\ngame =\nfullscreen = 1\nvsync = 1\nwidth = 1440\nheight = 1080\nvolume = 100\n"
             "; smooth = 1 interpolates movement and animations to the refresh rate; 0 = original 30 fps\n"
             "smooth = 1\nfilters = 1\n"
             "; cache_mb = memory for already rasterized graphics (MB)\n"
             "cache_mb = " << fl::g_lim.defaultCacheMB << "\n"
             "; low_latency = 1 keeps reading input while waiting for the display (0 = off)\n"
             "low_latency = 1\n"
             "; interp_lead = 0 (experimental, up to 0.5): draw ahead by that fraction of a tick; less visual delay, slight overshoot\n"
             "interp_lead = 0\n"
             "; performance = 1 shows the performance diagnostics and writes them to performance.txt (same as F3)\n"
             "performance = 0\n"
             "; joystick_acceleration = 1: with an analog stick (controller, Switch, touch joystick) the speed follows how far\n"
             ";   it is pushed, for small precise moves (top speed unchanged); 0 = always full speed, as on the keyboard\n"
             "joystick_acceleration = 1\n"
             << kLateSettings[0][1] << "\ngc_after_tick = 1\n" << kLateSettings[1][1] << "\nmax_margin = " << c.maxMarginMs << "\n"
             << kLateSettings[2][1] << "\nmods =\n"
#if defined(__ANDROID__) || defined(__SWITCH__)
             "; touch_controls = 0 turns off the touch joysticks, HUD taps and pause button (touch then only works in menus)\n"
             "touch_controls = 1\n"
#endif
#ifdef __ANDROID__
             "; render_height = 0 picks the resolution automatically (starts at 720 and goes down if the device can't keep\n"
             ";   60 fps); or force one: 1080 (or the screen's: sharpest), 720, 600, 540, 480 (fastest)\n"
             "render_height = 0\n"
#endif
             ;
        return;
    }
    std::set<std::string> seen;
    while (std::getline(f, line)) {
        line = trim(line); if (line.empty() || line[0] == ';' || line[0] == '#') continue;
        size_t eq = line.find('='); if (eq == std::string::npos) continue;
        std::string k = trim(line.substr(0, eq)), v = trim(line.substr(eq + 1));
        seen.insert(k);
        int n = atoi(v.c_str());
        if (k == "swf") c.swf = v; else if (k == "game" || k == "juego") c.game = v; else if (k == "fullscreen") c.fullscreen = n; else if (k == "vsync") c.vsync = n;
        else if (k == "width") c.width = n; else if (k == "height") c.height = n; else if (k == "volume") c.volume = n;
        else if (k == "smooth") c.smooth = n; else if (k == "filters") c.filters = n; else if (k == "cache_mb") c.cacheMB = n; else if (k == "gpu_sync") c.gpuSync = n; else if (k == "low_latency") c.lowLatency = n; else if (k == "interp_lead") c.interpLead = std::max(0.0, std::min(0.5, atof(v.c_str())));
        else if (k == "performance") c.performance = n; else if (k == "touch_controls") c.touchControls = n;
        else if (k == "joystick_acceleration") c.joystickAcceleration = n;
        else if (k == "render_height") c.renderHeight = n; else if (k == "auto_height") c.autoHeight = n;
        else if (k == "gc_after_tick") c.gcAfterTick = n; else if (k == "max_margin") c.maxMarginMs = std::max(0, std::min(30, n));
        else if (k == "mods") c.mods = v;
    }
    f.close();
    {   // an isaac.ini from an earlier version: add the newer settings (with their defaults) so they can be changed
        std::string add;
        for (auto& ls : kLateSettings) if (!seen.count(ls[0]))
            add += std::string("\n") + ls[1] + "\n" + ls[0] + " = " + (std::string(ls[0]) == "max_margin" ? std::to_string(c.maxMarginMs) : std::string(ls[2] ? ls[2] : "1")) + "\n";
        if (!add.empty()) { std::ofstream o(path, std::ios::app); o << add; }
    }
#ifdef __SWITCH__
    // console memory is fixed: never more than twice the planned amount, and never less (an isaac.ini from an
    // earlier version stored a smaller value, which no longer leaves room for the preload)
    c.cacheMB = std::max(fl::g_lim.defaultCacheMB, std::min(c.cacheMB, 2 * fl::g_lim.defaultCacheMB));
#endif
    if (c.cacheMB < 32) c.cacheMB = 32;
}
// changes (or adds) "key = value" in isaac.ini, preserving the rest of the file
static void saveIniValue(const std::string& path, const std::string& key, const std::string& val) {
    std::ifstream f(path); std::vector<std::string> lines; std::string line; bool found = false;
    while (std::getline(f, line)) {
        std::string t = trim(line);
        if (!line.empty() && line.back() == '\r') line.pop_back();
        size_t eq = t.find('=');
        if (!t.empty() && t[0] != ';' && eq != std::string::npos && trim(t.substr(0, eq)) == key) { line = key + " = " + val; found = true; }
        lines.push_back(line);
    }
    f.close();
    if (!found) lines.push_back(key + " = " + val);
    std::ofstream o(path, std::ios::binary);
#ifdef _WIN32
    for (auto& l : lines) o << l << "\r\n";
#else
    for (auto& l : lines) o << l << "\n";
#endif
}
#ifdef __SWITCH__
// ---- Switch controllers (libnx). In gameplay, the face buttons shoot according to their position (like the
// keyboard arrows); in menus and in the pause screen, A confirms and B goes back.
struct PadMap { u64 btn; int game; int pause; int menu; };   // Flash key in gameplay / paused / in menus
static const PadMap kPadMap[] = {
    {HidNpadButton_A, 39, 32, 32}, {HidNpadButton_B, 40, 27, 27}, {HidNpadButton_X, 38, 38, 38}, {HidNpadButton_Y, 37, 37, 37},
    // L bomb (E), ZL active item (space), R card/pill (Q); ZR held for 5 s restarts the run (see padPoll)
    {HidNpadButton_L, 'E', 0, 0}, {HidNpadButton_ZL, 32, 0, 0}, {HidNpadButton_R, 'Q', 32, 32}, {HidNpadButton_ZR, 0, 32, 32},
    {HidNpadButton_Plus, 'P', 'P', 13}, {HidNpadButton_Minus, 'P', 'P', 27},   // + and -: pause
    {HidNpadButton_Left, 'A', 37, 37}, {HidNpadButton_Right, 'D', 39, 39}, {HidNpadButton_Up, 'W', 38, 38}, {HidNpadButton_Down, 'S', 40, 40},
    {HidNpadButton_StickLLeft, 'A', 37, 37}, {HidNpadButton_StickLRight, 'D', 39, 39}, {HidNpadButton_StickLUp, 'W', 38, 38}, {HidNpadButton_StickLDown, 'S', 40, 40},
    {HidNpadButton_StickRLeft, 37, 37, 37}, {HidNpadButton_StickRRight, 39, 39, 39}, {HidNpadButton_StickRUp, 38, 38, 38}, {HidNpadButton_StickRDown, 40, 40, 40},
};
#endif
// error message (Switch: text console until + is pressed)
static void showError(const char* title, const char* msg, SDL_Window* w = nullptr) {
#ifdef __SWITCH__
    (void)title; (void)w;
    consoleInit(NULL);
    printf("\n  The Binding of Isaac: Wrath of the Lamb (recomp)\n\n");
    for (const char* p = msg; *p; p++) putchar(*p);
    printf("\n\n  Press + to exit.\n");
    PadState pad; padConfigureInput(1, HidNpadStyleSet_NpadStandard); padInitializeDefault(&pad);
    while (appletMainLoop()) { padUpdate(&pad); if (padGetButtonsDown(&pad) & HidNpadButton_Plus) break; consoleUpdate(NULL); }
    consoleExit(NULL);
#else
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, msg, w);
#endif
}
static bool fileExists(const std::string& p) { FILE* f = fopen(p.c_str(), "rb"); if (f) fclose(f); return f != nullptr; }

#ifdef __ANDROID__
// ---- Android: static helpers of the Java activity (android/app/src/main/java/.../IsaacActivity.java)
static jclass activityClass(JNIEnv* env) {
    jobject act = (jobject)SDL_AndroidGetActivity();
    jclass c = env->GetObjectClass(act); env->DeleteLocalRef(act); return c;
}
static std::string javaString(const char* method) {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv(); jclass c = activityClass(env);
    jmethodID m = env->GetStaticMethodID(c, method, "()Ljava/lang/String;");
    std::string r;
    if (m) { jstring js = (jstring)env->CallStaticObjectMethod(c, m); if (js) { const char* u = env->GetStringUTFChars(js, nullptr); r = u; env->ReleaseStringUTFChars(js, u); env->DeleteLocalRef(js); } }
    env->DeleteLocalRef(c); return r;
}
static bool javaBool(const char* method) {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv(); jclass c = activityClass(env);
    jmethodID m = env->GetStaticMethodID(c, method, "()Z");
    bool r = m && env->CallStaticBooleanMethod(c, m);
    env->DeleteLocalRef(c); return r;
}
static int javaInt(const char* method) {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv(); jclass c = activityClass(env);
    jmethodID m = env->GetStaticMethodID(c, method, "()I");
    int r = m ? env->CallStaticIntMethod(c, m) : 0;
    env->DeleteLocalRef(c); return r;
}
static void javaVoidInt(const char* method, int arg) {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv(); jclass c = activityClass(env);
    jmethodID m = env->GetStaticMethodID(c, method, "(I)V");
    if (m) env->CallStaticVoidMethod(c, m, (jint)arg);
    env->DeleteLocalRef(c);
}
static void javaVoid(const char* method, const char* arg = nullptr) {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv(); jclass c = activityClass(env);
    jmethodID m = env->GetStaticMethodID(c, method, arg ? "(Ljava/lang/String;)V" : "()V");
    if (m) { if (arg) { jstring js = env->NewStringUTF(arg); env->CallStaticVoidMethod(c, m, js); env->DeleteLocalRef(js); } else env->CallStaticVoidMethod(c, m); }
    env->DeleteLocalRef(c);
}
#endif

// ---- test script (--script): "tick action arg" per line
struct ScriptCmd { uint32_t tick; std::string op; std::string arg; };

#if !defined(__SWITCH__) && !defined(_WIN32) && !defined(__ANDROID__)
// (diagnostics: stack sampling with SIGPROF during a specific tick, ISAAC_SAMPLE_TICK=n)
#include <execinfo.h>
#include <signal.h>
#include <sys/time.h>
// (no memory allocation in the signal handler: fixed buffer)
static void* g_sbuf[20000][24]; static int g_slen[20000]; static volatile int g_sn = 0;
static volatile bool g_sampling = false;
static void profHandler(int) { if (!g_sampling || g_sn >= 20000) return; int i = g_sn; g_slen[i] = backtrace(g_sbuf[i], 24); g_sn = i + 1; }
static void profSampleStart() {
    void* warm[4]; backtrace(warm, 4);   // (the first call loads libgcc: outside the signal)
    signal(SIGPROF, profHandler);
    static timer_t tid; sigevent se{}; se.sigev_notify = SIGEV_SIGNAL; se.sigev_signo = SIGPROF;
    timer_create(CLOCK_MONOTONIC, &se, &tid);
    itimerspec its{}; its.it_interval.tv_nsec = 250000; its.it_value.tv_nsec = 250000; timer_settime(tid, 0, &its, nullptr);
    g_sampling = true;
}
static void profSampleStop() {
    g_sampling = false;
    FILE* f = fopen(getenv("ISAAC_SAMPLE_OUT") ? getenv("ISAAC_SAMPLE_OUT") : "samples.txt", "w");
    for (int k = 0; k < g_sn; k++) {
        char** syms = backtrace_symbols(g_sbuf[k], g_slen[k]);
        for (int i = 2; i < g_slen[k]; i++) { std::string t = syms[i]; size_t a = t.find("(+"), b = t.find(')', a);
            fprintf(f, "%s ", a != std::string::npos && b != std::string::npos ? t.substr(a + 2, b - a - 2).c_str() : "?"); }
        fprintf(f, "\n"); free(syms);
    }
    fclose(f);
}
#else
static void profSampleStart() {}
static void profSampleStop() {}
#endif
static double g_benchAlpha = 1.0;
// Windowed: fit the whole window (with its title bar) inside the usable screen area, without going under the
// taskbar, keeping the aspect ratio of the requested size, and centered.
static void fitWindow(SDL_Window* win, int wantW, int wantH) {
    if (!win || (SDL_GetWindowFlags(win) & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP))) return;
    int di = SDL_GetWindowDisplayIndex(win); if (di < 0) di = 0;
    SDL_Rect ub; if (SDL_GetDisplayUsableBounds(di, &ub) != 0) return;
    int top = 0, left = 0, bottom = 0, right = 0;
    if (SDL_GetWindowBordersSize(win, &top, &left, &bottom, &right) != 0 || top <= 0) { top = 32; left = right = bottom = 8; }   // (not decorated yet)
    int maxW = ub.w - left - right, maxH = ub.h - top - bottom;
    double k = std::min({1.0, (double)maxW / std::max(1, wantW), (double)maxH / std::max(1, wantH)});
    int w = std::max(320, (int)(wantW * k)), h = std::max(240, (int)(wantH * k));
    SDL_SetWindowSize(win, w, h);
    SDL_SetWindowPosition(win, ub.x + left + (maxW - w) / 2, ub.y + top + (maxH - h) / 2);
}
// ---- graphics preload: built-in list (one per game) + list learned on this console/PC (preload*.txt)
static const fl::PreKey kPreWotl[] = {
#include "preload_wotl.inc"
    {0, 0, false, 0, 0}
};
static const fl::PreKey kPreOg[] = {
#include "preload_og.inc"
    {0, 0, false, 0, 0}
};
static const fl::PreKey kPreEternal[] = {
#include "preload_eternal.inc"
    {0, 0, false, 0, 0}
};
static std::vector<fl::PreKey> loadPreloadList(const std::string& path, const std::string& game) {
    const fl::PreKey* k = game == "og" ? kPreOg : game == "eternal" ? kPreEternal : kPreWotl;
    size_t n = game == "og" ? sizeof kPreOg / sizeof *kPreOg : game == "eternal" ? sizeof kPreEternal / sizeof *kPreEternal : sizeof kPreWotl / sizeof *kPreWotl;
    std::vector<fl::PreKey> v;
    if (game != "none") v.assign(k, k + n - 1);   // ("none": only the file)
    if (FILE* f = fopen(path.c_str(), "rb")) {
        char line[128];
        while (fgets(line, sizeof line, f)) {
            if (line[0] == '#') continue;
            int id, kind, abs, var, q;
            if (sscanf(line, "%d %d %d %d %d", &id, &kind, &abs, &var, &q) == 5) {
                fl::PreKey p; p.id = (uint16_t)id; p.kind = (uint8_t)kind; p.abs = abs != 0; p.variant = (uint16_t)var; p.q100 = (int16_t)q; v.push_back(p);
            }
        }
        fclose(f);
    }
    return v;
}
static void saveLearned(const std::string& path) {
    std::vector<fl::PreKey> v = fl::renderTakeLearned();
    if (v.empty()) return;
    std::string t;
    char b[64];
    for (auto& p : v) { snprintf(b, sizeof b, "%d %d %d %d %d\n", p.id, p.kind, p.abs ? 1 : 0, p.variant, p.q100); t += b; }
    fl::platformAppendFileAsync(path, t);
}   // (--alpha: fixed interpolation in test mode, to measure its cost)
// Game selection screen (there are several versions in the folder). Keyboard, mouse, controller and touch.
// Returns the chosen index, or -1 if the window is closed.
static int chooseGame(SDL_Window* win, size_t n, const std::function<std::string(size_t)>& title, int sel) {
    std::vector<std::string> items; for (size_t i = 0; i < n; i++) items.push_back(title(i));
#ifdef __SWITCH__
    const char* hint = "A: PLAY    UP/DOWN: CHOOSE";
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    PadState pad; padInitializeDefault(&pad);
#else
    const char* hint = "ENTER: PLAY    ARROWS: CHOOSE    ESC: QUIT";
    for (int i = 0; i < SDL_NumJoysticks(); i++) if (SDL_IsGameController(i)) SDL_GameControllerOpen(i);
#endif
    std::vector<std::array<int, 4>> rects;
    int pressed = -1;
    for (;;) {
        int dw, dh; SDL_GL_GetDrawableSize(win, &dw, &dh);
        int ww, wh; SDL_GetWindowSize(win, &ww, &wh);
        double fx = ww ? (double)dw / ww : 1, fy = wh ? (double)dh / wh : 1;
        auto hit = [&](double x, double y) { for (size_t i = 0; i < rects.size(); i++) if (x >= rects[i][0] && x < rects[i][2] && y >= rects[i][1] && y < rects[i][3]) return (int)i; return -1; };
        int done = -2;
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT: return -1;
                case SDL_KEYDOWN:
                    switch (e.key.keysym.sym) {
                        case SDLK_UP: case SDLK_w: sel = (sel + (int)n - 1) % (int)n; break;
                        case SDLK_DOWN: case SDLK_s: sel = (sel + 1) % (int)n; break;
                        case SDLK_RETURN: case SDLK_KP_ENTER: case SDLK_SPACE: done = sel; break;
                        case SDLK_ESCAPE: return -1;
                        default: break;
                    }
                    break;
                case SDL_MOUSEMOTION: { int h = hit(e.motion.x * fx, e.motion.y * fy); if (h >= 0) sel = h; break; }
                case SDL_MOUSEBUTTONDOWN: if (e.button.button == SDL_BUTTON_LEFT) pressed = hit(e.button.x * fx, e.button.y * fy); break;
                case SDL_MOUSEBUTTONUP: if (e.button.button == SDL_BUTTON_LEFT) { int h = hit(e.button.x * fx, e.button.y * fy); if (h >= 0 && h == pressed) done = h; pressed = -1; } break;
                case SDL_FINGERDOWN: { int h = hit(e.tfinger.x * dw, e.tfinger.y * dh); if (h >= 0) sel = h; pressed = h; break; }
                case SDL_FINGERUP: { int h = hit(e.tfinger.x * dw, e.tfinger.y * dh); if (h >= 0 && h == pressed) done = h; pressed = -1; break; }
#ifndef __SWITCH__
                case SDL_CONTROLLERBUTTONDOWN:
                    if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP) sel = (sel + (int)n - 1) % (int)n;
                    else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) sel = (sel + 1) % (int)n;
                    else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_A || e.cbutton.button == SDL_CONTROLLER_BUTTON_START) done = sel;
                    break;
#endif
                default: break;
            }
        }
#ifdef __SWITCH__
        if (!appletMainLoop()) return -1;
        padUpdate(&pad);
        uint64_t k = padGetButtonsDown(&pad);
        if (k & (HidNpadButton_Up | HidNpadButton_StickLUp)) sel = (sel + (int)n - 1) % (int)n;
        if (k & (HidNpadButton_Down | HidNpadButton_StickLDown)) sel = (sel + 1) % (int)n;
        if (k & (HidNpadButton_A | HidNpadButton_Plus)) done = sel;
#endif
        if (done >= 0) return done;
        fl::renderChooser(dw, dh, "THE BINDING OF ISAAC", items, sel, hint, &rects);
        fl::renderEndFrame();
        if (const char* shot = getenv("ISAAC_CHOOSER_SHOT")) { fl::renderScreenshot(shot); return -1; }   // (testing)
        SDL_GL_SwapWindow(win);
        SDL_Delay(kIsSwitch ? 1 : 0);
    }
}

int main(int argc, char** argv) {
    fl::platformInitLimits();
    Config cfg;
#ifdef __SWITCH__
    // everything on the SD card: the .swf, isaac.ini, the save and the screenshots
    std::string base = "sdmc:/switch/isaac/";
    mkdir("sdmc:/switch", 0777); mkdir("sdmc:/switch/isaac", 0777);
    // saves (and the learned preload lists, screenshots, performance.txt) in sd:/switch/isaac/saves/, as on PC (the
    // folder next to the program). Up to 1.3.1 they were in sd:/switch/isaac/ itself: copied over the first time.
    std::string pref = base + "saves/";
    bool prefOk = mkdir("sdmc:/switch/isaac/saves", 0777) == 0 || errno == EEXIST;
    if (!prefOk) pref = base;
    freopen((base + "log.txt").c_str(), "w", stderr);   // program messages (useful if something fails)
    setvbuf(stderr, nullptr, _IONBF, 0);
    if (prefOk)
        for (const char* f : {"isaac_save.sol", "isaac_og_save.sol", "isaac_eternal_save.sol", "preload.txt", "preload_og.txt", "preload_eternal.txt"})
            if (!platformFileExists(pref + f) && platformFileExists(base + f) && platformCopyFile(base + f, pref + f))
                fprintf(stderr, "%s copied to the saves folder\n", f);
    // an uncaught exception must not return to the loader with threads (audio) still alive: log it and
    // terminate the whole process
    std::set_terminate([] {
        const char* what = "(unknown)";
        try { if (auto e = std::current_exception()) std::rethrow_exception(e); }
        catch (const std::exception& e) { what = e.what(); } catch (...) {}
        int used, total; fl::platformMemInfo(&used, &total);
        fprintf(stderr, "FATAL ERROR: uncaught exception: %s (memory %d/%d MB)\n", what, used, total);
        fflush(stderr);
        svcExitProcess();
    });
    {
        int used, total; fl::platformMemInfo(&used, &total);
        AppletType at = appletGetAppletType();
        fprintf(stderr, "Isaac recomp (Switch) - %s mode, memory %d/%d MB\n",
                at == AppletType_Application || at == AppletType_SystemApplication ? "application" : "applet (Album)", used, total);
        // Video capture (holding the Capture button): automatic for a host game whose setting is "automatic"; one set
        // to "enabled" needs the program to ask for it (libnx: appletInitializeGamePlayRecording). Only as an
        // application (the Album applet can't record at all).
        if (at == AppletType_Application) {
            bool sup = false;
            if (R_SUCCEEDED(appletIsGamePlayRecordingSupported(&sup)) && sup && R_SUCCEEDED(appletInitializeGamePlayRecording()))
                appletSetGamePlayRecordingState(true);
            fprintf(stderr, "video capture: %s\n", sup ? "available" : "not available with this host game");
        }
    }
#elif defined(__ANDROID__)
    // everything in the IsaacRecomp folder at the root of the internal storage (/storage/emulated/0/IsaacRecomp):
    // Isaac.exe, isaac.ini, saves, log.txt, performance.txt. Reading a file we didn't create there needs
    // "All files access" (Android 11+) or the storage permission (older versions): the activity asks for it.
    SDL_Init(SDL_INIT_VIDEO);
    if (!javaBool("hasStorageAccess")) {
        javaVoid("requestStorageAccess");
        while (!javaBool("hasStorageAccess")) {
            SDL_Event e; while (SDL_PollEvent(&e)) if (e.type == SDL_QUIT) { SDL_Quit(); exit(0); }
            SDL_Delay(100);
        }
    }
    std::string base = javaString("rootDir");
    if (base.empty()) base = "/storage/emulated/0/IsaacRecomp";
    mkdir(base.c_str(), 0777);
    base += "/";
    // saves (and the learned preload lists, screenshots, performance.txt) in IsaacRecomp/saves/, as on PC and Switch.
    // Up to 1.3.1 they were in IsaacRecomp/ itself: copied over the first time.
    std::string pref = base + "saves/";
    bool prefOk = mkdir((base + "saves").c_str(), 0777) == 0 || errno == EEXIST;
    if (!prefOk) pref = base;
    freopen((base + "log.txt").c_str(), "w", stderr);
    setvbuf(stderr, nullptr, _IONBF, 0);
    if (prefOk)
        for (const char* f : {"isaac_save.sol", "isaac_og_save.sol", "isaac_eternal_save.sol", "preload.txt", "preload_og.txt", "preload_eternal.txt"})
            if (!platformFileExists(pref + f) && platformFileExists(base + f) && platformCopyFile(base + f, pref + f))
                fprintf(stderr, "%s copied to the saves folder\n", f);
    std::set_terminate([] {
        const char* what = "(unknown)";
        try { if (auto e = std::current_exception()) std::rethrow_exception(e); }
        catch (const std::exception& e) { what = e.what(); } catch (...) {}
        fprintf(stderr, "FATAL ERROR: uncaught exception: %s\n", what); fflush(stderr);
        _exit(1);
    });
    logf("Isaac recomp (Android) - data folder %s\n", base.c_str());
#else
    std::string base = SDL_GetBasePath() ? SDL_GetBasePath() : "./";
    // saves (and the learned preload lists, screenshots, performance.txt) in a "saves" folder next to the program.
    // Up to 1.3.0 they were in the user's profile (%APPDATA%\IsaacRecomp\WrathOfTheLamb, ~/.local/share/...): what
    // is there is copied over the first time.
    #ifdef _WIN32
    std::string pref = base + "saves\\";
#else
    std::string pref = base + "saves/";
#endif
    if (!platformMkdir(pref.substr(0, pref.size() - 1))) {
        char* p = SDL_GetPrefPath("IsaacRecomp", "WrathOfTheLamb"); pref = p ? p : base; if (p) SDL_free(p);   // (read-only folder)
    } else if (char* p = SDL_GetPrefPath("IsaacRecomp", "WrathOfTheLamb")) {
        std::string old = p; SDL_free(p);
        for (const char* f : {"isaac_save.sol", "isaac_og_save.sol", "isaac_eternal_save.sol", "preload.txt", "preload_og.txt", "preload_eternal.txt"})
            if (!platformFileExists(pref + f) && platformFileExists(old + f) && platformCopyFile(old + f, pref + f))
                logf("%s copied from %s to the saves folder\n", f, old.c_str());
    }
#endif
#ifdef FL_LEAN
    fl::g_leanSwf = getenv("ISAAC_NOLEAN") == nullptr; fl::g_leanCacheDir = pref;   // (lean loading: PSP / tests)
#endif
    loadConfig(base + "isaac.ini", cfg);
    {   // mods: the "mods" folder next to the program (mods.h), turned on with `mods =` in isaac.ini
#ifdef _WIN32
        std::string modsDir = base + "mods\\";
#else
        std::string modsDir = base + "mods/";
#endif
        const char* env = getenv("ISAAC_MODS");   // (tests)
        fl::g_modsSaveDir = pref;
        fl::modsInit(modsDir, env ? env : cfg.mods);
    }
    {   // control bindings (rebinding screen: Options > Controls > Rebind controls)
        std::map<std::string, std::string> kv; std::ifstream f(base + "isaac.ini"); std::string line;
        while (std::getline(f, line)) {
            std::string t = trim(line); if (t.empty() || t[0] == ';' || t[0] == '#') continue;
            size_t eq = t.find('='); if (eq != std::string::npos) kv[trim(t.substr(0, eq))] = trim(t.substr(eq + 1));
        }
        bindingsLoad([&](const std::string& k) { auto it = kv.find(k); return it == kv.end() ? std::string() : it->second; });
        TouchControls::setLayout(g_bind.touchSwap, g_bind.touchSize, g_bind.touchType);
    }
    std::string forcedGame;   // --game og|wotl|eternal (testing, and restart after "Delete save")
    bool headless = false, norender = false, stats = false, perfStart = false; int maxFrames = -1; uint32_t monkeySeed = 0; std::string wavPath; std::vector<ScriptCmd> script; std::string shotPrefix;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--windowed") cfg.fullscreen = false;
        else if (a == "--fullscreen") cfg.fullscreen = true;
        else if (a == "--headless") { headless = true; cfg.fullscreen = false; cfg.vsync = false; }
        else if (a == "--frames" && i + 1 < argc) maxFrames = atoi(argv[++i]);
        else if (a == "--size" && i + 1 < argc) { sscanf(argv[++i], "%dx%d", &cfg.width, &cfg.height); }
        else if (a == "--trace") g_trace = true;
        else if (a == "--norender") norender = true;
        else if (a == "--perf") perfStart = true;
        else if (a == "--stats") { stats = true; fl::g_gcLog = true; }
        else if (a == "--monkey" && i + 1 < argc) monkeySeed = (uint32_t)atoi(argv[++i]);
        else if (a == "--wav" && i + 1 < argc) wavPath = argv[++i];
        else if (a == "--nosmooth") cfg.smooth = false;
        else if (a == "--alpha" && i + 1 < argc) g_benchAlpha = atof(argv[++i]);
        else if (a == "--save" && i + 1 < argc) g_savePath = argv[++i];
        else if (a == "--game" && i + 1 < argc) { forcedGame = argv[++i]; }
        else if (a == "--script" && i + 1 < argc) {
            std::ifstream f(argv[++i]); std::string line;
            while (std::getline(f, line)) { std::istringstream ss(line); ScriptCmd c; if (ss >> c.tick >> c.op) { std::getline(ss, c.arg); c.arg = trim(c.arg); script.push_back(c); } }
        } else if (a.size() > 4) { std::string ext = a.substr(a.size() - 4); for (auto& ch : ext) ch = (char)tolower((unsigned char)ch); if (ext == ".swf" || ext == ".exe") cfg.swf = a; }
    }
    if (headless) fl::g_rngSeedFixed = true;   // (deterministic tests)
    // ---- game data: the Steam Isaac.exe (the SWF is inside it) or an already extracted .swf. Three versions are
    // supported (original, Wrath of the Lamb and Eternal Edition), each with its own recompiled code; all
    // .exe/.swf files in the folder are scanned and, if there is more than one game, the user is asked which to open.
    struct Found { const KnownGame* g; std::string path; };
    std::vector<Found> found;
    auto addFile = [&](const std::string& path) {
        uint32_t len = peekSwfLength(path);
        const KnownGame* g = len ? knownGameBySwfLen(len) : nullptr;
        if (!g) return false;
        for (auto& f : found) if (f.g == g) return true;   // (one per game: the first one found)
        found.push_back({g, path});
        return true;
    };
    if (!cfg.swf.empty()) addFile(cfg.swf);
    if (found.empty()) {
        std::vector<std::string> dirs = {base};
        if (base != "./") dirs.push_back("./");
        for (auto& dir : dirs) {
            std::vector<std::string> names;
            if (DIR* dp = opendir(dir.c_str())) {
                while (dirent* e = readdir(dp)) {
                    std::string n = e->d_name; if (n.size() < 5) continue;
                    std::string ext = n.substr(n.size() - 4); for (auto& ch : ext) ch = (char)tolower((unsigned char)ch);
                    std::string low = n; for (auto& ch : low) ch = (char)tolower((unsigned char)ch);
                    if ((ext == ".exe" || ext == ".swf") && low != "isaacrecomp.exe") names.push_back(n);
                }
                closedir(dp);
            }
            std::sort(names.begin(), names.end(), [](const std::string& x, const std::string& y) {   // Isaac.exe first
                auto rank = [](const std::string& n) { std::string l = n; for (auto& c : l) c = (char)tolower((unsigned char)c); return l == "isaac.exe" ? 0 : 1; };
                return rank(x) != rank(y) ? rank(x) < rank(y) : x < y; });
            for (auto& n : names) addFile(dir + n);
            if (!found.empty()) break;
        }
    }
    std::sort(found.begin(), found.end(), [](const Found& x, const Found& y) { return x.g < y.g; });   // release order
    for (auto& f : found) logf("game found: %s at %s\n", f.g->title, f.path.c_str());

#ifdef __SWITCH__
    Uint32 sdlFlags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER;   // controllers are read through libnx
#else
    Uint32 sdlFlags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_TIMER;
#endif
    if (headless && norender) sdlFlags = SDL_INIT_TIMER | SDL_INIT_AUDIO;
    if (getenv("ISAAC_TOUCH")) cfg.touchControls = true;   // (testing on PC)
    if (cfg.touchControls) SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");   // touches are handled as such, not as a mouse
#ifdef __ANDROID__
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
    SDL_SetHint(SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "1");   // back = pause / Esc (instead of closing the app)
#endif
    if (SDL_Init(sdlFlags) != 0) { fprintf(stderr, "SDL: %s\n", SDL_GetError()); return 1; }
    if (found.empty()) {
#if defined(__ANDROID__)
        showError("The Binding of Isaac",
            ("Game data not found.\n\nCopy 'Isaac.exe' from the game's Steam folder into the IsaacRecomp folder of your "
             "device's internal storage:\n\n" + base + "\n\nThe original, Wrath of the Lamb and Eternal Edition all work; if you "
             "have several, give them different names (for example Isaac-Eternal.exe).").c_str());
#elif defined(__SWITCH__)
        showError("The Binding of Isaac", "  Game data not found.\n\n  Copy 'Isaac.exe' from the game's Steam folder\n  into the  sd:/switch/isaac/  folder on the SD card.\n\n"
                  "  The original, Wrath of the Lamb and Eternal Edition all work; if you\n  have several, give them different names (e.g. Isaac-Eternal.exe).");
#else
        showError("The Binding of Isaac",
            "Game data not found.\n\nCopy 'Isaac.exe' from the game's Steam folder next to this program\n"
            "or set its path in isaac.ini (swf = ...).\n\nThe original, Wrath of the Lamb and Eternal Edition all work; if you have several,\n"
            "give them different names (for example Isaac-Eternal.exe).");
#endif
        return 1;
    }
#ifdef __ANDROID__
    g_glES = true;
#else
    g_glES = getenv("ISAAC_GLES") != nullptr;   // (testing the Android renderer on PC)
#endif
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3); SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, g_glES ? 0 : 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, g_glES ? SDL_GL_CONTEXT_PROFILE_ES : SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8); SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    Uint32 wflags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
#ifdef __SWITCH__
    // 1080p when docked, 720p in handheld mode (SDL resizes on dock/undock)
    if (appletGetOperationMode() == AppletOperationMode_Console) { cfg.width = 1920; cfg.height = 1080; } else { cfg.width = 1280; cfg.height = 720; }
#elif defined(__ANDROID__)
    wflags |= SDL_WINDOW_FULLSCREEN;
#else
    if (cfg.fullscreen) wflags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
#endif
    SDL_Window* win = nullptr; SDL_GLContext gl = nullptr;
    if (!(headless && norender)) {   // (logic tests need neither a window nor a GPU)
        win = SDL_CreateWindow("The Binding of Isaac", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, cfg.width, cfg.height, wflags);
        if (win) { int w0, h0; SDL_GetWindowSize(win, &w0, &h0); if (h0 > 0) fl::g_windowAspect = (float)w0 / h0; }
        if (!win) { fprintf(stderr, "window: %s\n", SDL_GetError()); return 1; }
#if !defined(__SWITCH__) && !defined(__ANDROID__) && !defined(_WIN32)
        {   // window / taskbar icon (Windows: from the .exe's resources, windows/isaac.ico)
            static const unsigned char kIcon[64 * 64 * 4] = {
#include "icon64.inc"
            };
            if (SDL_Surface* ic = SDL_CreateRGBSurfaceWithFormatFrom((void*)kIcon, 64, 64, 32, 64 * 4, SDL_PIXELFORMAT_RGBA32)) { SDL_SetWindowIcon(win, ic); SDL_FreeSurface(ic); }
        }
#endif
#if !defined(__SWITCH__) && !defined(__ANDROID__)
        if (!headless) { SDL_PumpEvents(); fitWindow(win, cfg.width, cfg.height); }
#endif
        gl = SDL_GL_CreateContext(win);
        if (!gl) { showError("The Binding of Isaac", g_glES ? "OpenGL ES 3.0 is required" : "OpenGL 3.3 is required", win); return 1; }
        SDL_GL_SetSwapInterval(cfg.vsync ? 1 : 0);
        if (!renderInit()) { showError("The Binding of Isaac", g_glES ? "Could not initialize OpenGL ES 3.0" : "Could not initialize OpenGL 3.3", win); return 1; }
        g_gpuBitmaps = getenv("ISAAC_CPUBITMAPS") == nullptr;   // layers that are only displayed: drawn on the GPU
#ifdef __ANDROID__
        {   // render resolution: the display hardware scales the surface up for free (see IsaacActivity.setRenderHeight)
            int screenH = javaInt("screenHeight");
            int want = cfg.renderHeight > 0 ? cfg.renderHeight : cfg.autoHeight > 0 ? cfg.autoHeight : 720;
            want = std::max(360, want);
            if (screenH > 0 && want < screenH) {
                javaVoidInt("setRenderHeight", want);
                for (int i = 0; i < 150; i++) {   // wait (up to 1.5 s) for the new surface size before preloading at it
                    SDL_Event e; while (SDL_PollEvent(&e)) {}
                    int w, h; SDL_GL_GetDrawableSize(win, &w, &h); if (h == want) break;
                    SDL_Delay(10);
                }
            }
            int w, h; SDL_GL_GetDrawableSize(win, &w, &h);
            logf("render resolution %dx%d (screen height %d, render_height %d%s)\n", w, h, screenH, cfg.renderHeight, cfg.renderHeight ? "" : " = automatic");
        }
#endif
#ifdef __SWITCH__
        {
            int used, total; platformMemInfo(&used, &total);
            logf("OpenGL ready: %dx%d, memory %d/%d MB\n", cfg.width, cfg.height, used, total);
        }
#endif
    }

    // ---- which game to open
    int choice = 0;
    {
        std::string want = !forcedGame.empty() ? forcedGame : cfg.game;
        for (size_t i = 0; i < found.size(); i++) if (want == found[i].g->tag) choice = (int)i;
        if (found.size() > 1 && forcedGame.empty() && win && !headless) {
            choice = chooseGame(win, found.size(), [&](size_t i) { return std::string(!strcmp(found[i].g->tag, "og") ? "Original" : found[i].g->title); }, choice);
            if (choice < 0) { SDL_GL_DeleteContext(gl); SDL_DestroyWindow(win); SDL_Quit(); return 0; }
            saveIniValue(base + "isaac.ini", "game", found[choice].g->tag);
        }
    }
    std::string err;
    fl::initGlobals();
    if (!g_movie.load(found[choice].path, err)) { showError("The Binding of Isaac", err.c_str(), win); return 1; }
    cfg.swf = found[choice].path;
    const std::string gameTag = g_game->tag;
    if (!g_gameExact && !headless)
        showError("The Binding of Isaac",
            "This file does not exactly match the Steam version the game was recompiled for.\n"
            "It may not work correctly. Use the Steam Isaac.exe.", win);
    // each game has its own save and preload list (Wrath of the Lamb keeps the old names)
    std::string gsuf = gameTag == "wotl" ? "" : "_" + gameTag;
    if (g_savePath.empty()) g_savePath = pref + "isaac" + gsuf + "_save.sol";
    if (win) SDL_SetWindowTitle(win, (std::string("The Binding of Isaac") + (gameTag == "og" ? "" : std::string(": ") + g_code->title)).c_str());
    g_joyAccel = cfg.joystickAcceleration;
    applyLoadPatches();
    logf("SWF loaded: v%d, %dx%d @ %.0f fps, recompiled stage %dx%d\n", g_movie.version, g_movie.width, g_movie.height, g_movie.frameRate, g_stageW, g_stageH);
    g_renderCfg.smooth = cfg.smooth; g_renderCfg.filters = cfg.filters; g_renderCfg.cacheMB = cfg.cacheMB;
    if (kIsAndroid || getenv("ISAAC_MOBILE_FILTERS")) { g_renderCfg.filterBudget = 4; g_renderCfg.filterMaxAge = 3; g_renderCfg.filterLowRes = true; }
    g_renderCfg.rasterWaitMs = headless ? 1e9 : kIsSwitch ? 3.0 : 6.0;   // (testing: always complete, deterministic)
    g_renderCfg.rasterWaitNoFallbackMs = headless ? 1e9 : 40.0;          // no other scale to show: better to wait
    g_audioVolume = cfg.volume / 100.f;
    audioInit(!headless);   // in test mode audio advances with ticks, not with the clock
    g_isFullscreen = win && (SDL_GetWindowFlags(win) & (kIsAndroid ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_FULLSCREEN_DESKTOP)) != 0;
    playerInit();
    std::string preloadPath = pref + "preload" + gsuf + ".txt";
    size_t preloadTotal = 0;
    if (win && (!headless || getenv("ISAAC_PRELOAD"))) {   // (automated tests skip preloading: identical result, faster)
        // in the background, while the intro cutscene plays (it can't be skipped until this finishes)
        std::vector<PreKey> keys = loadPreloadList(preloadPath, gameTag);
        {   // list learned by earlier releases (precarga*.txt): merged once into the new file
            std::string old = pref + "precarga" + gsuf + ".txt";
            if (fileExists(old)) {
                std::vector<PreKey> o = loadPreloadList(old, "none");
                std::string t; char b[64];
                for (auto& p : o) { snprintf(b, sizeof b, "%d %d %d %d %d\n", p.id, p.kind, p.abs ? 1 : 0, p.variant, p.q100); t += b; keys.push_back(p); }
                if (FILE* f = fopen(preloadPath.c_str(), "ab")) { fputs(t.c_str(), f); fclose(f); }
                remove(old.c_str());
            }
        }
        int dw, dh; SDL_GL_GetDrawableSize(win, &dw, &dh);
        renderPreload(keys, dw, dh, nullptr);
        preloadTotal = keys.size();
        logf("preload: %zu graphics queued\n", keys.size());
    }
    for (int i = 0; i < SDL_NumJoysticks(); i++) if (SDL_IsGameController(i)) SDL_GameControllerOpen(i);

    const double dt = 1.0 / g_frameRate;
    const double freq = (double)SDL_GetPerformanceFrequency();
    auto nowSec = [&]() { return (double)SDL_GetPerformanceCounter() / freq; };
    bool running = true; size_t scriptPos = 0; uint64_t renderedFrames = 0;
    std::vector<double> logicMs, renderMs, allocs; std::vector<int> ballCount;
    // ---- performance diagnostics (F3): overlay + performance.txt in the saves folder
    bool perfOn = perfStart || cfg.performance; std::vector<std::string> perfLines;
    struct PerfWin { int frames = 0, ticks = 0, slow = 0, rasters = 0, held = 0, tex = 0, pending = 0; double frameMax = 0, logic = 0, logicMax = 0, render = 0, renderMax = 0, swap = 0, rasterMs = 0, upMB = 0, upMs = 0;
                     double adopt = 0, compile = 0, exec = 0, other = 0; int ent = 0, nodes = 0, quads = 0, draws = 0, filters = 0, fhits = 0; } pw;
    double pwStart = nowSec();
    // performance.txt is written from a separate thread (writing to the SD card is slow and must not stall the frame)
    std::string perfPath = pref + "performance.txt";
    auto perfLog = [&](const std::string& line) { platformAppendFileAsync(perfPath, line); };
    if (perfOn && !headless) perfLog("--- diagnostics enabled (performance = 1 in isaac.ini) ---\n");
    double gcMs0 = g_gcMs;
    auto entityCount = []() {
        Value b = getMemK(getMemK(Value((Object*)vm.root), intern("a")), intern("ball"));
        return b.isObj() && b.o->kind == OK_ARRAY ? (int)((Array*)b.o)->el.size() : 0;
    };
    auto runScript = [&](std::function<void(double, const std::string&)> shot) {
        while (scriptPos < script.size() && script[scriptPos].tick <= g_player.tick) {
            ScriptCmd& c = script[scriptPos++];
            if (c.op == "down") { int k = atoi(c.arg.c_str()); if (!(k == 27 && menuEscapeResumes())) keyEvent(k, k == 32 ? 32 : 0, true); }
            else if (c.op == "up") keyEvent(atoi(c.arg.c_str()), 0, false);
            else if (c.op == "shot") shot(1.0, c.arg);
            else if (c.op == "shotf") { std::istringstream ss(c.arg); double fr = 0; std::string file; ss >> fr >> file; shot(fr, file); }
            else if (c.op == "click") { double x, y; sscanf(c.arg.c_str(), "%lf %lf", &x, &y); mouseEvent(x, y, 0); mouseEvent(x, y, 1); mouseEvent(x, y, 2); }
            else if (c.op == "mmove" || c.op == "mdown" || c.op == "mup") { double x, y; sscanf(c.arg.c_str(), "%lf %lf", &x, &y); mouseEvent(x, y, c.op == "mmove" ? 0 : c.op == "mdown" ? 1 : 2); }
            else if (c.op == "eval") { debugEval(c.arg); }
            else if (c.op == "quit") running = false;
        }
    };
    auto monkeyStep = [&]() {
        if (!monkeySeed) return;
        static int from = getenv("ISAAC_MONKEY_FROM") ? atoi(getenv("ISAAC_MONKEY_FROM")) : 0;   // (testing: after the script)
        if ((int)g_player.tick < from) return;
        static uint32_t r = monkeySeed; static int held[8] = {0};
        static const int keysM[8] = {'W', 'A', 'S', 'D', 37, 38, 39, 40};
        auto rnd = [&]() { r = r * 1103515245u + 12345u; return (r >> 16) & 0x7fff; };
        for (int k = 0; k < 8; k++) {
            if (held[k] > 0 && --held[k] == 0) keyEvent(keysM[k], 0, false);
            else if (held[k] == 0 && rnd() % 40 == 0) { held[k] = 5 + rnd() % 40; keyEvent(keysM[k], 0, true); }
        }
        if (rnd() % 90 == 0) { int c = (int[]){32, 13, 'E', 69, 27, 'Q', 'P', 16}[rnd() % 8]; if (c == 27 && rnd() % 4) c = 32; keyEvent(c, c == 32 ? 32 : 0, true); keyEvent(c, 0, false); }
    };

    if (headless) {
        // ---------------------------------------------------------------- test mode: deterministic, single thread
        while (running && !g_player.quit) {
            SDL_Event e; while (SDL_PollEvent(&e)) if (e.type == SDL_QUIT) running = false;
            g_player.timeMs = g_player.tick * 1000.0 / g_frameRate;   // (exact: n * 1000 / 30, like the Ruffle harness)
            runScript([&](double fr, const std::string& file) {
                if (norender) return;
                int dw, dh; SDL_GL_GetDrawableSize(win, &dw, &dh); renderFrame(dw, dh, fr);
                if (perfOn) renderOverlay(perfLines);
                renderEndFrame();
                renderScreenshot(file.c_str());
            });
            if (!running) break;
            {   // (testing) auto-aim for the differential runs (debug.cpp)
                static int aimKey = 0;
                int k = fl::g_autoAim ? fl::autoAimKey() : 0;
                if (k != aimKey) { if (aimKey) keyEvent(aimKey, 0, false); if (k) keyEvent(k, 0, true); aimKey = k; }
            }
            monkeyStep();
            Uint64 lt0 = SDL_GetPerformanceCounter(); size_t heap0 = gcCount();
            static int sampleTick = getenv("ISAAC_SAMPLE_TICK") ? atoi(getenv("ISAAC_SAMPLE_TICK")) : -1;
            if ((int)g_player.tick == sampleTick) profSampleStart();
            runFrame(); runIntervals(); recompPatchesTick(); fl::modsTick();
            static const bool menuUi = getenv("ISAAC_MENUUI") != nullptr;   // (testing the controller menus / rebinding screen)
            if (menuUi) { bool ig = false, gp = recompInGameplay(&ig); menuUiTick(gp, ig); }
            gcCollect();
            if ((int)g_player.tick == sampleTick + 1) profSampleStop();
            double lms = (SDL_GetPerformanceCounter() - lt0) * 1000.0 / freq;
            static const bool tickMs = getenv("ISAAC_TICKMS") != nullptr;   // (testing) logic cost of every tick
            if (tickMs) logf("[tick] %u %.3f %d %zu\n", g_player.tick, lms, entityCount(), gcCount());
            if (stats) {
                int nb = entityCount();
                logicMs.push_back(lms); ballCount.push_back(nb); allocs.push_back((double)gcCount() - (double)heap0);
                if (lms > 8 && g_player.tick > 30) logf("[stats] slow logic tick %u: %.1f ms (%d entities)\n", g_player.tick, lms, nb);
            }
            if (maxFrames >= 0 && (int)g_player.tick >= maxFrames) running = false;
            fl::audioDumpTick(wavPath.empty() ? nullptr : wavPath.c_str());
            if (norender) continue;
            static const long renderFrom = getenv("ISAAC_RENDER_FROM") ? atol(getenv("ISAAC_RENDER_FROM")) : 0;   // (testing: faster shots)
            if ((long)g_player.tick < renderFrom) continue;
            g_rstats = RenderStats();
            SnapPtr snap = buildSnapshot(); snap->time = g_player.tick * dt;
            if (menuUi) menuUiFill(*snap);
            renderAdopt(snap);
            int dw, dh; SDL_GL_GetDrawableSize(win, &dw, &dh);
            Uint64 rt0 = SDL_GetPerformanceCounter();
            renderFrame(dw, dh, g_benchAlpha);
            static const bool gpuTime = getenv("ISAAC_GLFINISH") != nullptr;   // (testing: render time includes the GPU's)
            if (gpuTime) { renderEndFrame(); renderFinish(); }
            double rms = (SDL_GetPerformanceCounter() - rt0) * 1000.0 / freq;
            if (perfOn) renderOverlay(perfLines);
            renderEndFrame();
            double ovms = (SDL_GetPerformanceCounter() - rt0) * 1000.0 / freq - rms;
            if (stats) {
                renderMs.push_back(rms);
                static double sA, sC, sP, sE, sO; static int sN, sNodes, sCmds, sQuads, sDraws, sF, sFH; static long totRasters = 0;
                totRasters += g_rstats.rasters;
                if (g_player.tick % 500 == 0) logf("[render] rasterized up to tick %u: %ld\n", g_player.tick, totRasters);
                sA += g_rstats.adoptMs; sC += g_rstats.compileMs; sP += g_rstats.prerasterMs; sE += g_rstats.execMs; sO += ovms; sN++;
                sNodes += g_rstats.nodes; sCmds += g_rstats.cmds; sQuads += g_rstats.quads; sDraws += g_rstats.draws; sF += g_rstats.filters; sFH += g_rstats.filterHits;
                if (sN == 300) {
                    logf("[render] 300-frame average: adopt %.2f, compile %.2f, rasterize %.2f, draw %.2f, overlay %.2f ms | %d nodes, %d commands, %d quads, %d draws, filters %d (%d from cache)\n",
                         sA / sN, sC / sN, sP / sN, sE / sN, sO / sN, sNodes / sN, sCmds / sN, sQuads / sN, sDraws / sN, sF / sN, sFH / sN);
                    sA = sC = sP = sE = sO = 0; sN = sNodes = sCmds = sQuads = sDraws = sF = sFH = 0;
                }
                if (rms > 20) logf("[stats] slow render tick %u: %.1f ms (rasterized %d in %.1f ms, upload %.1f MB, %d quads, %d draws, %d filters, %d new textures)\n",
                                   g_player.tick, rms, g_rstats.rasters, g_rstats.rasterMs, g_rstats.uploadMB, g_rstats.quads, g_rstats.draws, g_rstats.filters, g_rstats.texCreates);
            }
            SDL_GL_SwapWindow(win); renderedFrames++;
        }
    } else {
        // ---------------------------------------------------------------- normal mode
        // Logic thread: exact 30 Hz ticks on its own clock; publishes one snapshot per tick.
        // Main thread: events + rendering at the refresh rate, interpolating between snapshots.
        struct InputEv { int type; int code, ascii; bool down; double x, y; };   // 0 key, 1 mouse, 2 release all, 3 fullscreen, 4 menu tap
        const bool touchUI = cfg.touchControls && !headless;
        std::atomic<bool> logicPaused{false};   // (Android: app in the background)
        std::atomic<uint32_t> consoleTicks{0};  // logic ticks spent paused by GenesisConsole (test injection clock)
        std::atomic<int> flushReq{0}, flushDone{0};   // save data written on the logic thread (app going to the background)
        std::vector<std::string> evalQ;         // (testing: ISAAC_TOUCH_INJECT "eval", run on the logic thread)
        std::mutex inMu, snapMu; std::vector<InputEv> inQ; std::deque<SnapPtr> snapQ;
        std::atomic<bool> logicRun{true};
        std::atomic<double> logicBase{0};
        // Phase lock (v1.5): where the display's frames start (seconds modulo the refresh period, measured by the render
        // loop) so the logic can publish each tick just before a frame starts (see the end of the logic loop). 0: off.
        std::atomic<double> framePhase{0}, framePeriod{0};
        // while graphics are preloading (during the intro cutscene), input does not reach the game
        bool inputBlocked = false;
        double blockedTouchAt = -1;   // last input attempt while preloading (shows the progress for a few seconds)
        auto pushIn = [&](InputEv ev) {
            if (inputBlocked && (ev.type == 0 || ev.type == 1 || ev.type == 4 || ev.type == 5)) { if (ev.type != 1 || ev.code == 1) blockedTouchAt = (double)SDL_GetPerformanceCounter() / SDL_GetPerformanceFrequency(); return; }
            std::lock_guard<std::mutex> lk(inMu); inQ.push_back(ev);
        };
        // Every input source (keyboard, touch, controller buttons, each stick, triggers, Switch pad...) presses game keys
        // through here: a key is down while ANY source holds it, so one source letting go can't release (or leave
        // pressed) a key another one still holds. (Before, a controller's left stick crossing into a menu - the boss
        // "VS" screen - turned its WASD into arrows and kept an arrow pressed after the fight started: Isaac
        // kept shooting by himself.)
        enum { SRC_KEYBOARD, SRC_TOUCH, SRC_BUTTONS, SRC_LSTICK, SRC_RSTICK, SRC_TRIGGERS, SRC_SWITCHPAD, SRC_SYSTEM, SRC_TRIGGER_R };
        uint16_t keyMask[256] = {};
        auto keySrc = [&](int src, int code, bool down, int ascii = 0) {
            if (code <= 0 || code >= 256) return;
            uint16_t old = keyMask[code], now = down ? (uint16_t)(old | (1u << src)) : (uint16_t)(old & ~(1u << src));
            keyMask[code] = now;
            if (!old != !now) pushIn({0, code, down ? ascii : 0, down, 0, 0});
        };
        std::function<void()> onReleaseAll;   // (controller state reset, set below)
        auto releaseAllKeys = [&]() {   // focus lost / app to the background: nothing stays pressed
            memset(keyMask, 0, sizeof keyMask);
            if (onReleaseAll) onReleaseAll();
            pushIn({2, 0, 0, false, 0, 0});
        };
        // ---- controller in menus (an outline on the selected option, see menuui.cpp) and the rebinding screen
        RebindScreen wiz;
        // ---- GenesisConsole (mod): opened during a run with its key (` by default), Back / Minus or a three-finger tap;
        // while open it takes every input (the game is paused). Functions set once the touch controls exist.
        const bool conOn = fl::genesisActive();
        SDL_Scancode conKey = SDL_SCANCODE_GRAVE;
        if (const fl::ModInfo* gm = fl::modActive("genesis")) {
            std::string k = gm->opt("key", "grave");
            if (k != "grave" && k != "`") { SDL_Scancode sc = SDL_GetScancodeFromName(k.c_str()); if (sc != SDL_SCANCODE_UNKNOWN) conKey = sc; }
        }
        bool conShown = false; double conIgnoreText = 0, conTouchToggle = 0;
        double conTouchPending = 0, mapTouchToggle = 0;   // (three fingers wait a moment: four fingers are Minimap+)
        std::deque<double> fingerDowns;                    // when the last fingers touched the screen (gestures)
        auto together = [&](int n) { return (int)fingerDowns.size() >= n && nowSec() - fingerDowns[fingerDowns.size() - n] < 0.25; };
        std::function<void()> conOpen, conClose; std::function<bool()> conCanOpen;
        int lastDevice = kIsAndroid ? DEV_TOUCH : kIsSwitch ? DEV_PAD : DEV_KEYBOARD;   // what the player used last
        bool sawKeyboard = false;
        bool navActive = false;                  // the logic thread is showing the outline
        int dpadDir = -1, stickNavDir = -1, lastNavDir = -1; double navNext = 0;   // held direction (auto-repeat)
        auto navSink = [&](int code) {
            if (wiz.isOpen()) { wiz.nav(code); return; }
            pushIn({5, code, 0, false, 0, 0}); navActive = true;
        };
        auto navOff = [&]() { if (navActive) { pushIn({5, NAV_OFF, 0, false, 0, 0}); navActive = false; } };
        struct Ctx { bool play, inGame, frog, lefty; };
        auto ctxNow = [&]() { const Snapshot* cs = renderCurrent(); return Ctx{cs && cs->gameplay, cs && cs->inGame, cs && cs->frog, cs && cs->lefty}; };
        // gameplay: the key a controller input sends (the original mapping unless the controller was rebound)
        auto padGameCode = [&](int input, int original) {
            if (!g_bind.padCustom) return original;
            int a = padActionOf(input); if (a < 0) return 0;
            Ctx c = ctxNow(); return gameCode(a, c.frog, c.lefty);
        };
        // movement / shooting direction (L U R D) -> key
        auto dirCode = [&](bool shoot, int k) {
            static const int wasd[4] = {'A', 'W', 'D', 'S'}, arrows[4] = {37, 38, 39, 40};
            static const int act[4] = {A_MOVE_LEFT, A_MOVE_UP, A_MOVE_RIGHT, A_MOVE_DOWN};
            if (!g_bind.padCustom) return shoot ? arrows[k] : wasd[k];
            Ctx c = ctxNow(); return gameCode(act[k] + (shoot ? A_SHOOT_UP - A_MOVE_UP : 0), c.frog, c.lefty);
        };
        std::string iniPath = base + "isaac.ini";
        auto setFullscreen = [&](bool full) {
            bool fs = SDL_GetWindowFlags(win) & SDL_WINDOW_FULLSCREEN_DESKTOP;
            if (fs != full) SDL_SetWindowFullscreen(win, full ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
            if (!full) { SDL_PumpEvents(); fitWindow(win, cfg.width, cfg.height); }   // (with title bar, inside the screen)
            g_isFullscreen = full;
            saveIniValue(iniPath, "fullscreen", full ? "1" : "0");   // remembered for next time
        };
        std::function<void()> logicBody = [&]() {
            SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH);   // wake up on time for each tick
            platformSetThread(1, 0x2C);                         // Switch: core 1 dedicated to logic
            setThreadRole(ROLE_LOGIC);
            double base = nowSec(); uint64_t n = 0;
            const double clockStart = base;   // getTimer(): real milliseconds since start, as Flash
            std::vector<InputEv> evs;
            uint32_t tapSpaceUntil = 0;
            bool tapOnButtons = false;    // touch: the last mouse-down happened on a screen with buttons   // touch: Space pressed by a tap on a screen without buttons, released at this tick
            while (logicRun) {
                if (flushReq.load() != flushDone.load()) { int r = flushReq.load(); sharedObjectFlushAll(); flushDone = r; }
                if (logicPaused) { SDL_Delay(20); base = nowSec() - n * dt; continue; }   // (frozen while in the background)
                double target = base + n * dt, now = nowSec();
                if (now < target) {
                    double wait = target - now;
                    if (wait > 0.003) SDL_Delay((Uint32)((wait - 0.002) * 1000)); else if (wait > 0.0003) SDL_Delay(0); else std::this_thread::yield();
                    continue;
                }
                // After a slow tick (building a new room can take hundreds of ms), the lost time is not caught up by
                // running several ticks in a row: that looked like a jump (the transition animation was skipped
                // and the game moved on unseen). Instead, continue from now at the normal pace.
                if (now - target > 2.5 * dt) { base += now - target; target = now; }
                std::vector<std::string> evals;
                { std::lock_guard<std::mutex> lk(inMu); evs.swap(inQ); evals.swap(evalQ); }
                for (auto& x : evals) debugEval(x);
                static const bool inputLog = getenv("ISAAC_INPUT_LOG") != nullptr;   // (testing)
                for (auto& ev : evs) {
                    if (inputLog) logf("[input] tick %u type %d code %d down %d at %.0f,%.0f\n", g_player.tick, ev.type, ev.code, ev.down, ev.x, ev.y);
                    if (ev.type == 0) { if (!(ev.code == 27 && ev.down && menuEscapeResumes())) keyEvent(ev.code, ev.ascii, ev.down); }
                    else if (ev.type == 1) {
                        if (ev.code == 1) tapOnButtons = touchButtonAt(ev.x, ev.y) || touchAnyButton();   // (see type 4)
                        menuMouse(ev.x, ev.y, ev.code); mouseEvent(ev.x, ev.y, ev.code);
                    }
                    else if (ev.type == 5) padNavEvent(ev.code);   // controller in menus (menuui.cpp)
                    else if (ev.type == 3) recompFullscreenChanged(ev.down);
                    else if (ev.type == 4) {
                        // a tap in a menu already went through as a mouse click; if there is nothing clickable on screen
                        // (cutscenes, title cards, death screen...) it means "continue": Space, held for a few ticks
                        // (decided by the screen when the finger went down: the tap's own click may have just changed the
                        // screen - the menu's Credits button starts the credits, and a Space there skips them at once)
                        if (!tapOnButtons && !touchButtonAt(ev.x, ev.y) && !touchAnyButton() && !tapSpaceUntil) { keyEvent(32, 32, true); tapSpaceUntil = g_player.tick + 3; }
                    }
                    else for (int k = 1; k < 256; k++) if (g_player.keys[k]) keyEvent(k, 0, false);
                }
                evs.clear();
                if (tapSpaceUntil && g_player.tick >= tapSpaceUntil) { keyEvent(32, 0, false); tapSpaceUntil = 0; }
                monkeyStep();
                // Flash's getTimer() is the real time. The game stirs the random generator at the start of each floor
                // with getTimer() % 100 draws; a frame clock (n * 33.3 ms) only gave 0, 33 or 66. Headless tests keep
                // the frame clock (deterministic, like the Ruffle harness); ISAAC_FRAMECLOCK=1 forces it here too.
                static const bool frameClock = getenv("ISAAC_FRAMECLOCK") != nullptr;
                // (while GenesisConsole pauses the game its clock stops too: the game measures some fades by getTimer()
                // between ticks, and a long pause made the screen flash bright when it went on)
                static double pausedSec = 0, lastNow = 0;
                { double now = nowSec(); if (fl::genesisPausesGame() && lastNow > 0) pausedSec += now - lastNow; lastNow = now; }
                g_player.timeMs = frameClock ? n * 1000.0 / g_frameRate : (nowSec() - clockStart - pausedSec) * 1000.0;
                renderFreeRetired();   // (snapshots the renderer is done with: freed here, not on the render thread)
                double t0 = nowSec();
                // (GenesisConsole open: the game is paused, the console keeps running)
                if (!fl::genesisPausesGame()) { runFrame(); runIntervals(); recompPatchesTick(); } else { consoleTicks++; recompMusicTick(); }
                fl::modsTick();
                { bool ig = false, gp = recompInGameplay(&ig); menuUiTick(gp, ig); }
                if (!cfg.gcAfterTick) gcCollect((nowSec() - t0) * 1000.0);
                SnapPtr snap = buildSnapshot();
                menuUiFill(*snap);
                snap->time = target; snap->logicMs = (nowSec() - t0) * 1000.0;
                snap->entities = entityCount(); snap->mouseHidden = g_mouseHidden; snap->gameplay = recompInGameplay(&snap->inGame);
                if (touchUI) snap->touchMask = recompTouchTargets(snap->touchRects);
                bool lastTick = maxFrames >= 0 && (int)g_player.tick >= maxFrames;
                snap->quit = g_player.quit || lastTick;
                snap->pubDelay = nowSec() - target;
                { std::lock_guard<std::mutex> lk(snapMu); snapQ.push_back(snap); }
                // gc_after_tick: the collection doesn't change the game state, so it can run once the tick is on its way
                // to the screen (it then doesn't count in the publish delay that sets the display margin)
                if (cfg.gcAfterTick) gcCollect(snap->logicMs);
                n++;
                // Phase lock: with 30 ticks/s on a 60 / 120 Hz display each tick is shown over exactly 2 (4) frames. If a
                // tick happens to be published just after a frame started, that frame can't use it (it shows a guess
                // instead, extrapolating the previous motion - "HELD" in performance.txt) and the tick reaches the
                // screen one frame later. The tick clock is nudged (at most 0.4 ms per tick, unnoticeable) until each
                // tick is published ~1.5 ms before a frame starts: the input it read reaches the screen as soon as
                // possible and no frame has to guess. Only the timing moves: the game itself is not affected.
                {
                    static double pubEst = 0.004;
                    pubEst = std::max(snap->pubDelay, pubEst - 0.00002);   // (the recent worst, slowly forgotten)
                    double P = framePeriod.load();
                    static const bool off = getenv("ISAAC_NOPHASELOCK") != nullptr;   // (testing)
                    if (P > 0 && !off) {
                        double e = std::remainder(base + n * dt + pubEst + 0.0015 - framePhase.load(), P);   // > 0: lands after a frame start
                        base -= std::max(-0.0004, std::min(0.0004, e * 0.15));
                    }
                }
                if (snap->quit) break;
            }
        };
        // large stack: recompiled AVM1 code can nest many calls
        g_retireSnapshots = true;
        SDL_Thread* logic = SDL_CreateThreadWithStackSize([](void* p) -> int { (*(std::function<void()>*)p)(); return 0; }, "logic", 16u << 20, &logicBody);
        if (!logic) {
            logf("could not create the logic thread: %s\n", SDL_GetError());
            showError("The Binding of Isaac", "  Could not create the game logic thread.\n  See log.txt.", win);
            return 1;
        }
        (void)logicBase;
        float padMove = -1.f;   // analog movement from a controller's left stick (0..1, -1 = none)
#ifdef __SWITCH__
        padConfigureInput(1, HidNpadStyleSet_NpadStandard);
        PadState pad; padInitializeDefault(&pad);
        int padHeld[sizeof(kPadMap) / sizeof(kPadMap[0])] = {0};   // key sent on press (to release the same one)
        std::map<int, int> keyRefs;                                  // several inputs can press the same key
        // Joy-Con / Pro Controller buttons -> controller input ids of controls.h (by position, like SDL's: Nintendo
        // B is the bottom face button, A the right one...)
        auto swInput = [](u64 btn) -> int {
            switch (btn) {
                case HidNpadButton_B: return SDL_CONTROLLER_BUTTON_A; case HidNpadButton_A: return SDL_CONTROLLER_BUTTON_B;
                case HidNpadButton_Y: return SDL_CONTROLLER_BUTTON_X; case HidNpadButton_X: return SDL_CONTROLLER_BUTTON_Y;
                case HidNpadButton_L: return SDL_CONTROLLER_BUTTON_LEFTSHOULDER; case HidNpadButton_R: return SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
                case HidNpadButton_ZL: return PAD_LT; case HidNpadButton_ZR: return PAD_RT;
                case HidNpadButton_Plus: return SDL_CONTROLLER_BUTTON_START; case HidNpadButton_Minus: return SDL_CONTROLLER_BUTTON_BACK;
                case HidNpadButton_StickL: return SDL_CONTROLLER_BUTTON_LEFTSTICK; case HidNpadButton_StickR: return SDL_CONTROLLER_BUTTON_RIGHTSTICK;
                case HidNpadButton_Up: return SDL_CONTROLLER_BUTTON_DPAD_UP; case HidNpadButton_Down: return SDL_CONTROLLER_BUTTON_DPAD_DOWN;
                case HidNpadButton_Left: return SDL_CONTROLLER_BUTTON_DPAD_LEFT; case HidNpadButton_Right: return SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
            }
            return -1;
        };
        auto padPoll = [&]() {
            padUpdate(&pad);
            u64 b = padGetButtons(&pad), bDown = padGetButtonsDown(&pad);
            // L3 + R3 (click both sticks): performance diagnostics, like F3 on PC
            const u64 both = HidNpadButton_StickL | HidNpadButton_StickR;
            if ((bDown & both) && (b & both) == both) {
                perfOn = !perfOn; perfLines.clear(); pw = PerfWin(); pwStart = nowSec();
                if (perfOn) perfLog("--- diagnostics enabled (L3+R3) ---\n");
            }
            {   // WideSaac's Minimap+: L3 alone (released without R3)
                static bool l3Solo = false;
                if (bDown & HidNpadButton_StickL) l3Solo = !(b & HidNpadButton_StickR);
                if (bDown & HidNpadButton_StickR) l3Solo = false;
                if (l3Solo && !(b & HidNpadButton_StickL)) { l3Solo = false; if (fl::wideMinimapAvailable()) fl::wideToggleMinimap(); }
            }
            if (bDown) lastDevice = DEV_PAD;
            if (conOn) {   // GenesisConsole: Minus opens it during a run; while open: A keyboard, X run, Y complete, B / Minus close
                if (fl::genesisOpen()) {
                    if (bDown && !SDL_IsScreenKeyboardShown(win)) {   // (the keyboard reads the controller itself)
                        if (bDown & (HidNpadButton_B | HidNpadButton_Minus)) { conClose(); return; }
                        if (bDown & HidNpadButton_A) SDL_StartTextInput();
                        if (bDown & HidNpadButton_X) fl::genesisKey(fl::CK_ENTER);
                        if (bDown & HidNpadButton_Y) fl::genesisKey(fl::CK_TAB);
                        if (bDown & HidNpadButton_Up) fl::genesisKey(fl::CK_UP);
                        if (bDown & HidNpadButton_Down) fl::genesisKey(fl::CK_DOWN);
                        if (bDown & HidNpadButton_Left) fl::genesisKey(fl::CK_LEFT);
                        if (bDown & HidNpadButton_Right) fl::genesisKey(fl::CK_RIGHT);
                        if (bDown & HidNpadButton_L) fl::genesisKey(fl::CK_PGUP);
                        if (bDown & HidNpadButton_R) fl::genesisKey(fl::CK_PGDN);
                    }
                    return;
                }
                if ((bDown & HidNpadButton_Minus) && conCanOpen()) { conOpen(); return; }
            }
            const Snapshot* cs = renderCurrent();
            bool inGame = cs && cs->inGame, play = cs && cs->gameplay && !wiz.isOpen();
            const size_t N = sizeof(kPadMap) / sizeof(kPadMap[0]);
            auto release = [&](size_t i) { if (!padHeld[i]) return; int k = padHeld[i]; padHeld[i] = 0; if (--keyRefs[k] == 0) keySrc(SRC_SWITCHPAD, k, false); };
            const u64 stickLDirs = HidNpadButton_StickLLeft | HidNpadButton_StickLRight | HidNpadButton_StickLUp | HidNpadButton_StickLDown;
            const u64 stickRDirs = HidNpadButton_StickRLeft | HidNpadButton_StickRRight | HidNpadButton_StickRUp | HidNpadButton_StickRDown;
            const u64 dpadAll = HidNpadButton_Up | HidNpadButton_Down | HidNpadButton_Left | HidNpadButton_Right;
            // menus / rebinding screen: D-pad and left stick move the outline, A confirms, B goes back
            if (!play) {
                int dir = -1;
                if (b & (HidNpadButton_Up | HidNpadButton_StickLUp)) dir = NAV_UP;
                else if (b & (HidNpadButton_Down | HidNpadButton_StickLDown)) dir = NAV_DOWN;
                else if (b & (HidNpadButton_Left | HidNpadButton_StickLLeft)) dir = NAV_LEFT;
                else if (b & (HidNpadButton_Right | HidNpadButton_StickLRight)) dir = NAV_RIGHT;
                dpadDir = dir;
                if (wiz.isOpen()) {
                    for (size_t i = 0; i < N; i++) release(i);
                    if (wiz.capturing()) {
                        for (u64 bit = 1; bit; bit <<= 1) if ((bDown & bit) && !(bit & (stickLDirs | stickRDirs))) { int in = swInput(bit); if (in >= 0) { if (!wiz.capturePad(in) && bit == HidNpadButton_B) wiz.nav(NAV_BACK); break; } }
                    } else {
                        if (bDown & (HidNpadButton_A | HidNpadButton_Plus)) wiz.nav(NAV_CONFIRM);
                        else if (bDown & (HidNpadButton_B | HidNpadButton_Minus)) wiz.nav(NAV_BACK);
                    }
                    return;
                }
                if (bDown & HidNpadButton_A) navSink(NAV_CONFIRM);
                else if (bDown & HidNpadButton_B) navSink(NAV_BACK);
            }
            // ZR held for 5 s in gameplay: restart (the game restarts when R is held for 1 s), unless ZR was rebound
            {
                static double zrSince = -1, rUntil = -1;
                double now = nowSec();
                bool zr = (b & HidNpadButton_ZR) && play && (!g_bind.padCustom || padActionOf(PAD_RT) < 0);
                if (!zr) zrSince = -1; else if (zrSince < 0) zrSince = now;
                if (zr && zrSince > 0 && now - zrSince >= 5.0 && rUntil < 0) { rUntil = now + 1.4; keySrc(SRC_SYSTEM, 82, true); zrSince = 1e18; }
                if (rUntil > 0 && now >= rUntil) { keySrc(SRC_SYSTEM, 82, false); rUntil = -1; }
            }
            // analog movement (joystick_acceleration): in gameplay the movement stick is read as a vector (round dead
            // zone, 8 directions) instead of libnx's digital stick "buttons", and its distance sets the speed
            const int moveStick = g_bind.padCustom ? g_bind.padMoveStick : 0;
            const bool analog = play && g_joyAccel;
            {
                static int held4[4] = {0, 0, 0, 0};
                int want[4] = {0, 0, 0, 0};
                padMove = -1.f;
                if (analog) {
                    HidAnalogStickState st = padGetStickPos(&pad, moveStick);
                    float m = std::min(1.f, std::hypot((float)st.x, (float)st.y) / 32767.f);
                    if (m > 0.2f) {
                        float ux = st.x / 32767.f / m, uy = -st.y / 32767.f / m;   // (libnx: y up)
                        want[0] = ux < -0.383f; want[2] = ux > 0.383f; want[1] = uy < -0.383f; want[3] = uy > 0.383f;
                        padMove = m;
                    }
                }
                for (int k = 0; k < 4; k++) {
                    int code = want[k] ? dirCode(false, k) : 0;
                    if (held4[k] == code) continue;
                    if (held4[k] && --keyRefs[held4[k]] == 0) keySrc(SRC_SWITCHPAD, held4[k], false);
                    if (code && keyRefs[code]++ == 0) keySrc(SRC_SWITCHPAD, code, true);
                    held4[k] = code;
                }
            }
            const u64 analogDirs = analog ? (moveStick ? stickRDirs : stickLDirs) : 0;
            auto stickDir = [](u64 btn) {   // L U R D index of a stick direction "button"
                if (btn & (HidNpadButton_StickLLeft | HidNpadButton_StickRLeft)) return 0;
                if (btn & (HidNpadButton_StickLUp | HidNpadButton_StickRUp)) return 1;
                if (btn & (HidNpadButton_StickLRight | HidNpadButton_StickRRight)) return 2;
                return 3;
            };
            auto codeFor = [&](size_t i) -> int {   // the key this button sends in the current context
                u64 btn = kPadMap[i].btn;
                if (!play) {
                    if (btn & (dpadAll | stickLDirs | HidNpadButton_A | HidNpadButton_B)) return 0;   // (the outline)
                    if (inGame && g_bind.padCustom && padActionOf(swInput(btn)) == A_PAUSE) return 'P';
                    return inGame ? kPadMap[i].pause : kPadMap[i].menu;
                }
                if (!g_bind.padCustom) return kPadMap[i].game;
                if (btn & (stickLDirs | stickRDirs)) {
                    bool isMoveStick = ((btn & stickLDirs) != 0) == (moveStick == 0);
                    return dirCode(!isMoveStick, stickDir(btn));
                }
                return padGameCode(swInput(btn), kPadMap[i].game);
            };
            for (size_t i = 0; i < N; i++) {
                bool down = (b & kPadMap[i].btn) != 0 && !(kPadMap[i].btn & analogDirs);
                int k = down ? codeFor(i) : 0;
                if (padHeld[i] && padHeld[i] != k) {
                    // released, or held while the game switched between playing and a menu: directions follow the
                    // mode (other buttons keep the key they pressed until released)
                    bool dirBtn = (kPadMap[i].btn & (dpadAll | stickLDirs | stickRDirs)) != 0;
                    if (!down || dirBtn || !k) release(i); else continue;
                }
                if (k && !padHeld[i]) {
                    padHeld[i] = k;
                    if (keyRefs[k]++ == 0) keySrc(SRC_SWITCHPAD, k, true, (k == 32 || k == 13 || k == 27) ? k : 0);
                }
            }
        };
#endif
#ifdef __ANDROID__
        platformSetThread(0, 0x2C);   // render thread on the fast cores (see platform.cpp)
#endif
        // presentation clock: one exact step per monitor refresh, corrected gradually
        double refresh = 60; { SDL_DisplayMode dm; if (SDL_GetWindowDisplayMode(win, &dm) == 0 && dm.refresh_rate > 0) refresh = dm.refresh_rate; }
        bool vsyncOn = cfg.vsync && SDL_GL_GetSwapInterval() != 0;
        double period = 1.0 / refresh, present = nowSec(), lastSwap = nowSec(), lastRefreshCheck = nowSec();
        double delta = 0.006;            // margin so the tick's snapshot is ready (adaptive)
        std::deque<double> procTimes;    // time taken by logic to publish each tick (s)
        bool quitSeen = false;
        double lastLearnSave = nowSec(), preloadT0 = nowSec();
        bool preloading = true; size_t preloadLeft = 0;
        const bool pace = vsyncOn && !cfg.gpuSync && cfg.lowLatency;   // (with gpu_sync we already wait for the whole GPU)
        // ---- touch screen (Android, Switch handheld; on PC with touch_controls = 1): joysticks, HUD taps, menus (touch.h)
        TouchControls touch({
            [&](int code, bool down) { keySrc(SRC_TOUCH, code, down, code == 32 ? 32 : 0); },
            [&](int type, double sx, double sy) { pushIn({1, type, 0, false, sx, sy}); },
            [&](double sx, double sy) { pushIn({4, 0, 0, false, sx, sy}); }});
        auto touchCtx = [&]() {
            TouchCtx c; const Snapshot* cs = renderCurrent();
            c.gameplay = cs && cs->gameplay; c.inGame = cs && cs->inGame;
            SDL_GL_GetDrawableSize(win, &c.winW, &c.winH);
            c.stageToWin = stageToWindowFor(c.winW, c.winH);
            if (cs) { c.mask = cs->touchMask; memcpy(c.rects, cs->touchRects, sizeof c.rects); }
            return c;
        };
        // (testing) ISAAC_TOUCH_INJECT=file: "<tick> down|move|up <finger> <x 0..1> <y 0..1>", "<tick> back",
        // "<tick> shot <file.bmp>", "<tick> quit": synthetic touch events for the real (threaded) game loop
        struct Inject { uint32_t tick; std::string op; long id; double x, y; std::string arg; };
        std::vector<Inject> inject; size_t injectPos = 0; std::string injectShot;
        if (const char* ip = getenv("ISAAC_TOUCH_INJECT")) {
            std::ifstream f(ip); std::string line;
            while (std::getline(f, line)) {
                std::istringstream ss(line); Inject j{0, "", 0, 0, 0, ""};
                if (!(ss >> j.tick >> j.op)) continue;
                if (j.op == "down" || j.op == "move" || j.op == "up") ss >> j.id >> j.x >> j.y; else { std::getline(ss, j.arg); j.arg = trim(j.arg); }
                inject.push_back(j);
            }
        }
        conCanOpen = [&]() { return conOn && !wiz.isOpen() && ctxNow().inGame; };
        conOpen = [&]() {
            touch.releaseAll(nowSec()); releaseAllKeys(); navOff();
            fl::genesisSetOpen(true); conShown = true; conIgnoreText = nowSec() + 0.08;   // (the opening key's own character)
            if (!kIsSwitch) SDL_StartTextInput();   // (Android: the on-screen keyboard; Switch: on A, it takes the screen)
        };
        conClose = [&]() { fl::genesisSetOpen(false); conShown = false; SDL_StopTextInput(); };
        auto backKey = [&]() {   // Android back button / gesture: pause in game, Esc in menus
            const Snapshot* cs = renderCurrent();
            int k = cs && cs->inGame ? 'P' : 27;
            keySrc(SRC_SYSTEM, k, true, k == 27 ? 27 : 0);
            SDL_AddTimer(120, [](Uint32, void* p) -> Uint32 { SDL_Event e{}; e.type = SDL_USEREVENT; e.user.code = 0x4B; e.user.data1 = p; SDL_PushEvent(&e); return 0; }, (void*)(intptr_t)k);
        };
        // Input polling. Called at the start of every frame and also repeatedly (every ~1 ms) while the main
        // thread waits for the GPU / the refresh, so a key or button press reaches the logic thread almost
        // immediately instead of waiting for the next frame (up to one refresh, ~8 ms on average). An input that
        // arrives just before a tick boundary therefore no longer slips to the following tick.
        // controller sticks (SDL): last position and the key each direction holds (L U R D)
        int lsX = 0, lsY = 0, rsX = 0, rsY = 0, mvHeld[4] = {0, 0, 0, 0}, shHeld[4] = {0, 0, 0, 0};
        bool lsPlay = false;                     // mode the sticks' keys were chosen for
        std::map<int, int> padBtnHeld;           // controller button -> key sent on press (released with it)
        bool navDpad[4] = {false, false, false, false};   // D-pad held in a menu (U D L R)
        int trigSent[2] = {0, 0}; bool trigOn[2] = {false, false};
        std::map<int, int> kbSent;               // keyboard scancode -> key sent on press
        int lastRebindReq = 0;
        // In gameplay the movement stick moves (WASD, analog with joystick_acceleration) and the other one shoots
        // (arrows); in menus the left stick drives the outline. Re-evaluated when the game switches between the two,
        // not only when a stick moves (a held stick must not keep the other mode's keys).
        auto updateSticks = [&](bool play) {
            const int dead = 12000;
            bool swap = g_bind.padCustom && g_bind.padMoveStick;
            int mx = swap ? rsX : lsX, my = swap ? rsY : lsY, sx = swap ? lsX : rsX, sy = swap ? lsY : rsY;
            int mv[4] = {0, 0, 0, 0}, sh[4] = {0, 0, 0, 0};
            lsPlay = play; padMove = -1.f; stickNavDir = -1;
            if (play && !wiz.isOpen()) {
                int want[4] = {0, 0, 0, 0};
                if (g_joyAccel) {   // analog movement: round dead zone, 8 directions, speed from the distance (moveCurve)
                    float m = std::min(1.f, std::hypot((float)mx, (float)my) / 32767.f);
                    if (m > 0.2f) {
                        float ux = mx / 32767.f / m, uy = my / 32767.f / m;
                        want[0] = ux < -0.383f; want[2] = ux > 0.383f; want[1] = uy < -0.383f; want[3] = uy > 0.383f;
                        padMove = m;
                    }
                } else { want[0] = mx < -dead; want[2] = mx > dead; want[1] = my < -dead; want[3] = my > dead; }
                int shw[4] = {sx < -dead, sy < -dead, sx > dead, sy > dead};
                for (int k = 0; k < 4; k++) { mv[k] = want[k] ? dirCode(false, k) : 0; sh[k] = shw[k] ? dirCode(true, k) : 0; }
            } else if (std::max(std::abs(lsX), std::abs(lsY)) > 16000) {
                stickNavDir = std::abs(lsX) > std::abs(lsY) ? (lsX < 0 ? NAV_LEFT : NAV_RIGHT) : (lsY < 0 ? NAV_UP : NAV_DOWN);
            }
            for (int k = 0; k < 4; k++) {
                if (mvHeld[k] != mv[k]) { if (mvHeld[k]) keySrc(SRC_LSTICK, mvHeld[k], false); if (mv[k]) keySrc(SRC_LSTICK, mv[k], true); mvHeld[k] = mv[k]; }
                if (shHeld[k] != sh[k]) { if (shHeld[k]) keySrc(SRC_RSTICK, shHeld[k], false); if (sh[k]) keySrc(SRC_RSTICK, sh[k], true); shHeld[k] = sh[k]; }
            }
        };
        onReleaseAll = [&]() {   // (the keys are already released: forget what the controller held)
            for (int k = 0; k < 4; k++) { mvHeld[k] = shHeld[k] = 0; navDpad[k] = false; }
            padBtnHeld.clear(); padMove = -1.f; dpadDir = -1; trigSent[0] = trigSent[1] = 0; kbSent.clear();
        };
        auto anyController = [&]() { for (int i = 0; i < SDL_NumJoysticks(); i++) if (SDL_IsGameController(i)) return true; return false; };
        auto openRebind = [&]() {
            bool avail[DEV_COUNT];
            avail[DEV_KEYBOARD] = !kIsSwitch && (!kIsAndroid || sawKeyboard);
            avail[DEV_PAD] = kIsSwitch || anyController();
            avail[DEV_TOUCH] = touchUI;
            int style = kIsSwitch ? 2 : 0;
            for (int i = 0; i < SDL_NumJoysticks() && !kIsSwitch; i++) if (SDL_IsGameController(i)) {
                SDL_GameControllerType t = SDL_GameControllerTypeForIndex(i);
                style = (t == SDL_CONTROLLER_TYPE_PS3 || t == SDL_CONTROLLER_TYPE_PS4 || t == SDL_CONTROLLER_TYPE_PS5) ? 1
                      : (t == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO) ? 2 : 0;
                break;
            }
            touch.releaseAll(nowSec()); releaseAllKeys();
            wiz.open(avail, lastDevice, style);
        };
        bool wizHand = false, snapHand = false, handShown = false;   // hand cursor (rebinding screen rows / game buttons)
        auto stagePoint = [&](double px, double py, double& sx, double& sy) {   // window pixels -> stage
            int dw, dh; SDL_GL_GetDrawableSize(win, &dw, &dh);
            Mat inv; stageToWindowFor(dw, dh).invert(inv); inv.apply(px, py, sx, sy);
        };
        auto pollInput = [&]() {
            if (conShown && !fl::genesisOpen()) { conShown = false; SDL_StopTextInput(); }   // (closed by a command / the run ended)
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                switch (e.type) {
                    case SDL_TEXTINPUT:
                        if (conOn && fl::genesisOpen() && nowSec() >= conIgnoreText) {
                            fl::genesisText(e.text.text);
#ifdef __SWITCH__
                            fl::genesisKey(fl::CK_ENTER);   // (the Switch keyboard sends the whole line when it closes)
#endif
                        }
                        break;
                    case SDL_MOUSEWHEEL:
                        if (conOn && fl::genesisOpen() && e.wheel.y) fl::genesisKey(e.wheel.y > 0 ? fl::CK_PGUP : fl::CK_PGDN);
                        break;
                    case SDL_QUIT: running = false; break;
                    case SDL_WINDOWEVENT:
                        if (e.window.event == SDL_WINDOWEVENT_FOCUS_LOST) { touch.releaseAll(nowSec()); releaseAllKeys(); }   // avoid "stuck" keys
                        break;
                    case SDL_USEREVENT:
                        if (e.user.code == 0x4B) { int k = (int)(intptr_t)e.user.data1; keySrc(SRC_SYSTEM, k, false); }   // (back key release)
                        break;
                    case SDL_APP_WILLENTERBACKGROUND: {   // Android: phone call, home button...: pause the run and freeze logic
                        const Snapshot* cs = renderCurrent();
                        if (cs && cs->gameplay) { pushIn({0, 'P', 0, true, 0, 0}); pushIn({0, 'P', 0, false, 0, 0}); }
                        touch.releaseAll(nowSec()); releaseAllKeys();
                        int r = ++flushReq;   // the save is written by the logic thread (it owns the game state)
                        for (int i = 0; i < 50 && flushDone.load() != r; i++) SDL_Delay(5);
                        break;
                    }
                    case SDL_APP_DIDENTERBACKGROUND: logicPaused = true; break;
                    case SDL_APP_WILLENTERFOREGROUND: case SDL_APP_DIDENTERFOREGROUND: logicPaused = false; break;
                    case SDL_FINGERDOWN: case SDL_FINGERMOTION: case SDL_FINGERUP:
                        // Multi-finger gestures (GenesisConsole: 3, Minimap+: 4) count only when all those fingers went down
                        // together (within 0.25 s): a thumb kept on a joystick plus a tap with two more is not a gesture
                        if (e.type == SDL_FINGERDOWN) { fingerDowns.push_back(nowSec()); if (fingerDowns.size() > 8) fingerDowns.pop_front(); }
                        if (e.type == SDL_FINGERDOWN && fl::wideMinimapAvailable()) {   // WideSaac: four fingers show / hide Minimap+
                            int nf = SDL_GetNumTouchFingers(e.tfinger.touchId);
                            if (nf >= 4 && together(4) && nowSec() > mapTouchToggle) {
                                mapTouchToggle = nowSec() + 0.8; conTouchPending = 0;
                                touch.releaseAll(nowSec()); fl::wideToggleMinimap();
                                break;
                            }
                            if (nf == 3 && together(3) && conOn && !fl::genesisOpen() && nowSec() > conTouchToggle) {   // (maybe a fourth is coming)
                                conTouchToggle = nowSec() + 0.8; conTouchPending = nowSec() + 0.2;
                                break;
                            }
                        }
                        if (conOn) {   // GenesisConsole: three fingers open / close it; a tap while open shows the keyboard
                            if (e.type == SDL_FINGERDOWN && SDL_GetNumTouchFingers(e.tfinger.touchId) >= 3 && together(3) && nowSec() > conTouchToggle) {
                                conTouchToggle = nowSec() + 0.8;
                                if (fl::genesisOpen()) conClose(); else if (conCanOpen()) conOpen();
                                break;
                            }
                            if (fl::genesisOpen()) {
                                if (e.type == SDL_FINGERUP && SDL_GetNumTouchFingers(e.tfinger.touchId) == 0 && nowSec() > conTouchToggle) SDL_StartTextInput();
                                break;
                            }
                        }
                        if (e.type == SDL_FINGERDOWN) { lastDevice = touchUI ? DEV_TOUCH : lastDevice; navOff(); }
                        if (wiz.isOpen()) {   // rebinding screen: taps on its rows
                            if (e.type == SDL_FINGERMOTION) break;
                            int dw, dh; SDL_GL_GetDrawableSize(win, &dw, &dh); double sx, sy;
                            stagePoint(e.tfinger.x * dw, e.tfinger.y * dh, sx, sy);
                            wiz.pointer(uiHitRow(sx, sy), e.type == SDL_FINGERDOWN ? 1 : 2);
                            break;
                        }
                        if (touchUI) {
                            TouchCtx c = touchCtx(); double px = e.tfinger.x * c.winW, py = e.tfinger.y * c.winH;
                            if (e.type == SDL_FINGERDOWN) touch.down((int64_t)e.tfinger.fingerId, px, py, c, nowSec());
                            else if (e.type == SDL_FINGERMOTION) touch.move((int64_t)e.tfinger.fingerId, px, py, c, nowSec());
                            else touch.up((int64_t)e.tfinger.fingerId, px, py, c, nowSec());
                            break;
                        }
#ifdef __SWITCH__
                        goto switchTouch;
#else
                        break;
#endif
                    case SDL_KEYDOWN: case SDL_KEYUP: {
                        bool down = e.type == SDL_KEYDOWN; SDL_Keycode sym = e.key.keysym.sym;
                        if (sym == SDLK_AC_BACK && conOn && fl::genesisOpen()) { if (down && !e.key.repeat) conClose(); break; }
                        if (sym == SDLK_AC_BACK) { if (down && !e.key.repeat) { if (wiz.isOpen()) wiz.nav(NAV_BACK); else backKey(); } break; }
                        if (down && !e.key.repeat && (sym == SDLK_F11 || (sym == SDLK_RETURN && (e.key.keysym.mod & KMOD_ALT)))) {
                            bool fs = SDL_GetWindowFlags(win) & SDL_WINDOW_FULLSCREEN_DESKTOP;
                            setFullscreen(!fs); pushIn({3, 0, 0, !fs, 0, 0}); break;
                        }
                        if (down && !e.key.repeat && sym == SDLK_F3) { perfOn = !perfOn; perfLines.clear(); pw = PerfWin(); pwStart = nowSec(); if (perfOn) perfLog("--- diagnostics enabled (F3) ---\n"); break; }
                        if (down && sym == SDLK_F12) { static int sn = 0; char nm[64]; snprintf(nm, sizeof nm, "screenshot_%d.bmp", ++sn); renderScreenshot((pref + nm).c_str()); break; }
                        if (down && !e.key.repeat) { lastDevice = DEV_KEYBOARD; sawKeyboard = true; navOff(); }
                        if (conOn && fl::genesisOpen()) {   // GenesisConsole: every key goes to it
                            if (!down) break;
                            bool ctrl = e.key.keysym.mod & KMOD_CTRL;
                            if (e.key.keysym.scancode == conKey || sym == SDLK_ESCAPE) { if (!e.key.repeat) conClose(); break; }
                            if (ctrl && sym == SDLK_v) { if (char* t = SDL_GetClipboardText()) { fl::genesisText(t); SDL_free(t); } break; }
                            if (ctrl && (sym == SDLK_u || sym == SDLK_c)) { fl::genesisKey(fl::CK_CLEARLINE); break; }
                            int ck = -1;
                            switch (sym) {
                                case SDLK_RETURN: case SDLK_KP_ENTER: ck = fl::CK_ENTER; break;
                                case SDLK_BACKSPACE: ck = fl::CK_BACKSPACE; break; case SDLK_DELETE: ck = fl::CK_DELETE; break;
                                case SDLK_LEFT: ck = fl::CK_LEFT; break; case SDLK_RIGHT: ck = fl::CK_RIGHT; break;
                                case SDLK_UP: ck = fl::CK_UP; break; case SDLK_DOWN: ck = fl::CK_DOWN; break;
                                case SDLK_HOME: ck = fl::CK_HOME; break; case SDLK_END: ck = fl::CK_END; break;
                                case SDLK_TAB: ck = fl::CK_TAB; break;
                                case SDLK_PAGEUP: ck = fl::CK_PGUP; break; case SDLK_PAGEDOWN: ck = fl::CK_PGDN; break;
                                default: break;
                            }
                            if (ck >= 0) fl::genesisKey(ck);
                            break;
                        }
                        if (conOn && down && !e.key.repeat && e.key.keysym.scancode == conKey && conCanOpen()) { conOpen(); break; }
                        if (down && !e.key.repeat && sym == SDLK_m && fl::wideMinimapAvailable() && kbActionOf(SDLK_m) < 0) { fl::wideToggleMinimap(); break; }   // (WideSaac's Minimap+)
                        if (wiz.isOpen()) {
                            if (!down) break;
                            if (wiz.capturing()) { if (!e.key.repeat && !wiz.captureKey(sym) && sym == SDLK_ESCAPE) wiz.nav(NAV_BACK); break; }
                            switch (sym) {
                                case SDLK_UP: case SDLK_w: wiz.nav(NAV_UP); break;
                                case SDLK_DOWN: case SDLK_s: wiz.nav(NAV_DOWN); break;
                                case SDLK_LEFT: case SDLK_a: wiz.nav(NAV_LEFT); break;
                                case SDLK_RIGHT: case SDLK_d: wiz.nav(NAV_RIGHT); break;
                                case SDLK_RETURN: case SDLK_KP_ENTER: case SDLK_SPACE: if (!e.key.repeat) wiz.nav(NAV_CONFIRM); break;
                                case SDLK_ESCAPE: case SDLK_BACKSPACE: if (!e.key.repeat) wiz.nav(NAV_BACK); break;
                                default: break;
                            }
                            break;
                        }
                        int sc = e.key.keysym.scancode;
                        if (e.key.repeat) {   // (auto-repeat: as before)
                            if (!down) break;
                            auto it = kbSent.find(sc); int c = it == kbSent.end() ? 0 : it->second;
                            if (c && (keyMask[c] & (1u << SRC_KEYBOARD))) pushIn({0, c, asciiOf(sym, e.key.keysym.mod & KMOD_SHIFT), true, 0, 0});
                            break;
                        }
                        if (!down) { auto it = kbSent.find(sc); if (it != kbSent.end()) { if (it->second) keySrc(SRC_KEYBOARD, it->second, false); kbSent.erase(it); } break; }
                        int c = sdlToFlash(sym);
                        if (!c) break;
                        Ctx cx = ctxNow();
                        if (g_bind.kbCustom && (cx.play || cx.inGame)) {   // rebound keys (see controls.h)
                            int a = kbActionOf(sym);
                            if (a >= 0 && (cx.play || a == A_PAUSE)) c = gameCode(a, cx.frog, cx.lefty);
                            else if (cx.play && isGameActionCode(c)) c = 0;   // an original key that now does nothing
                        }
                        if (kbSent.count(sc)) break;
                        kbSent[sc] = c;
                        if (c) keySrc(SRC_KEYBOARD, c, true, asciiOf(sym, e.key.keysym.mod & KMOD_SHIFT));
                        break;
                    }
                    case SDL_MOUSEMOTION: case SDL_MOUSEBUTTONDOWN: case SDL_MOUSEBUTTONUP: {
                        if (conOn && fl::genesisOpen()) break;
                        if (e.type == SDL_MOUSEMOTION ? (e.motion.xrel || e.motion.yrel) : true) { navOff(); if (e.type == SDL_MOUSEBUTTONDOWN && !kIsAndroid && !kIsSwitch) lastDevice = DEV_KEYBOARD; }
                        int mx, my; SDL_GetMouseState(&mx, &my);
                        int ww, wh, dw, dh; SDL_GetWindowSize(win, &ww, &wh); SDL_GL_GetDrawableSize(win, &dw, &dh);
                        double px = mx * (double)dw / ww, py = my * (double)dh / wh;
                        double sx, sy; stagePoint(px, py, sx, sy);
                        wizHand = uiHitRow(sx, sy) >= 0;
                        if (wiz.isOpen()) { if (e.type != SDL_MOUSEMOTION) wiz.pointer(uiHitRow(sx, sy), e.type == SDL_MOUSEBUTTONDOWN ? 1 : 2); break; }
                        pushIn({1, e.type == SDL_MOUSEMOTION ? 0 : e.type == SDL_MOUSEBUTTONDOWN ? 1 : 2, 0, false, sx, sy});
                        break;
                    }
#ifdef __SWITCH__
                    // touch screen (handheld mode) with touch_controls = 0: in menus and in the pause screen, a tap = a
                    // mouse click (the original game's buttons respond to the mouse). Does nothing during gameplay.
                    switchTouch: {
                        static SDL_FingerID active = -1;
                        const Snapshot* cs = renderCurrent();
                        bool play = cs && cs->gameplay;
                        if (e.type == SDL_FINGERDOWN) { if (active != -1 || play) break; active = e.tfinger.fingerId; }
                        else if (e.tfinger.fingerId != active) break;
                        int dw, dh; SDL_GL_GetDrawableSize(win, &dw, &dh);
                        double sx, sy; stagePoint(e.tfinger.x * dw, e.tfinger.y * dh, sx, sy);
                        pushIn({1, 0, 0, false, sx, sy});                                   // move (for the "over" state)
                        if (e.type == SDL_FINGERDOWN) pushIn({1, 1, 0, false, sx, sy});
                        if (e.type == SDL_FINGERUP) { pushIn({1, 2, 0, false, sx, sy}); active = -1; }
                        break;
                    }
#endif
                    case SDL_CONTROLLERDEVICEADDED: SDL_GameControllerOpen(e.cdevice.which); break;
                    case SDL_AUDIODEVICEADDED: case SDL_AUDIODEVICEREMOVED: if (!e.adevice.iscapture) audioDevicesChanged(nowSec()); break;
                    case SDL_CONTROLLERBUTTONDOWN: case SDL_CONTROLLERBUTTONUP: {
                        // same scheme as on Switch: the key depends on whether we are playing, paused or in a menu.
                        // In menus and on the pause screen the D-pad, A and B drive the outline (menuui.cpp).
                        // The key sent on press is the one released.
                        struct Map { int game, pause, menu; };
                        static const std::map<int, Map> kMap = {
                            {SDL_CONTROLLER_BUTTON_A, {40, 32, 32}}, {SDL_CONTROLLER_BUTTON_B, {39, 27, 27}},
                            {SDL_CONTROLLER_BUTTON_X, {37, 37, 37}}, {SDL_CONTROLLER_BUTTON_Y, {38, 38, 38}},
                            {SDL_CONTROLLER_BUTTON_LEFTSHOULDER, {'E', 0, 0}}, {SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, {32, 0, 0}},
                            {SDL_CONTROLLER_BUTTON_LEFTSTICK, {'P', 'P', 0}}, {SDL_CONTROLLER_BUTTON_RIGHTSTICK, {'Q', 0, 0}},
                            {SDL_CONTROLLER_BUTTON_START, {'P', 'P', 13}}, {SDL_CONTROLLER_BUTTON_BACK, {'P', 'P', 27}},
                            {SDL_CONTROLLER_BUTTON_DPAD_UP, {'W', 38, 38}}, {SDL_CONTROLLER_BUTTON_DPAD_DOWN, {'S', 40, 40}},
                            {SDL_CONTROLLER_BUTTON_DPAD_LEFT, {'A', 37, 37}}, {SDL_CONTROLLER_BUTTON_DPAD_RIGHT, {'D', 39, 39}},
                        };
                        bool d = e.type == SDL_CONTROLLERBUTTONDOWN; int btn = e.cbutton.button;
                        int dpad = btn == SDL_CONTROLLER_BUTTON_DPAD_UP ? 0 : btn == SDL_CONTROLLER_BUTTON_DPAD_DOWN ? 1 : btn == SDL_CONTROLLER_BUTTON_DPAD_LEFT ? 2 : btn == SDL_CONTROLLER_BUTTON_DPAD_RIGHT ? 3 : -1;
                        if (d) lastDevice = DEV_PAD;
                        if (conOn && fl::genesisOpen()) {   // GenesisConsole: A keyboard, X run, Y complete, D-pad, LB / RB scroll, B close
                            if (!d) break;
                            switch (btn) {
                                case SDL_CONTROLLER_BUTTON_A: SDL_StartTextInput(); break;
                                case SDL_CONTROLLER_BUTTON_X: fl::genesisKey(fl::CK_ENTER); break;
                                case SDL_CONTROLLER_BUTTON_Y: fl::genesisKey(fl::CK_TAB); break;
                                case SDL_CONTROLLER_BUTTON_DPAD_UP: fl::genesisKey(fl::CK_UP); break;
                                case SDL_CONTROLLER_BUTTON_DPAD_DOWN: fl::genesisKey(fl::CK_DOWN); break;
                                case SDL_CONTROLLER_BUTTON_DPAD_LEFT: fl::genesisKey(fl::CK_LEFT); break;
                                case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: fl::genesisKey(fl::CK_RIGHT); break;
                                case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: fl::genesisKey(fl::CK_PGUP); break;
                                case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: fl::genesisKey(fl::CK_PGDN); break;
                                case SDL_CONTROLLER_BUTTON_B: case SDL_CONTROLLER_BUTTON_BACK: conClose(); break;
                                default: break;
                            }
                            break;
                        }
                        if (conOn && d && btn == SDL_CONTROLLER_BUTTON_BACK && conCanOpen()) { conOpen(); break; }
                        if (!d) {   // release: whatever the press sent
                            if (dpad >= 0) { navDpad[dpad] = false; if (dpadDir == dpad) dpadDir = -1; }
                            auto h = padBtnHeld.find(btn); if (h == padBtnHeld.end()) break;
                            int code = h->second; padBtnHeld.erase(h);
                            bool still = false; for (auto& o : padBtnHeld) still |= o.second == code;
                            if (!still) keySrc(SRC_BUTTONS, code, false);
                            break;
                        }
                        if (wiz.isOpen()) {
                            if (wiz.capturing()) { if (!wiz.capturePad(btn) && btn == SDL_CONTROLLER_BUTTON_B) wiz.nav(NAV_BACK); break; }   // (B cancels a keyboard capture)
                            if (dpad >= 0) { navDpad[dpad] = true; dpadDir = dpad; }
                            else if (btn == SDL_CONTROLLER_BUTTON_A || btn == SDL_CONTROLLER_BUTTON_START) wiz.nav(NAV_CONFIRM);
                            else if (btn == SDL_CONTROLLER_BUTTON_B || btn == SDL_CONTROLLER_BUTTON_BACK) wiz.nav(NAV_BACK);
                            break;
                        }
                        Ctx cx = ctxNow();
                        if (!cx.play) {
                            if (dpad >= 0) { navDpad[dpad] = true; dpadDir = dpad; break; }
                            if (btn == SDL_CONTROLLER_BUTTON_A) { navSink(NAV_CONFIRM); break; }
                            if (btn == SDL_CONTROLLER_BUTTON_B) { navSink(NAV_BACK); break; }
                        }
                        if (btn == SDL_CONTROLLER_BUTTON_LEFTSTICK && cx.play && fl::wideMinimapAvailable() &&
                            (!g_bind.padCustom || padActionOf(btn) < 0 || padActionOf(btn) == A_PAUSE)) { fl::wideToggleMinimap(); break; }   // (Minimap+)
                        auto it = kMap.find(btn);
                        int code;
                        if (cx.play) code = padGameCode(btn, it == kMap.end() ? 0 : it->second.game);
                        else if (cx.inGame) code = g_bind.padCustom && padActionOf(btn) == A_PAUSE ? 'P' : it == kMap.end() ? 0 : it->second.pause;
                        else code = it == kMap.end() ? 0 : it->second.menu;
                        if (!code || padBtnHeld.count(btn)) break;
                        padBtnHeld[btn] = code;
                        keySrc(SRC_BUTTONS, code, true, (code == 32 || code == 13 || code == 27) ? code : 0);
                        break;
                    }
                    case SDL_CONTROLLERAXISMOTION: {
                        int v = e.caxis.value;
                        bool play = ctxNow().play;
                        switch (e.caxis.axis) {
                            case SDL_CONTROLLER_AXIS_LEFTX: lsX = v; updateSticks(play); break;
                            case SDL_CONTROLLER_AXIS_LEFTY: lsY = v; updateSticks(play); break;
                            case SDL_CONTROLLER_AXIS_RIGHTX: rsX = v; updateSticks(play); break;
                            case SDL_CONTROLLER_AXIS_RIGHTY: rsY = v; updateSticks(play); break;
                            case SDL_CONTROLLER_AXIS_TRIGGERLEFT: case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: {
                                int t = e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT ? 0 : 1;
                                bool on = v > 8000, was = trigOn[t]; trigOn[t] = on;
                                if (on && !was) lastDevice = DEV_PAD;
                                if (wiz.isOpen()) { if (on && !was && wiz.capturing()) wiz.capturePad(t ? PAD_RT : PAD_LT); break; }
                                int code = on && play ? padGameCode(t ? PAD_RT : PAD_LT, t ? 32 : 'E') : 0;
                                if (code != trigSent[t]) {
                                    int src = t ? SRC_TRIGGER_R : SRC_TRIGGERS;
                                    if (trigSent[t]) keySrc(src, trigSent[t], false);
                                    if (code) keySrc(src, code, true, code == 32 ? 32 : 0);
                                    trigSent[t] = code;
                                }
                                break;
                            }
                        }
                        break;
                    }
                    default: break;
                }
            }
            {   // the game switched between playing and a menu (boss "VS" screen, pause, death...): held sticks and
                // D-pad directions change meaning (movement keys <-> the outline in menus)
                Ctx cx = ctxNow(); bool play = cx.play && !wiz.isOpen();
                if (play != lsPlay) {
                    updateSticks(play);
                    static const int kDpadBtn[4] = {SDL_CONTROLLER_BUTTON_DPAD_UP, SDL_CONTROLLER_BUTTON_DPAD_DOWN, SDL_CONTROLLER_BUTTON_DPAD_LEFT, SDL_CONTROLLER_BUTTON_DPAD_RIGHT};
                    static const int kDpadKey[4] = {'W', 'S', 'A', 'D'};
                    for (int k = 0; k < 4; k++) {
                        int btn = kDpadBtn[k];
                        auto h = padBtnHeld.find(btn);
                        if (!play && h != padBtnHeld.end()) {   // held while moving: now it moves the outline
                            int code = h->second; padBtnHeld.erase(h);
                            bool still = false; for (auto& o : padBtnHeld) still |= o.second == code;
                            if (!still) keySrc(SRC_BUTTONS, code, false);
                            navDpad[k] = true; dpadDir = k;
                        } else if (play && navDpad[k] && h == padBtnHeld.end()) {   // held in the menu: now it moves
                            navDpad[k] = false; if (dpadDir == k) dpadDir = -1;
                            int code = padGameCode(btn, kDpadKey[k]);
                            if (code) { padBtnHeld[btn] = code; keySrc(SRC_BUTTONS, code, true); }
                        }
                    }
                }
                // outline: auto-repeat of a held direction
                int dir = play ? -1 : dpadDir >= 0 ? dpadDir : stickNavDir;
                double now = nowSec();
                if (dir != lastNavDir) { lastNavDir = dir; if (dir >= 0) { navSink(dir); navNext = now + 0.38; } }
                else if (dir >= 0 && now >= navNext) { navSink(dir); navNext = now + 0.11; }
            }
#ifdef __SWITCH__
            padPoll();
            g_fullscreenRequest = -1;   // (always fullscreen)
#elif defined(__ANDROID__)
            g_fullscreenRequest = -1;   // (always fullscreen)
#else
            { int rq = g_fullscreenRequest.exchange(-1); if (rq >= 0) setFullscreen(rq == 1); }   // menu option or F key
#endif
        };
        while (running) {
            double paceMs = 0;
            if (pace) {   // at most one frame queued (less input-to-image latency); input keeps being read meanwhile
                double w0 = nowSec();
                while (!renderPaceWait(1000000ull)) { pollInput(); if (nowSec() - w0 > 0.1) { renderPaceWait(~0ull); break; } }
                paceMs = (nowSec() - w0) * 1000.0;
            }   // at most one frame queued: less input-to-image latency
            if (preloading) {
                size_t left = renderPreloadPending(); preloadLeft = left;
                inputBlocked = left > 0;
                if (!left) { preloading = false; logf("preload finished in %.1f s (atlas: %d of %d pages)\n", nowSec() - preloadT0, renderAtlasPages(), std::max(4, g_renderCfg.cacheMB / 16)); }
            }
            if (nowSec() - lastLearnSave > 20) { lastLearnSave = nowSec(); saveLearned(preloadPath); }   // new graphics → preload.txt
            pollInput();
            while (injectPos < inject.size() && inject[injectPos].tick <= g_player.tick + consoleTicks.load()) {   // (testing)
                Inject& j = inject[injectPos++];
                if (j.op == "down" || j.op == "move" || j.op == "up") {
                    SDL_Event e{}; e.type = j.op == "down" ? SDL_FINGERDOWN : j.op == "move" ? SDL_FINGERMOTION : SDL_FINGERUP;
                    e.tfinger.fingerId = j.id; e.tfinger.x = (float)j.x; e.tfinger.y = (float)j.y; SDL_PushEvent(&e);
                } else if (j.op == "back") { SDL_Event e{}; e.type = SDL_KEYDOWN; e.key.keysym.sym = SDLK_AC_BACK; SDL_PushEvent(&e); }
                else if (j.op == "axis" || j.op == "button") {   // controller: "axis <SDL axis> <value>", "button <SDL button> <0|1>"
                    std::istringstream as(j.arg); int a = 0, v = 0; as >> a >> v;
                    SDL_Event e{};
                    if (j.op == "axis") { e.type = SDL_CONTROLLERAXISMOTION; e.caxis.axis = (Uint8)a; e.caxis.value = (Sint16)v; }
                    else { e.type = v ? SDL_CONTROLLERBUTTONDOWN : SDL_CONTROLLERBUTTONUP; e.cbutton.button = (Uint8)a; }
                    SDL_PushEvent(&e);
                }
                else if (j.op == "key" || j.op == "kdown" || j.op == "kup") {   // "key|kdown|kup <SDL key name>" (key: pressed and released)
                    SDL_Event e{}; e.key.keysym.sym = SDL_GetKeyFromName(j.arg.c_str()); e.key.keysym.scancode = SDL_GetScancodeFromName(j.arg.c_str());
                    if (j.op != "kup") { e.type = SDL_KEYDOWN; SDL_PushEvent(&e); }
                    if (j.op != "kdown") { e.type = SDL_KEYUP; SDL_PushEvent(&e); }
                }
                else if (j.op == "text") { SDL_Event e{}; e.type = SDL_TEXTINPUT; snprintf(e.text.text, sizeof e.text.text, "%s", j.arg.c_str()); SDL_PushEvent(&e); }
                else if (j.op == "shot") injectShot = j.arg;
                else if (j.op == "eval") { std::lock_guard<std::mutex> lk(inMu); evalQ.push_back(j.arg); }
                else if (j.op == "quit") running = false;
            }
            if (touchUI) touch.update(touchCtx(), nowSec());
            if (conTouchPending > 0 && nowSec() >= conTouchPending) { conTouchPending = 0; if (conCanOpen()) conOpen(); }   // (three fingers, no fourth)
            {   // rebinding screen: opened from the game's Control options page (menuui.cpp), saved when closed
                const Snapshot* cs = renderCurrent();
                if (cs && cs->rebindRequests != lastRebindReq) { lastRebindReq = cs->rebindRequests; if (!wiz.isOpen()) openRebind(); }
                wiz.tick(nowSec());
                if (wiz.takeSaveRequest()) {
                    bindingsSave([&](const std::string& k, const std::string& v) { saveIniValue(iniPath, k, v); });
                    TouchControls::setLayout(g_bind.touchSwap, g_bind.touchSize, g_bind.touchType);
                    releaseAllKeys();
                    logf("controls saved (keyboard %s, controller %s)\n", g_bind.kbCustom ? "custom" : "default", g_bind.padCustom ? "custom" : "default");
                }
            }
            {   // movement factor for the logic thread (joystick_acceleration): touch stick, else controller stick, else full
                float tm = touchUI ? touch.moveAmount() : -1.f;
                float k = !g_joyAccel ? 1.f : tm >= 0 ? moveCurve(tm, TouchControls::deadZone()) : padMove >= 0 ? moveCurve(padMove, 0.2f) : 1.f;
                g_moveScale = k > 0 ? k : 1.f;
            }
            double fstart = nowSec();
            if (fstart - lastRefreshCheck > 1.0) {   // the monitor may change (window moved)
                lastRefreshCheck = fstart;
                SDL_DisplayMode dm; if (SDL_GetWindowDisplayMode(win, &dm) == 0 && dm.refresh_rate > 0 && dm.refresh_rate != (int)refresh) { refresh = dm.refresh_rate; period = 1.0 / refresh; }
            }
            // moment at which this frame will be shown
            if (vsyncOn) {
                double expect = present + period, measured = lastSwap + period;
                if (std::fabs(measured - expect) > period * 0.5) present = measured;   // dropped frame or drift: resync
                else present = expect + (measured - expect) * 0.05;
            } else present = fstart;
            {   // phase lock (logic loop): where frames start, as a smoothed circular mean of fstart modulo the period
                static double pc = 0, ps = 0;
                double ratio = dt / period;
                if (vsyncOn && !preloading && std::fabs(ratio - std::round(ratio)) < 0.03) {
                    double ang = 2 * M_PI * std::fmod(fstart, period) / period;
                    pc = pc * 0.9 + std::cos(ang) * 0.1; ps = ps * 0.9 + std::sin(ang) * 0.1;
                    double ph = std::atan2(ps, pc) / (2 * M_PI) * period; if (ph < 0) ph += period;
                    framePhase = ph; framePeriod = period;
                } else framePeriod = 0;
            }
            // adaptive margin: the worst recent logic publish time (+2 ms)
            // (90th percentile: an isolated heavy tick, such as loading a room, doesn't raise the margin)
            double worst = 0;
            if (!procTimes.empty()) { std::vector<double> v(procTimes.begin(), procTimes.end()); std::sort(v.begin(), v.end()); worst = v[std::min(v.size() - 1, v.size() * 9 / 10)]; }
            double want = std::min(0.030, std::max(0.004, worst + 0.002));
            if (cfg.maxMarginMs > 0) want = std::min(want, std::max(0.004, cfg.maxMarginMs / 1000.0));   // max_margin (isaac.ini)
            if (want > delta) delta = std::min(want, delta + 0.0005); else delta = std::max(want, delta - 0.0001);
            double simT = present - delta + cfg.interpLead * dt;   // interp_lead: optional extrapolation (default 0)
            // adopt the snapshots that are due (in order: each may bring new pixels)
            int adopted = 0; double frameLogic = 0;
            g_rstats = RenderStats();
            for (;;) {
                SnapPtr sp;
                {
                    std::lock_guard<std::mutex> lk(snapMu);
                    if (snapQ.empty()) break;
                    if (renderCurrent() && snapQ.front()->time > simT && snapQ.size() < 6) break;
                    sp = snapQ.front(); snapQ.pop_front();
                }
                procTimes.push_back(sp->pubDelay); if (procTimes.size() > 60) procTimes.pop_front();
                if (perfOn) { pw.ticks++; pw.logic += sp->logicMs; pw.logicMax = std::max(pw.logicMax, sp->logicMs); pw.ent = sp->entities; }
                frameLogic = std::max(frameLogic, sp->logicMs);
                renderAdopt(sp); adopted++;
                if (sp->quit) quitSeen = true;
                SDL_ShowCursor(sp->mouseHidden ? SDL_DISABLE : SDL_ENABLE);
                snapHand = sp->handCursor;
            }
            audioDeviceTick(nowSec());
            {   // pointing at something clickable: the hand cursor, like the Flash player over a button
                bool hand = wiz.isOpen() ? wizHand : snapHand;
                if (hand != handShown && !kIsAndroid && !kIsSwitch) {
                    static SDL_Cursor* curHand = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
                    static SDL_Cursor* curArrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
                    SDL_Cursor* c = hand ? curHand : curArrow;
                    if (c) SDL_SetCursor(c);
                    handShown = hand;
                }
            }
            if (quitSeen) running = false;
            const Snapshot* cur = renderCurrent();
            double alpha = cur ? (simT - cur->time) / dt : 1.0;
            bool held = alpha > 1.02;
            // if logic falls behind (e.g. when loading a room), extrapolate motion a little instead of freezing
            if (alpha > 1.35) alpha = 1.35;
            int dw, dh; SDL_GL_GetDrawableSize(win, &dw, &dh);
            double r0 = nowSec();
            renderFrame(dw, dh, alpha);
            if (touchUI && !wiz.isOpen()) {
                TouchStick ts[2]; if (touch.sticks(ts)) renderTouchSticks(ts, 2);
#ifdef __SWITCH__
                const bool handheld = appletGetOperationMode() == AppletOperationMode_Handheld;   // (docked: no touch screen)
#else
                const bool handheld = true;
#endif
                if (handheld && cur && cur->gameplay && (cur->touchMask & 8)) {   // pause button next to -LIFE-
                    Mat m = stageToWindowFor(dw, dh); const float* r = cur->touchRects[3]; double x0, y0, x1, y1;
                    m.apply(r[0], r[1], x0, y0); m.apply(r[2], r[3], x1, y1);
                    renderPauseButton(x0, y0, x1, y1);
                }
            }
            if (inputBlocked && preloadTotal && blockedTouchAt > 0 && nowSec() - blockedTouchAt < 4) {   // "why doesn't it react?"
                size_t left = std::min(preloadTotal, preloadLeft);
                renderHint("LOADING " + std::to_string((int)(100 * (preloadTotal - left) / preloadTotal)) + "%");
            }
            if (perfOn) renderOverlay(perfLines);
            renderEndFrame();
            if (!injectShot.empty()) { renderScreenshot(injectShot.c_str()); injectShot.clear(); }
            double r1 = nowSec();
            SDL_GL_SwapWindow(win);
            if (vsyncOn && cfg.gpuSync) {
                // keep reading input until just before the expected refresh, then wait for it as before
                // (low_latency = 0 restores waiting the whole time inside glFinish)
                if (cfg.lowLatency) {
                    double until = lastSwap + period - 0.003;
                    for (;;) { pollInput(); double left = until - nowSec(); if (left <= 0) break; if (left > 0.0025) SDL_Delay(1); }
                }
                renderFinish();
            }   // waits for the refresh: the presentation clock stays exact
            else if (pace) renderPaceAfterSwap();
            double r2 = nowSec();
            double frameInterval = r2 - lastSwap; lastSwap = r2; renderedFrames++;
#ifdef __ANDROID__
            // automatic resolution (render_height = 0): if gameplay stays below ~53 fps for 10 s while the frame time
            // goes to drawing / waiting for the GPU (not to a late game tick), render at a lower resolution. Remembered
            // in isaac.ini (auto_height) so the next launch starts there. Only ever goes down.
            if (cfg.renderHeight == 0 && !preloading) {
                static double winT = 0, busy = 0, lastDrop = 0; static int frames = 0, heldN = 0, bad = 0;
                const Snapshot* cs2 = renderCurrent();
                if (cs2 && cs2->gameplay && frameInterval < 0.25) {
                    frames++; winT += frameInterval; busy += (r1 - r0) + paceMs / 1000.0; if (held) heldN++;
                    if (winT >= 5.0) {
                        double fps = frames / winT, load = busy / frames / period;
                        bool slow = fps < refresh * 0.88 && load > 0.75 && heldN < frames / 4;
                        bad = slow ? bad + 1 : 0;
                        if (bad >= 2 && nowSec() - lastDrop > 20) {
                            static const int ladder[] = {720, 600, 540, 480};
                            int w2, h2; SDL_GL_GetDrawableSize(win, &w2, &h2);
                            int next = 0; for (int L : ladder) if (L < h2 - 8) { next = L; break; }
                            if (next) {
                                logf("automatic resolution: %.0f fps at %d lines (load %.2f) -> %d\n", fps, h2, load, next);
                                javaVoidInt("setRenderHeight", next);
                                saveIniValue(iniPath, "auto_height", std::to_string(next));
                                lastDrop = nowSec();
                            }
                            bad = 0;
                        }
                        frames = 0; heldN = 0; winT = busy = 0;
                    }
                }
            }
#endif
            // if the driver ignores vsync, switch to limiting frames manually
            static double ema = 0; ema = ema == 0 ? frameInterval : ema * 0.95 + frameInterval * 0.05;
            if (vsyncOn && renderedFrames > 120 && ema < period * 0.6) { vsyncOn = false; logf("vsync not effective: limiting to %.0f fps\n", refresh); }
            if (!vsyncOn) { double next = fstart + 1.0 / std::max(60.0, refresh); while (nowSec() < next - 0.002) { pollInput(); SDL_Delay(1); } }
            if (perfOn) {
                double fms = frameInterval * 1000.0, rms = (r1 - r0) * 1000.0, sms = (r2 - r1) * 1000.0;
                pw.frames++; pw.frameMax = std::max(pw.frameMax, fms); pw.render += rms; pw.renderMax = std::max(pw.renderMax, rms); pw.swap += sms + paceMs;
                pw.rasters += g_rstats.rasters; pw.rasterMs += g_rstats.rasterMs; pw.upMB += g_rstats.uploadMB; pw.upMs += g_rstats.uploadMs; pw.tex += g_rstats.texCreates;
                pw.adopt += g_rstats.adoptMs; pw.compile += g_rstats.compileMs; pw.exec += g_rstats.execMs + g_rstats.prerasterMs; pw.pending += g_rstats.asyncPending;
                pw.other += std::max(0.0, (r0 - fstart) * 1000.0 - g_rstats.adoptMs);   // events, controller, etc.
                pw.nodes = g_rstats.nodes; pw.quads += g_rstats.quads; pw.draws += g_rstats.draws; pw.filters += g_rstats.filters; pw.fhits += g_rstats.filterHits;
                if (held) pw.held++;
                bool slow = fms > period * 1000.0 * 1.5;
                if (slow) {
                    pw.slow++;
                    char b[400]; snprintf(b, sizeof b, "hitch: %.1f ms frame (adopt %.1f, compile %.1f, draw %.1f, total render %.1f ms, GPU/vsync wait %.1f ms, rasterized %d in %.1f ms, pending %d, upload %.1f MB, new textures %d, logic %.1f ms%s)\n",
                                          fms, g_rstats.adoptMs, g_rstats.compileMs, g_rstats.execMs + g_rstats.prerasterMs, rms, sms, g_rstats.rasters, g_rstats.rasterMs, g_rstats.asyncPending, g_rstats.uploadMB, g_rstats.texCreates, frameLogic, held ? ", logic delayed" : "");
                    perfLog(b);
                }
                double secs = nowSec() - pwStart;
                if (secs >= 1.0 || perfLines.empty()) {
                    double fps = pw.frames / std::max(1e-3, secs), lg = pw.ticks ? pw.logic / pw.ticks : 0, rd = pw.frames ? pw.render / pw.frames : 0;
                    double sw = pw.frames ? pw.swap / pw.frames : 0, gc = g_gcMs - gcMs0; gcMs0 = g_gcMs;
                    char l[5][200];
                    snprintf(l[0], 200, "FPS %.0f (%.0f HZ)  MAX FRAME %.1f MS  HITCHES %d  HELD %d", fps, refresh, pw.frameMax, pw.slow, pw.held);
                    snprintf(l[1], 200, "LOGIC %.2f MS (MAX %.1f)  %d TICKS/S  MARGIN %.1f MS", lg, pw.logicMax, (int)(pw.ticks / std::max(1e-3, secs) + 0.5), delta * 1000);
                    snprintf(l[2], 200, "RENDER %.2f MS (MAX %.1f)  GPU/VSYNC WAIT %.1f MS", rd, pw.renderMax, sw);
                    snprintf(l[3], 200, "RASTER %d (%.1f MS)  UPLOAD %.1f MB (%.1f MS)  NEW TEX %d  GC %.1f MS  ENT %d", pw.rasters, pw.rasterMs, pw.upMB, pw.upMs, pw.tex, gc, pw.ent);
                    int nf = std::max(1, pw.frames);
                    snprintf(l[4], 200, "ADOPT %.2f  COMPILE %.2f  DRAW %.2f  OTHER %.2f MS | %d NODES %d QUADS %d DRAWS %d BLURS (%d REDONE)  PEND %d",
                             pw.adopt / nf, pw.compile / nf, pw.exec / nf, pw.other / nf, pw.nodes, pw.quads / nf, pw.draws / nf, pw.filters / nf, (pw.filters - pw.fhits) / nf, pw.pending);
                    perfLines.assign(l, l + 5);
                    if (pw.frames > 1) { char b[1200]; snprintf(b, sizeof b, "%s | %s | %s | %s | %s\n", l[0], l[1], l[2], l[3], l[4]); perfLog(b); }
                    pw = PerfWin(); pwStart = nowSec();
                }
            }
        }
        logicRun = false;
        SDL_WaitThread(logic, nullptr);
        saveLearned(preloadPath);
        SDL_Delay(200);   // (let the writer thread finish)
    }
    if (const char* lo = getenv("ISAAC_LEARN_OUT")) saveLearned(lo), SDL_Delay(300);   // (generate the preload list)
    if (stats && !logicMs.empty()) {
        auto rep = [](const char* name, std::vector<double> v) {
            if (v.empty()) return; std::vector<double> s = v; std::sort(s.begin(), s.end());
            double sum = 0; for (double x : s) sum += x;
            logf("[stats] %s: mean %.2f ms, p50 %.2f, p99 %.2f, max %.2f\n", name, sum / s.size(), s[s.size() / 2], s[s.size() * 99 / 100], s.back());
        };
        rep("logic", logicMs); rep("render (CPU)", renderMs);
        std::map<int, std::tuple<double, int, double>> byN;
        for (size_t i = 0; i < logicMs.size(); i++) { auto& e = byN[ballCount[i] / 5 * 5]; std::get<0>(e) += logicMs[i]; std::get<1>(e)++; if (allocs[i] > 0) std::get<2>(e) += allocs[i]; }
        for (auto& kv : byN) logf("[stats]   entities %3d-%3d: %6d ticks, mean logic %.2f ms, %.0f new objects/tick\n", kv.first, kv.first + 4,
                                  std::get<1>(kv.second), std::get<0>(kv.second) / std::get<1>(kv.second), std::get<2>(kv.second) / std::get<1>(kv.second));
    }
    if (stats) { rasterMemStats(); logf("[mem] heap objects %zu, SWF %zu MB, GC total %.1f ms\n", gcCount(), g_movie.data.size() >> 20, g_gcMs); }
    logf("logic ticks: %u, frames drawn: %llu\n", g_player.tick, (unsigned long long)renderedFrames);
    sharedObjectFlushAll();
    if (!wavPath.empty()) { fl::audioDumpClose(); }
    soundStopAll();
    if (gl) SDL_GL_DeleteContext(gl);
    if (win) SDL_DestroyWindow(win);
    SDL_Quit();
#ifdef __SWITCH__
    // On exit, hbloader loads the next program (the Homebrew Menu, or the game again) into this SAME
    // process: none of our threads may remain alive, or they keep running and corrupt the next one's memory.
    platformStopThreads();
#endif
    if (fl::g_restartRequest) {   // ("Delete save": start over with the save deleted)
#ifdef __SWITCH__
        if (envHasNextLoad() && argc > 0) { std::string a0 = std::string(argv[0]) + " --game " + gameTag; envSetNextLoad(argv[0], a0.c_str()); }
#elif defined(__ANDROID__)
        javaVoid("restartApp", gameTag.c_str());   // (relaunches the activity with --game <tag>)
#elif defined(_WIN32)
        wchar_t exe[32768]; GetModuleFileNameW(nullptr, exe, 32768);   // (wide: the folder may have any characters)
        STARTUPINFOW si{}; si.cb = sizeof si; PROCESS_INFORMATION pi{};
        std::wstring cl = std::wstring(GetCommandLineW()) + L" --game " + std::wstring(gameTag.begin(), gameTag.end());
        if (CreateProcessW(exe, &cl[0], nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) { CloseHandle(pi.hThread); CloseHandle(pi.hProcess); }
#else
        std::vector<char*> av(argv, argv + argc); std::string gt = gameTag; av.push_back((char*)"--game"); av.push_back(&gt[0]); av.push_back(nullptr);
        execv("/proc/self/exe", av.data());
#endif
    }
#ifdef __ANDROID__
    // Android may reuse this process for the next launch, and the runtime's globals would not start clean. _exit,
    // not exit: helper threads are still alive, and running static destructors under them crashed
    // ("pthread_mutex_lock called on a destroyed mutex").
    platformStopThreads(); SDL_Delay(100);
    fflush(nullptr);
    _exit(0);
#endif
    return 0;
}
