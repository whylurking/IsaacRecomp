# Itemnomicon: writes the test script that measures what each item does to the stats (GenesisConsole on).
#   python3 measure.py <og|wotl|eternal> > s.txt
#   ISAAC_MODS=genesis ISAAC_GENESIS_LOG=1 build/isaac --headless --norender --script s.txt [--game ...] <exe> 2> out.txt
#   python3 parse.py out.txt > measured_<game>.json
# For each item: back to 3 hearts / no pickups, fire for a while, "stats"; give the item, wait for the pickup
# animation, fire again (the tear rate and range are only updated by firing), "stats"; remove it.
import sys
game = sys.argv[1]
n = 132 if game == "og" else 198
out = open("tests/start_game.txt").read().splitlines()
t = 1000
out.append("%d eval genesis god" % t)
reset = ["soul 0", "maxhp 3", "hp 3", "coins 0", "bombs 1", "keys 0", "luck 0"]
for i in range(1, n + 1):
    for c in reset: out.append("%d eval genesis %s" % (t, c))
    out.append("%d eval set _root.a.player.xp 320" % t); out.append("%d eval set _root.a.player.yp 300" % t)
    t += 2
    out.append("%d down 39" % t); out.append("%d up 39" % (t + 24)); t += 26
    out.append("%d eval genesis stats" % t); t += 1
    out.append("%d eval genesis give %d" % (t, i)); t += 45
    out.append("%d down 39" % t); out.append("%d up 39" % (t + 24)); t += 26
    out.append("%d eval genesis stats" % t); t += 1
    out.append("%d eval genesis remove %d" % (t, i)); t += 1
out.append("%d quit" % t)
print("\n".join(out))
