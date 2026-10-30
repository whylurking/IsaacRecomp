// The Gone's unlocks and the things they bring (included by mod_thegone.cpp, inside its anonymous namespace).
//
// The Gone himself is locked until a run clears three floors in a row without taking a hit or picking up a heart of
// any kind. Runs as The Gone unlock four more things, each shown with its own "You unlocked" card (the game's
// secret-card animation, _root.unl) and listed with the game's found secrets (menu, _root.lockor while the menus show):
//   Mom's Heart          -> Negative Isaac (a card): a 15-second shield, as Book of Shadows (the game's playsave)
//   Mom's Heart, with no item but the starting one -> Nothing (treasure rooms): non-boss enemies may vanish (10%)
//   Isaac (Cathedral)    -> The Void Shard (devil rooms): enemy shots miss 25%; when hurt, 5%: 50 damage to the room
//   Satan (Sheol)        -> Black Mirror (treasure rooms, active, 3 charges): the room in negative, enemies fight
// Kept in saves/thegone_<game>.txt (never in the game's save). With GenesisConsole on nothing unlocks; its "unlock"
// command unlocks everything until the game is closed.
//
// The new things are given numbers past the game's: items 230-232, card 120 (its colour number), secret cards
// 150-154. The game's own code handles them (pools, pedestals, prices, pickup, the HUD, charges) and only their
// pictures are missing: every clip the game sends to one of those frame numbers (past its last frame; recorded in
// g_overFrames by gotoFrame) is drawn with the mod's picture instead (SnapOv hideKids + extra), and the HUD's active
// item clip (frames by the game's short active number, 231 - 40 = 191) the same while the Black Mirror is held.

enum { U_GONE, U_CARD, U_SHARD, U_MIRROR, U_NOTHING, U_N };
const char* kUKey[U_N] = {"the_gone", "negative_isaac", "void_shard", "black_mirror", "nothing"};
const char* kUName[U_N] = {"THE GONE", "NEGATIVE ISAAC", "THE VOID SHARD", "BLACK MIRROR", "NOTHING"};
const char* kUSub[U_N] = {"ONE HIT AND HE IS DONE", "A DARK CARD APPEARS", "SOLD BY THE DEVIL", "IT IS NOT YOU", "IT DOES NOTHING"};
const int kItemId[3] = {230, 231, 232};             // The Void Shard, Black Mirror, Nothing
const int kItemUnl[3] = {U_SHARD, U_MIRROR, U_NOTHING};
const char* kItemName[3] = {"The Void Shard", "Black Mirror", "Nothing"};
const char* kItemSub[3] = {"Shots slip through you", "SPACE to use", "..."};
const int kMirrorShort = 231 - 40;                  // the game's active-item number (itn: 40 past 54)
const int kCardCol = 120;
const int kSecret0 = 150;
inline int itemIndex(double it) { for (int k = 0; k < 3; k++) if (it == kItemId[k]) return k; return -1; }

// ---- saved state
struct UnlState { bool saved[U_N] = {}, session[U_N] = {}, coll[3] = {}; bool loaded = false; std::string game; };
UnlState g_u;
std::string unlFile() { return g_modsSaveDir + "thegone_" + game() + ".txt"; }
void unlLoad() {
    if (g_u.loaded && g_u.game == game()) return;
    bool sess[U_N]; for (int i = 0; i < U_N; i++) sess[i] = g_u.game == game() && g_u.session[i];
    g_u = UnlState(); g_u.loaded = true; g_u.game = game();
    for (int i = 0; i < U_N; i++) g_u.session[i] = sess[i];
    FILE* f = platformFopen(unlFile(), "rb");
    if (!f) return;
    char buf[256];
    while (fgets(buf, sizeof buf, f)) {
        std::string l = buf; size_t eq = l.find('=');
        if (l.empty() || l[0] == ';' || eq == std::string::npos) continue;
        std::string k = l.substr(0, eq); while (!k.empty() && k.back() == ' ') k.pop_back();
        bool v = atoi(l.c_str() + eq + 1) != 0;
        for (int i = 0; i < U_N; i++) if (k == kUKey[i]) g_u.saved[i] = v;
        for (int j = 0; j < 3; j++) if (k == std::string("collected_") + kUKey[kItemUnl[j]]) g_u.coll[j] = v;
    }
    fclose(f);
}
void unlSave() {
    if (g_saveLocked) return;   // (GenesisConsole: nothing is saved)
    std::string tmp = unlFile() + ".tmp";
    FILE* f = platformFopen(tmp, "wb");
    if (!f) return;
    fprintf(f, "; The Gone (mod): what is unlocked in this game. 1 = unlocked / collected\n");
    for (int i = 0; i < U_N; i++) fprintf(f, "%s = %d\n", kUKey[i], g_u.saved[i] ? 1 : 0);
    for (int j = 0; j < 3; j++) fprintf(f, "collected_%s = %d\n", kUKey[kItemUnl[j]], g_u.coll[j] ? 1 : 0);
    bool ok = fclose(f) == 0;
    if (ok) platformReplaceFile(tmp, unlFile());
}
bool unlocked(int i) { unlLoad(); return g_u.saved[i] || g_u.session[i]; }
std::vector<int> g_popups;   // unlock cards waiting to be shown
void unlock(int i) {
    if (unlocked(i) || g_saveLocked) return;
    g_u.saved[i] = true; unlSave();
    g_popups.push_back(i);
    logf("thegone: unlocked %s\n", kUKey[i]);
}

// ---- own random numbers (the game's stay as they would be)
uint32_t g_rng = 0x2545F491u;
uint32_t rnd(uint32_t n) { g_rng ^= g_rng << 13; g_rng ^= g_rng >> 17; g_rng ^= g_rng << 5; g_rng += g_player.tick * 2654435761u; return n ? (g_rng >> 8) % n : 0; }

