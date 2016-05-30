/* src/vm/jit/arm/md-abi.hpp - defines for arm ABI

   Copyright (C) 1996-2012
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


#ifndef MD_ABI_HPP_
#define MD_ABI_HPP_ 1

#include "config.h"


/* preallocated registers *****************************************************/

/* integer registers */

#define REG_RESULT      0    /* to deliver method results                     */
#define REG_RESULT2     1    /* to deliver long method results                */

#define REG_PV          12   /* intra-procedure-call scratch register         */
#define REG_SP          13   /* stack pointer                                 */
#define REG_LR          14   /* link register                                 */
#define REG_PC          15   /* program counter                               */

#define REG_METHODPTR   11   /* special register to fetch procedure vector    */

#define REG_ITMP1       10   /* temporary register                            */
#define REG_ITMP2       11   /* temporary register and method pointer         */
#define REG_ITMP3       9    /* temporary register                            */

#define REG_ITMP1_XPTR  10   /* exception pointer = temporary register 1      */
#define REG_ITMP2_XPC   11   /* exception pc = temporary register 2           */

#define REG_A0           0   /* define some argument registers                */
#define REG_A1           1
#define REG_A2           2
#define REG_A3           3

#define REG_SPLIT       16   /* dummy register to mark a split of longs and   */
                             /* doubles across register/stack barrier         */
                             /* set LOW_REG = ARG4 and HIGH_REG = REG_SPLIT   */

#define BITMASK_ARGS    0x0f /* bitmask for LDM/STM to save method arguments  */
#define BITMASK_RESULT  0x03 /* bitmask for LDM/STM to save method results    */

#if !defined(ENABLE_SOFTFLOAT)
/* floating point registers */

#define REG_FRESULT     0    /* to deliver floating point method results      */
#define REG_FTMP1       6    /* temporary floating point register             */
#define REG_FTMP2       7    /* temporary floating point register             */


/* The ARM hardfloat calling conventions are quite unique in that two 32-bit
 * float arguments can be assigned to the separate halves of one 64-bit
 * register. In assembler code, the float registers are laid out such that
 * s0/s1 == d0, s2/s3 == d1 and so on. In machine code, however, the numbering
 * of the single precision registers is so that s0/s16 == d0, s1/s17 == d1 and
 * so on. That's where this constant comes from. The second halves are
 * accessed by a higher number range. The constant is named this way because
 * the upper range of single-precision registers is only used for back-filled
 * float arguments passed to native functions. */

#define BACKFILL_OFFSET 16

#endif /* !defined(ENABLE_SOFTFLOAT) */


/* register count *************************************************************/
#define INT_REG_CNT     16   /* number of integer registers                   */
#define INT_TMP_CNT     0    /* number of integer registers                   */
#define INT_SAV_CNT     5    /* number of int callee saved registers          */
#define INT_ARG_CNT     4    /* number of int argument registers              */
#define INT_RES_CNT     7    /* number of reserved integer registers          */

#if defined(ENABLE_SOFTFLOAT)
# define FLT_REG_CNT    8    /* number of float registers                     */
# define FLT_TMP_CNT    0    /* number of flt temp registers                  */
# define FLT_SAV_CNT    0    /* number of flt callee saved registers          */
# define FLT_ARG_CNT    0    /* number of flt argument registers              */
# define FLT_RES_CNT    8    /* number of reserved float registers            */
#else
#if !defined(__ARMHF__)
# define FLT_REG_CNT    8    /* number of float registers                     */
# define FLT_TMP_CNT    6    /* number of flt temp registers                  */
# define FLT_SAV_CNT    0    /* number of flt callee saved registers          */
# define FLT_ARG_CNT    0    /* number of flt argument registers              */
# define FLT_RES_CNT    2    /* number of reserved float registers            */
#else
# define FLT_REG_CNT   16    /* number of float registers                     */
# define FLT_TMP_CNT    0    /* number of flt temp registers                  */
# define FLT_SAV_CNT    8    /* number of flt callee saved registers          */
# define FLT_ARG_CNT    6    /* number of flt argument registers              */
# define FLT_RES_CNT    2    /* number of reserved float registers            */
#endif
#endif /* defined(ENABLE_SOFTFLOAT) */


/* Register Pack/Unpack Macros ************************************************/

#if defined(__ARMEL__)

# define REG_ITMP12_PACKED    PACK_REGS(REG_ITMP1, REG_ITMP2)
# define REG_ITMP23_PACKED    PACK_REGS(REG_ITMP2, REG_ITMP3)
# define REG_RESULT_PACKED    PACK_REGS(REG_RESULT, REG_RESULT2)

# define REG_A0_A1_PACKED     PACK_REGS(REG_A0, REG_A1)
# define REG_A2_A3_PACKED     PACK_REGS(REG_A2, REG_A3)

#else /* defined(__ARMEB__) */

# define REG_ITMP12_PACKED    PACK_REGS(REG_ITMP2, REG_ITMP1)
# define REG_ITMP23_PACKED    PACK_REGS(REG_ITMP3, REG_ITMP2)
# define REG_RESULT_PACKED    PACK_REGS(REG_RESULT2, REG_RESULT)

# define REG_A0_A1_PACKED     PACK_REGS(REG_A1, REG_A0)
# define REG_A2_A3_PACKED     PACK_REGS(REG_A3, REG_A2)

#endif

#define REG_ITMP12_TYPED(t) ((IS_2_WORD_TYPE(t)) ? REG_ITMP12_PACKED : REG_ITMP1)
#define REG_RESULT_TYPED(t) ((IS_2_WORD_TYPE(t)) ? REG_RESULT_PACKED : REG_RESULT)

#endif // MD_ABI_HPP_


/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */
