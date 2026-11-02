// Continuum (mods/Continuum.mod): leave a run and come back to it later, as Rebirth's "Continue".
//
// The pause screen's "Exit to main menu" asks "Are you sure you want me to die?". With the mod:
//   - "Yes": as always (the run is over, to the main menu);
//   - "No", in a room with no enemies left (its doors open): the run is saved and the game goes to the main menu.
//     The next "Start" there goes straight back into that run, in that room. In a room with enemies "No" does what it
//     always did (back to the pause screen);
//   - a click anywhere else, Escape or the controller's B / circle: back to the pause screen, as before.
// The resumed run starts with the player on the room's centre tile, or, when something is there (a rock, a pit, the
// trapdoor, an item...), on the free tile nearest to the door he came in by.
//
// How: the game keeps everything about a run in _root's variables (the floor's rooms in _root.levz, what is left in
// each room in levit / levsav, the items in ups, hearts, coins...); each room is built again from them when the
// player walks in (_root goes to "reset", the room's clip _root.a starts over). Saving a run is writing those
// variables down (numbers, strings, arrays and plain objects; never the screen's clips, sounds or bitmaps, which
// the game makes again) - and resuming it is putting them back just after the menu starts a new run (darky(150),
// before the game's frame): the game then finds a floor already made (_root.levz) and builds the saved room, as
// after a door. A few variables are not the run's (the options, the unlocks, the menu's own) and are kept as the
// menu has them. Other mods with a run of their own carry on with it (The Gone's run, BaseTime's clocks).
// Kept in saves/continuum_<game>.txt, removed when the run is resumed.
#include "mods.h"
#include "core.h"
#include "display.h"
#include "platform.h"
#include "swfdata.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fl {
bool recompInGameplay(bool* inGame);
extern bool (*g_buttonReleaseHook)(ButtonObj* b);   // display.cpp: a click on a button, before its actions (true: taken)
extern int g_menuNavClick;                          // menuui.cpp: the click is the mouse's (0), the controller's A (1) or B (2)
// other mods' runs (mod_thegone.cpp, mod_basetime.cpp)
bool goneRunActive();
void goneResume(bool gone);
std::string basetimeSuspend();
void basetimeResume(const std::string& s);

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
int curFrame(const Value& v) { return v.isObj() && v.o->kind == OK_CLIP ? ((MovieClip*)v.o)->cur : 0; }
std::string savePath() { return g_modsSaveDir + "continuum_" + game() + ".txt"; }

// _root's variables that are not the run's: kept as the menu has them when a run is resumed
const std::set<std::string> kNotRun = {
    // the menus, the fades, the music
    "men", "fade", "fader", "dark", "undark", "dda", "colo", "musc", "lastn", "bosm", "dmusic", "musicoff", "soundoff",
    "vol", "svol", "mvol", "qq", "full", "intro", "menubeen", "lop2", "gof", "pass", "currUrl", "paus", "pauss",
    "showoff", "itex", "itex2", "unl2", "lockor", "nooo", "ex",
    // the save and the unlocks (they are in the save file, and may have changed in the menus)
    "so", "locker", "colss", "checker", "checker1", "checker2", "linx", "win", "wid", "wider", "colz",
};

std::set<std::string> s_sysKeys;   // _root's variables when the main menu first showed, before any run: the program's, not a run's
bool notRun(Str* k) {
    if (k->s == "hardmode") return false;   // (Eternal: read from the save at start, but set for the run on the character select)
    if (kNotRun.count(k->s) || s_sysKeys.count(k->s)) return true;
    const std::string& s = k->s;   // (the scripts' scratch variables: f1, v2, e, z...)
    if (s == "e" || s == "z" || s == "tex") return true;
    if (s.size() >= 2 && (s[0] == 'f' || s[0] == 'v') && strspn(s.c_str() + 1, "0123456789") == s.size() - 1) return true;
    return false;
}

