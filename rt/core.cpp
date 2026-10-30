// AVM1 core: GC, strings, conversions, properties, scopes and calls.
#include "core.h"
#include "display.h"
#include "platform.h"
#include <cstdarg>
#include <cstdio>
#include <algorithm>
#include <functional>
#include <chrono>
#if defined(__linux__) && !defined(__ANDROID__)   // (Android: no backtrace() before API 33)
#include <execinfo.h>
#include <dlfcn.h>
#endif
#include <typeinfo>
#include <string>
#include <random>
#include <chrono>
namespace fl { extern bool g_rngSeedFixed; }

namespace fl {

VMState vm;
bool g_trace = false;

void logf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); vfprintf(stderr, fmt, ap); va_end(ap); fflush(stderr);
}

// ============================================================ GC
static std::vector<GcObj*> g_heap;
static std::vector<GcObj**> g_roots;
static size_t g_lastLive = 0;   // (0 = no collection has happened yet: g_lim.gcMinLive is used)
std::vector<std::function<void(Tracer&)>> g_rootTracers;

void gcRegister(GcObj* o) { g_heap.push_back(o); }
void gcAddRoot(GcObj** slot) { g_roots.push_back(slot); }
static std::vector<GcObj*> g_pinnedRoots;
void gcPin(GcObj* o) { o->pinned = 1; g_pinnedRoots.push_back(o); }
size_t gcCount() { return g_heap.size(); }
static size_t g_extBytes = 0;
void gcExternalAlloc(size_t bytes) { g_extBytes += bytes; }

bool g_gcLog = false;
static void strCacheDropDead();
// Mark-and-sweep GC with lazy sweeping: marking (live objects only) is fast; dead objects are
// freed gradually over the following frames. An unmarked object is unreachable and can never
// become reachable again, so freeing it later is safe.
static std::vector<GcObj*> g_sweep; static size_t g_sweepPos = 0;
// Freeing is time-budgeted (not by object count): on Switch freeing an object costs several times more than on
// PC, and 60,000 at once took ~150 ms in a single tick.
static void internForget(Str* s);
static void sweepSome(double budgetMs) {
    auto t0 = std::chrono::steady_clock::now();
    while (g_sweepPos < g_sweep.size()) {
        size_t end = std::min(g_sweep.size(), g_sweepPos + 256);
        for (; g_sweepPos < end; g_sweepPos++) {
            GcObj* o = g_sweep[g_sweepPos];
            if (o->marked || o->pinned) { o->marked = 0; g_heap.push_back(o); }
            else { if (o->weakAtom) internForget((Str*)o); delete o; }
        }
        if (std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count() >= budgetMs) break;
    }
    if (g_sweepPos >= g_sweep.size()) { g_sweep.clear(); g_sweepPos = 0; }
}
double g_gcMs = 0;   // accumulated time spent in the collector (F3 diagnostics)
// The collector never takes the tick past this (logic + GC, ms; a tick has 33 ms): when it went up to 18 ms
// of sweeping on top of a busy tick, logic missed its deadline and the render thread stuttered.
static constexpr double kGcTickTargetMs = 12.0;
bool (*g_gcMemPressure)() = nullptr;   // (PSP) true when memory runs low: collect now
void gcStats(size_t* objects, size_t* lastLive, size_t* extBytes) { *objects = g_heap.size(); *lastLive = g_lastLive; *extBytes = g_extBytes; }
void gcCollect(double tickMs) {
    if (!g_sweep.empty()) {   // a few ms per tick until done
        auto s0 = std::chrono::steady_clock::now();
        // if dead objects pile up faster than they are freed (lots of garbage per tick), free faster,
        // but only within the tick's headroom (always at least gcSweepMs, so the sweep keeps progressing)
        size_t left = g_sweep.size() - g_sweepPos, thr = std::max(g_lastLive, g_lim.gcMinLive) * 2;
        double want = g_lim.gcSweepMs * (left > thr ? 3 : 1) * (g_heap.size() > 2 * thr ? 2 : 1);
        sweepSome(std::max(g_lim.gcSweepMs, std::min(want, kGcTickTargetMs - tickMs)));
        g_gcMs += std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - s0).count();
        return;
    }
    size_t trig = (size_t)(std::max(g_lastLive, g_lim.gcMinLive) * g_lim.gcGrowth);
    bool pressure = g_gcMemPressure && g_heap.size() > g_lim.gcMinLive / 2 && g_gcMemPressure();
    if (g_heap.size() < trig && g_extBytes < g_lim.extGcBytes && !pressure) return;
    // a heavy tick (room load, explosion...) postpones the mark to a lighter one, unless the heap is far over
    if (tickMs > kGcTickTargetMs * 0.6 && g_heap.size() < trig + trig / 2 && !pressure) return;
    g_extBytes = 0;
    auto t0 = std::chrono::steady_clock::now(); size_t before = g_heap.size();
    Tracer tr;
    static bool gcPath = getenv("ISAAC_GCPATH") != nullptr;
    std::unordered_map<GcObj*, GcObj*> pred; if (gcPath) tr.pred = &pred;
    // roots: pinned objects (interned strings are pinned too, but reference nothing)
    for (GcObj* o : g_pinnedRoots) { if (!o->marked && o->onHeap) { o->marked = 1; tr.count++; } tr.stack.push_back(o); }
    for (auto** r : g_roots) tr.mark(*r);
    tr.mark(vm.global); tr.mark(vm.globalScope); tr.mark(vm.root);
    for (auto& f : g_rootTracers) f(tr);
    auto tRoots = std::chrono::steady_clock::now(); size_t traced = 0;
    while (!tr.stack.empty()) { GcObj* o = tr.stack.back(); tr.stack.pop_back(); tr.cur = o; o->trace(tr); traced++; }
    if (g_gcLog) logf("[gc] roots %.1f ms, traversal %.1f ms (%zu objects traversed, %zu pinned)\n",
        std::chrono::duration<double, std::milli>(tRoots - t0).count(), std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - tRoots).count(), traced, g_pinnedRoots.size());
    if (gcPath) {   // reference chain keeping the oldest BitmapData alive
        std::vector<BitmapData*> bds;
        for (GcObj* o : g_heap) if (o->marked) if (Object* ob = dynamic_cast<Object*>(o)) if (ob->kind == OK_BITMAPDATA) bds.push_back((BitmapData*)ob);
        std::sort(bds.begin(), bds.end(), [](BitmapData* a, BitmapData* b) { return a->uid < b->uid; });
        BitmapData* oldest = bds.size() > 12 ? bds[12] : nullptr;
        for (GcObj* o = oldest; o; ) {
            std::string d = typeid(*o).name();
            if (auto* dob = dynamic_cast<DisplayObject*>(o)) d += " '" + std::string(dob->name ? dob->name->s : "-") + "' id=" + std::to_string(dob->charId) + (dob->removed ? " (REMOVED)" : "") + (dob->parent ? "" : " (no parent)");
            if (auto* f = dynamic_cast<Function*>(o)) d += std::string(" ") + f->dbgName;
            auto it2 = pred.find(o); GcObj* p = it2 == pred.end() ? nullptr : it2->second;
            if (auto* po = p ? dynamic_cast<Object*>(p) : nullptr) for (auto& pr : po->props.v) if (pr.v.t == T_OBJ && (GcObj*)pr.v.o == o) { d += "   <- ." + pr.k->s; break; }
            logf("[gcpath] %s\n", d.c_str());
            auto it = pred.find(o); o = it == pred.end() ? nullptr : it->second;
        }
    }
    if (getenv("ISAAC_GCHIST")) {   // (diagnostics: what the garbage is made of)
        std::unordered_map<std::string, size_t> h;
        for (GcObj* o : g_heap) if (!o->marked && !o->pinned) {
            std::string k = typeid(*o).name();
            if (Object* ob = dynamic_cast<Object*>(o)) k += "/k" + std::to_string((int)ob->kind);
            h[k]++;
        }
        std::vector<std::pair<size_t, std::string>> v; for (auto& kv : h) v.push_back({kv.second, kv.first});
        std::sort(v.rbegin(), v.rend());
        for (size_t i = 0; i < v.size() && i < 8; i++) logf("[gchist] %8zu %s\n", v[i].first, v[i].second.c_str());
    }
    strCacheDropDead();   // (unmarked = about to be freed)
    size_t live = tr.count;   // (without walking the whole heap: that cost ~10 ms with 800,000 objects)
    g_sweep.swap(g_heap); g_heap.clear(); g_heap.reserve(live + 65536); g_sweepPos = 0;
    g_lastLive = std::max<size_t>(live + live / 2, g_lim.gcMinLive);
    g_gcMs += std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    if (g_gcLog) {
        size_t nb = 0, bytes = 0, nbDead = 0, bytesDead = 0;
        for (GcObj* o : g_sweep) if (Object* ob = dynamic_cast<Object*>(o)) if (ob->kind == OK_BITMAPDATA) {
            BitmapData* b = (BitmapData*)ob; size_t sz = b->px.capacity() * 4;
            if (o->marked || o->pinned) { nb++; bytes += sz; } else { nbDead++; bytesDead += sz; }
        }
        logf("[gc] BitmapData live %zu (%zu MB), dead %zu (%zu MB)\n", nb, bytes >> 20, nbDead, bytesDead >> 20);
    }
    if (g_gcLog) logf("[gc] mark: %zu objects, %zu live, %.1f ms (deferred sweep)\n", before, live,
                      std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count());
}

