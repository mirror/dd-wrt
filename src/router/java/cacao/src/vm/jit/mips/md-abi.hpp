/* src/vm/jit/mips/md-abi.hpp - defines for MIPS ABI

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


#ifndef MD_ABI_HPP_
#define MD_ABI_HPP_ 1

#include "config.h"


/* preallocated registers *****************************************************/

/* integer registers */
  
#define REG_ZERO        0    /* always zero                                   */

#define REG_RESULT      2    /* to deliver method results                     */

#if SIZEOF_VOID_P == 4
#define REG_RESULT2     3    /* to deliver method results (low 32-bits)       */
#endif

#define REG_ITMP1       1    /* temporary register                            */
#if SIZEOF_VOID_P == 8
#define REG_ITMP2       3    /* temporary register and method pointer         */
#else
#define REG_ITMP2       24   /* temporary register and method pointer         */
#endif
#define REG_ITMP3       25   /* temporary register                            */

#define REG_RA          31   /* return address                                */
#define REG_SP          29   /* stack pointer                                 */
#define REG_GP          28   /* global pointer                                */

#define REG_PV          30   /* procedure vector, must be provided by caller  */
#define REG_METHODPTR   25   /* pointer to the place from where the procedure */
                             /* vector has been fetched                       */
#define REG_ITMP1_XPTR  REG_ITMP1 /* exception pointer = temporary register 1 */
#define REG_ITMP2_XPC   REG_ITMP2 /* exception pc = temporary register 2      */

#define REG_A0           4   /* define some argument registers                */
#define REG_A1           5
#define REG_A2           6
#define REG_A3           7

#if SIZEOF_VOID_P == 8
#define REG_A4           8
#endif

/* floating point registers */

#define REG_FRESULT      0   /* to deliver floating point method results      */

#define REG_IFTMP        1   /* temporary integer and floating point register */


#if SIZEOF_VOID_P == 8

/* MIPS64 defines */

#define REG_FTMP1        1   /* temporary floating point register             */
#define REG_FTMP2        2   /* temporary floating point register             */
#define REG_FTMP3        3   /* temporary floating point register             */

#define REG_FA0         12   /* define some argument registers                */
#define REG_FA1         13
#define REG_FA2         14

#define INT_REG_CNT     32   /* number of integer registers                   */
#define INT_SAV_CNT      8   /* number of int callee saved registers          */
#define INT_ARG_CNT      8   /* number of int argument registers              */
#define INT_TMP_CNT      5   /* number of integer temporary registers         */
#define INT_RES_CNT     10   /* number of integer reserved registers          */
                             /* + 1 REG_RET totals to 32                      */

#define FLT_REG_CNT     32   /* number of float registers                     */
#define FLT_SAV_CNT      4   /* number of flt callee saved registers          */
#define FLT_ARG_CNT      8   /* number of flt argument registers              */
#define FLT_TMP_CNT     16   /* number of float temporary registers           */
#define FLT_RES_CNT      3   /* number of float reserved registers            */
                             /* + 1 REG_RET totals to 32                      */

#define TRACE_ARGS_NUM  8

#else /* SIZEOF_VOID_P == 8 */

/* MIPS32 defines */

#define REG_FTMP1       2    /* temporary floating point register             */
#define REG_FTMP2       4    /* temporary floating point register             */
#define REG_FTMP3       6    /* temporary floating point register             */

#define INT_REG_CNT     32   /* number of integer registers                   */
#define INT_SAV_CNT     8    /* number of int callee saved registers          */
#define INT_ARG_CNT     4    /* number of int argument registers              */
#define INT_TMP_CNT     8    /* number of integer temporary registers         */
#define INT_RES_CNT    11    /* number of integer reserved registers          */
                             /* + 1 REG_RET totals to 32                      */

#if !defined(ENABLE_SOFT_FLOAT)

#define FLT_REG_CNT     32   /* number of float registers                     */
#define FLT_SAV_CNT     6    /* number of flt callee saved registers          */
#define FLT_ARG_CNT     2    /* number of flt argument registers              */
#define FLT_TMP_CNT     4    /* number of float temporary registers           */
#define FLT_RES_CNT     4    /* number of float reserved registers            */

#else /* !defined(ENABLE_SOFT_FLOAT) */

#define FLT_REG_CNT     0    /* number of float registers                     */
#define FLT_SAV_CNT     0    /* number of flt callee saved registers          */
#define FLT_ARG_CNT     0    /* number of flt argument registers              */
#define FLT_TMP_CNT     0    /* number of float temporary registers           */
#define FLT_RES_CNT     0    /* number of float reserved registers            */

#endif /* !defined(ENABLE_SOFT_FLOAT) */

#define TRACE_ARGS_NUM  2

#endif /* SIZEOF_VOID_P == 8 */


/* ABI defines ****************************************************************/

#if SIZEOF_VOID_P == 8
# define PA_SIZE    0                   /* we don't have a parameter area     */
#else
# define PA_SIZE    4 * 4               /* parameter area is max. 4 * 4 bytes */
#endif


/* packed register defines ****************************************************/

#if SIZEOF_VOID_P == 4
# if WORDS_BIGENDIAN == 1
#  define REG_RESULT_PACKED    PACK_REGS(REG_RESULT2, REG_RESULT)

#  define REG_A0_A1_PACKED     PACK_REGS(REG_A1, REG_A0)
#  define REG_A2_A3_PACKED     PACK_REGS(REG_A3, REG_A2)

#  define REG_ITMP12_PACKED    PACK_REGS(REG_ITMP2, REG_ITMP1)
#  define REG_ITMP23_PACKED    PACK_REGS(REG_ITMP3, REG_ITMP2)
# else
#  define REG_RESULT_PACKED    PACK_REGS(REG_RESULT, REG_RESULT2)

#  define REG_A0_A1_PACKED     PACK_REGS(REG_A0, REG_A1)
#  define REG_A2_A3_PACKED     PACK_REGS(REG_A2, REG_A3)

#  define REG_ITMP12_PACKED    PACK_REGS(REG_ITMP1, REG_ITMP2)
#  define REG_ITMP23_PACKED    PACK_REGS(REG_ITMP2, REG_ITMP3)
# endif
#endif

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
 */
