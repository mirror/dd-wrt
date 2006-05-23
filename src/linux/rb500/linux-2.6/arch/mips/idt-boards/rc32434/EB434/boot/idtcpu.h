/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *   IDT CPU register definitions. Though the registers are already defined
 *   under asm directory, they are once again declared here for the ease of
 *   syncing up with IDT bootloader code.
 *
 *  Copyright 2004 IDT Inc. (rischelp@idt.com)
 *         
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 **************************************************************************
 * May 2004 rkt
 *
 * Initial release based on IDT/Sim (IDT bootloader)
 *
 * 
 *
 **************************************************************************
 */

#if !defined(__IDTCPU_H__)
#define __IDTCPU_H__
/*
** memory configuration and mapping
*/
#define K0BASE		0x80000000
#define K0SIZE		0x20000000
#define K1BASE		0xa0000000
#define K1SIZE		0x20000000
#define K2BASE		0xc0000000
#if defined(CPU_R32364)
#define K2SIZE		0x40000000
#define ICEBASE		0xff000000
#define ICESIZE		0x01000000
#elif defined(CPU_R32438) || defined(CPU_R32434)
#define K2SIZE		0x20000000
#define K3BASE		0xe0000000
#define K3SIZE          0x20000000
#define ICEBASE		0xff200000
#define ICESIZE		0x00200000
#endif

#define KUBASE		0
#define KUSIZE		0x80000000

/*
** Exception Vectors
*/

#define	T_VEC	(K0BASE + 0x000)			/* tlbmiss vector */
#define X_VEC	(K0BASE + 0x080)			/* xtlbmiss vector */
#define C_VEC	(K1BASE + 0x100)			/* cache error vector */
#define E_VEC	(K0BASE + 0x180)			/* exception vector */
#define I_VEC	(K0BASE + 0X200)			/* interrupt vector */
#define	R_VEC	(K1BASE + 0x1fc00000)	/* reset vector */

/*
** Address conversion macros
*/
#ifdef CLANGUAGE
#define	CAST(as) (as)
#else
#define	CAST(as)
#endif

#define	K0_TO_K1(x)		(CAST(unsigned)(x) | 0xA0000000)	/* kseg0 to kseg1 */
#define	K1_TO_K0(x)		(CAST(unsigned)(x) & 0x9FFFFFFF)	/* kseg1 to kseg0 */
#define	K0_TO_PHYS(x)	(CAST(unsigned)(x) & 0x1FFFFFFF)	/* kseg0 to physical */
#define	K1_TO_PHYS(x)	(CAST(unsigned)(x) & 0x1FFFFFFF)	/* kseg1 to physical */
#define	PHYS_TO_K0(x)	(CAST(unsigned)(x) | 0x80000000)	/* physical to kseg0 */
#define	PHYS_TO_K1(x)	(CAST(unsigned)(x) | 0xA0000000)	/* physical to kseg1 */

#if defined(CPU_R32364)             /* Includes RC32332, RC32334 */
#define	CFG_ICE					0x80000000	/* ICE detect */
#define	CFG_ECMASK			0x70000000	/* System Clock Ratio */
#define	CFG_ECBY2				0x00000000 	/* divide by 2 */
#define	CFG_ECBY3				0x10000000 	/* divide by 3 */
#define	CFG_ECBY4				0x20000000 	/* divide by 4 */
#define	CFG_BE					0x00008000	/* Big Endian */
#define	CFG_ICMASK			0x00000e00	/* Instruction cache size */
#define	CFG_ICSHIFT			9
#define	CFG_DCMASK			0x000001c0	/* Data cache size */
#define	CFG_DCSHIFT			6
#define	CFG_IB					0x00000020	/* Instruction cache line size */
#define	CFG_DB					0x00000010	/* Data cache line size */
#define	CFG_K0MASK			0x00000007	/* KSEG0 coherency algorithm */
#elif defined(CPU_R32438) || defined(CPU_R32434)
#define	CFG_MM					0x00060000  /* write buffer Merge Mode */
#define CFG_BM					0x00010000  /* Burst Mode */
#define	CFG_BE					0x00008000	/* Big Endian */
#define	CFG_K0MASK			0x00000007	/* KSEG0 coherency algorithm */
#endif