// ============================================================ Strings
static const char* kDispProps[22] = {"_x", "_y", "_xscale", "_yscale", "_currentframe", "_totalframes", "_alpha", "_visible",
    "_width", "_height", "_rotation", "_target", "_framesloaded", "_name", "_droptarget", "_url", "_highquality", "_focusrect",
    "_soundbuftime", "_quality", "_xmouse", "_ymouse"};

struct SVHash { size_t operator()(std::string_view s) const { return std::hash<std::string_view>()(s); } };
static std::unordered_map<std::string_view, Str*, SVHash>* g_intern;

static void classify(Str* s) {
    const std::string& v = s->s;
    s->pathy = v.find_first_of(":./") != std::string::npos;
    if (!v.empty() && v[0] == '_') {
        for (int i = 0; i < 22; i++) {
            const char* p = kDispProps[i]; size_t n = strlen(p);
            if (n == v.size()) {
                bool ok = true;
                for (size_t k = 0; k < n; k++) if (tolower((unsigned char)v[k]) != p[k]) { ok = false; break; }
                if (ok) { s->dprop = (int8_t)i; break; }
            }
        }
    }
    if (!v.empty() && v.size() <= 10 && (v.size() == 1 || v[0] != '0')) {
        int64_t n = 0; bool ok = true;
        for (char c : v) { if (c < '0' || c > '9') { ok = false; break; } n = n * 10 + (c - '0'); }
        if (ok && n < 0x7fffffff) s->arrIdx = (int32_t)n;
    }
}

Str* intern(std::string_view sv) {
    if (!g_intern) g_intern = new std::unordered_map<std::string_view, Str*, SVHash>(1 << 14);
    auto it = g_intern->find(sv);
    if (it != g_intern->end()) { Str* f = it->second; if (f->weakAtom) { f->weakAtom = 0; f->pinned = 1; } return f; }
    Str* s = new Str(std::string(sv));
    s->onHeap = 1; s->pinned = 1; s->interned = true; s->atom = s;
    classify(s);
    gcRegister(s);
    g_intern->emplace(std::string_view(s->s), s);
    return s;
}
// Flash names every unnamed instance ("instance12345", a global counter): pinned forever, those names were the one
// thing that grew without limit in a long session (~250 KB per hour). They are interned weakly instead: freed with
// their object, unless a script used the name as a key (atomOf / intern pin it then).
Str* internWeak(std::string_view sv) {
    if (!g_intern) g_intern = new std::unordered_map<std::string_view, Str*, SVHash>(1 << 14);
    auto it = g_intern->find(sv);
    if (it != g_intern->end()) return it->second;
    Str* s = new Str(std::string(sv));
    s->onHeap = 1; s->weakAtom = 1; s->interned = true; s->atom = s;
    classify(s);
    gcRegister(s);
    g_intern->emplace(std::string_view(s->s), s);
    return s;
}
static void internForget(Str* s) { auto it = g_intern->find(std::string_view(s->s)); if (it != g_intern->end() && it->second == s) g_intern->erase(it); }
static std::unordered_map<std::string, size_t>* g_strProf;
static void strProfDump() {
    std::vector<std::pair<size_t, std::string>> v; for (auto& kv : *g_strProf) v.push_back({kv.second, kv.first});
    std::sort(v.rbegin(), v.rend());
    for (size_t i = 0; i < v.size() && i < 40; i++) fprintf(stderr, "[strprof] %9zu %s\n", v[i].first, v[i].second.c_str());
}
// Weak cache of short dynamic strings: the game rebuilds the same small strings every tick ("m12-3.mp3",
// "hat2", numbers converted to text...), and those were ~70% of all garbage. Identical content returns the
// same Str (they are immutable). It does not keep anything alive: entries whose Str was not marked are
// dropped at each collection (gcCollect), before the sweep frees them.
static constexpr size_t kStrCache = 8192, kStrCacheMaxLen = 48;
static Str* g_strCache[kStrCache];
static void strCacheDropDead() { for (auto& e : g_strCache) if (e && !e->marked) e = nullptr; }
Str* mkstr(std::string v) {
    if (v.size() <= 1) return intern(v);
    Str** slot = nullptr;
    if (v.size() <= kStrCacheMaxLen) {
        slot = &g_strCache[std::hash<std::string_view>()(v) & (kStrCache - 1)];
        if (*slot && (*slot)->s == v) return *slot;
    }
    static bool prof = getenv("ISAAC_STRPROF") != nullptr;
    if (prof) {
        if (!g_strProf) { g_strProf = new std::unordered_map<std::string, size_t>; atexit(strProfDump); }
        std::string k = v.substr(0, 40); for (auto& c : k) if (c >= '0' && c <= '9') c = '#';
        (*g_strProf)[k]++;
    }
    Str* s = new Str(std::move(v)); s->onHeap = 1; gcRegister(s);
    if (slot) *slot = s;
    return s;
}