// ============================================================ pictures
// shapes made of polygons: each part a fill colour and an optional outline
struct Part { std::vector<float> pts; uint32_t fill; uint32_t line = 0; float lw = 0; };
uint16_t partsShape(const std::vector<Part>& parts) {
    auto* sd = new ShapeDef(); sd->type = CH_SHAPE;
    float bx0 = 1e9f, by0 = 1e9f, bx1 = -1e9f, by1 = -1e9f;
    for (const Part& p : parts) {
        ShapeLayer L;
        Path path; path.verbs.push_back(0); path.pts.push_back(p.pts[0]); path.pts.push_back(p.pts[1]);
        for (size_t i = 2; i + 1 < p.pts.size(); i += 2) { path.verbs.push_back(1); path.pts.push_back(p.pts[i]); path.pts.push_back(p.pts[i + 1]); }
        path.verbs.push_back(1); path.pts.push_back(p.pts[0]); path.pts.push_back(p.pts[1]);
        for (size_t i = 0; i + 1 < p.pts.size(); i += 2) {
            float m = p.lw / 2;
            bx0 = std::min(bx0, p.pts[i] - m); by0 = std::min(by0, p.pts[i + 1] - m); bx1 = std::max(bx1, p.pts[i] + m); by1 = std::max(by1, p.pts[i + 1] + m);
        }
        if (p.fill >> 24) { FillStyle fs; fs.type = 0; fs.argb = p.fill; L.fills.push_back(fs); L.fillPaths.push_back(path); }
        if (p.lw > 0) { LineStyle ls; ls.width = p.lw; ls.argb = p.line; ls.join = 0; L.lines.push_back(ls); L.linePaths.push_back(path); }
        sd->layers.push_back(L);
    }
    sd->bounds = {bx0, by0, bx1, by1};
    return synthAddChar(sd);
}
std::vector<float> xf(const std::vector<float>& p, double s, double ox, double oy) {
    std::vector<float> r(p.size());
    for (size_t i = 0; i + 1 < p.size(); i += 2) { r[i] = (float)(ox + p[i] * s); r[i + 1] = (float)(oy + p[i + 1] * s); }
    return r;
}
// The Void Shard: a dark crystal splinter (unit size: about 1 high, centred on 0, 0)
const std::vector<float> kShard = {0.02f, -0.56f, 0.24f, -0.2f, 0.19f, 0.18f, 0.06f, 0.56f, -0.15f, 0.3f, -0.25f, -0.06f, -0.1f, -0.36f};
const std::vector<float> kShardFacet = {0.02f, -0.56f, 0.24f, -0.2f, 0.04f, 0.02f, -0.1f, -0.36f};
const std::vector<float> kShardFacet2 = {0.04f, 0.02f, 0.19f, 0.18f, 0.06f, 0.56f};
const std::vector<float> kShardGlint = {-0.05f, -0.3f, 0.0f, -0.42f, 0.02f, -0.24f};
uint16_t shardShape(double size, bool grey, uint32_t ink = 0xff6e6a66u) {
    std::vector<Part> p;
    float lw = (float)(size * 0.085);
    if (grey) {   // (in the cards' drawing style: ink, a lighter facet, a few strokes around it)
        p.push_back({xf(kShard, size, 0, 0), (ink & 0x00ffffffu) | 0xff000000u});
        p.push_back({xf(kShardFacet, size, 0, 0), 0xffa8a39du});
        p.push_back({xf(kShardGlint, size, 0, 0), 0xffe6e2dcu});
        const float rays[][4] = {{-0.42f, -0.36f, -0.3f, -0.27f}, {0.42f, -0.4f, 0.31f, -0.3f}, {-0.46f, 0.16f, -0.33f, 0.13f}, {0.44f, 0.22f, 0.32f, 0.17f}};
        for (auto& r : rays) {
            float dx = r[2] - r[0], dy = r[3] - r[1], l = std::sqrt(dx * dx + dy * dy), nx = -dy / l * 0.025f, ny = dx / l * 0.025f;
            p.push_back({xf({r[0] + nx, r[1] + ny, r[2] + nx, r[3] + ny, r[2] - nx, r[3] - ny, r[0] - nx, r[1] - ny}, size, 0, 0), (ink & 0x00ffffffu) | 0xff000000u});
        }
        return partsShape(p);
    }
    p.push_back({xf(kShard, size, 0, 0), 0xff1d1226u, 0xff000000u, lw});
    p.push_back({xf(kShardFacet, size, 0, 0), 0xff5b3c86u});
    p.push_back({xf(kShardFacet2, size, 0, 0), 0xff38244fu});
    p.push_back({xf(kShardGlint, size, 0, 0), 0xffe8dcffu});
    p.push_back({xf(kShard, size, 0, 0), 0x00000000u, 0xff000000u, lw});
    return partsShape(p);
}
// a card (Negative Isaac): w x h centred on 0, 0
uint16_t cardShapeId(double w, double h, uint32_t fill, uint32_t line, float lw) {
    std::vector<float> r = {(float)(-w / 2), (float)(-h / 2), (float)(w / 2), (float)(-h / 2), (float)(w / 2), (float)(h / 2), (float)(-w / 2), (float)(h / 2)};
    return partsShape({{r, fill, line, lw}});
}

// bounds of what a sprite shows on a frame (its shapes and, one level down, its clips' first frames)
bool frameBounds(uint16_t id, int frame, RectF& out, int depthLeft = 3) {
    CharDef* cd = g_movie.get(id);
    if (!cd) return false;
    if (cd->type == CH_SHAPE) { out = ((ShapeDef*)cd)->bounds; return out.valid(); }
    if (cd->type != CH_SPRITE || depthLeft <= 0) return false;
    SpriteDef* sp = (SpriteDef*)cd; spriteEnsure(sp);
    std::map<int, const PlaceCmd*> live;
    for (int i = 0; i < frame && i < (int)sp->frames.size(); i++)
        for (auto& c : sp->frames[i]) {
            if (c.kind == FrameCmd::REMOVE) live.erase(c.depth);
            else if (c.kind == FrameCmd::PLACE && c.place >= 0) {
                const PlaceCmd& pc = sp->places[c.place];
                if (pc.hasId) live[pc.depth] = &pc;
                else if (live.count(pc.depth)) { static PlaceCmd tmp; tmp = *live[pc.depth]; if (pc.hasMatrix) tmp.m = pc.m; }   // (moves: keep the first matrix)
            }
        }
    bool any = false; RectF u{1e9, 1e9, -1e9, -1e9};
    for (auto& kv : live) {
        RectF r; if (!frameBounds(kv.second->id, 1, r, depthLeft - 1)) continue;
        const Mat& m = kv.second->m; double xs[4], ys[4];
        m.apply(r.x0, r.y0, xs[0], ys[0]); m.apply(r.x1, r.y0, xs[1], ys[1]); m.apply(r.x0, r.y1, xs[2], ys[2]); m.apply(r.x1, r.y1, xs[3], ys[3]);
        for (int k = 0; k < 4; k++) { u.x0 = std::min(u.x0, xs[k]); u.y0 = std::min(u.y0, ys[k]); u.x1 = std::max(u.x1, xs[k]); u.y1 = std::max(u.y1, ys[k]); }
        any = true;
    }
    if (any) out = u;
    return any;
}

// game pictures found on the way (their character numbers differ between the three games)
uint16_t g_itemSprite = 0;      // the items' pictures (frame = item number), as on pedestals and in the collection
uint16_t g_portraitShape = 0;   // Isaac on the character select ring (grey drawing)
uint16_t g_playerSprite = 0;    // the player
uint16_t g_sketchSprite = 0;   // the items as grey drawings (collection, pause list, death diary)
void learnItemSprite(MovieClip* c) { if (!g_itemSprite && c && c->def && c->def->nframes >= 100 && c->def->nframes < 260) g_itemSprite = c->charId; }
void learnSketchSprite(MovieClip* c) { if (!g_sketchSprite && c && c->def && c->def->nframes >= 100) g_sketchSprite = c->charId; }

