/******************************************************************************
 *
 * Name:    skgeasf.c
 * Project: Gigabit Ethernet Adapters, Common Modules
 * Version: $Revision: 1.1.2.6 $
 * Date:    $Date: 2007/06/28 09:40:54 $
 * Purpose: ASF Handler.
 *
 ******************************************************************************/

/******************************************************************************
 *
 *  (C)Copyright 1998-2002 SysKonnect GmbH.
 *  (C)Copyright 2002-2003 Marvell.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  The information in this file is provided "AS IS" without warranty.
 *
 ******************************************************************************/

/******************************************************************************
 *
 * Description:
 *
 * This module is intended to handle all the asf functions
 *
 * Include File Hierarchy:
 *
 *   "h/skdrv1st.h"
 *   "h/skdrv2nd.h"
 *
 ******************************************************************************/

/*
 Event queue and dispatcher
*/
#if (defined(DEBUG) || ((!defined(LINT)) && (!defined(SK_SLIM))))
static const char SysKonnectFileId[] =
"$Header: /data/cvs/sweprojects/yukon2/lindrv/asf_linux/Attic/skgeasf.c,v 1.1.2.6 2007/06/28 09:40:54 marcusr Exp $" ;
#endif

#define __SKASF_C

#ifdef __cplusplus
extern "C" {
#endif  /* cplusplus */


// #include <ntddk.h>
// #include <wdm.h>


#include "h/sktypes.h"
#include "h/skdrv1st.h"
#include "h/skdrv2nd.h"
#include "h/skgeasf.h"
#include "h/skgespi.h"
#include "h/skfops.h"
#include <acpi/acpi.h>

//#include "h/yuk.h"
//#include "h/skvpd.h"

//#include <stdlib.h>

static char *AsfFileName  = "/etc/sk98lin/AcpiAsf.bin";
#ifndef USE_ASF_DASH_FW
static char *IpmiFileNameS1  = "/etc/sk98lin/ipmiyk2-s1.bin";
static char *IpmiFileNameS2  = "/etc/sk98lin/ipmiyk2-s2.bin";
#endif
#ifdef USE_ASF_DASH_FW
static char *DashFileNameS1  = "/etc/sk98lin/dashyex-s1.bin";
static char *DashFileNameS2  = "/etc/sk98lin/dashyex-s2.bin";
#endif
// static char *SimuAsfTab 	= "/etc/sk98lin/AcpiAsf.bin";

// ARP pattern 40 byte (5 bytes in mask)
// this pattern length corresponds with YLCI_MACRXFIFOTHRES
// Pattern mask for ARP Frames
#ifdef ASF_ONLY_ARP_REQUEST
static SK_U8 ARP_FRAME_PATTERN[] =
{
    /* MAC Header - 14 bytes */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /*Dest MAC Addr */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /*Src MAC Addr  */
    0x08, 0x06,                              /*Frame Type    */
    /* ARP Header - 28 bytes */
    0x00, 0x01,                              /* hard type    */
    0x08, 0x00,                              /* prot type    */
    0x06,                                    /* hard size    */
    0x04,                                    /* prot size    */
    0x00, 0x01,                              /* op = request */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /* senders mac  */
    0x00, 0x00, 0x00, 0x00,                  /* senders ip   */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /* target mac   */
    0x00, 0x00};
static SK_U8 ARP_PATTERN_MASK[] = { 0x00, 0xF0, 0x3F, 0x00, 0x00 };
#else
static SK_U8 ARP_FRAME_PATTERN[] =
{
    /* MAC Header - 14 bytes */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /*Dest MAC Addr */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /*Src MAC Addr  */
    0x08, 0x06,                              /*Frame Type    */
    /* ARP Header - 28 bytes */
    0x00, 0x01,                              /* hard type    */
    0x08, 0x00,                              /* prot type    */
    0x06,                                    /* hard size    */
    0x04,                                    /* prot size    */
    0x00, 0x00,                              /* op = request */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /* senders mac  */
    0x00, 0x00, 0x00, 0x00,                  /* senders ip   */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /* target mac   */
    0x00, 0x00};
static SK_U8 ARP_PATTERN_MASK[] = { 0x00, 0xF0, 0x00, 0x00, 0x00 };
#endif

// RSP pattern - 40 bytes (this makes 5 bytes in RSP_PATTERN_MASK)
// this pattern length corresponds with YLCI_MACRXFIFOTHRES
static SK_U8 RSP_FRAME_PATTERN[] =
{   /* MAC Header (14 bytes) */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /*Dest MAC Addr*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /*Src MAC Addr */
    0x08, 0x00,                             /*Frame Type   */
    /* IP Header (20 bytes) */
    0x45, 0x00, 0x00, 0x00,                 /* Version & Header Length */
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x11, 0x00, 0x00,                 /* Protocol */
    0x00, 0x00, 0x00, 0x00,                 /*Src IP address*/
    0x00, 0x00, 0x00, 0x00,                 /*My IP address*/
    /* part of UDP Header (6 bytes) */
    0x00, 0x00,                             /* src port   */
    0x02, 0x98,                             /* dest. port */
    0x00, 0x00};                            /* length     */

// Pattern mask for RSP Frames
static SK_U8 RSP_PATTERN_MASK[] = { 0x00, 0x70, 0x80, 0x00, 0x30 };

// RMCP pattern (unsecure port)
// this pattern length corresponds with YLCI_MACRXFIFOTHRES
static SK_U8 RMCP_FRAME_PATTERN[] =
{   /* MAC Header */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /*Dest MAC Addr*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /*Src MAC Addr */
    0x08, 0x00,                              /*Frame Type   */
    /* IP Header */
    0x45, 0x00, 0x00, 0x00,                  /* Version & Header Length */
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x11, 0x00, 0x00,                  /* Protocol */
    0x00, 0x00, 0x00, 0x00,                  /*Src IP address*/
    0x00, 0x00, 0x00, 0x00,                  /*My IP address*/
    /* UDP Header */
    0x00, 0x00,                             /* src port */
    0x02, 0x6f,                             /* unsecure dest. port */
    0x00, 0x00};

// Pattern mask for RMCP Frames
static SK_U8 RMCP_PATTERN_MASK[] = { 0x00, 0x70, 0x80, 0x00, 0x30 };

#if 0
// TCP pattern
static SK_U8 TCP_FRAME_PATTERN[] =
{   /* MAC Header */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /*Dest MAC Addr*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /*Src MAC Addr */
    0x08, 0x00,                              /*Frame Type   */
    /* IP Header */
    0x45, 0x00, 0x00, 0x00,                  /* Version & Header Length */
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x06, 0x00, 0x00,                  /* Protocol */
    0x00, 0x00, 0x00, 0x00,                  /*Src IP address*/
    0x00, 0x00, 0x00, 0x00,                  /*My IP address*/
    /* UDP Header */
    0x00, 0x00,                             /* src port */
    0x02, 0x6f,                             /* unsecure dest. port */
    0x00, 0x00};

// Pattern mask for TCP Frames
static SK_U8 TCP_PATTERN_MASK[] = 
{ 0x00, 0x70, 0x80, 0x00, 0x30 };
#endif

// ICMP pattern
static SK_U8 ICMP_FRAME_PATTERN[] =
{   /* MAC Header */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /*Dest MAC Addr*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /*Src MAC Addr */
    0x08, 0x00,                              /*Frame Type   */
    /* IP Header */
    0x45, 0x00, 0x00, 0x00,                  /* Version & Header Length */
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00,                  /* Protocol */
    0x00, 0x00, 0x00, 0x00,                  /*Src IP address*/
    0x00, 0x00, 0x00, 0x00,                  /*My IP address*/
    /* ICMP Header */
    0x00, 0x00,                          
    0x00, 0x00,                      
    0x00, 0x00};

// Pattern mask for ICMP Frames
static SK_U8 ICMP_PATTERN_MASK[] = 
{ 0x00, 0x70, 0x80, 0x00, 0x00 };

// SNMP pattern
static SK_U8 SNMP_FRAME_PATTERN[] =
{   /* MAC Header */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /* Dest MAC Addr */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /* Src MAC Addr */
    0x08, 0x00,                              /* Frame Type */
    /* IP Header */
    0x45, 0x00, 0x00, 0x00,                  /* Version & Header Length */
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x11, 0x00, 0x00,                  /* Protocol */
    0x00, 0x00, 0x00, 0x00,                  /* Src IP address */
    0x00, 0x00, 0x00, 0x00,                  /* My IP address */
    /* UDP Header */
    0x00, 0x00,                              /* src port */
    0x00, 0xa1,                              /* unsecure dest. port */
    0x00, 0x00};

// Pattern mask for SNMP Frames
static SK_U8 SNMP_PATTERN_MASK[] = 
{ 0x00, 0x70, 0x80, 0x00, 0x30 };

/*****************************************************************************
*
* SkAsfRestorePattern - interface function (global symbol)
*
* Description:
* restores pattern for ASF and IPMI
*
* Returns:
*   Always 0
*/

int SkAsfRestorePattern (
    SK_AC *pAC ,    /* Pointer to adapter context */
    SK_IOC IoC)     /* IO context handle */
{
    if (pAC->AsfData.OpMode == SK_GEASF_MODE_ASF) {

        // asf mode ->  we are running on
        // yukon ec with only one port
        AsfSetUpPattern(pAC, IoC, 0);

    } else {
        if (pAC->AsfData.OpMode == SK_GEASF_MODE_IPMI) {
            // ipmi mode ->  we are running on
            // yukon 2 with at least one port
            AsfSetUpPattern(pAC, IoC, 0);       // port A

            if (pAC->AsfData.DualMode == SK_GEASF_Y2_DUALPORT) {
                AsfSetUpPattern(pAC, IoC, 1);   // port B
            }
        }
    }

    return (SK_ASF_PNMI_ERR_OK);
}

/*****************************************************************************
*
* SkAsfInit - Init function of ASF
*
* Description:
*   SK_INIT_DATA: Initialises the data structures
*   SK_INIT_IO: Resets the XMAC statistics, determines the device and
*    connector type.
*   SK_INIT_RUN: Starts a timer event for port switch per hour
*    calculation.
*
* Returns:
*   Always 0
*/
int SkAsfInit(
             SK_AC *pAC,    /* Pointer to adapter context */
             SK_IOC IoC,    /* IO context handle */
             int Level)     /* Initialization level */
{
	SK_U32          TmpVal32;
	SK_U32          FlashOffset = 0;
	SK_U32          i;

	SK_U32          FileLengthS1;
	SK_U32          FileLengthS2;
	char            *FwFileNameS1 = NULL;
	char            *FwFileNameS2 = NULL;
	SK_U8           *pAsfFwS1 = NULL;
	SK_U8           *pAsfFwS2 = NULL;

	SK_U8           FlashOk;
	int             RetCode;
	SK_BOOL         DoUpdate = SK_FALSE;
	SK_U8           lRetCode;
	SK_U32          FwImageCsOk;
	SK_U32          FwFlashCsOk;
	SK_U32          FwImageCs = 0;
	SK_U32          FwFlashCs = 0;
	SK_U32          FwCs;
	SK_U32          *pTmp32;
	SK_U8           *pHciRecBuf;
	SK_EVPARA       EventParam; /* Event struct for timer event */
	unsigned long   FlashSize;
	unsigned long   EraseOff = 0;
	SK_U32          SpiRetVal;
	SK_U8           Tmp1Val8, Tmp2Val8;
	SK_BOOL         YukonEcA1;
	SK_U8           OldGuid[16];
	SK_U8           AsfFlag = 0, IpmiFlag = 0, AsfDashFlag = 0;
	SK_U8           AsfHintBit = 0, IpmiHintBit = 0, NoHintBit = 0;

	RetCode = SK_ASF_PNMI_ERR_OK;

	SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,
		("ASF: SkAsfInit: Called, level=%d  sizof ASF-MIB:0x%x Bytes\n", Level, sizeof(STR_ASF_MIB) ) );

	/* YukonEcA1 introduced by rschmidt */
	YukonEcA1 = (pAC->GIni.GIChipId == CHIP_ID_YUKON_EC && pAC->GIni.GIChipRev == CHIP_REV_YU_EC_A1);

	switch(Level)
	{
	case SK_INIT_DATA:
		/* Set structure to zero */
		//  This will be done in function "AsfReadConfiguration"
		//  SK_MEMSET((char *)&pAC->AsfData, 0, sizeof(pAC->AsfData));

		pAC->AsfData.ActivePort = 0;
		pAC->AsfData.OpMode     = SK_GEASF_MODE_IPMI;
		pAC->AsfData.ChipMode   = SK_GEASF_CHIP_UNKNOWN;

		pAC->AsfData.InitState  = ASF_INIT_UNDEFINED;
		break;

	case SK_INIT_IO:
#if (0)
		/*  Set OS Present Flag in ASF Status and Command Register */
		SK_IN32( IoC, REG_ASF_STATUS_CMD, &TmpVal32 );
		TmpVal32 |= BIT_4;
		SK_OUT32( IoC, REG_ASF_STATUS_CMD, TmpVal32 );
#endif

#ifdef USE_ASF_DASH_FW
		AsfResetOsPresentBit( pAC, IoC );

		YlciEnablePattern( pAC, IoC, 0, ASF_DASH_PATTERN_NUM_ARP );
		YlciEnablePattern( pAC, IoC, 0, ASF_DASH_PATTERN_NUM_ICMP );
		YlciEnablePattern(pAC, IoC, 0, ASF_DASH_PATTERN_NUM_SNMP);
#endif

#ifdef ASF_CHECK_HIDDEN_ID  // here we will check hidden id _and_ chip id

		/* check chip id */
		SK_IN8( IoC, B2_CHIP_ID, &Tmp1Val8 );
		switch(Tmp1Val8)
		{

#ifdef CHIP_ID_YUKON_EX					
		case CHIP_ID_YUKON_EX:
		  pAC->AsfData.ChipMode = SK_GEASF_CHIP_EX;
		  
		  //pAC->AsfData.FlashOffs 	= ASF_FLASH_EX_OFFS;
		  //pAC->AsfData.FlashOffsVer 	= ASF_FLASH_EX_OFFS_VER;
		  //pAC->AsfData.FlashOffsRev 	= ASF_FLASH_EX_OFFS_REV;
		  //pAC->AsfData.FlashOffsCs 	= ASF_FLASH_EX_OFFS_CS;
		  //pAC->AsfData.FlashOffsGuid	= ASF_FLASH_EX_OFFS_GUID;
		  //pAC->AsfData.FlashOffsAcpi	= ASF_FLASH_EX_OFFS_ACPI;
		  break;
#endif  //  CHIP_ID_YUKON_EX
		case CHIP_ID_YUKON_EC:
			/* YUKON_EC */
			/* chip-id is ok, check hidden id */
			SK_IN8( IoC, B2_MAC_CFG, &Tmp2Val8 );
			Tmp2Val8 &= 0x03;
			if( (Tmp2Val8 != 1) &&       //  88E8052
				(Tmp2Val8 != 3) ) {      //  88E8050
				RetCode = SK_ASF_PNMI_ERR_GENERAL;
			} else {
				pAC->AsfData.ChipMode = SK_GEASF_CHIP_EC;
			}
			break;
		case CHIP_ID_YUKON_XL:
			/* YUKON_2 */
                        /* chip-id is ok, check hidden id */
			SK_IN8( IoC, B2_MAC_CFG, &Tmp2Val8 );
			Tmp2Val8 &= 0x03;
			if(Tmp2Val8 != 0) {
				RetCode = SK_ASF_PNMI_ERR_GENERAL;
			} else {
				pAC->AsfData.ChipMode = SK_GEASF_CHIP_Y2;
			}
			break;
		default:
			/* Nothing to do. Chip id does not match */
			RetCode = SK_ASF_PNMI_ERR_GENERAL;
			break;
		}

		if (RetCode != SK_ASF_PNMI_ERR_OK) {

			pAC->AsfData.InitState = ASF_INIT_ERROR_CHIP_ID;
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("SkAsfInit: *** ASF/IPMI NOT SUPPORTED ***\n"));

			/* hidden ID doesn't match (which card do we access?)
			// do not set any registers

			AsfDisable(pAC, IoC);

			AsfResetCpu(pAC, IoC);       // reset cpu

			// disable all pattern for asf/ipmi
			YlciDisablePattern(pAC, IoC, 0, 4);
			YlciDisablePattern(pAC, IoC, 0, 5);
			YlciDisablePattern(pAC, IoC, 0, 6);

			if ( (CHIP_ID_YUKON_2(pAC)) && (pAC->GIni.GIMacsFound == 2) ) {
				// do not forget the second link
				// disable all pattern for asf/ipmi
				YlciDisablePattern(pAC, IoC, 1, 4);
				YlciDisablePattern(pAC, IoC, 1, 5);
				YlciDisablePattern(pAC, IoC, 1, 6);
			}
			*/
			break;
		}

#endif
		/* CHECK the ASF hint bits...
		 * all YukonII:
		 * Application Information Register auf 0x011e, Bit 7:6
		 * 
		 * Kodierung:
		 *      0b00    kein "hint"; jetziger Zustand
		 *      0b01    customer wants ASF to be loaded
		 *      0b10    customer wants IPMI to be loaded
		 *      0b11    customer does not want ASF or IPMI to go in here
		 * 
		 *  Alle bisherigen EEPROM Versionen bringen 0b00.
		 */
		SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("SkAsfInit: *** CHECK ASF hint bits ***\n"));
		SK_IN32(IoC, B2_Y2_HW_RES, &TmpVal32);
		switch(TmpVal32 & 0xc0) {
		case 0xc0:
			AsfHintBit  = 0;
			IpmiHintBit = 0;
			NoHintBit   = 1;
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("SkAsfInit: *** ASF hint bits: NO ASF/IPMI ***\n"));
			break;
		case 0x40:
			AsfHintBit  = 1;
			IpmiHintBit = 0;
			NoHintBit   = 0;
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("SkAsfInit: *** ASF hint bits: ASF ***\n"));
			break;
		case 0x80:
			AsfHintBit  = 0;
			IpmiHintBit = 1;
			NoHintBit   = 0;
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("SkAsfInit: *** ASF hint bits: IPMI ***\n"));
			break;
		default:
			AsfHintBit  = 0;
			IpmiHintBit = 0;
			NoHintBit   = 1;
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("SkAsfInit: *** ASF hint bits: Default ASF/IPMI ***\n"));
			break;
		}

		/* here we do not know which firmware we must load (ipmi or asf)... */
		pAC->AsfData.OpMode = SK_GEASF_MODE_UNKNOWN;
		AsfFlag  = 0;
		IpmiFlag = 0;
		AsfDashFlag = 0;		

		/* try to open the ASF binary */
		if ( fw_file_exists(pAC, AsfFileName) ) {
			/* here we have found the asf binary */
			
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("SkAsfInit: *** ASF binary file found...\n"));
			AsfFlag = 1;
			FwFileNameS1 = AsfFileName;
			FwFileNameS2 = NULL;
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("*** AsfFlag = 1 ***\n"));
		} else {
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("SkAsfInit: *** ASF binary file _NOT_ found!\n"));
		}


#ifdef USE_ASF_DASH_FW
		/* try to open ASF DASH binary */
		if ( fw_file_exists(pAC, DashFileNameS1)  && 
			fw_file_exists(pAC, DashFileNameS2) ) {
			/* here we have found the ASF DASH binary */
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("SkAsfInit: *** ASF DASH binary file found...\n"));
			AsfDashFlag = 1;
			FwFileNameS1 = DashFileNameS1;
			FwFileNameS2 = DashFileNameS2;
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("*** IpmiFlag = 1 ***\n"));
		} else {
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("SkAsfInit: *** ASF DASH binary file _NOT_ found!\n"));
		}
#else
		/* try to open IPMI binary */
		if ( fw_file_exists(pAC, IpmiFileNameS1)  && 
			fw_file_exists(pAC, IpmiFileNameS2) ) {
			/* here we have found the ipmi binary */
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("SkAsfInit: *** IPMI binary file found...\n"));
			IpmiFlag = 1;
			FwFileNameS1 = IpmiFileNameS1;
			FwFileNameS2 = IpmiFileNameS2;
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("*** IpmiFlag = 1 ***\n"));
		} else {
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("SkAsfInit: *** IPMI binary file _NOT_ found!\n"));
		}
#endif

		/* set the operation mode */
		if ( (AsfFlag == 1) && ( (AsfHintBit == 1) && (IpmiHintBit == 0) && (NoHintBit == 0) ) ) {
		/* we are in the ASF mode */
			if ( (pAC->AsfData.ChipMode == SK_GEASF_CHIP_EC) ||
				(pAC->AsfData.ChipMode == SK_GEASF_CHIP_Y2) ) {
				/* ASF can run on YukonEC and Yukon2 */
				pAC->AsfData.OpMode = SK_GEASF_MODE_ASF;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("SkAsfInit: *** SK_GEASF_MODE_ASF ***\n"));
				YlciDisablePattern(pAC, IoC, 0, 5);  //  Disable ARP pattern, OS is now responsible for ARP handling
			}
		} else {
		/* are we in the ipmi mode ? */
			if ( (IpmiFlag == 1) && ( (IpmiHintBit == 1) && (AsfHintBit == 0) && (NoHintBit == 0) ) ) {
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("*** Ipmi bits OK - ChipMode: %x ***\n", pAC->AsfData.ChipMode));
				if (pAC->AsfData.ChipMode == SK_GEASF_CHIP_Y2) {
					/* IPMI can run only on Yukon2 */
					pAC->AsfData.OpMode = SK_GEASF_MODE_IPMI;

					/* set ASF enable bit in general register (0x0004)
					 * and set the AsfEnable byte in pAC structure
					 * (pAC->GIni.GIAsfEnabled = SK_TRUE)
					 */
					AsfEnable(pAC, IoC);

					/* check if we have a dual port adapter */
					if ( (CHIP_ID_YUKON_2(pAC)) && (pAC->GIni.GIMacsFound == 2) ) {
						pAC->AsfData.DualMode = SK_GEASF_Y2_DUALPORT;
					} else {
						pAC->AsfData.DualMode = SK_GEASF_Y2_SINGLEPORT;
					}

					SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,
						("SkAsfInit: *** SK_GEASF_MODE_IPMI (%d) ***\n", pAC->AsfData.DualMode));

#if 0
					/*  Disable ARP pattern, OS is now responsible for ARP handling */
					YlciDisablePattern(pAC, IoC, 0, 5);
					// AsfSetUpPattern(pAC, IoC, 0);

					if (pAC->AsfData.DualMode == SK_GEASF_Y2_DUALPORT) {
					/* Disable ARP pattern, OS is now responsible for ARP handling */
						YlciDisablePattern(pAC, IoC, 1, 5);
						// AsfSetUpPattern(pAC, IoC, 1);
					}
#endif
				}
			}
		}

#ifdef USE_ASF_DASH_FW
		//  run Dash without hint bit
		if (pAC->AsfData.OpMode == SK_GEASF_MODE_UNKNOWN) {
		  if( (  (pAC->AsfData.ChipMode == SK_GEASF_CHIP_EC)|| 
			 (pAC->AsfData.ChipMode == SK_GEASF_CHIP_EX)  ) && 
		      (AsfDashFlag == 1) && (NoHintBit == 1) ) {

                    // ASF can run on YukonEC without hint bits
                    pAC->AsfData.OpMode = SK_GEASF_MODE_DASH;
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT, ("SkAsfInit: *** SK_GEASF_MODE_DASH EC ***\n"));
		    
		    /* To early, we need to do this if interface is up */
                    // YlciDisablePattern(pAC, IoC, 0, ASF_DASH_PATTERN_NUM_ARP );  //  Disable ARP pattern, OS is now responsible for ARP handling
                    // YlciDisablePattern(pAC, IoC, 0, ASF_DASH_PATTERN_NUM_ICMP);  //  Disable ICMP pattern, OS is now responsible for ICMP handling
		  }
		}
#endif

		if (pAC->AsfData.OpMode == SK_GEASF_MODE_UNKNOWN) {
			if( (pAC->AsfData.ChipMode == SK_GEASF_CHIP_EC) && (AsfFlag == 1) && (NoHintBit == 1) ) {
			/* ASF can run on YukonEC without hint bits */
				pAC->AsfData.OpMode = SK_GEASF_MODE_ASF;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("SkAsfInit: *** SK_GEASF_MODE_ASF EC ***\n"));
				YlciDisablePattern(pAC, IoC, 0, 5);  //  Disable ARP pattern, OS is now responsible for ARP handling
			} else {
			/* error - we could not find our operation mode! */
				pAC->AsfData.InitState = ASF_INIT_ERROR_OPMODE;
				RetCode = SK_ASF_PNMI_ERR_GENERAL;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("SkAsfInit: *** ASF/IPMI UNKNOWN OPMODE ***\n"));

				AsfDisable(pAC, IoC);    // disable pattern matching for ASF/IPMI

				AsfResetCpu(pAC, IoC);       // reset cpu

				/* disable all pattern for asf/ipmi */
				YlciDisablePattern(pAC, IoC, 0, 4);
				YlciDisablePattern(pAC, IoC, 0, 5);
				YlciDisablePattern(pAC, IoC, 0, 6);

				if ( (CHIP_ID_YUKON_2(pAC)) && (pAC->GIni.GIMacsFound == 2) ) {
				/* do not forget the second link
				 * disable all pattern for asf/ipmi
				 */
					YlciDisablePattern(pAC, IoC, 1, 4);
					YlciDisablePattern(pAC, IoC, 1, 5);
					YlciDisablePattern(pAC, IoC, 1, 6);
				}
				break; // leave "case SK_INIT_IO"
			}
		}

		/* Send CheckAlive command to CPU */
		if ( ((pAC->AsfData.OpMode == SK_GEASF_MODE_ASF) ||
		      (pAC->AsfData.OpMode == SK_GEASF_MODE_IPMI) ||
		      (pAC->AsfData.OpMode == SK_GEASF_MODE_DASH)) &&
		     (RetCode == SK_ASF_PNMI_ERR_OK) ) {

		  if( AsfCheckAliveCpu( pAC, IoC ) != 1 )  { //  Not alive
		    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("           ******************************\n"));
		    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfInit: *     CPU is NOT running !   *\n"));
		    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("           ******************************\n"));
		    pAC->AsfData.CpuAlive = 0;
		  } else  {
		    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("           ******************************\n"));
		    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfInit: *        CPU is running      *\n"));
		    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("           ******************************\n"));
		    pAC->AsfData.CpuAlive = 1;
		    
#if 0
		    if( AsfHciSendCommand( pAC, IoC, YASF_HOSTCMD_DRV_HELLO, 0, 0, 0, ASF_HCI_WAIT, 3 ) != HCI_EN_CMD_READY )  {
		      printk("ASF: DRV_HELLO failed\n");
		    } else {
		      printk("ASF: DRV_HELLO OK\n");
		    }
#endif
		  }
		}

		/* START FLASH PROC */
		/* Try to open the FW image file    */
		if (fw_read(pAC,FwFileNameS1,&pAsfFwS1,&FileLengthS1) && 
			fw_read(pAC,FwFileNameS2,&pAsfFwS2,&FileLengthS2)) {
			/* Set the flash offset to 128k */

			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    Flash files opened:\n"));
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("          %s: size: 0x%d offs:0x%x\n", FwFileNameS1, FileLengthS1, FlashOffset));
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("          %s: size: 0x%d offs:0x%x\n", FwFileNameS2, FileLengthS2, FlashOffset));

#ifdef USE_ASF_DASH_FW
			/*  calculate CS of the FW image */
			pTmp32 = (SK_U32 *) pAsfFwS1;
			for( i=0, FwCs=0; i<ASF_DASH_FLASH_SIZE_1; i+=4 )  {
			  FwCs += *pTmp32;
			  pTmp32++;
			}

			pTmp32 = (SK_U32 *) pAsfFwS2;
			for( i=0; i<ASF_DASH_FLASH_SIZE_2; i+=4 )  {
			  FwCs += *pTmp32;
			  pTmp32++;
			}
#else
			/*  calculate CS of the FW image */
			pTmp32 = (SK_U32 *) pAsfFwS1;
			for( i=0, FwCs=0; i<ASF_FLASH_SIZE; i+=4 )  {
				FwCs += *pTmp32;
				pTmp32++;
			}

			pTmp32 = (SK_U32 *) pAsfFwS2;
			for( i=0; i<ASF_FLASH_SIZE; i+=4 )  {
				FwCs += *pTmp32;
				pTmp32++;
			}

#endif

			if( FwCs == 0  )  {  //  CS == 0 => O.K.
				FwImageCsOk = 1;
				FwImageCs = *(pTmp32 - 1);
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    FW Image Checksum O.K. \n"));
			} else  {
				printk("sk98lin: File FW Checksum not OK\n");

				FwImageCsOk = 0;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    *** Error: FW Image Checksum:0x%x\n", FwCs));
			}

#ifdef USE_ASF_DASH_FW
			pAC->AsfData.DriverVersion[0] = 'v';
			pAC->AsfData.DriverVersion[1] = '3';
			pAC->AsfData.DriverVersion[2] = '.';
			pAC->AsfData.DriverVersion[3] = '0';
			pAC->AsfData.DriverVersion[4] = '0';

			for( i=0; i<5; i++ )
				pAC->AsfData.FileFwVersion[i] = *(pAsfFwS2 + ASF_FLASH_EX_OFFS_VER - 65536 + i);

			pAC->AsfData.FileFwRev = *(pAsfFwS2 + ASF_FLASH_EX_OFFS_REV - 65536);
#else
			pAC->AsfData.DriverVersion[0] = 'v';
			pAC->AsfData.DriverVersion[1] = '1';
			pAC->AsfData.DriverVersion[2] = '.';
			pAC->AsfData.DriverVersion[3] = '1';
			pAC->AsfData.DriverVersion[4] = '0';

			for( i=0; i<5; i++ )
				pAC->AsfData.FileFwVersion[i] = *(pAsfFwS2 + ASF_FLASH_OFFS_VER - 65536 + i);

			pAC->AsfData.FileFwRev = *(pAsfFwS2 + ASF_FLASH_OFFS_REV - 65536);
#endif
	
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    FW Image:%c%c%c%c %c Driver:%c%c%c%c\n",
					pAC->AsfData.FileFwVersion[1], pAC->AsfData.FileFwVersion[2],
					pAC->AsfData.FileFwVersion[3], pAC->AsfData.FileFwVersion[4],
					pAC->AsfData.FileFwRev,
					pAC->AsfData.DriverVersion[1], pAC->AsfData.DriverVersion[2],
					pAC->AsfData.DriverVersion[3], pAC->AsfData.DriverVersion[4] ));


			/* check, whether the FW file version suits the driver version */
			if( (pAC->AsfData.FileFwVersion[1] == pAC->AsfData.DriverVersion[1]) &&
				(pAC->AsfData.FileFwVersion[3] == pAC->AsfData.DriverVersion[3]) &&
				(pAC->AsfData.FileFwVersion[4] == pAC->AsfData.DriverVersion[4]) &&
				(FwImageCsOk == 1) ) {

				/* read the flash  (upper 128k) */
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    FW suits the driver´s version\n"));

				if( pAC->AsfData.ChipMode == SK_GEASF_CHIP_EX )  {
				  SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    *** Extreme -> Do not reset !!\n"));
				}
				else if (YukonEcA1)  {  // was if (pAC->GIni.GIChipRev == CHIP_REV_YU_EC_A1) before
				  SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    *** Chip Rev. A1 -> Do reset !!\n"));
				  				  
				  AsfResetCpu(pAC, IoC);


				  // AsfSmartResetCpu( pAC, IoC, ASF_RESET_HOT );  // fixed in A2
				} else  {
					/*
					 * just in case the FW is not running !!
					 * (shouldn´t happen with A2 and later versions)
					 */
					if( !pAC->AsfData.CpuAlive )  {
						SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("  *** FW is not running !!  *** \n"));

						AsfResetCpu(pAC, IoC);
						// AsfSmartResetCpu( pAC, IoC, ASF_RESET_HOT );
						// AsfRunCpu( IoC );
					}
				}

				if (!YukonEcA1) {
#ifdef USE_ASF_DASH_FW
				  AsfLockSpi( pAC, IoC );
#else
				  SK_OUT8(IoC, GPHY_CTRL + 2, 1);  //  Lock the SPI access
#endif
				}

				spi_init_pac( pAC );

				if (!flash_check_spi( &FlashSize )) {
#ifdef USE_ASF_DASH_FW
				  printk("sk98lin: SPI not present!\n");
#endif
				}
				else {
				  SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    Flash found with size of %d KBytes\n", FlashSize/1024));
				}

				/* Read flash low pages */				
#ifdef USE_ASF_DASH_FW
				FwCs = 0;

				SpiRetVal = spi_flash_manage( pAC->AsfData.FlashBuffer, ASF_FLASH_EX_OFFS, ASF_DASH_FLASH_SIZE_1, SPI_READ );

				pTmp32 = (SK_U32 *) pAC->AsfData.FlashBuffer;
				for( i=0; i< ASF_DASH_FLASH_SIZE_1; i+=4 )  {
				  FwCs += *pTmp32;
				  pTmp32++;
				}
#else
				SpiRetVal = spi_flash_manage( pAC->AsfData.FlashBuffer, ASF_FLASH_OFFS, ASF_FLASH_SIZE, SPI_READ );

				if( SpiRetVal == 0 )  {
				/* calculate CS of the FW flash */
					SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    *** Flash low pages loaded. Calculate the CS of the FW flash.\n"));

					pTmp32 = (SK_U32 *) pAC->AsfData.FlashBuffer;
					for( i=0, FwCs=0; i<ASF_FLASH_SIZE; i+=4 )  {
						FwCs += *pTmp32;
						pTmp32++;
					}
				} else {
					SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    *** Error: SPI read\n"));
					SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_ASF_E002, SKERR_ASF_E002MSG);
					RetCode = SK_ASF_PNMI_ERR_GENERAL;
				}
#endif


#ifdef USE_ASF_DASH_FW
				if( SpiRetVal == 0 )  {
				} else {
				  RetCode = SK_ASF_PNMI_ERR_GENERAL;
				}
