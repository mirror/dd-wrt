/**
 * @file IxPerfProfAccCodelet.c
 *
 * @date June-18-2003
 *
 * @brief This file contains the implementation of the PerfProf Access Codelet.
 *
 * Descriptions of the functions used in this codelet is contained in
 * IxPerfProfAccCodelet.h
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

/*
 * Put the user defined include files required.
 */
#include "IxPerfProfAccCodelet.h"
#include "IxPerfProfAcc.h"
#include "IxOsal.h"

#define     SAMPLES_REQUIRED        5     /* Number of samples required by the user to be
                                             printed onto the screen. Used for event sampling */
#define     DELAY                   20000 /* Delay used between the start and stop calls */
#ifdef __vxworks
#define     XCYCLE_DELAY            1000  /* Delay between the xcycle start and stop calls */ 
#define     XCYCLE_CONTINUOUS_MODE  0     /* When xcycle start is called, this parameter 
                                              indicates continuous counting */
#endif

/*
 * Declaration of private function 
 */
IxPerfProfAccStatus 
ixPerfProfAccCodeletSelection
     (IxPerfProfAccCodeletMode mode, 
                      UINT32 param1, 
                      UINT32 param2,
                      UINT32 param3, 
                      UINT32 param4, 
                      UINT32 param5, 
                      UINT32 param6,
                      UINT32 param7, 
                      UINT32 param8, 
                      UINT32 param9);

void
ixPerfProfAccCodeletHelp(void);

IxPerfProfAccStatus 
ixPerfProfAccCodeletAll(void);

IxPerfProfAccStatus
ixPerfProfAccCodeletBusPmu (IxPerfProfAccCodeletMode mode,
                            UINT32 param1,
                            UINT32 param2,
                            UINT32 param3,
                            UINT32 param4,
                            UINT32 param5,
                            UINT32 param6,
                            UINT32 param7);

void
ixPerfProfAccCodeletPMSR(void);

IxPerfProfAccStatus
ixPerfProfAccCodeletXscalePmuEventCount(UINT32 param1,
                                        UINT32 param2,
                                        UINT32 param3,
                                        UINT32 param4,
                                        UINT32 param5,
                                        UINT32 param6);

IxPerfProfAccStatus
ixPerfProfAccCodeletXscalePmuTimeSamp(UINT32 param1, UINT32 param2);

IxPerfProfAccStatus
ixPerfProfAccCodeletXscalePmuEventSamp(UINT32 param1,
                                UINT32 param2,
                                UINT32 param3,
                                UINT32 param4,
                                UINT32 param5,
                                UINT32 param6,
                                UINT32 param7,
                                UINT32 param8,
                                UINT32 param9);

IxPerfProfAccStatus
ixPerfProfAccCodeletXcycle(UINT32 param1);

void
ixPerfProfAccCodeletProfileSort (IxPerfProfAccXscalePmuSamplePcProfile *profileArray, UINT32 total);

/*
 * Global variables. 
 */
IxPerfProfAccXscalePmuSamplePcProfile timeProfile[IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES];
IxPerfProfAccXscalePmuSamplePcProfile eventProfile1[IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES];
IxPerfProfAccXscalePmuSamplePcProfile eventProfile2[IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES];
IxPerfProfAccXscalePmuSamplePcProfile eventProfile3[IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES];
IxPerfProfAccXscalePmuSamplePcProfile eventProfile4[IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES];

/*
 * Function definition: ixPerfProfAccCodeletBusPmu()
 * Function executes all bus pmu related functionalities. 
 */
