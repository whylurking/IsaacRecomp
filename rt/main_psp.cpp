// PSP entry point: one thread, logic tick then frame, 30 ticks per second (fewer when a tick takes longer: the game
// slows down, like the Flash player on a slow PC). Folder layout like the other platforms:
//   ms0:/PSP/GAME/IsaacRecomp/EBOOT.PBP, Isaac*.exe (the Steam game), isaac.ini, log.txt, saves/, tools/
#ifdef __PSP__
#include "display.h"
#include "snapshot.h"
#include "render.h"
#include "platform.h"
#include "controls.h"
#include "menuui.h"
#include <pspkernel.h>
#include <pspctrl.h>
#include <psppower.h>
#include <pspdisplay.h>
#include <psprtc.h>
#include <psploadexec.h>
#include <psploadexec_kernel.h>
extern "C" int sctrlKernelLoadExecVSHWithApitype(int apitype, const char* file, struct SceKernelLoadExecVSHParam* param);   // (CFW: SystemCtrlForUser)
#define SDL_MAIN_HANDLED   // (our own module info and main thread, not SDL2main's)
#include <SDL.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cerrno>
#include <cmath>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <atomic>

PSP_MODULE_INFO("IsaacRecomp", PSP_MODULE_USER, 1, 4);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_MAIN_THREAD_STACK_SIZE_KB(1024);
PSP_HEAP_THRESHOLD_SIZE_KB(1536);   // the heap takes all user memory except this (for threads, modules)

namespace fl {
extern const GameCode* const g_pspBuiltinGame;
extern bool g_rngSeedFixed;
void initGlobals();
void runIntervals();
void sharedObjectFlushAll();
extern std::string g_savePath;
void applyLoadPatches(); void recompPatchesTick(); bool recompInGameplay(bool* inGame);
extern float g_audioVolume;
extern std::atomic<float> g_moveScale; extern bool g_joyAccel;
extern size_t g_shapeBudgetBytes; extern bool g_shapeEvict; extern size_t g_soundCacheBytes;
void pspPresent(int vsyncs);
void renderSetBudget(int pages);
void audioInit(bool device);
void gcStats(size_t* objects, size_t* lastLive, size_t* extBytes);
void leanStats(size_t* shapeBytes, size_t* soundBytes);
extern bool (*g_gcMemPressure)();
extern uint32_t g_shapeParses, g_shapeTick;
void menuDebugFocus();
extern double g_bmDrawMs, g_lazyParseMs;
void renderMemStats(size_t* pages, size_t* bitmaps);
void rasterTrim(size_t budget);
void pspAllocInit(size_t bytes);
size_t pspHeapLargestFree();
void pspAllocStats(size_t* used, size_t* peak, size_t* size, size_t* largestFree);
}
using namespace fl;
static int entityCount() {
    Value b = getMemK(getMemK(Value((Object*)vm.root), intern("a")), intern("ball"));
    return b.isObj() && b.o->kind == OK_ARRAY ? (int)((Array*)b.o)->el.size() : 0;
}

// memory diagnostics: the heap in use, and the request that fails when memory runs out
#include <malloc.h>
#include <new>
static unsigned heapUsedKB() { struct mallinfo mi = mallinfo(); return (unsigned)(mi.uordblks / 1024); }
static unsigned heapArenaKB() { struct mallinfo mi = mallinfo(); return (unsigned)(mi.arena / 1024); }
bool g_logBigAllocs = false;
void* operator new(size_t n) {
    if (g_logBigAllocs && n >= 200000)
        fprintf(stderr, "[alloc] %u bytes from %p, heap %u KB\n", (unsigned)n, __builtin_return_address(0), heapUsedKB());
    // (blocks of 1 KB or more 16-byte aligned: BitmapData pixels are used as textures in place)
    if (void* p = n >= 1024 ? memalign(16, n) : malloc(n ? n : 1)) return p;
    struct mallinfo mi = mallinfo();
    fprintf(stderr, "out of memory: %u bytes requested, heap %u KB in use of %u KB, system free %u KB\n", (unsigned)n,
            (unsigned)(mi.uordblks / 1024), (unsigned)(mi.arena / 1024), (unsigned)(sceKernelTotalFreeMemSize() / 1024));
    throw std::bad_alloc();
}
void* operator new[](size_t n) { return operator new(n); }
void operator delete(void* p) noexcept { free(p); }
void operator delete[](void* p) noexcept { free(p); }
void operator delete(void* p, size_t) noexcept { free(p); }
void operator delete[](void* p, size_t) noexcept { free(p); }

