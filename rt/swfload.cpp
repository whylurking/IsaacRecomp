// SWF loader: builds the character definitions from the user's original file.
#include "swfdata.h"
#include <chrono>
#include "core.h"
#include <cstdio>
#include <cstring>
#include <map>
#include <algorithm>
#include <mutex>
#include <atomic>
#ifdef FL_LEAN
#include <zlib.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#include "../third_party/stb_image.h"

namespace fl {
Movie g_movie;

namespace {
struct Rd {
    const uint8_t* d; size_t p, end; uint32_t bitbuf = 0; int bitpos = 0;
    Rd(const uint8_t* data, size_t s, size_t e) : d(data), p(s), end(e) {}
    void align() { bitpos = 0; }
    uint8_t u8() { align(); return p < end ? d[p++] : 0; }
    uint16_t u16() { align(); uint16_t v = p + 1 < end ? (uint16_t)(d[p] | d[p + 1] << 8) : 0; p += 2; return v; }
    int16_t s16() { return (int16_t)u16(); }
    uint32_t u32() { align(); uint32_t v = p + 3 < end ? (uint32_t)(d[p] | d[p + 1] << 8 | d[p + 2] << 16 | (uint32_t)d[p + 3] << 24) : 0; p += 4; return v; }
    float fixed() { return (int32_t)u32() / 65536.0f; }
    float fixed8() { return (int16_t)u16() / 256.0f; }
    float f32() { uint32_t v = u32(); float f; memcpy(&f, &v, 4); return f; }
    std::string str() { align(); size_t s = p; while (p < end && d[p]) p++; std::string r((const char*)d + s, p - s); p++; return r; }
    uint32_t ub(int n) {
        uint32_t v = 0;
        for (int i = 0; i < n; i++) {
            if (bitpos == 0) { bitbuf = p < end ? d[p] : 0; p++; bitpos = 8; }
            bitpos--; v = (v << 1) | ((bitbuf >> bitpos) & 1);
        }
        return v;
    }
    int32_t sb(int n) { if (!n) return 0; uint32_t v = ub(n); if (v & (1u << (n - 1))) v |= ~0u << n; return (int32_t)v; }
    RectF rect() { align(); int n = ub(5); RectF r; r.x0 = sb(n) / 20.0; r.x1 = sb(n) / 20.0; r.y0 = sb(n) / 20.0; r.y1 = sb(n) / 20.0; align(); return r; }
    Mat matrix() {
        align(); Mat m;
        if (ub(1)) { int n = ub(5); m.a = sb(n) / 65536.0; m.d = sb(n) / 65536.0; }
        if (ub(1)) { int n = ub(5); m.b = sb(n) / 65536.0; m.c = sb(n) / 65536.0; }
        int n = ub(5); m.tx = sb(n) / 20.0; m.ty = sb(n) / 20.0; align(); return m;
    }
    CX cxform(bool alpha) {
        align(); CX c; bool ha = ub(1), hm = ub(1); int n = ub(4);
        if (hm) { c.rm = sb(n) / 256.f; c.gm = sb(n) / 256.f; c.bm = sb(n) / 256.f; if (alpha) c.am = sb(n) / 256.f; }
        if (ha) { c.ra = (float)sb(n); c.ga = (float)sb(n); c.ba = (float)sb(n); if (alpha) c.aa = (float)sb(n); }
        align(); return c;
    }
    uint32_t rgb() { uint8_t r = u8(), g = u8(), b = u8(); return 0xff000000u | r << 16 | g << 8 | b; }
    uint32_t rgba() { uint8_t r = u8(), g = u8(), b = u8(), a = u8(); return (uint32_t)a << 24 | r << 16 | g << 8 | b; }
};

std::map<uint64_t, FrameScript> g_scriptMap;
std::map<uint32_t, FrameScript> g_buttonMap;

void readFilters(Rd& r, std::vector<Filter>& out) {
    int n = r.u8();
    for (int i = 0; i < n; i++) {
        Filter f; f.type = r.u8();
        switch (f.type) {
            case 0: { f.color = r.rgba(); f.blurX = r.fixed(); f.blurY = r.fixed(); f.angle = r.fixed(); f.distance = r.fixed(); f.strength = r.fixed8();
                uint8_t fl = r.u8(); f.inner = fl & 0x80; f.knockout = fl & 0x40; f.passes = fl & 31; break; }
            case 1: { f.blurX = r.fixed(); f.blurY = r.fixed(); f.passes = r.u8() >> 3; break; }
            case 2: { f.color = r.rgba(); f.blurX = r.fixed(); f.blurY = r.fixed(); f.strength = r.fixed8();
                uint8_t fl = r.u8(); f.inner = fl & 0x80; f.knockout = fl & 0x40; f.passes = fl & 31; break; }
            case 3: { r.rgba(); f.color = r.rgba(); f.blurX = r.fixed(); f.blurY = r.fixed(); f.angle = r.fixed(); f.distance = r.fixed(); f.strength = r.fixed8(); r.u8(); break; }
            case 4: case 7: { int nc = r.u8(); for (int k = 0; k < nc; k++) r.rgba(); for (int k = 0; k < nc; k++) r.u8();
                f.blurX = r.fixed(); f.blurY = r.fixed(); f.angle = r.fixed(); f.distance = r.fixed(); f.strength = r.fixed8(); r.u8(); break; }
            case 5: { int mx = r.u8(), my = r.u8(); r.f32(); r.f32(); for (int k = 0; k < mx * my; k++) r.f32(); r.rgba(); r.u8(); break; }
            case 6: { for (int k = 0; k < 20; k++) r.f32(); break; }
            default: return;
        }
        out.push_back(f);
    }
}

void readPlace(Rd& r, int tag, size_t end, PlaceCmd& o) {
    if (tag == 4) {
        o.id = r.u16(); o.hasId = true; o.depth = r.u16(); o.m = r.matrix(); o.hasMatrix = true;
        if (r.p < end) { o.cx = r.cxform(false); o.hasCx = true; }
        return;
    }
    uint8_t fl = r.u8(), fl2 = tag == 70 ? r.u8() : 0;
    o.depth = r.u16(); o.move = fl & 1;
    if (tag == 70 && ((fl2 & 8) || ((fl2 & 16) && (fl & 2)))) r.str();
    if (fl & 2) { o.id = r.u16(); o.hasId = true; }
    if (fl & 4) { o.m = r.matrix(); o.hasMatrix = true; }
    if (fl & 8) { o.cx = r.cxform(true); o.hasCx = true; }
    if (fl & 16) { o.ratio = r.u16(); o.hasRatio = true; }
    if (fl & 32) { o.name = intern(r.str()); o.hasName = true; }
    if (fl & 64) { o.clipDepth = r.u16(); o.hasClipDepth = true; }
    if (tag == 70) {
        if (fl2 & 1) { readFilters(r, o.filters); o.hasFilters = true; }
        if (fl2 & 2) { o.blend = r.u8(); o.hasBlend = true; }
        if (fl2 & 4) r.u8();
    }
    // clip actions don't appear in this game
}

void readFillStyles(Rd& r, int shapeTag, std::vector<FillStyle>& out, bool morph = false, std::vector<FillStyle>* out1 = nullptr) {
    int n = r.u8();
    if (n == 255 && (shapeTag != 2 || morph)) n = r.u16();
    bool alpha = shapeTag >= 32 || morph;
    for (int i = 0; i < n; i++) {
        FillStyle f, f1; f.type = r.u8(); f1.type = f.type;
        if (f.type == 0) { f.argb = alpha ? r.rgba() : r.rgb(); if (morph) f1.argb = r.rgba(); }
        else if (f.type == 0x10 || f.type == 0x12 || f.type == 0x13) {
            f.m = r.matrix(); if (morph) f1.m = r.matrix();
            r.align(); uint8_t g = r.u8(); f.spread = g >> 6; f.interp = (g >> 4) & 3; int k = g & 15;
            if (morph) { f.spread = f1.spread = 0; k = g; }
            for (int j = 0; j < k; j++) {
                GradStop s; s.pos = r.u8() / 255.f; s.argb = alpha ? r.rgba() : r.rgb(); f.stops.push_back(s);
                if (morph) { GradStop s1; s1.pos = r.u8() / 255.f; s1.argb = r.rgba(); f1.stops.push_back(s1); }
            }
            if (f.type == 0x13) f.focal = r.fixed8();
        } else if (f.type >= 0x40) {
            f.bitmapId = r.u16(); f.m = r.matrix(); if (morph) { f1.bitmapId = f.bitmapId; f1.m = r.matrix(); }
        }
        out.push_back(f); if (out1) out1->push_back(f1);
    }
}
void readLineStyles(Rd& r, int shapeTag, std::vector<LineStyle>& out, bool morph = false, std::vector<LineStyle>* out1 = nullptr) {
    int n = r.u8(); if (n == 255) n = r.u16();
    for (int i = 0; i < n; i++) {
        LineStyle l, l1;
        l.width = r.u16() / 20.f; if (morph) l1.width = r.u16() / 20.f;
        if (shapeTag == 83 || shapeTag == 84) {
            r.align(); l.startCap = r.ub(2); l.join = r.ub(2); l.hasFill = r.ub(1); r.ub(1); r.ub(1); r.ub(1); r.ub(5);
            l.noClose = r.ub(1); l.endCap = r.ub(2); r.align();
            if (l.join == 2) l.miter = r.fixed8();
            if (l.hasFill) { std::vector<FillStyle> fs, fs1; Rd tmp = r; (void)tmp;
                // a single FILLSTYLE
                FillStyle f; f.type = r.u8();
                if (f.type == 0) { f.argb = r.rgba(); if (morph) r.rgba(); }
                else if (f.type == 0x10 || f.type == 0x12 || f.type == 0x13) {
                    f.m = r.matrix(); if (morph) r.matrix(); uint8_t g = r.u8(); int k = morph ? g : (g & 15);
                    for (int j = 0; j < k; j++) { GradStop s; s.pos = r.u8() / 255.f; s.argb = r.rgba(); f.stops.push_back(s); if (morph) { r.u8(); r.rgba(); } }
                    if (f.type == 0x13) f.focal = r.fixed8();
                } else if (f.type >= 0x40) { f.bitmapId = r.u16(); f.m = r.matrix(); if (morph) r.matrix(); }
                l.fill = f; l.argb = f.type == 0 ? f.argb : (f.stops.empty() ? 0xff000000 : f.stops[0].argb);
            } else { l.argb = r.rgba(); if (morph) l1.argb = r.rgba(); }
        } else {
            l.argb = (shapeTag >= 32 || morph) ? r.rgba() : r.rgb();
            if (morph) l1.argb = r.rgba();
        }
        l1.startCap = l.startCap; l1.endCap = l.endCap; l1.join = l.join;
        out.push_back(l); if (out1) out1->push_back(l1);
    }
}

// Directed edge in twips
struct Edge { int32_t x0, y0, cx, cy, x1, y1; bool curve; };
inline uint64_t ptKey(int32_t x, int32_t y) { return (uint64_t)(uint32_t)x << 32 | (uint32_t)y; }

void buildFillPaths(std::vector<std::vector<Edge>>& perFill, std::vector<Path>& out) {
    out.resize(perFill.size());
    for (size_t fi = 0; fi < perFill.size(); fi++) {
        auto& edges = perFill[fi];
        if (edges.empty()) continue;
        Path& path = out[fi];
        std::unordered_multimap<uint64_t, uint32_t> byStart; byStart.reserve(edges.size() * 2);
        for (uint32_t i = 0; i < edges.size(); i++) byStart.emplace(ptKey(edges[i].x0, edges[i].y0), i);
        std::vector<uint8_t> used(edges.size(), 0);
        for (uint32_t i = 0; i < edges.size(); i++) {
            if (used[i]) continue;
            int32_t sx = edges[i].x0, sy = edges[i].y0;
            path.verbs.push_back(0); path.pts.push_back(sx / 20.f); path.pts.push_back(sy / 20.f);
            uint32_t cur = i;
            while (true) {
                used[cur] = 1;
                const Edge& e = edges[cur];
                if (e.curve) { path.verbs.push_back(2); path.pts.insert(path.pts.end(), {e.cx / 20.f, e.cy / 20.f, e.x1 / 20.f, e.y1 / 20.f}); }
                else { path.verbs.push_back(1); path.pts.insert(path.pts.end(), {e.x1 / 20.f, e.y1 / 20.f}); }
                if (e.x1 == sx && e.y1 == sy) break;
                auto range = byStart.equal_range(ptKey(e.x1, e.y1));
                int64_t next = -1;
                for (auto it = range.first; it != range.second; ++it) if (!used[it->second]) { next = it->second; break; }
                if (next < 0) break;
                cur = (uint32_t)next;
            }
        }
    }
}

// Reads SHAPERECORDs. If glyph=true, there are no styles (font).
void readShapeRecords(Rd& r, int shapeTag, std::vector<ShapeLayer>& layers, int fillBits, int lineBits, std::vector<HitEdge>* hit = nullptr) {
    ShapeLayer* L = &layers.back();
    std::vector<std::vector<Edge>> fillEdges(L->fills.size() + 1);
    std::vector<Path> linePaths(L->lines.size() + 1);
    auto flushLayer = [&]() {
        std::vector<std::vector<Edge>> pf(fillEdges.begin() + 1, fillEdges.end());
        buildFillPaths(pf, L->fillPaths);
        L->linePaths.assign(linePaths.begin() + 1, linePaths.end());
    };
    int32_t x = 0, y = 0; int f0 = 0, f1 = 0, ln = 0;
    int32_t lastLx = INT32_MIN, lastLy = INT32_MIN; int lastLine = -1;
    auto addLine = [&](const Edge& e) {
        if (ln <= 0 || ln >= (int)linePaths.size()) return;
        Path& p = linePaths[ln];
        if (lastLine != ln || lastLx != e.x0 || lastLy != e.y0) { p.verbs.push_back(0); p.pts.push_back(e.x0 / 20.f); p.pts.push_back(e.y0 / 20.f); }
        if (e.curve) { p.verbs.push_back(2); p.pts.insert(p.pts.end(), {e.cx / 20.f, e.cy / 20.f, e.x1 / 20.f, e.y1 / 20.f}); }
        else { p.verbs.push_back(1); p.pts.insert(p.pts.end(), {e.x1 / 20.f, e.y1 / 20.f}); }
        lastLine = ln; lastLx = e.x1; lastLy = e.y1;
    };
    while (true) {
        if (r.p >= r.end) break;
        bool edge = r.ub(1);
        if (!edge) {
            uint32_t flags = r.ub(5);
            if (!flags) break;
            if (flags & 1) { int n = r.ub(5); x = r.sb(n); y = r.sb(n); }
            if (flags & 2) f0 = r.ub(fillBits);
            if (flags & 4) f1 = r.ub(fillBits);
            if (flags & 8) ln = r.ub(lineBits);
            if ((flags & 16) && hit) hit->push_back(HitEdge{0, 0, 0, 0, 0, 0, 2, false, false, -1});
            if (flags & 16) {
                flushLayer();
                layers.emplace_back(); L = &layers.back();
                readFillStyles(r, shapeTag, L->fills); readLineStyles(r, shapeTag, L->lines);
                r.align(); fillBits = r.ub(4); lineBits = r.ub(4);
                fillEdges.assign(L->fills.size() + 1, {}); linePaths.assign(L->lines.size() + 1, {});
                lastLine = -1;
            }
            continue;
        }
        Edge e; e.x0 = x; e.y0 = y;
        bool straight = r.ub(1); int nb = r.ub(4) + 2;
        if (straight) {
            int dx = 0, dy = 0;
            if (r.ub(1)) { dx = r.sb(nb); dy = r.sb(nb); }
            else if (r.ub(1)) dy = r.sb(nb); else dx = r.sb(nb);
            x += dx; y += dy; e.curve = false; e.cx = e.cy = 0;
        } else {
            int cdx = r.sb(nb), cdy = r.sb(nb), adx = r.sb(nb), ady = r.sb(nb);
            e.cx = x + cdx; e.cy = y + cdy; x = e.cx + adx; y = e.cy + ady; e.curve = true;
        }
        e.x1 = x; e.y1 = y;
        if (hit) {
            int32_t sw = (ln > 0 && ln - 1 < (int)L->lines.size()) ? (int32_t)std::lround(L->lines[ln - 1].width * 20.0) : -1;
            hit->push_back(HitEdge{e.x0, e.y0, e.cx, e.cy, e.x1, e.y1, (uint8_t)(e.curve ? 1 : 0), f0 > 0, f1 > 0, sw});
        }
        if (f1 > 0 && f1 < (int)fillEdges.size()) fillEdges[f1].push_back(e);
        if (f0 > 0 && f0 < (int)fillEdges.size()) {
            Edge rv{e.x1, e.y1, e.cx, e.cy, e.x0, e.y0, e.curve}; fillEdges[f0].push_back(rv);
        }
        addLine(e);
    }
    flushLayer();
}

// Flattened polygons for hit testing
void flatten(const Path& p, std::vector<float>& out) {
    size_t k = 0; float cx = 0, cy = 0;
    for (uint8_t v : p.verbs) {
        if (v == 0) { cx = p.pts[k]; cy = p.pts[k + 1]; k += 2; out.push_back(NAN); out.push_back(NAN); out.push_back(cx); out.push_back(cy); }
        else if (v == 1) { cx = p.pts[k]; cy = p.pts[k + 1]; k += 2; out.push_back(cx); out.push_back(cy); }
        else {
            float qx = p.pts[k], qy = p.pts[k + 1], ex = p.pts[k + 2], ey = p.pts[k + 3]; k += 4;
            for (int i = 1; i <= 6; i++) { float t = i / 6.f, u = 1 - t;
                out.push_back(u * u * cx + 2 * u * t * qx + t * t * ex); out.push_back(u * u * cy + 2 * u * t * qy + t * t * ey); }
            cx = ex; cy = ey;
        }
    }
}

ShapeDef* parseShape(Rd& r, int tag) {
    auto* s = new ShapeDef(); s->type = CH_SHAPE; s->id = r.u16();
    if (g_leanSwf) {   // records parsed on demand (shapeEnsure); only the bounds now
        s->rawTag = (uint16_t)tag; s->parsed = false;
        s->bounds = r.rect();
        s->raw = nullptr; s->rawOff = r.u32(); s->rawLen = r.u32();   // stub (leanTags): the records stay in the cache file
        return s;
    }
    s->bounds = r.rect();
    if (tag == 83) { r.rect(); r.u8(); }
    s->layers.emplace_back();
    readFillStyles(r, tag, s->layers.back().fills); readLineStyles(r, tag, s->layers.back().lines);
    r.align(); int fb = r.ub(4), lb = r.ub(4);
    readShapeRecords(r, tag, s->layers, fb, lb, &s->hitEdges);
    return s;   // (hit polygons are computed the first time they're needed: shapeHitPolys)
}

}  // namespace
// ---- lean loading: shapes parsed on demand, dropped again (least recently used first) above a memory budget
bool g_leanSwf = false;
static uint32_t g_shapeUseClock = 0;
static size_t g_shapeParsedBytes = 0;
static std::vector<ShapeDef*> g_shapeParsedList;
size_t g_shapeBudgetBytes = 6u << 20;
static size_t shapeBytes(const ShapeDef* s) {
    size_t n = sizeof(HitEdge) * s->hitEdges.capacity();
    for (auto& L : s->layers) {
        n += 64 + (L.fills.capacity() + L.lines.capacity()) * 48;
        for (auto& P : L.fillPaths) n += P.pts.capacity() * sizeof(float) + P.verbs.capacity() + 32;
        for (auto& P : L.linePaths) n += P.pts.capacity() * sizeof(float) + P.verbs.capacity() + 32;
    }
    for (auto& hp : s->hitPolys) n += hp.capacity() * sizeof(float);
    return n;
}
static void shapeDrop(ShapeDef* s) {
    std::vector<ShapeLayer>().swap(s->layers); std::vector<HitEdge>().swap(s->hitEdges);
    std::vector<std::vector<float>>().swap(s->hitPolys); s->hitBuilt = false; s->parsed = false;
    g_shapeParsedBytes -= std::min<size_t>(g_shapeParsedBytes, s->parsedBytes); s->parsedBytes = 0;
}
bool g_shapeEvict = false;   // drop parsed shapes over the budget: only with a single thread using them (PSP)
static std::mutex g_shapeMu;
static bool leanRead(uint32_t off, uint32_t len, std::vector<uint8_t>& out);
extern double g_lazyParseMs;
uint32_t g_shapeParses = 0;   // (diagnostics) shapes parsed (again)
uint32_t g_shapeTick = 0;     // (PSP) the logic tick: shapes used in the last 2 seconds are never dropped
void shapeEnsure(ShapeDef* s) {
    if (!s) return;
    s->lastUse = g_shapeEvict ? g_shapeTick : ++g_shapeUseClock;
    if (s->parsed) return;
    std::lock_guard<std::mutex> lk(g_shapeMu);
    if (s->parsed) return;
    auto tp0 = std::chrono::steady_clock::now();
    struct AddMs { std::chrono::steady_clock::time_point t; ~AddMs() { extern double g_lazyParseMs; g_lazyParseMs += std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t).count(); } } addMs{tp0};
    std::vector<uint8_t> buf;
    if (!s->raw && !leanRead(s->rawOff, s->rawLen, buf)) { s->parsed = true; return; }   // (read error: drawn empty)
    Rd r(s->raw ? s->raw : buf.data(), 0, s->rawLen); int tag = s->rawTag;
    s->bounds = r.rect();
    if (tag == 83) { r.rect(); r.u8(); }
    s->layers.emplace_back();
    readFillStyles(r, tag, s->layers.back().fills); readLineStyles(r, tag, s->layers.back().lines);
    r.align(); int fb = r.ub(4), lb = r.ub(4);
    readShapeRecords(r, tag, s->layers, fb, lb, &s->hitEdges);
    s->parsedBytes = (uint32_t)shapeBytes(s);
    std::atomic_thread_fence(std::memory_order_release);
    s->parsed = true;
    g_shapeParsedBytes += s->parsedBytes; g_shapeParsedList.push_back(s);
    g_shapeParses++;
    if (g_shapeEvict && g_shapeParsedBytes > g_shapeBudgetBytes) {   // drop the least recently used, down to 3/4 of the budget
        std::sort(g_shapeParsedList.begin(), g_shapeParsedList.end(), [](ShapeDef* a, ShapeDef* b) { return a->lastUse < b->lastUse; });
        size_t target = g_shapeBudgetBytes * 3 / 4, w = 0;
        for (size_t i = 0; i < g_shapeParsedList.size(); i++) {
            ShapeDef* x = g_shapeParsedList[i];
            // (what is on screen now isn't dropped, even over the budget: parsing it again every tick would be far worse)
            if (g_shapeParsedBytes > target && x != s && !x->pinned && x->parsed && x->lastUse + 60 < g_shapeTick) shapeDrop(x);
            else if (x->parsed) g_shapeParsedList[w++] = x;
        }
        g_shapeParsedList.resize(w);
    }
}
namespace {
MorphDef* parseMorph(Rd& r, int tag, size_t end) {
    auto* m = new MorphDef(); m->type = CH_MORPH; m->id = r.u16();
    m->b0 = r.rect(); m->b1 = r.rect();
    if (tag == 84) { r.rect(); r.rect(); r.u8(); }
    uint32_t off = r.u32(); size_t endEdgesPos = r.p + off;
    readFillStyles(r, 46, m->fills0, true, &m->fills1);
    readLineStyles(r, tag, m->lines0, true, &m->lines1);
    // start edges
    struct E { uint8_t kind; int32_t x0, y0, cx, cy, x1, y1; int f0, f1, ln; bool mv; };
    auto readEdges = [&](Rd& rr, bool withStyles) {
        std::vector<E> out; rr.align(); int fb = rr.ub(4), lb = rr.ub(4);
        int32_t x = 0, y = 0; int f0 = 0, f1 = 0, ln = 0; bool mv = true;
        while (rr.p < rr.end) {
            if (!rr.ub(1)) {
                uint32_t fl = rr.ub(5); if (!fl) break;
                if (fl & 1) { int n = rr.ub(5); x = rr.sb(n); y = rr.sb(n); mv = true; }
                if (fl & 2) f0 = rr.ub(fb); if (fl & 4) f1 = rr.ub(fb); if (fl & 8) ln = rr.ub(lb);
                (void)withStyles; continue;
            }
            E e{}; e.x0 = x; e.y0 = y; bool st = rr.ub(1); int nb = rr.ub(4) + 2;
            if (st) { int dx = 0, dy = 0; if (rr.ub(1)) { dx = rr.sb(nb); dy = rr.sb(nb); } else if (rr.ub(1)) dy = rr.sb(nb); else dx = rr.sb(nb);
                x += dx; y += dy; e.kind = 1; e.cx = (e.x0 + x) / 2; e.cy = (e.y0 + y) / 2; }
            else { int a = rr.sb(nb), b = rr.sb(nb), c = rr.sb(nb), d = rr.sb(nb); e.cx = x + a; e.cy = y + b; x = e.cx + c; y = e.cy + d; e.kind = 2; }
            e.x1 = x; e.y1 = y; e.f0 = f0; e.f1 = f1; e.ln = ln; e.mv = mv; mv = false; out.push_back(e);
        }
        return out;
    };
    Rd r0 = r; r0.end = endEdgesPos;
    auto s = readEdges(r0, true);
    Rd r1(r.d, endEdgesPos, end);
    auto e = readEdges(r1, false);
    size_t n = std::min(s.size(), e.size());
    for (size_t i = 0; i < n; i++) {
        MorphEdge me; me.kind = (s[i].kind == 2 || e[i].kind == 2) ? 2 : 1;
        me.x0[0] = s[i].x0 / 20.f; me.y0[0] = s[i].y0 / 20.f; me.cx[0] = s[i].cx / 20.f; me.cy[0] = s[i].cy / 20.f; me.x1[0] = s[i].x1 / 20.f; me.y1[0] = s[i].y1 / 20.f;
        me.x0[1] = e[i].x0 / 20.f; me.y0[1] = e[i].y0 / 20.f; me.cx[1] = e[i].cx / 20.f; me.cy[1] = e[i].cy / 20.f; me.x1[1] = e[i].x1 / 20.f; me.y1[1] = e[i].y1 / 20.f;
        me.fill0 = s[i].f0; me.fill1 = s[i].f1; me.line = s[i].ln; me.newPath = s[i].mv;
        m->edges.push_back(me);
    }
    return m;
}

void parseFont3(Rd& r, size_t end, int tag) {
    auto* f = new FontDef(); f->type = CH_FONT; f->id = r.u16();
    uint8_t fl = r.u8(); r.u8(); int nl = r.u8(); f->name.assign((const char*)r.d + r.p, nl); r.p += nl;
    f->hasLayout = fl & 0x80; bool wideOff = fl & 0x08, wideCodes = fl & 0x04; f->italic = fl & 2; f->bold = fl & 1;
    f->font3 = tag == 75;
    int n = r.u16(); size_t tableStart = r.p;
    std::vector<uint32_t> offs(n + 1);
    for (int i = 0; i < n; i++) offs[i] = wideOff ? r.u32() : r.u16();
    uint32_t codeOff = n ? (wideOff ? r.u32() : r.u16()) : 0;
    offs[n] = codeOff;
    float em = f->font3 ? 20480.f : 1024.f;
    for (int i = 0; i < n; i++) {
        Rd g(r.d, tableStart + offs[i], tableStart + offs[i + 1]);
        g.align(); int fb = g.ub(4), lb = g.ub(4);
        std::vector<ShapeLayer> layers(1);
        layers[0].fills.resize(1); layers[0].lines.resize(1);
        readShapeRecords(g, 22, layers, fb, lb);
        Path p;
        for (auto& L : layers) for (auto& fp : L.fillPaths) { p.verbs.insert(p.verbs.end(), fp.verbs.begin(), fp.verbs.end()); p.pts.insert(p.pts.end(), fp.pts.begin(), fp.pts.end()); }
        for (auto& v : p.pts) v = v * 20.f / em;   // to EM units (1.0 = font size)
        f->glyphs.push_back(std::move(p));
    }
    r.p = tableStart + codeOff;
    for (int i = 0; i < n; i++) { uint16_t c = wideCodes ? r.u16() : r.u8(); f->codes.push_back(c); f->codeToIdx[c] = i; }
    if (f->hasLayout) {
        f->ascent = r.u16() / em * 20.f / 20.f; f->descent = r.u16() / em; f->leading = r.s16() / em;
        f->ascent = f->ascent;  // already normalized
        for (int i = 0; i < n; i++) f->advances.push_back(r.s16() / em);
    } else f->advances.assign(n, 0.6f);
    (void)end;
    if ((size_t)f->id < g_movie.chars.size()) g_movie.chars[f->id] = f;
}

void parseText(Rd& r, int tag) {
    auto* t = new TextDef(); t->type = CH_TEXT; t->id = r.u16();
    t->bounds = r.rect(); t->m = r.matrix();
    int gb = r.u8(), ab = r.u8();
    uint16_t font = 0; float size = 12; uint32_t col = 0xff000000; float x = 0, y = 0;
    while (true) {
        uint8_t fl = r.u8(); if (!fl) break;
        if (fl & 8) font = r.u16();
        if (fl & 4) col = tag == 33 ? r.rgba() : r.rgb();
        if (fl & 1) x = r.s16() / 20.f;
        if (fl & 2) y = r.s16() / 20.f;
        if (fl & 8) size = r.u16() / 20.f;
        int n = r.u8();
        for (int i = 0; i < n; i++) {
            TextGlyph g; g.idx = r.ub(gb); int adv = r.sb(ab);
            g.x = x; g.y = y; g.size = size; g.argb = col; g.font = font; t->glyphs.push_back(g);
            x += adv / 20.f;
        }
        r.align();
    }
    g_movie.chars[t->id] = t;
}

void parseEditText(Rd& r) {
    auto* e = new EditTextDef(); e->type = CH_EDITTEXT; e->id = r.u16();
    e->bounds = r.rect();
    uint8_t f1 = r.u8(), f2 = r.u8();
    e->hasText = f1 & 0x80; e->wordWrap = f1 & 0x40; e->multiline = f1 & 0x20; e->password = f1 & 0x10; e->readOnly = f1 & 0x08;
    e->autoSize = f2 & 0x40; e->noSelect = f2 & 0x10; e->border = f2 & 0x08; e->html = f2 & 0x02; e->useOutlines = f2 & 0x01;
    if (f1 & 1) e->fontId = r.u16();
    if (f2 & 0x80) r.str();
    if (f1 & 1) e->height = r.u16() / 20.f;
    if (f1 & 4) e->argb = r.rgba();
    if (f1 & 2) r.u16();
    if (f2 & 0x20) { e->align = r.u8(); e->leftMargin = r.u16() / 20.f; e->rightMargin = r.u16() / 20.f; e->indent = r.u16() / 20.f; e->leading = r.s16() / 20.f; }
    e->variable = r.str();
    if (e->hasText) e->initialText = r.str();
    g_movie.chars[e->id] = e;
}

void parseButton2(Rd& r, size_t end) {
    auto* b = new ButtonDef(); b->type = CH_BUTTON; b->id = r.u16();
    b->trackAsMenu = r.u8() & 1;
    size_t actStart = r.p; uint16_t off = r.u16(); actStart += off;
    while (true) {
        uint8_t fl = r.u8(); if (!fl) break;
        ButtonRecord rec; rec.states = fl & 15; rec.id = r.u16(); rec.depth = r.u16(); rec.m = r.matrix(); rec.cx = r.cxform(true);
        if (fl & 0x10) readFilters(r, rec.filters);
        if (fl & 0x20) r.u8();
        b->records.push_back(rec);
    }
    if (off) {
        size_t p = actStart; int idx = 0;
        while (p + 4 <= end) {
            uint16_t sz = r.d[p] | r.d[p + 1] << 8; uint16_t cond = r.d[p + 2] | r.d[p + 3] << 8;
            auto it = g_buttonMap.find((uint32_t)b->id << 16 | idx);
            b->conds.push_back({cond, it == g_buttonMap.end() ? nullptr : it->second});
            idx++;
            if (!sz) break;
            p += sz;
        }
    }
    g_movie.chars[b->id] = b;
}

void parseTimeline(const uint8_t* d, size_t p, size_t end, SpriteDef* sp, bool isRoot);

}  // namespace

