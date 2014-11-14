/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#define DSL_INTERN

#include "drv_dsl_cpe_api.h"
#undef DSL_DBG_BLOCK
#define DSL_DBG_BLOCK DSL_DBG_LED

/** \addtogroup DRV_DSL_CPE_COMMON
 @{ */

#if defined(INCLUDE_ADSL_LED)

#if defined (INCLUDE_DSL_CPE_API_DANUBE)

/* wakeup and clean led module */
int stop_led_module = 0;

static DSL_Context_t *pGlobalContext;

static spinlock_t dsl_led_lock;

static DSL_uint16_t flash_freq = 0;
DSL_uint16_t flash, off, on;

static DSL_DRV_ThreadCtrl_t LedControl;

#ifdef INCLUDE_DSL_DATA_LED_SIMULATOR
static DSL_DRV_ThreadCtrl_t LedSim;
#endif /* INCLUDE_DSL_DATA_LED_SIMULATOR*/

/**
   \brief API for atm driver to notify led thread a data comming/sending
   This function provide a API used by ATM driver, the atm driver call this
   function once a atm packet sent/received.

   \return  0 success else failed

   \ingroup Internal
 */
static DSL_int_t DSL_DRV_LED_Flash ( DSL_void_t )
{
   DSL_uint32_t flags = 0;
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_Context_t *pContext = pGlobalContext;

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: IN - DSL_DRV_LED_Flash"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, pContext->pDevCtx);
   DSL_CHECK_ERR_CODE();

   spin_lock_irqsave(&dsl_led_lock, flags);
   if (!stop_led_module)
   {
      /* asking to flash led */
      pContext->bLedNeedToFlash = DSL_TRUE;
      pContext->bLedStatusOn    = DSL_FALSE;
   }
   spin_unlock_irqrestore(&dsl_led_lock, flags);
#ifndef RTEMS
   if (!stop_led_module)
#else
   if (stop_led_module)
#endif /* RTEMS*/
   {
      DSL_DRV_WAKEUP_EVENT(pContext->ledPollingEvent);
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_LED_Flash"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   return 0;
}

/*
   Led Thread Main function
*/
static DSL_int_t DSL_DRV_LED_Poll(
   DSL_DRV_ThreadParams_t *param)
{
   DSL_uint32_t flags = 0;
   DSL_int_t nOsRet = 0;
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_Context_t *pContext = (DSL_Context_t *)param->nArg1;
   DSL_LineStateValue_t nCurrentState = DSL_LINESTATE_UNKNOWN;

   /* begin polling ... */
   stop_led_module = 0;

   while (!stop_led_module)
   {
      /* Wake up every 10s */
      while (!(stop_led_module || pContext->bLedNeedToFlash) &&
         !(DSL_DRV_WAIT_EVENT_TIMEOUT(pContext->ledPollingEvent, 10000)));
      /* -ERESTARTSYS or condition evaluates to true */

      /* Only proceed if the specified line is in SHOWTIME state*/
      DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nCurrentState);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - [%s %d]: Failed to get "
            "current line state!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), __func__, __LINE__));

         spin_lock_irqsave(&dsl_led_lock, flags);
         stop_led_module = 1;
         pContext->bLedNeedToFlash = DSL_FALSE;
         spin_unlock_irqrestore(&dsl_led_lock, flags);
         break;
      }

      /* Only proceed if the specified line is in SHOWTIME state*/
      if ((nCurrentState != DSL_LINESTATE_SHOWTIME_TC_SYNC) &&
         (nCurrentState != DSL_LINESTATE_SHOWTIME_NO_SYNC))
      {
         spin_lock_irqsave(&dsl_led_lock, flags);
         pContext->bLedNeedToFlash = DSL_FALSE;
         spin_unlock_irqrestore(&dsl_led_lock, flags);
         continue;
      }
      
      if (pContext->bLedNeedToFlash && (pContext->bLedStatusOn == DSL_FALSE))
      {
         /* FLASH */
         /* use GPIO9 for TR68 data led off. */
         if (DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_INFO,
         DSL_CMV_ADDRESS_INFO_GPIO_CONTROL, 5, 1, &flash) != DSL_SUCCESS)
         {
            DSL_DEBUG(DSL_DBG_ERR, (pContext, "[%s %d]: CMV Write fail!" DSL_DRV_CRLF, __func__, __LINE__));
         }
         /* Let it flash for 1 second */
         if (!stop_led_module)
         {
            nErrCode = (DSL_Error_t)DSL_DRV_WAIT_EVENT_TIMEOUT(pContext->ledPollingEvent, 1000);
            if (nErrCode >= DSL_SUCCESS)
            {
               spin_lock_irqsave(&dsl_led_lock, flags);
               pContext->bLedStatusOn    = DSL_TRUE;
               pContext->bLedNeedToFlash = DSL_FALSE;
               spin_unlock_irqrestore(&dsl_led_lock, flags);
            }
         }
      }
      if ((pContext->bLedNeedToFlash == DSL_FALSE) && pContext->bLedStatusOn)
      {
         /* ON */
         /* use GPIO9 for TR68 data led off. */
         if (DSL_DRV_DANUBE_CmvWrite(pContext, DSL_CMV_GROUP_INFO,
            DSL_CMV_ADDRESS_INFO_GPIO_CONTROL, 5, 1, &on) != DSL_SUCCESS)
         {
            DSL_DEBUG(DSL_DBG_ERR, (pContext, "[%s %d]: CMV Write fail!" DSL_DRV_CRLF, __func__, __LINE__));
         }
         spin_lock_irqsave(&dsl_led_lock, flags);
         pContext->bLedStatusOn    = DSL_FALSE;
         spin_unlock_irqrestore(&dsl_led_lock, flags);
      }
   }

   nOsRet = DSL_DRV_ErrorToOS(nErrCode);

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: OUT - DSL_DRV_LED_Poll thread"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   return nOsRet;
}

