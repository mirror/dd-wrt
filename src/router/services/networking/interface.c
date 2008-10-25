/*
 * interface.c
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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <proto/ethernet.h>
#include <shutils.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <rc.h>

#include <cy_conf.h>
#include <cymac.h>

#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)
#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)

int ifconfig( char *name, int flags, char *addr, char *netmask )
{
    // char *down="down";
    // if (flags == IFUP)
    // down = "up";
    cprintf( "ifconfig %s = %s/%s\n", name, addr, netmask );
    if( !ifexists( name ) )
    {
	cprintf( "interface %s does not exists, ignoring\n", name );
	return -1;
    }
    // if (addr==NULL)
    // addr="0.0.0.0";
    // int ret;
    // if (netmask==NULL)
    // {
    // ret = eval("ifconfig",name,addr,down);
    // }else
    // {
    // ret = eval("ifconfig",name,addr,"netmask",netmask,down);
    // }
    int s;
    struct ifreq ifr;
    struct in_addr in_addr, in_netmask, in_broadaddr;

    cprintf( "ifconfig(): name=[%s] flags=[%s] addr=[%s] netmask=[%s]\n",
	     name, flags == IFUP ? "IFUP" : "0", addr, netmask );

    if( ( s = socket( AF_INET, SOCK_RAW, IPPROTO_RAW ) ) < 0 )
	goto err;
    cprintf( "ifconfig(): socket opened\n" );

    strncpy( ifr.ifr_name, name, IFNAMSIZ );
    cprintf( "ifconfig(): set interface name\n" );
    if( flags )
    {
	ifr.ifr_flags = flags;
	if( ioctl( s, SIOCSIFFLAGS, &ifr ) < 0 )
	    goto err;
    }
    cprintf( "ifconfig(): interface flags configured\n" );
    if( addr )
    {
	inet_aton( addr, &in_addr );
	sin_addr( &ifr.ifr_addr ).s_addr = in_addr.s_addr;
	ifr.ifr_addr.sa_family = AF_INET;
	if( ioctl( s, SIOCSIFADDR, &ifr ) < 0 )
	    goto err;
    }
    cprintf( "ifconfig() ip configured\n" );

    if( addr && netmask )
    {
	inet_aton( netmask, &in_netmask );
	sin_addr( &ifr.ifr_netmask ).s_addr = in_netmask.s_addr;
	ifr.ifr_netmask.sa_family = AF_INET;
	if( ioctl( s, SIOCSIFNETMASK, &ifr ) < 0 )
	    goto err;

	in_broadaddr.s_addr =
	    ( in_addr.s_addr & in_netmask.s_addr ) | ~in_netmask.s_addr;
	sin_addr( &ifr.ifr_broadaddr ).s_addr = in_broadaddr.s_addr;
	ifr.ifr_broadaddr.sa_family = AF_INET;
	if( ioctl( s, SIOCSIFBRDADDR, &ifr ) < 0 )
	    goto err;
    }
    cprintf( "ifconfig() mask configured\n" );

    close( s );
    cprintf( "ifconfig() done()\n" );
    return 0;

  err:
    cprintf( "ifconfig() done with error\n" );
    close( s );
#ifndef HAVE_SILENCE
    perror( name );
#endif
    return errno;
    // return ret;
}

static char *getPhyDev(  )
{
    FILE *in = fopen( "/proc/switch/eth0/enable", "rb" );

    if( in )
    {
	fclose( in );
	return "eth0";
    }
    in = fopen( "/proc/switch/eth1/enable", "rb" );
    if( in )
    {
	fclose( in );
	return "eth1";
    }
    in = fopen( "/proc/switch/eth2/enable", "rb" );
    if( in )
    {
	fclose( in );
	return "eth2";
    }
    return "eth0";
}

#define MAX_VLAN_GROUPS	16
#define MAX_DEV_IFINDEX	16

/*
 * configure vlan interface(s) based on nvram settings 
 */