#endif

				/* Read flash high pages */
#ifdef USE_ASF_DASH_FW
				SpiRetVal = spi_flash_manage( pAC->AsfData.FlashBuffer, ASF_FLASH_EX_OFFS + 65536, ASF_DASH_FLASH_SIZE_2, SPI_READ );
#else
				SpiRetVal = spi_flash_manage( pAC->AsfData.FlashBuffer, ASF_FLASH_OFFS + 65536, ASF_FLASH_SIZE, SPI_READ );
#endif

#ifdef USE_ASF_DASH_FW
				if( SpiRetVal == 0 )  {
				} else {
				  RetCode = SK_ASF_PNMI_ERR_GENERAL;
				}
#endif

				if (!YukonEcA1) {
#ifdef USE_ASF_DASH_FW
				  AsfUnlockSpi( pAC, IoC );
#else
				  SK_OUT8(IoC, GPHY_CTRL + 2, 0);  //  Unlock the SPI access
#endif
				}

#ifdef USE_ASF_DASH_FW
				if( pAC->AsfData.ChipMode == SK_GEASF_CHIP_EX )  {							
				  AsfRunCpu( pAC, IoC );
				}
#endif


				if (( SpiRetVal == 0 ) && (!RetCode)) {
				/* calculate CS of the FW flash */
					SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    *** Flash high pages loaded. Calculate the CS of the FW flash.\n"));
					pTmp32 = (SK_U32 *) pAC->AsfData.FlashBuffer;
#ifdef USE_ASF_DASH_FW
					for( i=0; i< ASF_DASH_FLASH_SIZE_2; i+=4 )  {
#else
					for( i=0; i<ASF_FLASH_SIZE; i+=4 )  {
#endif					
						FwCs += *pTmp32;
						pTmp32++;
					}

					if( FwCs == 0  )  {  //  CS == 0 => O.K.
						FwFlashCsOk = 1;
						FwFlashCs = *(pTmp32 - 1);
						SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    FW Flash Checksum O.K. \n"));
					} else  {
						FwFlashCsOk = 0;
#ifdef USE_ASF_DASH_FW
						printk("sk98lin: Chip FW Checksum not OK\n");
#endif						
						SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    *** Error: FW Flash Checksum:0x%x\n", FwCs));
					}

#ifdef USE_ASF_DASH_FW
					/* read the FW flash version/rev */
					for( i=0; i<5; i++ ) {
						pAC->AsfData.FlashFwVersion[i] =  pAC->AsfData.FlashBuffer[ASF_FLASH_EX_OFFS_VER - 65536 + i];
					}

					pAC->AsfData.FlashFwRev = pAC->AsfData.FlashBuffer[ASF_FLASH_EX_OFFS_REV - 65536 ];

					/* read the GUID from flash */
					SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("   *** GUID ***\n"));
					SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    "));
					for( i=0; i<16; i++ ) {
						pAC->AsfData.Mib.Guid[i] =  pAC->AsfData.FlashBuffer[ASF_FLASH_EX_OFFS_GUID - 65536 +i];
						SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("%x ", pAC->AsfData.Mib.Guid[i]));
						OldGuid[i] = pAC->AsfData.FlashBuffer[ASF_FLASH_EX_OFFS_GUID - 65536 +i];
					}
					SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("\n"));
					SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    FW Flash:>>%c%c%c%c %c<<\n",
						pAC->AsfData.FlashFwVersion[1], pAC->AsfData.FlashFwVersion[2],
						pAC->AsfData.FlashFwVersion[3], pAC->AsfData.FlashFwVersion[4],
						pAC->AsfData.FlashFwRev ));
#else
					/* read the FW flash version/rev */
					for( i=0; i<5; i++ ) {
						pAC->AsfData.FlashFwVersion[i] =  pAC->AsfData.FlashBuffer[ASF_FLASH_OFFS_VER - 65536 + i];
					}

					pAC->AsfData.FlashFwRev = pAC->AsfData.FlashBuffer[ASF_FLASH_OFFS_REV - 65536 ];

					/* read the GUID from flash */
					SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("   *** GUID ***\n"));
					SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    "));
					for( i=0; i<16; i++ ) {
						pAC->AsfData.Mib.Guid[i] =  pAC->AsfData.FlashBuffer[ASF_FLASH_OFFS_GUID - 65536 +i];
						SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("%x ", pAC->AsfData.Mib.Guid[i]));
						OldGuid[i] = pAC->AsfData.FlashBuffer[ASF_FLASH_OFFS_GUID - 65536 +i];
					}
					SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("\n"));
					SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    FW Flash:>>%c%c%c%c %c<<\n",
						pAC->AsfData.FlashFwVersion[1], pAC->AsfData.FlashFwVersion[2],
 						pAC->AsfData.FlashFwVersion[3], pAC->AsfData.FlashFwVersion[4],
						pAC->AsfData.FlashFwRev ));
#endif


#ifdef USE_ASF_DASH_FW
#ifdef FORCE_FW_FLASH
					// Flash though Flash Version already the Chip and Flash Version of Driver are the same
					pAC->AsfData.FileFwRev = 'x';
#endif
#endif

					/* check the FW version/rev and update the flash if necessary */
					if( (pAC->AsfData.FlashFwVersion[1] != pAC->AsfData.DriverVersion[1]) ||
						(pAC->AsfData.FlashFwVersion[3] != pAC->AsfData.DriverVersion[3]) ||
						(pAC->AsfData.FlashFwVersion[4] != pAC->AsfData.DriverVersion[4]) ||
						(pAC->AsfData.FlashFwRev != pAC->AsfData.FileFwRev)               ||
						(pAC->AsfData.FileFwRev == 'x')                                   ||   // Rev == 'x' means: do allways a flash update ! (for test purposes)
						(FwFlashCsOk != 1))                                                   // Checksum error in flash
					{
						SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    Updating flash\n"));
						// AsfResetCpu( pAC, IoC );
						for( i=0; i<ASF_FLASH_SIZE; i++ )
							pAC->AsfData.FlashBuffer[i] = *(pAsfFwS1 + FlashOffset + i);

						/* flash erase, determine flash size and select area to be erased */
						switch (FlashSize) {
							case ASF_FLASH_SIZE * 4:	/* 256 kB */
#ifdef USE_ASF_DASH_FW
							       FlashOffset = ASF_FLASH_EX_OFFS;
 								EraseOff = ASF_FLASH_EX_OFFS;
#else
								EraseOff = ASF_FLASH_OFFS;
#endif

								break;
							case ASF_FLASH_SIZE * 2:	/* 128 kB */
#ifdef USE_ASF_DASH_FW
							        FlashOffset = 0;
#endif
								EraseOff = 0;
								break;
							default:			/* unsupported */
								SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    Unsupported Flash Size: %lu\n", FlashSize ));
								RetCode = SK_ASF_PNMI_ERR_GENERAL;
#ifdef USE_ASF_DASH_FW
								printk("sk98lin: Flash Size not supported\n");
#endif
								break;
						}
						if (!RetCode)
							DoUpdate = SK_TRUE;
					} else  {
						SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    Flash is up to date\n"));
#ifdef USE_ASF_DASH_FW
						printk("sk98lin: Flash is up to date\n");
#endif
						
					}



					if (DoUpdate) {
					        /* This hangs the CPU in case we do NOT flash */
					        /* So we defer it to this point where we are */
  					        /* sure we will flash. */
#ifdef USE_ASF_DASH_FW
					        AsfSmartResetCpu( pAC, IoC, ASF_RESET_COLD );
#endif

#ifdef USE_ASF_DASH_FW
					        printk("sk98lin: Starting the flash process\n");
#endif
							
						if (spi_flash_erase( EraseOff, ASF_FLASH_SIZE * 2) == 0 ) {
							/*
							 * Handle sector 1
							 * (first flash file)
							 */
							/*  write sector 1 buffer to flash and check the buffer */
#ifdef USE_ASF_DASH_FW
							if (spi_flash_manage( pAC->AsfData.FlashBuffer, FlashOffset, ASF_DASH_FLASH_SIZE_1, SPI_WRITE ) == 0 ) {
								/*  read buffer back */
								if( spi_flash_manage( pAC->AsfData.FlashBuffer, FlashOffset, ASF_DASH_FLASH_SIZE_1, SPI_READ ) == 0 )  {
								  /*  compare buffer with content of flash image file  */
								  for( i=0,FlashOk=1; i<ASF_DASH_FLASH_SIZE_1; i++ )  {
								    if( pAC->AsfData.FlashBuffer[i] != *(pAsfFwS1 + i) ) {
								      FlashOk = 0;
								    }
								  }
#else
							if (spi_flash_manage( pAC->AsfData.FlashBuffer, ASF_FLASH_OFFS, ASF_FLASH_SIZE, SPI_WRITE ) == 0 ) {
								/*  read buffer back */
								if( spi_flash_manage( pAC->AsfData.FlashBuffer, ASF_FLASH_OFFS, ASF_FLASH_SIZE, SPI_READ ) == 0 )  {
									/*  compare buffer with content of flash image file  */
									for( i=0,FlashOk=1; i<ASF_FLASH_SIZE; i++ )  {
										if( pAC->AsfData.FlashBuffer[i] != *(pAsfFwS1 + FlashOffset + i) )
											FlashOk = 0;
									}
#endif
									

									if( FlashOk  )  {
									/* read the GUID from flash */
#ifdef USE_ASF_DASH_FW
									  printk("sk98lin: Flash Part 1 succeeded\n");
#endif
									
									   SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    Flash successfully updated (Sector1)\n"));
									} else  {
#ifdef USE_ASF_DASH_FW
									  printk("sk98lin: Flash Part 1 did not succeed\n");
#endif									
										RetCode = SK_ASF_PNMI_ERR_GENERAL;
										SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    *** Error: compare flash content (Sector1)\n"));
										SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_ASF_E007, SKERR_ASF_E007MSG);
									}
								} else  {
									RetCode = SK_ASF_PNMI_ERR_GENERAL;
									SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    *** Error: Flash reread (Sector1)\n"));
									SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_ASF_E006, SKERR_ASF_E006MSG);
								}
							} else  {
								RetCode = SK_ASF_PNMI_ERR_GENERAL;
								SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    *** Error: Flash write (Sector1)\n"));
								SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_ASF_E004, SKERR_ASF_E004MSG);
							}


							/*
							 * Handle sector 2
							 * (second flash file)
							 */
							FlashOk = 1;
							for( i=0; i<ASF_FLASH_SIZE; i++ )
#ifdef USE_ASF_DASH_FW
								pAC->AsfData.FlashBuffer[i] = *(pAsfFwS2 + i);
#else
								pAC->AsfData.FlashBuffer[i] = *(pAsfFwS2 + FlashOffset + i);
#endif

#ifdef USE_ASF_DASH_FW
							/*  write sector 2 buffer to flash and check the buffer */
							if (spi_flash_manage( pAC->AsfData.FlashBuffer, FlashOffset + ASF_DASH_FLASH_SIZE_1, ASF_DASH_FLASH_SIZE_2, SPI_WRITE ) == 0 ) {
								/*  read buffer back */
								if( spi_flash_manage( pAC->AsfData.FlashBuffer, FlashOffset + ASF_DASH_FLASH_SIZE_1, ASF_DASH_FLASH_SIZE_2, SPI_READ ) == 0 )  {
									/*  compare buffer with content of flash image file  */
									for( i=0,FlashOk=1; i<ASF_DASH_FLASH_SIZE_2; i++ )  {
										if( pAC->AsfData.FlashBuffer[i] != *(pAsfFwS2 + i) )
											FlashOk = 0;
									}
#else
							/*  write sector 2 buffer to flash and check the buffer */
							if (spi_flash_manage( pAC->AsfData.FlashBuffer, ASF_FLASH_OFFS + 65536, ASF_FLASH_SIZE, SPI_WRITE ) == 0 ) {
								/*  read buffer back */
								if( spi_flash_manage( pAC->AsfData.FlashBuffer, ASF_FLASH_OFFS + 65536, ASF_FLASH_SIZE, SPI_READ ) == 0 )  {
									/*  compare buffer with content of flash image file  */
									for( i=0,FlashOk=1; i<ASF_FLASH_SIZE; i++ )  {
										if( pAC->AsfData.FlashBuffer[i] != *(pAsfFwS2 +FlashOffset + i) )
											FlashOk = 0;
									}
#endif

									if( FlashOk  )  {
#ifdef USE_ASF_DASH_FW
									  printk("sk98lin: Flash Part 2 succeeded\n");
#endif

									/* read the GUID from flash */
										for( i=0; i<16; i++ )
											pAC->AsfData.Mib.Guid[i] =  pAC->AsfData.FlashBuffer[ASF_FLASH_OFFS_GUID - 65536 +i];

										/* check if new GUID */
										for( i=0; i<16; i++) {
											if(OldGuid[i] != pAC->AsfData.Mib.Guid[i]) {
												SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,(" *** NEW GUID (%x)***\n", pAC->AsfData.Mib.Guid[i]));
												pAC->AsfData.NewGuid = 1;
												break;
											}
										}
										SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    Flash successfully updated (Sector2)\n"));
									} else  {
#ifdef USE_ASF_DASH_FW
									  printk("sk98lin: Flash Part 2 did not succeed\n");
#endif
									
										RetCode = SK_ASF_PNMI_ERR_GENERAL;
										SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    *** Error: compare flash content (Sector2)\n"));
										SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_ASF_E007, SKERR_ASF_E007MSG);
									}
								} else  {
									RetCode = SK_ASF_PNMI_ERR_GENERAL;
									SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    *** Error: Flash reread (Sector2)\n"));
									SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_ASF_E006, SKERR_ASF_E006MSG);
								}
							} else  {
								RetCode = SK_ASF_PNMI_ERR_GENERAL;
								SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    *** Error: Flash write (Sector2)\n"));
								SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_ASF_E004, SKERR_ASF_E004MSG);
							}
						} else  {
							RetCode = SK_ASF_PNMI_ERR_GENERAL;
							SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    *** Error: Flash erase\n"));
							SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_ASF_E003, SKERR_ASF_E003MSG);
						}
						/* write pattern etc. */
						if (pAC->AsfData.OpMode == SK_GEASF_MODE_IPMI) {
							/* ipmi on yukon2 */
							AsfSetUpPattern(pAC, IoC, 0);
							if (pAC->AsfData.DualMode == SK_GEASF_Y2_DUALPORT) {
								AsfSetUpPattern(pAC, IoC, 1);
							}
						}

						/* run cpu */
						AsfRunCpu( pAC, IoC );
					} /* if DoUpdate */
				} else {
					SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    *** Error: SPI read\n"));
					SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_ASF_E002, SKERR_ASF_E002MSG);
					RetCode = SK_ASF_PNMI_ERR_GENERAL;
				}
			}

			/* Clear the buffer */
			if (pAsfFwS1 != NULL)
				kfree(pAsfFwS1);
			if (pAsfFwS2 != NULL)
				kfree(pAsfFwS2);
		}

		break;

	case SK_INIT_RUN:
		SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfInit: SK_INIT_RUN\n"));

		if( pAC->AsfData.InitState != ASF_INIT_UNDEFINED ) {

#ifdef USE_ASF_DASH_FW
		  /* We need to reenable the OS here and the ARPs done by the HOST */
		  /* as we will leave this function directly with quick break*/

		  /*  Set OS Present Flag in ASF Status and Command Register */
		  AsfSetOsPresentBit( pAC, IoC );

		  /*  Disable ARP pattern, host system takes over the ARP handling */
		  YlciDisablePattern(pAC, IoC, 0, ASF_DASH_PATTERN_NUM_ARP );  //  Disable ARP pattern, OS is now responsible for ARP handling
		  YlciDisablePattern(pAC, IoC, 0, ASF_DASH_PATTERN_NUM_ICMP);  //  Disable ICMP pattern, OS is now responsible for ICMP handling
		  YlciDisablePattern(pAC, IoC, 0, ASF_DASH_PATTERN_NUM_SNMP );  //  Disable ARP pattern, OS is now responsible for SNMP handling

		  AsfDisableFlushFifo( pAC, IoC );

		  AsfHciSendCommand( pAC, IoC, YASF_HOSTCMD_DRV_HELLO, 0, 0, 0, ASF_HCI_WAIT, 3 );
#endif
		  
		  break;
		}
		
#ifndef USE_ASF_DASH_FW
		YlciDisablePattern(pAC, IoC, 0, 5);  //  Disable ARP pattern, OS is now responsible for ARP handling

		if (pAC->AsfData.DualMode == SK_GEASF_Y2_DUALPORT) {
			YlciDisablePattern(pAC, IoC, 1, 5);  //  Disable ARP pattern, OS is now responsible for ARP handling
		}
#endif

		/* write pattern etc. */
		if (pAC->AsfData.OpMode == SK_GEASF_MODE_IPMI) {
		// ipmi on yukon2
			AsfSetUpPattern(pAC, IoC, 0);
			if (pAC->AsfData.DualMode == SK_GEASF_Y2_DUALPORT) {
				AsfSetUpPattern(pAC, IoC, 1);
			}
		}

		if( !pAC->AsfData.CpuAlive )  {
		  AsfResetCpu(pAC, IoC);
		  AsfRunCpu( pAC, IoC );
		}

		/*  ASF MIB parameter to default values */
		pAC->AsfData.Mib.WdTimeMax          = ASF_DEF_WATCHDOG_TIME_MAX;
		pAC->AsfData.Mib.WdTimeMin          = ASF_DEF_WATCHDOG_TIME_MIN;
		pAC->AsfData.Mib.RetransCountMin    = ASF_DEF_RETRANS_COUNT_MIN;
		pAC->AsfData.Mib.RetransCountMax    = ASF_DEF_RETRANS_COUNT_MAX;
		pAC->AsfData.Mib.RetransIntMin      = ASF_DEF_RETRANS_INT_MIN;
		pAC->AsfData.Mib.RetransIntMax      = ASF_DEF_RETRANS_INT_MAX;
		pAC->AsfData.Mib.HbIntMin           = ASF_DEF_HB_INT_MIN;
		pAC->AsfData.Mib.HbIntMax           = ASF_DEF_HB_INT_MAX;
		pAC->AsfData.Mib.Ena                = ASF_DEF_ASF_ENA;
		pAC->AsfData.Mib.RspEnable          = 0;
		pAC->AsfData.Mib.Retrans            = ASF_DEF_RETRANS;
		pAC->AsfData.Mib.RetransInt         = ASF_DEF_RETRANS_INT;
		pAC->AsfData.Mib.HbEna              = ASF_DEF_HB_ENA;
		pAC->AsfData.Mib.HbInt              = ASF_DEF_HB_INT;
		pAC->AsfData.Mib.WdEna              = ASF_DEF_WATCHDOG_ENA;
		pAC->AsfData.Mib.WdTime             = ASF_DEF_WATCHDOG_TIME;
		pAC->AsfData.Mib.CommunityName[0]   = 0x00;
		pAC->AsfData.Mib.RlmtMode           = 0xff;     // only for IPMI op mode
		pAC->AsfData.Mib.PattUpReq          = 0;        // update pattern request flag

		/* Start ASF timer */
		SK_MEMSET((char *)&EventParam, 0, sizeof(EventParam));
		SkTimerStart(pAC, IoC, &pAC->AsfData.AsfTimer,
					  5000000, SKGE_ASF, SK_ASF_EVT_TIMER_EXPIRED,
					  EventParam);
		//  initialize the FW WD functionality
		pAC->AsfData.FwWdIntervall = 120;
		if( pAC->AsfData.ChipMode == SK_GEASF_CHIP_EX )  {
		  pAC->AsfData.FwRamSize = 0;
		} else {

   		   if( RetCode == SK_ASF_PNMI_ERR_OK )  {
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    *** Check FW Ramsize\n"));
			lRetCode = AsfHciSendCommand(pAC, IoC, YASF_HOSTCMD_CFG_GET_ASF_RAMSIZE, 0, 0, 1, ASF_HCI_WAIT, 2 );

			if( lRetCode == HCI_EN_CMD_READY )  {
			// Fetch received data from HCI interface
				AsfHciGetData( pAC, &pHciRecBuf );
				pAC->AsfData.FwRamSize  = ((SK_U16)*(pHciRecBuf+2)) << 8;
				pAC->AsfData.FwRamSize |= ((SK_U16)*(pHciRecBuf+3)) << 0;

				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    *** FW RamSize: %dkB \n", pAC->AsfData.FwRamSize));
			}  //  if( lRetCode == HCI_EN_CMD_READY )
			if( lRetCode == HCI_EN_CMD_ERROR )  {
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    *** error\n"));
				RetCode = SK_ASF_PNMI_ERR_GENERAL;
			}
		   }
		}

		if( RetCode == SK_ASF_PNMI_ERR_OK )  {
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    *** ASF Init O.K. *** (%d)\n", pAC->GIni.GIRamSize));
			pAC->AsfData.InitState = ASF_INIT_OK;
			pAC->GIni.GIRamSize -= pAC->AsfData.FwRamSize;  //  shorten RAM buffer by FwRamSize
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    *** New RAM value (%dk)\n", pAC->GIni.GIRamSize));

#ifdef CHIP_ID_YUKON_EX				
		  switch(pAC->AsfData.OpMode)  {
		  case SK_GEASF_MODE_DASH:
		    pAC->GIni.GINumOfPattern -= 6;
		    break;
		  default:
		    pAC->GIni.GINumOfPattern -= 3;
		    break;
		  }
		  SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT,("    *** New # of pattern (%d)\n", pAC->GIni.GINumOfPattern));
#endif // CHIP_ID_YUKON_EX	
			
		} else  {
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    *** ASF Init failed ***\n"));

			AsfSmartResetCpu( pAC, IoC, ASF_RESET_HOT );
			pAC->AsfData.InitState = ASF_INIT_ERROR;  //  disable ASF functionality
			// after reset the cpu clean up some important registers
			AsfSetSMBusRegister(IoC);
		}

		/* fetch the home MAC address from adapter */
		for (i = 0; i < 6; i++) {
			SK_IN8(IoC, (B2_MAC_1 + i), &pAC->AsfData.Mib.MacSource[i] );
		}

#ifdef USE_ASF_DASH_FW
		printk("sk98lin: Using IP %u.%u.%u.%u for DASH FW\n", (unsigned int)pAC->IpAddr[0], (unsigned int)pAC->IpAddr[1], (unsigned int)pAC->IpAddr[2], (unsigned int)pAC->IpAddr[3]);
#endif
			
#ifdef USE_ASF_DASH_FW	
		/* Write patterns to pattern RAM on the Yukon board*/
		AsfSetUpPattern(pAC, IoC, 0);
#endif

#ifdef USE_ASF_DASH_FW
		/*  Set OS Present Flag in ASF Status and Command Register */
		AsfSetOsPresentBit( pAC, IoC );

		/*  Disable ARP pattern, host system takes over the ARP handling */
		YlciDisablePattern(pAC, IoC, 0, ASF_DASH_PATTERN_NUM_ARP );  //  Disable ARP pattern, OS is now responsible for ARP handling
		YlciDisablePattern(pAC, IoC, 0, ASF_DASH_PATTERN_NUM_ICMP);  //  Disable ICMP pattern, OS is now responsible for ICMP handling
		YlciDisablePattern(pAC, IoC, 0, ASF_DASH_PATTERN_NUM_SNMP);  //  Disable SNMP pattern, OS is now responsible for SNMP handling
#endif

#ifdef USE_ASF_DASH_FW
		if( AsfHciSendCommand( pAC, IoC, YASF_HOSTCMD_DRV_HELLO, 0, 0, 0, ASF_HCI_WAIT, 3 ) != HCI_EN_CMD_READY )  {
		  printk("sk98lin: Communication Driver <-> FW is failing\n");
		} else {
		  printk("sk98lin: Communication Driver <-> FW is working properly\n");
		}
#endif

		break;

	default:
		break; /* Nothing todo */
	}

	return( RetCode );
}

/*****************************************************************************
*
* AsfSetSMBusRegister - cleaning up register for SMBus
*
* Description:  If the ASF FW goes into smart reset, this function is
*               cleaning up the SMBus register.
*
* Returns:
*/

void AsfSetSMBusRegister(
    SK_IOC IoC)    /* IO context handle */
{

    SK_U32 TmpVal32;
    SK_U32 mask;

    // get register
    SK_IN32(IoC, REG_ASF_SMBUS_CFG, &TmpVal32);

    // delete bit 4: SMBC_IE
    // delete bit 5: SMBC_EA
    // delete bit 6: SMBC_GCE
    // delete bit 7: SMBC_DAE
    // delete bit 8: SMBC_SAE

    mask      = 0x000001f0;
    TmpVal32 &= (~mask);

    // set register
    SK_OUT32(IoC, REG_ASF_SMBUS_CFG, TmpVal32);

    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    *** ASF cleaning up SMBusRegister 0x%x with 0x%x ***\n", REG_ASF_SMBUS_CFG, TmpVal32));
}


/*****************************************************************************
*
* SkAsfDeInit - DeInit function of ASF
*
* Description:
*
* Returns:
*   Always 0
*/
int SkAsfDeInit(
SK_AC *pAC,         /* Pointer to adapter context */
SK_IOC IoC )  {     /* IO context handle */
#ifndef USE_ASF_DASH_FW
SK_U32  TmpVal32;
#endif

#ifdef USE_ASF_DASH_FW
    /*  Reset OS Present Flag in ASF Status and Command Register */
    AsfResetOsPresentBit( pAC, IoC );
#else
    /*  Reset OS Present Flag in ASF Status and Command Register */
    SK_IN32( IoC, REG_ASF_STATUS_CMD, &TmpVal32 );
    TmpVal32 &= ~BIT_4;
    SK_OUT32( IoC, REG_ASF_STATUS_CMD, TmpVal32 );
#endif

    if( pAC->AsfData.InitState == ASF_INIT_OK )  {
#ifdef USE_ASF_DASH_FW
      switch(pAC->AsfData.OpMode)  {
      case SK_GEASF_MODE_ASF:
	//  Enable ARP pattern, ASF FW is now responsible for ARP handling
	YlciEnablePattern ( pAC, IoC, 0, ASF_PATTERN_ID_ARP );
	break;
      case SK_GEASF_MODE_IPMI:
	//  Enable ARP pattern, ASF FW is now responsible for ARP handling
	YlciEnablePattern ( pAC, IoC, 0, ASF_PATTERN_ID_ARP );
	
	if (pAC->AsfData.DualMode == SK_GEASF_Y2_DUALPORT) {
	  YlciEnablePattern ( pAC, IoC, 1, ASF_PATTERN_ID_ARP );
	}
	break;
      case SK_GEASF_MODE_DASH:
	//  Enable ARP pattern, ASF FW is now responsible for ARP handling
	YlciEnablePattern( pAC, IoC, 0, ASF_DASH_PATTERN_NUM_ARP );
	YlciEnablePattern( pAC, IoC, 0, ASF_DASH_PATTERN_NUM_ICMP );
	YlciEnablePattern( pAC, IoC, 0, ASF_DASH_PATTERN_NUM_SNMP );		
	break;
      }  //  switch(pAC->AsfData.OpMode) 
      
      //  Inform the FW that the driver will be unloaded
      AsfHciSendCommand( pAC ,IoC , YASF_HOSTCMD_DRV_GOODBYE, 0, 0, 0, ASF_HCI_WAIT, 2 );
      
      //  will be done in FW now		
      // if( pAC->AsfData.ChipMode == SK_GEASF_CHIP_EX )
      // AsfEnableFlushFifo( pAC, IoC );
#else
      if (pAC->AsfData.OpMode == SK_GEASF_MODE_ASF) {
	//  Enable ARP pattern, ASF FW is now responsible for ARP handling
	YlciEnablePattern ( pAC, IoC, 0, 5 );
      }
      
      if (pAC->AsfData.OpMode == SK_GEASF_MODE_IPMI) {
	//  Enable ARP pattern, ASF FW is now responsible for ARP handling
	YlciEnablePattern ( pAC, IoC, 0, 5 );
	
	if (pAC->AsfData.DualMode == SK_GEASF_Y2_DUALPORT) {
	  YlciEnablePattern ( pAC, IoC, 1, 5 );
	}
      }
      
      //  Inform the FW that the driver will be unloaded
      AsfHciSendCommand( pAC, IoC, YASF_HOSTCMD_DRV_GOODBYE, 0, 0, 0, ASF_HCI_WAIT, 2 );
#endif
    }

    return( 0 );
}

/*****************************************************************************
*
* SkAsfDeInitStandBy - StandBy -DeInit function of ASF
*
* Description:
*
* Returns:
*   Always 0
*/
int SkAsfDeInitStandBy(
SK_AC *pAC,         /* Pointer to adapter context */
SK_IOC IoC )  {     /* IO context handle */

#ifdef USE_ASF_DASH_FW
    if( pAC->AsfData.InitState == ASF_INIT_OK )  {	
      //  will be done in FW now
      if( pAC->AsfData.ChipMode == SK_GEASF_CHIP_EX )
	AsfEnableFlushFifo( pAC, IoC );
      
      switch(pAC->AsfData.OpMode)  {
      case SK_GEASF_MODE_ASF:
      case SK_GEASF_MODE_IPMI:
	//  Enable ARP pattern, ASF FW is now responsible for ARP handling
	YlciEnablePattern ( pAC, IoC, pAC->AsfData.ActivePort, ASF_PATTERN_ID_ARP );
	break;
      case SK_GEASF_MODE_DASH:
	//  Enable ARP pattern, ASF FW is now responsible for ARP handling
	YlciEnablePattern( pAC, IoC, 0, ASF_DASH_PATTERN_NUM_ARP );
	YlciEnablePattern( pAC, IoC, 0, ASF_DASH_PATTERN_NUM_ICMP );		
	YlciEnablePattern( pAC, IoC, 0, ASF_DASH_PATTERN_NUM_SNMP );
	break;
      }  //  switch(pAC->AsfData.OpMode) 
      
      //  Inform the FW that the driver will be unloaded
      AsfHciSendCommand( pAC ,IoC , YASF_HOSTCMD_DRV_STANDBY, 0, 0, 0, ASF_HCI_WAIT, 2 );
    }
#else
    if( pAC->AsfData.InitState == ASF_INIT_OK )  {
        //  Enable ARP pattern, ASF FW is now responsible for ARP handling
        YlciEnablePattern ( pAC, IoC, pAC->AsfData.ActivePort, 5 );
        //  Inform the FW that the driver will be unloaded
        AsfHciSendCommand( pAC, IoC, YASF_HOSTCMD_DRV_STANDBY, 0, 0, 0, ASF_HCI_WAIT, 2 );
    }
#endif

    return( 0 );
}

/*****************************************************************************
*
* SkAsfInitStandBy - StandBy - Init function of ASF
*
* Description:
*
* Returns:
*   Always 0
*/
int SkAsfInitStandBy(
SK_AC   *pAC,       /* Pointer to adapter context */
SK_IOC  IoC,        /* IO context handle */
int     Level) {    /* Initialization level */
#ifndef USE_ASF_DASH_FW
    SK_U32          TmpVal32;
#endif
    SK_EVPARA       EventParam; /* Event struct for timer event */
	
#ifdef USE_ASF_DASH_FW
    if( pAC->AsfData.InitState == ASF_INIT_OK )  {
        switch(Level)
        {
            case SK_INIT_DATA:
                /*  Set OS Present Flag in ASF Status and Command Register */
	        AsfSetOsPresentBit( pAC, IoC );

                //  Disable ARP pattern, host system takes over the ARP handling
                YlciDisablePattern( pAC, IoC,pAC->AsfData.ActivePort, ASF_PATTERN_ID_ARP);
                //  Inform the FW that the driver will be activated again
                AsfHciSendCommand( pAC ,IoC , YASF_HOSTCMD_DRV_HELLO, 0, 0, 0, ASF_HCI_WAIT, 2 );

                break;

            case SK_INIT_IO:
                break;

            case SK_INIT_RUN:
                SK_MEMSET((char *)&EventParam, 0, sizeof(EventParam));
                SkTimerStart(pAC, IoC, &pAC->AsfData.AsfTimer,
                            5000000, SKGE_ASF, SK_ASF_EVT_TIMER_EXPIRED,  EventParam);
                break;
        }  //  switch( Level )
    }  //  if( pAC->AsfData.InitState == ASF_INIT_OK 
#else
    if( pAC->AsfData.InitState == ASF_INIT_OK )  {
        switch(Level)
        {
            case SK_INIT_DATA:
                /*  Set OS Present Flag in ASF Status and Command Register */
                SK_IN32( IoC, REG_ASF_STATUS_CMD, &TmpVal32 );
                TmpVal32 |= BIT_4;
                //  Disable ARP pattern, host system takes over the ARP handling
                YlciDisablePattern ( pAC, IoC, pAC->AsfData.ActivePort, 5 );

                // Inform the FW that the driver will be activated again
		// Schlechte Idee, schaltet ASF beim laden des Treibers ab
                // AsfHciSendCommand( pAC, IoC, YASF_HOSTCMD_DRV_HELLO, 0, 0, 0, ASF_HCI_WAIT, 2 );
                break;

            case SK_INIT_IO:
                break;

            case SK_INIT_RUN:
                SK_MEMSET((char *)&EventParam, 0, sizeof(EventParam));
                SkTimerStart(pAC, IoC, &pAC->AsfData.AsfTimer,
                            5000000, SKGE_ASF, SK_ASF_EVT_TIMER_EXPIRED,  EventParam);
                break;
        }  //  switch( Level )
    }  //  if( pAC->AsfData.InitState == ASF_INIT_OK )
#endif
    return( 0 );
}



/******************************************************************************
*
*   SkAsfSeprom2Mib     reads ASF MIB config data from SEPROM
*
*   Context:
*
*
*   Returns:    0:  successful
*               1:  error
*/

