/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/types.h>
#include <linux/kernel.h>

#include "ctrlEnv/mvCtrlEnvLib.h"
#include "ctrlEnv/sys/mvCpuIf.h"
#include "boardEnv/mvBoardEnvLib.h"
#include "mvDebug.h"
#include "mvSysHwConfig.h"
#include "pex/mvPexRegs.h"
#include "cntmr/mvCntmr.h"
#include "ctrlEnv/mvCtrlEnvLib.h"
#include "mvOs.h"
#include "asm/arch/xor.h"

/*************************************************************************************************************
 * Environment 
 *************************************************************************************************************/
extern u32 mvTclk;
extern u32 mvSysclk;
#if defined(MV_BRIDGE_SYNC_REORDER)
extern u32* mvUncachedParam;
#endif
EXPORT_SYMBOL(mv_early_printk);
EXPORT_SYMBOL(arm926_dma_inv_range);
EXPORT_SYMBOL(arm926_dma_flush_range);
EXPORT_SYMBOL(mvCtrlPwrClckGet);
EXPORT_SYMBOL(mvCtrlModelRevGet);
EXPORT_SYMBOL(mvTclk);
EXPORT_SYMBOL(mvSysclk);
EXPORT_SYMBOL(mvCtrlModelGet);
EXPORT_SYMBOL(mvOsIoUncachedMalloc);
EXPORT_SYMBOL(mvOsIoUncachedFree);
EXPORT_SYMBOL(mvOsIoCachedMalloc);
EXPORT_SYMBOL(mvOsIoCachedFree);
EXPORT_SYMBOL(mvDebugMemDump);
EXPORT_SYMBOL(mvHexToBin);
EXPORT_SYMBOL(mvBinToHex);
EXPORT_SYMBOL(mvSizePrint);
EXPORT_SYMBOL(mvDebugPrintMacAddr);
EXPORT_SYMBOL(mvCtrlEthMaxPortGet);
EXPORT_SYMBOL(mvCtrlTargetNameGet);
EXPORT_SYMBOL(mvBoardIdGet);
EXPORT_SYMBOL(mvBoardPhyAddrGet);
EXPORT_SYMBOL(mvCpuIfTargetWinGet);
EXPORT_SYMBOL(mvMacStrToHex);
EXPORT_SYMBOL(mvBoardTclkGet);
EXPORT_SYMBOL(mvBoardMacSpeedGet);


/*************************************************************************************************************
 * USB
 *************************************************************************************************************/
#ifdef CONFIG_MV_INCLUDE_USB
extern u32 mvIsUsbHost;

#include "usb/mvUsb.h"
EXPORT_SYMBOL(mvIsUsbHost);
EXPORT_SYMBOL(mvCtrlUsbMaxGet);
EXPORT_SYMBOL(mvUsbGetCapRegAddr);
EXPORT_SYMBOL(mvUsbGppInit);
EXPORT_SYMBOL(mvUsbBackVoltageUpdate);
#endif /* CONFIG_MV_INCLUDE_USB */

/*************************************************************************************************************
 * CESA
 *************************************************************************************************************/
#ifdef CONFIG_MV_INCLUDE_CESA
#include "ctrlEnv/sys/mvSysCesa.h"
#include "cesa/mvCesa.h"
#include "cesa/mvMD5.h"
#include "cesa/mvSHA1.h"
extern unsigned char*  mv_sram_usage_get(int* sram_size_ptr);
EXPORT_SYMBOL(mvCesaInit);
EXPORT_SYMBOL(mvCesaSessionOpen);
EXPORT_SYMBOL(mvCesaSessionClose);
EXPORT_SYMBOL(mvCesaAction);
EXPORT_SYMBOL(mvCesaReadyGet);
EXPORT_SYMBOL(mvCesaCopyFromMbuf);
EXPORT_SYMBOL(mvCesaCopyToMbuf);
EXPORT_SYMBOL(mvCesaMbufCopy);
EXPORT_SYMBOL(mvCesaCryptoIvSet);
EXPORT_SYMBOL(mvMD5);
EXPORT_SYMBOL(mvSHA1);