int start_config_vlan( void )
{
    int s;
    struct ifreq ifr;
    int i, j;
    char ea[ETHER_ADDR_LEN];
    char *phy = getPhyDev(  );

    // configure ports
    system2( "echo 1 > /proc/switch/eth0/reset" );
    system2( "echo 1 > /proc/switch/eth1/reset" );
    for( i = 0; i < 16; i++ )
    {
	char vlanb[16];

	sprintf( vlanb, "vlan%dports", i );
	if( nvram_get( vlanb ) == NULL || nvram_match( vlanb, "" ) )
	    continue;
	sysprintf( "echo %s > /proc/switch/%s/vlan/%d/ports",
		   nvram_safe_get( vlanb ), phy, i );
	sysprintf( "echo %s > /proc/switch/%s/vlan/%d/ports",
		   nvram_safe_get( vlanb ), phy, i );
    }

    /*
     * set vlan i/f name to style "vlan<ID>" 
     */

    eval( "vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD" );

    /*
     * create vlan interfaces 
     */
    if( ( s = socket( AF_INET, SOCK_RAW, IPPROTO_RAW ) ) < 0 )
	return errno;

    for( i = 0; i < MAX_VLAN_GROUPS; i++ )
    {
	char vlan_id[16];
	char *hwname, *hwaddr;

	if( !( hwname = nvram_nget( "vlan%dhwname", i ) ) )
	{
	    continue;
	}
	if( !( hwaddr = nvram_nget( "%smacaddr", hwname ) ) )
	{
	    continue;
	}
	if( strlen( hwname ) == 0 || strlen( hwaddr ) == 0 )
	{
	    continue;
	}
	ether_atoe( hwaddr, ea );
	for( j = 1; j <= MAX_DEV_IFINDEX; j++ )
	{
	    ifr.ifr_ifindex = j;
	    if( ioctl( s, SIOCGIFNAME, &ifr ) )
	    {
		continue;
	    }
	    if( ioctl( s, SIOCGIFHWADDR, &ifr ) )
	    {
		continue;
	    }
	    if( ifr.ifr_hwaddr.sa_family != ARPHRD_ETHER )
	    {
		continue;
	    }
	    if( !bcmp( ifr.ifr_hwaddr.sa_data, ea, ETHER_ADDR_LEN ) )
	    {
		break;
	    }
	}
	if( j > MAX_DEV_IFINDEX )
	{
	    continue;
	}
	if( ioctl( s, SIOCGIFFLAGS, &ifr ) )
	    continue;
	if( !( ifr.ifr_flags & IFF_UP ) )
	    ifconfig( ifr.ifr_name, IFUP, 0, 0 );
	snprintf( vlan_id, sizeof( vlan_id ), "%d", i );
	eval( "vconfig", "add", ifr.ifr_name, vlan_id );
    }

    close( s );

    return 0;
}

/*
 * begin lonewolf mods 
 */

