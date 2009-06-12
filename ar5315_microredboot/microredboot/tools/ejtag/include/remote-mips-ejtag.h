#ifndef __REMOTE_MIPS_EJTAG_H__
#define __REMOTE_MIPS_EJTAG_H__

#define EJTAG_BASE 0xFF200000
#define DSU_BASE 0xFF300000

#define DSU_DEBUG_CTRL 0x0
#define DSU_INSTR_BRK_STAT 0x4
#define DSU_DATA_BRK_STAT 0xC
#define DSU_PROC_BRK_STAT 0xC

/* following macro's are valid from 0 - E(max) */
#define DSU_INSTR_ADDR_BRK(X) (0x100|((X)<<4))
#define DSU_INSTR_BRK_CTRL(X) (0x104|((X)<<4))
#define DSU_INSTR_ADDR_BRK_MASK(X) (0x108|((X)<<4))

#define DSU_DATA_ADDR_BRK(X) (0x200|((X)<<4))
#define DSU_DATA_BRK_CTRL(X) (0x204|((X)<<4))
#define DSU_DATA_ADDR_BRK_MASK(X) (0x208|((X)<<4))
#define DSU_DATA_VAL_BRK(X) (0x20C|((X)<<4))

#define DSU_PROC_ADDR_BRK(X) (0x300|((X)<<4))
#define DSU_PROC_DATA_BRK(X) (0x304|((X)<<4))
#define DSU_PROC_DATA_MASK(X) (0x308|((X)<<4))
#define DSU_PROC_BRK_CTRL(X) (0x30C|((X)<<4))

/* DSU_DEBUG_CTRL fields */
#define DSU_DCR_TM (0x1<<0)       * rw  trace mode */
#define DSU_DCR_MSRST (0x1<<1)    /* rw  mask soft reset */
#define DSU_DCR_MP (0x1<<2)       /* rw  memory protection */
#define DSU_DCR_MNMI (0x1<<3)     /* rw  mask nonmaskable interrupt (in nondebug mode) */
#define DSU_DCR_MINT (0x1<<4)     /* rw  mask interrupt (in nondebug mode) */
#define DSU_DCR_TOF (0x1<<5)      /* rw1 test output full */
#define DSU_DCR_TIF (0x1<<6)      /* rw0 test input full */
#define DSU_DCR_ENM (0x1<<29)     /* r   endianess */
#define DSU_DCR_HIS (0x1<<30)     /* r   halt status */
#define DSU_DCR_DZS (0x1<<31)     /* r   doze status */

/* DSU_INSTR_BRK_STAT fields, 15 possible breakpoints */
#define DSU_IBS_BS(X) (0x1<<(X))  /* rw  instr addr breaks/triggers for chan X, X=0..14 */
#define DSU_IBS_BCN (0xF<<24)     /* r   number of break channels */

/* DSU_INSTR_BRK_CTRL(X) fields */
#define DSU_IBC_BE (0x1<<0)       /* rw  break enable */
#define DSU_IBC_TE (0x1<<2)       /* rw  trigger enable */

/* DSU_DATA_BRK_STAT fields, 15 possible breakpoints */
#define DSU_DBS_BS(X) (0x1<<(X))  /* rw  data addr breaks/triggers for chan X, X=0..14 */
#define DSU_DBS_BCN (0xF<<24)     /* r   number of break channels */

/* DSU_DATA_ADDR_BRK_CTRL(X) fields */
#define DSU_DBC_BE (0x1<<0)       /* rw  break enable */
#define DSU_DBC_TE (0x1<<2)       /* rw  trigger enable */
#define DSU_DBC_BLM (0xF<<4)      /* rw  byte lane mask */
#define DSU_DBC_NOLB (0x1<<12)    /* rw  no load break */
#define DSU_DBC_NOSB (0x1<<13)    /* rw  no store break */
#define DSU_DBC_BAI (0xf<<14)     /* rw  byte access ignored */

/* DSU_PROC_BRK_STAT fields, 15 possible breakpoints */
#define DSU_PBS_BS(X) (0x1<<(X))  /* rw  instr addr breaks/triggers for chan X, X=0..14 */
#define DSU_PBS_BCN (0xF<<24)     /* r   number of break channels */

/* DSU_DATA_ADDR_BRK_CTRL(X) fields */
#define DSU_PBC_BE (0x1<<0)       /* rw  break enable */
#define DSU_PBC_TE (0x1<<2)       /* rw  trigger enable */
#define DSU_PBC_IFUC (0x1<<4)     /* rw  instruction fetch from noncacheable area */
#define DSU_PBC_DFUC (0x1<<5)     /* rw  data fetch from noncacheable area */
#define DSU_PBC_DSUC (0x1<<6)     /* rw  data store to noncacheable area */
#define DSU_PBC_DSCA (0x1<<7)     /* rw  data store to cacheable area */
#define DSU_PBC_LAM (0xFFFFFF00<<8)/* rw  lower address mask */

