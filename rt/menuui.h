// Menus driven by a controller (focus outline + click), the "REBIND CONTROLS" entry on the game's Control options
// page, and the rebinding screen drawn inside the game (in its own HUD font). See NOTES.md.
#pragma once
#include <string>
#include <vector>

namespace fl {
struct Snapshot;

// What the rebinding screen shows (built by the main thread, drawn by the logic thread)
struct UiRow { std::string label, value; };
struct UiModel {
    bool open = false;
    std::string title, hint;
    std::vector<UiRow> rows;
    int sel = -1;
    bool capturing = false, blink = false;
};
void uiPublish(const UiModel& m);          // main thread
int uiHitRow(double sx, double sy);        // main thread: row at a stage point (-1 panel, no row; -2 outside)

// ---- logic thread
enum { NAV_UP, NAV_DOWN, NAV_LEFT, NAV_RIGHT, NAV_CONFIRM, NAV_BACK, NAV_OFF };
void padNavEvent(int code);
bool menuEscapeResumes();   // Escape pressed: on the pause screen it resumes instead (true: handled)
void menuMouse(double sx, double sy, int type);   // real mouse / touch events (before the game sees them)
void menuUiTick(bool gameplay, bool inGame);      // after the frame's scripts, before the snapshot
void menuUiFill(Snapshot& s);                     // focus outline, rebinding requests, game settings
extern int g_rebindRequests;                      // incremented when the player opens the rebinding screen
}