int start_setup_vlans( void )
{
#if defined(HAVE_RB500) || defined(HAVE_XSCALE) || defined(HAVE_MAGICBOX) || defined(HAVE_FONERA) || defined(HAVE_WHRAG108) || defined(HAVE_LS2) || defined(HAVE_CA8) || defined(HAVE_TW6600) || defined(HAVE_PB42) || defined(HAVE_LS5) || defined(HAVE_LSX) || defined(HAVE_DANUBE) || defined(HAVE_STORM) || defined(HAVE_ADM5120)
    return 0;
#else
    /*
     * VLAN #16 is just a convieniant way of storing tagging info.  There is
     * no VLAN #16 
     */

    if( !nvram_get( "port5vlans" ) || nvram_match( "vlans", "0" ) )
	return 0;		// for some reason VLANs are not set up, and
    // we don't want to disable everything!

    if( nvram_match( "wan_vdsl", "1" ) && !nvram_match( "fromvdsl", "1" ) )
    {
	nvram_set( "vdsl_state", "0" );
	enable_dtag_vlan( 1 );
	return 0;
    }

    int ports[21][6], i, j, ret = 0, tmp, workaround = 0, found;
    char *vlans, *next, vlan[4], buff[70], buff2[16];
    FILE *fp;
    char portsettings[16][64];
    char tagged[16];
    unsigned char mac[20];;
    struct ifreq ifr;
    int s;

    s = socket( AF_INET, SOCK_RAW, IPPROTO_RAW );
    strcpy( mac, nvram_safe_get( "et0macaddr" ) );

    int vlanmap[6] = { 0, 1, 2, 3, 4, 5 };	// 0=wan; 1,2,3,4=lan;
    // 5=internal 

    if( nvram_match( "vlan1ports", "0 5" ) )
    {
	vlanmap[0] = 0;
	vlanmap[5] = 5;
	if( nvram_match( "vlan0ports", "4 3 2 1 5*" ) )
	{
	    vlanmap[1] = 4;
	    vlanmap[2] = 3;
	    vlanmap[3] = 2;
	    vlanmap[4] = 1;
	}
	else if( nvram_match( "vlan0ports", "4 1 2 3 5*" ) )
	{
	    vlanmap[1] = 4;
	    vlanmap[2] = 1;
	    vlanmap[3] = 2;
	    vlanmap[4] = 3;
	}
	else			// nvram_match ("vlan0ports", "1 2 3 4 5*")
	    // nothing to do
	{
	}
    }
    else if( nvram_match( "vlan1ports", "4 5" ) )
    {
	vlanmap[0] = 4;
	vlanmap[5] = 5;
	if( nvram_match( "vlan0ports", "0 1 2 3 5*" ) )
	{
	    vlanmap[1] = 0;
	    vlanmap[2] = 1;
	    vlanmap[3] = 2;
	    vlanmap[4] = 3;
	}
	else			// nvram_match ("vlan0ports", "3 2 1 0 5*")
	{
	    vlanmap[1] = 3;
	    vlanmap[2] = 2;
	    vlanmap[3] = 1;
	    vlanmap[4] = 0;
	}
    }
    else if( nvram_match( "vlan1ports", "1 5" ) )	// Linksys WTR54GS
    {
	vlanmap[5] = 5;
	vlanmap[0] = 1;
	vlanmap[1] = 0;
    }
    // else if .... feel free to extend for giga routers

    int ast = 0;
    char *asttemp = nvram_safe_get( "vlan0ports" );

    if( strstr( asttemp, "5*" ) || strstr( asttemp, "8*" ) )
	ast = 1;

    memset( &portsettings[0][0], 0, 16 * 64 );
    memset( &tagged[0], 0, 16 );
    for( i = 0; i < 6; i++ )
    {
	vlans = nvram_nget( "port%dvlans", i );
	int use = vlanmap[i];

	if( vlans )
	{
	    int lastvlan = 0;
	    int portmask = 3;
	    int mask = 0;

	    foreach( vlan, vlans, next )
	    {
		tmp = atoi( vlan );
		if( tmp < 16 )
		{
		    lastvlan = tmp;
		    if( i == 5 )
		    {
			snprintf( buff, 9, "%d", tmp );
			eval( "vconfig", "set_name_type",
			      "VLAN_PLUS_VID_NO_PAD" );
			eval( "vconfig", "add", "eth0", buff );
			snprintf( buff, 9, "vlan%d", tmp );
			if( strcmp( nvram_safe_get( "wan_ifname" ), buff ) )
			{
			    if( strlen( nvram_nget( "%s_ipaddr", buff ) ) >
				0 )
				eval( "ifconfig", buff,
				      nvram_nget( "%s_ipaddr", buff ),
				      "netmask", nvram_nget( "%s_netmask",
							     buff ), "up" );
			    else
				eval( "ifconfig", buff, "0.0.0.0", "up" );
			}
		    }

		    sprintf( ( char * )&portsettings[tmp][0], "%s %d",
			     ( char * )&portsettings[tmp][0], use );
		}
		else
		{
		    if( tmp == 16 )
			tagged[use] = 1;
		    if( tmp == 17 )
			mask |= 4;
		    if( tmp == 18 )
			mask |= 1;
		    if( tmp == 19 )
			mask |= 2;
		    if( tmp == 20 )
			mask |= 8;

		}
	    }
	    if( mask & 8 && use < 5 )
	    {
		sysprintf( "echo 0 > /proc/switch/eth0/port/%d/enable", use );
	    }
	    else
	    {
		sysprintf( "echo 1 > /proc/switch/eth0/port/%d/enable", use );
	    }
	    snprintf( buff, 69, "/proc/switch/eth0/port/%d/media", use );
	    if( ( fp = fopen( buff, "r+" ) ) )
	    {
		if( ( mask & 4 ) == 4 )
		{
		    if( ( mask & 3 ) == 0 )
		    {
			fprintf( stderr, "set port %d to 100FD\n", use );
			fputs( "100FD", fp );
		    }
		    if( ( mask & 3 ) == 1 )
		    {
			fprintf( stderr, "set port %d to 10FD\n", use );
			fputs( "10FD", fp );
		    }
		    if( ( mask & 3 ) == 2 )
		    {
			fprintf( stderr, "set port %d to 100HD\n", use );
			fputs( "100HD", fp );
		    }
		    if( ( mask & 3 ) == 3 )
		    {
			fprintf( stderr, "set port %d to 10HD\n", use );
			fputs( "10HD", fp );
		    }
		}
		else
		{
		    fprintf( stderr, "set port %d to AUTO\n", use );
		    fputs( "AUTO", fp );
		}
		fclose( fp );
	    }

	}
    }
    // for (i = 0; i < 16; i++)
    // {
    // fprintf(stderr,"echo %s >
    // /proc/switch/eth0/vlan/%d/ports\n",portsettings[i], i);
    // }
    for( i = 0; i < 16; i++ )
    {
	char port[64];

	strcpy( port, &portsettings[i][0] );
	memset( &portsettings[i][0], 0, 64 );
	foreach( vlan, port, next )
	{
	    if( atoi( vlan ) < 5 && atoi( vlan ) >= 0
		&& tagged[atoi( vlan )] )
		sprintf( &portsettings[i][0], "%s %st", &portsettings[i][0],
			 vlan );
	    else if( atoi( vlan ) == 5 && tagged[atoi( vlan )] && !ast )
		sprintf( &portsettings[i][0], "%s %st", &portsettings[i][0],
			 vlan );
	    else if( atoi( vlan ) == 5 && tagged[atoi( vlan )] && ast )
		sprintf( &portsettings[i][0], "%s %s*", &portsettings[i][0],
			 vlan );
	    else
		sprintf( &portsettings[i][0], "%s %s", &portsettings[i][0],
			 vlan );
	}
    }
    for( i = 0; i < 16; i++ )
    {
	sysprintf( "echo " " > /proc/switch/eth0/vlan/%d/ports", i );
    }
    for( i = 0; i < 16; i++ )
    {
	fprintf( stderr, "configure vlan ports to %s\n", portsettings[i] );
	sysprintf( "echo %s > /proc/switch/eth0/vlan/%d/ports",
		   portsettings[i], i );
    }
    return ret;
#endif
}

