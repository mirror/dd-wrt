/* pipe system call for Linux/MIPS */

/*see uClibc's sh/pipe.c and glibc-2.2.4's mips/pipe.S */

#include <features.h>
#include <asm/asm.h>
#include <asm/unistd.h>
#include <asm/regdef.h>

        .globl  pipe
        .ent    pipe, 0
pipe:
#ifdef PROF
        .set            noat
        move            $1,ra
        subu            sp,sp,8         # _mcount pops 2 words from  stack
        jal             _mcount
        .set            at
#endif
        addiu   sp,sp,-24
        sw      a0,16(sp)
        li      v0,__NR_pipe
        syscall
        beqz    a3, 1f
        la      t3, errno
        sw      v0, (t3)
        li      v0, -1
        b       2f
1:
        lw      a0, 16(sp)
        sw      v0, 0(a0)
        sw      v1, 4(a0)
        li      v0, 0
2:
        addiu   sp,sp,24
        j       ra
        .end    pipe
        .size   pipe,.-pipe
