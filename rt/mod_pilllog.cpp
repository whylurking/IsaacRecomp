// PillLog (mods/PillLog.mod): the pills you have already taken this run are known by sight.
//
// The game keeps what each pill colour does once one has been taken (_root.piller[colour], decided the first time)
// and already writes it in the HUD next to a pill you hold. PillLog uses the same knowledge everywhere else:
//   - over every pill lying in the room (or for sale), its effect, green when it is a good one and red when not;
//   - when you pick one up, the banner says the effect instead of "Pills here!";
//   - on the pause screen, a list of the six pill colours of this run with what each one does ("???" until taken).
// The names are the ones the game itself shows (its HUD code, per version), including the variants that depend on
// the moment: Health Up / Down, Bad Trip / Full Health, and the good versions with PHD or the Lucky Foot.
//
// It only reads the game, except the banner's text (_root.hud.st1.tex, which the game only writes), so runs play
// exactly the same with it on. Drawn inside _root.hud, like StatOn.
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
Value V(const Value& o, const char* k) { return o.isObj() ? getMemK(o, intern(k)) : VUNDEF; }
double num(const Value& v, double def = 0) { double d = toNum(v); return std::isfinite(d) ? d : def; }
Value arrAt(const Value& a, int i) {
    if (!a.isObj() || a.o->kind != OK_ARRAY) return VUNDEF;
    Array* arr = (Array*)a.o;
    return i >= 0 && i < (int)arr->el.size() ? arr->el[i] : VUNDEF;
}
const uint32_t kGood = 0xff7be07bu, kBad = 0xffe86a5eu, kNeutral = 0xfff0e6d2u, kDim = 0xff9a9184u;

// the effect of pill colour `col` as the game's HUD names it (empty if not known yet), and whether it's good / bad
struct Effect { std::string name; int mood = 0; };   // mood: 1 good, -1 bad, 0 neither
Effect effectOf(int col) {
    Effect e;
    Value root((Object*)vm.root), a = V(root, "a");
    Value pv = arrAt(V(root, "piller"), col);
    double raw = toNum(pv);
    if (!truthy(pv) || !std::isfinite(raw)) return e;
    const std::string game = g_game ? g_game->tag : "wotl";
    Value ups = V(a, "ups"), player = V(a, "player");
    auto up = [&](int i) { return truthy(arrAt(ups, i)); };
    int code = (int)std::round(raw);
    bool good = code != raw || up(46) || up(75);   // (the "positive" variant: .2, or forced by Lucky Foot / PHD)
    double hp = num(V(player, "hp")), armor = num(V(root, "armor"));
    auto ud = [&](const char* what) { e.name = std::string(what) + (good ? " Up" : " Down"); e.mood = good ? 1 : -1; };
    switch (code) {
        case 14: if (game != "og") { ud("Luck"); break; } break;
        case 13:
            if (game == "og") { e.name = "TelePills"; e.mood = 0; }
            else { e.name = "Friends till the end!"; e.mood = 1; }
            break;
        case 6: if (hp + armor > 1) { e.name = "Bad Trip!"; e.mood = -1; break; } /* fall through */
        case 1: e.name = game == "eternal" && truthy(V(root, "hardmode")) ? "Full Health?" : "Full Health"; e.mood = 1; break;
        case 2: e.name = "I found Pills"; break;
        case 3: e.name = "Explosive Diarrhea!"; break;
        case 4: e.name = "Balls of Steel"; e.mood = 1; break;
        case 5: e.name = "Pretty Fly"; e.mood = 1; break;
        case 7: {
            double m = num(V(root, "skiner")) != 4 ? num(V(player, "mhp")) : armor;
            bool upH = m <= 1 || good;
            e.name = upH ? "Health Up" : "Health Down"; e.mood = upH ? 1 : -1;
            break;
        }
        case 8: e.name = "Bombs are Key"; break;
        case 9: ud("Tears"); break;
        case 10: {
            bool upR = good || (up(6) && num(V(root, "chaps")) < 8);
            e.name = upR ? "Range Up" : "Range Down"; e.mood = upR ? 1 : -1;
            break;
        }
        case 11: ud("Speed"); break;
        case 12: e.name = "Bad Gas"; break;
        case 17: if (game == "eternal") ud("Shot Speed"); break;
        case 126: e.name = "TelePills"; break;
        default: break;
    }
    return e;
}
uint32_t moodColor(int m) { return m > 0 ? kGood : m < 0 ? kBad : kNeutral; }
int pillFrame(int col) {   // the pill's look (frame of the pill sprite), as the game's pic()
    const std::string game = g_game ? g_game->tag : "wotl";
    if (game == "og") return col;
    Value p = arrAt(V(Value((Object*)vm.root), "picers"), col - 1);
    return (int)num(p, col);
}

