#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/route.h>

#include <broadcom.h>

static char word[256];
void ej_show_routeif( webs_t wp, int argc, char_t ** argv )
{
    char *arg;
    int which, count;
    char *next, *page;
    char *ipaddr, *netmask, *gateway, *metric, *ifname;
    static ifnamecopy[32];
    int temp;
    static char bufferif[512];

    page = websGetVar( wp, "route_page", NULL );
    if( !page )
	page = "0";
    which = atoi( page );
    char *sroute = nvram_safe_get( "static_route" );

    foreach( word, sroute, next )
    {
	if( which-- == 0 )
	{
	    netmask = word;
	    ipaddr = strsep( &netmask, ":" );
	    if( !ipaddr || !netmask )
		continue;
	    gateway = netmask;
	    netmask = strsep( &gateway, ":" );
	    if( !netmask || !gateway )
		continue;
	    metric = gateway;
	    gateway = strsep( &metric, ":" );
	    if( !gateway || !metric )
		continue;
	    ifname = metric;
	    metric = strsep( &ifname, ":" );
	    if( !metric || !ifname )
		continue;
	    break;
	}
    }
    if( !ifname )
	ifname = "br0";
    strcpy( ifnamecopy, ifname );

    memset( bufferif, 0, 512 );
    getIfList( bufferif, NULL );
    websWrite( wp, "<option value=\"lan\" %s >LAN &amp; WLAN</option>\n",
	       nvram_match( "lan_ifname",
			    ifname ) ? "selected=\"selected\"" : "" );
    websWrite( wp, "<option value=\"wan\" %s >WAN</option>\n",
	       nvram_match( "wan_ifname",
			    ifname ) ? "selected=\"selected\"" : "" );
    websWrite( wp, "<option value=\"any\" %s >ANY</option>\n",
	       strcmp( "any",
		       ifnamecopy ) == 0 ? "selected=\"selected\"" : "" );
    memset( word, 0, 256 );
    next = NULL;
    foreach( word, bufferif, next )
    {
	if( nvram_match( "lan_ifname", word ) )
	    continue;
	if( nvram_match( "wan_ifname", word ) )
	    continue;
	websWrite( wp, "<option value=\"%s\" %s >%s</option>\n", word,
		   strcmp( word,
			   ifnamecopy ) == 0 ? "selected=\"selected\"" : "",
		   word );
    }
}

/*
 * Example: 
 * static_route=192.168.2.0:255.255.255.0:192.168.1.2:1:br0
 * <% static_route("ipaddr", 0); %> produces "192.168.2.0"
 * <% static_route("lan", 0); %> produces "selected" if nvram_match("lan_ifname", "br0")
 */
void ej_static_route_setting( webs_t wp, int argc, char_t ** argv )
{
    char *arg;
    int which, count;
    char *next, *page;
    char name[50] = "", *ipaddr, *netmask, *gateway, *metric, *ifname;
    int temp;
    char new_name[200];

#ifdef FASTWEB
    ejArgs( argc, argv, "%s %d", &arg, &count );
#else
    if( ejArgs( argc, argv, "%s %d", &arg, &count ) < 2 )
    {
	websError( wp, 400, "Insufficient args\n" );
	return;
    }
#endif

    page = websGetVar( wp, "route_page", NULL );
    if( !page )
	page = "0";

    which = atoi( page );

    temp = which;

    if( !strcmp( arg, "name" ) )
    {
	char *sroutename = nvram_safe_get( "static_route_name" );

	foreach( word, sroutename, next )
	{
	    if( which-- == 0 )
	    {
		find_match_pattern( name, sizeof( name ), word, "$NAME:",
				    "" );
		httpd_filter_name( name, new_name, sizeof( new_name ), GET );
		websWrite( wp, new_name );
		return;
	    }

	}
    }
    char *sroute = nvram_safe_get( "static_route" );

    foreach( word, sroute, next )
    {
	if( which-- == 0 )
	{
	    netmask = word;
	    ipaddr = strsep( &netmask, ":" );
	    if( !ipaddr || !netmask )
		continue;
	    gateway = netmask;
	    netmask = strsep( &gateway, ":" );
	    if( !netmask || !gateway )
		continue;
	    metric = gateway;
	    gateway = strsep( &metric, ":" );
	    if( !gateway || !metric )
		continue;
	    ifname = metric;
	    metric = strsep( &ifname, ":" );
	    if( !metric || !ifname )
		continue;
	    if( !strcmp( arg, "ipaddr" ) )
	    {
		websWrite( wp, "%d", get_single_ip( ipaddr, count ) );
		return;
	    }
	    else if( !strcmp( arg, "netmask" ) )
	    {
		websWrite( wp, "%d", get_single_ip( netmask, count ) );
		return;
	    }
	    else if( !strcmp( arg, "gateway" ) )
	    {
		websWrite( wp, "%d", get_single_ip( gateway, count ) );
		return;
	    }
	    else if( !strcmp( arg, "metric" ) )
	    {
		websWrite( wp, metric );
		return;
	    }
	    else if( !strcmp( arg, "lan" )
		     && nvram_match( "lan_ifname", ifname ) )
	    {
		websWrite( wp, "selected=\"selected\"" );
		return;
	    }
	    else if( !strcmp( arg, "wan" )
		     && nvram_match( "wan_ifname", ifname ) )
	    {
		websWrite( wp, "selected=\"selected\"" );
		return;
	    }
	}
    }

    if( !strcmp( arg, "ipaddr" ) || !strcmp( arg, "netmask" )
	|| !strcmp( arg, "gateway" ) )
	websWrite( wp, "0" );
    else if( !strcmp( arg, "metric" ) )
	websWrite( wp, "0" );
    return;
}

void ej_static_route_table( webs_t wp, int argc, char_t ** argv )
{
    int i, page;
    int which;
    char *type;
    char word[256], *next;

#ifdef FASTWEB
    ejArgs( argc, argv, "%s", &type );
#else
    if( ejArgs( argc, argv, "%s", &type ) < 1 )
    {
	websError( wp, 400, "Insufficient args\n" );
	return;
    }
#endif

    page = atoi( websGetVar( wp, "route_page", "0" ) );	// default to 0
    if( !strcmp( type, "select" ) )
    {
	char *sroutename = nvram_safe_get( "static_route_name" );

	for( i = 0; i < STATIC_ROUTE_PAGE; i++ )
	{
	    char name[50] = " ";
	    char new_name[80] = " ";
	    char buf[80] = "";

	    which = i;
	    foreach( word, sroutename, next )
	    {
		if( which-- == 0 )
		{
		    find_match_pattern( name, sizeof( name ), word, "$NAME:",
					" " );
		    httpd_filter_name( name, new_name, sizeof( new_name ),
				       GET );
		}
	    }
	    snprintf( buf, sizeof( buf ), "(%s)", new_name );
	    websWrite( wp, "\t\t<option value=\"%d\" %s> %d %s</option>\n", i,
		       ( i == page ) ? "selected=\"selected\"" : "", i + 1,
		       buf );
	}
    }

    return;
}