MovieClip* newArt() { MovieClip* c = newDetachedClip(); return c; }
Object* g_artHold = nullptr;    // (an Array holding the pictures, so they stay alive)
void hold(DisplayObject* d) {
    if (!g_artHold) { g_artHold = gcNew<Array>(); g_artHold->proto = protos::array; gcAddRoot((GcObj**)&g_artHold); }
    ((Array*)g_artHold)->el.push_back(Value((Object*)d));
}
void unhold(DisplayObject* d) {
    if (!g_artHold) return;
    auto& el = ((Array*)g_artHold)->el;
    for (size_t i = 0; i < el.size(); i++) if (el[i].isObj() && el[i].o == (Object*)d) { el.erase(el.begin() + i); return; }
}
// an item's picture, in the coordinates of the sprite `spriteId` (sized and placed like its item 5, My Reflection)
// sketch: the grey drawings of the collection, the pause screen's list and the death diary (another sprite than the
// pedestals' coloured one)
DisplayObject* itemArt(int k, uint16_t spriteId, bool sketch) {
    if (k == 2) return nullptr;   // Nothing: no picture at all
    MovieClip* c = newArt();
    RectF ref; double cx = 0, cy = 0, size = 32;
    if (spriteId && frameBounds(spriteId, 5, ref)) { cx = (ref.x0 + ref.x1) / 2; cy = (ref.y0 + ref.y1) / 2; size = std::max(ref.x1 - ref.x0, ref.y1 - ref.y0); }
    if (k == 0) synthPut(c, shardShape(size * (sketch ? 0.85 : 0.95), sketch, 0xff58534fu), 1, cx, cy);
    else if (spriteId) {   // Black Mirror: My Reflection, in negative (as a sketch: drawn darker, a dark mirror)
        DisplayObject* m = synthPut(c, spriteId, 1, 0, 0);
        if (m && m->kind == OK_CLIP) { gotoFrame((MovieClip*)m, 5, true); }
        if (m && !sketch) { m->cx.rm = m->cx.gm = m->cx.bm = -1; m->cx.ra = m->cx.ga = m->cx.ba = 255; m->scriptColor = true; }
        if (m && sketch) { m->cx.rm = m->cx.gm = m->cx.bm = 0.3f; m->scriptColor = true; }
    }
    hold(c);
    return c;
}
// Negative Isaac. face: the HUD's card and the one Isaac holds up - a dark red card with his crying face in negative
// (the player's own head drawing, inverted); otherwise its back, on the floor: the cards' back (frame 7), which the
// floor pickup's red tint makes red. In the coordinates of the cards' sprite.
uint16_t g_headShape = 0;   // the player's head, front (learnt in a run)
DisplayObject* cardArt(uint16_t spriteId, bool face) {
    MovieClip* c = newArt();
    if (!face) {   // (the playing cards' red back; the original game has none: the tarot back, tinted red)
        DisplayObject* b = spriteId ? synthPut(c, spriteId, 1, 0, 0) : nullptr;
        if (b && b->kind == OK_CLIP) gotoFrame((MovieClip*)b, game() == "og" ? 7 : 70, true);
        hold(c);
        return c;
    }
    RectF ref; double cx = 0, cy = 0, w = 22, h = 30;
    if (spriteId && frameBounds(spriteId, 9, ref)) { cx = (ref.x0 + ref.x1) / 2; cy = (ref.y0 + ref.y1) / 2; w = (ref.x1 - ref.x0) * 0.86; h = (ref.y1 - ref.y0) * 0.9; }
    synthPut(c, cardShapeId(w, h, 0xff5a0f12u, 0xff000000u, (float)(w * 0.09)), 1, cx, cy);
    uint16_t pic = g_headShape ? g_headShape : g_portraitShape;
    CharDef* pd = pic ? g_movie.get(pic) : nullptr;
    if (pd && pd->type == CH_SHAPE) {
        RectF pr = ((ShapeDef*)pd)->bounds;
        double s = std::min(w * 0.82 / std::max(1.0, pr.x1 - pr.x0), h * 0.8 / std::max(1.0, pr.y1 - pr.y0));
        DisplayObject* f = synthPut(c, pic, 2, 0, 0);
        if (f) {
            f->m = Mat(); f->m.a = f->m.d = s; f->m.tx = cx - (pr.x0 + pr.x1) / 2 * s; f->m.ty = cy - (pr.y0 + pr.y1) / 2 * s; f->scriptXform = true;
            f->cx.rm = f->cx.gm = f->cx.bm = -1; f->cx.ra = f->cx.ga = f->cx.ba = 255; f->scriptColor = true;   // (in negative)
        }
    }
    hold(c);
    return c;
}

