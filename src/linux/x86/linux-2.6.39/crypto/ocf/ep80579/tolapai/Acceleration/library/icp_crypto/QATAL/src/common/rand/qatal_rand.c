/*
 ***************************************************************************
 *
 * This file is provided under a dual BSD/GPLv2 license.  When using or 
 *   redistributing this file, you may do so under either license.
 * 
 *   GPL LICENSE SUMMARY
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify 
 *   it under the terms of version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 * 
 *   This program is distributed in the hope that it will be useful, but 
 *   WITHOUT ANY WARRANTY; without even the implied warranty of 
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 *   General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License 
 *   along with this program; if not, write to the Free Software 
 *   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *   The full GNU General Public License is included in this distribution 
 *   in the file called LICENSE.GPL.
 * 
 *   Contact Information:
 *   Intel Corporation
 * 
 *   BSD LICENSE 
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 *   All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without 
 *   modification, are permitted provided that the following conditions 
 *   are met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided with the 
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its 
 *       contributors may be used to endorse or promote products derived 
 *       from this software without specific prior written permission.
 * 
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * 
 *  version: Security.L.1.0.3-98
 *
 ***************************************************************************/

/**
 *****************************************************************************
 * @file  qatal_rand.c
 *
 * @ingroup icp_Qatal
 *
 * @description
 *        This file contains the functions to used by create, test and remove
 *        the Entrophy Sample data
 *
 *****************************************************************************/

#include "qatal_rand.h"
#include "icp_qatal_cfg.h"
#include "qatal_log.h"
#include "qatal_mem.h"
#include "qatal_init_defs.h"
#include "qatal_common.h"

/*********************************************************
 ********************* Internal Data**********************
 *********************************************************/



static Cpa32U poker[MAX_BIT_COMBINATION];
                                    /* occurrences of each nibble possibility*/
static Cpa32U zeroruns[MAX_RUNS_PLUS_OVERFLOW];
                                    /* length of run of zeros                */
static Cpa32U oneruns[MAX_RUNS_PLUS_OVERFLOW];
                                    /* length of run of ones                 */

static Cpa32U sum_ones = 0;    /* sum of ones                            */
static Cpa32U crzero = 0;
static Cpa32U crone = 0;       /* carry runs zero, carry runs one       */

static Cpa32U pokersum = 0;
static Cpa32U oneruns_sixplus = 0;
static Cpa32U zeroruns_sixplus = 0;

/*********************************************************
 ********************* Internal API **********************
 *********************************************************/

/*
 * @fn Qatal_RandRANDCalc - Calculates the internal statics of the RBG Sample
 */