void BitmapDef::decode() {
    if (decoded) return;
    decoded = true;
    if (tag == 20 || tag == 36) {
        Rd r(data, 0, size); r.u16(); int fmt = r.u8(); w = r.u16(); h = r.u16();
        int ncol = fmt == 3 ? r.u8() + 1 : 0;
        int outlen = 0;
        char* z = stbi_zlib_decode_malloc((const char*)data + r.p, (int)(size - r.p), &outlen);
        if (!z) { w = h = 0; return; }
        const uint8_t* u = (const uint8_t*)z;
        px.assign((size_t)w * h, 0);
        bool alpha = tag == 36;
        if (fmt == 3) {
            int cs = alpha ? 4 : 3; std::vector<uint32_t> pal(ncol);
            for (int i = 0; i < ncol; i++) {
                uint8_t R = u[i * cs], G = u[i * cs + 1], B = u[i * cs + 2], A = alpha ? u[i * cs + 3] : 255;
                // Lossless2: palette colors are already premultiplied
                pal[i] = (uint32_t)A << 24 | R << 16 | G << 8 | B;
            }
            int stride = (w + 3) & ~3; const uint8_t* pix = u + ncol * cs;
            for (int y = 0; y < h; y++) for (int x = 0; x < w; x++) { int ix = pix[y * stride + x]; px[y * w + x] = ix < ncol ? pal[ix] : 0; }
        } else if (fmt == 5) {
            for (int i = 0; i < w * h; i++) {
                uint8_t A = u[i * 4], R = u[i * 4 + 1], G = u[i * 4 + 2], B = u[i * 4 + 3];
                if (!alpha) A = 255;
                px[i] = (uint32_t)A << 24 | R << 16 | G << 8 | B;
            }
        } else if (fmt == 4) {
            int stride = (w * 2 + 3) & ~3;
            for (int y = 0; y < h; y++) for (int x = 0; x < w; x++) {
                uint16_t v = u[y * stride + x * 2] << 8 | u[y * stride + x * 2 + 1];
                uint8_t R = ((v >> 10) & 31) * 255 / 31, G = ((v >> 5) & 31) * 255 / 31, B = (v & 31) * 255 / 31;
                px[y * w + x] = 0xff000000u | R << 16 | G << 8 | B;
            }
        }
        free(z);
    } else {   // JPEG (DefineBits + JPEGTables / DefineBitsJPEG2/3)
        std::vector<uint8_t> jpg;
        const uint8_t* img = data + 2; size_t ilen = size - 2; size_t alphaOff = 0;
        if (tag == 35) { uint32_t o = img[0] | img[1] << 8 | img[2] << 16 | (uint32_t)img[3] << 24; img += 4; ilen -= 4; alphaOff = o; ilen = o; }
        auto strip = [](const uint8_t* s, size_t n, std::vector<uint8_t>& o) {
            // strips bogus FFD9FFD8 markers at the start
            if (n >= 4 && s[0] == 0xFF && s[1] == 0xD9 && s[2] == 0xFF && s[3] == 0xD8) { s += 4; n -= 4; }
            o.insert(o.end(), s, s + n);
        };
        if (tag == 6) {
            extern std::vector<uint8_t> g_jpegTables;
            if (!g_jpegTables.empty()) {
                std::vector<uint8_t> t; strip(g_jpegTables.data(), g_jpegTables.size(), t);
                if (t.size() >= 2 && t[t.size() - 2] == 0xFF && t[t.size() - 1] == 0xD9) t.resize(t.size() - 2);
                jpg = t;
                std::vector<uint8_t> im; strip(img, ilen, im);
                if (im.size() >= 2 && im[0] == 0xFF && im[1] == 0xD8) jpg.insert(jpg.end(), im.begin() + 2, im.end());
                else jpg.insert(jpg.end(), im.begin(), im.end());
            } else strip(img, ilen, jpg);
        } else strip(img, ilen, jpg);
        int comp = 0;
        unsigned char* pixels = stbi_load_from_memory(jpg.data(), (int)jpg.size(), &w, &h, &comp, 4);
        if (!pixels) { w = h = 0; return; }
        px.resize((size_t)w * h);
        std::vector<uint8_t> alphaPlane;
        if (tag == 35 && alphaOff) {
            int ol = 0; char* z = stbi_zlib_decode_malloc((const char*)img + alphaOff, (int)(size - 6 - alphaOff), &ol);
            if (z) { alphaPlane.assign(z, z + ol); free(z); }
        }
        for (int i = 0; i < w * h; i++) {
            uint32_t A = alphaPlane.size() > (size_t)i ? alphaPlane[i] : 255;
            uint32_t R = pixels[i * 4] * A / 255, G = pixels[i * 4 + 1] * A / 255, B = pixels[i * 4 + 2] * A / 255;
            px[i] = A << 24 | R << 16 | G << 8 | B;
        }
        stbi_image_free(pixels);
    }
    if (kPxABGR) for (auto& p : px) p = pxSwapRB(p);   // (memory order: see kPxABGR)
}

