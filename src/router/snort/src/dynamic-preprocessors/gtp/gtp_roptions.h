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
 * This processes the rule options for this preprocessor
 *
 * Author: Hui Cao
 * Date: 07-25-2011
 ****************************************************************************/

#ifndef _GTP_ROPTIONS_H_
#define _GTP_ROPTIONS_H_

#include "gtp_config.h"



/********************************************************************
 * Structures
 ********************************************************************/
typedef struct _GTP_IEData
{
    uint16_t length;  /*length of the data*/
    uint16_t shift;  /*shift relative to the header*/
    uint32_t msg_id;  /* used to associate to current msg */

}GTP_IEData;

typedef struct _GTP_Roptions
{

	/* gtp_type data*/
	uint8_t gtp_type;
    /* gtp_version data*/
    uint8_t gtp_version;
    uint8_t *gtp_header;
    uint32_t msg_id;  /* used to associate to current msg */
	/* gtp ie data */
    GTP_IEData *gtp_infoElements;

} GTP_Roptions;

/*For every value types[i], the bit mask show the version to be applied
 * bit 1 is for version 0,
 * bit 2 is for version 1,
 * bit 3 is for version 2
 * */
typedef struct _GTP_TypeRuleOptData
{
    /*Total 256 types*/
    uint8_t types[MAX_GTP_TYPE_CODE + 1];
} GTP_TypeRuleOptData;

/*
 * byte 0 is for version 0,
 * byte 1 is for version 1,
 * byte 2 is for version 2
 * */
typedef struct _GTP_InfoRuleOptData
{
    uint8_t types[MAX_GTP_VERSION_CODE + 1];
} GTP_InfoRuleOptData;

/********************************************************************
 * Public function prototypes
 ********************************************************************/
void GTP_RegRuleOptions(struct _SnortConfig *sc);


#endif  /* _GTP_ROPTIONS_H_ */

