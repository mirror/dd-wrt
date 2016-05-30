/* src/vm/jit/x86_64/md-abi.hpp - defines for x86_64 Linux ABI

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

/* define registers ***********************************************************/

#define RIP    -1
#define RAX    0
#define RCX    1
#define RDX    2
#define RBX    3
#define RSP    4
#define RBP    5
#define RSI    6
#define RDI    7
#define R8     8
#define R9     9
#define R10    10
#define R11    11
#define R12    12
#define R13    13
#define R14    14
#define R15    15


#define XMM0   0
#define XMM1   1
#define XMM2   2
#define XMM3   3
#define XMM4   4
#define XMM5   5
#define XMM6   6
#define XMM7   7
#define XMM8   8
#define XMM9   9
#define XMM10  10
#define XMM11  11
#define XMM12  12
#define XMM13  13
#define XMM14  14
#define XMM15  15


/* preallocated registers *****************************************************/

/* integer registers */
  
#define REG_RESULT      RAX      /* to deliver method results                 */

#define REG_A0          RDI      /* define some argument registers            */
#define REG_A1          RSI
#define REG_A2          RDX
#define REG_A3          RCX

#define REG_ITMP1       RAX      /* temporary register                        */
#define REG_ITMP2       R10      /* temporary register and method pointer     */
#define REG_ITMP3       R11      /* temporary register                        */

#define REG_METHODPTR   REG_ITMP2/* pointer to the place from where the       */
                                 /* procedure vector has been fetched         */

#define REG_NULL        -1       /* used for reg_of_var where d is not needed */

#define REG_ITMP1_XPTR  REG_ITMP1/* exception pointer = temporary register 1  */
#define REG_ITMP2_XPC   REG_ITMP2/* exception pc = temporary register 2       */

#if defined(__SOLARIS__)
#undef REG_SP
#endif

#define REG_SP          RSP      /* stack pointer                             */


/* floating point registers */

#define REG_FRESULT     XMM0     /* to deliver floating point method results  */

#define REG_FA0         XMM0     /* define some argument registers            */
#define REG_FA1         XMM1

#define REG_FTMP1       XMM8     /* temporary floating point register         */
#define REG_FTMP2       XMM9     /* temporary floating point register         */
#define REG_FTMP3       XMM10    /* temporary floating point register         */

#define REG_IFTMP       10   /* temporary integer and floating point register */


#define INT_REG_CNT     16       /* number of integer registers               */
#define INT_SAV_CNT     5        /* number of integer callee saved registers  */
#define INT_ARG_CNT     6        /* number of integer argument registers      */
#define INT_TMP_CNT     1        /* number of integer temporary registers     */
#define INT_RES_CNT     3        /* number of integer reserved registers      */

#define FLT_REG_CNT     16       /* number of float registers                 */
#define FLT_SAV_CNT     0        /* number of float callee saved registers    */
#define FLT_ARG_CNT     8        /* number of float argument registers        */
#define FLT_TMP_CNT     5        /* number of float temporary registers       */
#define FLT_RES_CNT     3        /* number of float reserved registers        */

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
