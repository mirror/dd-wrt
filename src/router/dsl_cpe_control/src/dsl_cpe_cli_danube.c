/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   Danube specific DSL CLI, access function implementation
*/

#include "dsl_cpe_control.h"

#if defined (INCLUDE_DSL_CPE_API_DANUBE)

#include "dsl_cpe_cli.h"
#include "dsl_cpe_debug.h"
#include "drv_dsl_cpe_api.h"
#include "drv_dsl_cpe_api_ioctl.h"
#include "drv_dsl_cpe_cmv_danube.h"

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT

#undef DSL_CCA_DBG_BLOCK
#define DSL_CCA_DBG_BLOCK DSL_CCA_DBG_CLI

extern const char *sFailureReturn;

/* for debugging: */
#ifdef DSL_CLI_LOCAL
#undef DSL_CLI_LOCAL
#endif
#if 0
#define DSL_CLI_LOCAL
#else
#define DSL_CLI_LOCAL static
#endif

#ifndef _MKSTR_1
#define _MKSTR_1(x)    #x
#define _MKSTR(x)      _MKSTR_1(x)
#endif

#define DSL_CMV_MAX_LENGTH_TX     8
#define DSL_CMV_MAX_LENGTH_RX    10


static const DSL_char_t g_sCr[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_char_t sGroup[" _MKSTR(DSL_MAX_CMV_NAME_LENGTH_NO_TERM) "]" DSL_CPE_CRLF
   "   optn | cnfg | cntl | stat | rate | plam | info | test | dsl" DSL_CPE_CRLF
   "- DSL_int_t nAddress (dec)" DSL_CPE_CRLF
   "- DSL_int_t nIndex (dec)" DSL_CPE_CRLF
   "- DSL_int_t nLength (dec)" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
   "- DSL_uint16_t nData[12] (hex)" DSL_CPE_CRLF DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_DBG_CmvRead(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t nRet = 0;
   DSL_char_t sGroup[DSL_MAX_CMV_NAME_LENGTH] = {0};
   DSL_uint16_t nData[DSL_MAX_CMV_MSG_PAYLOAD_LENGTH] = {0};
   DSL_int_t nAddress = 0, nIndex = 0, nSize = 0, i = 0, j = 0;
   DSL_int_t nSendLen = 0, nMsgCnt = 0;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 3, DSL_CLI_MIN) == DSL_FALSE)
   {
      return -1;
   }
   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 4, DSL_CLI_MAX) == DSL_FALSE)
   {
      return -1;
   }

   nRet = DSL_CPE_sscanf (pCommands, "%s %d %d %d", sGroup, &nAddress, &nIndex, &nSize);
   if ((nRet < 3) || (nSize < 0) || (nAddress < 0) || (nIndex < 0))
   {
      return -1;
   }
   else if (nRet == 3)
   {
      nSize = 1;
   }

   nMsgCnt = (nSize / DSL_MAX_CMV_MSG_PAYLOAD_LENGTH) + 1;

   for (i = 0; i < nMsgCnt; i++)
   {
      if (i == (nMsgCnt -1))
      {
         nSendLen = nSize - (i * DSL_MAX_CMV_MSG_PAYLOAD_LENGTH);
      }
      else
      {
         nSendLen = DSL_MAX_CMV_MSG_PAYLOAD_LENGTH;
      }

      nRet = DSL_CMV_Read (DSL_CPE_GetGlobalContext(), sGroup,
                (DSL_uint16_t)nAddress, (DSL_uint16_t)nIndex, nSendLen, &nData[0]);
      if (nRet < 0)
      {
         DSL_CPE_FPrintf (out, "nReturn=%d"DSL_CPE_CRLF, nRet);
      }
      else
      {
         nIndex += nSendLen;

         if (i == 0)
         {
            DSL_CPE_FPrintf (out, "nReturn=%d nData=\"" DSL_CPE_CRLF, nRet);
         }
         for (j = 0; j < nSendLen; j++)
         {
            DSL_CPE_FPrintf (out, "0x%04X ", nData[j]);
         }
         DSL_CPE_FPrintf (out, DSL_CPE_CRLF);
      }
   }
   DSL_CPE_FPrintf (out, "\"" DSL_CPE_CRLF);

   return 0;
}

