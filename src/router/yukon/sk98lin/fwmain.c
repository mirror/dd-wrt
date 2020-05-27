/******************************************************************************
 *
 * Name:    fwmain.c
 * Project: fwcommon
 * Version: $Revision: #5 $
 * Date:    $Date: 2010/11/04 $
 * Purpose: common firmware interface for driver
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

#include "h/skdrv1st.h"
#include "h/skdrv2nd.h"

static void FwSetAsfCtrl( SK_AC *pAC, SK_IOC IoC);
static void FwEnableFlushFifo(  SK_AC *pAC, SK_IOC IoC);

#define NOT_EC_CHIP (pAC->FwCommon.ChipID == CHIP_ID_YUKON_EX) || \
		(pAC->FwCommon.ChipID == CHIP_ID_YUKON_SUPR)
	
#define FW_CPU_STATE_UNKNOWN       0
#define FW_CPU_STATE_RESET         1
#define FW_CPU_STATE_RUNNING       2


/*****************************************************************************
*
* FwSetOsPresentBit
*
* Description: Report to Firmware presence of OS
*
* Returns:
*
*/
void FwSetOsPresentBit(
	SK_AC  *pAC,  /* Pointer to adapter context */
	SK_IOC IoC)   /* IO context */
{
	SK_U32 TmpVal32;
	
	SK_IN32( IoC, HCU_CCSR, &TmpVal32 );
	
	if ( NOT_EC_CHIP )  {

		TmpVal32 |= HCU_CCSR_OS_PRSNT;
	}
	else {
		TmpVal32 |= Y2_ASF_OS_PRES;
	}
	SK_OUT32( IoC, HCU_CCSR, TmpVal32 );	
	FW_DBG_MSG_I(("FwSetOsPresentBit\n"));
}

/*****************************************************************************
*
* FwInit0 - Init0 function of Firmware
*
* Description:
* 	 Initialises the data structures
*
* Returns:
*   
*/
void FwInit0(
	SK_AC  *pAC,  /* Pointer to adapter context */
	SK_IOC IoC)   /* IO context handle */
{
	FW_DBG_MSG_I(("FwInit0\n"));

}

/*****************************************************************************
*
* FwResetOsPresentBit
*
* Description: Report to Firmware absence of OS
*
* Returns:
*
*/
void FwResetOsPresentBit(
	SK_AC  *pAC,  /* Pointer to adapter context */
	SK_IOC IoC)   /* IO context */
{
	SK_U32 TmpVal32;
	
	SK_IN32( IoC, HCU_CCSR, &TmpVal32 );
	
	if ( NOT_EC_CHIP )  {

		TmpVal32 &= ~HCU_CCSR_OS_PRSNT;
	}
	else {

		TmpVal32 &= ~Y2_ASF_OS_PRES;
	}
	SK_OUT32( IoC, HCU_CCSR, TmpVal32 );

	FW_DBG_MSG_I(("FwResetOsPresentBit\n"));
}

/*****************************************************************************
*
* FwSetAsfCtrl
*
* Description: FIFO ASF FIFO
*
* Returns:
*
*/
static void FwSetAsfCtrl(
	SK_AC  *pAC,  /* Pointer to adapter context */
	SK_IOC IoC)   /* IO context handle */
{
	SK_U32 TmpVal32;

	if ( NOT_EC_CHIP )  {

		SK_IN32( IoC, ASF_CTRL, &TmpVal32 );
				TmpVal32 &= ~(BIT_0 | BIT_1);
		TmpVal32 |= BIT_1;  /*   Release Reset */
		SK_OUT32( IoC, ASF_CTRL, TmpVal32 );	
	}

	FW_DBG_MSG_I(("FwSetAsfCtrl\n"));
}

/*****************************************************************************
*
* FwInit1 - Init1 function of Firmware
*
* Description:
* 	 Checks and initializes the firmware specific hardware
*
* Returns:
*/
void FwInit1(
	SK_AC  *pAC,  /* Pointer to adapter context */
	SK_IOC IoC)   /* IO context handle */
{
	SK_IN8( IoC, B2_CHIP_ID, &(pAC->FwCommon.ChipID));
	pAC->FwOs.FileIndex = 0;

	/* we need to save spi and pxe presence flags at begin
	 * to avoid read access to flash during programming
	 */
	pAC->FwApp.PxeSPI = FwCheckSPI(pAC, IoC);

	/*   check settings in ASF control register */
	FwSetAsfCtrl( pAC, IoC );
}

