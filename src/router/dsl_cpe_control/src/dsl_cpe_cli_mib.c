/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   DSL CLI, access function implementation
*/

#include "dsl_cpe_control.h"
#if defined(INCLUDE_DSL_CPE_CLI_SUPPORT) && defined(INCLUDE_DSL_ADSL_MIB)

#include "dsl_cpe_cli.h"
#include "dsl_cpe_cli_console.h"
#include "dsl_cpe_debug.h"
#include "drv_dsl_cpe_api.h"
#include "drv_dsl_cpe_api_ioctl.h"
#include "drv_dsl_cpe_api_adslmib.h"
#include "drv_dsl_cpe_api_adslmib_ioctl.h"

#undef DSL_CCA_DBG_BLOCK
#define DSL_CCA_DBG_BLOCK DSL_CCA_DBG_CLI


/* for debugging: */
#ifdef DSL_CLI_LOCAL
#undef DSL_CLI_LOCAL
#endif
#if 1
#define DSL_CLI_LOCAL
#else
#define DSL_CLI_LOCAL static
#endif

extern const char *sFailureReturn;

static const DSL_char_t g_sMibLeg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t flags" DSL_CPE_CRLF
   "   0x1 - line code" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t flags" DSL_CPE_CRLF
   "- DSL_int_t adslLineCode" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_MIB_LineEntryGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   adslLineTableEntry_t pData;
   DSL_uint32_t flags = 0;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(adslLineTableEntry_t));

   DSL_CPE_sscanf (pCommands, "%x", &flags);
   pData.flags = (DSL_uint8_t)flags;

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_MIB_ADSL_LINE_ENTRY_GET, (int) &pData);

   if (ret < 0)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, ret, DSL_CPE_Fd2DevStr(fd));
   }
   else
   {
      DSL_CPE_FPrintf (out, "nReturn=%d%s flags=0x%x adslLineCode=%d"DSL_CPE_CRLF,
         ret, DSL_CPE_Fd2DevStr(fd), pData.flags, pData.adslLineCode);

      DSL_CPE_FPrintf(out, DSL_CPE_CRLF);
   }
   return 0;
}

static const DSL_char_t g_sMibLeeg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t flags" DSL_CPE_CRLF
   "   atuc trans. capabilities = 0x1" DSL_CPE_CRLF
   "   atuc trans. config       = 0x2" DSL_CPE_CRLF
   "   atuc trans. actual       = 0x4" DSL_CPE_CRLF
   "   power state              = 0x8" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t flags" DSL_CPE_CRLF
   "- DSL_uint16_t adslLineTransAtucCap (hex)" DSL_CPE_CRLF
   "- DSL_uint16_t adslLineTransAtucConfig (hex)" DSL_CPE_CRLF
   "- DSL_uint16_t adslLineTransAtucActual (hex)" DSL_CPE_CRLF
   "- DSL_int_t adslLineGlitePowerState" DSL_CPE_CRLF
   "   none = 1" DSL_CPE_CRLF
   "   L0 = 2" DSL_CPE_CRLF
   "   L1 = 3" DSL_CPE_CRLF
   "   L3 = 4" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_MIB_LineExtEntryGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   adslLineExtTableEntry_t pData;
   DSL_uint32_t flags = 0;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(adslLineExtTableEntry_t));

   DSL_CPE_sscanf (pCommands, "%x", &flags);
   pData.flags = (DSL_uint8_t)flags;

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_MIB_ADSL_LINE_EXT_ENTRY_GET, (int) &pData);

   if (ret < 0)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, ret, DSL_CPE_Fd2DevStr(fd));
   }
   else
   {
      DSL_CPE_FPrintf (out, 
         "nReturn=%d%s flags=0x%x adslLineTransAtucCap=0x%x "
         "adslLineTransAtucConfig=0x%x adslLineTransAtucActual=0x%x "
         "adslLineGlitePowerState=%d"DSL_CPE_CRLF,
         ret, DSL_CPE_Fd2DevStr(fd), pData.flags, 
         pData.adslLineTransAtucCap,
         pData.adslLineTransAtucConfig,
         pData.adslLineTransAtucActual,
         pData.adslLineGlitePowerState);

      DSL_CPE_FPrintf(out, DSL_CPE_CRLF);
   }
   return 0;
}

static const DSL_char_t g_sMibAtucpeg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t flags (hex)" DSL_CPE_CRLF
   "   serial number  = 0x1" DSL_CPE_CRLF
   "   vendor id      = 0x2" DSL_CPE_CRLF
   "   version number = 0x4" DSL_CPE_CRLF
   "   line status    = 0x8" DSL_CPE_CRLF
   "   output power   = 0x10" DSL_CPE_CRLF
   "   att. rate      = 0x20" DSL_CPE_CRLF
   "   attenuation    = 0x40" DSL_CPE_CRLF
   "   snr margin     = 0x80" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t flags (hex)" DSL_CPE_CRLF
   "- DSL_char_t serial_no[32]" DSL_CPE_CRLF
   "- DSL_char_t vendor_id[16]" DSL_CPE_CRLF
   "- DSL_uint16_t country_code (hex)" DSL_CPE_CRLF
   "- DSL_char_t provider_id[4]" DSL_CPE_CRLF
   "- DSL_char_t revision_info[2]" DSL_CPE_CRLF
   "- DSL_char_t version_no[16]" DSL_CPE_CRLF
   "- DSL_uint32_t Attn" DSL_CPE_CRLF
   "- DSL_uint32_t SnrMgn" DSL_CPE_CRLF
   "- DSL_uint32_t status" DSL_CPE_CRLF
   "   noDefect            = 0x0" DSL_CPE_CRLF
   "   lossOfFraming       = 0x1" DSL_CPE_CRLF
   "   lossOfSignal        = 0x2" DSL_CPE_CRLF
   "   lossOfPower         = 0x4" DSL_CPE_CRLF
   "   lossOfSignalQuality = 0x8" DSL_CPE_CRLF
   "   lossOfLink          = 0x10" DSL_CPE_CRLF
   "   dataInitFailure     = 0x20" DSL_CPE_CRLF
   "   configInitFailure   = 0x40" DSL_CPE_CRLF
   "   protocolInitFailure = 0x80" DSL_CPE_CRLF
   "   noPeerAtuPresent    = 0x100" DSL_CPE_CRLF
   "- DSL_int_t outputPwr" DSL_CPE_CRLF
   "- DSL_uint32_t attainableRate" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_MIB_ATUC_PhysEntryGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   adslAtucPhysEntry_t pData;
   DSL_char_t buf[256];
   DSL_uint32_t flags = 0;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(adslAtucPhysEntry_t)); 

   DSL_CPE_sscanf (pCommands, "%x", &flags);
   pData.flags = flags;

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_MIB_ADSL_ATUC_PHYS_ENTRY_GET, (int) &pData);

   if (ret < 0)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, ret, DSL_CPE_Fd2DevStr(fd));
   }
   else
   {
      DSL_CPE_ArraySPrintF(buf, pData.serial_no,
         sizeof(pData.serial_no), sizeof(pData.serial_no[0]),
         DSL_ARRAY_FORMAT_HEX);
      DSL_CPE_FPrintf (out, "nReturn=%d%s flags=0x%x serial_no=%s ", ret, DSL_CPE_Fd2DevStr(fd),
         pData.flags, buf);

      DSL_CPE_ArraySPrintF(buf, pData.vendor_id.vendor_id,
         sizeof(pData.vendor_id.vendor_id), sizeof(pData.vendor_id.vendor_id[0]),
         DSL_ARRAY_FORMAT_HEX);
      DSL_CPE_FPrintf (out, "vendor_id=%s ", buf);

      DSL_CPE_FPrintf (out, "country_code=0x%x ", pData.vendor_id.vendor_info.country_code);

      DSL_CPE_ArraySPrintF(buf, pData.vendor_id.vendor_info.provider_id,
         sizeof(pData.vendor_id.vendor_info.provider_id), sizeof(pData.vendor_id.vendor_info.provider_id[0]),
         DSL_ARRAY_FORMAT_HEX);
      DSL_CPE_FPrintf (out, "provider_id=%s ", buf);

      DSL_CPE_ArraySPrintF(buf, pData.vendor_id.vendor_info.revision_info,
         sizeof(pData.vendor_id.vendor_info.revision_info), sizeof(pData.vendor_id.vendor_info.revision_info[0]),
         DSL_ARRAY_FORMAT_HEX);
      DSL_CPE_FPrintf (out, "revision_info=%s ", buf);

      DSL_CPE_ArraySPrintF(buf, pData.version_no,
         sizeof(pData.version_no), sizeof(pData.version_no[0]),
         DSL_ARRAY_FORMAT_HEX);
      DSL_CPE_FPrintf (out, "version_no=%s ", buf);

      DSL_CPE_FPrintf (out, "Attn=%d SnrMgn=%d ", pData.Attn, pData.SnrMgn);

      DSL_CPE_FPrintf (out, "status=%d ", pData.status);
      DSL_CPE_FPrintf (out, "outputPwr=%d ", pData.outputPwr);
      DSL_CPE_FPrintf (out, "attainableRate=%d"DSL_CPE_CRLF, pData.attainableRate);
      DSL_CPE_FPrintf(out, DSL_CPE_CRLF);
   }
   return 0;
}