static const DSL_char_t g_sCw[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_char_t sGroup[" _MKSTR(DSL_MAX_CMV_NAME_LENGTH_NO_TERM) "]" DSL_CPE_CRLF
   "   optn | cnfg | cntl | stat | rate | plam | info | test | dsl" DSL_CPE_CRLF
   "- DSL_int_t nAddress (dec)" DSL_CPE_CRLF
   "- DSL_int_t nIndex (dec)" DSL_CPE_CRLF
   "- DSL_uint16_t nData[12] (hex)" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_DBG_CmvWrite(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t nRet = 0;
   DSL_char_t sGroup[DSL_MAX_CMV_NAME_LENGTH] = {0};
   DSL_uint16_t nMsgPayload[DSL_MAX_CMV_MSG_PAYLOAD_LENGTH] = {0};
   DSL_uint32_t nValue = 0;
   DSL_uint16_t nAddress = 0, nIndex = 0;
   DSL_int_t nSize = 1;
   DSL_char_t string[100] = { 0 };
   DSL_char_t seps[]   = " ";
   DSL_char_t *token;
   DSL_int_t i = 0, nParams = 0;

   nParams = DSL_MAX_CMV_MSG_PAYLOAD_LENGTH + 3;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 4, DSL_CLI_MIN) == DSL_FALSE)
   {
      return -1;
   }

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, nParams, DSL_CLI_MAX) == DSL_FALSE)
   {
      return -1;
   }

   strncpy (string, pCommands, sizeof(string)-1);
   string[sizeof(string)-1]=0;

   /* Get first token */
   token = strtok (string, seps);
   if (token != DSL_NULL)
   {
      for (i = 0; i < nParams; i++)
      {
         if (i == 0)
         {
            /* First token is group name string */
            DSL_CPE_sscanf (token, "%s", &sGroup);
         }
         else if (i == 1)
         {
            /* Second token is address */
            DSL_CPE_sscanf (token, "%hu", &nAddress);
         }
         else if (i == 2)
         {
            /* Third token is index */
            DSL_CPE_sscanf (token, "%hu", &nIndex);
         }
         else
         {
            /* Following tokens includes message data */
            DSL_CPE_sscanf (token, "%x", &nValue);
            nMsgPayload[i - 3] = (DSL_uint16_t)nValue;
         }

         /* Get next token */
         token = strtok(DSL_NULL, seps);

         /* Exit scanning if no further information is included */
         if (token == DSL_NULL)
         {
            break;
         }
      }
   }
   nSize = (i - 3) + 1;

   nRet = (DSL_int_t)DSL_CMV_Write (DSL_CPE_GetGlobalContext(), sGroup, nAddress, nIndex,
      nSize, &(nMsgPayload[0]));
   if (nRet < 0)
   {
      DSL_CPE_FPrintf (out, "nReturn=%d"DSL_CPE_CRLF, nRet);
   }
   else
   {
      DSL_CPE_FPrintf (out, "nReturn=%d" DSL_CPE_CRLF, nRet);
   }

   return 0;
}

