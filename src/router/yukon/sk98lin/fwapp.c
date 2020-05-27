/******************************************************************************
 *
 * Name:    fwapp.c
 * Project: Gigabit Ethernet Adapters, Common Modules
 * Version: $Revision: #6 $
 * Date:    $Date: 2010/11/04 $
 * Purpose: Embedded SDK firmware application code.
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	LICENSE:
 *	(C)Copyright Marvell.
 *	
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *	
 *	The information in this file is provided "AS IS" without warranty.
 *	/LICENSE
 *
 ******************************************************************************/

#if (defined(DEBUG) || ((!defined(LINT)) && (!defined(SK_SLIM))))
static const char SysKonnectFileId[] =
"$Header: //Release/Yukon_1G/Shared/fw_interface/V1/embedded_sdk/fwapp.c#6 $" ;
#endif

#include "h/skdrv1st.h"
#include "h/skdrv2nd.h"

/******************************************************************************
		Local Variables
******************************************************************************/

/******************************************************************************
		Global Variables
******************************************************************************/

/******************************************************************************
		Local Functions
******************************************************************************/
static void   FwRestartCpu(SK_AC *, SK_IOC);
static void   FwInitRmt(SK_AC *);
static int    FwFillRmt(SK_AC *, SK_U32, char *, SK_U32);
#ifdef FW_RMT_VERBOSE_DEBUG
static void   FwPrintRmt(SK_AC *);
#endif

/******************************************************************************
		Global Functions
******************************************************************************/

extern SK_U32 SendFwCommand(SK_AC *, SK_U32, char *, SK_U32);

/*****************************************************************************
 *
 *  @brief This function initializes the FW application
 *         code at level 0.
 *
 *  @param pAC      Pointer to adapter context
 *  @param IoC      IO context handle 
 *  @return         SK_TRUE or SK_FALSE
 */
int FwAppInit0(SK_AC *pAC, SK_IOC IoC)
{
	SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT,
		("%s: FwAppInit0 ==>\n", SK_DRV_NAME));

	/* Set structure to zero */
	SK_MEMSET((char *)&(pAC->FwApp), 0, sizeof(SK_FWAPP));

	pAC->FwApp.ActivePort   = 0;
	pAC->FwApp.DualMode     = SK_GEASF_Y2_SINGLEPORT;
	pAC->FwApp.OpMode       = SK_GEASF_MODE_UNKNOWN;
	pAC->FwApp.ChipMode     = SK_GEASF_CHIP_UNKNOWN;
	pAC->FwApp.InitState    = ASF_INIT_UNDEFINED;
	pAC->RamAddr = 0;
	pAC->RamSelect = 0;

	/* Allocate and initialize RMT */
#ifdef FW_RMT_DEBUG
	SK_DBG_PRINTF("%s: FwAppInit0: Allocate pAC->FwApp.pRMT\n", SK_DRV_NAME);
#endif
	pAC->FwApp.pRMT = (FW_RMT *)FwOsAllocMemory(pAC, sizeof(FW_RMT));
	if (pAC->FwApp.pRMT == NULL) {
		SK_DBG_PRINTF("%s: FwAppInit0: Memory allocation error!\n", SK_DRV_NAME);

		SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT,
			("%s: FwAppInit0 <==\n", SK_DRV_NAME));

		return SK_FALSE;
	}
	FwInitRmt(pAC);

	SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT,
		("%s: FwAppInit0 <==\n", SK_DRV_NAME));

	return SK_TRUE;

} /* FwAppInit0 */

/*****************************************************************************
 *
 *  @brief This function initializes the FW application
 *         code at level 1.
 *
 *  @param pAC      Pointer to adapter context
 *  @param IoC      IO context handle 
 *  @return         SK_TRUE or SK_FALSE
 */
