/**
 * @file IxParityENAccMain.c
 *
 * @author Intel Corporation
 * @date 26 July 2004
 *
 * @brief  Source file for the Main sub-component of the IXP400 Parity 
 * Error Notifier access component.
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

#if defined(__ixp46X) || defined(__ixp43X)

/*
 * System include files
 */
#include "IxOsal.h"

/*
 * User defined include files
 */
#include "IxParityENAccMain.h"
#include "IxParityENAccNpePE.h"
#include "IxParityENAccPbcPE.h"
#include "IxParityENAccScpPE.h"
#include "IxParityENAccAqmPE.h"
#include "IxParityENAccMcuPE.h"
#include "IxParityENAccEbcPE.h"
#include "IxParityENAccIcE.h"
#include "IxParityENAccPmuE.h"
#include "IxFeatureCtrl.h"

/*
 * Variable declarations global to the Main sub-module
 */

/* IxParityENAcc module initialisation status */
static IxParityENAccInitStatus ixParityENAccInitStatus = FALSE;

/* Client callback routine */
static IxParityENAccCallback   ixParityENAccClientCb = NULL;

/* Fused-Out Modules */
static IxParityENAccFusedModules ixParityENAccFusedModules = 0;

/* Local instance of the configuration status of the hardware blocks */
static IxParityENAccParityConfigStatus ixParityENAccParityConfigStatus =
{
    /* NPE-A Configuration */
    { IX_PARITYENACC_DISABLE, IX_PARITYENACC_EVEN_PARITY }, 
    /* NPE-B Configuration */
    { IX_PARITYENACC_DISABLE, IX_PARITYENACC_EVEN_PARITY },
    /* NPE-C Configuration */
    { IX_PARITYENACC_DISABLE, IX_PARITYENACC_EVEN_PARITY },
    /* MCU ECC Configuration */
    { IX_PARITYENACC_DISABLE, IX_PARITYENACC_DISABLE, IX_PARITYENACC_DISABLE },
    /* SWCP Configuration */
    IX_PARITYENACC_DISABLE,
    /* AQM Configuration */
    IX_PARITYENACC_DISABLE,
    /* PBC Configuration */
    { IX_PARITYENACC_DISABLE, IX_PARITYENACC_DISABLE },
    /* EBC Configuration */
    {
        IX_PARITYENACC_DISABLE, IX_PARITYENACC_DISABLE,
        IX_PARITYENACC_DISABLE, IX_PARITYENACC_DISABLE,
        IX_PARITYENACC_DISABLE, IX_PARITYENACC_DISABLE,
        IX_PARITYENACC_DISABLE, IX_PARITYENACC_DISABLE,
        IX_PARITYENACC_DISABLE, IX_PARITYENACC_EVEN_PARITY
    }
};

static IxParityENAccPEParityErrorStats ixParityENAccPEParityErrorStats =
{
    { 0,0,0 },  /* NPE Stats */
    { 0,0,0 },  /* MCU Stats */
    { 0,0 },    /* PBC Stats */
    { 0,0 },    /* EBC Stats */
    0,          /* SWCP Stats */
    0           /* AQM Stats */
};

/*
 * Local functions declarations
 */
IX_STATUS
ixParityENAccConfigInit (void);

IX_STATUS
ixParityENAccModulesInit (void);

void 
ixParityENAccInvokeClientCallback (UINT32 irqNum);

void
ixParityENAccResetCommonStats(void);

/*
 * Local functions definitions
 */

/* 
 * Includes the fix to handle the NPE configuration hang issue 
 * with ethAcc END driver enabled in the bootrom and/or vxWorks.st 
 * which generates NPE related  parity errors prior to the NPE
 * parity detection configuration.
 */
/* Function to check and invoke the Client Callback Routine */
void ixParityENAccInvokeClientCallback (UINT32 irqNum)
{
    if ((IxParityENAccCallback)NULL != ixParityENAccClientCb)
    {
        if (TRUE != ixParityENAccInitStatus)
        {
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR,
                IX_OSAL_LOG_DEV_STDERR,
                "Initialisation is not yet complete (Status - %d)\n"
                "Interrupt #0x%x is being disabled", 
                ixParityENAccInitStatus,irqNum,0,0,0,0);
            ixOsalIrqDisable(irqNum);
        } 
        else
        {
            (*ixParityENAccClientCb)();
        } /* end of if */
    }
    else
    {
        IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDERR,
            "NULL Client Callback!!!\n"
            "Interrupt #0x%x is being disabled", irqNum,0,0,0,0,0);
        ixOsalIrqDisable(irqNum);
    } /* end of if */
} /* end of ixParityENAccInvokeClientCallback () function */

IX_STATUS
ixParityENAccConfigInit (void)
{
    /* Check for IXP46X || IXP43X device */
    if ( (IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X != ixFeatureCtrlDeviceRead()) && 
       (IX_FEATURE_CTRL_DEVICE_TYPE_IXP43X != ixFeatureCtrlDeviceRead()) )
    {
        IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
            "ixParityENAccConfigInit(): "
            "Parity Error Notifier not supported on this device\n", 0,0,0,0,0,0);
        return IX_FAIL;
    } /* end of if */

    if (IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X == ixFeatureCtrlDeviceRead())
    {
       /* Find the fused-out module status
        *
        * NOTE: For IXP46X, feature Control uses 
        * #define IX_FEATURECTRL_ECC_TIMESYNC for the ECC feature 
        * of SDRAM Controller and Timesync.
        */
       if (IX_FEATURE_CTRL_COMPONENT_DISABLED == 
          ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ECC_TIMESYNC))
       {
          IXP400_PARITYENACC_VAL_BIT_SET(ixParityENAccFusedModules,
             IXP400_PARITYENACC_FUSED_MODULE_MCU_ECC);
       } /* end of if */
    }
    else    
    {
       /* Find the fused-out module status
        *
        * NOTE: For IXP43X, feature Control uses #define IX_FEATURECTRL_ECC
        * for the ECC feature of SDRAM Controller.
        */
       if (IX_FEATURE_CTRL_COMPONENT_DISABLED == 
          ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ECC))
       {
          IXP400_PARITYENACC_VAL_BIT_SET(ixParityENAccFusedModules,
             IXP400_PARITYENACC_FUSED_MODULE_MCU_ECC);
       } /* end of if */
    }
 
    if (IX_FEATURE_CTRL_COMPONENT_DISABLED == 
        ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEA))
    {
        IXP400_PARITYENACC_VAL_BIT_SET(ixParityENAccFusedModules,
            IXP400_PARITYENACC_FUSED_MODULE_NPEA);
    } /* end of if */

    if (IX_FEATURE_CTRL_COMPONENT_DISABLED == 
        ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEB))
    {
        IXP400_PARITYENACC_VAL_BIT_SET(ixParityENAccFusedModules,
            IXP400_PARITYENACC_FUSED_MODULE_NPEB);
    } /* end of if */

    if (IX_FEATURE_CTRL_COMPONENT_DISABLED == 
        ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEC))
    {
        IXP400_PARITYENACC_VAL_BIT_SET(ixParityENAccFusedModules,
            IXP400_PARITYENACC_FUSED_MODULE_NPEC);
    } /* end of if */
    return IX_SUCCESS;
} /* end of ixParityENAccConfigInit() function */

IX_STATUS
ixParityENAccModulesInit (void)
{
    /*
     * This one is moved up in the Init chain due to the
     * interrupt disable function is being invoked by other
     * modules even before it was being initialised.
     */
    if (IX_SUCCESS != ixParityENAccIcEInit())
    {
        IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_DEBUG2, IX_OSAL_LOG_DEV_STDOUT,
            "ixParityENAccModulesInit(): INTC Configuration failed\n",0,0,0,0,0,0);
        return IX_FAIL;
    } /* end of if */

    if (IX_SUCCESS != ixParityENAccNpePEInit(ixNpePEInternalCallback))
    {
        IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_DEBUG2, IX_OSAL_LOG_DEV_STDOUT,
            "ixParityENAccModulesInit(): NPE Configuration failed\n",0,0,0,0,0,0);
        return IX_FAIL;
    } /* end of if */

    if (IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X == ixFeatureCtrlDeviceRead())
    {

        if (IX_SUCCESS != ixParityENAccSwcpPEInit(ixSwcpPEInternalCallback))
        {
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_DEBUG2, 
            IX_OSAL_LOG_DEV_STDOUT,
            "ixParityENAccModulesInit(): SWCP Configuration failed\n",0,0,0,
            0,0,0);
            return IX_FAIL;
        } /* end of if */

    }

    if (IX_SUCCESS != ixParityENAccAqmPEInit(ixAqmPEInternalCallback))
    {
        IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_DEBUG2, IX_OSAL_LOG_DEV_STDOUT,
            "ixParityENAccModulesInit(): AQM Configuration failed\n",0,0,0,0,0,0);
        return IX_FAIL;
    } /* end of if */

    if (IX_SUCCESS != ixParityENAccMcuPEInit(ixMcuPEInternalCallback))
    {
        IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_DEBUG2, IX_OSAL_LOG_DEV_STDOUT,
            "ixParityENAccModulesInit(): MCU Configuration failed\n",0,0,0,0,0,0);
        return IX_FAIL;
    } /* end of if */

    if (IX_SUCCESS != ixParityENAccPbcPEInit(ixPbcPEInternalCallback))
    {
        IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_DEBUG2, IX_OSAL_LOG_DEV_STDOUT,
            "ixParityENAccModulesInit(): PBC Configuration failed\n",0,0,0,0,0,0);
        return IX_FAIL;
    } /* end of if */

    if (IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X == ixFeatureCtrlDeviceRead())
    {
    
        if (IX_SUCCESS != ixParityENAccEbcPEInit(ixEbcPEInternalCallback))
        {
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_DEBUG2, 
            IX_OSAL_LOG_DEV_STDOUT,
            "ixParityENAccModulesInit(): EBC Configuration failed\n",0,0,
            0,0,0,0);
            return IX_FAIL;
        } /* end of if */

    }

    if (IX_SUCCESS != ixParityENAccPmuEInit())
    {
        IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_DEBUG2, IX_OSAL_LOG_DEV_STDOUT,
            "ixParityENAccModulesInit(): PMU Configuration failed\n",0,0,0,0,0,0);
        return IX_FAIL;
    } /* end of if */

    return IX_SUCCESS;
} /* end of ixParityENAccModulesInit() function */

void 
ixParityENAccResetCommonStats(void)
{
    /* Local instance of the configuration status of the hardware blocks */
    IxParityENAccParityConfigStatus ixTempParityENAccParityConfigStatus = 
    {
       /* NPE-A Configuration */
       { IX_PARITYENACC_DISABLE, IX_PARITYENACC_EVEN_PARITY },
       /* NPE-B Configuration */ 
       { IX_PARITYENACC_DISABLE, IX_PARITYENACC_EVEN_PARITY },
       /* NPE-C Configuration */ 
       { IX_PARITYENACC_DISABLE, IX_PARITYENACC_EVEN_PARITY },
       /* MCU ECC Configuration */
       { IX_PARITYENACC_DISABLE, IX_PARITYENACC_DISABLE, IX_PARITYENACC_DISABLE },
       /* SWCP Configuration */ 
       IX_PARITYENACC_DISABLE, 
       /* AQM Configuration */
       IX_PARITYENACC_DISABLE,
       /* PBC Configuration */
       { IX_PARITYENACC_DISABLE, IX_PARITYENACC_DISABLE },
       /* EBC Configuration */ 
       {
           IX_PARITYENACC_DISABLE, IX_PARITYENACC_DISABLE,
           IX_PARITYENACC_DISABLE, IX_PARITYENACC_DISABLE,
           IX_PARITYENACC_DISABLE, IX_PARITYENACC_DISABLE,
           IX_PARITYENACC_DISABLE, IX_PARITYENACC_DISABLE,
           IX_PARITYENACC_DISABLE, IX_PARITYENACC_EVEN_PARITY
       }
    };

    IxParityENAccPEParityErrorStats ixTempParityENAccPEParityErrorStats =
    {
        { 0,0,0 },  /* NPE Stats */
        { 0,0,0 },  /* MCU Stats */
        { 0,0 },    /* PBC Stats */
        { 0,0 },    /* EBC Stats */
        0,          /* SWCP Stats */
        0           /* AQM Stats */
    };

    /* Reset the parity config status to init time values */
    ixParityENAccParityConfigStatus = ixTempParityENAccParityConfigStatus;

    /* Reset the parity error statistics to init time values */
    ixParityENAccPEParityErrorStats = ixTempParityENAccPEParityErrorStats;
} /* end of ixParityENAccResetCommonStats() function */