SK_BOOL FwRemoveImage(
	SK_AC  *pAC,  /* Pointer to adapter context */
	SK_IOC IoC)   /* IO context handle */
{
	FW_DBG_MSG_I(("FwRemoveImage:...\n"));
	
	FwRemoveFirmware( pAC, IoC );

	return SK_TRUE;
}

/*****************************************************************************
*
* FwCpuState
*
* Description:	Check state of CPU
*
* Returns:
*
*/
SK_U8 FwCpuState(
	SK_AC  *pAC,  /* Pointer to adapter context */
	SK_IOC IoC)   /* IO context handle */
{
	SK_U32 TmpVal32;

	SK_IN32( IoC, HCU_CCSR, &TmpVal32 );

	if ( NOT_EC_CHIP )  {

		if( (TmpVal32 & HCU_CCSR_UC_STATE_MSK) == 0 ) {

			return( FW_CPU_STATE_RESET );
		}
		else if ( (TmpVal32 & HCU_CCSR_UC_STATE_MSK) == 1 ) {

			return( FW_CPU_STATE_RUNNING );
		}
		else {

			return( FW_CPU_STATE_UNKNOWN );
		}
	}
	else  { /*  if ( NOT_EC_CHIP ) */

		if( TmpVal32 & Y2_ASF_RESET ) {

			return( FW_CPU_STATE_RESET );
		}

		if( TmpVal32 & Y2_ASF_RUNNING ) {

			return( FW_CPU_STATE_RUNNING );
		}
	} /*  if ( NOT_EC_CHIP ) */

	return( FW_CPU_STATE_UNKNOWN );
}

#ifndef MV_INCLUDE_SDK_SUPPORT
/*****************************************************************************
*
* FwIsFwRunning
*
* Description: 
*
* if CPU is running send CHECK ALIVE command to firmware and wait for answer
* 
* Returns:
* 
* 0 if firmware doesn't answer
* 1 if firmware answers
*/
static SK_U8 FwIsFwRunning(
	SK_AC  *pAC,  /* Pointer to adapter context */
	SK_IOC IoC)   /* IO context handle */
{
	SK_U8 Alive;

	/* SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("AsfCheckAliveCpu -> \n")); */

	Alive = 0;  /*   Not alive */

	/*   check, whether cpu is in reset state... */
	if( FwCpuState(pAC, IoC) == ASF_CPU_STATE_RUNNING )  {

		if( FwHciSendCommand( pAC, IoC,
			YASF_HOSTCMD_CHECK_ALIVE, 0, 0, 0, ASF_HCI_WAIT, 2 )
			== HCI_EN_CMD_READY )  {

			Alive = 1;
			FW_DBG_MSG_I(("FwIsFwRunning: *** Firmware is running ***\n"));
		}
		else  {
			FW_DBG_MSG_I(("FwIsFwRunning: *** No answer from Firmware ***\n"));
		}
	}  /*   if( AsfCpuState(pAC ) != ASF_CPU_STATE_RESET )  { */
	else  {
		FW_DBG_MSG_I(("FwIsFwRunning: *** CPU is not running ***\n"));
	}

	return( Alive );
}
#endif

/*****************************************************************************
*
* FwResetCpu 
*
* Description: Reset CPU
*
* Returns:
*
*/
void FwResetCpu(
	SK_AC  *pAC,  /* Pointer to adapter context */
	SK_IOC IoC)   /* IO context handle */
{
	SK_U32 TmpVal32;

	if ( NOT_EC_CHIP )  {

		if( pAC->FwCommon.ChipID == CHIP_ID_YUKON_SUPR )
			SK_OUT32(IoC, CPU_WDOG, 0); /* stop the watch dog */
		
		SK_IN32( IoC, HCU_CCSR, &TmpVal32 );
		TmpVal32 &= ~(BIT_0 | BIT_1);
		SK_OUT32( IoC, HCU_CCSR, TmpVal32 );
	}
	else  {
		SK_IN32( IoC, HCU_CCSR, &TmpVal32 );
		TmpVal32 &= ~(BIT_2 | BIT_3);
		TmpVal32 |= BIT_3;
		SK_OUT32( IoC, HCU_CCSR, TmpVal32 );
	}
}