// ---- the "You unlocked" cards: the game's own card (paper and animation), with its heading, then the name in
// the character cards' lettering (between quotes), a picture and a line under it
struct SecretLayout { bool ok = false; uint16_t header = 0; double nameCy = 40, subCy = 160, picCy = 100, picH = 60, cx = 120, lineH = 20; uint32_t ink = 0xff6e6a66u; };
SecretLayout g_secret; bool g_secretTried = false;
// The cards: the paper (clips of the card's sprite, kept) and one shape of ink (heading, name, picture: hidden). From
// the first card: where the paper lies, and its heading ("YOU UNLOCKED": the ink on the line of its first letter).
void learnSecretLayout(uint16_t spriteId) {
    if (g_secretTried) return;
    g_secretTried = true;
    CharDef* cd = g_movie.get(spriteId);
    if (!cd || cd->type != CH_SPRITE) return;
    SpriteDef* sp = (SpriteDef*)cd; spriteEnsure(sp);
    ShapeDef* best = nullptr; double area = 0;
    double px0 = 1e9, py0 = 1e9, px1 = -1e9, py1 = -1e9;
    for (auto& c : sp->frames[0]) if (c.kind == FrameCmd::PLACE && c.place >= 0 && sp->places[c.place].hasId) {
        const PlaceCmd& pc = sp->places[c.place];
        CharDef* d = g_movie.get(pc.id);
        if (d && d->type == CH_SHAPE) { ShapeDef* s2 = (ShapeDef*)d; double ar = (s2->bounds.x1 - s2->bounds.x0) * (s2->bounds.y1 - s2->bounds.y0); if (ar > area) { area = ar; best = s2; } }
        else if (d && d->type == CH_SPRITE) {
            RectF r; if (!frameBounds(pc.id, 1, r)) continue;
            double xs[4], ys[4]; pc.m.apply(r.x0, r.y0, xs[0], ys[0]); pc.m.apply(r.x1, r.y0, xs[1], ys[1]); pc.m.apply(r.x0, r.y1, xs[2], ys[2]); pc.m.apply(r.x1, r.y1, xs[3], ys[3]);
            for (int k = 0; k < 4; k++) { px0 = std::min(px0, xs[k]); py0 = std::min(py0, ys[k]); px1 = std::max(px1, xs[k]); py1 = std::max(py1, ys[k]); }
        }
    }
    if (!best || px1 <= px0) { logf("thegone: the secret cards are not as expected\n"); return; }
    shapeEnsure(best);
    std::vector<SubP> ink; uint32_t inkColor = 0xff6e6a66u;
    for (auto& L : best->layers)
        for (size_t f = 0; f < L.fillPaths.size(); f++) {
            if (f < L.fills.size() && L.fills[f].type == 0) inkColor = L.fills[f].argb;
            for (auto& sp2 : subpaths(L.fillPaths[f])) ink.push_back(sp2);
        }
    if (ink.empty()) return;
    // the heading: the ink on the first line (the band of its first letter, top left)
    double top = 1e9; for (auto& q : ink) top = std::min(top, (double)q.y0);
    const SubP* first = nullptr;
    for (auto& q : ink) if (q.y0 < top + 8 && (!first || q.x0 < first->x0)) first = &q;
    double b0 = first->y0 - 2, b1 = first->y1 + 2;
    auto* hd = new ShapeDef(); hd->type = CH_SHAPE; ShapeLayer HL; FillStyle hf; hf.argb = inkColor; HL.fills.push_back(hf); Path all;
    double hx0 = 1e9, hx1 = -1e9;
    for (auto& q : ink) { double cy = (q.y0 + q.y1) / 2; if (cy < b0 || cy > b1) continue;
        all.verbs.insert(all.verbs.end(), q.p.verbs.begin(), q.p.verbs.end()); all.pts.insert(all.pts.end(), q.p.pts.begin(), q.p.pts.end());
        hx0 = std::min(hx0, (double)q.x0); hx1 = std::max(hx1, (double)q.x1); }
    HL.fillPaths.push_back(all); hd->layers.push_back(HL); hd->bounds = {hx0, b0, hx1, b1};
    g_secret.header = synthAddChar(hd);
    double lh = b1 - b0;
    g_secret.cx = (hx0 + hx1) / 2; g_secret.lineH = lh;
    g_secret.nameCy = b1 + lh * 0.7;
    g_secret.subCy = py1 - (py1 - py0) * 0.13;
    double picTop = g_secret.nameCy + lh * 0.8, picBot = g_secret.subCy - lh * 0.75;
    g_secret.picCy = (picTop + picBot) / 2; g_secret.picH = std::max(20.0, picBot - picTop);
    g_secret.ink = inkColor; g_secret.ok = true;
    logf("thegone: secret cards: paper %.0f-%.0f x %.0f-%.0f, heading %.0f-%.0f, name %.0f, picture %.0f (%.0f high), line %.0f\n",
         px0, px1, py0, py1, b0, b1, g_secret.nameCy, g_secret.picCy, g_secret.picH, g_secret.subCy);
}
uint16_t quoteShape(double x, double y, double k, uint32_t ink) {   // a pair of hand-drawn quote marks at (x, y)
    std::vector<Part> p;
    for (int i = 0; i < 2; i++) {
        double ox = x + i * 4.2 * k;
        p.push_back({{(float)(ox), (float)(y), (float)(ox + 2.6 * k), (float)(y - 0.4 * k), (float)(ox + 1.2 * k), (float)(y + 6 * k), (float)(ox - 0.4 * k), (float)(y + 5.6 * k)}, ink});
    }
    return partsShape(p);
}
DisplayObject* secretArt(int u, uint16_t spriteId) {
    learnSecretLayout(spriteId);
    MovieClip* c = newArt();
    int dp = 1;
    const SecretLayout& L = g_secret;
    uint32_t ink = L.ok ? L.ink : 0xff6e6a66u;
    double cx = L.cx;
    // the paper, as on the first card (the clips of its first frame)
    if (CharDef* cd = g_movie.get(spriteId)) if (cd->type == CH_SPRITE) {
        SpriteDef* sp = (SpriteDef*)cd; spriteEnsure(sp);
        for (auto& fc : sp->frames[0]) if (fc.kind == FrameCmd::PLACE && fc.place >= 0 && sp->places[fc.place].hasId) {
            const PlaceCmd& pc = sp->places[fc.place];
            CharDef* d = g_movie.get(pc.id);
            if (!d || d->type != CH_SPRITE) continue;
            DisplayObject* o = synthPut(c, pc.id, dp++, 0, 0);
            if (!o) continue;
            o->m = pc.m; o->scriptXform = true;
            if (pc.hasCx) { o->cx = pc.cx; o->scriptColor = true; }
        }
    }
    if (L.ok) synthPut(c, L.header, dp++, 0, 0);
    // the name
    if (g_kit.ok) {
        double x0 = 0;
        uint16_t nm = letteringShape(kUName[u], 1.4, cx, L.nameCy, 230, &x0);
        CharDef* nd = g_movie.get(nm);
        double w = nd ? ((ShapeDef*)nd)->bounds.x1 - ((ShapeDef*)nd)->bounds.x0 : 100;
        synthPut(c, nm, dp++, 0, 0);
        synthPut(c, quoteShape(x0 - 11, L.nameCy - 10, 1.3, ink), dp++, 0, 0);
        synthPut(c, quoteShape(x0 + w + 3, L.nameCy - 10, 1.3, ink), dp++, 0, 0);
        synthPut(c, letteringShape(kUSub[u], 0.95, cx, L.subCy, 230), dp++, 0, 0);
    } else {
        float w = 0; uint16_t t = synthHandText(std::string("\"") + kUName[u] + "\"", 16, ink, &w);
        synthPut(c, t, dp++, cx - w / 2, L.nameCy - 8);
        t = synthHandText(kUSub[u], 11, ink, &w);
        synthPut(c, t, dp++, cx - w / 2, L.subCy - 6);
    }
    // the picture
    double ph = L.picH;
    if (u == U_GONE || u == U_CARD) {
        if (u == U_CARD) synthPut(c, cardShapeId(ph * 0.66, ph * 0.94, 0xffe6e2dcu, ink, (float)(ph * 0.05)), dp++, cx, L.picCy);
        uint16_t pic = u == U_CARD && g_headShape ? g_headShape : g_portraitShape;
        CharDef* pd = pic ? g_movie.get(pic) : nullptr;
        if (pd && pd->type == CH_SHAPE) {
            RectF pr = ((ShapeDef*)pd)->bounds;
            double s = std::min(ph * (u == U_CARD ? 0.55 : 1.0) / std::max(1.0, pr.y1 - pr.y0), u == U_CARD ? ph * 0.5 / std::max(1.0, pr.x1 - pr.x0) : 1e9);
            DisplayObject* f = synthPut(c, pic, dp++, 0, 0);
            if (f) {
                f->m = Mat(); f->m.a = f->m.d = s; f->m.tx = cx - (pr.x0 + pr.x1) / 2 * s; f->m.ty = L.picCy - (pr.y0 + pr.y1) / 2 * s; f->scriptXform = true;
                if (u == U_GONE) { f->cx.am = 0.55f; f->scriptColor = true; }                 // (pale)
                else { f->cx.rm = f->cx.gm = f->cx.bm = -1; f->cx.ra = f->cx.ga = f->cx.ba = 255; f->scriptColor = true; }   // (in negative)
            }
        }
    } else if (u == U_SHARD) {
        synthPut(c, shardShape(ph, true, ink), dp++, cx, L.picCy);
    } else if (u == U_MIRROR && (g_sketchSprite || g_itemSprite)) {
        uint16_t spr = g_sketchSprite ? g_sketchSprite : g_itemSprite;
        RectF r; double s = 1, ox = 0, oy = 0;
        if (frameBounds(spr, 5, r)) { s = ph * 0.9 / std::max(1.0, r.y1 - r.y0); ox = cx - (r.x0 + r.x1) / 2 * s; oy = L.picCy - (r.y0 + r.y1) / 2 * s; }
        DisplayObject* m = synthPut(c, spr, dp++, 0, 0);
        if (m) {
            if (m->kind == OK_CLIP) gotoFrame((MovieClip*)m, 5, true);
            m->m = Mat(); m->m.a = m->m.d = s; m->m.tx = ox; m->m.ty = oy; m->scriptXform = true;
            // (in the card's grey: dark where the mirror is light, a negative)
            m->cx.rm = m->cx.gm = m->cx.bm = 0.42f; m->cx.ra = m->cx.ga = m->cx.ba = 18; m->scriptColor = true;   // (a dark mirror)
        }
    }   // (Nothing: nothing at all)
    hold(c);
    return c;
}

