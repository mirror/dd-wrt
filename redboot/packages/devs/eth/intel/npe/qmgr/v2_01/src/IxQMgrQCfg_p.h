/**
 * @file    IxQMgrQCfg_p.h
 *
 * @author Intel Corporation
 * @date    07-Feb-2002
 *
 * @brief   This file contains the internal functions for config
 *
 * 
 * @par
 * IXP400 SW Release version  2.0
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright 2002-2005 Intel Corporation All Rights Reserved.
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

#ifndef IXQMGRQCFG_P_H
#define IXQMGRQCFG_P_H

/*
 * User defined header files
 */
#include "IxQMgr.h"

/*
 * Typedefs
 */
typedef struct
{
    unsigned wmSetCnt;    

    struct
    {
	char *qName;
	BOOL isConfigured;	
	unsigned int qSizeInWords;
	unsigned int qEntrySizeInWords;
	unsigned int ne;
	unsigned int nf;
	unsigned int numEntries;
	UINT32 baseAddress;
	UINT32 readPtr;
	UINT32 writePtr;
    } qStats[IX_QMGR_MAX_NUM_QUEUES];

} IxQMgrQCfgStats;

/*
 * Initialize the QCfg subcomponent
 */ 
void
ixQMgrQCfgInit (void);

/*
 * Uninitialize the QCfg subcomponent
 */ 
void
ixQMgrQCfgUninit (void);

/*
 * Get the Q size in words
 */ 
IxQMgrQSizeInWords
ixQMgrQSizeInWordsGet (IxQMgrQId qId);

/*
 * Get the Q entry size in words
 */ 
IxQMgrQEntrySizeInWords
ixQMgrQEntrySizeInWordsGet (IxQMgrQId qId);

/*
 * Get the generic cfg stats
 */
IxQMgrQCfgStats*
ixQMgrQCfgStatsGet (void);

/*
 * Get queue specific stats
 */
IxQMgrQCfgStats*
ixQMgrQCfgQStatsGet (IxQMgrQId qId);

/*
 * Check is the queue configured
 */
BOOL
ixQMgrQIsConfigured(IxQMgrQId qId);
 
#endif /* IX_QMGRQCFG_P_H */
