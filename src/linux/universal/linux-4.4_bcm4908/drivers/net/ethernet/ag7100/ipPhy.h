/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright © 2003 Atheros Communications, Inc.,  All Rights Reserved.
 */

/*
 * icPhy.h - definitions for the ethernet PHY.
 * This code supports a simple 1-port ethernet phy, ICPLUS,
 * All definitions in this file are operating system independent!
 */

#ifndef IPPHY_H
#define IPPHY_H

/*****************/
/* PHY Registers */
/*****************/
#define IP_PHY_CONTROL                 0
#define IP_PHY_STATUS                  1
#define IP_PHY_ID1                     2
#define IP_PHY_ID2                     3
#define IP_AUTONEG_ADVERT              4
#define IP_LINK_PARTNER_ABILITY        5
#define IP_AUTONEG_EXPANSION           6


/* IP_PHY_CONTROL fields */
#define IP_CTRL_SOFTWARE_RESET                    0x8000
#define IP_CTRL_SPEED_100                         0x2000
#define IP_CTRL_AUTONEGOTIATION_ENABLE            0x1000
#define IP_CTRL_START_AUTONEGOTIATION             0x0200
#define IP_CTRL_SPEED_FULL_DUPLEX                 0x0100

/* Phy status fields */
#define IP_STATUS_AUTO_NEG_DONE                   0x0020
#define IP_STATUS_LINK_PASS                       0x0004

#define IP_AUTONEG_DONE(ip_phy_status)                   \
    (((ip_phy_status) &                                  \
        (IP_STATUS_AUTO_NEG_DONE)) ==                    \
        (IP_STATUS_AUTO_NEG_DONE))

/* ICPLUS_PHY_ID1 fields */
#define IP_PHY_ID1_EXPECTATION                    0x0243 /* OUI >> 6 */

/* ICPLUS_PHY_ID2 fields */
#define IP_OUI_LSB_MASK                           0xfc00
#define IP_OUI_LSB_EXPECTATION                    0x0c00
#define IP_OUI_LSB_SHIFT                              10
#define IP_MODEL_NUM_MASK                         0x03f0
#define IP_MODEL_NUM_SHIFT                             4
#define IP_REV_NUM_MASK                           0x000f
#define IP_REV_NUM_SHIFT                               0

/* Link Partner ability */
#define IP_LINK_100BASETX_FULL_DUPLEX       0x0100
#define IP_LINK_100BASETX                   0x0080
#define IP_LINK_10BASETX_FULL_DUPLEX        0x0040
#define IP_LINK_10BASETX                    0x0020

/* Advertisement register. */
#define IP_ADVERTISE_100FULL                0x0100
#define IP_ADVERTISE_100HALF                0x0080  
#define IP_ADVERTISE_10FULL                 0x0040  
#define IP_ADVERTISE_10HALF                 0x0020  

#define IP_ADVERTISE_ALL (IP_ADVERTISE_10HALF | IP_ADVERTISE_10FULL | \
                       IP_ADVERTISE_100HALF | IP_ADVERTISE_100FULL)
               
               
#define IP_VLAN_TAG_VALID                   0x81
#define IP_VLAN_TAG_SIZE                    4
#define IP_VLAN_TAG_OFFSET                  12   /* After DA & SA */
#define IP_SPECIAL_TAG_VALID                0x81
              
/****************************/
/* Global Control Registers */
/****************************/
/* IP Global register doesn't have names based on functionality
 * hence has to live with this names  for now */
