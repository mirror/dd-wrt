/*
 * Routines for managing persistent storage of port mappings, etc.
 *
 * Copyright 2004, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: nvparse.c,v 1.3 2005/11/11 09:26:18 seg Exp $
 */
#define __CONFIG_NAT__
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <syslog.h>

#include <typedefs.h>
#include <netconf.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <nvparse.h>
#include <utils.h>
#include <bcmconfig.h>
extern char *safe_snprintf( char *str, int *len, const char *fmt, ... );

#ifdef __CONFIG_NAT__
#if 0
bool valid_autofw_port( const netconf_app_t * app )
{
    /*
     * Check outbound protocol 
     */
    if( app->match.ipproto != IPPROTO_TCP
	&& app->match.ipproto != IPPROTO_UDP )
	return FALSE;

    /*
     * Check outbound port range 
     */
    if( ntohs( app->match.dst.ports[0] ) > ntohs( app->match.dst.ports[1] ) )
	return FALSE;

    /*
     * Check related protocol 
     */
    if( app->proto != IPPROTO_TCP && app->proto != IPPROTO_UDP )
	return FALSE;

    /*
     * Check related destination port range 
     */
    if( ntohs( app->dport[0] ) > ntohs( app->dport[1] ) )
	return FALSE;

    /*
     * Check mapped destination port range 
     */
    if( ntohs( app->to[0] ) > ntohs( app->to[1] ) )
	return FALSE;

    /*
     * Check port range size 
     */
    if( ( ntohs( app->dport[1] ) - ntohs( app->dport[0] ) ) !=
	( ntohs( app->to[1] ) - ntohs( app->to[0] ) ) )
	return FALSE;

    return TRUE;
}

bool get_autofw_port( int which, netconf_app_t * app )
{
    char name[] = "autofw_portXXXXXXXXXX", value[1000];
    char *out_proto, *out_start, *out_end, *in_proto, *in_start, *in_end,
	*to_start, *to_end;
    char *enable, *desc;

    memset( app, 0, sizeof( netconf_app_t ) );

    /*
     * Parse
     * out_proto:out_start-out_end,in_proto:in_start-in_end>to_start-to_end,enable,desc 
     */
    snprintf( name, sizeof( name ), "autofw_port%d", which );
    if( !nvram_invmatch( name, "" ) )
	return FALSE;
    strncpy( value, nvram_get( name ), sizeof( value ) );

    /*
     * Check for outbound port specification 
     */
    out_start = value;
    out_proto = strsep( &out_start, ":" );
    if( !out_start )
	return FALSE;

    /*
     * Check for related protocol specification 
     */
    in_proto = out_start;
    out_start = strsep( &in_proto, "," );
    if( !in_proto )
	return FALSE;

    /*
     * Check for related destination port specification 
     */
    in_start = in_proto;
    in_proto = strsep( &in_start, ":" );
    if( !in_start )
	return FALSE;

    /*
     * Check for mapped destination port specification 
     */
    to_start = in_start;
    in_start = strsep( &to_start, ">" );
    if( !to_start )
	return FALSE;

    /*
     * Check for enable specification 
     */
    enable = to_start;
    to_end = strsep( &enable, "," );
    if( !enable )
	return FALSE;

    /*
     * Check for description specification (optional) 
     */
    desc = enable;
    enable = strsep( &desc, "," );

    /*
     * Check for outbound port range (optional) 
     */
    out_end = out_start;
    out_start = strsep( &out_end, "-" );
    if( !out_end )
	out_end = out_start;

    /*
     * Check for related destination port range (optional) 
     */
    in_end = in_start;
    in_start = strsep( &in_end, "-" );
    if( !in_end )
	in_end = in_start;

    /*
     * Check for mapped destination port range (optional) 
     */
    to_end = to_start;
    to_start = strsep( &to_end, "-" );
    if( !to_end )
	to_end = to_start;

    /*
     * Parse outbound protocol 
     */
    if( !strncasecmp( out_proto, "tcp", 3 ) )
	app->match.ipproto = IPPROTO_TCP;
    else if( !strncasecmp( out_proto, "udp", 3 ) )
	app->match.ipproto = IPPROTO_UDP;
    else
	return FALSE;

    /*
     * Parse outbound port range 
     */
    app->match.dst.ports[0] = htons( atoi( out_start ) );
    app->match.dst.ports[1] = htons( atoi( out_end ) );

    /*
     * Parse related protocol 
     */
    if( !strncasecmp( in_proto, "tcp", 3 ) )
	app->proto = IPPROTO_TCP;
    else if( !strncasecmp( in_proto, "udp", 3 ) )
	app->proto = IPPROTO_UDP;
    else
	return FALSE;

    /*
     * Parse related destination port range 
     */
    app->dport[0] = htons( atoi( in_start ) );
    app->dport[1] = htons( atoi( in_end ) );

    /*
     * Parse mapped destination port range 
     */
    app->to[0] = htons( atoi( to_start ) );
    app->to[1] = htons( atoi( to_end ) );

    /*
     * Parse enable 
     */
    if( !strncasecmp( enable, "off", 3 ) )
	app->match.flags = NETCONF_DISABLED;

    /*
     * Parse description 
     */
    if( desc )
	strncpy( app->desc, desc, sizeof( app->desc ) );

    /*
     * Set interface name (match packets entering LAN interface) 
     */
    strncpy( app->match.in.name, nvram_safe_get( "lan_ifname" ), IFNAMSIZ );

    /*
     * Set LAN source port range (match packets from any source port) 
     */
    app->match.src.ports[1] = htons( 0xffff );

    /*
     * Set target (application specific port forward) 
     */
    app->target = NETCONF_APP;

    return valid_autofw_port( app );
}