/*
   Led Simulator Thread
*/
#ifdef INCLUDE_DSL_DATA_LED_SIMULATOR
static DSL_int_t DSL_DRV_LED_Sim(
   DSL_DRV_ThreadParams_t *param)
{
   DSL_int_t nOsRet = 0;
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_Context_t *pContext = (DSL_Context_t *)param->nArg1;
   DSL_uint16_t nPollTimeout = 0;

   /* begin polling ... */
   stop_led_module = 0;

   nPollTimeout = pContext->nDataLedSimControlData.nBlinkTimeout;

   while (!stop_led_module)
   {
      DSL_DRV_WAIT_EVENT_TIMEOUT(pContext->dataLedSimEvent, nPollTimeout);

      if (pContext->nDataLedSimControlData.nLedBehavior == DAL_DATA_LED_BLINK)
      {
         /* Trigger Data LED to blink*/
         DSL_DRV_LED_Flash();
      }

      nPollTimeout = pContext->nDataLedSimControlData.nBlinkTimeout;
   }

   nOsRet = DSL_DRV_ErrorToOS(nErrCode);

   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: OUT - DSL_DRV_LED_Poll thread"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   return nOsRet;
}
#endif /* INCLUDE_DSL_DATA_LED_SIMULATOR*/

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_led.h'
*/
DSL_Error_t DSL_DRV_LED_ModuleInit(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_FwVersion_t *pVer = DSL_NULL;
     
   DSL_CHECK_POINTER(pContext, pContext->pDevCtx);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_LED_ModuleInit"DSL_DRV_CRLF,
       DSL_DEV_NUM(pContext)));

   if (pContext->bLedInit)
   {
      DSL_DEBUG(DSL_DBG_WRN,
         (pContext, "DSL[%02d]: WARNING - LED module already started" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return DSL_SUCCESS;
   }

   if(DSL_DRV_MUTEX_LOCK(pContext->dataMutex))
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Couldn't lock data mutex!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }

   pVer = &(pContext->pDevCtx->data.version.fwVersion);

   if (pVer->bValid == DSL_TRUE)
   {
      if (pVer->nMajorVersion != DSL_AMAZON_SE_FW_MAJOR_NUMBER &&
          pVer->nMajorVersion != DSL_AR9_FW_MAJOR_NUMBER && 
          pVer->nMajorVersion != DSL_DANUBE_FW_MAJOR_NUMBER)
      {
         DSL_DEBUG(DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ADSL Data LED handling included but not implemented for Danube!"
            DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         nErrCode = DSL_ERR_NOT_IMPLEMENTED;
      }
   }
   else
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext,
         "DSL[%02d]: ERROR - No firmware version available" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
            
      nErrCode = DSL_WRN_DEVICE_NO_DATA;
   }

   DSL_DRV_MUTEX_UNLOCK(pContext->dataMutex);   

   if (nErrCode != DSL_SUCCESS)
   {
      return nErrCode;
   }

   spin_lock_init(&dsl_led_lock);
   pGlobalContext = pContext;
   pContext->bLedNeedToFlash = DSL_FALSE;
   pContext->bLedStatusOn = DSL_FALSE;
#if !((DSL_DATA_LED_FLASH_FREQUENCY >= 1) && (DSL_DATA_LED_FLASH_FREQUENCY <= 32))
#error Please use --with-adsl-data-led-flash-frequency=[1-32] to define ADSL Data Flash Frequency
#endif
   DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: ADSL data led flashing at %d Hz" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), DSL_DATA_LED_FLASH_FREQUENCY));
   flash_freq = DSL_DATA_LED_FLASH_FREQUENCY;
   if (flash_freq == 16)
   {
      flash = (pVer->nMajorVersion == DSL_DANUBE_FW_MAJOR_NUMBER) ? 0x901 : 0xA01;
   }
   else
   {
      flash_freq = ((flash_freq * 2) << 2) & 0xFC;
      flash = (pVer->nMajorVersion == DSL_DANUBE_FW_MAJOR_NUMBER) ? 0x902 : 0xA02;
      flash |= flash_freq;
   }
   
   off = (pVer->nMajorVersion == DSL_DANUBE_FW_MAJOR_NUMBER) ? 0x900 : 0xA00;

   on  = off | 0x3;
   
   /* adsl led for led function */
   DSL_DRV_INIT_EVENT ("atm_led", pContext->ledPollingEvent);

#ifdef INCLUDE_DSL_DATA_LED_SIMULATOR
   pContext->nDataLedSimControlData.nLedBehavior  = DSL_DATA_LED_OFF;
   pContext->nDataLedSimControlData.nBlinkTimeout = 1000;
   DSL_DRV_INIT_EVENT ("led_sim", pContext->dataLedSimEvent);   
   if ( DSL_DRV_THREAD (
           &LedSim, "atm_led_sim",
           DSL_DRV_LED_Sim, (DSL_uint32_t)pContext))
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ADSL DATA LED Simulator task start failed!"
         DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }
#else
   DSL_BSP_ATMLedCBRegister (DSL_DRV_LED_Flash);
#endif /* INCLUDE_DSL_DATA_LED_SIMULATOR*/

   pContext->bLedInit = DSL_TRUE;

   return (DSL_Error_t)DSL_DRV_THREAD (&LedControl, "atm_led_completion", DSL_DRV_LED_Poll, (DSL_uint32_t)pContext);
}
#endif /* INCLUDE_DSL_CPE_API_DANUBE */

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_led.h'
*/
DSL_Error_t DSL_DRV_LED_FirmwareInit(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_LED_FirmwareInit" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* This function is currently not used.
      LED configuration is done within device specific function
      DSL_DRV_DEV_AutobootHandleStart() */
   /*
   DSL_CHECK_CTX_POINTER(pContext);
   nErrCode = DSL_DRV_DEV_LED_FirmwareInit(pContext);
   */

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_LED_FirmwareInit, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#endif /* INCLUDE_ADSL_LED */

/** @} DRV_DSL_CPE_COMMON */

