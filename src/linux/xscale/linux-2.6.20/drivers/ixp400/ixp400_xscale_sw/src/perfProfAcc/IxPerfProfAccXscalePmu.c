/**
 * @file IxPerfProfAccXscalePmu.c
 *
 * @date April-09-2003
 *
 * @brief  Source file for the Xscale PMU public APIs and internal functions
 *
 *
 * Design Notes:
 *
 * 
 * @par
 * IXP400 SW Release Crypto version 2.3
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2005, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
 */

/*
 * Put the system defined include files required.
 */

#ifdef __vxworks
#include <vxWorks.h>
#include <symLib.h>
#include <sysSymTbl.h>
#include <stdio.h>
#elif defined (__linux)
#include <linux/module.h>
#endif

/*
 * Put the user defined include files required.
 */
#include "IxOsal.h"
#include "IxPerfProfAcc.h"
#include "IxFeatureCtrl.h"
#include "IxPerfProfAccXscalePmu_p.h"
#include "IxPerfProfAcc_p.h"

/*
 * #defines and macros used in this file.
 */

/* Maximum value for 32-bit counters*/
#define IX_PERFPROF_ACC_XSCALE_PMU_MAX_COUNTER_VALUE 0xffffffff

/* Num bits to shift in order to multiply by 8*/
#define IX_PERFPROF_ACC_XSCALE_PMU_SHIFT_TO_MULT_8 3

/* Define string size to be used to display symbol name */
#define IX_PERFPROF_ACC_XSCALE_PMU_SYMBOL_SIZE 1024

/*
 * Variable declarations global to this file only.  Externs are followed by
 * static variables
 */
#ifdef __vxworks
IMPORT UINT32 (*vxIrqIntStackBase)[]; /*base of interrupt stack*/
#endif /*ifdef vxworks*/

#ifdef __linux
extern UINT32 ixOsalLinuxInterruptedPc;
#endif

/* base value to be written to counters (overflow rate in cases of sampling)
 * this is an array of 5 elements, representing all the counters
 */
static UINT32 ctrBase[IX_PERFPROF_ACC_XSCALE_PMU_MAX_COUNTERS];
static UINT32 numberEvents = 0;    /*number of events to be monitored*/
static BOOL eventCounting = FALSE;  /*TRUE if event counting is on */

/* number of samples (overflows) for each counter; this is an array of 5
 * elements, representing all the counters
 */
static UINT32 ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_MAX_COUNTERS];

/* TRUE if buffer results full; FALSE if counter overflow */
static BOOL IxPerfProfAccXscalePmuIntrStatus [
    IX_PERFPROF_ACC_XSCALE_PMU_MAX_COUNTERS];

/*arrays to store pc addresses for each interrupt*/
static UINT32 clkCtrArray[IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES];
static UINT32 evtCtr1Array[IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES];
static UINT32 evtCtr2Array[IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES];
static UINT32 evtCtr3Array[IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES];
static UINT32 evtCtr4Array[IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES];
static UINT32  sumLen; /*number of occurrences of profile*/

static BOOL eventCountStarted = FALSE;  /*is TRUE if event counting has been
                                         *started; FALSE otherwise
                                         */
static BOOL timeSampStarted = FALSE;    /*is TRUE if time sampling has been
                                         *started; FALSE otherwise
                                         */
static BOOL eventSampStarted = FALSE;   /*is TRUE if event sampling has been
                                         *started; FALSE otherwise
                                         */

#ifdef __linux
/* Xscale PMU time sampling profile results used for linux file writing*/
static IxPerfProfAccXscalePmuSamplePcProfile  
             timeSampProfile[IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES];
static IxPerfProfAccXscalePmuSamplePcProfile 
             eventSampProfile1[IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES];
static IxPerfProfAccXscalePmuSamplePcProfile 
             eventSampProfile2[IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES];
static IxPerfProfAccXscalePmuSamplePcProfile 
             eventSampProfile3[IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES];
static IxPerfProfAccXscalePmuSamplePcProfile 
             eventSampProfile4[IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES];
#endif

/*
 * Function definition.
 */

#ifndef _DIAB_TOOL
INLINE UINT32
_ixPerfProfAccXscalePmuOverFlowRead(void)
{
    register UINT32 _value_;

    /*move to overflow flag status register from coprocessor 14 and store in
     *value
     */
    __asm volatile("mrc\tp14, 0, %0, c5, c1, 0" : "=r" (_value_));
    return _value_ & 0x1f; /*only set first 5 bits in the status register*/
}

INLINE void
_ixPerfProfAccXscalePmuOverFlowWrite(UINT32 value)
{
    /*move value to coprocessor 14 from overflow flag status register*/
    __asm("mcr\tp14, 0, %0, c5, c1, 0" : : "r" (value));
}

INLINE UINT32
_ixPerfProfAccXscalePmuCcntRead(void)
{
    register UINT32 _value_;

    /*move to ccnt (clk counter) register from coprocessor 14 and store in value
     */
    __asm volatile("mrc\tp14, 0, %0, c1, c1, 0" : "=r" (_value_));
    return _value_;
}

INLINE void
_ixPerfProfAccXscalePmuCcntWrite(UINT32 value)
{
    /*move value to coprocessor 14 from ccnt (clk counter) register*/
    __asm("mcr\tp14, 0, %0, c1, c1, 0" : : "r" (value));
}

INLINE unsigned
_ixPerfProfAccXscalePmuIntenRead(void)
{
    register UINT32 _value_;

    /*move to interrupt enable register from coprocessor 14 and store in value*/
    __asm volatile("mrc\tp14, 0, %0, c4, c1, 0" : "=r" (_value_));
    return _value_ & 0x1f; /*mask return value to ensure only first 5 bits in
                            *the register are set
                            */
}

INLINE void
_ixPerfProfAccXscalePmuIntenWrite(UINT32 value)
{
    /*move value to coprocessor 14 from interrupt enable register*/
    __asm("mcr\tp14, 0, %0, c4, c1, 0" : : "r" (value));
}

INLINE UINT32
_ixPerfProfAccXscalePmuPmncRead(void)
{
    register UINT32 _value_;

    /*move to performance monitor control register from coprocessor 14 and store
     *in value
     */
    __asm volatile("mrc\tp14, 0, %0, c0, c1, 0" : "=r" (_value_));
    return _value_ & 0xf;    /*mask return value to ensure only first 4 bits in
                              *the register are set
                              */
}

INLINE void
_ixPerfProfAccXscalePmuPmncWrite(UINT32 value)
{
    /*move value to coprocessor 14 from performance monitor control register*/
    __asm("mcr\tp14, 0, %0, c0, c1, 0" : : "r" (value));
}

INLINE void
_ixPerfProfAccXscalePmuEvtSelectWrite(UINT32 value)
{
    /*move to event select register from coprocessor 14 and store
     *in value
     */
    __asm("mcr\tp14, 0, %0, c8, c1, 0" : : "r" (value));
}

INLINE UINT32
_ixPerfProfAccXscalePmuEvtSelectRead(void)
{
    /*move to event select register from coprocessor 14 and store
     *in value
     */
    register UINT32 value = 0;
    __asm("mrc\tp14, 0, %0, c8, c1, 0" : : "r" (value));
    return (value);
}

INLINE UINT32
_ixPerfProfAccXscalePmuPmnRead(UINT32 num, BOOL *check)
{
    register UINT32 _value_ = 0;
    if (IX_PERFPROF_ACC_XSCALE_PMU_MAX_EVENTS < num) /*num passed in exceeds
                                                      *maximum counters allowed
                                                      */
    {
        *check = FALSE;
    }
    switch (num)/*move value to coprocessor 14 from event counter registers*/
    {
        case 0: /*read value of event counter 1 only*/
            __asm volatile("mrc\tp14, 0, %0, c0, c2, 0" : "=r" (_value_));
            break;

        case 1: /*read value of event counter 2 only*/
            __asm volatile("mrc\tp14, 0, %0, c1, c2, 0" : "=r" (_value_));
            break;

        case 2: /*read value of event counter 3 only*/
            __asm volatile("mrc\tp14, 0, %0, c2, c2, 0" : "=r" (_value_));
            break;

        case 3: /*read value of event counter 4 only*/
            __asm volatile("mrc\tp14, 0, %0, c3, c2, 0" : "=r" (_value_));
            break;

        default:
            break;
    }
    return _value_;
}

INLINE void
_ixPerfProfAccXscalePmuPmnWrite(UINT32 num, UINT32 value, BOOL *check)
{
    /*set value of *check to FALSE if num passed in is greater than maximum
     *counters allowed
     */
    if(IX_PERFPROF_ACC_XSCALE_PMU_MAX_EVENTS<num)
    {
        *check = FALSE;
    } /*end of if IX_PERFPROF_ACC_XSCALE_PMU_MAX_EVENTS*/

    switch (num)/*move to event counter registers from coprocessor 14 and store
                 *in value
                 */
    {
        case 0:   /*write value to event counter 1 only*/
            __asm("mcr\tp14, 0, %0, c0, c2, 0" : : "r" (value));
            break;

        case 1:   /*write value to event counter 2 only*/
            __asm("mcr\tp14, 0, %0, c1, c2, 0" : : "r" (value));
            break;

        case 2:   /*write value to event counter 3 only*/
            __asm("mcr\tp14, 0, %0, c2, c2, 0" : : "r" (value));
            break;

        case 3:   /*write value to event counter 4 only*/
            __asm("mcr\tp14, 0, %0, c3, c2, 0" : : "r" (value));
            break;

        default:
            break;
    } /*end of switch (num )*/
} /*end of _ixPerfProfAccXscalePmuPmnWrite()*/

#else /* _DIAB_TOOL defined */
__asm volatile UINT32
_ixPerfProfAccXscalePmuOverFlowRead(void)
{
! "r0"
    /*move to overflow flag status register from coprocessor 14 and store in
     *value
     */
    mrc\tp14, 0, r0, c5, c1, 0;
    and	r0, r0, #31; /*only set first 5 bits in the status register*/
    /* return value is returned through register R0 */
}

__asm volatile void
_ixPerfProfAccXscalePmuOverFlowWrite(UINT32 value)
{
%reg value
    /*move value to coprocessor 14 from overflow flag status register*/
    mcr\tp14, 0, value, c5, c1, 0;
}

__asm volatile UINT32
_ixPerfProfAccXscalePmuCcntRead(void)
{
! "r0"
    /*move to ccnt (clk counter) register from coprocessor 14 and store in value
     */
    mrc\tp14, 0, r0, c1, c1, 0;
    /* return value is returned through register R0 */
}

__asm volatile void
_ixPerfProfAccXscalePmuCcntWrite(UINT32 value)
{
% reg value
    /*move value to coprocessor 14 from ccnt (clk counter) register*/
    mcr\tp14, 0, value, c1, c1, 0;
}