static const DSL_char_t g_sDms[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint16_t nData[" _MKSTR(DSL_MAX_CMV_MSG_LENGTH)"] (hex)" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
   "- DSL_uint16_t nData[" _MKSTR(DSL_MAX_CMV_MSG_LENGTH)"] (hex)" DSL_CPE_CRLF DSL_CPE_CRLF "";
#else
   "";
#endif
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_DBG_DeviceMessageSend(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_DeviceMessage_t pMsg;
   DSL_uint16_t msg[DSL_MAX_CMV_MSG_LENGTH] = {0};
   DSL_uint16_t nPrintSize = 0;
   DSL_char_t string[100] = { 0 };
   DSL_char_t seps[]   = " ";
   DSL_char_t *token;
   DSL_uint32_t nValue = 0;
   DSL_int_t nSize = 0, i = 0;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 4, DSL_CLI_MIN) == DSL_FALSE)
   {
      return -1;
   }

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, DSL_MAX_CMV_MSG_LENGTH, DSL_CLI_MAX) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pMsg, 0, sizeof(pMsg));

   strncpy (string, pCommands, sizeof(string)-1);
   string[sizeof(string)-1]=0;

   /* Get number of included data words */
   nSize = 0;
   /* Get first token */
   token = strtok (string, seps);
   if (token != DSL_NULL)
   {
      for (i = 0; i < DSL_MAX_CMV_MSG_LENGTH; ++i)
      {
         /* Tokens includes message data */
         DSL_CPE_sscanf (token, "%x", &nValue);
         msg[i] = (DSL_uint16_t)nValue;

         /* Get next token */
         token = strtok(DSL_NULL, seps);

         /* Exit scanning if no further information is included */
         if (token == DSL_NULL)
         {
            break;
         }
      }
   }
   nSize = i + 1;

   if (nSize < DSL_CMV_HEADER_LENGTH)
   {
      return -1;
   }

   pMsg.data.nSizeTx = (DSL_uint16_t)(nSize * 2);

   pMsg.data.pMsg = (DSL_uint8_t *)&msg[0];

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_DBG_DEVICE_MESSAGE_SEND, (int) &pMsg);

   if ((ret < 0) && (pMsg.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, "nReturn=%d"DSL_CPE_CRLF, ret);
   }
   else
   {
      DSL_CPE_FPrintf (out, "nReturn=%d nSizeTx=%hu nSizeRx=%hu",
         pMsg.accessCtl.nReturn, pMsg.data.nSizeTx, pMsg.data.nSizeRx);

      if (pMsg.data.nSizeRx)
      {
         DSL_CPE_FPrintf (out, " nData=\"");
         nPrintSize = pMsg.data.nSizeRx - DSL_CMV_HEADER_LENGTH * 2;

         for (i = 0; i < (nPrintSize/2 + nPrintSize%2); i++)
         {
            DSL_CPE_FPrintf (out, "0x%04X ", msg[i + DSL_CMV_HEADER_LENGTH]);
         }

         DSL_CPE_FPrintf (out, " \"");
      }

      DSL_CPE_FPrintf (out, DSL_CPE_CRLF);
   }

   return 0;
}

static const DSL_char_t g_sVig[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
   "- DSL_char_t DSL_DriverVersionApi[" _MKSTR(MAX_INFO_STRING_LEN)"]"DSL_CPE_CRLF
   "- DSL_char_t DSL_ChipSetFWVersion[" _MKSTR(MAX_INFO_STRING_LEN)"]"DSL_CPE_CRLF
   "- DSL_char_t DSL_ChipSetHWVersion[" _MKSTR(MAX_INFO_STRING_LEN)"]"DSL_CPE_CRLF
   "- DSL_char_t DSL_ChipSetType[" _MKSTR(MAX_INFO_STRING_LEN)"]"DSL_CPE_CRLF
   "- DSL_char_t DSL_DriverVersionMeiBsp[" _MKSTR(MAX_INFO_STRING_LEN)"]"DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_VersionInformationGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_VersionInformation_t pData;
   DSL_CPE_Control_Context_t *pCtrlCtx = DSL_NULL;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_VERSION_INFORMATION_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, "nReturn=%d"DSL_CPE_CRLF, pData.accessCtl.nReturn);
   }
   else
   {
      pCtrlCtx = DSL_CPE_GetGlobalContext();
      if(pCtrlCtx == DSL_NULL)
      {
         DSL_CPE_FPrintf (out, "nReturn=%d"DSL_CPE_CRLF, -1);
      }

      if ((pCtrlCtx->driverVer.nMajor != pCtrlCtx->applicationVer.nMajor) ||
          (pCtrlCtx->driverVer.nMinor != pCtrlCtx->applicationVer.nMinor))
      {
         pData.accessCtl.nReturn = DSL_WRN_VERSION_INCOMPATIBLE;
      }

      DSL_CPE_FPrintf (out,
         "nReturn=%d DSL_DriverVersionApi=%s DSL_ChipSetFWVersion=%s "
         "DSL_ChipSetHWVersion=%s DSL_ChipSetType=%s DSL_DriverVersionMeiBsp=%s"
         DSL_CPE_CRLF, pData.accessCtl.nReturn, pData.data.DSL_DriverVersionApi,
         pData.data.DSL_ChipSetFWVersion, pData.data.DSL_ChipSetHWVersion,
         pData.data.DSL_ChipSetType, pData.data.DSL_DriverVersionMeiBsp);
   }

   return 0;
}