static const DSL_char_t g_sMibAturpeg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t flags (hex)" DSL_CPE_CRLF
   "   serial number  = 0x1" DSL_CPE_CRLF
   "   vendor id      = 0x2" DSL_CPE_CRLF
   "   version number = 0x4" DSL_CPE_CRLF
   "   line status    = 0x20" DSL_CPE_CRLF
   "   output power   = 0x40" DSL_CPE_CRLF
   "   att. rate      = 0x80" DSL_CPE_CRLF
   "   attenuation    = 0x100" DSL_CPE_CRLF
   "   snr margin     = 0x200" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t flags (hex)" DSL_CPE_CRLF
   "- DSL_char_t serial_no[32]" DSL_CPE_CRLF
   "- DSL_char_t vendor_id[16]" DSL_CPE_CRLF
   "- DSL_uint16_t country_code (hex)" DSL_CPE_CRLF
   "- DSL_char_t provider_id[4]" DSL_CPE_CRLF
   "- DSL_char_t revision_info[2]" DSL_CPE_CRLF
   "- DSL_char_t version_no[16]" DSL_CPE_CRLF
   "- DSL_uint32_t Attn" DSL_CPE_CRLF
   "- DSL_uint32_t SnrMgn" DSL_CPE_CRLF
   "- DSL_uint32_t status" DSL_CPE_CRLF
   "   noDefect            = 0x0" DSL_CPE_CRLF
   "   lossOfFraming       = 0x1" DSL_CPE_CRLF
   "   lossOfSignal        = 0x2" DSL_CPE_CRLF
   "   lossOfPower         = 0x4" DSL_CPE_CRLF
   "   lossOfSignalQuality = 0x8" DSL_CPE_CRLF
   "- DSL_int_t outputPwr" DSL_CPE_CRLF
   "- DSL_uint32_t attainableRate" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_MIB_ATUR_PhysEntryGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   adslAturPhysEntry_t pData;
   DSL_char_t buf[256];
   DSL_uint32_t flags = 0;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(adslAturPhysEntry_t)); 

   DSL_CPE_sscanf (pCommands, "%x", &flags);
   pData.flags = flags;

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_MIB_ADSL_ATUR_PHYS_ENTRY_GET, (int) &pData);

   if (ret < 0)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, ret, DSL_CPE_Fd2DevStr(fd));
   }
   else
   {
      DSL_CPE_ArraySPrintF(buf, pData.serial_no,
         sizeof(pData.serial_no), sizeof(pData.serial_no[0]),
         DSL_ARRAY_FORMAT_HEX);
      DSL_CPE_FPrintf (out, "nReturn=%d%s flags=0x%x serial_no=%s ",
         ret, DSL_CPE_Fd2DevStr(fd), pData.flags, buf);

      DSL_CPE_ArraySPrintF(buf, pData.vendor_id.vendor_id,
         sizeof(pData.vendor_id.vendor_id), sizeof(pData.vendor_id.vendor_id[0]),
         DSL_ARRAY_FORMAT_HEX);
      DSL_CPE_FPrintf (out, "vendor_id=%s ", buf);

      DSL_CPE_FPrintf (out, "country_code=0x%x ", pData.vendor_id.vendor_info.country_code);

      DSL_CPE_ArraySPrintF(buf, pData.vendor_id.vendor_info.provider_id,
         sizeof(pData.vendor_id.vendor_info.provider_id), sizeof(pData.vendor_id.vendor_info.provider_id[0]),
         DSL_ARRAY_FORMAT_HEX);
      DSL_CPE_FPrintf (out, "provider_id=%s ", buf);

      DSL_CPE_ArraySPrintF(buf, pData.vendor_id.vendor_info.revision_info,
         sizeof(pData.vendor_id.vendor_info.revision_info), sizeof(pData.vendor_id.vendor_info.revision_info[0]),
         DSL_ARRAY_FORMAT_HEX);
      DSL_CPE_FPrintf (out, "revision_info=%s ", buf);

      DSL_CPE_ArraySPrintF(buf, pData.version_no,
         sizeof(pData.version_no), sizeof(pData.version_no[0]),
         DSL_ARRAY_FORMAT_HEX);
      DSL_CPE_FPrintf (out, "version_no=%s ", buf);

      DSL_CPE_FPrintf (out, "Attn=%d SnrMgn=%d ", pData.Attn, pData.SnrMgn);

      DSL_CPE_FPrintf (out, "status=%d ", pData.status);
      DSL_CPE_FPrintf (out, "outputPwr=%d ", pData.outputPwr);
      DSL_CPE_FPrintf (out, "attainableRate=%d"DSL_CPE_CRLF, pData.attainableRate);

      DSL_CPE_FPrintf(out, DSL_CPE_CRLF);
   }
   return 0;
}

static const DSL_char_t g_sMibAtucceg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t flags (hex)" DSL_CPE_CRLF
   "   interleaver delay = 0x1" DSL_CPE_CRLF
   "   curr tx rate      = 0x2" DSL_CPE_CRLF
   "   prev tx rate      = 0x4" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t flags (hex)" DSL_CPE_CRLF
   "- DSL_uint32_t interleaveDelay" DSL_CPE_CRLF
   "- DSL_uint32_t currTxRate" DSL_CPE_CRLF
   "- DSL_uint32_t prevTxRate" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_MIB_ATUC_ChanEntryGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   adslAtucChanInfo_t pData;
   DSL_uint32_t flags = 0;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(adslAtucChanInfo_t)); 

   DSL_CPE_sscanf (pCommands, "%x", &flags);
   pData.flags = (DSL_uint8_t)flags;

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_MIB_ADSL_ATUC_CHAN_ENTRY_GET, (int) &pData);

   if (ret < 0)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, ret, DSL_CPE_Fd2DevStr(fd));
   }
   else
   {
      DSL_CPE_FPrintf (out, "nReturn=%d%s flags=0x%x interleaveDelay=%d currTxRate=%d prevTxRate=%d"DSL_CPE_CRLF,
      ret, DSL_CPE_Fd2DevStr(fd),
      pData.flags, pData.interleaveDelay, pData.currTxRate, pData.prevTxRate);
      
      DSL_CPE_FPrintf(out, DSL_CPE_CRLF);
   }
   return 0;
}

static const DSL_char_t g_sMibAturceg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t flags (hex)" DSL_CPE_CRLF
   "   interleaver delay = 0x1" DSL_CPE_CRLF
   "   curr tx rate      = 0x2" DSL_CPE_CRLF
   "   prev tx rate      = 0x4" DSL_CPE_CRLF
   "   crc blk length    = 0x8" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t flags (hex)" DSL_CPE_CRLF
   "- DSL_uint32_t interleaveDelay" DSL_CPE_CRLF
   "- DSL_uint32_t currTxRate" DSL_CPE_CRLF
   "- DSL_uint32_t prevTxRate" DSL_CPE_CRLF
   "- DSL_uint32_t crcBlkLen" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_MIB_ATUR_ChanEntryGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   adslAturChanInfo_t pData;
   DSL_uint32_t flags = 0;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(adslAturChanInfo_t)); 

   DSL_CPE_sscanf (pCommands, "%x", &flags);
   pData.flags = (DSL_uint8_t)flags;

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_MIB_ADSL_ATUR_CHAN_ENTRY_GET, (int) &pData);

   if (ret < 0)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, ret, DSL_CPE_Fd2DevStr(fd));
   }
   else
   {
      DSL_CPE_FPrintf (out, "nReturn=%d%s flags=0x%x interleaveDelay=%d currTxRate=%d prevTxRate=%d "
         "crcBlkLen=%d"DSL_CPE_CRLF,
         ret, DSL_CPE_Fd2DevStr(fd),
         pData.flags, pData.interleaveDelay, pData.currTxRate, pData.prevTxRate, pData.crcBlkLen);
      
      DSL_CPE_FPrintf(out, DSL_CPE_CRLF);
   }
   return 0;
}

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
static const DSL_char_t g_sMibAturpdeg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t flags (hex)" DSL_CPE_CRLF
   "   LOFS              = 0x1" DSL_CPE_CRLF
   "   LOSS              = 0x2" DSL_CPE_CRLF
   "   LPR               = 0x4" DSL_CPE_CRLF
   "   ESS               = 0x8" DSL_CPE_CRLF
   "   valid intervals   = 0x10" DSL_CPE_CRLF
   "   invalid intervals = 0x20" DSL_CPE_CRLF
   "   15min elapsed     = 0x40" DSL_CPE_CRLF
   "   15min LOFS        = 0x80" DSL_CPE_CRLF
   "   15min LOSS        = 0x100" DSL_CPE_CRLF
   "   15min LPR         = 0x200" DSL_CPE_CRLF
   "   15min ESS         = 0x400" DSL_CPE_CRLF
   "   1day elapsed      = 0x800" DSL_CPE_CRLF
   "   1day LOFS         = 0x1000" DSL_CPE_CRLF
   "   1day LOSS         = 0x2000" DSL_CPE_CRLF
   "   1day LPR          = 0x4000" DSL_CPE_CRLF
   "   1day ESS          = 0x8000" DSL_CPE_CRLF
   "   1day elapsed prev = 0x10000" DSL_CPE_CRLF   
   "   1day LOFS prev    = 0x20000" DSL_CPE_CRLF
   "   1day LOSS prev    = 0x40000" DSL_CPE_CRLF
   "   1day LPR prev     = 0x80000" DSL_CPE_CRLF
   "   1day ESS prev     = 0x100000" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t flags (hex)" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturPerfLofs" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturPerfLoss" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturPerfLprs" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturPerfESs" DSL_CPE_CRLF
   "- DSL_int_t adslAturPerfValidIntervals" DSL_CPE_CRLF
   "- DSL_int_t adslAturPerfInvalidIntervals" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturPerfCurr15MinTimeElapsed" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturPerfCurr15MinLofs" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturPerfCurr15MinLoss" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturPerfCurr15MinLprs" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturPerfCurr15MinESs" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturPerfCurr1DayTimeElapsed" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturPerfCurr1DayLofs" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturPerfCurr1DayLoss" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturPerfCurr1DayLprs" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturPerfCurr1DayESs" DSL_CPE_CRLF
   "- DSL_int_t adslAturPerfPrev1DayMoniSecs" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturPerfPrev1DayLofs" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturPerfPrev1DayLoss" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturPerfPrev1DayLprs" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturPerfPrev1DayESs" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_MIB_ATUR_PerfDataEntryGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   aturPerfDataEntry_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(aturPerfDataEntry_t)); 

   DSL_CPE_sscanf (pCommands, "%x", &pData.flags);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_MIB_ADSL_ATUR_PERF_DATA_ENTRY_GET, (int) &pData);

   if (ret < 0)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, ret, DSL_CPE_Fd2DevStr(fd));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         "nReturn=%d%s flags=0x%x "
         "adslAturPerfLofs=%d adslAturPerfLoss=%d adslAturPerfLprs=%d "
         "adslAturPerfESs=%d adslAturPerfValidIntervals=%d adslAturPerfInvalidIntervals=%d "
         "adslAturPerfCurr15MinTimeElapsed=%d adslAturPerfCurr15MinLofs=%d "
         "adslAturPerfCurr15MinLoss=%d adslAturPerfCurr15MinLprs=%d "
         "adslAturPerfCurr15MinESs=%d adslAturPerfCurr1DayTimeElapsed=%d "
         "adslAturPerfCurr1DayLofs=%d adslAturPerfCurr1DayLoss=%d "
         "adslAturPerfCurr1DayLprs=%d adslAturPerfCurr1DayESs=%d "
         "adslAturPerfPrev1DayMoniSecs=%d adslAturPerfPrev1DayLofs=%d "
         "adslAturPerfPrev1DayLoss=%d adslAturPerfPrev1DayLprs=%d "
         "adslAturPerfPrev1DayESs=%d"DSL_CPE_CRLF,
         ret,
         DSL_CPE_Fd2DevStr(fd),
         pData.flags,
         pData.adslAturPerfLofs,
         pData.adslAturPerfLoss,
         pData.adslAturPerfLprs,
         pData.adslAturPerfESs,
         pData.adslAturPerfValidIntervals,
         pData.adslAturPerfInvalidIntervals,
         pData.adslAturPerfCurr15MinTimeElapsed,
         pData.adslAturPerfCurr15MinLofs,
         pData.adslAturPerfCurr15MinLoss,
         pData.adslAturPerfCurr15MinLprs,
         pData.adslAturPerfCurr15MinESs,
         pData.adslAturPerfCurr1DayTimeElapsed,
         pData.adslAturPerfCurr1DayLofs,
         pData.adslAturPerfCurr1DayLoss,
         pData.adslAturPerfCurr1DayLprs,
         pData.adslAturPerfCurr1DayESs,
         pData.adslAturPerfPrev1DayMoniSecs,
         pData.adslAturPerfPrev1DayLofs,
         pData.adslAturPerfPrev1DayLoss,
         pData.adslAturPerfPrev1DayLprs,
         pData.adslAturPerfPrev1DayESs);
      DSL_CPE_FPrintf(out, DSL_CPE_CRLF);
   }
   return 0;
}