void
ixMcuPEInternalCallback (UINT32 irqNum, IxParityENAccPEIsr isrAddr)
{
    IxParityENAccIcParityInterruptStatus ixIcEDummyStatus =
    { FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE };
    /*
     * Output the MCU interrupt information for debug purpose
     */
    IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "ixMcuPEInternalCallback(): MCU interrupt detected\n"
        "IRQ #:%x ISR Address:%p\n", irqNum, (UINT32)isrAddr, 0,0,0,0);

    /*
     * Add the Delay for Max Data/Prefetch Abort Exception to trigger
     * and complete before this interrupt service routine to avoid the
     * potential race conditions between the Exception Handler & ISR 
     *
     * The delay is introduced by a dummy read of one/more of the 
     * interrupt controller registers.
     */
    ixParityENAccIcInterruptStatusGet(&ixIcEDummyStatus);

    /* Inform the client application of the interrupt condition */
    ixParityENAccInvokeClientCallback(irqNum);
} /* end of ixMcuPEInternalCallback() function */

void
ixNpePEInternalCallback (UINT32 irqNum, IxParityENAccPEIsr isrAddr)
{
    /* Output the NPE interrupt information for debug purpose */
    IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "ixNpePEInternalCallback(): NPE interrupt detected\n"
        "IRQ #:%x ISR Address:%p\n", irqNum, (UINT32)isrAddr, 0,0,0,0);

    /* Signal the client application of the interrupt condition */
    ixParityENAccInvokeClientCallback(irqNum);
} /* end of ixNpePEInternalCallback() function */

void
ixAqmPEInternalCallback (UINT32 irqNum, IxParityENAccPEIsr isrAddr)
{
    /* Output the AQM interrupt information for debug purpose */
    IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "ixAqmPEInternalCallback(): AQM interrupt detected\n"
        "IRQ #:%x ISR Address:%p\n", irqNum, (UINT32)isrAddr, 0,0,0,0);

    /* Signal the client application of the interrupt condition */
    ixParityENAccInvokeClientCallback(irqNum);
} /* end of ixAqmPEInternalCallback() function */

void 
ixEbcPEInternalCallback (UINT32 irqNum, IxParityENAccPEIsr isrAddr)
{
    /* Output the EBC interrupt information for debug purpose */
    IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "ixEbcPEInternalCallback(): EBCs interrupt detected\n"
        "IRQ #:%x ISR Address:%p\n", irqNum, (UINT32)isrAddr, 0,0,0,0);

    /* Signal the client application of the interrupt condition */
    ixParityENAccInvokeClientCallback(irqNum);
} /* end of ixEbcPEInternalCallback() function */

void 
ixPbcPEInternalCallback (UINT32 irqNum, IxParityENAccPEIsr isrAddr)
{
    /* Output the PBC interrupt information for debug purpose */
    IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "ixPbcPEInternalCallback(): PBC interrupt detected\n"
        "IRQ #:%x ISR Address:%p\n", irqNum, (UINT32)isrAddr, 0,0,0,0);

    /* Signal the client application of the interrupt condition */
    ixParityENAccInvokeClientCallback(irqNum);
} /* end of ixPbcPEInternalCallback() function */

void 
ixSwcpPEInternalCallback (UINT32 irqNum, IxParityENAccPEIsr isrAddr)
{
    /* Output the SWCP interrupt information for debug purpose */
    IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, 
        "ixSwcpPEInternalCallback(): SWCP interrupt detected\n"
        "IRQ #:%x ISR Address:%p\n", irqNum, (UINT32)isrAddr, 0,0,0,0);

    /* Signal the client application of the interrupt condition */
    if (IX_PARITYENACC_ENABLE == ixParityENAccParityConfigStatus.swcpEnabled)
    {
        ixParityENAccInvokeClientCallback(irqNum);
    } /* end of if */
} /* end of ixSwcpPEInternalCallback() function */

IX_STATUS
ixParityENAccCheckNpeIdValidity(UINT32 npeId)
{

    if (npeId < IXP400_PARITYENACC_PE_NPE_A || 
        npeId > IXP400_PARITYENACC_PE_NPE_C)
    {                                                         
       return IX_PARITYENACC_INVALID_PARAMETERS;    
    }
    
    if (IX_FEATURE_CTRL_DEVICE_TYPE_IXP43X == ixFeatureCtrlDeviceRead ())
    {
        if (npeId == IXP400_PARITYENACC_PE_NPE_B) 
        {
            return IX_PARITYENACC_INVALID_PARAMETERS;    
        }
    }

    return IX_SUCCESS;

}

/*
 * -------------------------------------------------------------------------- *
 *                            Public API definitions                          *
 * -------------------------------------------------------------------------- *
 */

PUBLIC IxParityENAccStatus
ixParityENAccInit (void)
{
    /* Initialised before? */
    if (TRUE == ixParityENAccInitStatus)
    {

        IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "Already Initialised\n", 0,0,0,0,0,0);

        return IX_PARITYENACC_ALREADY_INITIALISED;
    } /* end of if */

    /* Initialise sub modules */
    if (IX_SUCCESS != ixParityENAccConfigInit())
    {
        IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "Config Init Failed\n", 0,0,0,0,0,0);
        return IX_PARITYENACC_OPERATION_FAILED;
    } /* end of if */

    /* Initialise sub modules */
    if (IX_SUCCESS != ixParityENAccModulesInit())
    {
        IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "Modules Init Failed\n", 0,0,0,0,0,0);
        return IX_PARITYENACC_OPERATION_FAILED;
    } /* end of if */

    ixParityENAccInitStatus = TRUE;
    return IX_PARITYENACC_SUCCESS;
} /* end of ixParityENAccInit() function */


PUBLIC IxParityENAccStatus
ixParityENAccCallbackRegister (IxParityENAccCallback parityErrNfyCallback)
{
    IxParityENAccParityConfigStatus nullCfgStatus;

    /* Not initialised before? */
    if (FALSE == ixParityENAccInitStatus)
    {
        return IX_PARITYENACC_NOT_INITIALISED;
    } /* end of if */

    /* Verify the callback */
    if ((IxParityENAccCallback)NULL == parityErrNfyCallback)
    {
        return IX_PARITYENACC_INVALID_PARAMETERS;
    } /* end of if */

    /* Clear off the contents to zeros */
    ixOsalMemSet((void *) &nullCfgStatus, 0, sizeof(IxParityENAccParityConfigStatus));

    /* 
     * Verify for the parity error detection disable state
     * after excluding the parity type.
     */

    nullCfgStatus.npeAConfig.parityOddEven = ixParityENAccParityConfigStatus.npeAConfig.parityOddEven;
    nullCfgStatus.npeBConfig.parityOddEven = ixParityENAccParityConfigStatus.npeBConfig.parityOddEven;
    nullCfgStatus.npeCConfig.parityOddEven = ixParityENAccParityConfigStatus.npeCConfig.parityOddEven;
    nullCfgStatus.ebcConfig.parityOddEven  = ixParityENAccParityConfigStatus.ebcConfig.parityOddEven;

    /* Parity detection disabled on all hardware modules? */
    if (0 != memcmp((const void *)&nullCfgStatus, 
                    (const void *)&ixParityENAccParityConfigStatus,
                     sizeof(ixParityENAccParityConfigStatus)))
    {
        return IX_PARITYENACC_OPERATION_FAILED;
    } /* end of if */

#ifndef NDEBUG
    if ((IxParityENAccCallback)NULL == ixParityENAccClientCb)
    {
        IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
              "ixParityENAccCallbackRegister(): New Callback:%p\n",
              (UINT32)parityErrNfyCallback, 0,0,0,0,0);
    }
    else
    {
        IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
              "ixParityENAccCallbackRegister(): Re-registered Callback:%p\n",
              (UINT32)parityErrNfyCallback, 0,0,0,0,0);
    } /* end of if */
#endif /* end of #ifndef NDEBUG */

    ixParityENAccClientCb = parityErrNfyCallback;
    return IX_PARITYENACC_SUCCESS;
} /* end of ixParityENAccCallbackRegister() function */


PUBLIC IxParityENAccStatus ixParityENAccParityNPEConfigReUpdate(UINT32 npeID)
{
    UINT32 npeFuseBit;
    IX_STATUS status;
    IxParityENAccNpePEConfigOption ixNpePDCfg;
    static UINT32 npeIDToFuseBitMapTable[]=
    {
  	IXP400_PARITYENACC_FUSED_MODULE_NPEA,
  	IXP400_PARITYENACC_FUSED_MODULE_NPEB,
  	IXP400_PARITYENACC_FUSED_MODULE_NPEC
    };  
   
    /* Check for valid NPE Id */ 
    status = ixParityENAccCheckNpeIdValidity (npeID);
    if (status != IX_SUCCESS)
    {

        IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "ixParityENAccParityNPEConfigReUpdate(): "
                "Invalid NPE ID\n",0,0,0,0,0,0);
        return status;

    }
  
    /* Not initialised before? */
    if (FALSE == ixParityENAccInitStatus)
    {
        return IX_PARITYENACC_NOT_INITIALISED;
    } /* end of if */
    npeFuseBit = npeIDToFuseBitMapTable[npeID];
    
    switch(npeID)
    {
    	case IXP400_PARITYENACC_PE_NPE_A:
    	    ixNpePDCfg.ideEnabled    = ixParityENAccParityConfigStatus.npeAConfig.ideEnabled;
          ixNpePDCfg.parityOddEven = ixParityENAccParityConfigStatus.npeAConfig.parityOddEven;
          break;
          
    case IXP400_PARITYENACC_PE_NPE_B:
    	    ixNpePDCfg.ideEnabled    = ixParityENAccParityConfigStatus.npeBConfig.ideEnabled;
          ixNpePDCfg.parityOddEven = ixParityENAccParityConfigStatus.npeBConfig.parityOddEven;
          break;
    
    case IXP400_PARITYENACC_PE_NPE_C:
    	    ixNpePDCfg.ideEnabled    = ixParityENAccParityConfigStatus.npeCConfig.ideEnabled;
          ixNpePDCfg.parityOddEven = ixParityENAccParityConfigStatus.npeCConfig.parityOddEven;
          break;
     
     default:
       break;
    }
                    
    if (FALSE == IXP400_PARITYENACC_VAL_BIT_CHECK(ixParityENAccFusedModules,
                     npeFuseBit) )
    {
       return ixParityENAccNpePEDetectionConfigure(
                              npeID,ixNpePDCfg);
    }
    return IX_PARITYENACC_OPERATION_FAILED;
}

