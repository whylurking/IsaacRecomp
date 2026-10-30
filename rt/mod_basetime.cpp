// BaseTime (mods/BaseTime.mod): speedrun timers.
//   - game time (top): the time you can play (you can move): it stops while paused, in cutscenes, boss intros,
//     loading and between floors;
//   - total time (below): everything since the run started: pauses, cutscenes and loading included (the real clock).
// They start when you can first move in a run and stop when it ends (you die, or reach an ending and
// the game goes back to the menu). The main menu shows, on a note of its own, your best time and below it your last
// run's time ("win": it reached an ending; "died": it died or was left). The best time is the fastest run to your
// furthest ending: Mom, then Mom's Heart, then Isaac / Satan, then The Chest - a further ending always replaces it.
// Game time is shown as mm:ss, total time as mm:ss:mmm. The menu: the best (game) time and the last run's total
// time, both mm:ss. Kept in saves/basetime.txt.
//
// Game time is counted in game ticks (exactly 30 a second, also when the screen is slower: exact and repeatable);
// total time is the game's own real clock (getTimer: _root's timeMs), which keeps running while the game loads.
//
// How a run is followed (the game's own state, read-only):
//   - playing: _root.a.player exists and _root.a.unpause (recompInGameplay);
//   - start: the first tick playing after the menu or after the previous run ended (_root.newstartt is the game's
//     "new run" flag: set by the menu and by holding R, cleared when the run begins);
//   - death: the player's death (player.dones, its clip on frame 6). If an extra life brings you back, the run goes on;
//   - ending: _root on an ending frame (9-40), reached on floor _root.chaps (6 Mom, 8 Mom's Heart, 9 Isaac in the
//     Cathedral / Satan in Sheol, 11 The Chest). If the game comes back to a floor afterwards (Mom's ending, then
//     the Womb), the run goes on;
//   - the run is over (and saved) when the game goes back to the menu or a new run starts.
#include "mods.h"
#include "mod_ui.h"
#include "core.h"
#include "display.h"
#include "platform.h"
#include "snapshot.h"
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace fl {
bool recompInGameplay(bool* inGame);

namespace {
Value V(const Value& o, const char* k) { return o.isObj() ? getMemK(o, intern(k)) : VUNDEF; }
double num(const Value& v, double def = 0) { double d = toNum(v); return std::isfinite(d) ? d : def; }

enum Result { R_NONE, R_WIN, R_DEATH, R_QUIT };
// endings, from nearest to furthest (the rank: Isaac and Satan are as far)
const char* kEndingName[] = {"", "Mom", "Mom's Heart", "Isaac", "Satan", "The Chest"};
const int kEndingRank[] = {0, 1, 2, 3, 3, 4};
struct Best { int ending = 0; long ms = -1; };   // ending 0: none yet
enum { T_GAME, T_TOTAL };

bool s_running = false;
long s_gameTicks = 0; double s_total0 = 0;          // game ticks; the clock (ms) when the run started
long s_endGame = -1, s_endTotal = -1;               // frozen at a death / an ending (-1: running)
int s_result = R_NONE;                              // what the run would be if it stopped now
int s_runEnding = 0; long s_runEndMs[2] = {-1, -1}; // the furthest ending this run reached, and when
long s_lastMs[2] = {-1, -1}; int s_lastResult = R_NONE;
Best s_best[2];
bool s_loaded = false;

long gameMs(long ticks) { return ticks * 1000 / 30; }
long nowGame() { return s_endGame >= 0 ? s_endGame : gameMs(s_gameTicks); }
long nowTotal() { return s_endTotal >= 0 ? s_endTotal : (long)(g_player.timeMs - s_total0); }
std::string fmtMS(long ms) {   // mm:ss
    if (ms < 0) ms = 0;
    char b[24]; snprintf(b, sizeof b, "%02ld:%02ld", ms / 60000, ms / 1000 % 60);
    return b;
}
std::string fmt(long ms) {   // mm:ss:mmm
    if (ms < 0) ms = 0;
    char b[32]; snprintf(b, sizeof b, "%02ld:%02ld:%03ld", ms / 60000, ms / 1000 % 60, ms % 1000);
    return b;
}
std::string file() { return g_modsSaveDir + "basetime.txt"; }
void load() {
    s_loaded = true;
    FILE* f = platformFopen(file(), "rb"); if (!f) return;
    char k[40]; long v;
    while (fscanf(f, "%39s %ld", k, &v) == 2) {
        std::string key = k;
        if (key == "last_game") s_lastMs[T_GAME] = v; else if (key == "last_total") s_lastMs[T_TOTAL] = v;
        else if (key == "last_result") s_lastResult = (int)v;
        else if (key == "best_game_ending") s_best[T_GAME].ending = (int)v; else if (key == "best_game") s_best[T_GAME].ms = v;
        else if (key == "best_total_ending") s_best[T_TOTAL].ending = (int)v; else if (key == "best_total") s_best[T_TOTAL].ms = v;
        else if (key == "last_run") s_lastMs[T_GAME] = s_lastMs[T_TOTAL] = gameMs(v);   // (version 1.0's file)
    }
    for (auto& b : s_best) if (b.ending < 0 || b.ending > 5) b = Best();
    fclose(f);
}
void save() {
    if (genesisActive()) return;   // (GenesisConsole: nothing is saved)
    FILE* f = platformFopen(file(), "wb"); if (!f) return;
    fprintf(f, "last_game %ld\nlast_total %ld\nlast_result %d\nbest_game_ending %d\nbest_game %ld\nbest_total_ending %d\nbest_total %ld\n",
            s_lastMs[T_GAME], s_lastMs[T_TOTAL], s_lastResult, s_best[T_GAME].ending, s_best[T_GAME].ms, s_best[T_TOTAL].ending, s_best[T_TOTAL].ms);
    fclose(f);
}
void finish() {
    s_lastMs[T_GAME] = nowGame(); s_lastMs[T_TOTAL] = nowTotal();
    s_lastResult = s_runEnding ? R_WIN : R_DEATH;   // (an ending reached: a win; died or left: died)
    if (s_runEnding)
        for (int t = 0; t < 2; t++) {
            Best& b = s_best[t];
            int r = kEndingRank[s_runEnding], br = kEndingRank[b.ending];
            if (r > br || (r == br && (b.ms < 0 || s_runEndMs[t] < b.ms))) b = {s_runEnding, s_runEndMs[t]};
        }
    logf("basetime: run %s in %s (total %s)%s%s\n", s_lastResult == R_WIN ? "won" : "over", fmt(s_lastMs[T_GAME]).c_str(), fmt(s_lastMs[T_TOTAL]).c_str(),
         s_runEnding ? ", ending: " : "", s_runEnding ? kEndingName[s_runEnding] : "");
    save();
    s_running = false; s_result = R_NONE; s_endGame = s_endTotal = -1; s_runEnding = 0;
}
int endingHere(const Value& root) {   // which ending the game is showing (reached on this floor)
    int chaps = (int)num(V(root, "chaps"), 0);
    bool alt = truthy(V(root, "altchap"));
    if (chaps >= 11) return 5;
    if (chaps >= 9) return alt ? 3 : 4;
    if (chaps >= 7) return 2;
    return 1;
}

// ---- in-game display (inside _root.hud, like StatOn). One text field per character: a timer changes every tick,
// and a whole new text each tick would be rasterized and uploaded each tick (on the Switch that stalls the GPU);
// single characters are rasterized once and then reused (the text cache is keyed by content).
const int kSlots[2] = {11, 11};
const float kSize[2] = {14.f, 10.5f};
MovieClip* g_clip = nullptr; EditTextObj* g_c[2][11][2] = {};   // [line][char][shadow, text]
// ---- main menu display (on the menu sheet, so it moves with it)
MovieClip* g_menu = nullptr; std::string g_menuKey;
void removeClip() {
    if (g_clip && !g_clip->removed && g_clip->parent) removeObject(g_clip);
    g_clip = nullptr;
    for (auto& l : g_c) for (auto& c : l) c[0] = c[1] = nullptr;
}
void removeMenu() { if (g_menu && !g_menu->removed && g_menu->parent) removeObject(g_menu); g_menu = nullptr; g_menuKey.clear(); }

void setLine(int line, const std::string& s, uint32_t argb) {
    const float size = kSize[line];
    const float digitW = modui::textW("0", size) + 0.5f;
    double x = 0;
    for (int i = 0; i < kSlots[line]; i++) {
        std::string ch = i < (int)s.size() ? s.substr(i, 1) : "";
        // digits: a fixed-width slot (the numbers don't shift as they change), each digit centered in it (a narrow
        // "1" left-aligned left a hole before the next character: "01 :00"); other characters: their own width
        bool digit = !ch.empty() && ch[0] >= '0' && ch[0] <= '9';
        float w = ch.empty() ? 0 : modui::textW(ch, size);
        double cx = digit ? x + (digitW - 0.5f - w) / 2 : x;
        for (int k = 0; k < 2; k++) {
            EditTextObj* e = g_c[line][i][k]; if (!e) continue;
            e->argb = k == 0 ? 0xd0000000u : argb;
            e->setText(ch);
            if (e->m.tx != cx + k || !e->scriptXform) { e->m.tx = cx + k; e->scriptXform = true; }
        }
        if (!ch.empty()) x += digit ? digitW : w + 0.5f;
    }
}
void drawGame(const Value& hudV, bool hyper, double opacity) {
    MovieClip* hud = (MovieClip*)hudV.o;
    if (!g_clip || g_clip->removed || g_clip->parent != hud) {
        removeClip();
        g_clip = createEmptyMovieClip(hud, intern("__basetime"), 1047930);
        if (!g_clip) return;
        int dp = 1;
        for (int line = 0; line < 2; line++) {
            double y = line == 0 ? 0 : 17;
            for (int i = 0; i < kSlots[line]; i++)
                for (int k = 0; k < 2; k++) {   // (shadow, then text)
                    DisplayObject* d = synthPut(g_clip, synthTextField(16, kSize[line], k == 0 ? 0xd0000000u : 0xfff4f0e8u, 0), dp++, k, y + k);
                    g_c[line][i][k] = d && d->kind == OK_EDITTEXT ? (EditTextObj*)d : nullptr;
                }
        }
    }
    g_clip->cx.am = (float)opacity;
    // where: left of the map in the HUD band; with WideSaac's Hyper Zoom (no band) on the room's bottom left wall
    g_clip->m = Mat(); g_clip->m.tx = hyper ? 8 : 6; g_clip->m.ty = hyper ? 424 : 9; g_clip->scriptXform = true;
    uint32_t totalColor = s_result == R_WIN ? 0xff7be07bu : s_result == R_DEATH ? 0xffe86a5eu : 0xfff0c674u;
    setLine(0, fmtMS(nowGame()), s_result == R_NONE ? 0xfff4f0e8u : totalColor);
    setLine(1, fmt(nowTotal()), totalColor);
}
void drawMenu(int which) {
    Value men = V(V(Value((Object*)vm.root), "men"), "men");
    if (!men.isObj() || men.o->kind != OK_CLIP) { removeMenu(); return; }
    MovieClip* sheet = (MovieClip*)men.o;
    const Best& best = s_best[which];
    char key[96]; snprintf(key, sizeof key, "%d %ld %d %ld %d", which, s_lastMs[T_TOTAL], s_lastResult, best.ms, best.ending);
    if (g_menu && !g_menu->removed && g_menu->parent == sheet && g_menuKey == key) return;
    removeMenu();
    if (s_lastMs[T_TOTAL] < 0 && best.ms < 0) return;
    g_menu = createEmptyMovieClip(sheet, intern("__basetime"), 1048300);
    if (!g_menu) return;
    g_menuKey = key;
    // A note of its own, like the menu's paper notes (same paper colour and grey marker ink), taped on the main page
    // to the right of the menu's sheet (sheet coordinates: the page is at _root (-227.6, -198.6)), a little tilted.
    // The best time first (with the ending it reached), the last run below it.
    const uint32_t paper = 0xffe7e0c9u, ink = 0xff6f6a66u, inkDim = 0xff98938fu;
    const double W = 104, pad = 8;
    struct L { std::string t; float H; uint32_t c; double gap; };
    std::vector<L> lines;
    lines.push_back({best.ending ? std::string("Best - ") + kEndingName[best.ending] : std::string("Best time"), 8.5f, inkDim, 4});
    lines.push_back({best.ms >= 0 ? fmtMS(best.ms) : "--:--", 15, ink, 9});
    if (s_lastMs[T_TOTAL] >= 0) {   // (the last run: its total time, pauses included)
        lines.push_back({std::string("Last run - ") + (s_lastResult == R_WIN ? "win" : "died"), 8.5f, inkDim, 4});
        lines.push_back({fmtMS(s_lastMs[T_TOTAL]), 12, ink, 0});
    }
    double H = pad + 3;
    for (auto& l : lines) H += l.H + l.gap;
    H += pad;
    const double ang = -2.5 * M_PI / 180, cs = std::cos(ang), sn = std::sin(ang);
    g_menu->m.a = cs; g_menu->m.b = sn; g_menu->m.c = -sn; g_menu->m.d = cs;
    g_menu->m.tx = 486 + 227.6; g_menu->m.ty = 334 + 198.6; g_menu->scriptXform = true;
    int dp = 1;
    modui::rect(g_menu, dp, 2.5, 3.5, W, H, 0x30000000u);          // shadow
    modui::rect(g_menu, dp, 0, 0, W, H, paper);
    modui::rect(g_menu, dp, 0, H - 4, W, 4, 0xffdcd4b9u);          // (the paper's lower edge, a bit darker)
    if (DisplayObject* t = modui::rect(g_menu, dp, W / 2 - 17, -6, 34, 12, 0xb4f8f5ecu)) {   // tape, tilted the other way
        const double ta = 4 * M_PI / 180, c2 = std::cos(ta), s2 = std::sin(ta);
        t->m.a = 0.34 * c2; t->m.b = 0.34 * s2; t->m.c = -0.12 * s2; t->m.d = 0.12 * c2;
    }
    double y = pad + 3;
    for (auto& l : lines) {
        float w = 0; uint16_t id = synthHandText(l.t, l.H, l.c, &w);
        double k = w > W - 8 ? (W - 8) / w : 1;   // (a long ending name: a little narrower)
        DisplayObject* d = synthPut(g_menu, id, dp++, (W - w * k) / 2, y);
        if (d && k < 1) { d->m.a = k; d->scriptXform = true; }
        y += l.H + l.gap;
    }
}
}  // namespace

void basetimeTick(const ModInfo& m) {
    static bool rooted = false;
    if (!rooted) { rooted = true; gcAddRoot((GcObj**)&g_clip); gcAddRoot((GcObj**)&g_menu); for (auto& l : g_c) for (auto& c : l) { gcAddRoot((GcObj**)&c[0]); gcAddRoot((GcObj**)&c[1]); } }
    if (!s_loaded) load();
    Value root((Object*)vm.root);
    Value a = V(root, "a"), player = V(a, "player");
    bool inGame = false, play = recompInGameplay(&inGame);
    int frame = vm.root->kind == OK_CLIP ? ((MovieClip*)vm.root)->cur : 0;
    bool newRun = truthy(V(root, "newstartt"));
    bool menu = frame == 5;
    static uint32_t serial = 0; bool restarted = modsRunSerial() != serial; serial = modsRunSerial();
    // the run
    if (s_running && (menu || restarted || (newRun && s_gameTicks > 30))) finish();
    if (!s_running && play && !newRun && !menu) {
        s_running = true; s_gameTicks = 0; s_total0 = g_player.timeMs - 1000.0 / 30;   // (this tick counts in both)
        s_endGame = s_endTotal = -1;
        s_result = R_NONE; s_runEnding = 0; s_runEndMs[0] = s_runEndMs[1] = -1;
    }
    if (s_running) {
        bool dead = false;
        if (player.isObj()) {
            Value d = V(player, "d");
            dead = truthy(V(player, "dones")) && d.isObj() && d.o->kind == OK_CLIP && ((MovieClip*)d.o)->cur == 6;
        }
        bool ending = frame >= 9 && frame <= 40;
        if (s_result == R_NONE) {
            if (dead || ending) {
                s_result = dead ? R_DEATH : R_WIN;
                s_endGame = nowGame(); s_endTotal = nowTotal();
                if (ending) {
                    int e = endingHere(root);
                    if (kEndingRank[e] >= kEndingRank[s_runEnding]) { s_runEnding = e; s_runEndMs[T_GAME] = s_endGame; s_runEndMs[T_TOTAL] = s_endTotal; }
                }
            }
        } else if (play && !dead && !ending) { s_result = R_NONE; s_endGame = s_endTotal = -1; }   // (an extra life, or the run goes on after an ending)
        if (play && s_result == R_NONE) s_gameTicks++;
    }
    // display
    Value hud = V(root, "hud");
    if (menu) { removeClip(); drawMenu(T_GAME); return; }
    removeMenu();
    if (!inGame || !hud.isObj() || hud.o->kind != OK_CLIP || !s_running) { if (g_clip) g_clip->visible = false; return; }
    double op = std::max(0.1, std::min(1.0, atof(m.opt("opacity", "90").c_str()) / 100));
    bool hyper = wideMinimapAvailable();
    drawGame(hud, hyper, op);
    // Eternal's hard-mode mark (hud.hardon, the pentagram) is on the room's bottom left wall, where the timer goes
    // with Hyper Zoom: it goes up, just above the timer (only on screen: the game's clip keeps its place)
    if (hyper) {
        Value mark = V(hud, "hardon");
        if (mark.isObj() && mark.o->isDisplay() && ((DisplayObject*)mark.o)->visible) {
            DisplayObject* d = (DisplayObject*)mark.o;
            SnapOv& o = g_snapOv[d];
            if (!o.hasM) { o.hasM = true; o.m = d->m; }
            o.m.ty -= 35;
        }
    }
    if (g_clip) g_clip->visible = true;
}
}  // namespace fl
