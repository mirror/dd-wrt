/**
 * @file IxPerfProfAccXscalePmu_p.h
 *
 * @date April-09-2003
 *
 * @brief Private header file for the XScale PMU portion of the IxPerfProfAcc
 * software component
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

#ifndef IXPERFPROFACCXSCALEPMU_P_H
#define IXPERFPROFACCXSCALEPMU_P_H

#include "IxOsal.h"
#include "IxPerfProfAcc.h"

/*
 * #defines for function return types, etc.
 */
#define INLINE __inline__

#ifdef __vxworks

/* 
 * The define below allow functions to be inlined in 
 * vxWorks but removed from Linux to avoid inline warnings
 */
#define VXWORKS_INLINE __inline__ 

#elif defined(__linux)  

#define VXWORKS_INLINE
/* End of file value for proc file writing in linux */
#define IX_PERFPROF_ACC_XSCALE_PMU_EOF 1

/* Define to indicate symbol have been found in Linux */
#define IX_PERFPROF_ACC_XSCALE_PMU_SYMBOL_FOUND 1

#endif 


/*Pmu interrupt select bit in the XScale interrupt select register for IRQ*/
#define IX_PERFPROF_ACC_XSCALE_PMU_SELECT_IRQ_BIT 0xfffbffff 

/*Interrupt enable bit for XScale in the interrupt controller register*/
#define IX_PERFPROF_ACC_XSCALE_PMU_XSCALE_INTERRUPT_ENABLE_BIT 0x40000

/*Interrupt Controller Register Address */
#define IX_PERFPROF_ACC_XSCALE_PMU_INTR_ENABLE_REG_ADD	IX_OSAL_IXP400_ICMR
#define IX_PERFPROF_ACC_XSCALE_PMU_INTR_SELECT_REG_ADD	IX_OSAL_IXP400_ICLR

/*PMU Overflow Flag and Interrupt Enable Flag */
#define IX_PERFPROF_ACC_XSCALE_PMU_OFLOW_FLAG_CCNT   0x0001 /*clock counter*/
#define IX_PERFPROF_ACC_XSCALE_PMU_OFLOW_FLAG_PMN0   0x0002 /*event counter 1*/
#define IX_PERFPROF_ACC_XSCALE_PMU_OFLOW_FLAG_PMN1   0x0004 /*event counter 2*/
#define IX_PERFPROF_ACC_XSCALE_PMU_OFLOW_FLAG_PMN2   0x0008 /*event counter 3*/
#define IX_PERFPROF_ACC_XSCALE_PMU_OFLOW_FLAG_PMN3   0x0010 /*event counter 4*/
#define IX_PERFPROF_ACC_XSCALE_PMU_OFLOW_FLAG_PMN    0x001e /*all evt counters*/
#define IX_PERFPROF_ACC_XSCALE_PMU_OFLOW_FLAG_ALL    0x001f /*clock counter and 
                                                             *all event counters
                                                             */
#define IX_PERFPROF_ACC_XSCALE_PMU_INTR_DISABLE_ALL  0x0000 /*clock counter and 
                                                             *all event counters
                                                             */                                                             
                                                             
/*BSP PMU Interrupt Bit */
#define IX_PERFPROF_ACC_XSCALE_PMU_BSP_INTR_BIT	0x12                                                     

/* value to NOT select any events */
#define IX_PERFPROF_ACC_XSCALE_PMU_EVT_SELECT_NONE 0xff

/*default value of all counters*/      
#define IX_PERFPROF_ACC_XSCALE_PMU_CLK_CTR      0xffffffff   
#define IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR1     0xffffff00
#define IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR2     0xffff0000                                                          
#define IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR3     0xff000000                                                                                                               
     
/*value to shift for each event counter to write into event select register*/     
#define IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR2_BITS_TO_SHIFT   0x8
#define IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR3_BITS_TO_SHIFT   0x10
#define IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR4_BITS_TO_SHIFT   0x18

/*Bits to set in PMNC register*/
#define IX_PERFPROF_ACC_XSCALE_PMU_PMNC_DISABLE 0x0   /*to enable all counters*/
#define IX_PERFPROF_ACC_XSCALE_PMU_PMNC_ENABLE 0x1   /*to enable all counters*/
#define IX_PERFPROF_ACC_XSCALE_PMU_PMNC_RESET_EVT_CTR 0x2    /*to reset the 
                                                              *event counters
                                                              */
#define IX_PERFPROF_ACC_XSCALE_PMU_PMNC_RESET_CLK_CTR 0x4    /*to reset the 
                                                              *clock counter
                                                              */