MovieClip* g_clip = nullptr; MovieClip* g_labels = nullptr; MovieClip* g_list = nullptr;
std::string g_labelKey, g_listKey;
std::vector<MovieClip*> g_labelClips;
void removeAll() {
    if (g_clip && !g_clip->removed && g_clip->parent) removeObject(g_clip);
    g_clip = g_labels = g_list = nullptr; g_labelKey.clear(); g_listKey.clear(); g_labelClips.clear();
}
MovieClip* childClip(DisplayObject* d, const char* name) {
    if (!d || d->kind != OK_CLIP) return nullptr;
    DisplayObject* k = ((MovieClip*)d)->childByName(intern(name));
    return k && k->kind == OK_CLIP ? (MovieClip*)k : nullptr;
}
// a copy of the HUD's pill / card holder showing the pill of colour col
void pillIcon(MovieClip* parent, int& depth, double x, double y, double scale, int col) {
    Value extra = V(V(Value((Object*)vm.root), "hud"), "extra");
    if (!extra.isObj() || !extra.o->isDisplay()) return;
    DisplayObject* src = (DisplayObject*)extra.o;
    DisplayObject* c = synthPut(parent, src->charId, depth++, x, y);
    if (!c) return;
    c->m.a = c->m.d = scale; c->scriptXform = true;
    MovieClip* d = childClip(c, "d");
    if (!d) return;
    gotoFrame(d, 11, true);
    if (MovieClip* p = childClip(d, "p")) gotoFrame(p, pillFrame(col), true);
}
}  // namespace

