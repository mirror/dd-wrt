/*
 * @file    IxDmaAcc.c
 *
 * @date    18 October 2002
 *
 * @brief   API of the IXP400 DMA Access Driver Component (IxDma)
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
 * User defined include files listed below
 */
#include "IxOsal.h"
#include "IxDmaAcc.h"
#include "IxDmaAcc_p.h"
#include "IxNpeDl.h"
#include "IxQMgr.h"
#include "IxFeatureCtrl.h"


/*
 * System defined include files listed below
 */

/*
 *  Local Variables
 */
static BOOL ixDmaAccInitDone = FALSE;  /* Flag is true once DmaIitialization
                                          is done */
static IxQMgrQId ixDmaQIdDmaDone;     /* Done Q Id    */
static IxQMgrQId ixDmaQIdDmaRequest;  /* Request Q Id */
static IxDmaAccStats dmaStats;     /* Instantiate statistics data structure */
/***********************************************************************
 * @fn IxDmaAccInit
 * @param npeId : The Id for NPE A, NPE B or NPE C
 * @brief Initialize the DMA Access Component
 * @return IX_SUCCESS : Initialization succeeded
 * @return IX_FAIL : Initialization Failed
 ***********************************************************************/
PUBLIC IX_STATUS
ixDmaAccInit(IxNpeDlNpeId npeId)
{
    #if((CPU!=SIMSPARCSOLARIS) && (CPU!=SIMLINUX))
    /*
     * Check whether NPE is present 
     */
    if (IX_NPEDL_NPEID_NPEA == npeId)
    {  
        /* Check whether NPE A is present */ 
        if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEA)== 
            IX_FEATURE_CTRL_COMPONENT_DISABLED)
        {   
            /* NPE A does not present */
            ixOsalLog (IX_OSAL_LOG_LVL_WARNING,
                       IX_OSAL_LOG_DEV_STDOUT, 
                       "Warning:NPEA does not present.\n",
	               0,0,0,0,0,0);
            return IX_FAIL;
        }
    } 
    else if (IX_NPEDL_NPEID_NPEB == npeId)
    {  
        /* Check whether NPE B is present */ 
        if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEB)== 
            IX_FEATURE_CTRL_COMPONENT_DISABLED)
        {   
            /* NPE B does not present */
            ixOsalLog (IX_OSAL_LOG_LVL_WARNING,
                       IX_OSAL_LOG_DEV_STDOUT,
                       "Warning:NPEB does not present.\n",
	               0,0,0,0,0,0);
            return IX_FAIL; 
        }
    } 
    else if (IX_NPEDL_NPEID_NPEC == npeId)
    { 
        /* Check whether NPE C is present */ 
        if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEC)== 
            IX_FEATURE_CTRL_COMPONENT_DISABLED)
        {   
            /* NPE C does not present */
            ixOsalLog (IX_OSAL_LOG_LVL_WARNING,
                       IX_OSAL_LOG_DEV_STDOUT, 
                       "Warning:NPEC does not present.\n",
	               0,0,0,0,0,0);	        
            return IX_FAIL;
        } 
    }
    else
    {
        /* Invalid NPE ID */
        ixOsalLog (IX_OSAL_LOG_LVL_WARNING,
                   IX_OSAL_LOG_DEV_STDOUT,
                   "ixDmaAccInit : invalid Npe ID.\n",
                   0,0,0,0,0,0);             
        return IX_FAIL;
    }  
  #endif
    /* Check if ixDmaInit has been initialized already*/
    if( TRUE == ixDmaAccInitDone )
    {
        /* Log error message in debugging mode */
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDERR,
                   "\nixDmaAccInit : Dma initialization is performed more than once.",
                   0,0,0,0,0,0);
        return IX_FAIL;
    }

    /* Select the Queues based on the NPE to be used */
    switch (npeId)
    {
        case IX_NPEDL_NPEID_NPEA:
             /* Select the Queues for NPE A */
             ixDmaQIdDmaDone = IX_DMA_NPE_A_DONE_QID;            /* NPE A Done Q Id    */
             ixDmaQIdDmaRequest = IX_DMA_NPE_A_REQUEST_QID;      /* NPE A Request Q Id */
             break;
        case IX_NPEDL_NPEID_NPEB:
             /* Select the Queues for NPE B */
             ixDmaQIdDmaDone = IX_DMA_NPE_B_DONE_QID;            /* NPE B Done Q Id    */
             ixDmaQIdDmaRequest = IX_DMA_NPE_B_REQUEST_QID;      /* NPE B Request Q Id */
             break;
        case IX_NPEDL_NPEID_NPEC:
             /* Select the Queues for NPE C */
             ixDmaQIdDmaDone = IX_DMA_NPE_C_DONE_QID;            /* NPE C Done Q Id    */
             ixDmaQIdDmaRequest = IX_DMA_NPE_C_REQUEST_QID;      /* NPE C Request Q Id */
             break;
        default:
             /* Invalid NPE ID */
             ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
             			IX_OSAL_LOG_DEV_STDERR,
                        "\nixDmaAccInit : invalid Npe ID.",
                        0,0,0,0,0,0);
             return IX_FAIL;
     } /* end of switch (npeId) */

    /* Configure the Dma Request Q */
    if( IX_SUCCESS !=
        ixQMgrQConfig ( "DMA Request Q",
	                    ixDmaQIdDmaRequest,
                        IX_QMGR_Q_SIZE16,
	                    IX_QMGR_Q_ENTRY_SIZE1) )
    {
        /* Log error message in debugging mode */
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDERR,
                   "\nixDmaAccInit : DMA Request Q initialization failed.",
                   0,0,0,0,0,0);
        return (IX_FAIL);
    }

    /* Configure the Dma Done Q */
    if( IX_SUCCESS !=
        ixQMgrQConfig ( "DMA Done Q",
                        ixDmaQIdDmaDone,
                        IX_QMGR_Q_SIZE16,
                        IX_QMGR_Q_ENTRY_SIZE1) )
    {
        /* Log error message in debugging mode */
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDERR,
                   "\nixDmaAccInit : DMA Done Q initialization failed.",
                   0,0,0,0,0,0);
        return (IX_FAIL);
    }

    /* Initialize Descriptor Pool */
    if( IX_SUCCESS != ixDmaAccDescriptorPoolInit() )
    {
        /* Log error message in debugging mode */
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDERR,
                   "\nixDmaAccInit : Descriptor pool initialization failed.",
                   0,0,0,0,0,0);
        return (IX_FAIL);
    }

    if ( IX_SUCCESS !=
         ixQMgrNotificationDisable(ixDmaQIdDmaDone))
    {
        /* Log error message in debugging mode */
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDERR,
                   "\nixDmaAccInit : DMA done Q manager notification disable failed.",
                   0,0,0,0,0,0);
        return (IX_FAIL);
    }
    /* set up the Dma done call back */
    if ( IX_SUCCESS !=
         ixQMgrNotificationCallbackSet(
               ixDmaQIdDmaDone,
               (IxQMgrCallback) ixDmaTransferDoneCallback,
               IX_DMA_CALLBACK_ID_DMADONE ) )
    {
        /* Log error message in debugging mode */
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDERR,
                   "\nixDmaAccInit : Dma done callback registration failed.",
                   0,0,0,0,0,0);
        return (IX_FAIL);
    }

    if ( IX_SUCCESS !=
         ixQMgrNotificationEnable(
               ixDmaQIdDmaDone,
               IX_QMGR_Q_SOURCE_ID_NOT_E) )
    {
        /* Log error message in debugging mode */
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDERR,
                   "\nixDmaAccInit : Dma done Q manager enable failed.",
                   0,0,0,0,0,0);
        return (IX_FAIL);
    }
    /* Initialize dma statistics */
    dmaStats.successCnt = 0;
    dmaStats.failCnt = 0;
    dmaStats.qOverflowCnt = 0;
    dmaStats.qUnderflowCnt = 0;
    dmaStats.qDescAddrInvalidCnt = 0;
    ixDmaAccInitDone = TRUE;
    return (IX_SUCCESS);
}



