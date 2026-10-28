// WideSaac (mods/WideSaac.mod): the whole screen on any aspect ratio.
//
// The game is a 4:3 stage. Without the mod, the renderer letterboxes it (black bars). With WideSaac the stage is
// still drawn as large as the screen allows, but without cutting what lies around it: the game's own background art
// (the tiled faces behind every screen) fills the sides of a wide screen, or above and below a tall one.
//
// Hyper Zoom (option hyper_zoom = 1): during play, the view is the room alone (the HUD band above it is left out), as
// large as the screen allows, so the room fills a 16:9 / 16:10 screen. The map is gone; hearts, the active item with
// its charges, coins / bombs / keys and the pill / card are the game's own HUD pieces, drawn elsewhere: on the
// room's right side, like StatOn on the left. It stays on during the whole run (paused and dead too); boss intros, floor
// cutscenes and endings keep the room's view, fitted into it with black sides (bossIntro); an open GenesisConsole
// shows the normal screen. Screen-wide shading covers the whole window (stretchShades).
//
// Minimap+ (Hyper Zoom only): M, L3 on a controller or a four-finger tap shows / hides the game's own map, larger and
// see-through, in the middle of the room, over a dark backdrop sized to the rooms the map shows. The map is the game's
// _root.mapa (a bitmap the game redraws when the floor changes, centred on (130, 40)): only where it is drawn changes.
//
// Nothing here touches the game: the view (snapshot.view) and the moved / hidden HUD pieces (g_snapOv) only change
// what the renderer draws, so runs play exactly the same with the mod on.
#include "mods.h"
#include "core.h"
#include "display.h"
#include "snapshot.h"
#include "swfdata.h"
#include <algorithm>
#include <unordered_map>
#include <atomic>
#include <cmath>
#include <cstring>

