
#define VALIDSOURCE 1

#ifdef WEBS
#include <webs.h>
#include <uemf.h>
#include <ej.h>
#else /* !WEBS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <httpd.h>
#include <errno.h>
#endif /* WEBS */

#include <proto/ethernet.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/klog.h>
#include <sys/wait.h>
#include <cyutils.h>
#include <support.h>
#include <cy_conf.h>
// #ifdef EZC_SUPPORT
#include <ezc.h>
// #endif
#include <broadcom.h>
#include <wlutils.h>
#include <netdb.h>
#include <utils.h>

#ifdef FILTER_DEBUG
extern FILE *debout;

#define D(a) fprintf(debout,"%s\n",a); fflush(debout);
#else
#define D(a)
#endif



void ( *do_ej_buffer ) ( char *buffer, webs_t stream );
int ( *httpd_filter_name ) ( char *old_name, char *new_name, size_t size,
			     int type );
char *( *websGetVar ) ( webs_t wp, char *var, char *d );
int ( *websWrite ) ( webs_t wp, char *fmt, ... );
struct wl_client_mac *wl_client_macs;

void initWeb( struct Webenvironment *env )
{
    cprintf( "set websgetwar\n" );
    websGetVar = env->PwebsGetVar;
    httpd_filter_name = env->Phttpd_filter_name;
    wl_client_macs = env->Pwl_client_macs;
    websWrite = env->PwebsWrite;
    do_ej_buffer = env->Pdo_ej_buffer;
}

/*
 * Example: ISASCII("", 0); return true; ISASCII("", 1); return false;
 * ISASCII("abc123", 1); return true; 
 */
int ISASCII( char *value, int flag )
{
    int i, tag = TRUE;

#if COUNTRY == JAPAN
    return tag;			// don't check for japan version
#endif

    if( !strcmp( value, "" ) )
    {
	if( flag )
	    return 0;		// null
	else
	    return 1;
    }

    for( i = 0; *( value + i ); i++ )
    {
	if( !isascii( *( value + i ) ) )
	{
	    tag = FALSE;
	    break;
	}
    }
    return tag;
}

/*
 * Example: legal_hwaddr("00:11:22:33:44:aB"); return true;
 * legal_hwaddr("00:11:22:33:44:5"); return false;
 * legal_hwaddr("00:11:22:33:44:HH"); return false; 
 */
int legal_hwaddr( char *value )
{
    unsigned int hwaddr[6];
    int tag = TRUE;
    int i, count;

    /*
     * Check for bad, multicast, broadcast, or null address 
     */
    for( i = 0, count = 0; *( value + i ); i++ )
    {
	if( *( value + i ) == ':' )
	{
	    if( ( i + 1 ) % 3 != 0 )
	    {
		tag = FALSE;
		break;
	    }
	    count++;
	}
	else if( isxdigit( *( value + i ) ) )	/* one of 0 1 2 3 4 5 6 7 8 9 
						 * a b c d e f A B C D E F */
	    continue;
	else
	{
	    tag = FALSE;
	    break;
	}
    }

    if( !tag || i != 17 || count != 5 )	/* must have 17's characters and 5's
					 * ':' */
	tag = FALSE;
    else if( sscanf( value, "%x:%x:%x:%x:%x:%x",
		     &hwaddr[0], &hwaddr[1], &hwaddr[2],
		     &hwaddr[3], &hwaddr[4], &hwaddr[5] ) != 6 )
    {
	// (hwaddr[0] & 1) || // the bit 7 is 1
	// (hwaddr[0] & hwaddr[1] & hwaddr[2] & hwaddr[3] & hwaddr[4] &
	// hwaddr[5]) == 0xff ){ // FF:FF:FF:FF:FF:FF
	// (hwaddr[0] | hwaddr[1] | hwaddr[2] | hwaddr[3] | hwaddr[4] |
	// hwaddr[5]) == 0x00){ // 00:00:00:00:00:00
	tag = FALSE;
    }
    else
	tag = TRUE;

    return tag;
}

/*
 * Example: 255.255.255.0 (111111111111111111111100000000) is a legal netmask
 * 255.255.0.255 (111111111111110000000011111111) is an illegal netmask 
 */
int legal_netmask( char *value )
{
    struct in_addr ipaddr;
    int ip[4] = { 0, 0, 0, 0 };
    int i, j;
    int match0 = -1;
    int match1 = -1;
    int ret, tag;

    ret = sscanf( value, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3] );

    if( ret == 4 && inet_aton( value, &ipaddr ) )
    {
	for( i = 3; i >= 0; i-- )
	{
	    for( j = 1; j <= 8; j++ )
	    {
		if( ( ip[i] % 2 ) == 0 )
		    match0 = ( 3 - i ) * 8 + j;
		else if( ( ( ip[i] % 2 ) == 1 ) && match1 == -1 )
		    match1 = ( 3 - i ) * 8 + j;
		ip[i] = ip[i] / 2;
	    }
	}
    }

    if( match0 >= match1 )
	tag = FALSE;
    else
	tag = TRUE;

    return tag;
}

/*
 * Example: legal_ipaddr("192.168.1.1"); return true;
 * legal_ipaddr("192.168.1.1111"); return false; 
 */
int legal_ipaddr( char *value )
{
    struct in_addr ipaddr;
    int ip[4];
    int ret, tag;

    ret = sscanf( value, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3] );

    if( ret != 4 || !inet_aton( value, &ipaddr ) )
	tag = FALSE;
    else
	tag = TRUE;

    return tag;
}

int valid_wep_key( webs_t wp, char *value, struct variable *v )
{
    int i;

    switch ( strlen( value ) )
    {
	case 5:
	case 13:
	    for( i = 0; *( value + i ); i++ )
	    {
		if( isascii( *( value + i ) ) )
		{
		    continue;
		}
		else
		{
		    websDebugWrite( wp,
				    "Invalid <b>%s</b> %s: must be ascii code<br>",
				    v->longname, value );
		    return FALSE;
		}
	    }
	    break;
	case 10:
	case 26:
	    for( i = 0; *( value + i ); i++ )
	    {
		if( isxdigit( *( value + i ) ) )
		{		/* one of 0 1 2 3 4 5 6 7 8 9 a b c d e f A B 
				 * C D E F */
		    continue;
		}
		else
		{
		    websDebugWrite( wp,
				    "Invalid <b>%s</b> %s: must be hexadecimal digits<br>",
				    v->longname, value );
		    return FALSE;
		}
	    }
	    break;

	default:
	    websDebugWrite( wp,
			    "Invalid <b>%s</b>: must be 5 or 13 ASCII characters or 10 or 26 hexadecimal digits<br>",
			    v->longname );
	    return FALSE;

    }

    /*
     * for(i=0 ; *(value+i) ; i++){ if(isxdigit(*(value+i))){ continue; }
     * else{ websDebugWrite(wp, "Invalid <b>%s</b> %s: must be hexadecimal
     * digits<br>", v->longname, value); return FALSE; } }
     * 
     * if (i != length) { websDebugWrite(wp, "Invalid <b>%s</b> %s: must be
     * %d characters<br>", v->longname, value,length); return FALSE; } 
     */
    return TRUE;
}

void validate_statics( webs_t wp, char *value, struct variable *v )
{

    if( !sv_valid_statics( value ) )
    {
	websDebugWrite( wp,
			"Invalid <b>%s</b> %s: not a legal statics entry<br>",
			v->longname, value );
	return;
    }

    nvram_set( v->name, value );
}

int valid_netmask( webs_t wp, char *value, struct variable *v )
{

    if( !legal_netmask( value ) )
    {
	websDebugWrite( wp, "Invalid <b>%s</b> %s: not a legal netmask<br>",
			v->longname, value );
	return FALSE;
    }

    return TRUE;

}

void validate_netmask( webs_t wp, char *value, struct variable *v )
{
    if( valid_netmask( wp, value, v ) )
	nvram_set( v->name, value );
}

void validate_merge_netmask( webs_t wp, char *value, struct variable *v )
{
    char netmask[20], maskname[30];
    char *mask;
    int i;

    strcpy( netmask, "" );
    for( i = 0; i < 4; i++ )
    {
	snprintf( maskname, sizeof( maskname ), "%s_%d", v->name, i );
	mask = websGetVar( wp, maskname, NULL );
	if( mask )
	{
	    strcat( netmask, mask );
	    if( i < 3 )
		strcat( netmask, "." );
	}
	else
	{
	    return;
	}
    }

    if( valid_netmask( wp, netmask, v ) )
	nvram_set( v->name, netmask );
}

// Added by Daniel(2004-07-29) for EZC
// char webs_buf[5000];
// int webs_buf_offset = 0;

void
validate_list( webs_t wp, char *value, struct variable *v,
	       int ( *valid ) ( webs_t, char *, struct variable * ) )
{
    int n, i;
    char name[100];
    char buf[1000] = "", *cur = buf;

    n = atoi( value );

    for( i = 0; i < n; i++ )
    {
	snprintf( name, sizeof( name ), "%s%d", v->name, i );
	if( !( value = websGetVar( wp, name, NULL ) ) )
	    return;
	if( !*value && v->nullok )
	    continue;
	if( !valid( wp, value, v ) )
	    continue;
	cur += snprintf( cur, buf + sizeof( buf ) - cur, "%s%s",
			 cur == buf ? "" : " ", value );
    }
    nvram_set( v->name, buf );

}

int valid_ipaddr( webs_t wp, char *value, struct variable *v )
{
    struct in_addr netaddr, netmask;

    if( !legal_ipaddr( value ) )
    {
	websDebugWrite( wp, "Invalid <b>%s</b> %s: not an IP address<br>",
			v->longname, value );
	return FALSE;
    }

    if( v->argv )
    {
	if( !strcmp( v->argv[0], "lan" ) )
	{
	    if( *( value + strlen( value ) - 2 ) == '.'
		&& *( value + strlen( value ) - 1 ) == '0' )
	    {
		websDebugWrite( wp,
				"Invalid <b>%s</b> %s: not an IP address<br>",
				v->longname, value );
		return FALSE;
	    }
	}

	else if( !legal_ip_netmask( v->argv[0], v->argv[1], value ) )
	{
	    ( void )inet_aton( nvram_safe_get( v->argv[0] ), &netaddr );
	    ( void )inet_aton( nvram_safe_get( v->argv[1] ), &netmask );
	    websDebugWrite( wp, "Invalid <b>%s</b> %s: not in the %s/",
			    v->longname, value, inet_ntoa( netaddr ) );
	    websDebugWrite( wp, "%s network<br>", inet_ntoa( netmask ) );
	    return FALSE;
	}
    }

    return TRUE;
}

void validate_ipaddr( webs_t wp, char *value, struct variable *v )
{
    if( valid_ipaddr( wp, value, v ) )
	nvram_set( v->name, value );
}

void validate_ipaddrs( webs_t wp, char *value, struct variable *v )
{
    validate_list( wp, value, v, valid_ipaddr );
}

int valid_merge_ip_4( webs_t wp, char *value, struct variable *v )
{
    char ipaddr[20];

    if( atoi( value ) == 255 )
    {
	websDebugWrite( wp, "Invalid <b>%s</b> %s: out of range 0 - 254 <br>",
			v->longname, value );
	return FALSE;
    }

    sprintf( ipaddr, "%d.%d.%d.%s",
	     get_single_ip( nvram_safe_get( "lan_ipaddr" ), 0 ),
	     get_single_ip( nvram_safe_get( "lan_ipaddr" ), 1 ),
	     get_single_ip( nvram_safe_get( "lan_ipaddr" ), 2 ), value );

    if( !valid_ipaddr( wp, ipaddr, v ) )
    {
	return FALSE;
    }

    return TRUE;
}

/*
 * static void validate_merge_ip_4 (webs_t wp, char *value, struct variable
 * *v) { if (!strcmp (value, "")) { nvram_set (v->name, "0"); return; }
 * 
 * if (valid_merge_ip_4 (wp, value, v)) nvram_set (v->name, value); } 
 */

/*
 * Example: lan_ipaddr_0 = 192 lan_ipaddr_1 = 168 lan_ipaddr_2 = 1
 * lan_ipaddr_3 = 1 get_merge_ipaddr("lan_ipaddr", ipaddr); produces
 * ipaddr="192.168.1.1" 
 */
int get_merge_ipaddr( webs_t wp, char *name, char *ipaddr )
{
    char ipname[30];
    int i;
    char buf[50] = { 0 };
    char *ip[4];
    char *tmp;

    // cprintf("ip addr\n");
    strcpy( ipaddr, "" );
    // cprintf("safe get\n");
    char *ipa = nvram_safe_get( name );

    // cprintf("strcpy\n");
    if( ipa == NULL )
	strcpy( buf, "0.0.0.0" );
    else
	strcpy( buf, ipa );
    // cprintf("strsep\n");
    char *b = ( char * )&buf;

    ip[0] = strsep( &b, "." );
    ip[1] = strsep( &b, "." );
    ip[2] = strsep( &b, "." );
    ip[3] = b;

    for( i = 0; i < 4; i++ )
    {
	// cprintf("merge %s_%d\n",name,i);
	snprintf( ipname, sizeof( ipname ), "%s_%d", name, i );
	tmp = websGetVar( wp, ipname, ip[i] );
	if( tmp == NULL )
	    return 0;
	strcat( ipaddr, tmp );
	if( i < 3 )
	    strcat( ipaddr, "." );
    }

    return 1;

}

void validate_merge_ipaddrs( webs_t wp, char *value, struct variable *v )
{
    char ipaddr[20];

    get_merge_ipaddr( wp, v->name, ipaddr );

    if( valid_ipaddr( wp, ipaddr, v ) )
	nvram_set( v->name, ipaddr );
}

/*
 * Example: wan_mac_0 = 00 wan_mac_1 = 11 wan_mac_2 = 22 wan_mac_3 = 33
 * wan_mac_4 = 44 wan_mac_5 = 55 get_merge_mac("wan_mac",mac); produces
 * mac="00:11:22:33:44:55" 
 */
int get_merge_mac( webs_t wp, char *name, char *macaddr )
{
    char macname[30];
    char *mac;
    int i;

    strcpy( macaddr, "" );

    for( i = 0; i < 6; i++ )
    {
	snprintf( macname, sizeof( macname ), "%s_%d", name, i );
	mac = websGetVar( wp, macname, "00" );
	if( strlen( mac ) == 1 )
	    strcat( macaddr, "0" );
	strcat( macaddr, mac );
	if( i < 5 )
	    strcat( macaddr, ":" );
    }

    return 1;

}