int FwAppInit1(SK_AC *pAC, SK_IOC IoC)
{
	int RetCode = SK_TRUE;

	SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT,
		("%s: FwAppInit1 ==>\n", SK_DRV_NAME));

	/* Check chip ID and set appropriate flash offsets, */
	/* operational modes and chip modes depending on Chip ID. */
	switch (pAC->FwCommon.ChipID) {
	case CHIP_ID_YUKON_SUPR:
		pAC->FwApp.ChipMode        = SK_GEASF_CHIP_SU;
		pAC->FwApp.FlashSize       = ASF_FLASH_SU_SIZE;

		SK_STRNCPY(pAC->FwApp.DriverVersion, FW_API_VERSION, FW_API_VERSION_SIZE);

		SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT,
			("%s: FwAppInit1: pAC->FwApp.DriverVersion: %s!\n",
			SK_DRV_NAME, pAC->FwApp.DriverVersion));

		break;
	default:
		/* Chip ID does not match! */
		pAC->FwApp.InitState = ASF_INIT_ERROR_CHIP_ID;
		printk("%s: FwAppInit1: Chip ID does not match!\n", SK_DRV_NAME);

		RetCode = SK_FALSE;
		break;
	}

	if (RetCode != SK_TRUE) {
		pAC->FwApp.InitState = ASF_INIT_ERROR_CHIP_ID;

		SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT,
			("%s: FwAppInit1 <==\n", SK_DRV_NAME));

		return RetCode;
	}

	SetRamAddr(pAC, SK_ST_FIFOTYPE, HOST_READ_QWORD, SK_ST_BUFADDR_HIGH);
	pAC->FwApp.OpMode = SK_GEASF_MODE_EMBEDDED_SDK;

	SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT,
		("%s: FwAppInit1 <==\n", SK_DRV_NAME));

	return RetCode;

} /* FwAppInit1 */

/*****************************************************************************
 *
 *  @brief This function initializes the FW application
 *         code at level 2.
 *
 *  @param pAC      Pointer to adapter context
 *  @param IoC      IO context handle 
 *  @return         SK_TRUE or SK_FALSE
 */
int FwAppInit2(SK_AC *pAC, SK_IOC IoC)
{
	int RetCode = SK_TRUE;

	SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT,
		("%s: FwAppInit2 ==>\n", SK_DRV_NAME));

	/* FW initialization ok */
	pAC->FwApp.InitState = ASF_INIT_OK;

	/* Check if CPU is running */
	if (FwCpuState(pAC, IoC) != ASF_CPU_STATE_RUNNING) {
		SK_DBG_PRINTF("%s: FwAppInit2: CPU is NOT running! Start CPU!\n", SK_DRV_NAME);

		/* If not, start CPU */
		FwStartCpu(pAC, IoC);
	}
	pAC->FwApp.CpuAlive = 1;

	SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT,
		("%s: FwAppInit2: INIT OK pAC->FwApp.InitState: %d pAC->FwApp.OpMode: %d\n",
		SK_DRV_NAME, pAC->FwApp.InitState, pAC->FwApp.OpMode));

	/* Otherwise the transmitter and the receiver will be */
	/* deactivated if we shut down the network interface. */
	FwAsfEnable(pAC, IoC);

	SK_DBG_PRINTF("%s: Checking Driver <-> FW communication ...", DRV_NAME);
	if (FwHciSendCommand(pAC, IoC, YASF_HOSTCMD_CHECK_ALIVE, 0, 0, 0, ASF_HCI_WAIT, 0) != HCI_EN_CMD_READY) {
		SK_DBG_PRINTF(" FAILED!\n");
		pAC->FwState = SK_FALSE;
	} else {
		SK_DBG_PRINTF(" working!\n");
		pAC->FwState = SK_TRUE;
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT,("%s: FwAppInit2 <==\n", SK_DRV_NAME));

	return RetCode;

} /* FwAppInit2 */

/*****************************************************************************
 *
 *  @brief This function prepares the patches for the FW image.
 *
 *  @param pAC      Pointer to adapter context
 *  @param IoC      IO context handle 
 *  @return         SK_TRUE or SK_FALSE
 */