// ============================================================ saving variables
bool plainObj(Object* o) {
    return (o->kind == OK_ARRAY && o->proto == protos::array) || (o->kind == OK_PLAIN && o->proto == protos::object);
}
bool saveable(const Value& v, std::unordered_set<Object*>& seen, int depth) {
    if (v.t != T_OBJ) return true;
    Object* o = v.o;
    if (seen.count(o)) return true;
    if (!plainObj(o) || depth > 48) return false;
    seen.insert(o);
    if (o->kind == OK_ARRAY) for (auto& e : ((Array*)o)->el) if (!saveable(e, seen, depth + 1)) return false;
    for (auto& p : o->props.v) if (!saveable(p.v, seen, depth + 1)) return false;
    return true;
}
struct Writer {
    std::string out;
    std::unordered_map<Object*, int> ids;
    void str(const std::string& s) { out += std::to_string(s.size()); out += ':'; out += s; }
    void numb(double d) { char b[40]; snprintf(b, sizeof b, "%.17g;", d); out += b; }
    void props(Object* o) {
        out += std::to_string(o->props.v.size()); out += ';';
        for (auto& p : o->props.v) { str(p.k->s); out += (char)('0' + (p.flags & 7)); val(p.v); }
    }
    void val(const Value& v) {
        switch (v.t) {
            case T_UNDEF: out += 'u'; return;
            case T_HOLE: out += 'h'; return;
            case T_NULL: out += 'n'; return;
            case T_BOOL: out += v.b ? 'T' : 'F'; return;
            case T_NUM: out += 'd'; numb(v.n); return;
            case T_STR: out += 's'; str(v.s->s); return;
            case T_OBJ: break;
        }
        auto it = ids.find(v.o);
        if (it != ids.end()) { out += 'r'; out += std::to_string(it->second); out += ';'; return; }
        int id = (int)ids.size(); ids[v.o] = id;
        if (v.o->kind == OK_ARRAY) {
            Array* a = (Array*)v.o;
            out += 'A'; out += std::to_string(a->el.size()); out += ';';
            for (auto& e : a->el) val(e);
            if (a->customOrd) { out += 'o'; out += std::to_string(a->ord.size()); out += ';'; for (uint32_t i : a->ord) { out += std::to_string(i); out += ';'; } }
            else out += '-';
        } else out += 'O';
        props(v.o);
    }
};
struct Reader {
    const std::string& in; size_t p = 0; bool bad = false;
    std::vector<Object*> objs;
    explicit Reader(const std::string& s) : in(s) {}
    char ch() { if (p >= in.size()) { bad = true; return 0; } return in[p++]; }
    long integer(char end) {
        size_t q = in.find(end, p);
        if (q == std::string::npos) { bad = true; return 0; }
        long v = strtol(in.c_str() + p, nullptr, 10); p = q + 1; return v;
    }
    std::string str() {
        long n = integer(':');
        if (bad || n < 0 || p + n > in.size()) { bad = true; return ""; }
        std::string s = in.substr(p, n); p += n; return s;
    }
    void props(Object* o) {
        long n = integer(';');
        for (long i = 0; i < n && !bad; i++) {
            std::string k = str(); int fl = ch() - '0'; Value v = val();
            if (!bad) o->props.set(intern(k), v, (uint8_t)(fl & 7));
        }
    }
    Value val() {
        switch (ch()) {
            case 'u': return VUNDEF;
            case 'h': { Value v; v.t = T_HOLE; return v; }
            case 'n': return VNULL;
            case 'T': return VTRUE;
            case 'F': return VFALSE;
            case 'd': {
                size_t q = in.find(';', p);
                if (q == std::string::npos) { bad = true; return VUNDEF; }
                double d = strtod(in.substr(p, q - p).c_str(), nullptr); p = q + 1; return Value(d);
            }
            case 's': return Value(mkstr(str()));
            case 'r': { long id = integer(';'); if (id < 0 || id >= (long)objs.size()) { bad = true; return VUNDEF; } return Value(objs[id]); }
            case 'A': {
                Array* a = gcNew<Array>(); objs.push_back(a);
                long n = integer(';');
                if (n < 0 || n > 1000000) { bad = true; return VUNDEF; }
                a->el.reserve(n);
                for (long i = 0; i < n && !bad; i++) a->el.push_back(val());
                char c = ch();
                if (c == 'o') {
                    long m = integer(';');
                    for (long i = 0; i < m && !bad; i++) a->ord.push_back((uint32_t)integer(';'));
                    a->customOrd = true;
                } else if (c != '-') bad = true;
                props(a);
                return Value((Object*)a);
            }
            case 'O': { Object* o = gcNew<Object>(protos::object); objs.push_back(o); props(o); return Value(o); }
        }
        bad = true; return VUNDEF;
    }
};

