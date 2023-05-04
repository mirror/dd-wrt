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
 *
 ****************************************************************************/

#ifndef _DCE2_ROPTIONS_H_
#define _DCE2_ROPTIONS_H_

#include "dcerpc.h"

/********************************************************************
 * Structures
 ********************************************************************/
typedef struct _DCE2_Roptions
{
    /* dce_iface */
    int first_frag;    /* Set to sentinel if not applicable */
    Uuid iface;
    /* For connectionless */
    uint32_t iface_vers;        /* For connectionless */

    /* For connection-oriented */
    uint16_t iface_vers_maj;
    uint16_t iface_vers_min;

    /* dce_opnum */
    int opnum;    /* Set to sentinel if not applicable */

    /* dce_byte_test */
    int hdr_byte_order;   /* Set to sentinel if not applicable */
    int data_byte_order;  /* Set to sentinel if not applicable */

    /* dce_stub_data */
    const uint8_t *stub_data;  /* Set to NULL if not applicable */

} DCE2_Roptions;

/********************************************************************
 * Public function prototypes
 ********************************************************************/
struct _SnortConfig;
void DCE2_RegRuleOptions(struct _SnortConfig *);
void DCE2_PrintRoptions(DCE2_Roptions *);
int DCE2_GetByteOrder(void *, int32_t);

#endif  /* _DCE2_ROPTIONS_H_ */