__asm volatile unsigned
_ixPerfProfAccXscalePmuIntenRead(void)
{
! "r0"

    /*move to interrupt enable register from coprocessor 14 and store in value*/
    mrc\tp14, 0, r0, c4, c1, 0;
    and	r0, r0, #31; /*mask return value to ensure only first 5 bits in
                            *the register are set
                            */
    /* return value is returned through register R0 */
}

__asm volatile void
_ixPerfProfAccXscalePmuIntenWrite(UINT32 value)
{
% reg value
    /*move value to coprocessor 14 from interrupt enable register*/
    mcr\tp14, 0, value, c4, c1, 0;
}

__asm volatile UINT32
_ixPerfProfAccXscalePmuPmncRead(void)
{
! "r0"
    /*move to performance monitor control register from coprocessor 14 and store
     *in value
     */
    mrc\tp14, 0, r0, c0, c1, 0;
    and	r0, r0, #15;    /*mask return value to ensure only first 4 bits in
                              *the register are set
                              */
    /* return value is returned through register R0 */
}

__asm volatile void
_ixPerfProfAccXscalePmuPmncWrite(UINT32 value)
{
% reg value
    /*move value to coprocessor 14 from performance monitor control register*/
    mcr\tp14, 0, value, c0, c1, 0;
}

__asm volatile void
_ixPerfProfAccXscalePmuEvtSelectWrite(UINT32 value)
{
% reg value
    /*move to event select register from coprocessor 14 and store
     *in value
     */
    mcr\tp14, 0, value, c8, c1, 0;
}

__asm volatile UINT32
_ixPerfProfAccXscalePmuEvtSelectRead(void)
{
! "r0"
    /*move to event select register from coprocessor 14 and store
     *in value
     */
    mov	r0, #0;
    mrc\tp14, 0, r0, c8, c1, 0;
    /* return value is returned through register R0 */
}

__asm volatile UINT32
_ixPerfProfAccXscalePmuPmnCnt1Read_(void)
{
! "r0"
    mrc\tp14, 0, r0, c0, c2, 0;
    /* return value is returned through register R0 */
}

__asm volatile UINT32
_ixPerfProfAccXscalePmuPmnCnt2Read_(void)
{
! "r0"
    mrc\tp14, 0, r0, c1, c2, 0;
    /* return value is returned through register R0 */
}

__asm volatile UINT32
_ixPerfProfAccXscalePmuPmnCnt3Read_(void)
{
! "r0"
    mrc\tp14, 0, r0, c2, c2, 0;
    /* return value is returned through register R0 */
}

__asm volatile UINT32
_ixPerfProfAccXscalePmuPmnCnt4Read_(void)
{
! "r0"
    mrc\tp14, 0, r0, c3, c2, 0;
    /* return value is returned through register R0 */
}

INLINE UINT32
_ixPerfProfAccXscalePmuPmnRead(UINT32 num, BOOL *check)
{
    register UINT32 _value_ = 0;
    if (IX_PERFPROF_ACC_XSCALE_PMU_MAX_EVENTS < num) /*num passed in exceeds
                                                      *maximum counters allowed
                                                      */
    {
        *check = FALSE;
    }
    switch (num)/*move value to coprocessor 14 from event counter registers*/
    {
        case 0: /*read value of event counter 1 only*/
            return _ixPerfProfAccXscalePmuPmnCnt1Read_();
            break;

        case 1: /*read value of event counter 2 only*/
            return _ixPerfProfAccXscalePmuPmnCnt2Read_();
            break;

        case 2: /*read value of event counter 3 only*/
            return _ixPerfProfAccXscalePmuPmnCnt3Read_();
            break;

        case 3: /*read value of event counter 4 only*/
            return _ixPerfProfAccXscalePmuPmnCnt4Read_();
            break;

        default:
            break;
    }
    return _value_;
}

__asm volatile void
_ixPerfProfAccXscalePmuPmnCnt1Write(UINT32 value)
{
%reg value
    mcr\tp14, 0, value, c0, c2, 0;
}

__asm volatile void
_ixPerfProfAccXscalePmuPmnCnt2Write(UINT32 value)
{
%reg value
    mcr\tp14, 0, value, c1, c2, 0;
}

__asm volatile void
_ixPerfProfAccXscalePmuPmnCnt3Write(UINT32 value)
{
%reg value
    mcr\tp14, 0, value, c2, c2, 0;
}

__asm volatile void
_ixPerfProfAccXscalePmuPmnCnt4Write(UINT32 value)
{
%reg value
    mcr\tp14, 0, value, c3, c2, 0;
}

INLINE void
_ixPerfProfAccXscalePmuPmnWrite(UINT32 num, UINT32 value, BOOL *check)
{
    /*set value of *check to FALSE if num passed in is greater than maximum
     *counters allowed
     */
    if(IX_PERFPROF_ACC_XSCALE_PMU_MAX_EVENTS<num)
    {
        *check = FALSE;
    } /*end of if IX_PERFPROF_ACC_XSCALE_PMU_MAX_EVENTS*/

    switch (num)/*move to event counter registers from coprocessor 14 and store
                 *in value
                 */
    {

        case 0:   /*write value to event counter 1 only*/
            _ixPerfProfAccXscalePmuPmnCnt1Write(value);
            break;

        case 1:   /*write value to event counter 2 only*/
            _ixPerfProfAccXscalePmuPmnCnt2Write(value);
            break;

        case 2:   /*write value to event counter 3 only*/
            _ixPerfProfAccXscalePmuPmnCnt3Write(value);
            break;

        case 3:   /*write value to event counter 4 only*/
            _ixPerfProfAccXscalePmuPmnCnt4Write(value);
            break;

        default:
            break;
    } /*end of switch (num )*/
} /*end of _ixPerfProfAccXscalePmuPmnWrite()*/
#endif /* ifndef _DIAB_TOOL */

INLINE void
_ixPerfProfAccXscalePmuProfilePcStore (
    UINT32 eventCounterId,
    UINT32 idx,
    UINT32 pc)
{
    switch (eventCounterId)
    {
        case IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR1_ID :
            evtCtr1Array[idx] = pc ;
            break;

        case IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR2_ID :
            evtCtr2Array[idx] = pc ;
            break;

        case IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR3_ID :
            evtCtr3Array[idx] = pc ;
            break;

        case IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR4_ID :
            evtCtr4Array[idx] = pc ;
            break;

        default:
            break;
    } /*end of switch*/
}

VXWORKS_INLINE void
_ixPerfProfAccXscalePmuEventHandler (
    UINT32 eventOflowSelect,
    UINT32 eventCounterId,
    UINT32 pcAddr)
{
    BOOL checkNum = TRUE;       /*determines if num passed into
                                 *_ixPerfProfAccXscalePmuPmnWrite() is valid
                                 */

    if (eventCounting)  /*event counting only is on*/
    {
        IxPerfProfAccXscalePmuIntrStatus[eventCounterId] = TRUE; /*set to show
                                                                  *ctr overflowed
                                                                  */
        ctrSamples[eventCounterId]++;    /*keep track
                                          *of overflows
                                          */

        /*clear the clk cnt bit in the overflow flag reg*/
        _ixPerfProfAccXscalePmuOverFlowWrite(
            eventCounterId);
    }
    else if (eventSampStarted)/*is not event counting; only event-based sampling*/
    {
        if (IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES >
                ctrSamples[eventCounterId])             /*profile buffer not
                                                         *full yet
                                                         */
        {
            /* store PC and increment index */
            _ixPerfProfAccXscalePmuProfilePcStore (eventCounterId,
                                                   ctrSamples[eventCounterId],
                                                   pcAddr );

            ctrSamples[eventCounterId]++;

            /*clear the event counter bit in the overflow flag reg*/
            _ixPerfProfAccXscalePmuOverFlowWrite(eventCounterId);

            /*rewrite base value to evt counter 2 reg*/
            _ixPerfProfAccXscalePmuPmnWrite(eventCounterId,
                                            ctrBase[eventCounterId],
                                            &checkNum);
        }
        else    /*profile buffer is full*/
        {
            /*check if buffer full had been handled; if it is, we ignore this
             *interrupt. Otherwise, disable source of interrupt.
             */
            if (!IxPerfProfAccXscalePmuIntrStatus[eventCounterId])
            {
                /* Disable interrupt, by disabling the appropriate bit in the
                 * current value of the interrupt enable register
                 */
                _ixPerfProfAccXscalePmuIntenWrite(
                    _ixPerfProfAccXscalePmuIntenRead() &
                    ~(eventCounterId));

                /* Remember that we have reached MAX storage */
                IxPerfProfAccXscalePmuIntrStatus[eventCounterId] = TRUE;
                _ixPerfProfAccXscalePmuEvtSelectWrite(
                    (_ixPerfProfAccXscalePmuEvtSelectRead()) |
                    (IX_PERFPROF_ACC_XSCALE_PMU_EVT_SELECT_NONE <<
                     (eventCounterId <<
                     IX_PERFPROF_ACC_XSCALE_PMU_SHIFT_TO_MULT_8)));
             }/*end of if !IxPerfProfAccXscalePmuIntrStatus */
        }/*end of if-else*/
    }/*end of if-else eventCounting*/
} /* end of _ixPerfProfAccXscalePmuEventHandler() */


