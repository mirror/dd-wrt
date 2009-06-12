/****************************************************************************
*
*	Name:			cnxtadsl.h
*
*	Description:	
*
*	Copyright:		(c) 2001 Conexant Systems Inc.
*
*****************************************************************************
*  $Author: gerg $
*  $Revision: 1.1 $
*  $Modtime: 5/10/02 2:09p $
****************************************************************************/

#ifndef CNXTADSL_H
#define CNXTADSL_H

#include "bsptypes.h"
#include "cardinfo.h"
#include "hwiodmt.h"

/* Showtime rate */
#define SHOWTIME_SYS_CLK_MS_PERIOD  1000
/* Buffer manager rate */
#define BM_SYS_CLK_MS_PERIOD  (1000/2)

void CnxtAdslLEDTask( void *sem );

BOOL ADSL_Init(void);
UINT32 ADSL_UpdateState
(
	BOOL						InShowtime,
	UINT32						dwADSLDownRate,
	UINT32						dwADSLUpRate,
	UINT32						dwLineStatus,
	PHW_IO_TRANSCEIVER_STATUS_T	pHwIoTransceiverStatus
);

BOOL ADSL_RequestedLineState(BOOL Up);

UINT32 ADSL_GetRequestedLineState(void);
UINT32 ADSL_LinkStatus(void);
UINT32 ADSL_GetDownstreamDataRate(void);
UINT32 ADSL_GetUpstreamDataRate(void);
void ADSL_GetShowtimeParm(UINT32 index, UINT32* pdwValue);

void CardGetInfo(PCARDDATA pCardData);
void CardGetData(PCARDDATA pCardData);
void CardPutData(PCARDDATA pCardData);

#endif	// CNXTADSL_H

