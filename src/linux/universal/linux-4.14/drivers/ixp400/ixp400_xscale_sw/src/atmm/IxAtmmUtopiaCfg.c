/** 
 * @file IxAtmmUtopiaCfg.c
 * @author Intel Corporation
 * @date 6-MAR-2002
 *
 * @brief   
 * 
 * 
 * @par
 * IXP400 SW Release version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
 */

/*
 * Put the system defined include files required
 */

/*
 * Put the user defined include files required
 */
#include "IxOsal.h"
#include "IxAtmm.h"
#include "IxAtmdAccCtrl.h"
#include "IxAtmmUtopiaCfg_p.h"

/*
 * Typedefs.
 */

/*
 * #defines and macros used in this file.
 */

/*
 * Max number of loopback ports supported.
 */
#define IX_ATMM_MAX_LOOPBACK_PORTS   1

/*
 * Indicates the number of utopia ports to configure.
 */
#define IX_ATMM_NUM_PORTS_1 1
#define IX_ATMM_NUM_PORTS_2 2
#define IX_ATMM_NUM_PORTS_3 3
#define IX_ATMM_NUM_PORTS_4 4
#define IX_ATMM_NUM_PORTS_5 5
#define IX_ATMM_NUM_PORTS_6 6
#define IX_ATMM_NUM_PORTS_7 7
#define IX_ATMM_NUM_PORTS_8 8
#define IX_ATMM_NUM_PORTS_9 9
#define IX_ATMM_NUM_PORTS_10 10
#define IX_ATMM_NUM_PORTS_11 11 
#define IX_ATMM_NUM_PORTS_12 12

/*
 * defines for enabling, disabling or ignoring paramaters
 */
#define IX_ATMM_ENABLE  1
#define IX_ATMM_DISABLE 0
#define IX_ATMM_IGNORE  0

/*
 * ATM Master mode
 */
#define IX_ATMM_ATM_MASTER 0
/*
 * PHY Slave mode
 */
#define IX_ATMM_PHY_SLAVE 1

/*
 * Cell size supported
 */
#define IX_ATMM_RX_CELL_SIZE 53
#define IX_ATMM_TX_CELL_SIZE 52

/*
 * These defines specify the vci/vpi, port on which statistics will be gathered.
 * These are hard coded at compile time. Utopia statistics config cannot be reconfigured
 * at run time.
 */
#define IX_ATMM_UT_TX_STATS_CONFIG_PHY_ADDR 0
#define IX_ATMM_UT_TX_STATS_CONFIG_VPI      100
#define IX_ATMM_UT_TX_STATS_CONFIG_VCI      1
#define IX_ATMM_UT_TX_STATS_CONFIG_PTI      0
#define IX_ATMM_UT_TX_STATS_CONFIG_CLP      0

#define IX_ATMM_UT_RX_STATS_CONFIG_PHY_ADDR 0
#define IX_ATMM_UT_RX_STATS_CONFIG_VPI      100
#define IX_ATMM_UT_RX_STATS_CONFIG_VCI      1 
#define IX_ATMM_UT_RX_STATS_CONFIG_PTI      0
#define IX_ATMM_UT_RX_STATS_CONFIG_CLP      0

/*
 * Variable declarations global to this file only. Externs are followed by
 * static variables.
 */
static BOOL ixAtmmLoopbackOn = FALSE;

/*
 * Global variable for show purposes
 */
IxAtmdAccUtopiaConfig ixAtmmUtCfg;

/*
 * Static function prototypes
 */
PRIVATE 
BOOL phyCfgValidate (unsigned numPorts,
		     IxAtmmPhyMode phyMode,
		     IxAtmmPortCfg portCfgs[]);

PRIVATE
IX_STATUS utCfgParamsCheck (unsigned numPorts,
			    IxAtmmPhyMode phyMode,
			    IxAtmmPortCfg portCfgs[],
			    IxAtmmUtopiaLoopbackMode loopbackMode);

PRIVATE 
void utopiaConfigCreate (unsigned numPorts,
			 IxAtmmPhyMode phyMode,
			 IxAtmmPortCfg portCfgs[],			 
			 IxAtmdAccUtopiaConfig *utCfg,
			 IxAtmmUtopiaLoopbackMode loopbackMode);

PRIVATE 
void utopiaTxConfigCreate (unsigned numPorts,
			   IxAtmmPhyMode phyMode,
			   IxAtmmPortCfg portCfgs[],
			   IxAtmdAccUtopiaConfig *utCfg,
			   IxAtmmUtopiaLoopbackMode loopbackMode);

PRIVATE 
void utopiaRxConfigCreate (unsigned numPorts,
			   IxAtmmPhyMode phyMode,
			   IxAtmmPortCfg portCfgs[],
			   IxAtmdAccUtopiaConfig *utCfg,
			   IxAtmmUtopiaLoopbackMode loopbackMode);

PRIVATE 
void utopiaSysConfigCreate (IxAtmdAccUtopiaConfig *utCfg);

PRIVATE 
void utopiaRxTransTableSetup (unsigned numPorts,
			      IxAtmmPortCfg portCfgs[],
			      IxAtmdAccUtopiaConfig *utCfg);

PRIVATE 
void utopiaTxTransTableSetup (unsigned numPorts,
			      IxAtmmPortCfg portCfgs[],
			      IxAtmdAccUtopiaConfig *utCfg);

PRIVATE 
IX_STATUS utCfgShow (void);

PRIVATE 
IX_STATUS utStatusShow (void);



PRIVATE void
utopiaSysConfigReset (IxAtmdAccUtopiaConfig *utCfg);