// ---- every tick: the clips showing one of the new things get its picture
struct Hijack { int f = 0; DisplayObject* art = nullptr; };
std::map<MovieClip*, Hijack> g_hijack;
bool onStage(DisplayObject* d) { for (; d; d = d->parent) { if (d->removed) return false; if (d == (DisplayObject*)vm.root) return true; } return false; }
void pictureTick() {
    if (!g_overFrames) { g_overFrames = gcNew<Array>(); g_overFrames->proto = protos::array; gcAddRoot((GcObj**)&g_overFrames); }
    auto& el = ((Array*)g_overFrames)->el;
    std::map<MovieClip*, Hijack> keep;
    for (size_t i = 0; i < el.size();) {
        MovieClip* c = el[i].isObj() && el[i].o->kind == OK_CLIP ? (MovieClip*)el[i].o : nullptr;
        if (!c || !onStage(c) || c->reqFrame <= (int)c->def->nframes) { el.erase(el.begin() + i); continue; }
        i++;
        int f = c->reqFrame, k = itemIndex(f);
        bool known = k >= 0 || f == kCardCol || (f >= kSecret0 && f < kSecret0 + U_N);
        if (!known) continue;
        auto it = g_hijack.find(c);
        Hijack h;
        if (it != g_hijack.end() && it->second.f == f) h = it->second;
        else {
            if (it != g_hijack.end() && it->second.art) unhold(it->second.art);
            h.f = f;
            if (k >= 0) {
                // (a pedestal's or the held item's picture is the coloured one; anything else, the grey drawings)
                bool floorOrHeld = false;
                for (DisplayObject* q = c->parent; q && !floorOrHeld; q = q->parent) if (q->kind == OK_CLIP) { double sv = num(getMemK(Value((Object*)q), intern("s"))); floorOrHeld = sv == 5 || sv == 1; }
                if (floorOrHeld) learnItemSprite(c);
                bool sketch = c->charId == g_sketchSprite || (!floorOrHeld && c->charId != g_itemSprite);
                h.art = itemArt(k, c->charId, sketch);
            }
            else if (f == kCardCol) {   // (on the floor: a pickup's own picture, s 5; held up or in the HUD: its face)
                bool floor = false;
                for (DisplayObject* q = c->parent; q && !floor; q = q->parent) if (q->kind == OK_CLIP && num(getMemK(Value((Object*)q), intern("s"))) == 5) floor = true;
                h.art = cardArt(c->charId, !floor);
            }
            else h.art = secretArt(f - kSecret0, c->charId);
        }
        keep[c] = h;
        SnapOv& o = g_snapOv[c]; o.extra = h.art; o.hideKids = true;
    }
    // the HUD's active item while the Black Mirror is held (its clip goes by the game's short numbers)
    Value d = V(V(V(root(), "hud"), "it"), "d");
    if (d.isObj() && d.o->kind == OK_CLIP && num(V(root(), "it")) == kMirrorShort && ((MovieClip*)d.o)->reqFrame == kMirrorShort && g_itemSprite) {
        MovieClip* c = (MovieClip*)d.o;
        auto it = g_hijack.find(c);
        Hijack h;
        if (it != g_hijack.end() && it->second.f == -kMirrorShort) h = it->second;
        else {
            if (it != g_hijack.end() && it->second.art) unhold(it->second.art);
            h.f = -kMirrorShort;
            MovieClip* a = newArt();
            // (sized like the HUD's own pictures: its active item 1, The Bible... compared with the items' item 5)
            RectF hr, ir; double s = 1;
            if (frameBounds(c->charId, 1, hr) && frameBounds(g_itemSprite, 5, ir)) s = std::min(1.3, (hr.y1 - hr.y0) / std::max(1.0, ir.y1 - ir.y0));
            DisplayObject* m = synthPut(a, g_itemSprite, 1, 0, 0);
            if (m) {
                if (m->kind == OK_CLIP) gotoFrame((MovieClip*)m, 5, true);
                double hx = (hr.x0 + hr.x1) / 2, hy = (hr.y0 + hr.y1) / 2, ix = (ir.x0 + ir.x1) / 2, iy = (ir.y0 + ir.y1) / 2;
                m->m = Mat(); m->m.a = m->m.d = s; m->m.tx = hx - ix * s; m->m.ty = hy - iy * s; m->scriptXform = true;
                m->cx.rm = m->cx.gm = m->cx.bm = -1; m->cx.ra = m->cx.ga = m->cx.ba = 255; m->scriptColor = true;
            }
            hold(a); h.art = a;
        }
        keep[c] = h;
        SnapOv& o = g_snapOv[c]; o.hideKids = true; o.extra = h.art;
    }
    for (auto& kv : g_hijack) if (!keep.count(kv.first) && kv.second.art) unhold(kv.second.art);
    g_hijack.swap(keep);
}

// ============================================================ the game's functions, wrapped (each room is a new _root.a)
struct Wrap { const char* name; Function* wrap = nullptr; Function* orig = nullptr; Object* a = nullptr; };
Value callOrig(Wrap& w, Act& ac, Value thisv, int argc, Value* argv) { return w.orig ? callValue(ac, Value((Object*)w.orig), thisv, argc, argv) : VUNDEF; }
void hookFn(Wrap& w, const Value& a, NativeFn fn) {
    if (!w.wrap) {
        Function* f = gcNew<Function>(); f->proto = protos::function; f->nat = fn; f->dbgName = w.name; gcPin(f);
        w.wrap = f; gcAddRoot((GcObj**)&w.orig); gcAddRoot((GcObj**)&w.a);
    }
    Value cur = V(a, w.name);
    if (cur.isObj() && cur.o != (Object*)w.wrap && cur.o->kind == OK_FUNCTION) { w.orig = (Function*)cur.o; w.a = a.o; S(a, w.name, Value((Object*)w.wrap)); }
}
Value callIn(Act& ac, const Value& obj, const char* fn, std::initializer_list<Value> args) {
    std::vector<Value> av(args);
    return callMethodK(ac, obj, intern(fn), (int)av.size(), av.data());
}

// ---- the run (any character): floors cleared without a hit or a heart, for The Gone himself
struct Track { uint32_t serial = 0xffffffffu; double chaps = 0; bool clean = true; int streak = 0; bool ended = false;
               int baseCols = -1; double baseIt = 0; bool extraItem = false; bool burst = false; };
Track g_t;
void floorDone(const char* why) {
    if (g_t.clean) g_t.streak++; else g_t.streak = 0;
    g_t.clean = true;
    if (getenv("ISAAC_GONELOG")) logf("thegone: floor done (%s): %d in a row\n", why, g_t.streak);
    if (g_t.streak >= 3) unlock(U_GONE);
}

