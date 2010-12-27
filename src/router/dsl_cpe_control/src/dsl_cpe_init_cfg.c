/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/*
Includes
*/
#include "dsl_cpe_control.h"
#include "drv_dsl_cpe_api.h"
#include "dsl_cpe_init_cfg.h"

#undef DSL_CCA_DBG_BLOCK
#define DSL_CCA_DBG_BLOCK DSL_CCA_DBG_APP

#ifndef _lint
DSL_InitData_t gInitCfgData =
{
   DSL_CPE_FW_SET(DSL_NULL, 0x0, DSL_NULL, 0x0),
   DSL_CPE_XTU_SET(0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7),
   DSL_CPE_LINE_INV_NE_SET(DSL_NULL),
   DSL_CPE_AUTOBOOT_CTRL_SET(DSL_AUTOBOOT_CTRL_START),
   DSL_CPE_AUTOBOOT_CFG_SET(DSL_FALSE, DSL_FALSE, DSL_FALSE),
   DSL_CPE_TEST_MODE_CTRL_SET(DSL_TESTMODE_DISABLE),
   DSL_CPE_LINE_ACTIVATE_CTRL_SET(DSL_G997_INHIBIT_LDSF, DSL_G997_INHIBIT_ACSF, DSL_G997_NORMAL_STARTUP),
   /** Device specific configuration parameters*/
#ifdef INCLUDE_DSL_CPE_API_VINAX
   {
   DSL_CPE_LL_CFG_SET(DSL_DEV_RX_TX_GAINS_NA, -1, -1, DSL_DEV_CLOCKOUT_NA, DSL_DEV_FILTER_POTS_3, \
      DSL_DEV_HDLCMODE_NA, DSL_DEV_MIICLKSOURCE_NA, DSL_DEV_MIICLKCHAIN_ON, DSL_DEV_TCMODE_VPHY, \
      DSL_DEV_MIIMODE_MII, DSL_DEV_LINEMODE_VDSL2_B43, DSL_DEV_HYBRID_AD1_138_17_CPE_R2, -1, 14, \
      DSL_DEV_PARITY_ODD, 0xFF, DSL_BUS_WIDTH_16B, DSL_DEV_PARITY_ODD, \
      0xFF, 64, 10000, DSL_TRUE, DSL_TRUE, DSL_FALSE),
   DSL_CPE_SIC_SET(DSL_TC_EFM, DSL_EMF_TC_CLEANED, DSL_EMF_TC_CLEANED, DSL_SYSTEMIF_MII),
   DSL_CPE_MAC_CFG_SET(DSL_EFM_SPEED_100, DSL_EFM_DUPLEX_FULL, DSL_EFM_FLOWCTRL_ON, DSL_EFM_AUTONEG_OFF, \
      0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 1536, DSL_FALSE),
   DSL_CPE_SAR_CFG_SET(DSL_SAR_FILTER_MAC_DEST_ADDR, 32, 64, DSL_SAR_MUX_VC_MUX, DSL_FALSE)
   }
#endif
#if defined (INCLUDE_DSL_CPE_API_DANUBE)
   {
   DSL_CPE_SIC_SET(DSL_TC_ATM, DSL_EMF_TC_NORMAL, DSL_EMF_TC_NORMAL, DSL_SYSTEMIF_MII)
   }
#endif
};
#endif /* _lint*/