static const DSL_char_t g_sMibAtucpdeg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t flags (hex)" DSL_CPE_CRLF
   "   LOFS              = 0x1" DSL_CPE_CRLF
   "   LOSS              = 0x2" DSL_CPE_CRLF
   "   ESS               = 0x4" DSL_CPE_CRLF
   "   INITS              = 0x8" DSL_CPE_CRLF
   "   valid intervals   = 0x10" DSL_CPE_CRLF
   "   invalid intervals = 0x20" DSL_CPE_CRLF
   "   15min elapsed     = 0x40" DSL_CPE_CRLF
   "   15min LOFS        = 0x80" DSL_CPE_CRLF
   "   15min LOSS        = 0x100" DSL_CPE_CRLF
   "   15min ESS         = 0x200" DSL_CPE_CRLF
   "   15min INIT        = 0x400" DSL_CPE_CRLF
   "   1day elapsed      = 0x800" DSL_CPE_CRLF
   "   1day LOFS         = 0x1000" DSL_CPE_CRLF
   "   1day LOSS         = 0x2000" DSL_CPE_CRLF
   "   1day ESS          = 0x4000" DSL_CPE_CRLF
   "   1day INIT         = 0x8000" DSL_CPE_CRLF
   "   1day elapsed prev = 0x10000" DSL_CPE_CRLF   
   "   1day LOFS prev    = 0x20000" DSL_CPE_CRLF
   "   1day LOSS prev    = 0x40000" DSL_CPE_CRLF
   "   1day ESS prev     = 0x80000" DSL_CPE_CRLF
   "   1day INIT prev    = 0x100000" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t flags (hex)" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfLofs" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfLoss" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfESs" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfInits" DSL_CPE_CRLF
   "- DSL_int_t adslAtucPerfValidIntervals" DSL_CPE_CRLF
   "- DSL_int_t adslAtucPerfInvalidIntervals" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfCurr15MinTimeElapsed" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfCurr15MinLofs" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfCurr15MinLoss" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfCurr15MinESs" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfCurr15MinInits" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfCurr1DayTimeElapsed" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfCurr1DayLofs" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfCurr1DayLoss" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfCurr1DayESs" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfCurr1DayInits" DSL_CPE_CRLF
   "- DSL_int_t adslAtucPerfPrev1DayMoniSecs" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfPrev1DayLofs" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfPrev1DayLoss" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfPrev1DayESs" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfPrev1DayInits" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_MIB_ATUC_PerfDataEntryGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   atucPerfDataEntry_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(atucPerfDataEntry_t)); 

   DSL_CPE_sscanf (pCommands, "%x", &pData.flags);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_MIB_ADSL_ATUC_PERF_DATA_ENTRY_GET, (int) &pData);

   if (ret < 0)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, ret, DSL_CPE_Fd2DevStr(fd));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         "nReturn=%d%s flags=0x%x adslAtucPerfLofs=%d adslAtucPerfLoss=%d adslAtucPerfESs=%d "
         "adslAtucPerfInits=%d adslAtucPerfValidIntervals=%d adslAtucPerfInvalidIntervals=%d "
         "adslAtucPerfCurr15MinTimeElapsed=%d adslAtucPerfCurr15MinLofs=%d "
         "adslAtucPerfCurr15MinLoss=%d adslAtucPerfCurr15MinESs=%d "
         "adslAtucPerfCurr15MinInits=%d adslAtucPerfCurr1DayTimeElapsed=%d "
         "adslAtucPerfCurr1DayLofs=%d adslAtucPerfCurr1DayLoss=%d "
         "adslAtucPerfCurr1DayESs=%d adslAtucPerfCurr1DayInits=%d "
         "adslAtucPerfPrev1DayMoniSecs=%d adslAtucPerfPrev1DayLofs=%d "
         "adslAtucPerfPrev1DayLoss=%d adslAtucPerfPrev1DayESs=%d "
         "adslAtucPerfPrev1DayInits=%d"DSL_CPE_CRLF,
         ret,
         DSL_CPE_Fd2DevStr(fd),
         pData.flags,
         pData.adslAtucPerfLofs,
         pData.adslAtucPerfLoss,
         pData.adslAtucPerfESs,
         pData.adslAtucPerfInits,
         pData.adslAtucPerfValidIntervals,
         pData.adslAtucPerfInvalidIntervals,
         pData.adslAtucPerfCurr15MinTimeElapsed,
         pData.adslAtucPerfCurr15MinLofs,
         pData.adslAtucPerfCurr15MinLoss,
         pData.adslAtucPerfCurr15MinESs,
         pData.adslAtucPerfCurr15MinInits,
         pData.adslAtucPerfCurr1DayTimeElapsed,
         pData.adslAtucPerfCurr1DayLofs,
         pData.adslAtucPerfCurr1DayLoss,
         pData.adslAtucPerfCurr1DayESs,
         pData.adslAtucPerfCurr1DayInits,
         pData.adslAtucPerfPrev1DayMoniSecs,
         pData.adslAtucPerfPrev1DayLofs,
         pData.adslAtucPerfPrev1DayLoss,
         pData.adslAtucPerfPrev1DayESs,
         pData.adslAtucPerfPrev1DayInits);

      DSL_CPE_FPrintf(out, DSL_CPE_CRLF);
   }
   return 0;
}

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
static const DSL_char_t g_sMibAtucieg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_int_t IntervalNumber" DSL_CPE_CRLF
   "- DSL_uint32_t flags (hex)" DSL_CPE_CRLF
   "   LOF        = 0x1" DSL_CPE_CRLF
   "   LOS        = 0x2" DSL_CPE_CRLF
   "   ESS        = 0x4" DSL_CPE_CRLF   
   "   INIT       = 0x8" DSL_CPE_CRLF   
   "   VALID DATA = 0x10" DSL_CPE_CRLF   
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t flags (hex)" DSL_CPE_CRLF
   "- DSL_uint32_t intervalLOF" DSL_CPE_CRLF
   "- DSL_uint32_t intervalLOS" DSL_CPE_CRLF
   "- DSL_uint32_t intervalES" DSL_CPE_CRLF
   "- DSL_uint32_t intervalInits" DSL_CPE_CRLF
   "- DSL_int_t intervalValidData" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_MIB_ATUC_IntervalEntryGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   adslAtucIntvlInfo_t pData;
   DSL_uint32_t flags = 0;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(adslAtucIntvlInfo_t)); 

   DSL_CPE_sscanf (pCommands, "%d %x", &pData.IntervalNumber, &flags);
   pData.flags = (DSL_uint8_t)flags;

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_MIB_ADSL_ATUC_INTERVAL_ENTRY_GET, (int) &pData);

   if (ret < 0)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, ret, DSL_CPE_Fd2DevStr(fd));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         "nReturn=%d%s flags=0x%x "
         "intervalLOF=%d intervalLOS=%d intervalES=%d "
         "intervalInits=%d intervalValidData=%d"DSL_CPE_CRLF,
         ret,
         DSL_CPE_Fd2DevStr(fd),
         pData.flags,
         pData.intervalLOF,
         pData.intervalLOS,
         pData.intervalES,
         pData.intervalInits,
         pData.intervalValidData);
      
      DSL_CPE_FPrintf(out, DSL_CPE_CRLF);
   }
   return 0;
}