EXPORT_SYMBOL(mvCesaDebugQueue);
EXPORT_SYMBOL(mvCesaDebugSram);
EXPORT_SYMBOL(mvCesaDebugSAD);
EXPORT_SYMBOL(mvCesaDebugStatus);
EXPORT_SYMBOL(mvCesaDebugMbuf);
EXPORT_SYMBOL(mvCesaDebugSA);
EXPORT_SYMBOL(mv_sram_usage_get);
#endif

/*************************************************************************************************************
 * Flashes
 *************************************************************************************************************/
#if defined (CONFIG_MV_INCLUDE_SPI)
#include <sflash/mvSFlash.h>
#include <sflash/mvSFlashSpec.h>
EXPORT_SYMBOL(mvSFlashInit);
EXPORT_SYMBOL(mvSFlashSectorErase);
EXPORT_SYMBOL(mvSFlashChipErase);
EXPORT_SYMBOL(mvSFlashBlockRd);
EXPORT_SYMBOL(mvSFlashBlockWr);
EXPORT_SYMBOL(mvSFlashIdGet);
EXPORT_SYMBOL(mvSFlashWpRegionSet);
EXPORT_SYMBOL(mvSFlashWpRegionGet);
EXPORT_SYMBOL(mvSFlashStatRegLock);
EXPORT_SYMBOL(mvSFlashSizeGet);
EXPORT_SYMBOL(mvSFlashPowerSaveEnter);
EXPORT_SYMBOL(mvSFlashPowerSaveExit);
EXPORT_SYMBOL(mvSFlashModelGet);
#endif


/*************************************************************************************************************
 * SATA
 *************************************************************************************************************/
#ifdef CONFIG_MV_INCLUDE_INTEG_SATA
#include <ctrlEnv/sys/mvSysSata.h>
EXPORT_SYMBOL(mvSataWinInit);
#endif

/*************************************************************************************************************
 * DMA/XOR
 *************************************************************************************************************/

#if defined (CONFIG_MV_XORMEMCOPY) || defined (CONFIG_MV_IDMA_MEMCOPY)
EXPORT_SYMBOL(asm_memcpy);
EXPORT_SYMBOL(asm_memmove);
#endif
#if defined (CONFIG_MV_XORMEMZERO) || defined (CONFIG_MV_IDMA_MEMZERO)
EXPORT_SYMBOL(asm_memzero);
#endif

#if defined(CONFIG_MV_RAID5_XOR_OFFLOAD)
EXPORT_SYMBOL(xor_mv);
#endif

/*************************************************************************************************************
 * Networking
 *************************************************************************************************************/
#include "eth/mvEth.h"
#include "ctrlEnv/sys/mvSysGbe.h"
#include "eth-phy/mvEthPhy.h"
EXPORT_SYMBOL(mvEthInit);
EXPORT_SYMBOL(mvEthPhyRegRead);
EXPORT_SYMBOL(mvEthPhyRegWrite);

#if defined (CONFIG_MV_GATEWAY)

EXPORT_SYMBOL(mvEthPortTxDone);
EXPORT_SYMBOL(mvEthPortDisable); 
EXPORT_SYMBOL(ethResetTxDescRing); 
EXPORT_SYMBOL(mvEthPortInit); 
EXPORT_SYMBOL(mvEthPortEnable); 
EXPORT_SYMBOL(ethResetRxDescRing); 
EXPORT_SYMBOL(mvEthPortForceRx); 
EXPORT_SYMBOL(mvEthPortRxDone); 
EXPORT_SYMBOL(mvEthTxCoalSet); 
EXPORT_SYMBOL(mvEthPortForceTxDone); 
EXPORT_SYMBOL(mvEthTxResourceGet); 
EXPORT_SYMBOL(mvEthRxResourceGet); 
EXPORT_SYMBOL(mvEthPortFinish); 
EXPORT_SYMBOL(mvEthMacAddrGet); 
EXPORT_SYMBOL(mvEthPortTx); 
EXPORT_SYMBOL(mvEthMaxRxSizeSet); 
EXPORT_SYMBOL(mvEthPortRx); 
EXPORT_SYMBOL(mvEthRxCoalSet); 
EXPORT_SYMBOL(mvEthVlanPrioRxQueue); 
EXPORT_SYMBOL(mvEthRxFilterModeSet);
EXPORT_SYMBOL(mvEthMacAddrSet); 

