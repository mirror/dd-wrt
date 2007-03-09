/**
 * @file IxEthAccMii_p.h
 *
 * @author Intel Corporation
 * @date 
 *
 * @brief  MII Header file
 *
 * Design Notes:
 *
 * 
 * @par
 * IXP400 SW Release version  2.0
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright 2002-2005 Intel Corporation All Rights Reserved.
 * 
 * @par
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel Corporation
 * or its suppliers or licensors.  Title to the Material remains with
 * Intel Corporation or its suppliers and licensors.
 * 
 * @par
 * The Material is protected by worldwide copyright and trade secret laws
 * and treaty provisions. No part of the Material may be used, copied,
 * reproduced, modified, published, uploaded, posted, transmitted,
 * distributed, or disclosed in any way except in accordance with the
 * applicable license agreement .
 * 
 * @par
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel, except in accordance with the
 * applicable license agreement.
 * 
 * @par
 * Unless otherwise agreed by Intel in writing, you may not remove or
 * alter this notice or any other notice embedded in Materials by Intel
 * or Intel's suppliers or licensors in any way.
 * 
 * @par
 * For further details, please see the file README.TXT distributed with
 * this software.
 * 
 * @par
 * -- End Intel Copyright Notice --
 */

#ifndef IxEthAccMii_p_H
#define IxEthAccMii_p_H

/* MII definitions - these have been verified against the LXT971 and LXT972 PHYs*/

#define IXP425_ETH_ACC_MII_MAX_REG      32      /* max register per phy */

#define IX_ETH_ACC_MII_REG_SHL    16
#define IX_ETH_ACC_MII_ADDR_SHL   21

/* Definitions for MII access routines*/
 
#define IX_ETH_ACC_MII_GO                  BIT(31)
#define IX_ETH_ACC_MII_WRITE               BIT(26)
#define IX_ETH_ACC_MII_TIMEOUT_10TH_SECS        5    
#define IX_ETH_ACC_MII_10TH_SEC_IN_MILLIS     100
#define IX_ETH_ACC_MII_READ_FAIL           BIT(31)
 
#define IX_ETH_ACC_MII_PHY_DEF_DELAY   300  /* max delay before link up, etc. */
#define IX_ETH_ACC_MII_PHY_NO_DELAY    0x0  /* do not delay */
#define IX_ETH_ACC_MII_PHY_NULL        0xff /* PHY is not present */
#define IX_ETH_ACC_MII_PHY_DEF_ADDR    0x0  /* default PHY's logical address */

#ifndef IX_ETH_ACC_MII_MONITOR_DELAY
#   define IX_ETH_ACC_MII_MONITOR_DELAY   0x5    /* in seconds */
#endif

/* Register definition */  

#define IX_ETH_ACC_MII_CTRL_REG	    0x0	/* Control Register */
#define IX_ETH_ACC_MII_STAT_REG	    0x1	/* Status Register */
#define IX_ETH_ACC_MII_PHY_ID1_REG  0x2	/* PHY identifier 1 Register */
#define IX_ETH_ACC_MII_PHY_ID2_REG  0x3	/* PHY identifier 2 Register */
#define IX_ETH_ACC_MII_AN_ADS_REG   0x4	/* Auto-Negotiation 	  */
					/* Advertisement Register */
#define IX_ETH_ACC_MII_AN_PRTN_REG  0x5	/* Auto-Negotiation 	    */
					/* partner ability Register */
#define IX_ETH_ACC_MII_AN_EXP_REG   0x6	/* Auto-Negotiation   */
					/* Expansion Register */
#define IX_ETH_ACC_MII_AN_NEXT_REG  0x7	/* Auto-Negotiation 	       */
					/* next-page transmit Register */

IxEthAccStatus ixEthAccMdioShow (void);
IxEthAccStatus ixEthAccMiiInit(void);
void ixEthAccMiiUnload(void);

#endif  /*IxEthAccMii_p_H*/