namespace atoms {
Str *length, *prototype, *__proto__, *constructor, *this_, *_root, *_parent, *_global, *onEnterFrame, *onLoad, *valueOf,
    *toString, *undefined_, *null_, *true_, *false_, *empty, *NaN_;
}
void initAtoms() {
    using namespace atoms;
    length = intern("length"); prototype = intern("prototype"); __proto__ = intern("__proto__"); constructor = intern("constructor");
    this_ = intern("this"); _root = intern("_root"); _parent = intern("_parent"); _global = intern("_global");
    onEnterFrame = intern("onEnterFrame"); onLoad = intern("onLoad"); valueOf = intern("valueOf"); toString = intern("toString");
    undefined_ = intern("undefined"); null_ = intern("null"); true_ = intern("true"); false_ = intern("false"); empty = intern("");
    NaN_ = intern("NaN");
}

// ============================================================ Basic objects
Array::Array() { kind = OK_ARRAY; proto = protos::array; }
Function::Function() { kind = OK_FUNCTION; proto = protos::function; }
void Function::trace(Tracer& tr) { Object::trace(tr); tr.mark(scope); tr.mark(baseClip); }

// ============================================================ Conversions
static double decimalShift(double v, int exp) {
    double base = 10.0;
    if (exp > 0) { while (exp > 0) { if (exp & 1) v *= base; exp >>= 1; base *= base; } }
    else { unsigned e = (unsigned)(-(int64_t)exp); while (e > 0) { if (e & 1) v /= base; e >>= 1; base *= base; } }
    return v;
}

std::string numToString(double n) {
    if (std::isnan(n)) return "NaN";
    if (n == INFINITY) return "Infinity";
    if (n == -INFINITY) return "-Infinity";
    if (n == 0) return "0";
    if (n >= -2147483648.0 && n <= 2147483647.0 && n == std::floor(n)) {
        char b[16]; snprintf(b, sizeof b, "%d", (int)n); return b;
    }
    std::string buf; bool neg = false;
    if (n < 0) { n = -n; buf.push_back('-'); neg = true; }
    uint64_t bits; memcpy(&bits, &n, 8);
    int expb = (int)((bits >> 52) & 0x7ff) - 1023;
    if (expb == -1023) { double m = n * 1.801439850948198e16; memcpy(&bits, &m, 8); expb = (int)((bits >> 52) & 0x7ff) - 1023 - 54; }
    int exp = (int)std::round(expb * 0.301029995663981);
    double mant = decimalShift(n, -exp);
    if ((int)mant == 0) { exp -= 1; mant = decimalShift(n, -exp); }
    if ((int)mant >= 10) { exp += 1; mant = decimalShift(n, -exp); }
    auto digit = [&]() -> char { int d = (int)mant; mant -= d; mant *= 10.0; return (char)('0' + d); };
    const int MAXD = 15;
    if (exp >= 15) { buf.push_back(digit()); buf.push_back('.'); for (int i = 0; i < MAXD - 1; i++) buf.push_back(digit()); }
    else if (exp >= 0) {
        buf.push_back('0'); for (int i = 0; i <= exp; i++) buf.push_back(digit());
        buf.push_back('.'); for (int i = 0; i < MAXD - exp - 1; i++) buf.push_back(digit());
        exp = 0;
    } else if (exp >= -5) {
        buf += "00."; buf.append((size_t)(-exp - 1), '0'); for (int i = 0; i < MAXD; i++) buf.push_back(digit());
        exp = 0;
    } else {
        buf.push_back('0'); char d = digit(); if (d != '0') buf.push_back(d); buf.push_back('.');
        for (int i = 0; i < MAXD - 1; i++) buf.push_back(digit());
    }
    if (digit() >= '5') {
        for (int i = (int)buf.size() - 1; i >= 0; i--) {
            char& c = buf[i];
            if (c == '9') c = '0'; else if (c >= '0') { c++; break; }
        }
    }
    while (!buf.empty() && buf.back() == '0') buf.pop_back();
    if (!buf.empty() && buf.back() == '.') buf.pop_back();
    if (exp != 0) {
        size_t st = neg ? 1 : 0;
        size_t pos = st; while (pos < buf.size() && buf[pos] == '0') pos++;
        buf.erase(st, pos - st);
        if (buf.size() == st) { buf.push_back('1'); exp += 1; }
        else {
            size_t last = buf.find_last_not_of('0');
            if (last == st) { exp += (int)(buf.size() - st) - 1; buf.resize(st + 1); }
        }
        char e[16]; snprintf(e, sizeof e, "e%+d", exp); buf += e;
    }
    size_t i = neg ? 1 : 0;
    if (buf.size() > i + 1 && buf[i] == '0' && buf[i + 1] != '.') buf.erase(i, 1);
    return buf;
}

