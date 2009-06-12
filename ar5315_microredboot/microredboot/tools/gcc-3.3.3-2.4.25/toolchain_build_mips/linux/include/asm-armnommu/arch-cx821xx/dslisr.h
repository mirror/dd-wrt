/****************************************************************************
 *
 *  Name:			DSLISR.h
 *
 *  Description:	DSL interrupt routines header file
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
#ifndef DSLISR_H
#define DSLISR_H

#define RX_PENDING_ITEM	128

#define DSL_RX_PENDING_FLD    0x0000FF00
#define DSL_RX_PENDING_SHIFT  8

#define DSL_RXBUFFSIZE_SHIFT  16

/*---------------------------------------------------------------------------
 *  Module Function Prototypes
 *-------------------------------------------------------------------------*/

void DSL_ERR_Handler( void);
void DSL_RX_Handler( void);

void DSL_RX_Overrun_ISR(void);
void DSL_TX_DMA_ISR( void );
void DSL_TX_Underrun_ISR(void);
void DSLRxBufferUpdate(void);

#endif
