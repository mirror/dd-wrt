#ifndef CYGONCE_HAL_SPARCLITE_HAL_CPU_H
#define CYGONCE_HAL_SPARCLITE_HAL_CPU_H
// ====================================================================
//
//      hal_cpu.h
//
//      HAL CPU architecture file for MB8683x
//
// ====================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
// ====================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):           Red Hat
// Contributors:        Red Hat, hmt
// Date:                1999-03-01
// Purpose:             MB8683x SPARClite CPU symbols
// Description:
//
//####DESCRIPTIONEND####
//
// ====================================================================


// NOTE: THIS FILE HAS NOT BEEN "CLEANED UP" WRT NAMESPACE USE
//
//        it is only used internally to the SLEB HAL
//
// it should not be exported by inclusion in API header files
//

#ifndef __ASSEMBLER__
/*
 *  Register numbers. These are assumed to match the
 *  register numbers used by GDB.
 */
enum __regnames {
    REG_G0,     REG_G1,     REG_G2,     REG_G3,
    REG_G4,     REG_G5,     REG_G6,     REG_G7,
    REG_O0,     REG_O1,     REG_O2,     REG_O3,
    REG_O4,     REG_O5,     REG_SP,     REG_O7,
    REG_L0,     REG_L1,     REG_L2,     REG_L3,
    REG_L4,     REG_L5,     REG_L6,     REG_L7,
    REG_I0,     REG_I1,     REG_I2,     REG_I3,
    REG_I4,     REG_I5,     REG_FP,     REG_I7,

    REG_F0,     REG_F1,     REG_F2,     REG_F3,
    REG_F4,     REG_F5,     REG_F6,     REG_F7,
    REG_F8,     REG_F9,     REG_F10,    REG_F11,
    REG_F12,    REG_F13,    REG_F14,    REG_F15,
    REG_F16,    REG_F17,    REG_F18,    REG_F19,
    REG_F20,    REG_F21,    REG_F22,    REG_F23,
    REG_F24,    REG_F25,    REG_F26,    REG_F27,
    REG_F28,    REG_F29,    REG_F30,    REG_F31,

    REG_Y,      REG_PSR,    REG_WIM,    REG_TBR,
    REG_PC,     REG_NPC,    REG_FPSR,   REG_CPSR,
    REG_DIA1,   REG_DIA2,   REG_DDA1,   REG_DDA2,
    REG_DDV1,   REG_DDV2,   REG_DCR,    REG_DSR,

    REG_LAST
};
#endif

#ifdef __ASSEMBLER__
/*
 * Macros to glue together two tokens.
 */
#ifdef __STDC__
#define XGLUE(a,b) a##b
#else
#define XGLUE(a,b) a/**/b
#endif

#define GLUE(a,b) XGLUE(a,b)

#ifdef NEED_UNDERSCORE
#define SYM_NAME(name) GLUE(_,name)

        .macro FUNC_START name
        .align 4
        .globl _\name
        .type  _\name,#function
        .proc   04
    _\name:
        .endm

        .macro FUNC_END name
    .LL_\name:
        .size _\name,.LL_\name - _\name
        .endm

#else
#define SYM_NAME(name) name

        .macro FUNC_START name
        .align 4
        .globl \name
        .type  \name,#function
        .proc   04
    \name:
        .endm

        .macro FUNC_END name
    .LL\name:
        .size \name,.LL\name - \name
        .endm

#endif

#endif /* __ASSEMBLER__ */

/*
 *  breakpoint opcode.
 */
#define BREAKPOINT_OPCODE       0x91d02001

/*
 *  inline asm statement to cause breakpoint.
 */
#define BREAKPOINT()    asm volatile ("ta 1\n")

/*
 * Core Exception vectors.
 */
