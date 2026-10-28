// Character definitions read from the SWF (shapes, sprites, buttons, bitmaps, sounds, fonts, text).
#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cstring>

namespace fl {
struct Str;

struct Mat {
    double a = 1, b = 0, c = 0, d = 1, tx = 0, ty = 0;   // in pixels
    Mat operator*(const Mat& o) const {   // this ∘ o  (applies o first, then this)
        return {a * o.a + c * o.b, b * o.a + d * o.b, a * o.c + c * o.d, b * o.c + d * o.d,
                a * o.tx + c * o.ty + tx, b * o.tx + d * o.ty + ty};
    }
    void apply(double x, double y, double& ox, double& oy) const { ox = a * x + c * y + tx; oy = b * x + d * y + ty; }
    bool invert(Mat& out) const {
        double det = a * d - b * c; if (det == 0) return false;
        double id = 1 / det;
        out.a = d * id; out.b = -b * id; out.c = -c * id; out.d = a * id;
        out.tx = -(out.a * tx + out.c * ty); out.ty = -(out.b * tx + out.d * ty);
        return true;
    }
    bool operator==(const Mat& o) const { return a == o.a && b == o.b && c == o.c && d == o.d && tx == o.tx && ty == o.ty; }
};
// Pixel order in memory (SWF bitmaps, BitmapData, rasterized shapes): 0xAARRGGBB, except on the PSP, where it is
// 0xAABBGGRR, the GE's texture format: what is drawn on the CPU is used as a texture without a converted copy.
// Values that scripts see (getPixel, fillRect colours...) are always ARGB: pxFromArgb / pxToArgb at the boundary.
#ifdef __PSP__
constexpr bool kPxABGR = true;
#else
constexpr bool kPxABGR = false;
#endif
inline uint32_t pxSwapRB(uint32_t v) { return (v & 0xff00ff00u) | ((v >> 16) & 0xffu) | ((v & 0xffu) << 16); }
inline uint32_t pxFromArgb(uint32_t v) { return kPxABGR ? pxSwapRB(v) : v; }
inline uint32_t pxToArgb(uint32_t v) { return kPxABGR ? pxSwapRB(v) : v; }
struct CX {   // color transform: mult (1 = 256/256) and add (0..255)
    float rm = 1, gm = 1, bm = 1, am = 1, ra = 0, ga = 0, ba = 0, aa = 0;
    CX operator*(const CX& c) const {   // this ∘ c (c first, then this)
        return {rm * c.rm, gm * c.gm, bm * c.bm, am * c.am, rm * c.ra + ra, gm * c.ga + ga, bm * c.ba + ba, am * c.aa + aa};
    }
    bool identity() const { return rm == 1 && gm == 1 && bm == 1 && am == 1 && ra == 0 && ga == 0 && ba == 0 && aa == 0; }
    bool operator==(const CX& o) const { return !memcmp(this, &o, sizeof(CX)); }
};
struct RectF { double x0 = 0, y0 = 0, x1 = 0, y1 = 0; bool valid() const { return x1 > x0 && y1 > y0; } };

enum CharType : uint8_t { CH_SHAPE, CH_MORPH, CH_SPRITE, CH_BUTTON, CH_BITMAP, CH_SOUND, CH_FONT, CH_TEXT, CH_EDITTEXT };
struct CharDef { CharType type; uint16_t id; virtual ~CharDef() {} };

// ---- Shapes
struct GradStop { float pos; uint32_t argb; };
struct FillStyle {
    uint8_t type = 0;           // 0 solid, 0x10 linear, 0x12 radial, 0x13 focal, 0x40-0x43 bitmap
    uint32_t argb = 0xff000000; // not premultiplied
    Mat m;                      // gradient/bitmap matrix in pixels
    std::vector<GradStop> stops;
    uint8_t spread = 0, interp = 0;
    float focal = 0;
    uint16_t bitmapId = 0;
};
struct LineStyle {
    float width = 1;            // pixels
    uint32_t argb = 0xff000000;
    bool hasFill = false; FillStyle fill;
    uint8_t startCap = 0, endCap = 0, join = 0;   // 0 round, 1 none(butt), 2 square / join: 0 round, 1 bevel, 2 miter
    float miter = 3;
    bool noClose = false;
};
// Path: verbs 0=M 1=L 2=Q, coordinates in pixels
struct Path {
    std::vector<uint8_t> verbs;
    std::vector<float> pts;
    bool empty() const { return verbs.empty(); }
};
struct ShapeLayer {
    std::vector<FillStyle> fills;
    std::vector<LineStyle> lines;
    std::vector<Path> fillPaths;   // per fill style (0-based index)
    std::vector<Path> linePaths;   // per line style
};
struct ShapeDef;
const std::vector<std::vector<float>>& shapeHitPolys(ShapeDef* s);
// the SWF edge records in twips, for exact hit tests (Ruffle shape_utils::shape_hit_test). kind 0 line, 1 curve,
// 2 new style layer; f0/f1 = the edge has fill style 0 / 1; stroke = line width in twips (-1: none)
struct HitEdge { int32_t x0, y0, cx, cy, x1, y1; uint8_t kind; bool f0, f1; int32_t stroke; };
struct ShapeDef : CharDef {
    RectF bounds;
    std::vector<ShapeLayer> layers;
    std::vector<std::vector<float>> hitPolys;   // flattened polygons (for hitTest), per fill; see shapeHitPolys
    bool hitBuilt = false;
    std::vector<HitEdge> hitEdges;
    // lean loading (g_leanSwf, PSP): the records are parsed when first needed (shapeEnsure) and may be dropped
    // again to save memory; raw points at the DefineShape body (after the id) in the movie data
    const uint8_t* raw = nullptr; uint32_t rawLen = 0; uint16_t rawTag = 0; uint32_t rawOff = 0;   // (raw null: rawLen bytes at rawOff in the SWF cache file)
    bool parsed = true, pinned = false; uint32_t lastUse = 0; uint32_t parsedBytes = 0;
};
// lean loading: parses the shape's records if they aren't in memory (no-op otherwise). Call before using
// layers / hitEdges / hitPolys.
void shapeEnsure(ShapeDef* s);
extern bool g_leanSwf;

// ---- Morph: paired edges
struct MorphEdge { uint8_t kind; float x0[2], y0[2], cx[2], cy[2], x1[2], y1[2]; int fill0, fill1, line; bool newPath; };
struct MorphDef : CharDef {
    RectF b0, b1;
    std::vector<FillStyle> fills0, fills1;
    std::vector<LineStyle> lines0, lines1;
    std::vector<MorphEdge> edges;
};

// ---- Bitmaps
struct BitmapDef : CharDef {
    const uint8_t* data = nullptr; size_t size = 0; int tag = 0;
    int w = 0, h = 0;
    std::vector<uint32_t> px;   // premultiplied ARGB, decoded on demand
    bool decoded = false;
    void decode();
};

// ---- Sounds
struct SoundDef : CharDef {
    uint8_t format = 2, rate = 3; bool is16 = true, stereo = false;
    uint32_t samples = 0;
    const uint8_t* data = nullptr; size_t size = 0;   // MP3 data (after SeekSamples)
    int seek = 0;
    uint32_t fileOff = 0;   // lean loading: data stays in the SWF cache file (data == nullptr), see soundBytes
};
// lean loading: the MP3 bytes of a sound / of a timeline stream (read from the SWF cache file into a small cache
// when needed; the caller keeps the pointer while it plays)
struct StreamInfo;
std::shared_ptr<const std::vector<uint8_t>> soundBytes(SoundDef* s);
std::shared_ptr<const std::vector<uint8_t>> streamBytes(StreamInfo* st);
// lean loading: folder for the decompressed SWF (written the first time a game is loaded)
extern std::string g_leanCacheDir;

// ---- Filters
struct Filter { uint8_t type = 1; float blurX = 4, blurY = 4; int passes = 1; uint32_t color = 0; float strength = 1;
    float angle = 0, distance = 0; bool inner = false, knockout = false; };

// ---- Sprites
struct PlaceCmd {
    uint16_t depth = 0, id = 0, ratio = 0, clipDepth = 0;
    bool move = false, hasId = false, hasMatrix = false, hasCx = false, hasRatio = false, hasName = false, hasClipDepth = false, hasFilters = false, hasBlend = false;
    uint8_t blend = 0;
    Mat m; CX cx; Str* name = nullptr;
    std::vector<Filter> filters;
};
struct Act;
using FrameScript = void (*)(void* clip);
struct FrameCmd {
    enum Kind : uint8_t { PLACE, REMOVE, ACTION, SOUND } kind;
    uint16_t depth = 0;            // REMOVE
    int place = -1;                // index into SpriteDef::places
    FrameScript script = nullptr;  // ACTION
    uint16_t soundId = 0; uint8_t soundFlags = 0; uint32_t loops = 1, inPoint = 0, outPoint = 0;  // SOUND
    std::vector<std::array<uint32_t, 3>> envelope;   // SOUND: SoundInfo envelope (position at 44.1 kHz, left, right 0..32768)
};
struct StreamInfo {
    std::vector<uint8_t> mp3;            // all blocks concatenated (lean loading: filled from fileBlocks when played)
    std::vector<std::array<uint32_t, 2>> fileBlocks;   // lean loading: (offset in the SWF cache file, length) per block
    uint32_t leanSize = 0;
    std::vector<uint32_t> frameSample;   // starting sample per frame (0-based index)
    std::vector<int32_t> frameBlock;     // -1 if the frame has no block
    int rate = 44100; bool stereo = false;
};
struct SpriteDef : CharDef {
    int nframes = 1;
    std::vector<std::vector<FrameCmd>> frames;   // [0..nframes)
    std::vector<PlaceCmd> places;
    std::unordered_map<std::string, int> labels;   // label → frame (1-based)
    std::unique_ptr<StreamInfo> stream;
    Str* exportName = nullptr;
    const uint8_t* tlData = nullptr; size_t tlBeg = 0, tlEnd = 0;   // lean loading: timeline not parsed yet (spriteEnsure)
};
void spriteEnsure(SpriteDef* sp);   // parses a lean-loaded sprite's timeline (first instance)

// ---- Buttons
struct ButtonRecord { uint8_t states; uint16_t id, depth; Mat m; CX cx; std::vector<Filter> filters; };
struct ButtonCond { uint16_t cond; FrameScript script; };
struct ButtonDef : CharDef {
    std::vector<ButtonRecord> records;
    std::vector<ButtonCond> conds;
    bool trackAsMenu = false;
};

// ---- Fonts and text
struct FontDef : CharDef {
    std::string name; bool bold = false, italic = false, hasLayout = false; bool font3 = false;
    std::vector<Path> glyphs;          // in EM units (1024)
    std::vector<uint16_t> codes;
    std::vector<float> advances;       // EM
    std::unordered_map<uint32_t, int> codeToIdx;
    float ascent = 0, descent = 0, leading = 0;
};
struct TextGlyph { int idx; float x, y, size; uint32_t argb; uint16_t font; };
struct TextDef : CharDef { RectF bounds; Mat m; std::vector<TextGlyph> glyphs; };
struct EditTextDef : CharDef {
    RectF bounds;
    bool wordWrap = false, multiline = false, password = false, readOnly = true, html = false, useOutlines = false,
         border = false, noSelect = false, autoSize = false, hasText = false;
    uint16_t fontId = 0; float height = 12; uint32_t argb = 0xff000000;
    uint8_t align = 0; float leftMargin = 0, rightMargin = 0, indent = 0, leading = 0;
    std::string variable, initialText;
};

// ---- Movie
struct Movie {
    std::vector<uint8_t> data;
    int version = 8; float frameRate = 30; int width = 550, height = 400; uint32_t bgColor = 0xffffffff;
    std::vector<CharDef*> chars;               // by id
    SpriteDef rootDef;                         // main timeline (id 0)
    std::unordered_map<std::string, uint16_t> exports;
    CharDef* get(uint16_t id) const { return id < chars.size() ? chars[id] : nullptr; }
    bool load(const std::string& path, std::string& err);
};
extern Movie g_movie;

// Table of generated scripts (gen/*.cpp)
struct ScriptEntry { uint16_t sprite; uint16_t frame; uint16_t index; FrameScript fn; };
struct ButtonEntry { uint16_t button; uint16_t index; FrameScript fn; };
// recompiled code for one game (tools/gen.py --ns): one per supported version
struct GameCode {
    const char* id; const char* title;
    const ScriptEntry* frames; int nFrames; const ButtonEntry* buttons; int nButtons;
    int stageW, stageH; float frameRate; void (*initAtoms)();
};
extern const GameCode* g_code;                       // the one for the loaded game (chosen by main at startup)
extern int g_stageW, g_stageH; extern float g_frameRate;
struct KnownGame { const GameCode* code; const char* tag; uint32_t swfLen; uint32_t crc; const char* title; };   // code: null if this binary doesn't carry it (PSP: one game per binary)
extern const KnownGame* g_game;                      // loaded version (tag: "og", "wotl", "eternal")
extern bool g_gameExact;                             // the SWF matches the Steam one byte for byte
const KnownGame* knownGames(int* n);
const KnownGame* knownGameBySwfLen(uint32_t len);
uint32_t peekSwfLength(const std::string& path);    // size of the SWF contained in a .exe/.swf (0 = none)

}  // namespace fl