void
ixPerfProfAccXscalePmuIntrHandler (void)
{
    UINT32 overflow;    /*variable to determine if overflow bit is set */
    UINT32 pcAddr;     /*variable to store address of pc*/

#ifdef __vxworks
    /**
     * PMU event is presented as IRQ to XScale in VxWorks. VxWorks saves minimal
     * registers including R14 before switching to the interrupt routine.
     * Therefore, the interrupted PC is available at the base of the IRQ
     * interrupt stack.
     */
    pcAddr = (*vxIrqIntStackBase)[-1];

#elif defined(__linux)
    /**
     * For Linux, we need Osal to pass the interrupted PC
     * because Osal does not pass the frame pointers to our
     * ISR. ixOsalLinuxInterruptedPc is a global variables defined
     * in IxOsal.c
     */
    pcAddr = ixOsalLinuxInterruptedPc;
#endif /*end of if vxWorks*/

    overflow =_ixPerfProfAccXscalePmuOverFlowRead();    /*read the overflow flag
                                                         *status register to
                                                         *determine which counter
                                                         *has overflowed
                                                         */

    /* overflow occured in clock counter; for clock counting, only occurs when
     * counter is full
     */
    if (overflow & IX_PERFPROF_ACC_XSCALE_PMU_OFLOW_FLAG_CCNT)
    {
        if (eventCounting)  /*events counting only on*/
        {
            IxPerfProfAccXscalePmuIntrStatus[
                IX_PERFPROF_ACC_XSCALE_PMU_CLK_CTR_ID] = TRUE; /*set to show clk
                                                                *ctr overflowed
                                                                */
            ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_CLK_CTR_ID]++; /*keep track of
                                                                  *number of
                                                                  *overflows for
                                                                  *clk ctr
                                                                  */

            /*clear the clk cnt bit in the overflow flag reg*/
            _ixPerfProfAccXscalePmuOverFlowWrite(
                IX_PERFPROF_ACC_XSCALE_PMU_OFLOW_FLAG_CCNT);
        }
        else if (timeSampStarted)       /*is not event counting; only time-based
                                         *sampling
                                         */
        {
            if (IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES >
                ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_CLK_CTR_ID]) /*profile
                                                                    *buffer not
                                                                    *full yet
                                                                    */
            {

                /*store add of PC to results profile*/
                clkCtrArray[ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_CLK_CTR_ID]] =
                    pcAddr;
                ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_CLK_CTR_ID]++;

                /*clear the clk cnt bit in the overflow flag reg*/
                _ixPerfProfAccXscalePmuOverFlowWrite(
                    IX_PERFPROF_ACC_XSCALE_PMU_OFLOW_FLAG_CCNT);

                /*rewrite base value to clk cnt reg*/
                _ixPerfProfAccXscalePmuCcntWrite(
                    ctrBase[IX_PERFPROF_ACC_XSCALE_PMU_CLK_CTR_ID]);
            }
            else    /*profile buffer is full*/
            {
                /*check if this buffer is full; if is full, the interrupt is
                 *being triggered by another counter overflow
                 */
                if (!IxPerfProfAccXscalePmuIntrStatus[
                    IX_PERFPROF_ACC_XSCALE_PMU_CLK_CTR_ID])
                {
                    /* Disable clock counter interrupt, by disabling the appropriate
                     * bit in the current value of the interrupt enable register
                     */
                    _ixPerfProfAccXscalePmuIntenWrite(
                        _ixPerfProfAccXscalePmuIntenRead() &
                        ~(IX_PERFPROF_ACC_XSCALE_PMU_OFLOW_FLAG_CCNT));
                    /*set to show that buffer is full*/
                    IxPerfProfAccXscalePmuIntrStatus[
                        IX_PERFPROF_ACC_XSCALE_PMU_CLK_CTR_ID] = TRUE;
                 } /*end of if !IxPerfProfAccXscalePmuIntrStatus */
            }/*end of if-else IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES*/
        }/*end of if-else eventCounting*/
    }/*end of if overflow in clock counter*/

    /* overflow occured in evt counter 1; for evt counting, only occurs when
     * counter is full
     */
    if (overflow & IX_PERFPROF_ACC_XSCALE_PMU_OFLOW_FLAG_PMN0)
    {
        _ixPerfProfAccXscalePmuEventHandler (
            IX_PERFPROF_ACC_XSCALE_PMU_OFLOW_FLAG_PMN0,
            IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR1_ID,
            pcAddr);
    }/*end of else if overflow in evt counter 1*/

    /* overflow occured in evt counter 2; for evt counting, only occurs when
     * counter is full
     */
    if (overflow & IX_PERFPROF_ACC_XSCALE_PMU_OFLOW_FLAG_PMN1)
    {
        _ixPerfProfAccXscalePmuEventHandler (
            IX_PERFPROF_ACC_XSCALE_PMU_OFLOW_FLAG_PMN1,
            IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR2_ID,
            pcAddr);
    }

    /* overflow occured in evt counter 3; for evt counting, only occurs when
     * counter is full
     */
    if (overflow & IX_PERFPROF_ACC_XSCALE_PMU_OFLOW_FLAG_PMN2)
    {
        _ixPerfProfAccXscalePmuEventHandler (
            IX_PERFPROF_ACC_XSCALE_PMU_OFLOW_FLAG_PMN2,
            IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR3_ID,
            pcAddr);
    }/*end of else if overflow in evt counter 3*/

    /* overflow occured in evt counter 4; for evt counting, only occurs when
     * counter is full
     */
    if (overflow & IX_PERFPROF_ACC_XSCALE_PMU_OFLOW_FLAG_PMN3)
    {
        _ixPerfProfAccXscalePmuEventHandler (
            IX_PERFPROF_ACC_XSCALE_PMU_OFLOW_FLAG_PMN3,
            IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR4_ID,
            pcAddr);
    }/*end of else if overflow in evt counter 4*/

    /* Clear all bits in overflow flag register*/
    _ixPerfProfAccXscalePmuOverFlowWrite(
        IX_PERFPROF_ACC_XSCALE_PMU_OFLOW_FLAG_ALL);
}/*end of ixPerfProfAccXscalePmuIntrHandler()*/

void
ixPerfProfAccXscalePmuIntrConnect(void)
{
    /*Call intconnect() to connect interrupt handler*/
    ixOsalIrqBind(IX_OSAL_IXP400_XSCALE_PMU_IRQ_LVL,
		  (IxOsalVoidFnVoidPtr)ixPerfProfAccXscalePmuIntrHandler,
		  NULL);
}

void
ixPerfProfAccXscalePmuIntrEnable(BOOL clkCtr)
{
    UINT32 intrEnableRegBit = 0;    /*value to enter when enabling pmu interrupt
                                     *register
                                     */

    /*merely write to pmu interrupt enable register, nothing else; set the
     *appropriate bits in the PMU Interrupt Enable Register to enable
     *the interrupts for each clock or event counter
     */
    switch (numberEvents)
    {
        case 4: /*event counters 1,2,3,4 being monitored*/
            /*enable bits 1 to 4*/
            intrEnableRegBit |= IX_PERFPROF_ACC_XSCALE_PMU_OFLOW_FLAG_PMN3;

        case 3: /*event counters 1,2,3 being monitored*/
            /*enable bits 1,2 and 3*/
            intrEnableRegBit |= IX_PERFPROF_ACC_XSCALE_PMU_OFLOW_FLAG_PMN2;

        case 2: /*event counters 1,2 being monitored*/
            /*enable bits 1 and 2*/
            intrEnableRegBit |= IX_PERFPROF_ACC_XSCALE_PMU_OFLOW_FLAG_PMN1;

        case 1: /*only event counter 1 is being monitored; enable bit 1*/
            intrEnableRegBit |= IX_PERFPROF_ACC_XSCALE_PMU_OFLOW_FLAG_PMN0;
            break;

        default:
            break;
    }/*end of switch numberEvents*/

    if (clkCtr) /*for clock counting only or any event counting, need to enable
                 *clk ctr bit as well
                 */
    {
        intrEnableRegBit |= IX_PERFPROF_ACC_XSCALE_PMU_OFLOW_FLAG_CCNT;
    } /*end of if clkCtr*/

    /*write value of the interrupt enable register bit*/
    _ixPerfProfAccXscalePmuIntenWrite (intrEnableRegBit);
}/*end of ixPerfProfAccXscalePmuIntrEnable()*/

void
ixPerfProfAccXscalePmuBspIntrEnable(void)
{
    UINT32 *pIntrEnableRegAdd;     /*pointer to interrupt enable register*/

    /*enable xscale interrupt in the interrupt controller register*/
    pIntrEnableRegAdd =
        (UINT32 *)IX_PERFPROF_ACC_XSCALE_PMU_INTR_ENABLE_REG_ADD;

    /*set bit 18 to allow the XScale core to enable the interrupts*/
    *pIntrEnableRegAdd =
        ((*pIntrEnableRegAdd) |
         IX_PERFPROF_ACC_XSCALE_PMU_XSCALE_INTERRUPT_ENABLE_BIT);
}/*end of ixPerfProfAccXscalePmuBspIntrEnable()*/

void
ixPerfProfAccXscalePmuEventSelect (
    IxPerfProfAccXscalePmuEvent pmuEvent1,
    IxPerfProfAccXscalePmuEvent pmuEvent2,
    IxPerfProfAccXscalePmuEvent pmuEvent3,
    IxPerfProfAccXscalePmuEvent pmuEvent4)
{
    UINT32 evtSelRegVal = 0;    /*value to be written into each event counter*/

    switch (numberEvents) /*numberEvents is passed in by client when calling
                           *the start function for event counting/sampling;
                           *the event select register is 32bits, with every 8
                           *bits representing event counters 1-4; the bits
                           *corresponding to each event selected are written
                           *into this register
                           */
    {
        /*only 1 event specified; write value into counter 1
         *counter 1 is the first 8 bits in the event select register
         */
        case 1:
            evtSelRegVal = (pmuEvent1 | IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR1);
            break;

        /*2 events specified; write value into counters 1,2
         *counter 2 is bits 8-15 in the event select register
         */
        case 2:
            evtSelRegVal =
                ((pmuEvent2 <<
                  IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR2_BITS_TO_SHIFT) |
                 pmuEvent1 |
                 IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR2);
            break;

        /*3 events specified; write value into counters 1,2,3
         *counter 3 is bits 16-23 in the event select register
         */
        case 3:
            evtSelRegVal =
                ((pmuEvent3 <<
                  IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR3_BITS_TO_SHIFT) |
                 (pmuEvent2 <<
                  IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR2_BITS_TO_SHIFT) |
                 pmuEvent1 |
                 IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR3);
            break;

        /*4 events specified; write value into counters 1,2,3,4
         *counter 4 is bits 24-31 in the event select register
         */
        case 4:
            evtSelRegVal =
                ((pmuEvent4 <<
                  IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR4_BITS_TO_SHIFT) |
                 (pmuEvent3 <<
                  IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR3_BITS_TO_SHIFT) |
                 (pmuEvent2 <<
                  IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR2_BITS_TO_SHIFT) |
                 pmuEvent1);
            break;

        default:
            break;
    }/*end of switch (numnerEvents)*/

    /*write the value into the Event Select Register*/
    _ixPerfProfAccXscalePmuEvtSelectWrite(evtSelRegVal);
}/*end of ixPerfProfAccXscalePmuEventSelect()*/

void
ixPerfProfAccXscalePmuCtrEnableReset (
    BOOL clkCntDiv,
    BOOL enableCtrs,
    BOOL resetClkCtr,
    BOOL resetEvtCtr)
{
    UINT32 perfMtrCtrlRegBit = 0;   /*value to write to PMNC register*/

    if ( TRUE == clkCntDiv) /*clk counter divider is enabled*/
    {
        perfMtrCtrlRegBit = IX_PERFPROF_ACC_XSCALE_PMU_PMNC_DIVIDER;
    }
    if (enableCtrs) /*if need to enable counters*/
    {
        perfMtrCtrlRegBit = (perfMtrCtrlRegBit |
                             IX_PERFPROF_ACC_XSCALE_PMU_PMNC_ENABLE);
    }
    else
    {
        perfMtrCtrlRegBit = (perfMtrCtrlRegBit |
                             IX_PERFPROF_ACC_XSCALE_PMU_PMNC_DISABLE);
    }/*end of if enableCtrs*/
    if (resetClkCtr)
    {
        perfMtrCtrlRegBit = (perfMtrCtrlRegBit |
                             IX_PERFPROF_ACC_XSCALE_PMU_PMNC_RESET_CLK_CTR);
    }/*end of if resetClkCtr*/
    if (resetEvtCtr)
    {
        perfMtrCtrlRegBit = (perfMtrCtrlRegBit |
                             IX_PERFPROF_ACC_XSCALE_PMU_PMNC_RESET_EVT_CTR);
    } /*end of resetEvtCtr*/

    /*write the appropriate value to the PMNC reg*/
    _ixPerfProfAccXscalePmuPmncWrite(perfMtrCtrlRegBit);
}/*end of ixPerfProfAccXscalePmuCtrEnableReset()*/

