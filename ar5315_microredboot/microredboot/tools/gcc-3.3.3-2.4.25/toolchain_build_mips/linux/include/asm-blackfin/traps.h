/*
 *  linux/include/asm/traps.h
 *
 *  Copyright (C) 1993        Hamish Macdonald
 *
 *  Lineo, Inc    Jul 2001    Tony Kou
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */

#ifndef _FRIO_TRAPS_H
#define _FRIO_TRAPS_H

#ifndef __ASSEMBLY__
typedef void (*e_vector)(void);
extern e_vector vectors[];
#endif

#define VEC_SYS		(0)
#define VEC_EXCPT01	(1)
#define VEC_EXCPT02	(2)
#define VEC_EXCPT03	(3)
#define VEC_EXCPT04	(4)
#define VEC_EXCPT05	(5)
#define VEC_EXCPT06	(6)
#define VEC_EXCPT07	(7)
#define VEC_EXCPT08	(8)
#define VEC_EXCPT09	(9)
#define VEC_EXCPT10	(10)
#define VEC_EXCPT11	(11)
#define VEC_EXCPT12	(12)
#define VEC_EXCPT13	(13)
#define VEC_EXCPT14	(14)
#define VEC_EXCPT15	(15)
#define VEC_STEP	(16)
#define VEC_OVFLOW	(17)
#define VEC_UNDEF_I	(33)
#define VEC_ILGAL_I	(34)
#define VEC_CPLB_VL	(35)
#define VEC_MISALI_D	(36)
#define VEC_UNCOV	(37)
#define VEC_CPLB_M	(38)
#define VEC_CPLB_MHIT	(39)
#define VEC_WATCH	(40)
#define VEC_ISTRU_VL	(41)
#define VEC_MISALI_I	(42)
#define VEC_CPLB_I_VL	(43)
#define VEC_CPLB_I_M	(44)
#define VEC_CPLB_I_MHIT	(45)
#define VEC_ILL_RES	(46)	/* including unvalid supervisor mode insn */

#define VECOFF(vec) ((vec)<<2)

#ifndef __ASSEMBLY__

/* Status register bits */
#define PS_T  (0x8000)
#define PS_S  (0x0c00)	/*  Supervisor mode = 0b01 	*/
#define PS_D  (0x0c00)	/*  Debug mode = 0b1x		*/
#define PS_M  (0x1000)
#define PS_C  (0x0001)

#endif /* __ASSEMBLY__ */
#endif /* _FRIO_TRAPS_H */

