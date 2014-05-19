/**
 * @file IxHssAccService.c
 *
 * @author Intel Corporation
 * @date 30-Jan-02
 *
 * @brief HssAccess Service Interface
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

/**
 * Put the system defined include files required.
 */

/**
 * Put the user defined include files required.
 */

#include "IxHssAccError_p.h"
#include "IxOsal.h"
#include "IxHssAcc.h"
#include "IxFeatureCtrl.h"
#include "IxNpeDl.h"

#include "IxHssAccCommon_p.h"
#include "IxHssAccPktTx_p.h"
#include "IxHssAccPktRx_p.h"
#include "IxHssAccPCM_p.h" 
#include "IxHssAccCCM_p.h"
#include "IxHssAccChanRx_p.h"
#include "IxAccCommon.h"

/**
 * #defines and macros used in this file.
 */

/* the following defines are used to error check parameters */
#define IX_HSSACC_CHAN_RXBYTES_PER_TS_TRIG_DIV 8  /* trigger divider */
#define IX_HSSACC_CHAN_RXBYTES_PER_TS_TRIG_MIN 8  /* trigger minimum */
#define IX_HSSACC_CHAN_RXBYTES_PER_TS_MIN      16 /* minimum */
#define IX_HSSACC_PKT_MAX_TX_RX_BLOCK_SIZEW    16382 /* Applies to both packetised services */
#define IX_HSSACC_PKT_MAX_FRMFLAGSTART         2
#define IX_HSSACC_NUM_QMQS_PER_HSS             11
#define IX_HSSACC_MAX_NUM_QMQS                 (IX_HSSACC_NUM_QMQS_PER_HSS *\
                                                IX_HSSACC_HSS_PORT_MAX)
#define IX_HSSACC_CHANNELIZED                  1 


/**
 * Typedefs whose scope is limited to this file.
 */

typedef struct
{
    char *qName;
    IxQMgrQId qId;
    IxQMgrQSizeInWords qSizeInWords;
    IxQMgrQEntrySizeInWords qEntrySizeInWords;
} IxHssAccQConfigInfo;

/**
 * Variable declarations global to this file only.  Externs are followed by
 * static variables.
 */

/* this value will be set after successfully configuring a HSS port */
UINT32 ixHssAccServiceType = 0;
static BOOL ixHssAccHssInitialised[IX_HSSACC_HSS_PORT_MAX] = { FALSE, FALSE };
static UINT32 hssQmqsMax = IX_HSSACC_NUM_QMQS_PER_HSS; /* assume only HSS port 0 is required */
static IxHssAccQConfigInfo ixHssAccQConfigInfo[IX_HSSACC_MAX_NUM_QMQS] = {
    { "HSS0 Chl Rx Trig", IX_NPE_A_QMQ_HSS0_CHL_RX_TRIG , 32, 1 },
    { "HSS0 Pkt Rx"     , IX_NPE_A_QMQ_HSS0_PKT_RX      , 32, 1 },
    { "HSS0 Pkt Tx0"    , IX_NPE_A_QMQ_HSS0_PKT_TX0     , 16, 1 },
    { "HSS0 Pkt Tx1"    , IX_NPE_A_QMQ_HSS0_PKT_TX1     , 16, 1 },
    { "HSS0 Pkt Tx2"    , IX_NPE_A_QMQ_HSS0_PKT_TX2     , 16, 1 },
    { "HSS0 Pkt Tx3"    , IX_NPE_A_QMQ_HSS0_PKT_TX3     , 16, 1 },
    { "HSS0 Pkt Rx F0"  , IX_NPE_A_QMQ_HSS0_PKT_RX_FREE0, 16, 1 },
    { "HSS0 Pkt Rx F1"  , IX_NPE_A_QMQ_HSS0_PKT_RX_FREE1, 16, 1 },
    { "HSS0 Pkt Rx F2"  , IX_NPE_A_QMQ_HSS0_PKT_RX_FREE2, 16, 1 },
    { "HSS0 Pkt Rx F3"  , IX_NPE_A_QMQ_HSS0_PKT_RX_FREE3, 16, 1 },
    { "HSS0 Pkt TxDone" , IX_NPE_A_QMQ_HSS0_PKT_TX_DONE , 64, 1 },
    { "HSS1 Chl Rx Trig", IX_NPE_A_QMQ_HSS1_CHL_RX_TRIG , 32, 1 },
    { "HSS1 Pkt Rx"     , IX_NPE_A_QMQ_HSS1_PKT_RX      , 32, 1 },
    { "HSS1 Pkt Tx0"    , IX_NPE_A_QMQ_HSS1_PKT_TX0     , 16, 1 },
    { "HSS1 Pkt Tx1"    , IX_NPE_A_QMQ_HSS1_PKT_TX1     , 16, 1 },
    { "HSS1 Pkt Tx2"    , IX_NPE_A_QMQ_HSS1_PKT_TX2     , 16, 1 },
    { "HSS1 Pkt Tx3"    , IX_NPE_A_QMQ_HSS1_PKT_TX3     , 16, 1 },
    { "HSS1 Pkt Rx F0"  , IX_NPE_A_QMQ_HSS1_PKT_RX_FREE0, 16, 1 },
    { "HSS1 Pkt Rx F1"  , IX_NPE_A_QMQ_HSS1_PKT_RX_FREE1, 16, 1 },
    { "HSS1 Pkt Rx F2"  , IX_NPE_A_QMQ_HSS1_PKT_RX_FREE2, 16, 1 },
    { "HSS1 Pkt Rx F3"  , IX_NPE_A_QMQ_HSS1_PKT_RX_FREE3, 16, 1 },
    { "HSS1 Pkt TxDone" , IX_NPE_A_QMQ_HSS1_PKT_TX_DONE , 64, 1 }
};