std::vector<uint8_t> g_jpegTables;

namespace {
void parseTimeline(const uint8_t* d, size_t p, size_t end, SpriteDef* sp, bool isRoot) {
    int frame = 0;   // 0-based
    sp->frames.assign(std::max(sp->nframes, 1), {});
    int actionIdx = 0;
    StreamInfo* stream = nullptr;
    uint32_t streamSample = 0;
    while (p + 2 <= end) {
        uint16_t h = d[p] | d[p + 1] << 8; p += 2;
        int tag = h >> 6; size_t len = h & 63;
        if (len == 63) { len = d[p] | d[p + 1] << 8 | d[p + 2] << 16 | (size_t)d[p + 3] << 24; p += 4; }
        size_t b = p, e = p + len; p = e;
        Rd r(d, b, e);
        if (tag == 0) break;
        auto curFrame = [&]() -> std::vector<FrameCmd>& { if (frame >= (int)sp->frames.size()) sp->frames.resize(frame + 1); return sp->frames[frame]; };
        switch (tag) {
            case 1: frame++; actionIdx = 0; break;
            case 4: case 26: case 70: {
                PlaceCmd pc; readPlace(r, tag, e, pc);
                sp->places.push_back(pc);
                FrameCmd fc; fc.kind = FrameCmd::PLACE; fc.place = (int)sp->places.size() - 1; curFrame().push_back(fc);
                break;
            }
            case 5: case 28: {
                FrameCmd fc; fc.kind = FrameCmd::REMOVE; if (tag == 5) r.u16(); fc.depth = r.u16(); curFrame().push_back(fc); break;
            }
            case 12: {
                FrameCmd fc; fc.kind = FrameCmd::ACTION;
                auto it = g_scriptMap.find((uint64_t)sp->id << 32 | (uint64_t)(frame + 1) << 16 | actionIdx);
                fc.script = it == g_scriptMap.end() ? nullptr : it->second;
                if (!fc.script && e > b + 1) logf("warning: no compiled script for sprite %d frame %d #%d\n", sp->id, frame + 1, actionIdx);
                actionIdx++;
                if (fc.script) curFrame().push_back(fc);
                break;
            }
            case 15: {
                FrameCmd fc; fc.kind = FrameCmd::SOUND; fc.soundId = r.u16(); uint8_t fl = r.u8(); fc.soundFlags = fl;
                if (fl & 1) fc.inPoint = r.u32(); if (fl & 2) fc.outPoint = r.u32(); if (fl & 4) fc.loops = r.u16();
                if (fl & 8) { int n = r.u8(); for (int i = 0; i < n; i++) { uint32_t at = r.u32(); uint32_t l = r.u16(); uint32_t rr = r.u16(); fc.envelope.push_back({at, l, rr}); } }
                curFrame().push_back(fc); break;
            }
            case 43: { std::string lbl = r.str(); for (auto& ch : lbl) ch = (char)tolower((unsigned char)ch);   // (AVM1 labels are case-insensitive)
                       if (!sp->labels.count(lbl)) sp->labels[lbl] = frame + 1; break; }
            case 18: case 45: {
                r.u8(); uint8_t f = r.u8(); r.u16();
                if ((f >> 4) == 2) {
                    sp->stream.reset(new StreamInfo()); stream = sp->stream.get();
                    static const int rates[4] = {5512, 11025, 22050, 44100};
                    stream->rate = rates[(f >> 2) & 3]; stream->stereo = f & 1;
                    stream->frameSample.assign(sp->frames.size() + 1, 0); stream->frameBlock.assign(sp->frames.size() + 1, -1);
                }
                break;
            }
            case 19: {
                if (!stream) break;
                uint16_t sc = r.u16(); r.s16();
                if ((size_t)frame >= stream->frameBlock.size()) { stream->frameBlock.resize(frame + 1, -1); stream->frameSample.resize(frame + 1, 0); }
                stream->frameSample[frame] = streamSample;
                if (g_leanSwf) {   // stub: (offset in the cache file, length)
                    uint32_t off = r.u32(), ln = r.u32();
                    stream->frameBlock[frame] = (int32_t)stream->leanSize;
                    stream->fileBlocks.push_back({off, ln}); stream->leanSize += ln;
                } else {
                    stream->frameBlock[frame] = (int32_t)stream->mp3.size();
                    stream->mp3.insert(stream->mp3.end(), d + r.p, d + e);
                }
                streamSample += sc;
                break;
            }
            case 39: {
                if (!isRoot) break;
                auto* s2 = new SpriteDef(); s2->type = CH_SPRITE; s2->id = r.u16(); s2->nframes = r.u16();
                g_movie.chars[s2->id] = s2;
                if (g_leanSwf) { s2->tlData = d; s2->tlBeg = r.p; s2->tlEnd = e; }   // (parsed when first instantiated)
                else parseTimeline(d, r.p, e, s2, false);
                break;
            }
            case 2: case 22: case 32: case 83: if (isRoot) { auto* s = parseShape(r, tag); g_movie.chars[s->id] = s; } break;
            case 46: case 84: if (isRoot) { auto* m = parseMorph(r, tag, e); g_movie.chars[m->id] = m; } break;
            case 6: case 21: case 35: case 20: case 36: {
                auto* bm = new BitmapDef(); bm->type = CH_BITMAP; bm->id = d[b] | d[b + 1] << 8; bm->data = d + b; bm->size = len; bm->tag = tag;
                g_movie.chars[bm->id] = bm; break;
            }
            case 8: g_jpegTables.assign(d + b, d + e); break;
            case 14: {
                auto* s = new SoundDef(); s->type = CH_SOUND; s->id = r.u16(); uint8_t f = r.u8();
                s->format = f >> 4; s->rate = (f >> 2) & 3; s->is16 = f & 2; s->stereo = f & 1; s->samples = r.u32();
                if (s->format == 2) { s->seek = r.s16(); }
                if (g_leanSwf) { s->fileOff = r.u32(); s->size = r.u32(); s->data = nullptr; }   // stub
                else { s->data = d + r.p; s->size = e - r.p; }
                g_movie.chars[s->id] = s; break;
            }
            case 10: case 48: case 75: if (isRoot) parseFont3(r, e, tag); break;
            case 11: case 33: parseText(r, tag); break;
            case 37: parseEditText(r); break;
            case 34: parseButton2(r, e); break;
            case 56: {
                int n = r.u16();
                for (int i = 0; i < n; i++) { uint16_t id = r.u16(); std::string nm = r.str(); g_movie.exports[nm] = id;
                    if (auto* c = g_movie.get(id)) if (c->type == CH_SPRITE) ((SpriteDef*)c)->exportName = intern(nm); }
                break;
            }
            case 9: { uint8_t R = d[b], G = d[b + 1], B = d[b + 2]; g_movie.bgColor = 0xff000000u | R << 16 | G << 8 | B; break; }
            default: break;
        }
    }
    if (sp->frames.size() < (size_t)sp->nframes) sp->frames.resize(sp->nframes);
    sp->places.shrink_to_fit(); for (auto& f : sp->frames) f.shrink_to_fit();
}
}  // namespace
double g_lazyParseMs = 0;   // (diagnostics) time parsing timelines and shapes on demand
void spriteEnsure(SpriteDef* sp) {
    if (!sp->tlData) return;
    auto t0 = std::chrono::steady_clock::now();
    const uint8_t* d = sp->tlData; sp->tlData = nullptr;
    parseTimeline(d, sp->tlBeg, sp->tlEnd, sp, false);
    g_lazyParseMs += std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
}
namespace {
// The Steam game doesn't ship a standalone .swf: Isaac.exe is an MDM Zinc projector that carries the SWF (CWS) at the end
// of the executable, preceded by its length and encrypted with an XOR key k(i) = a + b*i (mod 256, from the first
// byte of the SWF). b = 0x21 and a changes with each executable (0xd5 the original, 0x83 Wrath of the Lamb, 0xa1 Eternal),
// so it is deduced from the header ("CWS"/"FWS"). Adobe projectors (SWF as is +
// 0xFA123456 + length) and an unencrypted SWF after the executable are also accepted.
static uint32_t rd32(const uint8_t* p) { return (uint32_t)p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }
static size_t peImageEnd(const uint8_t* d, size_t n) {
    size_t start = 0;
    if (n > 0x40) {
        uint32_t pe = rd32(d + 0x3c);
        if ((size_t)pe + 24 < n && d[pe] == 'P' && d[pe + 1] == 'E') {
            unsigned ns = d[pe + 6] | d[pe + 7] << 8, opt = d[pe + 20] | d[pe + 21] << 8;
            size_t sec = pe + 24 + opt;
            for (unsigned i = 0; i < ns && sec + 40 * i + 24 <= n; i++) start = std::max(start, (size_t)(rd32(d + sec + 40 * i + 16) + rd32(d + sec + 40 * i + 20)));
        }
    }
    return start;
}
struct ZincHit { size_t off; uint32_t len; uint8_t a, b; uint8_t hdr[8]; };
// searches for the encrypted SWF in d[from, n); total = actual file size (to validate the length)
static bool findZinc(const uint8_t* d, size_t n, size_t from, size_t base, size_t total, ZincHit& z) {
    for (size_t j = std::max<size_t>(from, 4); j + 8 <= n; j++) {
        for (uint8_t first : {(uint8_t)'C', (uint8_t)'F'}) {
            uint8_t a = d[j] ^ first, b = (uint8_t)((d[j + 1] ^ 'W') - a);
            if ((d[j + 2] ^ (uint8_t)(a + 2 * b)) != 'S') continue;
            uint8_t ver = d[j + 3] ^ (uint8_t)(a + 3 * b); uint32_t len = rd32(d + j - 4);
            if (ver < 1 || ver > 50 || len < 64 || base + j + len > total) continue;
            uint8_t h[8]; for (int i = 0; i < 8; i++) h[i] = d[j + i] ^ (uint8_t)(a + b * i);
            uint32_t full = rd32(h + 4);
            if (first == 'F' ? full != len : (full < len || full > len * 20u)) continue;
            z.off = j; z.len = len; z.a = a; z.b = b; memcpy(z.hdr, h, 8);
            return true;
        }
    }
    return false;
}
// Leaves only the SWF (still compressed) in data; returns false if the executable doesn't contain it.
static bool swfFromExe(std::vector<uint8_t>& data) {
    size_t n = data.size();
    auto keep = [&](size_t off, size_t len) { memmove(data.data(), data.data() + off, len); data.resize(len); };
    size_t start = peImageEnd(data.data(), n); if (start > n) start = 0;
    auto sig = [](uint8_t a, uint8_t b, uint8_t c) { return (a == 'C' || a == 'F') && b == 'W' && c == 'S'; };
    ZincHit z;
    if (findZinc(data.data(), n, start, 0, n, z)) {
        uint8_t k = z.a; uint8_t* p = data.data() + z.off;
        for (size_t i = 0; i < z.len; i++, k += z.b) p[i] ^= k;
        keep(z.off, z.len);
        return true;
    }
    if (n >= 16 && rd32(data.data() + n - 8) == 0xFA123456u) {   // Adobe Flash projector
        uint32_t len = rd32(data.data() + n - 4);
        if (len >= 8 && len <= n - 8) { size_t o = n - 8 - len; if (sig(data[o], data[o + 1], data[o + 2])) { keep(o, len); return true; } }
    }
    for (size_t j = start; j + 8 <= n; j++) {   // unencrypted SWF
        if (!sig(data[j], data[j + 1], data[j + 2]) || data[j + 3] < 1 || data[j + 3] > 50) continue;
        uint32_t full = rd32(data.data() + j + 4);
        if (data[j] == 'F' ? j + full <= n && full >= 64 : full > (n - j) / 2 && full < (n - j) * 20) { keep(j, data[j] == 'F' ? full : n - j); return true; }
    }
    return false;
}
}  // namespace

