/******************************************************************************
 *
 * Name:    fwhci.c
 * Project: fwcommon
 * Version: $Revision: #5 $
 * Date:    $Date: 2010/11/04 $
 * Purpose: firmware host communication interface
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

void FwHciStateMachine(
	SK_AC  *pAC,
	SK_IOC IoC,
	SK_U8  ToEna);

static SK_U8 FwHciSendMessage(
	SK_AC  *pAC,
	SK_IOC IoC,
	SK_U8  *message,
	SK_U8  length,
	SK_U8  ExpectResponse,
	SK_U8  Wait);

#ifdef DASH_APP
/******************************************************************************
 *
 *	Converting functions for HCI communication (network byte order)
 *
 * Description:
 *
 *  static SK_U16 FwfRaw16ToPlain16(SK_U8 *data)
 *  static void FwPlain16ToRaw16(SK_U16 val, SK_U8 *data)
 *  static SK_U32 FwRaw32ToPlain32(SK_U8 *data)
 *  static void FwPlain32ToRaw32(SK_U32 val, SK_U8 *data)
 *  static SK_U64 FwfRaw64ToPlain64(SK_U8 *data)
 *  static void FwPlain64ToRaw64(SK_U64 val, SK_U8 *data) 
 *
 * Returns:
 *
 */

static SK_U16 FwRaw16ToPlain16(SK_U8 *data)
{
	SK_U16 TmpVal16 = 0x0000;	
	TmpVal16  = ((SK_U16) *(data+0) << 8);
	TmpVal16 |= ((SK_U16) *(data+1) << 0);
	return NTOHS(TmpVal16);
}

static void FwPlain16ToRaw16(SK_U16 val, SK_U8 *data)
{
	SK_U16 TmpVal = NTOHS(val);	
	*(data++) = (SK_U8) ((TmpVal >>  8) & 0xFF);
	*(data++) = (SK_U8) ((TmpVal >>  0) & 0xFF);
}

static SK_U32 FwRaw32ToPlain32(SK_U8 *data)
{
	SK_U32 TmpVal32 = 0x00000000;
	TmpVal32  = ((SK_U32) *(data+0) << 24);
	TmpVal32 |= ((SK_U32) *(data+1) << 16);
	TmpVal32 |= ((SK_U32) *(data+2) << 8);
	TmpVal32 |= ((SK_U32) *(data+3) << 0);
	return NTOHL(TmpVal32);
}

static void FwPlain32ToRaw32(SK_U32 val, SK_U8 *data)
{
	SK_U32 TmpVal = NTOHL(val);
	*(data++) = (SK_U8) ((TmpVal >> 24) & 0xFF);
	*(data++) = (SK_U8) ((TmpVal >> 16) & 0xFF);
	*(data++) = (SK_U8) ((TmpVal >>  8) & 0xFF);
	*(data++) = (SK_U8) ((TmpVal >>  0) & 0xFF);
}

static SK_U64 FwRaw64ToPlain64(SK_U8 *data)
{
	SK_U64 TmpVal64 = 0;
#ifdef SK_LITTLE_ENDIAN
	TmpVal64  = ((SK_U64) *(data+7) << 56);
	TmpVal64 |= ((SK_U64) *(data+6) << 48);
	TmpVal64 |= ((SK_U64) *(data+5) << 40);
	TmpVal64 |= ((SK_U64) *(data+4) << 32);
	TmpVal64 |= ((SK_U64) *(data+3) << 24);
	TmpVal64 |= ((SK_U64) *(data+2) << 16);
	TmpVal64 |= ((SK_U64) *(data+1) << 8);
	TmpVal64 |= ((SK_U64) *(data+0) << 0);
#else
	TmpVal64  = ((SK_U64) *(data+0) << 56);
	TmpVal64 |= ((SK_U64) *(data+1) << 48);
	TmpVal64 |= ((SK_U64) *(data+2) << 40);
	TmpVal64 |= ((SK_U64) *(data+3) << 32);
	TmpVal64 |= ((SK_U64) *(data+4) << 24);
	TmpVal64 |= ((SK_U64) *(data+5) << 16);
	TmpVal64 |= ((SK_U64) *(data+6) << 8);
	TmpVal64 |= ((SK_U64) *(data+7) << 0);
#endif	
	return TmpVal64;
}

