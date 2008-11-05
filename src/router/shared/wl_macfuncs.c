/*
 * wl_macfuncs.c
 *
 * Copyright (C) 2005 - 2006 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */

#include <string.h>
#include <memory.h>
int getsocket( void );
#ifdef HAVE_MADWIFI
#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <bcmnvram.h>

#include "wireless.h"
#include "net80211/ieee80211.h"
#include "net80211/ieee80211_crypto.h"
#include "net80211/ieee80211_ioctl.h"
#include <stdio.h>


static int set80211param( char *iface, int op, int arg )
{
    struct iwreq iwr;

    memset( &iwr, 0, sizeof( iwr ) );
    strncpy( iwr.ifr_name, iface, IFNAMSIZ );
    iwr.u.mode = op;
    memcpy( iwr.u.name + sizeof( __u32 ), &arg, sizeof( arg ) );

    if( ioctl( getsocket(  ), IEEE80211_IOCTL_SETPARAM, &iwr ) < 0 )
    {
	perror( "ioctl[IEEE80211_IOCTL_SETPARAM]" );
	return -1;
    }
    return 0;
}

struct maclist
{
    uint count;			/* number of MAC addresses */
    struct ether_addr ea[1];	/* variable length array of MAC addresses */
};

void security_disable( char *iface )
{
#ifdef DEBUG
    printf( "Security Disable\n" );
#endif
    set80211param( iface, IEEE80211_PARAM_MACCMD, IEEE80211_MACCMD_FLUSH );
    set80211param( iface, IEEE80211_PARAM_MACCMD,
		   IEEE80211_MACCMD_POLICY_OPEN );

}
static const char *ieee80211_ntoa( const unsigned char
				   mac[IEEE80211_ADDR_LEN] )
{
    static char a[18];
    int i;

    i = snprintf( a, sizeof( a ), "%02x:%02x:%02x:%02x:%02x:%02x",
		  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] );
    return ( i < 17 ? NULL : a );
}

void set_maclist( char *iface, char *buf )
{
    struct sockaddr sa;
    struct maclist *maclist = ( struct maclist * )buf;

    if( maclist->count == 0 )
	security_disable( iface );
    uint i;

    for( i = 0; i < maclist->count; i++ )
    {
	memcpy( sa.sa_data, &maclist->ea[i], IEEE80211_ADDR_LEN );
	fprintf( stderr, "maclist add %s\n",
		 ieee80211_ntoa( ( unsigned char * )&maclist->ea[i] ) );
	do80211priv( iface, IEEE80211_IOCTL_ADDMAC, &sa, sizeof( sa ) );
    }
}
void security_deny( char *iface )
{
#ifdef DEBUG
    printf( "Policy Deny\n" );
#endif
    // fprintf(stderr,"maclist deny\n");
    set80211param( iface, IEEE80211_PARAM_MACCMD,
		   IEEE80211_MACCMD_POLICY_ALLOW );
}

void security_allow( char *iface )
{
#ifdef DEBUG
    printf( "Policy Deny\n" );
#endif
    // fprintf(stderr,"maclist allow\n");
    set80211param( iface, IEEE80211_PARAM_MACCMD,
		   IEEE80211_MACCMD_POLICY_DENY );
}

void kick_mac( char *iface, char *mac )
{
#ifdef DEBUG
    printf( "KickMac: %s\n", mac );
#endif
    struct ieee80211req_mlme mlme;

    mlme.im_op = IEEE80211_MLME_DISASSOC;
    // mlme.im_reason = IEEE80211_REASON_UNSPECIFIED;
    mlme.im_reason = IEEE80211_REASON_NOT_AUTHED;
    memcpy( mlme.im_macaddr, mac, 6 );

    do80211priv( iface, IEEE80211_IOCTL_SETMLME, &mlme, sizeof( mlme ) );
}
#elif HAVE_RT2880
#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <bcmnvram.h>
#include <stdio.h>



struct maclist
{
    uint count;			/* number of MAC addresses */
    struct ether_addr ea[1];	/* variable length array of MAC addresses */
};

void security_disable( char *iface )
{
	sysprintf("iwpriv %s set ACLClearAll=1",iface);
	sysprintf("iwpriv %s set AccessPolicy=0",iface);

}
static const char *ieee80211_ntoa( const unsigned char
				   mac[6] )
{
    static char a[18];
    int i;

    i = snprintf( a, sizeof( a ), "%02x:%02x:%02x:%02x:%02x:%02x",
		  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] );
    return ( i < 17 ? NULL : a );
}

void set_maclist( char *iface, char *buf )
{
    struct maclist *maclist = ( struct maclist * )buf;

    if( maclist->count == 0 )
	security_disable( iface );
    uint i;

    for( i = 0; i < maclist->count; i++ )
    {
	sysprintf("iwpriv %s set ACLAddEntry=%s",iface,ieee80211_ntoa( ( unsigned char * )&maclist->ea[i] ));
    }
}
void security_deny( char *iface )
{

    sysprintf("iwpriv %s set AccessPolicy=2",iface);
}

void security_allow( char *iface )
{
    sysprintf("iwpriv %s set AccessPolicy=1",iface);
}

void kick_mac( char *iface, char *mac )
{

sysprintf("iwpriv %s set DisConnectSta=%s",iface,ieee80211_ntoa( mac ));
}



#else

#include <wlutils.h>
#include <wlioctl.h>

#include <bcmnvram.h>
void set_maclist( char *iface, char *buf )
{
    wl_ioctl( iface, WLC_SET_MACLIST, buf, WLC_IOCTL_MAXLEN );
}

void security_disable( char *iface )
{
    int val;

    val = WLC_MACMODE_DISABLED;
    wl_ioctl( iface, WLC_SET_MACMODE, &val, sizeof( val ) );
}

void security_deny( char *iface )
{
    int val;

    val = WLC_MACMODE_DENY;
    wl_ioctl( iface, WLC_SET_MACMODE, &val, sizeof( val ) );
}

void security_allow( char *iface )
{
    int val;

    val = WLC_MACMODE_ALLOW;
    wl_ioctl( iface, WLC_SET_MACMODE, &val, sizeof( val ) );
}

void kick_mac( char *iface, char *mac )
{
    scb_val_t scb_val;

    scb_val.val = ( uint32 ) DOT11_RC_NOT_AUTH;
    memcpy( &scb_val.ea, mac, ETHER_ADDR_LEN );
    wl_ioctl( iface, WLC_SCB_DEAUTHENTICATE_FOR_REASON, &scb_val, sizeof( scb_val ) );	/* Kick 
											 * station 
											 * off 
																				 */
}
#endif