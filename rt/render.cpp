// OpenGL 3.3 / OpenGL ES 3.0 renderer over display-list snapshots: interpolation at the display refresh
// rate, atlas of rasterized textures, asynchronous uploads (PBO), stencil masks and blur.
#include "render.h"
#include <array>
#include "raster.h"
#include "snapshot.h"
#include "platform.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <deque>
#include <mutex>
#include <functional>

namespace fl {

// ============================================================ GL function loading
// typedefs for GL 1.0/1.1 functions (glext.h only defines them for 1.2+)
typedef void (APIENTRY *PFNGLGENTEXTURESPROC)(GLsizei, GLuint*);
typedef void (APIENTRY *PFNGLDELETETEXTURESPROC)(GLsizei, const GLuint*);
typedef void (APIENTRY *PFNGLBINDTEXTUREPROC)(GLenum, GLuint);
typedef void (APIENTRY *PFNGLTEXIMAGE2DPROC)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*);
typedef void (APIENTRY *PFNGLTEXSUBIMAGE2DPROC)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void*);
typedef void (APIENTRY *PFNGLTEXPARAMETERIPROC)(GLenum, GLenum, GLint);
typedef void (APIENTRY *PFNGLPIXELSTOREIPROC)(GLenum, GLint);
typedef const GLubyte* (APIENTRY *PFNGLGETSTRINGPROC_)(GLenum);
typedef void (APIENTRY *PFNGLDRAWARRAYSPROC)(GLenum, GLint, GLsizei);
typedef void (APIENTRY *PFNGLENABLEPROC)(GLenum);
typedef void (APIENTRY *PFNGLDISABLEPROC)(GLenum);
typedef void (APIENTRY *PFNGLBLENDFUNCPROC)(GLenum, GLenum);
typedef void (APIENTRY *PFNGLVIEWPORTPROC)(GLint, GLint, GLsizei, GLsizei);
typedef void (APIENTRY *PFNGLCLEARCOLORPROC)(GLfloat, GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY *PFNGLCLEARPROC)(GLbitfield);
typedef void (APIENTRY *PFNGLSCISSORPROC)(GLint, GLint, GLsizei, GLsizei);
typedef void (APIENTRY *PFNGLSTENCILFUNCPROC)(GLenum, GLint, GLuint);
typedef void (APIENTRY *PFNGLSTENCILOPPROC)(GLenum, GLenum, GLenum);
typedef void (APIENTRY *PFNGLSTENCILMASKPROC)(GLuint);
typedef void (APIENTRY *PFNGLCOLORMASKPROC)(GLboolean, GLboolean, GLboolean, GLboolean);
typedef void (APIENTRY *PFNGLCLEARSTENCILPROC)(GLint);
typedef GLboolean (APIENTRY *PFNGLISENABLEDPROC)(GLenum);
typedef GLenum (APIENTRY *PFNGLGETERRORPROC)(void);
typedef void (APIENTRY *PFNGLFINISHPROC)(void);
typedef void (APIENTRY *PFNGLFLUSHPROC_)(void);
typedef void (APIENTRY *PFNGLREADPIXELSPROC)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void*);
#define GLFUNCS(X) \
    X(PFNGLGENTEXTURESPROC, GenTextures) X(PFNGLDELETETEXTURESPROC, DeleteTextures) X(PFNGLBINDTEXTUREPROC, BindTexture) \
    X(PFNGLTEXIMAGE2DPROC, TexImage2D) X(PFNGLTEXSUBIMAGE2DPROC, TexSubImage2D) X(PFNGLTEXPARAMETERIPROC, TexParameteri) \
    X(PFNGLPIXELSTOREIPROC, PixelStorei) X(PFNGLGENERATEMIPMAPPROC, GenerateMipmap) X(PFNGLACTIVETEXTUREPROC, ActiveTexture) \
    X(PFNGLCREATESHADERPROC, CreateShader) X(PFNGLSHADERSOURCEPROC, ShaderSource) X(PFNGLCOMPILESHADERPROC, CompileShader) \
    X(PFNGLGETSHADERIVPROC, GetShaderiv) X(PFNGLGETSHADERINFOLOGPROC, GetShaderInfoLog) X(PFNGLCREATEPROGRAMPROC, CreateProgram) \
    X(PFNGLATTACHSHADERPROC, AttachShader) X(PFNGLLINKPROGRAMPROC, LinkProgram) X(PFNGLGETPROGRAMIVPROC, GetProgramiv) \
    X(PFNGLGETPROGRAMINFOLOGPROC, GetProgramInfoLog) X(PFNGLUSEPROGRAMPROC, UseProgram) X(PFNGLGETUNIFORMLOCATIONPROC, GetUniformLocation) \
    X(PFNGLUNIFORM1IPROC, Uniform1i) X(PFNGLUNIFORM1FPROC, Uniform1f) X(PFNGLUNIFORM2FPROC, Uniform2f) X(PFNGLUNIFORM4FPROC, Uniform4f) \
    X(PFNGLBINDATTRIBLOCATIONPROC, BindAttribLocation) \
    X(PFNGLGENBUFFERSPROC, GenBuffers) X(PFNGLBINDBUFFERPROC, BindBuffer) X(PFNGLBUFFERDATAPROC, BufferData) \
    X(PFNGLBUFFERSUBDATAPROC, BufferSubData) X(PFNGLGENVERTEXARRAYSPROC, GenVertexArrays) X(PFNGLBINDVERTEXARRAYPROC, BindVertexArray) \
    X(PFNGLENABLEVERTEXATTRIBARRAYPROC, EnableVertexAttribArray) X(PFNGLVERTEXATTRIBPOINTERPROC, VertexAttribPointer) \
    X(PFNGLDRAWARRAYSPROC, DrawArrays) X(PFNGLENABLEPROC, Enable) X(PFNGLDISABLEPROC, Disable) X(PFNGLBLENDFUNCPROC, BlendFunc) \
    X(PFNGLVIEWPORTPROC, Viewport) X(PFNGLCLEARCOLORPROC, ClearColor) X(PFNGLCLEARPROC, Clear) X(PFNGLSCISSORPROC, Scissor) \
    X(PFNGLSTENCILFUNCPROC, StencilFunc) X(PFNGLSTENCILOPPROC, StencilOp) X(PFNGLSTENCILMASKPROC, StencilMask) X(PFNGLCOLORMASKPROC, ColorMask) \
    X(PFNGLCLEARSTENCILPROC, ClearStencil) X(PFNGLGENFRAMEBUFFERSPROC, GenFramebuffers) X(PFNGLBINDFRAMEBUFFERPROC, BindFramebuffer) \
    X(PFNGLFRAMEBUFFERTEXTURE2DPROC, FramebufferTexture2D) X(PFNGLCHECKFRAMEBUFFERSTATUSPROC, CheckFramebufferStatus) \
    X(PFNGLGENRENDERBUFFERSPROC, GenRenderbuffers) X(PFNGLBINDRENDERBUFFERPROC, BindRenderbuffer) X(PFNGLRENDERBUFFERSTORAGEPROC, RenderbufferStorage) \
    X(PFNGLFRAMEBUFFERRENDERBUFFERPROC, FramebufferRenderbuffer) X(PFNGLUNIFORM3FPROC, Uniform3f) X(PFNGLMAPBUFFERRANGEPROC, MapBufferRange) X(PFNGLUNMAPBUFFERPROC, UnmapBuffer) X(PFNGLISENABLEDPROC, IsEnabled) \
    X(PFNGLDELETEFRAMEBUFFERSPROC, DeleteFramebuffers) X(PFNGLDELETERENDERBUFFERSPROC, DeleteRenderbuffers) X(PFNGLGETERRORPROC, GetError) X(PFNGLREADPIXELSPROC, ReadPixels) X(PFNGLFINISHPROC, Finish) X(PFNGLFLUSHPROC_, Flush) \
    X(PFNGLFENCESYNCPROC, FenceSync) X(PFNGLCLIENTWAITSYNCPROC, ClientWaitSync) X(PFNGLDELETESYNCPROC, DeleteSync) X(PFNGLGETSTRINGPROC_, GetString)
#define DECL(T, N) static T p_gl##N;
GLFUNCS(DECL)
#undef DECL
#define glGenTextures p_glGenTextures
#define glDeleteTextures p_glDeleteTextures
#define glBindTexture p_glBindTexture
#define glTexImage2D p_glTexImage2D
#define glTexSubImage2D p_glTexSubImage2D
#define glTexParameteri p_glTexParameteri
#define glPixelStorei p_glPixelStorei
#define glGenerateMipmap p_glGenerateMipmap
#define glActiveTexture p_glActiveTexture
#define glCreateShader p_glCreateShader
#define glShaderSource p_glShaderSource
#define glCompileShader p_glCompileShader
#define glGetShaderiv p_glGetShaderiv
#define glGetShaderInfoLog p_glGetShaderInfoLog
#define glCreateProgram p_glCreateProgram
#define glAttachShader p_glAttachShader
#define glLinkProgram p_glLinkProgram
#define glGetProgramiv p_glGetProgramiv
#define glGetProgramInfoLog p_glGetProgramInfoLog
#define glUseProgram p_glUseProgram
#define glGetUniformLocation p_glGetUniformLocation
#define glUniform1i p_glUniform1i
#define glUniform1f p_glUniform1f
#define glUniform2f p_glUniform2f
#define glUniform4f p_glUniform4f
#define glBindAttribLocation p_glBindAttribLocation
#define glGenBuffers p_glGenBuffers
#define glBindBuffer p_glBindBuffer
#define glBufferData p_glBufferData
#define glBufferSubData p_glBufferSubData
#define glGenVertexArrays p_glGenVertexArrays
#define glBindVertexArray p_glBindVertexArray
#define glEnableVertexAttribArray p_glEnableVertexAttribArray
#define glVertexAttribPointer p_glVertexAttribPointer
#define glDrawArrays p_glDrawArrays
#define glEnable p_glEnable
#define glDisable p_glDisable
#define glBlendFunc p_glBlendFunc
#define glViewport p_glViewport
#define glClearColor p_glClearColor
#define glClear p_glClear
#define glScissor p_glScissor
#define glStencilFunc p_glStencilFunc
#define glStencilOp p_glStencilOp
#define glStencilMask p_glStencilMask
#define glColorMask p_glColorMask
#define glClearStencil p_glClearStencil
#define glGenFramebuffers p_glGenFramebuffers
#define glBindFramebuffer p_glBindFramebuffer
#define glFramebufferTexture2D p_glFramebufferTexture2D
#define glCheckFramebufferStatus p_glCheckFramebufferStatus
#define glGenRenderbuffers p_glGenRenderbuffers
#define glBindRenderbuffer p_glBindRenderbuffer
#define glRenderbufferStorage p_glRenderbufferStorage
#define glFramebufferRenderbuffer p_glFramebufferRenderbuffer
#define glUniform3f p_glUniform3f
#define glMapBufferRange p_glMapBufferRange
#define glUnmapBuffer p_glUnmapBuffer
#define glIsEnabled p_glIsEnabled
#define glDeleteFramebuffers p_glDeleteFramebuffers
#define glDeleteRenderbuffers p_glDeleteRenderbuffers
#define glGetError p_glGetError
#define glReadPixels p_glReadPixels
#define glFinish p_glFinish
#define glFlush p_glFlush
#define glFenceSync p_glFenceSync
#define glClientWaitSync p_glClientWaitSync
#define glDeleteSync p_glDeleteSync
#define glGetString p_glGetString
static bool loadGL() {
    bool ok = true;
#define LOAD(T, N) p_gl##N = (T)SDL_GL_GetProcAddress("gl" #N); if (!p_gl##N) { logf("GL: missing gl" #N "\n"); ok = false; }
    GLFUNCS(LOAD)
#undef LOAD
    return ok;
}


// ============================================================ state
RenderConfig g_renderCfg;
RenderStats g_rstats;
Mat g_stageToWindow;
static GLuint g_prog, g_progBlur, g_vao;
static GLint u_tex, u_alphaTest, u_view, u_flip, ub_tex, ub_dir, ub_view, ub_flip, u_colOn, u_colR, u_colG, u_colB;
static int g_winW = 0, g_winH = 0;
static int g_stencilLevel = 0;
static uint32_t g_frameNo = 0;

struct Vtx { float x, y, u, v, mr, mg, mb, ma, ar, ag, ab, aa; };
static std::vector<Vtx> g_batch;
static GLuint g_batchTex = 0;
// ---- streaming ring buffers (vertices and pixel uploads)
// Each batch is written to a region of the ring the GPU no longer reads (per-frame fences) using an
// unsynchronized mapping. Previously the whole buffer was "orphaned" with glBufferData every frame: several
// drivers (Switch's, llvmpipe) then allocate new memory each time, which cost several ms
// (or hundreds) per frame.
struct StreamRing {
    GLenum target = 0; GLuint buf = 0; size_t size = 0, off = 0, align = 256;
    struct Seg { GLsync fence; std::vector<std::pair<size_t, size_t>> r; };
    std::deque<Seg> sent;                               // frames already submitted, in order
    std::vector<std::pair<size_t, size_t>> cur;         // regions used in this frame
    void init(GLenum t, size_t bytes, size_t al) {
        target = t; size = bytes / al * al; align = al;
        glGenBuffers(1, &buf); glBindBuffer(t, buf); glBufferData(t, size, nullptr, GL_STREAM_DRAW);
    }
    static bool overlap(size_t a, size_t b, size_t c, size_t d) { return a < d && c < b; }
    void waitUpTo(size_t idx) {   // fences signal in order: waiting on idx also frees the earlier ones
        glClientWaitSync(sent[idx].fence, GL_SYNC_FLUSH_COMMANDS_BIT, 2000000000ull);
        for (size_t i = 0; i <= idx; i++) { glDeleteSync(sent.front().fence); sent.pop_front(); }
    }
    void syncAll() { glFinish(); for (auto& sg : sent) glDeleteSync(sg.fence); sent.clear(); cur.clear(); }
    // reserves `bytes`; only waits if the GPU might still be reading that region (almost never: the ring holds many frames)
    size_t alloc(size_t bytes) {
        bytes = (bytes + align - 1) / align * align;
        if (bytes > size / 2) return SIZE_MAX;
        if (off + bytes > size) off = 0;
        size_t a = off, b = off + bytes;
        for (auto& r : cur) if (overlap(a, b, r.first, r.second)) { syncAll(); break; }
        for (size_t i = sent.size(); i-- > 0;) {
            bool hit = false; for (auto& r : sent[i].r) if (overlap(a, b, r.first, r.second)) { hit = true; break; }
            if (hit) { waitUpTo(i); break; }
        }
        off = b;
        if (!cur.empty() && cur.back().second == a) cur.back().second = b; else cur.push_back({a, b});
        return a;
    }
    void endFrame() {
        if (!cur.empty()) { sent.push_back({glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0), cur}); cur.clear(); }
        // discard already-signaled fences (without waiting)
        while (!sent.empty() && (sent.size() > 48 || glClientWaitSync(sent.front().fence, 0, 0) != GL_TIMEOUT_EXPIRED)) {
            if (sent.size() > 48) glClientWaitSync(sent.front().fence, GL_SYNC_FLUSH_COMMANDS_BIT, 2000000000ull);
            glDeleteSync(sent.front().fence); sent.pop_front();
        }
    }
};
static StreamRing g_vboRing, g_pboRing;
static const size_t kVboBytes = 8u << 20;
static bool g_frameOpen = false;