#define BSP_EXC_IACCESS     0
#define BSP_EXC_ILL         1
#define BSP_EXC_IPRIV       2
#define BSP_EXC_FPDIS       3
#define BSP_EXC_WINOVF      4
#define BSP_EXC_WINUND      5
#define BSP_EXC_ALIGN       6
#define BSP_EXC_DACCESS     7
#define BSP_EXC_TAGOVF      8
#define BSP_EXC_INT1        9
#define BSP_EXC_INT2       10
#define BSP_EXC_INT3       11
#define BSP_EXC_INT4       12
#define BSP_EXC_INT5       13
#define BSP_EXC_INT6       14
#define BSP_EXC_INT7       15
#define BSP_EXC_INT8       16
#define BSP_EXC_INT9       17
#define BSP_EXC_INT10      18
#define BSP_EXC_INT11      19
#define BSP_EXC_INT12      20
#define BSP_EXC_INT13      21
#define BSP_EXC_INT14      22
#define BSP_EXC_INT15      23
#define BSP_EXC_CPDIS      24
#define BSP_EXC_BREAK      25
#define BSP_EXC_WINFLUSH   26
#define BSP_EXC_SYSCALL    27
#define BSP_EXC_DEBUG      28
#define BSP_EXC_TRAP       29

#define BSP_MAX_EXCEPTIONS 30

#define BSP_VEC_MT_DEBUG   30
#define BSP_VEC_STUB_ENTRY 31
#define BSP_VEC_BSPDATA    32

#define NUM_VTAB_ENTRIES   33

#define CPU_WINSIZE 8

/*
 * Exception frame offsets.
 */
#define GPR_SIZE 4
#define FPR_SIZE 4
#define PTR_BYTES 4

/* Leave room for locals + hidden arg + arg spill + dword align */
#define FR_BIAS   ((16+1+6+1)*GPR_SIZE)

#define FR_G0     FR_BIAS
#define FR_G1     (FR_G0 + GPR_SIZE)
#define FR_G2     (FR_G1 + GPR_SIZE)
#define FR_G3     (FR_G2 + GPR_SIZE)
#define FR_G4     (FR_G3 + GPR_SIZE)
#define FR_G5     (FR_G4 + GPR_SIZE)
#define FR_G6     (FR_G5 + GPR_SIZE)
#define FR_G7     (FR_G6 + GPR_SIZE)
#define FR_O0     (FR_G7 + GPR_SIZE)
#define FR_O1     (FR_O0 + GPR_SIZE)
#define FR_O2     (FR_O1 + GPR_SIZE)
#define FR_O3     (FR_O2 + GPR_SIZE)
#define FR_O4     (FR_O3 + GPR_SIZE)
#define FR_O5     (FR_O4 + GPR_SIZE)
#define FR_O6     (FR_O5 + GPR_SIZE)
#define FR_SP     FR_O6
#define FR_O7     (FR_SP + GPR_SIZE)
#define FR_L0     (FR_O7 + GPR_SIZE)
#define FR_L1     (FR_L0 + GPR_SIZE)
#define FR_L2     (FR_L1 + GPR_SIZE)
#define FR_L3     (FR_L2 + GPR_SIZE)
#define FR_L4     (FR_L3 + GPR_SIZE)
#define FR_L5     (FR_L4 + GPR_SIZE)
#define FR_L6     (FR_L5 + GPR_SIZE)
#define FR_L7     (FR_L6 + GPR_SIZE)
#define FR_I0     (FR_L7 + GPR_SIZE)
#define FR_I1     (FR_I0 + GPR_SIZE)
#define FR_I2     (FR_I1 + GPR_SIZE)
#define FR_I3     (FR_I2 + GPR_SIZE)
#define FR_I4     (FR_I3 + GPR_SIZE)
#define FR_I5     (FR_I4 + GPR_SIZE)
#define FR_I6     (FR_I5 + GPR_SIZE)
#define FR_FP     FR_I6
#define FR_I7     (FR_FP + GPR_SIZE)

