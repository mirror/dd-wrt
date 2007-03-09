/**
 * @file IxEthMii.c
 *
 * @author Intel Corporation
 * @date
 *
 * @brief  MII control functions
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

#include "IxOsal.h"

#include "IxEthAcc.h"
#include "IxEthMii_p.h"

#ifdef __wince
#include "IxOsPrintf.h"
#endif

/* Array to store the phy IDs of the discovered phys */
PRIVATE UINT32 ixEthMiiPhyId[IXP425_ETH_ACC_MII_MAX_ADDR];

/*********************************************************
 *
 * Scan for PHYs on the MII bus. This function returns
 * an array of booleans, one for each PHY address.
 * If a PHY is found at a particular address, the
 * corresponding entry in the array is set to TRUE.
 *
 */

PUBLIC IX_STATUS
ixEthMiiPhyPresent(UINT32 phyId, BOOL *phyPresentPtr)
{
    UINT16 regval, regvalId1, regvalId2;

    /*Search for PHYs on the MII*/
    /*Search for existant phys on the MDIO bus*/

    if ((phyPresentPtr == NULL) || 
	(phyId > IXP425_ETH_ACC_MII_MAX_ADDR))
    {
	return IX_FAIL;
    }

    *phyPresentPtr = FALSE;

		if (phyId == 0)  /* Gateworks change to support DP83848 on GW2347 */
		{
		 ixEthAccMiiReadRtn(1, IX_ETH_MII_PHY_ID1_REG, &regvalId1);
	   ixEthAccMiiReadRtn(1, IX_ETH_MII_PHY_ID1_REG, &regvalId1);
		 ixEthAccMiiReadRtn(1, IX_ETH_MII_PHY_ID2_REG, &regvalId2);
     if ((regvalId1 == 0x2000) && (regvalId2 == 0x5c90)) /* National Phy */
	   	{
		  ixEthAccMiiReadRtn (1, 0x19, &regval); /* change Phy Address to 0 */
 		  ixEthAccMiiWriteRtn(1, 0x19, ((regval & 0xfffe) | 0x8000));

      /* 09/19/06 Gateworks change to force autonegotiate on GW2347 */		  
    
      ixEthAccMiiReadRtn (0, 0x00, &regval); /* read basic cmd reg */
      ixEthAccMiiWriteRtn(0, 0x00, (regval | 0x1000)); /* enable aneg */
      ixEthAccMiiWriteRtn(0, 0x00, (regval | 0x1200)); /* restart aneg */
    	}
     }

    ixEthMiiPhyId[phyId] = IX_ETH_MII_INVALID_PHY_ID;
    if(ixEthAccMiiReadRtn(phyId,
			  IX_ETH_MII_CTRL_REG,
			  &regval) == IX_ETH_ACC_SUCCESS)
    {
	if((regval & 0xffff) != 0xffff)
	{
	    /*Need to read the register twice here to flush PHY*/
	    ixEthAccMiiReadRtn(phyId,  IX_ETH_MII_PHY_ID1_REG, &regvalId1);
	    ixEthAccMiiReadRtn(phyId,  IX_ETH_MII_PHY_ID1_REG, &regvalId1);
	    ixEthAccMiiReadRtn(phyId,  IX_ETH_MII_PHY_ID2_REG, &regvalId2);
	    ixEthMiiPhyId[phyId] = (regvalId1 << IX_ETH_MII_REG_SHL) | regvalId2;
	    if    ((ixEthMiiPhyId[phyId] == IX_ETH_MII_KS8995_PHY_ID)
		      || (ixEthMiiPhyId[phyId] == IX_ETH_MII_LXT971_PHY_ID)
		      || (ixEthMiiPhyId[phyId] == IX_ETH_MII_LXT972_PHY_ID)
		      || (ixEthMiiPhyId[phyId] == IX_ETH_MII_LXT973_PHY_ID)
		      || (ixEthMiiPhyId[phyId] == IX_ETH_MII_LXT973A3_PHY_ID)
          || (ixEthMiiPhyId[phyId] == IX_ETH_MII_DP83848_PHY_ID)
	        || (ixEthMiiPhyId[phyId] == IX_ETH_MII_LXT9785_PHY_ID))
	    {
		/* supported phy */
		*phyPresentPtr = TRUE;
	    } /* end of if(ixEthMiiPhyId) */
	    else
	    {
#ifdef CYGPKG_HAL_ARM_XSCALE_IXDPG425
                if (phyId == 4 || phyId == 5)
		{
		    ixEthMiiPhyId[phyId] == 0xfffbfffb;
		    *phyPresentPtr = TRUE;
		    return IX_SUCCESS;
		}
#endif
		if (ixEthMiiPhyId[phyId] != IX_ETH_MII_INVALID_PHY_ID)
		{
		    /* unsupported phy */
		    ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
                               IX_OSAL_LOG_DEV_STDOUT,
			       "ixEthMiiPhyScan : unexpected Mii PHY%d ID %8.8x\n", 
			       phyId, ixEthMiiPhyId[phyId], 3, 4, 5, 6);
		    ixEthMiiPhyId[phyId] = IX_ETH_MII_UNKNOWN_PHY_ID;
		    *phyPresentPtr = TRUE;
		}
	    } 
	}
    }
    return IX_SUCCESS;
}