int flush_interfaces( void )
{
    char all_ifnames[256] = { 0 }, *c, *next, buff[32], buff2[32];

#ifdef HAVE_MADWIFI
#ifdef HAVE_GATEWORX
    snprintf( all_ifnames, 255, "%s %s %s", "ixp0",
	      nvram_safe_get( "lan_ifnames" ),
	      nvram_safe_get( "wan_ifnames" ) );
#elif HAVE_X86
    snprintf( all_ifnames, 255, "%s %s %s", "eth0",
	      nvram_safe_get( "lan_ifnames" ),
	      nvram_safe_get( "wan_ifnames" ) );
#elif HAVE_MAGICBOX
    snprintf( all_ifnames, 255, "%s %s %s", "eth0 eth1",
	      nvram_safe_get( "lan_ifnames" ),
	      nvram_safe_get( "wan_ifnames" ) );
#elif HAVE_DIR300
    snprintf( all_ifnames, 255, "%s %s %s", "eth0 vlan0 vlan2",
	      nvram_safe_get( "lan_ifnames" ),
	      nvram_safe_get( "wan_ifnames" ) );
#elif HAVE_MR3202A
    snprintf( all_ifnames, 255, "%s %s %s", "eth0 vlan1 vlan2",
	      nvram_safe_get( "lan_ifnames" ),
	      nvram_safe_get( "wan_ifnames" ) );
#elif HAVE_LS2
    snprintf( all_ifnames, 255, "%s %s %s", "eth0 vlan0 vlan2",
	      nvram_safe_get( "lan_ifnames" ),
	      nvram_safe_get( "wan_ifnames" ) );
#elif HAVE_LS5
    snprintf( all_ifnames, 255, "%s %s %s", "eth0",
	      nvram_safe_get( "lan_ifnames" ),
	      nvram_safe_get( "wan_ifnames" ) );
#elif HAVE_FONERA
    snprintf( all_ifnames, 255, "%s %s %s", "eth0 vlan0 vlan1",
	      nvram_safe_get( "lan_ifnames" ),
	      nvram_safe_get( "wan_ifnames" ) );
#elif HAVE_WHRAG108
    snprintf( all_ifnames, 255, "%s %s %s", "eth1",
	      nvram_safe_get( "lan_ifnames" ),
	      nvram_safe_get( "wan_ifnames" ) );
#elif HAVE_PB42
    snprintf( all_ifnames, 255, "%s %s %s", "eth0",
	      nvram_safe_get( "lan_ifnames" ),
	      nvram_safe_get( "wan_ifnames" ) );
#elif HAVE_LSX
    snprintf( all_ifnames, 255, "%s %s %s", "eth0",
	      nvram_safe_get( "lan_ifnames" ),
	      nvram_safe_get( "wan_ifnames" ) );
#elif HAVE_DANUBE
    snprintf( all_ifnames, 255, "%s %s %s", "eth0",
	      nvram_safe_get( "lan_ifnames" ),
	      nvram_safe_get( "wan_ifnames" ) );
#elif HAVE_STORM
    snprintf( all_ifnames, 255, "%s %s %s", "eth0",
	      nvram_safe_get( "lan_ifnames" ),
	      nvram_safe_get( "wan_ifnames" ) );
#elif HAVE_ADM5120
    snprintf( all_ifnames, 255, "%s %s %s", "eth0",
	      nvram_safe_get( "lan_ifnames" ),
	      nvram_safe_get( "wan_ifnames" ) );
#elif HAVE_TW6600
    snprintf( all_ifnames, 255, "%s %s %s", "eth0",
	      nvram_safe_get( "lan_ifnames" ),
	      nvram_safe_get( "wan_ifnames" ) );
#elif HAVE_CA8PRO
    snprintf( all_ifnames, 255, "%s %s %s", "vlan0 vlan1",
	      nvram_safe_get( "lan_ifnames" ),
	      nvram_safe_get( "wan_ifnames" ) );
#elif HAVE_CA8
    snprintf( all_ifnames, 255, "%s %s %s", "eth0",
	      nvram_safe_get( "lan_ifnames" ),
	      nvram_safe_get( "wan_ifnames" ) );
#else
    snprintf( all_ifnames, 255, "%s %s %s", "ixp0",
	      nvram_safe_get( "lan_ifnames" ),
	      nvram_safe_get( "wan_ifnames" ) );
#endif
#else
    if( wl_probe( "eth2" ) )
	snprintf( all_ifnames, 255, "%s %s %s", "eth0",
		  nvram_safe_get( "lan_ifnames" ),
		  nvram_safe_get( "wan_ifnames" ) );
    else
	snprintf( all_ifnames, 255, "%s %s %s %s", "eth0", "eth1",
		  nvram_safe_get( "lan_ifnames" ),
		  nvram_safe_get( "wan_ifnames" ) );
#endif
    // strcpy(all_ifnames, "eth0 ");
    // strcpy(all_ifnames, "eth0 eth1 "); //James, note: eth1 is the wireless 
    // interface on V2/GS's. I think we need a check here.
    // strcat(all_ifnames, nvram_safe_get("lan_ifnames"));
    // strcat(all_ifnames, " ");
    // strcat(all_ifnames, nvram_safe_get("wan_ifnames"));

    c = nvram_safe_get( "port5vlans" );
    if( c )
    {
	foreach( buff, c, next )
	{
	    if( atoi( buff ) > 15 )
		continue;
	    snprintf( buff2, sizeof( buff2 ), " vlan%s", buff );
	    strcat( all_ifnames, buff2 );
	}
    }

    foreach( buff, all_ifnames, next )
    {
	if( strcmp( buff, "br0" ) == 0 )
	    continue;
	ifconfig( buff, 0, 0, 0 );

	// eval ("ifconfig", buff, "down");
	eval( "ip", "addr", "flush", "dev", buff );
	ifconfig( buff, IFUP, 0, 0 );

	// eval ("ifconfig", buff, "up");
    }

    return 0;
}

