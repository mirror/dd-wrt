/* src/vm/jit/powerpc/darwin/md-abi.hpp - defines for PowerPC Darwin ABI

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

/* preallocated registers *****************************************************/

/* integer registers */
  
#define REG_RESULT       3   /* to deliver method results                     */
#define REG_RESULT2      4   /* to deliver long method results                */

#define REG_PV          13   /* procedure vector, must be provided by caller  */
#define REG_METHODPTR   12   /* pointer to the place from where the procedure */
                             /* vector has been fetched                       */
#define REG_ITMP1       11   /* temporary register                            */
#define REG_ITMP2       12   /* temporary register and method pointer         */
#define REG_ITMP3       16   /* temporary register                            */

#define REG_ITMP1_XPTR  11   /* exception pointer = temporary register 1      */
#define REG_ITMP2_XPC   12   /* exception pc = temporary register 2           */

#define REG_SP           1   /* stack pointer                                 */
#define REG_ZERO         0   /* almost always zero: only in address calc.     */

#define REG_A0           3   /* define some argument registers                */
#define REG_A1           4
#define REG_A2           5
#define REG_A3           6

/* floating point registers */

#define REG_FRESULT      1   /* to deliver floating point method results      */
#define REG_FTMP1       16   /* temporary floating point register             */
#define REG_FTMP2       17   /* temporary floating point register             */
#define REG_FTMP3        0   /* temporary floating point register             */

#define REG_IFTMP        0   /* temporary integer and floating point register */

#define REG_FA0          1   /* define some argument registers                */
#define REG_FA1          2


#define INT_REG_CNT     32   /* number of integer registers                   */
#define INT_SAV_CNT     10   /* number of int callee saved registers          */
#define INT_ARG_CNT      8   /* number of int argument registers              */
#define INT_TMP_CNT      8   /* number of integer temporary registers         */
#define INT_RES_CNT      6   /* number of integer reserved registers          */

#define FLT_REG_CNT     32   /* number of float registers                     */
#define FLT_SAV_CNT     10   /* number of float callee saved registers        */
#define FLT_ARG_CNT     13   /* number of float argument registers            */
#define FLT_TMP_CNT      6   /* number of float temporary registers           */
#define FLT_RES_CNT      3   /* number of float reserved registers            */


/* packed register defines ****************************************************/

#define REG_RESULT_PACKED    PACK_REGS(REG_RESULT2, REG_RESULT)

#define REG_A0_A1_PACKED     PACK_REGS(REG_A1, REG_A0)
#define REG_A2_A3_PACKED     PACK_REGS(REG_A3, REG_A2)

#define REG_ITMP12_PACKED    PACK_REGS(REG_ITMP2, REG_ITMP1)
#define REG_ITMP23_PACKED    PACK_REGS(REG_ITMP3, REG_ITMP2)


/* ABI defines ****************************************************************/

#define LA_SIZE             24  /* linkage area size                          */
#define LA_SIZE_ALIGNED     32  /* linkage area size aligned to 16-byte       */
#define LA_SIZE_IN_POINTERS LA_SIZE / SIZEOF_VOID_P

#define LA_LR_OFFSET         8  /* link register offset in linkage area       */

/* #define ALIGN_FRAME_SIZE(sp)       (sp) */

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