// ============================================================ the run being left
// the room is clear: no enemy closes its doors (the game counts them every image in shutdoor, ashut)
bool roomClear(const Value& a) {
    return num(V(a, "shutdoor"), 1) == 0 && num(V(a, "ashut"), 1) == 0 && num(V(a, "fra")) > 30 && num(V(a, "gameover")) == 0;
}
// where the player will appear: the centre tile, or the free one nearest to the way he came in
void spawnPoint(const Value& a, double& sx, double& sy) {
    Value lz = V(a, "levz"), ball = V(a, "ball");
    auto tileFree = [&](int t) {
        Value v = lz.isObj() && lz.o->kind == OK_ARRAY && t < (int)((Array*)lz.o)->el.size() ? ((Array*)lz.o)->el[t] : VUNDEF;
        if (!v.isUndef() && num(v) != 0) return false;   // a rock, a pit, fire, spikes, a block...
        double x = (t % 16) * 40.0, y = (t / 16) * 40.0;
        if (ball.isObj() && ball.o->kind == OK_ARRAY)   // anything standing there: the trapdoor, an item, a chest...
            for (auto& e : ((Array*)ball.o)->el) {
                if (!e.isObj() || !e.o->isDisplay() || V(a, "player").o == e.o) continue;
                if (truthy(V(e, "dones")) || !((DisplayObject*)e.o)->visible) continue;
                double ex = num(V(e, "xp"), -999), ey = num(V(e, "yp"), -999);
                if (std::fabs(ex - x) < 32 && std::fabs(ey - y) < 32) return false;
            }
        return true;
    };
    const int cx = 8, cy = 7;   // (320, 280): the middle of the room
    if (tileFree(cx + 16 * cy)) { sx = 320; sy = 280; return; }
    // the way in: where the game put the player when he came in (_root.playerx / playery, next to that door)
    double dx = num(V(root(), "playerx"), 320), dy = num(V(root(), "playery"), 400);
    double best = 1e30; sx = 320; sy = 400;
    for (int ty = 4; ty <= 10; ty++)
        for (int tx = 2; tx <= 14; tx++) {
            int t = tx + 16 * ty;
            if (!tileFree(t)) continue;
            double d = (tx * 40.0 - dx) * (tx * 40.0 - dx) + (ty * 40.0 - dy) * (ty * 40.0 - dy);
            if (d < best) { best = d; sx = tx * 40.0; sy = ty * 40.0; }
        }
}
bool saveRun(const Value& a) {
    Value R = root();
    Writer w;
    int n = 0; std::string body;
    std::vector<std::string> skipped;
    for (auto& p : vm.root->props.v) {
        if (notRun(p.k)) continue;
        if (p.v.t == T_OBJ && p.v.o->kind == OK_FUNCTION) continue;
        std::unordered_set<Object*> seen;
        if (!saveable(p.v, seen, 0)) { if (p.v.t == T_OBJ && !p.v.o->isDisplay()) skipped.push_back(p.k->s); continue; }
        w.str(p.k->s); w.val(p.v); n++;
    }
    double sx, sy; spawnPoint(a, sx, sy);
    std::string head = "continuum 1\ngame " + game() + "\ngone " + (goneRunActive() ? "1" : "0") + "\n";
    char b[64]; snprintf(b, sizeof b, "spawn %.0f %.0f\n", sx, sy); head += b;
    if (modActive("basetime")) head += "basetime " + basetimeSuspend() + "\n";
    head += "vars " + std::to_string(n) + " " + std::to_string(w.out.size()) + "\n";
    FILE* f = platformFopen(savePath(), "wb");
    if (!f) { logf("continuum: can't write %s\n", savePath().c_str()); return false; }
    bool ok = fwrite(head.data(), 1, head.size(), f) == head.size() && fwrite(w.out.data(), 1, w.out.size(), f) == w.out.size();
    ok = fclose(f) == 0 && ok;
    if (!ok) { platformRemove(savePath()); logf("continuum: can't write %s\n", savePath().c_str()); return false; }
    std::string sk; for (auto& s : skipped) sk += " " + s;
    logf("continuum: run saved (floor %g, room %g, %d variables, %zu bytes; spawn %.0f,%.0f)%s%s\n", num(V(R, "chaps")), num(V(R, "lev")), n, w.out.size(), sx, sy,
         sk.empty() ? "" : "; not saved:", sk.c_str());
    return true;
}
// what "Yes" does: the run ends, the main menu
void toMenu(const Value& a) {
    Value R = root();
    S(R, "levz", VUNDEF);
    call(a, "moveon", {});
    S(R, "door", VUNDEF);
    call(R, "gotoAndStop", {Value(mkstr("menu"))});
}

