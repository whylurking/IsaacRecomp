// Per-platform settings (memory and cores). PC: Windows/Linux. Nintendo Switch: devkitA64 + libnx.
#pragma once
#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>
#include <functional>
namespace fl {
// Memory limits: chosen at startup (platformInitLimits). On Switch they depend on the available memory:
// in application mode (launched by holding R over a game) there is ~3 GB and the same limits as on PC are used;
// from the Album (applet) there is only ~400 MB and the reduced ones are used.
struct Limits {
    size_t gcMinLive;          // objects: a collection is triggered when this doubles
    size_t extGcBytes;         // pixel memory (CPU+GPU) that brings a collection forward
    size_t pxPoolBytes;        // recycled pixel buffers
    size_t leafBudgetBytes;    // BitmapData.draw leaf cache
    size_t bmTexPoolMax;       // spare BitmapData textures
    size_t pboBytes;           // pixel upload ring
    int initialAtlasPages;     // atlas pages created at startup
    int defaultCacheMB;        // maximum memory for atlas pages (16 MB each)
    double gcSweepMs;          // time per tick to free dead objects (lazy sweep)
    bool small;                // reduced profile
    double gcGrowth = 2.0;     // a collection when the objects reach this times the (1.5 x) live ones of the last one
};
extern Limits g_lim;
void platformInitLimits();
// Rasterization threads: how many and where. "Role" of the thread requesting parallel work.
enum ThreadRole { ROLE_RENDER = 0, ROLE_LOGIC = 1 };
int platformPoolWorkers(ThreadRole role);                       // helper threads for parallelFor of that role
void platformPoolWorkerPlace(ThreadRole role, int i, int* core, int* prio);
int platformRasterServiceWorkers();                             // threads for the renderer's async rasterization
void platformRasterServicePlace(int i, int* core, int* prio);
void platformSetThread(int core, int prio);                     // core/priority of the current thread (Switch)
#if defined(__SWITCH__)
constexpr bool kIsSwitch = true;
#else
constexpr bool kIsSwitch = false;
#endif
#if defined(__ANDROID__)
constexpr bool kIsAndroid = true;
#else
constexpr bool kIsAndroid = false;
#endif

// pins the current thread to a core (Switch only, where all threads start on core 0)
void platformPinThread(int core);
// creates a worker thread (detached, lives until the program ends) on the given core.
// Returns false (and logs it) if the system does not allow creating more threads.
bool platformStartThread(void (*fn)(void*), void* arg, int core, const char* name);
// Stop all helper threads (before relaunching the .nro on Switch: hbloader loads the next program
// into the same process and live threads from the previous one would corrupt its memory). Each thread loop checks
// platformStopping() and exits; platformOnStop registers how to wake it.
bool platformStopping();
void platformOnStop(std::function<void()> wake);
void platformStopThreads();
// appends text to a file from a separate thread (writing to the SD card is slow: not on the render thread)
void platformAppendFileAsync(const std::string& path, const std::string& text);
// application memory (MB used / total) for the log; -1 if unknown
void platformMemInfo(int* usedMB, int* totalMB);
// files with UTF-8 paths (on Windows the C library takes paths in the "ANSI" code page, so a folder with accents,
// Cyrillic... in its name couldn't be opened): fopen, mkdir, atomic replace, exists, copy
FILE* platformFopen(const std::string& path, const char* mode);
bool platformMkdir(const std::string& path);
bool platformReplaceFile(const std::string& from, const std::string& to);
bool platformFileExists(const std::string& path);
bool platformRemove(const std::string& path);
bool platformCopyFile(const std::string& from, const std::string& to);
// the files (not folders) in a folder; false if it doesn't exist
bool platformListDir(const std::string& dir, std::vector<std::string>& names);
}
