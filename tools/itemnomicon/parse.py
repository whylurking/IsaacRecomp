# Itemnomicon: the stat changes per item from a measure.py run's log (see measure.py), as JSON on stdout
import json, re, sys
def stats(L, k):
    s, h, d = L[k], L[k + 1], {}
    for m in re.finditer(r"(speed|tears|damage|range|shotspeed|luck) (-?[\d.]+)", s): d[m.group(1)] = float(m.group(2))
    m = re.search(r"hearts ([\d.]+)/([\d.]+)\s+soul ([\d.]+)\s+coins (\d+)\s+bombs (\d+)\s+keys (\d+)", h)
    if m: d.update(hp=float(m.group(1)), mhp=float(m.group(2)), soul=float(m.group(3)), coins=int(m.group(4)), bombs=int(m.group(5)), keys=int(m.group(6)))
    return d
L = [l.split("] ", 1)[1].rstrip() for l in open(sys.argv[1]) if l.startswith("[genesis] ")]
out, pre, cur = {}, None, None
for k, l in enumerate(L):
    m = re.match(r"> give (\d+)$", l)
    if m: cur = int(m.group(1)); continue
    if l == "> stats":
        d = stats(L, k + 1)
        if cur is None: pre = d
        else:
            out[cur] = {x: round(d[x] - pre[x], 2) for x in d if x in pre and abs(d[x] - pre[x]) > 0.004}
            cur = None
json.dump(out, sys.stdout)
