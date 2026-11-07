// Challenge Lab (mods/ChallengeLab.mod): up to 8 challenges of your own, made with tools/challenge_lab_maker.html
// and kept in mods/customchallenges.txt (Wrath of the Lamb and Eternal Edition: the original game has no challenges).
//
// The menu's challenge list (the "To Do List": _root.pauss frame 9, pos 7 of the main menu) gets an arrow on its right,
// like the collection's: it turns the page to the Challenge Lab list, which ends with an arrow back. Each challenge
// says where it ends (Mom, Mom's Heart, Isaac, Satan, The Chest), the character (The Gone too, with his mod on), the
// starting items and trinket, the rules the game's own challenges use (darkness, the labyrinth or the Lost on every
// floor, fewer or no treasure rooms, bad luck) and, if wanted, other base stats. All of them can be played from the
// start, and their runs are runs apart: nothing they do is kept (no unlocks, secrets, endings or statistics).
//
// How a run follows those rules, with the game's own code wherever it has one (NOTES.md "v1.6: Challenge Lab"):
//   - _root.chala is the game's challenge number. During a run it is 11 (a challenge, none of the ten): no starting
//     items or D6 of a normal run, no reward of the ten (_root.lox[11] is nothing), Mom's trapdoor stays open;
//   - while a floor is made (the game calls _root.darky(120) first, wrapped here) it is the number whose treasure
//     rules are wanted (2 fewer, 5 none, 0 the usual) and the curses are held on as the game writes them (_root's
//     write guard: darks, double, big);
//   - the ending: Mom ends the run as challenges 1-4 do; after Mom's Heart, the chest ends the run (Mom's Heart), is
//     the beam to the Cathedral (Isaac, The Chest) or gives way to the trapdoor to Sheol (Satan); after Isaac, The
//     Chest's way is open as with the Polaroid (goal The Chest);
//   - a run apart: the save (so.data) is copied when the run starts and put back when it ends, nothing is written to
//     disk meanwhile (g_saveLocked) and no unlock flag can turn on (_root.locker / ulocker write guard);
//   - the run's rules live in a variable of _root (clabRun), so they go on after Continuum's resume.
// A finished challenge gets "complete = 1" in customchallenges.txt (not while GenesisConsole is on).
#include "mods.h"
#include "mod_hook.h"
#include "core.h"
#include "display.h"
#include "platform.h"
#include "swfdata.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace fl {
bool recompInGameplay(bool* inGame);
extern bool g_saveLocked;
extern std::string g_modsSaveDir;
extern DisplayObject* (*g_challengeNavHook)(bool right);   // menuui.cpp
uint32_t continuumResumes();   // mod_continuum.cpp
void goneHookItems(const Value& a);   // mod_thegone.cpp
extern std::string g_modsDir;
extern bool (*g_modMouseHook)(double x, double y, int type);   // display.cpp: before the buttons (true: taken)
extern bool (*g_modHandHook)(double x, double y);   // display.cpp / menuui.cpp: the hand cursor
// (mod_thegone.cpp) the character cards' lettering
bool cardLetters();
bool cardHasLetter(char c);
double cardLetterH();
double cardTextW(const std::string& t, double k);
uint16_t cardTextShape(const std::string& t, double k, double x, double cy);

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
int curFrame(const Value& v) { return v.isObj() && v.o->kind == OK_CLIP ? ((MovieClip*)v.o)->cur : 0; }
std::string trim(std::string s) {
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r' || s.back() == '\n')) s.pop_back();
    size_t i = 0; while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) i++;
    return s.substr(i);
}
std::string lower(std::string s) { for (auto& c : s) c = (char)tolower((unsigned char)c); return s; }
std::string fmtNum(double d) { char b[32]; snprintf(b, sizeof b, "%g", d); return b; }

// ============================================================ the challenges (mods/customchallenges.txt)
enum Goal { G_MOM, G_HEART, G_ISAAC, G_SATAN, G_CHEST };
const char* kGoalKey[] = {"mom", "moms_heart", "isaac", "satan", "the_chest"};
const int kGoalChaps[] = {6, 8, 9, 9, 11};
enum Treasure { T_NORMAL, T_FEWER, T_NONE };
// characters by _root.skiner (the game's order), then the mods' (7 The Gone: Isaac without his mod)
const char* kCharKey[] = {"isaac", "magdalene", "cain", "judas", "bluebaby", "eve", "samson", "thegone"};
const int kChars = 8, kGone = 7;
const int kMaxChallenges = 8, kMaxName = 20;   // (the game's longest names: "Isaac Was Good Today", "The Doctors Revenge!")

struct Chal {
    std::string name;
    int goal = G_MOM, character = 0, treasure = T_NORMAL, trinket = 0;
    bool dark = false, maze = false, lost = false, blind = false, badLuck = false, complete = false, hard = false;
    std::vector<int> items;
    // advanced: changes to the character's own stats (NAN / -1: as the character has them)
    double speed = NAN, tears = NAN, damage = NAN, range = NAN, shot = NAN, luck = NAN;
    int hearts = -1, soul = -1, coins = -1, bombs = -1, keys = -1;
};
std::vector<Chal> g_list;