SK_I8 SkAsfSeprom2Mib(
SK_AC *pAC,      /* Pointer to adapter context */
SK_IOC IoC ) {   /* IO context handle */

    SK_U8   ind;
    SK_U8   i;
    SK_U8   version;
    SK_I8   RetCode;
    SK_U32  our_reg2;
    SK_U32  Len;

    ind = 0;

    VPD_IN32(pAC, IoC, PCI_OUR_REG_2, &our_reg2);
    pAC->vpd.rom_size = 256 << ((our_reg2 & PCI_VPD_ROM_SZ) >> 14);

    Len = VpdReadBlock( pAC, IoC, &pAC->AsfData.VpdConfigBuf[0], ASF_VPD_CONFIG_BASE, ASF_VPD_CONFIG_SIZE );
    if( Len == ASF_VPD_CONFIG_SIZE )  {
        RetCode = SK_ASF_PNMI_ERR_OK;
        /* check, whether the signature is valid */
        if( pAC->AsfData.VpdConfigBuf[0] == 'A' &&
            pAC->AsfData.VpdConfigBuf[1] == 'S' &&
            pAC->AsfData.VpdConfigBuf[2] == 'F' &&
            pAC->AsfData.VpdConfigBuf[3] == '!' ) {
            ind = 4;
            version = pAC->AsfData.VpdConfigBuf[ind++];
            switch( version )  {
                case 1:
                    pAC->AsfData.Mib.Ena =          pAC->AsfData.VpdConfigBuf[ind++];
                    pAC->AsfData.Mib.RspEnable =    pAC->AsfData.VpdConfigBuf[ind++];
                    pAC->AsfData.Mib.Retrans =      (SK_U16)  pAC->AsfData.VpdConfigBuf[ind++];
                    pAC->AsfData.Mib.Retrans |=     (SK_U16) (pAC->AsfData.VpdConfigBuf[ind++] << 8);
                    pAC->AsfData.Mib.RetransInt =   (SK_U32)  pAC->AsfData.VpdConfigBuf[ind++];
                    pAC->AsfData.Mib.RetransInt |=  (SK_U32) (pAC->AsfData.VpdConfigBuf[ind++] << 8);
                    pAC->AsfData.Mib.RetransInt |=  (SK_U32) (pAC->AsfData.VpdConfigBuf[ind++] << 16);
                    pAC->AsfData.Mib.RetransInt |=  (SK_U32) (pAC->AsfData.VpdConfigBuf[ind++] << 24);
                    pAC->AsfData.Mib.HbEna =        pAC->AsfData.VpdConfigBuf[ind++];
                    pAC->AsfData.Mib.HbInt =        (SK_U32)  pAC->AsfData.VpdConfigBuf[ind++];
                    pAC->AsfData.Mib.HbInt |=       (SK_U32) (pAC->AsfData.VpdConfigBuf[ind++] << 8);
                    pAC->AsfData.Mib.HbInt |=       (SK_U32) (pAC->AsfData.VpdConfigBuf[ind++] << 16);
                    pAC->AsfData.Mib.HbInt |=       (SK_U32) (pAC->AsfData.VpdConfigBuf[ind++] << 24);
                    pAC->AsfData.Mib.WdEna =        pAC->AsfData.VpdConfigBuf[ind++];
                    pAC->AsfData.Mib.WdTime =       (SK_U32)  pAC->AsfData.VpdConfigBuf[ind++];
                    pAC->AsfData.Mib.WdTime |=      (SK_U32) (pAC->AsfData.VpdConfigBuf[ind++] << 8);
                    pAC->AsfData.Mib.WdTime |=      (SK_U32) (pAC->AsfData.VpdConfigBuf[ind++] << 16);
                    pAC->AsfData.Mib.WdTime |=      (SK_U32) (pAC->AsfData.VpdConfigBuf[ind++] << 24);
                    for( i=0; i<4; i++ )
                        pAC->AsfData.Mib.IpDest[i] =        pAC->AsfData.VpdConfigBuf[ind++];
                    for( i=0; i<4; i++ )
                        pAC->AsfData.Mib.IpSource[i] =      pAC->AsfData.VpdConfigBuf[ind++];
                    for( i=0; i<6; i++ )
                        pAC->AsfData.Mib.MacDest[i] =       pAC->AsfData.VpdConfigBuf[ind++];
                    for( i=0; i<64; i++ )
                        pAC->AsfData.Mib.CommunityName[i] = pAC->AsfData.VpdConfigBuf[ind++];
                    for( i=0; i<6; i++ )
                        pAC->AsfData.Mib.Reserved[i] =      pAC->AsfData.VpdConfigBuf[ind++];
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("     O.K.\n"));
                    break;

                case 2:
                    pAC->AsfData.Mib.Ena =                      pAC->AsfData.VpdConfigBuf[ind++];
                    pAC->AsfData.Mib.RspEnable =                pAC->AsfData.VpdConfigBuf[ind++];
                    pAC->AsfData.Mib.Retrans =      (SK_U16)    pAC->AsfData.VpdConfigBuf[ind++];
                    pAC->AsfData.Mib.RetransInt =   (SK_U32)    pAC->AsfData.VpdConfigBuf[ind++] * 10;
                    pAC->AsfData.Mib.HbEna =                    pAC->AsfData.VpdConfigBuf[ind++];
                    pAC->AsfData.Mib.HbInt =        (SK_U32)    pAC->AsfData.VpdConfigBuf[ind++] * 10;
                    pAC->AsfData.Mib.WdEna =                    pAC->AsfData.VpdConfigBuf[ind++];
                    pAC->AsfData.Mib.WdTime =       (SK_U32)    pAC->AsfData.VpdConfigBuf[ind++] * 10;
                    for( i=0; i<4; i++ )
                        pAC->AsfData.Mib.IpDest[i] =        pAC->AsfData.VpdConfigBuf[ind++];
                    for( i=0; i<4; i++ )
                        pAC->AsfData.Mib.IpSource[i] =      pAC->AsfData.VpdConfigBuf[ind++];
                    for( i=0; i<6; i++ )
                        pAC->AsfData.Mib.MacDest[i] =       pAC->AsfData.VpdConfigBuf[ind++];
                    for( i=0; i<10; i++ )
                        pAC->AsfData.Mib.CommunityName[i] = pAC->AsfData.VpdConfigBuf[ind++];
                    for( i=0; i<6; i++ )
                        pAC->AsfData.Mib.Reserved[i] =      pAC->AsfData.VpdConfigBuf[ind++];
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("     O.K.\n"));
                    break;

                default:
                    // invalidate ASF signature, this causes a new formating of the SEEPROM
                    pAC->AsfData.VpdConfigBuf[0] = 'x';
                    pAC->AsfData.VpdConfigBuf[1] = 'x';
                    pAC->AsfData.VpdConfigBuf[2] = 'x';
                    pAC->AsfData.VpdConfigBuf[3] = 'x';
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("     unknown version: 0x%x\n", version ));
                    break;
            }  //  switch( version )  {
        }
        else  {
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("     *** Error: Signature not valid\n"));
        }
    }  //  if( Len == ..  )
    else  {
        RetCode = SK_ASF_PNMI_ERR_GENERAL;
        SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("     *** failed\n"));
    }

    return( RetCode );
}

/******************************************************************************
*
*   SkAsfMib2Seprom     writes ASF MIB config data to SEPROM
*
*   Context:
*
*
*   Returns:    0:  successful
*               1:  error
*/

SK_I8 SkAsfMib2Seprom(
SK_AC *pAC,      /* Pointer to adapter context */
SK_IOC IoC ) {   /* IO context handle */

    SK_U8   ind;
    SK_U8   i;
    SK_I8   RetCode;
    SK_U32  our_reg2;
    SK_U32  Len;
    SK_U8   version;

    version = 2;

    ind = 0;
    pAC->AsfData.VpdConfigBuf[ind++] = 'A';
    pAC->AsfData.VpdConfigBuf[ind++] = 'S';
    pAC->AsfData.VpdConfigBuf[ind++] = 'F';
    pAC->AsfData.VpdConfigBuf[ind++] = '!';
    pAC->AsfData.VpdConfigBuf[ind++] = version;
    switch( version )  {
        case 1:  //  101 Bytes
            pAC->AsfData.VpdConfigBuf[ind++] = pAC->AsfData.Mib.Ena;
            pAC->AsfData.VpdConfigBuf[ind++] = pAC->AsfData.Mib.RspEnable;
            pAC->AsfData.VpdConfigBuf[ind++] = (SK_U8) (pAC->AsfData.Mib.Retrans >> 0) & 0x00ff;
            pAC->AsfData.VpdConfigBuf[ind++] = (SK_U8) (pAC->AsfData.Mib.Retrans >> 8) & 0x00ff;
            pAC->AsfData.VpdConfigBuf[ind++] = (SK_U8) (pAC->AsfData.Mib.RetransInt >> 0)  & 0x000000ff;
            pAC->AsfData.VpdConfigBuf[ind++] = (SK_U8) (pAC->AsfData.Mib.RetransInt >> 8)  & 0x000000ff;
            pAC->AsfData.VpdConfigBuf[ind++] = (SK_U8) (pAC->AsfData.Mib.RetransInt >> 16) & 0x000000ff;
            pAC->AsfData.VpdConfigBuf[ind++] = (SK_U8) (pAC->AsfData.Mib.RetransInt >> 24) & 0x000000ff;
            pAC->AsfData.VpdConfigBuf[ind++] = pAC->AsfData.Mib.HbEna;
            pAC->AsfData.VpdConfigBuf[ind++] = (SK_U8) (pAC->AsfData.Mib.HbInt >> 0)  & 0x000000ff;
            pAC->AsfData.VpdConfigBuf[ind++] = (SK_U8) (pAC->AsfData.Mib.HbInt >> 8)  & 0x000000ff;
            pAC->AsfData.VpdConfigBuf[ind++] = (SK_U8) (pAC->AsfData.Mib.HbInt >> 16) & 0x000000ff;
            pAC->AsfData.VpdConfigBuf[ind++] = (SK_U8) (pAC->AsfData.Mib.HbInt >> 24) & 0x000000ff;
            pAC->AsfData.VpdConfigBuf[ind++] = pAC->AsfData.Mib.WdEna;
            pAC->AsfData.VpdConfigBuf[ind++] = (SK_U8) (pAC->AsfData.Mib.WdTime >> 0)  & 0x000000ff;
            pAC->AsfData.VpdConfigBuf[ind++] = (SK_U8) (pAC->AsfData.Mib.WdTime >> 8)  & 0x000000ff;
            pAC->AsfData.VpdConfigBuf[ind++] = (SK_U8) (pAC->AsfData.Mib.WdTime >> 16) & 0x000000ff;
            pAC->AsfData.VpdConfigBuf[ind++] = (SK_U8) (pAC->AsfData.Mib.WdTime >> 24) & 0x000000ff;
            for( i=0; i<4; i++ )
                pAC->AsfData.VpdConfigBuf[ind++] = pAC->AsfData.Mib.IpDest[i];
            for( i=0; i<4; i++ )
                pAC->AsfData.VpdConfigBuf[ind++] = pAC->AsfData.Mib.IpSource[i];
            for( i=0; i<6; i++ )
                pAC->AsfData.VpdConfigBuf[ind++] = pAC->AsfData.Mib.MacDest[i];
            for( i=0; i<64; i++ )
                pAC->AsfData.VpdConfigBuf[ind++] = pAC->AsfData.Mib.CommunityName[i];
            for( i=0; i<6; i++ )
                pAC->AsfData.VpdConfigBuf[ind++] = pAC->AsfData.Mib.Reserved[i];
            break;

        case 2:  //  40 Bytes
            pAC->AsfData.VpdConfigBuf[ind++] = pAC->AsfData.Mib.Ena;
            pAC->AsfData.VpdConfigBuf[ind++] = pAC->AsfData.Mib.RspEnable;
            pAC->AsfData.VpdConfigBuf[ind++] = (SK_U8) pAC->AsfData.Mib.Retrans & 0x00ff;
            pAC->AsfData.VpdConfigBuf[ind++] = (SK_U8) (pAC->AsfData.Mib.RetransInt / 10)  & 0x000000ff;
            pAC->AsfData.VpdConfigBuf[ind++] = pAC->AsfData.Mib.HbEna;
            pAC->AsfData.VpdConfigBuf[ind++] = (SK_U8) (pAC->AsfData.Mib.HbInt / 10)  & 0x000000ff;
            pAC->AsfData.VpdConfigBuf[ind++] = pAC->AsfData.Mib.WdEna;
            pAC->AsfData.VpdConfigBuf[ind++] = (SK_U8) (pAC->AsfData.Mib.WdTime / 10)  & 0x000000ff;
            for( i=0; i<4; i++ )
                pAC->AsfData.VpdConfigBuf[ind++] = pAC->AsfData.Mib.IpDest[i];
            for( i=0; i<4; i++ )
                pAC->AsfData.VpdConfigBuf[ind++] = pAC->AsfData.Mib.IpSource[i];
            for( i=0; i<6; i++ )
                pAC->AsfData.VpdConfigBuf[ind++] = pAC->AsfData.Mib.MacDest[i];
            for( i=0; i<10; i++ )
                pAC->AsfData.VpdConfigBuf[ind++] = pAC->AsfData.Mib.CommunityName[i];
            for( i=0; i<6; i++ )
                pAC->AsfData.VpdConfigBuf[ind++] = pAC->AsfData.Mib.Reserved[i];
            break;

        default:
            break;
    }  //  switch( version )  {



    VPD_IN32(pAC, IoC, PCI_OUR_REG_2, &our_reg2);
    pAC->vpd.rom_size = 256 << ((our_reg2 & PCI_VPD_ROM_SZ) >> 14);

    // enable Config write Reg 0x158
    // SK_IN8( IoC, B2_TST_REG1, &TmpVal8 )
    // TmpVal8 &= ~0x03;
    // TmpVal8 |= 0x02;
    // SK_OUT8( IoC, B2_TST_REG1, TmpVal8 )
    SK_OUT8(IoC, B2_TST_CTRL1, TST_CFG_WRITE_ON);

    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfMib2Seprom \n"));
    Len = VpdWriteBlock(pAC,IoC, &pAC->AsfData.VpdConfigBuf[0], ASF_VPD_CONFIG_BASE, ASF_VPD_CONFIG_SIZE );
    if( Len == ASF_VPD_CONFIG_SIZE )  {
        RetCode = SK_ASF_PNMI_ERR_OK;
        SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    O.K. \n"));
    }
    else  {
        RetCode = SK_ASF_PNMI_ERR_GENERAL;
        SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    *** failed \n"));
    }

    // disable Config write Reg 0x158
    // SK_IN8( IoC, B2_TST_REG1, &TmpVal8 )
    // TmpVal8 &= ~0x03;
    // TmpVal8 |= 0x01;
    // SK_OUT8( IoC, B2_TST_REG1, TmpVal8 )
    SK_OUT8(IoC, B2_TST_CTRL1, TST_CFG_WRITE_OFF);

    return( RetCode );
}


/******************************************************************************
*
*   SkAsfTriggerPetFrames
*
*   Context:
*   init, pageable
*
*   Returns: void
*/

void SkAsfTriggerPetFrames (SK_IOC IoC, SK_U32 *TmpVal32)
{

    return;
}

/******************************************************************************
*
*   SkAsfPreSetOid -
*
* Context:
*   init, pageable
*
* Returns:
*/
int SkAsfPreSetOid(
            SK_AC *pAC, /* the adapter context */
            SK_IOC IoC, /* I/O context */
            SK_U32 Id,  /* OID  */
            SK_U32 Inst,
            SK_U8 *pBuf,
            unsigned int *pLen)
{
    SK_U32 RetCode = SK_ASF_PNMI_ERR_OK;


    switch( Id )  {
        case OID_SKGE_ASF_ENA:
            break;

        default:
            *pLen = 0;
            RetCode = SK_ASF_PNMI_ERR_NOT_SUPPORTED;
            break;
    }

    return(RetCode);
}

/******************************************************************************
*
*   SkAsfSetOid -
*
* Context:
*   init, pageable
*
* Returns:
*/
int SkAsfSetOid(
            SK_AC *pAC, /* the adapter context */
            SK_IOC IoC, /* I/O context */
            SK_U32 Id,  /* OID  */
            SK_U32 Inst,
            SK_U8 *pBuf,
            unsigned int *pLen)
{
    SK_U16          i;

    SK_U32  RetCode = SK_ASF_PNMI_ERR_OK;

    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfSetOid -> OID:0x%x Len:%d ***********************\n", Id, *pLen ));

    if( pAC->AsfData.InitState != ASF_INIT_OK )  {
        return( SK_ASF_PNMI_ERR_GENERAL );
    }

    switch( Id )  {
        case OID_SKGE_ASF_STORE_CONFIG:
            pAC->AsfData.Mib.NewParam = 4;
            break;

        case OID_SKGE_ASF_ENA:
            if( *pLen == 1 )  {
                if( *pBuf != pAC->AsfData.Mib.Ena )
                    pAC->AsfData.Mib.ConfigChange |= 0x01;
                pAC->AsfData.Mib.Ena = *pBuf;
                *pLen = 1;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_ENA: %d\n",  pAC->AsfData.Mib.Ena ));
            }
            else
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            break;

        case OID_SKGE_ASF_RETRANS:
            if( *pLen == 2 )  {
                pAC->AsfData.Mib.Retrans = *( (SK_U16 *) pBuf );
                *pLen = 2;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_RETRANS: %d\n",  pAC->AsfData.Mib.Retrans ));
            }
            else
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            break;

        case OID_SKGE_ASF_RETRANS_INT:
            if( *pLen == 2 )  {
                pAC->AsfData.Mib.RetransInt = *( (SK_U16 *) pBuf ) * ASF_GUI_TSF;
                *pLen = 2;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_RETRANS_INT: %d\n",  pAC->AsfData.Mib.RetransInt ));
            }
            else
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            break;

        case OID_SKGE_ASF_HB_ENA:
            if( *pLen == 1 )  {
                pAC->AsfData.Mib.HbEna = *pBuf;
                *pLen = 1;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_HB_ENA: %d\n",  pAC->AsfData.Mib.HbEna ));
            }
            else
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            break;

        case OID_SKGE_ASF_HB_INT:
            if( *pLen == 4 )  {
                pAC->AsfData.Mib.HbInt = *( (SK_U32 *) pBuf ) * ASF_GUI_TSF;
                *pLen = 4;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_HB_INT: %d\n",  pAC->AsfData.Mib.HbInt ));
            }
            else
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            break;

        case OID_SKGE_ASF_WD_ENA:
            if( *pLen == 1 )  {
                pAC->AsfData.Mib.WdEna = *pBuf;
                *pLen = 1;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_WD_ENA: %d\n",  pAC->AsfData.Mib.WdEna ));
            }
            else
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            break;

        case OID_SKGE_ASF_WD_TIME:
            if( *pLen == 4 )  {
                pAC->AsfData.Mib.WdTime = *( (SK_U32 *) pBuf ) * ASF_GUI_TSF;
                *pLen = 4;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_WD_TIME: %d\n",  pAC->AsfData.Mib.WdTime ));
            }
            else
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            break;

        case OID_SKGE_ASF_IP_DEST:
            if( AsfAsci2Ip( pBuf, *pLen, pAC->AsfData.Mib.IpDest )  == 1 )   {
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_IP_DEST: %d.%d.%d.%d\n",
                                        pAC->AsfData.Mib.IpDest[0], pAC->AsfData.Mib.IpDest[1],
                                        pAC->AsfData.Mib.IpDest[2], pAC->AsfData.Mib.IpDest[3] ));
            }
            else
                RetCode = SK_PNMI_ERR_GENERAL;
            break;

        case OID_SKGE_ASF_MAC_DEST:
            if( AsfAsci2Mac( pBuf, *pLen, pAC->AsfData.Mib.MacDest )  == 1 )   {
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_MAC_DEST: %02x:%02x:%02x:%02x:%02x:%02x\n",
                                        pAC->AsfData.Mib.MacDest[0], pAC->AsfData.Mib.MacDest[1],
                                        pAC->AsfData.Mib.MacDest[2], pAC->AsfData.Mib.MacDest[3],
                                        pAC->AsfData.Mib.MacDest[4], pAC->AsfData.Mib.MacDest[5] ));
#ifdef ASF_FW_ARP_RESOLVE
                //  just for FW-ARP-Resolve testing purposes
//                for( i=0; i<6; i++ )
//                    pAC->AsfData.Mib.MacDest[i] = 0;
#endif  //  ASF_FW_ARP_RESOLVE

            }
            else
                RetCode = SK_PNMI_ERR_GENERAL;
            break;

        case OID_SKGE_ASF_COMMUNITY_NAME:
            for( i=0; (i<*pLen)&&(i<63); i++ )
                pAC->AsfData.Mib.CommunityName[i] = *(pBuf + i);
            pAC->AsfData.Mib.CommunityName[i] = 0;
            break;

        case OID_SKGE_ASF_RSP_ENA:
            if( *pLen == 1 )  {
                if( *pBuf != pAC->AsfData.Mib.RspEnable )
                    pAC->AsfData.Mib.ConfigChange |= 0x02;

                pAC->AsfData.Mib.RspEnable = *pBuf;
                *pLen = 1;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_RSP_ENA: %d\n",  pAC->AsfData.Mib.RspEnable ));
            }
            else
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            break;

        case OID_SKGE_ASF_RETRANS_COUNT_MIN:
            if( *pLen == 4 )  {
                pAC->AsfData.Mib.RetransCountMin = *( (SK_U32 *) pBuf );
                *pLen = 4;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_RETRANS_COUNT_MIN: %d\n",  pAC->AsfData.Mib.RetransCountMin ));
            }
            else
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            break;

        case OID_SKGE_ASF_RETRANS_COUNT_MAX:
            if( *pLen == 4 )  {
                pAC->AsfData.Mib.RetransCountMax = *( (SK_U32 *) pBuf );
                *pLen = 4;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_RETRANS_COUNT_MAX: %d\n",  pAC->AsfData.Mib.RetransCountMax ));
            }
            else
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            break;

        case OID_SKGE_ASF_RETRANS_INT_MIN:
            if( *pLen == 4 )  {
                pAC->AsfData.Mib.RetransIntMin = *( (SK_U32 *) pBuf ) * ASF_GUI_TSF;
                *pLen = 4;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_RETRANS_INT_MIN: %d\n",  pAC->AsfData.Mib.RetransIntMin ));
            }
            else
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            break;

        case OID_SKGE_ASF_RETRANS_INT_MAX:
            if( *pLen == 4 )  {
                pAC->AsfData.Mib.RetransIntMax = *( (SK_U32 *) pBuf ) * ASF_GUI_TSF;
                *pLen = 4;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_RETRANS_INT_MAX: %d\n",  pAC->AsfData.Mib.RetransIntMax ));
            }
            else
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            break;

        case OID_SKGE_ASF_HB_INT_MIN:
            if( *pLen == 4 )  {
                pAC->AsfData.Mib.HbIntMin = *( (SK_U32 *) pBuf ) * ASF_GUI_TSF;
                *pLen = 4;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_HB_INT_MIN: %d\n",  pAC->AsfData.Mib.HbIntMin ));
            }
            else
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            break;

        case OID_SKGE_ASF_HB_INT_MAX:
            if( *pLen == 4 )  {
                pAC->AsfData.Mib.HbIntMax = *( (SK_U32 *) pBuf ) * ASF_GUI_TSF;
                *pLen = 4;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_HB_INT_MAX: %d\n",  pAC->AsfData.Mib.HbIntMax ));
            }
            else
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            break;

        case OID_SKGE_ASF_WD_TIME_MIN:
            if( *pLen == 4 )  {
                pAC->AsfData.Mib.WdTimeMin = *( (SK_U32 *) pBuf ) * ASF_GUI_TSF;
                *pLen = 4;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_WD_TIME_MIN: %d\n",  pAC->AsfData.Mib.WdTimeMin ));
            }
            else
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            break;

        case OID_SKGE_ASF_WD_TIME_MAX:
            if( *pLen == 4 )  {
                pAC->AsfData.Mib.WdTimeMax = *( (SK_U32 *) pBuf ) * ASF_GUI_TSF;
                *pLen = 4;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_WD_TIME_MAX: %d\n",  pAC->AsfData.Mib.WdTimeMax ));
            }
            else
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            break;

        case OID_SKGE_ASF_KEY_OP:
            if( *pLen == (RSP_KEYLENGTH*2) )  {
                AsfHex2Array( pBuf, *pLen, pAC->AsfData.Mib.KeyOperator );
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_KEY_OP:\n >"));
                for( i=0; i<RSP_KEYLENGTH; i++ )
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("%02x",pAC->AsfData.Mib.KeyOperator[i] ));
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("<\n"));
            }
            else
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            break;

        case OID_SKGE_ASF_KEY_ADM:
            if( *pLen == (RSP_KEYLENGTH*2) )  {
                AsfHex2Array( pBuf, *pLen, pAC->AsfData.Mib.KeyAdministrator );
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_KEY_ADM:\n >"));
                for( i=0; i<RSP_KEYLENGTH; i++ )
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("%02x",pAC->AsfData.Mib.KeyAdministrator[i] ));
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("<\n"));
            }
            else
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            break;

        case OID_SKGE_ASF_KEY_GEN:
            if( *pLen == (RSP_KEYLENGTH*2) )  {
                AsfHex2Array( pBuf, *pLen, pAC->AsfData.Mib.KeyGenerator );
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_KEY_GEN:\n >"));
                for( i=0; i<RSP_KEYLENGTH; i++ )
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("%02x",pAC->AsfData.Mib.KeyGenerator[i] ));
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("<\n"));
            }
            else
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            break;

        case OID_SKGE_ASF_PAR_1:
            if( *pLen == 1 )  {
                pAC->AsfData.Mib.Reserved[Id-OID_SKGE_ASF_PAR_1] = *pBuf;
                *pLen = 1;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_PAR_X: ---\n" ));
            }
            else
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            break;

        case OID_SKGE_ASF_IP_SOURCE:
            *pLen = 0;
            break;

        case OID_SKGE_ASF_FWVER_OID:
            // fall through
        case OID_SKGE_ASF_ACPI_OID:
            // fall through
        case OID_SKGE_ASF_SMBUS_OID:
            // these OIDs are read only - they cannot be set
            *pLen = 0;
            break;

        default:
            *pLen = 0;
            RetCode = SK_ASF_PNMI_ERR_NOT_SUPPORTED;
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID not supported \n" ));
            break;
    }

    if( RetCode != SK_ASF_PNMI_ERR_OK )  { //  No bytes used
        if( RetCode == SK_PNMI_ERR_TOO_SHORT )  {
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    *** Error Length: %d \n", *pLen ));
        }
        else  {
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    *** Error ???\n", *pLen ));
        }
        *pLen = 0;
    }
    else
        pAC->AsfData.Mib.NewParam = 2;  //  Trigger SEPROM Update


    return(RetCode);
}