IxPerfProfAccStatus
ixPerfProfAccCodeletBusPmu (IxPerfProfAccCodeletMode mode,
                            UINT32 param1,
                            UINT32 param2,
                            UINT32 param3,
                            UINT32 param4,
                            UINT32 param5,
                            UINT32 param6,
                            UINT32 param7) 
{
    IxPerfProfAccStatus busPmuStatus = IX_PERFPROF_ACC_STATUS_SUCCESS;
    IxPerfProfAccBusPmuResults BusPmuResults;       /* Stores the Bus PMU results */
    UINT32 pecCounter = 0;                          /* Counts number of PECs in Bus PMU */
 
    switch(mode)
    {
        /* Start executing BUS PMU North mode event counting */
        case IX_PERFPROF_ACC_CODELET_MODE_BUS_PMU_NORTH_MODE:
            printf("\n*************************************\n");
            printf("Executing BUS PMU NORTH BUS profiling\n");
            printf("*************************************\n");

            busPmuStatus = ixPerfProfAccBusPmuStart(IX_PERFPROF_ACC_BUS_PMU_MODE_NORTH, 
                                                     param1, 
                                                     param2,
                                                     param3, 
                                                     param4, 
                                                     param5, 
                                                     param6, 
                                                     param7);
            break;

        /* Start executing BUS PMU South mode event counting */
        case IX_PERFPROF_ACC_CODELET_MODE_BUS_PMU_SOUTH_MODE:
            printf("\n*************************************\n");
            printf("Executing BUS PMU SOUTH BUS profiling\n");
            printf("*************************************\n");

            busPmuStatus = ixPerfProfAccBusPmuStart(IX_PERFPROF_ACC_BUS_PMU_MODE_SOUTH, 
                                                     param1, 
                                                     param2,
                                                     param3, 
                                                     param4, 
                                                     param5, 
                                                     param6, 
                                                     param7);
            break;

        /* Start executing BUS PMU Sdram mode event counting */
        case IX_PERFPROF_ACC_CODELET_MODE_BUS_PMU_SDRAM_MODE:
            printf("\n*************************************\n");
            printf("Executing BUS PMU SDRAM BUS profiling\n");
            printf("*************************************\n");

            busPmuStatus = ixPerfProfAccBusPmuStart(IX_PERFPROF_ACC_BUS_PMU_MODE_SDRAM, 
                                                     param1, 
                                                     param2,
                                                     param3, 
                                                     param4, 
                                                     param5, 
                                                     param6, 
                                                     param7);
            break;

        default:
            break;
    }/* end switch */

    switch(busPmuStatus)
        {
         case IX_PERFPROF_ACC_STATUS_ANOTHER_UTIL_IN_PROGRESS:
             printf("Another utility currently running\n");
             return IX_PERFPROF_ACC_STATUS_FAIL;

         case IX_PERFPROF_ACC_STATUS_BUS_PMU_MODE_ERROR:
             printf("BUS PMU: Invalid mode selected\n");
             return IX_PERFPROF_ACC_STATUS_FAIL;

         case IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC1_ERROR:
             printf("BUS PMU: Invalid selection for PEC1\n");
             return IX_PERFPROF_ACC_STATUS_FAIL;

         case IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC2_ERROR:
             printf("BUS PMU: Invalid selection for PEC2\n");
             return IX_PERFPROF_ACC_STATUS_FAIL;

         case IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC3_ERROR:
             printf("BUS PMU: Invalid selection for PEC3\n");
             return IX_PERFPROF_ACC_STATUS_FAIL;

         case IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC4_ERROR:
             printf("BUS PMU: Invalid selection for PEC4\n");
             return IX_PERFPROF_ACC_STATUS_FAIL;

         case IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC5_ERROR:
             printf("BUS PMU: Invalid selection for PEC5\n");
             return IX_PERFPROF_ACC_STATUS_FAIL;

         case IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC6_ERROR:
             printf("BUS PMU: Invalid selection for PEC6\n");
             return IX_PERFPROF_ACC_STATUS_FAIL;

         case IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC7_ERROR:
             printf("BUS PMU: Invalid selection for PEC7\n");
             return IX_PERFPROF_ACC_STATUS_FAIL;

         case IX_PERFPROF_ACC_STATUS_SUCCESS:
             printf("Bus PMU started\n");
             break;

         default:
             printf("Unknown error\n");
             return IX_PERFPROF_ACC_STATUS_FAIL;
    }/* end switch */

    /* Run counting for this period of time */
    ixOsalSleep(DELAY);

    /* Stop counting */
    busPmuStatus = ixPerfProfAccBusPmuStop();

    /* Check status of returned value and if anything other than a success,
     * print error message and exit.
     */
    switch(busPmuStatus)
    {
         case IX_PERFPROF_ACC_STATUS_BUS_PMU_START_NOT_CALLED:
             printf("Need to call start first\n");
             return IX_PERFPROF_ACC_STATUS_FAIL;

         case IX_PERFPROF_ACC_STATUS_FAIL:
             printf("Fail to stop BUS Pmu\n");
             return IX_PERFPROF_ACC_STATUS_FAIL;

         case IX_PERFPROF_ACC_STATUS_SUCCESS:
             printf("Bus PMU stopped successfully\n");
             break;

         default:
             printf("Unknown error while stopping\n");
             return IX_PERFPROF_ACC_STATUS_FAIL;
    }/* end switch */

    /* Get results of the counters and print to the output. */
    ixPerfProfAccBusPmuResultsGet (&BusPmuResults);
    for (pecCounter = 0; IX_PERFPROF_ACC_BUS_PMU_MAX_PECS > pecCounter; pecCounter++)
    {
        printf("PEC %d upper 32 bit value = %u\n", pecCounter+1,
                                  BusPmuResults.statsToGetUpper32Bit[pecCounter]);
        printf("PEC %d lower 27 bit value = %u\n", pecCounter+1,
                                  BusPmuResults.statsToGetLower27Bit[pecCounter]);
    }/* end for */

    return busPmuStatus;
}/* end function ixPerfProfAccCodeletBusPmu */

/* Function definition: IxPerfProfAccCodeletPMSR()
 * Function calls PMSR Get and displays results onto the screen.
 */
void 
ixPerfProfAccCodeletPMSR()
{
    UINT32 pmsrValue = 0;          /* PMSR value read from the PMSR register. */
    UINT32 pssValue = 0;           /* Value corresponding to the pss portion of the total
                                      PMSR value */

    printf ("\n**************\n");
    printf ("GET PMSR VALUE\n");
    printf ("**************\n");

    ixPerfProfAccBusPmuPMSRGet (&pmsrValue);
    pssValue = pmsrValue & PSS_MASK;
    if(EXPANSION_BUS == pssValue)
    {
        printf("Expansion Bus was the previous slave on the arbiter\n");
        printf("accessing the AHBS\n");
    } /* end else if */

    else if (SDRAM_CONTROLLER == pssValue)
    {
        printf("Sdram controller was the previous slave on the arbiter\n");
        printf("accessing the AHBS\n");
    } /* end else if */

    else if (PCI == pssValue)
    {
        printf("PCI was the previous slave on the arbiter\n");
        printf("accessing the AHBS\n");
    } /* end else if */

    else if (QUEUE_MANAGER == pssValue)
    {
        printf("Queue manager was the previous slave on the arbiter\n");
        printf("accessing the AHBS\n");
    } /* end else if */

    else if (AHB_APB_BRIDGE == pssValue)
    {
        printf("Expansion Bus was the previous slave on the arbiter\n");
        printf("accessing the AHBS\n");
    } /* end else if */

    else
    {
        printf("Unknown value obtained. Last slave undetermined.\n");
    } /* end else */

} /*end of function ixPerfProfAccCodeletPMSR */
    
/* Function definition ixPerfProfAccCodeletXscalePmuEventCount()
 * Run xscale pmu event counting functionalities.
 */