/* Module - ixDmaAccUninit () */
PUBLIC IX_STATUS
ixDmaAccUninit (IxNpeDlNpeId npeId)
{
	IX_STATUS status;
    /*Check wheather the NPE is present*/
    #if((CPU!=SIMSPARCSOLARIS) && (CPU!=SIMLINUX))
    if (IX_NPEDL_NPEID_NPEA == npeId)
    {
        /* Check whether NPE A is present */
        if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEA)==
                IX_FEATURE_CTRL_COMPONENT_DISABLED)
        {
            /* NPE A does not present */
            ixOsalLog (IX_OSAL_LOG_LVL_WARNING,
                       IX_OSAL_LOG_DEV_STDOUT,
                       "Warning:NPEA does not present.\n",
                        0,0,0,0,0,0);
            return IX_FAIL; 
        }
    }
    else if (IX_NPEDL_NPEID_NPEB == npeId)
    {
        /* Check whether NPE B is present */
        if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEB)==
                IX_FEATURE_CTRL_COMPONENT_DISABLED)
        {
            /* NPE B does not present */
            ixOsalLog (IX_OSAL_LOG_LVL_WARNING,
                       IX_OSAL_LOG_DEV_STDOUT,
                       "Warning:NPEB does not present.\n",
                       0,0,0,0,0,0);
            return IX_FAIL;
        }
    }
    else if (IX_NPEDL_NPEID_NPEC == npeId)
    {
        /* Check whether NPE C is present */
        if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEC)==
                IX_FEATURE_CTRL_COMPONENT_DISABLED)
        {
            /* NPE C does not present */
            ixOsalLog (IX_OSAL_LOG_LVL_WARNING,
                       IX_OSAL_LOG_DEV_STDOUT,
                       "Warning:NPEC does not present.\n",
                       0,0,0,0,0,0);
            return IX_FAIL;
        }
    }
    else
    {
        /* Invalid NPE ID */
        ixOsalLog (IX_OSAL_LOG_LVL_WARNING,
                   IX_OSAL_LOG_DEV_STDOUT,
                   "ixDmaAccUninit : invalid Npe ID.\n",
                   0,0,0,0,0,0);
        return IX_FAIL;
    }

    #endif

    /* Select the Queues based on the NPE to be used */
    switch (npeId)
    {
        case IX_NPEDL_NPEID_NPEA:
             /* Select the Queues for NPE A */
             ixDmaQIdDmaDone = IX_DMA_NPE_A_DONE_QID;            /* NPE A Done Q Id    */
             ixDmaQIdDmaRequest = IX_DMA_NPE_A_REQUEST_QID;      /* NPE A Request Q Id */
             break;
        case IX_NPEDL_NPEID_NPEB:
             /* Select the Queues for NPE B */
             ixDmaQIdDmaDone = IX_DMA_NPE_B_DONE_QID;            /* NPE B Done Q Id    */
             ixDmaQIdDmaRequest = IX_DMA_NPE_B_REQUEST_QID;      /* NPE B Request Q Id */
             break;
        case IX_NPEDL_NPEID_NPEC:
             /* Select the Queues for NPE C */
             ixDmaQIdDmaDone = IX_DMA_NPE_C_DONE_QID;            /* NPE C Done Q Id    */
             ixDmaQIdDmaRequest = IX_DMA_NPE_C_REQUEST_QID;      /* NPE C Request Q Id */
             break;
        default:
             /* Invalid NPE ID */
             ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
                        IX_OSAL_LOG_DEV_STDERR,
                        "\n ixDmaAccUninit : invalid Npe ID.",
                        0,0,0,0,0,0);
             return IX_FAIL;
     } /* end of switch (npeId) */

    status = ixQMgrNotificationDisable (ixDmaQIdDmaDone);
	if (IX_SUCCESS != status)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
                   IX_OSAL_LOG_DEV_STDERR,
                   "\n ixDmaAccUninit : ixQMgrNotificationDisable failed.",
                   0,0,0,0,0,0);
	}

    status = ixQMgrNotificationCallbackSet (ixDmaQIdDmaDone, NULL, 0);
	if (IX_SUCCESS != status)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
                   IX_OSAL_LOG_DEV_STDERR,
                   "\n ixDmaAccUninit : ixQMgrNotificationCallbackSet failed.",
                   0,0,0,0,0,0);
	}
    status = ixQMgrNotificationDisable (ixDmaQIdDmaRequest);
	if (IX_SUCCESS != status)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
                   IX_OSAL_LOG_DEV_STDERR,
                   "\n ixDmaAccUninit : ixQMgrNotificationDisable failed.",
                   0,0,0,0,0,0);
	}
    status = ixQMgrNotificationCallbackSet (ixDmaQIdDmaRequest, NULL, 0);
	if (IX_SUCCESS != status)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
                   IX_OSAL_LOG_DEV_STDERR,
                   "\n ixDmaAccUninit : ixQMgrNotificationCallbackSet failed.",
                   0,0,0,0,0,0);
	}

    /* Initialize dma statistics */
    dmaStats.successCnt          = 0;
    dmaStats.failCnt             = 0;
    dmaStats.qOverflowCnt        = 0;
    dmaStats.qUnderflowCnt       = 0;
    dmaStats.qDescAddrInvalidCnt = 0;

    status = ixDmaAccDescriptorPoolUninit ();
	if (IX_SUCCESS != status)
    {
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
                   IX_OSAL_LOG_DEV_STDERR,
                   "\n ixDmaAccUninit : ixDmaAccDescriptorPoolUninit failed.",
                   0,0,0,0,0,0);
	}

	ixDmaAccInitDone = FALSE;
    return status;
}



