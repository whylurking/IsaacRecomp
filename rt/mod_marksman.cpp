// Marksman (mods/Marksman.mod): the endings each character has reached, as Rebirth's completion marks.
//
// - The character select gets one more note on the board (the game's own paper and pin, copied from the "Select!"
//   note), with a mark for each ending - Mom, Mom's Heart, Isaac, Satan, ??? (The Chest) - for the character shown:
//   faint while it hasn't been reached, in the notes' ink once it has, red when it was reached in Eternal Edition's
//   hard mode. The original game has Mom and Mom's Heart only. The Gone has his own marks (with his mod on).
// - During a challenge (the game's ten and Challenge Lab's) the goal's mark is shown small in the HUD band, right of
//   the map (with WideSaac's Hyper Zoom, on the room's top left wall).
// An ending counts when its boss room is cleared (it was seen shut first) on its floor - _root.chaps 6 Mom, 8 Mom's
// Heart / It Lives, 9 Isaac (the Cathedral, altchap) or Satan (Sheol), 11 ??? - or when the game shows the ending
// (_root frames 9-40, as BaseTime): normal runs only (not the challenges, not Challenge Lab, not with GenesisConsole).
// Kept in saves/marksman_<game>.txt. NOTES.md "v1.6: Marksman".
#include "mods.h"
#include "core.h"
#include "display.h"
#include "platform.h"
#include "swfdata.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace fl {
bool recompInGameplay(bool* inGame);
extern std::string g_modsSaveDir;
bool goneRunActive();   // mod_thegone.cpp
bool goneSelected();
bool clabRunActive();   // mod_challengelab.cpp
int clabGoal();
int clabCharacter();

namespace {
Value V(const Value& o, const char* k) { return o.isObj() ? getMemK(o, intern(k)) : VUNDEF; }
double num(const Value& v, double def = 0) { double d = toNum(v); return std::isfinite(d) ? d : def; }
Value root() { return Value((Object*)vm.root); }
std::string game() { return g_game ? g_game->tag : "wotl"; }
bool isOg() { return game() == "og"; }

// ============================================================ the marks
enum Ending { E_MOM, E_HEART, E_ISAAC, E_SATAN, E_CHEST, E_COUNT };
const char* kEndKey[] = {"mom", "moms_heart", "isaac", "satan", "the_chest"};
const char* kCharKey[] = {"isaac", "magdalene", "cain", "judas", "bluebaby", "eve", "samson", "thegone"};
const int kChars = 8, kGone = 7;
using Marks = std::array<int, E_COUNT>;   // 0 not reached, 1 reached, 2 reached in hard mode
std::map<int, Marks> g_marks;
std::array<bool, E_COUNT> g_seen{};   // the ending's last boss seen once, in any run (until then its mark isn't shown: no spoilers)
bool g_loaded = false;
std::string g_loadedFor;

std::string file() { return g_modsSaveDir + "marksman_" + game() + ".txt"; }
std::string trim(std::string s) {
    while (!s.empty() && strchr(" \t\r\n", s.back())) s.pop_back();
    size_t i = 0; while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) i++;
    return s.substr(i);
}
void load() {
    if (g_loaded && g_loadedFor == game()) return;
    g_loaded = true; g_loadedFor = game(); g_marks.clear(); g_seen.fill(false);
    FILE* f = platformFopen(file(), "rb");
    if (!f) return;
    char buf[512];
    while (fgets(buf, sizeof buf, f)) {   // "isaac = mom:1 moms_heart:2 ..."
        std::string l = trim(buf);
        if (l.empty() || l[0] == ';' || l[0] == '#') continue;
        size_t eq = l.find('='); if (eq == std::string::npos) continue;
        std::string k = trim(l.substr(0, eq)), v = l.substr(eq + 1);
        if (k == "seen") {   // "seen = mom moms_heart ..."
            size_t p = 0; v += " ";
            while (p < v.size()) { size_t e = v.find(' ', p); std::string t = v.substr(p, e - p); p = e + 1; for (int x = 0; x < E_COUNT; x++) if (t == kEndKey[x]) g_seen[x] = true; }
            continue;
        }
        int ch = -1; for (int i = 0; i < kChars; i++) if (k == kCharKey[i]) ch = i;
        if (ch < 0) continue;
        Marks m{}; size_t p = 0;
        while (p < v.size()) {
            while (p < v.size() && v[p] == ' ') p++;
            size_t e = v.find(' ', p); if (e == std::string::npos) e = v.size();
            std::string t = v.substr(p, e - p); p = e;
            size_t c = t.find(':'); if (c == std::string::npos) continue;
            for (int i = 0; i < E_COUNT; i++) if (t.substr(0, c) == kEndKey[i]) m[i] = std::max(0, std::min(2, atoi(t.c_str() + c + 1)));
        }
        g_marks[ch] = m;
    }
    fclose(f);
}
void save() {
    FILE* f = platformFopen(file(), "wb");
    if (!f) { logf("marksman: can't write %s\n", file().c_str()); return; }
    fprintf(f, "; Marksman: the endings each character has reached (0 not yet, 1 reached, 2 reached in hard mode)\n");
    fprintf(f, "; seen: the bosses already met once (until then their marks aren't shown)\nseen =");
    for (int i = 0; i < E_COUNT; i++) if (g_seen[i]) fprintf(f, " %s", kEndKey[i]);
    fprintf(f, " \n");
    for (auto& [ch, m] : g_marks) {
        fprintf(f, "%s =", kCharKey[ch]);
        for (int i = 0; i < E_COUNT; i++) fprintf(f, " %s:%d", kEndKey[i], m[i]);
        fprintf(f, "\n");
    }
    fclose(f);
}
Marks marksOf(int ch) { load(); auto it = g_marks.find(ch); return it == g_marks.end() ? Marks{} : it->second; }
void addMark(int ch, int e, int level) {
    if (ch < 0 || ch >= kChars || e < 0 || e >= E_COUNT) return;
    load();
    Marks& m = g_marks[ch];
    if (m[e] >= level && g_seen[e]) return;
    m[e] = std::max(m[e], level); g_seen[e] = true;
    save();
    logf("marksman: %s reached %s%s\n", kCharKey[ch], kEndKey[e], level == 2 ? " (hard mode)" : "");
}