/******************************************************************************
*
*   SkAsfGetOid -
*
* Context:
*   init, pageable
*
* Returns:
*/
int SkAsfGetOid(
            SK_AC *pAC, /* the adapter context */
            SK_IOC IoC, /* I/O context */
            SK_U32 Id,  /* OID  */
            SK_U32 Inst,
            SK_U8 *pBuf,
            unsigned int *pLen)
{
    SK_U32  TmpVal32;
    SK_U32  RetCode = SK_ASF_PNMI_ERR_OK;
    SK_U8   i;

    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfGetOid -> OID:0x%x Len:%d\n", Id, *pLen ));

    if( pAC->AsfData.InitState != ASF_INIT_OK )  {
        SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfSetOid -> Get OID denied, ASF not initialized !\n", Id, *pLen ));
        if( Id != OID_SKGE_ASF_CAP )
            return( SK_ASF_PNMI_ERR_GENERAL );
    }

    switch( Id )  {
        case OID_SKGE_ASF_CAP:
            if( *pLen >= 4 )  {
                TmpVal32 = 0;
                if( (pAC->AsfData.InitState != ASF_INIT_UNDEFINED) &&
                    (pAC->AsfData.InitState != ASF_INIT_ERROR_CHIP_ID) )
                    TmpVal32 |= BIT_0;  //  ASF capable
                if( pAC->AsfData.InitState == ASF_INIT_OK )
                    TmpVal32 |= BIT_1;  //  ASF Init OK
                if( pAC->AsfData.Mib.Ena )
                    TmpVal32 |= BIT_2;  //  ASF enable
                *( (SK_U32 *) pBuf ) = TmpVal32;
                *pLen = 4;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_ENA: %d\n",  pAC->AsfData.Mib.Ena ));
            }
            else  {
                *pLen = 4;
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            }
            break;

        case OID_SKGE_ASF_ENA:
            if( *pLen >= 1 )  {
                *( (SK_U8 *) pBuf ) = pAC->AsfData.Mib.Ena;
                *pLen = 1;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_ENA: %d\n",  pAC->AsfData.Mib.Ena ));
            }
            else  {
                *pLen = 1;
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            }
            break;

        case OID_SKGE_ASF_RETRANS:
            if( *pLen >= 2 )  {
                *( (SK_U16 *) pBuf ) = pAC->AsfData.Mib.Retrans;
                *pLen = 2;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_RETRANS: %d\n",  pAC->AsfData.Mib.Retrans ));
            }
            else  {
                *pLen = 2;
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            }
            break;

        case OID_SKGE_ASF_RETRANS_INT:
            if( *pLen >= 4 )  {
                *( (SK_U32 *) pBuf ) = pAC->AsfData.Mib.RetransInt / ASF_GUI_TSF;
                *pLen = 4;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_RETRANS_INT: %d\n",  pAC->AsfData.Mib.RetransInt ));
            }
            else  {
                *pLen = 4;
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            }
            break;

        case OID_SKGE_ASF_HB_ENA:
            if( *pLen >= 1 )  {
                *( (SK_U8 *) pBuf ) = pAC->AsfData.Mib.HbEna;
                *pLen = 1;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_HB_ENA: %d\n",  pAC->AsfData.Mib.HbEna ));
            }
            else  {
                *pLen = 1;
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            }
            break;

        case OID_SKGE_ASF_HB_INT:
            if( *pLen >= 4 )  {
                *( (SK_U32 *) pBuf ) = pAC->AsfData.Mib.HbInt / ASF_GUI_TSF;
                *pLen = 4;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_HB_INT: %d\n",  pAC->AsfData.Mib.HbInt ));
            }
            else  {
                *pLen = 4;
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            }
            break;

        case OID_SKGE_ASF_WD_ENA:
            if( *pLen >= 1 )  {
                *( (SK_U8 *) pBuf ) = pAC->AsfData.Mib.WdEna;
                *pLen = 1;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_WD_ENA: %d\n",  pAC->AsfData.Mib.WdEna ));
            }
            else  {
                *pLen = 1;
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            }
            break;

        case OID_SKGE_ASF_WD_TIME:
            if( *pLen >= 4 )  {
                *( (SK_U32 *) pBuf ) = pAC->AsfData.Mib.WdTime / ASF_GUI_TSF;
                *pLen = 4;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_WD_TIME: %d\n",  pAC->AsfData.Mib.WdTime ));
            }
            else  {
                *pLen = 4;
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            }
            break;

        case OID_SKGE_ASF_IP_SOURCE:
            if( *pLen >= 16 )  {
                AsfIp2Asci( pBuf+1, pLen, pAC->AsfData.Mib.IpSource );
                *pBuf = (SK_U8) *pLen;
                (*pLen)++;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_IP_SOURCE: %d.%d.%d.%d\n",
                                        pAC->AsfData.Mib.IpSource[0], pAC->AsfData.Mib.IpSource[1],
                                        pAC->AsfData.Mib.IpSource[2], pAC->AsfData.Mib.IpSource[3] ));
            }
            else  {
                *pLen = 16;
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            }
            break;

        case OID_SKGE_ASF_IP_DEST:
            if( *pLen >= 16 )  {
                AsfIp2Asci( pBuf+1, pLen, pAC->AsfData.Mib.IpDest );
                *pBuf = (SK_U8) *pLen;
                (*pLen)++;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_IP_DEST: %d.%d.%d.%d\n",
                                        pAC->AsfData.Mib.IpDest[0], pAC->AsfData.Mib.IpDest[1],
                                        pAC->AsfData.Mib.IpDest[2], pAC->AsfData.Mib.IpDest[3] ));
            }
            else  {
                *pLen = 16;
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            }
            break;

        case OID_SKGE_ASF_MAC_DEST:
            if( *pLen >= 18 )  {
                AsfMac2Asci( pBuf+1, pLen, pAC->AsfData.Mib.MacDest );
                *pBuf = (SK_U8) *pLen;
                (*pLen)++;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_MAC_DEST: %02x:%02x:%02x:%02x:%02x:%02x\n",
                                        pAC->AsfData.Mib.MacDest[0], pAC->AsfData.Mib.MacDest[1],
                                        pAC->AsfData.Mib.MacDest[2], pAC->AsfData.Mib.MacDest[3],
                                        pAC->AsfData.Mib.MacDest[4], pAC->AsfData.Mib.MacDest[5] ));
            }
            else  {
                *pLen = 18;
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            }
            break;

        case OID_SKGE_ASF_MAC_SOURCE:
            if( *pLen >= 18 )  {
                AsfMac2Asci( pBuf+1, pLen, pAC->AsfData.Mib.MacSource );
                *pBuf = (SK_U8) *pLen;
                (*pLen)++;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_MAC_SOURCE: %02x:%02x:%02x:%02x:%02x:%02x\n",
                                        pAC->AsfData.Mib.MacSource[0], pAC->AsfData.Mib.MacSource[1],
                                        pAC->AsfData.Mib.MacSource[2], pAC->AsfData.Mib.MacSource[3],
                                        pAC->AsfData.Mib.MacSource[4], pAC->AsfData.Mib.MacSource[5] ));
            }
            else  {
                *pLen = 18;
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            }
            break;

        case OID_SKGE_ASF_COMMUNITY_NAME:
            if( *pLen >= 64 )  {
                for( *pLen=0; *pLen<63; (*pLen)++  )  {
                    *(pBuf + *pLen + 1 ) = pAC->AsfData.Mib.CommunityName[*pLen];

                    if( pAC->AsfData.Mib.CommunityName[*pLen] != 0 )  {
                        *(pBuf + *pLen + 1 ) = pAC->AsfData.Mib.CommunityName[*pLen];
                    }
                    else  {
                        break;
                    }
                }
                *pBuf = (SK_U8) *pLen;
                (*pLen)++;
            }
            else  {
                *pLen = 64;
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            }
            break;

        case OID_SKGE_ASF_RSP_ENA:
            if( *pLen >= 1 )  {
                *( (SK_U8 *) pBuf ) = pAC->AsfData.Mib.RspEnable;
                *pLen = 1;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_RSP_ENA: %d\n",  pAC->AsfData.Mib.RspEnable ));
            }
            else  {
                *pLen = 1;
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            }
            break;

        case OID_SKGE_ASF_RETRANS_COUNT_MIN:
            if( *pLen >= 4 )  {
                *( (SK_U32 *) pBuf ) = pAC->AsfData.Mib.RetransCountMin;
                *pLen = 4;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_RETRANS_COUNT_MIN: %d\n",  pAC->AsfData.Mib.RetransCountMin ));
            }
            else  {
                *pLen = 4;
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            }
            break;

        case OID_SKGE_ASF_RETRANS_COUNT_MAX:
            if( *pLen >= 4 )  {
                *( (SK_U32 *) pBuf ) = pAC->AsfData.Mib.RetransCountMax;
                *pLen = 4;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_RETRANS_COUNT_MAX: %d\n",  pAC->AsfData.Mib.RetransCountMax ));
            }
            else  {
                *pLen = 4;
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            }
            break;

        case OID_SKGE_ASF_RETRANS_INT_MIN:
            if( *pLen >= 4 )  {
                *( (SK_U32 *) pBuf ) = pAC->AsfData.Mib.RetransIntMin / ASF_GUI_TSF;
                *pLen = 4;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_RETRANS_INT_MIN: %d\n",  pAC->AsfData.Mib.RetransIntMin ));
            }
            else  {
                *pLen = 4;
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            }
            break;

        case OID_SKGE_ASF_RETRANS_INT_MAX:
            if( *pLen >= 4 )  {
                *( (SK_U32 *) pBuf ) = pAC->AsfData.Mib.RetransIntMax / ASF_GUI_TSF;
                *pLen = 4;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_RETRANS_INT_MAX: %d\n",  pAC->AsfData.Mib.RetransIntMax ));
            }
            else  {
                *pLen = 4;
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            }
            break;

        case OID_SKGE_ASF_HB_INT_MIN:
            if( *pLen >= 4 )  {
                *( (SK_U32 *) pBuf ) = pAC->AsfData.Mib.HbIntMin / ASF_GUI_TSF;
                *pLen = 4;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_HB_INT_MIN: %d\n",  pAC->AsfData.Mib.HbIntMin ));
            }
            else  {
                *pLen = 4;
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            }
            break;

        case OID_SKGE_ASF_HB_INT_MAX:
            if( *pLen >= 4 )  {
                *( (SK_U32 *) pBuf ) = pAC->AsfData.Mib.HbIntMax / ASF_GUI_TSF;
                *pLen = 4;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_HB_INT_MAX: %d\n",  pAC->AsfData.Mib.HbIntMax ));
            }
            else  {
                *pLen = 4;
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            }
            break;

        case OID_SKGE_ASF_WD_TIME_MIN:
            if( *pLen >= 4 )  {
                *( (SK_U32 *) pBuf ) = pAC->AsfData.Mib.WdTimeMin / ASF_GUI_TSF;
                *pLen = 4;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_WD_TIME_MIN: %d\n",  pAC->AsfData.Mib.WdTimeMin ));
            }
            else  {
                *pLen = 4;
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            }
            break;

        case OID_SKGE_ASF_WD_TIME_MAX:
            if( *pLen >= 4 )  {
                *( (SK_U32 *) pBuf ) = pAC->AsfData.Mib.WdTimeMax / ASF_GUI_TSF;
                *pLen = 4;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_WD_TIME_MAX: %d\n",  pAC->AsfData.Mib.WdTimeMax ));
            }
            else  {
                *pLen = 4;
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            }
            break;

        case OID_SKGE_ASF_GUID:
            if( *pLen >= 33 )  {
                AsfArray2Hex( pBuf+1, 16, pAC->AsfData.Mib.Guid );
                *pBuf = 32;
                *pLen = 33;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_GUID\n" ));
            }
            else  {
                *pLen = 33;
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            }
            break;

        case OID_SKGE_ASF_KEY_OP:
            if( *pLen >= 41 )  {
                AsfArray2Hex( pBuf+1, 40, pAC->AsfData.Mib.KeyOperator );
                *pBuf = (SK_U8) 40;
                *pLen = 41;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_KEY_OP:\n >"));
                for( i=0; i<RSP_KEYLENGTH; i++ )
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("%02x",pAC->AsfData.Mib.KeyOperator[i] ));
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("<\n"));
            }
            else  {
                *pLen = 41;
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            }
            break;

        case OID_SKGE_ASF_KEY_ADM:
            if( *pLen >= 41 )  {
                AsfArray2Hex( pBuf+1, 40, pAC->AsfData.Mib.KeyAdministrator );
                *pBuf = (SK_U8) 40;
                *pLen = 41;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_KEY_ADM:\n >"));
                for( i=0; i<RSP_KEYLENGTH; i++ )
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("%02x",pAC->AsfData.Mib.KeyAdministrator[i] ));
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("<\n"));
            }
            else  {
                *pLen = 41;
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            }
            break;

        case OID_SKGE_ASF_KEY_GEN:
            if( *pLen >= 41 )  {
                AsfArray2Hex( pBuf+1, 40, pAC->AsfData.Mib.KeyGenerator );
                *pBuf = (SK_U8) 40;
                *pLen = 41;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_KEY_GEN:\n >"));
                for( i=0; i<RSP_KEYLENGTH; i++ )
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("%02x",pAC->AsfData.Mib.KeyGenerator[i] ));
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("<\n"));
            }
            else  {
                *pLen = 41;
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            }
            break;

        case OID_SKGE_ASF_PAR_1:
            if( *pLen >= 1 )  {
                *( (SK_U8 *) pBuf ) = pAC->AsfData.Mib.Reserved[Id-OID_SKGE_ASF_PAR_1];
                *pLen = 1;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_PAR_X: --\n"));
            }
            else  {
                *pLen = 1;
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            }
            break;

        case OID_SKGE_ASF_FWVER_OID:
            // returns the firmware revision to GUI
            if (*pLen >= ASF_FWVER_MAXBUFFLENGTH) {
                for (i=0; i < ASF_FWVER_MAXBUFFLENGTH; i++ ) {
                    // maybe we should lock the access to FwVersionString against reading/writing at the same time?
                    *((SK_U8 *)pBuf+i) = pAC->AsfData.FwVersionString[i];
                }
                *pLen = ASF_FWVER_MAXBUFFLENGTH;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_FWVER_OID: %d\n", *pLen));
            }
            else {
                // set the right length
                *pLen   = ASF_FWVER_MAXBUFFLENGTH;
                RetCode = SK_PNMI_ERR_TOO_SHORT;
            }
            break;

        case OID_SKGE_ASF_ACPI_OID:
            // returns ACPI/ASF table (ASF_ACPI_MAXBUFFLENGTH bytes) to GUI
            if ( (pAC->AsfData.Mib.Acpi.length > 0) && (pAC->AsfData.Mib.Acpi.length <= ASF_ACPI_MAXBUFFLENGTH) ) {
                if (*pLen >= pAC->AsfData.Mib.Acpi.length) {
                    // there is enough space in buffer for reporting ACPI buffer
                    for (i=0; i < pAC->AsfData.Mib.Acpi.length; i++) {
                        // maybe we should lock the access to Acpi.buffer against reading/writing at the same time?
                        *((SK_U8 *)pBuf+i) = pAC->AsfData.Mib.Acpi.buffer[i];
                    }
                    *pLen = pAC->AsfData.Mib.Acpi.length;
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_ACPI_OID: %d\n", *pLen));
                }
                else {
                    // there is not enough space in buffer to report ACPI buffer
                    *pLen   = ASF_ACPI_MAXBUFFLENGTH;
                    RetCode = SK_PNMI_ERR_TOO_SHORT;
                }
            }
            else {
                // no buffer to report
                *pLen = 0;
            }
            break;

        case OID_SKGE_ASF_SMBUS_OID:
            // set a request flag, that smbus info must be updated
            pAC->AsfData.Mib.SMBus.UpdateReq = 1;
            // returns SMBus table GUI
            if ( (pAC->AsfData.Mib.SMBus.length > 0) && (pAC->AsfData.Mib.SMBus.length <= ASF_SMBUS_MAXBUFFLENGTH) ) {
                if (*pLen >= pAC->AsfData.Mib.SMBus.length) {
                    // there is enough space in buffer for reporting ACPI buffer
                    for (i=0; i < pAC->AsfData.Mib.SMBus.length; i++) {
                        // maybe we should lock the access to SMBus.buffer against reading/writing at the same time?
                        *((SK_U8 *)pBuf+i) = pAC->AsfData.Mib.SMBus.buffer[i];
                    }
                    *pLen = pAC->AsfData.Mib.SMBus.length;
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_SMBUS_OID: %d\n", *pLen));
                }
                else {
                    // there is not enough space in buffer to report SMBus buffer
                    *pLen   = ASF_SMBUS_MAXBUFFLENGTH;
                    RetCode = SK_PNMI_ERR_TOO_SHORT;
                }
            }
            else {
                // no buffer to report
                *pLen = 0;
            }
            break;

        case OID_SKGE_ASF_HB_CAP:
            // oid not used
            *pLen = 0;
            break;

        default:
            *pLen   = 0;
            RetCode = SK_ASF_PNMI_ERR_NOT_SUPPORTED;
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID not supported\n",  pAC->AsfData.Mib.Ena ));
            break;
    }

    return(RetCode);
}


/******************************************************************************
*
*   SkAsfGet -
*
* Context:
*   init, pageable
*
* Returns:
*/
int SkAsfGet(
            SK_AC *pAC,    /* the adapter context */
            SK_IOC IoC, /* I/O context */
            SK_U8 *pBuf,
            unsigned int *pLen)
{
    SK_U32  RetCode = SK_ASF_PNMI_ERR_OK;

    return(RetCode) ;
}


/******************************************************************************
*
*   SkAsfPreSet -
*
* Context:
*   init, pageable
*
* Returns:
*/
int SkAsfPreSet(
               SK_AC *pAC, /* the adapter context */
               SK_IOC IoC, /* I/O context */
               SK_U8 *pBuf,
               unsigned int *pLen)
{
    /* preset the Return code to error */
    SK_U32  RetCode = SK_ASF_PNMI_ERR_OK;

    return(RetCode);
}

/******************************************************************************
*
*   SkAsfSet -
*
* Context:
*   init, pageable
*
* Returns:
*/
int SkAsfSet(
            SK_AC *pAC, /* the adapter context */
            SK_IOC IoC, /* I/O context */
            SK_U8 *pBuf,
            unsigned int *pLen)
{
    SK_U32  RetCode = SK_ASF_PNMI_ERR_OK;

    return(RetCode);
}

SK_I8 SkAsfStartWriteDeferredFlash( SK_AC *pAC, SK_IOC IoC )  {
    SK_I8	RetCode;
    SK_EVPARA	EventParam;

    RetCode = 1;
    if( pAC->AsfData.StateWrSpi == 0 )  {
        pAC->AsfData.StateWrSpi++;
        SK_MEMSET((char *)&EventParam, 0, sizeof(EventParam));
        SkTimerStart(pAC, IoC, &pAC->AsfData.AsfTimerWrSpi,
                     10000, SKGE_ASF, SK_ASF_EVT_TIMER_SPI_EXPIRED,
                     EventParam);
        RetCode = 0;
    }

    return( RetCode );
}

SK_I8 SkAsfWriteDeferredFlash( SK_AC *pAC, SK_IOC IoC )  {
    SK_I8       RetCode;
    SK_U8       StartAgain;

    RetCode = 1;  //  unsuccessfull
    StartAgain = 0;

    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("          WriteDeferredFlash:  State:%d\n", pAC->AsfData.StateWrSpi ));

#ifdef _XXX_

    switch( pAC->AsfData.StateWrSpi )  {
        case 0:  //  idle State
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("          WriteDeferredFlash:  idle\n"));
            break;

        case 1: //  erase SPI flash sector
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("          WriteDeferredFlash:  Erase flash\n"));
            if (spi_flash_erase(pAC, ASF_SPI_SECTOR, ASF_SPI_SECTOR_SIZE)  == 0 )  {  //  successfull
                RetCode = 0;
                StartAgain = 1;
                pAC->AsfData.StateWrSpi++;
            }
            else
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("          WriteDeferredFlash:  Error Erase flash\n"));
            break;

        default:
            //  128 * 0x100 (256)  Bytes
            if( pAC->AsfData.StateWrSpi <= 129 )  {
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("          WriteDeferredFlash:  Write addr:0x%x size:0x%x\n",
                                                                    ASF_SPI_SECTOR+((pAC->AsfData.StateWrSpi-2)*0x100), 0x100 ));
                if (spi_flash_manage(pAC,
                                     &pAC->AsfData.SpiBuf[(pAC->AsfData.StateWrSpi-2)*0x100],
                                     ASF_SPI_SECTOR+((pAC->AsfData.StateWrSpi-2)*0x100),
                                     0x100, SPI_WRITE)  == 0 ) {
                    RetCode = 0;
                    StartAgain = 1;
                    pAC->AsfData.StateWrSpi++;
                }
                else
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("          WriteDeferredFlash:  Error Write addr:0x%x size:0x%x\n",
                                                                   ASF_SPI_SECTOR+((pAC->AsfData.StateWrSpi-2)*0x100), 0x100 ));
            }
            else  {
                //  Finish
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("          WriteDeferredFlash:  Finish\n"));
                RetCode = 1;
            }
            break;

    }  //   switch( pAC->AsfData.StateWrSpi )

    if( StartAgain )  {
        SK_MEMSET((char *)&EventParam, 0, sizeof(EventParam));
        SkTimerStart(pAC, IoC, &pAC->AsfData.AsfTimerWrSpi,
                     10000, SKGE_ASF, SK_ASF_EVT_TIMER_SPI_EXPIRED,
                     EventParam);
    }
    else  {
        pAC->AsfData.StateWrSpi = 0;
        pAC->AsfData.Mib.WriteToFlash = 0;
    }

#endif //  _XXX_

    return( RetCode );
}


/*****************************************************************************
*
* SkAsfEvent - Event handler
*
* Description:
*   Handles the following events:
*   SK_ASF_EVT_TIMER_EXPIRED When a hardware counter overflows an
*
* Returns:
*   Always 0
*/
int SkAsfEvent(
              SK_AC *pAC, /* Pointer to adapter context */
              SK_IOC IoC, /* IO context handle */
              SK_U32 Event,  /* Event-Id */
              SK_EVPARA Param) /* Event dependent parameter */
{

    switch(Event)
    {
        case SK_ASF_EVT_TIMER_EXPIRED:
            // SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("ASF: SkAsfEvent -> Timer expired\n" ));
            SkAsfTimer( pAC, IoC );
            break;

        case SK_ASF_EVT_TIMER_SPI_EXPIRED:
            // SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("ASF: SkAsfEvent -> TimerWrSpi expired State:%d\n",
            //                                                pAC->AsfData.StateWrSpi ));
            SkAsfWriteDeferredFlash( pAC, IoC );
            break;

        case SK_ASF_EVT_TIMER_HCI_EXPIRED:
            //  SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("ASF: SkAsfEvent -> TimerWrHci expired GlHciState:%d\n",
            //                                                pAC->AsfData.GlHciState ));
            SkAsfHci( pAC, IoC, 1 );
            break;

        default:
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("ASF: SkAsfEvent -> Unknown Event:0x%x\n", Event ));
            break;
    }
    return(0); /* Success. */
} /* SkAsfEvent */

