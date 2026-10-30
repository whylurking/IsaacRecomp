#!/usr/bin/env python3
"""Static AVM1 -> C++ recompiler.

Usage: gen.py input.swf output_dir --ns NAME [--title "Game title"]
(each game lives in its own namespace fl::NAME; exports fl::g_code_NAME with its tables)

Generates:
  gen_atoms.cpp   table of interned strings K[]
  gen_N.cpp       translated functions (frame scripts, functions, buttons)
  gen_tables.cpp  tables (sprite, frame, index) -> function
"""
import sys, os, struct, math
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from swf import load, header, tags, action_blocks
from avm1 import decode as _decode_flat
import avm1

# ---------------------------------------------------------------- structured decoding
def decode_block(d, start, end, pool):
    """Decodes [start,end) and returns a list of Ins; DefineFunction carries .body = (list, pool) and its
    body does NOT appear in the outer list. The constant pool is per activation."""
    out = []
    a = start
    while a < end:
        ins, _ = avm1.decode(d, a, min(end, a + 3 + (struct.unpack_from('<H', d, a + 1)[0] if d[a] >= 0x80 else 0)), pool)
        x = ins[0]
        if x.op == 0x88:
            pool = x.args
        if x.op in (0x9B, 0x8E):
            nm, params, flags, regs, bs, be = x.args
            x.body = decode_block(d, bs, be, pool)
            x.body_range = (bs, be)
            out.append(x)
            a = be
            continue
        out.append(x)
        a = x.end
    return out

def cstr_lit(s):
    b = s.encode('latin1') if isinstance(s, str) else s
    out = ['"']
    for c in b:
        ch = chr(c)
        if ch == '"': out.append('\\"')
        elif ch == '\\': out.append('\\\\')
        elif ch == '?': out.append('\\?')
        elif 32 <= c < 127: out.append(ch)
        else: out.append('\\%03o' % c)
    out.append('"')
    return ''.join(out)

class Atoms:
    def __init__(s): s.idx = {}; s.lst = []
    def get(s, v):
        if v not in s.idx: s.idx[v] = len(s.lst); s.lst.append(v)
        return s.idx[v]

def num_lit(x):
    if x != x: return 'Value(NAN)'
    if x == math.inf: return 'Value(INFINITY)'
    if x == -math.inf: return 'Value(-INFINITY)'
    r = repr(float(x))
    return 'Value(%s)' % r

# ---------------------------------------------------------------- emitter
class Entry:
    __slots__ = ('code', 'atom', 'num', 'slot', 'kind')
    def __init__(s, code, atom=None, num=None, slot=None, kind='lit'):
        s.code = code; s.atom = atom; s.num = num; s.slot = slot; s.kind = kind

