/******************************************************************************

                              Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************/

#ifndef _DSL_CPE_BND_H_
#define _DSL_CPE_BND_H_

#ifdef __cplusplus
extern "C" {
#endif

/** \file
   Bonding implementation/interface for CPE Control Application
*/

#include "dsl_cpe_control.h"

#define DSL_CPE_BND_TX_RATE_RATIO_UNITY   (0x0100)

/*
   LineMonitorStateMachine
*/
typedef struct {
   DSL_uint16_t Port;
   DSL_uint16_t PafAvailable;
   DSL_uint16_t RemotePafAvailable;
   DSL_uint16_t PafAggregate;
   DSL_uint16_t PafEnable;
   DSL_uint32_t TxDataRate;
} DSL_CPE_BND_LineMonitorStateMachine_t;

/**
   Bonding Context
*/
typedef struct
{
   /**
   Line Monitor SM data*/
   DSL_CPE_BND_LineMonitorStateMachine_t lineMonitorStateMachine[2];
   /**
   Remote discovery code*/
   DSL_uint8_t remoteDiscoveryCode[6];
   /**
   Aggregate data state*/
   DSL_uint32_t aggregateReg;
} DSL_CPE_BND_Context_t;

/*
   Function to start Bonding handling
*/
DSL_Error_t DSL_CPE_BND_Start(
   DSL_CPE_Control_Context_t *pCtrlCtx,
   DSL_int_t fd);

/*
   Function to stop Bonding handling
*/
DSL_void_t DSL_CPE_BND_Stop(
   DSL_CPE_BND_Context_t *pBndContext);

/*
   Bonding handling for the Autoboot Restart Wait state
*/
DSL_Error_t DSL_CPE_BND_AutobootStatusRestartWait(
   DSL_int_t fd, DSL_uint_t nDevice);

/*
   Bonding handling for the Line State change
*/
DSL_Error_t DSL_CPE_BND_LineStateHandle(
   DSL_CPE_BND_Context_t *pBndCtx,
   DSL_int_t fd, DSL_uint_t nDevice,
   DSL_LineStateValue_t nLineState,
   DSL_LineStateValue_t nPrevLineState);

#ifdef __cplusplus
}
#endif

#endif /* _DSL_CPE_BND_H_ */