PUBLIC IX_STATUS
ixEthMiiPhyScan(BOOL phyPresent[], UINT32 maxPhyCount)
{
    UINT32 i;

    /*Search for PHYs on the MII*/
    /*Search for existant phys on the MDIO bus*/

    if ((phyPresent == NULL) || 
	(maxPhyCount > IXP425_ETH_ACC_MII_MAX_ADDR))
    {
	return IX_FAIL;
    }

    /* fill the array */
    for(i=0;
        i<IXP425_ETH_ACC_MII_MAX_ADDR;
	i++)
    {
	phyPresent[i] = FALSE;
    }

    /* iterate through the PHY addresses */
    for(i=0;
	maxPhyCount > 0 && i<IXP425_ETH_ACC_MII_MAX_ADDR;
	i++)
    {
	ixEthMiiPhyPresent(i, &phyPresent[i]);
	if (phyPresent[i])
	    maxPhyCount--;
    }
    return IX_SUCCESS;
}

/************************************************************
 *
 * Configure the PHY at the specified address
 *
 */
PUBLIC IX_STATUS
ixEthMiiPhyConfig(UINT32 phyAddr,
		  BOOL speed100,
		  BOOL fullDuplex,
		  BOOL autonegotiate)
{
    UINT16 regval=0;

    /* parameter check */
    if ((phyAddr < IXP425_ETH_ACC_MII_MAX_ADDR) &&
	(ixEthMiiPhyId[phyAddr] != IX_ETH_MII_INVALID_PHY_ID))
    {
    /*
     * set the control register
     */
	if(autonegotiate)
	{
	    regval |= IX_ETH_MII_CR_AUTO_EN | IX_ETH_MII_CR_RESTART;
	}
	else
	{
	    if(speed100)
	    {
		regval |= IX_ETH_MII_CR_100;
	    }
	    if(fullDuplex)
	    {
		regval |= IX_ETH_MII_CR_FDX;
	    }
	} /* end of if-else() */
	if (ixEthAccMiiWriteRtn(phyAddr, 
				IX_ETH_MII_CTRL_REG, 
				regval) == IX_ETH_ACC_SUCCESS)
	{
	    return IX_SUCCESS;
	}
    } /* end of if(phyAddr) */
    return IX_FAIL;
}

/******************************************************************
 *
 *  Enable the PHY Loopback at the specified address
 */