/*****************************************************************************
*
* SkAsfTimer - general ASF  timer
*
* Description:
*
* Returns:
*   Always 0
*/
void SkAsfTimer(
SK_AC *pAC, /* Pointer to adapter context */
SK_IOC IoC ) /* IO context handle */  {

    SK_EVPARA   EventParam;
    SK_U8       i, NewAddr;
    SK_U8       lRetCode;
    SK_U8       TmpBuffer[128];
    SK_U8       Ind;
    SK_U8       Length;
    SK_U8       *pHciRecBuf;

    AsfWatchCpu(pAC, IoC, 50 );

    pHciRecBuf = NULL;

    // check, whether IP address has changed
    NewAddr = 0;
    for( i=0; i<4; i++ )  {
        if( pAC->IpAddr[i] != pAC->AsfData.Mib.IpSource[i] )  {
            NewAddr = 1;
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: New IP Addr detected\n"));
        }
    }

    if( pAC->AsfData.Mib.NewParam > 1 ) {
        pAC->AsfData.Mib.NewParam--;
    }

    switch(  pAC->AsfData.GlHciState  )  {
        /*------------------------------------------------------------------------------------------------------
         *  check for changes in config data
         *----------------------------------------------------------------------------------------------------*/
        case 0:
	  
	  switch(pAC->AsfData.OpMode)  {
	  case SK_GEASF_MODE_ASF:
	    if(pAC->AsfData.LastGlHciState == 255) {
	      pAC->AsfData.LastGlHciState = 0;
	      pAC->AsfData.GlHciState     = 100;  //  Fetch ASF configuration from FW
	      pAC->AsfData.Mib.PattUpReq  = 1;    // ...and update pattern ram
	    }
	    else {
	      if ( (pAC->AsfData.Mib.SMBus.UpdateReq >= 1) && (pAC->AsfData.GlHciState == 0) ) {
		// lock this mechanism, if the GlHciState goes really to the "SMBus update state"
		// pAC->AsfData.Mib.SMBus.UpdateReq = 0;
		
		// go to SMBus update state
		pAC->AsfData.GlHciState = 20;
		SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("Go to State 20\n"));
	      }
	      
	      if ( (pAC->AsfData.Mib.NewParam == 1) || (NewAddr && pAC->AsfData.Mib.Ena))  {
		pAC->AsfData.GlHciState = 50;
		SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: Start Update ASF config data\n"));
	                    }
	      
	      if ( pAC->AsfData.VpdInitOk == 0 )  {
		pAC->AsfData.GlHciState = 1;
		SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: Start Init Vpd \n"));
	      }
	    }
#ifdef ASF_FW_WD
	    //  FW WD functionality
	    if( pAC->AsfData.FwWdIntervall > 1 )
	      pAC->AsfData.FwWdIntervall--;
	    if( pAC->AsfData.FwWdIntervall == 1 )  {
	                        pAC->AsfData.GlHciState = 1;  //  go to check alive
	    }
#endif
	    break;
	    
	  case SK_GEASF_MODE_IPMI:

	    if ( (NewAddr == 1) ||
		 (pAC->AsfData.ActivePort   != (SK_U8)(pAC->ActivePort & 0xff)) ||
		 (pAC->AsfData.PrefPort     != (SK_U8)(pAC->Rlmt.Net[0].PrefPort & 0xff)) ||
		 (pAC->AsfData.Mib.RlmtMode != (SK_U8)(pAC->Rlmt.NumNets & 0xff)) ) {
	      
	      // config has changed for IPMI
	      pAC->AsfData.GlHciState = 30;
	      SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,
			 ("SkAsfTimer: IPMI update config with %d 0x%08x 0x%08x\n",
			  NewAddr, pAC->ActivePort, pAC->Rlmt.Net[0].PrefPort, pAC->Rlmt.NumNets));
	    }
	    
	    // check the ipmi firmware
	    if( pAC->AsfData.FwWdIntervall > 1 ) {
	      pAC->AsfData.FwWdIntervall--;
	    }
	    else {
	      pAC->AsfData.GlHciState = 1;  //  go to check alive
	    }
	    
	    break;
	    
	  case SK_GEASF_MODE_DASH:
	    if ( NewAddr == 1 ){
	      // config has changed for DASH

	      pAC->AsfData.GlHciState = 40;
	      SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,
			 ("SkAsfTimer: DASH update config with NewAddr: %d\n",NewAddr ));
	    }
	    
	    // check the DASH firmware
	    if( pAC->AsfData.FwWdIntervall > 1 ) {
	      pAC->AsfData.FwWdIntervall--;
	    }
	    else {
	      pAC->AsfData.GlHciState = 1;  //  go to check alive
	    }
	    
	    //if( AsfHciSendCommand( pAC, IoC, YASF_HOSTCMD_DRV_HELLO, 0, 0, 0, ASF_HCI_WAIT, 3 ) != HCI_EN_CMD_READY )  {
	    //  printk("ASF: DRV_HELLO failed\n");
	    //} else {
	    //  printk("ASF: DRV_HELLO OK\n");
	    //}
	    
	    break;
	    
	  default:
	    printk("Unknow Mode\n");

	    break;
	  }
	  
	  break;
	  
            /*------------------------------------------------------------------------------------------------------
             *  Do VPD init
             *  Due to the SPI/VPD problem (A1), VpdInit MUST NOT be invoked while the ASF FW is running from flash.
             *  Therefore we do it here, while the ASF FW is running in the RamIdleLoop
             *----------------------------------------------------------------------------------------------------*/
        case 1:
            lRetCode = AsfHciSendCommand(pAC, IoC, YASF_HOSTCMD_CHECK_ALIVE, 0, 0, 0, ASF_HCI_NOWAIT, 0 );
            if( lRetCode == HCI_EN_CMD_READY )  {
                pAC->AsfData.GlHciState++;
                // SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,
                //    ("SkAsfTimer: *** CPU is running (%d) ***\n", pAC->AsfData.OpMode));
                // CheckAlive was initiated by WD
                if( pAC->AsfData.FwWdIntervall == 1 )  {
                    pAC->AsfData.GlHciState = 0;
                    pAC->AsfData.FwWdIntervall = 10;
                }
            }
            if( lRetCode == HCI_EN_CMD_ERROR )  {
                pAC->AsfData.GlHciState = 255;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: *** CPU is NOT running !!! ***\n"));
                AsfResetCpu( pAC, IoC );
                AsfRunCpu( pAC, IoC );
                // CheckAlive was initiated by WD
                if( pAC->AsfData.FwWdIntervall == 1 )  {
                    pAC->AsfData.GlHciState = 0;
                    pAC->AsfData.FwWdIntervall = 60;
                }
            }
            break;

        case 2:
            lRetCode = AsfHciSendCommand(pAC, IoC, YASF_HOSTCMD_ENTER_RAM_IDLE, 0, 0, 0, ASF_HCI_NOWAIT, 0 );
            if( lRetCode == HCI_EN_CMD_READY )  {
                pAC->AsfData.GlHciState++;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: sent ENTER_RAM_IDLE \n"));
            }
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;

        case 3:
            if( pAC->AsfData.VpdInitOk == 0 )  {
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: VpdInit\n"));
                if (VpdInit(pAC, IoC) == 0) {
                    pAC->AsfData.VpdInitOk = 1;
                    pAC->AsfData.GlHciState++;
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("SkAsfTimer: VPD init o.k.\n"));
                }
                else
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("SkAsfTimer: VPD init error\n"));
            }
            else
                pAC->AsfData.GlHciState++;
            break;
        case 4:
            lRetCode = AsfHciSendCommand(pAC, IoC, YASF_HOSTCMD_LEAVE_RAM_IDLE, 0, 0, 0, ASF_HCI_NOWAIT, 0 );
            if( lRetCode == HCI_EN_CMD_READY )  {
                pAC->AsfData.GlHciState++;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: sent LEAVE_RAM_IDLE \n"));
            }
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;
        case 5:
            lRetCode = AsfHciSendCommand(pAC, IoC, YASF_HOSTCMD_UPDATE_OWN_MACADDR, 0, 0, 0, ASF_HCI_NOWAIT, 0 );
            if( lRetCode == HCI_EN_CMD_READY )  {
                if(pAC->AsfData.NewGuid)
                    pAC->AsfData.GlHciState = 50;   // Write default parameters to SEEPROM
                else {
                    pAC->AsfData.GlHciState     = 100;  //  Fetch ASF configuration from FW
                    pAC->AsfData.Mib.PattUpReq  = 1;    // ...and update pattern ram
                }
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: sent LEAVE_RAM_IDLE \n"));
            }
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;


        /*------------------------------------------------------------------------------------------------------
         *  updates SMBus structure via HCI
         *----------------------------------------------------------------------------------------------------*/
        case 20:
            // If there is a request for updating, we work it out and go to idle state.
            pAC->AsfData.Mib.SMBus.UpdateReq = 0;

            // get SMBus infos from FW
            lRetCode = AsfHciSendCommand(pAC, IoC, YASF_HOSTCMD_CFG_GET_SMBUS_INFOS, 0, 0, 1, ASF_HCI_NOWAIT, 0 );

            if( lRetCode == HCI_EN_CMD_READY )  {
                // Fetch received data from HCI interface
                AsfHciGetData( pAC, &pHciRecBuf );
                Length = *(pHciRecBuf + 1);
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: SMBus info update via HCI l:%d\n", Length))

                pAC->AsfData.Mib.SMBus.length = 0;
                for( i=0; (i<ASF_SMBUS_MAXBUFFLENGTH)&&(i<Length); i++ )  {
                    pAC->AsfData.Mib.SMBus.length   += 1;
                    pAC->AsfData.Mib.SMBus.buffer[i] = *(pHciRecBuf+2+i);
                }
                pAC->AsfData.GlHciState = 0; // go to idle
            }
            if (lRetCode == HCI_EN_CMD_ERROR) {
                pAC->AsfData.GlHciState = 255;
            }
            break;


        /*------------------------------------------------------------------------------------------------------
         *  IPMI states 30...36
         *----------------------------------------------------------------------------------------------------*/

        case 30:
            // RLMT mode has changed (only relevant in IPMI operation mode)
            // in IPMI operation mode, we use the  YASF_HOSTCMD_CFG_SET_RSP_ENABLE message
            // as indicator for RLMT mode / Dual Net mode
            // pAC->AsfData.Mib.RlmtMode > 1  => Dual Net Mode On
            // else                           => RLMT Mode On
	    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,
                ("SkAsfTimer: sending  YASF_HOSTCMD_CFG_SET_RSP_ENABLE 0x%x -> 0x%x\n", pAC->AsfData.Mib.RlmtMode, pAC->Rlmt.NumNets));

            pAC->AsfData.Mib.RlmtMode = (SK_U8)(pAC->Rlmt.NumNets & 0xff);

            if (pAC->AsfData.Mib.RlmtMode > 1) {
                // switch to Dual net mode
                if (pAC->AsfData.DualMode == SK_GEASF_Y2_DUALPORT) {
                    // enable pattern for port b
                    YlciEnablePattern(pAC, IoC, 1, 4);      // RSP pattern
                    YlciEnablePattern(pAC, IoC, 1, 6);      // RMCP pattern
                }

                // enable pattern for port a
                YlciEnablePattern(pAC, IoC, 0, 4);          // RSP pattern
                YlciEnablePattern(pAC, IoC, 0, 6);          // RMCP pattern
            }

            Ind = 0;
            TmpBuffer[Ind++] = YASF_HOSTCMD_CFG_SET_RSP_ENABLE;
            TmpBuffer[Ind++] = 1 + 2;  // Length
            TmpBuffer[Ind++] = pAC->AsfData.Mib.RlmtMode;
            lRetCode = AsfHciSendData(pAC, IoC, TmpBuffer, 0, ASF_HCI_NOWAIT, 0 );
            pAC->AsfData.GlHciState++;
            break;

        case 31:
            lRetCode = AsfHciGetState( pAC );
            if( (lRetCode == HCI_EN_CMD_READY) || (lRetCode == HCI_EN_CMD_IDLE))
                pAC->AsfData.GlHciState++;
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;

        case 32:
	   SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("SkAsfTimer: sending  YASF_HOSTCMD_CFG_SET_ACTIVE_PORT act 0x%x -> 0x%x pref 0x%x -> 0x%x\n", pAC->AsfData.ActivePort, pAC->ActivePort, pAC->AsfData.PrefPort, pAC->Rlmt.Net[0].PrefPort));

            pAC->AsfData.ActivePort = (SK_U8)(pAC->ActivePort & 0xff);
            pAC->AsfData.PrefPort   = (SK_U8)(pAC->Rlmt.Net[0].PrefPort & 0xff);

            // write always pattern enable for the active port
            if (pAC->AsfData.Mib.RlmtMode <= 1) {
                // we are in rlmt mode
                if (pAC->AsfData.ActivePort == 1) {
                    if (pAC->AsfData.DualMode == SK_GEASF_Y2_DUALPORT) {
                        // enable pattern for port b
                        YlciEnablePattern(pAC, IoC, 1, 4);      // RSP pattern
                        YlciEnablePattern(pAC, IoC, 1, 6);      // RMCP pattern
                    }

                    // disable pattern for port a
                    YlciDisablePattern(pAC, IoC, 0, 4);         // RSP pattern
                    YlciDisablePattern(pAC, IoC, 0, 6);         // RMCP pattern
                }
                else {
                    // enable pattern for port a
                    YlciEnablePattern(pAC, IoC, 0, 4);          // RSP pattern
                    YlciEnablePattern(pAC, IoC, 0, 6);          // RMCP pattern

                    if (pAC->AsfData.DualMode == SK_GEASF_Y2_DUALPORT) {
                        // disable pattern for port b
                        YlciDisablePattern(pAC, IoC, 1, 4);     // RSP pattern
                        YlciDisablePattern(pAC, IoC, 1, 6);     // RMCP pattern
                    }
                }
            }

            Ind = 0;
            TmpBuffer[Ind++] = YASF_HOSTCMD_CFG_SET_ACTIVE_PORT;
            TmpBuffer[Ind++] = 1 + 3;  // Length
            TmpBuffer[Ind++] = pAC->AsfData.ActivePort;

            // preferred port
            if (pAC->AsfData.PrefPort == 2) {
                // note: 0 = "auto", 1 = port A, 2 = port B,
                TmpBuffer[Ind++] = 1;   // port B
            }
            else {
                TmpBuffer[Ind++] = 0;   // port A
            }

            lRetCode = AsfHciSendData(pAC, IoC, TmpBuffer, 0, ASF_HCI_NOWAIT, 0 );
            pAC->AsfData.GlHciState++;
            break;

        case 33:
            lRetCode = AsfHciGetState( pAC );
            if( (lRetCode == HCI_EN_CMD_READY) || (lRetCode == HCI_EN_CMD_IDLE) ) {
                pAC->AsfData.GlHciState++;          // next state
            }
            if( lRetCode == HCI_EN_CMD_ERROR ) {
                pAC->AsfData.GlHciState = 255;      // error state
            }
            break;

        case 34:
            // Comment: Error? Empty IpAddr! -> No SW-Assignment for the BMC-IP
            // printk("Omitting Command 34 <- not implemented\n");
            pAC->AsfData.GlHciState++;
            break;

        case 35:
            lRetCode = AsfHciGetState( pAC );
            if( (lRetCode == HCI_EN_CMD_READY) || (lRetCode == HCI_EN_CMD_IDLE))  {
                pAC->AsfData.GlHciState++;          // finisch
            }
            if( lRetCode == HCI_EN_CMD_ERROR ) {
                pAC->AsfData.GlHciState = 255;      // error
            }
            break;

        case 36:
            lRetCode = AsfHciSendCommand(pAC, IoC, YASF_HOSTCMD_CFG_GET_FW_VERSION_STRING, 0, 0, 1, ASF_HCI_NOWAIT, 0 );
            if( lRetCode == HCI_EN_CMD_READY )  {
                // Fetch received data from HCI interface
                AsfHciGetData( pAC, &pHciRecBuf );
                Length = *(pHciRecBuf + 1);
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: YASF_HOSTCMD_CFG_GET_FW_VERSION_STRING l:%d\n",Length))
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("         >"))
                for( i=0; (i<79)&&(i<Length); i++ )  {
                    pAC->AsfData.FwVersionString[i]   = *(pHciRecBuf+2+i);
                    pAC->AsfData.FwVersionString[i+1] = 0;
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("%c",pAC->AsfData.FwVersionString[i]))
                }
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("<\n"))
                pAC->AsfData.GlHciState = 254;      // go to finish
            }
            if( lRetCode == HCI_EN_CMD_ERROR ) {
                pAC->AsfData.GlHciState = 255;      // error state
            }
            break;

    // DASH ASF FW CMDS (40-48)
    case 40:
      pAC->AsfData.GlHciState++;
      break;
      
    case 41:
      pAC->AsfData.GlHciState++;
      break;
      
    case 42:
      SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,
		 ("SkAsfTimer: sending  YASF_HOSTCMD_CFG_SET_DASH_ENABLE \n"));
      Ind = 0;
      TmpBuffer[Ind++] = YASF_HOSTCMD_CFG_SET_DASH_ENABLE;
      TmpBuffer[Ind++] = 1 + 2;  // Length
      TmpBuffer[Ind++] = pAC->AsfData.Mib.RlmtMode;
      lRetCode = AsfHciSendData(pAC ,IoC , TmpBuffer, 0, ASF_HCI_WAIT, 2 );
      pAC->AsfData.GlHciState++;
      break;
      
    case 43:
      lRetCode = AsfHciGetState( pAC );
      if( (lRetCode == HCI_EN_CMD_READY) || (lRetCode == HCI_EN_CMD_IDLE)) {
	pAC->AsfData.GlHciState++;
      }

      if( lRetCode == HCI_EN_CMD_ERROR ) {
	pAC->AsfData.GlHciState = 255;
      }
      
      break;
	    
    case 44:
      //SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,
      //	 ("SkAsfTimer: sending  YASF_HOSTCMD_CFG_SET_ACTIVE_PORT act 0x%x -> 0x%x pref 0x%x -> 0x%x\n",
      //	  pAC->AsfData.ActivePort, pAC->ActivePort, pAC->AsfData.PrefPort, pAC->GeneralConfig.PrefPort));
      SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,
		 ("SkAsfTimer: sending  YASF_HOSTCMD_CFG_SET_ACTIVE_PORT act 0x%x -> 0x%x\n",
		  pAC->AsfData.ActivePort, pAC->ActivePort));

      
      pAC->AsfData.ActivePort = (SK_U8)(pAC->ActivePort & 0xff);
      // pAC->AsfData.PrefPort   = (SK_U8)(pAC->PrefPort & 0xff);
      
      Ind = 0;
      TmpBuffer[Ind++] = YASF_HOSTCMD_CFG_SET_ACTIVE_PORT;
      TmpBuffer[Ind++] = 1 + 3;  // Length
      TmpBuffer[Ind++] = pAC->AsfData.ActivePort;
      // preferred port
      if (pAC->AsfData.PrefPort == 2) {
	// note: 0 = "auto", 1 = port A, 2 = port B,
	TmpBuffer[Ind++] = 1;   // port B
      }
      else {
	TmpBuffer[Ind++] = 0;   // port A
      }
      
      lRetCode = AsfHciSendData(pAC ,IoC , TmpBuffer, 0, ASF_HCI_NOWAIT, 0 );
      pAC->AsfData.GlHciState++;
      break;
      
    case 45:
      lRetCode = AsfHciGetState( pAC );

      if( (lRetCode == HCI_EN_CMD_READY) || (lRetCode == HCI_EN_CMD_IDLE) ) {
	pAC->AsfData.GlHciState++;          // next state
      }
      if( lRetCode == HCI_EN_CMD_ERROR ) {
	pAC->AsfData.GlHciState = 255;      // error state
      }
      break;
      
    case 46:
      SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,
		 ("SkAsfTimer: sending  YASF_HOSTCMD_CFG_SET_IP_SOURCE %03d.%03d.%03d.%03d -> %03d.%03d.%03d.%03d\n",
		  pAC->AsfData.Mib.IpSource[0], pAC->AsfData.Mib.IpSource[1], pAC->AsfData.Mib.IpSource[2], pAC->AsfData.Mib.IpSource[3],
		  pAC->IpAddr[0], pAC->IpAddr[1], pAC->IpAddr[2], pAC->IpAddr[3]));
      Ind = 0;
      TmpBuffer[Ind++] = YASF_HOSTCMD_CFG_SET_IP_SOURCE;
      TmpBuffer[Ind++] = 4 + 2;  // Length

      for( i=0; i<4; i++ ) {
	TmpBuffer[Ind++] = pAC->IpAddr[i];
      }

      lRetCode = AsfHciSendData(pAC ,IoC , TmpBuffer, 0, ASF_HCI_NOWAIT, 0 );
      pAC->AsfData.GlHciState++;
      break;
      
    case 47:
      lRetCode = AsfHciGetState( pAC );
      if( (lRetCode == HCI_EN_CMD_READY) || (lRetCode == HCI_EN_CMD_IDLE))  {
	for( i=0; i<4; i++ ) {
	  pAC->AsfData.Mib.IpSource[i] = pAC->IpAddr[i];
	}
	pAC->AsfData.GlHciState++;          // finish
      }
      if( lRetCode == HCI_EN_CMD_ERROR ) {
	pAC->AsfData.GlHciState = 255;      // error
      }
      break;
      
    case 48:
      lRetCode = AsfHciSendCommand(pAC ,IoC , YASF_HOSTCMD_CFG_GET_FW_VERSION_STRING, 0, 0, 1, ASF_HCI_NOWAIT, 0 );
      if( lRetCode == HCI_EN_CMD_READY )  {
	// Fetch received data from HCI interface
	AsfHciGetData( pAC, &pHciRecBuf );
	Length = *(pHciRecBuf + 1);
	SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: YASF_HOSTCMD_CFG_GET_FW_VERSION_STRING l:%d\n",Length));
	for( i=0; (i<79)&&(i<Length); i++ )  {
	  pAC->AsfData.FwVersionString[i]   = *(pHciRecBuf+2+i);
	  pAC->AsfData.FwVersionString[i+1] = 0;
	}
	pAC->AsfData.GlHciState = 254;      // go to finish
      }
      if( lRetCode == HCI_EN_CMD_ERROR ) {
	pAC->AsfData.GlHciState = 255;      // error state
      }
      break;
        /*------------------------------------------------------------------------------------------------------
         *  Send new ASF configuration data to FW (SEEPROM)
         *----------------------------------------------------------------------------------------------------*/
        /* YASF_HOSTCMD_CFG_SET_ASF_ENABLE */
        case 50:
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: sending  YASF_HOSTCMD_CFG_SET_ASF_ENABLE\n"));
            Ind = 0;
            TmpBuffer[Ind++] = YASF_HOSTCMD_CFG_SET_ASF_ENABLE;
            TmpBuffer[Ind++] = 1 + 2;  // Length
            TmpBuffer[Ind++] = pAC->AsfData.Mib.Ena;
            lRetCode = AsfHciSendData(pAC, IoC, TmpBuffer, 0, ASF_HCI_NOWAIT, 0 );
            pAC->AsfData.GlHciState++;
            break;
        case 51:
            lRetCode = AsfHciGetState( pAC );
            if( (lRetCode == HCI_EN_CMD_READY) || (lRetCode == HCI_EN_CMD_IDLE))
                pAC->AsfData.GlHciState++;
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;
        /* YASF_HOSTCMD_CFG_SET_RSP_ENABLE */
        case 52:
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: sending  YASF_HOSTCMD_CFG_SET_RSP_ENABLE\n"));
            Ind = 0;
            TmpBuffer[Ind++] = YASF_HOSTCMD_CFG_SET_RSP_ENABLE;
            TmpBuffer[Ind++] = 1 + 2;  // Length
            TmpBuffer[Ind++] = pAC->AsfData.Mib.RspEnable;
            lRetCode = AsfHciSendData(pAC, IoC, TmpBuffer, 0, ASF_HCI_NOWAIT, 0 );
            pAC->AsfData.GlHciState++;
            break;
        case 53:
            lRetCode = AsfHciGetState( pAC );
            if( (lRetCode == HCI_EN_CMD_READY) || (lRetCode == HCI_EN_CMD_IDLE))
                pAC->AsfData.GlHciState++;
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;
        /* YASF_HOSTCMD_CFG_SET_RETRANS */
        case 54:
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: sending  YASF_HOSTCMD_CFG_SET_RETRANS\n"));
            Ind = 0;
            TmpBuffer[Ind++] = YASF_HOSTCMD_CFG_SET_RETRANS;
            TmpBuffer[Ind++] = 1 + 2;  // Length
            TmpBuffer[Ind++] = (SK_U8) pAC->AsfData.Mib.Retrans;
            lRetCode = AsfHciSendData(pAC, IoC, TmpBuffer, 0, ASF_HCI_NOWAIT, 0 );
            pAC->AsfData.GlHciState++;
            break;
        case 55:
            lRetCode = AsfHciGetState( pAC );
            if( (lRetCode == HCI_EN_CMD_READY) || (lRetCode == HCI_EN_CMD_IDLE))
                pAC->AsfData.GlHciState++;
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;
        /* YASF_HOSTCMD_CFG_SET_RETRANS_INT */
        case 56:
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: sending  YASF_HOSTCMD_CFG_SET_RETRANS_INT\n"));
            Ind = 0;
            TmpBuffer[Ind++] = YASF_HOSTCMD_CFG_SET_RETRANS_INT;
            TmpBuffer[Ind++] = 2 + 2;  // Length
            TmpBuffer[Ind++] = (SK_U8)((pAC->AsfData.Mib.RetransInt >> 8) & 0x000000FF);
            TmpBuffer[Ind++] = (SK_U8)((pAC->AsfData.Mib.RetransInt >> 0) & 0x000000FF);
            lRetCode = AsfHciSendData(pAC, IoC, TmpBuffer, 0, ASF_HCI_NOWAIT, 0 );
            pAC->AsfData.GlHciState++;
            break;
        case 57:
            lRetCode = AsfHciGetState( pAC );
            if( (lRetCode == HCI_EN_CMD_READY) || (lRetCode == HCI_EN_CMD_IDLE))
                pAC->AsfData.GlHciState++;
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;
        /* YASF_HOSTCMD_CFG_SET_HB_ENABLE */
        case 58:
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: sending  YASF_HOSTCMD_CFG_SET_HB_ENABLE\n"));
            Ind = 0;
            TmpBuffer[Ind++] = YASF_HOSTCMD_CFG_SET_HB_ENABLE;
            TmpBuffer[Ind++] = 1 + 2;  // Length
            TmpBuffer[Ind++] = (SK_U8) pAC->AsfData.Mib.HbEna;
            lRetCode = AsfHciSendData(pAC, IoC, TmpBuffer, 0, ASF_HCI_NOWAIT, 0 );
            pAC->AsfData.GlHciState++;
            break;
        case 59:
            lRetCode = AsfHciGetState( pAC );
            if( (lRetCode == HCI_EN_CMD_READY) || (lRetCode == HCI_EN_CMD_IDLE))
                pAC->AsfData.GlHciState++;
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;
        /* YASF_HOSTCMD_CFG_SET_HB_INT */
        case 60:
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: sending  YASF_HOSTCMD_CFG_SET_HB_INT\n"));
            Ind = 0;
            TmpBuffer[Ind++] = YASF_HOSTCMD_CFG_SET_HB_INT;
            TmpBuffer[Ind++] = 2 + 2;  // Length
            TmpBuffer[Ind++] = (SK_U8)((pAC->AsfData.Mib.HbInt >> 8) & 0x000000FF);
            TmpBuffer[Ind++] = (SK_U8)((pAC->AsfData.Mib.HbInt >> 0) & 0x000000FF);
            lRetCode = AsfHciSendData(pAC, IoC, TmpBuffer, 0, ASF_HCI_NOWAIT, 0 );
            pAC->AsfData.GlHciState++;
            break;
        case 61:
            lRetCode = AsfHciGetState( pAC );
            if( (lRetCode == HCI_EN_CMD_READY) || (lRetCode == HCI_EN_CMD_IDLE))
                pAC->AsfData.GlHciState++;
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;
        /* YASF_HOSTCMD_CFG_SET_DRWD_ENABLE */
        case 62:
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: sending  YASF_HOSTCMD_CFG_SET_DRWD_ENABLE\n"));
            Ind = 0;
            TmpBuffer[Ind++] = YASF_HOSTCMD_CFG_SET_DRWD_ENABLE;
            TmpBuffer[Ind++] = 1 + 2;  // Length
            TmpBuffer[Ind++] = (SK_U8) pAC->AsfData.Mib.WdEna;
            lRetCode = AsfHciSendData(pAC, IoC, TmpBuffer, 0, ASF_HCI_NOWAIT, 0 );
            pAC->AsfData.GlHciState++;
            break;
        case 63:
            lRetCode = AsfHciGetState( pAC );
            if( (lRetCode == HCI_EN_CMD_READY) || (lRetCode == HCI_EN_CMD_IDLE))
                pAC->AsfData.GlHciState++;
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;
        /* YASF_HOSTCMD_CFG_SET_DRWD_INT */
        case 64:
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: sending  YASF_HOSTCMD_CFG_SET_DRWD_INT\n"));
            Ind = 0;
            TmpBuffer[Ind++] = YASF_HOSTCMD_CFG_SET_DRWD_INT;
            TmpBuffer[Ind++] = 2 + 2;  // Length
            TmpBuffer[Ind++] = (SK_U8)((pAC->AsfData.Mib.WdTime >> 8) & 0x000000FF);
            TmpBuffer[Ind++] = (SK_U8)((pAC->AsfData.Mib.WdTime >> 0) & 0x000000FF);
            lRetCode = AsfHciSendData(pAC, IoC, TmpBuffer, 0, ASF_HCI_NOWAIT, 0 );
            pAC->AsfData.GlHciState++;
            break;
        case 65:
            lRetCode = AsfHciGetState( pAC );
            if( (lRetCode == HCI_EN_CMD_READY) || (lRetCode == HCI_EN_CMD_IDLE))
                pAC->AsfData.GlHciState++;
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;
        /* YASF_HOSTCMD_CFG_SET_IP_DESTINATION */
        case 66:
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: sending  YASF_HOSTCMD_CFG_SET_IP_DESTINATION\n"));
            Ind = 0;
            TmpBuffer[Ind++] = YASF_HOSTCMD_CFG_SET_IP_DESTINATION;
            TmpBuffer[Ind++] = 4 + 2;  // Length
            for( i=0; i<4; i++ )
                TmpBuffer[Ind++] = pAC->AsfData.Mib.IpDest[i];
            lRetCode = AsfHciSendData(pAC, IoC, TmpBuffer, 0, ASF_HCI_NOWAIT, 0 );
            pAC->AsfData.GlHciState++;
            break;
        case 67:
            lRetCode = AsfHciGetState( pAC );
            if( (lRetCode == HCI_EN_CMD_READY) || (lRetCode == HCI_EN_CMD_IDLE))
                pAC->AsfData.GlHciState++;
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;
        /* YASF_HOSTCMD_CFG_SET_IP_SOURCE */
        case 68:
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: sending  YASF_HOSTCMD_CFG_SET_IP_SOURCE\n"));
            Ind = 0;
            TmpBuffer[Ind++] = YASF_HOSTCMD_CFG_SET_IP_SOURCE;
            TmpBuffer[Ind++] = 4 + 2;  // Length
            for( i=0; i<4; i++ )
                TmpBuffer[Ind++] = pAC->IpAddr[i];
            lRetCode = AsfHciSendData(pAC, IoC, TmpBuffer, 0, ASF_HCI_NOWAIT, 0 );
            pAC->AsfData.GlHciState++;
            break;
        case 69:
            lRetCode = AsfHciGetState( pAC );
            if( (lRetCode == HCI_EN_CMD_READY) || (lRetCode == HCI_EN_CMD_IDLE))  {
                for( i=0; i<4; i++ )
                    pAC->AsfData.Mib.IpSource[i] = pAC->IpAddr[i];
                    #ifdef ASF_FW_ARP_RESOLVE
                        pAC->AsfData.GlHciState = 150;  //  do ARP resolve
                    #else
                        pAC->AsfData.GlHciState++;
                    #endif
            }
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;
        /* YASF_HOSTCMD_CFG_SET_MAC_DESTINATION */
        case 70:
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: sending  YASF_HOSTCMD_CFG_SET_MAC_DESTINATION\n"));
            Ind = 0;
            TmpBuffer[Ind++] = YASF_HOSTCMD_CFG_SET_MAC_DESTINATION;
            TmpBuffer[Ind++] = 6 + 2;  // Length
            for( i=0; i<6; i++ )
                TmpBuffer[Ind++] = pAC->AsfData.Mib.MacDest[i];
            lRetCode = AsfHciSendData(pAC, IoC, TmpBuffer, 0, ASF_HCI_NOWAIT, 0 );
            pAC->AsfData.GlHciState++;
            break;
        case 71:
            lRetCode = AsfHciGetState( pAC );
            if( (lRetCode == HCI_EN_CMD_READY) || (lRetCode == HCI_EN_CMD_IDLE))
                pAC->AsfData.GlHciState++;
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;
        /* YASF_HOSTCMD_CFG_SET_COMMUNITY_NAME */
        case 72:
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: sending  YASF_HOSTCMD_CFG_SET_COMMUNITY_NAME\n"));
            Ind = 0;
            TmpBuffer[Ind++] = YASF_HOSTCMD_CFG_SET_COMMUNITY_NAME;
            TmpBuffer[Ind++] = 2;  // Length
            for( i=0; i<64; i++ )  {
                TmpBuffer[1]++;
                TmpBuffer[Ind++] = pAC->AsfData.Mib.CommunityName[i];
                if( TmpBuffer[Ind-1] == 0 )
                    break;
            }
            lRetCode = AsfHciSendData(pAC, IoC, TmpBuffer, 0, ASF_HCI_NOWAIT, 0 );
            pAC->AsfData.GlHciState++;
            break;
        case 73:
            lRetCode = AsfHciGetState( pAC );
            if( (lRetCode == HCI_EN_CMD_READY) || (lRetCode == HCI_EN_CMD_IDLE))
                pAC->AsfData.GlHciState++;
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;
        /* YASF_HOSTCMD_CFG_SET_RSP_KEY_1 */
        case 74:
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: sending  YASF_HOSTCMD_CFG_SET_RSP_KEY_1\n >"));
            Ind = 0;
            TmpBuffer[Ind++] = YASF_HOSTCMD_CFG_SET_RSP_KEY_1;
            TmpBuffer[Ind++] = 20 + 2;  // Length
            for( i=0; i<20; i++ )  {
                TmpBuffer[Ind++] = pAC->AsfData.Mib.KeyOperator[i];
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("%02x",pAC->AsfData.Mib.KeyOperator[i] ));
            }
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("\n" ));
            lRetCode = AsfHciSendData(pAC, IoC, TmpBuffer, 0, ASF_HCI_NOWAIT, 0 );
            pAC->AsfData.GlHciState++;
            break;
        case 75:
            lRetCode = AsfHciGetState( pAC );
            if( (lRetCode == HCI_EN_CMD_READY) || (lRetCode == HCI_EN_CMD_IDLE))
                pAC->AsfData.GlHciState++;
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;
        /* YASF_HOSTCMD_CFG_SET_RSP_KEY_2 */
        case 76:
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: sending  YASF_HOSTCMD_CFG_SET_RSP_KEY_2\n >"));
            Ind = 0;
            TmpBuffer[Ind++] = YASF_HOSTCMD_CFG_SET_RSP_KEY_2;
            TmpBuffer[Ind++] = 20 + 2;  // Length
            for( i=0; i<20; i++ )  {
                TmpBuffer[Ind++] = pAC->AsfData.Mib.KeyAdministrator[i];
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("%02x",pAC->AsfData.Mib.KeyAdministrator[i] ));
            }
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("\n" ));
            lRetCode = AsfHciSendData(pAC, IoC, TmpBuffer, 0, ASF_HCI_NOWAIT, 0 );
            pAC->AsfData.GlHciState++;
            break;
        case 77:
            lRetCode = AsfHciGetState( pAC );
            if( (lRetCode == HCI_EN_CMD_READY) || (lRetCode == HCI_EN_CMD_IDLE))
                pAC->AsfData.GlHciState++;
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;
        /* YASF_HOSTCMD_CFG_SET_RSP_KEY_3 */
        case 78:
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: sending  YASF_HOSTCMD_CFG_SET_RSP_KEY_3\n >"));
            Ind = 0;
            TmpBuffer[Ind++] = YASF_HOSTCMD_CFG_SET_RSP_KEY_3;
            TmpBuffer[Ind++] = 20 + 2;  // Length
            for( i=0; i<20; i++ )  {
                TmpBuffer[Ind++] = pAC->AsfData.Mib.KeyGenerator[i];
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("%02x",pAC->AsfData.Mib.KeyGenerator[i] ));
            }
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("\n" ));
            lRetCode = AsfHciSendData(pAC, IoC, TmpBuffer, 0, ASF_HCI_NOWAIT, 0 );
            pAC->AsfData.GlHciState++;
            break;
        case 79:
            lRetCode = AsfHciGetState( pAC );
            if( (lRetCode == HCI_EN_CMD_READY) || (lRetCode == HCI_EN_CMD_IDLE)){
                if(pAC->AsfData.ChipMode == SK_GEASF_CHIP_Y2) {
                    pAC->AsfData.GlHciState++;
                }
                else {
                    pAC->AsfData.GlHciState+=2;
                }
            }
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;
        case 80:
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: sending  YASF_HOSTCMD_CFG_SET_ACTIVE_PORT\n"));
            Ind = 0;
            TmpBuffer[Ind++] = YASF_HOSTCMD_CFG_SET_ACTIVE_PORT;
            TmpBuffer[Ind++] = 1 + 3;  // Length
            TmpBuffer[Ind++] = pAC->AsfData.ActivePort;
            TmpBuffer[Ind++] = pAC->AsfData.ActivePort;     // Preferred port not set directly in RLMT
            lRetCode = AsfHciSendData(pAC, IoC, TmpBuffer, 0, ASF_HCI_NOWAIT, 0 );
            pAC->AsfData.GlHciState++;
            break;
        case 81:
            lRetCode = AsfHciGetState( pAC );
            if( (lRetCode == HCI_EN_CMD_READY) || (lRetCode == HCI_EN_CMD_IDLE))
                pAC->AsfData.GlHciState++;
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;
        case 82:
            lRetCode = AsfHciSendCommand(pAC, IoC, YASF_HOSTCMD_CFG_STORE_CONFIG, 0, 0, 0, ASF_HCI_NOWAIT, 0 );
            if( lRetCode == HCI_EN_CMD_READY )  {
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: sent  YASF_HOSTCMD_CFG_STORE_CONFIG\n"));
                pAC->AsfData.GlHciState = 100;
                //  check, whether ASF Enable has changed
                if( pAC->AsfData.Mib.ConfigChange & 0x01 )  {
                    // set/reset asf enable
                    if( pAC->AsfData.Mib.Ena )  {
                        AsfEnable(pAC, IoC );
                        /* Force initialization of the pattern ram
                         * (important for the very first enable of ASF)
                         */
                        pAC->AsfData.Mib.ConfigChange |= 0x02;
                    }
                    else  {
                        AsfDisable(pAC, IoC );
                    }
                }
                //  check, whether RSP Enable has changed
                if( pAC->AsfData.Mib.ConfigChange & 0x02 )  {
                    AsfSetUpPattern( pAC, IoC, 0 );     // Port 0
                    if(pAC->AsfData.ChipMode == SK_GEASF_CHIP_Y2) {
                        AsfSetUpPattern( pAC, IoC, 1 ); // Port 1
                    }
                }
                pAC->AsfData.Mib.ConfigChange = 0;
                pAC->AsfData.Mib.NewParam = 0;
            }
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;

        /*------------------------------------------------------------------------------------------------------
         *  Get ASF configuration data from FW (SEEPROM)
         *----------------------------------------------------------------------------------------------------*/

        case 100:
            lRetCode = AsfHciSendCommand(pAC, IoC, YASF_HOSTCMD_CFG_GET_ASF_ENABLE, 0, 0, 1, ASF_HCI_NOWAIT, 0 );
            if( lRetCode == HCI_EN_CMD_READY )  {
                // Fetch received data from HCI interface
                AsfHciGetData( pAC, &pHciRecBuf );
                pAC->AsfData.Mib.Ena = *(pHciRecBuf+2);
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: YASF_HOSTCMD_CFG_GET_ASF_ENABLE:0x%x \n",
                                                              pAC->AsfData.Mib.Ena ));
                pAC->AsfData.GlHciState++;
            }  //  if( lRetCode == HCI_EN_CMD_READY )
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;

        case 101:
            lRetCode = AsfHciSendCommand(pAC, IoC, YASF_HOSTCMD_CFG_GET_RSP_ENABLE, 0, 0, 1, ASF_HCI_NOWAIT, 0 );
            if( lRetCode == HCI_EN_CMD_READY )  {
                // Fetch received data from HCI interface
                AsfHciGetData( pAC, &pHciRecBuf );
                pAC->AsfData.Mib.RspEnable = *(pHciRecBuf+2);
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: YASF_HOSTCMD_CFG_GET_RSP_ENABLE:0x%x \n",
                                                              pAC->AsfData.Mib.RspEnable ));
                pAC->AsfData.GlHciState++;
            }  //  if( lRetCode == HCI_EN_CMD_READY )
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;

        case 102:
            lRetCode = AsfHciSendCommand(pAC, IoC, YASF_HOSTCMD_CFG_GET_RETRANS, 0, 0, 1, ASF_HCI_NOWAIT, 0 );
            if( lRetCode == HCI_EN_CMD_READY )  {
                // Fetch received data from HCI interface
                AsfHciGetData( pAC, &pHciRecBuf );
                pAC->AsfData.Mib.Retrans = *(pHciRecBuf+2);
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: YASF_HOSTCMD_CFG_GET_RETRANS:0x%x \n",
                                                              pAC->AsfData.Mib.Retrans ));
                pAC->AsfData.GlHciState++;
            }  //  if( lRetCode == HCI_EN_CMD_READY )
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;

        case 103:
            lRetCode = AsfHciSendCommand(pAC, IoC, YASF_HOSTCMD_CFG_GET_RETRANS_INT, 0, 0, 1, ASF_HCI_NOWAIT, 0 );
            if( lRetCode == HCI_EN_CMD_READY )  {
                // Fetch received data from HCI interface
                AsfHciGetData( pAC, &pHciRecBuf );
                pAC->AsfData.Mib.RetransInt  = ((SK_U32)*(pHciRecBuf+2)) << 8;
                pAC->AsfData.Mib.RetransInt |= ((SK_U32)*(pHciRecBuf+3)) << 0;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: YASF_HOSTCMD_CFG_GET_RETRANS_INT:0x%x \n",
                                                              pAC->AsfData.Mib.RetransInt ));
                pAC->AsfData.GlHciState++;
            }  //  if( lRetCode == HCI_EN_CMD_READY )
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;

        case 104:
            lRetCode = AsfHciSendCommand(pAC, IoC, YASF_HOSTCMD_CFG_GET_HB_ENABLE, 0, 0, 1, ASF_HCI_NOWAIT, 0 );
            if( lRetCode == HCI_EN_CMD_READY )  {
                // Fetch received data from HCI interface
                AsfHciGetData( pAC, &pHciRecBuf );
                pAC->AsfData.Mib.HbEna = *(pHciRecBuf+2);
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: YASF_HOSTCMD_CFG_GET_HB_ENABLE:0x%x \n",
                                                              pAC->AsfData.Mib.HbEna ));
                pAC->AsfData.GlHciState++;
            }  //  if( lRetCode == HCI_EN_CMD_READY )
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;

        case 105:
            lRetCode = AsfHciSendCommand(pAC, IoC, YASF_HOSTCMD_CFG_GET_HB_INT, 0, 0, 1, ASF_HCI_NOWAIT, 0 );
            if( lRetCode == HCI_EN_CMD_READY )  {
                // Fetch received data from HCI interface
                AsfHciGetData( pAC, &pHciRecBuf );
                pAC->AsfData.Mib.HbInt  = ((SK_U32)*(pHciRecBuf+2)) << 8;
                pAC->AsfData.Mib.HbInt |= ((SK_U32)*(pHciRecBuf+3)) << 0;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: YASF_HOSTCMD_CFG_GET_HB_INT:0x%x \n",
                                                              pAC->AsfData.Mib.HbInt));
                pAC->AsfData.GlHciState++;
            }  //  if( lRetCode == HCI_EN_CMD_READY )
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;

        case 106:
            lRetCode = AsfHciSendCommand(pAC, IoC, YASF_HOSTCMD_CFG_GET_DRWD_ENABLE, 0, 0, 1, ASF_HCI_NOWAIT, 0 );
            if( lRetCode == HCI_EN_CMD_READY )  {
                // Fetch received data from HCI interface
                AsfHciGetData( pAC, &pHciRecBuf );
                pAC->AsfData.Mib.WdEna = *(pHciRecBuf+2);
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: YASF_HOSTCMD_CFG_GET_DRWD_ENABLE:0x%x \n",
                                                              pAC->AsfData.Mib.WdEna ));
                pAC->AsfData.GlHciState++;
            }  //  if( lRetCode == HCI_EN_CMD_READY )
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;

        case 107:
            lRetCode = AsfHciSendCommand(pAC, IoC, YASF_HOSTCMD_CFG_GET_DRWD_INT, 0, 0, 1, ASF_HCI_NOWAIT, 0 );
            if( lRetCode == HCI_EN_CMD_READY )  {
                // Fetch received data from HCI interface
                AsfHciGetData( pAC, &pHciRecBuf );
                pAC->AsfData.Mib.WdTime  = ((SK_U32)*(pHciRecBuf+2)) << 8;
                pAC->AsfData.Mib.WdTime |= ((SK_U32)*(pHciRecBuf+3)) << 0;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: YASF_HOSTCMD_CFG_GET_DRWD_INT:0x%x \n",
                                                              pAC->AsfData.Mib.HbInt));
                pAC->AsfData.GlHciState++;
            }  //  if( lRetCode == HCI_EN_CMD_READY )
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;

        case 108:
            lRetCode = AsfHciSendCommand(pAC, IoC, YASF_HOSTCMD_CFG_GET_IP_DESTINATION, 0, 0, 1, ASF_HCI_NOWAIT, 0 );
            if( lRetCode == HCI_EN_CMD_READY )  {
                // Fetch received data from HCI interface
                AsfHciGetData( pAC, &pHciRecBuf );
                for( i=0; i<4; i++ )
                    pAC->AsfData.Mib.IpDest[i]  = *(pHciRecBuf+2+i);
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: YASF_HOSTCMD_CFG_GET_IP_DESTINATION %d.%d.%d.%d\n",
                        pAC->AsfData.Mib.IpDest[0], pAC->AsfData.Mib.IpDest[1],
                        pAC->AsfData.Mib.IpDest[2], pAC->AsfData.Mib.IpDest[3]));
                pAC->AsfData.GlHciState++;
            }  //  if( lRetCode == HCI_EN_CMD_READY )
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;

        case 109:
            lRetCode = AsfHciSendCommand(pAC, IoC, YASF_HOSTCMD_CFG_GET_IP_SOURCE, 0, 0, 1, ASF_HCI_NOWAIT, 0 );
            if( lRetCode == HCI_EN_CMD_READY )  {
                // Fetch received data from HCI interface
                AsfHciGetData( pAC, &pHciRecBuf );
                for( i=0; i<4; i++ )
                    pAC->AsfData.Mib.IpSource[i]  = *(pHciRecBuf+2+i);
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: YASF_HOSTCMD_CFG_GET_IP_SOURCE %d.%d.%d.%d\n",
                        pAC->AsfData.Mib.IpSource[0], pAC->AsfData.Mib.IpSource[1],
                        pAC->AsfData.Mib.IpSource[2], pAC->AsfData.Mib.IpSource[3]));
                pAC->AsfData.GlHciState++;
            }  //  if( lRetCode == HCI_EN_CMD_READY )
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;

        case 110:
            lRetCode = AsfHciSendCommand(pAC, IoC, YASF_HOSTCMD_CFG_GET_MAC_DESTINATION, 0, 0, 1, ASF_HCI_NOWAIT, 0 );
            if( lRetCode == HCI_EN_CMD_READY )  {
                // Fetch received data from HCI interface
                AsfHciGetData( pAC, &pHciRecBuf );
                for( i=0; i<6; i++ )
                    pAC->AsfData.Mib.MacDest[i]  = *(pHciRecBuf+2+i);
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,
                           ("SkAsfTimer: YASF_HOSTCMD_CFG_GET_MAC_DESTINATION %02x-%02x-%02x-%02x-%02x-%02x\n",
                       pAC->AsfData.Mib.MacDest[0], pAC->AsfData.Mib.MacDest[1], pAC->AsfData.Mib.MacDest[2],
                       pAC->AsfData.Mib.MacDest[3], pAC->AsfData.Mib.MacDest[4], pAC->AsfData.Mib.MacDest[5]));
                pAC->AsfData.GlHciState++;
            }  //  if( lRetCode == HCI_EN_CMD_READY )
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;

        case 111:
            lRetCode = AsfHciSendCommand(pAC, IoC, YASF_HOSTCMD_CFG_GET_COMMUNITY_NAME, 0, 0, 1, ASF_HCI_NOWAIT, 0 );
            if( lRetCode == HCI_EN_CMD_READY )  {
                // Fetch received data from HCI interface
                AsfHciGetData( pAC, &pHciRecBuf );
                Length = *(pHciRecBuf + 1);
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: YASF_HOSTCMD_CFG_GET_COMMUNITY_NAME l:%d >",Length))
                for( i=0; (i<63)&&(i<Length); i++ )  {
                    pAC->AsfData.Mib.CommunityName[i]  = *(pHciRecBuf+2+i);
                    pAC->AsfData.Mib.CommunityName[i+1]  = 0;
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("%c",pAC->AsfData.Mib.CommunityName[i]))
                }
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("<\n"))
                pAC->AsfData.GlHciState++;
            }  //  if( lRetCode == HCI_EN_CMD_READY )
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;

        case 112:
            lRetCode = AsfHciSendCommand(pAC, IoC, YASF_HOSTCMD_CFG_GET_RSP_KEY_1, 0, 0, 1, ASF_HCI_NOWAIT, 0 );
            if( lRetCode == HCI_EN_CMD_READY )  {
                // Fetch received data from HCI interface
                AsfHciGetData( pAC, &pHciRecBuf );
                Length = *(pHciRecBuf + 1);
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: YASF_HOSTCMD_CFG_GET_RSP_KEY_1: (%d)\n  >", Length));
                for( i=0; (i<20)&&(i<Length); i++ )  {
                    pAC->AsfData.Mib.KeyOperator[i]  = *(pHciRecBuf+2+i);
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("%02x", pAC->AsfData.Mib.KeyOperator[i]));
                }
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("<\n"));
                pAC->AsfData.GlHciState++;
            }  //  if( lRetCode == HCI_EN_CMD_READY )
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;

        case 113:
            lRetCode = AsfHciSendCommand(pAC, IoC, YASF_HOSTCMD_CFG_GET_RSP_KEY_2, 0, 0, 1, ASF_HCI_NOWAIT, 0 );
            if( lRetCode == HCI_EN_CMD_READY )  {
                // Fetch received data from HCI interface
                AsfHciGetData( pAC, &pHciRecBuf );
                Length = *(pHciRecBuf + 1);
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: YASF_HOSTCMD_CFG_GET_RSP_KEY_2: (%d)\n  >", Length));
                for( i=0; (i<20)&&(i<Length); i++ )  {
                    pAC->AsfData.Mib.KeyAdministrator[i]  = *(pHciRecBuf+2+i);
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("%02x", pAC->AsfData.Mib.KeyAdministrator[i]));
                }
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("<\n"));
                pAC->AsfData.GlHciState++;
            }  //  if( lRetCode == HCI_EN_CMD_READY )
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;

        case 114:
            lRetCode = AsfHciSendCommand(pAC, IoC, YASF_HOSTCMD_CFG_GET_RSP_KEY_3, 0, 0, 1, ASF_HCI_NOWAIT, 0 );
            if( lRetCode == HCI_EN_CMD_READY )  {
                // Fetch received data from HCI interface
                AsfHciGetData( pAC, &pHciRecBuf );
                Length = *(pHciRecBuf + 1);
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: YASF_HOSTCMD_CFG_GET_RSP_KEY_3: (%d)\n  >", Length));
                for( i=0; (i<20)&&(i<Length); i++ )  {
                    pAC->AsfData.Mib.KeyGenerator[i]  = *(pHciRecBuf+2+i);
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("%02x", pAC->AsfData.Mib.KeyGenerator[i]));
                }
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("<\n"));
                pAC->AsfData.GlHciState++;
            }  //  if( lRetCode == HCI_EN_CMD_READY )
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;

        case 115:
            lRetCode = AsfHciSendCommand(pAC, IoC, YASF_HOSTCMD_CFG_GET_FW_VERSION_STRING, 0, 0, 1, ASF_HCI_NOWAIT, 0 );
            if( lRetCode == HCI_EN_CMD_READY )  {
                // Fetch received data from HCI interface
                AsfHciGetData( pAC, &pHciRecBuf );
                Length = *(pHciRecBuf + 1);
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: YASF_HOSTCMD_CFG_GET_FW_VERSION_STRING l:%d\n",Length))
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("         >"))
                for( i=0; (i<79)&&(i<Length); i++ )  {
                    pAC->AsfData.FwVersionString[i]   = *(pHciRecBuf+2+i);
                    pAC->AsfData.FwVersionString[i+1] = 0;
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("%c",pAC->AsfData.FwVersionString[i]))
                }
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("<\n"))
                pAC->AsfData.GlHciState++;
            }  //  if( lRetCode == HCI_EN_CMD_READY )
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;

        case 116:
            // get SMBus infos from FW

            // do not ask twice
            pAC->AsfData.Mib.SMBus.UpdateReq = 0;

            lRetCode = AsfHciSendCommand(pAC, IoC, YASF_HOSTCMD_CFG_GET_SMBUS_INFOS, 0, 0, 1, ASF_HCI_NOWAIT, 0 );

            if( lRetCode == HCI_EN_CMD_READY )  {
                // Fetch received data from HCI interface
                AsfHciGetData( pAC, &pHciRecBuf );
                Length = *(pHciRecBuf + 1);
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: YASF_HOSTCMD_CFG_GET_SMBUS_INFOS l:%d\n", Length))

                pAC->AsfData.Mib.SMBus.length = 0;
                for( i=0; (i<ASF_SMBUS_MAXBUFFLENGTH)&&(i<Length); i++ )  {
                    pAC->AsfData.Mib.SMBus.length   += 1;
                    pAC->AsfData.Mib.SMBus.buffer[i] = *(pHciRecBuf+2+i);
                    if ( (i%16) != 0) {
                        SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,(" 0x%02x", pAC->AsfData.Mib.SMBus.buffer[i]))
                    }
                    else {
                        SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("\n         > 0x%02x", pAC->AsfData.Mib.SMBus.buffer[i]))
                    }
                }
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("\n\n"))
                if(pAC->AsfData.ChipMode == SK_GEASF_CHIP_Y2) {
                    pAC->AsfData.GlHciState++;
                }
                else {
                    pAC->AsfData.GlHciState+=2;
                }
            }  //  if( lRetCode == HCI_EN_CMD_READY )

            if( lRetCode == HCI_EN_CMD_ERROR ) {
                pAC->AsfData.GlHciState = 255;
            }
            break;

        case 117:
            lRetCode = AsfHciSendCommand(pAC, IoC, YASF_HOSTCMD_CFG_GET_ACTIVE_PORT, 0, 0, 1, ASF_HCI_NOWAIT, 0 );
            if( lRetCode == HCI_EN_CMD_READY )  {
                // Fetch received data from HCI interface
                AsfHciGetData( pAC, &pHciRecBuf );
                Length = *(pHciRecBuf + 1);
                pAC->AsfData.ActivePort = *(pHciRecBuf + 2);
                pAC->AsfData.PrefPort   = *(pHciRecBuf + 3);
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,
                    ("SkAsfTimer: YASF_HOSTCMD_CFG_GET_ACTIVE_PORT: %d %d\n  >",
                    pAC->AsfData.ActivePort, pAC->AsfData.PrefPort));
                pAC->AsfData.GlHciState++;
            }  //  if( lRetCode == HCI_EN_CMD_READY )
            if( lRetCode == HCI_EN_CMD_ERROR )
                pAC->AsfData.GlHciState = 255;
            break;

        case 118:
            if (pAC->AsfData.Mib.PattUpReq != 0)   {
                pAC->AsfData.Mib.PattUpReq = 0;

                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,
                    ("SkAsfTimer: update pattern in state %d with rsp %d\n",
                    pAC->AsfData.GlHciState, pAC->AsfData.Mib.RspEnable));

                AsfSetUpPattern(pAC, IoC, 0);       // Port 0
                if(pAC->AsfData.ChipMode == SK_GEASF_CHIP_Y2) {
                    AsfSetUpPattern(pAC, IoC, 1);   // Port 1
                }
            }
            pAC->AsfData.GlHciState = 254;
            break;

        /*------------------------------------------------------------------------------------------------------
         *  ARP resolve
         *----------------------------------------------------------------------------------------------------*/

        case 150:

#ifdef ASF_FW_ARP_RESOLVE
            if (pAC->AsfData.OpMode == SK_GEASF_MODE_ASF) {

                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: sending  ARP REQUEST\n"));
                Ind = 0;
                TmpBuffer[Ind++] = YASF_HOSTCMD_ARP_RESOLVE;
                TmpBuffer[Ind++] = 4 + 2;  // Length
                for( i=0; i<4; i++ )
                    TmpBuffer[Ind++] = pAC->AsfData.Mib.IpDest[i];
                AsfSetUpPattern( pAC, IoC, pAC->AsfData.ActivePort );  //  prepare pattern logik for ARP handling
                YlciEnablePattern ( pAC, IoC, pAC->AsfData.ActivePort, 5 );  //  enable the ARP pattern
                AsfEnable(pAC, IoC );  //  enable ASF logik for receiving ARP response
                lRetCode = AsfHciSendData(pAC, IoC, TmpBuffer, 1, ASF_HCI_NOWAIT, 0 );
                if( lRetCode == HCI_EN_CMD_READY )  {
                    YlciDisablePattern ( pAC, IoC, pAC->AsfData.ActivePort, 5 );  //  disable the ARP pattern
                    if( pAC->AsfData.Mib.Ena == 0 )
                        AsfDisable(pAC, IoC );
                    // Fetch received data from HCI interface
                    AsfHciGetData( pAC, &pHciRecBuf );
                    for( i=0; i<6; i++ )
                        pAC->AsfData.Mib.MacDest[i]  = *(pHciRecBuf+2+i);

                    if( (pAC->AsfData.Mib.MacDest[0] == 0) && (pAC->AsfData.Mib.MacDest[1] == 0) &&
                        (pAC->AsfData.Mib.MacDest[2] == 0) && (pAC->AsfData.Mib.MacDest[3] == 0) &&
                        (pAC->AsfData.Mib.MacDest[4] == 0) && (pAC->AsfData.Mib.MacDest[5] == 0)    )  {
                        SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,
                                   ("SkAsfTimer: YASF_HOSTCMD_ARP_RESOLVE     NOT RESOLVED !!\n"));
                    }
                    else  {
                        SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,
                                   ("SkAsfTimer: YASF_HOSTCMD_ARP_RESOLVE %02x-%02x-%02x-%02x-%02x-%02x\n",
                               pAC->AsfData.Mib.MacDest[0], pAC->AsfData.Mib.MacDest[1], pAC->AsfData.Mib.MacDest[2],
                               pAC->AsfData.Mib.MacDest[3], pAC->AsfData.Mib.MacDest[4], pAC->AsfData.Mib.MacDest[5]));

                    }

                    pAC->AsfData.GlHciState++;
                }  //  if( lRetCode == HCI_EN_CMD_READY )
                if( lRetCode == HCI_EN_CMD_ERROR )  {
                    pAC->AsfData.GlHciState = 255;
                    YlciDisablePattern ( pAC, IoC, pAC->AsfData.ActivePort, 5 );    //  disable the ARP pattern
                    if( pAC->AsfData.Mib.Ena == 0 )
                        AsfDisable(pAC, IoC );
                }
            }
            else {
                pAC->AsfData.GlHciState++;
            }

#else   //  ASF_FW_ARP_RESOLVE

            pAC->AsfData.GlHciState++;

#endif
            break;

        case 151:
            pAC->AsfData.GlHciState = 70;
            break;

        /*------------------------------------------------------------------------------------------------------
         *  Finish state
         *----------------------------------------------------------------------------------------------------*/
        case 254:
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: ***** Finish *****\n"));
            pAC->AsfData.FwError = 0;
            pAC->AsfData.GlHciState = 0;
            pAC->AsfData.Mib.NewParam = 0;
            pAC->AsfData.LastGlHciState = pAC->AsfData.GlHciState;
            break;

        /*------------------------------------------------------------------------------------------------------
         *  Error handling
         *----------------------------------------------------------------------------------------------------*/
        case 255:
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: ***** ERROR *****\n"));
            pAC->AsfData.FwError++;
            if(pAC->AsfData.FwError > 2) {
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("***** CPU RESET *****\n"));
                AsfSmartResetCpu( pAC, IoC, ASF_RESET_HOT );
                AsfRunCpu( pAC, IoC );
            }
            pAC->AsfData.LastGlHciState = pAC->AsfData.GlHciState; // Store Error State
            pAC->AsfData.GlHciState = 0;
            pAC->AsfData.Mib.NewParam = 0;
            pAC->AsfData.Hci.Status = HCI_EN_CMD_IDLE;
            break;
    }  //   switch(  &pAC->AsfData.GlHciState == 0  )  {

    if( pAC->AsfData.GlHciState != pAC->AsfData.LastGlHciState )  {
        SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfTimer: New State %d -> %d\n", pAC->AsfData.LastGlHciState, pAC->AsfData.GlHciState));
        if( pAC->AsfData.LastGlHciState != 255 ) { // Don't overwrite Error State, it will be checked in case 0
            pAC->AsfData.LastGlHciState = pAC->AsfData.GlHciState;
        }
    }

    if( pAC->AsfData.GlHciState == 0 )  {  //  idle
        SK_MEMSET((char *)&EventParam, 0, sizeof(EventParam));
        SkTimerStart(pAC, IoC, &pAC->AsfData.AsfTimer,
                     1000000, SKGE_ASF, SK_ASF_EVT_TIMER_EXPIRED,
                     EventParam);
    }
    else  {
        SK_MEMSET((char *)&EventParam, 0, sizeof(EventParam));
        SkTimerStart(pAC, IoC, &pAC->AsfData.AsfTimer,
                     30000, SKGE_ASF, SK_ASF_EVT_TIMER_EXPIRED,
                     EventParam);
    }

    return;
}

#if (0)
/*****************************************************************************
*
* AsfReadConfiguration
*
* Description:
*
* Returns:
*
*/
NDIS_STATUS AsfReadConfiguration(
IN SK_AC *pAc,
IN NDIS_HANDLE ConfigHandle,
IN NDIS_HANDLE WrapperConfigContext)
{
    SK_DBG_MSG(pAc, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,
    ("AsfReadConfiguration (No data loaded !)\n" ));

    return (NDIS_STATUS_SUCCESS);
}
#endif


/*****************************************************************************
*
* SkAsfShowMib
*
* Description:
*
* Returns:
*
*/
void SkAsfShowMib( SK_AC *pAC )  {

    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("ASF MIB (h):\n") );
    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("------------\n") );
    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("   Ena:            %d\n", pAC->AsfData.Mib.Ena ) );
    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("   Retrans:        %d Int:%d\n", pAC->AsfData.Mib.Retrans,
                                                                                    pAC->AsfData.Mib.RetransInt ) );
    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("   HbEna:          %d Int:%d\n", pAC->AsfData.Mib.HbEna,
                                                                                    pAC->AsfData.Mib.HbInt ) );
    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("   WdEna:          %d Time:%d\n", pAC->AsfData.Mib.WdEna,
                                                                                    pAC->AsfData.Mib.WdTime ) );
    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("   IpDest (d):     %d.%d.%d.%d\n",
                                                    pAC->AsfData.Mib.IpDest[0], pAC->AsfData.Mib.IpDest[1],
                                                    pAC->AsfData.Mib.IpDest[2], pAC->AsfData.Mib.IpDest[3] ) );
    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("   IpSource (d):   %d.%d.%d.%d\n",
                                                    pAC->AsfData.Mib.IpSource[0], pAC->AsfData.Mib.IpSource[1],
                                                    pAC->AsfData.Mib.IpSource[2], pAC->AsfData.Mib.IpSource[3] ) );
    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("   MacDest:        %02x:%02x:%02x:%02x:%02x:%02x\n",
                                                    pAC->AsfData.Mib.MacDest[0], pAC->AsfData.Mib.MacDest[1],
                                                    pAC->AsfData.Mib.MacDest[2], pAC->AsfData.Mib.MacDest[3],
                                                    pAC->AsfData.Mib.MacDest[4], pAC->AsfData.Mib.MacDest[5] ) );
    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("   MacSource:      %02x:%02x:%02x:%02x:%02x:%02x\n",
                                                    pAC->AsfData.Mib.MacSource[0], pAC->AsfData.Mib.MacSource[1],
                                                    pAC->AsfData.Mib.MacSource[2], pAC->AsfData.Mib.MacSource[3],
                                                    pAC->AsfData.Mib.MacSource[4], pAC->AsfData.Mib.MacSource[5] ) );
    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("   CommunityName:  >%s<\n", pAC->AsfData.Mib.CommunityName ) );
}

/*****************************************************************************
*
* AsfCpuState
*
* Description:
*
* Returns:
*
*/
SK_U8 AsfCpuState( SK_AC *pAC, SK_IOC IoC )  {
  SK_U32 TmpVal32;
  
  SK_IN32( IoC, REG_ASF_STATUS_CMD, &TmpVal32 );
  
  if( pAC->AsfData.ChipMode == SK_GEASF_CHIP_EX )  {
    if( (TmpVal32&(BIT_0 | BIT_1)) == 0 ) {
      return( ASF_CPU_STATE_RESET );
    }
    else  if ( (TmpVal32&(BIT_0 | BIT_1)) == 1 ) {
      return( ASF_CPU_STATE_RUNNING );
    }
    else  {
      return( ASF_CPU_STATE_UNKNOWN );
    }
  }
  else  {
    if( TmpVal32 & BIT_3 )
      return( ASF_CPU_STATE_RESET );
    if( TmpVal32 & BIT_2 )
      return( ASF_CPU_STATE_RUNNING );
  }
  
  return( ASF_CPU_STATE_UNKNOWN );
}

/*****************************************************************************
*
* AsfResetCpu
*
* Description:
*
* Returns:
*
*/
void AsfResetCpu(SK_AC *pAC, SK_IOC IoC)  {
    SK_U32 TmpVal32;

    if( pAC->AsfData.ChipMode == SK_GEASF_CHIP_EX )  {
      SK_IN32( IoC, REG_ASF_STATUS_CMD, &TmpVal32 );
      TmpVal32 &= ~(BIT_0 | BIT_1);
      SK_OUT32( IoC, REG_ASF_STATUS_CMD, TmpVal32 );
    }
    else  {
      SK_IN32( IoC, REG_ASF_STATUS_CMD, &TmpVal32 );
      TmpVal32 &= ~(BIT_2 | BIT_3);
      TmpVal32 |= BIT_3;
      SK_OUT32( IoC, REG_ASF_STATUS_CMD, TmpVal32 );
    }
}

/*****************************************************************************
*
* AsfSmartResetCpu
*
* Description:
*
* Returns:
*
*/
SK_U8 AsfSmartResetCpu( SK_AC *pAC, SK_IOC IoC, SK_U8 Cold )  {
SK_U64 StartTime, CurrTime;
SK_U8   ResetCommand;

    if( Cold )
        ResetCommand = YASF_HOSTCMD_RESET_COLD;
    else
        ResetCommand = YASF_HOSTCMD_RESET;

    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("AsfSmartResetCpu -> \n"));

    //  check, whether cpu is already in reset state...
    if( AsfCpuState( pAC, IoC ) != ASF_CPU_STATE_RESET )  {
        // ...if not, try SmartReset
        SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("AsfSmartResetCpu -> Send reset command\n"));
        if( AsfHciSendCommand( pAC, IoC, ResetCommand, 0, 0, 0, ASF_HCI_WAIT, 2 ) != HCI_EN_CMD_READY )  {
          //  if Smart Reset fails, do hard reset
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("AsfSmartResetCpu -> Do hard reset\n"));
            AsfResetCpu( pAC, IoC );
        }
        StartTime = SkOsGetTime(pAC);
        while( AsfCpuState( pAC, IoC ) != ASF_CPU_STATE_RESET ) {
            CurrTime = SkOsGetTime(pAC);
            if ( (CurrTime - StartTime) > (SK_TICKS_PER_SEC*1) )  {
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("AsfSmartResetCpu -> Error: CPU is not in reset state - Do hard reset\n"));
                AsfResetCpu( pAC, IoC );
                break;
            }
        }  //  while( AsfCpuState( IoC ) != ASF_CPU_STATE_RESET ) {

        if( AsfCpuState( pAC, IoC ) == ASF_CPU_STATE_RESET )
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("AsfSmartResetCpu -> CPU is in reset state\n"));

    }  //  if( AsfCpuState( IoC ) != ASF_CPU_STATE_RESET )  {
    else  {
        SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("AsfSmartResetCpu -> Cpu already in reset state\n"));
    }

    return( 1 );
}

/*****************************************************************************
*
* AsfSmartResetStateCpu
*
* Description:
*
* Returns:
*
*/
SK_U8 AsfSmartResetStateCpu( SK_AC *pAC, SK_IOC IoC )  {

    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("Send AsfSmartResetStateCpu\n"));
    return( AsfHciSendCommand( pAC, IoC, YASF_HOSTCMD_RESET_STATE, 0, 0, 0, ASF_HCI_WAIT, 3 ) );
}

/*****************************************************************************
*
* AsfRunCpu
*
* Description:
*
* Returns:
*
*/
void AsfRunCpu( SK_AC *pAC, SK_IOC IoC )  {
  SK_U32 TmpVal32;
  
  if( pAC->AsfData.ChipMode == SK_GEASF_CHIP_EX )  {
    SK_IN32( IoC, REG_ASF_STATUS_CMD, &TmpVal32 );
    TmpVal32 |= BIT_0;  // Set to running state
    SK_OUT32( IoC, REG_ASF_STATUS_CMD, TmpVal32 );
  }
  else  {
    SK_IN32( IoC, REG_ASF_STATUS_CMD, &TmpVal32 );
    TmpVal32 &= ~(BIT_2 | BIT_3);
    TmpVal32 |= BIT_3;  // Set to Reset state (Clk running)
    SK_OUT32( IoC, REG_ASF_STATUS_CMD, TmpVal32 );
    TmpVal32 &= ~(BIT_2 | BIT_3);
    TmpVal32 |= BIT_2;  // Set to running state
    SK_OUT32( IoC, REG_ASF_STATUS_CMD, TmpVal32 );
  }
}


/*****************************************************************************
*
* AsfCheckAliveCpu
*
* Description:
*
* Returns:
*
*/
SK_U8 AsfCheckAliveCpu( SK_AC *pAC, SK_IOC IoC )  {
SK_U8 Alive;

    //SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("AsfCheckAliveCpu -> \n"));
    Alive = 0;  //  Not alive
    //  check, whether cpu is in reset state...
    if( AsfCpuState( pAC, IoC ) != ASF_CPU_STATE_RESET )  {
        // SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("AsfSmartResetCpu -> Send Check Alive command\n"));
        if( AsfHciSendCommand( pAC, IoC, YASF_HOSTCMD_CHECK_ALIVE, 0, 0, 0, ASF_HCI_WAIT, 2 ) != HCI_EN_CMD_READY )  {
          //  if Smart Reset fails, do hard reset
            // SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("AsfCheckAliveCpu -> Not alive\n"));
        }
        else
            Alive = 1;
    }  //  if( AsfCpuState( IoC ) != ASF_CPU_STATE_RESET )  {
    else  {
        SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("AsfCheckAliveCpu -> *** CPU is in reset state ***\n"));
    }

    return( Alive );
}

/*****************************************************************************
*
* AsfSetOsPresentBit
*
* Description:
*
* Returns:
*
*/
void AsfSetOsPresentBit( SK_AC *pAC, SK_IOC IoC )  {
    SK_U32 TmpVal32;
	
    SK_IN32( IoC, REG_ASF_STATUS_CMD, &TmpVal32 );
    if( pAC->AsfData.ChipMode == SK_GEASF_CHIP_EX )
        TmpVal32 |= BIT_2;
	else
        TmpVal32 |= BIT_4;
    SK_OUT32( IoC, REG_ASF_STATUS_CMD, TmpVal32 );	
}

/*****************************************************************************
*
* AsfResetOsPresentBit
*
* Description:
*
* Returns:
*
*/
void AsfResetOsPresentBit( SK_AC *pAC, SK_IOC IoC )  {
    SK_U32 TmpVal32;

    SK_IN32( IoC, REG_ASF_STATUS_CMD, &TmpVal32 );
    if( pAC->AsfData.ChipMode == SK_GEASF_CHIP_EX )
	    TmpVal32 &= ~BIT_2;
	else
	    TmpVal32 &= ~BIT_4;
    SK_OUT32( IoC, REG_ASF_STATUS_CMD, TmpVal32 );	
}

/*****************************************************************************
*
* AsfEnableFlushFifo
*
* Description:
*
* Returns:
*
*/
void AsfEnableFlushFifo( SK_AC *pAC, SK_IOC IoC )  {
    SK_U32 TmpVal32;

    //ADD
    return;

#ifdef CHIP_ID_YUKON_EX	
    SK_IN32( IoC, RXMF_TCTL, &TmpVal32 );
	TmpVal32 &= ~(BIT_23 | BIT_22);
	TmpVal32 |= BIT_23;
    SK_OUT32( IoC, RXMF_TCTL, TmpVal32 );	
#endif

}

/*****************************************************************************
*
* AsfDisableFlushFifo
*
* Description:
*
* Returns:
*
*/
void AsfDisableFlushFifo( SK_AC *pAC , SK_IOC IoC)  {
    SK_U32 TmpVal32;

#ifdef CHIP_ID_YUKON_EX	
    SK_IN32( IoC, RXMF_TCTL, &TmpVal32 );
	TmpVal32 &= ~(BIT_23 | BIT_22);
	TmpVal32 |= BIT_22;
    SK_OUT32( IoC, RXMF_TCTL, TmpVal32 );	
#endif

}

/*****************************************************************************
*
* AsfLockSpi
*
* Description:
*
* Returns:
*
*/
void AsfLockSpi( SK_AC *pAC, SK_IOC IoC )  {
    SK_U8 TmpVal8;

    SK_IN8( IoC, GPHY_CTRL + 2, &TmpVal8);
    TmpVal8 |= BIT_0;
    SK_OUT8( IoC, GPHY_CTRL + 2, TmpVal8);
}

/*****************************************************************************
*
* AsfUnlockSpi
*
* Description:
*
* Returns:
*
*/
void AsfUnlockSpi( SK_AC *pAC, SK_IOC IoC )  {
    SK_U8 TmpVal8;

    SK_IN8( IoC, GPHY_CTRL + 2, &TmpVal8);
    TmpVal8 &= ~BIT_0;
    SK_OUT8( IoC, GPHY_CTRL + 2, TmpVal8);
}

/*****************************************************************************
*
* SkAsfHci
*
* Description:
*
* Returns:
*
*/
void SkAsfHci(
SK_AC *pAC, /* Pointer to adapter context */
SK_IOC IoC,
SK_U8   ToEna ) /* IO context handle */  {

    SK_EVPARA   EventParam;
    SK_U32      Cmd;
    SK_U32      SendValue;
    SK_U32      RecValue;

    if (pAC->AsfData.Hci.Status == HCI_EN_CMD_IDLE)
        return;

    pAC->AsfData.Hci.Cycles++;

    // Check, whether there is something to do
    SK_IN32( IoC, ASF_HCI_CMDREG, &Cmd );
    if( pAC->AsfData.Hci.OldCmdReg != Cmd )  {
        pAC->AsfData.Hci.OldCmdReg = Cmd;
        // SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfHci CmdReg: 0x%x\n", Cmd ));
    }

    // SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("                    CmdReg: 0x%x\n", Cmd ));

    if ((Cmd & (ASF_HCI_READ | ASF_HCI_WRITE | ASF_HCI_CMD_RD_READY | ASF_HCI_CMD_WR_READY | ASF_HCI_UNSUCCESS)) == 0)
    {
        // SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfHci State: 0x%x\n", pAC->AsfData.Hci.Status ));
        switch( pAC->AsfData.Hci.Status )  {

            case HCI_EN_CMD_WRITING:
                SendValue = 0;
                if (pAC->AsfData.Hci.SendLength > 0)
                {
                    pAC->AsfData.Hci.SendLength--;
                    SendValue = ((SK_U32)pAC->AsfData.Hci.TransmitBuf[pAC->AsfData.Hci.SendIndex +0]) << 24;
                    if (pAC->AsfData.Hci.SendLength > 0)
                    {
                        pAC->AsfData.Hci.SendLength--;
                        SendValue += ((SK_U32)pAC->AsfData.Hci.TransmitBuf[pAC->AsfData.Hci.SendIndex +1]) << 16;
                        if (pAC->AsfData.Hci.SendLength > 0)
                        {
                            pAC->AsfData.Hci.SendLength--;
                            SendValue += ((SK_U32)pAC->AsfData.Hci.TransmitBuf[pAC->AsfData.Hci.SendIndex +2]) << 8;
                            if (pAC->AsfData.Hci.SendLength > 0)
                            {
                                pAC->AsfData.Hci.SendLength--;
                                SendValue += ((SK_U32)pAC->AsfData.Hci.TransmitBuf[pAC->AsfData.Hci.SendIndex +3]) << 0;
                            }
                        }
                    }
                }

                // SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("        SendValue: 0x%x l:%d\n",
                //                                             SendValue, pAC->AsfData.Hci.SendLength ));
                SK_OUT32( IoC, ASF_HCI_DATAREG, SendValue);
                if (pAC->AsfData.Hci.SendLength == 0)
                {
                    SK_OUT32( IoC, ASF_HCI_CMDREG, ASF_HCI_WRITE | ASF_HCI_CMD_WR_READY | ASF_HCI_UNSUCCESS | (pAC->AsfData.Hci.SendIndex/4));
                    pAC->AsfData.Hci.Status = HCI_EN_CMD_WAIT;
                }
                else
                {
                    SK_OUT32( IoC, ASF_HCI_CMDREG, ASF_HCI_WRITE | ASF_HCI_UNSUCCESS | (pAC->AsfData.Hci.SendIndex/4));
                    pAC->AsfData.Hci.SendIndex += 4;
                }
                break;

            case HCI_EN_CMD_WAIT:
                if( pAC->AsfData.Hci.ExpectResponse )  {
                    pAC->AsfData.Hci.Status = HCI_EN_CMD_READING;
                    pAC->AsfData.Hci.ReceiveIndex = 0;
                    // SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("        Wait for response\n" ));
                    SK_OUT32( IoC, ASF_HCI_CMDREG, ASF_HCI_READ | ASF_HCI_UNSUCCESS | (pAC->AsfData.Hci.ReceiveIndex/4));
                }
                else  {
                    pAC->AsfData.Hci.Status = HCI_EN_CMD_READY;
                    pAC->AsfData.Hci.ReceiveBuf[1] = 0;  /*Length*/
                }
                break;

            case HCI_EN_CMD_READING:
                SK_IN32( IoC, ASF_HCI_DATAREG, &RecValue );
                // SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("        RecValue: 0x%x\n", RecValue ));
                pAC->AsfData.Hci.ReceiveBuf[pAC->AsfData.Hci.ReceiveIndex+3] = (SK_U8) (RecValue >>  0) & 0x000000ff;
                pAC->AsfData.Hci.ReceiveBuf[pAC->AsfData.Hci.ReceiveIndex+2] = (SK_U8) (RecValue >>  8) & 0x000000ff;
                pAC->AsfData.Hci.ReceiveBuf[pAC->AsfData.Hci.ReceiveIndex+1] = (SK_U8) (RecValue >> 16) & 0x000000ff;
                pAC->AsfData.Hci.ReceiveBuf[pAC->AsfData.Hci.ReceiveIndex+0] = (SK_U8) (RecValue >> 24) & 0x000000ff;
                pAC->AsfData.Hci.ReceiveIndex   += 4;
                //SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("        RecValue: 0x%x RecIndex:%d l:%d\n",
                //                          RecValue,pAC->AsfData.Hci.ReceiveIndex, pAC->AsfData.Hci.ReceiveBuf[1] ));
                if( pAC->AsfData.Hci.ReceiveBuf[1] > pAC->AsfData.Hci.ReceiveIndex )  { /* check length*/
                    SK_OUT32( IoC, ASF_HCI_CMDREG, ASF_HCI_READ | ASF_HCI_UNSUCCESS | (pAC->AsfData.Hci.ReceiveIndex/4));
                }
                else  {
                    SK_OUT32( IoC, ASF_HCI_CMDREG, ASF_HCI_CMD_RD_READY );
                    pAC->AsfData.Hci.Status = HCI_EN_CMD_READY;
                }
                break;

            case HCI_EN_CMD_READY:
                break;

        }  //  switch( pAC->AsfData.Hci.Status )  {

        pAC->AsfData.Hci.To = 0;

    }  //  if ((Cmd & (ASF_HCI_READ | ASF_HCI_WRITE | ASF_HCI_CMD_RD_READY | ASF_HCI_CMD_WR_READY)) == 0)
    else  {

        if( ToEna )
            pAC->AsfData.Hci.To++;
        /*  Timeout ! */
        if( pAC->AsfData.Hci.To > ASF_HCI_TO )  {
            pAC->AsfData.Hci.Status = HCI_EN_CMD_ERROR;
        }
    }

    if( ToEna )  {
        if( pAC->AsfData.Hci.Status != HCI_EN_CMD_IDLE &&
            pAC->AsfData.Hci.Status != HCI_EN_CMD_READY &&
            pAC->AsfData.Hci.Status != HCI_EN_CMD_ERROR ) {
            SK_MEMSET((char *)&EventParam, 0, sizeof(EventParam));
            SkTimerStart(pAC, IoC, &pAC->AsfData.Hci.AsfTimerHci,  /*  10ms */
                         10000, SKGE_ASF, SK_ASF_EVT_TIMER_HCI_EXPIRED,
                         EventParam);
        }
    }

    return;
}


/*****************************************************************************
*
* AsfHciGetData
*
* Description:
*
* Returns:
*
*/
SK_U8 AsfHciGetData( SK_AC  *pAC, SK_U8 **pHciRecBuf )  {
    *pHciRecBuf = pAC->AsfData.Hci.ReceiveBuf;
    return( 1 );
}

/*****************************************************************************
*
* AsfHciGetState
*
* Description:
*
* Returns:
*
*/
SK_U8   AsfHciGetState( SK_AC *pAC )  {
    SK_U8   Stat;
    SK_U64  DiffTime;

    Stat = pAC->AsfData.Hci.Status;
    if( Stat == HCI_EN_CMD_READY )  {
        pAC->AsfData.Hci.Status = HCI_EN_CMD_IDLE;
        DiffTime = SkOsGetTime(pAC) - pAC->AsfData.Hci.Time;
//        SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    *** Cycles:%d Time:%lums\n",
//             pAC->AsfData.Hci.Cycles, DiffTime / (10*1000) ));
    }
    return( Stat );
}


/*****************************************************************************
*
* AsfHciSendCommand
*
* Description:
*
* Returns:
*
*/
SK_U8   AsfHciSendCommand(
SK_AC   *pAC,
SK_IOC  IoC,
SK_U8   Command,
SK_U8   Par1,
SK_U8   Par2,
SK_U8   ExpectResponse,
SK_U8   Wait,
SK_U8   Retry )  {
    SK_U8   RetCode;
    SK_U8   Message[4];
    SK_U64  StartTime;
    SK_U64  CurrTime;
    SK_U64  TmpTime;

    do {
        if( pAC->AsfData.Hci.Status == HCI_EN_CMD_IDLE )  {
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    Send Command cmd:0x%x p1:0x%x p2:0x%x wait:%d Retry:%d\n",
                                                           Command, Par1, Par2, Wait, Retry ));
            Message[0] = Command;
            Message[1] = 2;  //  Length
            Message[2] = Par1;
            Message[3] = Par2;
            RetCode = AsfHciSendMessage(pAC, IoC, Message, 4, ExpectResponse, Wait );
        }

        if( Wait )  {
            StartTime = SkOsGetTime(pAC);

            TmpTime = StartTime + 1;
            do {
                CurrTime = SkOsGetTime(pAC);
                if( CurrTime > TmpTime )  {
		  SkAsfHci( pAC, IoC, 0 );

                    TmpTime = CurrTime + 1;
                }
                RetCode = AsfHciGetState( pAC );

                if  ( (CurrTime - StartTime) > (SK_TICKS_PER_SEC*1) ) {
                    RetCode = HCI_EN_CMD_ERROR;
                    break;
                }
            } while( (RetCode != HCI_EN_CMD_READY) && (RetCode != HCI_EN_CMD_ERROR) );

            if( (RetCode != HCI_EN_CMD_READY) )  {
                pAC->AsfData.Hci.Status = HCI_EN_CMD_IDLE;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    Command not confirmed RetCode:0x%x\n", RetCode ));
            }
        }  //  if( wait...
        else  {
            RetCode = AsfHciGetState( pAC );
            if( (RetCode == HCI_EN_CMD_ERROR) )  {
                pAC->AsfData.Hci.Status = HCI_EN_CMD_IDLE;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    Command not confirmed RetCode:0x%x\n", RetCode ));
            }
        }

        if( Retry > 0 )
            Retry--;
        else
            break;

    } while ( Wait && (RetCode != HCI_EN_CMD_READY) );

    if( (RetCode == HCI_EN_CMD_READY) )  {
        SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    Command successfully conveyed\n"));
#if (0)
	if (Wait)
	  printk("    Time required: %lu\n", (CurrTime-StartTime));
#endif
    }

    return( RetCode );
}