#ifdef __PSP__
// (PSP: one game per binary, the others' tables are absent: weak references resolve to null)
#define FL_GAMECODE extern const GameCode __attribute__((weak))
#else
#define FL_GAMECODE extern const GameCode
#endif
FL_GAMECODE g_code_og; FL_GAMECODE g_code_wotl; FL_GAMECODE g_code_eternal;
// supported versions: uncompressed SWF size (header) and CRC-32 of the whole SWF
static const KnownGame kGames[] = {
    {&g_code_og,      "og",      39199603, 0xf6a0e3e6u, "The Binding of Isaac"},
    {&g_code_wotl,    "wotl",    36209977, 0xb79d3bbcu, "Wrath of the Lamb"},
    {&g_code_eternal, "eternal", 46493245, 0x090ef7b9u, "Eternal Edition"},
};
const KnownGame* knownGames(int* n) { *n = (int)(sizeof kGames / sizeof kGames[0]); return kGames; }
const KnownGame* knownGameBySwfLen(uint32_t len) { for (auto& g : kGames) if (g.swfLen == len) return &g; return nullptr; }
const GameCode* g_code = &g_code_wotl;   // (replaced when a game is loaded)
int g_stageW = 0, g_stageH = 0; float g_frameRate = 30;
const KnownGame* g_game = nullptr;
bool g_gameExact = false;

