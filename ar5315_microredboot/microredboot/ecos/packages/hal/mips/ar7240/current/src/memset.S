/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1998, 1999, 2000 by Ralf Baechle
 * Copyright (C) 1999, 2000 Silicon Graphics, Inc.
 * Copyright (C) 2007  Maciej W. Rozycki
 */

#include <pkgconf/system.h>
#include <pkgconf/hal.h>

#ifdef CYGPKG_KERNEL
# include <pkgconf/kernel.h>
#endif

#include <cyg/hal/arch.inc>
#include <cyg/hal/plf_io.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/plf_defs.inc>
#include CYGBLD_HAL_PLATFORM_H


#include "asm.h"


#define ta0     t4
#define ta1     t5
#define ta2     t6
#define ta3     t7


#  define LWHI  lwl     /* high part is left in big-endian  */
#  define SWHI  swl     /* high part is left in big-endian  */
#  define LWLO  lwr     /* low part is right in big-endian  */
#  define SWLO  swr     /* low part is right in big-endian  */



#define ENTRY(name) \
  .globl name;                                                                \
  .align 2;                                                                   \
  .ent name,0;                                                                \
  name/* use a comment rather than ## to workaround bug in gcc-3.4.x */:

#undef END
#define END(function)                     \
  .end    function;                       \
  .size   function,.-function


ENTRY (memset)
       .set noreorder
        blt     a2, 12, smallclr        # small amount to clear?
        move    v0, a0                  # save s1 for result

        sll     t1, a1, 8               # compute  c << 8 in t1
        or      t1, t1, a1              # compute c << 8 | c in 11
        sll     t2, t1, 16              # shift that left 16
        or      t1, t2, t1              # or together

        SUBU    t0, zero, a0            # compute # bytes to word align address
        and     t0, t0, 3
        beq     t0, zero, 1f            # skip if word aligned
        SUBU    a2, a2, t0              # subtract from remaining count
        SWHI    t1, 0(a0)               # store 1, 2, or 3 bytes to align
        ADDU    a0, a0, t0
1:
        and     v1, a2, 3               # compute number of whole words left
        SUBU    t0, a2, v1
        SUBU    a2, a2, t0
        ADDU    t0, t0, a0              # compute ending address
2:
        ADDU    a0, a0, 4               # clear words
        bne     a0, t0, 2b              #  unrolling loop does not help
        sw      t1, -4(a0)              #  since we are limited by memory speed

smallclr:
        ble     a2, zero, 2f
        ADDU    t0, a2, a0              # compute ending address
1:
        ADDU    a0, a0, 1               # clear bytes
        bne     a0, t0, 1b
        sb      a1, -1(a0)
2:
        j       ra
        nop
        .set reorder
END (memset)

