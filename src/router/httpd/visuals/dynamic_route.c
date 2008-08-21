#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/route.h>

#include <broadcom.h>

/*
 * Dump route in <tr><td>IP</td><td>MASK</td><td>GW</td><td>Hop
 * Count</td><td>interface</td></tr> format 
 */
void ej_dump_route_table( webs_t wp, int argc, char_t ** argv )
{
    int count = 0;
    FILE *fp, *fp1;
    int flgs, ref, use, metric;
    unsigned long dest, gw, netmask;
    char line[256];
    struct in_addr dest_ip;
    struct in_addr gw_ip;
    struct in_addr netmask_ip;
    char sdest[16], sgw[16];
    int debug = 0, blank = 1;

    /*
     * open route table 
     */
    if( ( fp = fopen( "/proc/net/route", "r" ) ) == NULL )
    {
	websError( wp, 400, "No route table\n" );
	return;
    }

    /*
     * Read the route cache entries. 
     */
    // Iface Destination Gateway Flags RefCnt Use Metric Mask MTU Window IRTT 
    // 
    // vmnet1 004410AC 00000000 0001 0 0 0 00FFFFFF 40 0 0 

    while( fgets( line, sizeof( line ), fp ) != NULL )
    {
	if( count )
	{
	    int ifl = 0;

	    while( line[ifl] != ' ' && line[ifl] != '\t'
		   && line[ifl] != '\0' )
		ifl++;
	    line[ifl] = 0;	/* interface */
	    if( sscanf
		( line + ifl + 1, "%lx%lx%X%d%d%d%lx", &dest, &gw, &flgs,
		  &ref, &use, &metric, &netmask ) != 7 )
	    {
		break;
	    }
	    debug = 0;
	    dest_ip.s_addr = dest;
	    gw_ip.s_addr = gw;
	    netmask_ip.s_addr = netmask;

	    strcpy( sdest, ( dest_ip.s_addr == 0 ? "0.0.0.0" : inet_ntoa( dest_ip ) ) );	// default
	    strcpy( sgw, ( gw_ip.s_addr == 0 ? "0.0.0.0" : inet_ntoa( gw_ip ) ) );	// *

	    /*
	     * not 0x0001 route usable 
	     */
	    if( !( flgs & RTF_UP ) )
		continue;

	    /*
	     * filter ppp pseudo interface for DOD 
	     */
	    if( !strcmp( sdest, PPP_PSEUDO_IP )
		&& !strcmp( sgw, PPP_PSEUDO_GW ) )
		debug = 1;

	    /*
	     * Don't show loopback device 
	     */
	    if( !strcmp( line, "lo" ) )
		debug = 1;

	    /*
	     * Don't show eth1 information for pppoe mode 
	     */
	    if( !strcmp( line, nvram_safe_get( "wan_ifname" ) )
		&& nvram_match( "wan_proto", "pppoe" ) )
		debug = 1;

	    /*
	     * Don't show pseudo interface 
	     */
	    if( !strncmp( line, "ppp", 3 ) )
	    {
		fp1 = fopen( "/tmp/ppp/link", "r" );
		if( !fp1 )
		    debug = 1;
		else
		    fclose( fp1 );
	    }
	    char *ifname = line;

	    if( !strcmp( ifname, nvram_safe_get( "lan_ifname" ) ) )
		ifname = "LAN";
	    if( !strcmp( ifname, nvram_safe_get( "wan_ifname" ) ) )
		ifname = "WAN";

	    websWrite( wp, "%s%c'%s','%s','%s','%s'\n",
		       debug ? "//" : "",
		       blank ? ' ' : ',',
		       sdest, inet_ntoa( netmask_ip ), sgw, ifname );

	    if( debug && blank )
		blank = 1;
	    else
		blank = 0;

	}

	count++;
    }

    return;
}
