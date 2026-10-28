THE BINDING OF ISAAC — native version (C++ recomp)
==================================================

This is a static recompilation of the Flash game to C++: all of the ActionScript code has
been translated into compiled C++, and Flash Player has been replaced by a custom native
engine (SDL2 + OpenGL 3.3, or OpenGL ES 3.0 on Android). There is no Flash interpreter
and no Ruffle inside.

All three versions of the Flash game available on Steam are supported, each with its own
recompiled code:
  The Binding of Isaac (the original)         495 scripts, 180 functions, 54 buttons
  Wrath of the Lamb                           679 scripts, 225 functions, 62 buttons
  Eternal Edition (the last Flash version)    712 scripts, 245 functions, 62 buttons

WHAT YOU NEED
-------------
- 64-bit Windows (or Linux) with a graphics card that supports OpenGL 3.3.
- The Steam game, in any of its three versions. Its install folder contains two
  executables:
    Isaac.exe             the game itself: it contains all of its data (graphics, sounds,
                          music and code). This is the one you need.
    Binding_of_Isaac.exe  just the Steam launcher; it won't work.
  Graphics, sounds and music are read from Isaac.exe at startup; they aren't included here.
  The program detects which version it is by itself. (An extracted .swf works too.)

INSTALLATION
------------
1. Unzip the folder wherever you like.
2. Copy Isaac.exe from the game's Steam folder next to IsaacRecomp.exe
   (Steam → right-click the game → Manage → Browse local files),
   or set its path in isaac.ini (swf = C:\path\to\Isaac.exe). You can also copy the
   contents of this folder into the Steam folder, next to Isaac.exe.
   An extracted .swf still works.
3. Run IsaacRecomp.exe (on Linux: ./isaac).

SEVERAL VERSIONS AT ONCE
------------------------
Since all three are called Isaac.exe on Steam, copy them into the program's folder under
different names, for example:
    Isaac-Original.exe   Isaac-WotL.exe   Isaac-Eternal.exe