namespace fl {
bool recompInGameplay(bool* inGame);

namespace {
Value V(const Value& o, const char* k) { return o.isObj() ? getMemK(o, intern(k)) : VUNDEF; }
const double kRoomTop = 77.5;   // where the room starts under the HUD band (_root units: the game's 640x480)
Mat g_groupMove[WIDE_GROUPS];  // HUD-local move of each group this tick
bool g_groupOn[WIDE_GROUPS] = {};
std::atomic<bool> s_hyperNow{false}, s_mapWanted{false};
bool s_runSeen = false;   // Hyper Zoom was on in this run (until the menu)
// Minimap+: the backdrop (a clip in the HUD, under _root.mapa) and the fade
MovieClip* g_mapClip = nullptr; DisplayObject* g_mapBack = nullptr; DisplayObject* g_mapEdge[4] = {};
float g_mapFade = 0;
void removeMapClip() {
    if (g_mapClip && !g_mapClip->removed && g_mapClip->parent) removeObject(g_mapClip);
    g_mapClip = nullptr; g_mapBack = nullptr; for (auto& e : g_mapEdge) e = nullptr;
}
// the part of the map the game draws: its map code keeps the extremes of the room points it drew (map units) in
// _root.a.minx / maxxx / miny / maxy, and centres the map on them at (130, 40)
bool mapExtent(const Value& root, double& x0, double& y0, double& x1, double& y1) {
    Value a = V(root, "a");
    x0 = toNum(V(a, "minx")); x1 = toNum(V(a, "maxxx")); y0 = toNum(V(a, "miny")); y1 = toNum(V(a, "maxy"));
    return std::isfinite(x0) && std::isfinite(x1) && std::isfinite(y0) && std::isfinite(y1) && x1 >= x0 && y1 >= y0;
}
// Minimap+: where the map goes this tick (or hidden)
void minimap(const Value& root, MovieClip* hud, bool show) {
    g_mapFade = show ? std::min(1.f, g_mapFade + 0.25f) : std::max(0.f, g_mapFade - 0.25f);
    Value mapa = V(root, "mapa");
    DisplayObject* md = mapa.isObj() && mapa.o->isDisplay() ? (DisplayObject*)mapa.o : nullptr;
    double x0 = 0, y0 = 0, x1 = 0, y1 = 0;
    bool ok = g_mapFade > 0 && md && md->visible && hud && mapExtent(root, x0, y0, x1, y1);
    if (!ok) {
        if (md) g_snapOv[md].hide = true;
        if (g_mapClip) g_mapClip->visible = false;
        return;
    }
    // room pieces are about 24 x 9 map units around their point: half extents with a margin
    double hw = (x1 - x0) / 2 + 16, hh = (y1 - y0) / 2 + 9;
    const double cx = 320, cy = (kRoomTop + 480) / 2;
    double k = std::min(2.3, std::min(470 / (2 * hw), 300 / (2 * hh)));
    Mat A; A.a = A.d = k; A.tx = cx - 130 * k; A.ty = cy - 40 * k;
    // (Itemnomicon's card is drawn in the HUD, under the map: the map fades while it is shown)
    float under = 1 - 0.75f * itemnomiconShown();
    SnapOv& o = g_snapOv[md]; o.hasM = true; o.m = A * md->m; o.alpha = 0.72f * g_mapFade * under;
    // the backdrop
    if (!g_mapClip || g_mapClip->removed || g_mapClip->parent != hud) {
        removeMapClip();
        g_mapClip = createEmptyMovieClip(hud, intern("__widemap"), 1047900);
        if (!g_mapClip) return;
        g_mapBack = synthPut(g_mapClip, synthRect(100, 100, 0xff0b0908u), 1, 0, 0);
        for (int i = 0; i < 4; i++) g_mapEdge[i] = synthPut(g_mapClip, synthRect(100, 100, 0xffd9cbb0u), 2 + i, 0, 0);
    }
    g_mapClip->visible = true;
    g_mapClip->cx.am = g_mapFade * under;
    double bx0 = cx - hw * k, by0 = cy - hh * k, bw = 2 * hw * k, bh = 2 * hh * k;
    auto place = [](DisplayObject* d, double x, double y, double w, double h, float alpha) {
        if (!d) return;
        d->m = Mat(); d->m.a = w / 100; d->m.d = h / 100; d->m.tx = x; d->m.ty = y; d->scriptXform = true; d->cx.am = alpha;
    };
    place(g_mapBack, bx0, by0, bw, bh, 0.45f);
    const double t = 1.5;
    for (auto* e : g_mapEdge) if (e) e->visible = false;   // (no border: just the dark backdrop)
    (void)t;
}

// a group of HUD pieces: the source rectangle (HUD coordinates) and where it goes
struct Group { double sx0, sy0, sx1, sy1; };
const Group kSrc[WIDE_GROUPS] = {
    {455, 20, 625, 74},    // WIDE_LIFE: the hearts (lif) and "-LIFE-"
    {396, 0, 462, 82},     // WIDE_ACTIVE: the active item box with its charge bar (it)
    {212, 0, 300, 75},     // WIDE_COUNT: coins, bombs, keys (icons, numbers, golden key)
    {570, 50, 640, 120},   // WIDE_POCKET: the pill / card (pilll, extra)
};
// which group a HUD child belongs to, by its position
int groupOf(DisplayObject* d) {
    double x = d->m.tx, y = d->m.ty;   // (the HUD is at the origin, unscaled, in the three games)
    std::string n = d->name ? d->name->s : "";
    if (n == "lif") return WIDE_LIFE;
    if (n == "it") return WIDE_ACTIVE;
    if (n == "pilll" || n == "extra") return WIDE_POCKET;
    if (n == "cacher" || n == "kep") return WIDE_COUNT;
    if (y < 75 && x >= 212 && x < 300) return WIDE_COUNT;
    if (y < 75 && x >= 396 && x < 462 && d->kind == OK_EDITTEXT) return WIDE_ACTIVE;   // (the charge / SPACE text)
    return -1;
}
// Screen-wide shading (the pause screen's dimming, fades to black or white...) is a plain rectangle the size of the
// game's 640x480 screen: with the sides of a wide screen showing, they stayed bright around it. Such a rectangle
// (one solid fill, one four-cornered outline, covering the whole screen) is drawn larger (only on screen), so it
// covers the window too.
struct RectInfo { bool plain = false; RectF r; };
bool plainRect(uint16_t id, RectF& r) {
    static std::unordered_map<uint16_t, RectInfo> cache;
    auto it = cache.find(id);
    if (it != cache.end()) { r = it->second.r; return it->second.plain; }
    RectInfo info;
    CharDef* cd = g_movie.get(id);
    if (cd && cd->type == CH_SHAPE) {
        ShapeDef* sd = (ShapeDef*)cd; shapeEnsure(sd);
        if (sd->layers.size() == 1 && sd->layers[0].fills.size() == 1 && sd->layers[0].fills[0].type == 0 && sd->layers[0].fillPaths.size() == 1 &&
            sd->layers[0].lines.empty()) {
            const Path& p = sd->layers[0].fillPaths[0];
            int moves = 0; for (uint8_t v : p.verbs) moves += v == 0;
            if (moves == 1 && p.verbs.size() <= 6) { info.plain = true; info.r = sd->bounds; }
        }
    }
    cache[id] = info; r = info.r;
    return info.plain;
}
void stretchShades(MovieClip* root) {
    // root's children, the HUD's and the pause sheet's: where the game keeps its screen-wide layers
    std::vector<std::pair<DisplayObject*, Mat>> todo;   // (object, its matrix up to _root)
    for (DisplayObject* k : root->kids) todo.push_back({k, k->m});
    for (size_t i = 0; i < todo.size(); i++) {
        DisplayObject* d = todo[i].first; Mat toRoot = todo[i].second;
        if (!d->visible || d->removed) continue;
        std::string n = d->name ? d->name->s : "";
        if (d->kind == OK_CLIP) {
            MovieClip* c = (MovieClip*)d;
            int depth = 0; for (DisplayObject* p = d; p->parent && p->parent != root; p = p->parent) depth++;
            if (depth < 2 && n != "a" && n != "men")   // (not the room's clip nor the menus' sheet: big, never a shade)
                for (DisplayObject* k : c->kids) todo.push_back({k, toRoot * k->m});
            continue;
        }
        if (d->kind != OK_SHAPE) continue;
        RectF r;
        if (!plainRect(d->charId, r)) continue;
        double x0, y0, x1, y1, x2, y2, x3, y3;
        toRoot.apply(r.x0, r.y0, x0, y0); toRoot.apply(r.x1, r.y1, x1, y1); toRoot.apply(r.x1, r.y0, x2, y2); toRoot.apply(r.x0, r.y1, x3, y3);
        double bx0 = std::min({x0, x1, x2, x3}), bx1 = std::max({x0, x1, x2, x3}), by0 = std::min({y0, y1, y2, y3}), by1 = std::max({y0, y1, y2, y3});
        if (!(bx0 <= 2 && by0 <= 2 && bx1 >= 638 && by1 >= 478)) continue;
        // (not inside a filter: a larger shape would make its off-screen layer that much larger)
        bool filtered = false;
        for (DisplayObject* p = d; p && p != root; p = p->parent) if (!p->filters.empty()) { filtered = true; break; }
        if (filtered) continue;
        // larger around its middle (in its own coordinates), just enough for the window's shape (Hyper Zoom: the room
        // is 402.5 high instead of 480)
        double aspect = std::max(0.2, (double)g_windowAspect.load()), need = std::max(aspect * 480 / 640, 640 / (aspect * 480));
        const double k = std::max(1.0, need) * (480 / 402.5) * 1.1, cx = (r.x0 + r.x1) / 2, cy = (r.y0 + r.y1) / 2;
        Mat L; L.a = L.d = k; L.tx = cx * (1 - k); L.ty = cy * (1 - k);
        SnapOv& o = g_snapOv[d];
        o.m = (o.hasM ? o.m : d->m) * L; o.hasM = true;
        static const bool dbg = getenv("ISAAC_SHADELOG") != nullptr;   // (testing)
        if (dbg) logf("[shade] %s char %u in %s: k %.2f bounds %.0f,%.0f-%.0f,%.0f\n", n.c_str(), d->charId, d->parent && d->parent->name ? d->parent->name->s.c_str() : "-", k, bx0, by0, bx1, by1);
    }
}
// A boss's intro ("Isaac VS ...", _root on its "bosscuts" frame) comes between entering the boss room and the fight
// (and the floor cutscenes and endings, likewise):
// with Hyper Zoom the view stays the room's, so the zoom doesn't change; the intro's 640x480 is fitted into it
// (scaled to the room's height, centred) with black on its sides. The intro's frame around the stage, the HUD and the
// map are not drawn. Returns true if this tick is an intro.
bool bossIntro() {
    if (vm.root->kind != OK_CLIP || genesisOpen()) return false;
    MovieClip* root = (MovieClip*)vm.root;
    static int boss = 0, cuts = 0;
    if (!boss) {
        boss = 4; cuts = 3;   // (the three games)
        if (root->def) {
            auto it = root->def->labels.find("bosscuts"); if (it != root->def->labels.end()) boss = it->second;
            it = root->def->labels.find("cuts"); if (it != root->def->labels.end()) cuts = it->second;
        }
    }
    // also the floor cutscenes ("cuts", between floors) and, during a run, the endings (frames 9-40)
    if (root->cur == 5) s_runSeen = false;
    bool ending = s_runSeen && root->cur >= 9 && root->cur <= 40;
    if (root->cur != boss && root->cur != cuts && !ending) return false;
    const Mat& rm = root->m;
    double vx0, vy0, vx1, vy1; rm.apply(0, kRoomTop, vx0, vy0); rm.apply(640, 480, vx1, vy1);
    g_viewReq.set = true;
    g_viewReq.r[0] = (float)vx0; g_viewReq.r[1] = (float)vy0; g_viewReq.r[2] = (float)vx1; g_viewReq.r[3] = (float)vy1;
    const double s = (480 - kRoomTop) / 480;
    Mat t; t.a = t.d = s; t.tx = 320 - 320 * s; t.ty = kRoomTop;
    double cx0, cy0, cx1, cy1; rm.apply(t.tx, kRoomTop, cx0, cy0); rm.apply(640 - t.tx, 480, cx1, cy1);
    g_viewReq.cut = true;
    // (a pixel in from the sides: some intros have a white line at the stage's very edge, normally under the frame)
    const double in = std::max(1.0, rm.a * 0.6);
    g_viewReq.c[0] = (float)(cx0 + in); g_viewReq.c[1] = (float)cy0; g_viewReq.c[2] = (float)(cx1 - in); g_viewReq.c[3] = (float)cy1;
    for (DisplayObject* k : root->kids) {
        std::string n = k->name ? k->name->s : "";
        if (n == "overlay" || n == "hud" || n == "mapa" || n == "olda") { g_snapOv[k].hide = true; continue; }
        if (n.compare(0, 2, "__") == 0) continue;
        SnapOv& o = g_snapOv[k]; o.m = t * (o.hasM ? o.m : k->m); o.hasM = true;
    }
    if (g_mapClip) g_mapClip->visible = false;
    g_mapFade = 0;
    return true;
}
}  // namespace

bool wideMinimapAvailable() { return s_hyperNow.load(); }
void wideToggleMinimap() { if (s_hyperNow.load()) s_mapWanted = !s_mapWanted.load(); }
bool wideMinimapShown() { return s_mapWanted.load(); }

bool wideHudMove(int group, Mat& m) {
    if (group < 0 || group >= WIDE_GROUPS || !g_groupOn[group]) return false;
    m = g_groupMove[group];
    return true;
}

void wideTick(const ModInfo& mod) {
    static bool rooted = false;
    if (!rooted) { rooted = true; gcAddRoot((GcObj**)&g_mapClip); for (auto& e : g_mapEdge) gcAddRoot((GcObj**)&e); gcAddRoot((GcObj**)&g_mapBack); }
    s_hyperNow = false;
    for (auto& g : g_groupOn) g = false;
    g_viewReq = ViewReq();
    bool fill = mod.opt("fill", "1") != "0";
    bool hyper = mod.opt("hyper_zoom", "0") == "1";
    if (fill) { g_viewReq.set = true; g_viewReq.clip = false; g_viewReq.r[2] = (float)g_stageW; g_viewReq.r[3] = (float)g_stageH; }
    // the menus (_root frame 5) are one big sheet that pans: on a wide screen show more of it instead of the frame of
    // faces around the 640x480 (_root.overlay is drawn over everything outside it)
    if (fill && vm.root->kind == OK_CLIP && ((MovieClip*)vm.root)->cur == 5)
        for (DisplayObject* k : ((MovieClip*)vm.root)->kids)
            if (k->name && k->name->s == "overlay") g_snapOv[k].hide = true;
    if (fill && vm.root->kind == OK_CLIP) stretchShades((MovieClip*)vm.root);
    if (!hyper) return;
    bool inGame = false, play = recompInGameplay(&inGame);
    Value root((Object*)vm.root);
    if (bossIntro()) return;
    Value hudV = V(root, "hud");
    // (also paused and on the death screen: Hyper Zoom stays on during the whole run, only cutscenes and the console
    // show the normal screen)
    int cur = vm.root->kind == OK_CLIP ? ((MovieClip*)vm.root)->cur : 0;
    bool gameFrame = cur == 2;
    if (!gameFrame || genesisOpen()) { if (g_mapClip) g_mapClip->visible = false; g_mapFade = 0; return; }
    // the view: the room, in stage pixels - as soon as _root is on its game frame, also while a run is starting and
    // the HUD isn't shown yet (the HUD band, empty and white then, flashed above the room before the zoom came in)
    const Mat& rm = vm.root->m;
    double vx0, vy0, vx1, vy1; rm.apply(0, kRoomTop, vx0, vy0); rm.apply(640, 480, vx1, vy1);
    g_viewReq.set = true; g_viewReq.clip = !fill;
    g_viewReq.r[0] = (float)vx0; g_viewReq.r[1] = (float)vy0; g_viewReq.r[2] = (float)vx1; g_viewReq.r[3] = (float)vy1;
    if (!inGame || !hudV.isObj() || hudV.o->kind != OK_CLIP || !((MovieClip*)hudV.o)->visible) { if (g_mapClip) g_mapClip->visible = false; g_mapFade = 0; return; }
    MovieClip* hud = (MovieClip*)hudV.o;
    s_hyperNow = true; s_runSeen = true;
    // The column goes over the room's right wall, like StatOn over the left one: past the room, the game's own
    // frame overlay (_root.overlay, above the HUD) covers everything.
    const double R = 636, cx = 606;
    // targets: center (x, y) and scale of each group (_root units). The right door is around y 250-310.
    struct Tgt { double x, y, k; };
    const Tgt tgt[WIDE_GROUPS] = {
        {R - 4 - (kSrc[WIDE_LIFE].sx1 - kSrc[WIDE_LIFE].sx0) * 0.38, kRoomTop + 4 + (kSrc[WIDE_LIFE].sy1 - kSrc[WIDE_LIFE].sy0) * 0.38, 0.76},
        {cx, 170, 0.78},
        {cx - 22, 375, 0.8},
        {cx, 448, 0.75},
    };
    for (int g = 0; g < WIDE_GROUPS; g++) {
        const Group& s = kSrc[g];
        double scx = (s.sx0 + s.sx1) / 2, scy = (s.sy0 + s.sy1) / 2;
        Mat mv; mv.a = mv.d = tgt[g].k; mv.tx = tgt[g].x - scx * tgt[g].k; mv.ty = tgt[g].y - scy * tgt[g].k;
        g_groupMove[g] = mv; g_groupOn[g] = true;
    }
    // the HUD's pieces: moved with their group, or hidden if they are in the band above the room
    Mat inv; rm.invert(inv);
    for (DisplayObject* k : hud->kids) {
        int g = groupOf(k);
        if (g >= 0) { SnapOv& o = g_snapOv[k]; o.hasM = true; o.m = g_groupMove[g] * k->m; continue; }   // (keeping what other mods set)
        std::string n = k->name ? k->name->s : "";
        if (n == "paus" && k->kind == OK_CLIP && ((MovieClip*)k)->cur == 3) {
            // the death screen's diary page is taller than the room: a little smaller, in the middle of the room
            const double sc = 0.86, cx0 = 320, cy0 = 255, cy1 = (kRoomTop + 480) / 2;
            Mat t; t.a = t.d = sc; t.tx = cx0 - cx0 * sc; t.ty = cy1 - cy0 * sc;
            SnapOv& o = g_snapOv[k]; o.hasM = true; o.m = t * k->m;
            continue;
        }
        if (n == "paus" || n == "bar" || n.compare(0, 2, "__") == 0) continue;   // (the pause sheet, the boss's health bar, the mods' own clips)
        RectF b = stageBounds(k);
        if (!b.valid()) continue;
        double x0, y0, x1, y1; inv.apply(b.x0, b.y0, x0, y0); inv.apply(b.x1, b.y1, x1, y1);
        if (std::max(y0, y1) <= kRoomTop + 3) g_snapOv[k].hide = true;
    }
    // no map, or the Minimap+
    minimap(root, hud, play && s_mapWanted.load() && mod.opt("minimap", "1") != "0");
}
}  // namespace fl