#include "msApiDefs.h"
#include "msApiPrototype.h"
EXPORT_SYMBOL(gcosSetPortDefaultTc);
EXPORT_SYMBOL(gvlnSetPortVlanDBNum);
EXPORT_SYMBOL(eventSetActive);
EXPORT_SYMBOL(gprtSetPhyReg);
EXPORT_SYMBOL(gprtSetPagedPhyReg);
EXPORT_SYMBOL(gstpSetPortState);
EXPORT_SYMBOL(gqosUserPrioMapEn);
EXPORT_SYMBOL(gfdbFlushInDB);
EXPORT_SYMBOL(gsysSetUseDoubleTagData);
EXPORT_SYMBOL(gfdbAddMacEntry);
EXPORT_SYMBOL(gfdbFindAtuMacEntry);
EXPORT_SYMBOL(gqosSetPrioMapRule);
EXPORT_SYMBOL(gprtSetCtrMode);
EXPORT_SYMBOL(gsysSetPPUEn);
EXPORT_SYMBOL(gvlnSetPortVid);
EXPORT_SYMBOL(gprtClearAllCtr);
EXPORT_SYMBOL(gprtSetDoubleTag);
EXPORT_SYMBOL(gfdbDelAtuEntry);
EXPORT_SYMBOL(gprtSetVlanTunnel);
EXPORT_SYMBOL(gqosIpPrioMapEn);
EXPORT_SYMBOL(gprtSetProviderTag);
EXPORT_SYMBOL(gsysSetCPUPort);
EXPORT_SYMBOL(gstatsFlushAll);
EXPORT_SYMBOL(gprtSetForceFc);
EXPORT_SYMBOL(gfdbFlush);
EXPORT_SYMBOL(qdLoadDriver);
EXPORT_SYMBOL(gprtSetEgressMode);
EXPORT_SYMBOL(gprtPhyIntEnable);
EXPORT_SYMBOL(gvlnSetPortVlanDot1qMode);
EXPORT_SYMBOL(eventGetIntStatus);
EXPORT_SYMBOL(gstatsGetPortAllCounters3);
EXPORT_SYMBOL(gprtGetDuplex);
EXPORT_SYMBOL(gvlnSetPortVlanPorts);
EXPORT_SYMBOL(gprtGetLinkState);
EXPORT_SYMBOL(gprtGetPhyIntStatus);
EXPORT_SYMBOL(gfdbGetAtuEntryNext);
EXPORT_SYMBOL(gvtuAddEntry);
EXPORT_SYMBOL(gprtGetPhyIntPortSummary);
EXPORT_SYMBOL(gprtGetSpeedMode);
EXPORT_SYMBOL(gsysSetJumboMode);
EXPORT_SYMBOL(gprtSetHeaderMode);
#ifdef CONFIG_MV_GTW_IGMP
EXPORT_SYMBOL(gprtSetIGMPSnoop);
#endif 

#ifdef CONFIG_ETH_MULTI_Q
#include "eth/mvEthPolicy.h"
extern int MAX_SOFTIRQ_RESTART;
extern int netdev_max_backlog;
EXPORT_SYMBOL(MAX_SOFTIRQ_RESTART);
EXPORT_SYMBOL(netdev_max_backlog);
EXPORT_SYMBOL(mvMacHexToStr); 
EXPORT_SYMBOL(mvAsciiToHex);
EXPORT_SYMBOL(gcosSetDscp2Tc);
EXPORT_SYMBOL(mvEthRxPolicyInit); 
EXPORT_SYMBOL(mvEthRxPolicyGet); 
#endif 

#endif /* CONFIG_MV_GATEWAY */

#if defined(MV_BRIDGE_SYNC_REORDER)
EXPORT_SYMBOL(mvUncachedParam);
#endif