#define FR_FREG0  (FR_I7 + GPR_SIZE)
#define FR_FREG1  (FR_FREG0 + FPR_SIZE)
#define FR_FREG2  (FR_FREG1 + FPR_SIZE)
#define FR_FREG3  (FR_FREG2 + FPR_SIZE)
#define FR_FREG4  (FR_FREG3 + FPR_SIZE)
#define FR_FREG5  (FR_FREG4 + FPR_SIZE)
#define FR_FREG6  (FR_FREG5 + FPR_SIZE)
#define FR_FREG7  (FR_FREG6 + FPR_SIZE)
#define FR_FREG8  (FR_FREG7 + FPR_SIZE)
#define FR_FREG9  (FR_FREG8 + FPR_SIZE)
#define FR_FREG10 (FR_FREG9 + FPR_SIZE)
#define FR_FREG11 (FR_FREG10 + FPR_SIZE)
#define FR_FREG12 (FR_FREG11 + FPR_SIZE)
#define FR_FREG13 (FR_FREG12 + FPR_SIZE)
#define FR_FREG14 (FR_FREG13 + FPR_SIZE)
#define FR_FREG15 (FR_FREG14 + FPR_SIZE)
#define FR_FREG16 (FR_FREG15 + FPR_SIZE)
#define FR_FREG17 (FR_FREG16 + FPR_SIZE)
#define FR_FREG18 (FR_FREG17 + FPR_SIZE)
#define FR_FREG19 (FR_FREG18 + FPR_SIZE)
#define FR_FREG20 (FR_FREG19 + FPR_SIZE)
#define FR_FREG21 (FR_FREG20 + FPR_SIZE)
#define FR_FREG22 (FR_FREG21 + FPR_SIZE)
#define FR_FREG23 (FR_FREG22 + FPR_SIZE)
#define FR_FREG24 (FR_FREG23 + FPR_SIZE)
#define FR_FREG25 (FR_FREG24 + FPR_SIZE)
#define FR_FREG26 (FR_FREG25 + FPR_SIZE)
#define FR_FREG27 (FR_FREG26 + FPR_SIZE)
#define FR_FREG28 (FR_FREG27 + FPR_SIZE)
#define FR_FREG29 (FR_FREG28 + FPR_SIZE)
#define FR_FREG30 (FR_FREG29 + FPR_SIZE)
#define FR_FREG31 (FR_FREG30 + FPR_SIZE)

#define FR_Y      (FR_FREG31 + FPR_SIZE)
#define FR_PSR    (FR_Y      + GPR_SIZE)
#define FR_WIM    (FR_PSR    + GPR_SIZE)
#define FR_TBR    (FR_WIM    + GPR_SIZE)
#define FR_PC     (FR_TBR    + GPR_SIZE)
#define FR_NPC    (FR_PC     + GPR_SIZE)
#define FR_FPSR   (FR_NPC    + GPR_SIZE)
#define FR_CPSR   (FR_FPSR   + GPR_SIZE)
#define FR_DIA1   (FR_CPSR   + GPR_SIZE)
#define FR_DIA2   (FR_DIA1   + GPR_SIZE)
#define FR_DDA1   (FR_DIA2   + GPR_SIZE)
#define FR_DDA2   (FR_DDA1   + GPR_SIZE)
#define FR_DDV1   (FR_DDA2   + GPR_SIZE)
#define FR_DDV2   (FR_DDV1   + GPR_SIZE)
#define FR_DCR    (FR_DDV2   + GPR_SIZE)
#define FR_DSR    (FR_DCR    + GPR_SIZE)
#define FR_ASR17  (FR_DSR    + GPR_SIZE)

#define EX_STACK_SIZE (FR_ASR17 + GPR_SIZE)

#ifndef __ASSEMBLER__
/*
 *  How registers are stored for exceptions.
 */
