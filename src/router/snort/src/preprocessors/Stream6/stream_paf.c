/* $Id$ */
/****************************************************************************
 *
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
 ****************************************************************************/

//--------------------------------------------------------------------
// s5 stuff
//
// @file    stream5_paf.c
// @author  Russ Combs <rcombs@sourcefire.com>
//--------------------------------------------------------------------

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sf_types.h"
#include "snort_bounds.h"
#include "snort_debug.h"
#include "snort.h"
#include "memory_stats.h"
#include "sfPolicyUserData.h"
#include "stream_common.h"
#include "stream_paf.h"
#include "sftarget_protocol_reference.h"
#include "encode.h"

//--------------------------------------------------------------------
// private state
//--------------------------------------------------------------------
//
uint16_t global_mask = 0;

typedef enum {
    FT_NOP,  // no flush
    FT_SFP,  // abort paf
    FT_PAF,  // flush to paf pt when len >= paf
    FT_LIMIT, // flush to paf. flags are not updated
    FT_MAX,   // flush len when len >= mfp
    FT_DISC_S,// start of discard
    FT_DISC_E, // End of discard
    FT_PAF_HDR,
    FT_PSEUDO_FLUSH
} FlushType;

typedef struct {
    uint16_t cb_mask;
    uint8_t auto_on;
} PAF_Map;

typedef struct {
    uint32_t mfp;

    uint32_t prep_calls;
    uint32_t prep_bytes;

    PAF_Map port_map[MAXPORTS][2];
    PAF_Map service_map[MAX_PROTOCOL_ORDINAL][2];
} PAF_Config;

#define PAF_LIMIT_FUZZ ETHERNET_MTU

// for cb registration
#define MAX_CB 16  // depends on sizeof(PAF_Map.cb_mask)
static PAF_Callback s5_cb[MAX_CB];
static uint8_t s5_cb_idx = 0;
static PAF_Free_Callback s5_free_cb[MAX_CB];

// s5_len and s5_idx are used only during the
// lifetime of s5_paf_check()
static uint32_t s5_len;  // total bytes queued
static uint32_t s5_idx;  // offset from start of queued bytes

//--------------------------------------------------------------------

static uint32_t s5_paf_flush (
    PAF_Config* pc, PAF_State* ps, FlushType ft, uint64_t* flags)
{
    uint32_t at = 0;
    *flags &= ~(PKT_PDU_HEAD | PKT_PDU_TAIL);

    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
       "%s: type=%d, fpt=%u, len=%u, tot=%u\n",
        __FUNCTION__, ft, ps->fpt, s5_len, ps->tot);)

    switch ( ft )
    {
    case FT_PAF_HDR:
        at = ps->fpt_eoh;
        *flags |= PKT_PDU_TAIL;
        break;
    case FT_NOP:
        return 0;

    case FT_SFP:
        *flags = 0;
        return 0;

    case FT_PAF:
        at = ps->fpt;
        *flags |= PKT_PDU_TAIL;
        break;

    case FT_PSEUDO_FLUSH:
        at = s5_len;
        break;

    case FT_DISC_S:
        at = s5_len;
        ps->mode = FLUSH_MODE_DISCARD;
        if ( ps->fpt > s5_len )
            ps->fpt -= s5_len;
        else
            ps->fpt = 0;
        break;

    case FT_DISC_E:
        at = ps->fpt;
        ps->mode = FLUSH_MODE_NORMAL;
        *flags |= PKT_IGNORE | PKT_PDU_TAIL; //overloading existing flag
        break;

    case FT_LIMIT:
        if (ps->fpt > s5_len)
        {
            at = s5_len;
            ps->fpt -= s5_len;
        }
        else
        {
            at = ps->fpt;
            ps->fpt = s5_len - ps->fpt; // number of characters scanned but not flushing
        }
        break;

    // use of s5_len is suboptimal here because the actual amount
    // flushed is determined later and can differ in certain cases
    // such as exceeding s5_pkt->max_dsize.  the actual amount
    // flushed would ideally be applied to ps->fpt later.  for
    // now we try to circumvent such cases so we track correctly.
    case FT_MAX:
        at = s5_len;
        if( ps->mode == FLUSH_MODE_DISCARD )
            *flags |= PKT_IGNORE;
        if ( ps->fpt > s5_len )
            ps->fpt -= s5_len;
        else
            ps->fpt = 0;
        break;
    }

    if ( !at || !s5_len )
        return 0;

    // safety - prevent seq + at < seq
    if ( at > 0x7FFFFFFF )
        at = 0x7FFFFFFF;

    if ( !ps->tot )
        *flags |= PKT_PDU_HEAD;
	
    if (ft == FT_PSEUDO_FLUSH)
        return at;

    if ( *flags & PKT_PDU_TAIL )
        ps->tot = 0;
    else
        ps->tot += at;

    if(!ps->fpt_eoh)
    {
        ps->pos += at;
        ps->seq = ps->pos;
    }
    return at;
}