static void
Qatal_RandRANDCalc(Cpa8U nibble)
{



    /* The alogorith used to test the "Random-ness" of the data sample
       is specified in FIPS-140-1 as specified in the QAT-QL HLD. */

    /* For Every Nibble of data a series of statistics are gathered */

    switch (nibble)
    {

      case BIT_COMBITATION_0:   /* 0000 */
                                               /* Monobit test (no ones)*/
                        poker[BIT_COMBITATION_0]++;        /* Poker test */
                        if (crone > 0)
                        {
                            oneruns[crone]++;
                        }
                        crzero += 4;
                        crone = 0;
                        if (crzero > MAX_NO_OF_RUNS)
                        {
                            crzero = MAX_NO_OF_RUNS;
                        }                     /* So crzero can't exceed 37 */
                        break;
        case BIT_COMBITATION_1:    /* 0001 */
                        sum_ones++;                        /* Monobit test */
                        poker[BIT_COMBITATION_1]++;          /* Poker test */
                        crzero += 3;
                        zeroruns[crzero]++;
                        if (crone > 0)
                        {
                            oneruns[crone]++;
                        }
                        crzero = 0;
                        crone = 1;
                        break;
        case BIT_COMBITATION_2:   /* 0010 */
                        sum_ones++;           /* Monobit test */
                        poker[BIT_COMBITATION_2]++;  /* Poker test */
                        crzero += 2;
                        zeroruns[crzero]++;
                        oneruns[1]++;
                        if (crone > 0)
                        {
                            oneruns[crone]++;
                        }
                        crzero = 1;
                        crone = 0;
                        break;
        case BIT_COMBITATION_3:   /* 0011 */
                        sum_ones += 2;            /* Monobit test */
                        poker[BIT_COMBITATION_3]++;/* Poker test */
                        crzero += 2;
                        zeroruns[crzero]++;
                        if (crone > 0)
                        {
                            oneruns[crone]++;
                        }
                        crzero = 0;
                        crone = 2;
                        break;
        case BIT_COMBITATION_4:   /* 0100 */
                        sum_ones++;             /* Monobit test */
                        poker[BIT_COMBITATION_4]++;/* Poker test */
                        crzero++;
                        zeroruns[crzero]++;
                        oneruns[1]++;
                        if (crone > 0)
                        {
                            oneruns[crone]++;
                        }
                        crzero = 2;
                        crone = 0;
                        break;
        case BIT_COMBITATION_5:   /* 0101 */
                        sum_ones += 2;                  /* Monobit test */
                        poker[BIT_COMBITATION_5]++;     /* Poker test */
                        crzero++;
                        zeroruns[crzero]++;
                        zeroruns[1]++;
                        oneruns[1]++;
                        if (crone > 0)
                        {
                            oneruns[crone]++;
                        }
                        crzero = 0;
                        crone = 1;
                        break;
        case BIT_COMBITATION_6:   /* 0110 */
                        sum_ones += 2;                  /* Monobit test */
                        poker[BIT_COMBITATION_6]++;     /* Poker test */
                        crzero++;
                        zeroruns[crzero]++;
                        oneruns[2]++;
                        if (crone > 0)
                        {
                            oneruns[crone]++;
                        }
                        crzero = 1;
                        crone = 0;
                        break;
        case BIT_COMBITATION_7:   /* 0111 */
                        sum_ones += 3;                  /* Monobit test */
                        poker[BIT_COMBITATION_7]++;     /* Poker test */
                        crzero++;
                        zeroruns[crzero]++;
                        if (crone > 0)
                        {
                            oneruns[crone]++;
                        }
                        crzero = 0;
                        crone = 3;
                        break;
        case BIT_COMBITATION_8:   /* 1000 */
                        sum_ones += 1;                  /* Monobit test */
                        poker[BIT_COMBITATION_8]++;     /* Poker test */
                        crone++;
                        oneruns[crone]++;
                        if (crzero > 0)
                        {
                            zeroruns[crzero]++;
                        }
                        crzero = 3;
                        crone = 0;
                        break;
        case BIT_COMBITATION_9:   /* 1001 */
                        sum_ones += 2;                  /* Monobit test */
                        poker[BIT_COMBITATION_9]++;     /* Poker test */
                        crone++;
                        oneruns[crone]++;
                        zeroruns[2]++;
                        if (crzero > 0)
                        {
                            zeroruns[crzero]++;
                        }
                        crzero = 0;
                        crone = 1;
                        break;
        case BIT_COMBITATION_A:   /* 1010 */
                        sum_ones += 2;                  /* Monobit test */
                        poker[BIT_COMBITATION_A]++;     /* Poker test */
                        crone++;
                        oneruns[crone]++;
                        oneruns[1]++;
                        zeroruns[1]++;
                        if (crzero > 0)
                        {
                            zeroruns[crzero]++;
                        }
                        crzero = 1;
                        crone = 0;
                        break;
        case BIT_COMBITATION_B:   /* 1011 */
                        sum_ones += 3;                  /* Monobit test */
                        poker[BIT_COMBITATION_B]++;     /* Poker test */
                        crone++;
                        oneruns[crone]++;
                        zeroruns[1]++;
                        if (crzero > 0)
                        {
                            zeroruns[crzero]++;
                        }
                        crzero = 0;
                        crone = 2;
                        break;
        case BIT_COMBITATION_C:   /* 1100 */
                        sum_ones += 2;                  /* Monobit test */
                        poker[BIT_COMBITATION_C]++;     /* Poker test */
                        crone += 2;
                        oneruns[crone]++;
                        if (crzero > 0)
                        {
                            zeroruns[crzero]++;
                        }
                        crzero = 2;
                        crone = 0;
                        break;
        case BIT_COMBITATION_D:   /* 1101 */
                        sum_ones += 3;                  /* Monobit test */
                        poker[BIT_COMBITATION_D]++;     /* Poker test */
                        crone += 2;
                        oneruns[crone]++;
                        zeroruns[1]++;
                        if (crzero > 0)
                        {
                            zeroruns[crzero]++;
                        }
                        crzero = 0;
                        crone = 1;
                        break;
        case BIT_COMBITATION_E:   /* 1110 */
                        sum_ones += 3;                  /* Monobit test */
                        poker[BIT_COMBITATION_E]++;     /* Poker test */
                        crone += 3;
                        oneruns[crone]++;
                        if (crzero > 0)
                        {
                            zeroruns[crzero]++;
                        }
                        crzero = 1;
                        crone = 0;
                        break;
        case BIT_COMBITATION_F:   /* 1111 */
                        sum_ones += 4;                  /* Monobit test */
                        poker[BIT_COMBITATION_F]++;     /* Poker test*/
                        if (crzero > 0)
                        {
                            zeroruns[crzero]++;
                        }
                        crzero = 0;
                        crone += 4;
                        if (crone > MAX_NO_OF_RUNS)
                        {
                            crone = MAX_NO_OF_RUNS;
                                            /* So crone can't exceed 37 */
                        }
                        break;
        default:
#ifdef ICP_DEBUG
                        ixOsalStdLog("Incorrect nibble: %d\n", nibble);
#endif //ICP_DEBUG
                        break;
    }

    return;

}