IX_STATUS ixAtmdAccUtopiaConfigReset (const IxAtmdAccUtopiaConfig * utConfig);


/* ------------------------------------------------------
   Function definitions visable to this file only.
   ------------------------------------------------------ */
IX_STATUS
ixAtmmUtopiaCfgInit (unsigned numPorts,
		     IxAtmmPhyMode phyMode,
		     IxAtmmPortCfg portCfgs[],
		     IxAtmmUtopiaLoopbackMode loopbackMode)
{
    IX_STATUS retval;
    
    retval = utCfgParamsCheck (numPorts,
			       phyMode,
			       portCfgs,
			       loopbackMode);

    /* Loopback mode */
    if (retval == IX_SUCCESS)
    {
	if (loopbackMode == IX_ATMM_UTOPIA_LOOPBACK_ENABLED)
	{
	    ixAtmmLoopbackOn = TRUE;
	}

	/* Clear the config struct */
	ixOsalMemSet (&ixAtmmUtCfg, 0, sizeof (IxAtmdAccUtopiaConfig));
	        
	/* Fill out the utopia config structure */
	utopiaConfigCreate (numPorts,
			    phyMode,
			    portCfgs,
			    &ixAtmmUtCfg,
			    loopbackMode);
	
	/* Call IxAtmdAcc to write the config */
	retval = ixAtmdAccUtopiaConfigSet (&ixAtmmUtCfg);
    }
    
    return retval;
}



IX_STATUS
ixAtmmUtopiaCfgUninit (void)
{

    IX_STATUS retval;

    /* Fill out the utopia Sysconfig structure to reset*/
    utopiaSysConfigReset (&ixAtmmUtCfg);

    /* Call IxAtmdAcc to write the Config */
    retval = ixAtmdAccUtopiaConfigReset (&ixAtmmUtCfg);

    /* Clear the Config struct */
    ixOsalMemSet (&ixAtmmUtCfg, 0, sizeof (IxAtmdAccUtopiaConfig));
    return retval;
}


PUBLIC IX_STATUS
ixAtmmUtopiaStatusShow (void)
{
    IX_STATUS retval = IX_SUCCESS;

    if (IX_SUCCESS != utStatusShow ())
    {
	retval = IX_FAIL;
    }

    return retval;
}

PUBLIC IX_STATUS
ixAtmmUtopiaCfgShow (void)
{
    IX_STATUS retval = IX_SUCCESS;

    if ((IX_SUCCESS != utStatusShow ()) ||
	(IX_SUCCESS != utCfgShow ()))
    {
	retval = IX_FAIL;
    }

    return retval;
}

/* ------------------------------------------------------
   Function definitions visable to this file only.
   ------------------------------------------------------ */
PRIVATE
IX_STATUS utCfgParamsCheck (unsigned numPorts,			   
			    IxAtmmPhyMode phyMode,
			    IxAtmmPortCfg portCfgs[],
			    IxAtmmUtopiaLoopbackMode loopbackMode)
{
    /* Validate input parameters */
    if ((numPorts == 0) || (numPorts > IX_UTOPIA_MAX_PORTS))
    {
	return IX_ATMM_RET_INVALID_PORT;
    }

    if ((loopbackMode != IX_ATMM_UTOPIA_LOOPBACK_DISABLED) &&
	(loopbackMode != IX_ATMM_UTOPIA_LOOPBACK_ENABLED))
    {
	return IX_FAIL;
    }

    /* Loopback allowed on 1 port only */
    if ((loopbackMode == IX_ATMM_UTOPIA_LOOPBACK_ENABLED) &&
	(numPorts > IX_ATMM_MAX_LOOPBACK_PORTS))
    {
	return IX_FAIL;
    }

    if (!phyCfgValidate (numPorts,
			 phyMode,			 
			 portCfgs))
    {
	return IX_FAIL;
    }

    return IX_SUCCESS;
}    

PRIVATE BOOL
phyCfgValidate (unsigned numPorts,		
		IxAtmmPhyMode phyMode,
		IxAtmmPortCfg portCfgs[])
{
    /* MPHY mode */
    if (phyMode == IX_ATMM_MPHY_MODE)
    {
	UINT32 i;
	for (i=0; i<numPorts; i++)
	{
	    /* Validate the PHY address */
	    if ((portCfgs[i].UtopiaTxPhyAddr >= IX_ATMM_UTOPIA_SPHY_ADDR) ||
		(portCfgs[i].UtopiaRxPhyAddr >= IX_ATMM_UTOPIA_SPHY_ADDR))
	    {
		return FALSE;
	    }
	    /* N.B. not checking for PHY address uniqueness */
	}
        return TRUE;	
    }
    else if (phyMode == IX_ATMM_SPHY_MODE)
    {
	/* 
	 * Nothing to check for SPHY mode
	 * N.B: Phy address is ignored for SPHY mode
	 */
	
	return TRUE;
    }
    /* Any other value for phyMode is not allowed */
    return FALSE;
}

PRIVATE void
utopiaConfigCreate (unsigned numPorts,
		    IxAtmmPhyMode phyMode,
		    IxAtmmPortCfg portCfgs[],
		    IxAtmdAccUtopiaConfig *utCfg,
		    IxAtmmUtopiaLoopbackMode loopbackMode)
{
    utopiaTxConfigCreate (numPorts,
			  phyMode,
			  portCfgs,
			  utCfg,
			  loopbackMode);

    utopiaRxConfigCreate (numPorts,
			  phyMode,
			  portCfgs,
			  utCfg,
			  loopbackMode);

    utopiaSysConfigCreate (utCfg);
}

