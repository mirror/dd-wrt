
/******************************************************************************
*
* (c) Copyright 1996-2000, Palmchip Corporation
*
* This document is an unpublished work protected under the copyright laws
* of the United States containing the confidential, proprietary and trade
* secret information of Palmchip Corporation. This document may not be
* copied or reproduced in any form whatsoever without the express written
* permission of Palmchip Corporation.
*
*******************************************************************************
*
*  File Name: tmr.h
*     Author: Robin Bhagat 
*
*******************************************************************************
*
* Revision History:
*
*      Date    Name  Comments
*    --------  ---   ------------------------------------
*    08/12/97  RWB   Created.
*    10/23/00  IST   Merged header files for PalmPak 2.0
*
*
*
*******************************************************************************/
/* FILE_DESC ******************************************************************
//
// Purpose:
//    This file contains all the timer block definitions. 
//
// Sp. Notes:
//
 ******************************************************************************/

#ifndef TIMER_H
#define TIMER_H


/*=====================*
 *  Include Files      *
 *=====================*/
#include "pubdefs.h"
#include "product.h"


/*=====================*
 *  Defines            *
 *=====================*/

/* Common to all timers - used only in sysboot.s! */
#define TMR_STATUS_REG          (TMR_BASE)

/* Timer Status register bit definitions */
#define TMR_INT		       	(0x0001)
#define WD_INT		       	(0x0002)
#define TMR_INTS                (0x0003)
#define TMR_RESET	       	(0x0010)
#define WD_RESET	       	(0x0020)

/* Timer Control register bit definitions */

#define TMR_SCALE_MASK		(0xf)
#define TMR_NUM_SCALES          (0x10)

#define TMR_SCALE_DN_0		(0x0000)
#define TMR_SCALE_DN_4		(0x0001)
#define TMR_SCALE_DN_8		(0x0002)
#define TMR_SCALE_DN_16		(0x0003)
#define TMR_SCALE_DN_32		(0x0004)
#define TMR_SCALE_DN_64		(0x0005)
#define TMR_SCALE_DN_128	(0x0006)
#define TMR_SCALE_DN_256	(0x0007)

#define TMR_MODE_SHIFT		(4)
#define TMR_MODE_MASK		(0x3 << TMR_MODE_SHIFT)

#define TMR_FREE_RUN_MODE	(0x00)
#define TMR_PERIODIC_MODE	(0x01 << TMR_MODE_SHIFT)
#define TMR_TIME_OUT_MODE	(0x02 << TMR_MODE_SHIFT)        /* Not supported in AMBA */
#define TMR_WATCHDOG_MODE	(0x03 << TMR_MODE_SHIFT)	/* Not supported in AMBA */

#define TMR_ENABLE	       	(0x0080)
#define TMR_DISABLE             (0x0000)
#define TMR_MAX_CNT             (0xffff)

/* defines for timerBlockRegs timers */
#define TMR_BLK_TMR0		(0)
#define TMR_BLK_WDOG		(1)

/*=====================*
 *  Type defines       *
 *=====================*/

/* Structure of the individual subtimers. */
typedef struct timerRegs_t 
{
      volatile uint32 loadVal;
      volatile uint32 counter;
      volatile uint32 control;
      uint32          Reserved0;
} timerRegs;

/* Structure of the overall timer block. */
typedef struct timerBlockRegs_t 
{
      volatile uint32 status;
      uint32          Reserved1[3];
      timerRegs       timer[2];
} timerBlockRegs;

typedef enum timerMode_t 
{
	FreeRunning,
	Periodic,
	TimeOut,
	WatchDog
} timerMode;

typedef enum timerClkFreq_t 
{
	SysClk,
	SysClkDiv4,
	SysClkDiv8,
	SysClkDiv16,
	SysClkDiv32,
	SysClkDiv64,
	SysClkDiv128,
	SysClkDiv256,
	SysClkDiv512,
	SysClkDiv1k,
	SysClkDiv2k,
	SysClkDiv4k,
	SysClkDiv8k,
	SysClkDiv16k,
	SysClkDiv32k,
	SysClkDiv64k
} timerClkFreq;