/***********************************************************************
 * @fn ixDmaAccDmaTransfer
 *
 * @param In: ixDmaSourceAddr : Start address of DMA source
 * @param In: ixDmaDestinationAddr : Start address of DMA destination
 * @param In: ixDmaTransferLength : Size of DMA transfer (1-64Kb)
 * @param In: ixDmaTransferMode : DMA transfer mode
 * @param In: ixDmaAddressingMode : DMA addressing mode
 * @param In: ixTransferWidth : DMA transfer width
 * @param Callback : to call when DMA transfer is done
 * @brief Perform the DMA transfer
 *
 * @return IX_DMA_SUCCESS : notification that DMA request is succesful
 * @return IX_DMA_FAIL    : IxDmaAcc not yet initialized or internal error
 * @return IX_DMA_INVALID_TRANSFER_WIDTH : transfer width is not valid
 * @return IX_DMA_INVALID_TRANSFER_LENGTH: length of transfer not valid
 * @return IX_DMA_INVALID_TRANSFER_MODE  : transfer mode not valid
 * @return IX_DMA_INVALID_ADDRESS_MODE   : address mode not valid
 * @return IX_DMA_REQUEST_FIFO_FULL : IxDmaAcc request queue is full
 *
 ***********************************************************************/
PUBLIC IxDmaReturnStatus
ixDmaAccDmaTransfer(
    IxDmaAccDmaCompleteCallback callback,
    UINT32 sourceAddr,
    UINT32 destinationAddr,
    UINT16 transferLength,
    IxDmaTransferMode transferMode,
    IxDmaAddressingMode addressingMode,
    IxDmaTransferWidth transferWidth)
{
    UINT32 operationMode=0;  /* Variable for composing transfer mode is
                                initialized to zero */
    IxDmaReturnStatus dmaStatus;     /* Return status for Dma transfer */
    IxDmaNpeQDescriptor *descriptorPtr; /* Pointer to descriptor */
    UINT32 u32pNpeQDesc = 0;
	/* Check if ixDmaInit has been initialized already*/
    if( FALSE == ixDmaAccInitDone )
    {
        /* Log error message in debugging mode */
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDERR,
            	   "\nixDmaAccDmaTransfer : transfer called before initialization.",
            	   0,0,0,0,0,0);
        return (IX_DMA_FAIL);
    } /* end of if(!ixDmaAccInitDone) */

    /* Validate parameters provided by the caller : ixDmaAccParamsValidate */
    dmaStatus = ixDmaAccParamsValidate( sourceAddr,
                                        destinationAddr,
                                        transferLength,
                                        transferMode,
                                        addressingMode,
                                        transferWidth);

    if( IX_DMA_SUCCESS != dmaStatus )
    {
        /* Log error message in debugging mode */
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDERR,
                   "\nixDmaAccDmaTransfer : parameter validation failed.",
                   0,0,0,0,0,0);
        return dmaStatus;
    }  /* end of if(dmaStatus) */

    /* Compose the Addressing Mode field in the DMA Transfer Mode
       descriptor word */
    switch (addressingMode)
    {
        case IX_DMA_INC_SRC_INC_DST:
             /* Compose the Addressing Mode field for src address increment
                and dest address increment */
             operationMode |= IX_DMA_MODE_INC_INC;
             break;
        case IX_DMA_INC_SRC_FIX_DST:
             /* Compose the Addressing Mode field for src address increment
                and fixed dest address */
             operationMode |= IX_DMA_MODE_INC_FIX;
             break;
        case IX_DMA_FIX_SRC_INC_DST:
             /* Compose the Addressing Mode field for fixed src address and
                dest address increment */
             operationMode |= IX_DMA_MODE_FIX_INC;
             break;
        default:
             /* Mode is not valid and should have been rejected at validation
                check */
             return IX_DMA_INVALID_ADDRESS_MODE;
     } /* end of switch (transferMode) */

    /* Compose the Transfer Mode field in the DMA Transfer Mode
       descriptor word */
    switch (transferMode)
    {
        case IX_DMA_COPY:
             /* Compose the Transfer Mode field for copy only */
             operationMode |= IX_DMA_MODE_COPY;
             break;
        case IX_DMA_COPY_CLEAR:
             /* Compose the Transfer Mode field for copy and clear source */
             operationMode |= IX_DMA_MODE_COPY_CLEAR;
             break;
        case IX_DMA_COPY_BYTE_SWAP:
             /* Compose the Transfer Mode field for copy and byte swap */
             operationMode |= IX_DMA_MODE_COPY_BYTE_SWAP;
             break;
        case IX_DMA_COPY_REVERSE:
             /* Compose the Transfer Mode field for copy and byte reverse */
             operationMode |= IX_DMA_MODE_COPY_REVERSE;
             break;
        default:
             /* Mode is not valid and should have been rejected at validation check */
             return IX_DMA_INVALID_TRANSFER_MODE;
     } /* end of switch (transferMode) */

    /* Compose the Transfer Width field in the DMA Transfer Mode descriptor word */
    switch (transferWidth)
    {
        /* Compose the Transfer Width field for 32 bit src and 32 bit dst */
        case IX_DMA_32_SRC_32_DST:
             operationMode |= IX_DMA_MODE_TRANSWIDTH_32_32;
             break;
        /* Compose the Transfer Width field for 32 bit src and 16 bit dst */
        case IX_DMA_32_SRC_16_DST:
             operationMode |= IX_DMA_MODE_TRANSWIDTH_32_16;
             break;
        /* Compose the Transfer Width field for 32 bit src and 8 bit dst */
        case IX_DMA_32_SRC_8_DST:
             operationMode |= IX_DMA_MODE_TRANSWIDTH_32_8;
             break;
        /* Compose the Transfer Width field for 16 bit src and 3 bit dst */
        case IX_DMA_16_SRC_32_DST:
             operationMode |= IX_DMA_MODE_TRANSWIDTH_16_32;
             break;
        /* Compose the Transfer Width field for 16 bit src and 16 bit dst */
        case IX_DMA_16_SRC_16_DST:
             operationMode |= IX_DMA_MODE_TRANSWIDTH_16_16;
             break;
        /* Compose the Transfer Width field for 16 bit src and 8 bit dst */
        case IX_DMA_16_SRC_8_DST:
             operationMode |= IX_DMA_MODE_TRANSWIDTH_16_8;
             break;
        /* Compose the Transfer Width field for 8 bit src and 32 bit dst */
        case IX_DMA_8_SRC_32_DST:
             operationMode |= IX_DMA_MODE_TRANSWIDTH_8_32;
             break;
        /* Compose the Transfer Width field for 8 bit src and 16 bit dst */
        case IX_DMA_8_SRC_16_DST:
             operationMode |= IX_DMA_MODE_TRANSWIDTH_8_16;
             break;
        /* Compose the Transfer Width field for 8 bit src and 8 bit dst */
        case IX_DMA_8_SRC_8_DST:
             operationMode |= IX_DMA_MODE_TRANSWIDTH_8_8;
             break;
        /* Compose the Transfer Width field for 32 bit src and burst dst */
        case IX_DMA_32_SRC_BURST_DST:
             operationMode |= IX_DMA_MODE_TRANSWIDTH_32_BURST;
             break;
        /* Compose the Transfer Width field for 16 bit src and burst dst */
        case IX_DMA_16_SRC_BURST_DST:
             operationMode |= IX_DMA_MODE_TRANSWIDTH_16_BURST;
             break;
        /* Compose the Transfer Width field for 8 bit src and burst dst */
        case IX_DMA_8_SRC_BURST_DST:
             operationMode |= IX_DMA_MODE_TRANSWIDTH_8_BURST;
             break;
        /* Compose the Transfer Width field for burst src and 32 bit dst */
        case IX_DMA_BURST_SRC_32_DST:
             operationMode |= IX_DMA_MODE_TRANSWIDTH_BURST_32;
             break;
        /* Compose the Transfer Width field for burst src and 16 bit dst */
        case IX_DMA_BURST_SRC_16_DST:
             operationMode |= IX_DMA_MODE_TRANSWIDTH_BURST_16;
             break;
        /* Compose the Transfer Width field for burst src and 8 bit dst */
        case IX_DMA_BURST_SRC_8_DST:
             operationMode |= IX_DMA_MODE_TRANSWIDTH_BURST_8;
             break;
        /* Compose the Transfer Width field for burst src and burst dst */
        case IX_DMA_BURST_SRC_BURST_DST:
             operationMode |= IX_DMA_MODE_TRANSWIDTH_BURST_BURST;
             break;
        default:
             /* Mode is not valid and should have been rejected at
                validation check */
        return IX_DMA_INVALID_TRANSFER_WIDTH;
    } /* end of switch (transferWidth) */

    /* Since parameters are valid, get descriptor entry from the descriptor
       manager */
    if( IX_DMA_DM_FIFO_FULL == ixDmaAccDescriptorGet(&descriptorPtr) )
    {
        /* Log error message in debugging mode */
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDERR,
                   "\nDma transfer aborted : descriptor get failed.",
                   0,0,0,0,0,0);
        return (IX_DMA_REQUEST_FIFO_FULL);
    }  /* end of if(ixDmaAccDescriptorGet) */

    /* Compose the first sixteen bits with tranfer length */
    operationMode |= (UINT32) transferLength;

    /* Load first descriptor word : source address */
    descriptorPtr->sourceAddress = IX_OSAL_MMAP_VIRT_TO_PHYS(sourceAddr); 

    /* Load second descriptor word : destination address */
    descriptorPtr->destinationAddress = IX_OSAL_MMAP_VIRT_TO_PHYS(destinationAddr);

    /* Load third descriptor word : Dma transfer mode */
    descriptorPtr->operationMode = operationMode;

    /* Load third descriptor word : Pointer to callback function */
    descriptorPtr->pDmaCallback = callback;

    /* Flush the cache for descriptor before performing a write to the
       Dma request Q */
    IX_OSAL_CACHE_FLUSH(descriptorPtr,sizeof(IxDmaNpeQDescriptor));

    /* Translate descriptor address from Virtual to Physical */
    descriptorPtr = (IxDmaNpeQDescriptor*)
                    IX_OSAL_MMU_VIRT_TO_PHYS(descriptorPtr);

    /* Load descriptor pointer to the DMA REQUEST Q */
    u32pNpeQDesc = (UINT32)descriptorPtr; 
    if ( IX_SUCCESS !=
         ixQMgrQWrite( ixDmaQIdDmaRequest,
                       &u32pNpeQDesc) )
    {
        /* IX_DMA_REQUEST_FIFO_FULL, increase the counter */
        dmaStats.qOverflowCnt++;
        /* Descriptor failed to load into dma request queue */
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDERR,
                   "\nDma transfer aborted : dma request Q overflow.",
                   0,0,0,0,0,0);
        return (IX_DMA_REQUEST_FIFO_FULL);
    }  /* end of if (ixQMgrQWrite) */
    /* Increment counter for number of transfer requests completed
     * successfully without error
     */
    dmaStats.successCnt++;


    return(IX_DMA_SUCCESS);
} /* end of function ixDmaAccDmaTransfer */

