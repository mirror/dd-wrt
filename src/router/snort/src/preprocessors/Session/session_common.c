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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************/

#include "session_common.h"
#include "snort_session.h"

#ifdef ENABLE_HA
#include "stream5_ha.h"
#endif

// table of function pointers for functionality provided by other preprocs
// and used within session
static struct session_plugins *pfunks = NULL;


int SessionTrackingEnabled( SessionConfiguration *config, uint32_t protocol ) 
{
    if( config->disabled )
        return false;

    switch ( protocol )
    {
    case SESSION_PROTO_TCP:
        return config->track_tcp_sessions;
        break;

    case SESSION_PROTO_UDP:
        return config->track_udp_sessions;
        break;

    case SESSION_PROTO_ICMP:
        return config->track_icmp_sessions;
        break;
         
    case SESSION_PROTO_IP:
        return config->track_ip_sessions;
        break;
         
    default:
        LogMessage( "WARNING: Invalid Protocol %u in session tracking enabled request\n", protocol );
        return false;
        break;
    }
}

uint32_t GetSessionPruneLogMax( void )
{
    return session_configuration->prune_log_max;
}

uint32_t GetSessionMemCap( void )
{
    return session_configuration->memcap;

}

void SessionFreeConfig( SessionConfiguration *config )
{
    if( config == NULL )
        return;

#ifdef ENABLE_HA
    if( config->ha_config != NULL )
    {
        SessionHAConfigFree( config->ha_config );
        config->ha_config = NULL;
    }
#endif
    free( config->policy_ref_count );
    free( config );
}

struct session_plugins *getSessionPlugins( void )
{
    if( pfunks == NULL )
        pfunks = calloc( 1, sizeof( struct session_plugins ) );

    return pfunks;
}

void freeSessionPlugins( void )
{
    if( pfunks != NULL )
        free( pfunks );
    pfunks = NULL;
}

void registerDirectionPortCallback( uint8_t proto, set_dir_ports_cb cb_func )
{
    struct session_plugins *pfunks = getSessionPlugins( );

    if( proto== SESSION_PROTO_TCP )
        pfunks->set_tcp_dir_ports = cb_func;
    else if( proto == SESSION_PROTO_UDP )
        pfunks->set_udp_dir_ports = cb_func;
    else
        LogMessage( " WARNING: Invalid Protocol %u specified in direction callback registration\n", proto );
}

void registerFlushStreamCallback( bool client_to_server, flush_stream_cb cb_func )
{
    struct session_plugins *pfunks = getSessionPlugins( );

    if( client_to_server )
        pfunks->flush_client_stream = cb_func;
    else
        pfunks->flush_server_stream = cb_func;
}

#ifdef SNORT_RELOAD
void register_no_ref_policy_callback(SessionConfiguration *session_conf, NoRefCallback callback, void *data)
{
    if( session_conf )
    {
        session_conf->no_ref_cb = callback;
        session_conf->no_ref_cb_data = data;
    }
}
#endif