PUBLIC IxParityENAccStatus
ixParityENAccParityDetectionConfigure (
    const IxParityENAccHWParityConfig *hwParityConfig)
{
    /* Local Variables */
    IxParityENAccNpePEConfigOption ixNpePDCfg;
    IxParityENAccMcuPEConfigOption ixMcuPDCfg;
    IxParityENAccAqmPEConfigOption ixAqmPDCfg;
    IxParityENAccEbcPEConfigOption ixEbcPDCfg;
    IxParityENAccSwcpPEConfigOption ixSwcpPDCfg;

    BOOL ixPbcPrevInitTgtDisableCurrInitOrTgtEnable  = FALSE;
    BOOL ixPbcPrevInitTgtEnableCurrInitAndTgtDisable = FALSE;
    BOOL ixPbcPrevInitEnableCurrInitDisable = FALSE;
    BOOL ixPbcPrevTgtEnableCurrTgtDisable   = FALSE;
    BOOL ixParityENAccIXP46XCompDisabled = FALSE;
    IX_STATUS ixReturnStatus = IX_SUCCESS;

    /* Not initialised before? */
    if (FALSE == ixParityENAccInitStatus)
    {
        return IX_PARITYENACC_NOT_INITIALISED;
    } /* end of if */

    if ((IxParityENAccHWParityConfig *) NULL == hwParityConfig)
    {
        return IX_PARITYENACC_INVALID_PARAMETERS;
    } /* end of if */

    /*
     * In IXP43X , sub components like NPE B, SWCP, EBC are not
     * present in silicon.Hence we use this variable to, not initialize
     * these sub components in IXP43X
     */ 
    if (IX_FEATURE_CTRL_DEVICE_TYPE_IXP43X == ixFeatureCtrlDeviceRead())
    {
        ixParityENAccIXP46XCompDisabled = TRUE;
    }

    /*
     * Change in parity type on NPE-A/B/C?
     */
    if (((hwParityConfig->npeAConfig.ideEnabled != 
         ixParityENAccParityConfigStatus.npeAConfig.ideEnabled) ||
         ((hwParityConfig->npeAConfig.parityOddEven != 
         ixParityENAccParityConfigStatus.npeAConfig.parityOddEven) &&
          (hwParityConfig->npeAConfig.ideEnabled == IX_PARITYENACC_ENABLE))) &&
        (FALSE == IXP400_PARITYENACC_VAL_BIT_CHECK(ixParityENAccFusedModules,
                     IXP400_PARITYENACC_FUSED_MODULE_NPEA)))
    {
        ixNpePDCfg.ideEnabled    = hwParityConfig->npeAConfig.ideEnabled;
        ixNpePDCfg.parityOddEven = hwParityConfig->npeAConfig.parityOddEven;
            
        if (IX_SUCCESS == ixParityENAccNpePEDetectionConfigure(
                              IXP400_PARITYENACC_PE_NPE_A,ixNpePDCfg))
        {
            ixParityENAccParityConfigStatus.npeAConfig.ideEnabled = 
                hwParityConfig->npeAConfig.ideEnabled;
            ixParityENAccParityConfigStatus.npeAConfig.parityOddEven =
                hwParityConfig->npeAConfig.parityOddEven;
        } /* end/else of if */
#ifndef NDEBUG
        else
        {
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "ixParityENAccParityDetectionConfigure(): "
                "NPE-A Configuration failed\n",0,0,0,0,0,0);
            ixReturnStatus = IX_FAIL;
        } /* end of if */
#endif /* end of #ifndef NDEBUG */
    } /* end of if */

    if (((hwParityConfig->npeBConfig.ideEnabled != 
         ixParityENAccParityConfigStatus.npeBConfig.ideEnabled) ||
         ((hwParityConfig->npeBConfig.parityOddEven != 
         ixParityENAccParityConfigStatus.npeBConfig.parityOddEven) &&
          (hwParityConfig->npeBConfig.ideEnabled == IX_PARITYENACC_ENABLE))) &&
         ( FALSE == ixParityENAccIXP46XCompDisabled ) &&
        (FALSE == IXP400_PARITYENACC_VAL_BIT_CHECK(ixParityENAccFusedModules,
                     IXP400_PARITYENACC_FUSED_MODULE_NPEB)))
    {
        ixNpePDCfg.ideEnabled    = hwParityConfig->npeBConfig.ideEnabled;
        ixNpePDCfg.parityOddEven = hwParityConfig->npeBConfig.parityOddEven;

        if (IX_SUCCESS == ixParityENAccNpePEDetectionConfigure(
                              IXP400_PARITYENACC_PE_NPE_B,ixNpePDCfg))
        {
            ixParityENAccParityConfigStatus.npeBConfig.ideEnabled = 
                hwParityConfig->npeBConfig.ideEnabled;
            ixParityENAccParityConfigStatus.npeBConfig.parityOddEven =
                hwParityConfig->npeBConfig.parityOddEven;
        } /* end/else of if */
#ifndef NDEBUG
        else
        {
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
                "ixParityENAccParityDetectionConfigure(): "
                "NPE-B Configuration failed\n",0,0,0,0,0,0);
            ixReturnStatus = IX_FAIL;
        } /* end of if */
#endif /* end of #ifndef NDEBUG */
    } /* end of if */

    if (((hwParityConfig->npeCConfig.ideEnabled != 
         ixParityENAccParityConfigStatus.npeCConfig.ideEnabled) ||
         ((hwParityConfig->npeCConfig.parityOddEven != 
         ixParityENAccParityConfigStatus.npeCConfig.parityOddEven) &&
          (hwParityConfig->npeCConfig.ideEnabled == IX_PARITYENACC_ENABLE))) &&
        (FALSE == IXP400_PARITYENACC_VAL_BIT_CHECK(ixParityENAccFusedModules,
                     IXP400_PARITYENACC_FUSED_MODULE_NPEC)))
    {
        ixNpePDCfg.ideEnabled    = hwParityConfig->npeCConfig.ideEnabled;
        ixNpePDCfg.parityOddEven = hwParityConfig->npeCConfig.parityOddEven;

        if (IX_SUCCESS == ixParityENAccNpePEDetectionConfigure(
                              IXP400_PARITYENACC_PE_NPE_C, ixNpePDCfg))
        {
            ixParityENAccParityConfigStatus.npeCConfig.ideEnabled = 
                hwParityConfig->npeCConfig.ideEnabled;
            ixParityENAccParityConfigStatus.npeCConfig.parityOddEven =
                hwParityConfig->npeCConfig.parityOddEven;
        } /* else/end of if */
#ifndef NDEBUG
        else
        {
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
                "ixParityENAccParityDetectionConfigure(): "
                "NPE-C Configuration failed\n",0,0,0,0,0,0);
            ixReturnStatus = IX_FAIL;
        } /* end of if */
#endif /* end of #ifndef NDEBUG */
    } /* end of if */
    
    /*
     * Change in configuration of MCU ECC? 
     */
    if (((hwParityConfig->mcuConfig.singlebitDetectEnabled != 
         ixParityENAccParityConfigStatus.mcuConfig.singlebitDetectEnabled) ||
         (hwParityConfig->mcuConfig.singlebitCorrectionEnabled !=
         ixParityENAccParityConfigStatus.mcuConfig.singlebitCorrectionEnabled) ||
         (hwParityConfig->mcuConfig.multibitDetectionEnabled !=
         ixParityENAccParityConfigStatus.mcuConfig.multibitDetectionEnabled)) &&
        (FALSE == IXP400_PARITYENACC_VAL_BIT_CHECK(ixParityENAccFusedModules,
                     IXP400_PARITYENACC_FUSED_MODULE_MCU_ECC)))
    {
        ixMcuPDCfg.singlebitDetectEnabled = 
            hwParityConfig->mcuConfig.singlebitDetectEnabled;
        ixMcuPDCfg.singlebitCorrectionEnabled =
            hwParityConfig->mcuConfig.singlebitCorrectionEnabled;
        ixMcuPDCfg.multibitDetectionEnabled = 
            hwParityConfig->mcuConfig.multibitDetectionEnabled;

        if (IX_SUCCESS == ixParityENAccMcuPEDetectionConfigure(ixMcuPDCfg))
        {
            ixParityENAccParityConfigStatus.mcuConfig.singlebitDetectEnabled =
                hwParityConfig->mcuConfig.singlebitDetectEnabled;
            ixParityENAccParityConfigStatus.mcuConfig.singlebitCorrectionEnabled =
                hwParityConfig->mcuConfig.singlebitCorrectionEnabled;
            ixParityENAccParityConfigStatus.mcuConfig.multibitDetectionEnabled =
                hwParityConfig->mcuConfig.multibitDetectionEnabled;
        } /* end/else of if */
#ifndef NDEBUG
        else
        {
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "ixParityENAccParityDetectionConfigure(): "
                "MCU ECC Configuration failed\n",0,0,0,0,0,0);
            ixReturnStatus = IX_FAIL;
        } /* end of if */
#endif /* end of #ifndef NDEBUG */
    } /* end of if */

    /*
     * Change in configuration of SWCP?
     */
    if ( (hwParityConfig->swcpEnabled != ixParityENAccParityConfigStatus.
         swcpEnabled )  && (FALSE == ixParityENAccIXP46XCompDisabled) )
    {
        ixSwcpPDCfg = hwParityConfig->swcpEnabled;
        if (IX_SUCCESS == ixParityENAccSwcpPEDetectionConfigure(ixSwcpPDCfg))
        {
            ixParityENAccParityConfigStatus.swcpEnabled = hwParityConfig->swcpEnabled;
        } /* end/else of if */
#ifndef NDEBUG
        else
        {
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "ixParityENAccParityDetectionConfigure(): "
                "SWCP Configuration failed\n",0,0,0,0,0,0);
            ixReturnStatus = IX_FAIL;
        } /* end of if */
#endif /* end of #ifndef NDEBUG */
    } /* end of if */

    /*
     * Change in configuration of AQM?
     */
    if (hwParityConfig->aqmEnabled != ixParityENAccParityConfigStatus.aqmEnabled)
    {
        ixAqmPDCfg = hwParityConfig->aqmEnabled;
        if (IX_SUCCESS == ixParityENAccAqmPEDetectionConfigure(ixAqmPDCfg))
        {
            ixParityENAccParityConfigStatus.aqmEnabled = hwParityConfig->aqmEnabled;
        } /* else/end of if */
#ifndef NDEBUG
        else
        {
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "ixParityENAccParityDetectionConfigure(): "
                "SWCP Configuration failed\n",0,0,0,0,0,0);
            ixReturnStatus = IX_FAIL;
        } /* end of if */