// ============================================================ the run coming back
struct Saved { bool ok = false, gone = false; double sx = 320, sy = 280; std::string basetime, vars; int nvars = 0; };
Saved readSave() {
    Saved s;
    FILE* f = platformFopen(savePath(), "rb");
    if (!f) return s;
    std::string all; char buf[65536]; size_t r;
    while ((r = fread(buf, 1, sizeof buf, f)) > 0) all.append(buf, r);
    fclose(f);
    size_t p = 0; bool vars = false;
    while (p < all.size() && !vars) {
        size_t e = all.find('\n', p); if (e == std::string::npos) break;
        std::string line = all.substr(p, e - p); p = e + 1;
        if (line.compare(0, 5, "game ") == 0 && line.substr(5) != game()) return s;
        else if (line.compare(0, 5, "gone ") == 0) s.gone = line[5] == '1';
        else if (line.compare(0, 6, "spawn ") == 0) sscanf(line.c_str() + 6, "%lf %lf", &s.sx, &s.sy);
        else if (line.compare(0, 9, "basetime ") == 0) s.basetime = line.substr(9);
        else if (line.compare(0, 5, "vars ") == 0) {
            size_t len = 0; sscanf(line.c_str() + 5, "%d %zu", &s.nvars, &len);
            if (p + len > all.size()) return s;
            s.vars = all.substr(p, len); vars = true;
        }
    }
    s.ok = vars;
    return s;
}
bool hasSave() { FILE* f = platformFopen(savePath(), "rb"); if (!f) return false; fclose(f); return true; }