/***********************************************************************
 * @fn ixDmaShow
 * @param None
 * @brief Display component information for statistics purposes
 * @return IX_SUCCESS : Notification that statistics show is succesful
 * @return IX_FAIL    : Statistics show not succesful
 ***********************************************************************/
PUBLIC IX_STATUS
ixDmaAccShow(void)
{
    IX_STATUS status; /* Status to return to client for DmaAccShow */
    /* Return FAIL if initialization has not been done */
    if(ixDmaAccInitDone)
    {
        /* Show descriptor pool statistics */
        ixDmaAccDescPoolShow();
        /* Show Dma transfer Statistics */
        printf ("\n\nNumber of successful dma requests         : %d \n",
                dmaStats.successCnt);
        printf ("Number of unsuccessful dma requests       : %d \n",
                dmaStats.failCnt);
        /* Show Q Statistics */
        printf ("Number of times Dma Req Q overflow        : %d \n",
                dmaStats.qOverflowCnt);
        printf ("Number of times Dma Done Q underflow      : %d \n",
                dmaStats.qUnderflowCnt);
        printf ("Number of invalid Q descriptors submitted : %d \n",
                dmaStats.qDescAddrInvalidCnt);
        status = IX_SUCCESS;
    }
    else
    {
        printf("\nDma Show : Dma access driver not initialized");
        status = IX_FAIL;
    } /* end of if(ixDmaAccInitDone) */
    return status;
} /* end of function ixDmaAccShow */