#define IP_GLOBAL_PHY29_18_REG  18
#define IP_GLOBAL_PHY29_19_REG  19
#define IP_GLOBAL_PHY29_20_REG  20
#define IP_GLOBAL_PHY29_21_REG  21
#define IP_GLOBAL_PHY29_22_REG  22
#define IP_GLOBAL_PHY29_23_REG  23
#define IP_GLOBAL_PHY29_24_REG  24
#define IP_GLOBAL_PHY29_25_REG  25
#define IP_GLOBAL_PHY29_26_REG  26
#define IP_GLOBAL_PHY29_27_REG  27
#define IP_GLOBAL_PHY29_28_REG  28
#define IP_GLOBAL_PHY29_29_REG  29
#define IP_GLOBAL_PHY29_30_REG  30
#define IP_GLOBAL_PHY29_31_REG  31


#define IP_GLOBAL_PHY30_0_REG   0
#define IP_GLOBAL_PHY30_1_REG   1
#define IP_GLOBAL_PHY30_2_REG   2
#define IP_GLOBAL_PHY30_3_REG   3
#define IP_GLOBAL_PHY30_4_REG   4
#define IP_GLOBAL_PHY30_5_REG   5
#define IP_GLOBAL_PHY30_6_REG   6
#define IP_GLOBAL_PHY30_7_REG   7
#define IP_GLOBAL_PHY30_8_REG   8
#define IP_GLOBAL_PHY30_9_REG   9
#define IP_GLOBAL_PHY30_10_REG  10
#define IP_GLOBAL_PHY30_11_REG  11
#define IP_GLOBAL_PHY30_12_REG  12
#define IP_GLOBAL_PHY30_13_REG  13
#define IP_GLOBAL_PHY30_16_REG  16
#define IP_GLOBAL_PHY30_17_REG  17
#define IP_GLOBAL_PHY30_18_REG  18
#define IP_GLOBAL_PHY30_20_REG  20
#define IP_GLOBAL_PHY30_21_REG  21
#define IP_GLOBAL_PHY30_22_REG  22
#define IP_GLOBAL_PHY30_23_REG  23
#define IP_GLOBAL_PHY30_24_REG  24
#define IP_GLOBAL_PHY30_25_REG  25
#define IP_GLOBAL_PHY30_26_REG  26
#define IP_GLOBAL_PHY30_27_REG  27
#define IP_GLOBAL_PHY30_28_REG  28
#define IP_GLOBAL_PHY30_29_REG  29
#define IP_GLOBAL_PHY30_30_REG  30
#define IP_GLOBAL_PHY30_31_REG  31

#define IP_GLOBAL_PHY31_0_REG   0
#define IP_GLOBAL_PHY31_1_REG   1
#define IP_GLOBAL_PHY31_2_REG   2
#define IP_GLOBAL_PHY31_3_REG   3
#define IP_GLOBAL_PHY31_4_REG   4
#define IP_GLOBAL_PHY31_5_REG   5
#define IP_GLOBAL_PHY31_6_REG   6

#define IP_GLOBAL_PHY29_31_REG  31


#define IP_VLAN0_OUTPUT_PORT_MASK_S     0
#define IP_VLAN1_OUTPUT_PORT_MASK_S     8
#define IP_VLAN2_OUTPUT_PORT_MASK_S     0
#define IP_VLAN3_OUTPUT_PORT_MASK_S     8

/* Masks and shifts for 29.23 register */
#define IP_PORTX_ADD_TAG_S               11
#define IP_PORTX_REMOVE_TAG_S            6       
#define IP_PORT5_ADD_TAG_S               1
#define IP_PORT5_REMOVE_TAG_S            0

/* 
 * 30.9   Definitions 
 */
#define TAG_VLAN_ENABLE         0x0080
#define VID_INDX_SEL_M          0x0070            
#define VID_INDX_SEL_S          4            

                  
/* PHY Addresses */
#define IP_PHY0_ADDR    0
#define IP_PHY1_ADDR    1
#define IP_PHY2_ADDR    2
#define IP_PHY3_ADDR    3
#define IP_PHY4_ADDR    4

#define IP_GLOBAL_PHY29_ADDR    29
#define IP_GLOBAL_PHY30_ADDR    30
#define IP_GLOBAL_PHY31_ADDR    31


#endif
