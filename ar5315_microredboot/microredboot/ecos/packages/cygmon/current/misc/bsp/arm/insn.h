#ifndef __BSP_ARM_INSN_H__
#define __BSP_ARM_INSN_H__
//==========================================================================
//
//      insn.h
//
//      ARM(R) instruction descriptions.
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    
// Contributors: gthomas
// Date:         1999-10-20
// Purpose:      ARM(R) instruction descriptions.
// Description:  ARM is a Registered Trademark of Advanced RISC Machines
//               Limited.
//               Other Brands and Trademarks are the property of their
//               respective owners.
//
//####DESCRIPTIONEND####
//
//=========================================================================


/* Data Processing Immediate Type */
struct dpi_type {
    unsigned immediate   : 8;
    unsigned rotate      : 4;
    unsigned Rd          : 4;
    unsigned Rn          : 4;
    unsigned S_bit       : 1;
    unsigned opcode      : 4;
    unsigned rsv1        : 3;  /* == 001b */
    unsigned cond        : 4;
};
#define DPI_RSV1_VALUE 0x1

/* Data Processing Immediate Shift Type */
struct dpis_type {
    unsigned Rm          : 4;
    unsigned rsv2        : 1;  /* == 0b */
    unsigned shift       : 2;
    unsigned shift_immed : 5;
    unsigned Rd          : 4;
    unsigned Rn          : 4;
    unsigned S_bit       : 1;
    unsigned opcode      : 4;
    unsigned rsv1        : 3;  /* == 000b */
    unsigned cond        : 4;
};
#define DPIS_RSV1_VALUE 0x0
#define DPIS_RSV2_VALUE 0x0

/* Data Processing Register Shift Type */
struct dprs_type {
    unsigned Rm          : 4;
    unsigned rsv3        : 1;  /* == 1b */
    unsigned shift       : 2;
    unsigned rsv2        : 1;  /* == 0b */
    unsigned Rs          : 4;
    unsigned Rd          : 4;
    unsigned Rn          : 4;
    unsigned S_bit       : 1;
    unsigned opcode      : 4;
    unsigned rsv1        : 3;  /* == 000b */
    unsigned cond        : 4;
};
#define DPRS_RSV1_VALUE 0x0
#define DPRS_RSV2_VALUE 0x0
#define DPRS_RSV3_VALUE 0x1

/* Multiply Type */
struct m_type {
    unsigned Rm          : 4;
    unsigned rsv2        : 4;  /* == 1001b */
    unsigned Rs          : 4;
    unsigned Rn          : 4;
    unsigned Rd          : 4;
    unsigned S_bit       : 1;
    unsigned A_bit       : 1;
    unsigned rsv1        : 6;  /* == 000000b */
    unsigned cond        : 4;
};
#define M_RSV1_VALUE 0x0
#define M_RSV2_VALUE 0x9

/* Multiply Long Type */
struct ml_type {
    unsigned Rm          : 4;
    unsigned rsv2        : 4;  /* == 1001b */
    unsigned Rs          : 4;
    unsigned RdLo        : 4;
    unsigned RdHi        : 4;
    unsigned S_bit       : 1;
    unsigned A_bit       : 1;
    unsigned U_bit       : 1;
    unsigned rsv1        : 5;  /* == 00001b */
    unsigned cond        : 4;
};
#define ML_RSV1_VALUE 0x1
#define ML_RSV2_VALUE 0x9

/* Move from status register Type */
struct mrs_type {
    unsigned SBZ         : 12;
    unsigned Rd          : 4;
    unsigned SBO         : 4;
    unsigned rsv2        : 2;  /* == 00b */
    unsigned R_bit       : 1;
    unsigned rsv1        : 5;  /* == 00010b */
    unsigned cond        : 4;
};
#define MRS_RSV1_VALUE 0x2
#define MRS_RSV2_VALUE 0x0

/* Move Immediate to status register Type */
struct misr_type {
    unsigned immediate   : 8;
    unsigned rotate      : 4;
    unsigned SBO         : 4;
    unsigned mask        : 4;
    unsigned rsv2        : 2;  /* == 10b */
    unsigned R_bit       : 1;
    unsigned rsv1        : 5;  /* == 00110b */
    unsigned cond        : 4;
};
#define MISR_RSV1_VALUE 0x6
#define MISR_RSV2_VALUE 0x2

/* Move register to status register Type */
struct mrsr_type {
    unsigned Rm          : 4;
    unsigned rsv3        : 1;  /* == 0b */
    unsigned SBZ         : 7;
    unsigned SBO         : 4;
    unsigned mask        : 4;
    unsigned rsv2        : 2;  /* == 10b */
    unsigned R_bit       : 1;
    unsigned rsv1        : 5;  /* == 00010b */
    unsigned cond        : 4;
};
#define MRSR_RSV1_VALUE 0x2
#define MRSR_RSV2_VALUE 0x2
#define MRSR_RSV3_VALUE 0x0