int FwAppPatchImage(SK_AC *pAC, SK_IOC IoC)
{
	int RetCode;

	/* Nothing to patch... */
	pAC->FwCommon.PatchNumber = 0;

	SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT,
		("%s: FwAppPatchImage: pAC->FwCommon.PatchNumber: %d\n",
		SK_DRV_NAME, pAC->FwCommon.PatchNumber));

	RetCode = SK_TRUE;
	return RetCode;

} /* FwAppPatchImage */

/*****************************************************************************
 *
 *  @brief This function returns the full pathname
 *         of the FW image file.
 *
 *  @param pAC      Pointer to adapter context
 *  @return         Pointer to full FW file pathname
 */
char *FwAppGetImageName(SK_AC *pAC)
{
	SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT,
		("%s: FwAppGetImageName: pAC->FwOs.pFilePathName: %s\n",
		SK_DRV_NAME, pAC->FwOs.pFilePathName));

	return pAC->FwOs.pFilePathName;

} /* FwAppGetImageName */

/*****************************************************************************
 *
 *  @brief This function compares the required FW version
 *         with the version found in the FW image file.
 *
 *  @param pAC      Pointer to adapter context
 *  @return         SK_TRUE or SK_FALSE
 */
SK_BOOL FwAppIsVersionOk(SK_AC *pAC)
{
	SK_BOOL RetCode = SK_TRUE;
	int     i;

	/*
	 * Compare file firmware version with
	 * required firmware version.
	 */
	for (i = 0; i < FW_API_VERSION_SIZE; i++) {
		if (pAC->FwApp.DriverVersion[i] != pAC->FwCommon.FileVersion[i]) {
			RetCode = SK_FALSE;
		}
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT,
		("%s: FwAppIsVersionOk: RetCode: %d\n", SK_DRV_NAME, RetCode));

	return RetCode;

} /* FwAppIsVersionOk */

/*****************************************************************************
*
* FwMainStateMachine - general ASF  timer
*
* Description:
*
* Returns:
*   Always 0
*/
void FwMainStateMachine(
SK_AC  *pAC,	/* Pointer to adapter context */
SK_IOC IoC)		/* IO context handle */
{
	SK_EVPARA EventParam;

	SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT,
		("%s: FwMainStateMachine() called!\n", SK_DRV_NAME));

	if (pAC->FwApp.GlHciState == 0) { /* idle */

		SK_MEMSET((char *)&EventParam, 0, sizeof(EventParam));
		SkTimerStart(pAC, IoC, &pAC->FwApp.AsfTimer,
					1000000, SKGE_ASF, SK_ASF_EVT_TIMER_EXPIRED,
					EventParam);
	}
	else {
		SK_MEMSET((char *)&EventParam, 0, sizeof(EventParam));
		SkTimerStart(pAC, IoC, &pAC->FwApp.AsfTimer,
					30000, SKGE_ASF, SK_ASF_EVT_TIMER_EXPIRED,
					EventParam);
	}

	return;

} /* FwMainStateMachine */

/*****************************************************************************
*
* FwDriverHello - Hand over control from firmware to driver
*
* Description:
*
* Returns:
*   Nothing
*/
void FwDriverHello(
SK_AC  *pAC,	/* Pointer to adapter context */
SK_IOC IoC)		/* IO context handle */
{
	SK_U8   Result;
	SK_U8   TmpBuffer[4];

	/* Set OS Present Flag in ASF Status and Command Register */
	FwSetOsPresentBit(pAC, pAC->IoBase);

	TmpBuffer[0] = YASF_HOSTCMD_DRV_HELLO;
	TmpBuffer[1] = 3; /* Length */
	TmpBuffer[2] = 0; /* Original handling */
	TmpBuffer[3] = 0;

	/* Set the FW queue handling values */
	if (pAC->SdkQH == SK_TRUE) {
		if (pAC->Suspended)
			TmpBuffer[2] = 3; /* Suspend/new handling */
		else
			TmpBuffer[2] = 1; /* New handling */
	} else {
		if (pAC->Suspended)
			TmpBuffer[2] = 2; /* Suspend */
	}

	Result = FwHciSendData(pAC, pAC->IoBase, TmpBuffer, 0, ASF_HCI_WAIT, 0);
	if (Result == HCI_EN_CMD_ERROR) {
		printk("%s: FwDriverHello: YASF_HOSTCMD_DRV_HELLO NOT in FW!\n",
			SK_DRV_NAME);
		pAC->FwState = SK_FALSE;
	} else {
		printk("%s: FwDriverHello: YASF_HOSTCMD_DRV_HELLO in FW!\n",
			SK_DRV_NAME);
	}

	return;

} /* FwDriverHello */

