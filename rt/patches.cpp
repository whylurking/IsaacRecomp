// Recomp-specific tweaks to the original game (not part of the SWF):
//  - "mdm" object (Steam's Zinc wrapper): quitting the game and fullscreen actually work;
//  - options menu without the options that depended on Zinc/Flash and do nothing here.
#include "display.h"
#include "mods.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <atomic>

namespace fl {

// ---------------------------------------------------------------- subpaths of a shape
struct SubPath { size_t v0, v1, p0, p1; float x0, y0, x1, y1; };
static std::vector<SubPath> splitPath(const Path& p) {
    std::vector<SubPath> out; size_t k = 0;
    for (size_t i = 0; i < p.verbs.size(); i++) {
        uint8_t v = p.verbs[i];
        if (v == 0) { if (!out.empty()) { out.back().v1 = i; out.back().p1 = k; } out.push_back({i, 0, k, 0, 1e9f, 1e9f, -1e9f, -1e9f}); }
        int n = v == 2 ? 4 : 2;
        if (!out.empty()) for (int j = 0; j < n; j += 2) {
            SubPath& s = out.back(); float x = p.pts[k + j], y = p.pts[k + j + 1];
            s.x0 = std::min(s.x0, x); s.y0 = std::min(s.y0, y); s.x1 = std::max(s.x1, x); s.y1 = std::max(s.y1, y);
        }
        k += n;
    }
    if (!out.empty()) { out.back().v1 = p.verbs.size(); out.back().p1 = k; }
    return out;
}
void debugShapeSubpaths(uint16_t id) {
    CharDef* cd = g_movie.get(id); if (!cd || cd->type != CH_SHAPE) return;
    ShapeDef* s = (ShapeDef*)cd; shapeEnsure(s); s->pinned = true;   // (lean loading: patched shapes stay parsed)
    if (getenv("ISAAC_POLYDUMP")) {   // (testing) the hit-test polygons, for analysis
        for (auto& poly : shapeHitPolys(s)) { std::string o = "[poly]"; for (float v : poly) { char b[32]; snprintf(b, sizeof b, " %g", v); o += b; } logf("%s\n", o.c_str()); }
    }
    for (size_t l = 0; l < s->layers.size(); l++) for (size_t f = 0; f < s->layers[l].fillPaths.size(); f++)
        for (auto& sp : splitPath(s->layers[l].fillPaths[f]))
            logf("[sub] layer %zu fill %zu  x %.1f..%.1f  y %.1f..%.1f\n", l, f, sp.x0, sp.x1, sp.y0, sp.y1);
}

// ---------------------------------------------------------------- mdm object (Zinc)
std::atomic<int> g_fullscreenRequest{-1};   // -1 none; 0/1 = the game requests windowed / fullscreen
std::atomic<bool> g_isFullscreen{false};    // actual state (updated by the main thread)
Value mdmExit(Act&, Value, int, Value*) { g_player.quit = true; return VUNDEF; }
// "Delete save" (Collection) deletes the save and, in the Steam version, also its "serial.txt" copy. The
// already-computed stats, collection and character screens keep showing the old data until a restart,
// so after deleting, the game is restarted from scratch (main.cpp: g_restartRequest).
std::atomic<bool> g_restartRequest{false};
Value mdmDeleteFile(Act&, Value, int argc, Value* argv) {
    if (argc > 0 && toStr(argv[0])->s.find("serial.txt") != std::string::npos) {
        logf("save deleted: restarting the game\n");
        g_restartRequest = true; g_player.quit = true;
    }
    return VUNDEF;
}
static bool g_fsSynced = false;

// ---- analog movement ("joystick_acceleration"): the three games read movement in dirkey(false), which turns the
// pressed keys into xenf/yenf = -1/0/+1; that value then accelerates the player. Our native wrapper calls the
// original and scales xenf/yenf by g_moveScale (set by the input code from how far the stick is pushed): less push,
// less acceleration and a lower top speed; 1 = exactly the original movement (keyboard, D-pad, stick fully out).
std::atomic<float> g_moveScale{1.f};
bool g_joyAccel = true;
static Function* g_dirkeyOrig = nullptr;
static Function* g_dirkeyWrap = nullptr;
static Value dirkeyWrap(Act& a, Value thisv, int argc, Value* argv) {
    Value r = g_dirkeyOrig ? callValue(a, Value((Object*)g_dirkeyOrig), thisv, argc, argv) : VUNDEF;
    float k = g_moveScale.load();
    if (k < 0.999f && !(argc > 0 && truthy(argv[0]))) {   // dirkey(false) = movement (true = shooting)
        Value clip = getMemK(Value((Object*)vm.root), intern("a"));
        if (clip.isObj()) {
            static Str* sx = intern("xenf"); static Str* sy = intern("yenf");
            Value x = getMemK(clip, sx), y = getMemK(clip, sy);
            if (x.t == T_NUM && x.n != 0) setMemK(clip, sx, Value(x.n * k));
            if (y.t == T_NUM && y.n != 0) setMemK(clip, sy, Value(y.n * k));
        }
    }
    return r;
}
static void installMoveScale() {
    if (!g_joyAccel) return;
    static const char* test = getenv("ISAAC_MOVESCALE"); if (test) g_moveScale = (float)atof(test);   // (testing)
    Value clip = getMemK(Value((Object*)vm.root), intern("a"));
    if (!clip.isObj()) return;
    static Str* name = intern("dirkey");
    Value f = getMemK(clip, name);
    if (!f.isObj() || f.o->kind != OK_FUNCTION || f.o == (Object*)g_dirkeyWrap) return;
    if (!g_dirkeyWrap) {
        g_dirkeyWrap = gcNew<Function>(); g_dirkeyWrap->proto = protos::function; g_dirkeyWrap->nat = dirkeyWrap; g_dirkeyWrap->dbgName = "dirkey (analog)"; gcPin(g_dirkeyWrap);
        gcAddRoot((GcObj**)&g_dirkeyOrig);
    }
    g_dirkeyOrig = (Function*)f.o;
    setMemK(clip, name, Value((Object*)g_dirkeyWrap));
}
// ---- boss music: the music code (_root.soundfra) switches from the boss theme to the "boss defeated" jingle the
// first frame the room has no enemies left (_root.a.ashut == 0), and does it only once per room (bossmusic = 2).
// A boss that splits when it dies (Duke of Flies, Lord of the Flies, ...) leaves the room empty for one frame
// before its spawn is counted: the theme faded out and started again, and when the spawn died no jingle played.
// The wrapper hides that gap from the music code: the room must stay empty for a few frames first.
static Function* g_soundfraOrig = nullptr;
static Function* g_soundfraWrap = nullptr;
static int g_emptyFrames = 0;
static Value soundfraWrap(Act& a, Value thisv, int argc, Value* argv) {
    Value root((Object*)vm.root);
    static Str* sA = intern("a"); static Str* sShut = intern("ashut"); static Str* sBoss = intern("bossmusic");
    Value clip = getMemK(root, sA), shut = clip.isObj() ? getMemK(clip, sShut) : VUNDEF;
    bool empty = shut.t == T_NUM && shut.n == 0;
    g_emptyFrames = empty ? g_emptyFrames + 1 : 0;
    Value bm = getMemK(root, sBoss);
    bool hold = empty && g_emptyFrames < 3 && truthy(bm) && !(bm.t == T_NUM && bm.n == 2);
    if (hold) setMemK(clip, sShut, Value(1.0));
    Value r = g_soundfraOrig ? callValue(a, Value((Object*)g_soundfraOrig), thisv, argc, argv) : VUNDEF;
    if (hold) { Value now = getMemK(clip, sShut); if (now.t == T_NUM && now.n == 1) setMemK(clip, sShut, Value(0.0)); }
    return r;
}
static void installBossMusic() {
    static Str* name = intern("soundfra");
    Value root((Object*)vm.root);
    Value f = getMemK(root, name);
    if (!f.isObj() || f.o->kind != OK_FUNCTION || f.o == (Object*)g_soundfraWrap) return;
    if (!g_soundfraWrap) {
        g_soundfraWrap = gcNew<Function>(); g_soundfraWrap->proto = protos::function; g_soundfraWrap->nat = soundfraWrap; g_soundfraWrap->dbgName = "soundfra (boss music)"; gcPin(g_soundfraWrap);
        gcAddRoot((GcObj**)&g_soundfraOrig);
    }
    g_soundfraOrig = (Function*)f.o;
    setMemK(root, name, Value((Object*)g_soundfraWrap));
}
// (testing) ISAAC_SPLATLOG: who calls the game's splater() (blood drawn into the floor's bitmap)
static Function* g_splOrig = nullptr; static Function* g_splWrap = nullptr;
static Value splWrap(Act& a, Value thisv, int argc, Value* argv) {
    Value clip = getMemK(Value((Object*)vm.root), intern("a")), trg = getMemK(clip, intern("trg"));
    static const bool skip = getenv("ISAAC_SPLATSKIP") != nullptr; if (skip) return VUNDEF;
    logf("[splat] tick %u trg.s=%g trg.spl=%g xp=%g yp=%g\n", g_player.tick, toNum(getMemK(trg, intern("s"))), toNum(getMemK(trg, intern("spl"))), toNum(getMemK(trg, intern("xp"))), toNum(getMemK(trg, intern("yp"))));
    return g_splOrig ? callValue(a, Value((Object*)g_splOrig), thisv, argc, argv) : VUNDEF;
}
static void installSplatLog() {
    static const bool on = getenv("ISAAC_SPLATLOG") || getenv("ISAAC_SPLATSKIP"); if (!on) return;
    Value clip = getMemK(Value((Object*)vm.root), intern("a")); if (!clip.isObj()) return;
    Value f = getMemK(clip, intern("splater"));
    if (!f.isObj() || f.o->kind != OK_FUNCTION || f.o == (Object*)g_splWrap) return;
    if (!g_splWrap) { g_splWrap = gcNew<Function>(); g_splWrap->proto = protos::function; g_splWrap->nat = splWrap; gcPin(g_splWrap); gcAddRoot((GcObj**)&g_splOrig); }
    g_splOrig = (Function*)f.o; setMemK(clip, intern("splater"), Value((Object*)g_splWrap));
}
Value mdmFull(Act&, Value, int argc, Value* argv) {
    // at startup the game applies its saved setting: isaac.ini's takes precedence (and is synced in recompPatchesTick)
    if (g_fsSynced) { g_fullscreenRequest = argc > 0 && truthy(argv[0]) ? 1 : 0; if (g_trace) logf("[mdm] showFullScreen(%d)\n", g_fullscreenRequest.load()); }
    return VUNDEF;
}

// ---------------------------------------------------------------- per-game ids
// All three versions share these graphics (same shapes, different ids): options menu (sprite), shape with
// the labels, shape with the fullscreen note, and controls screen.
struct GameIds { const char* tag; uint16_t optSprite, optLabels, optNote, controls; };
static const GameIds kIds[] = {
    {"wotl",    1374, 1258, 1253, 1337},
    {"og",       944,  863,  858,  942},
    {"eternal", 1422, 1306, 1301, 1385},
};
static const GameIds* ids() {
    if (!g_game) return nullptr;
    for (auto& g : kIds) if (!strcmp(g.tag, g_game->tag)) return &g;
    return nullptr;
}

// ---------------------------------------------------------------- options menu
// Options sprite (1374 in WotL), frame 5. Shape 1258 contains all the labels; "Detail", "FullScreen
// Resolutions" and "Window size" are removed and the remaining rows are moved up. Sprite-local coordinates.
static const float kFullDy = -60, kMuteDy = -40;
static void patchOptionsShape() {
    CharDef* cd = g_movie.get(ids()->optLabels); if (!cd || cd->type != CH_SHAPE) return;
    ShapeDef* s = (ShapeDef*)cd; shapeEnsure(s); s->pinned = true;   // (lean loading: patched shapes stay parsed)
    for (auto& L : s->layers) for (auto& P : L.fillPaths) {
        if (P.empty()) continue;
        Path out;
        for (auto& sp : splitPath(P)) {
            float dy = 0;
            if (sp.y1 < 95) continue;                                  // Detail / FullScreen Resolutions
            if (sp.y0 >= 194 && sp.y1 <= 218.5f) continue;              // Window size
            if (sp.y0 >= 150 && sp.y1 <= 194) dy = kFullDy;             // FullScreen:
            else if (sp.y0 >= 218.5f) dy = kMuteDy;                     // Mute: / Music:
            out.verbs.insert(out.verbs.end(), P.verbs.begin() + sp.v0, P.verbs.begin() + sp.v1);
            for (size_t k = sp.p0; k < sp.p1; k += 2) { out.pts.push_back(P.pts[k]); out.pts.push_back(P.pts[k + 1] + dy); }
        }
        P = std::move(out);
    }
    s->hitPolys.clear(); s->hitBuilt = false;
}
// the note "(press F anytime to change to full)" is in shape 1253 (fill 2): it moves up with its row
static void patchOptionsNote() {
    CharDef* cd = g_movie.get(ids()->optNote); if (!cd || cd->type != CH_SHAPE) return;
    ShapeDef* s = (ShapeDef*)cd; shapeEnsure(s); s->pinned = true;   // (lean loading: patched shapes stay parsed)
    if (s->layers.empty() || s->layers[0].fillPaths.size() < 3) return;
    Path& P = s->layers[0].fillPaths[2];
    for (auto& sp : splitPath(P))
        if (sp.x0 >= 270 && sp.y0 >= 140 && sp.y1 <= 192)
            for (size_t k = sp.p0; k < sp.p1; k += 2) P.pts[k + 1] += kFullDy;
    s->hitPolys.clear(); s->hitBuilt = false;
}
// controls screen (shape 1337): "Game pad?: use JoyToKey (google it!)" no longer applies, the recomp
// supports controllers directly
static void patchControlsShape() {
    CharDef* cd = g_movie.get(ids()->controls); if (!cd || cd->type != CH_SHAPE) return;
    ShapeDef* s = (ShapeDef*)cd; shapeEnsure(s); s->pinned = true;   // (lean loading: patched shapes stay parsed)
    for (size_t l = 0; l < s->layers.size() && l < 2; l++) for (auto& P : s->layers[l].fillPaths) {
        if (P.empty()) continue;
        Path out;
        for (auto& sp : splitPath(P)) {
            if (l == 0 && sp.y0 >= 190) continue;
            if (l == 1 && sp.y0 >= 228 && sp.x0 >= 205) continue;
            out.verbs.insert(out.verbs.end(), P.verbs.begin() + sp.v0, P.verbs.begin() + sp.v1);
            out.pts.insert(out.pts.end(), P.pts.begin() + sp.p0, P.pts.begin() + sp.p1);
        }
        P = std::move(out);
    }
    s->hitPolys.clear(); s->hitBuilt = false;
}
void applyLoadPatches() { if (ids()) { patchOptionsShape(); patchOptionsNote(); patchControlsShape(); } }

static void setY(DisplayObject* d, double y) { if (d && std::fabs(d->m.ty - y) > 1e-6) { d->m.ty = y; d->scriptXform = true; } }
static void tweakOptions(MovieClip* p) {
    const GameIds* g = ids();
    if (!g || !p || p->kind != OK_CLIP || !p->def || p->def->id != g->optSprite || p->cur != 5) return;
    for (DisplayObject* k : p->kids) {
        const std::string n = k->name ? k->name->s : std::string();
        if ((n.size() == 3 && n[0] == 'b' && n[1] == 'q') || (n.size() == 2 && n[0] == 'r' && n[1] >= '0' && n[1] <= '9') ||
            (n.size() == 3 && n[0] == 'w' && n[1] == 'q')) k->visible = false;
        else if (n == "b0") setY(k, 158.8 + kFullDy);
        else if (n == "b1") setY(k, 227.6 + kMuteDy);
        else if (n == "b2") setY(k, 224.6 + kMuteDy);
    }
}

// GenesisConsole open (the game paused): the game's music code alone keeps running, so a song that ends starts again
// (or the next one plays) as usual instead of leaving silence until the console is closed
void recompMusicTick() {
    Value root((Object*)vm.root);
    Value f = getMemK(root, intern("soundfra"));
    if (!f.isObj() || f.o->kind != OK_FUNCTION) return;
    Act ac{vm.globalScope, VUNDEF, vm.root, nullptr};
    callValue(ac, f, root, 0, nullptr);
    runActions();
}
// every tick, after the logic
// Character select: a character must be unlocked to start a run, but the game's Select / Space checks
// _root.skiner, which the ring only updates on its own frame. Moving onto a locked character and pressing Select in
// the next instant passed the check with the previous character and started the run with the locked one. When the
// run starts (darky(150): men.men.pos 4) the ring is put back on the character that was checked (Random, cha.done,
// is left alone: it only picks unlocked ones).
static double s_csRoot = 0, s_csCha = 0, s_csPrevCha = 1e9; static int s_csPos = -1;
static void charSelectGuard(const Value& root) {
    Value sheet = getMemK(getMemK(root, intern("men")), intern("men"));
    if (!sheet.isObj() || sheet.o->kind != OK_CLIP) { s_csPos = -1; return; }
    Value cha = getMemK(sheet, intern("cha"));
    if (!cha.isObj() || cha.o->kind != OK_CLIP) {   // (the original game: the unnamed ring, with i0 and inc)
        cha = VUNDEF;
        for (DisplayObject* k : ((MovieClip*)sheet.o)->kids)
            if (k->kind == OK_CLIP && ((MovieClip*)k)->childByName(intern("i0")) && ((MovieClip*)k)->childByName(intern("inc"))) { cha = Value((Object*)k); break; }
        if (!cha.isObj()) { s_csPos = -1; return; }
    }
    int pos = (int)toNum(getMemK(sheet, intern("pos")));
    double c = toNum(getMemK(cha, intern("skiner")));
    // (a settled pair: the ring's index unchanged since the last tick, so _root.skiner is the one it shows)
    if (pos == 3 && c == s_csPrevCha) { s_csRoot = toNum(getMemK(root, intern("skiner"))); s_csCha = c; }
    s_csPrevCha = c;
    if (pos == 4 && s_csPos >= 3 && !truthy(getMemK(cha, intern("done")))) {
        if (c != s_csCha || toNum(getMemK(root, intern("skiner"))) != s_csRoot) {
            setMemK(cha, intern("skiner"), Value(s_csCha));
            setMemK(root, intern("skiner"), Value(s_csRoot));
            if (s_csPos == 3) logf("character select: the run starts with the character that was checked (%g)\n", s_csRoot);
        }
        s_csPos = 4;
        return;
    }
    s_csPos = pos;
}

void recompPatchesTick() {
    Value root((Object*)vm.root);
    charSelectGuard(root);
    installMoveScale();
    installBossMusic();
    installSplatLog();
    if (!g_fsSynced && g_player.tick >= 2) {   // the menu's "FullScreen" checkbox reflects the actual state
        Value so = getMemK(root, intern("so"));
        Value data = so.isObj() ? getMemK(so, intern("data")) : VUNDEF;
        if (data.isObj()) { setMemK(data, intern("full"), Value::boolean(g_isFullscreen.load())); g_fsSynced = true; }
    }
    Value paus = getMemK(root, intern("paus"));
    if (paus.isObj() && paus.o->kind == OK_CLIP) tweakOptions((MovieClip*)paus.o);
    // Eternal Edition: the HUD's hard-mode mark is set with `hud.hardon._visible = _root.hardmode`. With a save
    // that never toggled hard mode, hardmode is undefined; in this runtime (as in Ruffle) assigning undefined to
    // _visible changes nothing, so the mark stayed visible. Hide it whenever hard mode is not on.
    if (g_game && !strcmp(g_game->tag, "eternal")) {
        Value hud = getMemK(root, intern("hud"));
        Value mark = hud.isObj() ? getMemK(hud, intern("hardon")) : VUNDEF;
        if (mark.isObj() && mark.o->isDisplay() && !truthy(getMemK(root, intern("hardmode")))) ((DisplayObject*)mark.o)->visible = false;
    }
}
// actively playing: _root.a is the game clip and it is not paused (unpause = true)
bool recompInGameplay(bool* inGame) {
    Value a = getMemK(Value((Object*)vm.root), intern("a"));
    bool g = a.isObj() && getMemK(a, intern("player")).isObj();
    if (inGame) *inGame = g;
    return g && truthy(getMemK(a, intern("unpause")));
}
// Touch screens: where the HUD shows the bomb counter, the active item and the pill/card (stage
// coordinates, a little enlarged for fingers). Bit i of the result = target i is on screen. The three games share
// the HUD layout: hud.cacher (coins / bombs / keys, one row each; Eternal only, the other two have the same icons
// directly in hud), hud.it ("SPACE" box) and hud.extra (only visible while holding a pill or card).
int recompTouchTargets(float out[4][4]) {
    Value hud = getMemK(Value((Object*)vm.root), intern("hud"));
    if (!hud.isObj() || !hud.o->isDisplay() || !((DisplayObject*)hud.o)->visible) return 0;
    int mask = 0;
    auto rect = [&](const char* name, bool needVisible, RectF& r) {
        Value v = getMemK(hud, intern(name));
        if (!v.isObj() || !v.o->isDisplay()) return false;
        DisplayObject* d = (DisplayObject*)v.o;
        if (needVisible && !d->visible) return false;
        r = stageBounds(d);
        return r.valid();
    };
    RectF r; double pad = g_stageW / 120.0;   // (~1% of the stage width)
    // WideSaac's Hyper Zoom draws some HUD pieces elsewhere: their touch areas follow (stage -> HUD -> moved -> stage)
    Mat hudW = worldMatrix((DisplayObject*)hud.o), hudInv; hudW.invert(hudInv);
    auto moved = [&](int group, RectF& rr) {
        Mat mv; if (!wideHudMove(group, mv)) return;
        Mat t = hudW * mv * hudInv;
        double xs[2] = {rr.x0, rr.x1}, ys[2] = {rr.y0, rr.y1}; RectF o; bool first = true;
        for (double x : xs) for (double y : ys) {
            double px, py; t.apply(x, y, px, py);
            if (first) { o.x0 = o.x1 = px; o.y0 = o.y1 = py; first = false; }
            else { o.x0 = std::min(o.x0, px); o.x1 = std::max(o.x1, px); o.y0 = std::min(o.y0, py); o.y1 = std::max(o.y1, py); }
        }
        rr = o;
    };
    bool cacher = rect("cacher", false, r);
    if (!cacher) {   // the original and WotL have the same three icons loose in the HUD instead of in hud.cacher: the
                     // area Eternal's cacher covers (HUD coordinates), so the bomb row is found the same way
        Mat w = worldMatrix((DisplayObject*)hud.o);
        double xs[2] = {249.8, 279.4}, ys[2] = {11.1, 66.3};
        r = RectF();
        for (double x : xs) for (double y : ys) {
            double px, py; w.apply(x, y, px, py);
            if (!cacher) { r.x0 = r.x1 = px; r.y0 = r.y1 = py; cacher = true; }
            else { r.x0 = std::min(r.x0, px); r.x1 = std::max(r.x1, px); r.y0 = std::min(r.y0, py); r.y1 = std::max(r.y1, py); }
        }
    }
    if (cacher) moved(WIDE_COUNT, r);
    if (cacher) {   // middle row; the count text sits to the right of the icon
        double h = (r.y1 - r.y0) / 3, w = r.x1 - r.x0;
        out[0][0] = (float)(r.x0 - pad); out[0][1] = (float)(r.y0 + h - pad * 0.3);
        out[0][2] = (float)(r.x0 + w * 2.8 + pad); out[0][3] = (float)(r.y0 + 2 * h + pad * 0.3); mask |= 1;
    }
    if (rect("it", false, r)) { moved(WIDE_ACTIVE, r); out[1][0] = (float)(r.x0 - pad); out[1][1] = (float)(r.y0 - pad); out[1][2] = (float)(r.x1 + pad); out[1][3] = (float)(r.y1 + pad); mask |= 2; }
    if (rect("extra", true, r)) { moved(WIDE_POCKET, r); out[2][0] = (float)(r.x0 - pad); out[2][1] = (float)(r.y0 - pad); out[2][2] = (float)(r.x1 + pad); out[2][3] = (float)(r.y1 + pad); mask |= 4; }
    // pause button (drawn by the renderer, not part of the game): in the empty corner right of "-LIFE-",
    // 28 units square in the game's 640x480 space, mapped through _root's matrix (HD scale and offset)
    {
        double rx0 = 588, ry0 = 4, rx1 = 616, ry1 = 32, x0, y0, x1, y1;
        vm.root->m.apply(rx0, ry0, x0, y0); vm.root->m.apply(rx1, ry1, x1, y1);
        RectF pr; pr.x0 = std::min(x0, x1); pr.y0 = std::min(y0, y1); pr.x1 = std::max(x0, x1); pr.y1 = std::max(y0, y1);
        moved(WIDE_LIFE, pr); x0 = pr.x0; y0 = pr.y0; x1 = pr.x1; y1 = pr.y1;
        out[3][0] = (float)x0; out[3][1] = (float)y0; out[3][2] = (float)x1; out[3][3] = (float)y1; mask |= 8;
    }
    return mask;
}
// the player toggled fullscreen with F11 / Alt+Enter (logic thread)
void recompFullscreenChanged(bool full) {
    Value root((Object*)vm.root);
    Value so = getMemK(root, intern("so"));
    Value data = so.isObj() ? getMemK(so, intern("data")) : VUNDEF;
    if (data.isObj()) setMemK(data, intern("full"), Value::boolean(full));
    Value paus = getMemK(root, intern("paus"));
    if (paus.isObj() && paus.o->kind == OK_CLIP && ((MovieClip*)paus.o)->cur == 5) {
        Act ac{vm.globalScope, VUNDEF, vm.root, nullptr};
        callMethodK(ac, root, intern("showit"), 0, nullptr);
        runActions();
    }
}

}  // namespace fl