#ifdef INCLUDE_DSL_CONFIG_GET
static const DSL_char_t g_sIfcg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
   "- DSL_boolean_t bFtTxPotsHp" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_SnrMarginRebootMode_t nSnrMarginRebootMode" DSL_CPE_CRLF
   "   automode_api = 0" DSL_CPE_CRLF
   "   automode_fe = 1" DSL_CPE_CRLF
   "   automode_fw = 2" DSL_CPE_CRLF
   "   manual_off = 3" DSL_CPE_CRLF
   "   manual_user = 4" DSL_CPE_CRLF
   "- DSL_int16_t nUserMinSnrMargin (valid range -320..310)" DSL_CPE_CRLF
   "- DSL_boolean_t bL2modeDisable" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_boolean_t bL3modeDisable" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_boolean_t bFeRequestOff" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_InteropFeatureConfigGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_InteropFeatureConfig_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }


   ret = DSL_CPE_Ioctl (fd, DSL_FIO_INTEROP_FEATURE_CONFIG_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, "nReturn=%d"DSL_CPE_CRLF, pData.accessCtl.nReturn);
   }
   else
   {
      DSL_CPE_FPrintf (out, "nReturn=%d bFtTxPotsHp=%u nSnrMarginRebootMode=%u nUserMinSnrMargin=%d "
         "bL2modeDisable=%u bL3modeDisable=%u bFeRequestOff=%u"
         DSL_CPE_CRLF, pData.accessCtl.nReturn,
         pData.data.bFtTxPotsHp,
         pData.data.nSnrMarginRebootCfg.nSnrMarginRebootMode,
         pData.data.nSnrMarginRebootCfg.nUserMinSnrMargin,
         pData.data.bL2modeDisable, pData.data.bL3modeDisable,
         pData.data.bFeRequestOff);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CONFIG_GET*/

static const DSL_char_t g_sIfcs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_boolean_t bFtTxPotsHp" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_SnrMarginRebootMode_t nSnrMarginRebootMode" DSL_CPE_CRLF
   "   automode_api = 0" DSL_CPE_CRLF
   "   automode_fe = 1" DSL_CPE_CRLF
   "   automode_fw = 2" DSL_CPE_CRLF
   "   manual_off = 3" DSL_CPE_CRLF
   "   manual_user = 4" DSL_CPE_CRLF
   "- DSL_int16_t nUserMinSnrMargin" DSL_CPE_CRLF
   "   valid range = -320..310" DSL_CPE_CRLF
   "- DSL_boolean_t bL2modeDisable" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_boolean_t bL3modeDisable" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_boolean_t bFeRequestOff" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_InteropFeatureConfigSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_InteropFeatureConfig_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 6, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   DSL_CPE_sscanf (pCommands, "%u %u %hd %u %u %u",
      &(pData.data.bFtTxPotsHp),
      &(pData.data.nSnrMarginRebootCfg.nSnrMarginRebootMode),
      &(pData.data.nSnrMarginRebootCfg.nUserMinSnrMargin),
      &(pData.data.bL2modeDisable),
      &(pData.data.bL3modeDisable), &(pData.data.bFeRequestOff));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_INTEROP_FEATURE_CONFIG_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, "nReturn=%d"DSL_CPE_CRLF, pData.accessCtl.nReturn);
   }
   else
   {
      DSL_CPE_FPrintf (out, "nReturn=%d" DSL_CPE_CRLF, pData.accessCtl.nReturn);
   }

   return 0;
}

