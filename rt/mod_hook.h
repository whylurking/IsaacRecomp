// Mods that wrap one of the game's functions on _root.a (each room is a new _root.a, so the wrap is put back on every
// room's copy). Several mods can wrap the same function: each wrapper calls the one it replaced (callGameOrig), and a
// wrapper already somewhere in the chain on this _root.a is not put on again (so two mods don't keep re-wrapping
// each other's wrappers every tick).
#pragma once
#include "core.h"

namespace fl {
struct GameHook { const char* name; Function* wrap = nullptr; Function* orig = nullptr; Object* a = nullptr; };
void hookGameFn(GameHook& w, const Value& a, NativeFn fn);   // (logic thread, every tick: cheap when already on)
Value callGameOrig(GameHook& w, Act& ac, Value thisv, int argc, Value* argv);
}  // namespace fl