/*****************************************************************************
*
* FwDriverGoodbye - Hand over control from driver to firmware
*
* Description:
*
* Returns:
*   Nothing
*/
void FwDriverGoodbye(
SK_AC  *pAC,	/* Pointer to adapter context */
SK_IOC IoC)		/* IO context handle */
{
	SK_U8   Result;
	SK_U8   TmpBuffer[4];

	/* Reset OS Present Flag in ASF Status and Command Register */
	FwResetOsPresentBit(pAC, pAC->IoBase);

	/* Inform the FW that the driver will be unloaded */
	TmpBuffer[0] = YASF_HOSTCMD_DRV_GOODBYE;
	TmpBuffer[1] = 3; /* Length */
	TmpBuffer[2] = 0; /* Original handling */
	TmpBuffer[3] = 0;

	/* Set the FW queue handling values */
	if (pAC->SdkQH == SK_TRUE) {
		if (pAC->Suspended)
			TmpBuffer[2] = 3; /* Suspend/new handling */
		else
			TmpBuffer[2] = 1; /* New handling */
	} else {
		if (pAC->Suspended)
			TmpBuffer[2] = 2; /* Suspend */
	}

	Result = FwHciSendData(pAC, pAC->IoBase, TmpBuffer, 0, ASF_HCI_WAIT, 0);
	if (Result == HCI_EN_CMD_ERROR) {
		printk("%s: FwDriverGoodbye: YASF_HOSTCMD_DRV_GOODBYE NOT in FW!\n",
			SK_DRV_NAME);
		pAC->FwState = SK_FALSE;
	} else {
		printk("%s: FwDriverGoodbye: YASF_HOSTCMD_DRV_GOODBYE in FW!\n",
			SK_DRV_NAME);
	}

	return;

} /* FwDriverGoodbye */

#ifdef MV_INCLUDE_FW_SDK_LINK_MAINTENANCE
/*****************************************************************************
*
* FwCheckLinkState - Check the link state
*
* Description:
*
* Returns:
*   Nothing
*/
void FwCheckLinkState(
SK_AC  *pAC,	/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
int    Port,
SK_U16 *pPhyStat)
{
	*pPhyStat = 0x0;

	/* Check link state */
	GM_IN16(IoC, Port, GM_GP_CTRL, pPhyStat);
	*pPhyStat &= ~(GM_GPCR_AU_SPD_DIS | GM_GPCR_AU_FCT_DIS | GM_GPCR_AU_DUP_DIS | 
		GM_GPCR_FC_RX_DIS | GM_GPCR_FL_PASS | GM_GPCR_PART_ENA | 
		GM_GPCR_LOOP_ENA | BIT_10S | GM_GPCR_FC_TX_DIS | GM_GPCR_RMII_LB_ENA |
		GM_GPCR_RMII_PH_ENA);
}

