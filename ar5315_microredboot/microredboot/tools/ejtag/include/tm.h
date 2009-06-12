#ifndef __TM_H__
#define __TM_H__

#ifndef NUM_REGS
#define NUM_REGS 106
#endif

#ifndef MIPS_REGISTER_NAMES
#define MIPS_REGISTER_NAMES 	\
    {	"zero",	"at",	"v0",	"v1",	"a0",	"a1",	"a2",	"a3", \
	"t0",	"t1",	"t2",	"t3",	"t4",	"t5",	"t6",	"t7", \
	"s0",	"s1",	"s2",	"s3",	"s4",	"s5",	"s6",	"s7", \
	"t8",	"t9",	"k0",	"k1",	"gp",	"sp",	"s8",	"ra", \
	"sr",	"lo",	"hi",	"bad",	"cause","pc",    \
	"f0",   "f1",   "f2",   "f3",   "f4",   "f5",   "f6",   "f7", \
	"f8",   "f9",   "f10",  "f11",  "f12",  "f13",  "f14",  "f15", \
	"f16",  "f17",  "f18",  "f19",  "f20",  "f21",  "f22",  "f23",\
	"f24",  "f25",  "f26",  "f27",  "f28",  "f29",  "f30",  "f31",\
	"fsr",  "fir",  "fp",	"", \
	"idx",	"rdm",	"elo0",	"el01",	"ctxt",	"pmsk",	"wired","", \
	"bad",	"cnt1",	"ehi",	"cmp1",	"sr",	"cause","epc",	"prid", \
	"cnfg","",	"cnt2",	"cmp2",	"cnt3",	"cmp3","", "debug", \
	"depc",	"",	"",	"",	"",	"","",	"dsave", \
    }
#endif

/* Register numbers of various important registers.
   Note that some of these values are "real" register numbers,
   and correspond to the general registers of the machine,
   and some are "phony" register numbers which are too large
   to be actual register numbers as far as the user is concerned
   but do serve to get the desired values when passed to read_register.  */

#define ZERO_REGNUM 0		/* read-only register, always 0 */
#define V0_REGNUM 2		/* Function integer return value */
#define A0_REGNUM 4		/* Loc of first arg during a subr call */
#if MIPS_EABI
#define MIPS_LAST_ARG_REGNUM 11	/* EABI uses R4 through R11 for args */
#define MIPS_NUM_ARG_REGS 8
#else
#define MIPS_LAST_ARG_REGNUM 7	/* old ABI uses R4 through R7 for args */
#define MIPS_NUM_ARG_REGS 4
#endif
#define T9_REGNUM 25		/* Contains address of callee in PIC */
#define SP_REGNUM 29		/* Contains address of top of stack */
#define RA_REGNUM 31		/* Contains return address value */
#define PS_REGNUM 32		/* Contains processor status */
#define HI_REGNUM 34		/* Multiple/divide temp */
#define LO_REGNUM 33		/* ... */
#define BADVADDR_REGNUM 35	/* bad vaddr for addressing exception */
#define CAUSE_REGNUM 36		/* describes last exception */
#define PC_REGNUM 37		/* Contains program counter */
#define FP0_REGNUM 38		/* Floating point register 0 (single float) */
#define FPA0_REGNUM (FP0_REGNUM+12)	/* First float argument register */
#if MIPS_EABI			/* EABI uses F12 through F19 for args */
#define MIPS_LAST_FP_ARG_REGNUM (FP0_REGNUM+19)
#define MIPS_NUM_FP_ARG_REGS 8
#else /* old ABI uses F12 through F15 for args */
#define MIPS_LAST_FP_ARG_REGNUM (FP0_REGNUM+15)
#define MIPS_NUM_FP_ARG_REGS 4
#endif
#define FCRCS_REGNUM 70		/* FP control/status */
#define FCRIR_REGNUM 71		/* FP implementation/revision */
#define FP_REGNUM 72		/* Pseudo register that contains true address of executing stack frame */
#define	UNUSED_REGNUM 73	/* Never used, FIXME */
#define	FIRST_EMBED_REGNUM 74	/* First CP0 register for embedded use */
#define	PRID_REGNUM 89		/* Processor ID */
#define	LAST_EMBED_REGNUM 105	/* Last one */

#define TARGET_WAITKIND_STOPPED 0
#endif // __TM_H__





