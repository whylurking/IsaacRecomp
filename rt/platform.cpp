#include "platform.h"
#include "core.h"
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <sys/stat.h>
#include <dirent.h>
#endif
#include <cerrno>
#include <cstring>
#include <exception>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <vector>
#include <algorithm>
#ifdef __SWITCH__
#include <switch.h>
#endif
#ifdef __ANDROID__
#include <sched.h>
#include <unistd.h>
#include <cstdio>
#endif
namespace fl {
static const Limits kBig   = {400000, 192u << 20, 96u << 20, 160u << 20, 16, 48u << 20, 3, 768, 2.0, false};
static const Limits kSmall = {150000, 128u << 20, 32u << 20,  48u << 20,  6, 24u << 20, 2, 160, 3.0, true};
Limits g_lim = kBig;
void platformInitLimits() {
#if defined(__SWITCH__)
    int used, total; platformMemInfo(&used, &total);
    g_lim = total >= 1500 ? kBig : kSmall;
    if (!g_lim.small) g_lim.pboBytes = 32u << 20;   // (application mode: ~3 GB; atlas as on PC: the preload fits)
    g_lim.gcSweepMs = 3.0;
#elif defined(__ANDROID__)
    // phones share RAM with the GPU and the system kills apps that use too much: atlas like the Switch's
    // application mode (the preload needs ~25 pages of 16 MB), smaller upload ring
    g_lim = kBig;
    g_lim.defaultCacheMB = 512;
    g_lim.pboBytes = 32u << 20;
#elif defined(__PSP__)
    // PSP (2000 and later): ~50 MB for everything. No GPU-side layers, a few atlas pages (see render_psp.cpp)
    g_lim = {15000, 4u << 20, 1u << 20, 2u << 20, 0, 0, 0, 6, 2.0, true, 1.4};   // (objects ~200 bytes: collect early)
#elif defined(FL_SWITCH_MEMORY)
    g_lim = kSmall;
#else
    g_lim = kBig;
#endif
#if defined(FL_LEAN) && !defined(__PSP__)
    if (getenv("ISAAC_PSPLIMITS")) g_lim = {15000, 4u << 20, 1u << 20, 1u << 20, 0, 0, 0, 6, 2.0, true, 1.4};   // (tests: the PSP's)
#endif
}

#ifdef __ANDROID__
// Phones mix fast ("big") and slow ("little") cores. Logic and rendering go on the fastest ones; rasterization
// helpers on the rest, so they never take a big core away from the two threads that set the frame rate.
namespace {
struct CpuSets { int n = 0; cpu_set_t big, little; int nBig = 0, nLittle = 0; };
const CpuSets& cpuSets() {
    static CpuSets cs = [] {
        CpuSets c; CPU_ZERO(&c.big); CPU_ZERO(&c.little);
        int n = (int)sysconf(_SC_NPROCESSORS_CONF); if (n < 1) n = 1; if (n > 32) n = 32; c.n = n;
        long freq[32] = {0}, top = 0;
        for (int i = 0; i < n; i++) {
            char p[96]; snprintf(p, sizeof p, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq", i);
            if (FILE* f = fopen(p, "r")) { if (fscanf(f, "%ld", &freq[i]) != 1) freq[i] = 0; fclose(f); }
            top = std::max(top, freq[i]);
        }
        for (int i = 0; i < n; i++) {
            if (top > 0 && freq[i] >= top * 0.9) { CPU_SET(i, &c.big); c.nBig++; } else { CPU_SET(i, &c.little); c.nLittle++; }
        }
        // a lone prime core (1+3+4 layouts) is not enough for two threads: take the next tier too
        if (c.nBig == 1 && c.nLittle > 0) {
            long second = 0; for (int i = 0; i < n; i++) if (freq[i] < top * 0.9) second = std::max(second, freq[i]);
            for (int i = 0; i < n; i++) if (freq[i] < top * 0.9 && freq[i] >= second * 0.95) { CPU_SET(i, &c.big); CPU_CLR(i, &c.little); c.nBig++; c.nLittle--; }
        }
        if (c.nLittle == 0 || c.nBig == 0) { c.nBig = c.nLittle = 0; }   // all alike (or unreadable): no pinning
        logf("cpus: %d (fast %d, other %d)\n", n, c.nBig, c.nLittle);
        return c;
    }();
    return cs;
}
}
#endif

// Layout on Switch (the application has cores 0-2; priority: lower number = more urgent):
//   core 0: render (main thread, 0x2C) and, below it, one render rasterizer and one logic rasterizer
//   core 1: logic (0x2C), alone
//   core 2: render rasterizer (0x2D) and, below it, the logic one (0x3B)
// This way the renderer never waits for the logic to release a core (Horizon does not time-slice between
// threads of the same priority: one does not progress until the other blocks).
int platformPoolWorkers(ThreadRole role) {
#if defined(__PSP__)
    (void)role; return 0;   // one core: everything on the calling thread
#elif defined(__SWITCH__)
    return role == ROLE_LOGIC ? 2 : 1;
#elif defined(__ANDROID__)
    (void)role;
    const CpuSets& c = cpuSets();
    if (c.nLittle) return std::max(1, std::min(c.nLittle - 1, 4));
    unsigned hc = std::thread::hardware_concurrency();
    return hc > 2 ? (int)std::min(hc - 2, 4u) : 1;
#else
    (void)role;
    unsigned hc = std::thread::hardware_concurrency();
    return hc > 1 ? (int)std::min(hc - 1, 7u) : 0;
#endif
}
void platformPoolWorkerPlace(ThreadRole role, int i, int* core, int* prio) {
    if (role == ROLE_LOGIC) { *core = i == 0 ? 2 : 0; *prio = i == 0 ? 0x3B : 0x3D; }
    else { *core = 2; *prio = 0x2D; }
}
int platformRasterServiceWorkers() {
#if defined(__PSP__)
    return 0;
#elif defined(__SWITCH__)
    return 2;
#elif defined(__ANDROID__)
    const CpuSets& c = cpuSets();
    if (c.nLittle) return std::max(1, std::min(c.nLittle, 4));
    unsigned hc = std::thread::hardware_concurrency();
    return hc > 2 ? (int)std::min(hc - 2, 4u) : 1;
#else
    unsigned hc = std::thread::hardware_concurrency();
    return hc > 1 ? (int)std::min(hc - 1, 4u) : 1;
#endif
}
void platformRasterServicePlace(int i, int* core, int* prio) { *core = i == 0 ? 2 : 0; *prio = i == 0 ? 0x2D : 0x3C; }
void platformSetThread(int core, int prio) {
#ifdef __SWITCH__
    if (core >= 0) svcSetThreadCoreMask(CUR_THREAD_HANDLE, core, 1u << core);
    if (prio >= 0) svcSetThreadPriority(CUR_THREAD_HANDLE, prio);
#elif defined(__ANDROID__)
    // (the Switch priorities double as roles: 0x2C = logic / render, anything else = helper)
    (void)core;
    const CpuSets& c = cpuSets();
    if (!c.nBig) return;
    cpu_set_t set = prio == 0x2C ? c.big : c.little;
    sched_setaffinity(0, sizeof set, &set);
#else
    (void)core; (void)prio;
#endif
}
void platformPinThread(int core) {
#ifdef __SWITCH__
    // cores 0-2 for the application: 0 = render (main thread), 1 = logic, 2 = rasterization
    svcSetThreadCoreMask(CUR_THREAD_HANDLE, core, 1u << core);
#else
    (void)core;
#endif
}

#ifdef __SWITCH__
namespace {
struct StartArgs { void (*fn)(void*); void* arg; };
void threadEntry(void* a) { StartArgs sa = *(StartArgs*)a; delete (StartArgs*)a; sa.fn(sa.arg); }
}
#endif

static std::atomic<bool> g_stopping{false};
static std::mutex g_thMu;
static std::vector<std::function<void()>> g_wakes;
#ifdef __SWITCH__
static std::vector<Thread*> g_threads;
#endif
bool platformStopping() { return g_stopping.load(std::memory_order_relaxed); }
void platformOnStop(std::function<void()> wake) { std::lock_guard<std::mutex> lk(g_thMu); g_wakes.push_back(std::move(wake)); }
void platformStopThreads() {
    g_stopping = true;
    std::vector<std::function<void()>> w; { std::lock_guard<std::mutex> lk(g_thMu); w = g_wakes; }
    for (auto& f : w) f();
#ifdef __SWITCH__
    std::vector<Thread*> ts; { std::lock_guard<std::mutex> lk(g_thMu); ts.swap(g_threads); }
    for (Thread* t : ts) {
        threadWaitForExit(t);   // (each thread finishes the work at hand and exits)
        threadClose(t); delete t;
    }
    logf("helper threads stopped: %zu\n", ts.size());
#endif
}

bool platformStartThread(void (*fn)(void*), void* arg, int core, const char* name) {
#ifdef __SWITCH__
    // directly with libnx (not std::thread): this way a failure can be logged with its code and we can
    // continue with fewer threads instead of terminating the program. Priority 0x3B = that of libnx pthreads
    // (below the main thread), 256 KB stack.
    Thread* t = new Thread();
    StartArgs* sa = new StartArgs{fn, arg};
    Result rc = threadCreate(t, threadEntry, sa, nullptr, 256 * 1024, 0x3B, -2);   // the thread sets its own core
    if (R_SUCCEEDED(rc)) {
        rc = threadStart(t);
        if (R_FAILED(rc)) threadClose(t);
        else { std::lock_guard<std::mutex> lk(g_thMu); g_threads.push_back(t); }
    }
    if (R_FAILED(rc)) {
        int used, total; platformMemInfo(&used, &total);
        logf("could not create thread '%s' (core %d): result 0x%x (%d-%04d); memory %d/%d MB\n",
             name, core, rc, 2000 + R_MODULE(rc), R_DESCRIPTION(rc), used, total);
        delete sa; delete t;
        return false;
    }
    return true;
#else
    (void)core;
    try {
        std::thread([fn, arg] { fn(arg); }).detach();
        return true;
    } catch (const std::exception& e) {
        logf("could not create thread '%s': %s\n", name, e.what());
        return false;
    }
#endif
}

namespace {
struct AsyncWriter {
    std::mutex mu; std::condition_variable cv; std::vector<std::pair<std::string, std::string>> q; bool started = false;
};
AsyncWriter* g_writer = new AsyncWriter;   // (never destroyed)
}
void platformAppendFileAsync(const std::string& path, const std::string& text) {
    {
        std::lock_guard<std::mutex> lk(g_writer->mu);
        g_writer->q.push_back({path, text});
        if (!g_writer->started) {
            g_writer->started = true;
            platformStartThread([](void*) {
                platformSetThread(0, 0x3F);   // (Switch: core 0, lowest priority)
                for (;;) {
                    std::vector<std::pair<std::string, std::string>> batch;
                    { std::unique_lock<std::mutex> lk(g_writer->mu); g_writer->cv.wait(lk, [] { return !g_writer->q.empty() || platformStopping(); }); batch.swap(g_writer->q); }
                    if (batch.empty() && platformStopping()) return;
                    for (size_t i = 0; i < batch.size(); i++) {
                        std::string text = batch[i].second;   // merge what goes to the same file
                        while (i + 1 < batch.size() && batch[i + 1].first == batch[i].first) text += batch[++i].second;
                        if (FILE* f = fopen(batch[i].first.c_str(), "ab")) { fputs(text.c_str(), f); fclose(f); }
                    }
                }
            }, nullptr, 0, "file writer");
            platformOnStop([] { std::lock_guard<std::mutex> lk(g_writer->mu); g_writer->cv.notify_all(); });
        }
    }
    g_writer->cv.notify_one();
}

void platformMemInfo(int* usedMB, int* totalMB) {
    *usedMB = *totalMB = -1;
#ifdef __SWITCH__
    u64 used = 0, total = 0;
    if (R_SUCCEEDED(svcGetInfo(&used, InfoType_UsedMemorySize, CUR_PROCESS_HANDLE, 0))) *usedMB = (int)(used >> 20);
    if (R_SUCCEEDED(svcGetInfo(&total, InfoType_TotalMemorySize, CUR_PROCESS_HANDLE, 0))) *totalMB = (int)(total >> 20);
#endif
}

// ---- files with UTF-8 paths
#ifdef _WIN32
static std::wstring wpath(const std::string& u) {
    int n = MultiByteToWideChar(CP_UTF8, 0, u.c_str(), -1, nullptr, 0);
    std::wstring w(n > 0 ? n : 1, L'\0');
    if (n > 0) MultiByteToWideChar(CP_UTF8, 0, u.c_str(), -1, &w[0], n);
    w.resize(wcslen(w.c_str()));
    return w;
}
FILE* platformFopen(const std::string& path, const char* mode) { return _wfopen(wpath(path).c_str(), wpath(mode).c_str()); }
bool platformMkdir(const std::string& path) { return CreateDirectoryW(wpath(path).c_str(), nullptr) || GetLastError() == ERROR_ALREADY_EXISTS; }
bool platformListDir(const std::string& dir, std::vector<std::string>& names) {
    WIN32_FIND_DATAW fd;
    std::wstring w = wpath(dir); if (!w.empty() && w.back() != L'\\' && w.back() != L'/') w += L'\\';
    HANDLE h = FindFirstFileW((w + L"*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return GetLastError() == ERROR_FILE_NOT_FOUND;   // (exists but empty)
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        int n = WideCharToMultiByte(CP_UTF8, 0, fd.cFileName, -1, nullptr, 0, nullptr, nullptr);
        std::string u(n > 0 ? n : 1, '\0');
        if (n > 0) WideCharToMultiByte(CP_UTF8, 0, fd.cFileName, -1, &u[0], n, nullptr, nullptr);
        u.resize(strlen(u.c_str()));
        names.push_back(u);
    } while (FindNextFileW(h, &fd));
    FindClose(h);
    return true;
}
bool platformReplaceFile(const std::string& from, const std::string& to) {
    return MoveFileExW(wpath(from).c_str(), wpath(to).c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
}
#else
FILE* platformFopen(const std::string& path, const char* mode) { return fopen(path.c_str(), mode); }
bool platformListDir(const std::string& dir, std::vector<std::string>& names) {
    DIR* d = opendir(dir.c_str()); if (!d) return false;
    while (dirent* e = readdir(d)) {
        std::string n = e->d_name;
        if (n == "." || n == "..") continue;
#if defined(DT_DIR)
        if (e->d_type == DT_DIR) continue;
#endif
        names.push_back(n);
    }
    closedir(d);
    return true;
}
bool platformMkdir(const std::string& path) { return mkdir(path.c_str(), 0777) == 0 || errno == EEXIST; }
bool platformReplaceFile(const std::string& from, const std::string& to) {
#if defined(__SWITCH__) || defined(__PSP__)
    remove(to.c_str());   // (the SD card's / Memory Stick's FAT driver doesn't replace on rename)
#endif
    return rename(from.c_str(), to.c_str()) == 0;
}
#endif
bool platformFileExists(const std::string& path) { FILE* f = platformFopen(path, "rb"); if (f) fclose(f); return f != nullptr; }
bool platformCopyFile(const std::string& from, const std::string& to) {
    FILE* a = platformFopen(from, "rb"); if (!a) return false;
    FILE* b = platformFopen(to, "wb"); if (!b) { fclose(a); return false; }
    char buf[65536]; size_t n; bool ok = true;
    while ((n = fread(buf, 1, sizeof buf, a)) > 0) if (fwrite(buf, 1, n, b) != n) { ok = false; break; }
    fclose(a); if (fclose(b) != 0) ok = false;
    return ok;
}
}