// ============================================================ the symbols
// Drawn as marker strokes, as the notes' writing (round ends, a little uneven). Unit coordinates (-1..1, y down).
struct Seg { char op; float x, y, cx, cy; };   // 'M' / 'L' x y, 'Q' cx cy -> x y
struct Stroke { std::vector<Seg> segs; float width; bool fill = false; };
std::vector<Stroke> symbolStrokes(int e) {
    std::vector<Stroke> s;
    auto M = [](float x, float y) { return Seg{'M', x, y, 0, 0}; };
    auto L = [](float x, float y) { return Seg{'L', x, y, 0, 0}; };
    auto Q = [](float cx, float cy, float x, float y) { return Seg{'Q', x, y, cx, cy}; };
    auto dot = [&](float x, float y, float w) { s.push_back({{M(x, y), L(x + 0.001f, y + 0.001f)}, w}); };
    switch (e) {
    case E_MOM:   // a footprint: Mom's stomp
        s.push_back({{M(-0.12f, -0.30f), Q(0.42f, -0.42f, 0.40f, 0.08f), Q(0.38f, 0.52f, 0.06f, 0.82f), Q(-0.34f, 0.94f, -0.36f, 0.50f),
                      Q(-0.34f, 0.16f, -0.38f, -0.08f), Q(-0.40f, -0.28f, -0.12f, -0.30f)}, 0.15f, true});
        dot(-0.30f, -0.60f, 0.30f); dot(-0.02f, -0.72f, 0.22f); dot(0.20f, -0.66f, 0.19f); dot(0.38f, -0.53f, 0.16f); dot(0.50f, -0.36f, 0.14f);
        break;
    case E_HEART:   // Mom's Heart
        s.push_back({{M(0, 0.80f), Q(-0.98f, 0.14f, -0.80f, -0.40f), Q(-0.56f, -0.96f, 0, -0.46f), Q(0.56f, -0.96f, 0.80f, -0.40f),
                      Q(0.98f, 0.14f, 0, 0.80f)}, 0.15f, true});
        break;
    case E_ISAAC:   // a cross (the Cathedral)
        s.push_back({{M(0, -0.92f), L(0, 0.92f)}, 0.17f});
        s.push_back({{M(-0.52f, -0.36f), L(0.52f, -0.36f)}, 0.17f});
        break;
    case E_SATAN:   // the same cross upside down (Sheol)
        s.push_back({{M(0, -0.92f), L(0, 0.92f)}, 0.17f});
        s.push_back({{M(-0.52f, 0.36f), L(0.52f, 0.36f)}, 0.17f});
        break;
    case E_CHEST:   // ??? (The Chest)
        s.push_back({{M(-0.44f, -0.42f), Q(-0.44f, -0.90f, 0.02f, -0.90f), Q(0.48f, -0.90f, 0.48f, -0.46f), Q(0.48f, -0.14f, 0.12f, 0.04f),
                      Q(0.0f, 0.12f, 0.0f, 0.36f)}, 0.17f});
        dot(0.0f, 0.76f, 0.24f);
        break;
    }
    return s;
}
// one shape: the symbol `e` `size` units across. ink: the strokes; fill: inside the closed ones (0: none)
uint16_t symbolShape(int e, float size, uint32_t ink, uint32_t fill, float widthK) {
    static std::map<std::string, uint16_t> cache;
    char key[96]; snprintf(key, sizeof key, "%d|%.1f|%08x|%08x|%.2f", e, size, ink, fill, widthK);
    auto it = cache.find(key); if (it != cache.end()) return it->second;
    auto* sd = new ShapeDef(); sd->type = CH_SHAPE;
    const float h = size / 2;
    uint32_t seed = 7u + (uint32_t)e * 31u;
    auto jit = [&]() { seed = seed * 1103515245u + 12345u; return ((seed >> 16) % 1000 / 1000.0f - 0.5f) * 0.035f; };
    for (const Stroke& st : symbolStrokes(e)) {
        ShapeLayer L;
        Path p;
        for (const Seg& g : st.segs) {
            float x = (g.x + jit()) * h, y = (g.y + jit()) * h;
            if (g.op == 'Q') { p.verbs.push_back(2); p.pts.insert(p.pts.end(), {(g.cx + jit()) * h, (g.cy + jit()) * h, x, y}); }
            else { p.verbs.push_back(g.op == 'M' ? 0 : 1); p.pts.insert(p.pts.end(), {x, y}); }
        }
        if (st.fill && (fill >> 24)) { FillStyle f; f.argb = fill; L.fills.push_back(f); L.fillPaths.push_back(p); }
        LineStyle ls; ls.width = st.width * h * widthK; ls.argb = ink; ls.noClose = !st.fill;
        L.lines.push_back(ls); L.linePaths.push_back(p);
        sd->layers.push_back(L);
    }
    float pad = 0.3f * h;
    sd->bounds = {-h - pad, -h - pad, h + pad, h + pad};
    return cache[key] = synthAddChar(sd);
}
const uint32_t kInk = 0xff878484u, kGhost = 0x48878484u, kHardInk = 0xffb2392du, kHardFill = 0x50d24a3cu, kInkFill = 0x30878484u;

