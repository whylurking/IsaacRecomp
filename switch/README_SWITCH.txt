THE BINDING OF ISAAC — native port for Nintendo Switch (homebrew) — version 1.5.0
=================================================================================

All three versions of the Flash game on Steam are supported: the original, Wrath of the
Lamb and Eternal Edition (each with its own recompiled code).

This is the same native recomp as on PC (ActionScript translated to C++ plus a custom
engine) built for Switch as a homebrew application (.nro). There is no Flash emulator and
no Ruffle inside.

WHAT YOU NEED
-------------
- A Switch running custom firmware (Atmosphère) with the Homebrew Menu (hbmenu).
- The Steam game (any of the three versions): the Isaac.exe file from its install folder
  (Steam → right-click the game → Manage → Browse local files). It contains all of the
  game's data; graphics, sounds and music are read from it and are not included here.
  The other executable, Binding_of_Isaac.exe, is just the Steam launcher: it won't work.

INSTALLATION
------------
1. Copy the "switch" folder from this zip to the root of the SD card (it merges with the
   one you already have). You should end up with:  sd:/switch/isaac/isaac.nro
2. Copy Isaac.exe from Steam into that same folder:
      sd:/switch/isaac/Isaac.exe
   (an extracted .swf works too).
   To have several versions, copy them under different names, for example
      Isaac-Original.exe   Isaac-WotL.exe   Isaac-Eternal.exe
   The program recognizes each version by its contents. If there is more than one, a
   selection screen appears at startup (D-pad or stick and A, or tap the screen); your
   choice is preselected next time. Each version has its own save file.
3. Open the Homebrew Menu by HOLDING R while launching any game (title takeover /
   "application" mode) and choose "The Binding of Isaac".

   Important: do NOT open it from the Album. In that mode (applet) homebrew only gets
   about 400 MB, and the game needs around 250 MB of RAM plus GPU memory, so it could run
   out of memory or stutter. In application mode it has more than 3 GB available.

DISPLAY AND PERFORMANCE
-----------------------
- Docked: 1080p. Handheld: 720p. Vector graphics are rasterized at the screen's real
  resolution and re-rasterized when you dock or undock the console.
- 60 fps with interpolation, same as on PC: the logic runs at exactly 30 Hz on its own
  core (core 1, by itself) and the renderer interpolates between ticks on core 0. New
  graphics are rasterized in the background (core 2): if one isn't ready in time, it
  appears one frame later instead of freezing the image.
- Graphics are preloaded in the background during the intro cutscene (it can't be
  skipped until preloading finishes, a few seconds), so no gaps or blurry versions appear
  the first time something is shown. If you dock the console during play, graphics are
  re-rasterized at 1080p as they're needed.
- Latency: the GPU never has more than one frame queued behind the one being shown, and
  the controller keeps being read while the console waits for the GPU, so a button press
  reaches the game without waiting for the next frame (isaac.ini: low_latency = 1). If it
  ever feels less smooth, try low_latency = 0. For even less visual delay you can try
  interp_lead = 0.25 (experimental, up to 0.5): the image is drawn slightly ahead, at the
  cost of a tiny overshoot when you stop or turn. The game logic is never affected.
- This version uses a 960x720 internal stage (1440x1080 on PC) to reduce the memory used
  by the game's bitmaps: on Switch it looks just as sharp in handheld mode and very close
  when docked.

CONTROLS
--------
During play (they adapt automatically depending on whether you're playing, paused or in
a menu):
  Left stick / D-pad          move
  Right stick                 shoot in that direction
  A / B / X / Y               shoot right / down / up / left
  L                           bomb
  ZL                          active item (the one with charges)
  R                           consumable (card / pill)
  ZR (hold for 5 seconds)     restart the current run
  + or –                      pause
In menus and the pause screen:
  Left stick / D-pad          navigate the menu
  A                           confirm
  B or –                      back
  +                           Enter in menus; on the pause screen, resume the game
                              (– also resumes from pause)
  Touch screen (handheld)     tap menu and pause buttons (like the mouse on PC); on
                              screens without buttons (cutscenes) a tap continues.
Menus with the controller: the selected option is written in red; D-pad or left stick
chooses another, A presses it, B goes back (on the pause screen: resumes the game).
Rebinding: Options > "Controls" > REBIND CONTROLS lets you change which button does what
(and which stick moves), and the touch joysticks' side and size. Saved in isaac.ini.
Touch screen during play (handheld mode, as on Android):
  Finger on the left half     a joystick appears under it: move (how far you push it
                              sets your speed, see joystick_acceleration)
  Finger on the right half    a second joystick: shoot
  Bomb counter / SPACE box    drop a bomb / use the active item
  Pill or card (top right)    use it
  Button right of -LIFE-      pause (only shown in handheld mode)
  The joysticks and buttons can be mixed freely with the controller. To turn the touch
  controls off: touch_controls = 0 in isaac.ini.