static void FwPlain64ToRaw64(SK_U64 val, SK_U8 *data)
{
#ifdef SK_LITTLE_ENDIAN
	*(data+0) = (SK_U8) ((val >> 56) & 0xFF);
	*(data+1) = (SK_U8) ((val >> 48) & 0xFF);
	*(data+2) = (SK_U8) ((val >> 40) & 0xFF);
	*(data+3) = (SK_U8) ((val >> 32) & 0xFF);
	*(data+4) = (SK_U8) ((val >> 24) & 0xFF);
	*(data+5) = (SK_U8) ((val >> 16) & 0xFF);
	*(data+6) = (SK_U8) ((val >>  8) & 0xFF);
	*(data+7) = (SK_U8) ((val >>  0) & 0xFF);
#else
	*(data+7) = (SK_U8) ((val >> 56) & 0xFF);
	*(data+6) = (SK_U8) ((val >> 48) & 0xFF);
	*(data+5) = (SK_U8) ((val >> 40) & 0xFF);
	*(data+4) = (SK_U8) ((val >> 32) & 0xFF);
	*(data+3) = (SK_U8) ((val >> 24) & 0xFF);
	*(data+2) = (SK_U8) ((val >> 16) & 0xFF);
	*(data+1) = (SK_U8) ((val >>  8) & 0xFF);
	*(data+0) = (SK_U8) ((val >>  0) & 0xFF);
#endif	
}

static void FwPrintIpData(
	SK_AC   *pAC,
	SK_U32	Modules,
	SK_U32	Categories)  {
	SK_U16 i;
	TAsfDashIpInfoExt *IpConfig	=  &pAC->FwApp.Mib.IpInfoExt;

	SK_DBG_MSG(pAC, Modules, Categories, ("\nIP Configuration\n"));
	for( i=0; i<IpConfig->NumIpAddresses; i++  )  {
		if( i < ASF_DASH_IP_INFO_NUM_IPADDR )  {
			SK_DBG_MSG(pAC, Modules, Categories, ("  IP %d       Addr:0x%x Mask:0x%x\n", 
					i+1, IpConfig->IpAddress[i].Address, IpConfig->IpAddress[i].SubnetMask));
		}
	}
	
	for( i=0; i<IpConfig->NumIpGateways; i++  )  {
		if( i < ASF_DASH_IP_INFO_NUM_IPADDR )  {
			SK_DBG_MSG(pAC, Modules, Categories, ("  Gateway %d: 0x%x\n", 
					i+1, IpConfig->IpGateway[i] ));
		}
	}

	SK_DBG_MSG(pAC, Modules, Categories,         ("  DHCP Server %d: 0x%x\n", 
					i+1, IpConfig->DhcpServer ));
}
#endif /*DASH_APP*/

/*****************************************************************************
*
* FwHciGetData
*
* Description:
*
* Returns:
*
*/
SK_U8 FwHciGetData(
	SK_AC *pAC,  /* Pointer to adapter context */
	SK_U8 **pHciRecBuf ) {

	*pHciRecBuf = pAC->FwCommon.Hci.ReceiveBuf;

	SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("FwHciGetData\n"));
	SK_DBG_DMP(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, pAC->FwCommon.Hci.ReceiveBuf, pAC->FwCommon.Hci.ReceiveBuf[1] );
	return( 1 );
}

/*****************************************************************************
*
* FwHciGetState
*
* Description:
*
* Returns:
*
*/
SK_U8 FwHciGetState(
	SK_AC *pAC) {  /* Pointer to adapter context */

	SK_U8	Stat;
	SK_U64	DiffTime;

	Stat = pAC->FwCommon.Hci.Status;
	if ( Stat == HCI_EN_CMD_READY ) {

		pAC->FwCommon.Hci.Status = HCI_EN_CMD_IDLE;
		DiffTime = SkOsGetTime(pAC) - pAC->FwCommon.Hci.Time;
		SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    *** Ready -> Cycles:%d Time:%lums\n",
													   pAC->FwCommon.Hci.Cycles, DiffTime / (10*1000) ));
	}
	if ( Stat == HCI_EN_CMD_ERROR ) {
		DiffTime = SkOsGetTime(pAC) - pAC->FwCommon.Hci.Time;

		pAC->FwCommon.Hci.To = 0;		

		SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    *** Error -> Cycles:%d Time:%lums\n",
													   pAC->FwCommon.Hci.Cycles, DiffTime / (10*1000) ));
	}
	return( Stat );
}