PUBLIC IX_STATUS
ixEthMiiPhyLoopbackEnable (UINT32 phyAddr)
{
  UINT16 regval ;  

  if ((phyAddr < IXP425_ETH_ACC_MII_MAX_ADDR) && 
      (IX_ETH_MII_INVALID_PHY_ID != ixEthMiiPhyId[phyAddr]))
  {
      /* read/write the control register */
      if(ixEthAccMiiReadRtn (phyAddr,
			     IX_ETH_MII_CTRL_REG, 
			     &regval) 
	 == IX_ETH_ACC_SUCCESS)
      {
	  if(ixEthAccMiiWriteRtn (phyAddr, 
				  IX_ETH_MII_CTRL_REG, 
				  regval | IX_ETH_MII_CR_LOOPBACK)
	     == IX_ETH_ACC_SUCCESS)
	  {
	      return IX_SUCCESS;
	  }
      }
  }
  return IX_FAIL;
}

/******************************************************************
 *
 *  Disable the PHY Loopback at the specified address
 */
PUBLIC IX_STATUS
ixEthMiiPhyLoopbackDisable (UINT32 phyAddr)
{
  UINT16 regval ;  

  if ((phyAddr < IXP425_ETH_ACC_MII_MAX_ADDR) && 
      (IX_ETH_MII_INVALID_PHY_ID != ixEthMiiPhyId[phyAddr]))
  {
      /* read/write the control register */
      if(ixEthAccMiiReadRtn (phyAddr,
			     IX_ETH_MII_CTRL_REG, 
			     &regval) 
	 == IX_ETH_ACC_SUCCESS)
      {
	  if(ixEthAccMiiWriteRtn (phyAddr, 
				  IX_ETH_MII_CTRL_REG, 
				  regval & (~IX_ETH_MII_CR_LOOPBACK))
	     == IX_ETH_ACC_SUCCESS)
	  {
	      return IX_SUCCESS;
	  }
      }
  }
  return IX_FAIL;
}

/******************************************************************
 *
 *  Reset the PHY at the specified address
 */
PUBLIC IX_STATUS
ixEthMiiPhyReset(UINT32 phyAddr)
{
    UINT32 timeout;
    UINT16 regval;

    if ((phyAddr < IXP425_ETH_ACC_MII_MAX_ADDR) &&
	(ixEthMiiPhyId[phyAddr] != IX_ETH_MII_INVALID_PHY_ID))
    {
	if ((ixEthMiiPhyId[phyAddr] == IX_ETH_MII_LXT971_PHY_ID)	||
	    (ixEthMiiPhyId[phyAddr] == IX_ETH_MII_LXT972_PHY_ID)	||
	    (ixEthMiiPhyId[phyAddr] == IX_ETH_MII_LXT973_PHY_ID)	||
	    (ixEthMiiPhyId[phyAddr] == IX_ETH_MII_LXT973A3_PHY_ID)	||
		(ixEthMiiPhyId[phyAddr] == IX_ETH_MII_LXT9785_PHY_ID)
	    )
	{
	    /* use the control register to reset the phy */
	    ixEthAccMiiWriteRtn(phyAddr, 
				IX_ETH_MII_CTRL_REG,
				IX_ETH_MII_CR_RESET);
	 
	    /* poll until the reset bit is cleared */
	    timeout = 0;
	    do
	    {
		ixOsalSleep (IX_ETH_MII_RESET_POLL_MS);

		/* read the control register and check for timeout */
		ixEthAccMiiReadRtn(phyAddr, 
				   IX_ETH_MII_CTRL_REG,
				   &regval);
		if ((regval & IX_ETH_MII_CR_RESET) == 0)
		{
		    /* timeout bit is self-cleared */
		    break;
		}
		timeout += IX_ETH_MII_RESET_POLL_MS;
	    }
	    while (timeout < IX_ETH_MII_RESET_DELAY_MS);

	    /* check for timeout */
	    if (timeout >= IX_ETH_MII_RESET_DELAY_MS)
	    {
		ixEthAccMiiWriteRtn(phyAddr, IX_ETH_MII_CTRL_REG,
				    IX_ETH_MII_CR_NORM_EN);
		return IX_FAIL;
	    }

	    return IX_SUCCESS;
	} /* end of if(ixEthMiiPhyId) */
	else if (ixEthMiiPhyId[phyAddr] == IX_ETH_MII_KS8995_PHY_ID)
	{
	    /* reset bit is reserved, just reset the control register */
	    ixEthAccMiiWriteRtn(phyAddr, IX_ETH_MII_CTRL_REG,IX_ETH_MII_CR_NORM_EN);
	    return IX_SUCCESS;
	}
	else
	{
	    /* unknown PHY, set the control register reset bit,
	     * wait 2 s. and clear the control register.
	     */
	    ixEthAccMiiWriteRtn(phyAddr, IX_ETH_MII_CTRL_REG,
				IX_ETH_MII_CR_RESET);
	    
	    ixOsalSleep (IX_ETH_MII_RESET_DELAY_MS);
	    
	    ixEthAccMiiWriteRtn(phyAddr, IX_ETH_MII_CTRL_REG,
				IX_ETH_MII_CR_NORM_EN);
	    return IX_SUCCESS;
	} /* end of if-else(ixEthMiiPhyId) */
    } /* end of if(phyAddr) */
    return IX_FAIL;
}

