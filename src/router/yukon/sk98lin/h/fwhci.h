/******************************************************************************
 *
 * Name:    fwhci.h
 * Project: fwcommon
 * Version: $Revision: #4 $
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

#ifndef __SK_FWHCI_H__
#define __SK_FWHCI_H__

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* ASF HCI Master */
#define ASF_HCI_READ            0x08000000
#define ASF_HCI_WRITE           0x04000000
#define ASF_HCI_CMD_RD_READY    0x02000000
#define ASF_HCI_CMD_WR_READY    0x01000000
#define ASF_HCI_UNSUCCESS       0x00800000
#define ASF_HCI_OFFSET          0x000000ff

#define ASF_HCI_CMDREG          0x0e70
#define ASF_HCI_DATAREG         0x0e78

#define ASF_HCI_WAIT            1
#define ASF_HCI_NOWAIT          0

#ifdef DASH_APP
#define ASF_HCI_WAIT_TICKS      10000
#else
#define ASF_HCI_WAIT_TICKS      1
#endif

/* #define ASF_HCI_TO              100     * 1s * */
/* #define ASF_HCI_TO              500     * 5s * */
#define ASF_HCI_TO              5000     /* WAR for DASH: 50s */

#define HCI_EN_CMD_IDLE             0
#define HCI_EN_CMD_WRITING          1
#define HCI_EN_CMD_READING          2
#define HCI_EN_CMD_WAIT             3
#define HCI_EN_CMD_READY            4
#define HCI_EN_CMD_ERROR            5
#define HCI_EN_CMD_READY_END        6

#define ASF_HCI_REC_BUF_SIZE    128
#define ASF_HCI_TRA_BUF_SIZE    128

/*  endianess depended macros */

#define REVERSE_16(x)   ((((x)<<8)&0xff00) + (((x)>>8)&0x00ff))

#define REVERSE_32(x)   ( ((((SK_U32)(x))<<24UL)&0xff000000UL) + \
                          ((((SK_U32)(x))<< 8UL)&0x00ff0000UL) + \
                          ((((SK_U32)(x))>> 8UL)&0x0000ff00UL) + \
                          ((((SK_U32)(x))>>24UL)&0x000000ffUL) )

#ifdef SK_LITTLE_ENDIAN
#define LITTLEENDIAN_TO_HOST_16(x)	(x)
#define LITTLEENDIAN_TO_HOST_32(x)	(x)
#else
#define LITTLEENDIAN_TO_HOST_16(x)	REVERSE_16(x)
#define LITTLEENDIAN_TO_HOST_32(x)	REVERSE_32(x)
#endif

#ifdef DASH_APP
#ifdef SK_LITTLE_ENDIAN
#define NTOHS(x) REVERSE_16(x)
#define HTONS(x) REVERSE_16(x)
#define NTOHL(x) REVERSE_32(x)
#define HTONL(x) REVERSE_32(x)
#else
#define NTOHS(x) (x)
#define HTONS(x) (x)
#define NTOHL(x) (x)
#define HTONL(x) (x)
#endif
#endif

typedef struct s_Hci
{
    SK_U32      To;
    SK_U8       Status;
    SK_U8       OldStatus;
    SK_U32      OldCmdReg;
    SK_U8       SendIndex;
    SK_U8       ReceiveIndex;
    SK_U8       SendLength;
    SK_U8       ReceiveLength;
    SK_U8       ExpectResponse;
    SK_U8       Cycles;
    SK_U64      Time;
    SK_U8       ReceiveBuf  [ASF_HCI_REC_BUF_SIZE];
    SK_U8       TransmitBuf [ASF_HCI_TRA_BUF_SIZE];
    SK_TIMER    AsfTimerHci;
} STR_HCI;

#ifdef DASH_APP
static SK_U16 FwRaw16ToPlain16(SK_U8 *data);
static void FwPlain16ToRaw16(SK_U16 val, SK_U8 *data);
static SK_U32 FwRaw32ToPlain32(SK_U8 *data);
static void FwPlain32ToRaw32(SK_U32 val, SK_U8 *data);
static SK_U64 FwRaw64ToPlain64(SK_U8 *data);
static void FwPlain64ToRaw64(SK_U64 val, SK_U8 *data);
static void FwPrintIpData(
	SK_AC   *pAC,
	SK_U32	Modules,
	SK_U32	Categories);
extern SK_U8 FwHciPrepareMessage(
	SK_AC	*pAC,
	SK_U8 	Cmd,
	SK_U8	*pBuf,
	SK_U8	BufLen );
extern SK_U8 FwHciProcessData(SK_AC *pAC, SK_U8	Cmd, SK_U8 *pBuf );
#endif /*DASH_APP */
extern SK_U8 FwHciSendCommand(
	SK_AC 	*pAC,
	SK_IOC	IoC,
	SK_U8	Command,
	SK_U8	Par1,
	SK_U8	Par2,
	SK_U8	ExpectResponse,
	SK_U8	Wait,
	SK_U8	Retry );
extern SK_U8 FwHciSendData(
	SK_AC	*pAC,
	SK_IOC	IoC,
	SK_U8	*Buffer,
	SK_U8	ExpectResponse,
	SK_U8	Wait,
	SK_U8	Retry );
extern void FwHciStateMachine(	SK_AC *pAC, SK_IOC IoC,	SK_U8 ToEna );
extern SK_U8 FwHciGetData(SK_AC *pAC, SK_U8 **pHciRecBuf );
extern SK_U8 FwHciGetState(SK_AC *pAC);
extern SK_U8 FwHciFinishCommand(SK_AC *pAC, SK_IOC IoC);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* __SK_FWHCI_H__ */

