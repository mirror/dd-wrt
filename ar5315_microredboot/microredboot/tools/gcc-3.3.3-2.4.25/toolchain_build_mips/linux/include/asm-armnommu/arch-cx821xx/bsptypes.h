/****************************************************************************
*
*	Name:			BSPTYPES.H
*
*	Description:	Application Manager Application routines declaration.
*
*	Copyright:		(c) 1999 Conexant Systems Inc.
*
*****************************************************************************
*  $Author: gerg $
*  $Revision: 1.1 $
*  $Modtime: 8/27/02 10:49a $
****************************************************************************/

#ifndef BSPTYPES_H
#define BSPTYPES_H

#include <linux/autoconf.h>

#include "OsDefs.h"

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef I_O
#define I_O
#endif


#ifndef INT32
#define INT32 int
#endif







enum {LITTLE_ENDIAN, BIG_ENDIAN};

#ifdef LITTLE_ENDIAN_MEMORY
	#define BYTE_ORIENTATION  LITTLE_ENDIAN
#else
	#define BYTE_ORIENTATION  BIG_ENDIAN
#endif


#define BIT0             0x00000001
#define BIT1             0x00000002
#define BIT2             0x00000004
#define BIT3             0x00000008
#define BIT4             0x00000010
#define BIT5             0x00000020
#define BIT6             0x00000040
#define BIT7             0x00000080
#define BIT8             0x00000100
#define BIT9             0x00000200
#define BIT10            0x00000400
#define BIT11            0x00000800
#define BIT12            0x00001000
#define BIT13            0x00002000
#define BIT14            0x00004000
#define BIT15            0x00008000
#define BIT15            0x00008000
#define BIT16            0x00010000
#define BIT17            0x00020000
#define BIT18            0x00040000
#define BIT19            0x00080000
#define BIT20            0x00100000
#define BIT21            0x00200000
#define BIT22            0x00400000
#define BIT23            0x00800000
#define BIT24            0x01000000
#define BIT25            0x02000000
#define BIT26            0x04000000
#define BIT27            0x08000000
#define BIT28            0x10000000
#define BIT29            0x20000000
#define BIT30            0x40000000
#define BIT31            0x80000000



#ifndef UINT32
#define UINT32 unsigned long
#endif

#ifndef SINT32
#define SINT32 signed long
#endif

#ifndef UINT16
#define UINT16 unsigned short int
#endif

#ifndef SINT16
#define SINT16 signed short int
#endif

#ifndef INT16
#define INT16 short int
#endif

#ifndef UINT8
#define UINT8 unsigned char
#endif

#ifndef INT8
#define INT8 unsigned char
#endif

#ifndef SINT8
#define SINT8 signed char
#endif

#define uint32_ONE 0x80000000L
#define uint16_ONE 0x8000

#define sint32_MAX 0x7FFFFFFFL
#define sint16_MAX 0x7FFF

#define uint32_MAX 0xFFFFFFFFUL
#define uint16_MAX 0xFFFFU

#define sint32_MIN 0x00000001L
#define sint16_MIN 0x0001

typedef void (* JOB_TYPE)(void);

typedef struct
{
   SINT16 real,imag;
}SINT16CMPLX;

typedef struct
{
   SINT32 real,imag;
}SINT32CMPLX;

#ifndef NULL
#define NULL 0
#endif

#ifndef FUNCPTR
//#define FUNCPTR UINT32
typedef int	(*FUNCPTR)(void);
#endif

#define FALSE              0
#define TRUE               1

typedef void * HBM_CHANNEL;
typedef char * PCHAR;

#define IVEC_TO_INUM(intVec)   ((int) (intVec))
#define INUM_TO_IVEC(intNum)   ((void *) (intNum))