double strToNum(const std::string& str) {
    const char* s = str.c_str(); size_t n = str.size();
    // radix
    {
        size_t k = 0; if (k < n && (s[k] == '+' || s[k] == '-')) k++;
        if (k < n && s[k] == '0') {
            if (k + 1 < n && (s[k + 1] == 'x' || s[k + 1] == 'X')) {
                if (n < 2) return NAN;
                const char* h = s + 2; int64_t v = 0; bool any = false; bool neg = false;
                if (*h == '-') { neg = true; h++; } else if (*h == '+') h++;
                for (; *h; h++) { int d; if (*h >= '0' && *h <= '9') d = *h - '0'; else if (*h >= 'a' && *h <= 'f') d = *h - 'a' + 10; else if (*h >= 'A' && *h <= 'F') d = *h - 'A' + 10; else return NAN; v = v * 16 + d; any = true; }
                if (!any) return NAN;
                int32_t r = (int32_t)(uint32_t)(neg ? -v : v); return r;
            }
            bool oct = true; for (size_t j = k + 1; j < n; j++) if (s[j] < '0' || s[j] > '7') { oct = false; break; }
            if (oct) {
                bool neg = s[0] == '-'; int64_t v = 0;
                for (size_t j = k; j < n; j++) v = v * 8 + (s[j] - '0');
                return (int32_t)(uint32_t)(neg ? -v : v);
            }
        }
    }
    size_t i = 0;
    while (i < n && (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r' || s[i] == '\f' || s[i] == '\v')) i++;
    size_t start = i;
    if (i < n && (s[i] == '+' || s[i] == '-')) i++;
    size_t ds = i;
    while (i < n && s[i] >= '0' && s[i] <= '9') i++;
    if (i < n && s[i] == '.') { i++; while (i < n && s[i] >= '0' && s[i] <= '9') i++; }
    if (i == ds) return NAN;
    if (i < n && (s[i] == 'e' || s[i] == 'E')) {
        size_t j = i + 1; if (j < n && (s[j] == '+' || s[j] == '-')) j++;
        while (j < n && s[j] >= '0' && s[j] <= '9') j++;
        i = j;
    }
    if (i != n) return NAN;
    return strtod(std::string(s + start, i - start).c_str(), nullptr);
}

int toInt32(double d) {
    if (!std::isfinite(d)) return 0;
    if (d >= -2147483648.0 && d <= 2147483647.0) return (int32_t)d;
    double m = std::fmod(std::trunc(d), 4294967296.0);
    if (m < 0) m += 4294967296.0;
    return (int32_t)(uint32_t)m;
}

static Value callNamedMethod(Object* o, Str* name) {
    Value f = getProp(o, name);
    if (f.isObj() && f.o->kind == OK_FUNCTION) {
        Act a{vm.globalScope, Value(o), nullptr, nullptr};
        return callValue(a, f, Value(o), 0, nullptr);
    }
    return VUNDEF;
}

Value toPrimitive(const Value& v) {
    if (v.t != T_OBJ) return v;
    Object* o = v.o;
    if (o->kind == OK_BOXED) return ((Boxed*)o)->prim;
    if (o->isDisplay()) return v;   // clips are compared by path
    Value r = callNamedMethod(o, atoms::valueOf);
    if (r.t == T_OBJ || r.t == T_UNDEF) return r.t == T_UNDEF ? r : v;
    return r;
}

double toNum(const Value& v) {
    switch (v.t) {
        case T_NUM: return v.n;
        case T_BOOL: return v.b ? 1 : 0;
        case T_STR: return strToNum(v.s->s);
        case T_OBJ: {
            if (v.o->kind == OK_BOXED) return toNum(((Boxed*)v.o)->prim);
            if (v.o->isDisplay() || v.o->kind == OK_FUNCTION) return NAN;
            Value p = toPrimitive(v);
            if (p.t == T_OBJ) return NAN;
            return toNum(p);
        }
        default: return NAN;
    }
}

Str* toStr(const Value& v) {
    switch (v.t) {
        case T_STR: return v.s;
        case T_NUM: {
            double n = v.n;
            if (n >= 0 && n < 10 && n == (int)n) { char c[2] = {(char)('0' + (int)n), 0}; return intern(c); }
            return mkstr(numToString(n));
        }
        case T_BOOL: return v.b ? atoms::true_ : atoms::false_;
        case T_NULL: return atoms::null_;
        case T_OBJ: {
            Object* o = v.o;
            if (o->kind == OK_BOXED) return toStr(((Boxed*)o)->prim);
            if (o->isDisplay()) return targetPath((DisplayObject*)o);
            Value r = callNamedMethod(o, atoms::toString);
            if (r.t == T_STR) return r.s;
            return o->kind == OK_FUNCTION ? intern("[type Function]") : intern("[type Object]");
        }
        default: return atoms::undefined_;
    }
}

Object* toObject(const Value& v) {
    switch (v.t) {
        case T_OBJ: return v.o;
        case T_STR: case T_NUM: case T_BOOL: {
            Boxed* b = gcNew<Boxed>(); b->kind = OK_BOXED; b->prim = v;
            b->proto = v.t == T_STR ? protos::string : v.t == T_NUM ? protos::number : protos::boolean;
            return b;
        }
        default: return nullptr;
    }
}

Str* typeOf(const Value& v) {
    switch (v.t) {
        case T_NUM: return intern("number"); case T_BOOL: return intern("boolean"); case T_STR: return intern("string");
        case T_NULL: return intern("null");
        case T_OBJ: return v.o->kind == OK_FUNCTION ? intern("function") : v.o->kind == OK_CLIP ? intern("movieclip") : intern("object");
        default: return intern("undefined");
    }
}

// ============================================================ Operators
Value add2(const Value& l, const Value& r) {
    if (l.t == T_NUM && r.t == T_NUM) return Value(l.n + r.n);
    Value a = toPrimitive(r), b = toPrimitive(l);
    if (a.t == T_STR || b.t == T_STR) {
        Str* bs = toStr(b); Str* as = toStr(a);
        if (as->s.empty()) return bs; if (bs->s.empty()) return as;
        return mkstr(bs->s + as->s);
    }
    return Value(toNum(b) + toNum(a));
}

// abstract_lt(a, b): a < b
Value lt(const Value& a, const Value& b) {
    if (a.t == T_NUM && b.t == T_NUM) {
        if (std::isnan(a.n) || std::isnan(b.n)) return VUNDEF;
        return VB(a.n < b.n);
    }
    Value pa = a.t == T_OBJ && !a.o->isDisplay() ? toPrimitive(a) : a;
    if (pa.t == T_OBJ && !pa.o->isDisplay()) return VFALSE;
    Value pb = b.t == T_OBJ && !b.o->isDisplay() ? toPrimitive(b) : b;
    if (pb.t == T_OBJ && !pb.o->isDisplay()) return VFALSE;
    if (pa.t == T_STR && pb.t == T_STR) return VB(pa.s->s < pb.s->s);
    double x = toNum(pa), y = toNum(pb);
    if (std::isnan(x) || std::isnan(y)) return VUNDEF;
    return VB(x < y);
}

static bool eqImpl(const Value& a, const Value& b, int depth) {
    VT ta = a.t == T_HOLE ? T_UNDEF : a.t, tb = b.t == T_HOLE ? T_UNDEF : b.t;
    if ((ta == T_UNDEF || ta == T_NULL) && (tb == T_UNDEF || tb == T_NULL)) return true;
    if (ta == T_BOOL && tb == T_BOOL) return a.b == b.b;
    if (ta == T_STR && tb == T_STR) return a.s == b.s || a.s->s == b.s->s;
    if (ta == T_NUM && tb == T_NUM) return a.n == b.n || (std::isnan(a.n) && std::isnan(b.n));
    if (ta == T_OBJ && tb == T_OBJ) {
        if (a.o == b.o) return true;
        if (a.o->isDisplay() && b.o->isDisplay()) {
            DisplayObject* x = (DisplayObject*)a.o; DisplayObject* y = (DisplayObject*)b.o;
            if (!x->removed && !y->removed) {   // compare paths without building strings
                while (x && y) { if (x->name != y->name) return false; x = x->parent; y = y->parent; if (x == y) return true; }
                return x == y;
            }
            return targetPath(x)->s == targetPath(y)->s;
        }
        return false;
    }
    if (depth > 4) return false;
    if (ta == T_BOOL) return eqImpl(Value(a.b ? 1.0 : 0.0), b, depth + 1);
    if (tb == T_BOOL) return eqImpl(a, Value(b.b ? 1.0 : 0.0), depth + 1);
    if (ta == T_NUM && tb == T_STR) return a.n == strToNum(b.s->s);
    if (ta == T_STR && tb == T_NUM) return strToNum(a.s->s) == b.n;
    if (ta == T_OBJ && (tb == T_NUM || tb == T_STR)) {
        if (a.o->isDisplay()) return false;
        Value p = toPrimitive(a); return p.t != T_OBJ && p.t != T_UNDEF && eqImpl(p, b, depth + 1);
    }
    if (tb == T_OBJ && (ta == T_NUM || ta == T_STR)) {
        if (b.o->isDisplay()) return false;
        Value p = toPrimitive(b); return p.t != T_OBJ && p.t != T_UNDEF && eqImpl(a, p, depth + 1);
    }
    return false;
}
bool eq2(const Value& a, const Value& b) {
    if (a.t == T_NUM && b.t == T_NUM) return a.n == b.n || (std::isnan(a.n) && std::isnan(b.n));
    return eqImpl(a, b, 0);
}
bool strictEq(const Value& a, const Value& b) {
    VT ta = a.t == T_HOLE ? T_UNDEF : a.t, tb = b.t == T_HOLE ? T_UNDEF : b.t;
    if (ta != tb) return false;
    switch (ta) {
        case T_UNDEF: case T_NULL: return true;
        case T_BOOL: return a.b == b.b;
        case T_NUM: return a.n == b.n;
        case T_STR: return a.s == b.s || a.s->s == b.s->s;
        case T_OBJ: return a.o == b.o;
        default: return false;
    }
}

// Flash Player's random number generator (avmplus MathUtils.cpp, as Ruffle ports it in avm_rng.rs): Math.random()
// returns n / 2^31 for a 31-bit n, random(max) n % max. Seeded on first use from the clock, or from ISAAC_AVMRNG
// (tests: headless runs use a fixed seed so they are deterministic; the differential tests against Ruffle use the same
// seed on both sides).
static uint32_t g_rngU = 0;
uint64_t g_rngCalls = 0;
static void rngTrace(const char* what) {   // (testing) ISAAC_RNGTRACE: who asks for random numbers (addresses for addr2line)
    static int on = -1; if (on < 0) on = getenv("ISAAC_RNGTRACE") != nullptr;
    if (!on) return;
#if defined(__linux__) && !defined(__ANDROID__)
    void* bt[8]; int n = backtrace(bt, 8);
    fprintf(stderr, "[rngcall] %llu %s", (unsigned long long)g_rngCalls, what);
    for (int i = 2; i < n; i++) { Dl_info di; if (dladdr(bt[i], &di) && di.dli_fbase) fprintf(stderr, " 0x%lx", (unsigned long)((char*)bt[i] - (char*)di.dli_fbase)); }
    fprintf(stderr, "\n");
#endif
}
static int32_t avmRandom() {
    g_rngCalls++;
    if (g_rngU == 0) {
        const char* e = getenv("ISAAC_AVMRNG");
        g_rngU = e ? (uint32_t)strtoul(e, nullptr, 10) : (g_rngSeedFixed ? 0x15aacu : (uint32_t)std::chrono::steady_clock::now().time_since_epoch().count());
        if (g_rngU == 0) g_rngU = 1;
    }
    // RandomFastNext
    if (g_rngU & 1) g_rngU = (g_rngU >> 1) ^ 0x48000000u; else g_rngU >>= 1;
    int32_t n = (int32_t)g_rngU;
    n = (int32_t)((uint32_t)n * 71u);
    // RandomPureHasher
    int32_t seed = n;
    seed = (int32_t)((((uint32_t)seed << 13) ^ (uint32_t)seed) - (uint32_t)(seed >> 21));
    int32_t r = (int32_t)((uint32_t)seed * (uint32_t)seed);
    r = (int32_t)((uint32_t)r * 15731u);
    r = (int32_t)((uint32_t)r + 789221u);
    r = (int32_t)((uint32_t)r * (uint32_t)seed);
    r = (int32_t)((uint32_t)r + 1376312589u);
    r &= 0x7FFFFFFF;
    r = (int32_t)((uint32_t)r + (uint32_t)seed);
    r = (int32_t)((((uint32_t)r << 13) ^ (uint32_t)r) - (uint32_t)(r >> 21));
    return r & 0x7FFFFFFF;
}
bool g_rngSeedFixed = false;
// (mods) the generator's state, to call game functions that might draw random numbers without changing the run
void rngSave(uint32_t* state, uint64_t* calls) { *state = g_rngU; *calls = g_rngCalls; }
void rngRestore(uint32_t state, uint64_t calls) { g_rngU = state; g_rngCalls = calls; }
double randomDouble() { rngTrace("Math.random"); return (double)avmRandom() / 2147483648.0; }
Value randomNumber(const Value& maxv) {
    double d = toNum(maxv);
    int32_t m = std::isfinite(d) ? (int32_t)d : 0;
    if (m <= 0) return Value(0.0);
    if (getenv("ISAAC_RNGTRACE")) { char w[32]; snprintf(w, sizeof w, "random(%d)", m); rngTrace(w); }
    return Value((double)(avmRandom() % m));
}
Value getTime() { return Value(std::floor(g_player.timeMs)); }

// ============================================================ Properties
// Array element insertion order, for for-in (see Array::ord). Called before el[i] (a hole) gets a value.
void arrNoteInsert(Array* a, size_t i) {
    if (a->customOrd) { a->ord.push_back((uint32_t)i); return; }
    for (size_t j = a->el.size(); j-- > i + 1;) {
        if (a->el[j].t != T_HOLE) {   // added below an existing element: switch to an explicit order
            a->customOrd = true; a->ord.clear();
            for (size_t k = 0; k < a->el.size(); k++) if (a->el[k].t != T_HOLE) a->ord.push_back((uint32_t)k);
            a->ord.push_back((uint32_t)i);
            return;
        }
    }
}
// after el shrank (length / pop): drop the removed indices
void arrTruncOrder(Array* a) {
    if (!a->customOrd) return;
    size_t n = a->el.size(), w = 0;
    for (size_t r = 0; r < a->ord.size(); r++) if (a->ord[r] < n) a->ord[w++] = a->ord[r];
    a->ord.resize(w);
}
// el was rebuilt (shift, unshift, splice, sort, reverse...): back to index order
void arrResetOrder(Array* a) { a->customOrd = false; a->ord.clear(); }

static inline bool isRemovedDO(Object* o) { return o->isDisplay() && ((DisplayObject*)o)->removed; }

Value getProp(Object* o, Str* k) {
    if (FL_UNLIKELY(isRemovedDO(o))) {
        DisplayObject* r = doResolveRemoved((DisplayObject*)o);
        if (!r) return VUNDEF;
        o = r;
    }
    Object* p = o;
    for (int depth = 0; p && depth < 64; depth++, p = p->proto) {
        int i = p->props.find(k);
        if (i >= 0) { const Value& v = p->props.v[i].v; return v.t == T_HOLE ? VUNDEF : v; }
        switch (p->kind) {
            case OK_ARRAY: {
                Array* a = (Array*)p;
                if (k->arrIdx >= 0) { if ((size_t)k->arrIdx < a->el.size()) { const Value& v = a->el[k->arrIdx]; return v.t == T_HOLE ? VUNDEF : v; } }
                else if (k == atoms::length) return Value((double)a->el.size());
                break;
            }
            case OK_BOXED:
                if (k == atoms::length && ((Boxed*)p)->prim.t == T_STR) return Value((double)((Boxed*)p)->prim.s->s.size());
                break;
            default:
                if (p->isDisplay()) { Value out; if (doGetSpecial((DisplayObject*)p, k, out)) return out; }
                else if (p->kind >= OK_BITMAPDATA) {
                    extern bool nativeGet(Object*, Str*, Value&);
                    Value out; if (nativeGet(p, k, out)) return out;
                }
                break;
        }
        if (k == atoms::__proto__) return p->proto ? Value(p->proto) : VUNDEF;
    }
    return VUNDEF;
}

bool hasProp(Object* o, Str* k) {
    if (FL_UNLIKELY(isRemovedDO(o))) { DisplayObject* r = doResolveRemoved((DisplayObject*)o); if (!r) return false; o = r; }
    Object* p = o;
    for (int depth = 0; p && depth < 64; depth++, p = p->proto) {
        int i = p->props.find(k);
        if (i >= 0) return p->props.v[i].v.t != T_HOLE;
        if (p->kind == OK_ARRAY) {
            Array* a = (Array*)p;
            if (k->arrIdx >= 0 && (size_t)k->arrIdx < a->el.size() && a->el[k->arrIdx].t != T_HOLE) return true;
            if (k == atoms::length) return true;
        } else if (p->isDisplay()) {
            if (doHasSpecial((DisplayObject*)p, k)) return true;
        }
    }
    return false;
}

// hasProp(o, k) ? getProp(o, k) in one walk of the prototype chain (variable lookups along the scope chain)
static bool tryGetProp(Object* o, Str* k, Value& out) {
    if (FL_UNLIKELY(isRemovedDO(o))) { DisplayObject* r = doResolveRemoved((DisplayObject*)o); if (!r) return false; o = r; }
    Object* p = o;
    for (int depth = 0; p && depth < 64; depth++, p = p->proto) {
        int i = p->props.find(k);
        if (i >= 0) { const Value& v = p->props.v[i].v; if (v.t == T_HOLE) return false; out = v; return true; }
        if (p->kind == OK_ARRAY) {
            Array* a = (Array*)p;
            if (k->arrIdx >= 0 && (size_t)k->arrIdx < a->el.size() && a->el[k->arrIdx].t != T_HOLE) { out = a->el[k->arrIdx]; return true; }
            if (k == atoms::length) { out = Value((double)a->el.size()); return true; }
        } else if (p->isDisplay()) {
            if (doHasSpecial((DisplayObject*)p, k)) { out = getProp(o, k); return true; }
        }
    }
    return false;
}
WriteGuardFn g_writeGuard = nullptr;
static void setPropRaw(Object* o, Str* k, const Value& v);
void setProp(Object* o, Str* k, const Value& v) {
    if (FL_UNLIKELY(isRemovedDO(o))) { DisplayObject* r = doResolveRemoved((DisplayObject*)o); if (!r) return; o = r; }
    if (FL_UNLIKELY(o->guarded) && g_writeGuard) { Value w = v; if (g_writeGuard(o, k, w)) setPropRaw(o, k, w); return; }
    setPropRaw(o, k, v);
}
static void setPropRaw(Object* o, Str* k, const Value& v) {
    switch (o->kind) {
        case OK_ARRAY: {
            Array* a = (Array*)o;
            if (k->arrIdx >= 0) {
                size_t i = (size_t)k->arrIdx;
                if (i >= a->el.size()) { if (i > 50000000) return; size_t old = a->el.size(); a->el.resize(i + 1); for (size_t j = old; j < i; j++) a->el[j].t = T_HOLE; a->el[i].t = T_HOLE; }
                // new gaps are left as T_HOLE
                if (a->el[i].t == T_HOLE) arrNoteInsert(a, i);
                a->el[i] = v.t == T_HOLE ? VUNDEF : v;
                return;
            }
            if (k == atoms::length) {
                double n = toNum(v); int32_t len = toInt32(n); if (len < 0) len = 0;
                size_t old = a->el.size(); a->el.resize((size_t)len);
                for (size_t j = old; j < (size_t)len; j++) a->el[j].t = T_HOLE;
                if ((size_t)len < old) arrTruncOrder(a);
                return;
            }
            break;
        }
        default:
            if (o->isDisplay()) {
                if (k->dprop >= 0 && o->props.find(k) < 0) { if (doSetSpecial((DisplayObject*)o, k, v)) return; }
            } else if (o->kind >= OK_BITMAPDATA) {
                extern bool nativeSet(Object*, Str*, const Value&);
                if (nativeSet(o, k, v)) return;
            }
            break;
    }
    if (k == atoms::__proto__) { o->proto = v.isObj() ? v.o : nullptr; return; }
    o->props.set(k, v);
}

Str* keyOf(const Value& key) {
    if (key.t == T_STR) return atomOf(key.s);
    if (key.t == T_NUM) {
        double n = key.n;
        if (n >= 0 && n < 2147483647.0 && n == (int64_t)n) {
            static Str* cache[4096] = {};
            int i = (int)n;
            if (i < 4096) { if (!cache[i]) { char b[16]; snprintf(b, sizeof b, "%d", i); cache[i] = intern(b); } return cache[i]; }
            char b[16]; snprintf(b, sizeof b, "%d", i); return intern(b);
        }
        return intern(numToString(n));
    }
    return atomOf(toStr(key));
}

static Value getPrimMember(const Value& v, Str* k) {
    if (v.t == T_STR && k == atoms::length) return Value((double)v.s->s.size());
    Object* p = v.t == T_STR ? protos::string : v.t == T_NUM ? protos::number : v.t == T_BOOL ? protos::boolean : nullptr;
    if (!p) return VUNDEF;
    return getProp(p, k);
}

Value getMemK(const Value& obj, Str* k) {
    if (FL_LIKELY(obj.t == T_OBJ)) return getProp(obj.o, k);
    if (obj.t == T_STR || obj.t == T_NUM || obj.t == T_BOOL) return getPrimMember(obj, k);
    return VUNDEF;
}
Value getMem(const Value& obj, const Value& key) {
    if (obj.t == T_OBJ && obj.o->kind == OK_ARRAY && key.t == T_NUM) {
        Array* a = (Array*)obj.o; double n = key.n;
        if (n >= 0 && n == (int64_t)n) { if ((size_t)n < a->el.size()) { const Value& v = a->el[(size_t)n]; return v.t == T_HOLE ? VUNDEF : v; } return a->props.v.empty() ? VUNDEF : getProp(a, keyOf(key)); }
    }
    if (obj.t != T_OBJ && obj.t != T_STR && obj.t != T_NUM && obj.t != T_BOOL) return VUNDEF;
    return getMemK(obj, keyOf(key));
}
void setMemK(const Value& obj, Str* k, const Value& v) {
    if (FL_LIKELY(obj.t == T_OBJ)) setProp(obj.o, k, v);
    // primitives: assigned on a temporary object (no effect)
}
void setMem(const Value& obj, const Value& key, const Value& v) {
    if (obj.t != T_OBJ) return;
    if (obj.o->kind == OK_ARRAY && key.t == T_NUM && FL_LIKELY(!obj.o->guarded)) {
        Array* a = (Array*)obj.o; double n = key.n;
        if (n >= 0 && n == (int64_t)n && n < 50000000) {
            size_t i = (size_t)n;
            if (i >= a->el.size()) { size_t old = a->el.size(); a->el.resize(i + 1); for (size_t j = old; j < i; j++) a->el[j].t = T_HOLE; a->el[i].t = T_HOLE; }
            if (a->el[i].t == T_HOLE) arrNoteInsert(a, i);
            a->el[i] = v.t == T_HOLE ? VUNDEF : v; return;
        }
    }
    setProp(obj.o, keyOf(key), v);
}

// ============================================================ Variables and scopes
static Value resolvePathProperty(DisplayObject* d, Str* name, bool& found) {
    found = true;
    if (name == atoms::_root) return d ? Value((Object*)doRoot(d)) : Value((Object*)vm.root);
    if (name == atoms::_parent) { DisplayObject* p = d ? doParent(d) : nullptr; return p ? Value((Object*)p) : VUNDEF; }
    if (name == atoms::_global) return Value(vm.global);
    const std::string& s = name->s;
    if (s.size() > 6 && (s.compare(0, 6, "_level") == 0 || s.compare(0, 6, "_flash") == 0)) {
        if (s.substr(6) == "0") return Value((Object*)vm.root);
        return VUNDEF;
    }
    found = false; return VUNDEF;
}

static Value getVarSlow(Act& a, Str* name);

Value getVarK(Act& a, Str* name) {
    if (FL_UNLIKELY(name == atoms::this_)) return a.thisv;
    for (Scope* s = a.scope; s; s = s->parent) {
        Object* v = s->values;
        if (!v) continue;
        if (FL_UNLIKELY(isRemovedDO(v))) { v = a.target && !a.target->removed ? (Object*)a.target : (Object*)vm.root; }
        Value out;
        if (tryGetProp(v, name, out)) return out;
    }
    return getVarSlow(a, name);
}

static Value getVarSlow(Act& a, Str* name) {
    const std::string& s = name->s;
    if (s.find_first_of(":./") != std::string::npos) {
        size_t sep = s.find_last_of(":.");
        if (sep != std::string::npos) {
            if (sep == 0) return VUNDEF;
            Str* var = intern(s.substr(sep + 1));
            Value tgt = resolveTargetPath(a, mkstr(s.substr(0, sep)));
            if (tgt.isObj() && hasProp(tgt.o, var)) return getProp(tgt.o, var);
            return VUNDEF;
        }
        return resolveTargetPath(a, Value(name));
    }
    bool found; Value r = resolvePathProperty(a.target ? a.target : (DisplayObject*)vm.root, name, found);
    return r;
}

Value getVar(Act& a, const Value& name) { return getVarK(a, keyOf(name)); }

void setVarK(Act& a, Str* name, const Value& v) {
    if (FL_UNLIKELY(name->pathy)) {
        const std::string& s = name->s;
        size_t sep = s.find_last_of(":.");
        if (sep != std::string::npos && sep > 0) {
            Value tgt = resolveTargetPath(a, mkstr(s.substr(0, sep)));
            if (tgt.isObj()) setProp(tgt.o, intern(s.substr(sep + 1)), v);
            return;
        }
    }
    for (Scope* sc = a.scope; sc; sc = sc->parent) {
        Object* vals = sc->values;
        bool removed = vals && isRemovedDO(vals);
        if (!removed && (sc->cls == SC_TARGET || (vals && hasProp(vals, name)))) { if (vals) setProp(vals, name, v); return; }
        if (!sc->parent) { if (vals) setProp(vals, name, v); return; }
    }
}
void setVar(Act& a, const Value& name, const Value& v) { setVarK(a, keyOf(name), v); }

void defLocalK(Act& a, Str* name, const Value& v) {
    Scope* sc = a.scope;
    if (!sc->values) sc->values = gcNew<Object>();
    setProp(sc->values, name, v);
}
void defLocal(Act& a, const Value& name, const Value& v) { defLocalK(a, keyOf(name), v); }

// ============================================================ Calls
Value callValue(Act& a, const Value& fn, const Value& thisv, int argc, Value* argv) {
    if (fn.t != T_OBJ || fn.o->kind != OK_FUNCTION) return VUNDEF;
    Function* f = (Function*)fn.o;
    if (f->code) return f->code(a, f, thisv, argc, argv);
    if (f->nat) return f->nat(a, thisv, argc, argv);
    return VUNDEF;
}

Value callFunK(Act& a, Str* name, int argc, Value* argv) {
    // resolve with the object where it was found as this
    for (Scope* s = a.scope; s; s = s->parent) {
        Object* v = s->values;
        if (!v) continue;
        if (isRemovedDO(v)) v = a.target && !a.target->removed ? (Object*)a.target : (Object*)vm.root;
        if (hasProp(v, name)) {
            Value f = getProp(v, name);
            return callValue(a, f, Value(v), argc, argv);
        }
    }
    Value f = getVarSlow(a, name);
    return callValue(a, f, a.target ? Value((Object*)a.target) : VUNDEF, argc, argv);
}
Value callFun(Act& a, const Value& name, int argc, Value* argv) { return callFunK(a, keyOf(name), argc, argv); }

Value callMethodK(Act& a, const Value& obj, Str* name, int argc, Value* argv) {
    if (obj.isNullish()) return VUNDEF;
    if (name->s.empty()) return callValue(a, obj, VUNDEF, argc, argv);
    Value f = getMemK(obj, name);
    if (f.t != T_OBJ) return VUNDEF;
    return callValue(a, f, obj, argc, argv);
}
Value callMethod(Act& a, const Value& obj, const Value& name, int argc, Value* argv) {
    if (obj.isNullish()) return VUNDEF;
    if (name.isUndef()) return callValue(a, obj, VUNDEF, argc, argv);
    return callMethodK(a, obj, keyOf(name), argc, argv);
}

Value construct(Act& a, const Value& ctorv, int argc, Value* argv) {
    if (ctorv.t != T_OBJ || ctorv.o->kind != OK_FUNCTION) return VUNDEF;
    Function* f = (Function*)ctorv.o;
    if (f->ctor) return f->ctor(a, VUNDEF, argc, argv);
    Object* o = gcNew<Object>();
    Value pr = getProp(f, atoms::prototype);
    o->proto = pr.isObj() ? pr.o : protos::object;
    Value r;
    if (f->code) r = f->code(a, f, Value(o), argc, argv);
    else if (f->nat) r = f->nat(a, Value(o), argc, argv);
    if (r.t == T_OBJ) return r;
    return Value(o);
}
Value newObjectK(Act& a, Str* name, int argc, Value* argv) { return construct(a, getVarK(a, name), argc, argv); }
Value newObject(Act& a, const Value& name, int argc, Value* argv) { return newObjectK(a, keyOf(name), argc, argv); }
Value newMethod(Act& a, const Value& obj, const Value& name, int argc, Value* argv) {
    if (obj.isNullish()) return VUNDEF;
    if (name.isUndef() || (name.t == T_STR && name.s->s.empty())) return construct(a, obj, argc, argv);
    return construct(a, getMemK(obj, keyOf(name)), argc, argv);
}

Value makeFunction(Act& a, CodeFn code, const char* name) {
    Function* f = gcNew<Function>();
    f->code = code; f->scope = a.scope; f->baseClip = a.target; f->dbgName = name;
    // the captured scope must live on the heap
    return Value((Object*)f);
}
void defineFunction(Act& a, CodeFn code, Str* name) {
    Value f = makeFunction(a, code, name->s.c_str());
    defLocalK(a, name, f);
}

Value initArray(int n, Value* el) {
    Array* arr = gcNew<Array>();
    arr->el.assign(el, el + n);
    return Value((Object*)arr);
}
Value initObject(int n, Value* kv) {
    Object* o = gcNew<Object>(protos::object);
    // kv in pop order: value, name, value, name... (Ruffle/Flash assign in pop order)
    for (int i = 0; i < n; i++) setProp(o, keyOf(kv[2 * i + 1]), kv[2 * i]);
    return Value(o);
}

// ---- dynamic variants (non-literal argument count): operate on the real stack
static int popCount() { return std::max(0, toInt32(*--vm.sp)); }
void dynCallFunction(Act& a) {
    Value name = *--vm.sp; int n = popCount(); std::vector<Value> args(n); for (int i = 0; i < n; i++) args[i] = *--vm.sp;
    Value r = callFun(a, name, n, args.data()); *vm.sp++ = r;
}
void dynCallMethod(Act& a, int isNew) {
    Value name = *--vm.sp; Value obj = *--vm.sp; int n = popCount(); std::vector<Value> args(n); for (int i = 0; i < n; i++) args[i] = *--vm.sp;
    Value r = isNew ? newMethod(a, obj, name, n, args.data()) : callMethod(a, obj, name, n, args.data()); *vm.sp++ = r;
}
void dynNewObject(Act& a) {
    Value name = *--vm.sp; int n = popCount(); std::vector<Value> args(n); for (int i = 0; i < n; i++) args[i] = *--vm.sp;
    Value r = newObject(a, name, n, args.data()); *vm.sp++ = r;
}
void dynInitArray() { int n = popCount(); std::vector<Value> el(n); for (int i = 0; i < n; i++) el[i] = *--vm.sp; *vm.sp++ = initArray(n, el.data()); }
void dynInitObject() { int n = popCount(); std::vector<Value> kv(2 * n); for (int i = 0; i < 2 * n; i++) kv[i] = *--vm.sp; *vm.sp++ = initObject(n, kv.data()); }
void defLocal2(Act& a, const Value& name) {
    Str* k = keyOf(name); Scope* sc = a.scope;
    if (!sc->values) sc->values = gcNew<Object>();
    if (sc->values->props.find(k) < 0) setProp(sc->values, k, VUNDEF);
}
void traceValue_(const Value& v) { logf("[trace] %s\n", toStr(v)->s.c_str()); }

void collectKeys(Object* o, std::vector<Str*>& keys, int depth) {
    if (!o || depth > 32) return;
    std::vector<Str*> protoKeys;
    if (o->proto) collectKeys(o->proto, protoKeys, depth + 1);
    for (Str* k : protoKeys) if (o->props.find(k) < 0) keys.push_back(k);
    if (o->kind == OK_ARRAY) {
        Array* a = (Array*)o;
        if (a->customOrd) {
            for (size_t r = a->ord.size(); r-- > 0;) { uint32_t i = a->ord[r]; if (i < a->el.size() && a->el[i].t != T_HOLE) keys.push_back(keyOf(Value((double)i))); }
        } else {
            for (size_t i = a->el.size(); i-- > 0;) if (a->el[i].t != T_HOLE) keys.push_back(keyOf(Value((double)i)));
        }
    }
    for (size_t i = o->props.v.size(); i-- > 0;) {
        const Prop& p = o->props.v[i];
        if (!(p.flags & PF_DONTENUM) && p.v.t != T_HOLE) keys.push_back(p.k);
    }
    if (o->isDisplay()) doEnumChildren((DisplayObject*)o, keys);
}

void enumerate2(const Value& obj) {
    *vm.sp++ = VUNDEF;
    if (obj.t != T_OBJ) return;
    std::vector<Str*> keys;
    collectKeys(obj.o, keys, 0);
    for (size_t i = keys.size(); i-- > 0;) *vm.sp++ = Value(keys[i]);
}

Value makeArguments(int argc, Value* argv) { return initArray(argc, argv); }

void enterFunction(Act& a, Scope& local, Act& caller, Function* self, const Value& thisv) {
    local.parent = self->scope ? self->scope : vm.globalScope;
    local.cls = SC_LOCAL; local.values = nullptr;
    a.scope = &local;
    a.thisv = thisv;
    DisplayObject* base = self->baseClip;
    if (base && base->removed) base = doResolveRemoved(base);
    if (!base) base = (thisv.isObj() && thisv.o->isDisplay()) ? (DisplayObject*)thisv.o : caller.target;
    a.target = base;
    a.callee = self;
}

void preloadRegs(Act& a, uint16_t flags, Value* R, int argc, Value* argv) {
    int r = 1;
    if (flags & DF_PRELOAD_THIS) R[r++] = a.thisv;
    if (flags & DF_PRELOAD_ARGS) R[r++] = makeArguments(argc, argv);
    if (flags & DF_PRELOAD_SUPER) R[r++] = VUNDEF;
    if (flags & DF_PRELOAD_ROOT) R[r++] = Value((Object*)(a.target ? doRoot(a.target) : (DisplayObject*)vm.root));
    if (flags & DF_PRELOAD_PARENT) { DisplayObject* p = a.target ? doParent(a.target) : nullptr; R[r++] = p ? Value((Object*)p) : VUNDEF; }
    if (flags & DF_PRELOAD_GLOBAL) R[r++] = Value(vm.global);
}

bool baseRemoved(Act& a) { return a.target && a.target->removed; }

}  // namespace fl
