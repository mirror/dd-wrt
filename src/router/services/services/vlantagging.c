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
#include <linux/if.h>

#ifdef HAVE_VLANTAGGING
#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)

void start_vlantagging( void )
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
	{
	    break;
	}
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
	    ifconfig( vlan_name, IFUP, nvram_nget( "%s_ipaddr", vlan_name ),
		      nvram_nget( "%s_netmask", vlan_name ) );
	}
    }
    char eths[256];

    getIfLists( eths, 256 );
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

		ifconfig( word, IFUP, nvram_nget( "%s_ipaddr", word ),
			  nvram_nget( "%s_netmask", word ) );
	    }
	}
    }

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
	    char *mtu = nvram_nget("%s_mtu",tag);
	    if (mtu && strlen(mtu))
		{
		eval( "ifconfig", tag, "mtu", mtu); 
		eval( "ifconfig", port, "mtu", mtu); //sync mtu for interface
		}
	    br_add_interface( tag, port );
	    if( prio )
		br_set_port_prio( tag, port, prio );
	}
    }

}

void start_bridging( void )
{
    static char word[256];
    char *next, *wordlist;

    wordlist = nvram_safe_get( "bridges" );
    foreach( word, wordlist, next )
    {
	char *port = word;
	char *tag = strsep( &port, ">" );
	char *prio = port;
	char *mtu = strsep( &prio, ">" );

	strsep( &mtu, ">" );
	if( !tag || !port )
	    break;
	char ipaddr[32];

	sprintf( ipaddr, "%s_ipaddr", tag );
	char netmask[32];

	sprintf( netmask, "%s_netmask", tag );

	br_add_bridge( tag );
	if( !strcmp( port, "On" ) )
	    br_set_stp_state( tag, 1 );
	else
	    br_set_stp_state( tag, 0 );
	if( prio )
	    br_set_bridge_prio( tag, prio );
	if (mtu && strlen(mtu)>0)
	    nvram_nset(mtu,"%s_mtu",tag);
	
	if( !nvram_match( ipaddr, "0.0.0.0" )
	    && !nvram_match( netmask, "0.0.0.0" ) )
	{
	    eval( "ifconfig", tag, nvram_safe_get( ipaddr ), "netmask",
		  nvram_safe_get( netmask ), "up" );
	}
	else
	    eval( "ifconfig", tag, "0.0.0.0", "up" );
	eval( "ifconfig", tag, "mtu", mtu);
    }
    start_set_routes(  );
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
}

void stop_bridging( void )
{
    static char word[256];
    char *next, *wordlist;

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
