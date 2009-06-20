/*
 *  Copyright (c) 2003 Instant802 Networks, Inc.,  All Rights Reserved.
 *  Copyright (c) 2001 Atheros Communications, Inc.,  All Rights Reserved.
 */

/*
 * mips4Kx.h - mips 4Kx specific definitions
 */

#ifndef __MIPS4KX_H
#define __MIPS4KX_H

/*
 * CP0_CONFIG (select 0) register.
 */
#define CFG_K0MASK      0x00000007	/* KSEG0 coherency algorithm */
#define CFG_MT          0x00000380	/* MMU type */
#define CFG_MT_TLB      0x00000080	/* standard TLB (4Kc) */
#define CFG_MT_FIXED    0x00000180	/* fixed mapping (4Kp, 4Km) */
#define CFG_AR          0x00001c00	/* architecture revision */
#define CFG_AT          0x00006000	/* architecture type */
#define CFG_BE          0x00008000	/* big endian */
#define CFG_BM          0x00010000	/* burst order */
#define CFG_MM          0x00060000	/* write buffer merge mode */
#define CFG_MM_NONE     0x00000000
#define CFG_WC          0x00080000	/* WE for MT (charter only) */
#define CFG_MDU         0x00100000	/* interative multiply (4Kp) */
#define CFG_KU          0x0e000000	/* k/useg cacheability */
#define CFG_KU_S        25
#define CFG_K23         0x70000000	/* kseg2/3 cacheability */
#define CFG_K23_S       28
#define CFG_M           0x80000000	/* C0_CONFIG1 exists */

/*
 * CP0_CONFIG (select 1) register.
 */
#define CFG1_FP         0x00000001	/* FPU implemented (0) */
#define CFG1_EP         0x00000002	/* EJTAG implemented (1) */
#define CFG1_CA         0x00000004	/* MIPS16 implemented (0) */
#define CFG1_WR         0x00000008	/* watch implemented (1) */
#define CFG1_PC         0x00000010	/* perf count implemented (0) */
#define CFG1_DA         0x00000380	/* Num of d$ ways - 1 */
#define CFG1_DA_S       7
#define CFG1_DL         0x00001c00	/* d$ line size */
#define CFG1_DL_S       10
#define CFG1_DS         0x0000e000	/* Num of d$ sets per way */
#define CFG1_DS_S       13
#define CFG1_IA         0x00070000	/* Num of i$ ways - 1 */
#define CFG1_IA_S       16
#define CFG1_IL         0x00380000	/* i$ line size */
#define CFG1_IL_S       19
#define CFG1_IS         0x01c00000	/* Num of i$ sets per way */
#define CFG1_IS_S       22
#define CFG1_IS_64      0
#define CFG1_IS_128     1
#define CFG1_IS_256     2
#define CFG1_MMUSIZE    0x7e000000	/* Num tlb entries - 1 */
#define CFG1_MMUSIZE_S  25

#define CAUSE_IV        0x00800000	/* enable seperate interrupt vector */

#define SR_NMI          0x00080000	/* reset due to NMI */

#define MFC0_T0_CONFIG1 .word 0x40088001	/* Not built into gas */

/*
 * Override r4000.h defines that are not correct for the 4Kx.
 */
#undef CFG_CM
#undef CFG_ECMASK
#undef CFG_ECBY2
#undef CFG_ECBY3
#undef CFG_ECBY4
#undef CFG_EPMASK
#undef CFG_EPD
#undef CFG_SBMASK
#undef CFG_SBSHIFT
#undef CFG_SB4
#undef CFG_SB8
#undef CFG_SB16
#undef CFG_SB32
#undef CFG_SS
#undef CFG_SW
#undef CFG_EWMASK
#undef CFG_EWSHIFT
#undef CFG_EW64
#undef CFG_EW32
#undef CFG_SC
#undef CFG_SM
#undef CFG_EM
#undef CFG_EB
#undef CFG_ICMASK
#undef CFG_ICSHIFT
#undef CFG_DCMASK
#undef CFG_DCSHIFT
#undef CFG_IB
#undef CFG_DB
#undef CFG_CU

#undef SR_DE

#endif				/* __mips4Kx_h */