typedef struct
{
    unsigned long _g0;
    unsigned long _g1;
    unsigned long _g2;
    unsigned long _g3;
    unsigned long _g4;
    unsigned long _g5;
    unsigned long _g6;
    unsigned long _g7;
    unsigned long _o0;
    unsigned long _o1;
    unsigned long _o2;
    unsigned long _o3;
    unsigned long _o4;
    unsigned long _o5;
    unsigned long _sp;
    unsigned long _o7;
    unsigned long _l0;
    unsigned long _l1;
    unsigned long _l2;
    unsigned long _l3;
    unsigned long _l4;
    unsigned long _l5;
    unsigned long _l6;
    unsigned long _l7;
    unsigned long _i0;
    unsigned long _i1;
    unsigned long _i2;
    unsigned long _i3;
    unsigned long _i4;
    unsigned long _i5;
    unsigned long _fp;
    unsigned long _i7;

    unsigned long _fpr[32];

    unsigned long _y;
    unsigned long _psr;
    unsigned long _wim;
    unsigned long _tbr;

    unsigned long _pc;
    unsigned long _npc;
    unsigned long _fpsr;
    unsigned long _cpsr;
    unsigned long _dia1;
    unsigned long _dia2;
    unsigned long _dda1;
    unsigned long _dda2;
    unsigned long _ddv1;
    unsigned long _ddv2;
    unsigned long _dcr;
    unsigned long _dsr;

    unsigned long _asr17;

} ex_regs_t;

/*
 *  How gdb expects registers to be stored.
 */
typedef struct
{
    unsigned long _g0;
    unsigned long _g1;
    unsigned long _g2;
    unsigned long _g3;
    unsigned long _g4;
    unsigned long _g5;
    unsigned long _g6;
    unsigned long _g7;
    unsigned long _o0;
    unsigned long _o1;
    unsigned long _o2;
    unsigned long _o3;
    unsigned long _o4;
    unsigned long _o5;
    unsigned long _sp;
    unsigned long _o7;
    unsigned long _l0;
    unsigned long _l1;
    unsigned long _l2;
    unsigned long _l3;
    unsigned long _l4;
    unsigned long _l5;
    unsigned long _l6;
    unsigned long _l7;
    unsigned long _i0;
    unsigned long _i1;
    unsigned long _i2;
    unsigned long _i3;
    unsigned long _i4;
    unsigned long _i5;
    unsigned long _fp;
    unsigned long _i7;
    unsigned long _fpr[32];
    unsigned long _y;
    unsigned long _psr;
    unsigned long _wim;
    unsigned long _tbr;
    unsigned long _pc;
    unsigned long _npc;
    unsigned long _fpsr;
    unsigned long _cpsr;
    unsigned long _dia1;
    unsigned long _dia2;
    unsigned long _dda1;
    unsigned long _dda2;
    unsigned long _ddv1;
    unsigned long _ddv2;
    unsigned long _dcr;
    unsigned long _dsr;
} gdb_regs_t;


extern void __dcache_flush(void *addr, int nbytes);
extern void __icache_flush(void *addr, int nbytes);
extern int  __dcache_disable(void);
extern void __dcache_enable(void);
extern void __icache_disable(void);
extern void __icache_enable(void);


#endif /* !__ASSEMBLER__ */


#define PSR_INIT 0xfa7


/*
 * Memory-mapped (ASI=1) registers for MB8683x series.
 */
#define CBIR    0x00000000
#define LCR     0x00000004
#define LCSR    0x00000008
#define CSR     0x0000000C
#define RLCR    0x00000010
#define BCR     0x00000020
#define SSCR    0x00000080

#define SPGMR   0x00000120
#define ARSR1   0x00000124
#define ARSR2   0x00000128
#define ARSR3   0x0000012C
#define ARSR4   0x00000130
#define ARSR5   0x00000134
#define AMR0    0x00000140
#define AMR1    0x00000144
#define AMR2    0x00000148
#define AMR3    0x0000014C
#define AMR4    0x00000150
#define AMR5    0x00000154
#define WSSR0   0x00000160
#define WSSR1   0x00000164
#define WSSR2   0x00000168
#define BWCR    0x0000016C
#define REFTMR  0x00000174
#define DRLD    0x00000178
#define VER2    0x00020000
#define SLPMD   0x00020004

