#!/usr/bin/env python3
# Puts the mods that come with the program (mods/*.mod) into tools/mod_manager.html, so the page lists them without
# opening any folder: python3 tools/embed_mods.py (run after changing a .mod file; CMake runs it too).
import json, os, re, sys
root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
mods = []
for fn in sorted(os.listdir(os.path.join(root, 'mods'))):
    if not fn.endswith('.mod'): continue
    m = {'file': fn, 'id': '', 'name': '', 'version': '', 'description': '', 'warning': '', 'help': '', 'category': '', 'summary': '', 'options': []}
    note = ''
    for raw in open(os.path.join(root, 'mods', fn), encoding='utf-8-sig'):
        line = raw.strip()
        if not line: note = ''; continue
        if line[0] in ';#': note = line[1:].strip(); continue
        if '=' not in line: continue
        k, v = line.split('=', 1); k = k.strip().lower(); v = v.strip()
        if k in ('id', 'engine'): m[k] = v.lower()
        elif k in ('name', 'version', 'author', 'description', 'warning', 'help', 'category', 'summary'): m[k] = v
        else: m['options'].append({'key': k, 'value': v, 'note': note})
        note = ''
    if m['id']: mods.append(m)
path = os.path.join(root, 'tools', 'mod_manager.html')
html = open(path, encoding='utf-8').read()
data = json.dumps(mods, ensure_ascii=False, indent=0).replace('</', '<\\/')
new = re.sub(r'(<script id="builtin" type="application/json">).*?(</script>)', lambda mt: mt.group(1) + '\n' + data + '\n' + mt.group(2), html, flags=re.S)
if new != html:
    open(path, 'w', encoding='utf-8').write(new)
    print('mod_manager.html: %d mods' % len(mods))