/*****************************************************************************
*
* FwStartCpu
*
* Description:
*
* Returns:
*
*/
void FwStartCpu(
	SK_AC  *pAC,  /* Pointer to adapter context */
	SK_IOC IoC)   /* IO context */
{
	SK_U32 	TmpVal32;
	SK_U16	TmpVal16;
	
	if( NOT_EC_CHIP)  {

		/* W A R 
		 * CPU clock divider shouldn't be used because
		 * - ASF firmware may malfunction
		 * - Yukon-Supreme: Parallel FLASH doesn't support divided clocks
		 */
		if ( pAC->FwCommon.ChipID == CHIP_ID_YUKON_SUPR )  {
			
			SK_IN16(IoC, HCU_CCSR, &TmpVal16);
			
			if ((TmpVal16 & HCU_CCSR_CPU_CLK_DIVIDE_MSK) != 0) {

				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("\n*******************************************\n"));
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,(  "*  WARNING: CLOCK DIVIDE WAS SET : 0x%x    \n", TmpVal16));
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,(  "*******************************************\n"));
				TmpVal16 &= ~HCU_CCSR_CPU_CLK_DIVIDE_MSK;
				SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_ASF_E010, SKERR_ASF_E010MSG);
			}
			SK_OUT16(IoC, HCU_CCSR, TmpVal16);
		}
		
		SK_OUT32( IoC, ASF_CTRL, 0x02 );	/* enable FIFO & RAM access */
		SK_IN32( IoC, HCU_CCSR, &TmpVal32 );
		TmpVal32 &= ~HCU_CCSR_UC_STATE_MSK;   /*  reset halt if set */
		TmpVal32 |= HCU_CCSR_ASF_RUNNING;  /*  Set to running state */
		SK_OUT32( IoC, HCU_CCSR, TmpVal32 );
	}
	else  { /* if( NOT_EC_CHIP) */

		SK_IN32( IoC, B28_Y2_ASF_STAT_CMD, &TmpVal32 );
		TmpVal32 &= ~Y2_ASF_UC_STATE;
		TmpVal32 |= Y2_ASF_RESET;  /*  Set to Reset state (Clk running) */
		SK_OUT32( IoC, B28_Y2_ASF_STAT_CMD, TmpVal32 );
		TmpVal32 &= ~Y2_ASF_UC_STATE;
		TmpVal32 |= Y2_ASF_RUNNING;  /*  Set to running state */
		SK_OUT32( IoC, B28_Y2_ASF_STAT_CMD, TmpVal32 );
	
	} /* if( NOT_EC_CHIP) */
}

/*****************************************************************************
*
* FwSmartResetCpu
*
* Description:
*
* Returns:
*
*/
void FwSmartResetCpu(
	SK_AC  *pAC,  /* Pointer to adapter context */
	SK_IOC IoC,   /* IO context */
	SK_U8 Cold )
{
	SK_U64 StartTime, CurrTime;
	SK_U8   ResetCommand;

	if ( Cold ) {
		ResetCommand = YASF_HOSTCMD_RESET_COLD;
	}
	else {
		ResetCommand = YASF_HOSTCMD_RESET;
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("FwSmartResetCpu -> \n"));

	/*   check, whether cpu is already in reset state... */
	if ( FwCpuState(pAC, IoC) == ASF_CPU_STATE_RUNNING )  {

		/*  ...if not, try SmartReset */
		SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("FwSmartResetCpu -> Send reset command\n"));
		
		if ( pAC->FwCommon.ChipID == CHIP_ID_YUKON_SUPR ) {

			SK_OUT32(IoC, CPU_WDOG, 0); /* stop the watch dog */
		}

		if ( FwHciSendCommand( pAC, IoC, ResetCommand, 0, 0, 0, ASF_HCI_WAIT, 2 ) != HCI_EN_CMD_READY )  {

			/*   if Smart Reset fails, do hard reset */
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("FwSmartResetCpu -> Do hard reset\n"));
			FwResetCpu( pAC, IoC );
		}

		StartTime = SkOsGetTime(pAC);

		while( FwCpuState(pAC, IoC ) != ASF_CPU_STATE_RESET ) {

			CurrTime = SkOsGetTime(pAC);

			if ( (CurrTime - StartTime) > (SK_TICKS_PER_SEC*5) )  {
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("FwSmartResetCpu -> Error: CPU is not in reset state - Do hard reset\n"));
				FwResetCpu( pAC, IoC );
				break;
			}
		}  /*   while( AsfCpuState(pAC ) != ASF_CPU_STATE_RESET ) { */

		if( FwCpuState(pAC, IoC ) == ASF_CPU_STATE_RESET )
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("FwSmartResetCpu -> CPU is in reset state\n"));

	}  /*   if( AsfCpuState(pAC ) != ASF_CPU_STATE_RESET )  { */
	else  {
		SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("FwSmartResetCpu -> Cpu already in reset state\n"));
	}
}