bool set_autofw_port( int which, const netconf_app_t * app )
{
    char name[] = "autofw_portXXXXXXXXXX", value[1000], *cur = value;
    int len;

    if( !valid_autofw_port( app ) )
	return FALSE;

    /*
     * Set
     * out_proto:out_start-out_end,in_proto:in_start-in_end>to_start-to_end,enable,desc 
     */
    snprintf( name, sizeof( name ), "autofw_port%d", which );
    len = sizeof( value );

    /*
     * Set outbound protocol 
     */
    if( app->match.ipproto == IPPROTO_TCP )
	cur = safe_snprintf( cur, &len, "tcp" );
    else if( app->match.ipproto == IPPROTO_UDP )
	cur = safe_snprintf( cur, &len, "udp" );

    /*
     * Set outbound port 
     */
    cur = safe_snprintf( cur, &len, ":" );
    cur = safe_snprintf( cur, &len, "%d", ntohs( app->match.dst.ports[0] ) );
    cur = safe_snprintf( cur, &len, "-" );
    cur = safe_snprintf( cur, &len, "%d", ntohs( app->match.dst.ports[1] ) );

    /*
     * Set related protocol 
     */
    cur = safe_snprintf( cur, &len, "," );
    if( app->proto == IPPROTO_TCP )
	cur = safe_snprintf( cur, &len, "tcp" );
    else if( app->proto == IPPROTO_UDP )
	cur = safe_snprintf( cur, &len, "udp" );

    /*
     * Set related destination port range 
     */
    cur = safe_snprintf( cur, &len, ":" );
    cur = safe_snprintf( cur, &len, "%d", ntohs( app->dport[0] ) );
    cur = safe_snprintf( cur, &len, "-" );
    cur = safe_snprintf( cur, &len, "%d", ntohs( app->dport[1] ) );

    /*
     * Set mapped destination port range 
     */
    cur = safe_snprintf( cur, &len, ">" );
    cur = safe_snprintf( cur, &len, "%d", ntohs( app->to[0] ) );
    cur = safe_snprintf( cur, &len, "-" );
    cur = safe_snprintf( cur, &len, "%d", ntohs( app->to[1] ) );

    /*
     * Set enable 
     */
    cur = safe_snprintf( cur, &len, "," );
    if( app->match.flags & NETCONF_DISABLED )
	cur = safe_snprintf( cur, &len, "off" );
    else
	cur = safe_snprintf( cur, &len, "on" );

    /*
     * Set description 
     */
    if( *app->desc )
    {
	cur = safe_snprintf( cur, &len, "," );
	cur = safe_snprintf( cur, &len, app->desc );
    }

    /*
     * Do it 
     */
    if( nvram_set( name, value ) )
	return FALSE;

    return TRUE;
}

bool del_autofw_port( int which )
{
    char name[] = "autofw_portXXXXXXXXXX";

    snprintf( name, sizeof( name ), "autofw_port%d", which );
    return ( nvram_unset( name ) == 0 ) ? TRUE : FALSE;
}
#endif

