// Small drawing helpers shared by the mods that draw inside the game (Itemnomicon, PillLog): texts in the HUD's
// font (Upheaval, ASCII only), measured and wrapped, and plain rectangles. Everything is made of synthesized display
// objects (menuui.cpp) placed in a clip the mod owns, so the game's own objects are never touched. Logic thread.
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace fl {
struct DisplayObject; struct MovieClip; struct EditTextObj;
namespace modui {
float textW(const std::string& s, float size);                                 // width of a line (_root units)
std::vector<std::string> wrap(const std::string& s, float width, float size);  // into lines at most `width` wide
// a text field with its text (align: 0 left, 1 right, 2 center); `shadow`: a dark copy one unit down-right below it
EditTextObj* text(MovieClip* c, int& depth, double x, double y, float w, float size, uint32_t argb, int align,
                  const std::string& t, bool shadow = true);
// a rectangle (any size: one synthesized 100x100 shape, scaled)
DisplayObject* rect(MovieClip* c, int& depth, double x, double y, double w, double h, uint32_t argb);
void clear(MovieClip* c);   // removes every child
}  // namespace modui
}  // namespace fl