IxPerfProfAccStatus
ixPerfProfAccXscalePmuEvtCtrInit (void)
{
    BOOL checkNum = TRUE;   /*determines if num passed into
                             *_ixPerfProfAccXscalePmuPmnWrite is valid
                             */
    if ((0 == numberEvents ) ||
       (IX_PERFPROF_ACC_XSCALE_PMU_MAX_EVENTS <
        numberEvents ))
    {
        return IX_PERFPROF_ACC_STATUS_FAIL;
    }
    switch (numberEvents)
    {
        case 4:
            _ixPerfProfAccXscalePmuPmnWrite(
                IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR4_ID,
                ctrBase[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR4_ID],
                &checkNum);
            if (FALSE == checkNum)/*num passed into
                                   *_ixPerfProfAccXscalePmuPmnWrite is
                                   *greater than max allowed event counters
                                   */
            {
                return IX_PERFPROF_ACC_STATUS_XSCALE_PMU_NUM_INVALID;
            }

        case 3:
            _ixPerfProfAccXscalePmuPmnWrite(
                IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR3_ID,
                ctrBase[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR3_ID],
                &checkNum);
            if (FALSE == checkNum)/*num passed into
                                   *_ixPerfProfAccXscalePmuPmnWrite is
                                   *greater than max allowed event counters
                                   */
            {
                return IX_PERFPROF_ACC_STATUS_XSCALE_PMU_NUM_INVALID;
            }

        case 2:
            _ixPerfProfAccXscalePmuPmnWrite(
                IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR2_ID,
                ctrBase[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR2_ID],
                &checkNum);
            if (FALSE == checkNum)/*num passed into
                                   *_ixPerfProfAccXscalePmuPmnWrite is
                                   *greater than max allowed event counters
                                   */
            {
                return IX_PERFPROF_ACC_STATUS_XSCALE_PMU_NUM_INVALID;
            }

        case 1:
            _ixPerfProfAccXscalePmuPmnWrite(
                IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR1_ID,
                ctrBase[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR1_ID],
                &checkNum);
            if (FALSE == checkNum)/*num passed into
                                   *_ixPerfProfAccXscalePmuPmnWrite is
                                   *greater than max allowed event counters
                                   */
            {
                return IX_PERFPROF_ACC_STATUS_XSCALE_PMU_NUM_INVALID;
            }
            break;

        default:
            break;

    }/*end of switch (numberEvents)*/
    return IX_PERFPROF_ACC_STATUS_SUCCESS;
}/*end of ixPerfProfAccXscalePmuEvtCtrInit()*/

void
ixPerfProfAccXscalePmuIntrDisable (void)
{
    /*write to the PMNC to disable all clock and event counters*/
    _ixPerfProfAccXscalePmuPmncWrite(IX_PERFPROF_ACC_XSCALE_PMU_PMNC_DISABLE);

    /*disable all counter interrupts in interrupt enable register*/
    _ixPerfProfAccXscalePmuIntenWrite(
        IX_PERFPROF_ACC_XSCALE_PMU_INTR_DISABLE_ALL);

    /*disable xscale interrupt in the interrupt controller register*/
    ixOsalIrqUnbind(IX_PERFPROF_ACC_XSCALE_PMU_BSP_INTR_BIT);
}   /* end of ixPerfProfAccXscalePmuIntrDisable() */

void
ixPerfProfAccXscalePmuClkCntGet (IxPerfProfAccXscalePmuEvtCnt *clkCount)
{
    UINT32 currentCnt = _ixPerfProfAccXscalePmuCcntRead();  /*get current value
                                                             *of clk counter
                                                             */

    if (!eventCounting)  /*for time-based sampling*/
    {
        if (IxPerfProfAccXscalePmuIntrStatus[
                IX_PERFPROF_ACC_XSCALE_PMU_CLK_CTR_ID] == TRUE)
        {
            /*logs a warning*/
            IX_PERFPROF_ACC_LOG(
		IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDOUT,
                "*** Warning: Profiling table is full!\n\n",
                0,0, 0, 0, 0, 0);
        } /*end of if IxPerfProfAccXscalePmuIntrStatus*/
    }/*end of if !eventCounting*/

    /*assign value of end clock count*/
    clkCount->lower32BitsEventCount =
        currentCnt -
        ctrBase[IX_PERFPROF_ACC_XSCALE_PMU_CLK_CTR_ID];

    /*assign value of number of overflows for the clock counter*/
    clkCount->upper32BitsEventCount =
        ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_CLK_CTR_ID];
} /*end of ixPerfProfAccXscalePmuClkCntGet()*/

IxPerfProfAccStatus
ixPerfProfAccXscalePmuEvtCntGet (IxPerfProfAccXscalePmuEvtCnt *eventCount)
{
    UINT32 i;
    IxPerfProfAccStatus status = IX_PERFPROF_ACC_STATUS_SUCCESS;
    UINT32 currentCnt[IX_PERFPROF_ACC_XSCALE_PMU_MAX_EVENTS];/*local to this
                                                              *function
                                                              */
    BOOL checkNum = TRUE;
    for (i=0; i<IX_PERFPROF_ACC_XSCALE_PMU_MAX_EVENTS; i++)
    {
        currentCnt[i] = _ixPerfProfAccXscalePmuPmnRead(i, &checkNum);
        /*num events passed in to _ixPerfProfAccXscalePmuPmnRead is invalid*/
        if (FALSE == checkNum)
        {
            status = IX_PERFPROF_ACC_STATUS_XSCALE_PMU_NUM_INVALID;
            return status;
        } /*end of if FALSE==checkNum*/
    }/*end of for IX_PERFPROF_ACC_XSCALE_PMU_MAX_EVENTS loop*/

    for (i=0; i < numberEvents; i++)
    {
        /*results overflow use next 32bit counter to store overflow for event
         *counting
         */
        if(((eventCounting) &&
            (IxPerfProfAccXscalePmuIntrStatus[i] == TRUE))||
            (!eventCounting))
        {
            switch (i)
            {
                case 0:
                    eventCount[i].upper32BitsEventCount =
                        ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR1_ID];
                    break;

                case 1:
                    eventCount[i].upper32BitsEventCount =
                        ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR2_ID];
                    break;

                case 2:
                    eventCount[i].upper32BitsEventCount =
                        ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR3_ID];
                    break;

                case 3:
                    eventCount[i].upper32BitsEventCount =
                        ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR4_ID];
                    break;

                default:
                    break;

                } /*end of switch*/
        }/*end of if eventCounting*/
        /*always store end event count in lower 32 bits counter*/
        eventCount[i].lower32BitsEventCount =
            currentCnt[i] -
            ctrBase[i];
    }/*end of for numberEvents*/
    return status;
}/*end of ixPerfProfAccXscalePmuEvtCntGet()*/

void
ixPerfProfAccXscalePmuProfileConstruct (
    UINT32 pcAddr,
    IxPerfProfAccXscalePmuSamplePcProfile *profile)
{
    UINT32 i;

    for(i=0; IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES > i; i++)
    {
        if (profile[i].programCounter == pcAddr)   /*this PC already captured*/
        {
            profile[i].freq++; /*add to the frequency*/
            break;  /*break out of loop because this entry has been captured*/
        }
        else if (profile[i].programCounter== 0)    /*this PC is captured for
                                                    *the first time
                                                    */
        {
            profile[i].programCounter = pcAddr;
            profile[i].freq = 1;
            sumLen++;    /*number of different pc adds captured*/
            break;      /*break out of loop because entry has been captured*/
        }/*end of if-else profile[i]*/
    }/*end of for IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES>i*/
}/*end of ixPerfProfAccXscalePmuProfileConstruct()*/

void
ixPerfProfAccXscalePmuProfileSort (IxPerfProfAccXscalePmuSamplePcProfile *profileArray, UINT32 total)
{
    UINT32 i;           /* Counter to go through values in the array */
    UINT32 j;           /* Counter to compare with other value in array */
    UINT32 profileMax;  /* Higest value in array */
    UINT32 indexTempMax;/* Index of array component that contains maximum value */
    UINT32 pc;          /* Program counter value */
    for (i = 0 ; i < total; i++)
    {
        indexTempMax=i;
        /* Look for any value higher than the current value and store it in its location */
        profileMax = profileArray[i].freq;
        for (j =i ; j < total; j++)
        {
            if (profileMax < profileArray[j].freq)
            {
               indexTempMax=j;
               profileMax = profileArray[j].freq;
            } /* end if */

        } /* end for */
        /* swap freq */
        profileArray[indexTempMax].freq=profileArray[i].freq;
        profileArray[i].freq = profileMax;
        /* swap pc */
        pc = profileArray[indexTempMax].programCounter;
        profileArray[indexTempMax].programCounter = profileArray[i].programCounter;
        profileArray[i].programCounter= pc;
    } /* end for */
} /* end ixPerfProfAccXscalePmuProfileSort */

