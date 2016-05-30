/* src/vm/jit/intrp/dynamic-super.c  dynamic superinstruction support

   Copyright (C) 1995,1996,1997,1998,2000,2003,2004 Free Software Foundation, Inc.
   Taken from Gforth.

   Copyright (C) 1996-2013
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO

   This file is part of CACAO.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

*/


#define NO_IP 0 /* proper native code, without interpreter's ip */

#include "config.h"

#include <alloca.h>
#include <stdlib.h>
#include <assert.h>

#include "vm/types.hpp"

#include "mm/memory.hpp"

#include "threads/lock.hpp"

#include "toolbox/hashtable.hpp"
#include "toolbox/logging.hpp"

#include "vm/options.hpp"

#include "vm/jit/disass.hpp"
#include "vm/jit/intrp/intrp.h"


s4 no_super=0;   /* option: just use replication, but no dynamic superinsts */

static char MAYBE_UNUSED superend[]={
#include <java-superend.i>
};

const char * const prim_names[]={
#include <java-names.i>
};

enum {
#define INST_ADDR(_inst) N_##_inst
#include <java-labels.i>
#undef INST_ADDR
};

#define MAX_IMMARGS 1

typedef struct {
  Label start; /* NULL if not relocatable */
  s4 length; /* only includes the jump iff superend is true*/
  s4 restlength; /* length of the rest (i.e., the jump or (on superend) 0) */
  char superend; /* true if primitive ends superinstruction, i.e.,
                     unconditional branch, execute, etc. */
  s4 nimmargs;
  struct immarg {
    s4 offset; /* offset of immarg within prim */
    char rel;    /* true if immarg is relative */
  } immargs[MAX_IMMARGS];
} PrimInfo;

PrimInfo *priminfos;
s4 npriminfos;
Label before_goto;
u4 goto_len;

typedef struct superstart {
  struct superstart *next;
  s4 patcherm;  /* offset of patcher, -1 if super has no patcher */
  u4 length;    /* length of superinstruction */
  u1 *oldsuper; /* reused superinstruction: NULL if there is none */
  s4 dynsuperm;
  s4 dynsupern;
  u1 *mcodebase;
  u1 *ncodebase;
} superstart;

static hashtable hashtable_patchersupers;
#define HASHTABLE_PATCHERSUPERS_BITS 14

#if defined(ENABLE_THREADS)
static java_objectheader *lock_hashtable_patchersupers;
#endif

/* stuff for -no-replication */
typedef struct superreuse {
  struct superreuse *next;
  u1 *code;
  u4 length;
} superreuse;

static hashtable hashtable_superreuse;
#define HASHTABLE_SUPERREUSE_BITS 14

#if defined(ENABLE_THREADS)
static java_objectheader *lock_hashtable_superreuse;
#endif

# define debugp(x...) if (opt_verbose) fprintf(x)
#define debugp1(x...)

/* statistics variables */

u4 count_supers        = 0; /* dynamic superinstructions, including replicas */
u4 count_supers_unique = 0; /* dynamic superinstructions, without replicas */
u4 count_supers_reused = 0; /* reused dynamic superinstructions */
u4 count_patchers_exec = 0; /* executed patchers */
u4 count_patchers_last = 0; /* executed last patchers */
u4 count_patchers_ins  = 0; /* patchers inserted in patchersupers table */
u4 count_patchers      = 0; /* superstarts with patcherm!=-1 */
u4 count_supers_nopatch= 0; /* superinstructions for code without patchers */
u4 count_supers_patch  = 0; /* superinstructions for code with patchers */
u4 count_dispatches    = 0; /* dynamic superinstructions generated */
u4 count_disp_nonreloc = 0; /* */
u4 count_insts_reloc   = 0; /* relocatable insts compiled (append_prim) */
u4 count_insts         = 0; /* compiled insts (gen_inst) */
u4 count_native_code   = 0; /* native code bytes */
u4 count_native_saved  = 0; /* reclaimed native code */

/* determine priminfos */


java_objectheader *engine2(Inst *ip0, Cell * sp, Cell * fp);

static int compare_labels(const void *pa, const void *pb)
{
  char *a = *(char **)pa;
  char *b = *(char **)pb;
  return a-b;
}