std::string fileText(const std::string& path) {
    FILE* f = platformFopen(path, "rb"); if (!f) return "";
    std::string s; char b[4096]; size_t n;
    while ((n = fread(b, 1, sizeof b, f)) > 0) s.append(b, n);
    fclose(f);
    return s;
}
std::string listPath() { return g_modsDir + "customchallenges.txt"; }
// names are written with the menus' hand-drawn letters: letters, digits and : . - ! ? / ( ) and spaces
std::string cleanName(const std::string& s) {
    std::string o;
    for (char c : s) {
        if (isalnum((unsigned char)c) || strchr(" :.-!?/()", c)) o += c;
        if ((int)o.size() >= kMaxName) break;
    }
    return trim(o);
}
double numOr(const std::string& v, double def) { char* e = nullptr; double d = strtod(v.c_str(), &e); return e != v.c_str() && std::isfinite(d) ? d : def; }
void setKV(Chal& c, const std::string& key, std::string v) {
    std::string k = lower(trim(key));
    v = trim(v);
    size_t sc = v.find(';'); if (sc != std::string::npos && k != "name") v = trim(v.substr(0, sc));   // (a comment after the value)
    std::string lv = lower(v);
    auto cnt = [&](int def) { return v.empty() ? def : std::max(-1, std::min(99, (int)numOr(v, def))); };
    if (k == "name") c.name = cleanName(v);
    else if (k == "goal") { for (int i = 0; i < 5; i++) if (lv == kGoalKey[i]) c.goal = i; if (lv == "chest") c.goal = G_CHEST; }
    else if (k == "character") { for (int i = 0; i < kChars; i++) if (lv == kCharKey[i]) c.character = i; if (lv == "???" || lv == "blue_baby") c.character = 4; if (lv == "the_gone") c.character = kGone; }
    else if (k == "curse" || k == "curses") {
        c.dark = lv.find("darkness") != std::string::npos; c.maze = lv.find("labyrinth") != std::string::npos; c.lost = lv.find("lost") != std::string::npos;
        c.blind = lv.find("blind") != std::string::npos;
    }
    else if (k == "treasure_rooms" || k == "treasure") c.treasure = lv == "fewer" ? T_FEWER : lv == "none" ? T_NONE : T_NORMAL;
    else if (k == "bad_luck") c.badLuck = atoi(v.c_str()) != 0;
    else if (k == "hard_mode") c.hard = atoi(v.c_str()) != 0;
    else if (k == "complete") c.complete = atoi(v.c_str()) != 0;
    else if (k == "trinket") { int t = atoi(v.c_str()); c.trinket = t >= 1 && t <= 34 ? t : 0; }
    else if (k == "items") {
        c.items.clear();
        for (size_t q = 0; q < v.size();) {
            while (q < v.size() && !isdigit((unsigned char)v[q])) q++;
            if (q >= v.size()) break;
            int id = atoi(v.c_str() + q);
            if (((id >= 1 && id <= 198) || (id >= 230 && id <= 232)) && c.items.size() < 20) c.items.push_back(id);   // (230-232: The Gone's)
            while (q < v.size() && isdigit((unsigned char)v[q])) q++;
        }
    }
    else if (k == "speed") c.speed = v.empty() ? NAN : std::max(-0.9, std::min(2.0, numOr(v, NAN)));
    else if (k == "tears") c.tears = v.empty() ? NAN : std::max(-2.5, std::min(15.0, numOr(v, NAN)));
    else if (k == "damage") c.damage = v.empty() ? NAN : std::max(0.1, std::min(20.0, numOr(lv[0] == 'x' ? v.substr(1) : v, NAN)));
    else if (k == "range") c.range = v.empty() ? NAN : std::max(-6.0, std::min(30.0, numOr(v, NAN)));
    else if (k == "shot_speed") c.shot = v.empty() ? NAN : std::max(0.2, std::min(4.0, numOr(lv[0] == 'x' ? v.substr(1) : v, NAN)));
    else if (k == "luck") c.luck = v.empty() ? NAN : std::max(-10.0, std::min(10.0, numOr(v, NAN)));
    else if (k == "hearts") c.hearts = std::min(12, cnt(-1));
    else if (k == "soul_hearts") c.soul = std::min(12, cnt(-1));
    else if (k == "coins") c.coins = cnt(-1);
    else if (k == "bombs") c.bombs = cnt(-1);
    else if (k == "keys") c.keys = cnt(-1);
}
// the run's copy of its challenge (in _root.clabRun): the same keys and values
const std::string kSep = "\x1f";   // (between key=value pairs)
std::string toKV(const Chal& c) {
    std::string s = "name=" + c.name + kSep + "goal=" + kGoalKey[c.goal] + kSep + "character=" + kCharKey[c.character] + kSep + "items=";
    for (size_t i = 0; i < c.items.size(); i++) s += (i ? "," : "") + std::to_string(c.items[i]);
    s += kSep + "curse=" + (c.dark ? "darkness," : "") + (c.maze ? "labyrinth," : "") + (c.lost ? "lost," : "") + (c.blind ? "blind" : "");
    s += kSep + "treasure_rooms=" + (c.treasure == T_FEWER ? "fewer" : c.treasure == T_NONE ? "none" : "normal");
    s += kSep + "bad_luck=" + (c.badLuck ? "1" : "0") + kSep + "hard_mode=" + (c.hard ? "1" : "0") + kSep + "trinket=" + std::to_string(c.trinket);
    for (auto& kv : std::vector<std::pair<const char*, double>>{{"speed", c.speed}, {"tears", c.tears}, {"damage", c.damage}, {"range", c.range}, {"shot_speed", c.shot}, {"luck", c.luck}})
        if (std::isfinite(kv.second)) s += kSep + kv.first + "=" + fmtNum(kv.second);
    for (auto& kv : std::vector<std::pair<const char*, int>>{{"hearts", c.hearts}, {"soul_hearts", c.soul}, {"coins", c.coins}, {"bombs", c.bombs}, {"keys", c.keys}})
        if (kv.second >= 0) s += kSep + kv.first + "=" + std::to_string(kv.second);
    return s;
}
Chal fromKV(const std::string& s) {
    Chal c; size_t p = 0;
    while (p <= s.size()) {
        size_t e = s.find(kSep, p); if (e == std::string::npos) e = s.size();
        std::string f = s.substr(p, e - p); p = e + 1;
        size_t eq = f.find('='); if (eq != std::string::npos) setKV(c, f.substr(0, eq), f.substr(eq + 1));
    }
    return c;
}
// the eight challenges that come with the program (mods/customchallenges.txt, built in): written into the mods folder
// once, when it has no list yet (a list of the player's own is never touched; one deleted after that stays deleted:
// saves/challengelab_bundle.txt marks it as given)
const char* kBundle =
#include "clab_bundle.inc"
    ;
void giveBundle() {
    static bool done = false;
    if (done) return;
    done = true;
    std::string mark = g_modsSaveDir + "challengelab_bundle.txt";
    if (FILE* f = platformFopen(mark, "rb")) { fclose(f); return; }
    if (FILE* f = platformFopen(listPath(), "rb")) fclose(f);
    else if (FILE* f = platformFopen(listPath(), "wb")) { fputs(kBundle, f); fclose(f); logf("challengelab: the eight challenges that come with it written to %s\n", listPath().c_str()); }
    if (FILE* f = platformFopen(mark, "wb")) { fputs("; Challenge Lab: its own challenges were offered once (mods/customchallenges.txt)\n", f); fclose(f); }
}
void loadList() {
    g_list.clear();
    giveBundle();
    std::string text = fileText(listPath());
    Chal* cur = nullptr;
    size_t p = 0;
    while (p < text.size()) {
        size_t e = text.find('\n', p); if (e == std::string::npos) e = text.size();
        std::string line = trim(text.substr(p, e - p)); p = e + 1;
        if (line.compare(0, 3, "\xEF\xBB\xBF") == 0) line = line.substr(3);
        if (line.empty() || line[0] == ';' || line[0] == '#') continue;
        if (lower(line) == "[challenge]") {
            if ((int)g_list.size() >= kMaxChallenges) { cur = nullptr; continue; }
            g_list.push_back(Chal()); cur = &g_list.back(); continue;
        }
        size_t eq = line.find('=');
        if (cur && eq != std::string::npos) setKV(*cur, line.substr(0, eq), line.substr(eq + 1));
    }
    for (auto& c : g_list) if (c.name.empty()) c.name = "Challenge";
}
// finished: "complete = 1" in that challenge's section of the file (its n-th, if the name still matches)
void markDone(int slot, const std::string& name) {
    if (genesisActive()) { logf("challengelab: finished (not saved: GenesisConsole is on)\n"); return; }
    std::string text = fileText(listPath());
    if (text.empty()) return;
    std::vector<std::string> lines; size_t p = 0;
    while (p <= text.size()) { size_t e = text.find('\n', p); if (e == std::string::npos) { if (p < text.size()) lines.push_back(text.substr(p)); break; } lines.push_back(text.substr(p, e - p)); p = e + 1; }
    int sec = -1; int start = -1, end = (int)lines.size(); std::string secName;
    for (int i = 0; i < (int)lines.size(); i++) {
        std::string l = lower(trim(lines[i]));
        if (l == "[challenge]") { if (sec == slot) { end = i; break; } sec++; if (sec == slot) start = i; continue; }
        if (sec == slot) { size_t eq = l.find('='); if (eq != std::string::npos && trim(l.substr(0, eq)) == "name") secName = cleanName(trim(lines[i].substr(lines[i].find('=') + 1))); }
    }
    if (start < 0 || lower(secName) != lower(name)) { logf("challengelab: finished, but customchallenges.txt has changed: not marked\n"); return; }
    bool set = false;
    for (int i = start + 1; i < end; i++) {
        std::string l = lower(trim(lines[i])); size_t eq = l.find('=');
        if (eq != std::string::npos && trim(l.substr(0, eq)) == "complete") { lines[i] = "complete = 1"; set = true; }
    }
    if (!set) { int at = end; while (at > start + 1 && trim(lines[at - 1]).empty()) at--; lines.insert(lines.begin() + at, "complete = 1"); }
    std::string out; for (auto& l : lines) out += l + "\n";
    std::string tmp = listPath() + ".tmp";
    if (FILE* f = platformFopen(tmp, "wb")) {
        bool ok = fwrite(out.data(), 1, out.size(), f) == out.size();
        ok = fclose(f) == 0 && ok;
        if (ok) platformReplaceFile(tmp, listPath());
    }
}