PUBLIC IxPerfProfAccStatus
ixPerfProfAccXscalePmuEventCountStart(
    BOOL clkCntDiv,
    UINT32 numEvents,
    IxPerfProfAccXscalePmuEvent pmuEvent1,
    IxPerfProfAccXscalePmuEvent pmuEvent2,
    IxPerfProfAccXscalePmuEvent pmuEvent3,
    IxPerfProfAccXscalePmuEvent pmuEvent4 )
{
    UINT32 i;
    IxPerfProfAccStatus status = IX_PERFPROF_ACC_STATUS_SUCCESS;
    IxFeatureCtrlDeviceId deviceType = IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X;
    numberEvents = numEvents;
    
    deviceType = ixFeatureCtrlDeviceRead ();
    if(IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X == deviceType)
    {
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixPerfProfAcc - This PPU component is not supported\n",
            0, 0, 0, 0, 0, 0);
        return IX_PERFPROF_ACC_STATUS_COMPONENT_NOT_SUPPORTED;
    }

    /* initialize global variables*/
    ctrBase[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR1_ID] = 0;
    ctrBase[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR2_ID] = 0;
    ctrBase[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR3_ID] = 0;
    ctrBase[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR4_ID] = 0;
    ctrBase[IX_PERFPROF_ACC_XSCALE_PMU_CLK_CTR_ID] = 0;
    ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR1_ID] = 0;
    ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR2_ID] = 0;
    ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR3_ID] = 0;
    ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR4_ID] = 0;
    ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_CLK_CTR_ID] = 0;
    eventCounting = TRUE;   /*indicates that event counting is on*/

    /*check if any other util is currently running*/
    status = ixPerfProfAccLock();
    if (IX_PERFPROF_ACC_STATUS_ANOTHER_UTIL_IN_PROGRESS == status)
    {
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "Another utility is running\n",
            0,0, 0, 0, 0, 0);
        return status;
    }

    /*check validity of input parameters*/
    if (IX_PERFPROF_ACC_XSCALE_PMU_MAX_EVENTS < numEvents  )
    {
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "numEvents cannot be greater than 4\n",
            0,0, 0, 0, 0, 0);
        ixPerfProfAccUnlock(); /*call unlock; process has been terminated*/
        return IX_PERFPROF_ACC_STATUS_XSCALE_PMU_NUM_INVALID;
    } /*end of if IX_PERFPROF_ACC_XSCALE_PMU_MAX_EVENTS*/

    if((IX_PERFPROF_ACC_XSCALE_PMU_EVENT_MAX < pmuEvent1) ||
       (IX_PERFPROF_ACC_XSCALE_PMU_EVENT_MAX < pmuEvent2) ||
       (IX_PERFPROF_ACC_XSCALE_PMU_EVENT_MAX < pmuEvent3) ||
       (IX_PERFPROF_ACC_XSCALE_PMU_EVENT_MAX < pmuEvent4))
    {
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "value of event type entered is out of bounds\n",
            0,0, 0, 0, 0, 0);
        ixPerfProfAccUnlock(); /*call unlock; process has been terminated*/
        return IX_PERFPROF_ACC_STATUS_XSCALE_PMU_EVENT_INVALID;
    }/*end of if IX_PERFPROF_ACC_XSCALE_PMU_EVENT_MAX*/

    if (numEvents)  /*events counting*/
    {
        /* initialize interrupt status array */
        for (i=0;i<IX_PERFPROF_ACC_XSCALE_PMU_MAX_COUNTERS;i++)
        {
            IxPerfProfAccXscalePmuIntrStatus[i] = FALSE;
        }/* end of for loop */
    }
    else    /*clock counting only*/
    {
        IxPerfProfAccXscalePmuIntrStatus[
            IX_PERFPROF_ACC_XSCALE_PMU_CLK_CTR_ID] = FALSE;
    }/*end of if-else numEvents*/

    /* bind interrupt to handler*/
    ixPerfProfAccXscalePmuIntrConnect();

    /*assign the appropriate events to each event counter*/
    if (numEvents)
    {
        ixPerfProfAccXscalePmuEventSelect (
            pmuEvent1,
            pmuEvent2,
            pmuEvent3,
            pmuEvent4);
    }
    else    /*if no event counting, write default value to all event counters to
             *save power
             */
    {
        _ixPerfProfAccXscalePmuEvtSelectWrite(
            IX_PERFPROF_ACC_XSCALE_PMU_CLK_CTR);
    }/*end of if-else numEvents*/

    /* Clear all bits in overflow flag register*/
    _ixPerfProfAccXscalePmuOverFlowWrite(
        IX_PERFPROF_ACC_XSCALE_PMU_OFLOW_FLAG_ALL);

    /*write the pmu interrupt enable register*/
    ixPerfProfAccXscalePmuIntrEnable(TRUE);  /*clk ctr is on*/

    /*Configure  Performance Monitor Control Register - enable all counters,
     *reset event counters, reset clock counter, enable clock divider if
     *appropriate
     */
    ixPerfProfAccXscalePmuCtrEnableReset (clkCntDiv, TRUE, TRUE, TRUE);

    /*enable BSP interrupts*/
    ixPerfProfAccXscalePmuBspIntrEnable();

    /* initialize evt counters only if event counting on*/
    if (numEvents)
    {
        status = ixPerfProfAccXscalePmuEvtCtrInit();
        if (status == IX_PERFPROF_ACC_STATUS_FAIL)
        {
            /*error due to num events out of bounds*/
            IX_PERFPROF_ACC_LOG(
		IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDOUT,
                "Warning: error initializing event counter\n",
                0,0, 0, 0, 0, 0);
            ixPerfProfAccUnlock(); /*call unlock; process has been terminated*/
        } /*end of if status*/
    }/*end of if numEvents*/

    /*set clock counter to zero*/
    _ixPerfProfAccXscalePmuCcntWrite(
        ctrBase[IX_PERFPROF_ACC_XSCALE_PMU_CLK_CTR_ID]);
    eventCountStarted = TRUE; /*set to show that this process has been started*/
    return status;
}/*end of ixPerfProfAccXscalePmuEventCountStart()*/


PUBLIC IxPerfProfAccStatus
ixPerfProfAccXscalePmuEventCountStop (
    IxPerfProfAccXscalePmuResults *eventCountStopResults)
{
    UINT32 i;
    IxPerfProfAccXscalePmuEvtCnt  evtCnt[IX_PERFPROF_ACC_XSCALE_PMU_MAX_EVENTS];

    /*check whether ixPerfProfAccXscalePmuEventCountStart() has been called*/
    if (!eventCountStarted)
    {
        return IX_PERFPROF_ACC_STATUS_XSCALE_PMU_START_NOT_CALLED;
    }

    /*error check the parameter*/
    if (NULL == eventCountStopResults)
    {
	/* report the error */ 
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixPerfProfAccXscalePmuEventCountStop - eventCountStopResults is invalid\n",
            0, 0, 0, 0, 0, 0);
	/* return error */
        return IX_PERFPROF_ACC_STATUS_FAIL;
    }

    /*initialize counter results array*/
    for(i=0; i<IX_PERFPROF_ACC_XSCALE_PMU_MAX_EVENTS; i++)
    {
        evtCnt[i].lower32BitsEventCount = 0;
        evtCnt[i].upper32BitsEventCount = 0;
    } /*end of for loop*/

    /*disable all interrupts to stop counting*/
    ixPerfProfAccXscalePmuIntrDisable();

    /*get the final clock and event counts*/
    ixPerfProfAccXscalePmuResultsGet(eventCountStopResults);

    eventCounting = FALSE;  /*event counting has ended*/
    eventCountStarted = FALSE;  /*reset flag as event counting has ended*/

    /*call unlock because process has ended*/
    ixPerfProfAccUnlock();
    return IX_PERFPROF_ACC_STATUS_SUCCESS;
 }/*end of ixPerfProfAccXscalePmuEventCountStop()*/

PUBLIC IxPerfProfAccStatus
ixPerfProfAccXscalePmuTimeSampStart(
    UINT32 samplingRate,
    BOOL clkCntDiv)
{
    IxPerfProfAccStatus status = IX_PERFPROF_ACC_STATUS_SUCCESS;
    IxFeatureCtrlDeviceId deviceType = IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X;
 
    deviceType = ixFeatureCtrlDeviceRead ();
    if(IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X == deviceType)
    {
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixPerfProfAcc - This PPU component is not supported\n",
            0, 0, 0, 0, 0, 0);
        return IX_PERFPROF_ACC_STATUS_COMPONENT_NOT_SUPPORTED;
    }

    /*check if any other util is currently running*/
    status = ixPerfProfAccLock();
    if (IX_PERFPROF_ACC_STATUS_ANOTHER_UTIL_IN_PROGRESS == status)
    {
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "Another utility is running\n",
            0,0, 0, 0, 0, 0);
        return status;
    }

    /*check validity of parameters*/
    if ((0 == samplingRate) ||
       (IX_PERFPROF_ACC_XSCALE_PMU_MAX_COUNTER_VALUE < samplingRate))
    {
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "samplingRate can't be zero or greater than counter size\n",
            0,0, 0, 0, 0, 0);
        ixPerfProfAccUnlock(); /*call unlock; process has been terminated*/
        return IX_PERFPROF_ACC_STATUS_FAIL;
    } /*end of if samplingRate*/
    if ((FALSE != clkCntDiv) &&
       (TRUE!= clkCntDiv))
    {
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "enter TRUE or FALSE for clkCntDiv\n",
            0,0, 0, 0, 0, 0);
        ixPerfProfAccUnlock(); /*call unlock; process has been terminated*/
        return IX_PERFPROF_ACC_STATUS_FAIL;
    } /*end of if clkCntDiv*/

    /*initialize globals*/
    ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_CLK_CTR_ID] = 0;
    ctrBase[IX_PERFPROF_ACC_XSCALE_PMU_CLK_CTR_ID] =
        IX_PERFPROF_ACC_XSCALE_PMU_MAX_COUNTER_VALUE -
        samplingRate;
    ctrBase[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR1_ID] = 0;
    ctrBase[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR2_ID] = 0;
    ctrBase[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR3_ID] = 0;
    ctrBase[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR4_ID] = 0;
    eventCounting = FALSE;
    numberEvents=0;

    /*initialize interrupt status for clock counter only*/
    IxPerfProfAccXscalePmuIntrStatus[IX_PERFPROF_ACC_XSCALE_PMU_CLK_CTR_ID] =
        FALSE;

    /*bind interrupt to handler*/
    ixPerfProfAccXscalePmuIntrConnect();

    /* Clear clk ctr bits in overflow flag register*/
    _ixPerfProfAccXscalePmuOverFlowWrite(
        IX_PERFPROF_ACC_XSCALE_PMU_OFLOW_FLAG_CCNT);

    /*write to pmu interrupt enable register-clk counting only, and clk ctr on*/
    ixPerfProfAccXscalePmuIntrEnable(TRUE);

    /*Configure  Performance Monitor Control Register - enable all counters,
     * reset clock counter, enable clock divider if appropriate
     */
    ixPerfProfAccXscalePmuCtrEnableReset (clkCntDiv, TRUE, TRUE, FALSE);

    /*enable BSP interrupt*/
    ixPerfProfAccXscalePmuBspIntrEnable();

    /*initialize clk ctr to base value*/
    _ixPerfProfAccXscalePmuCcntWrite(
        ctrBase[IX_PERFPROF_ACC_XSCALE_PMU_CLK_CTR_ID]);

    /*no event counting, write default value to all event ctrs to save power*/
    _ixPerfProfAccXscalePmuEvtSelectWrite(IX_PERFPROF_ACC_XSCALE_PMU_CLK_CTR);
    timeSampStarted = TRUE; /*set to show that this process has been started*/
    return IX_PERFPROF_ACC_STATUS_SUCCESS;
}