bool valid_forward_port( const netconf_nat_t * nat )
{
    /*
     * Check WAN destination port range 
     */
    if( ntohs( nat->match.dst.ports[0] ) > ntohs( nat->match.dst.ports[1] ) )
	return FALSE;

    /*
     * Check protocol 
     */
    if( nat->match.ipproto != IPPROTO_TCP
	&& nat->match.ipproto != IPPROTO_UDP )
	return FALSE;

    /*
     * Check LAN IP address 
     */
    if( nat->ipaddr.s_addr == htonl( 0 ) )
	return FALSE;

    /*
     * Check LAN destination port range 
     */
    if( ntohs( nat->ports[0] ) > ntohs( nat->ports[1] ) )
	return FALSE;

    /*
     * Check port range size 
     */
    if( ( ntohs( nat->match.dst.ports[1] ) -
	  ntohs( nat->match.dst.ports[0] ) ) !=
	( ntohs( nat->ports[1] ) - ntohs( nat->ports[0] ) ) )
	return FALSE;

    return TRUE;
}

bool get_forward_port( int which, netconf_nat_t * nat )
{
    char name[] = "forward_portXXXXXXXXXX", value[1000];
    char *wan_port0, *wan_port1, *lan_ipaddr, *lan_port0, *lan_port1, *proto;
    char *enable, *desc;

    memset( nat, 0, sizeof( netconf_nat_t ) );

    /*
     * Parse
     * wan_port0-wan_port1>lan_ipaddr:lan_port0-lan_port1[:,]proto[:,]enable[:,]desc 
     */
    snprintf( name, sizeof( name ), "forward_port%d", which );
    if( !nvram_invmatch( name, "" ) )
	return FALSE;
    strncpy( value, nvram_get( name ), sizeof( value ) );

    /*
     * Check for LAN IP address specification 
     */
    lan_ipaddr = value;
    wan_port0 = strsep( &lan_ipaddr, ">" );
    if( !lan_ipaddr )
	return FALSE;

    /*
     * Check for LAN destination port specification 
     */
    lan_port0 = lan_ipaddr;
    lan_ipaddr = strsep( &lan_port0, ":" );
    if( !lan_port0 )
	return FALSE;

    /*
     * Check for protocol specification 
     */
    proto = lan_port0;
    lan_port0 = strsep( &proto, ":," );
    if( !proto )
	return FALSE;

    /*
     * Check for enable specification 
     */
    enable = proto;
    proto = strsep( &enable, ":," );
    if( !enable )
	return FALSE;

    /*
     * Check for description specification (optional) 
     */
    desc = enable;
    enable = strsep( &desc, ":," );

    /*
     * Check for WAN destination port range (optional) 
     */
    wan_port1 = wan_port0;
    wan_port0 = strsep( &wan_port1, "-" );
    if( !wan_port1 )
	wan_port1 = wan_port0;

    /*
     * Check for LAN destination port range (optional) 
     */
    lan_port1 = lan_port0;
    lan_port0 = strsep( &lan_port1, "-" );
    if( !lan_port1 )
	lan_port1 = lan_port0;

    /*
     * Parse WAN destination port range 
     */
    nat->match.dst.ports[0] = htons( atoi( wan_port0 ) );
    nat->match.dst.ports[1] = htons( atoi( wan_port1 ) );

    /*
     * Parse LAN IP address 
     */
    /*
     * Check IP, add by honor 
     */
    if( get_single_ip( lan_ipaddr, 0 ) !=
	get_single_ip( nvram_safe_get( "lan_ipaddr" ), 0 )
	|| get_single_ip( lan_ipaddr,
			  1 ) !=
	get_single_ip( nvram_safe_get( "lan_ipaddr" ), 1 )
	|| get_single_ip( lan_ipaddr,
			  2 ) !=
	get_single_ip( nvram_safe_get( "lan_ipaddr" ), 2 ) )
    {
	/*
	 * Lan IP Address have been changed, so we must to adjust IP 
	 */
	char ip3[5];
	char *ip;
	char buf[254];

	snprintf( ip3, sizeof( ip3 ), "%d", get_single_ip( lan_ipaddr, 3 ) );
	ip = get_complete_lan_ip( ip3 );
	( void )inet_aton( ip, &nat->ipaddr );
	snprintf( buf, sizeof( buf ), "%s-%s>%s:%s-%s,%s,%s,%s", wan_port0,
		  wan_port1, ip, lan_port0, lan_port1, proto, enable, desc );
	nvram_set( name, buf );
    }
    else
	( void )inet_aton( lan_ipaddr, &nat->ipaddr );

    /*
     * Parse LAN destination port range 
     */
    nat->ports[0] = htons( atoi( lan_port0 ) );
    nat->ports[1] = htons( atoi( lan_port1 ) );

    /*
     * Parse protocol 
     */
    if( !strncasecmp( proto, "tcp", 3 ) )
	nat->match.ipproto = IPPROTO_TCP;
    else if( !strncasecmp( proto, "udp", 3 ) )
	nat->match.ipproto = IPPROTO_UDP;
    else
	return FALSE;

    /*
     * Parse enable 
     */
    if( !strncasecmp( enable, "off", 3 ) )
	nat->match.flags = NETCONF_DISABLED;

    /*
     * Parse description 
     */
    if( desc )
	strncpy( nat->desc, desc, sizeof( nat->desc ) );
    /*
     * Set WAN source port range (match packets from any source port) 
     */
    nat->match.src.ports[1] = htons( 0xffff );

    /*
     * Set target (DNAT) 
     */
    nat->target = NETCONF_DNAT;

    return valid_forward_port( nat );
}

