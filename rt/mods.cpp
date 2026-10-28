// Mods: loading the .mod files and running the enabled ones (see mods.h)
#include "mods.h"
#include "snapshot.h"
#include "core.h"
#include "platform.h"
#include <algorithm>
#include <chrono>
#include <map>
#include <cctype>
#include <cstdio>

namespace fl {
static std::vector<ModInfo> g_mods;

static std::string trim(std::string s) {
    while (!s.empty() && isspace((unsigned char)s.back())) s.pop_back();
    size_t i = 0; while (i < s.size() && isspace((unsigned char)s[i])) i++;
    return s.substr(i);
}
static std::string lower(std::string s) { for (auto& c : s) c = (char)tolower((unsigned char)c); return s; }
static bool validId(const std::string& s) {
    if (s.empty() || s.size() > 40) return false;
    for (char c : s) if (!isalnum((unsigned char)c) && c != '_' && c != '-') return false;
    return true;
}
static const char* kEngines[] = {"staton", "genesis", "widesaac", "pilllog", "itemnomicon", "basetime", "thegone", "rainbow"};   // the built-in engines (mods.h)

static bool parseMod(const std::string& path, const std::string& file, ModInfo& m) {
    FILE* f = platformFopen(path, "rb"); if (!f) return false;
    std::string text; char buf[4096]; size_t n;
    while ((n = fread(buf, 1, sizeof buf, f)) > 0 && text.size() < (64u << 10)) text.append(buf, n);
    fclose(f);
    if (text.size() >= 3 && (unsigned char)text[0] == 0xEF && (unsigned char)text[1] == 0xBB && (unsigned char)text[2] == 0xBF) text.erase(0, 3);   // (UTF-8 BOM)
    m.file = file;
    size_t p = 0; std::string note;
    while (p < text.size()) {
        size_t e = text.find('\n', p); if (e == std::string::npos) e = text.size();
        std::string line = trim(text.substr(p, e - p)); p = e + 1;
        if (line.empty()) { note.clear(); continue; }
        if (line[0] == ';' || line[0] == '#') { note = trim(line.substr(1)); continue; }
        size_t eq = line.find('='); if (eq == std::string::npos) continue;
        std::string k = lower(trim(line.substr(0, eq))), v = trim(line.substr(eq + 1));
        if (k == "id") m.id = lower(v); else if (k == "name") m.name = v; else if (k == "version") m.version = v;
        else if (k == "author") m.author = v; else if (k == "description") m.description = v; else if (k == "engine") m.engine = lower(v);
        else if (k == "warning" || k == "help") {}   // (for the mod manager)
        else { if (!m.options.count(k)) m.optKeys.push_back(k); m.options[k] = v; m.optNotes[k] = note; }
        note.clear();
    }
    if (!validId(m.id)) return false;
    if (m.engine.empty()) m.engine = m.id;
    if (m.name.empty()) m.name = m.id;
    for (const char* e : kEngines) if (m.engine == e) m.known = true;
    return true;
}

// the mods that come with the program (mods/*.mod in the source tree, built in by CMakeLists.txt)
// ---- updating a built-in mod's file: when this program's copy of a mod is a newer version than the file in the
// folder, the file is rewritten, keeping the player's value of each option whose explanation (the comment just
// above it) didn't change; an option that changed meaning gets its new default.
struct OptLine { std::string value, note; };
static std::map<std::string, OptLine> modOptions(const std::string& text, std::string* version) {
    std::map<std::string, OptLine> out; std::string note;
    size_t p = 0;
    while (p <= text.size()) {
        size_t e = text.find('\n', p); if (e == std::string::npos) e = text.size();
        std::string line = trim(text.substr(p, e - p)); p = e + 1;
        if (line.empty()) { note.clear(); if (e >= text.size()) break; continue; }
        if (line[0] == ';' || line[0] == '#') { note = line; continue; }
        size_t eq = line.find('=');
        if (eq != std::string::npos) {
            std::string k = lower(trim(line.substr(0, eq))), v = trim(line.substr(eq + 1));
            if (k == "version" && version) *version = v;
            out[k] = {v, note};
        }
        note.clear();
        if (e >= text.size()) break;
    }
    return out;
}
static bool versionLess(const std::string& a, const std::string& b) {
    size_t i = 0, j = 0;
    while (i < a.size() || j < b.size()) {
        long x = 0, y = 0;
        while (i < a.size() && isdigit((unsigned char)a[i])) x = x * 10 + (a[i++] - '0');
        while (j < b.size() && isdigit((unsigned char)b[j])) y = y * 10 + (b[j++] - '0');
        if (x != y) return x < y;
        while (i < a.size() && !isdigit((unsigned char)a[i])) i++;
        while (j < b.size() && !isdigit((unsigned char)b[j])) j++;
    }
    return false;
}
static void upgradeBuiltin(const std::string& path, const char* builtin) {
    std::string old;
    if (FILE* f = platformFopen(path, "rb")) { char buf[4096]; size_t n; while ((n = fread(buf, 1, sizeof buf, f)) > 0) old.append(buf, n); fclose(f); }
    if (old.empty()) return;
    std::string vOld, vNew;
    auto mine = modOptions(old, &vOld); auto theirs = modOptions(builtin, &vNew);
    if (vNew.empty() || !versionLess(vOld, vNew)) return;
    if (lower(mine["id"].value) != lower(theirs["id"].value)) return;   // (another mod under the same file name)
    std::string out, note, text = builtin;
    size_t p = 0;
    while (p < text.size()) {
        size_t e = text.find('\n', p); if (e == std::string::npos) e = text.size();
        std::string raw = text.substr(p, e - p), line = trim(raw); p = e + 1;
        if (!line.empty() && line[0] != ';' && line[0] != '#') {
            size_t eq = line.find('=');
            if (eq != std::string::npos) {
                std::string k = lower(trim(line.substr(0, eq)));
                static const char* kMeta[] = {"id", "name", "version", "author", "description", "engine", "warning", "help"};
                bool meta = false; for (auto* m : kMeta) meta |= k == m;
                auto it = mine.find(k);
                if (!meta && it != mine.end() && it->second.note == note) raw = trim(line.substr(0, eq)) + " = " + it->second.value;
            }
            note.clear();
        } else note = line.empty() ? "" : line;
        out += raw + "\n";
    }
    if (FILE* f = platformFopen(path, "wb")) { fputs(out.c_str(), f); fclose(f); logf("mods: %s updated to version %s (your options kept)\n", path.c_str(), vNew.c_str()); }
}
static const struct { const char* file; const char* text; } kBuiltin[] = {
#include "mods_builtin.inc"
    {nullptr, nullptr}};

void modsInit(const std::string& dir, const std::string& enabledList) {
    g_mods.clear();
    // the mods that come with the program are put in the "mods" folder (created if needed) the first time this
    // version sees them: on Android it's the only way to get them, and a later version's new mods appear by
    // themselves. mods/installed.txt lists the ones already put there, so a mod deleted from the folder stays deleted.
    std::vector<std::string> names;
    if (!platformListDir(dir, names)) { if (platformMkdir(dir.substr(0, dir.size() - 1))) logf("mods: created %s\n", dir.c_str()); }
    {
        std::vector<std::string> done;
        if (FILE* f = platformFopen(dir + "installed.txt", "rb")) {
            char buf[256];
            while (fgets(buf, sizeof buf, f)) { std::string l = trim(buf); if (!l.empty() && l[0] != ';') done.push_back(lower(l)); }
            fclose(f);
        }
        bool changed = false;
        for (auto* b = kBuiltin; b->file; b++) {
            bool there = false; std::string realName; for (auto& n : names) if (lower(n) == lower(b->file)) { there = true; realName = n; }
            if (there) upgradeBuiltin(dir + realName, b->text);   // (a newer version of it comes with the program)
            if (std::find(done.begin(), done.end(), lower(b->file)) != done.end()) continue;
            if (!there) {
                if (FILE* f = platformFopen(dir + b->file, "wb")) { fputs(b->text, f); fclose(f); names.push_back(b->file); logf("mods: added %s\n", b->file); }
                else continue;
            }
            done.push_back(lower(b->file)); changed = true;
        }
        if (changed) if (FILE* f = platformFopen(dir + "installed.txt", "wb")) {
            fputs("; The mods that came with IsaacRecomp and were already put in this folder (one that is deleted is not put back)\n", f);
            for (auto& d : done) fprintf(f, "%s\n", d.c_str());
            fclose(f);
        }
    }
    // mods/mods.txt: which mods are on and their options, one [id] section each (written by the program, edited by
    // hand or made by tools/mod_manager.html). Without it (versions before 1.5's last ones): mods/enabled.txt, or
    // isaac.ini's "mods =" line, and each .mod file's own options; mods.txt is then written from them.
    struct TxtSec { bool hasOn = false, on = false; std::map<std::string, std::string> opts; };
    std::map<std::string, TxtSec> txt; bool haveTxt = false;
    const bool forced = getenv("ISAAC_MODS") != nullptr;   // (tests: exactly these mods, with their .mod files' options)
    if (FILE* f = forced ? nullptr : platformFopen(dir + "mods.txt", "rb")) {
        haveTxt = true; char buf[2048]; std::string sec;
        while (fgets(buf, sizeof buf, f)) {
            std::string l = trim(buf);
            if (l.size() >= 3 && (unsigned char)l[0] == 0xEF && (unsigned char)l[1] == 0xBB && (unsigned char)l[2] == 0xBF) l = trim(l.substr(3));
            if (l.empty() || l[0] == ';' || l[0] == '#') continue;
            if (l[0] == '[') { size_t e = l.find(']'); sec = lower(trim(l.substr(1, e == std::string::npos ? std::string::npos : e - 1))); txt[sec]; continue; }
            size_t eq = l.find('='); if (eq == std::string::npos || sec.empty()) continue;
            std::string k = lower(trim(l.substr(0, eq))), v = trim(l.substr(eq + 1));
            if (k == "on") { txt[sec].hasOn = true; txt[sec].on = v == "1" || lower(v) == "yes" || lower(v) == "true" || lower(v) == "on"; }
            else txt[sec].opts[k] = v;
        }
        fclose(f);
        logf("mods: the mods that are on and their options come from mods/mods.txt\n");
    }
    std::string list = enabledList;
    if (haveTxt) { list.clear(); for (auto& t : txt) if (t.second.on) list += t.first + ","; }
    else if (FILE* f = forced ? nullptr : platformFopen(dir + "enabled.txt", "rb")) {
        list.clear(); char buf[512];
        while (fgets(buf, sizeof buf, f)) { std::string l = trim(buf); if (!l.empty() && l[0] != ';' && l[0] != '#') list += l + ","; }
        fclose(f);
        logf("mods: the mods that are on come from mods/enabled.txt\n");
    }
    std::vector<std::string> want;
    {
        std::string cur;
        for (char c : list + ",") {
            if (c == ',' || c == ';' || c == ' ') { cur = lower(trim(cur)); if (!cur.empty()) want.push_back(cur); cur.clear(); }
            else cur += c;
        }
    }
    std::vector<std::string> files;
    for (auto& n : names) if (n.size() > 4 && lower(n.substr(n.size() - 4)) == ".mod") files.push_back(n);
    std::sort(files.begin(), files.end());
    for (auto& n : files) {
        ModInfo m;
        if (!parseMod(dir + n, n, m)) { logf("mods: %s is not a valid mod file (it needs an \"id = ...\" line)\n", n.c_str()); continue; }
        bool dup = false; for (auto& o : g_mods) dup |= o.id == m.id;
        if (dup) { logf("mods: %s repeats the id \"%s\": ignored\n", n.c_str(), m.id.c_str()); continue; }
        {   // mods.txt's option values
            auto it = txt.find(m.id);
            if (it != txt.end()) for (auto& o : it->second.opts) { if (!m.options.count(o.first)) m.optKeys.push_back(o.first); m.options[o.first] = o.second; }
        }
        m.enabled = std::find(want.begin(), want.end(), m.id) != want.end();
        if (m.enabled && !m.known) { logf("mods: %s (%s) needs \"%s\", which this version of the program doesn't have: not loaded\n", m.name.c_str(), n.c_str(), m.engine.c_str()); m.enabled = false; }
        // (one mod per engine: the first one listed in the folder)
        if (m.enabled) for (auto& o : g_mods) if (o.enabled && o.engine == m.engine) { logf("mods: %s and %s both use \"%s\": only %s is on\n", o.name.c_str(), m.name.c_str(), m.engine.c_str(), o.name.c_str()); m.enabled = false; }
        logf("mods: %s %s%s%s (%s)\n", m.enabled ? "ON " : "off", m.name.c_str(), m.version.empty() ? "" : " ", m.version.c_str(), n.c_str());
        g_mods.push_back(m);
    }
    for (auto& m : g_mods) if (m.enabled && m.engine == "genesis") genesisInit(m);
    // (re)write mods.txt when it doesn't exist yet or a mod isn't in it (a new one: listed, off), keeping what it said
    bool write = !haveTxt && !forced;
    for (auto& m : g_mods) if (!forced && !txt.count(m.id)) write = true;
    if (write) {
        std::string out =
            "; IsaacRecomp - mods.txt: which mods are on, and their options.\n"
            "; Change it with any text editor: \"on = 1\" turns a mod on, \"on = 0\" turns it off; the lines under it are\n"
            "; its options, each one explained just above it. tools/mod_manager.html can also make this file for you.\n"
            "; The game reads it when it starts. A new mod put in this folder is added here by itself (off).\n";
        for (auto& m : g_mods) {
            auto it = txt.find(m.id);
            bool on = it != txt.end() && it->second.hasOn ? it->second.on : m.enabled;
            const std::string& desc = m.description;
            out += "\n[" + m.id + "]\n; " + m.name + (m.version.empty() ? "" : " " + m.version) + (desc.empty() ? "" : " - " + desc) + "\n";
            out += std::string("on = ") + (on ? "1" : "0") + "\n";
            for (auto& k : m.optKeys) {
                auto nt = m.optNotes.find(k);
                if (nt != m.optNotes.end() && !nt->second.empty()) out += "; " + nt->second + "\n";
                out += k + " = " + m.opt(k) + "\n";
            }
        }
        // (sections of mods that aren't in the folder now are kept as they were)
        for (auto& t : txt) {
            bool there = false; for (auto& m : g_mods) there |= m.id == t.first;
            if (there) continue;
            out += "\n[" + t.first + "]\n; (no such mod in the mods folder now)\n";
            if (t.second.hasOn) out += std::string("on = ") + (t.second.on ? "1" : "0") + "\n";
            for (auto& o : t.second.opts) out += o.first + " = " + o.second + "\n";
        }
        if (FILE* f = platformFopen(dir + "mods.txt", "wb")) { fputs(out.c_str(), f); fclose(f); logf("mods: wrote mods/mods.txt\n"); }
    }
    for (auto& w : want) {
        bool there = false; for (auto& m : g_mods) there |= m.id == w;
        if (!there) logf("mods: \"%s\" is turned on but there is no such mod in %s\n", w.c_str(), dir.c_str());
    }
}
const std::vector<ModInfo>& modsAll() { return g_mods; }
const ModInfo* modActive(const char* engine) {
    for (auto& m : g_mods) if (m.enabled && m.engine == engine) return &m;
    return nullptr;
}
// ---- new runs: the game's newstart(true) (from the menu's run start, Replay after dying, R held) - wrapped on _root.a
// (a new clip with each room) while a mod needs to know (BaseTime, The Gone)
static uint32_t s_runSerial = 0;
static Function* s_nsWrap = nullptr; static Function* s_nsOrig = nullptr;
static Value wNewstart(Act& ac, Value thisv, int argc, Value* argv) {
    if (argc > 0 && truthy(argv[0])) s_runSerial++;
    return s_nsOrig ? callValue(ac, Value((Object*)s_nsOrig), thisv, argc, argv) : VUNDEF;
}
static void hookNewstart() {
    if (!s_nsWrap) {
        Function* f = gcNew<Function>(); f->proto = protos::function; f->nat = wNewstart; f->dbgName = "modsNewstart"; gcPin(f);
        s_nsWrap = f; gcAddRoot((GcObj**)&s_nsOrig);
    }
    Value a = getMemK(Value((Object*)vm.root), intern("a"));
    if (!a.isObj()) return;
    Str* k = intern("newstart");
    Value cur = getMemK(a, k);
    if (cur.isObj() && cur.o != (Object*)s_nsWrap && cur.o->kind == OK_FUNCTION) { s_nsOrig = (Function*)cur.o; setMemK(a, k, Value((Object*)s_nsWrap)); }
}
uint32_t modsRunSerial() { return s_runSerial; }

void modsTick() {
    { bool want = false; for (auto& m : g_mods) want |= m.enabled && (m.engine == "basetime" || m.engine == "thegone"); if (want) hookNewstart(); }
    g_snapOv.clear();   // (each mod adds its snapshot-only changes again every tick)
    static const bool prof = getenv("ISAAC_MODPROF") != nullptr;   // (testing: time spent in each mod, at exit)
    static std::map<std::string, std::pair<double, double>> times;   // engine -> (total, max) ms
    static uint32_t ticks = 0;
    if (prof && ++ticks % 3000 == 0) for (auto& t : times) logf("[modprof] %s: mean %.3f ms, max %.2f ms\n", t.first.c_str(), t.second.first / ticks, t.second.second);
    for (auto& m : g_mods) {
        if (!m.enabled) continue;
        auto t0 = prof ? std::chrono::steady_clock::now() : std::chrono::steady_clock::time_point();
        struct Done { bool on; std::chrono::steady_clock::time_point t0; const std::string& e; ~Done() {
            if (!on) return;
            double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
            auto& t = times[e]; t.first += ms; t.second = std::max(t.second, ms);
        } } done{prof, t0, m.engine};
        if (m.engine == "staton") statonTick(m);
        else if (m.engine == "genesis") genesisTick(m);
        else if (m.engine == "widesaac") wideTick(m);
        else if (m.engine == "pilllog") pilllogTick(m);
        else if (m.engine == "itemnomicon") itemnomiconTick(m);
        else if (m.engine == "basetime") basetimeTick(m);
        else if (m.engine == "thegone") thegoneTick(m);
        else if (m.engine == "rainbow") rainbowTick(m);
    }
}
}  // namespace fl