IxPerfProfAccStatus
ixPerfProfAccCodeletXscalePmuEventCount(
                              UINT32 param1,
                              UINT32 param2,
                              UINT32 param3,
                              UINT32 param4,
                              UINT32 param5,
                              UINT32 param6)
{
    IxPerfProfAccXscalePmuResults eventCountStopResults; /* Stores the event count results */
    IxPerfProfAccStatus eventCountStatus = IX_PERFPROF_ACC_STATUS_SUCCESS;

    printf("\n***********************************\n");
    printf("Executing Xscale PMU Event Counting\n");
    printf("***********************************\n");
    eventCountStatus = ixPerfProfAccXscalePmuEventCountStart(param1, param2, param3, param4,
                                                   param5, param6);

    /* Check returned value. If anything but success, print error message to the
     * screen and exit.
     */
    switch (eventCountStatus)
    {
       case IX_PERFPROF_ACC_STATUS_ANOTHER_UTIL_IN_PROGRESS:
           printf("Another utility currently running\n");
           return IX_PERFPROF_ACC_STATUS_FAIL;

       case IX_PERFPROF_ACC_STATUS_XSCALE_PMU_NUM_INVALID:
           printf("Invalid number of events chosen\n");
           return IX_PERFPROF_ACC_STATUS_FAIL;

       case IX_PERFPROF_ACC_STATUS_XSCALE_PMU_EVENT_INVALID:
           printf("Invalid event chosen\n");
           return IX_PERFPROF_ACC_STATUS_FAIL;

       case IX_PERFPROF_ACC_STATUS_SUCCESS:
           printf("Event count started\n");
           break;

       default:
           printf("Started event counting successfully\n");
           return IX_PERFPROF_ACC_STATUS_FAIL;
    } /* end switch */

    /* Wait for fixed period of time for counting to continue. */
    ixOsalSleep(DELAY);

    /* Stop event counting */
    eventCountStatus = ixPerfProfAccXscalePmuEventCountStop (&eventCountStopResults);

    /* Check if any error is returned. If not success print message and exit */
    switch(eventCountStatus)
    {
        case IX_PERFPROF_ACC_STATUS_XSCALE_PMU_START_NOT_CALLED:
            printf("Event Count Start not called\n");
            return IX_PERFPROF_ACC_STATUS_FAIL;

        case IX_PERFPROF_ACC_STATUS_SUCCESS:
            printf("Event counter successfully stopped\n");
            break;

        default:
            printf("Unknown error while stopping\n");
            return IX_PERFPROF_ACC_STATUS_FAIL;
    } /* end switch */

    /* Printf results to screen */
    printf("Event 1 lower 32 bit = %u\n", eventCountStopResults.event1_value);
    printf("Event 1 upper 32 bit = %u\n", eventCountStopResults.event1_samples);
    printf("Event 2 lower 32 bit = %u\n", eventCountStopResults.event2_value);
    printf("Event 2 upper 32 bit = %u\n", eventCountStopResults.event2_samples);
    printf("Event 3 lower 32 bit = %u\n", eventCountStopResults.event3_value);
    printf("Event 3 upper 32 bit = %u\n", eventCountStopResults.event3_samples);
    printf("Event 4 lower 32 bit = %u\n", eventCountStopResults.event4_value);
    printf("Event 4 upper 32 bit = %u\n", eventCountStopResults.event4_samples);
    printf("Clock counter value = %u\n", eventCountStopResults.clk_value);

    return eventCountStatus;

}  /* end ixPerfProfAccXscalePmuEventCount */

/* Function definition: ixPerfProfAccCodeletXscalePmuTimeSamp()
 * Run event sampling functionalities
 */
IxPerfProfAccStatus
ixPerfProfAccCodeletXscalePmuTimeSamp(UINT32 param1, UINT32 param2)
{
    IxPerfProfAccStatus timeSampStatus = IX_PERFPROF_ACC_STATUS_SUCCESS;
    IxPerfProfAccXscalePmuEvtCnt clkCount;
    UINT32 timeSampCount = 0;

    printf("\n**********************************\n");
    printf("Executing Xscale PMU Time Sampling\n");
    printf("**********************************\n");
    timeSampStatus = ixPerfProfAccXscalePmuTimeSampStart(param1, param2);
    switch(timeSampStatus)
    {
        case IX_PERFPROF_ACC_STATUS_ANOTHER_UTIL_IN_PROGRESS:
            printf("Another utility currently running\n");
            return IX_PERFPROF_ACC_STATUS_FAIL;

        case IX_PERFPROF_ACC_STATUS_FAIL:
            printf("Sampling rate bigger than maximum counter value\n");
            return IX_PERFPROF_ACC_STATUS_FAIL;
 
        case IX_PERFPROF_ACC_STATUS_SUCCESS:
            printf("Successfully started time sampling\n");
            break;

        default:
            printf("Unknown error\n");
            return IX_PERFPROF_ACC_STATUS_FAIL;
    }/* end switch */

    /* Delay while allowing sampling to continue. */
    ixOsalSleep(DELAY);

    /* Stop sampling get results and print to screen. */
    timeSampStatus = ixPerfProfAccXscalePmuTimeSampStop(&clkCount, timeProfile);
    switch(timeSampStatus)
    {
        case IX_PERFPROF_ACC_STATUS_XSCALE_PMU_START_NOT_CALLED:
            printf("Start function not called\n");
            return IX_PERFPROF_ACC_STATUS_FAIL;

        case IX_PERFPROF_ACC_STATUS_SUCCESS:
            printf("Successfully stopped time sampling\n");
            break;

        default:
            printf("Unknown error\n");
            return IX_PERFPROF_ACC_STATUS_FAIL; 
    }/* end switch */

    printf("Printing top 5 samples\n");
    printf("**********************\n");
    for(timeSampCount=0; timeSampCount<SAMPLES_REQUIRED; timeSampCount++)
    {
        printf("Program counter value for sample %d = 0x%x\n", timeSampCount+1,
                                         timeProfile[timeSampCount].programCounter);
        printf("Frequency of occurance of sample %d = %u\n", timeSampCount+1,
                                         timeProfile[timeSampCount].freq);
    }/* end for */

    return timeSampStatus;

}/* end function ixPerfProfAccCodeletXscalePmuTimeSamp */

/* Function definition: ixPerfProfAccCodeletXscalePmuEventSamp()
 * Run event sampling functionalities
 */