bool set_forward_port( int which, const netconf_nat_t * nat )
{
    char name[] = "forward_portXXXXXXXXXX", value[1000], *cur = value;
    int len;

    if( !valid_forward_port( nat ) )
	return FALSE;

    /*
     * Set
     * wan_port0-wan_port1>lan_ipaddr:lan_port0-lan_port1,proto,enable,desc 
     */
    snprintf( name, sizeof( name ), "forward_port%d", which );
    len = sizeof( value );

    /*
     * Set WAN destination port range 
     */
    cur = safe_snprintf( cur, &len, "%d", ntohs( nat->match.dst.ports[0] ) );
    cur = safe_snprintf( cur, &len, "-" );
    cur = safe_snprintf( cur, &len, "%d", ntohs( nat->match.dst.ports[1] ) );

    /*
     * Set LAN IP address 
     */
    cur = safe_snprintf( cur, &len, ">" );
    char client[32];
    cur = safe_snprintf( cur, &len, inet_ntop(AF_INET, &nat->ipaddr,client,16));

    /*
     * Set LAN destination port range 
     */
    cur = safe_snprintf( cur, &len, ":" );
    cur = safe_snprintf( cur, &len, "%d", ntohs( nat->ports[0] ) );
    cur = safe_snprintf( cur, &len, "-" );
    cur = safe_snprintf( cur, &len, "%d", ntohs( nat->ports[1] ) );

    /*
     * Set protocol 
     */
    cur = safe_snprintf( cur, &len, "," );
    if( nat->match.ipproto == IPPROTO_TCP )
	cur = safe_snprintf( cur, &len, "tcp" );
    else if( nat->match.ipproto == IPPROTO_UDP )
	cur = safe_snprintf( cur, &len, "udp" );

    /*
     * Set enable 
     */
    cur = safe_snprintf( cur, &len, "," );
    if( nat->match.flags & NETCONF_DISABLED )
	cur = safe_snprintf( cur, &len, "off" );
    else
	cur = safe_snprintf( cur, &len, "on" );

    /*
     * Set description 
     */
    if( *nat->desc )
    {
	cur = safe_snprintf( cur, &len, "," );
	cur = safe_snprintf( cur, &len, nat->desc );
    }

    ct_logger( LOG_INFO, "upnp[%d]: Set \"%s\" to \"%s\"", getpid(  ), value,
	       name );

    /*
     * Do it 
     */
    if( nvram_set( name, value ) )
	return FALSE;

    return TRUE;
}

bool del_forward_port( int which )
{
    char name[] = "forward_portXXXXXXXXXX";

    snprintf( name, sizeof( name ), "forward_port%d", which );

    ct_logger( LOG_INFO, "upnp[%d]: Del \"%s\" from \"%s\"", getpid(  ),
	       nvram_safe_get( name ), name );

    return ( nvram_unset( name ) == 0 ) ? TRUE : FALSE;
}

