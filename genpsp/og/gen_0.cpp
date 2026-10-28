#include "gen_decls.h"
namespace fl { namespace og {
void s_356_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_356_18_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    setVarK(a, K[0], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_360_19_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    setVarK(a, K[0], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_362_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actSetProperty(a, Value(K[1]), Value(7.0), VFALSE);
L_end:;
    vm.sp = sp0;
}

void s_367_4_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_381_4_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_404_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_444_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_473_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_485_8_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_536_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = actGetProperty(a, Value(K[1]), Value(5.0));
    s0 = randomNumber(s0);
    s0 = add2(s0, Value(1.0));
    actGotoFrame2(a, s0, false, 0);
L_end:;
    vm.sp = sp0;
}

void b_539_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = getVarK(a, K[2]);
    s0 = getMemK(s0, K[3]);
    setMemK(s0, K[4], VTRUE);
L_end:;
    vm.sp = sp0;
}

void b_542_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actGotoFrame(a, 5);
L_end:;
    vm.sp = sp0;
}

void b_545_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[2]);
    s2 = actGetProperty(a, Value(K[1]), Value(4.0));
    setMemK(s0, K[5], s2);
L_end:;
    vm.sp = sp0;
}

void b_679_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actGotoFrame(a, 4);
L_end:;
    vm.sp = sp0;
}

void s_693_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = getVarK(a, K[2]);
    s0 = getMemK(s0, K[6]);
    s0 = add2(s0, Value(1.0));
    actGotoFrame2(a, s0, false, 0);
L_end:;
    vm.sp = sp0;
}

void s_785_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = actGetProperty(a, Value(K[1]), Value(5.0));
    s0 = randomNumber(s0);
    s0 = add2(s0, Value(1.0));
    actGotoFrame2(a, s0, false, 0);
L_end:;
    vm.sp = sp0;
}