static const char* kVS = R"(#version 330 core
in vec2 aPos; in vec2 aUV; in vec4 aMul; in vec4 aAdd;
uniform vec2 uView; uniform float uFlip;   // uFlip = -1 when drawing into a BitmapData texture (row 0 = top)
out vec2 vUV; out vec4 vMul; out vec4 vAdd;
void main() { vUV = aUV; vMul = aMul; vAdd = aAdd; gl_Position = vec4(aPos.x / uView.x * 2.0 - 1.0, uFlip * (1.0 - aPos.y / uView.y * 2.0), 0.0, 1.0); }
)";
static const char* kFS = R"(#version 330 core
in vec2 vUV; in vec4 vMul; in vec4 vAdd;
uniform sampler2D uTex; uniform float uAlphaTest;
uniform float uColOn; uniform vec3 uColR, uColG, uColB;   // RainbowVision: a colour matrix (rows), on the window only
out vec4 frag;
vec4 shade();
void main() {
    vec4 c = shade();
    if (uColOn > 0.5 && uAlphaTest <= 0.0) c.rgb = clamp(vec3(dot(uColR, c.rgb), dot(uColG, c.rgb), dot(uColB, c.rgb)), 0.0, c.a);   // (premultiplied: linear, so the same)
    frag = c;
}
vec4 shade() {
    vec4 c = texture(uTex, vUV);
    if (uAlphaTest > 0.0) { if (c.a < 0.5) discard; return vec4(1.0); }
    // (premultiplied: a transparent texel leaves the destination unchanged, no need to discard it)
    if (vAdd == vec4(0.0) && vMul.r <= 1.0 && vMul.g <= 1.0 && vMul.b <= 1.0)   // common case: no additive term
        return vec4(c.rgb * vMul.rgb, c.a) * clamp(vMul.a, 0.0, 1.0);
    vec3 rgb = c.rgb / max(c.a, 1e-6);
    rgb = clamp(rgb * vMul.rgb + vAdd.rgb, 0.0, 1.0);
    float aadd = vAdd.a, inkA = 0.0;
    if (aadd > 9.0) {
        // option selected with a controller: its dark ink turns one fixed colour (pastel pink; deep red for the main
        // menu's Start), the paper stays as it is. aadd = 10 (pink) or 20 (red) + brightness of everything above the
        // option. Some labels are written in translucent black: their ink is made opaque so every option looks the same
        bool strong = aadd > 19.0;
        float fade = aadd - (strong ? 20.0 : 10.0); aadd = 0.0;
        float l = dot(rgb, vec3(0.299, 0.587, 0.114)) / max(fade, 0.02);
        inkA = 1.0 - smoothstep(0.66, 0.85, l);
        rgb = mix(rgb, (strong ? vec3(0.55, 0.07, 0.07) : vec3(0.78, 0.53, 0.53)) * fade, inkA);
    }
    float a = clamp(c.a * vMul.a + aadd * c.a, 0.0, 1.0);
    a = mix(a, min(1.0, a * 3.0), inkA);
    return vec4(rgb * a, a);
}
)";
static const char* kBlurFS = R"(#version 330 core
in vec2 vUV; in vec4 vMul; in vec4 vAdd;
uniform sampler2D uTex; uniform vec3 uDir;   // (dx, dy) in uv per texel, z = radius in texels
out vec4 frag;
void main() {
    int r = int(uDir.z);
    vec4 acc = vec4(0.0);
    for (int i = -r; i <= r; i++) acc += texture(uTex, vUV + uDir.xy * float(i));
    frag = acc / float(2 * r + 1);
}
)";

// OpenGL ES 3.0 (Android): same shaders with an ES header instead of "#version 330 core"
bool g_glES = false;
static GLuint compile(GLenum type, const char* src0) {
    std::string tmp = src0;
    if (g_glES && tmp.compare(0, 17, "#version 330 core") == 0) tmp = "#version 300 es\nprecision highp float;\nprecision highp int;" + tmp.substr(17);
    const char* src = tmp.c_str();
    GLuint s = glCreateShader(type); glShaderSource(s, 1, &src, nullptr); glCompileShader(s);
    GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) { char log[2048]; glGetShaderInfoLog(s, sizeof log, nullptr, log); logf("shader: %s\n", log); }
    return s;
}
static GLuint link(const char* vs, const char* fs) {
    GLuint p = glCreateProgram();
    glAttachShader(p, compile(GL_VERTEX_SHADER, vs)); glAttachShader(p, compile(GL_FRAGMENT_SHADER, fs));
    glBindAttribLocation(p, 0, "aPos"); glBindAttribLocation(p, 1, "aUV"); glBindAttribLocation(p, 2, "aMul"); glBindAttribLocation(p, 3, "aAdd");
    glLinkProgram(p);
    GLint ok; glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) { char log[2048]; glGetProgramInfoLog(p, sizeof log, nullptr, log); logf("link: %s\n", log); }
    return p;
}

// ============================================================ textures and uploads
static void g_texSmoothSet(GLuint t);
static GLuint newTexture(int w, int h) {
    GLuint t; glGenTextures(1, &t); glBindTexture(GL_TEXTURE_2D, t);
    // On Switch, unsized internal format: mesa then picks the same byte order as the data (BGRA) and
    // the upload is a direct copy, with no per-pixel conversion
    if (g_glES) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);   // (ES: no BGRA)
    else glTexImage2D(GL_TEXTURE_2D, 0, kIsSwitch ? GL_RGBA : GL_RGBA8, w, h, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    g_rstats.texCreates++;
    g_texSmoothSet(t);
    return t;
}
// OpenGL ES has no BGRA upload format: the pixels (0xAARRGGBB words, B G R A in memory) are converted to
// R G B A bytes while copying them (into the PBO or a scratch buffer). Same cost as the plain copy.
static inline void swizzleRow(uint32_t* d, const uint32_t* s, int n) {
    for (int i = 0; i < n; i++) { uint32_t v = s[i]; d[i] = (v & 0xff00ff00u) | ((v >> 16) & 0xffu) | ((v & 0xffu) << 16); }
}
// copies a rectangle of pixels (premultiplied BGRA) into an existing texture
static void texUploadImpl(GLuint tex, int x, int y, int w, int h, const uint32_t* src, int stride);
static void texUpload(GLuint tex, int x, int y, int w, int h, const uint32_t* src, int stride) {
    Uint64 t0 = SDL_GetPerformanceCounter();
    texUploadImpl(tex, x, y, w, h, src, stride);
    g_rstats.uploadMs += (SDL_GetPerformanceCounter() - t0) * 1000.0 / SDL_GetPerformanceFrequency();
}
static void texUploadImpl(GLuint tex, int x, int y, int w, int h, const uint32_t* src, int stride) {
    if (w <= 0 || h <= 0) return;
    const GLenum fmt = g_glES ? GL_RGBA : GL_BGRA, typ = g_glES ? GL_UNSIGNED_BYTE : GL_UNSIGNED_INT_8_8_8_8_REV;
    size_t bytes = (size_t)w * h * 4;
    glBindTexture(GL_TEXTURE_2D, tex);
    g_rstats.uploadMB += bytes / 1048576.0;
    // PBO: on PC the GPU does the copy to the texture without stalls. Not on Switch: the driver reads the PBO from
    // the CPU in uncached memory (~40 MB/s; 20 MB on a room change cost half a second); from regular
    // memory it's much faster.
    size_t off = SIZE_MAX;
    // (Android too: uploads through the PBO stalled the render thread, up to 100 ms per second for 0.2 MB)
    if (!kIsSwitch && !kIsAndroid) { glBindBuffer(GL_PIXEL_UNPACK_BUFFER, g_pboRing.buf); off = g_pboRing.alloc(bytes); }
    if (off != SIZE_MAX) {
        void* dst = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, off, bytes, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
        if (dst) {
            if (g_glES) for (int r = 0; r < h; r++) swizzleRow((uint32_t*)dst + (size_t)r * w, src + (size_t)r * stride, w);
            else if (stride == w) memcpy(dst, src, bytes);
            else for (int r = 0; r < h; r++) memcpy((char*)dst + (size_t)r * w * 4, src + (size_t)r * stride, (size_t)w * 4);
            glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
            glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, fmt, typ, (const void*)(uintptr_t)off);
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
            return;
        }
    }
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    if (g_glES) {
        static std::vector<uint32_t> tmp; tmp.resize((size_t)w * h);
        for (int r = 0; r < h; r++) swizzleRow(tmp.data() + (size_t)r * w, src + (size_t)r * stride, w);
        glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, fmt, typ, tmp.data());
        return;
    }
    glPixelStorei(GL_UNPACK_ROW_LENGTH, stride);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, fmt, typ, src);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}

// ============================================================ atlas of rasterized shapes
// Shapes are rasterized (CPU, PlutoVG) at the scale they're displayed at, quantized to quarter-octaves, and
// placed in 2048x2048 pages created up front. Creating textures mid-game stalled
// rendering on some drivers (~30 ms each time a new graphic appeared).
static const int kPage = 2048;
struct TexKey {
    const void* def; uint32_t variant; int16_t bucket;
    bool operator==(const TexKey& o) const { return def == o.def && variant == o.variant && bucket == o.bucket; }
};
struct TexKeyHash { size_t operator()(const TexKey& k) const { return std::hash<const void*>()(k.def) ^ (k.variant * 0x9e3779b1u) ^ ((size_t)k.bucket << 20); } };
struct TexEntry {
    int page = -1; GLuint tex = 0; int px = 0, py = 0, texW = 1, texH = 1;
    int w = 0, h = 0; double x0 = 0, y0 = 0, scale = 1; uint32_t lastUse = 0; bool empty = false;
};
struct Shelf { int y, h, x; };
struct Page { GLuint tex = 0; std::vector<Shelf> shelves; int nextY = 0; uint32_t lastUse = 0; std::vector<TexKey> keys; };
static std::unordered_map<TexKey, TexEntry, TexKeyHash> g_cache;
static std::vector<Page> g_pages;
static size_t g_bigBytes = 0;
static int g_rasterBudget = 0;

static bool shelfAlloc(Page& p, int w, int h, int& ox, int& oy) {
    Shelf* best = nullptr;
    for (auto& s : p.shelves) if (s.h >= h && s.h <= h + h / 2 + 8 && s.x + w <= kPage && (!best || s.h < best->h)) best = &s;
    if (best) { ox = best->x; oy = best->y; best->x += w; return true; }
    if (p.nextY + h <= kPage) { p.shelves.push_back({p.nextY, h, w}); ox = 0; oy = p.nextY; p.nextY += h; return true; }
    return false;
}
int renderAtlasPages() { return (int)g_pages.size(); }
static void addPage() { Page p; p.tex = newTexture(kPage, kPage); g_pages.push_back(p); }
static bool atlasPlace(int w, int h, int& page, int& ox, int& oy) {
    for (size_t i = 0; i < g_pages.size(); i++) if (shelfAlloc(g_pages[i], w, h, ox, oy)) { page = (int)i; return true; }
    size_t maxPages = (size_t)std::max(4, g_renderCfg.cacheMB / 16);
    if (g_pages.size() < maxPages) { addPage(); page = (int)g_pages.size() - 1; return shelfAlloc(g_pages.back(), w, h, ox, oy); }
    // evict the least recently used page (one not in use this frame)
    int lru = -1;
    for (size_t i = 0; i < g_pages.size(); i++) if (g_pages[i].lastUse != g_frameNo && (lru < 0 || g_pages[i].lastUse < g_pages[lru].lastUse)) lru = (int)i;
    if (lru < 0) return false;
    Page& p = g_pages[lru];
    for (auto& k : p.keys) { auto it = g_cache.find(k); if (it != g_cache.end() && it->second.page == lru) g_cache.erase(it); }
    p.keys.clear(); p.shelves.clear(); p.nextY = 0;
    page = lru; return shelfAlloc(p, w, h, ox, oy);
}
static size_t bigHigh() { return std::max<size_t>(256u << 20, (size_t)g_renderCfg.cacheMB * (2u << 20) / 3); }
static void evictBig() {
    if (g_bigBytes < bigHigh()) return;
    std::vector<std::pair<uint32_t, TexKey>> order;
    for (auto& kv : g_cache) if (kv.second.page < 0 && kv.second.tex) order.push_back({kv.second.lastUse, kv.first});
    std::sort(order.begin(), order.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    for (auto& o : order) {
        if (g_bigBytes < bigHigh() * 3 / 4 || o.first == g_frameNo) break;
        TexEntry& e = g_cache[o.second];
        glDeleteTextures(1, &e.tex); g_bigBytes -= (size_t)e.w * e.h * 4; g_cache.erase(o.second);
    }
}

static TexEntry* storeRaster(const TexKey& k, bool ok, Raster& r) {
    TexEntry e; e.lastUse = g_frameNo;
    if (!ok || r.w < 3 || r.h < 3) { e.empty = true; g_cache[k] = e; return nullptr; }
    e.w = r.w; e.h = r.h; e.x0 = r.x0; e.y0 = r.y0; e.scale = r.scale;
    int page, ox, oy;
    if (r.w <= 1536 && r.h <= 1536 && atlasPlace(r.w + 1, r.h + 1, page, ox, oy)) {
        Page& p = g_pages[page];
        e.page = page; e.tex = p.tex; e.px = ox; e.py = oy; e.texW = e.texH = kPage;
        p.keys.push_back(k); p.lastUse = g_frameNo;
    } else {
        e.page = -1; e.tex = newTexture(r.w, r.h); e.px = e.py = 0; e.texW = r.w; e.texH = r.h;
        g_bigBytes += (size_t)r.w * r.h * 4;
    }
    texUpload(e.tex, e.px, e.py, r.w, r.h, r.px.data(), r.w);
    return &(g_cache[k] = e);
}

// ============================================================ BitmapData → texture (by uid, pooled)
struct BmTex { GLuint tex = 0; int w = 0, h = 0; GLuint fbo = 0, rb = 0; };   // fbo: layer drawn on the GPU
static std::unordered_map<uint32_t, BmTex> g_bm;
static std::unordered_map<uint32_t, uint32_t> g_bmVer;   // content version of each bitmap texture (filter cache)
// Where a GPU bitmap has anything drawn (bitmap pixels; known only for the bitmaps the game draws into and clears):
// the game's full-room layers (blood, gibs, rocks' debris, shadows...) are mostly transparent, and drawing only their
// used part saves a lot of fill rate (each one covered the whole screen every frame). Not in the map: unknown (all).
struct BmBox { float x0 = 1e9f, y0 = 1e9f, x1 = -1e9f, y1 = -1e9f; bool empty() const { return x1 <= x0 || y1 <= y0; } };
static std::unordered_map<uint32_t, BmBox> g_bmBox;
static void bmBoxAdd(uint32_t uid, float x0, float y0, float x1, float y1) {
    auto it = g_bmBox.find(uid); if (it == g_bmBox.end()) return;   // (unknown stays unknown: the whole bitmap)
    BmBox& b = it->second; b.x0 = std::min(b.x0, x0); b.y0 = std::min(b.y0, y0); b.x1 = std::max(b.x1, x1); b.y1 = std::max(b.y1, y1);
}
static bool g_sceneBoxOn = false; static BmBox g_sceneBox;   // (renderScene: what its quads cover)
static std::vector<BmTex> g_bmPool;
static BmTex acquireBm(int w, int h) {
    for (size_t i = 0; i < g_bmPool.size(); i++) if (g_bmPool[i].w == w && g_bmPool[i].h == h) { BmTex t = g_bmPool[i]; g_bmPool.erase(g_bmPool.begin() + i); return t; }
    BmTex t; t.w = w; t.h = h; t.tex = newTexture(w, h); return t;
}
static void releaseBm(const BmTex& t0) {
    if (!t0.tex) return;
    BmTex t = t0;
    if (t.fbo) { glDeleteFramebuffers(1, &t.fbo); glDeleteRenderbuffers(1, &t.rb); t.fbo = t.rb = 0; }
    g_bmPool.push_back(t);
    if (g_bmPool.size() > g_lim.bmTexPoolMax) { glDeleteTextures(1, &g_bmPool.front().tex); g_bmPool.erase(g_bmPool.begin()); }
}

bool renderInit() {
    if (!loadGL()) return false;
    logf("OpenGL: %s | %s\n", (const char*)glGetString(GL_VERSION), (const char*)glGetString(GL_RENDERER));
    g_prog = link(kVS, kFS); g_progBlur = link(kVS, kBlurFS);
    u_tex = glGetUniformLocation(g_prog, "uTex"); u_alphaTest = glGetUniformLocation(g_prog, "uAlphaTest"); u_view = glGetUniformLocation(g_prog, "uView");
    ub_tex = glGetUniformLocation(g_progBlur, "uTex"); ub_dir = glGetUniformLocation(g_progBlur, "uDir"); ub_view = glGetUniformLocation(g_progBlur, "uView");
    u_flip = glGetUniformLocation(g_prog, "uFlip"); ub_flip = glGetUniformLocation(g_progBlur, "uFlip");
    u_colOn = glGetUniformLocation(g_prog, "uColOn"); u_colR = glGetUniformLocation(g_prog, "uColR"); u_colG = glGetUniformLocation(g_prog, "uColG"); u_colB = glGetUniformLocation(g_prog, "uColB");
    glUseProgram(g_progBlur); glUniform1f(ub_flip, 1.0f); glUseProgram(g_prog); glUniform1f(u_flip, 1.0f);
    glGenVertexArrays(1, &g_vao); glBindVertexArray(g_vao);
    g_vboRing.init(GL_ARRAY_BUFFER, kVboBytes, sizeof(Vtx) * 16);   // (offsets are multiples of the vertex size)
    for (int i = 0; i < 4; i++) glEnableVertexAttribArray(i);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vtx), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vtx), (void*)8);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vtx), (void*)16);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vtx), (void*)32);
    g_pboRing.init(GL_PIXEL_UNPACK_BUFFER, g_lim.pboBytes, 256);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, g_vboRing.buf);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    for (int i = 0; i < g_lim.initialAtlasPages; i++) addPage();   // atlas pages created up front
    return true;
}

