// Itemnomicon (mods/Itemnomicon.mod): walk up to an item, a trinket or a card you can pick up or buy, and a card
// above it says what it is: its name (in the colour of the item's quality), what kind of thing it is (an active item
// shows its charges), the stats it changes (green up, red down) and a short description of what it does.
//
// The stat changes were measured in the game itself (each item given alone to Isaac at the start of a run, in each of
// the three versions; the tears from the game's own tear formula, which matches those measurements), so they are the
// game's numbers and not a wiki's: mod_itemnomicon_data.inc. They are what the item adds to a character with
// nothing else: some items stack differently with others.
//
// Read-only: it looks at the room's pickups (_root.a.ball entries with s == 5) and draws inside _root.hud, like
// StatOn, so runs play exactly the same with it on.
#include "mods.h"
#include "mod_ui.h"
#include "core.h"
#include "display.h"
#include <cmath>
#include <cstdio>
#include <string>

namespace fl {
bool recompInGameplay(bool* inGame);

namespace {
#include "mod_itemnomicon_data.inc"

Value V(const Value& o, const char* k) { return o.isObj() ? getMemK(o, intern(k)) : VUNDEF; }
double num(const Value& v, double def = 0) { double d = toNum(v); return std::isfinite(d) ? d : def; }
const double kRoomTop = 77.5;

enum Kind { K_NONE, K_ITEM, K_TRINKET, K_CARD };
struct Target { Kind kind = K_NONE; int id = 0; bool shop = false; double x = 0, y = 0; };

// what a pickup is (nothing if it isn't something Itemnomicon describes)
Target classify(const Value& b) {
    Target t;
    if (!b.isObj() || num(V(b, "s")) != 5 || truthy(V(b, "empty")) || truthy(V(b, "dones"))) return t;   // (dones: being picked up)
    Value d = V(b, "d");
    if (!d.isObj() || d.o->kind != OK_CLIP) return t;
    MovieClip* dc = (MovieClip*)d.o;
    if (dc->removed || !dc->visible) return t;
    int fr = dc->cur;
    int it = (int)num(V(b, "it")), col = (int)num(V(b, "col"));
    const std::string game = g_game ? g_game->tag : "wotl";
    int nItems = game == "og" ? 132 : 198;
    t.x = num(V(b, "xp")); t.y = num(V(b, "yp"));
    if ((fr == 10 || fr == 15) && it >= 1 && it <= nItems) { t.kind = K_ITEM; t.id = it; t.shop = fr == 15; return t; }
    if (fr == 7 || fr == 15) {
        t.shop = fr == 15;
        int nTr = game == "og" ? 0 : game == "eternal" ? 34 : 33;
        if (col >= 29 && col < 29 + nTr) { t.kind = K_TRINKET; t.id = col - 28; return t; }
        if ((col >= 7 && col <= 28) || (game != "og" && col >= 70 && col <= 74)) { t.kind = K_CARD; t.id = col; return t; }
    }
    t.kind = K_NONE;
    return t;
}

// Item quality (0-4), as Rebirth / Repentance rate the same items (the item numbers are the same there); 43 and 61
// are not real items in these games
const uint8_t kQuality[199] = {0,3,2,3,4,1,1,3,1,0,2,2,4,2,2,1,1,2,2,1,2,1,0,0,0,0,0,1,1,1,1,1,2,1,2,1,0,1,1,0,0,0,1,0,0,1,1,1,2,2,3,3,3,1,1,1,0,1,3,1,1,0,2,2,2,0,1,1,3,3,3,2,2,2,2,2,1,1,3,3,3,3,2,1,2,2,0,2,1,1,2,1,2,1,1,1,1,1,2,1,2,3,1,1,2,4,2,1,3,1,1,0,2,2,4,2,2,1,4,2,1,2,2,1,1,2,0,3,1,1,2,1,2,3,2,2,0,0,2,1,1,1,2,2,1,2,3,1,1,3,2,1,2,3,2,2,2,2,3,3,1,3,2,1,1,2,2,2,4,4,2,1,3,2,1,2,2,0,2,3,1,3,4,2,3,3,0,2,1,3,2,2,1,2,1,1,2,2,1};
// the name's colour: quality 0 grey, 1 white, 2 blue, 3 violet, 4 gold; trinkets pink, cards teal
uint32_t qualityColor(int q) { static const uint32_t c[5] = {0xffb4aea6u, 0xfff6f2eau, 0xff6fb4ffu, 0xffc890ffu, 0xffffc94au}; return c[std::max(0, std::min(4, q))]; }

struct Line { std::string text; uint32_t argb; float size; };
const uint32_t kUp = 0xff7be07bu, kDown = 0xffe86a5eu, kBody = 0xffd9d0c1u, kDim = 0xff9a9184u;
// accent per kind: items gold, active items blue, trinkets violet, cards teal
uint32_t accent(Kind k, bool active) { return k == K_TRINKET ? 0xffb68ee0u : k == K_CARD ? 0xff6ccfc0u : active ? 0xff7fb2f0u : 0xfff0c674u; }

MovieClip* g_clip = nullptr; MovieClip* g_card = nullptr; DisplayObject* g_ptr = nullptr;
std::string g_key; float g_fade = 0; double g_w = 0, g_h = 0;
Target g_shown; double g_playerX = 0, g_playerY = 0;

void removeAll() {
    if (g_clip && !g_clip->removed && g_clip->parent) removeObject(g_clip);
    g_clip = g_card = nullptr; g_ptr = nullptr; g_key.clear(); g_fade = 0;
}

// builds the card for t in g_card (at the origin; moved later)
void build(const Target& t) {
    modui::clear(g_card);
    const std::string game = g_game ? g_game->tag : "wotl";
    std::string name, tag, desc, stats; bool active = false;
    if (t.kind == K_ITEM) {
        const ItmItem& it = kItmItems[t.id];
        name = it.name; desc = it.desc;
        const char* s = game == "og" ? it.og : game == "eternal" ? it.eternal : it.wotl;
        stats = s ? s : "";
        active = it.charges >= 0;
        if (!active) tag = "PASSIVE ITEM";
        else if (it.charges > 0) { char b[48]; snprintf(b, sizeof b, "ACTIVE ITEM - %d CHARGE%s", it.charges, it.charges == 1 ? "" : "S"); tag = b; }
        else tag = "ACTIVE ITEM";
    } else if (t.kind == K_TRINKET) {
        const ItmTrinket& tr = kItmTrinkets[t.id - 1];
        name = tr.name; desc = tr.desc; stats = game == "eternal" ? tr.statsEternal : tr.stats; tag = "TRINKET";
    } else {
        for (const ItmCard& c : kItmCards) if (c.col == t.id) { name = c.name; desc = c.desc; }
        tag = t.id >= 70 ? "PLAYING CARD" : "TAROT CARD";
    }
    // the name: items in the colour of their quality, trinkets and cards in their accent colour
    uint32_t nameColor = t.kind == K_ITEM ? qualityColor(kQuality[t.id]) : accent(t.kind, false);
    if (t.kind == K_ITEM && t.id != 43 && t.id != 61) tag += "  -  QUALITY " + std::to_string(kQuality[t.id]);
    if (t.shop) tag += "  -  FOR SALE";
    // lines
    const float kTitle = 12.5f, kTag = 7.5f, kStat = 10, kDesc = 9;
    std::vector<Line> statLines;
    size_t p = 0;
    while (p < stats.size()) {
        size_t e = stats.find('|', p); if (e == std::string::npos) e = stats.size();
        std::string s = stats.substr(p, e - p);
        if (s.size() > 1) statLines.push_back({s.substr(1), s[0] == '+' ? kUp : kDown, kStat});
        p = e + 1;
    }
    // width: the title and the stat lines on one line each, between 150 and 230
    const double pad = 7;
    double w = modui::textW(name, kTitle) + 2 * pad + 4;
    for (auto& l : statLines) w = std::max(w, (double)modui::textW(l.text, kStat) + 2 * pad + 12);
    w = std::max(150.0, std::min(230.0, std::ceil(w)));
    std::vector<std::string> descLines = modui::wrap(desc, (float)(w - 2 * pad), kDesc);
    double h = pad + 15 + 11 + 5 + (statLines.empty() ? 0 : statLines.size() * 12.5 + 4) + descLines.size() * 11.5 + pad - 2;
    int dp = 1;
    uint32_t acc = accent(t.kind, active);
    g_ptr = modui::rect(g_card, dp, 0, 0, 100, 100, 0xe6120e0cu);      // the notch pointing at the thing (placed every tick)
    modui::rect(g_card, dp, 0, 0, w, h, 0xe6120e0cu);                 // the card
    modui::rect(g_card, dp, 0, 0, w, 2, acc);                         // its accent edge
    modui::rect(g_card, dp, 0, h - 1, w, 1, 0x40ffffffu);
    double y = pad - 2;
    modui::text(g_card, dp, pad, y, (float)(w - 2 * pad), kTitle, nameColor, 0, name); y += 15;
    modui::text(g_card, dp, pad, y, (float)(w - 2 * pad), kTag, kDim, 0, tag, false); y += 11;
    modui::rect(g_card, dp, pad, y, w - 2 * pad, 1, 0x30ffffffu); y += 5;
    for (auto& l : statLines) {
        // a small arrow-like mark, then the text
        modui::rect(g_card, dp, pad, y + 4.5, 5, 5, l.argb);
        modui::text(g_card, dp, pad + 10, y, (float)(w - 2 * pad - 10), l.size, l.argb, 0, l.text);
        y += 12.5;
    }
    if (!statLines.empty()) y += 4;
    for (auto& d : descLines) { modui::text(g_card, dp, pad, y, (float)(w - 2 * pad), kDesc, kBody, 0, d); y += 11.5; }
    g_w = w; g_h = h;
}
}  // namespace

float itemnomiconShown() { return g_clip && g_clip->visible ? g_fade : 0.f; }

void itemnomiconTick(const ModInfo& m) {
    static bool rooted = false;
    if (!rooted) { rooted = true; gcAddRoot((GcObj**)&g_clip); gcAddRoot((GcObj**)&g_card); gcAddRoot((GcObj**)&g_ptr); }
    Value root((Object*)vm.root);
    Value a = V(root, "a"), hud = V(root, "hud");
    bool inGame = false, play = recompInGameplay(&inGame);
    if (!inGame || !hud.isObj() || hud.o->kind != OK_CLIP || !a.isObj()) { if (g_clip) removeAll(); return; }
    if (!g_clip || g_clip->removed || g_clip->parent != (MovieClip*)hud.o) {
        removeAll();
        g_clip = createEmptyMovieClip((MovieClip*)hud.o, intern("__itemnomicon"), 1047950);
        if (!g_clip) return;
        g_card = createEmptyMovieClip(g_clip, intern("card"), 1);
        if (!g_card) { removeAll(); return; }
    }
    // the nearest thing within reach of the player
    Target best;
    Value player = V(a, "player"), balls = V(a, "ball");
    // range: in tiles (a tile is ~45 units of the 640-wide room), 0.5 to 4
    double range = std::max(0.5, std::min(4.0, atof(m.opt("range", "1").c_str()))) * 45 + 12;
    if (play && player.isObj() && balls.isObj() && balls.o->kind == OK_ARRAY && ((DisplayObject*)hud.o)->visible) {
        double px = num(V(player, "xp")), py = num(V(player, "yp")), bd = range * range;
        g_playerX = px; g_playerY = py;
        for (const Value& b : ((Array*)balls.o)->el) {
            Target t = classify(b);
            if (t.kind == K_NONE) continue;
            double dx = t.x - px, dy = t.y - py, d2 = dx * dx + dy * dy;
            if (d2 < bd) { bd = d2; best = t; }
        }
    }
    char key[48]; snprintf(key, sizeof key, "%d:%d:%d", (int)best.kind, best.id, (int)best.shop);
    if (best.kind != K_NONE && g_key != key) {
        if (g_fade > 0.05f && !g_key.empty()) g_fade = 0;   // (another one: start over)
        g_key = key; build(best);
    }
    if (best.kind != K_NONE) { g_shown = best; g_fade = std::min(1.f, g_fade + 0.25f); }
    else g_fade = std::max(0.f, g_fade - 0.2f);
    g_clip->visible = g_fade > 0 && !g_key.empty();
    if (!g_clip->visible) return;
    double op = std::max(0.3, std::min(1.0, atof(m.opt("opacity", "100").c_str()) / 100));
    g_clip->cx.am = (float)(g_fade * op);
    // Over the thing (items float above their pedestal); under it if you stand above it, so it doesn't cover you;
    // beside it when neither fits. The notch points at the thing.
    const double ix = g_shown.x, iy = g_shown.y;
    double above = iy - (g_shown.kind == K_ITEM ? 58 : 28) - g_h, under = iy + 24;
    bool playerAbove = g_playerY < iy - 12;
    int side = 0;   // 0 above, 1 under, 2 beside
    if (playerAbove) side = under + g_h <= 478 ? 1 : 2;
    else if (above < kRoomTop + 4) side = under + g_h <= 478 ? 1 : 2;
    double x, top;
    if (side == 2) {
        bool left = g_playerX >= ix ? ix - 34 - g_w >= 6 : ix + 34 + g_w > 634;   // (the side away from you, if it fits)
        x = left ? ix - 34 - g_w : ix + 34;
        top = iy - 40 - g_h / 2;
    } else {
        x = ix - g_w / 2;
        top = side == 1 ? under : above;
    }
    x = std::max(6.0, std::min(634 - g_w, x));
    top = std::max(kRoomTop + 2, std::min(top, 478 - g_h));
    bool below = side == 1;
    double rise = (1 - g_fade) * 6;   // (it rises a little as it appears)
    g_card->m = Mat(); g_card->m.tx = std::round(x); g_card->m.ty = std::round(top + rise); g_card->scriptXform = true;
    if (g_ptr) g_ptr->visible = side != 2;
    if (g_ptr && side != 2) {   // a small diamond half out of the card's edge, toward the thing (a 100x100 square turned 45 degrees)
        const double k = 9 / 100.0 / std::sqrt(2.0), px = std::max(10.0, std::min(g_w - 10, g_shown.x - std::round(x)));
        Mat r; r.a = k; r.b = k; r.c = -k; r.d = k;
        r.tx = px; r.ty = (below ? 0 : g_h) - 100 * k;   // (the square's corner (0, 0) is the diamond's top; its middle on the edge)
        g_ptr->m = r; g_ptr->scriptXform = true;
    }
}
}  // namespace fl
