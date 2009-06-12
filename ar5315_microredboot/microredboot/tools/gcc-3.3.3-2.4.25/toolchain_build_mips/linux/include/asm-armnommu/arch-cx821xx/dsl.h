/****************************************************************************
 *
 *  Name:			DSL.h
 *
 *  Description:	DSL routine header file
 *
 *  Copyright (c) 2001 Conexant
 *
 ****************************************************************************
 *
 *  $Author: gerg $
 *  $Revision: 1.1 $
 *  $Modtime: 11/08/02 12:50p $
 *
 ***************************************************************************/
#ifndef DSL_H
#define DSL_H

/*********************************************************/
/* Define the Base addresses of DSL registers            */
/*********************************************************/
/* DMA used for DSL */
#define pDSL_CTRL_DMA	( volatile UINT32* )0x340010
#define pDSL_TXCC_DMA	( volatile UINT32* )0x340014
#define pDSL_RXCC_DMA	( volatile UINT32* )0x340018
#define pDSL_CTRL			( volatile UINT32* )0x34001c
#define pDSL_ISR			( volatile UINT32* )0x340020
#define pDSL_IER			( volatile UINT32* )0x340024

#define pDSL_RXCC_DMA2 	( volatile UINT32* )0x340038
#define pDSL_RXTIMER	( volatile UINT32* )0x34003C

/*********************************************************/
/* Define DSL registers bit definitions.                 */
/*********************************************************/

/* Master DSL interrupt mask bit */
#define Int_DSL_RX_DMA 0x00000400
#define Int_DSL_TX_DMA 0x00000800
#define Int_DSL 	   0x00200000

/* DSL Control DMA Reg bit definitions */
#define DSLCTRDMA_TX_CLRPEND    BIT0
#define DSLCTRDMA_TX_CLRQWCNT   BIT1
#define DSLCTRDMA_RX_CLRPEND	  BIT2
#define DSLCTRDMA_RX_CLRQWCNT   BIT3

/*  These bit will not work if we decide to clear bit with shift position > 23 ( 24 bits shift ) */
/*  i.e  u &= ~DEFINE_CONST ==> u &= ~(DEFINE_CONST) */

/* DSL Control/Status Reg bit definitions */
#define DSLCTR_SRESET            BIT0
#define DSLCTR_IDLE_INSERT       BIT1
#define DSLCTR_STREAM            BIT2
#define DSLCTR_AIE				   BIT3
#define DSLCTR_UNDERRUN_DIRECT   BIT4
#define DSLCTR_DMAHEADER         BIT10
#define DSLCTR_DSL_LPBK  			BIT12
#define DSLCTR_TXENABLE		   	BIT25
#define DSLCTR_RXENABLE	   		BIT26
#define DSLCTR_RX_KICK           BIT27
#define DSLCTR_TX_IDLE				BIT30
#define DSLCTR_RX_IDLE				BIT31 

/* DSL Interrupt/Status Reg bit definitions */
#define DSLINT_RX_PKTXFRDONE	   BIT0
#define DSLINT_RX_OVERRUN			BIT1
#define DSLINT_RX_PENDFULL       BIT3
#define DSLINT_TX_PKTXFRDONE	   BIT4
#define DSLINT_TX_UNDERRUN       BIT5
#define DSLINT_TX_PENDZERO       BIT7
#define DSLINT_RX_THRESHOLD		BIT16		
#define DSLINT_RX_WATCHDOG			BIT17

#define DSLINT_STATUS_MASK       (DSLINT_RX_OVERRUN | DSLINT_TX_UNDERRUN | DSLINT_RX_THRESHOLD | DSLINT_RX_WATCHDOG)

/*********************************************************/
/* Define DSL DMA buffers map.                           */
/*********************************************************/

#define DSL_MAX_PACKET_SIZE      56
#define DSL_DMA_DATA             53
#define DSL_DMA_BLOCK_DWORDS     ( DSL_MAX_PACKET_SIZE / 4 )
#define DSL_DMA_BLOCK_QWORDS     ( DSL_MAX_PACKET_SIZE / 8 )

#define DSL_MAX_TX_BUFF          128

#define DSL_TX_LINK_SIZE         8
#define DSL_TX_FRAME_SIZE        (DSL_MAX_PACKET_SIZE + DSL_TX_LINK_SIZE)
#define DSL_TXBUFF_SIZE          (DSL_MAX_TX_BUFF * DSL_TX_FRAME_SIZE)

#define DSL_MAX_RXBUFF_SIZE      (DSL_MAX_PACKET_SIZE * 254)

#define DSL_DMA_RAM_START  SRAMSTART
#define DSL_DMA_RAM_END    SRAMEND

#define pDSL_DMA_TX_BUFF   (volatile UINT32 *) DSL_DMA_RAM_START
#define pDSL_DMA_RX_BUFF   (volatile UINT32 *) ((UINT8 *) pDSL_DMA_TX_BUFF + DSL_TXBUFF_SIZE)

#define DSL_MAX_RX_BUFF 254
//#define DSL_MAX_RX_BUFF 128

#define DSL_RXBUFF_SIZE    (DSL_MAX_RX_BUFF * DSL_MAX_PACKET_SIZE)

#define DSL_TIMEOUT_PACKET_TIME  3

/*********************************************************/
/* Define DSL variables.                                 */
/*********************************************************/

extern UINT32 DSL_ISR_img;
extern UINT32 DSL_IER_img;
extern UINT32 DSL_CTRL_img;

extern UINT32 DSLByteCount;
extern RXBUFFERSTRUCT DslTxBuffer;

extern RXBUFFERSTRUCT DslRxBuffer;

typedef struct tagDSL_PACKET_RXBLK {
	volatile UINT32 * ptr;
	UINT32 ID;
} DSL_PACKET_RXBLK, *PDSL_PACKET_RXBLK;

extern DSL_PACKET_RXBLK Dsl_Packet_Rxblk[DSL_MAX_RX_BUFF];

/*********************************************************/
/* Define DSL functions.                                 */
/*********************************************************/

void SetDSLInterrupts(void);
BOOL DSLWriteBuffer(volatile UINT32* RxBuff,UINT32 ByteCount);
void DSL_init(void);
void InitDSLTxBuffer(void);
void InitDSLRxBuffer(void);

void DSL_ClearStatus(UINT8 uDMACh);
void DSL_EnableInterface(UINT8 uDMACh);

void DSL_Enable_Int(UINT8 uDMACh);
void DSL_Disable_Int(UINT8 uDMACh);
void DSL_TX_Int(UINT8 IntState);
void DSL_TXUnderrun_Int(UINT8 IntState);
void DSL_RX_Int(UINT8 IntState);
void DSL_RXOverrun_Int(UINT8 IntState);
void DSLInitRxTimer(void);
void DSL_RXTimeout_Int(UINT8 IntState);
void DSL_RXThresh_Int(UINT8 IntState, UINT32 ThresholdLevel);
UINT32 DSL_DMAEnable(UINT8 uDMACh);
UINT32 DSL_DMADisable(UINT8 uDMACh);
#endif