/*
 * Primary cache mode
 */
#if defined(CPU_R32364)
#define CFG_C_NCHRNT_WT_NWA			0
#define CFG_C_NCHRNT_WT					1
#define CFG_C_UNCACHED					2
#define CFG_C_NCHRNT_WB					3

/* Cache Operations */
#define Index_Invalidate_I      0x0         /* 0       0 */
#define Index_Writeback_Inv_D   0x1         /* 0       1 */
#define Index_Invalidate_SI     0x2         /* 0       2 */
#define Index_Writeback_Inv_SD  0x3         /* 0       3 */
#define Index_Load_Tag_I        0x4         /* 1       0 */
#define Index_Load_Tag_D        0x5         /* 1       1 */
#define Index_Load_Tag_SI       0x6         /* 1       2 */
#define Index_Load_Tag_SD       0x7         /* 1       3 */
#define Index_Store_Tag_I       0x8         /* 2       0 */
#define Index_Store_Tag_D       0x9         /* 2       1 */
#define Index_Store_Tag_SI      0xA         /* 2       2 */
#define Index_Store_Tag_SD      0xB         /* 2       3 */
#define Create_Dirty_Exc_D      0xD         /* 3       1 */
#define Create_Dirty_Exc_SD     0xF         /* 3       3 */
#define Hit_Invalidate_I        0x10        /* 4       0 */
#define Hit_Invalidate_D        0x11        /* 4       1 */
#define Hit_Invalidate_SI       0x12        /* 4       2 */
#define Hit_Invalidate_SD       0x13        /* 4       3 */
#define Hit_Writeback_Inv_D     0x15        /* 5       1 */
#define Hit_Writeback_Inv_SD    0x17        /* 5       3 */
#define Fill_I                  0x14        /* 5       0 */
#define Hit_Writeback_D         0x19        /* 6       1 */
#define Hit_Writeback_SD        0x1B        /* 6       3 */
#define Hit_Writeback_I         0x18        /* 6       0 */
#define Hit_Set_Virtual_SI      0x1E        /* 7       2 */
#define Hit_Set_Virtual_SD      0x1F        /* 7       3 */
#define CFG_EW32        				0x00040000      /* 32 bit */
#elif defined(CPU_R32438) || defined(CPU_R32434)
#define CFG_C_UNCACHED					2
#define CFG_C_NCHRNT_WB					3

/* Cache Operations */
#define Index_Invalidate_I      0x0         /* 0       0 */
#define Index_Invalidate_D      0x1         /* 0       0 */
#define Index_Load_Tag_I        0x4         /* 1       0 */
#define Index_Load_Tag_D        0x5         /* 1       1 */
#define Index_Store_Tag_I       0x8         /* 2       0 */
#define Index_Store_Tag_D       0x9         /* 2       1 */
#define Hit_Invalidate_I        0x10        /* 4       0 */
#define Hit_Invalidate_D        0x11        /* 4       1 */
#define Fill_I                  0x14        /* 5       0 */
#define Fetch_Lock_I						0x1C        /* 7       0 */
#define Fetch_Lock_D						0x1D        /* 7       1 */
#define CFG_EW32        				0x00040000      /* 32 bit */
#endif

/*
** TLB resource defines
*/

#define	N_TLB_ENTRIES				16
#define	TLBHI_VPN2MASK			0xffffe000
#define	TLBHI_PIDMASK				0x000000ff
#define	TLBHI_NPID					256

#define	TLBLO_PFNMASK				0x03ffffc0
#define	TLBLO_PFNSHIFT			6
#define	TLBLO_D							0x00000004	/* writeable */
#define	TLBLO_V							0x00000002	/* valid bit */
#define	TLBLO_G							0x00000001	/* global access bit */
#define	TLBLO_CMASK					0x00000038	/* cache algorithm mask */
#define	TLBLO_CSHIFT				3

