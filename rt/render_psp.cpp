// PSP renderer (sceGu): draws a snapshot with textured quads. Same interface as render.cpp (GL), much simpler:
// - one thread: the frame is drawn right after the logic tick that built the snapshot;
// - vector graphics are rasterized on the CPU (raster.cpp / plutovg) into 512x512 atlas pages in main memory
//   (the GE reads textures from RAM); least recently used pages are recycled when the budget is reached;
// - BitmapData layers are kept as textures in RAM, updated with the snapshot's uploads (dirty rectangles);
// - colour transforms: multiply via the vertex colour, additive terms with a second pass of an alpha-only copy;
// - masks with the stencil (the alpha bits of the 8888 frame buffer; normal drawing never writes them);
// - no blur filters, no screen captures, no GPU drawing into layers (all BitmapData stay on the CPU).
#ifdef __PSP__
#include "render.h"
#include "raster.h"
#include "core.h"
#include <pspkernel.h>
#include <pspgu.h>
#include <pspdisplay.h>
#include <psputils.h>
#include <malloc.h>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <unordered_map>

namespace fl {
RenderConfig g_renderCfg;
RenderStats g_rstats;
Mat g_stageToWindow;
bool g_glES = false;
bool g_retireSnapshots = false;

static const int kBufW = 512, kScrW = 480, kScrH = 272;
static unsigned int __attribute__((aligned(64))) g_list[384 * 1024 / 4];   // display list (~150 bytes a quad)
static void* const kFb0 = (void*)0;                    // VRAM offsets
static void* const kFb1 = (void*)(kBufW * kScrH * 4);
static void* g_drawBuf = kFb0;
static bool g_inFrame = false;

struct V { float u, v; uint32_t c; float x, y, z; };
static const int kVFmt = GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D;

static inline uint32_t vcol(float r, float g, float b, float a) {
    auto q = [](float v) { int i = (int)(v * 255.f + 0.5f); return (uint32_t)(i < 0 ? 0 : i > 255 ? 255 : i); };
    return q(r) | q(g) << 8 | q(b) << 16 | q(a) << 24;
}

// ---------------------------------------------------------------- atlas (rasterized shapes, texts, morphs)
static const int kPage = 512;
struct Page { uint32_t* px = nullptr; int nextY = 0; struct Shelf { int y, h, x; }; std::vector<Shelf> shelves; uint32_t lastUse = 0; };
static std::vector<Page> g_pages;
static int g_maxPages = 5;   // (512x512x4: 1 MB each)
static uint32_t g_frameNo = 0;
struct TexKey { const void* def; uint32_t variant; int16_t bucket; bool mask;
    bool operator==(const TexKey& o) const { return def == o.def && variant == o.variant && bucket == o.bucket && mask == o.mask; } };
struct TexKeyHash { size_t operator()(const TexKey& k) const { return std::hash<const void*>()(k.def) ^ (k.variant * 0x9e3779b1u) ^ ((size_t)k.bucket << 20) ^ (k.mask ? 0x55555555u : 0); } };
struct TexEntry { int page = -1, px = 0, py = 0, w = 0, h = 0; float x0 = 0, y0 = 0, scale = 1; bool empty = false; };
static std::unordered_map<TexKey, TexEntry, TexKeyHash> g_cache;

static bool shelfAlloc(Page& p, int w, int h, int& ox, int& oy) {
    for (auto& s : p.shelves) if (h <= s.h && s.x + w <= kPage) { ox = s.x; oy = s.y; s.x += w; return true; }
    if (p.nextY + h > kPage) return false;
    p.shelves.push_back({p.nextY, h, w}); ox = 0; oy = p.nextY; p.nextY += h;
    return true;
}
static void pageReset(int i) {
    Page& p = g_pages[i];
    p.shelves.clear(); p.nextY = 0;
    for (auto it = g_cache.begin(); it != g_cache.end();) { if (it->second.page == i) it = g_cache.erase(it); else ++it; }
}
static bool atlasPlace(int w, int h, int& page, int& ox, int& oy) {
    if (w > kPage || h > kPage) return false;
    for (size_t i = 0; i < g_pages.size(); i++) if (shelfAlloc(g_pages[i], w, h, ox, oy)) { page = (int)i; return true; }
    if ((int)g_pages.size() < g_maxPages) {
        Page p; p.px = (uint32_t*)memalign(64, kPage * kPage * 4);
        if (p.px) { g_pages.push_back(p); page = (int)g_pages.size() - 1; return shelfAlloc(g_pages[page], w, h, ox, oy); }
    }
    // recycle the least recently used page (not one used in this frame: its quads are already in the list)
    int best = -1;
    for (size_t i = 0; i < g_pages.size(); i++) if (g_pages[i].lastUse != g_frameNo && (best < 0 || g_pages[i].lastUse < g_pages[best].lastUse)) best = (int)i;
    if (best < 0) return false;
    pageReset(best);
    page = best; return shelfAlloc(g_pages[best], w, h, ox, oy);
}
static TexEntry* storePixels(const TexKey& k, const Raster& r, bool mask) {
    TexEntry e;
    if (r.w < 3 || r.h < 3) { e.empty = true; return &(g_cache[k] = e); }
    int w = std::min(r.w, kPage), h = std::min(r.h, kPage);
    int page, ox, oy;
    if (!atlasPlace(w, h, page, ox, oy)) { e.empty = true; return &(g_cache[k] = e); }
    Page& p = g_pages[page];
    for (int y = 0; y < h; y++) {
        uint32_t* d = p.px + (size_t)(oy + y) * kPage + ox; const uint32_t* s = r.px.data() + (size_t)y * r.w;
        if (mask) for (int x = 0; x < w; x++) { uint32_t a = s[x] >> 24; d[x] = a | a << 8 | a << 16 | a << 24; }
        else memcpy(d, s, w * 4);   // (already in the GE's order: kPxABGR)
        sceKernelDcacheWritebackRange(d, w * 4);
    }
    e.page = page; e.px = ox; e.py = oy; e.w = w; e.h = h; e.x0 = r.x0; e.y0 = r.y0; e.scale = r.scale;
    return &(g_cache[k] = e);
}

static std::shared_ptr<Snapshot> g_cur;
// The PSP's FPU is single precision only (doubles are emulated, ~100 cycles an operation): the renderer works in float.
struct FMat {
    float a = 1, b = 0, c = 0, d = 1, tx = 0, ty = 0;
    FMat() {}
    explicit FMat(const Mat& m) : a((float)m.a), b((float)m.b), c((float)m.c), d((float)m.d), tx((float)m.tx), ty((float)m.ty) {}
    FMat operator*(const FMat& o) const {
        FMat r; r.a = a * o.a + c * o.b; r.b = b * o.a + d * o.b; r.c = a * o.c + c * o.d; r.d = b * o.c + d * o.d;
        r.tx = a * o.tx + c * o.ty + tx; r.ty = b * o.tx + d * o.ty + ty; return r;
    }
    void apply(float x, float y, float& ox, float& oy) const { ox = a * x + c * y + tx; oy = b * x + d * y + ty; }
};
static float maxScale(const FMat& m) { return std::sqrt(std::max(m.a * m.a + m.b * m.b, m.c * m.c + m.d * m.d)); }
static uint64_t g_rasterSpentUs = 0; static const uint64_t kRasterBudgetUs = 10000;
static TexEntry* rasterFor(const SnapNode& n, const FMat& m, bool mask) {
    if (n.kind != SN_CONTENT || !n.def) return nullptr;
    float scale = maxScale(m);
    if (!(scale >= 1.f / 64)) scale = 1.f / 64; if (scale > 16) scale = 16;
    int bucket = (int)std::ceil(std::log2(scale) * 2.f - 1e-4f);   // half octaves (memory over sharpness)
    uint32_t variant = n.objKind == OK_EDITTEXT ? n.textVersion : n.objKind == OK_MORPH ? (uint32_t)(n.ratio & 0xffc0) : 0;
    TexKey k{n.def, variant, (int16_t)bucket, mask};
    auto it = g_cache.find(k);
    if (it != g_cache.end()) { if (it->second.empty) return nullptr; if (it->second.page >= 0) g_pages[it->second.page].lastUse = g_frameNo; return &it->second; }
    // a frame rasterizes at most ~10 ms of new graphics (entering a room: dozens at once); the rest appear in the
    // next frames instead of freezing the image (the game's logic is not affected)
    if (g_rasterSpentUs > kRasterBudgetUs) { g_rstats.rasterDeferred++; return nullptr; }
    RasterSrc s; s.kind = n.objKind; s.def = n.def; s.ratio = n.ratio; s.argb = n.argb;
    if (n.text >= 0) s.text = &g_cur->texts[n.text];
    Raster r;
    uint64_t t0 = sceKernelGetSystemTimeWide();
    bool ok = rasterizeSrc(s, std::pow(2.0, bucket / 2.0), r);
    uint64_t dt = sceKernelGetSystemTimeWide() - t0;
    g_rstats.rasters++; g_rstats.rasterMs += dt / 1000.0; g_rasterSpentUs += dt;
    if (!ok) { TexEntry e; e.empty = true; g_cache[k] = e; return nullptr; }
    TexEntry* e = storePixels(k, r, mask);
    if (e && !e->empty) g_pages[e->page].lastUse = g_frameNo;
    return e && !e->empty ? e : nullptr;
}

// ---------------------------------------------------------------- BitmapData textures
struct BmTex { int w = 0, h = 0, stride = 0; uint32_t* px = nullptr; };
static std::unordered_map<uint32_t, BmTex> g_bm;
static void bmFree(BmTex& t) { free(t.px); t.px = nullptr; }
static BmTex& bmGet(uint32_t uid, int w, int h) {
    BmTex& t = g_bm[uid];
    if (t.w != w || t.h != h || !t.px) {
        bmFree(t);
        t.w = w; t.h = h; t.stride = (w + 3) & ~3;
        t.px = (uint32_t*)memalign(64, (size_t)t.stride * h * 4);
        if (t.px) memset(t.px, 0, (size_t)t.stride * h * 4);
    }
    return t;
}

// ---------------------------------------------------------------- drawing
static int g_boundPage = -2; static const void* g_boundPtr = nullptr;
static void bindTex(const void* ptr, int tw, int th, int tbw) {
    if (ptr == g_boundPtr) return;
    sceGuTexImage(0, tw, th, tbw, ptr);
    g_boundPtr = ptr;
}
static int pow2(int v) { int p = 1; while (p < v) p <<= 1; return p; }
// textured quad: local rectangle (x0,y0)-(x1,y1) through m, texels (u0,v0)-(u1,v1)
static void quad(const FMat& m, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, uint32_t col) {
    V* v = (V*)sceGuGetMemory(6 * sizeof(V));
    float px[4], py[4];
    m.apply(x0, y0, px[0], py[0]); m.apply(x1, y0, px[1], py[1]); m.apply(x1, y1, px[2], py[2]); m.apply(x0, y1, px[3], py[3]);
    float us[4] = {u0, u1, u1, u0}, vs[4] = {v0, v0, v1, v1};
    static const int idx[6] = {0, 1, 2, 0, 2, 3};
    for (int i = 0; i < 6; i++) { int j = idx[i]; v[i] = {us[j], vs[j], col, px[j], py[j], 0.f}; }
    sceGuDrawArray(GU_TRIANGLES, kVFmt, 6, 0, v);
    g_rstats.quads++;
}
static void blendNormal() { sceGuBlendFunc(GU_ADD, GU_FIX, GU_ONE_MINUS_SRC_ALPHA, 0xffffffff, 0); }
// draws a rasterized entry with a colour transform (multiply pass, then the additive pass if any)
static void drawEntry(const SnapNode& n, const FMat& m, const CX& cx, TexEntry* e) {
    const Page& p = g_pages[e->page];
    bindTex(p.px, kPage, kPage, kPage);
    float inv = 1.f / e->scale;
    float am = std::min(1.f, std::max(0.f, cx.am));
    uint32_t col = vcol(cx.rm * am, cx.gm * am, cx.bm * am, am);
    quad(m, e->x0 + inv, e->y0 + inv, e->x0 + (e->w - 1) * inv, e->y0 + (e->h - 1) * inv,
         (float)(e->px + 1), (float)(e->py + 1), (float)(e->px + e->w - 1), (float)(e->py + e->h - 1), col);
    if ((cx.ra != 0 || cx.ga != 0 || cx.ba != 0) && cx.aa < 9 * 255) {
        TexEntry* me = rasterFor(n, m, true);
        if (!me) return;
        const Page& mp = g_pages[me->page];
        float r = cx.ra / 255.f * am, g = cx.ga / 255.f * am, b = cx.ba / 255.f * am;
        for (int sign : {1, -1}) {
            float rr = std::max(0.f, r * sign), gg = std::max(0.f, g * sign), bb = std::max(0.f, b * sign);
            if (rr <= 0 && gg <= 0 && bb <= 0) continue;
            sceGuBlendFunc(sign > 0 ? GU_ADD : GU_REVERSE_SUBTRACT, GU_FIX, GU_FIX, 0xffffffff, 0xffffffff);
            bindTex(mp.px, kPage, kPage, kPage);
            float minv = 1.f / me->scale;
            quad(m, me->x0 + minv, me->y0 + minv, me->x0 + (me->w - 1) * minv, me->y0 + (me->h - 1) * minv,
                 (float)(me->px + 1), (float)(me->py + 1), (float)(me->px + me->w - 1), (float)(me->py + me->h - 1), vcol(rr, gg, bb, 0));
            blendNormal();
        }
    }
}
static bool bmDirect(const uint32_t* px, int w);
static void drawBitmap(const SnapNode& n, const FMat& m, const CX& cx) {
    BmTex t;
    if (n.bmPx && bmDirect(n.bmPx, n.bmW)) { t.w = n.bmW; t.h = n.bmH; t.stride = n.bmW; t.px = (uint32_t*)n.bmPx; }
    else {
        auto it = g_bm.find(n.bmUid);
        if (it == g_bm.end() || !it->second.px || it->second.w != n.bmW || it->second.h != n.bmH) return;
        t = it->second;
    }
    float am = std::min(1.f, std::max(0.f, cx.am));
    uint32_t col = vcol(cx.rm * am, cx.gm * am, cx.bm * am, am);
    // in tiles of at most 512x512 (the GE's largest texture); the buffer is read in place (stride = tbw)
    for (int ty = 0; ty < t.h; ty += kPage) for (int tx = 0; tx < t.w; tx += kPage) {
        int w = std::min(kPage, t.w - tx), h = std::min(kPage, t.h - ty);
        bindTex(t.px + (size_t)ty * t.stride + tx, pow2(w), pow2(h), t.stride);
        quad(m, (float)tx, (float)ty, (float)(tx + w), (float)(ty + h), 0, 0, (float)w, (float)h, col);
    }
}

static int g_stencil = 0;
static void drawNode(uint32_t i, const FMat& parent, const CX& pcx, bool maskPass);
static std::vector<uint32_t> g_kidStack;   // (children lists of the containers being drawn, without an allocation each)
static void drawChildren(uint32_t i, const FMat& m, const CX& cx, bool maskPass) {
    const std::vector<SnapNode>& N = g_cur->nodes;
    size_t base = g_kidStack.size();
    for (uint32_t j = i + 1; j < N[i].end; j = N[j].end) g_kidStack.push_back(j);
    size_t nk = g_kidStack.size() - base;
    auto kids = [&](size_t q) { return g_kidStack[base + q]; };
    for (size_t q = 0; q < nk; q++) {
        const SnapNode& k = N[kids(q)];
        if (k.clipDepth && !maskPass) {
            size_t e = q + 1;
            while (e < nk && N[kids(e)].depth <= k.clipDepth) e++;
            // mask: write stencil = level + 1 where the mask's pixels are (colour untouched)
            sceGuPixelMask(0x00ffffff); sceGuDisable(GU_BLEND); sceGuEnable(GU_ALPHA_TEST); sceGuAlphaFunc(GU_GREATER, 0x40, 0xff);
            sceGuEnable(GU_STENCIL_TEST); sceGuStencilFunc(GU_EQUAL, g_stencil, 0xff); sceGuStencilOp(GU_KEEP, GU_KEEP, GU_INCR);
            drawNode(kids(q), m, cx, true);
            g_stencil++;
            sceGuPixelMask(0xff000000); sceGuEnable(GU_BLEND); sceGuDisable(GU_ALPHA_TEST);
            sceGuStencilFunc(GU_EQUAL, g_stencil, 0xff); sceGuStencilOp(GU_KEEP, GU_KEEP, GU_KEEP);
            for (size_t r = q + 1; r < e; r++) drawNode(kids(r), m, cx, false);
            // pop
            sceGuPixelMask(0x00ffffff); sceGuDisable(GU_BLEND); sceGuEnable(GU_ALPHA_TEST);
            sceGuStencilFunc(GU_EQUAL, g_stencil, 0xff); sceGuStencilOp(GU_KEEP, GU_KEEP, GU_DECR);
            drawNode(kids(q), m, cx, true);
            g_stencil--;
            sceGuPixelMask(0xff000000); sceGuEnable(GU_BLEND); sceGuDisable(GU_ALPHA_TEST);
            sceGuStencilFunc(GU_EQUAL, g_stencil, 0xff); sceGuStencilOp(GU_KEEP, GU_KEEP, GU_KEEP);
            if (g_stencil == 0) sceGuDisable(GU_STENCIL_TEST);
            q = e - 1;
            continue;
        }
        drawNode(kids(q), m, cx, maskPass);
    }
    g_kidStack.resize(base);
}
static void drawNode(uint32_t i, const FMat& parent, const CX& pcx, bool maskPass) {
    const SnapNode& n = g_cur->nodes[i];
    if (!n.visible && !maskPass) return;
    FMat m = parent * FMat(n.m);
    CX cx = maskPass ? CX() : pcx * n.cx;
    if (!maskPass && cx.am <= 0 && cx.aa <= 0) return;
    switch (n.kind) {
        case SN_CONTAINER: drawChildren(i, m, cx, maskPass); break;
        case SN_BITMAP: drawBitmap(n, m, cx); break;
        case SN_CONTENT: { TexEntry* e = rasterFor(n, m, false); if (e) drawEntry(n, m, cx, e); break; }
    }
}

// ---------------------------------------------------------------- frame
static const Snapshot* g_prevForFree = nullptr;
bool renderInit() {
    sceGuInit();
    sceGuStart(GU_DIRECT, g_list);
    sceGuDrawBuffer(GU_PSM_8888, kFb0, kBufW);
    sceGuDispBuffer(kScrW, kScrH, kFb1, kBufW);
    sceGuDepthBuffer((void*)(kBufW * kScrH * 8), kBufW);
    sceGuOffset(2048 - kScrW / 2, 2048 - kScrH / 2);
    sceGuViewport(2048, 2048, kScrW, kScrH);
    sceGuScissor(0, 0, kScrW, kScrH); sceGuEnable(GU_SCISSOR_TEST);
    sceGuDisable(GU_DEPTH_TEST); sceGuDepthMask(GU_TRUE);
    sceGuEnable(GU_TEXTURE_2D); sceGuTexMode(GU_PSM_8888, 0, 0, 0);
    sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA); sceGuTexFilter(GU_LINEAR, GU_LINEAR); sceGuTexWrap(GU_CLAMP, GU_CLAMP);
    sceGuEnable(GU_BLEND); blendNormal();
    sceGuShadeModel(GU_FLAT);
    sceGuFinish(); sceGuSync(0, 0);
    sceDisplayWaitVblankStart(); sceGuDisplay(GU_TRUE);
    g_stageToWindow = stageToWindowFor(kScrW, kScrH);
    return true;
}
Mat stageToWindowFor(int winW, int winH) {
    double s = std::min((double)winW / g_stageW, (double)winH / g_stageH);
    Mat m; m.a = m.d = s; m.tx = std::floor((winW - g_stageW * s) / 2); m.ty = std::floor((winH - g_stageH * s) / 2);
    return m;
}
// BitmapData pixels are in the GE's order (kPxABGR): drawn in place when the rows are 16-byte aligned (width a multiple
// of 4); otherwise copied into a texture with aligned rows. Either way the changed rows go from the data cache to RAM.
static bool bmDirect(const uint32_t* px, int w) { return (w & 3) == 0 && ((uintptr_t)px & 15) == 0; }
void pspBitmapUpload(uint32_t uid, int w, int h, int x0, int y0, int x1, int y1, const uint32_t* px) {
    if (bmDirect(px, w)) {
        auto it = g_bm.find(uid); if (it != g_bm.end()) { bmFree(it->second); g_bm.erase(it); }
        sceKernelDcacheWritebackRange(px + (size_t)y0 * w, (size_t)(y1 - y0) * w * 4);
        g_rstats.uploadMB += (x1 - x0) * (y1 - y0) * 4 / 1048576.0;
        return;
    }
    BmTex& t = bmGet(uid, w, h);
    if (!t.px) return;
    for (int y = y0; y < y1; y++) {
        uint32_t* d = t.px + (size_t)y * t.stride + x0; const uint32_t* sp = px + (size_t)y * w + x0;
        memcpy(d, sp, (x1 - x0) * 4);
        sceKernelDcacheWritebackRange(d, (x1 - x0) * 4);
    }
    g_rstats.uploadMB += (x1 - x0) * (y1 - y0) * 4 / 1048576.0;
}
void renderAdopt(const SnapPtr& s) {
    for (uint32_t uid : s->freedBitmaps) { auto it = g_bm.find(uid); if (it != g_bm.end()) { bmFree(it->second); g_bm.erase(it); } }
    for (auto& u : s->uploads) {
        BmTex& t = bmGet(u.uid, u.w, u.h);
        if (!t.px) continue;
        int w = u.x1 - u.x0;
        for (int y = u.y0; y < u.y1; y++) {
            uint32_t* d = t.px + (size_t)y * t.stride + u.x0; const uint32_t* sp = u.px.data() + (size_t)(y - u.y0) * w;
            memcpy(d, sp, w * 4);
            sceKernelDcacheWritebackRange(d, w * 4);
        }
        g_rstats.uploadMB += w * (u.y1 - u.y0) * 4 / 1048576.0;
    }
    g_cur = s;
}
void renderFreeRetired() {}
const Snapshot* renderCurrent() { return g_cur.get(); }
static void beginFrame(uint32_t clearColor) {
    if (g_inFrame) return;
    g_inFrame = true; g_frameNo++; g_boundPtr = nullptr;
    sceGuStart(GU_DIRECT, g_list);
    sceGuPixelMask(0);
    sceGuClearColor(clearColor); sceGuClearStencil(0);
    sceGuClear(GU_COLOR_BUFFER_BIT | GU_STENCIL_BUFFER_BIT);
    sceGuPixelMask(0xff000000);   // normal drawing never writes alpha (it is the stencil)
    sceGuEnable(GU_BLEND); blendNormal();
}
void renderFrame(int winW, int winH, double) {
    if (!g_cur) { beginFrame(0); return; }
    beginFrame(0xff000000);
    g_rasterSpentUs = 0;
    // stage background, then the display list
    Mat st = stageToWindowFor(winW, winH);
    {
        uint32_t bg = g_cur->bgColor;
        sceGuDisable(GU_TEXTURE_2D);
        struct PV { uint32_t c; float x, y, z; };
        PV* v = (PV*)sceGuGetMemory(2 * sizeof(PV));
        uint32_t c = 0xff000000u | (bg & 0xff) << 16 | (bg & 0xff00) | ((bg >> 16) & 0xff);
        v[0] = {c, (float)st.tx, (float)st.ty, 0}; v[1] = {c, (float)(st.tx + g_stageW * st.a), (float)(st.ty + g_stageH * st.d), 0};
        sceGuDrawArray(GU_SPRITES, GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, v);
        sceGuEnable(GU_TEXTURE_2D);
    }
    g_stencil = 0;
    // only the stage is shown (as on the other platforms: what lies outside it, black bars)
    { int x0 = (int)st.tx, y0 = (int)st.ty; sceGuScissor(x0, y0, (int)std::ceil(st.tx + g_stageW * st.a) - x0, (int)std::ceil(st.ty + g_stageH * st.d) - y0); }   // (x, y, width, height)
    if (!g_cur->nodes.empty()) drawNode(0, FMat(st), CX(), false);
    sceGuScissor(0, 0, kScrW, kScrH);
    sceGuDisable(GU_STENCIL_TEST);
    if (g_cur->focusOn) {   // controller focus (menuui.cpp): a pink frame around the selected option
        const float* r = g_cur->focusRect; double x0, y0, x1, y1;
        st.apply(r[0], r[1], x0, y0); st.apply(r[2], r[3], x1, y1);
        sceGuDisable(GU_TEXTURE_2D);
        struct PV { uint32_t c; float x, y, z; };
        uint32_t c = g_cur->focusStrong ? 0xff12128cu : 0xff8888c8u;
        float t = 2;
        float rects[4][4] = {{(float)x0 - t, (float)y0 - t, (float)x1 + t, (float)y0}, {(float)x0 - t, (float)y1, (float)x1 + t, (float)y1 + t},
                             {(float)x0 - t, (float)y0, (float)x0, (float)y1}, {(float)x1, (float)y0, (float)x1 + t, (float)y1}};
        for (auto& q : rects) {
            PV* v = (PV*)sceGuGetMemory(2 * sizeof(PV));
            v[0] = {c, q[0], q[1], 0}; v[1] = {c, q[2], q[3], 0};
            sceGuDrawArray(GU_SPRITES, GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, v);
        }
        sceGuEnable(GU_TEXTURE_2D);
    }
}