// ============================================================ the run (its challenge in _root.clabRun)
struct Run { bool on = false, given = false; int slot = -1; Chal c; };
Run parseRun() {
    Run r;
    Value v = V(root(), "clabRun");
    if (v.t != T_STR) return r;
    const std::string& s = v.s->s;
    size_t a = s.find('\x1e'), b = a == std::string::npos ? a : s.find('\x1e', a + 1), c = b == std::string::npos ? b : s.find('\x1e', b + 1);
    if (c == std::string::npos || s.compare(0, a, "2") != 0) return r;
    r.on = true; r.given = s.substr(a + 1, b - a - 1) == "1"; r.slot = atoi(s.c_str() + b + 1);
    r.c = fromKV(s.substr(c + 1));
    return r;
}
void storeRun(const Run& r) {
    std::string s = "2\x1e" + std::string(r.given ? "1" : "0") + "\x1e" + std::to_string(r.slot) + "\x1e" + toKV(r.c);
    S(root(), "clabRun", Value(mkstr(s)));
}
const double kRunChala = 11;   // (a challenge, none of the game's ten)
int genChala(const Chal& c) {   // the challenge number whose treasure rules are wanted while a floor is made
    if (c.treasure == T_NONE) return 5;
    if (c.treasure == T_FEWER) return 2;
    return 0;
}
// the run's challenge, cached (parsed again when _root.clabRun changes)
Run s_run; Str* s_runStr = nullptr;
const Run& run() {
    Value v = V(root(), "clabRun");
    Str* cur = v.t == T_STR ? v.s : nullptr;
    if (cur != s_runStr) { s_runStr = cur; s_run = parseRun(); }
    return s_run;
}

// ---- a run apart: the save copied at its start and put back at its end
Object* s_snap = nullptr;      // (a copy of so.data)
bool s_sep = false;
Object* s_lk = nullptr; Object* s_ulk = nullptr;   // the unlock flags, write-guarded during the run
bool s_ownWrite = false;       // (the mod's own changes to them)
bool s_flagsOn = false;        // (all of them on, for the run)
Value deepCopy(const Value& v, std::unordered_map<Object*, Object*>& seen) {
    if (v.t != T_OBJ) return v;
    Object* o = v.o;
    auto it = seen.find(o); if (it != seen.end()) return Value(it->second);
    if (o->kind == OK_ARRAY) {
        Array* a = gcNew<Array>(); seen[o] = a;
        Array* src = (Array*)o;
        a->el.reserve(src->el.size());
        for (auto& e : src->el) a->el.push_back(deepCopy(e, seen));
        a->ord = src->ord; a->customOrd = src->customOrd;
        for (auto& p : o->props.v) a->props.set(p.k, deepCopy(p.v, seen), p.flags);
        return Value((Object*)a);
    }
    if (o->kind == OK_PLAIN) {
        Object* c = gcNew<Object>(o->proto); seen[o] = c;
        for (auto& p : o->props.v) c->props.set(p.k, deepCopy(p.v, seen), p.flags);
        return Value(c);
    }
    return v;
}
void setFlag(const Value& arr, int i) { s_ownWrite = true; setMem(arr, Value((double)i), VTRUE); s_ownWrite = false; }
Value s_hardBefore;   // _root.hardmode before the run apart (a boolean or undefined: nothing to root)
void beginApart(const Chal& c) {
    Value R = root(), data = V(V(R, "so"), "data");
    if (!s_sep) {
        if (!data.isObj()) return;
        std::unordered_map<Object*, Object*> seen;
        Value cp = deepCopy(data, seen);
        s_snap = cp.isObj() ? cp.o : nullptr;
        s_sep = true;
        s_hardBefore = V(data, "hard");   // (Eternal's hard mode: the challenge's own, then the save's again - as the game
                                          //  sets _root.hardmode from so.data.hard when the save is read)
        logf("challengelab: a run apart (nothing it does is kept)\n");
    }
    g_saveLocked = true;
    Value lk = V(R, "locker"), ul = V(R, "ulocker");
    s_lk = lk.isObj() ? lk.o : nullptr; s_ulk = ul.isObj() ? ul.o : nullptr;
    if (s_lk) s_lk->guarded = 1;
    if (s_ulk) s_ulk->guarded = 1;
    // a save limbo: everything as unlocked (every item in the pools, Mom as beaten - the game ends the first run that
    // beats her -, every character on the ring), in ulocker too (a flag on in locker and not in ulocker is a new
    // unlock, and the game would show its card)
    if (lk.isObj() && lk.o->kind == OK_ARRAY && !s_flagsOn) {
        s_flagsOn = true;
        int n = std::max(100, (int)((Array*)lk.o)->el.size());
        for (int i = 1; i < n; i++) { if (!truthy(arrGet(lk, i))) setFlag(lk, i); if (!truthy(arrGet(ul, i))) setFlag(ul, i); }
    }
}
void endApart() {
    if (!s_sep) return;
    s_sep = false;
    Value R = root(), so = V(R, "so");
    if (s_snap && so.isObj()) {
        std::unordered_map<Object*, Object*> seen;
        Value data = deepCopy(Value(s_snap), seen);
        S(so, "data", data);
        // (_root's own names for the same arrays, as the game sets them when the save is read)
        const char* pairs[][2] = {{"locker", "lock"}, {"lockor", "lockor"}, {"ulocker", "ulock"}, {"colss", "cols"}, {"bossk", "boss"}, {"lcomp", "lcomp"}, {"minwin", "mwin"}};
        for (auto& p : pairs) { Value v = V(data, p[1]); if (v.isObj()) S(R, p[0], v); }
    }
    S(R, "hardmode", s_hardBefore); s_hardBefore = VUNDEF;
    s_snap = nullptr; s_flagsOn = false;
    if (s_lk) s_lk->guarded = 0;
    if (s_ulk) s_ulk->guarded = 0;
    s_lk = s_ulk = nullptr;
    g_saveLocked = genesisActive();
    logf("challengelab: the run apart is over: the save is as it was\n");
}

// ---- the save put back before the main menu is built (its pictures read the unlock flags when they appear: Isaac
// beating Mom on the menu's note, the characters): _root.gotoAndStop("menu") is wrapped on _root itself
Function* s_gotoWrap = nullptr;
Value wGoto(Act& ac, Value thisv, int argc, Value* argv) {
    if (argc > 0 && s_sep) {
        Value a0 = argv[0];
        bool menu = (a0.t == T_STR && lower(a0.s->s) == "menu") || (a0.isNum() && a0.n == 5);
        if (menu) { if (V(root(), "clabRun").t == T_STR) S(root(), "clabRun", VUNDEF); endApart(); }
    }
    Value orig = getMemK(Value(protos::movieclip), intern("gotoAndStop"));
    return orig.isObj() ? callValue(ac, orig, thisv, argc, argv) : VUNDEF;
}
void hookRootGoto() {
    if (!s_gotoWrap) { Function* f = gcNew<Function>(); f->proto = protos::function; f->nat = wGoto; f->dbgName = "clabGoto"; gcPin(f); s_gotoWrap = f; }
    Value cur = getMemK(root(), intern("gotoAndStop"));
    if (!(cur.isObj() && cur.o == (Object*)s_gotoWrap)) vm.root->props.set(intern("gotoAndStop"), Value((Object*)s_gotoWrap), PF_DONTENUM);
}