#define	TLBLO_UNCACHED			(CFG_C_UNCACHED << TLBLO_CSHIFT)
#define	TLBLO_NCHRNT_WT_NWA	(CFG_C_NCHRNT_WT_NWA << TLBLO_CSHIFT)
#if defined(CPU_R32364)
#define	TLBLO_NCHRNT_WT			(CFG_C_NCHRNT_WT << TLBLO_CSHIFT)
#define	TLBLO_NCHRNT_WB			(CFG_C_NCHRNT_WB << TLBLO_CSHIFT)
#endif

#define	TLBINX_PROBE				0x80000000
#define	TLBINX_INXMASK			0x0000003f

#define	TLBRAND_RANDMASK		0x0000003f

#define	TLBCTXT_BASEMASK		0xff800000
#define	TLBCTXT_BASESHIFT		23

#define	TLBCTXT_VPN2MASK		0x007ffff0
#define	TLBCTXT_VPN2SHIFT		4

#define	TLBPGMASK_MASK			0x01ffe000

#define	SR_CUMASK				0xf0000000	/* coproc usable bits */
#define	SR_CU3					0x80000000	/* Coprocessor 3 usable */
#define	SR_CU2					0x40000000	/* Coprocessor 2 usable */
#define	SR_CU1					0x20000000	/* Coprocessor 1 usable */
#define	SR_CU0					0x10000000	/* Coprocessor 0 usable */

/* #define	SR_PE						0x00100000*/  /* cache parity error */

#if defined(CPU_R32364)
#define	SR_RE						0X02000000	/* Reverse Endianness */
#define	SR_DL						0x01000000	/* Data Cache Locking */
#define	SR_IL						0x00800000	/* Instruction Cache Locking */

#define	SR_BEV					0x00400000	/* Use boot exception vectors */
#define	SR_SR						0x00100000	/* Soft reset */
#define	SR_CH						0x00040000	/* Cache hit */
#define	SR_CE						0x00020000	/* Use cache ECC  */
#define	SR_DE						0x00010000	/* Disable cache exceptions */

#elif defined(CPU_R32438) || defined(CPU_R32434)
#define	SR_RP						0X08000000	/* Reduced Power mode */

#define	SR_RE						0X02000000	/* Reverse Endianness */

#define	SR_BEV					0x00400000	/* Use boot exception vectors */
#define	SR_TS						0X00200000	/* TLB Shutdown */
#define	SR_SR						0x00100000	/* Soft reset */
#define	SR_NMI					0X00080000	/* NMI */
#endif
/*
**	status register interrupt masks and bits
*/

#define	SR_IMASK				0x0000ff00	/* Interrupt mask */
#define	SR_IMASK8				0x00000000	/* mask level 8 */
#define	SR_IMASK7				0x00008000	/* mask level 7 */
#define	SR_IMASK6				0x0000c000	/* mask level 6 */
#define	SR_IMASK5				0x0000e000	/* mask level 5 */
#define	SR_IMASK4				0x0000f000	/* mask level 4 */
#define	SR_IMASK3				0x0000f800	/* mask level 3 */
#define	SR_IMASK2				0x0000fc00	/* mask level 2 */
#define	SR_IMASK1				0x0000fe00	/* mask level 1 */
#define	SR_IMASK0				0x0000ff00	/* mask level 0 */

#define	SR_IMASKSHIFT		8

#define	SR_IBIT8				0x00008000	/* bit level 8 */
#define	SR_IBIT7				0x00004000	/* bit level 7 */
#define	SR_IBIT6				0x00002000	/* bit level 6 */
#define	SR_IBIT5				0x00001000	/* bit level 5 */
#define	SR_IBIT4				0x00000800	/* bit level 4 */
#define	SR_IBIT3				0x00000400	/* bit level 3 */
#define	SR_IBIT2				0x00000200	/* bit level 2 */
#define	SR_IBIT1				0x00000100	/* bit level 1 */