/*****************************************************************************
*
* FwHciFinishCommand
*
* Description:
*	Waits some time if the current command finishes.
*
* Returns:
*
*/
SK_U8 FwHciFinishCommand(
	SK_AC *pAC,    /* Pointer to adapter context */
	SK_IOC IoC) {  /* IO context */
	
	SK_U64  StartTime;
	SK_U64  CurrTime;
	SK_U64  TmpTime;
	SK_U8   RetCode;

	SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("FwHciFinishCommand\n"));
	StartTime = SkOsGetTime(pAC);
	TmpTime = StartTime + ASF_HCI_WAIT_TICKS;

	do {
		CurrTime = SkOsGetTime(pAC);

		/* mlindner: Fix for a MVista timer problem */
		/* if ( CurrTime > TmpTime ) { */

			FwHciStateMachine( pAC, IoC, 0 );
			TmpTime = CurrTime + ASF_HCI_WAIT_TICKS;
		/* } */
		RetCode = FwHciGetState( pAC );
		if ( (CurrTime - StartTime) > (SK_TICKS_PER_SEC*5) ) {

			RetCode = HCI_EN_CMD_ERROR;
			break;
		}
	} while( (RetCode != HCI_EN_CMD_READY) && (RetCode != HCI_EN_CMD_ERROR) );

	if ( (RetCode != HCI_EN_CMD_READY) ) {

		pAC->FwCommon.Hci.Status = HCI_EN_CMD_IDLE;
		SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    Command not confirmed RetCode:0x%x\n", RetCode ));
	}

	return( RetCode );
}

/*****************************************************************************
*
* FwHciSendMessage
*
* Description:
*
* Returns:
*
*/
static SK_U8 FwHciSendMessage(
	SK_AC  *pAC,  /* Pointer to adapter context */
	SK_IOC IoC,   /* IO context */
	SK_U8  *message,
	SK_U8  length,
	SK_U8  ExpectResponse,
	SK_U8  Wait ) {

	SK_U8 RetCode;
	SK_U8 i;

	RetCode = 0;

	if ( length > ASF_HCI_TRA_BUF_SIZE ) {

		return( RetCode );
	}

	if ( pAC->FwCommon.Hci.Status == HCI_EN_CMD_IDLE ) {

		/*  SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    Send Message\n" )); */

		SK_OUT32( IoC, ASF_HCI_CMDREG, (SK_U32) 0 );
		SK_OUT32( IoC, ASF_HCI_DATAREG, (SK_U32) 0 );

		for( i=0; i<length; i++ ) {
			pAC->FwCommon.Hci.TransmitBuf[i] = message[i];
		}
		pAC->FwCommon.Hci.SendLength = length;
		if( ExpectResponse ) {
			pAC->FwCommon.Hci.ExpectResponse = 1;
		}
		else {
			pAC->FwCommon.Hci.ExpectResponse = 0;
		}
		pAC->FwCommon.Hci.Status = HCI_EN_CMD_WRITING;
		pAC->FwCommon.Hci.OldCmdReg = 0;
		pAC->FwCommon.Hci.SendIndex = 0;
		pAC->FwCommon.Hci.Cycles = 0;
		pAC->FwCommon.Hci.To = 0;
		pAC->FwCommon.Hci.Time = SkOsGetTime( pAC );
		RetCode = 1;  /*  successfull */
	}
	else {
		SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    Send Message -> Not Idle\n" ));
	}

	if ( !Wait ) {

		/*  SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    Send Message -> Start Timer\n" )); */
		/*SK_MEMSET((char *)&EventParam, 0, sizeof(EventParam));

		SkTimerStart(pAC, IoC, &pAC->FwCommon.Hci.AsfTimerHci,
					 10000, SKGE_ASF, SK_ASF_EVT_TIMER_HCI_EXPIRED,
					 EventParam);*/

		/* Start state machine immediatetly (dr)*/
		FwHciStateMachine(pAC, IoC, 1);
	}
	else {
		/* Added (ts) */
		FwHciStateMachine(pAC, IoC, 0);
	}
	return( RetCode );
}