void validate_merge_mac( webs_t wp, char *value, struct variable *v )
{
    char macaddr[20];

    get_merge_mac( wp, v->name, macaddr );

    if( valid_hwaddr( wp, macaddr, v ) )
	nvram_set( v->name, macaddr );

}

void validate_dns( webs_t wp, char *value, struct variable *v )
{
    char buf[100] = "", *cur = buf;
    char ipaddr[20], ipname[30];
    char *ip;
    int i, j;

    for( j = 0; j < 3; j++ )
    {
	strcpy( ipaddr, "" );
	for( i = 0; i < 4; i++ )
	{
	    snprintf( ipname, sizeof( ipname ), "%s%d_%d", v->name, j, i );
	    ip = websGetVar( wp, ipname, NULL );
	    if( ip )
	    {
		strcat( ipaddr, ip );
		if( i < 3 )
		    strcat( ipaddr, "." );
	    }
	    else
		return;
	}

	if( !strcmp( ipaddr, "0.0.0.0" ) )
	    continue;
	if( !valid_ipaddr( wp, ipaddr, v ) )
	    continue;
	cur += snprintf( cur, buf + sizeof( buf ) - cur, "%s%s",
			 cur == buf ? "" : " ", ipaddr );
    }
    nvram_set( v->name, buf );

    dns_to_resolv(  );
}

int valid_choice( webs_t wp, char *value, struct variable *v )
{
    char **choice;

    for( choice = v->argv; *choice; choice++ )
    {
	if( !strcmp( value, *choice ) )
	    return TRUE;
    }

    websDebugWrite( wp, "Invalid <b>%s</b> %s: not one of ", v->longname,
		    value );
    for( choice = v->argv; *choice; choice++ )
	websDebugWrite( wp, "%s%s", choice == v->argv ? "" : "/", *choice );
    websDebugWrite( wp, "<br>" );
    return FALSE;
}

void validate_choice( webs_t wp, char *value, struct variable *v )
{
    if( valid_choice( wp, value, v ) )
	nvram_set( v->name, value );
}

void validate_noack( webs_t wp, char *value, struct variable *v )
{
    char *wme;

    /*
     * return if wme is not enabled 
     */
    if( !( wme = websGetVar( wp, "wl_wme", NULL ) ) )
	return;
    else if( strcmp( wme, "on" ) )
	return;

    validate_choice( wp, value, v );
}

int valid_range( webs_t wp, char *value, struct variable *v )
{
    int n, start, end;

    n = atoi( value );
    start = atoi( v->argv[0] );
    end = atoi( v->argv[1] );

    if( !ISDIGIT( value, 1 ) || n < start || n > end )
    {
	websDebugWrite( wp, "Invalid <b>%s</b> %s: out of range %d-%d<br>",
			v->longname, value, start, end );
	return FALSE;
    }

    return TRUE;
}

void validate_range( webs_t wp, char *value, struct variable *v )
{
    char buf[20];
    int range;

    if( valid_range( wp, value, v ) )
    {
	range = atoi( value );
	snprintf( buf, sizeof( buf ), "%d", range );
	nvram_set( v->name, buf );
    }
}

int valid_name( webs_t wp, char *value, struct variable *v )
{
    int n, max;

    n = atoi( value );

    if( !ISASCII( value, 1 ) )
    {
	return FALSE;
    }
    if( v )
    {
	max = atoi( v->argv[0] );
	if( strlen( value ) > max )
	{
	    return FALSE;
	}
    }
    return TRUE;
}

void validate_name( webs_t wp, char *value, struct variable *v )
{
    if( valid_name( wp, value, v ) )
	nvram_set( v->name, value );
}

void validate_reboot( webs_t wp, char *value, struct variable *v )
{
    if( value && v )
    {
	nvram_set( v->name, value );
	nvram_set( "do_reboot", "1" );
    }
}

/*
 * the html always show "d6nw5v1x2pc7st9m" so we must filter it. 
 */
void validate_password( webs_t wp, char *value, struct variable *v )
{
    if( strcmp( value, TMP_PASSWD ) && valid_name( wp, value, v ) )
    {
	nvram_set( v->name, zencrypt( value ) );

	system2( "/sbin/setpasswd" );
    }
}

void validate_password2( webs_t wp, char *value, struct variable *v )
{
    if( strcmp( value, TMP_PASSWD ) && valid_name( wp, value, v ) )
    {
	nvram_set( v->name, value );
    }
}

int valid_hwaddr( webs_t wp, char *value, struct variable *v )
{
    /*
     * Make exception for "NOT IMPLELEMENTED" string 
     */
    if( !strcmp( value, "NOT_IMPLEMENTED" ) )
	return ( TRUE );

    /*
     * Check for bad, multicast, broadcast, or null address 
     */
    if( !legal_hwaddr( value ) )
    {
	websDebugWrite( wp,
			"Invalid <b>%s</b> %s: not a legal MAC address<br>",
			v->longname, value );
	return FALSE;
    }

    return TRUE;
}

void validate_hwaddr( webs_t wp, char *value, struct variable *v )
{
    if( valid_hwaddr( wp, value, v ) )
	nvram_set( v->name, value );
}

void validate_hwaddrs( webs_t wp, char *value, struct variable *v )
{
    validate_list( wp, value, v, valid_hwaddr );
}

void validate_wan_ipaddr( webs_t wp, char *value, struct variable *v )
{
    char wan_ipaddr[20], wan_netmask[20], wan_gateway[20];
    char *wan_proto = websGetVar( wp, "wan_proto", NULL );
    char *pptp_use_dhcp = websGetVar( wp, "pptp_use_dhcp", NULL );

    int pptp_skip_check = FALSE;

    struct variable wan_variables[] = {
	{NULL},
	{NULL},
      {argv:ARGV( "wan_ipaddr", "wan_netmask" )},
    }, *which;

    which = &wan_variables[0];

    get_merge_ipaddr( wp, "wan_ipaddr", wan_ipaddr );
    get_merge_ipaddr( wp, "wan_netmask", wan_netmask );
    if( !strcmp( wan_proto, "pptp" ) )
    {
	nvram_set( "pptp_pass", "0" );	// disable pptp passthrough
	get_merge_ipaddr( wp, "pptp_server_ip", wan_gateway );
    }
    else
	get_merge_ipaddr( wp, "wan_gateway", wan_gateway );

    if( !strcmp( wan_proto, "pptp" ) && !strcmp( "0.0.0.0", wan_ipaddr ) )
    {				// Sveasoft: allow 0.0.0.0 for pptp IP addr
	pptp_skip_check = TRUE;
	nvram_set( "pptp_use_dhcp", "1" );
    }
    else
	nvram_set( "pptp_use_dhcp", "0" );

    if( FALSE == pptp_skip_check
	&& !valid_ipaddr( wp, wan_ipaddr, &which[0] ) )
	return;

    nvram_set( "wan_ipaddr_buf", nvram_safe_get( "wan_ipaddr" ) );
    nvram_set( "wan_ipaddr", wan_ipaddr );

    if( FALSE == pptp_skip_check
	&& !valid_netmask( wp, wan_netmask, &which[1] ) )
	return;

    nvram_set( "wan_netmask", wan_netmask );

    if( !valid_ipaddr( wp, wan_gateway, &which[2] )
	&& strcmp( wan_gateway, "0.0.0.0" ) )
	return;

    if( !strcmp( wan_proto, "pptp" ) )
	nvram_set( "pptp_server_ip", wan_gateway );
    else
	nvram_set( "wan_gateway", wan_gateway );

    if( !strcmp( wan_proto, "pptp" ) && !strcmp( pptp_use_dhcp, "1" ) )
    {
	if( !legal_ipaddr( wan_gateway ) )
	    return;
	nvram_set( "pptp_server_ip", wan_gateway );
	return;
    }
}

#ifdef HAVE_PORTSETUP
void validate_portsetup( webs_t wp, char *value, struct variable *v )
{
    char *next;
    char var[64];
    char eths[256];

    getIfLists( eths, 256 );
    foreach( var, eths, next )
    {
	char val[64];

	sprintf( val, "%s_bridged", var );
	char *bridged = websGetVar( wp, val, NULL );

	if( bridged )
	    nvram_set( val, bridged );

	sprintf( val, "%s_multicast", var );
	char *multicast = websGetVar( wp, val, NULL );

	if( multicast )
	    nvram_set( val, multicast );

	if( bridged && strcmp( bridged, "0" ) == 0 )
	{
	    sprintf( val, "%s_ipaddr", var );
	    char ipaddr[64];

	    if( get_merge_ipaddr( wp, val, ipaddr ) )
		nvram_set( val, ipaddr );
	    sprintf( val, "%s_netmask", var );
	    char netmask[64];

	    if( get_merge_ipaddr( wp, val, netmask ) )
		nvram_set( val, netmask );
	}
    }
    next = websGetVar( wp, "wan_ifname", NULL );
    if( next )
    {
	nvram_set( "wan_ifname2", next );
    }
}
#endif
int lan_ip_changed;

void validate_lan_ipaddr( webs_t wp, char *value, struct variable *v )
{
    char lan_ipaddr[20], lan_netmask[20];

    get_merge_ipaddr( wp, "lan_netmask", lan_netmask );
    get_merge_ipaddr( wp, v->name, lan_ipaddr );

    if( !valid_ipaddr( wp, lan_ipaddr, v ) )
	return;

    if( strcmp( nvram_safe_get( "lan_ipaddr" ), lan_ipaddr ) )
    {
	unlink( "/tmp/udhcpd.leases" );
	unlink( "/jffs/udhcpd.leases" );
	unlink( "/tmp/dnsmasq.leases" );
	unlink( "/jffs/dnsmasq.leases" );
    }
    if( strcmp( nvram_safe_get( "lan_netmask" ), lan_netmask ) )
    {
	unlink( "/tmp/udhcpd.leases" );
	unlink( "/jffs/udhcpd.leases" );
	unlink( "/tmp/dnsmasq.leases" );
	unlink( "/jffs/dnsmasq.leases" );
    }

    if( strcmp( lan_ipaddr, nvram_safe_get( "lan_ipaddr" ) ) ||
	strcmp( lan_netmask, nvram_safe_get( "lan_netmask" ) ) )
	lan_ip_changed = 1;
    else
	lan_ip_changed = 0;

    nvram_set( v->name, lan_ipaddr );
    nvram_set( "lan_netmask", lan_netmask );

}
#define SRL_VALID(v)        (((v) > 0) && ((v) <= 15))
#define SFBL_VALID(v)       (((v) > 0) && ((v) <= 15))
#define LRL_VALID(v)        (((v) > 0) && ((v) <= 15))
#define LFBL_VALID(v)       (((v) > 0) && ((v) <= 15))

void
validate_wl_wme_tx_params(webs_t wp, char *value, struct variable *v, char *varname)
{
	int srl = 0, sfbl = 0, lrl = 0, lfbl = 0, max_rate = 0, nmode = 0;
	char *s, *errmsg;
	char tmp[256];



	/* return if wme is not enabled */
	if (!(s = websGetVar(wp, "wl0_wme", NULL))) {
		return;
	} else if (!strcmp(s, "off")) {
		return;
	}

	/* return if afterburner enabled */
	if ((s = websGetVar(wp, "wl0_afterburner", NULL)) && (!strcmp(s, "auto"))) {
		return;
	}

	if (!value || atoi(value) != 5) {		/* Number of INPUTs */
		return;
	}

	s = nvram_get(v->name);

	if (s != NULL)
		sscanf(s, "%d %d %d %d %d", &srl, &sfbl, &lrl, &lfbl, &max_rate);

	if ((value = websGetVar(wp, strcat_r(v->name, "0", tmp), NULL)) != NULL)
		srl = atoi(value);

	if (!SRL_VALID(srl)) {
		errmsg = "Short Retry Limit must be in the range 1 to 15";
	return;
	}

	if ((value = websGetVar(wp, strcat_r(v->name, "1", tmp), NULL)) != NULL)
		sfbl = atoi(value);

	if (!SFBL_VALID(sfbl)) {
		errmsg = "Short Fallback Limit must be in the range 1 to 15";
	return;
	}

	if ((value = websGetVar(wp, strcat_r(v->name, "2", tmp), NULL)) != NULL)
		lrl = atoi(value);

	if (!LRL_VALID(lrl)) {
		errmsg = "Long Retry Limit must be in the range 1 to 15";
	return;
	}

	if ((value = websGetVar(wp, strcat_r(v->name, "3", tmp), NULL)) != NULL)
		lfbl = atoi(value);

	if (!LFBL_VALID(lfbl)) {
		errmsg = "Long Fallback Limit must be in the range 1 to 15";
	return;
	}

	if ((value = websGetVar(wp, strcat_r(v->name, "4", tmp), NULL)) != NULL)
		max_rate = atoi(value);

	s = nvram_get("wl0_nmode");
	if (s != NULL)
		nmode = atoi(s);

	sprintf(tmp, "%d %d %d %d %d",
	        srl, sfbl, lrl, lfbl, max_rate);

	nvram_set(v->name, tmp);


	return;

}

void validate_wl_wme_params( webs_t wp, char *value, struct variable *v )
{
    int n, i;
    int cwmin = 0, cwmax = 0;
    char *wme, *afterburner;
    char name[100];
    char buf[1000] = "", *cur = buf;
    struct
    {
	char *name;
	int range;
	char *arg1;
	char *arg2;
    } field_attrib[] =
    {
	{
	"WME AC CWmin", 1, "0", "32767"},
	{
	"WME AC CWmax", 1, "0", "32767"},
	{
	"WME AC AIFSN", 1, "1", "15"},
	{
	"WME AC TXOP(b)", 1, "0", "65504"},
	{
	"WME AC TXOP(a/g)", 1, "0", "65504"},
	{
	"WME AC Admin Forced", 0, "on", "off"}
    };

    /*
     * return if wme is not enabled 
     */
    if( !( wme = websGetVar( wp, "wl_wme", NULL ) ) )
	return;
    else if( strcmp( wme, "on" ) )
	return;

    /*
     * return if afterburner enabled 
     */
    if( ( afterburner = websGetVar( wp, "wl_afterburner", NULL ) )
	&& ( !strcmp( afterburner, "auto" ) ) )
	return;

    n = atoi( value ) + 1;

    for( i = 0; i < n; i++ )
    {
	snprintf( name, sizeof( name ), "%s%d", v->name, i );
	if( !( value = websGetVar( wp, name, NULL ) ) )
	    return;
	if( !*value && v->nullok )
	    continue;

	if( i == 0 )
	    cwmin = atoi( value );
	else if( i == 1 )
	{
	    cwmax = atoi( value );
	    if( cwmax < cwmin )
	    {
		websDebugWrite( wp,
				"Invalid <b>%s</b> %d: greater than <b>%s</b> %d<br>",
				field_attrib[0].name, cwmin,
				field_attrib[i].name, cwmax );
		return;
	    }
	}
	if( field_attrib[i].range )
	{
	    if( atoi( value ) < atoi( field_attrib[i].arg1 )
		|| atoi( value ) > atoi( field_attrib[i].arg2 ) )
	    {
		websDebugWrite( wp,
				"Invalid <b>%s</b> %d: should be in range %s to %s<br>",
				field_attrib[i].name, atoi( value ),
				field_attrib[i].arg1, field_attrib[i].arg2 );
		return;
	    }
	}
	else
	{
	    if( strcmp( value, field_attrib[i].arg1 )
		&& strcmp( value, field_attrib[i].arg2 ) )
	    {
		websDebugWrite( wp,
				"Invalid <b>%s</b> %s: should be %s or %s<br>",
				field_attrib[i].name, value,
				field_attrib[i].arg1, field_attrib[i].arg2 );
	    }
	}

	cur += snprintf( cur, buf + sizeof( buf ) - cur, "%s%s",
			 cur == buf ? "" : " ", value );
    }

    nvram_set( v->name, buf );
}