// ---- _root's write guard (chained with GenesisConsole's): curses held while a floor is made, no unlock turning on
WriteGuardFn s_prevGuard = nullptr;
bool s_making = false;   // a floor is being made (from darky(120) to the end of that tick)
Str* kDarks; Str* kDouble; Str* kBig; Str* kWtfuuz; Str* kClassit; Str* kPlaysp;
bool s_playcFirst = false; double s_playcAdd = 0;   // (the speed, see wPlayc)
bool labGuard(Object* o, Str* k, Value& v) {
    if (s_ownWrite) return true;
    if (s_playcFirst && o == (Object*)vm.root && k == kPlaysp) { s_playcFirst = false; v = Value(std::max(0.1, num(v, 1) + s_playcAdd)); }
    if (s_sep && !s_ownWrite && (o == s_lk || o == s_ulk) && truthy(v) && !truthy(getProp(o, k))) return false;
    if (s_making && o == (Object*)vm.root) {
        const Run& r = run();
        if (r.on) {
            double chaps = num(V(root(), "chaps"));
            if (k == kDarks && r.c.dark && chaps != 11) v = VTRUE;
            else if (k == kBig && r.c.lost && chaps < 9) v = VTRUE;
            else if (k == kDouble && r.c.maze && chaps < 8) v = VTRUE;
            else if (k == kWtfuuz) v = VTRUE;   // (the game's "given already": no D6, no items of the ten for this run)
            else if (k == kClassit) v = VFALSE;   // (no character's own items either - Magdalene's Yum Heart, Cain's
                                                  //  Lucky Foot...: only the challenge's. The Gone's mod gives his)
        }
    }
    return s_prevGuard ? s_prevGuard(o, k, v) : true;
}

// ---- making a floor: _root.darky(120) is the first thing the game does (and newstats, at a run's start)
GameHook g_wDarky{"darky"};
Value wDarky(Act& ac, Value thisv, int argc, Value* argv) {
    if (argc > 0 && num(argv[0]) == 120) {
        const Run& r = run();
        if (r.on) { S(root(), "chala", Value((double)genChala(r.c))); s_making = true; vm.root->guarded = 1; }
    }
    return callGameOrig(g_wDarky, ac, thisv, argc, argv);
}

// ---- The Chest as the goal: after Isaac, the chest takes you there as it does with the Polaroid (trinket 47)
GameHook g_wTrixx{"trixx"};
Value wTrixx(Act& ac, Value thisv, int argc, Value* argv) {
    Value r = callGameOrig(g_wTrixx, ac, thisv, argc, argv);
    if (argc > 0 && num(argv[0]) == 47 && !truthy(r)) {
        Value R = root();
        const Run& run1 = run();
        if (run1.on && run1.c.goal == G_CHEST && num(V(R, "chaps")) == 9 && truthy(V(R, "altchap")) && num(V(R, "lev")) == num(V(R, "bossl"))) return VTRUE;
    }
    return r;
}

// ---- base stats (advanced): after the game works them out (they are worked out again all the time)
GameHook g_wDmgdo{"dmgdo"}, g_wFirr{"firr"}, g_wCreate{"create"}, g_wPlayc{"playc"}, g_wPlff{"plff"}, g_wFriends{"friends"};
int s_inPlff = 0;   // inside plff (the player's own shot; the familiars shoot from friends)
std::vector<Value> s_blindTears;   // Curse of the Blind: the player's tears of this image, gone at its end
int s_inFriends = 0;   // inside friends (the familiars: it can be called from plff, as they shoot when he does)
Value wFriends(Act& ac, Value t, int argc, Value* argv) {
    s_inFriends++;
    Value r = callGameOrig(g_wFriends, ac, t, argc, argv);
    s_inFriends--;
    return r;
}
Value wPlff(Act& ac, Value t, int argc, Value* argv) {
    s_inPlff++;
    Value r = callGameOrig(g_wPlff, ac, t, argc, argv);
    s_inPlff--;
    return r;
}
Value wDmgdo(Act& ac, Value t, int argc, Value* argv) {
    Value r = callGameOrig(g_wDmgdo, ac, t, argc, argv);
    const Run& rr = run();
    if (rr.on && std::isfinite(rr.c.damage) && r.isNum() && !std::isfinite(genesisOverride(2))) return Value(r.n * rr.c.damage);
    return r;
}
Value wFirr(Act& ac, Value t, int argc, Value* argv) {
    Value r = callGameOrig(g_wFirr, ac, t, argc, argv);
    const Run& rr = run();
    Value p = V(V(root(), "a"), "player");
    if (rr.on && std::isfinite(rr.c.tears) && !std::isfinite(genesisOverride(1)) && argc > 0 && argv[0].isObj() && p.isObj() && argv[0].o == p.o) {
        double fire = num(V(argv[0], "fire"), 10), tps = 30.0 / (fire + 1) + rr.c.tears;
        double f = std::max(1.0, 30.0 / std::max(0.3, tps) - 1);
        S(argv[0], "fire", Value(f)); S(root(), "firra", Value(f));
    }
    return r;
}
Value wCreate(Act& ac, Value t, int argc, Value* argv) {
    Value r = callGameOrig(g_wCreate, ac, t, argc, argv);
    const Run& rr = run();
    // the player's own tear: made in plff (the familiars' tears have the same type, from friends)
    bool own = false;
    if (rr.on && s_inPlff > 0 && s_inFriends == 0 && argc >= 7 && r.isObj() && std::round(toNum(argv[6])) == 2 && !truthy(V(r, "knife"))) {
        // (plff also makes the shooting familiars' tears, at their own place: his are made right on him - within
        // 8 px, for the eye - while a familiar, even next to him, is 25 px away or more)
        Value p = V(V(root(), "a"), "player");
        double dx = toNum(argv[0]) - num(V(p, "xp")), dy = toNum(argv[1]) - num(V(p, "yp"));
        own = dx * dx + dy * dy < 10 * 10;
    }
    if (own) {
        if (rr.c.blind) {   // Curse of the Blind: he aims (his familiars shoot that way) but his own tears never leave
            // (hidden now, gone after this image: the familiars shoot off the player's own shot - with it gone at
            // once, they didn't shoot at all)
            if (r.o->isDisplay()) ((DisplayObject*)r.o)->visible = false;
            s_blindTears.push_back(r);
            return r;
        }
        if (std::isfinite(rr.c.range) && !std::isfinite(genesisOverride(3))) {   // its height and fall, as the game's "firrr" counts them
            double dm = num(V(r, "dm"), 0), f0 = -dm * 5 - num(V(r, "dy"), 0), f = std::max(5.0, f0 + rr.c.range * 23.0 / 6.5);
            S(r, "dy", Value(-f - dm * 5)); S(root(), "firrr", Value(f));
        }
        if (std::isfinite(rr.c.shot) && !std::isfinite(genesisOverride(4))) { S(r, "xbew", Value(num(V(r, "xbew")) * rr.c.shot)); S(r, "ybew", Value(num(V(r, "ybew")) * rr.c.shot)); }
    }
    return r;
}
// the speed: playc works _root.playsp out (its first write: 1 + the items', then a few more added to it) and moves the
// player with it in the same call, so it is changed as it is written: the first write of each call gets the
// challenge's difference (the root's write guard is on for the call)
Value wPlayc(Act& ac, Value t, int argc, Value* argv) {
    const Run& rr = run();
    bool on = rr.on && std::isfinite(rr.c.speed) && !std::isfinite(genesisOverride(0));
    uint8_t g = vm.root->guarded;
    if (on) { s_playcFirst = true; s_playcAdd = rr.c.speed; vm.root->guarded = 1; }
    Value r = callGameOrig(g_wPlayc, ac, t, argc, argv);
    if (on) { s_playcFirst = false; vm.root->guarded = g; }
    return r;
}
bool wantsStats(const Chal& c) { return std::isfinite(c.speed) || std::isfinite(c.tears) || std::isfinite(c.damage) || std::isfinite(c.range) || std::isfinite(c.shot); }