/* CBIR bit fields */
#define CBIR_ICEN    0x01  /* Icache enable */
#define CBIR_ICLOCK  0x02  /* Icache lock   */
#define CBIR_DCEN    0x04  /* Dcache enable */
#define CBIR_DCLOCK  0x08  /* Dcache lock   */
#define CBIR_PBEN    0x10  /* Prefetch Buffer enable */
#define CBIR_WBEN    0x20  /* Write Buffer enable */

/* LCR bit fields */
#define LCR_ILOCK    0x01  /* Icache Auto-lock enable */
#define LCR_DLOCK    0x02  /* Dcache Auto-lock enable */

/* WSSRn bit fields */
#define WSSR_OVERRIDE 1
#define WSSR_SINGLE   2
#define WSSR_WAITEN   4

#define WSSR_CNT1_SHIFT 8
#define WSSR_CNT2_SHIFT 3
#define WSSR_CS0_SHIFT  6
#define WSSR_CS1_SHIFT  19

#define WSSR_SUBVAL(c1,c2,flags) (((c1)<<8)|((c2)<<3)|(flags))
#define WSSR_VAL(c1b,c2b,flagsb,c1a,c2a,flagsa) \
           ((WSSR_SUBVAL(c1b,c2b,flagsb)<<19)|(WSSR_SUBVAL(c1a,c2a,flagsa)<<6))

#define SPGMR_VAL(asi,addr) (((asi)<<23)|(((addr)>>9)&0x007ffffe))
#define ARSR_VAL(asi,base)  ((((asi)&0xff)<<23)|(((base)>>9)&0x007ffffe))
#define AMR_VAL(asi,base)   (((~(asi))<<23)|(((~(base))>>9)&0x007fffff))

/* SSCR bit fields */
#define SSCR_TIMER  0x04
#define SSCR_WAIT   0x08
#define SSCR_CS     0x10
#define SSCR_SAMEPG 0x20
#define SSCR_DRAM   0x40
#define SSCR_BURST  0x80

/* BCR bit fields */
#define BCR_IBE   1
#define BCR_DBE   2


/* DRAM Controller registers as offsets into CS3 space */
#define DBANKR  0x20
#define DTIMR   0x24

/* DBANKR bit fields */
#define DBANKR_512K     0
#define DBANKR_1M       1
#define DBANKR_2M       2
#define DBANKR_4M       3
#define DBANKR_8M       4
#define DBANKR_16M      5
#define DBANKR_32M      6
#define DBANKR_64M      7
#define DBANKR_CA9      (2<<4)
#define DBANKR_CA10     (3<<4)
#define DBANKR_CA11     (4<<4)
#define DBANKR_CA12     (5<<4)
#define DBANKR_4C1W     (0<<7)  /* 4CAS/1WE */
#define DBANKR_1C4W     (1<<7)  /* 1CAS/4WE */
#define DBANKR_EDO      (1<<8)
#define DBANKR_SA01     (1<<9)
#define DBANKR_SA02     (2<<9)
#define DBANKR_SA04     (3<<9)

/* DTIMR bit fields */
#define DTIMR_RP1     0
#define DTIMR_RP2     1
#define DTIMR_CAS1    (0<<1)
#define DTIMR_CAS2    (1<<1)
#define DTIMR_CBR1    (0<<2)
#define DTIMR_CBR2    (1<<2)
#define DTIMR_CBR3    (2<<2)
#define DTIMR_RPS2    (0<<4)
#define DTIMR_RPS4    (1<<4)


/* -------------------------------------------------------------------*/
#endif  /* CYGONCE_HAL_SPARCLITE_HAL_CPU_H */
/* EOF hal_cpu.h */