#ifndef HAVE_MSSID
void validate_security_mode( webs_t wp, char *value, struct variable *v )
{
    char *security_mode_last = websGetVar( wp, "security_mode_last", NULL );
    char *wl_wep_last = websGetVar( wp, "wl_wep_last", NULL );
    int from_index_page = 0;
    char *wl_wep = NULL;

    // If you don't press "Edit Security Setting" to set some value, and
    // direct select to enable "Wireless Security".
    // It'll returned, due to security_mode_buf is space.
    if( !strcmp( value, "enabled" ) )
    {
	if( nvram_match( "security_mode_last", "" ) )	// from index.asp and 
							// first time
	    return;
	else
	{
	    if( !security_mode_last )
	    {			// from index.asp
		from_index_page = 1;
		value = nvram_safe_get( "security_mode_last" );
		wl_wep = nvram_safe_get( "wl_wep_last" );
	    }
	    else
	    {			// from WL_WPATable.asp page
		value = websGetVar( wp, "security_mode_last", NULL );
		wl_wep = nvram_safe_get( "wl_wep_last" );
	    }
	}
    }

    if( !valid_choice( wp, value, v ) )
	return;

    if( !strcmp( value, "disabled" ) )
    {
	nvram_set( "security_mode", "disabled" );
	nvram_set( "wl_akm", "" );
	nvram_set( "wl_auth_mode", "none" );
	nvram_set( "wl_wep", "disabled" );
    }
    else if( !strcmp( value, "psk" ) )
    {
	nvram_set( "wl_akm", value );
	nvram_set( "wl_auth_mode", "none" );
	nvram_set( "wl_wep", "disabled" );
    }
    else if( !strcmp( value, "wpa" ) )
    {
	nvram_set( "wl_akm", value );
	nvram_set( "wl_auth_mode", "none" );
	nvram_set( "wl_wep", "disabled" );
    }
    else if( !strcmp( value, "radius" ) )
    {
	nvram_set( "security_mode", "radius" );
	nvram_set( "wl_akm", "" );
	nvram_set( "wl_auth_mode", "radius" );
	nvram_set( "wl_wep", "enabled" );	// the nas need this value,
						// the "restricted" is no
						// longer need. (20040624 by
						// honor)
    }
    else if( !strcmp( value, "wep" ) )
    {
	nvram_set( "wl_akm", "" );
	nvram_set( "wl_auth_mode", "none" );
	nvram_set( "wl_wep", "enabled" );	// the nas need this value,
						// the "restricted" is no
						// longer need. (20040624 by
						// honor)
    }
    else if( !strcmp( value, "psk2" ) )
    {				// WPA2 Only Mode
	nvram_set( "wl_akm", value );
	nvram_set( "wl_auth_mode", "none" );
	nvram_set( "wl_wep", "disabled" );
    }
    else if( !strcmp( value, "wpa2" ) )
    {				// WPA2 Only Mode
	nvram_set( "wl_akm", value );
	nvram_set( "wl_auth_mode", "none" );
	nvram_set( "wl_wep", "disabled" );
    }
    else if( !strcmp( value, "psk psk2" ) )
    {				// WPA2 Mixed Mode
	nvram_set( "wl_akm", value );
	nvram_set( "wl_auth_mode", "none" );
	nvram_set( "wl_wep", "disabled" );
    }
    else if( !strcmp( value, "wpa wpa2" ) )
    {				// WPA2 Mixed Mode
	nvram_set( "wl_akm", value );
	nvram_set( "wl_auth_mode", "none" );
	nvram_set( "wl_wep", "disabled" );
    }

    if( security_mode_last )
	nvram_set( "security_mode_last", security_mode_last );

    if( wl_wep_last )
	nvram_set( "wl_wep_last", wl_wep_last );

    nvram_set( v->name, value );
}
#endif
void validate_wl_key( webs_t wp, char *value, struct variable *v )
{
    char *c;

    switch ( strlen( value ) )
    {
	case 5:
	case 13:
	    break;
	case 10:
	case 26:
	    for( c = value; *c; c++ )
	    {
		if( !isxdigit( *c ) )
		{
		    websDebugWrite( wp,
				    "Invalid <b>%s</b>: character %c is not a hexadecimal digit<br>",
				    v->longname, *c );
		    return;
		}
	    }
	    break;
	default:
	    websDebugWrite( wp,
			    "Invalid <b>%s</b>: must be 5 or 13 ASCII characters or 10 or 26 hexadecimal digits<br>",
			    v->longname );
	    return;
    }

    nvram_set( v->name, value );
}

#ifndef GMODE_AFTERBURNER
#	define GMODE_AFTERBURNER 7
#endif

void validate_wl_wep( webs_t wp, char *value, struct variable *v )
{
    if( !valid_choice( wp, value, v ) )
	return;
#ifdef ABURN_WSEC_CHECK
    if( strcmp( value, "off" )
	&& atoi( nvram_safe_get( "wl_gmode" ) ) == GMODE_AFTERBURNER )
    {
	websDebugWrite( wp,
			"<br>Invalid <b>%s</b>: must be set to <b>Off</b> when 54g Mode is AfterBurner.",
			v->longname );
	return;
    }
#endif
    nvram_set( v->name, value );
}

void validate_auth_mode( webs_t wp, char *value, struct variable *v )
{
    if( !valid_choice( wp, value, v ) )
	return;
    nvram_set( v->name, value );
}

void validate_wpa_psk( webs_t wp, char *value, struct variable *v )
{
    int len = strlen( value );
    char *c;

    if( len == 64 )
    {
	for( c = value; *c; c++ )
	{
	    if( !isxdigit( ( int )*c ) )
	    {
		websDebugWrite( wp,
				"Invalid <b>%s</b>: character %c is not a hexadecimal digit<br>",
				v->longname, *c );
		return;
	    }
	}
    }
    else if( len < 8 || len > 63 )
    {
	websDebugWrite( wp,
			"Invalid <b>%s</b>: must be between 8 and 63 ASCII characters or 64 hexadecimal digits<br>",
			v->longname );
	return;
    }

    nvram_set( v->name, value );
}

#ifdef HAVE_MSSID

void validate_wl_wep_key( webs_t wp, char *value, struct variable *v )
{
    char buf[200] = "";
    int error_value = 0;
    struct variable wl_wep_variables[] = {
      {argv:ARGV( "16" )},
      {argv:ARGV( "5", "10" )},
	// for 64 bit
      {argv:ARGV( "13", "26" )},
	// for 128 bit
      {argv:ARGV( "1", "4" )},
    }, *which;

    char *wep_bit = "", *wep_passphrase = "", *wep_key1 = "", *wep_key2 =
	"", *wep_key3 = "", *wep_key4 = "", *wep_tx = "";
    char new_wep_passphrase[50] = "", new_wep_key1[30] =
	"", new_wep_key2[30] = "", new_wep_key3[30] = "", new_wep_key4[30] =
	"";
    int index;

    which = &wl_wep_variables[0];

    wep_bit = websGetVar( wp, "wl_wep_bit", NULL );	// 64 or 128
    if( !wep_bit )
	return;
    if( strcmp( wep_bit, "64" ) && strcmp( wep_bit, "128" ) )
	return;

    wep_passphrase = websGetVar( wp, "wl_passphrase", "" );
    // if(!wep_passphrase) return ;

    // strip_space(wep_passphrase);
    if( strcmp( wep_passphrase, "" ) )
    {
	if( !valid_name( wp, wep_passphrase, &which[0] ) )
	{
	    error_value = 1;
	}
	else
	{
	    httpd_filter_name( wep_passphrase, new_wep_passphrase,
			       sizeof( new_wep_passphrase ), SET );
	}
    }

    wep_key1 = websGetVar( wp, "wl_key1", "" );
    wep_key2 = websGetVar( wp, "wl_key2", "" );
    wep_key3 = websGetVar( wp, "wl_key3", "" );
    wep_key4 = websGetVar( wp, "wl_key4", "" );
    wep_tx = websGetVar( wp, "wl_key", NULL );

    if( !wep_tx )
    {
	error_value = 1;
	return;
    }

    index = ( atoi( wep_bit ) == 64 ) ? 1 : 2;

    if( strcmp( wep_key1, "" ) )
    {
	if( !valid_wep_key( wp, wep_key1, &which[index] ) )
	{
	    error_value = 1;
	}
	else
	{
	    httpd_filter_name( wep_key1, new_wep_key1, sizeof( new_wep_key1 ),
			       SET );
	}

    }
    if( strcmp( wep_key2, "" ) )
    {
	if( !valid_wep_key( wp, wep_key2, &which[index] ) )
	{
	    error_value = 1;
	}
	else
	{
	    httpd_filter_name( wep_key2, new_wep_key2, sizeof( new_wep_key2 ),
			       SET );
	}
    }
    if( strcmp( wep_key3, "" ) )
    {
	if( !valid_wep_key( wp, wep_key3, &which[index] ) )
	{
	    error_value = 1;
	}
	else
	{
	    httpd_filter_name( wep_key3, new_wep_key3, sizeof( new_wep_key3 ),
			       SET );
	}
    }
    if( strcmp( wep_key4, "" ) )
    {
	if( !valid_wep_key( wp, wep_key4, &which[index] ) )
	{
	    error_value = 1;
	}
	else
	{
	    httpd_filter_name( wep_key4, new_wep_key4, sizeof( new_wep_key4 ),
			       SET );
	}
    }

    if( !error_value )
    {
	snprintf( buf, sizeof( buf ), "%s:%s:%s:%s:%s:%s", new_wep_passphrase,
		  new_wep_key1, new_wep_key2, new_wep_key3, new_wep_key4,
		  wep_tx );
	nvram_set( "wl_wep_bit", wep_bit );
	nvram_set( "wl_wep_buf", buf );

	nvram_set( "wl_passphrase", wep_passphrase );
	nvram_set( "wl_key", wep_tx );
	nvram_set( "wl_key1", wep_key1 );
	nvram_set( "wl_key2", wep_key2 );
	nvram_set( "wl_key3", wep_key3 );
	nvram_set( "wl_key4", wep_key4 );

	if( !strcmp( wep_key1, "" ) && !strcmp( wep_key2, "" ) && !strcmp( wep_key3, "" ) && !strcmp( wep_key4, "" ) )	// Allow 
															// null 
															// wep
	    nvram_set( "wl_wep", "off" );
	else
	    nvram_set( "wl_wep", "restricted" );
    }

}

#else

void validate_wl_wep_key( webs_t wp, char *value, struct variable *v )
{
    char buf[200] = "";
    struct variable wl_wep_variables[] = {
      {argv:ARGV( "16" )},
      {argv:ARGV( "5", "10" )},
	// for 64 bit
      {argv:ARGV( "13", "26" )},
	// for 128 bit
      {argv:ARGV( "1", "4" )},
    }, *which;
    int error_value = 0;
    char *wep_bit = "", *wep_passphrase = "", *wep_key1 = "", *wep_key2 =
	"", *wep_key3 = "", *wep_key4 = "", *wep_tx = "";
    char new_wep_passphrase[50] = "", new_wep_key1[30] =
	"", new_wep_key2[30] = "", new_wep_key3[30] = "", new_wep_key4[30] =
	"";
    int index;

    which = &wl_wep_variables[0];

    wep_bit = websGetVar( wp, "wl_wep_bit", NULL );	// 64 or 128
    if( !wep_bit )
	return;
    if( strcmp( wep_bit, "64" ) && strcmp( wep_bit, "128" ) )
	return;

    wep_passphrase = websGetVar( wp, "wl_passphrase", "" );
    // if(!wep_passphrase) return ;

    // strip_space(wep_passphrase);
    if( strcmp( wep_passphrase, "" ) )
    {
	if( !valid_name( wp, wep_passphrase, &which[0] ) )
	{
	    error_value = 1;
	}
	else
	{
	    httpd_filter_name( wep_passphrase, new_wep_passphrase,
			       sizeof( new_wep_passphrase ), SET );
	}
    }

    wep_key1 = websGetVar( wp, "wl_key1", "" );
    wep_key2 = websGetVar( wp, "wl_key2", "" );
    wep_key3 = websGetVar( wp, "wl_key3", "" );
    wep_key4 = websGetVar( wp, "wl_key4", "" );
    wep_tx = websGetVar( wp, "wl_key", NULL );

    if( !wep_tx )
    {
	error_value = 1;
	return;
    }

    index = ( atoi( wep_bit ) == 64 ) ? 1 : 2;

    if( strcmp( wep_key1, "" ) )
    {
	if( !valid_wep_key( wp, wep_key1, &which[index] ) )
	{
	    error_value = 1;
	}
	else
	{
	    httpd_filter_name( wep_key1, new_wep_key1, sizeof( new_wep_key1 ),
			       SET );
	}

    }
    if( strcmp( wep_key2, "" ) )
    {
	if( !valid_wep_key( wp, wep_key2, &which[index] ) )
	{
	    error_value = 1;
	}
	else
	{
	    httpd_filter_name( wep_key2, new_wep_key2, sizeof( new_wep_key2 ),
			       SET );
	}
    }
    if( strcmp( wep_key3, "" ) )
    {
	if( !valid_wep_key( wp, wep_key3, &which[index] ) )
	{
	    error_value = 1;
	}
	else
	{
	    httpd_filter_name( wep_key3, new_wep_key3, sizeof( new_wep_key3 ),
			       SET );
	}
    }
    if( strcmp( wep_key4, "" ) )
    {
	if( !valid_wep_key( wp, wep_key4, &which[index] ) )
	{
	    error_value = 1;
	}
	else
	{
	    httpd_filter_name( wep_key4, new_wep_key4, sizeof( new_wep_key4 ),
			       SET );
	}
    }

    if( !error_value )
    {
	snprintf( buf, sizeof( buf ), "%s:%s:%s:%s:%s:%s", new_wep_passphrase,
		  new_wep_key1, new_wep_key2, new_wep_key3, new_wep_key4,
		  wep_tx );
	nvram_set( "wl_wep_bit", wep_bit );
	nvram_set( "wl_wep_buf", buf );

	nvram_set( "wl_passphrase", wep_passphrase );
	nvram_set( "wl_key", wep_tx );
	nvram_set( "wl_key1", wep_key1 );
	nvram_set( "wl_key2", wep_key2 );
	nvram_set( "wl_key3", wep_key3 );
	nvram_set( "wl_key4", wep_key4 );

	if( !strcmp( wep_key1, "" ) && !strcmp( wep_key2, "" ) && !strcmp( wep_key3, "" ) && !strcmp( wep_key4, "" ) )	// Allow 
															// null 
															// wep
	    nvram_set( "wl_wep", "off" );
	else
	    nvram_set( "wl_wep", "restricted" );
    }

}
#endif
void validate_wl_auth( webs_t wp, char *value, struct variable *v )
{
    if( !valid_choice( wp, value, v ) )
	return;
    /*
     * // not to check , spec for linksys if (atoi(value) == 1) { char
     * wl_key[] = "wl_keyXXX";
     * 
     * snprintf(wl_key, sizeof(wl_key), "wl_key%s",
     * nvram_safe_get("wl_key")); if (!strlen(nvram_safe_get(wl_key))) {
     * websDebugWrite(wp, "Invalid <b>%s</b>: must first specify a valid
     * <b>Network Key</b><br>", v->longname); return; } } 
     */
    nvram_set( v->name, value );
}