PUBLIC IxPerfProfAccStatus
ixPerfProfAccXscalePmuTimeSampStop(
    IxPerfProfAccXscalePmuEvtCnt *clkCount,
    IxPerfProfAccXscalePmuSamplePcProfile *timeProfile)
{
    UINT32 i;
    UINT32 samples;

#ifdef __vxworks
    char     symbolName[IX_PERFPROF_ACC_XSCALE_PMU_SYMBOL_SIZE];  /* Location where the symbol name 
                                                                     from search will be stored */
    int      symbolValue;           /* Location where the correct symbol address is stored */
    SYM_TYPE pointerType;
    FILE     *fpTimeSampResults;    /* Pointer to output file */
    float    percentage;
#endif

    /*check whether ixPerfProfAccXscalePmuTimeSampStart() has been called*/
    if (!timeSampStarted)
    {
        return IX_PERFPROF_ACC_STATUS_XSCALE_PMU_START_NOT_CALLED;
    }

    /*error check the parameter*/
    if (NULL == clkCount)
    {
	/* report the error */ 
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixPerfProfAccXscalePmuTimeSampStop - clkCount is invalid\n",
            0, 0, 0, 0, 0, 0);
	/* return error */
        return IX_PERFPROF_ACC_STATUS_FAIL;
    }

    /*error check the parameter*/
    if (NULL == timeProfile)
    {
	/* report the error */ 
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixPerfProfAccXscalePmuTimeSampStop - timeProfile is invalid\n",
            0, 0, 0, 0, 0, 0);
	/* return error */
        return IX_PERFPROF_ACC_STATUS_FAIL;
    }

    /*disable all interrupts to stop counting*/
    ixPerfProfAccXscalePmuIntrDisable();
    /*get the final clock count and number of samples taken*/
    ixPerfProfAccXscalePmuClkCntGet (clkCount);

    /*construct profiling summary*/
    for (i=0; i<ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_CLK_CTR_ID]; i++)
    {
        ixPerfProfAccXscalePmuProfileConstruct (clkCtrArray[i], timeProfile);
    }/*end of for ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_CLK_CTR_ID]*/
    timeSampStarted = FALSE; /*reset flag as time sampling has ended*/

    /*call unlock because process has ended*/
    ixPerfProfAccUnlock();

    ixPerfProfAccXscalePmuProfileSort(timeProfile, clkCount->upper32BitsEventCount);

    #ifdef __vxworks
    /* Create file called timeSampleResults.txt */
    fpTimeSampResults = fopen("timeSampleResults.txt","w");
    /* Check if fopen is successful. If not, log message */
    if(NULL == fpTimeSampResults)
    {
        IX_PERFPROF_ACC_LOG(
                IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDOUT,
                "*** Warning: Time Sampling Results file could not be generated.\n\n",
                0,0, 0, 0, 0, 0);
    }
    else
    {
        /* print table header to file */
        fprintf(fpTimeSampResults,"Total Number of samples = %u\n\n\n", 
                                     clkCount->upper32BitsEventCount);
        fprintf(fpTimeSampResults,"Hits    Percent PC Address Symbol Address Offset ClosestRoutine\n");
        fprintf(fpTimeSampResults,"------- ------- ---------- -------------- ------ --------------\n");

        /* Find symbol names for PC address and write values to file */
        for (samples = 0; samples < IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES; samples++)
        {
            if(timeProfile[samples].freq > 0)
            {
                symFindByValue (sysSymTbl, timeProfile[samples].programCounter, symbolName,
                                      &symbolValue, &pointerType);
                percentage=((float)timeProfile[samples].freq/
                                      (float)clkCount->upper32BitsEventCount)*100;
                fprintf (fpTimeSampResults,"%7u %7.4f %10x %14x %6x %s\n", 
                                      timeProfile[samples].freq, 
                                      percentage,
                                      timeProfile[samples].programCounter,
                                      symbolValue,
                                      timeProfile[samples].programCounter - symbolValue,
                                      symbolName); 
            }
            else
            {
                break;
            }
        }

        /* Close pointer to output file */
        fclose(fpTimeSampResults);
    
    }/* if - else NULL == fpTimeSampResults */
 
    /* For Linux copy results to global variable */
    #elif defined (__linux) 
    for (samples = 0; samples < clkCount->upper32BitsEventCount; samples++)
    {
        timeSampProfile[samples] = timeProfile[samples];
    }
    #endif
    return IX_PERFPROF_ACC_STATUS_SUCCESS;
}

PUBLIC IxPerfProfAccStatus
ixPerfProfAccXscalePmuEventSampStart(
    UINT32 numEvents,
    IxPerfProfAccXscalePmuEvent pmuEvent1,
    UINT32 eventRate1,
    IxPerfProfAccXscalePmuEvent pmuEvent2,
    UINT32 eventRate2,
    IxPerfProfAccXscalePmuEvent pmuEvent3,
    UINT32 eventRate3,
    IxPerfProfAccXscalePmuEvent pmuEvent4,
    UINT32 eventRate4)
{
    UINT32  i;
    IxPerfProfAccStatus status = IX_PERFPROF_ACC_STATUS_SUCCESS;
    IxFeatureCtrlDeviceId deviceType = IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X;
 
    deviceType = ixFeatureCtrlDeviceRead ();
    if(IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X == deviceType)
    { 
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixPerfProfAcc - This PPU component is not supported\n",
            0, 0, 0, 0, 0, 0);
        return IX_PERFPROF_ACC_STATUS_COMPONENT_NOT_SUPPORTED;
    }

    /*check if any other util is currently running*/
    status = ixPerfProfAccLock();
    if (IX_PERFPROF_ACC_STATUS_ANOTHER_UTIL_IN_PROGRESS == status)
    {
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "Another utility is running\n",
            0,0, 0, 0, 0, 0);
        return status;
    }

    /*check validity of input parameters*/
    if ((IX_PERFPROF_ACC_XSCALE_PMU_MAX_EVENTS < numEvents) || (0 == numEvents))
    {
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "range of numEvents is between 1 and 4\n",
            0,0, 0, 0, 0, 0);
        ixPerfProfAccUnlock(); /*call unlock; process has been terminated*/
        return IX_PERFPROF_ACC_STATUS_XSCALE_PMU_NUM_INVALID;
    }/*end of if numEvents*/
    if((IX_PERFPROF_ACC_XSCALE_PMU_EVENT_MAX < pmuEvent1) ||
       (IX_PERFPROF_ACC_XSCALE_PMU_EVENT_MAX < pmuEvent2) ||
       (IX_PERFPROF_ACC_XSCALE_PMU_EVENT_MAX < pmuEvent3) ||
       (IX_PERFPROF_ACC_XSCALE_PMU_EVENT_MAX < pmuEvent4))
    {
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "value of event type entered is out of bounds\n",
            0,0, 0, 0, 0, 0);
        ixPerfProfAccUnlock(); /*call unlock; process has been terminated*/
        return IX_PERFPROF_ACC_STATUS_XSCALE_PMU_EVENT_INVALID;
    }/*end of if pmuEvent*/

    numberEvents = numEvents;

    /*initialize globals*/
    ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR1_ID] = 0;
    ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR2_ID] = 0;
    ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR3_ID] = 0;
    ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR4_ID] = 0;
    eventCounting = FALSE;

    /* set ctr base values
     * First initialize all to zero. Then set to the right value as requested by
     *users.  The start value is MAX (0xFFFFFFFF) minus eventRate. The result is
     *exactly eventRate away from overflow.
     */

    ctrBase[IX_PERFPROF_ACC_XSCALE_PMU_CLK_CTR_ID] = 0;
    ctrBase[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR1_ID] = 0;
    ctrBase[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR2_ID] = 0;
    ctrBase[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR3_ID] = 0;
    ctrBase[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR4_ID] = 0;

    /* If user does not select zero, then we need to set the start based to
     *MAX (0xFFFFFFFF) minus eventRate
     */
    if (0 != eventRate1)/*if rate is zero, that is base*/
    {
        ctrBase[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR1_ID] =
            IX_PERFPROF_ACC_XSCALE_PMU_MAX_COUNTER_VALUE -
            eventRate1;
    }
    if (0 != eventRate2)/*if rate is zero, that is base*/
    {
        ctrBase[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR2_ID] =
            IX_PERFPROF_ACC_XSCALE_PMU_MAX_COUNTER_VALUE -
            eventRate2;
    }
    if (0 != eventRate3)/*if rate is zero, that is base*/
    {
        ctrBase[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR3_ID] =
            IX_PERFPROF_ACC_XSCALE_PMU_MAX_COUNTER_VALUE -
            eventRate3;
    }
    if (0 != eventRate4)/*if rate is zero, that is base*/
    {
        ctrBase[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR4_ID] =
            IX_PERFPROF_ACC_XSCALE_PMU_MAX_COUNTER_VALUE -
            eventRate4;
    }

    /*initialize interrupt status for all counters*/
    for (i=0;i<IX_PERFPROF_ACC_XSCALE_PMU_MAX_EVENTS;i++)
    {
        IxPerfProfAccXscalePmuIntrStatus[i] = FALSE;
    }/*end of for loop*/

    /*bind interrupt to handler*/
    ixPerfProfAccXscalePmuIntrConnect();

    /*assign the appropriate events to each event counter*/
    ixPerfProfAccXscalePmuEventSelect (
        pmuEvent1,
        pmuEvent2,
        pmuEvent3,
        pmuEvent4);

    /* Clear all evt ctr bits in overflow flag register*/
    _ixPerfProfAccXscalePmuOverFlowWrite(
        IX_PERFPROF_ACC_XSCALE_PMU_OFLOW_FLAG_PMN);

    /*write pmu interrupt enable register-no clk counting, and clk ctr is off*/
    ixPerfProfAccXscalePmuIntrEnable(FALSE);
    /*Configure  Performance Monitor Control Register - enable all counters,
     * reset event counters, no clock divider
     */
    ixPerfProfAccXscalePmuCtrEnableReset (
        FALSE,
        TRUE,
        FALSE,
        TRUE);

    /*enable BSP interrupt*/
    ixPerfProfAccXscalePmuBspIntrEnable();

    /*initialize event ctrs to base value*/
    status = ixPerfProfAccXscalePmuEvtCtrInit();
    if (status == IX_PERFPROF_ACC_STATUS_FAIL)
    {
    /*error due to num events out of bounds*/
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDOUT,
            "Warning: error initializing event counter\n",
            0,0, 0, 0, 0, 0);
        ixPerfProfAccUnlock(); /*call unlock; process has been terminated*/
    } /*end of if status*/
    eventSampStarted = TRUE; /*set to show that this process has been started*/
    return status;
}

