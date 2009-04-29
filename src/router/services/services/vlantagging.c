/*
 * vlantagging.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
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
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <sys/sysinfo.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <getopt.h>
#include <err.h>

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <shutils.h>
#include <utils.h>
#include <unistd.h>
#include <services.h>
#include "../networking/libbridge.h"

#ifdef HAVE_VLANTAGGING
#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)

void start_vlantagging( void )
{
    static char word[256];
    char *next, *wordlist;

    wordlist = nvram_safe_get( "vlan_tags" );
    foreach( word, wordlist, next )
    {
	char *port = word;
	char *tag = strsep( &port, ">" );

	if( !tag || !port )
	{
	    break;
	}
	eval( "vconfig", "set_name_type", "DEV_PLUS_VID_NO_PAD" );
	eval( "vconfig", "add", tag, port );
	char vlan_name[32];

	sprintf( vlan_name, "%s.%s", tag, port );

	char var[64];

	sprintf( var, "%s_bridged", vlan_name );
	if( nvram_default_match( var, "1", "1" ) )
	{
	    eval( "ifconfig", vlan_name, "0.0.0.0", "up" );
	}
	else
	{
	    eval( "ifconfig", vlan_name,nvram_nget( "%s_ipaddr",vlan_name),"netmask", nvram_nget( "%s_netmask", vlan_name ), "up" );
	}
    }
    char eths[256];

/*    getIfLists( eths, 256 );
    foreach( word, eths, next )
    {
	if( strcmp( get_wan_face(  ), word )
	    && strcmp( nvram_safe_get( "lan_ifname" ), word ) )
	{
	    char var[32];

	    sprintf( var, "%s_bridged", word );
	    if( nvram_default_match( var, "1", "1" ) )
	    {
		eval( "ifconfig", word, "0.0.0.0", "up" );
	    }
	    else
	    {
		eval( "ifconfig", word, nvram_nget( "%s_ipaddr", word ),"netmask",nvram_nget( "%s_netmask", word ), "up" );
	    }
	}
    }
*/
    start_set_routes(  );
}

void stop_vlantagging( void )
{
    static char word[256];
    char *next, *wordlist;

    wordlist = nvram_safe_get( "vlan_tags" );
    eval( "vconfig", "set_name_type", "DEV_PLUS_VID_NO_PAD" );
    foreach( word, wordlist, next )
    {
	char *port = word;
	char *tag = strsep( &port, ">" );

	if( !tag || !port )
	    break;
	char vlan_name[32];

	sprintf( vlan_name, "%s.%s", tag, port );
	if( ifexists( vlan_name ) )
	{
	    eval( "vconfig", "rem", vlan_name );
	}
    }
}
void start_bridgesif( void )
{
    if( nvram_match( "lan_stp", "0" ) )
	br_set_stp_state( "br0", 0 );
    else
	br_set_stp_state( "br0", 1 );

    static char word[256];
    char *next, *wordlist;

    wordlist = nvram_safe_get( "bridgesif" );
    foreach( word, wordlist, next )
    {
	char *port = word;
	char *tag = strsep( &port, ">" );
	char *prio = port;

	strsep( &prio, ">" );
	if( !tag || !port )
	    break;
	if( strncmp( tag, "EOP", 3 ) )
	{
/*	    char *mtu = nvram_nget("%s_mtu",tag);
	    if (mtu && strlen(mtu))
		{
		eval( "ifconfig", tag, "mtu", mtu); 
		eval( "ifconfig", port, "mtu", mtu); //sync mtu for interface
		}*/
	    br_add_interface( tag, port );
	    if( prio )
		br_set_port_prio( tag, port, prio );
	}
    }
    struct ifreq ifr;
    int s;
    char eabuf[32];
    if( ( s = socket( AF_INET, SOCK_RAW, IPPROTO_RAW ) ) < 0 )
	return;
    strncpy( ifr.ifr_name, nvram_safe_get("lan_ifname"), IFNAMSIZ );
    if( ioctl( s, SIOCGIFHWADDR, &ifr ) == 0 )
    {
	nvram_set( "lan_hwaddr",ether_etoa( ifr.ifr_hwaddr.sa_data, eabuf ) );
    }
    close(s);
}

void start_bridging( void )
{
    static char word[256];
    char *next, *wordlist;
#ifdef HAVE_MICRO
    br_init(  );
#endif
    wordlist = nvram_safe_get( "bridges" );
    foreach( word, wordlist, next )
    {
	char *stp = word;
	char *bridge = strsep( &stp, ">" );
	char *prio = stp;

	stp = strsep( &prio, ">" );
	char *mtu = prio;

	prio = strsep( &mtu, ">" );
	if( !prio )
	{
	    prio = mtu;
	    mtu = NULL;
	}
	if( !bridge || !stp )
	    break;

	if( prio && mtu && strlen( mtu ) > 0 )
	    nvram_nset( mtu, "%s_mtu", bridge );
	br_add_bridge( bridge );
	if( !strcmp( stp, "On" ) )
	    br_set_stp_state( bridge, 1 );
	else
	    br_set_stp_state( bridge, 0 );

	br_set_bridge_forward_delay( bridge, 1 );

	if( prio )
	    br_set_bridge_prio( bridge, prio );

	eval( "ifconfig", bridge, "up" );
    }
#ifdef HAVE_MICRO
    br_shutdown(  );
#endif

    start_set_routes(  );
}