/*****************************************************************************
*
* FwCheckLinkMode - Check the firmware link state and link mode
*
* Description:
*
* Returns:
*   Nothing
*/
void FwCheckLinkMode(
SK_AC  *pAC,	/* Pointer to adapter context */
SK_IOC IoC,		/* IO context handle */
int    Port)
{
	SK_U16 PhyStat;
	SK_U32 PhyCtrl;

	PhyStat = 0x0;
	PhyCtrl = 0x0;

	pAC->GIni.GIDontInitPhy = SK_FALSE;

	if (pAC->LinkInfo[Port].LinkMaintenance == SK_TRUE) {

		/* Check if link is up */
		SK_IN32(pAC->IoBase, MR_ADDR(Port, GPHY_CTRL), &PhyCtrl);

		if ((PhyCtrl & GPC_PHY_LINK_UP) == 0) {
#ifdef MV_FW_SDK_LINK_MAINTENANCE_DEBUG
			printk("FwCheckLinkMode: No link on port %d\n", Port);
#endif
			return;
		}
		else {
#ifdef MV_FW_SDK_LINK_MAINTENANCE_DEBUG
			printk("FwCheckLinkMode: Link up on port %d\n", Port);
#endif
			/* Check link state */
			FwCheckLinkState(pAC, pAC->IoBase, Port, &PhyStat);
#ifdef MV_FW_SDK_LINK_MAINTENANCE_DEBUG
			printk("FwCheckLinkMode: GM_GP_CTRL:  0x%x\n",
				PhyStat);
#endif
			if (pAC->DriverLinkStat != PhyStat) {
#ifdef MV_FW_SDK_LINK_MAINTENANCE_DEBUG
				printk("FwCheckLinkMode: Link status changed\n");
				printk("FwCheckLinkMode: pAC->DriverLinkStat:     0x%x\n",
					pAC->DriverLinkStat);
#endif
				return;
			}

			/* Store link state */
			pAC->DriverLinkStat = PhyStat;

			/* Enable link maintenance */
			pAC->GIni.GIDontInitPhy = SK_TRUE;
#ifdef MV_FW_SDK_LINK_MAINTENANCE_DEBUG
			printk("FwCheckLinkMode: Link match, GIDontInitPhy: %d\n",
				pAC->GIni.GIDontInitPhy);
#endif
		}
	}
	else {
#ifdef MV_FW_SDK_LINK_MAINTENANCE_DEBUG
		printk("FwCheckLinkMode: UseLinkMaintenance not set!\n");
#endif
	}

	return;

} /* FwCheckLinkMode */
#endif

/*****************************************************************************
*
* FwRestartCpu - Reset and start firmware CPU
* Description:
*
* Returns:
*   Nothing
*/
static void FwRestartCpu(
SK_AC	*pAC,	/* Pointer to adapter context */
SK_IOC	IoC)	/* IO context handle */
{
#ifdef FW_RMT_DEBUG
	SK_BOOL RetCode;

	/* Check CPU state. */
	RetCode = FwCpuState(pAC, IoC);

	SK_DBG_PRINTF("%s: FwRestartCpu: CPU State: 0x%x\n", DRV_NAME, RetCode);
#endif

	/* Reset CPU. */
	SK_DBG_PRINTF("%s: FwRestartCpu: Reset CPU\n", DRV_NAME);
	FwResetCpu(pAC, IoC);

	/* Start CPU. */
	SK_DBG_PRINTF("%s: FwRestartCpu: Start CPU\n", DRV_NAME);
	FwStartCpu(pAC, IoC);

	FwAsfEnable(pAC, IoC);
	pAC->FwState = SK_TRUE;

	return;

} /* FwRestartCpu */

/*****************************************************************************
*
* FwRecoverFirmware - Restart firmware and trigger RMT update
* Description:
*
* Returns:
*   Nothing
*/
void FwRecoverFirmware(
SK_AC	*pAC,	/* Pointer to adapter context */
SK_IOC	IoC)	/* IO context handle */
{
	if (pAC->FwState == SK_TRUE) {
		return;
	}

	FwRestartCpu(pAC, IoC);
	FwOsSleep(FW_RECOVER_DELAY_VALUE);
	FwDriverHello(pAC, IoC);

	if (pAC->FwState == SK_TRUE) {

		/* Trigger FW message recovery (RMT). */
		if (pAC->FwApp.pRMT->RmtFilled == SK_TRUE) {
			pAC->FwApp.pRMT->RecoverMessageTable = SK_TRUE;
		}
	}
	else {
		SK_DBG_PRINTF("%s: FwRecoverFirmware: Unable to restart FW\n",
			DRV_NAME);
	}

} /* FwRecoverFirmware */