IxPerfProfAccStatus
ixPerfProfAccCodeletXscalePmuEventSamp(UINT32 param1,
                                UINT32 param2,
                                UINT32 param3,
                                UINT32 param4,
                                UINT32 param5,
                                UINT32 param6,
                                UINT32 param7,
                                UINT32 param8,
                                UINT32 param9)
{
    IxPerfProfAccStatus eventSampStatus = IX_PERFPROF_ACC_STATUS_SUCCESS;
    IxPerfProfAccXscalePmuResults eventSampResults;      /* Stores results from GetResults
                                                            during event sampling */
    UINT32 eventSampCount = 0;     /* Increments counters  for event counting */
    UINT32 frequency = 0;          /* Frequency of samples */
    UINT32 evt1Samp=0;             /* Stores Event 1 sample */
    UINT32 evt2Samp=0;             /* Stores Event 2 sample */
    UINT32 evt3Samp=0;             /* Stores Event 3 sample */
    UINT32 evt4Samp=0;             /* Stores Event 4 sample */
    UINT32 numPc = 0;              /* Number of unique pc addresses */

    printf("\n***********************************\n");
    printf("Executing Xscale PMU Event Sampling\n");
    printf("***********************************\n");

    eventSampStatus = ixPerfProfAccXscalePmuEventSampStart(param1, 
                                                           param2, 
                                                           param3, 
                                                           param4,
                                                           param5, 
                                                           param6, 
                                                           param7, 
                                                           param8, 
                                                           param9);

    switch(eventSampStatus)
    {
        case IX_PERFPROF_ACC_STATUS_ANOTHER_UTIL_IN_PROGRESS:
            printf("Another utility currently in progress\n");
            return IX_PERFPROF_ACC_STATUS_FAIL;

        case IX_PERFPROF_ACC_STATUS_XSCALE_PMU_NUM_INVALID:
            printf("Invalid number of events chosen\n");
            return IX_PERFPROF_ACC_STATUS_FAIL;

        case IX_PERFPROF_ACC_STATUS_XSCALE_PMU_EVENT_INVALID:
            printf("Invalid event chosen\n");
            return IX_PERFPROF_ACC_STATUS_FAIL;

        case IX_PERFPROF_ACC_STATUS_FAIL:
            printf("Sampling rate bigger than counter size\n");
            return IX_PERFPROF_ACC_STATUS_FAIL;

        case IX_PERFPROF_ACC_STATUS_SUCCESS:
            printf("Event Sampling Started\n");
            break;

        default:
            printf("Unknown error\n");
            return IX_PERFPROF_ACC_STATUS_FAIL;
    }/* end switch */

    /* Delay to enable event sampling to continue. */
    ixOsalSleep(DELAY);

    /* Stop event sampling get results and print to screen */
    eventSampStatus = ixPerfProfAccXscalePmuEventSampStop(eventProfile1, eventProfile2,
                                                 eventProfile3, eventProfile4);
    switch(eventSampStatus)
    {
        case IX_PERFPROF_ACC_STATUS_XSCALE_PMU_START_NOT_CALLED:
            printf("Start function not called\n");
            return IX_PERFPROF_ACC_STATUS_FAIL;

        case IX_PERFPROF_ACC_STATUS_SUCCESS:
            printf("Event Sampling Stopped\n");
            break;

        default:
            printf("Unknown error while stopping\n");
            return IX_PERFPROF_ACC_STATUS_FAIL;
    }/* end switch */

    /* Get results */
    ixPerfProfAccXscalePmuResultsGet(&eventSampResults);
    /* Get number of samples for event 1 */
    evt1Samp = eventSampResults.event1_samples;
    numPc=0;
    frequency=0;
    /* For number of samples in event 1, get total frequency */
    while (frequency < evt1Samp)
    {
        frequency += eventProfile1[numPc].freq;
        numPc++;
    }/* end while */

    if (frequency != evt1Samp)
    {
        printf("\nALERT SAMPLE numbers does not match !");
    }/* end if */

    /* Print values of defined number of samples to the screen. */
    printf("List of top 5 samples for every counter in descending frequency\n");
    printf("***************************************************************\n\n");

    for (eventSampCount=0; eventSampCount < SAMPLES_REQUIRED; eventSampCount++)
    {
        printf("Event 1 sample %d program counter = 0x%x\n", eventSampCount+1,
                                          eventProfile1[eventSampCount].programCounter);
        printf("Event 1 sample %d frequency = %u\n", eventSampCount+1,
                                         eventProfile1[eventSampCount].freq);
    }/* end for */
    printf("*****************************************************\n\n");

    /* Get number of samples collected for event 2 */
    evt2Samp = eventSampResults.event2_samples;
    numPc=0;
    frequency=0;
    /* Get total frequency of all samples */
    while (frequency < evt2Samp)
    {
        frequency += eventProfile2[numPc].freq;
        numPc++;
    } /* end while */

    if (frequency != evt2Samp)
    {
        printf("\nALERT SAMPLE numbers does not match !");
    } /* end if */

    printf("List of samples for every counter of EventProfile2 in descending frequency\n");
    printf("**************************************************************************\n\n");

    for (eventSampCount=0; eventSampCount < SAMPLES_REQUIRED; eventSampCount++)
    {
        printf("Event 2 sample %d program counter = 0x%x\n", eventSampCount+1,
                                          eventProfile2[eventSampCount].programCounter);
        printf("Event 2 sample %d frequency = %u\n", eventSampCount+1,
                                          eventProfile2[eventSampCount].freq);
    } /* end for */
    printf("*****************************************************\n\n");

    /* Get total number of samples taken for event 3 */
    evt3Samp = eventSampResults.event3_samples;
    numPc=0;
    frequency=0;
    /* Get total of the frequencies */
    while (frequency < evt3Samp)
    {
        frequency += eventProfile3[numPc].freq;
        numPc++;
    } /* end while */

    if (frequency != evt3Samp)
    {
        printf("\nALERT SAMPLE numbers does not match !");
    } /* end if */

    /* Print defined number of results onto the screen */
    printf("List of samples for every counter of EventProfile3 in descending frequency\n");
    printf("**************************************************************************\n\n");

    for (eventSampCount=0; eventSampCount < SAMPLES_REQUIRED; eventSampCount++)
    {
        printf("Event 3 sample %d program counter = 0x%x\n", eventSampCount+1,
                                          eventProfile3[eventSampCount].programCounter);
        printf("Event 3 sample %d frequency = %u\n", eventSampCount+1,
                                          eventProfile3[eventSampCount].freq);
    } /* end for */
    printf("*****************************************************\n\n");

    /* Get total number of samples collected for event 4 */
    evt4Samp = eventSampResults.event4_samples;
    numPc=0;
    frequency=0;
    /* Get total frequency of samples */
    while (frequency < evt4Samp)
    {
        frequency += eventProfile4[numPc].freq;
        numPc++;
    } /* end while */

    if (frequency != evt4Samp)
    {
        printf("\nALERT SAMPLE numbers does not match !");
    } /* end if */

    printf("List of samples for every counter of EventProfile4 in descending frequency\n");
    printf("**************************************************************************\n\n");

    /* Print results of defined number of samples for event 4 */
    for (eventSampCount=0; eventSampCount < SAMPLES_REQUIRED; eventSampCount++)
    {
        printf("Event 4 sample %d program counter = 0x%x\n", eventSampCount+1,
                                          eventProfile4[eventSampCount].programCounter);
        printf("Event 4 sample %d frequency = %u\n", eventSampCount+1,
                                          eventProfile4[eventSampCount].freq);
    } /* end for */
    printf("*****************************************************\n\n");

    return eventSampStatus; 

} /* end function ixPerfProfAccCodeletXscalePmuEventSamp*/