static const DSL_char_t g_sMibAturieg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_int_t IntervalNumber" DSL_CPE_CRLF
   "- DSL_uint32_t flags (hex)" DSL_CPE_CRLF
   "   LOF        = 0x1" DSL_CPE_CRLF
   "   LOS        = 0x2" DSL_CPE_CRLF
   "   LPR        = 0x4" DSL_CPE_CRLF   
   "   ESS        = 0x8" DSL_CPE_CRLF   
   "   VALID DATA = 0x10" DSL_CPE_CRLF   
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t flags (hex)" DSL_CPE_CRLF
   "- DSL_uint32_t intervalLOF" DSL_CPE_CRLF
   "- DSL_uint32_t intervalLOS" DSL_CPE_CRLF
   "- DSL_uint32_t intervalLPR" DSL_CPE_CRLF
   "- DSL_uint32_t intervalES" DSL_CPE_CRLF
   "- DSL_int_t intervalValidData" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_MIB_ATUR_IntervalEntryGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   adslAturIntvlInfo_t pData;
   DSL_uint32_t flags = 0;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(adslAturIntvlInfo_t)); 

   DSL_CPE_sscanf (pCommands, "%d %x", &pData.IntervalNumber, &flags);
   pData.flags = (DSL_uint8_t)flags;

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_MIB_ADSL_ATUR_INTERVAL_ENTRY_GET, (int) &pData);

   if (ret < 0)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, ret, DSL_CPE_Fd2DevStr(fd));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         "nReturn=%d%s flags=0x%x "
         "intervalLOF=%d intervalLOS=%d intervalLPR=%d "
         "intervalES=%d intervalValidData=%d"DSL_CPE_CRLF,
         ret,
         DSL_CPE_Fd2DevStr(fd),
         pData.flags,
         pData.intervalLOF,
         pData.intervalLOS,
         pData.intervalLPR,
         pData.intervalES,
         pData.intervalValidData);
      
      DSL_CPE_FPrintf(out, DSL_CPE_CRLF);
   }
   return 0;
}
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
static const DSL_char_t g_sMibAtuccpdeg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t flags (hex)" DSL_CPE_CRLF
   "   recv_blk               = 0x1" DSL_CPE_CRLF
   "   tx_blk                 = 0x2" DSL_CPE_CRLF
   "   corr_blk               = 0x4" DSL_CPE_CRLF
   "   uncorr_blk             = 0x8" DSL_CPE_CRLF
   "   valid intervals        = 0x10" DSL_CPE_CRLF
   "   invalid intervals      = 0x20" DSL_CPE_CRLF
   "   15min time elapsed     = 0x40" DSL_CPE_CRLF
   "   15min recv_blk         = 0x80" DSL_CPE_CRLF
   "   15min tx_blk           = 0x100" DSL_CPE_CRLF
   "   15min corr_blk         = 0x200" DSL_CPE_CRLF
   "   15min uncorr_blk       = 0x400" DSL_CPE_CRLF
   "   1day time elapsed      = 0x800" DSL_CPE_CRLF
   "   1day recv_blk          = 0x1000" DSL_CPE_CRLF
   "   1day tx_blk            = 0x2000" DSL_CPE_CRLF
   "   1day corr_blk          = 0x4000" DSL_CPE_CRLF
   "   1day uncorr_blk        = 0x8000" DSL_CPE_CRLF
   "   prev 1day time elapsed = 0x10000" DSL_CPE_CRLF
   "   prev 1day recv_blk     = 0x20000" DSL_CPE_CRLF
   "   prev 1day tx_blk       = 0x40000" DSL_CPE_CRLF
   "   prev 1day corr_blk     = 0x80000" DSL_CPE_CRLF
   "   prev 1day uncorr_blk   = 0x100000" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t flags (hex)" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucChanReceivedBlks" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucChanTransmittedBlks" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucChanCorrectedBlks" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucChanUncorrectBlks" DSL_CPE_CRLF
   "- DSL_int_t adslAtucChanPerfValidIntervals" DSL_CPE_CRLF
   "- DSL_int_t adslAtucChanPerfInvalidIntervals" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucChanPerfCurr15MinTimeElapsed" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucChanPerfCurr15MinReceivedBlks" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucChanPerfCurr15MinTransmittedBlks" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucChanPerfCurr15MinCorrectedBlks" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucChanPerfCurr15MinUncorrectBlks" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucChanPerfCurr1DayTimeElapsed" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucChanPerfCurr1DayReceivedBlks" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucChanPerfCurr1DayTransmittedBlks" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucChanPerfCurr1DayCorrectedBlks" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucChanPerfCurr1DayUncorrectBlks" DSL_CPE_CRLF
   "- DSL_int_t adslAtucChanPerfPrev1DayMoniSecs" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucChanPerfPrev1DayReceivedBlks" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucChanPerfPrev1DayTransmittedBlks" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucChanPerfPrev1DayCorrectedBlks" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucChanPerfPrev1DayUncorrectBlks" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_MIB_ATUC_ChanPerfDataEntryGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   atucChannelPerfDataEntry_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(atucChannelPerfDataEntry_t)); 

   DSL_CPE_sscanf (pCommands, "%x", &pData.flags);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_MIB_ADSL_ATUC_CHAN_PERF_DATA_ENTRY_GET, (int) &pData);

   if (ret < 0)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, ret, DSL_CPE_Fd2DevStr(fd));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         "nReturn=%d%s flags=0x%x "
         "adslAtucChanReceivedBlks=%d adslAtucChanTransmittedBlks=%d "
         "adslAtucChanCorrectedBlks=%d adslAtucChanUncorrectBlks=%d "
         "adslAtucChanPerfValidIntervals=%d adslAtucChanPerfInvalidIntervals=%d "
         "adslAtucChanPerfCurr15MinTimeElapsed=%d adslAtucChanPerfCurr15MinReceivedBlks=%d "
         "adslAtucChanPerfCurr15MinTransmittedBlks=%d adslAtucChanPerfCurr15MinCorrectedBlks=%d "
         "adslAtucChanPerfCurr15MinUncorrectBlks=%d adslAtucChanPerfCurr1DayTimeElapsed=%d "
         "adslAtucChanPerfCurr1DayReceivedBlks=%d adslAtucChanPerfCurr1DayTransmittedBlks=%d "
         "adslAtucChanPerfCurr1DayCorrectedBlks=%d adslAtucChanPerfCurr1DayUncorrectBlks=%d "
         "adslAtucChanPerfPrev1DayMoniSecs=%d adslAtucChanPerfPrev1DayReceivedBlks=%d "
         "adslAtucChanPerfPrev1DayTransmittedBlks=%d adslAtucChanPerfPrev1DayCorrectedBlks=%d "
         "adslAtucChanPerfPrev1DayUncorrectBlks=%d"DSL_CPE_CRLF,
         ret,
         DSL_CPE_Fd2DevStr(fd),
         pData.flags,
         pData.adslAtucChanReceivedBlks,
         pData.adslAtucChanTransmittedBlks,
         pData.adslAtucChanCorrectedBlks,
         pData.adslAtucChanUncorrectBlks,
         pData.adslAtucChanPerfValidIntervals,
         pData.adslAtucChanPerfInvalidIntervals,
         pData.adslAtucChanPerfCurr15MinTimeElapsed,
         pData.adslAtucChanPerfCurr15MinReceivedBlks,
         pData.adslAtucChanPerfCurr15MinTransmittedBlks,
         pData.adslAtucChanPerfCurr15MinCorrectedBlks,
         pData.adslAtucChanPerfCurr15MinUncorrectBlks,
         pData.adslAtucChanPerfCurr1DayTimeElapsed,
         pData.adslAtucChanPerfCurr1DayReceivedBlks,
         pData.adslAtucChanPerfCurr1DayTransmittedBlks,
         pData.adslAtucChanPerfCurr1DayCorrectedBlks,
         pData.adslAtucChanPerfCurr1DayUncorrectBlks,
         pData.adslAtucChanPerfPrev1DayMoniSecs,
         pData.adslAtucChanPerfPrev1DayReceivedBlks,
         pData.adslAtucChanPerfPrev1DayTransmittedBlks,
         pData.adslAtucChanPerfPrev1DayCorrectedBlks,
         pData.adslAtucChanPerfPrev1DayUncorrectBlks);
      
      DSL_CPE_FPrintf(out, DSL_CPE_CRLF);
   }
   return 0;
}

