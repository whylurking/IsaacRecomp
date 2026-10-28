// Debug utilities for test scripts (--script ... eval <expr>)
#include "display.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <sstream>
#include <vector>
#include "menuui.h"
#include "controls.h"
#include "mods.h"
namespace fl {
static Value evalPath(const std::string& p) {
    Value cur((Object*)vm.root);
    size_t i = 0;
    std::string s = p; if (s.compare(0, 6, "_root.") == 0) s = s.substr(6); else if (s == "_root") return cur;
    while (i <= s.size()) {
        size_t j = s.find('.', i); std::string part = s.substr(i, j == std::string::npos ? std::string::npos : j - i);
        if (!part.empty()) { Value prev = cur; cur = getMemK(cur, intern(part)); if (i == 0 && cur.isUndef() && vm.global) cur = getMemK(Value(vm.global), intern(part)); (void)prev; }
        if (j == std::string::npos) break; i = j + 1;
    }
    return cur;
}
static void dumpTree(DisplayObject* d, int depth, int maxDepth) {
    if (depth > maxDepth) return;
    logf("%*s[%d] %s id=%d kind=%d vis=%d x=%.1f y=%.1f sx=%.2f fr=%d/%d clip=%d\n", depth * 2, "", d->depth, d->name ? d->name->s.c_str() : "-", d->charId, d->kind,
         d->visible, d->m.tx, d->m.ty, d->sxP / 100.0, d->kind == OK_CLIP ? ((MovieClip*)d)->cur : 0, d->kind == OK_CLIP ? ((MovieClip*)d)->def->nframes : 0, d->clipDepth);
    if (d->kind == OK_CLIP) for (auto* k : ((MovieClip*)d)->kids) dumpTree(k, depth + 1, maxDepth);
}
bool g_autoAim = false;
// (testing, differential runs) the arrow key that shoots at the nearest living enemy (0: none); same rule as the
// Ruffle harness's "aimkey"
int autoAimKey() {
    double px = toNum(evalPath("_root.a.player.xp")), py = toNum(evalPath("_root.a.player.yp"));
    double best = INFINITY, bx = 0, by = 0;
    Value arr = evalPath("_root.a.ball");
    if (arr.isObj() && arr.o->kind == OK_ARRAY) {
        Array* a = (Array*)arr.o;
        for (size_t i = 0; i < a->el.size(); i++) {
            Value b = a->el[i]; if (!b.isObj()) continue;
            double sv = toNum(getMemK(b, intern("s"))); if (std::isnan(sv)) sv = 0;
            bool dones = truthy(getMemK(b, intern("dones")));
            double hp = toNum(getMemK(b, intern("hp")));
            if (sv < 10.0 || dones || !(hp > 0.0)) continue;
            double x = toNum(getMemK(b, intern("xp"))), y = toNum(getMemK(b, intern("yp")));
            double d = (x - px) * (x - px) + (y - py) * (y - py);
            if (d < best) { best = d; bx = x; by = y; }
        }
    }
    if (!std::isfinite(best)) return 0;
    double dx = bx - px, dy = by - py;
    if (std::fabs(dx) > std::fabs(dy)) return dx > 0 ? 39 : 37;
    return dy > 0 ? 40 : 38;
}
void debugEval(const std::string& e) {
    if (e == "wide minimap") { wideToggleMinimap(); logf("[wide] minimap %s\n", wideMinimapShown() ? "on" : "off"); return; }
    if (e.compare(0, 8, "genesis ") == 0) { genesisDebug(e.substr(8)); return; }   // (GenesisConsole: open, type ..., a command)
    if (e.compare(0, 5, "call ") == 0) {   // call <path> <method> [numeric args...]
        std::istringstream ss(e.substr(5)); std::string path, method; ss >> path >> method;
        std::vector<Value> args; std::string a; while (ss >> a) { double n = strToNum(a); args.push_back(std::isnan(n) ? Value(mkstr(a)) : Value(n)); }
        Act ac{vm.globalScope, VUNDEF, vm.root, nullptr};
        Value r = callMethodK(ac, evalPath(path), intern(method), (int)args.size(), args.data());
        runActions();
        logf("[call] %s = %s\n", e.substr(5).c_str(), toStr(r)->s.c_str());
        return;
    }
    if (e.compare(0, 7, "dumpbd ") == 0) {   // dumpbd <path> <file>: dumps a BitmapData (premultiplied RGBA, raw)
        std::istringstream ss(e.substr(7)); std::string path, file; ss >> path >> file;
        Value v = evalPath(path);
        if (!v.isObj() || v.o->kind != OK_BITMAPDATA) { logf("[dumpbd] %s is not a BitmapData\n", path.c_str()); return; }
        BitmapData* b = (BitmapData*)v.o; b->ensurePx(); FILE* f = fopen(file.c_str(), "wb"); if (!f) return;
        fprintf(f, "%d %d\n", b->w, b->h); fwrite(b->px.data(), 4, b->px.size(), f); fclose(f);
        logf("[dumpbd] %s %dx%d -> %s\n", path.c_str(), b->w, b->h, file.c_str());
        return;
    }
    if (e.compare(0, 10, "hidedepth ") == 0) {   // hidedepth <path> <depth>
        char path[256] = {0}; int dp = 0; sscanf(e.c_str() + 10, "%255s %d", path, &dp); Value v = evalPath(path);
        if (v.isObj() && v.o->kind == OK_CLIP) for (auto* k : ((MovieClip*)v.o)->kids) if (k->depth == dp) k->visible = false;
        return;
    }
    if (e.compare(0, 10, "shapeinfo ") == 0) { extern void debugShapeSubpaths(uint16_t); debugShapeSubpaths((uint16_t)atoi(e.c_str() + 10)); return; }
    if (e.compare(0, 5, "tree ") == 0) { int md = 3; char path[256] = {0}; sscanf(e.c_str() + 5, "%255s %d", path, &md); Value v = evalPath(path);
        if (v.isObj() && v.o->isDisplay()) dumpTree((DisplayObject*)v.o, 0, md); return; }
    if (e.compare(0, 4, "nav ") == 0) { padNavEvent(atoi(e.c_str() + 4)); return; }   // controller navigation (menuui.cpp)
    if (e.compare(0, 6, "rebind") == 0) {   // rebind [nav]: open the rebinding screen / send it a navigation code
        static RebindScreen w;
        if (e.compare(0, 11, "rebind key ") == 0) w.captureKey(atoi(e.c_str() + 11));
        else if (e.compare(0, 11, "rebind pad ") == 0) w.capturePad(atoi(e.c_str() + 11));
        else if (e == "rebind save") bindingsSave([](const std::string& k, const std::string& v) { logf("[ini] %s = %s\n", k.c_str(), v.c_str()); });
        else if (e.size() > 7) w.nav(atoi(e.c_str() + 7));
        else { bool av[DEV_COUNT] = {true, true, true}; w.open(av, DEV_KEYBOARD, 0); }
        return;
    }
    if (e == "touchtargets") {   // (testing) the HUD's tappable areas (stage coordinates)
        extern int recompTouchTargets(float out[4][4]); float r[4][4]; int m = recompTouchTargets(r);
        for (int i = 0; i < 4; i++) if (m & (1 << i)) logf("[touch] target %d: %.0f,%.0f-%.0f,%.0f\n", i, r[i][0], r[i][1], r[i][2], r[i][3]);
        return;
    }
    if (e.compare(0, 6, "props ") == 0) {   // (testing) every enumerable property, sorted (same format as the harness)
        extern void collectKeys(Object* o, std::vector<Str*>& keys, int depth);
        std::string path = e.substr(6); Value v = evalPath(path);
        if (v.isObj()) {
            std::vector<Str*> keys; collectKeys(v.o, keys, 0);
            std::vector<std::pair<std::string, std::string>> kv;
            for (Str* k : keys) { Value x = getMemK(v, k); kv.push_back({k->s, x.isObj() ? std::string("[obj]") : toStr(x)->s}); }
            std::sort(kv.begin(), kv.end());
            for (auto& p : kv) logf("[prop] %s %s = %s\n", path.c_str(), p.first.c_str(), p.second.c_str());
        }
        return;
    }
    if (e.compare(0, 9, "hitdebug ") == 0) { char path[256]; double x, y; if (sscanf(e.c_str() + 9, "%255s %lf %lf", path, &x, &y) == 3) { Value v = evalPath(path);
        extern void debugHit(DisplayObject*, double, double); if (v.isObj() && v.o->isDisplay()) debugHit((DisplayObject*)v.o, x, y); } return; }
    if (e.compare(0, 5, "copy ") == 0) {   // copy <dst path> <src path>
        std::istringstream ss(e.substr(5)); std::string dst, src; ss >> dst >> src;
        size_t dot = dst.find_last_of('.'); Value obj = dot == std::string::npos ? Value((Object*)vm.root) : evalPath(dst.substr(0, dot));
        if (obj.isObj()) setMemK(obj, intern(dot == std::string::npos ? dst : dst.substr(dot + 1)), evalPath(src));
        return;
    }
    if (e.compare(0, 6, "setat ") == 0) {   // setat <array path> <index path> <value>
        std::istringstream ss(e.substr(6)); std::string arr, idx, val; ss >> arr >> idx >> val;
        Value a = evalPath(arr); double n = strToNum(val);
        if (a.isObj()) setMemK(a, intern(toStr(evalPath(idx))->s), std::isnan(n) ? Value(mkstr(val)) : Value(n));
        return;
    }
    if (e == "aim 1" || e == "aim 0") { extern bool g_autoAim; g_autoAim = e == "aim 1"; return; }
    if (e == "rng") { extern uint64_t g_rngCalls; logf("[rng] %llu\n", (unsigned long long)g_rngCalls); return; }
    if (e == "balls") {   // (testing: differential runs against Ruffle) the game's enemy list, same format as the harness
        static const char* kF[] = {"e", "s", "xp", "yp", "xbew", "ybew", "hp", "fra", "_x", "_y", "_currentframe", "d._currentframe", "d.d._currentframe",
                                   "_rotation", "_xscale", "_alpha", "_visible", "d._x", "d._y", "d._rotation", "d._xscale", "d._visible", "dones"};
        Value arr = evalPath("_root.a.ball");
        if (arr.isObj() && arr.o->kind == OK_ARRAY) {
            Array* a = (Array*)arr.o;
            for (size_t i = 0; i < a->el.size(); i++) {
                std::string line = "[ball] " + std::to_string(g_player.tick) + " " + std::to_string(i) + ":";
                Value b = a->el[i];
                // a dead clip reference that resolves to nothing is not an object (Ruffle's as_object)
                if (b.isObj() && b.o->isDisplay() && ((DisplayObject*)b.o)->removed && !doResolveRemoved((DisplayObject*)b.o)) b = VUNDEF;
                if (b.isObj()) for (const char* f : kF) {
                    Value v = b; std::string fs = f; size_t p0 = 0;
                    while (true) { size_t d = fs.find('.', p0); std::string part = fs.substr(p0, d == std::string::npos ? std::string::npos : d - p0);
                        v = v.isObj() ? getMemK(v, intern(part)) : VUNDEF; if (d == std::string::npos) break; p0 = d + 1; }
                    line += std::string(" ") + f + "=" + toStr(v)->s;
                }
                logf("%s\n", line.c_str());
            }
        }
        return;
    }
    if (e.compare(0, 7, "bounds ") == 0) {   // bounds <path>: a display object's bounds in its parent's space
        Value v = evalPath(e.substr(7));
        if (v.isObj() && v.o->isDisplay()) { RectF r = localBounds((DisplayObject*)v.o); logf("[bounds] %s %.1f %.1f %.1f %.1f\n", e.substr(7).c_str(), r.x0, r.y0, r.x1, r.y1); }
        return;
    }
    if (e == "pickups") {   // (testing) the pickups of the room: s 5 entries of _root.a.ball
        static const char* kF[] = {"xp", "yp", "d._currentframe", "d.d._currentframe", "d.d.d._currentframe", "it", "col", "empty", "alt", "done", "dones"};
        Value arr = evalPath("_root.a.ball");
        if (arr.isObj() && arr.o->kind == OK_ARRAY) {
            Array* a = (Array*)arr.o;
            for (size_t i = 0; i < a->el.size(); i++) {
                Value b = a->el[i];
                if (!b.isObj() || toNum(getMemK(b, intern("s"))) != 5) continue;
                std::string line = "[pickup] " + std::to_string(i) + ":";
                for (const char* f : kF) {
                    Value v = b; std::string fs = f; size_t p0 = 0;
                    while (true) { size_t d = fs.find('.', p0); std::string part = fs.substr(p0, d == std::string::npos ? std::string::npos : d - p0);
                        v = v.isObj() ? getMemK(v, intern(part)) : VUNDEF; if (d == std::string::npos) break; p0 = d + 1; }
                    line += std::string(" ") + f + "=" + toStr(v)->s;
                }
                logf("%s\n", line.c_str());
            }
        }
        return;
    }
    if (e == "focus") { extern void menuDebugFocus(); menuDebugFocus(); return; }
    if (e == "buttons") { extern void debugListButtons(); debugListButtons(); return; }
    if (e.compare(0, 4, "set ") == 0) {
        size_t sp = e.find(' ', 4); std::string path = e.substr(4, sp - 4), val = e.substr(sp + 1);
        size_t dot = path.find_last_of('.'); Value obj = evalPath(path.substr(0, dot));
        double n = strToNum(val); setMemK(obj, intern(path.substr(dot + 1)), std::isnan(n) ? Value(mkstr(val)) : Value(n));
        return;
    }
    Value v = evalPath(e);
    logf("[eval] %s = %s\n", e.c_str(), toStr(v)->s.c_str());
}
}