#ifdef __vxworks
/* Function definition: 
 * Run xcycle functionalities
 */
IxPerfProfAccStatus
ixPerfProfAccCodeletXcycle(UINT32 param1)
{
    IxPerfProfAccStatus xcycleStatus = IX_PERFPROF_ACC_STATUS_SUCCESS;
    IxPerfProfAccXcycleResults xcycleResults;     /* Stores the Idle Cycle results */
    UINT32 numBaselineCycle = 0;   /* Stores the baseline from Xcycle calculations */

        printf("\n*****************************\n");
        printf("Executing Xcycle Measurement\n");
        printf("****************************\n");
        /* Get Baseline */
        if (XCYCLE_CONTINUOUS_MODE == param1)
        {
            printf("Cannot select 0 number of runs\n");
            return IX_PERFPROF_ACC_STATUS_FAIL;
        }

        xcycleStatus = ixPerfProfAccXcycleBaselineRun(&numBaselineCycle);
        switch (xcycleStatus)
        {
            case IX_PERFPROF_ACC_STATUS_ANOTHER_UTIL_IN_PROGRESS:
                printf("Another utility currently running\n");
                return IX_PERFPROF_ACC_STATUS_FAIL;

            case IX_PERFPROF_ACC_STATUS_XCYCLE_MEASUREMENT_IN_PROGRESS:
                printf("XCycle measurement in progress\n");
                return IX_PERFPROF_ACC_STATUS_FAIL;

            case IX_PERFPROF_ACC_STATUS_XCYCLE_PRIORITY_SET_FAIL:
                printf("Failed to set priority\n");
                return IX_PERFPROF_ACC_STATUS_FAIL;

            case IX_PERFPROF_ACC_STATUS_XCYCLE_PRIORITY_RESTORE_FAIL:
                printf("Failed to restore priority\n");
                return IX_PERFPROF_ACC_STATUS_FAIL;

            case IX_PERFPROF_ACC_STATUS_SUCCESS:
                printf("Baseline successfully obtained\n");
                break;

            default:
                printf("Unknown error baseline not obtained\n");
                return IX_PERFPROF_ACC_STATUS_FAIL;
        } /* end switch */

        /* Start calculation of idle cycles for a given number of runs*/
        xcycleStatus = ixPerfProfAccXcycleStart(param1);
        switch (xcycleStatus)
                {
            case IX_PERFPROF_ACC_STATUS_XCYCLE_NO_BASELINE:
                printf("Cannot start...no baseline set\n");
                return IX_PERFPROF_ACC_STATUS_FAIL;
 
            case IX_PERFPROF_ACC_STATUS_XCYCLE_MEASUREMENT_REQUEST_OUT_OF_RANGE:
                printf("Number of measurements requested exceeds limit\n");
                printf("Maximum limit defined by\n");
                printf("IX_PERFPROF_ACC_XCYCLE_MAX_NUM_OF_MEASUREMENTS\n");
                return IX_PERFPROF_ACC_STATUS_FAIL;
 
            case IX_PERFPROF_ACC_STATUS_XCYCLE_MEASUREMENT_IN_PROGRESS:
                printf("Xcycle measurement already in progress\n");
                return IX_PERFPROF_ACC_STATUS_FAIL;
 
            case IX_PERFPROF_ACC_STATUS_ANOTHER_UTIL_IN_PROGRESS:
                printf("Another utility is currently running. Xcycle not started\n");
                return IX_PERFPROF_ACC_STATUS_FAIL;

            case IX_PERFPROF_ACC_STATUS_XCYCLE_THREAD_CREATE_FAIL:
                printf("Failed to create thread\n");
                return IX_PERFPROF_ACC_STATUS_FAIL;
 
            case IX_PERFPROF_ACC_STATUS_SUCCESS:
                printf("Xcycle Measurement Started\n");
                break;

            default:
                printf("Unknown error\n");
                return IX_PERFPROF_ACC_STATUS_FAIL;
       } /* end switch */

       /* Delay to allow idle cycle measurement to complete before getting results. */
       while(ixPerfProfAccXcycleInProgress())
       {
           ixOsalSleep(XCYCLE_DELAY);
       }

       /* Get results. If anything except success is returned, print error message and
        * exit.
        */
       xcycleStatus = ixPerfProfAccXcycleResultsGet (&xcycleResults);
       switch(xcycleStatus)
       {
            case IX_PERFPROF_ACC_STATUS_XCYCLE_NO_BASELINE:
                printf("Cannot start...no baseline set\n");
                return IX_PERFPROF_ACC_STATUS_FAIL;

            case IX_PERFPROF_ACC_STATUS_XCYCLE_MEASUREMENT_IN_PROGRESS:
                printf("Xcycle measurement already in progress\n");
                return IX_PERFPROF_ACC_STATUS_FAIL;

            case IX_PERFPROF_ACC_STATUS_FAIL:
                printf("No measurements have been performed\n");
                return IX_PERFPROF_ACC_STATUS_FAIL;

            case IX_PERFPROF_ACC_STATUS_SUCCESS:
                printf("Results obtained\n");
                break;

            default:
                printf("Unknown error\n");
                return IX_PERFPROF_ACC_STATUS_FAIL;
       }/* end switch */

       /* Print calculated values to screen. */
       printf("Maximum percentage of idle cycles = %f\n", xcycleResults.maxIdlePercentage);
       printf("Minimum percentage of idle cycles = %f\n", xcycleResults.minIdlePercentage);
       printf("Average percentage of idle cycles = %f\n", xcycleResults.aveIdlePercentage);
       printf("Total number of measurements      = %u\n", xcycleResults.totalMeasurements);

       return xcycleStatus;

}/* end function ixPerfProfAccCodeletXcycle */
#endif


/*
 * Function definition: ixPerfProfAccCodeletSelection()
 * Function accepts user selection and runs individual functionality based on the input.
 * This function is also called by demo all and runs all the functionalities. It returns a 
 * fail if any of the user selection is wrong or a fail is returned by the function it calls.
 * In either case an error message is printed to the screen and function is exited. When all the 
 * necessary functionalities are executed correctly, this function returns a success.
 */
