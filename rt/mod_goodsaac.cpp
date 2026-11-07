// Good Saac (mods/GoodSaac.mod): fixes for bugs of the original games (see NOTES.md "v1.6: Good Saac").
//
// Each fix is an option (all on by default) and only touches what the bug touches; most of them are what Eternal
// Edition itself fixed later, done the Eternal way for the original and Wrath of the Lamb:
//   subtitles      (og, wotl) the pickup banner's line under some items' names says the wrong thing (Sacred Heart
//                  "Tears Up", MEAT! "Tears + HP Up", Magic 8 Ball and Tooth Picks "Tears Up"...): Eternal's lines,
//                  which match what the items do in all three games (their code is the same: powerlevel / ups[]);
//   names          (og, wotl) mini-boss banners say "Magdalena vs ..." and "XXX vs ...": "Magdalene", "???" (Eternal);
//   eternal_heart  (og, wotl) an Eternal Heart doesn't save a character with no heart containers (???, or anyone
//                  after deals took them all): playerhurt's death test ignores it. Eternal's test is
//                  `hp <= 0 && armor <= 0 && !eta || !(mhp > 0 || armor > 0 || eta)`: the wrapped playerhurt undoes
//                  the death the original test gave when Eternal's would not have (the hit's own animation instead);
//   mom_voice      (og, wotl) Mom's Heart / It Lives! in its last phase (momstate 3) says "Isaac!" (and "heartout")
//                  every tick for a second: the original enters its "come out" branch on every image of the out
//                  animation. Eternal enters it only from the hidden frame; here the sounds are played at most once a
//                  second (the game's logic is untouched);
//   challenge_music the challenge room's music (m6 / m27) went on after leaving the room by teleport or an extra
//                  life, until its loop point: _root.amusic is cleared when a room starts, but only a miniboss room
//                  (_root.amas) fades the music then. Here the music fades as for a miniboss room;
// Good Saac 1.1 (checked one by one against the three games' code, NOTES.md "Good Saac: the bug list"):
//   heart_name     (all) "<3" has no name in the collection: its text field (ittz) is HTML and "<3" reads as the
//                  start of a tag. Shown as "&lt;3" there (st1 itself is untouched: other code uses it as plain text);
//   last_will      (all) the death diary's "killed by" picture: frame 202 (Razor Blade, the only source 202) is empty:
//                  the Razor Blade's grey drawing (the diary's own item sprite) is put there; any other empty
//                  picture ("killed by nothing": a shot whose shooter the game didn't record...) shows the nearest
//                  enemy that has one;
//   shop_pills     (wotl, eternal) a pill for sale shows its colour number (col) as a picture, while on the floor, in
//                  the HUD and when used the game shows pic(col) (the run's colours, _root.picers): bought, it
//                  changed colour. Shown as pic(col), as everywhere else;
//   challenge_pause (all) the challenge room's waves (chama, timed by chamb) kept coming while paused, with nobody
//                  to fight them: all at once. The wave timer is held while paused;
//   mom_red        (all) Mom's death turns the screen red for a moment (colorit(_root) while momdown counts): a
//                  teleport in that moment left the new room red until the game was closed. Reset on a new room;
//   gamekid_head   (all) The Gamekid flips the player's head (pacman); the code that puts it back (pacoo) never runs,
//                  so it stayed flipped (Technology's laser out of the wrong side) until the next room;
//   daddy_stomps   (wotl, eternal) Daddy Longlegs stomps when it reaches its target from more than 20 px away
//                  (whaf); next to a boss that doesn't move (Gurdy, Isaac) it never leaves that distance: one stomp.
//                  It stomps again there when its wait (fire) is over, as it does on the way in;
//   foot_shadow    (all) killing Mom (or Satan) while her foot was coming down still hurt: the foot's hit lands
//                  after momkill(). Hits from Mom's parts (45) and Satan (84) while momdown counts are ignored;
//   unlocks        (horseman: all) beating a Headless Horseman (bosss 22) unlocked C.H.A.D.'s secret (locker 32,
//                  Little Chad) as well; (wotl, eternal) The Chest with ??? and with Eve unlocked each other's trinket
//                  (bosssp's f10: 97 Eve's Bird Foot for ???, 94 ???'s Soul for Eve). The right ones are unlocked
//                  instead. (The challenges' rewards, _root.lox, are right: 7 Years Bad Luck 78 Lucky Toe, Large
//                  Marge 77 Burnt Penny; only the Steam achievement names, acnam, are mixed up there);
//   xl_depths      (wotl, eternal) in an XL Depths, the first boss (bossl2) took Mom's branch of bosssp while Mom had
//                  never been beaten (or in challenges 1-4): no item, and Large Marge won there. It is a normal boss;
//   mom_deals      (all) after Mom, the devil / angel room door never showed: her parts hide the door holes they come
//                  out of (door._visible = false), the deal's door too. Shown once she is dead (momdown);
//   breakfast      (all) with every item already owned, giveit() never finds one and the game freezes: Breakfast
//                  instead (Rebirth's "breakfasting");
//   forget_me_now  (wotl, eternal) Forget Me Now in the Cathedral made a Sheol: the floor is made again, and the
//                  Cathedral comes from _root.heaven == 2, which every room of that floor clears. Kept for it;
//   secrets_keys   (og, wotl) the secrets screen moved two secrets per arrow key: its buttons react to the key
//                  (keyPress) and its onEnterFrame too (Eternal took the key off the buttons). One step;
//   start_spot     (all) a new run started where the player last came into a room in the run before (_root.playerx
//                  / playery: set as he goes through a door, never cleared; the first run finds them undefined and
//                  puts him at his usual start). Cleared whenever the main menu shows (Continuum sets them after that
//                  for the run it brings back);
//   familiar_shots (all) a familiar's shots flew off at a mad speed, the wrong way, or not at all once
//                  Robo-Baby was there: the familiars shoot in plff, one after another, along _root.a.xenf / yenf
//                  (the player's aim), and Robo-Baby's laser (lasershow) leaves there the way to its target, not a
//                  direction: the ones after it (Harlequin Baby's two shots, Sister Maggy's...) went that way, as
//                  fast as the target was far. Inside plff, lasershow puts xenf / yenf back as it found them;
//   familiars      (all) no limit to the babies that follow him, as in Rebirth: the game makes Math.min(3, the
//                  baby items he has) of them (2 in the original), and friends gives each its kind (`baa`) from
//                  the items, skipping the last two kinds given - so with more items some never showed. The missing
//                  babies are made as the game makes them (create(..., 3), `minions` + 1) and their kinds set so
//                  that each baby item (each copy of it) has one;
//   select_click   (wotl) changing character with the keys made no sound (the arrows' buttons play it on a click).
//                  The original and Eternal play it themselves (in Eternal this made it sound twice);
#include "mods.h"
#include "mod_hook.h"
#include "core.h"
#include "display.h"
#include "swfdata.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

