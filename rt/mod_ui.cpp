// Drawing helpers for the mods (mod_ui.h)
#include "mod_ui.h"
#include "mods.h"
#include "core.h"
#include "display.h"
#include "swfdata.h"
#include <sstream>

namespace fl {
namespace modui {
static const FontDef* font() {
    static const FontDef* f = nullptr;
    if (!f) for (CharDef* c : g_movie.chars) if (c && c->type == CH_FONT && ((FontDef*)c)->name.compare(0, 8, "Upheaval") == 0) { f = (FontDef*)c; break; }
    return f;
}
float textW(const std::string& s, float size) {
    const FontDef* f = font(); if (!f) return s.size() * size * 0.6f;
    float w = 0;
    for (unsigned char c : s) {
        auto it = f->codeToIdx.find(c); int i = it == f->codeToIdx.end() ? -1 : it->second;
        w += (i >= 0 && i < (int)f->advances.size()) ? f->advances[i] * size : size * 0.5f;
    }
    return w;
}
std::vector<std::string> wrap(const std::string& s, float width, float size) {
    std::vector<std::string> lines; std::string cur;
    std::istringstream ss(s); std::string w; bool first = true;
    while (ss >> w) {
        std::string t = first ? w : cur + " " + w;
        if (!first && textW(t, size) > width) { lines.push_back(cur); cur = w; }
        else cur = t;
        first = false;
    }
    if (!first) lines.push_back(cur);
    return lines;
}
EditTextObj* text(MovieClip* c, int& depth, double x, double y, float w, float size, uint32_t argb, int align, const std::string& t, bool shadow) {
    if (shadow) {
        uint32_t a = (uint32_t)((argb >> 24) * 0.8) << 24;
        DisplayObject* s = synthPut(c, synthTextField(w, size, a, align), depth++, x + 1, y + 1);
        if (s && s->kind == OK_EDITTEXT) ((EditTextObj*)s)->setText(t);
    }
    DisplayObject* d = synthPut(c, synthTextField(w, size, argb, align), depth++, x, y);
    if (!d || d->kind != OK_EDITTEXT) return nullptr;
    ((EditTextObj*)d)->setText(t);
    return (EditTextObj*)d;
}
DisplayObject* rect(MovieClip* c, int& depth, double x, double y, double w, double h, uint32_t argb) {
    DisplayObject* d = synthPut(c, synthRect(100, 100, argb | 0xff000000u), depth++, x, y);
    if (!d) return nullptr;
    d->m.a = w / 100; d->m.d = h / 100; d->scriptXform = true;
    d->cx.am = (argb >> 24) / 255.f;
    return d;
}
void clear(MovieClip* c) {
    if (!c) return;
    std::vector<DisplayObject*> kids = c->kids;
    for (DisplayObject* k : kids) removeObject(k);
}
}  // namespace modui
}  // namespace fl