static Label bsearch_next(Label key, Label *a, u4 n)
     /* a is sorted; return the label >=key that is the closest in a;
        return NULL if there is no label in a >=key */
{
  int mid = (n-1)/2;
  if (n<1)
    return NULL;
  if (n == 1) {
    if (a[0] < key)
      return NULL;
    else
      return a[0];
  }
  if (a[mid] < key)
    return bsearch_next(key, a+mid+1, n-mid-1);
  else
    return bsearch_next(key, a, mid+1);
}

#if 0
  each VM instruction <x> looks like this:
  H_<x>:
    <skip>
  I_<x>: /* entry point */
    ...
  J_<x>: /* start of dispatch */
    NEXT_P1_5; /* dispatch except GOTO */
  K_<x>: /* just before goto */
    DO_GOTO; /* goto part of dispatch */

/* 
 * We need I_<x> for threaded code: this is the code address stored in
 * direct threaded code.

 * We need the <skip> to detect non-relocatability (by comparing
 * engine and engine2 with different skips).  H_<x> is there to ensure
 * that gcc does not think that <skip> is dead code.

 * We need J_<x> to implement dynamic superinstructions: copy
 * everything between I_<x> and J_<x> to get a VM instruction without
 * dispatch.

 * At the end of a dynamic superinstruction we need a dispatch.  We
 * need the DO_GOTO to work around gcc bug 15242: we copy everything
 * up to K_<x> and then copy the indirect jump from &&before_goto.
 */

#endif

static void check_prims(Label symbols1[])
{
  int i;
  Label *symbols2, *symbols3, *js1, *ks1, *ks1sorted;
  Label after_goto, before_goto2;

  if (opt_verbose)
#ifdef __VERSION__
    fprintf(stderr, "Compiled with gcc-" __VERSION__ "\n");
#endif
  for (i=0; symbols1[i]!=0; i++)
    ;
  npriminfos = i;
  
#ifndef NO_DYNAMIC
  if (opt_no_dynamic)
    return;
  symbols2=(Inst *)engine2(0,0,0);
#if NO_IP
  symbols3=(Inst *)engine3(0,0,0);
#else
  symbols3=symbols1;
#endif
  js1 = symbols1+i+1;
  ks1 = js1+i;
  before_goto = ks1[i+1]; /* after ks and after after_last */
  after_goto = ks1[i+2];
  before_goto2 = (ks1+(symbols2-symbols1))[i+1];
  /* some gccs reorder the code; below we look for the next K label
     with binary search, and whether it belongs to the current
     instruction; prepare for this by sorting the K labels */
  ks1sorted = (Label *)alloca((i+1)*sizeof(Label));
  MCOPY(ks1sorted,ks1,Label,i+1);
  qsort(ks1sorted, i+1, sizeof(Label), compare_labels);

  /* check whether the "goto *" is relocatable */
  goto_len = after_goto-before_goto;
  debugp(stderr, "goto * %p %p len=%d\n",
	 before_goto,before_goto2,goto_len);
  if (memcmp(before_goto, before_goto2, goto_len)!=0) { /* unequal */
    opt_no_dynamic = true;
    debugp(stderr,"  not relocatable, disabling dynamic code generation\n");
    return;
  }
  
  priminfos = calloc(i,sizeof(PrimInfo));
  for (i=0; symbols1[i]!=0; i++) {
    /* check whether the VM instruction i has the same code in
       symbols1 and symbols2 (then it is relocatable).  Also, for real
       native code (not activated), check where the versions in
       symbols1 and symbols 3 differ; these are places for
       patching. */
    int prim_len = js1[i]-symbols1[i];
    PrimInfo *pi=&priminfos[i];
    int j=0;
    char *s1 = (char *)symbols1[i];
    char *s2 = (char *)symbols2[i];
    char *s3 = (char *)symbols3[i];
    Label endlabel = bsearch_next(s1+1,ks1sorted,npriminfos+1);

    pi->start = s1;
    pi->superend = superend[i]|no_super;
    pi->length = prim_len;
    pi->restlength = endlabel - symbols1[i] - pi->length;
    pi->nimmargs = 0;

    debugp(stderr, "%-15s %3d %p %p %p len=%3ld restlen=%2ld s-end=%1d",
	      prim_names[i], i, s1, s2, s3, (long)(pi->length), (long)(pi->restlength), pi->superend);
    if (endlabel == NULL) {
      pi->start = NULL; /* not relocatable */
      if (pi->length<0) pi->length=1000;
      debugp(stderr,"\n   non_reloc: no K label > start found\n");
      continue;
    }
    if (js1[i] > endlabel && !pi->superend) {
      pi->start = NULL; /* not relocatable */
      pi->length = endlabel-symbols1[i];
      debugp(stderr,"\n   non_reloc: there is a K label before the J label (restlength<0)\n");
      continue;
    }
    if (js1[i] < pi->start && !pi->superend) {
      pi->start = NULL; /* not relocatable */
      pi->length = endlabel-symbols1[i];
      debugp(stderr,"\n   non_reloc: J label before I label (length<0)\n");
      continue;
    }
    assert(pi->length>=0);
    assert(pi->restlength >=0);
    while (j<(pi->length+pi->restlength)) {
      if (s1[j]==s3[j]) {
	if (s1[j] != s2[j]) {
	  pi->start = NULL; /* not relocatable */
	  debugp(stderr,"\n   non_reloc: engine1!=engine2 offset %3d",j);
	  /* assert(j<prim_len); */
	  break;
	}
	j++;
      } else {
        /* only used for NO_IP native code generation */
        /* here we find differences in inline arguments */
        assert(false);
      }
    }
    debugp(stderr,"\n");
  }
#endif
}

