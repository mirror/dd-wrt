/* src/vm/jit/sparc64/md-abi.hpp - defines for Sparc ABI

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

   Contact: cacao@cacaojvm.org

   Authors: Alexander Jordan

   Changes:

*/

#ifndef MD_ABI_HPP_
#define MD_ABI_HPP_ 1

/* preallocated registers *****************************************************/

/* integer registers */
  
#define REG_RESULT_CALLEE	24   /* to deliver method results				  */
#define REG_RESULT_CALLER	 8   /* to read method results                    */

#define REG_RA_CALLEE     	31   /* callee reads return address here          */
#define REG_RA_CALLER     	15   /* caller puts address of call instr here    */

#define REG_PV_CALLEE  		29   /* procedure vector, as found by callee	  */
#define REG_PV_CALLER		13   /* caller provides PV here */


#define REG_METHODPTR    2   /* pointer to the place from where the procedure */
                             /* vector has been fetched         */
                             
                             
#define REG_ITMP1        1   /* temporary register (scratch)                  */
#define REG_ITMP2        2   /* temporary register (application)              */
#define REG_ITMP3        3   /* temporary register (application)              */

#define REG_ITMP2_XPTR   2   /* exception pointer = temporary register 2      */
#define REG_ITMP3_XPC    3   /* exception pc = temporary register 3           */

#define REG_SP          14   /* stack pointer                                 */
#define REG_FP          30   /* frame pointer                                 */
#define REG_ZERO         0   /* always zero                                   */

#define REG_OUT0         8   /* define some argument registers                */
#define REG_OUT1         9
#define REG_OUT2        10
#define REG_OUT3        11
#define REG_OUT4        12
#define REG_OUT5        13   /* available only when doing a C-call            */

/* floating point registers */
/* only using the lower half of the floating registers for now */


#define REG_FRESULT      0   /* to deliver floating point method results      */

#define REG_FTMP1        1   /* temporary floating point register             */
#define REG_FTMP2        2   /* temporary floating point register             */
#define REG_FTMP3        3   /* temporary floating point register             */

#define REG_IFTMP        1   /* temporary integer and floating point register */

#define REG_F0           0


#define INT_REG_CNT     32   /* number of integer registers                   */
#define INT_SAV_CNT     13   /* number of int callee saved registers          */
#define INT_ARG_CNT      5   /* number of int argument registers (-1 for PV)  */
#define INT_TMP_CNT      0   /* int temp registers (%g4-%g5)                  */
#define INT_RES_CNT     14   /* number of reserved integer registers          */
                             /* pv, zero, %g6, %g7, sp, ra                    */

#define FLT_REG_CNT     16   /* number of float registers                     */
#define FLT_SAV_CNT      0   /* number of flt callee saved registers          */
#define FLT_ARG_CNT      5   /* number of flt argument registers              */
#define FLT_TMP_CNT      7   /* number of flt temp registers                  */
#define FLT_RES_CNT      3   /* number of reserved float registers            */
                             /* the one "missing" register is the return reg  */
                             
/* different argument counts when following the ABI for native functions */
#define INT_NATARG_CNT   6
#define FLT_NATARG_CNT  16

#define TRACE_ARGS_NUM   6

/* helpers for stack addressing and window handling */

#define BIAS          2047    /* SPARC V9: stack @ address SP + BIAS           */

/* SPARC ABI always wants argument slots on the stack, even when not used */

#define WINSAVE_CNT     16    /* number of regs that SPARC saves onto stack    */
#define ABIPARAMS_CNT    6    /* param slots the ABI always requires on stack  */

#define JITSTACK_CNT    (WINSAVE_CNT + ABIPARAMS_CNT)
#define CSTACK_CNT      (WINSAVE_CNT + ABIPARAMS_CNT)

#define JITSTACK        ((JITSTACK_CNT) * 8 + BIAS)
#define CSTACK          ((CSTACK_CNT) * 8 + BIAS)



/* applies when the caller's window was saved */
#define REG_WINDOW_TRANSPOSE(reg) \
	(reg + 16)

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
