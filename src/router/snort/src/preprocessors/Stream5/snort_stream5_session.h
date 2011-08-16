/****************************************************************************
 *
 * Copyright (C) 2005-2011 Sourcefire, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************/
 
#ifndef SNORT_STREAM5_SESSION_H_
#define SNORT_STREAM5_SESSION_H_

#include "sfxhash.h"
#include "stream5_common.h"
#include "rules.h"
#include "treenodes.h"

typedef void(*Stream5SessionCleanup)(Stream5LWSession *ssn);

typedef struct _Stream5SessionCache
{
    SFXHASH *hashTable;
    uint32_t timeout;
    uint32_t max_sessions;
    uint32_t cleanup_sessions;
    uint32_t prunes;
    Stream5SessionCleanup cleanup_fcn;
} Stream5SessionCache;

/**list of ignored rules.
 */
typedef struct _IgnoredRuleList
{
    OptTreeNode *otn;
    struct _IgnoredRuleList *next;
} IgnoredRuleList;

#if 0
void PrintSessionKey(SessionKey *);
#endif

Stream5SessionCache *InitLWSessionCache(int max_sessions,
                                        uint32_t session_timeout,
                                        uint32_t cleanup_sessions,
                                        uint32_t cleanup_percent,
                                        Stream5SessionCleanup clean_fcn);
Stream5LWSession *GetLWSession(Stream5SessionCache *, Packet *, SessionKey *);
Stream5LWSession *GetLWSessionFromKey(Stream5SessionCache *, SessionKey *);
Stream5LWSession *NewLWSession(Stream5SessionCache *, Packet *, SessionKey *, void *);
int DeleteLWSession(Stream5SessionCache *, Stream5LWSession *, char *reason);
void PrintLWSessionCache(Stream5SessionCache *);
int DeleteLWSessionCache(Stream5SessionCache *sessionCache);
int PurgeLWSessionCache(Stream5SessionCache *);
int PruneLWSessionCache(Stream5SessionCache *,
                      uint32_t thetime,
                      Stream5LWSession *save_me,
                      int memcheck);
int GetLWSessionCount(Stream5SessionCache *);
void GetLWPacketDirection(Packet *p, Stream5LWSession *ssn);
void FreeLWApplicationData(Stream5LWSession *ssn);
void setPortFilterList(
        uint8_t *portList, 
        int isUdp,
        int ignoreAnyAnyRules,
        tSfPolicyId policyId
        );
int Stream5AnyAnyFlow(
        uint8_t *portList, 
        OptTreeNode *otn,
        RuleTreeNode *rtn, 
        int any_any_flow,
        IgnoredRuleList **ppIgnoredRuleList,
        int ignoreAnyAnyRules
        );
void s5PrintPortFilter(
        uint8_t portList[]
        );
int
Stream5SetRuntimeConfiguration(
        Stream5LWSession *lwssn,
        char protocol
        );

#endif /* SNORT_STREAM5_SESSION_H_ */