#define IX_PERFPROF_ACC_XSCALE_PMU_PMNC_DIVIDER 0x8    /*to enable the clock 
                                                        *count divider
                                                        */

/*id for each event and clock counter*/
#define IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR1_ID 0x0
#define IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR2_ID 0x1
#define IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR3_ID 0x2
#define IX_PERFPROF_ACC_XSCALE_PMU_EVT_CTR4_ID 0x3
#define IX_PERFPROF_ACC_XSCALE_PMU_CLK_CTR_ID  0x4

/*maximim number of events that can be monitored at a time*/
#define IX_PERFPROF_ACC_XSCALE_PMU_MAX_EVENTS	0x4

/*maximum number of event plus clock counters*/
#define IX_PERFPROF_ACC_XSCALE_PMU_MAX_COUNTERS    0x5

/**
 * ixPerfProfAccXscalePmuIntrConnect(void)
 *
 * To present interrupts as type IRQ to XScale and to call intConnect() 
 * which connects the handler routine to the specified interrupt vector.
 *
 */
void
ixPerfProfAccXscalePmuIntrConnect(void);

/**
 * ixPerfProfAccXscalePmuBspIntrEnable(void)
 *
 * This sets the bsp PMU interrupt by calling the intEnable() function.  
 * It then enables the XScale PMU interrupt in the interrupt enable register by 
 * setting bit 18 to value of 1.
 *
 */
void
ixPerfProfAccXscalePmuBspIntrEnable(void);

/**
 * ixPerfProfAccXscalePmuEventSelect(
      IxPerfProfAccXscalePmuEvent pmuEvent1,
      IxPerfProfAccXscalePmuEvent pmuEvent2,       
      IxPerfProfAccXscalePmuEvent pmuEvent3,          
      IxPerfProfAccXscalePmuEvent pmuEvent4)
 *
 * This assigns the appropriate value of the event for each event counter and 
 * writes it into the PMU Event Select Register.
 * Parameters are:
 *
 * IxPerfProfAccXscalePmuEvent pmuEvent1 - event for counter 1
 * IxPerfProfAccXscalePmuEvent pmuEvent2 - event for counter 2      
 * IxPerfProfAccXscalePmuEvent pmuEvent3 - event for counter 3         
 * IxPerfProfAccXscalePmuEvent pmuEvent4 - event for counter 4
 *
 */
void
ixPerfProfAccXscalePmuEventSelect(
        IxPerfProfAccXscalePmuEvent pmuEvent1,
        IxPerfProfAccXscalePmuEvent pmuEvent2,       
        IxPerfProfAccXscalePmuEvent pmuEvent3,          
        IxPerfProfAccXscalePmuEvent pmuEvent4);
        
/**
 * ixPerfProfAccXscalePmuIntrEnable(BOOL clkCtr);
 *
 * This assigns the appropriate value to enable interrupts for each counter.
 * This value is then written to the Interrupt Enable Register. Parameters are:
 *
 * BOOL cklCtr - TRUE if the clock counter is ON which is in the case 
 *               of event counting and time-based sampling                            
 */
void
ixPerfProfAccXscalePmuIntrEnable(BOOL clkCtr);
 
/**
 * ixPerfProfAccXscalePmuCtrEnableReset (
        BOOL clkCntDiv, 
        BOOL enableCtrs,
        BOOL resetClkCtr,
        BOOL resetEvtCtr)
 *
 * This assigns the appropriate value to either enable/disable or reset 
 * event and clock counters.  All counters are enabled/disabled simulanaeously.  
 * The event counters have to be reset simultanaeously, however the clock 
 * counter can be reset separately.  The clock divider can also be enabled.  
 * This value is then written to the Performance Monitor Control Register.  
 * Parameters are:
 *
 * BOOL clkCntDiv - TRUE if clock counter divider; FALSE if not
 * BOOL enableCtrs  - TRUE if need to enable all counters
 * BOOL resetClkCtr - TRUE if need to reset clock counter
 * BOOL resetEvtCtr - TRUE if need to reset event counters
 *
 */ 
void
ixPerfProfAccXscalePmuCtrEnableReset (
    BOOL clkCntDiv, 
    BOOL enableCtrs,
    BOOL resetClkCtr,
    BOOL resetEvtCtr);  

/**
 * ixPerfProfAccXscalePmuEvtCtrInit (void)
 *
 * This initializes the event counters that are monitoring events.
 *
 */ 
IxPerfProfAccStatus
ixPerfProfAccXscalePmuEvtCtrInit (void);

