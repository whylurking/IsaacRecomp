// Mods: files in the "mods" folder next to the program (mods/*.mod), turned on or off with `mods =` in isaac.ini
// (a comma-separated list of mod ids; tools/mod_manager.html edits it). A .mod file is a small text file:
//   id = staton            (required: what `mods =` refers to; letters, digits, _ and -)
//   name = StatOn          description = ...   version = 1.0   author = ...
//   engine = staton        (the code that runs it, built into the program; defaults to the id)
//   any other "key = value" lines are the mod's own options
// Mods never change the game's logic unless they say so: StatOn only reads the game's variables. See NOTES.md.
#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace fl {
struct CharDef; struct DisplayObject; struct MovieClip; struct RectF;

struct ModInfo {
    std::string file, id, name, version, author, description, engine;
    std::map<std::string, std::string> options;
    std::vector<std::string> optKeys;                 // the options in the file's order
    std::map<std::string, std::string> optNotes;      // the comment just above each option (its explanation)
    bool enabled = false, known = false;   // known: the program has the engine it asks for
    std::string opt(const std::string& k, const std::string& def = "") const { auto it = options.find(k); return it == options.end() ? def : it->second; }
};
// scans `dir` (ending in a separator) for .mod files and turns on those listed in `enabledList` (logged in log.txt);
// creates the folder, with the built-in mods, when it doesn't exist. Main thread, at startup.
void modsInit(const std::string& dir, const std::string& enabledList);
const std::vector<ModInfo>& modsAll();
const ModInfo* modActive(const char* engine);   // the enabled mod run by that engine, or null
// logic thread, every tick after the frame's scripts (and the recomp's own patches), before the snapshot
void modsTick();

// built-in engines
void statonTick(const ModInfo& m);
void genesisTick(const ModInfo& m);
void genesisInit(const ModInfo& m);   // (startup, when the mod is on)
void wideTick(const ModInfo& m);
void pilllogTick(const ModInfo& m);
void itemnomiconTick(const ModInfo& m);
void basetimeTick(const ModInfo& m);
void thegoneTick(const ModInfo& m);
void rainbowTick(const ModInfo& m);
uint32_t modsRunSerial();   // goes up each time the game starts a new run (newstart(true): Replay, R held...)
float itemnomiconShown();   // how visible Itemnomicon's card is (0-1; logic thread): WideSaac's Minimap+ fades under it
// WideSaac's Hyper Zoom moves groups of HUD pieces: the move (in HUD coordinates) of a group this tick, if any
enum { WIDE_LIFE, WIDE_ACTIVE, WIDE_COUNT, WIDE_POCKET, WIDE_GROUPS };
struct Mat;
bool wideHudMove(int group, Mat& m);
// Minimap+ (Hyper Zoom): available while the Hyper Zoom view is on (any thread); toggled by the main thread (M, L3,
// four fingers) or by tests (script "eval wide minimap")
bool wideMinimapAvailable();
void wideToggleMinimap();
bool wideMinimapShown();

// the character's stats as StatOn shows them (logic thread): speed, tears, damage, range, shot speed, luck (NAN: the
// game has none). Damage runs the game's dmgdo() without side effects, so call it at most every few ticks.
void currentStats(double out[6], bool withDamage = true);
extern std::string g_modsSaveDir;   // the save folder (set by main): files a mod keeps (GenesisConsole's history)

// ---- GenesisConsole (mod_genesis.cpp). The main thread handles input; the console itself runs on the logic thread.
enum ConKey { CK_ENTER, CK_BACKSPACE, CK_DELETE, CK_LEFT, CK_RIGHT, CK_UP, CK_DOWN, CK_HOME, CK_END, CK_TAB, CK_PGUP, CK_PGDN,
              CK_CLEARLINE };
bool genesisActive();                       // the mod is on (any thread)
bool genesisOpen();                         // the console is open (any thread)
void genesisSetOpen(bool open);             // main thread (opened only during a run)
void genesisKey(int ck);                    // main thread: an editing key
void genesisText(const std::string& utf8);  // main thread: typed / pasted text
bool genesisPausesGame();                   // logic thread: the console is open and pauses the game
double genesisOverride(int stat);           // the value a console command fixed for a stat (currentStats order), NAN if none
void genesisDebug(const std::string& cmd);  // tests (script "eval genesis ..."): open, close, type <text>, key <n>, or a command

// synthesized display objects (menuui.cpp), for mods that draw inside the game
uint16_t synthAddChar(CharDef* d);
uint16_t synthRect(float w, float h, uint32_t argb);
uint16_t synthTextField(float w, float size, uint32_t argb, int align);
uint16_t synthHandText(const std::string& text, float H, uint32_t argb, float* outW = nullptr);   // marker-style text, like the menus'   // align 0 left, 1 right, 2 center
DisplayObject* synthPut(MovieClip* parent, uint16_t id, int depth, double x, double y, const char* name = nullptr);
// (menuui.cpp) options a mod adds to the controller's choices on a menu screen: an object and its stage rectangle
void menuSetExtras(const std::vector<std::pair<DisplayObject*, RectF>>& v);
}
