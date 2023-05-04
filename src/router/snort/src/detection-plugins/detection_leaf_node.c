/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/*
CHANGE HISTORY
==============

2014-10-30	Victor Roemer	<viroemer@cisco.com>
 . REMOVED leaf node evaluation from `detection_options.c'.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sftarget_reader.h"

typedef enum _LEAF_STATUS {
    Leaf_SkipPorts,
    Leaf_CheckPorts,
    Leaf_Abort
} LEAF_STATUS;

#ifdef HACK_DETECTION_LEAF_NODE_C
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

// signature.h
typedef enum _ServiceOverride {
    ServiceOverride_ElsePorts = 0,
    ServiceOverride_AndPorts,
    ServiceOverride_OrPorts,
    ServiceOverride_Nil
} ServiceOverride;

typedef struct _ServiceInfo {
    uint16_t service_ordinal;
} ServiceInfo;

typedef struct _SigInfo {
    ServiceInfo services[8];
    unsigned int num_services;
ServiceOverride service_override;
} SigInfo;

// treenodes.h
typedef struct {
    SigInfo sigInfo;
} OptTreeNode;

// decode.h
typedef struct {
    uint16_t application_protocol_ordinal;
} Packet;

// detection-plugins/detection_options.h
typedef struct {
    OptTreeNode option_data[1];
} detection_option_tree_node_t;

typedef struct {
    Packet p[1];
} detection_option_eval_data_t;

// prototypes
static inline LEAF_STATUS leaf_node_check_otn_service (OptTreeNode*, Packet*);
static inline LEAF_STATUS detection_leaf_node_eval (detection_option_tree_node_t*, detection_option_eval_data_t*);
#endif // DETECTION_LEAF_NODE_C

// (detection_option_eval_data_t*) helper(s)
#define PacketService(p)     (p)->application_protocol_ordinal

// (OptTreeNode*) helper(s)
#define OtnServiceCount(otn) (otn)->sigInfo.num_services
#define OtnAndPorts(otn)     ((otn)->sigInfo.service_override == ServiceOverride_AndPorts)
#define OtnOrPorts(otn)      ((otn)->sigInfo.service_override == ServiceOverride_OrPorts)
#define OtnElsePorts(otn)    ((otn)->sigInfo.service_override == ServiceOverride_ElsePorts)

//#define PortOnlyRule(otn)    (OtnServiceCount (otn) == 0)
#define PacketUnknown(pkt)   (PacketService (pkt) == 0)


static inline LEAF_STATUS
leaf_node_check_otn_service (OptTreeNode * otn, Packet * packet)
{
    bool service_match = false;
    unsigned int i;

    if (PacketUnknown (packet))
    {
#ifdef TARGET_BASED
        if (OtnAndPorts (otn))
            return (Leaf_Abort);
#endif
        return (Leaf_CheckPorts);
    }

#ifdef TARGET_BASED
    for (i = 0; i < OtnServiceCount (otn); i++)
    {
        const uint16_t ordinal = otn->sigInfo.services[ i ].service_ordinal;
        if (PacketService (packet) == ordinal)
        {
            service_match = true;
        }
    }
#endif

    if (service_match)
    {
        // identified service matches the rule
#ifdef TARGET_BASED
        if (OtnAndPorts (otn))
            return (Leaf_CheckPorts);
#endif
        return (Leaf_SkipPorts);
    }
    else
    {
#ifdef TARGET_BASED
        if (!OtnOrPorts (otn))
            return (Leaf_Abort);
#endif
    }

    return (Leaf_CheckPorts);
}

static inline LEAF_STATUS
detection_leaf_node_eval (detection_option_tree_node_t * node,
                          detection_option_eval_data_t * eval_data)
{
    OptTreeNode *otn = (OptTreeNode*) node->option_data;
    Packet * packet = (Packet*) eval_data->p;


#ifdef TARGET_BASED
    if (!IsAdaptiveConfigured() || PacketService(packet) <= 0) 
        return (Leaf_CheckPorts);
#endif

    return leaf_node_check_otn_service (otn, packet);
}