class FnGen:
    """Generates the body of a block (DoAction or function)."""
    def __init__(s, gen, name, insts, kind, flags=0, regs=4, params=(), dbg=''):
        s.gen = gen; s.name = name; s.insts = insts; s.kind = kind  # 'frame' | 'func'
        s.flags = flags; s.regs = max(regs, 4); s.params = params; s.dbg = dbg
        s.lines = []; s.S = []; s.maxslot = 0; s.maxp = 0; s.pcount = 0
        s.labels = set(); s.uses_end = False; s.has_nested = any(i.op in (0x9B, 0x8E) for i in insts)
        positions = {i.pos for i in insts}
        s.endpos = insts[-1].end if insts else 0
        for i in insts:
            if i.op in (0x99, 0x9D):
                t = i.args
                if t in positions: s.labels.add(t)
                else: s.uses_end = True

    def emit(s, line): s.lines.append('    ' + line)
    def slot(s, k):
        s.maxslot = max(s.maxslot, k + 1); return 's%d' % k
    def ptemp(s):
        n = s.pcount; s.pcount += 1; s.maxp = max(s.maxp, s.pcount); return 'p%d' % n

    def push_lit(s, code, atom=None, num=None): s.S.append(Entry(code, atom=atom, num=num, kind='lit'))
    def push_expr(s, expr):
        k = len(s.S); v = s.slot(k)
        s.emit('%s = %s;' % (v, expr))
        s.S.append(Entry(v, slot=k, kind='slot'))
    def pop(s):
        if s.S: return s.S.pop()
        t = s.ptemp(); s.emit('%s = *--vm.sp;' % t)
        return Entry(t, kind='tmp')
    def flush(s):
        for e in s.S: s.emit('*vm.sp++ = %s;' % e.code)
        s.S = []
    def top_code(s):
        if s.S: return s.S[-1].code
        return 'vm.sp[-1]'

    def after_call(s):
        if s.kind == 'func':
            s.emit('if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }')
        else:
            s.emit('if (FL_UNLIKELY(baseRemoved(a))) goto L_end;'); s.uses_end = True

    def args_list(s, n):
        return [s.pop() for _ in range(n)]

    def call_args(s, entries):
        if not entries: return 'nullptr', ''
        return 'av', 'Value av[%d] = {%s}; ' % (len(entries), ', '.join(e.code for e in entries))

    def literal_count(s, depth_from_top):
        """If the entry at that depth (0 = top) is a numeric literal, returns it."""
        if len(s.S) > depth_from_top:
            e = s.S[-1 - depth_from_top]
            if e.kind == 'lit' and e.num is not None and e.num == int(e.num) and 0 <= e.num < 4096:
                return int(e.num)
        return None

    def gen_body(s):
        K = s.gen.atoms
        for x in s.insts:
            s.pcount = 0
            if x.pos in s.labels:
                s.flush(); s.lines.append('L_%x:;' % x.pos)
            op = x.op; nm = x.name
            if op == 0x96:   # Push
                for ty, v in x.args:
                    if ty == 'str': i = K.get(v); s.push_lit('Value(K[%d])' % i, atom=i)
                    elif ty == 'num': s.push_lit(num_lit(v), num=v)
                    elif ty == 'null': s.push_lit('VNULL')
                    elif ty == 'undef': s.push_lit('VUNDEF')
                    elif ty == 'bool': s.push_lit('VTRUE' if v else 'VFALSE')
                    elif ty == 'reg':
                        if v >= s.regs: s.regs = v + 1
                        s.push_expr('R[%d]' % v)
            elif op == 0x17:   # Pop
                if s.S: s.S.pop()
                else: s.emit('--vm.sp;')
            elif op in (0x47, 0x0B, 0x0C, 0x0D, 0x3F, 0x48, 0x67, 0x49, 0x66, 0x10, 0x11, 0x0A, 0x0F, 0x0E, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x21, 0x13, 0x29, 0x68):
                r = s.pop(); l = s.pop()
                f = {0x47: 'add2(%s, %s)', 0x0B: 'sub(%s, %s)', 0x0C: 'mul(%s, %s)', 0x0D: 'divv(%s, %s)', 0x3F: 'modv(%s, %s)',
                     0x48: 'less2(%s, %s)', 0x67: 'greater(%s, %s)', 0x49: 'VB(eq2(%s, %s))', 0x66: 'VB(strictEq(%s, %s))',
                     0x10: 'andv(%s, %s)', 0x11: 'orv(%s, %s)', 0x0A: 'Value(toNum(%s) + toNum(%s))', 0x0F: 'VB(toNum(%s) < toNum(%s))',
                     0x0E: 'VB(toNum(%s) == toNum(%s))', 0x60: 'Value((double)(toInt32(%s) & toInt32(%s)))', 0x61: 'Value((double)(toInt32(%s) | toInt32(%s)))',
                     0x62: 'Value((double)(toInt32(%s) ^ toInt32(%s)))', 0x63: 'Value((double)(int32_t)((uint32_t)toInt32(%s) << (toInt32(%s) & 31)))',
                     0x64: 'Value((double)(toInt32(%s) >> (toInt32(%s) & 31)))', 0x65: 'Value((double)((uint32_t)toInt32(%s) >> (toInt32(%s) & 31)))',
                     0x21: 'Value(mkstr(toStr(%s)->s + toStr(%s)->s))', 0x13: 'VB(toStr(%s)->s == toStr(%s)->s)', 0x29: 'VB(toStr(%s)->s < toStr(%s)->s)',
                     0x68: 'VB(toStr(%s)->s > toStr(%s)->s)'}[op]
                s.push_expr(f % (l.code, r.code))
            elif op in (0x12, 0x50, 0x51, 0x18, 0x4A, 0x4B, 0x44, 0x30, 0x14):
                v = s.pop()
                f = {0x12: 'notv(%s)', 0x50: 'incv(%s)', 0x51: 'decv(%s)', 0x18: 'toIntegerv(%s)', 0x4A: 'Value(toNum(%s))',
                     0x4B: 'Value(toStr(%s))', 0x44: 'Value(typeOf(%s))', 0x30: 'randomNumber(%s)', 0x14: 'Value((double)toStr(%s)->s.size())'}[op]
                s.push_expr(f % v.code)
            elif op == 0x34: s.push_expr('getTime()')
            elif op == 0x1C:   # GetVariable
                n = s.pop()
                if n.atom is not None: s.push_expr('getVarK(a, K[%d])' % n.atom)
                else: s.push_expr('getVar(a, %s)' % n.code)
            elif op == 0x1D:
                v = s.pop(); n = s.pop()
                if n.atom is not None: s.emit('setVarK(a, K[%d], %s);' % (n.atom, v.code))
                else: s.emit('setVar(a, %s, %s);' % (n.code, v.code))
            elif op == 0x3C:
                v = s.pop(); n = s.pop()
                if n.atom is not None: s.emit('defLocalK(a, K[%d], %s);' % (n.atom, v.code))
                else: s.emit('defLocal(a, %s, %s);' % (n.code, v.code))
            elif op == 0x41:
                n = s.pop(); s.emit('defLocal2(a, %s);' % n.code)
            elif op == 0x4E:   # GetMember
                k = s.pop(); o = s.pop()
                if k.atom is not None: s.push_expr('getMemK(%s, K[%d])' % (o.code, k.atom))
                else: s.push_expr('getMem(%s, %s)' % (o.code, k.code))
            elif op == 0x4F:
                v = s.pop(); k = s.pop(); o = s.pop()
                if k.atom is not None: s.emit('setMemK(%s, K[%d], %s);' % (o.code, k.atom, v.code))
                else: s.emit('setMem(%s, %s, %s);' % (o.code, k.code, v.code))
            elif op == 0x3D:   # CallFunction
                n = s.literal_count(1)
                if n is None: s.flush(); s.emit('dynCallFunction(a);'); s.after_call(); continue
                name = s.pop(); s.pop(); args = s.args_list(n)
                arr, decl = s.call_args(args)
                k = len(s.S); v = s.slot(k)
                if name.atom is not None: s.emit('{ %s%s = callFunK(a, K[%d], %d, %s); }' % (decl, v, name.atom, n, arr))
                else: s.emit('{ %s%s = callFun(a, %s, %d, %s); }' % (decl, v, name.code, n, arr))
                s.S.append(Entry(v, slot=k, kind='slot')); s.after_call()
            elif op in (0x52, 0x53):   # CallMethod / NewMethod
                n = s.literal_count(2)
                if n is None: s.flush(); s.emit('dynCallMethod(a, %d);' % (1 if op == 0x53 else 0)); s.after_call(); continue
                name = s.pop(); obj = s.pop(); s.pop(); args = s.args_list(n)
                arr, decl = s.call_args(args)
                k = len(s.S); v = s.slot(k)
                if op == 0x52:
                    if name.atom is not None: s.emit('{ %s%s = callMethodK(a, %s, K[%d], %d, %s); }' % (decl, v, obj.code, name.atom, n, arr))
                    else: s.emit('{ %s%s = callMethod(a, %s, %s, %d, %s); }' % (decl, v, obj.code, name.code, n, arr))
                else:
                    s.emit('{ %s%s = newMethod(a, %s, %s, %d, %s); }' % (decl, v, obj.code, name.code, n, arr))
                s.S.append(Entry(v, slot=k, kind='slot')); s.after_call()
            elif op == 0x40:   # NewObject
                n = s.literal_count(1)
                if n is None: s.flush(); s.emit('dynNewObject(a);'); s.after_call(); continue
                name = s.pop(); s.pop(); args = s.args_list(n)
                arr, decl = s.call_args(args)
                k = len(s.S); v = s.slot(k)
                if name.atom is not None: s.emit('{ %s%s = newObjectK(a, K[%d], %d, %s); }' % (decl, v, name.atom, n, arr))
                else: s.emit('{ %s%s = newObject(a, %s, %d, %s); }' % (decl, v, name.code, n, arr))
                s.S.append(Entry(v, slot=k, kind='slot')); s.after_call()
            elif op == 0x42:   # InitArray
                n = s.literal_count(0)
                if n is None: s.flush(); s.emit('dynInitArray();'); continue
                s.pop(); args = s.args_list(n)
                arr, decl = s.call_args(args)
                k = len(s.S); v = s.slot(k)
                s.emit('{ %s%s = initArray(%d, %s); }' % (decl, v, n, arr)); s.S.append(Entry(v, slot=k, kind='slot'))
            elif op == 0x43:   # InitObject
                n = s.literal_count(0)
                if n is None: s.flush(); s.emit('dynInitObject();'); continue
                s.pop(); args = s.args_list(2 * n)
                arr, decl = s.call_args(args)
                k = len(s.S); v = s.slot(k)
                s.emit('{ %s%s = initObject(%d, %s); }' % (decl, v, n, arr)); s.S.append(Entry(v, slot=k, kind='slot'))
            elif op == 0x55:   # Enumerate2
                o = s.pop(); s.flush(); s.emit('enumerate2(%s);' % o.code)
            elif op == 0x46:   # Enumerate
                n = s.pop(); s.flush(); s.emit('enumerate2(getVar(a, %s));' % n.code)
            elif op == 0x4C:   # PushDuplicate
                if s.S:
                    e = s.S[-1]
                    if e.kind == 'lit': s.S.append(Entry(e.code, atom=e.atom, num=e.num, kind='lit'))
                    else: s.push_expr(e.code)
                else:
                    s.emit('*vm.sp = vm.sp[-1]; vm.sp++;')
            elif op == 0x4D:   # StackSwap
                if len(s.S) >= 2:
                    n = len(s.S); e1 = s.S[-2]; e2 = s.S[-1]
                    if e1.kind == 'lit' and e2.kind == 'lit': s.S[-2], s.S[-1] = e2, e1
                    else:
                        a_, b_ = s.slot(n - 2), s.slot(n - 1)
                        s.emit('{ Value t_ = %s; %s = %s; %s = t_; }' % (e1.code, a_, e2.code, b_))
                        s.S[-2] = Entry(a_, slot=n - 2, kind='slot'); s.S[-1] = Entry(b_, slot=n - 1, kind='slot')
                else:
                    s.flush(); s.emit('std::swap(vm.sp[-1], vm.sp[-2]);')
            elif op == 0x87:   # StoreRegister
                if x.args >= s.regs: s.regs = x.args + 1
                s.emit('R[%d] = %s;' % (x.args, s.top_code()))
            elif op == 0x99:   # Jump
                s.flush()
                s.emit('goto %s;' % ('L_%x' % x.args if x.args in s.labels else 'L_end'))
            elif op == 0x9D:   # If
                c = s.pop(); s.flush()
                s.emit('if (truthy(%s)) goto %s;' % (c.code, 'L_%x' % x.args if x.args in s.labels else 'L_end'))
            elif op == 0x3E:   # Return
                v = s.pop()
                if s.kind == 'func': s.emit('{ Value rv_ = %s; vm.sp = sp0; return rv_; }' % v.code)
                else: s.emit('goto L_end;'); s.uses_end = True
                s.S = []
            elif op == 0x00: pass
            elif op == 0x81: s.flush(); s.emit('actGotoFrame(a, %d);' % x.args)
            elif op == 0x9F:
                f = s.pop(); s.flush(); s.emit('actGotoFrame2(a, %s, %s, %d);' % (f.code, 'true' if x.args[0] else 'false', x.args[1]))
            elif op == 0x8C: s.flush(); s.emit('actGotoLabel(a, K[%d]);' % K.get(x.args))
            elif op == 0x06: s.flush(); s.emit('actPlay(a);')
            elif op == 0x07: s.flush(); s.emit('actStop(a);')
            elif op == 0x04: s.flush(); s.emit('actNextFrame(a);')
            elif op == 0x05: s.flush(); s.emit('actPrevFrame(a);')
            elif op == 0x83: s.flush(); s.emit('actGetURL(a, %s, %s);' % (cstr_lit(x.args[0]), cstr_lit(x.args[1])))
            elif op == 0x25: t = s.pop(); s.emit('actRemoveSprite(a, %s);' % t.code)
            elif op == 0x22:
                i = s.pop(); t = s.pop(); s.push_expr('actGetProperty(a, %s, %s)' % (t.code, i.code))
            elif op == 0x23:
                v = s.pop(); i = s.pop(); t = s.pop(); s.emit('actSetProperty(a, %s, %s, %s);' % (t.code, i.code, v.code))
            elif op == 0x26: v = s.pop(); s.emit('traceValue_(%s);' % v.code)
            elif op == 0x88: pass
            elif op == 0x09: s.emit('soundStopAll();')
            elif op in (0x9B, 0x8E):
                fname, params, flags, regs, bs, be = x.args
                fn = s.gen.function(x, s.dbg)
                if fname:
                    s.emit('defineFunction(a, %s, K[%d]);' % (fn, K.get(fname)))
                else:
                    s.push_expr('makeFunction(a, %s, "")' % fn)
            else:
                s.emit('/* unsupported: %s */' % nm)
                s.gen.unsupported[nm] = s.gen.unsupported.get(nm, 0) + 1
        s.flush()

    def render(s):
        s.gen_body()
        out = []
        if s.kind == 'frame':
            out.append('void %s(void* c_) {' % s.name)
            out.append('    Act a; beginTimeline(a, (DisplayObject*)c_);')
            out.append('    Value* sp0 = vm.sp; (void)sp0;')
        else:
            out.append('Value %s(Act& caller, Function* self, Value thisv, int argc, Value* argv) {' % s.name)
            out.append('    Act a;')
            if s.has_nested:
                out.append('    Scope* L_ = gcNew<Scope>(); enterFunction(a, *L_, caller, self, %s);' % ('caller.thisv' if s.flags & 3 else 'thisv'))
            else:
                out.append('    Scope L_; enterFunction(a, L_, caller, self, %s);' % ('caller.thisv' if s.flags & 3 else 'thisv'))
            out.append('    Value* sp0 = vm.sp; (void)argv; (void)argc;')
        out.append('    Value R[%d];' % s.regs)
        if s.kind == 'func':
            r = 1
            if s.flags & 0x01: out.append('    R[%d] = thisv;' % r); r += 1
            if s.flags & 0x04: out.append('    R[%d] = makeArguments(argc, argv);' % r); r += 1
            if s.flags & 0x10: out.append('    R[%d] = VUNDEF;' % r); r += 1
            if s.flags & 0x40: out.append('    R[%d] = Value((Object*)(a.target ? doRoot(a.target) : (DisplayObject*)vm.root));' % r); r += 1
            if s.flags & 0x80: out.append('    { DisplayObject* p_ = a.target ? doParent(a.target) : nullptr; R[%d] = p_ ? Value((Object*)p_) : VUNDEF; }' % r); r += 1
            if s.flags & 0x100: out.append('    R[%d] = Value(vm.global);' % r); r += 1
            if not (s.flags & 0x0C) and s.gen.version_fn2:
                pass
            for i, (reg, pname) in enumerate(s.params):
                val = 'argc > %d ? argv[%d] : VUNDEF' % (i, i)
                if reg: out.append('    R[%d] = %s;' % (reg, val))
                else: out.append('    defLocalK(a, K[%d], %s);' % (s.gen.atoms.get(pname), val))
        if s.maxslot: out.append('    Value %s;' % ', '.join('s%d' % i for i in range(s.maxslot)))
        if s.maxp: out.append('    Value %s;' % ', '.join('p%d' % i for i in range(s.maxp)))
        out.extend(s.lines)
        out.append('L_end:;')
        if s.kind == 'frame':
            out.append('    vm.sp = sp0;')
        else:
            out.append('    vm.sp = sp0; return VUNDEF;')
        out.append('}')
        return '\n'.join(out)

