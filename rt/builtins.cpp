// Global objects and native classes used by the game.
#include "display.h"
#include "platform.h"
#include "snapshot.h"
#include <algorithm>
#include <cstdio>
#include <functional>

namespace fl {

namespace protos { Object *object, *array, *function, *string, *number, *boolean, *movieclip, *button, *textfield,
    *bitmapdata, *sound, *color, *transform, *colortransform, *matrix, *rectangle, *point, *sharedobject; }
extern std::vector<std::function<void(Tracer&)>> g_rootTracers;
std::string g_savePath;   // SharedObject file (set by main)

// ---- utilities
static inline Value arg(int argc, Value* argv, int i) { return i < argc ? argv[i] : VUNDEF; }
static inline double argn(int argc, Value* argv, int i, double def = NAN) { return i < argc ? toNum(argv[i]) : def; }
static Function* nativeFn(NativeFn f, const char* name) {
    Function* fn = gcNew<Function>(); fn->nat = f; fn->dbgName = name; gcPin(fn); return fn;
}
static void defn(Object* o, const char* name, NativeFn f) { o->props.set(intern(name), Value((Object*)nativeFn(f, name)), PF_DONTENUM); }
static void defv(Object* o, const char* name, const Value& v) { o->props.set(intern(name), v, PF_DONTENUM); }
static Object* newProtoObj(Object* proto) { Object* o = gcNew<Object>(proto); gcPin(o); return o; }
static Function* ctorFn(NativeFn ctor, Object* proto, const char* name) {
    Function* f = nativeFn(ctor, name); f->ctor = ctor;
    f->props.set(atoms::prototype, Value(proto), PF_DONTENUM);
    proto->props.set(atoms::constructor, Value((Object*)f), PF_DONTENUM);
    return f;
}
static DisplayObject* asDO(const Value& v) {
    if (!v.isObj() || !v.o->isDisplay()) return nullptr;
    DisplayObject* d = (DisplayObject*)v.o;
    if (d->removed) d = doResolveRemoved(d);
    return d;
}
static MovieClip* asClip(const Value& v) { DisplayObject* d = asDO(v); return d && d->kind == OK_CLIP ? (MovieClip*)d : nullptr; }

// ============================================================ Object / Function
static Value obj_toString(Act&, Value, int, Value*) { return Value(intern("[object Object]")); }
static Value obj_valueOf(Act&, Value t, int, Value*) { return t; }
static Value obj_hasOwn(Act&, Value t, int argc, Value* argv) {
    if (!t.isObj()) return VFALSE; Str* k = keyOf(arg(argc, argv, 0));
    return VB(t.o->props.find(k) >= 0 || (t.o->kind == OK_ARRAY && k->arrIdx >= 0 && (size_t)k->arrIdx < ((Array*)t.o)->el.size()));
}
static Value obj_ctor(Act&, Value, int argc, Value* argv) {
    if (argc > 0 && argv[0].isObj()) return argv[0];
    if (argc > 0 && !argv[0].isNullish()) return Value(toObject(argv[0]));
    return Value(gcNew<Object>(protos::object));
}
static Value fn_call(Act& a, Value t, int argc, Value* argv) { return callValue(a, t, arg(argc, argv, 0), argc > 0 ? argc - 1 : 0, argv + 1); }
static Value fn_apply(Act& a, Value t, int argc, Value* argv) {
    std::vector<Value> args;
    Value arr = arg(argc, argv, 1);
    if (arr.isObj() && arr.o->kind == OK_ARRAY) for (auto& v : ((Array*)arr.o)->el) args.push_back(v.t == T_HOLE ? VUNDEF : v);
    return callValue(a, t, arg(argc, argv, 0), (int)args.size(), args.data());
}

// ============================================================ Array
static Array* thisArr(const Value& t) { return t.isObj() && t.o->kind == OK_ARRAY ? (Array*)t.o : nullptr; }
static Value arr_ctor(Act&, Value, int argc, Value* argv) {
    Array* a = gcNew<Array>();
    if (argc == 1 && argv[0].t == T_NUM) { int n = toInt32(argv[0].n); if (n > 0) { a->el.resize(n); for (auto& v : a->el) v.t = T_HOLE; } }
    else a->el.assign(argv, argv + argc);
    return Value((Object*)a);
}
static Value arr_push(Act&, Value t, int argc, Value* argv) { Array* a = thisArr(t); if (!a) return VUNDEF; for (int i = 0; i < argc; i++) { if (a->customOrd) a->ord.push_back((uint32_t)a->el.size()); a->el.push_back(argv[i]); } return Value((double)a->el.size()); }
static Value arr_pop(Act&, Value t, int, Value*) { Array* a = thisArr(t); if (!a || a->el.empty()) return VUNDEF; Value v = a->el.back(); a->el.pop_back(); arrTruncOrder(a); return v.t == T_HOLE ? VUNDEF : v; }
static Value arr_shift(Act&, Value t, int, Value*) { Array* a = thisArr(t); if (!a || a->el.empty()) return VUNDEF; Value v = a->el.front(); a->el.erase(a->el.begin()); arrTruncOrder(a); return v.t == T_HOLE ? VUNDEF : v; }
static Value arr_unshift(Act&, Value t, int argc, Value* argv) { Array* a = thisArr(t); if (!a) return VUNDEF; size_t old = a->el.size(); a->el.insert(a->el.begin(), argv, argv + argc);
    if (a->customOrd) for (size_t i = a->el.size(); i-- > old;) a->ord.push_back((uint32_t)i);   // Ruffle sets the top first
    return Value((double)a->el.size()); }
static int clampIdx(double v, int len) { if (std::isnan(v)) return 0; int i = (int)v; if (i < 0) i = std::max(0, len + i); return std::min(i, len); }
static Value arr_slice(Act&, Value t, int argc, Value* argv) {
    Array* a = thisArr(t); if (!a) return VUNDEF; int len = (int)a->el.size();
    int s = clampIdx(argn(argc, argv, 0, 0), len), e = argc > 1 && !argv[1].isUndef() ? clampIdx(toNum(argv[1]), len) : len;
    Array* r = gcNew<Array>(); if (e > s) r->el.assign(a->el.begin() + s, a->el.begin() + e); return Value((Object*)r);
}
static Value arr_splice(Act&, Value t, int argc, Value* argv) {
    Array* a = thisArr(t); if (!a || argc < 1) return VUNDEF; int len = (int)a->el.size();
    int s = clampIdx(toNum(argv[0]), len);
    int del = argc > 1 ? std::max(0, std::min(toInt32(argv[1]), len - s)) : len - s;
    Array* r = gcNew<Array>(); r->el.assign(a->el.begin() + s, a->el.begin() + s + del);
    a->el.erase(a->el.begin() + s, a->el.begin() + s + del);
    if (argc > 2) a->el.insert(a->el.begin() + s, argv + 2, argv + argc);
    if (a->customOrd) { if (a->el.size() > (size_t)len) { for (size_t i = len; i < a->el.size(); i++) a->ord.push_back((uint32_t)i); } else arrTruncOrder(a); }
    return Value((Object*)r);
}
static Value arr_join(Act&, Value t, int argc, Value* argv) {
    Array* a = thisArr(t); if (!a) return Value(atoms::empty);
    std::string sep = argc > 0 && !argv[0].isUndef() ? toStr(argv[0])->s : ",";
    std::string out;
    for (size_t i = 0; i < a->el.size(); i++) { if (i) out += sep; const Value& v = a->el[i]; if (!v.isUndef()) out += toStr(v)->s; }
    return Value(mkstr(out));
}
static Value arr_toString(Act& ac, Value t, int, Value*) { Value c = Value(intern(",")); return arr_join(ac, t, 1, &c); }
static Value arr_concat(Act&, Value t, int argc, Value* argv) {
    Array* a = thisArr(t); Array* r = gcNew<Array>(); if (a) r->el = a->el;
    for (int i = 0; i < argc; i++) { if (Array* b = thisArr(argv[i])) r->el.insert(r->el.end(), b->el.begin(), b->el.end()); else r->el.push_back(argv[i]); }
    return Value((Object*)r);
}
static Value arr_reverse(Act&, Value t, int, Value*) { if (Array* a = thisArr(t)) std::reverse(a->el.begin(), a->el.end()); return t; }
static Value arr_sort(Act& ac, Value t, int argc, Value* argv) {
    Array* a = thisArr(t); if (!a) return t;
    Value cmp = arg(argc, argv, 0);
    if (cmp.isObj() && cmp.o->kind == OK_FUNCTION) {
        std::stable_sort(a->el.begin(), a->el.end(), [&](const Value& x, const Value& y) { Value args[2] = {x, y}; return toNum(callValue(ac, cmp, VUNDEF, 2, args)) < 0; });
    } else {
        bool numeric = cmp.t == T_NUM && (toInt32(cmp) & 16);
        std::stable_sort(a->el.begin(), a->el.end(), [&](const Value& x, const Value& y) {
            if (numeric) return toNum(x) < toNum(y);
            return toStr(x)->s < toStr(y)->s; });
    }
    return t;
}

// ============================================================ String / Number / Boolean
static Str* thisStr(const Value& t) { if (t.t == T_STR) return t.s; if (t.isObj() && t.o->kind == OK_BOXED) return toStr(((Boxed*)t.o)->prim); return toStr(t); }
static Value str_ctor(Act&, Value, int argc, Value* argv) { return Value(argc ? toStr(argv[0]) : atoms::empty); }
static Value num_ctor(Act&, Value, int argc, Value* argv) { return Value(argc ? toNum(argv[0]) : 0.0); }
static Value bool_ctor(Act&, Value, int argc, Value* argv) { return VB(argc ? truthy(argv[0]) : false); }
static Value str_split(Act&, Value t, int argc, Value* argv) {
    std::string s = thisStr(t)->s; Array* r = gcNew<Array>();
    if (argc == 0 || argv[0].isUndef()) { r->el.push_back(Value(mkstr(s))); return Value((Object*)r); }
    std::string sep = toStr(argv[0])->s;
    if (sep.empty()) { for (char c : s) r->el.push_back(Value(mkstr(std::string(1, c)))); return Value((Object*)r); }
    size_t p = 0;
    while (true) { size_t q = s.find(sep, p); if (q == std::string::npos) { r->el.push_back(Value(mkstr(s.substr(p)))); break; } r->el.push_back(Value(mkstr(s.substr(p, q - p)))); p = q + sep.size(); }
    return Value((Object*)r);
}
static Value str_indexOf(Act&, Value t, int argc, Value* argv) {
    std::string s = thisStr(t)->s; std::string n = toStr(arg(argc, argv, 0))->s; int st = argc > 1 ? std::max(0, toInt32(argv[1])) : 0;
    size_t r = s.find(n, st); return Value(r == std::string::npos ? -1.0 : (double)r);
}
static Value str_lastIndexOf(Act&, Value t, int argc, Value* argv) {
    std::string s = thisStr(t)->s; std::string n = toStr(arg(argc, argv, 0))->s;
    size_t r = s.rfind(n); return Value(r == std::string::npos ? -1.0 : (double)r);
}
static Value str_substr(Act&, Value t, int argc, Value* argv) {
    std::string s = thisStr(t)->s; int len = (int)s.size();
    int st = argc > 0 ? toInt32(argv[0]) : 0; if (st < 0) st = std::max(0, len + st); if (st > len) st = len;
    int n = argc > 1 && !argv[1].isUndef() ? toInt32(argv[1]) : len - st; if (n < 0) n = 0;
    return Value(mkstr(s.substr(st, n)));
}
static Value str_substring(Act&, Value t, int argc, Value* argv) {
    std::string s = thisStr(t)->s; int len = (int)s.size();
    int a = std::max(0, std::min(len, argc > 0 ? toInt32(argv[0]) : 0));
    int b = argc > 1 && !argv[1].isUndef() ? std::max(0, std::min(len, toInt32(argv[1]))) : len;
    if (a > b) std::swap(a, b); return Value(mkstr(s.substr(a, b - a)));
}
static Value str_slice(Act&, Value t, int argc, Value* argv) {
    std::string s = thisStr(t)->s; int len = (int)s.size();
    int a = clampIdx(argn(argc, argv, 0, 0), len), b = argc > 1 && !argv[1].isUndef() ? clampIdx(toNum(argv[1]), len) : len;
    return Value(mkstr(b > a ? s.substr(a, b - a) : ""));
}
static Value str_charAt(Act&, Value t, int argc, Value* argv) { std::string s = thisStr(t)->s; int i = toInt32(arg(argc, argv, 0)); return Value(i >= 0 && i < (int)s.size() ? mkstr(std::string(1, s[i])) : atoms::empty); }
static Value str_charCodeAt(Act&, Value t, int argc, Value* argv) { std::string s = thisStr(t)->s; int i = toInt32(arg(argc, argv, 0)); return Value(i >= 0 && i < (int)s.size() ? (double)(uint8_t)s[i] : NAN); }
static Value str_lower(Act&, Value t, int, Value*) { std::string s = thisStr(t)->s; for (auto& c : s) c = (char)tolower((unsigned char)c); return Value(mkstr(s)); }
static Value str_upper(Act&, Value t, int, Value*) { std::string s = thisStr(t)->s; for (auto& c : s) c = (char)toupper((unsigned char)c); return Value(mkstr(s)); }
static Value str_toString(Act&, Value t, int, Value*) { return Value(thisStr(t)); }
static Value str_fromCharCode(Act&, Value, int argc, Value* argv) { std::string s; for (int i = 0; i < argc; i++) s += (char)toInt32(argv[i]); return Value(mkstr(s)); }
static Value num_toString(Act&, Value t, int argc, Value* argv) {
    double n = t.t == T_NUM ? t.n : (t.isObj() && t.o->kind == OK_BOXED ? toNum(((Boxed*)t.o)->prim) : toNum(t));
    int radix = argc ? toInt32(argv[0]) : 10;
    if (radix == 10 || radix < 2 || radix > 36) return Value(toStr(Value(n)));
    int64_t v = (int64_t)n; bool neg = v < 0; if (neg) v = -v; std::string s;
    do { int d = (int)(v % radix); s.insert(s.begin(), (char)(d < 10 ? '0' + d : 'a' + d - 10)); v /= radix; } while (v);
    if (neg) s.insert(s.begin(), '-'); return Value(mkstr(s));
}
static Value num_valueOf(Act&, Value t, int, Value*) { if (t.isObj() && t.o->kind == OK_BOXED) return ((Boxed*)t.o)->prim; return t; }

// ============================================================ Math
#define MATH1(name, expr) static Value math_##name(Act&, Value, int argc, Value* argv) { double x = argn(argc, argv, 0); return Value(expr); }
MATH1(abs, std::fabs(x)) MATH1(acos, std::acos(x)) MATH1(asin, std::asin(x)) MATH1(atan, std::atan(x)) MATH1(ceil, std::ceil(x))
MATH1(cos, std::cos(x)) MATH1(exp, std::exp(x)) MATH1(floor, std::floor(x)) MATH1(log, std::log(x)) MATH1(sin, std::sin(x))
MATH1(sqrt, std::sqrt(x)) MATH1(tan, std::tan(x))
static Value math_round(Act&, Value, int argc, Value* argv) { double x = argn(argc, argv, 0); return Value(std::floor(x + 0.5)); }
static Value math_atan2(Act&, Value, int argc, Value* argv) { return Value(std::atan2(argn(argc, argv, 0), argn(argc, argv, 1))); }
static Value math_pow(Act&, Value, int argc, Value* argv) { return Value(std::pow(argn(argc, argv, 0), argn(argc, argv, 1))); }
static Value math_max(Act&, Value, int argc, Value* argv) {
    if (argc == 0) return Value(-INFINITY); double r = -INFINITY;
    for (int i = 0; i < argc; i++) { double x = toNum(argv[i]); if (std::isnan(x)) return Value(NAN); if (x > r) r = x; } return Value(r);
}
static Value math_min(Act&, Value, int argc, Value* argv) {
    if (argc == 0) return Value(INFINITY); double r = INFINITY;
    for (int i = 0; i < argc; i++) { double x = toNum(argv[i]); if (std::isnan(x)) return Value(NAN); if (x < r) r = x; } return Value(r);
}
static Value math_random(Act&, Value, int, Value*) { return Value(randomDouble()); }

// ============================================================ Miscellaneous globals
static Value g_isNaN(Act&, Value, int argc, Value* argv) { return VB(std::isnan(argn(argc, argv, 0))); }
static Value g_isFinite(Act&, Value, int argc, Value* argv) { return VB(std::isfinite(argn(argc, argv, 0))); }
static Value g_parseInt(Act&, Value, int argc, Value* argv) {
    std::string s = toStr(arg(argc, argv, 0))->s; int radix = argc > 1 ? toInt32(argv[1]) : 0;
    const char* p = s.c_str(); while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
    bool neg = false; if (*p == '-' || *p == '+') { neg = *p == '-'; p++; }
    if ((radix == 0 || radix == 16) && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) { radix = 16; p += 2; }
    if (radix == 0) radix = (p[0] == '0' && p[1]) ? 8 : 10;
    double v = 0; bool any = false;
    for (; *p; p++) { int d; char c = *p; if (c >= '0' && c <= '9') d = c - '0'; else if (c >= 'a' && c <= 'z') d = c - 'a' + 10; else if (c >= 'A' && c <= 'Z') d = c - 'A' + 10; else break; if (d >= radix) break; v = v * radix + d; any = true; }
    return Value(any ? (neg ? -v : v) : NAN);
}
static Value g_parseFloat(Act&, Value, int argc, Value* argv) { std::string s = toStr(arg(argc, argv, 0))->s; char* e; double v = strtod(s.c_str(), &e); return Value(e == s.c_str() ? NAN : v); }
static Value g_traceFn(Act&, Value, int argc, Value* argv) { logf("[trace] %s\n", toStr(arg(argc, argv, 0))->s.c_str()); return VUNDEF; }
static Value g_noop(Act&, Value, int, Value*) { return VUNDEF; }
static Value g_getTimer(Act&, Value, int, Value*) { return getTime(); }
static Value g_escape(Act&, Value, int argc, Value* argv) {
    std::string s = toStr(arg(argc, argv, 0))->s, o; char b[4];
    for (unsigned char c : s) { if (isalnum(c)) o += (char)c; else { snprintf(b, sizeof b, "%%%02X", c); o += b; } }
    return Value(mkstr(o));
}
static Value g_unescape(Act&, Value, int argc, Value* argv) {
    std::string s = toStr(arg(argc, argv, 0))->s, o;
    for (size_t i = 0; i < s.size(); i++) { if (s[i] == '%' && i + 2 < s.size()) { o += (char)strtol(s.substr(i + 1, 2).c_str(), nullptr, 16); i += 2; } else o += s[i]; }
    return Value(mkstr(o));
}

// setInterval / clearInterval
struct Interval { int id; Value fn; Value obj; Str* method; double period, next; std::vector<Value> args; };
static std::vector<Interval> g_intervals; static int g_intervalId = 0;
static Value g_setInterval(Act&, Value, int argc, Value* argv) {
    Interval iv; iv.id = ++g_intervalId; iv.method = nullptr;
    int k = 0;
    if (argc >= 3 && argv[0].isObj() && argv[0].o->kind != OK_FUNCTION) { iv.obj = argv[0]; iv.method = keyOf(argv[1]); k = 2; }
    else { iv.fn = arg(argc, argv, 0); k = 1; }
    iv.period = std::max(1.0, argn(argc, argv, k, 1)); iv.next = g_player.timeMs + iv.period;
    for (int i = k + 1; i < argc; i++) iv.args.push_back(argv[i]);
    g_intervals.push_back(iv); return Value((double)iv.id);
}
static Value g_clearInterval(Act&, Value, int argc, Value* argv) {
    int id = toInt32(arg(argc, argv, 0));
    g_intervals.erase(std::remove_if(g_intervals.begin(), g_intervals.end(), [&](const Interval& i) { return i.id == id; }), g_intervals.end());
    return VUNDEF;
}
void runIntervals() {
    for (size_t i = 0; i < g_intervals.size(); i++) {
        if (g_player.timeMs < g_intervals[i].next) continue;
        Interval iv = g_intervals[i];
        g_intervals[i].next = g_player.timeMs + iv.period;
        Act a{vm.globalScope, VUNDEF, vm.root, nullptr};
        if (iv.method) callMethodK(a, iv.obj, iv.method, (int)iv.args.size(), iv.args.data());
        else callValue(a, iv.fn, VUNDEF, (int)iv.args.size(), iv.args.data());
        runActions();
    }
}

// ============================================================ Key / Mouse
static Value key_isDown(Act&, Value, int argc, Value* argv) { int c = toInt32(arg(argc, argv, 0)); return VB(c > 0 && c < 256 && g_player.keys[c]); }
static Value key_getCode(Act&, Value, int, Value*) { return Value((double)g_player.lastKey); }
static Value key_getAscii(Act&, Value, int, Value*) { return Value((double)g_player.lastAscii); }
static void addListener(std::vector<Object*>& v, const Value& o) { if (o.isObj() && std::find(v.begin(), v.end(), o.o) == v.end()) v.push_back(o.o); }
static void removeListener(std::vector<Object*>& v, const Value& o) { if (o.isObj()) v.erase(std::remove(v.begin(), v.end(), o.o), v.end()); }
static Value key_addL(Act&, Value, int argc, Value* argv) { addListener(g_player.keyListeners, arg(argc, argv, 0)); return VTRUE; }
static Value key_remL(Act&, Value, int argc, Value* argv) { removeListener(g_player.keyListeners, arg(argc, argv, 0)); return VTRUE; }
static Value mouse_addL(Act&, Value, int argc, Value* argv) { addListener(g_player.mouseListeners, arg(argc, argv, 0)); return VTRUE; }
// Mouse.removeListener() with no argument: the game calls it at the start of each room to remove the previous
// room's listener. In Flash it removes nothing, so each room left its whole clip (with its 6.7 MB layers) alive
// forever; here it removes all of them, as the game intended.
static Value mouse_remL(Act&, Value, int argc, Value* argv) { if (argc == 0) { g_player.mouseListeners.clear(); return VTRUE; } removeListener(g_player.mouseListeners, arg(argc, argv, 0)); return VTRUE; }
bool g_mouseHidden = false;
static Value mouse_hide(Act&, Value, int, Value*) { g_mouseHidden = true; return Value(1.0); }
static Value mouse_show(Act&, Value, int, Value*) { g_mouseHidden = false; return Value(1.0); }

// ============================================================ MovieClip
static Value mc_gotoAndStop(Act&, Value t, int argc, Value* argv) {
    MovieClip* c = asClip(t); if (!c || argc < 1) return VUNDEF;
    MovieClip* tg; int f = frameFromValue(c, argv[0], &tg); if (f > 0) gotoFrame(tg, f, true); return VUNDEF;
}
static Value mc_gotoAndPlay(Act&, Value t, int argc, Value* argv) {
    MovieClip* c = asClip(t); if (!c || argc < 1) return VUNDEF;
    MovieClip* tg; int f = frameFromValue(c, argv[0], &tg); if (f > 0) gotoFrame(tg, f, false); return VUNDEF;
}
static Value mc_play(Act&, Value t, int, Value*) { if (MovieClip* c = asClip(t)) c->playing = true; return VUNDEF; }
static Value mc_stop(Act& a, Value t, int, Value*) { if (MovieClip* c = asClip(t)) { Act b = a; b.target = c; actStop(b); } return VUNDEF; }
static Value mc_nextFrame(Act& a, Value t, int, Value*) { if (MovieClip* c = asClip(t)) { Act b = a; b.target = c; actNextFrame(b); } return VUNDEF; }
static Value mc_prevFrame(Act& a, Value t, int, Value*) { if (MovieClip* c = asClip(t)) { Act b = a; b.target = c; actPrevFrame(b); } return VUNDEF; }
static Value mc_attachMovie(Act&, Value t, int argc, Value* argv) {
    MovieClip* c = asClip(t); if (!c || argc < 3) return VUNDEF;
    Object* init = argc > 3 && argv[3].isObj() ? argv[3].o : nullptr;
    MovieClip* r = attachMovie(c, toStr(argv[0])->s, atomOf(toStr(argv[1])), toInt32(argv[2]), init);
    return r ? Value((Object*)r) : VUNDEF;
}
static Value mc_createEmpty(Act&, Value t, int argc, Value* argv) {
    MovieClip* c = asClip(t); if (!c || argc < 2) return VUNDEF;
    return Value((Object*)createEmptyMovieClip(c, atomOf(toStr(argv[0])), toInt32(argv[1])));
}
static Value mc_remove(Act&, Value t, int, Value*) { if (DisplayObject* d = asDO(t)) removeMovieClip(d); return VUNDEF; }
static Value mc_swapDepths(Act&, Value t, int argc, Value* argv) { if (DisplayObject* d = asDO(t)) swapDepths(d, arg(argc, argv, 0)); return VUNDEF; }
static Value mc_getDepth(Act&, Value t, int, Value*) { DisplayObject* d = asDO(t); return d ? Value((double)(d->depth - DEPTH_BIAS)) : VUNDEF; }
static Value mc_nextDepth(Act&, Value t, int, Value*) {
    MovieClip* c = asClip(t); if (!c) return VUNDEF; int m = 0;
    for (auto* k : c->kids) if (k->depth - DEPTH_BIAS + 1 > m) m = k->depth - DEPTH_BIAS + 1;
    return Value((double)m);
}
static Value mc_hitTest(Act& a, Value t, int argc, Value* argv) {
    DisplayObject* d = asDO(t); if (!d) return VFALSE;
    if (argc >= 2) {
        // The docs say "Stage coordinates", but Flash Player actually takes the point in _root coordinates
        // (Ruffle does the same). Isaac scales _root (x2.25 on PC, x1.5 on Switch), so without this the
        // creep/blood test (shit() -> blorz.hitTest) checked a point 1.5-2.25x away from the player.
        double gx = toNum(argv[0]), gy = toNum(argv[1]);
        if (!std::isfinite(gx) || !std::isfinite(gy)) return VFALSE;
        double ox = gx, oy = gy;
        if (vm.root) vm.root->m.apply(gx, gy, gx, gy);
        bool r = hitTestPoint(d, gx, gy, argc > 2 && truthy(argv[2]));
        static const bool trace = getenv("ISAAC_HITTRACE") != nullptr;
        if (trace) logf("[hit] t=%u %s %s %s %s = %s\n", g_player.tick, targetPath(d)->s.c_str(), toStr(Value(ox))->s.c_str(), toStr(Value(oy))->s.c_str(), argc > 2 && truthy(argv[2]) ? "true" : "false", r ? "true" : "false");
        return VB(r);
    }
    Value o = resolveTargetPath(a, arg(argc, argv, 0));
    DisplayObject* e = asDO(o); if (!e) return VFALSE;
    RectF b1, b2;
    Mat w1 = worldMatrix(d), w2 = worldMatrix(e);
    RectF l1 = localBounds(d), l2 = localBounds(e);
    (void)l1; (void)l2;
    // bounds in global coordinates
    auto glob = [](DisplayObject* x) { RectF r = localBounds(x); Mat pm = x->parent ? worldMatrix(x->parent) : Mat(); RectF o;
        double xs[4] = {r.x0, r.x1, r.x0, r.x1}, ys[4] = {r.y0, r.y0, r.y1, r.y1}; bool f = true;
        for (int i = 0; i < 4; i++) { double gx, gy; pm.apply(xs[i], ys[i], gx, gy); if (f) { o.x0 = o.x1 = gx; o.y0 = o.y1 = gy; f = false; } else { o.x0 = std::min(o.x0, gx); o.x1 = std::max(o.x1, gx); o.y0 = std::min(o.y0, gy); o.y1 = std::max(o.y1, gy); } }
        return o; };
    (void)w1; (void)w2;
    b1 = glob(d); b2 = glob(e);
    return VB(b1.x0 <= b2.x1 && b2.x0 <= b1.x1 && b1.y0 <= b2.y1 && b2.y0 <= b1.y1);
}
static Value mc_attachBitmap(Act&, Value t, int argc, Value* argv) {
    MovieClip* c = asClip(t); if (!c || argc < 2) return VUNDEF;
    Value b = argv[0]; if (!b.isObj() || b.o->kind != OK_BITMAPDATA) return VUNDEF;
    BitmapObj* bo = gcNew<BitmapObj>(); bo->kind = OK_BITMAP; bo->proto = protos::object; bo->bd = (BitmapData*)b.o;
    bo->bd->displayed = true;   // displayed layer: whatever is drawn into it can be done on the GPU
    bo->smoothing = argc > 3 && truthy(argv[3]);
    int depth = toInt32(argv[1]) + DEPTH_BIAS;
    if (DisplayObject* old = c->childAtDepth(depth)) removeObject(old);
    bo->depth = depth; bo->placedByScript = true; c->insertChild(bo);
    return VUNDEF;
}
static Value mc_bytes(Act&, Value, int, Value*) { return Value((double)g_movie.data.size()); }
static Value mc_getBounds(Act&, Value t, int, Value*) {
    DisplayObject* d = asDO(t); Object* o = gcNew<Object>(protos::object); if (!d) return Value(o);
    RectF r; Mat save = d->m; d->m = Mat(); r = localBounds(d); d->m = save;
    setProp(o, intern("xMin"), Value(r.x0)); setProp(o, intern("xMax"), Value(r.x1)); setProp(o, intern("yMin"), Value(r.y0)); setProp(o, intern("yMax"), Value(r.y1));
    return Value(o);
}
static Value mc_localToGlobal(Act&, Value t, int argc, Value* argv) {
    DisplayObject* d = asDO(t); Value p = arg(argc, argv, 0); if (!d || !p.isObj()) return VUNDEF;
    Mat w = worldMatrix(d); double x, y; w.apply(toNum(getProp(p.o, intern("x"))), toNum(getProp(p.o, intern("y"))), x, y);
    setProp(p.o, intern("x"), Value(x)); setProp(p.o, intern("y"), Value(y)); return VUNDEF;
}
static Value mc_globalToLocal(Act&, Value t, int argc, Value* argv) {
    DisplayObject* d = asDO(t); Value p = arg(argc, argv, 0); if (!d || !p.isObj()) return VUNDEF;
    Mat w = worldMatrix(d), inv; if (!w.invert(inv)) return VUNDEF;
    double x, y; inv.apply(toNum(getProp(p.o, intern("x"))), toNum(getProp(p.o, intern("y"))), x, y);
    setProp(p.o, intern("x"), Value(x)); setProp(p.o, intern("y"), Value(y)); return VUNDEF;
}

// ============================================================ BitmapData
void bitmapDraw(BitmapData* bd, Object* src, const Mat& m, const CX* cx);   // raster.cpp
static BitmapData* asBD(const Value& v) { return v.isObj() && v.o->kind == OK_BITMAPDATA ? (BitmapData*)v.o : nullptr; }
// script colour (ARGB) -> stored pixel (premultiplied, in memory order: see kPxABGR), and back
static uint32_t premul(uint32_t argb) {
    uint32_t a = argb >> 24; if (a == 255) return pxFromArgb(argb); if (a == 0) return 0;
    uint32_t r = ((argb >> 16) & 255) * a / 255, g = ((argb >> 8) & 255) * a / 255, b = (argb & 255) * a / 255;
    return pxFromArgb(a << 24 | r << 16 | g << 8 | b);
}
static uint32_t unpremul(uint32_t p) {
    p = pxToArgb(p);
    uint32_t a = p >> 24; if (a == 255 || a == 0) return a == 0 ? 0 : p;
    uint32_t r = std::min<uint32_t>(255u, ((p >> 16) & 255) * 255 / a), g = std::min<uint32_t>(255u, ((p >> 8) & 255) * 255 / a), b = std::min<uint32_t>(255u, (p & 255) * 255 / a);
    return a << 24 | r << 16 | g << 8 | b;
}
static Value bd_ctor(Act&, Value, int argc, Value* argv) {
    BitmapData* b = gcNew<BitmapData>(); b->proto = protos::bitmapdata;
    b->w = std::max(0, std::min(8191, toInt32(arg(argc, argv, 0)))); b->h = std::max(0, std::min(8191, toInt32(arg(argc, argv, 1))));
    b->transparent = argc > 2 ? truthy(argv[2]) : true;
    uint32_t fill = argc > 3 ? (uint32_t)toInt32(argv[3]) : 0xffffffffu;
    if (!b->transparent) fill |= 0xff000000u;
    size_t n = (size_t)b->w * b->h;
    b->lazy = true; b->lazyFill = premul(fill);   // (pixels are allocated only if the CPU needs them)
    gcExternalAlloc(n * 4 * 2);   // CPU memory + GPU texture
    b->dirty(0, 0, b->w, b->h);
    return Value((Object*)b);
}
static Value bd_getPixel(Act&, Value t, int argc, Value* argv) {
    BitmapData* b = asBD(t); if (!b) return VUNDEF; int x = toInt32(arg(argc, argv, 0)), y = toInt32(arg(argc, argv, 1));
    if (x < 0 || y < 0 || x >= b->w || y >= b->h) return Value(0.0);
    if (!b->cpuRead) bdNeedCpu(b);
    b->ensurePx();
    return Value((double)(unpremul(b->px[(size_t)y * b->w + x]) & 0xffffff));
}
static Value bd_getPixel32(Act&, Value t, int argc, Value* argv) {
    BitmapData* b = asBD(t); if (!b) return VUNDEF; int x = toInt32(arg(argc, argv, 0)), y = toInt32(arg(argc, argv, 1));
    if (x < 0 || y < 0 || x >= b->w || y >= b->h) return Value(0.0);
    if (!b->cpuRead) bdNeedCpu(b);
    b->ensurePx();
    return Value((double)(int32_t)unpremul(b->px[(size_t)y * b->w + x]));
}
static Value bd_setPixel(Act&, Value t, int argc, Value* argv) {
    BitmapData* b = asBD(t); if (!b) return VUNDEF; int x = toInt32(arg(argc, argv, 0)), y = toInt32(arg(argc, argv, 1));
    if (x < 0 || y < 0 || x >= b->w || y >= b->h) return VUNDEF;
    if (!b->cpuRead) bdNeedCpu(b);
    b->ensurePx();
    uint32_t c = (uint32_t)toInt32(arg(argc, argv, 2)) & 0xffffff;
    uint32_t& p = b->px[(size_t)y * b->w + x];
    uint32_t a = b->transparent ? (unpremul(p) >> 24) : 255;
    p = premul(a << 24 | c); b->dirty(x, y, x + 1, y + 1); return VUNDEF;
}
static Value bd_setPixel32(Act&, Value t, int argc, Value* argv) {
    BitmapData* b = asBD(t); if (!b) return VUNDEF; int x = toInt32(arg(argc, argv, 0)), y = toInt32(arg(argc, argv, 1));
    if (x < 0 || y < 0 || x >= b->w || y >= b->h) return VUNDEF;
    if (!b->cpuRead) bdNeedCpu(b);
    b->ensurePx();
    uint32_t c = (uint32_t)toInt32(arg(argc, argv, 2)); if (!b->transparent) c |= 0xff000000u;
    b->px[(size_t)y * b->w + x] = premul(c); b->dirty(x, y, x + 1, y + 1); return VUNDEF;
}
static bool rectArg(const Value& r, int& x, int& y, int& w, int& h) {
    if (!r.isObj()) return false;
    x = (int)std::floor(toNum(getProp(r.o, intern("x")))); y = (int)std::floor(toNum(getProp(r.o, intern("y"))));
    w = (int)std::ceil(toNum(getProp(r.o, intern("width")))); h = (int)std::ceil(toNum(getProp(r.o, intern("height"))));
    return true;
}
static Value bd_fillRect(Act&, Value t, int argc, Value* argv) {
    BitmapData* b = asBD(t); int x, y, w, h; if (!b || !rectArg(arg(argc, argv, 0), x, y, w, h)) return VUNDEF;
    uint32_t c = (uint32_t)toInt32(arg(argc, argv, 1)); if (!b->transparent) c |= 0xff000000u; c = premul(c);
    int x0 = std::max(0, x), y0 = std::max(0, y), x1 = std::min(b->w, x + w), y1 = std::min(b->h, y + h);
    if (bdUseGpu(b)) { gpuFill(b, x0, y0, x1, y1, c); return VUNDEF; }
    if (b->lazy && x0 == 0 && y0 == 0 && x1 == b->w && y1 == b->h) { b->lazyFill = c; b->dirty(0, 0, b->w, b->h); return VUNDEF; }
    b->ensurePx();
#if defined(__PSP__) || defined(FL_LEAN)   // (FL_LEAN: tested on PC against Ruffle)
    if (x0 == 0 && y0 == 0 && x1 == b->w && y1 == b->h && b->h <= 4096 && b->w < 65536) {
        bool known = b->fillKnown && b->fillVal == c && (int)b->rowSpan.size() == b->h;
        b->fillKnown = false;   // (dirty() below doesn't track spans)
        if (known) {   // only what changed since the last clear
            int dy0 = -1, dy1 = -1, dxa = b->w, dxb = 0;
            for (int yy = 0; yy < b->h; yy++) {
                uint32_t sp = b->rowSpan[yy]; int a = sp & 0xffff, e = sp >> 16;
                if (e <= a) continue;
                std::fill(b->px.begin() + (size_t)yy * b->w + a, b->px.begin() + (size_t)yy * b->w + e, c);
                if (dy0 < 0) dy0 = yy; dy1 = yy + 1; dxa = std::min(dxa, a); dxb = std::max(dxb, e);
            }
            if (dy0 >= 0) b->dirty(dxa, dy0, dxb, dy1);
        } else {
            std::fill(b->px.begin(), b->px.end(), c);
            b->dirty(0, 0, b->w, b->h);
        }
        b->rowSpan.assign(b->h, 0); b->fillKnown = true; b->fillVal = c;
        return VUNDEF;
    }
#endif
    for (int yy = y0; yy < y1; yy++) std::fill(b->px.begin() + (size_t)yy * b->w + x0, b->px.begin() + (size_t)yy * b->w + x1, c);
    b->dirty(x0, y0, x1, y1); return VUNDEF;
}
static Value bd_copyPixels(Act&, Value t, int argc, Value* argv) {
    BitmapData* d = asBD(t); BitmapData* s = asBD(arg(argc, argv, 0)); int x, y, w, h;
    if (!d || !s || !rectArg(arg(argc, argv, 1), x, y, w, h)) return VUNDEF;
    Value pt = arg(argc, argv, 2); int dx = 0, dy = 0;
    if (pt.isObj()) { dx = (int)toNum(getProp(pt.o, intern("x"))); dy = (int)toNum(getProp(pt.o, intern("y"))); }
    bool merge = argc > 5 && truthy(argv[5]);
    if (!s->cpuRead) bdNeedCpu(s);   // (its pixels are read)
    s->ensurePx();
    if (s != d && bdUseGpu(d)) { gpuCopy(d, s, x, y, w, h, dx, dy, merge); return VUNDEF; }
    d->ensurePx();
    for (int yy = 0; yy < h; yy++) {
        int sy = y + yy, ty = dy + yy; if (sy < 0 || sy >= s->h || ty < 0 || ty >= d->h) continue;
        for (int xx = 0; xx < w; xx++) {
            int sx = x + xx, tx = dx + xx; if (sx < 0 || sx >= s->w || tx < 0 || tx >= d->w) continue;
            uint32_t sp = s->px[(size_t)sy * s->w + sx]; uint32_t& dp = d->px[(size_t)ty * d->w + tx];
            if (merge && d->transparent) {
                uint32_t sa = sp >> 24, ia = 255 - sa;
                uint32_t r = ((sp >> 16) & 255) + ((dp >> 16) & 255) * ia / 255, g = ((sp >> 8) & 255) + ((dp >> 8) & 255) * ia / 255,
                         bb = (sp & 255) + (dp & 255) * ia / 255, a = sa + (dp >> 24) * ia / 255;
                dp = std::min<uint32_t>(a, 255u) << 24 | std::min<uint32_t>(r, 255u) << 16 | std::min<uint32_t>(g, 255u) << 8 | std::min<uint32_t>(bb, 255u);
            } else dp = d->transparent ? sp : (sp | 0xff000000u);
        }
    }
    d->dirty(dx, dy, dx + w, dy + h); return VUNDEF;
}
static Mat matFromObj(const Value& v) {
    Mat m; if (!v.isObj()) return m;
    Object* o = v.o;
    m.a = toNum(getProp(o, intern("a"))); m.b = toNum(getProp(o, intern("b"))); m.c = toNum(getProp(o, intern("c")));
    m.d = toNum(getProp(o, intern("d"))); m.tx = toNum(getProp(o, intern("tx"))); m.ty = toNum(getProp(o, intern("ty")));
    if (std::isnan(m.a)) m.a = 1; if (std::isnan(m.b)) m.b = 0; if (std::isnan(m.c)) m.c = 0; if (std::isnan(m.d)) m.d = 1;
    if (std::isnan(m.tx)) m.tx = 0; if (std::isnan(m.ty)) m.ty = 0;
    return m;
}
static CX cxFromObj(Object* o) {
    CX c;
    auto g = [&](const char* n, float def) { Value v = getProp(o, intern(n)); double d = toNum(v); return std::isnan(d) ? def : (float)d; };
    c.rm = g("redMultiplier", 1); c.gm = g("greenMultiplier", 1); c.bm = g("blueMultiplier", 1); c.am = g("alphaMultiplier", 1);
    c.ra = g("redOffset", 0); c.ga = g("greenOffset", 0); c.ba = g("blueOffset", 0); c.aa = g("alphaOffset", 0);
    return c;
}
static Value bd_draw(Act&, Value t, int argc, Value* argv) {
    BitmapData* b = asBD(t); Value src = arg(argc, argv, 0); if (!b || !src.isObj()) return VUNDEF;
    Object* so = src.o;
    if (so->isDisplay()) { DisplayObject* d = asDO(src); if (!d) return VUNDEF; so = d; }
    Mat m = argc > 1 && argv[1].isObj() ? matFromObj(argv[1]) : Mat();
    CX cx; bool hasCx = argc > 2 && argv[2].isObj();
    if (hasCx) cx = cxFromObj(argv[2].o);
    if (so->kind == OK_BITMAPDATA && !((BitmapData*)so)->gpu) bdNeedCpu((BitmapData*)so);   // (source is read on the CPU)
    if (so->kind == OK_BITMAPDATA) ((BitmapData*)so)->ensurePx();
    if (so != b && bdUseGpu(b)) { gpuDraw(b, so, m, hasCx ? &cx : nullptr); return VUNDEF; }
    b->ensurePx();
    bitmapDraw(b, so, m, hasCx ? &cx : nullptr);
    return VUNDEF;
}
static Value bd_dispose(Act&, Value t, int, Value*) {
    if (BitmapData* b = asBD(t)) {
        extern void bitmapGone(BitmapData*);
        b->disposed = true; b->lazy = false; bdPoolGive(b->px); b->w = b->h = 0; b->version++;
        bitmapGone(b);
    }
    return VUNDEF;
}
static Value bd_clone(Act&, Value t, int, Value*) {
    BitmapData* s = asBD(t); if (!s) return VUNDEF;
    if (!s->cpuRead) bdNeedCpu(s);
    s->ensurePx();
    BitmapData* b = gcNew<BitmapData>(); b->proto = protos::bitmapdata; b->w = s->w; b->h = s->h; b->transparent = s->transparent;
    bdPoolTake(b->px, s->px.size()); b->px.assign(s->px.begin(), s->px.end()); gcExternalAlloc(s->px.size() * 4 * 2); b->dirty(0, 0, b->w, b->h);
    return Value((Object*)b);
}

// ============================================================ geom
static Value rect_ctor(Act&, Value, int argc, Value* argv) {
    Object* o = gcNew<Object>(protos::rectangle);
    setProp(o, intern("x"), Value(argn(argc, argv, 0, 0))); setProp(o, intern("y"), Value(argn(argc, argv, 1, 0)));
    setProp(o, intern("width"), Value(argn(argc, argv, 2, 0))); setProp(o, intern("height"), Value(argn(argc, argv, 3, 0)));
    return Value(o);
}
static Value point_ctor(Act&, Value, int argc, Value* argv) {
    Object* o = gcNew<Object>(protos::point);
    setProp(o, intern("x"), Value(argn(argc, argv, 0, 0))); setProp(o, intern("y"), Value(argn(argc, argv, 1, 0)));
    return Value(o);
}
static void setMat(Object* o, const Mat& m) {
    setProp(o, intern("a"), Value(m.a)); setProp(o, intern("b"), Value(m.b)); setProp(o, intern("c"), Value(m.c));
    setProp(o, intern("d"), Value(m.d)); setProp(o, intern("tx"), Value(m.tx)); setProp(o, intern("ty"), Value(m.ty));
}
static Value matrix_ctor(Act&, Value, int argc, Value* argv) {
    Object* o = gcNew<Object>(protos::matrix);
    Mat m; if (argc >= 1) m.a = argn(argc, argv, 0, 1); if (argc >= 2) m.b = argn(argc, argv, 1, 0); if (argc >= 3) m.c = argn(argc, argv, 2, 0);
    if (argc >= 4) m.d = argn(argc, argv, 3, 1); if (argc >= 5) m.tx = argn(argc, argv, 4, 0); if (argc >= 6) m.ty = argn(argc, argv, 5, 0);
    setMat(o, m); return Value(o);
}
static Value matrix_translate(Act&, Value t, int argc, Value* argv) { if (!t.isObj()) return VUNDEF; Mat m = matFromObj(t); m.tx += argn(argc, argv, 0, 0); m.ty += argn(argc, argv, 1, 0); setMat(t.o, m); return VUNDEF; }
static Value matrix_scale(Act&, Value t, int argc, Value* argv) {
    if (!t.isObj()) return VUNDEF; Mat m = matFromObj(t); double sx = argn(argc, argv, 0, 1), sy = argn(argc, argv, 1, 1);
    Mat s; s.a = sx; s.d = sy; setMat(t.o, s * m); return VUNDEF;
}
static Value matrix_rotate(Act&, Value t, int argc, Value* argv) {
    if (!t.isObj()) return VUNDEF; Mat m = matFromObj(t); double r = argn(argc, argv, 0, 0);
    Mat s; s.a = std::cos(r); s.b = std::sin(r); s.c = -std::sin(r); s.d = std::cos(r); setMat(t.o, s * m); return VUNDEF;
}
static Value matrix_identity(Act&, Value t, int, Value*) { if (t.isObj()) setMat(t.o, Mat()); return VUNDEF; }
static Value matrix_concat(Act&, Value t, int argc, Value* argv) { if (!t.isObj()) return VUNDEF; Mat m = matFromObj(t), o = matFromObj(arg(argc, argv, 0)); setMat(t.o, o * m); return VUNDEF; }
static Value matrix_clone(Act&, Value t, int, Value*) { Object* o = gcNew<Object>(protos::matrix); setMat(o, matFromObj(t)); return Value(o); }

static void setCxProps(Object* o, const CX& c) {
    setProp(o, intern("redMultiplier"), Value((double)c.rm)); setProp(o, intern("greenMultiplier"), Value((double)c.gm));
    setProp(o, intern("blueMultiplier"), Value((double)c.bm)); setProp(o, intern("alphaMultiplier"), Value((double)c.am));
    setProp(o, intern("redOffset"), Value((double)c.ra)); setProp(o, intern("greenOffset"), Value((double)c.ga));
    setProp(o, intern("blueOffset"), Value((double)c.ba)); setProp(o, intern("alphaOffset"), Value((double)c.aa));
}
static Value ct_ctor(Act&, Value, int argc, Value* argv) {
    Object* o = gcNew<Object>(protos::colortransform);
    CX c; c.rm = (float)argn(argc, argv, 0, 1); c.gm = (float)argn(argc, argv, 1, 1); c.bm = (float)argn(argc, argv, 2, 1); c.am = (float)argn(argc, argv, 3, 1);
    c.ra = (float)argn(argc, argv, 4, 0); c.ga = (float)argn(argc, argv, 5, 0); c.ba = (float)argn(argc, argv, 6, 0); c.aa = (float)argn(argc, argv, 7, 0);
    setCxProps(o, c); return Value(o);
}
struct TransformObj final : Object { DisplayObject* target = nullptr; void trace(Tracer& tr) override { Object::trace(tr); tr.mark(target); } };
static Value transform_ctor(Act&, Value, int argc, Value* argv) {
    TransformObj* t = gcNew<TransformObj>(); t->kind = OK_TRANSFORM; t->proto = protos::transform; t->target = asDO(arg(argc, argv, 0));
    return Value((Object*)t);
}

// ============================================================ Sound
struct SoundObj final : Object {
    DisplayObject* target = nullptr; SoundDef* def = nullptr; float vol = 100, pan = 0;
    // Flash keeps `position` after the sound ends (== duration) or is stopped: the games remove each
    // sound's clip when position == duration, so returning 0 there leaked one clip per sound played
    bool started = false, stopped = false; double lastPos = 0;
    void trace(Tracer& tr) override { Object::trace(tr); tr.mark(target); }
};
float g_masterVolume = 1.0f;
// length in ms from the SWF's sample count and its declared rate (5.5/11/22/44 kHz)
static double soundDurationMs(const SoundDef* d) {
    if (!d) return 0;
    static const double kRates[4] = {5512.5, 11025, 22050, 44100};
    return std::floor(d->samples * 1000.0 / kRates[d->rate & 3]);
}
static SoundObj* asSnd(const Value& v) { return v.isObj() && v.o->kind == OK_SOUND ? (SoundObj*)v.o : nullptr; }
static Value snd_ctor(Act&, Value, int argc, Value* argv) {
    SoundObj* s = gcNew<SoundObj>(); s->kind = OK_SOUND; s->proto = protos::sound; s->target = asDO(arg(argc, argv, 0));
    return Value((Object*)s);
}
static Value snd_attach(Act&, Value t, int argc, Value* argv) {
    SoundObj* s = asSnd(t); if (!s) return VUNDEF;
    auto it = g_movie.exports.find(toStr(arg(argc, argv, 0))->s);
    if (it != g_movie.exports.end()) { CharDef* cd = g_movie.get(it->second); if (cd && cd->type == CH_SOUND) s->def = (SoundDef*)cd; }
    return VUNDEF;
}
static Value snd_start(Act&, Value t, int argc, Value* argv) {
    SoundObj* s = asSnd(t); if (!s || !s->def) return VUNDEF;
    double off = argn(argc, argv, 0, 0); if (std::isnan(off)) off = 0;
    double lp = argn(argc, argv, 1, 1); int loops = std::isnan(lp) || lp < 1 ? 1 : (int)std::min(lp, 1e6);
    soundStart(s->def, off, loops, s, s->vol / 100.f, s->pan / 100.f);
    s->started = true; s->stopped = false; s->lastPos = std::floor(off * 1000.0);
    return VUNDEF;
}
static Value snd_stop(Act&, Value t, int argc, Value* argv) {
    SoundObj* s = asSnd(t); if (!s) return VUNDEF;
    if (argc > 0) {
        auto it = g_movie.exports.find(toStr(argv[0])->s);
        if (it != g_movie.exports.end()) { extern void soundStopDef(SoundDef*); CharDef* cd = g_movie.get(it->second); if (cd && cd->type == CH_SOUND) soundStopDef((SoundDef*)cd); }
        return VUNDEF;
    }
    if (!s->target && !s->def) soundStopAll();
    else {
        bool act = false; double ms = soundPositionMs(s, &act);
        if (act) s->lastPos = std::floor(ms);
        s->stopped = true;
        soundStopOwner(s);
    }
    return VUNDEF;
}
static Value snd_setVolume(Act&, Value t, int argc, Value* argv) {
    SoundObj* s = asSnd(t); if (!s) return VUNDEF;
    double v = argn(argc, argv, 0, 100); if (std::isnan(v)) return VUNDEF;
    s->vol = (float)v;
    if (!s->target && !s->def) g_masterVolume = (float)(v / 100.0);
    soundSetVolumeOwner(s, s->vol / 100.f, s->pan / 100.f);
    return VUNDEF;
}
static Value snd_getVolume(Act&, Value t, int, Value*) { SoundObj* s = asSnd(t); return s ? Value((double)s->vol) : VUNDEF; }
static Value snd_setPan(Act&, Value t, int argc, Value* argv) { SoundObj* s = asSnd(t); if (s) { s->pan = (float)argn(argc, argv, 0, 0); soundSetVolumeOwner(s, s->vol / 100.f, s->pan / 100.f); } return VUNDEF; }
static Value snd_getPan(Act&, Value t, int, Value*) { SoundObj* s = asSnd(t); return s ? Value((double)s->pan) : VUNDEF; }

// ============================================================ SharedObject (disk persistence)
static void ser(std::string& o, const Value& v, int depth) {
    char b[64];
    switch (v.t) {
        case T_NUM: snprintf(b, sizeof b, "n%.17g;", v.n); o += b; break;
        case T_BOOL: o += v.b ? "t" : "f"; break;
        case T_NULL: o += "z"; break;
        case T_STR: snprintf(b, sizeof b, "s%zu:", v.s->s.size()); o += b; o += v.s->s; break;
        case T_OBJ: {
            if (depth > 32 || v.o->isDisplay() || v.o->kind == OK_FUNCTION) { o += "u"; break; }
            if (v.o->kind == OK_ARRAY) {
                Array* a = (Array*)v.o; snprintf(b, sizeof b, "a%zu:", a->el.size()); o += b;
                for (auto& e : a->el) ser(o, e.t == T_HOLE ? VUNDEF : e, depth + 1);
            } else {
                snprintf(b, sizeof b, "o%zu:", v.o->props.v.size()); o += b;
                for (auto& p : v.o->props.v) { snprintf(b, sizeof b, "%zu:", p.k->s.size()); o += b; o += p.k->s; ser(o, p.v, depth + 1); }
            }
            break;
        }
        default: o += "u"; break;
    }
}
static Value deser(const std::string& s, size_t& i, int depth) {
    if (i >= s.size() || depth > 40) return VUNDEF;
    char c = s[i++];
    auto num = [&]() { size_t e = s.find_first_of(":;", i); size_t n = strtoull(s.substr(i, e - i).c_str(), nullptr, 10); i = e + 1; return n; };
    switch (c) {
        case 'n': { size_t e = s.find(';', i); double d = strtod(s.substr(i, e - i).c_str(), nullptr); i = e + 1; return Value(d); }
        case 't': return VTRUE; case 'f': return VFALSE; case 'z': return VNULL;
        case 's': { size_t n = num(); std::string v = s.substr(i, n); i += n; return Value(mkstr(v)); }
        case 'a': { size_t n = num(); Array* a = gcNew<Array>(); for (size_t k = 0; k < n; k++) a->el.push_back(deser(s, i, depth + 1)); return Value((Object*)a); }
        case 'o': { size_t n = num(); Object* o = gcNew<Object>(protos::object);
            for (size_t k = 0; k < n; k++) { size_t kl = num(); std::string key = s.substr(i, kl); i += kl; Value v = deser(s, i, depth + 1); setProp(o, intern(key), v); }
            return Value(o); }
        default: return VUNDEF;
    }
}
static Object* g_so = nullptr;
bool g_saveLocked = false;   // (GenesisConsole: nothing is written to the save while it is on)
static Value so_flush(Act&, Value t, int, Value*) {
    if (!t.isObj()) return VFALSE;
    if (g_saveLocked) return VTRUE;
    std::string o = "ISAACSO1"; ser(o, getProp(t.o, intern("data")), 0);
    if (g_savePath.empty()) return VFALSE;
    std::string tmp = g_savePath + ".tmp";
    FILE* f = platformFopen(tmp, "wb"); if (!f) { logf("save: can't write %s\n", tmp.c_str()); return VFALSE; }
    bool ok = fwrite(o.data(), 1, o.size(), f) == o.size();
    if (fclose(f) != 0) ok = false;
    if (!ok || !platformReplaceFile(tmp, g_savePath)) { logf("save: can't write %s\n", g_savePath.c_str()); return VFALSE; }
    return VTRUE;
}
void sharedObjectFlushAll() { if (g_so) { Act a{vm.globalScope, VUNDEF, vm.root, nullptr}; so_flush(a, Value(g_so), 0, nullptr); } }
static Value so_clear(Act&, Value t, int, Value*) { if (t.isObj()) setProp(t.o, intern("data"), Value(gcNew<Object>(protos::object))); return VUNDEF; }
static Value so_getLocal(Act&, Value, int, Value*) {
    if (g_so) return Value(g_so);
    Object* so = gcNew<Object>(protos::sharedobject); so->kind = OK_SHAREDOBJ; gcPin(so);
    Value data = Value(gcNew<Object>(protos::object));
    if (!g_savePath.empty()) {
        if (FILE* f = platformFopen(g_savePath, "rb")) {
            std::string s; char buf[65536]; size_t n; while ((n = fread(buf, 1, sizeof buf, f)) > 0) s.append(buf, n); fclose(f);
            if (s.compare(0, 8, "ISAACSO1") == 0) { size_t i = 8; Value d = deser(s, i, 0); if (d.isObj()) data = d; }
        }
    }
    setProp(so, intern("data"), data);
    g_so = so; return Value(so);
}

// ============================================================ Native properties (getters/setters)
bool nativeGet(Object* o, Str* k, Value& out) {
    switch (o->kind) {
        case OK_BITMAPDATA: {
            BitmapData* b = (BitmapData*)o;
            if (k->s == "width") { out = Value((double)b->w); return true; }
            if (k->s == "height") { out = Value((double)b->h); return true; }
            if (k->s == "transparent") { out = VB(b->transparent); return true; }
            if (k->s == "rectangle") { Value args[4] = {Value(0.0), Value(0.0), Value((double)b->w), Value((double)b->h)}; Act a{}; out = rect_ctor(a, VUNDEF, 4, args); return true; }
            return false;
        }
        case OK_SOUND: {
            SoundObj* s = (SoundObj*)o;
            if (k->s == "position") {
                bool act = false; double ms = soundPositionMs(s, &act);
                if (act) s->lastPos = std::floor(ms);
                else if (s->started && !s->stopped) s->lastPos = soundDurationMs(s->def);   // played to the end
                out = Value(s->lastPos); return true;
            }
            if (k->s == "duration") { out = Value(soundDurationMs(s->def)); return true; }
            return false;
        }
        case OK_TRANSFORM: {
            TransformObj* t = (TransformObj*)o; if (!t->target) return false;
            if (k->s == "colorTransform") { Object* c = gcNew<Object>(protos::colortransform); setCxProps(c, t->target->cx); out = Value(c); return true; }
            if (k->s == "matrix") { Object* m = gcNew<Object>(protos::matrix); setMat(m, t->target->m); out = Value(m); return true; }
            if (k->s == "concatenatedMatrix") { Object* m = gcNew<Object>(protos::matrix); setMat(m, worldMatrix(t->target)); out = Value(m); return true; }
            return false;
        }
        default: return false;
    }
}
bool nativeSet(Object* o, Str* k, const Value& v) {
    if (o->kind == OK_TRANSFORM) {
        TransformObj* t = (TransformObj*)o; if (!t->target) return true;
        if (k->s == "colorTransform" && v.isObj()) { t->target->cx = cxFromObj(v.o); t->target->scriptColor = true; return true; }
        if (k->s == "matrix" && v.isObj()) { t->target->setMatrix(matFromObj(v)); t->target->scriptXform = true; return true; }
    }
    return false;
}

static Value ct_getRgb(Act&, Value t, int, Value*) { if (!t.isObj()) return VUNDEF; CX c = cxFromObj(t.o); return Value((double)(((int)c.ra & 255) << 16 | ((int)c.ga & 255) << 8 | ((int)c.ba & 255))); }

// ============================================================ Initialization
void initGlobals() {
    initAtoms();
    using namespace protos;
    object = newProtoObj(nullptr);
    function = newProtoObj(object);
    array = newProtoObj(object); string = newProtoObj(object); number = newProtoObj(object); boolean = newProtoObj(object);
    movieclip = newProtoObj(object); button = newProtoObj(object); textfield = newProtoObj(object);
    bitmapdata = newProtoObj(object); sound = newProtoObj(object); color = newProtoObj(object); transform = newProtoObj(object);
    colortransform = newProtoObj(object); matrix = newProtoObj(object); rectangle = newProtoObj(object); point = newProtoObj(object);
    sharedobject = newProtoObj(object);

    Object* g = gcNew<Object>(object); gcPin(g); vm.global = g;
    vm.globalScope = gcNew<Scope>(nullptr, SC_GLOBAL, g); gcPin(vm.globalScope);

    defn(object, "toString", obj_toString); defn(object, "valueOf", obj_valueOf); defn(object, "hasOwnProperty", obj_hasOwn);
    defn(object, "addProperty", g_noop); defn(object, "watch", g_noop); defn(object, "unwatch", g_noop);
    defn(function, "call", fn_call); defn(function, "apply", fn_apply);
    defv(g, "Object", Value((Object*)ctorFn(obj_ctor, object, "Object")));
    { Function* f = nativeFn(g_noop, "Function"); f->props.set(atoms::prototype, Value(function), PF_DONTENUM); defv(g, "Function", Value((Object*)f)); }

    defn(array, "push", arr_push); defn(array, "pop", arr_pop); defn(array, "shift", arr_shift); defn(array, "unshift", arr_unshift);
    defn(array, "slice", arr_slice); defn(array, "splice", arr_splice); defn(array, "join", arr_join); defn(array, "toString", arr_toString);
    defn(array, "concat", arr_concat); defn(array, "reverse", arr_reverse); defn(array, "sort", arr_sort);
    defv(g, "Array", Value((Object*)ctorFn(arr_ctor, array, "Array")));

    defn(string, "split", str_split); defn(string, "indexOf", str_indexOf); defn(string, "lastIndexOf", str_lastIndexOf);
    defn(string, "substr", str_substr); defn(string, "substring", str_substring); defn(string, "slice", str_slice);
    defn(string, "charAt", str_charAt); defn(string, "charCodeAt", str_charCodeAt); defn(string, "toLowerCase", str_lower);
    defn(string, "toUpperCase", str_upper); defn(string, "toString", str_toString); defn(string, "valueOf", str_toString);
    { Function* f = ctorFn(str_ctor, string, "String"); defn(f, "fromCharCode", str_fromCharCode); defv(g, "String", Value((Object*)f)); }
    defn(number, "toString", num_toString); defn(number, "valueOf", num_valueOf);
    { Function* f = ctorFn(num_ctor, number, "Number"); defv(f, "MAX_VALUE", Value(1.79769313486231e+308)); defv(f, "MIN_VALUE", Value(4.9e-324));
      defv(f, "NaN", Value(NAN)); defv(f, "POSITIVE_INFINITY", Value(INFINITY)); defv(f, "NEGATIVE_INFINITY", Value(-INFINITY)); defv(g, "Number", Value((Object*)f)); }
    defn(boolean, "toString", str_toString); defn(boolean, "valueOf", num_valueOf);
    defv(g, "Boolean", Value((Object*)ctorFn(bool_ctor, boolean, "Boolean")));

    Object* math = newProtoObj(object);
    defn(math, "abs", math_abs); defn(math, "acos", math_acos); defn(math, "asin", math_asin); defn(math, "atan", math_atan);
    defn(math, "atan2", math_atan2); defn(math, "ceil", math_ceil); defn(math, "cos", math_cos); defn(math, "exp", math_exp);
    defn(math, "floor", math_floor); defn(math, "log", math_log); defn(math, "max", math_max); defn(math, "min", math_min);
    defn(math, "pow", math_pow); defn(math, "random", math_random); defn(math, "round", math_round); defn(math, "sin", math_sin);
    defn(math, "sqrt", math_sqrt); defn(math, "tan", math_tan);
    defv(math, "PI", Value(3.141592653589793)); defv(math, "E", Value(2.718281828459045)); defv(math, "LN2", Value(0.6931471805599453));
    defv(math, "LN10", Value(2.302585092994046)); defv(math, "LOG2E", Value(1.442695040888963387)); defv(math, "LOG10E", Value(0.4342944819032518));
    defv(math, "SQRT1_2", Value(0.7071067811865476)); defv(math, "SQRT2", Value(1.4142135623730951));
    defv(g, "Math", Value(math));

    defn(g, "isNaN", g_isNaN); defn(g, "isFinite", g_isFinite); defn(g, "parseInt", g_parseInt); defn(g, "parseFloat", g_parseFloat);
    defn(g, "trace", g_traceFn); defn(g, "getTimer", g_getTimer); defn(g, "escape", g_escape); defn(g, "unescape", g_unescape);
    defn(g, "setInterval", g_setInterval); defn(g, "clearInterval", g_clearInterval); defn(g, "updateAfterEvent", g_noop);
    defn(g, "ASSetPropFlags", g_noop); defn(g, "fscommand", g_noop);
    defv(g, "NaN", Value(NAN)); defv(g, "Infinity", Value(INFINITY));

    Object* key = newProtoObj(object);
    defn(key, "isDown", key_isDown); defn(key, "getCode", key_getCode); defn(key, "getAscii", key_getAscii);
    defn(key, "addListener", key_addL); defn(key, "removeListener", key_remL); defn(key, "isToggled", g_noop);
    const std::pair<const char*, int> kc[] = {{"BACKSPACE", 8}, {"CAPSLOCK", 20}, {"CONTROL", 17}, {"DELETEKEY", 46}, {"DOWN", 40}, {"END", 35},
        {"ENTER", 13}, {"ESCAPE", 27}, {"HOME", 36}, {"INSERT", 45}, {"LEFT", 37}, {"PGDN", 34}, {"PGUP", 33}, {"RIGHT", 39},
        {"SHIFT", 16}, {"SPACE", 32}, {"TAB", 9}, {"UP", 38}, {"ALT", 18}};
    for (auto& p : kc) defv(key, p.first, Value((double)p.second));
    defv(g, "Key", Value(key));
    Object* mouse = newProtoObj(object);
    defn(mouse, "addListener", mouse_addL); defn(mouse, "removeListener", mouse_remL); defn(mouse, "hide", mouse_hide); defn(mouse, "show", mouse_show);
    defv(g, "Mouse", Value(mouse));

    // MovieClip
    defn(movieclip, "gotoAndStop", mc_gotoAndStop); defn(movieclip, "gotoAndPlay", mc_gotoAndPlay); defn(movieclip, "play", mc_play);
    defn(movieclip, "stop", mc_stop); defn(movieclip, "nextFrame", mc_nextFrame); defn(movieclip, "prevFrame", mc_prevFrame);
    defn(movieclip, "attachMovie", mc_attachMovie); defn(movieclip, "createEmptyMovieClip", mc_createEmpty);
    defn(movieclip, "removeMovieClip", mc_remove); defn(movieclip, "swapDepths", mc_swapDepths); defn(movieclip, "getDepth", mc_getDepth);
    defn(movieclip, "getNextHighestDepth", mc_nextDepth); defn(movieclip, "hitTest", mc_hitTest); defn(movieclip, "attachBitmap", mc_attachBitmap);
    defn(movieclip, "getBytesLoaded", mc_bytes); defn(movieclip, "getBytesTotal", mc_bytes); defn(movieclip, "getBounds", mc_getBounds);
    defn(movieclip, "localToGlobal", mc_localToGlobal); defn(movieclip, "globalToLocal", mc_globalToLocal);
    for (const char* n : {"clear", "beginFill", "endFill", "lineStyle", "moveTo", "lineTo", "curveTo", "startDrag", "stopDrag", "setMask"}) defn(movieclip, n, g_noop);
    defv(g, "MovieClip", Value((Object*)ctorFn(g_noop, movieclip, "MovieClip")));
    defn(button, "getDepth", mc_getDepth); defn(button, "swapDepths", mc_swapDepths);
    defv(g, "Button", Value((Object*)ctorFn(g_noop, button, "Button")));
    defv(g, "TextField", Value((Object*)ctorFn(g_noop, textfield, "TextField")));

    // Sound
    defn(sound, "attachSound", snd_attach); defn(sound, "start", snd_start); defn(sound, "stop", snd_stop);
    defn(sound, "setVolume", snd_setVolume); defn(sound, "getVolume", snd_getVolume); defn(sound, "setPan", snd_setPan); defn(sound, "getPan", snd_getPan);
    defn(sound, "loadSound", g_noop);
    defv(g, "Sound", Value((Object*)ctorFn(snd_ctor, sound, "Sound")));

    // SharedObject
    defn(sharedobject, "flush", so_flush); defn(sharedobject, "clear", so_clear);
    { Function* f = ctorFn(g_noop, sharedobject, "SharedObject"); defn(f, "getLocal", so_getLocal); defv(g, "SharedObject", Value((Object*)f)); }

    // ContextMenu / System / Stage
    { Object* cmp = newProtoObj(object); defn(cmp, "hideBuiltInItems", g_noop); defv(g, "ContextMenu", Value((Object*)ctorFn(nullptr, cmp, "ContextMenu")));
      Function* f = (Function*)getProp(g, intern("ContextMenu")).o; f->ctor = nullptr; f->nat = g_noop; }
    Object* sys = newProtoObj(object); Object* caps = newProtoObj(object);
    setProp(caps, intern("os"), Value(intern("Windows XP"))); setProp(caps, intern("version"), Value(intern("WIN 10,0,0,0")));
    setProp(caps, intern("screenResolutionX"), Value(1920.0)); setProp(caps, intern("screenResolutionY"), Value(1080.0));
    setProp(caps, intern("playerType"), Value(intern("StandAlone")));
    setProp(sys, intern("capabilities"), Value(caps)); defn(sys, "setClipboard", g_noop); defv(g, "System", Value(sys));
    Object* stage = newProtoObj(object);
    setProp(stage, intern("width"), Value((double)g_stageW)); setProp(stage, intern("height"), Value((double)g_stageH));
    setProp(stage, intern("scaleMode"), Value(intern("showAll"))); defn(stage, "addListener", g_noop); defv(g, "Stage", Value(stage));

    // flash.display / flash.geom
    Object* flash = newProtoObj(object); Object* disp = newProtoObj(object); Object* geom = newProtoObj(object);
    defn(bitmapdata, "getPixel", bd_getPixel); defn(bitmapdata, "getPixel32", bd_getPixel32); defn(bitmapdata, "setPixel", bd_setPixel);
    defn(bitmapdata, "setPixel32", bd_setPixel32); defn(bitmapdata, "fillRect", bd_fillRect); defn(bitmapdata, "copyPixels", bd_copyPixels);
    defn(bitmapdata, "draw", bd_draw); defn(bitmapdata, "dispose", bd_dispose); defn(bitmapdata, "clone", bd_clone);
    setProp(disp, intern("BitmapData"), Value((Object*)ctorFn(bd_ctor, bitmapdata, "BitmapData")));
    setProp(geom, intern("Rectangle"), Value((Object*)ctorFn(rect_ctor, rectangle, "Rectangle")));
    setProp(geom, intern("Point"), Value((Object*)ctorFn(point_ctor, point, "Point")));
    defn(matrix, "translate", matrix_translate); defn(matrix, "scale", matrix_scale); defn(matrix, "rotate", matrix_rotate);
    defn(matrix, "identity", matrix_identity); defn(matrix, "concat", matrix_concat); defn(matrix, "clone", matrix_clone);
    setProp(geom, intern("Matrix"), Value((Object*)ctorFn(matrix_ctor, matrix, "Matrix")));
    defn(colortransform, "getRgb", ct_getRgb);
    setProp(geom, intern("ColorTransform"), Value((Object*)ctorFn(ct_ctor, colortransform, "ColorTransform")));
    setProp(geom, intern("Transform"), Value((Object*)ctorFn(transform_ctor, transform, "Transform")));
    setProp(flash, intern("display"), Value(disp)); setProp(flash, intern("geom"), Value(geom));
    defv(g, "flash", Value(flash));
    // mdm: Zinc wrapper of the Steam version (exit, fullscreen); see patches.cpp
    {
        extern Value mdmExit(Act&, Value, int, Value*); extern Value mdmFull(Act&, Value, int, Value*);
        Object* mdm = newProtoObj(object); Object* app = newProtoObj(object); Object* forms = newProtoObj(object); Object* form = newProtoObj(object);
        defn(app, "exit", mdmExit); defn(app, "restore", g_noop);
        extern Value mdmDeleteFile(Act&, Value, int, Value*);
        Object* fs = newProtoObj(object);   // (Steam's serial.txt backup is unused: saveFile/loadFile do nothing)
        defn(fs, "saveFile", g_noop); defn(fs, "loadFile", g_noop); defn(fs, "deleteFile", mdmDeleteFile);
        setProp(mdm, intern("FileSystem"), Value(fs));
        defn(form, "showFullScreen", mdmFull); defn(form, "hideCaption", g_noop);
        setProp(forms, intern("thisForm"), Value(form));
        setProp(mdm, intern("Application"), Value(app)); setProp(mdm, intern("Forms"), Value(forms));
        // Eternal Edition only takes the Zinc path (windowed/fullscreen and _root scaling, which the HD patch
        // replaces) if mdm.System.macVerString exists; otherwise it assumes it runs in a browser ("nozinc") and uses
        // 2560x1920 layers. A string that does not contain "type Function" is not treated as Windows (no DirectX).
        Object* sysz = newProtoObj(object);
        setProp(sysz, intern("macVerString"), Value(mkstr("recomp")));
        setProp(mdm, intern("System"), Value(sysz));
        if (!getenv("ISAAC_NOMDM")) defv(g, "mdm", Value(mdm));   // (differential tests: no Zinc, like Ruffle)
    }
    for (Object* o : {g, flash, disp, geom, math, key, mouse, sys, caps, stage}) gcPin(o);

    g_rootTracers.push_back([](Tracer& tr) {
        for (auto& iv : g_intervals) { traceValue(tr, iv.fn); traceValue(tr, iv.obj); for (auto& v : iv.args) traceValue(tr, v); }
        tr.mark(g_so);
    });
}

}  // namespace fl
