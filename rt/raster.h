#pragma once
#include "display.h"
#include "platform.h"
#include <vector>
#include <functional>
typedef struct plutovg_canvas plutovg_canvas_t;
namespace fl {
// rasterizable content independent of the live object (the renderer works with snapshots)
struct RasterSrc { int kind = 0; const CharDef* def = nullptr; uint16_t ratio = 0; const std::string* text = nullptr; uint32_t argb = 0; };
RasterSrc rasterSrcOf(DisplayObject* d);
bool contentBoundsSrc(const RasterSrc& s, RectF& b);
struct Raster {
    std::vector<uint32_t> px; int w = 0, h = 0; double x0 = 0, y0 = 0, scale = 1;
    // "frame" graphics (a huge shape that is almost all off screen, like the game's 3000x3000 border overlay): only the part
    // around the origin is rasterized; the rest of the shape's bounds (fx0..fy1, local units) is painted with the flat `fill`
    // colour (premultiplied ARGB: the mean of the raster's border)
    bool frame = false; uint32_t fill = 0; double fx0 = 0, fy0 = 0, fx1 = 0, fy1 = 0;
};
bool rasterize(DisplayObject* d, double scale, Raster& out);
void rasterFrameSelfTest();
bool rasterizeSrc(const RasterSrc& s, double scale, Raster& out);
bool contentBounds(DisplayObject* d, RectF& b);
void bitmapDraw(BitmapData* bd, Object* src, const Mat& m, const CX* cx);
ShapeDef* morphShape(const MorphDef* md, uint16_t ratio);
// runs f(0..n-1) spread across the raster threads (and the current thread)
void parallelFor(int n, const std::function<void(int)>& f);
int rasterThreads();
// role of the current thread (decides the number and placement of its helper threads)
void setThreadRole(ThreadRole r);
}
