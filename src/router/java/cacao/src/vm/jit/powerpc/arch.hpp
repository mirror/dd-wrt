/* src/vm/jit/powerpc/arch.hpp - architecture defines for PowerPC

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


#ifndef ARCH_HPP_
#define ARCH_HPP_ 1

#define JIT_COMPILER_VIA_SIGNAL

#include "config.h"


/* define architecture features ***********************************************/

#define SUPPORT_DIVISION                 1

#define SUPPORT_I2F                      0
#define SUPPORT_I2D                      0
#define SUPPORT_L2F                      0
#define SUPPORT_L2D                      0

#define SUPPORT_F2I                      1
#define SUPPORT_F2L                      0
#define SUPPORT_D2I                      1
#define SUPPORT_D2L                      0

#define SUPPORT_LONG_ADD                 1
#define SUPPORT_LONG_CMP                 1
#define SUPPORT_LONG_SHIFT               0
#define SUPPORT_LONG_MUL                 0
#define SUPPORT_LONG_DIV                 0

#define SUPPORT_LONG_DIV_POW2            0
#define SUPPORT_LONG_REM_POW2            0

#define SUPPORT_CONST_LOGICAL            1  /* AND, OR, XOR with immediates   */
#define SUPPORT_CONST_MUL                1  /* mutiply with immediate         */

#define SUPPORT_CONST_STORE              0  /* do we support const stores     */
#define SUPPORT_CONST_STORE_ZERO_ONLY    0  /* on some risc machines we can   */
                                            /* only store REG_ZERO            */


/* float **********************************************************************/

#define SUPPORT_FLOAT                    1

#if defined(ENABLE_SOFT_FLOAT_CMP)
# define SUPPORT_FLOAT_CMP               0
#else
# define SUPPORT_FLOAT_CMP               1
#endif


/* double *********************************************************************/

#define SUPPORT_DOUBLE                   1

#if defined(ENABLE_SOFT_FLOAT_CMP)
# define SUPPORT_DOUBLE_CMP              0
#else
# define SUPPORT_DOUBLE_CMP              1
#endif


#define SPECIALMEMUSE

/* Memory Positions for not Interface Stackslots (allocate_scratch_registers)*/
/* are not properly aligned in case HAS_4_BYTE_STACKSLOT is not defined!     */
/* For HAS_4_BYTE_STACKSLOT archs no distinction is made between long and dbl*/
#define SUPPORT_COMBINE_INTEGER_REGISTERS


/* branches *******************************************************************/

#define SUPPORT_BRANCH_CONDITIONAL_CONDITION_REGISTER       1
#define SUPPORT_BRANCH_CONDITIONAL_ONE_INTEGER_REGISTER     0
#define SUPPORT_BRANCH_CONDITIONAL_TWO_INTEGER_REGISTERS    0


/* exceptions *****************************************************************/

#define SUPPORT_HARDWARE_DIVIDE_BY_ZERO  0


/* stackframe *****************************************************************/

#define STACKFRMAE_RA_BETWEEN_FRAMES              0
#define STACKFRAME_RA_TOP_OF_FRAME                0
#define STACKFRAME_RA_LINKAGE_AREA                1
#define STACKFRAME_LEAFMETHODS_RA_REGISTER        1
#define STACKFRAME_SYNC_NEEDS_TWO_SLOTS           1


/* replacement ****************************************************************/

#define REPLACEMENT_PATCH_SIZE           4 /* bytes */

/* subtype ********************************************************************/

#define USES_NEW_SUBTYPE                 1

#endif // ARCH_HPP_


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
