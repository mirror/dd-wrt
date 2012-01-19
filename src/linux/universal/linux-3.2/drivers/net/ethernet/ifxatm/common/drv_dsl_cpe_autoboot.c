/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#define DSL_INTERN

#include "drv_dsl_cpe_api.h"
#include "drv_dsl_cpe_autoboot.h"

#undef DSL_DBG_BLOCK
#define DSL_DBG_BLOCK DSL_DBG_AUTOBOOT_THREAD

/** \addtogroup DRV_DSL_CPE_COMMON
 @{ */

static DSL_int_t DSL_DRV_AutobootThreadMain(
   DSL_DRV_ThreadParams_t *param);

static DSL_Error_t DSL_DRV_AutobootHandleStart(
   DSL_Context_t *pContext);

static DSL_Error_t DSL_DRV_AutobootHandleRestart(
   DSL_Context_t *pContext);

static DSL_Error_t DSL_DRV_AutobootHandleShowtime(
   DSL_Context_t *pContext);

static DSL_Error_t DSL_DRV_AutobootHandleException(
   DSL_Context_t *pContext);

static DSL_Error_t DSL_DRV_AutobootHandleFwRequest(
   DSL_Context_t *pContext);

static DSL_Error_t DSL_DRV_AutobootHandleFwWait(
   DSL_Context_t *pContext);

static DSL_Error_t DSL_DRV_AutobootHandleFwReady(
   DSL_Context_t *pContext);

static DSL_Error_t DSL_DRV_AutobootHandleL3(
   DSL_Context_t *pContext,
   DSL_boolean_t bL3Forced);

static DSL_Error_t DSL_DRV_AutobootHandleTraining(
   DSL_Context_t *pContext);