/*****************************************************************************
*
* AsfHciSendData
*
* Description:
*
* Returns:
*
*/
SK_U8   AsfHciSendData(
SK_AC   *pAC,
SK_IOC  IoC,
SK_U8   *Buffer,
SK_U8   ExpectResponse,
SK_U8   Wait,
SK_U8   Retry )  {
    SK_U8   RetCode;
    SK_U64  StartTime;
    SK_U64  CurrTime;
    SK_U64  TmpTime;
    SK_U8   Length;
    SK_U8   Command;

    do {
        if( pAC->AsfData.Hci.Status == HCI_EN_CMD_IDLE )  {
            Command = *(Buffer + 0 );
            Length =  *(Buffer + 1);
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    Send Data cmd:0x%x Length:%d wait:%d Retry:%d\n",
                                                           Command, Length, Wait, Retry ));
            RetCode = AsfHciSendMessage(pAC, IoC, Buffer, Length, ExpectResponse, Wait );
        }
        if( Wait )  {
            StartTime = SkOsGetTime(pAC);
            TmpTime = StartTime + 1;
            do {
                CurrTime = SkOsGetTime(pAC);
                if( CurrTime > TmpTime )  {
                    SkAsfHci( pAC, IoC, 0 );
                    TmpTime = CurrTime + 1;
                }
                RetCode = AsfHciGetState( pAC );
                if ( (CurrTime - StartTime) > (SK_TICKS_PER_SEC*1) )  {
                    RetCode = HCI_EN_CMD_ERROR;
                    break;
                }
            } while ( (RetCode == HCI_EN_CMD_WRITING) || (RetCode == HCI_EN_CMD_WAIT) );

            if( RetCode != HCI_EN_CMD_READY )  {
                pAC->AsfData.Hci.Status = HCI_EN_CMD_IDLE;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    Data not confirmed RetCode:0x%x\n", RetCode ));
            }
        }  //  if( wait...
        else  {
            RetCode = AsfHciGetState( pAC );
            if( RetCode == HCI_EN_CMD_ERROR )  {
                pAC->AsfData.Hci.Status = HCI_EN_CMD_IDLE;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    Data not confirmed RetCode:0x%x\n", RetCode ));
            }
        }
        if( Retry > 0 )
            Retry--;
        else
            break;
    } while ( Wait && (RetCode != HCI_EN_CMD_READY) );

    if( RetCode == HCI_EN_CMD_READY )  {
        SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    Data successfully conveyed\n"));
    }

    return( RetCode );
}

/*****************************************************************************
*
* AsfHciSendMessage
*
* Description:
*
* Returns:
*
*/
SK_U8    AsfHciSendMessage(
SK_AC   *pAC,
SK_IOC  IoC,
SK_U8   *message,
SK_U8   length,
SK_U8   ExpectResponse,
SK_U8   Wait )  {
    SK_U8       RetCode;
    SK_U8       i;
    SK_EVPARA   EventParam;

    RetCode = 0;

    if( length > ASF_HCI_TRA_BUF_SIZE )
        return( RetCode );

    if( pAC->AsfData.Hci.Status == HCI_EN_CMD_IDLE )
    {
        // SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    Send Message\n" ));

        SK_OUT32( IoC, ASF_HCI_CMDREG, (SK_U32) 0 );
        SK_OUT32( IoC, ASF_HCI_DATAREG, (SK_U32) 0 );
        for( i=0; i<length; i++ )
            pAC->AsfData.Hci.TransmitBuf[i] = message[i];
        pAC->AsfData.Hci.SendLength = length;
        if( ExpectResponse )
            pAC->AsfData.Hci.ExpectResponse = 1;
        else
            pAC->AsfData.Hci.ExpectResponse = 0;
        pAC->AsfData.Hci.Status = HCI_EN_CMD_WRITING;
        pAC->AsfData.Hci.OldCmdReg = 0;
        pAC->AsfData.Hci.SendIndex = 0;
        pAC->AsfData.Hci.Cycles = 0;
        pAC->AsfData.Hci.Time = SkOsGetTime( pAC );
        RetCode = 1;  //  successfull
    }
    else
        SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    Send Message -> Not Idle\n" ));


    if( !Wait )  {
        // SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    Send Message -> Start Timer\n" ));
        SK_MEMSET((char *)&EventParam, 0, sizeof(EventParam));
        SkTimerStart(pAC, IoC, &pAC->AsfData.Hci.AsfTimerHci,
                     10000, SKGE_ASF, SK_ASF_EVT_TIMER_HCI_EXPIRED,
                     EventParam);
    }

    return( RetCode );
}


void AsfWatchCpu(
SK_AC   *pAC,
SK_IOC  IoC,
SK_U32  par )  {

#ifdef _XXX_
SK_U32  FwAlive, LastFetch;

    SK_IN32( IoC, B28_Y2_DATA_REG_4, &FwAlive );
    SK_IN32( IoC, 0x64, &LastFetch );
    if( LastFetch < 0x20000 )
        pAC->AsfData.FwError++;
    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("(%d) AsfWatchCpu: FwAlive:0x%x LastFetch:0x%x Err:%d\n",
                                                   par, FwAlive , LastFetch, pAC->AsfData.FwError ));
#endif // _XXX_

    return;
}


void AsfEnable(
SK_AC   *pAC,
SK_IOC  IoC )  {
    SK_U32 TmpVal32;

    SK_IN32(IoC, B0_CTST, &TmpVal32);
    TmpVal32 &= ~0x00003000;    // clear bit13, bit12
    TmpVal32 |= 0x00002000;             // set bit13
    SK_OUT32(IoC, B0_CTST, TmpVal32);
    SK_OUT32(IoC, B0_CTST, Y2_ASF_ENABLE);

    pAC->GIni.GIAsfEnabled = SK_TRUE;   // update asf flag for common modules
}

void AsfDisable(
SK_AC   *pAC,
SK_IOC  IoC )  {
    SK_U32 TmpVal32;

    SK_IN32(IoC, B0_CTST, &TmpVal32);
    TmpVal32 &= ~0x00003000;    // clear bit13, bit12
    TmpVal32 |= 0x00001000;             // set bit13
    SK_OUT32(IoC, B0_CTST, TmpVal32);
    SK_OUT32(IoC, B0_CTST, Y2_ASF_DISABLE);

    pAC->GIni.GIAsfEnabled = SK_FALSE;  // update asf flag for common modules
}



/*****************************************************************************
*
* AsfWriteIpmiPattern
*
* Description:  write all 3 pattern for IPMI to pattern ram
*
* Notes:        none
*
* Context:      none
*
* Returns:      YLCI_SUCCESS
*               YLCI_ERROR
*
*/

SK_I8 AsfWriteIpmiPattern(
    SK_AC   *pAC,
    SK_IOC  IoC,
    SK_U8   port)
{
    SK_I8   RetVal;
    SK_U8   i, j;
    SK_U32  idx;
    SK_U32  PattRamCluster[ASF_YEC_PATTRAM_CLUSTER_WORDS];
    SK_U8   PattSrcByte[2];     // 2 pattern bytes to read/write in one cluster
    SK_U32  TmpVal32;
    SK_U32  mask;
    SK_U8   pattern_no;

    RetVal = 1;     // success

    if (RetVal == 1) {

        for (i = 0; (i < ASF_YEC_PATTRAM_CLUSTER_SIZE) && (RetVal == 1); i++) {
            // pattern ram is organized into cluster (i is cluster index)
            // _after_ writing a whole cluster (= 128bit), the pattern will be written into ram!

            // read a cluster
            for (j=0; j < ASF_YEC_PATTRAM_CLUSTER_WORDS; j++) {
                PattRamCluster[j] = 0;
            }

            //-----------------------
            // read source pattern 6
            //-----------------------
            pattern_no = 6;
            for (j=0; j < 2; j++) {
                // we have 2 pattern bytes to read/write for one cluster
                // i is cluster index
                // j is byte index
                idx = 2*i+j;
                if (idx < 40) {
                    // we can read from our pattern pointer
                    PattSrcByte[j] = RMCP_FRAME_PATTERN[idx];

                    // read from our enable mask pointer
                    TmpVal32 = RMCP_PATTERN_MASK[idx/8];
                    mask     = 0x01 << (idx % 8);

                    if ( (TmpVal32 & mask) != 0 ) {
                        // enable byte
                        PattRamCluster[j*2+1] |= (BIT_24 << pattern_no);
                    }
                    else {
                        // disable byte
                        PattRamCluster[j*2+1] &= ~(BIT_24 << pattern_no);
                    }
                }
                else {
                    // fill up with zeros
                    PattSrcByte[j] = 0;
                    // disable byte
                    PattRamCluster[j*2+1] &= ~(BIT_24 << pattern_no);
                }
            }

            // upper words are interesting here
            idx = 1;

            // first byte
            mask = 0x000000ff;
            j    = pattern_no % 4;
            PattRamCluster[idx] &= ~(mask << 8 * j);    // delete byte in word
            mask = PattSrcByte[0];
            PattRamCluster[idx] |= (mask << 8 * j);     // write pattern byte

            // second byte
            mask = 0x000000ff;
            PattRamCluster[idx+2] &= ~(mask << 8 * j);  // delete byte in word
            mask = PattSrcByte[1];
            PattRamCluster[idx+2] |= (mask << 8 * j);   // write pattern byte

            //-----------------------
            // read source pattern 5
            //-----------------------
            pattern_no = 5;
            for (j=0; j < 2; j++) {
                // we have 2 pattern bytes to read/write for one cluster
                // i is cluster index
                // j is byte index
                idx = 2*i+j;
                if (idx < 40) {
                    // we can read from our pattern pointer
                    PattSrcByte[j] = ARP_FRAME_PATTERN[idx];

                    // read from our enable mask pointer
                    TmpVal32 = ARP_PATTERN_MASK[idx/8];
                    mask     = 0x01 << (idx % 8);

                    if ( (TmpVal32 & mask) != 0 ) {
                        // enable byte
                        PattRamCluster[j*2+1] |= (BIT_24 << pattern_no);
                    }
                    else {
                        // disable byte
                        PattRamCluster[j*2+1] &= ~(BIT_24 << pattern_no);
                    }
                }
                else {
                    // fill up with zeros
                    PattSrcByte[j] = 0;
                    // disable byte
                    PattRamCluster[j*2+1] &= ~(BIT_24 << pattern_no);
                }
            }

            // upper words are interesting here
            idx = 1;

            // first byte
            mask = 0x000000ff;
            j    = pattern_no % 4;
            PattRamCluster[idx] &= ~(mask << 8 * j);    // delete byte in word
            mask = PattSrcByte[0];
            PattRamCluster[idx] |= (mask << 8 * j);     // write pattern byte

            // second byte
            mask = 0x000000ff;
            PattRamCluster[idx+2] &= ~(mask << 8 * j);  // delete byte in word
            mask = PattSrcByte[1];
            PattRamCluster[idx+2] |= (mask << 8 * j);   // write pattern byte

            //-----------------------
            // read source pattern 4
            //-----------------------
            pattern_no = 4;
            for (j=0; j < 2; j++) {
                // we have 2 pattern bytes to read/write for one cluster
                // i is cluster index
                // j is byte index
                idx = 2*i+j;
                if (idx < 40) {
                    // we can read from our pattern pointer
                    PattSrcByte[j] = RSP_FRAME_PATTERN[idx];

                    // read from our enable mask pointer
                    TmpVal32 = RSP_PATTERN_MASK[idx/8];
                    mask     = 0x01 << (idx % 8);

                    if ( (TmpVal32 & mask) != 0 ) {
                        // enable byte
                        PattRamCluster[j*2+1] |= (BIT_24 << pattern_no);
                    }
                    else {
                        // disable byte
                        PattRamCluster[j*2+1] &= ~(BIT_24 << pattern_no);
                    }
                }
                else {
                    // fill up with zeros
                    PattSrcByte[j] = 0;
                    // disable byte
                    PattRamCluster[j*2+1] &= ~(BIT_24 << pattern_no);
                }
            }

            // upper words are interesting here
            idx = 1;

            // first byte
            mask = 0x000000ff;
            j    = pattern_no % 4;
            PattRamCluster[idx] &= ~(mask << 8 * j);    // delete byte in word
            mask = PattSrcByte[0];
            PattRamCluster[idx] |= (mask << 8 * j);     // write pattern byte

            // second byte
            mask = 0x000000ff;
            PattRamCluster[idx+2] &= ~(mask << 8 * j);  // delete byte in word
            mask = PattSrcByte[1];
            PattRamCluster[idx+2] |= (mask << 8 * j);   // write pattern byte

            // write a cluster
            // after writing the last cluster word, the hardware will trigger writing all cluster words
            for (j=0; j < ASF_YEC_PATTRAM_CLUSTER_WORDS; j++) {
                idx = ASF_YEC_PATTRAM_CLUSTER_WORDS*ASF_YEC_PATTRAM_CLUSTER_BYTES*i + ASF_YEC_PATTRAM_CLUSTER_BYTES*j;

                if( port == 0 )  {
		    SK_OUT32( IoC, WOL_PATT_RAM_1+idx, PattRamCluster[j] );
		}
                else  {
                    SK_OUT32( IoC, WOL_PATT_RAM_2+idx, PattRamCluster[j] );
                }
            }
        }
    }

    return (RetVal);
}

void AsfSetUpPattern(
SK_AC   *pAC,
SK_IOC  IoC,
SK_U8   port )  {

    SK_U8   TmpVal8;
    SK_U16  TmpVal16;
    SK_U32  TmpVal32;

    if( port > 1 )
        return;

    switch (pAC->AsfData.OpMode) {
        case SK_GEASF_MODE_ASF:
            // see "problems.txt" note #344
            // set MAC Rx fifo flush threshold register
            // (please refer also to note #yuk_hw01)
            SK_IN32(IoC, ASF_YEC_MAC_FIFO_FLUSHTHRES1+(port*0x80), &TmpVal32);
            TmpVal32 &= ~0x7f;  // delete bit 6:0
            TmpVal32 |= ASF_YLCI_MACRXFIFOTHRES;
            SK_OUT32(IoC, ASF_YEC_MAC_FIFO_FLUSHTHRES1+(port*0x80), TmpVal32);

            // disable Wake Up Frame Unit before write to pattern ram
            SK_IN16(IoC, ASF_YEC_PATTERN_CTRL1+(port*0x80), &TmpVal16);
            TmpVal16 &= ~0x03;   // clear bit 0, bit1
            TmpVal16 |=  0x01;   // set bit 0
            SK_OUT16(IoC, ASF_YEC_PATTERN_CTRL1+(port*0x80), TmpVal16);

            //  Set new pattern for ASF
            //  This will override the driver WOL pattern, but the driver is going to set
            //  the WOL pattern again before it "shut down" or "stand by"
            if( pAC->AsfData.Mib.RspEnable )  {
                // security is on - write ARP and RSP pattern
                AsfWritePatternRam(pAC, IoC, port, 5, 6, 40, 40,
                                   ARP_PATTERN_MASK, ARP_FRAME_PATTERN,
                                   RSP_PATTERN_MASK, RSP_FRAME_PATTERN);
            }
            else  {
                // security is off - write ARP and RMCP pattern
                AsfWritePatternRam(pAC, IoC, port, 5, 6, 40, 40,
                                   ARP_PATTERN_MASK, ARP_FRAME_PATTERN,
                                   RMCP_PATTERN_MASK, RMCP_FRAME_PATTERN);
            }

            // set pattern length register
            SK_IN32(IoC, ASF_YEC_PATTERN_LENGTH_R1_H+(port*0x80), &TmpVal32);
            TmpVal32 &= ~(0x0000007f << 8*(5%4));
            TmpVal32 |= (40-1) << 8*(5%4);     // write length-1 to pattern length register
            SK_OUT32(IoC, ASF_YEC_PATTERN_LENGTH_R1_H+(port*0x80), TmpVal32);
            // set pattern length register
            SK_IN32(IoC, ASF_YEC_PATTERN_LENGTH_R1_H+(port*0x80), &TmpVal32);
            TmpVal32 &= ~(0x0000007f << 8*(6%4));
            TmpVal32 |= (40-1) << 8*(6%4);     // write length-1 to pattern length register
            SK_OUT32(IoC, ASF_YEC_PATTERN_LENGTH_R1_H+(port*0x80), TmpVal32);

            // set ASF match enable register (incomming packets will redirect to ASF queue)
            SK_IN8(IoC, ASF_YEC_PATTERN_MATCHENA1+(port*0x80), &TmpVal8);
            TmpVal8 |= 0x40;    // pattern 6 enable
            SK_OUT8(IoC, ASF_YEC_PATTERN_MATCHENA1+(port*0x80), TmpVal8);

            // enable pattern pattno
            SK_IN8(IoC, ASF_YEC_PATTERN_ENA1+(port*0x80), &TmpVal8);
            TmpVal8 |= 0x40;    // pattern 6 enable
            SK_OUT8(IoC, ASF_YEC_PATTERN_ENA1+(port*0x80), TmpVal8);

            // enable Wake Up Frame Unit
            SK_IN16(IoC, ASF_YEC_PATTERN_CTRL1+(port*0x80), &TmpVal16);
            TmpVal16 &= ~0x03;   // delete bit 0 and 1
            TmpVal16 |= 0x02;   // set bit 1
            SK_OUT16(IoC, ASF_YEC_PATTERN_CTRL1+(port*0x80), TmpVal16);
            break;
        case SK_GEASF_MODE_IPMI:
            // disable Wake Up Frame Unit before write to pattern ram
            SK_IN16(IoC, ASF_YEC_PATTERN_CTRL1+(port*0x80), &TmpVal16);
            TmpVal16 &= ~0x03;   // clear bit 0, bit1
            TmpVal16 |= 0x01;   // set bit 0
            SK_OUT16(IoC, ASF_YEC_PATTERN_CTRL1+(port*0x80), TmpVal16);

            // write all 3 pattern (RMCP, RSP and ARP)
            AsfWriteIpmiPattern(pAC, IoC, port);

            // set pattern length register
            SK_IN32(IoC, ASF_YEC_PATTERN_LENGTH_R1_H+(port*0x80), &TmpVal32);
            TmpVal32 &= 0xff000000;     // delete length for pattern 4, 5 and 6
            TmpVal32 |= 0x00272727;     // set new length for pattern 4, 5 and 6
            SK_OUT32(IoC, ASF_YEC_PATTERN_LENGTH_R1_H+(port*0x80), TmpVal32);

            // set ASF match enable register (incomming packets will redirect to ASF queue)
            SK_IN8(IoC, ASF_YEC_PATTERN_MATCHENA1+(port*0x80), &TmpVal8);
            TmpVal8 = 0x50;    // enable pattern 4 and 6 (do not enable arp pattern)
            SK_OUT8(IoC, ASF_YEC_PATTERN_MATCHENA1+(port*0x80), TmpVal8);

            // enable pattern pattno
            SK_IN8(IoC, ASF_YEC_PATTERN_ENA1+(port*0x80), &TmpVal8);
            TmpVal8 = 0x50;    // enable pattern 4 and 6 (do not enable arp pattern)
            SK_OUT8(IoC, ASF_YEC_PATTERN_ENA1+(port*0x80), TmpVal8);

            // enable Wake Up Frame Unit
            SK_IN16(IoC, ASF_YEC_PATTERN_CTRL1+(port*0x80), &TmpVal16);
            TmpVal16 &= ~0x03;   // delete bit 0 and 1
            TmpVal16 |= 0x02;   // set bit 1
            SK_OUT16(IoC, ASF_YEC_PATTERN_CTRL1+(port*0x80), TmpVal16);
            break;

        case SK_GEASF_MODE_DASH:
	    //  currently only supported on Yukon Extreme
	    if( pAC->AsfData.ChipMode != SK_GEASF_CHIP_EX )
	      break;
			
            // disable Wake Up Frame Unit before write to pattern ram
            SK_IN16(IoC, ASF_YEC_PATTERN_CTRL1+(port*0x80), &TmpVal16);
            TmpVal16 &= ~0x03;   // clear bit 0, bit1
            TmpVal16 |=  0x01;   // set bit 0
            SK_OUT16(IoC, ASF_YEC_PATTERN_CTRL1+(port*0x80), TmpVal16);

            // write all 3 pattern 
	    AsfWritePatternRamEx(pAC, IoC, port, ASF_DASH_PATTERN_NUM_ARP,  40, ARP_PATTERN_MASK, ARP_FRAME_PATTERN );
	    AsfWritePatternRamEx(pAC, IoC, port, ASF_DASH_PATTERN_NUM_ICMP, 40, ICMP_PATTERN_MASK, ICMP_FRAME_PATTERN );
	    AsfWritePatternRamEx(pAC, IoC, port, ASF_DASH_PATTERN_NUM_SNMP, 40, SNMP_PATTERN_MASK, SNMP_FRAME_PATTERN );
	    //AsfWritePatternRamEx(pAC, IoC, port, ASF_DASH_PATTERN_NUM_RMCP, 40, RMCP_PATTERN_MASK, RMCP_FRAME_PATTERN );
	    //AsfWritePatternRamEx(pAC, IoC, port, ASF_DASH_PATTERN_NUM_RSP,  40, RSP_PATTERN_MASK, RSP_FRAME_PATTERN );
	    //AsfWritePatternRamEx(pAC, IoC, port, ASF_DASH_PATTERN_NUM_TCP1, 40, TCP_PATTERN_MASK, TCP_FRAME_PATTERN );
	    //AsfWritePatternRamEx(pAC, IoC, port, ASF_DASH_PATTERN_NUM_TCP2, 40, TCP_PATTERN_MASK, TCP_FRAME_PATTERN );
	    
	    //YlciEnablePattern( pAC, IoC, port, ASF_DASH_PATTERN_NUM_RMCP );		
	    //YlciEnablePattern( pAC, IoC, port, ASF_DASH_PATTERN_NUM_RSP );		
	    //YlciEnablePattern( pAC, IoC, port, ASF_DASH_PATTERN_NUM_TCP1 );		
	    //YlciEnablePattern( pAC, IoC, port, ASF_DASH_PATTERN_NUM_TCP2 );		
			
            // enable Wake Up Frame Unit
            SK_IN16(IoC, ASF_YEC_PATTERN_CTRL1+(port*0x80), &TmpVal16);
            TmpVal16 &= ~0x03;   // delete bit 0 and 1
            TmpVal16 |=  0x02;   // set bit 1
            SK_OUT16(IoC, ASF_YEC_PATTERN_CTRL1+(port*0x80), TmpVal16);
            break;
			
        default:
            break;
    }

    return;
}

/*****************************************************************************
*
* AsfWritePatternRam
*
* Description:  write to pattern ram
*
* Notes:        none
*
* Context:      none
*
* Returns:      1:  SUCCESS
*               0:  YLCI_ERROR
*
*/

SK_I8 AsfWritePatternRam(
SK_AC   *pAC,
SK_IOC  IoC,
SK_U8   Port,                       /* for future use   */
SK_U8   PatternId1,                  /* 0..6             */
SK_U8   PatternId2,                  /* 0..6             */
SK_U8   Length1,                     /* 1..128 bytes     */
SK_U8   Length2,                     /* 1..128 bytes     */
SK_U8   *pMask1,
SK_U8   *pPattern1,
SK_U8   *pMask2,
SK_U8   *pPattern2)

{
    SK_U8   i, j;
    SK_I8   RetVal;
    SK_U32  idx;
    SK_U32  PattRamCluster[ASF_YEC_PATTRAM_CLUSTER_WORDS];
    SK_U8   PattSrcByte1[2];     // 2 pattern bytes to read/write in one cluster
    SK_U8   PattSrcByte2[2];     // 2 pattern bytes to read/write in one cluster
    SK_U32  TmpVal32;
    SK_U32  mask;

    RetVal = 1;  //  success

    // pattern size up to 128 bytes, pattern id can be 0...6
    if ( (Length1 <= SK_POW_PATTERN_LENGTH) && (PatternId1 < SK_NUM_WOL_PATTERN) ) {

        for (i = 0; (i < ASF_YEC_PATTRAM_CLUSTER_SIZE) && (RetVal == 1); i++) {
            // pattern ram is organized into cluster (i is cluster index)
            // _after_ writing a whole cluster (= 128bit), the pattern will be written into ram!

            // read a cluster
            for (j=0; j < ASF_YEC_PATTRAM_CLUSTER_WORDS; j++) {
                PattRamCluster[j] = 0;
            }
            // read source pattern 1
            for (j=0; j < 2; j++) {
                // we have 2 pattern bytes to read/write for one cluster
                // i is cluster index
                // j is byte index
                idx = 2*i+j;
                if ( idx < Length1 ) {
                    // we can read from our pattern pointer
                    PattSrcByte1[j] = pPattern1[idx];

                    // read from our enable mask pointer
                    TmpVal32 = pMask1[idx/8];
                    mask     = 0x01 << (idx % 8);

                    if ( (TmpVal32 & mask) != 0 ) {
                        // enable byte
                        PattRamCluster[j*2+1] |= (BIT_24 << PatternId1);
                    }
                    else {
                        // disable byte
                        PattRamCluster[j*2+1] &= ~(BIT_24 << PatternId1);
                    }
                }
                else {
                    // fill up with zeros
                    PattSrcByte1[j] = 0;
                    // disable byte
                    PattRamCluster[j*2+1] &= ~(BIT_24 << PatternId1);
                }
            }
            // read source pattern 2
            for (j=0; j < 2; j++) {
                // we have 2 pattern bytes to read/write for one cluster
                // i is cluster index
                // j is byte index
                idx = 2*i+j;
                if ( idx < Length2 ) {
                    // we can read from our pattern pointer
                    PattSrcByte2[j] = pPattern2[idx];

                    // read from our enable mask pointer
                    TmpVal32 = pMask2[idx/8];
                    mask     = 0x01 << (idx % 8);

                    if ( (TmpVal32 & mask) != 0 ) {
                        // enable byte
                        PattRamCluster[j*2+1] |= (BIT_24 << PatternId2);
                    }
                    else {
                        // disable byte
                        PattRamCluster[j*2+1] &= ~(BIT_24 << PatternId2);
                    }
                }
                else {
                    // fill up with zeros
                    PattSrcByte2[j] = 0;
                    // disable byte
                    PattRamCluster[j*2+1] &= ~(BIT_24 << PatternId2);
                }
            }
            // set our pattern into PattRamCluster[]
            if (PatternId1 >= 4) {
                // upper words are interesting here
                idx = 1;
            }
            else {
                // lower words are interesting here
                idx = 0;
            }
            // first byte
            mask = 0x000000ff;
            j    = PatternId1 % 4;
            PattRamCluster[idx] &= ~(mask << 8 * j);    // delete byte in word
            mask = PattSrcByte1[0];
            PattRamCluster[idx] |= (mask << 8 * j);     // write pattern byte
            // second byte
            mask = 0x000000ff;
            PattRamCluster[idx+2] &= ~(mask << 8 * j);  // delete byte in word
            mask = PattSrcByte1[1];
            PattRamCluster[idx+2] |= (mask << 8 * j);   // write pattern byte

            // set our pattern into PattRamCluster[]
            if (PatternId2 >= 4) {
                // upper words are interesting here
                idx = 1;
            }
            else {
                // lower words are interesting here
                idx = 0;
            }
            // first byte
            mask = 0x000000ff;
            j    = PatternId2 % 4;
            PattRamCluster[idx] &= ~(mask << 8 * j);    // delete byte in word
            mask = PattSrcByte2[0];
            PattRamCluster[idx] |= (mask << 8 * j);     // write pattern byte
            // second byte
            mask = 0x000000ff;
            PattRamCluster[idx+2] &= ~(mask << 8 * j);  // delete byte in word
            mask = PattSrcByte2[1];
            PattRamCluster[idx+2] |= (mask << 8 * j);   // write pattern byte

            // write a cluster
            // after writing the last cluster word, the hardware will trigger writing all cluster words
            for (j=0; j < ASF_YEC_PATTRAM_CLUSTER_WORDS; j++) {
                idx = ASF_YEC_PATTRAM_CLUSTER_WORDS*ASF_YEC_PATTRAM_CLUSTER_BYTES*i + ASF_YEC_PATTRAM_CLUSTER_BYTES*j;
                if( Port == 0 )  {
                    SK_OUT32( IoC, WOL_PATT_RAM_1+idx, PattRamCluster[j] );
                }
                else  {
                    SK_OUT32( IoC, WOL_PATT_RAM_2+idx, PattRamCluster[j] );
                }
            }
        }
    }
    else {
        RetVal = 0;  //  error
    }

    return(RetVal);
}

/*****************************************************************************
*
* YlciWritePatternRamEx
*
* Description:  write to pattern ram
*
* Notes:        none
*
* Context:      none
*
* Returns:      1:  SUCCESS
*               0:  YLCI_ERROR
*
*/

// #ifdef CHIP_ID_YUKON_EX		
SK_I8 AsfWritePatternRamEx(
SK_AC   *pAC,
SK_IOC  IoC,
SK_U8   Port,                        /* for future use   */
SK_U8   PatternId ,                  /* 0..9             */
SK_U8   Length ,                     /* 1..128 bytes     */
SK_U8   *pMask ,
SK_U8   *pPattern )

{
    SK_U8   i, j;
    SK_I8   RetVal;
    SK_U32  idx;
    SK_U32  PattRamCluster[ASF_YEC_PATTRAM_CLUSTER_WORDS];
    SK_U8   PattSrcByte[2];     // 2 pattern bytes to read/write in one cluster
    SK_U32  TmpVal32;
    SK_U32  mask;
	SK_U8   TmpPatternId;
	SK_U8	bank;
	SK_U16  AsfYexPatternLengthReg;
	
    RetVal = 1;  //  success

    // pattern size up to 128 bytes, pattern id can be 0...6
    if ( Length <= SK_POW_PATTERN_LENGTH ) {
    	if ( PatternId < SK_NUM_WOL_PATTERN_EX ) {
			
			if( PatternId < SK_NUM_WOL_PATTERN )  {
				TmpPatternId = PatternId;
				bank = 0;
			}
			else  {
				TmpPatternId = PatternId - SK_NUM_WOL_PATTERN;
				bank = 1;
			}

			//  select pattern bank
            SK_IN32( IoC, PAT_CSR, &TmpVal32 );
			if( bank == 0 )
				TmpVal32 &= ~(PAT_CSR_BSEL);
			else
				TmpVal32 |= PAT_CSR_BSEL;
            SK_OUT32( IoC, PAT_CSR, TmpVal32 );
			
	        for (i = 0; (i < ASF_YEC_PATTRAM_CLUSTER_SIZE) && (RetVal == 1); i++) {
	            // pattern ram is organized into cluster (i is cluster index)
	            // _after_ writing a whole cluster (= 128bit), the pattern will be written into ram!

	            // read a cluster
	            for (j=0; j < ASF_YEC_PATTRAM_CLUSTER_WORDS; j++) {
	                idx = ASF_YEC_PATTRAM_CLUSTER_WORDS*ASF_YEC_PATTRAM_CLUSTER_BYTES*i + ASF_YEC_PATTRAM_CLUSTER_BYTES*j;
	                if( Port == 0 )  {
	                    SK_IN32( IoC, WOL_PATT_RAM_1+idx, &PattRamCluster[j] );
	                }
	                else  {
	                    SK_IN32( IoC, WOL_PATT_RAM_2+idx, &PattRamCluster[j] );
	                }
	            }
	            // read source pattern 
	            for (j=0; j < 2; j++) {
	                // we have 2 pattern bytes to read/write for one cluster
	                // i is cluster index
	                // j is byte index
	                idx = 2*i+j;
	                if ( idx < Length ) {
	                    // we can read from our pattern pointer
	                    PattSrcByte[j] = pPattern[idx];

	                    // read from our enable mask pointer
	                    TmpVal32 = pMask[idx/8];
	                    mask     = 0x01 << (idx % 8);

	                    if ( (TmpVal32 & mask) != 0 ) {
	                        // enable byte
	                        PattRamCluster[j*2+1] |= (BIT_24 << TmpPatternId);
	                    }
	                    else {
	                        // disable byte
	                        PattRamCluster[j*2+1] &= ~(BIT_24 << TmpPatternId);
	                    }
	                }
	                else {
	                    // fill up with zeros
	                    PattSrcByte[j] = 0;
	                    // disable byte
	                    PattRamCluster[j*2+1] &= ~(BIT_24 << TmpPatternId);
	                }
	            }
	            // set our pattern into PattRamCluster[]
	            if (TmpPatternId >= 4) {
	                // upper words are interesting here
	                idx = 1;
	            }
	            else {
	                // lower words are interesting here
	                idx = 0;
	            }
	            // first byte
	            mask = 0x000000ff;
	            j    = TmpPatternId % 4;
	            PattRamCluster[idx] &= ~(mask << 8 * j);    // delete byte in word
	            mask = PattSrcByte[0];
	            PattRamCluster[idx] |= (mask << 8 * j);     // write pattern byte
	            // second byte
	            mask = 0x000000ff;
	            PattRamCluster[idx+2] &= ~(mask << 8 * j);  // delete byte in word
	            mask = PattSrcByte[1];
	            PattRamCluster[idx+2] |= (mask << 8 * j);   // write pattern byte

	            // write a cluster
	            // after writing the last cluster word, the hardware will trigger writing all cluster words
	            for (j=0; j < ASF_YEC_PATTRAM_CLUSTER_WORDS; j++) {
	                idx = ASF_YEC_PATTRAM_CLUSTER_WORDS*ASF_YEC_PATTRAM_CLUSTER_BYTES*i + ASF_YEC_PATTRAM_CLUSTER_BYTES*j;
	                if( Port == 0 )  {
	                    SK_OUT32( IoC, WOL_PATT_RAM_1+idx, PattRamCluster[j] );
	                }
	                else  {
	                    SK_OUT32( IoC, WOL_PATT_RAM_2+idx, PattRamCluster[j] );
	                }
	            }
	        }  //  	for (i = 0; (i < ASF_YEC_PATTRAM_CLUSTER_SIZE) && (RetVal == 1); i++)
	        
			//  Set pattern length register
			AsfYexPatternLengthReg = ASF_YEX_PATTERN_LENGTH_R1_L+(Port*0x80)+( (PatternId/4)*sizeof(SK_U32) );
		    SK_IN32(IoC, AsfYexPatternLengthReg, &TmpVal32);		
		    TmpVal32 &= ~(0x0000007f << 8*(PatternId%4));			
		    TmpVal32 |= (Length-1) << 8*(PatternId%4);     // write length-1 to pattern length register
		    SK_OUT32(IoC, AsfYexPatternLengthReg, TmpVal32);
			
    	}  //  if ( PatternId < SK_NUM_WOL_PATTERN_EX ) 
		else  {
	        RetVal = 0;  //  error
		}
    }  //   if ( Length1 <= SK_POW_PATTERN_LENGTH )
    else {
        RetVal = 0;  //  error
    }
    return(RetVal);
}
// #endif //  CHIP_ID_YUKON_EX	

