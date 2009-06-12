#include <features.h>

#define ALIGN   2

/* Note: .abicalls goes at top of routine, and only one of them. */
#if defined(USER_ABICALLS)
#define ABISETUP                                \
        .set    noreorder;                      \
        .cpload t9;                             \
        .set    reorder;

        .abicalls
#else
#define ABISETUP
#endif

#ifndef __UCLIBC_USE_UNIFIED_SYSCALL__
#define SYSCALL__(name)                         \
        .text;                                  \
        .align  ALIGN;                          \
        .globl  name;                           \
        .ent    name, 0;                        \
  name: ;                                       \
        ABISETUP                                \
        li      v0, __NR_##name ;               \
        syscall;                                \
        la      t3, errno;                      \
        beqz    a3, 1f;                         \
        negu    a1, v0;                         \
        sw      a1, 0(t3);                      \
        li      v0, -1;                         \
  1: ;                                          \
        j ra;                                   \
        .end    name;                           \
        .size   name,.-name;
#else   /* Unified syscall */
#define SYSCALL__(name)                         \
        .text;                                  \
/*      .align  ALIGN;*/                        \
        .globl  name;                           \
        .ent    name, 0;                        \
  name: ;                                       \
        ABISETUP                                \
        .set    push;                           \
        .set    noreorder;                      \
        j       __uClibc_syscall;               \
        addiu   v0, zero, __NR_##name ;         \
        .set    pop;                            \
        .end    name;                           \
        .size   name,.-name;
#endif  /* __UCLIBC_USE_UNIFIED_SYSCALL__ */

#undef ALIGN
#define __ASSEMBLY__
#include <asm/asm.h>
#include <asm/unistd.h>
#include <asm/mipsregs.h>
#include <asm/regdef.h>
/* #include <asm/stackframe.h> */

#undef ALIGN
#define ALIGN   2
#define _SYSCALL0(type,name)    SYSCALL__(name)
_SYSCALL0(pid_t, fork);

