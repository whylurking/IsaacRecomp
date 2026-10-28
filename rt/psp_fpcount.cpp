// (PSP diagnostics, ISAAC_PSP_FPCOUNT=ON) counts the software double-precision operations (the PSP's FPU is single
// precision only): linked with -Wl,--wrap for each libgcc helper
#if defined(__PSP__) && defined(ISAAC_PSP_FPCOUNT)
#include <cstdint>
extern "C" {
uint32_t g_fpCount[12];
#define W2(name, i, R) R __real_##name(double, double); R __wrap_##name(double a, double b) { g_fpCount[i]++; return __real_##name(a, b); }
W2(__adddf3, 0, double) W2(__subdf3, 1, double) W2(__muldf3, 2, double) W2(__divdf3, 3, double)
W2(__nedf2, 4, int) W2(__eqdf2, 5, int) W2(__ltdf2, 6, int) W2(__gtdf2, 7, int) W2(__gedf2, 8, int) W2(__ledf2, 9, int) W2(__unorddf2, 10, int)
int __real___fixdfsi(double); int __wrap___fixdfsi(double a) { g_fpCount[11]++; return __real___fixdfsi(a); }
}
#endif