Wrap g_wAi{"aistuff"};
int g_mirrorT = 0; Object* g_mirrorA = nullptr;
// Black Mirror: while it lasts, the enemies' own code (aistuff) runs with `player` set to one of them - the game's
// own decoy mechanism (rply / decer) - so they chase and shoot it instead of Isaac; the target moves round every second
Value wAi(Act& ac, Value thisv, int argc, Value* argv) {
    Value a((Object*)g_wAi.a);
    if (g_mirrorT <= 0 || g_mirrorA != a.o || truthy(V(a, "rply"))) return callOrig(g_wAi, ac, thisv, argc, argv);
    Value balls = V(a, "ball"); std::vector<Value> en;
    if (balls.isObj() && balls.o->kind == OK_ARRAY)
        for (const Value& b : ((Array*)balls.o)->el) if (b.isObj() && num(V(b, "s")) >= 10 && !truthy(V(b, "dones")) && num(V(b, "hp")) > 0) en.push_back(b);
    if (en.size() < 2) return callOrig(g_wAi, ac, thisv, argc, argv);
    Value player = V(a, "player"), target = en[(g_player.tick / 30) % en.size()];
    S(a, "player", target);
    Value r = callOrig(g_wAi, ac, thisv, argc, argv);
    S(a, "player", player);
    return r;
}
Wrap g_wHurt{"playerhurt"}, g_wPower{"powerlevel"}, g_wTater{"tater"}, g_wTart{"tart"};
bool ownsItem(int k) { Value ups = V(root(), "ups"); return num(arrGet(ups, kItemId[k])) > 0; }
Value wHurt(Act& ac, Value thisv, int argc, Value* argv) {
    // The Void Shard: an enemy shot (source 4) misses one time in four
    if (ownsItem(0) && argc > 1 && num(argv[1]) == 4 && rnd(4) == 0) { if (getenv("ISAAC_GONELOG")) logf("thegone: Void Shard: a shot missed\n"); return VUNDEF; }
    Value r = callOrig(g_wHurt, ac, thisv, argc, argv);
    if (truthy(r)) {
        g_t.clean = false;
        if (ownsItem(0) && rnd(20) == 0) g_t.burst = true;   // (done after the tick's scripts: 50 damage to the room)
    }
    return r;
}
Value wPower(Act& ac, Value thisv, int argc, Value* argv) {
    Value a((Object*)g_wPower.a), highs = V(a, "highs");
    int k = itemIndex(num(V(highs, "it"), -1));
    bool wasEmpty = truthy(V(highs, "empty"));
    Value colss = V(root(), "colss"), before = k >= 0 ? arrGet(colss, kItemId[k]) : VUNDEF;
    Value r = callOrig(g_wPower, ac, thisv, argc, argv);
    if (k < 0 || wasEmpty || !truthy(V(highs, "empty"))) return r;
    // picked up: the collection is the mod's (the game's save keeps its own items only)
    if (colss.isObj()) setMem(colss, Value((double)kItemId[k]), before);
    if (!g_u.coll[k]) { g_u.coll[k] = true; unlSave(); }
    if (k != 1) return r;
    // Black Mirror: an active item. The game took it as a passive one (collection list, its count): it is made the
    // held active item as the game does for its own (the one held before goes back on the pedestal)
    Value R = root(), cols = V(R, "cols");
    if (cols.isObj() && cols.o->kind == OK_ARRAY && !((Array*)cols.o)->el.empty() && num(((Array*)cols.o)->el.back()) == kItemId[1]) ((Array*)cols.o)->el.pop_back();
    Value ups = V(R, "ups"); if (ups.isObj()) setMem(ups, Value((double)kItemId[1]), Value(1.0));
    S(a, "itb", Value(15.0));
    Value hudIt = V(V(R, "hud"), "it"); if (hudIt.isObj() && hudIt.o->kind == OK_CLIP) gotoFrame((MovieClip*)hudIt.o, 8, true);
    S(R, "colit", Value((double)kItemId[1]));
    Value iter = V(R, "iter");
    double old = num(V(R, "it"));
    if (iter.isObj()) setMem(iter, Value(old), V(R, "itc"));
    S(R, "it", Value((double)kMirrorShort));
    if (old > 0 && highs.isObj() && highs.o != V(a, "player").o) {
        double back = old + num(callIn(ac, a, "itn2", {Value(old)}), 40);
        Value hd = V(highs, "d");
        if (hd.isObj() && hd.o->kind == OK_CLIP) gotoFrame((MovieClip*)hd.o, 10, true);
        Value dd = V(V(highs, "d"), "d"), ddd = V(dd, "d");
        if (ddd.isObj() && ddd.o->kind == OK_CLIP) gotoFrame((MovieClip*)ddd.o, 1, false);
        S(highs, "it", Value(back)); S(highs, "empty", VFALSE); S(highs, "done", VFALSE); S(highs, "dones", VFALSE);
        Value dddd = V(ddd, "d"); if (dddd.isObj() && dddd.o->kind == OK_CLIP) gotoFrame((MovieClip*)dddd.o, (int)back, true);
    }
    if (iter.isObj() && !std::isfinite(toNum(arrGet(iter, kMirrorShort)))) setMem(iter, Value((double)kMirrorShort), Value(1.0));
    S(R, "itc", arrGet(iter, kMirrorShort));
    return r;
}
// The Gone: items whose only effect is more health never come up: chosen again from the same pool (the game's own
// giveit, called again) before they appear
bool goneRunNow();
const int kHealthOnly[] = {15, 16, 22, 23, 24, 25, 26, 45, 92};
Wrap g_wGive{"giveit"};
Value wGive(Act& ac, Value thisv, int argc, Value* argv) {
    Value r = callOrig(g_wGive, ac, thisv, argc, argv);
    if (!goneRunNow()) return r;
    for (int tries = 0; tries < 20; tries++) {
        bool bad = false; for (int h : kHealthOnly) bad |= num(r) == h;
        if (!bad) break;
        r = callOrig(g_wGive, ac, thisv, argc, argv);
    }
    return r;
}
// cards: Negative Isaac comes up as often as any one other card
Value wTater(Act& ac, Value thisv, int argc, Value* argv) {
    Value r = callOrig(g_wTater, ac, thisv, argc, argv);
    if (!unlocked(U_CARD)) return r;
    // one more of the playing cards (70-74, red backs: when the game picks one of them, 1 in 6 it is Negative Isaac);
    // the original game has none: one more card among its 22
    double v = num(r);
    if (game() == "og" ? rnd(23) == 0 : (v >= 70 && v <= 74 && rnd(6) == 0)) return Value((double)kCardCol);
    return r;
}
Value wTart(Act& ac, Value thisv, int argc, Value* argv) {
    if (argc > 0 && num(argv[0]) == kCardCol) {
        Value name(mkstr("Negative Isaac"));
        if (argc > 1 && truthy(argv[1])) return name;
        callIn(ac, Value((Object*)g_wTart.a), "st11", {name});
        return VUNDEF;
    }
    return callOrig(g_wTart, ac, thisv, argc, argv);
}