/*
 * int start_nonstd_interfaces (void) { char br_ifnames[256],
 * all_ifnames[256], ifnames[256], buff[256], buff2[256], buff3[32]; char
 * *next, *next2; int i, j, k, cidr;
 * 
 * strcpy (br_ifnames, nvram_safe_get ("lan_ifnames")); if (strlen
 * (br_ifnames) > 0) strcat (br_ifnames, " ");
 * 
 * strcat (br_ifnames, nvram_safe_get ("wan_ifname")); strcat (br_ifnames, "
 * "); strcat (br_ifnames, nvram_safe_get ("wan_ifnames"));
 * 
 * strcpy (all_ifnames, "eth1"); strcpy (ifnames, nvram_safe_get
 * ("port5vlans")); if (strlen (ifnames) < 1) strcat (all_ifnames, " eth0");
 * foreach (buff, ifnames, next) { if (atoi (buff) > 15) continue; snprintf
 * (buff2, 63, " vlan%s", buff); strcat (all_ifnames, buff2); }
 * 
 * strcpy (ifnames, "");
 * 
 * foreach (buff, all_ifnames, next) { i = 1;
 * 
 * foreach (buff2, br_ifnames, next2) { if (strcmp (buff, buff2) == 0) i = 0;
 * }
 * 
 * if (i) { if (strlen (ifnames) > 1) strcat (ifnames, " ");
 * 
 * strcat (ifnames, buff); } }
 * 
 * i = 1;
 * 
 * foreach (buff2, ifnames, next) { snprintf (buff, sizeof (buff),
 * "%s_ipaddr", buff2); strcpy (all_ifnames, nvram_safe_get (buff)); snprintf
 * (buff, sizeof (buff), "%s_netmask", buff2); strcpy (br_ifnames,
 * nvram_safe_get (buff));
 * 
 * next2 = strtok (br_ifnames, "."); j = 0; cidr = 0; while ((next2 != NULL)
 * && (j < 4)) { for (k = 0; k < 8; k++) cidr += ((atoi (next2) << j) & 255)
 * >> 7;
 * 
 * next2 = strtok (NULL, "."); j++; }
 * 
 * snprintf (buff, sizeof (buff), "%s/%d", all_ifnames, cidr);
 * 
 * eval ("ifconfig", buff2, "up"); eval ("ip", "addr", "add", buff,
 * "broadcast", "+", "dev", buff2);
 * 
 * snprintf (buff, sizeof (buff), "%s_alias", buff2); strcpy (all_ifnames,
 * nvram_safe_get (buff)); foreach (buff3, all_ifnames, next2) { snprintf
 * (buff, sizeof (buff), "%s:%d", buff2, i); eval ("ip", "addr", "add", buff3, 
 * "broadcast", "+", "label", buff, "dev", buff2); i++; } } return 0; } 
 */