// ============================================================ quad batches
// ---- recorded frame: the quads for the whole frame are uploaded at once (a single buffer mapping) and the
// draws and state changes are replayed in order at the end (renderEndFrame). Each mapping and each state
// change has a noticeable fixed cost in the Switch driver; this way there's one per frame, not dozens.
struct GOp { bool draw; GLuint tex; uint32_t first, count; std::function<void()> fn; };
static std::vector<GOp> g_gops;
static std::vector<Vtx> g_fverts;
static GLuint g_bigVbo = 0;
static void glrec(std::function<void()> f) { g_gops.push_back({false, 0, 0, 0, std::move(f)}); g_frameOpen = true; }
static void flush() {
    if (g_batch.empty()) return;
    uint32_t first = (uint32_t)g_fverts.size(), n = (uint32_t)g_batch.size();
    g_fverts.insert(g_fverts.end(), g_batch.begin(), g_batch.end());
    if (!g_gops.empty() && g_gops.back().draw && g_gops.back().tex == g_batchTex && g_gops.back().first + g_gops.back().count == first)
        g_gops.back().count += n;   // same state and texture: a single draw
    else { g_gops.push_back({true, g_batchTex, first, n, nullptr}); g_rstats.draws++; }
    g_rstats.quads += (int)n / 6;
    g_frameOpen = true;
    g_batch.clear();
}
static void submitFrame() {
    flush();
    if (g_gops.empty()) return;
    size_t bytes = g_fverts.size() * sizeof(Vtx), base = 0;
    if (bytes) {
        size_t at = g_vboRing.alloc(bytes);
        if (at != SIZE_MAX) {
            glBindBuffer(GL_ARRAY_BUFFER, g_vboRing.buf);
            void* dst = glMapBufferRange(GL_ARRAY_BUFFER, at, bytes, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
            if (dst) { memcpy(dst, g_fverts.data(), bytes); glUnmapBuffer(GL_ARRAY_BUFFER); }
            else glBufferSubData(GL_ARRAY_BUFFER, at, bytes, g_fverts.data());
            base = at / sizeof(Vtx);
        } else {   // (huge frame: separate buffer)
            if (!g_bigVbo) glGenBuffers(1, &g_bigVbo);
            glBindBuffer(GL_ARRAY_BUFFER, g_bigVbo); glBufferData(GL_ARRAY_BUFFER, bytes, g_fverts.data(), GL_STREAM_DRAW);
        }
        // the VAO stores each attribute's buffer: point them at the right one
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vtx), (void*)0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vtx), (void*)8);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vtx), (void*)16);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vtx), (void*)32);
    }
    GLuint bound = (GLuint)-1;
    for (GOp& op : g_gops) {
        if (op.draw) {
            if (op.tex != bound) { glBindTexture(GL_TEXTURE_2D, op.tex); bound = op.tex; }
            glDrawArrays(GL_TRIANGLES, (GLint)(base + op.first), (GLsizei)op.count);
        } else { op.fn(); bound = (GLuint)-1; }
    }
    g_gops.clear(); g_fverts.clear();
}
// a texture's filter: only changed if it differs from the current one (each change is costly in the driver)
static std::unordered_map<GLuint, bool> g_texSmooth;
static void g_texSmoothSet(GLuint t) { g_texSmooth[t] = true; }   // (new textures use linear filtering)
static void setSmooth(GLuint tex, bool smooth) {
    auto it = g_texSmooth.find(tex);
    if (it != g_texSmooth.end() && it->second == smooth) return;
    g_texSmooth[tex] = smooth;
    glrec([tex, smooth] {
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, smooth ? GL_LINEAR : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, smooth ? GL_LINEAR : GL_NEAREST);
    });
}
// draws the local rectangle [x0,x1]x[y0,y1] with matrix m (→ destination pixels)
static void quad(GLuint tex, const Mat& m, double x0, double y0, double x1, double y1, const CX& cx, float u0 = 0, float v0 = 0, float u1 = 1, float v1 = 1) {
    if (tex != g_batchTex) { flush(); g_batchTex = tex; }
    float mr = cx.rm, mg = cx.gm, mb = cx.bm, ma = cx.am, ar = cx.ra / 255.f, ag = cx.ga / 255.f, ab = cx.ba / 255.f, aa = cx.aa / 255.f;
    double px[4], py[4];
    m.apply(x0, y0, px[0], py[0]); m.apply(x1, y0, px[1], py[1]); m.apply(x1, y1, px[2], py[2]); m.apply(x0, y1, px[3], py[3]);
    static const bool qlog = getenv("ISAAC_QUADLOG") != nullptr;   // (testing) big quads
    if (qlog) {   // (fill area clipped to the window, summed per frame)
        double xa = std::min({px[0], px[1], px[2], px[3]}), xb = std::max({px[0], px[1], px[2], px[3]}), ya = std::min({py[0], py[1], py[2], py[3]}), yb = std::max({py[0], py[1], py[2], py[3]});
        xa = std::max(0.0, xa); ya = std::max(0.0, ya); xb = std::min((double)g_winW, xb); yb = std::min((double)g_winH, yb);
        extern double g_fillPx; if (xb > xa && yb > ya) g_fillPx += (xb - xa) * (yb - ya);
    }
    if (g_sceneBoxOn) for (int i = 0; i < 4; i++) {
        g_sceneBox.x0 = std::min(g_sceneBox.x0, (float)px[i]); g_sceneBox.y0 = std::min(g_sceneBox.y0, (float)py[i]);
        g_sceneBox.x1 = std::max(g_sceneBox.x1, (float)px[i]); g_sceneBox.y1 = std::max(g_sceneBox.y1, (float)py[i]);
    }
    float us[4] = {u0, u1, u1, u0}, vs[4] = {v0, v0, v1, v1};
    int idx[6] = {0, 1, 2, 0, 2, 3};
    for (int i : idx) g_batch.push_back({(float)px[i], (float)py[i], us[i], vs[i], mr, mg, mb, ma, ar, ag, ab, aa});
}
static GLuint whiteTex() {
    static GLuint w = 0;
    if (!w) { uint32_t px = 0xffffffffu; w = newTexture(1, 1); glBindTexture(GL_TEXTURE_2D, w); glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, g_glES ? GL_RGBA : GL_BGRA, g_glES ? GL_UNSIGNED_BYTE : GL_UNSIGNED_INT_8_8_8_8_REV, &px); }
    return w;
}

// ============================================================ render targets (FBO) for filters
struct RT { GLuint fbo = 0, tex = 0, rb = 0; int w = 0, h = 0; };
static std::vector<RT> g_rtPool;
// returns an intermediate texture of at least w x h (the smallest that fits): filters are processed one
// after another and returned when done, so a couple per frame suffice even when there are many
double g_fillPx = 0;
static RT acquireRT(int w, int h, bool exact = false) {
    static const bool qlog = getenv("ISAAC_QUADLOG") != nullptr;
    if (qlog && (long)w * h > 3000000) logf("[rt] %dx%d\n", w, h);
    int best = -1;
    if (exact) for (size_t i = 0; i < g_rtPool.size(); i++) if (g_rtPool[i].w == w && g_rtPool[i].h == h) { best = (int)i; break; }
    if (!exact)
    for (size_t i = 0; i < g_rtPool.size(); i++) {
        const RT& r = g_rtPool[i];
        if (r.w >= w && r.h >= h && (best < 0 || (long)r.w * r.h < (long)g_rtPool[best].w * g_rtPool[best].h)) best = (int)i;
    }
    if (best >= 0) { RT r = g_rtPool[best]; g_rtPool.erase(g_rtPool.begin() + best); return r; }
    if (!exact) { w = (w + 255) & ~255; h = (h + 255) & ~255; }   // rounded sizes: easier to reuse
    RT r; r.w = w; r.h = h;
    r.tex = newTexture(w, h);
    glGenRenderbuffers(1, &r.rb); glBindRenderbuffer(GL_RENDERBUFFER, r.rb); glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
    glGenFramebuffers(1, &r.fbo); glBindFramebuffer(GL_FRAMEBUFFER, r.fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, r.tex, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, r.rb);
    return r;
}
static void releaseRT(const RT& r) { g_rtPool.push_back(r); if (g_rtPool.size() > 12) { RT o = g_rtPool.front(); g_rtPool.erase(g_rtPool.begin());
    glrec([o] { glDeleteFramebuffers(1, &o.fbo); glDeleteTextures(1, &o.tex); glDeleteRenderbuffers(1, &o.rb); }); } }   // (after its last recorded use)
struct Target { GLuint fbo; int w, h; float flip = 1.f; };
static std::vector<Target> g_targets;
// RainbowVision (mods/RainbowVision.mod): a colour matrix applied to what reaches the window (not to the layers
// drawn off screen first: they reach the window afterwards). Set by the mod (logic thread), read here.
extern std::atomic<bool> g_colorMatOn; extern float g_colorMat[9];   // (mod_rainbow.cpp)
static void bindTarget(const Target& t) {
    flush();
    bool col = t.fbo == 0 && g_colorMatOn.load(std::memory_order_acquire);
    float m[9]; memcpy(m, g_colorMat, sizeof m);
    glrec([t, col, m] {
        glBindFramebuffer(GL_FRAMEBUFFER, t.fbo); glViewport(0, 0, t.w, t.h); glUseProgram(g_prog); glUniform2f(u_view, (float)t.w, (float)t.h); glUniform1f(u_flip, t.flip);
        glUniform1f(u_colOn, col ? 1.0f : 0.0f);
        if (col) { glUniform3f(u_colR, m[0], m[1], m[2]); glUniform3f(u_colG, m[3], m[4], m[5]); glUniform3f(u_colB, m[6], m[7], m[8]); }
    });
}
static bool g_recStencil = false, g_recScissor = false;   // recorded state (not GL's current state)

// ============================================================ snapshots and interpolation
// The renderer shows the state between the last two ticks: lerp(previous, current, alpha). Everything that
// changes continuously (position, scale, rotation, transparency, color, morphs, timeline
// tweens) is shown at the monitor's refresh rate. Frame-by-frame drawing changes
// (another graphic at the same depth) and jumps (teleports, new objects) are applied as-is.
static SnapPtr g_prevSnap, g_curSnap;
bool g_retireSnapshots = false;
static std::mutex g_retireMu; static std::vector<SnapPtr> g_retired;
struct NodeInterp { int32_t prev = -1; bool snap = true, jumped = false; };
static std::vector<NodeInterp> g_interp;
static std::vector<double> g_wscale;

static double maxScale(const Mat& m) { return std::sqrt(std::max(m.a * m.a + m.b * m.b, m.c * m.c + m.d * m.d)); }
static inline double lerpd(double a, double b, double t) { return a + (b - a) * t; }
static double angDiff(double a, double b) { double d = b - a; while (d > M_PI) d -= 2 * M_PI; while (d < -M_PI) d += 2 * M_PI; return d; }
static Mat lerpMat(const Mat& p, const Mat& c, double t) {
    Mat r;
    r.tx = lerpd(p.tx, c.tx, t); r.ty = lerpd(p.ty, c.ty, t);
    if (p.a == c.a && p.b == c.b && p.c == c.c && p.d == c.d) { r.a = c.a; r.b = c.b; r.c = c.c; r.d = c.d; return r; }
    if (p.b == 0 && p.c == 0 && c.b == 0 && c.c == 0) { r.a = lerpd(p.a, c.a, t); r.d = lerpd(p.d, c.d, t); r.b = r.c = 0; return r; }
    // scale and rotation separately (interpolating components would shrink rotating objects)
    double psx = std::hypot(p.a, p.b), csx = std::hypot(c.a, c.b), psy = std::hypot(p.c, p.d), csy = std::hypot(c.c, c.d);
    double prx = std::atan2(p.b, p.a), crx = std::atan2(c.b, c.a), pry = std::atan2(-p.c, p.d), cry = std::atan2(-c.c, c.d);
    double sx = lerpd(psx, csx, t), sy = lerpd(psy, csy, t);
    double rx = prx + angDiff(prx, crx) * t, ry = pry + angDiff(pry, cry) * t;
    r.a = sx * std::cos(rx); r.b = sx * std::sin(rx); r.c = -sy * std::sin(ry); r.d = sy * std::cos(ry);
    return r;
}
static CX lerpCX(const CX& p, const CX& c, float t) {
    if (p == c) return c;
    CX r;
    r.rm = p.rm + (c.rm - p.rm) * t; r.gm = p.gm + (c.gm - p.gm) * t; r.bm = p.bm + (c.bm - p.bm) * t; r.am = p.am + (c.am - p.am) * t;
    r.ra = p.ra + (c.ra - p.ra) * t; r.ga = p.ga + (c.ga - p.ga) * t; r.ba = p.ba + (c.ba - p.ba) * t; r.aa = p.aa + (c.aa - p.aa) * t;
    return r;
}

