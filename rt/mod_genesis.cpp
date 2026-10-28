// GenesisConsole (mods/GenesisConsole.mod): a console inside the game, in the spirit of Rebirth's debug console.
//
// While the mod is on, nothing is saved and nothing can be unlocked: SharedObject.flush() writes nothing
// (builtins.cpp, g_saveLocked) and _root.locker - the game's unlock flags, which also trigger the achievements - is
// write-guarded so no flag can turn on (core.cpp, g_writeGuard). The save is still read, so what was unlocked before
// stays available.
//
// The console: a key (` by default), the controller's Back / Minus button or a three-finger tap opens it during a
// run; the game is paused while it is open (unless pause = 0). The main thread only forwards input (genesisKey /
// genesisText); editing, history, completion and the commands run here on the logic thread, which owns the game
// state, and the console is drawn in the game's display list (_root.__genesis) with the HUD's font, like StatOn.
//
// The commands use the game's own functions: create() spawns (pickups 5.0F + col * 1e-9, pedestals 5.1 + item /
// 10000, enemies by type), powerlevel() gives an item as a character's starting items are given (golev), hurt()
// kills, teller() + tar teleports, newstart() changes floor. Stats the game recomputes all the time are held by
// wrapping its functions on _root.a (dmgdo: damage, firr: tears, create: range and shot speed of the player's tears,
// playerhurt: god mode), like the analog-movement wrapper of dirkey (patches.cpp), and _root.playsp (speed) with
// the write guard. See NOTES.md "v1.5: GenesisConsole" and tools/genesis_wiki.html.
#include "mods.h"
#include "core.h"
#include "display.h"
#include "swfdata.h"
#include "platform.h"
#include <algorithm>
#include <atomic>
#include <cctype>
#include <cstdarg>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <sstream>

