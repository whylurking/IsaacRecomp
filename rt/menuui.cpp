// Controller menu navigation, the "REBIND CONTROLS" entry and the rebinding screen inside the game (see menuui.h).
//
// The game's menus are Flash buttons made for the mouse (and a few keys). With a controller, the logic thread
// keeps a "focused" button among the ones on screen: the D-pad / stick moves it to the nearest button in that
// direction, A (cross) clicks it exactly like the mouse would, B (circle) goes back. The renderer draws an outline
// around it. The rebinding screen and its entry on the Control options page are built from shapes and text fields
// synthesized at run time (the text in the game's own HUD font) and placed in the display list, so they are drawn,
// scaled and layered like the rest of the game.
#include "menuui.h"
#include "core.h"
#include "display.h"
#include "snapshot.h"
#include <algorithm>
#include <cmath>
#include <map>
#include <mutex>
#include <cctype>
#include <cstdlib>
#include "platform.h"

namespace fl {
extern std::vector<std::function<void(Tracer&)>> g_rootTracers;

void visibleButtons(std::vector<ButtonObj*>& out);
ButtonObj* topButtonAt(double x, double y);
int g_rebindRequests = 0;
uint16_t g_synthCharBase = 0xffff;   // first synthesized character id (the renderer doesn't learn them for preloading)

// ============================================================ shared model (main thread -> logic thread)
static std::mutex g_uiMu;
static UiModel g_uiModel; static int g_uiVersion = 0;
static std::vector<RectF> g_uiRows; static RectF g_uiPanel; static bool g_uiShown = false;

void uiPublish(const UiModel& m) { std::lock_guard<std::mutex> lk(g_uiMu); g_uiModel = m; g_uiVersion++; }
int uiHitRow(double sx, double sy) {
    std::lock_guard<std::mutex> lk(g_uiMu);
    if (!g_uiShown) return -2;
    for (size_t i = 0; i < g_uiRows.size(); i++) { const RectF& r = g_uiRows[i]; if (sx >= r.x0 && sx <= r.x1 && sy >= r.y0 && sy <= r.y1) return (int)i; }
    if (sx >= g_uiPanel.x0 && sx <= g_uiPanel.x1 && sy >= g_uiPanel.y0 && sy <= g_uiPanel.y1) return -1;
    return -2;
}

// ============================================================ synthesized shapes and texts
static std::map<std::string, uint16_t> g_synth;
static uint16_t addChar(CharDef* d) {
    // character ids from the top of the 16-bit range down (the SWF's own ids are far below)
    static int next = 0xfffe;
    if (g_movie.chars.size() < 0x10000) g_movie.chars.resize(0x10000, nullptr);
    while (next > 0 && g_movie.chars[next]) next--;
    if (next <= 0) return 0;
    d->id = (uint16_t)next; g_movie.chars[next] = d;
    if (next < g_synthCharBase) g_synthCharBase = (uint16_t)next;
    return (uint16_t)next--;
}
static uint16_t rectShape(float w, float h, uint32_t argb) {
    char key[96]; snprintf(key, sizeof key, "r%.1f,%.1f,%08x", w, h, argb);
    auto it = g_synth.find(key); if (it != g_synth.end()) return it->second;
    auto* s = new ShapeDef(); s->type = CH_SHAPE;
    s->bounds = {0, 0, w, h};
    ShapeLayer L; FillStyle f; f.type = 0; f.argb = argb; L.fills.push_back(f);
    Path p; p.verbs = {0, 1, 1, 1, 1}; p.pts = {0, 0, w, 0, w, h, 0, h, 0, 0};
    L.fillPaths.push_back(p);
    s->layers.push_back(L);
    return g_synth[key] = addChar(s);
}
static uint16_t hudFontId() {
    static int id = -1;
    if (id >= 0) return (uint16_t)id;
    id = 0;
    for (CharDef* c : g_movie.chars)
        if (c && c->type == CH_FONT && ((FontDef*)c)->name.compare(0, 8, "Upheaval") == 0) { id = c->id; break; }
    return (uint16_t)id;
}
static uint16_t textField(float w, float size, uint32_t argb, int align) {
    char key[96]; snprintf(key, sizeof key, "t%.1f,%.1f,%08x,%d", w, size, argb, align);
    auto it = g_synth.find(key); if (it != g_synth.end()) return it->second;
    auto* e = new EditTextDef(); e->type = CH_EDITTEXT;
    e->bounds = {0, 0, w, size * 1.4};
    e->fontId = hudFontId(); e->height = size; e->argb = argb; e->align = (uint8_t)align;
    e->readOnly = true; e->noSelect = true; e->useOutlines = true; e->hasText = true;
    return g_synth[key] = addChar(e);
}
#include "menuui_glyphs.inc"
// A word written with the hand-drawn glyphs as one shape (strokes with round ends, like a marker). H = cap height.
static uint16_t handwritten(const char* text, float H, uint32_t argb, float* outW = nullptr) {
    char key[160]; snprintf(key, sizeof key, "h%s|%.1f|%08x", text, H, argb);
    const float sw = 0.17f * H, spacing = 0.085f;
    auto it = g_synth.find(key);
    auto glyph = [](char c) -> const HandGlyph* {
        for (auto& g : kHandGlyphs) if (g.c == c) return &g;
        for (auto& g : kHandGlyphs) if (g.c == (char)tolower((unsigned char)c)) return &g;
        return nullptr;
    };
    float x = 0;
    for (const char* p = text; *p; p++) if (const HandGlyph* g = glyph(*p)) x += (g->w + spacing) * H;
    if (outW) *outW = x;
    if (it != g_synth.end()) return it->second;
    auto* s = new ShapeDef(); s->type = CH_SHAPE;
    ShapeLayer L; LineStyle ls; ls.width = sw; ls.argb = argb; ls.startCap = ls.endCap = 0; ls.join = 0; ls.noClose = true;
    L.lines.push_back(ls);
    Path path; uint32_t seed = 7;
    auto jitter = [&]() { seed = seed * 1103515245u + 12345u; return ((seed >> 16) % 1000 / 1000.0f - 0.5f) * 0.03f; };
    x = 0;
    for (const char* p = text; *p; p++) {
        const HandGlyph* g = glyph(*p); if (!g) continue;
        float rot = jitter() * 1.2f, dy = jitter() * 0.6f;   // each letter a little off, as if written by hand
        bool start = true;
        for (int i = 0; i < g->n; i++) {
            float gx = g->pts[2 * i], gy = g->pts[2 * i + 1];
            if (std::isnan(gx)) { start = true; continue; }
            float px = gx + jitter(), py = gy + jitter() + dy;
            px += (py - 0.5f) * rot;
            path.verbs.push_back(start ? 0 : 1); path.pts.push_back(x + px * H); path.pts.push_back(py * H);
            start = false;
        }
        x += (g->w + spacing) * H;
    }
    L.linePaths.push_back(path);
    s->bounds = {-sw, -sw, x + sw, H + sw};
    s->layers.push_back(L);
    return g_synth[key] = addChar(s);
}
#include "menuui_caps.inc"
// Any text in the same marker style (the mods' menu texts): capitals, digits and a few signs from kHandCaps; small
// letters as small capitals (y from 0.36 down). H = cap height.
static uint16_t handText(const std::string& text, float H, uint32_t argb, float* outW) {
    char key[200]; snprintf(key, sizeof key, "H%s|%.1f|%08x", text.c_str(), H, argb);
    const float sw = 0.17f * H, spacing = 0.15f, space = 0.34f;
    auto cap = [](char c) -> const HandCap* { char u = (char)toupper((unsigned char)c); for (auto& g : kHandCaps) if (g.c == u) return &g; return nullptr; };
    float x = 0;
    for (char c : text) { const HandCap* g = cap(c); bool small = c >= 'a' && c <= 'z'; x += g ? (g->w * (small ? 0.78f : 1) + spacing) * H : space * H; }
    if (outW) *outW = x;
    auto it = g_synth.find(key); if (it != g_synth.end()) return it->second;
    auto* s = new ShapeDef(); s->type = CH_SHAPE;
    ShapeLayer L; LineStyle ls; ls.width = sw; ls.argb = argb; ls.startCap = ls.endCap = 0; ls.join = 0; ls.noClose = true;
    L.lines.push_back(ls);
    Path path; uint32_t seed = 11;
    auto jitter = [&]() { seed = seed * 1103515245u + 12345u; return ((seed >> 16) % 1000 / 1000.0f - 0.5f) * 0.03f; };
    x = 0;
    for (char c : text) {
        const HandCap* g = cap(c);
        if (!g) { x += space * H; continue; }
        bool small = c >= 'a' && c <= 'z';
        float kx = small ? 0.78f : 1, ky = small ? 0.64f : 1, y0 = small ? 0.36f : 0;
        float rot = jitter() * 1.2f, dy = jitter() * 0.6f;
        bool start = true;
        for (int i = 0; i < g->n; i++) {
            float gx = g->pts[2 * i], gy = g->pts[2 * i + 1];
            if (std::isnan(gx)) { start = true; continue; }
            float py = y0 + gy * ky + jitter() + dy, px = gx * kx + jitter() + (py - 0.5f) * rot;
            path.verbs.push_back(start ? 0 : 1); path.pts.push_back(x + px * H); path.pts.push_back(py * H);
            start = false;
        }
        x += (g->w * kx + spacing) * H;
    }
    L.linePaths.push_back(path);
    s->bounds = {-sw, -sw, x + sw, H + sw};
    s->layers.push_back(L);
    return g_synth[key] = addChar(s);
}
static DisplayObject* put(MovieClip* parent, uint16_t id, int depth, double x, double y, const char* name = nullptr) {
    if (!id) return nullptr;
    PlaceCmd pc; pc.hasMatrix = true; pc.m.tx = x; pc.m.ty = y;
    DisplayObject* d = instantiate(parent, id, depth, &pc, false);
    if (d && name) d->name = intern(name);
    return d;
}
static DisplayObject* putText(MovieClip* parent, int depth, double x, double y, float w, float size, uint32_t argb, int align, const std::string& text) {
    DisplayObject* d = put(parent, textField(w, size, argb, align), depth, x, y);
    if (d && d->kind == OK_EDITTEXT) ((EditTextObj*)d)->setText(text);
    return d;
}

// (shared with the mods: mods.h)
uint16_t synthAddChar(CharDef* d) { return addChar(d); }
uint16_t synthRect(float w, float h, uint32_t argb) { return rectShape(w, h, argb); }
uint16_t synthHandText(const std::string& text, float H, uint32_t argb, float* outW) { return handText(text, H, argb, outW); }
uint16_t synthTextField(float w, float size, uint32_t argb, int align) { return textField(w, size, argb, align); }
DisplayObject* synthPut(MovieClip* parent, uint16_t id, int depth, double x, double y, const char* name) { return put(parent, id, depth, x, y, name); }

// ============================================================ "REBIND CONTROLS" on the Control options page
// The options sheet (_root.paus: _root.pauss in the menus, _root.hud.paus during a run) shows "Control options" on
// its frame 8 in the three games: "Lefty Flip" and "Azerty" toggles, then empty paper. The entry goes below them.
static DisplayObject* g_rebindLabel = nullptr;
static bool g_labelPressed = false;
static void removeLabel() {
    if (g_rebindLabel) { if (!g_rebindLabel->removed && g_rebindLabel->parent) removeObject(g_rebindLabel); g_rebindLabel = nullptr; }
}
static MovieClip* optionsSheet() {
    Value p = getMemK(Value((Object*)vm.root), intern("paus"));
    if (!p.isObj() || p.o->kind != OK_CLIP) return nullptr;
    MovieClip* c = (MovieClip*)p.o;
    if (c->removed) return nullptr;
    for (DisplayObject* q = c; q; q = q->parent) if (!q->visible) return nullptr;
    return c;
}
static void updateRebindLabel() {
    MovieClip* sheet = optionsSheet();
    bool want = sheet && sheet->cur == 8;
    if (!want || (g_rebindLabel && (g_rebindLabel->removed || g_rebindLabel->parent != sheet))) { removeLabel(); if (!want) return; }
    if (g_rebindLabel) return;
    // in the sheet's own coordinates: the two toggles' labels start at x ~ 73, rows at y ~ 76 and ~ 123;
    // the free line below them is at ~ 205 (above the gamepad doodle)
    // written in the same place and size as the other two labels: their capitals start at x ~ 73, the rows are
    // ~ 44 apart (Lefty Flip at y ~ 66, Azerty at ~ 110) and capitals are ~ 26 tall
    g_rebindLabel = put(sheet, handwritten("Rebind Controls", 26, 0xff878484u), DEPTH_BIAS + 9000, 74, 155);
    if (g_rebindLabel) g_rebindLabel->name = intern("__rebind");
}
static bool labelHit(double sx, double sy) {
    if (!g_rebindLabel || g_rebindLabel->removed) return false;
    RectF r = stageBounds(g_rebindLabel);
    double pad = g_stageH / 120.0;
    return r.valid() && sx >= r.x0 - pad && sx <= r.x1 + pad && sy >= r.y0 - pad && sy <= r.y1 + pad;
}
void menuMouse(double sx, double sy, int type) {
    if (type == 1) g_labelPressed = labelHit(sx, sy);
    else if (type == 2) { if (g_labelPressed && labelHit(sx, sy)) g_rebindRequests++; g_labelPressed = false; }
}

// ============================================================ rebinding screen (drawn in the game's display list)
static MovieClip* g_ui = nullptr; static int g_uiBuilt = -1;
static void removeUi() {
    if (g_ui) { if (!g_ui->removed && g_ui->parent) removeObject(g_ui); g_ui = nullptr; }
    std::lock_guard<std::mutex> lk(g_uiMu); g_uiShown = false; g_uiRows.clear();
}
static void buildUi(const UiModel& m) {
    removeUi();
    if (!m.open) return;
    MovieClip* root = vm.root;
    g_ui = createEmptyMovieClip(root, intern("__controls"), 1000000);
    if (!g_ui) return;
    // _root's visible area (the stage, in _root coordinates: the HD patch scales _root)
    Mat inv; if (!root->m.invert(inv)) return;
    double rx0, ry0, rx1, ry1; inv.apply(0, 0, rx0, ry0); inv.apply(g_stageW, g_stageH, rx1, ry1);
    const int n = (int)m.rows.size();
    const double W = 480, RH = 21.5, top = 50, bottom = 34, H = top + n * RH + bottom;
    double cx = (rx0 + rx1) / 2, cy = (ry0 + ry1) / 2, x0 = std::round(cx - W / 2), y0 = std::round(cy - H / 2);
    int dp = 1;
    put(g_ui, rectShape((float)(rx1 - rx0 + 2), (float)(ry1 - ry0 + 2), 0xa0000000u), dp++, rx0 - 1, ry0 - 1);   // dim the game
    put(g_ui, rectShape((float)W + 6, (float)H + 6, 0x60000000u), dp++, x0 + 4, y0 + 6);                      // shadow
    put(g_ui, rectShape((float)W + 6, (float)H + 6, 0xff0c0c0cu), dp++, x0 - 3, y0 - 3);                     // border
    put(g_ui, rectShape((float)W, (float)H, 0xffe2dbcfu), dp++, x0, y0);                                       // paper
    putText(g_ui, dp++, x0, y0 + 9, (float)W, 26, 0xff8c2a2au, 2, m.title);
    put(g_ui, rectShape((float)W - 40, 2, 0xff8d8780u), dp++, x0 + 20, y0 + top - 6);
    std::vector<RectF> rows;
    for (int i = 0; i < n; i++) {
        double ry = y0 + top + i * RH;
        bool sel = i == m.sel;
        if (sel) put(g_ui, rectShape((float)W - 24, (float)RH - 1, 0xff4a4744u), dp++, x0 + 12, ry);
        uint32_t lc = sel ? 0xffebe5dbu : 0xff4a4744u, vc = sel ? 0xfff0c674u : 0xff8c2a2au;
        putText(g_ui, dp++, x0 + 18, ry + 1, 220, 15, lc, 0, m.rows[i].label);
        std::string v = m.rows[i].value;
        if (sel && m.capturing && !m.blink) v.clear();
        if (!v.empty()) putText(g_ui, dp++, x0 + W - 18 - 260, ry + 1, 260, 15, vc, 1, v);
        double ax0, ay0, ax1, ay1;
        root->m.apply(x0 + 8, ry, ax0, ay0); root->m.apply(x0 + W - 8, ry + RH, ax1, ay1);
        rows.push_back({std::min(ax0, ax1), std::min(ay0, ay1), std::max(ax0, ax1), std::max(ay0, ay1)});
    }
    putText(g_ui, dp++, x0, y0 + H - 26, (float)W, 12, 0xff66625du, 2, m.hint);
    double px0, py0, px1, py1; root->m.apply(x0, y0, px0, py0); root->m.apply(x0 + W, y0 + H, px1, py1);
    std::lock_guard<std::mutex> lk(g_uiMu);
    g_uiRows = rows; g_uiPanel = {std::min(px0, px1), std::min(py0, py1), std::max(px0, px1), std::max(py0, py1)}; g_uiShown = true;
}

// ============================================================ controller navigation
static bool g_navOn = false;
static DisplayObject* g_focus = nullptr;
static double g_lastFx = -1, g_lastFy = -1;
struct Pulse { int code; uint32_t until; };
static std::vector<Pulse> g_pulses;
static bool g_uiOpen = false, g_inGame = false, g_gameplay = false;
static bool g_hand = false;
static bool g_focusShown = false; static RectF g_focusRect; static DisplayObject* g_focusShow = nullptr;

// d: what is selected (a button, or the "Rebind Controls" label); show: what turns red (the button itself, or the
// clip around it when the button is only an invisible hit area: the challenges, the collection's items)
struct Cand { DisplayObject* d; RectF r; double cx, cy; DisplayObject* show; };
static DisplayObject* topLevel(DisplayObject* d) { while (d->parent && d->parent != vm.root) d = d->parent; return d; }
static double area(const RectF& r) { return (r.x1 - r.x0) * (r.y1 - r.y0); }
static uint32_t g_clickTick = 0; static bool g_afterClick = false, g_hideMoving = false;   // (see menuUiTick)
static ButtonObj* g_modalBtn = nullptr;   // a button whose hit area covers the screen ("No" of a dialog, a sheet's Back)
// a point inside the option where a click reaches it (its center, else a 5x5 grid inside its rect)
static bool pointOn(const Cand& c, double& x, double& y) {
    x = c.cx; y = c.cy;
    if (topButtonAt(x, y) == c.d) return true;
    for (int j = 1; j <= 5; j++) for (int i = 1; i <= 5; i++) {
        x = c.r.x0 + (c.r.x1 - c.r.x0) * i / 6; y = c.r.y0 + (c.r.y1 - c.r.y0) * j / 6;
        if (topButtonAt(x, y) == c.d) return true;
    }
    x = c.cx; y = c.cy;
    return false;
}
// the pause screen's "Items collected" icons (hud.paus.i0..i19) are the collection's item clips, whose invisible
// button does nothing there: not options
static bool itemIcon(ButtonObj* b) {
    DisplayObject* p = b->parent;
    if (!p || !p->name || p->name->s.size() < 2 || p->name->s[0] != 'i') return false;
    for (size_t k = 1; k < p->name->s.size(); k++) if (!isdigit((unsigned char)p->name->s[k])) return false;
    DisplayObject* sheet = p->parent;
    return sheet && sheet->name && sheet->name->s == "paus" && sheet->parent && sheet->parent->name && sheet->parent->name->s == "hud";
}
// (mods) more options drawn by a mod (The Gone's items in the collection): their stage rectangles, set every tick
static std::vector<std::pair<DisplayObject*, RectF>> g_menuExtras;
void menuSetExtras(const std::vector<std::pair<DisplayObject*, RectF>>& v) { g_menuExtras = v; }
static std::vector<Cand> candidates() {
    std::vector<Cand> out;
    std::vector<ButtonObj*> bs; visibleButtons(bs);
    std::vector<Cand> all;
    for (ButtonObj* b : bs) {
        if (!b->enabled || b->removed) continue;
        if (itemIcon(b)) continue;
        bool acts = false; for (auto& c : b->def->conds) if (c.script) acts = true;
        if (!acts) continue;   // (does nothing when pressed: the pause screen's "Options" left under the "are you sure?" note)
        double a = 1; for (DisplayObject* p = b; p; p = p->parent) a *= p->cx.am;
        if (a < 0.05) continue;
        RectF r = stageBounds(b);
        DisplayObject* show = b;
        if (!r.valid() || r.x1 - r.x0 < 4 || r.y1 - r.y0 < 4) {   // draws nothing: its hit area, and the clip around it
            r = hitStageBounds(b);
            show = nullptr;
            if (b->parent && b->parent != vm.root) {
                RectF pr = stageBounds(b->parent);
                if (pr.valid() && area(pr) < area(r) * 6) show = b->parent;
            }
        }
        if (!r.valid() || r.x1 - r.x0 < 4 || r.y1 - r.y0 < 4) continue;
        double cx = (r.x0 + r.x1) / 2, cy = (r.y0 + r.y1) / 2;
        if (cx < 0 || cy < 0 || cx > g_stageW || cy > g_stageH) continue;
        all.push_back({b, r, cx, cy, show});
    }
    if (g_rebindLabel && !g_rebindLabel->removed) {
        RectF r = stageBounds(g_rebindLabel);
        if (r.valid()) all.push_back({g_rebindLabel, r, (r.x0 + r.x1) / 2, (r.y0 + r.y1) / 2, g_rebindLabel});
    }
    if (all.empty()) return out;
    // a dialog: a button whose hit area covers the whole screen (clicking anywhere outside answers "No": the "are you
    // sure you want me to die?" note over the main menu) -> only the buttons of the clip it belongs to
    {
        DisplayObject* modal = nullptr; g_modalBtn = nullptr;
        for (ButtonObj* b : bs) {
            if (!b->enabled || b->removed || !b->parent || b->parent == vm.root) continue;
            RectF h = hitStageBounds(b);
            if (h.valid() && area(h) >= 0.8 * g_stageW * g_stageH) { modal = b->parent; g_modalBtn = b; }   // (the last one: on top)
        }
        if (modal) {
            std::vector<Cand> in;
            for (auto& c : all) for (DisplayObject* p = c.d->parent; p; p = p->parent) if (p == modal) { in.push_back(c); break; }
            if (!in.empty()) all.swap(in);
        }
    }
    // only the topmost layer: the options sheet over the main menu, the pause sheet over the room...
    int best = -1 << 30;
    for (auto& c : all) best = std::max(best, topLevel(c.d)->depth);
    for (auto& c : all) {
        if (topLevel(c.d)->depth != best) continue;
        bool dup = false;
        for (auto& o : out) if (std::fabs(o.cx - c.cx) < 6 && std::fabs(o.cy - c.cy) < 6) { o = c; dup = true; break; }
        if (!dup) out.push_back(c);
    }
    if (!g_modalBtn)   // (a mod's options: on the screen it drew them on, not under a dialog)
        for (auto& e : g_menuExtras) if (e.first && e.second.valid()) out.push_back({e.first, e.second, (e.second.x0 + e.second.x1) / 2, (e.second.y0 + e.second.y1) / 2, nullptr});
    // under such a full-screen button, only what the mouse could still click: the pause screen's own options stay
    // in the display list under the in-game "are you sure?" note
    if (g_modalBtn) {
        std::vector<Cand> in;
        double x, y;
        for (auto& c : out) if (c.d == g_rebindLabel || pointOn(c, x, y)) in.push_back(c);
        if (!in.empty()) out.swap(in);
    }
    return out;
}
static const Cand* findFocus(const std::vector<Cand>& cs) {
    for (auto& c : cs) if (c.d == g_focus) return &c;
    return nullptr;
}
static void hover(const Cand& c) {
    bool extra = false; for (auto& e : g_menuExtras) extra |= e.first == c.d;   // (a mod's option: it looks at the pointer)
    if (c.d->kind == OK_BUTTON || extra) mouseEvent(c.cx, c.cy, 0);
    g_lastFx = c.cx; g_lastFy = c.cy;
}
static bool isBackButton(DisplayObject* d) {   // the sheets' "Back" buttons answer to Escape (key code 19)
    if (d->kind != OK_BUTTON) return false;
    for (auto& c : ((ButtonObj*)d)->def->conds) if ((c.cond >> 9) == 19) return true;
    return false;
}
static bool named(DisplayObject* d, const char* n) { return d && d->name && d->name->s == n; }
static DisplayObject* startButton() {   // the title menu's "Start": the first button of _root.men.men (the 3 games)
    std::vector<ButtonObj*> bs; visibleButtons(bs);
    for (ButtonObj* b : bs) {
        DisplayObject* p = b->parent;
        if (named(p, "men") && named(p->parent, "men") && p->parent->parent == vm.root) return b;
    }
    return nullptr;
}
static MovieClip* charRing();
// the option selected when a screen opens (or the selected one goes away)
static const Cand* defaultFocus(const std::vector<Cand>& cs) {
    const Cand* pick = nullptr;
    if (MovieClip* ring = charRing()) {   // "Who am I?": Select (the note at the bottom right, outside the ring)
        for (auto& c : cs) {
            bool inRing = false; for (DisplayObject* q = c.d; q; q = q->parent) if (q == ring) { inRing = true; break; }
            if (!inRing && (!pick || c.cx + c.cy > pick->cx + pick->cy)) pick = &c;
        }
        if (pick) return pick;
    }
    {   // a Yes / No dialog: "No" (the full-screen button; on the pause screen itself that one is Back: not this rule)
        MovieClip* sheet = optionsSheet();
        bool pauseScreen = g_inGame && (!sheet || sheet->cur == 2);
        if (!pauseScreen) for (auto& c : cs) if (c.d == g_modalBtn && (cs.size() == 2 || !isBackButton(c.d))) return &c;
    }
    if (DisplayObject* st = startButton())   // the main menu: always Start
        for (auto& c : cs) if (c.d == st) return &c;
    for (auto& c : cs)   // the collection: its first item
        if (named(c.d, "i0") || named(c.d->parent, "i0")) return &c;
    std::string top = topLevel(cs[0].d)->name ? topLevel(cs[0].d)->name->s : std::string();
    if (top == "pauss" || top == "hud") {   // the paper sheets (pause, options, controls, challenges): the first option
        for (auto& c : cs) {
            if (isBackButton(c.d)) continue;
            if (!pick || c.r.y0 < pick->r.y0 - 8 || (c.r.y0 < pick->r.y0 + 8 && c.r.x0 < pick->r.x0)) pick = &c;
        }
        if (pick) return pick;
    }
    if (g_lastFx >= 0) {   // elsewhere: the option nearest to where the selection was
        double bd = 1e30;
        for (auto& c : cs) { double d = std::hypot(c.cx - g_lastFx, c.cy - g_lastFy); if (d < bd) { bd = d; pick = &c; } }
        return pick;
    }
    for (auto& c : cs) if (!pick || area(c.r) > area(pick->r)) pick = &c;
    return pick;
}
static const Cand* ensureFocus(const std::vector<Cand>& cs) {
    if (cs.empty()) { g_focus = nullptr; return nullptr; }
    if (const Cand* f = findFocus(cs)) return f;
    const Cand* pick = defaultFocus(cs);
    g_focus = pick->d; hover(*pick);
    return pick;
}
// while the screen moves (the main menu's camera travelling to the collection, a sheet sliding in) nothing is
// shown or chosen: what is selected afterwards is decided once it settles
static uint64_t g_layoutSig = 0; static uint32_t g_movingSince = 0, g_stillSince = 0;
static bool screenMoving(const std::vector<Cand>& cs) {
    uint64_t h = 1469598103934665603ull;
    for (auto& c : cs) {
        DisplayObject* p = c.d->parent ? c.d->parent : c.d;
        Mat w = worldMatrix(p);
        int64_t v[2] = {(int64_t)std::lround(w.tx), (int64_t)std::lround(w.ty)};
        for (int64_t x : v) { h ^= (uint64_t)x; h *= 1099511628211ull; }
    }
    uint32_t t = g_player.tick;
    if (h != g_layoutSig) { g_layoutSig = h; if (t > g_stillSince + 1) g_movingSince = t; g_stillSince = t; }
    bool moving = t < g_stillSince + 2;
    return moving && t < g_movingSince + 45;   // (something that never stops moving doesn't hide it forever)
}
static void pulse(int code, int ascii) {
    keyEvent(code, ascii, true);
    g_pulses.push_back({code, g_player.tick + 3});
}
static void click(const Cand& c);
// B / circle: what the mouse would do to go back. The pause screen itself: resume (P). A dialog: its full-screen
// "No". A sheet: its Back button (Escape would close every sheet at once: Controls -> straight to the main menu).
// Otherwise Escape, which the game's Back buttons also answer to.
static void goBack(const std::vector<Cand>& cs) {
    if (g_inGame && !g_gameplay) {
        MovieClip* sheet = optionsSheet();
        if (!sheet || sheet->cur == 2) { pulse(80, 0); return; }   // the pause screen itself: resume
    }
    for (auto& c : cs) if (c.d == g_modalBtn) { click(c); return; }
    for (auto& c : cs) if (isBackButton(c.d)) { click(c); return; }
    g_afterClick = true; g_clickTick = g_player.tick;
    pulse(27, 27);
}
static void click(const Cand& c) {
    g_afterClick = true; g_clickTick = g_player.tick;
    if (c.d == g_rebindLabel) { g_rebindRequests++; return; }
    double x, y; bool hit = pointOn(c, x, y);
    if (getenv("ISAAC_INPUT_LOG")) logf("[nav] click %s at %.0f,%.0f (hit %d)\n", targetPath(c.d)->s.c_str(), x, y, (int)hit);
    mouseEvent(x, y, 0); mouseEvent(x, y, 1); mouseEvent(x, y, 2);
}

// Escape on the pause screen itself (not one of its sheets): resume, as P does. The pause sheet's "Exit to main
// menu" also answers to Escape, so it used to ask whether to quit the run instead. Logic thread (key events).
bool recompInGameplay(bool* inGame);
bool menuEscapeResumes() {
    bool inGame = false, play = recompInGameplay(&inGame);
    if (!inGame || play) return false;
    MovieClip* sheet = optionsSheet();
    if (sheet && sheet->cur != 2) return false;
    pulse(80, 0);
    return true;
}
// the ring of characters, when the menu shows the character select page (_root.men.men.pos 3): _root.men.men.cha
// (Wrath of the Lamb, Eternal), or in the original the clip with i0 and inc
static MovieClip* charRing() {
    Value sheet = getMemK(getMemK(Value((Object*)vm.root), intern("men")), intern("men"));
    if (!sheet.isObj() || sheet.o->kind != OK_CLIP || toNum(getMemK(sheet, intern("pos"))) != 3) return nullptr;
    Value cha = getMemK(sheet, intern("cha"));
    if (cha.isObj() && cha.o->kind == OK_CLIP) return ((MovieClip*)cha.o)->visible ? (MovieClip*)cha.o : nullptr;
    for (DisplayObject* k : ((MovieClip*)sheet.o)->kids)
        if (k->kind == OK_CLIP && ((MovieClip*)k)->childByName(intern("i0")) && ((MovieClip*)k)->childByName(intern("inc"))) return k->visible ? (MovieClip*)k : nullptr;
    return nullptr;
}
void padNavEvent(int code) {
    if (code == NAV_OFF) { g_navOn = false; return; }
    if (g_uiOpen || g_gameplay) return;
    g_navOn = true;
    std::vector<Cand> cs = candidates();
    if (cs.empty()) {   // a screen without buttons (cutscenes, title cards...): the keys it expects
        static const int arrows[4] = {38, 40, 37, 39};
        if (code <= NAV_RIGHT) pulse(arrows[code], 0);
        else if (code == NAV_CONFIRM) pulse(32, 32);
        else if (code == NAV_BACK) goBack(cs);
        return;
    }
    if (screenMoving(cs) && !findFocus(cs)) return;   // (nothing to choose from until it settles)
    const Cand* f = findFocus(cs);
    bool fresh = !f;
    if (getenv("ISAAC_INPUT_LOG")) logf("[nav] event %d: %d candidates, focus %p %s\n", code, (int)cs.size(), (void*)g_focus, fresh ? "lost" : "kept");
    if (!f) f = ensureFocus(cs);
    if (code == NAV_CONFIRM) { if (!fresh || cs.size() == 1) click(*f); return; }
    if (code == NAV_BACK) { goBack(cs); return; }
    // the character select screen ("Who am I?"): left / right turn the ring of characters (the game's own arrow
    // keys); up / down go through its other options from top to bottom (Back, Hard mode, Select, Random)
    if (MovieClip* ring = charRing()) {
        if (code == NAV_LEFT || code == NAV_RIGHT) { pulse(code == NAV_LEFT ? 37 : 39, 0); return; }
        if (code == NAV_UP || code == NAV_DOWN) {
            if (fresh) return;   // (the first press only shows Select, where the selection starts)
            // (the ring's own buttons: its two arrows, and Random below them - the lowest one)
            std::vector<const Cand*> opts; const Cand* random = nullptr;
            for (auto& c : cs) {
                bool inRing = false; for (DisplayObject* q = c.d; q; q = q->parent) if (q == ring) { inRing = true; break; }
                if (!inRing) opts.push_back(&c);
                else if (!random || c.cy > random->cy) random = &c;
            }
            if (random) opts.push_back(random);
            std::sort(opts.begin(), opts.end(), [](const Cand* a, const Cand* b) { return a->cy < b->cy; });
            if (opts.empty()) return;
            int at = -1; for (size_t i = 0; i < opts.size(); i++) if (opts[i]->d == f->d) at = (int)i;
            int to = at < 0 ? 0 : std::max(0, std::min((int)opts.size() - 1, at + (code == NAV_DOWN ? 1 : -1)));
            g_focus = opts[to]->d; hover(*opts[to]);
            return;
        }
    }
    if (fresh) return;   // the first press only shows where the outline is
    static const double dirs[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
    double dx = dirs[code][0], dy = dirs[code][1];
    // nearest option in that direction: first among those inside a 45-degree cone, then anything ahead. "Sideways"
    // is measured from the current option's center to the nearest point of the other one's span, so a long label
    // whose center is far to the side (Rebind Controls above Back) still counts as straight ahead, while hit areas
    // that overlap a little (the collection's slightly tilted grid) don't make the item to the left count as "below".
    // Along the direction, the distance between the facing edges counts most.
    auto gap = [&](const Cand& c) {   // distance along the direction between the facing edges (can be < 0 if they overlap)
        if (dx > 0) return c.r.x0 - f->r.x1; if (dx < 0) return f->r.x0 - c.r.x1;
        if (dy > 0) return c.r.y0 - f->r.y1; return f->r.y0 - c.r.y1;
    };
    auto sideOff = [&](const Cand& c) {
        double p = dx ? f->cy : f->cx, b0 = dx ? c.r.y0 : c.r.x0, b1 = dx ? c.r.y1 : c.r.x1;
        return std::max(0.0, std::max(b0 - p, p - b1));
    };
    const Cand* best = nullptr; double bs = 1e30;
    for (int pass = 0; pass < 2 && !best; pass++)
        for (auto& c : cs) {
            if (c.d == f->d) continue;
            double vx = c.cx - f->cx, vy = c.cy - f->cy;
            double along = vx * dx + vy * dy, side = std::fabs(vx * dy - vy * dx);
            if (along < 4) continue;
            double so = sideOff(c);
            if (pass == 0 && so > along) continue;               // outside the cone
            if (pass == 1 && dy && so > along * 2) continue;     // (up/down never sideways; left/right reach the
                                                                  //  options at the edge, e.g. Collection -> Exit)
            double sc = std::max(0.0, gap(c)) + along * 0.3 + so + side * 0.2;
            if (sc < bs) { bs = sc; best = &c; }
        }
    if (best) { g_focus = best->d; hover(*best); }
}

// ============================================================ per tick
void menuUiTick(bool gameplay, bool inGame) {
    static bool rooted = false;
    if (!rooted) { rooted = true; gcAddRoot((GcObj**)&g_rebindLabel); gcAddRoot((GcObj**)&g_ui); gcAddRoot((GcObj**)&g_focus); gcAddRoot((GcObj**)&g_focusShow); }
    g_gameplay = gameplay; g_inGame = inGame;
    for (size_t i = 0; i < g_pulses.size();) {
        if (g_player.tick >= g_pulses[i].until) { keyEvent(g_pulses[i].code, 0, false); g_pulses.erase(g_pulses.begin() + i); }
        else i++;
    }
    UiModel m; int ver;
    { std::lock_guard<std::mutex> lk(g_uiMu); ver = g_uiVersion; if (ver != g_uiBuilt) m = g_uiModel; }
    if (ver != g_uiBuilt) { g_uiBuilt = ver; g_uiOpen = m.open; buildUi(m); }
    else if (g_ui && (g_ui->removed || !g_ui->parent)) { g_ui = nullptr; g_uiBuilt = -1; }   // (the game cleared _root: rebuild)
    g_focusShown = false; g_focusShow = nullptr;
    // the mouse over something clickable: the hand cursor (main.cpp)
#ifdef __PSP__
    g_hand = false;   // (no mouse)
#else
    g_hand = !gameplay && !g_uiOpen && (topButtonAt(g_player.mouseX, g_player.mouseY) || labelHit(g_player.mouseX, g_player.mouseY));
#endif
    if (gameplay && !g_uiOpen) { removeLabel(); return; }
    updateRebindLabel();
    if (!g_navOn || g_uiOpen) return;
#ifdef __PSP__
    // (PSP: finding the options takes several ms with emulated doubles: every 4th tick; meanwhile the last result.
    // The objects it refers to are kept alive until then)
    static std::vector<Cand> s_cs; static uint32_t s_csTick = 0; static bool s_moving = false, s_traced = false;
    if (!s_traced) { s_traced = true; g_rootTracers.push_back([](Tracer& tr) { for (auto& c : s_cs) { tr.mark(c.d); tr.mark(c.show); } }); }
    if (s_cs.empty() || g_player.tick - s_csTick >= 4 || g_player.tick < s_csTick) { s_cs = candidates(); s_csTick = g_player.tick; s_moving = screenMoving(s_cs); }
    const std::vector<Cand>& cs = s_cs;
    bool moving = s_moving;
#else
    std::vector<Cand> cs = candidates();
    bool moving = screenMoving(cs);
#endif
    // a press that sets the screen moving (the camera travelling to Stats and back, a sheet closing): nothing is
    // shown until it settles, instead of the old selection sliding away with it
    if (g_afterClick) {
        if (moving) g_hideMoving = true;
        else if (g_player.tick > g_clickTick + 3) g_afterClick = false;
    }
    if (g_hideMoving) { if (moving) return; g_hideMoving = false; }
    const Cand* f = findFocus(cs);   // (what is selected stays shown while things move; a new choice waits until they settle)
    if (!f && !moving) f = ensureFocus(cs);
    if (!f) return;
    g_focusShown = true; g_focusRect = f->r; g_focusShow = f->show;
}

void menuDebugFocus() {   // (testing) eval focus
    std::vector<Cand> cs = candidates();
    const Cand* f = findFocus(cs);
    logf("[focus] %s show=%s rect=%.0f,%.0f-%.0f,%.0f shown=%d (%d options)\n", g_focus ? targetPath(g_focus)->s.c_str() : "-",
         g_focusShow ? targetPath(g_focusShow)->s.c_str() : "-", f ? f->r.x0 : 0, f ? f->r.y0 : 0, f ? f->r.x1 : 0, f ? f->r.y1 : 0,
         (int)g_focusShown, (int)cs.size());
}
void menuUiFill(Snapshot& s) {
    s.rebindRequests = g_rebindRequests;
    Value so = getMemK(Value((Object*)vm.root), intern("so"));
    Value data = so.isObj() ? getMemK(so, intern("data")) : VUNDEF;
    if (data.isObj()) { s.frog = truthy(getMemK(data, intern("frog"))); s.lefty = truthy(getMemK(data, intern("left"))); }
    s.focusOn = g_focusShown;
    s.focusUid = g_focusShown && g_focusShow && !g_focusShow->removed ? g_focusShow->uid : 0;
    s.focusStrong = s.focusUid && g_focus && g_focus == startButton();   // (the main menu's Start: the game's own deep red)
    s.handCursor = g_hand;
    if (!g_focusShown) return;
    double pad = g_stageH / 160.0;
    s.focusRect[0] = (float)(g_focusRect.x0 - pad); s.focusRect[1] = (float)(g_focusRect.y0 - pad);
    s.focusRect[2] = (float)(g_focusRect.x1 + pad); s.focusRect[3] = (float)(g_focusRect.y1 + pad);
}

}  // namespace fl