// ---- the start: the items, the trinket, the hearts, pickups and luck
int s_fill = 0;   // ticks until the heart containers are filled
int s_soulWait = 0; double s_soul = 0;   // and the soul hearts set again (??? gets his after the start: his own then went on top)
const char* kTrinketName[34] = {"Fish Head", "Pinky Eye", "Push Pin", "Liberty Cap", "Umbilical Cord", "Childs Heart", "Curved Horn",
    "Rusted Key", "Goat Hoof", "Moms Pearl", "Cancer", "Red Patch", "Match Stick", "Lucky Toe", "Cursed Skull", "Safety Cap",
    "Ace of Spades", "Isaacs Fork", "The Polaroid", "A Missing Page", "Bloody Penny", "Burnt Penny", "Flat Penny",
    "Counterfeit Penny", "Tick", "Isaacs Head", "Maggys Faith", "Judas' Tongue", "???'s Soul", "Samsons Lock", "Cains Eye",
    "Eves Bird Foot", "The Left Hand", "Game Squid"};   // (the game's own names, as its HUD shows them)
void startRun(Run& r, const Value& a) {
    Value R = root(), p = V(a, "player");
    if (!p.isObj()) return;
    const Chal& c = r.c;
    goneHookItems(a);
    S(a, "gofind", VTRUE);   // (as the game gives its challenges' items)
    for (int it : c.items) {
        if (it >= 230 && !modActive("thegone")) continue;   // (The Gone's items: only with his mod on)
        S(a, "lastcra", Value(-100.0)); S(p, "empty", VFALSE); S(a, "highs", p); S(p, "it", Value((double)it));
        call(a, "powerlevel", {});
    }
    S(a, "gofind", VUNDEF);
    int nTr = game() == "eternal" ? 34 : 33;
    if (c.trinket >= 1 && c.trinket <= nTr) {   // held, as a picked-up one: _root.trix (28 + its number), its name for the HUD
        S(R, "trix", Value(28.0 + c.trinket)); S(R, "trixer", Value(mkstr(kTrinketName[c.trinket - 1])));
    }
    // the luck: newstats gave 0 (-2 when the floor's rules were the 7 Years Bad Luck number)
    double luck = num(V(R, "luck"));
    bool gave2 = genChala(c) == 2;
    if (c.badLuck && !gave2) luck -= 2; else if (!c.badLuck && gave2) luck += 2;
    if (std::isfinite(c.luck)) luck += c.luck;
    S(R, "luck", Value(luck));
    if (c.hearts >= 0 && c.character != kGone) {
        // heart containers: the game works them out every image from the character's own (3,4,2,1,3,2,1) and the
        // items' (ups[22] Lunch...): the difference goes into ups[22], and they are filled once that has happened
        static const int kBase[7] = {3, 4, 2, 1, 3, 2, 1};
        int sk = (int)num(V(R, "skiner"), 0), d = c.hearts - kBase[std::max(0, std::min(6, sk))];
        for (const Value& u : {V(R, "ups"), V(a, "ups")}) if (u.isObj()) setMem(u, Value(22.0), Value(num(arrGet(u, 22)) + d));
        if (V(R, "ups").o == V(a, "ups").o && d) setMem(V(R, "ups"), Value(22.0), Value(num(arrGet(V(R, "ups"), 22)) - d));   // (one array)
        s_fill = 3;
    }
    if (c.soul >= 0 && c.character != kGone) { S(R, "armor", Value((double)c.soul)); s_soul = c.soul; s_soulWait = 4; }
    if (c.coins >= 0) S(R, "coins", Value((double)c.coins));
    if (c.bombs >= 0) S(R, "bombs", Value((double)c.bombs));
    if (c.keys >= 0) S(R, "keys", Value((double)c.keys));
    r.given = true; storeRun(r);
    logf("challengelab: run started (%zu items)\n", c.items.size());
}

void fillTick(const Value& a) {
    if (s_soulWait > 0 && --s_soulWait == 0) S(root(), "armor", Value(s_soul));
    if (s_fill <= 0 || --s_fill > 0) return;
    Value p = V(a, "player");
    double m = num(V(p, "mhp"), 0);
    if (p.isObj() && m > 0) { S(p, "hp", Value(m)); S(root(), "hp", Value(m)); }
}