/*****************************************************************************
*
* FwHandleRmt - Handle RMT (Recovery Message Table)
*
* Description:
*
* Returns:
*   Nothing
*/
int FwHandleRmt(
SK_AC	*pAC,			/* Pointer to adapter context */
SK_U32	ID,
char	*pMemBuf,
SK_U32	Length)
{
	if (ID != RMT_MSG_ID_DEFAULT) {
		if (ID == RMT_MSG_ID_RESET) {
#ifdef FW_RMT_DEBUG
			SK_DBG_PRINTF("%s: FwHandleRmt: Receive RMT_MSG_ID_RESET: 0x%x\n",
				DRV_NAME, ID);
#endif
			FwInitRmt(pAC);
		}
		else {
#ifdef FW_RMT_DEBUG
			SK_DBG_PRINTF("%s: FwHandleRmt: Receive RMT_MSG_ID: 0x%x\n",
				DRV_NAME, ID);
#ifdef FW_RMT_VERBOSE_DEBUG
			PrintDataBuffer("sk98lin: FwHandleRmt",
				(char *)pMemBuf, Length);
#endif
#endif
			if (FwFillRmt(pAC, ID, (char *)pMemBuf, Length)) {
#ifdef FW_RMT_DEBUG
				SK_DBG_PRINTF("%s: FwHandleRmt: ERROR: RMT full.\n",
					DRV_NAME);
#endif
				return 1;
			}
		}
	}

	return 0;

} /* FwHandleRmt */

/*****************************************************************************
*
* FwInitRmt - Initialize RMT (Recovery Message Table)
*
* Description:
*
* Returns:
*   Nothing
*/
static void FwInitRmt(
SK_AC	*pAC)	/* Pointer to adapter context */
{
	int	i;

#ifdef FW_RMT_DEBUG
	SK_DBG_PRINTF("%s: FwInitRmt: Clear and initialize RMT\n", DRV_NAME);
#endif

	/* Clear and initialize RMT. */
	SK_MEMSET((char *)pAC->FwApp.pRMT, 0, sizeof(FW_RMT));
	pAC->FwApp.pRMT->RmtFilled = SK_FALSE;
	pAC->FwApp.pRMT->RecoverMessageTable = SK_FALSE;
	for (i = 0; i < FW_RMT_ARRAY_SIZE; i++) {
		pAC->FwApp.pRMT->RmtMessage[i].MessageIndex = i;
	}

} /* FwInitRmt */

/*****************************************************************************
*
* FwFillRmt - Fill RMT (Recovery Message Table)
*
* Description:
*
* Returns:
*   Nothing
*/
int FwFillRmt(
SK_AC	*pAC,			/* Pointer to adapter context */
SK_U32	ID,
char	*pMemBuf,
SK_U32	Length)
{
	SK_BOOL	MessageHandled;
	int		i;

	/* Check the message length. */
	if (Length > FW_RMT_MESSAGE_SIZE) {

		SK_DBG_PRINTF("%s: FwFillRmt: WARNING: Message too long. ID: 0x%x not stored\n",
			DRV_NAME, ID);

		return 1;
	}

	MessageHandled = SK_FALSE;
	for (i = 0; i < FW_RMT_ARRAY_SIZE; i++) {
		if (pAC->FwApp.pRMT->RmtMessage[i].MessageID == ID) {
#ifdef FW_RMT_DEBUG
			SK_DBG_PRINTF("%s: FwFillRmt: Index: %d REPLACE ID 0x%x with length 0x%x\n",
				DRV_NAME, pAC->FwApp.pRMT->RmtMessage[i].MessageIndex,
				ID, Length);
#endif
			/* Replace message. */
			SK_MEMSET((char *)pAC->FwApp.pRMT->RmtMessage[i].Message,
				0, FW_RMT_MESSAGE_SIZE);
			SK_MEMCPY((char *)pAC->FwApp.pRMT->RmtMessage[i].Message,
				(char *)pMemBuf, Length);
			pAC->FwApp.pRMT->RmtMessage[i].MessageLength = Length;
			pAC->FwApp.pRMT->RmtFilled = SK_TRUE;
			MessageHandled = SK_TRUE;
			break;
		}
	}

	/* If MessageHandled is FALSE, the message was not replaced. */
	if (MessageHandled == SK_FALSE) {
		for (i = 0; i < FW_RMT_ARRAY_SIZE; i++) {
			if (pAC->FwApp.pRMT->RmtMessage[i].MessageID == RMT_MSG_ID_DEFAULT) {
#ifdef FW_RMT_DEBUG
				SK_DBG_PRINTF("%s: FwFillRmt: Index: %d STORE ID 0x%x with length 0x%x\n",
					DRV_NAME, pAC->FwApp.pRMT->RmtMessage[i].MessageIndex,
					ID, Length);
#endif
				/* Store message. */
				pAC->FwApp.pRMT->RmtMessage[i].MessageID = ID;
				SK_MEMSET((char *)pAC->FwApp.pRMT->RmtMessage[i].Message,
					0, FW_RMT_MESSAGE_SIZE);
				SK_MEMCPY((char *)pAC->FwApp.pRMT->RmtMessage[i].Message,
					(char *)pMemBuf, Length);
				pAC->FwApp.pRMT->RmtMessage[i].MessageLength = Length;
				pAC->FwApp.pRMT->RmtFilled = SK_TRUE;
				MessageHandled = SK_TRUE;
				break;
			}
		}
	}

#ifdef FW_RMT_VERBOSE_DEBUG
	FwPrintRmt(pAC);
#endif

	/* If MessageHandled is FALSE, the RMT is full. */
	if (MessageHandled == SK_FALSE) {

		SK_DBG_PRINTF("%s: FwFillRmt: ERROR: RMT full. ID: 0x%x not stored\n",
			DRV_NAME, ID);

		return 1;
	}

	return 0;

} /* FwFillRmt */