/*****************************************************************************
*
* FwEnableFlushFifo
*
* Description:
*
* Returns:
*
*/
static void FwEnableFlushFifo(
	SK_AC  *pAC,
	SK_IOC IoC )
{
	SK_U32 TmpVal32;

#ifdef CHIP_ID_YUKON_EX
	SK_IN32( IoC, RXMF_TCTL, &TmpVal32 );
	TmpVal32 &= ~(BIT_23 | BIT_22);
	TmpVal32 |= BIT_23;
	SK_OUT32( IoC, RXMF_TCTL, TmpVal32 );
#endif

}

void FwAsfEnable(
	SK_AC  *pAC,
	SK_IOC IoC )
{
	SK_OUT32(IoC, B0_CTST, Y2_ASF_ENABLE);

	pAC->GIni.GIAsfEnabled = SK_TRUE;   /*  update asf flag for common modules */
	pAC->GIni.GIAsfRunning = SK_TRUE;   /*  update asf flag for common modules */
}

void FwAsfDisable(
	SK_AC  *pAC,
	SK_IOC IoC )
{
	/*  SK_IN32(IoC, B0_CTST, &TmpVal32); */
	/*  TmpVal32 &= ~0x00003000;    // clear bit13, bit12 */
	/*  TmpVal32 |= 0x00001000;             // set bit13 */
	/*  SK_OUT32(IoC, B0_CTST, TmpVal32); */

	/* Don't disable ASF for restart after Standby/Hibernate 
	 * to avoid full initialization in common modules (driver)
	 * that conflicts with running Firmware
	 *
	 * SK_OUT32(IoC, B0_CTST, Y2_ASF_DISABLE);
	 *
	 * pAC->GIni.GIAsfEnabled = SK_FALSE;  // update asf flag for common modules
	 */
}

SK_BOOL FwInit2(
	SK_AC  *pAC,  /* Pointer to adapter context */
	SK_IOC IoC)   /* IO context handle */
{
	SK_FW_HANDLE FileHandle = 0;
	SK_BOOL RetVal = SK_FALSE;
	pAC->FwCommon.ImageName = FwAppGetImageName(pAC);
	FileHandle = FwOsOpenFile(pAC, pAC->FwCommon.ImageName);

	if (FileHandle) {
		/* if image CS ok and version suitable for driver */
		if (FwIsImageOk(pAC, FileHandle)) {
			FwAppPatchImage(pAC, IoC); /* prepare patches for image */
			pAC->FwCommon.ImageSize = FwOsGetFileSize(pAC, FileHandle);
			pAC->FwCommon.ImageChkSum = FwChkSumImage(pAC, FileHandle);

			/* calculate new ChkSum */
			if (!FwIsFwInFlashOk(pAC, IoC) ) { /* Is Fw in Flash need update?*/
				RetVal = FwProgrammImage(pAC, IoC, FileHandle);
			}
			else {
				RetVal = SK_TRUE; /* Firmware is ok */
			}

			FwFreePatchMemory(pAC);
		}

		FwOsCloseFile(pAC, FileHandle);
	}
	else { /* firmware image was not found */
#ifndef MV_INCLUDE_SDK_SUPPORT
		if (FwIsFwRunning(pAC, IoC)) {

			/* remove fw from flash and clear parameters */
			FwRemoveImage(pAC, IoC); 
			FwAsfDisable(pAC, IoC); 
		}
#endif
	}

	return (RetVal);
}