/*
 * Example: 00:11:22:33:44:55=1 00:12:34:56:78:90=0 (ie 00:11:22:33:44:55 if
 * filterd, and 00:12:34:56:78:90 is not) wl_maclist = "00:11:22:33:44:55" 
 */
void validate_wl_hwaddrs( webs_t wp, char *value, struct variable *v )
{
    int i;
    int error_value = 0;
    char buf[19 * WL_FILTER_MAC_NUM * WL_FILTER_MAC_PAGE] = "", *cur = buf;
    char *wordlist;
    unsigned int m[6];
    char *ifname = websGetVar( wp, "ifname", NULL );	// 64 or 128

    if( ifname == NULL )
	return;
    char mlist[32];

    sprintf( mlist, "%s_maclist", ifname );
    wordlist = nvram_safe_get( mlist );
    if( !wordlist )
	return;

    for( i = 0; i < WL_FILTER_MAC_NUM * WL_FILTER_MAC_PAGE; i++ )
    {
	char filter_mac[] = "ath10.99_macXXX";
	char *mac = NULL;
	char mac1[20];

	snprintf( filter_mac, sizeof( filter_mac ), "%s%s%d", ifname, "_mac",
		  i );

	mac = websGetVar( wp, filter_mac, NULL );

	if( !mac || !strcmp( mac, "0" ) || !strcmp( mac, "" ) )
	{
	    continue;
	}

	if( strlen( mac ) == 12 )
	{
	    sscanf( mac, "%02X%02X%02X%02X%02X%02X", &m[0],
		    &m[1], &m[2], &m[3], &m[4], &m[5] );
	    sprintf( mac1, "%02X:%02X:%02X:%02X:%02X:%02X", m[0], m[1], m[2],
		     m[3], m[4], m[5] );
	}
	else if( strlen( mac ) == 17 )
	{
	    sscanf( mac, "%02X:%02X:%02X:%02X:%02X:%02X", &m[0],
		    &m[1], &m[2], &m[3], &m[4], &m[5] );
	    sprintf( mac1, "%02X:%02X:%02X:%02X:%02X:%02X", m[0], m[1], m[2],
		     m[3], m[4], m[5] );
	}
	else
	{
	    mac1[0] = 0;
	}

	if( !valid_hwaddr( wp, mac1, v ) )
	{
	    error_value = 1;
	    continue;
	}
	cur += snprintf( cur, buf + sizeof( buf ) - cur, "%s%s",
			 cur == buf ? "" : " ", mac1 );

    }

    if( !error_value )
    {
	nvram_set( v->name, buf );
	nvram_set( mlist, buf );
	nvram_set( "wl_active_mac", "" );
	nvram_set( "wl0_active_mac", "" );
    }
}

/*
 * Example: name:[on|off]:[tcp|udp|both]:8000:80>100 
 */

void validate_forward_proto( webs_t wp, char *value, struct variable *v )
{
    int i, error = 0;
    char *buf, *cur;
    int count, sof;
    struct variable forward_proto_variables[] = {
      {argv:ARGV( "12" )},
      {argv:ARGV( "0", "65535" )},
      {argv:ARGV( "0", "65535" )},
	{NULL},
    }, *which;
    buf = nvram_safe_get( "forward_entries" );
    if( buf == NULL || strlen( buf ) == 0 )
	return;
    count = atoi( buf );
    sof = ( count * 128 ) + 1;
    buf = ( char * )malloc( sof );
    cur = buf;
    buf[0] = 0;

    for( i = 0; i < count; i++ )
    {

	char forward_name[] = "nameXXX";
	char forward_from[] = "fromXXX";
	char forward_to[] = "toXXX";
	char forward_ip[] = "ipXXX";
	char forward_tcp[] = "tcpXXX";	// for checkbox
	char forward_udp[] = "udpXXX";	// for checkbox
	char forward_pro[] = "proXXX";	// for select, cisco style UI
	char forward_enable[] = "enableXXX";
	char *name = "", new_name[200] = "", *from = "", *to = "", *ip =
	    "", *tcp = "", *udp = "", *enable = "", proto[10], *pro = "";

	snprintf( forward_name, sizeof( forward_name ), "name%d", i );
	snprintf( forward_from, sizeof( forward_from ), "from%d", i );
	snprintf( forward_to, sizeof( forward_to ), "to%d", i );
	snprintf( forward_ip, sizeof( forward_ip ), "ip%d", i );
	snprintf( forward_tcp, sizeof( forward_tcp ), "tcp%d", i );
	snprintf( forward_udp, sizeof( forward_udp ), "udp%d", i );
	snprintf( forward_enable, sizeof( forward_enable ), "enable%d", i );
	snprintf( forward_pro, sizeof( forward_pro ), "pro%d", i );

	name = websGetVar( wp, forward_name, "" );
	from = websGetVar( wp, forward_from, "0" );
	to = websGetVar( wp, forward_to, "0" );
	ip = websGetVar( wp, forward_ip, "0" );
	tcp = websGetVar( wp, forward_tcp, NULL );	// for checkbox
	udp = websGetVar( wp, forward_udp, NULL );	// for checkbox
	pro = websGetVar( wp, forward_pro, NULL );	// for select option
	enable = websGetVar( wp, forward_enable, "off" );

	which = &forward_proto_variables[0];

	if( !*from && !*to && !*ip )
	    continue;
	if( !strcmp( ip, "0" ) || !strcmp( ip, "" ) )
	    continue;
	if( ( !strcmp( from, "0" ) || !strcmp( from, "" ) ) &&
	    ( !strcmp( to, "0" ) || !strcmp( to, "" ) ) &&
	    ( !strcmp( ip, "0" ) || !strcmp( ip, "" ) ) )
	{
	    continue;
	}

	/*
	 * check name 
	 */
	if( strcmp( name, "" ) )
	{
	    if( !valid_name( wp, name, &which[0] ) )
	    {
		continue;
	    }
	    else
	    {
		httpd_filter_name( name, new_name, sizeof( new_name ), SET );
	    }
	}

	if( !strcmp( from, "" ) )
	    from = to;
	if( !strcmp( to, "" ) )
	    to = from;

	if( atoi( from ) > atoi( to ) )
	{
	    SWAP( from, to );
	}

	if( !valid_range( wp, from, &which[1] )
	    || !valid_range( wp, to, &which[2] ) )
	{
	    continue;
	}

	if( pro )
	{			// use select option
	    strcpy( proto, pro );
	}
	else
	{			// use checkbox
	    if( tcp && udp )
		strcpy( proto, "both" );
	    else if( tcp && !udp )
		strcpy( proto, "tcp" );
	    else if( !tcp && udp )
		strcpy( proto, "udp" );
	}
	/*
	 * check ip address 
	 */

	if( !*ip )
	{
	    error = 1;
	    // websWrite(wp, "Invalid <b>%s</b> : must specify a
	    // ip<br>",which[4].longname);
	    continue;
	}

	/*
	 * Sveasoft add - new format allows full IP address 
	 */
	if( sv_valid_ipaddr( ip ) )
	{
	    cur += snprintf( cur, buf + sof - cur, "%s%s:%s:%s:%d:%d>%s",
			     cur == buf ? "" : " ", new_name, enable, proto,
			     atoi( from ), atoi( to ), ip );
	}
	/*
	 * Sveasoft - for backwords compatability allow single number 
	 */
	else if( sv_valid_range( ip, 0, 254 ) )
	{
	    char fullip[16] = { 0 };
	    int f_ip[4];

	    sscanf( nvram_safe_get( "lan_ipaddr" ), "%d.%d.%d.%d", &f_ip[0],
		    &f_ip[1], &f_ip[2], &f_ip[3] );
	    snprintf( fullip, 15, "%d.%d.%d.%d", f_ip[0], f_ip[1], f_ip[2],
		      atoi( ip ) );
	    cur +=
		snprintf( cur, buf + sof - cur, "%s%s:%s:%s:%d:%d>%s",
			  cur == buf ? "" : " ", new_name, enable, proto,
			  atoi( from ), atoi( to ), fullip );

	}
	else
	{
	    error = 1;
	    continue;
	}

    }
    if( !error )
	nvram_set( v->name, buf );
    free( buf );
}

/*
 * Example: name:[on|off]:[tcp|udp|both]:8000:80>100 
 */

void validate_forward_spec( webs_t wp, char *value, struct variable *v )
{
    int i, error = 0;
    char *buf, *cur;
    int count, sof;
    struct variable forward_proto_variables[] = {
      {argv:ARGV( "12" )},
      {argv:ARGV( "0", "65535" )},
      {argv:ARGV( "0", "65535" )},
	{NULL},
    }, *which;
    buf = nvram_safe_get( "forwardspec_entries" );
    if( buf == NULL || strlen( buf ) == 0 )
	return;
    count = atoi( buf );
    sof = ( count * 128 ) + 1;
    buf = ( char * )malloc( sof );
    cur = buf;
    buf[0] = 0;

    for( i = 0; i < count; i++ )
    {

	char forward_name[] = "nameXXX";
	char forward_from[] = "fromXXX";
	char forward_to[] = "toXXX";
	char forward_ip[] = "ipXXX";
	char forward_tcp[] = "tcpXXX";	// for checkbox
	char forward_udp[] = "udpXXX";	// for checkbox
	char forward_pro[] = "proXXX";	// for select, cisco style UI
	char forward_enable[] = "enableXXX";
	char *name = "", new_name[200] = "", *from = "", *to = "", *ip =
	    "", *tcp = "", *udp = "", *enable = "", proto[10], *pro = "";

	snprintf( forward_name, sizeof( forward_name ), "name%d", i );
	snprintf( forward_from, sizeof( forward_from ), "from%d", i );
	snprintf( forward_to, sizeof( forward_to ), "to%d", i );
	snprintf( forward_ip, sizeof( forward_ip ), "ip%d", i );
	snprintf( forward_tcp, sizeof( forward_tcp ), "tcp%d", i );
	snprintf( forward_udp, sizeof( forward_udp ), "udp%d", i );
	snprintf( forward_enable, sizeof( forward_enable ), "enable%d", i );
	snprintf( forward_pro, sizeof( forward_pro ), "pro%d", i );

	name = websGetVar( wp, forward_name, "" );
	from = websGetVar( wp, forward_from, "0" );
	to = websGetVar( wp, forward_to, "0" );
	ip = websGetVar( wp, forward_ip, "0" );
	tcp = websGetVar( wp, forward_tcp, NULL );	// for checkbox
	udp = websGetVar( wp, forward_udp, NULL );	// for checkbox
	pro = websGetVar( wp, forward_pro, NULL );	// for select option
	enable = websGetVar( wp, forward_enable, "off" );

	which = &forward_proto_variables[0];

	if( !*from && !*to && !*ip )
	    continue;
	if( !strcmp( ip, "0" ) || !strcmp( ip, "" ) )
	    continue;
	if( ( !strcmp( from, "0" ) || !strcmp( from, "" ) ) &&
	    ( !strcmp( to, "0" ) || !strcmp( to, "" ) ) &&
	    ( !strcmp( ip, "0" ) || !strcmp( ip, "" ) ) )
	{
	    continue;
	}

	/*
	 * check name 
	 */
	if( strcmp( name, "" ) )
	{
	    if( !valid_name( wp, name, &which[0] ) )
	    {
		continue;
	    }
	    else
	    {
		httpd_filter_name( name, new_name, sizeof( new_name ), SET );
	    }
	}

	if( !strcmp( from, "" ) )
	    from = to;
	if( !strcmp( to, "" ) )
	    to = from;

	/*
	 * if(atoi(from) > atoi(to)){ SWAP(from, to); } 
	 */

	if( !valid_range( wp, from, &which[1] )
	    || !valid_range( wp, to, &which[2] ) )
	{
	    continue;
	}

	if( pro )
	{			// use select option
	    strcpy( proto, pro );
	}
	else
	{			// use checkbox
	    if( tcp && udp )
		strcpy( proto, "both" );
	    else if( tcp && !udp )
		strcpy( proto, "tcp" );
	    else if( !tcp && udp )
		strcpy( proto, "udp" );
	}
	/*
	 * check ip address 
	 */

	if( !*ip )
	{
	    error = 1;
	    // websWrite(wp, "Invalid <b>%s</b> : must specify a
	    // ip<br>",which[4].longname);
	    continue;
	}

	/*
	 * Sveasoft add - new format allows full IP address 
	 */
	if( sv_valid_ipaddr( ip ) )
	{
	    cur += snprintf( cur, buf + sof - cur, "%s%s:%s:%s:%d>%s:%d",
			     cur == buf ? "" : " ", new_name, enable, proto,
			     atoi( from ), ip, atoi( to ) );
	}
	/*
	 * Sveasoft - for backwords compatability allow single number 
	 */
	else if( sv_valid_range( ip, 0, 254 ) )
	{
	    char fullip[16] = { 0 };
	    int f_ip[4];

	    sscanf( nvram_safe_get( "lan_ipaddr" ), "%d.%d.%d.%d", &f_ip[0],
		    &f_ip[1], &f_ip[2], &f_ip[3] );
	    snprintf( fullip, 15, "%d.%d.%d.%d", f_ip[0], f_ip[1], f_ip[2],
		      atoi( ip ) );
	    cur +=
		snprintf( cur, buf + sof - cur, "%s%s:%s:%s:%d>%s:%d",
			  cur == buf ? "" : " ", new_name, enable, proto,
			  atoi( from ), fullip, atoi( to ) );

	}
	else
	{
	    error = 1;
	    continue;
	}

    }
    if( !error )
	nvram_set( v->name, buf );
    free( buf );
}

