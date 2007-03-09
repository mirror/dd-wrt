/**
 * @file    IxQMgrQAccess_p.h
 *
 * @author Intel Corporation
 * @date    30-Oct-2001
 *
 * @brief   QAccess private header file
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

#ifndef IXQMGRQACCESS_P_H
#define IXQMGRQACCESS_P_H

/*
 * User defined header files
 */
#include "IxQMgr.h"

/* 
 * Global variables declarations.
 */
extern volatile UINT32 * ixQMgrAqmIfQueAccRegAddr[]; 

/* 
 * Initialise the Queue Access component
 */
void
ixQMgrQAccessInit (void);

/*
 * read the remainder of a multi-word queue entry 
 * (the first word is already read)
 */
IX_STATUS
ixQMgrQReadMWordsMinus1 (IxQMgrQId qId,
                         UINT32 *entry);

/*
 * Fast access : pop a q entry from a single word queue
 */
extern __inline__ UINT32 ixQMgrQAccessPop(IxQMgrQId qId);

extern __inline__ UINT32 ixQMgrQAccessPop(IxQMgrQId qId)
{
  return *(ixQMgrAqmIfQueAccRegAddr[qId]);
}

/*
 * Fast access : push a q entry in a single word queue
 */
extern __inline__ void ixQMgrQAccessPush(IxQMgrQId qId, UINT32 entry);

extern __inline__ void ixQMgrQAccessPush(IxQMgrQId qId, UINT32 entry)
{
  *(ixQMgrAqmIfQueAccRegAddr[qId]) = entry;
}

#endif/*IXQMGRQACCESS_P_H*/