// ---------------------------------------------------------------- text (diagnostics, chooser, hints)
#include "font5x7.inc"
static uint32_t* g_txt = nullptr; static const int kTxtW = 512, kTxtH = 64;
static void drawText(const std::vector<std::string>& lines, float x, float y, int px, uint32_t ink, uint32_t paper) {
    if (lines.empty()) return;
    if (!g_txt) g_txt = (uint32_t*)memalign(64, kTxtW * kTxtH * 4);
    if (!g_txt) return;
    const Glyph* table[128] = {};
    for (auto& f : kFont) table[(unsigned char)f.c] = &f;
    int cw = 6, lh = 9;
    // each line rendered at 1 px per dot into the text texture, drawn scaled by px
    for (size_t li = 0; li < lines.size(); li++) {
        const std::string& L = lines[li];
        int W = std::min(kTxtW, (int)L.size() * cw + 2), H = lh;
        for (int yy = 0; yy < H; yy++) for (int xx = 0; xx < W; xx++) g_txt[yy * kTxtW + xx] = paper;
        for (size_t ci = 0; ci < L.size() && (int)(ci + 1) * cw < kTxtW; ci++) {
            unsigned char c = (unsigned char)toupper((unsigned char)L[ci]);
            const Glyph* g = c < 128 ? table[c] : nullptr; if (!g) continue;
            for (int r = 0; r < 7; r++) for (int k = 0; k < 5; k++) if (g->r[r][k] == '#') g_txt[(1 + r) * kTxtW + 1 + (int)ci * cw + k] = ink;
        }
        sceKernelDcacheWritebackRange(g_txt, kTxtW * H * 4);
        // the texture is rewritten for the next line: draw this one now and wait for it
        sceGuTexFilter(GU_NEAREST, GU_NEAREST);
        g_boundPtr = nullptr; bindTex(g_txt, kTxtW, 16, kTxtW);
        FMat m; m.a = m.d = (float)px; m.tx = x; m.ty = y + li * lh * px;
        quad(m, 0, 0, W, H, 0, 0, (float)W, (float)H, 0xffffffffu);
        sceGuFinish(); sceGuSync(0, 0); sceGuStart(GU_DIRECT, g_list);
        sceGuTexFilter(GU_LINEAR, GU_LINEAR); g_boundPtr = nullptr;
    }
}
void renderOverlay(const std::vector<std::string>& lines) { beginFrame(0); drawText(lines, 2, 2, 1, 0xff8cffffu, 0xa6000000u); }
void renderHint(const std::string& text) { if (!text.empty()) { beginFrame(0); drawText({text}, kScrW - 6 * (float)text.size() - 8, kScrH - 14, 1, 0xffffffffu, 0xa6000000u); } }
void renderLoadingScreen(int, int, float frac) {
    beginFrame(0xff000000);
    char b[32]; snprintf(b, sizeof b, "LOADING %d%%", (int)(frac * 100));
    drawText({b}, kScrW / 2 - 60, kScrH / 2 - 9, 2, 0xffffffffu, 0xff000000u);
}
void renderChooser(int, int, const std::string& title, const std::vector<std::string>& items, int sel, const std::string& hint,
                   std::vector<std::array<int, 4>>* rects) {
    beginFrame(0xff101010);
    drawText({title}, 24, 24, 2, 0xffffffffu, 0xff101010u);
    for (size_t i = 0; i < items.size(); i++) {
        bool on = (int)i == sel;
        drawText({(on ? "> " : "  ") + items[i]}, 24, 80 + 28 * (float)i, 2, on ? 0xff8888ffu : 0xffc0c0c0u, 0xff101010u);
        if (rects) rects->push_back({24, 80 + 28 * (int)i, kScrW - 48, 24});
    }
    drawText({hint}, 24, kScrH - 30, 1, 0xff909090u, 0xff101010u);
}
void renderTouchSticks(const TouchStick*, int) {}
void renderPauseButton(double, double, double, double) {}
void renderEndFrame() {
    if (!g_inFrame) return;
    sceGuFinish(); sceGuSync(0, 0);
    g_inFrame = false;
}
// shows the frame: waits for the vertical blank (vsyncs = how many refreshes it stays on screen)
void pspPresent(int vsyncs) {
    for (int i = 0; i < std::max(1, vsyncs); i++) sceDisplayWaitVblankStart();
    g_drawBuf = sceGuSwapBuffers();
}
bool renderScreenshot(const char*) { return false; }
void renderPreload(const std::vector<PreKey>&, int, int, const std::function<void(float)>&) {}
size_t renderPreloadPending() { return 0; }
std::vector<PreKey> renderTakeLearned() { return {}; }
void renderMarkKnown(const std::vector<PreKey>&) {}
int renderAtlasPages() { return (int)g_pages.size(); }
void renderMemStats(size_t* pages, size_t* bitmaps) { *pages = g_pages.size() * (size_t)kPage * kPage * 4; size_t n = 0; for (auto& kv : g_bm) if (kv.second.px) n += (size_t)kv.second.stride * kv.second.h * 4; *bitmaps = n; }
void renderSetBudget(int pages) { g_maxPages = std::max(2, pages); }
}  // namespace fl
#endif