PRIVATE void 
utopiaRxConfigCreate (unsigned numPorts,
		      IxAtmmPhyMode phyMode,
		      IxAtmmPortCfg portCfgs[],
		      IxAtmdAccUtopiaConfig *utCfg,
		      IxAtmmUtopiaLoopbackMode loopbackMode)
{
    /* Rx Config */
    if (loopbackMode == IX_ATMM_UTOPIA_LOOPBACK_ENABLED)
    {
	utCfg->utRxConfig.rxInterface = IX_ATMM_PHY_SLAVE;
    }
    else
    {
	utCfg->utRxConfig.rxInterface = IX_ATMM_ATM_MASTER;
    }
    utCfg->utRxConfig.rxMode = phyMode;

    utCfg->utRxConfig.rxOctet        = IX_ATMM_DISABLE;
    utCfg->utRxConfig.rxParity       = IX_ATMM_DISABLE;
    utCfg->utRxConfig.rxEvenParity   = IX_ATMM_DISABLE;
    utCfg->utRxConfig.rxHEC          = IX_ATMM_ENABLE;
    utCfg->utRxConfig.rxCOSET        = IX_ATMM_ENABLE;
    utCfg->utRxConfig.rxHECpass      = IX_ATMM_DISABLE; 
    utCfg->utRxConfig.reserved_1     = IX_ATMM_DISABLE;
    utCfg->utRxConfig.rxCellSize     = IX_ATMM_RX_CELL_SIZE;
    utCfg->utRxConfig.rxHashEnbGFC   = IX_ATMM_DISABLE;
    utCfg->utRxConfig.rxPreHash      = IX_ATMM_ENABLE;
    utCfg->utRxConfig.reserved_2     = IX_ATMM_DISABLE;
    utCfg->utRxConfig.rxAddrRange    = numPorts - 1;
    utCfg->utRxConfig.reserved_3     = IX_ATMM_DISABLE;
    utCfg->utRxConfig.rxPHYAddr      = IX_ATMM_UT_RX_STATS_CONFIG_PHY_ADDR;

    /* Rx Stats config */
    utCfg->utRxStatsConfig.vpi = IX_ATMM_UT_RX_STATS_CONFIG_VPI;
    utCfg->utRxStatsConfig.vci = IX_ATMM_UT_RX_STATS_CONFIG_VCI;
    utCfg->utRxStatsConfig.pti = IX_ATMM_UT_TX_STATS_CONFIG_PTI;
    utCfg->utRxStatsConfig.clp = IX_ATMM_UT_TX_STATS_CONFIG_CLP;

    /* Rx Idle cell definition */
    utCfg->utRxDefineIdle.vpi = 0;
    utCfg->utRxDefineIdle.vci = 0;
    utCfg->utRxDefineIdle.pti = IX_ATMM_IGNORE;
    utCfg->utRxDefineIdle.clp = IX_ATMM_IGNORE;

    /* Rx Enable fields */
    utCfg->utRxEnableFields.defineRxIdleGFC   = IX_ATMM_IGNORE;
    utCfg->utRxEnableFields.defineRxIdlePTI   = IX_ATMM_IGNORE;
    utCfg->utRxEnableFields.defineRxIdleCLP   = IX_ATMM_IGNORE;
    utCfg->utRxEnableFields.phyStatsRxEnb     = IX_ATMM_DISABLE;
    utCfg->utRxEnableFields.vcStatsRxEnb      = IX_ATMM_DISABLE;
    utCfg->utRxEnableFields.vcStatsRxGFC      = IX_ATMM_IGNORE;
    utCfg->utRxEnableFields.vcStatsRxPTI      = IX_ATMM_IGNORE;
    utCfg->utRxEnableFields.vcStatsRxCLP      = IX_ATMM_IGNORE;
    utCfg->utRxEnableFields.discardHecErr     = IX_ATMM_ENABLE;
    utCfg->utRxEnableFields.discardParErr     = IX_ATMM_DISABLE;
    utCfg->utRxEnableFields.discardIdle       = IX_ATMM_ENABLE;
    utCfg->utRxEnableFields.enbHecErrCnt      = IX_ATMM_ENABLE;
    utCfg->utRxEnableFields.enbParErrCnt      = IX_ATMM_ENABLE;
    utCfg->utRxEnableFields.enbIdleCellCnt    = IX_ATMM_ENABLE;
    utCfg->utRxEnableFields.enbSizeErrCnt     = IX_ATMM_ENABLE;
    utCfg->utRxEnableFields.enbRxCellCnt      = IX_ATMM_ENABLE;
    utCfg->utRxEnableFields.reserved_1        = 0;
    utCfg->utRxEnableFields.rxCellOvrInt      = IX_ATMM_ENABLE;
    utCfg->utRxEnableFields.invalidHecOvrInt  = IX_ATMM_ENABLE;
    utCfg->utRxEnableFields.invalidParOvrInt  = IX_ATMM_ENABLE;
    utCfg->utRxEnableFields.invalidSizeOvrInt = IX_ATMM_ENABLE;
    utCfg->utRxEnableFields.rxIdleOvrInt      = IX_ATMM_ENABLE;
    utCfg->utRxEnableFields.reserved_2        = 0;
    utCfg->utRxEnableFields.rxAddrMask        = 0;
    
    utopiaRxTransTableSetup (numPorts, portCfgs, utCfg);
}