typedef struct tagHWSTATESTRUCT {
	UINT32 eDeviceType;
  	UINT32 HWPowerMode;
	UINT32 eUSBCurrState;
	UINT32 eUSBPrevState;
	#if CONFIG_CNXT_ADSL || CONFIG_CNXT_ADSL_MODULE
		UINT32 eADSLLineState;
		UINT32 eADSLRequestLineState;
		UINT32 dwADSLDownRate;
		UINT32 dwADSLUpRate;
		UINT32 dwLineStatus;
	#endif
	UINT32 bSRAMState;
	UINT32 bSDRAMState;
	UINT32 bFLASHState;
	UINT32 bEEPROMState;
	UINT32 eUARTState;
	void   *g_pUARTChan;
	UINT32 bSelfTestState;
	UINT32 dwMACAddressLow;
	UINT32 dwMACAddressHigh;

	// stuff for the performance CP panel
	#if CONFIG_CNXT_ADSL || CONFIG_CNXT_ADSL_MODULE
		UINT32	Transmit_State;
		UINT32	Receive_State;
		UINT32	Process_State;
		UINT32	Up_SNR_Margin_Cur;
		UINT32	Up_SNR_Margin_Min;
		UINT32	Up_SNR_Margin_MinBin;
		UINT32	Down_SNR_Margin_Cur;
		UINT32	Down_SNR_Margin_Min;
		UINT32	Down_SNR_Margin_Min_Bin;
		UINT32	Up_Attn;
		UINT32	Down_Attn;
		UINT32	Tx_Power;
		UINT32	Up_Bits_Per_Frame;
		UINT32	Down_Bits_Per_Frame;
		UINT32	Startup_Attempts;
		UINT32	Up_CRC_Errors;
		UINT32	Down_CRC_Errors;
		UINT32	Up_FEC_Errors;
		UINT32	Down_FEC_Errors;
		UINT32	Up_HEC_Errors;
		UINT32	Down_HEC_Errors;
		UINT32	Retrain_Attempts;
	#endif

   UINT32   p_GRP0_INPUT_ENABLE_Image;
   UINT32   p_GRP1_INPUT_ENABLE_Image;
   UINT32   p_GRP2_INPUT_ENABLE_Image;

} HWSTATESTRUCT, *PHWSTATESTRUCT;


typedef struct tagRXBUFFERSTRUCT
{
  volatile UINT32 *pdwRxStart;
  volatile UINT32 *pdwRxEnd;
  volatile UINT32 *pdwRxPtr;
  volatile UINT8 *pcData; 
  UINT32 		BuffSize;
} RXBUFFERSTRUCT, *PRXBUFFERSTRUCT;


typedef struct tagEPINFOSTRUCT
{
  UINT32 bTxEnable;
  UINT32 bTxSendMode;
  UINT32 bTXWA;
  UINT32 TxIRQCount;
  UINT32 TxUnderrun; 
  UINT32 TxPacket;
  UINT32 TxTotalPending;   /* Total Pending count waiting to be sent to USB */
  UINT32 TxPendingInc;     /* Pending count on attempt to dynamically add to DMA Tx link */
  UINT32 TxRejectCnt;
  volatile UINT32 *pTxHead;
  volatile UINT32 *pTxTail;
  volatile UINT32 *pTxCurr;
  UINT32 TxBufferSize;
  UINT32 bRxEnable;
  UINT32 RxIRQCount;
  UINT32 RxIRQThresh;
  UINT32 RxTimeOut;
  UINT32 RxPacket;
  UINT32 RxTotalPending;   /* Total Pending count waiting to be sent from USB */
  UINT32 RxBadPacket;
  UINT32 RxOverrun;
} EPINFOSTRUCT, *PEPINFOSTRUCT;


typedef enum
{
   DEVICE_P5200X10,
   DEVICE_P5200X180,
   DEVICE_P5300,
   DEVICE_RUSHMORE,
   DEVICE_RUSHMORE_X45,
   DEVICE_RUSHMORE_X50,
   #ifdef DEVICE_YELLOWSTONE
   DEVICE_OLDFAITHFUL
   #endif
} eDeviceType;


typedef enum
{
   NO_UART,                /* No UART support */
   HW_UART,                /* HW 16550 UART   */
   VIRTUAL_UART            /* VIRTUAL UART over USB endpoint 3 */
} eUARTType;

#if USB_SYNC_EP_INT_STAT
extern UINT32  USB_IDAT_VALUE;
#endif

extern UINT32  USB_IDAT_IMAGE;

extern HWSTATESTRUCT HWSTATE;


#define NELEMENTS(array) \
        (sizeof(array) / sizeof((array)[0]))

#ifndef STATUS
#define STATUS int
#endif

#define ERROR (-1)
#define OK     0


#endif	//BSPTYPES_H