int s_resume = 0;   // 1: resumed on the menu, waiting for the game's frame; 2: waiting for the run to be playable
Saved s_saved;
bool resumeRun() {
    Saved s = readSave();
    if (!s.ok) { logf("continuum: the saved run can't be read: a new run\n"); platformRemove(savePath()); return false; }
    Value R = root();
    // the game's own start of a run (the character select's "Start"), then the saved run's variables over it
    S(R, "chala", VUNDEF);
    call(R, "darky", {Value(150.0)});
    Reader rd(s.vars);
    std::vector<std::pair<Str*, Value>> vals;
    for (int i = 0; i < s.nvars && !rd.bad; i++) {
        std::string k = rd.str(); Value v = rd.val();
        if (!rd.bad) vals.push_back({intern(k), v});
    }
    if (rd.bad) { logf("continuum: the saved run can't be read: a new run\n"); platformRemove(savePath()); return true; }
    for (auto& kv : vals) setMemK(R, kv.first, kv.second);   // (no collection can run before they are in _root)
    S(R, "door", VUNDEF);   // (no door: the room doesn't slide in)
    S(R, "playerx", Value(s.sx)); S(R, "playery", Value(s.sy));
    S(R, "newstartt", VFALSE);
    platformRemove(savePath());
    logf("continuum: run resumed (floor %g, room %g, %zu variables)\n", num(V(R, "chaps")), num(V(R, "lev")), vals.size());
    s_saved = s;
    return true;
}

// ============================================================ the "are you sure?" note
// On the pause sheet's frame 6 the note's "Yes" is a button (it also answers to Enter); "No" is the full-screen
// button under everything (it also answers to Escape), which goes back to the pause screen (frame 2) wherever it is
// clicked. Its word "No" is drawn on the note just to the right of "Yes" ("Yes / No"): a click there (or the
// controller's A on it) is the new "No"; anywhere else, Escape and B / circle still go back to the pause screen.
int s_lastPos = -100;
Value pauseSheet() { return V(V(root(), "hud"), "paus"); }
bool keyButton(ButtonObj* b, int code) {   // a button that answers to this key (SWF key codes: 13 Enter, 19 Escape)
    if (!b->def) return false;
    for (auto& c : b->def->conds) if ((c.cond >> 9) == code) return true;
    return false;
}
bool onNoWord(double mx, double my) {
    Value ps = pauseSheet();
    if (!ps.isObj() || ps.o->kind != OK_CLIP) return false;
    for (DisplayObject* k : ((MovieClip*)ps.o)->kids) {
        if (k->kind != OK_BUTTON || !k->visible || !keyButton((ButtonObj*)k, 13)) continue;
        RectF y = hitStageBounds((ButtonObj*)k);   // "Yes"
        if (!y.valid()) continue;
        double w = y.x1 - y.x0, h = y.y1 - y.y0;
        if (getenv("ISAAC_CONT_LOG")) logf("[continuum] \"No\": %.0f,%.0f-%.0f,%.0f, pointer %.0f,%.0f\n", y.x1 + 0.6 * w, y.y0 - 0.25 * h, y.x1 + 1.9 * w, y.y1 + 0.25 * h, mx, my);
        return mx >= y.x1 + 0.6 * w && mx <= y.x1 + 1.9 * w && my >= y.y0 - 0.25 * h && my <= y.y1 + 0.25 * h;
    }
    return false;
}
bool s_leave = false;   // "No" was chosen in a clear room: the run is saved and left this tick
bool onRelease(ButtonObj* b) {   // (display.cpp: a click on a button, before its actions; logic thread)
    if (!modActive("continuum") || !b || !b->parent) return false;
    Value ps = pauseSheet();
    if (!ps.isObj() || (DisplayObject*)ps.o != b->parent || curFrame(ps) != 6 || !keyButton(b, 19)) return false;
    bool inGame = false, play = recompInGameplay(&inGame);
    Value a = V(root(), "a");
    if (!inGame || play || !a.isObj()) return false;
    if (g_menuNavClick == 2) return false;   // (B / circle: back to the pause screen)
    if (g_menuNavClick == 0 && !onNoWord(g_player.mouseX, g_player.mouseY)) return false;   // (a click outside the word: the same)
    if (!roomClear(a)) { logf("continuum: \"No\" with enemies in the room: not saved\n"); return false; }
    if (!saveRun(a)) return false;
    toMenu(a);
    return true;
}
}  // namespace