/*****************************************************************
 *
 *  Link state query functions
 */

PUBLIC IX_STATUS
ixEthMiiLinkStatus(UINT32 phyAddr,
           BOOL *linkUp,
           BOOL *speed100,
           BOOL *fullDuplex,
           BOOL *autoneg)
{
    UINT16 ctrlRegval, statRegval, regval, regval4, regval5;

    /* check the parameters */
    if ((linkUp == NULL) || 
	(speed100 == NULL) || 
	(fullDuplex == NULL) ||
	(autoneg == NULL))
    {
	return IX_FAIL;
    }

    *linkUp = FALSE;
    *speed100 = FALSE;
    *fullDuplex = FALSE;
    *autoneg = FALSE;

    if ((phyAddr < IXP425_ETH_ACC_MII_MAX_ADDR) &&
	(ixEthMiiPhyId[phyAddr] != IX_ETH_MII_INVALID_PHY_ID))
    {
	if ((ixEthMiiPhyId[phyAddr] == IX_ETH_MII_LXT971_PHY_ID)	||
	    (ixEthMiiPhyId[phyAddr] == IX_ETH_MII_LXT972_PHY_ID)	||
	    (ixEthMiiPhyId[phyAddr] == IX_ETH_MII_LXT9785_PHY_ID)
		)
	{
	    /* --------------------------------------------------*/
	    /* Retrieve information from PHY specific register   */
	    /* --------------------------------------------------*/
	    if (ixEthAccMiiReadRtn(phyAddr, 
				   IX_ETH_MII_STAT2_REG, 
				   &regval) != IX_ETH_ACC_SUCCESS)
	    {
		return IX_FAIL;
	    }
	    *linkUp = ((regval & IX_ETH_MII_SR2_LINK) != 0);
	    *speed100 = ((regval & IX_ETH_MII_SR2_100) != 0);
	    *fullDuplex = ((regval & IX_ETH_MII_SR2_FD) != 0);
	    *autoneg = ((regval & IX_ETH_MII_SR2_AUTO) != 0);
	    return IX_SUCCESS;
	} /* end of if(ixEthMiiPhyId) */
	else
	{    
	    /* ----------------------------------------------------*/
	    /* Retrieve information from status and ctrl registers */
	    /* ----------------------------------------------------*/
	    if (ixEthAccMiiReadRtn(phyAddr,  
				   IX_ETH_MII_CTRL_REG, 
				   &ctrlRegval) != IX_ETH_ACC_SUCCESS)
	    {
		return IX_FAIL;
	    }
	    ixEthAccMiiReadRtn(phyAddr,  IX_ETH_MII_STAT_REG, &statRegval);
	    
	    *linkUp = ((statRegval & IX_ETH_MII_SR_LINK_STATUS) != 0);
	    if (*linkUp)
	    {
		*autoneg = ((ctrlRegval & IX_ETH_MII_CR_AUTO_EN) != 0) &&
		    ((statRegval &  IX_ETH_MII_SR_AUTO_SEL) != 0) &&
		    ((statRegval & IX_ETH_MII_SR_AUTO_NEG) != 0);
		
		if (*autoneg)
		{
		    /* mask the current stat values with the capabilities */
		    ixEthAccMiiReadRtn(phyAddr, IX_ETH_MII_AN_ADS_REG, &regval4);
		    ixEthAccMiiReadRtn(phyAddr, IX_ETH_MII_AN_PRTN_REG, &regval5);
		    /* merge the flags from the 3 registers */
		    regval = (statRegval & ((regval4 & regval5) << 6));
		    /* initialise from status register values */
		    if ((regval & IX_ETH_MII_SR_TX_FULL_DPX) != 0)
		    {
			/* 100 Base X full dplx */
			*speed100 = TRUE;
			*fullDuplex = TRUE;
			return IX_SUCCESS;
		    }
		    if ((regval & IX_ETH_MII_SR_TX_HALF_DPX) != 0)
		    {
			/* 100 Base X half dplx */
			*speed100 = TRUE;
			return IX_SUCCESS;
		    }
		    if ((regval & IX_ETH_MII_SR_10T_FULL_DPX) != 0)
		    {
			/* 10 mb full dplx */
			*fullDuplex = TRUE;
			return IX_SUCCESS;
		    }
		    if ((regval & IX_ETH_MII_SR_10T_HALF_DPX) != 0)
		    {
			/* 10 mb half dplx */
			return IX_SUCCESS;
		    }
		} /* end of if(autoneg) */
		else
		{
		    /* autonegotiate not complete, return setup parameters */
		    *speed100 = ((ctrlRegval & IX_ETH_MII_CR_100) != 0);
		    *fullDuplex = ((ctrlRegval & IX_ETH_MII_CR_FDX) != 0);
		}
	    } /* end of if(linkUp) */
	} /* end of if-else(ixEthMiiPhyId) */
    } /* end of if(phyAddr) */
    else
    {
	return IX_FAIL;
    } /* end of if-else(phyAddr) */
    return IX_SUCCESS;
}