//--------------------------------------------------------------------
static inline int8_t s5_paf_user_data_index(PAF_State* ps, uint8_t id)
{
    int8_t i;
   //use id and iterate over ps.cb_id[]. compare. if matching return index
    for( i = 0; i < MAX_PAF_USER; i++ )
    {
      // Check ch_id[i] against (id+1)
      if( ps->cb_id[i] == (id + 1) )
          return i;
    }

   //if id doesnt match and user slot is available, this is new PAF call
   //use the free slot and return its index
    for( i = 0; i < MAX_PAF_USER; i++ )
    {
        if(ps->user[i] == NULL)
          return i;
    }

    return -1;
}

//--------------------------------------------------------------------
int8_t s5_paf_get_user_data_index(PAF_State* ps, uint8_t id)
{
   return s5_paf_user_data_index(ps, id-1);
}
//--------------------------------------------------------------------

static bool s5_paf_callback (
    PAF_State* ps, void* ssn,
    const uint8_t* data, uint32_t len, uint64_t *flags)
{
    PAF_Status paf = PAF_ABORT;
    uint16_t mask = ps->cb_mask;
    bool update = false;
    int i = 0;
    int8_t udata_idx;

    while ( mask )
    {
        uint16_t bit = (1<<i);
        if ( bit & mask )
        {
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
                "%s: mask=%d, i=%u\n", __FUNCTION__, mask, i);)
            udata_idx = s5_paf_user_data_index(ps, i);

            if(udata_idx == -1)
            {
                if ( ++i == MAX_CB )
                    break;
                else
                    continue;
            }


           // Assign (id+1) to cb_id[i] to ensure cb_id[i] = 0 indicates only unintialized value
            ps->cb_id[udata_idx] = ( i + 1 ) ;
            paf = s5_cb[i](ssn, &ps->user[udata_idx], data, len, flags, &ps->fpt, &ps->fpt_eoh);

            if ( paf == PAF_ABORT )
            {
                // this one bailed out
                ps->cb_mask ^= bit;
            }
            else if (paf != PAF_SEARCH)
            {
                // this one selected
                if (!(*flags & PKT_UPGRADE_PROTO))
                    ps->cb_mask = bit;

                update = true;
                break;
            }
            mask ^= bit;
        }
        if ( ++i == MAX_CB )
            break;
    }
    if ( !ps->cb_mask )
    {
        ps->paf = PAF_ABORT;
        update = true;
    }
    else if ( paf != PAF_ABORT )
    {
        ps->paf = paf;
    }
    if ( update )
    {
        ps->fpt += s5_idx;

        if ( ps->fpt <= s5_len )
        {
            s5_idx = ps->fpt;
            return true;
        }
        if(ps->fpt_eoh && (paf != PAF_SKIP)){
            ps->fpt_eoh += s5_idx;
            return true;
        }
    }
    s5_idx = s5_len;
    return false;
}

//--------------------------------------------------------------------