#ifdef DASH_APP
/*****************************************************************************
*
* FwfHciPrepareMessage
*
* Description:
*
* Returns:
*
*/
SK_U8 FwHciPrepareMessage(
	SK_AC  *pAC,  /* Pointer to adapter context */
	SK_U8  Cmd,
	SK_U8  *pBuf,
	SK_U8  BufLen ) {
	SK_U8  *pBufTmp;
	SK_U16 i;
	SK_U8  CalcLen = 0;
	SK_U8  Len = 0;

	SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,
		("FwHciPrepareMessage: Cmd:%d L:%d\n", Cmd, BufLen));

	if ( BufLen < 2 ) {
		return 0;
	}

	pBufTmp = pBuf;
	*(pBuf++) = Cmd;
	*(pBuf++) = 0;  /*    will be filled in later */
	CalcLen += 2;
	switch( Cmd ) {
		case YASF_HOSTCMD_CFG_SET_EXT_IP_INFO:

			CalcLen += 	(pAC->FwApp.Mib.IpInfoExt.NumIpAddresses*sizeof( SK_U32)*2)+
						(pAC->FwApp.Mib.IpInfoExt.NumIpGateways*sizeof(SK_U32))+
						(pAC->FwApp.Mib.IpInfoExt.NumDnsServer*sizeof(SK_U32))+
						(2*sizeof(SK_U32))+ (2*sizeof(SK_U64))+4;

			if ( BufLen < CalcLen ) {
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,
					("    ERROR: BufLen (%d) < CalcLen (%d)\n", BufLen, CalcLen ));
				break;
			}

			/*   write IP addresses */
			*(pBuf++) = pAC->FwApp.Mib.IpInfoExt.NumIpAddresses;

			for( i=0; i<pAC->FwApp.Mib.IpInfoExt.NumIpAddresses; i++ ) {
				FwPlain32ToRaw32( pAC->FwApp.Mib.IpInfoExt.IpAddress[i].Address, pBuf );
				pBuf += sizeof( SK_U32 );
			}
			/*   write IP subnet masks				 */
			*(pBuf++) = pAC->FwApp.Mib.IpInfoExt.NumIpAddresses;

			for( i=0; i<pAC->FwApp.Mib.IpInfoExt.NumIpAddresses; i++ ) {

				FwPlain32ToRaw32( pAC->FwApp.Mib.IpInfoExt.IpAddress[i].SubnetMask, pBuf );
				pBuf += sizeof( SK_U32 );
			}
			/*   write IP Gateways */
			*(pBuf++) = pAC->FwApp.Mib.IpInfoExt.NumIpGateways;

			for( i=0; i<pAC->FwApp.Mib.IpInfoExt.NumIpGateways; i++ ) {

				FwPlain32ToRaw32( pAC->FwApp.Mib.IpInfoExt.IpGateway[i], pBuf );
				pBuf += sizeof( SK_U32 );
			}
			FwPlain32ToRaw32( pAC->FwApp.Mib.IpInfoExt.DhcpEnabled, pBuf );
			pBuf += sizeof( SK_U32 );
			FwPlain32ToRaw32( pAC->FwApp.Mib.IpInfoExt.DhcpServer, pBuf );
			pBuf += sizeof( SK_U32 );
			FwPlain64ToRaw64( pAC->FwApp.Mib.IpInfoExt.DhcpLeaseObtained, pBuf );
			pBuf += sizeof( SK_U64 );
			FwPlain64ToRaw64( pAC->FwApp.Mib.IpInfoExt.DhcpLeaseExpires, pBuf );
			pBuf += sizeof( SK_U64 );
			/*   write DNS server */
			*(pBuf++) = pAC->FwApp.Mib.IpInfoExt.NumDnsServer;
			for( i=0; i<pAC->FwApp.Mib.IpInfoExt.NumDnsServer; i++ ) {
				FwPlain32ToRaw32( pAC->FwApp.Mib.IpInfoExt.DnsServer[i], pBuf );
				pBuf += sizeof( SK_U32 );
			}
			Len = (SK_U8) (pBuf-pBufTmp);
			*(pBufTmp+1) = Len;
			break;
			
	}

	if ( Len > 0 )
		SK_DBG_DMP( pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, pBufTmp, Len );

	return Len;
}