// ---- the run's effects
double g_prevPilc = 0, g_prevIt = 0, g_prevItc = 0;
Object* g_nothingA = nullptr; double g_nothingLev = -1; int g_nothingWait = 0;
std::vector<Value> enemies(const Value& a) {
    std::vector<Value> out;
    Value balls = V(a, "ball");
    if (!balls.isObj() || balls.o->kind != OK_ARRAY) return out;
    for (const Value& b : ((Array*)balls.o)->el) {
        if (!b.isObj() || num(V(b, "s")) < 10 || truthy(V(b, "dones")) || truthy(V(b, "done")) || truthy(V(b, "friend")) || num(V(b, "hp")) <= 0) continue;
        out.push_back(b);
    }
    return out;
}
void effectsTick(const Value& a, bool play) {
    Value R = root();
    // names and the active item's charges (3), on every new room's copies
    Value st1 = V(R, "st1"), st2 = V(R, "st2x");
    for (int k = 0; k < 3; k++) {
        if (st1.isObj() && arrGet(st1, kItemId[k]).t != T_STR) setMem(st1, Value((double)kItemId[k]), Value(mkstr(kItemName[k])));
        if (st2.isObj() && arrGet(st2, kItemId[k]).t != T_STR) setMem(st2, Value((double)kItemId[k]), Value(mkstr(kItemSub[k])));
    }
    if (!g_headShape) {   // (the player's head drawing, for Negative Isaac's face)
        Value hd = V(V(V(V(V(a, "player"), "d"), "d"), "d"), "d");
        if (hd.isObj() && hd.o->kind == OK_CLIP) for (DisplayObject* k : ((MovieClip*)hd.o)->kids) if (k->kind == OK_SHAPE) { g_headShape = k->charId; break; }
    }
    Value itc = V(a, "itc");
    if (itc.isObj() && num(arrGet(itc, kMirrorShort)) != 3) setMem(itc, Value((double)kMirrorShort), Value(3.0));
    hookFn(g_wHurt, a, wHurt); hookFn(g_wAi, a, wAi); hookFn(g_wGive, a, wGive); hookFn(g_wPower, a, wPower); hookFn(g_wTater, a, wTater); hookFn(g_wTart, a, wTart);
    // the pools: each unlocked item, until seen this run
    static bool seen[3]; static uint32_t seenSerial = 0xffffffffu;
    if (seenSerial != modsRunSerial()) { seenSerial = modsRunSerial(); for (bool& s : seen) s = false; }
    Value balls = V(a, "ball");
    if (balls.isObj() && balls.o->kind == OK_ARRAY)
        for (const Value& b : ((Array*)balls.o)->el) {
            if (!b.isObj() || num(V(b, "s")) != 5) continue;
            int k = itemIndex(num(V(b, "it"), -1)); if (k >= 0) seen[k] = true;
            if (!g_itemSprite) { Value i4 = V(V(V(V(b, "d"), "d"), "d"), "d"); if (i4.isObj() && i4.o->kind == OK_CLIP) learnItemSprite((MovieClip*)i4.o); }
            if (false && num(V(b, "col")) == kCardCol) {   // Negative Isaac on the floor (the original game: a red tarot back)
                Value d = V(b, "d");
                if (d.isObj() && d.o->isDisplay()) { SnapOv& o = g_snapOv[(DisplayObject*)d.o]; o.hasCx = true; o.cx = CX(); o.cx.rm = 1.5f; o.cx.gm = 0.32f; o.cx.bm = 0.32f; o.cx.ra = 20; }
            }
        }
    for (int k = 0; k < 3; k++) {
        if (ownsItem(k)) seen[k] = true;
        if (seen[k] || !unlocked(kItemUnl[k])) continue;
        Value pool = V(R, k == 0 ? "ittt7" : "ittt");
        if (!pool.isObj() || pool.o->kind != OK_ARRAY) continue;
        bool in = false; for (const Value& v : ((Array*)pool.o)->el) in |= num(v) == kItemId[k];
        if (!in) ((Array*)pool.o)->el.push_back(Value((double)kItemId[k]));
    }
    if (!play) return;
    // Negative Isaac used: a shield, as Book of Shadows (15 s; it goes with the room, the game's own rule)
    double pilc = toNum(V(R, "pilc"));
    if (g_prevPilc == kCardCol && !(pilc > 0)) {
        S(a, "playsave", Value(450.0));
        call(a, "st22", {Value(mkstr("Untouchable"))});
    }
    g_prevPilc = std::isfinite(pilc) ? pilc : 0;
    // Black Mirror used (its charge spent)
    double it = num(V(R, "it")), itcNow = num(V(R, "itc"));
    if (g_prevIt == kMirrorShort && it == kMirrorShort && g_prevItc >= 1 && itcNow < 1) {
        g_mirrorT = 210; g_mirrorA = a.o;
        call(R, "soundy", {Value(mkstr("Devil_Card.mp")), Value(100.0)});
    }
    g_prevIt = it; g_prevItc = itcNow;
    if (g_mirrorT > 0 && g_mirrorA != a.o) g_mirrorT = 0;   // (left the room)
    if (g_mirrorT > 0) {
        g_mirrorT--;
        // the room in negative (fading back over the last second)
        double k = std::min(1.0, g_mirrorT / 30.0);
        SnapOv& o = g_snapOv[(DisplayObject*)a.o];
        o.hasCx = true; o.cx = CX(); float m = (float)(1 - 2 * k); o.cx.rm = o.cx.gm = o.cx.bm = m; o.cx.ra = o.cx.ga = o.cx.ba = (float)(255 * k);
        // the enemies go for each other
        std::vector<Value> en = enemies(a);
        for (size_t i = 0; i < en.size(); i++) {
            double x = num(V(en[i], "xp")), y = num(V(en[i], "yp")), bd = 1e18; int bj = -1;
            for (size_t j = 0; j < en.size(); j++) if (j != i) { double dx = num(V(en[j], "xp")) - x, dy = num(V(en[j], "yp")) - y, d2 = dx * dx + dy * dy; if (d2 < bd) { bd = d2; bj = (int)j; } }
            if (bj < 0) continue;
            double dx = num(V(en[bj], "xp")) - x, dy = num(V(en[bj], "yp")) - y, d = std::sqrt(bd);
            // (the ones that walk: their whole movement turned towards the nearest other one, at least at a walk -
            // they no longer go for the player; the ones standing still stay put, and shots still aim at him)
            double vx = num(V(en[i], "xbew")), vy = num(V(en[i], "ybew")), sp = std::sqrt(vx * vx + vy * vy);
            if (d > 1 && sp > 0.15) { sp = std::max(sp, 1.6); S(en[i], "xbew", Value(dx / d * sp)); S(en[i], "ybew", Value(dy / d * sp)); }
            // (touching each other hurts, as touching Isaac does: every other enemy it touches, twice a second)
            if ((g_player.tick + i) % 15 == 0)
                for (size_t j = 0; j < en.size(); j++) if (j != i) {
                    double ex = num(V(en[j], "xp")) - x, ey = num(V(en[j], "yp")) - y;
                    if (ex * ex + ey * ey < 36 * 36) call(a, "hurt", {en[j], Value(10.0)});
                }
        }
        // their shots (s 9) hurt the enemies they hit (not the one that just fired: a few ticks old)
        static std::map<Object*, uint32_t> born;
        Value balls = V(a, "ball");
        if (balls.isObj() && balls.o->kind == OK_ARRAY) {
            std::map<Object*, uint32_t> now;
            for (const Value& b : ((Array*)balls.o)->el) {
                if (!b.isObj() || num(V(b, "s")) != 9 || truthy(V(b, "done"))) continue;
                auto it = born.find(b.o); uint32_t t0 = it == born.end() ? g_player.tick : it->second; now[b.o] = t0;
                if (g_player.tick - t0 < 5) continue;
                double x = num(V(b, "xp")), y = num(V(b, "yp"));
                for (const Value& e : en) {
                    double dx = num(V(e, "xp")) - x, dy = num(V(e, "yp")) - y;
                    if (dx * dx + dy * dy < 18 * 18) { call(a, "hurt", {e, Value(3.5)}); S(b, "done", VTRUE); break; }
                }
            }
            born.swap(now);
        }
    } else g_mirrorA = nullptr;
    // The Void Shard: 50 damage to every enemy in the room (after a hit)
    if (g_t.burst) {
        g_t.burst = false;
        for (const Value& e : enemies(a)) call(a, "hurt", {e, Value(50.0)});
        if (getenv("ISAAC_GONELOG")) logf("thegone: Void Shard: 50 damage to the room\n");
        call(R, "soundy", {Value(mkstr("Death_Card.mp")), Value(100.0)});
    }
    // Nothing: entering a room, each enemy but the bosses may just not be there (no reward: it never was)
    double lev = num(V(R, "lev"), -1);
    if (a.o != g_nothingA || lev != g_nothingLev) { g_nothingA = a.o; g_nothingLev = lev; g_nothingWait = 3; }
    if (const char* t = getenv("ISAAC_GONENOTHINGAT")) if (g_player.tick == (uint32_t)atoi(t)) g_nothingWait = 1;   // (testing)
    if (g_nothingWait > 0 && --g_nothingWait == 0 && ownsItem(2))
        for (const Value& e : enemies(a)) {
            if (truthy(V(e, "bosser")) || truthy(V(e, "aboss")) || truthy(V(e, "boss")) || truthy(V(e, "minb")) || V(e, "worm").t != T_UNDEF || V(e, "mags").t != T_UNDEF) continue;
            if (rnd(10) == 0) { S(e, "done", VTRUE); if (getenv("ISAAC_GONELOG")) logf("thegone: Nothing: enemy %g gone\n", num(V(e, "s"))); }
        }
}

// ---- the run's progress towards the unlocks
void trackTick(const Value& a, bool inGame, bool play, int rootFrame, bool goneRun) {
    Value R = root();
    if (g_t.serial != modsRunSerial()) { g_t = Track(); g_t.serial = modsRunSerial(); }
    bool ending = rootFrame >= 9 && rootFrame <= 40;
    if (ending && !g_t.ended) {
        g_t.ended = true;
        floorDone("ending");
        if (goneRun) {
            int chaps = (int)num(V(R, "chaps")), e = chaps >= 11 ? 5 : chaps >= 9 ? (truthy(V(R, "altchap")) ? 3 : 4) : chaps >= 7 ? 2 : 1;
            if (e == 2) { unlock(U_CARD); if (!g_t.extraItem) unlock(U_NOTHING); }
            if (e == 3) unlock(U_SHARD);
            if (e == 4) unlock(U_MIRROR);
        }
    }
    if (!inGame || !a.isObj()) return;
    if (!ending) g_t.ended = false;
    double chaps = num(V(R, "chaps"));
    if (g_t.chaps == 0) g_t.chaps = chaps;
    else if (chaps > g_t.chaps) { g_t.chaps = chaps; floorDone("next floor"); }
    // items besides the starting one (The Gone's runs: Nothing needs none)
    Value cols = V(R, "cols");
    int n = cols.isObj() && cols.o->kind == OK_ARRAY ? (int)((Array*)cols.o)->el.size() : 0;
    double it = num(V(R, "it"));
    if (play) {
        if (g_t.baseCols < 0) { g_t.baseCols = n; g_t.baseIt = it; }
        else if (n > g_t.baseCols || it != g_t.baseIt) g_t.extraItem = true;
    }
}
void heartPicked() { g_t.clean = false; }