static inline bool s5_paf_eval (
    PAF_Config* pc, PAF_State* ps, void* ssn,
    uint16_t port, uint64_t *flags, uint32_t fuzz,
    const uint8_t* data, uint32_t len, FlushType* ft)
{
    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
        "%s: paf=%d, idx=%u, len=%u, fpt=%u\n",
        __FUNCTION__, ps->paf, s5_idx, s5_len, ps->fpt);)

    switch ( ps->paf )
    {
    case PAF_SEARCH:
        if ( s5_len > s5_idx )
        {
            return s5_paf_callback(ps, ssn, data, len, flags);
        }
        return false;

    case PAF_FLUSH:
    case PAF_PSEUDO_FLUSH_SEARCH:
        if ( s5_len >= ps->fpt )
        {
            *ft = FT_PAF;
            ps->paf = PAF_SEARCH;
            return true;
        }
        if ( s5_len >= pc->mfp + fuzz )
        {
            *ft = FT_MAX;
            return false;
        }
        if(ps->fpt_eoh){
            *ft = FT_PAF_HDR;
            return true;
        }
        return false;

    case PAF_LIMIT:
        // if we are within PAF_LIMIT_FUZZ character of paf_max ...
        if ( s5_len + PAF_LIMIT_FUZZ >= pc->mfp + fuzz)
        {
            *ft = FT_LIMIT;
            ps->paf = PAF_PERFORMED_LMT_FLUSH;
            return false;
        }
        ps->paf = PAF_SEARCH;
        return false;

    case PAF_SKIP:
    case PAF_PSEUDO_FLUSH_SKIP:
        if ( s5_len > ps->fpt )
        {
            bool ret;

            if ( ps->fpt > s5_idx )
            {
                uint32_t delta = ps->fpt - s5_idx;
                if ( delta > len )
                    return false;
                data += delta;
                len -= delta;
            }
            s5_idx = ps->fpt;
            ps->paf = PAF_SKIP;
            ret =  s5_paf_callback(ps, ssn, data, len, flags);
            if (ps->paf == PAF_PSEUDO_FLUSH_SKIP)
            {
                /*
                 * If we have processed entire data, check if we
                 * can prepare for pseudo flush.
                 */
                if (!ret &&  (s5_len < ps->fpt))
                {
                    *ft = FT_PSEUDO_FLUSH;
                }
            }
            return ret;
        }
        return false;

    case PAF_PERFORMED_LMT_FLUSH:
        // increment position by previously scanned bytes. set in s5_paf_flush
        ps->paf = PAF_SEARCH;
        s5_idx += ps->fpt;
        ps->fpt = 0;
        return true;

    case PAF_DISCARD_START:
        /*
         * We can flush at this point,
         * but this will result in splitting of segments
         * so we will try flushing at s5_len if PDU end
         * doesnt come before.
         */
        if( s5_len ==  ps->fpt )
        {
            *ft = FT_DISC_S;
            ps->paf = PAF_SEARCH;
            return true;
        }
        else if( s5_len > ps->fpt )
        {
            ps->mode = FLUSH_MODE_PRE_DISCARD;
            ps->paf = PAF_SEARCH;
            s5_idx = ps->fpt;
            return true;
        }

        if ( s5_len >= pc->mfp + fuzz )
        {
            *ft = FT_MAX;
            return false;
        }
        return false;
    case PAF_DISCARD_END:
        if( s5_len >= ps->fpt )
        {
            if( ps->mode != FLUSH_MODE_DISCARD )
            {
                ps->mode = FLUSH_MODE_NORMAL;
                *ft = FT_PAF;
            }
            else
                *ft = FT_DISC_E;
            ps->paf = PAF_SEARCH;
            return true;
        }
        if ( s5_len >= pc->mfp + fuzz )
        {
            /*
             * If mode is set to PRE_DISCARD,
             * we will anyway flush at s5_len
             * so no need to set to FT_MAX
             */
            if( ps->mode != FLUSH_MODE_PRE_DISCARD )
            {
                *ft = FT_MAX;
                return false;
            }
        }
        return false;
    case PAF_IGNORE:
    {
        return false;
    }
    default:
        // PAF_ABORT || PAF_START
        ps->mode = FLUSH_MODE_NORMAL;
        break;
    }

    *ft = FT_SFP;
    return false;
}

//--------------------------------------------------------------------
// helper functions

static PAF_Config* get_config (struct _SnortConfig *sc, tSfPolicyId pid)
{
    StreamConfig *config;

    config = getStreamPolicyConfig( pid, true );
    if ( ( config != NULL ) && ( config->tcp_config != NULL ) )
        return config->tcp_config->paf_config;
    else
        return NULL;
}

static int install_callback (PAF_Callback cb)
{
    int i;

    for ( i = 0; i < s5_cb_idx; i++ )
    {
        if ( s5_cb[i] == cb )
            break;
    }
    if ( i == MAX_CB )
        return -1;

    if ( i == s5_cb_idx )
    {
        s5_cb[i] = cb;
        s5_cb_idx++;
    }
    return i;
}

//--------------------------------------------------------------------
// public stuff
//--------------------------------------------------------------------

void s5_paf_setup (PAF_State* ps, uint16_t mask)
{
    // this is already cleared when instantiated
    //memset(ps, 0, sizeof(*ps));
    ps->paf = PAF_START;
    ps->cb_mask = mask;
    ps->mode = FLUSH_MODE_NORMAL;
}