IxPerfProfAccStatus 
ixPerfProfAccCodeletSelection(
      IxPerfProfAccCodeletMode mode, 
      UINT32 param1, 
      UINT32 param2,
      UINT32 param3, 
      UINT32 param4, 
      UINT32 param5, 
      UINT32 param6,
      UINT32 param7, 
      UINT32 param8, 
      UINT32 param9)
{
    IxPerfProfAccStatus status = IX_PERFPROF_ACC_STATUS_SUCCESS;

    /* If help mode selected, call the help function. */
    if (IX_PERFPROF_ACC_CODELET_MODE_HELP == mode)
    {
        ixPerfProfAccCodeletHelp();
        return IX_PERFPROF_ACC_STATUS_SUCCESS; 
    }/* end if */

    /* If BUS PMU modes selected, print the relevant messages on the screen and start counting
     * of events.
     */
    else if ((IX_PERFPROF_ACC_CODELET_MODE_BUS_PMU_NORTH_MODE == mode)||
             (IX_PERFPROF_ACC_CODELET_MODE_BUS_PMU_SOUTH_MODE == mode)||
             (IX_PERFPROF_ACC_CODELET_MODE_BUS_PMU_SDRAM_MODE == mode)) 
    {   
        status = ixPerfProfAccCodeletBusPmu(mode, 
                                          param1, 
                                          param2, 
                                          param3, 
                                          param4, 
                                          param5, 
                                          param6, 
                                          param7);
        if (IX_PERFPROF_ACC_STATUS_SUCCESS != status)
        {
            return status;
        }
    }

    /* If bus pmu pmsr is selected, get the pmsr value and perform bit-wise 
     * to obtain required information manipulation 
     */
    else if (IX_PERFPROF_ACC_CODELET_MODE_BUS_PMU_PMSR_GET == mode)
    {
        ixPerfProfAccCodeletPMSR();
    } /* end else if */ 

    
    /* If Xscale PMU event counting is selected, start the counting, check for errors,
     * wait for a preset period while counting is taking place, stop and read values.
     */
    else if (IX_PERFPROF_ACC_CODELET_MODE_XSCALE_PMU_EVENT_COUNTING == mode)
    {
        status = ixPerfProfAccCodeletXscalePmuEventCount(param1, 
                                                         param2, 
                                                         param3,
                                                         param4, 
                                                         param5, 
                                                         param6);
        if (IX_PERFPROF_ACC_STATUS_SUCCESS != status)
        {
            return status;
        }
    } /*end else if*/
 
    /* If Xscale PMU time sampling is selected, start the sampling, check for errors,
     * wait for a preset period while counting is taking place, stop and read values.
     */
    else if (IX_PERFPROF_ACC_CODELET_MODE_XSCALE_PMU_TIME_SAMPLING == mode)
    {
        status = ixPerfProfAccCodeletXscalePmuTimeSamp(param1, param2);
        if (IX_PERFPROF_ACC_STATUS_SUCCESS != status)
        {
            return status;
        }
    } /*end else if*/
        

    /* If Xscale PMU Event Sampling is selected, start the sampling, check for errors,
     * wait for a preset period while counting is taking place, stop and read values.
     */
    else if (IX_PERFPROF_ACC_CODELET_MODE_XSCALE_PMU_EVENT_SAMPLING == mode)
    {
        status = ixPerfProfAccCodeletXscalePmuEventSamp(param1, 
                                                        param2, 
                                                        param3, 
                                                        param4,
                                                        param5, 
                                                        param6, 
                                                        param7, 
                                                        param8, 
                                                        param9);
        if (IX_PERFPROF_ACC_STATUS_SUCCESS != status)
        {
            return status;
        }
    }

    #ifdef __vxworks
    /* If Xcycle mode is selected, start the baseline calculation, 
     * start the idle cycle calculation, check for errors,
     * wait for a preset period while counting is taking place, stop and read values.
     */
    else if (IX_PERFPROF_ACC_CODELET_MODE_XCYCLE == mode)
    {
        status = ixPerfProfAccCodeletXcycle(param1);
        if (IX_PERFPROF_ACC_STATUS_SUCCESS != status)
        {
            return status;
        }
    }
    #endif
  
    else
    {
       printf("Invalid mode selected. Please try again\n");
       return IX_PERFPROF_ACC_STATUS_FAIL;
    }/* end else */
 
    return IX_PERFPROF_ACC_STATUS_SUCCESS; 
}/* end function ixPerfProfAccCodeletSelection() */

/**
 *Function definition: ixPerfProfAccCodeletAll()
 *Called to demonstrated all the functionalities available. Function calls all the 
 *individual functions and exits if there is an error.
 */

