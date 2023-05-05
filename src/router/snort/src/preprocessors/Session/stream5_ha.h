/* $Id$ */
/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2004-2013 Sourcefire, Inc.
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

/**************************************************************************
 *
 * stream5_ha.h
 *
 * Authors: Michael Altizer <maltizer@sourcefire.com>, Russ Combs <rcombs@sourcefire.com>
 *
 * Description:
 *
 * Stream high availability exported functionality.
 *
 **************************************************************************/

#ifndef __STREAM_HA_H__
#define __STREAM_HA_H__

#ifdef ENABLE_HA

#include "sf_types.h"
#include "session_common.h"
#include "snort_session.h"

typedef enum
{
    HA_EVENT_UPDATE,
    HA_EVENT_DELETE,
    HA_EVENT_MAX
} HA_Event;

typedef SessionControlBlock *(*f_ha_create_session) (const SessionKey *);
typedef int (*f_ha_delete_session) (const SessionKey *);
typedef SessionControlBlock *(*f_ha_get_lws) (const SessionKey *);
typedef void (*f_ha_deactivate_session) (SessionControlBlock *);

typedef struct
{
    f_ha_get_lws get_lws;
    f_ha_create_session create_session;
    f_ha_deactivate_session deactivate_session;
    f_ha_delete_session delete_session;
} HA_Api;

extern int ha_set_api(unsigned proto, const HA_Api *);

// Used with StreamLWSession.ha_flags:
#define HA_FLAG_STANDBY         0x01    // session is not active
#define HA_FLAG_NEW             0x02    // flow has never been synchronized
#define HA_FLAG_MODIFIED        0x04    // session HA state information has been modified
#define HA_FLAG_MAJOR_CHANGE    0x08    // session HA state information has been modified in a major fashion
#define HA_FLAG_CRITICAL_CHANGE 0x10    // session HA state information has been modified in a critical fashion
#define HA_FLAG_DELETED         0x20    // flow deletion message has been sent

static inline void ha_track_modify(SessionControlBlock *lws)
{
    lws->ha_flags |= HA_FLAG_MODIFIED;
}

int RegisterSessionHAFuncs(uint32_t preproc_id, uint8_t subcode, uint8_t size,
                            StreamHAProducerFunc produce, StreamHAConsumerFunc consume);
void UnregisterSessionHAFuncs(uint32_t preproc_id, uint8_t subcode);
void SessionSetHAPendingBit(void *ssnptr, int bit);

void SessionHAInit(struct _SnortConfig *sc, char *args);
void SessionHAReload(struct _SnortConfig *sc, char *args, void **new_config);
int SessionVerifyHAConfig(struct _SnortConfig *sc, void *swap_config);
void *SessionHASwapReload( struct _SnortConfig *sc, void *data );
void SessionHAConfigFree( void *data );

void SessionHAPostConfigInit(struct _SnortConfig *sc, int unused, void *arg);
void SessionCleanHA(void);
void SessionPrintHAStats(void);
void SessionResetHAStats(void);
void SessionProcessHA(void *ssnptr, const DAQ_PktHdr_t *pkthdr);
void SessionHANotifyDeletion(SessionControlBlock *scb);
#ifdef HAVE_DAQ_QUERYFLOW
#ifdef REG_TEST
int SessionHAQueryDAQState( DAQ_PktHdr_t *pkthdr);
#else
int SessionHAQueryDAQState(const  DAQ_PktHdr_t *pkthdr);
#endif
#endif

#endif /* ENABLE_HA */

#endif /* __STREAM_HA_H__ */