static bool is_relocatable(ptrint p)
{
  return !opt_no_dynamic && priminfos[p].start != NULL;
}

static
void append_prim(codegendata *cd, ptrint p)
{
  PrimInfo *pi = &priminfos[p];
  debugp1(stderr,"append_prim %p %s\n",cd->lastmcodeptr, prim_names[p]);
  if (cd->ncodeptr + pi->length + pi->restlength + goto_len >
      cd->ncodebase + cd->ncodesize) {
    cd->ncodeptr = codegen_ncode_increase(cd, cd->ncodeptr);
  }
  memcpy(cd->ncodeptr, pi->start, pi->length);
  cd->ncodeptr += pi->length;
  count_insts_reloc++;
}

static void init_dynamic_super(codegendata *cd)
{
  cd->lastpatcheroffset = -1;
  cd->dynsuperm = -1;  /* the next VM instr may be non-relocatable */
  cd->dynsupern = cd->ncodeptr - cd->ncodebase;
}


/******************* -no-replication stuff **********************/

/* bugs: superinstructions are inserted into the table only after the
   end of a method, so there may be replication within a method even
   with -no-replication.  Moreover, there is a race condition that can
   cause varying amounts of replication between different threads;
   this could only be avoided by eliminating the bug above and having
   more locking. */

static u4 hash_superreuse(u1 *code, u4 length)
     /* calculates a hash value for given code length */
{
  u4 r=0;
  u4 i;

  for (i=0; i<(length&(~3)); i+=sizeof(u4)) {
    r += *(s4 *)(code+i); /* !! align each superinstruction */
  }
  return (r+(r>>HASHTABLE_SUPERREUSE_BITS))&((1<<HASHTABLE_SUPERREUSE_BITS)-1);
}

static void superreuse_insert(u1 *code, u4 length)
{
  u4 slot = hash_superreuse(code, length);
  superreuse **listp = (superreuse **)&hashtable_superreuse.ptr[slot];
  superreuse *sr = NEW(superreuse);
  sr->code = code;
  sr->length = length;

  LOCK_MONITOR_ENTER(lock_hashtable_superreuse);

  sr->next = *listp;
  *listp = sr;

  LOCK_MONITOR_EXIT(lock_hashtable_superreuse);

  count_supers_unique++;
}

static u1 *superreuse_lookup(u1 *code, u4 length)
     /* returns earlier instances code address, or NULL */
{
  u4 slot = hash_superreuse(code, length);
  superreuse *sr = (superreuse *)hashtable_superreuse.ptr[slot];
  /* since there is another race condition anyway, we don't bother
     locking here; both race conditions don't affect the correctness */
  for (; sr != NULL; sr = sr->next)
    if (length == sr->length && memcmp(code, sr->code, length)==0)
      return sr->code;
  return NULL;
}

/******************* patcher stuff *******************************/

