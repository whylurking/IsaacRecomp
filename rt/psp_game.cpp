// PSP: which game this binary carries (ISAAC_PSP_GAME: g_code_og, g_code_wotl or g_code_eternal; none in EBOOT.PBP,
// the launcher). The reference pulls that game's code out of libisaacgen.a; the others stay out (swfload.cpp refers
// to them weakly).
#ifdef __PSP__
#include "swfdata.h"
namespace fl {
#ifdef ISAAC_PSP_GAME
extern const GameCode ISAAC_PSP_GAME;
extern const GameCode* const g_pspBuiltinGame;
const GameCode* const g_pspBuiltinGame = &ISAAC_PSP_GAME;
#else
extern const GameCode* const g_pspBuiltinGame;
const GameCode* const g_pspBuiltinGame = nullptr;
#endif
}
#endif