IxPerfProfAccStatus
ixPerfProfAccCodeletAll(void)
{
   IxPerfProfAccStatus status = IX_PERFPROF_ACC_STATUS_SUCCESS;
   BOOL clkCntDiv = FALSE;
   UINT32 numEvents = 4;
   UINT32 samplingRate = 0xffff;
   #ifdef __vxworks
   UINT32 numRun = 20;
   #endif
   
   printf("***********************\n");
   printf("Executing Demo All Mode\n");
   printf("***********************\n\n");
   /* Demo north mode of bus pmu functionality */
   status = ixPerfProfAccCodeletSelection
                            (IX_PERFPROF_ACC_CODELET_MODE_BUS_PMU_NORTH_MODE, 
                             IX_PERFPROF_ACC_BUS_PMU_PEC1_NORTH_BUS_IDLE_SELECT,
                             IX_PERFPROF_ACC_BUS_PMU_PEC2_NORTH_BUS_WRITE_SELECT,
                             IX_PERFPROF_ACC_BUS_PMU_PEC3_NORTH_BUS_READ_SELECT,
                             IX_PERFPROF_ACC_BUS_PMU_PEC4_NORTH_NPEA_REQ_SELECT,
                             IX_PERFPROF_ACC_BUS_PMU_PEC5_NORTH_NPEB_REQ_SELECT,
                             IX_PERFPROF_ACC_BUS_PMU_PEC6_NORTH_NPEC_REQ_SELECT,
                             IX_PERFPROF_ACC_BUS_PMU_PEC7_CYCLE_COUNT_SELECT,
                             0,
                             0);
                             
   if (IX_PERFPROF_ACC_STATUS_SUCCESS != status)
   {
       return status;
   } /* end if */

   else
   {
       printf("Events selected:\n");
       printf("IX_PERFPROF_ACC_BUS_PMU_PEC1_NORTH_BUS_IDLE_SELECT\n");
       printf("IX_PERFPROF_ACC_BUS_PMU_PEC2_NORTH_BUS_WRITE_SELECT\n");
       printf("IX_PERFPROF_ACC_BUS_PMU_PEC3_NORTH_BUS_READ_SELECT\n");
       printf("IX_PERFPROF_ACC_BUS_PMU_PEC4_NORTH_NPEA_REQ_SELECT\n");
       printf("IX_PERFPROF_ACC_BUS_PMU_PEC5_NORTH_NPEB_REQ_SELECT\n");
       printf("IX_PERFPROF_ACC_BUS_PMU_PEC6_NORTH_NPEC_REQ_SELECT\n");
       printf("IX_PERFPROF_ACC_BUS_PMU_PEC7_CYCLE_COUNT_SELECT\n"); 
   }

   /* Demonstrate event counting mode of xscale pmu functionality */
   status = ixPerfProfAccCodeletSelection
                            (IX_PERFPROF_ACC_CODELET_MODE_XSCALE_PMU_EVENT_COUNTING,
                             clkCntDiv,
                             numEvents,
                             IX_PERFPROF_ACC_XSCALE_PMU_EVENT_CACHE_MISS,
                             IX_PERFPROF_ACC_XSCALE_PMU_EVENT_BRANCH_EXEC,
                             IX_PERFPROF_ACC_XSCALE_PMU_EVENT_INST_EXEC,
                             IX_PERFPROF_ACC_XSCALE_PMU_EVENT_CACHE_INSTRUCTION,
                             0,
                             0,
                             0);
                              
   if (IX_PERFPROF_ACC_STATUS_SUCCESS != status)
   {
       return status;
   } /* end if */

   else
   {
       printf("Events selected:\n");
       printf("IX_PERFPROF_ACC_XSCALE_PMU_EVENT_CACHE_MISS\n");
       printf("IX_PERFPROF_ACC_XSCALE_PMU_EVENT_BRANCH_EXEC\n");
       printf("IX_PERFPROF_ACC_XSCALE_PMU_EVENT_INST_EXEC\n");
       printf("IX_PERFPROF_ACC_XSCALE_PMU_EVENT_CACHE_INSTRUCTION\n");
   }
   /* Demonstrate time sampling mode of xscale pmu functionality */
   status = ixPerfProfAccCodeletSelection
                            (IX_PERFPROF_ACC_CODELET_MODE_XSCALE_PMU_TIME_SAMPLING,
                             samplingRate,
                             clkCntDiv,
                             0,
                             0,
                             0,
                             0,
                             0,
                             0,
                             0);
                              
   if (IX_PERFPROF_ACC_STATUS_SUCCESS != status)
   {
       return status;
   } /* end if */
   
   /* Demonstrate event sampling of xscale pmu functionality */
   status = ixPerfProfAccCodeletSelection
                            (IX_PERFPROF_ACC_CODELET_MODE_XSCALE_PMU_EVENT_SAMPLING,
                             numEvents,
                             IX_PERFPROF_ACC_XSCALE_PMU_EVENT_ONCE,
                             samplingRate,
                             IX_PERFPROF_ACC_XSCALE_PMU_EVENT_INST_TLB_MISS,
                             samplingRate,
                             IX_PERFPROF_ACC_XSCALE_PMU_EVENT_CACHE_MISS,
                             samplingRate,
                             IX_PERFPROF_ACC_XSCALE_PMU_EVENT_BRANCH_EXEC,
                             samplingRate);
                             
   if (IX_PERFPROF_ACC_STATUS_SUCCESS != status)
   {
       return status;
   } /* end if */

   else
   {
       printf("Events selected:\n");
       printf("IX_PERFPROF_ACC_XSCALE_PMU_EVENT_ONCE\n");
       printf("IX_PERFPROF_ACC_XSCALE_PMU_EVENT_INST_TLB_MISS\n");
       printf("IX_PERFPROF_ACC_XSCALE_PMU_EVENT_CACHE_MISS\n");
       printf("IX_PERFPROF_ACC_XSCALE_PMU_EVENT_BRANCH_EXEC\n");
   }
   /* Demonstrate obtaining PMSR value */
   status = ixPerfProfAccCodeletSelection
                            (IX_PERFPROF_ACC_CODELET_MODE_BUS_PMU_PMSR_GET,
                             0,
                             0,
                             0,
                             0,
                             0,
                             0,
                             0,
                             0,
                             0);

   if (IX_PERFPROF_ACC_STATUS_SUCCESS != status)
   {
       return status;
   } /* end if */

   #ifdef __vxworks
   /* Demonstrate xcycle measurement functionality */
   status = ixPerfProfAccCodeletSelection
                            (IX_PERFPROF_ACC_CODELET_MODE_XCYCLE,
                             numRun,
                             0,
                             0,
                             0,
                             0,
                             0,
                             0,
                             0,
                             0);
                             
   if (IX_PERFPROF_ACC_STATUS_SUCCESS != status)
   {
       return status;
   } /* end if */
   #endif

   return IX_PERFPROF_ACC_STATUS_SUCCESS;

} /* end of function ixPerfProfAccCodeletAll */

/**
 *Function definition: ixPerfProfAccCodeletHelp()
 *Print "how to use" information on the screen.
 */