#endif /* end of #ifndef NDEBUG */
    } /* end of if */

    /* 
     * Change in the configuration of PCI Bus Controller
     * 
     * Send the configuration request only when 
     * a) previously both Initiator and Target interfaces are both disabled 
     *    and now both or one of them is to be enabled
     * b) previously both Initiator and Target interfaces are both enabled
     *    and now both of them to be disabled
     * c) previously one of them enabled and same is to be disabled
     */
    ixPbcPrevInitTgtDisableCurrInitOrTgtEnable = 
        (((ixParityENAccParityConfigStatus.pbcConfig.pbcInitiatorEnabled ==
                                         IX_PARITYENACC_DISABLE) &&
          (ixParityENAccParityConfigStatus.pbcConfig.pbcTargetEnabled ==
                                         IX_PARITYENACC_DISABLE)) &&
         ((hwParityConfig->pbcConfig.pbcInitiatorEnabled == 
                                         IX_PARITYENACC_ENABLE) ||
          (hwParityConfig->pbcConfig.pbcTargetEnabled == 
                                         IX_PARITYENACC_ENABLE)));

    ixPbcPrevInitTgtEnableCurrInitAndTgtDisable = 
    (((ixParityENAccParityConfigStatus.pbcConfig.pbcInitiatorEnabled ==
                                         IX_PARITYENACC_ENABLE) &&
          (ixParityENAccParityConfigStatus.pbcConfig.pbcTargetEnabled ==
                                         IX_PARITYENACC_ENABLE)) &&
         ((hwParityConfig->pbcConfig.pbcInitiatorEnabled == 
                                         IX_PARITYENACC_DISABLE) &&
          (hwParityConfig->pbcConfig.pbcTargetEnabled == 
                                         IX_PARITYENACC_DISABLE)));

    ixPbcPrevInitEnableCurrInitDisable = 
        (((ixParityENAccParityConfigStatus.pbcConfig.pbcInitiatorEnabled ==
                                         IX_PARITYENACC_ENABLE) &&
          (ixParityENAccParityConfigStatus.pbcConfig.pbcTargetEnabled ==
                                         IX_PARITYENACC_DISABLE)) &&
         ((hwParityConfig->pbcConfig.pbcInitiatorEnabled == 
                                         IX_PARITYENACC_DISABLE) &&
          (hwParityConfig->pbcConfig.pbcTargetEnabled == 
                                         IX_PARITYENACC_DISABLE)));

    ixPbcPrevTgtEnableCurrTgtDisable = 
        (((ixParityENAccParityConfigStatus.pbcConfig.pbcInitiatorEnabled ==
                                         IX_PARITYENACC_DISABLE) &&
          (ixParityENAccParityConfigStatus.pbcConfig.pbcTargetEnabled ==
                                         IX_PARITYENACC_ENABLE)) &&
         ((hwParityConfig->pbcConfig.pbcInitiatorEnabled == 
                                         IX_PARITYENACC_DISABLE) &&
          (hwParityConfig->pbcConfig.pbcTargetEnabled == 
                                         IX_PARITYENACC_DISABLE)));

    if  (FALSE == IXP400_PARITYENACC_VAL_BIT_CHECK(ixParityENAccFusedModules,
                     IXP400_PARITYENACC_FUSED_MODULE_PBC))
    {
        if ((TRUE == ixPbcPrevInitTgtEnableCurrInitAndTgtDisable) ||
            (TRUE == ixPbcPrevInitEnableCurrInitDisable)          ||
            (TRUE == ixPbcPrevTgtEnableCurrTgtDisable)            ||
            (TRUE == ixPbcPrevInitTgtDisableCurrInitOrTgtEnable))
        {
            IxParityENAccPEConfigOption pbcConfigOption = 
                (TRUE == ixPbcPrevInitTgtDisableCurrInitOrTgtEnable) ? 
                    IXP400_PARITYENACC_PE_ENABLE : IXP400_PARITYENACC_PE_DISABLE;
        
            if (IX_SUCCESS == ixParityENAccPbcPEDetectionConfigure(pbcConfigOption))
            {
                ixParityENAccParityConfigStatus.pbcConfig.pbcInitiatorEnabled =
                    hwParityConfig->pbcConfig.pbcInitiatorEnabled;
                ixParityENAccParityConfigStatus.pbcConfig.pbcTargetEnabled = 
                    hwParityConfig->pbcConfig.pbcTargetEnabled;
            } /* else/end of if */
#ifndef NDEBUG
            else
            {
                IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
                    "ixParityENAccParityDetectionConfigure(): "
                    "PBC Configuration failed\n",0,0,0,0,0,0);
                ixReturnStatus = IX_FAIL;
            } /* end of if */
#endif /* end of #ifndef NDEBUG */
        } /* No need to send the configuration request but update required */
        else
        {       
            ixParityENAccParityConfigStatus.pbcConfig.pbcInitiatorEnabled =
                hwParityConfig->pbcConfig.pbcInitiatorEnabled;
            ixParityENAccParityConfigStatus.pbcConfig.pbcTargetEnabled = 
                hwParityConfig->pbcConfig.pbcTargetEnabled;
        } /* end of if */
    } /* end of if */

    /*
     * Change in the configuration of Expansion Bus Controller
     */
    if ((hwParityConfig->ebcConfig.ebcCs0Enabled != 
        ixParityENAccParityConfigStatus.ebcConfig.ebcCs0Enabled) ||
        ((hwParityConfig->ebcConfig.parityOddEven != 
          ixParityENAccParityConfigStatus.ebcConfig.parityOddEven) &&
         ( FALSE == ixParityENAccIXP46XCompDisabled ) &&
         (ixParityENAccParityConfigStatus.ebcConfig.ebcCs0Enabled ==
         IX_PARITYENACC_ENABLE)))
    {
        ixEbcPDCfg.ebcCsExtSource = IXP400_PARITYENACC_PE_EBC_CS;
        ixEbcPDCfg.ebcInOrOutbound.ebcCsEnabled = 
            hwParityConfig->ebcConfig.ebcCs0Enabled;
        ixEbcPDCfg.ebcCsId = IXP400_PARITYENACC_PE_EBC_CHIPSEL0;
        ixEbcPDCfg.parityOddEven = hwParityConfig->ebcConfig.parityOddEven;
    
        if (IX_SUCCESS == ixParityENAccEbcPEDetectionConfigure(ixEbcPDCfg))
        {
            ixParityENAccParityConfigStatus.ebcConfig.ebcCs0Enabled =
                hwParityConfig->ebcConfig.ebcCs0Enabled;
            ixParityENAccParityConfigStatus.ebcConfig.parityOddEven = 
                hwParityConfig->ebcConfig.parityOddEven;
        } /* else/end of if */
#ifndef NDEBUG
        else
        {
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "ixParityENAccParityDetectionConfigure(): "
                "EBC Configuration of CS0 failed\n",0,0,0,0,0,0);
            ixReturnStatus = IX_FAIL;
        } /* end of if */
#endif /* end of #ifndef NDEBUG */
    } /* end of if */

    if ((hwParityConfig->ebcConfig.ebcCs1Enabled != 
        ixParityENAccParityConfigStatus.ebcConfig.ebcCs1Enabled) ||
        ((hwParityConfig->ebcConfig.parityOddEven != 
          ixParityENAccParityConfigStatus.ebcConfig.parityOddEven) &&
         ( FALSE == ixParityENAccIXP46XCompDisabled ) &&
         (hwParityConfig->ebcConfig.ebcCs1Enabled == IX_PARITYENACC_ENABLE)))
    {
        ixEbcPDCfg.ebcCsExtSource = IXP400_PARITYENACC_PE_EBC_CS;
        ixEbcPDCfg.ebcInOrOutbound.ebcCsEnabled = 
            hwParityConfig->ebcConfig.ebcCs1Enabled;
        ixEbcPDCfg.ebcCsId = IXP400_PARITYENACC_PE_EBC_CHIPSEL1;
        ixEbcPDCfg.parityOddEven = hwParityConfig->ebcConfig.parityOddEven;
    
        if (IX_SUCCESS == ixParityENAccEbcPEDetectionConfigure(ixEbcPDCfg))
        {
            ixParityENAccParityConfigStatus.ebcConfig.ebcCs1Enabled =
                hwParityConfig->ebcConfig.ebcCs1Enabled;
            ixParityENAccParityConfigStatus.ebcConfig.parityOddEven = 
                hwParityConfig->ebcConfig.parityOddEven;
        } /* else/end of if */
#ifndef NDEBUG
        else
        {
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "ixParityENAccParityDetectionConfigure(): "
                "EBC Configuration of CS1 failed\n",0,0,0,0,0,0);
            ixReturnStatus = IX_FAIL;
        } /* end of if */
#endif /* end of #ifndef NDEBUG */
    } /* end of if */

    if ((hwParityConfig->ebcConfig.ebcCs2Enabled != 
        ixParityENAccParityConfigStatus.ebcConfig.ebcCs2Enabled) ||
        ((hwParityConfig->ebcConfig.parityOddEven != 
          ixParityENAccParityConfigStatus.ebcConfig.parityOddEven) &&
         ( FALSE == ixParityENAccIXP46XCompDisabled ) &&
         (hwParityConfig->ebcConfig.ebcCs2Enabled == IX_PARITYENACC_ENABLE)))
    {
        ixEbcPDCfg.ebcCsExtSource = IXP400_PARITYENACC_PE_EBC_CS;
        ixEbcPDCfg.ebcInOrOutbound.ebcCsEnabled = 
            hwParityConfig->ebcConfig.ebcCs2Enabled;
        ixEbcPDCfg.ebcCsId = IXP400_PARITYENACC_PE_EBC_CHIPSEL2;
        ixEbcPDCfg.parityOddEven = hwParityConfig->ebcConfig.parityOddEven;
    
        if (IX_SUCCESS == ixParityENAccEbcPEDetectionConfigure(ixEbcPDCfg))
        {
            ixParityENAccParityConfigStatus.ebcConfig.ebcCs2Enabled =
                hwParityConfig->ebcConfig.ebcCs2Enabled;
            ixParityENAccParityConfigStatus.ebcConfig.parityOddEven = 
                hwParityConfig->ebcConfig.parityOddEven;
        } /* else/end of if */
#ifndef NDEBUG
        else
        {
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "ixParityENAccParityDetectionConfigure(): "
                "EBC Configuration of CS2 failed\n",0,0,0,0,0,0);
            ixReturnStatus = IX_FAIL;
        } /* end of if */
#endif /* end of #ifndef NDEBUG */
    } /* end of if */

    if ((hwParityConfig->ebcConfig.ebcCs3Enabled != 
        ixParityENAccParityConfigStatus.ebcConfig.ebcCs3Enabled) ||
        ((hwParityConfig->ebcConfig.parityOddEven != 
          ixParityENAccParityConfigStatus.ebcConfig.parityOddEven) &&
         ( FALSE == ixParityENAccIXP46XCompDisabled ) &&
         (hwParityConfig->ebcConfig.ebcCs3Enabled == IX_PARITYENACC_ENABLE)))
    {
        ixEbcPDCfg.ebcCsExtSource = IXP400_PARITYENACC_PE_EBC_CS;
        ixEbcPDCfg.ebcInOrOutbound.ebcCsEnabled = 
            hwParityConfig->ebcConfig.ebcCs3Enabled;
        ixEbcPDCfg.ebcCsId = IXP400_PARITYENACC_PE_EBC_CHIPSEL3;
        ixEbcPDCfg.parityOddEven = hwParityConfig->ebcConfig.parityOddEven;
    
        if (IX_SUCCESS == ixParityENAccEbcPEDetectionConfigure(ixEbcPDCfg))
        {
            ixParityENAccParityConfigStatus.ebcConfig.ebcCs3Enabled =
                hwParityConfig->ebcConfig.ebcCs3Enabled;
            ixParityENAccParityConfigStatus.ebcConfig.parityOddEven = 
                hwParityConfig->ebcConfig.parityOddEven;
        } /* else/end of if */
#ifndef NDEBUG
        else
        {
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "ixParityENAccParityDetectionConfigure(): "
                "EBC Configuration of CS3 failed\n",0,0,0,0,0,0);
            ixReturnStatus = IX_FAIL;
        } /* end of if */