#ifdef INCLUDE_DSL_CPE_API_VINAX
static DSL_Error_t DSL_VNX_ProcessDevConfigLine(
      DSL_char_t *pLine,
     DSL_uint32_t cfgLine,
      DSL_DeviceLowLevelConfig_t *pRetLLCfg )
{
   DSL_Error_t nRet = DSL_SUCCESS;
   DSL_int_t nGainSel = -1, RxGain = -1, TxGain = -1, nClkOut = -1,
             nFilter = -1, nHdlcIfMode = -1, nMiiClockSource = -1,
             nMiiClockChain = -1, nTcMode = -1, nMiiMode = -1,
             nLineMode = -1, nHybrid = -1, nBaseAddr = 0, nIrqNum = 0,
             nUtopiaParity = -1, nUtopiaPhyAdr = 0, nUtopiaBusWidth = 0,
             nPosPhyParity = -1, nPosPhyAdr = 0, nPosphyChunkSize = 0,
             nEapsTimeout = -1, bVirtualNoiseSupportUs = -1,
             bVirtualNoiseSupportDs = -1, bNtrEnable = -1;

   switch(cfgLine)
   {
   case 0:
      DSL_CPE_sscanf(pLine,
         "%d %d %d",
         &nGainSel,
         &RxGain,
         &TxGain);

      pRetLLCfg->nGainSelection            = (DSL_DEV_RxTxGainSelection_t)nGainSel;
      pRetLLCfg->nUserGainSettings.nRxGain = (DSL_int16_t)RxGain;
      pRetLLCfg->nUserGainSettings.nTxGain = (DSL_int16_t)TxGain;
      break;

   case 1:
      DSL_CPE_sscanf(pLine,
         "%d %d %d",
         &nClkOut,
         &nFilter,
         &nHdlcIfMode);

      pRetLLCfg->nClockoutEnable           = (DSL_DEV_Clockout_t)nClkOut;
      pRetLLCfg->nFilter                   = (DSL_DEV_Filter_t)nFilter;
      pRetLLCfg->nHdlcIfMode               = (DSL_DEV_HdlcMode_t)nHdlcIfMode;
      break;

   case 2:
      DSL_CPE_sscanf(pLine,
         "%d %d %d",
         &nMiiClockSource,
         &nMiiClockChain,
        &nTcMode);

      pRetLLCfg->nMiiClockSource           = (DSL_DEV_MiiClockSource_t)nMiiClockSource;
      pRetLLCfg->nMiiClockChain            = (DSL_DEV_MiiClockChain_t)nMiiClockChain;
      pRetLLCfg->nTcMode                   = (DSL_DEV_TcMode_t)nTcMode;
      break;

   case 3:
      DSL_CPE_sscanf(pLine,
         "%d %x %d",
         &nMiiMode,
         &nLineMode,
         &nHybrid);

      pRetLLCfg->nMiiMode                  = (DSL_DEV_MiiMode_t)nMiiMode;
      pRetLLCfg->nLineMode                 = (DSL_DEV_LineMode_t)nLineMode;
      pRetLLCfg->nHybrid                   = (DSL_DEV_Hybrid_t)nHybrid;
      break;

   case 4:
      DSL_CPE_sscanf(pLine,
         "%x %d %d",
        &nBaseAddr,
        &nIrqNum,
        &nUtopiaParity);

      pRetLLCfg->nBaseAddr                 = (DSL_uint32_t)nBaseAddr;
      pRetLLCfg->nIrqNum                   = (DSL_int8_t)nIrqNum;
      pRetLLCfg->nUtopiaParity             = (DSL_Parity_t)nUtopiaParity;
      break;

   case 5:
      DSL_CPE_sscanf(pLine,
         "%x %d %d",
        &nUtopiaPhyAdr,
         &nUtopiaBusWidth,
         &nPosPhyParity);

      pRetLLCfg->nUtopiaPhyAdr             = (DSL_uint8_t)nUtopiaPhyAdr;
      pRetLLCfg->nUtopiaBusWidth           = (DSL_UtopiaBusWidth_t)nUtopiaBusWidth;
      pRetLLCfg->nPosPhyParity             = (DSL_Parity_t)nPosPhyParity;

      break;
   case 6:
      DSL_CPE_sscanf(pLine,
         "%x %d %d",
         &nPosPhyAdr,
         &nPosphyChunkSize,
         &nEapsTimeout);

      pRetLLCfg->nPosPhyAdr                = (DSL_uint8_t)nPosPhyAdr;
      pRetLLCfg->nPosphyChunkSize          = (DSL_int32_t)nPosphyChunkSize;
      pRetLLCfg->nEapsTimeout              = (DSL_int32_t)nEapsTimeout;

      break;
   case 7:
      DSL_CPE_sscanf(pLine,
         "%d %d %d",
         &bVirtualNoiseSupportUs,
         &bVirtualNoiseSupportDs,
         &bNtrEnable);

      pRetLLCfg->bVirtualNoiseSupportUs    = (DSL_boolean_t)bVirtualNoiseSupportUs;
      pRetLLCfg->bVirtualNoiseSupportDs    = (DSL_boolean_t)bVirtualNoiseSupportDs;
      pRetLLCfg->bNtrEnable                = (DSL_boolean_t)bNtrEnable;

      break;
   default:
      break;
   }

   return(nRet);
}

