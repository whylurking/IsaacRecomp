// Vector rasterization (PlutoVG): shapes, morphs, text and BitmapData.draw().
#include "raster.h"
#include <chrono>
#include "snapshot.h"
#include "platform.h"
#include "display.h"
#include <plutovg.h>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <SDL.h>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>

extern "C" __thread int plutovg_texture_smooth;
namespace fl {

// ============================================================ color utilities
static inline float clamp01(float v) { return v < 0 ? 0 : v > 1 ? 1 : v; }
static plutovg_color_t colorOf(uint32_t argb, const CX* cx) {
    float a = (argb >> 24) / 255.f, r = ((argb >> 16) & 255) / 255.f, g = ((argb >> 8) & 255) / 255.f, b = (argb & 255) / 255.f;
    if (cx) {
        r = clamp01(r * cx->rm + cx->ra / 255.f); g = clamp01(g * cx->gm + cx->ga / 255.f);
        b = clamp01(b * cx->bm + cx->ba / 255.f); a = clamp01(a * cx->am + cx->aa / 255.f);
    }
    if (kPxABGR) std::swap(r, b);   // (plutovg packs the channels as given: the PSP's order)
    return {r, g, b, a};
}
static plutovg_matrix_t pm(const Mat& m) { return {(float)m.a, (float)m.b, (float)m.c, (float)m.d, (float)m.tx, (float)m.ty}; }

// ============================================================ SWF bitmap surfaces (cached per color transform)
struct BmKey { uint16_t id; CX cx; bool operator<(const BmKey& o) const { if (id != o.id) return id < o.id; return memcmp(&cx, &o.cx, sizeof(CX)) < 0; } };
struct BmSurf { plutovg_surface_t* s = nullptr; uint32_t lastUse = 0; };
static std::map<BmKey, BmSurf> g_bmSurf;
static uint32_t g_bmClock = 0;   // (rasterTrim)
static std::vector<uint32_t> g_bmTmpStore[64];

static std::mutex g_bmMu, g_morphMu;   // caches shared with the raster threads
static plutovg_surface_t* bitmapSurface(uint16_t id, const CX* cx) {
    std::lock_guard<std::mutex> lk(g_bmMu);
    BmKey k{id, cx ? *cx : CX()};
    auto it = g_bmSurf.find(k);
    if (it != g_bmSurf.end()) { it->second.lastUse = g_bmClock; return it->second.s; }
    CharDef* cd = g_movie.get(id);
    if (!cd || cd->type != CH_BITMAP) return nullptr;
    BitmapDef* b = (BitmapDef*)cd; b->decode();
    if (!b->w || !b->h) return nullptr;
    plutovg_surface_t* s;
    if (!cx || cx->identity()) s = plutovg_surface_create_for_data((unsigned char*)b->px.data(), b->w, b->h, b->w * 4);
    else {
        s = plutovg_surface_create(b->w, b->h);
        uint32_t* dst = (uint32_t*)plutovg_surface_get_data(s);
        for (int i = 0; i < b->w * b->h; i++) {
            uint32_t p = pxToArgb(b->px[i]); float a = (p >> 24) / 255.f;
            float r = a > 0 ? ((p >> 16) & 255) / 255.f / a : 0, g = a > 0 ? ((p >> 8) & 255) / 255.f / a : 0, bb = a > 0 ? (p & 255) / 255.f / a : 0;
            plutovg_color_t c = colorOf(0xff000000u | (uint32_t)(r * 255) << 16 | (uint32_t)(g * 255) << 8 | (uint32_t)(bb * 255), cx);
            float na = clamp01(a * cx->am + cx->aa / 255.f);
            dst[i] = (uint32_t)(na * 255 + .5f) << 24 | (uint32_t)(c.r * na * 255 + .5f) << 16 | (uint32_t)(c.g * na * 255 + .5f) << 8 | (uint32_t)(c.b * na * 255 + .5f);
        }
    }
    g_bmSurf[k] = {s, g_bmClock};
    return s;
}
// (PSP: memory) drops the SWF bitmaps (decoded, and their colour-transformed copies) not used since the last call,
// least recently used first, while they take more than the budget. Called once per tick, when nothing is drawing.
void rasterTrim(size_t budget) {
    std::lock_guard<std::mutex> lk(g_bmMu);
    uint32_t now = g_bmClock++;
    size_t bytes = 0;
    for (auto& kv : g_bmSurf) bytes += (size_t)plutovg_surface_get_width(kv.second.s) * plutovg_surface_get_height(kv.second.s) * 4;
    if (bytes <= budget) return;
    std::vector<std::pair<uint32_t, BmKey>> order;
    for (auto& kv : g_bmSurf) if (kv.second.lastUse != now) order.push_back({kv.second.lastUse, kv.first});
    std::sort(order.begin(), order.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    for (auto& o : order) {
        if (bytes <= budget * 3 / 4) break;
        auto it = g_bmSurf.find(o.second);
        plutovg_surface_t* sf = it->second.s;
        bytes -= (size_t)plutovg_surface_get_width(sf) * plutovg_surface_get_height(sf) * 4;
        plutovg_surface_destroy(sf);
        uint16_t id = it->first.id; g_bmSurf.erase(it);
        bool more = false; for (auto& kv : g_bmSurf) if (kv.first.id == id) { more = true; break; }
        if (!more) if (CharDef* cd = g_movie.get(id)) if (cd->type == CH_BITMAP) {   // (the decoded pixels too: decoded again when needed)
            BitmapDef* b = (BitmapDef*)cd; std::vector<uint32_t>().swap(b->px); b->decoded = false;
        }
    }
}

// ============================================================ style painting
static void setFillPaint(plutovg_canvas_t* cv, const FillStyle& f, const CX* cx) {
    switch (f.type) {
        case 0x00: { plutovg_color_t c = colorOf(f.argb, cx); plutovg_canvas_set_color(cv, &c); break; }
        case 0x10: case 0x12: case 0x13: {
            std::vector<plutovg_gradient_stop_t> st;
            for (auto& s : f.stops) st.push_back({s.pos, colorOf(s.argb, cx)});
            if (st.empty()) st.push_back({0, {0, 0, 0, 1}});
            plutovg_spread_method_t spread = f.spread == 1 ? PLUTOVG_SPREAD_METHOD_REFLECT : f.spread == 2 ? PLUTOVG_SPREAD_METHOD_REPEAT : PLUTOVG_SPREAD_METHOD_PAD;
            plutovg_matrix_t m = pm(f.m);
            const float R = 16384.f / 20.f;
            if (f.type == 0x10) plutovg_canvas_set_linear_gradient(cv, -R, 0, R, 0, spread, st.data(), (int)st.size(), &m);
            else plutovg_canvas_set_radial_gradient(cv, 0, 0, R, f.type == 0x13 ? f.focal * R : 0, 0, 0, spread, st.data(), (int)st.size(), &m);
            break;
        }
        default: {
            plutovg_surface_t* s = bitmapSurface(f.bitmapId, cx);
            if (!s) { plutovg_color_t c{0, 0, 0, 0}; plutovg_canvas_set_color(cv, &c); break; }
            Mat bm = f.m; bm.a /= 20; bm.b /= 20; bm.c /= 20; bm.d /= 20;
            plutovg_matrix_t m = pm(bm);
            plutovg_texture_smooth = (f.type == 0x40 || f.type == 0x41) ? 1 : 0;
            plutovg_canvas_set_texture(cv, s, (f.type == 0x40 || f.type == 0x42) ? PLUTOVG_TEXTURE_TYPE_TILED : PLUTOVG_TEXTURE_TYPE_PLAIN, 1.0f, &m);
            break;
        }
    }
}
static void addPath(plutovg_canvas_t* cv, const Path& p) {
    size_t k = 0;
    for (uint8_t v : p.verbs) {
        if (v == 0) { plutovg_canvas_move_to(cv, p.pts[k], p.pts[k + 1]); k += 2; }
        else if (v == 1) { plutovg_canvas_line_to(cv, p.pts[k], p.pts[k + 1]); k += 2; }
        else { plutovg_canvas_quad_to(cv, p.pts[k], p.pts[k + 1], p.pts[k + 2], p.pts[k + 3]); k += 4; }
    }
}
static double matScale(const Mat& m) { return std::sqrt(std::fabs(m.a * m.d - m.b * m.c)); }

// Flash rasterizes each shape in one pass: fills that share an edge don't let what's behind show through. Painting
// each fill separately with antialiasing leaves a semi-transparent pixel on the shared edge (the thin white
// "outline" over the background). When the shape is painted onto an empty surface (textures and leaves), it is
// painted front to back with the SATURATE operator, which distributes the pixel's remaining coverage and leaves it
// opaque; on all other pixels the result is the same as painting back to front.
thread_local bool g_frontToBack = false;

static void fillOne(plutovg_canvas_t* cv, const ShapeLayer& L, size_t i, const plutovg_matrix_t& pmm, const CX* cx) {
    if (L.fillPaths[i].empty()) return;
    plutovg_canvas_set_matrix(cv, &pmm);
    setFillPaint(cv, L.fills[i], cx);
    plutovg_canvas_set_fill_rule(cv, PLUTOVG_FILL_RULE_EVEN_ODD);
    plutovg_canvas_new_path(cv);
    addPath(cv, L.fillPaths[i]);
    plutovg_canvas_fill(cv);
}
static void lineOne(plutovg_canvas_t* cv, const ShapeLayer& L, size_t i, const plutovg_matrix_t& pmm, double sc, const CX* cx) {
    if (L.linePaths[i].empty()) return;
    const LineStyle& ls = L.lines[i];
    plutovg_canvas_set_matrix(cv, &pmm);
    if (ls.hasFill) setFillPaint(cv, ls.fill, cx);
    else { plutovg_color_t c = colorOf(ls.argb, cx); plutovg_canvas_set_color(cv, &c); }
    float w = ls.width; if (w * sc < 1.0) w = (float)(1.0 / sc);
    plutovg_canvas_set_line_width(cv, w);
    plutovg_canvas_set_line_cap(cv, ls.startCap == 1 ? PLUTOVG_LINE_CAP_BUTT : ls.startCap == 2 ? PLUTOVG_LINE_CAP_SQUARE : PLUTOVG_LINE_CAP_ROUND);
    plutovg_canvas_set_line_join(cv, ls.join == 1 ? PLUTOVG_LINE_JOIN_BEVEL : ls.join == 2 ? PLUTOVG_LINE_JOIN_MITER : PLUTOVG_LINE_JOIN_ROUND);
    plutovg_canvas_set_miter_limit(cv, ls.miter);
    plutovg_canvas_new_path(cv);
    addPath(cv, L.linePaths[i]);
    plutovg_canvas_stroke(cv);
}

void paintShape(plutovg_canvas_t* cv, const ShapeDef* s, const Mat& m, const CX* cx) {
    shapeEnsure(const_cast<ShapeDef*>(s));
    plutovg_matrix_t pmm = pm(m);
    double sc = std::max(1e-6, matScale(m));
    if (g_frontToBack) {
        plutovg_operator_t op0 = plutovg_canvas_get_operator(cv);
        plutovg_canvas_set_operator(cv, PLUTOVG_OPERATOR_SATURATE);
        for (size_t li = s->layers.size(); li-- > 0;) {
            const ShapeLayer& L = s->layers[li];
            for (size_t i = std::min(L.linePaths.size(), L.lines.size()); i-- > 0;) lineOne(cv, L, i, pmm, sc, cx);
            for (size_t i = std::min(L.fillPaths.size(), L.fills.size()); i-- > 0;) fillOne(cv, L, i, pmm, cx);
        }
        plutovg_canvas_set_operator(cv, op0);
        return;
    }
    for (auto& L : s->layers) {
        for (size_t i = 0; i < L.fillPaths.size() && i < L.fills.size(); i++) fillOne(cv, L, i, pmm, cx);
        for (size_t i = 0; i < L.linePaths.size() && i < L.lines.size(); i++) lineOne(cv, L, i, pmm, sc, cx);
    }
}

// ============================================================ morph → shape
struct MorphCacheKey { const MorphDef* m; uint16_t r; bool operator<(const MorphCacheKey& o) const { return m != o.m ? m < o.m : r < o.r; } };
static std::map<MorphCacheKey, ShapeDef*> g_morphCache;

static uint32_t lerpColor(uint32_t a, uint32_t b, float t) {
    auto ch = [&](int s) { float x = ((a >> s) & 255) * (1 - t) + ((b >> s) & 255) * t; return (uint32_t)(x + 0.5f) << s; };
    return ch(24) | ch(16) | ch(8) | ch(0);
}
ShapeDef* morphShape(const MorphDef* md, uint16_t ratio) {
    uint16_t rq = ratio & 0xffc0;   // quantized (1024 steps)
    std::lock_guard<std::mutex> lk(g_morphMu);
    auto it = g_morphCache.find({md, rq});
    if (it != g_morphCache.end()) return it->second;
    float t = rq / 65535.f;
    ShapeDef* s = new ShapeDef(); s->type = CH_SHAPE; s->id = md->id;
    s->bounds.x0 = md->b0.x0 + (md->b1.x0 - md->b0.x0) * t; s->bounds.x1 = md->b0.x1 + (md->b1.x1 - md->b0.x1) * t;
    s->bounds.y0 = md->b0.y0 + (md->b1.y0 - md->b0.y0) * t; s->bounds.y1 = md->b0.y1 + (md->b1.y1 - md->b0.y1) * t;
    s->layers.emplace_back(); ShapeLayer& L = s->layers.back();
    for (size_t i = 0; i < md->fills0.size(); i++) {
        FillStyle f = md->fills0[i]; const FillStyle& f1 = md->fills1[i];
        f.argb = lerpColor(f.argb, f1.argb, t);
        f.m.a += (f1.m.a - f.m.a) * t; f.m.b += (f1.m.b - f.m.b) * t; f.m.c += (f1.m.c - f.m.c) * t; f.m.d += (f1.m.d - f.m.d) * t;
        f.m.tx += (f1.m.tx - f.m.tx) * t; f.m.ty += (f1.m.ty - f.m.ty) * t;
        for (size_t k = 0; k < f.stops.size() && k < f1.stops.size(); k++) { f.stops[k].pos += (f1.stops[k].pos - f.stops[k].pos) * t; f.stops[k].argb = lerpColor(f.stops[k].argb, f1.stops[k].argb, t); }
        L.fills.push_back(f);
    }
    for (size_t i = 0; i < md->lines0.size(); i++) {
        LineStyle l = md->lines0[i]; const LineStyle& l1 = md->lines1[i];
        l.width += (l1.width - l.width) * t; l.argb = lerpColor(l.argb, l1.argb, t); L.lines.push_back(l);
    }
    // interpolated edges per fill style (linked the same way as in shapes)
    struct E { int32_t x0, y0, cx, cy, x1, y1; };
    std::vector<std::vector<E>> per(L.fills.size() + 1);
    L.linePaths.assign(L.lines.size(), {});
    for (auto& e : md->edges) {
        auto lp = [&](const float* v) { return (int32_t)std::lround((v[0] + (v[1] - v[0]) * t) * 20); };
        E x{lp(e.x0), lp(e.y0), lp(e.cx), lp(e.cy), lp(e.x1), lp(e.y1)};
        if (e.fill1 > 0 && e.fill1 < (int)per.size()) per[e.fill1].push_back(x);
        if (e.fill0 > 0 && e.fill0 < (int)per.size()) per[e.fill0].push_back({x.x1, x.y1, x.cx, x.cy, x.x0, x.y0});
        if (e.line > 0 && e.line <= (int)L.lines.size()) {
            Path& p = L.linePaths[e.line - 1];
            p.verbs.push_back(0); p.pts.insert(p.pts.end(), {x.x0 / 20.f, x.y0 / 20.f});
            p.verbs.push_back(2); p.pts.insert(p.pts.end(), {x.cx / 20.f, x.cy / 20.f, x.x1 / 20.f, x.y1 / 20.f});
        }
    }
    L.fillPaths.assign(L.fills.size(), {});
    for (size_t fi = 1; fi < per.size(); fi++) {
        auto& edges = per[fi]; Path& path = L.fillPaths[fi - 1];
        std::vector<uint8_t> used(edges.size(), 0);
        for (size_t i = 0; i < edges.size(); i++) {
            if (used[i]) continue;
            int32_t sx = edges[i].x0, sy = edges[i].y0; size_t cur = i;
            path.verbs.push_back(0); path.pts.insert(path.pts.end(), {sx / 20.f, sy / 20.f});
            while (true) {
                used[cur] = 1; const E& e = edges[cur];
                path.verbs.push_back(2); path.pts.insert(path.pts.end(), {e.cx / 20.f, e.cy / 20.f, e.x1 / 20.f, e.y1 / 20.f});
                if (e.x1 == sx && e.y1 == sy) break;
                size_t nx = SIZE_MAX;
                for (size_t j = 0; j < edges.size(); j++) if (!used[j] && edges[j].x0 == e.x1 && edges[j].y0 == e.y1) { nx = j; break; }
                if (nx == SIZE_MAX) break;
                cur = nx;
            }
        }
    }
    g_morphCache[{md, rq}] = s;
    return s;
}

// ============================================================ text
static void paintGlyphs(plutovg_canvas_t* cv, const FontDef* f, const std::vector<std::pair<int, std::pair<float, float>>>& glyphs, float size, const Mat& m, plutovg_color_t col) {
    plutovg_canvas_set_color(cv, &col);
    plutovg_canvas_set_fill_rule(cv, PLUTOVG_FILL_RULE_EVEN_ODD);
    for (auto& g : glyphs) {
        if (g.first < 0 || g.first >= (int)f->glyphs.size()) continue;
        Mat gm; gm.a = size; gm.d = size; gm.tx = g.second.first; gm.ty = g.second.second;
        plutovg_matrix_t mm = pm(m * gm);
        plutovg_canvas_set_matrix(cv, &mm);
        plutovg_canvas_new_path(cv);
        addPath(cv, f->glyphs[g.first]);
        plutovg_canvas_fill(cv);
    }
}

void paintText(plutovg_canvas_t* cv, const TextDef* t, const Mat& m, const CX* cx) {
    Mat tm = m * t->m;
    for (auto& g : t->glyphs) {
        CharDef* cd = g_movie.get(g.font);
        if (!cd || cd->type != CH_FONT) continue;
        std::vector<std::pair<int, std::pair<float, float>>> one{{g.idx, {g.x, g.y}}};
        paintGlyphs(cv, (FontDef*)cd, one, g.size, tm, colorOf(g.argb, cx));
    }
}

static float glyphAdvance(const FontDef* f, int idx, float size) { return (idx >= 0 && idx < (int)f->advances.size()) ? f->advances[idx] * size : size * 0.5f; }

void paintEditText(plutovg_canvas_t* cv, const EditTextDef* d, const std::string& txt, uint32_t argb, const Mat& m, const CX* cx) {
    CharDef* cd = g_movie.get(d->fontId);
    if (!cd || cd->type != CH_FONT) return;
    const FontDef* f = (FontDef*)cd;
    float size = d->height;
    float width = (float)(d->bounds.x1 - d->bounds.x0) - 4 - d->leftMargin - d->rightMargin;
    float ascent = f->hasLayout ? f->ascent * size : size * 0.8f;
    float descent = f->hasLayout ? f->descent * size : size * 0.2f;
    float lineH = ascent + descent + d->leading;
    // split into lines (with word wrap)
    std::vector<std::string> lines; std::string cur;
    auto measure = [&](const std::string& s) { float w = 0; for (unsigned char c : s) { auto it = f->codeToIdx.find(c); w += glyphAdvance(f, it == f->codeToIdx.end() ? -1 : it->second, size); } return w; };
    for (size_t i = 0; i <= txt.size(); i++) {
        char c = i < txt.size() ? txt[i] : '\n';
        if (c == '\r' || c == '\n') { lines.push_back(cur); cur.clear(); if (c == '\r' && i + 1 < txt.size() && txt[i + 1] == '\n') i++; continue; }
        cur += c;
        if (d->wordWrap && measure(cur) > width) {
            size_t sp = cur.find_last_of(' ');
            if (sp != std::string::npos && sp > 0) { lines.push_back(cur.substr(0, sp)); cur = cur.substr(sp + 1); }
        }
    }
    if (!d->multiline && lines.size() > 1) { std::string all; for (auto& l : lines) all += l; lines.assign(1, all); }
    plutovg_color_t col = colorOf(argb, cx);
    float y = (float)d->bounds.y0 + 2 + ascent;
    for (auto& line : lines) {
        float lw = measure(line);
        float x = (float)d->bounds.x0 + 2 + d->leftMargin + d->indent;
        if (d->align == 1) x = (float)d->bounds.x1 - 2 - d->rightMargin - lw;
        else if (d->align == 2) x = ((float)d->bounds.x0 + (float)d->bounds.x1) / 2 - lw / 2 + (d->leftMargin - d->rightMargin) / 2;
        std::vector<std::pair<int, std::pair<float, float>>> gl;
        for (unsigned char c : line) {
            auto it = f->codeToIdx.find(c); int idx = it == f->codeToIdx.end() ? -1 : it->second;
            gl.push_back({idx, {x, y}}); x += glyphAdvance(f, idx, size);
        }
        paintGlyphs(cv, f, gl, size, m, col);
        y += lineH;
    }
}

// ============================================================ local bounds of the rasterizable "content"
RasterSrc rasterSrcOf(DisplayObject* d) {
    RasterSrc s; s.kind = d->kind; s.ratio = d->ratio;
    if (d->kind == OK_EDITTEXT) { EditTextObj* e = (EditTextObj*)d; s.def = e->def; s.text = &e->text; s.argb = e->argb; }
    else if (d->kind == OK_SHAPE || d->kind == OK_MORPH || d->kind == OK_TEXT) s.def = ((ShapeObj*)d)->def;
    return s;
}
bool contentBoundsSrc(const RasterSrc& s, RectF& b) {
    if (!s.def) return false;
    switch (s.kind) {
        case OK_SHAPE: b = ((const ShapeDef*)s.def)->bounds; return true;
        case OK_MORPH: b = morphShape((const MorphDef*)s.def, s.ratio)->bounds; return true;
        case OK_TEXT: b = ((const TextDef*)s.def)->bounds; return true;
        case OK_EDITTEXT: b = ((const EditTextDef*)s.def)->bounds; b.y1 += ((const EditTextDef*)s.def)->height; return true;
        default: return false;
    }
}
void paintContentSrc(plutovg_canvas_t* cv, const RasterSrc& s, const Mat& m, const CX* cx) {
    switch (s.kind) {
        case OK_SHAPE: paintShape(cv, (const ShapeDef*)s.def, m, cx); break;
        case OK_MORPH: paintShape(cv, morphShape((const MorphDef*)s.def, s.ratio), m, cx); break;
        case OK_TEXT: paintText(cv, (const TextDef*)s.def, m, cx); break;
        case OK_EDITTEXT: { static const std::string empty; paintEditText(cv, (const EditTextDef*)s.def, s.text ? *s.text : empty, s.argb, m, cx); break; }
        default: break;
    }
}
bool contentBounds(DisplayObject* d, RectF& b) { return contentBoundsSrc(rasterSrcOf(d), b); }
void paintContent(plutovg_canvas_t* cv, DisplayObject* d, const Mat& m, const CX* cx) { paintContentSrc(cv, rasterSrcOf(d), m, cx); }

bool rasterizeSrc(const RasterSrc& src, double scale, Raster& out) {
    RectF b;
    if (!contentBoundsSrc(src, b) || !b.valid()) return false;
    double pad = 2.0 / scale;
    b.x0 -= pad; b.y0 -= pad; b.x1 += pad; b.y1 += pad;
    int w = (int)std::ceil((b.x1 - b.x0) * scale), h = (int)std::ceil((b.y1 - b.y0) * scale);
    if (w <= 0 || h <= 0) return false;
#ifdef __PSP__
    const int MAXT = 512;    // (the GE's largest texture, an atlas page: bigger ones are rasterized smaller and scaled up)
#else
    const int MAXT = 4096;
#endif
    if (w > MAXT || h > MAXT) { double f = std::min((double)MAXT / w, (double)MAXT / h); scale *= f; w = std::min(MAXT, (int)std::ceil((b.x1 - b.x0) * scale)); h = std::min(MAXT, (int)std::ceil((b.y1 - b.y0) * scale)); }
    out.w = w; out.h = h; out.x0 = b.x0; out.y0 = b.y0; out.scale = scale;
    out.px.assign((size_t)w * h, 0);
    plutovg_surface_t* s = plutovg_surface_create_for_data((unsigned char*)out.px.data(), w, h, w * 4);
    plutovg_canvas_t* cv = plutovg_canvas_create(s);
    Mat m; m.a = scale; m.d = scale; m.tx = -b.x0 * scale; m.ty = -b.y0 * scale;
    g_frontToBack = src.kind == OK_SHAPE || src.kind == OK_MORPH;
    paintContentSrc(cv, src, m, nullptr);
    g_frontToBack = false;
    plutovg_canvas_destroy(cv); plutovg_surface_destroy(s);
    return true;
}
bool rasterize(DisplayObject* d, double scale, Raster& out) { return rasterizeSrc(rasterSrcOf(d), scale, out); }

// ============================================================ raster threads
// Each thread that calls parallelFor (logic: BitmapData.draw; render: new textures) has its own
// thread pool, so they never wait on each other. While a job runs, the data it reads
// (shape definitions, locked caches) does not change.
struct ThreadPool {
    std::mutex mu; std::condition_variable cv, done;
    const std::function<void(int)>* fn = nullptr;
    std::atomic<int> next{0};
    int n = 0, busy = 0, nthreads = 0;
    uint64_t gen = 0;
    void worker() {
        uint64_t seen = 0;
        std::unique_lock<std::mutex> lk(mu);
        for (;;) {
            cv.wait(lk, [&] { return gen != seen || platformStopping(); });
            if (platformStopping()) return;
            seen = gen;
            const std::function<void(int)>* f = fn; int cnt = n;
            busy++;
            lk.unlock();
            for (int i; (i = next.fetch_add(1)) < cnt;) (*f)(i);
            lk.lock();
            if (--busy == 0) done.notify_all();
        }
    }
    void run(int cnt, const std::function<void(int)>& f) {
        {
            std::unique_lock<std::mutex> lk(mu);
            done.wait(lk, [&] { return busy == 0; });   // no thread is still working on a previous job
            fn = &f; n = cnt; next = 0; gen++;
        }
        cv.notify_all();
        for (int i; (i = next.fetch_add(1)) < cnt;) f(i);
        std::unique_lock<std::mutex> lk(mu);
        done.wait(lk, [&] { return busy == 0; });
    }
};
static thread_local ThreadRole tl_role = ROLE_RENDER;
void setThreadRole(ThreadRole r) { tl_role = r; }
static int poolSize() {
    int n = platformPoolWorkers(tl_role);
    if (const char* e = getenv("ISAAC_THREADS")) n = std::max(0, std::min(15, atoi(e)));
    return n;
}
static thread_local ThreadPool* tl_pool = nullptr;   // (never destroyed: the threads wait on it at exit)
struct WorkerStart { ThreadPool* p; int core, prio; };
static ThreadPool* myPool() {
    if (!tl_pool) {
        tl_pool = new ThreadPool();
        { ThreadPool* p = tl_pool; platformOnStop([p] { std::lock_guard<std::mutex> lk(p->mu); p->cv.notify_all(); }); }
        int want = poolSize(), got = 0;
        for (int i = 0; i < want; i++) {
            int core = -1, prio = -1; platformPoolWorkerPlace(tl_role, i, &core, &prio);
            auto* ws = new WorkerStart{tl_pool, core, prio};
            bool ok = platformStartThread([](void* a) {
                WorkerStart w = *(WorkerStart*)a; delete (WorkerStart*)a;
                platformSetThread(w.core, w.prio); w.p->worker();
            }, ws, core, "raster");
            if (!ok) { delete ws; break; }   // with fewer threads (or none) it still works, just slower
            got++;
        }
        tl_pool->nthreads = got;
        if (got < want) logf("raster threads: %d of %d\n", got, want);
    }
    return tl_pool;
}
int rasterThreads() { return myPool()->nthreads; }
void parallelFor(int n, const std::function<void(int)>& f) {
    if (n <= 0) return;
    ThreadPool* p = myPool();
    if (n == 1 || p->nthreads == 0) { for (int i = 0; i < n; i++) f(i); return; }
    p->run(n, f);
}

// ============================================================ BitmapData.draw
// Two phases: (1) walk the tree and plan each vector leaf (shape, text) with its final matrix;
// (2) rasterize in parallel those not in the leaf cache and composite them in order.
// Leaves are rasterized with the fractional part of the translation quantized to 1/64 px and composited
// at an integer offset (premultiplied source-over, associative), so the result is the same as
// painting them directly. The same pieces (backgrounds, walls, rocks) repeat across rooms.
struct LeafKey {
    const void* def; uint32_t variant; float a, b, c, d; float cx[8]; uint32_t fxy;
    bool operator==(const LeafKey& o) const { return !memcmp(this, &o, sizeof *this); }
};
struct LeafHash { size_t operator()(const LeafKey& k) const {
    const uint8_t* p = (const uint8_t*)&k; uint64_t h = 1469598103934665603ull;
    for (size_t i = 0; i < sizeof k; i++) { h ^= p[i]; h *= 1099511628211ull; }
    return (size_t)h;
} };
struct LeafRaster { std::vector<uint32_t> px; int w = 0, h = 0; uint64_t lastUse = 0; };
struct LeafPlan { LeafKey key; DisplayObject* d; Mat mm; CX cx; bool hasCx; int bx, by, w, h, dx, dy; };
static std::unordered_map<LeafKey, LeafRaster*, LeafHash> g_leafCache;
static size_t g_leafBytes = 0; static uint64_t g_leafSeq = 0;

struct DrawCtx {
    bool collect = false; int clip = 0; BitmapData* bd = nullptr;
    std::vector<LeafPlan> plans;
    bool any = false; int x0 = 0, y0 = 0, x1 = 0, y1 = 0;   // dirty rectangle
    int nFast = 0, nSlow = 0, nDirect = 0;   // statistics (ISAAC_DRAWPROF)
    void grow(int a, int b, int c, int d) {
        a = std::max(a, 0); b = std::max(b, 0); c = std::min(c, bd->w); d = std::min(d, bd->h);
        if (c <= a || d <= b) return;
        if (!any) { x0 = a; y0 = b; x1 = c; y1 = d; any = true; }
        else { x0 = std::min(x0, a); y0 = std::min(y0, b); x1 = std::max(x1, c); y1 = std::max(y1, d); }
    }
    void growMat(const Mat& m, double bx0, double by0, double bx1, double by1) {
        double xs[4] = {bx0, bx1, bx0, bx1}, ys[4] = {by0, by0, by1, by1}, lx = 1e300, ly = 1e300, hx = -1e300, hy = -1e300;
        for (int i = 0; i < 4; i++) { double x, y; m.apply(xs[i], ys[i], x, y); lx = std::min(lx, x); hx = std::max(hx, x); ly = std::min(ly, y); hy = std::max(hy, y); }
        if (!(lx < 1e9 && ly < 1e9 && hx > -1e9 && hy > -1e9)) { grow(0, 0, bd->w, bd->h); return; }
        grow((int)std::floor(lx) - 2, (int)std::floor(ly) - 2, (int)std::ceil(hx) + 2, (int)std::ceil(hy) + 2);
    }
};
static DrawCtx g_dc;
static bool g_noLeaf = getenv("ISAAC_NOLEAF") != nullptr;   // TEMP

static bool leafIdent(DisplayObject* d, const void*& def, uint32_t& variant) {
    variant = 0;
    switch (d->kind) {
        case OK_SHAPE: case OK_TEXT: def = ((ShapeObj*)d)->def; return true;
        case OK_MORPH: def = ((ShapeObj*)d)->def; variant = d->ratio & 0xffc0; return true;
        case OK_EDITTEXT: def = ((EditTextObj*)d)->def; variant = ((EditTextObj*)d)->version; return true;
        default: return false;
    }
}

static bool planLeaf(DisplayObject* d, const Mat& m, const CX& cx, LeafPlan& p) {
    if (!std::isfinite(m.a) || !std::isfinite(m.b) || !std::isfinite(m.c) || !std::isfinite(m.d) || !std::isfinite(m.tx) || !std::isfinite(m.ty)) return false;
    if (std::fabs(m.tx) > 1e6 || std::fabs(m.ty) > 1e6) return false;
    const void* def; uint32_t variant;
    if (!leafIdent(d, def, variant)) return false;
    RectF b; if (!contentBounds(d, b) || !b.valid()) return false;
    double ftx = std::floor(m.tx), fty = std::floor(m.ty);
    const int kSub = 64;   // subpixel quantized to 1/64 px
    int fx = (int)std::lround((m.tx - ftx) * kSub), fy = (int)std::lround((m.ty - fty) * kSub);
    if (fx >= kSub) { fx = 0; ftx += 1; } if (fy >= kSub) { fy = 0; fty += 1; }
    Mat mm = m; mm.tx = (double)fx / kSub; mm.ty = (double)fy / kSub;
    double xs[4] = {b.x0, b.x1, b.x0, b.x1}, ys[4] = {b.y0, b.y0, b.y1, b.y1}, lx = 1e300, ly = 1e300, hx = -1e300, hy = -1e300;
    for (int i = 0; i < 4; i++) { double x, y; mm.apply(xs[i], ys[i], x, y); lx = std::min(lx, x); hx = std::max(hx, x); ly = std::min(ly, y); hy = std::max(hy, y); }
    int x0 = (int)std::floor(lx) - 2, y0 = (int)std::floor(ly) - 2, x1 = (int)std::ceil(hx) + 2, y1 = (int)std::ceil(hy) + 2;
    p.w = x1 - x0; p.h = y1 - y0;
#ifdef __PSP__
    if (p.w <= 0 || p.h <= 0 || p.w > 1024 || p.h > 1024) return false;   // (memory: big ones painted straight into the bitmap)
#else
    if (p.w <= 0 || p.h <= 0 || p.w > 4096 || p.h > 4096) return false;
#endif
    p.d = d; p.mm = mm; p.cx = cx; p.hasCx = !cx.identity();
    p.bx = x0; p.by = y0; p.dx = (int)ftx + x0; p.dy = (int)fty + y0;
    memset(&p.key, 0, sizeof p.key);
    p.key.def = def; p.key.variant = variant;
    p.key.a = (float)m.a; p.key.b = (float)m.b; p.key.c = (float)m.c; p.key.d = (float)m.d;
    if (p.hasCx) { p.key.cx[0] = cx.rm; p.key.cx[1] = cx.gm; p.key.cx[2] = cx.bm; p.key.cx[3] = cx.am; p.key.cx[4] = cx.ra; p.key.cx[5] = cx.ga; p.key.cx[6] = cx.ba; p.key.cx[7] = cx.aa; }
    else { p.key.cx[0] = p.key.cx[1] = p.key.cx[2] = p.key.cx[3] = 1; }
    p.key.fxy = (uint32_t)(fx | fy << 8);
    return true;
}

// rasterizes rows [y0, y1) of a leaf (large leaves are split into bands across the threads;
// the rasterizer clips per pixel, so the result is identical to a single pass)
static void rasterLeafRows(const LeafPlan& p, LeafRaster* r, int y0, int y1) {
    static bool prof = getenv("ISAAC_LEAFPROF") != nullptr; uint64_t t0 = prof ? SDL_GetPerformanceCounter() : 0;
    struct Rep { const LeafPlan& p; int y0, y1; uint64_t t0; ~Rep() { if (t0) { double ms = (SDL_GetPerformanceCounter() - t0) * 1000.0 / SDL_GetPerformanceFrequency();
        if (ms > 2) { fprintf(stderr, "[leaf] id=%d kind=%d %dx%d rows %d-%d %.2f ms", p.d->charId, p.d->kind, p.w, p.h, y0, y1, ms);
            if (p.d->kind == OK_SHAPE) { const ShapeDef* sd = (const ShapeDef*)((ShapeObj*)p.d)->def; size_t verbs = 0, nf = 0, nl = 0; std::string types;
                for (auto& L : sd->layers) { for (auto& f : L.fills) { nf++; char b[8]; snprintf(b, 8, "%x,", f.type); types += b; } for (auto& fp : L.fillPaths) verbs += fp.verbs.size(); for (auto& lp : L.linePaths) verbs += lp.verbs.size(); nl += L.lines.size(); }
                fprintf(stderr, " layers %zu fills %zu lines %zu verbs %zu types %s", sd->layers.size(), nf, nl, verbs, types.substr(0, 80).c_str()); }
            fprintf(stderr, "\n"); } } } } rep{p, y0, y1, t0};
    plutovg_surface_t* s = plutovg_surface_create_for_data((unsigned char*)(r->px.data() + (size_t)y0 * p.w), p.w, y1 - y0, p.w * 4);
    plutovg_canvas_t* cv = plutovg_canvas_create(s);
    Mat t; t.tx = -p.bx; t.ty = -p.by - y0;
    g_frontToBack = p.d->kind == OK_SHAPE || p.d->kind == OK_MORPH;
    paintContent(cv, p.d, t * p.mm, p.hasCx ? &p.cx : nullptr);
    g_frontToBack = false;
    plutovg_canvas_destroy(cv); plutovg_surface_destroy(s);
}

static inline uint32_t byteMul(uint32_t x, uint32_t a) {
    uint32_t t = (x & 0xff00ff) * a; t = (t + ((t >> 8) & 0xff00ff) + 0x800080) >> 8; t &= 0xff00ff;
    x = ((x >> 8) & 0xff00ff) * a; x = x + ((x >> 8) & 0xff00ff) + 0x800080; x &= 0xff00ff00;
    return x | t;
}
// composites src (premultiplied) over the destination BitmapData at (dx, dy)
static void blendOver(const uint32_t* src, int sw, int sh, int sstride, int dx, int dy) {
    BitmapData* bd = g_dc.bd;
    int x0 = std::max(0, dx), y0 = std::max(0, dy), x1 = std::min(bd->w, dx + sw), y1 = std::min(bd->h, dy + sh);
    if (x1 <= x0 || y1 <= y0) return;
    for (int y = y0; y < y1; y++) {
        const uint32_t* s = src + (size_t)(y - dy) * sstride + (x0 - dx);
        uint32_t* d = bd->px.data() + (size_t)y * bd->w + x0;
        for (int i = 0, n = x1 - x0; i < n; i++) {
            uint32_t p = s[i], a = p >> 24;
            if (a == 255) d[i] = p; else if (a) d[i] = p + byteMul(d[i], 255 - a);
        }
    }
    g_dc.grow(x0, y0, x1, y1);
}

static void drawTree(plutovg_canvas_t* cv, Object* o, const Mat& m, const CX& cx, int depth);

static void drawBitmapData(plutovg_canvas_t* cv, BitmapData* b, const Mat& m, const CX& cx, bool smooth) {
    if (g_dc.collect || !b->w || !b->h || b == g_dc.bd) { if (b == g_dc.bd && !g_dc.collect) logf("BitmapData.draw onto itself: ignored\n"); return; }
    if (b->gpu) bdNeedCpu(b);   // (a CPU draw that includes a GPU layer: should not happen in this game)
    b->ensurePx();
    std::vector<uint32_t> tmp;
    const uint32_t* src = b->px.data();
    if (!cx.identity()) {
        tmp = b->px;
        for (auto& p : tmp) {
            float a = (p >> 24) / 255.f; if (a <= 0) { p = 0; continue; }
            p = pxToArgb(p);
            float r = ((p >> 16) & 255) / 255.f / a, g = ((p >> 8) & 255) / 255.f / a, bb = (p & 255) / 255.f / a;
            float nr = clamp01(r * cx.rm + cx.ra / 255.f), ng = clamp01(g * cx.gm + cx.ga / 255.f), nb = clamp01(bb * cx.bm + cx.ba / 255.f), na = clamp01(a * cx.am + cx.aa / 255.f);
            p = pxFromArgb((uint32_t)(na * 255 + .5f) << 24 | (uint32_t)(nr * na * 255 + .5f) << 16 | (uint32_t)(ng * na * 255 + .5f) << 8 | (uint32_t)(nb * na * 255 + .5f));
        }
        src = tmp.data();
    }
    // fast path: scale 1 and integer translation (e.g. the room background when copying the screen)
    double rx = std::round(m.tx), ry = std::round(m.ty);
    if (g_dc.clip == 0 && !g_noLeaf && std::fabs(m.a - 1) < 1e-6 && std::fabs(m.d - 1) < 1e-6 && std::fabs(m.b) < 1e-6 && std::fabs(m.c) < 1e-6 &&
        std::fabs(m.tx - rx) < 1e-3 && std::fabs(m.ty - ry) < 1e-3 && std::fabs(rx) < 1e6 && std::fabs(ry) < 1e6) {
        blendOver(src, b->w, b->h, b->w, (int)rx, (int)ry); g_dc.nFast++;
        return;
    }
    plutovg_surface_t* s = plutovg_surface_create_for_data((unsigned char*)src, b->w, b->h, b->w * 4);
    plutovg_texture_smooth = smooth ? 1 : 0;
    plutovg_matrix_t mm = pm(m);
    plutovg_canvas_set_matrix(cv, &mm);
    plutovg_matrix_t id{1, 0, 0, 1, 0, 0};
    plutovg_canvas_set_texture(cv, s, PLUTOVG_TEXTURE_TYPE_PLAIN, 1.0f, &id);
    plutovg_canvas_new_path(cv);
    plutovg_canvas_move_to(cv, 0, 0); plutovg_canvas_line_to(cv, (float)b->w, 0); plutovg_canvas_line_to(cv, (float)b->w, (float)b->h);
    plutovg_canvas_line_to(cv, 0, (float)b->h); plutovg_canvas_close_path(cv);
    plutovg_canvas_fill(cv);
    plutovg_surface_destroy(s); g_dc.nSlow++;
    if (getenv("ISAAC_DRAWPROF")) fprintf(stderr, "  [slow bitmap] %dx%d m=(%.4f %.4f %.4f %.4f %.3f %.3f) clip=%d\n", b->w, b->h, m.a, m.b, m.c, m.d, m.tx, m.ty, g_dc.clip);
    g_dc.growMat(m, 0, 0, b->w, b->h);
}

static void addMaskPaths(plutovg_canvas_t* cv, DisplayObject* d, const Mat& m, int depth) {
    if (depth > 32) return;
    if (d->kind == OK_SHAPE) {
        ShapeDef* s = (ShapeDef*)((ShapeObj*)d)->def; shapeEnsure(s);
        plutovg_matrix_t mm = pm(m); plutovg_canvas_set_matrix(cv, &mm);
        for (auto& L : s->layers) for (auto& p : L.fillPaths) addPath(cv, p);
    } else if (d->kind == OK_CLIP) {
        for (auto* k : ((MovieClip*)d)->kids) addMaskPaths(cv, k, m * k->m, depth + 1);
    }
}

static void drawChildren(plutovg_canvas_t* cv, const std::vector<DisplayObject*>& kids, const Mat& m, const CX& cx, int depth) {
    for (size_t i = 0; i < kids.size(); i++) {
        DisplayObject* k = kids[i];
        if (k->clipDepth) {
            // mask: clips siblings up to clipDepth (painted directly, without the leaf cache)
            size_t j = i + 1;
            while (j < kids.size() && kids[j]->depth <= k->clipDepth) j++;
            if (!g_dc.collect) {
                plutovg_canvas_save(cv);
                plutovg_canvas_new_path(cv);
                plutovg_canvas_set_fill_rule(cv, PLUTOVG_FILL_RULE_NON_ZERO);
                addMaskPaths(cv, k, m * k->m, 0);
                plutovg_canvas_clip(cv);
                g_dc.clip++;
                for (size_t q = i + 1; q < j; q++) drawTree(cv, kids[q], m * kids[q]->m, cx * kids[q]->cx, depth + 1);
                g_dc.clip--;
                plutovg_canvas_restore(cv);
            }
            i = j - 1;
            continue;
        }
        drawTree(cv, k, m * k->m, cx * k->cx, depth + 1);
    }
}

static void drawLeaf(plutovg_canvas_t* cv, DisplayObject* d, const Mat& m, const CX& cx) {
    if (g_dc.clip == 0 && !g_noLeaf) {
        LeafPlan p;
        if (planLeaf(d, m, cx, p)) {
            BitmapData* bd = g_dc.bd;
            if (p.dx >= bd->w || p.dy >= bd->h || p.dx + p.w <= 0 || p.dy + p.h <= 0) return;   // outside the bitmap
            if (g_dc.collect) { g_dc.plans.push_back(p); return; }
            auto it = g_leafCache.find(p.key);
            if (it != g_leafCache.end()) {
                LeafRaster* r = it->second; r->lastUse = g_leafSeq;
                blendOver(r->px.data(), r->w, r->h, r->w, p.dx, p.dy);
                return;
            }
        }
    }
    if (g_dc.collect) return;
    paintContent(cv, d, m, cx.identity() ? nullptr : &cx); g_dc.nDirect++;
    RectF b; if (contentBounds(d, b)) g_dc.growMat(m, b.x0, b.y0, b.x1, b.y1);
}

static void drawTree(plutovg_canvas_t* cv, Object* o, const Mat& m, const CX& cx, int depth) {
    if (depth > 64) return;
    if (o->kind == OK_BITMAPDATA) { drawBitmapData(cv, (BitmapData*)o, m, cx, true); return; }
    if (!o->isDisplay()) return;
    DisplayObject* d = (DisplayObject*)o;
    if (!d->visible && depth > 0) return;
    switch (d->kind) {
        case OK_CLIP: drawChildren(cv, ((MovieClip*)d)->kids, m, cx, depth); break;
        case OK_BUTTON: drawChildren(cv, ((ButtonObj*)d)->kids, m, cx, depth); break;
        case OK_BITMAP: if (((BitmapObj*)d)->bd) drawBitmapData(cv, ((BitmapObj*)d)->bd, m, cx, ((BitmapObj*)d)->smoothing); break;
        default: drawLeaf(cv, d, m, cx); break;
    }
}

static void evictLeaves() {
    if (g_leafBytes <= g_lim.leafBudgetBytes) return;
    std::vector<std::pair<uint64_t, LeafKey>> order;
    for (auto& kv : g_leafCache) order.push_back({kv.second->lastUse, kv.first});
    std::sort(order.begin(), order.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    for (auto& o : order) {
#ifdef __PSP__
        if (g_leafBytes <= g_lim.leafBudgetBytes * 3 / 4) break;   // (memory: even the leaves of this draw, it is done)
#else
        if (g_leafBytes <= g_lim.leafBudgetBytes * 3 / 4 || o.first == g_leafSeq) break;
#endif
        auto it = g_leafCache.find(o.second);
        g_leafBytes -= it->second->px.size() * 4; delete it->second; g_leafCache.erase(it);
    }
}

double g_bmDrawMs = 0;   // (diagnostics) time in BitmapData.draw
void bitmapDrawImpl(BitmapData* bd, Object* src, const Mat& m, const CX* cx);
void bitmapDraw(BitmapData* bd, Object* src, const Mat& m, const CX* cx) {
    auto t0 = std::chrono::steady_clock::now();
    bitmapDrawImpl(bd, src, m, cx);
    g_bmDrawMs += std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
}
void bitmapDrawImpl(BitmapData* bd, Object* src, const Mat& m, const CX* cx) {
    if (!bd->w || !bd->h || bd->disposed) return;
    bd->ensurePx();
    static bool prof = getenv("ISAAC_DRAWPROF") != nullptr;
    uint64_t t0 = prof ? SDL_GetPerformanceCounter() : 0;
    CX c = cx ? *cx : CX();
    g_leafSeq++;
    g_dc = DrawCtx(); g_dc.bd = bd;
    // phase 1: plan leaves and rasterize the missing ones in parallel
    g_dc.collect = true;
    drawTree(nullptr, src, m, c, 0);
    std::vector<const LeafPlan*> miss;
    {
        std::unordered_map<LeafKey, int, LeafHash> seen;
        for (auto& p : g_dc.plans) if (!g_leafCache.count(p.key) && seen.emplace(p.key, 0).second) miss.push_back(&p);
    }
    int nmiss = (int)miss.size();
    if (!miss.empty()) {
        std::vector<LeafRaster*> out(miss.size());
        struct Job { int leaf, y0, y1; size_t cost; };
        std::vector<Job> jobs;
        int nt = rasterThreads() + 1;
        for (size_t i = 0; i < miss.size(); i++) {
            const LeafPlan& p = *miss[i];
            LeafRaster* r = out[i] = new LeafRaster(); r->w = p.w; r->h = p.h; r->px.assign((size_t)p.w * p.h, 0);
            int bands = 1;
            if (nt > 1 && (size_t)p.w * p.h > 150000) bands = std::max(1, std::min(nt * 2, p.h / 48));
            for (int b = 0; b < bands; b++) jobs.push_back({(int)i, p.h * b / bands, p.h * (b + 1) / bands, (size_t)p.w * (p.h / bands)});
        }
        std::sort(jobs.begin(), jobs.end(), [](const Job& a, const Job& b) { return a.cost > b.cost; });
        parallelFor((int)jobs.size(), [&](int j) { const Job& jb = jobs[j]; rasterLeafRows(*miss[jb.leaf], out[jb.leaf], jb.y0, jb.y1); });
        for (size_t i = 0; i < miss.size(); i++) { out[i]->lastUse = g_leafSeq; g_leafBytes += out[i]->px.size() * 4; g_leafCache[miss[i]->key] = out[i]; }
    }
    uint64_t t1 = prof ? SDL_GetPerformanceCounter() : 0;
    // phase 2: composite in order
    g_dc.collect = false; g_dc.plans.clear();
    plutovg_surface_t* s = plutovg_surface_create_for_data((unsigned char*)bd->px.data(), bd->w, bd->h, bd->w * 4);
    plutovg_canvas_t* cv = plutovg_canvas_create(s);
    drawTree(cv, src, m, c, 0);
    plutovg_canvas_destroy(cv); plutovg_surface_destroy(s);
    if (g_dc.any) {
        if (!bd->transparent)
            for (int y = g_dc.y0; y < g_dc.y1; y++) { uint32_t* row = bd->px.data() + (size_t)y * bd->w; for (int x = g_dc.x0; x < g_dc.x1; x++) row[x] |= 0xff000000u; }
        bd->dirty(g_dc.x0, g_dc.y0, g_dc.x1, g_dc.y1);
    }
    evictLeaves();
    if (prof) {
        double f = 1000.0 / SDL_GetPerformanceFrequency();
        fprintf(stderr, "[draw] %dx%d src kind=%d id=%d m=(%.3f %.3f %.1f %.1f) %.2f ms (raster %d new leaves in %.2f ms) dirty %dx%d bitmaps %d fast %d slow, %d direct leaves\n", bd->w, bd->h, (int)src->kind,
            src->isDisplay() ? (int)((DisplayObject*)src)->charId : -1, m.a, m.d, m.tx, m.ty, (SDL_GetPerformanceCounter() - t0) * f, nmiss, (t1 - t0) * f,
            g_dc.any ? g_dc.x1 - g_dc.x0 : 0, g_dc.any ? g_dc.y1 - g_dc.y0 : 0, g_dc.nFast, g_dc.nSlow, g_dc.nDirect);
    }
    g_dc = DrawCtx();
}

void rasterMemStats() {
    size_t bm = 0; for (auto& kv : g_bmSurf) bm += (size_t)plutovg_surface_get_width(kv.second.s) * plutovg_surface_get_height(kv.second.s) * 4;
    size_t mor = 0; for (auto& kv : g_morphCache) for (auto& L : kv.second->layers) for (auto& P : L.fillPaths) mor += P.pts.size() * 4 + P.verbs.size();
    size_t pts = 0, hit = 0, nsh = 0;
    for (CharDef* cd : g_movie.chars) if (cd && cd->type == CH_SHAPE) { ShapeDef* sd = (ShapeDef*)cd; nsh++;
        for (auto& L : sd->layers) { for (auto& P : L.fillPaths) pts += P.pts.capacity() * 4 + P.verbs.capacity(); for (auto& P : L.linePaths) pts += P.pts.capacity() * 4 + P.verbs.capacity(); }
        for (auto& h : sd->hitPolys) hit += h.capacity() * 4; }
    logf("[mem] shapes %zu: paths %zu MB, hit polygons %zu MB\n", nsh, pts >> 20, hit >> 20);
    logf("[mem] bitmap surfaces %zu MB (%zu), morphs %zu (%zu MB), leaves %zu MB (%zu)\n", bm >> 20, g_bmSurf.size(), g_morphCache.size(), mor >> 20, g_leafBytes >> 20, g_leafCache.size());
}
}  // namespace fl
