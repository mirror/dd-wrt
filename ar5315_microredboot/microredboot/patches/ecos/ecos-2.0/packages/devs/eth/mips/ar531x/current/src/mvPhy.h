//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Atheros Communications, Inc.
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
// Alternative licenses for eCos may be arranged by contacting the copyright
// holders.
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Atheros Communications, Inc.
// Contributors: Atheros Engineering
// Date:         2003-10-22
// Purpose:      
// Description:  AR531X ethernet hardware driver
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================
/*
 * mvPhy.h - definitions for the ethernet PHY -- Marvell 88E6060
 * All definitions in this file are operating system independent!
 */

#ifndef MVPHY_H
#define MVPHY_H

/*****************/
/* PHY Registers */
/*****************/
#define MV_PHY_CONTROL                 0
#define MV_PHY_STATUS                  1
#define MV_PHY_ID1                     2
#define MV_PHY_ID2                     3
#define MV_AUTONEG_ADVERT              4
#define MV_LINK_PARTNER_ABILITY        5
#define MV_AUTONEG_EXPANSION           6
#define MV_NEXT_PAGE_TRANSMIT          7
#define MV_LINK_PARTNER_NEXT_PAGE      8
#define MV_PHY_SPECIFIC_CONTROL_1     16
#define MV_PHY_SPECIFIC_STATUS        17
#define MV_PHY_INTERRUPT_ENABLE       18
#define MV_PHY_INTERRUPT_STATUS       19
#define MV_PHY_INTERRUPT_PORT_SUMMARY 20
#define MV_RECEIVE_ERROR_COUNTER      21
#define MV_LED_PARALLEL_SELECT        22
#define MV_LED_STREAM_SELECT_LEDS     23
#define MV_PHY_LED_CONTROL            24
#define MV_PHY_MANUAL_LED_OVERRIDE    25
#define MV_VCT_CONTROL                26
#define MV_VCT_STATUS                 27
#define MV_PHY_SPECIFIC_CONTROL_2     28

/* MV_PHY_CONTROL fields */
#define MV_CTRL_SOFTWARE_RESET                    0x8000
#define MV_CTRL_AUTONEGOTIATION_ENABLE            0x1000

/* MV_PHY_ID1 fields */
#define MV_PHY_ID1_EXPECTATION                    0x0141 /* OUI >> 6 */

/* MV_PHY_ID2 fields */
#define MV_OUI_LSB_MASK                           0xfc00
#define MV_OUI_LSB_EXPECTATION                    0x0c00
#define MV_OUI_LSB_SHIFT                              10
#define MV_MODEL_NUM_MASK                         0x03f0
#define MV_MODEL_NUM_SHIFT                             4
#define MV_REV_NUM_MASK                           0x000f
#define MV_REV_NUM_SHIFT                               0

/* MV_PHY_SPECIFIC_STATUS fields */
#define MV_STATUS_RESOLVED_SPEED_100              0x4000
#define MV_STATUS_RESOLVED_DUPLEX_FULL            0x2000
#define MV_STATUS_RESOLVED                        0x0800
#define MV_STATUS_REAL_TIME_LINK_UP               0x0400

/* Check if autonegotiation is complete and link is up */
#define MV_AUTONEG_DONE(mv_phy_specific_status)                   \
    (((mv_phy_specific_status) &                                  \
        (MV_STATUS_RESOLVED | MV_STATUS_REAL_TIME_LINK_UP)) ==    \
        (MV_STATUS_RESOLVED | MV_STATUS_REAL_TIME_LINK_UP))


/*************************/
/* Switch Port Registers */
/*************************/
#define MV_PORT_STATUS                 0
#define MV_SWITCH_ID                   3
#define MV_PORT_CONTROL                4
#define MV_PORT_BASED_VLAN_MAP         6
#define MV_PORT_ASSOCIATION_VECTOR    11
#define MV_RX_COUNTER                 16
#define MV_TX_COUNTER                 17