// size of the (uncompressed) SWF carried by a file, reading only the beginning: 0 if it isn't a game
uint32_t peekSwfLength(const std::string& path) {
    FILE* f = fopen(path.c_str(), "rb"); if (!f) return 0;
    fseek(f, 0, SEEK_END); long total = ftell(f); fseek(f, 0, SEEK_SET);
    std::vector<uint8_t> h(std::min<long>(total, 8192));
    uint32_t res = 0;
    if (fread(h.data(), 1, h.size(), f) == h.size() && h.size() >= 8) {
        if ((h[0] == 'C' || h[0] == 'F') && h[1] == 'W' && h[2] == 'S') res = rd32(h.data() + 4);
        else if (h[0] == 'M' && h[1] == 'Z') {
            size_t start = peImageEnd(h.data(), (size_t)total);
            if (start < (size_t)total) {   // the Zinc SWF sits near the start of the appended data
                std::vector<uint8_t> o(std::min<long>(total - (long)start, 1 << 20));
                fseek(f, (long)start, SEEK_SET);
                ZincHit z;
                if (fread(o.data(), 1, o.size(), f) == o.size() && findZinc(o.data(), o.size(), 0, start, (size_t)total, z)) res = rd32(z.hdr + 4);
            }
        }
    }
    fclose(f);
    return res;
}