/*****************************************************************************
*
* FwHciProcessData
*
* Description:
*
* Returns:
*
*/
SK_U8 FwHciProcessData(
	SK_AC *pAC,  /* Pointer to adapter context */
	SK_U8 Cmd,
	SK_U8 *pBuf ) {

	SK_U8  RetVal = 0;
	SK_U8  RecCmd, Length;
	SK_U16 i;

	RecCmd = *(pBuf++);
	Length = *(pBuf++);

	SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, 
		("FwHciProcessData: Cmd:%d RecCmd%d L:%d\n", Cmd, RecCmd, Length ));

	if ( Cmd == RecCmd ) {
		switch( RecCmd ) {

			case YASF_HOSTCMD_CFG_GET_EXT_IP_INFO:

					/*   read IP addresses */
				pAC->FwApp.Mib.IpInfoExt.NumIpAddresses = *(pBuf++);
				Length--;
				if ( Length >= (pAC->FwApp.Mib.IpInfoExt.NumIpAddresses*sizeof(SK_U32)) ) {

					for( i=0; i<pAC->FwApp.Mib.IpInfoExt.NumIpAddresses; i++ ) {

						if ( i < ASF_DASH_IP_INFO_NUM_IPADDR ) {

							pAC->FwApp.Mib.IpInfoExt.IpAddress[i].Address = FwRaw32ToPlain32( pBuf ); 
						}
						pBuf+=sizeof(SK_U32);
						Length-=sizeof(SK_U32);
					}
				}
				else {
					break;  /*   failure */
				}

				/*   read IP subnet masks */
				if ( pAC->FwApp.Mib.IpInfoExt.NumIpAddresses == *(pBuf) ) {
					
					pBuf++;
					Length--;

					if ( Length >= (pAC->FwApp.Mib.IpInfoExt.NumIpAddresses*sizeof(SK_U32)) ) {

						for( i=0; i<pAC->FwApp.Mib.IpInfoExt.NumIpAddresses; i++ ) {

							if ( i < ASF_DASH_IP_INFO_NUM_IPADDR ) {

								pAC->FwApp.Mib.IpInfoExt.IpAddress[i].SubnetMask = FwRaw32ToPlain32( pBuf );
							}

							pBuf+=sizeof(SK_U32);
							Length-=sizeof(SK_U32);
						}
					}
					else {
						break;
					}
				}
				else {
					break;  /*   failure */
				}

				/*   read IP gateways */
				pAC->FwApp.Mib.IpInfoExt.NumIpGateways = *(pBuf++);
				Length--;

				if ( Length >= (pAC->FwApp.Mib.IpInfoExt.NumIpGateways*sizeof(SK_U32)) ) {

					for( i=0; i<pAC->FwApp.Mib.IpInfoExt.NumIpGateways; i++ ) {
						
						if ( i < ASF_DASH_IP_INFO_NUM_IPADDR ) {

							pAC->FwApp.Mib.IpInfoExt.IpGateway[i] = FwRaw32ToPlain32( pBuf );
						}

						pBuf+=sizeof(SK_U32);
						Length-=sizeof(SK_U32);
					}
				}
				else {
					break;  /*   failure */
				}

				/*   read DHCP enable */
				if ( Length >= sizeof(SK_U32) ) {

					pAC->FwApp.Mib.IpInfoExt.DhcpEnabled = FwRaw32ToPlain32( pBuf );
					pBuf+=sizeof(SK_U32);
					Length-=sizeof(SK_U32);
				}
				else 
					break;  /*   failure */

				/*   read DHCP server */
				if ( Length >= sizeof(SK_U32) ) {
					pAC->FwApp.Mib.IpInfoExt.DhcpServer = FwRaw32ToPlain32( pBuf );
					pBuf+=sizeof(SK_U32);
					Length-=sizeof(SK_U32);
				}
				else 
					break;  /*   failure */

				/*   read DHCP Lease Obtained */
				if ( Length >= sizeof(SK_U64) ) {
					pAC->FwApp.Mib.IpInfoExt.DhcpLeaseObtained = FwRaw64ToPlain64( pBuf );
					pBuf+=sizeof(SK_U64);
					Length-=sizeof(SK_U64);
				}
				else 
					break;  /*   failure */

				/*   read DHCP Lease Expires */
				if ( Length >= sizeof(SK_U64) ) {
					pAC->FwApp.Mib.IpInfoExt.DhcpLeaseExpires = FwRaw64ToPlain64( pBuf );
					pBuf+=sizeof(SK_U64);
					Length-=sizeof(SK_U64);
				}
				else 
					break;  /*   failure */

				/*   read DNS servers */
				pAC->FwApp.Mib.IpInfoExt.NumDnsServer = *(pBuf++);
				Length--;
				if ( Length >= (pAC->FwApp.Mib.IpInfoExt.NumDnsServer*sizeof(SK_U32)) ) {
					for( i=0; i<pAC->FwApp.Mib.IpInfoExt.NumDnsServer; i++ ) {
						if ( i < ASF_DASH_IP_INFO_NUM_IPADDR ) {
							pAC->FwApp.Mib.IpInfoExt.DnsServer[i] = FwRaw32ToPlain32( pBuf ); 
						}
						pBuf+=sizeof(SK_U32);
						Length-=sizeof(SK_U32);
					}
				}
				else 
					break;  /*   failure */

				FwPrintIpData( pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL);
				RetVal = 1;  /*   receive cmd successfully parsed */
				break;

			default:
				break;
		}
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,
				("FwHciProcessData: YASF_HOSTCMD_CFG_GET_EXT_IP_INFO: >"));

	return RetVal;
}
#endif /*DASH_APP*/

