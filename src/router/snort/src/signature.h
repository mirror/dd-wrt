/* $Id$ */
/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Author(s):   Andrew R. Baker <andrewb@sourcefire.com>
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
#ifndef __SIGNATURE_H__
#define __SIGNATURE_H__

#ifdef OSF1
#include <sys/bitypes.h>
#endif

#include <sys/types.h>
#include <stdio.h>

#include "sfutil/sfghash.h"
#include "sf_types.h"

struct _OptTreeNode;
struct _SnortConfig;
struct _RuleTreeNode;

/* this contains a list of the URLs for various reference systems */
typedef struct _ReferenceSystemNode
{
    char *name;
    char *url;
    struct _ReferenceSystemNode *next;

} ReferenceSystemNode;

ReferenceSystemNode * ReferenceSystemAdd(ReferenceSystemNode **, char *, char *);
ReferenceSystemNode * ReferenceSystemLookup(ReferenceSystemNode *, char *);
void ParseReferenceSystemConfig(char *args);


/* XXX: update to point to the ReferenceURLNode in the referenceURL list */
typedef struct _ReferenceNode
{
    char *id;
    ReferenceSystemNode *system;
    struct _ReferenceNode *next;

} ReferenceNode;

ReferenceNode * AddReference(struct _SnortConfig *, ReferenceNode **, char *, char *);
void FPrintReference(FILE *, ReferenceNode *);

/* struct for rule classification */
typedef struct _ClassType
{
    char *type;      /* classification type */
    int id;          /* classification id */
    char *name;      /* "pretty" classification name */
    int priority;    /* priority */
    struct _ClassType *next;
} ClassType;

void ParseClassificationConfig(char *);

/* NOTE:  These lookups can only be done during parse time */
ClassType * ClassTypeLookupByType(struct _SnortConfig *, char *);
ClassType * ClassTypeLookupById(struct _SnortConfig *, int);

/*
 *  sid-gid -> otn mapping
 */
typedef struct _OtnKey
{
   uint32_t gid;
   uint32_t sid;

} OtnKey;

#define SI_RULE_FLUSHING_OFF 0
#define SI_RULE_FLUSHING_ON  1

#define SI_RULE_TYPE_DETECT  0
#define SI_RULE_TYPE_DECODE  1
#define SI_RULE_TYPE_PREPROC 2

#ifdef TARGET_BASED
typedef struct _ServiceInfo
{
    char *service;
    int16_t service_ordinal;
} ServiceInfo;

typedef enum _ServiceOverride {
    ServiceOverride_ElsePorts = 0,
    ServiceOverride_AndPorts,
    ServiceOverride_OrPorts,
    ServiceOverride_Nil
} ServiceOverride;
#endif

typedef struct _SigInfo
{
    uint32_t   generator;
    uint32_t   id;
    uint32_t   rev;
    uint32_t   class_id;
    ClassType   *classType;
    uint32_t   priority;
    const char        *message;
    ReferenceNode *refs;
    char          shared; /* shared object rule */
    char          dup_opt_func; /* has soid, and refers to another shared object rule */
    char          rule_type; /* 0-std rule, 1-decoder, rule, 3 preprocessor rule */
    char          rule_flushing; /* 0-disabled, 1-enabled */
    OtnKey otnKey;
#ifdef TARGET_BASED
    unsigned int num_services;
    ServiceInfo *services;
    ServiceOverride service_override;
#endif

#if defined(FEAT_OPEN_APPID)
    unsigned int num_appid;
#endif /* defined(FEAT_OPEN_APPID) */
} SigInfo;

SFGHASH * SoRuleOtnLookupNew(void);
void SoRuleOtnLookupAdd(SFGHASH *, struct _OptTreeNode *);
struct _OptTreeNode * SoRuleOtnLookup(SFGHASH *, uint32_t gid, uint32_t sid);
struct _OptTreeNode * SoRuleOtnLookupNext(uint32_t gid, uint32_t sid);
void SoRuleOtnLookupFree(SFGHASH *);

SFGHASH * OtnLookupNew(void);
void OtnLookupAdd(SFGHASH *, struct _OptTreeNode *);
struct _OptTreeNode * OtnLookup(SFGHASH *, uint32_t gid, uint32_t sid);
void OtnLookupFree(SFGHASH *);

void OtnRemove(SFGHASH *, SFGHASH *, struct _OptTreeNode *);
void OtnDeleteData(void *data);
void OtnFree(void *data);

static inline bool IsPreprocDecoderRule(char rule_type)
{
    if ((rule_type == SI_RULE_TYPE_DECODE)
            || (rule_type == SI_RULE_TYPE_PREPROC))
        return true;
    return false;
}

#endif /* SIGNATURE */
