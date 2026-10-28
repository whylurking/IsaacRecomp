// StatOn (mods/StatOn.mod): the character's stats on the left of the screen during a run, each with its symbol and
// value, and for a moment the change when one goes up (green) or down (red). Inspired by Repentance's HUD.
//
// Read-only: the values come from the game's own variables; the damage from the game's dmgdo(), called with the
// random number generator and the two variables that function writes (_root.firrb, chal) put back as they were,
// so a run plays exactly the same with the mod on or off. Drawn inside _root.hud (shown, hidden and scaled with
// the game's HUD) with synthesized shapes and the HUD's font, like the rebinding screen (menuui.cpp).
//   Speed       _root.playsp                                  (1.00 at the start)
//   Tears       30 / (_root.firra + 1): shots per second       (2.73)
//   Damage      _root.a.dmgdo(), x1.15 with the items that boost tears (Eternal)   (3.50)
//   Range       _root.firrr scaled so the start is 6.50        (_root.firrr: 23)
//   Shot speed  the tears' speed multiplier (Wrath of the Lamb, Eternal; formulas below)   (1.00)
//   Luck        _root.luck (Wrath of the Lamb, Eternal; the original has none: the row is hidden)
//   Devil / angel room: the chance that the boss room of this floor opens a door to the devil room (or, in Wrath of
//   the Lamb and Eternal, the angel room), with the game's own formula (see dealChance)
#include "mods.h"
#include "core.h"
#include "display.h"
#include "swfdata.h"
#include <cmath>
#include <cstdio>
#include <cstring>