// ---- after Mom's Heart: what leads on
int s_heartRoom = -1;
void heartRoom(const Run& r, const Value& a) {
    Value R = root();
    if (num(V(R, "chaps")) != 8 || num(V(R, "lev")) != num(V(R, "bossl"))) { s_heartRoom = -1; return; }
    // (the chest is the beam to the Cathedral while _root.heaven is set: Isaac and The Chest; else it ends the run)
    if (num(V(R, "heaven")) != 2) S(R, "heaven", VB(r.c.goal == G_ISAAC || r.c.goal == G_CHEST));
    Value ball = V(a, "ball");
    if (!ball.isObj() || ball.o->kind != OK_ARRAY) return;
    std::vector<Value> chest, trap;
    for (auto& e : ((Array*)ball.o)->el) {
        if (!e.isObj() || num(V(e, "s")) != 5 || truthy(V(e, "done"))) continue;
        int fr = curFrame(V(e, "d"));
        if (fr == 34) chest.push_back(e); else if (fr == 9) trap.push_back(e);
    }
    if (chest.empty() && trap.empty()) return;
    int lev = (int)num(V(R, "lev"));
    if (s_heartRoom == lev) return;
    s_heartRoom = lev;
    auto away = [](const Value& e) { S(e, "done", VTRUE); if (e.o->isDisplay()) ((DisplayObject*)e.o)->visible = false; };
    if (r.c.goal == G_SATAN) {
        for (auto& c : chest) away(c);
        if (trap.empty()) call(a, "create", {Value(320.0), Value(280.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(5.09)});
    } else for (auto& t : trap) away(t);   // (Mom's Heart, Isaac, The Chest: only the chest)
}

// ---- the end
bool s_sawBoss = false; int s_bossRoom = -1; bool s_finished = false;
bool onGoalFloor(const Chal& c) {
    Value R = root();
    double chaps = num(V(R, "chaps"));
    if (chaps != kGoalChaps[c.goal]) return false;
    if (c.goal == G_ISAAC) return truthy(V(R, "altchap"));
    if (c.goal == G_SATAN) return !truthy(V(R, "altchap"));
    return true;
}
void endTick(const Run& r, const Value& a) {
    Value R = root();
    int lev = (int)num(V(R, "lev"), -1);
    if (lev != s_bossRoom) { s_bossRoom = lev; s_sawBoss = false; }
    if (!onGoalFloor(r.c) || lev != (int)num(V(R, "bossl"), -2)) return;
    bool clear = num(V(a, "shutdoor"), 1) == 0 && num(V(a, "ashut"), 1) == 0;
    if (!clear) s_sawBoss = true;
    if (r.c.goal == G_MOM) {
        // Mom ends the run, as the game's challenges 1-4 (after the kill, when her red flash is at its height)
        double md = num(V(a, "momdown"));
        if (md > 0 && md <= 50 && !s_finished) {
            s_finished = true;
            markDone(r.slot, r.c.name);
            call(R, "colorit", {R, Value(1.0), Value(1.0), Value(1.0), Value(0.0), Value(0.0), Value(0.0)});
            S(R, "levz", VUNDEF);
            call(a, "moveon", {});
            S(R, "door", VUNDEF);
            call(R, "gotoAndStop", {Value(9.0)});
            logf("challengelab: finished (Mom)\n");
        }
        return;
    }
    if (s_sawBoss && clear && !s_finished) { s_finished = true; markDone(r.slot, r.c.name); logf("challengelab: finished (%s)\n", kGoalKey[r.c.goal]); }
}

// ============================================================ the names, in the character cards' lettering
// The character select's cards (Isaac, Magdalene...) are written by hand in the same hand as the challenge list; The
// Gone's mod takes their letters apart (mod_thegone.cpp) and Challenge Lab writes its names with them: capitals as
// they are, small letters smaller (as the game's own names: "Dark was the Night"), on one line. A character the
// cards don't have (W Q X Z, digits, signs) is written in the menus' marker letters at the same height.
const double kNameH = 29;   // (capitals' height on the sheet: the game's own names')
DisplayObject* putName(MovieClip* parent, int& pdp, const std::string& text, double x, double y, uint32_t ink, double scale, double* outW) {
    char nm[24]; snprintf(nm, sizeof nm, "__n%d", pdp);
    MovieClip* ui = createEmptyMovieClip(parent, intern(nm), pdp++);
    if (!ui) { if (outW) *outW = 0; return nullptr; }
    ui->m = Mat(); ui->m.a = ui->m.d = scale; ui->m.tx = x; ui->m.ty = y; ui->scriptXform = true;
    int dp = 1;
    bool cards = cardLetters();
    double kBig = cards ? kNameH / cardLetterH() : 1, kSmall = kBig * 0.74, cx = 0, gap = 1.6;
    // runs of the same kind: (cards' capitals | cards' small | marker), each one shape
    size_t i = 0;
    while (i < text.size()) {
        char ch = text[i];
        auto kind = [&](char c) { char u = (char)toupper((unsigned char)c); if (!cards || !cardHasLetter(u)) return 2; return c == ' ' ? 3 : islower((unsigned char)c) ? 1 : 0; };
        int k0 = kind(ch);
        if (k0 == 3) { cx += 7 * kBig; i++; continue; }
        size_t j = i; while (j < text.size() && kind(text[j]) == k0) j++;
        std::string run = text.substr(i, j - i);
        if (k0 == 2) {   // marker letters, capitals' or small letters' height
            std::string up = run; for (auto& c : up) c = (char)toupper((unsigned char)c);
            bool small = islower((unsigned char)run[0]);
            float h = (float)(small ? kNameH * 0.74 : kNameH * 0.95), w = 0;
            synthPut(ui, synthHandText(up, h, ink, &w), dp++, cx, kNameH - h);
            cx += w + gap;
        } else {
            std::string up = run; for (auto& c : up) c = (char)toupper((unsigned char)c);
            double k = k0 == 1 ? kSmall : kBig, h = cardLetterH() * k;
            synthPut(ui, cardTextShape(up, k, cx, kNameH - h / 2), dp++, 0, 0);   // (letters centred on cy: on the capitals' baseline)
            cx += cardTextW(up, k) + gap;
        }
        i = j;
    }
    if (outW) *outW = std::max(0.0, cx - gap) * scale;
    return ui;
}

// ============================================================ the menu: the list
MovieClip* g_ui = nullptr;        // our clip on the challenge sheet
Object* g_uiSheet = nullptr;
bool s_lab = false;               // the sheet shows the Challenge Lab page
struct Hit { RectF r; int what; DisplayObject* d; };   // what: -1 to the lab, -2 back, 0.. a challenge
std::vector<Hit> g_hits;
int s_pressed = -100;
std::string s_built;
uint16_t s_arrowId = 0; Mat s_arrowM;   // the collection's arrow (its picture)

Value sheetV() { return V(root(), "pauss"); }
MovieClip* challengeSheet() {   // the main menu's challenge list, on screen
    if (vm.root->kind != OK_CLIP || ((MovieClip*)vm.root)->cur != 5) return nullptr;
    Value p = sheetV();
    if (!p.isObj() || p.o->kind != OK_CLIP) return nullptr;
    MovieClip* c = (MovieClip*)p.o;
    if (c->removed || c->cur != 9) return nullptr;
    for (DisplayObject* q = c; q; q = q->parent) if (!q->visible) return nullptr;
    if (!c->childByName(intern("c1"))) return nullptr;
    return c;
}
void findArrow() {   // the collection's "next page" arrow: a button of men.men.col that isn't an item slot
    if (s_arrowId) return;
    Value col = V(V(V(root(), "men"), "men"), "col");
    if (!col.isObj() || col.o->kind != OK_CLIP) return;
    for (DisplayObject* k : ((MovieClip*)col.o)->kids) {
        if (k->kind != OK_BUTTON || (k->name && k->name->s.size() > 1 && k->name->s[0] == 'i')) continue;
        ButtonObj* b = (ButtonObj*)k;
        for (auto& rec : b->def->records)
            if (rec.states & 1) { s_arrowId = rec.id; s_arrowM = rec.m; return; }
    }
}
void removeUi() {
    if (g_ui && !g_ui->removed && g_ui->parent) removeObject(g_ui);
    g_ui = nullptr; g_uiSheet = nullptr; g_hits.clear(); s_built.clear();
    menuSetExtras("challengelab", {});
}
void showGameList(MovieClip* sheet, bool show) {
    for (int i = 1; i <= 10; i++) if (DisplayObject* c = sheet->childByName(intern(("c" + std::to_string(i)).c_str()))) c->visible = show;
}
// sheet coordinates (WotL / Eternal frame 9): the rows c1..c10 at x 71 (their numbers, drawn on the sheet itself, at
// ~34-70), the title "To Do List" at ~124-304 x -126..-94, Back at the bottom right
const double kRowX = 71, kCapH = 27;
const double kRowY[10] = {-87.5, -47, -6, 25.4, 69.7, 103.8, 145.6, 186.8, 229.7, 267.7};
const uint32_t kInk = 0xff878484u, kInkSoft = 0xffa1a1a1u, kPaper = 0xffe0d9dau;
DisplayObject* arrow(MovieClip* ui, int& dp, double x, double y, bool left) {
    if (!s_arrowId) return nullptr;
    DisplayObject* d = synthPut(ui, s_arrowId, dp++, x, y);
    if (!d) return nullptr;
    Mat m = s_arrowM; m.tx += x; m.ty += y;
    if (!left) { m.a = -m.a; m.c = -m.c; }   // (the picture points left: the collection's button turns it)
    d->setMatrix(m);
    return d;
}
// stage rectangle -> the sheet's own coordinates
RectF toSheet(MovieClip* sheet, const RectF& r) {
    Mat inv; if (!worldMatrix(sheet).invert(inv)) return RectF();
    double ax, ay, bx, by; inv.apply(r.x0, r.y0, ax, ay); inv.apply(r.x1, r.y1, bx, by);
    return {std::min(ax, bx), std::min(ay, by), std::max(ax, bx), std::max(ay, by)};
}
ButtonObj* backButton(MovieClip* sheet) {   // the sheet's Back (it answers to Escape)
    for (DisplayObject* k : sheet->kids)
        if (k->kind == OK_BUTTON && ((ButtonObj*)k)->def)
            for (auto& c : ((ButtonObj*)k)->def->conds) if ((c.cond >> 9) == 19) return (ButtonObj*)k;
    return nullptr;
}
Object* s_shiftedBack = nullptr;
// the arrow to and from the Challenge Lab: right of Back, inside the paper (Back moves a little to the left for it)
DisplayObject* pageArrow(MovieClip* sheet, int& dp, bool left) {
    ButtonObj* back = backButton(sheet);
    double ax = 380, ay = 300;
    if (back) {
        if (s_shiftedBack != (Object*)back) { s_shiftedBack = back; back->m.tx -= 30; back->scriptXform = true; }
        RectF br = toSheet(sheet, hitStageBounds(back));
        if (br.valid()) { ax = br.x1 + 6; ay = (br.y0 + br.y1) / 2; }
    }
    DisplayObject* d = arrow(g_ui, dp, 0, 0, left);
    if (!d) return nullptr;
    Mat m = d->m; m.a *= 0.85; m.b *= 0.85; m.c *= 0.85; m.d *= 0.85; d->setMatrix(m);
    RectF r = toSheet(sheet, stageBounds(d));
    if (r.valid()) { Mat m2 = d->m; m2.tx += ax - r.x0; m2.ty += ay - (r.y0 + r.y1) / 2; d->setMatrix(m2); }
    return d;
}
// paper over something of the sheet's (its title, its unused numbers): soft edges, so the paper's own shading shows less
void cover(MovieClip* ui, int& dp, double x, double y, double w, double h) {
    for (int i = 3; i >= 1; i--) synthPut(ui, synthRect((float)(w + 6 * i), (float)(h + 6 * i), (kPaper & 0x00ffffffu) | 0x40000000u), dp++, x - 3 * i, y - 3 * i);
    synthPut(ui, synthRect((float)w, (float)h, kPaper), dp++, x, y);
}
void buildUi(MovieClip* sheet) {
    loadList();
    std::string sig = s_lab ? "lab" : "std";
    if (s_lab) for (auto& c : g_list) sig += "|" + c.name + (c.complete ? "+" : "");
    if (g_ui && !g_ui->removed && g_ui->parent == sheet && sig == s_built) return;
    if (g_ui && !g_ui->removed && g_ui->parent) removeObject(g_ui);
    g_ui = createEmptyMovieClip(sheet, intern("__clab"), DEPTH_BIAS + 9100);
    g_uiSheet = sheet; s_built = sig; g_hits.clear();
    if (!g_ui) return;
    int dp = 1;
    findArrow();
    if (!s_lab) {   // the game's list: the arrow to ours
        showGameList(sheet, true);
        if (DisplayObject* d = pageArrow(sheet, dp, false)) g_hits.push_back({{}, -1, d});
        return;
    }
    showGameList(sheet, false);
    // the title: "Challenge Lab" over "To Do List", in the list's lettering
    cover(g_ui, dp, 100, -133, 236, 44);
    double tw = 0;
    if (DisplayObject* t = putName(g_ui, dp, "Challenge Lab", 0, -128, kInk, 1.2, &tw)) { Mat m = t->m; m.tx = 214 - tw / 2; t->setMatrix(m); t->scriptXform = true; }
    const int n = (int)g_list.size();
    // the sheet's numbers past the list: covered
    if (n < 10) cover(g_ui, dp, 29, kRowY[n] - 3, 44, kRowY[9] + 42 - (kRowY[n] - 3));
    if (n == 0) {
        double w = 0;
        putName(g_ui, dp, "No challenges yet!", kRowX, kRowY[0], kInk, 1, &w);
        synthPut(g_ui, synthHandText("Make them with", 18, kInkSoft, nullptr), dp++, kRowX, kRowY[2]);
        synthPut(g_ui, synthHandText("tools/challenge_lab_maker.html", 13, kInkSoft, nullptr), dp++, kRowX, kRowY[3]);
    }
    for (int i = 0; i < n; i++) {
        const Chal& c = g_list[i];
        double w = 0;
        DisplayObject* d = putName(g_ui, dp, c.name, kRowX, kRowY[i] + 2, kInk, 1, &w);
        if (c.complete) synthPut(g_ui, synthRect((float)w + 8, 3, 0xff7b7878u), dp++, kRowX - 4, kRowY[i] + kCapH * 0.55);   // finished: crossed out
        g_hits.push_back({{}, i, d});
    }
    if (DisplayObject* d = pageArrow(sheet, dp, true)) g_hits.push_back({{}, -2, d});
}
void updateHits() {   // their places on the stage (for the mouse and the controller)
    std::vector<std::pair<DisplayObject*, RectF>> ex;
    double pad = g_stageH / 160.0;
    for (auto& h : g_hits) {
        h.r = h.d ? stageBounds(h.d) : RectF();
        if (h.r.valid()) { h.r.x0 -= pad; h.r.y0 -= pad; h.r.x1 += pad; h.r.y1 += pad; }
        if (h.r.valid() && h.what > -100) ex.push_back({h.d, h.r});
    }
    menuSetExtras("challengelab", ex);
}
// (menuui.cpp: the controller's left / right on the challenge list) the arrow: right on the game's page, left on ours
DisplayObject* labNav(bool right) {
    if (!modActive("challengelab") || !g_ui) return nullptr;
    for (auto& h : g_hits) if (h.d && h.r.valid() && ((!s_lab && right && h.what == -1) || (s_lab && !right && h.what == -2))) return h.d;
    return nullptr;
}
int hitAt(double x, double y) {
    for (auto& h : g_hits) if (h.r.valid() && x >= h.r.x0 && x <= h.r.x1 && y >= h.r.y0 && y <= h.r.y1) return h.what;
    return -1000;
}
// the character: the character select's ring writes _root.skiner every image from its own slot (and its clip starts
// over, at Isaac, when the menu moves on): its slots are Isaac, Magdalene, Cain, Judas, Eve, ???, Samson (??? only once
// unlocked). Kept until the run starts.
void setCharacter(int ch) {
    if (ch >= kGone) ch = 0;   // (a mod's character: Isaac, the mod makes him what he is)
    Value R = root(), cha = V(V(V(R, "men"), "men"), "cha");
    bool seven = truthy(arrGet(V(R, "locker"), 41));
    int ring = ch;
    if (ch == 5) ring = 4;
    else if (ch == 4) ring = 5;
    else if (ch == 6) ring = seven ? 6 : 5;
    if (cha.isObj()) S(cha, "skiner", Value((double)ring));
    S(R, "skiner", Value((double)ch));
}
uint32_t s_resumes = 0;   // Continuum's resumes seen (see challengelabTick)
void startChallenge(int i) {
    if (i < 0 || i >= (int)g_list.size()) return;
    Chal c = g_list[i];
    if (c.character == kGone && !modActive("thegone")) { logf("challengelab: The Gone's mod is off: Isaac\n"); c.character = 0; }
    Value R = root();
    Run r; r.on = true; r.slot = i; r.c = c;
    storeRun(r);
    beginApart(c);
    if (game() == "eternal") S(R, "hardmode", VB(c.hard));   // (the character select's switch; the HUD shows its mark)
    setCharacter(c.character);
    S(R, "chala", Value((double)genChala(c)));
    call(R, "darky", {Value(150.0)});
    s_finished = false; s_heartRoom = -1;
    s_resumes = continuumResumes();
    logf("challengelab: \"%s\" (%s, %s)\n", c.name.c_str(), kCharKey[c.character], kGoalKey[c.goal]);
}
bool onMouse(double x, double y, int type) {   // (display.cpp, logic thread)
    if (!modActive("challengelab") || !g_ui || type == 0 || !challengeSheet()) return false;
    int w = hitAt(x, y);
    if (type == 1) { s_pressed = w; return w > -1000; }
    int p = s_pressed; s_pressed = -100;
    if (w <= -1000 || w != p) return w > -1000;
    if (w == -1) s_lab = true;
    else if (w == -2) s_lab = false;
    else if (w >= 0) startChallenge(w);
    return true;
}
bool onHand(double x, double y) {   // (menuui.cpp: the hand cursor over our arrows and rows, as over the game's buttons)
    return modActive("challengelab") && g_ui && challengeSheet() && hitAt(x, y) > -1000;
}
}  // namespace