PRIVATE void
utopiaTxConfigCreate (unsigned numPorts,
		      IxAtmmPhyMode phyMode,
		      IxAtmmPortCfg portCfgs[],
		      IxAtmdAccUtopiaConfig *utCfg,
		      IxAtmmUtopiaLoopbackMode loopbackMode)
{
    /* Tx Config */
    utCfg->utTxConfig.reserved_1  = 0;
    utCfg->utTxConfig.txInterface = IX_ATMM_ATM_MASTER; /* ATM Master */
    utCfg->utTxConfig.txMode = phyMode;

    utCfg->utTxConfig.txOctet        = IX_ATMM_DISABLE; /* Cell level handshaking */
    utCfg->utTxConfig.txParity       = IX_ATMM_ENABLE;
    utCfg->utTxConfig.txEvenParity   = IX_ATMM_DISABLE;
    utCfg->utTxConfig.txHEC          = IX_ATMM_ENABLE;
    utCfg->utTxConfig.txCOSET        = IX_ATMM_ENABLE;
    utCfg->utTxConfig.reserved_2     = 0;
    utCfg->utTxConfig.txCellSize     = IX_ATMM_TX_CELL_SIZE;
    utCfg->utTxConfig.reserved_3     = 0;
    utCfg->utTxConfig.txAddrRange    = numPorts - 1;
    utCfg->utTxConfig.reserved_4     = 0;
    utCfg->utTxConfig.txPHYAddr      = IX_ATMM_UT_TX_STATS_CONFIG_PHY_ADDR;
    
    /* Tx Stats config */
    utCfg->utTxStatsConfig.vpi = IX_ATMM_UT_TX_STATS_CONFIG_VPI;
    utCfg->utTxStatsConfig.vci = IX_ATMM_UT_TX_STATS_CONFIG_VCI;
    utCfg->utTxStatsConfig.pti = IX_ATMM_UT_TX_STATS_CONFIG_PTI;
    utCfg->utTxStatsConfig.clp = IX_ATMM_UT_TX_STATS_CONFIG_CLP;

    /* Tx Idle cell */
    utCfg->utTxDefineIdle.vpi  = 0;
    utCfg->utTxDefineIdle.vci  = 0;
    utCfg->utTxDefineIdle.pti  = 0;
    utCfg->utTxDefineIdle.clp  = 0;
    
    /* Tx Enable fields */
    utCfg->utTxEnableFields.defineTxIdleGFC  = IX_ATMM_IGNORE;
    utCfg->utTxEnableFields.defineTxIdlePTI  = IX_ATMM_IGNORE;
    utCfg->utTxEnableFields.defineTxIdleCLP  = IX_ATMM_IGNORE;
    utCfg->utTxEnableFields.phyStatsTxEnb    = IX_ATMM_DISABLE;
    utCfg->utTxEnableFields.vcStatsTxEnb     = IX_ATMM_DISABLE;
    utCfg->utTxEnableFields.vcStatsTxGFC     = IX_ATMM_IGNORE;
    utCfg->utTxEnableFields.vcStatsTxPTI     = IX_ATMM_IGNORE;
    utCfg->utTxEnableFields.vcStatsTxCLP     = IX_ATMM_IGNORE;    
    utCfg->utTxEnableFields.reserved_1       = 0;
    utCfg->utTxEnableFields.txPollStsInt     = IX_ATMM_ENABLE;
    utCfg->utTxEnableFields.txCellOvrInt     = IX_ATMM_ENABLE;
    utCfg->utTxEnableFields.txIdleCellOvrInt = IX_ATMM_ENABLE;
    utCfg->utTxEnableFields.enbIdleCellCnt   = IX_ATMM_ENABLE;
    utCfg->utTxEnableFields.enbTxCellCnt     = IX_ATMM_ENABLE;
    utCfg->utTxEnableFields.reserved_2       = 0;

    utopiaTxTransTableSetup (numPorts, portCfgs, utCfg);
}

PRIVATE void
utopiaSysConfigCreate (IxAtmdAccUtopiaConfig *utCfg)
{
    /* Utopia system configuration */
    utCfg->utSysConfig.reserved_1  = 0;
    utCfg->utSysConfig.txEnbFSM = IX_ATMM_ENABLE;
    utCfg->utSysConfig.rxEnbFSM = IX_ATMM_ENABLE;
    if (ixAtmmLoopbackOn)
    {
	utCfg->utSysConfig.tstLoop = IX_ATMM_ENABLE;
	utCfg->utSysConfig.disablePins = IX_ATMM_ENABLE;
    }
    else
    {
	utCfg->utSysConfig.tstLoop = IX_ATMM_DISABLE;
	utCfg->utSysConfig.disablePins = IX_ATMM_DISABLE;
    }

    utCfg->utSysConfig.txReset     = IX_ATMM_DISABLE;
    utCfg->utSysConfig.rxReset     = IX_ATMM_DISABLE;
    utCfg->utSysConfig.reserved_2  = 0;
}


/* Reset the System Configuration Structure elements */
PRIVATE void
utopiaSysConfigReset (IxAtmdAccUtopiaConfig *utCfg)
{
    /* Utopia system configuration */
    utCfg->utSysConfig.reserved_1  = 0;
    utCfg->utSysConfig.txEnbFSM    = IX_ATMM_DISABLE;
    utCfg->utSysConfig.rxEnbFSM    = IX_ATMM_DISABLE;
    utCfg->utSysConfig.tstLoop     = IX_ATMM_DISABLE;
    utCfg->utSysConfig.disablePins = IX_ATMM_DISABLE;
    utCfg->utSysConfig.txReset     = IX_ATMM_DISABLE;
    utCfg->utSysConfig.rxReset     = IX_ATMM_DISABLE;
    utCfg->utSysConfig.reserved_2  = 0;
}


