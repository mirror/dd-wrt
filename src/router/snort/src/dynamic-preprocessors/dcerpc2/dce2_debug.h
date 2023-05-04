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
 * Provides macros and functions for debugging the preprocessor.
 * If Snort is not configured to do debugging, macros are empty.
 *
 * 8/17/2008 - Initial implementation ... Todd Wease <twease@sourcefire.com>
 *
 ****************************************************************************/

#ifndef _DCE2_DEBUG_H_
#define _DCE2_DEBUG_H_

#include <stdio.h>

/********************************************************************
 * Public function prototypes
 ********************************************************************/
// Use like this: DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ALL, "msg\n");)
void DCE2_DebugMsg(int, const char *, ...);

int DCE2_DebugThis(int level);

/********************************************************************
 * Macros
 ********************************************************************/
#define DCE2_DEBUG_VARIABLE   "DCE2_DEBUG"

#define DCE2_DEBUG__NONE      0x00000000
#define DCE2_DEBUG__ROPTIONS  0x00000001
#define DCE2_DEBUG__CONFIG    0x00000002
#define DCE2_DEBUG__MAIN      0x00000004
#define DCE2_DEBUG__SMB       0x00000008
#define DCE2_DEBUG__CO        0x00000010
#define DCE2_DEBUG__EVENT     0x00000020
#define DCE2_DEBUG__MEMORY    0x00000040
#define DCE2_DEBUG__HTTP      0x00000080
#define DCE2_DEBUG__CL        0x00000100
#define DCE2_DEBUG__PAF       0x00000200
#define DCE2_DEBUG__ALL       0xffffffff

#define DCE2_DEBUG__START_MSG  "DCE/RPC Preprocessor *************************************"
#define DCE2_DEBUG__END_MSG    "**********************************************************"
#define DCE2_DEBUG__PAF_START_MSG  "DCE/RPC PAF =============================================="
#define DCE2_DEBUG__PAF_END_MSG    "=========================================================="

#ifdef DEBUG
#include <assert.h>
#define DCE2_ASSERT(code)             assert(code)
#else
#define DCE2_ASSERT(code)
#endif

#ifdef DEBUG_MSGS
#define DCE2_DEBUG_VAR(code)          code
#define DCE2_DEBUG_CODE(level, code)  { if (DCE2_DebugThis(level)) { code } }
#else
#define DCE2_DEBUG_VAR(code)
#define DCE2_DEBUG_CODE(level, code)
#endif

#endif  /* _DCE2_DEBUG_H_ */