#define	SR_KSMASK				0x00000016	/* Kernel mode mask */
#define	SR_KSUSER				0x00000000	/* User Mode */
#define	SR_KSKERNEL			0x00000016	/* Kernel Mode */

#define	SR_ERL					0x00000004	/* Error level */
#define	SR_EXL					0x00000002	/* Exception level */
#define	SR_IE						0x00000001	/* Interrupts enabled */
#define	NOT_SR_IEC      0xfffffffe  /* assembler problem with li ~SR_IEC */

/*
 * Cause Register
 */
#define	CAUSE_BD				0x80000000	/* Branch delay slot */
#define	CAUSE_CEMASK		0x30000000	/* coprocessor error */
#define	CAUSE_CESHIFT		28
#if defined(CPU_R32364)
#define	CAUSE_IPE				0x04000000	/* Imprecise exception */
#define	CAUSE_DW				0x02000000	/* Data watch */
#define	CAUSE_IW				0x01000000	/* Instruction watch */
#elif defined(CPU_R32438) || defined(CPU_R32434)
#define CAUSE_IV			 	0x00800000	/* Interrupt Vector location */
#define CAUSE_WP			 	0x00400000	/* Watch Exception deferred */
#endif

#define	CAUSE_IPMASK		0x0000FF00	/* Pending interrupt mask */
#define	CAUSE_IPSHIFT		8

/* Notice: Watch Exception if Exc. Code is 23 is not included in the mask
 *	   for R32364.
 */
#define	CAUSE_EXCMASK		0x0000003C	/* Cause code bits */
#define	CAUSE_EXCSHIFT	2

#ifndef XDS
/*
**  Coprocessor 0 registers
*/
#define	C0_INX					$0		/* tlb index */
#define	C0_RANDOM				$1
#define	C0_TLBLO0				$2		/* tlb entry low 0 */
#define	C0_TLBLO1				$3		/* tlb entry low 1 */
#define	C0_CTXT					$4		/* tlb context */
#define	C0_PAGEMASK			$5		/* tlb page mask */
#define	C0_WIRED				$6		/* number of wired tlb entries */

#define	C0_BADVADDR			$8		/* bad virtual address */
#define	C0_COUNT				$9		/* timer count */
#define	C0_TLBHI				$10		/* tlb entry hi */
#define	C0_COMPARE			$11		/* timer comparator  */
#define	C0_SR						$12		/* status register */
#define	C0_CAUSE				$13		/* exception cause */
#define	C0_EPC					$14		/* exception pc */
#define	C0_PRID					$15		/* revision identifier */
#define	C0_CONFIG				$16		/* configuration register */

#if defined(CPU_R32364)
#define	C0_IWATCH				$18		/* Instr brk pt Virtual add. */
#define	C0_DWATCH				$19		/* Data brk pt Virtual add. */

#define	C0_IEPC					$22		/* Imprecise Exception pc */
#define	C0_DEPC					$23		/* Debug Exception pc */
#define	C0_DEBUG				$24		/* Debug control/status reg */

#define	C0_ECC					$26		/* primary cache Parity control */
#define	C0_CACHEERR			$27		/* cache error status */
#define	C0_TAGLO				$28		/* cache tag lo */
#define	C0_ERRPC				$30		/* cache error pc */
#elif defined(CPU_R32438) || defined(CPU_R32434)
#define	C0_WATCHLO			$18		/* Watchpoint address (low) */
#define	C0_WATCHHI			$19		/* Watchpoint address (high) */

#define	C0_DEBUG				$23		/* Debug control/status reg */
#define	C0_DEPC					$24		/* Debug Exception pc */

#define	C0_ERRCTL				$26		/* Cache Error Control */
#define	C0_TAGLO				$28		/* Cache Tag Lo */
#define	C0_ERRPC				$30		/* Cache Error PC */
#define C0_DESAVE				$31		/* Debug scratchpad reg. */
#endif 

#endif XDS
#endif /* defined(__IDTCPU_H__) */
