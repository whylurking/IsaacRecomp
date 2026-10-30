// PSP: large blocks (64 KB and more: BitmapData pixels, rasterized shapes, atlas pages, sound data, the SWF) come from
// their own arena instead of newlib's heap. Mixed with the many small, long-lived objects of the game, they left the
// heap so fragmented that a 300 KB request failed with 11 MB free. Linked with -Wl,--wrap for malloc, free, realloc,
// calloc and memalign (C and C++ code alike; libc's internal _malloc_r calls stay in newlib's heap, they are small).
// The arena is one block taken from the heap at startup (pspAllocInit): best fit over an address-ordered free list,
// neighbours merged when freed, 64-byte aligned (cache lines, GE textures).
#ifdef __PSP__
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <pspthreadman.h>
#include <malloc.h>

extern "C" {
void* __real_malloc(size_t);
void __real_free(void*);
void* __real_realloc(void*, size_t);
void* __real_calloc(size_t, size_t);
void* __real_memalign(size_t, size_t);
}

namespace {
constexpr size_t kBig = 64 << 10, kAlign = 64;
struct Blk { size_t size; Blk* next; bool free; };   // header, kAlign bytes before the user pointer (size includes it)
uint8_t* g_lo = nullptr; uint8_t* g_hi = nullptr;
Blk* g_first = nullptr;                              // every block, in address order
size_t g_used = 0, g_peak = 0;
SceUID g_sem = -1;
struct Lock { Lock() { if (g_sem >= 0) sceKernelWaitSema(g_sem, 1, nullptr); } ~Lock() { if (g_sem >= 0) sceKernelSignalSema(g_sem, 1); } };
inline bool inArena(const void* p) { return p >= g_lo && p < g_hi; }
inline Blk* hdr(void* p) { return (Blk*)((uint8_t*)p - kAlign); }

void* arenaAlloc(size_t n) {
    size_t need = (n + kAlign + kAlign - 1) & ~(kAlign - 1);
    Lock lk;
    Blk* best = nullptr;
    for (Blk* b = g_first; b; b = b->next) if (b->free && b->size >= need && (!best || b->size < best->size)) best = b;
    if (!best) return nullptr;
    if (best->size - need >= kBig / 4) {   // split: the rest stays free
        Blk* r = (Blk*)((uint8_t*)best + need);
        r->size = best->size - need; r->next = best->next; r->free = true;
        best->size = need; best->next = r;
    }
    best->free = false;
    g_used += best->size; if (g_used > g_peak) g_peak = g_used;
    return (uint8_t*)best + kAlign;
}
void arenaFree(void* p) {
    Lock lk;
    Blk* b = hdr(p); b->free = true; g_used -= b->size;
    // merge with the following free block, then with the previous one
    if (b->next && b->next->free) { b->size += b->next->size; b->next = b->next->next; }
    Blk* prev = nullptr; for (Blk* x = g_first; x && x != b; x = x->next) prev = x;
    if (prev && prev->free) { prev->size += b->size; prev->next = b->next; }
}
}  // namespace

namespace fl {
// takes `bytes` of the heap for the arena (called once, before anything large is allocated)
void pspAllocInit(size_t bytes) {
    uint8_t* m = (uint8_t*)__real_memalign(kAlign, bytes);
    if (!m) return;
    g_sem = sceKernelCreateSema("bigalloc", 0, 1, 1, nullptr);
    g_lo = m; g_hi = m + (bytes & ~(kAlign - 1));
    g_first = (Blk*)g_lo; g_first->size = g_hi - g_lo; g_first->next = nullptr; g_first->free = true;
}
// the largest block newlib's heap can still give (binary search; done once, at startup)
size_t pspHeapLargestFree() {
    size_t lo = 0, hi = 64u << 20;
    while (hi - lo > (256u << 10)) { size_t mid = (lo + hi) / 2; void* p = __real_malloc(mid); if (p) { __real_free(p); lo = mid; } else hi = mid; }
    return lo;
}
void pspAllocStats(size_t* used, size_t* peak, size_t* size, size_t* largestFree) {
    Lock lk;
    size_t lf = 0; for (Blk* b = g_first; b; b = b->next) if (b->free && b->size > lf) lf = b->size;
    *used = g_used; *peak = g_peak; *size = g_hi - g_lo; *largestFree = lf;
}
}

extern "C" {
void* __wrap_malloc(size_t n) {
    if (n >= kBig && g_lo) if (void* p = arenaAlloc(n)) return p;
    return __real_malloc(n);
}
void* __wrap_memalign(size_t a, size_t n) {
    if (n >= kBig && g_lo && a <= kAlign) if (void* p = arenaAlloc(n)) return p;
    return __real_memalign(a, n);
}
void __wrap_free(void* p) {
    if (!p) return;
    if (inArena(p)) arenaFree(p); else __real_free(p);
}
void* __wrap_calloc(size_t c, size_t n) {
    size_t t = c * n;
    if (t >= kBig && g_lo) if (void* p = arenaAlloc(t)) { memset(p, 0, t); return p; }
    return __real_calloc(c, n);
}
void* __wrap_realloc(void* p, size_t n) {
    if (!p) return __wrap_malloc(n);
    if (!n) { __wrap_free(p); return nullptr; }
    if (!inArena(p) && n < kBig) return __real_realloc(p, n);
    size_t old = inArena(p) ? hdr(p)->size - kAlign : n;   // (newlib block: its size isn't known here, copy n at most)
    if (inArena(p) && old >= n) return p;
    void* q = __wrap_malloc(n);
    if (!q) return nullptr;
    if (inArena(p)) memcpy(q, p, old < n ? old : n);
    else { size_t on = malloc_usable_size(p); memcpy(q, p, on < n ? on : n); }
    __wrap_free(p);
    return q;
}
}
#endif