static void computeInterp() {
    Snapshot& cur = *g_curSnap;
    size_t n = cur.nodes.size();
    g_interp.assign(n, NodeInterp());
    cur.jump.assign(n, 0.f);
    cur.fresh.assign(n, 1);
    if (!g_prevSnap) return;
    Snapshot& prev = *g_prevSnap;
    g_wscale.resize(n);   // world scale of each node (at the current tick)
    for (size_t i = 0; i < n; i++) {
        const SnapNode& c = cur.nodes[i];
        double ps = c.parent >= 0 ? g_wscale[c.parent] : 1.0;
        g_wscale[i] = ps * maxScale(c.m);
        int pi = prev.findUid(c.uid);
        if (pi < 0) continue;
        cur.fresh[i] = 0;
        const SnapNode& p = prev.nodes[pi];
        NodeInterp& ni = g_interp[i];
        ni.prev = pi;
        // the parent teleported: show the whole subtree as it is now. (A new tear is created at (0,0) after the
        // tick's movement code ran; the next tick moves it to Isaac's head and raises its graphic to the tear's
        // height. Interpolating that child alone made the tear appear low and rise.)
        if (c.parent >= 0 && g_interp[c.parent].jumped) { ni.jumped = true; continue; }
        bool snap = p.kind != c.kind || p.objKind != c.objKind || p.def != c.def || p.visible != c.visible || p.bmUid != c.bmUid;
        if (!snap && p.m.a == c.m.a && p.m.b == c.m.b && p.m.c == c.m.c && p.m.d == c.m.d && p.m.tx == c.m.tx && p.m.ty == c.m.ty) {
            ni.snap = false; continue;   // (most common case: it hasn't moved)
        }
        double dp = p.m.a * p.m.d - p.m.b * p.m.c, dc = c.m.a * c.m.d - c.m.b * c.m.c;
        if ((dp < 0) != (dc < 0)) snap = true;                                    // flip (mirror)
        double sp = maxScale(p.m), sc = maxScale(c.m);
        if (sp > 0 && sc > 0 && (sc / sp > 2.0 || sp / sc > 2.0)) snap = true;    // abrupt size change
        if ((p.m.b != 0 || c.m.b != 0) && std::fabs(angDiff(std::atan2(p.m.b, p.m.a), std::atan2(c.m.b, c.m.a))) > M_PI / 2) snap = true;
        // jumps: more than 250 stage px and much more than the previous tick's displacement
        double jump = std::hypot(c.m.tx - p.m.tx, c.m.ty - p.m.ty) * ps;
        double last = (size_t)pi < prev.jump.size() ? prev.jump[pi] : 0;
        if (jump > 250 && jump > 2.5 * last + 40) { snap = true; ni.jumped = true; }
        // created during the last tick at the default (0,0) and only positioned now (attachMovie, then placed by
        // the next tick's code): it never was at (0,0) on screen in a meaningful way, so don't slide from there
        bool wasFresh = (size_t)pi < prev.fresh.size() && prev.fresh[pi];
        if (wasFresh && p.m.tx == 0 && p.m.ty == 0 && (c.m.tx != 0 || c.m.ty != 0)) { snap = true; ni.jumped = true; }
        if (!snap) cur.jump[i] = (float)jump;
        ni.snap = snap;
    }
}

static inline double msSince(Uint64 t0) { return (SDL_GetPerformanceCounter() - t0) * 1000.0 / SDL_GetPerformanceFrequency(); }
static void runGpuOps(Snapshot& s);
void renderAdopt(const SnapPtr& s) {
    Uint64 t0 = SDL_GetPerformanceCounter();
    // 1) what was drawn into GPU layers during the tick, in order; 2) pixels changed on the CPU; 3) what was freed
    if (!s->gpuOps.empty()) runGpuOps(*s);
    for (auto& up : s->uploads) {
        BmTex& t = g_bm[up.uid];
        if (!t.tex || t.w != up.w || t.h != up.h) { if (t.tex) releaseBm(t); t = acquireBm(up.w, up.h); }
        texUpload(t.tex, up.x0, up.y0, up.x1 - up.x0, up.y1 - up.y0, up.px.data(), up.x1 - up.x0);
        g_bmVer[up.uid]++;
        bmBoxAdd(up.uid, (float)up.x0, (float)up.y0, (float)up.x1, (float)up.y1);
    }
    s->uploads.clear(); s->uploads.shrink_to_fit();
    for (uint32_t uid : s->freedBitmaps) { auto it = g_bm.find(uid); if (it != g_bm.end()) { releaseBm(it->second); g_bm.erase(it); } g_bmBox.erase(uid); }
    if (!s->indexed) {
        uint32_t cap = 16; while (cap < s->nodes.size() * 2) cap <<= 1;
        s->uidTable.assign(cap, 0); s->uidMask = cap - 1;
        for (size_t i = 0; i < s->nodes.size(); i++) {
            uint32_t uid = s->nodes[i].uid;
            uint32_t h = (uid * 2654435761u) & s->uidMask;
            while (s->uidTable[h] && (uint32_t)s->uidTable[h] != uid) h = (h + 1) & s->uidMask;
            s->uidTable[h] = ((uint64_t)i << 32) | uid;
        }
        s->indexed = true;
    }
    SnapPtr old = std::move(g_prevSnap);
    g_prevSnap = g_curSnap; g_curSnap = s;
    computeInterp();
    // freeing a snapshot means hundreds of free() calls (nodes, texts, draw scenes); on Switch that competed
    // with the logic thread's allocator for the malloc lock and showed up as 20-60 ms "adopt" spikes.
    // With a logic thread, it gets them back and frees them there.
    if (old && g_retireSnapshots) { std::lock_guard<std::mutex> lk(g_retireMu); g_retired.push_back(std::move(old)); }
    old.reset();
    g_rstats.adoptMs += msSince(t0);
}
void renderFreeRetired() {
    std::vector<SnapPtr> v;
    { std::lock_guard<std::mutex> lk(g_retireMu); v.swap(g_retired); }
}   // (v is destroyed here, on the calling thread)
const Snapshot* renderCurrent() { return g_curSnap.get(); }
void renderFinish() { glFinish(); }
// Low-latency frame pacing (Switch): a fence after each present and, before starting the
// next frame, wait for the GPU to have finished the one before it. This way there's never more than one
// frame queued behind the one being displayed, and input is read as late as possible.
static GLsync g_paceFence[2] = {nullptr, nullptr}; static int g_paceIdx = 0;
void renderPaceAfterSwap() {
    if (g_paceFence[g_paceIdx]) glDeleteSync(g_paceFence[g_paceIdx]);
    g_paceFence[g_paceIdx] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    if (kIsAndroid) {
        // Android hands out the next screen buffer only when the compositor frees one, and the driver waits for it at
        // the frame's first draw - i.e. AFTER input was read (~14 ms per frame on the test phone). Touching the
        // window surface right now moves that wait here, before the next frame reads input: same frame rate,
        // fresher input. (The clear is overwritten by the next frame.)
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0, 0, 0, 1); glClear(GL_COLOR_BUFFER_BIT);
        glFlush();
    }
    g_paceIdx ^= 1;
}
bool renderPaceWait(uint64_t timeoutNs) {
    GLsync f = g_paceFence[g_paceIdx];   // the one from the frame before the last presented one
    if (!f) return true;
    GLenum r = glClientWaitSync(f, GL_SYNC_FLUSH_COMMANDS_BIT, timeoutNs);
    if (r == GL_TIMEOUT_EXPIRED) return false;
    glDeleteSync(f); g_paceFence[g_paceIdx] = nullptr;
    return true;
}

// ============================================================ compiling the frame into commands
enum : uint8_t { C_LEAF, C_BITMAP, C_SPUSH, C_SPUSH_END, C_SPOP, C_SPOP_END, C_FILTER, C_FILTER_END };
struct Cmd { uint8_t op; uint32_t node = 0, match = 0; uint16_t ratio = 0; Mat m; CX cx; RectF b; bool anyB = false; float bx = 0, by = 0; int passes = 1; };
static std::vector<Cmd> g_cmds;
static double g_alpha = 1;

static void growR(RectF& r, bool& any, const Mat& m, double x0, double y0, double x1, double y1) {
    double xs[4] = {x0, x1, x0, x1}, ys[4] = {y0, y0, y1, y1};
    for (int i = 0; i < 4; i++) { double x, y; m.apply(xs[i], ys[i], x, y);
        if (!any) { r.x0 = r.x1 = x; r.y0 = r.y1 = y; any = true; } else { r.x0 = std::min(r.x0, x); r.x1 = std::max(r.x1, x); r.y0 = std::min(r.y0, y); r.y1 = std::max(r.y1, y); } }
}

static void compileNode(uint32_t i, const Mat& parent, const CX& pcx, bool maskPass);
static float g_inkFade = -1;             // >= 0 while compiling the subtree of the focused option (see kFS)
static float g_inkBase = 10;            // 10: pink, 20: deep red (see kFS)
static void inkMark(CX& cx) { if (g_inkFade >= 0) cx.aa = (g_inkBase + g_inkFade) * 255; }
static uint32_t g_sceneTargetUid = 0;   // BitmapData whose texture is being drawn into (0 = the screen)
static void compileChildren(uint32_t i, const Mat& m, const CX& cx, bool maskPass) {
    const std::vector<SnapNode>& N = g_curSnap->nodes;
    uint32_t kids[1024]; std::vector<uint32_t> big; uint32_t* K = kids; size_t nk = 0;
    for (uint32_t j = i + 1; j < N[i].end; j = N[j].end) {
        if (nk == 1024) { big.assign(kids, kids + 1024); K = nullptr; }
        if (K) K[nk] = j; else big.push_back(j);
        nk++;
    }
    if (!K) K = big.data();
    for (size_t q = 0; q < nk; q++) {
        const SnapNode& k = N[K[q]];
        if (k.clipDepth && !maskPass) {
            size_t e = q + 1;
            while (e < nk && N[K[e]].depth <= k.clipDepth) e++;
            Cmd c; c.op = C_SPUSH; g_cmds.push_back(c);
            compileNode(K[q], m, cx, true);
            c.op = C_SPUSH_END; g_cmds.push_back(c);
            for (size_t r = q + 1; r < e; r++) compileNode(K[r], m, cx, false);
            c.op = C_SPOP; g_cmds.push_back(c);
            compileNode(K[q], m, cx, true);
            c.op = C_SPOP_END; g_cmds.push_back(c);
            q = e - 1;
            continue;
        }
        compileNode(K[q], m, cx, maskPass);
    }
}
static void compileNode(uint32_t i, const Mat& parent, const CX& pcx, bool maskPass) {
    const SnapNode& n = g_curSnap->nodes[i];
    if (!n.visible && !maskPass) return;
    const NodeInterp& ip = g_interp[i];
    const SnapNode* p = (ip.prev >= 0 && !ip.snap && g_alpha != 1) ? &g_prevSnap->nodes[ip.prev] : nullptr;
    Mat local = p ? lerpMat(p->m, n.m, g_alpha) : n.m;
    Mat m = parent * local;
    CX cx = maskPass ? CX() : pcx * (p ? lerpCX(p->cx, n.cx, (float)g_alpha) : n.cx);
    if (!maskPass && cx.am <= 0 && cx.aa <= 0) return;
    // option selected with a controller (menuui.cpp): everything drawn inside it goes through the shader's "ink"
    // mode (dark ink -> one fixed red). It is told how bright the parents leave it (for slightly tinted screens)
    struct InkScope { float saved = g_inkFade; ~InkScope() { g_inkFade = saved; } } inkScope;
    if (!maskPass && n.uid && n.uid == g_curSnap->focusUid && g_inkFade < 0) {
        float fade = std::max(0.f, std::min(1.f, (pcx.rm + pcx.gm + pcx.bm) / 3 + (pcx.ra + pcx.ga + pcx.ba) / (3 * 255.f)));
        if (fade > 0.97f) { g_inkFade = fade; g_inkBase = g_curSnap->focusStrong ? 20.f : 10.f; }   // (once the screen starts fading to black the selection isn't shown)
    }
    if (!maskPass && n.blur && g_renderCfg.filters && n.kind != SN_BITMAP) {
        size_t at = g_cmds.size();
        Cmd f; f.op = C_FILTER; f.node = i;
        double sc = maxScale(m) / std::max(1e-6, maxScale(local));
        f.bx = (float)(n.blurX * sc); f.by = (float)(n.blurY * sc); f.passes = n.blurPasses;
        g_cmds.push_back(f);
        if (n.kind == SN_CONTAINER) compileChildren(i, m, cx, false);
        else { Cmd c; c.op = C_LEAF; c.node = i; c.m = m; c.cx = cx; inkMark(c.cx); c.ratio = p && n.objKind == OK_MORPH ? (uint16_t)std::max(0L, std::min(65535L, std::lround(lerpd(p->ratio, n.ratio, std::min(1.0, g_alpha))))) : n.ratio; g_cmds.push_back(c); }
        Cmd e; e.op = C_FILTER_END; g_cmds.push_back(e);
        // bounds (destination px) of everything drawn inside
        RectF r; bool any = false;
        for (size_t k = at + 1; k < g_cmds.size(); k++) {
            const Cmd& c = g_cmds[k];
            const SnapNode& cn = g_curSnap->nodes[c.node];
            if (c.op == C_LEAF && cn.hasBounds) growR(r, any, c.m, cn.bounds.x0, cn.bounds.y0, cn.bounds.x1, cn.bounds.y1);
            else if (c.op == C_BITMAP) growR(r, any, c.m, 0, 0, cn.bmW, cn.bmH);
        }
        g_cmds[at].b = r; g_cmds[at].anyB = any; g_cmds[at].match = (uint32_t)g_cmds.size() - 1;
        return;
    }
    switch (n.kind) {
        case SN_CONTAINER: compileChildren(i, m, cx, maskPass); break;
        case SN_BITMAP: { Cmd c; c.op = C_BITMAP; c.node = i; c.m = m; c.cx = cx; inkMark(c.cx); g_cmds.push_back(c); break; }
        case SN_CONTENT: {
            Cmd c; c.op = C_LEAF; c.node = i; c.m = m; c.cx = cx; inkMark(c.cx);
            c.ratio = p && n.objKind == OK_MORPH ? (uint16_t)std::max(0L, std::min(65535L, std::lround(lerpd(p->ratio, n.ratio, std::min(1.0, g_alpha))))) : n.ratio;
            g_cmds.push_back(c); break;
        }
    }
}