PUBLIC IxPerfProfAccStatus
ixPerfProfAccXscalePmuEventSampStop(
    IxPerfProfAccXscalePmuSamplePcProfile *eventProfile1,
    IxPerfProfAccXscalePmuSamplePcProfile *eventProfile2,
    IxPerfProfAccXscalePmuSamplePcProfile *eventProfile3,
    IxPerfProfAccXscalePmuSamplePcProfile *eventProfile4)
{
    UINT32 i;
    UINT32 j;
    UINT32   samples;
    IxPerfProfAccXscalePmuResults eventSampResults;
    #ifdef __vxworks
    char     symbolName[IX_PERFPROF_ACC_XSCALE_PMU_SYMBOL_SIZE]; /* Location where symbold 
                                                                    name for pc addr is stored */
    int      symbolValue;         /* Location where correct pc addr is stored */
    SYM_TYPE pointerType;
    FILE   *fpEventSampResults;   /* Pointer to results output file */
    float    percentage;
    #endif

    /*check whether ixPerfProfAccXscalePmuTimeSampStart() has been called*/
    if (!eventSampStarted)
    {
        return IX_PERFPROF_ACC_STATUS_XSCALE_PMU_START_NOT_CALLED;
    }

    /*error check the parameter*/
    if (NULL == eventProfile1)
    {
	/* report the error */ 
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixPerfProfAccXscalePmuEventSampStop - eventProfile1 is invalid\n",
            0, 0, 0, 0, 0, 0);
	/* return error */
        return IX_PERFPROF_ACC_STATUS_FAIL;
    }

    /*error check the parameter*/
    if (NULL == eventProfile2)
    {
	/* report the error */ 
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixPerfProfAccXscalePmuEventSampStop - eventProfile2 is invalid\n",
            0, 0, 0, 0, 0, 0);
	/* return error */
        return IX_PERFPROF_ACC_STATUS_FAIL;
    }

    /*error check the parameter*/
    if (NULL == eventProfile3)
    {
	/* report the error */ 
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixPerfProfAccXscalePmuEventSampStop - eventProfile3 is invalid\n",
            0, 0, 0, 0, 0, 0);
	/* return error */
        return IX_PERFPROF_ACC_STATUS_FAIL;
    }

    /*error check the parameter*/
    if (NULL == eventProfile4)
    {
	/* report the error */ 
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixPerfProfAccXscalePmuEventSampStop - eventProfile4 is invalid\n",
            0, 0, 0, 0, 0, 0);
	/* return error */
        return IX_PERFPROF_ACC_STATUS_FAIL;
    }

    /*disable all interrupts to stop counting*/
    ixPerfProfAccXscalePmuIntrDisable();

    /*construct profiling summary*/
    for (i=0; i<IX_PERFPROF_ACC_XSCALE_PMU_MAX_EVENTS; i++)
    {
        sumLen = 0;
        switch(i)
        {
            case 0:
                for (j=0; j<ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR1_ID]; j++)
                {
                    ixPerfProfAccXscalePmuProfileConstruct (
                        evtCtr1Array[j],
                        eventProfile1);
                }/*end of for ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR1_ID]*/
                break;

            case 1:
                for (j=0; j<ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR2_ID]; j++)
                {
                    ixPerfProfAccXscalePmuProfileConstruct (
                        evtCtr2Array[j],
                        eventProfile2);
                }/*end of for ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR2_ID]*/
                break;

            case 2:
                for (j=0; j<ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR3_ID]; j++)
                {
                    ixPerfProfAccXscalePmuProfileConstruct(
                        evtCtr3Array[j],
                        eventProfile3);
                }/*end of for ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR2_ID]*/
                break;

            case 3:
                for (j=0; j<ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR4_ID]; j++)
                {
                    ixPerfProfAccXscalePmuProfileConstruct(
                        evtCtr4Array[j],
                        eventProfile4);
                }/*end of for ctrSamples[IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR2_ID]*/
                break;

            default:
                break;
        }/*end of switch*/
    }/*end of for IX_PERFPROF_ACC_XSCALE_PMU_MAX_EVENTS*/
    eventSampStarted = FALSE; /*reset flag as event sampling has ended*/

    /*call unlock because process has ended*/
    ixPerfProfAccUnlock();
 
    ixPerfProfAccXscalePmuResultsGet(&eventSampResults);
    /* For each event profile, sort the profile out into descending order of frequencies */
    ixPerfProfAccXscalePmuProfileSort(eventProfile1, eventSampResults.event1_samples);
    ixPerfProfAccXscalePmuProfileSort(eventProfile2, eventSampResults.event2_samples);
    ixPerfProfAccXscalePmuProfileSort(eventProfile3, eventSampResults.event3_samples);
    ixPerfProfAccXscalePmuProfileSort(eventProfile4, eventSampResults.event4_samples);

    #ifdef __vxworks
    fpEventSampResults = fopen("eventSampleResults.txt","w");
    /* Check if failure in file creation. If there is, log message. */
    if(NULL == fpEventSampResults)
    {
        IX_PERFPROF_ACC_LOG(
                IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDOUT,
                "*** Warning: Event Sampling Results file could not be generated.\n\n",
                0,0, 0, 0, 0, 0);
    }
    
    else 
    {
        /* Print table header to file */
        fprintf(fpEventSampResults,"Total Number of samples for Event 1= %u\n\n\n",
                                     eventSampResults.event1_samples);
        fprintf(fpEventSampResults,"Hits    Percent PC Address Symbol Address Offset ClosestRoutine\n");
        fprintf(fpEventSampResults,"------- ------- ---------- -------------- ------ --------------\n");

        /* For each event write profile address, frequency and symbol to file. */
        for (samples = 0; samples < IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES; samples++)
        {
           percentage=((float)eventProfile1[samples].freq/
                             (float)eventSampResults.event1_samples)*100;
           if(eventProfile1[samples].freq > 0)
           {
               symFindByValue (sysSymTbl, eventProfile1[samples].programCounter, symbolName,
                                               &symbolValue, &pointerType);
               fprintf (fpEventSampResults,"%7u %7.4f %10x %14x %6x %s\n",
                                         eventProfile1[samples].freq,
                                         percentage,
                                         eventProfile1[samples].programCounter,
                                         symbolValue,
                                         eventProfile1[samples].programCounter - symbolValue,
                                         symbolName);
           }
           else
           {
               break;
           }
        }
    
        
        fprintf(fpEventSampResults,"---------------------------------------------------\n\n\n");
        /* Print table header to output file */
        fprintf(fpEventSampResults,"Total Number of samples for Event 2= %u\n\n\n",
                                                     eventSampResults.event2_samples);
        fprintf(fpEventSampResults,"Hits    Percent PC Address Symbol Address Offset ClosestRoutine\n");
        fprintf(fpEventSampResults,"------- ------- ---------- -------------- ------ --------------\n");
        /* For every sample in profile, display address frequency and symbol name */ 
        for (samples = 0; samples < IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES; samples++)
        {
           percentage=((float)eventProfile2[samples].freq/
                             (float)eventSampResults.event2_samples)*100;
           if(eventProfile2[samples].freq > 0)
           {
               symFindByValue (sysSymTbl, eventProfile2[samples].programCounter, symbolName,
                                &symbolValue, &pointerType);
               fprintf (fpEventSampResults,"%7u %7.4f %10x %14x %6x %s\n",
                                eventProfile2[samples].freq,
                                percentage,
                                eventProfile2[samples].programCounter,
                                symbolValue,
                                eventProfile2[samples].programCounter - symbolValue,
                                symbolName);
           }
           else
           {
               break;
           }
        }
  
        fprintf(fpEventSampResults,"---------------------------------------------------\n\n\n");
        /* Print table header into output file */
        fprintf(fpEventSampResults,"Total Number of samples for Event 3= %u\n\n\n",
                                                    eventSampResults.event3_samples);
        fprintf(fpEventSampResults,"Hits    Percent PC Address Symbol Address Offset ClosestRoutine\n");
        fprintf(fpEventSampResults,"------- ------- ---------- -------------- ------ --------------\n");
        /* For every sample in profile, print address, frequency and symbol name */ 
        for (samples = 0; samples < IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES; samples++)
        { 
            percentage = ((float)eventProfile3[samples].freq/
                           (float)eventSampResults.event3_samples)*100;
            if(eventProfile3[samples].freq > 0)
            {
                symFindByValue (sysSymTbl, eventProfile3[samples].programCounter, symbolName,
                                &symbolValue, &pointerType);
                fprintf (fpEventSampResults,"%7u %7.4f %10x %14x %6x %s\n",
                                eventProfile3[samples].freq,
                                percentage,
                                eventProfile3[samples].programCounter,
                                symbolValue,
                                eventProfile3[samples].programCounter - symbolValue,
                                symbolName);
            }
            else
            {
                break;
            }

        }
  
        fprintf(fpEventSampResults,"---------------------------------------------------\n\n\n");
        /* Print table header to output file */
        fprintf(fpEventSampResults,"Total Number of samples for Event 4= %u\n\n\n",
                                                    eventSampResults.event4_samples);
        fprintf(fpEventSampResults,"Hits    Percent PC Address Symbol Address Offset ClosestRoutine\n");
        fprintf(fpEventSampResults,"------- ------- ---------- -------------- ------ --------------\n");
        /* For every sample in profile print frequency, address and symbol name */
        for (samples = 0; samples < IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES; samples++)
        {
            percentage=((float)eventProfile4[samples].freq/
                              (float)eventSampResults.event4_samples)*100;
            if(eventProfile4[samples].freq > 0)
            {
                symFindByValue (sysSymTbl, eventProfile4[samples].programCounter, symbolName,
                                &symbolValue, &pointerType);
                fprintf (fpEventSampResults,"%7u %7.4f %10x %14x %6x %s\n",
                                eventProfile4[samples].freq,
                                percentage,
                                eventProfile4[samples].programCounter,
                                symbolValue,
                                eventProfile4[samples].programCounter - symbolValue,
                                symbolName);
            }
            else
            {
                break;
            }
        }

    /* Close pointer to file */   
    fclose(fpEventSampResults); 
    } /* if - else fopen */

    /* For Linux, assign values to global variables to be used during file output */
    #elif defined (__linux)
    for (samples = 0; samples < IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES; samples++)
    {
        eventSampProfile1[samples] = eventProfile1[samples];
        eventSampProfile2[samples] = eventProfile2[samples];
        eventSampProfile3[samples] = eventProfile3[samples];
        eventSampProfile4[samples] = eventProfile4[samples];
    }
    #endif
    
    return IX_PERFPROF_ACC_STATUS_SUCCESS;
}