void validate_dynamic_route( webs_t wp, char *value, struct variable *v )
{
    struct variable dr_variables[] = {
      {argv:ARGV( "0", "1", "2", "3" )},
    }, *which;
    char *dr_setting;

    which = &dr_variables[0];

    if( valid_choice( wp, value, v ) )
	nvram_set( v->name, value );

    dr_setting = websGetVar( wp, "dr_setting", NULL );
    if( !dr_setting )
	return;

    if( !valid_choice( wp, dr_setting, &which[0] ) )
	return;

    nvram_set( "dr_setting", dr_setting );

    if( !dr_setting || atoi( dr_setting ) == 0 )
    {
	nvram_set( "dr_lan_tx", "0" );
	nvram_set( "dr_lan_rx", "0" );
	nvram_set( "dr_wan_tx", "0" );
	nvram_set( "dr_wan_rx", "0" );
    }
    else if( atoi( dr_setting ) == 1 )
    {
	nvram_set( "dr_lan_tx", "1 2" );
	nvram_set( "dr_lan_rx", "1 2" );
	nvram_set( "dr_wan_tx", "0" );
	nvram_set( "dr_wan_rx", "0" );
    }
    else if( atoi( dr_setting ) == 2 )
    {
	nvram_set( "dr_lan_tx", "0" );
	nvram_set( "dr_lan_rx", "0" );
	nvram_set( "dr_wan_tx", "1 2" );
	nvram_set( "dr_wan_rx", "1 2" );
    }
    else if( atoi( dr_setting ) == 3 )
    {
	nvram_set( "dr_lan_tx", "1 2" );
	nvram_set( "dr_lan_rx", "1 2" );
	nvram_set( "dr_wan_tx", "1 2" );
	nvram_set( "dr_wan_rx", "1 2" );
    }
    else
    {
	nvram_set( "dr_lan_tx", "0" );
	nvram_set( "dr_lan_rx", "0" );
	nvram_set( "dr_wan_tx", "0" );
	nvram_set( "dr_wan_rx", "0" );
    }

    /*
     * <lonewolf> 
     */
    if( atoi( websGetVar( wp, "dyn_default", "0" ) ) == 1 )
	nvram_set( "dyn_default", "1" );
    else
	nvram_set( "dyn_default", "0" );

    if( nvram_match( "wk_mode", "ospf" ) )
    {
	nvram_set( "zebra_conf", websGetVar( wp, "zebra_conf", "" ) );
	nvram_set( "ospfd_conf", websGetVar( wp, "ospfd_conf", "" ) );
	nvram_set( "zebra_copt", websGetVar( wp, "zebra_copt", "0" ) );
	nvram_set( "ospfd_copt", websGetVar( wp, "ospfd_copt", "0" ) );
    }
    /*
     * </lonewolf> 
     */
}
void validate_wl_gmode( webs_t wp, char *value, struct variable *v )
{
    if( !valid_choice( wp, value, v ) )
	return;
    if( atoi( value ) == GMODE_AFTERBURNER )
    {
	nvram_set( "wl_lazywds", "0" );
	nvram_set( "wl_wds", "" );
	nvram_set( "wl_mode", "ap" );
	/*
	 * if(nvram_invmatch("security_mode", "disabled") &&
	 * nvram_invmatch("security_mode", "wep")){
	 * nvram_set("security_mode", "disabled");
	 * nvram_set("security_mode_last", nvram_safe_get("security_mode"));
	 * nvram_set("security_mode", "disabled"); } 
	 */
    }
    nvram_set( v->name, value );

    return;
    /*
     * force certain wireless variables to fixed values 
     */
    if( atoi( value ) == GMODE_AFTERBURNER )
    {
	if( nvram_invmatch( "wl_auth_mode", "disabled" ) ||
#ifdef ABURN_WSEC_CHECK
	    nvram_invmatch( "wl_wep", "off" ) ||
#endif
	    nvram_invmatch( "wl_mode", "ap" ) ||
	    nvram_invmatch( "wl_lazywds", "0" )
	    || nvram_invmatch( "wl_wds", "" ) )
	{
	    /*
	     * notify the user 
	     */
	    /*
	     * #ifdef ABURN_WSEC_CHECK websWrite (wp, "Invalid <b>%s</b>:
	     * AfterBurner mode requires:" "<br><b>Network Authentication</b> 
	     * set to <b>Disabled</b>" "<br><b>Data Encryption</b> set to
	     * <b>Off</b>" "<br><b>Mode</b> set to <b>Access Point</b>" //
	     * "<br><b>Bridge Restrict</b> set to <b>Enabled</b>" "<br><b>WDS 
	     * devices</b> disabled." "<br>", v->name); #else websWrite (wp,
	     * "Invalid <b>%s</b>: AfterBurner mode requires:"
	     * "<br><b>Network Authentication</b> set to <b>Disabled</b>"
	     * "<br><b>Mode</b> set to <b>Access Point</b>" // "<br><b>Bridge 
	     * Restrict</b> set to <b>Enabled</b>" "<br><b>WDS devices</b>
	     * disabled." "<br>", v->name); #endif
	     */
	    return;
	}
    }
}

/*
 * UI Mode GMODE Afterburner Override Basic Rate Set FrameBurst CTS
 * Protection Mixed 6 - AfterBurner -1 Default ON -1(auto) 54g-Only 6 -
 * AfterBurner -1 ALL ON 0(off) 11b-Only 0 - 54g Legacy B NA Default ON
 * -1(auto) 
 */

/*
 * Sveasoft note: settings for b-only, mixed, and g-mode set back to original 
 * defaults before "afterburner" mods. Afterburner bizarre settings
 * maintained for "speedbooster" mode 
 */

void convert_wl_gmode( char *value, char *prefix )
{
    /*
     * if (nvram_match("wl_mode","ap")) { if(!strcmp(value, "disabled")){
     * nvram_set("wl_net_mode", value); nvram_set("wl_gmode", "-1"); } else
     * if(!strcmp(value, "mixed")){ nvram_set("wl_net_mode", value);
     * nvram_set("wl_gmode", "6"); nvram_set("wl_afterburner", "auto"); //
     * From 3.61.13.0 // nvram_set("wl_afterburner_override", "-1");
     * nvram_set("wl_rateset", "default"); nvram_set("wl_frameburst", "on");
     * // nvram_set("wl_gmode_protection", "off"); } else if(!strcmp(value,
     * "g-only")){ nvram_set("wl_net_mode", value); // In order to backward
     * compatiable old firmware, we reserve original value "6", and we will
     * exec "wl gmode 1" later nvram_set("wl_gmode", "6");
     * nvram_set("wl_afterburner", "auto"); nvram_set("wl_rateset", "all");
     * nvram_set("wl_frameburst", "on"); //nvram_set("wl_gmode_protection",
     * "off"); } else if(!strcmp(value, "speedbooster")){
     * nvram_set("wl_net_mode", value); nvram_set("wl_gmode", "6");
     * nvram_set("wl_afterburner_override", "1"); nvram_set("wl_rateset",
     * "all"); nvram_set("wl_frameburst", "on"); //
     * nvram_set("wl_gmode_protection", "off"); }
     * 
     * else if(!strcmp(value, "b-only")){ nvram_set("wl_net_mode", value);
     * nvram_set("wl_gmode", "0"); nvram_set("wl_afterburner", "off");
     * nvram_set("wl_rateset", "default"); nvram_set("wl_frameburst", "on");
     * 
     * } }else
     */
    {
#ifndef HAVE_MSSID
	if( nvram_nmatch( value, "%s_net_mode", prefix ) )
	{
	    return;
	}
#endif
	if( !strcmp( value, "disabled" ) )
	{
	    nvram_nset( value, "%s_net_mode", prefix );
	    nvram_nset( "-1", "%s_gmode", prefix );
#ifdef HAVE_MSSID
	    nvram_nset( "-1", "%s_nmode", prefix );
#endif
	    nvram_nset( "0", "%s_nreqd", prefix );
	}
	else if( !strcmp( value, "mixed" ) )
	{
	    nvram_nset( value, "wl_net_mode", prefix );
#ifdef HAVE_MSSID
	    nvram_nset( "1", "%s_gmode", prefix );
	    nvram_nset( "-1", "%s_nmode", prefix );
#else
	    nvram_nset( "6", "%s_gmode", prefix );
#endif
	    nvram_nset( "auto", "%s_afterburner", prefix );
	    nvram_nset( "default", "%s_rateset", prefix );
	    nvram_nset( "on", "%s_frameburst", prefix );
	    nvram_nset( "g", "%s_phytype", prefix );
	    nvram_nset( "0", "%s_nreqd", prefix );
	}
#ifdef HAVE_MSSID
	else if( !strcmp( value, "bg-mixed" ) )
	{
	    nvram_nset( value, "%s_net_mode", prefix );
	    nvram_nset( "1", "%s_gmode", prefix );
	    nvram_nset( "auto", "%s_afterburner", prefix );
	    nvram_nset( "default", "%s_rateset", prefix );
	    nvram_nset( "on", "%s_frameburst", prefix );
	    nvram_nset( "0", "%s_nmode", prefix );
	    nvram_nset( "g", "%s_phytype", prefix );
	    nvram_nset( "0", "%s_nreqd", prefix );
	}
#endif
	else if( !strcmp( value, "g-only" ) )
	{
	    nvram_nset( value, "wl_net_mode", prefix );
#ifdef HAVE_MSSID
	    nvram_nset( "0", "wl_nmode", prefix );
#endif
	    nvram_nset( "2", "wl_gmode", prefix );
	    nvram_nset( "g", "wl_phytype", prefix );
	    nvram_nset( "0", "wl_nreqd", prefix );

	}
	else if( !strcmp( value, "b-only" ) )
	{
	    nvram_nset( value, "%s_net_mode", prefix );
	    nvram_nset( "0", "%s_gmode", prefix );
#ifdef HAVE_MSSID
	    nvram_nset( "0", "%s_nmode", prefix );
#endif
	    nvram_nset( "off", "%s_afterburner", prefix );
	    nvram_nset( "default", "%s_rateset", prefix );
	    nvram_nset( "on", "%s_frameburst", prefix );
	    nvram_nset( "g", "%s_phytype", prefix );
	    nvram_nset( "0", "%s_nreqd", prefix );
	}
#ifdef HAVE_MSSID
	else if( !strcmp( value, "n-only" ) )
	{
	    nvram_nset( value, "%s_net_mode", prefix );
	    nvram_nset( "1", "%s_gmode", prefix );
	    nvram_nset( "2", "%s_nmode", prefix );
	    nvram_nset( "1", "%s_nreqd", prefix );
	    nvram_nset( "off", "%s_afterburner", prefix );	// From
								// 3.61.13.0
	    nvram_nset( "n", "%s_phytype", prefix );
	}
#endif
	else if( !strcmp( value, "a-only" ) )
	{
	    nvram_nset( value, "%s_net_mode", prefix );
	    nvram_nset( "a", "%s_phytype", prefix );
	    nvram_nset( "0", "%s_nreqd", prefix );
	}
    }
}

void validate_wl_net_mode( webs_t wp, char *value, struct variable *v )
{

    if( !valid_choice( wp, value, v ) )
	return;

    convert_wl_gmode( value, "wl" );

    nvram_set( v->name, value );
}

#ifdef HAVE_PPPOESERVER

void validate_chaps( webs_t wp, char *value, struct variable *v )
{

    int i, error = 0;
    char *buf, *cur;
    int count, sof;
    struct variable chaps_variables[] = {
      {argv:ARGV( "30" )},
      {argv:ARGV( "30" )},
	{NULL},
    }, *which;
    buf = nvram_safe_get( "pppoeserver_chapsnum" );
    if( buf == NULL || strlen( buf ) == 0 )
	return;
    count = atoi( buf );
    sof = ( count * 128 ) + 1;
    buf = ( char * )malloc( sof );
    cur = buf;
    buf[0] = 0;

    for( i = 0; i < count; i++ )
    {

	char chap_user[] = "userXXX";
	char chap_pass[] = "passXXX";
	char chap_ip[] = "ipXXX";
	char chap_enable[] = "enableXXX";
	char *user = "", new_user[200] = "", *pass = "", new_pass[200] =
	    "", *ip = "", *enable = "";

	snprintf( chap_user, sizeof( chap_user ), "user%d", i );
	snprintf( chap_pass, sizeof( chap_pass ), "pass%d", i );
	snprintf( chap_ip, sizeof( chap_ip ), "ip%d", i );
	snprintf( chap_enable, sizeof( chap_enable ), "enable%d", i );

	user = websGetVar( wp, chap_user, "" );
	pass = websGetVar( wp, chap_pass, "" );
	ip = websGetVar( wp, chap_ip, "0" );
	enable = websGetVar( wp, chap_enable, "off" );

	which = &chaps_variables[0];

	if( !*ip )
	    continue;
	if( !strcmp( ip, "0" ) || !strcmp( ip, "" ) )
	    continue;

	/*
	 * check name 
	 */
	if( strcmp( user, "" ) )
	{
	    if( !valid_name( wp, user, &which[0] ) )
	    {
		continue;
	    }
	    else
	    {
		httpd_filter_name( user, new_user, sizeof( new_user ), SET );
	    }
	}

	if( strcmp( pass, "" ) )
	{
	    if( !valid_name( wp, pass, &which[1] ) )
	    {
		continue;
	    }
	    else
	    {
		httpd_filter_name( pass, new_pass, sizeof( new_pass ), SET );
	    }
	}

	/*
	 * check ip address 
	 */
	if( !*ip )
	{
	    error = 1;
	    // websWrite(wp, "Invalid <b>%s</b> : must specify a
	    // ip<br>",which[4].longname);
	    continue;
	}

	if( sv_valid_ipaddr( ip ) )
	{
	    cur += snprintf( cur, buf + sof - cur, "%s%s:%s:%s:%s",
			     cur == buf ? "" : " ", new_user, new_pass, ip,
			     enable );
	}
	else
	{
	    error = 1;
	    continue;
	}

    }
    if( !error )
	nvram_set( v->name, buf );
    free( buf );
}
#endif
#ifdef HAVE_MILKFISH