void patchersuper_rewrite(Inst *p)
     /* p is the address of a patcher; if this is the last patcher in
        a dynamic superinstruction, rewrite the threaded code to use
        the dynamic superinstruction; the data for that comes from
        hashtable_patchersupers */
{
  ptrint key = (ptrint)p;
  u4 slot = ((key + (key>>HASHTABLE_PATCHERSUPERS_BITS)) & 
             ((1<<HASHTABLE_PATCHERSUPERS_BITS)-1));
  superstart **listp = (superstart **)&hashtable_patchersupers.ptr[slot];
  superstart *ss;
  count_patchers_exec++;

  LOCK_MONITOR_ENTER(lock_hashtable_patchersupers);

  for (; ss=*listp,  ss!=NULL; listp = &(ss->next)) {
    if (p == ((Inst *)(ss->mcodebase + ss->patcherm))) {
      Inst target;
      if (ss->oldsuper != NULL)
        target = (Inst)ss->oldsuper;
      else
        target = (Inst)(ss->ncodebase + ss->dynsupern);
      *(Inst *)(ss->mcodebase + ss->dynsuperm) = target;
      debugp1(stderr, "patcher rewrote %p into %p\n", 
             ss->mcodebase + ss->dynsuperm, target);
      *listp = ss->next;
      FREE(ss, superstart);
      count_patchers_last++;
      break;
    }
  }

  LOCK_MONITOR_EXIT(lock_hashtable_patchersupers);
}

static void hashtable_patchersupers_insert(superstart *ss)
{
  ptrint key = (ptrint)(ss->mcodebase + ss->patcherm);
  u4 slot = ((key + (key>>HASHTABLE_PATCHERSUPERS_BITS)) & 
             ((1<<HASHTABLE_PATCHERSUPERS_BITS)-1));
  void **listp = &hashtable_patchersupers.ptr[slot];

  LOCK_MONITOR_ENTER(lock_hashtable_patchersupers);

  ss->next = (superstart *)*listp;
  *listp = (void *)ss;

  LOCK_MONITOR_EXIT(lock_hashtable_patchersupers);

  count_patchers_ins++;
}

void dynamic_super_rewrite(codegendata *cd)
     /* rewrite the threaded code in the superstarts list to point to
        the dynamic superinstructions */
{
  superstart *ss = cd->superstarts;
  superstart *ssnext;
  for (; ss != NULL; ss = ssnext) {
    ssnext = ss->next;
    if (opt_no_replication && ss->oldsuper == NULL)
      superreuse_insert(cd->ncodebase + ss->dynsupern, ss->length);
    if (ss->patcherm == -1) {
      assert(ss->oldsuper == NULL);
      *(Inst *)(cd->mcodebase + ss->dynsuperm) =
        (Inst)(cd->ncodebase + ss->dynsupern);
      debugp1(stderr, "rewrote %p into %p\n", 
             cd->mcodebase + ss->dynsuperm, cd->ncodebase + ss->dynsupern);
      count_supers_nopatch++;
    } else {
      /* fprintf(stderr,"%p stage2\n", ss); */
      ss->mcodebase = cd->mcodebase;
      ss->ncodebase = cd->ncodebase;
      hashtable_patchersupers_insert(ss);
      count_supers_patch++;
    }
  }
}

static void new_dynamic_super(codegendata *cd)
     /* add latest dynamic superinstruction to the appropriate table
        for rewriting the threaded code either on relocation (if there
        is no patcher), or when the last patcher is executed (if there
        is a patcher). */
{
  superstart *ss;
  u1 *oldsuper = NULL;
  u1 *code = cd->ncodebase + cd->dynsupern;
  u4 length = cd->ncodeptr - code;
  count_supers++;
  if (opt_no_replication) {
    oldsuper = superreuse_lookup(code, length);
    if (oldsuper != NULL) {
      count_supers_reused++;
      count_native_saved += length;
      cd->ncodeptr = code;
      if (cd->lastpatcheroffset == -1) {
        *(Inst *)(cd->mcodebase + cd->dynsuperm) = (Inst)oldsuper;
        debugp1(stderr, "rewrote %p into %p (reused)\n", 
                cd->mcodebase + ss->dynsuperm, oldsuper);
        return;
      }
    }
  }
  if (cd->lastpatcheroffset == -1) {
    ss = DNEW(superstart);
  } else {
    ss = NEW(superstart);
    count_patchers++;
    /* fprintf(stderr,"%p stage1\n", ss); */
  }
  ss->patcherm = cd->lastpatcheroffset;
  ss->oldsuper = oldsuper;
  ss->length   = length;
  ss->dynsuperm = cd->dynsuperm;
  ss->dynsupern = cd->dynsupern;
  ss->next = cd->superstarts;
  cd->superstarts = ss;
  count_native_code += cd->ncodeptr - code;
}