/*****************************************************************************
*
* FwInit - Init function of FW common module
*
* Description:
*   SK_INIT_DATA: Initialises the data structures
*   SK_INIT_IO: Resets the XMAC statistics, determines the device and
*    connector type.
*   SK_INIT_RUN: Starts a timer event for port switch per hour
*    calculation.
*
* Returns:
*   1 on success
*   0 on error
*/
int FwInit(
	SK_AC  *pAC,   /* Pointer to adapter context */
	SK_IOC IoC,    /* IO context handle */
	int    Level)  /* Initialization level */
{
	int RetCode;

	RetCode = SK_FALSE;

	FW_DBG_MSG_I(("FwInit: Called, level=%d\n", Level));

	switch(Level) {

	case SK_INIT_DATA:

		FwInit0(pAC, IoC);
		RetCode = FwAppInit0(pAC, IoC);
		break;

	case SK_INIT_IO:

		FwInit1(pAC, IoC);
		RetCode = FwAppInit1(pAC, IoC);
		break;

	case SK_INIT_RUN:

		if ((RetCode = FwInit2(pAC, IoC)) == SK_TRUE) {
			/* FW image is ok */
			FW_DBG_MSG_I(("FwInit: Image OK, call FwAppInit2\n"));
			RetCode = FwAppInit2(pAC, IoC);
		}
		break;

	default:
		break; /* Nothing todo */
	}

	return( RetCode );
}

/*****************************************************************************
*
* FwDeInit - DeInit function of FW common module
*
* Description:
*
* Returns:
*   Always 0
*/
int FwDeInit(
	SK_AC  *pAC,  /* Pointer to adapter context */
	SK_IOC IoC )  /* IO context handle */
{
	/*  Reset OS Present Flag in ASF Status and Command Register */
	FwResetOsPresentBit( pAC, IoC );

	if( pAC->FwApp.InitState == ASF_INIT_OK )  {
		switch(pAC->FwApp.OpMode)  {
			case SK_GEASF_MODE_ASF:
				/*   Enable ARP pattern, ASF FW is now responsible for ARP handling */
				FwEnablePattern ( pAC, IoC, 0, ASF_PATTERN_ID_ARP );
				break;
			case SK_GEASF_MODE_IPMI:
				/*   Enable ARP pattern, ASF FW is now responsible for ARP handling */
				FwEnablePattern ( pAC, IoC, 0, ASF_PATTERN_ID_ARP );

				if (pAC->FwApp.DualMode == SK_GEASF_Y2_DUALPORT) {
					FwEnablePattern ( pAC, IoC, 1, ASF_PATTERN_ID_ARP );
				}
				break;
			case SK_GEASF_MODE_DASH:
			case SK_GEASF_MODE_DASH_ASF:
				/*   Enable ARP pattern, ASF FW is now responsible for ARP handling */
				FwEnablePattern( pAC, IoC, 0, ASF_DASH_PATTERN_NUM_ARP );
				FwEnablePattern( pAC, IoC, 0, ASF_DASH_PATTERN_NUM_ICMP );		
				break;
		} /*   switch(pAC->FwApp.OpMode)  */

		/*   Inform the FW that the driver will be unloaded */
		FwHciSendCommand( pAC, IoC, YASF_HOSTCMD_DRV_GOODBYE, 0, 0, 0, ASF_HCI_WAIT, 2 );

		/*   will be done in FW now */
		if( (pAC->FwApp.ChipMode == SK_GEASF_CHIP_EX) ||
			(pAC->FwApp.ChipMode == SK_GEASF_CHIP_SU)  )
			FwEnableFlushFifo( pAC, IoC );
	}


	/* SkPflCheck( pAC, IoC); */   /*   just for debugging */

	if( SK_FW_DBG(pAC) & DEBUGFW_ERASE_FLASH_DURING_UNLOAD )  {
		FwRemoveFirmware( pAC, IoC );
	}

	return( 0 );
}

