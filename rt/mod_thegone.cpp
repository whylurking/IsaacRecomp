// The Gone (mods/TheGone.mod): a new character, in the spirit of Rebirth's The Lost.
//
// Character select: one more place on the ring of characters, after the last one (before Isaac), with a white,
// see-through Isaac and its own card. The ring is the game's (_root.men.men.cha: its onEnterFrame turns it and picks
// _root.skiner from cha.skiner); the mod wraps that function: the game's code runs as for Isaac, then the portraits
// are laid out again on a ring with one more place (same formulas), and the card shows The Gone's.
//   Moving onto it: cha.skiner going past the last character (chars) or below 0 (-1); from it, +1 is Isaac and -1 the
//   last character. While it is selected the game sees Isaac (cha.skiner 0).
//
// In a run started with it (the game plays Isaac):
//   - it starts with Spirit of the Night (given as the game gives starting items) and looks white instead of black
//     (a snapshot-only colour transform of the player: the inverse of the item's dark body);
//   - no hearts: every tick red hearts and containers are 0 and soul hearts at most half a heart (the game's own rule
//     kills when a hit leaves no red and no soul hearts), eternal hearts are dropped, and the HUD's hearts are hidden;
//     hearts on the floor or for sale can't be picked up (they're pushed around, as at full health);
//   - no extra lives: 1up!, Dead Cat and the Ankh are taken off as soon as they're picked up;
//   - devil rooms: one deal for free (the game's own purchase, paid as if it cost coins and given back), and then the
//     other deals of the room disappear.
// Everything goes through the game's own code and state, so a run with The Gone is as repeatable as any other.
#include "mods.h"
#include "core.h"
#include "display.h"
#include "swfdata.h"
#include "snapshot.h"
#include "mod_ui.h"
#include <algorithm>
#include <cmath>
#include <map>
#include <cstdio>
#include <string>
#include <vector>