#ifdef ISAAC_PSP_FPCOUNT
extern "C" uint32_t g_fpCount[12];
#endif
static volatile bool g_exit = false;
static int exitCb(int, int, void*) { g_exit = true; return 0; }
static int cbThread(SceSize, void*) {
    int cb = sceKernelCreateCallback("exit", exitCb, nullptr);
    sceKernelRegisterExitCallback(cb);
    sceKernelSleepThreadCB();
    return 0;
}
static double nowSec() { return sceKernelGetSystemTimeWide() / 1e6; }

// message on a plain screen (before / instead of the game), until a button is pressed
static void fatalScreen(const std::string& msg) {
    std::vector<std::string> lines; size_t p = 0;
    while (p <= msg.size()) { size_t q = msg.find('\n', p); if (q == std::string::npos) q = msg.size(); std::string l = msg.substr(p, q - p);
        while (l.size() > 74) { lines.push_back(l.substr(0, 74)); l = l.substr(74); } lines.push_back(l); p = q + 1; }
    lines.push_back(""); lines.push_back("PRESS START TO EXIT");
    SceCtrlData pad; uint32_t prev = 0xffffffffu;
    while (!g_exit) {
        renderChooser(480, 272, "THE BINDING OF ISAAC", {}, -1, "", nullptr);
        renderOverlay(lines);
        renderEndFrame(); pspPresent(1);
        sceCtrlReadBufferPositive(&pad, 1);
        if ((pad.Buttons & PSP_CTRL_START) && !(prev & PSP_CTRL_START)) break;
        prev = pad.Buttons;
    }
    sceKernelExitGame();
}

static std::string iniGet(const std::string& path, const std::string& key) {
    FILE* f = fopen(path.c_str(), "r"); if (!f) return "";
    char line[256]; std::string r;
    while (fgets(line, sizeof line, f)) {
        std::string s = line; size_t eq = s.find('='); if (eq == std::string::npos || s[0] == ';') continue;
        auto trim = [](std::string t) { while (!t.empty() && isspace((unsigned char)t.back())) t.pop_back(); size_t i = 0; while (i < t.size() && isspace((unsigned char)t[i])) i++; return t.substr(i); };
        if (trim(s.substr(0, eq)) == key) r = trim(s.substr(eq + 1));
    }
    fclose(f); return r;
}

static void iniSet(const std::string& path, const std::string& key, const std::string& val) {
    std::string out; bool done = false;
    if (FILE* f = fopen(path.c_str(), "r")) {
        char line[256];
        while (fgets(line, sizeof line, f)) {
            std::string l = line; size_t eq = l.find('=');
            std::string k = eq == std::string::npos ? "" : l.substr(0, eq); while (!k.empty() && isspace((unsigned char)k.back())) k.pop_back();
            if (!done && l[0] != ';' && k == key) { out += key + " = " + val + "\n"; done = true; } else out += l;
        }
        fclose(f);
    }
    if (!done) out += key + " = " + val + "\n";
    if (FILE* f = fopen(path.c_str(), "w")) { fputs(out.c_str(), f); fclose(f); }
}