/***********************************************************************
 * @fn ixDmaTransferDoneCallback
 * @param  QId :  Queue Identifier for Dma done
 * @param  cbId : Callback Identifier for Dma done
                  (this parameter is not used)
 * @brief This callback is registered with the queue manager for
 *        notification of Dma transfer done event
 *        Q manager calls this function when Queue is Not Empty
 *
 * @return none
 *
 ***********************************************************************/
void
ixDmaTransferDoneCallback(
    IxQMgrQId qId,
    IxQMgrCallbackId cbId )
{
    UINT32 qEntry;                               /* Temporary variable for
                                                    pointer to descriptor */
    IxDmaNpeQDescriptor *descriptorPtr;          /* Temporary variable for
                                                    pointer to descriptor */
    IxDmaReturnStatus status = IX_DMA_FAIL;      /* Default return status is
                                                    FAIL */
    /* Read the queue to get the descriptor */
    if ( IX_SUCCESS != ixQMgrQRead ( ixDmaQIdDmaDone, &qEntry ) )
    {
        /* increase the counter */
        dmaStats.qUnderflowCnt++;
        /* Descriptor failed to read from dma done queue */
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDERR,
            	   "\nixDmaTransferDoneCallback : failed to read descriptor "
            	   "pointer from Dma Done Q.",
            	   0,0,0,0,0,0);
        return;
    }

    /* Load descriptor from the Dma Done queue to temporary descriptor pointer */
    descriptorPtr = (IxDmaNpeQDescriptor *)
                    IX_OSAL_MMU_PHYS_TO_VIRT(qEntry);
    if ( NULL != descriptorPtr)
    {
        /* Refresh the cache for descriptor obtained */
        IX_OSAL_CACHE_INVALIDATE (descriptorPtr,
                                  sizeof (IxDmaNpeQDescriptor));

        /* Check for error condition in the above descriptor : examine
           operationMode parameter */
        if ( descriptorPtr->operationMode == 0 )
        {
           /* Increment counter for number of requests completed
            * but operation failed
            */
           dmaStats.failCnt++;
           /* Dma transfer failed */
           ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
           			  IX_OSAL_LOG_DEV_STDERR,
                      "\nixDmaTransferDoneCallback : dma transfer failed.",
                      0,0,0,0,0,0);
           status = IX_DMA_FAIL;
        }
        else
        {
           status = IX_DMA_SUCCESS;
        }  /* end of if (descriptorPtr->operationMode) */

        /* Free above descriptor from the buffer pool : the oldest descriptor
           in the pool */
        if( IX_DMA_DM_SUCCESS != ixDmaAccDescriptorFree(descriptorPtr) )
        {
            /* Log error message in debugging mode */
            ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            		   IX_OSAL_LOG_DEV_STDERR,
                       "\nixDmaTransferDoneCallback : descriptor free failed.",
                       0,0,0,0,0,0);
            status = IX_DMA_FAIL;
            /* Call the callback with the Error Condition returned by the NPE */
            descriptorPtr->pDmaCallback( status );
            return;
        }
    }
    else
    {
        /* Increment counter for Q descriptor address received as NULL
         */
        dmaStats.qDescAddrInvalidCnt++;
        /* Log error message in debugging mode */
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDERR,
                   "\nixDmaTransferDoneCallback : Illegal descriptor returned "
                   "by DMA Done Q.",
                   0,0,0,0,0,0);
        status = IX_DMA_FAIL;
    }
    /* Call the callback with the Error Condition returned by the NPE */
    descriptorPtr->pDmaCallback( status );
    return;
} /* end of ixDmaTransferDoneCallback */