// ---- lean loading (PSP: the decompressed SWF doesn't fit in memory)
// The first time, the SWF is extracted from the executable and decompressed to a cache file (streaming: neither the
// .exe nor the SWF is ever whole in memory). The movie data in memory is that SWF without the MP3 data of its sounds
// and timeline streams (~70 % of it): those tags keep a stub with the data's offset and length in the cache file,
// read when the sound plays.
std::string g_leanCacheDir;
static FILE* g_swfFile = nullptr;
static std::mutex g_swfFileMu;
static uint32_t crcUpdate(uint32_t crc, const uint8_t* p, size_t n) {
    static uint32_t tab[256]; static bool init = false;
    if (!init) { for (uint32_t i = 0; i < 256; i++) { uint32_t c = i; for (int k = 0; k < 8; k++) c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1))); tab[i] = c; } init = true; }
    for (size_t i = 0; i < n; i++) crc = tab[(crc ^ p[i]) & 255] ^ (crc >> 8);
    return crc;
}
#ifdef FL_LEAN
static bool extractSwf(const std::string& exe, const std::string& out, uint32_t& outLen, uint32_t& outCrc, std::string& err) {
    FILE* f = fopen(exe.c_str(), "rb"); if (!f) { err = "cannot open " + exe; return false; }
    fseek(f, 0, SEEK_END); long total = ftell(f); fseek(f, 0, SEEK_SET);
    std::vector<uint8_t> h(std::min<long>(total, 8192));
    if (fread(h.data(), 1, h.size(), f) != h.size() || h.size() < 8) { fclose(f); err = "read error"; return false; }
    size_t swfOff = 0; uint8_t ka = 0, kb = 0; bool xr = false;
    if ((h[0] == 'C' || h[0] == 'F') && h[1] == 'W' && h[2] == 'S') swfOff = 0;
    else if (h[0] == 'M' && h[1] == 'Z') {
        size_t start = peImageEnd(h.data(), (size_t)total);
        if (start >= (size_t)total) { fclose(f); err = exe + " does not contain the game."; return false; }
        std::vector<uint8_t> o(std::min<long>(total - (long)start, 1 << 20));
        fseek(f, (long)start, SEEK_SET);
        ZincHit z;
        if (fread(o.data(), 1, o.size(), f) != o.size() || !findZinc(o.data(), o.size(), 0, start, (size_t)total, z)) {
            fclose(f); err = exe + " does not contain the game.\nUse Isaac.exe from the Steam folder (not Binding_of_Isaac.exe)."; return false;
        }
        swfOff = start + z.off; ka = z.a; kb = z.b; xr = true;
    } else { fclose(f); err = "not a SWF"; return false; }
    FILE* w = fopen((out + ".tmp").c_str(), "wb"); if (!w) { fclose(f); err = "cannot write " + out; return false; }
    fseek(f, (long)swfOff, SEEK_SET);
    std::vector<uint8_t> in(256 << 10), dec(256 << 10);
    uint8_t k = ka; bool ok = true; uint32_t crc = 0xffffffffu, written = 0, full = 0; bool compressed = false;
    z_stream zs{}; bool zinit = false; bool headerDone = false; uint8_t hdr[8]; int hn = 0;
    for (;;) {
        size_t n = fread(in.data(), 1, in.size(), f);
        if (n == 0) break;
        if (xr) for (size_t i = 0; i < n; i++, k += kb) in[i] ^= k;
        size_t p = 0;
        if (!headerDone) {
            while (hn < 8 && p < n) hdr[hn++] = in[p++];
            if (hn < 8) continue;
            compressed = hdr[0] == 'C'; full = rd32(hdr + 4); hdr[0] = 'F';
            fwrite(hdr, 1, 8, w); crc = crcUpdate(crc, hdr, 8); written = 8; headerDone = true;
            if (compressed) { if (inflateInit(&zs) != Z_OK) { ok = false; break; } zinit = true; }
        }
        if (!compressed) {
            size_t m = std::min<size_t>(n - p, full > written ? full - written : 0);
            fwrite(in.data() + p, 1, m, w); crc = crcUpdate(crc, in.data() + p, m); written += (uint32_t)m;
            if (written >= full) break;
            continue;
        }
        zs.next_in = in.data() + p; zs.avail_in = (uInt)(n - p);
        int rc = Z_OK;
        while (zs.avail_in > 0 && rc != Z_STREAM_END) {
            zs.next_out = dec.data(); zs.avail_out = (uInt)dec.size();
            rc = inflate(&zs, Z_NO_FLUSH);
            if (rc != Z_OK && rc != Z_STREAM_END && rc != Z_BUF_ERROR) { ok = false; break; }
            size_t m = dec.size() - zs.avail_out;
            fwrite(dec.data(), 1, m, w); crc = crcUpdate(crc, dec.data(), m); written += (uint32_t)m;
        }
        if (!ok || rc == Z_STREAM_END) break;
    }
    if (zinit) inflateEnd(&zs);
    fclose(f);
    if (fclose(w) != 0) ok = false;
    if (!ok || written != full) { remove((out + ".tmp").c_str()); err = "could not decompress the game (" + std::to_string(written) + " of " + std::to_string(full) + " bytes)"; return false; }
    remove(out.c_str()); rename((out + ".tmp").c_str(), out.c_str());
    outLen = written; outCrc = ~crc;
    if (FILE* idf = fopen((out + ".id").c_str(), "w")) { fprintf(idf, "%u %u\n", (unsigned)outLen, (unsigned)outCrc); fclose(idf); }
    return true;
}
// copies the tags of [pos, end) of the cache file into out, without the sound data (stubs instead)
static bool leanTags(FILE* f, size_t pos, size_t end, std::vector<uint8_t>& out) {
    auto put32 = [&](uint32_t v) { for (int i = 0; i < 4; i++) out.push_back((uint8_t)(v >> (8 * i))); };
    auto hdrLong = [&](int tag, uint32_t len) { uint16_t hh = (uint16_t)(tag << 6 | 63); out.push_back(hh & 255); out.push_back(hh >> 8); put32(len); };
    std::vector<uint8_t> body;
    while (pos + 2 <= end) {
        uint8_t hb[6]; fseek(f, (long)pos, SEEK_SET); if (fread(hb, 1, 2, f) != 2) return false;
        uint16_t hh = hb[0] | hb[1] << 8; int tag = hh >> 6; uint32_t len = hh & 63; pos += 2;
        if (len == 63) { if (fread(hb + 2, 1, 4, f) != 4) return false; len = rd32(hb + 2); pos += 4; }
        size_t b = pos; pos += len;
        if (tag == 14 || tag == 19) {
            uint8_t first[9]; size_t keep = tag == 19 ? 4 : 7;
            if (fread(first, 1, std::min<size_t>(9, len), f) < std::min<size_t>(9, len)) return false;
            if (tag == 14 && len >= 9 && (first[2] >> 4) == 2) keep = 9;   // MP3: + SeekSamples
            keep = std::min<size_t>(keep, len);
            hdrLong(tag, (uint32_t)(keep + 8)); out.insert(out.end(), first, first + keep);
            put32((uint32_t)(b + keep)); put32((uint32_t)(len - keep));
        } else if (tag == 39) {
            uint8_t sh[4]; if (len < 4 || fread(sh, 1, 4, f) != 4) return false;
            std::vector<uint8_t> sub; sub.insert(sub.end(), sh, sh + 4);
            std::swap(out, sub);
            bool ok = leanTags(f, b + 4, b + len, out);
            std::swap(out, sub);
            if (!ok) return false;
            hdrLong(39, (uint32_t)sub.size()); out.insert(out.end(), sub.begin(), sub.end());
        } else if ((tag == 2 || tag == 22 || tag == 32 || tag == 83) && len > 3) {   // shape: id and bounds; the records stay in the file
            uint8_t first[19]; size_t n = std::min<size_t>(sizeof first, len);
            if (fread(first, 1, n, f) != n) return false;
            size_t keep = std::min<size_t>(n, 2 + (5 + 4 * (first[2] >> 3) + 7) / 8);
            hdrLong(tag, (uint32_t)(keep + 8)); out.insert(out.end(), first, first + keep);
            put32((uint32_t)(b + 2)); put32(len - 2);
        } else if (tag == 12 && len > 1) {   // DoAction: recompiled, the bytecode isn't needed (1 byte: "not empty")
            out.push_back((uint8_t)(12 << 6 | 1)); out.push_back((uint8_t)((12 << 6 | 1) >> 8)); out.push_back(0);
        } else {
            body.resize(len); if (len && fread(body.data(), 1, len, f) != len) return false;
            if (len < 63 && tag != 2 && tag != 22 && tag != 32 && tag != 83) { uint16_t h2 = (uint16_t)(tag << 6 | len); out.push_back(h2 & 255); out.push_back(h2 >> 8); }
            else hdrLong(tag, len);
            out.insert(out.end(), body.begin(), body.end());
        }
        if (tag == 0) break;
    }
    return true;
}
#endif
static bool leanRead(uint32_t off, uint32_t len, std::vector<uint8_t>& out) {
    std::lock_guard<std::mutex> lk(g_swfFileMu);
    out.resize(len);
    if (!g_swfFile || fseek(g_swfFile, (long)off, SEEK_SET) != 0 || fread(out.data(), 1, len, g_swfFile) != len) { logf("game data: read error\n"); return false; }
    return true;
}
static std::map<const void*, std::weak_ptr<const std::vector<uint8_t>>> g_soundCacheW;
static size_t g_soundRecentBytes;
void leanStats(size_t* shapeBytes, size_t* soundBytes) { *shapeBytes = g_shapeParsedBytes; *soundBytes = g_soundRecentBytes; }
static std::vector<std::pair<const void*, std::shared_ptr<const std::vector<uint8_t>>>> g_soundRecent;   // keeps the last ones loaded
size_t g_soundCacheBytes = 8u << 20;
static std::shared_ptr<const std::vector<uint8_t>> readBlocks(const void* key, const std::vector<std::array<uint32_t, 2>>& blocks) {
    std::lock_guard<std::mutex> lk(g_swfFileMu);
    auto it = g_soundCacheW.find(key);
    if (it != g_soundCacheW.end()) if (auto sp = it->second.lock()) return sp;
    auto v = std::make_shared<std::vector<uint8_t>>();
    size_t total = 0; for (auto& b : blocks) total += b[1];
    v->resize(total);
    size_t o = 0;
    for (auto& b : blocks) {
        if (!g_swfFile || fseek(g_swfFile, (long)b[0], SEEK_SET) != 0 || fread(v->data() + o, 1, b[1], g_swfFile) != b[1]) { logf("sound data: read error\n"); v->clear(); break; }
        o += b[1];
    }
    std::shared_ptr<const std::vector<uint8_t>> sp = v;
    g_soundCacheW[key] = sp;
    g_soundRecent.push_back({key, sp}); g_soundRecentBytes += total;
    while (g_soundRecentBytes > g_soundCacheBytes && g_soundRecent.size() > 1) { g_soundRecentBytes -= g_soundRecent.front().second->size(); g_soundRecent.erase(g_soundRecent.begin()); }
    return sp;
}
std::shared_ptr<const std::vector<uint8_t>> soundBytes(SoundDef* s) {
    if (!g_leanSwf) return nullptr;
    return readBlocks(s, {{s->fileOff, (uint32_t)s->size}});
}
std::shared_ptr<const std::vector<uint8_t>> streamBytes(StreamInfo* st) {
    if (!g_leanSwf) return nullptr;
    return readBlocks(st, st->fileBlocks);
}
#ifdef FL_LEAN
static bool loadLean(Movie& m, const std::string& path, std::string& err) {
    // cache file named after the executable's size and modification-independent name
    FILE* f = fopen(path.c_str(), "rb"); if (!f) { err = "cannot open " + path; return false; }
    fseek(f, 0, SEEK_END); long exeLen = ftell(f); fclose(f);
    std::string cache = g_leanCacheDir + "swf_" + std::to_string(exeLen) + ".cache";
    uint32_t len = 0, crc = 0;
    {
        FILE* idf = fopen((cache + ".id").c_str(), "r"); unsigned a = 0, b = 0;
        bool have = idf && fscanf(idf, "%u %u", &a, &b) == 2; if (idf) fclose(idf);
        FILE* cf = have ? fopen(cache.c_str(), "rb") : nullptr; long cl = -1;
        if (cf) { fseek(cf, 0, SEEK_END); cl = ftell(cf); fclose(cf); }
        if (have && cl == (long)a) { len = a; crc = b; }
        else {
            logf("decompressing the game to %s...\n", cache.c_str());
            if (!extractSwf(path, cache, len, crc, err)) return false;
        }
    }
    g_game = nullptr; g_gameExact = false;
    for (auto& g : kGames) if (g.crc == crc) { g_game = &g; g_gameExact = true; }
    if (!g_game) g_game = knownGameBySwfLen(len);
    if (!g_game) { err = path + ": is not one of the versions of The Binding of Isaac supported by the recomp\n(original, Wrath of the Lamb or Eternal Edition from Steam)."; return false; }
    if (g_swfFile) fclose(g_swfFile);
    g_swfFile = fopen(cache.c_str(), "rb"); if (!g_swfFile) { err = "cannot open " + cache; return false; }
    static char vbuf[16 << 10]; setvbuf(g_swfFile, vbuf, _IOFBF, sizeof vbuf);   // (shapes: small reads anywhere)
    uint8_t h[32]; if (fread(h, 1, sizeof h, g_swfFile) != sizeof h) { err = "read error"; return false; }
    int nb = h[8] >> 3; size_t hdrLen = 8 + (5 + 4 * nb + 7) / 8 + 4;
    m.data.assign(h, h + hdrLen);
    std::lock_guard<std::mutex> lk(g_swfFileMu);
    if (!leanTags(g_swfFile, hdrLen, len, m.data)) { err = "read error in " + cache; return false; }
    m.data.shrink_to_fit();
    logf("game read from %s (SWF of %u bytes, %zu in memory)\n", path.c_str(), (unsigned)len, m.data.size());
    return true;
}

