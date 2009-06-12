/*
 * linux/asm/assembler.h
 *
 * This file contains arm architecture specific defines
 * for the different processors.
 *
 * Do not include any C declarations in this file - it is included by
 * assembler source.
 */
#ifndef __ASSEMBLY__
#error "Only include this from assembly code"
#endif

#include <asm/proc/ptrace.h>
#include <asm/proc/assembler.h>

/*
 * Endian independent macros for shifting bytes within registers.
 */
#ifndef __ARMEB__
#define pull            lsr
#define push            lsl
#define byte(x)         (x*8)
#else
#define pull            lsl
#define push            lsr
#define byte(x)         ((3-x)*8)
#endif