/* Branch/Exchange Type */
struct bx_type {
    unsigned Rm          : 4;
    unsigned rsv2        : 4;  /* == 0001b */
    unsigned SBO3        : 4;
    unsigned SBO2        : 4;
    unsigned SBO1        : 4;
    unsigned rsv1        : 8;  /* == 00010010b */
    unsigned cond        : 4;
};
#define BX_RSV1_VALUE 0x12
#define BX_RSV2_VALUE 0x1

/* Load/Store Immediate Offset Type */
struct lsio_type {
    unsigned immediate   : 12;
    unsigned Rd          : 4;
    unsigned Rn          : 4;
    unsigned L_bit       : 1;
    unsigned W_bit       : 1;
    unsigned B_bit       : 1;
    unsigned U_bit       : 1;
    unsigned P_bit       : 1;
    unsigned rsv1        : 3;  /* == 010b */
    unsigned cond        : 4;
};
#define LSIO_RSV1_VALUE 0x2

/* Load/Store Register Offset Type */
struct lsro_type {
    unsigned Rm          : 4;
    unsigned rsv2        : 1;  /* == 0b */
    unsigned shift       : 2;
    unsigned shift_immed : 5;
    unsigned Rd          : 4;
    unsigned Rn          : 4;
    unsigned L_bit       : 1;
    unsigned W_bit       : 1;
    unsigned B_bit       : 1;
    unsigned U_bit       : 1;
    unsigned P_bit       : 1;
    unsigned rsv1        : 3;  /* == 011b */
    unsigned cond        : 4;
};
#define LSRO_RSV1_VALUE 0x3
#define LSRO_RSV2_VALUE 0x0

/* Load/Store halfword/signed byte Immediate Offset Type */
struct lshwi_type {
    unsigned Lo_Offset   : 4;
    unsigned rsv4        : 1;  /* == 1b */
    unsigned H_bit       : 1;
    unsigned S_bit       : 1;
    unsigned rsv3        : 1;  /* == 1b */
    unsigned Hi_Offset   : 4;
    unsigned Rd          : 4;
    unsigned Rn          : 4;
    unsigned L_bit       : 1;
    unsigned W_bit       : 1;
    unsigned rsv2        : 1;  /* == 1b */
    unsigned U_bit       : 1;
    unsigned P_bit       : 1;
    unsigned rsv1        : 3;  /* == 000b */
    unsigned cond        : 4;
};
#define LSHWI_RSV1_VALUE 0x0
#define LSHWI_RSV2_VALUE 0x1
#define LSHWI_RSV3_VALUE 0x1
#define LSHWI_RSV4_VALUE 0x1

/* Load/Store halfword/signed byte Register Offset Type */
struct lshwr_type {
    unsigned Rm          : 4;
    unsigned rsv4        : 1;  /* == 1b */
    unsigned H_bit       : 1;
    unsigned S_bit       : 1;
    unsigned rsv3        : 1;  /* == 1b */
    unsigned SBZ         : 4;
    unsigned Rd          : 4;
    unsigned Rn          : 4;
    unsigned L_bit       : 1;
    unsigned W_bit       : 1;
    unsigned rsv2        : 1;  /* == 0b */
    unsigned U_bit       : 1;
    unsigned P_bit       : 1;
    unsigned rsv1        : 3;  /* == 000b */
    unsigned cond        : 4;
};
#define LSHWR_RSV1_VALUE 0x3
#define LSHWR_RSV2_VALUE 0x1
#define LSHWR_RSV3_VALUE 0x1
#define LSHWR_RSV4_VALUE 0x1

/* Swap/Swap Byte Type */
struct swap_type {
    unsigned Rm          : 4;
    unsigned rsv3        : 4;  /* == 1001b */
    unsigned SBZ         : 4;
    unsigned Rd          : 4;
    unsigned Rn          : 4;
    unsigned rsv2        : 2;  /* == 00b */
    unsigned B_bit       : 1;
    unsigned rsv1        : 5;  /* == 00010b */
    unsigned cond        : 4;
};
#define SWAP_RSV1_VALUE 0x2
#define SWAP_RSV2_VALUE 0x0
#define SWAP_RSV3_VALUE 0x9

/* Load/Store Multiple Type */
struct lsm_type {
    unsigned Reg_List    : 16 ;
    unsigned Rn          : 4;
    unsigned L_bit       : 1;
    unsigned W_bit       : 1;
    unsigned S_bit       : 1;
    unsigned U_bit       : 1;
    unsigned P_bit       : 1;
    unsigned rsv1        : 3;  /* == 100b */
    unsigned cond        : 4;
};
#define LSM_RSV1_VALUE 0x4

/* Coprocessor Data Processing Type */
struct cpdp_type {
    unsigned CRm         : 4;
    unsigned rsv2        : 1;  /* == 0b */
    unsigned op2         : 3;
    unsigned cp_num      : 4;
    unsigned CRd         : 4;
    unsigned CRn         : 4;
    unsigned op1         : 4;
    unsigned rsv1        : 4;  /* == 1110b */
    unsigned cond        : 4;
};
#define CPDP_RSV1_VALUE 0xE
#define CPDP_RSV2_VALUE 0x0