Analog movement: the left stick's speed follows how far you push it (full speed at 3/4
of the way, never faster than the original). joystick_acceleration = 0 in isaac.ini
restores the original all-or-nothing movement.
Diagnostics: press both sticks at once (L3 + R3) to show or hide performance data (like
F3 on PC). While it's on, the data is saved to performance.txt.
"Delete Save" (in Collection) deletes the save and restarts the program automatically.
To quit: "Exit Game" in the main menu, or the HOME button and close the program.

FILES IN sd:/switch/isaac/
--------------------------
  isaac.nro             the program
  Isaac*.exe            the Steam game (you provide it; one per version)
  isaac.ini             settings (volume, smooth = 0 for the original 30 fps,
                        game = og/wotl/eternal, low_latency, touch_controls,
                        joystick_acceleration, gc_after_tick, max_margin, mods...)
  log.txt               program messages from the last session
  saves/                your progress and the files the game learns on this console:
    isaac_save.sol        the Wrath of the Lamb save (isaac_og_save.sol for the original,
                          isaac_eternal_save.sol for Eternal Edition). Same format as on
                          PC: they can be copied between PC and Switch.
    performance.txt       performance data (only if you turned on L3 + R3)
    preload*.txt          graphics seen on this console that weren't in the included list
                          (preloaded next time; safe to delete)
  mods/                 the mods (.mod files), all off until you turn them on (below)
  tools/                save_converter.html: converts saves from the Flash game (open it
                        on a PC or phone browser), as in the PC version
                        mod_manager.html: turns mods on and off
                        genesis_wiki.html: what GenesisConsole can do
Up to version 1.3.1 the saves were directly in sd:/switch/isaac/. Version 1.4.0 copies
them into saves/ the first time it starts; the old files are left where they were (you
can delete them once you've checked your progress is there).

MODS
----
Mods are small optional changes in sd:/switch/isaac/mods/, all off until you turn them on.
Which mods are on, and their options, are in sd:/switch/isaac/mods/mods.txt (written
the first time the game starts): change "on = 0" to "on = 1" for the ones you want, with
any text editor. Or, on a PC or phone, open tools/mod_manager.html in a browser, choose,
save mods.txt and copy it to sd:/switch/isaac/mods/ (replacing the old one).
  StatOn          your stats on the left of the screen during a run (speed, tears, damage,
                  range, shot speed, luck) and the chance of a devil / angel room, as in
                  Repentance.
  GenesisConsole  a console to give items, spawn anything, change stats, god mode,
                  teleport... Minus opens it during a run; A shows the keyboard (what you
                  type runs when you press OK), X runs the line again, Y completes it,
                  the D-pad recalls earlier commands, B or Minus close it. In handheld
                  mode, a tap with three fingers opens / closes it too. Everything it can
                  do: tools/genesis_wiki.html. While it is on, NOTHING IS SAVED (no
                  unlocks, achievements or statistics; your save is left as it was).
                  Its "unlock" command unlocks everything until you close the game.
  WideSaac        fill = 1: the game's background instead of black bars at the
                  sides of the screen; with Hyper Zoom
                  (hyper_zoom = 1) the room fills the screen while you play, with the
                  hearts, active item, coins / bombs / keys on its right side. L3
                  (click the left stick; a four-finger tap in handheld mode) shows or
                  hides Minimap+, the map larger and see-through in the middle.
  PillLog         the effect of pills you already took this run, written over them, in
                  the pickup banner and in a list on the pause screen.
  Itemnomicon    walk up to an item, trinket or card and a card over it shows its
                  name, the stats it changes and what it does.
  BaseTime        speedrun timers (game time, and total time with milliseconds);
                  your last and best times on the main menu.
  The Gone        a new character (like Rebirth's The Lost) after the last one on the
                  character select screen: no hearts, one hit and he dies. Unlock him
                  by clearing three floors in a row without a hit or a heart; with him,
                  Mom's Heart, Isaac and Satan unlock four new things of his own.

IF SOMETHING GOES WRONG
-----------------------
- If a text screen with an error appears, read it: it's almost always because Isaac.exe
  is missing or isn't the Steam v1.48 version. Press + to exit.
- If it crashes, goes black or runs slowly, send me:
    sd:/switch/isaac/log.txt
    sd:/switch/isaac/saves/performance.txt   (play for a while with L3 + R3 turned on)
  and tell me whether you opened it in application mode (holding R) or from the Album,
  and whether the console was docked or handheld.
- If Atmosphère shows an error report (crash report), the file in
  sd:/atmosphere/crash_reports/ helps too.

LICENSES
--------
The third-party code included in the .nro (libnx, SDL2, Mesa, libdrm_nouveau, PlutoVG,
dr_mp3, stb_image and the GCC/newlib runtime) is distributed with its licenses, which are
in the "licenses" folder. The Binding of Isaac is © Edmund McMillen and Florian Himsl;
this package does not contain any game data.
