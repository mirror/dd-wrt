/* src/vm/jit/parisc/md-abi.h - defines for PA-RISC ABI

   Copyright (C) 2006 R. Grafl, A. Krall, C. Kruegel,
   C. Oates, R. Obermaisser, M. Platter, M. Probst, S. Ring,
   E. Steiner, C. Thalinger, D. Thuernbeck, P. Tomsich, C. Ullrich,
   J. Wenninger, Institut f. Computersprachen - TU Wien

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

   Authors: Christian Thalinger

   Changes:

*/


#ifndef _MD_ABI_H
#define _MD_ABI_H

/* preallocated registers *****************************************************/

/* integer registers */
  
#define REG_RESULT      0    /* to deliver method results                     */

#define REG_RA          26   /* return address                                */
#define REG_PV          27   /* procedure vector, must be provided by caller  */
#define REG_METHODPTR   28   /* pointer to the place from where the procedure */
                             /* vector has been fetched                       */
#define REG_ITMP1       25   /* temporary register                            */
#define REG_ITMP2       28   /* temporary register and method pointer         */
#define REG_ITMP3       29   /* temporary register                            */

#define REG_ITMP1_XPTR  25   /* exception pointer = temporary register 1      */
#define REG_ITMP2_XPC   28   /* exception pc = temporary register 2           */

#define REG_SP          30   /* stack pointer                                 */
#define REG_ZERO        31   /* always zero                                   */

#define REG_A0          16   /* define some argument registers                */
#define REG_A1          17
#define REG_A2          18
#define REG_A3          19

/* floating point registers */

#define REG_FRESULT     0    /* to deliver floating point method results      */

#define REG_FTMP1       28   /* temporary floating point register             */
#define REG_FTMP2       29   /* temporary floating point register             */
#define REG_FTMP3       30   /* temporary floating point register             */

#define REG_IFTMP       28   /* temporary integer and floating point register */


#define INT_REG_CNT     32   /* number of integer registers                   */
#define INT_SAV_CNT      7   /* number of int callee saved registers          */
#define INT_ARG_CNT      6   /* number of int argument registers              */
#define INT_TMP_CNT     11   /* number of int temp registers                  */
#define INT_RES_CNT      7   /* number of reserved integer registers          */
                             /* the one "missing" register is the return reg  */

#define FLT_REG_CNT     32   /* number of float registers                     */
#define FLT_SAV_CNT      8   /* number of flt callee saved registers          */
#define FLT_ARG_CNT      6   /* number of flt argument registers              */
#define FLT_TMP_CNT     13   /* number of flt temp registers                  */
#define FLT_RES_CNT      4   /* number of reserved float registers            */
                             /* the one "missing" register is the return reg  */

#define TRACE_ARGS_NUM   6

#endif /* _MD_ABI_H */


/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 */