// ============================================================ rasterizing what's missing (in parallel)
static bool texKeyOf(const SnapNode& n, uint16_t ratio, double scale, TexKey& k) {
    if (n.kind != SN_CONTENT || !n.def) return false;
    uint32_t variant = n.objKind == OK_EDITTEXT ? n.textVersion : n.objKind == OK_MORPH ? (uint32_t)(ratio & 0xffc0) : 0;
    if (!(scale >= 1.0 / 64)) scale = 1.0 / 64; if (scale > 64) scale = 64;
    int bucket = (int)std::ceil(std::log2(scale) * 4.0 - 1e-6);
    k = TexKey{n.def, variant, (int16_t)bucket};
    return true;
}
static RasterSrc srcOf(const SnapNode& n, uint16_t ratio) {
    RasterSrc s; s.kind = n.objKind; s.def = n.def; s.ratio = ratio; s.argb = n.argb;
    if (n.text >= 0) s.text = &g_curSnap->texts[n.text];
    return s;
}
static TexEntry* nearbyRaster(const TexKey& k) {
    for (int db = 1; db <= 8; db++) for (int sgn : {1, -1}) {
        auto it2 = g_cache.find({k.def, k.variant, (int16_t)(k.bucket + db * sgn)});
        if (it2 != g_cache.end() && !it2->second.empty) return &it2->second;
    }
    return nullptr;
}
// ---- background rasterization
// Graphics not yet in the atlas are rasterized on separate threads. The frame waits at most
// g_renderCfg.rasterWaitMs (meanwhile the render thread itself also rasterizes); anything not ready is
// drawn at another already-rasterized scale or appears in the next frame. This way entering a room with many
// new graphics doesn't freeze the image.
struct RJob {
    TexKey k; RasterSrc src; std::string text; double scale = 1; Raster r; bool ok = false; double ms = 0;
    std::atomic<bool> done{false};
    bool pre = false;       // from the preload (background queue)
    bool inPreQueue = false; // still waiting in that queue (guarded by g_rqMu)
    bool wanted = false;    // needed by a frame: store it in the atlas as soon as it's ready
};
static std::mutex g_rqMu;
static std::condition_variable* g_rqCv = new std::condition_variable;       // (never destroyed: threads
static std::condition_variable* g_rqDoneCv = new std::condition_variable;   //  wait on them while exiting)
static std::deque<RJob*> g_rqQueue;
static std::deque<RJob*> g_rqPreQueue;   // preload: only when nothing is urgent
static size_t g_preTotal = 0, g_preDone = 0;
static std::vector<RJob*> g_rqFinished;
static std::unordered_map<TexKey, RJob*, TexKeyHash> g_rqInflight;   // (render thread only)
static int g_rqWorkers = -1;
static void rqRun(RJob* j) {
    Uint64 t0 = SDL_GetPerformanceCounter();
    j->ok = rasterizeSrc(j->src, j->scale, j->r);
    j->ms = (SDL_GetPerformanceCounter() - t0) * 1000.0 / SDL_GetPerformanceFrequency();
    { std::lock_guard<std::mutex> lk(g_rqMu); g_rqFinished.push_back(j); j->done = true; }
    g_rqDoneCv->notify_all();
}
static void rqWorker(void* arg) {
    int i = (int)(intptr_t)arg, core = -1, prio = -1;
    platformRasterServicePlace(i, &core, &prio); platformSetThread(core, prio);
    for (;;) {
        RJob* j;
        {
            std::unique_lock<std::mutex> lk(g_rqMu);
            g_rqCv->wait(lk, [] { return !g_rqQueue.empty() || !g_rqPreQueue.empty() || platformStopping(); });
            if (platformStopping()) return;
            if (!g_rqQueue.empty()) { j = g_rqQueue.front(); g_rqQueue.pop_front(); }
            else { j = g_rqPreQueue.front(); g_rqPreQueue.pop_front(); j->inPreQueue = false; }
        }
        rqRun(j);
    }
}
static void rqStart() {
    if (g_rqWorkers >= 0) return;
    g_rqWorkers = 0;
    platformOnStop([] { std::lock_guard<std::mutex> lk(g_rqMu); g_rqCv->notify_all(); });
    int want = platformRasterServiceWorkers();
    if (const char* e = getenv("ISAAC_THREADS")) want = std::max(0, std::min(8, atoi(e)));
    for (int i = 0; i < want; i++) {
        int core = -1, prio = -1; platformRasterServicePlace(i, &core, &prio);
        if (!platformStartThread(rqWorker, (void*)(intptr_t)i, core, "render rasterizer")) break;
        g_rqWorkers++;
    }
}
// stores what's already rasterized in the atlas. Preload work: at most preBudgetMs per frame (uploading
// many textures at once would stutter the opening cutscene); everything the frame needs, in full.
static void rqDrain(double preBudgetMs = 1e9) {
    std::vector<RJob*> fin, later;
    { std::lock_guard<std::mutex> lk(g_rqMu); fin.swap(g_rqFinished); }
    Uint64 t0 = SDL_GetPerformanceCounter(), freq = SDL_GetPerformanceFrequency();
    for (RJob* j : fin) {
        if (j->pre && !j->wanted && (SDL_GetPerformanceCounter() - t0) * 1000.0 / freq > preBudgetMs) { later.push_back(j); continue; }
        g_rqInflight.erase(j->k);
        if (j->pre) g_preDone++;
        else { g_rstats.rasters++; g_rstats.rasterMs += j->ms; }
        storeRaster(j->k, j->ok, j->r);
        delete j;
    }
    if (!later.empty()) { std::lock_guard<std::mutex> lk(g_rqMu); g_rqFinished.insert(g_rqFinished.begin(), later.begin(), later.end()); }
}
// ---- graphics seen (for preloading next time)
static std::vector<PreKey> g_learnNew;
static std::unordered_map<uint64_t, bool> g_learnSeen;
// (the scale enters by quarter-octave step: keys a few hundredths apart are the same texture and are learned once)
static uint64_t preHash(const PreKey& p) { int16_t qb = (int16_t)std::ceil(p.q100 / 100.0 - 1e-6); return (uint64_t)p.id | (uint64_t)p.kind << 16 | (uint64_t)p.abs << 24 | (uint64_t)p.variant << 32 | (uint64_t)(uint16_t)qb << 48; }
static double winQ(int winH) { return 4.0 * std::log2(std::max(1, winH) / 720.0); }
extern uint16_t g_synthCharBase;
static void learnKey(const SnapNode& n, const TexKey& k, double scale) {
    if (!n.def || n.objKind == OK_EDITTEXT) return;   // (editable texts change: not preloaded)
    if (n.def->id >= g_synthCharBase) return;         // (shapes made at run time: menuui.cpp)
    PreKey p; p.id = n.def->id; p.kind = n.objKind; p.abs = g_sceneTargetUid != 0; p.variant = (uint16_t)k.variant;
    double q = 4.0 * std::log2(std::max(1.0 / 64, std::min(64.0, scale)));   // (unrounded: the exact scale)
    p.q100 = (int16_t)std::lround((p.abs ? q : q - winQ(g_winH)) * 100);
    if (!p.abs && p.q100 > 800) return;   // giant zooms: not worth preloading (see renderPreload)
    if (g_learnSeen.emplace(preHash(p), true).second) g_learnNew.push_back(p);
}
std::vector<PreKey> renderTakeLearned() { std::vector<PreKey> v; v.swap(g_learnNew); return v; }
void renderMarkKnown(const std::vector<PreKey>& keys) { for (auto& p : keys) g_learnSeen.emplace(preHash(p), true); }

// A shape that is one solid axis-aligned rectangle (screen fades, dimming, plain panels) is drawn as a coloured quad,
// not rasterized: the same picture at any size (WideSaac draws the screen-wide ones larger), no texture.
struct PlainRect { bool plain = false; RectF r; uint32_t argb = 0; };
static const bool g_noPlainRects = getenv("ISAAC_NOPLAINRECT") != nullptr;   // (testing)
static const PlainRect& plainRectOf(const CharDef* d) {
    static std::unordered_map<const CharDef*, PlainRect> cache;
    auto it = cache.find(d); if (it != cache.end()) return it->second;
    PlainRect pr;
    if (d && d->type == CH_SHAPE) {
        ShapeDef* sd = (ShapeDef*)d; shapeEnsure(sd);
        if (sd->layers.size() == 1 && sd->layers[0].fills.size() == 1 && sd->layers[0].fills[0].type == 0 && sd->layers[0].fillPaths.size() == 1 &&
            sd->layers[0].lines.empty()) {
            const Path& p = sd->layers[0].fillPaths[0];
            bool ok = !p.verbs.empty() && p.verbs[0] == 0 && p.verbs.size() <= 6;
            for (size_t i = 1; ok && i < p.verbs.size(); i++) ok = p.verbs[i] == 1;
            float x0 = 1e9f, y0 = 1e9f, x1 = -1e9f, y1 = -1e9f;
            for (size_t i = 0; ok && i + 1 < p.pts.size(); i += 2) { x0 = std::min(x0, p.pts[i]); x1 = std::max(x1, p.pts[i]); y0 = std::min(y0, p.pts[i + 1]); y1 = std::max(y1, p.pts[i + 1]); }
            for (size_t i = 0; ok && i + 1 < p.pts.size(); i += 2)   // every point on a corner
                ok = (std::fabs(p.pts[i] - x0) < 0.01f || std::fabs(p.pts[i] - x1) < 0.01f) && (std::fabs(p.pts[i + 1] - y0) < 0.01f || std::fabs(p.pts[i + 1] - y1) < 0.01f);
            if (ok && x1 > x0 && y1 > y0) { pr.plain = true; pr.r = {x0, y0, x1, y1}; pr.argb = sd->layers[0].fills[0].argb; }
        }
    }
    return cache[d] = pr;
}
static void prerasterize() {
    rqStart();
    rqDrain(1.5);
    std::vector<RJob*> need; int added = 0;
    for (const Cmd& c : g_cmds) {
        if (c.op != C_LEAF) continue;
        const SnapNode& n = g_curSnap->nodes[c.node];
        if (n.objKind == OK_SHAPE && !g_noPlainRects && plainRectOf(n.def).plain) continue;
        TexKey k; if (!texKeyOf(n, c.ratio, maxScale(c.m), k)) continue;
        if (g_cache.count(k)) continue;
        auto it = g_rqInflight.find(k);
        if (it != g_rqInflight.end()) {
            RJob* j = it->second; j->wanted = true;
            if (!j->done) {
                if (j->pre) {   // was in the preload: move it to the front
                    std::lock_guard<std::mutex> lk(g_rqMu);
                    if (j->inPreQueue) { g_rqPreQueue.erase(std::find(g_rqPreQueue.begin(), g_rqPreQueue.end(), j)); j->inPreQueue = false; g_rqQueue.push_front(j); added++; }
                }
                need.push_back(j);
            } else need.push_back(j);
            continue;
        }
        if (added >= g_rasterBudget && nearbyRaster(k)) continue;   // too many new ones: use another scale this frame
        learnKey(n, k, maxScale(c.m));
        RJob* j = new RJob(); j->k = k; j->src = srcOf(n, c.ratio); j->scale = std::pow(2.0, k.bucket / 4.0);
        if (j->src.text) { j->text = *j->src.text; j->src.text = &j->text; }   // (the snapshot may go away first)
        g_rqInflight[k] = j; need.push_back(j); added++;
        { std::lock_guard<std::mutex> lk(g_rqMu); g_rqQueue.push_back(j); }
    }
    if (added) g_rqCv->notify_all();
    if (need.empty()) return;
    // wait (while helping) for this frame's work, with a time limit. If any of them has no other
    // scale to show in the meantime, wait longer: a slightly longer frame is better than a gap.
    double waitMs = g_rqWorkers > 0 ? g_renderCfg.rasterWaitMs : 1e9;
    for (RJob* j : need) if (!j->done && !nearbyRaster(j->k)) { waitMs = std::max(waitMs, g_renderCfg.rasterWaitNoFallbackMs); break; }
    Uint64 t0 = SDL_GetPerformanceCounter(), freq = SDL_GetPerformanceFrequency();
    auto elapsed = [&] { return (SDL_GetPerformanceCounter() - t0) * 1000.0 / freq; };
    for (;;) {
        bool all = true; for (RJob* j : need) if (!j->done) { all = false; break; }
        if (all) break;
        double left = waitMs - elapsed();
        if (left <= 0) break;
        RJob* mine = nullptr;
        {
            std::unique_lock<std::mutex> lk(g_rqMu);
            if (!g_rqQueue.empty()) { mine = g_rqQueue.front(); g_rqQueue.pop_front(); }
            else g_rqDoneCv->wait_for(lk, std::chrono::microseconds((long)(std::min(left, 50.0) * 1000)));
        }
        if (mine) rqRun(mine);
    }
    int pending = 0; for (RJob* j : need) if (!j->done) pending++;
    g_rstats.asyncPending += pending;
    rqDrain(1.5);
}
static TexEntry* rasterFor(const Cmd& c) {
    const SnapNode& n = g_curSnap->nodes[c.node];
    TexKey k; if (!texKeyOf(n, c.ratio, maxScale(c.m), k)) return nullptr;
    auto it = g_cache.find(k);
    TexEntry* e = nullptr;
    if (it != g_cache.end()) e = it->second.empty ? nullptr : &it->second;
    else e = nearbyRaster(k);   // still in flight: another scale, if any (otherwise it appears within a frame)
    if (e) { e->lastUse = g_frameNo; if (e->page >= 0) g_pages[e->page].lastUse = g_frameNo; }
    return e;
}