/* This variable is made global to re-configure the Queues */
static BOOL ixHssAccQmqsConfigured = FALSE;

/*
 * This variable is the common mutex that is shared between
 * Ethernet & HssAcc services when the co-exist
 */
IxOsalMutex ixEthHssCoexistLock;

/*
 * This variable is used to indicate that Mutex initialization
 * is done or not
 */
BOOL ixEthHssComMutexInitDone=FALSE;

/*
 * This variable is used to indicate presence of Ethernet & HssAcc
 * services co existence
 */
BOOL ixEthHssAccCoexistEnable = FALSE;


/**
 * Extern function prototypes.
 */

/**
 * Static function prototypes.
 */

/* Used to check if image downloaded contains Ethernet HssAcc co-exist images */
IX_STATUS ixEthHssAccCoExistCheck(void);


IX_STATUS
ixEthHssAccCoExistCheck(void)
{

    UINT8 functionalityId;

    if (IX_SUCCESS != ixNpeDlLoadedImageFunctionalityGet(IX_NPEDL_NPEID_NPEA, &functionalityId))
    {
        return IX_FAIL;
    }


    /*
     * To enable Ethernet & HSS co-existence feature in NPE A, 
     * check the functionality of downloaded
     * NPE image when the functionality id is either 0x00900000
     * (HSS channelized + learning/filtering support) or
     * 0x00910000(HSS channelized + header conversion support)
     */

    if ((functionalityId == IX_FUNCTIONID_FROM_NPEIMAGEID_GET(
         IX_NPEDL_NPEIMAGE_NPEA_ETH_MACFILTERLEARN_HSSCHAN_COEXIST) ||
         functionalityId == IX_FUNCTIONID_FROM_NPEIMAGEID_GET(
         IX_NPEDL_NPEIMAGE_NPEA_ETH_HDRCONV_HSSCHAN_COEXIST)) &&
         ixEthHssAccCoexistEnable != TRUE)
    {
        ixEthHssAccCoexistEnable = TRUE;
    }

    return IX_SUCCESS;

}


/**
 * Function definition: ixHssAccPortInit
 */