/*****************************************************************************
*
* FwWriteRmt - Write RMT (Recovery Message Table) to firmware
*
* Description:
*
* Returns:
*   Nothing
*/
int FwWriteRmt(
SK_AC	*pAC)	/* Pointer to adapter context */
{
	int			i;
	int			RetCode = 1;
	SK_U8		RdTry = FW_MAX_READTRY;
	FwCmdResult	*FwCommandResult;

	if (pAC->FwApp.pRMT->RecoverMessageTable == SK_FALSE) {
#ifdef FW_RMT_DEBUG
		SK_DBG_PRINTF("%s: FwWriteRmt: No need to write RMT\n", DRV_NAME);
#endif
		return RetCode;
	}

	if (pAC->FwApp.pRMT->RmtFilled == SK_FALSE) {
		SK_DBG_PRINTF("%s: FwWriteRmt: RMT empty\n", DRV_NAME);
		pAC->FwApp.pRMT->RecoverMessageTable = SK_FALSE;
		return RetCode;
	}

	SK_DBG_PRINTF("%s: FwWriteRmt: Write RMT to FW START\n", DRV_NAME);

	for (i = 0; i < FW_RMT_ARRAY_SIZE; i++) {
		if (pAC->FwApp.pRMT->RmtMessage[i].MessageID != RMT_MSG_ID_DEFAULT) {
#ifdef FW_RMT_DEBUG
			SK_DBG_PRINTF("%s: FwWriteRmt: Write MessageIndex %d to FW\n",
				DRV_NAME,
				pAC->FwApp.pRMT->RmtMessage[i].MessageIndex);
#endif
			SendFwCommand(pAC, SKGE_DGRAM_MSG2FW,
				(char *)pAC->FwApp.pRMT->RmtMessage[i].Message,
				pAC->FwApp.pRMT->RmtMessage[i].MessageLength);

			/* Try to read the result. */
			RetCode = 2;
			for (RdTry = 0; RdTry < FW_MAX_READTRY; RdTry++) {

				/* Wait for result. */
				FwOsSleep(FW_READ_DELAY_VALUE);

				/* Check if we have a result. */
				if ((pAC->pFwBuffer) && (pAC->FwBufferLen)) {
					FwCommandResult = (FwCmdResult *)pAC->FwApp.pRMT->RmtMessage[i].Message;
#ifdef FW_RMT_DEBUG
					SK_DBG_PRINTF("%s: FwWriteRmt: Result in loop %d for MessageIndex %d: 0x%x\n",
						DRV_NAME, RdTry,
						pAC->FwApp.pRMT->RmtMessage[i].MessageIndex,
						FwCommandResult->Result);
#ifdef FW_RMT_VERBOSE_DEBUG
					PrintDataBuffer("sk98lin: FwWriteRmt",
						(char *)pAC->FwApp.pRMT->RmtMessage[i].Message,
						pAC->FwApp.pRMT->RmtMessage[i].MessageLength);
#endif
#endif
					if (FwCommandResult->Result != FW_RESULT_OK) {

						SK_DBG_PRINTF("%s: FwWriteRmt: ERROR: RMT ID 0x%x"
							" at INDEX %d with result 0x%x from FW\n",
							DRV_NAME,
							pAC->FwApp.pRMT->RmtMessage[i].MessageID,
							pAC->FwApp.pRMT->RmtMessage[i].MessageIndex,
							FwCommandResult->Result);

						RetCode = 0;
					}
					else {
						RetCode = 1;
					}
					pAC->FwBufferLen = 0;
					FwOsFreeMemory(pAC, (SK_U8 *)pAC->pFwBuffer);
					pAC->pFwBuffer = NULL;
					break;
				}
			}

			/* ERROR: No result after 200 milliseconds. */
			if (RetCode == 2) {
				SK_DBG_PRINTF("%s: FwWriteRmt: ERROR: RMT ID 0x%x"
					" at INDEX %d without result from FW\n",
					DRV_NAME,
					pAC->FwApp.pRMT->RmtMessage[i].MessageID,
					pAC->FwApp.pRMT->RmtMessage[i].MessageIndex);
				pAC->FwApp.pRMT->RecoverMessageTable = SK_TRUE;
				return RetCode;
			}
			else if (RetCode == 0) {
				pAC->FwApp.pRMT->RecoverMessageTable = SK_TRUE;
				return RetCode;
			}
		}
	}

	/* Recovery successful, set RecoverMessageTable to false. */
	pAC->FwApp.pRMT->RecoverMessageTable = SK_FALSE;

	SK_DBG_PRINTF("%s: FwWriteRmt: Write RMT to FW END\n", DRV_NAME);

	return RetCode;

} /* FwWriteRmt */