#endif /* end of #ifndef NDEBUG */
    } /* end of if */

    if ((hwParityConfig->ebcConfig.ebcCs4Enabled != 
        ixParityENAccParityConfigStatus.ebcConfig.ebcCs4Enabled) ||
        ((hwParityConfig->ebcConfig.parityOddEven != 
          ixParityENAccParityConfigStatus.ebcConfig.parityOddEven) &&
         ( FALSE == ixParityENAccIXP46XCompDisabled ) &&
         (hwParityConfig->ebcConfig.ebcCs4Enabled == IX_PARITYENACC_ENABLE)))
    {
        ixEbcPDCfg.ebcCsExtSource = IXP400_PARITYENACC_PE_EBC_CS;
        ixEbcPDCfg.ebcInOrOutbound.ebcCsEnabled = 
            hwParityConfig->ebcConfig.ebcCs4Enabled;
        ixEbcPDCfg.ebcCsId = IXP400_PARITYENACC_PE_EBC_CHIPSEL4;
        ixEbcPDCfg.parityOddEven = hwParityConfig->ebcConfig.parityOddEven;
    
        if (IX_SUCCESS == ixParityENAccEbcPEDetectionConfigure(ixEbcPDCfg))
        {
            ixParityENAccParityConfigStatus.ebcConfig.ebcCs4Enabled =
                hwParityConfig->ebcConfig.ebcCs4Enabled;
            ixParityENAccParityConfigStatus.ebcConfig.parityOddEven = 
                hwParityConfig->ebcConfig.parityOddEven;
        } /* else/end of if */
#ifndef NDEBUG
        else
        {
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "ixParityENAccParityDetectionConfigure(): "
                "EBC Configuration of CS4 failed\n",0,0,0,0,0,0);
            ixReturnStatus = IX_FAIL;
        } /* end of if */
#endif /* end of #ifndef NDEBUG */
    } /* end of if */

    if ((hwParityConfig->ebcConfig.ebcCs5Enabled != 
        ixParityENAccParityConfigStatus.ebcConfig.ebcCs5Enabled) ||
        ((hwParityConfig->ebcConfig.parityOddEven != 
          ixParityENAccParityConfigStatus.ebcConfig.parityOddEven) &&
         ( FALSE == ixParityENAccIXP46XCompDisabled ) &&
         (hwParityConfig->ebcConfig.ebcCs5Enabled == IX_PARITYENACC_ENABLE)))
    {
        ixEbcPDCfg.ebcCsExtSource = IXP400_PARITYENACC_PE_EBC_CS;
        ixEbcPDCfg.ebcInOrOutbound.ebcCsEnabled = 
            hwParityConfig->ebcConfig.ebcCs5Enabled;
        ixEbcPDCfg.ebcCsId = IXP400_PARITYENACC_PE_EBC_CHIPSEL5;
        ixEbcPDCfg.parityOddEven = hwParityConfig->ebcConfig.parityOddEven;
    
        if (IX_SUCCESS == ixParityENAccEbcPEDetectionConfigure(ixEbcPDCfg))
        {
            ixParityENAccParityConfigStatus.ebcConfig.ebcCs5Enabled =
                hwParityConfig->ebcConfig.ebcCs5Enabled;
            ixParityENAccParityConfigStatus.ebcConfig.parityOddEven = 
                hwParityConfig->ebcConfig.parityOddEven;
        } /* else/end of if */
#ifndef NDEBUG
        else
        {
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "ixParityENAccParityDetectionConfigure(): "
                "EBC Configuration of CS5 failed\n",0,0,0,0,0,0);
            ixReturnStatus = IX_FAIL;
        } /* end of if */
#endif /* end of #ifndef NDEBUG */
    } /* end of if */

    if ((hwParityConfig->ebcConfig.ebcCs6Enabled != 
        ixParityENAccParityConfigStatus.ebcConfig.ebcCs6Enabled) ||
        ((hwParityConfig->ebcConfig.parityOddEven != 
          ixParityENAccParityConfigStatus.ebcConfig.parityOddEven) &&
         ( FALSE == ixParityENAccIXP46XCompDisabled ) &&
         (hwParityConfig->ebcConfig.ebcCs6Enabled == IX_PARITYENACC_ENABLE)))
    {
        ixEbcPDCfg.ebcCsExtSource = IXP400_PARITYENACC_PE_EBC_CS;
        ixEbcPDCfg.ebcInOrOutbound.ebcCsEnabled = 
            hwParityConfig->ebcConfig.ebcCs6Enabled;
        ixEbcPDCfg.ebcCsId = IXP400_PARITYENACC_PE_EBC_CHIPSEL6;
        ixEbcPDCfg.parityOddEven = hwParityConfig->ebcConfig.parityOddEven;
    
        if (IX_SUCCESS == ixParityENAccEbcPEDetectionConfigure(ixEbcPDCfg))
        {
            ixParityENAccParityConfigStatus.ebcConfig.ebcCs6Enabled =
                hwParityConfig->ebcConfig.ebcCs6Enabled;
            ixParityENAccParityConfigStatus.ebcConfig.parityOddEven = 
                hwParityConfig->ebcConfig.parityOddEven;
        } /* else/end of if */
#ifndef NDEBUG
        else
        {
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "ixParityENAccParityDetectionConfigure(): "
                "EBC Configuration of CS6 failed\n",0,0,0,0,0,0);
            ixReturnStatus = IX_FAIL;
        } /* end of if */
#endif /* end of #ifndef NDEBUG */
    } /* end of if */

    if ((hwParityConfig->ebcConfig.ebcCs7Enabled != 
        ixParityENAccParityConfigStatus.ebcConfig.ebcCs7Enabled) ||
        ((hwParityConfig->ebcConfig.parityOddEven != 
          ixParityENAccParityConfigStatus.ebcConfig.parityOddEven) &&
         ( FALSE == ixParityENAccIXP46XCompDisabled ) &&
         (hwParityConfig->ebcConfig.ebcCs7Enabled == IX_PARITYENACC_ENABLE)))
    {
        ixEbcPDCfg.ebcCsExtSource = IXP400_PARITYENACC_PE_EBC_CS;
        ixEbcPDCfg.ebcInOrOutbound.ebcCsEnabled = 
            hwParityConfig->ebcConfig.ebcCs7Enabled;
        ixEbcPDCfg.ebcCsId = IXP400_PARITYENACC_PE_EBC_CHIPSEL7;
        ixEbcPDCfg.parityOddEven = hwParityConfig->ebcConfig.parityOddEven;
    
        if (IX_SUCCESS == ixParityENAccEbcPEDetectionConfigure(ixEbcPDCfg))
        {
            ixParityENAccParityConfigStatus.ebcConfig.ebcCs7Enabled =
                hwParityConfig->ebcConfig.ebcCs7Enabled;
            ixParityENAccParityConfigStatus.ebcConfig.parityOddEven = 
                hwParityConfig->ebcConfig.parityOddEven;
        } /* else/end of if */
#ifndef NDEBUG
        else
        {
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "ixParityENAccParityDetectionConfigure(): "
                "EBC Configuration of CS7 failed\n",0,0,0,0,0,0);
            ixReturnStatus = IX_FAIL;
        } /* end of if */
#endif /* end of #ifndef NDEBUG */
    } /* end of if */

    if ((hwParityConfig->ebcConfig.ebcExtMstEnabled != 
         ixParityENAccParityConfigStatus.ebcConfig.ebcExtMstEnabled) ||
        ((hwParityConfig->ebcConfig.parityOddEven != 
          ixParityENAccParityConfigStatus.ebcConfig.parityOddEven) &&
         ( FALSE == ixParityENAccIXP46XCompDisabled ) &&
         (hwParityConfig->ebcConfig.ebcExtMstEnabled == IX_PARITYENACC_ENABLE)))
    {
        ixEbcPDCfg.ebcCsExtSource = IXP400_PARITYENACC_PE_EBC_EXTMST;
        ixEbcPDCfg.ebcInOrOutbound.ebcExtMstEnabled = 
            hwParityConfig->ebcConfig.ebcExtMstEnabled;
        ixEbcPDCfg.parityOddEven = hwParityConfig->ebcConfig.parityOddEven;
        
        if (IX_SUCCESS == ixParityENAccEbcPEDetectionConfigure(ixEbcPDCfg))
        {
            ixParityENAccParityConfigStatus.ebcConfig.ebcExtMstEnabled =
                hwParityConfig->ebcConfig.ebcExtMstEnabled;
            ixParityENAccParityConfigStatus.ebcConfig.parityOddEven = 
                hwParityConfig->ebcConfig.parityOddEven;
        } /* else/end of if */
#ifndef NDEBUG
        else
        {
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
                "ixParityENAccParityDetectionConfigure(): "
                "EBC Configuration of EXT failed\n",0,0,0,0,0,0);
            ixReturnStatus = IX_FAIL;
        } /* end of if */
#endif /* end of #ifndef NDEBUG */
    } /* end of if */

    if (IX_FAIL == ixReturnStatus)
    {
        IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
            "ixParityENAccParityDetectionConfigure(): "
            "One/More of hardware block's Configuration failed\n", 0,0,0,0,0,0);
    } /* end of if */

    return IX_PARITYENACC_SUCCESS;
} /* end of ixParityENAccParityDetectionConfigure() function */

PUBLIC IxParityENAccStatus
ixParityENAccParityDetectionQuery(
    IxParityENAccHWParityConfig * const hwParityConfig)
{
    /* Not initialised before? */
    if (FALSE == ixParityENAccInitStatus)
    {
        return IX_PARITYENACC_NOT_INITIALISED;
    } /* end of if */

    /* Verify the parameter reference */
    if ((IxParityENAccHWParityConfig *)NULL == hwParityConfig)
    {
        return IX_PARITYENACC_INVALID_PARAMETERS;
    } /* end of if */

    /* Return the current config status of all hardware blocks */
    ixOsalMemCopy((void *)hwParityConfig, (void *)&ixParityENAccParityConfigStatus,
        sizeof(IxParityENAccHWParityConfig));

    return IX_PARITYENACC_SUCCESS;
} /* end of ixParityENAccParityDetectionQuery() function */