/*
 * @fn Qatal_RandRANDinit - initialize the data block to gather the statistics
 */

static void
Qatal_RandRANDinit(void )
{
    Cpa8U count = 0;

    sum_ones = 0;    /* sum of ones                            */
    crzero = 0;
    crone = 0;       /* carry runs zero, carry runs one       */

    pokersum = 0;
    oneruns_sixplus = 0;
    zeroruns_sixplus = 0;
    for (count = 0; count < MAX_BIT_COMBINATION; count++)
    {
        poker[count] = 0;                      /* initialize poker */
    }

    for (count = 0; count < MAX_RUNS_PLUS_OVERFLOW; count++)
    {
        zeroruns[count] = 0;                 /* initialize runs */
        oneruns[count] = 0;
    }
    return ;
}

/*********************************************************
 ********************* External API **********************
 *********************************************************/
/*
 * @fn Qatal_RandInit
 */


/* Allocates a block of data for RAND init */
CpaStatus
Qatal_RandInit( void )
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    Cpa8U *pQATAL_RAND_PTR = NULL;
    Cpa8U **ppQATAL_RAND_PTR = &pQATAL_RAND_PTR;
    CpaStatus tmpstatus = CPA_STATUS_SUCCESS;

    /* Return the address of RAND Entrophy block */
    tmpstatus = Qatal_GetRANDDataBlockPtr(ppQATAL_RAND_PTR);

    /* If pointer to RAND Block is NULL allocate shared DRAM */
    if ((NULL == pQATAL_RAND_PTR) && (CPA_STATUS_FAIL == tmpstatus))
    {
        /* allocate memory for RAND */
        status = QATAL_MEM_SHARED_ALLOC(ppQATAL_RAND_PTR,
                    QATAL_RAND_BLOCK_SIZE );

        if (CPA_STATUS_SUCCESS != status )
        {
            status = CPA_STATUS_RESOURCE;

        }
        else
        {
            status = Qatal_PutRANDDataBlockPtr(ppQATAL_RAND_PTR);

            /* Initialize Block sample to 0x55 */
            QATAL_MEM_SET(pQATAL_RAND_PTR,0x55,QATAL_RAND_BLOCK_SIZE);

            if (CPA_STATUS_SUCCESS != status )
            {

                if (NULL != ppQATAL_RAND_PTR)
                {
                    QATAL_MEM_SHARED_FREE(&pQATAL_RAND_PTR);
                }
                status = CPA_STATUS_RESOURCE;
            }
        }
    }
    /* The pointer is not empty .. why? is memory all ready allocated or
       did previous deallocate fail */
    else
    {
        status = CPA_STATUS_FAIL;
    }
    return status;
}