// ============================================================ the note on the character select
MovieClip* g_note = nullptr; MovieClip* g_noteMarks = nullptr; Object* g_noteBoard = nullptr;
std::string g_noteKey;
bool near(const RectF& r, double cx, double cy, double w, double h, double tol) {
    return r.valid() && std::fabs((r.x0 + r.x1) / 2 - cx) < tol && std::fabs((r.y0 + r.y1) / 2 - cy) < tol &&
           std::fabs((r.x1 - r.x0) - w) < tol && std::fabs((r.y1 - r.y0) - h) < tol;
}
Mat inverse(const Mat& m) {
    double det = m.a * m.d - m.b * m.c; Mat r;
    if (std::fabs(det) < 1e-12) return r;
    r.a = m.d / det; r.b = -m.b / det; r.c = -m.c / det; r.d = m.a / det;
    r.tx = -(r.a * m.tx + r.c * m.ty); r.ty = -(r.b * m.tx + r.d * m.ty);
    return r;
}
Mat mul(const Mat& p, const Mat& q) {   // p after q
    Mat r;
    r.a = p.a * q.a + p.c * q.b; r.b = p.b * q.a + p.d * q.b;
    r.c = p.a * q.c + p.c * q.d; r.d = p.b * q.c + p.d * q.d;
    r.tx = p.a * q.tx + p.c * q.ty + p.tx; r.ty = p.b * q.tx + p.d * q.ty + p.ty;
    return r;
}
void removeNote() {
    if (g_note && !g_note->removed && g_note->parent) removeObject(g_note);
    g_note = nullptr; g_noteMarks = nullptr; g_noteBoard = nullptr; g_noteKey.clear();
}
// The board (_root.men.men) at the character select: the "Select!" note is two copies of the game's paper clip (a
// shadow and the paper) centred near (1138, 928) on the stage, 453 x 441, and its pin two small clips at (1112, 798).
// The new note is a copy of them, smaller: in Eternal Edition between the hard mode note and Select! (over both
// papers, under the character sheet and the writing), elsewhere where Eternal's hard mode note is.
void buildNote(MovieClip* board) {
    removeNote();
    std::vector<DisplayObject*> paper, pin;
    const double u = g_stageH / 1080.0;   // (the numbers here are for the PC's 1440x1080 stage; Switch / Android 960x720)
    // (found by its centre in the board's own coordinates, 1183.5, 839.8 in the three games: the board slides from
    // sheet to sheet, so its place on the stage changes)
    {
        Mat bi = inverse(worldMatrix(board));
        for (DisplayObject* k : board->kids) {
            if (k->kind != OK_CLIP || !k->visible) continue;
            RectF r = stageBounds(k);
            if (!r.valid() || r.x1 - r.x0 < 300 * u) continue;
            double cx = (r.x0 + r.x1) / 2, cy = (r.y0 + r.y1) / 2;
            double lx = bi.a * cx + bi.c * cy + bi.tx, ly = bi.b * cx + bi.d * cy + bi.ty;
            if (std::fabs(lx - 1183.5) < 8 && std::fabs(ly - 839.8) < 8) paper.push_back(k);
        }
    }
    if (paper.empty()) return;
    {   // its pin: 26 px left of the paper's centre and 129 above
        RectF pr = stageBounds(paper.back());
        double pcx = (pr.x0 + pr.x1) / 2 - 26 * u, pcy = (pr.y0 + pr.y1) / 2 - 129 * u;
        for (DisplayObject* k : board->kids)
            if (k->kind == OK_CLIP && k->visible && near(stageBounds(k), pcx, pcy, 40 * u, 36 * u, 8 * u)) pin.push_back(k);
    }
    Mat bw = worldMatrix(board), inv = inverse(bw);
    RectF sel = stageBounds(paper.back());
    const double sx = (sel.x0 + sel.x1) / 2, sy = (sel.y0 + sel.y1) / 2;
    const bool eternal = game() == "eternal";
    // (Eternal: right of the character sheet, over the hard mode note's blank bottom, above Select!'s pin; the others:
    // where that note is in Eternal, tucked under the character sheet)
    const double tx = sx + (eternal ? 4 : -6) * u, ty = sy - (eternal ? 266 : 478) * u;
    const double k = eternal ? 0.5 : 0.6, rot = (eternal ? -5.0 : 4.0) * M_PI / 180;
    // depth: Eternal over the hard mode note's paper (566) and under its writing (654); else under the sheet (528)
    int depth = eternal ? 570 : 200, top = eternal ? 650 : 520;
    while (depth < top && board->childAtDepth(depth)) depth++;
    g_note = createEmptyMovieClip(board, intern("__marksman"), depth - DEPTH_BIAS);   // (it adds the scripts' depth bias)
    if (!g_note) return;
    g_noteBoard = board;
    // the note's matrix (board coordinates): the Select! note's centre to the new place, turned and smaller
    Mat toOrigin; toOrigin.tx = -(inv.a * sx + inv.c * sy + inv.tx); toOrigin.ty = -(inv.b * sx + inv.d * sy + inv.ty);
    Mat sc; sc.a = k * std::cos(rot); sc.b = k * std::sin(rot); sc.c = -k * std::sin(rot); sc.d = k * std::cos(rot);
    Mat back; back.tx = inv.a * tx + inv.c * ty + inv.tx; back.ty = inv.b * tx + inv.d * ty + inv.ty;
    g_note->setMatrix(mul(back, mul(sc, toOrigin))); g_note->scriptXform = true;
    int dp = 1;
    auto copy = [&](DisplayObject* src) {
        DisplayObject* d = synthPut(g_note, src->charId, dp++, src->m.tx, src->m.ty);
        if (d) { d->setMatrix(src->m); d->scriptXform = true; d->cx = src->cx; d->scriptColor = true; }
        return d;
    };
    // (the paper a little whiter than the yellow notes it lies on, and its shadow a little darker: its edges show)
    for (size_t i = 0; i < paper.size(); i++)
        if (DisplayObject* d = copy(paper[i])) {
            if (i + 1 < paper.size()) d->cx.am = std::min(1.0f, d->cx.am * 1.8f);
            else { d->cx.ra += 14; d->cx.ga += 16; d->cx.ba += 30; }
        }
    // the marks: a clip at the paper's centre (board coordinates), above the paper, under the pin
    g_noteMarks = createEmptyMovieClip(g_note, intern("marks"), dp++);
    if (g_noteMarks) {   // (in stage pixels of the Select! note: the board is drawn 2.25 times larger)
        double bs = std::sqrt(bw.a * bw.a + bw.b * bw.b);
        Mat m; m.a = m.d = bs > 0 ? u / bs : u; m.tx = inv.a * sx + inv.c * sy + inv.tx; m.ty = inv.b * sx + inv.d * sy + inv.ty;
        g_noteMarks->setMatrix(m); g_noteMarks->scriptXform = true;
    }
    for (DisplayObject* p : pin) copy(p);
}
// where each ending's mark goes on the note (paper units from its centre) and how big
struct Slot { double x, y, size; };
Slot slotFor(int e, bool og) {
    if (og) return e == E_MOM ? Slot{-84, 30, 116} : Slot{84, 30, 124};
    switch (e) {
    case E_ISAAC: return {-104, 0, 88};
    case E_HEART: return {0, -2, 106};
    case E_SATAN: return {104, 0, 88};
    case E_MOM: return {-56, 112, 92};
    default: return {56, 112, 88};
    }
}
bool seenBoss(int e) {   // the boss met in some run (or its ending reached: older files; Mom beaten: the save says so)
    load();
    if (g_seen[e]) return true;
    for (auto& [ch, m] : g_marks) if (m[e]) return true;
    if (e == E_MOM) {   // (_root.locker[4]: Mom beaten, kept in the game's save)
        Value lk = V(root(), "locker");
        if (lk.isObj() && truthy(getMem(lk, Value(4.0)))) return true;
    }
    return false;
}
void markSeen(int e) {
    load();
    if (e < 0 || g_seen[e]) return;
    g_seen[e] = true; save();
    logf("marksman: %s seen\n", kEndKey[e]);
}
void drawMarks(int ch) {
    if (!g_noteMarks) return;
    Marks m = ch >= 0 ? marksOf(ch) : Marks{};
    std::string key = std::to_string(ch);
    for (int v : m) key += std::to_string(v);
    for (int e = 0; e < E_COUNT; e++) key += seenBoss(e) ? "s" : "-";
    if (key == g_noteKey) return;
    g_noteKey = key;
    for (DisplayObject* d : std::vector<DisplayObject*>(g_noteMarks->kids)) removeObject(d);
    const bool og = isOg();
    int dp = 1;
    for (int e = 0; e < E_COUNT; e++) {
        if ((og && e > E_HEART) || !seenBoss(e)) continue;
        Slot s = slotFor(e, og);
        uint16_t id = m[e] == 2 ? symbolShape(e, (float)s.size, kHardInk, kHardFill, 1.0f)
                    : m[e] == 1 ? symbolShape(e, (float)s.size, kInk, kInkFill, 1.0f)
                                : symbolShape(e, (float)s.size, kGhost, 0, 0.8f);
        synthPut(g_noteMarks, id, dp++, s.x, s.y);
    }
}
int selectedCharacter() {   // the character the character select shows (game order; 7 The Gone), -1 if unknown
    if (goneSelected()) return kGone;
    double sk = num(V(root(), "skiner"), -1);
    return sk >= 0 && sk < 7 ? (int)sk : -1;
}
void noteTick() {
    Value b = V(V(root(), "men"), "men");
    if (!b.isObj() || b.o->kind != OK_CLIP) { if (g_note && (g_note->removed || !g_note->parent)) removeNote(); g_note = nullptr; g_noteMarks = nullptr; g_noteBoard = nullptr; return; }
    MovieClip* board = (MovieClip*)b.o;
    if (!g_note || g_note->removed || g_note->parent != board || g_noteBoard != board) {
        // (built when the board is at the character select: the Select! note is found by its place on the stage)
        if (num(V(board, "pos")) != 3) return;
        buildNote(board);
        if (!g_note) return;
    }
    drawMarks(selectedCharacter());
}