static DSL_Error_t DSL_DRV_AutobootThreadInit(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_boolean_t bFirst = DSL_FALSE, bInitComplete = DSL_FALSE;
   DSL_AutobootCtrlSet_t nAutobootStartupMode = DSL_AUTOBOOT_CTRL_STOP;

   DSL_CHECK_CTX_POINTER(pContext);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: Set init state in the autoboot thread..."DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CTX_READ_SCALAR(pContext, nErrCode, pDevCtx->bFirstPowerOn, bFirst);
   DSL_CTX_READ_SCALAR(pContext, nErrCode, nAutobootStartupMode,
      nAutobootStartupMode);
   DSL_CTX_READ_SCALAR(pContext, nErrCode, bInitComplete, bInitComplete);

   if (bFirst == DSL_TRUE || nAutobootStartupMode == DSL_AUTOBOOT_CTRL_STOP)
   {
      /* In case of 1st activation move to the
         DSL_AUTOBOOTSTATE_FIRMWARE_READY state*/
      nErrCode = DSL_DRV_AutobootStateSet(
                    pContext, DSL_AUTOBOOTSTATE_FIRMWARE_READY,
                    DSL_AUTOBOOT_FW_READY_POLL_TIME);
   }

   if (nErrCode == DSL_SUCCESS)
   {
      DSL_CTX_WRITE_SCALAR(pContext, nErrCode, bAutobootStopPending, DSL_FALSE);
   }

   if (pContext->disconnectTime == 0)
      pContext->disconnectTime = DSL_DRV_TimeMSecGet();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: DSL_DRV_AutobootThreadInit: Disconnected at %d sec"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), DSL_DRV_TimeSecGet(pContext->disconnectTime)/1000));

   DSL_DRV_WAKEUP_EVENT(pContext->autobootEvent);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: Set init state in the autoboot is completed..."DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_autoboot.h'
*/
DSL_Error_t DSL_DRV_AutobootThreadStart(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pContext->pDevCtx);
   DSL_CHECK_ERR_CODE();

   if (pContext->bAutobootThreadStarted == DSL_FALSE)
   {
      DSL_DEBUG(DSL_DBG_MSG,
         (pContext, "DSL[%02d]: Autoboot thread will be started..."DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      /* Set Autoboot Status*/
      nErrCode = DSL_DRV_AutobootStatusSet(pContext, DSL_AUTOBOOT_STATUS_STARTING,
         DSL_FW_REQUEST_NA);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG(DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Autoboot status set failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
      }

      pContext->bAutobootThreadShutdown = DSL_FALSE;
      pContext->nAutobootPollTime = DSL_AUTOBOOT_IDLE_POLL_TIME;

      DSL_DRV_INIT_EVENT("autbtev", pContext->autobootEvent);

      if ((nErrCode == DSL_SUCCESS)
          && (nErrCode = DSL_DRV_AutobootThreadInit(pContext)) == DSL_SUCCESS)
      {
#ifdef INCLUDE_DSL_DELT
         DSL_CTX_WRITE_SCALAR(pContext, nErrCode, bLoopDiagnosticsCompleted, DSL_FALSE);
#endif /* INCLUDE_DSL_DELT*/
         DSL_DEBUG(DSL_DBG_MSG, 
            (pContext, "DSL[%02d]: Starting autoboot thread..."DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
            
         nErrCode = (DSL_Error_t)DSL_DRV_THREAD(
            &pContext->AutobootControl, "autbtex", DSL_DRV_AutobootThreadMain, (DSL_uint32_t)pContext);
      }

      DSL_DEBUG(DSL_DBG_MSG,
         (pContext, "DSL[%02d]: Autoboot thread start finished with %d code..."DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), nErrCode));
         
      return nErrCode;
   }
   else
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: Autoboot thread is already running..."DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return DSL_ERR_AUTOBOOT_BUSY; /* Device or resource busy */
   }
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_autoboot.h'
*/
static DSL_int_t DSL_DRV_AutobootThreadMain(
   DSL_DRV_ThreadParams_t *param)
{
   DSL_Context_t *pContext = (DSL_Context_t *)param->nArg1;
   DSL_int_t nOsRet = 0;
   DSL_Error_t nRet;
   DSL_boolean_t bAutobootThreadShutdown = DSL_FALSE;
   DSL_Error_t nErrCode = DSL_SUCCESS;
#if defined(INCLUDE_DSL_CPE_API_VINAX) || (defined(INCLUDE_DSL_CPE_API_DANUBE) && defined(INCLUDE_DSL_G997_LINE_INVENTORY))
   DSL_int_t nEventType = 0;
   DSL_uint32_t nTimeoutID = 0;
#endif /* defined(INCLUDE_DSL_CPE_API_VINAX) || (defined(INCLUDE_DSL_CPE_API_DANUBE) && defined(INCLUDE_DSL_G997_LINE_INVENTORY))*/

   if (pContext == DSL_NULL)
   {
      return -1;
   }

   pContext->bAutobootThreadStarted = DSL_TRUE;

   DSL_CTX_READ_SCALAR(
      pContext, nErrCode, bAutobootThreadShutdown, bAutobootThreadShutdown);

   /* Set Autoboot Status*/
   nErrCode = DSL_DRV_AutobootStatusSet(pContext, DSL_AUTOBOOT_STATUS_RUNNING,
      DSL_FW_REQUEST_NA);

   while( bAutobootThreadShutdown == DSL_FALSE && nErrCode == DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_MSG,
         (pContext, "DSL[%02d]: Autoboot is running..."DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      DSL_DRV_WAIT_EVENT_TIMEOUT(pContext->autobootEvent, pContext->nAutobootPollTime);

      /* Check autoboot state*/
      if ((nRet = DSL_DRV_AutobootStateCheck(pContext)) != DSL_SUCCESS)
      {
         DSL_DEBUG(DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - autoboot state machine status check failed"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         nOsRet = DSL_DRV_ErrorToOS(nRet);
         break;
      }

#if defined(INCLUDE_DSL_CPE_API_VINAX) || (defined(INCLUDE_DSL_CPE_API_DANUBE) && defined(INCLUDE_DSL_G997_LINE_INVENTORY))
      while( DSL_DRV_Timeout_GetNextActiveEvent(
                pContext, &nEventType, &nTimeoutID ) >= DSL_SUCCESS)
      {
         /* Clear Active event */
         nErrCode = DSL_DRV_Timeout_RemoveEvent(pContext, nTimeoutID);
         if( nErrCode != DSL_SUCCESS )
         {
            DSL_DEBUG(DSL_DBG_MSG,
               (pContext, "DSL[%02d]: ERROR - timeout event remove failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
         }

         /* Handle Timeout event*/
         nErrCode = DSL_DRV_OnTimeoutEvent(pContext, nEventType, nTimeoutID);
         if( nErrCode != DSL_SUCCESS )
         {
            DSL_DEBUG(DSL_DBG_MSG,
               (pContext, "DSL[%02d]: ERROR - timeout event handle failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
         }
      }
#endif /* defined(INCLUDE_DSL_CPE_API_VINAX) || (defined(INCLUDE_DSL_CPE_API_DANUBE) && defined(INCLUDE_DSL_G997_LINE_INVENTORY))*/

      DSL_CTX_READ_SCALAR(
         pContext, nErrCode, bAutobootThreadShutdown, bAutobootThreadShutdown);
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: autoboot ending (%lu)"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), DSL_DRV_TimeMSecGet()));

   pContext->bAutobootThreadStarted = DSL_FALSE;

   /* Set Autoboot Status*/
   nErrCode = DSL_DRV_AutobootStatusSet(pContext, DSL_AUTOBOOT_STATUS_STOPPED,
      DSL_FW_REQUEST_NA);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: autoboot complete (%lu)"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), DSL_DRV_TimeMSecGet()));

   DSL_DRV_THREAD_DELETE(pContext->AutobootControl, 0);

   return nOsRet;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_autoboot.h'
*/
DSL_Error_t DSL_DRV_AutobootThreadStop(
   DSL_Context_t *pContext)
{
   if( pContext->bAutobootThreadStarted == DSL_FALSE )
      return DSL_ERR_AUTOBOOT_NOT_STARTED;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: Stopping autoboot thread... (%lu)"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), DSL_DRV_TimeMSecGet()));

   pContext->bAutobootThreadShutdown = DSL_TRUE;

   DSL_DRV_WAKEUP_EVENT(pContext->autobootEvent);

   DSL_DRV_WAIT_COMPLETION(&pContext->AutobootControl);

   pContext->bAutobootThreadStarted = DSL_FALSE;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: Autoboot thread has stopped... (%lu)"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), DSL_DRV_TimeMSecGet()));

   return DSL_SUCCESS;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_autoboot.h'
*/
DSL_Error_t DSL_DRV_AutobootTimeoutSet(
   DSL_Context_t *pContext,
   DSL_uint32_t nTimeout)
{
   DSL_DRV_MUTEX_LOCK(pContext->dataMutex);

   pContext->autobootStartTime = DSL_DRV_TimeMSecGet();
   pContext->nAutobootTimeoutLimit = (DSL_int_t)nTimeout;

   DSL_DRV_MUTEX_UNLOCK(pContext->dataMutex);

   return DSL_SUCCESS;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_autoboot.h'
*/
DSL_Error_t DSL_DRV_AutobootStateSet(
   DSL_Context_t *pContext,
   DSL_Autoboot_State_t nState,
   DSL_uint32_t nPollTimeSec)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_TestModeControlSet_t nTestMode;

   DSL_CTX_READ_SCALAR(pContext, nErrCode, nTestModeControl, nTestMode);

   if (nTestMode == DSL_TESTMODE_LOCK)
   {
      DSL_DEBUG(DSL_DBG_MSG,
         (pContext, "DSL[%02d]: DSL_DRV_AutobootStateSet: state will not be changed "
         "because of test lock"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
   }
   else
   {
      DSL_DEBUG(DSL_DBG_MSG, (pContext,
         "DSL[%02d]: Autoboot State: %s" DSL_DRV_CRLF, DSL_DEV_NUM(pContext),
         DSL_DBG_PRN_AUTOBOOT_STATE[nState]));

      DSL_CTX_WRITE_SCALAR(pContext, nErrCode, nAutobootState, nState);
      DSL_CTX_WRITE_SCALAR(pContext, nErrCode, nAutobootPollTime, nPollTimeSec);
   }

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_autoboot.h'
*/
DSL_Error_t DSL_DRV_AutobootStateCheck(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_Autoboot_State_t nAutoBootSt = DSL_AUTOBOOTSTATE_UNKNOWN,
                        nPrev = DSL_AUTOBOOTSTATE_UNKNOWN;
   DSL_DEV_Handle_t dev;
   DSL_boolean_t bPowerManagementL3Forced, bAutobootContinue = DSL_FALSE,
                 bAutobootRestart = DSL_FALSE, bAutobootReboot = DSL_FALSE;
#if defined (INCLUDE_DSL_CPE_API_DANUBE)
   DSL_LineStateValue_t nCurrentState = DSL_LINESTATE_UNKNOWN;
   DSL_TestModeControlSet_t nTestMode = DSL_TESTMODE_DISABLE;
#endif /* defined (INCLUDE_DSL_CPE_API_DANUBE)*/

   DSL_CHECK_POINTER(pContext, pContext->pDevCtx);
   DSL_CHECK_ERR_CODE();

   dev = DSL_DRV_LowDeviceGet(pContext->pDevCtx);
   if (dev == DSL_NULL)
   {
      return DSL_ERR_INTERNAL;
   }

#if defined (INCLUDE_DSL_CPE_API_DANUBE)
   /* Check if MEI reboot event occured*/
   if (pContext->bMeiReboot)
   {
      /* Reset MEI reboot flag*/
      pContext->bMeiReboot = DSL_FALSE;
      
      /* Get Line state*/
      DSL_CTX_READ(pContext, nErrCode, nLineState, nCurrentState);
      
      if ((nCurrentState != DSL_LINESTATE_IDLE) && (nCurrentState != DSL_LINESTATE_SILENT))
      {
         DSL_CTX_READ(pContext, nErrCode, nTestModeControl, nTestMode);
       
         if (nTestMode == DSL_TESTMODE_SHOWTIME_LOCK ||
             nTestMode == DSL_TESTMODE_TRAINING_LOCK)
         {
            DSL_DEBUG(DSL_DBG_MSG,
               (pContext, "DSL[%02d]: Autoboot Locked... "DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
         }
         else
         {
            /* Set reboot flag*/
            DSL_CTX_WRITE_SCALAR(pContext, nErrCode, bAutobootReboot, DSL_TRUE);
         }
      }
   }
#endif /* defined (INCLUDE_DSL_CPE_API_DANUBE)*/

   /* Get Restart flag*/
   DSL_CTX_READ(pContext, nErrCode, bAutobootRestart, bAutobootRestart);

   /* Check for external restart trigger*/
   if (bAutobootRestart)
   {
      /* Set Autoboot state*/
      nErrCode = DSL_DRV_AutobootStateSet(pContext, DSL_AUTOBOOTSTATE_RESTART,
                    DSL_AUTOBOOT_RESTART_POLL_TIME);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext,"DSL[%02d]: ERROR - Autoboot state set failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         return nErrCode;
      }
   }

   /* Get Reboot flag*/
   DSL_CTX_READ(pContext, nErrCode, bAutobootReboot, bAutobootReboot);

   /* Check for external restart trigger*/
   if (bAutobootReboot)
   {
      /* Set Autoboot state*/
      nErrCode = DSL_DRV_AutobootStateSet(pContext, DSL_AUTOBOOTSTATE_EXCEPTION,
                    DSL_AUTOBOOT_EXCEPTION_POLL_TIME);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext,"DSL[%02d]: ERROR - Autoboot state set failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         return nErrCode;
      }

       /* Reset Reboot flag*/
      DSL_CTX_WRITE_SCALAR(pContext, nErrCode, bAutobootReboot, DSL_FALSE);
   }

   /* Get Autoboot state*/
   DSL_CTX_READ(pContext, nErrCode, nAutobootState, nPrev);

   switch (nPrev)
   {
      case DSL_AUTOBOOTSTATE_FIRMWARE_REQUEST:
         nErrCode = DSL_DRV_AutobootHandleFwRequest(pContext);
         break;

      case DSL_AUTOBOOTSTATE_RESTART_WAIT:
         /* Wait for the external trigger to continue Autoboot processing*/
         DSL_CTX_READ_SCALAR(
            pContext, nErrCode, bAutobootContinue, bAutobootContinue);

         if (bAutobootContinue)
         {
            /* Reset external trigger*/
            DSL_CTX_WRITE_SCALAR(
               pContext, nErrCode, bAutobootContinue, DSL_FALSE);

            /* Switch to RESTART...*/
            nErrCode = DSL_DRV_AutobootStateSet(
                          pContext,
                          DSL_AUTOBOOTSTATE_RESTART,
                          DSL_AUTOBOOT_RESTART_POLL_TIME);

            /* Set Autoboot Status*/
            nErrCode = DSL_DRV_AutobootStatusSet(pContext,
                          DSL_AUTOBOOT_STATUS_RUNNING, DSL_FW_REQUEST_NA);
         }
         break;
         
      case DSL_AUTOBOOTSTATE_FIRMWARE_WAIT:
         nErrCode = DSL_DRV_AutobootHandleFwWait(pContext);
         break;

      case DSL_AUTOBOOTSTATE_FIRMWARE_READY:
         /* Handle Fw Ready state*/
         nErrCode = DSL_DRV_AutobootHandleFwReady(pContext);
         break;

      case DSL_AUTOBOOTSTATE_CONFIG_WRITE_WAIT:
         /* Wait for the external trigger to continue Autoboot processing*/
         DSL_CTX_READ_SCALAR(pContext, nErrCode, bAutobootContinue,
            bAutobootContinue);

         if (bAutobootContinue)
         {
            /* Reset external trigger*/
            DSL_CTX_WRITE_SCALAR(pContext, nErrCode, bAutobootContinue,
               DSL_FALSE);

            DSL_CTX_READ_SCALAR(pContext, nErrCode, bPowerManagementL3Forced,
               bPowerManagementL3Forced);

            if (bPowerManagementL3Forced != DSL_FALSE)
            {
               DSL_DRV_AutobootStateSet(pContext, DSL_AUTOBOOTSTATE_L3,
                  DSL_AUTOBOOT_L3_POLL_TIME);
            }
            else
            {
               DSL_DRV_AutobootStateSet(pContext, DSL_AUTOBOOTSTATE_INIT,
                  DSL_AUTOBOOT_INIT_POLL_TIME);
            }
            
            /* Set Autoboot Status*/
            nErrCode = DSL_DRV_AutobootStatusSet(pContext,
               DSL_AUTOBOOT_STATUS_RUNNING, DSL_FW_REQUEST_NA);
         }
         break;

      case DSL_AUTOBOOTSTATE_LINK_ACTIVATE_WAIT:
         /* Wait for the external trigger to continue Autoboot processing*/
         DSL_CTX_READ_SCALAR(pContext, nErrCode, bAutobootContinue,
            bAutobootContinue);

         if (bAutobootContinue)
         {
            /* Reset external trigger*/
            DSL_CTX_WRITE_SCALAR(pContext, nErrCode, bAutobootContinue,
               DSL_FALSE);
   
            /* Activate Link*/
            nErrCode = DSL_DRV_DEV_LinkActivate(pContext);
            if( nErrCode != DSL_SUCCESS )
            {
               DSL_DEBUG( DSL_DBG_ERR,
                  (pContext, "DSL[%02d]: ERROR - Link Activation failed!"DSL_DRV_CRLF,
                  DSL_DEV_NUM(pContext)));
                  
               break;
            }

            DSL_DRV_AutobootTimeoutSet(
               pContext,
               #ifdef INCLUDE_DSL_CPE_API_VINAX
               120
               #else
               60
               #endif
               );

            /* Change Autoboot State*/
            nErrCode = DSL_DRV_AutobootStateSet(
                          pContext,
                          DSL_AUTOBOOTSTATE_TRAIN,
                          DSL_AUTOBOOT_TRAINING_POLL_TIME);
 
            if( nErrCode != DSL_SUCCESS )
            {
               DSL_DEBUG( DSL_DBG_ERR,
                  (pContext, "DSL[%02d]: ERROR - Autoboot state set failed!"DSL_DRV_CRLF,
                  DSL_DEV_NUM(pContext)));
                  
               break;
            }

            /* Set Autoboot Status*/
            nErrCode = DSL_DRV_AutobootStatusSet(pContext, 
               DSL_AUTOBOOT_STATUS_RUNNING,
               DSL_FW_REQUEST_NA);
         }
         break;
         
      case DSL_AUTOBOOTSTATE_L3:
         DSL_CTX_READ_SCALAR(pContext, nErrCode, bPowerManagementL3Forced,
            bPowerManagementL3Forced);
         nErrCode = DSL_DRV_AutobootHandleL3(pContext,
            bPowerManagementL3Forced);
         break;

      case DSL_AUTOBOOTSTATE_INIT:
         nErrCode = DSL_DRV_AutobootHandleStart(pContext);
         break;

      case DSL_AUTOBOOTSTATE_DIAGNOSTIC:
      case DSL_AUTOBOOTSTATE_TRAIN:
         nErrCode = DSL_DRV_AutobootHandleTraining(pContext);
         break;

      case DSL_AUTOBOOTSTATE_SHOWTIME:
         nErrCode = DSL_DRV_AutobootHandleShowtime(pContext);
         break;

      case DSL_AUTOBOOTSTATE_EXCEPTION:
         nErrCode = DSL_DRV_AutobootHandleException(pContext);
         break;

      case DSL_AUTOBOOTSTATE_RESTART:
         nErrCode = DSL_DRV_AutobootHandleRestart(pContext);
         break;

      default:
         DSL_DEBUG(DSL_DBG_ERR,
            (pContext, "DSL[%02d]: autoboot unknown state: %d???"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), pContext->nAutobootState));
            
         nErrCode = DSL_DRV_AutobootStateSet(
                       pContext,
                       pContext->nAutobootState,
                       DSL_AUTOBOOT_IDLE_POLL_TIME);
         break;
   }

   if (nErrCode != DSL_SUCCESS)
   {
      nErrCode =  DSL_DRV_AutobootStateSet(pContext, DSL_AUTOBOOTSTATE_RESTART,
         DSL_AUTOBOOT_RESTART_POLL_TIME);
   }


   DSL_CTX_READ_SCALAR(pContext, nErrCode, nAutobootState, nAutoBootSt);

   if (nPrev != nAutoBootSt)
   {
      DSL_DEBUG(DSL_DBG_MSG, (pContext, "DSL[%02d]: autoboot state was switched: %d "
         "to %d (%ld), retCode=%d"DSL_DRV_CRLF, DSL_DEV_NUM(pContext), (int)nPrev,
         (int)pContext->nAutobootState, DSL_DRV_TimeMSecGet(), nErrCode));
   }

   return nErrCode;
}

#ifdef HAS_TO_BE_CLARIFIED
static DSL_Error_t DSL_DRV_FirmwareStartupInit(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_FirmwareStartupInit"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

#if defined(_INCLUDE_ADSL_LATENCY)
   nErrCode = DSL_DRV_DualLatency_FirmwareInit(pContext);
   if (nErrCode != DSL_SUCCESS)
   {
      return nErrCode;
   }
#endif

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_FirmwareStartupInit"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   return nErrCode;
}
#endif /* HAS_TO_BE_CLARIFIED*/

/*
 * Write down the initial configuration to the firmware and activate link
 */
static DSL_Error_t DSL_DRV_AutobootHandleStart(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
#ifdef INCLUDE_DSL_DELT
   DSL_G997_LDSF_t nLoopMode = DSL_G997_INHIBIT_LDSF;
#endif /* INCLUDE_DSL_DELT*/
   DSL_G997_StartupMode_t nStartupMode = DSL_G997_NORMAL_STARTUP;
   DSL_uint8_t nAutoCount = 0;
   DSL_boolean_t bFirst = DSL_FALSE;
   DSL_boolean_t bWaitBeforeLinkActivation = DSL_FALSE;
   DSL_Autoboot_State_t nState = DSL_AUTOBOOTSTATE_UNKNOWN;   

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_AutobootHandleStart"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

#if defined(INCLUDE_DSL_CPE_API_VINAX) || (defined(INCLUDE_DSL_CPE_API_DANUBE) && defined(INCLUDE_DSL_G997_LINE_INVENTORY))
   DSL_DRV_Timeout_RemoveAllEvents(pContext);
#endif /* defined(INCLUDE_DSL_CPE_API_VINAX) || (defined(INCLUDE_DSL_CPE_API_DANUBE) && defined(INCLUDE_DSL_G997_LINE_INVENTORY))*/

   pContext->bGotShortInitResponse = DSL_FALSE;
   pContext->bGotShowtime = DSL_FALSE;
   pContext->bGotFullInit = DSL_FALSE;

#ifdef HAS_TO_BE_CLARIFIED
   if ((nErrCode = DSL_DRV_FirmwareStartupInit(pContext)) != DSL_SUCCESS)
   {
      return nErrCode;
   }
#endif /* HAS_TO_BE_CLARIFIED */

#ifdef INCLUDE_DSL_DELT
   DSL_CTX_READ_SCALAR(pContext, nErrCode, lineActivateConfig.nLDSF, nLoopMode);
#endif /* INCLUDE_DSL_DELT*/
   DSL_CTX_READ_SCALAR(pContext, nErrCode, lineActivateConfig.nStartupMode,
      nStartupMode);
   DSL_CTX_READ_SCALAR(pContext, nErrCode, pDevCtx->bFirstPowerOn, bFirst);

#ifdef INCLUDE_DSL_DELT
   if (nLoopMode == DSL_G997_AUTO_LDSF)
   {
      DSL_CTX_READ_SCALAR(pContext, nErrCode, nLoopAutoCount, nAutoCount);
   }
   else
   {
      if (nLoopMode == DSL_G997_FORCE_LDSF)
      {
         nAutoCount = 1;
      }
   }
#endif /* INCLUDE_DSL_DELT*/

   if ((nErrCode = DSL_DRV_DEV_AutobootHandleStart(pContext,
      (nAutoCount > 0) ? DSL_TRUE : DSL_FALSE,
      (nStartupMode == DSL_G997_FORCE_SHORT_STARTUP) ? DSL_TRUE : DSL_FALSE ))
      != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: Could not handle modem "
         "start!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

#if defined(INCLUDE_DSL_G997_STATUS) || defined(INCLUDE_DSL_G997_ALARM) || defined(INCLUDE_DSL_CPE_API_VINAX)
      /* Set LINIT value to 'LINIT_UNKNOWN' in case of an error */
      DSL_DRV_HandleLinitValue(pContext, LINIT_UNKNOWN, LINIT_SUB_NONE);
#endif /* defined(INCLUDE_DSL_G997_STATUS) || defined(INCLUDE_DSL_G997_ALARM) || defined(INCLUDE_DSL_CPE_API_VINAX)*/

      return nErrCode;
   }

#ifdef INCLUDE_DSL_PM
   if (!bFirst)
   {
      /* Restore PM module counters*/
      nErrCode = DSL_DRV_PM_CountersRestore(pContext);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - PM counters restore failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
            
         return nErrCode;
      }
   }
#endif /* INCLUDE_DSL_PM*/

   /* Get actual autoboot state*/
   DSL_CTX_READ_SCALAR(pContext, nErrCode, nAutobootState, nState);
   if( nState == DSL_AUTOBOOTSTATE_INIT )
   {
      /* Get Link Activation option*/
      DSL_CTX_READ_SCALAR(pContext, nErrCode,
         nAutobootConfig.nStateMachineOptions.bWaitBeforeLinkActivation, 
         bWaitBeforeLinkActivation);

      if (bWaitBeforeLinkActivation)
      {
         nErrCode = DSL_DRV_AutobootStateSet(
                       pContext,
                       DSL_AUTOBOOTSTATE_LINK_ACTIVATE_WAIT,
                       DSL_AUTOBOOT_LINK_ACTIVATE_WAIT_POLL_TIME);

         /* Set Autoboot Status*/
         nErrCode = DSL_DRV_AutobootStatusSet(pContext, 
            DSL_AUTOBOOT_STATUS_LINK_ACTIVATE_WAIT,
            DSL_FW_REQUEST_NA);

         /* IDLE line state reached*/
         DSL_DRV_LineStateSet(pContext, DSL_LINESTATE_IDLE);
      }
      else
      {
         /* IDLE line state reached*/
         DSL_DRV_LineStateSet(pContext, DSL_LINESTATE_IDLE);

         /* Activate Link*/
         nErrCode = DSL_DRV_DEV_LinkActivate(pContext);
         if( nErrCode != DSL_SUCCESS )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - Link Activation failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
               
            return nErrCode;
         }

         DSL_DRV_AutobootTimeoutSet(pContext, 120);

         /* Change Autoboot State*/
         nErrCode = DSL_DRV_AutobootStateSet(
                       pContext,
                       DSL_AUTOBOOTSTATE_TRAIN,
                       DSL_AUTOBOOT_TRAINING_POLL_TIME);
   
         if( nErrCode != DSL_SUCCESS )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - Autoboot state set failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
               
            return nErrCode;
         }
      }
   }

   if (bFirst == DSL_TRUE && nErrCode == DSL_SUCCESS)
   {
      DSL_CTX_WRITE_SCALAR(pContext, nErrCode, pDevCtx->bFirstPowerOn, DSL_FALSE);
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_AutobootHandleStart"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   return nErrCode;
}

static DSL_Error_t DSL_DRV_AutobootHandleRestart(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_boolean_t bAutobootFwLoadPending = DSL_FALSE;
   DSL_FirmwareRequestType_t nFwReqType = DSL_FW_REQUEST_NA;
   DSL_boolean_t bAutobootRestart = DSL_FALSE;
#ifdef INCLUDE_DSL_CPE_API_VINAX
   DSL_FirmwareRequestType_t nFwType = DSL_FW_REQUEST_NA;
   DSL_uint32_t nEapsTimeoutId = 0;
#endif /* INCLUDE_DSL_CPE_API_VINAX*/
   
   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_AutobootHandleRestart"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

#ifdef INCLUDE_DSL_CPE_API_VINAX
   /* Get EAPS timeout ID from the DSL CPE Context*/
   DSL_CTX_READ(pContext, nErrCode, nEapsTimeoutId, nEapsTimeoutId);
   if (nEapsTimeoutId != 0)
   {
      /* Remove EAPS timeout event*/
      nErrCode = DSL_DRV_Timeout_RemoveEvent(pContext, nEapsTimeoutId);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG(DSL_DBG_MSG,
            (pContext, "DSL[%02d]: ERROR - EAPS timeout event remove failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
      }
   }
   
   /* Save Clause30 counters*/
   nErrCode = DSL_DRV_DEV_Clause30CountersSave(pContext);
   if (nErrCode < DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Clause30 counters save failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }

   /* Update nLastShowtime value*/
   DSL_CTX_WRITE(pContext, nErrCode, pDevCtx->data.nLastShowtime, pContext->pDevCtx->data.nCurrShowtime);
   /* Reset bPermanentShowtime information*/
   DSL_CTX_WRITE_SCALAR(pContext, nErrCode, pDevCtx->data.bPermanentShowtime, DSL_FALSE);   
#endif /* INCLUDE_DSL_CPE_API_VINAX*/

   DSL_DRV_LineStateSet(pContext, DSL_LINESTATE_NOT_INITIALIZED);   

   /* Reset the showtime indication flag*/
   DSL_CTX_WRITE_SCALAR(pContext, nErrCode, bShowtimeReached, DSL_FALSE);
   
   nErrCode = DSL_DRV_AutobootThreadInit(pContext);
   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Autoboot thread init failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
            
      return nErrCode;
   }

#ifdef INCLUDE_DSL_PM
   /* Suspend PM module*/
   nErrCode = DSL_DRV_PM_Suspend(pContext);

   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM module suspend failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
               
      return nErrCode;
   }
#endif /* #ifdef INCLUDE_DSL_PM*/

   /* Update DSL CPE API Context Data*/
   nErrCode = DSL_DRV_CtxDataUpdate(pContext);

   DSL_CTX_READ(pContext, nErrCode, bAutobootRestart, bAutobootRestart);
   if (bAutobootRestart)
   {
#ifdef INCLUDE_DSL_CPE_API_VINAX
      /* Reset nPrevMode value*/
      DSL_CTX_WRITE_SCALAR(pContext, nErrCode, pDevCtx->data.nPrevMode, FWMODE_NA);

      /* Reset CAM state*/
      nErrCode = DSL_DRV_VNX_CamFsmStateSet(pContext, DSL_CAM_INIT);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - CAM FSM state set failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
            
         return nErrCode;
      }
#endif /* #ifdef INCLUDE_DSL_CPE_API_VINAX*/
      /* Reset external trigger*/
      DSL_CTX_WRITE_SCALAR(pContext, nErrCode, bAutobootRestart, DSL_FALSE);
   }

#ifdef INCLUDE_DSL_CPE_API_VINAX
   DSL_CTX_READ_SCALAR(pContext, nErrCode, pDevCtx->data.deviceCfg.nFwType, nFwType);
   nFwReqType = nFwType;
#else
   nFwReqType = DSL_FW_REQUEST_ADSL;
#endif /* #ifdef INCLUDE_DSL_CPE_API_VINAX*/

   DSL_CTX_READ_SCALAR(pContext, nErrCode, bAutobootFwLoadPending, bAutobootFwLoadPending);
   /* Check for pending FW download*/
   if( bAutobootFwLoadPending )
   {
      nErrCode = DSL_DRV_AutobootStateSet(
                    pContext, DSL_AUTOBOOTSTATE_FIRMWARE_WAIT, DSL_AUTOBOOT_FW_WAIT_POLL_TIME);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Autoboot State set failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
      }

      /* Set Autoboot Status */
      nErrCode = DSL_DRV_AutobootStatusSet(pContext, DSL_AUTOBOOT_STATUS_FW_WAIT,
         nFwReqType);

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Autoboot Status set failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
      }

      /* Activate Autoboot timeout for the Firmware Wait dtate*/
      DSL_DRV_AutobootTimeoutSet(pContext, 10);
   }
   else
   {
#if defined (INCLUDE_DSL_CPE_API_DANUBE) && !defined (INCLUDE_DSL_FIRMWARE_MEMORY_FREE)
      /* Reboot device*/
      nErrCode = DSL_DRV_DEV_Reboot(pContext);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Device reboot failed, trying to request FW!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
               
         nErrCode = DSL_DRV_AutobootStateSet(
                       pContext, DSL_AUTOBOOTSTATE_FIRMWARE_REQUEST, DSL_AUTOBOOT_FW_REQUEST_POLL_TIME);
      }
      else
      {
         DSL_CTX_WRITE_SCALAR(pContext, nErrCode, bFwReLoaded, DSL_TRUE);
         
         nErrCode = DSL_DRV_AutobootStateSet(
            pContext, DSL_AUTOBOOTSTATE_FIRMWARE_READY, DSL_AUTOBOOT_FW_READY_POLL_TIME);
      }
#else
      nErrCode = DSL_DRV_AutobootStateSet(
                    pContext, DSL_AUTOBOOTSTATE_FIRMWARE_REQUEST, DSL_AUTOBOOT_FW_REQUEST_POLL_TIME);
#endif /* defined (INCLUDE_DSL_CPE_API_DANUBE) && !defined (INCLUDE_DSL_FIRMWARE_MEMORY_FREE)*/
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_AutobootHandleRestart, retCode=%d"DSL_DRV_CRLF,
      nErrCode, DSL_DEV_NUM(pContext)));

   return nErrCode;
}

static DSL_Error_t DSL_DRV_AutobootHandleTraining(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_Autoboot_State_t nAutobootState = DSL_AUTOBOOTSTATE_UNKNOWN;
   DSL_TestModeControlSet_t nTestMode;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_AutobootHandleTraining"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   nErrCode = DSL_DRV_DEV_AutobootHandleTraining(pContext);

   if (nErrCode == DSL_SUCCESS)
   {
      DSL_CTX_READ_SCALAR(pContext, nErrCode, nAutobootState, nAutobootState);

      if ((nAutobootState == DSL_AUTOBOOTSTATE_TRAIN) ||
          (nAutobootState == DSL_AUTOBOOTSTATE_DIAGNOSTIC))
      {
         DSL_uint32_t currentTime;

         currentTime = DSL_DRV_TimeMSecGet();
         
         pContext->autobootStartTime = pContext->autobootStartTime > currentTime ?
                                       currentTime : pContext->autobootStartTime;
         
         if ((pContext->autobootStartTime + (DSL_uint32_t)(pContext->nAutobootTimeoutLimit*1000))
                < currentTime )
         {
            DSL_CTX_READ_SCALAR(pContext, nErrCode, nTestModeControl, nTestMode);
            if (nTestMode == DSL_TESTMODE_TRAINING_LOCK ||
                nTestMode == DSL_TESTMODE_QUIET)
            {
               DSL_DEBUG(DSL_DBG_MSG,
                  (pContext, "DSL[%02d]: time out on training but training lock is set, "
                  "autoboot will not be restarted"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
            }
            else
            {
#if defined(INCLUDE_DSL_G997_STATUS) || defined(INCLUDE_DSL_G997_ALARM) || defined(INCLUDE_DSL_CPE_API_VINAX)
               /* Generate LINIT event on timeout*/
               DSL_DRV_HandleLinitValue(pContext, LINIT_NO_PEER_XTU, LINIT_SUB_NONE);
#endif /* #if defined(INCLUDE_DSL_G997_STATUS) || defined(INCLUDE_DSL_G997_ALARM) || defined(INCLUDE_DSL_CPE_API_VINAX)*/

#ifdef INCLUDE_DSL_CPE_API_VINAX
               /* Reset nPrevMode value*/
               DSL_CTX_WRITE_SCALAR(pContext, nErrCode, pDevCtx->data.nPrevMode, FWMODE_NA);
            
               /* Reset CAM state in case of timeout while training*/
               nErrCode = DSL_DRV_VNX_CamFsmStateSet(pContext, DSL_CAM_INIT);
               if( nErrCode != DSL_SUCCESS )
               {
                  DSL_DEBUG( DSL_DBG_ERR,
                     (pContext, "DSL[%02d]: ERROR - CAM FSM state set failed!"DSL_DRV_CRLF,
                     DSL_DEV_NUM(pContext)));
            
                  return nErrCode;
               }
#endif /* #ifdef INCLUDE_DSL_CPE_API_VINAX*/

               DSL_DEBUG(DSL_DBG_ERR, (pContext,
                  "DSL[%02d]: Reboot on training timeout (%d)!!!"DSL_DRV_CRLF,
                   DSL_DEV_NUM(pContext), pContext->nAutobootTimeoutLimit));
               nErrCode = DSL_DRV_AutobootStateSet(
                             pContext, DSL_AUTOBOOTSTATE_RESTART,
                             DSL_AUTOBOOT_RESTART_POLL_TIME);
            }
         }
      }
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_AutobootHandleTraining nReturn(%d)"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

static DSL_Error_t DSL_DRV_AutobootHandleShowtime(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_LineStateValue_t nOldLineState = DSL_LINESTATE_UNKNOWN,
      nLineState = DSL_LINESTATE_UNKNOWN;
   DSL_TestModeControlSet_t nTestMode = DSL_TESTMODE_DISABLE;
   DSL_int16_t nMinSnrm = 0, nSnrm = 0;
   DSL_G997_PowerManagement_t nPms = DSL_G997_PMS_NA;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_AutobootHandleShowtime"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nOldLineState);
   DSL_CTX_READ_SCALAR(pContext, nErrCode, nTestModeControl, nTestMode);

   nErrCode = DSL_DRV_LineStateUpdate(pContext);
   if (nErrCode == DSL_SUCCESS)
   {
      DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nLineState);
      switch (nLineState)
      {
         case DSL_LINESTATE_SHOWTIME_NO_SYNC:
         case DSL_LINESTATE_SHOWTIME_TC_SYNC:
            nErrCode = DSL_DRV_ShowtimeStatusUpdate(pContext, DSL_FALSE);
            if (nErrCode == DSL_SUCCESS)
            {
               DSL_CTX_READ(pContext, nErrCode,
                  powerMgmtStatus.nPowerManagementStatus, nPms);
               if (nPms == DSL_G997_PMS_L3)
               {
                  DSL_DEBUG(DSL_DBG_MSG,
                     (pContext, "DSL[%02d]: Transition to L3 detected, restarting..."
                     DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
                     
                     nErrCode = DSL_DRV_AutobootStateSet(
                                   pContext,
                                   DSL_AUTOBOOTSTATE_RESTART,
                                   DSL_AUTOBOOT_RESTART_POLL_TIME);
                     break;
               }
            }

            if (nErrCode == DSL_SUCCESS)
            {
               DSL_CTX_READ(pContext, nErrCode, nMinSnrmDs, nMinSnrm);
               DSL_CTX_READ(pContext, nErrCode, nSnrmDs, nSnrm);

               if (nSnrm < nMinSnrm)
               {
                  if (nTestMode == DSL_TESTMODE_SHOWTIME_LOCK
                     || nTestMode == DSL_TESTMODE_TRAINING_LOCK)
                  {
                     DSL_DEBUG(DSL_DBG_MSG,
                        (pContext, "DSL[%02d]: SNR Margin DS (%d/10dB) < "
                        "MINSNRM (%d/10dB), but showtime or training"
                        " lock"DSL_DRV_CRLF, DSL_DEV_NUM(pContext),
                        nSnrm, nMinSnrm));
                  }
                  else
                  {
                     DSL_DEBUG(DSL_DBG_MSG,
                        (pContext, "DSL[%02d]: SNR Margin DS (%d/10dB) < "
                        "MINSNRM (%d/10dB), set reboot flag!"DSL_DRV_CRLF,
                        DSL_DEV_NUM(pContext), nSnrm, nMinSnrm));
                        
                     nErrCode = DSL_DRV_AutobootStateSet(
                                   pContext,
                                   DSL_AUTOBOOTSTATE_EXCEPTION,
                                   DSL_AUTOBOOT_EXCEPTION_POLL_TIME);
                  }
               }
            }
            break;
         default:
            nErrCode = DSL_DRV_AutobootStateSet(
                          pContext,
                          DSL_AUTOBOOTSTATE_EXCEPTION,
                          DSL_AUTOBOOT_EXCEPTION_POLL_TIME);
            break;
      }
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_AutobootHandleShowtime nReturn(%d)"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

static DSL_Error_t DSL_DRV_AutobootHandleException(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_boolean_t bWaitBeforeRestart = DSL_FALSE;

   nErrCode = DSL_DRV_LineStateSet(pContext, DSL_LINESTATE_EXCEPTION);
   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Line state set failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }

   /* Call device specific stuff*/
   nErrCode = DSL_DRV_DEV_AutobootHandleException(pContext);
   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Exception state handle failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }

   /* Get Restart option*/
   DSL_CTX_READ_SCALAR(pContext, nErrCode,
      nAutobootConfig.nStateMachineOptions.bWaitBeforeRestart, 
      bWaitBeforeRestart);

   /* Switch to the next state according to the restart option*/
   if (bWaitBeforeRestart)
   {
#ifdef INCLUDE_DSL_PM
      nErrCode = DSL_DRV_PM_FwPollingStop(pContext);
      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG(DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - PM firmware polling stop failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
      }
#endif /* INCLUDE_DSL_PM*/
      DSL_DRV_AutobootStateSet(pContext, DSL_AUTOBOOTSTATE_RESTART_WAIT,
         DSL_AUTOBOOT_RESTART_WAIT_POLL_TIME);
      /* Set Autoboot Status*/
      DSL_DRV_AutobootStatusSet(
         pContext, DSL_AUTOBOOT_STATUS_RESTART_WAIT, DSL_FW_REQUEST_NA);
   }
   else
   {
      /* Switch to RESTART...*/
      nErrCode = DSL_DRV_AutobootStateSet(
                    pContext,
                    DSL_AUTOBOOTSTATE_RESTART,
                    DSL_AUTOBOOT_RESTART_POLL_TIME);
   }

   return nErrCode;
}

static DSL_Error_t DSL_DRV_AutobootHandleFwRequest(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
#ifndef INCLUDE_FW_REQUEST_SUPPORT
   DSL_int32_t nOff = 0, nLoff = 0;
#else
   DSL_FirmwareRequestData_t fwReqData = { DSL_FW_REQUEST_NA };
#endif /* #ifndef INCLUDE_FW_REQUEST_SUPPORT*/
   DSL_FirmwareRequestType_t nFwType = DSL_FW_REQUEST_NA;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_AutobootHandleFwRequest"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CTX_WRITE_SCALAR(pContext, nErrCode, bFwReLoaded, DSL_FALSE);

#if defined(INCLUDE_DSL_CPE_API_VINAX)
   /* Call device specific stuff*/
   nErrCode = DSL_DRV_DEV_AutobootHandleFwRequest(pContext);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Device Specific Firmware request handling failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return nErrCode;
   }

   DSL_CTX_READ_SCALAR(pContext, nErrCode, pDevCtx->data.deviceCfg.nFwType, nFwType);
#else
   nFwType = DSL_FW_REQUEST_ADSL;
#endif

   while(1)
   {
#ifndef INCLUDE_FW_REQUEST_SUPPORT
      /* Check if the FW download available from the Context*/
      if( (pContext->pFirmware  != DSL_NULL && pContext->nFirmwareSize) ||
          (pContext->pFirmware2 != DSL_NULL && pContext->nFirmwareSize2) )
      {
            /* Try to download stored FW binary from the Context*/
            nErrCode = DSL_DRV_FwDownload(pContext,
                       (DSL_char_t*)pContext->pFirmware,  pContext->nFirmwareSize,
                       (DSL_char_t*)pContext->pFirmware2, pContext->nFirmwareSize2,
                       &nLoff, &nOff, DSL_TRUE);

         if( nErrCode != DSL_SUCCESS )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - Firmware download failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
         }
         else
         {
               nErrCode = DSL_DRV_AutobootStateSet(
                  pContext, DSL_AUTOBOOTSTATE_FIRMWARE_READY, DSL_AUTOBOOT_FW_READY_POLL_TIME);
         }
      }
      else
      {
      /* ... Switch to the DSL_AUTOBOOTSTATE_FIRMWARE_WAIT state*/
         nErrCode = DSL_DRV_AutobootStateSet(
            pContext, DSL_AUTOBOOTSTATE_FIRMWARE_WAIT, DSL_AUTOBOOT_FW_WAIT_POLL_TIME);

         /* Set Autoboot Status*/
         nErrCode = DSL_DRV_AutobootStatusSet(pContext, DSL_AUTOBOOT_STATUS_FW_WAIT,
               nFwType);

         if( nErrCode != DSL_SUCCESS )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - Autoboot Status set failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
         }

         /* Activate Autoboot timeout for the Firmware Wait dtate*/
         DSL_DRV_AutobootTimeoutSet(pContext, 10);
  
         DSL_DEBUG( DSL_DBG_MSG,
            (pContext, "DSL[%02d]: No stored FW, entering FW wait state..."DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
      }
#else /* INCLUDE_FW_REQUEST_SUPPORT */
      #ifdef INCLUDE_DSL_CPE_API_VINAX
      fwReqData.nFirmwareRequestType = nFwType;
      #else
      fwReqData.nFirmwareRequestType = DSL_FW_REQUEST_ADSL;
      #endif /* INCLUDE_DSL_CPE_API_VINAX*/

      DSL_CTX_WRITE_SCALAR(pContext, nErrCode, bFwRequestHandled, DSL_FALSE);

      nErrCode = DSL_DRV_AutobootStateSet(
                    pContext,
                    DSL_AUTOBOOTSTATE_FIRMWARE_WAIT,
                    DSL_AUTOBOOT_FW_WAIT_POLL_TIME);

      /* Set Autoboot Status*/
      nErrCode = DSL_DRV_AutobootStatusSet(pContext, DSL_AUTOBOOT_STATUS_FW_WAIT, nFwType);

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Autoboot Status set failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
            
         break;
      }

      nErrCode = DSL_DRV_EventGenerate( pContext, 0, DSL_ACCESSDIR_NA,
         DSL_XTUDIR_NA, DSL_EVENT_S_FIRMWARE_REQUEST,
         (DSL_EventData_Union_t*)&fwReqData, sizeof(DSL_FirmwareRequestData_t));

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Event(%d) generate failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), DSL_EVENT_S_FIRMWARE_REQUEST));
            
         break;
      }

      /* Activate Autoboot timeout for the Firmware Wait dtate*/
      DSL_DRV_AutobootTimeoutSet(pContext, 10);
#endif
      break;
   } /* while(1)*/

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_AutobootHandleFwRequest, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

static DSL_Error_t DSL_DRV_AutobootHandleFwWait(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_boolean_t bFwRequestHandled = DSL_FALSE;
   DSL_uint32_t currentTime;

   DSL_CTX_READ_SCALAR(pContext, nErrCode, bFwRequestHandled, bFwRequestHandled);

   if (bFwRequestHandled)
   {
      DSL_DEBUG(DSL_DBG_MSG,
         (pContext, "DSL[%02d]: FW request handled..."DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      /* Move to the next state*/
      nErrCode = DSL_DRV_AutobootStateSet(
                    pContext,
                    DSL_AUTOBOOTSTATE_FIRMWARE_READY,
                    DSL_AUTOBOOT_FW_READY_POLL_TIME);

      /* Set Autoboot Status*/
      nErrCode = DSL_DRV_AutobootStatusSet(pContext, DSL_AUTOBOOT_STATUS_RUNNING,
         DSL_FW_REQUEST_NA);

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Autoboot Status set failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
      }

      DSL_CTX_WRITE_SCALAR(pContext, nErrCode, bFwReLoaded, DSL_TRUE);
   }
   else
   {
      DSL_DEBUG( DSL_DBG_MSG,
         (pContext, "DSL[%02d]: Autoboot waiting for the FW..."DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      currentTime = DSL_DRV_TimeMSecGet();
      
      pContext->autobootStartTime = pContext->autobootStartTime > currentTime ?
                                    currentTime : pContext->autobootStartTime;

      /* Check for the Firmware Wait state timeout*/      
      if ((pContext->autobootStartTime + (DSL_uint32_t)(pContext->nAutobootTimeoutLimit*1000))
             < currentTime )
      {
         /* Tru to request FW again */
         nErrCode = DSL_DRV_AutobootStateSet(
                       pContext, DSL_AUTOBOOTSTATE_FIRMWARE_REQUEST,
                       DSL_AUTOBOOT_FW_REQUEST_POLL_TIME);
      }
   }

   return nErrCode;
}

static DSL_Error_t DSL_DRV_AutobootHandleFwReady(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_boolean_t bPowerManagementL3Forced, bWaitBeforeConfigWrite = DSL_FALSE;

   /* Reset bFwRequestHandled flag*/
   DSL_CTX_WRITE_SCALAR(pContext, nErrCode, bFwRequestHandled, DSL_FALSE);

   /* Reset modem*/
   nErrCode = DSL_DRV_LinkReset(pContext);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Modem reset failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return nErrCode;
   }

   DSL_CTX_READ_SCALAR(pContext, nErrCode, bPowerManagementL3Forced,
      bPowerManagementL3Forced);

   /* Get Link Activation option*/
   DSL_CTX_READ_SCALAR(pContext, nErrCode,
      nAutobootConfig.nStateMachineOptions.bWaitBeforeConfigWrite, 
      bWaitBeforeConfigWrite);

   /* Switch to the next state according to the link activation option*/
   if (bWaitBeforeConfigWrite)
   {
      DSL_DRV_AutobootStateSet(pContext, DSL_AUTOBOOTSTATE_CONFIG_WRITE_WAIT,
         DSL_AUTOBOOT_CONFIG_WRITE_WAIT_POLL_TIME);

      /* Set Autoboot Status*/
      DSL_DRV_AutobootStatusSet(pContext, 
         DSL_AUTOBOOT_STATUS_CONFIG_WRITE_WAIT, DSL_FW_REQUEST_NA);

      /* IDLE line state reached*/
      DSL_DRV_LineStateSet(pContext, DSL_LINESTATE_IDLE);
   }
   else
   {
      if (bPowerManagementL3Forced != DSL_FALSE)
      {
         DSL_DRV_AutobootStateSet(pContext, DSL_AUTOBOOTSTATE_L3,
            DSL_AUTOBOOT_L3_POLL_TIME);
      }
      else
      {
         DSL_DRV_AutobootStateSet(pContext, DSL_AUTOBOOTSTATE_INIT,
            DSL_AUTOBOOT_INIT_POLL_TIME);
      }
   }

#if defined(INCLUDE_DSL_PM) 
   /* Resume PM module*/
   nErrCode = DSL_DRV_PM_Resume(pContext);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM resume failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
            
      return nErrCode;
   }
#endif /* #if defined(INCLUDE_DSL_PM)*/

   return(nErrCode);
}

static DSL_Error_t DSL_DRV_AutobootHandleL3(
   DSL_Context_t *pContext,
   DSL_boolean_t bL3Forced)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_AutobootHandleL3"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Call device specific implementation*/
   nErrCode = DSL_DRV_DEV_AutobootHandleL3(pContext, bL3Forced);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_AutobootHandleFwRequest, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return(nErrCode);
}
/** @} DRV_DSL_CPE_COMMON */