/**
   This function returns the Initial Low Level Configuration from the give file
   or the fixed default settings.

   \param pName      file name of the given LowLevelConfig
   \param ppRetLLCfg Pointer to return the settings.

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS in case of success
   - DSL_ERROR if operation failed
   - The return pointer is set to the given configuration or ULL

*/
DSL_Error_t DSL_CPE_GetInitialLowLevelConfig( DSL_char_t const *pName,
                                          DSL_DeviceLowLevelConfig_t *pRetLLCfg )
{
   DSL_CPE_File_t *fdCfg = DSL_NULL;
   DSL_Error_t nRet = DSL_SUCCESS;
   DSL_uint32_t nCfgFileLine;
   DSL_char_t  line[256];

/*
# VINAX Low Level Configuration File
#
# Parameters must be separated by tabs or spaces.
# Empty lines and comments will be ignored.
#

# nGainSel      RxGain                 TxGain
#
# NA      = -1
# DEFAULT = 0
# USER    = 1
#
#  (dec)         (dec)                 (dec)
    -1            -1                    -1

# nClkOut       nFilter                nHdlcIfMode
#
# NA   = -1     NA     = -1            NA = -1
# ON   = 0      OFF    = 0             OD = 0
# OFF  = 1      ISDN   = 1             PP = 1
#               POTS   = 2
#               POTS_2 = 3
#               POTS_3 = 4
#
#  (dec)         (dec)                  (dec)
    -1            -1                     -1

# nMiiClockource nMiiClockChain        nTcMode
#
# NA = -1        NA  = -1              NA   = -1
# EXT = 0        OFF = 0               MAC  = 0
# INT = 1        ON  = 1               VPHY = 1
# CRY = 2
#
#  (dec)           (dec)                (dec)
    -1               1                    1

# nMiiMode       nLineMode             nHybrid
#
# NA     = -1    NA         = -1       NA                 = -1
# MII    = 0     VDSL2_B43  = 0x0001   AD1_138_17         = 0
# RMII   = 1     VDSL2_A43  = 0x0002   AD1_25_17          = 1
# SMII   = 2     VDSL2_V43  = 0x0004   AD1_138_30         = 2
# SSSMII = 3     VDSL1_V43P = 0x0008   AD1_138_17_CPE     = 3
# OFF    = 4     VDSL1_V43I = 0x0010   F1_998             = 4
#                ADSL1_C43  = 0x0020   F1_998_U0          = 5
#                ADSL2_J43  = 0x0040   F1_998_U017_o_ISDN = 6
#                ADSL2_B43C = 0x0080   F1_JAP30           = 7
#                ADSL2_A43C = 0x0100   F1_998E17_U0       = 8
#
#  (dec)           (hex)                 (dec)
     0             0x1                    0

#   nBaseAddr     nIrqNum              nUtopiaParity
#                                      NA   = -1
#                                      ODD  = 0
#                                      EVEN = 1
#
#     (hex)        (dec)                 (dec)
     0xC0400080       14                     0

# nUtopiaPhyAdr   nUtopiaBusWidth      nPosPhyParity
#                 default(16b) = 0     NA   = -1
#                 8-bit        = 1     ODD  = 0
#                 16-bit       = 2
#
#
#    (hex)            (dec)                (dec)
      0xFF              0                    0

# nPosPhyAdr      nPosphyChunkSize
#
#
#   (hex)             (dec)
    0xFF               64
*/

   if (pName != DSL_NULL)
   {
      fdCfg = DSL_CPE_FOpen(pName, "r");
      if (fdCfg == DSL_NULL)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "DSL: Error opening low level configuration file %s " DSL_CPE_CRLF , pName));
         return DSL_ERROR;
      }

      nCfgFileLine = 0;
      while( (DSL_CPE_FGets(line, sizeof(line), fdCfg)) != DSL_NULL )
      {
         if( (line[0] == '\n') || (line[0] == '#') )
         {
            continue;
         }

         nRet = DSL_VNX_ProcessDevConfigLine(line, nCfgFileLine, pRetLLCfg);

         nCfgFileLine++;

         if( nRet != DSL_SUCCESS )
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "DSL: Error processing configuration file %s, line %d "DSL_CPE_CRLF, pName, nCfgFileLine));
            break;
         }
      }
      DSL_CPE_FClose (fdCfg);
   }
   else
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_WRN, (DSL_CPE_PREFIX
         "DSL: Low Level Configuration file not specified, using default configuration"DSL_CPE_CRLF));
   }

   return nRet;
}
#endif /* #ifdef INCLUDE_DSL_CPE_API_VINAX*/