// ============================================================ execution
static Mat g_off;   // offset of the current target (a filter's FBO)
static void execRange(size_t from, size_t to);
// Filter cache: if the contents of a blur haven't changed (same pieces, same relative
// positions, same color, same bitmap contents), the previous frame's result is reused
// instead of redoing the blur (like cacheAsBitmap in Flash). Menus have about ten per frame.
struct FCache { RT rt; uint64_t sig = 0; uint32_t lastFrame = 0, computed = 0; int w = 0, h = 0; float ds = 1; };
static int g_filterRecomputes = 0;   // this frame
static std::unordered_map<uint64_t, FCache> g_fcache;
static TexEntry* rasterFor(const Cmd& c);
static uint64_t filterSig(size_t i, int x0, int y0, int w, int h, float ds, float bx, float by, int passes) {
    uint64_t hsh = 1469598103934665603ull;
    auto mix = [&](const void* p, size_t n) { const uint8_t* b = (const uint8_t*)p; for (size_t k = 0; k < n; k++) { hsh ^= b[k]; hsh *= 1099511628211ull; } };
    int hdr[4] = {w, h, passes, (int)g_sceneTargetUid}; mix(hdr, sizeof hdr); float fp[3] = {ds, bx, by}; mix(fp, sizeof fp);
    const Cmd& f = g_cmds[i];
    for (size_t k = i + 1; k < f.match; k++) {
        const Cmd& c = g_cmds[k];
        mix(&c.op, 1);
        if (c.op != C_LEAF && c.op != C_BITMAP && c.op != C_FILTER) continue;
        const SnapNode& n = g_curSnap->nodes[c.node];
        Mat m = g_off * c.m; m.tx -= x0; m.ty -= y0;
        double mm[6] = {m.a, m.b, m.c, m.d, m.tx, m.ty}; mix(mm, sizeof mm);
        mix(&c.cx, sizeof c.cx); mix(&c.ratio, sizeof c.ratio);
        if (c.op == C_FILTER) { float fb[3] = {c.bx, c.by, (float)c.passes}; mix(fb, sizeof fb); continue; }
        mix(&n.def, sizeof n.def); mix(&n.textVersion, sizeof n.textVersion);
        if (c.op == C_BITMAP) { uint32_t bv[2] = {n.bmUid, g_bmVer[n.bmUid]}; mix(bv, sizeof bv); mix(&n.smoothing, 1); }
        else { TexEntry* e = rasterFor(c); const void* ep = e; mix(&ep, sizeof ep); if (e) { int pos[4] = {e->page, (int)e->tex, e->px, e->py}; mix(pos, sizeof pos); } }
    }
    return hsh;
}
static void execFilter(size_t i) {
    const Cmd& f = g_cmds[i];
    if (!f.anyB) return;
    Target cur = g_targets.back();
    RectF r; bool any = false; growR(r, any, g_off, f.b.x0, f.b.y0, f.b.x1, f.b.y1);
    int passes = std::max(1, std::min(3, f.passes));
    float bx = f.bx, by = f.by;
    int padx = (int)std::ceil(bx * passes) + 2, pady = (int)std::ceil(by * passes) + 2;
    int x0 = (int)std::floor(r.x0) - padx, y0 = (int)std::floor(r.y0) - pady, x1 = (int)std::ceil(r.x1) + padx, y1 = (int)std::ceil(r.y1) + pady;
    x0 = std::max(x0, -padx); y0 = std::max(y0, -pady); x1 = std::min(x1, cur.w + padx); y1 = std::min(y1, cur.h + pady);
    int w = x1 - x0, h = y1 - y0;
    if (w <= 0 || h <= 0 || w > 4096 || h > 4096) return;
    flush();
    g_rstats.filters++;
    // large blurs at reduced resolution (the result is indistinguishable, and on the Switch GPU a
    // full-screen blur cost more than the entire rest of the frame)
    float bmax = std::max(bx, by);
    static bool flog = getenv("ISAAC_FILTERLOG") != nullptr;
    if (flog) logf("[filter] frame %u: %dx%d blur %.1fx%.1f passes %d\n", g_frameNo, w, h, bx, by, passes);
    float ds = g_renderCfg.filterLowRes ? (bmax >= 12 ? 0.25f : bmax >= 4 ? 0.5f : 1.f) : bmax >= 24 ? 0.25f : bmax >= 8 ? 0.5f : 1.f;
    int fw = w, fh = h;                                      // full-scale size
    w = std::max(1, (int)std::ceil(fw * ds)); h = std::max(1, (int)std::ceil(fh * ds));
    bx *= ds; by *= ds;
    // same contents as last time?
    uint64_t key = ((uint64_t)g_curSnap->nodes[f.node].uid << 32) ^ g_sceneTargetUid;
    uint64_t sig = filterSig(i, x0, y0, w, h, ds, bx, by, passes);
    {
        FCache& fc = g_fcache[key];
        bool same = fc.rt.fbo && fc.sig == sig && fc.rt.w >= w && fc.rt.h >= h;
        // over this frame's budget: show the previous result if it is recent and about the same size
        bool stale = !same && fc.rt.fbo && g_renderCfg.filterBudget > 0 && g_filterRecomputes >= g_renderCfg.filterBudget &&
                     g_frameNo - fc.computed < (uint32_t)g_renderCfg.filterMaxAge && fc.ds == ds &&
                     std::abs(fc.w - w) <= 2 + w / 16 && std::abs(fc.h - h) <= 2 + h / 16;
        if (same || stale) {
            fc.lastFrame = g_frameNo; g_rstats.filterHits++;
            int cw = same ? w : fc.w, ch = same ? h : fc.h;
            Mat place; place.a = 1 / ds; place.d = -1 / ds; place.tx = x0; place.ty = y0 + ch / ds;
            quad(fc.rt.tex, place, 0, 0, cw, ch, CX(), 0, 0, (float)cw / fc.rt.w, (float)ch / fc.rt.h);
            flush();
            return;
        }
        g_filterRecomputes++;
    }
    RT a = acquireRT(w, h), b = acquireRT(a.w, a.h, true);
    int RW = a.w, RH = a.h;
    float u1 = (float)w / RW, v1 = (float)h / RH;
    int savedStencil = g_stencilLevel; g_stencilLevel = 0;
    bool stencilOn = g_recStencil, scissorOn = g_recScissor; g_recStencil = g_recScissor = false;
    GLuint bfbo = b.fbo;
    glrec([bfbo] { glDisable(GL_STENCIL_TEST); glDisable(GL_SCISSOR_TEST); glClearColor(0, 0, 0, 0); glClearStencil(0);
                   glBindFramebuffer(GL_FRAMEBUFFER, bfbo); glClear(GL_COLOR_BUFFER_BIT); });
    g_targets.push_back({a.fbo, w, h}); bindTarget(g_targets.back());
    glrec([] { glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); });
    Mat savedOff = g_off; Mat sh; sh.tx = -x0; sh.ty = -y0; Mat sc; sc.a = sc.d = ds; g_off = sc * (sh * g_off);
    execRange(i + 1, f.match);
    flush();
    g_off = savedOff;
    // separable blur (box, repeated "passes" times): X a->b, Y b->a
    glrec([w, h] { glUseProgram(g_progBlur); glUniform2f(ub_view, (float)w, (float)h); glUniform1i(ub_tex, 0); glDisable(GL_BLEND); });
    Mat flipm; flipm.d = -1; flipm.ty = h;   // FBO textures are flipped in Y
    for (int p = 0; p < passes; p++) {
        for (int axis = 0; axis < 2; axis++) {
            int ri = (int)std::round((axis == 0 ? bx : by) / 2);
            RT& src = axis == 0 ? a : b; RT& dst = axis == 0 ? b : a;
            GLuint dfbo = dst.fbo;
            glrec([dfbo, w, h, axis, RW, RH, ri] { glBindFramebuffer(GL_FRAMEBUFFER, dfbo); glViewport(0, 0, w, h);
                glUniform3f(ub_dir, axis == 0 ? 1.f / RW : 0.f, axis == 1 ? 1.f / RH : 0.f, (float)std::max(0, ri)); });
            quad(src.tex, flipm, 0, 0, w, h, CX(), 0, 0, u1, v1);
            flush();
        }
    }
    glrec([] { glEnable(GL_BLEND); });
    g_targets.pop_back(); bindTarget(g_targets.back());
    g_recStencil = stencilOn; g_recScissor = scissorOn;
    glrec([stencilOn, scissorOn] { if (stencilOn) glEnable(GL_STENCIL_TEST); if (scissorOn) glEnable(GL_SCISSOR_TEST); });
    g_stencilLevel = savedStencil;
    Mat place; place.a = 1 / ds; place.d = -1 / ds; place.tx = x0; place.ty = y0 + h / ds;
    (void)fw; (void)fh;
    if (ds != 1.f) setSmooth(a.tex, true);
    quad(a.tex, place, 0, 0, w, h, CX(), 0, 0, u1, v1);
    flush();
    // keep the result for the next frame (this entry's previous one goes back to the pool)
    FCache& fc = g_fcache[key];
    if (fc.rt.fbo) releaseRT(fc.rt);
    fc.rt = a; fc.sig = sig; fc.lastFrame = g_frameNo; fc.computed = g_frameNo; fc.w = w; fc.h = h; fc.ds = ds;
    releaseRT(b);
}
static void trimFilterCache() {   // filters no longer being drawn
    for (auto it = g_fcache.begin(); it != g_fcache.end();) {
        if (it->second.lastFrame + 2 < g_frameNo) { if (it->second.rt.fbo) releaseRT(it->second.rt); it = g_fcache.erase(it); } else ++it;
    }
}
static void execRange(size_t from, size_t to) {
    for (size_t i = from; i < to; i++) {
        const Cmd& c = g_cmds[i];
        switch (c.op) {
            case C_LEAF: {
                {
                    const SnapNode& n = g_curSnap->nodes[c.node];
                    const PlainRect& pr = n.objKind == OK_SHAPE ? plainRectOf(n.def) : PlainRect();
                    if (pr.plain && !g_noPlainRects) {
                        CX k = c.cx; uint32_t f = pr.argb;
                        k.rm *= ((f >> 16) & 255) / 255.f; k.gm *= ((f >> 8) & 255) / 255.f; k.bm *= (f & 255) / 255.f; k.am *= (f >> 24) / 255.f;
                        GLuint w = whiteTex();
                        if (w != g_batchTex) { flush(); g_batchTex = w; }
                        quad(w, g_off * c.m, pr.r.x0, pr.r.y0, pr.r.x1, pr.r.y1, k);
                        break;
                    }
                }
                TexEntry* e = rasterFor(c);
                if (!e) break;
                double inv = 1.0 / e->scale;
                quad(e->tex, g_off * c.m, e->x0 + inv, e->y0 + inv, e->x0 + (e->w - 1) * inv, e->y0 + (e->h - 1) * inv, c.cx,
                     (float)(e->px + 1) / e->texW, (float)(e->py + 1) / e->texH, (float)(e->px + e->w - 1) / e->texW, (float)(e->py + e->h - 1) / e->texH);
                break;
            }
            case C_BITMAP: {
                const SnapNode& n = g_curSnap->nodes[c.node];
                if (n.bmUid == g_sceneTargetUid) break;   // (a BitmapData is not drawn onto itself)
                auto it = g_bm.find(n.bmUid);
                if (it == g_bm.end() || !it->second.tex || it->second.w != n.bmW || it->second.h != n.bmH) break;
                GLuint t = it->second.tex;
                Mat m = g_off * c.m;
                float bx0 = 0, by0 = 0, bx1 = (float)n.bmW, by1 = (float)n.bmH;
                static const bool noBox = getenv("ISAAC_NOBMBOX") != nullptr;   // (testing)
                auto bb = noBox ? g_bmBox.end() : g_bmBox.find(n.bmUid);
                if (bb != g_bmBox.end()) {   // only the part with something drawn (2 pixels more: filtering)
                    if (bb->second.empty()) break;
                    bx0 = std::max(0.f, std::floor(bb->second.x0) - 2); by0 = std::max(0.f, std::floor(bb->second.y0) - 2);
                    bx1 = std::min((float)n.bmW, std::ceil(bb->second.x1) + 2); by1 = std::min((float)n.bmH, std::ceil(bb->second.y1) + 2);
                    if (bx1 <= bx0 || by1 <= by0) break;
                }
                if (t != g_batchTex) { flush(); setSmooth(t, n.smoothing || maxScale(m) < 0.999); g_batchTex = t; }
                quad(t, m, bx0, by0, bx1, by1, c.cx, bx0 / n.bmW, by0 / n.bmH, bx1 / n.bmW, by1 / n.bmH);
                break;
            }
            case C_SPUSH: {
                flush();
                int lv = g_stencilLevel; g_recStencil = true;
                glrec([lv] { glEnable(GL_STENCIL_TEST); glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                             glStencilFunc(GL_EQUAL, lv, 0xff); glStencilOp(GL_KEEP, GL_KEEP, GL_INCR); glUniform1f(u_alphaTest, 1.0f); });
                break;
            }
            case C_SPUSH_END: {
                flush();
                int lv = ++g_stencilLevel;
                glrec([lv] { glUniform1f(u_alphaTest, 0.0f); glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                             glStencilFunc(GL_EQUAL, lv, 0xff); glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); });
                break;
            }
            case C_SPOP: {
                flush();
                int lv = g_stencilLevel;
                glrec([lv] { glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); glStencilFunc(GL_EQUAL, lv, 0xff);
                             glStencilOp(GL_KEEP, GL_KEEP, GL_DECR); glUniform1f(u_alphaTest, 1.0f); });
                break;
            }
            case C_SPOP_END: {
                flush();
                int lv = --g_stencilLevel;
                if (lv == 0) g_recStencil = false;
                glrec([lv] { glUniform1f(u_alphaTest, 0.0f); glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                             glStencilFunc(GL_EQUAL, lv, 0xff); glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); if (lv == 0) glDisable(GL_STENCIL_TEST); });
                break;
            }
            case C_FILTER: execFilter(i); i = c.match; break;
            case C_FILTER_END: break;
        }
    }
}

// ============================================================ BitmapData layers on the GPU
// What the game draws into layers it only displays (room floor, blood, shadows, minimap, the screen
// capture for room transitions) is drawn here, into the layer's texture, with the same
// scene compiler as the screen. The logic no longer rasterizes or composites those drawings in software.
static BmTex& bmTarget(uint32_t uid, int w, int h) {
    BmTex& t = g_bm[uid];
    if (!t.tex || t.w != w || t.h != h) { if (t.tex) releaseBm(t); t = acquireBm(w, h); }
    if (!t.fbo) {
        glGenRenderbuffers(1, &t.rb); glBindRenderbuffer(GL_RENDERBUFFER, t.rb); glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
        glGenFramebuffers(1, &t.fbo); glBindFramebuffer(GL_FRAMEBUFFER, t.fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, t.tex, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, t.rb);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    return t;
}
static void beginTarget(GLuint fbo, int w, int h) {
    g_targets.clear(); g_targets.push_back({fbo, w, h, -1.f});
    bindTarget(g_targets.back());
    g_off = Mat(); g_stencilLevel = 0; g_recStencil = g_recScissor = false; g_batchTex = 0;
    glrec([] {
        glDisable(GL_SCISSOR_TEST); glDisable(GL_STENCIL_TEST); glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND); glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glStencilMask(0xff); glClearStencil(0); glClear(GL_STENCIL_BUFFER_BIT);
        glUniform1i(u_tex, 0); glUniform1f(u_alphaTest, 0.0f); glActiveTexture(GL_TEXTURE0);
    });
}
// draws a scene (subtree) into a BitmapData's texture
static void renderScene(const SnapPtr& scene, const Mat& m, const CX& cx, uint32_t uid, const BmTex& t) {
    SnapPtr savedCur = g_curSnap, savedPrev = g_prevSnap;
    std::vector<NodeInterp> savedInterp; savedInterp.swap(g_interp);
    std::vector<Cmd> savedCmds; savedCmds.swap(g_cmds);
    double savedAlpha = g_alpha, savedWait = g_renderCfg.rasterWaitMs; int savedBudget = g_rasterBudget;
    g_curSnap = scene; g_prevSnap = nullptr; g_interp.assign(scene->nodes.size(), NodeInterp()); g_alpha = 1;
    g_renderCfg.rasterWaitMs = 1e9; g_rasterBudget = 1 << 30;   // (no fallback scale here: it's drawn once)
    g_sceneTargetUid = uid;
    beginTarget(t.fbo, t.w, t.h);
    if (!scene->nodes.empty()) compileNode(0, m, cx, false);
    prerasterize();
    execRange(0, g_cmds.size());
    flush();
    g_sceneTargetUid = 0;
    g_curSnap = savedCur; g_prevSnap = savedPrev; g_interp.swap(savedInterp); g_cmds.swap(savedCmds);
    g_alpha = savedAlpha; g_renderCfg.rasterWaitMs = savedWait; g_rasterBudget = savedBudget;
}
static void runGpuOps(Snapshot& s) {
    static const bool opLog = getenv("ISAAC_GPUOPLOG") != nullptr;   // (testing)
    for (GpuOp& op : s.gpuOps) {
        Uint64 opT0 = SDL_GetPerformanceCounter();
        struct OpLogEnd { bool on; Uint64 t0; const GpuOp& op; uint32_t tick; ~OpLogEnd() { if (on) logf("[gpuop] tick %u kind %d uid %u %dx%d %.2f ms\n", tick, (int)op.kind, op.uid, op.w, op.h, (SDL_GetPerformanceCounter() - t0) * 1000.0 / SDL_GetPerformanceFrequency()); } } opEnd{opLog, opT0, op, s.tick};
        if (g_frameOpen) submitFrame();   // (what was recorded earlier goes first)
        g_bmVer[op.uid]++;
        switch (op.kind) {
            case GpuOp::UPLOAD: {
                BmTex& t = bmTarget(op.uid, op.w, op.h);
                texUpload(t.tex, 0, 0, op.w, op.h, op.px.data(), op.w);
                BmBox b;   // (where its pixels aren't transparent)
                for (int y = 0; y < op.h; y++) {
                    const uint32_t* row = op.px.data() + (size_t)y * op.w;
                    int x0 = 0; while (x0 < op.w && !(row[x0] >> 24)) x0++;
                    if (x0 == op.w) continue;
                    int x1 = op.w - 1; while (x1 > x0 && !(row[x1] >> 24)) x1--;
                    b.x0 = std::min(b.x0, (float)x0); b.x1 = std::max(b.x1, (float)x1 + 1); b.y0 = std::min(b.y0, (float)y); b.y1 = (float)y + 1;
                }
                g_bmBox[op.uid] = b;
                break;
            }
            case GpuOp::FILL: {
                BmTex& t = bmTarget(op.uid, op.w, op.h);
                beginTarget(t.fbo, t.w, t.h);
                float a = (op.color >> 24) / 255.f, r = ((op.color >> 16) & 255) / 255.f, g = ((op.color >> 8) & 255) / 255.f, b = (op.color & 255) / 255.f;
                int x0 = op.x0, y0 = op.y0, w = op.x1 - op.x0, h = op.y1 - op.y0;   // (texture row 0 = bitmap row 0)
                glrec([=] { glEnable(GL_SCISSOR_TEST); glScissor(x0, y0, w, h); glClearColor(r, g, b, a); glClear(GL_COLOR_BUFFER_BIT); glDisable(GL_SCISSOR_TEST); });
                if (a == 0 && x0 <= 0 && y0 <= 0 && op.x1 >= op.w && op.y1 >= op.h) g_bmBox[op.uid] = BmBox();   // (cleared: nothing)
                else if (a > 0) bmBoxAdd(op.uid, (float)x0, (float)y0, (float)op.x1, (float)op.y1);
                break;
            }
            case GpuOp::COPY: {
                BmTex& t = bmTarget(op.uid, op.w, op.h);
                int w = op.x1 - op.x0, h = op.y1 - op.y0;
                GLuint tmp = newTexture(w, h);
                texUpload(tmp, 0, 0, w, h, op.px.data(), w);
                beginTarget(t.fbo, t.w, t.h);
                bool merge = op.merge;
                if (!merge) glrec([] { glDisable(GL_BLEND); });
                setSmooth(tmp, false);
                Mat id; quad(tmp, id, op.x0, op.y0, op.x1, op.y1, CX());
                bmBoxAdd(op.uid, (float)op.x0, (float)op.y0, (float)op.x1, (float)op.y1);
                flush();
                if (!merge) glrec([] { glEnable(GL_BLEND); });
                submitFrame();
                glDeleteTextures(1, &tmp);
                break;
            }
            case GpuOp::DRAW: {
                for (auto& up : op.scene->uploads) {   // CPU bitmaps that appear in the drawing
                    g_bmVer[up.uid]++;
                    BmTex& bt = g_bm[up.uid];
                    if (!bt.tex || bt.w != up.w || bt.h != up.h) { if (bt.tex) releaseBm(bt); bt = acquireBm(up.w, up.h); }
                    texUpload(bt.tex, up.x0, up.y0, up.x1 - up.x0, up.y1 - up.y0, up.px.data(), up.x1 - up.x0);
                    bmBoxAdd(up.uid, (float)up.x0, (float)up.y0, (float)up.x1, (float)up.y1);
                }
                BmTex& t = bmTarget(op.uid, op.w, op.h);
                g_sceneBoxOn = true; g_sceneBox = BmBox();
                renderScene(op.scene, op.m, op.cx, op.uid, t);
                g_sceneBoxOn = false;
                if (!g_sceneBox.empty()) bmBoxAdd(op.uid, g_sceneBox.x0, g_sceneBox.y0, g_sceneBox.x1, g_sceneBox.y1);
                break;
            }
        }
        if (g_frameOpen) submitFrame();
    }
    s.gpuOps.clear(); s.gpuOps.shrink_to_fit();
}