/**
 * ixPerfProfAccXscalePmuClkCtrInit (void)
 *
 * brief This initializes the clock counter.
 */ 
void
ixPerfProfAccXscalePmuClkCtrInit (void);


/**
 * ixPerfProfAccXscalePmuIntrDisable (void) 
 *
 * This will go through the process of disabling the PMU interrupt by :
 * - disabling the PMNC which disables all the counters
 * - disabling the appropriate counter interrupts which were enabled by 
 *   ixPerfProfAccXscalePmuIntrEnable
 * - disabling the XScale PMU interrupt
 * - disabling the BSP PMU interrupt
 *
 */ 
void
ixPerfProfAccXscalePmuIntrDisable (void);

/**
 * ixPerfProfAccXscalePmuClkCntGet (IxPerfProfAccXscalePmuEvtCnt *clkCount) 
 *
 * This will get the current clock count and store it in a pointer.  The 
 * resulting clock over flow is also obtained.  Parameters are:
 *
 * IxPerfProfAccXscalePmuEvtCnt *clkCount
 *
 */
void
ixPerfProfAccXscalePmuClkCntGet (IxPerfProfAccXscalePmuEvtCnt *clkCount);

/**
 * ixPerfProfAccXscalePmuEvtCntGet (        
     IxPerfProfAccXscalePmuEvtCnt *eventCount)
 *
 * This will get the current event count for each of the counters that 
 * were activated by the client and store them in pointers. Parameters are:
 *
 * IxPerfProfAccXscalePmuEvtCnt *eventCount - pointer to the struct that 
 * contains the upper and lower 32 values of a counter.  Before calling this 
 * function, the struct will be declared as an array of 4 to represent the 
 * values of each of the event counters.                                             
 *
 */
IxPerfProfAccStatus
ixPerfProfAccXscalePmuEvtCntGet (        
        IxPerfProfAccXscalePmuEvtCnt *eventCount);

/**
 * ixPerfProfAccXscalePmuProfileConstruct(
     UINT32 pcAddr,
     IxPerfProfAccXscalePmuSamplePcProfile *profile)
 *
 * This will construct the profiling results summary by:
 *  -Creating and initializing the results array
 *  -For the number of samples taken, capture the different values of PC and the
 *   frequencies of their occurrence.
 * Parameters are:
 *
 * UINT32 pcAddr - address of the PC
 * IxPerfProfAccXscalePmuSamplePcProfile *profile - pointer to results profile
 *
 */        
void
ixPerfProfAccXscalePmuProfileConstruct (
    UINT32 pcAddr,
    IxPerfProfAccXscalePmuSamplePcProfile *profile);

/**
 * ixPerfProfAccXscalePmuOverFlowRead(void)
 *
 * This inline function will use assembly code to read the overflow flag
 * status register and return it
 *
 */
INLINE UINT32
_ixPerfProfAccXscalePmuOverFlowRead(void);

/**
 * ixPerfProfAccXscalePmuOverFlowWrite(UINT32 value)
 *
 * This inline function will use assembly code to write the overflow flag
 * status register.  Parameters are:
 *
 * UINT32 value - value to write to register
 *
 * @return - none 
 */
INLINE void 
_ixPerfProfAccXscalePmuOverFlowWrite(UINT32 value);

/**
 * ixPerfProfAccXscalePmuCcntRead(void)
 *
 * This inline function will use assembly code to read the clock counter
 * register and return it
 *
 */
INLINE UINT32
_ixPerfProfAccXscalePmuCcntRead(void);

/**
 * ixPerfProfAccXscalePmuCcntWrite(void)
 *
 * This inline function will use assembly code to write the clock counter
 * register.  Parameters are:
 *
 * UINT32 value - value to write to register
 *
 */
INLINE void 
_ixPerfProfAccXscalePmuCcntWrite(UINT32 value);

/**
 * ixPerfProfAccXscalePmuIntenRead(void)
 *
 * This inline function will use assembly code to read the PMU interrupt 
 * enable register and return it
 *
 */
INLINE unsigned
_ixPerfProfAccXscalePmuIntenRead(void);

/**
 * ixPerfProfAccXscalePmuIntenWrite(UINT32 value)
 *
 * This inline function will use assembly code to write the PMU interrupt 
 * enable register.  Parameters are:
 *
 * UINT32 value - value to write to register
 *
 */
INLINE void
_ixPerfProfAccXscalePmuIntenWrite(UINT32 value);

/**
 * ixPerfProfAccXscalePmuPmncRead(void)
 *
 * This inline function will use assembly code to read the PMNC register and 
 * return it and return it
 *
 */