/*****************************************************************
 *
 *  Link state display functions
 */

PUBLIC IX_STATUS
ixEthMiiPhyShow (UINT32 phyAddr)
{
    BOOL linkUp, speed100, fullDuplex, autoneg;
    UINT16 cregval;
    UINT16 sregval;
    

    ixEthAccMiiReadRtn(phyAddr,  IX_ETH_MII_STAT_REG, &sregval);
    ixEthAccMiiReadRtn(phyAddr,  IX_ETH_MII_CTRL_REG, &cregval);

    /* get link information */
    if (ixEthMiiLinkStatus(phyAddr,
			   &linkUp,
			   &speed100,
			   &fullDuplex,
			   &autoneg) != IX_ETH_ACC_SUCCESS)
    {
	printf("PHY Status unknown\n");
	return IX_FAIL;
    }

    printf("PHY ID [phyAddr]: %8.8x\n",ixEthMiiPhyId[phyAddr]);
    printf( " Status reg:  %4.4x\n",sregval);
    printf( " control reg: %4.4x\n",cregval);
    /* display link information */
    printf("PHY Status:\n");
    printf("    Link is %s\n",
	   (linkUp ? "Up" : "Down"));
    if((sregval & IX_ETH_MII_SR_REMOTE_FAULT) != 0)
    {
	printf("    Remote fault detected\n");
    }
    printf("    Auto Negotiation %s\n",
	   (autoneg ? "Completed" : "Not Completed"));

    printf("PHY Configuration:\n");
    printf("    Speed %sMb/s\n",
	   (speed100 ? "100" : "10"));
    printf("    %s Duplex\n",
	   (fullDuplex ? "Full" : "Half"));
    printf("    Auto Negotiation %s\n",
	   (autoneg ? "Enabled" : "Disabled"));
    return IX_SUCCESS;
}