static void convert_forward_proto( const char *name, int ipproto )
{
    char var[1000], *next;
    char *wan_port0, *wan_port1, *lan_ipaddr, *lan_port0, *lan_port1;
    netconf_nat_t nat, unused;
    bool valid;
    int i;

    foreach( var, nvram_safe_get( name ), next )
    {
	/*
	 * Parse wan_port0-wan_port1>lan_ipaddr:lan_port0-lan_port1 
	 */
	lan_ipaddr = var;
	wan_port0 = strsep( &lan_ipaddr, ">" );
	if( !lan_ipaddr )
	    continue;
	lan_port0 = lan_ipaddr;
	lan_ipaddr = strsep( &lan_port0, ":" );
	if( !lan_port0 )
	    continue;
	wan_port1 = wan_port0;
	wan_port0 = strsep( &wan_port1, "-" );
	if( !wan_port1 )
	    wan_port1 = wan_port0;
	lan_port1 = lan_port0;
	lan_port0 = strsep( &lan_port1, "-" );
	if( !lan_port1 )
	    lan_port1 = lan_port0;

	/*
	 * Set up parameters 
	 */
	memset( &nat, 0, sizeof( netconf_nat_t ) );
	nat.match.ipproto = ipproto;
	nat.match.dst.ports[0] = htons( atoi( wan_port0 ) );
	nat.match.dst.ports[1] = htons( atoi( wan_port1 ) );
	( void )inet_aton( lan_ipaddr, &nat.ipaddr );
	nat.ports[0] = htons( atoi( lan_port0 ) );
	nat.ports[1] = htons( atoi( lan_port1 ) );

	/*
	 * Replace an unused or invalid entry 
	 */
	for( i = 0; get_forward_port( i, &unused ); i++ );
	valid = set_forward_port( i, &nat );
	assert( valid );
    }

    nvram_unset( name );
}

#endif /* __CONFIG_NAT__ */

/*
 * wl_wds<N> is authentication protocol dependant.
 * when auth is "psk":
 *      wl_wds<N>=mac,role,crypto,auth,ssid,passphrase
 */
bool
get_wds_wsec( int unit, int which, char *mac, char *role,
	      char *crypto, char *auth, ... )
{
    char name[] = "wlXXXXXXX_wdsXXXXXXX", value[1000], *next;

    snprintf( name, sizeof( name ), "wl%d_wds%d", unit, which );
    strncpy( value, nvram_safe_get( name ), sizeof( value ) );
    next = value;

    /*
     * separate mac 
     */
    strcpy( mac, strsep( &next, "," ) );
    if( !next )
	return FALSE;

    /*
     * separate role 
     */
    strcpy( role, strsep( &next, "," ) );
    if( !next )
	return FALSE;

    /*
     * separate crypto 
     */
    strcpy( crypto, strsep( &next, "," ) );
    if( !next )
	return FALSE;

    /*
     * separate auth 
     */
    strcpy( auth, strsep( &next, "," ) );
    if( !next )
	return FALSE;

    if( !strcmp( auth, "psk" ) )
    {
	va_list va;

	va_start( va, auth );

	/*
	 * separate ssid 
	 */
	strcpy( va_arg( va, char * ), strsep( &next, "," ) );

	if( !next )
	    goto fail;

	/*
	 * separate passphrase 
	 */
	strcpy( va_arg( va, char * ), next );

	va_end( va );
	return TRUE;
      fail:
	va_end( va );
	return FALSE;
    }

    return FALSE;
}

bool
set_wds_wsec( int unit, int which, char *mac, char *role,
	      char *crypto, char *auth, ... )
{
    char name[] = "wlXXXXXXX_wdsXXXXXXX", value[10000];

    snprintf( name, sizeof( name ), "wl%d_wds%d", unit, which );
    snprintf( value, sizeof( value ), "%s,%s,%s,%s", mac, role, crypto,
	      auth );

    if( !strcmp( auth, "psk" ) )
    {
	int offset;
	va_list va;

	va_start( va, auth );
	offset = strlen( value );
	snprintf( &value[offset], sizeof( value ) - offset, ",%s,%s",
		  va_arg( va, char * ), va_arg( va, char * ) );
	va_end( va );

	if( nvram_set( name, value ) )
	    return FALSE;

	return TRUE;
    }

    return FALSE;
}

bool del_wds_wsec( int unit, int which )
{
    char name[] = "wlXXXXXXX_wdsXXXXXXX";

    snprintf( name, sizeof( name ), "wl%d_wds%d", unit, which );

    nvram_unset( name );

    return TRUE;
}