void continuumDebug(const std::string& cmd) {
    if (cmd == "keys") {   // _root's variables: what would be saved
        for (auto& p : vm.root->props.v) {
            const char* what = "";
            std::unordered_set<Object*> seen;
            if (notRun(p.k)) what = "not the run's";
            else if (p.v.t == T_OBJ && p.v.o->kind == OK_FUNCTION) what = "function";
            else if (!saveable(p.v, seen, 0)) what = p.v.t == T_OBJ && p.v.o->isDisplay() ? "display" : "NOT SAVEABLE";
            else what = "saved";
            std::string val = p.v.t == T_OBJ ? (p.v.o->kind == OK_ARRAY ? "[array " + std::to_string(((Array*)p.v.o)->el.size()) + "]" : "{object}") : std::string(toStr(p.v)->s).substr(0, 40);
            logf("[continuum] %s = %s (%s)\n", p.k->s.c_str(), val.c_str(), what);
        }
    } else if (cmd.compare(0, 5, "dump ") == 0) {   // the run's variables, one a line, into a file
        if (FILE* f = fopen(cmd.substr(5).c_str(), "wb")) {
            for (auto& p : vm.root->props.v) {
                std::unordered_set<Object*> seen;
                if (notRun(p.k) || (p.v.t == T_OBJ && p.v.o->kind == OK_FUNCTION) || !saveable(p.v, seen, 0)) continue;
                Writer w; w.val(p.v); fprintf(f, "%s %s\n", p.k->s.c_str(), w.out.c_str());
            }
            fclose(f);
        }
    } else if (cmd == "dialog") {
        Value ps = pauseSheet();
        if (ps.isObj() && ps.o->kind == OK_CLIP)
            for (DisplayObject* k : ((MovieClip*)ps.o)->kids) {
                RectF r = k->kind == OK_BUTTON ? hitStageBounds((ButtonObj*)k) : stageBounds(k);
                logf("[continuum] child %d kind %d id %d vis %d: %.0f,%.0f-%.0f,%.0f\n", k->depth, k->kind, k->charId, (int)k->visible, r.x0, r.y0, r.x1, r.y1);
            }
    } else if (cmd == "state") {
        Value a = V(root(), "a");
        double sx = 0, sy = 0; if (a.isObj()) spawnPoint(a, sx, sy);
        logf("[continuum] clear %d shutdoor %g ashut %g fra %g paus %d unpause %d spawn %.0f,%.0f save %d\n", a.isObj() && roomClear(a), num(V(a, "shutdoor"), -1), num(V(a, "ashut"), -1),
             num(V(a, "fra")), curFrame(pauseSheet()), (int)truthy(V(a, "unpause")), sx, sy, (int)hasSave());
    }
}
void continuumTick(const ModInfo&) {
    g_buttonReleaseHook = onRelease;
    Value R = root();
    int frame = vm.root->kind == OK_CLIP ? ((MovieClip*)vm.root)->cur : 0;
    bool inGame = false, play = recompInGameplay(&inGame);
    if (frame == 5 && s_sysKeys.empty()) for (auto& p : vm.root->props.v) s_sysKeys.insert(p.k->s);
    // ---- resuming
    if (frame == 5) {
        Value sheet = V(V(R, "men"), "men");
        Value pv = V(sheet, "pos");
        int pos = pv.isUndef() ? -1 : (int)num(pv, -2);
        if (s_resume == 0 && pos == 3 && s_lastPos == -1 && hasSave()) { if (resumeRun()) s_resume = 1; }
        s_lastPos = pos;
    } else s_lastPos = -100;
    if (s_resume == 1 && frame != 5) {
        s_resume = 2;
        if (modActive("thegone")) goneResume(s_saved.gone);
    }
    if (s_resume == 2 && play) {
        s_resume = 0;
        if (modActive("basetime") && !s_saved.basetime.empty()) basetimeResume(s_saved.basetime);
    }
    if (frame == 5 && s_resume == 2) s_resume = 0;
}
}  // namespace fl