/*
 * @fn Qatal_RandShutdown
 */


/* DeAllocates a block of data for RAND */
CpaStatus
Qatal_RandShutdown(void )
{
    CpaStatus tmpstatus = CPA_STATUS_SUCCESS;
    CpaStatus status = CPA_STATUS_SUCCESS;
    Cpa8U *pQATAL_RAND_PTR = NULL;
    Cpa8U **ppQATAL_RAND_PTR = &pQATAL_RAND_PTR;

    /* Return the address of RAND Entrophy block */
    tmpstatus = Qatal_GetRANDDataBlockPtr(ppQATAL_RAND_PTR);

    /* Is pointer allocated .. if so free it */
    if ((pQATAL_RAND_PTR != NULL) &&
        (CPA_STATUS_SUCCESS == tmpstatus))
    {
        /* deallocate memory for RAND */
        QATAL_MEM_SHARED_FREE(&pQATAL_RAND_PTR);
        pQATAL_RAND_PTR = NULL;
        tmpstatus = Qatal_PutRANDDataBlockPtr(ppQATAL_RAND_PTR);
        if (CPA_STATUS_SUCCESS == tmpstatus )
        {
            status = CPA_STATUS_RESOURCE;
        }
    }
    /* Pointer was free .. ie no memory to free .. therefor return error msg. */
    else
    {
        status = CPA_STATUS_RESOURCE;
    }
    return status;
}

/*
 * @fn icp_QatalRandEntrophyTestRun
 */

CpaStatus
icp_QatalRandEntrophyTestRun(void )
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    Cpa8U *pQATAL_RAND_PTR = NULL;
    Cpa8U *plptr = NULL;
    Cpa8U **ppQATAL_RAND_PTR = &pQATAL_RAND_PTR;
    Cpa8U nibbleA = 0;
    Cpa8U nibbleB = 0;
    Cpa32U xadder = 0;
    Cpa32U i = 0;
    Cpa32U temp = 0;

#ifdef ICP_DEBUG
    Cpa8U *pMsg = NULL;
#endif //ICP_DEBUG

    /* Return the address of RAND Entrophy block */
    status = Qatal_GetRANDDataBlockPtr(ppQATAL_RAND_PTR);


#ifdef ICP_DEBUG
        ixOsalStdLog("Entrophy Block Addr >>");
        ixOsalStdLog("%016lX", (long unsigned int)pQATAL_RAND_PTR);
        ixOsalStdLog("\n");

        ixOsalStdLog("Entropht Block Data >>");

        pMsg = (Cpa8U *)pQATAL_RAND_PTR;
        for (i = 0; i < 16 ; i++)
        {
            ixOsalStdLog("%02X", pMsg[i]);
            if (i%4 == 3)
            {
                ixOsalStdLog(" ");
            }
        }
        ixOsalStdLog(" <<< End of msg \n");