/*****************************************************************************
*
* FwDeInitStandBy - StandBy -DeInit function of FW common module
*
* Description:
*
* Returns:
*   Always 0
*/
int FwDeInitStandBy(
	SK_AC  *pAC,  /* Pointer to adapter context */
	SK_IOC IoC )  /* IO context handle */
{
	if( pAC->FwApp.InitState == ASF_INIT_OK )  {	
		
		/*   will be done in FW now */
		if( (pAC->FwApp.ChipMode == SK_GEASF_CHIP_EX) ||
			(pAC->FwApp.ChipMode == SK_GEASF_CHIP_SU)  )
		  	FwEnableFlushFifo( pAC, IoC );

		switch(pAC->FwApp.OpMode)  {
			case SK_GEASF_MODE_ASF:
			case SK_GEASF_MODE_IPMI:
				/*   Enable ARP pattern, ASF FW is now responsible for ARP handling */
				FwEnablePattern ( pAC, IoC, pAC->FwApp.ActivePort, ASF_PATTERN_ID_ARP );
				break;
			case SK_GEASF_MODE_DASH:
			case SK_GEASF_MODE_DASH_ASF:
				/*   Enable ARP pattern, ASF FW is now responsible for ARP handling */
				FwEnablePattern( pAC, IoC, 0, ASF_DASH_PATTERN_NUM_ARP );
				FwEnablePattern( pAC, IoC, 0, ASF_DASH_PATTERN_NUM_ICMP );		
				break;
		} /*   switch(pAC->FwApp.OpMode)  */
		
		/*   Inform the FW that the driver will be unloaded */
		FwHciSendCommand( pAC ,IoC , YASF_HOSTCMD_DRV_STANDBY, 0, 0, 0, ASF_HCI_WAIT, 2 );
		pAC->FwApp.StandBy = 1;
	}

	return( 0 );
}

/*****************************************************************************
*
* FwInitStandBy - StandBy - Init function of FW common module
*
* Description:
*
* Returns:
*   Always 0
*/
int FwInitStandBy(
	SK_AC  *pAC,  /* Pointer to adapter context */
	SK_IOC IoC,   /* IO context handle */
	int    Level) /* Initialization level */
{
	SK_EVPARA EventParam; /* Event struct for timer event */

	if( pAC->FwApp.InitState == ASF_INIT_OK )  {
		switch(Level) {
			case SK_INIT_DATA:
				break;

			case SK_INIT_IO:
				break;

			case SK_INIT_RUN:
				/*  Set OS Present Flag in ASF Status and Command Register */
				FwSetOsPresentBit( pAC ,IoC );
				/*   Disable ARP pattern, host system takes over the ARP handling */
				FwDisablePattern( pAC, IoC,pAC->FwApp.ActivePort, ASF_PATTERN_ID_ARP);
				/*   Inform the FW that the driver will be activated again */
				FwHciSendCommand( pAC ,IoC , YASF_HOSTCMD_DRV_HELLO, 0, 0, 0, ASF_HCI_WAIT, 2 );
				SK_MEMSET((char *)&EventParam, 0, sizeof(EventParam));
				SkTimerStart(pAC, IoC, &pAC->FwApp.AsfTimer,
							5000000, SKGE_ASF, SK_ASF_EVT_TIMER_EXPIRED, EventParam);
				pAC->FwApp.Mib.NewParam = 1;
				pAC->FwApp.StandBy = 0;
				break;
		}  /*   switch( Level ) */
	}  /*   if( pAC->FwApp.InitState == ASF_INIT_OK ) */

	return( 0 );
}

/*****************************************************************************
*
* FwEvent - Event handler
*
* Description:
*   Handles the following events:
*   SK_ASF_EVT_TIMER_EXPIRED When a hardware counter overflows an
*
* Returns:
*   Always 0
*/
int FwEvent(
	SK_AC     *pAC,   /* Pointer to adapter context */
	SK_IOC    IoC,    /* IO context handle */
	SK_U32    Event,  /* Event-Id */
	SK_EVPARA Param)  /* Event dependent parameter */
{
	switch(Event) {
		case SK_ASF_EVT_TIMER_EXPIRED:
			/*  SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("FW: FwEvent -> Timer expired\n" )); */
			FwMainStateMachine( pAC, IoC );
			break;

		case SK_ASF_EVT_TIMER_HCI_EXPIRED:
			/*   SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("FW: FwEvent -> TimerWrHci expired GlHciState:%d\n", */
			/*                                                 pAC->FwApp.GlHciState )); */
			FwHciStateMachine( pAC, IoC, 1 );
			break;

		default:
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("FW: FwEvent -> Unknown Event:0x%x\n", Event ));
			break;
	}
	return(0); /* Success. */
} /* FwEvent */