// ---- the found secrets (menus): the mod's unlock cards among the game's while the menus show; never saved
void secretsInMenu(bool menu) {
    Value lockor = V(root(), "lockor");
    if (!lockor.isObj() || lockor.o->kind != OK_ARRAY) return;
    auto& el = ((Array*)lockor.o)->el;
    for (size_t i = 0; i < el.size();) { double v = num(el[i]); if (v >= kSecret0 && v < kSecret0 + U_N) el.erase(el.begin() + i); else i++; }
    if (!menu) return;
    for (int u = U_N; u-- > 0;) if (unlocked(u)) el.insert(el.begin(), Value((double)(kSecret0 + u)));
}
std::vector<Value> g_stash;
void saveHook(const Value& data, bool before) {
    Value lockor = V(data, "lockor");
    if (!lockor.isObj() || lockor.o->kind != OK_ARRAY) return;
    auto& el = ((Array*)lockor.o)->el;
    if (before) {
        g_stash.clear();
        for (size_t i = 0; i < el.size();) { double v = num(el[i]); if (v >= kSecret0 && v < kSecret0 + U_N) { g_stash.push_back(el[i]); el.erase(el.begin() + i); } else i++; }
    } else { el.insert(el.begin(), g_stash.begin(), g_stash.end()); g_stash.clear(); }
}

// ---- the unlock cards, in a run (or an ending): the game's own animation, once it is free
void popupTick() {
    static bool test = false;   // (testing: ISAAC_GONEPOPUPS shows every card once a run is on)
    if (!test && getenv("ISAAC_GONEPOPUPS") && V(root(), "a").isObj()) { test = true; for (int u = 0; u < U_N; u++) g_popups.push_back(u); }
    if (g_popups.empty()) return;
    Value unl = V(root(), "unl");
    if (!unl.isObj() || unl.o->kind != OK_CLIP || ((MovieClip*)unl.o)->cur != 1) return;
    int u = g_popups.front(); g_popups.erase(g_popups.begin());
    S(root(), "unlll", Value((double)(kSecret0 + u)));
    S(unl, "nogo", VTRUE);
    call(unl, "gotoAndPlay", {Value(2.0)});
    call(root(), "soundy", {Value(mkstr("Choir_unlock.wav")), Value(100.0)});
}

// ---- the collection ("Stuff I found", the menus): the three items after the game's last one, on its last page, as
// a "?" until picked up once; pointing at one shows it large with its name, as the game's own slots do
MovieClip* g_collArt = nullptr; Object* g_collCol = nullptr; std::string g_collKey; std::vector<RectF> g_collRects;
void collectionTick() {
    Value sheet = V(V(root(), "men"), "men"), colV = V(sheet, "col");
    if (!colV.isObj() || colV.o->kind != OK_CLIP) return;
    MovieClip* col = (MovieClip*)colV.o;
    if (col->cur != col->def->nframes) return;   // (the last page only)
    std::map<int, DisplayObject*> slot;
    for (DisplayObject* k : col->kids) if (k->name && k->name->s.size() > 1 && k->name->s[0] == 'i' && isdigit((unsigned char)k->name->s[1])) slot[atoi(k->name->s.c_str() + 1)] = k;
    if (slot.size() < 20) return;
    int n = slot.rbegin()->first;
    if (!slot.count(n - 10)) return;
    DisplayObject* last = slot[n];
    double ddx = last->m.tx - slot[n - 10]->m.tx, ddy = last->m.ty - slot[n - 10]->m.ty;
    double sx[3], sy[3];
    for (int k = 0; k < 3; k++) {
        int j = n + 1 + k;
        DisplayObject* ref = slot.count(j - 10) ? slot[j - 10] : nullptr;
        if (!ref) return;
        sx[k] = ref->m.tx + ddx; sy[k] = ref->m.ty + ddy;
    }
    unlLoad();
    std::string key = std::to_string(last->charId) + (g_u.coll[0] ? "1" : "0") + (g_u.coll[1] ? "1" : "0") + (g_u.coll[2] ? "1" : "0");
    if (!g_collArt || g_collCol != (Object*)col || key != g_collKey) {
        if (!g_collArt) gcAddRoot((GcObj**)&g_collArt), gcAddRoot((GcObj**)&g_collCol);
        g_collArt = newArt(); g_collCol = col; g_collKey = key;
        for (int k = 0; k < 3; k++) {
            MovieClip* s = createEmptyMovieClip(g_collArt, intern(("s" + std::to_string(k)).c_str()), k + 1);
            if (!s) continue;
            s->m = last->m; s->m.tx = sx[k]; s->m.ty = sy[k]; s->scriptXform = true;
            if (g_u.coll[k]) { if (DisplayObject* a = itemArt(k, last->charId, true)) { a->depth = 1 + DEPTH_BIAS; a->parent = s; s->insertChild(a); } }
            else if (DisplayObject* q = synthPut(s, last->charId, 1, 0, 0)) { if (q->kind == OK_CLIP) gotoFrame((MovieClip*)q, 201, true); }
        }
    }
    g_snapOv[col].extra = g_collArt;
    {   // (the controller can go to them: their places on the stage, beside the game's own slots)
        RectF a = stageBounds(slot[n - 10]), b = stageBounds(last);
        std::vector<std::pair<DisplayObject*, RectF>> ex; g_collRects.clear();
        for (int k = 0; k < 3; k++) {
            RectF r = stageBounds(slot[n + 1 + k - 10]);
            if (!r.valid() || !a.valid() || !b.valid()) continue;
            double ox = b.x0 - a.x0, oy = b.y0 - a.y0;
            r.x0 += ox; r.x1 += ox; r.y0 += oy; r.y1 += oy;
            MovieClip* sc = (MovieClip*)g_collArt->childByName(intern(("s" + std::to_string(k)).c_str()));
            if (sc) ex.push_back({sc, r});
            g_collRects.push_back(r);
        }
        if (getenv("ISAAC_GONELOG")) { static size_t last = 99; if (ex.size() != last) { last = ex.size(); logf("thegone: %zu collection slots for the controller\n", ex.size()); for (auto& e : ex) logf("thegone:   %.0f,%.0f-%.0f,%.0f\n", e.second.x0, e.second.y0, e.second.x1, e.second.y1); RectF lb = stageBounds(slot[n]); logf("thegone:   last %.0f,%.0f-%.0f,%.0f\n", lb.x0, lb.y0, lb.x1, lb.y1); } }
        menuSetExtras(ex);
    }
    // pointing at one of them (the mouse, or the controller's selection, which moves it): the large picture and the
    // name, after the game set its own
    for (size_t k = 0; k < g_collRects.size() && k < 3; k++) {
        const RectF& r = g_collRects[k];
        if (g_player.mouseX < r.x0 || g_player.mouseX > r.x1 || g_player.mouseY < r.y0 || g_player.mouseY > r.y1) continue;
        Value itt = V(sheet, "itt");
        if (itt.isObj() && itt.o->kind == OK_CLIP) {
            MovieClip* ic = (MovieClip*)itt.o;
            gotoFrame(ic, 199, true);   // (empty; the picture is the mod's: frame 230+ is something else in that sprite)
            static DisplayObject* big[3] = {}; static uint16_t bigFor = 0;
            if (bigFor != ic->charId) { bigFor = ic->charId; for (auto& b : big) b = nullptr; }
            if (g_u.coll[k] && !big[k] && k != 2) big[k] = itemArt(k, ic->charId, true);
            if (g_u.coll[k]) { SnapOv& o = g_snapOv[ic]; o.hideKids = true; o.extra = big[k]; }
        }
        S(sheet, "ittz", Value(mkstr(g_u.coll[k] ? kItemName[k] : "?")));
        updateBoundTexts();   // (its text field shows it now: the game's own update ran before this)
        // (selected: red, as the game's own slots)
        if (MovieClip* sc = (MovieClip*)g_collArt->childByName(intern(("s" + std::to_string(k)).c_str()))) {
            SnapOv& o = g_snapOv[sc]; o.hasCx = true; o.cx = CX(); o.cx.rm = 0.6f; o.cx.gm = o.cx.bm = 0.15f; o.cx.ra = 150;
        }
    }
}