static const DSL_char_t g_sMibAturcpdeg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t flags (hex)" DSL_CPE_CRLF
   "   recv_blk               = 0x1" DSL_CPE_CRLF
   "   tx_blk                 = 0x2" DSL_CPE_CRLF
   "   corr_blk               = 0x4" DSL_CPE_CRLF
   "   uncorr_blk             = 0x8" DSL_CPE_CRLF
   "   valid intervals        = 0x10" DSL_CPE_CRLF
   "   invalid intervals      = 0x20" DSL_CPE_CRLF
   "   15min time elapsed     = 0x40" DSL_CPE_CRLF
   "   15min recv_blk         = 0x80" DSL_CPE_CRLF
   "   15min tx_blk           = 0x100" DSL_CPE_CRLF
   "   15min corr_blk         = 0x200" DSL_CPE_CRLF
   "   15min uncorr_blk       = 0x400" DSL_CPE_CRLF
   "   1day time elapsed      = 0x800" DSL_CPE_CRLF
   "   1day recv_blk          = 0x1000" DSL_CPE_CRLF
   "   1day tx_blk            = 0x2000" DSL_CPE_CRLF
   "   1day corr_blk          = 0x4000" DSL_CPE_CRLF
   "   1day uncorr_blk        = 0x8000" DSL_CPE_CRLF
   "   prev 1day time elapsed = 0x10000" DSL_CPE_CRLF
   "   prev 1day recv_blk     = 0x20000" DSL_CPE_CRLF
   "   prev 1day tx_blk       = 0x40000" DSL_CPE_CRLF
   "   prev 1day corr_blk     = 0x80000" DSL_CPE_CRLF
   "   prev 1day uncorr_blk   = 0x100000" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t flags (hex)" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturChanReceivedBlks" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturChanTransmittedBlks" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturChanCorrectedBlks" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturChanUncorrectBlks" DSL_CPE_CRLF
   "- DSL_int_t adslAturChanPerfValidIntervals" DSL_CPE_CRLF
   "- DSL_int_t adslAturChanPerfInvalidIntervals" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturChanPerfCurr15MinTimeElapsed" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturChanPerfCurr15MinReceivedBlks" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturChanPerfCurr15MinTransmittedBlks" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturChanPerfCurr15MinCorrectedBlks" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturChanPerfCurr15MinUncorrectBlks" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturChanPerfCurr1DayTimeElapsed" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturChanPerfCurr1DayReceivedBlks" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturChanPerfCurr1DayTransmittedBlks" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturChanPerfCurr1DayCorrectedBlks" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturChanPerfCurr1DayUncorrectBlks" DSL_CPE_CRLF
   "- DSL_int_t adslAturChanPerfPrev1DayMoniSecs" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturChanPerfPrev1DayReceivedBlks" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturChanPerfPrev1DayTransmittedBlks" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturChanPerfPrev1DayCorrectedBlks" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturChanPerfPrev1DayUncorrectBlks" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_MIB_ATUR_ChanPerfDataEntryGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   aturChannelPerfDataEntry_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(aturChannelPerfDataEntry_t)); 

   DSL_CPE_sscanf (pCommands, "%x", &pData.flags);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_MIB_ADSL_ATUR_CHAN_PERF_DATA_ENTRY_GET, (int) &pData);

   if (ret < 0)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, ret, DSL_CPE_Fd2DevStr(fd));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         "nReturn=%d%s flags=0x%x "
         "adslAturChanReceivedBlks=%d adslAturChanTransmittedBlks=%d "
         "adslAturChanCorrectedBlks=%d adslAturChanUncorrectBlks=%d "
         "adslAturChanPerfValidIntervals=%d adslAturChanPerfInvalidIntervals=%d "
         "adslAturChanPerfCurr15MinTimeElapsed=%d adslAturChanPerfCurr15MinReceivedBlks=%d "
         "adslAturChanPerfCurr15MinTransmittedBlks=%d adslAturChanPerfCurr15MinCorrectedBlks=%d "
         "adslAturChanPerfCurr15MinUncorrectBlks=%d adslAturChanPerfCurr1DayTimeElapsed=%d "
         "adslAturChanPerfCurr1DayReceivedBlks=%d adslAturChanPerfCurr1DayTransmittedBlks=%d "
         "adslAturChanPerfCurr1DayCorrectedBlks=%d adslAturChanPerfCurr1DayUncorrectBlks=%d "
         "adslAturChanPerfPrev1DayMoniSecs=%d adslAturChanPerfPrev1DayReceivedBlks=%d "
         "adslAturChanPerfPrev1DayTransmittedBlks=%d adslAturChanPerfPrev1DayCorrectedBlks=%d "
         "adslAturChanPerfPrev1DayUncorrectBlks=%d"DSL_CPE_CRLF,
         ret,
         DSL_CPE_Fd2DevStr(fd),
         pData.flags,
         pData.adslAturChanReceivedBlks,
         pData.adslAturChanTransmittedBlks,
         pData.adslAturChanCorrectedBlks,
         pData.adslAturChanUncorrectBlks,
         pData.adslAturChanPerfValidIntervals,
         pData.adslAturChanPerfInvalidIntervals,
         pData.adslAturChanPerfCurr15MinTimeElapsed,
         pData.adslAturChanPerfCurr15MinReceivedBlks,
         pData.adslAturChanPerfCurr15MinTransmittedBlks,
         pData.adslAturChanPerfCurr15MinCorrectedBlks,
         pData.adslAturChanPerfCurr15MinUncorrectBlks,
         pData.adslAturChanPerfCurr1DayTimeElapsed,
         pData.adslAturChanPerfCurr1DayReceivedBlks,
         pData.adslAturChanPerfCurr1DayTransmittedBlks,
         pData.adslAturChanPerfCurr1DayCorrectedBlks,
         pData.adslAturChanPerfCurr1DayUncorrectBlks,
         pData.adslAturChanPerfPrev1DayMoniSecs,
         pData.adslAturChanPerfPrev1DayReceivedBlks,
         pData.adslAturChanPerfPrev1DayTransmittedBlks,
         pData.adslAturChanPerfPrev1DayCorrectedBlks,
         pData.adslAturChanPerfPrev1DayUncorrectBlks);
      
      DSL_CPE_FPrintf(out, DSL_CPE_CRLF);
   }
   return 0;
}

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
static const DSL_char_t g_sMibAtuccieg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_int_t IntervalNumber" DSL_CPE_CRLF
   "- DSL_uint32_t flags (hex)" DSL_CPE_CRLF
   "   interval num   = 0x1" DSL_CPE_CRLF
   "   recv_blk       = 0x2" DSL_CPE_CRLF
   "   tx_blk         = 0x4" DSL_CPE_CRLF
   "   corr_blk       = 0x8" DSL_CPE_CRLF
   "   uncorr_blk     = 0x10" DSL_CPE_CRLF
   "   interval valid = 0x20" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t flags (hex)" DSL_CPE_CRLF
   "- DSL_uint32_t chanIntervalRecvdBlks" DSL_CPE_CRLF
   "- DSL_uint32_t chanIntervalXmitBlks" DSL_CPE_CRLF
   "- DSL_uint32_t chanIntervalCorrectedBlks" DSL_CPE_CRLF
   "- DSL_uint32_t chanIntervalUncorrectBlks" DSL_CPE_CRLF
   "- DSL_int_t intervalValidData" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_MIB_ATUC_ChanIntervalEntryGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   adslAtucChanIntvlInfo_t pData;
   DSL_uint32_t flags = 0;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(adslAtucChanIntvlInfo_t)); 

   DSL_CPE_sscanf (pCommands, "%d %x", &pData.IntervalNumber, &flags);
   pData.flags = (DSL_uint8_t)flags;

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_MIB_ADSL_ATUC_CHAN_INTERVAL_ENTRY_GET, (int) &pData);

   if (ret < 0)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, ret, DSL_CPE_Fd2DevStr(fd));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         "nReturn=%d%s flags=0x%x "
         "chanIntervalRecvdBlks=%d chanIntervalXmitBlks=%d chanIntervalCorrectedBlks=%d "
         "chanIntervalUncorrectBlks=%d intervalValidData=%d"DSL_CPE_CRLF,
         ret,
         DSL_CPE_Fd2DevStr(fd),
         pData.flags,
         pData.chanIntervalRecvdBlks,
         pData.chanIntervalXmitBlks,
         pData.chanIntervalCorrectedBlks,
         pData.chanIntervalUncorrectBlks,
         pData.intervalValidData);
      
      DSL_CPE_FPrintf(out, DSL_CPE_CRLF);
   }
   return 0;
}