#ifdef FW_RMT_VERBOSE_DEBUG
/*****************************************************************************
*
* FwPrintRmt - Print RMT (Recovery Message Table) to console
*
* Description:
*
* Returns:
*   Nothing
*/
static void FwPrintRmt(
SK_AC	*pAC)	/* Pointer to adapter context */
{
	int	i;

	SK_DBG_PRINTF("\n%s: FwPrintRmt: Print RMT START\n\n", DRV_NAME);

	for (i = 0; i < FW_RMT_ARRAY_SIZE; i++) {
		if (pAC->FwApp.pRMT->RmtMessage[i].MessageID != RMT_MSG_ID_DEFAULT) {
			SK_DBG_PRINTF("%s: FwPrintRmt: MessageIndex:  %d\n",
				DRV_NAME, pAC->FwApp.pRMT->RmtMessage[i].MessageIndex);
			SK_DBG_PRINTF("%s: FwPrintRmt: MessageID:     0x%x\n",
				DRV_NAME, pAC->FwApp.pRMT->RmtMessage[i].MessageID);
			SK_DBG_PRINTF("%s: FwPrintRmt: MessageLength: 0x%x\n",
				DRV_NAME, pAC->FwApp.pRMT->RmtMessage[i].MessageLength);
			PrintDataBuffer("sk98lin: FwPrintRmt",
				(char *)pAC->FwApp.pRMT->RmtMessage[i].Message,
				pAC->FwApp.pRMT->RmtMessage[i].MessageLength);
		}
	}

	SK_DBG_PRINTF("%s: FwPrintRmt: Print RMT END\n\n", DRV_NAME);

} /* FwInitRmt */
#endif