IX_STATUS 
ixHssAccPortInit (IxHssAccHssPort hssPortId, 
		  IxHssAccConfigParams *configParams, 
		  IxHssAccTdmSlotUsage *tdmMap, 
		  IxHssAccLastErrorCallback lastHssErrorCallback)
{
    IX_STATUS status = IX_SUCCESS;
    BOOL tdmMapInvalid = FALSE;
    int i;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering ixHssAccPortInit\n");
 
    /* Error check the parameters */
    for (i = 0; i < IX_HSSACC_TSLOTS_PER_HSS_PORT; i++)
    {
	if (tdmMap[i] >= IX_HSSACC_TDMMAP_MAX)
	{
	    tdmMapInvalid = TRUE;
	    break;
	}	    
    }
    if (tdmMapInvalid ||
	IX_HSSACC_ENUM_INVALID (hssPortId, hssPortMax) ||
	(lastHssErrorCallback == NULL) ||
	(configParams->hssPktChannelCount > IX_HSSACC_HDLC_PORT_MAX) ||
	configParams->numChannelised > IX_HSSACC_MAX_CHAN_TIMESLOTS)
    {
	/* report the error */
	IX_HSSACC_REPORT_ERROR ("ixHssAccPortInit - invalid parameter\n");
	/* return error */
	status = IX_HSSACC_PARAM_ERR;

    }
    else
    {
	status = ixHssAccComPortInit (hssPortId, configParams, tdmMap, 
				      lastHssErrorCallback);
	if (status == IX_SUCCESS)
	{
	    ixHssAccHssInitialised[hssPortId] = TRUE;
	}
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting ixHssAccPortInit\n");
    return status;
}


/**
 * Function definition: ixHssAccLastErrorRetrievalInitiate
 */
IX_STATUS 
ixHssAccLastErrorRetrievalInitiate (IxHssAccHssPort hssPortId)
{
    IX_STATUS status = IX_SUCCESS;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccLastErrorRetrievalInitiate\n");

    /* Error check the parameters */
    if (IX_HSSACC_ENUM_INVALID (hssPortId, hssPortMax))
    {
	/* report the error */
	IX_HSSACC_REPORT_ERROR ("ixHssAccLastErrorRetrievalInitiate - invalid parameter\n");
	/* return error */
	status = IX_HSSACC_PARAM_ERR;
    }
    else if (ixHssAccHssInitialised[hssPortId] == FALSE)
    {
	/* report the error */
	IX_HSSACC_REPORT_ERROR ("ixHssAccLastErrorRetrievalInitiate - hss port not initialised\n");
	/* return error */
	status = IX_FAIL;
    }
    else
    {
	/* initiate the retrieval - the response will be asynchronous */
	status = ixHssAccComLastHssErrGet (hssPortId);
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting ixHssAccLastErrorRetrievalInitiate\n");
    return status;
}

/**
 * Function definition: ixHssAccInit
 */
IX_STATUS 
ixHssAccInit (void)
{
    unsigned index1 = 0;
    static BOOL qmqsConfigured = FALSE;
    IX_STATUS status = IX_SUCCESS;
    UINT8 imageId;

    qmqsConfigured = ixHssAccQmqsConfigured;


    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccInit\n");

    /* Check for HSS & HDLC port being present before proceeding*/
    if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_HSS)== 
        IX_FEATURE_CTRL_COMPONENT_DISABLED)
    {
        IX_HSSACC_REPORT_ERROR("Warning: the HSS Port component you"
            " specified does not exist\n");
        return IX_FAIL;
    }
    
    if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_HDLC)== 
        IX_FEATURE_CTRL_COMPONENT_DISABLED)
    {
        IX_HSSACC_REPORT_ERROR("Warning: the HDLC Port component you"
            " specified does not exist\n");
        return IX_FAIL; 
    }
    
 
    if (IX_SUCCESS != ixNpeDlLoadedImageFunctionalityGet (IX_NPEDL_NPEID_NPEA, 
							  &imageId))
    {
	/* if image ID is not available, meaning no HSS image has been downloaded */
	IX_HSSACC_REPORT_ERROR("ixHssAccInit: NPE downloader did not provide the image ID"
			       " for NPE-A. No HSS image has been downloaded\n");
	status = IX_FAIL;
    }
    else
    {
	if ((imageId == IX_FUNCTIONID_FROM_NPEIMAGEID_GET(IX_NPEDL_NPEIMAGE_NPEA_HSS0_ATM_SPHY_1_PORT))
           || (imageId == IX_FUNCTIONID_FROM_NPEIMAGEID_GET(IX_NPEDL_NPEIMAGE_NPEA_HSS_TSLOT_SWITCH )))
	{
	    /* Enabling HSS port 0 only */
	    hssPortMax = IX_HSSACC_SINGLE_HSS_PORT;
	    hssQmqsMax = IX_HSSACC_NUM_QMQS_PER_HSS;
	}
	else if ((imageId == IX_FUNCTIONID_FROM_NPEIMAGEID_GET(IX_NPEDL_NPEIMAGE_NPEA_HSS0_ATM_MPHY_4_PORT))
	   || (imageId == IX_FUNCTIONID_FROM_NPEIMAGEID_GET(IX_NPEDL_NPEIMAGE_NPEA_ETH_MACFILTERLEARN_HSSCHAN_COEXIST))
           || (imageId == IX_FUNCTIONID_FROM_NPEIMAGEID_GET(IX_NPEDL_NPEIMAGE_NPEA_ETH_HDRCONV_HSSCHAN_COEXIST)))
	{
	    /* Enabling HSS port 0 only */
	    hssPortMax = IX_HSSACC_SINGLE_HSS_PORT;
	    hssQmqsMax = 1;	/*Configure only to set Chann Rx Trig queue only */	
	}
	else if (imageId == IX_FUNCTIONID_FROM_NPEIMAGEID_GET(IX_NPEDL_NPEIMAGE_NPEA_HSS_2_PORT )) 
	{
            if (IX_FEATURE_CTRL_DEVICE_TYPE_IXP43X == ixFeatureCtrlDeviceRead ())
	    {
	        IX_HSSACC_REPORT_ERROR("ixHssAccInit:This image is not supported in IXP43X"
	        " as it supports only single HSS port(HSS port 0)\n");
                status = IX_FAIL;

	    }
	    else
	    {	
	        /* Enabling dual HSS ports */
	        hssPortMax = IX_HSSACC_DUAL_HSS_PORTS;
	        hssQmqsMax = IX_HSSACC_MAX_NUM_QMQS;
	    }
	}
	else
	{
            /* Error: HSS NPE image not available */
	    IX_HSSACC_REPORT_ERROR("ixHssAccInit: HSS NPE image does not exist in NPE-A\n");
	    status = IX_FAIL;
	}
    }

    if (status == IX_SUCCESS)
    {
	if (!qmqsConfigured)
	{
	    /* QMQ's configure */
	    do
	    {
		status = ixQMgrQConfig (ixHssAccQConfigInfo[index1].qName,
					ixHssAccQConfigInfo[index1].qId,
					ixHssAccQConfigInfo[index1].qSizeInWords,
					ixHssAccQConfigInfo[index1].qEntrySizeInWords);
		index1++;
	    } while ((status == IX_SUCCESS) && index1 < hssQmqsMax);

	    if (status == IX_SUCCESS)
	    {
		qmqsConfigured = TRUE;

            ixHssAccQmqsConfigured = qmqsConfigured;

	    }
	    else
	    {
		status = IX_FAIL;
	    }
	}
    }


    /* 
     * Check if the image downloaded is Ethernet HssAcc coexist channelized images 
     */
    if ( (imageId == IX_FUNCTIONID_FROM_NPEIMAGEID_GET(IX_NPEDL_NPEIMAGE_NPEA_ETH_MACFILTERLEARN_HSSCHAN_COEXIST)) 
      || (imageId == IX_FUNCTIONID_FROM_NPEIMAGEID_GET(IX_NPEDL_NPEIMAGE_NPEA_ETH_HDRCONV_HSSCHAN_COEXIST))
      || (imageId == IX_FUNCTIONID_FROM_NPEIMAGEID_GET(IX_NPEDL_NPEIMAGE_NPEA_HSS0_ATM_MPHY_4_PORT))) 
    {
        ixHssAccServiceType = IX_HSSACC_CHANNELIZED;
    }