/*****************************************************************************
*
* FwfHciSendCommand
*
* Description:
*
* Returns:
*
*/
SK_U8 FwHciSendCommand(
	SK_AC  *pAC,  /* Pointer to adapter context */
	SK_IOC IoC,   /* IO context */
	SK_U8  Command,
	SK_U8  Par1,
	SK_U8  Par2,
	SK_U8  ExpectResponse,
	SK_U8  Wait,
	SK_U8  Retry ) {

	SK_U8 Message[4];
	SK_U8 RetCode;

	if ( Wait && ( pAC->FwCommon.Hci.Status != HCI_EN_CMD_IDLE )) {

		/* NOTE: This may take some time. */
		(void) FwHciFinishCommand(pAC, IoC);
	}

	do {
		if ( pAC->FwCommon.Hci.Status == HCI_EN_CMD_IDLE ) {
			
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    Send Command cmd:0x%x p1:0x%x p2:0x%x wait:%d Retry:%d\n",
															Command, Par1, Par2, Wait, Retry ));
			Message[0] = Command;
			Message[1] = 2;  /*   Length */
			Message[2] = Par1;
			Message[3] = Par2;
			RetCode = FwHciSendMessage(pAC, IoC, Message, 4, ExpectResponse, Wait );
		}

		if ( Wait ) {

			RetCode = FwHciFinishCommand(pAC, IoC);
		}  /*   if ( wait... */
		else {
			RetCode = FwHciGetState( pAC );
			if ( (RetCode == HCI_EN_CMD_ERROR) ) {

				pAC->FwCommon.Hci.Status = HCI_EN_CMD_IDLE;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    Command not confirmed RetCode:0x%x\n", RetCode ));
			}
		}

		if ( Retry > 0 )
			Retry--;
		else {
			break;
		}

	} while ( Wait && (RetCode != HCI_EN_CMD_READY) );

	if ( (RetCode == HCI_EN_CMD_READY) ) {
		SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    Command successfully conveyed\n"));
	}

	return( RetCode );
}

/*****************************************************************************
*
* FwfHciSendData
*
* Description:
*
* Returns:
*
*/
SK_U8 FwHciSendData(
	SK_AC  *pAC,  /* Pointer to adapter context */
	SK_IOC IoC,   /* IO context */
	SK_U8  *Buffer,
	SK_U8  ExpectResponse,
	SK_U8  Wait,
	SK_U8  Retry ) {

	SK_U8 RetCode;
	SK_U8 Length;
	SK_U8 Command;

	do {
		if ( pAC->FwCommon.Hci.Status == HCI_EN_CMD_IDLE ) {
			Command = *(Buffer + 0 );
			Length =  *(Buffer + 1);
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    Send Data cmd:0x%x Length:%d wait:%d Retry:%d\n",
															Command, Length, Wait, Retry ));
			SK_DBG_DMP(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, Buffer, Length );
			RetCode = FwHciSendMessage(pAC, IoC, Buffer, Length, ExpectResponse, Wait );
		}
		if ( Wait ) {
			RetCode = FwHciFinishCommand(pAC, IoC);
		}  /*   if ( wait... */
		else  {
			RetCode = FwHciGetState( pAC );
			if ( RetCode == HCI_EN_CMD_ERROR ) {
				pAC->FwCommon.Hci.Status = HCI_EN_CMD_IDLE;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    Data not confirmed RetCode:0x%x\n", RetCode ));
			}
		}
		if ( Retry > 0 ) {
			Retry--;
		}
		else
			break;
	} while ( Wait && (RetCode != HCI_EN_CMD_READY) );

	if ( RetCode == HCI_EN_CMD_READY ) {
		SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    Data successfully conveyed\n"));
	}

	return( RetCode );
}

