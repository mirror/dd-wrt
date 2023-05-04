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

/* $Id$ */
#ifndef __RULES_H__
#define __RULES_H__


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "event.h"
#include "decode.h"
#include "signature.h"
#include "parser/IpAddrSet.h"
#include "spo_plugbase.h"
#include "sf_vartable.h"
#include "sf_types.h"
#include "plugin_enum.h"
#include "sfutil/sfportobject.h"
#include "detection_options.h"

#define EXCEPT_SRC_IP  0x01
#define EXCEPT_DST_IP  0x02
#define ANY_SRC_PORT   0x04
#define ANY_DST_PORT   0x08
#define ANY_FLAGS      0x10
#define EXCEPT_SRC_PORT 0x20
#define EXCEPT_DST_PORT 0x40
#define BIDIRECTIONAL   0x80
#define ANY_SRC_IP      0x100
#define ANY_DST_IP      0x200

#define EXCEPT_IP      0x01

#define R_FIN          0x01
#define R_SYN          0x02
#define R_RST          0x04
#define R_PSH          0x08
#define R_ACK          0x10
#define R_URG          0x20
#define R_ECE          0x40  /* ECN echo, RFC 3168 */
#define R_CWR          0x80  /* Congestion Window Reduced, RFC 3168 */

#define MODE_EXIT_ON_MATCH   0
#define MODE_FULL_SEARCH     1

#define CHECK_SRC_IP         0x01
#define CHECK_DST_IP         0x02
#define INVERSE              0x04
#define CHECK_SRC_PORT       0x08
#define CHECK_DST_PORT       0x10

#define SESSION_PRINTABLE    1
#define SESSION_ALL          2

#define MODE_EXIT_ON_MATCH   0
#define MODE_FULL_SEARCH     1

#define SRC                  0
#define DST                  1

#ifndef PARSERULE_SIZE
#define PARSERULE_SIZE	     65535
#endif

/*  D A T A  S T R U C T U R E S  *********************************************/
/* I'm forward declaring the rules structures so that the function
   pointer lists can reference them internally */

struct _ListHead;    /* forward decleartion of ListHead data struct */

typedef enum _RuleType
{
    RULE_TYPE__NONE = 0,
    RULE_TYPE__ALERT,
    RULE_TYPE__DROP,
    RULE_TYPE__LOG,
    RULE_TYPE__PASS,
    RULE_TYPE__REJECT,
    RULE_TYPE__SDROP,
    RULE_TYPE__MAX

} RuleType;

#ifndef f_ptr
#define f_ptr fptr.fptr
#endif
#ifndef vf_ptr
#define vf_ptr fptr.void_fptr
#endif

typedef struct _RspFpList
{
    union {
        int (*fptr)(Packet*, void*);
        void *vfptr;
    } fptr;
    void *params; /* params for the plugin.. type defined by plugin */
    struct _RspFpList *next;
} RspFpList;

typedef struct _TagData
{
    int tag_type;       /* tag type (session/host) */
    int tag_seconds;    /* number of "seconds" units to tag for */
    int tag_packets;    /* number of "packets" units to tag for */
    int tag_bytes;      /* number of "type" units to tag for */
    int tag_metric;     /* (packets | seconds | bytes) units */
    int tag_direction;  /* source or dest, used for host tagging */
} TagData;

struct _RuleListNode;

typedef struct _ListHead
{
    struct _OutputFuncNode *LogList;
    struct _OutputFuncNode *AlertList;
    struct _RuleListNode *ruleListNode;
} ListHead; 

typedef struct _RuleListNode
{
    ListHead *RuleList;         /* The rule list associated with this node */
    RuleType mode;              /* the rule mode */
    int rval;                   /* 0 == no detection, 1 == detection event */
    int evalIndex;              /* eval index for this rule set */
    char *name;                 /* name of this rule list (for debugging)  */
    struct _RuleListNode *next; /* the next RuleListNode */
} RuleListNode;

typedef struct _RuleState
{
    uint32_t sid;
    uint32_t gid;
    int state;
    RuleType action;
    struct _RuleState *next;

} RuleState;

#endif /* __RULES_H__ */