/* MV_SWITCH_ID fields */
#define MV_SWITCH_ID_DEV_MASK                       0xfff0
#define MV_SWITCH_ID_DEV_EXPECTATION                0x0600
#define MV_SWITCH_ID_DEV_SHIFT                           4
#define MV_SWITCH_ID_REV_MASK                       0x000f
#define MV_SWITCH_ID_REV_SHIFT                           0

/* MV_PORT_CONTROL fields */
#define MV_PORT_CONTROL_PORT_STATE_MASK             0x0003
#define MV_PORT_CONTROL_PORT_STATE_DISABLED         0x0000
#define MV_PORT_CONTROL_PORT_STATE_FORWARDING       0x0003

#define MV_PORT_CONTROL_EGRESS_MODE                 0x0100 /* Receive */
#define MV_PORT_CONTROL_INGRESS_TRAILER             0x4000 /* Transmit */

#define MV_EGRESS_TRAILER_VALID                       0x80
#define MV_INGRESS_TRAILER_OVERRIDE                   0x80

#define MV_PHY_TRAILER_SIZE                              4


/***************************/
/* Switch Global Registers */
/***************************/
#define MV_SWITCH_GLOBAL_STATUS        0
#define MV_SWITCH_MAC_ADDR0            1
#define MV_SWITCH_MAC_ADDR2            2
#define MV_SWITCH_MAC_ADDR4            3
#define MV_SWITCH_GLOBAL_CONTROL       4
#define MV_ATU_CONTROL                10
#define MV_ATU_OPERATION              11
#define MV_ATU_DATA                   12
#define MV_ATU_MAC_ADDR0              13
#define MV_ATU_MAC_ADDR2              14
#define MV_ATU_MAC_ADDR4              15

/* MV_SWITCH_GLOBAL_STATUS fields */
#define MV_SWITCH_STATUS_READY_MASK  0x0800

/* MV_SWITCH_GLOBAL_CONTROL fields */
#define MV_CTRMODE_MASK              0x0100
#define MV_CTRMODE_GOODFRAMES        0x0000
#define MV_CTRMODE_BADFRAMES         0x0100

/* MV_ATU_CONTROL fields */
#define MV_ATUCTRL_ATU_SIZE_MASK     0x3000
#define MV_ATUCTRL_ATU_SIZE_SHIFT        12
#define MV_ATUCTRL_ATU_SIZE_DEFAULT       2 /* 1024 entry database */
#define MV_ATUCTRL_AGE_TIME_MASK     0x0ff0
#define MV_ATUCTRL_AGE_TIME_SHIFT         4
#define MV_ATUCTRL_AGE_TIME_DEFAULT      19 /* 19 * 16 = 304 seconds */

/* MV_ATU_OPERATION fields */
#define MV_ATU_BUSY_MASK             0x8000
#define MV_ATU_IS_BUSY               0x8000
#define MV_ATU_IS_FREE               0x0000
#define MV_ATU_OP_MASK               0x7000
#define MV_ATU_OP_FLUSH_ALL          0x1000
#define MV_ATU_OP_GET_NEXT           0x4000

/* MV_ATU_DATA fields */
#define MV_ENTRYPRI_MASK             0xc000
#define MV_ENTRYPRI_SHIFT                14
#define MV_PORTVEC_MASK              0x03f0
#define MV_PORTVEC_SHIFT                  4
#define MV_ENTRYSTATE_MASK           0x000f
#define MV_ENTRYSTATE_SHIFT               0

/* PHY Address for the switch itself */
#define MV_SWITCH_GLOBAL_ADDR 0x1f

BOOL    mv_phySetup(int ethUnit, UINT32 phyBase);
void    mv_phyCheckStatusChange(int ethUnit);
BOOL    mv_phyIsSpeed100(int ethUnit);
BOOL    mv_phyIsFullDuplex(int ethUnit);

#endif /* MVPHY_H */