static const DSL_char_t g_sMibAturcieg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_int_t IntervalNumber" DSL_CPE_CRLF
   "- DSL_uint32_t flags (hex)" DSL_CPE_CRLF
   "   interval num   = 0x1" DSL_CPE_CRLF
   "   recv_blk       = 0x2" DSL_CPE_CRLF
   "   tx_blk         = 0x4" DSL_CPE_CRLF
   "   corr_blk       = 0x8" DSL_CPE_CRLF
   "   uncorr_blk     = 0x10" DSL_CPE_CRLF
   "   interval valid = 0x20" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t flags (hex)" DSL_CPE_CRLF
   "- DSL_uint32_t chanIntervalRecvdBlks" DSL_CPE_CRLF
   "- DSL_uint32_t chanIntervalXmitBlks" DSL_CPE_CRLF
   "- DSL_uint32_t chanIntervalCorrectedBlks" DSL_CPE_CRLF
   "- DSL_uint32_t chanIntervalUncorrectBlks" DSL_CPE_CRLF
   "- DSL_int_t intervalValidData" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_MIB_ATUR_ChanIntervalEntryGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   adslAturChanIntvlInfo_t pData;
   DSL_uint32_t flags = 0;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(adslAturChanIntvlInfo_t)); 

   DSL_CPE_sscanf (pCommands, "%d %x", &pData.IntervalNumber, &flags);
   pData.flags = (DSL_uint8_t)flags;

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_MIB_ADSL_ATUR_CHAN_INTERVAL_ENTRY_GET, (int) &pData);

   if (ret < 0)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, ret, DSL_CPE_Fd2DevStr(fd));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         "nReturn=%d%s flags=0x%x "
         "chanIntervalRecvdBlks=%d chanIntervalXmitBlks=%d chanIntervalCorrectedBlks=%d "
         "chanIntervalUncorrectBlks=%d intervalValidData=%d"DSL_CPE_CRLF,
         ret,
         DSL_CPE_Fd2DevStr(fd),
         pData.flags,
         pData.chanIntervalRecvdBlks,
         pData.chanIntervalXmitBlks,
         pData.chanIntervalCorrectedBlks,
         pData.chanIntervalUncorrectBlks,
         pData.intervalValidData);
      
      DSL_CPE_FPrintf(out, DSL_CPE_CRLF);
   }
   return 0;
}
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
static const DSL_char_t g_sMibAtucpdeeg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t flags (hex)" DSL_CPE_CRLF
   "   FASTR                  = 0x1" DSL_CPE_CRLF
   "   FAILED_FASTR           = 0x2" DSL_CPE_CRLF
   "   SESL                   = 0x4" DSL_CPE_CRLF
   "   UASL                   = 0x8" DSL_CPE_CRLF
   "   15min FASTR            = 0x10" DSL_CPE_CRLF
   "   15min FAILED_FASTR     = 0x20" DSL_CPE_CRLF
   "   15min SESL             = 0x40" DSL_CPE_CRLF
   "   15min UASL             = 0x80" DSL_CPE_CRLF
   "   1day FASTR             = 0x100" DSL_CPE_CRLF
   "   1day FAILED_FASTR      = 0x200" DSL_CPE_CRLF
   "   1day SESL              = 0x400" DSL_CPE_CRLF
   "   1day UASL              = 0x800" DSL_CPE_CRLF
   "   1day FASTR prev        = 0x1000" DSL_CPE_CRLF
   "   1day FAILED_FASTR prev = 0x2000" DSL_CPE_CRLF
   "   1day SESL prev         = 0x4000" DSL_CPE_CRLF
   "   1day UASL prev         = 0x8000" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t flags (hex)" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfStatFastR" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfStatFailedFastR" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfStatSesL" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfStatUasL" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfCurr15MinFastR" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfCurr15MinFailedFastR" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfCurr15MinSesL" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfCurr15MinUasL" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfCurr1DayFastR" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfCurr1DayFailedFastR" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfCurr1DaySesL" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfCurr1DayUasL" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfPrev1DayFastR" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfPrev1DayFailedFastR" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfPrev1DaySesL" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucPerfPrev1DayUasL" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_MIB_ATUC_PerfDataExtEntryGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   atucPerfDataExtEntry_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(atucPerfDataExtEntry_t)); 

   DSL_CPE_sscanf (pCommands, "%x", &pData.flags);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_MIB_ADSL_ATUC_PERF_DATA_EXT_ENTRY_GET, (int) &pData);

   if (ret < 0)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, ret, DSL_CPE_Fd2DevStr(fd));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         "nReturn=%d%s flags=0x%x "
         "adslAtucPerfStatFastR=%d adslAtucPerfStatFailedFastR=%d "
         "adslAtucPerfStatSesL=%d adslAtucPerfStatUasL=%d "
         "adslAtucPerfCurr15MinFastR=%d adslAtucPerfCurr15MinFailedFastR=%d "
         "adslAtucPerfCurr15MinSesL=%d adslAtucPerfCurr15MinUasL=%d "
         "adslAtucPerfCurr1DayFastR=%d adslAtucPerfCurr1DayFailedFastR=%d "
         "adslAtucPerfCurr1DaySesL=%d adslAtucPerfCurr1DayUasL=%d "
         "adslAtucPerfPrev1DayFastR=%d adslAtucPerfPrev1DayFailedFastR=%d "
         "adslAtucPerfPrev1DaySesL=%d adslAtucPerfPrev1DayUasL=%d"DSL_CPE_CRLF,
         ret,
         DSL_CPE_Fd2DevStr(fd),
         pData.flags,
         pData.adslAtucPerfStatFastR,
         pData.adslAtucPerfStatFailedFastR,
         pData.adslAtucPerfStatSesL,
         pData.adslAtucPerfStatUasL,
         pData.adslAtucPerfCurr15MinFastR,
         pData.adslAtucPerfCurr15MinFailedFastR,
         pData.adslAtucPerfCurr15MinSesL,
         pData.adslAtucPerfCurr15MinUasL,
         pData.adslAtucPerfCurr1DayFastR,
         pData.adslAtucPerfCurr1DayFailedFastR,
         pData.adslAtucPerfCurr1DaySesL,
         pData.adslAtucPerfCurr1DayUasL,
         pData.adslAtucPerfPrev1DayFastR,
         pData.adslAtucPerfPrev1DayFailedFastR,
         pData.adslAtucPerfPrev1DaySesL,
         pData.adslAtucPerfPrev1DayUasL);

      DSL_CPE_FPrintf(out, DSL_CPE_CRLF);
   }
   return 0;
}

static const DSL_char_t g_sMibAturpdeeg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t flags (hex)" DSL_CPE_CRLF
   "   SESL                   = 0x1" DSL_CPE_CRLF
   "   UASL                   = 0x2" DSL_CPE_CRLF
   "   15min SESL             = 0x4" DSL_CPE_CRLF
   "   15min UASL             = 0x8" DSL_CPE_CRLF
   "   1day SESL              = 0x10" DSL_CPE_CRLF
   "   1day UASL              = 0x20" DSL_CPE_CRLF
   "   1day SESL prev         = 0x40" DSL_CPE_CRLF
   "   1day UASL prev         = 0x80" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t flags (hex)" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturPerfStatSesL" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturPerfStatUasL" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturPerfCurr15MinSesL" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturPerfCurr15MinUasL" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturPerfCurr1DaySesL" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturPerfCurr1DayUasL" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturPerfPrev1DaySesL" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturPerfPrev1DayUasL" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_MIB_ATUR_PerfDataExtEntryGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   aturPerfDataExtEntry_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(aturPerfDataExtEntry_t)); 

   DSL_CPE_sscanf (pCommands, "%x", &pData.flags);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_MIB_ADSL_ATUR_PERF_DATA_EXT_ENTRY_GET, (int) &pData);

   if (ret < 0)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, ret, DSL_CPE_Fd2DevStr(fd));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         "nReturn=%d%s flags=0x%x "
         "adslAturPerfStatSesL=%d adslAturPerfStatUasL=%d "
         "adslAturPerfCurr15MinFastR=%d adslAturPerfCurr15MinFailedFastR=%d "
         "adslAturPerfCurr15MinSesL=%d adslAturPerfCurr15MinUasL=%d "
         "adslAturPerfCurr1DaySesL=%d adslAturPerfCurr1DayUasL=%d "
         "adslAturPerfPrev1DaySesL=%d adslAturPerfPrev1DayUasL=%d"DSL_CPE_CRLF,
         ret,
         DSL_CPE_Fd2DevStr(fd),
         pData.flags,
         pData.adslAturPerfStatSesL,
         pData.adslAturPerfStatUasL,
         pData.adslAturPerfCurr15MinSesL,
         pData.adslAturPerfCurr15MinUasL,
         pData.adslAturPerfCurr1DaySesL,
         pData.adslAturPerfCurr1DayUasL,
         pData.adslAturPerfPrev1DaySesL,
         pData.adslAturPerfPrev1DayUasL);

      DSL_CPE_FPrintf(out, DSL_CPE_CRLF);
   }
   return 0;
}

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
static const DSL_char_t g_sMibAtucieeg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_int_t IntervalNumber" DSL_CPE_CRLF
   "- DSL_uint32_t flags (hex)" DSL_CPE_CRLF
   "   FASTR        = 0x1" DSL_CPE_CRLF
   "   FAILED_FASTR = 0x2" DSL_CPE_CRLF
   "   SESL         = 0x4" DSL_CPE_CRLF   
   "   UASL         = 0x8" DSL_CPE_CRLF   
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t flags (hex)" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucIntervalFastR" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucIntervalFailedFastR" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucIntervalSesL" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucIntervalUasL" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_MIB_ATUC_IntervalExtEntryGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   adslAtucInvtlExtInfo_t pData;
   DSL_uint32_t flags = 0;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(adslAtucInvtlExtInfo_t)); 

   DSL_CPE_sscanf (pCommands, "%d %x", &pData.IntervalNumber, &flags);
   pData.flags = (DSL_uint8_t)flags;

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_MIB_ADSL_ATUC_INTERVAL_EXT_ENTRY_GET, (int) &pData);

   if (ret < 0)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, ret, DSL_CPE_Fd2DevStr(fd));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         "nReturn=%d%s flags=0x%x "
         "adslAtucIntervalFastR=%d adslAtucIntervalFailedFastR=%d "
         "adslAtucIntervalSesL=%d adslAtucIntervalUasL=%d"DSL_CPE_CRLF,
         ret,
         DSL_CPE_Fd2DevStr(fd),
         pData.flags,
         pData.adslAtucIntervalFastR,
         pData.adslAtucIntervalFailedFastR,
         pData.adslAtucIntervalSesL,
         pData.adslAtucIntervalUasL);
      
      DSL_CPE_FPrintf(out, DSL_CPE_CRLF);
   }
   return 0;
}