/* Coprocessor Register Transfer Type */
struct cprt_type {
    unsigned CRm         : 4;
    unsigned rsv2        : 1;  /* == 1b */
    unsigned op2         : 3;
    unsigned cp_num      : 4;
    unsigned Rd          : 4;
    unsigned CRn         : 4;
    unsigned L_bit       : 1;
    unsigned op1         : 3;
    unsigned rsv1        : 4;  /* == 1110b */
    unsigned cond        : 4;
};
#define CPRT_RSV1_VALUE 0xE
#define CPRT_RSV2_VALUE 0x1

/* Coprocessor Load/Store Type */
struct cpls_type {
    unsigned offset      : 8;
    unsigned cp_num      : 4;
    unsigned CRd         : 4;
    unsigned Rn          : 4;
    unsigned L_bit       : 1;
    unsigned W_bit       : 1;
    unsigned N_bit       : 1;
    unsigned U_bit       : 1;
    unsigned P_bit       : 1;
    unsigned rsv1        : 3;  /* == 110b */
    unsigned cond        : 4;
};
#define CPLS_RSV1_VALUE 0x6

/* Branch/Branch w/ Link Type */
struct bbl_type {
    unsigned offset      : 24;
    unsigned L_bit       : 1;
    unsigned rsv1        : 3;  /* == 101b */
    unsigned cond        : 4;
};
#define BBL_RSV1_VALUE 0x5

/* SWI Type */
struct swi_type {
    unsigned swi_number  : 24;
    unsigned rsv1        : 4;  /* == 1111b */
    unsigned cond        : 4;
};
#define SWI_RSV1_VALUE 0xF

/* Undefined Instruction Type */
struct undef_type {
    unsigned pad2        : 4;
    unsigned rsv2        : 1;  /* == 1b */
    unsigned pad1        : 20;
    unsigned rsv1        : 3;  /* == 011b */
    unsigned cond        : 4;
};
#define UNDEF_RSV1_VALUE 0x3
#define UNDEF_RSV2_VALUE 0x1

union arm_insn {
    unsigned long          word;
    struct dpi_type        dpi;
    struct dpis_type       dpis;
    struct dprs_type       dprs;
    struct m_type          m;
    struct ml_type         ml;
    struct mrs_type        mrs;
    struct misr_type       misr;
    struct mrsr_type       mrsr;
    struct bx_type         bx;
    struct lsio_type       lsio;
    struct lsro_type       lsro;
    struct lshwi_type      lshwi;
    struct lshwr_type      lshwr;
    struct swap_type       swap;
    struct lsm_type        lsm;
    struct cpdp_type       cpdp;
    struct cprt_type       cprt;
    struct cpls_type       cpls;
    struct bbl_type        bbl;
    struct swi_type        swi;
    struct undef_type      undef;
};

/*
 * Conditional field values
 */
#define COND_EQ     0x0
#define COND_NE     0x1
#define COND_CS_HI  0x2
#define COND_CC_LO  0x3
#define COND_MI     0x4
#define COND_PL     0x5
#define COND_VS     0x6
#define COND_VC     0x7
#define COND_HI     0x8
#define COND_LS     0x9
#define COND_GE     0xA
#define COND_LT     0xB
#define COND_GT     0xC
#define COND_LE     0xD
#define COND_AL     0xE
#define COND_NV     0xF

/*
 * Data Processiong Opcode field values
 */
#define DP_OPCODE_MOV  0xD
#define DP_OPCODE_MVN  0xF
#define DP_OPCODE_ADD  0x4
#define DP_OPCODE_ADC  0x5
#define DP_OPCODE_SUB  0x2
#define DP_OPCODE_SBC  0x6
#define DP_OPCODE_RSB  0x3
#define DP_OPCODE_RSC  0x7
#define DP_OPCODE_AND  0x0
#define DP_OPCODE_EOR  0x1
#define DP_OPCODE_ORR  0xC
#define DP_OPCODE_BIC  0xE
#define DP_OPCODE_CMP  0xA
#define DP_OPCODE_CMN  0xB
#define DP_OPCODE_TST  0x8
#define DP_OPCODE_TEQ  0x9

/*
 * Shift field values
 */
#define SHIFT_LSL   0x0
#define SHIFT_LSR   0x1
#define SHIFT_ASR   0x2
#define SHIFT_ROR   0x3
#define SHIFT_RRX   0x3    /* Special case: ROR(0) implies RRX */

/*
 * Load/Store indexing definitions
 */
#define LS_INDEX_POST      0x0
#define LS_INDEX_PRE       0x1

/*
 * Load/Store offset operation definitions
 */
#define LS_OFFSET_SUB      0x0
#define LS_OFFSET_ADD      0x1

/*
 * Load/Store size definitions
 */
#define LS_SIZE_WORD       0x0
#define LS_SIZE_BYTE       0x1

/*
 * Load/Store Update definitions
 */
#define LS_NO_UPDATE       0x0
#define LS_UPDATE          0x1

/*
 * Load/Store Opcode definitions
 */
#define LS_STORE           0x0
#define LS_LOAD            0x1

#endif // __BSP_ARM_INSN_H__