PRIVATE void
utopiaRxTransTableSetup (unsigned numPorts,
			 IxAtmmPortCfg portCfgs[],
			 IxAtmdAccUtopiaConfig *utCfg)
{
    /* Utopia Rx translation table */
    switch (numPorts)
    {
	/* 
	 * N.B: Each case will fall thru to the next. e.g. for case
	 * IX_ATMM_NUM_PORTS_4 phys 3,2,1,0 will be configured.
	 */
	case IX_ATMM_NUM_PORTS_12:
	    utCfg->utRxTransTable1.phy11     = portCfgs[11].UtopiaRxPhyAddr;
	case IX_ATMM_NUM_PORTS_11:
	    utCfg->utRxTransTable1.phy10     = portCfgs[10].UtopiaRxPhyAddr;
	case IX_ATMM_NUM_PORTS_10:
	    utCfg->utRxTransTable1.phy9      = portCfgs[9].UtopiaRxPhyAddr;
	case IX_ATMM_NUM_PORTS_9:
	    utCfg->utRxTransTable1.phy8      = portCfgs[8].UtopiaRxPhyAddr;
	case IX_ATMM_NUM_PORTS_8:
	    utCfg->utRxTransTable1.phy7      = portCfgs[7].UtopiaRxPhyAddr;
	case IX_ATMM_NUM_PORTS_7:
	    utCfg->utRxTransTable1.phy6      = portCfgs[6].UtopiaRxPhyAddr;
	case IX_ATMM_NUM_PORTS_6:
	    utCfg->utRxTransTable0.phy5      = portCfgs[5].UtopiaRxPhyAddr;
	case IX_ATMM_NUM_PORTS_5:
	    utCfg->utRxTransTable0.phy4      = portCfgs[4].UtopiaRxPhyAddr;
	case IX_ATMM_NUM_PORTS_4:
	    utCfg->utRxTransTable0.phy3      = portCfgs[3].UtopiaRxPhyAddr;
	case IX_ATMM_NUM_PORTS_3:
	    utCfg->utRxTransTable0.phy2      = portCfgs[2].UtopiaRxPhyAddr;
	case IX_ATMM_NUM_PORTS_2:
	    utCfg->utRxTransTable0.phy1      = portCfgs[1].UtopiaRxPhyAddr;
	case IX_ATMM_NUM_PORTS_1:
	    utCfg->utRxTransTable0.phy0      = portCfgs[0].UtopiaRxPhyAddr;
	    break;
	default:
	    /* Cant get here as numPorts is checked outside */
	    break;
    }
}

PRIVATE void
utopiaTxTransTableSetup (unsigned numPorts,
			 IxAtmmPortCfg portCfgs[],
			 IxAtmdAccUtopiaConfig *utCfg)
{
    /* Utopia Tx translation table */
    switch (numPorts)
    {
	/* 
	 * N.B: Each case will fall thru to the next. e.g. for case
	 * IX_ATMM_NUM_PORTS_4 phys 3,2,1,0 will be configured.
	 */
	case IX_ATMM_NUM_PORTS_12:
	    utCfg->utTxTransTable1.phy11     = portCfgs[11].UtopiaTxPhyAddr;
	case IX_ATMM_NUM_PORTS_11:
	    utCfg->utTxTransTable1.phy10     = portCfgs[10].UtopiaTxPhyAddr;
	case IX_ATMM_NUM_PORTS_10:
	    utCfg->utTxTransTable1.phy9      = portCfgs[9].UtopiaTxPhyAddr;
	case IX_ATMM_NUM_PORTS_9:
	    utCfg->utTxTransTable1.phy8      = portCfgs[8].UtopiaTxPhyAddr;
	case IX_ATMM_NUM_PORTS_8:
	    utCfg->utTxTransTable1.phy7      = portCfgs[7].UtopiaTxPhyAddr;
	case IX_ATMM_NUM_PORTS_7:
	    utCfg->utTxTransTable1.phy6      = portCfgs[6].UtopiaTxPhyAddr;
	case IX_ATMM_NUM_PORTS_6:
	    utCfg->utTxTransTable0.phy5      = portCfgs[5].UtopiaTxPhyAddr;
	case IX_ATMM_NUM_PORTS_5:
	    utCfg->utTxTransTable0.phy4      = portCfgs[4].UtopiaTxPhyAddr;
	case IX_ATMM_NUM_PORTS_4:
	    utCfg->utTxTransTable0.phy3      = portCfgs[3].UtopiaTxPhyAddr;
	case IX_ATMM_NUM_PORTS_3:
	    utCfg->utTxTransTable0.phy2      = portCfgs[2].UtopiaTxPhyAddr;
	case IX_ATMM_NUM_PORTS_2:
	    utCfg->utTxTransTable0.phy1      = portCfgs[1].UtopiaTxPhyAddr;
	case IX_ATMM_NUM_PORTS_1:
	    utCfg->utTxTransTable0.phy0      = portCfgs[0].UtopiaTxPhyAddr;
	    break;
	default:
	    /* Cant get here as numPorts is checked outside */
	    break;
    }
}