int main(int argc, char** argv) {
    { int th = sceKernelCreateThread("cb", cbThread, 0x11, 0xFA0, 0, nullptr); if (th >= 0) sceKernelStartThread(th, 0, nullptr); }
    scePowerSetClockFrequency(333, 333, 166);
    std::string base = argc > 0 && argv[0] ? argv[0] : "ms0:/PSP/GAME/IsaacRecomp/EBOOT.PBP";
    base = base.substr(0, base.find_last_of('/') + 1);
    // started by EBOOT.PBP (the launcher) as bin/<game>/EBOOT.PBP, with the Isaac*.exe to play as argument
    bool child = g_pspBuiltinGame != nullptr;
    if (child) { size_t k = base.rfind("/bin/"); if (k != std::string::npos) base.resize(k + 1); }
    freopen((base + "log.txt").c_str(), child ? "a" : "w", stderr);
    setvbuf(stderr, nullptr, _IOLBF, 0);
    std::string pref = base + "saves/";
    mkdir((base + "saves").c_str(), 0777);
    platformInitLimits();
    renderInit();
    { void* probe = malloc(1); free(probe); }   // (the heap is reserved on the first allocation)
    pspAllocInit(24u << 20);   // the large blocks' arena (psp_alloc.cpp)
    logf("Isaac recomp (PSP) - folder %s, heap %u KB (%u KB in use), system free %u KB\n", base.c_str(), heapArenaKB(), heapUsedKB(),
         (unsigned)(sceKernelTotalFreeMemSize() / 1024));

    // ---- the game: Isaac*.exe in the folder (the first one recognized; isaac.ini "swf" can name one)
    std::string iniPath = base + "isaac.ini";
    if (FILE* f = fopen(iniPath.c_str(), "r")) fclose(f);
    else if (FILE* f = fopen(iniPath.c_str(), "w")) {
        fputs("; The Binding of Isaac configuration (native recomp, PSP)\n; swf = file name of the Steam Isaac.exe in this folder (empty: the first one found)\nswf =\n"
              "; game = og, wotl or eternal: the version to start when there are several\ngame =\nvolume = 100\n"
              "; sound = 0 turns the sound off (a little more speed in busy rooms)\nsound = 1\n"
              "; performance = 1 shows the performance diagnostics and writes them to saves/performance.txt\nperformance = 0\n", f);
        fclose(f);
    }
    struct Found { std::string path; const KnownGame* g; };
    std::vector<Found> found;
    {
        std::string want = iniGet(iniPath, "swf");
        std::vector<std::string> names;
        if (child && argc > 1) names.push_back(argv[1]);   // (only that one: the launcher chose it)
        else if (!want.empty()) names.push_back(want);
        if (DIR* d = child && argc > 1 ? nullptr : opendir(base.c_str())) {
            while (dirent* e = readdir(d)) {
                std::string n = e->d_name, l = n; for (auto& c : l) c = (char)tolower((unsigned char)c);
                if (l.size() > 4 && (l.compare(l.size() - 4, 4, ".exe") == 0 || l.compare(l.size() - 4, 4, ".swf") == 0)) names.push_back(n);
            }
            closedir(d);
        }
        for (auto& n : names) {
            std::string p = n.find(':') != std::string::npos ? n : base + n;
            uint32_t len = peekSwfLength(p);
            const KnownGame* g = len ? knownGameBySwfLen(len) : nullptr;
            if (!g || (child && !g->code)) continue;
            bool dup = false; for (auto& f : found) dup |= f.g == g;
            if (!dup) found.push_back({p, g});
        }
    }
    if (found.empty())
        fatalScreen("Isaac.exe was not found.\n\nCopy Isaac.exe from the Steam version of The Binding of Isaac\n(original, Wrath of the Lamb or Eternal Edition) into\n" + base);
    int choice = 0;
    if (found.size() > 1) {   // selection screen (D-pad, X)
        std::string last = iniGet(iniPath, "game");
        for (size_t i = 0; i < found.size(); i++) if (last == found[i].g->tag) choice = (int)i;
        std::vector<std::string> items; for (auto& f : found) items.push_back(f.g->title);
        SceCtrlData pad; uint32_t prev = 0xffffffffu;
        for (;;) {
            if (g_exit) sceKernelExitGame();
            renderChooser(480, 272, "THE BINDING OF ISAAC", items, choice, "UP / DOWN: CHOOSE    X: PLAY", nullptr);
            renderEndFrame(); pspPresent(1);
            sceCtrlReadBufferPositive(&pad, 1);
            uint32_t nw = pad.Buttons & ~prev; prev = pad.Buttons;
            if (nw & PSP_CTRL_UP) choice = (choice + (int)found.size() - 1) % (int)found.size();
            if (nw & PSP_CTRL_DOWN) choice = (choice + 1) % (int)found.size();
            if (nw & PSP_CTRL_CROSS) break;
        }
    }
    if (found.size() > 1 && iniGet(iniPath, "game") != found[choice].g->tag) iniSet(iniPath, "game", found[choice].g->tag);
    // ---- each game's code is in its own program (the three don't fit in memory together): start it
    if (!found[choice].g->code) {
        std::string bin = base + "bin/" + found[choice].g->tag + "/EBOOT.PBP", args = bin + '\0' + found[choice].path + '\0';
        logf("starting %s for %s\n", bin.c_str(), found[choice].path.c_str());
        fclose(stderr);
        // as the PSP menu starts homebrew from the Memory Stick (custom firmware's SystemCtrl: the program gets the
        // whole memory, MEMSIZE = 1); the system's own LoadExec if that isn't available (emulators)
        SceKernelLoadExecVSHParam vp; memset(&vp, 0, sizeof vp);
        vp.size = sizeof vp; vp.args = args.size(); vp.argp = (void*)args.data(); vp.key = "game";
        int r = sctrlKernelLoadExecVSHWithApitype(0x141, bin.c_str(), &vp);
        SceKernelLoadExecParam prm{}; prm.size = sizeof prm; prm.args = args.size(); prm.argp = (void*)args.data();
        int r2 = sceKernelLoadExec(bin.c_str(), &prm);
        freopen((base + "log.txt").c_str(), "a", stderr);
        logf("could not start %s: %08x / %08x\n", bin.c_str(), (unsigned)r, (unsigned)r2);
        char msg[64]; snprintf(msg, sizeof msg, " (error %08x)", (unsigned)r);
        fatalScreen("Could not start " + bin + msg + ".\n\nCopy the whole IsaacRecomp folder of the download, with its bin folder.");
    }
    // ---- load (the first time the game is decompressed to saves/: about a minute)
    renderLoadingScreen(480, 272, 0); renderEndFrame(); pspPresent(1);
    g_leanSwf = true; g_leanCacheDir = pref; g_shapeEvict = true; g_shapeBudgetBytes = 5u << 20; g_soundCacheBytes = 2u << 20;
    std::string err;
    initGlobals();
    if (!g_movie.load(found[choice].path, err)) fatalScreen(err);
    std::string gameTag = g_game->tag, gsuf = gameTag == "wotl" ? "" : "_" + gameTag;
    g_savePath = pref + "isaac" + gsuf + "_save.sol";
    applyLoadPatches();
    logf("SWF loaded: v%d, %dx%d, stage %dx%d, heap in use %u KB of %u KB\n", g_movie.version, g_movie.width, g_movie.height, g_stageW, g_stageH,
         heapUsedKB(), heapArenaKB());
    {
        std::string v = iniGet(iniPath, "volume"); g_audioVolume = v.empty() ? 1.f : std::max(0, std::min(100, atoi(v.c_str()))) / 100.f;
    }
    // memory: a collection is forced when less than 3 MB of the heap are left (it can't grow past user memory), or
    // when the large blocks' arena has no free block of 2 MB
    static unsigned s_heapLimitKB = 0;
    { size_t lo = pspHeapLargestFree(); s_heapLimitKB = heapUsedKB() + (unsigned)(lo >> 10);
      logf("heap: %u KB in use, %u KB more available\n", heapUsedKB(), (unsigned)(lo >> 10)); }
    g_gcMemPressure = []() {
        size_t au, ap, as, af; pspAllocStats(&au, &ap, &as, &af);
        return heapUsedKB() + 3 * 1024 > s_heapLimitKB || af < (3u << 19);
    };
    bool sound = iniGet(iniPath, "sound") != "0";
    SDL_SetMainReady(); if (sound) SDL_Init(SDL_INIT_AUDIO);
    audioInit(sound);
    playerInit();
    bool perf = iniGet(iniPath, "performance") == "1";
    // (tests, e.g. in an emulator: saves/autotest.txt = lines "<tick> <button mask hex>", the buttons held from that
    // tick; "<tick> key <flash code> <1|0>", "<tick> click <x> <y>" (stage coordinates), "<tick> quit")
    struct AutoEv { uint32_t tick; int type; uint32_t a; double x, y; };   // type 0 buttons, 1 key, 2 click
    std::vector<AutoEv> autoIn; uint32_t autoQuit = 0, autoGod = 0; size_t autoPos = 0; uint32_t autoMask = 0;
    if (FILE* af = fopen((pref + "autotest.txt").c_str(), "r")) {
        char l[128];
        while (fgets(l, sizeof l, af)) {
            unsigned t; char w[32]; double x = 0, y = 0; int n = 0;
            if (sscanf(l, "%u %31s %n", &t, w, &n) < 2) continue;
            if (!strcmp(w, "quit")) autoQuit = t;
            else if (!strcmp(w, "god")) autoGod = t;
            else if (!strcmp(w, "key")) { int c = 0, d = 0; sscanf(l + n, "%d %d", &c, &d); autoIn.push_back({t, 1, (uint32_t)(c | (d ? 0x10000 : 0)), 0, 0}); }
            else if (!strcmp(w, "click")) { sscanf(l + n, "%lf %lf", &x, &y); autoIn.push_back({t, 2, 0, x, y}); }
            else autoIn.push_back({t, 0, (uint32_t)strtoul(w, nullptr, 16), 0, 0});
        }
        fclose(af); perf = true; g_logBigAllocs = true; logf("autotest: %zu events\n", autoIn.size());
    }
    FILE* perfLog = perf ? fopen((pref + "performance.txt").c_str(), "a") : nullptr;

    // ---- controls. Gameplay: stick / D-pad move, Triangle / Cross / Square / Circle shoot (up, down, left, right),
    // L bomb, R active item, Select pill or card, Start pause. Menus: D-pad / stick choose, Cross confirm, Circle back.
    struct Btn { uint32_t mask; int game, pause, menu; };
    static const Btn kBtns[] = {
        {PSP_CTRL_TRIANGLE, 38, 38, 0}, {PSP_CTRL_CROSS, 40, 32, 0}, {PSP_CTRL_SQUARE, 37, 37, 0}, {PSP_CTRL_CIRCLE, 39, 27, 0},
        {PSP_CTRL_LTRIGGER, 'E', 0, 0}, {PSP_CTRL_RTRIGGER, 32, 0, 0}, {PSP_CTRL_SELECT, 'Q', 'P', 27}, {PSP_CTRL_START, 'P', 'P', 13},
        {PSP_CTRL_UP, 'W', 38, 0}, {PSP_CTRL_DOWN, 'S', 40, 0}, {PSP_CTRL_LEFT, 'A', 37, 0}, {PSP_CTRL_RIGHT, 'D', 39, 0},
    };
    sceCtrlSetSamplingCycle(0); sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
    int held[sizeof kBtns / sizeof kBtns[0]] = {0};
    int keyRefs[256] = {0};
    auto keyDown = [&](int c) { if (c > 0 && c < 256 && keyRefs[c]++ == 0) keyEvent(c, (c == 32 || c == 13 || c == 27) ? c : 0, true); };
    auto keyUp = [&](int c) { if (c > 0 && c < 256 && keyRefs[c] > 0 && --keyRefs[c] == 0) keyEvent(c, 0, false); };
    int stickHeld[4] = {0, 0, 0, 0};
    uint32_t prevButtons = 0; int navRepeatDir = -1; double navNext = 0;
    auto pollInput = [&]() {
        SceCtrlData pad; sceCtrlPeekBufferPositive(&pad, 1);
        if (!autoIn.empty() || autoQuit) {
            while (autoPos < autoIn.size() && autoIn[autoPos].tick <= g_player.tick) {
                const AutoEv& e = autoIn[autoPos++];
                if (e.type == 0) autoMask = e.a;
                else if (e.type == 1) { int c = e.a & 0xffff; keyEvent(c, c == 32 ? 32 : 0, (e.a >> 16) != 0); }
                else { mouseEvent(e.x, e.y, 0); mouseEvent(e.x, e.y, 1); mouseEvent(e.x, e.y, 2); }
            }
            pad.Buttons |= autoMask;
            if (autoQuit && g_player.tick >= autoQuit) g_player.quit = true;
            if (autoGod && g_player.tick >= autoGod && g_player.tick % 30 == 0) {   // (no damage: _root.a.lasth, as in the PC tests)
                Value a = getMemK(Value((Object*)vm.root), intern("a"));
                if (a.isObj()) setProp(a.o, intern("lasth"), Value(1e8));
            }
            if (g_player.tick % 30 == 0) menuDebugFocus();
        }
        bool ig = false, play = recompInGameplay(&ig);
        uint32_t b = pad.Buttons, nw = b & ~prevButtons, up = prevButtons & ~b; prevButtons = b;
        // analog nub: movement (8 directions, speed from the distance) in play; menu navigation otherwise
        float ax = (pad.Lx - 128) / 127.f, ay = (pad.Ly - 128) / 127.f, mag = std::min(1.f, std::hypot(ax, ay));
        int want[4] = {0, 0, 0, 0};   // left, up, right, down
        float moveScale = 1.f;
        if (play && mag > 0.3f) {
            float ux = ax / mag, uy = ay / mag;
            want[0] = ux < -0.383f; want[2] = ux > 0.383f; want[1] = uy < -0.383f; want[3] = uy > 0.383f;
            float t = (mag - 0.3f) / (0.8f - 0.3f); moveScale = 0.4f + 0.6f * std::max(0.f, std::min(1.f, t));
        }
        static const int kMove[4] = {'A', 'W', 'D', 'S'};
        for (int k = 0; k < 4; k++) if (stickHeld[k] != want[k]) { if (want[k]) keyDown(kMove[k]); else keyUp(kMove[k]); stickHeld[k] = want[k]; }
        bool dpadMove = (b & (PSP_CTRL_UP | PSP_CTRL_DOWN | PSP_CTRL_LEFT | PSP_CTRL_RIGHT)) != 0;
        g_moveScale = g_joyAccel && !dpadMove && mag > 0.3f ? moveScale : 1.f;
        // buttons
        for (size_t i = 0; i < sizeof kBtns / sizeof kBtns[0]; i++) {
            const Btn& k = kBtns[i];
            if (up & k.mask) { if (held[i]) keyUp(held[i]); held[i] = 0; }
            if (!(nw & k.mask)) continue;
            if (!play && !ig) {   // menus: the controller outline (menuui.cpp)
                if (k.mask == PSP_CTRL_CROSS) { padNavEvent(NAV_CONFIRM); continue; }
                if (k.mask == PSP_CTRL_CIRCLE) { padNavEvent(NAV_BACK); continue; }
                if (k.mask & (PSP_CTRL_UP | PSP_CTRL_DOWN | PSP_CTRL_LEFT | PSP_CTRL_RIGHT)) continue;
            } else if (!play) {   // pause screen
                if (k.mask == PSP_CTRL_CROSS) { padNavEvent(NAV_CONFIRM); continue; }
                if (k.mask == PSP_CTRL_CIRCLE) { padNavEvent(NAV_BACK); continue; }
                if (k.mask & (PSP_CTRL_UP | PSP_CTRL_DOWN | PSP_CTRL_LEFT | PSP_CTRL_RIGHT)) continue;
            }
            int code = play ? k.game : ig ? k.pause : k.menu;
            if (code) { held[i] = code; keyDown(code); }
        }
        // menu navigation (D-pad or nub) with auto-repeat
        if (!play) {
            int dir = (b & PSP_CTRL_UP) ? NAV_UP : (b & PSP_CTRL_DOWN) ? NAV_DOWN : (b & PSP_CTRL_LEFT) ? NAV_LEFT : (b & PSP_CTRL_RIGHT) ? NAV_RIGHT : -1;
            if (dir < 0 && mag > 0.6f) dir = std::fabs(ax) > std::fabs(ay) ? (ax < 0 ? NAV_LEFT : NAV_RIGHT) : (ay < 0 ? NAV_UP : NAV_DOWN);
            double t = nowSec();
            if (dir != navRepeatDir) { navRepeatDir = dir; if (dir >= 0) { padNavEvent(dir); navNext = t + 0.40; } }
            else if (dir >= 0 && t >= navNext) { padNavEvent(dir); navNext = t + 0.12; }
        } else navRepeatDir = -1;
    };

    // ---- main loop
    const double dt = 1.0 / g_frameRate;
    double next = nowSec(); double perfT0 = nowSec(); int perfTicks = 0; double perfLogic = 0, perfRender = 0, perfMaxTick = 0, perfFrame = 0, perfOther = 0, perfSnap = 0, perfGc = 0, perfInt = 0, perfPat = 0, perfRasterMs = 0; int perfRasters = 0;
    double worstFrame = 0, worstDraw = 0, worstParse = 0;
    double clockStart = nowSec();
    std::vector<std::string> perfLines;
    while (!g_exit && !g_player.quit) {
        pollInput();
        g_player.timeMs = autoIn.empty() && !autoQuit ? (nowSec() - clockStart) * 1000.0 : g_player.tick * 1000.0 / g_frameRate;   // (autotest: the frame clock)
        double t0 = nowSec();
        g_shapeTick = g_player.tick;
        double bm0 = g_bmDrawMs, lp0 = g_lazyParseMs;
        runFrame(); double tA = nowSec(); runIntervals(); double tI = nowSec(); recompPatchesTick(); double tP = nowSec();
        { bool ig = false, gp = recompInGameplay(&ig); menuUiTick(gp, ig); }
        double tB = nowSec();
        if (perf) { perfInt += (tI - tA) * 1000.0; perfPat += (tP - tI) * 1000.0; }
        SnapPtr snap = buildSnapshot(); menuUiFill(*snap);
        if (perf && (tA - t0) * 1000.0 > worstFrame) { worstFrame = (tA - t0) * 1000.0; worstDraw = g_bmDrawMs - bm0; worstParse = g_lazyParseMs - lp0; }
        if (perf) { perfFrame += (tA - t0) * 1000.0; perfOther += (tB - tA) * 1000.0; perfSnap += (nowSec() - tB) * 1000.0; }
        snap->time = g_player.tick * dt;
        double t1 = nowSec();
        gcCollect((t1 - t0) * 1000.0);
        double t2 = nowSec();
        if (perf) perfGc += (t2 - t1) * 1000.0;
        if (g_player.tick % (autoIn.empty() ? 150 : 30) == 0) {   // memory report in log.txt (every 5 s)
            size_t objs, live, ext, shp, snd; gcStats(&objs, &live, &ext); leanStats(&shp, &snd);
            size_t pg, bm; renderMemStats(&pg, &bm);
            size_t au, ap, as, af; pspAllocStats(&au, &ap, &as, &af);
            logf("[mem] large blocks %u KB (peak %u) of %u KB, largest free %u KB\n", (unsigned)(au >> 10), (unsigned)(ap >> 10), (unsigned)(as >> 10), (unsigned)(af >> 10));
            logf("[mem] tick %u: heap %u KB, objects %u (collect at %u), shapes %u KB, sounds %u KB, atlas %u KB, bitmap textures %u KB\n", g_player.tick, heapUsedKB(),
                 (unsigned)objs, (unsigned)(live * 2), (unsigned)(shp / 1024), (unsigned)(snd / 1024), (unsigned)(pg / 1024), (unsigned)(bm / 1024));
        }
        g_rstats = RenderStats();
        renderAdopt(snap);
        renderFrame(480, 272, 1.0);
        if (perf) renderOverlay(perfLines);
        renderEndFrame();
        rasterTrim(3u << 19);   // (decoded SWF bitmaps: 1.5 MB)
        double t3 = nowSec();
        perfRasters += g_rstats.rasters; perfRasterMs += g_rstats.rasterMs;
        // pacing: one tick every 1/30 s (2 refreshes); a late tick is shown at the next refresh
        next += dt;
        double now = nowSec();
        int vs = now < next - 0.020 ? 2 : 1;
        if (now > next + dt) next = now;   // too late: don't try to catch up (the game slows down)
        pspPresent(vs);
        if (perf) {
            perfTicks++; perfLogic += (t1 - t0) * 1000.0 + (t2 - t1) * 1000.0; perfRender += (t3 - t2) * 1000.0; perfMaxTick = std::max(perfMaxTick, (t3 - t0) * 1000.0);
            if (now - perfT0 >= 1.0) {
                char l1[160], l2[160];
                snprintf(l1, sizeof l1, "TICKS/S %d  LOGIC %.1f MS  RENDER %.1f MS  MAX %.0f MS", (int)(perfTicks / (now - perfT0) + 0.5), perfLogic / perfTicks, perfRender / perfTicks, perfMaxTick);
                snprintf(l2, sizeof l2, "ENT %d  RASTER %d (%.0f MS)  SHAPES %u  HEAP %u KB", entityCount(), perfRasters, perfRasterMs, g_shapeParses / perfTicks, heapUsedKB()); g_shapeParses = 0; perfRasters = 0; perfRasterMs = 0;
                char l3[200];
                snprintf(l3, sizeof l3, "FRAME %.1f  INTERVALS %.1f  PATCHES %.1f  UI %.1f  SNAPSHOT %.1f  GC %.1f MS", perfFrame / perfTicks, perfInt / perfTicks, perfPat / perfTicks,
                         (perfOther - perfInt - perfPat) / perfTicks, perfSnap / perfTicks, perfGc / perfTicks);
                perfLines = {l1, l2, l3};
                if (worstFrame > 40) { char l5[160]; snprintf(l5, sizeof l5, "SLOWEST TICK %.0f MS: BITMAP DRAWS %.0f, LOADING GRAPHICS %.0f", worstFrame, worstDraw, worstParse); perfLines.push_back(l5); }
                worstFrame = worstDraw = worstParse = 0;
#ifdef ISAAC_PSP_FPCOUNT
                { char l4[300]; int n = 0;
                  static const char* nm[12] = {"add", "sub", "mul", "div", "ne", "eq", "lt", "gt", "ge", "le", "unord", "fix"};
                  for (int i = 0; i < 12; i++) { n += snprintf(l4 + n, sizeof l4 - n, "%s %u ", nm[i], g_fpCount[i] / perfTicks); g_fpCount[i] = 0; }
                  perfLines.push_back(l4); }
#endif
                if (perfLog) { std::string all; for (auto& l : perfLines) all += (all.empty() ? "" : " | ") + l; fprintf(perfLog, "%s\n", all.c_str()); fflush(perfLog); }
                perfT0 = now; perfTicks = 0; perfLogic = perfRender = perfMaxTick = perfFrame = perfOther = perfSnap = perfGc = perfInt = perfPat = 0;
            }
        }
    }
    sharedObjectFlushAll();
    if (perfLog) fclose(perfLog);
    sceKernelExitGame();
    return 0;
}
#endif