class Gen:
    def __init__(s):
        s.atoms = Atoms(); s.funcs = []; s.fcount = 0; s.unsupported = {}; s.is_v1 = {}; s.version_fn2 = True
        s.frame_entries = []; s.button_entries = []
    def function(s, x, dbg):
        fname, params, flags, regs, bs, be = x.args
        s.fcount += 1
        name = 'f%d_%s' % (s.fcount, ''.join(c if c.isalnum() else '_' for c in fname)[:40])
        if x.op == 0x9B:
            s.is_v1[name] = True; regs = 4
        g = FnGen(s, name, x.body, 'func', flags if x.op == 0x8E else 0, regs, params if x.op == 0x8E else [(0, p) for _, p in params], dbg)
        s.funcs.append((name, g.render(), 'static'))
        return name
    def frame(s, name, insts, dbg):
        g = FnGen(s, name, insts, 'frame', dbg=dbg)
        s.funcs.append((name, g.render(), 'extern'))

def room_scale_patch(insts, S):
    """(--room-scale S, PSP) the room layers (floor, blood, gibs: BitmapData of gridxs x gridys) at scale S instead of
    the game's lowest tier (640x480 at hdx = 1): the tier is the game's own quality setting (800x620 at 1.25, 1100x800
    at 1.667), the layers are drawn scaled by hdx and shown at 100/hdx %. Only the bitmaps change: the collision grid,
    sized from gridxs/gridys too, keeps the 640x480 tier's size. Returns the number of places patched (0 or 3)."""
    def push(i, *vals): return insts[i].op == 0x96 and insts[i].args == list(vals)
    n = 0
    for i in range(len(insts) - 5):
        if push(i, ('str', 'gridxs'), ('num', 640)) and push(i + 2, ('str', 'gridys'), ('num', 480)) and push(i + 4, ('str', 'hdx'), ('num', 1)):
            insts[i].args = [('str', 'gridxs'), ('num', (int(round(640 * S)) + 3) & ~3)]   # (a multiple of 4: GE textures in place)
            insts[i + 2].args = [('str', 'gridys'), ('num', int(round(480 * S)))]
            insts[i + 4].args = [('str', 'hdx'), ('num', S)]
            n += 1
    for name, full in (('xenf', 640), ('yenf', 480)):
        var = 'gridxs' if name == 'xenf' else 'gridys'
        for i in range(len(insts) - 1):
            if push(i, ('str', name), ('str', var)) and insts[i + 1].op == 0x1C:   # name = (gridxs - ...): 640 instead
                insts[i].args = [('str', name), ('num', full)]
                insts[i + 1].op = 0x4A; insts[i + 1].name = 'ToNumber'
                n += 1
    return n