/*****************************************************************************
*
* FwHciStateMachine
*
* Description:
*
* Returns:
*
*/
void FwHciStateMachine(
	SK_AC  *pAC,   /* Pointer to adapter context */
	SK_IOC IoC,    /* IO context */
	SK_U8  ToEna ) {

	SK_EVPARA EventParam;
	SK_U32    Cmd;
	SK_U32    SendValue;
	SK_U32    RecValue;
	SK_U32    TmpVal32;

	if (pAC->FwCommon.Hci.Status == HCI_EN_CMD_IDLE) {
		return;
	}

	pAC->FwCommon.Hci.Cycles++;

	/* Check, whether there is something to do */
	SK_IN32( IoC, ASF_HCI_CMDREG, &Cmd );

	if ( pAC->FwCommon.Hci.OldCmdReg != Cmd ) {

		pAC->FwCommon.Hci.OldCmdReg = Cmd;

		SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_IRQ,("FwHciStateMachine CmdReg: 0x%x\n", Cmd ));
	}

	if ((Cmd & (ASF_HCI_READ | 
				ASF_HCI_WRITE | 
				ASF_HCI_CMD_RD_READY | 
				ASF_HCI_CMD_WR_READY | 
				ASF_HCI_UNSUCCESS)) == 0) {

		/*  SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("FwHciStateMachine State: 0x%x\n", pAC->FwCommon.Hci.Status )); */
		switch( pAC->FwCommon.Hci.Status ) {

			case HCI_EN_CMD_WRITING:
				SendValue = 0;
				if (pAC->FwCommon.Hci.SendLength > 0) {

					pAC->FwCommon.Hci.SendLength--;
					SendValue = ((SK_U32)pAC->FwCommon.Hci.TransmitBuf[pAC->FwCommon.Hci.SendIndex +0]) << 24;

					if (pAC->FwCommon.Hci.SendLength > 0) {

						pAC->FwCommon.Hci.SendLength--;
						SendValue += ((SK_U32)pAC->FwCommon.Hci.TransmitBuf[pAC->FwCommon.Hci.SendIndex +1]) << 16;
						
						if (pAC->FwCommon.Hci.SendLength > 0) {

							pAC->FwCommon.Hci.SendLength--;
							SendValue += ((SK_U32)pAC->FwCommon.Hci.TransmitBuf[pAC->FwCommon.Hci.SendIndex +2]) << 8;
							if (pAC->FwCommon.Hci.SendLength > 0) {

								pAC->FwCommon.Hci.SendLength--;
								SendValue += ((SK_U32)pAC->FwCommon.Hci.TransmitBuf[pAC->FwCommon.Hci.SendIndex +3]) << 0;
							}
						}
					}
				}

				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_IRQ,("        SendValue: 0x%x l:%d\n",
															  SendValue, pAC->FwCommon.Hci.SendLength ));
				SK_OUT32( IoC, ASF_HCI_DATAREG, SendValue);

				if (pAC->FwCommon.Hci.SendLength == 0) {

					SK_OUT32( IoC, ASF_HCI_CMDREG, ASF_HCI_WRITE | ASF_HCI_CMD_WR_READY | ASF_HCI_UNSUCCESS | (pAC->FwCommon.Hci.SendIndex/4));
					pAC->FwCommon.Hci.Status = HCI_EN_CMD_WAIT;
				}
				else {
					SK_OUT32( IoC, ASF_HCI_CMDREG, ASF_HCI_WRITE | ASF_HCI_UNSUCCESS | (pAC->FwCommon.Hci.SendIndex/4));
					pAC->FwCommon.Hci.SendIndex += 4;
				}

				/*  report command to FW (interrupt wakes CPU from sleep mode) */
				SK_IN32( IoC, HCU_CCSR, &TmpVal32);
				SK_OUT32( IoC, HCU_CCSR, TmpVal32 | HCU_CCSR_SET_IRQ_HCU);

				break;

			case HCI_EN_CMD_WAIT:

				if ( pAC->FwCommon.Hci.ExpectResponse ) {

					pAC->FwCommon.Hci.Status = HCI_EN_CMD_READING;
					pAC->FwCommon.Hci.ReceiveIndex = 0;
					SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("        Wait for response\n" ));
					SK_OUT32( IoC, ASF_HCI_CMDREG, ASF_HCI_READ | ASF_HCI_UNSUCCESS | (pAC->FwCommon.Hci.ReceiveIndex/4));
				}
				else  {
					pAC->FwCommon.Hci.Status = HCI_EN_CMD_READY;
					pAC->FwCommon.Hci.ReceiveBuf[1] = 0;  /*Length*/
					SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("        Cmd Ready\n" ));
				}

				/*  report command to FW (interrupt wakes CPU from sleep mode) */
				SK_IN32( IoC, HCU_CCSR, &TmpVal32);
				SK_OUT32( IoC, HCU_CCSR, TmpVal32 | HCU_CCSR_SET_IRQ_HCU);
				break;

			case HCI_EN_CMD_READING:

				SK_IN32( IoC, ASF_HCI_DATAREG, &RecValue );
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_IRQ,("        RecValue: 0x%x\n", RecValue ));
				pAC->FwCommon.Hci.ReceiveBuf[pAC->FwCommon.Hci.ReceiveIndex+3] = (SK_U8) (RecValue >>  0) & 0x000000ff;
				pAC->FwCommon.Hci.ReceiveBuf[pAC->FwCommon.Hci.ReceiveIndex+2] = (SK_U8) (RecValue >>  8) & 0x000000ff;
				pAC->FwCommon.Hci.ReceiveBuf[pAC->FwCommon.Hci.ReceiveIndex+1] = (SK_U8) (RecValue >> 16) & 0x000000ff;
				pAC->FwCommon.Hci.ReceiveBuf[pAC->FwCommon.Hci.ReceiveIndex+0] = (SK_U8) (RecValue >> 24) & 0x000000ff;
				pAC->FwCommon.Hci.ReceiveIndex   += 4;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_IRQ,("        RecValue: 0x%x RecIndex:%d l:%d\n",
															  RecValue,pAC->FwCommon.Hci.ReceiveIndex, pAC->FwCommon.Hci.ReceiveBuf[1] ));

				if ( pAC->FwCommon.Hci.ReceiveBuf[1] > pAC->FwCommon.Hci.ReceiveIndex ) { /* check length*/
					SK_OUT32( IoC, ASF_HCI_CMDREG, ASF_HCI_READ | ASF_HCI_UNSUCCESS | (pAC->FwCommon.Hci.ReceiveIndex/4));
				}
				else  {
					SK_OUT32( IoC, ASF_HCI_CMDREG, ASF_HCI_CMD_RD_READY );
					pAC->FwCommon.Hci.Status = HCI_EN_CMD_READY_END;
				}

				/*  report command to FW (interrupt wakes CPU from sleep mode) */
				SK_IN32( IoC, HCU_CCSR, &TmpVal32);
				SK_OUT32( IoC, HCU_CCSR, TmpVal32 | HCU_CCSR_SET_IRQ_HCU);
				break;

			case HCI_EN_CMD_READY_END:
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_IRQ, ("FwHciStateMachine -> Read completed\n"));
				pAC->FwCommon.Hci.Status = HCI_EN_CMD_READY;
				break;

			case HCI_EN_CMD_READY:
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_IRQ, ("FwHciStateMachine -> HCI_EN_CMD_READY (%d)\n",pAC->FwCommon.Hci.Cycles));
				break;

		}  /*   switch( pAC->FwCommon.Hci.Status ) { */

		pAC->FwCommon.Hci.To = 0;

	}  /*   if ((Cmd & (ASF_HCI_READ | ASF_HCI_WRITE | ASF_HCI_CMD_RD_READY | ASF_HCI_CMD_WR_READY)) == 0) */
	else  {

		if ( ToEna ) {
			pAC->FwCommon.Hci.To++;
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_IRQ,("                    CmdReg: 0x%x TO:%d\n", Cmd, pAC->FwCommon.Hci.To ));
		}
		/*  Timeout ! */
		if ( pAC->FwCommon.Hci.To > ASF_HCI_TO ) {
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, ("FwHciStateMachine -> ERROR TO:%d\n",pAC->FwCommon.Hci.To));
			pAC->FwCommon.Hci.Status = HCI_EN_CMD_ERROR;
		}
	}

	if ( ToEna ) {
		if ( pAC->FwCommon.Hci.Status != HCI_EN_CMD_IDLE &&
			 pAC->FwCommon.Hci.Status != HCI_EN_CMD_READY &&
			 pAC->FwCommon.Hci.Status != HCI_EN_CMD_ERROR ) {
			SK_MEMSET((char *)&EventParam, 0, sizeof(EventParam));
/*             SkTimerStart(pAC, IoC, &pAC->FwCommon.Hci.AsfTimerHci,  *  100ms * */
/*                          100000, SKGE_ASF, SK_ASF_EVT_TIMER_HCI_EXPIRED, */
/*                          EventParam); */

			SkTimerStart(pAC, IoC, &pAC->FwCommon.Hci.AsfTimerHci,  /*  10ms */
						 100000, SKGE_ASF, SK_ASF_EVT_TIMER_HCI_EXPIRED,
						 EventParam);
		}
	}

	return;
}