/* If ETH-HSS co-exist feature exists we are initializing the common mutex.
 * Care should be taken as to whether this mutex is not already initialized by 
 * ixEthAccInit(). 
 */

if ( IX_SUCCESS == ixEthHssAccCoExistCheck())
{
    
    if ( TRUE == ixEthHssAccCoexistEnable )
    {

        if (FALSE == ixEthHssComMutexInitDone )    
        {
            printf("Initializing common mutex for ETH-HSS coexist \n"); 
            if (IX_SUCCESS != ixOsalMutexInit (&ixEthHssCoexistLock))
            {
                /* Log error message in debugging mode */
                ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            		   IX_OSAL_LOG_DEV_STDERR,
                       "ETH-HSS common mutex initialization failed.\n",
                       0, 0, 0, 0, 0, 0);

                /* Mutex initialization failed */
                return IX_FAIL;
            } /* end of if (ixOsalMutexInit) */
            ixEthHssComMutexInitDone = TRUE; 
        }

    }


}

/* Allocate the buffers based on if image is channelized image  or
   packetized image only for ETH-HSS co-exist images.
*/
if (ixHssAccServiceType != IX_HSSACC_CHANNELIZED) 
{

    if (status == IX_SUCCESS)
    {
	/* intialise the PCM - this should be called before the RxInit and TxInit */
	status = ixHssAccPCMInit ();
    }

    if (status == IX_SUCCESS)
    { 
	/* initialise the PktTx module */
	status = ixHssAccPktTxInit ();
    }

    if (status == IX_SUCCESS)
    {
	/* initialise the PktRx module */
	status = ixHssAccPktRxInit ();
    }



}

    if (status == IX_SUCCESS)
    { 
	/* initialise the Common module */
	status = ixHssAccComInit ();
    }

    if (status == IX_SUCCESS)
    { 
	/* initialise the CCM module */
	status = ixHssAccCCMInit ();
    }

    if (status == IX_SUCCESS)
    { 
	/* initialise the ChanRx module */
	status = ixHssAccChanRxInit ();
    }

    if (status != IX_SUCCESS)
    { 
	IX_HSSACC_REPORT_ERROR ("ixHssAccInit: failed to init\n");
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting ixHssAccInit\n");
    return status;
}


/**
 * Function definition: ixHssAccUninit
 */
PUBLIC IX_STATUS
ixHssAccUninit (void)
{
    IX_STATUS status = IX_SUCCESS;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT,
              "Entering ixHssAccUninit\n");

    if (IX_SUCCESS == status)
    {
    /* uninitialise the ChanRx module */
        status = ixHssAccChanRxUninit ();
    }

    if (IX_SUCCESS == status)
    {
    /* uninitialise the Common module */
        status = ixHssAccComUninit ();
    }

/*    
 * Here a check is made to see if ETH HSS services co-exist if so then
 * un initialization is not done for packetized images
 */    
    if (ixHssAccServiceType != IX_HSSACC_CHANNELIZED) 
    {

    
       if (IX_SUCCESS == status)
       {
          /* uninitialise the PktRx module */
          status = ixHssAccPktRxUninit ();
       }
    
       if (IX_SUCCESS == status)
       {
          /* uninitialise the PktTx module */
          status = ixHssAccPktTxUninit ();
       }

    }
   
    if (ixHssAccQmqsConfigured)
    {
        ixHssAccQmqsConfigured = FALSE;
    }


/*
 * Here we are destroying the common mutex that is used
 * Ethernet HssAcc services co existence
 */ 
   if (TRUE == ixEthHssAccCoexistEnable )
   {
        if ( TRUE == ixEthHssComMutexInitDone )
        {
            status = ixOsalMutexDestroy (&ixEthHssCoexistLock);
            if (IX_SUCCESS == status )
            {   /*uninitialisation complete */
                ixEthHssComMutexInitDone = FALSE;
            }
        }
   }
    
    if (IX_SUCCESS != status)
    {
        IX_HSSACC_REPORT_ERROR ("ixHssAccUninit: failed to uninit\n");
    }
    
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT,
              "Exiting ixHssAccUninit\n");
    return status;
}



/**
 * Function definition: ixHssAccPktPortConnect
 */