INLINE UINT32
_ixPerfProfAccXscalePmuPmncRead(void);

/**
 * ixPerfProfAccXscalePmuPmncWrite(void)
 *
 * This inline function will use assembly code to write the PMNC register
 * Parameters are:
 *
 * UINT32 value - value to write to register
 *
 */
INLINE void
_ixPerfProfAccXscalePmuPmncWrite(UINT32 value);

/**
 * ixPerfProfAccXscalePmuEvtSelectWrite(UINT32 value)
 *
 * This inline function will use assembly code to write to the event select 
 * register.  Parameters are:
 *
 * UINT32 value - The specific value to write into the register
 *
 */
INLINE void 
_ixPerfProfAccXscalePmuEvtSelectWrite(UINT32 value);

/**
 * ixPerfProfAccXscalePmuEvtSelectRead(void)
 *
 * This inline function will use assembly code to read the event select
 * register.  
 * 
 * Return
 * UINT32 value - The specific value read from the register
 *
 */
INLINE UINT32
_ixPerfProfAccXscalePmuEvtSelectRead(void);

/**
 * ixPerfProfAccXscalePmuPmnRead(UINT32 num, BOOL *check)
 *
 * This inline function will use assembly code to read event registers and 
 * return it.  Parameters are:
 *
 * UINT32 num - The specific event counter register to read from
 * BOOL *check - pointer to check for validity of num passed in; value is set
 * to FALSE if num passed in is invalid, and TRUE if num passed in is valid. The
 * pointer should be initialized to TRUE before being passed in         
 *
 */
INLINE UINT32
_ixPerfProfAccXscalePmuPmnRead(UINT32 num, BOOL *check);

/**
 * ixPerfProfAccXscalePmuPmnWrite(UINT32 num, UINT32 value, BOOL *check)
 *
 * This inline function will use assembly code to write to the  event 
 * registers.  Parameters are:
 *
 * UINT32 num - The specific event counter register to write to
 * UINT32 value - The value to be written to the register
 * BOOL *check - pointer to check for validity of num passed in; value is set
 * to FALSE if num passed in is invalid, and TRUE if num passed in is valid. The
 * pointer should be initialized to TRUE before being passed in 
 */
INLINE void 
_ixPerfProfAccXscalePmuPmnWrite(UINT32 num, UINT32 value, BOOL *check);

/**
 * _ixPerfProfAccXscalePmuProfilePcStore ()
 *
 * This inline function will store the interrupted Pc into array selected 
 * by eventCounterId. 
 * 
 *
 * UINT32 eventCounterId - which event counter is selected. 
 * UINT32 idx - Index of array which PC is to be stored. 
 * UINT32 pc - Value of PC to be stored. 
 */
INLINE void
_ixPerfProfAccXscalePmuProfilePcStore (
	UINT32 eventCounterId, 
	UINT32 idx, 
	UINT32 pc);


/**
 * _ixPerfProfAccXscalePmuEventHandler ()
 *
 * This inline function handle interrupts generated by PMU 
 * events (excluding clock interrupts).  
 * 
 * UINT32 eventOflowSelect - overflow bit for the event 
 * UINT32 eventCounterId - which event counter is selected. 
 * UINT32 pcAddr - Value of PC to be stored (used only for profiling) 
 */
VXWORKS_INLINE void 
_ixPerfProfAccXscalePmuEventHandler (
		UINT32 eventOflowSelect, 
		UINT32 eventCounterId, 
		UINT32 pcAddr); 

/**
 * ixPerfProfAccXscalePmuIntrHandler (void);
 * 
 * Interrupt Handler for Xscale PMU events
 *
 */
void
ixPerfProfAccXscalePmuIntrHandler (void);

/**
 * ixPerfProfAccXscalePmuProfileSort 
 *            (IxPerfProfAccXscalePmuSamplePcProfile *profileArray, UINT32 total)
 * This function sorts the profile into the order of descending frequencies. 
 *
 * IxPerfProfAccXscalePmuSamplePcProfile *profileArray - Profile that need sorting.
 * UINT32 total - Total elements required in profile.
 */
void
ixPerfProfAccXscalePmuProfileSort 
             (IxPerfProfAccXscalePmuSamplePcProfile *profileArray, UINT32 total);

#ifdef __linux
void
ixPerfProfAccXscalePmuSymbolGet(UINT32 pcAddress, 
                                char *symbol, 
                                UINT32 *symbolAddress, 
                                char *module);

#endif
#endif /*ifdef IXPERFPROFACCXSCALEPMU_P_H*/