void challengelabDebug(const std::string& cmd) {
    MovieClip* sh = challengeSheet();
    Value p = sheetV();
    logf("[clab] sheet %p (pauss %d frame %d) arrow %d ui %p lab %d hits %zu list %zu\n", (void*)sh, (int)p.isObj(), curFrame(p), s_arrowId, (void*)g_ui, (int)s_lab, g_hits.size(), g_list.size());
    for (auto& h : g_hits) logf("[clab]   %d: %.0f,%.0f-%.0f,%.0f\n", h.what, h.r.x0, h.r.y0, h.r.x1, h.r.y1);
    if (sh) { Mat w = worldMatrix(sh); logf("[clab] sheet matrix %.3f %.3f %.3f %.3f %.1f %.1f\n", w.a, w.b, w.c, w.d, w.tx, w.ty);
        for (int i = 1; i <= 10; i++) if (DisplayObject* c = sh->childByName(intern(("c" + std::to_string(i)).c_str()))) { RectF r = stageBounds(c); logf("[clab]   c%d at %.1f,%.1f: %.0f,%.0f-%.0f,%.0f\n", i, c->m.tx, c->m.ty, r.x0, r.y0, r.x1, r.y1); } }
    if (cmd == "lab") s_lab = true;
    if (cmd == "std") s_lab = false;
    if (cmd.compare(0, 6, "start ") == 0) { loadList(); startChallenge(atoi(cmd.c_str() + 6)); }
    if (cmd == "done") { const Run& r = run(); if (r.on) markDone(r.slot, r.c.name); }
}
// (other mods: The Gone, BaseTime) a Challenge Lab run: its character (7 The Gone), -1 when there is none
int clabCharacter() { const Run& r = run(); return r.on ? r.c.character : -1; }
bool clabRunActive() { return run().on; }
// (Marksman) a Challenge Lab run's goal: 0 Mom, 1 Mom's Heart, 2 Isaac, 3 Satan, 4 The Chest; -1 when there is none
int clabGoal() { const Run& r = run(); return r.on ? r.c.goal : -1; }

