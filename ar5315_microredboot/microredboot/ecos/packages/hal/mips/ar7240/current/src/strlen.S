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


ENTRY (strlen)
       ADDU    v1, a0, 1
1:
       lb      v0, 0(a0)               # get byte from string
       ADDU    a0, a0, 1               # increment pointer
       bne     v0, zero, 1b            # continue if not end
       nop
       j       ra
       SUBU    v0, a0, v1              # compute length - 1 for '\0' char
END (strlen)