PRIVATE IX_STATUS
utCfgShow (void)
{
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "=================== Utopia Config ====================\n",0,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxConfig.rxInterface = %d\n", ixAtmmUtCfg.utRxConfig.rxInterface,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxConfig.rxMode = %d\n", ixAtmmUtCfg.utRxConfig.rxMode,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxConfig.rxOctet = %d\n", ixAtmmUtCfg.utRxConfig.rxOctet,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxConfig.rxParity = %d\n", ixAtmmUtCfg.utRxConfig.rxParity,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxConfig.rxEvenParity = %d\n", ixAtmmUtCfg.utRxConfig.rxEvenParity,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxConfig.rxHEC = %d\n", ixAtmmUtCfg.utRxConfig.rxHEC,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxConfig.rxCOSET = %d\n", ixAtmmUtCfg.utRxConfig.rxCOSET,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxConfig.rxHECpass = %d\n", ixAtmmUtCfg.utRxConfig.rxHECpass,0,0,0,0,0 );
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxConfig.reserved_1 = %d\n", ixAtmmUtCfg.utRxConfig.reserved_1,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxConfig.rxCellSize = %d\n", ixAtmmUtCfg.utRxConfig.rxCellSize,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxConfig.rxHashEnbGFC = %d\n", ixAtmmUtCfg.utRxConfig.rxHashEnbGFC,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxConfig.rxPreHash = %d\n", ixAtmmUtCfg.utRxConfig.rxPreHash,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxConfig.reserved_2 = %d\n", ixAtmmUtCfg.utRxConfig.reserved_2,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxConfig.rxAddrRange = %d\n", ixAtmmUtCfg.utRxConfig.rxAddrRange,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxConfig.reserved_3 = %d\n", ixAtmmUtCfg.utRxConfig.reserved_3,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxConfig.rxPHYAddr = %d\n", ixAtmmUtCfg.utRxConfig.rxPHYAddr,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxStatsConfig.vpi = %d\n", ixAtmmUtCfg.utRxStatsConfig.vpi,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxStatsConfig.vci = %d\n", ixAtmmUtCfg.utRxStatsConfig.vci,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxStatsConfig.pti = %d\n",ixAtmmUtCfg.utRxStatsConfig.pti,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxStatsConfig.clp = %d\n", ixAtmmUtCfg.utRxStatsConfig.clp,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxDefineIdle.vpi = %d\n", ixAtmmUtCfg.utRxDefineIdle.vpi,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxDefineIdle.vci = %d\n", ixAtmmUtCfg.utRxDefineIdle.vci,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxDefineIdle.pti = %d\n", ixAtmmUtCfg.utRxDefineIdle.pti,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxDefineIdle.clp = %d\n", ixAtmmUtCfg.utRxDefineIdle.clp,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxEnableFields.defineRxIdleGFC = %d\n", ixAtmmUtCfg.utRxEnableFields.defineRxIdleGFC,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxEnableFields.defineRxIdlePTI = %d\n", ixAtmmUtCfg.utRxEnableFields.defineRxIdlePTI,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxEnableFields.defineRxIdleCLP = %d\n", ixAtmmUtCfg.utRxEnableFields.defineRxIdleCLP,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxEnableFields.phyStatsRxEnb = %d\n", ixAtmmUtCfg.utRxEnableFields.phyStatsRxEnb,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxEnableFields.vcStatsRxEnb = %d\n", ixAtmmUtCfg.utRxEnableFields.vcStatsRxEnb,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxEnableFields.vcStatsRxGFC = %d\n", ixAtmmUtCfg.utRxEnableFields.vcStatsRxGFC,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxEnableFields.vcStatsRxPTI = %d\n", ixAtmmUtCfg.utRxEnableFields.vcStatsRxPTI,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxEnableFields.vcStatsRxCLP = %d\n", ixAtmmUtCfg.utRxEnableFields.vcStatsRxCLP,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxEnableFields.discardHecErr = %d\n", ixAtmmUtCfg.utRxEnableFields.discardHecErr,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxEnableFields.discardParErr = %d\n", ixAtmmUtCfg.utRxEnableFields.discardParErr,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxEnableFields.discardIdle = %d\n", ixAtmmUtCfg.utRxEnableFields.discardIdle,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxEnableFields.enbHecErrCnt = %d\n", ixAtmmUtCfg.utRxEnableFields.enbHecErrCnt,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxEnableFields.enbParErrCnt = %d\n", ixAtmmUtCfg.utRxEnableFields.enbParErrCnt,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxEnableFields.enbIdleCellCnt = %d\n", ixAtmmUtCfg.utRxEnableFields.enbIdleCellCnt,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxEnableFields.enbSizeErrCnt = %d\n", ixAtmmUtCfg.utRxEnableFields.enbSizeErrCnt,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxEnableFields.enbRxCellCnt = %d\n", ixAtmmUtCfg.utRxEnableFields.enbRxCellCnt,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxEnableFields.reserved_1 = %d\n", ixAtmmUtCfg.utRxEnableFields.reserved_1,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxEnableFields.rxCellOvrInt = %d\n", ixAtmmUtCfg.utRxEnableFields.rxCellOvrInt,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxEnableFields.invalidHecOvrInt = %d\n", ixAtmmUtCfg.utRxEnableFields.invalidHecOvrInt,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxEnableFields.invalidParOvrInt = %d\n", ixAtmmUtCfg.utRxEnableFields.invalidParOvrInt,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxEnableFields.invalidSizeOvrInt = %d\n", ixAtmmUtCfg.utRxEnableFields.invalidSizeOvrInt,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxEnableFields.rxIdleOvrInt = %d\n", ixAtmmUtCfg.utRxEnableFields.rxIdleOvrInt,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxEnableFields.reserved_2 = %d\n", ixAtmmUtCfg.utRxEnableFields.reserved_2,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxEnableFields.rxAddrMask = %d\n", ixAtmmUtCfg.utRxEnableFields.rxAddrMask,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxConfig.reserved_1 = %d\n", ixAtmmUtCfg.utTxConfig.reserved_1,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxConfig.txInterface = %d\n", ixAtmmUtCfg.utTxConfig.txInterface,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxConfig.txMode = %d\n", ixAtmmUtCfg.utTxConfig.txMode,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxConfig.txOctet = %d\n", ixAtmmUtCfg.utTxConfig.txOctet,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxConfig.txParity = %d\n", ixAtmmUtCfg.utTxConfig.txParity,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxConfig.txEvenParity = %d\n", ixAtmmUtCfg.utTxConfig.txEvenParity,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxConfig.txHEC = %d\n", ixAtmmUtCfg.utTxConfig.txHEC,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxConfig.txCOSET = %d\n", ixAtmmUtCfg.utTxConfig.txCOSET,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxConfig.reserved_2 = %d\n", ixAtmmUtCfg.utTxConfig.reserved_2,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxConfig.txCellSize = %d\n", ixAtmmUtCfg.utTxConfig.txCellSize,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxConfig.reserved_3 = %d\n", ixAtmmUtCfg.utTxConfig.reserved_3,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxConfig.txAddrRange = %d\n", ixAtmmUtCfg.utTxConfig.txAddrRange,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxConfig.reserved_4 = %d\n", ixAtmmUtCfg.utTxConfig.reserved_4,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxConfig.txPHYAddr = %d\n", ixAtmmUtCfg.utTxConfig.txPHYAddr,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxStatsConfig.vpi = %d\n", ixAtmmUtCfg.utTxStatsConfig.vpi,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxStatsConfig.vci = %d\n", ixAtmmUtCfg.utTxStatsConfig.vci,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxStatsConfig.pti = %d\n", ixAtmmUtCfg.utTxStatsConfig.pti,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxStatsConfig.clp = %d\n", ixAtmmUtCfg.utTxStatsConfig.clp,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxDefineIdle.vpi = %d\n", ixAtmmUtCfg.utTxDefineIdle.vpi,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxDefineIdle.vci = %d\n", ixAtmmUtCfg.utTxDefineIdle.vci,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxDefineIdle.pti = %d\n", ixAtmmUtCfg.utTxDefineIdle.pti,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxDefineIdle.clp = %d\n", ixAtmmUtCfg.utTxDefineIdle.clp,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxEnableFields.defineTxIdleGFC = %d\n", ixAtmmUtCfg.utTxEnableFields.defineTxIdleGFC,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxEnableFields.defineTxIdlePTI = %d\n", ixAtmmUtCfg.utTxEnableFields.defineTxIdlePTI,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxEnableFields.defineTxIdleCLP = %d\n", ixAtmmUtCfg.utTxEnableFields.defineTxIdleCLP,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxEnableFields.phyStatsTxEnb = %d\n", ixAtmmUtCfg.utTxEnableFields.phyStatsTxEnb,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxEnableFields.vcStatsTxEnb = %d\n", ixAtmmUtCfg.utTxEnableFields.vcStatsTxEnb,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxEnableFields.vcStatsTxGFC = %d\n", ixAtmmUtCfg.utTxEnableFields.vcStatsTxGFC,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxEnableFields.vcStatsTxPTI = %d\n", ixAtmmUtCfg.utTxEnableFields.vcStatsTxPTI,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxEnableFields.vcStatsTxCLP = %d\n", ixAtmmUtCfg.utTxEnableFields.vcStatsTxCLP,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxEnableFields.reserved_1 = %d\n", ixAtmmUtCfg.utTxEnableFields.reserved_1,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxEnableFields.txPollStsInt = %d\n", ixAtmmUtCfg.utTxEnableFields.txPollStsInt,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxEnableFields.txCellOvrInt = %d\n", ixAtmmUtCfg.utTxEnableFields.txCellOvrInt,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxEnableFields.txIdleCellOvrInt = %d\n", ixAtmmUtCfg.utTxEnableFields.txIdleCellOvrInt,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxEnableFields.enbIdleCellCnt = %d\n", ixAtmmUtCfg.utTxEnableFields.enbIdleCellCnt,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxEnableFields.enbTxCellCnt = %d\n", ixAtmmUtCfg.utTxEnableFields.enbTxCellCnt,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxTransTable1.phy11= %d\n", ixAtmmUtCfg.utTxTransTable1.phy11,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxTransTable1.phy10= %d\n", ixAtmmUtCfg.utTxTransTable1.phy10,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxTransTable1.phy9 = %d\n", ixAtmmUtCfg.utTxTransTable1.phy9,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxTransTable1.phy8 = %d\n", ixAtmmUtCfg.utTxTransTable1.phy8,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxTransTable1.phy7 = %d\n", ixAtmmUtCfg.utTxTransTable1.phy7,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxTransTable1.phy6 = %d\n", ixAtmmUtCfg.utTxTransTable1.phy6,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxTransTable0.phy5 = %d\n", ixAtmmUtCfg.utTxTransTable0.phy5,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxTransTable0.phy4 = %d\n", ixAtmmUtCfg.utTxTransTable0.phy4,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxTransTable0.phy3 = %d\n", ixAtmmUtCfg.utTxTransTable0.phy3,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxTransTable0.phy2 = %d\n", ixAtmmUtCfg.utTxTransTable0.phy2,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxTransTable0.phy1 = %d\n", ixAtmmUtCfg.utTxTransTable0.phy1,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxTransTable0.phy0 = %d\n", ixAtmmUtCfg.utTxTransTable0.phy0,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxTransTable1.phy11= %d\n", ixAtmmUtCfg.utRxTransTable1.phy11,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxTransTable1.phy10= %d\n", ixAtmmUtCfg.utRxTransTable1.phy10,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxTransTable1.phy9 = %d\n", ixAtmmUtCfg.utRxTransTable1.phy9,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxTransTable1.phy8 = %d\n", ixAtmmUtCfg.utRxTransTable1.phy8,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxTransTable1.phy7 = %d\n", ixAtmmUtCfg.utRxTransTable1.phy7,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxTransTable1.phy6 = %d\n", ixAtmmUtCfg.utRxTransTable1.phy6,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxTransTable0.phy5 = %d\n", ixAtmmUtCfg.utRxTransTable0.phy5,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxTransTable0.phy4 = %d\n", ixAtmmUtCfg.utRxTransTable0.phy4,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxTransTable0.phy3 = %d\n", ixAtmmUtCfg.utRxTransTable0.phy3,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxTransTable0.phy2 = %d\n", ixAtmmUtCfg.utRxTransTable0.phy2,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxTransTable0.phy1 = %d\n", ixAtmmUtCfg.utRxTransTable0.phy1,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxTransTable0.phy0 = %d\n", ixAtmmUtCfg.utRxTransTable0.phy0,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utSysConfig.reserved_1 = %d\n", ixAtmmUtCfg.utSysConfig.reserved_1,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utSysConfig.txEnableFSM = %d\n", ixAtmmUtCfg.utSysConfig.txEnbFSM,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utSysConfig.rxEnableFSM = %d\n", ixAtmmUtCfg.utSysConfig.rxEnbFSM,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utSysConfig.disablePins = %d\n", ixAtmmUtCfg.utSysConfig.disablePins,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utSysConfig.tstLoop = %d\n", ixAtmmUtCfg.utSysConfig.tstLoop,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utSysConfig.txReset = %d\n", ixAtmmUtCfg.utSysConfig.txReset,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utSysConfig.rxReset = %d\n", ixAtmmUtCfg.utSysConfig.rxReset,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utSysConfig.reserved_2 = %d\n", ixAtmmUtCfg.utSysConfig.reserved_2,0,0,0,0,0);
    return IX_SUCCESS;
}