void s5_paf_clear (PAF_State* ps)
{
    int i;

    for(i = 0; i < MAX_PAF_USER; i++)
    {
        if ( ps->user[i] )
        {
            if (s5_free_cb[ps->cb_id[i]])
            {
                s5_free_cb[ps->cb_id[i]](ps->user[i]);
            }
            else
            {
                free(ps->user[i]);
            }
            ps->user[i] = NULL;
        }
    }
    ps->paf = PAF_ABORT;
}

//--------------------------------------------------------------------

uint32_t s5_paf_check (
    void* pv, PAF_State* ps, void* ssn,
    const uint8_t* data, uint32_t len, uint32_t total,
    uint32_t seq, uint16_t port, uint64_t* flags, uint32_t fuzz)
{
    PAF_Config* pc = pv;

    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
        "%s: len=%u, amt=%u, seq=%u, cur=%u, pos=%u, fpt=%u, tot=%u, paf=%d\n",
        __FUNCTION__, len, total, seq, ps->seq, ps->pos, ps->fpt, ps->tot, ps->paf);)

    if ( !s5_paf_initialized(ps) )
    {
        ps->seq = ps->pos = seq;
        ps->paf = PAF_SEARCH;
    }
    else if (*flags & PKT_PSEUDO_FLUSH)
    {
        return s5_paf_flush(pc, ps, FT_PSEUDO_FLUSH, flags);
    }
    else if ( SEQ_GT(seq, ps->seq) )
    {
        // if seq jumped we have a gap.  Flush any queued data, then abort
        s5_len = total - len;

        if(s5_len)
        {
            ps->fpt = 0;
            return s5_paf_flush(pc, ps, FT_MAX, flags);
        }
        *flags = 0;
        return 0;
    }
    else if ( SEQ_LEQ(seq + len, ps->seq) )
    {
        return 0;
    }
    else if ( SEQ_LT(seq, ps->seq) )
    {
        uint32_t shift = ps->seq - seq;
        data += shift;
        len -= shift;
    }
    ps->fpt_eoh = 0;
    ps->seq += len;

    pc->prep_calls++;
    pc->prep_bytes += len;

    s5_idx = total - len;

    // if 'total' is greater than the maximum paf_max AND 'total' is greater
    // than paf_max bytes + fuzz (i.e. after we have finished analyzing the
    // current segment, total bytes analyzed will be greater than the
    // configured (fuzz + paf_max) == (pc->mfp + fuzz), we must ensure a flush
    // occurs at the paf_max byte.  So, we manually set the data's length and
    // total queued bytes (s5_len) to guarantee that at most paf_max bytes will
    // be analyzed and flushed since the last flush point.  It should also be
    // noted that we perform the check here rather in in s5_paf_flush() to
    // avoid scanning the same data twice. The first scan would analyze the
    // entire segment and the second scan would analyze this segments
    // unflushed data.
    if ( ps->mode == FLUSH_MODE_NORMAL &&
            total >= MAXIMUM_PAF_MAX && total > pc->mfp + fuzz )
    {
        s5_len = MAXIMUM_PAF_MAX + fuzz;
        len = (len > ( total - s5_len )) ? len + s5_len - total : 0;
        if( !len )
            return 0;
    }
    else
    {
        s5_len = total;
    }

    do {
        FlushType ft = FT_NOP;
        uint32_t idx = s5_idx;
        uint32_t shift;

        bool cont = s5_paf_eval(pc, ps, ssn, port, flags, fuzz, data, len, &ft);

        if (ps->paf == PAF_IGNORE)
            break;

        if ( ft != FT_NOP )
            return s5_paf_flush(pc, ps, ft, flags);

        if ( !cont )
            break;

        if ( s5_idx > idx )
        {
            shift = s5_idx - idx;
            if ( shift > len )
                shift = len;
            data += shift;
            len -= shift;
        }

    } while ( 1 );

    //Added for Http/2
    if (ps->paf == PAF_IGNORE)
    {
        *flags |= PKT_PURGE;
        ps->paf = PAF_SEARCH;
        return 0;
    }
    //start discarding from now on
    if(ps->mode == FLUSH_MODE_PRE_DISCARD)
        return s5_paf_flush(pc, ps, FT_DISC_S, flags);
    else if ( (s5_len >= pc->mfp+fuzz) )
        return s5_paf_flush(pc, ps, FT_MAX, flags);

    return 0;
}

//--------------------------------------------------------------------
// port registration foo

