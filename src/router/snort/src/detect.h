/* $Id$ */
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

/*  I N C L U D E S  ************************************************/
#ifndef __DETECT_H__
#define __DETECT_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "snort_debug.h"
#include "decode.h"
#include "rules.h"
#include "treenodes.h"
#include "parser.h"
#include "plugbase.h"
#include "log.h"
#include "event.h"
#include "sfutil/sfportobject.h"

/*  P R O T O T Y P E S  ******************************************************/
extern int do_detect;
extern int do_detect_content;
extern uint16_t event_id;

/* rule match action functions */
int PassAction(void);
int ActivateAction(Packet *, OptTreeNode *, RuleTreeNode *, Event *);
int AlertAction(Packet *, OptTreeNode *, RuleTreeNode *, Event *);
int DropAction(Packet *, OptTreeNode *, RuleTreeNode *, Event *);
int SDropAction(Packet *, OptTreeNode *, Event *);
int DynamicAction(Packet *, OptTreeNode *, RuleTreeNode *, Event *);
int LogAction(Packet *, OptTreeNode *, RuleTreeNode *, Event *);

/* detection/manipulation funcs */
int Preprocess(Packet *);
int  Detect(Packet *);
void CallOutputPlugins(Packet *);
int EvalPacket(ListHead *, int, Packet * );
int EvalHeader(RuleTreeNode *, Packet *, int);
int EvalOpts(OptTreeNode *, Packet *);
void TriggerResponses(Packet *, OptTreeNode *);

int CheckAddrPort(sfip_var_t *, PortObject* , Packet *, uint32_t, int);

/* detection modules */
int CheckBidirectional(Packet *, struct _RuleTreeNode *, RuleFpList *, int);
int CheckSrcIP(Packet *, struct _RuleTreeNode *, RuleFpList *, int);
int CheckDstIP(Packet *, struct _RuleTreeNode *, RuleFpList *, int);
int CheckSrcIPNotEq(Packet *, struct _RuleTreeNode *, RuleFpList *, int);
int CheckDstIPNotEq(Packet *, struct _RuleTreeNode *, RuleFpList *, int);
int CheckSrcPortEqual(Packet *, struct _RuleTreeNode *, RuleFpList *, int);
int CheckDstPortEqual(Packet *, struct _RuleTreeNode *, RuleFpList *, int);
int CheckSrcPortNotEq(Packet *, struct _RuleTreeNode *, RuleFpList *, int);
int CheckDstPortNotEq(Packet *, struct _RuleTreeNode *, RuleFpList *, int);

int RuleListEnd(Packet *, struct _RuleTreeNode *, RuleFpList *, int);
int OptListEnd(void *option_data, Packet *p);

void CallLogPlugins(Packet *, const char *, Event *);
void CallAlertPlugins(Packet *, const char *, Event *);
void CallLogFuncs(Packet *, const char *, ListHead *, Event *);
void CallAlertFuncs(Packet *, const char *, ListHead *, Event *);

static inline void DisableDetect( Packet *p )
{
    DisableAppPreprocessors( p );
    do_detect_content = 0;
}

static inline void DisableAllDetect( Packet *p )
{
    DisableAppPreprocessors( p );
    do_detect = do_detect_content = 0;
}

static inline void EnableContentDetect( void )
{
    do_detect_content = 1;
}

static inline void DisablePacketAnalysis( Packet *p )
{
    DisableAllPreprocessors ( p );
    do_detect = do_detect_content = 0;
}

static inline void EnableContentPreprocDetection( Packet *p, PreprocEnableMask enabled_pps )
{
    EnableContentDetect();
    EnablePreprocessors( p, enabled_pps );
}

/* counter for number of times we evaluate rules.  Used to
 * cache result of check for rule option tree nodes. */
extern uint64_t rule_eval_pkt_count;


#endif /* __DETECT_H__ */
