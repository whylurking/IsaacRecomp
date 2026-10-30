; IsaacRecomp mod. Turn it on or off with tools/mod_manager.html (or "mods =" in isaac.ini).
id = pilllog
name = PillLog
version = 1.1
author = IsaacRecomp
category = HUD
summary = Pills you already took this run show their effect.
description = Pills you have already taken this run are known by sight: their effect is written over every pill of that colour lying in the room or for sale (green when it's a good one, red when not), the pause screen lists the six pill colours of the run with what each one does, and (option "banner") the banner can say the effect when you pick one up instead of "Pills here!". The names are the game's own, as its HUD shows them. It only reads the game: runs play exactly the same with it on.

; options
; labels = 1 writes the effect over known pills in the room (0: no)
labels = 1
; banner = 1 shows the effect in the banner when you pick up a known pill (0, the default: the game's "Pills here!")
banner = 0
; pause_list = 1 lists the pills of the run on the pause screen (0: no)
pause_list = 1
; opacity = 30-100 (%)
opacity = 100
