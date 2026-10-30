// Audio mixer: MP3 decoded in streaming fashion (dr_mp3) from the SWF data.
#include "display.h"
#include "platform.h"
#include <SDL.h>
#include <functional>
#include <algorithm>
#include <mutex>
#include <unordered_map>
#include <atomic>
#include <array>
#include <string>
#include <cctype>
#define DR_MP3_IMPLEMENTATION
#define DR_MP3_NO_STDIO
#include "../third_party/drmp3.h"

namespace fl {
extern std::vector<std::function<void(Tracer&)>> g_rootTracers;
extern float g_masterVolume;
float g_audioVolume = 1.0f;   // global volume from the settings

static const int kRate = 44100;
#ifdef __PSP__
static constexpr bool kIsPsp = true;
#else
static constexpr bool kIsPsp = false;
#endif
static int g_rate = kRate;   // output rate: the device's own (Android, Switch: 48 kHz), 44.1 kHz in tests
struct EnvPt { uint32_t at; float l, r; };   // SoundInfo envelope point, position in output frames
struct Chan {
    // Threads: the logic thread starts / stops channels and owns owner, def, stream, gen, srcRate, startFrame, skip,
    // music; the mixer (audio callback) owns the decoder and the buffers while it mixes. `m` is held only around one
    // channel's work (mixing it, or starting / stopping / seeking it), so the game never waits for the whole mix; the
    // state the game reads every tick (active, decOk, published position) is atomic and read without waiting.
    std::mutex m;
    std::atomic<bool> active{false};
    std::atomic<bool> decOk{false};
    drmp3 dec;
    SoundDef* def = nullptr; StreamInfo* stream = nullptr;
    Object* owner = nullptr;
    bool music = false;             // never taken over for another sound (loops, streams, long sounds: music)
    int loops = 1; uint64_t startFrame = 0; uint64_t skip = 0;
    std::atomic<float> vol{1}, pan{0};
    int srcRate = kRate, srcCh = 1;
    double pos = 0;                 // fractional position in the source buffer
    bool eof = false;               // no more source data: play out what is left in buf
    float buf[2][2048 + 4]; int bufLen = 0; int bufBase = 0;   // decoded samples (per channel: 2048 per refill + up to 3 kept for interpolation)
    uint64_t framesConsumed = 0;    // source frames consumed since startFrame (for position)
    std::atomic<double> posPub{0};  // (published by the mixer) source frames played since startFrame
    std::atomic<double> curPub{0};  // (published by the mixer) absolute source frame being played (streams)
    std::vector<EnvPt> env; size_t envNext = 0; EnvPt envPrev{0, 1, 1}, envNextPt{0, 1, 1}; uint32_t envCur = 0;
    std::shared_ptr<const std::vector<uint8_t>> bytes;   // lean loading: keeps the MP3 data alive while playing
    int gen = 0;
    // (PSP) decoded-sound cache: plays from `pcm` instead of the MP3 decoder; `rec` records the first playback
    std::shared_ptr<struct PcmClip> pcm; uint64_t pcmPos = 0;
    std::shared_ptr<struct PcmClip> rec;
};
// A short sound decoded once (int16, source channels), from frame `base` (its SeekSamples) to the end. On the PSP the
// MP3 decoder took about a fifth of the CPU, most of it for the same few effects over and over (tears, hits, steps).
struct PcmClip { std::vector<int16_t> s; int ch = 1, rate = 44100; uint64_t base = 0, frames = 0; uint32_t lastUse = 0; };
#ifdef ISAAC_PCMCACHE_TEST
static constexpr size_t kPcmBudget = 1536u << 10, kPcmMaxClip = 192u << 10;   // (testing the cache on PC)
#else
static constexpr size_t kPcmBudget = kIsPsp ? (1536u << 10) : 0, kPcmMaxClip = 192u << 10;
#endif
static std::mutex g_pcmMu;
static std::unordered_map<const SoundDef*, std::shared_ptr<PcmClip>> g_pcmCache;
static size_t g_pcmBytes = 0; static uint32_t g_pcmClock = 0;
static std::shared_ptr<PcmClip> pcmFind(const SoundDef* s) {
    std::lock_guard<std::mutex> lk(g_pcmMu);
    auto it = g_pcmCache.find(s); if (it == g_pcmCache.end()) return nullptr;
    it->second->lastUse = ++g_pcmClock; return it->second;
}
static void pcmStore(const SoundDef* s, std::shared_ptr<PcmClip> c) {
    std::lock_guard<std::mutex> lk(g_pcmMu);
    if (g_pcmCache.count(s)) return;
    c->s.shrink_to_fit(); c->lastUse = ++g_pcmClock;
    g_pcmBytes += c->s.size() * 2; g_pcmCache[s] = c;
    while (g_pcmBytes > kPcmBudget && !g_pcmCache.empty()) {   // least recently used out (a playing one stays alive)
        auto old = g_pcmCache.begin();
        for (auto it = g_pcmCache.begin(); it != g_pcmCache.end(); ++it) if (it->second->lastUse < old->second->lastUse) old = it;
        g_pcmBytes -= old->second->s.size() * 2; g_pcmCache.erase(old);
    }
}
#ifdef __PSP__
static const int kChans = 24;   // (memory: about 33 KB each)
#else
static const int kChans = 48;
#endif
static Chan g_chans[kChans];
static void recAbort(Chan& c) { c.rec.reset(); }
static SDL_AudioDeviceID g_dev = 0;
static int g_genCounter = 0;

static std::recursive_mutex g_devMu;   // g_dev can be reopened by the main thread (output device changes)
void audioLock() { g_devMu.lock(); }
void audioUnlock() { g_devMu.unlock(); }

// (channel locked)
static bool chanSeek(Chan& c, uint64_t frame) {
    if (!c.decOk) return false;
    recAbort(c);
    bool ok = true;
    if (c.pcm) { c.pcmPos = frame < c.pcm->base ? c.pcm->base : std::min(frame, c.pcm->base + c.pcm->frames); ok = frame >= c.pcm->base; }
    else ok = drmp3_seek_to_pcm_frame(&c.dec, frame) != 0;
    c.bufLen = 0; c.pos = 0; c.eof = false; c.framesConsumed = frame >= c.startFrame ? frame - c.startFrame : 0;
    c.posPub = (double)c.framesConsumed; c.curPub = (double)frame;
    return ok;
}
static void chanFree(Chan& c) { if (c.decOk && !c.pcm) drmp3_uninit(&c.dec); c.pcm.reset(); c.rec.reset(); c.decOk = false; c.active = false; c.owner = nullptr; c.def = nullptr; c.stream = nullptr; c.env.clear(); c.bytes.reset(); }

// envelope (Ruffle's EnvelopeSignal): linear between points, counted in output frames from the start
static void envStart(Chan& c, const std::vector<std::array<uint32_t, 3>>* env) {
    c.env.clear(); c.envNext = 0; c.envCur = 0;
    if (!env || env->empty()) return;
    double sc = (double)g_rate / 44100.0;
    for (auto& p : *env) c.env.push_back({(uint32_t)(p[0] * sc), p[1] / 32768.f, p[2] / 32768.f});
    c.envNextPt = c.env[0]; c.envNext = 1;
    c.envPrev = {0, c.envNextPt.l, c.envNextPt.r};
}
static inline void envStep(Chan& c, float& l, float& r) {
    if (c.envPrev.at < c.envNextPt.at) {
        float t = (float)(c.envCur - c.envPrev.at) / (float)(c.envNextPt.at - c.envPrev.at);
        l = c.envPrev.l + (c.envNextPt.l - c.envPrev.l) * t; r = c.envPrev.r + (c.envNextPt.r - c.envPrev.r) * t;
    } else { l = c.envNextPt.l; r = c.envNextPt.r; }
    if (c.envCur < 0xFFFFFFFFu) c.envCur++;
    while (c.envCur > c.envNextPt.at) {
        c.envPrev = c.envNextPt;
        if (c.envNext < c.env.size()) c.envNextPt = c.env[c.envNext++];
        else c.envNextPt = {0xFFFFFFFFu, c.envPrev.l, c.envPrev.r};
        if (c.envPrev.at > c.envNextPt.at) c.envNextPt.at = c.envPrev.at;
    }
}

// refills the source buffer; returns false at the end of the sound (no more loops)
// source reads: the decoded-sound cache or the MP3 decoder
static uint64_t srcRead(Chan& c, int want, float* tmp) {
    if (c.pcm) {
        uint64_t end = c.pcm->base + c.pcm->frames, n = c.pcmPos < end ? std::min<uint64_t>(want, end - c.pcmPos) : 0;
        const int16_t* p = c.pcm->s.data() + (size_t)(c.pcmPos - c.pcm->base) * c.pcm->ch;
        for (uint64_t i = 0; i < n * c.pcm->ch; i++) tmp[i] = p[i] * (1.f / 32768.f);
        c.pcmPos += n;
        return n;
    }
    uint64_t got = drmp3_read_pcm_frames_f32(&c.dec, want, tmp);
    if (c.rec) {
        if (got) {
            size_t o = c.rec->s.size(); c.rec->s.resize(o + got * c.srcCh);
            for (uint64_t i = 0; i < got * c.srcCh; i++) { float v = tmp[i] * 32767.f; c.rec->s[o + i] = (int16_t)(v > 32767.f ? 32767.f : v < -32768.f ? -32768.f : v); }
            c.rec->frames += got;
            if (c.rec->s.size() * 2 > kPcmMaxClip) c.rec.reset();   // (too long: not kept)
        } else { if (c.rec->frames) pcmStore(c.def, c.rec); c.rec.reset(); }   // the whole sound decoded: kept for the next time
    }
    return got;
}
static void srcSeek(Chan& c, uint64_t frame) {
    if (c.pcm) { c.pcmPos = std::max(frame, c.pcm->base); return; }
    drmp3_seek_to_pcm_frame(&c.dec, frame);
}
static bool refill(Chan& c) {
    float tmp[2048 * 2];
    int want = 2048;
    uint64_t got = srcRead(c, want, tmp);
    if (got == 0) {
        if (c.loops > 1) {
            c.loops--;
            srcSeek(c, c.skip);
            c.framesConsumed = 0;
            got = srcRead(c, want, tmp);
            if (got == 0) return false;
        } else return false;
    }
    for (uint64_t i = 0; i < got; i++) {
        float l = tmp[i * c.srcCh], r = c.srcCh > 1 ? tmp[i * c.srcCh + 1] : l;
        c.buf[0][i] = l; c.buf[1][i] = r;
    }
    c.bufLen = (int)got;
    return true;
}

static void mix(void*, Uint8* stream, int len) {
#ifdef __PSP__
    // (PSP: 16-bit output: mixed in floats here, converted at the end instead of by SDL)
    static float s_mix[4096 * 2];
    int frames = std::min(4096, len / (int)(sizeof(int16_t) * 2)); float* out = s_mix;
#else
    float* out = (float*)stream; int frames = len / (int)(sizeof(float) * 2);
#endif
    std::fill(out, out + frames * 2, 0.f);
    float master = g_masterVolume * g_audioVolume;
    for (auto& c : g_chans) {
        if (!c.active.load(std::memory_order_acquire)) continue;
        std::lock_guard<std::mutex> lk(c.m);
        if (!c.active) continue;
        double step = (double)c.srcRate / g_rate;
        float vol = c.vol.load(std::memory_order_relaxed), pan = c.pan.load(std::memory_order_relaxed);
        float lv = vol * master * (pan > 0 ? 1 - pan : 1), rv = vol * master * (pan < 0 ? 1 + pan : 1);
        bool hasEnv = !c.env.empty();
        bool ended = false;
#ifdef __PSP__
        // (PSP: doubles are emulated in software: the position in 16.16 fixed point within the buffer)
        int64_t pf = (int64_t)(c.pos * 65536.0); const int64_t sf = (int64_t)(step * 65536.0 + 0.5);
#define POS_INT ((int)(pf >> 16))
#define POS_FRAC ((float)(pf & 0xffff) * (1.f / 65536.f))
#define POS_GE(v) (pf >= ((int64_t)(v) << 16))
#define POS_SUB(v) (pf -= (int64_t)(v) << 16)
#define POS_ADV() (pf += sf)
#else
#define POS_INT ((int)c.pos)
#define POS_FRAC ((float)(c.pos - (int)c.pos))
#define POS_GE(v) (c.pos >= (v))
#define POS_SUB(v) (c.pos -= (v))
#define POS_ADV() (c.pos += step)
#endif
        for (int i = 0; i < frames; i++) {
            // keep 3 samples of history across refills: the cubic interpolation reads one before and two after
            while (!c.eof && POS_GE(c.bufLen - 2)) {
                int k = std::min(3, c.bufLen);
                float hl[3], hr[3];
                for (int q = 0; q < k; q++) { hl[q] = c.buf[0][c.bufLen - k + q]; hr[q] = c.buf[1][c.bufLen - k + q]; }
                int oldLen = c.bufLen;
                c.framesConsumed += oldLen - k;   // (before refill: a loop restart resets it)
                if (!refill(c)) { c.framesConsumed -= oldLen - k; c.eof = true; break; }   // (buf untouched: the tail is still played)
                memmove(&c.buf[0][k], &c.buf[0][0], sizeof(float) * c.bufLen); memmove(&c.buf[1][k], &c.buf[1][0], sizeof(float) * c.bufLen);
                for (int q = 0; q < k; q++) { c.buf[0][q] = hl[q]; c.buf[1][q] = hr[q]; }
                c.bufLen += k;
                POS_SUB(oldLen - k);
            }
            if (c.eof && POS_GE(c.bufLen - 1)) { ended = true; break; }
            int ip = POS_INT; float t = POS_FRAC;
            int im = ip > 0 ? ip - 1 : 0, i2 = std::min(ip + 2, c.bufLen - 1);
            // Catmull-Rom (cubic Hermite): noticeably cleaner than linear when 11/22 kHz sounds are upsampled
            auto cr = [t](float pm, float p0, float p1, float p2) {
                return p0 + 0.5f * t * (p1 - pm + t * (2 * pm - 5 * p0 + 4 * p1 - p2 + t * (3 * (p0 - p1) + p2 - pm)));
            };
            float l = cr(c.buf[0][im], c.buf[0][ip], c.buf[0][ip + 1], c.buf[0][i2]);
            float r = cr(c.buf[1][im], c.buf[1][ip], c.buf[1][ip + 1], c.buf[1][i2]);
            if (hasEnv) { float el, er; envStep(c, el, er); l *= el; r *= er; }
            out[i * 2] += l * lv; out[i * 2 + 1] += r * rv;
            POS_ADV();
        }
#ifdef __PSP__
        c.pos = pf / 65536.0;
#endif
#undef POS_INT
#undef POS_FRAC
#undef POS_GE
#undef POS_SUB
#undef POS_ADV
        c.posPub.store((double)c.framesConsumed + c.pos, std::memory_order_relaxed);
        if (c.decOk) c.curPub.store((double)(c.pcm ? c.pcmPos : c.dec.currentPCMFrame) - c.bufLen + c.pos, std::memory_order_relaxed);
        if (ended) { if (c.decOk && !c.pcm) drmp3_uninit(&c.dec); c.pcm.reset(); c.rec.reset(); c.decOk = false; c.active.store(false, std::memory_order_release); }
    }
#ifdef __PSP__
    int16_t* o16 = (int16_t*)stream;
    for (int i = 0; i < frames * 2; i++) { float v = out[i] * 32767.f; o16[i] = (int16_t)(v > 32767.f ? 32767.f : v < -32768.f ? -32768.f : v); }
#else
    for (int i = 0; i < frames * 2; i++) out[i] = std::max(-1.f, std::min(1.f, out[i]));
#endif
}

// ---- output device
// SDL normally follows the system's default output. On PC, plugging a controller with its own audio (DualSense,
// DualShock 4, Xbox headset jack) can make it the default, and the sound "disappears" into the controller. So the
// device that was the default when the game started is opened by name and kept; when it goes away, or the default
// changes to something that isn't a controller (headphones plugged in...), the game moves to the new default.
static std::string g_devName;   // "" = SDL's default device
static double g_devCheckAt[2] = {-1, -1};
static bool isControllerAudio(const std::string& n) {
    std::string l; for (char ch : n) l += (char)tolower((unsigned char)ch);
    for (const char* k : {"controller", "dualsense", "dualshock", "xbox", "gamepad", "joy-con"}) if (l.find(k) != std::string::npos) return true;
    return false;
}
static std::string defaultOutputName() {
#if SDL_VERSION_ATLEAST(2, 24, 0)
    char* name = nullptr; SDL_AudioSpec spec{};
    if (SDL_GetDefaultAudioInfo(&name, &spec, 0) == 0 && name) { std::string r = name; SDL_free(name); return r; }
#endif
    return std::string();
}
// the system's default is a controller (connected before the game started): another output instead. Speakers or
// headphones first, then anything that isn't a monitor (HDMI / DisplayPort audio), a digital output or virtual
static std::string bestNonController() {
    std::string best; int bestScore = -1000;
    int n = SDL_GetNumAudioDevices(0);
    for (int i = 0; i < n; i++) {
        const char* nm = SDL_GetAudioDeviceName(i, 0);
        if (!nm || isControllerAudio(nm)) continue;
        std::string l; for (const char* p = nm; *p; p++) l += (char)tolower((unsigned char)*p);
        int sc = 0;
        for (const char* k : {"speaker", "altavo", "haut-parleur", "lautsprecher", "alto-falante", "headphone", "auricular",
                              "casque", "kopfh", "realtek", "headset"}) if (l.find(k) != std::string::npos) sc += 10;
        for (const char* k : {"hdmi", "displayport", "display audio", "nvidia", "amd high definition", "digital", "spdif",
                              "s/pdif", "virtual", "steam streaming", "cable", "vb-audio", "monitor"}) if (l.find(k) != std::string::npos) sc -= 10;
        if (sc > bestScore) { bestScore = sc; best = nm; }
    }
    return best;
}
static bool openDevice(const std::string& name) {
    SDL_AudioSpec want{}, have{};
    // mix directly at the device's rate (one resampling step instead of ours + SDL's conversion)
    want.freq = (kIsAndroid || kIsSwitch) ? 48000 : kRate; want.format = kIsPsp ? AUDIO_S16SYS : AUDIO_F32SYS; want.channels = 2; want.samples = kIsAndroid ? 512 : 1024; want.callback = mix;   // (Android: ~11 ms)
    SDL_AudioDeviceID d = SDL_OpenAudioDevice(name.empty() ? nullptr : name.c_str(), 0, &want, &have, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
    if (!d) { logf("audio: %s: %s\n", name.empty() ? "default device" : name.c_str(), SDL_GetError()); return false; }
    if (have.freq >= 8000 && have.freq <= 192000) g_rate = have.freq;
    g_dev = d; g_devName = name;
    logf("audio: %s, %d Hz, %d channels, %d-sample buffer%s\n", SDL_GetCurrentAudioDriver() ? SDL_GetCurrentAudioDriver() : "?", have.freq, have.channels, have.samples,
         name.empty() ? "" : " (fixed output device)");
    SDL_PauseAudioDevice(g_dev, 0);
    return true;
}
void audioInit(bool device) {
    if (!device) { g_rootTracers.push_back([](Tracer& tr) { for (auto& c : g_chans) if (c.active || c.decOk) tr.mark(c.owner); }); return; }
    std::string def = (kIsAndroid || kIsSwitch) ? std::string() : defaultOutputName();
    if (!def.empty() && isControllerAudio(def)) {
        std::string other = bestNonController();
        logf("audio: the default output is a controller (%s)%s%s\n", def.c_str(), other.empty() ? "" : ", using ", other.c_str());
        if (!other.empty()) def = other;
    }
    if (def.empty() || !openDevice(def)) openDevice(std::string());
    g_rootTracers.push_back([](Tracer& tr) { for (auto& c : g_chans) if (c.active || c.decOk) tr.mark(c.owner); });
}
// main thread: an audio device was added or removed (the system's default is checked a little later: it changes
// asynchronously)
void audioDevicesChanged(double now) {
    if (kIsAndroid || kIsSwitch || g_devName.empty() || !g_dev) return;
    g_devCheckAt[0] = now + 0.5; g_devCheckAt[1] = now + 2.5;
}
void audioDeviceTick(double now) {
    for (double& at : g_devCheckAt) {
        if (at < 0 || now < at) continue;
        at = -1;
        std::string def = defaultOutputName();
        bool lost = SDL_GetAudioDeviceStatus(g_dev) == SDL_AUDIO_STOPPED;
        bool moved = !def.empty() && def != g_devName && !isControllerAudio(def);
        if (!lost && !moved) continue;
        std::string want = moved ? def : std::string();
        if (lost && !moved) want = (!def.empty() && !isControllerAudio(def)) ? def : bestNonController();
        logf("audio: output device %s, switching to %s\n", lost ? "lost" : "changed", want.empty() ? "the default" : want.c_str());
        std::lock_guard<std::recursive_mutex> lk(g_devMu);
        SDL_CloseAudioDevice(g_dev); g_dev = 0;
        if (want.empty() || !openDevice(want)) openDevice(std::string());
    }
}

// a free channel, returned locked. All in use: the oldest one that isn't music is taken over; if every channel is
// playing music (or a loop), the new sound is not played (as Flash does past its channel limit).
static Chan* allocChan() {
    for (auto& c : g_chans) {
        if (c.active.load(std::memory_order_acquire)) continue;
        c.m.lock();
        if (!c.active) { if (c.decOk && !c.pcm) drmp3_uninit(&c.dec); c.pcm.reset(); c.rec.reset(); c.decOk = false; return &c; }
        c.m.unlock();
    }
    Chan* best = nullptr;
    for (auto& c : g_chans) if (!c.music && (!best || c.gen < best->gen)) best = &c;
    if (!best) return nullptr;
    best->m.lock(); chanFree(*best); return best;
}

int soundStart(SoundDef* s, double offsetSec, int loops, Object* owner, float vol, float pan, const std::vector<std::array<uint32_t, 3>>* env) {
    if (!s || s->format != 2 || !s->size) return -1;
    std::shared_ptr<const std::vector<uint8_t>> bytes;
    const uint8_t* data = s->data; size_t size = s->size;
    if (!data) { bytes = soundBytes(s); if (!bytes || bytes->empty()) return -1; data = bytes->data(); size = bytes->size(); }
    std::shared_ptr<PcmClip> clip = kPcmBudget ? pcmFind(s) : nullptr;
    uint64_t skip = s->seek > 0 ? (uint64_t)s->seek : 0;
    if (clip && clip->base > skip + (uint64_t)std::max(0.0, offsetSec * clip->rate)) clip.reset();
    Chan* c = allocChan();
    if (!c) return -1;
    c->bytes = clip ? nullptr : bytes;
    c->pcm = clip; c->rec.reset();
    if (clip) { c->decOk = true; c->srcRate = clip->rate; c->srcCh = clip->ch; c->pcmPos = clip->base; }
    else {
        c->decOk = drmp3_init_memory(&c->dec, data, size, nullptr) != 0;
        if (!c->decOk) { c->m.unlock(); return -1; }
        c->srcRate = (int)c->dec.sampleRate; c->srcCh = (int)c->dec.channels;
    }
    c->def = s; c->stream = nullptr; c->owner = owner; c->loops = std::max(1, loops); c->vol = vol; c->pan = pan;
    static const double kRates[4] = {5512.5, 11025, 22050, 44100};
    c->music = c->loops > 1 || s->samples / kRates[s->rate & 3] > 10.0;
    c->skip = skip;
    c->startFrame = c->skip + (uint64_t)std::max(0.0, offsetSec * c->srcRate);
    if (c->startFrame) chanSeek(*c, c->startFrame);
    // the first time a short sound plays from its start, its decoded samples are kept (srcRead)
    if (!clip && kPcmBudget && !c->music && c->startFrame == c->skip && (uint64_t)s->samples * c->srcCh * 2 <= kPcmMaxClip) {
        c->rec = std::make_shared<PcmClip>(); c->rec->ch = c->srcCh; c->rec->rate = c->srcRate; c->rec->base = c->startFrame;
        c->rec->s.reserve((size_t)s->samples * c->srcCh);
    }
    c->bufLen = 0; c->pos = 0; c->eof = false; c->framesConsumed = 0; c->posPub = 0; c->curPub = (double)c->startFrame;
    envStart(*c, env);
    c->gen = ++g_genCounter;
    c->active.store(true, std::memory_order_release);
    int idx = (int)(c - g_chans);
    c->m.unlock();
    return idx;
}

int streamStart(StreamInfo* st, int frame0, Object* owner) {
    if (!st) return -1;
    std::shared_ptr<const std::vector<uint8_t>> bytes;
    const uint8_t* data = st->mp3.data(); size_t size = st->mp3.size();
    if (!st->fileBlocks.empty()) { bytes = streamBytes(st); if (!bytes || bytes->empty()) return -1; data = bytes->data(); size = bytes->size(); }
    if (!size) return -1;
    Chan* c = allocChan();
    if (!c) return -1;
    c->bytes = bytes; c->pcm.reset(); c->rec.reset();
    c->decOk = drmp3_init_memory(&c->dec, data, size, nullptr) != 0;
    if (!c->decOk) { c->m.unlock(); return -1; }
    c->def = nullptr; c->stream = st; c->owner = owner; c->loops = 1; c->vol = 1; c->pan = 0; c->music = true;
    c->srcRate = (int)c->dec.sampleRate; c->srcCh = (int)c->dec.channels; c->skip = 0;
    c->startFrame = 0; c->env.clear();
    uint64_t target = frame0 < (int)st->frameSample.size() ? st->frameSample[frame0] : 0;
    if (target) chanSeek(*c, target); else { c->bufLen = 0; c->pos = 0; c->eof = false; c->framesConsumed = 0; c->posPub = 0; c->curPub = 0; }
    c->gen = ++g_genCounter;
    c->active.store(true, std::memory_order_release);
    int idx = (int)(c - g_chans);
    c->m.unlock();
    return idx;
}

void streamSync(int ch, StreamInfo* st, int frame0) {
    if (ch < 0 || ch >= kChans) return;
    Chan& c = g_chans[ch];
    if (!c.active || c.stream != st || frame0 >= (int)st->frameSample.size()) return;
    // actual playback position (published by the mixer: the decoder's frame minus what is still buffered). (A counter
    // used to be used instead, but it drifted: it resynced on almost every tick, and resyncing backwards decodes the
    // MP3 from the start: up to ~20 ms per tick on PC and much more on Switch.)
    double cur = c.curPub.load(std::memory_order_relaxed);
    double want = st->frameSample[frame0];
    if (std::fabs(cur - want) > c.srcRate * 0.2) {
        static bool dbg = getenv("ISAAC_STREAMDBG") != nullptr;
        if (dbg) fprintf(stderr, "[stream] seek frame0=%d cur=%.0f want=%.0f rate=%d\n", frame0, cur, want, c.srcRate);
        std::lock_guard<std::mutex> lk(c.m);
        if (c.active && c.decOk && c.stream == st) chanSeek(c, (uint64_t)want);
    }
}

bool chanAlive(int ch) { if (ch < 0 || ch >= kChans) return false; return g_chans[ch].active.load(std::memory_order_acquire); }
static void chanStop(Chan& c) { std::lock_guard<std::mutex> lk(c.m); chanFree(c); }
void soundStopChan(int ch) { if (ch < 0 || ch >= kChans) return; chanStop(g_chans[ch]); }
void soundStopOwner(Object* owner) { for (auto& c : g_chans) if (c.owner == owner && (c.active || c.decOk)) chanStop(c); }
void soundStopAll() { for (auto& c : g_chans) if (c.active || c.decOk) chanStop(c); }
void soundStopDef(SoundDef* s) { for (auto& c : g_chans) if (c.def == s && c.active) chanStop(c); }
bool soundPlayingDef(SoundDef* s) { for (auto& c : g_chans) if (c.active && c.def == s) return true; return false; }
void soundSetVolumeOwner(Object* owner, float vol, float pan) {
    for (auto& c : g_chans) if (c.active && c.owner == owner) { c.vol.store(vol, std::memory_order_relaxed); c.pan.store(pan, std::memory_order_relaxed); }
}
double soundPositionMs(Object* owner, bool* active) {
    double r = 0; int best = -1;
    for (auto& c : g_chans) if (c.owner == owner && c.decOk && c.gen > best) {
        best = c.gen;
        r = (c.posPub.load(std::memory_order_relaxed) + (double)(c.startFrame > c.skip ? c.startFrame - c.skip : 0)) * 1000.0 / c.srcRate;
    }
    if (active) *active = best >= 0;
    return r;
}

// ---- audio dump to WAV (device-less tests): mixes 1/30 s per tick
static FILE* g_wav = nullptr; static uint32_t g_wavFrames = 0;
void audioDumpTick(const char* path) {
    if (!path) {   // (test mode without --wav: still consume 1/30 s per tick, as in real time)
        static double acc = 0; acc += kRate / 30.0; int n = (int)acc; acc -= n;
        std::vector<float> buf(n * 2);
        static const bool mixMs = getenv("ISAAC_MIXMS") != nullptr;   // (testing) cost of mixing 1/30 s
        Uint64 t0 = SDL_GetPerformanceCounter();
        audioLock(); mix(nullptr, (Uint8*)buf.data(), n * 2 * sizeof(float)); audioUnlock();
        if (mixMs) { int act = 0; for (auto& c : g_chans) act += c.active; logf("[mix] %.3f %d\n", (SDL_GetPerformanceCounter() - t0) * 1000.0 / SDL_GetPerformanceFrequency(), act); }
        return;
    }
    if (!g_wav) { g_wav = fopen(path, "wb"); if (!g_wav) return; uint8_t h[44] = {0}; fwrite(h, 1, 44, g_wav); }
    static double accum = 0; accum += kRate / 30.0; int n = (int)accum; accum -= n;
    std::vector<float> buf(n * 2);
    audioLock(); mix(nullptr, (Uint8*)buf.data(), n * 2 * sizeof(float)); audioUnlock();
    std::vector<int16_t> s16(n * 2); for (int i = 0; i < n * 2; i++) s16[i] = (int16_t)(buf[i] * 32767);
    fwrite(s16.data(), 2, n * 2, g_wav); g_wavFrames += n;
}
void audioDumpClose() {
    if (!g_wav) return;
    uint32_t dataBytes = g_wavFrames * 4; uint8_t h[44];
    auto w32 = [&](int o, uint32_t v) { h[o] = v; h[o + 1] = v >> 8; h[o + 2] = v >> 16; h[o + 3] = v >> 24; };
    auto w16 = [&](int o, uint16_t v) { h[o] = v; h[o + 1] = v >> 8; };
    memcpy(h, "RIFF", 4); w32(4, 36 + dataBytes); memcpy(h + 8, "WAVEfmt ", 8); w32(16, 16); w16(20, 1); w16(22, 2); w32(24, kRate);
    w32(28, kRate * 4); w16(32, 4); w16(34, 16); memcpy(h + 36, "data", 4); w32(40, dataBytes);
    fseek(g_wav, 0, SEEK_SET); fwrite(h, 1, 44, g_wav); fclose(g_wav); g_wav = nullptr;
}
}  // namespace fl