// ============================================================ preload
// Background preload: everything is queued on the background queue and the rasterizer threads work through it
// while the game starts (the opening cutscene); each frame stores a bit of it in the atlas.
void renderPreload(const std::vector<PreKey>& keys, int winW, int winH, const std::function<void(float)>&) {
    g_winW = winW; g_winH = winH;
    rqStart();
    std::vector<RJob*> jobs;
    for (const PreKey& p : keys) {
        if (p.id >= g_movie.chars.size() || !g_movie.chars[p.id]) continue;
        const CharDef* def = g_movie.chars[p.id];
        bool ok = (p.kind == OK_SHAPE && def->type == CH_SHAPE) || (p.kind == OK_MORPH && def->type == CH_MORPH) || (p.kind == OK_TEXT && def->type == CH_TEXT);
        if (!ok) continue;
        int bucket = (int)std::ceil((p.abs ? p.q100 / 100.0 : p.q100 / 100.0 + winQ(winH)) - 1e-6);   // (like texKeyOf)
        if (bucket < -24 || bucket > 24) continue;
        // giant zooms (more than 4x the 720p reference) are transient: rasterized on demand, never preloaded
        if (!p.abs && p.q100 > 800) continue;
        TexKey k{def, p.variant, (int16_t)bucket};
        if (g_cache.count(k) || g_rqInflight.count(k)) continue;
        {   // budget: preloading never fills more than ~60% of the atlas (the rest is for what the game shows)
            RasterSrc src; src.kind = p.kind; src.def = def; src.ratio = p.variant; RectF b;
            if (!contentBoundsSrc(src, b) || !b.valid()) continue;
            double sc = std::pow(2.0, bucket / 4.0);
            double area = std::min(4096.0, (b.x1 - b.x0) * sc + 4) * std::min(4096.0, (b.y1 - b.y0) * sc + 4);
            static double budget = 0;
            if (budget == 0) budget = 0.6 * (double)std::max(4, g_renderCfg.cacheMB / 16) * kPage * kPage;
            if (area > budget) continue;
            budget -= area;
        }
        RJob* j = new RJob(); j->k = k; j->src.kind = p.kind; j->src.def = def; j->src.ratio = p.variant; j->scale = std::pow(2.0, bucket / 4.0);
        j->pre = true; j->inPreQueue = true;
        g_rqInflight[k] = j; jobs.push_back(j);
    }
    renderMarkKnown(keys);
    g_preTotal += jobs.size();
    {
        std::lock_guard<std::mutex> lk(g_rqMu);
        // (without helper threads, the render thread does it bit by bit: see renderPreloadPending)
        for (RJob* j : jobs) g_rqPreQueue.push_back(j);
    }
    g_rqCv->notify_all();
}
size_t renderPreloadPending() {
    if (g_preDone >= g_preTotal) return 0;
    if (g_rqWorkers == 0) {   // no threads: a bit per frame on this thread
        Uint64 t0 = SDL_GetPerformanceCounter(), freq = SDL_GetPerformanceFrequency();
        while ((SDL_GetPerformanceCounter() - t0) * 1000.0 / freq < 4) {
            RJob* j = nullptr;
            { std::lock_guard<std::mutex> lk(g_rqMu); if (!g_rqPreQueue.empty()) { j = g_rqPreQueue.front(); g_rqPreQueue.pop_front(); j->inPreQueue = false; } }
            if (!j) break;
            rqRun(j);
        }
    }
    rqDrain(1.5);
    return g_preTotal - g_preDone;
}
void renderLoadingScreen(int winW, int winH, float frac) {
    g_winW = winW; g_winH = winH;
    g_targets.clear(); g_targets.push_back({0, winW, winH});
    bindTarget(g_targets.back());
    glrec([] { glDisable(GL_SCISSOR_TEST); glDisable(GL_STENCIL_TEST); glEnable(GL_BLEND); glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
               glClearColor(0, 0, 0, 1); glClear(GL_COLOR_BUFFER_BIT); glUniform1i(u_tex, 0); glUniform1f(u_alphaTest, 0.0f); glActiveTexture(GL_TEXTURE0); });
    GLuint white = whiteTex(); Mat id;
    double bw = winW * 0.5, bh = std::max(6.0, winH * 0.012), x0 = (winW - bw) / 2, y0 = winH * 0.62;
    CX dim; dim.rm = dim.gm = dim.bm = 0.25f; CX lit; lit.rm = 0.85f; lit.gm = 0.80f; lit.bm = 0.72f;
    if (white != g_batchTex) { flush(); g_batchTex = white; }
    quad(white, id, x0, y0, x0 + bw, y0 + bh, dim);
    quad(white, id, x0, y0, x0 + bw * std::max(0.f, std::min(1.f, frac)), y0 + bh, lit);
    flush();
    std::vector<std::string> l = {"LOADING GRAPHICS..."};
    renderOverlay(l);
}

// ============================================================ performance overlay (F3)
#include "font5x7.inc"
// The text is drawn into a texture only when it changes (once per second) and painted with a single quad.
// (Previously it was one quad per dot of each letter: thousands per frame, costing several ms on Switch.)
static GLuint g_ovTex = 0; static int g_ovTW = 0, g_ovTH = 0, g_ovW = 0, g_ovH = 0, g_ovPx = 0;
static std::vector<std::string> g_ovLines;
void renderOverlay(const std::vector<std::string>& lines) {
    if (lines.empty()) return;
    int px = std::max(1, (g_winH + 270) / 540);   // 2 px per dot at 1080p
    if (lines != g_ovLines || px != g_ovPx || !g_ovTex) {
        g_ovLines = lines; g_ovPx = px;
        int cw = 6 * px, lh = 9 * px, pad = 2 * px;
        size_t maxLen = 0; for (auto& l : lines) maxLen = std::max(maxLen, l.size());
        int W = 2 * pad + (int)maxLen * cw, H = 2 * pad + (int)lines.size() * lh;
        std::vector<uint32_t> buf((size_t)W * H, 0xA6000000u);   // 65% black (premultiplied)
        const Glyph* table[128] = {};
        for (auto& f : kFont) table[(unsigned char)f.c] = &f;
        for (size_t li = 0; li < lines.size(); li++)
            for (size_t ci = 0; ci < lines[li].size(); ci++) {
                unsigned char c = (unsigned char)toupper((unsigned char)lines[li][ci]);
                const Glyph* g = c < 128 ? table[c] : nullptr;
                if (!g) continue;
                int x0 = pad + (int)ci * cw, y0 = pad + (int)li * lh;
                for (int r = 0; r < 7; r++) for (int k = 0; k < 5; k++) if (g->r[r][k] == '#')
                    for (int yy = 0; yy < px; yy++) for (int xx = 0; xx < px; xx++)
                        buf[(size_t)(y0 + r * px + yy) * W + x0 + k * px + xx] = 0xFFFFFF8Cu;
            }
        if (W > g_ovTW || H > g_ovTH) {
            if (g_ovTex) glDeleteTextures(1, &g_ovTex);
            g_ovTW = std::max(W, g_ovTW); g_ovTH = std::max(H, g_ovTH);
            g_ovTex = newTexture(g_ovTW, g_ovTH);
        }
        texUpload(g_ovTex, 0, 0, W, H, buf.data(), W);
        g_ovW = W; g_ovH = H;
    }
    if (g_ovTex != g_batchTex) { flush(); setSmooth(g_ovTex, false); g_batchTex = g_ovTex; }
    Mat id;
    quad(g_ovTex, id, 8, 8, 8 + g_ovW, 8 + g_ovH, CX(), 0, 0, (float)g_ovW / g_ovTW, (float)g_ovH / g_ovTH);
    flush();
    g_batchTex = 0;
}

// ============================================================ small hint text (bottom-right corner)
// e.g. "LOADING 45%" when the player touches the screen while graphics are still being preloaded.
static GLuint g_hintTex = 0; static int g_hintTW = 0, g_hintTH = 0, g_hintW = 0, g_hintH = 0; static std::string g_hintKey;
void renderHint(const std::string& text) {
    if (text.empty()) return;
    int px = std::max(2, (g_winH + 180) / 360);   // 3 px per dot at 1080p
    std::string key = text + "|" + std::to_string(px);
    if (key != g_hintKey || !g_hintTex) {
        g_hintKey = key;
        int cw = 6 * px, pad = 3 * px;
        int W = 2 * pad + (int)text.size() * cw, H = 2 * pad + 7 * px;
        std::vector<uint32_t> buf((size_t)W * H, 0x99000000u);   // 60% black (premultiplied)
        const Glyph* table[128] = {};
        for (auto& f : kFont) table[(unsigned char)f.c] = &f;
        for (size_t ci = 0; ci < text.size(); ci++) {
            unsigned char c = (unsigned char)toupper((unsigned char)text[ci]);
            const Glyph* g = c < 128 ? table[c] : nullptr; if (!g) continue;
            int x0 = pad + (int)ci * cw, y0 = pad;
            for (int r = 0; r < 7; r++) for (int k = 0; k < 5; k++) if (g->r[r][k] == '#')
                for (int yy = 0; yy < px; yy++) for (int xx = 0; xx < px; xx++) buf[(size_t)(y0 + r * px + yy) * W + x0 + k * px + xx] = 0xFFFFFFFFu;
        }
        if (W > g_hintTW || H > g_hintTH) {
            if (g_hintTex) glDeleteTextures(1, &g_hintTex);
            g_hintTW = std::max(W, g_hintTW); g_hintTH = std::max(H, g_hintTH); g_hintTex = newTexture(g_hintTW, g_hintTH);
        }
        texUpload(g_hintTex, 0, 0, W, H, buf.data(), W);
        g_hintW = W; g_hintH = H;
    }
    if (g_hintTex != g_batchTex) { flush(); setSmooth(g_hintTex, false); g_batchTex = g_hintTex; }
    Mat id; int m = std::max(8, g_winH / 40);
    quad(g_hintTex, id, g_winW - m - g_hintW, g_winH - m - g_hintH, g_winW - m, g_winH - m, CX(), 0, 0, (float)g_hintW / g_hintTW, (float)g_hintH / g_hintTH);
    flush();
    g_batchTex = 0;
}

