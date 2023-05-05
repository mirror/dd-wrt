/*
** $Id$
**
** pcrm.h
**
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Marc Norton <mnorton@sourcefire.com>
** Dan Roelker <droelker@sourcefire.com>
**
** 
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
**
** Packet Classification-Rule Manager
**
*/
#ifndef _PCRM_H
#define _PCRM_H

#include "decode.h"
#include "bitop.h"

typedef void * RULE_PTR;

#define ANYPORT   (-1)


/*
** Macros to walk a RULE_NODE list, and get the
** RULE_PTR from a RULE_NODE, these eliminate 
** subroutine calls, in high performance needs.
*/
#define PRM_GET_FIRST_GROUP_NODE(pg) (pg->pgHead)
#define PRM_GET_NEXT_GROUP_NODE(rn)  (rn->rnNext)

#define PRM_GETRULE_FROM_NODE(rn)     (rn->rnRuleData)

#define PRM_GET_FIRST_GROUP_NODE_NC(pg) (pg->pgHeadNC)
#define PRM_GET_NEXT_GROUP_NODE_NC(rn)  (rn->rnNext)

typedef enum _PmType
{
    PM_TYPE__CONTENT = 0,
    PM_TYPE__HTTP_URI_CONTENT,
    PM_TYPE__HTTP_HEADER_CONTENT,
    PM_TYPE__HTTP_CLIENT_BODY_CONTENT,
    PM_TYPE__HTTP_METHOD_CONTENT,
    PM_TYPE__MAX

} PmType;

typedef struct _not_rule_node_ {

  struct _not_rule_node_ * next;
  
  int iPos; /* RULE_NODE->iRuleNodeID */
  
  
} NOT_RULE_NODE;


typedef struct _rule_node_ {

  struct  _rule_node_ * rnNext;
 
  RULE_PTR rnRuleData; 
  int iRuleNodeID;
 
}RULE_NODE;

typedef struct {
  
  /* Content List */
  RULE_NODE *pgHead, *pgTail, *pgCur;
  int   pgContentCount;
 
  /* No-Content List */
  RULE_NODE *pgHeadNC, *pgTailNC, *pgCurNC;
  int   pgNoContentCount;

  /*  Uri-Content List */
  RULE_NODE *pgUriHead, *pgUriTail, *pgUriCur;
  int   pgUriContentCount;
 
  /* Pattern Matching data structures (MPSE) */
  void *pgPms[PM_TYPE__MAX];

  /* detection option tree */
  void *pgNonContentTree;
  
  int avgLen;  
  int minLen;
  int maxLen;
  int c1,c2,c3,c4,c5;

  /*
  *   Not rule list for this group
  */
  NOT_RULE_NODE *pgNotRuleList;

  /*
  **  Count of rule_node's in this group/list 
  */
  int pgCount;

  int pgNQEvents;
  int pgQEvents;
 
}PORT_GROUP;



typedef struct {

  int        prmNumDstRules;
  int        prmNumSrcRules;
  int        prmNumGenericRules;
#ifdef TARGET_BASED
  int        prmNumNoServiceDstRules;
  int        prmNumNoServiceSrcRules;
#endif
  
  int        prmNumDstGroups;
  int        prmNumSrcGroups;
#ifdef TARGET_BASED
  int        prmNumNoServiceDstGroups;
  int        prmNumNoServiceSrcGroups;
#endif

  PORT_GROUP *prmDstPort[MAX_PORTS];
  PORT_GROUP *prmSrcPort[MAX_PORTS];
  /* char       prmConflicts[MAX_PORTS]; */
#ifdef TARGET_BASED
  PORT_GROUP *prmNoServiceDstPort[MAX_PORTS];
  PORT_GROUP *prmNoServiceSrcPort[MAX_PORTS];
#endif
  PORT_GROUP *prmGeneric;

} PORT_RULE_MAP ;


typedef struct {

  int        prmNumRules;
  int        prmNumGenericRules;
  
  int        prmNumGroups;

  PORT_GROUP prmByteGroup[256];
  PORT_GROUP prmGeneric;

} BYTE_RULE_MAP ;


PORT_RULE_MAP * prmNewMap(void);
BYTE_RULE_MAP * prmNewByteMap(void);

void prmFreeMap( PORT_RULE_MAP * p );
void prmFreeByteMap( BYTE_RULE_MAP * p );

int prmxAddPortRule( PORT_GROUP * p, RULE_PTR rd );
int prmxAddPortRuleUri( PORT_GROUP * p, RULE_PTR rd );
int prmxAddPortRuleNC( PORT_GROUP * p, RULE_PTR rd );

int prmAddRule( PORT_RULE_MAP * p, int dport, int sport, RULE_PTR rd );
int prmAddByteRule( BYTE_RULE_MAP * p, int dport, RULE_PTR rd );

int prmAddRuleUri( PORT_RULE_MAP * p, int dport, int sport, RULE_PTR rd );
int prmAddRuleNC( PORT_RULE_MAP * p, int dport, int sport, RULE_PTR rd );
int prmAddByteRuleNC( BYTE_RULE_MAP * p, int dport, RULE_PTR rd );

void prmAddNotNode( PORT_GROUP * pg, int id );

int prmCompileGroups( PORT_RULE_MAP * p );
int prmCompileByteGroups( BYTE_RULE_MAP * p );

int prmShowStats( PORT_RULE_MAP * p );
int prmShowByteStats( BYTE_RULE_MAP * p );

int prmShowEventStats( PORT_RULE_MAP * p );
int prmShowEventByteStats( BYTE_RULE_MAP * p );

RULE_PTR prmGetFirstRule( PORT_GROUP * pg );
RULE_PTR prmGetNextRule( PORT_GROUP * pg );

RULE_PTR prmGetFirstRuleUri( PORT_GROUP * pg );
RULE_PTR prmGetNextRuleUri( PORT_GROUP * pg );

RULE_PTR prmGetFirstRuleNC( PORT_GROUP * pg );
RULE_PTR prmGetNextRuleNC( PORT_GROUP * pg );


#ifdef TARGET_BASED
// Get all the port groups
int prmFindRuleGroup(
        PORT_RULE_MAP*,
        int dest_port,
        int source_port,
        PORT_GROUP** source,
        PORT_GROUP** dest,
        PORT_GROUP** source_noservice,
        PORT_GROUP** dest_noservice,
        PORT_GROUP** anyany);
// Return only the noservice port groups
int prmFindNoServiceRuleGroup(
        PORT_RULE_MAP*,
        int dest_port,
        int source_port,
        PORT_GROUP** source_noservice,
        PORT_GROUP** dest_noservice,
        PORT_GROUP** anyany);
#else
// Get all the port groups
int prmFindRuleGroup(
        PORT_RULE_MAP * p,
        int dest_port,
        int source_port,
        PORT_GROUP** src,
        PORT_GROUP** dst,
        PORT_GROUP** gen);
#endif
int prmFindByteRuleGroup( BYTE_RULE_MAP * p, int dport, PORT_GROUP **dst , PORT_GROUP ** gen);
int prmFindGenericRuleGroup(PORT_RULE_MAP *prm, PORT_GROUP ** gen);

PORT_GROUP * prmFindDstRuleGroup( PORT_RULE_MAP * p, int port );
PORT_GROUP * prmFindSrcRuleGroup( PORT_RULE_MAP * p, int port );

PORT_GROUP * prmFindByteRuleGroupUnique( BYTE_RULE_MAP * p, int port );

int      prmSetGroupPatData( PORT_GROUP * pg, void * data );
void *   prmGetGroupPatData( PORT_GROUP * pg );


#endif