#define ZERO	0	/* WIRED ZERO */
#define AT	1	/* ASSEMBLER TEMP */
#define V0	2	/* RETURN VALUE */
#define V1	3
#define A0	4	/* ARGUMENT REGISTERS */
#define A1	5
#define A2	6
#define A3	7
#define T0	8	/* CALLER SAVED */
#define T1	9
#define T2	10
#define T3	11
#define T4	12
#define T5	13
#define T6	14
#define T7	15
#define S0	16	/* CALLEE SAVED */
#define S1	17
#define S2	18
#define S3	19
#define S4	20
#define S5	21
#define S6	22
#define S7	23
#define T8	24	/* CODE GENERATOR */
#define T9	25
#define K0	26	/* KERNEL TEMPORARY */
#define K1	27
#define GP	28	/* GLOBAL POINTER */
#define SP	29	/* STACK POINTER */
#define FP	30	/* FRAME POINTER */
#define RA	31	/* return address */

#define SR      32
#define LO      33
#define HI      34
#define BAD     35
#define CAUSE   36
#define PC      37

/* coprocessor 0 definitions */
#define CP0_INDEX      0       
#define CP0_RANDOM     1       
#define CP0_ENTRYLO0   2       
#define CP0_ENTRYLO1   3       
#define CP0_CONTEXT    4       
#define CP0_PAGEMASK   5       
#define CP0_WIRED      6       
#define CP0_RESERVED7  7       
#define CP0_BADVADDR   8
#define CP0_COUNT1     9       
#define CP0_ENTRYHI    10      
#define CP0_COMPARE1   11      
#define CP0_STATUS     12      
#define CP0_CAUSE      13      
#define CP0_EPC        14      
#define CP0_PRID       15      
#define CP0_DEBUG      23      
#define CP0_DEPC       24      
#define CP0_COUNT2     18      
#define CP0_COMPARE2   19      
#define CP0_COUNT3     20      
#define CP0_COMPARE3   21      
#define CP0_CONFIG     16      
#define CP0_Reserved25 25      
#define CP0_Reserved26 26      
#define CP0_Reserved27 27      
#define CP0_Reserved28 28      
#define CP0_Reserved29 29      
#define CP0_Reserved30 30      
#define CP0_DESAVE     31      


/*------------------------------------------------------------------------*/
/*  CP0 STATUS BITS       						  */
/*------------------------------------------------------------------------*/
#define CP0_PRID_MINREV(X) ((X)&0xf)
#define CP0_PRID_MAJREV(X) ((X)>>4&0xf)
#define CP0_PRID_MANUF(X) ((X)>>8&0xff)
#define CP0_PRID_CPUID(X) ((X)>>16&0xfffff)

#define PHILIPS_SEMICONDUCTORS 0x2b
#define PHILIPS_PR3930         0x0010

/*------------------------------------------------------------------------*/
/*  CP0 STATUS BITS       						  */
/*------------------------------------------------------------------------*/
/*
 * Written to the spec defined in section 4.2.3 in the
 * PR3930 User's Manual Version 2.4
 */