void
ixPerfProfAccCodeletHelp(void)
{
    printf("******************\n");
    printf("PerfProf Help Menu\n");
    printf("******************\n");

    printf("To use the codelet, call function ixPerfProfAccCodeletMain\n");
    printf("passing in one of the following as the mode:\n");
    printf("0 - for help\n");
    printf("1 - for demonstration of all profiling\n");
    printf("2 - for demonstration of Bus Pmu North Bus profiling\n");
    printf("3 - for demonstration of Bus Pmu South Bus profiling\n");
    printf("4 - for demonstration of Bus Pmu Sdram Bus profiling\n");
    printf("5 - for demonstration of XScale Pmu Event Sampling\n");
    printf("6 - for demonstration of XScale Pmu Time Sampling\n");
    printf("7 - for demonstration of XScale Pmu Event Counting\n");
    printf("8 - for demonstration of Xcycle measurement\n");

    printf("Users will be able to start the codelet by calling ixPerfProfAccCodeletMain\n");
    printf("and passing in up to eight parameters. The parameters are represented in the\n");
    printf("following order:\n");
    printf("\nHelp Mode\n");
    printf("Mode - Select IX_PERFPROF_ACC_CODELET_MODE_HELP\n");
    printf("Set the rest of the parameters to 0.\n");
    printf("\nAll functionalities mode.\n");
    printf("Mode - Select IX_PERFPROF_ACC_CODELET_MODE_ALL\n");
    printf("Set the rest of the parameters to 0.\n");
    printf("\nBus PMU north mode\n");
    printf("Mode - Select IX_PERFPROF_ACC_CODELET_MODE_BUS_PMU_NORTH_MODE\n");
    printf("param1 - Select proper PEC1 value from main header file.\n");
    printf("param2 - Select proper PEC2 value from main header file.\n");
    printf("param3 - Select proper PEC3 value from main header file.\n");
    printf("param4 - Select proper PEC4 value from main header file.\n");
    printf("param5 - Select proper PEC5 value from main header file.\n"); 
    printf("param6 - Select proper PEC6 value from main header file.\n");
    printf("param7 - Select proper PEC7 value from main header file.\n");
    printf("Set the rest of the parameters to 0.");
    printf("\nBus PMU south mode\n");
    printf("Mode - Select IX_PERFPROF_ACC_CODELET_MODE_BUS_PMU_SOUTH_MODE\n");
    printf("param1 - Select proper PEC1 value from main header file.\n");
    printf("param2 - Select proper PEC2 value from main header file.\n");
    printf("param3 - Select proper PEC3 value from main header file.\n");
    printf("param4 - Select proper PEC4 value from main header file.\n");
    printf("param5 - Select proper PEC5 value from main header file.\n"); 
    printf("param6 - Select proper PEC6 value from main header file.\n");
    printf("param7 - Select proper PEC7 value from main header file.\n");
    printf("Set the rest of the parameters to 0.");
    printf("\nBus PMU sdram mode\n");
    printf("Mode - Select IX_PERFPROF_ACC_CODELET_MODE_BUS_PMU_SDRAM_MODE\n");
    printf("param1 - Select proper PEC1 value from main header file.\n");
    printf("param2 - Select proper PEC2 value from main header file.\n");
    printf("param3 - Select proper PEC3 value from main header file.\n");
    printf("param4 - Select proper PEC4 value from main header file.\n");
    printf("param5 - Select proper PEC5 value from main header file.\n"); 
    printf("param6 - Select proper PEC6 value from main header file.\n");
    printf("param7 - Select proper PEC7 value from main header file.\n");
    printf("Set the rest of the parameters to 0.");
    printf("\nBus PMU PMSR Get\n");
    printf("Mode - Select IX_PERFPROF_ACC_CODELET_MODE_BUS_PMU_PMSR_GET\n");
    printf("Set the rest of the parameters to 0.\n");
    printf("\nXScale PMU Event Sampling\n");
    printf("Mode - Select IX_PERFPROF_ACC_CODELET_MODE_XSCALE_PMU_EVENT_SAMPLING\n");
    printf("param1 - Number of events\n");
    printf("param2 - Event 1\n");
    printf("param3 - Sampling rate of Event 1\n");
    printf("param4 - Event 2\n");
    printf("param5 - Sampling rate of Event 2\n");
    printf("param6 - Event 3\n");
    printf("param7 - Sampling rate of Event 3\n");
    printf("param8 - Event 4\n");
    printf("param9 - Sampling rate of Event 4\n");
    printf("\nXScale PMU Time Sampling\n");
    printf("Mode - Select IX_PERFPROF_ACC_CODELET_MODE_XSCALE_PMU_TIME_SAMPLING\n");
    printf("param1 - Sampling rate.\n");
    printf("param2 - Clock count divider.\n");
    printf("Set the rest of the parameters to 0.\n");
    printf("\nXScale PMU Event Counting \n");
    printf("Mode - Select IX_PERFPROF_ACC_CODELET_MODE_XSCALE_PMU_EVENT_COUNTING\n");
    printf("param1 - Clock count divider.\n");
    printf("param2 - Number of events.\n");
    printf("param3 - Event 1.\n");
    printf("param4 - Event 2.\n");
    printf("param5 - Event 3.\n");
    printf("param6 - Event 4.\n");
    printf("Set the rest of the parameters to 0.\n");
    #ifdef __vxworks
    printf("\nXcycle Measurement\n");
    printf("Select IX_PERFPROF_ACC_CODELET_MODE_XCYCLE\n");
    printf("param1 - Number of runs required.\n");
    printf("Set the rest of the parameters to 0.\n");
    #endif
    return;
} /* end of function ixPerfProfAccCodeletHelp */

/**
 *Function definition: ixPerfProfAccCodeletProfileSort()
 *Sort the event results array into descending order of frequency.
 */
void
ixPerfProfAccCodeletProfileSort (IxPerfProfAccXscalePmuSamplePcProfile *profileArray, UINT32 total)
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
} /* end ixPerfProfAccCodeletProfileSort */

/*
 * Function definition: ixPerfProfAccCodeletMain -
 * calls the help function if help mode is chosen. Else it calls
 *  the function that enables selection of individual modes and corresponding events.
 */
PUBLIC void
ixPerfProfAccCodeletMain(
     IxPerfProfAccCodeletMode mode,
     UINT32 param1,
     UINT32 param2,
     UINT32 param3,
     UINT32 param4,
     UINT32 param5,
     UINT32 param6,
     UINT32 param7,
     UINT32 param8,
     UINT32 param9)
{
    /* Initialise status */
    IxPerfProfAccStatus statusMain = IX_PERFPROF_ACC_STATUS_SUCCESS;

    /* Check if mode selected  is for demo all. If it is, call the demo all function */
    if (IX_PERFPROF_ACC_CODELET_MODE_ALL == mode)
    {
        statusMain = ixPerfProfAccCodeletAll();
        /* If function does not return a success print help and exit. */
        if (IX_PERFPROF_ACC_STATUS_SUCCESS!=statusMain)
        {
           ixPerfProfAccCodeletHelp();
        }/* end if */

        return;
    }/* end if */

    /* If mode selected is NOT demo all, then call the individual mode selection function */
    else
    {
        statusMain = ixPerfProfAccCodeletSelection
                            (mode, param1, param2, param3, param4, param5,
                             param6, param7, param8, param9);
        /* If function does not return a success due to some error, print help and exit. */
        if (IX_PERFPROF_ACC_STATUS_SUCCESS!=statusMain)
        {
           ixPerfProfAccCodeletHelp();
        }/* end if */

        return;
    }/* end else */
} /* end of function ixPerfProfAccCodeletMain */