PUBLIC IxParityENAccStatus
ixParityENAccParityErrorContextGet(
    IxParityENAccParityErrorContextMessage * const pecMessage)
{
    /* Local Variables */
    IxParityENAccIcParityInterruptStatus ixIcParityInterruptStatus =
    { FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE };
    IxParityENAccMcuPEParityErrorContext  ixMcuPECMsg;
    IxParityENAccNpePEParityErrorContext  ixNpePECMsg;
    IxParityENAccSwcpPEParityErrorContext ixSwcpPECMsg;
    IxParityENAccAqmPEParityErrorContext  ixAqmPECMsg;
    IxParityENAccPbcPEParityErrorContext  ixPbcPECMsg;
    IxParityENAccEbcPEParityErrorContext  ixEbcPECMsg;
    IxParityENAccPmuEAHBErrorTransaction  ixPmuAhbTransactionStatus;

    /* Not initialised before? */
    if (FALSE == ixParityENAccInitStatus)
    {
        return IX_PARITYENACC_NOT_INITIALISED;
    } /* end of if */

    if ((IxParityENAccParityErrorContextMessage *) NULL == pecMessage)
    {
        return IX_PARITYENACC_INVALID_PARAMETERS;
    } /* end of if */

    /* Get the pending parity interrupts status */
    if (IX_SUCCESS != ixParityENAccIcInterruptStatusGet(
                          &ixIcParityInterruptStatus))
    {
        return IX_PARITYENACC_OPERATION_FAILED;
    } /* end of if */

    /*
     * Process interrupts as per the following priority
     *
     * 0 - MCU   (Multi, Single-bit, Overflow in that order)
     * 1 - NPE-A (IMem, DMem and Ext Error in that order)
     * 2 - NPE-B (IMem, DMem and Ext Error in that order)
     * 3 - NPE-C (IMem, DMem and Ext Error in that order)
     * 4 - SWCP
     * 5 - QM
     * 6 - PCI   (Initiator Rd, Target Wr and Initiator Wr in that order)
     * 7 - EXP   (Inbound Wr, Outbound Rd in that order)
     */
    if (TRUE == ixIcParityInterruptStatus.mcuParityInterrupt)
    {
        if (IX_SUCCESS != ixParityENAccMcuPEParityErrorContextFetch(&ixMcuPECMsg))
        {
            return IX_PARITYENACC_OPERATION_FAILED;
        } /* end of if */

        /* No Parity! Might have been cleared by data/prefetch abort 
         * Exception Handler before this function is invoked
         */
        if (IXP400_PARITYENACC_PE_MCU_NOPARITY == ixMcuPECMsg.mcuParitySource)
        {
            return IX_PARITYENACC_NO_PARITY;
        } /* end of if */

        pecMessage->pecParitySource = ixMcuPECMsg.mcuParitySource;
        pecMessage->pecAccessType   = ixMcuPECMsg.mcuAccessType;
        pecMessage->pecRequester    = ixMcuPECMsg.mcuRequester;
        pecMessage->pecAddress      = ixMcuPECMsg.mcuParityAddress;
        pecMessage->pecData         = ixMcuPECMsg.mcuParityData;

        /* Get Last Erroneous AHB Transaction Master & Slave details */
        if (IX_SUCCESS != ixParityENAccPmuEAHBTransactionStatus (
                              &ixPmuAhbTransactionStatus))
        {
            return IX_PARITYENACC_OPERATION_FAILED;
        } /* end of if */

        pecMessage->ahbErrorTran.ahbErrorMaster = ixPmuAhbTransactionStatus.ahbErrorMaster;
        pecMessage->ahbErrorTran.ahbErrorSlave = ixPmuAhbTransactionStatus.ahbErrorSlave;

        /* Increment statistics */
        switch (pecMessage->pecParitySource)
        {
            case IX_PARITYENACC_MCU_SBIT:
            {
                ixParityENAccPEParityErrorStats.mcuStats.parityErrorsSingleBit++;
                break;
            } /* end of case MCU SBIT */
            case IX_PARITYENACC_MCU_MBIT:
            {
                ixParityENAccPEParityErrorStats.mcuStats.parityErrorsMultiBit++;
                break;
            } /* end of case MCU MBIT */
            case IX_PARITYENACC_MCU_OVERFLOW:
            {
                ixParityENAccPEParityErrorStats.mcuStats.parityErrorsOverflow++;
                break;
            } /* end of case MCU OVERFLOW */
            default:
            {
                IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
                    "ixParityENAccParityErrorContextGet(): "
                    "Invalid Parity Source: %0x\n", pecMessage->pecParitySource, 0,0,0,0,0);
                return IX_PARITYENACC_OPERATION_FAILED;
          } /* end of case default */
        } /* end of switch */

        return IX_PARITYENACC_SUCCESS;
    } /* end of if (TRUE == ixIcParityInterruptStatus.mcuParityInterrupt) */

    if (TRUE == ixIcParityInterruptStatus.npeAParityInterrupt)
    {
        if (IX_SUCCESS != ixParityENAccNpePEParityErrorContextFetch (
                              IXP400_PARITYENACC_PE_NPE_A, &ixNpePECMsg))
        {
            return IX_PARITYENACC_OPERATION_FAILED;
        } /* end of if */
        
        pecMessage->pecParitySource = 
            (IXP400_PARITYENACC_PE_NPE_IMEM == ixNpePECMsg.npeParitySource) ?
                IX_PARITYENACC_NPE_A_IMEM :
                (IXP400_PARITYENACC_PE_NPE_DMEM  == ixNpePECMsg.npeParitySource) ?
                     IX_PARITYENACC_NPE_A_DMEM : IX_PARITYENACC_NPE_A_EXT;

        pecMessage->pecAccessType = ixNpePECMsg.npeAccessType;

        /* Increment statistics */
        switch (pecMessage->pecParitySource)
        {
            case IX_PARITYENACC_NPE_A_IMEM:
            {
                ixParityENAccPEParityErrorStats.npeStats.parityErrorsIMem++;
                break;
            } /* end of case NPE - A IMEM */
            case IX_PARITYENACC_NPE_A_DMEM:
            {
                ixParityENAccPEParityErrorStats.npeStats.parityErrorsDMem++;
                break;
            } /* end of case NPE - A DMEM */
            case IX_PARITYENACC_NPE_A_EXT:
            {
                ixParityENAccPEParityErrorStats.npeStats.parityErrorsExternal++;
                break;
            } /* end of case NPE - A EXT */
            default:
            {
                IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
                    "ixParityENAccParityErrorContextGet(): "
                    "Invalid Parity Source: %0x\n", pecMessage->pecParitySource, 0,0,0,0,0);
                return IX_PARITYENACC_OPERATION_FAILED;
          } /* end of case default */
        } /* end of switch */

        return IX_PARITYENACC_SUCCESS;
    } /* end of if (TRUE == ixIcParityInterruptStatus.npeAParityInterrupt) */

    if (TRUE == ixIcParityInterruptStatus.npeBParityInterrupt)
    {
        if (IX_SUCCESS != ixParityENAccNpePEParityErrorContextFetch (
                              IXP400_PARITYENACC_PE_NPE_B, &ixNpePECMsg))
        {
            return IX_PARITYENACC_OPERATION_FAILED;
        } /* end of if */

        pecMessage->pecParitySource = 
            (IXP400_PARITYENACC_PE_NPE_IMEM == ixNpePECMsg.npeParitySource) ?
                IX_PARITYENACC_NPE_B_IMEM :
                (IXP400_PARITYENACC_PE_NPE_DMEM  == ixNpePECMsg.npeParitySource) ?
                     IX_PARITYENACC_NPE_B_DMEM : IX_PARITYENACC_NPE_B_EXT;

        pecMessage->pecAccessType = ixNpePECMsg.npeAccessType;

        /* Increment statistics */
        switch (pecMessage->pecParitySource)
        {
            case IX_PARITYENACC_NPE_B_IMEM:
            {
                ixParityENAccPEParityErrorStats.npeStats.parityErrorsIMem++;
                break;
            } /* end of case NPE - B IMEM */
            case IX_PARITYENACC_NPE_B_DMEM:
            {
                ixParityENAccPEParityErrorStats.npeStats.parityErrorsDMem++;
                break;
            } /* end of case NPE - B DMEM */
            case IX_PARITYENACC_NPE_B_EXT:
            {
                ixParityENAccPEParityErrorStats.npeStats.parityErrorsExternal++;
                break;
            } /* end of case NPE - B EXT */
            default:
            {
                IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
                    "ixParityENAccParityErrorContextGet(): "
                    "Invalid Parity Source: %0x\n", pecMessage->pecParitySource, 0,0,0,0,0);
                return IX_PARITYENACC_OPERATION_FAILED;
          } /* end of case default */
        } /* end of switch */

        return IX_PARITYENACC_SUCCESS;
    } /* end of if (TRUE == ixIcParityInterruptStatus.npeBParityInterrupt) */

    if (TRUE == ixIcParityInterruptStatus.npeCParityInterrupt)
    {
        if (IX_SUCCESS != ixParityENAccNpePEParityErrorContextFetch (
                              IXP400_PARITYENACC_PE_NPE_C, &ixNpePECMsg))
        {
            return IX_PARITYENACC_OPERATION_FAILED;
        } /* end of if */

        pecMessage->pecParitySource = 
            (IXP400_PARITYENACC_PE_NPE_IMEM == ixNpePECMsg.npeParitySource) ?
                IX_PARITYENACC_NPE_C_IMEM :
                (IXP400_PARITYENACC_PE_NPE_DMEM  == ixNpePECMsg.npeParitySource) ?
                     IX_PARITYENACC_NPE_C_DMEM : IX_PARITYENACC_NPE_C_EXT;

        pecMessage->pecAccessType = ixNpePECMsg.npeAccessType;

        /* Increment statistics */
        switch (pecMessage->pecParitySource)
        {
            case IX_PARITYENACC_NPE_C_IMEM:
            {
                ixParityENAccPEParityErrorStats.npeStats.parityErrorsIMem++;
                break;
            } /* end of case NPE - C IMEM */
            case IX_PARITYENACC_NPE_C_DMEM:
            {
                ixParityENAccPEParityErrorStats.npeStats.parityErrorsDMem++;
                break;
            } /* end of case NPE - C DMEM */
            case IX_PARITYENACC_NPE_C_EXT:
            {
                ixParityENAccPEParityErrorStats.npeStats.parityErrorsExternal++;
                break;
            } /* end of case NPE - C EXT */
            default:
            {
                IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
                    "ixParityENAccParityErrorContextGet(): "
                    "Invalid Parity Source: %0x\n", pecMessage->pecParitySource, 0,0,0,0,0);
                return IX_PARITYENACC_OPERATION_FAILED;
          } /* end of case default */
        } /* end of switch */

        return IX_PARITYENACC_SUCCESS;
    } /* end of if (TRUE == ixIcParityInterruptStatus.npeCParityInterrupt) */

    if (TRUE == ixIcParityInterruptStatus.swcpParityInterrupt)
    {
        if (IX_SUCCESS != ixParityENAccSwcpPEParityErrorContextFetch (
                              &ixSwcpPECMsg))
        {
            return IX_PARITYENACC_OPERATION_FAILED;
        } /* end of if */

        pecMessage->pecParitySource = ixSwcpPECMsg.swcpParitySource;
        pecMessage->pecAccessType = ixSwcpPECMsg.swcpAccessType;

        /* Increment statistics */
        ixParityENAccPEParityErrorStats.swcpStats++;

        return IX_PARITYENACC_SUCCESS;
    } /* end of if (TRUE == ixIcParityInterruptStatus.swcpParityInterrupt) */

    if (TRUE == ixIcParityInterruptStatus.aqmParityInterrupt)
    {
        if (IX_SUCCESS != ixParityENAccAqmPEParityErrorContextFetch(
                              &ixAqmPECMsg))
        {
            return IX_PARITYENACC_OPERATION_FAILED;
        } /* end of if */

        pecMessage->pecParitySource = ixAqmPECMsg.aqmParitySource;
        pecMessage->pecAccessType   = ixAqmPECMsg.aqmAccessType;
        pecMessage->pecAddress      = ixAqmPECMsg.aqmParityAddress;
        pecMessage->pecData         = ixAqmPECMsg.aqmParityData;

        /* Increment statistics */
        ixParityENAccPEParityErrorStats.aqmStats++;

        return IX_PARITYENACC_SUCCESS;
    } /* end of if (TRUE == ixIcParityInterruptStatus.aqmParityInterrupt) */

    if (TRUE == ixIcParityInterruptStatus.pbcParityInterrupt)
    {
        if (IX_SUCCESS != ixParityENAccPbcPEParityErrorContextFetch(
                              &ixPbcPECMsg))
        {
            return IX_PARITYENACC_OPERATION_FAILED;
        } /* end of if */

        pecMessage->pecParitySource = ixPbcPECMsg.pbcParitySource;
        pecMessage->pecAccessType   = ixPbcPECMsg.pbcAccessType;

        /* Increment statistics */
        switch (pecMessage->pecParitySource)
        {
            case IX_PARITYENACC_PBC_INITIATOR:
            {
                ixParityENAccPEParityErrorStats.pbcStats.parityErrorsPciInitiator++;
                break;
            } /* end of case PCI Initiator interface */
            case IX_PARITYENACC_PBC_TARGET:
            {
                ixParityENAccPEParityErrorStats.pbcStats.parityErrorsPciTarget++;
                break;
            } /* end of case PCI Target interface */
            default:
            {
                IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
                    "ixParityENAccParityErrorContextGet(): "
                    "Invalid Parity Source: %0x\n", pecMessage->pecParitySource, 0,0,0,0,0);
                return IX_PARITYENACC_OPERATION_FAILED;
          } /* end of case default */
        } /* end of switch */

        return IX_PARITYENACC_SUCCESS;
    } /* end of if (TRUE == ixIcParityInterruptStatus.pbcParityInterrupt) */

    if (TRUE == ixIcParityInterruptStatus.ebcParityInterrupt)
    {
        if (IX_SUCCESS != ixParityENAccEbcPEParityErrorContextFetch(
                              &ixEbcPECMsg))
        {
            return IX_PARITYENACC_OPERATION_FAILED;
        } /* end of if */

        pecMessage->pecParitySource = ixEbcPECMsg.ebcParitySource;
        pecMessage->pecAccessType   = ixEbcPECMsg.ebcAccessType;
        pecMessage->pecAddress      = ixEbcPECMsg.ebcParityAddress;

        /* Increment statistics */
        switch (pecMessage->pecParitySource)
        {
            case IX_PARITYENACC_EBC_CS:
            {
                ixParityENAccPEParityErrorStats.ebcStats.parityErrorsOutbound++;
                break;
            } /* end of case EBC CS */
            case IX_PARITYENACC_EBC_EXTMST:
            {
                ixParityENAccPEParityErrorStats.ebcStats.parityErrorsInbound++;
                break;
            } /* end of case EBC EXT Master */     
            default:
            {
                IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                    "ixParityENAccParityErrorContextGet(): "
                    "Invalid Parity Source: %0x\n", pecMessage->pecParitySource, 0,0,0,0,0);
                return IX_PARITYENACC_OPERATION_FAILED;
            } /* end of case default */
        } /* end of switch */

        return IX_PARITYENACC_SUCCESS;
    } /* end of if (TRUE == ixIcParityInterruptStatus.ebcParityInterrupt) */