// ============================================================ the challenge's goal in the HUD
MovieClip* g_goal = nullptr; int g_goalShown = -1;
void removeGoal() { if (g_goal && !g_goal->removed && g_goal->parent) removeObject(g_goal); g_goal = nullptr; g_goalShown = -1; }
int challengeGoal() {   // the goal of the challenge being played (an Ending), -1 when it isn't one
    if (clabRunActive()) return clabGoal();
    int chala = (int)num(V(root(), "chala"));
    if (chala >= 1 && chala <= 4) return E_MOM;
    if (chala >= 5 && chala <= 8) return E_HEART;
    if (chala == 9) return E_SATAN;    // (after Mom's Heart only the trapdoor: Sheol)
    if (chala == 10) return E_ISAAC;   // (only the chest: the beam to the Cathedral)
    return -1;
}
void goalTick(bool inGame) {
    Value hud = V(root(), "hud");
    int goal = inGame ? challengeGoal() : -1;
    if (goal < 0 || !hud.isObj() || hud.o->kind != OK_CLIP) { if (g_goal) g_goal->visible = false; return; }
    MovieClip* h = (MovieClip*)hud.o;
    if (!g_goal || g_goal->removed || g_goal->parent != h || g_goalShown != goal) {
        removeGoal();
        g_goal = createEmptyMovieClip(h, intern("__marksman"), 1047950);
        if (!g_goal) return;
        g_goalShown = goal;
        int dp = 1;
        synthPut(g_goal, symbolShape(goal, 21, 0xd0000000u, 0xd0000000u, 1.15f), dp++, 1.2, 1.2);   // (shadow)
        synthPut(g_goal, symbolShape(goal, 21, 0xfff4f0e8u, 0x60f4f0e8u, 1.15f), dp++, 0, 0);
    }
    // where: in the HUD band right of the map; with WideSaac's Hyper Zoom (no band) on the room's top left wall
    bool hyper = wideMinimapAvailable();
    // (the symbol's centre: in the HUD band, left of the map under BaseTime's time; with Hyper Zoom (no band) on the
    // view's top left corner, above StatOn's column)
    Mat m; m.tx = hyper ? 24 : 16; m.ty = hyper ? 90 : 50;
    g_goal->setMatrix(m); g_goal->scriptXform = true;
    g_goal->cx.am = 0.85f;
    g_goal->visible = true;
}