void pilllogTick(const ModInfo& m) {
    static bool rooted = false;
    if (!rooted) { rooted = true; gcAddRoot((GcObj**)&g_clip); gcAddRoot((GcObj**)&g_labels); gcAddRoot((GcObj**)&g_list); }
    Value root((Object*)vm.root);
    Value a = V(root, "a"), hudV = V(root, "hud");
    bool inGame = false, play = recompInGameplay(&inGame);
    if (!inGame || !hudV.isObj() || hudV.o->kind != OK_CLIP || !a.isObj()) { if (g_clip) removeAll(); return; }
    MovieClip* hud = (MovieClip*)hudV.o;
    if (!g_clip || g_clip->removed || g_clip->parent != hud) {
        removeAll();
        g_clip = createEmptyMovieClip(hud, intern("__pilllog"), 1047940);
        if (!g_clip) return;
        g_labels = createEmptyMovieClip(g_clip, intern("labels"), 1);
        g_list = createEmptyMovieClip(g_clip, intern("list"), 2);
        if (!g_labels || !g_list) { removeAll(); return; }
    }
    // the banner of a pill just picked up
    if (m.opt("banner", "1") != "0") {
        Value st1 = V(hudV, "st1");
        Value tex = V(st1, "tex");
        int pilc = (int)num(V(root, "pilc"), 0);
        if (tex.t == T_STR && tex.s->s == "Pills here!" && pilc >= 1 && pilc <= 6) {
            Effect e = effectOf(pilc);
            if (!e.name.empty()) setMemK(st1, intern("tex"), Value(mkstr(e.name)));
        }
    }
    // labels over the pills in the room
    struct Seen { int col; double x, y; Effect e; };
    std::vector<Seen> seen;
    Value balls = V(a, "ball");
    if (play && hud->visible && m.opt("labels", "1") != "0" && balls.isObj() && balls.o->kind == OK_ARRAY) {
        for (const Value& b : ((Array*)balls.o)->el) {
            if (!b.isObj() || num(V(b, "s")) != 5 || truthy(V(b, "empty")) || truthy(V(b, "dones"))) continue;   // (dones: being picked up)
            Value d = V(b, "d");
            if (!d.isObj() || d.o->kind != OK_CLIP || !((DisplayObject*)d.o)->visible) continue;
            int fr = ((MovieClip*)d.o)->cur, col = (int)num(V(b, "col"));
            if ((fr != 7 && fr != 15) || col < 1 || col > 6) continue;
            Effect e = effectOf(col);
            if (e.name.empty()) continue;
            seen.push_back({col, num(V(b, "xp")), num(V(b, "yp")), e});
            if (seen.size() >= 12) break;
        }
    }
    std::string key;
    for (auto& s : seen) key += std::to_string(s.col) + s.e.name + ";";
    if (key != g_labelKey) {
        g_labelKey = key;
        modui::clear(g_labels); g_labelClips.clear();
        int n = 1;
        for (auto& s : seen) {
            MovieClip* c = createEmptyMovieClip(g_labels, intern(("l" + std::to_string(n)).c_str()), n);
            n++;
            if (!c) continue;
            int dp = 1; const float size = 10.5f;
            double w = std::ceil(modui::textW(s.e.name, size)) + 10;
            modui::rect(c, dp, -w / 2, 0, w, 16, 0xc8120e0cu);
            modui::rect(c, dp, -w / 2, 15, w, 1, (moodColor(s.e.mood) & 0x00ffffffu) | 0xa0000000u);
            modui::text(c, dp, -w / 2, 1, (float)w, size, moodColor(s.e.mood), 2, s.e.name);
            g_labelClips.push_back(c);
        }
    }
    for (size_t i = 0; i < g_labelClips.size() && i < seen.size(); i++) {
        MovieClip* c = g_labelClips[i];
        c->m = Mat(); c->m.tx = std::round(std::max(40.0, std::min(600.0, seen[i].x))); c->m.ty = std::round(seen[i].y - 34); c->scriptXform = true;
    }
    g_labels->visible = !seen.empty();
    // the pause screen's list
    bool paused = false;
    if (inGame && !play && m.opt("pause_list", "1") != "0") {
        Value paus = V(hudV, "paus");
        if (paus.isObj() && paus.o->kind == OK_CLIP) { MovieClip* pc = (MovieClip*)paus.o; paused = pc->visible && pc->cur == 2 && hud->visible; }
    }
    if (paused) {
        std::string lk;
        Effect eff[7];
        for (int c = 1; c <= 6; c++) { eff[c] = effectOf(c); lk += eff[c].name + ";" + std::to_string(pillFrame(c)) + ";"; }
        if (lk != g_listKey) {
            g_listKey = lk;
            modui::clear(g_list);
            int dp = 1;
            const double x0 = 507, y0 = 168, w = 126, rowH = 22;
            double h = 24 + 6 * rowH + 6;
            modui::rect(g_list, dp, x0, y0, w, h, 0xd8120e0cu);
            modui::rect(g_list, dp, x0, y0, w, 2, 0xfff0c674u);
            modui::text(g_list, dp, x0 + 8, y0 + 4, (float)w - 16, 11, 0xfff0c674u, 0, "PILLS");
            int known = 0; for (int c = 1; c <= 6; c++) known += !eff[c].name.empty();
            modui::text(g_list, dp, x0 + 8, y0 + 7, (float)w - 16, 7.5f, kDim, 1, std::to_string(known) + " / 6 KNOWN", false);
            for (int c = 1; c <= 6; c++) {
                double y = y0 + 24 + (c - 1) * rowH;
                pillIcon(g_list, dp, x0 + 13, y + 9, 0.55, c);
                const Effect& e = eff[c];
                std::string t = e.name.empty() ? "???" : e.name;
                float size = 8.5f;
                while (size > 6.5f && modui::textW(t, size) > w - 34) size -= 0.5f;
                modui::text(g_list, dp, x0 + 27, y + 3, (float)w - 32, size, e.name.empty() ? kDim : moodColor(e.mood), 0, t);
            }
        }
    }
    g_list->visible = paused;
}
}  // namespace fl
