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

#ifdef CLANGUAGE
struct ireg_desc {
	char 	*ptr_field_name;	/* field name   */
	short	num_digits;				/* number ofdigits to display */
	short	num_spaces;				/* number of spaces to follow */
	reg_t	fld_mask;					/* mask to extract value of field */
	int	fld_shift;					/* shift amount to position field */
  short    cpu;
	char *CONST *ptr_enum_list;	/* ptr to an enumeration list */
	  };

/*
** reg_name - structure that gives the reg. name, alt. reg name
**		the reg index for fetching the value, the number
**		of spaces req. so a tabular display will align
**		a pointer to a structure defining the fields if
**		required and a flag for the output type.
*/
struct reg_name {
	char	*register_name;
	char	*alt_reg_name;
	short	reg_index;
	short	space_pad;
	CONST struct ireg_desc *ptr_reg_desc_flds;
	unsigned char format_type;
	unsigned char print_type;
	short   reg_group;
  short    cpu;
	  };

/* print format specifiers */
#define PRT_HEX		0
#define PRT_SGL 	1
#define PRT_DBL 	2

/* register group classifiers */
#define GRP_CPU		0x0001
#define GRP_FPR		0x0002
#define GRP_FPS		0x0004
#define GRP_FPD		0x0008
#define GRP_CP0		0x0010
#define GRP_CP0R	0x0020
#endif CLANGUAGE

/*
** register names
*/
#define r0		$0
#define r1		$1
#define r2		$2
#define r3		$3
#define r4		$4
#define r5		$5
#define r6		$6
#define r7		$7
#define r8		$8
#define r9		$9
#define r10		$10
#define r11		$11
#define r12		$12
#define r13		$13
#define r14		$14
#define r15		$15
#define r16		$16
#define r17		$17
#define r18		$18
#define r19		$19
#define r20		$20
#define r21		$21
#define r22		$22
#define r23		$23
#define r24		$24
#define r25		$25
#define r26		$26
#define r27		$27
#define r28		$28
#define r29		$29
#define r30		$30
#define r31		$31

#define zero	$0		/* wired zero */
#define AT		$at		/* assembler temp */
#define v0		$2		/* return value */
#define v1		$3
#define a0		$4		/* argument registers a0-a3 */
#define a1		$5
#define a2		$6
#define a3		$7
#define t0		$8		/* caller saved  t0-t9 */
#define t1		$9
#define t2		$10
#define t3		$11
#define t4		$12
#define t5		$13
#define t6		$14
#define t7		$15
#define s0		$16		/* callee saved s0-s8 */
#define s1		$17
#define s2		$18
#define s3		$19
#define s4		$20
#define s5		$21
#define s6		$22
#define s7		$23
#define t8		$24
#define t9		$25
#define k0		$26		/* kernel usage */
#define k1		$27		/* kernel usage */
#define gp		$28		/* sdata pointer */
#define sp		$29		/* stack pointer */
#define s8		$30		/* yet another saved reg for the callee */
#define fp		$30		/* frame pointer - this is being phased out by MIPS */
#define ra		$31		/* return address */

/*
** relative position of registers in save reg area
*/
#define	R_R0		0
#define	R_R1		1
#define	R_R2		2
#define	R_R3		3
#define	R_R4		4
#define	R_R5		5
#define	R_R6		6
#define	R_R7		7
#define	R_R8		8
#define	R_R9		9
#define	R_R10		10
#define	R_R11		11
#define	R_R12		12
#define	R_R13		13
#define	R_R14		14
#define	R_R15		15
#define	R_R16		16
#define	R_R17		17
#define	R_R18		18
#define	R_R19		19
#define	R_R20		20
#define	R_R21		21
#define	R_R22		22
#define	R_R23		23
#define	R_R24		24
#define	R_R25		25
#define	R_R26		26
#define	R_R27		27
#define	R_R28		28
#define	R_R29		29
#define	R_R30		30
#define	R_R31		31
#define NCLIENTREGS	32
#define	R_EPC				32
#define	R_MDHI			33
#define	R_MDLO		  34
#define	R_SR				35
#define	R_CAUSE			36
#define	R_TLBHI			37
#define	R_TLBLO0		38
#define	R_BADVADDR	39
#define	R_INX				40
#define	R_RAND			41
#define	R_CTXT			42
#define	R_EXCTYPE		43
#define R_MODE			44
#define R_PRID			45
#define R_TLBLO1		46
#define R_PAGEMASK	47
#define R_WIRED			48
#define R_COUNT			49
#define R_COMPARE		50
#define R_CONFIG		51
#if defined(CPU_R32434)
#define R_WATCHLO   52
#define R_WATCHHI   53
#elif defined(CPU_R32364)
#define R_IWATCH    52
#define R_DWATCH    53
#define R_ECC				54
#define R_CACHEERR	55
#endif
#define R_TAGLO			56
#define R_TAGHI			57
#define R_ERRPC			58

#define NREGS			  59

#if __mips >= 3

#define R_SZ		8
#ifndef CLANGUAGE
#define sreg		sd
#define lreg		ld
#define rmfc0		mfc0
#define rmtc0		mtc0
#endif

#else

#define R_SZ		4
#ifndef CLANGUAGE
#define sreg		sw
#define lreg		lw
#define rmfc0		mfc0
#define rmtc0		mtc0
#endif

#endif

/*
** For those who like to think in terms of the compiler names for the regs
*/
#define	R_ZERO	R_R0
#define	R_AT		R_R1
#define	R_V0		R_R2
#define	R_V1		R_R3
#define	R_A0		R_R4
#define	R_A1		R_R5
#define	R_A2		R_R6
#define	R_A3		R_R7
#define	R_T0		R_R8
#define	R_T1		R_R9
#define	R_T2		R_R10
#define	R_T3		R_R11
#define	R_T4		R_R12
#define	R_T5		R_R13
#define	R_T6		R_R14
#define	R_T7		R_R15
#define	R_S0		R_R16
#define	R_S1		R_R17
#define	R_S2		R_R18
#define	R_S3		R_R19
#define	R_S4		R_R20
#define	R_S5		R_R21
#define	R_S6		R_R22
#define	R_S7		R_R23
#define	R_T8		R_R24
#define	R_T9		R_R25
#define	R_K0		R_R26
#define	R_K1		R_R27
#define	R_GP		R_R28
#define	R_SP		R_R29
#define	R_FP		R_R30
#define	R_RA		R_R31