PRIVATE IX_STATUS
utStatusShow (void)
{
    IxAtmdAccUtopiaStatus utStat;
    IX_STATUS retval;

    retval = ixAtmdAccUtopiaStatusGet (&utStat);

    if (retval == IX_SUCCESS)
    {
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "=================== Utopia Status ====================\n",0,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxCellCount = %d\n", utStat.utTxCellCount,0,0,0,0,0);               
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxIdleCellCount = %d\n", utStat.utTxIdleCellCount,0,0,0,0,0);           
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxCellConditionStatus.reserved_1 = %d\n", utStat.utTxCellConditionStatus.reserved_1,0,0,0,0,0);           
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxCellConditionStatus.txFIFO2Underflow = %d\n", utStat.utTxCellConditionStatus.txFIFO2Underflow,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxCellConditionStatus.txFIFO1Underflow = %d\n", utStat.utTxCellConditionStatus.txFIFO1Underflow,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxCellConditionStatus.txFIFO2Overflow = %d\n", utStat.utTxCellConditionStatus.txFIFO2Overflow,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxCellConditionStatus.txFIFO1Overflow = %d\n", utStat.utTxCellConditionStatus.txFIFO1Overflow,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxCellConditionStatus.txIdleCellCountOvr = %d\n", utStat.utTxCellConditionStatus.txIdleCellCountOvr,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxCellConditionStatus.txCellCountOvr = %d\n", utStat.utTxCellConditionStatus.txCellCountOvr,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utTxCellConditionStatus.reserved_2 = %d\n", utStat.utTxCellConditionStatus.reserved_2,0,0,0,0,0);    
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxCellCount = %d\n", utStat.utRxCellCount,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxIdleCellCount = %d\n", utStat.utRxIdleCellCount,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxInvalidHECount = %d\n", utStat.utRxInvalidHECount,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxInvalidParCount = %d\n", utStat.utRxInvalidParCount,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxInvalidSizeCount = %d\n", utStat.utRxInvalidSizeCount,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxCellConditionStatus.reserved_1 = %d\n", utStat.utRxCellConditionStatus.reserved_1,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxCellConditionStatus.rxCellCountOvr = %d\n", utStat.utRxCellConditionStatus.rxCellCountOvr,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxCellConditionStatus.invalidHecCountOvr = %d\n", utStat.utRxCellConditionStatus.invalidHecCountOvr,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxCellConditionStatus.invalidParCountOvr = %d\n", utStat.utRxCellConditionStatus.invalidParCountOvr,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxCellConditionStatus.invalidSizeCountOvr = %d\n", utStat.utRxCellConditionStatus.invalidSizeCountOvr,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxCellConditionStatus.rxIdleCountOvr = %d\n", utStat.utRxCellConditionStatus.rxIdleCountOvr,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxCellConditionStatus.reserved_2 = %d\n", utStat.utRxCellConditionStatus.reserved_2,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxCellConditionStatus.rxFIFO2Underflow = %d\n", utStat.utRxCellConditionStatus.rxFIFO2Underflow,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxCellConditionStatus.rxFIFO1Underflow = %d\n", utStat.utRxCellConditionStatus.rxFIFO1Underflow,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxCellConditionStatus.rxFIFO2Overflow = %d\n", utStat.utRxCellConditionStatus.rxFIFO2Overflow,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxCellConditionStatus.rxFIFO1Overflow = %d\n", utStat.utRxCellConditionStatus.rxFIFO1Overflow,0,0,0,0,0);
	ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,
		   "utRxCellConditionStatus.reserved_3 = %d\n", utStat.utRxCellConditionStatus.reserved_3,0,0,0,0,0);
    }

    return retval;
}
