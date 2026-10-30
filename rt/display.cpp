// Display list, timelines and action queue (port of Ruffle's AVM1 semantics).
#include "display.h"
#include "platform.h"
#include <algorithm>
#include <cstdio>
#include <functional>

namespace fl {

Player g_player;
extern std::vector<std::function<void(Tracer&)>> g_rootTracers;
static int g_instanceCounter = 0;
constexpr int AVM_MAX_DEPTH = 2130706428;
constexpr int AVM_MAX_REMOVE_DEPTH = 2130706416;
static const double kPi = 3.14159265358979323846;

// ============================================================ Transforms
// Port of Ruffle's DisplayObjectBase transform (display_object.rs): the matrix a..d are f32, scale / rotation are
// cached and returned exactly as set by script (Percent / Degrees, Rust's to_degrees / to_radians).
static inline double f32r(double v) { return (double)(float)v; }
static const double kR2D = 57.2957795130823208767981548141051703;   // f64::to_degrees
static inline double d2r(double d) { return d * (3.14159265358979323846 / 180.0); }   // f64::to_radians
void DisplayObject::setMatrix(const Mat& mm) {
    m = mm; m.a = f32r(m.a); m.b = f32r(m.b); m.c = f32r(m.c); m.d = f32r(m.d);
    srCached = false;
}
void DisplayObject::cacheSR() {
    if (srCached) return;
    double rx = std::atan2(m.b, m.a), ry = std::atan2(-m.c, m.d);
    rotD = rx * kR2D;
    sxP = std::sqrt(m.a * m.a + m.b * m.b) * 100.0;
    syP = std::sqrt(m.c * m.c + m.d * m.d) * 100.0;
    skew = ry - rx;
    srCached = true;
}
void DisplayObject::setRotation(double deg) {
    cacheSR(); rotD = deg;
    double r = d2r(deg);
    if (std::isnan(r)) return;
    double cx_ = std::cos(r), sx_ = std::sin(r), cy_ = std::cos(r + skew), sy_ = std::sin(r + skew);
    double ux = sxP / 100.0, uy = syP / 100.0;
    m.a = f32r(ux * cx_); m.b = f32r(ux * sx_); m.c = f32r(uy * -sy_); m.d = f32r(uy * cy_);
}
void DisplayObject::setScaleX(double pct) {
    cacheSR(); sxP = pct;
    if (std::isnan(pct)) pct = 0;
    double r = d2r(rotD); if (std::isnan(r)) r = 0;
    double u = pct / 100.0;
    m.a = f32r(std::cos(r) * u); m.b = f32r(std::sin(r) * u);
}
void DisplayObject::setScaleY(double pct) {
    cacheSR(); syP = pct;
    if (std::isnan(pct)) pct = 0;
    double r = d2r(rotD); if (std::isnan(r)) r = 0;
    double u = pct / 100.0;
    m.c = f32r(-std::sin(r + skew) * u); m.d = f32r(std::cos(r + skew) * u);
}
void DisplayObject::trace(Tracer& tr) { Object::trace(tr); tr.mark(parent); tr.mark(name); tr.mark(path); }
void BitmapObj::trace(Tracer& tr) { DisplayObject::trace(tr); tr.mark(bd); }
void ButtonObj::trace(Tracer& tr) { DisplayObject::trace(tr); for (auto* k : kids) tr.mark(k); for (auto* k : hitKids) tr.mark(k); }
void MovieClip::trace(Tracer& tr) { DisplayObject::trace(tr); for (auto* k : kids) tr.mark(k); tr.mark(tscope); tr.mark(execNext); }

// ============================================================ Children
DisplayObject* MovieClip::childAtDepth(int d) const {
    auto it = std::lower_bound(kids.begin(), kids.end(), d, [](DisplayObject* c, int dd) { return c->depth < dd; });
    return (it != kids.end() && (*it)->depth == d) ? *it : nullptr;
}
void MovieClip::reindexName(Str* n) {
    if (!n) return;
    DisplayObject* best = nullptr;
    for (auto* k : kids) if (k->name == n) { best = k; break; }
    if (best) byName[n] = best; else byName.erase(n);
}
void MovieClip::insertChild(DisplayObject* c) {
    auto it = std::lower_bound(kids.begin(), kids.end(), c->depth, [](DisplayObject* x, int dd) { return x->depth < dd; });
    kids.insert(it, c);
    c->parent = this;
    if (c->name) reindexName(c->name);
}
void MovieClip::eraseChild(DisplayObject* c) {
    auto it = std::find(kids.begin(), kids.end(), c);
    if (it != kids.end()) kids.erase(it);
    if (c->name) reindexName(c->name);
}
void MovieClip::renameChild(DisplayObject* c, Str* nn) {
    Str* old = c->name; c->name = nn;
    if (old) reindexName(old);
    if (nn) reindexName(nn);
}

// ============================================================ Paths
static void buildPath(DisplayObject* d, std::string& out, bool slash) {
    if (!d->parent) { out = slash ? "" : "_level0"; return; }
    buildPath(d->parent, out, slash);
    out += slash ? "/" : ".";
    out += d->name ? d->name->s : std::string("");
}
Str* targetPath(DisplayObject* d) {
    if (d->removed && d->path) return d->path;
    std::string s; buildPath(d, s, false); return mkstr(s);
}
static Str* slashPath(DisplayObject* d) { std::string s; buildPath(d, s, true); if (s.empty()) s = "/"; return mkstr(s); }

DisplayObject* doRoot(DisplayObject* d) { while (d->parent) d = d->parent; return d; }
DisplayObject* doParent(DisplayObject* d) { return d->parent; }
Str* doPath(DisplayObject* d) { return targetPath(d); }
bool doRemoved(DisplayObject* d) { return d->removed; }

static DisplayObject* resolveDotPath(DisplayObject* start, const std::string& path) {
    DisplayObject* cur = start;
    size_t i = 0;
    if (path.empty()) return start;
    if (path[0] == '/') { cur = vm.root; i = 1; }
    while (i <= path.size() && cur) {
        size_t j = path.find_first_of("./:", i);
        std::string part = path.substr(i, j == std::string::npos ? std::string::npos : j - i);
        if (!part.empty()) {
            if (part == "_root" || part == "_level0") cur = doRoot(cur);
            else if (part == "_parent" || part == "..") cur = cur->parent;
            else if (part == "this" ) {}
            else if (cur->kind == OK_CLIP) {
                Str* k = intern(part);
                int pi = cur->props.find(k);
                if (pi >= 0 && cur->props.v[pi].v.isObj() && cur->props.v[pi].v.o->isDisplay()) cur = (DisplayObject*)cur->props.v[pi].v.o;
                else cur = ((MovieClip*)cur)->childByName(k);
            } else cur = nullptr;
        }
        if (j == std::string::npos) break;
        i = j + 1;
    }
    return cur;
}

DisplayObject* doResolveRemoved(DisplayObject* d) {
    if (!d->removed) return d;
    if (!d->path) return nullptr;
    const std::string& p = d->path->s;
    if (p.compare(0, 8, "_level0.") != 0) return nullptr;
    DisplayObject* r = resolveDotPath(vm.root, p.substr(8));
    return (r && !r->removed) ? r : nullptr;
}

Value resolveTargetPath(Act& a, const Value& path) {
    if (path.isObj()) return path;
    DisplayObject* start = a.target ? a.target : (DisplayObject*)vm.root;
    Str* s = toStr(path);
    if (s->s.empty()) return Value((Object*)start);
    DisplayObject* r = resolveDotPath(start, s->s);
    if (!r) {   // try the scope chain
        for (Scope* sc = a.scope; sc; sc = sc->parent)
            if (sc->values && sc->values->isDisplay()) { r = resolveDotPath((DisplayObject*)sc->values, s->s); if (r) break; }
    }
    return r ? Value((Object*)r) : VUNDEF;
}

// ============================================================ Display properties
static double roundTwips(double v) { return std::round(v * 20.0) / 20.0; }
// _x / _y: Flash stores whole twips, truncating toward zero (Ruffle: Twips::from_pixels, `(px * 20) as i32`, which
// saturates); +-Infinity becomes -Infinity (so -107374182.4)
static double toTwips(double px) {
    if (std::isinf(px)) px = -INFINITY;
    double t = px * 20.0;
    int32_t i = std::isnan(t) ? 0 : t >= 2147483647.0 ? 2147483647 : t <= -2147483648.0 ? (int32_t)-2147483648LL : (int32_t)t;
    return i / 20.0;
}
// a display property's new value: undefined, null and NaN are ignored; Infinity is accepted (Ruffle:
// property_coerce_to_number)
static bool coerceNum(const Value& v, double& out) {
    if (v.isNullish()) return false;
    out = toNum(v);
    return !std::isnan(out);
}
static bool coerceProp(const Value& v, double& out) {
    if (v.isNullish()) return false;
    out = toNum(v);
    return std::isfinite(out);
}
static Value mouseLocal(DisplayObject* d, bool yaxis) {
    Mat w = worldMatrix(d), inv;
    if (!w.invert(inv)) return Value(0.0);
    double x, y; inv.apply(g_player.mouseX, g_player.mouseY, x, y);
    return Value(roundTwips(yaxis ? y : x));
}
static const char* kQuality[] = {"LOW", "MEDIUM", "HIGH", "BEST"};

static Value getDisplayProp(DisplayObject* d, int idx) {
    switch (idx) {
        case 0: return Value(d->m.tx);
        case 1: return Value(d->m.ty);
        case 2: d->cacheSR(); return Value(d->sxP);
        case 3: d->cacheSR(); return Value(d->syP);
        case 4: {
            if (d->kind != OK_CLIP) return Value(1.0);
            MovieClip* c = (MovieClip*)d;
            return Value((double)std::max(1, c->cur));
        }
        case 5: case 12: return Value((double)(d->kind == OK_CLIP ? ((MovieClip*)d)->def->nframes : 1));
        case 6: return Value((double)d->cx.am * 100.0);
        case 7: return VB(d->visible);
        case 8: case 9: { RectF b = localBounds(d); return Value(idx == 8 ? b.x1 - b.x0 : b.y1 - b.y0); }
        case 10: d->cacheSR(); return Value(d->rotD);
        case 11: return Value(slashPath(d));
        case 13: return Value(d->name ? d->name : atoms::empty);
        case 14: return Value(atoms::empty);
        case 15: return Value(intern("file:///The%20Binding%20of%20Isaac%20-%20Wrath%20of%20the%20Lamb.swf"));
        case 16: return Value((double)(g_player.quality >= 2 ? 1 : 0));
        case 17: return VTRUE;
        case 18: return Value(5.0);
        case 19: return Value(intern(kQuality[g_player.quality & 3]));
        case 20: return mouseLocal(d, false);
        case 21: return mouseLocal(d, true);
    }
    return VUNDEF;
}

static void setDisplayProp(DisplayObject* d, int idx, const Value& v) {
    double n;
    switch (idx) {
        case 0: if (coerceNum(v, n)) { d->m.tx = toTwips(n); d->scriptXform = true; } break;
        case 1: if (coerceNum(v, n)) { d->m.ty = toTwips(n); d->scriptXform = true; } break;
        case 2: if (coerceProp(v, n)) { d->setScaleX(n); d->scriptXform = true; } break;
        case 3: if (coerceProp(v, n)) { d->setScaleY(n); d->scriptXform = true; } break;
        case 6: if (coerceProp(v, n)) { d->cx.am = (float)(n / 100.0); d->scriptXform = true; } break;
        case 7: if (coerceProp(v, n) || v.t == T_BOOL) d->visible = v.t == T_BOOL ? v.b : n != 0; break;
        case 8: case 9: {
            if (!coerceProp(v, n)) break;
            // Ruffle's set_width / set_height (bounds in the object's own space, rotation-aware)
            Mat save = d->m; d->m = Mat(); RectF b = localBounds(d); d->m = save;
            double ow = b.x1 - b.x0, oh = b.y1 - b.y0;
            d->cacheSR();
            double px = d->sxP / 100.0, py = d->syP / 100.0, r = d2r(d->rotD);
            double c = std::fabs(std::cos(r)), sn = std::fabs(std::sin(r)), nx, ny;
            if (idx == 8) {
                double ar = oh / ow, tx = ow != 0 ? n / ow : 0, ty = ow != 0 ? n / oh : 0;
                nx = ar * (c * tx + sn * ty) / ((c + ar * sn) * (ar * c + sn));
                ny = (sn * px + ar * c * py) / (ar * c + sn);
            } else {
                double ar = ow / oh, tx = oh != 0 ? n / ow : 0, ty = oh != 0 ? n / oh : 0;
                nx = (ar * c * px + sn * py) / (ar * c + sn);
                ny = ar * (sn * tx + c * ty) / ((c + ar * sn) * (ar * c + sn));
            }
            if (!std::isfinite(nx)) nx = 0;
            if (!std::isfinite(ny)) ny = 0;
            d->setScaleX(nx * 100.0); d->setScaleY(ny * 100.0); d->scriptXform = true;
            break;
        }
        case 10: if (coerceProp(v, n)) {
            double deg = std::fmod(n, 360.0); if (deg < -180) deg += 360; else if (deg > 180) deg -= 360;
            d->setRotation(deg); d->scriptXform = true; } break;
        case 13: if (d->parent) d->parent->renameChild(d, atomOf(toStr(v))); else d->name = atomOf(toStr(v)); break;
        case 16: if (coerceProp(v, n)) g_player.quality = n > 0 ? 2 : 0; break;
        case 19: {
            std::string q = toStr(v)->s; for (auto& c : q) c = (char)toupper((unsigned char)c);
            for (int i = 0; i < 4; i++) if (q == kQuality[i]) g_player.quality = i;
            break;
        }
        default: break;
    }
}

bool doGetSpecial(DisplayObject* d, Str* k, Value& out) {
    if (!k->s.empty() && k->s[0] == '_') {
        bool found; extern Value resolvePathPropertyPublic(DisplayObject*, Str*, bool&);
        if (k == atoms::_root) { out = Value((Object*)doRoot(d)); return true; }
        if (k == atoms::_parent) { out = d->parent ? Value((Object*)d->parent) : VUNDEF; return true; }
        if (k == atoms::_global) { out = Value(vm.global); return true; }
        if (k->s == "_level0") { out = Value((Object*)vm.root); return true; }
        (void)found;
    }
    if (d->kind == OK_CLIP) {
        DisplayObject* c = ((MovieClip*)d)->childByName(k);
        if (c) {
            if (c->kind == OK_CLIP || c->kind == OK_BUTTON || c->kind == OK_EDITTEXT) out = Value((Object*)c);
            else out = Value((Object*)d);
            return true;
        }
    }
    if (k->dprop >= 0) { out = getDisplayProp(d, k->dprop); return true; }
    return false;
}
bool doHasSpecial(DisplayObject* d, Str* k) {
    if (k->dprop >= 0) return true;
    if (d->kind == OK_CLIP && ((MovieClip*)d)->childByName(k)) return true;
    if (k == atoms::_root || k == atoms::_parent || k == atoms::_global || k->s == "_level0") return true;
    return false;
}
bool doSetSpecial(DisplayObject* d, Str* k, const Value& v) {
    if (k->dprop < 0) return false;
    if (k->dprop == 4 || k->dprop == 5 || k->dprop == 11 || k->dprop == 12 || k->dprop == 14 || k->dprop == 15 || k->dprop == 20 || k->dprop == 21) return true;  // read-only
    setDisplayProp(d, k->dprop, v);
    return true;
}
void doEnumChildren(DisplayObject* d, std::vector<Str*>& keys) {
    if (d->kind != OK_CLIP) return;
    auto& kids = ((MovieClip*)d)->kids;
    for (size_t i = kids.size(); i-- > 0;) if (kids[i]->name) keys.push_back(kids[i]->name);
}

// ============================================================ Object creation
Object* g_overFrames = nullptr;
MovieClip* newClip(SpriteDef* def) {
    spriteEnsure(def);
    MovieClip* c = gcNew<MovieClip>();
    c->kind = OK_CLIP; c->proto = protos::movieclip; c->def = def; c->charId = def->id;
    return c;
}

static DisplayObject* createForChar(uint16_t id) {
    CharDef* cd = g_movie.get(id);
    if (!cd) return nullptr;
    switch (cd->type) {
        case CH_SPRITE: return newClip((SpriteDef*)cd);
        case CH_SHAPE: case CH_MORPH: case CH_TEXT: {
            ShapeObj* s = gcNew<ShapeObj>(); s->kind = cd->type == CH_SHAPE ? OK_SHAPE : cd->type == CH_MORPH ? OK_MORPH : OK_TEXT;
            s->def = cd; s->charId = id; s->proto = protos::object; return s;
        }
        case CH_EDITTEXT: {
            EditTextObj* e = gcNew<EditTextObj>(); e->kind = OK_EDITTEXT; e->def = (EditTextDef*)cd; e->charId = id; e->proto = protos::textfield;
            e->argb = e->def->argb;
            std::string t = e->def->initialText;
            e->setText(t);
            if (!e->def->variable.empty()) g_player.boundTexts.push_back(e);
            return e;
        }
        case CH_BUTTON: {
            ButtonObj* b = gcNew<ButtonObj>(); b->kind = OK_BUTTON; b->def = (ButtonDef*)cd; b->charId = id; b->proto = protos::button;
            return b;
        }
        default: return nullptr;
    }
}

static void buttonSetState(ButtonObj* b, int state);

void applyPlace(DisplayObject* d, const PlaceCmd& pc) {
    // Flash Player keeps applying the timeline's matrix to an object whose color was set through
    // Transform.colorTransform (only its color stops following the timeline): the game recolors the HUD hearts
    // every tick and then switches -LIFE- to its 24/48-heart layout with gotoAndStop, which must move/shrink them.
    // (Ruffle locks the whole PlaceObject here; the original game shows the hearts re-laid out.)
    if (!d->scriptXform && pc.hasMatrix) d->setMatrix(pc.m);
    if (!d->scriptXform && !d->scriptColor && pc.hasCx) d->cx = pc.cx;
    if (pc.hasRatio) d->ratio = pc.ratio;
    if (pc.hasFilters) d->filters = pc.filters;
    if (pc.hasBlend) d->blend = pc.blend;
}

void addToExec(MovieClip* c) {
    if (c->inExec) return;
    c->inExec = true; c->execNext = g_player.execHead; g_player.execHead = c;
}

static void runFrameInternal(MovieClip* c, bool display, bool sounds);

static void postInstantiateClip(MovieClip* c, bool runFrame, Object* init) {
    if (!c->name) { char b[32]; snprintf(b, sizeof b, "instance%d", ++g_instanceCounter); if (c->parent) c->parent->renameChild(c, internWeak(b)); else c->name = internWeak(b); }
    addToExec(c);
    if (runFrame) {
        // run_frame_avm1: Load event and advance to the first frame
        if (!c->initialized) { c->initialized = true; queueMethod(c, atoms::onLoad); }
        else queueMethod(c, atoms::onEnterFrame);
        if (c->playing) runFrameInternal(c, true, true);
    }
    if (init) {
        std::vector<Str*> keys;
        for (size_t i = init->props.v.size(); i-- > 0;) keys.push_back(init->props.v[i].k);
        for (size_t i = keys.size(); i-- > 0;) setProp(c, keys[i], getProp(init, keys[i]));
    }
}

DisplayObject* instantiate(MovieClip* parent, uint16_t id, int depth, const PlaceCmd* pc, bool byTimeline) {
    if (byTimeline && parent->childAtDepth(depth)) return nullptr;
    DisplayObject* d = createForChar(id);
    if (!d) return nullptr;
    d->depth = depth; d->parent = parent; d->placeFrame = parent->cur; d->timelinePlaced = byTimeline;
    if (pc) {
        applyPlace(d, *pc);
        if (pc->hasName) d->name = pc->name;
        if (pc->hasClipDepth) d->clipDepth = pc->clipDepth;
    }
    parent->insertChild(d);
    if (d->kind == OK_BUTTON) buttonSetState((ButtonObj*)d, 0);
    if (d->kind == OK_CLIP) postInstantiateClip((MovieClip*)d, true, nullptr);
    return d;
}

// ============================================================ Removal
void markRemoved(DisplayObject* d) {
    if (d->removed) return;
    d->path = targetPath(d);
    d->removed = true;
    if (d->kind == OK_CLIP) {
        MovieClip* c = (MovieClip*)d;
        if (c->streamChan >= 0) { soundStopChan(c->streamChan); c->streamChan = -1; }
        for (auto* k : c->kids) markRemoved(k);
    } else if (d->kind == OK_BUTTON) {
        for (auto* k : ((ButtonObj*)d)->kids) markRemoved(k);
        if (g_player.hoverButton == d) g_player.hoverButton = nullptr;
        if (g_player.pressButton == d) g_player.pressButton = nullptr;
    }
}
void removeObject(DisplayObject* d) {
    if (d->parent && !d->removed) {
        // onUnload
        if (d->kind == OK_CLIP) {
            Value f = getProp(d, intern("onUnload"));
            if (f.isObj()) { Act a; beginTimeline(a, d); callValue(a, f, Value((Object*)d), 0, nullptr); }
        }
        markRemoved(d);
        d->parent->eraseChild(d);
    } else if (d->parent) d->parent->eraseChild(d);
}
bool removeMovieClip(DisplayObject* d) {
    if (d->depth >= DEPTH_BIAS && d->depth < AVM_MAX_REMOVE_DEPTH && !d->removed && d->parent) { removeObject(d); return true; }
    return false;
}

static void replaceAtDepth(MovieClip* parent, DisplayObject* d, int depth) {
    if (DisplayObject* old = parent->childAtDepth(depth)) removeObject(old);
    d->depth = depth; parent->insertChild(d);
}

bool swapDepths(DisplayObject* d, const Value& target) {
    if (d->removed || !d->parent) return false;
    MovieClip* p = d->parent;
    int depth; bool have = false;
    if (target.t == T_NUM) { depth = toInt32(target.n) + DEPTH_BIAS; have = true; }
    else {
        Act a{vm.globalScope, VUNDEF, d, nullptr};
        Value t = resolveTargetPath(a, target);
        if (t.isObj() && t.o->isDisplay() && ((DisplayObject*)t.o)->parent == p && !((DisplayObject*)t.o)->removed) { depth = ((DisplayObject*)t.o)->depth; have = true; }
        else depth = 0;
    }
    if (!have || depth < 0 || depth > AVM_MAX_DEPTH) return false;
    if (depth != d->depth) {
        DisplayObject* other = p->childAtDepth(depth);
        int old = d->depth;
        p->eraseChild(d);
        if (other) { p->eraseChild(other); other->depth = old; p->insertChild(other); other->scriptXform = true; }
        d->depth = depth; p->insertChild(d);
        d->scriptXform = true;
    }
    return true;
}

MovieClip* attachMovie(MovieClip* parent, const std::string& linkName, Str* newName, int depth, Object* init) {
    auto it = g_movie.exports.find(linkName);
    if (it == g_movie.exports.end()) return nullptr;
    CharDef* cd = g_movie.get(it->second);
    if (!cd || cd->type != CH_SPRITE) return nullptr;
    depth += DEPTH_BIAS;
    if (depth < 0 || depth > AVM_MAX_DEPTH) return nullptr;
    MovieClip* c = newClip((SpriteDef*)cd);
    c->placedByScript = true; c->name = newName;
    replaceAtDepth(parent, c, depth);
    postInstantiateClip(c, true, init);
    return c;
}

MovieClip* createEmptyMovieClip(MovieClip* parent, Str* name, int depth) {
    static SpriteDef* empty = nullptr;
    if (!empty) { empty = new SpriteDef(); empty->type = CH_SPRITE; empty->id = 0xffff; empty->nframes = 1; empty->frames.resize(1); }
    MovieClip* c = newClip(empty);
    c->placedByScript = true; c->name = name;
    replaceAtDepth(parent, c, depth + DEPTH_BIAS);
    postInstantiateClip(c, true, nullptr);
    return c;
}

MovieClip* newDetachedClip() {
    static SpriteDef* empty = nullptr;
    if (!empty) { empty = new SpriteDef(); empty->type = CH_SPRITE; empty->id = 0xffff; empty->nframes = 1; empty->frames.resize(1); }
    MovieClip* c = newClip(empty);
    c->placedByScript = true; c->initialized = true; c->cur = 1;
    return c;
}

// ============================================================ Timeline sounds
static void startTimelineSound(MovieClip* c, const FrameCmd& fc) {
    CharDef* cd = g_movie.get(fc.soundId);
    if (!cd || cd->type != CH_SOUND) return;
    SoundDef* s = (SoundDef*)cd;
    if (fc.soundFlags & 0x20) { extern void soundStopDef(SoundDef*); soundStopDef(s); return; }
    if ((fc.soundFlags & 0x10) && soundPlayingDef(s)) return;
    soundStart(s, fc.inPoint / 44100.0, fc.loops ? fc.loops : 1, c, 1.0f, 0.0f, fc.envelope.empty() ? nullptr : &fc.envelope);
}

static void handleStream(MovieClip* c) {
    StreamInfo* st = c->def->stream.get();
    if (!st) return;
    int f0 = c->cur - 1;
    if (f0 < 0 || f0 >= (int)st->frameBlock.size() || st->frameBlock[f0] < 0) return;
    if (c->streamChan < 0 || !chanAlive(c->streamChan)) c->streamChan = streamStart(st, f0, c);
    else streamSync(c->streamChan, st, f0);
    c->lastStreamFrame = f0;
}
static void stopStream(MovieClip* c) { if (c->streamChan >= 0) { soundStopChan(c->streamChan); c->streamChan = -1; } }

// ============================================================ Frame advance
static void runGoto(MovieClip* c, int frame, bool implicit);

static void replaceWith(DisplayObject* d, uint16_t id) {
    if (d->kind == OK_SHAPE || d->kind == OK_MORPH || d->kind == OK_TEXT) {
        CharDef* cd = g_movie.get(id);
        if (cd && (cd->type == CH_SHAPE || cd->type == CH_MORPH || cd->type == CH_TEXT)) {
            ShapeObj* s = (ShapeObj*)d; s->def = cd; s->charId = id;
            s->kind = cd->type == CH_SHAPE ? OK_SHAPE : cd->type == CH_MORPH ? OK_MORPH : OK_TEXT;
        }
    }
}

static void runFrameInternal(MovieClip* c, bool display, bool sounds) {
    SpriteDef* def = c->def;
    if (c->cur < def->nframes) c->cur++;
    else if (def->nframes > 1) { runGoto(c, 1, true); return; }
    else { c->playing = false; return; }
    auto& cmds = def->frames[c->cur - 1];
    for (size_t i = 0; i < cmds.size(); i++) {
        const FrameCmd& fc = cmds[i];
        switch (fc.kind) {
            case FrameCmd::PLACE: {
                if (!display) break;
                const PlaceCmd& pc = def->places[fc.place];
                if (!pc.move && pc.hasId) instantiate(c, pc.id, pc.depth, &pc, true);
                else if (pc.move && pc.hasId) {
                    if (DisplayObject* ch = c->childAtDepth(pc.depth)) { replaceWith(ch, pc.id); applyPlace(ch, pc); ch->placeFrame = c->cur; }
                } else if (DisplayObject* ch = c->childAtDepth(pc.depth)) applyPlace(ch, pc);
                break;
            }
            case FrameCmd::REMOVE:
                if (display) if (DisplayObject* ch = c->childAtDepth(fc.depth)) removeObject(ch);
                break;
            case FrameCmd::ACTION: queueScript(c, fc.script); break;
            case FrameCmd::SOUND: if (sounds) startTimelineSound(c, fc); break;
        }
    }
    if (sounds) handleStream(c);
}

struct GotoPlace { int depth, frame, index; PlaceCmd pc; };

static bool survivesRewind(MovieClip* c, DisplayObject* old, const std::vector<GotoPlace>& cmds) {
    (void)c;
    bool candidate = old->depth < DEPTH_BIAS;
    if (!candidate && old->kind != OK_MORPH) return true;
    const GotoPlace* fp = nullptr;
    for (auto& g : cmds) if (g.depth == old->depth) { fp = &g; break; }
    if (!fp) return false;
    if (old->placedByScript && old->depth < DEPTH_BIAS) return false;
    const PlaceCmd& np = fp->pc;
    bool idEq = np.hasId && old->charId == np.id;
    bool ratioEq = !np.hasRatio || old->ratio == np.ratio;
    bool clipEq = !np.hasClipDepth || old->clipDepth == np.clipDepth;
    bool cxEq = !np.hasCx || old->cx == np.cx;
    bool mEq = !np.hasMatrix || old->m == np.m;
    switch (old->kind) {
        case OK_MORPH: case OK_SHAPE: case OK_TEXT: return ratioEq && idEq && clipEq && mEq && cxEq;
        case OK_BUTTON: case OK_EDITTEXT: case OK_BITMAP: return ratioEq && idEq && clipEq;
        default: return ratioEq;
    }
}

static void runGotoCmd(MovieClip* c, const GotoPlace& g, bool rewind) {
    DisplayObject* child = c->childAtDepth(g.depth);
    const PlaceCmd& pc = g.pc;
    bool isModify = pc.move && !pc.hasId, isReplace = pc.move && pc.hasId, isPlace = !pc.move && pc.hasId;
    if (child && (rewind || isModify)) applyPlace(child, pc);
    else if (isReplace && child) { replaceWith(child, pc.id); applyPlace(child, pc); child->placeFrame = g.frame; }
    else if (isPlace || isReplace) { DisplayObject* d = instantiate(c, pc.id, g.depth, &pc, true); if (d) d->placeFrame = g.frame; }
}

static void runGoto(MovieClip* c, int frame, bool implicit) {
    (void)implicit;
    int before = c->cur;
    stopStream(c);
    bool rewind = frame <= c->cur;
    if (rewind) c->cur = 0;
    int from = c->cur;
    int clamped = std::min(frame, c->def->nframes);
    std::vector<GotoPlace> cmds; cmds.reserve(32);
    int index = 0;
    while (c->cur < clamped) {
        c->cur++;
        for (const FrameCmd& fc : c->def->frames[c->cur - 1]) {
            if (fc.kind == FrameCmd::PLACE) {
                index++;
                GotoPlace gp; gp.pc = c->def->places[fc.place]; gp.depth = gp.pc.depth; gp.frame = c->cur; gp.index = index;
                if (rewind && !gp.pc.move && gp.pc.hasId) {
                    if (!gp.pc.hasMatrix) { gp.pc.hasMatrix = true; gp.pc.m = Mat(); }
                    if (!gp.pc.hasCx) { gp.pc.hasCx = true; gp.pc.cx = CX(); }
                    if (!gp.pc.hasRatio) { gp.pc.hasRatio = true; gp.pc.ratio = 0; }
                    if (!gp.pc.hasFilters) { gp.pc.hasFilters = true; gp.pc.filters.clear(); }
                }
                auto it = std::find_if(cmds.begin(), cmds.end(), [&](const GotoPlace& x) { return x.depth == gp.depth; });
                if (it != cmds.end()) {
                    PlaceCmd& cur = it->pc; const PlaceCmd& nx = gp.pc;
                    bool nxModify = nx.move && !nx.hasId;
                    if (!nxModify) { cur.move = nx.move; cur.hasId = nx.hasId; cur.id = nx.id; it->frame = gp.frame; }
                    if (nx.hasMatrix) { cur.hasMatrix = true; cur.m = nx.m; }
                    if (nx.hasCx) { cur.hasCx = true; cur.cx = nx.cx; }
                    if (nx.hasRatio) { cur.hasRatio = true; cur.ratio = nx.ratio; }
                    if (nx.hasBlend) { cur.hasBlend = true; cur.blend = nx.blend; }
                    if (nx.hasFilters) { cur.hasFilters = true; cur.filters = nx.filters; }
                } else cmds.push_back(gp);
            } else if (fc.kind == FrameCmd::REMOVE) {
                auto it = std::find_if(cmds.begin(), cmds.end(), [&](const GotoPlace& x) { return x.depth == fc.depth; });
                if (it != cmds.end()) { *it = cmds.back(); cmds.pop_back(); }
                if (!rewind) {
                    int to = c->cur; c->cur = from;
                    if (DisplayObject* ch = c->childAtDepth(fc.depth)) removeObject(ch);
                    c->cur = to;
                }
            }
        }
    }
    bool hit = c->cur == frame;
    if (rewind) {
        std::vector<DisplayObject*> kids = c->kids;
        for (auto* k : kids) if (!survivesRewind(c, k, cmds)) removeObject(k);
    }
    std::sort(cmds.begin(), cmds.end(), [](const GotoPlace& a, const GotoPlace& b) { return a.index < b.index; });
    for (auto& g : cmds) if (g.frame < frame) runGotoCmd(c, g, rewind);
    if (hit) { c->cur--; runFrameInternal(c, false, frame != before); }
    else c->cur = clamped;
    for (auto& g : cmds) if (g.frame >= frame) runGotoCmd(c, g, rewind);
}

void gotoFrame(MovieClip* c, int frame, bool stop) {
    if (const char* gt = getenv("ISAAC_GOTOTRACE")) {   // (testing) who sends a clip to a frame
        std::string p = targetPath(c)->s;
        if (p.find(gt) != std::string::npos) logf("[goto] t=%u %s -> %d (from %d)\n", g_player.tick, p.c_str(), frame, c->cur);
    }
    c->reqFrame = frame;
    if (frame >= 100 && c->def && frame > (int)c->def->nframes && g_overFrames && g_overFrames->kind == OK_ARRAY) {
        auto& el = ((Array*)g_overFrames)->el;
        if (el.size() < 4096) { bool have = false; for (const Value& v : el) if (v.isObj() && v.o == (Object*)c) { have = true; break; } if (!have) el.push_back(Value((Object*)c)); }
    }
    if (frame < 1) frame = 1;
    if (stop) { c->playing = false; stopStream(c); } else c->playing = true;
    if (frame != c->cur) runGoto(c, frame, false);
}

// Rust's str::parse::<f64>() (Ruffle parses goto frame strings with it): decimal only, no whitespace, "inf"/"nan"
static bool rustParseF64(const std::string& s, double& out) {
    if (s.empty()) return false;
    size_t i = 0; if (s[0] == '+' || s[0] == '-') i++;
    std::string rest = s.substr(i); for (auto& ch : rest) ch = (char)tolower((unsigned char)ch);
    if (rest == "inf" || rest == "infinity") { out = s[0] == '-' ? -INFINITY : INFINITY; return true; }
    if (rest == "nan") { out = NAN; return true; }
    bool digits = false; size_t j = i;
    while (j < s.size() && isdigit((unsigned char)s[j])) { j++; digits = true; }
    if (j < s.size() && s[j] == '.') { j++; while (j < s.size() && isdigit((unsigned char)s[j])) { j++; digits = true; } }
    if (!digits) return false;
    if (j < s.size() && (s[j] == 'e' || s[j] == 'E')) {
        size_t k = j + 1; if (k < s.size() && (s[k] == '+' || s[k] == '-')) k++;
        size_t k0 = k; while (k < s.size() && isdigit((unsigned char)s[k])) k++;
        if (k == k0) return false;
        j = k;
    }
    if (j != s.size()) return false;
    out = strtod(s.c_str(), nullptr);
    return true;
}
// gotoAndStop / gotoAndPlay argument (Ruffle avm1/globals/movie_clip.rs goto_frame): a whole number is a frame;
// anything else (including fractional numbers) is coerced to a string and resolved as "path:frame" or
// "path.frame" (the right-most ':' or '.'), the frame being a number or a label of that clip. So
// gotoAndStop(7.5) looks for a clip named "7" and does nothing.
int frameFromValue(MovieClip* c, const Value& v, MovieClip** target) {
    *target = c;
    if (v.t == T_NUM && v.n == std::floor(v.n)) return toInt32(v.n);
    Str* s = toStr(v);
    std::string fs = s->s;
    size_t sep = fs.find_last_of(":.");
    if (sep != std::string::npos) {
        Act a{vm.globalScope, VUNDEF, c, nullptr};
        Value t = resolveTargetPath(a, mkstr(fs.substr(0, sep)));
        if (!t.isObj() || t.o->kind != OK_CLIP) return 0;
        *target = (MovieClip*)t.o; fs = fs.substr(sep + 1);
    }
    double n;
    if (rustParseF64(fs, n)) return toInt32(n);
    for (auto& ch : fs) ch = (char)tolower((unsigned char)ch);   // (labels are stored lowercase: case-insensitive)
    auto it = (*target)->def->labels.find(fs);
    if (it != (*target)->def->labels.end()) return it->second;
    return 0;
}

// ============================================================ Action queue and main loop
void queueScript(DisplayObject* clip, FrameScript fn) { if (fn) g_player.queue.push_back({clip, fn, nullptr}); }
void queueMethod(DisplayObject* clip, Str* m) { g_player.queue.push_back({clip, nullptr, m}); }

void beginTimeline(Act& a, DisplayObject* clip) {
    MovieClip* c = clip->kind == OK_CLIP ? (MovieClip*)clip : clip->parent;
    if (!c) c = vm.root;
    if (!c->tscope) c->tscope = gcNew<Scope>(vm.globalScope, SC_TARGET, (Object*)c);
    a.scope = c->tscope; a.thisv = Value((Object*)c); a.target = c; a.callee = nullptr;
}

void runActions() {
    int guard = 0;
    while (!g_player.queue.empty() && guard++ < 1000000) {
        Player::Queued q = g_player.queue.front(); g_player.queue.pop_front();
        if (q.clip->removed) continue;
        Value* sp0 = vm.sp;
        if (q.fn) q.fn(q.clip);
        else {
            Value f = getProp(q.clip, q.method);
            if (f.isObj() && f.o->kind == OK_FUNCTION) {
                Act a; beginTimeline(a, q.clip);
                callValue(a, f, Value((Object*)q.clip), 0, nullptr);
            }
        }
        vm.sp = sp0;
    }
}

void runFrameClip(MovieClip* c) {
    if (!c->initialized) { c->initialized = true; queueMethod(c, atoms::onLoad); }
    else queueMethod(c, atoms::onEnterFrame);
    if (c->playing) runFrameInternal(c, true, true);
}

void runFrame() {
    g_player.tick++;
    MovieClip* prev = nullptr;
    MovieClip* c = g_player.execHead;
    while (c) {
        MovieClip* next = c->execNext;
        if (c->removed) {
            if (prev) prev->execNext = next; else g_player.execHead = next;
            c->execNext = nullptr; c->inExec = false;
        } else { runFrameClip(c); prev = c; }
        c = next;
    }
    runActions();
    updateBoundTexts();
}

void playerInit() {
    g_rootTracers.push_back([](Tracer& tr) {
        for (MovieClip* c = g_player.execHead; c; c = c->execNext) tr.mark(c);
        for (auto* o : g_player.keyListeners) tr.mark(o);
        for (auto* o : g_player.mouseListeners) tr.mark(o);
        for (auto* t : g_player.boundTexts) tr.mark(t);
        tr.mark(g_player.hoverButton); tr.mark(g_player.pressButton);
    });
    MovieClip* root = newClip(&g_movie.rootDef);
    root->name = atoms::empty;
    vm.root = root;
    root->setMatrix(Mat());
    addToExec(root);
}

// ============================================================ Timeline actions
static MovieClip* targetClip(Act& a) {
    DisplayObject* t = a.target;
    if (t && t->removed) t = doResolveRemoved(t);
    if (!t) return nullptr;
    return t->kind == OK_CLIP ? (MovieClip*)t : nullptr;
}
void actGotoFrame(Act& a, int frame0) { if (MovieClip* c = targetClip(a)) gotoFrame(c, frame0 + 1, true); }
void actGotoFrame2(Act& a, const Value& f, bool play, int bias) {
    MovieClip* c = targetClip(a); if (!c) return;
    MovieClip* t; int fr = frameFromValue(c, f, &t);
    if (fr == 0 && !(f.t == T_NUM)) return;
    fr += bias;
    if (fr > 0) gotoFrame(t, fr, !play);
}
void actGotoLabel(Act& a, Str* label) {
    MovieClip* c = targetClip(a); if (!c) return;
    std::string l = label->s; for (auto& ch : l) ch = (char)tolower((unsigned char)ch);
    auto it = c->def->labels.find(l);
    if (it != c->def->labels.end()) gotoFrame(c, it->second, true);
}
void actStop(Act& a) { if (MovieClip* c = targetClip(a)) { c->playing = false; stopStream(c); } }
void actPlay(Act& a) { if (MovieClip* c = targetClip(a)) c->playing = true; }
void actNextFrame(Act& a) { if (MovieClip* c = targetClip(a)) { if (c->cur < c->def->nframes) gotoFrame(c, c->cur + 1, true); else { c->playing = false; } } }
void actPrevFrame(Act& a) { if (MovieClip* c = targetClip(a)) { if (c->cur > 1) gotoFrame(c, c->cur - 1, true); else c->playing = false; } }
void actGetURL(Act& a, const char* url, const char* target) {
    (void)a;
    std::string u = url;
    if (u.compare(0, 10, "FSCommand:") == 0) fscommand(u.substr(10), target);
}
void actRemoveSprite(Act& a, const Value& target) {
    Value t = resolveTargetPath(a, target);
    if (t.isObj() && t.o->isDisplay()) removeMovieClip((DisplayObject*)t.o);
}
Value actGetProperty(Act& a, const Value& target, const Value& idx) {
    double i = toNum(idx);
    Value t = (target.t == T_STR && target.s->s.empty()) ? Value((Object*)a.target) : resolveTargetPath(a, target);
    if (!t.isObj() || !t.o->isDisplay() || !(i >= 0 && i < 22)) return VUNDEF;
    return getDisplayProp((DisplayObject*)t.o, (int)i);
}
void actSetProperty(Act& a, const Value& target, const Value& idx, const Value& v) {
    double i = toNum(idx);
    Value t = (target.t == T_STR && target.s->s.empty()) ? Value((Object*)a.target) : resolveTargetPath(a, target);
    if (!(i >= 0 && i < 22)) return;
    if (!t.isObj() || !t.o->isDisplay()) { if ((int)i == 19 || (int)i == 16) setDisplayProp(vm.root, (int)i, v); return; }
    setDisplayProp((DisplayObject*)t.o, (int)i, v);
}

// ============================================================ Matrices and bounds
Mat worldMatrix(DisplayObject* d) {
    Mat m = d->m;
    for (DisplayObject* p = d->parent; p; p = p->parent) m = p->m * m;
    extern Mat g_stageMatrix;
    return m;
}

static void growRect(RectF& r, const Mat& m, const RectF& b) {
    double xs[4] = {b.x0, b.x1, b.x0, b.x1}, ys[4] = {b.y0, b.y0, b.y1, b.y1};
    for (int i = 0; i < 4; i++) {
        double x, y; m.apply(xs[i], ys[i], x, y);
        if (!r.valid() && r.x0 == 0 && r.x1 == 0 && r.y0 == 0 && r.y1 == 0) { r.x0 = r.x1 = x; r.y0 = r.y1 = y; r.x1 += 1e-9; r.y1 += 1e-9; }
        else { r.x0 = std::min(r.x0, x); r.x1 = std::max(r.x1, x); r.y0 = std::min(r.y0, y); r.y1 = std::max(r.y1, y); }
    }
}
static void boundsInto(DisplayObject* d, const Mat& m, RectF& r) {
    switch (d->kind) {
        case OK_SHAPE: growRect(r, m, ((ShapeDef*)((ShapeObj*)d)->def)->bounds); break;
        case OK_TEXT: growRect(r, m, ((TextDef*)((ShapeObj*)d)->def)->bounds); break;
        case OK_MORPH: growRect(r, m, ((MorphDef*)((ShapeObj*)d)->def)->b0); break;
        case OK_EDITTEXT: growRect(r, m, ((EditTextObj*)d)->def->bounds); break;
        case OK_BITMAP: { auto* b = ((BitmapObj*)d)->bd; if (b) { RectF rb; rb.x1 = b->w; rb.y1 = b->h; growRect(r, m, rb); } break; }
        case OK_CLIP: for (auto* k : ((MovieClip*)d)->kids) boundsInto(k, m * k->m, r); break;
        case OK_BUTTON: for (auto* k : ((ButtonObj*)d)->kids) boundsInto(k, m * k->m, r); break;
        default: break;
    }
}
RectF localBounds(DisplayObject* d) { RectF r; boundsInto(d, d->m, r); return r; }
RectF stageBounds(DisplayObject* d) { RectF r; boundsInto(d, worldMatrix(d), r); return r; }
// a button's hit area in stage coordinates (for buttons that draw nothing themselves: invisible hit zones)
RectF hitStageBounds(ButtonObj* b) { RectF r; Mat w = worldMatrix(b); for (auto* k : b->hitKids) boundsInto(k, w * k->m, r); return r; }

// ============================================================ hitTest
static bool pointInPolys(const std::vector<std::vector<float>>& polys, double x, double y) {
    for (auto& poly : polys) {
        bool inside = false; float sx = 0, sy = 0, px = 0, py = 0; bool have = false;
        for (size_t i = 0; i + 1 < poly.size(); i += 2) {
            float qx = poly[i], qy = poly[i + 1];
            if (std::isnan(qx)) {   // new contour: close the previous one
                if (have) { if (((py > y) != (sy > y)) && (x < (sx - px) * (y - py) / (sy - py) + px)) inside = !inside; }
                have = false; continue;
            }
            if (!have) { sx = px = qx; sy = py = qy; have = true; continue; }
            if (((py > y) != (qy > y)) && (x < (qx - px) * (y - py) / (qy - py) + px)) inside = !inside;
            px = qx; py = qy;
        }
        if (have) { if (((py > y) != (sy > y)) && (x < (sx - px) * (y - py) / (sy - py) + px)) inside = !inside; }
        if (inside) return true;
    }
    return false;
}
// ---- exact shape hit test, ported from Ruffle (render/src/shape_utils.rs): winding numbers on the SWF edges in
// twips (one-sided fill edges only; even-odd per style layer), strokes with half their width (at least 1 px on
// screen). Flattening the curves (as the hit polygons did) missed up to ~1 px near curved edges: creep/blood
// tests (hitTest(x, y, true)) disagreed with Flash on the borders of the splats.
static int windingLine(int64_t px, int64_t py, int64_t ax, int64_t ay, int64_t bx, int64_t by) {
    int64_t d0x = px - ax, d0y = py - ay, d1x = bx - ax, d1y = by - ay;
    if (ay <= py && py < by && d1x * d0y >= d1y * d0x) return 1;
    if (by <= py && py < ay && d1x * d0y <= d1y * d0x) return -1;
    return 0;
}
static void solveQuadratic(double a, double b, double c, double& t0, double& t1) {
    if (std::fabs(a) <= 1e-6) { if (b >= 0) { t0 = NAN; t1 = -c / b; } else { t0 = -c / b; t1 = NAN; } return; }
    double disc = b * b - 4 * a * c;
    if (disc < 0) { t0 = t1 = NAN; return; }
    disc = std::sqrt(disc);
    if (b >= 0) { t0 = (-b - disc) / (2 * a); t1 = (-b + disc) / (2 * a); }
    else { double r0 = (-b + disc) / (2 * a), r1 = (-b - disc) / (2 * a); t0 = r1; t1 = r0; }
}
static int windingCurve(int64_t px, int64_t py, int64_t bx0, int64_t by0, int64_t cx0, int64_t cy0, int64_t ax0, int64_t ay0) {
    int64_t d0x = bx0 - px, d0y = by0 - py, d1x = cx0 - px, d1y = cy0 - py, d2x = ax0 - px, d2y = ay0 - py;
    if ((d0y < 0 && d1y < 0 && d2y < 0) || (d0y > 0 && d1y > 0 && d2y > 0) || (d0x <= 0 && d1x <= 0 && d2x <= 0)) return 0;
    double x0 = (double)d0x, y0 = (double)d0y, x1 = (double)d1x, y1 = (double)d1y, x2 = (double)d2x, y2 = (double)d2y;
    double a = y0 - 2.0 * y1 + y2, b = 2.0 * (y1 - y0), c = y0;
    double t0, t1; solveQuadratic(a, b, c, t0, t1);
    bool v0 = std::isfinite(t0), v1 = std::isfinite(t1);
    if (!v0 && !v1) return 0;
    int w = 0;
    double ax = x0 - 2.0 * x1 + x2, bx = 2.0 * (x1 - x0);
    double te = -0.5 * b / a;
    bool mono = te <= 0.0 || te >= 1.0;   // (NaN when a == 0: not monotonic, as in Ruffle)
    auto in = [](double lo, double hi) { return lo <= 0.0 && 0.0 < hi; };
    if (a >= 0.0) {
        double ymin = mono ? std::min(y0, y2) : a * te * te + b * te + c;
        if (v0 && in(ymin, y0)) { double x = x0 + bx * t0 + ax * t0 * t0; if (x > 0.0) w += 1; }
        if (v1 && in(ymin, y2)) { double x = x0 + bx * t1 + ax * t1 * t1; if (x > 0.0) w -= 1; }
    } else {
        double ymax = mono ? std::max(y0, y2) : a * te * te + b * te + c;
        if (v1 && in(y0, ymax)) { double x = x0 + bx * t1 + ax * t1 * t1; if (x > 0.0) w -= 1; }
        if (v0 && in(y2, ymax)) { double x = x0 + bx * t0 + ax * t0 * t0; if (x > 0.0) w += 1; }
    }
    return w;
}
static bool strokeLineHit(double px, double py, double x0, double y0, double x1, double y1, double w, double w2) {
    if (px < std::min(x0, x1) - w || px > std::max(x0, x1) + w || py < std::min(y0, y1) - w || py > std::max(y0, y1) + w) return false;
    double abx = x1 - x0, aby = y1 - y0, apx = px - x0, apy = py - y0, dotA = abx * apx + aby * apy, dist;
    if (dotA <= 0) dist = apx * apx + apy * apy;
    else {
        double bpx = px - x1, bpy = py - y1, dotB = abx * bpx + aby * bpy;
        if (dotB >= 0) dist = bpx * bpx + bpy * bpy;
        else { double len = abx * abx + aby * aby, ex = apx - dotA * abx / len, ey = apy - dotA * aby / len; dist = ex * ex + ey * ey; }
    }
    return dist <= w2;
}
static bool strokeCurveHit(double px, double py, double x0, double y0, double x1, double y1, double x2, double y2, double w, double w2) {
    if (px < std::min({x0, x1, x2}) - w || px > std::max({x0, x1, x2}) + w || py < std::min({y0, y1, y2}) - w || py > std::max({y0, y1, y2}) + w) return false;
    double ax = x1 - x0, ay = y1 - y0, bx = x2 - x1 - ax, by = y2 - y1 - ay, mx = x0 - px, my = y0 - py;
    double a = bx * bx + by * by, b = 3.0 * (ax * bx + ay * by), c = 2.0 * (ax * ax + ay * ay) + (mx * bx + my * by), d = mx * ax + my * ay;
    auto distAt = [&](double t) { double u = 1 - t, cx = u * u * x0 + 2 * u * t * x1 + t * t * x2, cy = u * u * y0 + 2 * u * t * y1 + t * t * y2; return (cx - px) * (cx - px) + (cy - py) * (cy - py); };
    double dist = std::min(distAt(0.0), distAt(1.0));
    // roots of a t^3 + b t^2 + c t + d (Ruffle solve_cubic: Cardano)
    double roots[3]; int nr = 0;
    if (std::fabs(a) <= 1e-6) { double t0, t1; solveQuadratic(b, c, d, t0, t1); roots[nr++] = t0; roots[nr++] = t1; }
    else {
        double p = (b * b - 3.0 * a * c) / (9.0 * a * a), q = (9.0 * a * b * c - 27.0 * a * a * d - 2.0 * b * b * b) / (54.0 * a * a * a);
        double offset = b / (3.0 * a), disc = p * p * p - q * q;
        if (disc > 0.0) {
            double theta = std::acos(q / (p * std::sqrt(p))), r = 2.0 * std::sqrt(p);
            roots[nr++] = r * std::cos(theta / 3.0) - offset; roots[nr++] = r * std::cos((theta + 2.0 * kPi) / 3.0) - offset; roots[nr++] = r * std::cos((theta + 4.0 * kPi) / 3.0) - offset;
        } else {
            double g1 = std::cbrt(q + std::sqrt(-disc)), g2 = std::cbrt(q - std::sqrt(-disc));
            roots[nr++] = g1 + g2 - offset;
            if (disc == 0.0) roots[nr++] = -0.5 * (g1 + g2) - offset;
        }
    }
    for (int i = 0; i < nr; i++) if (roots[i] >= 0 && roots[i] <= 1) dist = std::min(dist, distAt(roots[i]));
    return dist <= w2;
}
// point in the shape's local twips; minWidth = 1 screen pixel in local twips
static bool shapeHitTwips(const ShapeDef* s, int64_t px, int64_t py, double minWidth) {
    int winding = 0;
    for (const HitEdge& e : s->hitEdges) {
        if (e.kind == 2) { if (winding & 1) return true; winding = 0; continue; }
        if (e.kind == 0) {
            if (e.f1) { if (!e.f0) winding += windingLine(px, py, e.x0, e.y0, e.x1, e.y1); }
            else if (e.f0) winding += windingLine(px, py, e.x1, e.y1, e.x0, e.y0);
            if (e.stroke >= 0) { double w = 0.5 * std::max((double)e.stroke, minWidth); if (strokeLineHit((double)px, (double)py, e.x0, e.y0, e.x1, e.y1, w, w * w)) return true; }
        } else {
            if (e.f1) { if (!e.f0) winding += windingCurve(px, py, e.x0, e.y0, e.cx, e.cy, e.x1, e.y1); }
            else if (e.f0) winding += windingCurve(px, py, e.x1, e.y1, e.cx, e.cy, e.x0, e.y0);
            if (e.stroke >= 0) { double w = 0.5 * std::max((double)e.stroke, minWidth); if (strokeCurveHit((double)px, (double)py, e.x0, e.y0, e.cx, e.cy, e.x1, e.y1, w, w * w)) return true; }
        }
    }
    return winding & 1;
}
static bool hitShape(DisplayObject* d, const Mat& world, double gx, double gy, bool shapeFlag) {
    if (!d->visible && shapeFlag) return false;
    switch (d->kind) {
        case OK_CLIP: {
            // in display order; a mask (clipDepth) that isn't hit hides the children it masks (Ruffle hit_test_shape)
            auto& kids = ((MovieClip*)d)->kids;
            int clipDepth = 0;
            for (DisplayObject* k : kids) {
                if (k->clipDepth > 0) { clipDepth = hitShape(k, world * k->m, gx, gy, shapeFlag) ? 0 : k->clipDepth; continue; }
                if (k->depth >= clipDepth && hitShape(k, world * k->m, gx, gy, shapeFlag)) return true;
            }
            return false;
        }
        case OK_BUTTON: {
            for (auto* k : ((ButtonObj*)d)->hitKids) if (hitShape(k, world * k->m, gx, gy, shapeFlag)) return true;
            return false;
        }
        default: break;
    }
    Mat inv; if (!world.invert(inv)) return false;
    double lx, ly; inv.apply(gx, gy, lx, ly);
    if (d->kind == OK_SHAPE) {
        ShapeDef* s = (ShapeDef*)((ShapeObj*)d)->def;
        if (lx < s->bounds.x0 || lx > s->bounds.x1 || ly < s->bounds.y0 || ly > s->bounds.y1) return false;
        if (!shapeFlag) return true;
        shapeEnsure(s);
        // global point in whole twips (as Ruffle), then local twips rounded
        double tgx = std::round(gx * 20.0) / 20.0, tgy = std::round(gy * 20.0) / 20.0;
        inv.apply(tgx, tgy, lx, ly);
        double sx = std::sqrt(inv.a * inv.a + inv.b * inv.b), sy = std::sqrt(inv.c * inv.c + inv.d * inv.d);
        return shapeHitTwips(s, (int64_t)std::llround(lx * 20.0), (int64_t)std::llround(ly * 20.0), 20.0 * std::max(sx, sy));
    }
    RectF b; boundsInto(d, Mat(), b);
    return lx >= b.x0 && lx <= b.x1 && ly >= b.y0 && ly <= b.y1;
}
// (testing) eval hitdebug <path> x y: every shape under the clip whose bounds contain the point (_root coordinates)
static void hitDebugRec(DisplayObject* d, const Mat& world, double gx, double gy) {
    if (d->kind == OK_CLIP) { for (auto* k : ((MovieClip*)d)->kids) hitDebugRec(k, world * k->m, gx, gy); return; }
    Mat inv; if (!world.invert(inv)) return;
    double lx, ly; inv.apply(gx, gy, lx, ly);
    if (d->kind == OK_SHAPE) {
        ShapeDef* s = (ShapeDef*)((ShapeObj*)d)->def; shapeEnsure(s);
        if (lx < s->bounds.x0 || lx > s->bounds.x1 || ly < s->bounds.y0 || ly > s->bounds.y1) return;
        logf("[hitdbg] %s shape %d local %.4f %.4f vis=%d poly=%d exact=%d\n", targetPath(d->parent)->s.c_str(), s->id, lx, ly, (int)d->visible,
             (int)pointInPolys(shapeHitPolys(s), lx, ly), (int)hitShape(d, world, gx, gy, true));
    }
}
void debugHit(DisplayObject* d, double x, double y) { double gx = x, gy = y; if (vm.root) vm.root->m.apply(gx, gy, gx, gy); hitDebugRec(d, worldMatrix(d), gx, gy); }
bool hitTestPoint(DisplayObject* d, double gx, double gy, bool shapeFlag) {
    if (d->removed) return false;
    Mat w = worldMatrix(d);
    if (!shapeFlag) {
        RectF b; boundsInto(d, w, b);
        return gx >= b.x0 && gx <= b.x1 && gy >= b.y0 && gy <= b.y1;
    }
    return hitShape(d, w, gx, gy, true);
}

// ============================================================ Buttons
static void buttonSetState(ButtonObj* b, int state) {
    b->state = state;
    for (auto* k : b->kids) markRemoved(k);
    b->kids.clear();
    uint8_t mask = state == 0 ? 1 : state == 1 ? 2 : 4;
    for (auto& r : b->def->records) {
        if (!(r.states & mask)) continue;
        DisplayObject* d = createForChar(r.id);
        if (!d) continue;
        d->depth = r.depth; d->setMatrix(r.m); d->cx = r.cx; d->filters = r.filters;
        d->parent = nullptr;
        b->kids.push_back(d);
        if (d->kind == OK_CLIP) {
            MovieClip* mc = (MovieClip*)d;
            mc->parent = b->parent;   // script context: the clip containing the button
            postInstantiateClip(mc, true, nullptr);
        }
    }
    std::sort(b->kids.begin(), b->kids.end(), [](DisplayObject* x, DisplayObject* y) { return x->depth < y->depth; });
    if (b->hitKids.empty()) {
        for (auto& r : b->def->records) {
            if (!(r.states & 8)) continue;
            DisplayObject* d = createForChar(r.id); if (!d) continue;
            d->depth = r.depth; d->setMatrix(r.m); b->hitKids.push_back(d);
        }
    }
}

static void runButtonConds(ButtonObj* b, uint16_t condMask, int key) {
    if (!b->parent) return;
    for (auto& c : b->def->conds) {
        bool match = key ? ((c.cond >> 9) == key) : (c.cond & condMask);
        if (match && c.script) queueScript(b->parent, c.script);
    }
}

static void collectButtons(DisplayObject* d, std::vector<ButtonObj*>& out, bool visibleOnly) {
    if (visibleOnly && !d->visible) return;
    if (d->kind == OK_BUTTON) { out.push_back((ButtonObj*)d); return; }
    if (d->kind == OK_CLIP) for (auto* k : ((MovieClip*)d)->kids) collectButtons(k, out, visibleOnly);
}

void keyEvent(int code, int ascii, bool down) {
    if (code > 0 && code < 256) g_player.keys[code] = down;
    if (down) { g_player.lastKey = code; g_player.lastAscii = ascii; }
    // Key listeners
    std::vector<Object*> ls = g_player.keyListeners;
    for (auto* o : ls) {
        Value f = getProp(o, intern(down ? "onKeyDown" : "onKeyUp"));
        if (f.isObj()) { Act a{vm.globalScope, Value(o), vm.root, nullptr}; callValue(a, f, Value(o), 0, nullptr); }
    }
    if (down) {
        int bkey = 0;
        switch (code) {
            case 37: bkey = 1; break; case 39: bkey = 2; break; case 36: bkey = 3; break; case 35: bkey = 4; break;
            case 45: bkey = 5; break; case 46: bkey = 6; break; case 8: bkey = 8; break; case 13: bkey = 13; break;
            case 38: bkey = 14; break; case 40: bkey = 15; break; case 33: bkey = 16; break; case 34: bkey = 17; break;
            case 9: bkey = 18; break; case 27: bkey = 19; break;
            default: bkey = ascii; break;
        }
        if (bkey) {
            std::vector<ButtonObj*> bs; collectButtons(vm.root, bs, false);
            for (auto* b : bs) if (b->enabled && !b->removed) runButtonConds(b, 0, bkey);
        }
    }
    runActions();
}

static ButtonObj* buttonAt(double x, double y) {
    std::vector<ButtonObj*> bs; collectButtons(vm.root, bs, true);
    for (size_t i = bs.size(); i-- > 0;) {
        ButtonObj* b = bs[i];
        if (!b->enabled) continue;
        Mat w = worldMatrix(b);
        for (auto* k : b->hitKids) if (hitShape(k, w * k->m, x, y, true)) return b;
    }
    return nullptr;
}

// touch screens (logic thread): is there a button under this stage point / is any button on screen at all?
bool touchButtonAt(double x, double y) { return buttonAt(x, y) != nullptr; }
// controller menu navigation (menuui.cpp)
void visibleButtons(std::vector<ButtonObj*>& out) { collectButtons(vm.root, out, true); }
ButtonObj* topButtonAt(double x, double y) { return buttonAt(x, y); }
void debugListButtons() {   // (testing) eval buttons
    std::vector<ButtonObj*> bs; collectButtons(vm.root, bs, true);
    for (auto* b : bs) {
        RectF r = stageBounds(b);
        double a = 1; for (DisplayObject* p = b; p; p = p->parent) a *= p->cx.am;
        RectF h = hitStageBounds(b);
        if (r.valid()) { ButtonObj* t = buttonAt((r.x0 + r.x1) / 2, (r.y0 + r.y1) / 2); logf("[button]   top at center: %s %s\n", t == b ? "itself" : t ? targetPath(t)->s.c_str() : "none", t && t != b ? "(other)" : ""); }
        logf("[button] %s en=%d alpha=%.2f rect=%.0f,%.0f-%.0f,%.0f hit=%.0f,%.0f-%.0f,%.0f kids=%d conds=%d\n", targetPath(b)->s.c_str(), (int)b->enabled, a, r.x0, r.y0, r.x1, r.y1, h.x0, h.y0, h.x1, h.y1, (int)b->kids.size(), (int)b->def->conds.size());
    }
}
bool touchAnyButton() {
    std::vector<ButtonObj*> bs; collectButtons(vm.root, bs, true);
    for (auto* b : bs) if (b->enabled && !b->removed) return true;
    return false;
}
void mouseEvent(double x, double y, int type) {
    g_player.mouseX = x; g_player.mouseY = y;
    ButtonObj* over = buttonAt(x, y);
    if (over != g_player.hoverButton) {
        if (g_player.hoverButton && !g_player.hoverButton->removed) { buttonSetState(g_player.hoverButton, 0); runButtonConds(g_player.hoverButton, 0x02, 0); }
        g_player.hoverButton = over;
        if (over) { buttonSetState(over, g_player.mouseDown && g_player.pressButton == over ? 2 : 1); runButtonConds(over, 0x01, 0); }
    }
    const char* ev = type == 0 ? "onMouseMove" : type == 1 ? "onMouseDown" : "onMouseUp";
    if (type == 1) {
        g_player.mouseDown = true;
        if (over) { g_player.pressButton = over; buttonSetState(over, 2); runButtonConds(over, 0x04, 0); }
    } else if (type == 2) {
        g_player.mouseDown = false;
        if (g_player.pressButton) {
            ButtonObj* p = g_player.pressButton; g_player.pressButton = nullptr;
            if (p == over && !p->removed) { buttonSetState(p, 1); runButtonConds(p, 0x08, 0); }
            else if (!p->removed) { buttonSetState(p, 0); runButtonConds(p, 0x100 | 0x40, 0); }
        }
    }
    std::vector<Object*> ls = g_player.mouseListeners;
    for (auto* o : ls) {
        Value f = getProp(o, intern(ev));
        if (f.isObj()) { Act a{vm.globalScope, Value(o), vm.root, nullptr}; callValue(a, f, Value(o), 0, nullptr); }
    }
    runActions();
}

// ============================================================ Variable-bound texts
static std::string stripHtml(const std::string& s) {
    std::string o; bool tag = false;
    for (size_t i = 0; i < s.size(); i++) {
        char c = s[i];
        if (c == '<') { tag = true; if (s.compare(i, 3, "<br") == 0 || s.compare(i, 4, "</p>") == 0) o += '\n'; continue; }
        if (c == '>') { tag = false; continue; }
        if (!tag) {
            if (c == '&') {
                if (s.compare(i, 4, "&lt;") == 0) { o += '<'; i += 3; continue; }
                if (s.compare(i, 4, "&gt;") == 0) { o += '>'; i += 3; continue; }
                if (s.compare(i, 5, "&amp;") == 0) { o += '&'; i += 4; continue; }
                if (s.compare(i, 6, "&quot;") == 0) { o += '"'; i += 5; continue; }
                if (s.compare(i, 6, "&apos;") == 0) { o += '\''; i += 5; continue; }
            }
            o += c;
        }
    }
    return o;
}

void updateBoundTexts() {
    auto& v = g_player.boundTexts;
    size_t w = 0;
    for (size_t i = 0; i < v.size(); i++) {
        EditTextObj* e = v[i];
        if (e->removed) continue;
        v[w++] = e;
        if (!e->parent) continue;
        const std::string& var = e->def->variable;
        Value val;
        size_t sep = var.find_last_of(":.");
        if (sep == std::string::npos) {
            Str* k = intern(var);
            if (!hasProp(e->parent, k)) continue;
            val = getProp(e->parent, k);
        } else {
            Act a{vm.globalScope, VUNDEF, e->parent, nullptr};
            Value t = resolveTargetPath(a, mkstr(var.substr(0, sep)));
            if (!t.isObj()) continue;
            Str* k = intern(var.substr(sep + 1));
            if (!hasProp(t.o, k)) continue;
            val = getProp(t.o, k);
        }
        std::string s = toStr(val)->s;
        if (e->def->html) s = stripHtml(s);
        e->setText(s);
    }
    v.resize(w);
}

void fscommand(const std::string& cmd, const std::string& arg) {
    if (cmd == "quit") g_player.quit = true;
    (void)arg;
}

// Pixel buffer pool: each room creates several 1500x1116 BitmapData (6.7 MB); reusing those from
// previous rooms avoids page faults and memory growth until the next collection.
static std::vector<std::vector<uint32_t>> g_pxPool; static size_t g_pxPoolBytes = 0;
void bdPoolTake(std::vector<uint32_t>& v, size_t n) {
    for (size_t i = g_pxPool.size(); i-- > 0;) {
        size_t c = g_pxPool[i].capacity();
        if (c >= n && c <= n + n / 4) { g_pxPoolBytes -= c * 4; v.swap(g_pxPool[i]); g_pxPool.erase(g_pxPool.begin() + i); return; }
    }
}
void bdPoolGive(std::vector<uint32_t>& v) {
    size_t c = v.capacity();
    if (c * 4 >= (256u << 10)) {
        g_pxPool.emplace_back(); g_pxPool.back().swap(v); g_pxPoolBytes += c * 4;
        while (g_pxPoolBytes > g_lim.pxPoolBytes && !g_pxPool.empty()) { g_pxPoolBytes -= g_pxPool.front().capacity() * 4; g_pxPool.erase(g_pxPool.begin()); }
    }
    std::vector<uint32_t>().swap(v);
}
BitmapData::~BitmapData() { extern void bitmapGone(BitmapData*); bitmapGone(this); bdPoolGive(px); }

}  // namespace fl