#ifdef INCLUDE_DEVICE_EXCEPTION_CODES
static const DSL_char_t g_sLecg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
   "- DSL_uint32_t nError1" DSL_CPE_CRLF
   "- DSL_uint32_t nError2" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_DBG_LastExceptionCodesGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_DBG_LastExceptionCodes_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_DBG_LAST_EXCEPTION_CODES_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, "nReturn=%d"DSL_CPE_CRLF, pData.accessCtl.nReturn);
   }
   else
   {
      DSL_CPE_FPrintf (out, "nReturn=%d nError1=%u nError2=%u"
         DSL_CPE_CRLF, pData.accessCtl.nReturn,
         pData.data.nError1, pData.data.nError2);
   }

   return 0;
}
#endif /* INCLUDE_DEVICE_EXCEPTION_CODES*/

#ifndef DSL_CPE_DEBUG_DISABLE
static const DSL_char_t g_sDfcs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_DebugFeatureSelector_t nConfigSelector" DSL_CPE_CRLF
   "   DSL_DFC_DATA_LED_BEHAVIOR = 0" DSL_CPE_CRLF
   "   DSL_DFC_DATA_LED_BLINK_TIMEOUT = 1" DSL_CPE_CRLF
   "- DSL_int32_t nFeatureValue" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_DBG_DebugFeatureConfigSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_DBG_DebugFeatureConfig_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   DSL_CPE_sscanf (pCommands, "%u %u",
      &(pData.data.nConfigSelector),
      &(pData.data.nFeatureValue));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_DBG_DEBUG_FEATURE_CONFIG_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, "nReturn=%d"DSL_CPE_CRLF, pData.accessCtl.nReturn);
   }
   else
   {
      DSL_CPE_FPrintf (out, "nReturn=%d" DSL_CPE_CRLF, pData.accessCtl.nReturn);
   }

   return 0;
}

static const DSL_char_t g_sDajes[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_int32_t nEnable" DSL_CPE_CRLF
   "     off        = 0" DSL_CPE_CRLF
   "     on         = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_DBG_DebugArcJtagEnableSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0, nIoctlRet = 0, mei_fd = 0;
   DSL_int32_t nEnable = 0;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   DSL_CPE_sscanf (pCommands, "%u", &nEnable);

   mei_fd = DSL_CPE_DEV_DeviceOpen(DSL_CPE_IFX_LOW_DEV, 0);

   if (mei_fd > 0)
   {
      nIoctlRet = DSL_CPE_Ioctl (mei_fd, DSL_FIO_BSP_JTAG_ENABLE, (int) &nEnable);

     ret = (DSL_int32_t) (DSL_CPE_Close (mei_fd));
      if (ret < 0)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR,
            (DSL_CPE_PREFIX"error on closing MEI BSP device!" DSL_CPE_CRLF));
      }
   }
   else
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "Error: MEI BSP device file open failed!" DSL_CPE_CRLF));
   }

   DSL_CPE_FPrintf (out, "nReturn=%d"DSL_CPE_CRLF, ret);

   return 0;
}