#define CP0_Status_IEc_Mask           	0x00000001   	/* [0]     Interrupt Enable Current (0 disable, 1 enable) (SR_IEC) */
#define CP0_Status_KUc_Mask           	0x00000002   	/* [1]     Kernel/User Mode Current (0 kernel, 1 user) (SR_KUC) */
#define CP0_Status_IEp_Mask           	0x00000004   	/* [2]     Interrupt Enable Previous (SR_IEP) */
#define CP0_Status_KUp_Mask           	0x00000008   	/* [3]     Kernel/User Mode Previous (SR_KUP) */
#define CP0_Status_IEo_Mask           	0x00000010   	/* [4]     Interrupt Enable Old (SR_IEO) */
#define CP0_Status_KUo_Mask           	0x00000020   	/* [5]     Kernel/User Mode Old (SR_KUO) */
#define CP0_Status_Reserved_6_Mask    	0x00000040   	/* [6]     Reserved */
#define CP0_Status_Reserved_7_Mask    	0x00000080   	/* [7]     Reserved*/
#define CP0_Status_SWM0_Mask           	0x00000100   	/* [8]     Software Interrupt 0 Mask (1 sets corresponding bit to enable) */
#define CP0_Status_SWM1_Mask           	0x00000200   	/* [9]     Software Interrupt 1 Mask (1 sets corresponding bit to enable) */
#define CP0_Status_SWMAll_Mask          	0x00000300   	/* [9-8]   Software Interrupt Mask (1 sets corresponding bit to enable) */
#define CP0_Status_IM_Mask            	0x0000FC00   	/* [15-10] Interrupt Mask (1 sets corresponding bit to enable) */
#define CP0_Status_Reserved_16_Mask   	0x00010000   	/* [16]    Reserved (SR_ISC) */
#define CP0_Status_Reserved_17_Mask   	0x00020000   	/* [17]    Reserved (SR_SWC) */ 
#define CP0_Status_SR_Mask            	0x00040000   	/* [18]    Soft Reset (cleared by writing 1, 0 has no effect) (SR_SR) */
#define CP0_Status_Reserved_19_Mask   	0x00080000   	/* [19]    Reserved (SR_CM) */
#define CP0_Status_NMI_Mask           	0x00100000   	/* [20]    Non-maskable Interrupt, Parity Error for R3000A (SR_PE, SR_NMI) */
#define CP0_Status_Reserved_21_Mask   	0x00200000   	/* [21]    Reserved (SR_TS) */
#define CP0_Status_BEV_Mask           	0x00400000   	/* [22]    Boot Exception Vectors (0 normal vectors, 1 bootstrap) (SR_BEV) */
#define CP0_Status_DCD_Mask           	0x00800000   	/* [23]    Data Cache Disable (1 enable) */
#define CP0_Status_ICD_Mask           	0x01000000   	/* [24]    Instruction Cache Disable (1 enable) */
#define CP0_Status_RE_Mask            	0x02000000   	/* [25]    Reverse (Cowboy!) Endian (1 enable) */
#define CP0_Status_Reserved_26_Mask   	0x04000000   	/* [26]    Reserved */
#define CP0_Status_Reserved_27_Mask   	0x08000000   	/* [27]    Reserved */
#define CP0_Status_CU0_Mask           	0x10000000   	/* [28]    Coprocessor Usable (1 enables cp0) (SR_CU0) */
#define CP0_Status_CU1_Mask           	0x20000000   	/* [29]    Coprocessor Usable (1 enables cp1) (SR_CU1) */
#define CP0_Status_CU2_Mask           	0x40000000   	/* [30]    Coprocessor Usable (1 enables cp2) (SR_CU2) */
#define CP0_Status_CU3_Mask           	0x80000000   	/* [31]    Coprocessor Usable (1 enables cp3) (SR_CU3) */

#define CP0_Status_CUAll_Mask         	0xF0000000   	/* [31-28] Coprocessor Usable Bits (SR_CUMASK) */

	
/*
 * Status Interrupt Mask
 *
 * Written to the spec defined in the section Timer Interrupts, page 7 in the 
 * PR3930 Data Sheet Version 2.5
 */

#define CP0_Status_IM_ExtHwInt0_Mask  	0x00000400   	/* [10]    External Hardware Interrupt 0 */
#define CP0_Status_IM_ExtHwInt1_Mask  	0x00000800   	/* [11]    External Hardware Interrupt 1 */
#define CP0_Status_IM_ExtHwInt2_Mask  	0x00001000   	/* [12]    External Hardware Interrupt 2 */
#define CP0_Status_IM_Timer1_Mask     	0x00002000   	/* [13]    Timer 1 */
#define CP0_Status_IM_Timer2_Mask     	0x00004000   	/* [14]    Timer 2 */
#define CP0_Status_IM_Timer3_Mask     	0x00008000   	/* [15]    Timer 3 */
#define CP0_Status_IM_TimerAll_Mask   	0x0000E000   	/* [15-13] All Timers */