void validate_aliases( webs_t wp, char *value, struct variable *v )
{

    int i, error = 0;
    char *buf, *cur;
    int count, sof;
    struct variable alias_variables[] = {
      {argv:ARGV( "30" )},
      {argv:ARGV( "30" )},
	{NULL},
    }, *which;
    buf = nvram_safe_get( "milkfish_ddaliasesnum" );
    if( buf == NULL || strlen( buf ) == 0 )
	return;
    count = atoi( buf );
    sof = ( count * 128 ) + 1;
    buf = ( char * )malloc( sof );
    cur = buf;
    buf[0] = 0;

    for( i = 0; i < count; i++ )
    {

	char alias_user[] = "userXXX";
	char alias_pass[] = "passXXX";
	char *user = "", new_user[200] = "", *pass = "", new_pass[200] = "";

	snprintf( alias_user, sizeof( alias_user ), "user%d", i );
	snprintf( alias_pass, sizeof( alias_pass ), "pass%d", i );

	user = websGetVar( wp, alias_user, "" );
	pass = websGetVar( wp, alias_pass, "" );

	which = &alias_variables[0];
	if( strcmp( user, "" ) )
	{
	    if( !valid_name( wp, user, &which[0] ) )
	    {
		continue;
	    }
	    else
	    {
		httpd_filter_name( user, new_user, sizeof( new_user ), SET );
	    }
	}

	if( strcmp( pass, "" ) )
	{
	    if( !valid_name( wp, pass, &which[1] ) )
	    {
		continue;
	    }
	    else
	    {
		httpd_filter_name( pass, new_pass, sizeof( new_pass ), SET );
	    }
	}
	cur += snprintf( cur, buf + sof - cur, "%s%s:%s",
			 cur == buf ? "" : " ", new_user, new_pass );

    }
    if( !error )
	nvram_set( v->name, buf );
    free( buf );

}

void validate_subscribers( webs_t wp, char *value, struct variable *v )
{

    int i, error = 0;
    char *buf, *cur;
    int count, sof;
    struct variable subscriber_variables[] = {
      {argv:ARGV( "30" )},
      {argv:ARGV( "30" )},
	{NULL},
    }, *which;
    buf = nvram_safe_get( "milkfish_ddsubscribersnum" );
    if( buf == NULL || strlen( buf ) == 0 )
	return;
    count = atoi( buf );
    sof = ( count * 128 ) + 1;
    buf = ( char * )malloc( sof );
    cur = buf;
    buf[0] = 0;

    for( i = 0; i < count; i++ )
    {

	char subscriber_user[] = "userXXX";
	char subscriber_pass[] = "passXXX";
	char *user = "", new_user[200] = "", *pass = "", new_pass[200] = "";

	snprintf( subscriber_user, sizeof( subscriber_user ), "user%d", i );
	snprintf( subscriber_pass, sizeof( subscriber_pass ), "pass%d", i );

	user = websGetVar( wp, subscriber_user, "" );
	pass = websGetVar( wp, subscriber_pass, "" );

	which = &subscriber_variables[0];
	if( strcmp( user, "" ) )
	{
	    if( !valid_name( wp, user, &which[0] ) )
	    {
		continue;
	    }
	    else
	    {
		httpd_filter_name( user, new_user, sizeof( new_user ), SET );
	    }
	}

	if( strcmp( pass, "" ) )
	{
	    if( !valid_name( wp, pass, &which[1] ) )
	    {
		continue;
	    }
	    else
	    {
		httpd_filter_name( pass, new_pass, sizeof( new_pass ), SET );
	    }
	}
	cur += snprintf( cur, buf + sof - cur, "%s%s:%s",
			 cur == buf ? "" : " ", new_user, new_pass );

    }
    if( !error )
	nvram_set( v->name, buf );
    free( buf );

}
#endif

#ifdef HAVE_RADLOCAL
void validate_iradius( webs_t wp, char *value, struct variable *v )
{
    char username[32] = "iradiusxxx_name";
    char active[32] = "iradiusxxx_active";
    char del[32] = "iradiusxxx_delete";

    char *sln = nvram_safe_get( "iradius_count" );

    if( sln == NULL || strlen( sln ) == 0 )
	return;
    int leasenum = atoi( sln );

    if( leasenum == 0 )
	return;
    char *leases;
    int i;

    leases = ( char * )malloc( ( 128 * leasenum ) + 1 );
    memset( leases, 0, ( 128 * leasenum ) + 1 );
    int leasen = 0;

    cprintf( "build mac list\n" );

    struct timeval now;

    gettimeofday( &now, NULL );

    for( i = 0; i < leasenum; i++ )
    {
	snprintf( del, 31, "iradius%d_delete", i );
	char *d = websGetVar( wp, del, "" );

	cprintf( "radius delete = %s\n", d );
	if( strcmp( d, "1" ) == 0 )
	    continue;

	snprintf( username, 31, "iradius%d_name", i );
	strcat( leases, websGetVar( wp, username, "00:00:00:00:00:00" ) );
	strcat( leases, " " );

	snprintf( active, 31, "iradius%d_active", i );
	strcat( leases, websGetVar( wp, active, "0" ) );
	strcat( leases, " " );

	snprintf( active, 31, "iradius%d_lease", i );
	char *time = websGetVar( wp, active, "-1" );
	int t = -1;

	if( strcmp( time, "over" ) )
	    t = atoi( time );
	if( t == -1 )
	{
	    strcat( leases, "-1" );
	}
	else
	{
	    char st[32];

	    sprintf( st, "%ld", ( now.tv_sec + t * 60 ) );
	    strcat( leases, st );
	}
	strcat( leases, " " );

	leasen++;
    }

    cprintf( "done %s\n", leases );
    nvram_store_collection( "iradius", leases );
    cprintf( "stored\n" );
    char nr[16];

    sprintf( nr, "%d", leasen );
    nvram_set( "iradius_count", nr );
    nvram_commit(  );
    free( leases );
}
#endif

#ifdef HAVE_CHILLILOCAL
void validate_userlist( webs_t wp, char *value, struct variable *v )
{
    char username[32] = "fon_userxxx_name";
    char password[32] = "fon_userxxx_password";
    char *sln = nvram_safe_get( "fon_usernames" );

    if( sln == NULL || strlen( sln ) == 0 )
	return;
    int leasenum = atoi( sln );

    if( leasenum == 0 )
	return;
    char *leases;
    int i;

    leases = ( char * )malloc( ( 128 * leasenum ) + 1 );
    memset( leases, 0, ( 128 * leasenum ) + 1 );

    for( i = 0; i < leasenum; i++ )
    {
	snprintf( username, 31, "fon_user%d_name", i );
	strcat( leases, websGetVar( wp, username, "" ) );
	strcat( leases, "=" );
	snprintf( password, 31, "fon_user%d_password", i );
	strcat( leases, websGetVar( wp, password, "" ) );
	strcat( leases, " " );
    }
    nvram_set( "fon_userlist", leases );
    nvram_commit(  );
    free( leases );
}

#endif

void filterstring( char *str, char character )
{
    if( str == NULL )
	return;
    int c;
    int i;
    int len = strlen( str );

    c = 0;
    for( i = 0; i < len; i++ )
    {
	if( str[i] != character )
	    str[c++] = str[i];
    }
    str[c++] = 0;
}

char *buildmac( char *in )
{
    char mac[20];
    char *outmac;

    outmac = malloc( 20 );
    strncpy( mac, in, 20 );
    filterstring( mac, ':' );
    filterstring( mac, '-' );
    filterstring( mac, ' ' );
    if( strlen( mac ) != 12 )
    {
	free( outmac );
	return NULL;		// error. invalid mac
    }
    int i;
    int c = 0;

    for( i = 0; i < 12; i += 2 )
    {
	outmac[c++] = mac[i];
	outmac[c++] = mac[i + 1];
	if( i < 10 )
	    outmac[c++] = ':';
    }
    outmac[c++] = 0;
    return outmac;
}

void validate_staticleases( webs_t wp, char *value, struct variable *v )
{
    char lease_hwaddr[32] = "leasexxx_hwaddr";
    char lease_hostname[32] = "leasexxx_hostname";
    char lease_ip[32] = "leasexxx_ip";
    char *sln = nvram_safe_get( "static_leasenum" );
    char *hwaddr;

    if( sln == NULL || strlen( sln ) == 0 )
	return;
    int leasenum = atoi( sln );

    if( leasenum == 0 )
	return;
    char *leases;
    int i;

    leases = ( char * )malloc( ( 54 * leasenum ) + 1 );
    memset( leases, 0, ( 54 * leasenum ) + 1 );

    for( i = 0; i < leasenum; i++ )
    {
	snprintf( lease_hwaddr, 31, "lease%d_hwaddr", i );
	hwaddr = websGetVar( wp, lease_hwaddr, NULL );
	if( hwaddr == NULL )
	    break;
	char *mac = buildmac( hwaddr );

	if( mac == NULL )
	{
	    free( leases );
	    websError( wp, 400, "%s is not a valid mac adress\n", hwaddr );
	    return;
	}
	strcat( leases, mac );
	free( mac );
	snprintf( lease_hostname, 31, "lease%d_hostname", i );
	char *hostname = websGetVar( wp, lease_hostname, NULL );

	snprintf( lease_ip, 31, "lease%d_ip", i );
	char *ip = websGetVar( wp, lease_ip, "" );

	if( hostname == NULL || strlen( hostname ) == 0 || ip == NULL
	    || strlen( ip ) == 0 )
	    break;
	strcat( leases, "=" );
	strcat( leases, hostname );
	strcat( leases, "=" );
	strcat( leases, ip );
	strcat( leases, " " );
    }
    nvram_set( "static_leases", leases );
    nvram_commit(  );
    free( leases );
}

void dhcp_check( webs_t wp, char *value, struct variable *v )
{
    return;			// The udhcpd can valid lease table when
				// re-load udhcpd.leases. by honor 2003-08-05
}

void validate_wds( webs_t wp, char *value, struct variable *v )
{
#ifdef HAVE_MADWIFI
    int h, i, devcount = 0;	// changed from 2 to 3
#elif HAVE_MSSID
    int h, i, devcount = 1;	// changed from 2 to 3
#else
    int h, i, devcount = 3;	// changed from 2 to 3
#endif
    struct variable wds_variables[] = {
      {argv:NULL},
      {argv:NULL},
      {argv:NULL},
      {argv:NULL},
      {argv:NULL},
    };

    char *val = NULL;
    char wds[32] = "";
    char wdsif_var[32] = "";
    char enabled_var[32];
    char hwaddr_var[32] = "";
    char ipaddr_var[32] = "";
    char netmask_var[32] = "";
    char desc_var[32] = "";
    char hwaddr[18] = "";
    char ipaddr[16] = "";
    char netmask[16] = "";
    char desc[48] = "";
    char wds_if[32] = { 0 };
    char wds_list[199] = "";
    char *interface = websGetVar( wp, "interface", NULL );

    if( interface == NULL )
	return;

    char wl0wds[32];

    sprintf( wl0wds, "%s_wds", interface );
    nvram_set( wl0wds, "" );
    snprintf( wds, 31, "%s_br1", interface );
    snprintf( enabled_var, 31, "%s_enable", wds );
    cprintf( "wds_validate\n" );
    /*
     * validate separate br1 bridge params 
     */
    if( nvram_match( enabled_var, "1" ) )
    {

	memset( ipaddr, 0, sizeof( ipaddr ) );
	memset( netmask, 0, sizeof( netmask ) );

	// disable until validated
	nvram_set( enabled_var, "0" );

	// subnet params validation
	for( i = 0; i < 4; i++ )
	{

	    snprintf( ipaddr_var, 31, "%s_%s%d", wds, "ipaddr", i );
	    val = websGetVar( wp, ipaddr_var, NULL );
	    if( val )
	    {
		strcat( ipaddr, val );
		if( i < 3 )
		    strcat( ipaddr, "." );
	    }
	    else
		break;

	    snprintf( netmask_var, 31, "%s_%s%d", wds, "netmask", i );
	    val = websGetVar( wp, netmask_var, NULL );
	    if( val )
	    {
		strcat( netmask, val );

		if( i < 3 )
		    strcat( netmask, "." );
	    }
	    else
		break;
	}

	if( !valid_ipaddr( wp, ipaddr, &wds_variables[1] ) ||
	    !valid_netmask( wp, netmask, &wds_variables[2] ) )
	    return;

	snprintf( ipaddr_var, 31, "%s_%s", wds, "ipaddr" );
	snprintf( netmask_var, 31, "%s_%s", wds, "netmask" );

	nvram_set( enabled_var, "1" );
	snprintf( ipaddr_var, 31, "%s_%s%d", wds, "ipaddr", i );
	nvram_set( ipaddr_var, ipaddr );
	snprintf( netmask_var, 31, "%s_%s%d", wds, "netmask", i );
	nvram_set( netmask_var, netmask );
    }
    else
	nvram_set( enabled_var, "0" );

    for( h = 1; h <= MAX_WDS_DEVS; h++ )
    {
	memset( hwaddr, 0, sizeof( hwaddr ) );
	memset( desc, 0, sizeof( desc ) );
	snprintf( wds, 31, "%s_wds%d", interface, h );
	snprintf( enabled_var, 31, "%s_enable", wds );

	for( i = 0; i < 6; i++ )
	{

	    snprintf( hwaddr_var, 31, "%s_%s%d", wds, "hwaddr", i );
	    val = websGetVar( wp, hwaddr_var, NULL );

	    if( val )
	    {
		strcat( hwaddr, val );
		if( i < 5 )
		    strcat( hwaddr, ":" );
	    }
	}

	if( !valid_hwaddr( wp, hwaddr, &wds_variables[0] ) )
	{
	    return;
	}

	snprintf( hwaddr_var, 31, "%s_%s", wds, "hwaddr" );
	nvram_set( hwaddr_var, hwaddr );

	snprintf( desc_var, 31, "%s_%s", wds, "desc" );
	val = websGetVar( wp, desc_var, NULL );
	if( val )
	{
	    strcat( desc, val );
	    snprintf( desc_var, 31, "%s_%s", wds, "desc" );
	    nvram_set( desc_var, desc );
	}

	/*
	 * <lonewolf> 
	 */
	snprintf( desc_var, 31, "%s_%s", wds, "ospf" );
	val = websGetVar( wp, desc_var, "" );
	if( val )
	{
	    snprintf( desc_var, 31, "%s_%s", wds, "ospf" );
	    nvram_set( desc_var, val );
	}
	/*
	 * </lonewolf> 
	 */

	if( strcmp( hwaddr, "00:00:00:00:00:00" )
	    && nvram_invmatch( enabled_var, "0" ) )
	{
	    snprintf( wds_list, 199, "%s %s", wds_list, hwaddr );
	}

	if( nvram_match( enabled_var, "1" ) )
	{

	    memset( ipaddr, 0, sizeof( ipaddr ) );
	    memset( netmask, 0, sizeof( netmask ) );

	    // disable until validated
	    nvram_set( enabled_var, "0" );

	    // subnet params validation
	    for( i = 0; i < 4; i++ )
	    {

		snprintf( ipaddr_var, 31, "%s_%s%d", wds, "ipaddr", i );
		val = websGetVar( wp, ipaddr_var, NULL );
		if( val )
		{
		    strcat( ipaddr, val );
		    if( i < 3 )
			strcat( ipaddr, "." );
		}
		else
		    break;

		snprintf( netmask_var, 31, "%s_%s%d", wds, "netmask", i );
		val = websGetVar( wp, netmask_var, NULL );
		if( val )
		{
		    strcat( netmask, val );

		    if( i < 3 )
			strcat( netmask, "." );
		}
		else
		    break;
	    }

	    if( !valid_ipaddr( wp, ipaddr, &wds_variables[1] ) ||
		!valid_netmask( wp, netmask, &wds_variables[2] ) )
	    {
		continue;
	    }

	    snprintf( ipaddr_var, 31, "%s_%s", wds, "ipaddr" );
	    snprintf( netmask_var, 31, "%s_%s", wds, "netmask" );

	    nvram_set( enabled_var, "1" );
	    nvram_set( ipaddr_var, ipaddr );
	    nvram_set( netmask_var, netmask );
	}

	/*
	 * keep the wds devices in sync w enabled entries 
	 */
	snprintf( wdsif_var, 31, "%s_if", wds );
	if( !nvram_match( enabled_var, "0" ) )
	{
#ifdef HAVE_MADWIFI
	    snprintf( wds_if, 31, "%s.wds%d", interface, ( devcount++ ) );
#else
	    // quick and dirty
	    if( !strcmp( interface, "wl0" ) )
		snprintf( wds_if, 31, "wds0.%d", ( devcount++ ) );
	    else if( !strcmp( interface, "wl1" ) )
		snprintf( wds_if, 31, "wds1.%d", ( devcount++ ) );
	    else
		snprintf( wds_if, 31, "wds%d.%d",
			  get_wl_instance( interface ), ( devcount++ ) );
#endif
	    nvram_set( wdsif_var, wds_if );
	}
	else
	    nvram_unset( wdsif_var );

    }

    nvram_set( wl0wds, wds_list );
}