// ============================================================ the endings
int s_bossRoom = -1; bool s_sawShut = false; int s_lastRootFrame = 0;
bool s_testGenesis = false;   // (tests: "eval marks genesis" lets GenesisConsole's runs count)
int endingOfFloor() {   // the ending whose last boss is on this floor, -1 if none
    Value R = root();
    int chaps = (int)num(V(R, "chaps"));
    if (chaps == 6) return E_MOM;
    if (chaps == 8) return E_HEART;
    if (isOg()) return -1;
    if (chaps == 9) return truthy(V(R, "altchap")) ? E_ISAAC : E_SATAN;
    if (chaps == 11) return E_CHEST;
    return -1;
}
int runCharacter() {
    if (goneRunActive()) return kGone;
    double sk = num(V(root(), "skiner"), -1);
    return sk >= 0 && sk < 7 ? (int)sk : -1;
}
bool counts() {   // a normal run (marks are for those only)
    if ((genesisActive() && !s_testGenesis) || clabRunActive()) return false;
    return num(V(root(), "chala")) == 0;
}
void endingTick(bool inGame) {
    Value R = root(), a = V(R, "a");
    int frame = vm.root->kind == OK_CLIP ? ((MovieClip*)vm.root)->cur : 0;
    int level = game() == "eternal" && truthy(V(R, "hardmode")) ? 2 : 1;
    // an ending shown (_root frames 9-40)
    if (frame >= 9 && frame <= 40 && !(s_lastRootFrame >= 9 && s_lastRootFrame <= 40) && counts()) {
        int e = endingOfFloor();
        if (e >= 0) addMark(runCharacter(), e, level);
    }
    s_lastRootFrame = frame;
    if (!inGame || !a.isObj()) { s_bossRoom = -1; return; }
    // the floor's last boss beaten: its room cleared after it was seen shut
    int lev = (int)num(V(R, "lev"), -1);
    if (lev != s_bossRoom) { s_bossRoom = lev; s_sawShut = false; }
    int e = endingOfFloor();
    if (e < 0 || lev != (int)num(V(R, "bossl"), -2)) return;
    if (!genesisActive() || s_testGenesis) markSeen(e);   // (any run: the challenges' too)
    bool clear = num(V(a, "shutdoor"), 1) == 0 && num(V(a, "ashut"), 1) == 0;
    if (!clear) s_sawShut = true;
    double md = num(V(a, "momdown"));
    if (((s_sawShut && clear) || (e == E_MOM && md > 0)) && counts()) addMark(runCharacter(), e, level);
}
}  // namespace