/*------------------------------------------------------------------------*/
/*  CP0 DEBUG BITS       						  */
/*------------------------------------------------------------------------*/
#define CP0_DEBUG_SS_EXC (1<<0)             /* single step */
#define CP0_DEBUG_BP_EXC (1<<1)             /* break point */
#define CP0_DEBUG_DBL_EXC (1<<2)            /* data address break load */
#define CP0_DEBUG_DBS_EXC (1<<3)            /* data address break store */
#define CP0_DEBUG_DIB_EXC (1<<4)            /* instruction addr break store */
#define CP0_DEBUG_DINT_EXC (1<<5)           /* processor/bus break */
#define CP0_DEBUG_JTAGRST (1<<7)            /* reset dsu and ejtag */
#define CP0_DEBUG_SST_EN (1<<8)             /* single step enable */
#define CP0_DEBUG_EXC_CODE_SHIFT 10
#define CP0_DEBUG_EXC_CODE_MASK  (0x1f<<CP0_DEBUG_EXC_CODE_SHIFT)
#define CP0_DEBUG_BSSF (1<<10)              /* bus error exception */
#define CP0_DEBUG_TLF (1<<11)               /* tlb exception */
#define CP0_DEBUG_OES (1<<12)               /* other exception status */
#define CP0_DEBUG_UMS (1<<13)               /* UTLB miss exception status */
#define CP0_DEBUG_NIS (1<<14)               /* non maskable interrupt status */
#define CP0_DEBUG_DDBL (1<<18)              /* dbg data-break load imprec. */
#define CP0_DEBUG_DDBS (1<<19)              /* dbg data-break store imprec. */
#define CP0_DEBUG_IEXI (1<<20)              /* imprec. error exc inhibit */
#define CP0_DEBUG_DBEP (1<<21)              /* dbus error exception pending */
#define CP0_DEBUG_CAEP (1<<22)              /* cache error exc pending */
#define CP0_DEBUG_MCEP (1<<23)              /* mach. check exc pending */
#define CP0_DEBUG_IFEP (1<<24)              /* instr fetch error pending */
#define CP0_DEBUG_CNT_DM (1<<25)            /* debug mode count reg runs */
#define CP0_DEBUG_HALT (1<<26)              /* system bus stopped */
#define CP0_DEBUG_LSDM (1<<28)              /* load-store DSEG to memory */
#define CP0_DEBUG_DM (1<<30)                /* debug mode status */
#define CP0_DEBUG_DBD (1<<31)               /* debug branch delay */

/* instruction definitions */
#define MTC0(RT,RD) ((0x408<<20)|((RT)<<16)|((RD)<<11))
#define MFC0(RT,RD) ((0x400<<20)|((RT)<<16)|((RD)<<11))
#define MFLO(RD) (((RD)<<11)|0x12)
#define MFHI(RD) (((RD)<<11)|0x10)
#define MTLO(RD) (((RD)<<11)|0x13)
#define MTHI(RD) (((RD)<<11)|0x11)
#define SW(RT,OFF,BASE) ((0x2B<<26)|((BASE)<<21)|((RT)<<16)|(OFF))
#define LW(RT,OFF,BASE) ((0x23<<26)|((BASE)<<21)|((RT)<<16)|(OFF))
#define LUI(RT,IMM) ((0x0F<<26)|((RT)<<16)|(IMM))
#define OR(RD,RS,RT) ((RS)<<21|(RT)<<16|(RD)<<11|0x25)
#define ORI(RT,RS,IMM) ((0x0D<<26)|((RS)<<21)|((RT)<<16)|(IMM))
#define ADDIU(RT,RS,IMM) ((0x09<<26)|((RS)<<21)|((RT)<<16)|(IMM))
#define ADDI(RT,RS,IMM) ((0x08<<26)|((RS)<<21)|((RT)<<16)|(IMM))
#define XOR(RD,RS,RT) ((RS)<<21|(RT)<<16|(RD)<<11|0x26)
#define XORI(RT,RS,IMM) ((0x38<<24)|((RS)<<21)|((RT)<<16)|(IMM))
#define SDBBP(CODE) (0x7000003F|(((CODE)&0xFFFFF)<<6))
#define MIPS_SYNC    0x0000000f 
#define DERET    0x4200001F 
#define NOP      0x00000000 
#define SSNOP    0x00000040
#define CACHE(OP,OFF,BASE) (0xBC<<24|(BASE)<<21|(OP)<<16|(OFF))
#define ICACHE 0x0
#define DCACHE 0x1

#define EJTAG_REGBITS(X)  ((X)&1 ? 64:32)
#define EJTAG_BRKCHAN(X)  (((X)>>1) & 0xf)
#define EJTAG_IADRBRK(X)  (((X)>>5)&1)         // instruction address break 
#define EJTAG_DADRBRK(X)  (((X)>>6)&1)         // data address break 
#define EJTAG_PBADRBRK(X) (((X)>>7)&1)        // processor bus address break 
	     
#define EJTAG_MANUFID(X)  (((X)>>1)&0x3ff)
#define EJTAG_PARTID(X)   (((X)>>12)&0xFFFF)

/* Breakpoint types.  Values 0, 1, and 2 must agree with the watch
   types passed by breakpoint.c to target_insert_watchpoint.
   Value 3 is our own invention, and is used for ordinary instruction
   breakpoints.  Value 4 is used to mark an unused watchpoint in tables.  */
enum break_type
  {
    BREAK_WRITE,		/* 0 */
    BREAK_READ,			/* 1 */
    BREAK_ACCESS,		/* 2 */
    BREAK_FETCH,		/* 3 */
    BREAK_UNUSED		/* 4 */
  };

#endif /* __REMOTE_MIPS_EJTAG_H__ */