namespace fl {
bool recompInGameplay(bool* inGame);
extern bool g_saveLocked;
std::string g_modsSaveDir;

namespace {
// ============================================================ shared with the main thread
std::atomic<bool> s_active{false}, s_open{false}, s_pause{true};
struct ConEv { int kind; int code; std::string text; };   // kind 0: ConKey, 1: text, 2: debug line
std::mutex s_mu;
std::vector<ConEv> s_events;
std::atomic<double> s_ovr[6];   // stat overrides (NAN: none), read by StatOn on the same thread, atomics anyway

// ============================================================ small helpers (logic thread)
Value root() { return Value((Object*)vm.root); }
Value V(const Value& o, const char* k) { return o.isObj() ? getMemK(o, intern(k)) : VUNDEF; }
void S(const Value& o, const char* k, const Value& v) { if (o.isObj()) setMemK(o, intern(k), v); }
double num(const Value& v, double def = 0) { double d = toNum(v); return std::isfinite(d) ? d : def; }
std::string game() { return g_game ? g_game->tag : "wotl"; }
Value call(const Value& obj, const char* fn, std::initializer_list<Value> args) {
    std::vector<Value> av(args);
    Act ac{vm.globalScope, VUNDEF, vm.root, nullptr};
    Value r = callMethodK(ac, obj, intern(fn), (int)av.size(), av.data());
    runActions();
    return r;
}
std::string lower(std::string s) { for (auto& c : s) c = (char)tolower((unsigned char)c); return s; }
std::string norm(const std::string& s) { std::string o; for (char c : s) if (isalnum((unsigned char)c)) o += (char)tolower((unsigned char)c); return o; }
std::string trim(const std::string& s) { size_t a = s.find_first_not_of(" \t"), b = s.find_last_not_of(" \t"); return a == std::string::npos ? "" : s.substr(a, b - a + 1); }
bool isNumber(const std::string& s, double* out = nullptr) {
    if (s.empty()) return false;
    char* end = nullptr; double d = strtod(s.c_str(), &end);
    if (!end || *end || !std::isfinite(d)) return false;
    if (out) *out = d;
    return true;
}
std::string fmt(const char* f, ...) __attribute__((format(printf, 1, 2)));
std::string fmt(const char* f, ...) { char b[512]; va_list ap; va_start(ap, f); vsnprintf(b, sizeof b, f, ap); va_end(ap); return b; }
std::string fnum(double v) { std::string s = fmt("%.2f", v); while (s.size() > 1 && s.back() == '0') s.pop_back(); if (s.back() == '.') s.pop_back(); return s; }

// ============================================================ the log
enum { C_TEXT, C_ECHO, C_OK, C_ERR, C_DIM, C_HEAD };
const uint32_t kColors[] = {0xffe8e2d6u, 0xfff0c674u, 0xff8fe08fu, 0xffff7b6bu, 0xff9a948au, 0xfff4f0e8u};
struct LogLine { std::string text; int color; };
std::vector<LogLine> g_log;
int g_logVersion = 0;
void out(const std::string& t, int color = C_TEXT) {
    size_t p = 0;
    do { size_t e = t.find('\n', p); g_log.push_back({t.substr(p, e == std::string::npos ? std::string::npos : e - p), color}); p = e == std::string::npos ? e : e + 1; } while (p != std::string::npos);
    if (g_log.size() > 400) g_log.erase(g_log.begin(), g_log.begin() + (g_log.size() - 400));
    g_logVersion++;
    static const bool mirror = getenv("ISAAC_GENESIS_LOG") != nullptr;   // (tests: the console's output in log.txt)
    if (mirror) logf("[genesis] %s\n", t.c_str());
}
void ok(const std::string& t) { out(t, C_OK); }
void err(const std::string& t) { out(t, C_ERR); }

// ============================================================ names
struct Named { int id; std::string name; };
// the item names of this version (_root.st1: the names the game shows when an item is picked up)
std::vector<Named> itemNames() {
    std::vector<Named> v;
    Value st1 = V(root(), "st1");
    if (!st1.isObj() || st1.o->kind != OK_ARRAY) return v;
    Array* a = (Array*)st1.o;
    for (size_t i = 1; i < a->el.size(); i++) if (a->el[i].t == T_STR) v.push_back({(int)i, trim(a->el[i].s->s)});
    return v;
}
// trinkets (Wrath of the Lamb, Eternal): pickup col 29 + index, names from the game's script
const char* kTrinkets[] = {"Fish Head", "Pinky Eye", "Push Pin", "Liberty Cap", "Umbilical Cord", "Childs Heart", "Curved Horn",
    "Rusted Key", "Goat Hoof", "Moms Pearl", "Cancer", "Red Patch", "Match Stick", "Lucky Toe", "Cursed Skull", "Safety Cap",
    "Ace of Spades", "Isaacs Fork", "The Polaroid", "A Missing Page", "Bloody Penny", "Burnt Penny", "Flat Penny",
    "Counterfeit Penny", "Tick", "Isaacs Head", "Maggys Faith", "Judas' Tongue", "?\?\?'s Soul", "Samsons Lock", "Cains Eye",
    "Eves Bird Foot", "The Left Hand", "Game Squid"};
std::vector<Named> trinketNames() {
    std::vector<Named> v;
    if (game() == "og") return v;
    int n = game() == "eternal" ? 34 : 33;
    for (int i = 0; i < n; i++) v.push_back({i + 1, kTrinkets[i]});
    return v;
}
// cards: pickup col 7-28 (tarot) and 70-74 (Wrath of the Lamb, Eternal), names from the game's tart()
const char* kTarot[] = {"X Wheel of Fortune", "XV The Devil", "XIII Death", "XI Strength", "0 The Fool", "I The Magician",
    "IX The Hermit", "XVIII The Moon", "XII The Hanged Man", "XX Judgement", "VII The Chariot", "VI The Lovers", "XXI The World",
    "VIII Justice", "XIV Temperance", "IV The Emperor", "XVI The Tower", "V The Hierophant", "II The High Priestess",
    "III The Empress", "XIX The Sun", "XVII The Stars"};
const char* kPlaying[] = {"2 of Spades", "2 of Clubs", "2 of Hearts", "2 of Diamonds", "The Joker"};
std::vector<Named> cardNames() {
    std::vector<Named> v;
    for (int i = 0; i < 22; i++) v.push_back({7 + i, kTarot[i]});
    if (game() != "og") for (int i = 0; i < 5; i++) v.push_back({70 + i, kPlaying[i]});
    return v;
}
// pickups: create() type 5.0F + col * 1e-9 (F: the pickup's frame)
struct Pickup { const char* name; int frame, col; bool notOg; const char* what; };
const Pickup kPickups[] = {
    {"heart", 1, 1, false, "a red heart"}, {"halfheart", 1, 2, false, "half a red heart"}, {"soulheart", 1, 3, false, "a soul heart"},
    {"eternalheart", 1, 4, true, "an eternal heart"},
    {"penny", 2, 1, false, "a penny"}, {"coin", 2, 1, false, "a penny"}, {"nickel", 2, 2, false, "a nickel (5)"}, {"dime", 2, 3, true, "a dime (10)"},
    {"key", 3, 1, false, "a key"}, {"goldenkey", 3, 2, true, "a golden key"},
    {"bomb", 4, 1, false, "a bomb"}, {"doublebomb", 4, 2, false, "a double bomb"}, {"trollbomb", 4, 3, true, "a troll bomb"},
    {"chest", 5, 0, false, "a chest"}, {"lockedchest", 6, 0, false, "a locked chest (needs a key)"},
    {"slotmachine", 8, 0, false, "a slot machine"},
};
// enemies and bosses: create() type (the numbers are the game's, kept by Rebirth for the same monsters)
struct Enemy { int type; const char* name; bool boss; bool notOg; };
#include "mod_genesis_enemies.inc"

// lookup: an id, the exact name (letters and digits only, any case), a unique beginning, or a unique part
int findNamed(const std::vector<Named>& v, const std::string& q, std::vector<const Named*>& many) {
    many.clear();
    double d;
    if (isNumber(q, &d)) { for (auto& n : v) if (n.id == (int)d && d == (int)d) return n.id; return -1; }
    std::string k = norm(q);
    if (k.empty()) return -1;
    for (auto& n : v) if (norm(n.name) == k) return n.id;
    for (auto& n : v) if (norm(n.name).compare(0, k.size(), k) == 0) many.push_back(&n);
    if (many.size() == 1) return many[0]->id;
    if (many.empty()) for (auto& n : v) if (norm(n.name).find(k) != std::string::npos) many.push_back(&n);
    if (many.size() == 1) return many[0]->id;
    return -1;
}
std::vector<Named> enemyNames(bool bosses) {
    std::vector<Named> v;
    for (auto& e : kEnemies) if (e.boss == bosses && !(e.notOg && game() == "og")) v.push_back({e.type, e.name});
    return v;
}
const Enemy* enemyByType(int t) { for (auto& e : kEnemies) if (e.type == t) return &e; return nullptr; }
std::string lookupError(const char* what, const std::string& q, const std::vector<const Named*>& many) {
    if (many.empty()) return fmt("No %s called \"%s\" (try: find %s)", what, q.c_str(), q.c_str());
    std::string s = fmt("\"%s\" could be:", q.c_str());
    for (size_t i = 0; i < many.size() && i < 6; i++) s += fmt(" %s (%d)%s", many[i]->name.c_str(), many[i]->id, i + 1 < std::min<size_t>(many.size(), 6) ? "," : "");
    if (many.size() > 6) s += fmt(" and %d more", (int)many.size() - 6);
    return s;
}

// ============================================================ game state helpers
bool inRun() { bool ig = false; recompInGameplay(&ig); return ig; }
Value gameA() { return V(root(), "a"); }
Value player() { return V(gameA(), "player"); }
// where to put the n-th of `count` spawned things: around the player, inside the room
void spawnPos(int i, int count, bool nearPlayer, double& x, double& y) {
    Value p = player();
    double px = num(V(p, "xp"), 320), py = num(V(p, "yp"), 280);
    double r = nearPlayer ? 70 : 130, ang = (count > 1 ? i * 6.2831853 / count : 0) + (px > 320 ? 3.14159265 : 0);
    if (!nearPlayer && count == 1) ang = std::atan2(280 - py, 320 - px);
    x = px + r * std::cos(ang); y = py + r * std::sin(ang);
    x = std::max(90.0, std::min(550.0, x)); y = std::max(170.0, std::min(400.0, y));
}
Value create(double x, double y, double type) { return call(gameA(), "create", {Value(x), Value(y), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(type)}); }

// ============================================================ toggles and stat overrides
bool g_god = false, g_doors = false;
double g_gameFirrr = NAN;
const char* kStatNames[6] = {"speed", "tears", "damage", "range", "shotspeed", "luck"};
double ovr(int i) { return s_ovr[i].load(); }
bool anyCreateOvr() { return std::isfinite(ovr(3)) || std::isfinite(ovr(4)); }

// ---- wrappers of the game's functions on _root.a
enum { H_DMGDO, H_FIRR, H_CREATE, H_PLAYERHURT, H_COUNT };
const char* kHookName[H_COUNT] = {"dmgdo", "firr", "create", "playerhurt"};
Function* g_hookWrap[H_COUNT] = {};
Function* g_hookOrig[H_COUNT] = {};
Object* g_hookA[H_COUNT] = {};   // the clip the original came from (a new room = a new _root.a)
Value callOrig(int h, Act& ac, Value thisv, int argc, Value* argv) {
    return g_hookOrig[h] ? callValue(ac, Value((Object*)g_hookOrig[h]), thisv, argc, argv) : VUNDEF;
}
double fireDelay(double tearsPerSecond) { return std::max(1.0, 30.0 / tearsPerSecond - 1); }
// the game's own shot-speed multiplier (StatOn's formula) without the override
double gameShotSpeed() {
    double o = s_ovr[4].exchange(NAN), st[6]; currentStats(st, false); s_ovr[4] = o;
    return std::isfinite(st[4]) && st[4] > 0 ? st[4] : 1;
}
Value wDmgdo(Act& ac, Value t, int argc, Value* argv) {
    Value r = callOrig(H_DMGDO, ac, t, argc, argv);
    return std::isfinite(ovr(2)) ? Value(ovr(2)) : r;
}
Value wFirr(Act& ac, Value t, int argc, Value* argv) {
    Value r = callOrig(H_FIRR, ac, t, argc, argv);
    if (std::isfinite(ovr(1)) && argc > 0 && argv[0].isObj() && argv[0].o == player().o) {
        double f = fireDelay(ovr(1));
        S(argv[0], "fire", Value(f)); S(root(), "firra", Value(f));
    }
    return r;
}
Value wCreate(Act& ac, Value t, int argc, Value* argv) {
    Value r = callOrig(H_CREATE, ac, t, argc, argv);
    if (argc >= 7 && r.isObj() && std::round(toNum(argv[6])) == 2 && !truthy(V(r, "ba")) && !truthy(V(r, "knife"))) {   // the player's tear
        if (std::isfinite(ovr(3))) {   // range: the tear's height (dy) and fall (dm), as the game's "firrr" counts them
            double f = ovr(3) * 23.0 / 6.5, dm = num(V(r, "dm"), 0);
            g_gameFirrr = num(V(root(), "firrr"), 23);   // (what the game made of it: shown again after "range reset")
            S(r, "dy", Value(-f - dm * 5)); S(root(), "firrr", Value(f));
        }
        if (std::isfinite(ovr(4))) {
            double k = ovr(4) / gameShotSpeed();
            S(r, "xbew", Value(num(V(r, "xbew")) * k)); S(r, "ybew", Value(num(V(r, "ybew")) * k));
        }
    }
    return r;
}
Value wPlayerhurt(Act& ac, Value t, int argc, Value* argv) {
    if (g_god) return VUNDEF;
    return callOrig(H_PLAYERHURT, ac, t, argc, argv);
}
const NativeFn kHookFn[H_COUNT] = {wDmgdo, wFirr, wCreate, wPlayerhurt};
bool hookWanted(int h) {
    switch (h) {
        case H_DMGDO: return std::isfinite(ovr(2));
        case H_FIRR: return std::isfinite(ovr(1));
        case H_CREATE: return anyCreateOvr();
        case H_PLAYERHURT: return g_god;
    }
    return false;
}
// installs / removes the wrappers on the current _root.a (every tick: a new room has a new _root.a)
void updateHooks() {
    Value a = gameA();
    if (!a.isObj()) return;
    for (int h = 0; h < H_COUNT; h++) {
        if (!g_hookWrap[h]) {
            Function* f = gcNew<Function>(); f->proto = protos::function; f->nat = kHookFn[h]; f->dbgName = kHookName[h]; gcPin(f);
            g_hookWrap[h] = f;
            gcAddRoot((GcObj**)&g_hookOrig[h]); gcAddRoot((GcObj**)&g_hookA[h]);
        }
        Str* k = intern(kHookName[h]);
        Value cur = getMemK(a, k);
        bool installed = cur.isObj() && cur.o == (Object*)g_hookWrap[h];
        if (hookWanted(h)) {
            if (!installed && cur.isObj() && cur.o->kind == OK_FUNCTION) {
                g_hookOrig[h] = (Function*)cur.o; g_hookA[h] = a.o;
                setMemK(a, k, Value((Object*)g_hookWrap[h]));
                // a new room: what the game computed before the wrapper was there
                if (h == H_FIRR) { Value p = player(); if (p.isObj()) { double f = fireDelay(ovr(1)); S(p, "fire", Value(f)); S(root(), "firra", Value(f)); } }
            }
        } else if (installed && g_hookOrig[h] && g_hookA[h] == a.o) {
            setMemK(a, k, Value((Object*)g_hookOrig[h]));
        }
    }
}

// ---- write guard: no unlocks; the speed held
Object* g_locker = nullptr;
Str* kPlaysp = nullptr;
bool writeGuard(Object* o, Str* k, Value& v) {
    if (o == g_locker) {   // an unlock turning on (they never turn off): dropped
        Value cur = getProp(o, k);
        return !(truthy(v) && !truthy(cur));
    }
    if (o == (Object*)vm.root && k == kPlaysp && std::isfinite(ovr(0))) v = Value(ovr(0));
    return true;
}
void updateGuards() {
    Value lk = V(root(), "locker");
    Object* l = lk.isObj() ? lk.o : nullptr;
    if (l != g_locker) { g_locker = l; if (l) l->guarded = 1; }
    vm.root->guarded = std::isfinite(ovr(0)) ? 1 : 0;
}

// ============================================================ commands
struct Args { std::string line, cmd; std::vector<std::string> w; std::string rest(size_t from) const {
    // the words from `from` on, as typed (names have spaces)
    std::istringstream ss(line); std::string x; size_t i = 0; std::string r;
    while (ss >> x) { if (i++ >= from) r += (r.empty() ? "" : " ") + x; }
    return r;
} };
struct Cmd { const char* name; const char* alias; const char* usage; const char* help; void (*fn)(const Args&); bool needRun; };
extern const Cmd kCmds[];
extern const int kCmdCount;

bool needRun() { if (inRun()) return true; err("Only during a run (not in a cutscene or a boss intro)"); return false; }
int intArg(const Args& a, size_t i, int def) { double d; return i < a.w.size() && isNumber(a.w[i], &d) ? (int)d : def; }

void cmdHelp(const Args& a) {
    if (a.w.size() > 1) {
        std::string q = lower(a.w[1]);
        for (int i = 0; i < kCmdCount; i++) {
            const Cmd& c = kCmds[i];
            if (q == c.name || (c.alias[0] && q == c.alias)) {
                out(fmt("%s %s", c.name, c.usage), C_HEAD);
                out(c.help);
                if (c.alias[0]) out(fmt("Short: %s", c.alias), C_DIM);
                return;
            }
        }
        err(fmt("No command \"%s\"", a.w[1].c_str())); return;
    }
    out("Commands (help <command> for details):", C_HEAD);
    std::string line;
    for (int i = 0; i < kCmdCount; i++) {
        std::string n = kCmds[i].name;
        if (line.size() + n.size() + 2 > 64) { out(line); line.clear(); }
        line += (line.empty() ? "" : "  ") + n;
    }
    if (!line.empty()) out(line);
    out("Tab completes names, Up/Down recall commands. Nothing is saved while this mod is on.", C_DIM);
}
void cmdClear(const Args&) { g_log.clear(); g_logVersion++; }
void cmdClose(const Args&) { s_open = false; }

void cmdGive(const Args& a) {
    if (a.w.size() < 2) { err("give <item>: an item number or name"); return; }
    std::string q = a.rest(1);
    std::vector<const Named*> many; auto items = itemNames();
    int id = findNamed(items, q, many);
    if (id < 0) { err(lookupError("item", q, many)); return; }
    Value A = gameA(), p = player();
    if (!p.isObj()) { err("No player"); return; }
    // as the game gives a character's starting items (golev): the item "picked up" from the player himself
    S(p, "empty", VFALSE); S(A, "lastcra", Value(-100.0)); S(A, "highs", p); S(p, "it", Value((double)id));
    call(A, "powerlevel", {});
    for (auto& n : items) if (n.id == id) { ok(fmt("Given: %s (%d)", n.name.c_str(), id)); break; }
}
void cmdRemove(const Args& a) {
    if (a.w.size() < 2) { err("remove <item>"); return; }
    std::string q = a.rest(1);
    std::vector<const Named*> many; auto items = itemNames();
    int id = findNamed(items, q, many);
    if (id < 0) { err(lookupError("item", q, many)); return; }
    Value ups = V(root(), "ups");
    if (!ups.isObj() || ups.o->kind != OK_ARRAY) { err("No items"); return; }
    Array* arr = (Array*)ups.o;
    double n = (size_t)id < arr->el.size() ? num(arr->el[id]) : 0;
    if (n <= 0) { err(fmt("You don't have %s", q.c_str())); return; }
    setMem(ups, Value((double)id), Value(n - 1));
    Value A = gameA();
    call(A, "upa", {});
    if (player().isObj()) call(A, "firr", {player()});
    std::string nm; for (auto& x : items) if (x.id == id) nm = x.name;
    ok(fmt("Removed: %s (%d). Effects the item had when it was picked up (hearts, pickups) stay.", nm.c_str(), id));
}
// spawn <what> [count]
void cmdSpawn(const Args& a) {
    if (a.w.size() < 2) { err("spawn <what> [count]: an enemy, a boss, a pickup, or item / trinket / card / pill <name>"); return; }
    std::vector<std::string> w = a.w;
    int count = 1;
    double cn;
    std::string kind = lower(w[1]);
    bool named = kind == "item" || kind == "pedestal" || kind == "trinket" || kind == "card" || kind == "pill" || kind == "enemy" || kind == "boss";
    if (w.size() > (named ? 3u : 2u) && isNumber(w.back(), &cn) && cn >= 1 && cn == (int)cn) { count = std::min(20, (int)cn); w.pop_back(); }
    std::string q; for (size_t i = 2; i < w.size(); i++) q += (q.empty() ? "" : " ") + w[i];
    std::vector<const Named*> many;
    auto put = [&](double type, bool nearP, const std::string& what) {
        for (int i = 0; i < count; i++) { double x, y; spawnPos(i, count, nearP, x, y); create(x, y, type); }
        ok(count > 1 ? fmt("Spawned %d x %s", count, what.c_str()) : fmt("Spawned %s", what.c_str()));
    };
    if (kind == "item" || kind == "pedestal") {
        int id = findNamed(itemNames(), q, many); if (id < 0) { err(lookupError("item", q, many)); return; }
        for (auto& n : itemNames()) if (n.id == id) { put(5.1 + id / 10000.0, true, n.name + " (on a pedestal)"); return; }
    }
    if (kind == "trinket") {
        if (game() == "og") { err("The original game has no trinkets"); return; }
        int id = findNamed(trinketNames(), q, many); if (id < 0) { err(lookupError("trinket", q, many)); return; }
        put(5.07 + (28 + id) * 1e-9, true, trinketNames()[id - 1].name); return;
    }
    if (kind == "card") {
        int id = findNamed(cardNames(), q, many); if (id < 0) { err(lookupError("card", q, many)); return; }
        for (auto& n : cardNames()) if (n.id == id) { put(5.07 + id * 1e-9, true, n.name); return; }
    }
    if (kind == "pill") {
        int c = q.empty() ? 0 : atoi(q.c_str());
        if (!q.empty() && (c < 1 || c > 6)) { err("pill [1-6]: the pill's colour (what it does changes every run)"); return; }
        put(5.07 + (c ? c : 1 + rand() % 6) * 1e-9, true, "a pill"); return;
    }
    // a pickup by name
    std::string k; for (size_t i = 1; i < w.size(); i++) k += norm(w[i]);
    for (auto& p : kPickups) {
        if (k == p.name || k == std::string(p.name) + "s") {
            if (p.notOg && game() == "og") { err(fmt("The original game has no %s", p.name)); return; }
            put(5 + p.frame / 100.0 + p.col * 1e-9, true, p.what); return;
        }
    }
    // an enemy or a boss: a type number (with a variant: 10.1), or a name
    std::string eq = q.empty() ? w[1] : w[1] + " " + q;
    if (kind == "enemy" || kind == "boss") eq = q;
    double tn;
    if (isNumber(eq, &tn)) {
        int t = (int)std::floor(tn);
        if (t < 10 || t > (game() == "og" ? 84 : 102) || t == 33 || t == 37 || t == 70) { err(fmt("No enemy type %s in this version (10-%d)", eq.c_str(), game() == "og" ? 84 : 102)); return; }
        const Enemy* e = enemyByType(t);
        put(tn, false, e ? fmt("%s (%s)", e->name, eq.c_str()) : eq); return;
    }
    std::vector<Named> all = enemyNames(false), b = enemyNames(true);
    if (kind != "enemy" && kind != "boss") all.insert(all.end(), b.begin(), b.end()); else if (kind == "boss") all = b;
    int id = findNamed(all, eq, many);
    if (id < 0) { err(lookupError("enemy, boss or pickup", eq, many)); return; }
    put(id, false, enemyByType(id)->name);
}
void cmdKill(const Args&) {
    Value A = gameA(), balls = V(A, "ball");
    if (!balls.isObj() || balls.o->kind != OK_ARRAY) { err("Nothing to kill"); return; }
    std::vector<Value> targets;
    Array* arr = (Array*)balls.o;
    for (auto& b : arr->el) {
        if (!b.isObj()) continue;
        double s = num(V(b, "s"));
        if (s < 10 || truthy(V(b, "dones")) || !(num(V(b, "hp")) > 0)) continue;
        if (s == 33 || s == 37 || s == 42 || s == 44) continue;   // (fires, stone heads: not enemies to kill)
        targets.push_back(b);
    }
    for (auto& b : targets) call(A, "hurt", {b, Value(num(V(b, "hp")) + 1000)});
    ok(fmt("Killed %d", (int)targets.size()));
}
void cmdGod(const Args& a) {
    std::string v = a.w.size() > 1 ? lower(a.w[1]) : "";
    g_god = v == "on" ? true : v == "off" ? false : !g_god;
    ok(g_god ? "God mode on: nothing hurts you (devil deals still cost hearts)" : "God mode off");
}
void cmdDoors(const Args& a) {
    std::string v = a.w.size() > 1 ? lower(a.w[1]) : "";
    if (v == "on") { g_doors = true; ok("Doors on: every room's doors open by themselves (as with Dad's Key)"); }
    else if (v == "off") { g_doors = false; ok("Doors off"); }
    else { S(gameA(), "keyd", VTRUE); ok("Doors opened (doors on: in every room)"); }
}
void setNum(const Value& o, const char* k, const std::string& name, const Args& a, double lo, double hi, bool half = false) {
    double d;
    if (a.w.size() < 2 || !isNumber(a.w[1], &d)) { err(fmt("%s <number>", a.cmd.c_str())); return; }
    d = std::max(lo, std::min(hi, d)); if (half) d = std::round(d * 2) / 2; else d = std::round(d);
    S(o, k, Value(d));
    ok(fmt("%s: %s", name.c_str(), fnum(d).c_str()));
}
void cmdCoins(const Args& a) { setNum(root(), "coins", "Coins", a, 0, 99); }
void cmdBombs(const Args& a) { setNum(root(), "bombs", "Bombs", a, 0, 99); }
void cmdKeys(const Args& a) { setNum(root(), "keys", "Keys", a, 0, 99); }
void cmdSoul(const Args& a) { setNum(root(), "armor", "Soul hearts", a, 0, 12, true); }
void cmdHp(const Args& a) {
    double d; Value p = player();
    if (a.w.size() < 2 || !isNumber(a.w[1], &d)) { err("hp <hearts> (halves: 2.5)"); return; }
    double m = num(V(p, "mhp"), 3);
    d = std::max(0.5, std::min(m, std::round(d * 2) / 2));
    S(p, "hp", Value(d)); ok(fmt("Red hearts: %s of %s", fnum(d).c_str(), fnum(m).c_str()));
}
void cmdMaxhp(const Args& a) {
    double d; Value p = player();
    if (a.w.size() < 2 || !isNumber(a.w[1], &d)) { err("maxhp <heart containers>"); return; }
    d = std::max(0.0, std::min(12.0, std::round(d)));
    S(p, "mhp", Value(d)); if (num(V(p, "hp")) > d) S(p, "hp", Value(d));
    ok(fmt("Heart containers: %s", fnum(d).c_str()));
}
void cmdHeal(const Args&) { Value p = player(); S(p, "hp", V(p, "mhp")); ok("Healed"); }
void cmdCharge(const Args&) { S(root(), "itc", Value(1.0)); ok("Active item charged"); }
void cmdStat(int i, const Args& a) {
    if (a.w.size() < 2) {
        double st[6]; currentStats(st);
        out(fmt("%s: %s%s", kStatNames[i], std::isfinite(st[i]) ? fnum(st[i]).c_str() : "-", std::isfinite(ovr(i)) ? " (fixed by the console)" : ""));
        return;
    }
    std::string v = lower(a.w[1]);
    if (v == "reset" || v == "off") {
        s_ovr[i] = NAN;
        if (i == 1 && player().isObj()) { updateHooks(); call(gameA(), "firr", {player()}); }
        if (i == 3 && std::isfinite(g_gameFirrr)) S(root(), "firrr", Value(g_gameFirrr));
        ok(fmt("%s: back to the game's", kStatNames[i])); return;
    }
    double d;
    if (!isNumber(v, &d)) { err(fmt("%s <number> | reset", kStatNames[i])); return; }
    static const double lo[6] = {0.2, 0.5, 0.1, 1, 0.3, -10}, hi[6] = {3, 15, 500, 30, 4, 50};
    d = std::max(lo[i], std::min(hi[i], d));
    if (i == 5) {
        if (V(root(), "luck").t == T_UNDEF && game() == "og") { err("The original game has no luck"); return; }
        S(root(), "luck", Value(d)); ok(fmt("Luck: %s", fnum(d).c_str())); return;
    }
    s_ovr[i] = d;
    updateGuards(); updateHooks();
    if (i == 0) S(root(), "playsp", Value(d));
    if (i == 1 && player().isObj()) call(gameA(), "firr", {player()});
    if (i == 3) { if (!std::isfinite(g_gameFirrr)) g_gameFirrr = num(V(root(), "firrr"), 23); S(root(), "firrr", Value(d * 23.0 / 6.5)); }
    ok(fmt("%s: %s (until \"%s reset\")", kStatNames[i], fnum(d).c_str(), kStatNames[i]));
}
void cmdSpeed(const Args& a) { cmdStat(0, a); }
void cmdTears(const Args& a) { cmdStat(1, a); }
void cmdDamage(const Args& a) { cmdStat(2, a); }
void cmdRange(const Args& a) { cmdStat(3, a); }
void cmdShotspeed(const Args& a) { cmdStat(4, a); }
void cmdLuck(const Args& a) { cmdStat(5, a); }
void cmdStats(const Args&) {
    double st[6]; currentStats(st);
    std::string s;
    for (int i = 0; i < 6; i++) if (std::isfinite(st[i])) s += fmt("%s%s %s%s", s.empty() ? "" : "   ", kStatNames[i], fnum(st[i]).c_str(), std::isfinite(ovr(i)) ? "*" : "");
    out(s);
    Value p = player();
    out(fmt("hearts %s/%s   soul %s   coins %d   bombs %d   keys %d%s%s", fnum(num(V(p, "hp"))).c_str(), fnum(num(V(p, "mhp"))).c_str(),
            fnum(num(V(root(), "armor"))).c_str(), (int)num(V(root(), "coins")), (int)num(V(root(), "bombs")), (int)num(V(root(), "keys")),
            g_god ? "   GOD" : "", g_doors ? "   DOORS" : ""));
    bool any = false; for (int i = 0; i < 6; i++) any |= std::isfinite(ovr(i));
    if (any) out("* fixed by the console", C_DIM);
}
void cmdItems(const Args&) {
    Value ups = V(root(), "ups");
    if (!ups.isObj() || ups.o->kind != OK_ARRAY) { out("No items"); return; }
    Array* arr = (Array*)ups.o; auto names = itemNames();
    std::string s; int n = 0;
    for (auto& it : names) {
        double c = (size_t)it.id < arr->el.size() ? num(arr->el[it.id]) : 0;
        if (c <= 0) continue;
        std::string e = c > 1 ? fmt("%s x%d (%d)", it.name.c_str(), (int)c, it.id) : fmt("%s (%d)", it.name.c_str(), it.id);
        if (s.size() + e.size() > 70) { out(s); s.clear(); }
        s += (s.empty() ? "" : ", ") + e; n++;
    }
    if (!s.empty()) out(s);
    if (!n) out("No items yet");
    double it = num(V(root(), "it"));   // (the active item's number minus 32, or minus 40 from item 55: powerlevel / itn)
    if (it > 0) {
        int id = (int)it + 32 < 55 ? (int)it + 32 : (int)it + 40;
        for (auto& n : names) if (n.id == id) out(fmt("Active item: %s (%d)", n.name.c_str(), id), C_DIM);
    }
}
void cmdFind(const Args& a) {
    if (a.w.size() < 2) { err("find <text>"); return; }
    std::string k = norm(a.rest(1)); int shown = 0;
    auto list = [&](const char* what, const std::vector<Named>& v) {
        std::string s;
        for (auto& n : v) if (norm(n.name).find(k) != std::string::npos) { if (shown++ >= 40) return; s += fmt("%s%s %d", s.empty() ? "" : ", ", n.name.c_str(), n.id); }
        if (!s.empty()) out(fmt("%s: %s", what, s.c_str()));
    };
    list("items", itemNames()); list("trinkets", trinketNames()); list("cards", cardNames());
    list("enemies", enemyNames(false)); list("bosses", enemyNames(true));
    std::string p; for (auto& x : kPickups) if (std::string(x.name).find(k) != std::string::npos && !(x.notOg && game() == "og")) p += (p.empty() ? "" : ", ") + std::string(x.name);
    if (!p.empty()) out("pickups: " + p);
    if (!shown && p.empty()) err(fmt("Nothing called \"%s\"", a.rest(1).c_str()));
}
// goto <room>
struct RoomRef { const char* name; const char* var; int lev; bool notOg; };
const RoomRef kRooms[] = {
    {"start", nullptr, 35, false}, {"boss", "bossl", 0, false}, {"treasure", "boner", 0, false}, {"treasure2", "boner2", 0, false},
    {"shop", "shopl", 0, false}, {"secret", "hide", 0, false}, {"supersecret", "hide2", 0, false}, {"library", "lib", 0, true},
    {"arcade", "gamb", 0, false}, {"curse", "cus", 0, true}, {"challenge", "chamb", 0, true}, {"sacrifice", "sac", 0, true},
    {"miniboss", "minb", 0, false}, {"devil", nullptr, 166, false}, {"angel", nullptr, 166, true}, {"error", nullptr, 169, true},
};
void cmdGoto(const Args& a) {
    if (a.w.size() < 2) { std::string s; for (auto& r : kRooms) if (!(r.notOg && game() == "og")) s += (s.empty() ? "" : ", ") + std::string(r.name); err("goto <room>: " + s); return; }
    std::string q = lower(a.w[1]);
    for (auto& r : kRooms) {
        if (q != r.name || (r.notOg && game() == "og")) continue;
        double lev = r.lev;
        if (r.var) { Value v = V(root(), r.var); lev = num(v, 0); if (!(lev > 0)) { err(fmt("This floor has no %s room", r.name)); return; } }
        Value levz = V(root(), "levz");
        if (lev == 166) {   // the devil room's contents are picked when the floor is made: devil or angel on demand
            bool angel = q == "angel";
            setMem(levz, Value(166.0), Value(mkstr(angel ? "sata" : "satan"))); S(root(), "altsat", VB(angel));
        }
        if (lev == 169 && !truthy(getMem(levz, Value(169.0)))) setMem(levz, Value(169.0), Value(mkstr("error0")));
        Value A = gameA();
        S(A, "tar", Value(lev));
        call(A, "teller", {});
        ok(fmt("Going to the %s room", r.name));
        s_open = false;   // (the teleport plays once the game runs again)
        return;
    }
    err(fmt("No room \"%s\" (goto: list)", a.w[1].c_str()));
}
// stage: the floor number as the game counts it (_root.chaps: 1-8, 9 Sheol / Cathedral, 11 The Chest), or a name.
// The alternative floors (Wrath of the Lamb, Eternal) are a coin flip when the floor is made (_root.altchap); the
// game forces it when _root.heaven is 2 (the Cathedral route), which the room set-up resets right after. For the
// normal floor the floor is made again until it comes out normal.
struct StageReq { bool on = false; int chaps = 0, want = -1, tries = 0; Object* oldA = nullptr; } g_stage;
const struct { const char* name; int chaps; int alt; } kFloors[] = {
    {"basement", 1, 0}, {"cellar", 1, 1}, {"caves", 3, 0}, {"catacombs", 3, 1}, {"depths", 5, 0}, {"necropolis", 5, 1},
    {"womb", 7, 0}, {"utero", 7, 1}, {"sheol", 9, 0}, {"cathedral", 9, 1}, {"chest", 11, 0}, {"thechest", 11, 0},
};
void makeFloor() {
    if (g_stage.want == 1) S(root(), "heaven", Value(2.0));
    S(root(), "chaps", Value((double)(g_stage.chaps - 1)));
    g_stage.oldA = gameA().isObj() ? gameA().o : nullptr;
    call(gameA(), "newstart", {Value(0.0), Value(1.0)});
}
void cmdStage(const Args& a) {
    bool og = game() == "og";
    const char* usage = "stage <1-9 | 11> [alt]: 1-2 basement, 3-4 caves, 5-6 depths, 7-8 womb, 9 sheol, 11 the chest; or a name "
                        "(cellar, catacombs, necropolis, utero, cathedral, chest...) and 1 or 2";
    if (a.w.size() < 2) { err(usage); return; }
    int n = 0, want = -1;
    std::string k = norm(a.w[1]);
    double d;
    if (isNumber(a.w[1], &d)) n = (int)d;
    else for (auto& f : kFloors) {
        if (norm(f.name) != k) continue;
        n = f.chaps; want = og ? -1 : f.alt;
        if (n < 9 && a.w.size() > 2 && isNumber(a.w[2], &d) && d == 2) n++;
    }
    for (size_t i = 2; i < a.w.size(); i++) { std::string x = lower(a.w[i]); if (x == "alt") want = 1; else if (x == "normal") want = 0; }
    if (n == 10) n = 9;
    if (n < 1 || n > 11 || n == 10) { err(usage); return; }
    if (og && want == 1) { err("The original game has no alternative floors"); return; }
    if (og && n == 11) { err("The original game has no Chest floor"); return; }
    if (n == 11) want = -1;
    g_stage = StageReq(); g_stage.on = true; g_stage.chaps = n; g_stage.want = og ? -1 : want;
    makeFloor();
    ok(fmt("Floor %d%s", n, want == 1 ? " (alternative)" : want == 0 ? " (normal)" : ""));
    s_open = false;
}
// (logic tick) a floor asked for with "stage": done, or made again if it came out alternative when normal was asked
void stageTick() {
    if (!g_stage.on) return;
    Value A = gameA();
    if (!A.isObj() || A.o == g_stage.oldA || !V(A, "player").isObj()) return;
    bool alt = num(V(root(), "altchap")) == 9;
    if (g_stage.want == 0 && alt && g_stage.tries++ < 12) { makeFloor(); return; }
    g_stage.on = false;
}
void cmdNext(const Args&) {
    double c = num(V(root(), "chaps"), 1);
    if (c >= 11) { err("This is the last floor"); return; }
    if (c == 9 && game() == "og") { err("This is the last floor"); return; }
    if (c == 9) S(root(), "chaps", Value(10.0));   // (Sheol / Cathedral: on to the Chest; 10 would be made 9 again)
    call(gameA(), "newstart", {Value(0.0), Value(1.0)});
    ok("Next floor"); s_open = false;
}
void cmdMap(const Args&) {
    S(root(), "world", VTRUE);
    call(gameA(), "upa", {});
    ok("Map, compass and secret rooms shown for this floor");
}
void cmdDebugEnemies(const Args&) {   // (hidden) every enemy type known to the console
    std::string s;
    for (auto& e : kEnemies) { std::string x = fmt("%d %s%s", e.type, e.name, e.boss ? "*" : ""); if (s.size() + x.size() > 70) { out(s); s.clear(); } s += (s.empty() ? "" : ", ") + x; }
    if (!s.empty()) out(s);
}

const Cmd kCmds[] = {
    {"help", "?", "[command]", "The commands, or how one works.", cmdHelp, false},
    {"give", "g", "<item>", "Gives an item, as if picked up: its number or name (give 7, give blood of the martyr, give mom's knife). Tab completes the name.", cmdGive, true},
    {"remove", "r", "<item>", "Takes an item away (one of it). One-time effects (hearts, pickups it gave) stay.", cmdRemove, true},
    {"spawn", "s", "<what> [count]", "Spawns next to you: an enemy or boss by name or type number (spawn gaper 3, spawn monstro, spawn 10.1), a pickup (heart, halfheart, soulheart, eternalheart, penny, nickel, dime, key, goldenkey, bomb, doublebomb, trollbomb, chest, lockedchest, slotmachine), or: item <item>, trinket <name>, card <name>, pill [1-6]. Up to 20 at once.", cmdSpawn, true},
    {"kill", "killall", "", "Kills every enemy in the room.", cmdKill, true},
    {"god", "", "[on|off]", "God mode: nothing hurts you. Without on/off it switches.", cmdGod, true},
    {"doors", "", "[on|off]", "Opens the room's doors now, like Dad's Key. \"doors on\" keeps opening them in every room.", cmdDoors, true},
    {"goto", "tp", "<room>", "Teleports to a room of this floor: start, boss, treasure, treasure2, shop, secret, supersecret, library, arcade, curse, challenge, sacrifice, miniboss, devil, angel, error.", cmdGoto, true},
    {"stage", "", "<floor> [alt|normal]", "Goes to a floor: its number (1-2 basement, 3-4 caves, 5-6 depths, 7-8 womb, 9 sheol, 11 the chest) or its name with 1 or 2 (stage cellar 2, stage catacombs, stage necropolis, stage utero, stage cathedral, stage chest). With a number, alt or normal picks the alternative floor or not; otherwise the game picks, as usual.", cmdStage, true},
    {"next", "", "", "Goes down to the next floor.", cmdNext, true},
    {"map", "", "", "Shows the whole map of this floor, with the secret rooms.", cmdMap, true},
    {"hp", "", "<hearts>", "Red hearts (halves allowed: hp 2.5), up to your heart containers.", cmdHp, true},
    {"maxhp", "", "<containers>", "Heart containers (0-12).", cmdMaxhp, true},
    {"soul", "", "<hearts>", "Soul hearts.", cmdSoul, true},
    {"heal", "", "", "Fills your red hearts.", cmdHeal, true},
    {"coins", "", "<0-99>", "Sets your coins.", cmdCoins, true},
    {"bombs", "", "<0-99>", "Sets your bombs.", cmdBombs, true},
    {"keys", "", "<0-99>", "Sets your keys.", cmdKeys, true},
    {"charge", "", "", "Charges your active item.", cmdCharge, true},
    {"speed", "", "<0.2-3> | reset", "Fixes your speed (1 at the start) whatever your items say, until reset.", cmdSpeed, true},
    {"tears", "", "<0.5-15> | reset", "Fixes your tears (shots per second, 2.73 at the start) until reset.", cmdTears, true},
    {"damage", "", "<0.1-500> | reset", "Fixes your damage (3.5 at the start) until reset. Items that multiply tear damage still do.", cmdDamage, true},
    {"range", "", "<1-30> | reset", "Fixes your range (6.5 at the start) until reset.", cmdRange, true},
    {"shotspeed", "", "<0.3-4> | reset", "Fixes your shot speed (1 at the start) until reset.", cmdShotspeed, true},
    {"luck", "", "<-10-50>", "Sets your luck (Wrath of the Lamb, Eternal Edition).", cmdLuck, true},
    {"stats", "", "", "Your stats, hearts and pickups.", cmdStats, true},
    {"items", "", "", "The items you have, with their numbers.", cmdItems, true},
    {"find", "", "<text>", "Searches items, trinkets, cards, enemies, bosses and pickups by name and shows their numbers.", cmdFind, false},
    {"clear", "cls", "", "Clears the console.", cmdClear, false},
    {"close", "exit", "", "Closes the console (also Esc, the console key, or B).", cmdClose, false},
    {"enemylist", "", "", "Every enemy and boss the console knows (* boss).", cmdDebugEnemies, false},
};
const int kCmdCount = sizeof(kCmds) / sizeof(kCmds[0]);

void runCommand(const std::string& raw) {
    std::string line = trim(raw);
    if (line.empty()) return;
    out("> " + line, C_ECHO);
    Args a; a.line = line;
    std::istringstream ss(line); std::string x; while (ss >> x) a.w.push_back(x);
    a.cmd = lower(a.w[0]);
    for (int i = 0; i < kCmdCount; i++) {
        const Cmd& c = kCmds[i];
        if (a.cmd == c.name || (c.alias[0] && a.cmd == c.alias)) {
            if (c.needRun && !needRun()) return;
            c.fn(a);
            return;
        }
    }
    err(fmt("Unknown command \"%s\" (help: the list)", a.w[0].c_str()));
}

// ============================================================ editing, history, completion
std::string g_line; size_t g_cur = 0;
std::vector<std::string> g_hist; int g_histPos = -1; std::string g_histDraft;
int g_scroll = 0;   // log lines scrolled up
int g_editVersion = 0;
std::vector<std::string> g_compl; int g_complIdx = -1; std::string g_complHead; std::string g_complLine;

std::string historyPath() { return g_modsSaveDir.empty() ? "" : g_modsSaveDir + "genesis_history.txt"; }
void loadHistory() {
    std::string p = historyPath(); if (p.empty()) return;
    FILE* f = platformFopen(p, "rb"); if (!f) return;
    char buf[512];
    while (fgets(buf, sizeof buf, f)) { std::string s = trim(std::string(buf).substr(0, strcspn(buf, "\r\n"))); if (!s.empty()) g_hist.push_back(s); }
    fclose(f);
    if (g_hist.size() > 100) g_hist.erase(g_hist.begin(), g_hist.end() - 100);
}
void saveHistory() {
    std::string p = historyPath(); if (p.empty()) return;
    if (FILE* f = platformFopen(p, "wb")) { for (auto& h : g_hist) fprintf(f, "%s\n", h.c_str()); fclose(f); }
}
// the words that can follow what is typed (for Tab and the hint line)
std::vector<std::string> candidates(const std::string& line, std::string& head) {
    std::vector<std::string> out;
    size_t sp = line.find(' ');
    if (sp == std::string::npos) {   // the command
        head = "";
        std::string k = lower(line);
        for (int i = 0; i < kCmdCount; i++) if (std::string(kCmds[i].name).compare(0, k.size(), k) == 0) out.push_back(kCmds[i].name);
        return out;
    }
    std::string cmd = lower(line.substr(0, sp));
    std::string rest = line.substr(sp + 1);
    head = line.substr(0, sp + 1);
    std::vector<Named> pool;
    auto addNames = [&](const std::vector<Named>& v) { pool.insert(pool.end(), v.begin(), v.end()); };
    if (cmd == "give" || cmd == "g" || cmd == "remove" || cmd == "r") addNames(itemNames());
    else if (cmd == "spawn" || cmd == "s") {
        size_t sp2 = rest.find(' ');
        std::string kind = lower(sp2 == std::string::npos ? rest : rest.substr(0, sp2));
        if (sp2 != std::string::npos && (kind == "item" || kind == "trinket" || kind == "card" || kind == "enemy" || kind == "boss")) {
            head += rest.substr(0, sp2 + 1); rest = rest.substr(sp2 + 1);
            if (kind == "item") addNames(itemNames()); else if (kind == "trinket") addNames(trinketNames());
            else if (kind == "card") addNames(cardNames()); else addNames(enemyNames(kind == "boss"));
        } else {
            for (const char* k : {"item", "trinket", "card", "pill"}) pool.push_back({0, k});
            for (auto& p : kPickups) if (!(p.notOg && game() == "og")) pool.push_back({0, p.name});
            addNames(enemyNames(false)); addNames(enemyNames(true));
        }
    } else if (cmd == "goto" || cmd == "tp") { for (auto& r : kRooms) if (!(r.notOg && game() == "og")) pool.push_back({0, r.name}); }
    else if (cmd == "help" || cmd == "?") { for (int i = 0; i < kCmdCount; i++) pool.push_back({0, kCmds[i].name}); }
    else if (cmd == "god" || cmd == "doors") { pool.push_back({0, "on"}); pool.push_back({0, "off"}); }
    else if (cmd == "speed" || cmd == "tears" || cmd == "damage" || cmd == "range" || cmd == "shotspeed") pool.push_back({0, "reset"});
    std::string k = norm(rest);
    std::vector<std::string> starts, parts;
    for (auto& n : pool) {
        std::string nn = norm(n.name);
        if (nn.compare(0, k.size(), k) == 0) starts.push_back(n.name);
        else if (!k.empty() && nn.find(k) != std::string::npos) parts.push_back(n.name);
    }
    out = starts; out.insert(out.end(), parts.begin(), parts.end());
    // (duplicates: the same name twice in the game's list)
    std::vector<std::string> u; for (auto& s : out) if (std::find(u.begin(), u.end(), s) == u.end()) u.push_back(s);
    return u;
}
void edited() { g_editVersion++; g_complIdx = -1; }
void key(int ck) {
    switch (ck) {
        case CK_ENTER: {
            std::string l = trim(g_line);
            g_line.clear(); g_cur = 0; g_histPos = -1; g_scroll = 0; edited();
            if (l.empty()) return;
            if (g_hist.empty() || g_hist.back() != l) { g_hist.push_back(l); if (g_hist.size() > 100) g_hist.erase(g_hist.begin()); saveHistory(); }
            runCommand(l);
            return;
        }
        case CK_BACKSPACE: if (g_cur > 0) { g_line.erase(g_cur - 1, 1); g_cur--; edited(); } return;
        case CK_DELETE: if (g_cur < g_line.size()) { g_line.erase(g_cur, 1); edited(); } return;
        case CK_LEFT: if (g_cur > 0) { g_cur--; g_editVersion++; } return;
        case CK_RIGHT: if (g_cur < g_line.size()) { g_cur++; g_editVersion++; } return;
        case CK_HOME: g_cur = 0; g_editVersion++; return;
        case CK_END: g_cur = g_line.size(); g_editVersion++; return;
        case CK_CLEARLINE: g_line.clear(); g_cur = 0; edited(); return;
        case CK_PGUP: g_scroll += 5; g_logVersion++; return;
        case CK_PGDN: g_scroll = std::max(0, g_scroll - 5); g_logVersion++; return;
        case CK_UP: case CK_DOWN: {
            if (g_hist.empty()) return;
            if (g_histPos < 0) { if (ck == CK_DOWN) return; g_histDraft = g_line; g_histPos = (int)g_hist.size() - 1; }
            else if (ck == CK_UP) g_histPos = std::max(0, g_histPos - 1);
            else if (++g_histPos >= (int)g_hist.size()) { g_histPos = -1; g_line = g_histDraft; g_cur = g_line.size(); edited(); return; }
            g_line = g_hist[g_histPos]; g_cur = g_line.size(); int hp = g_histPos; edited(); g_histPos = hp;
            return;
        }
        case CK_TAB: {
            if (g_complIdx >= 0 && g_line == g_complLine && !g_compl.empty()) g_complIdx = (g_complIdx + 1) % (int)g_compl.size();
            else {
                g_compl = candidates(g_line.substr(0, g_cur), g_complHead);
                if (g_compl.empty()) return;
                g_complIdx = 0;
            }
            g_line = g_complHead + g_compl[g_complIdx];
            if (g_compl.size() == 1 && g_complHead.empty()) g_line += " ";
            g_cur = g_line.size(); g_complLine = g_line; g_editVersion++;
            return;
        }
    }
}
void typed(const std::string& t) {
    std::string s;
    for (unsigned char c : t) if (c >= 32 && c < 127) s += (char)c;   // (the font has ASCII only)
    if (s.empty() || g_line.size() + s.size() > 160) return;
    g_line.insert(g_cur, s); g_cur += s.size(); g_histPos = -1; edited();
}

// ============================================================ drawing
const FontDef* hudFont() {
    static const FontDef* f = nullptr;
    if (!f) for (CharDef* c : g_movie.chars) if (c && c->type == CH_FONT && ((FontDef*)c)->name.compare(0, 8, "Upheaval") == 0) { f = (FontDef*)c; break; }
    return f;
}
float textW(const std::string& s, float size) {
    const FontDef* f = hudFont(); if (!f) return s.size() * size * 0.6f;
    float w = 0;
    for (unsigned char c : s) { auto it = f->codeToIdx.find(c); int i = it == f->codeToIdx.end() ? -1 : it->second; w += (i >= 0 && i < (int)f->advances.size()) ? f->advances[i] * size : size * 0.5f; }
    return w;
}
std::vector<std::string> wrap(const std::string& s, float width, float size) {
    std::vector<std::string> lines; std::string cur;
    std::istringstream ss(s); std::string w; bool first = true;
    while (ss >> w) {
        std::string t = first ? w : cur + " " + w;
        if (!first && textW(t, size) > width) { lines.push_back(cur); cur = "  " + w; }
        else cur = t;
        first = false;
    }
    if (!first || lines.empty()) lines.push_back(cur);
    return lines;
}
MovieClip* g_ui = nullptr; MovieClip* g_badge = nullptr;
std::vector<EditTextObj*> g_logFields; EditTextObj* g_input = nullptr; EditTextObj* g_hintField = nullptr; EditTextObj* g_status = nullptr;
DisplayObject* g_caret = nullptr;
float g_logW = 0, g_inputX = 0, g_inputW = 0; int g_drawnLog = -1, g_drawnEdit = -1;
const float kSize = 12, kLineH = 15;
void removeUi() { if (g_ui && !g_ui->removed && g_ui->parent) removeObject(g_ui); g_ui = nullptr; g_logFields.clear(); g_input = g_hintField = g_status = nullptr; g_caret = nullptr; }
void removeBadge() { if (g_badge && !g_badge->removed && g_badge->parent) removeObject(g_badge); g_badge = nullptr; }
EditTextObj* field(MovieClip* c, int& depth, double x, double y, float w, float size, uint32_t argb, int align = 0) {
    DisplayObject* d = synthPut(c, synthTextField(w, size, argb, align), depth++, x, y);
    return d && d->kind == OK_EDITTEXT ? (EditTextObj*)d : nullptr;
}
void setField(EditTextObj* e, const std::string& t, uint32_t argb) {
    if (!e) return;
    if (e->argb != argb) { e->argb = argb; e->setText(t + " "); }
    e->setText(t);
}
// the visible part of _root (the stage, in _root coordinates: the HD patch scales _root)
bool visibleArea(double& x0, double& y0, double& x1, double& y1) {
    Mat inv; if (!vm.root->m.invert(inv)) return false;
    inv.apply(0, 0, x0, y0); inv.apply(g_stageW, g_stageH, x1, y1);
    if (x0 > x1) std::swap(x0, x1);
    if (y0 > y1) std::swap(y0, y1);
    return true;
}
void buildUi(double opacity) {
    removeUi();
    double rx0, ry0, rx1, ry1; if (!visibleArea(rx0, ry0, rx1, ry1)) return;
    g_ui = createEmptyMovieClip(vm.root, intern("__genesis"), 1048500);
    if (!g_ui) return;
    double W = rx1 - rx0 - 16, H = std::round((ry1 - ry0) * 0.56), x0 = rx0 + 8, y0 = ry0 + 8;
    int dp = 1;
    synthPut(g_ui, synthRect((float)W, (float)H, (uint32_t)(std::round(opacity * 255)) << 24 | 0x0e0c0bu), dp++, x0, y0);
    synthPut(g_ui, synthRect((float)W, 1.5f, 0xff8c2a2au), dp++, x0, y0 + H);
    if (EditTextObj* t = field(g_ui, dp, x0 + 6, y0 + 3, 200, 11, 0xffd8574bu)) t->setText("GENESIS CONSOLE");
    g_status = field(g_ui, dp, x0 + W - 6 - 420, y0 + 4, 420, 9, 0xff9a948au, 1);
    synthPut(g_ui, synthRect((float)W - 12, 1, 0x60ffffffu), dp++, x0 + 6, y0 + 19);
    double inputY = y0 + H - 36, hintY = y0 + H - 17;
    int n = std::max(3, (int)((inputY - 4 - (y0 + 23)) / kLineH));
    g_logW = (float)W - 20;
    for (int i = 0; i < n; i++) g_logFields.push_back(field(g_ui, dp, x0 + 8, y0 + 23 + i * kLineH, g_logW + 8, kSize, kColors[0]));
    synthPut(g_ui, synthRect((float)W - 12, 18, 0x50000000u), dp++, x0 + 6, inputY - 1);
    EditTextObj* prompt = field(g_ui, dp, x0 + 8, inputY - 1, 20, 13, 0xfff0c674u); if (prompt) prompt->setText(">");
    g_inputX = (float)(x0 + 22); g_inputW = (float)W - 40;
    g_input = field(g_ui, dp, g_inputX, inputY - 1, g_inputW + 8, 13, 0xfff4f0e8u);
    g_caret = synthPut(g_ui, synthRect(1.5f, 14, 0xfff0c674u), dp++, g_inputX + 2, inputY + 1);
    g_hintField = field(g_ui, dp, x0 + 8, hintY, (float)W - 16, 9.5f, 0xff9a948au);
    g_drawnLog = g_drawnEdit = -1;
}
std::string inputHint() {
    if (g_line.empty())
        return "help: commands   Tab: complete   Up/Down: earlier commands   Esc: close   (pad: A keyboard, X run, Y complete, B close)";
    std::string head;
    auto c = candidates(g_line.substr(0, g_cur), head);
    if (c.empty()) {
        std::string cmd = lower(g_line.substr(0, g_line.find(' ')));
        for (int i = 0; i < kCmdCount; i++) if (cmd == kCmds[i].name || (kCmds[i].alias[0] && cmd == kCmds[i].alias)) return std::string(kCmds[i].name) + " " + kCmds[i].usage;
        return "";
    }
    std::string s;
    for (size_t i = 0; i < c.size() && s.size() < 110; i++) s += (s.empty() ? "" : "   ") + c[i];
    if (s.size() >= 110 && c.size() > 1) s += "  ...";
    return "Tab: " + s;
}
void drawUi(uint32_t frame) {
    if (!g_ui) return;
    if (g_drawnLog != g_logVersion) {
        g_drawnLog = g_logVersion;
        std::vector<LogLine> vis;   // wrapped lines, newest last
        for (auto& l : g_log) for (auto& w : wrap(l.text, g_logW, kSize)) vis.push_back({w, l.color});
        int n = (int)g_logFields.size();
        int maxScroll = std::max(0, (int)vis.size() - n); g_scroll = std::min(g_scroll, maxScroll);
        int first = std::max(0, (int)vis.size() - n - g_scroll);
        for (int i = 0; i < n; i++) {
            int k = first + i;
            if (k < (int)vis.size()) setField(g_logFields[i], vis[k].text, kColors[vis[k].color]); else setField(g_logFields[i], "", kColors[0]);
        }
    }
    if (g_drawnEdit != g_editVersion) {
        g_drawnEdit = g_editVersion;
        // the part of the line that fits, keeping the caret visible
        size_t from = 0;
        while (from < g_cur && textW(g_line.substr(from, g_cur - from), 13) > g_inputW - 10) from++;
        std::string shown = g_line.substr(from);
        while (!shown.empty() && textW(shown, 13) > g_inputW) shown.pop_back();
        if (g_input) g_input->setText(shown);
        if (g_caret) { g_caret->m.tx = g_inputX + 2 + textW(g_line.substr(from, g_cur - from), 13); g_caret->scriptXform = true; }
        if (g_hintField) g_hintField->setText(inputHint());
        std::string st = "NOT SAVED";
        if (g_god) st += "   GOD"; if (g_doors) st += "   DOORS";
        for (int i = 0; i < 5; i++) if (std::isfinite(ovr(i))) st += fmt("   %s %s", kStatNames[i], fnum(ovr(i)).c_str());
        if (g_status) g_status->setText(st);
    }
    if (g_caret) g_caret->visible = (frame / 16) % 2 == 0;
}
void updateBadge(bool show) {
    if (!show) { removeBadge(); return; }
    if (g_badge && !g_badge->removed && g_badge->parent == vm.root) return;
    double rx0, ry0, rx1, ry1; if (!visibleArea(rx0, ry0, rx1, ry1)) return;
    g_badge = createEmptyMovieClip(vm.root, intern("__genesisbadge"), 1048400);
    if (!g_badge) return;
    g_badge->cx.am = 0.55f;
    int dp = 1;
    EditTextObj* t = field(g_badge, dp, rx1 - 4 - 160, ry1 - 16, 160, 9, 0xffefe9dcu, 1);
    if (t) t->setText("GENESIS - NOT SAVED");
}

// ============================================================ test hook
std::vector<std::string> g_debugCmds;
}  // namespace

// ============================================================ API
void genesisInit(const ModInfo& m) {
    s_active = true;
    g_saveLocked = true;
    s_pause = m.opt("pause", "1") != "0";
    for (auto& o : s_ovr) o = NAN;
    kPlaysp = intern("playsp");
    g_writeGuard = writeGuard;
    loadHistory();
    logf("mods: GenesisConsole is on: nothing is saved and nothing can be unlocked\n");
}
bool genesisActive() { return s_active.load(); }
bool genesisOpen() { return s_open.load(); }
void genesisSetOpen(bool open) { if (s_active) s_open = open; }
void genesisKey(int ck) { std::lock_guard<std::mutex> lk(s_mu); s_events.push_back({0, ck, ""}); }
void genesisText(const std::string& t) { std::lock_guard<std::mutex> lk(s_mu); s_events.push_back({1, 0, t}); }
bool genesisPausesGame() { return s_active && s_open && s_pause && inRun(); }
double genesisOverride(int stat) { return s_active && stat >= 0 && stat < 6 ? s_ovr[stat].load() : NAN; }
void genesisDebug(const std::string& c) { std::lock_guard<std::mutex> lk(s_mu); s_events.push_back({2, 0, c}); }

void genesisTick(const ModInfo& m) {
    static bool rooted = false;
    if (!rooted) { rooted = true; gcAddRoot((GcObj**)&g_ui); gcAddRoot((GcObj**)&g_badge); gcAddRoot((GcObj**)&g_locker); gcAddRoot((GcObj**)&g_caret);
}
    static uint32_t frame = 0; frame++;
    bool run = inRun();
    updateGuards();
    if (run) updateHooks();
    if (run && g_doors && !truthy(V(gameA(), "keyo"))) S(gameA(), "keyd", VTRUE);
    stageTick();
    // input
    std::vector<ConEv> evs;
    { std::lock_guard<std::mutex> lk(s_mu); evs.swap(s_events); }
    static const bool evLog = getenv("ISAAC_GENESIS_LOG") != nullptr;
    for (auto& e : evs) {
        if (evLog) logf("[genesis-ev] kind %d code %d text '%s' open %d line '%s'\n", e.kind, e.code, e.text.c_str(), (int)s_open.load(), g_line.c_str());
        if (e.kind == 2) {   // tests
            const std::string& c = e.text;
            if (c == "open") s_open = true;
            else if (c == "close") s_open = false;
            else if (c.compare(0, 5, "type ") == 0) typed(c.substr(5));
            else if (c.compare(0, 4, "key ") == 0) key(atoi(c.c_str() + 4));
            else runCommand(c);
            continue;
        }
        if (!s_open) continue;
        if (e.kind == 0) key(e.code); else typed(e.text);
    }
    if (s_open && !run) { s_open = false; }
    if (s_open) {
        if (!g_ui || g_ui->removed || g_ui->parent != vm.root) {
            if (g_log.empty()) { out("GenesisConsole - help: the commands. Nothing is saved while this mod is on.", C_HEAD); }
            buildUi(std::max(0.3, std::min(1.0, atof(m.opt("opacity", "85").c_str()) / 100.0)));
        }
        drawUi(frame);
    } else if (g_ui) removeUi();
    updateBadge(run && !s_open && m.opt("badge", "1") != "0");
}
}  // namespace fl