#endif
bool Movie::load(const std::string& path, std::string& err) {
#ifdef FL_LEAN
    if (g_leanSwf) {
        if (!loadLean(*this, path, err)) return false;
        if (!g_game->code) { err = std::string("this program does not contain ") + g_game->title + "."; return false; }
        g_code = g_game->code;
        g_stageW = g_code->stageW; g_stageH = g_code->stageH; g_frameRate = g_code->frameRate;
        g_code->initAtoms();
        logf("game: %s%s\n", g_game->title, g_gameExact ? "" : " (the SWF does not exactly match the Steam version)");
        version = data[3];
        Rd r(data.data(), 8, data.size());
        RectF rc = r.rect(); width = (int)rc.x1; height = (int)rc.y1;
        frameRate = r.u16() / 256.f; rootDef.nframes = r.u16(); rootDef.id = 0; rootDef.type = CH_SPRITE;
        chars.assign(65536, nullptr);
        for (int i = 0; i < g_code->nFrames; i++) {
            auto& s = g_code->frames[i];
            g_scriptMap[(uint64_t)s.sprite << 32 | (uint64_t)s.frame << 16 | s.index] = s.fn;
        }
        for (int i = 0; i < g_code->nButtons; i++) g_buttonMap[(uint32_t)g_code->buttons[i].button << 16 | g_code->buttons[i].index] = g_code->buttons[i].fn;
        parseTimeline(data.data(), r.p, data.size(), &rootDef, true);
        return true;
    }
#endif
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) { err = "cannot open " + path; return false; }
    fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
    data.resize(n);
    if (fread(data.data(), 1, n, f) != (size_t)n) { fclose(f); err = "read error"; return false; }
    fclose(f);
    if (n > 2 && data[0] == 'M' && data[1] == 'Z') {
        if (!swfFromExe(data)) { err = path + " does not contain the game.\nUse Isaac.exe from the Steam folder (not Binding_of_Isaac.exe)."; return false; }
        n = (long)data.size();
        logf("game read from %s (SWF of %ld bytes)\n", path.c_str(), n);
    }
    if (n < 8 || (data[0] != 'F' && data[0] != 'C') || data[1] != 'W' || data[2] != 'S') { err = "not a SWF"; return false; }
    if (data[0] == 'C') {
        int outlen = 0; uint32_t full = data[4] | data[5] << 8 | data[6] << 16 | (uint32_t)data[7] << 24;
        std::vector<uint8_t> out(full);
        memcpy(out.data(), data.data(), 8);
        outlen = stbi_zlib_decode_buffer((char*)out.data() + 8, full - 8, (const char*)data.data() + 8, (int)n - 8);
        if (outlen < 0) { err = "zlib"; return false; }
        out[0] = 'F'; data.swap(out);
    }
    {   // which game it is: the recompiled code must match this exact version
        static uint32_t tab[256];
        for (uint32_t i = 0; i < 256; i++) { uint32_t c = i; for (int k = 0; k < 8; k++) c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1))); tab[i] = c; }
        uint32_t crc = 0xffffffffu;
        for (uint8_t b : data) crc = tab[(crc ^ b) & 255] ^ (crc >> 8);
        crc = ~crc;
        g_game = nullptr; g_gameExact = false;
        for (auto& g : kGames) if (g.crc == crc) { g_game = &g; g_gameExact = true; }
        if (!g_game) g_game = knownGameBySwfLen((uint32_t)data.size());
        if (!g_game) { err = path + ": is not one of the versions of The Binding of Isaac supported by the recomp\n(original, Wrath of the Lamb or Eternal Edition from Steam)."; data.clear(); return false; }
        if (!g_game->code) { err = std::string("this program does not contain ") + g_game->title + "."; return false; }
        g_code = g_game->code;
        g_stageW = g_code->stageW; g_stageH = g_code->stageH; g_frameRate = g_code->frameRate;
        g_code->initAtoms();
        logf("game: %s%s\n", g_game->title, g_gameExact ? "" : " (the SWF does not exactly match the Steam version)");
    }
    version = data[3];
    Rd r(data.data(), 8, data.size());
    RectF rc = r.rect(); width = (int)rc.x1; height = (int)rc.y1;
    frameRate = r.u16() / 256.f; rootDef.nframes = r.u16(); rootDef.id = 0; rootDef.type = CH_SPRITE;
    chars.assign(65536, nullptr);
    for (int i = 0; i < g_code->nFrames; i++) {
        auto& s = g_code->frames[i];
        g_scriptMap[(uint64_t)s.sprite << 32 | (uint64_t)s.frame << 16 | s.index] = s.fn;
    }
    for (int i = 0; i < g_code->nButtons; i++) g_buttonMap[(uint32_t)g_code->buttons[i].button << 16 | g_code->buttons[i].index] = g_code->buttons[i].fn;
    parseTimeline(data.data(), r.p, data.size(), &rootDef, true);
    return true;
}

// flattened polygons for hitTest(x, y, true); previously computed at load time (~95 MB for all shapes)
const std::vector<std::vector<float>>& shapeHitPolys(ShapeDef* s) {
    shapeEnsure(s);
    if (!s->hitBuilt) {
        s->hitPolys.clear();
        for (auto& L : s->layers) for (auto& fp : L.fillPaths) { std::vector<float> poly; flatten(fp, poly); if (!poly.empty()) s->hitPolys.push_back(std::move(poly)); }
        s->hitBuilt = true;
    }
    return s->hitPolys;
}

}  // namespace fl