void append_dispatch(codegendata *cd)
{
  debugp1(stderr,"append_dispatch %p\n",cd->lastmcodeptr);
  if (cd->lastinstwithoutdispatch != ~0) {
    PrimInfo *pi = &priminfos[cd->lastinstwithoutdispatch];
    
    memcpy(cd->ncodeptr, pi->start + pi->length, pi->restlength);
    cd->ncodeptr += pi->restlength;
    memcpy(cd->ncodeptr, before_goto, goto_len);
    cd->ncodeptr += goto_len;
    cd->lastinstwithoutdispatch = ~0;
    new_dynamic_super(cd);
    count_dispatches++;
  }
  init_dynamic_super(cd);
}

static void compile_prim_dyn(codegendata *cd, ptrint p)
     /* compile prim #p dynamically (mod flags etc.)  */
{
  if (opt_no_dynamic)
    return;
  assert(p<npriminfos);
  if (!is_relocatable(p)) {
    if (cd->lastinstwithoutdispatch != ~0) /* only count real dispatches */
      count_disp_nonreloc++;
    append_dispatch(cd); /* to previous dynamic superinstruction */
    return;
  }
  if (cd->dynsuperm == -1)
    cd->dynsuperm = cd->lastmcodeptr - cd->mcodebase;
  append_prim(cd, p);
  cd->lastinstwithoutdispatch = p;
  if (priminfos[p].superend)
    append_dispatch(cd);
  return;
}

static void replace_patcher(codegendata *cd, ptrint p)
     /* compile p dynamically, and note that there is a patcher here */
{
  if (opt_no_quicksuper) {
    append_dispatch(cd);
  } else {
    cd->lastpatcheroffset = cd->lastmcodeptr - cd->mcodebase;
    compile_prim_dyn(cd, p);
  }
}

void gen_inst1(codegendata *cd, ptrint instr)
{
  /* actually generate the threaded code instruction */

  debugp1(stderr, "gen_inst %p, %s\n", cd->lastmcodeptr, prim_names[instr]);
  *((Inst *) cd->lastmcodeptr) = vm_prim[instr];
  switch (instr) {
  case N_PATCHER_ACONST:         replace_patcher(cd, N_ACONST1); break;
  case N_PATCHER_ARRAYCHECKCAST: replace_patcher(cd, N_ARRAYCHECKCAST); break;
  case N_PATCHER_GETSTATIC_INT:  replace_patcher(cd, N_GETSTATIC_INT); break;
  case N_PATCHER_GETSTATIC_LONG: replace_patcher(cd, N_GETSTATIC_LONG); break;
  case N_PATCHER_GETSTATIC_CELL: replace_patcher(cd, N_GETSTATIC_CELL); break;
  case N_PATCHER_PUTSTATIC_INT:  replace_patcher(cd, N_PUTSTATIC_INT); break;
  case N_PATCHER_PUTSTATIC_LONG: replace_patcher(cd, N_PUTSTATIC_LONG); break;
  case N_PATCHER_PUTSTATIC_CELL: replace_patcher(cd, N_PUTSTATIC_CELL); break;
  case N_PATCHER_GETFIELD_INT:   replace_patcher(cd, N_GETFIELD_INT); break;
  case N_PATCHER_GETFIELD_LONG:  replace_patcher(cd, N_GETFIELD_LONG); break;
  case N_PATCHER_GETFIELD_CELL:  replace_patcher(cd, N_GETFIELD_CELL); break;
  case N_PATCHER_PUTFIELD_INT:   replace_patcher(cd, N_PUTFIELD_INT); break;
  case N_PATCHER_PUTFIELD_LONG:  replace_patcher(cd, N_PUTFIELD_LONG); break;
  case N_PATCHER_PUTFIELD_CELL:  replace_patcher(cd, N_PUTFIELD_CELL); break;
  case N_PATCHER_MULTIANEWARRAY: replace_patcher(cd, N_MULTIANEWARRAY); break;
  case N_PATCHER_INVOKESTATIC:   replace_patcher(cd, N_INVOKESTATIC); break;
  case N_PATCHER_INVOKESPECIAL:  replace_patcher(cd, N_INVOKESPECIAL); break;
  case N_PATCHER_INVOKEVIRTUAL:  replace_patcher(cd, N_INVOKEVIRTUAL); break;
  case N_PATCHER_INVOKEINTERFACE:replace_patcher(cd, N_INVOKEINTERFACE); break;
  case N_PATCHER_CHECKCAST:      replace_patcher(cd, N_CHECKCAST); break;
  case N_PATCHER_INSTANCEOF:     replace_patcher(cd, N_INSTANCEOF); break;
  case N_PATCHER_NATIVECALL:     
    if (opt_verbosecall)
      replace_patcher(cd, N_TRACENATIVECALL);
    else
      replace_patcher(cd, N_NATIVECALL);
    break;
  case N_TRANSLATE: break;

  default:
    compile_prim_dyn(cd, instr);
    break;
  }
  count_insts++;
}

