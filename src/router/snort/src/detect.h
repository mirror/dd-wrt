/* $Id$ */
/*
** Copyright (C) 2002-2011 Sourcefire, Inc.
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
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/*  I N C L U D E S  ************************************************/
#ifndef __DETECT_H__
#define __DETECT_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.h"
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

/* rule match action functions */
int PassAction(void);
int ActivateAction(Packet *, OptTreeNode *, Event *);
int AlertAction(Packet *, OptTreeNode *, Event *);
int DropAction(Packet *, OptTreeNode *, Event *);
int SDropAction(Packet *, OptTreeNode *, Event *);
int DynamicAction(Packet *, OptTreeNode *, Event *);
int LogAction(Packet *, OptTreeNode *, Event *);

/* detection/manipulation funcs */
int Preprocess(Packet *);
int  Detect(Packet *);
void CallOutputPlugins(Packet *);
int EvalPacket(ListHead *, int, Packet * );
int EvalHeader(RuleTreeNode *, Packet *, int);
int EvalOpts(OptTreeNode *, Packet *);
void TriggerResponses(Packet *, OptTreeNode *);

#ifdef SUP_IP6
int CheckAddrPort(sfip_var_t *, PortObject* , Packet *, uint32_t, int);
#else
int CheckAddrPort(IpAddrSet *, PortObject* , Packet *, uint32_t, int);
#endif

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

void CallLogPlugins(Packet *, char *, void *, Event *);
void CallAlertPlugins(Packet *, char *, void *, Event *);
void CallLogFuncs(Packet *, char *, ListHead *, Event *);
void CallAlertFuncs(Packet *, char *, ListHead *, Event *);

static INLINE void DisableDetect(Packet *p)
{
    DisablePreprocessors(p);
    do_detect_content = 0;
}

static INLINE void DisableAllDetect(Packet *p)
{
    DisablePreprocessors(p);
    do_detect = do_detect_content = 0;
}

/* counter for number of times we evaluate rules.  Used to
 * cache result of check for rule option tree nodes. */
extern uint64_t rule_eval_pkt_count;


#endif /* __DETECT_H__ */