void challengelabTick(const ModInfo&) {
    static bool rooted = false;
    if (!rooted) {
        rooted = true; gcAddRoot((GcObj**)&g_ui); gcAddRoot((GcObj**)&g_uiSheet); gcAddRoot((GcObj**)&s_snap); gcAddRoot((GcObj**)&s_shiftedBack);
        gcAddRoot((GcObj**)&s_lk); gcAddRoot((GcObj**)&s_ulk); gcAddRoot((GcObj**)&s_runStr);
        kDarks = intern("darks"); kDouble = intern("double"); kBig = intern("big"); kWtfuuz = intern("wtfuuz"); kClassit = intern("classit"); kPlaysp = intern("playsp");
    }
    for (const Value& t : s_blindTears) { S(t, "done", VTRUE); if (t.o->isDisplay()) ((DisplayObject*)t.o)->visible = false; }
    s_blindTears.clear();
    g_modMouseHook = onMouse;
    g_modHandHook = onHand;
    g_challengeNavHook = labNav;
    if (g_writeGuard != labGuard) { s_prevGuard = g_writeGuard; g_writeGuard = labGuard; }
    s_making = false;   // (a floor is made in one tick)
    if (!genesisActive()) vm.root->guarded = 0;   // (GenesisConsole looks after it while it is on)
    Value R = root();
    int frame = vm.root->kind == OK_CLIP ? ((MovieClip*)vm.root)->cur : 0;
    static int lastFrame = 0;
    bool toMenu = frame == 5 && lastFrame != 5;
    lastFrame = frame;
    hookGameFn(g_wDarky, R, wDarky);
    hookRootGoto();
    // ---- the menu
    if (toMenu) { if (V(R, "clabRun").t == T_STR) S(R, "clabRun", VUNDEF); s_lab = false; }   // (a run's variable: not made before one)
    MovieClip* sheet = game() == "og" ? nullptr : challengeSheet();
    if (!sheet) {
        if (g_ui) { if (g_uiSheet && g_uiSheet != (Object*)sheet && !((MovieClip*)g_uiSheet)->removed) showGameList((MovieClip*)g_uiSheet, true); removeUi(); }
        s_lab = false;
    } else {
        buildUi(sheet);
        updateHits();
    }
    // ---- a run
    Run r = run();
    if (!r.on) { endApart(); return; }
    beginApart(r.c);
    // (the run is about to start: the menu fades out. Also a run Continuum brings back: the game takes the character
    // from the menu as it starts, and a challenge's start puts Isaac there)
    if (frame == 5) setCharacter(r.c.character);
    S(R, "chala", Value(kRunChala));
    Value a = V(R, "a");
    if (frame != 2 || !a.isObj()) return;
    hookGameFn(g_wTrixx, a, wTrixx);
    if (r.c.blind || std::isfinite(r.c.range) || std::isfinite(r.c.shot)) { hookGameFn(g_wCreate, a, wCreate); hookGameFn(g_wPlff, a, wPlff); hookGameFn(g_wFriends, a, wFriends); }
    if (wantsStats(r.c)) {
        if (std::isfinite(r.c.damage)) hookGameFn(g_wDmgdo, a, wDmgdo);
        if (std::isfinite(r.c.tears)) hookGameFn(g_wFirr, a, wFirr);
        if (std::isfinite(r.c.range) || std::isfinite(r.c.shot)) hookGameFn(g_wCreate, a, wCreate);
        if (std::isfinite(r.c.speed)) hookGameFn(g_wPlayc, a, wPlayc);
    }
    static uint32_t serial = 0; bool restarted = modsRunSerial() != serial; serial = modsRunSerial();
    // (R held: the run again. Not when Continuum has just brought the run back: its start goes through newstart
    // too, but the items, hearts and pickups given at the start are already in the saved variables)
    if (restarted && r.given && continuumResumes() != s_resumes) { restarted = false; s_resumes = continuumResumes(); }
    if (restarted && r.given) { r.given = false; storeRun(r); s_finished = false; }
    bool inGame = false, play = recompInGameplay(&inGame);
    if (play && !r.given) startRun(r, a);
    fillTick(a);
    heartRoom(r, a);
    endTick(r, a);
}
}  // namespace fl
