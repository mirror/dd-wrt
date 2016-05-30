/* src/vm/jit/s390/md-abi.h - defines for s390 Linux ABI

   Copyright (C) 1996-2010
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


#ifndef _MD_ABI_H
#define _MD_ABI_H

/* define registers ***********************************************************/

#define R0     0
#define R1     1
#define R2     2
#define R3     3
#define R4     4
#define R5     5
#define R6     6 
#define R7     7
#define R8     8
#define R9     9
#define R10    10
#define R11    11
#define R12    12
#define R13    13
#define R14    14
#define R15    15

#define F0     0
#define F1     1
#define F2     2
#define F3     3
#define F4     4
#define F5     5
#define F6     6
#define F7     7
#define F8     8
#define F9     9
#define F10    10
#define F11    11
#define F12    12
#define F13    13
#define F14    14
#define F15    15

/* preallocated registers *****************************************************/

/* integer registers */
  
#define REG_RESULT      R2       /* to deliver method results                 */
#define REG_RESULT2     R3

#define REG_ITMP1       R1      /* temporary register                        */
#define REG_ITMP2       R14     /* temporary register and method pointer     */
#define REG_ITMP3       R0      /* temporary register                        */

#define IS_REG_ITMP(x) (((x) == REG_ITMP1) || ((x) == REG_ITMP2) || ((x) == REG_ITMP3))

#define REG_ITMP12_PACKED    PACK_REGS(REG_ITMP2, REG_ITMP1)
#define REG_ITMP23_PACKED    PACK_REGS(REG_ITMP3, REG_ITMP2)
#define REG_ITMP13_PACKED    PACK_REGS(REG_ITMP3, REG_ITMP1)
/* even odd */
#define REG_ITMP31_PACKED    PACK_REGS(REG_ITMP1, REG_ITMP3)
#define REG_RESULT_PACKED    PACK_REGS(REG_RESULT2, REG_RESULT)

#define REG_METHODPTR   REG_ITMP1/* pointer to the place from where the       */
                                 /* procedure vector has been fetched         */

#define REG_NULL        -1       /* used for reg_of_var where d is not needed */

#define REG_ITMP1_XPTR  REG_ITMP3/* exception pointer = temporary register 3  */
#define REG_ITMP2_XPC   REG_ITMP1/* exception pc = temporary register 1       */

#define REG_SP          R15      /* stack pointer                             */
#define REG_RA          R14      /* same as itmp3 */

#define REG_PV  R13

#define REG_A0          R2      /* define some argument registers            */
#define REG_A1          R3
#define REG_A2          R4 
#define REG_A3          R5
#define REG_A4          R6

#define REG_FA0         F0
#define REG_FA1         F2

/* floating point registers */

#define REG_FRESULT     F0       /* to deliver floating point method results  */

#define REG_FTMP1       F4       /* temporary floating point register         */
#define REG_FTMP2       F6       /* temporary floating point register         */

/* No ftmp3 */

#define INT_REG_CNT     16       /* number of integer registers               */
#define INT_SAV_CNT     6        /* number of integer callee saved registers  */
#define INT_ARG_CNT     5        /* number of integer argument registers      */
#define INT_TMP_CNT     0        /* number of integer temporary registers     */
#define INT_RES_CNT     5        /* number of integer reserved registers      */

#define FLT_REG_CNT     16       /* number of float registers                 */
#define FLT_SAV_CNT     0        /* number of float callee saved registers    */
#define FLT_ARG_CNT     2        /* number of float argument registers        */
#define FLT_TMP_CNT     12       /* number of float temporary registers       */
#define FLT_RES_CNT     2        /* number of float reserved registers        */

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
