/******************************************************************************
 *
 * Name:    fwptrn.h
 * Project: firmware common modules
 * Version: $Revision: #4 $
 * Date:    $Date: 2010/11/04 $
 * Purpose: firmware pattern function
 *
 ******************************************************************************/

/******************************************************************************
 *
 *      LICENSE:
 *      (C)Copyright Marvell.
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      The information in this file is provided "AS IS" without warranty.
 *      /LICENSE
 *
 ******************************************************************************/

#ifndef __SK_FWPTRN_H__
#define __SK_FWPTRN_H__

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/*   Yukon EC, etc... */
#define ASF_YEC_YTB_BASE_WOL_CTRL1          ((SK_U32)0x0f20)                    /*  YTB WOL CTRL register link 1 */
#define ASF_YEC_PATTRAM_CLUSTER_BYTES       ((SK_U8)4)    /*  4 bytes is a word */
#define ASF_YEC_PATTRAM_CLUSTER_WORDS       ((SK_U8)4)    /*  4 words in a cluster */
#define ASF_YEC_PATTRAM_CLUSTER_SIZE        ((SK_U8)64)   /*  pattern ram has 64 cluster */

#define ASF_YEC_PATTERN_ENA1                (ASF_YEC_YTB_BASE_WOL_CTRL1 + 0x02)     /*  enable pattern register, width:8 */
#define ASF_YEC_PATTERN_LENGTH_R1_L         (ASF_YEC_YTB_BASE_WOL_CTRL1 + 0x10)     /*  pattern length register, pattern 0-3, width: 4x8 */
#define ASF_YEC_PATTERN_LENGTH_R1_H         (ASF_YEC_YTB_BASE_WOL_CTRL1 + 0x14)     /*  pattern length register, pattern 4-6, width: 3x8 */
#define ASF_YEC_PATTERN_MATCHENA1           (ASF_YEC_YTB_BASE_WOL_CTRL1 + 0x0b)     /*  ASF/PME match enable register, width: 8 */
#define ASF_YEC_PATTERN_CTRL1               (ASF_YEC_YTB_BASE_WOL_CTRL1 + 0x00)     /*  match result, match control, wol ctrl and status */

#define ASF_YEC_YTB_BASE_MACRXFIFO1         ((SK_U32)0x0c40)                        /*  base of receive MAC fifo registers, port 1 */
#define ASF_YEC_MAC_FIFO_CTRL1              (ASF_YEC_YTB_BASE_MACRXFIFO1 + 0x08)    /*  control/test Rx MAC, link1, 32 bit */
#define ASF_YEC_MAC_FIFO_FLUSHMASK1         (ASF_YEC_YTB_BASE_MACRXFIFO1 + 0x0c)    /*  flush mask register Rx MAC, link1, 32 bit */
#define ASF_YEC_MAC_FIFO_FLUSHTHRES1        (ASF_YEC_YTB_BASE_MACRXFIFO1 + 0x10)    /*  Rx MAC FIFO Flush Threshold, link1, 32 bit */

#define ASF_YLCI_MACRXFIFOTHRES             8                                       /*  mac rx threshold in qwords */

#define ASF_PATTERN_ID_RSP					0
#define ASF_PATTERN_ID_ARP					1
#define ASF_PATTERN_ID_RMCP					2

/*   Yukon Extreme */
#define ASF_YEX_YTB_BASE_WOL_CTRL1          ((SK_U32)0x0f20)                    /*  YTB WOL CTRL register link 1 */
#define ASF_YEX_PATTRAM_CLUSTER_BYTES       ((SK_U8)4)    /*  4 bytes is a word */
#define ASF_YEX_PATTRAM_CLUSTER_WORDS       ((SK_U8)4)    /*  4 words in a cluster */
#define ASF_YEX_PATTRAM_CLUSTER_SIZE        ((SK_U8)64)   /*  pattern ram has 64 cluster */

#define ASF_YEX_PATTERN_ENA1                (ASF_YEX_YTB_BASE_WOL_CTRL1 + 0x02)     /*  enable pattern register, width:8 */
#define ASF_YEX_PATTERN_LENGTH_R1_L         (ASF_YEX_YTB_BASE_WOL_CTRL1 + 0x10)     /*  pattern length register, pattern 0-3, width: 4x8 */
#define ASF_YEX_PATTERN_LENGTH_R1_H         (ASF_YEX_YTB_BASE_WOL_CTRL1 + 0x14)     /*  pattern length register, pattern 4-6, width: 3x8 */
#define ASF_YEX_PATTERN_LENGTH_R1_EH        (ASF_YEX_YTB_BASE_WOL_CTRL1 + 0x18)     /*  pattern length register, pattern 7-8, width: 3x8 */
#define ASF_YEX_PATTERN_MATCHENA1           (ASF_YEX_YTB_BASE_WOL_CTRL1 + 0x0c)     /*  ASF/PME match enable register, width: 8 */
#define ASF_YEX_PATTERN_CTRL1               (ASF_YEX_YTB_BASE_WOL_CTRL1 + 0x00)     /*  match result, match control, wol ctrl and status */

/*   Yukon Supreme */
#define ASF_DASH_PATTERN_NUM_TCP 	13
#define ASF_DASH_PATTERN_NUM_UDP	12
#define ASF_DASH_PATTERN_NUM_ICMP	11
#define ASF_DASH_PATTERN_NUM_ARP	10

extern void FwCheckPattern(SK_AC *pAC, SK_IOC IoC, SK_U8 port );
extern void FwSetUpPattern(SK_AC *pAC, SK_IOC IoC, SK_U8 port );
extern SK_I8 FwEnablePattern (SK_AC *pAC, SK_IOC IoC, SK_U8 port, SK_U8 pattno);
extern SK_I8 FwDisablePattern (SK_AC *pAC, SK_IOC IoC, SK_U8 port, SK_U8 pattno);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* __SK_FWPTRN_H__ */