/***********************************************************************
 * @fn ixDmaAccParamsValidate
 * @param None
 * @brief Validate parameters provided for Dma transfer
 * @return IX_SUCCESS : Validation succesful
 * @return IX_FAIL    : Validation failed
 ***********************************************************************/
IxDmaReturnStatus
ixDmaAccParamsValidate(
                        UINT32 sourceAddr,
                        UINT32 destinationAddr,
                        UINT16 transferLength,
                        IxDmaTransferMode transferMode,
                        IxDmaAddressingMode addressingMode,
                        IxDmaTransferWidth transferWidth)
{
    /* Check : source address is word aligned
     *         A mask of value Hex 0x03 (binary 0x0011)
     *         is used to determine if Destination address is
     *         a multiple of 4.
     *         This must be true for ByteSwap or Byte Reverse Transfer Mode
     *         to be valid.
     */
    if ( (0x00 != (sourceAddr & 0x03)) &&
         ( (IX_DMA_COPY_BYTE_SWAP == transferMode)||
           (IX_DMA_COPY_REVERSE == transferMode) ) )
    {
        /* Log error message in debugging mode */
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDERR,
        		   "\nixDmaAccParamsValidate : Dma Source address is not "
        		   "aligned for Byte Swap or Byte Reverse Modes.",
        		   0,0,0,0,0,0);
       return (IX_DMA_FAIL);
    }  /* end of if(sourceAddr) */

    /* Check : destination address word aligned
     *         A mask of value Hex 0x03 (binary 0x0011)
     *         is used to determine if Destination address is
     *         a multiple of 4.
     *         This must be true for ByteSwap or Byte Reverse Transfer Mode
     *         to be valid.
     */
    if ( ( 0x00 != (destinationAddr & 0x03)) &&
         ( (IX_DMA_COPY_BYTE_SWAP == transferMode)||
           (IX_DMA_COPY_REVERSE == transferMode) ) )
    {
        /* Log error message in debugging mode */
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDERR,
        		   "\nixDmaAccParamsValidate : Dma Destination address not "
        		   "word aligned for Byte Swap or Byte Reverse Modes.",
        		   0,0,0,0,0,0);
        return (IX_DMA_FAIL);
    }  /* end of if(destinationAddr) */

    /* Check : Addressing Mode validation
    Note : Fixed Source to Fixed Destination is not valid
    */

    if ( addressingMode >= IX_DMA_FIX_SRC_FIX_DST )
    {
        /* Log error message in debugging mode */
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDERR,
                   "\nixDmaAccParamsValidate : Dma addressing mode not valid.",
                   0,0,0,0,0,0);
        return IX_DMA_INVALID_ADDRESS_MODE;
    }  /* end of if(AddresingMode) */

    /* Check : Transfer Mode validation */
    if ( transferMode >= IX_DMA_TRANSFER_MODE_INVALID )
    {
        /* Log error message in debugging mode */
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDERR,
                   "\nixDmaAccParamsValidate : Dma transfer mode not valid.",
                   0,0,0,0,0,0);
        return IX_DMA_INVALID_TRANSFER_MODE;
    }  /* end of if(transferMode) */

    /* Check : transfer length range check*/
    if (0 == transferLength)
    {
        /* Log error message in debugging mode */
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDERR,
                   "\nixDmaAccParamsValidate : Invalid Dma Transfer length.",
                   0,0,0,0,0,0);
       return IX_DMA_INVALID_TRANSFER_LENGTH;
    } /* end of if(transferLength) */

    /* Check : Transfer Width Validation
     *         A mask of value Hex 0x03 (binary 0x0011)
     *         is used to determine if Transfer Length is
     *         a multiple of 4.
     *         This must be true for 32 bit Transfer Widths
     *         to be valid.
     */

    if ( ( (IX_DMA_32_SRC_32_DST == transferWidth)||
           (IX_DMA_32_SRC_16_DST == transferWidth)||
           (IX_DMA_32_SRC_8_DST == transferWidth)||
           (IX_DMA_16_SRC_32_DST == transferWidth)||
           (IX_DMA_8_SRC_32_DST == transferWidth)||
           (IX_DMA_32_SRC_BURST_DST == transferWidth)||
           (IX_DMA_BURST_SRC_32_DST == transferWidth) )
           && (0x00 != (transferLength & 0x03)) )
    {
        /* Log error message in debugging mode */
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDERR,
        		   "\nixDmaAccParamsValidate : Dma transfer length is not "
        		   "4 byte multiple for 32 bit transfer mode.",
        		   0,0,0,0,0,0);
        return IX_DMA_INVALID_TRANSFER_WIDTH;
    } /* end of if(transferWidth) */

    /* Check : Transfer width validation
     *         A mask of value Hex 0x01 (binary 0x0001)
     *         is used to determine if Transfer Length is
     *         a multiple of 2.
     *         This must be true for 16 bit Transfer Widths
     *         to be valid.
     */

    if ( ( (IX_DMA_16_SRC_16_DST == transferWidth)||
           (IX_DMA_16_SRC_8_DST == transferWidth)||
           (IX_DMA_8_SRC_16_DST == transferWidth)||
           (IX_DMA_16_SRC_BURST_DST == transferWidth)||
           (IX_DMA_BURST_SRC_16_DST == transferWidth) )
           && (0x0 != (transferLength & 0x1)) )
    {
        /* Log error message in debugging mode */
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDERR,
        		   "\nixDmaAccParamsValidate : Dma transfer length is not "
        		   "2 byte multiple for 16 bit transfer mode.",
        		   0,0,0,0,0,0);
        return IX_DMA_INVALID_TRANSFER_WIDTH;
    } /* end of if(transferWidth) */

    /* Check : Transfer width validation
     *         Reject the following illegal case
     *         AddressMode=IncSrc_FixedDest and Transfer Width == xx_Burst
     */

    if ( ( (IX_DMA_32_SRC_BURST_DST == transferWidth)||
           (IX_DMA_16_SRC_BURST_DST == transferWidth)||
           (IX_DMA_8_SRC_BURST_DST == transferWidth)||
           (IX_DMA_BURST_SRC_BURST_DST == transferWidth) )
           && (IX_DMA_INC_SRC_FIX_DST==addressingMode) )
    {
        /* Log error message in debugging mode */
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDERR,
        		   "\nixDmaAccParamsValidate : Burst destination not supported "
        		   "in Inc Source to Fixed Dest Address Mode.",
        		   0,0,0,0,0,0);
        return IX_DMA_INVALID_TRANSFER_WIDTH;
    } /* end of if(transferWidth) */
    /* Check : Transfer width validation
     *         Reject the following illegal case
     *         AddressMode=FixedSrc_IncDest and Transfer Width == Burst_xx
     */

    if ( ( (IX_DMA_BURST_SRC_32_DST == transferWidth)||
           (IX_DMA_BURST_SRC_16_DST == transferWidth)||
           (IX_DMA_BURST_SRC_8_DST == transferWidth)||
           (IX_DMA_BURST_SRC_BURST_DST == transferWidth) )
           && (IX_DMA_FIX_SRC_INC_DST==addressingMode) )
    {
        /* Log error message in debugging mode */
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDERR,
        		   "\nixDmaAccParamsValidate : Burst source not supported "
        		   "in Fixed Src and Inc Dest Address Mode.",
        		   0,0,0,0,0,0);
        return IX_DMA_INVALID_TRANSFER_WIDTH;
    } /* end of if(transferWidth) */
    return (IX_DMA_SUCCESS);
} /* end of function ixDmaAccParamsValidate */

