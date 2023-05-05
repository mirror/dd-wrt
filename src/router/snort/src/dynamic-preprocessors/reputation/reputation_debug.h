/****************************************************************************
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2011-2013 Sourcefire, Inc.
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
 * 6/11/2011 - Initial implementation ... Hui Cao <hcao@sourcefire.com>
 *
 ****************************************************************************/

#ifndef _REPUTATION_DEBUG_H_
#define _REPUTATION_DEBUG_H_

#include <stdio.h>
#include "sfPolicyUserData.h"

/********************************************************************
 * Macros
 ********************************************************************/
#define DEBUG_REPUTATION            0x00000020  /* 16 */


#define REPUTATION_DEBUG__START_MSG  "REPUTATION Start ********************************************"
#define REPUTATION_DEBUG__END_MSG    "REPUTATION End **********************************************"


#endif  /* _REPUTATION_DEBUG_H_ */

