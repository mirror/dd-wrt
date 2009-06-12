/****************************************************************************
*
*	Name:			cnxttimer.h
*
*	Description:	
*
*	Copyright:		(c) 2002 Conexant Systems Inc.
*
*****************************************************************************

  This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your option)
any later version.

  This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
more details.

  You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc., 59
Temple Place, Suite 330, Boston, MA 02111-1307 USA

****************************************************************************
*  $Author: gerg $
*  $Revision: 1.1 $
*  $Modtime: 10/14/02 12:58p $
****************************************************************************/

#ifndef CNXTTIMER_H
#define CNXTTIMER_H

#ifdef __cplusplus
extern "C" {
#endif


/* register definitions */
#ifdef DEVICE_YELLOWSTONE
	#define TIMER_BASE                  0x00601400
	#define HW_TIMER_INT_ENABLE			( volatile UINT32* )( TIMER_BASE + 0x000 )
	#define HW_TIMER_INT_ASSIGN_POLLED	( volatile UINT32* )( TIMER_BASE + 0x004 )
	#define HW_TIMER_INT_STAT_POLLED	   ( volatile UINT32* )( TIMER_BASE + 0x008 )
	#define HW_TIMER_INT_STAT			   ( volatile UINT32* )( TIMER_BASE + 0x00c )
	#define HW_TIMER_INT_MASK			   ( volatile UINT32* )( TIMER_BASE + 0x010 )
	#define HW_TIMER_INT_STAT_MASKED	   ( volatile UINT32* )( TIMER_BASE + 0x014 )
	#define HW_TIMER_INT_ASSIGN_A		   ( volatile UINT32* )( TIMER_BASE + 0x018 )
	#define HW_TIMER_INT_ASSIGN_B		   ( volatile UINT32* )( TIMER_BASE + 0x01c )
	#define HW_TIMER_INT_STAT_MASKED_A	( volatile UINT32* )( TIMER_BASE + 0x020 )
	#define HW_TIMER_INT_STAT_MASKED_B	( volatile UINT32* )( TIMER_BASE + 0x024 )

	#define HW_TIMER_CSR                ( volatile UINT32* )( TIMER_BASE + 0x100 )

	#define GPT_ENA_BIT_OFFSET          8
	#define GPT_MODE_BIT_OFFSET         16

	#define HW_TIMER_PRESCALE_BASE      (TIMER_BASE + 0x104)
	#define HW_TIMER_LIMIT_BASE         (TIMER_BASE + 0x108)
	#define HW_TIMER_COUNT_BASE         (TIMER_BASE + 0x10C)
	#define HW_TIMER_REG_OFFSET		   0xC

	#ifdef PO6_FPGA
		#define GPT_PRESCALE_MAX_VALUE      0xFFFFFF
	#else
		#define GPT_PRESCALE_MAX_VALUE      0x3FF
	#endif

#else
	#define HW_TIMER_COUNT_BASE  0x00350020
	#define HW_TIMER_LIMIT_BASE  (HW_TIMER_COUNT_BASE + 0x10)
	#define HW_TIMER_REG_OFFSET		   4
#endif

#define TM_CNT1 ((volatile UINT32 *) (HW_TIMER_COUNT_BASE))
#define TM_CNT2 ((volatile UINT32 *) (HW_TIMER_COUNT_BASE + 0x4))
#define TM_CNT3 ((volatile UINT32 *) (HW_TIMER_COUNT_BASE + 0x8))
#define TM_CNT4 ((volatile UINT32 *) (HW_TIMER_COUNT_BASE + 0xC))

#define TM_LMT1 ((volatile UINT32 *) (HW_TIMER_LIMIT_BASE))
#define TM_LMT2 ((volatile UINT32 *) (HW_TIMER_LIMIT_BASE + 0x4))
#define TM_LMT3 ((volatile UINT32 *) (HW_TIMER_LIMIT_BASE + 0x8))
#define TM_LMT4 ((volatile UINT32 *) (HW_TIMER_LIMIT_BASE + 0xC))

#define SYS_TIMER_CNT               TM_CNT2
#define SYS_TIMER_LIMIT             TM_LMT2
#define AUX_TIMER_CNT               TM_CNT4
#define AUX_TIMER_LIMIT             TM_LMT4
#define TIMESTAMP_TIMER_CNT         TM_CNT1
#define TIMESTAMP_TIMER_LIMIT       TM_LMT1

#define SYS_TIMER_INT               INT_TM2
#define AUX_TIMER_INT               INT_TM4
#define TIMESTAMP_TIMER_INT         INT_TM1

#define SYS_TIMER_INT_LVL           (INT_LVL_TIMER_2)
#define AUX_TIMER_INT_LVL           (INT_LVL_TIMER_4)
#define TIMESTAMP_TIMER_INT_LVL     (INT_LVL_TIMER_1)

/*
 * Clock rates depend upon CPU power and work load of application.
 * The values below are minimum and maximum allowed by the hardware.
 * Note that it takes 3 ticks to reload the 16-bit counter and we don't
 * accept values that would mean a zero reload value as we don't know what
 * that will do.
 * So:
 * min frequency = roundup(clock_rate/(max_counter_value+3))
 * max frequency = rounddown(clock_rate/(min_counter_value+3))
 * i.e.                      SYS_CLK_RATE_MAX (SYS_TIMER_CLK/4)
 * However, we must set maxima that are sustainable on a running
 * system. Experiments suggest that a 16MHz PID board can sustain a
 * maximum clock rate of 10000 to 10500. The values below have been
 * chosen so that there is a reasonable margin and the BSP passes the
 * test suite.
 */

#define SYS_TIMER_TC_DISABLE  0x0              /* 0 Written to TIM_LIM2 shuts off timer    */
#define AUX_TIMER_TC_DISABLE  0x0              /* 0 Written to TIM_LIM4 shuts off timer    */
#define TIMESTAMP_TIMER_TC_DISABLE  0x0        /* 0 Written to TIM_LIM1 shuts off timer    */

#ifdef DEVICE_YELLOWSTONE
	#define HW_CLK_SPEED          10000000         /* Timer speed is 0.1 uSec */
#else
	#define HW_CLK_SPEED          1000000          /* Timer speed is 1.0 uSec */
#endif

#define SYS_CLK_SPEED         (HW_CLK_SPEED)   /* Frequency of system timer   */
#define AUX_CLK_SPEED         (HW_CLK_SPEED)   /* Frequency of auxiliary timer*/
#define TIMESTAMP_CLK_SPEED   (HW_CLK_SPEED)   /* Frequency of timestamp timer*/

#define CLK_RELOAD_TICKS       3               /* three ticks to reload timer*/

#ifdef DEVICE_YELLOWSTONE
	#define SYS_CLK_RATE_MIN 1        /* Min resolution of 1 = 1 sec          */
	#define SYS_CLK_RATE_MAX 1000000  /* Max resolution of 1/1000000 = 1 uSec */

	#define AUX_CLK_RATE_MIN 10       /* Min resolution of 1/10 = 100ms */
	#define AUX_CLK_RATE_MAX 1000     /* Max resolution of 1/1000 = 1ms */
#else
	#define SYS_CLK_RATE_MIN 10       /* Min resolution of 1/10 = 100ms */
	#define SYS_CLK_RATE_MAX 1000     /* Max resolution of 1/1000 = 1ms */

	#define AUX_CLK_RATE_MIN 10       /* Min resolution of 1/10 = 100ms */
	#define AUX_CLK_RATE_MAX 2000     /* Max resolution of 1/2000 = 0.5ms */
#endif

#define CLK_SEC_RATE  1           /* Sec resolution range           */
#define CLK_mSEC_RATE 1000        /* mSec resolution range          */
#define CLK_uSEC_RATE 1000000     /* uSec resolution range          */

/* ELAPSED_TIME structure and function prototype */
typedef struct tagElapsedTime {
   UINT32 days;
   UINT32 hours;
   UINT32 minutes;
   UINT32 seconds;
} ELAPSED_TIME, *PELAPSED_TIME;


/* Timer structure */
typedef struct tagHW_Timer {
   #ifdef DEVICE_YELLOWSTONE
   volatile UINT32 *pTimerIntEnable;
   volatile UINT32 *pTimerIntAssignPolled;
   volatile UINT32 *pTimerIntStatPolled;
   volatile UINT32 *pTimerIntStat;
   volatile UINT32 *pTimerIntMask;
   volatile UINT32 *pTimerIntStatMask;
   volatile UINT32 *pTimerIntAssignA;
   volatile UINT32 *pTimerIntAssignB;
   volatile UINT32 *pTimerIntStatMaskA;
   volatile UINT32 *pTimerIntStatMaskB;
   volatile UINT32 *pTimerCSR;
   volatile UINT32 *pTimerPrescale;			/* Timer HW registers */
   #endif

   volatile UINT32 *pTimerCount;
   volatile UINT32 *pTimerLimit;

   UINT32          TimerNum;
   UINT32 		    TimerOpMode;
   UINT32          TimerResolution;
   UINT32          TimerIntSource;
   int             TimerIntLevel;

   void			   (*pTimerRoutine)(int);
   int             TimerArg;

   UINT32          TimerRunning;
   UINT32          TimerConnected;
   UINT32          TimerTicksPerSecond;
   UINT32          TimerClkSpeed;
   UINT32		   TimerHwTicks;
   BOOL			   TimerShiftRate;
   BOOL			   TimerOsIntConnected;
} HW_TIMER, *PHW_TIMER;

typedef enum
{
   TIMER1,
   TIMER2,
   TIMER3,
   TIMER4,

   #ifdef DEVICE_YELLOWSTONE
   TIMER5,
   TIMER6,
   #endif

   MAX_NUM_HW_TIMERS
} eTimerType;

typedef enum
{
   TIMER_ONESHOT,
   TIMER_PERIODIC,
} eTimerMode;

#define AUX_TIMER          TIMER4

void   TimerEnable (PHW_TIMER Timer, UINT32 bEnableInt);
void   TimerDisable (PHW_TIMER Timer, UINT32 bIntEnable);
void   TimerConfigure(eTimerType TimerNum, HW_TIMER TimerConfig);
void   CnxtTimerInit(void);
void   TimerIntCtrl(PHW_TIMER Timer, UINT32 bEnableInt);
STATUS TimerSetCallBack ( FUNCPTR routine,	 /* Routine to be called at each clock interrupt */
                          int arg,            /* Argument with which to call routine */	
                          eTimerType TimerNum /* Which timer */
                        );
void TimerSetResolution(PHW_TIMER Timer);
void TimerSetPeriod(PHW_TIMER Timer);
void TimerClearIntStatus(PHW_TIMER Timer);
void Timer3IntHandler (int irq, void *dev_id, struct pt_regs * regs);

extern void TaskDelayMsec(UINT32 TimeDuration);
extern BOOL sysTimerDelay( UINT32 uMicroSec );
extern BOOLEAN gp_timer_init( WORD timerInt );

extern BOOLEAN gp_timer_connect
(
	EVENT_HNDL	*pEvent,
	WORD		timerInt
);

extern BOOLEAN gp_timer_start
(
	WORD 	usec,
	BOOLEAN periodic,
	WORD	timerInt
);

int sysClkRateGet( void );

void sysAuxClkDisable (void);
void sysAuxClkEnable (void);
int	sysAuxClkRateSet( int ticksPerSecond );
int sysAuxClkConnect( FUNCPTR routine, int arg );
int sysClkRateGet (void);
void sysAuxClkInt (int irq, void *dev_id, struct pt_regs * regs);
ULONG tickGet(void);
STATUS TimerSetCallBack ( FUNCPTR routine, int arg, eTimerType TimerNum );

// Virtual Callback functions
int sysAuxClkConnectExt(FUNCPTR routine,int arg);
STATUS sysAuxClkDisconnectExt (int cbh);
void sysAuxClkDisableExt (int cbh);
void sysAuxClkEnableExt (int cbh);
int sysAuxClkRateGetExt (int cbh);
STATUS sysAuxClkRateSetExt(int cbh,int ticksPerSecond);
STATUS sysAuxClkSetOneShotExt(FUNCPTR routine,int arg,UINT32 us);

/* register definitions */

#define GP_TIMER_1	INT_LVL_TIMER_2
#define GP_TIMER_2	INT_LVL_TIMER_4

#define CLK_RELOAD_TICKS       3               /* three ticks to reload timer*/

#ifdef __cplusplus
}
#endif

#endif	/* CNXTTIMER_H */