void b_788_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[2]);
    setMemK(s0, K[7], VUNDEF);
    s1 = getVarK(a, K[2]);
    s1 = getMemK(s1, K[3]);
    { s0 = callMethodK(a, s1, K[8], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    s0 = getVarK(a, K[2]);
    setMemK(s0, K[9], VUNDEF);
    s2 = getVarK(a, K[2]);
    { Value av[1] = {Value(K[10])}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void b_791_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s1 = getVarK(a, K[2]);
    s1 = getMemK(s1, K[3]);
    { s0 = callMethodK(a, s1, K[8], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    s0 = getVarK(a, K[2]);
    setMemK(s0, K[7], VUNDEF);
    s0 = getVarK(a, K[2]);
    setMemK(s0, K[9], VUNDEF);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {VTRUE}; s0 = callMethodK(a, s2, K[12], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    s2 = getVarK(a, K[2]);
    { Value av[1] = {Value(K[13])}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_856_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = getVarK(a, K[2]);
    s0 = getMemK(s0, K[14]);
    actGotoFrame2(a, s0, false, 0);
L_end:;
    vm.sp = sp0;
}

void s_857_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
    setVarK(a, K[15], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_857_87_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    setVarK(a, K[16], VFALSE);
L_end:;
    vm.sp = sp0;
}

void b_862_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actGotoFrame(a, 7);
L_end:;
    vm.sp = sp0;
}

void b_865_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = getVarK(a, K[2]);
    s0 = getMemK(s0, K[17]);
    s0 = VB(eq2(s0, Value(2.0)));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1a21682;
    actGotoFrame(a, 1);
    goto L_1a216c1;
L_1a21682:;
    actGotoFrame(a, 0);
    s0 = getVarK(a, K[2]);
    s0 = getMemK(s0, K[18]);
    s0 = getMemK(s0, K[18]);
    setMemK(s0, K[19], Value(-1.0));
    s0 = getVarK(a, K[2]);
    s0 = getMemK(s0, K[18]);
    s0 = getMemK(s0, K[18]);
    setMemK(s0, K[20], Value(30.0));
L_1a216c1:;
L_end:;
    vm.sp = sp0;
}

void b_869_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[22], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    s1 = getVarK(a, K[2]);
    { s0 = callMethodK(a, s1, K[23], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void b_873_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[22], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    s1 = getVarK(a, K[2]);
    { s0 = callMethodK(a, s1, K[23], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void b_879_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[2]);
    { Value av[1] = {Value(K[24])}; s0 = callMethodK(a, s2, K[25], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void b_885_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[2]);
    { Value av[1] = {Value(K[26])}; s0 = callMethodK(a, s2, K[25], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void b_891_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[2]);
    { Value av[1] = {Value(K[27])}; s0 = callMethodK(a, s2, K[25], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void b_897_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[2]);
    { Value av[1] = {Value(K[28])}; s0 = callMethodK(a, s2, K[25], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void b_904_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = actGetProperty(a, Value(K[1]), Value(4.0));
    s2 = getVarK(a, K[2]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[29], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void b_916_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[2]);
    { Value av[1] = {Value(2.0)}; s0 = callMethodK(a, s2, K[30], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void b_918_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[2]);
    { Value av[1] = {VTRUE}; s0 = callMethodK(a, s2, K[30], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void b_923_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[2]);
    { Value av[1] = {VFALSE}; s0 = callMethodK(a, s2, K[30], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void b_927_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = getVarK(a, K[2]);
    s0 = getMemK(s0, K[3]);
    setMemK(s0, K[4], VTRUE);
L_end:;
    vm.sp = sp0;
}

void b_928_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[2]);
    setMemK(s0, K[7], VUNDEF);
    s1 = getVarK(a, K[2]);
    s1 = getMemK(s1, K[3]);
    { s0 = callMethodK(a, s1, K[8], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    s0 = getVarK(a, K[2]);
    setMemK(s0, K[9], VUNDEF);
    s2 = getVarK(a, K[2]);
    { Value av[1] = {Value(K[10])}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void b_933_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[2]);
    setMemK(s0, K[7], VUNDEF);
    s1 = getVarK(a, K[2]);
    s1 = getMemK(s1, K[3]);
    { s0 = callMethodK(a, s1, K[8], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    s0 = getVarK(a, K[2]);
    setMemK(s0, K[9], VUNDEF);
    s2 = getVarK(a, K[2]);
    { Value av[1] = {Value(K[10])}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void b_936_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actGotoFrame(a, 1);
L_end:;
    vm.sp = sp0;
}

void b_939_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3, s4;
    s4 = getVarK(a, K[33]);
    { Value av[2] = {Value(K[31]), Value(K[32])}; s1 = callMethodK(a, s4, K[34], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    setVarK(a, K[31], s1);
    s1 = getVarK(a, K[31]);
    { s0 = callMethodK(a, s1, K[35], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    s1 = getVarK(a, K[31]);
    { s0 = callMethodK(a, s1, K[36], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    s1 = getVarK(a, K[38]);
    s1 = getMemK(s1, K[39]);
    s1 = getMemK(s1, K[40]);
    defLocalK(a, K[37], s1);
    s0 = getVarK(a, K[37]);
    s0 = add2(s0, Value(K[41]));
    s2 = getVarK(a, K[38]);
    s2 = getMemK(s2, K[42]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[43], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    s1 = getVarK(a, K[2]);
    { s0 = callMethodK(a, s1, K[44], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    s1 = getVarK(a, K[45]);
    { s0 = callMethodK(a, s1, K[46], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void b_940_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    s1 = getVarK(a, K[45]);
    { s0 = callMethodK(a, s1, K[46], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void b_943_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actGotoFrame(a, 4);
L_end:;
    vm.sp = sp0;
}

Value f1_itit(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = thisv;
    R[2] = Value((Object*)(a.target ? doRoot(a.target) : (DisplayObject*)vm.root));
    Value s0, s1, s2, s3;
    Value p0;
    setVarK(a, K[47], VTRUE);
    setVarK(a, K[48], VTRUE);
    setVarK(a, K[49], Value(0.0));
L_1a28dd8:;
    s0 = getVarK(a, K[49]);
    s0 = less2(s0, Value(20.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_end;
    s1 = R[1];
    s3 = getVarK(a, K[49]);
    s2 = add2(Value(K[49]), s3);
    s1 = getMem(s1, s2);
    setVarK(a, K[50], s1);
    s1 = R[2];
    s1 = getMemK(s1, K[52]);
    s2 = getVarK(a, K[49]);
    s1 = getMem(s1, s2);
    setVarK(a, K[51], s1);
    s0 = getVarK(a, K[51]);
    s0 = greater(s0, Value(0.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1a28e44;
    s0 = getVarK(a, K[51]);
    s2 = getVarK(a, K[50]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1a28f1f;
L_1a28e44:;
    s0 = R[2];
    s0 = getMemK(s0, K[53]);
    s0 = greater(s0, Value(0.0));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1a28e67;
    --vm.sp;
    s0 = getVarK(a, K[47]);
    *vm.sp++ = s0;
L_1a28e67:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1a28e94;
    setVarK(a, K[47], VFALSE);
    s0 = R[2];
    s0 = getMemK(s0, K[53]);
    s2 = getVarK(a, K[50]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1a28f1f;
L_1a28e94:;
    s0 = R[2];
    s0 = getMemK(s0, K[54]);
    s0 = VB(eq2(s0, Value(11.0)));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1a28eb3;
    --vm.sp;
    s0 = getVarK(a, K[48]);
    *vm.sp++ = s0;
L_1a28eb3:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1a28f11;
    setVarK(a, K[48], VFALSE);
    s0 = R[2];
    s0 = getMemK(s0, K[55]);
    s0 = less2(s0, Value(7.0));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1a28ef5;
    s2 = getVarK(a, K[50]);
    { Value av[1] = {Value(61.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1a28f0c;
L_1a28ef5:;
    s2 = getVarK(a, K[50]);
    { Value av[1] = {Value(43.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1a28f0c:;
    goto L_1a28f1f;
L_1a28f11:;
    s0 = getVarK(a, K[50]);
    setMemK(s0, K[56], VFALSE);
L_1a28f1f:;
    s1 = getVarK(a, K[49]);
    s1 = incv(s1);
    setVarK(a, K[49], s1);
    goto L_1a28dd8;
L_end:;
    vm.sp = sp0; return VUNDEF;
}

void s_944_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    defineFunction(a, f1_itit, K[57]);
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_944_2_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    { s0 = callFunK(a, K[57], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_944_3_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    { s0 = callFunK(a, K[57], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    s0 = getVarK(a, K[2]);
    s0 = getMemK(s0, K[58]);
    setMemK(s0, K[15], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_944_5_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    s1 = getVarK(a, K[2]);
    { s0 = callMethodK(a, s1, K[59], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    { s0 = callFunK(a, K[57], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    s1 = getVarK(a, K[2]);
    { s0 = callMethodK(a, s1, K[23], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_944_6_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    { s0 = callFunK(a, K[57], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_944_8_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    s1 = getVarK(a, K[2]);
    { s0 = callMethodK(a, s1, K[59], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    { s0 = callFunK(a, K[57], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    s1 = getVarK(a, K[2]);
    { s0 = callMethodK(a, s1, K[23], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_951_9_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    s1 = getVarK(a, K[45]);
    { s0 = callMethodK(a, s1, K[46], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_952_7_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    s1 = getVarK(a, K[45]);
    { s0 = callMethodK(a, s1, K[60], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_959_83_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
    actSetProperty(a, Value(K[1]), Value(7.0), VFALSE);
L_end:;
    vm.sp = sp0;
}

void s_960_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actSetProperty(a, Value(K[1]), Value(7.0), VFALSE);
L_end:;
    vm.sp = sp0;
}

void s_1045_41_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    s1 = getVarK(a, K[45]);
    { s0 = callMethodK(a, s1, K[60], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1046_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
    setVarK(a, K[20], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_1077_23_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_1080_19_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[45]);
    { Value av[1] = {Value(1.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1081_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_1086_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = actGetProperty(a, Value(K[1]), Value(5.0));
    s0 = randomNumber(s0);
    s0 = add2(s0, Value(1.0));
    actGotoFrame2(a, s0, false, 0);
L_end:;
    vm.sp = sp0;
}

void s_1087_141_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    setVarK(a, K[0], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_1095_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_1095_18_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    setVarK(a, K[0], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_1099_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = actGetProperty(a, Value(K[1]), Value(5.0));
    s0 = randomNumber(s0);
    s0 = add2(s0, Value(1.0));
    actGotoFrame2(a, s0, false, 0);
L_end:;
    vm.sp = sp0;
}

void s_1109_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_1109_18_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    setVarK(a, K[0], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_1128_15_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    s1 = getVarK(a, K[45]);
    { s0 = callMethodK(a, s1, K[60], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1183_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    actStop(a);
    s1 = getVarK(a, K[61]);
    { s0 = callMethodK(a, s1, K[62], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1187_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = actGetProperty(a, Value(K[1]), Value(5.0));
    s0 = randomNumber(s0);
    s0 = add2(s0, Value(1.0));
    actGotoFrame2(a, s0, false, 0);
L_end:;
    vm.sp = sp0;
}

void s_1188_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_1188_18_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    setVarK(a, K[0], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_1197_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_1275_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_1275_18_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    setVarK(a, K[0], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_1279_18_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_1289_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = getVarK(a, K[45]);
    s0 = getMemK(s0, K[45]);
    s0 = getMemK(s0, K[45]);
    s0 = getMemK(s0, K[45]);
    s0 = getMemK(s0, K[63]);
    s0 = VB(eq2(s0, Value(4.0)));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1a54153;
    actSetProperty(a, Value(K[1]), Value(7.0), VFALSE);
L_1a54153:;
L_end:;
    vm.sp = sp0;
}

void s_1295_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_1296_59_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[45]);
    s0 = getMemK(s0, K[45]);
    setMemK(s0, K[64], VTRUE);
    s2 = getVarK(a, K[45]);
    { Value av[1] = {Value(3.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1314_26_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s3 = getVarK(a, K[2]);
    { Value av[2] = {Value(K[65]), Value(100.0)}; s0 = callMethodK(a, s3, K[66], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1314_35_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[45]);
    { Value av[1] = {Value(1.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1316_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_1333_9_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    setVarK(a, K[67], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_1340_16_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    setVarK(a, K[67], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_1341_16_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    setVarK(a, K[67], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_1342_16_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    setVarK(a, K[67], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_1343_5_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    setVarK(a, K[67], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_1343_8_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = getVarK(a, K[2]);
    s0 = getMemK(s0, K[3]);
    s0 = getMemK(s0, K[68]);
    s0 = getMemK(s0, K[69]);
    s0 = less2(s0, Value(15.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1a5d954;
    setVarK(a, K[67], VTRUE);
L_1a5d954:;
L_end:;
    vm.sp = sp0;
}

void s_1344_18_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    setVarK(a, K[67], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_1348_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1351_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1354_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1357_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1360_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1363_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1366_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1369_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1372_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1375_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1378_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1381_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1384_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1387_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1390_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1391_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_1394_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1397_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1402_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1403_22_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[45]);
    { Value av[1] = {Value(1.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1404_22_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[45]);
    { Value av[1] = {Value(1.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1405_22_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[45]);
    { Value av[1] = {Value(1.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1462_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    actStop(a);
    s1 = getVarK(a, K[61]);
    { s0 = callMethodK(a, s1, K[62], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1467_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_1474_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_1480_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    actStop(a);
    s1 = getVarK(a, K[61]);
    { s0 = callMethodK(a, s1, K[62], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_1485_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_1502_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_1534_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_1685_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_1711_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_1754_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_1793_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_1797_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_1802_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_1803_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_1812_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_1853_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_1858_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_1863_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_1955_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_1993_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2002_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2015_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2024_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2033_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2042_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2051_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2060_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2069_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2079_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2079_5_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2100_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2109_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2118_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2127_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2144_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2154_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2163_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2172_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2198_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2207_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2216_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2216_5_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2225_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2234_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2243_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2263_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2272_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2281_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2294_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2307_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2316_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2325_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2364_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2373_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2382_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2383_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2555_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2564_20_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actGotoFrame(a, 0);
    actPlay(a);
L_end:;
    vm.sp = sp0;
}

void s_2564_21_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2568_42_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2570_42_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2572_42_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2573_42_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2576_42_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2577_42_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2580_42_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2620_78_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2624_78_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2628_78_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2632_78_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2636_78_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2640_78_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2644_78_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2740_20_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2741_20_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2742_21_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2743_20_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2744_20_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2745_20_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2746_21_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2750_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2755_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = getVarK(a, K[2]);
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(45.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1b01152;
    actGotoFrame(a, 1);
    goto L_1b01157;
L_1b01152:;
    actGotoFrame(a, 0);
L_1b01157:;
L_end:;
    vm.sp = sp0;
}

void s_2763_14_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2806_17_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2844_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_2862_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    actStop(a);
    s1 = getVarK(a, K[61]);
    { s0 = callMethodK(a, s1, K[62], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_2885_37_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = actGetProperty(a, Value(K[1]), Value(4.0));
    s0 = sub(s0, Value(2.0));
    actGotoFrame2(a, s0, false, 0);
L_end:;
    vm.sp = sp0;
}

void s_2888_27_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    s1 = getVarK(a, K[45]);
    { s0 = callMethodK(a, s1, K[60], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_2941_27_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    s1 = getVarK(a, K[45]);
    { s0 = callMethodK(a, s1, K[60], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_2951_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_2954_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_2957_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_2961_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_2964_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_2968_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_2971_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_2976_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_2980_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    actStop(a);
    s1 = getVarK(a, K[61]);
    { s0 = callMethodK(a, s1, K[62], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_2990_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3023_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    actStop(a);
    s1 = getVarK(a, K[61]);
    { s0 = callMethodK(a, s1, K[62], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

Value f2_onEnterFrame(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = Value((Object*)(a.target ? doRoot(a.target) : (DisplayObject*)vm.root));
    Value s0;
    s0 = R[1];
    s0 = getMemK(s0, K[72]);
    actGotoFrame2(a, s0, false, 0);
L_end:;
    vm.sp = sp0; return VUNDEF;
}

void s_3031_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    defineFunction(a, f2_onEnterFrame, K[73]);
    s0 = getVarK(a, K[2]);
    s0 = getMemK(s0, K[72]);
    actGotoFrame2(a, s0, false, 0);
L_end:;
    vm.sp = sp0;
}

void s_3050_33_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    s1 = getVarK(a, K[45]);
    { s0 = callMethodK(a, s1, K[60], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3052_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    actStop(a);
    s1 = getVarK(a, K[61]);
    { s0 = callMethodK(a, s1, K[62], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3075_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    actStop(a);
    s1 = getVarK(a, K[61]);
    { s0 = callMethodK(a, s1, K[62], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3093_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    actStop(a);
    s1 = getVarK(a, K[61]);
    { s0 = callMethodK(a, s1, K[62], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3118_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3122_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    actStop(a);
    s1 = getVarK(a, K[61]);
    { s0 = callMethodK(a, s1, K[62], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3124_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    actStop(a);
    s1 = getVarK(a, K[61]);
    { s0 = callMethodK(a, s1, K[62], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3127_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3137_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    actStop(a);
    s1 = getVarK(a, K[61]);
    { s0 = callMethodK(a, s1, K[62], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3184_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    actStop(a);
    s1 = getVarK(a, K[61]);
    { s0 = callMethodK(a, s1, K[62], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3199_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    actStop(a);
    s1 = getVarK(a, K[61]);
    { s0 = callMethodK(a, s1, K[62], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3205_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3207_9_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    setVarK(a, K[22], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_3208_8_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    setVarK(a, K[22], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_3208_10_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    setVarK(a, K[22], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_3208_13_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    setVarK(a, K[22], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_3208_15_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    setVarK(a, K[22], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_3208_17_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    setVarK(a, K[22], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_3208_19_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    setVarK(a, K[22], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_3208_21_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    setVarK(a, K[22], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_3208_23_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    setVarK(a, K[22], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_3210_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3231_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3240_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3251_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3256_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    actStop(a);
    s1 = getVarK(a, K[61]);
    { s0 = callMethodK(a, s1, K[62], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3259_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3262_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3265_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3268_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3272_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3276_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3279_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3283_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3288_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3291_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[21]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[70], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3293_18_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    s1 = getVarK(a, K[45]);
    { s0 = callMethodK(a, s1, K[60], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3311_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3319_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3354_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3358_26_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    s1 = getVarK(a, K[45]);
    { s0 = callMethodK(a, s1, K[60], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3359_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3367_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3375_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3380_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3384_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    actStop(a);
    s1 = getVarK(a, K[61]);
    { s0 = callMethodK(a, s1, K[62], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3395_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3402_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3413_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3424_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3447_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3481_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3500_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3544_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3546_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3565_33_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[45]);
    { Value av[1] = {Value(11.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3593_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3629_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3631_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    actStop(a);
    s1 = getVarK(a, K[61]);
    { s0 = callMethodK(a, s1, K[62], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3646_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3652_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actSetProperty(a, Value(K[1]), Value(7.0), VFALSE);
L_end:;
    vm.sp = sp0;
}

void s_3655_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    actStop(a);
    s1 = getVarK(a, K[61]);
    { s0 = callMethodK(a, s1, K[62], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3661_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3663_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    actStop(a);
    s1 = getVarK(a, K[61]);
    { s0 = callMethodK(a, s1, K[62], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3665_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    actStop(a);
    s1 = getVarK(a, K[61]);
    { s0 = callMethodK(a, s1, K[62], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3668_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3685_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3694_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3708_50_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[2]);
    { Value av[1] = {Value(K[74])}; s0 = callMethodK(a, s2, K[66], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3708_74_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[45]);
    s2 = getVarK(a, K[45]);
    s2 = getMemK(s2, K[75]);
    s2 = add2(s2, Value(15.0));
    setMemK(s0, K[75], s2);
    s0 = getVarK(a, K[45]);
    setMemK(s0, K[76], VFALSE);
L_end:;
    vm.sp = sp0;
}

void s_3709_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    actStop(a);
    s1 = getVarK(a, K[61]);
    { s0 = callMethodK(a, s1, K[62], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3717_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3730_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3733_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3735_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actSetProperty(a, Value(K[1]), Value(7.0), VFALSE);
L_end:;
    vm.sp = sp0;
}

void s_3737_9_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    setVarK(a, K[77], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_3737_55_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    setVarK(a, K[77], VFALSE);
L_end:;
    vm.sp = sp0;
}

void s_3737_62_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[45]);
    { Value av[1] = {Value(1.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3738_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3742_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3748_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3750_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3756_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
    setVarK(a, K[78], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_3776_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3787_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    actStop(a);
    s1 = getVarK(a, K[61]);
    { s0 = callMethodK(a, s1, K[62], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3818_15_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    s1 = getVarK(a, K[45]);
    { s0 = callMethodK(a, s1, K[60], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3838_17_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    setVarK(a, K[0], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_3839_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3856_41_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3866_37_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3867_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3949_24_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    s1 = getVarK(a, K[45]);
    { s0 = callMethodK(a, s1, K[60], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_3970_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3986_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_3997_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4001_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    actStop(a);
    s1 = getVarK(a, K[61]);
    { s0 = callMethodK(a, s1, K[62], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4020_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4023_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4025_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4027_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4041_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4043_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4047_192_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    setVarK(a, K[0], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_4070_35_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[45]);
    { Value av[1] = {Value(1.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4074_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4098_35_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[45]);
    { Value av[1] = {Value(1.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4117_35_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[45]);
    { Value av[1] = {Value(1.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4143_20_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4148_22_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[45]);
    { Value av[1] = {Value(1.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4173_20_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4185_19_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[45]);
    { Value av[1] = {Value(1.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4212_35_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[45]);
    { Value av[1] = {Value(1.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4263_61_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = getVarK(a, K[45]);
    s0 = getMemK(s0, K[45]);
    s0 = getMemK(s0, K[45]);
    setMemK(s0, K[0], VTRUE);
L_end:;
    vm.sp = sp0;
}

void s_4269_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4343_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s0 = getVarK(a, K[2]);
    s0 = getMemK(s0, K[3]);
    s0 = getMemK(s0, K[79]);
    s3 = getVarK(a, K[80]);
    { Value av[2] = {Value(7.0), s0}; s0 = callMethodK(a, s3, K[81], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    actGotoFrame2(a, s0, false, 0);
L_end:;
    vm.sp = sp0;
}

void s_4344_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4344_56_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    s1 = getVarK(a, K[2]);
    s1 = getMemK(s1, K[3]);
    { s0 = callMethodK(a, s1, K[82], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4358_49_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[45]);
    { Value av[1] = {Value(1.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

Value f3_nooo(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    Value s0, s1;
    s0 = getVarK(a, K[83]);
    if (truthy(s0)) goto L_1c0fd80;
    s1 = getVarK(a, K[38]);
    s1 = getMemK(s1, K[39]);
    { s0 = callMethodK(a, s1, K[84], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_end;
L_1c0fd80:;
    actGetURL(a, "FSCommand:quit", "");
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f4_ffss1(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = argc > 0 ? argv[0] : VUNDEF;
    Value s0;
    s0 = R[1];
    s0 = VB(eq2(s0, VTRUE));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c0fdc5;
    R[1] = Value(1.0);
L_1c0fdc5:;
    s0 = R[1];
    s0 = greater(s0, Value(0.0));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c0fde7;
    R[1] = Value(0.0);
L_1c0fde7:;
    s0 = R[1];
    { Value rv_ = s0; vm.sp = sp0; return rv_; }
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f5_add(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = argc > 0 ? argv[0] : VUNDEF;
    Value s0, s1, s2, s3, s4;
    s0 = R[1];
    s0 = getMemK(s0, K[69]);
    s0 = greater(s0, Value(0.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c0feb2;
    s1 = getVarK(a, K[51]);
    s3 = R[1];
    s3 = getMem(s3, Value(0.0));
    { Value av[1] = {s3}; s3 = callFunK(a, K[86], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s2 = add2(Value(K[87]), s3);
    s1 = add2(s1, s2);
    setVarK(a, K[51], s1);
    setVarK(a, K[88], Value(1.0));
L_1c0fe56:;
    s0 = getVarK(a, K[88]);
    s1 = R[1];
    s1 = getMemK(s1, K[69]);
    s0 = less2(s0, s1);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c0fead;
    s1 = getVarK(a, K[51]);
    s3 = R[1];
    s4 = getVarK(a, K[88]);
    s3 = getMem(s3, s4);
    { Value av[1] = {s3}; s3 = callFunK(a, K[86], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s2 = add2(Value(K[89]), s3);
    s1 = add2(s1, s2);
    setVarK(a, K[51], s1);
    s1 = getVarK(a, K[88]);
    s1 = incv(s1);
    setVarK(a, K[88], s1);
    goto L_1c0fe56;
L_1c0fead:;
    goto L_end;
L_1c0feb2:;
    s0 = R[1];
    s0 = VB(eq2(s0, VUNDEF));
    s1 = R[1];
    s1 = notv(s1);
    s0 = orv(s0, s1);
    s1 = R[1];
    s1 = VB(eq2(s1, Value(K[90])));
    s0 = orv(s0, s1);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c0fee2;
    R[1] = Value(0.0);
L_1c0fee2:;
    s1 = getVarK(a, K[51]);
    s3 = R[1];
    s2 = add2(Value(K[87]), s3);
    s1 = add2(s1, s2);
    setVarK(a, K[51], s1);
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f6_sub(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[2] = argc > 0 ? argv[0] : VUNDEF;
    Value s0, s1, s2, s3;
    Value p0;
    s1 = getVarK(a, K[51]);
    s1 = incv(s1);
    setVarK(a, K[51], s1);
    s0 = getVarK(a, K[92]);
    s1 = getVarK(a, K[51]);
    s0 = getMem(s0, s1);
    R[1] = s0;
    s0 = R[2];
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c0ffce;
    s2 = R[1];
    { Value av[1] = {Value(K[89])}; s0 = callMethodK(a, s2, K[93], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    R[1] = s0;
    s0 = R[1];
    enumerate2(s0);
L_1c0ff59:;
    R[0] = vm.sp[-1];
    p0 = *--vm.sp;
    s0 = VB(eq2(p0, VNULL));
    if (truthy(s0)) goto L_1c0ffc8;
    s1 = R[0];
    setVarK(a, K[88], s1);
    s0 = R[1];
    s1 = getVarK(a, K[88]);
    s2 = R[1];
    s3 = getVarK(a, K[88]);
    s2 = getMem(s2, s3);
    s2 = mul(s2, Value(1.0));
    setMem(s0, s1, s2);
    s0 = R[1];
    s1 = getVarK(a, K[88]);
    s0 = getMem(s0, s1);
    s0 = greater(s0, Value(0.0));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c0ffc3;
    s0 = R[1];
    s1 = getVarK(a, K[88]);
    setMem(s0, s1, Value(0.0));
L_1c0ffc3:;
    goto L_1c0ff59;
L_1c0ffc8:;
    s0 = R[1];
    { Value rv_ = s0; vm.sp = sp0; return rv_; }
L_1c0ffce:;
    s0 = R[1];
    s0 = mul(s0, Value(1.0));
    s0 = greater(s0, Value(0.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c0fffe;
    s0 = R[1];
    s0 = mul(s0, Value(1.0));
    R[1] = s0;
L_1c0fffe:;
    s0 = R[1];
    s0 = VB(eq2(s0, Value(K[94])));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c10015;
    s0 = R[1];
L_1c10015:;
    s0 = R[1];
    s0 = VB(eq2(s0, Value(K[95])));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c10030;
    R[1] = VTRUE;
L_1c10030:;
    s0 = R[1];
    s0 = VB(eq2(s0, Value(K[90])));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c1004b;
    R[1] = VFALSE;
L_1c1004b:;
    s0 = R[1];
    s0 = VB(eq2(s0, Value(K[96])));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c10069;
    R[1] = Value(0.0);
L_1c10069:;
    s0 = R[1];
    { Value rv_ = s0; vm.sp = sp0; return rv_; }
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f7_aloc(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    Value s0, s1, s2, s3;
    Value p0;
    { Value av[1] = {Value(200.0)}; s1 = newObjectK(a, K[99], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setVarK(a, K[98], s1);
    setVarK(a, K[100], Value(0.0));
L_1c100a5:;
    s0 = getVarK(a, K[100]);
    s0 = less2(s0, Value(200.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c100e6;
    s0 = getVarK(a, K[98]);
    s1 = getVarK(a, K[100]);
    s2 = getVarK(a, K[101]);
    s3 = getVarK(a, K[100]);
    s2 = getMem(s2, s3);
    setMem(s0, s1, s2);
    s1 = getVarK(a, K[100]);
    s1 = incv(s1);
    setVarK(a, K[100], s1);
    goto L_1c100a5;
L_1c100e6:;
    s0 = getVarK(a, K[71]);
    enumerate2(s0);
L_1c100ed:;
    R[0] = vm.sp[-1];
    p0 = *--vm.sp;
    s0 = VB(eq2(p0, VNULL));
    if (truthy(s0)) goto L_1c101ae;
    s1 = R[0];
    setVarK(a, K[100], s1);
    s0 = getVarK(a, K[71]);
    s1 = getVarK(a, K[100]);
    s0 = getMem(s0, s1);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c101a9;
    s1 = getVarK(a, K[100]);
    s1 = mul(s1, Value(2.0));
    s1 = add2(s1, Value(11.0));
    setVarK(a, K[51], s1);
    s0 = getVarK(a, K[98]);
    s1 = getVarK(a, K[51]);
    s2 = getVarK(a, K[98]);
    s3 = getVarK(a, K[51]);
    s2 = getMem(s2, s3);
    s3 = getVarK(a, K[100]);
    s3 = modv(s3, Value(4.0));
    s3 = add2(s3, Value(1.0));
    s2 = add2(s2, s3);
    setMem(s0, s1, s2);
    s0 = getVarK(a, K[98]);
    s1 = getVarK(a, K[51]);
    s0 = getMem(s0, s1);
    s0 = greater(s0, Value(9.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c101a9;
    s0 = getVarK(a, K[98]);
    s1 = getVarK(a, K[51]);
    s2 = getVarK(a, K[98]);
    s3 = getVarK(a, K[51]);
    s2 = getMem(s2, s3);
    s2 = sub(s2, Value(9.0));
    setMem(s0, s1, s2);
L_1c101a9:;
    goto L_1c100ed;
L_1c101ae:;
    setVarK(a, K[51], Value(K[1]));
    setVarK(a, K[100], Value(0.0));
L_1c101c7:;
    s0 = getVarK(a, K[100]);
    s0 = less2(s0, Value(200.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c10208;
    s1 = getVarK(a, K[51]);
    s2 = getVarK(a, K[98]);
    s3 = getVarK(a, K[100]);
    s2 = getMem(s2, s3);
    s1 = add2(s1, s2);
    setVarK(a, K[51], s1);
    s1 = getVarK(a, K[100]);
    s1 = incv(s1);
    setVarK(a, K[100], s1);
    goto L_1c101c7;
L_1c10208:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[103]);
    { Value av[1] = {s0}; s0 = callFunK(a, K[91], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[104]);
    { Value av[1] = {s0}; s0 = callFunK(a, K[91], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[103]);
    { Value av[1] = {s0}; s0 = callFunK(a, K[91], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[105]);
    { Value av[1] = {s0}; s0 = callFunK(a, K[91], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[106]);
    { Value av[1] = {s0}; s0 = callFunK(a, K[91], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[107]);
    { Value av[1] = {s0}; s0 = callFunK(a, K[91], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[108]);
    { Value av[1] = {s0}; s0 = callFunK(a, K[91], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[109]);
    { Value av[1] = {s0}; s0 = callFunK(a, K[91], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[110]);
    { Value av[1] = {s0}; s0 = callFunK(a, K[91], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[111]);
    { Value av[1] = {s0}; s0 = callFunK(a, K[91], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[112]);
    { Value av[1] = {s0}; s0 = callFunK(a, K[91], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[52]);
    { Value av[1] = {s0}; s0 = callFunK(a, K[91], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[113]);
    { Value av[1] = {s0}; s0 = callFunK(a, K[91], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[114]);
    { Value av[1] = {s0}; s0 = callFunK(a, K[91], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[115]);
    { Value av[1] = {s0}; s0 = callFunK(a, K[91], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    defLocal2(a, Value(K[116]));
    s0 = getVarK(a, K[83]);
    if (truthy(s0)) goto L_end;
    s1 = getVarK(a, K[38]);
    s1 = getMemK(s1, K[39]);
    s1 = getMemK(s1, K[40]);
    setVarK(a, K[116], s1);
    s0 = getVarK(a, K[51]);
    s1 = getVarK(a, K[116]);
    s1 = add2(s1, Value(K[41]));
    s3 = getVarK(a, K[38]);
    s3 = getMemK(s3, K[42]);
    { Value av[2] = {s1, s0}; s0 = callMethodK(a, s3, K[117], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f8_aloc2(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = Value((Object*)(a.target ? doRoot(a.target) : (DisplayObject*)vm.root));
    Value s0, s1, s2, s3;
    R[3] = VUNDEF;
    R[2] = VUNDEF;
    s0 = getVarK(a, K[83]);
    if (truthy(s0)) goto L_end;
    s0 = getVarK(a, K[38]);
    s0 = getMemK(s0, K[39]);
    s0 = getMemK(s0, K[40]);
    R[3] = s0;
    s0 = R[3];
    s0 = add2(s0, Value(K[41]));
    s2 = getVarK(a, K[38]);
    s2 = getMemK(s2, K[42]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[119], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    R[2] = s0;
    s3 = R[2];
    { Value av[1] = {Value(K[87])}; s1 = callMethodK(a, s3, K[93], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setVarK(a, K[92], s1);
    setVarK(a, K[51], Value(0.0));
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[120]);
    if (truthy(s0)) goto L_1c1075c;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[111]);
    s0 = greater(s0, Value(5.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c10536;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    setMemK(s0, K[120], VTRUE);
L_1c10536:;
    s0 = getVarK(a, K[92]);
    s0 = getMemK(s0, K[69]);
    s0 = greater(s0, Value(10.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c1075c;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    setMemK(s0, K[120], VTRUE);
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { s2 = callFunK(a, K[97], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setMemK(s0, K[103], s2);
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { s2 = callFunK(a, K[97], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setMemK(s0, K[104], s2);
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { s2 = callFunK(a, K[97], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setMemK(s0, K[103], s2);
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { s2 = callFunK(a, K[97], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setMemK(s0, K[105], s2);
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { s2 = callFunK(a, K[97], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setMemK(s0, K[106], s2);
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { s2 = callFunK(a, K[97], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setMemK(s0, K[107], s2);
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { s2 = callFunK(a, K[97], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setMemK(s0, K[108], s2);
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { s2 = callFunK(a, K[97], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setMemK(s0, K[109], s2);
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { s2 = callFunK(a, K[97], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setMemK(s0, K[110], s2);
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { s2 = callFunK(a, K[97], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setMemK(s0, K[111], s2);
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { s2 = callFunK(a, K[97], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setMemK(s0, K[112], s2);
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { Value av[1] = {VTRUE}; s2 = callFunK(a, K[97], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setMemK(s0, K[52], s2);
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { Value av[1] = {VTRUE}; s2 = callFunK(a, K[97], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setMemK(s0, K[113], s2);
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { Value av[1] = {VTRUE}; s2 = callFunK(a, K[97], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setMemK(s0, K[114], s2);
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { Value av[1] = {VTRUE}; s2 = callFunK(a, K[97], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setMemK(s0, K[115], s2);
L_1c1075c:;
    s3 = getVarK(a, K[92]);
    s3 = getMem(s3, Value(0.0));
    { Value av[1] = {Value(K[1])}; s1 = callMethodK(a, s3, K[93], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setVarK(a, K[98], s1);
    setVarK(a, K[100], Value(0.0));
L_1c10792:;
    s0 = getVarK(a, K[100]);
    s0 = less2(s0, Value(100.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_end;
    s0 = getVarK(a, K[71]);
    s1 = getVarK(a, K[100]);
    s0 = getMem(s0, s1);
    if (truthy(s0)) goto L_1c108a1;
    s1 = getVarK(a, K[100]);
    s1 = mul(s1, Value(2.0));
    s1 = add2(s1, Value(11.0));
    setVarK(a, K[51], s1);
    s1 = getVarK(a, K[101]);
    s2 = getVarK(a, K[51]);
    s1 = getMem(s1, s2);
    s1 = mul(s1, Value(1.0));
    s2 = getVarK(a, K[100]);
    s2 = modv(s2, Value(4.0));
    s2 = mul(s2, Value(1.0));
    s1 = add2(s1, s2);
    s1 = add2(s1, Value(1.0));
    setVarK(a, K[47], s1);
    s0 = getVarK(a, K[47]);
    s0 = greater(s0, Value(9.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c1083f;
    s1 = getVarK(a, K[47]);
    s1 = sub(s1, Value(9.0));
    setVarK(a, K[47], s1);
L_1c1083f:;
    s0 = getVarK(a, K[47]);
    s1 = getVarK(a, K[98]);
    s2 = getVarK(a, K[51]);
    s1 = getMem(s1, s2);
    s0 = VB(eq2(s0, s1));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c108a1;
    s0 = getVarK(a, K[71]);
    s1 = getVarK(a, K[100]);
    s2 = getVarK(a, K[121]);
    s3 = getVarK(a, K[100]);
    R[0] = VTRUE;
    setMem(s2, s3, VTRUE);
    s2 = R[0];
    setMem(s0, s1, s2);
    s0 = getVarK(a, K[100]);
    s2 = R[1];
    s2 = getMemK(s2, K[122]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[123], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c108a1:;
    s1 = getVarK(a, K[100]);
    s1 = incv(s1);
    setVarK(a, K[100], s1);
    goto L_1c10792;
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f9_soundy(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[5];
    R[1] = Value((Object*)(a.target ? doRoot(a.target) : (DisplayObject*)vm.root));
    R[2] = argc > 0 ? argv[0] : VUNDEF;
    R[3] = argc > 1 ? argv[1] : VUNDEF;
    R[4] = argc > 2 ? argv[2] : VUNDEF;
    Value s0, s1, s2, s3, s4, s5, s6;
    Value p0;
    s0 = R[1];
    s0 = getMemK(s0, K[125]);
    s1 = R[2];
    s0 = getMem(s0, s1);
    s0 = VB(eq2(s0, VUNDEF));
    s1 = R[4];
    s0 = orv(s0, s1);
    s0 = notv(s0);
    if (truthy(s0)) goto L_end;
    s0 = R[1];
    s0 = getMemK(s0, K[125]);
    s1 = R[2];
    setMem(s0, s1, VTRUE);
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[126]);
    s0 = notv(s0);
    s1 = R[4];
    s0 = orv(s0, s1);
    s0 = notv(s0);
    if (truthy(s0)) goto L_end;
    s3 = R[2];
    { Value av[1] = {Value(K[128])}; s1 = callMethodK(a, s3, K[129], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s1 = greater(s1, Value(0.0));
    s1 = notv(s1);
    setVarK(a, K[127], s1);
    s2 = R[2];
    { Value av[1] = {Value(K[130])}; s0 = callMethodK(a, s2, K[129], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = greater(s0, Value(0.0));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c10984;
    --vm.sp;
    s0 = getVarK(a, K[127]);
    *vm.sp++ = s0;
L_1c10984:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c1099a;
    s0 = R[2];
    s0 = add2(s0, Value(K[131]));
    R[2] = s0;
L_1c1099a:;
    setVarK(a, K[127], VTRUE);
    s0 = R[3];
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c109c2;
    R[3] = Value(100.0);
L_1c109c2:;
    s0 = R[3];
    s1 = R[1];
    s1 = getMemK(s1, K[132]);
    s0 = mul(s0, s1);
    R[3] = s0;
    s1 = R[1];
    s1 = getMemK(s1, K[134]);
    s1 = getMemK(s1, K[69]);
    setVarK(a, K[133], s1);
    R[0] = VFALSE;
    setVarK(a, K[51], VFALSE);
    s1 = R[0];
    setVarK(a, K[47], s1);
    s0 = R[1];
    s0 = getMemK(s0, K[135]);
    s1 = R[1];
    s1 = getMemK(s1, K[136]);
    s0 = VB(eq2(s0, s1));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c10b26;
    s0 = R[2];
    s0 = VB(eq2(s0, Value(K[137])));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c10a4d;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[138]);
    s0 = VB(eq2(s0, Value(6.0)));
    *vm.sp++ = s0;
L_1c10a4d:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c10a7d;
    setVarK(a, K[51], VTRUE);
    R[2] = Value(K[139]);
    s0 = R[1];
    setMemK(s0, K[140], VTRUE);
    goto L_1c10b26;
L_1c10a7d:;
    s0 = R[2];
    s0 = VB(eq2(s0, Value(K[137])));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c10aa4;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[138]);
    s0 = VB(eq2(s0, Value(8.0)));
    *vm.sp++ = s0;
L_1c10aa4:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c10ad4;
    setVarK(a, K[51], VTRUE);
    R[2] = Value(K[141]);
    s0 = R[1];
    setMemK(s0, K[140], VTRUE);
    goto L_1c10b26;
L_1c10ad4:;
    s0 = R[2];
    s0 = VB(eq2(s0, Value(K[137])));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c10afb;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[138]);
    s0 = greater(s0, Value(8.0));
    *vm.sp++ = s0;
L_1c10afb:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c10b26;
    setVarK(a, K[51], VTRUE);
    R[2] = Value(K[142]);
    s0 = R[1];
    setMemK(s0, K[140], VTRUE);
L_1c10b26:;
    s0 = R[1];
    s2 = R[1];
    s2 = getMemK(s2, K[143]);
    s2 = add2(s2, Value(1.0));
    setMemK(s0, K[143], s2);
    s0 = R[1];
    s0 = getMemK(s0, K[134]);
    s1 = getVarK(a, K[133]);
    s2 = R[1];
    s2 = getMemK(s2, K[143]);
    s2 = add2(s2, Value(2000000000.0));
    s4 = R[1];
    s4 = getMemK(s4, K[143]);
    s3 = add2(Value(K[134]), s4);
    s6 = R[1];
    { Value av[3] = {Value(K[144]), s3, s2}; s2 = callMethodK(a, s6, K[145], 3, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setMem(s0, s1, s2);
    s0 = R[1];
    s0 = getMemK(s0, K[134]);
    s1 = getVarK(a, K[133]);
    s0 = getMem(s0, s1);
    s2 = R[2];
    setMemK(s0, K[146], s2);
    s0 = R[1];
    s0 = getMemK(s0, K[134]);
    s1 = getVarK(a, K[133]);
    s0 = getMem(s0, s1);
    s2 = R[1];
    s2 = getMemK(s2, K[134]);
    s3 = getVarK(a, K[133]);
    s2 = getMem(s2, s3);
    { Value av[1] = {s2}; s2 = newObjectK(a, K[147], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setMemK(s0, K[63], s2);
    s0 = getVarK(a, K[127]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c10c26;
    s0 = R[2];
    s2 = R[1];
    s2 = getMemK(s2, K[134]);
    s3 = getVarK(a, K[133]);
    s2 = getMem(s2, s3);
    s2 = getMemK(s2, K[63]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[148], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1c10c57;
L_1c10c26:;
    s1 = R[2];
    s3 = R[1];
    s3 = getMemK(s3, K[134]);
    s4 = getVarK(a, K[133]);
    s3 = getMem(s3, s4);
    s3 = getMemK(s3, K[63]);
    { Value av[2] = {s1, VFALSE}; s0 = callMethodK(a, s3, K[149], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c10c57:;
    setVarK(a, K[88], Value(0.0));
L_1c10c65:;
    s0 = getVarK(a, K[88]);
    s0 = less2(s0, Value(25.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c10d0b;
    s0 = R[2];
    s2 = getVarK(a, K[88]);
    s1 = add2(Value(K[150]), s2);
    s1 = add2(s1, Value(K[151]));
    s0 = VB(eq2(s0, s1));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c10cd0;
    setVarK(a, K[51], VTRUE);
    setVarK(a, K[47], VTRUE);
    s0 = getVarK(a, K[88]);
    s0 = VB(eq2(s0, Value(19.0)));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c10cd0;
    R[3] = Value(0.0);
L_1c10cd0:;
    s0 = R[2];
    s2 = getVarK(a, K[88]);
    s1 = add2(Value(K[150]), s2);
    s1 = add2(s1, Value(K[152]));
    s0 = VB(eq2(s0, s1));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c10cf9;
    setVarK(a, K[51], VTRUE);
L_1c10cf9:;
    s1 = getVarK(a, K[88]);
    s1 = incv(s1);
    setVarK(a, K[88], s1);
    goto L_1c10c65;
L_1c10d0b:;
    s0 = getVarK(a, K[51]);
    s1 = R[2];
    s1 = VB(eq2(s1, Value(K[137])));
    s0 = orv(s0, s1);
    s1 = R[2];
    s1 = VB(eq2(s1, Value(K[153])));
    s0 = orv(s0, s1);
    s1 = R[2];
    s1 = VB(eq2(s1, Value(K[154])));
    s0 = orv(s0, s1);
    s1 = R[2];
    s1 = VB(eq2(s1, Value(K[155])));
    s0 = orv(s0, s1);
    s1 = R[2];
    s1 = VB(eq2(s1, Value(K[156])));
    s0 = orv(s0, s1);
    s1 = R[2];
    s1 = VB(eq2(s1, Value(K[157])));
    s0 = orv(s0, s1);
    s1 = R[2];
    s1 = VB(eq2(s1, Value(K[158])));
    s0 = orv(s0, s1);
    s1 = R[2];
    s1 = VB(eq2(s1, Value(K[159])));
    s0 = orv(s0, s1);
    s1 = R[2];
    s1 = VB(eq2(s1, Value(K[160])));
    s0 = orv(s0, s1);
    s1 = R[2];
    s1 = VB(eq2(s1, Value(K[161])));
    s0 = orv(s0, s1);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c10f06;
    s0 = R[2];
    s0 = VB(eq2(s0, Value(K[137])));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c10db0;
    s0 = R[1];
    setMemK(s0, K[140], VTRUE);
L_1c10db0:;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c10dd7;
    s0 = R[1];
    setMemK(s0, K[163], VTRUE);
L_1c10dd7:;
    s0 = R[2];
    s0 = VB(eq2(s0, Value(K[153])));
    s1 = R[2];
    s1 = VB(eq2(s1, Value(K[154])));
    s0 = orv(s0, s1);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c10e11;
    s0 = R[1];
    s0 = getMemK(s0, K[134]);
    s1 = getVarK(a, K[133]);
    s0 = getMem(s0, s1);
    setMemK(s0, K[164], VTRUE);
L_1c10e11:;
    s0 = R[1];
    s0 = getMemK(s0, K[165]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c10f01;
    s0 = getVarK(a, K[47]);
    s1 = R[2];
    s1 = VB(eq2(s1, Value(K[158])));
    s0 = orv(s0, s1);
    s1 = R[2];
    s1 = VB(eq2(s1, Value(K[159])));
    s0 = orv(s0, s1);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c10ea2;
    s0 = R[1];
    s0 = getMemK(s0, K[134]);
    s1 = getVarK(a, K[133]);
    s0 = getMem(s0, s1);
    setMemK(s0, K[166], VTRUE);
    s1 = getVarK(a, K[167]);
    s3 = R[1];
    s3 = getMemK(s3, K[134]);
    s4 = getVarK(a, K[133]);
    s3 = getMem(s3, s4);
    s3 = getMemK(s3, K[63]);
    { Value av[2] = {s1, Value(0.0)}; s0 = callMethodK(a, s3, K[168], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1c10ed9;
L_1c10ea2:;
    s3 = R[1];
    s3 = getMemK(s3, K[134]);
    s4 = getVarK(a, K[133]);
    s3 = getMem(s3, s4);
    s3 = getMemK(s3, K[63]);
    { Value av[2] = {Value(0.0), Value(0.0)}; s0 = callMethodK(a, s3, K[168], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c10ed9:;
    s0 = R[1];
    s2 = R[1];
    s2 = getMemK(s2, K[134]);
    s3 = getVarK(a, K[133]);
    s2 = getMem(s2, s3);
    setMemK(s0, K[165], s2);
    s0 = getVarK(a, K[169]);
    R[3] = s0;
L_1c10f01:;
    goto L_1c1100d;
L_1c10f06:;
    s2 = R[2];
    { Value av[1] = {Value(K[170])}; s0 = callMethodK(a, s2, K[129], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = greater(s0, Value(0.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c10f86;
    s3 = R[1];
    s3 = getMemK(s3, K[134]);
    s4 = getVarK(a, K[133]);
    s3 = getMem(s3, s4);
    s3 = getMemK(s3, K[63]);
    { Value av[2] = {Value(0.0), Value(999999.0)}; s0 = callMethodK(a, s3, K[168], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = R[1];
    s0 = getMemK(s0, K[134]);
    s1 = getVarK(a, K[133]);
    s0 = getMem(s0, s1);
    setMemK(s0, K[171], VTRUE);
    goto L_1c10fbd;
L_1c10f86:;
    s3 = R[1];
    s3 = getMemK(s3, K[134]);
    s4 = getVarK(a, K[133]);
    s3 = getMem(s3, s4);
    s3 = getMemK(s3, K[63]);
    { Value av[2] = {Value(0.0), Value(0.0)}; s0 = callMethodK(a, s3, K[168], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c10fbd:;
    s2 = R[2];
    { Value av[1] = {Value(K[172])}; s0 = callMethodK(a, s2, K[129], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = greater(s0, Value(0.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c1100d;
    s0 = getVarK(a, K[173]);
    setMemK(s0, K[64], VTRUE);
    s1 = R[1];
    s1 = getMemK(s1, K[134]);
    s2 = getVarK(a, K[133]);
    s1 = getMem(s1, s2);
    setVarK(a, K[173], s1);
L_1c1100d:;
    s0 = R[3];
    s2 = R[1];
    s2 = getMemK(s2, K[134]);
    s3 = getVarK(a, K[133]);
    s2 = getMem(s2, s3);
    s2 = getMemK(s2, K[63]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[174], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = R[1];
    s0 = getMemK(s0, K[134]);
    s1 = getVarK(a, K[133]);
    s0 = getMem(s0, s1);
    { Value rv_ = s0; vm.sp = sp0; return rv_; }
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f10_soz(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = Value((Object*)(a.target ? doRoot(a.target) : (DisplayObject*)vm.root));
    Value s0, s1, s2, s3, s4;
    s4 = getVarK(a, K[33]);
    { Value av[2] = {Value(K[31]), Value(K[32])}; s1 = callMethodK(a, s4, K[34], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setVarK(a, K[31], s1);
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[175]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c110b4;
    { Value av[1] = {Value(K[26])}; s0 = callFunK(a, K[25], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1c110d5;
L_1c110b4:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[175]);
    { Value av[1] = {s0}; s0 = callFunK(a, K[25], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c110d5:;
    s0 = R[1];
    s2 = getVarK(a, K[31]);
    s2 = getMemK(s2, K[102]);
    s2 = getMemK(s2, K[177]);
    setMemK(s0, K[176], s2);
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[105]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c11129;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    setMemK(s0, K[105], Value(0.0));
L_1c11129:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[106]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c11160;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    setMemK(s0, K[106], Value(0.0));
L_1c11160:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[107]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c11197;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    setMemK(s0, K[107], Value(0.0));
L_1c11197:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[108]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c111ce;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    setMemK(s0, K[108], Value(0.0));
L_1c111ce:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[109]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c11205;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    setMemK(s0, K[109], Value(0.0));
L_1c11205:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[110]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c1123c;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    setMemK(s0, K[110], Value(0.0));
L_1c1123c:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[111]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c11273;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    setMemK(s0, K[111], Value(0.0));
L_1c11273:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[112]);
    s0 = greater(s0, Value(0.0));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c112af;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    setMemK(s0, K[112], Value(0.0));
L_1c112af:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[52]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c112e7;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { s2 = initArray(0, nullptr); }
    setMemK(s0, K[52], s2);
L_1c112e7:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[178]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c1131f;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { s2 = initArray(0, nullptr); }
    setMemK(s0, K[178], s2);
L_1c1131f:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[122]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c11357;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { s2 = initArray(0, nullptr); }
    setMemK(s0, K[122], s2);
L_1c11357:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[179]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c1138f;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { s2 = initArray(0, nullptr); }
    setMemK(s0, K[179], s2);
L_1c1138f:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[113]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c113c7;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { s2 = initArray(0, nullptr); }
    setMemK(s0, K[113], s2);
L_1c113c7:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[114]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c1144f;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { Value av[10] = {Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0)}; s2 = initArray(10, av); }
    setMemK(s0, K[114], s2);
L_1c1144f:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[115]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c114d7;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { Value av[10] = {Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0)}; s2 = initArray(10, av); }
    setMemK(s0, K[115], s2);
L_1c114d7:;
    s0 = R[1];
    s2 = getVarK(a, K[31]);
    s2 = getMemK(s2, K[102]);
    s2 = getMemK(s2, K[115]);
    setMemK(s0, K[180], s2);
    s0 = R[1];
    s2 = getVarK(a, K[31]);
    s2 = getMemK(s2, K[102]);
    s2 = getMemK(s2, K[178]);
    setMemK(s0, K[71], s2);
    s0 = R[1];
    s2 = getVarK(a, K[31]);
    s2 = getMemK(s2, K[102]);
    s2 = getMemK(s2, K[122]);
    setMemK(s0, K[122], s2);
    s0 = R[1];
    s2 = getVarK(a, K[31]);
    s2 = getMemK(s2, K[102]);
    s2 = getMemK(s2, K[179]);
    setMemK(s0, K[121], s2);
    s0 = R[1];
    s2 = getVarK(a, K[31]);
    s2 = getMemK(s2, K[102]);
    s2 = getMemK(s2, K[181]);
    setMemK(s0, K[181], s2);
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[182]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c1159a;
    s0 = R[1];
    setMemK(s0, K[183], VTRUE);
    goto L_1c115b7;
L_1c1159a:;
    s0 = R[1];
    s2 = getVarK(a, K[31]);
    s2 = getMemK(s2, K[102]);
    s2 = getMemK(s2, K[182]);
    setMemK(s0, K[183], s2);
L_1c115b7:;
    { s0 = callFunK(a, K[124], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    { s0 = callFunK(a, K[118], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[52]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c1160d;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { s2 = initArray(0, nullptr); }
    setMemK(s0, K[52], s2);
L_1c1160d:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[178]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c11645;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { s2 = initArray(0, nullptr); }
    setMemK(s0, K[178], s2);
L_1c11645:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[122]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c1167d;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { s2 = initArray(0, nullptr); }
    setMemK(s0, K[122], s2);
L_1c1167d:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[179]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c116b5;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { s2 = initArray(0, nullptr); }
    setMemK(s0, K[179], s2);
L_1c116b5:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[113]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c116ed;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { s2 = initArray(0, nullptr); }
    setMemK(s0, K[113], s2);
L_1c116ed:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[114]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c11775;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { Value av[10] = {Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0)}; s2 = initArray(10, av); }
    setMemK(s0, K[114], s2);
L_1c11775:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[115]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c117fd;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    { Value av[10] = {Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0)}; s2 = initArray(10, av); }
    setMemK(s0, K[115], s2);
L_1c117fd:;
    s0 = R[1];
    s2 = getVarK(a, K[31]);
    s2 = getMemK(s2, K[102]);
    s2 = getMemK(s2, K[52]);
    setMemK(s0, K[184], s2);
    s0 = R[1];
    s2 = getVarK(a, K[31]);
    s2 = getMemK(s2, K[102]);
    s2 = getMemK(s2, K[113]);
    setMemK(s0, K[185], s2);
    s0 = R[1];
    s2 = getVarK(a, K[31]);
    s2 = getMemK(s2, K[102]);
    s2 = getMemK(s2, K[114]);
    setMemK(s0, K[114], s2);
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f11_unlos(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = Value((Object*)(a.target ? doRoot(a.target) : (DisplayObject*)vm.root));
    Value s0, s1, s2, s3;
    Value p0;
    s0 = R[1];
    s0 = getMemK(s0, K[58]);
    s0 = getMemK(s0, K[17]);
    s0 = greater(s0, Value(2.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c1189e;
    s1 = R[1];
    s1 = getMemK(s1, K[58]);
    { s0 = callMethodK(a, s1, K[186], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c1189e:;
    s0 = R[1];
    s0 = getMemK(s0, K[58]);
    s0 = getMemK(s0, K[17]);
    s0 = VB(eq2(s0, Value(1.0)));
    s1 = R[1];
    s1 = getMemK(s1, K[58]);
    s1 = getMemK(s1, K[15]);
    s0 = orv(s0, s1);
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c118e4;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[58]);
    s0 = getMemK(s0, K[16]);
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c118e4:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c11a7f;
    s0 = R[1];
    s0 = getMemK(s0, K[58]);
    setMemK(s0, K[15], VFALSE);
    setVarK(a, K[51], Value(0.0));
    setVarK(a, K[48], Value(1000.0));
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    enumerate2(s0);
L_1c11928:;
    R[0] = vm.sp[-1];
    p0 = *--vm.sp;
    s0 = VB(eq2(p0, VNULL));
    if (truthy(s0)) goto L_1c119b0;
    s1 = R[0];
    setVarK(a, K[88], s1);
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s1 = getVarK(a, K[88]);
    s0 = getMem(s0, s1);
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c1196e;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[121]);
    s1 = getVarK(a, K[88]);
    s0 = getMem(s0, s1);
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c1196e:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c119ab;
    s1 = getVarK(a, K[88]);
    setVarK(a, K[187], s1);
    s0 = getVarK(a, K[187]);
    s1 = getVarK(a, K[48]);
    s0 = less2(s0, s1);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c119ab;
    s1 = getVarK(a, K[187]);
    setVarK(a, K[48], s1);
    s1 = getVarK(a, K[88]);
    setVarK(a, K[51], s1);
L_1c119ab:;
    goto L_1c11928;
L_1c119b0:;
    s0 = getVarK(a, K[51]);
    s0 = VB(eq2(s0, Value(0.0)));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c11a7f;
    s0 = R[1];
    s0 = getMemK(s0, K[58]);
    setMemK(s0, K[16], VTRUE);
    s2 = R[1];
    s2 = getMemK(s2, K[58]);
    { Value av[1] = {Value(4.0)}; s0 = callMethodK(a, s2, K[188], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = R[1];
    s0 = getMemK(s0, K[121]);
    s1 = getVarK(a, K[51]);
    setMem(s0, s1, VTRUE);
    s0 = getVarK(a, K[51]);
    s2 = R[1];
    s2 = getMemK(s2, K[122]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[123], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = R[1];
    s2 = getVarK(a, K[48]);
    setMemK(s0, K[14], s2);
    s0 = getVarK(a, K[189]);
    s1 = getVarK(a, K[51]);
    s0 = getMem(s0, s1);
    s0 = VB(eq2(s0, Value(0.0)));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c11a7f;
    s0 = getVarK(a, K[189]);
    s1 = getVarK(a, K[51]);
    s0 = getMem(s0, s1);
    { Value av[1] = {s0}; s0 = callFunK(a, K[190], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c11a7f:;
    s0 = R[1];
    s0 = getMemK(s0, K[17]);
    s0 = greater(s0, Value(2.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_end;
    s0 = R[1];
    s2 = getVarK(a, K[31]);
    s2 = getMemK(s2, K[102]);
    s2 = getMemK(s2, K[105]);
    setMemK(s0, K[191], s2);
    s0 = R[1];
    s0 = getMemK(s0, K[191]);
    s0 = less2(s0, Value(2.0));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c11aea;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    setMem(s0, Value(8.0), VTRUE);
L_1c11aea:;
    s0 = R[1];
    s0 = getMemK(s0, K[191]);
    s0 = less2(s0, Value(3.0));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c11b1e;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    setMem(s0, Value(9.0), VTRUE);
L_1c11b1e:;
    s0 = R[1];
    s0 = getMemK(s0, K[191]);
    s0 = less2(s0, Value(4.0));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c11b52;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    setMem(s0, Value(10.0), VTRUE);
L_1c11b52:;
    s0 = R[1];
    s0 = getMemK(s0, K[191]);
    s0 = less2(s0, Value(5.0));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c11b86;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    setMem(s0, Value(11.0), VTRUE);
L_1c11b86:;
    s0 = R[1];
    s0 = getMemK(s0, K[191]);
    s0 = less2(s0, Value(6.0));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c11bba;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    setMem(s0, Value(42.0), VTRUE);
L_1c11bba:;
    s0 = R[1];
    s0 = getMemK(s0, K[191]);
    s0 = less2(s0, Value(7.0));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c11bee;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    setMem(s0, Value(43.0), VTRUE);
L_1c11bee:;
    s0 = R[1];
    s0 = getMemK(s0, K[191]);
    s0 = less2(s0, Value(8.0));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c11c22;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    setMem(s0, Value(44.0), VTRUE);
L_1c11c22:;
    s0 = R[1];
    s0 = getMemK(s0, K[191]);
    s0 = less2(s0, Value(9.0));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c11c56;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    setMem(s0, Value(45.0), VTRUE);
L_1c11c56:;
    s0 = R[1];
    s0 = getMemK(s0, K[191]);
    s0 = less2(s0, Value(10.0));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c11ca3;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    setMem(s0, Value(12.0), VTRUE);
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    setMem(s0, Value(41.0), VTRUE);
L_1c11ca3:;
    setVarK(a, K[88], Value(0.0));
L_1c11cb1:;
    s0 = getVarK(a, K[88]);
    s0 = less2(s0, Value(3.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c11d0e;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s1 = getVarK(a, K[88]);
    s1 = add2(s1, Value(15.0));
    s2 = R[1];
    s2 = getMemK(s2, K[71]);
    s3 = getVarK(a, K[88]);
    s3 = add2(s3, Value(1.0));
    s2 = getMem(s2, s3);
    setMem(s0, s1, s2);
    s1 = getVarK(a, K[88]);
    s1 = incv(s1);
    setVarK(a, K[88], s1);
    goto L_1c11cb1;
L_1c11d0e:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(54.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_end;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    setMem(s0, Value(56.0), VTRUE);
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    setMem(s0, Value(57.0), VTRUE);
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f12_soundfra(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = Value((Object*)(a.target ? doRoot(a.target) : (DisplayObject*)vm.root));
    Value s0, s1, s2, s3, s4, s5;
    Value p0, p1;
    s0 = R[1];
    { s2 = initArray(0, nullptr); }
    setMemK(s0, K[125], s2);
    s2 = getVarK(a, K[193]);
    { Value av[1] = {Value(77.0)}; s0 = callMethodK(a, s2, K[194], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c11dd6;
    s0 = getVarK(a, K[195]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c11dd1;
    setVarK(a, K[195], VFALSE);
    s0 = R[1];
    s2 = R[1];
    s2 = getMemK(s2, K[176]);
    s2 = notv(s2);
    setMemK(s0, K[176], s2);
L_1c11dd1:;
    goto L_1c11de1;
L_1c11dd6:;
    setVarK(a, K[195], VTRUE);
L_1c11de1:;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c11e1c;
    s0 = R[1];
    s2 = R[1];
    s2 = getMemK(s2, K[165]);
    setMemK(s0, K[162], s2);
    s0 = R[1];
    setMemK(s0, K[165], VUNDEF);
L_1c11e1c:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s2 = R[1];
    s2 = getMemK(s2, K[176]);
    setMemK(s0, K[177], s2);
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = getMemK(s0, K[146]);
    s0 = VB(eq2(s0, Value(K[137])));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c11e78;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = getMemK(s0, K[63]);
    s0 = getMemK(s0, K[196]);
    s0 = greater(s0, Value(4000.0));
    *vm.sp++ = s0;
L_1c11e78:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c11e89;
    setVarK(a, K[163], VTRUE);
L_1c11e89:;
    s0 = R[1];
    s0 = getMemK(s0, K[3]);
    s0 = getMemK(s0, K[197]);
    s0 = greater(s0, Value(0.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c11f4e;
    s0 = R[1];
    s0 = getMemK(s0, K[198]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c11eea;
    s0 = R[1];
    s5 = R[1];
    { Value av[2] = {Value(K[199]), Value(100.0)}; s2 = callMethodK(a, s5, K[66], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setMemK(s0, K[198], s2);
L_1c11eea:;
    s0 = R[1];
    s0 = getMemK(s0, K[3]);
    s0 = getMemK(s0, K[197]);
    s0 = mul(s0, Value(30.0));
    s0 = add2(s0, Value(30.0));
    s3 = getVarK(a, K[80]);
    { Value av[2] = {Value(150.0), s0}; s0 = callMethodK(a, s3, K[81], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s2 = R[1];
    s2 = getMemK(s2, K[198]);
    s2 = getMemK(s2, K[63]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[174], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1c11f73;
L_1c11f4e:;
    s0 = R[1];
    s0 = getMemK(s0, K[198]);
    setMemK(s0, K[64], VTRUE);
    s0 = R[1];
    setMemK(s0, K[198], VUNDEF);
L_1c11f73:;
    s0 = R[1];
    s0 = getMemK(s0, K[200]);
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c11fbc;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[3]);
    s0 = getMemK(s0, K[201]);
    s0 = getMemK(s0, K[75]);
    s0 = greater(s0, Value(0.0));
    s1 = R[1];
    s1 = getMemK(s1, K[17]);
    s1 = VB(eq2(s1, Value(2.0)));
    s1 = notv(s1);
    s0 = orv(s0, s1);
    *vm.sp++ = s0;
L_1c11fbc:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c11fe2;
    s0 = R[1];
    setMemK(s0, K[200], VFALSE);
    s0 = R[1];
    setMemK(s0, K[163], VTRUE);
L_1c11fe2:;
    s0 = R[1];
    s0 = getMemK(s0, K[202]);
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c12001;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[203]);
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c12001:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c12027;
    s0 = R[1];
    setMemK(s0, K[202], VFALSE);
    s0 = R[1];
    setMemK(s0, K[163], VTRUE);
L_1c12027:;
    s0 = getVarK(a, K[17]);
    s0 = VB(eq2(s0, Value(2.0)));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c12055;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = getMemK(s0, K[146]);
    s0 = VB(eq2(s0, Value(K[156])));
    *vm.sp++ = s0;
L_1c12055:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c1206b;
    s0 = R[1];
    setMemK(s0, K[163], VFALSE);
L_1c1206b:;
    s0 = R[1];
    s0 = getMemK(s0, K[176]);
    s1 = R[1];
    s1 = getMemK(s1, K[163]);
    s0 = orv(s0, s1);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c12167;
    s2 = R[1];
    s2 = getMemK(s2, K[162]);
    s2 = getMemK(s2, K[63]);
    { s1 = callMethodK(a, s2, K[204], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setVarK(a, K[51], s1);
    s0 = getVarK(a, K[51]);
    s0 = greater(s0, Value(5.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c120f6;
    s0 = getVarK(a, K[51]);
    s0 = sub(s0, Value(5.0));
    s2 = R[1];
    s2 = getMemK(s2, K[162]);
    s2 = getMemK(s2, K[63]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[174], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1c12162;
L_1c120f6:;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    setMemK(s0, K[0], VTRUE);
    s0 = R[1];
    setMemK(s0, K[162], VUNDEF);
    s0 = R[1];
    setMemK(s0, K[163], VFALSE);
    s0 = getVarK(a, K[205]);
    s1 = getVarK(a, K[206]);
    s1 = VB(eq2(s1, Value(2.0)));
    s0 = orv(s0, s1);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c12157;
    setVarK(a, K[205], VFALSE);
    goto L_1c12162;
L_1c12157:;
    setVarK(a, K[206], VTRUE);
L_1c12162:;
    goto L_1c1311c;
L_1c12167:;
    s0 = getVarK(a, K[17]);
    s0 = VB(eq2(s0, Value(1.0)));
    s0 = notv(s0);
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c12194;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[17]);
    s0 = greater(s0, Value(5.0));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c12194:;
    s0 = getVarK(a, K[17]);
    s0 = VB(eq2(s0, Value(20.0)));
    p0 = *--vm.sp;
    s0 = orv(p0, s0);
    s1 = getVarK(a, K[17]);
    s1 = VB(eq2(s1, Value(21.0)));
    s0 = orv(s0, s1);
    s1 = getVarK(a, K[17]);
    s1 = VB(eq2(s1, Value(41.0)));
    s0 = orv(s0, s1);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c13111;
    s1 = R[1];
    s1 = getMemK(s1, K[17]);
    s1 = VB(eq2(s1, Value(2.0)));
    s2 = s1;
    s2 = notv(s2);
    *vm.sp++ = Value(K[207]);
    *vm.sp++ = s1;
    if (truthy(s2)) goto L_1c12202;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[136]);
    s1 = R[1];
    s1 = getMemK(s1, K[208]);
    s0 = VB(eq2(s0, s1));
    *vm.sp++ = s0;
L_1c12202:;
    p0 = *--vm.sp;
    p1 = *--vm.sp;
    setVar(a, p1, p0);
    s1 = R[1];
    s1 = getMemK(s1, K[17]);
    s1 = VB(eq2(s1, Value(2.0)));
    s2 = s1;
    s2 = notv(s2);
    *vm.sp++ = Value(K[209]);
    *vm.sp++ = s1;
    if (truthy(s2)) goto L_1c1223b;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[136]);
    s1 = R[1];
    s1 = getMemK(s1, K[210]);
    s0 = VB(eq2(s0, s1));
    *vm.sp++ = s0;
L_1c1223b:;
    *vm.sp = vm.sp[-1]; vm.sp++;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c1225b;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[136]);
    s1 = R[1];
    s1 = getMemK(s1, K[211]);
    s0 = VB(eq2(s0, s1));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c1225b:;
    p0 = *--vm.sp;
    p1 = *--vm.sp;
    setVar(a, p1, p0);
    s1 = R[1];
    s1 = getMemK(s1, K[17]);
    s1 = VB(eq2(s1, Value(2.0)));
    s2 = s1;
    s2 = notv(s2);
    *vm.sp++ = Value(K[212]);
    *vm.sp++ = s1;
    if (truthy(s2)) goto L_1c12291;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[136]);
    s0 = VB(eq2(s0, Value(166.0)));
    *vm.sp++ = s0;
L_1c12291:;
    *vm.sp = vm.sp[-1]; vm.sp++;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c122b1;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[213]);
    s1 = R[1];
    s1 = getMemK(s1, K[138]);
    s0 = VB(eq2(s0, s1));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c122b1:;
    p0 = *--vm.sp;
    p1 = *--vm.sp;
    setVar(a, p1, p0);
    s1 = R[1];
    s1 = getMemK(s1, K[17]);
    s1 = VB(eq2(s1, Value(2.0)));
    s2 = s1;
    s2 = notv(s2);
    *vm.sp++ = Value(K[214]);
    *vm.sp++ = s1;
    if (truthy(s2)) goto L_1c122ea;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[136]);
    s1 = R[1];
    s1 = getMemK(s1, K[215]);
    s0 = VB(eq2(s0, s1));
    *vm.sp++ = s0;
L_1c122ea:;
    *vm.sp = vm.sp[-1]; vm.sp++;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c1230a;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[136]);
    s1 = R[1];
    s1 = getMemK(s1, K[211]);
    s0 = VB(eq2(s0, s1));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c1230a:;
    p0 = *--vm.sp;
    p1 = *--vm.sp;
    setVar(a, p1, p0);
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = VB(eq2(s0, VUNDEF));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c12333;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[165]);
    s0 = VB(eq2(s0, VUNDEF));
    *vm.sp++ = s0;
L_1c12333:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c129a4;
    s1 = getVarK(a, K[216]);
    s1 = decv(s1);
    setVarK(a, K[216], s1);
    s0 = getVarK(a, K[214]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c1239b;
    s0 = getVarK(a, K[206]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c12382;
    { Value av[1] = {Value(K[217])}; s0 = callFunK(a, K[66], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setVarK(a, K[206], VFALSE);
    goto L_1c12396;
L_1c12382:;
    { Value av[1] = {Value(K[218])}; s0 = callFunK(a, K[66], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c12396:;
    goto L_1c1299f;
L_1c1239b:;
    s0 = getVarK(a, K[212]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c123f0;
    s0 = getVarK(a, K[206]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c123d7;
    { Value av[1] = {Value(K[219])}; s0 = callFunK(a, K[66], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setVarK(a, K[206], VFALSE);
    goto L_1c123eb;
L_1c123d7:;
    { Value av[1] = {Value(K[220])}; s0 = callFunK(a, K[66], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c123eb:;
    goto L_1c1299f;
L_1c123f0:;
    s0 = getVarK(a, K[209]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c12445;
    s0 = getVarK(a, K[206]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c1242c;
    { Value av[1] = {Value(K[221])}; s0 = callFunK(a, K[66], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setVarK(a, K[206], VFALSE);
    goto L_1c12440;
L_1c1242c:;
    { Value av[1] = {Value(K[222])}; s0 = callFunK(a, K[66], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c12440:;
    goto L_1c1299f;
L_1c12445:;
    s0 = getVarK(a, K[207]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c1249a;
    s0 = getVarK(a, K[206]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c12481;
    { Value av[1] = {Value(K[223])}; s0 = callFunK(a, K[66], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setVarK(a, K[206], VFALSE);
    goto L_1c12495;
L_1c12481:;
    { Value av[1] = {Value(K[224])}; s0 = callFunK(a, K[66], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c12495:;
    goto L_1c1299f;
L_1c1249a:;
    s0 = R[1];
    s0 = getMemK(s0, K[17]);
    s0 = VB(eq2(s0, Value(20.0)));
    s1 = R[1];
    s1 = getMemK(s1, K[17]);
    s1 = VB(eq2(s1, Value(21.0)));
    s0 = orv(s0, s1);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c1257c;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[105]);
    s0 = greater(s0, Value(9.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c12533;
    s0 = getVarK(a, K[206]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c1251a;
    { Value av[1] = {Value(K[225])}; s0 = callFunK(a, K[66], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setVarK(a, K[206], VFALSE);
    goto L_1c1252e;
L_1c1251a:;
    { Value av[1] = {Value(K[226])}; s0 = callFunK(a, K[66], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c1252e:;
    goto L_1c12577;
L_1c12533:;
    s0 = getVarK(a, K[206]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c12563;
    { Value av[1] = {Value(K[160])}; s0 = callFunK(a, K[66], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setVarK(a, K[206], VFALSE);
    goto L_1c12577;
L_1c12563:;
    { Value av[1] = {Value(K[161])}; s0 = callFunK(a, K[66], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c12577:;
    goto L_1c1299f;
L_1c1257c:;
    s0 = R[1];
    s0 = getMemK(s0, K[17]);
    s0 = VB(eq2(s0, Value(5.0)));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c125af;
    { Value av[1] = {Value(K[159])}; s0 = callFunK(a, K[66], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1c1299f;
L_1c125af:;
    s0 = R[1];
    s0 = getMemK(s0, K[140]);
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c125ce;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[200]);
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c125ce:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c1282f;
    s0 = R[1];
    s0 = getMemK(s0, K[3]);
    s0 = getMemK(s0, K[227]);
    s0 = greater(s0, Value(0.9));
    s1 = R[1];
    s1 = getMemK(s1, K[17]);
    s1 = VB(eq2(s1, Value(4.0)));
    s0 = orv(s0, s1);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c12740;
    s1 = R[1];
    s1 = getMemK(s1, K[136]);
    s2 = R[1];
    s2 = getMemK(s2, K[135]);
    s1 = VB(eq2(s1, s2));
    setVarK(a, K[51], s1);
    s0 = R[1];
    s0 = getMemK(s0, K[138]);
    s0 = VB(eq2(s0, Value(6.0)));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c1264c;
    --vm.sp;
    s0 = getVarK(a, K[51]);
    *vm.sp++ = s0;
L_1c1264c:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c1266b;
    { Value av[1] = {Value(K[228])}; s0 = callFunK(a, K[66], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1c1273b;
L_1c1266b:;
    s0 = R[1];
    s0 = getMemK(s0, K[138]);
    s0 = VB(eq2(s0, Value(8.0)));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c1268d;
    --vm.sp;
    s0 = getVarK(a, K[51]);
    *vm.sp++ = s0;
L_1c1268d:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c126ac;
    { Value av[1] = {Value(K[229])}; s0 = callFunK(a, K[66], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1c1273b;
L_1c126ac:;
    s0 = R[1];
    s0 = getMemK(s0, K[138]);
    s0 = VB(eq2(s0, Value(9.0)));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c126ce;
    --vm.sp;
    s0 = getVarK(a, K[51]);
    *vm.sp++ = s0;
L_1c126ce:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c126ed;
    { Value av[1] = {Value(K[230])}; s0 = callFunK(a, K[66], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1c1273b;
L_1c126ed:;
    s0 = R[1];
    s0 = getMemK(s0, K[206]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c12727;
    { Value av[1] = {Value(K[157])}; s0 = callFunK(a, K[66], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = R[1];
    setMemK(s0, K[206], VFALSE);
    goto L_1c1273b;
L_1c12727:;
    { Value av[1] = {Value(K[158])}; s0 = callFunK(a, K[66], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c1273b:;
    goto L_1c1282a;
L_1c12740:;
    s0 = R[1];
    s0 = getMemK(s0, K[206]);
    s0 = VB(eq2(s0, Value(2.0)));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c12773;
    { Value av[1] = {Value(K[231])}; s0 = callFunK(a, K[66], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1c1282a;
L_1c12773:;
    s0 = R[1];
    s0 = getMemK(s0, K[206]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c12816;
    s0 = R[1];
    s0 = getMemK(s0, K[3]);
    s0 = getMemK(s0, K[232]);
    s0 = greater(s0, Value(50.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c127fe;
    s0 = R[1];
    s0 = getMemK(s0, K[136]);
    s1 = R[1];
    s1 = getMemK(s1, K[135]);
    s0 = VB(eq2(s0, s1));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c127e2;
    { Value av[2] = {Value(K[153]), Value(35.0)}; s0 = callFunK(a, K[66], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1c127fe;
L_1c127e2:;
    { Value av[2] = {Value(K[154]), Value(85.0)}; s0 = callFunK(a, K[66], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c127fe:;
    s0 = R[1];
    setMemK(s0, K[206], Value(2.0));
    goto L_1c1282a;
L_1c12816:;
    { Value av[1] = {Value(K[231])}; s0 = callFunK(a, K[66], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c1282a:;
    goto L_1c1299f;
L_1c1282f:;
    s2 = R[1];
    s2 = getMemK(s2, K[138]);
    s2 = divv(s2, Value(2.0));
    s4 = getVarK(a, K[80]);
    { Value av[1] = {s2}; s2 = callMethodK(a, s4, K[233], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s4 = getVarK(a, K[80]);
    { Value av[2] = {s2, Value(4.0)}; s1 = callMethodK(a, s4, K[81], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setVarK(a, K[51], s1);
    s0 = getVarK(a, K[51]);
    s0 = greater(s0, Value(0.0));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c1289d;
    setVarK(a, K[51], Value(1.0));
L_1c1289d:;
    s0 = R[1];
    s0 = getMemK(s0, K[138]);
    s0 = VB(eq2(s0, Value(9.0)));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c128c5;
    setVarK(a, K[51], Value(19.0));
L_1c128c5:;
    s0 = R[1];
    s0 = getMemK(s0, K[200]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c128f9;
    s0 = R[1];
    setMemK(s0, K[140], VFALSE);
    setVarK(a, K[51], Value(7.0));
    goto L_1c12941;
L_1c128f9:;
    s0 = R[1];
    s0 = getMemK(s0, K[203]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c12941;
    setVarK(a, K[51], Value(6.0));
    s0 = R[1];
    s0 = getMemK(s0, K[136]);
    s1 = getVarK(a, K[234]);
    s0 = VB(eq2(s0, s1));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c12941;
    s0 = R[1];
    setMemK(s0, K[202], VTRUE);
L_1c12941:;
    s0 = getVarK(a, K[206]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c1297e;
    s1 = getVarK(a, K[51]);
    s0 = add2(Value(K[150]), s1);
    s0 = add2(s0, Value(K[152]));
    { Value av[1] = {s0}; s0 = callFunK(a, K[66], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setVarK(a, K[206], VFALSE);
    goto L_1c1299f;
L_1c1297e:;
    s1 = getVarK(a, K[51]);
    s0 = add2(Value(K[150]), s1);
    s0 = add2(s0, Value(K[151]));
    { Value av[1] = {s0}; s0 = callFunK(a, K[66], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c1299f:;
    goto L_1c12a6c;
L_1c129a4:;
    s0 = R[1];
    s0 = getMemK(s0, K[235]);
    s0 = VB(eq2(s0, VUNDEF));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c129e2;
    --vm.sp;
    s1 = R[1];
    s1 = getMemK(s1, K[162]);
    s1 = getMemK(s1, K[63]);
    { s0 = callMethodK(a, s1, K[204], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s1 = getVarK(a, K[169]);
    s0 = less2(s0, s1);
    *vm.sp++ = s0;
L_1c129e2:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c12a6c;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = getMemK(s0, K[236]);
    if (truthy(s0)) goto L_1c12a6c;
    s1 = getVarK(a, K[169]);
    s3 = R[1];
    s3 = getMemK(s3, K[162]);
    s3 = getMemK(s3, K[63]);
    { s2 = callMethodK(a, s3, K[204], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s2 = add2(s2, Value(1.0));
    s4 = getVarK(a, K[80]);
    { Value av[2] = {s2, s1}; s1 = callMethodK(a, s4, K[81], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setVarK(a, K[237], s1);
    s0 = getVarK(a, K[237]);
    s2 = R[1];
    s2 = getMemK(s2, K[162]);
    s2 = getMemK(s2, K[63]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[174], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c12a6c:;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = getMemK(s0, K[146]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c12a92;
    --vm.sp;
    s0 = getVarK(a, K[238]);
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c12a92:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c1310c;
    s0 = R[1];
    s0 = getMemK(s0, K[140]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c12c52;
    s0 = R[1];
    s0 = getMemK(s0, K[136]);
    s1 = R[1];
    s1 = getMemK(s1, K[135]);
    s0 = VB(eq2(s0, s1));
    s0 = notv(s0);
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c12ae1;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[136]);
    s1 = R[1];
    s1 = getMemK(s1, K[208]);
    s0 = VB(eq2(s0, s1));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c12ae1:;
    *vm.sp = vm.sp[-1]; vm.sp++;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c12b17;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[136]);
    s0 = VB(eq2(s0, Value(166.0)));
    s0 = notv(s0);
    s1 = R[1];
    s1 = getMemK(s1, K[213]);
    s2 = R[1];
    s2 = getMemK(s2, K[138]);
    s1 = VB(eq2(s1, s2));
    s1 = notv(s1);
    s0 = orv(s0, s1);
    *vm.sp++ = s0;
L_1c12b17:;
    *vm.sp = vm.sp[-1]; vm.sp++;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c12b37;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[136]);
    s1 = R[1];
    s1 = getMemK(s1, K[211]);
    s0 = VB(eq2(s0, s1));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c12b37:;
    *vm.sp = vm.sp[-1]; vm.sp++;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c12b57;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[136]);
    s1 = R[1];
    s1 = getMemK(s1, K[234]);
    s0 = VB(eq2(s0, s1));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c12b57:;
    *vm.sp = vm.sp[-1]; vm.sp++;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c12b74;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[17]);
    s0 = VB(eq2(s0, Value(4.0)));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c12b74:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c12b95;
    s0 = R[1];
    setMemK(s0, K[140], VFALSE);
    setVarK(a, K[163], VTRUE);
L_1c12b95:;
    s0 = R[1];
    s0 = getMemK(s0, K[3]);
    s0 = getMemK(s0, K[227]);
    s0 = VB(eq2(s0, Value(0.0)));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c12bcf;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = getMemK(s0, K[146]);
    s0 = VB(eq2(s0, Value(K[239])));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c12bcf:;
    *vm.sp = vm.sp[-1]; vm.sp++;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c12bef;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = getMemK(s0, K[146]);
    s0 = VB(eq2(s0, Value(K[231])));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c12bef:;
    *vm.sp = vm.sp[-1]; vm.sp++;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c12c11;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[3]);
    s0 = getMemK(s0, K[232]);
    s0 = greater(s0, Value(20.0));
    *vm.sp++ = s0;
L_1c12c11:;
    *vm.sp = vm.sp[-1]; vm.sp++;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c12c2e;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[140]);
    s0 = VB(eq2(s0, Value(2.0)));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c12c2e:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c12c52;
    s0 = R[1];
    setMemK(s0, K[140], Value(2.0));
    setVarK(a, K[163], VTRUE);
L_1c12c52:;
    s0 = getVarK(a, K[17]);
    s0 = VB(eq2(s0, Value(5.0)));
    s0 = notv(s0);
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c12c81;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = getMemK(s0, K[146]);
    s0 = VB(eq2(s0, Value(K[159])));
    *vm.sp++ = s0;
L_1c12c81:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c12c97;
    s0 = R[1];
    setMemK(s0, K[163], VTRUE);
L_1c12c97:;
    s0 = getVarK(a, K[17]);
    s0 = VB(eq2(s0, Value(5.0)));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c12cc6;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = getMemK(s0, K[146]);
    s0 = VB(eq2(s0, Value(K[159])));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c12cc6:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c12cdc;
    s0 = R[1];
    setMemK(s0, K[163], VTRUE);
L_1c12cdc:;
    s0 = getVarK(a, K[207]);
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c12d02;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = getMemK(s0, K[146]);
    s0 = VB(eq2(s0, Value(K[224])));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c12d02:;
    *vm.sp = vm.sp[-1]; vm.sp++;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c12d22;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = getMemK(s0, K[146]);
    s0 = VB(eq2(s0, Value(K[223])));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c12d22:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c12d38;
    s0 = R[1];
    setMemK(s0, K[163], VTRUE);
L_1c12d38:;
    s0 = getVarK(a, K[207]);
    s0 = notv(s0);
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c12d76;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = getMemK(s0, K[146]);
    s0 = VB(eq2(s0, Value(K[224])));
    s1 = R[1];
    s1 = getMemK(s1, K[162]);
    s1 = getMemK(s1, K[146]);
    s1 = VB(eq2(s1, Value(K[223])));
    s0 = orv(s0, s1);
    *vm.sp++ = s0;
L_1c12d76:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c12d8c;
    s0 = R[1];
    setMemK(s0, K[163], VTRUE);
L_1c12d8c:;
    s0 = getVarK(a, K[209]);
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c12db2;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = getMemK(s0, K[146]);
    s0 = VB(eq2(s0, Value(K[221])));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c12db2:;
    *vm.sp = vm.sp[-1]; vm.sp++;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c12dd2;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = getMemK(s0, K[146]);
    s0 = VB(eq2(s0, Value(K[222])));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c12dd2:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c12de8;
    s0 = R[1];
    setMemK(s0, K[163], VTRUE);
L_1c12de8:;
    s0 = getVarK(a, K[209]);
    s0 = notv(s0);
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c12e26;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = getMemK(s0, K[146]);
    s0 = VB(eq2(s0, Value(K[221])));
    s1 = R[1];
    s1 = getMemK(s1, K[162]);
    s1 = getMemK(s1, K[146]);
    s1 = VB(eq2(s1, Value(K[222])));
    s0 = orv(s0, s1);
    *vm.sp++ = s0;
L_1c12e26:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c12e47;
    s0 = R[1];
    setMemK(s0, K[163], VTRUE);
    setVarK(a, K[205], VTRUE);
L_1c12e47:;
    s0 = getVarK(a, K[212]);
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c12e6d;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = getMemK(s0, K[146]);
    s0 = VB(eq2(s0, Value(K[219])));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c12e6d:;
    *vm.sp = vm.sp[-1]; vm.sp++;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c12e8d;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = getMemK(s0, K[146]);
    s0 = VB(eq2(s0, Value(K[220])));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c12e8d:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c12ea3;
    s0 = R[1];
    setMemK(s0, K[163], VTRUE);
L_1c12ea3:;
    s0 = getVarK(a, K[212]);
    s0 = notv(s0);
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c12ee1;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = getMemK(s0, K[146]);
    s0 = VB(eq2(s0, Value(K[219])));
    s1 = R[1];
    s1 = getMemK(s1, K[162]);
    s1 = getMemK(s1, K[146]);
    s1 = VB(eq2(s1, Value(K[220])));
    s0 = orv(s0, s1);
    *vm.sp++ = s0;
L_1c12ee1:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c12ef7;
    s0 = R[1];
    setMemK(s0, K[163], VTRUE);
L_1c12ef7:;
    s0 = getVarK(a, K[214]);
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c12f1d;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = getMemK(s0, K[146]);
    s0 = VB(eq2(s0, Value(K[217])));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c12f1d:;
    *vm.sp = vm.sp[-1]; vm.sp++;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c12f3d;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = getMemK(s0, K[146]);
    s0 = VB(eq2(s0, Value(K[218])));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c12f3d:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c12f53;
    s0 = R[1];
    setMemK(s0, K[163], VTRUE);
L_1c12f53:;
    s0 = getVarK(a, K[214]);
    s0 = notv(s0);
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c12f91;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = getMemK(s0, K[146]);
    s0 = VB(eq2(s0, Value(K[217])));
    s1 = R[1];
    s1 = getMemK(s1, K[162]);
    s1 = getMemK(s1, K[146]);
    s1 = VB(eq2(s1, Value(K[218])));
    s0 = orv(s0, s1);
    *vm.sp++ = s0;
L_1c12f91:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c12fb2;
    s0 = R[1];
    setMemK(s0, K[163], VTRUE);
    setVarK(a, K[205], VTRUE);
L_1c12fb2:;
    s0 = getVarK(a, K[17]);
    s0 = VB(eq2(s0, Value(20.0)));
    s0 = notv(s0);
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c12fda;
    --vm.sp;
    s0 = getVarK(a, K[17]);
    s0 = VB(eq2(s0, Value(21.0)));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c12fda:;
    *vm.sp = vm.sp[-1]; vm.sp++;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c13041;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = getMemK(s0, K[146]);
    s0 = VB(eq2(s0, Value(K[160])));
    s1 = R[1];
    s1 = getMemK(s1, K[162]);
    s1 = getMemK(s1, K[146]);
    s1 = VB(eq2(s1, Value(K[161])));
    s0 = orv(s0, s1);
    s1 = R[1];
    s1 = getMemK(s1, K[162]);
    s1 = getMemK(s1, K[146]);
    s1 = VB(eq2(s1, Value(K[225])));
    s0 = orv(s0, s1);
    s1 = R[1];
    s1 = getMemK(s1, K[162]);
    s1 = getMemK(s1, K[146]);
    s1 = VB(eq2(s1, Value(K[226])));
    s0 = orv(s0, s1);
    *vm.sp++ = s0;
L_1c13041:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c13057;
    s0 = R[1];
    setMemK(s0, K[163], VTRUE);
L_1c13057:;
    s0 = getVarK(a, K[17]);
    s0 = VB(eq2(s0, Value(20.0)));
    s1 = getVarK(a, K[17]);
    s1 = VB(eq2(s1, Value(21.0)));
    s0 = orv(s0, s1);
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c130f6;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = getMemK(s0, K[146]);
    s0 = VB(eq2(s0, Value(K[160])));
    s0 = notv(s0);
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c130b6;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = getMemK(s0, K[146]);
    s0 = VB(eq2(s0, Value(K[161])));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c130b6:;
    *vm.sp = vm.sp[-1]; vm.sp++;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c130d6;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = getMemK(s0, K[146]);
    s0 = VB(eq2(s0, Value(K[225])));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c130d6:;
    *vm.sp = vm.sp[-1]; vm.sp++;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c130f6;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s0 = getMemK(s0, K[146]);
    s0 = VB(eq2(s0, Value(K[226])));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c130f6:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c1310c;
    s0 = R[1];
    setMemK(s0, K[163], VTRUE);
L_1c1310c:;
    goto L_1c1311c;
L_1c13111:;
    setVarK(a, K[163], VTRUE);
L_1c1311c:;
    s0 = getVarK(a, K[134]);
    enumerate2(s0);
L_1c13123:;
    R[0] = vm.sp[-1];
    p0 = *--vm.sp;
    s0 = VB(eq2(p0, VNULL));
    if (truthy(s0)) goto L_1c134c7;
    s1 = R[0];
    setVarK(a, K[100], s1);
    s1 = getVarK(a, K[134]);
    s2 = getVarK(a, K[100]);
    s1 = getMem(s1, s2);
    setVarK(a, K[50], s1);
    s0 = getVarK(a, K[50]);
    s0 = getMemK(s0, K[64]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c131d1;
    s2 = getVarK(a, K[50]);
    s2 = getMemK(s2, K[63]);
    { s1 = callMethodK(a, s2, K[204], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s1 = sub(s1, Value(20.0));
    setVarK(a, K[51], s1);
    s0 = getVarK(a, K[51]);
    s2 = getVarK(a, K[50]);
    s2 = getMemK(s2, K[63]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[174], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = getVarK(a, K[51]);
    s0 = less2(s0, Value(20.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c131d1;
    s0 = getVarK(a, K[50]);
    setMemK(s0, K[0], VTRUE);
L_1c131d1:;
    s0 = getVarK(a, K[50]);
    s0 = getMemK(s0, K[166]);
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c13211;
    --vm.sp;
    s0 = getVarK(a, K[50]);
    s0 = getMemK(s0, K[63]);
    s0 = getMemK(s0, K[240]);
    s1 = getVarK(a, K[241]);
    s0 = sub(s0, s1);
    s1 = getVarK(a, K[50]);
    s1 = getMemK(s1, K[63]);
    s1 = getMemK(s1, K[196]);
    s0 = less2(s0, s1);
    *vm.sp++ = s0;
L_1c13211:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c1325b;
    s1 = getVarK(a, K[50]);
    s1 = getMemK(s1, K[63]);
    { s0 = callMethodK(a, s1, K[62], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s1 = getVarK(a, K[167]);
    s3 = getVarK(a, K[50]);
    s3 = getMemK(s3, K[63]);
    { Value av[2] = {s1, Value(0.0)}; s0 = callMethodK(a, s3, K[168], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c1325b:;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s1 = getVarK(a, K[50]);
    s0 = VB(eq2(s0, s1));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c132a1;
    --vm.sp;
    s0 = getVarK(a, K[50]);
    s0 = getMemK(s0, K[63]);
    s0 = getMemK(s0, K[240]);
    s1 = getVarK(a, K[241]);
    s0 = sub(s0, s1);
    s1 = getVarK(a, K[50]);
    s1 = getMemK(s1, K[63]);
    s1 = getMemK(s1, K[196]);
    s0 = less2(s0, s1);
    *vm.sp++ = s0;
L_1c132a1:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c132e2;
    s1 = getVarK(a, K[50]);
    s1 = getMemK(s1, K[63]);
    { s0 = callMethodK(a, s1, K[62], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = getVarK(a, K[50]);
    setMemK(s0, K[0], VTRUE);
    s0 = R[1];
    setMemK(s0, K[162], VUNDEF);
L_1c132e2:;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s1 = getVarK(a, K[50]);
    s0 = VB(eq2(s0, s1));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c13308;
    --vm.sp;
    s0 = getVarK(a, K[50]);
    s0 = getMemK(s0, K[164]);
    *vm.sp++ = s0;
L_1c13308:;
    *vm.sp = vm.sp[-1]; vm.sp++;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c1333e;
    --vm.sp;
    s0 = getVarK(a, K[50]);
    s0 = getMemK(s0, K[63]);
    s0 = getMemK(s0, K[240]);
    s0 = sub(s0, Value(2500.0));
    s1 = getVarK(a, K[50]);
    s1 = getMemK(s1, K[63]);
    s1 = getMemK(s1, K[196]);
    s0 = less2(s0, s1);
    *vm.sp++ = s0;
L_1c1333e:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c13353;
    s0 = R[1];
    setMemK(s0, K[162], VUNDEF);
L_1c13353:;
    s0 = R[1];
    s0 = getMemK(s0, K[235]);
    s1 = getVarK(a, K[50]);
    s0 = VB(eq2(s0, s1));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c1339b;
    --vm.sp;
    s0 = getVarK(a, K[50]);
    s0 = getMemK(s0, K[63]);
    s0 = getMemK(s0, K[240]);
    s0 = sub(s0, Value(1000.0));
    s1 = getVarK(a, K[50]);
    s1 = getMemK(s1, K[63]);
    s1 = getMemK(s1, K[196]);
    s0 = less2(s0, s1);
    *vm.sp++ = s0;
L_1c1339b:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c133b0;
    s0 = R[1];
    setMemK(s0, K[235], VUNDEF);
L_1c133b0:;
    s0 = getVarK(a, K[50]);
    s0 = getMemK(s0, K[63]);
    s0 = getMemK(s0, K[240]);
    s1 = getVarK(a, K[50]);
    s1 = getMemK(s1, K[63]);
    s1 = getMemK(s1, K[196]);
    s0 = VB(eq2(s0, s1));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c133ea;
    --vm.sp;
    s0 = getVarK(a, K[50]);
    s0 = getMemK(s0, K[166]);
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c133ea:;
    *vm.sp = vm.sp[-1]; vm.sp++;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c133ff;
    --vm.sp;
    s0 = getVarK(a, K[50]);
    s0 = getMemK(s0, K[171]);
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c133ff:;
    s0 = getVarK(a, K[50]);
    s0 = getMemK(s0, K[0]);
    p0 = *--vm.sp;
    s0 = orv(p0, s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c134c2;
    s0 = R[1];
    s0 = getMemK(s0, K[235]);
    s1 = getVarK(a, K[50]);
    s0 = VB(eq2(s0, s1));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c13439;
    s0 = R[1];
    setMemK(s0, K[235], VUNDEF);
L_1c13439:;
    s0 = R[1];
    s0 = getMemK(s0, K[162]);
    s1 = getVarK(a, K[50]);
    s0 = VB(eq2(s0, s1));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c13460;
    s0 = R[1];
    setMemK(s0, K[162], VUNDEF);
L_1c13460:;
    s1 = getVarK(a, K[50]);
    s1 = getMemK(s1, K[63]);
    { s0 = callMethodK(a, s1, K[62], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s2 = getVarK(a, K[50]);
    { Value av[1] = {Value(100.0)}; s0 = callMethodK(a, s2, K[242], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = getVarK(a, K[50]);
    actRemoveSprite(a, s0);
    s1 = getVarK(a, K[100]);
    s3 = getVarK(a, K[134]);
    { Value av[2] = {s1, Value(1.0)}; s0 = callMethodK(a, s3, K[243], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c134c2:;
    goto L_1c13123;
L_1c134c7:;
    s0 = randomNumber(Value(1000.0));
    s0 = VB(eq2(s0, Value(0.0)));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c135a3;
    { s1 = initArray(0, nullptr); }
    setVarK(a, K[51], s1);
    setVarK(a, K[100], Value(0.0));
L_1c134fc:;
    s0 = getVarK(a, K[100]);
    s1 = getVarK(a, K[134]);
    s1 = getMemK(s1, K[69]);
    s0 = less2(s0, s1);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c13541;
    s0 = getVarK(a, K[51]);
    s1 = getVarK(a, K[100]);
    s2 = getVarK(a, K[134]);
    s3 = getVarK(a, K[100]);
    s2 = getMem(s2, s3);
    setMem(s0, s1, s2);
    s1 = getVarK(a, K[100]);
    s1 = incv(s1);
    setVarK(a, K[100], s1);
    goto L_1c134fc;
L_1c13541:;
    { s1 = initArray(0, nullptr); }
    setVarK(a, K[134], s1);
    setVarK(a, K[100], Value(0.0));
L_1c1355e:;
    s0 = getVarK(a, K[100]);
    s1 = getVarK(a, K[51]);
    s1 = getMemK(s1, K[69]);
    s0 = less2(s0, s1);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c135a3;
    s0 = getVarK(a, K[134]);
    s1 = getVarK(a, K[100]);
    s2 = getVarK(a, K[51]);
    s3 = getVarK(a, K[100]);
    s2 = getMem(s2, s3);
    setMem(s0, s1, s2);
    s1 = getVarK(a, K[100]);
    s1 = incv(s1);
    setVarK(a, K[100], s1);
    goto L_1c1355e;
L_1c135a3:;
    s0 = R[1];
    s0 = getMemK(s0, K[235]);
    s0 = VB(eq2(s0, VUNDEF));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_end;
    s1 = R[1];
    s1 = getMemK(s1, K[162]);
    s1 = getMemK(s1, K[63]);
    { s0 = callMethodK(a, s1, K[204], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = sub(s0, Value(7.0));
    s3 = getVarK(a, K[80]);
    { Value av[2] = {Value(5.0), s0}; s0 = callMethodK(a, s3, K[244], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s2 = R[1];
    s2 = getMemK(s2, K[162]);
    s2 = getMemK(s2, K[63]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[174], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f13_prelod(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = Value((Object*)(a.target ? doRoot(a.target) : (DisplayObject*)vm.root));
    Value s0, s1, s2, s3, s4, s5;
    s2 = R[1];
    { s1 = callMethodK(a, s2, K[247], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setVarK(a, K[246], s1);
    s2 = R[1];
    { s1 = callMethodK(a, s2, K[249], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setVarK(a, K[248], s1);
    s1 = getVarK(a, K[246]);
    s2 = getVarK(a, K[248]);
    s1 = divv(s1, s2);
    s1 = mul(s1, Value(100.0));
    s3 = getVarK(a, K[80]);
    { Value av[1] = {s1}; s1 = callMethodK(a, s3, K[233], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setVarK(a, K[250], s1);
    s1 = getVarK(a, K[250]);
    s2 = getVarK(a, K[252]);
    s1 = sub(s1, s2);
    s1 = mul(s1, Value(0.5));
    setVarK(a, K[251], s1);
    setVarK(a, K[253], Value(100.0));
    s2 = getVarK(a, K[193]);
    { Value av[1] = {Value(65.0)}; s0 = callMethodK(a, s2, K[194], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c136f6;
    s1 = getVarK(a, K[253]);
    s1 = add2(s1, Value(5.0));
    setVarK(a, K[253], s1);
L_1c136f6:;
    s1 = getVarK(a, K[252]);
    s2 = getVarK(a, K[251]);
    s3 = getVarK(a, K[253]);
    s5 = getVarK(a, K[80]);
    { Value av[2] = {s3, s2}; s2 = callMethodK(a, s5, K[81], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s1 = add2(s1, s2);
    setVarK(a, K[252], s1);
    s0 = getVarK(a, K[17]);
    s0 = VB(eq2(s0, Value(1.0)));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c13743;
    setVarK(a, K[254], VUNDEF);
L_1c13743:;
    s0 = getVarK(a, K[252]);
    s0 = greater(s0, Value(99.5));
    s0 = notv(s0);
    if (truthy(s0)) goto L_end;
    s2 = getVarK(a, K[255]);
    { s1 = callMethodK(a, s2, K[256], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setVarK(a, K[250], s1);
    s2 = R[1];
    { Value av[1] = {Value(K[206])}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f14_fullf(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = Value((Object*)(a.target ? doRoot(a.target) : (DisplayObject*)vm.root));
    R[2] = argc > 0 ? argv[0] : VUNDEF;
    Value s0, s1, s2, s3, s4;
    Value p0;
    s0 = getVarK(a, K[83]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c13874;
    setVarK(a, K[183], VTRUE);
    s0 = R[1];
    s2 = R[1];
    R[0] = Value(125.0);
    setMemK(s2, K[258], Value(125.0));
    s2 = R[0];
    setMemK(s0, K[257], s2);
    s1 = R[1];
    s1 = getMemK(s1, K[3]);
    { s0 = callMethodK(a, s1, K[259], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = R[1];
    s3 = R[1];
    s3 = getMemK(s3, K[258]);
    s3 = divv(s3, Value(125.0));
    s2 = sub(Value(1.0), s3);
    s2 = mul(s2, Value(800.0));
    s2 = divv(s2, Value(2.0));
    setMemK(s0, K[260], s2);
    s0 = R[1];
    s3 = R[1];
    s3 = getMemK(s3, K[257]);
    s3 = divv(s3, Value(125.0));
    s2 = sub(Value(1.0), s3);
    s2 = mul(s2, Value(600.0));
    s2 = divv(s2, Value(2.0));
    setMemK(s0, K[261], s2);
    goto L_end;
L_1c13874:;
    s1 = getVarK(a, K[232]);
    setVarK(a, K[262], s1);
    s0 = R[2];
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c138b0;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s2 = getVarK(a, K[31]);
    s2 = getMemK(s2, K[102]);
    s2 = getMemK(s2, K[263]);
    s2 = notv(s2);
    setMemK(s0, K[263], s2);
L_1c138b0:;
    actGetURL(a, "FSCommand:allowscale", "true");
    s0 = R[1];
    s0 = getMemK(s0, K[181]);
    s0 = greater(s0, Value(0.0));
    s0 = notv(s0);
    s1 = getVarK(a, K[264]);
    s1 = notv(s1);
    s0 = orv(s0, s1);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c139ad;
    s0 = getVarK(a, K[265]);
    if (truthy(s0)) goto L_1c13940;
    s0 = getVarK(a, K[266]);
    s0 = greater(s0, Value(1000.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c13928;
    s0 = R[1];
    setMemK(s0, K[181], Value(6.0));
    goto L_1c1393b;
L_1c13928:;
    s0 = R[1];
    setMemK(s0, K[181], Value(1.0));
L_1c1393b:;
    goto L_1c139ad;
L_1c13940:;
    s0 = getVarK(a, K[266]);
    s0 = greater(s0, Value(1200.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c1399a;
    s0 = getVarK(a, K[265]);
    s0 = VB(eq2(s0, Value(2.0)));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c13982;
    s0 = R[1];
    setMemK(s0, K[181], Value(5.0));
    goto L_1c13995;
L_1c13982:;
    s0 = R[1];
    setMemK(s0, K[181], Value(4.0));
L_1c13995:;
    goto L_1c139ad;
L_1c1399a:;
    s0 = R[1];
    setMemK(s0, K[181], Value(2.0));
L_1c139ad:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[263]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c13b80;
    s1 = getVarK(a, K[267]);
    s2 = R[1];
    s2 = getMemK(s2, K[181]);
    s1 = getMem(s1, s2);
    s1 = getMem(s1, Value(0.0));
    setVarK(a, K[47], s1);
    s1 = getVarK(a, K[267]);
    s2 = R[1];
    s2 = getMemK(s2, K[181]);
    s1 = getMem(s1, s2);
    s1 = getMem(s1, Value(1.0));
    setVarK(a, K[48], s1);
    s2 = getVarK(a, K[38]);
    s2 = getMemK(s2, K[268]);
    s2 = getMemK(s2, K[269]);
    { Value av[1] = {VTRUE}; s0 = callMethodK(a, s2, K[270], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = getVarK(a, K[264]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c13a92;
    s0 = getVarK(a, K[271]);
    s1 = getVarK(a, K[48]);
    s2 = getVarK(a, K[47]);
    s4 = getVarK(a, K[38]);
    s4 = getMemK(s4, K[272]);
    s4 = getMemK(s4, K[273]);
    { Value av[3] = {s2, s1, s0}; s0 = callMethodK(a, s4, K[274], 3, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s2 = getVarK(a, K[38]);
    s2 = getMemK(s2, K[268]);
    s2 = getMemK(s2, K[269]);
    { Value av[1] = {VTRUE}; s0 = callMethodK(a, s2, K[275], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c13a92:;
    s0 = getVarK(a, K[47]);
    s0 = less2(s0, Value(1000.0));
    s0 = notv(s0);
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c13aba;
    --vm.sp;
    s0 = getVarK(a, K[48]);
    s0 = less2(s0, Value(800.0));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c13aba:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c13adc;
    s0 = R[1];
    setMemK(s0, K[258], Value(166.7));
    goto L_1c13b7b;
L_1c13adc:;
    s0 = getVarK(a, K[47]);
    s0 = less2(s0, Value(800.0));
    s0 = notv(s0);
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c13b04;
    --vm.sp;
    s0 = getVarK(a, K[48]);
    s0 = less2(s0, Value(700.0));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c13b04:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c13b22;
    s0 = R[1];
    setMemK(s0, K[258], Value(154.0));
    goto L_1c13b7b;
L_1c13b22:;
    s0 = getVarK(a, K[47]);
    s0 = less2(s0, Value(720.0));
    s0 = notv(s0);
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c13b4a;
    --vm.sp;
    s0 = getVarK(a, K[48]);
    s0 = less2(s0, Value(600.0));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c13b4a:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c13b68;
    s0 = R[1];
    setMemK(s0, K[258], Value(125.0));
    goto L_1c13b7b;
L_1c13b68:;
    s0 = R[1];
    setMemK(s0, K[258], Value(100.0));
L_1c13b7b:;
    goto L_1c13d3c;
L_1c13b80:;
    s2 = getVarK(a, K[38]);
    s2 = getMemK(s2, K[268]);
    s2 = getMemK(s2, K[269]);
    { Value av[1] = {VFALSE}; s0 = callMethodK(a, s2, K[270], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s1 = getVarK(a, K[38]);
    s1 = getMemK(s1, K[39]);
    { s0 = callMethodK(a, s1, K[276], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = getVarK(a, K[264]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c13c14;
    s1 = getVarK(a, K[38]);
    s1 = getMemK(s1, K[272]);
    s1 = getMemK(s1, K[273]);
    { s0 = callMethodK(a, s1, K[277], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s2 = getVarK(a, K[38]);
    s2 = getMemK(s2, K[268]);
    s2 = getMemK(s2, K[269]);
    { Value av[1] = {VFALSE}; s0 = callMethodK(a, s2, K[275], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c13c14:;
    s0 = getVarK(a, K[183]);
    s0 = VB(eq2(s0, Value(2.0)));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c13c85;
    s0 = getVarK(a, K[38]);
    s0 = getMemK(s0, K[268]);
    s0 = getMemK(s0, K[269]);
    setMemK(s0, K[278], Value(1024.0));
    s0 = getVarK(a, K[38]);
    s0 = getMemK(s0, K[268]);
    s0 = getMemK(s0, K[269]);
    setMemK(s0, K[279], Value(800.0));
    s0 = R[1];
    setMemK(s0, K[258], Value(163.5));
    goto L_1c13d3c;
L_1c13c85:;
    s0 = getVarK(a, K[183]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c13ce9;
    s0 = getVarK(a, K[38]);
    s0 = getMemK(s0, K[268]);
    s0 = getMemK(s0, K[269]);
    setMemK(s0, K[278], Value(800.0));
    s0 = getVarK(a, K[38]);
    s0 = getMemK(s0, K[268]);
    s0 = getMemK(s0, K[269]);
    setMemK(s0, K[279], Value(620.0));
    s0 = R[1];
    setMemK(s0, K[258], Value(125.0));
    goto L_1c13d3c;
L_1c13ce9:;
    s0 = getVarK(a, K[38]);
    s0 = getMemK(s0, K[268]);
    s0 = getMemK(s0, K[269]);
    setMemK(s0, K[278], Value(640.0));
    s0 = getVarK(a, K[38]);
    s0 = getMemK(s0, K[268]);
    s0 = getMemK(s0, K[269]);
    setMemK(s0, K[279], Value(510.0));
    s0 = R[1];
    setMemK(s0, K[258], Value(100.0));
L_1c13d3c:;
    s0 = R[1];
    setMemK(s0, K[258], Value(56.6667));
    s0 = R[1];
    s2 = R[1];
    s2 = getMemK(s2, K[258]);
    setMemK(s0, K[257], s2);
    s1 = R[1];
    s1 = getMemK(s1, K[3]);
    { s0 = callMethodK(a, s1, K[259], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = R[1];
    s4 = R[1];
    s4 = getMemK(s4, K[258]);
    s3 = mul(Value(640.0), s4);
    s3 = divv(s3, Value(100.0));
    s2 = sub(Value(363.0), s3);
    s2 = divv(s2, Value(2.0));
    setMemK(s0, K[260], s2);
    s0 = R[1];
    s4 = R[1];
    s4 = getMemK(s4, K[257]);
    s3 = mul(Value(480.0), s4);
    s3 = divv(s3, Value(100.0));
    s2 = sub(Value(272.0), s3);
    s2 = divv(s2, Value(2.0));
    setMemK(s0, K[261], s2);
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[263]);
    if (truthy(s0)) goto L_end;
    s1 = getVarK(a, K[38]);
    s1 = getMemK(s1, K[272]);
    { s0 = callMethodK(a, s1, K[280], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    R[2] = s0;
    s0 = getVarK(a, K[38]);
    s0 = getMemK(s0, K[268]);
    s0 = getMemK(s0, K[269]);
    s2 = R[2];
    s2 = getMem(s2, Value(0.0));
    s3 = getVarK(a, K[38]);
    s3 = getMemK(s3, K[268]);
    s3 = getMemK(s3, K[269]);
    s3 = getMemK(s3, K[278]);
    s2 = sub(s2, s3);
    s2 = divv(s2, Value(2.0));
    s2 = sub(s2, Value(2.0));
    setMemK(s0, K[281], s2);
    s0 = getVarK(a, K[38]);
    s0 = getMemK(s0, K[268]);
    s0 = getMemK(s0, K[269]);
    s2 = R[2];
    s2 = getMem(s2, Value(1.0));
    s3 = getVarK(a, K[38]);
    s3 = getMemK(s3, K[268]);
    s3 = getMemK(s3, K[269]);
    s3 = getMemK(s3, K[279]);
    s2 = sub(s2, s3);
    s2 = divv(s2, Value(2.0));
    s2 = sub(s2, Value(20.0));
    setMemK(s0, K[282], s2);
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f15_callit(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = argc > 0 ? argv[0] : VUNDEF;
    Value s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
    R[2] = VUNDEF;
    R[3] = VUNDEF;
    s0 = getVarK(a, K[83]);
    if (truthy(s0)) goto L_end;
    s0 = getVarK(a, K[38]);
    s0 = getMemK(s0, K[39]);
    s0 = getMemK(s0, K[40]);
    R[2] = s0;
    s0 = getVarK(a, K[264]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c13f98;
    s2 = R[2];
    s3 = R[2];
    s3 = add2(s3, Value(K[284]));
    s4 = R[1];
    s3 = add2(s3, s4);
    s11 = getVarK(a, K[38]);
    s11 = getMemK(s11, K[286]);
    { Value av[10] = {Value(K[285]), Value(0.0), Value(0.0), Value(500.0), Value(600.0), Value(K[1]), s3, s2, Value(2.0), Value(4.0)}; s0 = callMethodK(a, s11, K[287], 10, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    R[3] = s0;
    goto L_end;
L_1c13f98:;
    s0 = getVarK(a, K[288]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c13fd3;
    s0 = R[1];
    s2 = getVarK(a, K[38]);
    s2 = getMemK(s2, K[286]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[289], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    R[1] = Value(K[1]);
    goto L_1c13fe3;
L_1c13fd3:;
    s1 = R[1];
    s0 = add2(Value(K[290]), s1);
    R[1] = s0;
L_1c13fe3:;
    s1 = getVarK(a, K[288]);
    s1 = notv(s1);
    setVarK(a, K[288], s1);
    s2 = R[2];
    s3 = R[2];
    s3 = add2(s3, Value(K[291]));
    s4 = R[1];
    s3 = add2(s3, s4);
    s11 = getVarK(a, K[38]);
    s11 = getMemK(s11, K[286]);
    { Value av[10] = {Value(K[285]), Value(0.0), Value(0.0), Value(500.0), Value(600.0), Value(K[1]), s3, s2, Value(2.0), Value(4.0)}; s0 = callMethodK(a, s11, K[287], 10, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    R[3] = s0;
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f16_doit(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = Value((Object*)(a.target ? doRoot(a.target) : (DisplayObject*)vm.root));
    R[2] = argc > 0 ? argv[0] : VUNDEF;
    Value s0, s1, s2;
    s0 = R[2];
    s0 = getMemK(s0, K[292]);
    R[0] = s0;
    s0 = VB(strictEq(s0, Value(K[293])));
    if (truthy(s0)) goto L_1c140d2;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(K[294])));
    if (truthy(s0)) goto L_1c140fc;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(K[295])));
    if (truthy(s0)) goto L_1c14126;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(K[296])));
    if (truthy(s0)) goto L_1c1413f;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(K[297])));
    if (truthy(s0)) goto L_1c14169;
    goto L_1c14180;
L_1c140d2:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s2 = getVarK(a, K[31]);
    s2 = getMemK(s2, K[102]);
    s2 = getMemK(s2, K[103]);
    s2 = notv(s2);
    setMemK(s0, K[103], s2);
    goto L_end;
L_1c140fc:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s2 = getVarK(a, K[31]);
    s2 = getMemK(s2, K[102]);
    s2 = getMemK(s2, K[104]);
    s2 = notv(s2);
    setMemK(s0, K[104], s2);
    goto L_end;
L_1c14126:;
    { Value av[1] = {VTRUE}; s0 = callFunK(a, K[283], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_end;
L_1c1413f:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s2 = getVarK(a, K[31]);
    s2 = getMemK(s2, K[102]);
    s2 = getMemK(s2, K[126]);
    s2 = notv(s2);
    setMemK(s0, K[126], s2);
    goto L_end;
L_1c14169:;
    s0 = R[1];
    s2 = R[1];
    s2 = getMemK(s2, K[176]);
    s2 = notv(s2);
    setMemK(s0, K[176], s2);
L_1c14180:;
    { Value rv_ = VUNDEF; vm.sp = sp0; return rv_; }
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f17_wiq(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = Value((Object*)(a.target ? doRoot(a.target) : (DisplayObject*)vm.root));
    R[2] = argc > 0 ? argv[0] : VUNDEF;
    Value s0, s1, s2, s3, s4;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s2 = R[1];
    s4 = R[2];
    R[0] = s4;
    setMemK(s2, K[183], s4);
    s2 = R[0];
    setMemK(s0, K[182], s2);
    { s0 = callFunK(a, K[23], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    { s0 = callFunK(a, K[283], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f18_reser(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = Value((Object*)(a.target ? doRoot(a.target) : (DisplayObject*)vm.root));
    R[2] = argc > 0 ? argv[0] : VUNDEF;
    Value s0, s1, s2, s3, s4;
    s0 = getVarK(a, K[264]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_end;
    s1 = getVarK(a, K[267]);
    s2 = R[2];
    s1 = getMem(s1, s2);
    setVarK(a, K[47], s1);
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s2 = R[1];
    s4 = R[2];
    R[0] = s4;
    setMemK(s2, K[181], s4);
    s2 = R[0];
    setMemK(s0, K[181], s2);
    { s0 = callFunK(a, K[23], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[263]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_end;
    { s0 = callFunK(a, K[283], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f19_showit(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = Value((Object*)(a.target ? doRoot(a.target) : (DisplayObject*)vm.root));
    Value s0, s1, s2, s3;
    s1 = R[1];
    s1 = getMemK(s1, K[298]);
    setVarK(a, K[50], s1);
    s2 = R[1];
    s2 = getMemK(s2, K[298]);
    s2 = getMemK(s2, K[295]);
    { Value av[1] = {Value(1.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s2 = R[1];
    s2 = getMemK(s2, K[298]);
    s2 = getMemK(s2, K[296]);
    { Value av[1] = {Value(1.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s2 = R[1];
    s2 = getMemK(s2, K[298]);
    s2 = getMemK(s2, K[297]);
    { Value av[1] = {Value(1.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s2 = R[1];
    s2 = getMemK(s2, K[298]);
    s2 = getMemK(s2, K[294]);
    { Value av[1] = {Value(1.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s2 = R[1];
    s2 = getMemK(s2, K[298]);
    s2 = getMemK(s2, K[293]);
    { Value av[1] = {Value(1.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s2 = getVarK(a, K[50]);
    s2 = getMemK(s2, K[299]);
    { Value av[1] = {Value(2.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s2 = getVarK(a, K[50]);
    s2 = getMemK(s2, K[300]);
    { Value av[1] = {Value(2.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s2 = getVarK(a, K[50]);
    s2 = getMemK(s2, K[301]);
    { Value av[1] = {Value(2.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = R[1];
    s0 = getMemK(s0, K[183]);
    s0 = VB(eq2(s0, Value(2.0)));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c14408;
    s2 = getVarK(a, K[50]);
    s2 = getMemK(s2, K[299]);
    { Value av[1] = {Value(1.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1c14464;
L_1c14408:;
    s0 = R[1];
    s0 = getMemK(s0, K[183]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c14441;
    s2 = getVarK(a, K[50]);
    s2 = getMemK(s2, K[300]);
    { Value av[1] = {Value(1.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1c14464;
L_1c14441:;
    s2 = getVarK(a, K[50]);
    s2 = getMemK(s2, K[301]);
    { Value av[1] = {Value(1.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c14464:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[263]);
    if (truthy(s0)) goto L_1c144a3;
    s2 = R[1];
    s2 = getMemK(s2, K[298]);
    s2 = getMemK(s2, K[295]);
    { Value av[1] = {Value(2.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c144a3:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[126]);
    if (truthy(s0)) goto L_1c144e2;
    s2 = R[1];
    s2 = getMemK(s2, K[298]);
    s2 = getMemK(s2, K[296]);
    { Value av[1] = {Value(2.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c144e2:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[103]);
    if (truthy(s0)) goto L_1c14521;
    s2 = R[1];
    s2 = getMemK(s2, K[298]);
    s2 = getMemK(s2, K[293]);
    { Value av[1] = {Value(2.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c14521:;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[104]);
    if (truthy(s0)) goto L_1c14560;
    s2 = R[1];
    s2 = getMemK(s2, K[298]);
    s2 = getMemK(s2, K[294]);
    { Value av[1] = {Value(2.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c14560:;
    s0 = R[1];
    s0 = getMemK(s0, K[176]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c14599;
    s2 = R[1];
    s2 = getMemK(s2, K[298]);
    s2 = getMemK(s2, K[297]);
    { Value av[1] = {Value(2.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c14599:;
    s0 = R[1];
    s0 = getMemK(s0, K[181]);
    s2 = getVarK(a, K[50]);
    s2 = getMemK(s2, K[302]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setVarK(a, K[100], Value(1.0));
L_1c145cd:;
    s0 = getVarK(a, K[100]);
    s0 = less2(s0, Value(8.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_end;
    s1 = getVarK(a, K[50]);
    s3 = getVarK(a, K[100]);
    s2 = add2(Value(K[304]), s3);
    s1 = getMem(s1, s2);
    setVarK(a, K[303], s1);
    s0 = getVarK(a, K[100]);
    s2 = getVarK(a, K[303]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = getVarK(a, K[303]);
    s0 = getMemK(s0, K[305]);
    s2 = getVarK(a, K[100]);
    s3 = R[1];
    s3 = getMemK(s3, K[181]);
    s2 = VB(eq2(s2, s3));
    setMemK(s0, K[56], s2);
    s1 = getVarK(a, K[100]);
    s1 = incv(s1);
    setVarK(a, K[100], s1);
    goto L_1c145cd;
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f20_qshow(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = Value((Object*)(a.target ? doRoot(a.target) : (DisplayObject*)vm.root));
    Value s0, s1, s2, s3;
    Value p0, p1;
    { Value av[4] = {Value(K[24]), Value(K[26]), Value(K[28]), Value(K[27])}; s1 = initArray(4, av); }
    setVarK(a, K[51], s1);
    setVarK(a, K[88], Value(0.0));
L_1c14690:;
    s0 = getVarK(a, K[88]);
    s0 = less2(s0, Value(4.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_end;
    s1 = R[1];
    s1 = getMemK(s1, K[298]);
    s3 = getVarK(a, K[88]);
    s2 = add2(Value(K[306]), s3);
    s1 = getMem(s1, s2);
    setVarK(a, K[50], s1);
    s0 = getVarK(a, K[51]);
    s1 = getVarK(a, K[88]);
    s0 = getMem(s0, s1);
    s1 = getVarK(a, K[307]);
    s0 = VB(eq2(s0, s1));
    s1 = getVarK(a, K[88]);
    s1 = VB(eq2(s1, Value(3.0)));
    s2 = s1;
    s2 = notv(s2);
    *vm.sp++ = s0;
    *vm.sp++ = s1;
    if (truthy(s2)) goto L_1c146fc;
    --vm.sp;
    s0 = getVarK(a, K[307]);
    s0 = VB(eq2(s0, VUNDEF));
    *vm.sp++ = s0;
L_1c146fc:;
    p0 = *--vm.sp;
    p1 = *--vm.sp;
    s0 = orv(p1, p0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c14725;
    s2 = getVarK(a, K[50]);
    { Value av[1] = {Value(1.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1c14742;
L_1c14725:;
    s2 = getVarK(a, K[50]);
    { Value av[1] = {Value(2.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c14742:;
    s1 = getVarK(a, K[88]);
    s1 = incv(s1);
    setVarK(a, K[88], s1);
    goto L_1c14690;
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f21_qua(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = Value((Object*)(a.target ? doRoot(a.target) : (DisplayObject*)vm.root));
    R[2] = argc > 0 ? argv[0] : VUNDEF;
    Value s0, s1, s2;
    s1 = getVarK(a, K[31]);
    s1 = getMemK(s1, K[102]);
    s1 = getMemK(s1, K[175]);
    setVarK(a, K[307], s1);
    s0 = R[2];
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c1479b;
    s1 = R[2];
    setVarK(a, K[307], s1);
    goto L_1c14818;
L_1c1479b:;
    s0 = getVarK(a, K[307]);
    s0 = VB(eq2(s0, Value(K[27])));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c147c1;
    setVarK(a, K[307], Value(K[24]));
    goto L_1c14818;
L_1c147c1:;
    s0 = getVarK(a, K[307]);
    s0 = VB(eq2(s0, Value(K[26])));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c147e6;
    setVarK(a, K[307], Value(K[28]));
    goto L_1c14818;
L_1c147e6:;
    s0 = getVarK(a, K[307]);
    s0 = VB(eq2(s0, Value(K[28])));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c1480c;
    setVarK(a, K[307], Value(K[27]));
    goto L_1c14818;
L_1c1480c:;
    setVarK(a, K[307], Value(K[26]));
L_1c14818:;
    s0 = getVarK(a, K[307]);
    s0 = VB(eq2(s0, Value(K[27])));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c1483d;
    setVarK(a, K[308], Value(K[26]));
    goto L_1c1484b;
L_1c1483d:;
    s1 = getVarK(a, K[307]);
    setVarK(a, K[308], s1);
L_1c1484b:;
    s0 = getVarK(a, K[307]);
    s2 = R[1];
    s2 = getMemK(s2, K[3]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[309], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s2 = getVarK(a, K[307]);
    setMemK(s0, K[175], s2);
    { s0 = callFunK(a, K[59], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f22_goblack(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = Value((Object*)(a.target ? doRoot(a.target) : (DisplayObject*)vm.root));
    Value s0, s1;
    Value p0;
    s0 = getVarK(a, K[17]);
    s0 = less2(s0, Value(3.0));
    s0 = notv(s0);
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c148cf;
    --vm.sp;
    s0 = getVarK(a, K[17]);
    s0 = less2(s0, Value(10.0));
    *vm.sp++ = s0;
L_1c148cf:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c148e6;
    s1 = getVarK(a, K[17]);
    setVarK(a, K[51], s1);
    goto L_1c148f4;
L_1c148e6:;
    setVarK(a, K[51], Value(0.0));
L_1c148f4:;
    s0 = getVarK(a, K[310]);
    s1 = getVarK(a, K[51]);
    s0 = VB(eq2(s0, s1));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_end;
    s1 = getVarK(a, K[51]);
    setVarK(a, K[310], s1);
    s0 = getVarK(a, K[311]);
    setMemK(s0, K[312], Value(100.0));
    s0 = R[1];
    s0 = getMemK(s0, K[313]);
    setMemK(s0, K[56], VFALSE);
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f23_colorit(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[10];
    R[3] = argc > 0 ? argv[0] : VUNDEF;
    R[4] = argc > 1 ? argv[1] : VUNDEF;
    R[5] = argc > 2 ? argv[2] : VUNDEF;
    R[6] = argc > 3 ? argv[3] : VUNDEF;
    R[7] = argc > 4 ? argv[4] : VUNDEF;
    R[8] = argc > 5 ? argv[5] : VUNDEF;
    R[9] = argc > 6 ? argv[6] : VUNDEF;
    Value s0, s1, s2;
    s1 = getVarK(a, K[315]);
    s1 = getMemK(s1, K[316]);
    { s0 = newMethod(a, s1, Value(K[317]), 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    R[1] = s0;
    s0 = R[1];
    s2 = R[4];
    setMemK(s0, K[318], s2);
    s0 = R[1];
    s2 = R[5];
    setMemK(s0, K[319], s2);
    s0 = R[1];
    s2 = R[6];
    setMemK(s0, K[320], s2);
    s0 = R[1];
    s2 = R[7];
    setMemK(s0, K[321], s2);
    s0 = R[1];
    s2 = R[8];
    setMemK(s0, K[322], s2);
    s0 = R[1];
    s2 = R[9];
    setMemK(s0, K[323], s2);
    s0 = R[3];
    s2 = getVarK(a, K[315]);
    s2 = getMemK(s2, K[316]);
    { Value av[1] = {s0}; s0 = newMethod(a, s2, Value(K[324]), 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    R[2] = s0;
    s0 = R[2];
    s2 = R[1];
    setMemK(s0, K[325], s2);
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f24_uncolera(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    Value s0, s1;
    s1 = getVarK(a, K[313]);
    s1 = getMemK(s1, K[54]);
    setVarK(a, K[303], s1);
    s0 = getVarK(a, K[327]);
    s0 = less2(s0, Value(0.0));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c14aab;
    s0 = getVarK(a, K[327]);
    s0 = modv(s0, Value(5.0));
    s0 = VB(eq2(s0, Value(0.0)));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c14aab;
    s0 = getVarK(a, K[327]);
    s0 = modv(s0, Value(10.0));
    s0 = VB(eq2(s0, Value(0.0)));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c14aab;
L_1c14aab:;
    s1 = getVarK(a, K[327]);
    s1 = decv(s1);
    setVarK(a, K[327], s1);
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f25_darky(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[5];
    R[1] = thisv;
    R[2] = Value((Object*)(a.target ? doRoot(a.target) : (DisplayObject*)vm.root));
    R[3] = argc > 0 ? argv[0] : VUNDEF;
    Value s0, s1, s2;
    s0 = R[3];
    s0 = VB(eq2(s0, Value(150.0)));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c14b08;
    s0 = R[3];
    s0 = greater(s0, Value(0.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c14b03;
    s1 = R[3];
    setVarK(a, K[329], s1);
L_1c14b03:;
    goto L_1c14bb6;
L_1c14b08:;
    setVarK(a, K[329], Value(5.0));
    s0 = R[2];
    s0 = getMemK(s0, K[18]);
    s0 = getMemK(s0, K[18]);
    setMemK(s0, K[19], Value(4.0));
    setVarK(a, K[238], VTRUE);
    s0 = R[2];
    s0 = getMemK(s0, K[162]);
    setMemK(s0, K[0], VTRUE);
    s0 = R[2];
    s0 = getMemK(s0, K[176]);
    if (truthy(s0)) goto L_1c14b83;
    s2 = R[2];
    { Value av[1] = {Value(K[156])}; s0 = callMethodK(a, s2, K[66], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c14b83:;
    s0 = R[2];
    setMemK(s0, K[206], VTRUE);
    s0 = R[2];
    setMemK(s0, K[163], VFALSE);
    s0 = R[2];
    setMemK(s0, K[138], Value(1.0));
L_1c14bb6:;
    R[4] = VUNDEF;
    s0 = getVarK(a, K[329]);
    s0 = greater(s0, Value(0.0));
    s1 = getVarK(a, K[330]);
    s0 = orv(s0, s1);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c14dcf;
    R[3] = Value(2.5);
    s0 = getTime();
    s1 = getVarK(a, K[331]);
    s0 = sub(s0, s1);
    R[3] = s0;
    s0 = R[3];
    s0 = divv(s0, Value(15.0));
    R[3] = s0;
    s0 = R[3];
    s0 = greater(s0, Value(15.0));
    s1 = R[3];
    s1 = greater(s1, Value(0.0));
    s1 = notv(s1);
    s0 = orv(s0, s1);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c14c41;
    setVarK(a, K[207], Value(15.0));
L_1c14c41:;
    s0 = getVarK(a, K[238]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c14ca5;
    s1 = getVarK(a, K[329]);
    s2 = R[3];
    s2 = mul(s2, Value(2.0));
    s1 = add2(s1, s2);
    setVarK(a, K[329], s1);
    s0 = getVarK(a, K[329]);
    s0 = greater(s0, Value(160.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c14ca5;
    setVarK(a, K[238], VFALSE);
    s2 = R[2];
    { Value av[1] = {Value(K[332])}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c14ca5:;
    s1 = getVarK(a, K[329]);
    s2 = R[3];
    s1 = sub(s1, s2);
    setVarK(a, K[329], s1);
    s1 = getVarK(a, K[329]);
    s1 = greater(s1, Value(0.0));
    setVarK(a, K[330], s1);
    s1 = getVarK(a, K[329]);
    s1 = divv(s1, Value(100.0));
    s0 = sub(Value(1.0), s1);
    R[3] = s0;
    setVarK(a, K[47], Value(0.0));
    s2 = getVarK(a, K[315]);
    s2 = getMemK(s2, K[316]);
    { s1 = newMethod(a, s2, Value(K[317]), 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setVarK(a, K[333], s1);
    s0 = getVarK(a, K[333]);
    s2 = R[3];
    setMemK(s0, K[318], s2);
    s0 = getVarK(a, K[333]);
    s2 = R[3];
    setMemK(s0, K[319], s2);
    s0 = getVarK(a, K[333]);
    s2 = R[3];
    setMemK(s0, K[320], s2);
    s0 = getVarK(a, K[333]);
    s2 = getVarK(a, K[47]);
    setMemK(s0, K[321], s2);
    s0 = getVarK(a, K[333]);
    s2 = getVarK(a, K[47]);
    setMemK(s0, K[322], s2);
    s0 = getVarK(a, K[333]);
    s2 = getVarK(a, K[47]);
    setMemK(s0, K[323], s2);
    s0 = R[1];
    s2 = getVarK(a, K[315]);
    s2 = getMemK(s2, K[316]);
    { Value av[1] = {s0}; s0 = newMethod(a, s2, Value(K[324]), 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    R[4] = s0;
    s0 = R[4];
    s2 = getVarK(a, K[333]);
    setMemK(s0, K[325], s2);
L_1c14dcf:;
    s1 = getTime();
    setVarK(a, K[331], s1);
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f26_onEnterFrame(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = Value((Object*)(a.target ? doRoot(a.target) : (DisplayObject*)vm.root));
    Value s0, s1, s2, s3;
    Value p0, p1;
    s0 = getVarK(a, K[17]);
    s0 = VB(eq2(s0, Value(2.0)));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c14e59;
    s0 = R[1];
    s0 = getMemK(s0, K[298]);
    s1 = R[1];
    s1 = getMemK(s1, K[313]);
    s1 = getMemK(s1, K[298]);
    s0 = VB(eq2(s0, s1));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c14e54;
    s0 = R[1];
    s2 = R[1];
    s2 = getMemK(s2, K[313]);
    s2 = getMemK(s2, K[298]);
    setMemK(s0, K[298], s2);
    { s0 = callFunK(a, K[118], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c14e54:;
    goto L_1c14e9e;
L_1c14e59:;
    s0 = R[1];
    s0 = getMemK(s0, K[298]);
    s1 = R[1];
    s1 = getMemK(s1, K[335]);
    s0 = VB(eq2(s0, s1));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c14e9e;
    { s0 = callFunK(a, K[118], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = R[1];
    s2 = R[1];
    s2 = getMemK(s2, K[335]);
    setMemK(s0, K[298], s2);
L_1c14e9e:;
    s1 = getVarK(a, K[232]);
    s1 = incv(s1);
    setVarK(a, K[232], s1);
    R[2] = VUNDEF;
    s0 = getVarK(a, K[232]);
    s0 = modv(s0, Value(100.0));
    s0 = VB(eq2(s0, Value(10.0)));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c14f21;
    s0 = getVarK(a, K[83]);
    if (truthy(s0)) goto L_1c14f21;
    s0 = getVarK(a, K[38]);
    s0 = getMemK(s0, K[39]);
    s0 = getMemK(s0, K[40]);
    R[2] = s0;
    s1 = R[2];
    s1 = add2(s1, Value(K[337]));
    s3 = getVarK(a, K[38]);
    s3 = getMemK(s3, K[42]);
    { Value av[2] = {s1, Value(K[336])}; s0 = callMethodK(a, s3, K[117], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c14f21:;
    { s0 = callFunK(a, K[334], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    { s0 = callFunK(a, K[245], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    { s0 = callFunK(a, K[328], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = getVarK(a, K[311]);
    s0 = getMemK(s0, K[312]);
    s0 = greater(s0, Value(0.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c14f98;
    s0 = getVarK(a, K[311]);
    s2 = getVarK(a, K[311]);
    s2 = getMemK(s2, K[312]);
    s2 = sub(s2, Value(7.0));
    setMemK(s0, K[312], s2);
    goto L_1c14fae;
L_1c14f98:;
    s0 = getVarK(a, K[311]);
    setMemK(s0, K[312], Value(0.0));
L_1c14fae:;
    s2 = getVarK(a, K[193]);
    { Value av[1] = {Value(32.0)}; s0 = callMethodK(a, s2, K[194], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c1502d;
    --vm.sp;
    s0 = getVarK(a, K[17]);
    s0 = VB(eq2(s0, Value(3.0)));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c14fff;
    --vm.sp;
    s0 = getVarK(a, K[338]);
    s0 = getMemK(s0, K[17]);
    s0 = greater(s0, Value(3.0));
    *vm.sp++ = s0;
L_1c14fff:;
    s0 = getVarK(a, K[17]);
    s0 = VB(eq2(s0, Value(4.0)));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c1502c;
    --vm.sp;
    s0 = getVarK(a, K[338]);
    s0 = getMemK(s0, K[17]);
    s0 = greater(s0, Value(21.0));
    *vm.sp++ = s0;
L_1c1502c:;
    p0 = *--vm.sp;
    p1 = *--vm.sp;
    s0 = orv(p1, p0);
    *vm.sp++ = s0;
L_1c1502d:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c1505d;
    s2 = R[1];
    { Value av[1] = {Value(K[332])}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = R[1];
    setMemK(s0, K[163], VTRUE);
L_1c1505d:;
    s2 = getVarK(a, K[193]);
    { Value av[1] = {Value(32.0)}; s0 = callMethodK(a, s2, K[194], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c15090;
    --vm.sp;
    s0 = getVarK(a, K[17]);
    s0 = VB(eq2(s0, Value(6.0)));
    *vm.sp++ = s0;
L_1c15090:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c150be;
    s2 = R[1];
    { Value av[1] = {Value(7.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setVarK(a, K[339], VTRUE);
L_1c150be:;
    s2 = getVarK(a, K[193]);
    { Value av[1] = {Value(32.0)}; s0 = callMethodK(a, s2, K[194], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c15125;
    --vm.sp;
    s0 = getVarK(a, K[17]);
    s0 = VB(eq2(s0, Value(21.0)));
    s1 = getVarK(a, K[17]);
    s1 = VB(eq2(s1, Value(7.0)));
    s0 = orv(s0, s1);
    s1 = R[1];
    s1 = getMemK(s1, K[340]);
    s2 = s1;
    s2 = notv(s2);
    *vm.sp++ = s0;
    *vm.sp++ = s1;
    if (truthy(s2)) goto L_1c15124;
    --vm.sp;
    s0 = getVarK(a, K[17]);
    s0 = VB(eq2(s0, Value(20.0)));
    *vm.sp++ = s0;
L_1c15124:;
    p0 = *--vm.sp;
    p1 = *--vm.sp;
    s0 = orv(p1, p0);
    *vm.sp++ = s0;
L_1c15125:;
    *vm.sp = vm.sp[-1]; vm.sp++;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c15135;
    --vm.sp;
    s0 = getVarK(a, K[339]);
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c15135:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c15166;
    s0 = R[1];
    setMemK(s0, K[340], VFALSE);
    s2 = R[1];
    { Value av[1] = {Value(K[10])}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c15166:;
    s2 = getVarK(a, K[193]);
    { Value av[1] = {Value(32.0)}; s0 = callMethodK(a, s2, K[194], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c15199;
    --vm.sp;
    s0 = getVarK(a, K[17]);
    s0 = VB(eq2(s0, Value(9.0)));
    *vm.sp++ = s0;
L_1c15199:;
    *vm.sp = vm.sp[-1]; vm.sp++;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c151a9;
    --vm.sp;
    s0 = getVarK(a, K[339]);
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c151a9:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c151d7;
    s2 = R[1];
    { Value av[1] = {Value(20.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setVarK(a, K[339], VTRUE);
L_1c151d7:;
    s2 = getVarK(a, K[193]);
    { Value av[1] = {Value(32.0)}; s0 = callMethodK(a, s2, K[194], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c15231;
    --vm.sp;
    s0 = getVarK(a, K[17]);
    s0 = greater(s0, Value(9.0));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1c15221;
    --vm.sp;
    s0 = getVarK(a, K[17]);
    s0 = less2(s0, Value(20.0));
    *vm.sp++ = s0;
L_1c15221:;
    s0 = getVarK(a, K[17]);
    s0 = VB(eq2(s0, Value(22.0)));
    p0 = *--vm.sp;
    s0 = orv(p0, s0);
    *vm.sp++ = s0;
L_1c15231:;
    *vm.sp = vm.sp[-1]; vm.sp++;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c15241;
    --vm.sp;
    s0 = getVarK(a, K[339]);
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1c15241:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1c1526f;
    s2 = R[1];
    { Value av[1] = {Value(21.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setVarK(a, K[339], VTRUE);
L_1c1526f:;
    s2 = getVarK(a, K[193]);
    { Value av[1] = {Value(32.0)}; s0 = callMethodK(a, s2, K[194], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    if (truthy(s0)) goto L_1c1529c;
    setVarK(a, K[339], VFALSE);
L_1c1529c:;
    { s0 = callFunK(a, K[192], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    { s0 = callFunK(a, K[254], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s2 = getVarK(a, K[193]);
    { Value av[1] = {Value(73.0)}; s0 = callMethodK(a, s2, K[194], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c15324;
    s0 = getVarK(a, K[341]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c1531f;
    { s0 = callFunK(a, K[25], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setVarK(a, K[341], VFALSE);
    s2 = R[1];
    { Value av[1] = {Value(K[342])}; s0 = callMethodK(a, s2, K[66], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1c1531f:;
    goto L_1c15330;
L_1c15324:;
    setVarK(a, K[341], VTRUE);
L_1c15330:;
    s2 = getVarK(a, K[193]);
    { Value av[1] = {Value(72.0)}; s0 = callMethodK(a, s2, K[194], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c153e6;
    s0 = getVarK(a, K[343]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c153e1;
    s0 = getVarK(a, K[183]);
    s0 = VB(eq2(s0, Value(2.0)));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c15384;
    setVarK(a, K[183], VTRUE);
    goto L_1c153ae;
L_1c15384:;
    s0 = getVarK(a, K[183]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c153a0;
    setVarK(a, K[183], VFALSE);
    goto L_1c153ae;
L_1c153a0:;
    setVarK(a, K[183], Value(2.0));
L_1c153ae:;
    { s0 = callFunK(a, K[283], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setVarK(a, K[343], VFALSE);
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s2 = getVarK(a, K[183]);
    setMemK(s0, K[182], s2);
L_1c153e1:;
    goto L_1c153f2;
L_1c153e6:;
    setVarK(a, K[343], VTRUE);
L_1c153f2:;
    s2 = getVarK(a, K[193]);
    { Value av[1] = {Value(70.0)}; s0 = callMethodK(a, s2, K[194], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c1545d;
    s0 = getVarK(a, K[344]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c15458;
    s0 = R[1];
    s0 = getMemK(s0, K[3]);
    s0 = getMemK(s0, K[4]);
    if (truthy(s0)) goto L_1c15458;
    { Value av[1] = {VTRUE}; s0 = callFunK(a, K[283], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setVarK(a, K[344], VFALSE);
L_1c15458:;
    goto L_end;
L_1c1545d:;
    setVarK(a, K[344], VTRUE);
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f27_(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = thisv;
    R[2] = argc > 0 ? argv[0] : VUNDEF;
    Value s0, s1, s2, s3, s4, s5, s6, s7;
    s7 = getVarK(a, K[315]);
    s7 = getMemK(s7, K[316]);
    { Value av[6] = {Value(0.566667), Value(0.0), Value(0.0), Value(0.566667), Value(0.0), Value(0.0)}; s0 = newMethod(a, s7, Value(K[599]), 6, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s1 = R[2];
    s3 = R[1];
    { Value av[2] = {s1, s0}; s0 = callMethodK(a, s3, K[597], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f28_(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = argc > 0 ? argv[0] : VUNDEF;
    Value s0, s1;
    s0 = R[1];
    s0 = getMemK(s0, K[608]);
    s0 = VB(eq2(s0, Value(K[95])));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_end;
    s0 = getVarK(a, K[262]);
    s0 = add2(s0, Value(100.0));
    s1 = getVarK(a, K[232]);
    s0 = less2(s0, s1);
    s0 = notv(s0);
    if (truthy(s0)) goto L_end;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    s0 = getMemK(s0, K[263]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_end;
    s0 = getVarK(a, K[31]);
    s0 = getMemK(s0, K[102]);
    setMemK(s0, K[263], VFALSE);
    { s0 = callFunK(a, K[283], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_end:;
    vm.sp = sp0; return VUNDEF;
}

void s_0_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3, s4, s5, s6;
    Value p0;
    defineFunction(a, f3_nooo, K[85]);
    defineFunction(a, f4_ffss1, K[86]);
    defineFunction(a, f5_add, K[91]);
    defineFunction(a, f6_sub, K[97]);
    defineFunction(a, f7_aloc, K[118]);
    defineFunction(a, f8_aloc2, K[124]);
    defineFunction(a, f9_soundy, K[66]);
    defineFunction(a, f10_soz, K[44]);
    defineFunction(a, f11_unlos, K[192]);
    defineFunction(a, f12_soundfra, K[245]);
    defineFunction(a, f13_prelod, K[254]);
    defineFunction(a, f14_fullf, K[283]);
    defineFunction(a, f15_callit, K[190]);
    defineFunction(a, f16_doit, K[22]);
    defineFunction(a, f17_wiq, K[30]);
    defineFunction(a, f18_reser, K[29]);
    defineFunction(a, f19_showit, K[23]);
    defineFunction(a, f20_qshow, K[59]);
    defineFunction(a, f21_qua, K[25]);
    defineFunction(a, f22_goblack, K[314]);
    defineFunction(a, f23_colorit, K[326]);
    defineFunction(a, f24_uncolera, K[328]);
    defineFunction(a, f25_darky, K[334]);
    defineFunction(a, f26_onEnterFrame, K[73]);
    setVarK(a, K[83], VFALSE);
    { Value av[133] = {Value(0.0), Value(K[476]), Value(K[475]), Value(K[474]), Value(K[473]), Value(K[472]), Value(K[471]), Value(K[470]), Value(K[469]), Value(K[468]), Value(K[467]), Value(K[466]), Value(K[465]), Value(K[464]), Value(K[463]), Value(K[462]), Value(K[461]), Value(K[460]), Value(K[459]), Value(K[458]), Value(K[457]), Value(K[456]), Value(K[455]), Value(K[454]), Value(K[453]), Value(K[452]), Value(K[451]), Value(K[450]), Value(K[449]), Value(K[448]), Value(K[447]), Value(K[446]), Value(K[445]), Value(K[444]), Value(K[443]), Value(K[442]), Value(K[441]), Value(K[440]), Value(K[439]), Value(K[438]), Value(K[437]), Value(K[436]), Value(K[435]), Value(K[434]), Value(K[433]), Value(K[432]), Value(K[431]), Value(K[430]), Value(K[429]), Value(K[428]), Value(K[427]), Value(K[426]), Value(K[425]), Value(K[424]), Value(K[423]), Value(K[422]), Value(K[421]), Value(K[420]), Value(K[419]), Value(K[418]), Value(K[417]), Value(K[416]), Value(K[415]), Value(K[414]), Value(K[413]), Value(K[412]), Value(K[411]), Value(K[410]), Value(K[409]), Value(K[408]), Value(K[407]), Value(K[406]), Value(K[405]), Value(K[404]), Value(K[403]), Value(K[402]), Value(K[401]), Value(K[400]), Value(K[399]), Value(K[398]), Value(K[397]), Value(K[396]), Value(K[395]), Value(K[394]), Value(K[393]), Value(K[392]), Value(K[391]), Value(K[390]), Value(K[389]), Value(K[388]), Value(K[387]), Value(K[386]), Value(K[385]), Value(K[384]), Value(K[383]), Value(K[382]), Value(K[381]), Value(K[380]), Value(K[379]), Value(K[378]), Value(K[377]), Value(K[376]), Value(K[375]), Value(K[374]), Value(K[373]), Value(K[372]), Value(K[371]), Value(K[370]), Value(K[369]), Value(K[368]), Value(K[367]), Value(K[366]), Value(K[365]), Value(K[364]), Value(K[363]), Value(K[362]), Value(K[361]), Value(K[360]), Value(K[359]), Value(K[358]), Value(K[357]), Value(K[357]), Value(K[356]), Value(K[355]), Value(K[354]), Value(K[353]), Value(K[352]), Value(K[351]), Value(K[350]), Value(K[349]), Value(K[348]), Value(K[347]), Value(K[346])}; s1 = initArray(133, av); }
    setVarK(a, K[345], s1);
    { Value av[141] = {Value(0.0), Value(K[526]), Value(K[536]), Value(K[535]), Value(K[507]), Value(0.0), Value(K[526]), Value(K[507]), Value(0.0), Value(K[534]), Value(0.0), Value(K[466]), Value(K[533]), Value(K[532]), Value(K[531]), Value(K[529]), Value(K[529]), Value(0.0), Value(K[530]), Value(0.0), Value(0.0), Value(0.0), Value(K[529]), Value(K[529]), Value(K[529]), Value(K[529]), Value(K[529]), Value(K[528]), Value(K[528]), Value(K[527]), Value(K[527]), Value(K[527]), Value(K[526]), Value(K[480]), Value(K[480]), Value(K[480]), Value(K[480]), Value(K[480]), Value(K[480]), Value(K[480]), Value(K[480]), Value(K[480]), Value(K[480]), Value(K[480]), Value(K[480]), Value(K[480]), Value(0.0), Value(K[480]), Value(K[525]), Value(K[480]), Value(K[524]), Value(K[524]), Value(K[523]), Value(K[522]), Value(0.0), Value(K[504]), Value(K[480]), Value(K[482]), Value(K[480]), Value(K[521]), Value(K[520]), Value(0.0), Value(K[519]), Value(K[518]), Value(K[517]), Value(K[516]), Value(0.0), Value(0.0), Value(K[515]), Value(K[514]), Value(K[513]), Value(K[512]), Value(K[511]), Value(0.0), Value(K[510]), Value(K[509]), Value(K[508]), Value(K[480]), Value(K[480]), Value(K[507]), Value(K[507]), Value(K[506]), Value(K[505]), Value(K[480]), Value(K[480]), Value(K[480]), Value(K[480]), Value(K[504]), Value(0.0), Value(K[503]), Value(K[502]), Value(K[501]), Value(K[500]), Value(K[480]), Value(K[499]), Value(0.0), Value(K[498]), Value(K[480]), Value(0.0), Value(0.0), Value(0.0), Value(K[497]), Value(K[480]), Value(K[496]), Value(K[495]), Value(K[480]), Value(K[494]), Value(K[480]), Value(K[493]), Value(K[492]), Value(K[491]), Value(K[480]), Value(K[490]), Value(0.0), Value(K[489]), Value(K[488]), Value(K[487]), Value(0.0), Value(0.0), Value(K[481]), Value(K[486]), Value(K[485]), Value(K[484]), Value(K[480]), Value(K[480]), Value(K[483]), Value(K[480]), Value(K[480]), Value(K[482]), Value(K[481]), Value(K[480]), Value(K[479]), Value(K[478]), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0)}; s1 = initArray(141, av); }
    setVarK(a, K[477], s1);
    { Value av[65] = {Value(0.0), Value(K[587]), Value(K[586]), Value(K[585]), Value(K[584]), Value(0.0), Value(0.0), Value(0.0), Value(K[583]), Value(K[582]), Value(K[581]), Value(K[580]), Value(0.0), Value(K[579]), Value(K[578]), Value(0.0), Value(0.0), Value(0.0), Value(K[577]), Value(K[576]), Value(K[575]), Value(K[574]), Value(0.0), Value(0.0), Value(0.0), Value(K[573]), Value(K[572]), Value(K[571]), Value(K[570]), Value(K[569]), Value(K[568]), Value(K[427]), Value(K[567]), Value(K[566]), Value(K[565]), Value(K[564]), Value(K[563]), Value(K[562]), Value(K[561]), Value(K[560]), Value(K[559]), Value(K[558]), Value(K[557]), Value(K[556]), Value(K[555]), Value(K[554]), Value(K[553]), Value(K[552]), Value(K[551]), Value(K[550]), Value(K[549]), Value(K[548]), Value(K[547]), Value(K[546]), Value(K[545]), Value(K[544]), Value(0.0), Value(0.0), Value(K[543]), Value(K[542]), Value(K[541]), Value(K[540]), Value(K[539]), Value(K[538]), Value(K[537])}; s1 = initArray(65, av); }
    setVarK(a, K[189], s1);
    actGetURL(a, "FSCommand:showmenu", "false");
    actGetURL(a, "FSCommand:trapallkeys", "true");
    actStop(a);
    setVarK(a, K[252], Value(0.0));
    s0 = getVarK(a, K[2]);
    { s2 = initArray(0, nullptr); }
    setMemK(s0, K[134], s2);
    s0 = getVarK(a, K[2]);
    setMemK(s0, K[143], Value(0.0));
    s0 = getVarK(a, K[2]);
    { s2 = initArray(0, nullptr); }
    setMemK(s0, K[125], s2);
    s0 = getVarK(a, K[2]);
    setMemK(s0, K[132], Value(0.7));
    setVarK(a, K[169], Value(35.0));
    setVarK(a, K[329], Value(0.0));
    setVarK(a, K[167], Value(0.03));
    { Value av[1] = {Value(200.0)}; s1 = newObjectK(a, K[99], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    setVarK(a, K[101], s1);
    setVarK(a, K[51], Value(5.0));
    setVarK(a, K[47], Value(6.0));
    setVarK(a, K[100], Value(0.0));
L_1c15e05:;
    s0 = getVarK(a, K[100]);
    s0 = less2(s0, Value(200.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c15e41;
    s0 = getVarK(a, K[101]);
    s1 = getVarK(a, K[100]);
    setMem(s0, s1, Value(0.0));
    s1 = getVarK(a, K[100]);
    s1 = incv(s1);
    setVarK(a, K[100], s1);
    goto L_1c15e05;
L_1c15e41:;
    setVarK(a, K[100], Value(0.0));
L_1c15e4f:;
    s0 = getVarK(a, K[100]);
    s0 = less2(s0, Value(1000.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c15f4e;
    s1 = getVarK(a, K[51]);
    s2 = getVarK(a, K[47]);
    s1 = add2(s1, s2);
    setVarK(a, K[51], s1);
    s1 = getVarK(a, K[47]);
    s1 = incv(s1);
    setVarK(a, K[47], s1);
    s0 = getVarK(a, K[47]);
    s0 = greater(s0, Value(5.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c15ea7;
    setVarK(a, K[47], Value(2.0));
L_1c15ea7:;
    s0 = getVarK(a, K[51]);
    s0 = less2(s0, Value(200.0));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c15ed2;
    s1 = getVarK(a, K[51]);
    s1 = sub(s1, Value(200.0));
    setVarK(a, K[51], s1);
L_1c15ed2:;
    s0 = getVarK(a, K[101]);
    s1 = getVarK(a, K[51]);
    s2 = getVarK(a, K[101]);
    s3 = getVarK(a, K[51]);
    s2 = getMem(s2, s3);
    s3 = getVarK(a, K[100]);
    s3 = modv(s3, Value(5.0));
    s2 = add2(s2, s3);
    setMem(s0, s1, s2);
    s0 = getVarK(a, K[101]);
    s1 = getVarK(a, K[51]);
    s0 = getMem(s0, s1);
    s0 = less2(s0, Value(9.0));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c15f3c;
    s0 = getVarK(a, K[101]);
    s1 = getVarK(a, K[51]);
    s2 = getVarK(a, K[101]);
    s3 = getVarK(a, K[51]);
    s2 = getMem(s2, s3);
    s2 = sub(s2, Value(9.0));
    setMem(s0, s1, s2);
L_1c15f3c:;
    s1 = getVarK(a, K[100]);
    s1 = incv(s1);
    setVarK(a, K[100], s1);
    goto L_1c15e4f;
L_1c15f4e:;
    s0 = getVarK(a, K[2]);
    s2 = getVarK(a, K[101]);
    setMemK(s0, K[588], s2);
    s0 = getVarK(a, K[2]);
    { s2 = initArray(0, nullptr); }
    setMemK(s0, K[184], s2);
    { s0 = callFunK(a, K[44], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    setVarK(a, K[206], VTRUE);
    setVarK(a, K[216], Value(0.0));
    setVarK(a, K[241], Value(30.0));
    setVarK(a, K[589], Value(0.0));
    { s0 = callFunK(a, K[254], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    s2 = actGetProperty(a, Value(K[1]), Value(15.0));
    { s1 = callMethodK(a, s2, K[256], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    setVarK(a, K[590], s1);
    setVarK(a, K[591], VTRUE);
    s4 = getVarK(a, K[2]);
    { Value av[2] = {Value(K[592]), Value(299.0)}; s1 = callMethodK(a, s4, K[593], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    setVarK(a, K[50], s1);
    s6 = getVarK(a, K[315]);
    s6 = getMemK(s6, K[595]);
    { Value av[4] = {Value(363.0), Value(272.0), VTRUE, Value(0.0)}; s1 = newMethod(a, s6, Value(K[596]), 4, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    setVarK(a, K[594], s1);
    s0 = getVarK(a, K[594]);
    s2 = getVarK(a, K[594]);
    s2 = getMemK(s2, K[598]);
    setMemK(s0, K[597], s2);
    s0 = getVarK(a, K[594]);
    s2 = makeFunction(a, f27_, "");
    setMemK(s0, K[598], s2);
    s0 = getVarK(a, K[50]);
    s2 = getVarK(a, K[50]);
    s4 = divv(Value(100.0), Value(0.566667));
    R[0] = s4;
    setMemK(s2, K[257], s4);
    s2 = R[0];
    setMemK(s0, K[258], s2);
    s1 = getVarK(a, K[2]);
    s1 = getMemK(s1, K[594]);
    s3 = getVarK(a, K[50]);
    { Value av[2] = {s1, Value(1.0)}; s0 = callMethodK(a, s3, K[600], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    { Value av[2] = {Value(K[601]), Value(301.0)}; s1 = callFunK(a, K[593], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    setVarK(a, K[50], s1);
    s6 = getVarK(a, K[315]);
    s6 = getMemK(s6, K[595]);
    { Value av[4] = {Value(450.0), Value(150.0), VTRUE, Value(0.0)}; s1 = newMethod(a, s6, Value(K[596]), 4, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    setVarK(a, K[602], s1);
    s3 = getVarK(a, K[602]);
    s5 = getVarK(a, K[50]);
    { Value av[4] = {s3, Value(1.0), VTRUE, VFALSE}; s0 = callMethodK(a, s5, K[600], 4, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    s0 = getVarK(a, K[50]);
    setMemK(s0, K[260], Value(-45.0));
    s2 = getVarK(a, K[603]);
    { Value av[1] = {Value(500.0)}; s0 = callMethodK(a, s2, K[242], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    s0 = getVarK(a, K[83]);
    if (truthy(s0)) goto L_1c16262;
    s2 = getVarK(a, K[38]);
    s2 = getMemK(s2, K[272]);
    { s1 = callMethodK(a, s2, K[280], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    setVarK(a, K[51], s1);
L_1c16262:;
    s1 = getVarK(a, K[51]);
    s1 = getMem(s1, Value(0.0));
    setVarK(a, K[266], s1);
    s1 = getVarK(a, K[51]);
    s1 = getMem(s1, Value(0.0));
    s2 = getVarK(a, K[51]);
    s2 = getMem(s2, Value(1.0));
    s1 = divv(s1, s2);
    setVarK(a, K[265], s1);
    s1 = getVarK(a, K[51]);
    s1 = getMem(s1, Value(2.0));
    setVarK(a, K[271], s1);
    s0 = getVarK(a, K[271]);
    s0 = greater(s0, Value(10.0));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c162d5;
    setVarK(a, K[271], Value(32.0));
L_1c162d5:;
    s0 = getVarK(a, K[265]);
    s0 = greater(s0, Value(1.5));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c162ff;
    setVarK(a, K[265], VFALSE);
    goto L_1c16326;
L_1c162ff:;
    s0 = getVarK(a, K[265]);
    s0 = greater(s0, Value(1.7));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c16326;
    setVarK(a, K[265], Value(2.0));
L_1c16326:;
    s0 = getVarK(a, K[83]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c16342;
    setVarK(a, K[264], VFALSE);
    goto L_1c16435;
L_1c16342:;
    s1 = getVarK(a, K[38]);
    s1 = getMemK(s1, K[272]);
    s1 = getMemK(s1, K[605]);
    defLocalK(a, K[604], s1);
    s1 = getVarK(a, K[604]);
    s1 = add2(s1, Value(K[290]));
    setVarK(a, K[604], s1);
    s3 = getVarK(a, K[604]);
    { Value av[1] = {Value(K[606])}; s1 = callMethodK(a, s3, K[129], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    s1 = greater(s1, Value(0.0));
    setVarK(a, K[264], s1);
    s0 = getVarK(a, K[38]);
    s0 = getMemK(s0, K[39]);
    s2 = makeFunction(a, f28_, "");
    setMemK(s0, K[607], s2);
L_1c16435:;
    setVarK(a, K[262], Value(0.0));
    { Value av[2] = {Value(1024.0), Value(768.0)}; s1 = initArray(2, av); }
    { Value av[2] = {Value(1280.0), Value(720.0)}; s2 = initArray(2, av); }
    { Value av[2] = {Value(1280.0), Value(800.0)}; s3 = initArray(2, av); }
    { Value av[2] = {Value(640.0), Value(480.0)}; s4 = initArray(2, av); }
    { Value av[2] = {Value(720.0), Value(480.0)}; s5 = initArray(2, av); }
    { Value av[2] = {Value(800.0), Value(600.0)}; s6 = initArray(2, av); }
    { Value av[7] = {Value(0.0), s6, s5, s4, s3, s2, s1}; s1 = initArray(7, av); }
    setVarK(a, K[267], s1);
    s0 = getVarK(a, K[311]);
    setMemK(s0, K[312], Value(0.0));
    setVarK(a, K[310], Value(0.0));
    { Value av[3] = {Value(K[313]), Value(K[313]), Value(300.0)}; s0 = callFunK(a, K[145], 3, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    { s0 = callFunK(a, K[283], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    setVarK(a, K[232], Value(0.0));
    setVarK(a, K[51], Value(K[1]));
    setVarK(a, K[48], VFALSE);
    s0 = getVarK(a, K[2]);
    s0 = getMemK(s0, K[71]);
    enumerate2(s0);
L_1c16579:;
    R[0] = vm.sp[-1];
    p0 = *--vm.sp;
    s0 = VB(eq2(p0, VNULL));
    if (truthy(s0)) goto L_1c16607;
    s1 = R[0];
    setVarK(a, K[100], s1);
    s0 = getVarK(a, K[2]);
    s0 = getMemK(s0, K[71]);
    s1 = getVarK(a, K[100]);
    s0 = getMem(s0, s1);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c16602;
    s0 = getVarK(a, K[189]);
    s1 = getVarK(a, K[100]);
    s0 = getMem(s0, s1);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c16602;
    s0 = getVarK(a, K[48]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c165dd;
    s1 = getVarK(a, K[51]);
    s1 = add2(s1, Value(K[290]));
    setVarK(a, K[51], s1);
L_1c165dd:;
    s1 = getVarK(a, K[51]);
    s2 = getVarK(a, K[189]);
    s3 = getVarK(a, K[100]);
    s2 = getMem(s2, s3);
    s1 = add2(s1, s2);
    setVarK(a, K[51], s1);
    setVarK(a, K[48], VTRUE);
L_1c16602:;
    goto L_1c16579;
L_1c16607:;
    s0 = getVarK(a, K[48]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1c16648;
    s0 = getVarK(a, K[51]);
    { Value av[1] = {s0}; s0 = callFunK(a, K[190], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    s0 = getVarK(a, K[264]);
    if (truthy(s0)) goto L_1c16648;
    s0 = getVarK(a, K[51]);
    { Value av[1] = {s0}; s0 = callFunK(a, K[190], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_1c16648:;
    { s1 = newObjectK(a, K[610], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    defLocalK(a, K[609], s1);
    s1 = getVarK(a, K[609]);
    { s0 = callMethodK(a, s1, K[611], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    s0 = getVarK(a, K[2]);
    s2 = getVarK(a, K[609]);
    setMemK(s0, K[10], s2);
L_end:;
    vm.sp = sp0;
}

void b_4376_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[2]);
    { Value av[1] = {Value(22.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void b_4381_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[2]);
    { Value av[1] = {Value(9.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void b_4385_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[2]);
    { Value av[1] = {Value(10.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void b_4389_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[2]);
    { Value av[1] = {Value(11.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void b_4393_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[2]);
    { Value av[1] = {Value(12.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void b_4397_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[2]);
    { Value av[1] = {Value(13.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void b_4401_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[2]);
    { Value av[1] = {Value(14.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void b_4405_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[2]);
    { Value av[1] = {Value(15.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void b_4409_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[2]);
    { Value av[1] = {Value(16.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void b_4413_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[2]);
    { Value av[1] = {Value(17.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void b_4417_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[2]);
    { Value av[1] = {Value(18.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void b_4421_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s2 = getVarK(a, K[2]);
    { Value av[1] = {Value(19.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4422_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4487_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = getVarK(a, K[2]);
    s0 = getMemK(s0, K[612]);
    actGotoFrame2(a, s0, false, 0);
L_end:;
    vm.sp = sp0;
}

void s_4512_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = getVarK(a, K[2]);
    s0 = getMemK(s0, K[612]);
    actGotoFrame2(a, s0, false, 0);
L_end:;
    vm.sp = sp0;
}

void s_4535_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = getVarK(a, K[2]);
    s0 = getMemK(s0, K[6]);
    s0 = add2(s0, Value(1.0));
    actGotoFrame2(a, s0, false, 0);
L_end:;
    vm.sp = sp0;
}

void s_4537_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = getVarK(a, K[2]);
    s0 = getMemK(s0, K[6]);
    s0 = add2(s0, Value(1.0));
    actGotoFrame2(a, s0, false, 0);
L_end:;
    vm.sp = sp0;
}

void s_0_2_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    s1 = getVarK(a, K[2]);
    { s0 = callMethodK(a, s1, K[314], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_0_2_1(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4678_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[2]);
    s0 = getMemK(s0, K[136]);
    s1 = getVarK(a, K[2]);
    s1 = getMemK(s1, K[136]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[136]);
    s1 = mul(s1, s2);
    s0 = add2(s0, s1);
    s1 = getVarK(a, K[2]);
    s1 = getMemK(s1, K[136]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[136]);
    s1 = mul(s1, s2);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[136]);
    s1 = mul(s1, s2);
    s0 = add2(s0, s1);
    s1 = actGetProperty(a, Value(K[1]), Value(5.0));
    s0 = modv(s0, s1);
    s0 = add2(s0, Value(1.0));
    actGotoFrame2(a, s0, false, 0);
L_end:;
    vm.sp = sp0;
}

void s_4686_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2;
    s0 = getVarK(a, K[2]);
    s0 = getMemK(s0, K[136]);
    s1 = getVarK(a, K[2]);
    s1 = getMemK(s1, K[136]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[136]);
    s1 = mul(s1, s2);
    s0 = add2(s0, s1);
    s1 = getVarK(a, K[2]);
    s1 = getMemK(s1, K[136]);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[136]);
    s1 = mul(s1, s2);
    s2 = getVarK(a, K[2]);
    s2 = getMemK(s2, K[136]);
    s1 = mul(s1, s2);
    s0 = add2(s0, s1);
    s1 = actGetProperty(a, Value(K[1]), Value(5.0));
    s0 = modv(s0, s1);
    s0 = add2(s0, Value(1.0));
    actGotoFrame2(a, s0, false, 0);
L_end:;
    vm.sp = sp0;
}

void s_4711_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4722_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = actGetProperty(a, Value(K[1]), Value(5.0));
    s0 = randomNumber(s0);
    s0 = add2(s0, Value(1.0));
    actGotoFrame2(a, s0, false, 0);
L_end:;
    vm.sp = sp0;
}

void s_4733_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = actGetProperty(a, Value(K[1]), Value(5.0));
    s0 = randomNumber(s0);
    s0 = add2(s0, Value(1.0));
    actGotoFrame2(a, s0, false, 0);
L_end:;
    vm.sp = sp0;
}

void s_4738_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = actGetProperty(a, Value(K[1]), Value(5.0));
    s0 = randomNumber(s0);
    s0 = add2(s0, Value(1.0));
    actGotoFrame2(a, s0, false, 0);
L_end:;
    vm.sp = sp0;
}

void s_4743_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = actGetProperty(a, Value(K[1]), Value(5.0));
    s0 = randomNumber(s0);
    s0 = add2(s0, Value(1.0));
    actGotoFrame2(a, s0, false, 0);
L_end:;
    vm.sp = sp0;
}

void s_4753_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4758_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = actGetProperty(a, Value(K[1]), Value(5.0));
    s0 = randomNumber(s0);
    s0 = add2(s0, Value(1.0));
    actGotoFrame2(a, s0, false, 0);
L_end:;
    vm.sp = sp0;
}

void s_4770_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = actGetProperty(a, Value(K[1]), Value(5.0));
    s0 = randomNumber(s0);
    s0 = add2(s0, Value(1.0));
    actGotoFrame2(a, s0, false, 0);
L_end:;
    vm.sp = sp0;
}

void s_4775_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = actGetProperty(a, Value(K[1]), Value(5.0));
    s0 = randomNumber(s0);
    s0 = add2(s0, Value(1.0));
    actGotoFrame2(a, s0, false, 0);
L_end:;
    vm.sp = sp0;
}

void s_4780_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = actGetProperty(a, Value(K[1]), Value(5.0));
    s0 = randomNumber(s0);
    s0 = add2(s0, Value(1.0));
    actGotoFrame2(a, s0, false, 0);
L_end:;
    vm.sp = sp0;
}

void s_4797_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4802_9_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4803_14_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s3 = getVarK(a, K[2]);
    { Value av[2] = {Value(K[613]), Value(100.0)}; s0 = callMethodK(a, s3, K[66], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4803_23_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4808_8_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4811_20_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4812_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s0 = getVarK(a, K[614]);
    s3 = getVarK(a, K[45]);
    s3 = getMemK(s3, K[615]);
    s2 = sub(Value(0.0), s3);
    s2 = sub(s2, Value(90.0));
    setMemK(s0, K[615], s2);
L_end:;
    vm.sp = sp0;
}

void s_4812_27_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4813_11_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4828_19_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4829_11_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4834_42_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1;
    s1 = getVarK(a, K[45]);
    { s0 = callMethodK(a, s1, K[60], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4850_22_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4862_7_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s3 = getVarK(a, K[2]);
    { Value av[2] = {Value(K[616]), Value(100.0)}; s0 = callMethodK(a, s3, K[66], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4862_21_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4863_5_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s3 = getVarK(a, K[2]);
    { Value av[2] = {Value(K[613]), Value(100.0)}; s0 = callMethodK(a, s3, K[66], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4863_11_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4868_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s0 = getVarK(a, K[614]);
    s3 = getVarK(a, K[45]);
    s3 = getMemK(s3, K[615]);
    s2 = sub(Value(0.0), s3);
    s2 = sub(s2, Value(90.0));
    setMemK(s0, K[615], s2);
L_end:;
    vm.sp = sp0;
}

void s_4868_27_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4869_14_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s3 = getVarK(a, K[2]);
    { Value av[2] = {Value(K[613]), Value(100.0)}; s0 = callMethodK(a, s3, K[66], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4869_23_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4870_9_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4871_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4900_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-72.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4902_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    setVarK(a, K[100], Value(0.0));
L_1e28b51:;
    s0 = getVarK(a, K[100]);
    s0 = less2(s0, Value(50.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e28bd3;
    s0 = actGetProperty(a, Value(K[1]), Value(13.0));
    s2 = getVarK(a, K[100]);
    s1 = add2(Value(K[3]), s2);
    s0 = VB(eq2(s0, s1));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e28bc4;
    setVarK(a, K[51], VTRUE);
    s1 = getVarK(a, K[100]);
    s0 = sub(Value(0.0), s1);
    s0 = mul(s0, Value(0.01));
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, s0}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_1e28bc4:;
    s1 = getVarK(a, K[100]);
    s1 = incv(s1);
    setVarK(a, K[100], s1);
    goto L_1e28b51;
L_1e28bd3:;
    s0 = getVarK(a, K[51]);
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e28c07;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-0.01)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_1e28c07:;
L_end:;
    vm.sp = sp0;
}

void s_4904_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    setVarK(a, K[100], Value(0.0));
L_1e28c89:;
    s0 = getVarK(a, K[100]);
    s0 = less2(s0, Value(100.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e28d00;
    s0 = actGetProperty(a, Value(K[1]), Value(13.0));
    s2 = getVarK(a, K[100]);
    s1 = add2(Value(K[3]), s2);
    s0 = VB(eq2(s0, s1));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e28cf1;
    s0 = getVarK(a, K[100]);
    s0 = divv(s0, Value(100.0));
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, s0}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
    setVarK(a, K[100], Value(101.0));
L_1e28cf1:;
    s1 = getVarK(a, K[100]);
    s1 = incv(s1);
    setVarK(a, K[100], s1);
    goto L_1e28c89;
L_1e28d00:;
L_end:;
    vm.sp = sp0;
}

void s_4905_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-73.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4906_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-74.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4908_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(1.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4911_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-1.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4913_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(1.3)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4914_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-13.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4916_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    setVarK(a, K[100], Value(0.0));
L_1e2963f:;
    s0 = getVarK(a, K[100]);
    s0 = less2(s0, Value(20.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e296c1;
    s0 = actGetProperty(a, Value(K[1]), Value(13.0));
    s2 = getVarK(a, K[100]);
    s1 = add2(Value(K[3]), s2);
    s0 = VB(eq2(s0, s1));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e296b2;
    setVarK(a, K[51], VTRUE);
    s1 = getVarK(a, K[100]);
    s0 = sub(Value(0.0), s1);
    s0 = mul(s0, Value(0.01));
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, s0}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_1e296b2:;
    s1 = getVarK(a, K[100]);
    s1 = incv(s1);
    setVarK(a, K[100], s1);
    goto L_1e2963f;
L_1e296c1:;
    s0 = getVarK(a, K[51]);
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e296f1;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(3.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_1e296f1:;
L_end:;
    vm.sp = sp0;
}

void s_4917_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-18.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4919_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-10.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4921_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-7.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4922_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-15.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4923_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-16.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4924_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-14.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4926_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    setVarK(a, K[100], Value(0.0));
L_1e29a5d:;
    s0 = getVarK(a, K[100]);
    s0 = less2(s0, Value(20.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e29adf;
    s0 = actGetProperty(a, Value(K[1]), Value(13.0));
    s2 = getVarK(a, K[100]);
    s1 = add2(Value(K[3]), s2);
    s0 = VB(eq2(s0, s1));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e29ad0;
    setVarK(a, K[51], VTRUE);
    s1 = getVarK(a, K[100]);
    s0 = sub(Value(0.0), s1);
    s0 = mul(s0, Value(0.01));
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, s0}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_1e29ad0:;
    s1 = getVarK(a, K[100]);
    s1 = incv(s1);
    setVarK(a, K[100], s1);
    goto L_1e29a5d;
L_1e29adf:;
    s0 = getVarK(a, K[51]);
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e29b13;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(1.93)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_1e29b13:;
L_end:;
    vm.sp = sp0;
}

void s_4927_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-20.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4928_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-35.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4929_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-31.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4931_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-27.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4932_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-26.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4933_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-21.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4934_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-30.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4935_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-29.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4936_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-55.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4937_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-22.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4938_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-57.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4941_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-9.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4943_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(1.5)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4945_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-3.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4947_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-4.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4949_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-5.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4954_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    actStop(a);
L_end:;
    vm.sp = sp0;
}

void s_4955_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-2.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4956_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(1.4)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4957_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-68.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4958_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-71.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4960_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-6.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4961_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-52.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4965_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-12.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4966_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-17.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4967_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-64.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4968_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-25.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4969_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-32.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4970_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-63.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4973_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-23.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4974_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-33.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4975_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(4.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4976_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-67.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4977_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-46.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4979_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-19.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4980_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-48.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4983_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-49.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4985_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-11.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4986_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-70.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4987_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-44.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4988_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-47.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4989_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-50.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4990_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-51.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4991_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-62.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4992_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-66.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4993_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-40.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4994_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-37.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4995_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-38.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4996_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-39.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4997_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-60.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4998_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-42.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_4999_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-58.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_5000_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-59.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_5003_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-41.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_5005_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-34.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_5006_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-54.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_5007_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-56.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_5010_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-43.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_5012_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-36.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_5013_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-53.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_5014_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-69.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_5015_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-75.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_5016_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0, s1, s2, s3;
    s1 = getVarK(a, K[21]);
    s3 = getVarK(a, K[2]);
    s3 = getMemK(s3, K[3]);
    { Value av[2] = {s1, Value(-65.0)}; s0 = callMethodK(a, s3, K[617], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) goto L_end;
L_end:;
    vm.sp = sp0;
}

void s_5027_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = actGetProperty(a, Value(K[1]), Value(5.0));
    s0 = randomNumber(s0);
    s0 = add2(s0, Value(1.0));
    actGotoFrame2(a, s0, false, 0);
L_end:;
    vm.sp = sp0;
}

void s_5034_1_0(void* c_) {
    Act a; beginTimeline(a, (DisplayObject*)c_);
    Value* sp0 = vm.sp; (void)sp0;
    Value R[4];
    Value s0;
    s0 = actGetProperty(a, Value(K[1]), Value(5.0));
    s0 = randomNumber(s0);
    s0 = add2(s0, Value(1.0));
    actGotoFrame2(a, s0, false, 0);
L_end:;
    vm.sp = sp0;
}

Value f29_pffx(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = Value((Object*)(a.target ? doRoot(a.target) : (DisplayObject*)vm.root));
    R[2] = argc > 0 ? argv[0] : VUNDEF;
    Value s0, s1, s2, s3;
    Value p0;
    s1 = R[1];
    s1 = getMemK(s1, K[7]);
    s2 = R[2];
    s2 = add2(s2, Value(1.0));
    s1 = getMem(s1, s2);
    s2 = R[1];
    s2 = getMemK(s2, K[7]);
    s3 = R[2];
    s3 = sub(s3, Value(1.0));
    s2 = getMem(s2, s3);
    s1 = add2(s1, s2);
    s2 = R[1];
    s2 = getMemK(s2, K[7]);
    s3 = R[2];
    s3 = add2(s3, Value(10.0));
    s2 = getMem(s2, s3);
    s1 = add2(s1, s2);
    s2 = R[1];
    s2 = getMemK(s2, K[7]);
    s3 = R[2];
    s3 = sub(s3, Value(10.0));
    s2 = getMem(s2, s3);
    s1 = add2(s1, s2);
    setVarK(a, K[47], s1);
    s0 = R[1];
    s0 = getMemK(s0, K[7]);
    s1 = R[2];
    s0 = getMem(s0, s1);
    s0 = VB(eq2(s0, Value(0.0)));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1e7a5e9;
    --vm.sp;
    s0 = getVarK(a, K[47]);
    s0 = less2(s0, Value(2.0));
    *vm.sp++ = s0;
L_1e7a5e9:;
    *vm.sp = vm.sp[-1]; vm.sp++;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1e7a5fe;
    --vm.sp;
    s0 = getVarK(a, K[207]);
    s1 = getVarK(a, K[209]);
    s0 = less2(s0, s1);
    *vm.sp++ = s0;
L_1e7a5fe:;
    s0 = R[2];
    s0 = VB(eq2(s0, Value(35.0)));
    p0 = *--vm.sp;
    s0 = orv(p0, s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_end;
    s0 = R[1];
    s0 = getMemK(s0, K[7]);
    s1 = R[2];
    setMem(s0, s1, Value(1.0));
    s0 = R[2];
    s2 = getVarK(a, K[618]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s1 = getVarK(a, K[214]);
    s1 = incv(s1);
    setVarK(a, K[214], s1);
    s0 = R[2];
    s0 = VB(eq2(s0, Value(35.0)));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_end;
    s1 = getVarK(a, K[207]);
    s1 = incv(s1);
    setVarK(a, K[207], s1);
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f30_hat(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = Value((Object*)(a.target ? doRoot(a.target) : (DisplayObject*)vm.root));
    R[3] = argc > 0 ? argv[0] : VUNDEF;
    Value s0, s1, s2;
    R[2] = Value(0.0);
    s0 = R[1];
    s0 = getMemK(s0, K[621]);
    s1 = R[3];
    setMem(s0, s1, VTRUE);
    s0 = R[3];
    R[0] = s0;
    s0 = VB(strictEq(s0, Value(10.0)));
    if (truthy(s0)) goto L_1e7a8ec;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(11.0)));
    if (truthy(s0)) goto L_1e7a8ec;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(12.0)));
    if (truthy(s0)) goto L_1e7a8ec;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(31.0)));
    if (truthy(s0)) goto L_1e7a8ec;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(37.0)));
    if (truthy(s0)) goto L_1e7a8ec;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(4.0)));
    if (truthy(s0)) goto L_1e7a8fe;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(6.0)));
    if (truthy(s0)) goto L_1e7a8fe;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(7.0)));
    if (truthy(s0)) goto L_1e7a8fe;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(9.0)));
    if (truthy(s0)) goto L_1e7a8fe;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(16.0)));
    if (truthy(s0)) goto L_1e7a8fe;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(19.0)));
    if (truthy(s0)) goto L_1e7a8fe;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(21.0)));
    if (truthy(s0)) goto L_1e7a8fe;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(23.0)));
    if (truthy(s0)) goto L_1e7a8fe;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(28.0)));
    if (truthy(s0)) goto L_1e7a8fe;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(29.0)));
    if (truthy(s0)) goto L_1e7a8fe;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(32.0)));
    if (truthy(s0)) goto L_1e7a8fe;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(2.0)));
    if (truthy(s0)) goto L_1e7a910;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(5.0)));
    if (truthy(s0)) goto L_1e7a910;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(20.0)));
    if (truthy(s0)) goto L_1e7a910;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(30.0)));
    if (truthy(s0)) goto L_1e7a910;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(34.0)));
    if (truthy(s0)) goto L_1e7a910;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(3.0)));
    if (truthy(s0)) goto L_1e7a922;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(8.0)));
    if (truthy(s0)) goto L_1e7a922;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(22.0)));
    if (truthy(s0)) goto L_1e7a922;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(36.0)));
    if (truthy(s0)) goto L_1e7a922;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(26.0)));
    if (truthy(s0)) goto L_1e7a934;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(24.0)));
    if (truthy(s0)) goto L_1e7a934;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(27.0)));
    if (truthy(s0)) goto L_1e7a934;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(33.0)));
    if (truthy(s0)) goto L_1e7a934;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(35.0)));
    if (truthy(s0)) goto L_1e7a934;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(13.0)));
    if (truthy(s0)) goto L_1e7a946;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(14.0)));
    if (truthy(s0)) goto L_1e7a946;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(15.0)));
    if (truthy(s0)) goto L_1e7a946;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(17.0)));
    if (truthy(s0)) goto L_1e7a946;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(18.0)));
    if (truthy(s0)) goto L_1e7a946;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(25.0)));
    if (truthy(s0)) goto L_1e7a946;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(39.0)));
    if (truthy(s0)) goto L_1e7a946;
    goto L_1e7a95c;
L_1e7a8ec:;
    R[2] = Value(2.0);
    goto L_1e7a95c;
L_1e7a8fe:;
    R[2] = Value(5.0);
    goto L_1e7a95c;
L_1e7a910:;
    R[2] = Value(3.0);
    goto L_1e7a95c;
L_1e7a922:;
    R[2] = Value(4.0);
    goto L_1e7a95c;
L_1e7a934:;
    R[2] = Value(1.0);
    goto L_1e7a95c;
L_1e7a946:;
    R[2] = Value(0.0);
    goto L_1e7a95c;
L_1e7a95c:;
    s0 = R[1];
    s0 = getMemK(s0, K[622]);
    s1 = R[2];
    s2 = R[3];
    setMem(s0, s1, s2);
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f31_gish(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = argc > 0 ? argv[0] : VUNDEF;
    Value s0, s1, s2;
    s0 = getVarK(a, K[623]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7a9a4;
    s2 = R[1];
    { Value av[1] = {Value(2.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_end;
L_1e7a9a4:;
    s2 = R[1];
    { Value av[1] = {Value(1.0)}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f32_dmgdo(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = Value((Object*)(a.target ? doRoot(a.target) : (DisplayObject*)vm.root));
    Value s0, s1, s2, s3, s4;
    Value p0;
    R[2] = Value(0.0);
    s0 = getVarK(a, K[624]);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7a9f4;
    s0 = R[2];
    s0 = add2(s0, Value(2.0));
    R[2] = s0;
L_1e7a9f4:;
    s0 = R[2];
    s1 = getVarK(a, K[625]);
    s0 = add2(s0, s1);
    R[2] = s0;
    s0 = getVarK(a, K[626]);
    s0 = getMem(s0, Value(7.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7aa22;
    s0 = R[2];
    s0 = incv(s0);
    R[2] = s0;
L_1e7aa22:;
    s0 = getVarK(a, K[626]);
    s0 = getMem(s0, Value(90.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7aa42;
    s0 = R[2];
    s0 = incv(s0);
    R[2] = s0;
L_1e7aa42:;
    s0 = getVarK(a, K[626]);
    s0 = getMem(s0, Value(109.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7aa74;
    s0 = R[2];
    s1 = R[1];
    s1 = getMemK(s1, K[627]);
    s1 = mul(s1, Value(0.04));
    s0 = add2(s0, s1);
    R[2] = s0;
L_1e7aa74:;
    s0 = getVarK(a, K[626]);
    s0 = getMem(s0, Value(122.0));
    s0 = greater(s0, Value(0.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7aaca;
    s0 = R[2];
    s1 = getVarK(a, K[626]);
    s1 = getMem(s1, Value(122.0));
    s2 = getVarK(a, K[626]);
    s2 = getMem(s2, Value(122.0));
    s1 = mul(s1, s2);
    s1 = mul(s1, Value(1.5));
    s0 = add2(s0, s1);
    R[2] = s0;
L_1e7aaca:;
    s0 = R[2];
    s1 = getVarK(a, K[626]);
    s1 = getMem(s1, Value(50.0));
    s2 = getVarK(a, K[626]);
    s2 = getMem(s2, Value(51.0));
    s1 = add2(s1, s2);
    s2 = getVarK(a, K[626]);
    s2 = getMem(s2, Value(70.0));
    s1 = add2(s1, s2);
    s2 = getVarK(a, K[626]);
    s2 = getMem(s2, Value(79.0));
    s1 = add2(s1, s2);
    s2 = getVarK(a, K[626]);
    s2 = getMem(s2, Value(80.0));
    s3 = getVarK(a, K[626]);
    s3 = getMem(s3, Value(4.0));
    s2 = add2(s2, s3);
    s2 = mul(s2, Value(0.5));
    s1 = add2(s1, s2);
    s2 = getVarK(a, K[626]);
    s2 = getMem(s2, Value(12.0));
    s3 = getVarK(a, K[626]);
    s3 = getMem(s3, Value(101.0));
    s2 = add2(s2, s3);
    s3 = getVarK(a, K[626]);
    s3 = getMem(s3, Value(121.0));
    s2 = add2(s2, s3);
    s2 = mul(s2, Value(0.3));
    s1 = add2(s1, s2);
    s0 = add2(s0, s1);
    R[2] = s0;
    s0 = getVarK(a, K[626]);
    s0 = getMem(s0, Value(120.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7abb8;
    s0 = R[2];
    s0 = mul(s0, Value(0.9));
    R[2] = s0;
    s0 = R[2];
    s0 = sub(s0, Value(0.4));
    R[2] = s0;
L_1e7abb8:;
    s0 = getVarK(a, K[628]);
    s0 = greater(s0, Value(0.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7abe5;
    s0 = R[2];
    s0 = add2(s0, Value(0.7));
    R[2] = s0;
L_1e7abe5:;
    s2 = R[2];
    s2 = mul(s2, Value(1.2));
    s1 = add2(Value(1.0), s2);
    s3 = getVarK(a, K[80]);
    { Value av[1] = {s1}; s1 = callMethodK(a, s3, K[629], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = mul(Value(3.5), s1);
    R[2] = s0;
    { Value av[6] = {Value(0.0), Value(0.0), Value(0.2), Value(0.35), Value(0.05), Value(-0.25)}; s0 = initArray(6, av); }
    R[3] = s0;
    s0 = R[2];
    s2 = R[3];
    s3 = R[1];
    s3 = getMemK(s3, K[6]);
    s2 = getMem(s2, s3);
    s1 = add2(Value(1.0), s2);
    s0 = mul(s0, s1);
    R[2] = s0;
    s0 = R[1];
    s2 = R[2];
    setMemK(s0, K[630], s2);
    s0 = getVarK(a, K[626]);
    s0 = getMem(s0, Value(69.0));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1e7aca8;
    --vm.sp;
    s0 = getVarK(a, K[626]);
    s0 = getMem(s0, Value(118.0));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1e7aca8:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1e7ad1d;
    s2 = getVarK(a, K[232]);
    s3 = getVarK(a, K[632]);
    s2 = sub(s2, s3);
    s2 = divv(s2, Value(10.0));
    s1 = add2(Value(1.0), s2);
    s4 = getVarK(a, K[80]);
    { Value av[2] = {Value(5.0), s1}; s1 = callMethodK(a, s4, K[81], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s1 = mul(s1, Value(1.8));
    s1 = sub(s1, Value(2.0));
    s4 = getVarK(a, K[80]);
    { Value av[2] = {Value(1.0), s1}; s1 = callMethodK(a, s4, K[244], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setVarK(a, K[631], s1);
    s0 = R[2];
    s1 = getVarK(a, K[631]);
    s0 = mul(s0, s1);
    R[2] = s0;
L_1e7ad1d:;
    s0 = R[2];
    { Value rv_ = s0; vm.sp = sp0; return rv_; }
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f33_omgz(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = Value((Object*)(a.target ? doRoot(a.target) : (DisplayObject*)vm.root));
    R[2] = argc > 0 ? argv[0] : VUNDEF;
    Value s0, s1, s2;
    s0 = randomNumber(Value(3.0));
    s0 = VB(eq2(s0, Value(0.0)));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7ad6e;
    s0 = R[2];
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_end;
L_1e7ad6e:;
    s0 = R[2];
    s2 = R[1];
    s2 = getMemK(s2, K[635]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f34_itemzz(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = Value((Object*)(a.target ? doRoot(a.target) : (DisplayObject*)vm.root));
    Value s0, s1, s2, s3, s4, s5, s6, s7;
    Value p0;
    s0 = R[1];
    { Value av[67] = {Value(1.0), Value(1.0), Value(2.0), Value(3.0), Value(4.0), Value(5.0), Value(6.0), Value(7.0), Value(8.0), Value(8.0), Value(10.0), Value(12.0), Value(13.0), Value(14.0), Value(15.0), Value(19.0), Value(19.0), Value(21.0), Value(22.0), Value(23.0), Value(24.0), Value(25.0), Value(26.0), Value(27.0), Value(28.0), Value(29.0), Value(30.0), Value(31.0), Value(32.0), Value(37.0), Value(38.0), Value(39.0), Value(40.0), Value(41.0), Value(44.0), Value(47.0), Value(48.0), Value(49.0), Value(50.0), Value(51.0), Value(53.0), Value(54.0), Value(55.0), Value(56.0), Value(57.0), Value(58.0), Value(59.0), Value(60.0), Value(62.0), Value(65.0), Value(66.0), Value(67.0), Value(69.0), Value(70.0), Value(71.0), Value(72.0), Value(75.0), Value(76.0), Value(77.0), Value(85.0), Value(115.0), Value(120.0), Value(121.0), Value(124.0), Value(125.0), Value(128.0), Value(129.0)}; s2 = initArray(67, av); }
    setMemK(s0, K[634], s2);
    s0 = R[1];
    s5 = randomNumber(Value(3.0));
    s4 = add2(Value(29.0), s5);
    s6 = randomNumber(Value(3.0));
    s5 = add2(Value(29.0), s6);
    s7 = randomNumber(Value(2.0));
    s6 = add2(Value(27.0), s7);
    { Value av[10] = {Value(22.0), Value(23.0), Value(24.0), Value(25.0), Value(26.0), s6, s5, s4, Value(32.0), Value(51.0)}; s2 = initArray(10, av); }
    setMemK(s0, K[637], s2);
    s0 = R[1];
    { Value av[6] = {Value(11.0), Value(11.0), Value(16.0), Value(42.0), Value(77.0), Value(77.0)}; s2 = initArray(6, av); }
    setMemK(s0, K[638], s2);
    s0 = R[1];
    { Value av[4] = {Value(54.0), Value(64.0), Value(21.0), Value(60.0)}; s2 = initArray(4, av); }
    setMemK(s0, K[639], s2);
    s0 = R[1];
    s6 = randomNumber(Value(3.0));
    s5 = add2(Value(29.0), s6);
    s7 = randomNumber(Value(2.0));
    s6 = add2(Value(27.0), s7);
    { Value av[6] = {Value(38.0), s6, s5, Value(26.0), Value(1.0), Value(50.0)}; s2 = initArray(6, av); }
    setMemK(s0, K[640], s2);
    s0 = R[1];
    { Value av[7] = {Value(8.0), Value(67.0), Value(79.0), Value(80.0), Value(81.0), Value(118.0), Value(115.0)}; s2 = initArray(7, av); }
    setMemK(s0, K[635], s2);
    s0 = R[1];
    { Value av[8] = {Value(26.0), Value(17.0), Value(19.0), Value(21.0), Value(44.0), Value(70.0), Value(14.0), Value(13.0)}; s2 = initArray(8, av); }
    setMemK(s0, K[641], s2);
    s0 = randomNumber(Value(2.0));
    s0 = VB(eq2(s0, Value(0.0)));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b07a;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(17.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b07a:;
    s0 = randomNumber(Value(3.0));
    s0 = VB(eq2(s0, Value(0.0)));
    s0 = notv(s0);
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1e7b0aa;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(48.0));
    *vm.sp++ = s0;
L_1e7b0aa:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1e7b103;
    s0 = randomNumber(Value(3.0));
    s0 = VB(eq2(s0, Value(0.0)));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b0ea;
    s2 = R[1];
    s2 = getMemK(s2, K[635]);
    { Value av[1] = {Value(35.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1e7b103;
L_1e7b0ea:;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(35.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b103:;
    s0 = randomNumber(Value(2.0));
    s0 = VB(eq2(s0, Value(0.0)));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b138;
    s2 = R[1];
    s2 = getMemK(s2, K[639]);
    { Value av[1] = {Value(116.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b138:;
    s0 = randomNumber(Value(2.0));
    s0 = VB(eq2(s0, Value(0.0)));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b16d;
    s2 = R[1];
    s2 = getMemK(s2, K[639]);
    { Value av[1] = {Value(63.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b16d:;
    s0 = randomNumber(Value(5.0));
    s0 = VB(eq2(s0, Value(0.0)));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b1a7;
    s2 = R[1];
    s2 = getMemK(s2, K[635]);
    { Value av[1] = {Value(68.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1e7b1c0;
L_1e7b1a7:;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(68.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b1c0:;
    s0 = randomNumber(Value(3.0));
    s0 = VB(eq2(s0, Value(0.0)));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b1f5;
    s2 = R[1];
    s2 = getMemK(s2, K[635]);
    { Value av[1] = {Value(82.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b1f5:;
    s0 = randomNumber(Value(3.0));
    s0 = VB(eq2(s0, Value(0.0)));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b22f;
    s2 = R[1];
    s2 = getMemK(s2, K[635]);
    { Value av[1] = {Value(84.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1e7b264;
L_1e7b22f:;
    s0 = randomNumber(Value(2.0));
    s0 = VB(eq2(s0, Value(0.0)));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b264;
    s2 = R[1];
    s2 = getMemK(s2, K[638]);
    { Value av[1] = {Value(84.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b264:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(3.0));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1e7b28f;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[6]);
    s0 = VB(eq2(s0, Value(3.0)));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1e7b28f:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1e7b322;
    s0 = randomNumber(Value(3.0));
    s0 = VB(eq2(s0, Value(0.0)));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b2cf;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(34.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1e7b322;
L_1e7b2cf:;
    s0 = randomNumber(Value(2.0));
    s0 = VB(eq2(s0, Value(0.0)));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b309;
    s2 = R[1];
    s2 = getMemK(s2, K[638]);
    { Value av[1] = {Value(34.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1e7b322;
L_1e7b309:;
    s2 = R[1];
    s2 = getMemK(s2, K[635]);
    { Value av[1] = {Value(34.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b322:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(1.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b352;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(45.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b352:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(2.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b3bd;
    s0 = randomNumber(Value(3.0));
    s0 = VB(eq2(s0, Value(0.0)));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b3a4;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(46.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1e7b3bd;
L_1e7b3a4:;
    s2 = R[1];
    s2 = getMemK(s2, K[635]);
    { Value av[1] = {Value(46.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b3bd:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(7.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b3ed;
    s2 = R[1];
    s2 = getMemK(s2, K[639]);
    { Value av[1] = {Value(78.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b3ed:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(8.0));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1e7b41d;
    --vm.sp;
    s0 = randomNumber(Value(3.0));
    s0 = VB(eq2(s0, Value(0.0)));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1e7b41d:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1e7b43c;
    s2 = R[1];
    s2 = getMemK(s2, K[638]);
    { Value av[1] = {Value(20.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b43c:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(9.0));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1e7b46c;
    --vm.sp;
    s0 = randomNumber(Value(3.0));
    s0 = VB(eq2(s0, Value(0.0)));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1e7b46c:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1e7b48b;
    s2 = R[1];
    s2 = getMemK(s2, K[635]);
    { Value av[1] = {Value(83.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b48b:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(10.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b4f6;
    s0 = randomNumber(Value(3.0));
    s0 = VB(eq2(s0, Value(0.0)));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b4dd;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(74.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1e7b4f6;
L_1e7b4dd:;
    s2 = R[1];
    s2 = getMemK(s2, K[635]);
    { Value av[1] = {Value(74.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b4f6:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(11.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b526;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(52.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b526:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(12.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b556;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(36.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b556:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(13.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b586;
    s2 = R[1];
    s2 = getMemK(s2, K[638]);
    { Value av[1] = {Value(89.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b586:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(14.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b5b6;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(91.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b5b6:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(18.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b5cd;
L_1e7b5cd:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(19.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b5fd;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(86.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b5fd:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(20.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b62d;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(87.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b62d:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(21.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b65d;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(88.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b65d:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(25.0));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1e7b68d;
    --vm.sp;
    s0 = randomNumber(Value(3.0));
    s0 = VB(eq2(s0, Value(0.0)));
    s0 = notv(s0);
    *vm.sp++ = s0;
L_1e7b68d:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1e7b6ac;
    s2 = R[1];
    s2 = getMemK(s2, K[637]);
    { Value av[1] = {Value(92.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b6ac:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(26.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b6dc;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(98.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b6dc:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(27.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b70c;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(94.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b70c:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(28.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b73c;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(95.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b73c:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(33.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b76c;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(93.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b76c:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(29.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b7d7;
    s0 = randomNumber(Value(3.0));
    s0 = VB(eq2(s0, Value(0.0)));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b7be;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(97.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1e7b7d7;
L_1e7b7be:;
    s2 = R[1];
    s2 = getMemK(s2, K[635]);
    { Value av[1] = {Value(97.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b7d7:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(30.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b807;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(99.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b807:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(31.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b837;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(100.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b837:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(32.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b867;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(96.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b867:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(34.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b897;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(101.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b897:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(36.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b8c7;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(102.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b8c7:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(37.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b932;
    s0 = randomNumber(Value(3.0));
    s0 = VB(eq2(s0, Value(0.0)));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b919;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(103.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1e7b932;
L_1e7b919:;
    s2 = R[1];
    s2 = getMemK(s2, K[635]);
    { Value av[1] = {Value(103.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b932:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(40.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b99d;
    s0 = randomNumber(Value(3.0));
    s0 = VB(eq2(s0, Value(0.0)));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b984;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(104.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1e7b99d;
L_1e7b984:;
    s2 = R[1];
    s2 = getMemK(s2, K[635]);
    { Value av[1] = {Value(104.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b99d:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(38.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b9cd;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(105.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b9cd:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(35.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7b9fd;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(106.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7b9fd:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(39.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7ba2d;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(107.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7ba2d:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(43.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7ba5d;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(108.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7ba5d:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(44.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7bac8;
    s0 = randomNumber(Value(3.0));
    s0 = VB(eq2(s0, Value(0.0)));
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7baaf;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(109.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1e7bac8;
L_1e7baaf:;
    s2 = R[1];
    s2 = getMemK(s2, K[635]);
    { Value av[1] = {Value(109.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7bac8:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(47.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7baf8;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(110.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7baf8:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(46.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7bb28;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(111.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7bb28:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(55.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7bb50;
    { Value av[1] = {Value(114.0)}; s0 = callFunK(a, K[636], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7bb50:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(56.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7bb80;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(117.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7bb80:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(57.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7bba8;
    { Value av[1] = {Value(122.0)}; s0 = callFunK(a, K[636], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7bba8:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(58.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7bbd0;
    { Value av[1] = {Value(126.0)}; s0 = callFunK(a, K[636], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7bbd0:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(59.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7bc00;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(112.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7bc00:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(60.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7bc30;
    s2 = R[1];
    s2 = getMemK(s2, K[634]);
    { Value av[1] = {Value(131.0)}; s0 = callMethodK(a, s2, K[619], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7bc30:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(61.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7bc58;
    { Value av[1] = {Value(113.0)}; s0 = callFunK(a, K[636], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7bc58:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(62.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7bc80;
    { Value av[1] = {Value(127.0)}; s0 = callFunK(a, K[636], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7bc80:;
    s0 = R[1];
    s0 = getMemK(s0, K[71]);
    s0 = getMem(s0, Value(63.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7bca8;
    { Value av[1] = {Value(123.0)}; s0 = callFunK(a, K[636], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_1e7bca8:;
    s0 = R[1];
    s0 = getMemK(s0, K[637]);
    enumerate2(s0);
L_1e7bcb1:;
    R[0] = vm.sp[-1];
    p0 = *--vm.sp;
    s0 = VB(eq2(p0, VNULL));
    if (truthy(s0)) goto L_1e7bd4e;
    s1 = R[0];
    setVarK(a, K[88], s1);
    setVarK(a, K[49], Value(0.0));
L_1e7bcd6:;
    s0 = getVarK(a, K[49]);
    s1 = R[1];
    s1 = getMemK(s1, K[634]);
    s1 = getMemK(s1, K[69]);
    s0 = less2(s0, s1);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7bd49;
    s0 = R[1];
    s0 = getMemK(s0, K[637]);
    s1 = getVarK(a, K[88]);
    s0 = getMem(s0, s1);
    s1 = R[1];
    s1 = getMemK(s1, K[634]);
    s2 = getVarK(a, K[49]);
    s1 = getMem(s1, s2);
    s0 = VB(eq2(s0, s1));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7bd3a;
    s1 = getVarK(a, K[49]);
    s3 = R[1];
    s3 = getMemK(s3, K[634]);
    { Value av[2] = {s1, Value(1.0)}; s0 = callMethodK(a, s3, K[243], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1e7bd49;
L_1e7bd3a:;
    s1 = getVarK(a, K[49]);
    s1 = incv(s1);
    setVarK(a, K[49], s1);
    goto L_1e7bcd6;
L_1e7bd49:;
    goto L_1e7bcb1;
L_1e7bd4e:;
    s0 = R[1];
    s0 = getMemK(s0, K[639]);
    enumerate2(s0);
L_1e7bd57:;
    R[0] = vm.sp[-1];
    p0 = *--vm.sp;
    s0 = VB(eq2(p0, VNULL));
    if (truthy(s0)) goto L_1e7bdf4;
    s1 = R[0];
    setVarK(a, K[88], s1);
    setVarK(a, K[49], Value(0.0));
L_1e7bd7c:;
    s0 = getVarK(a, K[49]);
    s1 = R[1];
    s1 = getMemK(s1, K[634]);
    s1 = getMemK(s1, K[69]);
    s0 = less2(s0, s1);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7bdef;
    s0 = R[1];
    s0 = getMemK(s0, K[639]);
    s1 = getVarK(a, K[88]);
    s0 = getMem(s0, s1);
    s1 = R[1];
    s1 = getMemK(s1, K[634]);
    s2 = getVarK(a, K[49]);
    s1 = getMem(s1, s2);
    s0 = VB(eq2(s0, s1));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7bde0;
    s1 = getVarK(a, K[49]);
    s3 = R[1];
    s3 = getMemK(s3, K[634]);
    { Value av[2] = {s1, Value(1.0)}; s0 = callMethodK(a, s3, K[243], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1e7bdef;
L_1e7bde0:;
    s1 = getVarK(a, K[49]);
    s1 = incv(s1);
    setVarK(a, K[49], s1);
    goto L_1e7bd7c;
L_1e7bdef:;
    goto L_1e7bd57;
L_1e7bdf4:;
    s0 = R[1];
    s0 = getMemK(s0, K[640]);
    enumerate2(s0);
L_1e7bdfd:;
    R[0] = vm.sp[-1];
    p0 = *--vm.sp;
    s0 = VB(eq2(p0, VNULL));
    if (truthy(s0)) goto L_end;
    s1 = R[0];
    setVarK(a, K[88], s1);
    setVarK(a, K[49], Value(0.0));
L_1e7be22:;
    s0 = getVarK(a, K[49]);
    s1 = R[1];
    s1 = getMemK(s1, K[634]);
    s1 = getMemK(s1, K[69]);
    s0 = less2(s0, s1);
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7be95;
    s0 = R[1];
    s0 = getMemK(s0, K[640]);
    s1 = getVarK(a, K[88]);
    s0 = getMem(s0, s1);
    s1 = R[1];
    s1 = getMemK(s1, K[634]);
    s2 = getVarK(a, K[49]);
    s1 = getMem(s1, s2);
    s0 = VB(eq2(s0, s1));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7be86;
    s1 = getVarK(a, K[49]);
    s3 = R[1];
    s3 = getMemK(s3, K[634]);
    { Value av[2] = {s1, Value(1.0)}; s0 = callMethodK(a, s3, K[243], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_1e7be95;
L_1e7be86:;
    s1 = getVarK(a, K[49]);
    s1 = incv(s1);
    setVarK(a, K[49], s1);
    goto L_1e7be22;
L_1e7be95:;
    goto L_1e7bdfd;
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f35_newstats(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = Value((Object*)(a.target ? doRoot(a.target) : (DisplayObject*)vm.root));
    Value s0, s1, s2, s3, s4;
    Value p0;
    s2 = R[1];
    { Value av[1] = {Value(120.0)}; s0 = callMethodK(a, s2, K[334], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = R[1];
    setMemK(s0, K[9], VUNDEF);
    s0 = R[1];
    { Value av[3] = {Value(1.0), Value(1.0), Value(1.0)}; s2 = initArray(3, av); }
    setMemK(s0, K[643], s2);
    s0 = R[1];
    setMemK(s0, K[644], VTRUE);
    s0 = R[1];
    setMemK(s0, K[645], VFALSE);
    s0 = R[1];
    setMemK(s0, K[646], Value(0.0));
    s0 = R[1];
    setMemK(s0, K[647], Value(0.0));
    s0 = R[1];
    setMemK(s0, K[55], VUNDEF);
    s0 = R[1];
    setMemK(s0, K[648], Value(0.0));
    s0 = R[1];
    setMemK(s0, K[649], Value(0.0));
    s0 = R[1];
    s0 = getMemK(s0, K[138]);
    s0 = greater(s0, Value(2.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7bf68;
    s0 = R[1];
    setMemK(s0, K[163], VTRUE);
L_1e7bf68:;
    s0 = R[1];
    setMemK(s0, K[138], Value(1.0));
    s0 = R[1];
    setMemK(s0, K[650], Value(0.0));
    s0 = R[1];
    setMemK(s0, K[54], Value(0.0));
    s0 = R[1];
    setMemK(s0, K[651], Value(1.0));
    s0 = R[1];
    setMemK(s0, K[652], Value(1.0));
    s0 = R[1];
    setMemK(s0, K[653], Value(1.0));
    s0 = R[1];
    setMemK(s0, K[654], Value(0.0));
    s0 = R[1];
    { Value av[5] = {Value(1.0), Value(1.0), Value(1.0), Value(1.0), Value(1.0)}; s2 = initArray(5, av); }
    setMemK(s0, K[622], s2);
    s0 = R[1];
    setMemK(s0, K[655], VFALSE);
    s0 = R[1];
    { s2 = initArray(0, nullptr); }
    setMemK(s0, K[621], s2);
    s0 = R[1];
    s0 = getMemK(s0, K[6]);
    s0 = greater(s0, Value(0.0));
    s1 = s0;
    s1 = notv(s1);
    *vm.sp++ = s0;
    if (truthy(s1)) goto L_1e7c040;
    --vm.sp;
    s0 = R[1];
    s0 = getMemK(s0, K[6]);
    s0 = less2(s0, Value(4.0));
    *vm.sp++ = s0;
L_1e7c040:;
    p0 = *--vm.sp;
    s0 = notv(p0);
    if (truthy(s0)) goto L_1e7c06d;
    s0 = R[1];
    s0 = getMemK(s0, K[6]);
    s0 = add2(s0, Value(9.0));
    { Value av[1] = {s0}; s0 = callFunK(a, K[621], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = R[1];
    setMemK(s0, K[656], VTRUE);
L_1e7c06d:;
    s0 = R[1];
    s2 = R[1];
    s4 = R[1];
    R[0] = Value(0.0);
    setMemK(s4, K[657], Value(0.0));
    s4 = R[0];
    R[0] = s4;
    setMemK(s2, K[541], s4);
    s2 = R[0];
    setMemK(s0, K[627], s2);
    s0 = R[1];
    { Value av[6] = {Value(0.0), Value(1.0), Value(2.0), Value(3.0), Value(5.0), Value(6.0)}; s2 = initArray(6, av); }
    setMemK(s0, K[658], s2);
    s0 = R[1];
    s2 = R[1];
    s2 = getMemK(s2, K[6]);
    s2 = add2(s2, Value(1.0));
    setMemK(s0, K[79], s2);
    s0 = R[1];
    s0 = getMemK(s0, K[6]);
    R[0] = s0;
    s0 = VB(strictEq(s0, Value(0.0)));
    if (truthy(s0)) goto L_1e7c153;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(1.0)));
    if (truthy(s0)) goto L_1e7c166;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(2.0)));
    if (truthy(s0)) goto L_1e7c178;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(3.0)));
    if (truthy(s0)) goto L_1e7c198;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(4.0)));
    if (truthy(s0)) goto L_1e7c1c0;
    s0 = R[0];
    s0 = VB(strictEq(s0, Value(5.0)));
    if (truthy(s0)) goto L_1e7c1ed;
    goto L_1e7c21a;
L_1e7c153:;
    s0 = R[1];
    s2 = R[1];
    s2 = getMemK(s2, K[541]);
    s2 = incv(s2);
    setMemK(s0, K[541], s2);
    goto L_1e7c21a;
L_1e7c166:;
    s0 = R[1];
    setMemK(s0, K[79], Value(3.0));
    goto L_1e7c21a;
L_1e7c178:;
    s0 = R[1];
    setMemK(s0, K[79], Value(2.0));
    s0 = R[1];
    s2 = R[1];
    s2 = getMemK(s2, K[657]);
    s2 = incv(s2);
    setMemK(s0, K[657], s2);
    goto L_1e7c21a;
L_1e7c198:;
    s0 = R[1];
    setMemK(s0, K[79], Value(4.0));
    s0 = R[1];
    s2 = R[1];
    s2 = getMemK(s2, K[627]);
    s2 = add2(s2, Value(3.0));
    setMemK(s0, K[627], s2);
    goto L_1e7c21a;
L_1e7c1c0:;
    s0 = R[1];
    setMemK(s0, K[79], Value(5.0));
    { Value av[1] = {Value(31.0)}; s0 = callFunK(a, K[621], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = R[1];
    setMemK(s0, K[656], VTRUE);
    goto L_1e7c21a;
L_1e7c1ed:;
    s0 = R[1];
    setMemK(s0, K[79], Value(6.0));
    { Value av[1] = {Value(37.0)}; s0 = callFunK(a, K[621], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    s0 = R[1];
    setMemK(s0, K[656], VTRUE);
    goto L_1e7c21a;
L_1e7c21a:;
    s0 = R[1];
    setMemK(s0, K[75], Value(1000.0));
    s0 = R[1];
    setMemK(s0, K[659], Value(0.0));
    s0 = R[1];
    { s2 = initArray(0, nullptr); }
    setMemK(s0, K[52], s2);
    s0 = R[1];
    setMemK(s0, K[53], Value(0.0));
    s0 = R[1];
    { s2 = initArray(0, nullptr); }
    setMemK(s0, K[660], s2);
    s0 = R[1];
    setMemK(s0, K[213], VFALSE);
    s0 = R[1];
    setMemK(s0, K[661], VFALSE);
    s0 = R[1];
    setMemK(s0, K[662], VTRUE);
    s0 = R[1];
    setMemK(s0, K[663], Value(0.0));
    s0 = R[1];
    setMemK(s0, K[664], Value(10.0));
    s0 = R[1];
    setMemK(s0, K[665], Value(23.0));
    s0 = R[1];
    s0 = getMemK(s0, K[6]);
    s0 = VB(eq2(s0, Value(2.0)));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7c2e3;
    s0 = R[1];
    s2 = R[1];
    s2 = getMemK(s2, K[665]);
    s2 = sub(s2, Value(5.0));
    setMemK(s0, K[665], s2);
L_1e7c2e3:;
    { Value av[6] = {Value(0.0), Value(0.0), Value(0.2), Value(0.35), Value(0.05), Value(-0.25)}; s0 = initArray(6, av); }
    R[2] = s0;
    s2 = R[2];
    s3 = R[1];
    s3 = getMemK(s3, K[6]);
    s2 = getMem(s2, s3);
    s1 = add2(Value(1.0), s2);
    setVarK(a, K[250], s1);
    s0 = R[1];
    s3 = getVarK(a, K[250]);
    s2 = mul(Value(3.5), s3);
    setMemK(s0, K[630], s2);
    s0 = R[1];
    { Value av[346] = {Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0), Value(0.0)}; s2 = initArray(346, av); }
    setMemK(s0, K[626], s2);
    { s0 = callFunK(a, K[642], 0, nullptr); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f36_hider(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = Value((Object*)(a.target ? doRoot(a.target) : (DisplayObject*)vm.root));
    Value s0, s1, s2, s3, s4, s5;
    s0 = R[1];
    s0 = getMemK(s0, K[667]);
    s0 = notv(s0);
    s0 = notv(s0);
    if (truthy(s0)) goto L_end;
    s0 = R[1];
    s5 = R[1];
    { Value av[2] = {Value(K[668]), Value(100.0)}; s2 = callMethodK(a, s5, K[66], 2, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    setMemK(s0, K[235], s2);
    s0 = R[1];
    setMemK(s0, K[667], VTRUE);
L_end:;
    vm.sp = sp0; return VUNDEF;
}

Value f37_itmc(Act& caller, Function* self, Value thisv, int argc, Value* argv) {
    Act a;
    Scope L_; enterFunction(a, L_, caller, self, caller.thisv);
    Value* sp0 = vm.sp; (void)argv; (void)argc;
    Value R[4];
    R[1] = Value((Object*)(a.target ? doRoot(a.target) : (DisplayObject*)vm.root));
    Value s0, s1, s2;
    s0 = R[1];
    s0 = getMemK(s0, K[669]);
    s1 = getVarK(a, K[100]);
    s0 = getMem(s0, s1);
    s0 = greater(s0, Value(0.0));
    s0 = notv(s0);
    if (truthy(s0)) goto L_1e7d037;
    s0 = R[1];
    s0 = getMemK(s0, K[669]);
    s1 = getVarK(a, K[100]);
    s0 = getMem(s0, s1);
    s2 = getVarK(a, K[670]);
    s2 = getMemK(s2, K[671]);
    { Value av[1] = {s0}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
    goto L_end;
L_1e7d037:;
    s2 = getVarK(a, K[670]);
    s2 = getMemK(s2, K[671]);
    { Value av[1] = {Value(K[672])}; s0 = callMethodK(a, s2, K[11], 1, av); }
    if (FL_UNLIKELY(baseRemoved(a))) { vm.sp = sp0; return VUNDEF; }
L_end:;
    vm.sp = sp0; return VUNDEF;
}

}}  // namespace fl::og