SK_I8 YlciEnablePattern (
SK_AC   *pAC,
SK_IOC  IoC,
SK_U8   port,
SK_U8   pattno )
{


    if (port > 1) {
        return (1);
    }
    
    if( pAC->AsfData.ChipMode == SK_GEASF_CHIP_EX )  {
      SK_U16   val16;
      
      if( pAC->AsfData.OpMode != SK_GEASF_MODE_DASH )
	pattno += 6;
      
      // set ASF match enable register (incomming packets will redirect to ASF queue)
      SK_IN16(IoC, ASF_YEX_PATTERN_MATCHENA1+(port*0x80), &val16);
      val16 |= (0x01 << pattno);
      SK_OUT16(IoC, ASF_YEX_PATTERN_MATCHENA1+(port*0x80), val16);
      
      // enable pattern pattno
      SK_IN16(IoC, ASF_YEX_PATTERN_ENA1+(port*0x80), &val16);
      val16 |= (0x01 << pattno);
      SK_OUT16(IoC, ASF_YEX_PATTERN_ENA1+(port*0x80), val16);
    }
    else  {
      SK_U8   val8;
      pattno += 4;
      // set ASF match enable register (incomming packets will redirect to ASF queue)
      SK_IN8(IoC, ASF_YEC_PATTERN_MATCHENA1+(port*0x80), &val8);
      val8 |= (0x01 << pattno);
      SK_OUT8(IoC, ASF_YEC_PATTERN_MATCHENA1+(port*0x80), val8);
      
      // enable pattern pattno
      SK_IN8(IoC, ASF_YEC_PATTERN_ENA1+(port*0x80), &val8);
      val8 |= (0x01 << pattno);
      SK_OUT8(IoC, ASF_YEC_PATTERN_ENA1+(port*0x80), val8);
    }

    return ( 1 );
}

// Called YlciDisablePatternType in Windows Code
SK_I8 YlciDisablePattern (
SK_AC   *pAC,
SK_IOC  IoC,
SK_U8   port,
SK_U8   pattno )
{
    if (port > 1) {
        return (1);
    }
    if( pAC->AsfData.ChipMode == SK_GEASF_CHIP_EX )  {
      SK_U16   val16;
      
      if( pAC->AsfData.OpMode != SK_GEASF_MODE_DASH )
	pattno += 6;
      
      // set ASF match disable register (incomming packets will redirect to ASF queue)
      SK_IN16(IoC, ASF_YEX_PATTERN_MATCHENA1+(port*0x80), &val16);
      val16 &= ~(0x01 << pattno);
      SK_OUT16(IoC, ASF_YEX_PATTERN_MATCHENA1+(port*0x80), val16);
      
      // disable pattern pattno
      SK_IN16(IoC, ASF_YEX_PATTERN_ENA1+(port*0x80), &val16);
	    val16 &= ~(0x01 << pattno);
	    SK_OUT16(IoC, ASF_YEX_PATTERN_ENA1+(port*0x80), val16);
    }
    else  {
      SK_U8   val8;
      pattno += 4;
      // set ASF match disable register (incomming packets will redirect to ASF queue)
      SK_IN8(IoC, ASF_YEC_PATTERN_MATCHENA1+(port*0x80), &val8);
      val8 &= ~(0x01 << pattno);
      SK_OUT8(IoC, ASF_YEC_PATTERN_MATCHENA1+(port*0x80), val8);
      
      // disable pattern pattno
      SK_IN8(IoC, ASF_YEC_PATTERN_ENA1+(port*0x80), &val8);
      val8 &= ~(0x01 << pattno);
      SK_OUT8(IoC, ASF_YEC_PATTERN_ENA1+(port*0x80), val8);
    }	
    return ( 1 );
}

#if (0)

/*****************************************************************************
*
* SkAsfAcpi
*
* Description:  Searches for the Root System Description Pointer "RSD PTR"
*               within the range of 0xE0000 .. 0xFFFFF (ACPI Spec 5.2.4.1).
*               The "RSD PTR" is the entry point of the ACPI data base. It
*               contains pointer to the "RSDT" (32bit) and "XSDT" (64bit)
*               tables, which in turn contain lists of pointers that are
*               pointing to ACPI sub tables e.g. "ASF!" .
*
* Notes:        none
*
* Context:      none
*
* Returns:       1: OK
*                0: UNDEFINED
*               <0: Error Codes
*
*/
SK_I8 SkAsfAcpi(
SK_AC   *pAC,
SK_IOC  IoC,
SK_U8 *pImage )  {
    HANDLE              SectionHandle;
    NTSTATUS            NtRet;
    unsigned char       *pU8;
    PVOID               BaseAddress;
    ULONG               CommitSize;
    ULONG               i;
    LARGE_INTEGER       MaxSize;
    LARGE_INTEGER       SectOffset;
    SIZE_T              ViewSize;
    SK_U32              RsdtAddr;
    SK_U64              XsdtAddr;
    SK_I8               AcpiState;
    SK_I8               RetVal;
    UNICODE_STRING      SectionName;
    OBJECT_ATTRIBUTES   ObjAttrs;

    WCHAR               const SectionNameConst[] = L"\\Device\\PhysicalMemory";

    RsdtAddr = 0;
    AcpiState = ASF_ACPI_STATE_OK;

    //  Initialize object attributes
    SectionName.Buffer =        (PWSTR)&SectionNameConst[0];
    SectionName.Length =        wcslen(SectionNameConst) * sizeof(WCHAR);
    SectionName.MaximumLength = SectionName.Length + sizeof(WCHAR);

#if NDIS_VERSION > 50
    // WinXP
    InitializeObjectAttributes(
                            &ObjAttrs,
                            &SectionName,
                            (OBJ_KERNEL_HANDLE | OBJ_FORCE_ACCESS_CHECK),
                            (HANDLE)NULL,
                            (PSECURITY_DESCRIPTOR)NULL
                           );
#else
    // Win2k
    InitializeObjectAttributes(
                            &ObjAttrs,
                            &SectionName,
                            (OBJ_KERNEL_HANDLE),
                            (HANDLE)NULL,
                            (PSECURITY_DESCRIPTOR)NULL
                           );
#endif

    //  Get a SectionHandle to the "\\Device\\PhysicalMemory" section
    NtRet = ZwOpenSection( &SectionHandle,      //  OUT PHANDLE  SectionHandle
                           SECTION_MAP_READ,    //  IN ACCESS_MASK  DesiredAccess,
                           &ObjAttrs );         //  IN POBJECT_ATTRIBUTES  ObjectAttributes

    if( NtRet == STATUS_SUCCESS )  {
        BaseAddress = NULL;
        SectOffset.QuadPart =   0xe0000;
        CommitSize =            0x1ffff;
        ViewSize =              0x1ffff;
        NtRet = ZwMapViewOfSection( SectionHandle,          // IN HANDLE  SectionHandle
                                    NtCurrentProcess(),     // IN HANDLE  ProcessHandle
                                    &BaseAddress,           // IN OUT PVOID  *BaseAddress
                                    0L,                     // IN ULONG  ZeroBits
                                    CommitSize,             // IN ULONG  CommitSize,
                                    &SectOffset,            // IN OUT PLARGE_INTEGER  SectionOffset  OPTIONAL,
                                    &ViewSize,              // IN OUT PSIZE_T  ViewSize,
                                    ViewShare,              // IN SECTION_INHERIT  InheritDisposition,
                                    0,                      // IN ULONG  AllocationType,
                                    PAGE_READONLY           // IN ULONG  Protect
                                    );
        if( NtRet == STATUS_SUCCESS )  {
            //  Update ASF! table
            //  Search for the "RSD PTR" signature
            pU8 = ((unsigned char *) BaseAddress);
            for( i=0; i<(0x1ffff-7); i++ ) {
                if( (*(pU8 + 0 ) == 'R') && (*(pU8 + 1 ) == 'S') && (*(pU8 + 2 ) == 'D') && (*(pU8 + 3 ) == ' ') &&
                    (*(pU8 + 4 ) == 'P') && (*(pU8 + 5 ) == 'T') && (*(pU8 + 6 ) == 'R')    )  {
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfAcpi -> RSD PTR found at 0x%x (phys)\n", 0xe0000+i ));
                    //  Get Address of the Root System Description Table "RSDT"
                    RsdtAddr = *( (SK_U32 *)(pU8 + 16) );
                    if( RsdtAddr != 0 )  {
                        //  32bit platform
                        // SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfAcpi -> RSDT (0x%x)\n", RsdtAddr));
                        RetVal = SkAsfAcpiRsdt( pAC, IoC, pImage, SectionHandle, RsdtAddr );
                        if( RetVal < ASF_ACPI_STATE_UNDEFINED )
                            AcpiState = RetVal;
                    }
                    else  {
                        //  64bit platform
                        XsdtAddr = *( (SK_U64 *)(pU8 + 24) );
                        // SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfAcpi -> XSDT (0x%x)\n", XsdtAddr ));
                        RetVal = SkAsfAcpiXsdt( pAC, IoC, pImage, SectionHandle, XsdtAddr );
                        if( RetVal < ASF_ACPI_STATE_UNDEFINED )
                            AcpiState = RetVal;
                    }
                    break;
                }
                else
                    pU8++;
            }
            if( i >= (0x1ffff-7) )  {
                AcpiState = ASF_ACPI_STATE_ERROR_NO_RSDPTR;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfAcpi -> *** Error: no RSD PTR found\n"));
            }
            //  Update GUID
            //  Search for the "_UUID_" signature
            pU8 = ((unsigned char *) BaseAddress);
            for( i=0; i<(0x1ffff-7); i++ ) {
                if( (*(pU8 + 0 ) == '_') && (*(pU8 + 1 ) == 'U') && (*(pU8 + 2 ) == 'U') && (*(pU8 + 3 ) == 'I') &&
                    (*(pU8 + 4 ) == 'D') && (*(pU8 + 5 ) == '_')   )  {
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfAcpi -> _UUID_ found at 0x%x (phys)\n", 0xe0000+i ));
//                    RetVal = SkAsfPatchGuid( pAC, IoC, pImage, (pU8+6) );
                    RetVal = SkAsfPatchGuid( pAC, IoC, pImage, (pU8+9) );   // pw Bug Fix
                    if( RetVal < ASF_ACPI_STATE_UNDEFINED )
                        AcpiState = RetVal;
                    break;
                }
                else
                    pU8++;
            }
            if( i >= (0x1ffff-7) )  {
                AcpiState = ASF_ACPI_STATE_ERROR_NO_RSDPTR;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfAcpi -> *** Error: no _UUID_ found\n"));
            }

            ZwUnmapViewOfSection( NtCurrentProcess(), BaseAddress );
        }  //  if( NtRet == STATUS_SUCCESS )  {
        ZwClose( SectionHandle );
    }

    return( AcpiState );
}
#endif

#if (0)

/*****************************************************************************
*
* SkAsfAcpiRsdt
*
* Description:  Searches in the pointer list of the RSDT table for the "ASF!"
*               table pointer.
*
* Notes:        none
*
* Context:      none
*
* Returns:       1: OK
*                0: UNDEFINED
*               <0: Error Codes
*
*/
SK_I8 SkAsfAcpiRsdt(
SK_AC   *pAC,
SK_IOC  IoC,
SK_U8   *pImage,
HANDLE  SectionHandle,
SK_U32  PhysAddr    )  {
    SK_I8 RetVal;
    NTSTATUS            NtRet;
    unsigned char       *pU8;
    PVOID               BaseAddress;
    ULONG               CommitSize;
    ULONG               i;
    LARGE_INTEGER       MaxSize;
    LARGE_INTEGER       SectOffset;
    SIZE_T              ViewSize;

    SK_U32              Length;
    SK_U32              NumTables;
    SK_U32              TableAddr;
    SK_U8               CheckSum;
    SK_U8               Rev;

    RetVal = ASF_ACPI_STATE_UNDEFINED;

    //  Try to map a view to the "RSDT" table
    BaseAddress = NULL;
    SectOffset.QuadPart =   PhysAddr; //  will be roundet down to the next allocation boundery
    CommitSize =            PAGE_SIZE*2;
    ViewSize =              PAGE_SIZE*2;
    NtRet = ZwMapViewOfSection( SectionHandle,          // IN HANDLE  SectionHandle
                                NtCurrentProcess(),     // IN HANDLE  ProcessHandle
                                &BaseAddress,           // IN OUT PVOID  *BaseAddress
                                0L,                     // IN ULONG  ZeroBits
                                CommitSize,             // IN ULONG  CommitSize,
                                &SectOffset,            // IN OUT PLARGE_INTEGER  SectionOffset  OPTIONAL,
                                &ViewSize,              // IN OUT PSIZE_T  ViewSize,
                                ViewShare,              // IN SECTION_INHERIT  InheritDisposition,
                                0,                      // IN ULONG  AllocationType,
                                PAGE_READONLY           // IN ULONG  Protect
                                );
    if( NtRet == STATUS_SUCCESS )  {
        pU8 = ((unsigned char *) BaseAddress);
        //   search for "RSDT" signature, because BaseAddress is roundet down to the
        //   next allocation boundary  (PAGE_SIZE ??)
        for( i=0; i<PAGE_SIZE; i++ )  {
            if( (*(pU8 + 0 ) == 'R') && (*(pU8 + 1 ) == 'S') &&
                (*(pU8 + 2 ) == 'D') && (*(pU8 + 3 ) == 'T')    )
                break;
            else
                pU8++;
        }
        //  if "RSDT" Table has been found, check header
        if( i < PAGE_SIZE )  {
            //  Check the signature and header
            Length = *( (SK_U32 *) (pU8 + 4) );
            Rev = *( pU8 + 8);
            CheckSum = 0;
            for( i=0; i<Length; i++ )
                CheckSum += *(pU8+i);
            if( (CheckSum == 0) && (Rev == 1) )  {
                if( (Length >= TABLE_HEADER_LENGTH))  {
                    NumTables = (Length - TABLE_HEADER_LENGTH) / 4;
                    // SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfAcpiRsdt -> %d tables in the list\n", NumTables));
                    pU8 += TABLE_HEADER_LENGTH;
                    //  check all list entries (phys. addr) for a valid "ASF!" table
                    for( i=0; i<NumTables; i++ )  {
                        RetVal = SkAsfAcpiAsf( pAC, IoC, pImage, SectionHandle, *((SK_U32 *) pU8) );
                        if( RetVal == ASF_ACPI_STATE_OK )
                            break;
                        pU8 += 4;
                    }
                }
                else
                    RetVal = ASF_ACPI_STATE_ERROR_RSDT_NO_TABLE;
            }
            else
                RetVal = ASF_ACPI_STATE_ERROR_RSDT_HEADER;
        }  //  if( i < PAGE_SIZE )  {
        else
            RetVal = ASF_ACPI_STATE_ERROR_RSDT;
        ZwUnmapViewOfSection( NtCurrentProcess(), BaseAddress );
    }
    else
        RetVal = ASF_ACPI_STATE_ERROR_RSDT;

    return( RetVal );
}
#endif

#if (0)
/*****************************************************************************
*
* SkAsfAcpiXsdt
*
* Description:  Searches in the pointer list of the XSDT table for the "ASF!"
*               table pointer.
*
* Notes:        none
*
* Context:      none
*
* Returns:       1: OK
*                0: UNDEFINED
*               <0: Error Codes
*
*/
SK_I8 SkAsfAcpiXsdt(
SK_AC   *pAC,
SK_IOC  IoC,
SK_U8   *pImage,
HANDLE  SectionHandle,
SK_U64  PhysAddr )  {
    SK_I8 RetVal;

    RetVal = ASF_ACPI_STATE_ERROR_XSDT;

    return( RetVal );
}
#endif

#if (0)
/*****************************************************************************
*
* SkAsfAcpiAsf
*
* Description:  Checks, whether the given PhysAddr points to a valid ASF! table.
*
* Notes:        none
*
* Context:      none
*
* Returns:       1: OK
*                0: UNDEFINED
*               <0: Error Codes
*
*/
SK_I8 SkAsfAcpiAsf(
SK_AC   *pAC,
SK_IOC  IoC,
SK_U8   *pImage,
HANDLE  SectionHandle,
SK_U32  PhysAddr    )  {
    SK_I8 RetVal;
    NTSTATUS            NtRet;
    unsigned char       *pU8;
    PVOID               BaseAddress;
    ULONG               CommitSize;
    ULONG               i;
    LARGE_INTEGER       MaxSize;
    LARGE_INTEGER       SectOffset;
    SIZE_T              ViewSize;

    SK_U32              Length;
    SK_U32              NumTables;
    SK_U32              TableAddr;
    SK_U8               CheckSum;
    SK_U8               Rev;

    NDIS_STATUS     Status;
    NDIS_HANDLE     FileHandle;
    UINT            FileLength;
    SK_U8           *pAcpiAsf;

    RetVal = ASF_ACPI_STATE_ERROR_RSDT_NO_ASF_TABLE;

    /* Try to open the ASF Simulation file (AcpiAsf.bin) */
    NdisOpenFile( (PNDIS_STATUS) &Status,
                  (PNDIS_HANDLE) &FileHandle,
                  (PUINT) &FileLength,
                  (PNDIS_STRING) &SimuAsfTab,
                  HighestPhysAddress  );

    if(Status == NDIS_STATUS_SUCCESS) {
        SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("*** AcpiAsf.bin opened successfully ***\n"));
        /* Try to map AcpiAsf.bin */
        NdisMapFile( (PNDIS_STATUS)&Status, (PVOID)&pAcpiAsf, FileHandle );
        if(Status == NDIS_STATUS_SUCCESS) {
            pU8 = ((unsigned char *) pAcpiAsf);
            //   search for "ASF!" signature, because BaseAddress is roundet down to the
            //   next allocation boundary  (PAGE_SIZE ??)
            for( i=0; i<PAGE_SIZE; i++ )  {
                if( (*(pU8 + 0 ) == 'A') && (*(pU8 + 1 ) == 'S') &&
                    (*(pU8 + 2 ) == 'F') && (*(pU8 + 3 ) == '!')    )
                    break;
                else
                    pU8++;
            }
            //  if "ASF!" Table has been found, check header
            if( i < PAGE_SIZE )  {
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("Simulation ASF! table found\n"));
                Length = *( (SK_U32 *) (pU8 + 4) );
                Rev = *( pU8 + 8);
                CheckSum = 0;
                for( i=0; i<Length; i++ )
                    CheckSum += *(pU8+i);
                if( (CheckSum == 0) && ( (Rev == 0x10) || (Rev == 0x20) ) )  {  //  Revision 0x10, 0x20 => ASF
                    RetVal = SkAsfPatchAsfTable( pAC, IoC, pImage, pU8, Length );
                }
                else
                    RetVal = ASF_ACPI_STATE_ERROR_ASF_HEADER;
            }  //  if( i < PAGE_SIZE )  {
            else
                RetVal = ASF_ACPI_STATE_ERROR_ASF;
        }
    }
    else { // No Simulation File, take REAL ASF! Table
        //  Try to map a view to the "ASF" table
        BaseAddress = NULL;
        SectOffset.QuadPart =   PhysAddr;
        CommitSize =            PAGE_SIZE*2;
        ViewSize =              PAGE_SIZE*2;
        NtRet = ZwMapViewOfSection( SectionHandle,          // IN HANDLE  SectionHandle
                                    NtCurrentProcess(),     // IN HANDLE  ProcessHandle
                                    &BaseAddress,           // IN OUT PVOID  *BaseAddress
                                    0L,                     // IN ULONG  ZeroBits
                                    CommitSize,             // IN ULONG  CommitSize,
                                    &SectOffset,            // IN OUT PLARGE_INTEGER  SectionOffset  OPTIONAL,
                                    &ViewSize,              // IN OUT PSIZE_T  ViewSize,
                                    ViewShare,              // IN SECTION_INHERIT  InheritDisposition,
                                    0,                      // IN ULONG  AllocationType,
                                    PAGE_READONLY           // IN ULONG  Protect
                                    );
        if( NtRet == STATUS_SUCCESS )  {
            pU8 = ((unsigned char *) BaseAddress);
            //   search for "ASF!" signature, because BaseAddress is roundet down to the
            //   next allocation boundary  (PAGE_SIZE ??)
            for( i=0; i<PAGE_SIZE; i++ )  {
                if( (*(pU8 + 0 ) == 'A') && (*(pU8 + 1 ) == 'S') &&
                    (*(pU8 + 2 ) == 'F') && (*(pU8 + 3 ) == '!')    )
                    break;
                else
                    pU8++;
            }
            //  if "ASF!" Table has been found, check header
            if( i < PAGE_SIZE )  {
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("SkAsfAcpiAsf -> ASF! table found at 0x%x (phys)\n", PhysAddr ));
                Length = *( (SK_U32 *) (pU8 + 4) );
                Rev = *( pU8 + 8);
                CheckSum = 0;
                for( i=0; i<Length; i++ )
                    CheckSum += *(pU8+i);
                if( (CheckSum == 0) && ( (Rev == 0x10) || (Rev == 0x20) ) )  {  //  Revision 0x10, 0x20 => ASF
                    RetVal = SkAsfPatchAsfTable( pAC, IoC, pImage, pU8, Length );
                }
                else
                    RetVal = ASF_ACPI_STATE_ERROR_ASF_HEADER;
            }  //  if( i < PAGE_SIZE )  {
            else
                RetVal = ASF_ACPI_STATE_ERROR_ASF;

            ZwUnmapViewOfSection( NtCurrentProcess(), BaseAddress );
        }
        else
            RetVal = ASF_ACPI_STATE_ERROR_ASF;

        ZwUnmapViewOfSection( NtCurrentProcess(), BaseAddress );
    }

    return( RetVal );
}
#endif

#if (0)
/*****************************************************************************
*
* SkAsfPatchAsfTable
*
* Description:  Ovverides the table in the fw image with the current
*               BIOS - ASF! - table.
*
* Notes:        none
*
* Context:      none
*
* Returns:       1: OK
*                0: UNDEFINED
*               <0: Error Codes
*
*/
SK_I8 SkAsfPatchAsfTable(
SK_AC   *pAC,
SK_IOC  IoC,
SK_U8 *pImage,
SK_U8 *pAsfTable,
SK_U32 TableLength )  {

    SK_I8           RetVal;
    SK_U32          i,j;
    SK_U8           *pAcpiAsf;
    SK_U8           FwImageCsOk;
    SK_U32          FwImageCs;
    SK_U32          FwCs;
    SK_U32          *pTmp32;

    RetVal = ASF_ACPI_STATE_OK;

    //  calculate CS of the FW image
    pTmp32 = (SK_U32 *) pImage;
    for( i=0, FwCs=0; i<ASF_FLASH_SIZE; i+=4 )  {
        FwCs += *pTmp32;
        pTmp32++;
    }
    if( FwCs == 0  )  {  //  CS == 0 => O.K.
        pAcpiAsf = (SK_U8 *) (pImage + ASF_FLASH_OFFS_ACPI);
        //  write new "ASF!" - table to the FW image file
        for( i=0; i<TableLength; i++ )  {
            *(pAcpiAsf+i) = *(pAsfTable+i);
        }
        //  erase dynamic parts of the ASF! - table
        SkAsfExamineAsfTable( pAC, IoC, pAcpiAsf, TableLength );
        //  calculate new FW image checksum
        pTmp32 = (SK_U32 *) pImage;
        for( i=0, FwCs=0; i<ASF_FLASH_SIZE-4; i+=4 )  {
            FwCs += *pTmp32;
            pTmp32++;
        }
        *pTmp32 = (~FwCs)+1;
    }  //  if( FwCs == 0  )  {  //  CS == 0 => O.K.
    else
        RetVal = ASF_ACPI_STATE_ERROR_FILE_CS;

    return( RetVal);
}
#endif

#if (0)
/*****************************************************************************
*
* SkAsfPatchGuid
*
* Description:  Ovverides the GUID in the fw image with the current
*               SMBios UUID.
*
* Notes:        none
*
* Context:      none
*
* Returns:       1: OK
*                0: UNDEFINED
*               <0: Error Codes
*
*/
SK_I8 SkAsfPatchGuid(
SK_AC   *pAC,
SK_IOC  IoC,
SK_U8 *pImage,
SK_U8 *pGuid )  {
    SK_I8           RetVal;
    SK_U32          i;
    SK_U8           *pAcpiAsf;
    SK_U8           FwImageCsOk;
    SK_U32          FwImageCs;
    SK_U32          FwCs;
    SK_U32          *pTmp32;

    RetVal = ASF_ACPI_STATE_OK;

    //  calculate CS of the FW image
    pTmp32 = (SK_U32 *) pImage;
    for( i=0, FwCs=0; i<ASF_FLASH_SIZE; i+=4 )  {
        FwCs += *pTmp32;
        pTmp32++;
    }
    if( FwCs == 0  )  {  //  CS == 0 => O.K.
        pAcpiAsf = (SK_U8 *) (pImage + ASF_FLASH_OFFS_GUID);
        //  write new GUID table to the FW image file
        for( i=0; i<16; i++ )  {
            *(pAcpiAsf+i) = *(pGuid+i);
        }
        //  calculate new FW image checksum
        pTmp32 = (SK_U32 *) pImage;
        for( i=0, FwCs=0; i<ASF_FLASH_SIZE-4; i+=4 )  {
            FwCs += *pTmp32;
            pTmp32++;
        }
        *pTmp32 = (~FwCs)+1;
    }  //  if( FwCs == 0  )  {  //  CS == 0 => O.K.
    else
        RetVal = ASF_ACPI_STATE_ERROR_FILE_CS;

    return( RetVal);
}
#endif

#if (0)
/*****************************************************************************
*
* SkAsfExamineAsfTable
*
* Description:  Ovverides the dynamic parts of the ASF! table in order to
*               avoid frequently flash writes caused by changing data.
*
* Notes:        none
*
* Context:      none
*
* Returns:       1: OK
*                0: UNDEFINED
*               <0: Error Codes
*
*/
void SkAsfExamineAsfTable(
SK_AC   *pAC,
SK_IOC  IoC,
SK_U8 *pAsf,
SK_U32 TableLength )  {
    SK_U8   *pTmp, *pRmcp, *x;
    SK_U8   Type;
    SK_U8   LastRecord;
    SK_U8   Cs;
    SK_U16  Length;
    SK_U16  i,j;

    // copy the ASF table to our asf-mib structure
    // and print the asf table to debug output
    pAC->AsfData.Mib.Acpi.length = TableLength;
    for( i=0; i<TableLength; i++ )  {
        if (i < ASF_ACPI_MAXBUFFLENGTH) {
            pAC->AsfData.Mib.Acpi.buffer[i] = *(pAsf+i);
        }
        if( i % 16 )  {
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,(" %02x", *(pAsf+i)));
        }
        else  {
            SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("\n %02x", *(pAsf+i)));
        }
    }
    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("\n"));

    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("\nASF! Table:\n"));
    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    L:%d Rev:%d Cs:0x%x\n", *((SK_U32 *)(pAsf+4)), *(pAsf+8),*(pAsf+9)  ));
    pTmp = pAsf+36;  //  offset to the 1st information record
    do {
        Type =          (*pTmp) & 0x7f;
        LastRecord =    (*pTmp) & 0x80;
        Length =        *((SK_U16 *) (pTmp+2));
        switch( Type )  {
            case ASF_RECORD_INFO:
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    ASF_INFO L:%d\n", *((SK_U16 *)(pTmp+2)) ));
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("      MinWd %d MinWait:%d\n", *(pTmp+4),*(pTmp+5) ));
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("      SystemId:   %02x\n", *((SK_U16 *)(pTmp+6)) ));
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("      ManufactId: %02x %02x %02x %02x Flags:0x%x\n",
                                                                      *(pTmp+8),*(pTmp+9),*(pTmp+10),*(pTmp+11),*(pTmp+12)  ));
                pTmp += Length;
                break;
            case ASF_RECORD_ALRT:
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    ASF_ALRT L:%d\n", *((SK_U16 *)(pTmp+4)) ));
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("      As:0x%x DAs:0x%x\n", *(pTmp+4),*(pTmp+5) ));
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("      NumAlrt:%d L:%d\n", *(pTmp+6),*(pTmp+7) ));
                x = pTmp+8;
                for( i=0; i<*(pTmp+6); i++ )  {
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("        Ev%d",i));
                    for( j=0; j<12; j++ )
                        SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,(" %02x", *(x++) ));
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("\n"));
                }
                pTmp += Length;
                break;
            case ASF_RECORD_RCTL:
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    ASF_RCTL L:%d\n", *((SK_U16 *)(pTmp+2)) ));
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("      NumCtrl:%d L:%d\n", *(pTmp+4),*(pTmp+5) ));
                x = pTmp+6;
                for( i=0; i<*(pTmp+4); i++ )  {
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("        Ctrl%d",i));
                    for( j=0; j<4; j++ )
                        SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,(" %02x", *(x++) ));
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("\n"));
                }
                pTmp += Length;
                break;
            case ASF_RECORD_RMCP:
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    ASF_RMCP L:%d\n", *((SK_U16 *)(pTmp+2)) ));
                pRmcp = pTmp+4;
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("        "));
                for( i=0; i<19; i++ )
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,(" %02x", *(pRmcp+i) ));
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("\n"));

                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("        RemCtrlCap:"));
                for( i=0; i<7; i++ )
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,(" %02x", *(pRmcp+i) ));
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("\n"));
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("        BootOptionCompletionCode: %02x\n", *(pRmcp+7)));
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("        IANA EnterpriseId:        %02x %02x %02x %02x\n",
                                                                                *(pRmcp+8),*(pRmcp+9), *(pRmcp+10), *(pRmcp+11)  ));
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("        SpecialCommand:           %02x\n", *(pRmcp+12)));
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("        SpecialCommandPar:        %02x %02x\n", *(pRmcp+13), *(pRmcp+14)));
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("        BootOptions:              %02x %02x\n", *(pRmcp+15), *(pRmcp+16)));
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("        OEM Parameter:            %02x %02x\n", *(pRmcp+17), *(pRmcp+18)));
                pRmcp +=    7;      //  skip Remote Control Capabilities
                //  if cpu is running, provide RMCP data to firmware
                if( pAC->AsfData.CpuAlive )
                    SkAsfSendRmcpData( pAC, IoC, pRmcp, 12 );
                for( i=0; i<12; i++ )
                    *(pRmcp++) = 0; //  erase dynamic part of the record
                pTmp += Length;
                break;
            case ASF_RECORD_ADDR:
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    ASF_ADDR L:%d SEEPROM Adr:0x%x NumAddr:%d\n",
                                                               *((SK_U16 *)(pTmp+2)), *(pTmp+4), *(pTmp+5) ));
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("      Addr:"));
                for( i=0; i<*(pTmp+5); i++ )
                    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,(" %02x", *(pTmp+6+i) ));
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("\n"));
                pTmp += Length;
                break;
            default:
                SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    UNKNOWN Type:0x%x L:%d\n", *pTmp, *((SK_U16 *)(pTmp+2)) ));
                LastRecord = 1;
                break;
        }
        if( LastRecord )
            break;
    } while ( (SK_U32)(pTmp-pAsf) < TableLength );

    //  calculate new checksum
    *(pAsf+9) = 0; //  delete old Cs
    for( Cs=0, i=0; i<TableLength; i++ )
        Cs += *(pAsf + i);
    *(pAsf+9) = (~Cs)+1;  //  write new Cs

#ifdef CS_CHECK
    //  just for a check
    for( Cs=0, i=0; i<TableLength; i++ )
        Cs += *(pAsf + i);
#endif  // CS_CHECK

    return;
}
#endif


SK_I8 SkAsfSendRmcpData(
SK_AC   *pAC,
SK_IOC  IoC,
SK_U8   *pData,
SK_U8   Length )  {
    SK_I8   RetVal;
    SK_U8   lRetCode;
    SK_U8   Ind, i;
    SK_U8   TmpBuffer [20];

    RetVal = 1;
    Ind = 0;

    if( Length <= 18 )  {
        TmpBuffer[Ind++] = YASF_HOSTCMD_ACPI_RMCP_DATA;
        TmpBuffer[Ind++] = 2 + Length;  // Length
        for( i=0; i<Length; i++ )
            TmpBuffer[Ind++] = *(pData + i );
        lRetCode = AsfHciSendData(pAC, IoC, TmpBuffer, 0, ASF_HCI_WAIT, 0 );
        if( lRetCode == HCI_EN_CMD_ERROR )
            RetVal = 0;
    }
    else
        RetVal = -1;

    return( RetVal );
}


#ifdef __cplusplus
}
#endif  /* __cplusplus */
/* End of file */