#ifdef INCLUDE_DSL_CONFIG_GET
static const DSL_char_t g_sDfcg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_DebugFeatureSelector_t nConfigSelector" DSL_CPE_CRLF
   "   DSL_DFC_DATA_LED_BEHAVIOR = 0" DSL_CPE_CRLF
   "   DSL_DFC_DATA_LED_BLINK_TIMEOUT = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
   "- DSL_int32_t nFeatureValue" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_DBG_DebugFeatureConfigGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_DBG_DebugFeatureConfig_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_DBG_DebugFeatureConfig_t));

   DSL_CPE_sscanf (pCommands, "%u",
      &(pData.data.nConfigSelector));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_DBG_DEBUG_FEATURE_CONFIG_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, "nReturn=%d"DSL_CPE_CRLF, pData.accessCtl.nReturn);
   }
   else
   {
      DSL_CPE_FPrintf (out, "nReturn=%d nFeatureValue=%u"
         DSL_CPE_CRLF, pData.accessCtl.nReturn,
         pData.data.nFeatureValue);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CONFIG_GET*/
#endif /* DSL_CPE_DEBUG_DISABLE*/

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
static const DSL_char_t g_sPMccseg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   most recent showtime = 0" DSL_CPE_CRLF
   "   former most recent showtime = 1.." _MKSTR(DSL_PM_CHANNEL_SHOWTIME_RECORDS_NUM) "" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   most recent showtime = 0" DSL_CPE_CRLF
   "   former most recent showtime = 1.." _MKSTR(DSL_PM_CHANNEL_SHOWTIME_RECORDS_NUM) "" DSL_CPE_CRLF
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nSuperFrame" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_ChannelCountersShowtimeExtGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_ChannelCountersExt_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 3, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   DSL_CPE_sscanf (pCommands, "%bu %u %u", &pData.nChannel, &pData.nDirection,
      &pData.nHistoryInterval);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_CHANNEL_COUNTERS_SHOWTIME_EXT_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, "nReturn=%d"DSL_CPE_CRLF, pData.accessCtl.nReturn);
   }
   else
   {
      DSL_CPE_FPrintf (out,
         "nReturn=%d nChannel=%hu nDirection=%u nHistoryInterval=%u "
         "nElapsedTime=%u bValid=%u nSuperFrame=%u" DSL_CPE_CRLF,
         pData.accessCtl.nReturn, pData.nChannel, pData.nDirection,
         pData.nHistoryInterval, pData.interval.nElapsedTime,
         pData.interval.bValid, pData.data.nSuperFrame);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

DSL_void_t DSL_CPE_CLI_DeviceCommandsRegister (DSL_void_t)
{
   /* Debug functionalities */
   DSL_CPE_CLI_CMD_ADD_COMM ("cr", "CmvRead", DSL_CPE_CLI_DBG_CmvRead, g_sCr);
   DSL_CPE_CLI_CMD_ADD_COMM ("cw", "CmvWrite", DSL_CPE_CLI_DBG_CmvWrite, g_sCw);
   DSL_CPE_CLI_CMD_ADD_COMM ("dms", "DeviceMessageSend", DSL_CPE_CLI_DBG_DeviceMessageSend, g_sDms);
   DSL_CPE_CLI_CMD_ADD_COMM ("vig", "VersionInformationGet", DSL_CPE_CLI_VersionInformationGet, g_sVig);
#ifdef INCLUDE_DSL_CONFIG_GET
   DSL_CPE_CLI_CMD_ADD_COMM ("ifcg", "InteropFeatureConfigGet", DSL_CPE_CLI_InteropFeatureConfigGet, g_sIfcg);
#endif /* INCLUDE_DSL_CONFIG_GET*/
   DSL_CPE_CLI_CMD_ADD_COMM ("ifcs", "InteropFeatureConfigSet", DSL_CPE_CLI_InteropFeatureConfigSet, g_sIfcs);
#ifdef INCLUDE_DEVICE_EXCEPTION_CODES
   DSL_CPE_CLI_CMD_ADD_COMM ("lecg", "LastExceptionCodesGet", DSL_CPE_CLI_DBG_LastExceptionCodesGet, g_sLecg);
#endif /* INCLUDE_DEVICE_EXCEPTION_CODES*/
#ifndef DSL_CPE_DEBUG_DISABLE
   DSL_CPE_CLI_CMD_ADD_COMM ("dfcs", "DebugFeatureConfigSet", DSL_CPE_CLI_DBG_DebugFeatureConfigSet, g_sDfcs);
   DSL_CPE_CLI_CMD_ADD_COMM ("dajes", "DebugArcJtagEnableSet", DSL_CPE_CLI_DBG_DebugArcJtagEnableSet, g_sDajes);
#ifdef INCLUDE_DSL_CONFIG_GET
   DSL_CPE_CLI_CMD_ADD_COMM ("dfcg", "DebugFeatureConfigGet", DSL_CPE_CLI_DBG_DebugFeatureConfigGet, g_sDfcg);
#endif /* INCLUDE_DSL_CONFIG_GET*/
#endif /* DSL_CPE_DEBUG_DISABLE*/
#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_CPE_CLI_CMD_ADD_COMM ("pmccseg", "PM_ChannelCountersShowtimeExtGet", DSL_CPE_CLI_PM_ChannelCountersShowtimeExtGet, g_sPMccseg);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */
}

#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

#endif /* #if defined (INCLUDE_DSL_CPE_API_DANUBE)*/