void finish_ss(codegendata *cd)
     /* conclude the last static super and compile it dynamically */
{
#if 1
  if (cd->lastmcodeptr != NULL) {
    gen_inst1(cd, *(ptrint *)(cd->lastmcodeptr));
    cd->lastmcodeptr = NULL;
  }
#endif
}

#if 0
void gen_inst(codegendata *cd, ptrint instr)
{
  cd->lastmcodeptr = cd->mcodeptr;
  gen_inst1(cd, instr);
  cd->mcodeptr += sizeof(Inst);
  cd->lastmcodeptr = NULL;
}
#else
void gen_inst(codegendata *cd, ptrint instr)
{
  /* vmgen-0.6.2 generates gen_... calls with Inst ** as first
     parameter, but we need to pass in cd to make lastmcodeptr
     thread-safe */
  ptrint *lastmcodeptr = (ptrint *)cd->lastmcodeptr;
  
  if (lastmcodeptr != NULL) {
    ptrint combo;

    assert((u1 *) lastmcodeptr < cd->mcodeptr &&
           cd->mcodeptr < (u1 *) (lastmcodeptr + 40));

    combo = peephole_opt(*lastmcodeptr, instr, peeptable);

    if (combo != -1) {
      *lastmcodeptr = combo;
      return;
    }
    finish_ss(cd);
  }

  /* actually generate the threaded code instruction */

  *((Inst *) cd->mcodeptr) = (Inst) instr;

  cd->lastmcodeptr = cd->mcodeptr;
  cd->mcodeptr += sizeof(Inst);
}
#endif

void print_dynamic_super_statistics(void)
{
  dolog("count_supers        = %d", count_supers        );
  dolog("count_supers_unique = %d", count_supers_unique );
  dolog("count_supers_reused = %d", count_supers_reused );
  dolog("count_patchers_exec = %d", count_patchers_exec );
  dolog("count_patchers_last = %d", count_patchers_last );
  dolog("count_patchers_ins  = %d", count_patchers_ins  );
  dolog("count_patchers      = %d", count_patchers      );
  dolog("count_supers_nopatch= %d", count_supers_nopatch);
  dolog("count_supers_patch  = %d", count_supers_patch  );
  dolog("count_dispatches    = %d", count_dispatches    );
  dolog("count_disp_nonreloc = %d", count_disp_nonreloc );
  dolog("count_insts_reloc   = %d", count_insts_reloc   );
  dolog("count_insts         = %d", count_insts         );
  dolog("count_native_code   = %d", count_native_code   );
  dolog("count_native_saved  = %d", count_native_saved  );
}

#if defined(ENABLE_DISASSEMBLER)
void disassemble_prim(int n)
{
  PrimInfo *p = &(priminfos[n]);
  u1 *start = vm_prim[n];

#if defined(ENABLE_JIT)
  printf("%s (len=%d, restlen=%d):\n",prim_names[n],p->length, p->restlength);
  disassemble(start, start + p->length + p->restlength);
#endif
}
#endif /* defined(ENABLE_DISASSEMBLER) */

void dynamic_super_init(void)
{
  check_prims(vm_prim);

#if defined(ENABLE_DISASSEMBLER)
  if (opt_verbose) {
    disassemble_prim(N_IADD);
    disassemble_prim(N_ILOAD);
    disassemble_prim(N_GETFIELD_INT);
  }
#endif

  hashtable_create(&hashtable_patchersupers, 1<<HASHTABLE_PATCHERSUPERS_BITS);
  if (opt_no_replication)
    hashtable_create(&hashtable_superreuse,  1<<HASHTABLE_SUPERREUSE_BITS);

#if defined(ENABLE_THREADS)
  /* create patchersupers hashtable lock object */

  lock_hashtable_patchersupers = NEW(java_objectheader);
  lock_hashtable_superreuse  = NEW(java_objectheader);

  lock_init_object_lock(lock_hashtable_patchersupers);
  lock_init_object_lock(lock_hashtable_superreuse);
#endif
}
