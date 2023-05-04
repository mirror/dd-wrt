/****************************************************************************
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2008-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************
 * 8/17/2008 - Initial implementation ... Todd Wease <twease@sourcefire.com>
 *
 ****************************************************************************/

#ifndef _DCE2_CL_H_
#define _DCE2_CL_H_

#include "dce2_list.h"
#include "dce2_session.h"
#include "sf_types.h"

/********************************************************************
 * Macros
 ********************************************************************/
#define DCE2_MOCK_HDR_LEN__CL  (sizeof(DceRpcClHdr))

/********************************************************************
 * Structures
 ********************************************************************/
typedef struct _DCE2_ClTracker
{
    DCE2_List *act_trackers;  /* List of activity trackers */

} DCE2_ClTracker;

/********************************************************************
 * Public function prototypes
 ********************************************************************/
void DCE2_ClInitRdata(uint8_t *);
void DCE2_ClProcess(DCE2_SsnData *, DCE2_ClTracker *);
void DCE2_ClCleanTracker(DCE2_ClTracker *);

#endif   /* _DCE2_CL_H_ */