char *getBridgeMTU( char *ifname )
{
    static char word[256];
    char *next, *wordlist;

    wordlist = nvram_safe_get( "bridges" );
    foreach( word, wordlist, next )
    {
	char *stp = word;
	char *bridge = strsep( &stp, ">" );
	char *mtu = stp;
	char *prio = strsep( &mtu, ">" );

	if( prio )
	    strsep( &mtu, ">" );

	if( !bridge || !stp )
	    break;
	if( !strcmp( bridge, ifname ) )
	{
	    if( !prio || !mtu )
		return "1500";
	    else
		return mtu;
	}
    }
    return "1500";
}

char *getMTU(char *ifname)
{
    if (!ifname)
	return "1500";
    char *mtu = nvram_nget("%s_mtu",ifname);
    if (!mtu || strlen(mtu)==0)
	return "1500";
    return mtu;
}

char *getBridge( char *ifname )
{
    static char word[256];
    char *next, *wordlist;

    wordlist = nvram_safe_get( "bridgesif" );
    foreach( word, wordlist, next )
    {
	char *port = word;
	char *tag = strsep( &port, ">" );
	char *prio = port;

	strsep( &prio, ">" );
	if( !tag || !port )
	    break;
	if( !strcmp( port, ifname ) )
	    return tag;
    }
    return nvram_safe_get( "lan_ifname" );
}

char *getRealBridge( char *ifname )
{
    static char word[256];
    char *next, *wordlist;

    wordlist = nvram_safe_get( "bridgesif" );
    foreach( word, wordlist, next )
    {
	char *port = word;
	char *tag = strsep( &port, ">" );
	char *prio = port;

	strsep( &prio, ">" );
	if( !tag || !port )
	    break;
	if( !strcmp( port, ifname ) )
	    return tag;
    }
    return NULL;
}

char *getBridgePrio( char *ifname )
{
    static char word[256];
    char *next, *wordlist;

    wordlist = nvram_safe_get( "bridgesif" );
    foreach( word, wordlist, next )
    {
	char *port = word;
	char *tag = strsep( &port, ">" );
	char *prio = port;

	strsep( &prio, ">" );
	if( !tag || !port )
	    break;
	if( !strcmp( port, ifname ) )
	    return port;
    }
    return "0";
}

void stop_bridgesif( void )
{
    static char word[256];
    char *next, *wordlist;
#ifdef HAVE_MICRO
    br_init(  );
#endif

    wordlist = nvram_safe_get( "bridgesif" );
    foreach( word, wordlist, next )
    {
	char *port = word;
	char *tag = strsep( &port, ">" );
	char *prio = port;

	strsep( &prio, ">" );
	if( !tag || !port )
	    break;
	if( ifexists( port ) )
	    br_del_interface( tag, port );
    }
#ifdef HAVE_MICRO
    br_shutdown(  );
#endif
}

void stop_bridging( void )
{
    static char word[256];
    char *next, *wordlist;
#ifdef HAVE_MICRO
    br_init(  );
#endif

    wordlist = nvram_safe_get( "bridges" );
    foreach( word, wordlist, next )
    {
	char *port = word;
	char *tag = strsep( &port, ">" );
	char *prio = port;

	strsep( &prio, ">" );
	if( !tag || !port )
	    break;
	if( ifexists( tag ) )
	{
	    eval( "ifconfig", tag, "down" );
	    br_del_bridge( tag );
	}
    }
#ifdef HAVE_MICRO
    br_shutdown(  );
#endif
}

#else
char *getBridge( char *ifname )
{
    return nvram_safe_get( "lan_ifname" );
}

char *getRealBridge( char *ifname )
{
    return NULL;
}

char *getBridgePrio( char *ifname )
{
    return "0";
}
#endif

int getbridge_main( int argc, char *argv[] )
{
    if( argc < 2 )
    {
	fprintf( stderr, "syntax: getbridge [ifname]\n" );
	return -1;
    }
    char *bridge = getBridge( argv[1] );

    fprintf( stdout, "%s\n", bridge );
    return 0;
}

int getbridgeprio_main( int argc, char *argv[] )
{
    if( argc < 2 )
    {
	fprintf( stderr, "syntax: getbridgeprio [ifname]\n" );
	return -1;
    }
    char *bridge = getBridgePrio( argv[1] );

    fprintf( stdout, "%s\n", bridge );
    return 0;
}