def main():
    args = sys.argv[1:]
    ns = args[args.index('--ns') + 1]; del args[args.index('--ns'):args.index('--ns') + 2]
    room_scale = None
    if '--room-scale' in args:
        room_scale = float(args[args.index('--room-scale') + 1]); del args[args.index('--room-scale'):args.index('--room-scale') + 2]
    title = ns
    if '--title' in args: title = args[args.index('--title') + 1]; del args[args.index('--title'):args.index('--title') + 2]
    src, outdir = args[0], args[1]
    NSO, NSC = 'namespace fl { namespace %s {\n' % ns, '}}  // namespace fl::%s\n' % ns
    os.makedirs(outdir, exist_ok=True)
    d = load(src)
    rc, fr, nf, p = header(d)
    G = Gen()
    counters = {}
    for kind, owner, frame, start, end, extra in action_blocks(d):
        if kind == 'frame':
            key = (owner, frame + 1)
            idx = counters.get(key, 0); counters[key] = idx + 1
            insts = decode_block(d, start, end, None)
            if room_scale:
                k = room_scale_patch(insts, room_scale)
                if k: print('room layers at scale %g: sprite %d (%d places)' % (room_scale, owner, k)); assert k == 3
            name = 's_%d_%d_%d' % (owner, frame + 1, idx)
            G.frame(name, insts, name)
            G.frame_entries.append((owner, frame + 1, idx, name))
        elif kind == 'button':
            key = ('b', owner)
            idx = counters.get(key, 0); counters[key] = idx + 1
            insts = decode_block(d, start, end, None)
            name = 'b_%d_%d' % (owner, idx)
            G.frame(name, insts, name)
            G.button_entries.append((owner, idx, name))
    # --- write files
    with open(os.path.join(outdir, 'gen_decls.h'), 'w') as o:
        o.write('#pragma once\n#include "genrt.h"\n' + NSO + 'extern Str* K[];\n')
        for name, code, linkage in G.funcs:
            if linkage == 'extern': o.write('void %s(void*);\n' % name)
            else: o.write('Value %s(Act&, Function*, Value, int, Value*);\n' % name)
        o.write(NSC)
    files = []; buf = []; size = 0; target = 350_000
    for item in G.funcs:
        if buf and size + len(item[1]) > target:
            files.append(buf); buf = []; size = 0
        buf.append(item); size += len(item[1])
    if buf: files.append(buf)
    for i, f in enumerate(files):
        with open(os.path.join(outdir, 'gen_%d.cpp' % i), 'w') as o:
            o.write('#include "gen_decls.h"\n' + NSO)
            for name, code, linkage in f:
                o.write(code); o.write('\n\n')
            o.write(NSC)
    with open(os.path.join(outdir, 'gen_tables.cpp'), 'w') as o:
        o.write('#include "genrt.h"\n' + NSO)
        for owner, frame, idx, name in G.frame_entries: o.write('void %s(void*);\n' % name)
        for owner, idx, name in G.button_entries: o.write('void %s(void*);\n' % name)
        o.write('void genInitAtoms();\nstatic const ScriptEntry g_frameScripts[] = {\n')
        for owner, frame, idx, name in G.frame_entries: o.write('  {%d, %d, %d, %s},\n' % (owner, frame, idx, name))
        o.write('  {0, 0, 0, nullptr}};\n')
        o.write('static const ButtonEntry g_buttonScripts[] = {\n')
        for owner, idx, name in G.button_entries: o.write('  {%d, %d, %s},\n' % (owner, idx, name))
        o.write('  {0, 0, nullptr}};\n')
        o.write(NSC + 'namespace fl {\nextern const GameCode g_code_%s = {%s, %s, %s::g_frameScripts, %d, %s::g_buttonScripts, %d, %d, %d, %rf, %s::genInitAtoms};\n}\n' % (
            ns, cstr_lit(ns), cstr_lit(title), ns, len(G.frame_entries), ns, len(G.button_entries), rc[1] // 20, rc[3] // 20, fr, ns))
    with open(os.path.join(outdir, 'gen_atoms.cpp'), 'w') as o:
        o.write('#include "genrt.h"\n' + NSO)
        o.write('Str* K[%d];\n' % max(1, len(G.atoms.lst)))
        o.write('static const struct { const char* s; unsigned n; } kAtoms[] = {\n')
        for v in G.atoms.lst:
            o.write('  {%s, %d},\n' % (cstr_lit(v), len(v.encode('latin1'))))
        o.write('  {"", 0}};\n')
        o.write('void genInitAtoms() { for (unsigned i = 0; i < %d; i++) { K[i] = intern(std::string_view(kAtoms[i].s, kAtoms[i].n)); } }\n' % len(G.atoms.lst))
        o.write(NSC)
    print('blocks: %d frame, %d button; functions: %d; atoms: %d; files: %d' % (
        len(G.frame_entries), len(G.button_entries), G.fcount, len(G.atoms.lst), len(files)))
    if G.unsupported: print('UNSUPPORTED:', G.unsupported)

if __name__ == '__main__':
    main()
