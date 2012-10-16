/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DRV_DSL_CPE_AUTOBOOT_H
#define _DRV_DSL_CPE_AUTOBOOT_H

#ifdef __cplusplus
   extern "C" {
#endif

/** This type represents internal autoboot thread state */
typedef enum DSL_Autoboot_State
{
   DSL_AUTOBOOTSTATE_UNKNOWN = 0,
   DSL_AUTOBOOTSTATE_FIRMWARE_REQUEST = 1,
   DSL_AUTOBOOTSTATE_FIRMWARE_WAIT    = 2,
   DSL_AUTOBOOTSTATE_FIRMWARE_READY = 3,
   DSL_AUTOBOOTSTATE_L3 = 4,
   DSL_AUTOBOOTSTATE_INIT = 5,
   DSL_AUTOBOOTSTATE_TRAIN = 6,
   DSL_AUTOBOOTSTATE_SHOWTIME = 7,
   DSL_AUTOBOOTSTATE_EXCEPTION  = 8,
   DSL_AUTOBOOTSTATE_DIAGNOSTIC = 9,
   DSL_AUTOBOOTSTATE_RESTART    = 10,
   DSL_AUTOBOOTSTATE_CONFIG_WRITE_WAIT = 11,
   DSL_AUTOBOOTSTATE_LINK_ACTIVATE_WAIT = 12,
   DSL_AUTOBOOTSTATE_RESTART_WAIT = 13,
   DSL_AUTOBOOTSTATE_LAST = 14
} DSL_Autoboot_State_t;

/**
   Routine that starts autoboot thread

   \param pContext Pointer to dsl library context structure, [I]

   \return  Return values are defined within the \ref DSL_Error_t definition
    - DSL_SUCCESS in case of success
    - DSL_ERROR if operation failed
    - or any other defined specific error code

   \ingroup DRV_DSL_CPE_INIT
 */
#ifndef SWIG
DSL_Error_t DSL_DRV_AutobootThreadStart(
   DSL_Context_t *pContext
);
#endif

/**
   Routine that stops autoboot thread

   \param pContext Pointer to dsl library context structure, [I]

   \return  Return values are defined within the \ref DSL_Error_t definition
    - DSL_SUCCESS in case of success
    - DSL_ERROR if operation failed
    - or any other defined specific error code

   \ingroup DRV_DSL_CPE_INIT
 */
#ifndef SWIG
DSL_Error_t DSL_DRV_AutobootThreadStop(
   DSL_Context_t *pContext
);
#endif

/**
   Restarts the autoboot thread

   \param pContext Pointer to dsl library context structure, [I]

   \return  Return values are defined within the \ref DSL_Error_t definition
    - DSL_SUCCESS in case of success
    - DSL_ERROR if operation failed
    - or any other defined specific error code

   \ingroup DRV_DSL_CPE_INIT
*/

#ifndef SWIG

DSL_Error_t DSL_AutobootThreadRestart(
   DSL_Context_t *pContext
);

#endif

#ifndef SWIG
DSL_Error_t DSL_DRV_AutobootStateSet(
   DSL_Context_t *pContext,
   DSL_Autoboot_State_t nState,
   DSL_uint32_t nPollTimeSec);
#endif

/**
   This routine handles autoboot state machine

   \param pContext Pointer to dsl library context structure, [I]

   \return  Return values are defined within the \ref DSL_Error_t definition
    - DSL_SUCCESS in case of success
    - DSL_ERROR if operation failed
    - or any other defined specific error code

   \ingroup DRV_DSL_CPE_INIT
*/

#ifndef SWIG

DSL_Error_t DSL_DRV_AutobootStateCheck(
   DSL_Context_t *pContext
);

#endif

/**
   This routine sets autoboot timeout value

   \param pContext Pointer to dsl library context structure, [I]
   \param nTimeout Timeout value, [I]

   \return  Return values are defined within the \ref DSL_Error_t definition
    - DSL_SUCCESS in case of success
    - DSL_ERROR if operation failed
    - or any other defined specific error code

   \ingroup DRV_DSL_CPE_INIT
*/

#ifndef SWIG
DSL_Error_t DSL_DRV_AutobootTimeoutSet(
   DSL_Context_t *pContext,
   DSL_uint32_t nTimeout);
#endif

/** This constant defines default value for autoboot thread poll time, sec */
#define DSL_AUTOBOOT_IDLE_POLL_TIME 1000
#define DSL_AUTOBOOT_L3_POLL_TIME 1000
#define DSL_AUTOBOOT_TRAINING_POLL_TIME 1000
#define DSL_AUTOBOOT_INIT_POLL_TIME 100
#define DSL_AUTOBOOT_SHOWTIME_POLL_TIME 1000
#define DSL_AUTOBOOT_EXCEPTION_POLL_TIME 0
#define DSL_AUTOBOOT_DIAGNOSTIC_POLL_TIME 1000
#define DSL_AUTOBOOT_FW_REQUEST_POLL_TIME 0
#define DSL_AUTOBOOT_FW_WAIT_POLL_TIME 1000
#define DSL_AUTOBOOT_LINK_ACTIVATE_WAIT_POLL_TIME 1000
#define DSL_AUTOBOOT_CONFIG_WRITE_WAIT_POLL_TIME 1000
#define DSL_AUTOBOOT_RESTART_WAIT_POLL_TIME 1000
#define DSL_AUTOBOOT_FW_READY_POLL_TIME 100
#define DSL_AUTOBOOT_RESTART_POLL_TIME 0

#ifdef __cplusplus
}
#endif

#endif /* _DRV_DSL_CPE_AUTOBOOT_H */
