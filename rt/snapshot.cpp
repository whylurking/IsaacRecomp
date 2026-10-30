// Building the display list snapshot (logic thread, at the end of each tick).
#include "snapshot.h"
#include <unordered_set>
#include <algorithm>
#include <cstring>

namespace fl {

static std::vector<uint32_t> g_freed;
static std::unordered_set<BitmapData*> g_sent;   // BitmapData whose texture the renderer holds
void bitmapGone(BitmapData* b) {   // destroyed or released (dispose)
    if (b->sentW) g_freed.push_back(b->uid);
    b->sentW = b->sentH = 0;
    g_sent.erase(b);
}

static uint32_t g_serial = 0;
bool bdUseGpu(BitmapData* b);
static void addBitmap(Snapshot& s, BitmapData* b) {
    if (b->snapSerial == s.serial) return;   // already included in this snapshot
    b->snapSerial = s.serial;
    if (s.main) b->snapTick = s.tick;
    if (b->gpu) return;                      // its content is already (and drawn) on the GPU
    if (b->lazy && bdUseGpu(b)) return;      // new single-color layer: moves to the GPU without allocating pixels
    b->ensurePx();
    if (!b->w || !b->h || b->px.size() < (size_t)b->w * b->h) return;
    bool full = b->sentW != b->w || b->sentH != b->h;
    int x0 = 0, y0 = 0, x1 = b->w, y1 = b->h;
    if (!full) {
        if (b->dx1 <= b->dx0 || b->dy1 <= b->dy0) return;   // unchanged
        x0 = b->dx0; y0 = b->dy0; x1 = b->dx1; y1 = b->dy1;
    }
#ifdef __PSP__
    // (PSP: one thread, and the GE is idle between frames: straight into the renderer's texture, without a copy)
    extern void pspBitmapUpload(uint32_t uid, int w, int h, int x0, int y0, int x1, int y1, const uint32_t* px);
    pspBitmapUpload(b->uid, b->w, b->h, x0, y0, x1, y1, b->px.data());
#else
    BitmapUpload up; up.uid = b->uid; up.w = b->w; up.h = b->h; up.x0 = x0; up.y0 = y0; up.x1 = x1; up.y1 = y1;
    int w = x1 - x0;
    up.px.resize((size_t)w * (y1 - y0));
    for (int y = y0; y < y1; y++) memcpy(up.px.data() + (size_t)(y - y0) * w, b->px.data() + (size_t)y * b->w + x0, (size_t)w * 4);
    s.uploads.push_back(std::move(up));
#endif
    b->sentW = b->w; b->sentH = b->h;
    g_sent.insert(b);
    b->dx0 = b->dx1 = b->dy0 = b->dy1 = 0;
}

static void addNode(Snapshot& s, DisplayObject* d, int32_t parent) {
    uint32_t idx = (uint32_t)s.nodes.size();
    s.nodes.emplace_back();
    {
        SnapNode& n = s.nodes.back();
        n.uid = d->uid; n.parent = parent; n.objKind = (uint8_t)d->kind;
        n.visible = d->visible; n.clipDepth = d->clipDepth; n.depth = d->depth;
        n.m = d->m; n.cx = d->cx; n.ratio = d->ratio;
        if (!g_snapOv.empty()) { auto it = g_snapOv.find(d); if (it != g_snapOv.end()) { if (it->second.hide) n.visible = false; if (it->second.hasM) n.m = it->second.m; if (it->second.hasCx) n.cx = it->second.cx * n.cx; if (it->second.alpha >= 0) { n.cx.am *= it->second.alpha; n.cx.aa *= it->second.alpha; } } }
        for (auto& f : d->filters) if (f.type == 1 && (f.blurX > 0.5f || f.blurY > 0.5f)) { n.blur = true; n.blurX = f.blurX; n.blurY = f.blurY; n.blurPasses = f.passes; }
        switch (d->kind) {
            case OK_CLIP: case OK_BUTTON: n.kind = SN_CONTAINER; break;
            case OK_BITMAP: {
                n.kind = SN_BITMAP;
                BitmapObj* bo = (BitmapObj*)d;
                if (bo->bd) { BitmapData* b = bo->bd; addBitmap(s, b); n.bmUid = b->uid; n.bmW = b->w; n.bmH = b->h; n.smoothing = bo->smoothing; if (kPxABGR && b->px.size() >= (size_t)b->w * b->h) n.bmPx = b->px.data(); }
                break;
            }
            case OK_SHAPE: case OK_MORPH: case OK_TEXT: case OK_EDITTEXT: {
                n.kind = SN_CONTENT;
                RasterSrc src = rasterSrcOf(d);
                n.def = src.def; n.argb = src.argb;
                if (d->kind == OK_EDITTEXT) { EditTextObj* e = (EditTextObj*)d; n.textVersion = e->version; n.text = (int32_t)s.texts.size(); s.texts.push_back(e->text); }
                n.hasBounds = contentBoundsSrc(src, n.bounds);
                break;
            }
            default: n.kind = SN_CONTAINER; break;
        }
    }
    const SnapOv* ov = nullptr;
    if (!g_snapOv.empty()) { auto it = g_snapOv.find(d); if (it != g_snapOv.end()) ov = &it->second; }
    if (!ov || !ov->hideKids) {
        if (d->kind == OK_CLIP) for (DisplayObject* k : ((MovieClip*)d)->kids) addNode(s, k, (int32_t)idx);
        else if (d->kind == OK_BUTTON) for (DisplayObject* k : ((ButtonObj*)d)->kids) addNode(s, k, (int32_t)idx);
    }
    if (ov && ov->extra) { DisplayObject* x = ov->extra; addNode(s, x, (int32_t)idx); }
    s.nodes[idx].end = (uint32_t)s.nodes.size();
}

static std::vector<GpuOp> g_gpuOps;
bool g_gpuBitmaps = false;

bool bdUseGpu(BitmapData* b) {
    if (b->gpu) return true;
    if (!g_gpuBitmaps || !b->displayed || b->cpuRead || !b->w || !b->h || b->disposed) return false;
    // moves to the GPU with whatever content it has now (if still a single color, a fill is enough)
    GpuOp op; op.uid = b->uid; op.w = b->w; op.h = b->h;
    if (b->lazy) { op.kind = GpuOp::FILL; op.x1 = b->w; op.y1 = b->h; op.color = b->lazyFill; }
    else { op.kind = GpuOp::UPLOAD; op.px.swap(b->px); b->lazy = true; b->lazyFill = 0; }   // (the CPU no longer needs them)
    g_gpuOps.push_back(std::move(op));
    b->gpu = true; b->sentW = b->w; b->sentH = b->h; b->dx0 = b->dx1 = b->dy0 = b->dy1 = 0;
    g_sent.insert(b);
    return true;
}
void bdNeedCpu(BitmapData* b) {
    if (!b->gpu) { b->cpuRead = true; return; }
    static int warned = 0;
    if (warned++ < 5) logf("warning: reading on the CPU the pixels of a layer drawn on the GPU (%dx%d); moving it back to the CPU\n", b->w, b->h);
    b->gpu = false; b->cpuRead = true;
}
void gpuFill(BitmapData* b, int x0, int y0, int x1, int y1, uint32_t c) {
    if (x1 <= x0 || y1 <= y0) return;
    GpuOp op; op.kind = GpuOp::FILL; op.uid = b->uid; op.w = b->w; op.h = b->h; op.x0 = x0; op.y0 = y0; op.x1 = x1; op.y1 = y1; op.color = c;
    g_gpuOps.push_back(std::move(op));
}
void gpuCopy(BitmapData* d, BitmapData* s, int sx, int sy, int w, int h, int dx, int dy, bool merge) {
    // clip to both bitmaps
    if (sx < 0) { dx -= sx; w += sx; sx = 0; } if (sy < 0) { dy -= sy; h += sy; sy = 0; }
    if (dx < 0) { sx -= dx; w += dx; dx = 0; } if (dy < 0) { sy -= dy; h += dy; dy = 0; }
    w = std::min({w, s->w - sx, d->w - dx}); h = std::min({h, s->h - sy, d->h - dy});
    if (w <= 0 || h <= 0) return;
    GpuOp op; op.kind = GpuOp::COPY; op.uid = d->uid; op.w = d->w; op.h = d->h; op.x0 = dx; op.y0 = dy; op.x1 = dx + w; op.y1 = dy + h;
    op.merge = merge && d->transparent;
    op.px.resize((size_t)w * h);
    for (int y = 0; y < h; y++) {
        const uint32_t* src = s->px.data() + (size_t)(sy + y) * s->w + sx;
        uint32_t* dst = op.px.data() + (size_t)y * w;
        if (d->transparent) memcpy(dst, src, (size_t)w * 4); else for (int x = 0; x < w; x++) dst[x] = src[x] | 0xff000000u;
    }
    g_gpuOps.push_back(std::move(op));
}
void gpuDraw(BitmapData* b, Object* src, const Mat& m, const CX* cx) {
    auto sc = std::make_shared<Snapshot>();
    sc->tick = g_player.tick; sc->serial = ++g_serial; sc->main = false;
    if (src->kind == OK_BITMAPDATA) {   // BitmapData source: a single bitmap node
        BitmapData* sb = (BitmapData*)src;
        if (sb == b) return;
        addBitmap(*sc, sb);
        SnapNode n; n.uid = 0xffffffffu; n.end = 1; n.kind = SN_BITMAP; n.objKind = OK_BITMAP; n.bmUid = sb->uid; n.bmW = sb->w; n.bmH = sb->h; n.smoothing = true;
        sc->nodes.push_back(n);
    } else if (src->isDisplay()) {
        static const bool opLog = getenv("ISAAC_GPUOPLOG") != nullptr;   // (testing)
        if (opLog) { DisplayObject* d = (DisplayObject*)src; logf("[gpudraw] tick %u into %ux%u: %s (char %u, kind %d, %zu kids)\n", g_player.tick, b->w, b->h, d->name ? d->name->s.c_str() : "-", d->charId, (int)d->kind, d->kind == OK_CLIP ? ((MovieClip*)d)->kids.size() : (size_t)0); }
        addNode(*sc, (DisplayObject*)src, -1);
        if (sc->nodes.empty()) return;
        // BitmapData.draw does not apply the source's own matrix, color or visibility
        sc->nodes[0].m = Mat(); sc->nodes[0].cx = CX(); sc->nodes[0].visible = true;
    } else return;
    GpuOp op; op.kind = GpuOp::DRAW; op.uid = b->uid; op.w = b->w; op.h = b->h; op.m = m; if (cx) op.cx = *cx; op.scene = std::move(sc);
    g_gpuOps.push_back(std::move(op));
}

ViewReq g_viewReq;
std::unordered_map<DisplayObject*, SnapOv> g_snapOv;
std::atomic<float> g_windowAspect{0.f};
SnapPtr buildSnapshot() {
    SnapPtr s = std::make_shared<Snapshot>();
    if (g_viewReq.set) { s->viewSet = true; s->viewClip = g_viewReq.clip; for (int i = 0; i < 4; i++) s->view[i] = g_viewReq.r[i]; }
    if (g_viewReq.set && g_viewReq.cut) { s->viewCut = true; for (int i = 0; i < 4; i++) s->cut[i] = g_viewReq.c[i]; }
    s->tick = g_player.tick; s->serial = ++g_serial; s->main = true;
    s->gpuOps.swap(g_gpuOps);
    s->bgColor = g_movie.bgColor;
    static size_t lastN = 256; s->nodes.reserve(lastN + lastN / 4);
    if (vm.root) addNode(*s, vm.root, -1);
    lastN = s->nodes.size();
    // bitmaps no longer on screen (e.g. the previous room's layers) return their texture to the
    // renderer right away, so it can be reused in the next room without creating new textures; if they
    // are shown again, they are sent in full once more
    for (auto it = g_sent.begin(); it != g_sent.end();) {
        BitmapData* b = *it;
        if (!b->gpu && b->snapTick + 2 <= s->tick) {   // (a GPU layer keeps its content until destroyed)
            g_freed.push_back(b->uid); b->sentW = b->sentH = 0; it = g_sent.erase(it);
        }
        else ++it;
    }
    s->freedBitmaps.swap(g_freed);
    return s;
}

}  // namespace fl