namespace fl {
bool recompInGameplay(bool* inGame);

namespace {
Value V(const Value& o, const char* k) { return o.isObj() ? getMemK(o, intern(k)) : VUNDEF; }
void S(const Value& o, const char* k, const Value& v) { if (o.isObj()) setMemK(o, intern(k), v); }
double num(const Value& v, double def = 0) { double d = toNum(v); return std::isfinite(d) ? d : def; }
Value root() { return Value((Object*)vm.root); }
std::string game() { return g_game ? g_game->tag : "wotl"; }
Value call(const Value& obj, const char* fn, std::initializer_list<Value> args) {
    std::vector<Value> av(args);
    Act ac{vm.globalScope, VUNDEF, vm.root, nullptr};
    Value r = callMethodK(ac, obj, intern(fn), (int)av.size(), av.data());
    runActions();
    return r;
}
Value arrGet(const Value& a, int i) {
    if (!a.isObj() || a.o->kind != OK_ARRAY) return VUNDEF;
    Array* arr = (Array*)a.o;
    return i >= 0 && i < (int)arr->el.size() ? arr->el[i] : VUNDEF;
}
void arrZero(const Value& a, int i) { if (a.isObj() && a.o->kind == OK_ARRAY && num(arrGet(a, i)) != 0) setMem(a, Value((double)i), Value(0.0)); }

// ============================================================ character select
bool s_sel = false;            // The Gone is the selected character
bool s_goneRun = false;        // the current run is The Gone's
Object* g_cha = nullptr;       // the ring clip whose onEnterFrame is wrapped
Function* g_wrap = nullptr; Function* g_orig = nullptr;
MovieClip* g_portrait = nullptr; MovieClip* g_card = nullptr;
double s_rp = 360, s_rb = 0;
const double kPi = 3.141592653589793;

int charCount() {
    bool more = truthy(arrGet(V(root(), "locker"), 41));
    return game() == "og" ? (more ? 6 : 5) : (more ? 7 : 6);
}
// the portrait clip the game's ring code uses for slot e, and which skiner value shows it in front
MovieClip* slotClip(const Value& cha, int e, int chars) {
    std::string n = "i" + std::to_string(e);
    if (game() == "og") { if (e == 5) n = "i4"; else if (e == 4) n = "i5"; }
    else { if (e == 5) n = chars == 6 ? "i6" : "i4"; else if (e == 4) n = "i5"; }
    Value v = V(cha, n.c_str());
    return v.isObj() && v.o->kind == OK_CLIP ? (MovieClip*)v.o : nullptr;
}
int frontValue(int e, int chars) {   // o (the game's angle offset in places) -> the skiner value that brings it to the front
    static const int og5[] = {0, 2, 3, 4, 1}, og6[] = {0, 3, 4, 5, 2, 1}, w6[] = {0, 3, 2, 1, 4, 5}, w7[] = {0, 3, 2, 1, 4, 5, 6};
    int o;
    if (game() == "og") o = chars == 5 ? og5[e] : og6[e];
    else o = -(chars == 6 ? w6[e] : w7[e]);
    return ((-o) % chars + chars) % chars;
}
void place(DisplayObject* d, double angDeg, int e, bool ghost) {   // (ghost: The Gone's portrait, a little see-through)
    double a = angDeg / 180 * kPi;
    double x = -std::sin(a) * 100 + 100, y = std::cos(a) * 25 + 65;
    Value v((Object*)d);
    setMemK(v, intern("_x"), Value(x)); setMemK(v, intern("_y"), Value(y));
    setMemK(v, intern("_xscale"), Value(y)); setMemK(v, intern("_yscale"), Value(y));
    if (ghost) d->cx.am = (float)std::min(1.0, (y + 20) / 100.0) * 0.8f;
    else setMemK(v, intern("_alpha"), Value(y + 20));
    swapDepths(d, Value(std::round(y) * 10 + e));
}
// ---- The Gone's card in the game's own lettering. The cards of the other characters (the ring's `inc` clip, a frame per
// character) are shapes: name, tally marks and item names drawn by hand. Their letters are taken apart (each
// shape's outlines, grouped into rows and then into letters by where they lie) and put together again into new
// shapes: "THE GONE" and the starting item's name, in the same hand and grey. The four icons (heart, speed, damage,
// item) are the game's own clips, as on every card.
struct Glyph { std::vector<Path> paths; float x0, y0, x1, y1; };
struct GlyphKit { std::map<char, Glyph> g; Glyph tally; bool tallyOk = false; FillStyle fill; bool ok = false; uint16_t heart = 0, speed = 0, dmg = 0, box = 0; double rowY[3] = {0, 41, 75}; double iconX[4] = {0, 66.7, 140.7, 35}; };
struct SubP { Path p; float x0, y0, x1, y1; };
std::vector<SubP> subpaths(const Path& p) {
    std::vector<SubP> out; size_t k = 0;
    for (size_t i = 0; i < p.verbs.size(); i++) {
        uint8_t v = p.verbs[i]; int n = v == 2 ? 4 : 2;
        if (v == 0) out.push_back({Path(), 1e9f, 1e9f, -1e9f, -1e9f});
        if (out.empty()) { k += n; continue; }
        SubP& s = out.back(); s.p.verbs.push_back(v);
        for (int j = 0; j < n; j++) s.p.pts.push_back(p.pts[k + j]);
        for (int j = 0; j < n; j += 2) { float x = p.pts[k + j], y = p.pts[k + j + 1]; s.x0 = std::min(s.x0, x); s.y0 = std::min(s.y0, y); s.x1 = std::max(s.x1, x); s.y1 = std::max(s.y1, y); }
        k += n;
    }
    return out;
}
// rows (top to bottom) of letters (left to right) of a card shape
std::vector<std::vector<Glyph>> letters(ShapeDef* sd, FillStyle* fill) {
    shapeEnsure(sd);
    std::vector<SubP> all;
    for (auto& L : sd->layers) for (size_t f = 0; f < L.fillPaths.size(); f++) {
        if (fill && f < L.fills.size()) *fill = L.fills[f];
        for (auto& sp : subpaths(L.fillPaths[f])) all.push_back(sp);
    }
    std::sort(all.begin(), all.end(), [](const SubP& a, const SubP& b) { return a.y0 + a.y1 < b.y0 + b.y1; });
    std::vector<std::vector<SubP>> rows;
    for (auto& sp : all) {
        double cy = (sp.y0 + sp.y1) / 2;
        if (rows.empty()) { rows.push_back({sp}); continue; }
        double ry0 = 1e9, ry1 = -1e9; for (auto& q : rows.back()) { ry0 = std::min(ry0, (double)q.y0); ry1 = std::max(ry1, (double)q.y1); }
        if (cy > ry1 + 2) rows.push_back({sp}); else rows.back().push_back(sp);
    }
    std::vector<std::vector<Glyph>> out;
    for (auto& r : rows) {
        std::sort(r.begin(), r.end(), [](const SubP& a, const SubP& b) { return a.x0 < b.x0; });
        std::vector<Glyph> gl;
        for (auto& sp : r) {
            // (inside the previous letter, or mostly over it: a hole, a dot, the second stroke of the same letter)
            if (!gl.empty() && sp.x0 < gl.back().x1 - 0.35 * std::min(sp.x1 - sp.x0, gl.back().x1 - gl.back().x0)) {
                Glyph& g = gl.back(); g.paths.push_back(sp.p);
                g.x0 = std::min(g.x0, sp.x0); g.y0 = std::min(g.y0, sp.y0); g.x1 = std::max(g.x1, sp.x1); g.y1 = std::max(g.y1, sp.y1);
            } else gl.push_back({{sp.p}, sp.x0, sp.y0, sp.x1, sp.y1});
        }
        out.push_back(gl);
    }
    return out;
}
ShapeDef* cardShape(const Value& incV, int frame) {   // the lettering shape of card `frame` (1-based)
    if (!incV.isObj() || incV.o->kind != OK_CLIP) return nullptr;
    MovieClip* inc = (MovieClip*)incV.o;
    CharDef* cd = g_movie.get(inc->charId);
    if (!cd || cd->type != CH_SPRITE) return nullptr;
    SpriteDef* sp = (SpriteDef*)cd; spriteEnsure(sp);
    auto depth1 = [](SpriteDef* s, int f) -> uint16_t {
        uint16_t id = 0;
        for (int i = 0; i <= f && i < (int)s->frames.size(); i++)
            for (auto& c : s->frames[i]) if (c.kind == FrameCmd::PLACE && c.place >= 0) { const PlaceCmd& p = s->places[c.place]; if (p.depth == 1 && p.hasId) id = p.id; }
        return id;
    };
    uint16_t id = depth1(sp, frame - 1);
    CharDef* c = g_movie.get(id);
    if (c && c->type == CH_SPRITE) { SpriteDef* s2 = (SpriteDef*)c; spriteEnsure(s2); c = g_movie.get(depth1(s2, 0)); }
    return c && c->type == CH_SHAPE ? (ShapeDef*)c : nullptr;
}
void dumpCards(const Value& inc) {
    for (int f = 1; f <= 7; f++) {
        ShapeDef* sd = cardShape(inc, f); if (!sd) { logf("[gonecard] frame %d: no shape\n", f); continue; }
        auto rows = letters(sd, nullptr);
        for (size_t r = 0; r < rows.size(); r++) {
            std::string o; char b[64];
            for (auto& g : rows[r]) { snprintf(b, sizeof b, " %.0f-%.0f/%.0f-%.0f", g.x0, g.x1, g.y0, g.y1); o += b; }
            logf("[gonecard] frame %d (shape %d) row %zu: %zu letters:%s\n", f, sd->id, r, rows[r].size(), o.c_str());
        }
    }
}
// where each letter is found: (card frame, row, x centre of the letter in that row). The cards (same in the three
// games): 1 ISAAC / tallies / NONE, 2 MAGDALENE / YUM HEART, 3 CAIN / LUCK FOOT, 4 JUDAS / BOOK OF BELIAL,
// 5 EVE / DEAD BIRD / WHORE OF BABYLON, 6 ??? / THE POOP, 7 SAMSON / BLOOD LUST. Item-sized letters (row 2+) where
// possible: a few letters touch their neighbour and come out as one outline (the G of MAGDALENE): G is a C with a bar.
struct LetterSrc { char c; int frame, row; double cx; };
const LetterSrc kLetters[] = {
    {'T', 2, 2, 174.6}, {'H', 2, 2, 106}, {'E', 2, 2, 123}, {'A', 2, 2, 138}, {'R', 2, 2, 159.5}, {'U', 2, 2, 63.8},
    {'Y', 2, 2, 50.6}, {'M', 2, 2, 79.4}, {'N', 1, 2, 82.5}, {'O', 1, 2, 99.5}, {'L', 3, 2, 58}, {'C', 3, 2, 89.5},
    {'K', 3, 2, 104}, {'F', 3, 2, 127}, {'D', 5, 2, 53}, {'B', 5, 2, 119.5}, {'I', 5, 2, 133}, {'P', 6, 3, 117.5},
    {'S', 1, 0, 75.5}, {'J', 4, 0, 62}, {'V', 5, 0, 95},
};
struct CardKit {
    bool ok = false; FillStyle fill; std::map<char, Glyph> g;
    std::vector<Glyph> tallies;                                  // Isaac's speed and damage tallies (II, II), in place
    struct Icon { uint16_t id; double x, y; }; std::vector<Icon> icons;   // heart, speed, damage, item box
    double nameCy = 12, itemCy = 89;
};
CardKit g_kit;
void buildKit(const Value& inc) {
    g_kit = CardKit();
    std::map<int, std::vector<std::vector<Glyph>>> rows;
    for (int f = 1; f <= 7; f++) if (ShapeDef* sd = cardShape(inc, f)) rows[f] = letters(sd, f == 1 ? &g_kit.fill : nullptr);
    for (const LetterSrc& L : kLetters) {
        auto it = rows.find(L.frame);
        if (it == rows.end() || L.row >= (int)it->second.size()) return;
        const Glyph* best = nullptr; double bd = 1e9;
        for (const Glyph& gl : it->second[L.row]) { double d = std::fabs((gl.x0 + gl.x1) / 2 - L.cx); if (d < bd) { bd = d; best = &gl; } }
        if (!best || bd > 4) { logf("thegone: letter %c not found on the cards\n", L.c); return; }
        g_kit.g[L.c] = *best;
    }
    if (rows[1].size() < 3) return;
    for (const Glyph& gl : rows[1][1]) if (gl.x0 > 100) g_kit.tallies.push_back(gl);
    g_kit.nameCy = (rows[1][0].front().y0 + rows[1][0].front().y1) / 2 + 3;
    g_kit.itemCy = (rows[1][2].front().y0 + rows[1][2].front().y1) / 2;
    // the icons: the clips placed on Isaac's card (frame 1) besides its lettering
    if (MovieClip* m = (MovieClip*)inc.o) {
        SpriteDef* sp = (SpriteDef*)g_movie.get(m->charId);
        uint16_t sub = 0;
        for (auto& c : sp->frames[0]) if (c.kind == FrameCmd::PLACE && c.place >= 0 && sp->places[c.place].depth == 1) sub = sp->places[c.place].id;
        CharDef* cd = g_movie.get(sub);
        if (cd && cd->type == CH_SPRITE) {
            SpriteDef* s2 = (SpriteDef*)cd; spriteEnsure(s2);
            for (auto& c : s2->frames[0]) if (c.kind == FrameCmd::PLACE && c.place >= 0) {
                const PlaceCmd& pc = s2->places[c.place];
                if (pc.depth > 1 && pc.hasId) g_kit.icons.push_back({pc.id, pc.m.tx, pc.m.ty});
            }
        }
    }
    g_kit.ok = g_kit.icons.size() == 4 && g_kit.tallies.size() == 4;
    if (!g_kit.ok) logf("thegone: the character cards are not as expected (%zu icons, %zu tallies)\n", g_kit.icons.size(), g_kit.tallies.size());
}
// a text in the cards' lettering, as one shape: centred on (cx, cy), letters scaled by k, at most maxW wide
uint16_t letteringShape(const std::string& text, double k, double cx, double cy, double maxW, double* outX0 = nullptr, bool left = false) {
    const double gap = 1.2, space = 7;
    auto glyph = [](char c) -> const Glyph* { auto it = g_kit.g.find(c == 'G' ? 'C' : c); return it == g_kit.g.end() ? nullptr : &it->second; };
    double w = 0;
    for (char c : text) { if (c == ' ') { w += space; continue; } if (const Glyph* g = glyph(c)) w += (g->x1 - g->x0) + gap; }
    w -= gap;
    double kk = w * k > maxW ? maxW / w : k;
    static std::map<std::string, std::pair<uint16_t, double>> cache;   // (built once: the shapes stay)
    char key[160]; snprintf(key, sizeof key, "%s|%g|%g|%g|%g|%d", text.c_str(), k, cx, cy, maxW, (int)left);
    auto ci = cache.find(key);
    if (ci != cache.end()) { if (outX0) *outX0 = ci->second.second; return ci->second.first; }
    auto* sd = new ShapeDef(); sd->type = CH_SHAPE;
    ShapeLayer L; L.fills.push_back(g_kit.fill);
    Path out;
    double x = left ? cx : cx - w * kk / 2, x0 = x;
    if (outX0) *outX0 = x0;
    float bx0 = 1e9f, by0 = 1e9f, bx1 = -1e9f, by1 = -1e9f;
    auto addPath = [&](const Path& p, double ox, double oy, double sx, double sy, double gx0, double gcy) {
        size_t k2 = 0;
        for (uint8_t v : p.verbs) {
            int n = v == 2 ? 4 : 2;
            out.verbs.push_back(v);
            for (int j = 0; j < n; j += 2) {
                float X = (float)(ox + (p.pts[k2 + j] - gx0) * sx), Y = (float)(oy + (p.pts[k2 + j + 1] - gcy) * sy);
                out.pts.push_back(X); out.pts.push_back(Y);
                bx0 = std::min(bx0, X); by0 = std::min(by0, Y); bx1 = std::max(bx1, X); by1 = std::max(by1, Y);
            }
            k2 += n;
        }
    };
    for (char c : text) {
        if (c == ' ') { x += space * kk; continue; }
        const Glyph* g = glyph(c); if (!g) continue;
        double gcy = (g->y0 + g->y1) / 2, gw = g->x1 - g->x0;
        for (const Path& p : g->paths) addPath(p, x, cy, kk, k, g->x0, gcy);
        if (c == 'G') {   // the bar that makes a C a G
            double bxa = x + gw * kk * 0.42, bxb = x + gw * kk * 0.98, bya = cy + (g->y1 - gcy) * k * 0.05, byb = bya + 3.2 * k;
            Path bar; bar.verbs = {0, 1, 1, 1, 1}; bar.pts = {(float)bxa, (float)bya, (float)bxb, (float)(bya - 0.4), (float)bxb, (float)byb, (float)bxa, (float)(byb + 0.3), (float)bxa, (float)bya};
            addPath(bar, 0, 0, 1, 1, 0, 0);
        }
        x += (gw + gap) * kk;
    }
    L.fillPaths.push_back(out);
    sd->layers.push_back(L);
    sd->bounds = {bx0, by0, bx1, by1};
    uint16_t id = synthAddChar(sd);
    cache[key] = {id, x0};
    return id;
}
void buildCard(MovieClip* cha) {
    g_card = createEmptyMovieClip(cha, intern("__gonecard"), 30);
    if (!g_card) return;
    Value inc = V(Value((Object*)cha), "inc");
    if (inc.isObj() && inc.o->kind == OK_CLIP) {   // in the card's own place
        g_card->m = ((MovieClip*)inc.o)->m; g_card->scriptXform = true;
        static bool tried = false;
        if (!tried) { tried = true; buildKit(inc); }
    }
    if (!g_kit.ok) return;
    int dp = 1;
    // name (the names are a little larger than the item names the letters come from)
    synthPut(g_card, letteringShape("THE GONE", 1.1, 95, g_kit.nameCy, 112), dp++, 0, 0);
    // stats: hearts (none), speed and damage as Isaac's (his speed; 3.5 damage), the icons where the cards have them
    for (int i = 0; i < 3; i++) synthPut(g_card, g_kit.icons[i].id, dp++, g_kit.icons[i].x, g_kit.icons[i].y);
    static uint16_t tallyId = 0;
    if (!tallyId) {
        auto* sd = new ShapeDef(); sd->type = CH_SHAPE;
        ShapeLayer L; L.fills.push_back(g_kit.fill); Path all; float bx0 = 1e9f, by0 = 1e9f, bx1 = -1e9f, by1 = -1e9f;
        for (const Glyph& t : g_kit.tallies) {
            for (const Path& p : t.paths) { all.verbs.insert(all.verbs.end(), p.verbs.begin(), p.verbs.end()); all.pts.insert(all.pts.end(), p.pts.begin(), p.pts.end()); }
            bx0 = std::min(bx0, t.x0); by0 = std::min(by0, t.y0); bx1 = std::max(bx1, t.x1); by1 = std::max(by1, t.y1);
        }
        L.fillPaths.push_back(all); sd->layers.push_back(L); sd->bounds = {bx0, by0, bx1, by1};
        tallyId = synthAddChar(sd);
    }
    synthPut(g_card, tallyId, dp++, 0, 0);
    // the starting item(s): the item box and the name, the pair centred as on the cards (one row, or two like Eve's)
    std::vector<std::string> items = game() == "og" ? std::vector<std::string>{"TRANSCENDENCE", "OUIJA BOARD"} : std::vector<std::string>{"SPIRIT OF THE NIGHT"};
    const CardKit::Icon& box = g_kit.icons[3];
    for (size_t i = 0; i < items.size(); i++) {
        double cy = g_kit.itemCy + i * 31.6, boxW = 34, maxText = 172;
        // measure at full size, then centre box + text around the cards' middle (~91)
        double x0 = 0;
        uint16_t probe = letteringShape(items[i], 1.0, 0, cy, maxText, &x0, true);
        CharDef* pd = g_movie.get(probe);
        double tw = pd ? ((ShapeDef*)pd)->bounds.x1 - ((ShapeDef*)pd)->bounds.x0 : 100;
        double left = 91 - (boxW + 6 + tw) / 2;
        synthPut(g_card, box.id, dp++, left, box.y + (cy - g_kit.itemCy));
        synthPut(g_card, probe, dp++, left + boxW + 6, 0);
    }
}
Value wRing(Act& ac, Value thisv, int argc, Value* argv) {
    Value cha = thisv.isObj() ? thisv : Value(g_cha);
    int chars = charCount();
    double raw = num(V(cha, "skiner"));
    if (!s_sel) {
        if (raw == chars || raw == -1) { s_sel = true; S(cha, "skiner", Value(0.0)); }
    } else if (raw != 0) {
        s_sel = false;
        S(cha, "skiner", Value(raw == -1 ? chars - 1.0 : raw == 1 ? 0.0 : raw));
    }
    Value r = g_orig ? callValue(ac, Value((Object*)g_orig), thisv, argc, argv) : VUNDEF;
    if (!cha.isObj() || cha.o->kind != OK_CLIP) return r;
    MovieClip* c = (MovieClip*)cha.o;
    // the ring again, with one more place: The Gone's, at skiner value `chars`
    int n = chars + 1;
    double step = 360.0 / n, v = s_sel ? chars : num(V(cha, "skiner"));
    double x = s_rp + s_rb * 3 - v * step;
    while (x >= 180) x -= 360;
    while (x < -180) x += 360;
    while (s_rp < 180) s_rp += 360;
    s_rb -= x * 0.1; s_rb *= 0.8; s_rp += s_rb;
    for (int e = 0; e < chars; e++) if (MovieClip* t = slotClip(cha, e, chars)) place(t, s_rp - frontValue(e, chars) * step, e, false);
    if (!g_portrait || g_portrait->removed || g_portrait->parent != c) {
        MovieClip* i0 = slotClip(cha, 0, chars);
        g_portrait = nullptr;
        if (i0) {
            // (a free depth: the portraits use round(y) * 10 + e, 400-910, and placing at a used one would replace it)
            DisplayObject* d = synthPut(c, i0->charId, 5000, 100, 65, "__gone");
            if (d && d->kind == OK_CLIP) {
                g_portrait = (MovieClip*)d;
                gotoFrame(g_portrait, 1, true);
                // white and pale: the drawing's dark lines become light grey
                g_portrait->cx.rm = g_portrait->cx.gm = g_portrait->cx.bm = 0.35f;
                g_portrait->cx.ra = g_portrait->cx.ga = g_portrait->cx.ba = 160;
                g_portrait->scriptColor = true;
            }
        }
    }
    if (g_portrait) place(g_portrait, s_rp - chars * step, chars, true);
    if (getenv("ISAAC_GONECARDS")) { static bool once = false; if (!once) { once = true; dumpCards(V(cha, "inc")); } }
    if (!g_card || g_card->removed || g_card->parent != c) buildCard(c);
    if (g_card) g_card->visible = s_sel;
    return r;
}
// "Random" (the card's button, or R): the game picks one of the unlocked characters (cha.rand, then starts the run);
// The Gone is one more choice, as likely as each of the others
Function* g_randWrap = nullptr; Function* g_randOrig = nullptr;
Value wRander(Act& ac, Value thisv, int argc, Value* argv) {
    Value cha = thisv.isObj() ? thisv : Value(g_cha);
    bool wasDone = truthy(V(cha, "done"));
    Value rand = V(cha, "rand");
    int n = rand.isObj() && rand.o->kind == OK_ARRAY ? (int)((Array*)rand.o)->el.size() - 1 : 0;   // (rand = [0] + the unlocked)
    // (its own little generator: the game's random numbers stay as they would be without the mod)
    static uint32_t seed = 0; seed = seed * 1664525u + 1013904223u + g_player.tick * 2654435761u;
    bool gone = n > 0 && (seed >> 8) % (uint32_t)(n + 1) == (uint32_t)n;
    Value r = g_randOrig ? callValue(ac, Value((Object*)g_randOrig), thisv, argc, argv) : VUNDEF;
    if (gone && !wasDone && truthy(V(cha, "done"))) { S(cha, "skiner", Value(0.0)); s_sel = true; logf("thegone: Random picked The Gone\n"); }
    else if (!wasDone && truthy(V(cha, "done"))) s_sel = false;
    return r;
}
// wraps the ring's onEnterFrame (a new ring each time the menu is built)
void ringTick() {
    // (the ring: _root.men.men.cha in Wrath of the Lamb and Eternal, unnamed in the original: the clip with i0 and inc)
    Value sheet = V(V(root(), "men"), "men");
    if (!sheet.isObj() || sheet.o->kind != OK_CLIP) return;
    Value cha = V(sheet, "cha");
    if (!cha.isObj() || cha.o->kind != OK_CLIP) {
        cha = VUNDEF;
        for (DisplayObject* k : ((MovieClip*)sheet.o)->kids)
            if (k->kind == OK_CLIP && ((MovieClip*)k)->childByName(intern("i0")) && ((MovieClip*)k)->childByName(intern("inc"))) { cha = Value((Object*)k); break; }
        if (!cha.isObj()) return;
    }
    if (cha.o != g_cha) { g_cha = cha.o; g_orig = nullptr; s_sel = false; s_rp = 360; s_rb = 0; g_portrait = nullptr; g_card = nullptr; }
    if (!g_wrap) { Function* f = gcNew<Function>(); f->proto = protos::function; f->nat = wRing; f->dbgName = "thegoneRing"; gcPin(f); g_wrap = f; }
    if (!g_randWrap) { Function* f = gcNew<Function>(); f->proto = protos::function; f->nat = wRander; f->dbgName = "thegoneRandom"; gcPin(f); g_randWrap = f; gcAddRoot((GcObj**)&g_randOrig); }
    Value rv = V(cha, "rander");
    if (rv.isObj() && rv.o->kind == OK_FUNCTION && rv.o != (Object*)g_randWrap) { g_randOrig = (Function*)rv.o; S(cha, "rander", Value((Object*)g_randWrap)); }
    Value cur = V(cha, "onEnterFrame");
    if (!(cur.isObj() && cur.o == (Object*)g_wrap) && cur.isObj() && cur.o->kind == OK_FUNCTION) {
        g_orig = (Function*)cur.o;
        S(cha, "onEnterFrame", Value((Object*)g_wrap));
    }
    // the card: the game's (Isaac's) is hidden while The Gone is selected
    if (s_sel) { Value inc = V(cha, "inc"); if (inc.isObj() && inc.o->isDisplay()) g_snapOv[(DisplayObject*)inc.o].hide = true; }
}

// ============================================================ the run
bool s_running = false;
MovieClip* g_hudPatch = nullptr;   // covers the HUD's "-LIFE-" (drawn in the HUD band's own colour)
double s_dealRoom = -1e9; bool s_dealTaken = false;
Function* g_bjWrap = nullptr; Function* g_bjOrig = nullptr; Object* g_bjA = nullptr;
const int kShopPrice[] = {0, 3, 5, 5, 15, 2, 3, 3, 7, 1, 2, 3, 5, 5, 0, 0, 3, 3};   // the game's shop / deal prices by slot frame

Value wBalljunk(Act& ac, Value thisv, int argc, Value* argv) {
    auto orig = [&]() { return g_bjOrig ? callValue(ac, Value((Object*)g_bjOrig), thisv, argc, argv) : VUNDEF; };
    if (!s_goneRun) return orig();
    Value a((Object*)g_bjA), lows = V(a, "lows"), highs = V(a, "highs"), player = V(a, "player");
    if (!player.isObj() || !lows.isObj() || lows.o != player.o || !highs.isObj() || num(V(highs, "s")) != 5 || truthy(V(highs, "dones"))) return orig();
    Value d = V(highs, "d");
    if (!d.isObj() || d.o->kind != OK_CLIP) return orig();
    int fr = ((MovieClip*)d.o)->cur;
    Value dd = V(d, "d");
    int sub = dd.isObj() && dd.o->kind == OK_CLIP ? ((MovieClip*)dd.o)->cur : 0;
    if (fr == 1 || (fr == 15 && (sub == 1 || sub == 5 || sub == 13 || sub == 17))) {   // a heart (on the floor or for sale)
        S(a, "nohit", VFALSE);
        return VUNDEF;
    }
    if (fr == 15 && truthy(V(a, "satan"))) {   // a devil deal
        double lev = num(V(root(), "lev"));
        if (lev != s_dealRoom) { s_dealRoom = lev; s_dealTaken = false; }
        if (s_dealTaken) { S(a, "nohit", VFALSE); return VUNDEF; }
        double price = sub >= 0 && sub < (int)(sizeof kShopPrice / sizeof kShopPrice[0]) ? kShopPrice[sub] : 0;
        Value coins = V(root(), "coins");
        S(a, "satan", VFALSE); S(root(), "coins", Value(num(coins) + price));
        Value r = orig();
        S(a, "satan", VTRUE);
        if (truthy(V(highs, "done"))) {
            s_dealTaken = true;
            // what the game counts for a deal taken (angel rooms, the ??? unlock)
            double evs = num(V(root(), "evs")) + 1; S(root(), "evs", Value(evs));
            if (evs > 1) { Value lk = V(root(), "locker"); if (lk.isObj()) setMem(lk, Value(54.0), VTRUE); }
            // the other deals disappear
            Value balls = V(a, "ball");
            if (balls.isObj() && balls.o->kind == OK_ARRAY)
                for (const Value& b : ((Array*)balls.o)->el) {
                    if (!b.isObj() || b.o == highs.o || num(V(b, "s")) != 5 || truthy(V(b, "done"))) continue;
                    Value bd = V(b, "d");
                    if (bd.isObj() && bd.o->kind == OK_CLIP && ((MovieClip*)bd.o)->cur == 15) S(b, "done", VTRUE);
                }
        } else S(root(), "coins", coins);
        return r;
    }
    return orig();
}
void hookBalljunk(const Value& a) {
    if (!g_bjWrap) {
        Function* f = gcNew<Function>(); f->proto = protos::function; f->nat = wBalljunk; f->dbgName = "thegoneBalljunk"; gcPin(f);
        g_bjWrap = f; gcAddRoot((GcObj**)&g_bjOrig); gcAddRoot((GcObj**)&g_bjA);
    }
    Value cur = V(a, "balljunk");
    if (cur.isObj() && cur.o != (Object*)g_bjWrap && cur.o->kind == OK_FUNCTION) {
        g_bjOrig = (Function*)cur.o; g_bjA = a.o;
        S(a, "balljunk", Value((Object*)g_bjWrap));
    }
}
void startRun(const Value& a) {
    Value p = V(a, "player");
    if (!p.isObj()) return;
    // Spirit of the Night, as the game gives a character's starting items (the original game doesn't have it: its
    // two effects as two items, Transcendence and the Ouija Board)
    for (double it : game() == "og" ? std::vector<double>{20, 115} : std::vector<double>{159}) {
        S(p, "empty", VFALSE); S(a, "lastcra", Value(-100.0)); S(a, "highs", p); S(p, "it", Value(it));
        call(a, "powerlevel", {});
    }
    S(root(), "armor", Value(game() == "og" ? 0.0 : 0.5));
    logf("thegone: a run as The Gone\n");
}
}  // namespace

void thegoneTick(const ModInfo&) {
    static bool rooted = false;
    if (!rooted) {
        rooted = true;
        for (GcObj** r : {(GcObj**)&g_cha, (GcObj**)&g_orig, (GcObj**)&g_portrait, (GcObj**)&g_card, (GcObj**)&g_hudPatch}) gcAddRoot(r);
    }
    Value R = root();
    int frame = vm.root->kind == OK_CLIP ? ((MovieClip*)vm.root)->cur : 0;
    if (frame == 5) ringTick();
    // the run
    Value a = V(R, "a");
    bool inGame = false, play = recompInGameplay(&inGame);
    bool newRun = truthy(V(R, "newstartt"));
    static uint32_t serial = 0; bool restarted = modsRunSerial() != serial; serial = modsRunSerial();
    if (s_running && (frame == 5 || newRun || restarted)) { s_running = false; s_goneRun = false; }
    if (!s_running && play && !newRun && frame != 5) {
        s_running = true; s_goneRun = s_sel;
        if (s_goneRun) startRun(a);
    }
    if (!s_goneRun || !inGame || !a.isObj()) {
        if (g_hudPatch && !g_hudPatch->removed && g_hudPatch->parent) removeObject(g_hudPatch);
        g_hudPatch = nullptr;
        return;
    }
    hookBalljunk(a);
    Value p = V(a, "player"), ups = V(R, "ups"), aups = V(a, "ups");
    if (p.isObj() && !getenv("ISAAC_GONE_NOHP")) {
        // (the original game only hurts a player with hp > 0: there red hearts are kept at half a heart, which is
        // never drawn - the HUD's hearts are hidden - and no soul heart (armor 0): a hit takes the half heart, dead.
        // With a soul heart the hit took it and he lived on: that game recomputes his heart containers (mhp) every
        // frame, so its "no hearts left" test never saw 0 containers)
        double hp0 = game() == "og" ? 0.5 : 0;
        if (num(V(p, "hp")) != hp0 && !truthy(V(p, "dones"))) S(p, "hp", Value(hp0));
        if (num(V(p, "mhp")) != 0) S(p, "mhp", Value(0.0));
    }
    double armorMax = game() == "og" ? 0 : 0.5;
    if (num(V(R, "armor")) > armorMax) S(R, "armor", Value(armorMax));
    if (truthy(V(R, "eta"))) S(R, "eta", VFALSE);
    for (int i : {11, 81, 161}) { arrZero(ups, i); arrZero(aups, i); }
    if (num(V(R, "catlives")) != 0) S(R, "catlives", Value(0.0));
    // drawn: no hearts in the HUD, and a white body
    Value hud = V(R, "hud"), lif = V(hud, "lif");
    if (!getenv("ISAAC_GONE_NOHUD") && lif.isObj() && lif.o->isDisplay()) g_snapOv[(DisplayObject*)lif.o].hide = true;
    if (hud.isObj() && hud.o->kind == OK_CLIP && (!g_hudPatch || g_hudPatch->removed || g_hudPatch->parent != (MovieClip*)hud.o)) {
        g_hudPatch = createEmptyMovieClip((MovieClip*)hud.o, intern("__thegone"), 1047920);
        int dp = 1;
        if (g_hudPatch) modui::rect(g_hudPatch, dp, 498, 2, 92, 22, 0xff1b1818u);
    }
    if (p.isObj() && p.o->isDisplay() && !getenv("ISAAC_GONE_NOCX")) {
        SnapOv& o = g_snapOv[(DisplayObject*)p.o];
        o.hasCx = true;
        if (game() == "og") { o.cx.rm = o.cx.gm = o.cx.bm = 0.3f; o.cx.ra = o.cx.ga = o.cx.ba = 175; }   // (pale: no dark body to turn)
        else { o.cx.rm = o.cx.gm = o.cx.bm = -0.8f; o.cx.ra = o.cx.ga = o.cx.ba = 235; }   // (dark -> light, light -> dark)
        o.cx.am = 0.85f;
    }
}
}  // namespace fl