#endif //ICP_DEBUG


    if (CPA_STATUS_SUCCESS == status)
    {
        /* init working data - no return code - it can not fail */
        Qatal_RandRANDinit();

        /* for every nibble in every byte of the data sample */
        plptr = pQATAL_RAND_PTR ;
        while (xadder < NO_OF_BITS)
        {
            /* Check if NULL ptr ... added due to KlockWorks */

            if (NULL == plptr)
            {
                status = CPA_STATUS_FAIL;
                xadder =  NO_OF_BITS;
            }
            else
            {
                nibbleA = (*plptr) & 0x0F;
                xadder = xadder + NEXT_NIBBLE;
                Qatal_RandRANDCalc(nibbleA);

                if (xadder < NO_OF_BITS)
                {
                    nibbleB = ((*plptr) >> NEXT_NIBBLE) & 0x0F;
                    xadder = xadder + NEXT_NIBBLE;/* increment bit position */
                    Qatal_RandRANDCalc(nibbleB);
                }
                /* Next Address */
                plptr ++;
            }
        }
        /* Need to count for any runs at the end of the 20000 bits */
        if(crone > 0)
        {
            oneruns[crone]++;
        }
        if(crzero > 0)
        {
            zeroruns[crzero]++;
        }

        /* PRECALCULATE FOR POKER TEST */
        for (i = 0; i < MAX_BIT_COMBINATION; i++)
        {
            temp = poker[i] * poker[i];
            pokersum += temp;
        }


        /* Precalculate for RUNS: Sum up runs between 6 & 33 */
        for (i = 6; i < MAX_RUNS_PLUS_OVERFLOW; i++)
        {
            oneruns_sixplus = oneruns_sixplus + oneruns[i];
            zeroruns_sixplus = zeroruns_sixplus + zeroruns[i];
        }

        /* MONOBIT RESULTS */
        if (sum_ones <= TOTAL_ONES_LL)
        {
            status = CPA_STATUS_FAIL;
        }
        else if (sum_ones >= TOTAL_ONES_UL)
        {
            status = CPA_STATUS_FAIL;
        }
        /* POKER RESULTS */
        else if (pokersum <= 1562822)
        {
            status = CPA_STATUS_FAIL;
        }
        else if (pokersum >= 1580438) /* max upper limit of 57.3984 */
        {
            status = CPA_STATUS_FAIL;
        }
        /* RUNS RESULT */
        else if ( (oneruns[1] < 2315) || (oneruns[1] > 2685) )
        {
            status = CPA_STATUS_FAIL;
        }
        else if ( (oneruns[2] < 1114) || (oneruns[2] > 1386) )
        {
            status = CPA_STATUS_FAIL;
        }
        else if ( (oneruns[3] < 527) || (oneruns[3] > 723) )
        {
            status = CPA_STATUS_FAIL;
        }
        else if ( (oneruns[4] < 240) || (oneruns[4] > 384) )
        {
            status = CPA_STATUS_FAIL;
        }
        else if ( (oneruns[5] < 103) || (oneruns[5] > 209) )
        {
            status = CPA_STATUS_FAIL;
        }
        else if  ( (oneruns_sixplus < 103) || (oneruns_sixplus > 209) )
        {
            status = CPA_STATUS_FAIL;
        }
        else if ( (zeroruns[1] < 2267) || (zeroruns[1] > 2733) )
        {
            status = CPA_STATUS_FAIL;
        }
        else if ( (zeroruns[2] < 1079) || (zeroruns[2] > 1421) )
        {
            status = CPA_STATUS_FAIL;
        }
        else if ( (zeroruns[3] < 502) || (zeroruns[3] > 748) )
        {
            status = CPA_STATUS_FAIL;
        }
        else if ( (zeroruns[4] < 223) || (zeroruns[4] > 402) )
        {
            status = CPA_STATUS_FAIL;
        }
        else if ( (zeroruns[5] < 90) || (zeroruns[5] > 223) )
        {
            status = CPA_STATUS_FAIL;
        }
        else if ( (zeroruns_sixplus < 90) || (zeroruns_sixplus > 223) )
        {
            status = CPA_STATUS_FAIL;
        }
        /* LONG RUN TEST */
        else if ((oneruns[MAX_NO_OF_RUNS] +
                  oneruns[MAX_NO_OF_RUNS + 1] +
                  oneruns[MAX_NO_OF_RUNS + 2] +
                  oneruns[MAX_NO_OF_RUNS + 3]) > 0)
        {
           status = CPA_STATUS_FAIL;
        }
        else if ((zeroruns[MAX_NO_OF_RUNS] +
                  zeroruns[MAX_NO_OF_RUNS + 1 ] +
                  zeroruns[MAX_NO_OF_RUNS + 2 ] +
                  zeroruns[MAX_NO_OF_RUNS + 3]) > 0)
        {
            status = CPA_STATUS_FAIL;
        }
        else
        {
            status = CPA_STATUS_SUCCESS;
        }
    }
    else
    {
        status = CPA_STATUS_FAIL;
    }
    return status;
}