PUBLIC void
ixPerfProfAccXscalePmuResultsGet(IxPerfProfAccXscalePmuResults *results)
{
    UINT32 i;
    IxPerfProfAccXscalePmuEvtCnt pClkCount;
    IxPerfProfAccXscalePmuEvtCnt pEventCount[
        IX_PERFPROF_ACC_XSCALE_PMU_MAX_EVENTS];
    IxFeatureCtrlDeviceId deviceType = IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X;

    deviceType = ixFeatureCtrlDeviceRead ();
    if(IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X == deviceType)
    {
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixPerfProfAcc - This PPU component is not supported\n",
            0, 0, 0, 0, 0, 0);
        return;
    }

    /*error check the parameter*/
    if (NULL == results)
    {
	/* report the error */ 
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixPerfProfAccXscalePmuResultsGet - results is invalid\n",
            0, 0, 0, 0, 0, 0);
        return;
    }

    /*initialize counter and overflow variables*/
    pClkCount.lower32BitsEventCount = 0;
    pClkCount.upper32BitsEventCount = 0;
    for(i=0; i<IX_PERFPROF_ACC_XSCALE_PMU_MAX_EVENTS; i++)
    {
        pEventCount[i].lower32BitsEventCount = 0;
        pEventCount[i].upper32BitsEventCount = 0;
    }

    /*set numberEvents to be the max so that all counters are obtained*/
    numberEvents = IX_PERFPROF_ACC_XSCALE_PMU_MAX_EVENTS;

    /*get the clk count and overflow*/
    ixPerfProfAccXscalePmuClkCntGet (&pClkCount);
    ixPerfProfAccXscalePmuEvtCntGet (pEventCount);

    /*assign values from ixPerfProfAccXscalePmuClkCntGet to *results struct*/
    results->clk_value = pClkCount.lower32BitsEventCount;
    results->clk_samples = pClkCount.upper32BitsEventCount;

    /*assign values from ixPerfProfAccXscalePmuEvtCntGet to *results struct*/
    results->event1_value = pEventCount[0].lower32BitsEventCount;
    results->event1_samples = pEventCount[0].upper32BitsEventCount;
    results->event2_value = pEventCount[1].lower32BitsEventCount;
    results->event2_samples = pEventCount[1].upper32BitsEventCount;
    results->event3_value = pEventCount[2].lower32BitsEventCount;
    results->event3_samples = pEventCount[2].upper32BitsEventCount;
    results->event4_value = pEventCount[3].lower32BitsEventCount;
    results->event4_samples= pEventCount[3].upper32BitsEventCount;
}

#ifdef __linux
int
ixPerfProfAccXscalePmuTimeSampCreateProcFile (char *buf, char **start, off_t offset,
                                      int count, int *eof, void *data)
{
   UINT32 samples=0;
   UINT32 symbolAddress=0;
   UINT32 addressOffset=0;
   int len = 0;
   char symbolName[IX_PERFPROF_ACC_XSCALE_PMU_SYMBOL_SIZE];
   char symbolModule[IX_PERFPROF_ACC_XSCALE_PMU_SYMBOL_SIZE];
   IxPerfProfAccXscalePmuEvtCnt clkCount;

   /*get the final clock count and number of samples taken*/
   ixPerfProfAccXscalePmuClkCntGet (&clkCount);
   len += sprintf(buf+len, "Total Samples Recorded = %u\n\n\n",clkCount.upper32BitsEventCount);
   len += sprintf(buf+len, "Hits    PC Address Symbol Address Offset Routine\n");
   len += sprintf(buf+len, "------- ---------- -------------- ------ -------------------\n");

    /* Print all samples with frequencies greater than zero to proc file */
   for (samples = 0; samples < IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES; samples++)
   {
       if(timeSampProfile[samples].freq > 0)
       {
          ixPerfProfAccXscalePmuSymbolGet(timeSampProfile[samples].programCounter, 
                                      symbolName, &symbolAddress, symbolModule);
          addressOffset = timeSampProfile[samples].programCounter - symbolAddress;
          len += sprintf (buf+len, "%7u %10x %14x %6x %s [Module - %s]\n",
                                     timeSampProfile[samples].freq,
                                     timeSampProfile[samples].programCounter,
                                     symbolAddress,
                                     addressOffset,
                                     symbolName,
                                     symbolModule);
       }
       else
       {
          break;
       }
   }

   /* Set eond of file to indicate that there will be nothing else to be printed to this file */
   *eof = IX_PERFPROF_ACC_XSCALE_PMU_EOF;
   return len;
}


int
ixPerfProfAccXscalePmuEventSampCreateProcFile (char *buf, char **start, off_t offset,
                                      int count, int *eof, void *data)
{
   UINT32 samples;
   int len = 0;
   UINT32 symbolAddress=0;
   UINT32 addressOffset=0;
   char symbolName[IX_PERFPROF_ACC_XSCALE_PMU_SYMBOL_SIZE];
   char symbolModule[IX_PERFPROF_ACC_XSCALE_PMU_SYMBOL_SIZE];
   IxPerfProfAccXscalePmuResults eventSampResults;
   
   ixPerfProfAccXscalePmuResultsGet(&eventSampResults);

  /* Write results for individual events to proc file */
   len += sprintf(buf+len, "Total Number of Samples for Event1 = %u\n\n\n",
                                               eventSampResults.event1_samples);
   len += sprintf(buf+len, "Hits    PC Address Symbol Address Offset Routine\n");
   len += sprintf(buf+len, "------- ---------- -------------- ------ -------------------\n");

    /* Print all samples with frequencies greater than zero to proc file */
   for (samples = 0; samples < IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES; samples++)
   {
       if(eventSampProfile1[samples].freq > 0)
       {
          ixPerfProfAccXscalePmuSymbolGet(eventSampProfile1[samples].programCounter, 
                                      symbolName, &symbolAddress, symbolModule);
          addressOffset = eventSampProfile1[samples].programCounter - symbolAddress;
          len += sprintf (buf+len, "%7u %10x %14x %6x %s [Module - %s]\n",
                                     eventSampProfile1[samples].freq,
                                     eventSampProfile1[samples].programCounter,
                                     symbolAddress,
                                     addressOffset,
                                     symbolName,
                                     symbolModule);
                    
       }
       else
       {
          break;
       }
   }
   len += sprintf(buf+len, "-----------------------------------------------------\n\n\n");

   len += sprintf(buf+len, "Total Number of Samples for Event2 = %u\n\n\n",
                                               eventSampResults.event2_samples);
   len += sprintf(buf+len, "Hits    PC Address Symbol Address Offset Routine\n");
   len += sprintf(buf+len, "------- ---------- -------------- ------ -------------------\n"); 
   for (samples = 0; samples < IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES; samples++)
   {
       if(eventSampProfile2[samples].freq > 0)
       {
          ixPerfProfAccXscalePmuSymbolGet(eventSampProfile2[samples].programCounter, 
                                      symbolName, &symbolAddress, symbolModule);
          addressOffset = eventSampProfile2[samples].programCounter - symbolAddress;
          len += sprintf (buf+len, "%7u %10x %14x %6x %s [Module - %s]\n",
                                     eventSampProfile2[samples].freq,
                                     eventSampProfile2[samples].programCounter,
                                     symbolAddress,
                                     addressOffset,
                                     symbolName,
                                     symbolModule);
                    
       }
       else
       {
          break;
       }
   }

   len += sprintf(buf+len, "-----------------------------------------------------\n\n\n");

   len += sprintf(buf+len, "Total Number of Samples for Event3 = %u\n\n\n",
                                               eventSampResults.event3_samples);
   len += sprintf(buf+len, "Hits    PC Address Symbol Address Offset Routine\n");
   len += sprintf(buf+len, "------- ---------- -------------- ------ -------------------\n");

   for (samples = 0; samples < IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES; samples++)
   {
       
       if(eventSampProfile3[samples].freq > 0)
       {
          ixPerfProfAccXscalePmuSymbolGet(eventSampProfile3[samples].programCounter, 
                                      symbolName, &symbolAddress, symbolModule);
          addressOffset = eventSampProfile3[samples].programCounter - symbolAddress;
          len += sprintf (buf+len, "%7u %10x %14x %6x %s [Module - %s]\n",
                                     eventSampProfile3[samples].freq,
                                     eventSampProfile3[samples].programCounter,
                                     symbolAddress,
                                     addressOffset,
                                     symbolName,
                                     symbolModule);
                    
       }
       else
       {
          break;
       }
   }
   len += sprintf(buf+len, "-----------------------------------------------------\n\n\n");

   len += sprintf(buf+len, "Total Number of Samples for Event4 = %u\n\n\n",
                                               eventSampResults.event4_samples);
   len += sprintf(buf+len, "Hits    PC Address Symbol Address Offset Routine\n");
   len += sprintf(buf+len, "------- ---------- -------------- ------ -------------------\n");

   for (samples = 0; samples < IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES; samples++)
   {      
       if(eventSampProfile4[samples].freq > 0)
       {
          ixPerfProfAccXscalePmuSymbolGet(eventSampProfile4[samples].programCounter, 
                                      symbolName, &symbolAddress, symbolModule);
          addressOffset = eventSampProfile4[samples].programCounter - symbolAddress;
          len += sprintf (buf+len, "%7u %10x %14x %6x %s [Module - %s]\n",
                                     eventSampProfile4[samples].freq,
                                     eventSampProfile4[samples].programCounter,
                                     symbolAddress,
                                     addressOffset,
                                     symbolName,
                                     symbolModule);
                    
       }
       else
       {
          break;
       }
   }

   len += sprintf(buf+len, "-----------------------------------------------------\n\n\n");
       
   /* Set end of file to indicate that there will be nothing else to be printed to this file */
   *eof = IX_PERFPROF_ACC_XSCALE_PMU_EOF;
   return len;
}

void
ixPerfProfAccXscalePmuSymbolGet(UINT32 pcAddress, 
                                char *symbol, 
                                UINT32 *symbolAddress, 
                                char *module)
{
    UINT32 symbolFound = 0;
    UINT32 count=0;
    UINT32 j=0;
    struct module *mod;
    struct module_symbol *msym;

    /* While symbol is not found and count has not reached the accuracy required, 
     * Search for symbol. */
    while((symbolFound!=IX_PERFPROF_ACC_XSCALE_PMU_SYMBOL_FOUND)&&
          (count<IX_PERFPROF_ACC_XSCALE_PMU_SYMBOL_ACCURACY))
    {
        /* If symbol is not found, search in every module. */
        for (mod = &__this_module; mod != NULL; mod = mod->next) 
        {
            if (!MOD_CAN_QUERY(mod))
            continue;
       
            /* Go through every symbol in module */               
            for (j = 0, msym = mod->syms; j < mod->nsyms; ++j, ++msym)
            {
                /* If PC address matches the symbol address, return symbol name */
                if(msym->value == pcAddress)
                {
                    *symbolAddress = msym->value;
                    strcpy(symbol, msym->name);
                    /* Get module name */
                    if(mod->name[0]==0)
                    {
                        strcpy(module, "kernel");
                    }
                    else
                    {
                        strcpy(module, mod->name);
                    }
                    symbolFound = IX_PERFPROF_ACC_XSCALE_PMU_SYMBOL_FOUND;
                    break;
                } 
            }
       
            if ((IX_PERFPROF_ACC_XSCALE_PMU_SYMBOL_FOUND == symbolFound)||
                (0 == pcAddress))
            {
                break;
            }
       }

       if (0 == pcAddress) 
       {
           break;
       }

       pcAddress--;
       count++;
       
    }/* End while */
    
    if (IX_PERFPROF_ACC_XSCALE_PMU_SYMBOL_FOUND != symbolFound)
    {
         strcpy(symbol,"No lower symbol found.");
         *symbolAddress = 0;
    }
        
}
#endif