#ifndef NDEBUG
    IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "ixParityENAccParityErrorContextGet():"
        "No Parity Interrupt Observed\n",0,0,0,0,0,0);
#endif /* end of #ifndef NDEBUG */

    return IX_PARITYENACC_NO_PARITY;
} /* end of ixParityENAccParityErrorContextGet() function */ 

PUBLIC IxParityENAccStatus
ixParityENAccParityErrorInterruptClear (
    const IxParityENAccParityErrorContextMessage *pecMessage)
{
    /* Local variables */
    IxParityENAccPbcPEParityErrorContext ixPbcPECMsg;
    BOOL invalidParitySrc = FALSE;

    /* Not initialised before? */
    if (FALSE == ixParityENAccInitStatus)
    {
        return IX_PARITYENACC_NOT_INITIALISED;
    } /* end of if */

    if ((IxParityENAccParityErrorContextMessage *) NULL == pecMessage)
    {
        return IX_PARITYENACC_INVALID_PARAMETERS;
    } /* end of if */

    /* check for sub components not present in IXP43X */
    if (IX_FEATURE_CTRL_DEVICE_TYPE_IXP43X == ixFeatureCtrlDeviceRead())
    {

        switch (pecMessage->pecParitySource)
        {

            case IX_PARITYENACC_NPE_B_IMEM:
            case IX_PARITYENACC_NPE_B_DMEM:
            case IX_PARITYENACC_NPE_B_EXT:
            case IX_PARITYENACC_SWCP:
            case IX_PARITYENACC_EBC_CS:
            case IX_PARITYENACC_EBC_EXTMST:
                invalidParitySrc = TRUE;
            break;

            default:
            break;

        }

        if ( TRUE == invalidParitySrc )
        {   
 
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, 
            IX_OSAL_LOG_DEV_STDERR,
            "ixParityENAccParityErrorInterruptClear(): "
            "Invalid Parity Source: %0x\n", pecMessage->pecParitySource, 
            0,0,0,0,0);
            return IX_PARITYENACC_INVALID_PARAMETERS;
        }

    }

    /* Delegate to the respective sub-module */
    switch (pecMessage->pecParitySource)
    {
        case IX_PARITYENACC_NPE_A_IMEM:
        case IX_PARITYENACC_NPE_A_DMEM:
        case IX_PARITYENACC_NPE_A_EXT:
        {
            if (IX_SUCCESS != ixParityENAccNpePEParityInterruptClear(
                                  IXP400_PARITYENACC_PE_NPE_A))
            {
                return IX_PARITYENACC_OPERATION_FAILED;
            } /* end of if */
            break;
        } /* end of case NPE-A */       

        case IX_PARITYENACC_NPE_B_IMEM:
        case IX_PARITYENACC_NPE_B_DMEM:
        case IX_PARITYENACC_NPE_B_EXT:
        {
            if (IX_SUCCESS != ixParityENAccNpePEParityInterruptClear(
                                  IXP400_PARITYENACC_PE_NPE_B))
            {
                return IX_PARITYENACC_OPERATION_FAILED;
            } /* end of if */
            break;
        } /* end of case NPE-B */       
        case IX_PARITYENACC_NPE_C_IMEM:
        case IX_PARITYENACC_NPE_C_DMEM:
        case IX_PARITYENACC_NPE_C_EXT:
        {
            if (IX_SUCCESS != ixParityENAccNpePEParityInterruptClear(
                                  IXP400_PARITYENACC_PE_NPE_C))
            {
                return IX_PARITYENACC_OPERATION_FAILED;
            } /* end of if */
            break;
        } /* end of case NPE-C */       
        case IX_PARITYENACC_SWCP:
        {
            if (IX_SUCCESS != ixParityENAccSwcpPEParityInterruptClear())
            {
                return IX_PARITYENACC_OPERATION_FAILED;
            } /* end of if */
            break;
        } /* end of case SWCP */        
        case IX_PARITYENACC_AQM:
        {
            if (IX_SUCCESS != ixParityENAccAqmPEParityInterruptClear())
            {
                return IX_PARITYENACC_OPERATION_FAILED;
            } /* end of if */
            break;
        } /* end of case AQM */     
        case IX_PARITYENACC_MCU_SBIT:
        {
            if (IX_SUCCESS != ixParityENAccMcuPEParityInterruptClear(
                                  IXP400_PARITYENACC_PE_MCU_SBIT, pecMessage->pecAddress))
            {
                return IX_PARITYENACC_OPERATION_FAILED;
            } /* end of if */
            break;
        } /* end of case MCU SBIT */
        case IX_PARITYENACC_MCU_MBIT:
        {
            if (IX_SUCCESS != ixParityENAccMcuPEParityInterruptClear(
                                  IXP400_PARITYENACC_PE_MCU_MBIT, pecMessage->pecAddress))
            {
                return IX_PARITYENACC_OPERATION_FAILED;
            } /* end of if */
            break;
        } /* end of case MCU MBIT */
        case IX_PARITYENACC_MCU_OVERFLOW:
        {
            if (IX_SUCCESS != ixParityENAccMcuPEParityInterruptClear(
                                  IXP400_PARITYENACC_PE_MCU_OVERFLOW, pecMessage->pecAddress))
            {
                return IX_PARITYENACC_OPERATION_FAILED;
            } /* end of if */
            break;
        } /* end of case MCU OVERFLOW */        
        case IX_PARITYENACC_PBC_INITIATOR:
        case IX_PARITYENACC_PBC_TARGET:
        {
            ixPbcPECMsg.pbcParitySource = pecMessage->pecParitySource;
            ixPbcPECMsg.pbcAccessType = pecMessage->pecAccessType;

            if (IX_SUCCESS != ixParityENAccPbcPEParityInterruptClear(
                                  ixPbcPECMsg))
            {
                return IX_PARITYENACC_OPERATION_FAILED;
            } /* end of if */
            break;
        } /* end of case PBC Target */      
        case IX_PARITYENACC_EBC_CS:
        {
            if (IX_SUCCESS != ixParityENAccEbcPEParityInterruptClear(
                                  IXP400_PARITYENACC_PE_EBC_CS))
            {
                return IX_PARITYENACC_OPERATION_FAILED;
            } /* end of if */
            break;
        } /* end of case EBC */
        case IX_PARITYENACC_EBC_EXTMST:
        {
            if (IX_SUCCESS != ixParityENAccEbcPEParityInterruptClear(
                                  IXP400_PARITYENACC_PE_EBC_EXTMST))
            {
                return IX_PARITYENACC_OPERATION_FAILED;
            } /* end of if */
            break;
        } /* end of case EBC */     
        default:
        {
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "ixParityENAccParityErrorInterruptClear(): "
                "Invalid Parity Source: %0x\n", pecMessage->pecParitySource, 0,0,0,0,0);
            return IX_PARITYENACC_INVALID_PARAMETERS;
        } /* end of case default */
    } /* end of switch */

    return IX_PARITYENACC_SUCCESS;
} /* end of ixParityENAccParityErrorInterruptClear() */


PUBLIC IxParityENAccStatus
ixParityENAccStatsGet (
    IxParityENAccParityErrorStats * const ixParityErrorStats)
{
    /* Not initialised before? */
    if (FALSE == ixParityENAccInitStatus)
    {
        return IX_PARITYENACC_NOT_INITIALISED;
    } /* end of if */

    if ((IxParityENAccParityErrorStats *) NULL == ixParityErrorStats)
    {
        return IX_PARITYENACC_INVALID_PARAMETERS;
    } /* end of if */

    /* Return Parity Error Stats for all hardware blocks */
    ixOsalMemCopy((void *)ixParityErrorStats, (void *)&ixParityENAccPEParityErrorStats,
        sizeof(IxParityENAccParityErrorStats));

    return IX_PARITYENACC_SUCCESS;
} /* end of ixParityENAccStatsGet() function */


PUBLIC IxParityENAccStatus
ixParityENAccStatsShow (void)
{
    /* Not initialised before? */
    if (FALSE == ixParityENAccInitStatus)
    {
        return IX_PARITYENACC_NOT_INITIALISED;
    } /* end of if */

    /* Display Parity Error Stats for all hardware blocks */
    IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "NPE Parity Errors (IMem=%u, DMem=%u, Ext Err=%u)\n",
        ixParityENAccPEParityErrorStats.npeStats.parityErrorsIMem,
        ixParityENAccPEParityErrorStats.npeStats.parityErrorsDMem,
        ixParityENAccPEParityErrorStats.npeStats.parityErrorsExternal,0,0,0);

    IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "MCU Parity Errors (SBit=%u, MBit=%u, OvrFlow=%u)\n",
        ixParityENAccPEParityErrorStats.mcuStats.parityErrorsSingleBit,
        ixParityENAccPEParityErrorStats.mcuStats.parityErrorsMultiBit,
        ixParityENAccPEParityErrorStats.mcuStats.parityErrorsOverflow,0,0,0);

    IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "PBC Parity Errors (Initiator=%u, Target=%u)\n",
        ixParityENAccPEParityErrorStats.pbcStats.parityErrorsPciInitiator,
        ixParityENAccPEParityErrorStats.pbcStats.parityErrorsPciTarget,0,0,0,0);
    
    if (IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X == ixFeatureCtrlDeviceRead())
    {

        IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
            "EBC Parity Errors (Ext Master=%u, ChipSelect=%u)\n",
            ixParityENAccPEParityErrorStats.ebcStats.parityErrorsInbound,
            ixParityENAccPEParityErrorStats.ebcStats.parityErrorsOutbound,0,
            0,0,0);

        IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
            "SWCP Parity Errors (%u)\n",
            ixParityENAccPEParityErrorStats.swcpStats,0,0,0,0,0);

    }

    IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "AQM Parity Errors (%u)\n",
        ixParityENAccPEParityErrorStats.aqmStats,0,0,0,0,0);

    return IX_PARITYENACC_SUCCESS;
} /* end of ixParityENAccStatsShow() function */