namespace fl {
bool recompInGameplay(bool* inGame);

namespace {
// ---- symbols (16x16 units): light fill with a dark outline, like the HUD's own icons
struct Icon {
    ShapeDef* s = new ShapeDef();
    ShapeLayer L;
    float k = 1.f;
    Icon() { s->type = CH_SHAPE; }
    int fill(uint32_t argb) { FillStyle f; f.type = 0; f.argb = argb; L.fills.push_back(f); L.fillPaths.emplace_back(); return (int)L.fills.size() - 1; }
    int line(float w, uint32_t argb) { LineStyle l; l.width = w * k; l.argb = argb; l.startCap = l.endCap = 0; l.join = 0; L.lines.push_back(l); L.linePaths.emplace_back(); return (int)L.lines.size() - 1; }
    static void mv(Path& p, float x, float y, float k) { p.verbs.push_back(0); p.pts.push_back(x * k); p.pts.push_back(y * k); }
    static void ln(Path& p, float x, float y, float k) { p.verbs.push_back(1); p.pts.push_back(x * k); p.pts.push_back(y * k); }
    static void qd(Path& p, float cx, float cy, float x, float y, float k) { p.verbs.push_back(2); p.pts.insert(p.pts.end(), {cx * k, cy * k, x * k, y * k}); }
    // a closed outline: points, and 'q' entries (NAN, cx, cy, x, y) for curves
    void shape(Path& p, std::initializer_list<float> pts) {
        std::vector<float> v(pts); size_t i = 0; bool first = true; float x0 = 0, y0 = 0;
        while (i < v.size()) {
            if (std::isnan(v[i])) { qd(p, v[i + 1], v[i + 2], v[i + 3], v[i + 4], k); i += 5; continue; }
            if (first) { mv(p, v[i], v[i + 1], k); x0 = v[i]; y0 = v[i + 1]; first = false; } else ln(p, v[i], v[i + 1], k);
            i += 2;
        }
        ln(p, x0, y0, k);
    }
    void circle(Path& p, float cx, float cy, float r) {
        const float c = r / std::cos(3.14159265f / 8);
        mv(p, cx + r, cy, k);
        for (int i = 0; i < 8; i++) {
            float a0 = i * 3.14159265f / 4, am = a0 + 3.14159265f / 8, a1 = a0 + 3.14159265f / 4;
            qd(p, cx + c * std::cos(am), cy + c * std::sin(am), cx + r * std::cos(a1), cy + r * std::sin(a1), k);
        }
    }
    void open(Path& p, std::initializer_list<float> pts) {
        std::vector<float> v(pts);
        for (size_t i = 0; i + 1 < v.size(); i += 2) { if (i == 0) mv(p, v[0], v[1], k); else ln(p, v[i], v[i + 1], k); }
    }
    uint16_t done() { s->layers.push_back(L); s->bounds = {-2, -2, 16 * k + 2, 16 * k + 2}; return synthAddChar(s); }
};
const uint32_t kInk = 0xff141414u, kLight = 0xffefe9dcu, kShade = 0xffb9b1a3u;
const float Q = NAN;

uint16_t iconSpeed() {   // a running shoe with speed lines
    Icon ic; int f = ic.fill(kLight), o = ic.line(1.5f, kInk), m = ic.line(1.3f, kInk);
    Path p;
    ic.shape(p, {6, 1.5f, 11.5f, 1.5f, 11.5f, 7.5f, Q, 15.5f, 8.5f, 15.5f, 12, 15.5f, 14.5f, 4.5f, 14.5f, Q, 3, 14.5f, 3.5f, 11, 6, 8.5f});
    ic.L.fillPaths[f] = p; ic.L.linePaths[o] = p;
    ic.open(ic.L.linePaths[m], {6.5f, 11.5f, 14.f, 11.5f});   // the sole
    Path s; ic.open(s, {0.5f, 5, 4, 5}); Icon::mv(s, 0.5f, 8.5f, ic.k); Icon::ln(s, 3.5f, 8.5f, ic.k); Icon::mv(s, 1.5f, 12, ic.k); Icon::ln(s, 3, 12, ic.k);
    int sl = ic.line(1.4f, kLight); ic.L.linePaths[sl] = s;
    return ic.done();
}
uint16_t iconTears() {   // a tear
    Icon ic; int f = ic.fill(kLight), o = ic.line(1.5f, kInk), h = ic.fill(0xffffffffu);
    Path p; ic.shape(p, {8, 1, Q, 14, 8.5f, 13.5f, 11, Q, 13, 15, 8, 15, Q, 3, 15, 2.5f, 11, Q, 2, 8.5f, 8, 1});
    ic.L.fillPaths[f] = p; ic.L.linePaths[o] = p;
    ic.circle(ic.L.fillPaths[h], 6, 11, 1.4f);   // shine
    return ic.done();
}
uint16_t iconDamage() {   // a sword, point up and to the right
    Icon ic; int f = ic.fill(kLight), o = ic.line(1.4f, kInk), g = ic.line(3.4f, kInk), gi = ic.line(1.6f, kShade);
    Path blade; ic.shape(blade, {15.2f, 0.8f, 14.6f, 4.4f, 7.4f, 11.6f, 4.4f, 8.6f, 11.6f, 1.4f});
    ic.L.fillPaths[f] = blade; ic.L.linePaths[o] = blade;
    ic.open(ic.L.linePaths[g], {3, 8.5f, 7.5f, 13}); Icon::mv(ic.L.linePaths[g], 5.2f, 10.8f, ic.k); Icon::ln(ic.L.linePaths[g], 1.8f, 14.2f, ic.k);
    ic.open(ic.L.linePaths[gi], {3, 8.5f, 7.5f, 13}); Icon::mv(ic.L.linePaths[gi], 5.2f, 10.8f, ic.k); Icon::ln(ic.L.linePaths[gi], 1.8f, 14.2f, ic.k);
    return ic.done();
}
uint16_t iconRange() {   // a target
    Icon ic; int o = ic.line(3.2f, kInk), w = ic.line(1.5f, kLight), c = ic.fill(kLight);
    Path r; ic.circle(r, 8, 8, 6.3f); ic.circle(r, 8, 8, 2.9f);
    ic.L.linePaths[o] = r; ic.L.linePaths[w] = r;
    ic.circle(ic.L.fillPaths[c], 8, 8, 1.1f);
    return ic.done();
}
uint16_t iconShotSpeed() {   // an arrow with speed lines
    Icon ic; int f = ic.fill(kLight), o = ic.line(1.5f, kInk);
    Path p; ic.shape(p, {5, 6, 9, 6, 9, 2.5f, 15, 8, 9, 13.5f, 9, 10, 5, 10});
    ic.L.fillPaths[f] = p; ic.L.linePaths[o] = p;
    Path s; ic.open(s, {0.8f, 6, 3.2f, 6}); Icon::mv(s, 0.8f, 10, ic.k); Icon::ln(s, 3.2f, 10, ic.k);
    int so = ic.line(3.0f, kInk), sw = ic.line(1.4f, kLight); ic.L.linePaths[so] = s; ic.L.linePaths[sw] = s;
    return ic.done();
}
uint16_t iconLuck() {   // a four-leaf clover
    Icon ic; int st = ic.line(3.4f, kInk), stw = ic.line(1.4f, kLight), f = ic.fill(kLight), o = ic.line(1.4f, kInk);
    ic.open(ic.L.linePaths[st], {8.5f, 9, 13, 15}); ic.open(ic.L.linePaths[stw], {8.5f, 9, 13, 15});
    Path p; ic.circle(p, 5, 5, 3.1f); ic.circle(p, 11, 5, 3.1f); ic.circle(p, 5, 11, 3.1f); ic.circle(p, 11, 11, 3.1f);
    ic.L.linePaths[o] = p; ic.L.fillPaths[f] = p;
    return ic.done();
}
uint16_t iconDevil() {   // a horned head
    Icon ic; int f = ic.fill(0xffd8574bu), o = ic.line(1.5f, kInk), e = ic.fill(kInk);
    Path p;
    ic.shape(p, {3.6f, 6.4f, 1.2f, 0.8f, 6.2f, 4.4f, Q, 8, 3.9f, 9.8f, 4.4f, 14.8f, 0.8f, 12.4f, 6.4f,
                 Q, 14, 8.4f, 13.6f, 10.8f, Q, 12.6f, 15.4f, 8, 15.4f, Q, 3.4f, 15.4f, 2.4f, 10.8f, Q, 2, 8.4f, 3.6f, 6.4f});
    ic.L.fillPaths[f] = p; ic.L.linePaths[o] = p;
    ic.shape(ic.L.fillPaths[e], {4.6f, 9.2f, 7.2f, 10.4f, 5.2f, 11.4f});
    ic.shape(ic.L.fillPaths[e], {11.4f, 9.2f, 8.8f, 10.4f, 10.8f, 11.4f});
    return ic.done();
}
uint16_t iconAngel() {   // a halo over a head
    Icon ic; int h = ic.line(3.4f, kInk), hw = ic.line(1.6f, 0xfff6dd7au), f = ic.fill(kLight), o = ic.line(1.5f, kInk);
    Path halo; Icon::mv(halo, 2.4f, 3.4f, ic.k);
    Icon::qd(halo, 2.4f, 0.6f, 8, 0.6f, ic.k); Icon::qd(halo, 13.6f, 0.6f, 13.6f, 3.4f, ic.k);
    Icon::qd(halo, 13.6f, 6.2f, 8, 6.2f, ic.k); Icon::qd(halo, 2.4f, 6.2f, 2.4f, 3.4f, ic.k);
    ic.L.linePaths[h] = halo; ic.L.linePaths[hw] = halo;
    Path p; ic.circle(p, 8, 11.2f, 4.4f);
    ic.L.fillPaths[f] = p; ic.L.linePaths[o] = p;
    return ic.done();
}

// ---- layout (HUD coordinates: the game's 640x480; the left wall of the room)
const double kX = 3, kY = 140, kRow = 23, kTextX = 21, kTextY = -1, kTextW = 46, kDeltaX = 55, kDealGap = 7;
const float kSize = 14, kDeltaSize = 11;
const int kDeltaTicks = 60, kFadeTicks = 15;

struct Row {
    const char* name; uint16_t (*icon)(); bool percent = false;
    DisplayObject* sym = nullptr; EditTextObj* shadow = nullptr; EditTextObj* text = nullptr;
    EditTextObj* up = nullptr; EditTextObj* upShadow = nullptr; EditTextObj* down = nullptr; EditTextObj* downShadow = nullptr;
    double prev = NAN, delta = 0; uint32_t deltaUntil = 0; bool shown = true;
};
const int kRows = 8;
Row g_rows[kRows] = {{"speed", iconSpeed}, {"tears", iconTears}, {"damage", iconDamage}, {"range", iconRange}, {"shotspeed", iconShotSpeed},
                     {"luck", iconLuck}, {"devil", iconDevil, true}, {"angel", iconAngel, true}};
MovieClip* g_clip = nullptr; bool g_rooted = false;
uint16_t g_iconIds[kRows];
double g_damage = NAN; uint32_t g_damageTick = 0;
double g_range = NAN;   // the range shown (see statonTick)

Value V(const Value& o, const char* k) { return o.isObj() ? getMemK(o, intern(k)) : VUNDEF; }
double num(const Value& v, double def) { double d = toNum(v); return std::isfinite(d) ? d : def; }

// dmgdo() without side effects (see the top of the file)
double damageOf(const Value& root, const Value& a) {
    Value fn = V(a, "dmgdo");
    if (!fn.isObj()) return NAN;
    Str* kFirrb = intern("firrb"); Str* kChal = intern("chal");
    bool hadFirrb = hasProp(root.o, kFirrb), hadChal = hasProp(a.o, kChal);
    Value firrb = getMemK(root, kFirrb), chal = getMemK(a, kChal);
    uint32_t rs; uint64_t rc; rngSave(&rs, &rc);
    Act ac{vm.globalScope, VUNDEF, vm.root, nullptr};
    double d = toNum(callMethodK(ac, a, intern("dmgdo"), 0, nullptr));
    rngRestore(rs, rc);
    if (hadFirrb) setMemK(root, kFirrb, firrb); else root.o->props.erase(kFirrb);
    if (hadChal) setMemK(a, kChal, chal); else a.o->props.erase(kChal);
    return d;
}

void removeClip() {
    if (g_clip && !g_clip->removed && g_clip->parent) removeObject(g_clip);
    g_clip = nullptr;
    for (auto& r : g_rows) { r.sym = nullptr; r.shadow = r.text = r.up = r.upShadow = r.down = r.downShadow = nullptr; r.prev = NAN; r.deltaUntil = 0; }
    g_damage = NAN; g_range = NAN;
}

EditTextObj* text(MovieClip* c, int& depth, double x, double y, float w, float size, uint32_t argb, int align) {
    DisplayObject* d = synthPut(c, synthTextField(w, size, argb, align), depth++, x, y);
    return d && d->kind == OK_EDITTEXT ? (EditTextObj*)d : nullptr;
}
void build(MovieClip* hud, double alpha) {
    g_clip = createEmptyMovieClip(hud, intern("__staton"), 1048000);
    if (!g_clip) return;
    g_clip->cx.am = (float)alpha;
    static bool icons = false;
    if (!icons) { icons = true; for (int i = 0; i < kRows; i++) g_iconIds[i] = g_rows[i].icon(); }
    int depth = 1;
    for (int i = 0; i < kRows; i++) {
        Row& r = g_rows[i];
        double y = kY + i * kRow + (r.percent ? kDealGap : 0);
        r.sym = synthPut(g_clip, g_iconIds[i], depth++, kX, y);
        r.shadow = text(g_clip, depth, kX + kTextX + 1, y + kTextY + 1, kTextW, kSize, 0xc0000000u, 0);
        r.text = text(g_clip, depth, kX + kTextX, y + kTextY, kTextW, kSize, 0xfff4f0e8u, 0);
        r.upShadow = text(g_clip, depth, kX + kDeltaX + 1, y + kTextY + 2 + 1, kTextW, kDeltaSize, 0xc0000000u, 0);
        r.up = text(g_clip, depth, kX + kDeltaX, y + kTextY + 2, kTextW, kDeltaSize, 0xff7be07bu, 0);
        r.downShadow = text(g_clip, depth, kX + kDeltaX + 1, y + kTextY + 2 + 1, kTextW, kDeltaSize, 0xc0000000u, 0);
        r.down = text(g_clip, depth, kX + kDeltaX, y + kTextY + 2, kTextW, kDeltaSize, 0xffe86a5eu, 0);
        for (EditTextObj* t : {r.up, r.upShadow, r.down, r.downShadow}) if (t) t->visible = false;
    }
}
// The chance that the boss room opens the devil / angel room door, in [0, 1]: the formula the game uses when the boss
// room's door slots are set up after the boss dies (the `trg.sat` branch of the room script; the same in the three
// versions except the floors). Values the game rolls later are taken as they would be if nothing changes: the boss
// fight without damage counts until the boss room is entered (`bossdmg` is reset with each room).
//   f = min(0.3, Pentagrams * 0.2) + 0.01  + 0.25 The Book of Belial held (_root.it 2)  + 0.35 Book of Revelations
//       (_root.it 38)  + 0.35 _root.bumkill  + 0.35 no damage in the boss fight  + 0.99 no damage on this floor
//       (_root.devil true); halved if the last devil room was less than 3 floors ago, and again if less than 2
//   never on the first floor, nor from Sheol on (the original: only Sheol); once rolled, 1 or 0 (_root.devil 2 / 3)
// Wrath of the Lamb / Eternal: on a floor whose start found evs == 0 (no devil deal taken) and an earlier devil room,
// the door leads to the angel room instead, 50% (_root.altsat, rolled when the floor starts, shown only as 50/50
// until the door opens).
void dealChance(const Value& root, const Value& a, const std::string& game, double ups51, double* devil, double* angel) {
    *devil = *angel = 0;
    double chaps = num(V(root, "chaps"), 1);
    Value dv = V(root, "devil");
    double d = dv.t == T_NUM ? dv.n : NAN;
    bool angelOk = game != "og" && num(V(root, "evs"), 0) == 0 && num(V(root, "lastdev"), 0) > 0;
    if (d == 2) { (truthy(V(root, "altsat")) && game != "og" ? *angel : *devil) = 1; return; }
    if (d == 3 || chaps == 1 || (game == "og" ? chaps == 9 : chaps >= 9)) return;
    double f = std::min(0.3, ups51 * 0.2) + 0.01;
    double it = num(V(root, "it"), 0);
    if (it == 2) f += 0.25;
    if (it == 38) f += 0.35;
    if (truthy(V(root, "bumkill"))) f += 0.35;
    double lev = num(V(root, "lev"), -1);
    bool inBoss = lev == num(V(root, "bossl"), -2) || lev == num(V(root, "bossl2"), -2);
    if (!inBoss || truthy(V(a, "bossdmg"))) f += 0.35;
    if (truthy(dv)) f += 0.99;
    double last = toNum(V(root, "lastdev"));   // (undefined: NaN, no halving)
    if (last + 3 > chaps) f *= 0.5;
    if (last + 2 > chaps) f *= 0.5;
    f = std::max(0.0, std::min(1.0, f));
    if (angelOk) { *devil = f * 0.5; *angel = f * 0.5; } else *devil = f;
}
// the six stats (speed, tears, damage, range, shot speed, luck). Damage: the given value, or dmgdo() when NAN.
void computeStats(const Value& root, const Value& a, const std::string& game, double vals[6], double damage) {
    Value upsV = V(a, "ups");   // (the copy the game's own code reads: upa() refreshes it from _root.ups)
    auto ups = [&](int i) -> double {
        if (!upsV.isObj() || upsV.o->kind != OK_ARRAY) return 0;
        Array* arr = (Array*)upsV.o;
        return i < (int)arr->el.size() ? num(arr->el[i], 0) : 0;
    };
    double skiner = num(V(root, "skiner"), 0);
    if (!std::isfinite(damage)) {
        damage = damageOf(root, a);
        if (std::isfinite(damage) && game == "eternal" && (ups(4) || truthy(V(a, "doub")) || ups(7) || ups(12))) damage *= 1.15;
    }
    // shot speed: the multiplier the tears' speed gets when fired
    double shot = 1;
    if (game == "wotl") {
        if (ups(143) || ups(165) || ups(183) || ups(176) || ups(182) || ups(194) || skiner == 6) {
            shot = 1 + ups(143) * 0.2 + ups(165) * 0.23 + ups(176) * 0.16 + (ups(183) + ups(194)) * 0.16 - ups(182) * 0.25;
            if (skiner == 6) shot += 0.31;
            shot = std::min(shot, 1.4);
        }
    } else if (game == "eternal") {
        if (ups(143) || ups(165) || ups(110) || ups(16) || ups(108) || ups(159) || ups(153) || ups(169) || ups(183) || ups(176) || ups(182) || ups(194) || skiner == 6) {
            shot = 1 - (ups(110) + ups(108)) * 0.25 - 0.17 * (ups(153) + ups(16) + ups(159) + ups(169)) + ups(143) * 0.2 + ups(165) * 0.23 + ups(176) * 0.16
                 + (ups(183) + ups(194)) * 0.16 - ups(182) * 0.25;
            if (skiner == 6) shot += 0.31;
            shot = std::max(0.6, std::min(shot, 1.42));
        }
        if (ups(5)) shot *= 1.4;
    }
    Value luck = V(root, "luck");
    vals[0] = num(V(root, "playsp"), 1);
    vals[1] = 30.0 / (num(V(root, "firra"), 10) + 1);
    vals[2] = damage;
    vals[3] = num(V(root, "firrr"), 23) * 6.5 / 23.0;
    vals[4] = shot;
    vals[5] = luck.t == T_UNDEF ? NAN : num(luck, 0);
    // (range and shot speed fixed with GenesisConsole only reach the game's variables with the next tear)
    for (int i = 3; i <= 4; i++) { double o = genesisOverride(i); if (std::isfinite(o)) vals[i] = o; }
}
}  // namespace

void currentStats(double out[6], bool withDamage) {
    Value root((Object*)vm.root);
    Value a = V(root, "a");
    for (int i = 0; i < 6; i++) out[i] = NAN;
    if (!a.isObj()) return;
    computeStats(root, a, g_game ? g_game->tag : "wotl", out, withDamage ? NAN : 0.0);
    if (!withDamage) out[2] = NAN;
}

void statonTick(const ModInfo& m) {
    if (!g_rooted) { g_rooted = true; gcAddRoot((GcObj**)&g_clip); }
    Value root((Object*)vm.root);
    Value a = V(root, "a"), hud = V(root, "hud");
    bool inGame = false; recompInGameplay(&inGame);
    if (!inGame || !hud.isObj() || hud.o->kind != OK_CLIP || !a.isObj()) { if (g_clip) removeClip(); return; }
    if (!g_clip || g_clip->removed || g_clip->parent != (MovieClip*)hud.o) {
        removeClip();
        double op = atof(m.opt("opacity", "90").c_str());
        build((MovieClip*)hud.o, std::max(0.1, std::min(1.0, op / 100.0)));
        if (!g_clip) return;
    }
    const std::string game = g_game ? g_game->tag : "wotl";
    // damage: every 6 ticks (dmgdo runs a little of the game's script)
    bool dmgNow = !std::isfinite(g_damage) || g_player.tick - g_damageTick >= 6 || g_player.tick < g_damageTick;
    double base[6];
    computeStats(root, a, game, base, dmgNow ? NAN : g_damage);
    if (dmgNow) { if (std::isfinite(base[2])) g_damage = base[2]; g_damageTick = g_player.tick; }
    Value luck = V(root, "luck");
    Value upsV = V(a, "ups");
    double ups51 = upsV.isObj() && upsV.o->kind == OK_ARRAY && ((Array*)upsV.o)->el.size() > 51 ? num(((Array*)upsV.o)->el[51], 0) : 0;
    double devil = 0, angel = 0;
    dealChance(root, a, game, ups51, &devil, &angel);
    // Range is read from the last tear the game fired, and every tear's fall has a small random part (dm = rand() *
    // 0.2: up to 0.28 of range less): changes smaller than that are that noise, not a change of the stat.
    if (!std::isfinite(g_range) || std::fabs(base[3] - g_range) >= 0.3) g_range = base[3];
    double vals[kRows] = {base[0], base[1], std::isfinite(g_damage) ? g_damage : 3.5, g_range, base[4], num(luck, 0), devil * 100, angel * 100};
    bool showChanges = m.opt("show_changes", "1") != "0";
    bool showDeals = m.opt("show_deals", "1") != "0";
    for (int i = 0; i < kRows; i++) {
        Row& r = g_rows[i];
        bool show = i == 5 ? luck.t != T_UNDEF    // (the original game has no luck)
                  : i == 6 ? showDeals : i == 7 ? showDeals && game != "og" : true;   // (nor angel rooms)
        if (show != r.shown) {
            r.shown = show;
            for (DisplayObject* d : {(DisplayObject*)r.sym, (DisplayObject*)r.shadow, (DisplayObject*)r.text}) if (d) d->visible = show;
        }
        double v = r.percent ? std::round(vals[i] * 10) / 10 : std::round(vals[i] * 100) / 100;
        char buf[32];
        if (r.percent) snprintf(buf, sizeof buf, v < 10 && v != 0 ? "%.1f%%" : "%.0f%%", v);
        else snprintf(buf, sizeof buf, "%.2f", v);
        if (r.text) r.text->setText(buf);
        if (r.shadow) r.shadow->setText(buf);
        const double eps = r.percent ? 0.05 : 0.005;
        if (std::isfinite(r.prev) && std::fabs(v - r.prev) >= eps && showChanges) {
            r.delta = (g_player.tick < r.deltaUntil ? r.delta : 0) + (v - r.prev);
            r.deltaUntil = g_player.tick + kDeltaTicks;
        }
        r.prev = v;
        bool active = g_player.tick < r.deltaUntil && std::fabs(r.delta) >= eps && show;
        bool upward = r.delta > 0;
        if (active) {
            if (r.percent) snprintf(buf, sizeof buf, std::fabs(r.delta) < 10 ? "%s%.1f%%" : "%s%.0f%%", upward ? "+" : "-", std::fabs(r.delta));
            else snprintf(buf, sizeof buf, "%s%.2f", upward ? "+" : "-", std::fabs(r.delta));
            EditTextObj* t = upward ? r.up : r.down; EditTextObj* sh = upward ? r.upShadow : r.downShadow;
            if (t) t->setText(buf);
            if (sh) sh->setText(buf);
            uint32_t left = r.deltaUntil - g_player.tick;
            float al = left < (uint32_t)kFadeTicks ? (float)left / kFadeTicks : 1.f;
            for (EditTextObj* x : {t, sh}) if (x) x->cx.am = al;
        }
        for (EditTextObj* x : {r.up, r.upShadow}) if (x) x->visible = active && upward;
        for (EditTextObj* x : {r.down, r.downShadow}) if (x) x->visible = active && !upward;
    }
}
}  // namespace fl
