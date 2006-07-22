/**
 * IxPerfProfAccXcycle_p.h 
 *
 * Date April 10 2003
 *
 * Header file for the Xcycle module of PerfProfAcc component
 *
 *
 * Design Notes:
 *
 * @par 
 * IXP400 SW Release version  2.1
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright (c) 2002-2005 Intel Corporation All Rights Reserved.
 * 
 * @par
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel Corporation
 * or its suppliers or licensors.  Title to the Material remains with
 * Intel Corporation or its suppliers and licensors.
 * 
 * @par
 * The Material is protected by worldwide copyright and trade secret laws
 * and treaty provisions. No part of the Material may be used, copied,
 * reproduced, modified, published, uploaded, posted, transmitted,
 * distributed, or disclosed in any way except in accordance with the
 * applicable license agreement .
 * 
 * @par
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel, except in accordance with the
 * applicable license agreement.
 * 
 * @par
 * Unless otherwise agreed by Intel in writing, you may not remove or
 * alter this notice or any other notice embedded in Materials by Intel
 * or Intel's suppliers or licensors in any way.
 * 
 * @par
 * For further details, please see the file README.TXT distributed with
 * this software.
 * 
 * @par
 * -- End Intel Copyright Notice --
 */


#ifndef IXPERFPROFACCXCYCLE_P_H
#define IXPERFPROFACCXCYCLE_P_H

/*
 * Intermodule dependancies
 */

#include "IxOsal.h"
#include "IxPerfProfAcc.h"

#define INLINE __inline__

/* 
 * Break a one second duration into smaller time slices. It is 
 * not possible to break out of measurement within a time slice. 
 * The program check for time passes after each time slice. 
 * This allow the program to break out of a loop that takes too 
 * long to complete. The maximum time allowed for each measurement 
 * before it is terminated is defined in 
 * IX_PERFPROF_ACC_XCYCLE_MAX_TIME_IN_ONE_MEASUREMENT. 
 */ 
#define IX_PERFPROF_ACC_XCYCLE_TIME_SLICES_PER_SEC 	128 

/* 
 * Time in seconds allowed in one measurement before the 
 * measurement is cut short
 */ 
#define IX_PERFPROF_ACC_XCYCLE_MAX_TIME_IN_ONE_MEASUREMENT 5	

/* System timer in Hertz. Multiply clock speed in MHz by 1000000 */ 
#define IX_PERFPROF_ACC_XCYCLE_TIMER_FREQ ixOsalTimestampResolutionGet() 

/* 
 * Number of loops to find loops per slice per sec. We need a arbitary 
 * but reasonable number to perform enough looping to find 
 * how many loops it takes for a duration of 1 sec 
 * Experiment shows any value between 0x10000 to 0x100000 is acceptable. 
 */ 
#define IX_PERFPROF_ACC_XCYCLE_SEED_FOR_BASELINE (0x10000) 

/* 
 * Constants to set priority for different OSes 
 */ 
#define IX_PERFPROF_ACC_XCYCLE_LINUXAPP_PRIORITY_HIGHEST (-20) 

#define IX_PERFPROF_ACC_XCYCLE_LINUXAPP_PRIORITY_LOWEST	 (19)

#define IX_PERFPROF_ACC_XCYCLE_VXWORKS_PRIORITY_HIGHEST (0) 

#define IX_PERFPROF_ACC_XCYCLE_VXWORKS_PRIORITY_LOWEST	(255)

/*
 * Internal data structure 
 */
typedef struct  
{
	UINT32 xcycleCountStart;
	UINT32 xcycleCountStop;
	UINT32 totalTimeSlices; 
}IxPerfProfAccXcycleMeasuredResults;

/* 
 * Private functions 
 */

/**
 * Param "UINT32 numLoop [in] numLoop" Number of iterations to run
 *
 * Global Data  : 
 *                        - None.
 *                        
 *
 * Perform a small loop of numLoop times. 
 * This function is used to consume a fixed amount of XScale cycle 
 * without needing to access any memory. 
 *
 * return 
 *      - None
 *              
 * Reentrant    : no
 * ISR Callable : no
 *
 */
INLINE void
ixPerfProfAccXcycleLoopIter(UINT32 numLoop);

/**
 * Compute number of numLoop where ixPerfProfAccXcycleLoopIter(numLoop)
 * takes 1/IX_PERFPROF_ACC_XCYCLE_TIME_SLICES_PER_SEC seconds to complete
 * 
 * return 
 *      - Number of loops required for ixPerfProfAccXcycleLoopIter to run 
 *			for 1/IX_PERFPROF_ACC_XCYCLE_TIME_SLICES_PER_SEC seconds
 
 *              
 * Reentrant    : no
 * ISR Callable : no
 *
 */
UINT32 ixPerfProfAccXcycleComputeLoopsPerSlice(void);

/**
 * 		
 * Run numMeasurementsRequested measurements. Collect time needed for 
 * each execution. 
 * Stop measurement if stopMeasurement is TRUE.   
 *
 * param UINT32 [in] numMeasurementsRequested
 *
 * Global Data  : 
 *                        - None.
 *                        
 * return 
 *      - None
 *              
 * Reentrant    : no
 * ISR Callable : no
 *
 */
void 
ixPerfProfAccXcycleNumPeriodRun(UINT32 numMeasurementsRequested) ;

/**	
 *
 * Read the APB timer
 * 
 * param None
 *
 * Global Data  : 
 *                        - None.
 * return 
 *      - Value of APB timer
 *              
 * Reentrant    : no
 * ISR Callable : no
 *
 */
INLINE UINT32
ixPerfProfAccXcycleApbTimerGet(void); 

#endif /* ifndef IXPERFPROFACCXCYCLE_P_H */
