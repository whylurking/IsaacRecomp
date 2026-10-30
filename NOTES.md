# The Binding of Isaac (Flash) — native C++ recomp (design notes)

## Multiple versions (v9)
- Three games in one executable: original (`og`), Wrath of the Lamb (`wotl`) and Eternal Edition (`eternal`).
  `tools/gen.py ... --ns NAME` generates each one into `gen/NAME/` (PC, 1440x1080) and `gen720/NAME/` (Switch) inside
  `namespace fl::NAME`, with its own `GameCode g_code_NAME` (script/button tables, stage, atoms).
- swfload.cpp: `kGames` (decompressed SWF size + CRC-32); Movie::load identifies the game and sets `g_code`
  before walking the timeline. `peekSwfLength` reads only the beginning of an .exe/.swf (selector).
- Zinc: XOR key a + 0x21*i with a different `a` per executable (0xd5 og, 0x83 wotl, 0xa1 eternal): deduced from
  "CWS"/"FWS".
- The HD patch (patchhd.py) works as-is for all three. Eternal only takes the Zinc path of `fullf` if
  `mdm.System.macVerString` exists (otherwise "nozinc" mode with layers at 4x): builtins.cpp defines it.
- Switch: on exit ALWAYS call platformStopThreads() (hbloader loads hbmenu and the next .nro in the same process).
- Switch: pacing with fences (renderPaceAfterSwap/renderPaceWait, `low_latency`): at most one frame queued on the
  GPU; previously the driver allowed more to queue and the stall showed up inside "draw" (~16 ms), with more input lag.
- patches.cpp: per-game ids (`kIds`) for the options menu and the controls screen.
- main.cpp: finds every .exe/.swf in the folder, selector (`chooseGame` + `renderChooser`) if there are several,
  `game =` in isaac.ini (old `juego =` still accepted), `--game` (tests and restart after "Delete save"); save and
  preload list per game.
- Regenerate: FFDec `-export script`, patchhd.py on `frame_1/DoAction.as` (2.25 and 1.5), `-importScript`,
  `patchhd.py --bin`, `gen.py --ns`.

## Pipeline
1. `isaac.swf` (original) → `tools/patch1080.py` (+ FFDec importScript) → `isaac1080.swf`
   (1080p patch: _root at 225 %, 1440x1080 stage, high-resolution BitmapData).
2. `tools/gen.py isaac1080.swf gen/` translates ALL of the AVM1 bytecode (frame DoActions, functions,
   button actions) to C++ (`gen/*.cpp`). No interpreter: each block is a C++ function.
3. The executable loads the assets (shapes, sprites, sounds, fonts) from the user's ORIGINAL SWF at
   runtime (no assets are redistributed). Actions are linked by
   (sprite_id, frame, index_in_frame) and (button_id, condition_index).

## Semantics (ported from Ruffle: https://github.com/ruffle-rs/ruffle)
- SWF8: case-sensitive, undefined→NaN, _x… properties case-insensitive.
- Scopes: Local → Target(clip) → Global. `set` without var: local if it exists, otherwise on the Target clip.
- get on DisplayObject: own props → (_root/_parent/_global/_levelN) → children by name → _x… props → proto.
- for-in: proto keys first, then own keys in REVERSE insertion order, then children (by descending depth).
- AVM1 frame: execution list (newest clip first). For each clip: enqueue onEnterFrame
  (or onLoad the first time); if playing: advance the frame (place/remove immediate, DoAction enqueued).
  Then the action queue is drained (FIFO). instantiate_child immediately runs the child's 1st frame.
  goto: rewind with PlaceObject merging by depth (see run_goto in Ruffle).
- Script depths = AS depth + 16384.

## Threads and rendering
- Logic (own thread, `main.cpp`): 30 Hz ticks on a fixed clock (T_n = base + n/30). At the end of each tick
  `buildSnapshot()` (snapshot.cpp) copies the display list into a flat preorder tree (`SnapNode`:
  uid, local matrix, cx, content, masks, filters) + BitmapData pixel uploads (dirty rectangle)
  + freed bitmaps. Input (keyboard, mouse, controller) arrives through a queue from the main thread.