void validate_filter_ip_grp( webs_t wp, char *value, struct variable *v )
{
    D( "validate_filter_ip_grp" );
    int i = 0;
    char buf[256] = "";
    char *ip0, *ip1, *ip2, *ip3, *ip4, *ip5, *ip_range0_0, *ip_range0_1,
	*ip_range1_0, *ip_range1_1;
    unsigned char ip[10] = { 0, 0, 0, 0, 0, 0, 0 };
    struct variable filter_ip_variables[] = {
      {argv:ARGV( "0", "255" )},
    }, *which;
    char _filter_ip[] = "filter_ip_grpXXX";

    // char _filter_rule[] = "filter_ruleXXX";
    // char _filter_tod[] = "filter_todXXX";

    which = &filter_ip_variables[0];

    ip0 = websGetVar( wp, "ip0", "0" );
    ip1 = websGetVar( wp, "ip1", "0" );
    ip2 = websGetVar( wp, "ip2", "0" );
    ip3 = websGetVar( wp, "ip3", "0" );
    ip4 = websGetVar( wp, "ip4", "0" );
    ip5 = websGetVar( wp, "ip5", "0" );
    ip_range0_0 = websGetVar( wp, "ip_range0_0", "0" );
    ip_range0_1 = websGetVar( wp, "ip_range0_1", "0" );
    ip_range1_0 = websGetVar( wp, "ip_range1_0", "0" );
    ip_range1_1 = websGetVar( wp, "ip_range1_1", "0" );

    if( !valid_range( wp, ip0, &which[0] ) ||
	!valid_range( wp, ip1, &which[0] ) ||
	!valid_range( wp, ip2, &which[0] ) ||
	!valid_range( wp, ip3, &which[0] ) ||
	!valid_range( wp, ip4, &which[0] ) ||
	!valid_range( wp, ip5, &which[0] ) ||
	!valid_range( wp, ip_range0_0, &which[0] ) ||
	!valid_range( wp, ip_range0_1, &which[0] ) ||
	!valid_range( wp, ip_range1_0, &which[0] ) ||
	!valid_range( wp, ip_range1_0, &which[0] ) )
    {
	D( "invalid range, return" );
	return;
    }

    if( atoi( ip0 ) )
	ip[i++] = atoi( ip0 );
    if( atoi( ip1 ) )
	ip[i++] = atoi( ip1 );
    if( atoi( ip2 ) )
	ip[i++] = atoi( ip2 );
    if( atoi( ip3 ) )
	ip[i++] = atoi( ip3 );
    if( atoi( ip4 ) )
	ip[i++] = atoi( ip4 );
    if( atoi( ip5 ) )
	ip[i++] = atoi( ip5 );

    if( atoi( ip_range0_0 ) > atoi( ip_range0_1 ) )
	SWAP( ip_range0_0, ip_range0_1 );

    if( atoi( ip_range1_0 ) > atoi( ip_range1_1 ) )
	SWAP( ip_range1_0, ip_range1_1 );

    sprintf( buf, "%d %d %d %d %d %d %s-%s %s-%s", ip[0], ip[1], ip[2], ip[3],
	     ip[4], ip[5], ip_range0_0, ip_range0_1, ip_range1_0,
	     ip_range1_1 );

    snprintf( _filter_ip, sizeof( _filter_ip ), "filter_ip_grp%s",
	      nvram_safe_get( "filter_id" ) );
    nvram_set( _filter_ip, buf );

    // snprintf(_filter_rule, sizeof(_filter_rule), "filter_rule%s",
    // nvram_safe_get("filter_id"));
    // snprintf(_filter_tod, sizeof(_filter_tod), "filter_tod%s",
    // nvram_safe_get("filter_id"));
    // if(nvram_match(_filter_rule, "")){
    // nvram_set(_filter_rule, "$STAT:1$NAME:$$");
    // nvram_set(_filter_tod, "0:0 23:59 0-6");
    // }
    D( "success return" );

}

/*
 * Example: tcp:100-200 udp:210-220 both:250-260 
 */

void validate_filter_port( webs_t wp, char *value, struct variable *v )
{
    int i;
    char buf[1000] = "", *cur = buf;
    struct variable filter_port_variables[] = {
      {argv:ARGV( "0",
	      "65535" )},
      {argv:ARGV( "0",
	      "65535" )},
    }, *which;
    D( "validate_filter_port" );
    which = &filter_port_variables[0];

    for( i = 0; i < FILTER_PORT_NUM; i++ )
    {
	char filter_port[] = "protoXXX";
	char filter_port_start[] = "startXXX";
	char filter_port_end[] = "endXXX";
	char *port, *start, *end;
	char *temp;

	snprintf( filter_port, sizeof( filter_port ), "proto%d", i );
	snprintf( filter_port_start, sizeof( filter_port_start ), "start%d",
		  i );
	snprintf( filter_port_end, sizeof( filter_port_end ), "end%d", i );
	port = websGetVar( wp, filter_port, NULL );
	start = websGetVar( wp, filter_port_start, NULL );
	end = websGetVar( wp, filter_port_end, NULL );

	if( !port || !start || !end )
	    continue;

	if( !*start && !*end )
	    continue;

	if( ( !strcmp( start, "0" ) || !strcmp( start, "" ) ) &&
	    ( !strcmp( end, "0" ) || !strcmp( end, "" ) ) )
	    continue;

	if( !*start || !*end )
	{
	    // websWrite(wp, "Invalid <b>%s</b>: must specify a LAN Port
	    // Range<br>", v->longname);
	    continue;
	}
	if( !valid_range( wp, start, &which[0] )
	    || !valid_range( wp, end, &which[1] ) )
	{
	    continue;
	}
	if( atoi( start ) > atoi( end ) )
	{
	    temp = start;
	    start = end;
	    end = temp;
	}
	cur += snprintf( cur, buf + sizeof( buf ) - cur, "%s%s:%d-%d",
			 cur == buf ? "" : " ", port, atoi( start ),
			 atoi( end ) );
    }

    nvram_set( v->name, buf );
    D( "success return" );
}

void validate_filter_dport_grp( webs_t wp, char *value, struct variable *v )
{
    int i;
    char buf[1000] = "", *cur = buf;
    struct variable filter_port_variables[] = {
      {argv:ARGV( "0",
	      "65535" )},
      {argv:ARGV( "0",
	      "65535" )},
    }, *which;
    char _filter_port[] = "filter_dport_grpXXX";

    // char _filter_rule[] = "filter_ruleXXX";
    // char _filter_tod[] = "filter_todXXX";
    D( "validate_filter-dport-grp" );
    which = &filter_port_variables[0];

    for( i = 0; i < FILTER_PORT_NUM; i++ )
    {
	char filter_port[] = "protoXXX";
	char filter_port_start[] = "startXXX";
	char filter_port_end[] = "endXXX";
	char *port, *start, *end;
	char *temp;

	snprintf( filter_port, sizeof( filter_port ), "proto%d", i );
	snprintf( filter_port_start, sizeof( filter_port_start ), "start%d",
		  i );
	snprintf( filter_port_end, sizeof( filter_port_end ), "end%d", i );
	port = websGetVar( wp, filter_port, NULL );
	start = websGetVar( wp, filter_port_start, NULL );
	end = websGetVar( wp, filter_port_end, NULL );

	if( !port || !start || !end )
	    continue;

	if( !*start && !*end )
	    continue;

	if( ( !strcmp( start, "0" ) || !strcmp( start, "" ) ) &&
	    ( !strcmp( end, "0" ) || !strcmp( end, "" ) ) )
	    continue;

	if( !*start || !*end )
	{
	    // websWrite(wp, "Invalid <b>%s</b>: must specify a LAN Port
	    // Range<br>", v->longname);
	    continue;
	}
	if( !valid_range( wp, start, &which[0] )
	    || !valid_range( wp, end, &which[1] ) )
	{
	    continue;
	}
	if( atoi( start ) > atoi( end ) )
	{
	    temp = start;
	    start = end;
	    end = temp;
	}
	cur += snprintf( cur, buf + sizeof( buf ) - cur, "%s%s:%d-%d",
			 cur == buf ? "" : " ", port, atoi( start ),
			 atoi( end ) );
    }

    snprintf( _filter_port, sizeof( _filter_port ), "filter_dport_grp%s",
	      nvram_safe_get( "filter_id" ) );
    nvram_set( _filter_port, buf );

    // snprintf(_filter_rule, sizeof(_filter_rule), "filter_rule%s",
    // nvram_safe_get("filter_id"));
    // snprintf(_filter_tod, sizeof(_filter_tod), "filter_tod%s",
    // nvram_safe_get("filter_id"));
    // if(nvram_match(_filter_rule, "")){
    // nvram_set(_filter_rule, "$STAT:1$NAME:$$");
    // nvram_set(_filter_tod, "0:0 23:59 0-6");
    // }
    D( "success return" );
}

/*
 * Example: 2 00:11:22:33:44:55 00:11:22:33:44:56 
 */

void validate_filter_mac_grp( webs_t wp, char *value, struct variable *v )
{

    int i;
    char buf[1000] = "", *cur = buf;
    char _filter_mac[] = "filter_mac_grpXXX";

    // char _filter_rule[] = "filter_ruleXXX";
    // har _filter_tod[] = "filter_todXXX";
    D( "validate_filter__mac_grp" );

    for( i = 0; i < FILTER_MAC_NUM; i++ )
    {
	char filter_mac[] = "macXXX";
	char *mac, mac1[20] = "";

	snprintf( filter_mac, sizeof( filter_mac ), "mac%d", i );

	mac = websGetVar( wp, filter_mac, NULL );
	if( !mac )
	    continue;

	if( strcmp( mac, "" ) && strcmp( mac, "00:00:00:00:00:00" )
	    && strcmp( mac, "000000000000" ) )
	{
	    if( strlen( mac ) == 12 )
	    {
		char hex[] = "XX";
		unsigned char h;

		while( *mac )
		{
		    strncpy( hex, mac, 2 );
		    h = ( unsigned char )strtoul( hex, NULL, 16 );
		    if( strlen( mac1 ) )
			sprintf( mac1 + strlen( mac1 ), ":" );
		    sprintf( mac1 + strlen( mac1 ), "%02X", h );
		    mac += 2;
		}
		mac1[17] = '\0';
	    }
	    else if( strlen( mac ) == 17 )
	    {
		strcpy( mac1, mac );
	    }
	    if( !valid_hwaddr( wp, mac1, v ) )
	    {
		continue;
	    }
	}
	else
	{
	    continue;
	}
	cur += snprintf( cur, buf + sizeof( buf ) - cur, "%s%s",
			 cur == buf ? "" : " ", mac1 );
    }

    snprintf( _filter_mac, sizeof( _filter_mac ), "filter_mac_grp%s",
	      nvram_safe_get( "filter_id" ) );
    nvram_set( _filter_mac, buf );

    // snprintf(_filter_rule, sizeof(_filter_rule), "filter_rule%s",
    // nvram_safe_get("filter_id"));
    // snprintf(_filter_tod, sizeof(_filter_tod), "filter_tod%s",
    // nvram_safe_get("filter_id"));
    // if(nvram_match(_filter_rule, "")){
    // nvram_set(_filter_rule, "$STAT:1$NAME:$$");
    // nvram_set(_filter_tod, "0:0 23:59 0-6");
    // }
    D( "success return" );
}

/*
 * Format: url0=www.kimo.com.tw, ...  keywd0=sex, ... 
 */