IX_STATUS 
ixHssAccPktPortConnect (IxHssAccHssPort hssPortId, 
			IxHssAccHdlcPort hdlcPortId, 
			BOOL hdlcFraming, 
			IxHssAccHdlcMode hdlcMode,
			BOOL hdlcBitInvert,
			unsigned blockSizeInWords,
			UINT32 rawIdleBlockPattern,
			IxHssAccPktHdlcFraming hdlcTxFraming, 
			IxHssAccPktHdlcFraming hdlcRxFraming, 
			unsigned frmFlagStart, 
			IxHssAccPktRxCallback rxCallback, 
			IxHssAccPktUserId rxUserId,
			IxHssAccPktRxFreeLowCallback rxFreeLowCallback, 
			IxHssAccPktUserId rxFreeLowUserId,
			IxHssAccPktTxDoneCallback txDoneCallback,
			IxHssAccPktUserId txDoneUserId)
{
    IX_STATUS status = IX_SUCCESS;
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering IxHssAccPktPortConnect\n");
   

    /* check for Ethernet HssAcc co-exist channelized images */
    if ( ixHssAccServiceType == IX_HSSACC_CHANNELIZED )
    {
        /* report the error */
        IX_HSSACC_REPORT_ERROR ("This image does not support packetized servicer\n");
        return IX_FAIL; 
    }
 
    if ((IX_HSSACC_ENUM_INVALID (hssPortId, hssPortMax)) ||
	(IX_HSSACC_ENUM_INVALID (hdlcPortId, IX_HSSACC_HDLC_PORT_MAX)) ||
	(blockSizeInWords > IX_HSSACC_PKT_MAX_TX_RX_BLOCK_SIZEW) ||
	(frmFlagStart > IX_HSSACC_PKT_MAX_FRMFLAGSTART) ||
        (rxCallback == NULL) ||
	(txDoneCallback == NULL))
    {
	IX_HSSACC_REPORT_ERROR ("ixHssAccPktPortConnect - invalid parameter\n");
	status = IX_HSSACC_PARAM_ERR;
    } 
    else if (ixHssAccHssInitialised[hssPortId] == FALSE)
    {
	IX_HSSACC_REPORT_ERROR ("ixHssAccPktPortConnect - hss port not initialised\n");
	status = IX_FAIL;
    }
    else
    {
	status = ixHssAccPCMConnect (hssPortId, hdlcPortId, hdlcFraming, 
				     hdlcMode, hdlcBitInvert,
				     blockSizeInWords, rawIdleBlockPattern,
				     hdlcTxFraming, 
				     hdlcRxFraming, frmFlagStart, 
				     rxCallback, rxUserId, 
				     rxFreeLowCallback, rxFreeLowUserId, 
				     txDoneCallback, txDoneUserId);

    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting IxHssAccPktPortConnect\n");
    return status;
}


/**
 * Function definition: ixHssAccPktPortEnable
 */
IX_STATUS 
ixHssAccPktPortEnable (IxHssAccHssPort hssPortId, IxHssAccHdlcPort hdlcPortId)
{
    IX_STATUS status = IX_SUCCESS;
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccPktPortEnable\n");
    
   
    /* check for Ethernet HssAcc co-exist images */
    if ( ixHssAccServiceType == IX_HSSACC_CHANNELIZED )
    {
       /* report the error */
       IX_HSSACC_REPORT_ERROR ("This image does'nt support packetized services\nn");
        return IX_FAIL; 
    }
 
    if ((IX_HSSACC_ENUM_INVALID (hssPortId, hssPortMax)) ||
	(IX_HSSACC_ENUM_INVALID (hdlcPortId, IX_HSSACC_HDLC_PORT_MAX)))
    {
	IX_HSSACC_REPORT_ERROR ("IxHssAccPktEnable: Incorrect Parameter\n");
	status = IX_HSSACC_PARAM_ERR;
    }
    else 
    {
	status = ixHssAccPCMPortEnable (hssPortId, hdlcPortId);
    }
    

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting ixHssAccPktPortEnable \n");


    return status;
}


/**
 * Function definition: ixHssAccPktPortDisable
 */
IX_STATUS 
ixHssAccPktPortDisable (IxHssAccHssPort hssPortId, IxHssAccHdlcPort hdlcPortId)
{
    IX_STATUS status = IX_SUCCESS;
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccPktPortDisable \n");

    /* check for Ethernet HssAcc co-exist channelized images */
    if ( ixHssAccServiceType == IX_HSSACC_CHANNELIZED )
    {
        /* report the error */
        IX_HSSACC_REPORT_ERROR ("This image does not support packetized servicer\n");
        return IX_FAIL; 
    }
    
    if ((IX_HSSACC_ENUM_INVALID (hssPortId, hssPortMax)) ||
	(IX_HSSACC_ENUM_INVALID (hdlcPortId, IX_HSSACC_HDLC_PORT_MAX)))
    {
	IX_HSSACC_REPORT_ERROR ("ixHssAccPktPortDisable: Invalid parameter\n");
	status = IX_HSSACC_PARAM_ERR;
    }
    else
    {
	status = ixHssAccPCMPortDisable (hssPortId, hdlcPortId);
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting ixHssAccPktPortDisable \n");

    return status;
}


/**
 * Function definition: ixHssAccPktPortDisconnect
 */
IX_STATUS 
ixHssAccPktPortDisconnect (IxHssAccHssPort hssPortId, IxHssAccHdlcPort hdlcPortId)
{
    IX_STATUS status = IX_SUCCESS;
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering IxHssAccPktPortDisconnect \n");
    
    /* check for Ethernet HssAcc co-exist channelized images */
    if ( ixHssAccServiceType == IX_HSSACC_CHANNELIZED )
    {
        /* report the error */
        IX_HSSACC_REPORT_ERROR ("This image does not support packetized servicer\n");
        return IX_FAIL; 
    }
    
    if ((IX_HSSACC_ENUM_INVALID (hssPortId, hssPortMax)) ||
	(IX_HSSACC_ENUM_INVALID (hdlcPortId, IX_HSSACC_HDLC_PORT_MAX)))
    {
	IX_HSSACC_REPORT_ERROR ("ixHssAccPktPortDisconnect: Invalid Parameter\n");
	status = IX_HSSACC_PARAM_ERR;
    }
    else
    {
	status = ixHssAccPCMDisconnect (hssPortId, hdlcPortId); 
    }
    
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting IxHssAccPktPortDisconnect \n");
    return status;
}

/**
 * Function definition: ixHssAccPktPortIsDisconnectComplete
 */
BOOL
ixHssAccPktPortIsDisconnectComplete (IxHssAccHssPort hssPortId, 
				     IxHssAccHdlcPort hdlcPortId)
{
    BOOL status;
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccPktPortIsDisconnectComplete\n");
    
    /* check for Ethernet HssAcc co-exist channelized images */
    if ( ixHssAccServiceType == IX_HSSACC_CHANNELIZED )
    {
        /* report the error */
        IX_HSSACC_REPORT_ERROR ("This image does not support packetized servicer\n");
        return IX_FAIL; 
    }
    
    if ((IX_HSSACC_ENUM_INVALID (hssPortId, hssPortMax)) ||
	(IX_HSSACC_ENUM_INVALID (hdlcPortId, IX_HSSACC_HDLC_PORT_MAX)))
    {
	IX_HSSACC_REPORT_ERROR ("ixHssAccPktPortIsDisconnectComplete:"
				" Invalid Parameter\n");
	status = FALSE;
    }
    else
    {
	status = ixHssAccPCMIsDisconnectComplete (hssPortId, hdlcPortId); 
    }
    
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting ixHssAccPktPortIsDisconnectComplete\n");
    return status;
}

/**
 * Function definition: ixHssAccPktPortRxFreeReplenish
 */
IX_STATUS 
ixHssAccPktPortRxFreeReplenish (IxHssAccHssPort hssPortId, 
				IxHssAccHdlcPort hdlcPortId, IX_OSAL_MBUF *buffer)
{
    IX_STATUS status = IX_SUCCESS;
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering IxHssAccPktPortRxFreeReplenish \n");
    
    /* check for Ethernet HssAcc co-exist channelized images */
    if ( ixHssAccServiceType == IX_HSSACC_CHANNELIZED )
    {
        /* report the error */
        IX_HSSACC_REPORT_ERROR ("This image does not support packetized servicer\n");
        return IX_FAIL; 
    }
   
    if (IX_HSSACC_ENUM_INVALID (hssPortId, hssPortMax) ||
	IX_HSSACC_ENUM_INVALID (hdlcPortId, IX_HSSACC_HDLC_PORT_MAX) ||
	(buffer == NULL) ||
	(IX_OSAL_MBUF_MLEN(buffer) < IX_HSSACC_PKT_MIN_RX_MBUF_SIZE))
    {
	IX_HSSACC_REPORT_ERROR ("IxHssAccPktPortRxFreeReplenish:"
				"Parameter Error\n");
	status = IX_HSSACC_PARAM_ERR;
    }
    else
    {
	status = ixHssAccPktRxFreeReplenish (hssPortId, 
					     hdlcPortId, 
					     buffer);
    }
    
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting IxHssAccPktPortRxFreeReplenish \n");

    return status;
}


/**
 * Function definition: ixHssAccPktTx
 */
IX_STATUS 
ixHssAccPktPortTx (IxHssAccHssPort hssPortId, IxHssAccHdlcPort hdlcPortId,
		   IX_OSAL_MBUF *buffer)
{
    IX_STATUS status = IX_SUCCESS;
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering IxHssAccPktPortTx \n");
    
    /* check for Ethernet HssAcc co-exist channelized images */
    if ( ixHssAccServiceType == IX_HSSACC_CHANNELIZED )
    {
        /* report the error */
        IX_HSSACC_REPORT_ERROR ("This image does not support packetized servicer\n");
        return IX_FAIL; 
    }
    
    if ((IX_HSSACC_ENUM_INVALID (hssPortId, hssPortMax)) ||
	(IX_HSSACC_ENUM_INVALID (hdlcPortId, IX_HSSACC_HDLC_PORT_MAX)) ||
	(buffer == NULL))
    {
	IX_HSSACC_REPORT_ERROR ("ixHssAccPktPortTx:"
				"Parameter Error\n");
	status = IX_HSSACC_PARAM_ERR;
    }
    else
    {
	status = ixHssAccPktTxInternal (hssPortId, hdlcPortId, buffer);
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting IxHssAccPktPortTx \n");

    return status;
}

/**
 * Function definition: ixHssAccChanConnect
 */
IX_STATUS 
ixHssAccChanConnect (IxHssAccHssPort hssPortId, 
		     unsigned bytesPerTSTrigger, 
		     UINT8 *rxCircular, 
		     unsigned numRxBytesPerTS, 
		     UINT32 *txPtrList, 
		     unsigned numTxPtrLists, 
		     unsigned numTxBytesPerBlk, 
		     IxHssAccChanRxCallback rxCallback)
{
    IX_STATUS status = IX_SUCCESS;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccChanConnect\n");

    /* Error check the parameters */
    if (IX_HSSACC_ENUM_INVALID (hssPortId, hssPortMax)          ||
	((bytesPerTSTrigger % IX_HSSACC_CHAN_RXBYTES_PER_TS_TRIG_DIV) != 0) ||
	(bytesPerTSTrigger < IX_HSSACC_CHAN_RXBYTES_PER_TS_TRIG_MIN)        ||
	(rxCircular == NULL)                                                ||
	((numRxBytesPerTS % IX_HSSACC_CHAN_RXBYTES_PER_TS_TRIG_DIV) != 0)   ||
	(numRxBytesPerTS < IX_HSSACC_CHAN_RXBYTES_PER_TS_MIN)               ||
	(txPtrList == NULL)                                                 ||
	(numTxPtrLists == 0)                                                ||
	((numTxBytesPerBlk % IX_HSSACC_CHAN_TXBYTES_PER_BLK_DIV) != 0)      ||
	(numTxBytesPerBlk < IX_HSSACC_CHAN_TXBYTES_PER_BLK_MIN)             ||
	(numTxBytesPerBlk > IX_HSSACC_CHAN_TXBYTES_PER_BLK_MAX))
    {
	/* report the error */
	IX_HSSACC_REPORT_ERROR ("ixHssAccChanConnect - invalid parameter\n");
	/* return error */
	status = IX_HSSACC_PARAM_ERR;
    }
    else if (ixHssAccHssInitialised[hssPortId] == FALSE)
    {
	/* report the error */
	IX_HSSACC_REPORT_ERROR ("ixHssAccChanConnect - hss port not initialised\n");
	/* return error */
	status = IX_FAIL;
    }
    else
    {
	status = ixHssAccCCMConnect (hssPortId, bytesPerTSTrigger, rxCircular, 
				     numRxBytesPerTS, txPtrList, numTxPtrLists, 
				     numTxBytesPerBlk, rxCallback);
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting ixHssAccChanConnect\n");
    return status;
}

/**
 * Function definition: ixHssAccChanPortEnable
 */
IX_STATUS ixHssAccChanPortEnable (IxHssAccHssPort hssPortId)
{
    IX_STATUS status = IX_SUCCESS;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering ixHssAccChanPortEnable\n");

    /* Error check the parameters */
    if (IX_HSSACC_ENUM_INVALID (hssPortId, hssPortMax))
    {
	/* report the error */
	IX_HSSACC_REPORT_ERROR ("ixHssAccChanPortEnable - invalid parameter\n");
	/* return error */
	status = IX_HSSACC_PARAM_ERR;
    }
    else
    {
	status = ixHssAccCCMPortEnable (hssPortId);
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting ixHssAccChanPortEnable\n");
    return status;
}

/**
 * Function definition: ixHssAccChanPortDisable
 */
IX_STATUS ixHssAccChanPortDisable (IxHssAccHssPort hssPortId)
{
    IX_STATUS status = IX_SUCCESS;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering ixHssAccChanPortDisable\n");

    /* Error check the parameters */
    if (IX_HSSACC_ENUM_INVALID (hssPortId, hssPortMax))
    {
	/* report the error */
	IX_HSSACC_REPORT_ERROR ("ixHssAccChanPortDisable - invalid parameter\n");
	/* return error */
	status = IX_HSSACC_PARAM_ERR;
    }
    else
    {
	status = ixHssAccCCMPortDisable (hssPortId);
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting ixHssAccChanPortDisable\n");
    return status;
}

/**
 * Function definition: ixHssAccChanDisconnect
 */
IX_STATUS ixHssAccChanDisconnect (IxHssAccHssPort hssPortId)
{
    IX_STATUS status = IX_SUCCESS;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccChanDisconnect\n");

    /* Error check the parameters */
    if (IX_HSSACC_ENUM_INVALID (hssPortId, hssPortMax))
    {
	/* report the error */
	IX_HSSACC_REPORT_ERROR ("ixHssAccChanDisconnect - invalid parameter\n");
	/* return error */
	status = IX_HSSACC_PARAM_ERR;
    }
    else
    {
	status = ixHssAccCCMDisconnect (hssPortId);
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting ixHssAccChanDisconnect\n");
    return status;
}

/**
 * Function definition: ixHssAccChanStatusQuery
 */
IX_STATUS 
ixHssAccChanStatusQuery (IxHssAccHssPort hssPortId, 
			 BOOL *dataRecvd, 
			 unsigned *rxOffset, 
			 unsigned *txOffset, 
			 unsigned *numHssErrs)
{
    IX_STATUS status = IX_SUCCESS;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering ixHssAccChanStatusQuery\n");

    /* Error check the parameters */
    if (IX_HSSACC_ENUM_INVALID (hssPortId, hssPortMax) ||
	dataRecvd == NULL ||
	rxOffset == NULL  ||
	txOffset == NULL  ||
	numHssErrs == NULL)
    {
	/* report the error */
	IX_HSSACC_REPORT_ERROR ("ixHssAccChanStatusQuery - invalid parameter\n");
	/* return error */
	status = IX_HSSACC_PARAM_ERR;
    }
    else
    {
	status = ixHssAccChanRxQCheck (hssPortId, dataRecvd, rxOffset,
				       txOffset, numHssErrs);
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting ixHssAccChanStatusQuery\n");
    return status;
}


/**
 * Function definition: ixHssAccChanTslotSwitchEnable
 */
IX_STATUS 
ixHssAccChanTslotSwitchEnable (IxHssAccHssPort hssPortId, 
			 UINT32 srcTimeslot, 
			 UINT32 destTimeslot,
			 UINT32 *tsSwitchHandle)
{
    IX_STATUS status = IX_SUCCESS;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccChanTslotSwitchEnable\n");

    /* Error check the parameters */
    if (IX_HSSACC_ENUM_INVALID (hssPortId, IX_HSSACC_CHAN_TSLOTSWITCH_NUM_HSS_PORT_MAX) ||
	IX_HSSACC_ENUM_INVALID (srcTimeslot, IX_HSSACC_TSLOTS_PER_HSS_PORT)             ||
	IX_HSSACC_ENUM_INVALID (destTimeslot, IX_HSSACC_TSLOTS_PER_HSS_PORT)            ||
	(tsSwitchHandle == NULL))
    {
    	/* report the error */
	IX_HSSACC_TRACE0 (IX_HSSACC_DEBUG, 
				"ixHssAccChanTslotSwitchEnable - invalid parameter\n");	
	/* return error */
	status = IX_HSSACC_PARAM_ERR;
    }
    else if (ixHssAccHssInitialised[hssPortId] == FALSE)
    {
	/* report the error */
	IX_HSSACC_TRACE0 (IX_HSSACC_DEBUG, 
			"ixHssAccChanTslotSwitchEnable - hss port not initialised\n");

	/* return error */
	status = IX_FAIL;
    }
    else if (!ixHssAccComIsChanTimeslot(hssPortId, srcTimeslot))
    {
	/* report the error */
      IX_HSSACC_TRACE0 (IX_HSSACC_DEBUG, 
			"ixHssAccChanTslotSwitchEnable - srcTimeslot is not a"
	    		" channelised timeslot\n");

	/* return error */
	status = IX_FAIL;
    }
    else if (!ixHssAccComIsChanTimeslot(hssPortId, destTimeslot))
    {
	/* report the error */
      IX_HSSACC_TRACE0 (IX_HSSACC_DEBUG, 
			"ixHssAccChanTslotSwitchEnable - destTimeslot is not a"
	    		" channelised timeslot\n");

	/* return error */
	status = IX_FAIL;
    }
    else if (ixHssAccComNumChanTimeslotGet(hssPortId) < IX_HSSACC_CHAN_TSLOTSWITCH_NUM_CHAN_MIN)
    {
	/* report the error */
      IX_HSSACC_TRACE0 (IX_HSSACC_DEBUG, 
			"ixHssAccChanTslotSwitchEnable - this service requires"
	    		" at least 8 channelised timeslots to exist per HSS port\n");
	

	/* return error */
	status = IX_FAIL;
    }
    else
    {
	status = ixHssAccCCMTslotSwitchEnable (hssPortId, srcTimeslot, 
			 		       destTimeslot, tsSwitchHandle);
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting ixHssAccChanTslotSwitchEnable\n");
    return status;
}

/**
 * Function definition: ixHssAccChanTslotSwitchDisable
 */
IX_STATUS 
ixHssAccChanTslotSwitchDisable (IxHssAccHssPort hssPortId, 
			 UINT32 tsSwitchHandle)
{
    IX_STATUS status = IX_SUCCESS;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccChanTslotSwitchDisable\n");

    /* Error check the parameters */
    if (IX_HSSACC_ENUM_INVALID (hssPortId, IX_HSSACC_CHAN_TSLOTSWITCH_NUM_HSS_PORT_MAX) ||
	IX_HSSACC_ENUM_INVALID (tsSwitchHandle, IX_HSSACC_CHAN_TSLOTSWITCH_NUM_BYPASS_MAX))
    {
    	/* report the error */
	IX_HSSACC_TRACE0 (IX_HSSACC_DEBUG, 
			"ixHssAccChanTslotSwitchDisable - invalid parameter\n");

	/* return error */
	status = IX_HSSACC_PARAM_ERR;
    }
    else
    {
	status = ixHssAccCCMTslotSwitchDisable (hssPortId, tsSwitchHandle);
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting ixHssAccChanTslotSwitchDisable\n");
    return status;
}

/**
 * Function definition: ixHssAccChanTslotSwitchGctDownload
 */
IX_STATUS 
ixHssAccChanTslotSwitchGctDownload (IxHssAccHssPort hssPortId, 
			 UINT8 *gainCtrlTable,
			 UINT32 tsSwitchHandle)
{
    IX_STATUS status = IX_SUCCESS;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccChanTslotSwitchGctDownload\n");

    /* Error check the parameters */
    if (IX_HSSACC_ENUM_INVALID (hssPortId, IX_HSSACC_CHAN_TSLOTSWITCH_NUM_HSS_PORT_MAX)    ||
	IX_HSSACC_ENUM_INVALID (tsSwitchHandle, IX_HSSACC_CHAN_TSLOTSWITCH_NUM_BYPASS_MAX) ||
	(gainCtrlTable == NULL))
    {
    	/* report the error */
      IX_HSSACC_TRACE0 (IX_HSSACC_DEBUG, 
			"ixHssAccChanTslotSwitchGctDownload - invalid parameter\n");
	
	/* return error */
	status = IX_HSSACC_PARAM_ERR;
    }
    else
    {
	status = ixHssAccCCMTslotSwitchGctDownload (hssPortId, gainCtrlTable, 
						    tsSwitchHandle);
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting ixHssAccChanTslotSwitchGctDownload\n");
    return status;
}

/**
 * Function definition: ixHssAccShow
 */
void ixHssAccShow (void)
{
    ixHssAccComShow ();
    ixHssAccChanRxShow ();
    ixHssAccCCMShow ();
    ixHssAccPCMShow ();
    ixHssAccPktTxShow ();
    ixHssAccPktRxShow ();

    printf ("\n");
}

/**
 * Function definition: ixHssAccStatsInit
 */
void ixHssAccStatsInit (void)
{

    ixHssAccComStatsInit ();
    ixHssAccChanRxStatsInit ();
    ixHssAccCCMStatsInit ();
    ixHssAccPCMStatsInit ();
    ixHssAccPktTxStatsInit ();
    ixHssAccPktRxStatsInit ();

}