static const DSL_char_t g_sMibAturieeg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_int_t IntervalNumber" DSL_CPE_CRLF
   "- DSL_uint32_t flags (hex)" DSL_CPE_CRLF
   "   SESL         = 0x1" DSL_CPE_CRLF   
   "   UASL         = 0x2" DSL_CPE_CRLF   
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t flags (hex)" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturIntervalSesL" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturIntervalUasL" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_MIB_ATUR_IntervalExtEntryGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   adslAturInvtlExtInfo_t pData;
   DSL_uint32_t flags = 0;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(adslAturInvtlExtInfo_t)); 

   DSL_CPE_sscanf (pCommands, "%d %x", &pData.IntervalNumber, &flags);
   pData.flags = (DSL_uint8_t)flags;

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_MIB_ADSL_ATUR_INTERVAL_EXT_ENTRY_GET, (int) &pData);

   if (ret < 0)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, ret, DSL_CPE_Fd2DevStr(fd));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         "nReturn=%d%s flags=0x%x "
         "adslAturIntervalSesL=%d adslAturIntervalUasL=%d"DSL_CPE_CRLF,
         ret,
         DSL_CPE_Fd2DevStr(fd),
         pData.flags,
         pData.adslAturIntervalSesL,
         pData.adslAturIntervalUasL);
      
      DSL_CPE_FPrintf(out, DSL_CPE_CRLF);
   }
   return 0;
}
#endif /*INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/
#endif /* INCLUDE_DSL_PM*/

static const DSL_char_t g_sMibLacpeg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t flags (hex)" DSL_CPE_CRLF
   "   ATUC 15min LOFS           = 0x1" DSL_CPE_CRLF   
   "   ATUC 15min LOSS           = 0x2" DSL_CPE_CRLF   
   "   ATUC 15min ESS            = 0x4" DSL_CPE_CRLF   
   "   ATUC fast rate UP         = 0x8" DSL_CPE_CRLF
   "   ATUC interleave rate UP   = 0x10" DSL_CPE_CRLF
   "   ATUC fast rate DOWN       = 0x20" DSL_CPE_CRLF
   "   ATUC interleave rate DOWN = 0x40" DSL_CPE_CRLF
   "   ATUC failure trap         = 0x80" DSL_CPE_CRLF
   "   ATUR 15min LOFS           = 0x100" DSL_CPE_CRLF   
   "   ATUR 15min LOSS           = 0x200" DSL_CPE_CRLF   
   "   ATUR 15min LPRS           = 0x400" DSL_CPE_CRLF   
   "   ATUR 15min ESS            = 0x800" DSL_CPE_CRLF   
   "   ATUR fast rate UP         = 0x1000" DSL_CPE_CRLF
   "   ATUR interleave rate UP   = 0x2000" DSL_CPE_CRLF
   "   ATUR fast rate DOWN       = 0x4000" DSL_CPE_CRLF
   "   ATUR interleave rate DOWN = 0x8000" DSL_CPE_CRLF
   "   row status                = 0x10000" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t flags (hex)" DSL_CPE_CRLF
   "- DSL_int_t adslAtucThresh15MinLofs" DSL_CPE_CRLF
   "- DSL_int_t adslAtucThresh15MinLoss" DSL_CPE_CRLF
   "- DSL_int_t adslAtucThresh15MinESs" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucThreshFastRateUp" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucThreshInterleaveRateUp" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucThreshFastRateDown" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucThreshInterleaveRateDown" DSL_CPE_CRLF
   "- DSL_int_t adslAtucInitFailureTrapEnable" DSL_CPE_CRLF
   "- DSL_int_t adslAturThresh15MinLofs" DSL_CPE_CRLF
   "- DSL_int_t adslAturThresh15MinLoss" DSL_CPE_CRLF
   "- DSL_int_t adslAturThresh15MinLprs" DSL_CPE_CRLF
   "- DSL_int_t adslAturThresh15MinESs" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturThreshFastRateUp" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturThreshInterleaveRateUp" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturThreshFastRateDown" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturThreshInterleaveRateDown" DSL_CPE_CRLF
   "- DSL_int_t adslLineAlarmConfProfileRowStatus" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_MIB_LineAlarmConfProfileEntryGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   adslLineAlarmConfProfileEntry_t pData;
   DSL_uint32_t flags = 0;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(adslLineAlarmConfProfileEntry_t)); 

   DSL_CPE_sscanf (pCommands, "%x", &flags);
   pData.flags = flags;

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_MIB_ADSL_LINE_ALARM_CONF_PROFILE_ENTRY_GET, (int) &pData);

   if (ret < 0)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, ret, DSL_CPE_Fd2DevStr(fd));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         "nReturn=%d%s flags=0x%x "
         "adslAtucThresh15MinLofs=%u adslAtucThresh15MinLoss=%u "
         "adslAtucThresh15MinESs=%u adslAtucThreshFastRateUp=%u "
         "adslAtucThreshInterleaveRateUp=%u adslAtucThreshFastRateDown=%u "
         "adslAtucThreshInterleaveRateDown=%u adslAtucInitFailureTrapEnable=%u "
         "adslAturThresh15MinLofs=%u adslAturThresh15MinLoss=%u "
         "adslAturThresh15MinLprs=%u adslAturThresh15MinESs=%u "
         "adslAturThreshFastRateUp=%u adslAturThreshInterleaveRateUp=%u "
         "adslAturThreshFastRateDown=%u adslAturThreshInterleaveRateDown=%u "
         "adslLineAlarmConfProfileRowStatus=%u"DSL_CPE_CRLF,
         ret,
         DSL_CPE_Fd2DevStr(fd),
         pData.flags,
         pData.adslAtucThresh15MinLofs,
         pData.adslAtucThresh15MinLoss,
         pData.adslAtucThresh15MinESs,
         pData.adslAtucThreshFastRateUp,
         pData.adslAtucThreshInterleaveRateUp,         
         pData.adslAtucThreshFastRateDown,
         pData.adslAtucThreshInterleaveRateDown,
         pData.adslAtucInitFailureTrapEnable,         
         pData.adslAturThresh15MinLofs,
         pData.adslAturThresh15MinLoss,
         pData.adslAturThresh15MinLprs,         
         pData.adslAturThresh15MinESs,
         pData.adslAturThreshFastRateUp,
         pData.adslAturThreshInterleaveRateUp,         
         pData.adslAturThreshFastRateDown,
         pData.adslAturThreshInterleaveRateDown,         
         pData.adslLineAlarmConfProfileRowStatus);
      
      DSL_CPE_FPrintf(out, DSL_CPE_CRLF);
   }
   return 0;
}

static const DSL_char_t g_sMibLacpes[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t flags (hex)" DSL_CPE_CRLF
   "   ATUC 15min LOFS           = 0x1" DSL_CPE_CRLF   
   "   ATUC 15min LOSS           = 0x2" DSL_CPE_CRLF   
   "   ATUC 15min ESS            = 0x4" DSL_CPE_CRLF   
   "   ATUC fast rate UP         = 0x8" DSL_CPE_CRLF
   "   ATUC interleave rate UP   = 0x10" DSL_CPE_CRLF
   "   ATUC fast rate DOWN       = 0x20" DSL_CPE_CRLF
   "   ATUC interleave rate DOWN = 0x40" DSL_CPE_CRLF
   "   ATUC failure trap         = 0x80" DSL_CPE_CRLF
   "   ATUR 15min LOFS           = 0x100" DSL_CPE_CRLF   
   "   ATUR 15min LOSS           = 0x200" DSL_CPE_CRLF   
   "   ATUR 15min LPRS           = 0x400" DSL_CPE_CRLF   
   "   ATUR 15min ESS            = 0x800" DSL_CPE_CRLF   
   "   ATUR fast rate UP         = 0x1000" DSL_CPE_CRLF
   "   ATUR interleave rate UP   = 0x2000" DSL_CPE_CRLF
   "   ATUR fast rate DOWN       = 0x4000" DSL_CPE_CRLF
   "   ATUR interleave rate DOWN = 0x8000" DSL_CPE_CRLF
   "   row status                = 0x10000" DSL_CPE_CRLF
   "- DSL_int_t adslAtucThresh15MinLofs" DSL_CPE_CRLF
   "- DSL_int_t adslAtucThresh15MinLoss" DSL_CPE_CRLF
   "- DSL_int_t adslAtucThresh15MinESs" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucThreshFastRateUp" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucThreshInterleaveRateUp" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucThreshFastRateDown" DSL_CPE_CRLF
   "- DSL_uint32_t adslAtucThreshInterleaveRateDown" DSL_CPE_CRLF
   "- DSL_int_t adslAtucInitFailureTrapEnable" DSL_CPE_CRLF
   "- DSL_int_t adslAturThresh15MinLofs" DSL_CPE_CRLF
   "- DSL_int_t adslAturThresh15MinLoss" DSL_CPE_CRLF
   "- DSL_int_t adslAturThresh15MinLprs" DSL_CPE_CRLF
   "- DSL_int_t adslAturThresh15MinESs" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturThreshFastRateUp" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturThreshInterleaveRateUp" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturThreshFastRateDown" DSL_CPE_CRLF
   "- DSL_uint32_t adslAturThreshInterleaveRateDown" DSL_CPE_CRLF
   "- DSL_int_t adslLineAlarmConfProfileRowStatus" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t flags (hex)" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_MIB_LineAlarmConfProfileEntrySet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   adslLineAlarmConfProfileEntry_t pData;
   DSL_uint32_t flags = 0;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 18, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(adslLineAlarmConfProfileEntry_t)); 

   DSL_CPE_sscanf (pCommands, "%x %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
      &flags,
      &pData.adslAtucThresh15MinLofs, &pData.adslAtucThresh15MinLoss,
      &pData.adslAtucThresh15MinESs, &pData.adslAtucThreshFastRateUp,
      &pData.adslAtucThreshInterleaveRateUp, &pData.adslAtucThreshFastRateDown,
      &pData.adslAtucThreshInterleaveRateDown, &pData.adslAtucInitFailureTrapEnable,
      &pData.adslAturThresh15MinLofs, &pData.adslAturThresh15MinLoss,
      &pData.adslAturThresh15MinLprs, &pData.adslAturThresh15MinESs,
      &pData.adslAturThreshFastRateUp, &pData.adslAturThreshInterleaveRateUp,
      &pData.adslAturThreshFastRateDown, &pData.adslAturThreshInterleaveRateDown,
      &pData.adslLineAlarmConfProfileRowStatus);

   pData.flags = flags;

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_MIB_ADSL_LINE_ALARM_CONF_PROFILE_ENTRY_SET, (int) &pData);

   DSL_CPE_FPrintf (out, sFailureReturn, ret, DSL_CPE_Fd2DevStr(fd));
   
   return 0;
}

