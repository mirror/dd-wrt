//=============================================================================
//
//      mod_regs_eth.h
//
//      EtherC (ethernet controller) Module register definitions
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   jskov
// Contributors:jskov
// Date:        2002-01-30
//              
//####DESCRIPTIONEND####
//
//=============================================================================

#define CYGARC_REG_EDMR                 0xfffffd00
#define CYGARC_REG_EDTRR                0xfffffd04
#define CYGARC_REG_EDRRR                0xfffffd08
#define CYGARC_REG_TDLAR                0xfffffd0c
#define CYGARC_REG_RDLAR                0xfffffd10
#define CYGARC_REG_EESR                 0xfffffd14
#define CYGARC_REG_EESIPR               0xfffffd18
#define CYGARC_REG_TRSCER               0xfffffd1c
#define CYGARC_REG_RMFCR                0xfffffd20
#define CYGARC_REG_TFTR                 0xfffffd24
#define CYGARC_REG_FDR                  0xfffffd28
#define CYGARC_REG_RCR                  0xfffffd2c
#define CYGARC_REG_EDOCR                0xfffffd30
#define CYGARC_REG_RBWAR                0xfffffd40
#define CYGARC_REG_RDFAR                0xfffffd44
#define CYGARC_REG_TBRAR                0xfffffd4c
#define CYGARC_REG_TDFAR                0xfffffd50

#define CYGARC_REG_ECMR                 0xfffffd60
#define CYGARC_REG_ECSR                 0xfffffd64
#define CYGARC_REG_ECSIPR               0xfffffd68
#define CYGARC_REG_PIR                  0xfffffd6c
#define CYGARC_REG_MAHR                 0xfffffd70
#define CYGARC_REG_MALR                 0xfffffd74
#define CYGARC_REG_RFLR                 0xfffffd78
#define CYGARC_REG_PSR                  0xfffffd7c
#define CYGARC_REG_TROCR                0xfffffd80
#define CYGARC_REG_CDCR                 0xfffffd84
#define CYGARC_REG_LCCR                 0xfffffd88
#define CYGARC_REG_CNDCR                0xfffffd8c
#define CYGARC_REG_IFLCR                0xfffffd90
#define CYGARC_REG_CECFR                0xfffffd94
#define CYGARC_REG_FRECR                0xfffffd98
#define CYGARC_REG_TSFRCR               0xfffffd9c
#define CYGARC_REG_TLFRCR               0xfffffda0
#define CYGARC_REG_RFCR                 0xfffffda4
#define CYGARC_REG_MAFCR                0xfffffda8


#define CYGARC_REG_PIR_MDI              0x00000008
#define CYGARC_REG_PIR_MDO              0x00000004
#define CYGARC_REG_PIR_MMD_WRITE        0x00000002
#define CYGARC_REG_PIR_MMD_READ         0x00000000
#define CYGARC_REG_PIR_MDC              0x00000001

#define CYGARC_REG_RCR_RNC              0x00000001

#define CYGARC_REG_EESR_RFCOF           0x01000000
#define CYGARC_REG_EESR_ECI             0x00400000
#define CYGARC_REG_EESR_TC              0x00200000
#define CYGARC_REG_EESR_TDE             0x00100000
#define CYGARC_REG_EESR_TFUF            0x00080000
#define CYGARC_REG_EESR_FR              0x00040000
#define CYGARC_REG_EESR_RDE             0x00020000
#define CYGARC_REG_EESR_RFOF            0x00010000
#define CYGARC_REG_EESR_ITF             0x00001000
#define CYGARC_REG_EESR_CND             0x00000800
#define CYGARC_REG_EESR_DLC             0x00000400
#define CYGARC_REG_EESR_CD              0x00000200
#define CYGARC_REG_EESR_TRO             0x00000100
#define CYGARC_REG_EESR_RMAF            0x00000080
#define CYGARC_REG_EESR_RRF             0x00000010
#define CYGARC_REG_EESR_RTLF            0x00000008
#define CYGARC_REG_EESR_RTSF            0x00000004
#define CYGARC_REG_EESR_PRE             0x00000002
#define CYGARC_REG_EESR_CERF            0x00000001

#define CYGARC_REG_EESIPR_RFCOFIP       0x01000000
#define CYGARC_REG_EESIPR_ECIIP         0x00400000
#define CYGARC_REG_EESIPR_TCIP          0x00200000
#define CYGARC_REG_EESIPR_TDEIP         0x00100000
#define CYGARC_REG_EESIPR_TFUFIP        0x00080000
#define CYGARC_REG_EESIPR_FRIP          0x00040000
#define CYGARC_REG_EESIPR_RDEIP         0x00020000
#define CYGARC_REG_EESIPR_RFOFIP        0x00010000
#define CYGARC_REG_EESIPR_ITFIP         0x00001000
#define CYGARC_REG_EESIPR_CNDIP         0x00000800
#define CYGARC_REG_EESIPR_DLCIP         0x00000400
#define CYGARC_REG_EESIPR_CDIP          0x00000200
#define CYGARC_REG_EESIPR_TROIP         0x00000100
#define CYGARC_REG_EESIPR_RMAFIP        0x00000080
#define CYGARC_REG_EESIPR_RRFIP         0x00000010
#define CYGARC_REG_EESIPR_RTLFIP        0x00000008
#define CYGARC_REG_EESIPR_RTSFIP        0x00000004
#define CYGARC_REG_EESIPR_PREIP         0x00000002
#define CYGARC_REG_EESIPR_CERFIP        0x00000001


#define CYGARC_REG_ECMR_PRCEF           0x00001000
#define CYGARC_REG_ECMR_MPDE            0x00000200
#define CYGARC_REG_ECMR_RE              0x00000040
#define CYGARC_REG_ECMR_TE              0x00000020
#define CYGARC_REG_ECMR_ILB             0x00000008
#define CYGARC_REG_ECMR_ELB             0x00000004
#define CYGARC_REG_ECMR_DM              0x00000002
#define CYGARC_REG_ECMR_PRM             0x00000001


#define CYGARC_REG_EDMR_DL64            0x00000020
#define CYGARC_REG_EDMR_DL32            0x00000010
#define CYGARC_REG_EDMR_DL16            0x00000000
#define CYGARC_REG_EDMR_SWR             0x00000001

#define CYGARC_REG_PSR_LMON             0x00000001

#define CYGARC_REG_EDTRR_TR             0x00000001

#define CYGARC_REG_EDRRR_RR             0x00000001