namespace fl {
namespace {
Value V(const Value& o, const char* k) { return o.isObj() ? getMemK(o, intern(k)) : VUNDEF; }
void S(const Value& o, const char* k, const Value& v) { if (o.isObj()) setMemK(o, intern(k), v); }
double num(const Value& v, double def = 0) { double d = toNum(v); return std::isfinite(d) ? d : def; }
Value root() { return Value((Object*)vm.root); }
std::string str(const Value& v) { return v.t == T_STR ? v.s->s : std::string(); }
bool isOld() { return !g_game || strcmp(g_game->tag, "eternal") != 0; }   // og / wotl

struct Opts { bool subtitles = true, names = true, eternalHeart = true, momVoice = true, challengeMusic = true,
              heartName = true, lastWill = true, shopPills = true, challengePause = true, momRed = true, gamekidHead = true,
              daddyStomps = true, footShadow = true, unlocks = true, momDeals = true, breakfast = true, xlDepths = true, forgetMeNow = true, secretsKeys = true, selectClick = true, startSpot = true, familiarShots = true, familiars = true; } g_o;
bool isEternal() { return g_game && strcmp(g_game->tag, "eternal") == 0; }
bool isOg() { return g_game && strcmp(g_game->tag, "og") == 0; }
Value arrAt(const Value& a, int i) {
    if (!a.isObj() || a.o->kind != OK_ARRAY) return VUNDEF;
    Array* arr = (Array*)a.o;
    return i >= 0 && i < (int)arr->el.size() ? arr->el[i] : VUNDEF;
}
void arrSet(const Value& a, int i, const Value& v) { if (a.isObj() && a.o->kind == OK_ARRAY) setMem(a, Value((double)i), v); }
Value callFn(const Value& obj, const char* fn, std::initializer_list<Value> args) {
    std::vector<Value> av(args);
    Act ac{vm.globalScope, VUNDEF, vm.root, nullptr};
    Value r = callMethodK(ac, obj, intern(fn), (int)av.size(), av.data());
    runActions();
    return r;
}
MovieClip* clipOf(const Value& v) { return v.isObj() && v.o->kind == OK_CLIP ? (MovieClip*)v.o : nullptr; }
uint32_t g_tick = 0;

// ---- subtitles: Eternal Edition's lines (st2x), where the older games' were wrong
const std::pair<int, const char*> kSubs[] = {
    {55, "Extra Tears"}, {79, "Dmg + Speed!"}, {80, "Dmg + Tears!"}, {87, "Extra Tears"}, {90, "Dmg Up + Tears"},
    {120, "Tears + Speed"}, {121, "Dmg + HP + Range"}, {154, "Dmg up!"}, {176, "HP + Shot Speed"}, {182, "Dmg + Homing"},
    {183, "Shot Speed Up"}, {193, "Dmg + HP Up"}, {194, "Shot Speed up"}, {197, "Dmg Up + Range"}};
void subtitlesTick() {
    Value st2 = V(root(), "st2x");
    if (!st2.isObj() || st2.o->kind != OK_ARRAY) return;
    Array* arr = (Array*)st2.o;
    for (auto& s : kSubs) {
        if (s.first >= (int)arr->el.size()) continue;   // (the original has 132 items)
        const Value& cur = arr->el[s.first];
        if (cur.t != T_STR || cur.s->s != s.second) setMem(st2, Value((double)s.first), Value(mkstr(s.second)));
    }
}

// ---- names: the mini-boss banner (st11 "<character> vs <boss>")
GameHook g_wSt11{"st11"};
Value wSt11(Act& ac, Value thisv, int argc, Value* argv) {
    if (g_o.names && argc >= 1 && argv[0].t == T_STR) {
        std::string t = argv[0].s->s;
        if (t.compare(0, 13, "Magdalena vs ") == 0) t = "Magdalene" + t.substr(9);
        else if (t.compare(0, 7, "XXX vs ") == 0) t = "???" + t.substr(3);
        if (t != argv[0].s->s) { Value v[1] = {Value(mkstr(t.c_str()))}; return callGameOrig(g_wSt11, ac, thisv, 1, v); }
    }
    return callGameOrig(g_wSt11, ac, thisv, argc, argv);
}

// ---- eternal_heart: playerhurt's death test, as Eternal's
GameHook g_wHurt{"playerhurt"};
Value wHurt(Act& ac, Value thisv, int argc, Value* argv) {
    Value a((Object*)g_wHurt.a), R = root(), pl = V(a, "player");
    // foot_shadow: Mom (45) and Satan (84) are dead (momkill: momdown counts down from 100): their falling foot no
    // longer hurts
    if (g_o.footShadow && argc >= 2 && (num(argv[1]) == 45 || num(argv[1]) == 84) && num(V(a, "momdown")) > 0) {
        logf("goodsaac: no damage from a dead boss's foot\n");
        return Value(false);
    }
    bool was = truthy(V(pl, "dones"));
    Value r = callGameOrig(g_wHurt, ac, thisv, argc, argv);
    if (!g_o.eternalHeart || !isOld() || was || !truthy(V(pl, "dones")) || !truthy(V(R, "eta"))) return r;
    if (argc >= 3 && truthy(argv[2])) return r;   // (f3: a hit that can't kill; not the death test's doing)
    double hp = num(V(pl, "hp")), mhp = num(V(pl, "mhp")), armor = num(V(R, "armor"));
    bool origDies = (hp <= 0 && armor <= 0) || !(mhp > 0 || armor > 0);
    if (!origDies) return r;   // (died some other way)
    // Eternal: (hp <= 0 && armor <= 0 && !eta) || !(mhp > 0 || armor > 0 || eta) - false with eta: alive, as its else
    S(pl, "dones", Value(false));
    Value d = V(pl, "d");
    if (d.isObj() && d.o->kind == OK_CLIP) gotoFrame((MovieClip*)d.o, 2, true);
    logf("goodsaac: the eternal heart kept the player alive (hp %.1f, containers %.1f, soul %.1f)\n", hp, mhp, armor);
    return r;
}

// ---- mom_voice: Mom's Heart's "Isaac!" (and its heartout) at most once a second
GameHook g_wSoundy{"soundy"};
uint32_t g_lastVoice = 0, g_lastOut = 0;
Value wSoundy(Act& ac, Value thisv, int argc, Value* argv) {
    if (g_o.momVoice && argc >= 1 && argv[0].t == T_STR) {
        const std::string& n = argv[0].s->s;
        uint32_t* last = n.compare(0, 23, "Mom_Vox_Filtered_Isaac_") == 0 ? &g_lastVoice : n == "heartout.wav" ? &g_lastOut : nullptr;
        if (last) {
            if (*last && g_tick - *last < 30) return VUNDEF;
            *last = g_tick;
        }
    }
    return callGameOrig(g_wSoundy, ac, thisv, argc, argv);
}

// ---- challenge_music
void challengeMusicTick() {
    Value R = root(), m = V(R, "musc");
    std::string n = str(V(m, "nam"));
    if (n.size() < 3 || (n.compare(0, 3, "m6-") != 0 && n.compare(0, 4, "m27-") != 0)) return;
    if (truthy(V(R, "amusic")) || truthy(V(R, "bossmusic")) || truthy(V(R, "fade")) || truthy(V(R, "amas"))) return;
    S(R, "fade", Value(true));
    logf("goodsaac: challenge music faded (%s)\n", n.c_str());
}

// ---- heart_name: "<3" in the collection's HTML text field (the pickup banner's isn't HTML: it shows it)
void heartNameTick(const Value& R) {
    Value sheet = V(V(R, "men"), "men");
    if (str(V(sheet, "ittz")) == "<3") { S(sheet, "ittz", Value(mkstr("&lt;3"))); updateBoundTexts(); }
}

// ---- last_will: the death diary's picture
MovieClip* g_willKill = nullptr; bool g_willDone = false;
bool hasKids(MovieClip* c) { for (DisplayObject* k : c->kids) if (k && !k->removed) return true; return false; }
void lastWillTick(const Value& R, const Value& a) {
    Value paus = V(V(R, "hud"), "paus");
    MovieClip* pc = clipOf(paus), *kill = clipOf(V(paus, "kill"));
    if (!pc || pc->cur != 3 || !kill) { g_willKill = nullptr; g_willDone = false; return; }
    if (kill != g_willKill) { g_willKill = kill; g_willDone = false; }
    if (g_willDone || hasKids(kill)) { g_willDone = true; return; }
    g_willDone = true;
    int f = kill->cur;
    if (f == 202) {   // Razor Blade: its grey drawing (the diary's item list: paus.i0, the item sprite with every item)
        MovieClip* i0 = clipOf(V(paus, "i0"));
        if (!i0 || !i0->def || i0->def->nframes < 126) return;
        RectF box;   // (the size of the other pictures: the pill's, frame 201)
        gotoFrame(kill, 201, true);
        for (DisplayObject* k : kill->kids) if (k && !k->removed) { RectF b = localBounds(k); if (b.valid()) box = b; }
        gotoFrame(kill, 202, true);
        if (!box.valid()) return;
        DisplayObject* m = synthPut(kill, i0->charId, 1, 0, 0);
        if (!m || m->kind != OK_CLIP) return;
        gotoFrame((MovieClip*)m, 126, true);
        RectF ib = localBounds(m);
        if (!ib.valid()) return;
        double sc = std::min((box.x1 - box.x0) / (ib.x1 - ib.x0), (box.y1 - box.y0) / (ib.y1 - ib.y0)) ;
        m->m = Mat(); m->m.a = m->m.d = sc;
        m->m.tx = (box.x0 + box.x1) / 2 - (ib.x0 + ib.x1) / 2 * sc; m->m.ty = (box.y0 + box.y1) / 2 - (ib.y0 + ib.y1) / 2 * sc;
        m->scriptXform = true;
        logf("goodsaac: the diary shows the Razor Blade\n");
        return;
    }
    // "killed by nothing": the nearest enemy (s >= 10) whose picture isn't empty
    Value pl = V(a, "player"), balls = V(a, "ball");
    double px = num(V(pl, "xp")), py = num(V(pl, "yp"));
    std::vector<std::pair<double, int>> cand;
    if (balls.isObj() && balls.o->kind == OK_ARRAY)
        for (const Value& b : ((Array*)balls.o)->el) {
            double sv = num(V(b, "s"), -1);
            if (sv < 10 || sv != std::floor(sv) || sv > 202) continue;
            double dx = num(V(b, "xp")) - px, dy = num(V(b, "yp")) - py;
            cand.push_back({dx * dx + dy * dy, (int)sv});
        }
    std::sort(cand.begin(), cand.end());
    for (auto& c : cand) {
        gotoFrame(kill, c.second, true);
        if (hasKids(kill)) { logf("goodsaac: the diary's empty picture (%d) shows enemy %d\n", f, c.second); return; }
    }
    gotoFrame(kill, f, true);
}

// ---- shop_pills: a pill for sale shows the run's colour of it (pic(col)), as on the floor. A shop slot's picture is
// d.d.d (frame col: the same sprite as a floor pill's p, frames 1-6 and the run's special colours 101-103)
void shopPillsTick(const Value& R, const Value& a) {
    Value picers = V(R, "picers"), balls = V(a, "ball");
    if (!picers.isObj() || !balls.isObj() || balls.o->kind != OK_ARRAY) return;
    for (const Value& b : ((Array*)balls.o)->el) {
        if (num(V(b, "s")) != 5) continue;
        MovieClip* d = clipOf(V(b, "d"));
        if (!d || d->cur != 15) continue;
        MovieClip* dd = clipOf(V(V(b, "d"), "d"));
        double col = num(V(b, "col"));
        if (!dd || (dd->cur != 3 && dd->cur != 7) || col < 1 || col > 6 || col != std::floor(col)) continue;
        int f = (int)num(arrAt(picers, (int)col - 1), col);
        MovieClip* pic = clipOf(V(V(V(b, "d"), "d"), "d"));
        if (pic && pic->cur != f && pic->def && f <= (int)pic->def->nframes) gotoFrame(pic, f, true);
    }
}

// ---- challenge_pause: the wave timer (chamb) holds while the game is paused
double g_chambHeld = -1;
void challengePauseTick(const Value& R, const Value& a) {
    bool waves = num(V(a, "chama")) > 1 && !truthy(V(R, "chama"));
    if (!waves || truthy(V(a, "unpause"))) { g_chambHeld = -1; return; }
    if (g_chambHeld < 0) g_chambHeld = std::min(num(V(a, "chamb")), 48.0);
    S(a, "chamb", Value(g_chambHeld));
}

// ---- mom_red: a new room while Mom's red flash (or the light beam's white) was on
Object* g_lastA = nullptr; bool g_tinted = false;
void momRedTick(const Value& R, const Value& a) {
    if (a.o != g_lastA) {
        if (g_lastA && g_tinted) { callFn(a, "colorit", {R, Value(1.0), Value(1.0), Value(1.0), Value(0.0), Value(0.0), Value(0.0)}); logf("goodsaac: the screen's colour put back (new room during Mom's flash)\n"); }
        g_lastA = a.o;
    }
    g_tinted = num(V(a, "momdown")) > 0 || num(V(a, "beamer")) > 0;
}

// ---- gamekid_head
Object* g_pacA = nullptr;   // (the room in which The Gamekid was on: only its head is put back)
void gamekidHeadTick(const Value& a) {
    if (truthy(V(a, "pacman"))) { g_pacA = a.o; return; }
    if (g_pacA != a.o) return;
    Value h = V(V(V(V(V(a, "player"), "d"), "d"), "d"), "d");
    if (!clipOf(h)) return;
    if (num(V(h, "_xscale"), 100) < 0) { S(h, "_xscale", Value(100.0)); S(h, "_x", Value(0.0)); logf("goodsaac: the head turned back after The Gamekid\n"); }
}

// ---- daddy_stomps
std::map<Object*, int> g_dadFrame;   // (testing, ISAAC_GOODLOG: stomps)
void daddyTick(const Value& a, bool fix) {
    Value balls = V(a, "ball"), pl = V(a, "player");
    if (!balls.isObj() || balls.o->kind != OK_ARRAY) return;
    static const bool dbg = getenv("ISAAC_GOODLOG") != nullptr;
    if (dbg) for (const Value& b : ((Array*)balls.o)->el) {
        if (!truthy(V(b, "dad"))) continue;
        MovieClip* dd = clipOf(V(V(b, "d"), "d")); int f = dd ? dd->cur : 0;
        if (g_dadFrame[b.o] == 1 && f == 2) logf("goodsaac: [dbg] daddy stomps (tick %u)\n", g_tick);
        g_dadFrame[b.o] = f;
    }
    if (!fix) return;
    for (const Value& b : ((Array*)balls.o)->el) {
        if (!truthy(V(b, "dad")) || truthy(V(b, "whaf")) || num(V(b, "fire")) > 0) continue;
        MovieClip* dd = clipOf(V(V(b, "d"), "d"));
        Value t = V(b, "trg2");
        if (!dd || dd->cur != 1 || !t.isObj() || t.o == pl.o || truthy(V(t, "dones")) || num(V(t, "hp")) <= 0) continue;
        double dx = num(V(t, "xp")) - num(V(b, "xp")), dy = num(V(t, "yp")) - num(V(b, "yp"));
        if (dx * dx + dy * dy >= 30 * 30) continue;
        gotoFrame(dd, 2, true);   // (the game's own start of a stomp: d.d.nextFrame(), stomps 1-3)
        if (dbg) logf("goodsaac: [dbg] daddy stomp started by the fix\n");
        S(b, "stomps", Value((double)(g_tick % 3 + 1)));
    }
}

// ---- unlocks (bosssp / giveit)
bool lk(const Value& L, int i) { return truthy(arrAt(L, i)); }
GameHook g_wBoss{"bosssp"};
Value wBoss(Act& ac, Value thisv, int argc, Value* argv) {
    Value R = root(), L = V(R, "locker");
    double chaps = num(V(R, "chaps")), chala = num(V(R, "chala")), skiner = num(V(R, "skiner"));
    bool b94 = lk(L, 94), b97 = lk(L, 97), b4 = lk(L, 4);
    // xl_depths: the first boss of an XL Depths (bossl2) as a normal boss, not as Mom
    bool xl = g_o.xlDepths && !isOg() && chaps == 6 && truthy(V(R, "double")) && num(V(R, "lev")) == num(V(R, "bossl2"))
              && (!b4 || (chala >= 1 && chala <= 4));
    if (xl) { arrSet(L, 4, Value(true)); S(R, "chala", Value(0.0)); logf("goodsaac: XL Depths' first boss is a normal boss\n"); }
    Value r = callGameOrig(g_wBoss, ac, thisv, argc, argv);
    if (xl) { arrSet(L, 4, Value(b4)); S(R, "chala", Value(chala)); }
    if (g_o.unlocks && !isOg()) {
        auto swap = [&](int wrong, bool wasWrong, int right, const char* what) {
            if (wasWrong || !lk(L, wrong)) return;
            arrSet(L, wrong, Value(false)); arrSet(L, right, Value(true));
            logf("goodsaac: %s unlocks secret %d (the game gave %d)\n", what, right, wrong);
        };
        if (chaps == 11 && skiner == 4) swap(97, b97, 94, "The Chest with ???");
        if (chaps == 11 && skiner == 5) swap(94, b94, 97, "The Chest with Eve");
    }
    return r;
}
GameHook g_wGive2{"giveit"};
int g_giveDepth = 0, g_itzCalls = 0;
Value wGive2(Act& ac, Value thisv, int argc, Value* argv) {
    Value R = root(), L = V(R, "locker");
    bool b32 = lk(L, 32);
    if (g_giveDepth++ == 0) g_itzCalls = 0;
    Value r = callGameOrig(g_wGive2, ac, thisv, argc, argv);
    g_giveDepth--;
    if (g_o.unlocks && !b32 && lk(L, 32) && num(V(R, "bosss")) == 22) { arrSet(L, 32, Value(false)); logf("goodsaac: the Headless Horseman doesn't unlock C.H.A.D.'s secret\n"); }
    return r;
}

// ---- breakfast: giveit loops until itz() accepts an item (itz refuses the ones you have, and more); with every item
// owned (rerolls, Habit, Scapular...) it never ends and the game freezes. As Rebirth does: Breakfast (25) instead
GameHook g_wItz{"itz"};
Value wItz(Act& ac, Value thisv, int argc, Value* argv) {
    if (g_o.breakfast && g_giveDepth > 0 && ++g_itzCalls > 600) {
        if (g_itzCalls == 601) logf("goodsaac: no item left to give: Breakfast\n");
        return Value(25.0);
    }
    return callGameOrig(g_wItz, ac, thisv, argc, argv);
}

// ---- mom_deals: in Mom's room, her hands and eyes come out of the door holes and hide the doors (door._visible = false
// when a part settles on one): the devil / angel room door the room-clear roll opened stays hidden. It is shown again
// once she is dead
void momDealsTick(const Value& R, const Value& a) {
    if (num(V(R, "lev")) != num(V(R, "bossl")) || num(V(R, "devil")) != 2 || !truthy(V(a, "keyo")) || num(V(a, "momdown")) <= 0) return;
    Value doors = V(a, "door");
    if (!doors.isObj() || doors.o->kind != OK_ARRAY) return;
    for (const Value& d : ((Array*)doors.o)->el) {
        if (!d.isObj() || !d.o->isDisplay() || !truthy(V(d, "sat"))) continue;
        DisplayObject* o = (DisplayObject*)d.o;
        if (o->visible) continue;
        o->visible = true;   // (and its hole opened, as the room-clear code does for a door that was visible: levz / clevc 0)
        double blo = num(V(d, "blo"), -1);
        if (blo >= 0) { arrSet(V(a, "levz"), (int)blo, Value(0.0)); arrSet(V(a, "clevc"), (int)blo, Value(0.0)); }
        logf("goodsaac: the deal's door in Mom's room is shown\n");
    }
}

// ---- forget_me_now: the Cathedral stays a Cathedral (moveon is called by newstart, before the floor is made)
GameHook g_wMoveon{"moveon"};
Value wMoveon(Act& ac, Value thisv, int argc, Value* argv) {
    Value R = root();
    if (g_o.forgetMeNow && num(V(R, "shroom")) == 3 && num(V(R, "altchap")) == 9 && num(V(R, "chaps")) == 9) {
        S(R, "heaven", Value(2.0));
        logf("goodsaac: Forget Me Now in the Cathedral: a Cathedral again\n");
    }
    return callGameOrig(g_wMoveon, ac, thisv, argc, argv);
}

// ---- secrets_keys: one secret per arrow key on the secrets screen
bool g_keyWas[2] = {false, false}; int g_pressTick = -100; double g_axBase = 0, g_axLast = 0;
void secretsKeysTick(const Value& R) {
    Value sheet = V(V(R, "men"), "men");
    bool on = num(V(sheet, "pos")) == 1 && truthy(V(sheet, "achs"));
    bool down[2] = {g_player.keys[37], g_player.keys[39]};
    bool press = (down[0] && !g_keyWas[0]) || (down[1] && !g_keyWas[1]);
    g_keyWas[0] = down[0]; g_keyWas[1] = down[1];
    double ax = num(V(sheet, "ax"));
    if (!on) { g_axLast = ax; g_pressTick = -100; return; }
    if (press) { g_pressTick = (int)g_tick; g_axBase = g_axLast; }
    if ((int)g_tick - g_pressTick <= 2) {
        Value lo = V(R, "lockor");
        int n = lo.isObj() && lo.o->kind == OK_ARRAY ? (int)((Array*)lo.o)->el.size() : 0;
        if (n > 2) {
            int d = (((int)ax - (int)g_axBase) % n + n) % n;
            if (d == 2) ax = ((int)g_axBase + 1) % n;
            else if (d == n - 2) ax = ((int)g_axBase - 1 + n) % n;
            if (ax != num(V(sheet, "ax"))) { S(sheet, "ax", Value(ax)); logf("goodsaac: the secrets screen moves one secret per key\n"); }
        }
    }
    g_axLast = ax;
}

// ---- familiar_shots: Robo-Baby's laser no longer turns the next familiars' shots
GameHook g_wPlff{"plff"}, g_wLaser{"lasershow"};
int g_inPlff = 0;
Value wPlff(Act& ac, Value t, int argc, Value* argv) {
    g_inPlff++;
    Value r = callGameOrig(g_wPlff, ac, t, argc, argv);
    g_inPlff--;
    return r;
}
Value wLaser(Act& ac, Value t, int argc, Value* argv) {
    if (g_inPlff == 0) return callGameOrig(g_wLaser, ac, t, argc, argv);
    Value a((Object*)g_wLaser.a);
    Value x = V(a, "xenf"), y = V(a, "yenf");
    Value r = callGameOrig(g_wLaser, ac, t, argc, argv);
    S(a, "xenf", x); S(a, "yenf", y);
    return r;
}

Value callA(const Value& obj, const char* fn, std::initializer_list<Value> args) {
    std::vector<Value> av(args);
    Act ac{vm.globalScope, VUNDEF, vm.root, nullptr};
    Value r = callMethodK(ac, obj, intern(fn), (int)av.size(), av.data());
    runActions();
    return r;
}
// ---- familiars: as many babies as baby items
GameHook g_wFriends{"friends"};
const int kBabies[10][2] = {{8, 0}, {67, 1}, {95, 2}, {99, 3}, {100, 4}, {113, 5}, {163, 6}, {167, 7}, {174, 8}, {188, 9}};   // item, kind
std::map<Object*, int> g_babyKind;   // each baby's kind (`baa`), while there are more than the game's own number
Value wFriends(Act& ac, Value t, int argc, Value* argv);
// a baby: a follower (s 3) that isn't one of the others. Every follower is made by create(..., 3) and gets `baa`;
// the others are marked when made (onEnterFrame): The Relic and its kind `mon`, Dead Cat's `cat`, Cube of Meat's
// `meat`, Guardian Angel's `ang`, the flies' `fly`, `bum`, `eye`, `hairb`, `hol`, `ni`, `bird`, `ggho`, `dad`, `maga`,
// `bluf` (friends' own test is !maga && !bum && !hol && !cat && !mon after its branches for the rest)
bool isBaby(const Value& e) {
    static const char* kOthers[] = {"mon", "cat", "meat", "ang", "fly", "bum", "eye", "hairb", "hol", "ni", "bird", "ggho", "dad", "maga", "bluf"};
    if (!e.isObj() || num(V(e, "s")) != 3 || V(e, "baa").t != T_NUM || truthy(V(e, "done"))) return false;
    for (const char* k : kOthers) if (truthy(V(e, k))) return false;
    return true;
}
void familiarsTick(const Value& R, const Value& a) {
    int cap = isOg() ? 2 : 3;
    Value ups = V(R, "ups"), p = V(a, "player"), balls = V(a, "ball");
    if (!ups.isObj() || !p.isObj() || !balls.isObj() || balls.o->kind != OK_ARRAY) return;
    std::vector<int> want;   // the kinds he should have, one per baby item (and copy)
    for (auto& b : kBabies) {
        if (isOg() && b[0] > 113) continue;
        int n = (int)num(getMem(ups, Value((double)b[0])), 0);
        for (int i = 0; i < n && want.size() < 40; i++) want.push_back(b[1]);
    }
    if ((int)want.size() <= cap) { g_babyKind.clear(); return; }   // (the game's own number: nothing to do)
    hookGameFn(g_wFriends, a, wFriends);
    std::vector<Value> babies;
    for (const Value& e : ((Array*)balls.o)->el)
        if (isBaby(e)) babies.push_back(e);
    double minions = num(V(a, "minions"), 0);
    Value d = V(p, "d");
    bool still = num(V(d, "_currentframe"), 1) < 3;   // (the game makes them only then: not while he changes room)
    if ((int)babies.size() < (int)want.size() && minions >= cap && minions < want.size() && still) {
        S(a, "minions", Value(minions + 1));
        callA(a, "create", {Value(num(V(p, "xp"))), Value(num(V(p, "yp")) - 10), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(3.0)});
        return;   // (its kind comes from friends next image; set below after that)
    }
    // the kinds: friends works each baby's out again every image (from the items, skipping the last two given), so
    // each baby keeps one here - the same as long as it is needed - and friends' choice is replaced right after it
    std::map<Object*, int> keep;
    std::vector<int> need = want;
    for (const Value& b : babies) {
        auto it = g_babyKind.find(b.o);
        if (it == g_babyKind.end()) continue;
        auto n = std::find(need.begin(), need.end(), it->second);
        if (n != need.end()) { keep[b.o] = it->second; need.erase(n); }
    }
    for (const Value& b : babies) if (!keep.count(b.o) && !need.empty()) { keep[b.o] = need.front(); need.erase(need.begin()); }
    g_babyKind.swap(keep);
}
Value wFriends(Act& ac, Value t, int argc, Value* argv) {
    // friends picks the kind from the items (in its own order, skipping the last two it gave: lastbb, lastbbb); for
    // the call on one of these babies it sees only that baby's item and no last ones, so it picks that kind - and
    // moves and draws it as that kind - and then everything is put back
    Value a((Object*)g_wFriends.a), R = root();
    Value trg = V(a, "trg"), ups = V(a, "ups").isObj() ? V(a, "ups") : V(R, "ups");
    auto it = trg.isObj() ? g_babyKind.find(trg.o) : g_babyKind.end();
    if (it == g_babyKind.end() || !ups.isObj()) return callGameOrig(g_wFriends, ac, t, argc, argv);
    Value saved[10];
    for (int i = 0; i < 10; i++) { Value k((double)kBabies[i][0]); saved[i] = getMem(ups, k); setMem(ups, k, Value(kBabies[i][1] == it->second ? 1.0 : 0.0)); }
    Value lb = V(a, "lastbb"), lbb = V(a, "lastbbb");
    S(a, "lastbb", Value(-1.0)); S(a, "lastbbb", Value(-1.0));
    Value r = callGameOrig(g_wFriends, ac, t, argc, argv);
    for (int i = 0; i < 10; i++) setMem(ups, Value((double)kBabies[i][0]), saved[i]);
    S(a, "lastbb", lb); S(a, "lastbbb", lbb);
    return r;
}

// ---- start_spot: no place left over from the last run
int g_lastRootFrame = 0;
void startSpotTick(const Value& R) {
    int frame = vm.root->kind == OK_CLIP ? ((MovieClip*)vm.root)->cur : 0;
    if (frame == 5 && g_lastRootFrame != 5 && (!V(R, "playerx").isUndef() || !V(R, "playery").isUndef())) {
        S(R, "playerx", VUNDEF); S(R, "playery", VUNDEF); S(R, "door", VUNDEF);
    }
    g_lastRootFrame = frame;
}

// ---- select_click: the character select's sound when the keys change the character
double g_skinLast = -1;
void selectClickTick(const Value& R) {
    Value sheet = V(V(R, "men"), "men"), cha = V(sheet, "cha");
    if (num(V(sheet, "pos")) != 3 || !cha.isObj()) { g_skinLast = -1; return; }
    double sk = num(V(cha, "skiner"));
    if (g_skinLast >= 0 && sk != g_skinLast && (g_player.keys[37] || g_player.keys[39]))
        callFn(R, "soundy", {Value(mkstr(g_player.keys[37] ? "character_select_left.wav" : "character_select_right.wav"))});
    g_skinLast = sk;
}
}  // namespace

void goodsaacDebug(const std::string&) {   // (tests: "eval gsaac babies") every follower, and which are babies
    Value R = root(), a = V(R, "a"), balls = V(a, "ball");
    if (!balls.isObj() || balls.o->kind != OK_ARRAY) return;
    int n = 0;
    for (const Value& e : ((Array*)balls.o)->el) {
        if (!e.isObj() || num(V(e, "s")) != 3 || truthy(V(e, "done"))) continue;
        bool b = isBaby(e); n += b;
        auto it = g_babyKind.find(e.o);
        logf("[gsaac] follower baa %g mon %g baby %d kept %d\n", num(V(e, "baa")), num(V(e, "mon")), (int)b, it == g_babyKind.end() ? -1 : it->second);
    }
    logf("[gsaac] babies %d minions %g\n", n, num(V(a, "minions")));
}

void goodsaacTick(const ModInfo& m) {
    auto on = [&](const char* k) { std::string v = m.opt(k, "1"); return !(v == "0" || v == "no" || v == "off" || v == "false"); };
    g_o.subtitles = on("subtitles"); g_o.names = on("names"); g_o.eternalHeart = on("eternal_heart");
    g_o.momVoice = on("mom_voice"); g_o.challengeMusic = on("challenge_music");
    g_o.heartName = on("heart_name"); g_o.lastWill = on("last_will"); g_o.shopPills = on("shop_pills");
    g_o.challengePause = on("challenge_pause"); g_o.momRed = on("mom_red"); g_o.gamekidHead = on("gamekid_head");
    g_o.daddyStomps = on("daddy_stomps"); g_o.footShadow = on("foot_shadow"); g_o.momDeals = on("mom_deals"); g_o.breakfast = on("breakfast"); g_o.unlocks = on("unlocks"); g_o.xlDepths = on("xl_depths");
    g_o.forgetMeNow = on("forget_me_now"); g_o.secretsKeys = on("secrets_keys"); g_o.selectClick = on("select_click");
    g_o.startSpot = on("start_spot"); g_o.familiarShots = on("familiar_shots"); g_o.familiars = on("familiars");
    g_tick++;
    bool old = isOld();
    Value R = root(), a = V(R, "a");
    if (old && g_o.subtitles) subtitlesTick();
    if (old && g_o.momVoice) hookGameFn(g_wSoundy, R, wSoundy);
    if (g_o.challengeMusic) challengeMusicTick();
    if (g_o.heartName) heartNameTick(R);
    if (g_o.secretsKeys && !isEternal()) secretsKeysTick(R);
    if (g_o.selectClick && !isEternal()) selectClickTick(R);   // (Eternal plays it itself: it sounded twice)
    if (g_o.startSpot) startSpotTick(R);
    if (!a.isObj()) return;
    if (g_o.lastWill) lastWillTick(R, a);
    if (g_o.shopPills && !isOg()) shopPillsTick(R, a);
    if (g_o.challengePause) challengePauseTick(R, a);
    if (g_o.momRed) momRedTick(R, a);
    if (g_o.gamekidHead) gamekidHeadTick(a);
    if (!isOg()) daddyTick(a, g_o.daddyStomps);
    if (g_o.unlocks || g_o.xlDepths) hookGameFn(g_wBoss, a, wBoss);
    if (g_o.unlocks || g_o.breakfast) hookGameFn(g_wGive2, a, wGive2);
    if (g_o.breakfast) hookGameFn(g_wItz, a, wItz);
    if (g_o.momDeals) momDealsTick(R, a);
    if (g_o.forgetMeNow && !isOg()) hookGameFn(g_wMoveon, a, wMoveon);
    if (g_o.familiars) familiarsTick(R, a);
    if (g_o.familiarShots) { hookGameFn(g_wPlff, a, wPlff); hookGameFn(g_wLaser, a, wLaser); }
    if (old && g_o.names) hookGameFn(g_wSt11, a, wSt11);
    if ((old && g_o.eternalHeart) || g_o.footShadow) hookGameFn(g_wHurt, a, wHurt);
}
}  // namespace fl