void validate_filter_web( webs_t wp, char *value, struct variable *v )
{
    int i;
    char buf[1000] = "", *cur = buf;
    char buf1[1000] = "", *cur1 = buf1;
    char filter_host[] = "filter_web_hostXXX";
    char filter_url[] = "filter_web_urlXXX";

    D( "validate_filter_web" );
    /*
     * Handle Website Blocking by URL Address 
     */
    for( i = 0; i < 9; i++ )
    {
	char filter_host[] = "hostXXX";
	char *host;

	snprintf( filter_host, sizeof( filter_host ), "host%d", i );
	host = websGetVar( wp, filter_host, "" );

	if( !strcmp( host, "" ) )
	    continue;
	int offset = 0;

	if( startswith( host, "http://" ) )
	    offset = 7;
	cur += snprintf( cur, buf + sizeof( buf ) - cur, "%s%s",
			 cur == buf ? "" : "<&nbsp;>", &host[offset] );
    }

    if( strcmp( buf, "" ) )
	strcat( buf, "<&nbsp;>" );

    snprintf( filter_host, sizeof( filter_host ), "filter_web_host%s",
	      nvram_safe_get( "filter_id" ) );
    nvram_set( filter_host, buf );

    /*
     * Handle Website Blocking by Keyword 
     */
    for( i = 0; i < 8; i++ )
    {
	char filter_url[] = "urlXXX";
	char *url;

	snprintf( filter_url, sizeof( filter_url ), "url%d", i );
	url = websGetVar( wp, filter_url, "" );

	if( !strcmp( url, "" ) )
	    continue;

	cur1 += snprintf( cur1, buf1 + sizeof( buf1 ) - cur1, "%s%s",
			  cur1 == buf1 ? "" : "<&nbsp;>", url );
    }
    if( strcmp( buf1, "" ) )
	strcat( buf1, "<&nbsp;>" );

    snprintf( filter_url, sizeof( filter_url ), "filter_web_url%s",
	      nvram_safe_get( "filter_id" ) );
    nvram_set( filter_url, buf1 );
    D( "everything okay" );
}

/*
 * Example: name:on:both:1000-2000>3000-4000 
 */

void validate_port_trigger( webs_t wp, char *value, struct variable *v )
{
    int i, error = 0;
    char *buf, *cur;
    int count, sof;
    struct variable trigger_variables[] = {
      {argv:ARGV( "12" )},
      {argv:ARGV( "0", "65535" )},
      {argv:ARGV( "0", "65535" )},
      {argv:ARGV( "0", "65535" )},
      {argv:ARGV( "0", "65535" )},
    }, *which;

    buf = nvram_safe_get( "trigger_entries" );
    if( buf == NULL || strlen( buf ) == 0 )
	return;
    count = atoi( buf );
    sof = ( count * 46 ) + 1;
    buf = ( char * )malloc( sof );
    cur = buf;
    memset( buf, 0, sof );

    for( i = 0; i < count; i++ )
    {

	char trigger_name[] = "nameXXX";
	char trigger_enable[] = "enableXXX";
	char trigger_i_from[] = "i_fromXXX";
	char trigger_i_to[] = "i_toXXX";
	char trigger_o_from[] = "o_fromXXX";
	char trigger_o_to[] = "o_toXXX";
	char trigger_proto[] = "proXXX";
	char *name = "", *enable, new_name[200] = "", *i_from = "", *i_to =
	    "", *o_from = "", *o_to = "", *proto = "both";

	snprintf( trigger_name, sizeof( trigger_name ), "name%d", i );
	snprintf( trigger_enable, sizeof( trigger_enable ), "enable%d", i );
	snprintf( trigger_i_from, sizeof( trigger_i_from ), "i_from%d", i );
	snprintf( trigger_i_to, sizeof( trigger_i_to ), "i_to%d", i );
	snprintf( trigger_o_from, sizeof( trigger_o_from ), "o_from%d", i );
	snprintf( trigger_o_to, sizeof( trigger_o_to ), "o_to%d", i );
	snprintf( trigger_proto, sizeof( trigger_proto ), "pro%d", i );

	name = websGetVar( wp, trigger_name, "" );
	enable = websGetVar( wp, trigger_enable, "off" );
	i_from = websGetVar( wp, trigger_i_from, NULL );
	i_to = websGetVar( wp, trigger_i_to, NULL );
	o_from = websGetVar( wp, trigger_o_from, NULL );
	o_to = websGetVar( wp, trigger_o_to, NULL );
	proto = websGetVar( wp, trigger_proto, "both" );
	which = &trigger_variables[0];

	if( !i_from || !i_to || !o_from || !o_to )
	    continue;

	if( ( !strcmp( i_from, "0" ) || !strcmp( i_from, "" ) ) &&
	    ( !strcmp( i_to, "0" ) || !strcmp( i_to, "" ) ) &&
	    ( !strcmp( o_from, "0" ) || !strcmp( o_from, "" ) ) &&
	    ( !strcmp( o_to, "0" ) || !strcmp( o_to, "" ) ) )
	    continue;

	if( !strcmp( i_from, "0" ) || !strcmp( i_from, "" ) )
	    i_from = i_to;
	if( !strcmp( i_to, "0" ) || !strcmp( i_to, "" ) )
	    i_to = i_from;
	if( !strcmp( o_from, "0" ) || !strcmp( o_from, "" ) )
	    o_from = o_to;
	if( !strcmp( o_to, "0" ) || !strcmp( o_to, "" ) )
	    o_to = o_from;

	if( atoi( i_from ) > atoi( i_to ) )
	    SWAP( i_from, i_to );

	if( atoi( o_from ) > atoi( o_to ) )
	    SWAP( o_from, o_to );

	if( strcmp( name, "" ) )
	{
	    if( !valid_name( wp, name, &which[0] ) )
	    {
		error = 1;
		continue;
	    }
	    else
	    {
		httpd_filter_name( name, new_name, sizeof( new_name ), SET );
	    }
	}

	if( !valid_range( wp, i_from, &which[1] )
	    || !valid_range( wp, i_to, &which[2] )
	    || !valid_range( wp, o_from, &which[3] )
	    || !valid_range( wp, o_to, &which[4] ) )
	{
	    error = 1;
	    continue;
	}

	cur += snprintf( cur, buf + sof - cur, "%s%s:%s:%s:%s-%s>%s-%s",
			 cur == buf ? "" : " ", new_name, enable, proto,
			 i_from, i_to, o_from, o_to );

    }

    if( !error )
	nvram_set( v->name, buf );
    free( buf );
}

void validate_blocked_service( webs_t wp, char *value, struct variable *v )
{
    int i;
    char buf[1000] = "", *cur = buf;
    char port_grp[] = "filter_port_grpXXX";

    D( "validate_blocked_service" );
    for( i = 0; i < BLOCKED_SERVICE_NUM; i++ )
    {
	char blocked_service[] = "blocked_serviceXXX";
	char *service;

	snprintf( blocked_service, sizeof( blocked_service ),
		  "blocked_service%d", i );
	service = websGetVar( wp, blocked_service, NULL );
	if( !service || !strcmp( service, "None" ) )
	    continue;

	cur +=
	    snprintf( cur, buf + sizeof( buf ) - cur, "%s%s", service,
		      "<&nbsp;>" );
	// cur == buf ? "" : "<&nbsp;>", service);
    }

    snprintf( port_grp, sizeof( port_grp ), "filter_port_grp%s",
	      nvram_safe_get( "filter_id" ) );
    nvram_set( port_grp, buf );
    D( "right" );
}

/*
 * validates the p2p catchall filter 
 */
void validate_catchall( webs_t wp, char *value, struct variable *v )
{
    char *p2p;
    char port_grp[] = "filter_p2p_grpXXX";

    p2p = websGetVar( wp, "filter_p2p", NULL );
    if( p2p )
    {
	snprintf( port_grp, sizeof( port_grp ), "filter_p2p_grp%s",
		  nvram_safe_get( "filter_id" ) );
	nvram_set( port_grp, p2p );
    }

    return;
}

void save_olsrd( webs_t wp );
void addDeletion( char *word );

void validate_static_route( webs_t wp, char *value, struct variable *v )
{
#ifdef HAVE_OLSRD
    save_olsrd( wp );
#endif

    int i, tmp = 1;
    char word[256], *next;
    char buf[1000] = "", *cur = buf;
    char buf_name[1000] = "", *cur_name = buf_name;
    char old[STATIC_ROUTE_PAGE][60];
    char old_name[STATIC_ROUTE_PAGE][30];
    char backuproute[256];
    struct variable static_route_variables[] = {
      {argv:NULL},
      {argv:NULL},
      {argv:NULL},
      {argv:ARGV( "lan", "wan" )},
    };

    char *name, ipaddr[20], netmask[20], gateway[20], *metric, *ifname, *page;
    char new_name[80];
    char temp[30], *val = NULL;

    name = websGetVar( wp, "route_name", "" );	// default empty if no find
						// route_name
    metric = websGetVar( wp, "route_metric", "0" );
    /*
     * validate ip address 
     */
    strcpy( ipaddr, "" );
    for( i = 0; i < 4; i++ )
    {
	snprintf( temp, sizeof( temp ), "%s_%d", "route_ipaddr", i );
	val = websGetVar( wp, temp, NULL );
	if( val )
	{
	    strcat( ipaddr, val );
	    if( i < 3 )
		strcat( ipaddr, "." );
	}
	else
	{
	    // free (ipaddr);
	    return;
	}
    }

    /*
     * validate netmask 
     */
    strcpy( netmask, "" );
    for( i = 0; i < 4; i++ )
    {
	snprintf( temp, sizeof( temp ), "%s_%d", "route_netmask", i );
	val = websGetVar( wp, temp, NULL );
	if( val )
	{
	    strcat( netmask, val );
	    if( i < 3 )
		strcat( netmask, "." );
	}
	else
	{
	    // free (netmask);
	    // free (ipaddr);
	    return;
	}
    }

    /*
     * validate gateway 
     */
    strcpy( gateway, "" );
    for( i = 0; i < 4; i++ )
    {
	snprintf( temp, sizeof( temp ), "%s_%d", "route_gateway", i );
	val = websGetVar( wp, temp, NULL );
	if( val )
	{
	    strcat( gateway, val );
	    if( i < 3 )
		strcat( gateway, "." );
	}
	else
	{
	    // free (gateway);
	    // free (netmask);
	    // free (ipaddr);
	    return;
	}
    }

    page = websGetVar( wp, "route_page", NULL );
    ifname = websGetVar( wp, "route_ifname", NULL );

    if( !page || !ipaddr || !netmask || !gateway || !metric || !ifname )
	return;

    // Allow Defaultroute here

    if( !strcmp( ipaddr, "0.0.0.0" ) && !strcmp( netmask, "0.0.0.0" )
	&& strcmp( gateway, "0.0.0.0" ) )
    {
	tmp = 1;
	goto write_nvram;
    }
    if( ( !strcmp( ipaddr, "0.0.0.0" ) || !strcmp( ipaddr, "" ) ) &&
	( !strcmp( netmask, "0.0.0.0" ) || !strcmp( netmask, "" ) ) &&
	( !strcmp( gateway, "0.0.0.0" ) || !strcmp( gateway, "" ) ) )
    {
	tmp = 0;
	goto write_nvram;
    }

    // if (!valid_choice (wp, ifname, &static_route_variables[3]))
    // {
    // free (gateway);
    // free (netmask);
    // free (ipaddr);

    // return;
    // }

    if( !*ipaddr )
    {
	websDebugWrite( wp,
			"Invalid <b>%s</b>: must specify an IP Address<br>",
			v->longname );
	// free (gateway);
	// free (netmask);
	// free (ipaddr);

	return;
    }
    if( !*netmask )
    {
	websDebugWrite( wp,
			"Invalid <b>%s</b>: must specify a Subnet Mask<br>",
			v->longname );
	// free (gateway);
	// free (netmask);
	// free (ipaddr);

	return;
    }
    if( !valid_ipaddr( wp, ipaddr, &static_route_variables[0] ) ||
	!valid_netmask( wp, netmask, &static_route_variables[1] ) ||
	!valid_ipaddr( wp, gateway, &static_route_variables[2] ) )
    {
	// free (gateway);
	// free (netmask);
	// free (ipaddr);

	return;
    }

    /*
     * save old value in nvram 
     */

  write_nvram:
    if( !strcmp( ifname, "lan" ) )
    {
	ifname = nvram_safe_get( "lan_ifname" );
	static_route_variables[2].argv = NULL;
    }
    if( !strcmp( ifname, "wan" ) )
    {
	ifname = nvram_safe_get( "wan_ifname" );
	static_route_variables[2].argv = NULL;
    }
    else
    {
	static_route_variables[2].argv = NULL;
    }

    for( i = 0; i < STATIC_ROUTE_PAGE; i++ )
    {
	strcpy( old[i], "" );
	strcpy( old_name[i], "" );
    }
    i = 0;
    foreach( word, nvram_safe_get( "static_route" ), next )
    {
	strcpy( old[i], word );
	i++;
    }
    i = 0;
    foreach( word, nvram_safe_get( "static_route_name" ), next )
    {
	strcpy( old_name[i], word );
	i++;
    }

    strcpy( backuproute, old[atoi( page )] );
    if( !tmp )
    {
	char met[16];
	char ifn[16];

	sscanf( old[atoi( page )], "%s:%s:%s:%s:%s", ipaddr, netmask, gateway,
		met, ifn );
	// fprintf (stderr, "deleting %s %s %s %s %s\n", ipaddr, netmask,
	// gateway,
	// met, ifn);
	route_del( ifn, atoi( met ) + 1, ipaddr, gateway, netmask );

	snprintf( old[atoi( page )], sizeof( old[0] ), "%s", "" );
	snprintf( old_name[atoi( page )], sizeof( old_name[0] ), "%s", "" );
    }
    else
    {
	snprintf( old[atoi( page )], sizeof( old[0] ), "%s:%s:%s:%s:%s",
		  ipaddr, netmask, gateway, metric, ifname );
	httpd_filter_name( name, new_name, sizeof( new_name ), SET );
	snprintf( old_name[atoi( page )], sizeof( old_name[0] ), "$NAME:%s$$",
		  new_name );
    }
    if( strcmp( backuproute, old[atoi( page )] ) )
    {
	if( strlen( backuproute ) > 0 )
	{
	    addAction( "static_route_del" );
	    addDeletion( backuproute );
	}
    }

    for( i = 0; i < STATIC_ROUTE_PAGE; i++ )
    {
	if( strcmp( old[i], "" ) )
	    cur += snprintf( cur, buf + sizeof( buf ) - cur, "%s%s",
			     cur == buf ? "" : " ", old[i] );
	if( strcmp( old_name[i], "" ) )
	    cur_name +=
		snprintf( cur_name, buf_name + sizeof( buf_name ) - cur_name,
			  "%s%s", cur_name == buf_name ? "" : " ",
			  old_name[i] );
    }

    nvram_set( v->name, buf );
    nvram_set( "static_route_name", buf_name );

    // if (ipaddr)
    // free (ipaddr);
    // if (netmask)
    // free (netmask);
    // if (gateway)
    // free (gateway);
}
