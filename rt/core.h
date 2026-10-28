// Core of the native AVM1 runtime: values, strings, objects, GC, scopes and operations.
// Semantics follow Flash Player 8 / Ruffle (SWF8, case-sensitive).
#pragma once
#include <cstdint>
#include <cstring>
#include <cmath>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

#if defined(__GNUC__)
#define FL_LIKELY(x) __builtin_expect(!!(x), 1)
#define FL_UNLIKELY(x) __builtin_expect(!!(x), 0)
#define FL_NOINLINE __attribute__((noinline))
#else
#define FL_LIKELY(x) (x)
#define FL_UNLIKELY(x) (x)
#define FL_NOINLINE
#endif

namespace fl {

struct Str; struct Object; struct Function; struct Scope; struct DisplayObject; struct MovieClip;

// ---------------------------------------------------------------- GC
struct Tracer;
struct GcObj {
    uint8_t marked = 0;
    uint8_t pinned = 0;     // never freed (interned strings, prototypes)
    uint8_t onHeap = 0;     // 0 = stack object (local scopes), not managed by the GC
    uint8_t weakAtom = 0;   // interned string the GC may free (automatic instance names): pinned when scripts use it
    virtual ~GcObj() {}
    virtual void trace(Tracer&) {}
};
struct Tracer {
    std::vector<GcObj*> stack; size_t count = 0;
    std::unordered_map<GcObj*, GcObj*>* pred = nullptr; GcObj* cur = nullptr;   // diagnostics (ISAAC_GCPATH)
    inline void mark(GcObj* o) { if (o && o->onHeap && !o->marked) { o->marked = 1; count++; stack.push_back(o); if (pred) (*pred)[o] = cur; } }
};
void gcRegister(GcObj* o);
template <class T, class... A> inline T* gcNew(A&&... a) { T* o = new T(std::forward<A>(a)...); o->onHeap = 1; gcRegister(o); return o; }
void gcCollect(double tickMs = 0); // only between frames (no live activations); tickMs = logic time already spent this tick
void gcAddRoot(GcObj** slot);     // static root
void gcPin(GcObj* o);             // never freed and traced as a root (prototypes, native functions)
size_t gcCount();
void gcExternalAlloc(size_t bytes);   // external memory (pixels) that brings the next collection forward

// ---------------------------------------------------------------- Strings
struct Str final : GcObj {
    std::string s;
    uint32_t hash = 0;
    int32_t arrIdx = -1;     // canonical array index ("0","12"...) or -1
    int8_t dprop = -1;       // display property index (_x=0...), case-insensitive, or -1
    bool interned = false;
    bool pathy = false;      // contains ":" "." or "/" (target path)
    Str* atom = nullptr;     // interned version (cache)
    explicit Str(std::string v) : s(std::move(v)) {}
    size_t len() const { return s.size(); }
};
Str* intern(std::string_view sv);
Str* mkstr(std::string s);                  // dynamic string (not interned)
Str* internWeak(std::string_view sv);       // interned, but freed when nothing references it (until a script uses it as a key)
inline Str* atomOf(Str* s) { if (s->interned) { if (s->weakAtom) { s->weakAtom = 0; s->pinned = 1; } return s; } if (!s->atom) s->atom = intern(s->s); return s->atom; }

// ---------------------------------------------------------------- Values
enum VT : uint8_t { T_UNDEF = 0, T_NULL, T_BOOL, T_NUM, T_STR, T_OBJ, T_HOLE };
struct Value {
    VT t;
    union { bool b; double n; Str* s; Object* o; };
    constexpr Value() : t(T_UNDEF), n(0) {}
    constexpr Value(double d) : t(T_NUM), n(d) {}
    Value(int i) : t(T_NUM), n((double)i) {}
    Value(Str* x) : t(x ? T_STR : T_UNDEF) { s = x; }
    Value(Object* x) : t(x ? T_OBJ : T_UNDEF) { o = x; }
    static Value boolean(bool v) { Value r; r.t = T_BOOL; r.b = v; return r; }
    static Value null() { Value r; r.t = T_NULL; return r; }
    bool isUndef() const { return t == T_UNDEF || t == T_HOLE; }
    bool isNullish() const { return t == T_UNDEF || t == T_NULL || t == T_HOLE; }
    bool isObj() const { return t == T_OBJ; }
    bool isStr() const { return t == T_STR; }
    bool isNum() const { return t == T_NUM; }
};
inline void traceValue(Tracer& tr, const Value& v) { if (v.t == T_OBJ) tr.mark((GcObj*)v.o); else if (v.t == T_STR) tr.mark(v.s); }
#define VUNDEF ::fl::Value()
#define VNULL ::fl::Value::null()
#define VTRUE ::fl::Value::boolean(true)
#define VFALSE ::fl::Value::boolean(false)
#define VB(x) ::fl::Value::boolean(x)

// ---------------------------------------------------------------- Ordered property map
enum PropFlag : uint8_t { PF_DONTENUM = 1 };
struct Prop { Str* k; Value v; uint8_t flags; };
struct PropMap {
    std::vector<Prop> v;
    // open-addressing index (keys = interned pointers), built once there are more than 8 properties
    uint32_t* tab = nullptr; uint32_t mask = 0;
    PropMap() {}
    PropMap(const PropMap&) = delete;
    ~PropMap() { delete[] tab; }
    static inline uint32_t hp(Str* k) {
        if constexpr (sizeof(void*) == 4) { uint32_t x = ((uint32_t)(uintptr_t)k >> 4) * 0x9E3779B1u; return (x >> 16) | (x << 16); }   // (PSP: no 64-bit multiply)
        else { uint64_t x = (uint64_t)(uintptr_t)k >> 4; x *= 0x9E3779B97F4A7C15ull; return (uint32_t)(x >> 32); }
    }
    void rebuild() {
        uint32_t cap = 16; while (cap < v.size() * 3) cap <<= 1;
        delete[] tab; tab = new uint32_t[cap]; mask = cap - 1;
        memset(tab, 0xff, cap * sizeof(uint32_t));
        for (uint32_t i = 0; i < v.size(); i++) { uint32_t h = hp(v[i].k) & mask; while (tab[h] != 0xffffffffu) h = (h + 1) & mask; tab[h] = i; }
    }
    int find(Str* k) const {
        if (tab) {
            uint32_t h = hp(k) & mask;
            while (true) { uint32_t i = tab[h]; if (i == 0xffffffffu) return -1; if (v[i].k == k) return (int)i; h = (h + 1) & mask; }
        }
        for (size_t i = 0, n = v.size(); i < n; i++) if (v[i].k == k) return (int)i;
        return -1;
    }
    Value* getp(Str* k) { int i = find(k); return i < 0 ? nullptr : &v[i].v; }
    void set(Str* k, const Value& val, uint8_t fl = 0) {
        int i = find(k);
        if (i >= 0) { v[i].v = val; return; }
        v.push_back({k, val, fl});
        if (tab) {
            if (v.size() * 2 > mask + 1) rebuild();
            else { uint32_t h = hp(k) & mask; while (tab[h] != 0xffffffffu) h = (h + 1) & mask; tab[h] = (uint32_t)(v.size() - 1); }
        } else if (v.size() > 8) rebuild();
    }
    bool erase(Str* k) {
        int i = find(k); if (i < 0) return false;
        v.erase(v.begin() + i);
        if (tab) rebuild();
        return true;
    }
    void trace(Tracer& tr) const { for (auto& p : v) { tr.mark(p.k); traceValue(tr, p.v); } }
};

// ---------------------------------------------------------------- Objects
enum OK : uint8_t {
    OK_PLAIN = 0, OK_ARRAY, OK_FUNCTION, OK_BOXED, OK_BITMAPDATA, OK_SOUND, OK_TRANSFORM, OK_SHAREDOBJ,
    OK_COLOR,
    OK_DISPLAY_FIRST = 32,
    OK_CLIP = 32, OK_BUTTON, OK_EDITTEXT, OK_SHAPE, OK_MORPH, OK_TEXT, OK_BITMAP,
};
struct Object : GcObj {
    OK kind = OK_PLAIN;
    uint8_t guarded = 0;   // writes to its properties go through g_writeGuard first (mods: GenesisConsole)
    Object* proto = nullptr;
    PropMap props;
    Object() {}
    explicit Object(Object* p) : proto(p) {}
    bool isDisplay() const { return kind >= OK_DISPLAY_FIRST; }
    void trace(Tracer& tr) override { tr.mark(proto); props.trace(tr); }
};
struct Array final : Object {
    std::vector<Value> el;   // T_HOLE = missing
    // for-in order (Flash/Ruffle: most recently added element first). While elements are only added in ascending
    // index order, that is descending index order and `ord` is unused; once an element is added below an existing
    // one (e.g. `levz = new Array(200)` filled in tile order), `ord` lists the element indices in insertion order.
    std::vector<uint32_t> ord;
    bool customOrd = false;
    Array();
    void trace(Tracer& tr) override { Object::trace(tr); for (auto& x : el) traceValue(tr, x); }
};
struct Boxed final : Object {   // boxed String/Number/Boolean
    Value prim;
    void trace(Tracer& tr) override { Object::trace(tr); traceValue(tr, prim); }
};

struct VMState;
struct Act;
using NativeFn = Value (*)(Act& a, Value thisv, int argc, Value* argv);
using CodeFn = Value (*)(Act& caller, Function* self, Value thisv, int argc, Value* argv);
struct Function final : Object {
    NativeFn nat = nullptr;
    NativeFn ctor = nullptr;       // native constructor (new X()): receives an already-created this or undefined
    CodeFn code = nullptr;
    Scope* scope = nullptr;        // captured scope
    DisplayObject* baseClip = nullptr;
    const char* dbgName = "";
    Function();
    void trace(Tracer& tr) override;
};

// ---------------------------------------------------------------- Scopes and activations
enum SC : uint8_t { SC_GLOBAL, SC_TARGET, SC_LOCAL };
struct Scope final : GcObj {
    Scope* parent = nullptr;
    SC cls = SC_LOCAL;
    Object* values = nullptr;      // created lazily for SC_LOCAL
    Scope() {}
    Scope(Scope* p, SC c, Object* v) : parent(p), cls(c), values(v) {}
    void trace(Tracer& tr) override { tr.mark(parent); tr.mark(values); }
};

// Activation: one per action block or function call (lives on the C++ stack).
struct Act {
    Scope* scope;
    Value thisv;
    DisplayObject* target;   // base clip / target
    Function* callee;
};

// ---------------------------------------------------------------- VM stack (for for-in and && / ||)
struct VMState {
#ifdef __PSP__
    Value stack[1 << 15];   // (PSP: 512 KB)
#else
    Value stack[1 << 16];
#endif
    Value* sp = stack;
    Object* global = nullptr;
    Scope* globalScope = nullptr;
    MovieClip* root = nullptr;
    uint32_t swfVersion = 8;
};
extern VMState vm;

// ---------------------------------------------------------------- Conversions
double toNum(const Value& v);
Str* toStr(const Value& v);
std::string numToString(double n);
double strToNum(const std::string& s);
inline bool truthy(const Value& v) {
    switch (v.t) {
        case T_BOOL: return v.b;
        case T_NUM: return v.n != 0 && !std::isnan(v.n);
        case T_STR: return !v.s->s.empty();
        case T_OBJ: return true;
        default: return false;
    }
}
int toInt32(double d);   // (int, not int32_t: on PSP int32_t is long)
inline int toInt32(const Value& v) { return toInt32(toNum(v)); }
Value toPrimitive(const Value& v);
Object* toObject(const Value& v);   // primitives → Boxed; undefined/null → nullptr

// ---------------------------------------------------------------- Operations
Value add2(const Value& l, const Value& r);
Value lt(const Value& a, const Value& b);           // abstract_lt(a,b): Bool or undefined
bool eq2(const Value& a, const Value& b);
bool strictEq(const Value& a, const Value& b);
inline Value less2(const Value& l, const Value& r) { return lt(l, r); }
inline Value greater(const Value& l, const Value& r) { return lt(r, l); }
inline Value sub(const Value& l, const Value& r) { return Value(toNum(l) - toNum(r)); }
inline Value mul(const Value& l, const Value& r) { return Value(toNum(l) * toNum(r)); }
inline Value divv(const Value& l, const Value& r) { return Value(toNum(l) / toNum(r)); }
inline Value modv(const Value& l, const Value& r) { return Value(std::fmod(toNum(l), toNum(r))); }
inline Value notv(const Value& v) { return VB(!truthy(v)); }
inline Value andv(const Value& l, const Value& r) { return VB(truthy(l) && truthy(r)); }
inline Value orv(const Value& l, const Value& r) { return VB(truthy(l) || truthy(r)); }
inline Value incv(const Value& v) { return Value(toNum(v) + 1); }
inline Value decv(const Value& v) { return Value(toNum(v) - 1); }
inline Value toIntegerv(const Value& v) { return Value((double)toInt32(v)); }
Value randomNumber(const Value& max);
Value getTime();
double randomDouble();   // [0,1)
void rngSave(uint32_t* state, uint64_t* calls);   // (mods: side-effect-free calls into the game)
void rngRestore(uint32_t state, uint64_t calls);
Str* typeOf(const Value& v);

// Properties
Value getProp(Object* o, Str* k);                  // with prototype chain
bool hasProp(Object* o, Str* k);
void setProp(Object* o, Str* k, const Value& v);
// Write guard (mods): for objects with `guarded` set, every property write (setProp, setMem*, setVar*) is first passed
// to it. It may change the value; returning false drops the write. Null when no mod uses it.
using WriteGuardFn = bool (*)(Object* o, Str* k, Value& v);
extern WriteGuardFn g_writeGuard;
Value getMem(const Value& obj, const Value& key);
Value getMemK(const Value& obj, Str* k);
void setMem(const Value& obj, const Value& key, const Value& v);
void setMemK(const Value& obj, Str* k, const Value& v);
Str* keyOf(const Value& key);                      // key → atom

// Variables
Value getVarK(Act& a, Str* name);
void setVarK(Act& a, Str* name, const Value& v);
void defLocalK(Act& a, Str* name, const Value& v);
Value getVar(Act& a, const Value& name);
void setVar(Act& a, const Value& name, const Value& v);
void defLocal(Act& a, const Value& name, const Value& v);

// Calls
Value callValue(Act& a, const Value& fn, const Value& thisv, int argc, Value* argv);
Value callFunK(Act& a, Str* name, int argc, Value* argv);
Value callFun(Act& a, const Value& name, int argc, Value* argv);
Value callMethodK(Act& a, const Value& obj, Str* name, int argc, Value* argv);
Value callMethod(Act& a, const Value& obj, const Value& name, int argc, Value* argv);
Value newObjectK(Act& a, Str* name, int argc, Value* argv);
Value newObject(Act& a, const Value& name, int argc, Value* argv);
Value newMethod(Act& a, const Value& obj, const Value& name, int argc, Value* argv);
Value construct(Act& a, const Value& ctor, int argc, Value* argv);
Value makeFunction(Act& a, CodeFn code, const char* name);
void defineFunction(Act& a, CodeFn code, Str* name);
Value initArray(int n, Value* el);
Value initObject(int n, Value* kv);   // kv: [v0,k0,v1,k1...] in pop order
void arrNoteInsert(Array* a, size_t i);   // for-in order bookkeeping (Array::ord)
void arrTruncOrder(Array* a);
void arrResetOrder(Array* a);
void enumerate2(const Value& obj);    // pushes undefined + keys onto vm.sp

// Building activations for compiled functions
// DefineFunction2 flags
enum : uint16_t { DF_PRELOAD_THIS = 1, DF_SUPPRESS_THIS = 2, DF_PRELOAD_ARGS = 4, DF_SUPPRESS_ARGS = 8,
    DF_PRELOAD_SUPER = 16, DF_SUPPRESS_SUPER = 32, DF_PRELOAD_ROOT = 64, DF_PRELOAD_PARENT = 128, DF_PRELOAD_GLOBAL = 256 };
void enterFunction(Act& a, Scope& local, Act& caller, Function* self, const Value& thisv);
void preloadRegs(Act& a, uint16_t flags, Value* R, int argc, Value* argv);
Value makeArguments(int argc, Value* argv);
bool baseRemoved(Act& a);

// Timeline actions (implemented in display.cpp)
void actGotoFrame(Act& a, int frame0);
void actGotoFrame2(Act& a, const Value& f, bool play, int bias);
void actGotoLabel(Act& a, Str* label);
void actStop(Act& a);
void actPlay(Act& a);
void actNextFrame(Act& a);
void actPrevFrame(Act& a);
void actGetURL(Act& a, const char* url, const char* target);
void actRemoveSprite(Act& a, const Value& target);
Value actGetProperty(Act& a, const Value& target, const Value& idx);
void actSetProperty(Act& a, const Value& target, const Value& idx, const Value& v);

// Display object hooks (display.cpp)
bool doGetSpecial(DisplayObject* d, Str* k, Value& out);
bool doHasSpecial(DisplayObject* d, Str* k);
bool doSetSpecial(DisplayObject* d, Str* k, const Value& v);   // true if it was a display property
void doEnumChildren(DisplayObject* d, std::vector<Str*>& keys);
bool doRemoved(DisplayObject* d);
DisplayObject* doResolveRemoved(DisplayObject* d);   // dead reference → clip with the same path, or nullptr
Str* doPath(DisplayObject* d);
DisplayObject* doRoot(DisplayObject* d);
DisplayObject* doParent(DisplayObject* d);
Value resolveTargetPath(Act& a, const Value& path);   // "/a/b", "_root.a", clip...

// Common atoms
namespace atoms {
extern Str *length, *prototype, *__proto__, *constructor, *this_, *_root, *_parent, *_global, *onEnterFrame, *onLoad,
    *valueOf, *toString, *undefined_, *null_, *true_, *false_, *empty, *NaN_;
}
void initAtoms();

// Prototypes
namespace protos { extern Object *object, *array, *function, *string, *number, *boolean, *movieclip, *button, *textfield,
    *bitmapdata, *sound, *color, *transform, *colortransform, *matrix, *rectangle, *point, *sharedobject; }

// Debug utilities
void logf(const char* fmt, ...);
extern bool g_trace;

}  // namespace fl