void marksmanDebug(const std::string& cmd) {
    if (cmd == "state") {
        load();
        logf("[marks] file %s, selected %d, run character %d, floor ending %d, goal %d\n", file().c_str(), selectedCharacter(), runCharacter(), endingOfFloor(), challengeGoal());
        { Value R = root(), a = V(R, "a"); logf("[marks] chaps %g lev %g bossl %g shutdoor %g ashut %g momdown %g sawShut %d counts %d frame %d\n", num(V(R, "chaps")), num(V(R, "lev")), num(V(R, "bossl")), num(V(a, "shutdoor"), -1), num(V(a, "ashut"), -1), num(V(a, "momdown")), (int)s_sawShut, (int)counts(), vm.root->kind == OK_CLIP ? ((MovieClip*)vm.root)->cur : 0); }
        for (auto& [ch, m] : g_marks) logf("[marks]   %s: %d %d %d %d %d\n", kCharKey[ch], m[0], m[1], m[2], m[3], m[4]);
        if (g_note) for (DisplayObject* k : g_note->kids) { RectF r = stageBounds(k); logf("[marks]   kid %d id %d m %.3f %.3f %.3f %.3f %.1f %.1f am %.2f vis %d: %.0f,%.0f-%.0f,%.0f\n", k->depth, k->charId, k->m.a, k->m.b, k->m.c, k->m.d, k->m.tx, k->m.ty, k->cx.am, k->visible, r.x0, r.y0, r.x1, r.y1); }
        if (g_note) logf("[marks] note m %.3f %.3f %.3f %.3f %.1f %.1f\n", g_note->m.a, g_note->m.b, g_note->m.c, g_note->m.d, g_note->m.tx, g_note->m.ty);
        if (g_note) { RectF r = stageBounds(g_note); logf("[marks] note at %.0f,%.0f-%.0f,%.0f depth %d\n", r.x0, r.y0, r.x1, r.y1, g_note->depth); }
        return;
    }
    if (cmd.compare(0, 3, "at ") == 0) {   // at <x> <y>: the board's children over that point of the stage
        double x = 0, y = 0; sscanf(cmd.c_str() + 3, "%lf %lf", &x, &y);
        std::function<void(DisplayObject*, std::string, int)> walk = [&](DisplayObject* d, std::string path, int lv) {
            RectF r = stageBounds(d);
            if (!(r.valid() && x >= r.x0 && x <= r.x1 && y >= r.y0 && y <= r.y1) || !d->visible) return;
            path += "/" + std::to_string(d->depth) + (d->name ? ":" + d->name->s : "") + "#" + std::to_string(d->charId);
            logf("[marks] at: %s %.0f,%.0f-%.0f,%.0f\n", path.c_str(), r.x0, r.y0, r.x1, r.y1);
            if (d->kind == OK_CLIP && lv < 4) for (DisplayObject* k : ((MovieClip*)d)->kids) walk(k, path, lv + 1);
        };
        walk(vm.root, "", 0);
        return;
    }
    if (cmd == "genesis") { s_testGenesis = true; return; }
    if (cmd.compare(0, 4, "add ") == 0) {   // add <character 0-7> <ending 0-4> <1|2>
        int ch = 0, e = 0, l = 1;
        if (sscanf(cmd.c_str() + 4, "%d %d %d", &ch, &e, &l) == 3) addMark(ch, e, l);
        return;
    }
}

void marksmanTick(const ModInfo& mod) {
    static bool rooted = false;
    if (!rooted) { rooted = true; gcAddRoot((GcObj**)&g_note); gcAddRoot((GcObj**)&g_noteMarks); gcAddRoot((GcObj**)&g_goal); gcAddRoot((GcObj**)&g_noteBoard); }
    bool inGame = false;
    recompInGameplay(&inGame);
    auto on = [&](const char* k) { std::string v = mod.opt(k, "1"); return !(v == "0" || v == "no" || v == "off" || v == "false"); };
    endingTick(inGame);
    if (on("note")) noteTick(); else removeNote();
    if (on("challenge_goal")) goalTick(inGame); else removeGoal();
}
}  // namespace fl