// ============================================================ touch joysticks (Android)
// Two anti-aliased circles generated once (a ring for the base, a disc for the knob), drawn on top of the
// frame in window pixels with a low alpha: discreet, and only while a finger is down.
static GLuint g_stickTex = 0;
void renderTouchSticks(const TouchStick* sticks, int n) {
    if (!g_stickTex) {
        const int S = 256; std::vector<uint32_t> buf((size_t)S * 2 * S, 0);
        for (int y = 0; y < S; y++) for (int x = 0; x < S; x++) {
            double dx = x + 0.5 - S / 2.0, dy = y + 0.5 - S / 2.0, r = std::sqrt(dx * dx + dy * dy), R = S / 2.0 - 2;
            auto put = [&](int col, double a, uint32_t rgb) {
                a = std::max(0.0, std::min(1.0, a)); uint32_t A = (uint32_t)(a * 255 + 0.5);
                uint32_t rr = ((rgb >> 16) & 255) * A / 255, gg = ((rgb >> 8) & 255) * A / 255, bb = (rgb & 255) * A / 255;
                buf[(size_t)y * 2 * S + col * S + x] = A << 24 | rr << 16 | gg << 8 | bb;
            };
            // base: thick ring with a faint fill
            double ring = std::min(r - (R - 10), R - r) + 0.5, edge = R - r + 0.5;
            put(0, std::max(std::min(1.0, ring), 0.0) * 0.9 + std::max(0.0, std::min(1.0, edge)) * 0.25 * (r < R - 10), 0xFFFFFF);
            // knob: solid disc
            put(1, R - r + 0.5, 0xFFFFFF);
        }
        g_stickTex = newTexture(2 * S, S);
        texUpload(g_stickTex, 0, 0, 2 * S, S, buf.data(), 2 * S);
        glBindTexture(GL_TEXTURE_2D, g_stickTex); glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
    bool any = false; for (int i = 0; i < n; i++) any |= sticks[i].active;
    if (!any) return;
    if (g_stickTex != g_batchTex) { flush(); g_batchTex = g_stickTex; }
    Mat id;
    for (int i = 0; i < n; i++) {
        const TouchStick& t = sticks[i]; if (!t.active) continue;
        CX base; base.am = 0.30f; CX knob; knob.am = 0.40f;
        quad(g_stickTex, id, t.cx - t.r, t.cy - t.r, t.cx + t.r, t.cy + t.r, base, 0, 0, 0.5f, 1);
        double kr = t.r * 0.45;
        quad(g_stickTex, id, t.kx - kr, t.ky - kr, t.kx + kr, t.ky + kr, knob, 0.5f, 0, 1, 1);
    }
    flush();
    g_batchTex = 0;
}
// pause button for touch screens (window pixels): rounded dark square with two bars
static GLuint g_pauseTex = 0;
void renderPauseButton(double x0, double y0, double x1, double y1) {
    if (!g_pauseTex) {
        const int S = 128; std::vector<uint32_t> buf((size_t)S * S, 0);
        auto cov = [](double d) { return std::max(0.0, std::min(1.0, d + 0.5)); };   // signed distance -> coverage
        for (int y = 0; y < S; y++) for (int x = 0; x < S; x++) {
            double px = x + 0.5, py = y + 0.5;
            // rounded square (radius 26), 92% of the cell
            double h = S * 0.46, rr = 26, qx = std::fabs(px - S / 2.0) - (h - rr), qy = std::fabs(py - S / 2.0) - (h - rr);
            double box = -(std::hypot(std::max(qx, 0.0), std::max(qy, 0.0)) + std::min(std::max(qx, qy), 0.0) - rr);
            // two bars 18 wide, 56 tall, 16 apart
            auto bar = [&](double cx) { double bx = std::fabs(px - cx) - 9, by = std::fabs(py - S / 2.0) - 28; return -(std::max(bx, by)); };
            double bars = std::max(bar(S / 2.0 - 17), bar(S / 2.0 + 17));
            double a = cov(box) * 0.55, w = cov(bars);
            double outA = w + a * (1 - w);                     // white bars over a 55% black square
            uint32_t A = (uint32_t)(outA * 255 + 0.5), c = (uint32_t)(w * 255 + 0.5);
            buf[(size_t)y * S + x] = A << 24 | c << 16 | c << 8 | c;
        }
        g_pauseTex = newTexture(S, S);
        texUpload(g_pauseTex, 0, 0, S, S, buf.data(), S);
        glBindTexture(GL_TEXTURE_2D, g_pauseTex); glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
    if (g_pauseTex != g_batchTex) { flush(); g_batchTex = g_pauseTex; }
    Mat id; CX cx; cx.am = 0.8f;
    quad(g_pauseTex, id, x0, y0, x1, y1, cx);
    flush();
    g_batchTex = 0;
}

// ============================================================ game selection (several versions available)
// Simple screen using the same 5x7 font: title, one row per game (the selected one highlighted) and a hint.
// Composed on the CPU only when something changes and painted with one quad.
static GLuint g_chTex = 0; static int g_chTW = 0, g_chTH = 0; static std::string g_chKey;
void renderChooser(int winW, int winH, const std::string& title, const std::vector<std::string>& items, int sel,
                   const std::string& hint, std::vector<std::array<int, 4>>* rects) {
    g_winW = winW; g_winH = winH;
    g_targets.clear(); g_targets.push_back({0, winW, winH});
    bindTarget(g_targets.back());
    glrec([] { glDisable(GL_SCISSOR_TEST); glDisable(GL_STENCIL_TEST); glEnable(GL_BLEND); glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
               glClearColor(0, 0, 0, 1); glClear(GL_COLOR_BUFFER_BIT); glUniform1i(u_tex, 0); glUniform1f(u_alphaTest, 0.0f); glActiveTexture(GL_TEXTURE0); });
    int px = std::max(2, winH / 180);            // dot size for the options (6 px at 1080p)
    int tpx = px + px / 2, hpx = std::max(1, px / 2 + 1);
    int lh = 9 * px, rowH = lh + 6 * px;
    int titleY = winH / 5, listY = titleY + 9 * tpx + 10 * px;
    std::vector<std::array<int, 4>> rc;
    size_t maxLen = 0; for (auto& it : items) maxLen = std::max(maxLen, it.size());
    for (size_t i = 0; i < items.size(); i++) {
        int w = (int)maxLen * 6 * px + 12 * px;
        int x0 = (winW - w) / 2, y0 = listY + (int)i * rowH;
        rc.push_back({x0, y0, x0 + w, y0 + rowH - 2 * px});
    }
    if (rects) *rects = rc;
    char kb[64]; snprintf(kb, sizeof kb, "%dx%d|%d|", winW, winH, sel);
    std::string key = kb + title + "|" + hint; for (auto& it : items) key += "|" + it;
    if (key != g_chKey || !g_chTex) {
        g_chKey = key;
        std::vector<uint32_t> buf((size_t)winW * winH, 0);
        const Glyph* table[128] = {};
        for (auto& f : kFont) table[(unsigned char)f.c] = &f;
        auto text = [&](const std::string& str, int x0, int y0, int p, uint32_t col) {
            for (size_t ci = 0; ci < str.size(); ci++) {
                unsigned char c = (unsigned char)toupper((unsigned char)str[ci]);
                const Glyph* g = c < 128 ? table[c] : nullptr; if (!g) continue;
                for (int r = 0; r < 7; r++) for (int k = 0; k < 5; k++) if (g->r[r][k] == '#')
                    for (int yy = 0; yy < p; yy++) for (int xx = 0; xx < p; xx++) {
                        int X = x0 + (int)ci * 6 * p + k * p + xx, Y = y0 + r * p + yy;
                        if (X >= 0 && Y >= 0 && X < winW && Y < winH) buf[(size_t)Y * winW + X] = col;
                    }
            }
        };
        auto box = [&](const std::array<int, 4>& r, uint32_t col) {
            for (int y = std::max(0, r[1]); y < std::min(winH, r[3]); y++) for (int x = std::max(0, r[0]); x < std::min(winW, r[2]); x++) buf[(size_t)y * winW + x] = col;
        };
        text(title, (winW - (int)title.size() * 6 * tpx) / 2, titleY, tpx, 0xFFE8DCC8u);
        for (size_t i = 0; i < items.size(); i++) {
            bool on = (int)i == sel;
            if (on) box(rc[i], 0xFF5A1A14u);   // dark red, like the game's menus
            int tw = (int)items[i].size() * 6 * px;
            text(items[i], (rc[i][0] + rc[i][2] - tw) / 2, rc[i][1] + (rc[i][3] - rc[i][1] - 7 * px) / 2, px, on ? 0xFFFFFFFFu : 0xFF8C8C8Cu);
        }
        if (!hint.empty()) text(hint, (winW - (int)hint.size() * 6 * hpx) / 2, winH - winH / 8, hpx, 0xFF707070u);
        if (winW > g_chTW || winH > g_chTH) {
            if (g_chTex) glDeleteTextures(1, &g_chTex);
            g_chTW = std::max(winW, g_chTW); g_chTH = std::max(winH, g_chTH);
            g_chTex = newTexture(g_chTW, g_chTH);
        }
        texUpload(g_chTex, 0, 0, winW, winH, buf.data(), winW);
    }
    if (g_chTex != g_batchTex) { flush(); setSmooth(g_chTex, false); g_batchTex = g_chTex; }
    Mat id;
    quad(g_chTex, id, 0, 0, winW, winH, CX(), 0, 0, (float)winW / g_chTW, (float)winH / g_chTH);
    flush();
    g_batchTex = 0;
}

// ============================================================ frame
// The visible part of the stage (snapshot view, WideSaac): fitted to the window and centered. Changes are eased over
// a few frames (pausing with Hyper Zoom shows the whole stage again). Default: the whole stage, letterboxed.
static double g_viewCur[4] = {0, 0, 0, 0}; static bool g_viewClip = true; static Uint64 g_viewT = 0;
static void updateView() {
    double tgt[4] = {0, 0, (double)g_stageW, (double)g_stageH}; bool clip = true;
    static const char* env = getenv("ISAAC_VIEW");   // (testing) "x0 y0 x1 y1 clip"
    if (env) { int c = 1; sscanf(env, "%lf %lf %lf %lf %d", &tgt[0], &tgt[1], &tgt[2], &tgt[3], &c); clip = c != 0; }
    else if (g_curSnap && g_curSnap->viewSet && g_curSnap->view[2] > g_curSnap->view[0] && g_curSnap->view[3] > g_curSnap->view[1]) {
        for (int i = 0; i < 4; i++) tgt[i] = g_curSnap->view[i];
        clip = g_curSnap->viewClip;
    }
    Uint64 now = SDL_GetPerformanceCounter();
    double dt = g_viewT ? (double)(now - g_viewT) / SDL_GetPerformanceFrequency() : 1.0;
    g_viewT = now;
    if (g_viewCur[2] <= g_viewCur[0]) for (int i = 0; i < 4; i++) g_viewCur[i] = tgt[i];
    double k = 1 - std::exp(-std::min(dt, 0.25) * 14), err = 0;
    for (int i = 0; i < 4; i++) { g_viewCur[i] += (tgt[i] - g_viewCur[i]) * k; err = std::max(err, std::fabs(tgt[i] - g_viewCur[i])); }
    if (err < 0.5) for (int i = 0; i < 4; i++) g_viewCur[i] = tgt[i];
    g_viewClip = clip && err < 0.5;   // (while easing, nothing is cut)
}
Mat stageToWindowFor(int winW, int winH) {
    double x0 = g_viewCur[0], y0 = g_viewCur[1], w = g_viewCur[2] - g_viewCur[0], h = g_viewCur[3] - g_viewCur[1];
    if (!(w > 0 && h > 0)) { x0 = y0 = 0; w = g_stageW; h = g_stageH; }
    double sc = std::min((double)winW / w, (double)winH / h);
    Mat stage; stage.a = stage.d = sc; stage.tx = (winW - w * sc) / 2 - x0 * sc; stage.ty = (winH - h * sc) / 2 - y0 * sc;
    return stage;
}
void renderFrame(int winW, int winH, double alpha) {
    g_filterRecomputes = 0;
    g_frameNo++; g_winW = winW; g_winH = winH;
    if (winH > 0) g_windowAspect = (float)winW / winH;
    g_alpha = g_renderCfg.smooth ? std::max(0.0, std::min(1.35, alpha)) : 1.0;
    g_rasterBudget = g_renderCfg.rasterPerFrame;
    if (g_frameOpen) renderEndFrame();   // (in case the caller didn't)
    trimFilterCache();
    glBindBuffer(GL_ARRAY_BUFFER, g_vboRing.buf);
    evictBig();
    g_targets.clear(); g_targets.push_back({0, winW, winH});
    bindTarget(g_targets.back());
    g_recStencil = g_recScissor = false;
    glrec([] {
        glDisable(GL_DEPTH_TEST); glDisable(GL_STENCIL_TEST); glDisable(GL_SCISSOR_TEST); glEnable(GL_BLEND); glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(0, 0, 0, 1); glClearStencil(0); glStencilMask(0xff); glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glUniform1i(u_tex, 0); glUniform1f(u_alphaTest, 0.0f);
        glActiveTexture(GL_TEXTURE0);
    });
    updateView();
    Mat stage = stageToWindowFor(winW, winH);
    g_stageToWindow = stage;
    if (!g_curSnap) return;
    bool full = !g_viewClip || g_viewCur[0] != 0 || g_viewCur[1] != 0 || g_viewCur[2] != g_stageW || g_viewCur[3] != g_stageH;
    // the area drawn: the whole stage (default), or the view (clipped) / the whole window (not clipped)
    double ax0 = 0, ay0 = 0, ax1 = g_stageW, ay1 = g_stageH;
    if (full) {
        if (g_viewClip) { ax0 = g_viewCur[0]; ay0 = g_viewCur[1]; ax1 = g_viewCur[2]; ay1 = g_viewCur[3]; }
        else { Mat inv; stage.invert(inv); inv.apply(0, 0, ax0, ay0); inv.apply(winW, winH, ax1, ay1); }
    }
    if (g_curSnap->viewCut && !getenv("ISAAC_VIEW")) {   // (black around it: the clear colour)
        ax0 = std::max(ax0, (double)g_curSnap->cut[0]); ay0 = std::max(ay0, (double)g_curSnap->cut[1]);
        ax1 = std::min(ax1, (double)g_curSnap->cut[2]); ay1 = std::min(ay1, (double)g_curSnap->cut[3]);
        if (ax1 < ax0) ax1 = ax0;
        if (ay1 < ay0) ay1 = ay0;
    }
    {
        uint32_t bg = g_curSnap->viewCut ? 0 : g_curSnap->bgColor; CX c;   // (cut: black, like around it) c.rm = ((bg >> 16) & 255) / 255.f; c.gm = ((bg >> 8) & 255) / 255.f; c.bm = (bg & 255) / 255.f;
        quad(whiteTex(), stage, ax0, ay0, ax1, ay1, c);
    }
    flush();
    {
        double wx0, wy0, wx1, wy1; stage.apply(ax0, ay0, wx0, wy0); stage.apply(ax1, ay1, wx1, wy1);
        wx0 = std::max(0.0, wx0); wy0 = std::max(0.0, wy0); wx1 = std::min((double)winW, wx1); wy1 = std::min((double)winH, wy1);
        int sx = (int)wx0, sy = (int)(winH - wy1), sw = (int)std::ceil(wx1 - wx0), sh = (int)std::ceil(wy1 - wy0);
        g_recScissor = true;
        glrec([sx, sy, sw, sh] { glEnable(GL_SCISSOR_TEST); glScissor(sx, sy, sw, sh); });
    }
    g_stencilLevel = 0; g_off = Mat();
    g_cmds.clear();
    Uint64 t0 = SDL_GetPerformanceCounter();
    if (!g_curSnap->nodes.empty()) compileNode(0, stage, CX(), false);
    g_rstats.compileMs += msSince(t0); t0 = SDL_GetPerformanceCounter();
    g_rstats.nodes = (int)g_curSnap->nodes.size(); g_rstats.cmds = (int)g_cmds.size();
    prerasterize();
    g_rstats.prerasterMs += msSince(t0); t0 = SDL_GetPerformanceCounter();
    g_rasterBudget = 0;
    execRange(0, g_cmds.size());
    flush();
    g_recScissor = false;
    glrec([] { glDisable(GL_SCISSOR_TEST); });
    g_rstats.execMs += msSince(t0);
    g_batchTex = 0;
    static const bool qlog = getenv("ISAAC_QUADLOG") != nullptr;
    if (qlog) { static double tot = 0; static int n = 0; tot += g_fillPx; g_fillPx = 0; if (++n % 100 == 0) logf("[fill] %.1f window-areas per frame\n", tot / n / ((double)g_winW * g_winH)); }
}

void renderEndFrame() {
    Uint64 t0 = SDL_GetPerformanceCounter();
    submitFrame();
    g_vboRing.endFrame(); g_pboRing.endFrame();
    g_frameOpen = false;
    g_rstats.execMs += msSince(t0);
}

bool renderScreenshot(const char* path) {
    std::vector<uint8_t> px((size_t)g_winW * g_winH * 4);
    glReadPixels(0, 0, g_winW, g_winH, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
    std::string p = path;
    if (p.size() > 4 && p.substr(p.size() - 4) == ".bmp") {
        SDL_Surface* s = SDL_CreateRGBSurfaceWithFormat(0, g_winW, g_winH, 24, SDL_PIXELFORMAT_RGB24);
        if (!s) return false;
        for (int y = 0; y < g_winH; y++) for (int x = 0; x < g_winW; x++) {
            const uint8_t* src = &px[((size_t)(g_winH - 1 - y) * g_winW + x) * 4];
            uint8_t* dst = (uint8_t*)s->pixels + y * s->pitch + x * 3; dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2];
        }
        bool ok = SDL_SaveBMP(s, path) == 0; SDL_FreeSurface(s); return ok;
    }
    FILE* f = fopen(path, "wb"); if (!f) return false;
    fprintf(f, "P6\n%d %d\n255\n", g_winW, g_winH);
    for (int y = g_winH - 1; y >= 0; y--) for (int x = 0; x < g_winW; x++) fwrite(&px[((size_t)y * g_winW + x) * 4], 1, 3, f);
    fclose(f); return true;
}

}  // namespace fl
