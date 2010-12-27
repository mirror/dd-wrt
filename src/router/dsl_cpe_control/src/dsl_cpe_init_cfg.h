/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DSL_CPE_INIT_CFG_H
#define _DSL_CPE_INIT_CFG_H

#ifdef __cplusplus
   extern "C" {
#endif

#define DSL_CPE_FW_SET(pFw1, szFw1, pFw2, szFw2) pFw1, szFw1, pFw2, szFw2

#define DSL_CPE_XTU_SET(oct0, oct1, oct2, oct3, oct4, oct5, oct6, oct7) \
   {{oct0, oct1, oct2, oct3, oct4, oct5, oct6, oct7}}

#define DSL_CPE_LINE_INV_NE_SET(pLi) pLi

#define DSL_CPE_AUTOBOOT_CTRL_SET(opt) opt
#define DSL_CPE_AUTOBOOT_CFG_SET(opt1, opt2, opt3) {{opt1, opt2, opt3}}

#define DSL_CPE_TEST_MODE_CTRL_SET(opt) opt

#define DSL_CPE_LINE_ACTIVATE_CTRL_SET(LDSF,ACSF,STARTUP_MODE) {LDSF,ACSF,STARTUP_MODE}

#define DSL_CPE_LL_CFG_SET(RXTXGAINSEL, RXGAIN, TXGAIN, CLOCKOUT, FILTER, \
   HDLCMODE, MIICLOCKSOURCE, MIICLOCKCHAIN, TCMODE, MIIMODE, LINEMODE, \
   HYBRID, BASEADDR, IRQNUM, UTOPIAPARITY, \
   UTOPIAADDR, UTOPIABUSWIDTH, POSPHYPARITY, POSPHYADDR, POSPHYCHUNKSIZE, EAPS_TO, \
   VIRTUAL_NOISE_ENABLE_US, VIRTUAL_NOISE_ENABLE_DS, NTR_ENABLE) \
   {RXTXGAINSEL, {RXGAIN, TXGAIN}, CLOCKOUT, FILTER, HDLCMODE,  \
   MIICLOCKSOURCE, MIICLOCKCHAIN, TCMODE, MIIMODE, LINEMODE, \
   HYBRID, BASEADDR, IRQNUM, UTOPIAPARITY, \
   UTOPIAADDR, UTOPIABUSWIDTH, POSPHYPARITY, POSPHYADDR, POSPHYCHUNKSIZE, EAPS_TO, \
   VIRTUAL_NOISE_ENABLE_US, VIRTUAL_NOISE_ENABLE_DS, NTR_ENABLE}

#define DSL_CPE_SIC_SET(TCLAYER, EFM_TC_CFG_US, EFM_TC_CFG_DS, SICS) \
   {TCLAYER, EFM_TC_CFG_US, EFM_TC_CFG_DS, SICS}

#define DSL_CPE_MAC_CFG_SET(SPEED, DUPLEX, FLOW, ANEGOT, MAC_OCT_0, MAC_OCT_1, MAC_OCT_2, \
   MAC_OCT_3, MAC_OCT_4, MAC_OCT_5, MAX_FRAME_SIZE, EXT_ETH_OAM) \
   {SPEED, DUPLEX, FLOW, ANEGOT, {MAC_OCT_0, MAC_OCT_1, MAC_OCT_2, MAC_OCT_3, MAC_OCT_4, MAC_OCT_5}, \
   MAX_FRAME_SIZE, EXT_ETH_OAM}

#define DSL_CPE_SAR_CFG_SET(CFILTER, DEFVPI, DEFVCI, MUXTYPE, FCSPRESERVE) \
   {CFILTER, DEFVPI, DEFVCI, MUXTYPE, FCSPRESERVE}

extern DSL_InitData_t gInitCfgData;

#ifdef INCLUDE_DSL_CPE_API_VINAX
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
                                          DSL_DeviceLowLevelConfig_t *pRetLLCfg );
#endif /* #ifdef INCLUDE_DSL_CPE_API_VINAX*/


#ifdef __cplusplus
}
#endif

#endif /* _DSL_CPE_INIT_CFG_H */
