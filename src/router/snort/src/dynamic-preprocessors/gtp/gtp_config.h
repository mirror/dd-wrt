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
 * Provides convenience functions for parsing and querying configuration.
 *
 * 8/1/2011 - Initial implementation ... Hui Cao <hcao@sourcefire.com>
 *
 ****************************************************************************/

#ifndef _GTP_CONFIG_H_
#define _GTP_CONFIG_H_

#include "sfPolicyUserData.h"
#include "snort_bounds.h"
#include "gtp_debug.h"

#define GTP_NAME  "gtp"

#define MAX_GTP_TYPE_CODE      (255)
#define MIN_GTP_TYPE_CODE      (0)
#define MAX_GTP_IE_CODE        (255)
#define MIN_GTP_IE_CODE        (0)
#define MAX_GTP_VERSION_CODE   (2)
#define MIN_GTP_VERSION_CODE   (0)

/*
 * Message type
 */
typedef struct _GTP_MsgType
{
    uint8_t type; /* the message type*/
    uint8_t isKeyword; /*whether the name can be used as keyword*/
    char *name;  /*name of the type*/

}GTP_MsgType;


/*
 * Information elements
 */
typedef struct _GTP_InfoElement
{
    uint8_t type; /* the IE type*/
    uint8_t isKeyword; /*whether the name can be used as keyword*/
    char *name;  /*name of the IE*/
    uint16_t length; /* the length of IE; if 0, means variable length*/

}GTP_InfoElement;


/*
 * One of these structures is kept for each configured
 * server port.
 */
typedef struct _gtpPortlistNode
{
	uint16_t server_port;
	struct _gtpPortlistNode* nextp;
} GTPPortNode;

/*
 * GTP preprocessor configuration.
 *
 * ports: Which ports to check for GTP messages
 * infoElementTable: information elements table, for quick retrieve
 * msgTypeTable: message type table, for quick retrieve
 */
typedef struct _gtpConfig
{

	uint8_t  ports[MAXPORTS/8];
	GTP_InfoElement* infoElementTable[MAX_GTP_VERSION_CODE + 1 ][MAX_GTP_IE_CODE + 1];
	GTP_MsgType *msgTypeTable[MAX_GTP_VERSION_CODE + 1][MAX_GTP_TYPE_CODE + 1];
	int ref_count;

} GTPConfig;

/********************************************************************
 * Public function prototypes
 ********************************************************************/
void  ParseGTPArgs(GTPConfig *, u_char*);
GTP_MsgType*  GetMsgTypeByName(uint8_t, char *);
GTP_InfoElement*  GetInfoElementByName(uint8_t, char *);

#endif