static const DSL_char_t g_sMibTg[] =
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- adslAturTrapsFlags_t flags (hex)" DSL_CPE_CRLF
   "   ATUC LOFS        = 0x1" DSL_CPE_CRLF
   "   ATUC LOSS        = 0x2" DSL_CPE_CRLF
   "   ATUC ESS         = 0x4" DSL_CPE_CRLF
   "   ATUC rate change = 0x8" DSL_CPE_CRLF
   "   ATUR LOFS        = 0x10" DSL_CPE_CRLF
   "   ATUR LOSS        = 0x20" DSL_CPE_CRLF
   "   ATUR LPRS        = 0x40" DSL_CPE_CRLF
   "   ATUR ESS         = 0x80" DSL_CPE_CRLF
   "   ATUR rate change = 0x100" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_MIB_TrapsGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   adslAturTrapsFlags_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(adslAturTrapsFlags_t)); 

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_MIB_ADSL_TRAPS_GET, (int) &pData);

   if (ret < 0)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, ret, DSL_CPE_Fd2DevStr(fd));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         "nReturn=%d%s flags=0x%x"DSL_CPE_CRLF,
         ret,
         DSL_CPE_Fd2DevStr(fd),
         pData);
      
      DSL_CPE_FPrintf(out, DSL_CPE_CRLF);
   }
   return 0;
}

static const DSL_char_t g_sMibEtg[] =
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- adslAturExtTrapsFlags_t flags (hex)" DSL_CPE_CRLF
   "   ATUC 15min FAILED FASTR = 0x1" DSL_CPE_CRLF
   "   ATUC 15min SESL         = 0x2" DSL_CPE_CRLF
   "   ATUC 15min UASL         = 0x4" DSL_CPE_CRLF
   "   ATUR 15min SESL         = 0x8" DSL_CPE_CRLF
   "   ATUR 15min UASL         = 0x10" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_MIB_ExtTrapsGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   adslAturExtTrapsFlags_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(adslAturExtTrapsFlags_t)); 

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_MIB_ADSL_EXT_TRAPS_GET, (int) &pData);

   if (ret < 0)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, ret, DSL_CPE_Fd2DevStr(fd));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         "nReturn=%d%s flags=0x%x"DSL_CPE_CRLF,
         ret, DSL_CPE_Fd2DevStr(fd), pData);
      
      DSL_CPE_FPrintf(out, DSL_CPE_CRLF);
   }
   return 0;
}

/**
   Register the CLI commands.

   \param pContext Pointer to dsl library context structure, [I]
   \param command optional parameters [I]
*/
DSL_void_t DSL_CPE_CLI_MibCommandsRegister(DSL_void_t)
{
   DSL_CPE_CLI_CMD_ADD_COMM ("mibleg",     "MIB_LineEntryGet",      DSL_CPE_CLI_MIB_LineEntryGet, g_sMibLeg);
   DSL_CPE_CLI_CMD_ADD_COMM ("mibleeg",    "MIB_LineExtEntryGet",   DSL_CPE_CLI_MIB_LineExtEntryGet, g_sMibLeeg);
   DSL_CPE_CLI_CMD_ADD_COMM ("mibatucpeg", "MIB_ATUC_PhysEntryGet", DSL_CPE_CLI_MIB_ATUC_PhysEntryGet, g_sMibAtucpeg);
   DSL_CPE_CLI_CMD_ADD_COMM ("mibaturpeg", "MIB_ATUR_PhysEntryGet", DSL_CPE_CLI_MIB_ATUR_PhysEntryGet, g_sMibAturpeg);
   DSL_CPE_CLI_CMD_ADD_COMM ("mibatucceg", "MIB_ATUC_ChanEntryGet", DSL_CPE_CLI_MIB_ATUC_ChanEntryGet, g_sMibAtucceg);
   DSL_CPE_CLI_CMD_ADD_COMM ("mibaturceg", "MIB_ATUR_ChanEntryGet", DSL_CPE_CLI_MIB_ATUR_ChanEntryGet, g_sMibAturceg);

   DSL_CPE_CLI_CMD_ADD_COMM ("miblacpeg",  "MIB_LineAlarmConfProfileEntryGet", DSL_CPE_CLI_MIB_LineAlarmConfProfileEntryGet, g_sMibLacpeg);
   DSL_CPE_CLI_CMD_ADD_COMM ("miblacpes",  "MIB_LineAlarmConfProfileEntrySet", DSL_CPE_CLI_MIB_LineAlarmConfProfileEntrySet, g_sMibLacpes);

   DSL_CPE_CLI_CMD_ADD_COMM ("mibtg",  "MIB_TrapsGet",    DSL_CPE_CLI_MIB_TrapsGet, g_sMibTg);
   DSL_CPE_CLI_CMD_ADD_COMM ("mibetg", "MIB_ExtTrapsGet", DSL_CPE_CLI_MIB_ExtTrapsGet, g_sMibEtg);

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_CPE_CLI_CMD_ADD_COMM ("mibatucieg",  "MIB_ATUC_IntervalEntryGet", DSL_CPE_CLI_MIB_ATUC_IntervalEntryGet, g_sMibAtucieg);
   DSL_CPE_CLI_CMD_ADD_COMM ("mibaturieg",  "MIB_ATUR_IntervalEntryGet", DSL_CPE_CLI_MIB_ATUR_IntervalEntryGet, g_sMibAturieg);
   DSL_CPE_CLI_CMD_ADD_COMM ("mibatucieeg", "MIB_ATUC_IntervalExtEntryGet", DSL_CPE_CLI_MIB_ATUC_IntervalExtEntryGet, g_sMibAtucieeg);
   DSL_CPE_CLI_CMD_ADD_COMM ("mibaturieeg", "MIB_ATUR_IntervalExtEntryGet", DSL_CPE_CLI_MIB_ATUR_IntervalExtEntryGet, g_sMibAturieeg);
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
   DSL_CPE_CLI_CMD_ADD_COMM ("mibatucpdeg", "MIB_ATUC_PerfDataEntryGet", DSL_CPE_CLI_MIB_ATUC_PerfDataEntryGet, g_sMibAtucpdeg);
   DSL_CPE_CLI_CMD_ADD_COMM ("mibaturpdeg", "MIB_ATUR_PerfDataEntryGet", DSL_CPE_CLI_MIB_ATUR_PerfDataEntryGet, g_sMibAturpdeg);
   DSL_CPE_CLI_CMD_ADD_COMM ("mibatucpdeeg", "MIB_ATUC_PerfDataExtEntryGet", DSL_CPE_CLI_MIB_ATUC_PerfDataExtEntryGet, g_sMibAtucpdeeg);
   DSL_CPE_CLI_CMD_ADD_COMM ("mibaturpdeeg", "MIB_ATUR_PerfDataExtEntryGet", DSL_CPE_CLI_MIB_ATUR_PerfDataExtEntryGet, g_sMibAturpdeeg);
#endif /* INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
   DSL_CPE_CLI_CMD_ADD_COMM ("mibatuccpdeg", "MIB_ATUC_ChanPerfDataEntryGet", DSL_CPE_CLI_MIB_ATUC_ChanPerfDataEntryGet, g_sMibAtuccpdeg);
   DSL_CPE_CLI_CMD_ADD_COMM ("mibaturcpdeg", "MIB_ATUR_ChanPerfDataEntryGet", DSL_CPE_CLI_MIB_ATUR_ChanPerfDataEntryGet, g_sMibAturcpdeg);
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_CPE_CLI_CMD_ADD_COMM ("mibatuccieg", "MIB_ATUC_ChanIntervalEntryGet", DSL_CPE_CLI_MIB_ATUC_ChanIntervalEntryGet, g_sMibAtuccieg);
   DSL_CPE_CLI_CMD_ADD_COMM ("mibaturcieg", "MIB_ATUR_ChanIntervalEntryGet", DSL_CPE_CLI_MIB_ATUR_ChanIntervalEntryGet, g_sMibAturcieg);
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/
#endif /* INCLUDE_DSL_PM*/
}

#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT && INCLUDE_DSL_ADSL_MIB*/