uint8_t s5_paf_register_port (struct _SnortConfig *sc,
    tSfPolicyId pid, uint16_t port, bool c2s, PAF_Callback cb, bool auto_on)
{
    PAF_Config* pc = get_config(sc, pid);
    int i, dir = c2s ? 1 : 0;

    if ( !pc )
        return 0;

    i = install_callback(cb);

    if ( i < 0 )
        return 0;

    pc->port_map[port][dir].cb_mask |= (1<<i);

    if ( !pc->port_map[port][dir].auto_on )
        pc->port_map[port][dir].auto_on = (uint8_t)auto_on;

    return i+1;
}

uint16_t s5_paf_port_registration (void* pv, uint16_t port, bool c2s, bool flush)
{
    PAF_Config* pc = pv;
    PAF_Map* pm;

    if ( !pc )
        return false;

    pm = pc->port_map[port] + (c2s?1:0);

    if ( !pm->cb_mask )
        return 0;
    if ( pm->auto_on || flush )
        return pm->cb_mask;

    return 0;
}

uint16_t s5_paf_port_registration_all (void* pv, uint16_t port, bool c2s, bool flush)
{
    PAF_Config* pc = pv;
    PAF_Map* pm;

    if ( !pc )
        return false;

    pm = pc->port_map[port] + (c2s?1:0);

    if ( !pm->cb_mask )
        pm->cb_mask = global_mask;

    if ( pm->auto_on || flush )
        return pm->cb_mask;

    return 0;
}

//--------------------------------------------------------------------
// service registration foo

uint8_t s5_paf_register_service (struct _SnortConfig *sc,
    tSfPolicyId pid, uint16_t service, bool c2s, PAF_Callback cb, bool auto_on)
{
    PAF_Config* pc = get_config(sc, pid);
    int i, dir = c2s ? 1 : 0;

    if ( !pc )
        return 0;

    i = install_callback(cb);

    if ( i < 0 )
        return 0;

    pc->service_map[service][dir].cb_mask |= (1<<i);
    global_mask |= (1<<i);

    if ( !pc->service_map[service][dir].auto_on )
        pc->service_map[service][dir].auto_on = (uint8_t)auto_on;

    return i+1;
}

/*  cb_mask_cmp
 *
 *  This api compares expected cb_mask w.r.t appid
 *  and cb_mask populated in stream paf_state.
 *
 *  Input  : PAF_Config - Paf configurations.
 *           Service    - Service id like for HTTP id is 5.
 *           c2s        - True  - This represents flow is client to server.
 *                      - False - This represents flow is server to client.
 *           cb_mask    - cb_mask populated in stream paf_state 
 *
 *  Return : -1 - If PAF_Config is not present.
 *            0 - If both the cb_mask matches.
 *            1 - If both the cb_mask does not match.
 */            
int cb_mask_cmp(void* pv, uint16_t service, bool c2s, uint16_t cb_mask)
{
    PAF_Config* pc = (PAF_Config*)pv;
    PAF_Map* pm;

    if ( !pc )
        return -1;

    pm = pc->service_map[service] + (c2s?1:0);
    if (pm && pm->cb_mask && (pm->cb_mask != cb_mask))
        return 1;

    return 0;
}

uint16_t s5_paf_service_registration (void* pv, uint16_t service, bool c2s, bool flush)
{
    PAF_Config* pc = pv;
    PAF_Map* pm;

    if ( !pc )
        return false;

    pm = pc->service_map[service] + (c2s?1:0);
    if ( !pm->cb_mask )
        return 0;

    if ( pm->auto_on || flush )
        return pm->cb_mask;

    return 0;
}

//--------------------------------------------------------------------

void* s5_paf_new (void)
{
    PAF_Config* pc = SnortPreprocAlloc(1, sizeof(*pc), PP_STREAM, 
                         PP_MEM_CATEGORY_CONFIG);
    assert( pc );

    pc->mfp = ScPafMax();

    if ( !pc->mfp )
        // this ensures max < IP_MAXPACKET
        pc->mfp = MAXIMUM_PAF_MAX;

    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
        "%s: mfp=%u\n",
        __FUNCTION__, pc->mfp);)

    return pc;
}

void s5_paf_delete (void* pv)
{
    PAF_Config* pc = (PAF_Config*)pv;

    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
        "%s: prep=%u/%u\n",  __FUNCTION__,
        pc->prep_calls, pc->prep_bytes);)

    SnortPreprocFree(pc, sizeof(*pc), PP_STREAM, PP_MEM_CATEGORY_CONFIG);
}

void s5_paf_register_free (uint8_t id, PAF_Free_Callback cb)
{
    s5_free_cb[id] = cb;
}
