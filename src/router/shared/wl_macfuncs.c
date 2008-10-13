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
#ifndef HAVE_MADWIFI
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
											 * AP 
											 */
}
#else
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

/*
 * Atheros 
 */


#define IOCTL_ERR(x) [x - SIOCIWFIRSTPRIV] "ioctl[" #x "]"
static int
set80211priv( struct iwreq *iwr, const char *ifname, int op, void *data,
	      size_t len )
{
#define	N(a)	(sizeof(a)/sizeof(a[0]))

    memset( iwr, 0, sizeof( struct iwreq ) );
    strncpy( iwr->ifr_name, ifname, IFNAMSIZ );
    if( len < IFNAMSIZ )
    {
	/*
	 * Argument data fits inline; put it there.
	 */
	memcpy( iwr->u.name, data, len );
    }
    else
    {
	/*
	 * Argument data too big for inline transfer; setup a
	 * parameter block instead; the kernel will transfer
	 * the data for the driver.
	 */
	iwr->u.data.pointer = data;
	iwr->u.data.length = len;
    }

    if( ioctl( getsocket(  ), op, iwr ) < 0 )
    {
	static const char *opnames[] = {
	    IOCTL_ERR( IEEE80211_IOCTL_SETPARAM ),
	    IOCTL_ERR( IEEE80211_IOCTL_GETPARAM ),
	    IOCTL_ERR( IEEE80211_IOCTL_SETMODE ),
	    IOCTL_ERR( IEEE80211_IOCTL_GETMODE ),
	    IOCTL_ERR( IEEE80211_IOCTL_SETWMMPARAMS ),
	    IOCTL_ERR( IEEE80211_IOCTL_GETWMMPARAMS ),
	    IOCTL_ERR( IEEE80211_IOCTL_SETCHANLIST ),
	    IOCTL_ERR( IEEE80211_IOCTL_GETCHANLIST ),
	    IOCTL_ERR( IEEE80211_IOCTL_CHANSWITCH ),
	    IOCTL_ERR( IEEE80211_IOCTL_GETCHANINFO ),
	    IOCTL_ERR( IEEE80211_IOCTL_SETOPTIE ),
	    IOCTL_ERR( IEEE80211_IOCTL_GETOPTIE ),
	    IOCTL_ERR( IEEE80211_IOCTL_SETMLME ),
	    IOCTL_ERR( IEEE80211_IOCTL_SETKEY ),
	    IOCTL_ERR( IEEE80211_IOCTL_DELKEY ),
	    IOCTL_ERR( IEEE80211_IOCTL_ADDMAC ),
	    IOCTL_ERR( IEEE80211_IOCTL_DELMAC ),
	    IOCTL_ERR( IEEE80211_IOCTL_WDSADDMAC ),
#ifdef OLD_MADWIFI
	    IOCTL_ERR( IEEE80211_IOCTL_WDSDELMAC ),
#else
	    IOCTL_ERR( IEEE80211_IOCTL_WDSSETMAC ),
#endif
	};
	op -= SIOCIWFIRSTPRIV;
	if( 0 <= op && op < N( opnames ) )
	    perror( opnames[op] );
	else
	    perror( "ioctl[unknown???]" );
	return -1;
    }
    return 0;
#undef N
}

int do80211priv( const char *ifname, int op, void *data, size_t len )
{
    struct iwreq iwr;

    if( set80211priv( &iwr, ifname, op, data, len ) < 0 )
	return -1;
    if( len < IFNAMSIZ )
	memcpy( data, iwr.u.name, len );
    return iwr.u.data.length;
}

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
#endif
