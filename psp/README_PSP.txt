THE BINDING OF ISAAC — native port for PSP (homebrew) — version 1.5.0
=====================================================================

All three versions of the Flash game on Steam are supported: the original, Wrath of the
Lamb and Eternal Edition (each with its own recompiled code).

This is the same native recomp as on PC, Switch and Android (ActionScript translated to
C++ plus a custom engine), built for the PSP. There is no Flash emulator inside.

WHAT YOU NEED
-------------
- A PSP 2000, 3000 or Go with custom firmware (6.61 PRO-C, LME or ARK-4), or a PS Vita
  with Adrenaline. The PSP 1000 (32 MB of RAM) is not supported: the game needs the 64 MB
  of the later models.
- About 60 MB free on the Memory Stick for each version (the game is unpacked there the
  first time).
- The Steam game (any of the three versions): the Isaac.exe file from its install folder
  (Steam → right-click the game → Manage → Browse local files). It contains all of the
  game's data; graphics, sounds and music are read from it and are not included here.
  The other executable, Binding_of_Isaac.exe, is just the Steam launcher: it won't work.

INSTALLATION
------------
1. Copy the "PSP" folder from this zip to the root of the Memory Stick (it merges with the
   one you already have). You should end up with:  ms0:/PSP/GAME/IsaacRecomp/EBOOT.PBP
2. Copy Isaac.exe from Steam into that same folder:
      ms0:/PSP/GAME/IsaacRecomp/Isaac.exe
   (an extracted .swf works too).
   To have several versions, copy them under different names, for example
      Isaac-Original.exe   Isaac-WotL.exe   Isaac-Eternal.exe
   The program recognizes each version by its contents. If there is more than one, a
   selection screen appears at startup (D-pad and X); your choice is preselected next
   time. Each version has its own save file.
3. Start "The Binding of Isaac" from the Game menu.
   The first time a version starts it is unpacked into saves/ (about a minute); after
   that it starts in a few seconds.

How it works: EBOOT.PBP is a small launcher that finds the games and starts the program of
the version you choose, bin/og, bin/wotl or bin/eternal (the three don't fit in the PSP's
memory together). Keep the bin folder next to EBOOT.PBP.

DISPLAY AND PERFORMANCE
-----------------------
- 30 fps (the game's own rate), the image at the PSP's 272 lines, 4:3 in the middle of the
  screen. In very busy rooms the game can slow down a little, as the Flash game did on a
  slow PC: it never skips logic.
- To see the performance data, set performance = 1 in isaac.ini: it is shown at the top of
  the screen and saved to saves/performance.txt.
- sound = 0 in isaac.ini turns the sound off (a little more speed in busy rooms).

CONTROLS
--------
During play:
  Analog stick / D-pad        move
  Triangle / X / Square / O   shoot up / down / left / right
  L                           bomb
  R                           active item (the one with charges)
  Select                      consumable (card / pill)
  Start                       pause
In menus and the pause screen:
  D-pad / analog stick        navigate (the selected option is framed)
  X                           confirm
  O                           back (on the pause screen: resume the game)
  Start                       pause / resume
To quit: "Exit Game" in the main menu, or the HOME button.

FILES IN ms0:/PSP/GAME/IsaacRecomp/
-----------------------------------
  EBOOT.PBP             the launcher
  bin/                  the program of each version (og, wotl, eternal)
  Isaac*.exe            the Steam game (you provide it; one per version)
  isaac.ini             settings (swf, game = og/wotl/eternal, volume, sound,
                        performance)
  log.txt               program messages from the last session
  saves/                your progress, plus the unpacked game:
    isaac_save.sol        the Wrath of the Lamb save (isaac_og_save.sol for the original,
                          isaac_eternal_save.sol for Eternal Edition). Same format as on
                          PC, Switch and Android: they can be copied between them.
    swf_*.cache           the unpacked game (one per version; made again if deleted)
    performance.txt       performance data (only with performance = 1)
  tools/                save_converter.html: converts saves from the Flash game (open it
                        on a PC or phone browser), as in the other versions

IF SOMETHING GOES WRONG
-----------------------
- If a text screen with an error appears, read it: it's almost always because Isaac.exe
  is missing or isn't the Steam version. Press Start to exit.
- If it crashes, freezes or runs slowly, send me:
    ms0:/PSP/GAME/IsaacRecomp/log.txt
    ms0:/PSP/GAME/IsaacRecomp/saves/performance.txt   (play for a while with
                                                       performance = 1)
  and tell me which model and firmware you use (or Adrenaline on Vita).

LICENSES
--------
The third-party code included in the program (PSPSDK, SDL2, PlutoVG, dr_mp3, stb_image,
zlib, LLVM compiler-rt's floating-point routines and the GCC/newlib runtime) is
distributed with its licenses, which are in the "licenses" folder. The Binding of Isaac is
© Edmund McMillen and Florian Himsl; this package does not contain any game data.
