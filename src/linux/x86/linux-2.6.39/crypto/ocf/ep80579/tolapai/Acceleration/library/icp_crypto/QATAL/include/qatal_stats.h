/******************************************************************************
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
 *****************************************************************************/

/**
 ***************************************************************************
 * @file qatal_stats.h
 *
 * @defgroup QATALStats Statistics
 *
 * @ingroup QATAL
 *
 * @description
 *      This file contains the definitions and prototypes for QATAL
 *      statistics.
 *
 * @lld_start
 *      - <b>Incrementing Statistics:</b>\n
 *      Atomically increment the statistic on the internal stats structure.
 *
 *
 *      - <b>Providing a copy of the stats back to the user:</b>\n
 *      Read the variable for each stat field in the local internal stat structure. 
 *
 *      - <b>Stats Show:</b>\n
 *      Read the variables for each field in the local internal stat structure and print to the screen
 *
 * @lld_end
 *
 ***************************************************************************/

 

/***************************************************************************/

#ifndef QATAL_STATS_H
#define QATAL_STATS_H

/*
******************************************************************************
* Include public/global header files
******************************************************************************
*/

#include "cpa.h"

/*
*******************************************************************************
* Include private header files
*******************************************************************************
*/



/**
 *****************************************************************************
 * @ingroup icp_Qatal
 *      QAT-AL
 * @description
 *      This structure contains statistics on the QATAL
 *      Statistics are set to zero when the component
 *      is initialized. 
 * 
  ****************************************************************************/
typedef struct icp_qatal_stats_s
{
	Cpa32U totalNRGBTestInvoked;
    /**< OUT: Total number of NGB Tests invoked*/
    Cpa32U totalNumHeartbeatInvoked;
    /**< OUT: Total number of Heatbeat invoked */

    Cpa32U totalNumStatsinvoked;
    /**< OUT: Number of Stats tests invoked */
   Cpa32U totalNumStartALreq;
    /**< OUT: Number of Stats No. QAT Start AL requested*/

}icp_qatal_stats_t;




/**
*****************************************************************************
 * @ingroup Qatal
 *      Component statistics
 *
 * @description
 *      This enum is used to indicate the statistics we are gatherering
 *
 *****************************************************************************/
typedef enum {
    QATAL_NRBG_TEST_INVOKED = 0,
    /**< Component No. of times NRBG Test invoked */
    QATAL_HEATBEAT_INVOKED,
    /**< Component No. of Heartbeat Tests invoked*/
    QATAL_STATS_INVOKED,
    /**< Component No. of STATS test invoked*/
	 QATAL_STATS_START_REQ,
    /**< Component No. of STATS No. QAT Start AL requested*/
	QATAL_END_STAT,
	/**< End of Stats marker */
    
}qatal_comp_stats_type_t;

/**
*******************************************************************************
 * @ingroup qatalStats
 *      initialises the Qat AL stats
 *
 * @description
 *      This function initialises the stats array
 *
 * @retval None
 *
 *****************************************************************************/

void
Qatal_StatsInit(void);


/**
*******************************************************************************
 * @ingroup QATALStats
 *      shutdown the QAT-AL stats
 *
 * @description
 *      shutdown the QAT-AL stats
 *
 * @retval None
 *
 *****************************************************************************/
void
Qatal_StatsShutdown(void);

/**
*******************************************************************************
 * @ingroup QATALStats
 
 *      Increment a stat
 *
 * @description
 *      This function incrementes a stat
 *
 * @param offset     IN  offset of stat field in structure
 *
 * @retval None
 *
 *****************************************************************************/

void
Qatal_StatsInc(Cpa32U offset);

/**
*******************************************************************************
 * @ingroup QATALStats
 *      Copy the contents of the statistics structure for QAT-AL
 *
 * @description
 *      This function copies the statistics structure for QAT-AL
 *
 * @param pQatalStats  OUT stats structure to copy the stats for the into
 *
 * @retval None
 *
 *****************************************************************************/
void
Qatal_StatsCopyGet(icp_qatal_stats_t *pQatalStats);

/**
*******************************************************************************
 * @ingroup QATALStats
 *      Copy the contents of the statistics structure for QAT-AL
 *
 * @description
 *      This function copies the statistics structure for QAT-AL
 *      into an address supplied as a parameter.
 *
 * @param pQatalStats  OUT Address of stats structure to copy the stats
 *
 * @retval None
 *
 *****************************************************************************/
void
Qatal_StatsGetAddress(icp_qatal_stats_t **pQatalStats);

/**
*******************************************************************************
 * @ingroup QATALStats
 *      print the stats to standard output
 *
 * @description
 *      The statistics are printed to standard output.
 *
 * @retval None
 *
 * @see QATAL_StatsCopyGet()
 *
 *****************************************************************************/
void
Qatal_StatsShow(void);


#endif /*QATAL_STATS_H_*/