- Render (main thread): presentation clock that advances one refresh period per frame (with
  vsync + glFinish to measure it accurately), simulation time = presentation − margin (adaptive, p90 of the
  logic's publish delay + 2 ms). Adopts snapshots in order (applying their uploads) and
  draws lerp(previous, current, α). Matched by uid; no interpolation (snap) if the object is new,
  changes graphic/visibility, flips, changes scale/rotation a lot, or jumps >250 px without having been thrown.
  Matrices: scale and angle handled separately. If the logic is late, extrapolates up to α = 1.35.
- The renderer never touches live objects: it rasterizes from `RasterSrc` (def + ratio + copied text).
- Shapes → PlutoVG in parallel (thread pool per calling thread: raster.cpp) → atlas of 2048² pages
  created at startup (shelf packing; the least-used page is evicted). No mipmaps (scale quantized to
  quarter octaves). Uploads through a PBO ring (no waits). BitmapData textures per uid with size
  pooling; bitmaps that stop being visible return their texture after 2 ticks.
- Per-frame commands: leaves, bitmaps, masks (stencil) and blur groups (FBO, reusable best-fit
  intermediate textures).
- BitmapData.draw (logic thread): two phases — plan leaves (shape + final matrix, 1/64 subpixel),
  rasterize those missing from the leaf cache (in parallel, large ones in strips) and composite in order
  (premultiplied source-over). Whatever is under a mask is painted directly. Exact dirty rectangle.
- PlutoVG: per-thread persistent cell memory; plutovg_texture_smooth per thread.
- GC: marking from roots + pinned objects (gcPin), lazy sweep; also triggered by external memory
  (pixels). Pixel buffers are recycled.

## Status (first release)
- Recompiled: 679 frame scripts, 225 functions, 62 button actions (47 distinct AVM1 opcodes, all supported).
- Verified: intro, cutscenes with streaming audio, menus, character select, rooms, transitions,
  items, combat, damage, saving. 3×20,000 ticks with random input and no failures. Logic ≈1.5 ms/tick.
- Compared against Ruffle (same patched SWF): same composition on the title and character select screens.

## Rebuilding
    python3 tools/patch1080.py patch1080/scripts/frame_1/DoAction.as   # on top of the FFDec decomp
    java -jar ffdec-cli.jar -importScript isaac.swf s1.swf patch1080/scripts
    python3 tools/patch1080.py --bin s1.swf isaac1080.swf
    python3 tools/gen.py isaac1080.swf gen
    cmake -G Ninja -B build && ninja -C build
    # Windows: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/mingw64.cmake -DCMAKE_PREFIX_PATH=<SDL2-mingw>/x86_64-w64-mingw32 ...
  `gen/` ships pre-generated, so CMake is all you need.

## Automated tests (headless)
    isaac isaac.swf --headless [--norender] --frames N --script script.txt --monkey SEED --wav out.wav [--stats] [--perf]
  script: "<tick> down|up <keycode>", "<tick> shot file.ppm", "<tick> shotf <frac> file.ppm",
  "<tick> eval _root.a.player._x", "<tick> eval tree _root 2", "<tick> eval set <path> <value>",
  "<tick> eval call <path> <method> [args]", "<tick> eval dumpbd <BitmapData path> file.raw", "<tick> quit".
  --stats: logic/render times per tick and collections. --perf: F3 overlay enabled.
  Environment variables: ISAAC_THREADS=n (extra raster threads), ISAAC_DRAWPROF=1 (BitmapData.draw
  profile), ISAAC_LEAFPROF=1 (slow leaves), ISAAC_NOLEAF=1 (draw() without cache, for comparison).

## Switch (homebrew, .nro)
- Toolchain: devkitA64 + libnx + switch-sdl2 + switch-mesa (nouveau, desktop OpenGL via EGL).
  With devkitPro's pacman, just install `switch-dev switch-sdl2 switch-mesa switch-libdrm_nouveau`.
  Without it, `switch/build-devkit.sh` builds everything from source (gcc 16.1, binutils 2.46, newlib,
  libnx, libdrm_nouveau, mesa 20.1 switch branch, SDL 2.28.5), with the libraries built -fPIC (.nro files are PIE).
- Build (DEVKITPRO must be in the environment: libnx's specs read it):
      export DEVKITPRO=/opt/devkitpro
      cmake -G Ninja -B build-switch -DCMAKE_TOOLCHAIN_FILE=cmake/switch.cmake && ninja -C build-switch
  → build-switch/isaac.nro (nacptool + elf2nro with switch/icon.jpg in POST_BUILD; nacp author "native recomp").
- 720 variant (`gen720/`): 960x720 stage with `_root` at 150 % (on PC 1440x1080 at 225 %), which
  cuts the game's fullscreen BitmapData to less than half. Generated the same way as `gen/`:
      python3 tools/patchhd.py --scale 1.5 patch720/scripts/frame_1/DoAction.as   # copy of the FFDec decomp
      java -jar ffdec-cli.jar -importScript isaac.swf s720.swf patch720/scripts
      python3 tools/patchhd.py --scale 1.5 --bin s720.swf isaac720.swf
      python3 tools/gen.py isaac720.swf gen720
  CMake uses gen720 on Switch (ISAAC_GEN_DIR). With --scale 2.25 patchhd.py reproduces patch1080.
- rt/platform.h: per-platform memory limits (GC threshold, external memory, pixel pool, leaf
  cache, render cache, PBO, atlas pages, raster threads). Building on x86 with
  `-DCMAKE_CXX_FLAGS=-DFL_SWITCH_MEMORY -DISAAC_GEN_DIR=gen720` applies the Switch limits so they can be measured on PC
  (build-swmem: peak ≈ 254 MB RSS while playing).
- Threads: logic on core 1 (16 MB stack), 2 rasterizers on cores 2 and 1, render on core 0
  (core 3 belongs to the system).
- Render (v6): recorded frame (GOp: draw calls + state changes in std::function) replayed in
  renderEndFrame with a single VBO mapping; VBO/PBO rings with fences (StreamRing), no glBufferData per
  frame; asynchronous rasterization (RJob, queue + service threads; the frame waits up to rasterWaitMs while helping);
  no glFinish on Switch (gpu_sync=0). Threads: logic core 1 (0x2C); raster service core 2 (0x2D) and
  0 (0x3C); logic helpers core 2 (0x3B) and 0 (0x3D). Horizon doesn't time-slice between threads of equal
  priority: a render thread must not share a core and priority with long-running logic work.
- v7: "GPU" BitmapData (display.h: displayed/gpu/cpuRead/lazy): layers attached with attachBitmap and never
  read receive draw/fillRect/copyPixels as GpuOps (snapshot.cpp) that the renderer runs in order into an FBO on
  the layer's texture (render.cpp: runGpuOps/renderScene, uFlip = -1). The logic no longer rasterizes or composites; a
  new BitmapData doesn't allocate pixels until the CPU needs them (ensurePx). Reading a GPU layer moves it back to
  the CPU with a warning in log.txt (doesn't happen in this game: only grid/grid2/tests/org are read).
- v7: preloading (PreKey: id, type, variant, scale relative to screen height or absolute for layers): built-in lists
  rt/preload_<game>.inc (tools/preload_list.py merges the ISAAC_LEARN_OUT lists) + learned preload*.txt
  (old precarga*.txt files are merged in once).
- v7: filter cache by uid + content signature; blur with downsampling (1/2 if radius >= 8, 1/4 if >= 24).
- v8: game data from Steam's Isaac.exe (MDM Zinc projector, UPX): after the PE image comes a container
  (header 14 09 13 49 49 13 09 14) holding the SWF as CWS (zlib level 6), preceded by its u32 length and encrypted with
  XOR k(i) = 0x83 + 0x21*i. swfload.cpp: swfFromExe (also Adobe projectors and unencrypted SWFs). Verified:
  the decrypted SWF is byte-for-byte identical to the v1.48 .swf (md5 055f7891...). Binding_of_Isaac.exe is the launcher.
- v8: asynchronous preloading (renderPreload enqueues into g_rqPreQueue; renderPreloadPending advances it each frame) during
  the intro cutscene, with input blocked (pushIn drops keys/mouse) until it finishes.
- v8: seams between adjacent fills: on empty surfaces (textures, leaves) paintShape paints front to
  back with PLUTOVG_OPERATOR_SATURATE (added to third_party/plutovg: dst += src * min(coverage, 1 - dst.a)).
  In the interior it's equivalent to reversed SRC_OVER; on shared edges it leaves the pixel opaque, like Flash.
- v8: mdm.FileSystem.deleteFile("serial.txt") (Delete Save) → g_restartRequest; main relaunches the executable
  (Switch: envSetNextLoad; Windows: CreateProcess; Linux: execv). On Switch hbloader loads the .nro again in the
  SAME process: all helper threads must be stopped first (platformStopThreads; the loops check
  platformStopping()), or the previous program's threads stay alive and corrupt the new one's heap.
- Memory limits (g_lim) chosen at startup: Switch in application mode (>1.5 GB) uses the PC ones.
- Touch: SDL_FINGER* → stage mouse outside of gameplay (the game's on(release) buttons).
- Input: libnx pad → game keys with three mappings (gameplay / pause / menus) depending on the snapshot.
  L3+R3 = F3. Everything (swf, ini, save, log.txt) lives in sdmc:/switch/isaac/.
- Tests: cmake/aarch64-linux-test.cmake builds for aarch64-linux (static) and runs under qemu:
  same deterministic result as x86 (--monkey). Also tested on real hardware.
- Fixes that came out of the port and benefit PC: Mouse.removeListener() with no arguments (it cleared the
  listeners and kept each room's clip alive: ~1 GB after a long run) and shape collision polygons
  computed only when needed (−95 MB).

## PSP (shelved)
Started for v1.5 and shelved: it runs on a real PSP but at about one frame every 2-4 s (drawing a frame took 140-280 ms
on the console against 1-3 ms in PPSSPP, and the game's logic 25-95 ms per tick in rooms; the budget is 33 ms). The
code stays (it builds, and the shared changes it brought help every platform: `tryGetProp`, 32-bit `PropMap::hp` on
32-bit targets, weak auto instance names). The notes below are kept for a future attempt (a much lighter game, or the
Vita).

### PSP notes
Build: `cmake -G Ninja -B build-psp -DCMAKE_TOOLCHAIN_FILE=$PSPDEV/psp/share/pspdev.cmake && ninja -C build-psp` (pspdev,
gcc 15). Targets: `isaac` (EBOOT.PBP: the launcher, no game code) and `isaac_<game>` (`bin/<game>/EBOOT.PBP`).
Test in PPSSPP: `PPSSPPHeadless -j --memstick <dir> --appendconfig <ini with [SystemParam] PSPModel = 1> <EBOOT.PBP>`
(`--jit-profile FILE` for a profile of the hottest blocks; symbols from an unstripped build: CMAKE_BUILD_TYPE other
than Release). `saves/autotest.txt` drives a test run (main_psp.cpp: buttons, keys, clicks, god mode, quit) with the
frame clock, and turns the performance log on.
- Stage 363x272 (`genpsp/`, patchhd.py --scale 0.566667): the PSP screen's height, 4:3 in the middle (scissor).
- Memory (64 MB models, MEMSIZE = 1; ~41 MB of heap after the program):
  - One game per binary: the three games' code (~5-7 MB of text each) don't fit together. swfload.cpp refers to the
    `g_code_*` tables weakly; psp_game.cpp pulls one into each binary. The launcher starts `bin/<game>/EBOOT.PBP` with
    `sctrlKernelLoadExecVSHWithApitype(0x141)` (custom firmware's SystemCtrl, as the XMB launches homebrew: the child
    gets MEMSIZE's memory) and falls back to sceKernelLoadExec (emulators). argv[1] = the .exe chosen.
  - Lean SWF (FL_LEAN): decompressed once to `saves/swf_<len>.cache`; in RAM only the tags without sound data, shape
    records or DoAction bytecode (stubs with file offsets): 3.2 MB instead of 36. Shapes parsed on demand (LRU,
    5 MB, never one used in the last 2 s), sprite timelines parsed when first instantiated (`spriteEnsure`; 25 MB
    if parsed up front), decoded SWF bitmaps dropped over 1.5 MB (`rasterTrim`).
  - Room layers (floor, blood, gibs) at the PSP's scale: `gen.py --room-scale 0.566667` sets the game's own quality
    tier (gridxs x gridys at hdx) to 364x272 at 0.566667 instead of 640x480 at 1; the collision grid keeps the
    640x480 size. Verified identical to Ruffle (unpatched SWF) in fights of every game.
  - BitmapData pixels and rasterized shapes are stored in the GE's order on the PSP (0xAABBGGRR, `kPxABGR`,
    swfdata.h): pxFromArgb/pxToArgb at the script boundary (getPixel, setPixel, fillRect), colorOf swaps for plutovg.
    BitmapData textures are drawn in place (width a multiple of 4, 16-byte aligned) without a copy.
  - Large blocks (>= 64 KB) come from their own 24 MB arena (psp_alloc.cpp, malloc & co. wrapped): mixed with the
    small objects, newlib's heap fragmented until a 300 KB request failed with 11 MB free.
  - GC: gcMinLive 15000, growth 1.4, and a collection when either heap runs low (`g_gcMemPressure`).
  - Automatic instance names ("instanceN") are interned weakly (internWeak): pinned forever they grew without limit.
- Speed (PPSSPP's estimate of 333 MHz; real hardware may be slower: memory latency isn't modelled):
  - The FPU is single precision: doubles are emulated. libgcc's fp-bit routines took over half the CPU; LLVM
    compiler-rt's (third_party/crt_fp) are several times faster with bit-identical results (300,000 random cases
    against x86). AVM numbers stay doubles (the game encodes ids in decimals: 5.01...); the renderer uses floats.
  - Menu focus search every 4th tick; audio: resampling position in 16.16 fixed point, 16-bit output, 24 channels,
    short sounds decoded once and kept as PCM (1.5 MB LRU).
  - fillRect of a whole bitmap only clears what changed since the previous one (row spans): the game clears a
    300x300 collision-pair table every tick.
  - Rasterization: at most ~10 ms of new graphics per frame (the rest in the next frames); plutovg's cell pool 1 MB.
  - Single thread: input, logic, snapshot, GC, render, present (2 vsyncs = 30 fps, 1 when late).

## Input latency (v1.1)
- Input is read continuously: `pollInput()` runs at the start of each frame and also every ~1 ms while the main
  thread waits (Switch: GPU fence wait; PC: until 3 ms before the expected vblank, then glFinish; no vsync: the frame
  limiter). An input no longer waits for the next frame, and no longer slips to the following logic tick when a tick
  boundary falls inside that wait (model: ~8 ms less on average from press to first visible movement).
- `interp_lead` (0..0.5, default 0): simT += lead·dt, i.e. optional partial extrapolation of the interpolated view.
  Rendering only; the logic is untouched.

## Sound clip leak (v1.1)
- All three games remove each sound's `soundz` clip when `s.duration == s.position`. `position` returned 0 once the
  channel had finished, so every sound leaked a clip under _root (~4000 after 30 min: logic 30 ms/tick, GC 150 ms/s,
  eventually bad_alloc on Switch). Now SoundObj keeps the last position, and a sound that played to the end reports
  position == duration (Flash behaviour). `duration` uses the SWF rate field (5.5/11/22/44 kHz).
- Preload: keys above q100 = 800 (4x) are neither learned nor preloaded; learning dedupes by scale bucket; preloading
  stops at ~60% of the atlas (estimated from content bounds).

## Creep / blood damage (v1.1)
- All three games test floor creep with `blorz.hitTest(x + _X, y + _Y, true)` (`shit()`), and enemies on hazards
  with `dang2.hitTest(xp, yp, true)`. Flash takes that point in **_root coordinates** (the docs say "Stage"; Ruffle
  does the same: `avm1_root().local_to_global`). We treated it as stage coordinates, and since _root is scaled
  (x2.25 PC, x1.5 Switch), the test landed 1.5-2.25x away from the player: blood far away hurt, blood underneath
  didn't. `mc_hitTest` now maps the point through `_root`'s matrix. Repro: `eval call _root.a parc bloo 300 300`
  then `eval call _root.a shit 300 300` (true) / `shit 675 675` (false; it was true before).

## GC hitches (v1.1)
- Switch perf logs after ~30 min: phases of 30-90 s with GC 200-650 ms/s (lazy sweep at its 18 ms/tick ceiling),
  logic 15-38 ms/tick, "logic delayed" hitches, and 20-70 ms render-thread `adopt` spikes at the same moments
  (snapshot frees competing for the malloc lock).
- ~70% of the garbage was short strings rebuilt every tick ("m12-3.mp3", "hat2", numbers as text). `mkstr` now has
  a weak 8192-entry cache for strings <= 48 chars (Strs are immutable; entries not marked are dropped right after
  each mark). Garbage per tick in Caves: 117-254 -> 49-75 objects. Headless runs stay bit-identical.
- `gcCollect(tickMs)`: the sweep never takes the tick past 12 ms (minimum gcSweepMs, so it always progresses), and a
  mark is postponed on a heavy tick unless the heap is 1.5x over the trigger.
- Replaced snapshots are handed back to the logic thread (`renderFreeRetired`) instead of being freed on the render
  thread.
- Diagnostics: `ISAAC_GCHIST=1` (garbage by type at each collection), `ISAAC_STRPROF=1` (dynamic strings created, by
  pattern), `--stats` prints the total GC time.

## Android (v1.2)
- Same program built as `libmain.so` (SDL 2.30.9's Java activity, fetched by CMake from GitHub) with `gen720/`;
  arm64-v8a, minSdk 24. Gradle project in `android/`; `.github/workflows/android.yml` builds the APK in CI (Google's
  SDK hosts are not reachable from the development sandbox). See `android/README.md`.
- Renderer: `g_glES` → `#version 300 es` shaders and RGBA8 textures. ES has no BGRA upload format, so `texUpload`
  swaps R/B while copying into the PBO (same cost as the memcpy). Output is pixel-identical to desktop GL (max
  difference 2/255 on Mesa). `ISAAC_GLES=1` tests it on PC.
- Data: `/storage/emulated/0/IsaacRecomp/` (from `IsaacActivity.rootDir()` via JNI) holds Isaac.exe, isaac.ini,
  saves, log.txt, performance.txt. Needs MANAGE_EXTERNAL_STORAGE (Android 11+) / storage permission (older): native
  `main` waits (pumping events) until `hasStorageAccess()`. Process is `exit(0)`ed at the end (Android may reuse it).
  "Delete save" restarts through `RestartActivity` (separate process). Background: P pressed if playing, logic
  thread frozen (`logicPaused`) until foreground.
- Touch (`rt/touch.cpp`, `TouchControls`, main thread): in gameplay (snapshot `gameplay`) a finger on the left half
  spawns the movement stick (WASD, 8 sectors), on the right half the shooting stick (arrows, dominant axis with
  hysteresis); floating base; dead zone 18%. HUD targets come from the logic thread each tick
  (`recompTouchTargets`: `hud.cacher` middle row = bombs/E, `hud.it` = Space, `hud.extra` when visible = Q; same
  layout in the three games) and win over the sticks. Keys are held at least 90 ms (game polls at 30 Hz).
  Menus: the finger is the mouse; a short tap also sends an InputEv type 4: the logic thread presses Space for 3
  ticks if there is no button under it and no button on screen (`touchButtonAt` / `touchAnyButton`) - skips cutscenes,
  title cards. Back key: P in game (pause/unpause), Esc in menus (title: the game's quit dialog).
- Controllers (all SDL platforms): context-aware mapping like Switch (gameplay / pause / menu): A confirms and B goes
  back in menus; Start/Back pause in game.
- First device test (a mid-range phone): menu 18-25 fps (17 blurs on the title
  screen; each recomputed blur = FBO switches, very expensive on tiled GPUs), gameplay ~50 fps with many HELD
  frames, preload 34 s, and a tap skipped the intro during the preload (type 4 was not blocked). Changes:
  * render resolution: `IsaacActivity.setRenderHeight` → `SurfaceHolder.setFixedSize` (the display hardware scales
    for free). `IsaacSurface` re-normalizes touches by the view size (SDL divides by the buffer size). Default 720
    lines; automatic mode steps down 720→600→540→480 when gameplay stays <88% of the refresh for 10 s with the time
    in render/GPU wait (not HELD), stored as `auto_height`. `render_height` forces a value.
  * `gpu_sync` off on Android (glFinish after swap serialized CPU and GPU): fence pacing as on Switch.
  * blurs on mobile (`filterBudget` 4/frame, `filterMaxAge` 3, `filterLowRes`): changed blurs beyond the budget
    show their previous result (refreshed at least every 3 frames); smaller RTs. Overlay shows `BLURS (REDONE)`
    and upload time.
  * threads: logic and render pinned to the fastest cores (cpuinfo_max_freq), helpers to the others
    (`platformSetThread`: prio 0x2C = main roles).
  * taps blocked while preloading; touching shows "LOADING n%" (`renderHint`). `_exit` at the end (exit ran static
    destructors under live helper threads: FORTIFY "destroyed mutex"). Saves flushed on the logic thread when
    the app goes to the background.
  * joysticks: base fixed (radius back to 12% of the screen height, as in the first build) (knob clamps at the edge). Pause button right of
    "-LIFE-" (root rect 588,4-616,32; touch area +40%), drawn by `renderPauseButton`.
- Testing on PC: `ISAAC_TOUCH=1`, `ISAAC_TOUCH_INJECT=file` (synthetic touches/back/eval/screenshots in the threaded
  loop), `ISAAC_INPUT_LOG=1`. Under Xvfb without an audio device the opening cutscene can end by itself early; from
  there the title's Start did not react once (not seen when the intro is skipped with a tap).
- Second device test: performance fine. The picture filled only the top-left 2/3 of the screen: SDL adds its
  SurfaceView without layout params, so after `setFixedSize` the view measured itself to the buffer size; now it
  gets MATCH_PARENT (the buffer is then scaled to the screen). Also the activity
  forces immersive mode itself (`goImmersive`, on focus/resume; cutout short edges). Audio sounded worse than on PC:
  we mixed at 44.1 kHz and SDL resampled again to the device's 48 kHz; now the device is opened with
  `SDL_AUDIO_ALLOW_FREQUENCY_CHANGE` (48 kHz requested on Android) and we mix at its rate, with Catmull-Rom
  interpolation instead of linear (3 samples of history across refills; channel end timing and Sound.position
  unchanged, headless runs stay identical).
- Third device test (input and sound lag). In gameplay DRAW was ~14 ms per frame with ~20 quads: the driver waits
  for a free window buffer at the frame's first draw, after input was read. `renderPaceAfterSwap` now clears and
  flushes the default framebuffer right after the swap on Android, so that wait happens before the next frame polls
  input (same frame rate). Texture uploads go straight from memory on Android (the PBO path stalled: 0.2 MB/s cost
  up to 100 ms/s). Audio: 512-sample buffer on Android, and CMake patches SDL 2.30's AAudio backend to request
  `AAUDIO_PERFORMANCE_MODE_LOW_LATENCY` (SDL leaves that call unused; without it Android picks large buffers).
- Fourth device test (a 120 Hz tablet). Audio sometimes started as a rough "motor" buzz that went away when another
  app opened an audio stream (screen recording): continuous underruns. With LOW_LATENCY the AAudio stream's buffer
  size defaults to one burst, smaller than SDL's 512-frame write. The CMake patch now also enables
  `AAudioStream_setBufferSizeInFrames`/`getBufferCapacityInFrames` and sets the buffer to 2x the SDL block after
  open/reopen (capped by the capacity). Input lag: render 1-3 ms, many HELD frames at 120 Hz; what remains is the
  30 Hz game tick plus interpolation (up to ~1 tick); `interp_lead` trades part of it for slight overshoot.
- Analog movement (`joystick_acceleration`, default 1; all platforms with a stick): the game only knows 8 digital
  directions, so a native wrapper around `_root.a.dirkey` (installed by `installMoveScale`, re-wrapped if the game
  redefines it) scales the player's `xenf`/`yenf` by `g_moveScale` after the original runs. The main thread sets
  `g_moveScale` from the movement stick each frame: `moveCurve` = 0 inside the dead zone (touch 18%, controller
  20%), then 40% rising linearly to 100% at 75% of the stick's travel, never above the original speed. Keyboard,
  D-pad and `joystick_acceleration = 0` keep scale 1 (headless runs and determinism unaffected). Switch: in
  gameplay the left stick position is read directly (`padGetStickPos`) instead of the digital StickL buttons.
  Measured: scale 1 / 0.7 / 0.4 = 211 / 148 / 85 px/s.
- Switch (v1.2): `touch_controls` defaults to 1 on Switch as well, so the handheld touch screen uses the same
  `TouchControls` as Android (sticks, HUD taps, menus as mouse + tap = Space). The old Switch-only path
  (`switchTouch`: menus only) remains for `touch_controls = 0`. The pause button is drawn only in handheld mode
  (`appletGetOperationMode`). The .nro metadata (nacptool) carries the author and version 1.2.0.

## v1.3 fixes
- HUD hearts (>12): the game recolors every heart each tick with `colorit` (`new Transform(mc).colorTransform =`), then
  `hud.lif.gotoAndStop(2|3)` for the 24/48-heart layout, whose PlaceObject moves/shrinks h0..h11. We flagged the
  colorTransform setter as "transformed by script" (as Ruffle does), which froze h0..h2's matrix: big hearts on top of
  the small ones. The original Flash Player re-lays them out, so `Transform.colorTransform` now only sets
  `scriptColor` (blocks the timeline's color, not its matrix). Determinism runs unchanged.
- Tears: `create()` attaches the ball at (0,0) after the tick's movement code (`actions1` runs before `playc`); the
  next tick moves it to Isaac and sets `d._y = dy` (-23, the tear's height). The renderer snapped the parent's
  teleport but interpolated the child `d` from 0 to -23: the tear appeared low and rose. Now a teleported node's
  subtree snaps with it (`NodeInterp::jumped`), and a node created at (0,0) in the previous tick that then moves is
  treated as a teleport (`Snapshot::fresh`).
- Stuck shooting: SDL controllers shared one `padKeys` map between the left stick (WASD in game, arrows in menus),
  the right stick, buttons and triggers, and the left stick only re-mapped when it moved. Holding it into the boss VS
  screen turned WASD into an arrow that stayed pressed once the fight began. Now every source goes through
  `keySrc(src, code, down)` (a key is down while any source holds it); the left stick and held D-pad directions are
  re-mapped when gameplay/menu changes (also on Switch for D-pad/StickL buttons). Focus loss / background clears the
  mask and the controller state. `ISAAC_TOUCH_INJECT` gained `axis <n> <value>` and `button <n> <0|1>`.
- Android audio: SDL 2.30's `aaudio_PlayDevice` writes with a 1 ms timeout and drops the rest of the block. With the
  low-latency buffer full most of the time, most of each 512-frame block was lost (the "motor" sound; screen
  recording re-routes the stream to a path with other timing, which hid it). CMake now patches it to write the whole
  block (50 ms timeout, up to 4 tries; blocking is what paces SDL's audio thread) and to grow the buffer by one burst
  on each new underrun (Oboe-style latency tuning). If re-opening the stream after an output change failed, SDL's next
  write used a NULL stream and crashed (the app restarted): it now waits and retries every second. The manifest also
  handles more configuration changes itself (density, font scale, touchscreen, color mode).
- Touch HUD targets: on a wide phone the bomb row is ~2 mm tall and a tap just above/below it spawned the movement
  stick. Targets are now tested in window pixels, grown to at least 13% of the screen height around their center,
  nearest center wins.

## v1.3 features: controller menus, rebinding, save converter
- Controller navigation (`rt/menuui.cpp`, logic thread). The game's menus are SWF buttons for the mouse. In menus /
  the pause screen the main thread turns D-pad / left stick / A / B into InputEv type 5 (`NAV_*`, auto-repeat 0.38 s
  then 0.11 s); `padNavEvent` keeps a focused button among the visible, enabled, on-stage ones of the topmost
  `_root` child that has any (options sheet over the main menu, ...), moves it to the nearest button in the pressed
  direction (distance along + 2.2 x sideways), hovers it with a mouse move (the game's own rollover art) and clicks it
  with a synthetic mouse down/up at a point inside its hit shape. B: `_root.paus` on frame 2 (the pause screen) ->
  P (resume), else Escape (the game's Back buttons have `keyPress "<Escape>"`). Screens without buttons get arrow /
  Space / Escape pulses (3 ticks). The snapshot carries the uid of what turns red: the button, or for buttons that
  are only an invisible hit area (the challenges `pauss.cN`, the collection's `col.iN`) the clip around them; their
  rect is the hit area (`hitStageBounds`). The renderer marks every leaf of that subtree with alpha-add = (10 +
  brightness of the parents) * 255, and the fragment shader's "ink" mode turns pixels darker than paper into one fixed
  red, making translucent ink opaque (some labels are black at ~35% alpha) so every option has the same red. Once the
  parents are darker than 0.97 (the game's `darky` fade to black is a colorTransform on `_root`) it isn't shown: the
  old additive tint stayed red over the fade. Direction choice: candidates inside a 45-degree cone or on the same line
  first (distance between facing edges + perpendicular gap), then anything ahead (within 2:1 for up/down; any for
  left/right, e.g. Collection -> Exit Game). Default selection when a screen opens or the selected option disappears:
  Start (first button of `_root.men.men`) when it is on screen; in the paper sheets (`pauss`, `hud.paus`) the topmost
  option that isn't a Back button (key condition 19 = Escape); elsewhere the one nearest the last selection. While the
  candidates' parents move (the main menu camera, sheets sliding in; capped at 45 ticks) nothing is shown or chosen.
  The mouse over a button (or the rebinding rows) sets `Snapshot::handCursor` -> SDL hand cursor (PC only).
- PC audio: the default output device at start-up is opened by name (`SDL_GetDefaultAudioInfo`), so a controller with
  audio that becomes the system default (DualSense on Windows) doesn't take the sound. On SDL_AUDIODEVICEADDED/REMOVED
  the default is re-checked after 0.5 and 2.5 s: the game moves to it if it isn't a controller ("controller",
  "dualsense", "xbox"... in its name) or if its own device was lost. Android / Switch keep SDL's default device. Mouse/touch/keyboard input sends NAV_OFF (outline hidden). Held directions change meaning when the
  game switches between gameplay and a menu (re-evaluated every frame). Headless: `ISAAC_MENUUI=1` + `eval nav N`.
- "REBIND CONTROLS" entry: `_root.paus` (pauss in menus, hud.paus in a run) frame 8 is "Control options" in the
  three games (same layout); a synthesized text field is placed in it at script depth (local 64,186), removed on
  other frames. Click/tap (`menuMouse`) or controller confirm -> `g_rebindRequests` -> snapshot -> main thread opens
  the screen.
- Synthesized characters: `rectShape` / `textField` build ShapeDef / EditTextDef at run time (ids from 0xfffe down,
  cached by parameters) and `instantiate` them, so the rebinding screen is a normal part of the display list,
  drawn in the game's HUD font (Upheaval, found by name) and scaled with `_root`. `g_synthCharBase` keeps them out of
  the learned preload lists.
- Rebinding screen: state on the main thread (`RebindScreen`, `rt/controls.cpp`), which publishes a `UiModel`; the
  logic thread rebuilds `_root.__controls` when its version changes and returns the row rects (stage) for taps.
  While it is open no input reaches the game. Capture takes the next key / controller button (triggers = 100/101);
  assigning an input already used by another action swaps them. Closing saves isaac.ini.
- Bindings: actions -> keys (SDL keycodes) / controller inputs (SDL GameController numbering everywhere; Switch
  buttons mapped by position). A device that was never customized keeps the original behaviour exactly; once
  customized, its inputs are translated in gameplay (and Pause in the pause screen) to the key the game reads for that
  action given `so.data.frog` / `so.data.left` (`gameCode`), and original action keys that are no longer bound do
  nothing in gameplay. Controller: `controller_move_stick` swaps stick roles. Touch: side and joystick size.
- Save converter (`tools/save_converter.html`, shipped in the packages as `save_converter.html`): ISAACSO1 <-> Flash
  LSO (AMF0). Keeps every field both ways except the Flash-only `wind`/`res` (not written to IsaacRecomp); writes
  `cocks = true` to Flash saves (otherwise the Flash game restores stats from its serial.txt backup); empty cols/boss
  slots become 0 in Flash and undefined in IsaacRecomp, like each game writes them. Tested both ways and round trips
  on saves of the three versions.

## v1.3 follow-ups
- "Rebind controls" label: written with hand-drawn stroke glyphs (`rt/menuui_glyphs.inc`, original artwork; small
  caps like the game's marker labels) as one synthesized line shape, cap height 26 at (74,155) in the options sheet.
  The game's own menu labels are plain shapes (no font to reuse).
- The Hourglass (removed in v1.4, see below): slowed enemies advance their animation only on even ticks (`trgnextd` skips `nextFrame` when
  `trg.free && slow > 0 && !fra2`), while attacks fire on `trg.d.d._currentframe == N`, so each attack frame was seen
  on two ticks. `trgnextd` is wrapped natively: on a held tick the clip gets `holdTick`, and `_currentframe` reads
  N + 0.5 for the rest of that tick (equality and end-of-animation checks fail, range checks still pass).
- Touch: a gameplay <-> menu switch no longer drops the joysticks; their keys are released and re-pressed from the
  finger's last position when play resumes (`Stick::fx/fy`). Joystick type 2: the base is dragged so the finger is
  never farther than the radius.
- Touch taps: whether a tap may become Space is decided by the screen at finger-down (`tapOnButtons`): the tap's own
  click could change the screen first (Credits button -> credits, which a Space skips).

## v1.3.1
- Controller focus colour: the shader's ink mode has two targets, selected by the alpha-add marker: 10 + fade =
  pastel pink (0.78, 0.53, 0.53), 20 + fade = deep red, used only for the main menu's Start (`Snapshot::focusStrong`).
- Navigation: the "same line" test uses the middle half of each rect's perpendicular span. The collection's hit areas
  (70 px, 53 px apart, grid tilted ~5 degrees) overlap their neighbours, so with full spans the item to the left
  counted as "below" and moves went sideways. Score adds 0.3 x the centers' perpendicular offset. Collection opens on
  `i0`.
- Dialogs: a button whose hit area covers >= 80% of the stage (the exit note `men.ex`: "No" is everywhere outside the
  note; also the sheets' Back buttons) restricts the candidates to the clip that contains it; otherwise the exit note
  shared the `men` layer with the main menu and the selection stayed behind it.
- PC saves: `<program folder>/saves/` (was SDL_GetPrefPath; the old files are copied once, never deleted; if the
  folder can't be created, the old location is used). Windows file names are UTF-8 everywhere: the exe carries a
  manifest with `activeCodePage = UTF-8` (`windows/isaac.rc`; Windows 10 1903+), and the save goes through
  `platformFopen` / `platformReplaceFile` (`_wfopen`, `MoveFileExW`) so it works on older Windows too. The restart after
  "Delete save" uses the wide APIs. Before, a user folder with non-ANSI characters made the save unreadable/unwritable.
- Navigation scoring (replaces the above): sideways = distance from the current option's center to the other one's
  span (0 if the center is in front of it); pass 0 needs sideways <= along (centers); score = facing-edge gap +
  0.3 along + sideways + 0.2 center offset. Fixes Fullscreen -> Controls (skipping Music) and Back -> Lefty Flip
  (skipping the wide Rebind label).
- Not options: buttons without any action script (the pause's "Options" left under the in-game exit note on frame 6),
  the pause screen's item icons (`hud.paus.iN`), and, when a full-screen-hit button exists, buttons the mouse can't
  reach (`pointOn`: center or a 5x5 grid must hit the button itself).
- B: on the pause screen itself P; else the dialog's full-screen button ("No"), else a visible Back button (key cond
  19), clicked like the mouse; Escape only when there is none (Escape closed every sheet: Controls -> main menu).
  Default in a dialog: its full-screen button.
- While the screen moves, the current selection stays shown if it still exists; after a press that sets the screen
  moving (`g_afterClick`, 3 ticks) nothing is shown until it settles (the old option used to slide away with the
  camera and the new default appeared after it, which looked like flicker).
- Character select "Random" (`rander`) only acts when more than one character is unlocked (original behaviour).
- PC audio at start-up: if the default output is a controller, the best other output is opened (names with
  speakers / headphones in several languages first; HDMI, digital and virtual outputs last).
- Touch bomb target: `hud.cacher` exists only in Eternal; the original and WotL have the coin/bomb/key icons directly
  in `hud` at the same place, so the target comes from the same HUD-local rect (249.8,11.1)-(279.4,66.3).

## v1.4: differential tests against Ruffle (enemies)
- `tools/difftest/`: a patched Ruffle (`isaac_harness`) runs the same HD-patched SWF and test script as the recomp,
  with Flash's random generator seeded identically (`ISAAC_AVMRNG`) and a virtual clock; ball/player/room state is
  compared tick by tick. Enemy runs: every enemy type of the three games alone in the first room (600 ticks of
  random input and shooting); room tours: 12 random rooms per floor on every floor, god mode, random input.
- Random numbers: the recomp now uses Flash Player's generator (avmplus RandomFastNext + RandomPureHasher, as Ruffle):
  Math.random() = n / 2^31, random(max) = n % max. Seeded from the clock in play, fixed in headless tests.
- getTimer() in tests: `tick * 1000 / fps` (was `tick * (1/fps) * 1000`, which gave 27599 instead of 27600 and changed
  the number of random numbers the game burns at the start of each floor).
- `_x` / `_y`: stored in whole twips truncated toward zero (Flash/Ruffle), not rounded; +-Infinity -> -107374182.4;
  NaN/undefined/null ignored, Infinity accepted (was rejected).
- gotoAndStop/gotoAndPlay(frame): only a whole number is a frame number. Anything else (a fractional number too) is
  converted to a string and resolved as "path:frame" / "path.frame" (right-most ':' or '.'), the frame being a
  number (Rust f64 syntax) or a label. So `gotoAndStop(567.5)` looks for a clip named "567" and does nothing; the
  recomp went to frame 567 (clamped to the last one): a WotL boss (type 97) flashed its last sprite frame for a tick
  when hit. Frame labels are case-insensitive (stored lowercase).
- hitTest(x, y, true): exact port of Ruffle's shape_hit_test (winding numbers on the SWF edges in twips, one-sided
  fill edges, even-odd per style layer, strokes hit within half their width, min 1 px on screen) and of its mask
  handling in clips (children under a mask only hit where the mask does). The flattened polygons (6 segments per
  curve, strokes ignored) were off by up to ~1 px near curves: on WotL's spider boss (type 100) the web creep caught
  the player one tick later than in Flash (`shit()` -> `blorz.hitTest`), so movement diverged.
- Results after the fixes: every enemy type of the three games (72 OG, 90 WotL, 90 Eternal; 600 ticks each) and room
  tours of every floor (12 rooms x 300 ticks; 40-50 different enemy types met per game) identical to Ruffle, including
  rotation, scale, alpha, visibility and animation frames. Eternal runs without `mdm` there ("nozinc": _root at
  125 %), so it uses `tools/difftest/start_eternal.txt` (mouse clicks at that scale).
- The Hourglass fix (v1.3, `trgnextd` wrapper) was a deviation: Ruffle confirms the original fires twice per attack
  while slowed. Removed in v1.4 (the original behaviour is kept).
- Fistula (OG, types 71-73): identical to Ruffle in the empty room and in real rooms. It does not cross pits in the
  original either: non-flying enemies collide with `mhit()` (any `levz >= 1`, pits are 3); only `flyby` enemies use
  `mhitx()`, which lets them over pits.

## v1.4: bosses
- Each boss room template (`_root.levz[_root.bossl]`, b1-0 ... b42-x, horsemen h1-h5, alt bosses a*, demon, satan,
  krampus...) is played on its floor (`tools/difftest/gen_boss.py`: `newstart` up to the chapter, teleport to the boss
  room with that template, auto-aim at the nearest enemy (`eval aim 1`, same rule in both harnesses), random movement,
  god mode) and compared with Ruffle tick by tick until the boss dies.
- for-in over arrays: Flash (and Ruffle) store array elements as ordinary properties, so `for (z in arr)` returns the
  elements most recently ADDED first, not by descending index. `levz = new Array(200)` is filled in tile order, and
  `for (z in levz)` (killing every poop when a boss dies, `killa`) visited the cells in another order: the random
  numbers went to other cells (WotL b2-0, splash rotations, then everything). `Array::ord` keeps the insertion order
  once an element is added below an existing one; while arrays only grow upwards the old descending-index order is
  the same thing. Array setProp also left gaps as `undefined` instead of holes (they were enumerated).

## v1.4: items
- Each item (OG 1-132, WotL/Eternal 1-198) is put on a pedestal under the player (`create(x, y, 0,0,0,0, 5.1 +
  item/10000)`), picked up, then three waves of enemies are fought with auto-aim; the active item is charged
  (`_root.itc = 1`) and used every 90 ticks, bombs are placed (`tools/difftest/gen_item.py`). Pills, cards and trinkets:
  `5.07 + col * 1e-9` (col 1-6 pills, 7-28 and 70-74 cards, 29-61 trinkets), used with Q.
- _xscale / _yscale / _rotation: ported Ruffle's transform cache. The values set by script are kept (percent,
  degrees) and returned exactly; the matrix is rebuilt from them with f32 a..d; after a matrix change (timeline) they
  are derived from the matrix (atan2 * 180/pi, Rust's to_degrees / to_radians constants). The recomp stored radians
  and unit scales, so `_rotation` read back with an error in the last bit: Mom's Knife (item 114) turns with
  `knife.d.z._rotation += f1 * 0.7` and computes its position from it, so it drifted by 1e-12 px per tick until it
  hit something one tick apart from Flash. `_width` / `_height` setters follow Ruffle too (rotation-aware).
- The Hourglass (item 66, and Eternal's item 124): identical once the v1.3 slow fix was removed (v1.4: reverted).

## v1.4: physics / HD patch audit
- Ruffle with the ORIGINAL swf (800x600 stage, _root at 125 %) vs Ruffle with the HD-patched swf (1440x1080, 225 %),
  same input and seed (`tools/difftest` cmp_scale idea: only the start click differs): OG 72 enemy types (alone and
  killed), 8 floors of rooms, 132 items (1-66 all, odd ones above); WotL 90 enemy types, 8 floors, odd items 1-99.
  399 runs, all identical: the HD patch does not change the game (positions, knockback, hit tests...).
- getTimer() in play is the real time since start (as Flash): the game burns getTimer() % 100 random numbers at the
  start of each floor (`levz.length <= 1`), and the frame clock (n * 1000 / 30) only gave 0 / 33 / 66. Headless runs
  keep the frame clock (deterministic, as the Ruffle harness); `ISAAC_FRAMECLOCK=1` forces it in play. setInterval
  follows the same clock. The AUTO quality code may now see < 30 fps after a slow frame and lower `_quality`, which the
  renderer ignores.
- `-ffp-contract=off` for every target: ARM compilers (Switch, Android) contracted a*b+c into fma by default, so the
  physics differed from PC / Flash in the last bit.
- Known, not changed: SharedObject saves array holes as undefined (the game only tests them for truthiness); Eternal's
  HD scale (Zinc path) can't be checked in Ruffle (no mdm), same patch as OG/WotL.

## v1.4: input delay on Switch (performance.txt from a real console)
- Floor 1: logic ~2.5 ms/tick, display margin ~4.5-5 ms, ~30 extrapolated frames/s. Busy rooms on floors 2-3: logic
  5-7.5 ms (GC 40-64 ms per second with 16+ entities), margin 8.7-11 ms, 6-15 extrapolated frames/s: ~5-10 ms more
  input-to-image delay. Render draw ~16.2 ms per frame on every floor (no headroom, but constant).
- `gc_after_tick` (isaac.ini, default 1): gcCollect runs after the snapshot is published, so it no longer counts in
  `pubDelay` (the margin follows pubDelay). `max_margin` (ms, default 6 on Switch, 0 = automatic elsewhere) caps the
  adaptive margin; late ticks are extrapolated (alpha up to 1.35). Both are written into an older isaac.ini with their
  explanation (`kLateSettings`).
- Testing aids: `ISAAC_TICKMS=1` (headless: cost of every tick), `ISAAC_MIXMS=1` (headless: cost of mixing 1/30 s).
- Switch layout (and Android: IsaacRecomp/saves/): saves / preload / performance.txt in sd:/switch/isaac/saves/ (copied from sd:/switch/isaac/ the
  first time), tools/save_converter.html next to the .nro, as on PC.

## v1.4: sound
- Mixer locking: the audio callback used to run inside SDL's device lock and the game took the same lock for every
  Sound call (position / getVolume / setVolume / start / stop): every tick the game asks each playing sound for its
  position (the `sz` loop), so it waited for the whole mix (MP3 decoding included). Now each channel has its own
  mutex, held only while that channel is mixed / started / stopped / seeked; what the game reads every tick (active,
  decoder alive, played position, stream position) is published by the mixer in atomics. Volume / pan are atomics.
- Timeline sound envelopes (SoundInfo HasEnvelope): parsed and applied like Ruffle's EnvelopeSignal (levels / 32768,
  linear between points, positions at 44.1 kHz scaled to the output rate, counted from the start). Used by the
  ending animations (Chest_Drop at ~30 %, "Satan find" at 25-50 %), which played at full volume.
- Channel limit (48): when all are busy the oldest sound that isn't music (looping, a stream, or longer than 10 s) is
  replaced; if every channel holds music the new sound is dropped. The music used to be the first one replaced.
- Switch mixes at 48 kHz (the console's rate) like Android: no second resampling by SDL.
- Opening a chest that gives a pedestal (or some other outcomes) pushes Isaac away in the original too:
  `xbew -= dx * (70 - d) / d * 0.4` (up to ~25 px/tick when touching it); identical to Ruffle in 22 runs.
- Icon: windows/isaac.ico (16-256 px, from the Switch / Android artwork) as resource 1 in windows/isaac.rc (SDL uses the
  first icon group for the window class); Linux: rt/icon64.inc (64x64 RGBA) with SDL_SetWindowIcon.
- Android build: core.cpp's ISAAC_RNGTRACE backtrace is Linux-only (`!__ANDROID__`: bionic has no backtrace() before
  API 33). Syntax check with an empty execinfo.h first in the include path to catch this on PC.

## v1.5: mods
- `mods/*.mod` next to the program (Android `IsaacRecomp/mods/`, Switch `sd:/switch/isaac/mods/`): ini-style text,
  `id` (required; `[a-z0-9_-]{1,40}`, lower-cased), `name`, `version`, `author`, `description`, `engine` (defaults to
  the id); every other `key = value` is an option of the mod (`ModInfo::opt`). Turned on by id with `mods = a, b` in
  isaac.ini (`ISAAC_MODS` overrides it, for tests). One mod per engine. Everything is logged in log.txt.
- Engines are built into the program (`kEngines` in mods.cpp; `modsTick` dispatches them). A mod file whose engine this
  version doesn't have is listed but not loaded. No scripting: a mod is code in the runtime plus a file that turns it
  on and carries its options.
- The mods in the source tree's `mods/` are built in (CMakeLists.txt turns them into `build/gen_inc/mods_builtin.inc`)
  and written into the mods folder the first time the program sees each one (a deleted mod stays deleted - see "v1.5:
  closing round" -, a new version's new mods appear). This is how Android gets them: the APK has no folder
  to unpack. Folder listing / files go through `platformListDir` / `platformFopen` (UTF-8 paths on Windows).
  `warning` and `help` (a page in tools/) are only for the mod manager.
- `modsTick()` runs on the logic thread after the frame's scripts and `recompPatchesTick()`, before the snapshot, in
  both the headless and the threaded loops.
- tools/mod_manager.html: picks the game folder with the File System Access API (Chrome / Edge / Opera: reads
  `mods/*.mod` and `isaac.ini`, writes both back) or takes dropped / chosen files in any browser and downloads the
  new isaac.ini. It only rewrites the `mods =` line (added with its comment if missing; CRLF kept) and, for options
  changed in the page, those lines of the .mod file. Its .mod parser follows mods.cpp.

### StatOn
- Read-only, and verified so: with it on, item runs (wotl 7 items, eternal 7, og 3; 600 ticks each, `batch_item.sh`
  with `ISAAC_MODS=staton`) give output identical to Ruffle's line for line.
- Damage is the game's `_root.a.dmgdo()`. It touches the RNG and writes `_root.firrb` and `a.chal`, so StatOn saves and
  restores the RNG state and call count (`rngSave` / `rngRestore` in core.cpp) and puts both variables back (erasing
  them if they didn't exist). Eternal fires tears with an extra x1.15 when `ups[4]`, `doub`, `ups[7]` or `ups[12]`;
  StatOn shows that. Called every 6 ticks.
- Other stats: speed `_root.playsp` (recomputed by the game every tick), tears `30 / (_root.firra + 1)` per second,
  range `_root.firrr * 6.5 / 23`, shot speed the per-version multiplier from `a.ups` (a copy of `_root.ups` refreshed
  by `upa()`: `_root.ups` can be ahead of it), luck `_root.luck` (undefined in the original game: row hidden).
- Devil / angel room chance (`dealChance`): the formula of the boss room's door set-up (`trg.sat` in the room script,
  the same in the three games): `min(0.3, ups[51] * 0.2) + 0.01`, +0.25 `_root.it == 2` (Book of Belial: `_root.it`
  is the active item's number minus 32, or 40 from item 55), +0.35 `_root.it == 38` (Book of Revelations), +0.35
  `_root.bumkill`, +0.35 `bossdmg` (reset with each room: counted as true outside the boss room), +0.99 `_root.devil`
  (true at the floor start, false once hurt by `playerhurt` with `f2 <= 200`); halved if `_root.lastdev + 3 > chaps`,
  again if `+ 2 >`; never on chaps 1, nor >= 9 (the original: == 9). `_root.devil` 2 = the door appeared, 3 = the roll
  failed. Angel (WotL / Eternal): `_root.altsat = random(2) == 0 && evs == 0 && lastdev > 0` at the floor start; shown
  as half and half, the real roll only once the door opens.
- Drawn as clip `__staton` at depth 1048000 inside `_root.hud` (so it follows the HUD's visibility and scale), with the
  synthesized shapes / text fields of menuui.cpp (`synth*` wrappers) and the HUD's font. Rebuilt when the HUD is
  replaced; removed outside gameplay (`recompInGameplay`). Deltas add up for 60 ticks and fade in the last 15.

## v1.5: GenesisConsole
- `rt/mod_genesis.cpp` (`mods/GenesisConsole.mod`, engine `genesis`). The main thread only routes input (main.cpp:
  the console key - scancode, ` by default, so the key left of 1 on any layout -, SDL_TEXTINPUT, the controller's
  Back / Switch Minus, three fingers on a touch screen) to `genesisKey` / `genesisText`; editing, history
  (`saves/genesis_history.txt`), Tab completion and the commands run in `genesisTick` on the logic thread. While it is
  open, every input goes to it (keys released on open) and the logic loop skips runFrame / runIntervals /
  recompPatchesTick (`genesisPausesGame`), still building snapshots so the console is drawn. Headless runs don't
  pause (scripts count ticks); `eval genesis open|close|type ...|key <n>|<command>` drives it in tests,
  `ISAAC_GENESIS_LOG=1` copies its output to the log. The test injection clock (ISAAC_TOUCH_INJECT) adds the ticks
  spent paused; it has `key|kdown|kup <SDL key name>` and `text <string>`.
- On the Switch, SDL's inline software keyboard (SDL_StartTextInput on A) sends the whole line when it closes: run at
  once. Pad input is ignored while it is shown (it reads the controller itself).
- Nothing saved: `g_saveLocked` makes SharedObject.flush a no-op (exit, background, the game's own flush). Nothing
  unlocked: `_root.locker` (the unlock flags; `unlos()` shows them and calls the Steam achievements) is write-guarded:
  a write turning an entry on is dropped. The write guard is generic: `Object::guarded` (a byte in the padding after
  `kind`) routes setProp, and setMem's array fast path, through `g_writeGuard`; objects without the flag pay one test.
- Game functions used: `create(x, y, 0, 0, 0, 0, type)` (pickups `5 + frame / 100 + col * 1e-9`: frame 1 hearts (col
  1 red, 2 half, 3 soul, 4 eternal), 2 coins (1 penny, 2 nickel, 3 dime), 3 keys (2 golden), 4 bombs (2 double, 3
  troll), 5 chest, 6 locked chest, 7 pill / card / trinket (col 1-6 pills, 7-28 tarot, 70-74 playing cards, 29-62
  trinkets), 8 slot machine; `5.1 + item / 10000` a pedestal; enemies by type, a decimal = variant). The original has
  no eternal hearts, dimes, golden keys, troll bombs, trinkets or playing cards. `give`: as golev() gives starting
  items (`player.empty = false; lastcra = -100; highs = player; player.it = n; powerlevel()`). `kill`: `hurt(ball,
  hp + 1000)` on every ball with s >= 10 (not 33, 37, 42, 44). `goto`: `a.tar = room; a.teller()` (room 166: devil /
  angel by `levz[166]` "satan" / "sata" and `altsat`; 169 the error room). `stage`: `_root.chaps = n - 1;
  newstart(0, 1)`; chaps > 8 other than 11 becomes 9, so `next` from 9 sets 10 first. The alternative floor is a coin
  flip at generation unless `_root.heaven == 2` (reset by the next room's set-up); for the normal floor the floor is
  made again (stageTick) until `altchap` is 0. `doors`: `a.keyd = true` (Dad's Key). `map`: `_root.world = true` +
  `upa()`.
- Held stats: the game recomputes them all the time, so the console wraps functions on `_root.a` (installed / removed
  every tick: each room makes a new `_root.a`; same technique as the analog-movement `dirkey` wrapper): `dmgdo`
  returns the fixed damage; `firr(trg)` (called at each shot: `player.fire` is the cooldown, `_root.firra` what the
  HUD / StatOn read) sets the fixed delay `30 / tears - 1`; `create` changes the player's tears (round(type) 2,
  not `ba` / knife) for range (`dy = -range * 23 / 6.5 - dm * 5`, the inverse of `firrr = -dm * 5 - dy`) and shot
  speed (velocity * fixed / the game's own multiplier); `playerhurt` does nothing in god mode. Speed: `_root.playsp`
  is computed and used inside playc(), so _root is write-guarded while speed is fixed (every write of playsp becomes
  the fixed value).
- Enemy names (`mod_genesis_enemies.inc`): the game has none; Rebirth kept the Flash type numbers, and every type of
  WotL was spawned and looked at to check them (95 shows nothing alone). Item names: `_root.st1` at run time;
  trinkets / cards: the lists in the game's own functions.
- Drawn as `_root.__genesis` (depth 1048500, above the rebinding screen) with synthesized rectangles / text fields in
  the HUD font; text measured with the font's advances (raster.cpp's layout) for wrapping and the caret. Badge
  `_root.__genesisbadge` while a run is on.

## v1.5: WideSaac
- The renderer has a view (snapshot `view` / `viewClip`, set by the logic thread through `g_viewReq`): the stage
  rectangle fitted to the window and centered; `viewClip = false` draws (and scissors to) the whole window. Unset:
  the whole stage letterboxed, as before. View changes are eased (~14/s exponential) and nothing is cut while easing.
  `stageToWindowFor` follows the view, so mouse / touch mapping, the touch pause button and the focus outline do too.
- Outside the 640x480 frame the games draw `_root.overlay` (above the HUD): a tiled background of faces. With
  `fill`, the view is the stage with `viewClip = false`, so that art fills the sides of a wide screen instead of
  black bars (it covers about 1.5 stages; past it, black).
- Hyper Zoom, while playing (`recompInGameplay`, GenesisConsole closed): the view is the room, _root (0, 77.5) -
  (640, 480). HUD pieces are moved / hidden in the snapshot only (`g_snapOv`: per display object, hide and / or a
  replacement local matrix, applied in `addNode`), so the game never sees it and runs are unchanged. Groups (HUD
  coordinates, the same in the three games): `lif` (hearts) to the top right; `it` (active item and its charge
  pips) + its text; the counters block (x 212-300, y < 75: icons, numbers, `kep`, Eternal's `cacher`); `pilll` /
  `extra` (pill / card). Everything else entirely above the room is hidden, and `_root.mapa`. They go over the room's
  right wall: past the room, `_root.overlay` would cover them. `wideHudMove(group)` gives the move to
  `recompTouchTargets`, so the touch areas (bomb counter, active item, pill / card, pause button) follow.
- `g_windowAspect` (set by the renderer each frame) tells the logic thread the window's shape.

- Minimap+ (option `minimap`, Hyper Zoom only): `_root.mapa` is a bitmap the game redraws with every room (its map
  code draws each seen room piece at `(e % 10 * 25 + 15, floor(e / 10) * 10 + 7)` map units and centres the map on
  (130, 40) through `mapa._x / _y`, keeping the extremes in `_root.a.minx / maxxx / miny / maxy`). Shown, its snapshot
  matrix is `translate(room centre) * scale(k) * translate(-130, -40) * mapa.m` (k up to 2.3, fitted to 470 x 300) with
  `SnapOv::alpha` (new: multiplies the node's alpha); a backdrop (`__widemap`, a clip at depth 1047900 in the HUD,
  under `mapa`: the HUD is at depth 16684, mapa 16685) is sized from those extremes. Fades in / out over 4 ticks, and
  to a quarter while Itemnomicon's card is shown (the card is in the HUD too, so under the map).
- Toggle (main thread, `wideToggleMinimap`; `wideMinimapAvailable` is set by wideTick while the Hyper Zoom view is
  on): M (unless M was bound to an action in the rebinding screen; the game doesn't use it), the controller's L3 (in
  place of its default pause: Start / Back still pause; not when L3 was rebound to another action), on Switch L3
  released without R3 (L3 + R3 stays the diagnostics overlay), and four fingers: three fingers wait 0.2 s for a
  fourth before opening GenesisConsole. Tests: `eval wide minimap`.
- WideSaac leaves the HUD's `__*` children (the mods' own clips) alone when it hides the band.

## v1.5: PillLog and Itemnomicon
- Shared: `rt/mod_ui.*` (text fields in the HUD font, measured and wrapped; rectangles as a scaled 100x100 synthesized
  shape). Both draw in a clip inside `_root.hud` (`__pilllog` 1047940, `__itemnomicon` 1047950) and read the room's
  pickups: `_root.a.ball` entries with `s == 5`, not `empty` (a taken pedestal) nor `dones` (being picked up).
  `d._currentframe`: 10 pedestal (`it` = item), 15 shop slot (`d.d` frame 4 an item with `it`; 3 a card / pill with
  `col`; 1, 2, 16 hearts / bombs / keys...), 7 pill / card / trinket on the floor (`col`). Test: `eval pickups`.
- PillLog: `_root.piller[col]` is a pill colour's effect, rolled the first time one is taken (`piller()`; `.2` = the
  positive variant, forced by Lucky Foot / PHD) and reset with each run; the HUD already names the held pill with it
  (`_root.hud.pilll.tex`). `effectOf` is that HUD switch per version (OG: 13 = TelePills, no 14 / 17; Eternal: 17
  Shot Speed, "Full Health?" in hard mode). The pill's look: `pic(col)` = `_root.picers[col - 1]` (OG: col). The
  pickup banner is `st11("Pills here!")` -> `_root.hud.st1.tex`, which the game only writes: PillLog replaces that text
  when the held pill is known (the only write; a determinism run with all the mods on differs from one without only
  in that text). The pause list (when `hud.paus` is on frame 2) shows pill icons as copies of the HUD's `extra` clip
  (frame 11 of its `d`, `d.p` on the pill's frame).
- Itemnomicon: the nearest described thing within `range` of the player (not while paused). Texts and stat lines
  are generated: `tools/itemnomicon/gendata.py` -> `rt/mod_itemnomicon_data.inc` from `texts.py` (names and
  descriptions, checked against the game's code: `powerlevel()` per item, the `ups[n]` / `trixx(n)` uses, the card
  switch) and `measured_*.json` (`measure.py` / `parse.py`: each item given alone with GenesisConsole, before / after
  stats). Tears come from the game's tear formula (`firr()` per version) instead: Ipecac, Mutant Spider and
  Polyphemus set `ups[2]` (the Inner Eye's triple shot) and removing them doesn't clear it, which skews the later
  measurements; the formula matches every clean measurement. Range changes under 0.5 are the game's random tear
  height. Active items get no stat lines (giving one replaces the held one). Charges: the game's `itc` table by
  `_root.it` (item - 32, or - 40 from 55; `itc[n]` set later for the WotL actives). Trinket stats from the code
  (Curved Horn +2 damage after dmgdo's square root, Goat Hoof +0.15 speed, Cancer -2 tear delay, Eternal's Game
  Squid +1 damage). The card goes over the thing, under it if the player stands above, beside it if neither fits.
- Mod manager and `mods/enabled.txt`: Chrome's File System Access refuses `.ini` names ("Name is not allowed" from
  getFileHandle), so the manager can't read or write `isaac.ini` there. It writes the list of mods that are on to
  `mods/enabled.txt` (one id per line, `;` comments) and `modsInit` uses that file, when it exists, instead of
  isaac.ini's `mods =` (logged). It still shows isaac.ini's line if the browser lets it read the file (directory
  iteration), and downloads enabled.txt in browsers without folder access.

## v1.5: BaseTime and The Gone
- Shared: `modsTick` clears `g_snapOv` once per tick before the mods run (each mod adds its snapshot-only changes;
  WideSaac merges into existing entries instead of replacing them). `SnapOv::hasCx` applies a colour transform over the
  node's own. `modsRunSerial()` counts the game's `newstart(true)` calls (new run: from the menu, Replay after dying,
  R held), from a wrapper on `_root.a.newstart` installed while BaseTime or The Gone is on: `_root.newstartt` is set
  and cleared within the same tick on Replay, so it can't be seen from outside. `synthHandText` (menuui.cpp) writes any
  text in the menus' marker style with `menuui_caps.inc` (`tools/hand_glyphs.py`: original strokes for A-Z, digits,
  a few signs; small letters as small capitals).
- BaseTime: game ticks (30/s). Run: starts on the first tick playing (`recompInGameplay`) after the menu / a new run
  (`newstartt` false); counts every tick. Floor time: ticks playing; reset when `_root.chaps` changes. End candidates:
  death (`player.dones` with `player.d` on frame 6; 50 ticks later deathscripts revives with 1up! / Dead Cat / Ankh,
  which discards it) and endings (`_root` frames 9-40: `_root.gotoAndStop(9 + wins ...)` from the chest / big
  chest; Mom's ending in the original goes back to the game, which discards it). Finalized (logged, saved to
  `saves/basetime.txt`, not with GenesisConsole on) at the menu (`_root` frame 5) or a new run; with no candidate it's
  "unfinished". Drawn in `_root.hud` (`__basetime`, left of the map; with Hyper Zoom at the room's bottom left), and
  on the main menu as a clip on the menu sheet (`_root.men.men`, which pans: the main page is at sheet (-227.6,
  -198.6) from _root, the same in the three games).
- The Gone, character select: the ring is `_root.men.men.cha` (WotL / Eternal; unnamed in the original: the child
  with `i0` and `inc`). Its onEnterFrame (keys / arrows change `cha.skiner`, the ring turns toward `skiner * 360 /
  chars`, `_root.skiner` from it, portraits i0..i6 placed with `x = -sin(a) * 100 + 100`, `y = cos(a) * 25 + 65`,
  scale / alpha y, depth `round(y) * 10 + e`) is wrapped: `cha.skiner` == chars or -1 selects The Gone (the game then
  sees 0: Isaac), +1 / -1 from it go to Isaac / the last character; after the game's code the portraits are placed
  again on a ring of chars + 1 (the per-version slot tables: which `i` clip and which skiner value brings each to the
  front) with a pale copy of i0 for The Gone. Its card: `inc` hidden in the snapshot, `__gonecard` in marker text.
- The Gone, in the run (the game plays Isaac): Spirit of the Night given like a starting item (original: Transcendence
  + Ouija Board); every tick `player.hp` and `player.mhp` 0 (the game recomputes mhp each frame from a base of 3; hp
  0 with soul hearts is how ??? lives), `_root.armor` at most 0.5 (a hit then kills by the game's own rule in
  playerhurt), `_root.eta` false, `ups` 11 / 81 / 161 and `catlives` 0 (both `_root.ups` and `a.ups`). `balljunk`
  (the collision handler; `lows` / `highs` the pair) is wrapped: a heart (pickup frame 1, or a shop slot on frames 1,
  5, 13, 17) isn't collected (`nohit = false`: pushed, as at full health); a devil deal (shop slot, `a.satan`) is
  bought with the game's coin path (`satan` false and its price added to the coins for the call, given back if not
  bought), then `_root.evs` / `locker[54]` as the deal path does, and the other deals get `done` (removed). HUD:
  `lif` hidden in the snapshot, "-LIFE-" (part of the band's art) covered by a rectangle in the band's colour; the
  player clip's colour transform inverted (Spirit of the Night's black body becomes white; original: just pale).

## v1.5: later fixes (mods)
- Text raster cache: `EditTextObj::version` (the cache key's variant) is now a hash of the text and colour instead of
  a counter, so a field showing a text it already showed reuses its raster. BaseTime's run timer (a new string every
  tick) was rasterized and uploaded 60 times a second (text + shadow): on the Switch that meant GPU stalls of ~43 ms
  (draw time, "rasterized 2" in performance.txt) a few times a second. It now draws one field per character, so
  after the first seconds nothing is rasterized at all.
- StatOn's range: `_root.firrr` comes from the last tear, whose fall has a random part (`dm = rand() * 0.2`, up to
  0.28 of range): changes under 0.3 are ignored, so the value and its +/- no longer flicker while shooting.
- WideSaac: in the menus (`_root` frame 5) `_root.overlay` is hidden, so on a wide screen the menu's own sheet (much
  larger than the stage) fills it. Hyper Zoom stays on whenever `_root` is on its game frame (2) with a player, also
  paused and on the death screen (whose diary page, `hud.paus` frame 3, is drawn at 0.86 in the room's middle);
  Minimap+ only while playing.
- The Gone's portrait (a pale copy of Isaac's) is created at a free depth (5000): the ring's portraits use
  `round(y) * 10 + e` (400-910), and placing it at 900 replaced Isaac's when Isaac was in front.
- BaseTime: digits sit in fixed-width slots (the timer doesn't shift as it counts) and are centred in them: the
  narrow "1" left-aligned left a hole before the next character ("01 :00"). Main menu: a note of its own (the menu
  notes' paper `0xe7e0c9` and grey ink, tilted 2.5 deg, shadow, tape). With Hyper Zoom the timer is on the room's
  bottom left wall, where Eternal's hard-mode pentagram (`hud.hardon`) is: the mark is drawn 35 units higher
  (snapshot only).
- WideSaac, boss intros: `_root` on its `bosscuts` frame (4) between the boss room's door and the fight showed the
  whole 640x480 (the zoom went out and back in). With Hyper Zoom the view stays the room's and the intro's root
  children are drawn scaled by 402.5/480 and centred into it (`g_snapOv`); `overlay`, `hud`, `mapa` and `olda` are
  hidden. The new `ViewReq::cut` / `Snapshot::viewCut` rectangle (stage pixels) narrows the renderer's drawn area
  (scissor and background quad), so the sides are the clear colour (black) whatever the intro draws there.
- Boss music (patches.cpp, all three games): `soundfra()` ends the boss theme the first frame `_root.a.ashut == 0`
  (`bossmusic = 2`, once per room). A boss that splits on death (Duke of Flies' `flya()` in `deathscripts`) leaves
  one frame with no counted enemies before its spawn is counted: the theme faded and restarted (ashut > 0.9 again)
  and when the spawn died `bossmusic` was already 2, so no "boss defeated" jingle. `_root.soundfra` is wrapped: while
  the room has been empty for fewer than 3 frames, the music code sees `ashut = 1` (restored afterwards). Music only.
- Phase lock (main.cpp, threaded loop): performance.txt showed "HELD" ~30 of 60 frames during play on Switch: a
  tick published just after a frame started is due for that frame but not there, so the frame extrapolates the
  previous motion (alpha up to 1.35) and the tick appears one frame later. Not a v1.5 regression in code, but a
  faster logic (lower margin) made it more frequent. The render loop now measures where frames start (circular mean
  of `fstart mod period`) and, when the tick period is a whole number of refresh periods (60 / 120 Hz), the logic
  loop nudges its clock (`base`, at most 0.4 ms per tick) so each tick is published ~1.5 ms + the recent worst
  publish delay before a frame starts: no extrapolated frames, evenly spaced interpolation (alpha ~0.5 / ~1.0) and
  the earliest possible first appearance of each tick. `ISAAC_NOPHASELOCK=1` turns it off. Timing only.
- `ISAAC_MODPROF=1`: time spent in each mod (mean / max ms, logged every 3000 ticks). All mods are under 0.01 ms per
  tick on PC.

## v1.5: last round (mods)
- Built-in mods are updated in place: when the program's copy of a mod (`mods/*.mod`, built in) has a higher
  `version` than the file in the mods folder, the file is rewritten (`upgradeBuiltin`, mods.cpp), keeping the player's
  value of every option whose comment line above it is unchanged (an option that changed meaning - Itemnomicon's
  `range`, now in tiles - gets the new default). A deleted mod stays deleted, as before.
- Mod manager: an option whose comment reads `; key = a | b | c: ...` is a drop-down list; 0/1 options are tick boxes
  unless the comment gives a numeric range (`range = 0.5-4` with value 1).
- The Gone: the original game only hurts a player with `hp > 0` (`if (lasth - fra < 0 && ... && player.hp > 0 ...)`
  in its hurt function), so hp kept at 0 made him immortal there. In the original his red hearts are kept at 0.5
  (never drawn: the HUD's hearts are hidden): a hit takes the half soul heart (armor) and the game's own death test
  (`player.hp <= 0 or !(player.mhp > 0 or armor > 0)`) kills him. Damage: Isaac's (`firrb = 3.5`).
- The Gone's card on the character select screen is made of the game's own lettering: the cards of the ring's `inc`
  clip (a frame per character, a sprite whose depth-1 shape is the hand-drawn text; identical in the three games)
  are split into outlines, grouped into rows (by height) and letters (by horizontal overlap), and the letters needed
  are taken from known places (`kLetters`: e.g. T, H, E, A, R from YUM HEART) and put together into new shapes:
  "THE GONE" (x1.1) and the item name(s), scaled down to fit. G is a C with a bar (MAGDALENE's G touches its D: one
  outline). Icons (heart, speed, damage, item box) are the clips placed on Isaac's card; the speed and damage tallies
  are Isaac's (II, II). OG: two item rows, TRANSCENDENCE and OUIJA BOARD, as Eve's two items.
- Itemnomicon: no background; texts drawn with eight dark copies around them (`modui::textOutlined`: one raster for
  the eight, same field and text). Names in the colour of the item's quality (Rebirth / Repentance's quality of the
  same item number, `kQuality`; 43 and 61 are not real items). `range` in tiles (45 units each, +12 for the bodies).
- BaseTime: game time = ticks while `recompInGameplay` (pauses, cutscenes, loading excluded); total time = the game
  clock `g_player.timeMs` since the run's first tick (real time in the threaded loop, the frame clock in headless
  tests). Both freeze at a death or an ending and resume if the run goes on. Endings: `_root` frames 9-40, named by
  the floor (`_root.chaps`: 6 Mom (the first Mom kill only), 8 Mom's Heart, 9 Isaac (Cathedral, altchap) / Satan
  (Sheol), 11 The Chest) with ranks 1, 2, 3, 3, 4. The best time (one per `menu_time`: game / total) is the fastest
  run to the furthest-ranked ending; a further ending replaces it whatever its time. A run that reached any ending is
  a "win". basetime.txt: `last_game`, `last_total`, `last_result`, `best_game(_ending)`, `best_total(_ending)` (1.0's
  `last_run` is read as both times; its `best_run` is dropped: its ending is unknown).
- WideSaac, shading: a plain rectangle (one solid fill, one outline of at most 6 verbs, no lines) that covers the whole
  640x480 in `_root` coordinates - among `_root`'s children, the HUD's and the pause sheet's (two levels) - is drawn
  8x larger around its middle (snapshot only), so the pause dimming and fades cover the sides of a wide screen.
  Cutscenes: `bossIntro` also handles the `cuts` frame (between floors) and, once Hyper Zoom was on in the run,
  the endings (frames 9-40).
- RainbowVision (`mod_rainbow.cpp`, engine `rainbow`): daltonization (Fidaner et al. 2005) with the Machado et al.
  2009 simulation matrices (full severity): `C = I + k * Shift * (I - Sim)`, Shift moving red's loss into green and
  blue (protan / deutan) or blue's into red and green (tritan). Applied in the main fragment shader (`uColOn`,
  `uColR/G/B` rows) only while drawing to the window (`Target.fbo == 0`: off-screen layers get it when composited),
  on premultiplied colour (linear, clamped to alpha). Not on the PSP renderer.

## v1.5: final round
- mods/mods.txt replaces enabled.txt / isaac.ini's `mods =` and the per-file option edits: one `[id]` section per mod
  with `on = 0/1` and its options (comment lines: the mod's name, version and description, and each option's note,
  taken from the .mod files: `ModInfo::optKeys` / `optNotes`). Read at startup (options override the .mod file's);
  written when missing (from enabled.txt / isaac.ini, for older installs) or when a mod isn't in it (added, off);
  sections of mods no longer in the folder are kept. `ISAAC_MODS` (tests) ignores it and doesn't write it.
  tools/mod_manager.html no longer opens folders: the built-in mods are put in the page (`tools/embed_mods.py`, run
  by the packaging script), an existing mods.txt can be opened (file input / drop; it also brings other mods'
  sections), and the result is downloaded (or copied) as mods.txt.
- The Gone in the original game: its `onEnterFrame` recomputes `player.mhp` (heart containers) from the character
  every frame, so after a hit took the half soul heart the death test (`!(mhp > 0 or armor > 0)`) never fired and a
  second hit was needed. There his soul hearts are now kept at 0 (`armor` 0) and red at 0.5: the first hit takes
  the half red heart, `hp <= 0`, dead.
- The Gone and "Random": the ring's `rander()` (its Random button and R) picks from `cha.rand` (the unlocked ones);
  it's wrapped so The Gone is one more equal choice (own generator: the game's random numbers are not used).
- Multi-finger gestures (main.cpp): the last finger-down times are kept; 3 / 4 fingers count only when the last 3 /
  4 went down within 0.25 s.
- WideSaac: shading rectangles are scaled just enough for the window's shape (not 8x) and never inside a filter;
  the search looks inside every `_root` child but the room (`a`) and the menu sheet (`men`), so the main menu's
  options sheet (`pauss`) dims the whole screen. With Hyper Zoom the room view starts as soon as `_root` is on its
  game frame (before the HUD shows: the empty, white HUD band flashed at the top as a run started). Boss intros:
  the cut rectangle is a pixel in from each side (some intros have a white line at the stage's very edge) and the
  background inside it is black. Minimap+ has no border.
- Renderer: (1) GPU bitmaps keep a box of where they have content (`g_bmBox`: cleared by a transparent fill,
  grown by uploads - exact for full ones -, copies and draws - the quads `renderScene` emits -); a bitmap is drawn
  only inside its box (the game's full-room layers: blood, gibs, debris, shadows are mostly empty). (2) A shape that
  is one solid axis-aligned rectangle (`plainRectOf`) is drawn as a coloured quad with the white texture, never
  rasterized (fades, dimming, panels; any scale). `ISAAC_NOBMBOX`, `ISAAC_NOPLAINRECT` turn them off (tests);
  `ISAAC_QUADLOG` logs the fill (window areas per frame), `ISAAC_GPUOPLOG` the bitmap operations, `ISAAC_GLFINISH`
  makes headless render times include the GPU's, `ISAAC_SPLATLOG` / `ISAAC_SPLATSKIP` trace the game's splater().
- Switch: `appletInitializeGamePlayRecording` + `appletSetGamePlayRecordingState(true)` as an application when the
  host game supports recording (needed when its setting is "enabled" rather than "automatic").
- BaseTime: game time mm:ss, total time mm:ss:mmm; the menu shows the best game time and the last run's total time
  (mm:ss); `menu_time` is gone. Itemnomicon is back to its dark card (with the quality colour and "QUALITY n").

## v1.5: closing round
- Pause: `menuEscapeResumes()` (menuui.cpp): Escape / back while in a run, paused, with no options sheet open (or it
  on its first frame), sends P (80) - the game's a-script only resumes on P; its own Escape handling showed the
  "back to the main menu?" question.
- "Who Am I?" with a controller (`padNavEvent`): on the menu sheet's `pos` 3 the ring (`cha`, or in the original the
  clip with `i0` and `inc`) turns with left / right (sends 37 / 39); up / down move the focus among the buttons
  outside the ring, sorted by height.
- WideSaac: `fill` defaults to 0 (black bars). The renderer clips to the game area also while the view is easing
  (the background showed for a moment between screens). `stretchShades` recurses to depth 3 and only skips the
  inner `men` (the Delete Save / Exit Game dims live in `_root.men.ex` and `_root.pauss`); a rectangle counts as a
  shade when it covers 85% of the stage. Hyper Zoom adds a view cut from the room's top edge up (`g_viewReq.cut`),
  so the HUD band can't flash white above the room; with a cut the background quad is black. Soul hearts (heart
  clip 609 frames 5+) have a 20%-alpha outline made for the dark HUD band: with Hyper Zoom each gets a black
  silhouette of the red heart under it (`hud.lif.__soulback`, depth -16384, below the timeline children).
- GenesisConsole: while it pauses the game, `recompMusicTick()` (patches.cpp) still runs `_root.soundfra`, so a song
  that ends starts the next one. `unlock` sets indices 1-99 of `_root.locker` / `ulocker` (the same arrays as
  `so.data.lock` / `ulock`; nothing is queued in `lockor`, so no achievement pop-ups) with the write guard off for
  that object; nothing is flushed and `g_saveLocked` stays on, so it lasts only until the game is closed.
- mods/installed.txt is gone: a built-in mod with no file whose id has a section in mods.txt was deleted by the
  player and is not written again. An existing installed.txt is read once: its deleted mods get (empty) sections,
  mods.txt is rewritten and installed.txt removed (`platformRemove`). Deleting mods.txt brings deleted built-in mods
  back.
- .mod files have two more lines for the mod manager: `category` (HUD, Screen, Characters, Cheats; others listed
  after them) and `summary` (one line). The manager groups by category, shows the summary (the full description
  under "More"), shows options only while a mod is on, and draws `key = a-b` options as sliders (step 0.1 when a
  bound has decimals, 5 for spans over 20, else 1).
- StatOn 1.1: `size` (1-5: scale 0.7 / 0.85 / 1 / 1.2 / 1.4, anchored at the top row) and `margin` (1-5: 10 px of the
  stage further from the left edge each step). Itemnomicon 1.3: the line under the name is only "PASSIVE ITEM" /
  "ACTIVE ITEM" and " - QUALITY n" (charges and "FOR SALE" made it too long for narrow cards).

## v1.5: The Gone's unlocks
- Runtime: `MovieClip::reqFrame` keeps the frame number last asked for (gotoFrame); a clip sent past its last frame
  with a number >= 100 is listed in `g_overFrames` (an Array: it keeps them alive until the mod drops them).
  `SnapOv` gained `hideKids` (the node's children are not emitted) and `extra` (a display object emitted as its last
  child: mods draw detached clips, `newDetachedClip()`, that are on no display list). `g_soSaveHook` (builtins.cpp)
  runs around every SharedObject flush, before / after serialising (The Gone takes its secret cards out of lockor).
- mod_thegone_unlocks.inc (included by mod_thegone.cpp). New things get numbers past the game's: items 230 (The Void
  Shard), 231 (Black Mirror; the game's short active number 231 - itn(231) = 191) and 232 (Nothing), card colour 120
  (Negative Isaac), secret cards 150-154. The game's code does the rest: pools (`_root.ittt` treasure, `ittt7` devil:
  added while unlocked and not seen this run), pedestals and prices (devil frame 9: 2 hearts / 3 soul hearts for
  ???), pickup and banner (`st1` / `st2x` names), card drops (`tater` wrapped: 1 in 27), `tart` (the card's name).
  Every clip sent to one of those numbers (past its sprite's last frame) is drawn with the mod's picture (hideKids +
  extra), cached per clip; the HUD's active clip (`hud.it.d`, by short number) the same while `_root.it` is 191.
  Floor cards (inside a pickup, `s` 5) get the cards' back (frame 7) and the pickup's red tint; held or in the HUD,
  the face: a dark red card with the player's head drawing (learnt from `player.d.d.d.d`) in negative.
- Black Mirror: `powerlevel` is wrapped; the game takes 231 as a passive (it isn't in its active list), so after its
  pickup the mod does what the game's active branch does (the held active back on the pedestal, `_root.it` 191,
  `itc` from `_root.iter`); `a.itc[191] = 3` (charges) on each room's copy. Used = `_root.itc` falls from full while
  `_root.it` is 191: 210 ticks of a negative room (snapshot colour transform of `_root.a`) and each enemy pushed
  towards the nearest other, hurting it (`hurt`, 5) every 8 ticks when they touch.
- The Void Shard (`_root.ups[230]`): `playerhurt` wrapped: source 4 (enemy shots) skipped one time in four; after a
  real hit, one in twenty: `hurt(e, 50)` for every enemy (done in the mod's tick, not inside the game's call).
- Nothing (`ups[232]`): 3 ticks into each room, each enemy (`s` >= 10, not `bosser` / `aboss` / `boss` / `minb`, no
  `worm` / `mags` parts) gets `done = true` one time in ten: the game's ball loop removes it with no death (no drop).
- Negative Isaac used (`_root.pilc` goes from 120 to nothing): `a.playsave = 450` (Book of Shadows' shield, 300 in
  the game: it ends with the room as the game's).
- Progress: the floor streak counts a floor as clean until `playerhurt` returns true or a heart is picked up (the
  existing `balljunk` wrapper: floor heart frame 1, shop frames 15 / 1, 5, 13, 17); counted when `_root.chaps` goes
  up or an ending shows (root frames 9-40). Endings of a Gone run (by `_root.chaps` and `altchap`, as BaseTime):
  Mom's Heart (card; Nothing too if `_root.cols` didn't grow and `_root.it` didn't change after the start), Isaac,
  Satan. Unlocks are refused while `g_saveLocked` (GenesisConsole); its `unlock` sets session-only flags.
- Locked: The Gone's portrait darkened with "LOCKED" (the row of another portrait's locked frame), his card with
  "UNLOCKED BY" (Magdalene's locked card row) and three lines in the cards' lettering. Select is blocked by setting
  `_root.skiner` to 99 while he is selected (the game's start buttons only accept known, unlocked characters); Random
  skips him.
- Unlock cards: `_root.unlll = 150 + n`, `_root.unl.gotoAndPlay(2)` when `unl` is idle (frame 1). The card's own
  paper (the sprite clips of its first frame) is placed again, with the heading "YOU UNLOCKED" (the ink on the line
  of the first card's first letter), the name between quotes and a line in the character cards' lettering, and a
  picture. In the menus the cards are put at the front of `_root.lockor` (the found secrets) and taken out of
  every save.
- Collection: on the last page of `men.men.col`, after the game's last slot (the same row step as the last ten), a
  "?" (slot frame 201) or the item; pointing at one sets the large picture (`itt`) and `ittz` after the game's code.
- Itemnomicon describes them (`goneThingInfo`).
- Not changed: Eternal Edition's empty pedestals can be pushed around: its pedestal code (`if (enfcheck(xp, yp, xpp,
  ypp, 30)) { xp = xp * 0.3 + xpp * 0.7; ...; if (levzz(...) < 1) xpp = xp }`) moves a pedestal's home to where it
  was pushed; Wrath of the Lamb and the original always put it back.
- A tear is made at the room's origin and placed on the next tick; a very large one (high damage) showed its shadow
  at the top left for that image. A room child at exactly (0, 0) with `d.sh` is hidden for that image
  (mods.cpp `hideUnplacedTears`, always on).
- WideSaac: the death screen's diary is scaled down with Hyper Zoom; its dimming (a child covering 85% of the stage)
  gets the inverse matrix so it still covers everything. The menus (root frame 5) are always drawn uncut.
- Floor pictures: an item clip inside a pickup (`s` 5) or the player (`s` 1, the held item) is the coloured sprite
  (learnt there: `g_itemSprite`); any other (the collection, the pause screen's list, the death diary: the grey
  drawings, `g_sketchSprite`, learnt from `men.men.col.i0`) gets the grey version: the shard in ink, Black Mirror as My
  Reflection's drawing darkened (no negative). The HUD's Black Mirror uses the coloured sprite, inverted, as on its
  pedestal. Negative Isaac is a playing card: `tater` returns 120 for 1 in 6 of its playing cards (70-74; the
  original game, which has none, 1 in 23 of all), and its floor picture is the playing cards' red back (frame 70).
- Collection: the controller can select the three slots (menuui `menuSetExtras`: a mod's extra options with their
  stage rectangles, added after the topmost-layer filter, not under a dialog; selecting one moves the pointer there
  as for buttons); pointing at one shows it (`itt`, `ittz`).
- Black Mirror: every walking enemy (speed > 0.15) gets its whole velocity turned towards the nearest other enemy
  (at least 1.6), every tick, so they stop going for the player; shots still aim at him.
- The Gone: no D6 / The Dice (the game gives it to `_root.skiner` 0 when `locker[38]`, once per run by
  `_root.wtfuuz`: set at his start, and `_root.it` cleared).
- patches.cpp `charSelectGuard`: Select / Space check `_root.skiner`, which the ring sets on its own frame from
  `cha.skiner`; pressing Select right after moving checked the previous character and started the new one. The pair
  (`cha.skiner`, `_root.skiner`) is recorded when settled (unchanged since the last tick) while `pos` is 3, and put
  back on every tick of `pos` 4 (the start), unless Random chose (`cha.done`). The Gone's mod keeps its own selection
  as it was at that check.
- main.cpp: getTimer() leaves out the time GenesisConsole kept the game paused (`pausedSec`). The game's fade
  (`darky`) moves by the time since its last tick; the original game's clamp is broken (`f5 = 15` instead of `f1`),
  so a long pause pushed `dark` far below 0 and the colour multiplier far above 1.
- menuui: on "Who Am I?" the first selection is Select (the option outside the ring with the largest x + y); up / down
  also reach Random (the ring's lowest button); the first up / down only shows the selection.
- Black Mirror (revised): while it lasts, `aistuff` (the enemies' code) is wrapped and runs with `a.player` set to
  one of the enemies (the target changes every second) - the game's own decoy mechanism (`rply` / `decer`) - so they
  chase and shoot it instead of Isaac; their shots (`s` 9, 5+ ticks old) hurt an enemy they touch (3.5), and every enemy hurts each other one it touches (`hurt` 10, every 15 ticks), like contact with Isaac. Back to
  normal when it ends or the room changes (a new `_root.a`).
- The Gone: `giveit` is wrapped: an item whose only effect is health (15, 16, 22-26, 45, 92) is chosen again from the
  same pool. At his start Isaac's D6 is also taken out of `ups` / `cols` (Replay gave it before the mod's tick).
- Collection: the selected slot is drawn red (as the game's own); the large picture (`itt`) is set to its empty frame
  and drawn by the mod, and its name (`ittz`) is pushed to its text field at once (`updateBoundTexts`: the game's update ran before) (its frames past 199 are other pictures: 230 was a soul heart). Original game: Negative
  Isaac's floor picture is the plain tarot back. GenesisConsole lists "Negative Isaac" among the cards with The Gone on.