/*=====================*
 *  External Variables *
 *=====================*/

/*=====================*
 *  External Functions *
 *=====================*/
PUBLIC void SetTimerDivider ( timerBlockRegs *tPtr, int timerNum, 
			      timerClkFreq freq );
PUBLIC void SetTimerMode    ( timerBlockRegs *tPtr, int timerNum, 
			      timerMode mode );
PUBLIC void Delay_us ( timerBlockRegs *tPtr, int timerNum, uint32 microsecs );

#ifdef SIM_DEBUG
PUBLIC uint32 GetMult( timerClkFreq freq );
#endif /* SIM_DEBUG */


/*=====================*
 *  Macro Functions    *
 *=====================*/

/* FUNCTION_DESC **************************************************************
//
// NAME           EnableTimer()
//
// SYNOPSIS       void EnableTimer ( timerBlockRegs *tPtr, int timerNum )
//
// TYPE           Inline function
//
// INPUT          timerBlockRegs *tPtr: base pointer of the timer block
//                int timerNum: number for timer offset within block
//
// OUTPUT         None
//
// DESCRIPTION    This function enables the timer. The timer will start counting
//                down from the current value in the counter.
//
// NOTE           For PalmPak 2.0, timerNum is 0 for timer0, 1 for watchdog.
//
 ******************************************************************************/
#define EnableTimer(tPtr,timerNum)	do { tPtr->timer[timerNum].control |= (uint32)TMR_ENABLE; } while (0)


/* FUNCTION_DESC **************************************************************
//
// NAME           DisableTimer()
//
// SYNOPSIS       void DisableTimer ( timerBlockRegs *tPtr, int timerNum )
//
// TYPE           Inline function
//
// INPUT          timerBlockRegs *tPtr: base pointer of the timer block
//                int timerNum: number for timer offset within block
//
// OUTPUT         None
//
// DESCRIPTION    This function will disable the Timer.
//
// NOTE           This function preserves the current counter value. If   
//                the timer is enabled again without being reset, the timer   
//                will start counting down from the previously stopped value. 
//                For PalmPak 2.0, timerNum is 0 for timer0, 1 for watchdog.
//
 ******************************************************************************/
#define DisableTimer(tPtr,timerNum)	do { tPtr->timer[timerNum].control &= ~((uint32)TMR_ENABLE); } while (0)


/* FUNCTION_DESC **************************************************************
//
// NAME           LoadTimer()
//
// SYNOPSIS       void LoadTimer ( timerBlockRegs *tPtr, int timerNum, 
//                                 uint32 val )
//
// TYPE           Inline function
//
// INPUT          timerBlockRegs *tPtr: base pointer of the timer block
//                int timerNum: number for timer offset within block
//                uint32 val:  counter value.  
//
// OUTPUT         None
//
// DESCRIPTION    This function sets the given Timer load value. It also
//                initializes the timer counter value.
//                If the timer is enabled, it will start counting down from this 
//                value at the configured clock rate, and generate an 
//                interrupt when the counter reaches to zero.
//
// NOTE           For PalmPak 2.0, timerNum is 0 for timer0, 1 for watchdog.
//
 ******************************************************************************/
#define LoadTimer(tPtr,timerNum,val)	do { tPtr->timer[timerNum].loadVal = val; } while (0)


/* FUNCTION_DESC **************************************************************
//
// NAME           ResetTimer()
//
// SYNOPSIS       void ResetTimer ( timerBlockRegs *tPtr, int timerNum )
//
// TYPE           Inline function
//
// INPUT          timerBlockRegs *tPtr: pointer to the timer block
//                int timerNum: number for timer offset within block
//
// OUTPUT         None
//
// DESCRIPTION    This function resets the given Timer counter value.
//                The timer counter will be reloaded with the timer load value.
//
// NOTE           This function no longer takes a bitmask value, but instead
//                an offset constant (i.e. 0 for timer0, 1 for watchdog).
//
//                Implementation will change with new file conventions.
//
 ******************************************************************************/