PUBLIC IxParityENAccStatus
ixParityENAccStatsReset (void)
{
    /* Not initialised before? */
    if (FALSE == ixParityENAccInitStatus)
    {
        return IX_PARITYENACC_NOT_INITIALISED;
    } /* end of if */

    /* Clear off Parity Error Stats for all hardware blocks */
    ixOsalMemSet((void *)&ixParityENAccPEParityErrorStats, 0, 
        sizeof(ixParityENAccPEParityErrorStats));

    return IX_PARITYENACC_SUCCESS;
} /* end of ixParityENAccStatsReset() function */

PUBLIC IxParityENAccStatus
ixParityENAccNPEParityErrorCheck(UINT32 npeID,
    IxParityENAccParityErrorContextMessage * const pecMessage)
{
    /* Local Variables */
    IX_STATUS status;
    IxParityENAccIcParityInterruptStatus ixIcParityInterruptStatus =
    { FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE };
    IxParityENAccNpePEParityErrorContext  ixNpePECMsg;
    
    /* Not initialised before? */
    if (FALSE == ixParityENAccInitStatus)
    {
        return IX_PARITYENACC_NOT_INITIALISED;
    } /* end of if */

    if ((IxParityENAccParityErrorContextMessage *) NULL == pecMessage)
    {
        return IX_PARITYENACC_INVALID_PARAMETERS;
    } /* end of if */

    status = ixParityENAccCheckNpeIdValidity(npeID);
    if (status != IX_SUCCESS)
    {
        IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "ixParityENAccNPEParityErrorCheck(): "
                "Invalid NPE ID\n",0,0,0,0,0,0);
        return status; 
    }

    /* Get the pending parity interrupts status */
    if (IX_SUCCESS != ixParityENAccIcInterruptStatusGet(
                          &ixIcParityInterruptStatus))
    {
        return IX_PARITYENACC_OPERATION_FAILED;
    } /* end of if */

     
    if (npeID==0&& TRUE == ixIcParityInterruptStatus.npeAParityInterrupt)
    {
        if (IX_SUCCESS != ixParityENAccNpePEParityErrorContextFetch (
                              IXP400_PARITYENACC_PE_NPE_A, &ixNpePECMsg))
        {
            return IX_PARITYENACC_OPERATION_FAILED;
        } /* end of if */
        
        pecMessage->pecParitySource = 
            (IXP400_PARITYENACC_PE_NPE_IMEM == ixNpePECMsg.npeParitySource) ?
                IX_PARITYENACC_NPE_A_IMEM :
                (IXP400_PARITYENACC_PE_NPE_DMEM  == ixNpePECMsg.npeParitySource) ?
                     IX_PARITYENACC_NPE_A_DMEM : IX_PARITYENACC_NPE_A_EXT;

        pecMessage->pecAccessType = ixNpePECMsg.npeAccessType;

        /* Increment statistics */
        switch (pecMessage->pecParitySource)
        {
            case IX_PARITYENACC_NPE_A_IMEM:
            {
                ixParityENAccPEParityErrorStats.npeStats.parityErrorsIMem++;
                break;
            } /* end of case NPE - A IMEM */
            case IX_PARITYENACC_NPE_A_DMEM:
            {
                ixParityENAccPEParityErrorStats.npeStats.parityErrorsDMem++;
                break;
            } /* end of case NPE - A DMEM */
            case IX_PARITYENACC_NPE_A_EXT:
            {
                ixParityENAccPEParityErrorStats.npeStats.parityErrorsExternal++;
                break;
            } /* end of case NPE - A EXT */
            default:
            {
                IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
                    "ixParityENAccParityErrorContextGet(): "
                    "Invalid Parity Source: %0x\n", pecMessage->pecParitySource, 0,0,0,0,0);
                return IX_PARITYENACC_OPERATION_FAILED;
          } /* end of case default */
        } /* end of switch */

        return IX_PARITYENACC_SUCCESS;
    } /* end of if (TRUE == ixIcParityInterruptStatus.npeAParityInterrupt) */

    if (npeID==1 && TRUE == ixIcParityInterruptStatus.npeBParityInterrupt)
    {
        if (IX_SUCCESS != ixParityENAccNpePEParityErrorContextFetch (
                              IXP400_PARITYENACC_PE_NPE_B, &ixNpePECMsg))
        {
            return IX_PARITYENACC_OPERATION_FAILED;
        } /* end of if */

        pecMessage->pecParitySource = 
            (IXP400_PARITYENACC_PE_NPE_IMEM == ixNpePECMsg.npeParitySource) ?
                IX_PARITYENACC_NPE_B_IMEM :
                (IXP400_PARITYENACC_PE_NPE_DMEM  == ixNpePECMsg.npeParitySource) ?
                     IX_PARITYENACC_NPE_B_DMEM : IX_PARITYENACC_NPE_B_EXT;

        pecMessage->pecAccessType = ixNpePECMsg.npeAccessType;

        /* Increment statistics */
        switch (pecMessage->pecParitySource)
        {
            case IX_PARITYENACC_NPE_B_IMEM:
            {
                ixParityENAccPEParityErrorStats.npeStats.parityErrorsIMem++;
                break;
            } /* end of case NPE - B IMEM */
            case IX_PARITYENACC_NPE_B_DMEM:
            {
                ixParityENAccPEParityErrorStats.npeStats.parityErrorsDMem++;
                break;
            } /* end of case NPE - B DMEM */
            case IX_PARITYENACC_NPE_B_EXT:
            {
                ixParityENAccPEParityErrorStats.npeStats.parityErrorsExternal++;
                break;
            } /* end of case NPE - B EXT */
            default:
            {
                IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
                    "ixParityENAccParityErrorContextGet(): "
                    "Invalid Parity Source: %0x\n", pecMessage->pecParitySource, 0,0,0,0,0);
                return IX_PARITYENACC_OPERATION_FAILED;
          } /* end of case default */
        } /* end of switch */

        return IX_PARITYENACC_SUCCESS;
    } /* end of if (TRUE == ixIcParityInterruptStatus.npeBParityInterrupt) */

    if (npeID==2 && TRUE == ixIcParityInterruptStatus.npeCParityInterrupt)
    {
        if (IX_SUCCESS != ixParityENAccNpePEParityErrorContextFetch (
                              IXP400_PARITYENACC_PE_NPE_C, &ixNpePECMsg))
        {
            return IX_PARITYENACC_OPERATION_FAILED;
        } /* end of if */

        pecMessage->pecParitySource = 
            (IXP400_PARITYENACC_PE_NPE_IMEM == ixNpePECMsg.npeParitySource) ?
                IX_PARITYENACC_NPE_C_IMEM :
                (IXP400_PARITYENACC_PE_NPE_DMEM  == ixNpePECMsg.npeParitySource) ?
                     IX_PARITYENACC_NPE_C_DMEM : IX_PARITYENACC_NPE_C_EXT;

        pecMessage->pecAccessType = ixNpePECMsg.npeAccessType;

        /* Increment statistics */
        switch (pecMessage->pecParitySource)
        {
            case IX_PARITYENACC_NPE_C_IMEM:
            {
                ixParityENAccPEParityErrorStats.npeStats.parityErrorsIMem++;
                break;
            } /* end of case NPE - C IMEM */
            case IX_PARITYENACC_NPE_C_DMEM:
            {
                ixParityENAccPEParityErrorStats.npeStats.parityErrorsDMem++;
                break;
            } /* end of case NPE - C DMEM */
            case IX_PARITYENACC_NPE_C_EXT:
            {
                ixParityENAccPEParityErrorStats.npeStats.parityErrorsExternal++;
                break;
            } /* end of case NPE - C EXT */
            default:
            {
                IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
                    "ixParityENAccParityErrorContextGet(): "
                    "Invalid Parity Source: %0x\n", pecMessage->pecParitySource, 0,0,0,0,0);
                return IX_PARITYENACC_OPERATION_FAILED;
          } /* end of case default */
        } /* end of switch */

        return IX_PARITYENACC_SUCCESS;
    } /* end of if (TRUE == ixIcParityInterruptStatus.npeCParityInterrupt) */
     return IX_PARITYENACC_NO_PARITY;
} /* end of ixParityENAccNPEParityErrorCheck() function */ 

PUBLIC IxParityENAccStatus 
ixParityENAccUnload(void) 
{    
    if (TRUE == ixParityENAccInitStatus)
    {
        /* Reset the IxParityENAcc module initialisation status */
        ixParityENAccInitStatus = FALSE;

        /* Unload the NPE parity error detection component */
        if(IX_SUCCESS != ixParityENAccNpePEUnload())
        {
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDERR,
                "ixParityENAccNpePEUnload(): "\
                "Can't unload the NPE parity error detection component!!!\n",0,0,0,0,0,0);
        }		
	
        /* Unload the AQM parity error detection component */
        if(IX_SUCCESS != ixParityENAccAqmPEUnload())
        {
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDERR,
                "ixParityENAccAqmPEUnload(): "\
                "Can't unload the AQM parity error detection component!!!\n",0,0,0,0,0,0);
        }		
			
        /* Unload the EBC parity error detection component , skip 
         * if it is IXP43X processor
         */
        if (IX_FEATURE_CTRL_DEVICE_TYPE_IXP43X != ixFeatureCtrlDeviceRead ())
        {
            if(IX_SUCCESS != ixParityENAccEbcPEUnload())
            {
                IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDERR,
                    "ixParityENAccEbcPEUnload(): "\
                    "Can't unload the EBC parity error detection component!!!\n",0,0,0,0,0,0);
            }	
        }	
	
        /* Unload the PBC parity error detection component */
        if(IX_SUCCESS != ixParityENAccPbcPEUnload())
        {
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDERR,
                "ixParityENAccPbcPEUnload(): "\
                "Can't unload the PBC parity error detection component!!!\n",0,0,0,0,0,0);
        }		

        /* Unload the MCU parity error detection component */
        if(IX_SUCCESS != ixParityENAccMcuPEUnload())
        {
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDERR,
                "ixParityENAccMcuPEUnload(): "\
                "Can't unload the MCU parity error detection component!!!\n",0,0,0,0,0,0);
        }
	
        /* Unload the SWCP parity error detection component , skip
         * unloading if it is IXP43X processor since SWCP processor 
         * is not found in IXP43X is not available
         */
        if (IX_FEATURE_CTRL_DEVICE_TYPE_IXP43X != ixFeatureCtrlDeviceRead ())
        {
            if(IX_SUCCESS != ixParityENAccSwcpPEUnload())
            {
                IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDERR,
                    "ixParityENAccSwcpPEUnload(): "\
                    "Can't unload the SWCP parity error detection component!!!\n",0,0,0,0,0,0);
            }
        }		

        /* Unload the PMU parity error detection component */
        if(IX_SUCCESS != ixParityENAccPmuEUnload())
        {
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDERR,
                "ixParityENAccPmuEUnload(): "\
                "Can't unload the PMU parity error detection component!!!\n",0,0,0,0,0,0);
        }		

        /* Unload the INTC parity error detection component */
        if(IX_SUCCESS != ixParityENAccIcEUnload())
        {
            IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDERR,
                "ixParityENAccIcEUnload(): "\
                "Can't unload the INTC parity error detection component!!!\n",0,0,0,0,0,0);
        }		
        
	/* Reset the common statistics */
        ixParityENAccResetCommonStats();
	
        /* Reset the Client callback routine */
        ixParityENAccClientCb = NULL; 
        
        /* Reset the Fused-Out Modules */
        ixParityENAccFusedModules = 0;
    }
    else
    {
        IXP400_PARITYENACC_MSGLOG(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "IxParityENAcc Component Not Intialized...Can't Unload!!!\n", 0,0,0,0,0,0);

        return IX_PARITYENACC_NOT_INITIALISED;
    }
    
    return IX_PARITYENACC_SUCCESS;
} /* end of ixParityENAccUnload() function */

#endif /* __ixp46X || __ixp43X */