(the name doesn't matter: each version is recognized by its contents). If there is more
than one, a selection screen appears at startup (arrow keys or mouse, Enter to play; with
a controller: D-pad and A). Your choice is remembered (isaac.ini: game = og, wotl or
eternal) and preselected next time. Each version has its own save file.

WHAT'S DIFFERENT FROM THE FLASH VERSION
---------------------------------------
- Native 1080p: vector graphics are rasterized at your screen's real resolution
  (1440p/4K too). The original 4:3 aspect ratio is kept, with bars on the sides.
- 60 fps (or your monitor's refresh rate: 120, 144...): the game logic still runs at
  exactly 30 Hz, so speed, physics, fire rates and enemy patterns are the original ones.
  Each frame is drawn by interpolating between the last two logic ticks: movement,
  scaling, rotation, fades, color changes, morphs and the timeline's tweened animations
  (including room transitions) all play at the refresh rate. In exchange, the image runs
  on average about 20 ms behind the logic (half a tick); set smooth = 0 to go back to the
  original 30 fps.
- The game logic runs on its own thread and drawing on another, with its own clock synced
  to the monitor's refresh. A heavy tick (loading a room, an explosion with lots of
  objects) no longer delays frames.
- Performance: in testing, the game logic took on average less than 1.5 ms per tick (out
  of the 33 ms available) on a modest virtual machine, even with more than 20 enemies on
  screen. New graphics are rasterized in parallel on all cores and placed in a texture
  atlas created at startup.

CONTROLS
--------
Same as the original: WASD to move, arrow keys to shoot, E or Shift for bombs, Space for
the active item, Q for card/pill, P or Esc to pause.
- F11, Alt+Enter or F (during play): toggle fullscreen / window. Also available from
  Options; remembered for next time (isaac.ini).
- F12: screenshot (screenshot_N.bmp in the save folder).
- F3: performance diagnostics (see below).
Controller (XInput / SDL): left stick or D-pad = move; right stick or A/B/X/Y = shoot;
LB or left trigger = bomb; RB or right trigger = active item; right stick click = card/
pill (Q); Start, Back or left stick click = pause. In menus and in the pause screen the
selected option is highlighted in pink: D-pad or left stick = choose, A (cross on PlayStation
controllers) = press it, B (circle) = go back / close the pause screen, Start = Enter.
REBINDING: Options > "Controls" > REBIND CONTROLS (also from the pause screen's Options)
opens a screen to change the controls of the device you are using - keyboard, controller
or touch screen - with your device's own buttons. Pick an action and press the new key or
button (it swaps places with whatever had it); RESET TO DEFAULTS restores the original
layout. Controller: you can also choose which stick moves. Touch: which side moves, the
joystick size and its type (type 1: the base stays where the finger landed; type 2: the
base follows the finger). Saved in isaac.ini (bind_keyboard, bind_controller,
controller_move_stick, touch_move_side, touch_joystick_size, touch_joystick_type). While a device uses custom
controls, the game's "Lefty Flip" and "Azerty" options no longer change it.

MOVING SAVES BETWEEN THE FLASH GAME AND THIS VERSION
---------------------------------------------------
Open tools/save_converter.html (included in this package) in any web browser. It turns the
Flash game's so.sol into IsaacRecomp's save file and back, for all three versions,
without uploading anything. Close the game before replacing a save and keep a copy of
the old one. (Converter originally made by g333nza.)

MODS
----
Mods are small optional changes, kept as .mod files in the "mods" folder next to the
program (Android: IsaacRecomp/mods; Switch: sd:/switch/isaac/mods; the program puts the
mods that come with it there the first time). They are all off until you turn them on.
Which mods are on, and their options, are in one plain text file: mods/mods.txt (the game
writes it the first time it starts). Change it with any text editor: one [id] section
per mod, "on = 1" (on) or "on = 0" (off), and the mod's options under it, each explained
on the line above. Or open tools/mod_manager.html in a web browser (offline, nothing to
install): flip the switches, change the options and save mods.txt, then put it in the
mods folder in place of the old one (open your current mods.txt there first to start
from it). Mods are read when the game starts; log.txt says which ones are on.
Included:
  StatOn (staton)     shows your stats on the left of the screen during a run, each with
                      its symbol: speed, tears, damage, range, shot speed and luck (luck
                      only in Wrath of the Lamb and Eternal Edition, the original game
                      has none). When one changes, the difference appears next to it for
                      a couple of seconds, green up and red down, as in Repentance.
                      Below them, the chance that this floor's boss room opens the devil
                      room and the angel room (the game's own formula: no damage taken,
                      Pentagram, Book of Belial, earlier devil rooms...).
                      Options in StatOn.mod (or in the mod manager): show_changes = 0
                      hides the differences, show_deals = 0 the devil / angel chances,
                      opacity = 10-100. It only reads the game: runs play exactly the
                      same with it on.
  GenesisConsole      a console to type commands during a run, like Rebirth's: give
  (genesis)           items, spawn enemies, bosses, pickups, trinkets and cards, change
                      your stats, hearts and pickups, god mode, open every door, go to
                      any room or floor... Open it with the key above Tab, the
                      controller's Back / Minus button or a three-finger tap; the game
                      pauses while it is open. Type "help" for the commands; all of
                      them are explained in tools/genesis_wiki.html.
                      WHILE IT IS ON, NOTHING IS SAVED: no achievements, unlocks,
                      statistics or collection (your save is left as it was).
  WideSaac (widesaac) the whole screen on wide screens (16:9, 16:10, 21:9...) and any
                      other shape: no black bars, the game's own background fills the
                      sides, and the menus fill the whole screen (more of the menu's
                      own sheet is shown, nothing is stretched); screen-wide shading
                      (the pause screen, fades) covers the whole screen too. With Hyper Zoom (option hyper_zoom, a tick box in the mod
                      manager) the room itself fills the screen while you play: the
                      HUD band and the map go, and your hearts (top right), active
                      item with its charges, coins / bombs / keys and pill or card are
                      shown on the room's right side. It stays like that the whole
                      run, also paused and on the death screen; a boss's "VS" intro,
                      the cutscenes between floors and the endings are shown in the
                      room's space, black on their sides: the zoom never changes. With Hyper Zoom, M (L3 on a controller, a four-finger tap
                      on a touch screen) shows or hides Minimap+: the floor's map,
                      larger and see-through, in the middle of the room (option
                      minimap = 0: no map at all). It only changes what is drawn: runs
                      play the same.
  PillLog (pilllog)   pills you have already taken this run are known by sight: their
                      effect is written over every pill of that colour in the room or
                      for sale (green good, red bad), the banner says the effect when
                      you pick one up instead of "Pills here!", and the pause screen
                      lists the six pill colours of the run with what each does ("???"
                      until taken). The names are the ones the game itself shows next
                      to a pill you hold. Options: labels, banner, pause_list (1/0).
  Itemnomicon        walk up to an item, a trinket or a card you can pick up or buy
  (itemnomicon)      and a card over it says what it is: its name (in the colour of its
                      quality, as Rebirth rates the same items: grey 0, white 1, blue
                      2, violet 3, gold 4), passive or active (with its charges), the
                      stats it changes (green up, red down) and what it does. The stat changes are the
                      game's own, measured in each version (for Isaac with nothing
                      else: some items stack differently). Options: range = 0.5-4
                      tiles (1: next to it), opacity = 30-100. Both only read the
                      game: runs play the same with them on.
  BaseTime (basetime) speedrun timers, left of the map (with Hyper Zoom, at the room's
                      bottom left): the game time (mm:ss, while you can move: it stops
                      while paused, in cutscenes and while loading) and below it the
                      total time (mm:ss:mmm, everything since the run started). The
                      main menu shows, on a note of its own, your best game time - the
                      fastest run to your furthest ending: Mom, then Mom's Heart, then
                      Isaac or Satan, then The Chest - and below it your last run's
                      total time, with "win" (it reached an ending) or "died". Kept in
                      saves/basetime.txt. It only reads the game.
  The Gone (thegone)  a new character, like Rebirth's The Lost: on the character select
                      screen, one more place after the last character, with his own
                      card like the others' (name, stats, starting item). A pale Isaac
                      (3.5 damage, Isaac's speed)
                      who starts with Spirit of the Night (the original game, which
                      doesn't have it: Transcendence and the Ouija Board) and has no
                      hearts: he can't pick up hearts, gain heart containers or extra
                      lives, and one hit kills him. In a devil room he can take one
                      deal for free; then the other deals disappear. "Random" can
                      pick him too.
  RainbowVision       colours for colour-blind players: the whole picture is
  (rainbowvision)     recoloured so colours you can't tell apart look different
                      (hearts, pills, enemies). Option type = deuteranopia (green-
                      blind, the most common), protanopia (red-blind) or tritanopia
                      (blue-blind); strength = 0-100. Only the colours on screen change.

THE isaac.ini FILE
------------------
Created automatically next to the executable the first time you run it.
  fullscreen = 1      fullscreen (borderless)
  vsync = 1           vertical sync
  width/height        window size in windowed mode
  volume = 100        master volume (0-100)
  smooth = 1          interpolation at the refresh rate (0 = original 30 fps)
  filters = 1         blur effects in cutscenes
  cache_mb = 768      maximum video memory for rasterized textures
  gpu_sync = 1        waits for each refresh before preparing the next frame
                      (steadier pacing; set it to 0 if things feel worse)
  low_latency = 1     keeps reading keyboard/controller input while waiting for the
                      refresh, so a press reaches the game up to ~8 ms sooner (0 = off)
  interp_lead = 0     experimental, 0 to 0.5: draws that fraction of a tick ahead
                      (less visual delay, but movement can overshoot slightly when you
                      stop or turn). The game logic is never affected.
  gc_after_tick = 1   frees memory after each game tick has been sent to the screen, so
                      it doesn't delay the image (0 = inside the tick, as in 1.3)
  max_margin = 0      how far behind the game the image may be shown, in ms. It follows
                      how long the game's ticks take (smooth motion); a cap keeps the
                      delay constant, late ticks being extrapolated. 0 = automatic
                      (4-30 ms); 6 on Switch
  game = wotl         last version played (og, wotl or eternal; the old key
                      "juego" is still accepted)
  performance = 0     1 = performance diagnostics always on (same as F3, see below)
  touch_controls = 1  (Android, Switch) 0 turns off the touch joysticks, HUD taps and the
                      pause button (touch then only works in menus)
  render_height = 0   (Android only) rendering resolution, 0 = automatic (see ANDROID)
  joystick_acceleration = 1
                      analog movement: with a stick (touch, controller, Switch) the
                      speed depends on how far you push it, from 40% just past the
                      dead zone to full speed at 3/4 of the way (never faster than the
                      original). 0 = always full speed, as in the original game
  mods =              the mods that are on, used only until mods/mods.txt exists (see MODS)

IF YOU NOTICE STUTTERING OR SLOWDOWNS: F3
-----------------------------------------
Press F3 during play. The following appear in the top-left corner, updated every second:
  FPS (HZ)              frames per second and monitor refresh rate
  MAX FRAME             the slowest frame of the last second
  HITCHES               frames that took longer than 1.5 refreshes
  HELD                  frames where the logic hadn't finished its tick yet
  LOGIC / MARGIN        logic time per tick (30 per second) and the margin the renderer
                        leaves it to finish (adjusts itself)
  RENDER                CPU time spent preparing the frame
  GPU/VSYNC WAIT        time spent waiting for the graphics card / the refresh
                        (with vsync it's normal for this to be high: it's idle time)
  RASTER / UPLOAD       new graphics rasterized and MB sent to the GPU
  NEW TEX / GC          textures created (should be 0 while playing) and garbage collector
While F3 is on, "performance.txt" is also written to the save folder (one line per second
and one line per stutter, with its breakdown). If something runs slowly, play the
problem room for a while with F3 on and send that file: it shows exactly which part is
taking the time.

ANDROID
-------
Needs a 64-bit ARM phone or tablet (practically all of them since 2017) with Android 7.0
or newer and OpenGL ES 3.0.
1. Install IsaacRecomp.apk (allow "install unknown apps" for your browser or file
   manager when Android asks).
2. Create a folder called IsaacRecomp at the root of the internal storage (next to
   Download, DCIM...) and copy Isaac.exe from the game's Steam folder into it. Several
   versions work as on PC (different names, a menu to choose).
3. Open Isaac Recomp. The first time it asks for "All files access" (Android 11 and
   newer) or for access to your files: the game, isaac.ini, the saves and log.txt all
   live in that folder.
Touch controls: put a finger on the left half of the screen to move and on the right
half to shoot; a joystick appears where the finger lands, stays there while you slide
it, and disappears when you lift it (how far you push the left one sets your speed, see
joystick_acceleration). Tap the bomb counter to drop a bomb, the "SPACE"
box to use the active item and the pill/card (top right, when you have one) to use it.
The pause button sits right of "-LIFE-"; the back button/gesture also pauses (and goes
back in menus). In menus, tap the options; on screens without options (cutscenes,
title cards) a tap continues. While the game is still loading its graphics (the first
seconds, during the opening cutscene) touches are ignored; touching shows the progress.
Bluetooth or USB controllers work as on PC (see CONTROLS); the joysticks only appear
when you touch the screen.
Performance diagnostics: set performance = 1 in IsaacRecomp/isaac.ini; the overlay
appears and IsaacRecomp/saves/performance.txt is written.
The game runs at 960x720 (like on Switch), scaled to the screen with black bars. To keep
60 fps it renders at 720 lines and, if the device still can't keep up during play, goes
down to 600, 540 or 480 by itself (remembered in isaac.ini as auto_height). To choose
yourself, set render_height in isaac.ini (e.g. 1080 for the sharpest image, 480 for the
fastest); 0 = automatic.

SAVE DATA
---------
The game is saved when you quit (and whenever the game asks for it) in:
  Windows / Linux: the "saves" folder next to the program (next to "tools")
  Android: IsaacRecomp/saves/ in the internal storage (up to 1.3.1: IsaacRecomp/, copied
           over the first time 1.4.0 starts)
  Switch:  sd:/switch/isaac/saves/ (next to "tools"; up to 1.3.1: sd:/switch/isaac/, copied
           over the first time 1.4.0 starts)
with one file per version: isaac_og_save.sol (original), isaac_save.sol (Wrath of the
Lamb) and isaac_eternal_save.sol (Eternal Edition).
Up to version 1.3.0 the PC versions saved in %APPDATA%\IsaacRecomp\WrathOfTheLamb\
(Windows) or ~/.local/share/IsaacRecomp/WrathOfTheLamb/ (Linux): the first time 1.3.1
starts, the saves found there are copied to the "saves" folder (the old ones are kept).
The same folder also holds preload.txt / preload_og.txt / preload_eternal.txt
(graphics learned on this machine), screenshots and performance.txt.
Saves from the Flash game can be converted with tools/save_converter.html (see above).

CHANGES IN VERSION 15 (v1.5.0 public release numbering)
--------------------------------------------------------
- Mods: a "mods" folder next to the program, a mod manager (tools/mod_manager.html) to
  turn them on and off and change their options, and the first eight mods (see MODS), off
  unless turned on:
  - StatOn: your stats on the left of the screen during a run, with their symbols, as in
    Repentance, and the chances of a devil room and an angel room on this floor.
  - GenesisConsole: a console to give items, spawn anything, change your stats, god mode,
    teleport... (tools/genesis_wiki.html explains it all). Nothing is saved while it is on.
  - WideSaac: the whole screen on wide screens, and Hyper Zoom: the room fills the screen
    while you play, with the HUD on its right side and Minimap+ on M / L3 / four fingers.
  - PillLog: the effect of the pills you already know, written over them, in the pickup
    banner and in a list on the pause screen.
  - Itemnomicon: over items, trinkets and cards near you, their quality, stats and
    what they do.
  - BaseTime: speedrun timers (game time and total time), and on the menu your best time
    to your furthest ending and your last run.
  - The Gone: a new character, in the spirit of The Lost: no hearts, one hit and he dies.
  - RainbowVision: colours for colour-blind players (protanopia, deuteranopia, tritanopia).
- Touch screens: GenesisConsole's three-finger tap and Minimap+'s four-finger tap count
  only when the fingers touch the screen together, not when you tap while holding a stick.
- Faster drawing (all games, above all on Switch): the game's big see-through layers
  (blood, debris, shadows) are drawn only where they have something, and plain
  rectangles (fades, dimming) are drawn directly instead of as pictures.
- Switch: video capture (holding the Capture button) is asked for, for host games whose
  setting needs it.
- A mod that comes with the program is updated when a newer version of it comes with a
  newer program; your options are kept (except one whose meaning changed).
- Boss music: after a boss that splits when it dies (Duke of Flies...), the boss theme
  started again and no "boss defeated" music played when the last fly died. The room
  must now stay empty for a moment before the music changes.
- Less input lag and smoother motion: each game tick is now timed to arrive just before
  the screen draws a new image (60 / 120 Hz screens), so what you press shows up as soon
  as possible and no image has to guess where things are (fewer "HELD" frames in
  performance.txt).
- Texts that change all the time (BaseTime's timers) no longer make the game stutter, above
  all on Switch: a text that was already drawn is reused instead of drawn again.
- mods/mods.txt: which mods are on and their options, in one text file the game writes
  and reads (tools/mod_manager.html makes it too, without opening any folder).
- Slightly lower memory use and faster variable lookups in the game's code (on every
  platform; runs play exactly the same).

CHANGES IN VERSION 14 (v1.4.0 public release numbering)
--------------------------------------------------------
- Gameplay checked against the original: every enemy, boss, item, pill, card and trinket
  of the three games was played side by side with the original Flash game (in the Ruffle
  emulator, same input and same random numbers) and compared frame by frame. Fixed:
  - Mom's Knife drifted from the original over time (rotations and scales are now kept
    exactly as the game sets them).
  - When a boss died, the poops in the room broke in a different order than in the
    original (the order the game goes through lists in "for ... in" loops).
- The Hourglass behaves as in the original again: slowed enemies fire twice per attack
  and the Widow spawns twice the spiders (version 12 had changed it, but that is how the
  original game plays).
- The game's timer follows the real clock, as in Flash: the random numbers the game
  shuffles at the start of each floor vary as much as in the original.
- Switch and Android: the game's calculations give exactly the same results as on PC.
- Switch: saves, preload*.txt and performance.txt now go in sd:/switch/isaac/saves/, and
  the save converter comes in sd:/switch/isaac/tools/, as on PC. The saves in
  sd:/switch/isaac/ are copied there the first time (the old files are kept).
- Android: the same, in IsaacRecomp/saves/ (copied from IsaacRecomp/ the first time).
- The program has the same icon as the Switch and Android versions (Windows: the .exe and
  its window; Linux: the window).
- Save converter (tools/save_converter.html): shows the current save folders.
- Sound: the game no longer waits for the sound mixer (smoother busy rooms, especially on
  Switch); the music is no longer the first sound cut when too many play at once; the
  chest and Satan sounds of the endings play at their original (lower) volume; Switch
  mixes at the console's own 48 kHz.
- Lower input delay in busy rooms (most noticeable on Switch). Two new isaac.ini settings,
  added to an existing isaac.ini automatically: gc_after_tick (memory is freed after each
  game tick is sent to the screen) and max_margin (how far behind the game the image may
  be shown; 6 ms on Switch, automatic elsewhere; 0 = automatic as in 1.3).

CHANGES IN VERSION 13 (v1.3.1 public release numbering)
--------------------------------------------------------
- PC: saves now go in a "saves" folder next to the program. Older saves are copied there
  automatically the first time. Folders whose names have accents, Cyrillic or other
  non-English letters (Windows user names in other languages) no longer stop the game
  from loading or writing its save.
- Controllers in menus: the selected option is pastel pink again (the same tone
  everywhere); only the main menu's Start keeps the deep red.
- Controllers: moving through the collection's grid goes to the item next to the
  selected one, as expected; the first item is selected when it opens.
- Controllers: the "Are you sure you want me to die?" note (Exit Game, in the main menu
  and in the pause screen) can be answered with the controller (No is selected first);
  the options under it can no longer be selected.
- Controllers: smoother menu movement. Each press goes to the next option in that
  direction (Options: Fullscreen > Music > Controls; Controls: Back > Rebind Controls),
  B / circle goes back one screen (Controls > Options > main menu) like the Back buttons,
  and the selection no longer flickers or slides away while the screen moves (returning
  to the main menu, Stats). The pause screen's collected items are not selectable.
- PC: starting the game with a controller connected no longer sends the sound to the
  controller (Windows makes it the default output): the speakers or headphones are used.
- Touch (Android, Switch): tapping the bomb counter places a bomb in the original game
  and Wrath of the Lamb too (it only worked in Eternal Edition).

CHANGES IN VERSION 12 (v1.3.0 public release numbering)
------------------------------------------------------
- Controllers in menus: the selected option is written in red (the game's own rollover
  art is shown too); the D-pad/left stick chooses the nearest option in that direction,
  A (cross) presses it and B (circle) goes back or closes the pause screen. Works in every
  menu of the three games, on PC, Android and Switch, including the challenges list and
  the items of the collection. Each screen opens on its first option (Start in the main
  menu, Options in the pause screen, Fullscreen in Options, Lefty Flip in Controls).
- Mouse: the pointer becomes a hand over anything that can be clicked, as in the original.
- PC: connecting a controller with its own audio (DualSense, DualShock 4, Xbox headset)
  no longer takes the game's sound with it: the game keeps the output device it started
  with, and follows the system when you switch to other speakers or headphones.
- New: control rebinding (Options > Controls > REBIND CONTROLS) for keyboard,
  controller and touch screen, drawn inside the game. See CONTROLS.
- New: save converter between the Flash game and this version, both directions, all
  three versions (tools/save_converter.html, based on the one made by g333nza).
- Touch: new joystick type 2 (the base follows the finger), in the rebinding screen.
- Touch: a finger kept on a joystick through a pause or the boss "VS" screen keeps
  working when play resumes (Isaac used to stand still at the start of boss fights until
  the finger was lifted).
- The Hourglass (and other slowing effects): slowed bosses no longer fire twice per
  attack, and the Widow no longer spawns twice the spiders.
- Credits (main menu > Credits > Credits): a tap no longer skips the credits as soon as
  they start (they returned to the menu at once, restarting the music).
- Hearts: with more than 12 hearts the -LIFE- display now switches to its smaller
  24/48-heart layout as in the original, instead of the first hearts staying big and
  overlapping the rest.
- Tears no longer appear for an instant below Isaac's head and then jump up to it.
- Controllers: holding the left stick or the D-pad while the game switched to a menu
  (the boss "VS" screen, pause) could leave an arrow key pressed afterwards, so Isaac
  kept shooting by himself and the shooting controls behaved oddly. Inputs from the
  keyboard, touch and each part of the controller no longer interfere with each other.
- Android: fixed choppy, "motor-like" sound on some devices (it went away while
  recording the screen): most of every block of sound was being dropped.
- Android: changing the audio output (headphones, Bluetooth) could crash and restart the
  game; the sound now recovers instead.
- Android: the bomb counter, active item, card/pill and pause buttons are easier to hit
  on small and wide screens (the bomb row was only a couple of millimetres tall).

CHANGES IN VERSION 11 (v1.2.0 public release numbering)
-------------------------------------------------------
- New: Android version (see ANDROID): touch controls with two floating joysticks and
  taps on the HUD, Bluetooth controllers, OpenGL ES renderer.
- Controllers: menus and the pause screen can now be used with the controller alone
  (A confirms, B goes back, D-pad/left stick choose), as on Switch. Start/Back pause.
- New isaac.ini option: performance = 1 turns the performance diagnostics on at startup.
- Analog movement (joystick_acceleration = 1, on by default): with a stick (touch,
  controller, Switch) the speed follows how far you push it, like in Rebirth. Full
  speed is reached at 3/4 of the stick's travel and is never above the original.
  Keyboard and D-pad are unchanged.
- Switch: in handheld mode the touch screen now works during play too: the same
  floating joysticks and HUD taps as on Android, plus a pause button next to -LIFE-
  (see README_SWITCH.txt). The .nro shows version 1.2.0.
- Audio: sound is now mixed directly at the output device's sample rate with smoother
  (cubic) interpolation, instead of being resampled twice. Cleaner high frequencies.

CHANGES IN VERSION 10 (v1.1.0 public release numbering)
-------------------------------------------------
- Fixed a slowdown that built up during long sessions (and could end in a crash on
  Switch): every sound the game played left an invisible clip behind, because
  Sound.position returned 0 instead of the sound's length once it had finished. After
  20-30 minutes there were thousands of them. All three versions were affected.
- Sound.duration now uses each sound's real sample rate.
- The learned preload list no longer grows without limit, and preloading never takes
  more than ~60% of the texture atlas.
- Eternal Edition: the hard-mode mark no longer shows on the HUD when hard mode is off.
- Fixed damage from blood/creep puddles: the game checked the floor at the wrong
  spot (1.5x-2.25x further from the top-left corner than the player), so puddles far
  away could hurt you and the ones under you sometimes didn't. All three versions.
- Fewer hitches in long sessions: the game now produces ~60% less memory garbage, and
  cleaning it up can no longer make a game tick miss its deadline.
- Lower input latency: input is read continuously while waiting for the display
  (low_latency), plus the experimental interp_lead option.

CHANGES IN VERSION 9
--------------------
- Besides Wrath of the Lamb, the original game and Eternal Edition can now be played (see
  "SEVERAL VERSIONS AT ONCE"). Everything from previous versions (60 fps, 1080p,
  preloading, simplified options menu, controllers...) works the same in all three.
- File names and settings are now in English: log.txt, performance.txt, preload*.txt,
  screenshot_N.bmp and "game =" in isaac.ini (old files and the old key still work).

CHANGES IN VERSION 8
--------------------
- Game data is read directly from Steam's Isaac.exe: there's no need to extract the .swf
  anymore. The Windows program has been renamed IsaacRecomp.exe so it isn't confused
  with it.
- No more loading screen: graphics are preloaded in the background while the intro
  cutscene plays. It can't be skipped until preloading finishes (a few seconds).
- "Delete Save" (in Collection) really deletes the save: the game restarts itself so
  that stats, collection and unlocks go back to zero.
- In windowed mode, the window fits the usable area of the screen (without being covered
  by the taskbar) and its title bar is visible.
- The thin white seams between colored areas of the same drawing are gone (they showed up
  in boss backgrounds, floor transitions...): shapes are rasterized the way Flash does it,
  without the background showing through the joins.

CHANGES IN VERSION 7
--------------------
- Preloading: at startup the game's graphics (HUD, cutscenes, menus, enemies...) are
  rasterized in advance, so gaps or blurry versions no longer appear the first time
  something is shown. The list is included and fills itself in with whatever you see
  (preload.txt in the save folder).
- Room transitions: everything the game draws into its layers (each room's floor and
  decoration, blood, shadows, the minimap and the screen capture that slides when you
  change rooms) is now drawn by the graphics card. Going through a door used to cost the
  logic two ticks of ~35 ms on PC (and hundreds of ms on Switch); now it's under 3 ms.
- If a tick takes too long, the logic carries on from that point instead of catching up
  by running several ticks in a row (which skipped the transition animation).
- Blurs: computed at reduced resolution when they're large, and reused if their content
  doesn't change (the main menu has a dozen of them).
- Streaming music and sounds: fixed the playback position calculation.

CHANGES IN VERSION 6
--------------------
- New graphics (when entering a room, enemies spawning) are rasterized in the background:
  a frame waits a few ms at most and, if something isn't ready, it appears on the next
  frame instead of freezing the image.
- Each frame uploads its vertices in one go to a ring buffer and replays its draw calls
  at the end (previously a fresh 16 MB buffer was allocated every frame and dozens of
  small uploads were made). The F3 overlay is drawn as a single image and shows the frame
  breakdown (adopt, build, draw); performance.txt is written from another thread.
- The garbage collector frees dead objects based on time (a few ms per tick) instead of
  count: the periodic logic hitches are gone.

CHANGES IN VERSION 5
--------------------
- New: Nintendo Switch version (homebrew), in a separate package with its own
  README_SWITCH.txt. The save file is the same on both platforms.
- Fixed a memory leak: every room visited was kept in memory (with its 6.7 MB layers)
  because the game clears its mouse listeners in a way the engine didn't understand.
  After a long run the game could exceed 1 GB; now it stays stable.
- Hit areas of vector graphics are only computed when the game uses them: about 95 MB
  less memory and slightly faster startup.

CHANGES IN VERSION 4
--------------------
- "Exit Game" in the main menu now closes the game (saving first). The Steam version did this
  through its wrapper (Zinc), which doesn't exist here.
- Cleaned-up options menu: "Detail", "FullScreen Resolutions" and "Window size" have been
  removed, since they depended on Flash/Zinc and did nothing; "FullScreen" now actually
  works. On the controls screen, "Game pad?: use JoyToKey (google it!)" has been removed:
  controllers work out of the box.

CHANGES IN VERSION 3
--------------------
- New rendering engine: logic and drawing run on separate threads, communicating through
  scene snapshots (one per tick). The renderer interpolates between the last two at the
  monitor's refresh rate, with a presentation clock that advances exactly one refresh per
  frame, so motion advances in even steps.
  Previously only movement made by the game code was smoothed; now room transitions,
  fades, color flashes, rotations and scaling, and the timeline's tweened animations also
  run at 60 fps.
- Fixed the ~30 ms stutters (about 3 per second) that the diagnostics recorded every time
  a new graphic appeared: creating textures mid-game stalled the video driver. Now
  graphics go into an atlas created at startup, pixels are uploaded through an
  asynchronous buffer (PBO) without waiting, each room's layers reuse the previous room's
  textures, and menu blurs reuse their intermediate textures.

CHANGES IN VERSION 2
--------------------
- Fixed: Isaac's sprite (and those of other characters) would sometimes "come apart" and
  appear shifted downward for an instant while walking. The 60 fps smoothing was also
  extrapolating the jumps in timeline animations (body parts change position from one
  keyframe to the next); now only movement made by the game code is extrapolated, and
  only if it's consistent across ticks.
- Performance with many enemies and when changing rooms:
  * every blood splat or debris sent the whole floor layer (6.7 MB) to the graphics card;
    now only the small piece that changes is sent;
  * the garbage collector paused for ~100 ms every minute or two (more often the more
    enemies there were); now pauses are 1-2 ms;
  * each room's layers (6.7 MB each, in memory and on the GPU) weren't freed until that
    collection and piled up room after room; now they're recycled;
  * the vector rasterizer was up to 3-4 times slower than necessary with complex shapes,
    and ran on a single core; room backgrounds that have already been seen are reused.
    Entering a room cost 70-260 ms in one go; now 20-50 ms the first time and 10-25 ms
    when coming back (measured on a virtual machine with only 2 cores; considerably less
    on a normal PC);
  * vertices are sent to the GPU without sync waits;
  * cutscene blurs reuse their intermediate textures and also look right on 16:9 screens.

KNOWN LIMITATIONS
-----------------
- Frame-by-frame animations (where each frame is a different drawing, e.g. Isaac's legs
  while walking or some particles) have 30 drawings per second in the original, and
  that's how they look: there are no in-between drawings to show. Everything that moves,
  rotates, scales or changes color does run at the refresh rate.
- Tested with automated runs (thousands of frames with random input, menus, cutscenes,
  rooms, items and combat), but not in every possible situation. If you find something
  odd, note down when it happens.

Third-party software: SDL2 (zlib), PlutoVG (MIT), dr_mp3 (public domain / MIT-0),
stb_image (public domain / MIT). See THIRD-PARTY-LICENSES.txt.