#define ResetTimer(tPtr,timerNum)   do { if (timerNum==TMR_BLK_TMR0) tPtr->status|=TMR_RESET; if (timerNum==TMR_BLK_WDOG) tPtr->status|=WD_RESET; } while (0)


/* FUNCTION_DESC **************************************************************
//
// NAME           IsTimerEnabled()
//
// SYNOPSIS       bool IsTimerEnabled ( timerBlockRegs *tPtr, int timerNum )
//
// TYPE           Inline function
//
// INPUT          timerBlockRegs *tPtr: pointer to the timer block
//                int timerNum: number for timer offset within block
//
// OUTPUT         TRUE/FALSE
//
// DESCRIPTION    This function returns TRUE if the Timer specified by timerNum
//                is enabled.
//
// NOTE           None
//
//                
//
 ******************************************************************************/
#define IsTimerEnabled(tPtr,timerNum)   ( (bool) tPtr->timer[timerNum].control & TMR_ENABLE )


/* FUNCTION_DESC **************************************************************
//
// NAME           IsTimerInt()
//
// SYNOPSIS       bool IsTimerInt ( timerBlockRegs *tPtr, int timerNum )
//
// TYPE           Inline function
//
// INPUT          timerBlockRegs *tPtr: pointer to the timer block
//                int timerNum: number for timer offset within block
//                
// OUTPUT         TRUE/FALSE
//
// DESCRIPTION    This function returns TRUE if the Timer interrupt has
//                occurred, otherwise it returns FALSE.
//
// NOTE           This function does not clear the interrupt.  
//                No longer takes a bitmask value, but instead
//                an offset constant (i.e. 0 for timer0, 1 for watchdog).
//
//                Implementation will change with new file conventions.
//
 ******************************************************************************/
#define IsTimerInt(tPtr,timerNum)   ( (bool) ((tPtr->status & ((timerNum==TMR_BLK_TMR0)?TMR_INT:WD_INT) ) ? TRUE : FALSE) )


/* FUNCTION_DESC **************************************************************
//
// NAME           ClearTimerInt()
//
// SYNOPSIS       void ClearTimerInt ( timerBlockRegs *tPtr, int timerNum )
//
// TYPE           Inline function
//
// INPUT          timerBlockRegs *tPtr: pointer to the timer block
//                int timerNum: number for timer offset within block
//                
// OUTPUT         None
//
// DESCRIPTION    This function clears the Timer interrupt.
//
// NOTE           This function no longer takes a bitmask value, but instead
//                an offset constant (i.e. 0 for timer0, 1 for watchdog).
//
//                Implementation will change with new file conventions.
//
 ******************************************************************************/
#define ClearTimerInt(tPtr,timerNum)   do { if (timerNum==TMR_BLK_TMR0) tPtr->status|=TMR_INT; if (timerNum==TMR_BLK_WDOG) tPtr->status|=WD_INT; } while (0)


/* FUNCTION_DESC **************************************************************
//
// NAME           ReadTimerCount()
//
// SYNOPSIS       void ReadTimerCount ( tmrBlockRegs *tPtr, int timerNum )
//
// TYPE           Inline function
//
// INPUT          timerBlockRegs *tPtr: base pointer of the timer block
//                int timerNum: number for timer offset within block
//                
// OUTPUT         uint32 count: timer count value
//
// DESCRIPTION    This function returns the current value of the counter.
//
// NOTE           For PalmPak 2.0, timerNum is 0 for timer0, 1 for watchdog.
//
 ******************************************************************************/
#define ReadTimerCount(tPtr,timerNum)	( tPtr->timer[timerNum].counter )



#endif /* TIMER_H */


