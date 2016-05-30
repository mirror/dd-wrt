/* src/vm/jit/i386/md-abi.hpp - defines for i386 Linux ABI

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
  
#define REG_RESULT      EAX     /* to deliver method results                  */
#define REG_RESULT2     EDX     /* to deliver long method results             */

#define REG_ITMP1       EAX     /* temporary register                         */
#define REG_ITMP2       ECX     /* temporary register                         */
#define REG_ITMP3       EDX     /* temporary register                         */

#define REG_METHODPTR   REG_ITMP2/* pointer to the place from where the       */
                                 /* procedure vector has been fetched         */

#define REG_NULL        -1      /* used for reg_of_var where d is not needed  */

#define REG_ITMP1_XPTR  EAX     /* exception pointer = temporary register 1   */
#define REG_ITMP2_XPC   ECX     /* exception pc = temporary register 2        */

#if defined(__SOLARIS__)
#undef REG_SP
#endif

#define REG_SP          ESP     /* stack pointer                              */

/* floating point registers */

#define REG_FRESULT     0       /* to deliver floating point method results   */
#define REG_FTMP1       6       /* temporary floating point register          */
#define REG_FTMP2       7       /* temporary floating point register          */
#define REG_FTMP3       7       /* temporary floating point register          */


#define INT_REG_CNT     8       /* number of integer registers                */
#define INT_SAV_CNT     3       /* number of integer callee saved registers   */
#define INT_ARG_CNT     0       /* number of integer argument registers       */
#define INT_TMP_CNT     1       /* number of integer temporary registers      */
#define INT_RES_CNT     3       /* numebr of integer reserved registers       */

#define FLT_REG_CNT     8       /* number of float registers                  */
#define FLT_SAV_CNT     0       /* number of float callee saved registers     */
#define FLT_ARG_CNT     0       /* number of float argument registers         */
#define FLT_TMP_CNT     0       /* number of float temporary registers        */
#define FLT_RES_CNT     8       /* numebr of float reserved registers         */

#define REG_RES_CNT     3       /* number of reserved registers               */


/* packed register defines ****************************************************/

#define REG_ITMP12_PACKED               PACK_REGS(REG_ITMP1, REG_ITMP2)
#define REG_ITMP13_PACKED               PACK_REGS(REG_ITMP1, REG_ITMP3)
#define REG_ITMP23_PACKED               PACK_REGS(REG_ITMP2, REG_ITMP3)

#define REG_RESULT_PACKED               PACK_REGS(REG_RESULT, REG_RESULT2)

#define EAX_EDX_PACKED                  PACK_REGS(EAX, EDX)

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
