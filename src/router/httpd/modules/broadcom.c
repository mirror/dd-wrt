
/*
 * Broadcom Home Gateway Reference Design
 * Web Page Configuration Support Routines
 *
 * Copyright 2001-2003, Broadcom Corporation
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 * $Id: broadcom.c,v 1.9 2005/11/30 11:53:42 seg Exp $
 */

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
#include <dlfcn.h>

int debug_value = 0;

// static char * rfctime(const time_t *timep);
// static char * reltime(unsigned int seconds);

// #if defined(linux)

#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/klog.h>
#include <sys/wait.h>
#define sys_stats(url) eval("stats", (url))

// tofu

/*
 * Deal with side effects before committing 
 */
int sys_commit( void )
{
    if( nvram_match( "dhcpnvram", "1" ) )
    {
	killall( "dnsmasq", SIGUSR2 );	// update lease -- tofu
	sleep( 1 );
    }

    // if (nvram_match("wan_proto", "pppoe") || nvram_match("wan_proto",
    // "pptp") )
    // nvram_set("wan_ifname", "ppp0");
    // else
    // nvram_set("wan_ifname", nvram_get("pppoe_ifname"));
    return nvram_commit(  );
}

/*
 * Variables are set in order (put dependent variables later). Set
 * nullok to TRUE to ignore zero-length values of the variable itself.
 * For more complicated validation that cannot be done in one pass or
 * depends on additional form components or can throw an error in a
 * unique painful way, write your own validation routine and assign it
 * to a hidden variable (e.g. filter_ip).
 */
/*
 * DD-WRT enhancement by seg This functions parses all
 * /etc/config/xxxxx.nvramconfig files and creates the web var tab. so these
 * vars arent defined anymore staticly 
 */

#include <stdlib.h>
#include <malloc.h>
#include <dirent.h>
#include <stdlib.h>

char *toUP( char *a )
{
    int i;
    int slen = strlen( a );

    for( i = 0; i < slen; i++ )
    {
	if( a[i] > 'a' - 1 && a[i] < 'z' + 1 )
	    a[i] -= 'a' + 'A';
    }
    return a;
}

int stricmp( char *a, char *b )
{
    if( strlen( a ) != strlen( b ) )
	return -1;
    return strcmp( toUP( a ), toUP( b ) );
}

void StringStart( FILE * in )
{
    while( getc( in ) != '"' )
    {
	if( feof( in ) )
	    return;
    }
}

char *getFileString( FILE * in )
{
    char *buf;
    int i, b;

    buf = malloc( 1024 );
    StringStart( in );
    for( i = 0; i < 1024; i++ )
    {
	b = getc( in );
	if( b == EOF )
	    return NULL;
	if( b == '"' )
	{
	    buf[i] = 0;
	    buf = realloc( buf, strlen( buf ) + 1 );
	    return buf;
	}
	buf[i] = b;
    }
    return buf;
}

void skipFileString( FILE * in )
{
    int i, b;

    StringStart( in );
    for( i = 0; i < 1024; i++ )
    {
	b = getc( in );
	if( b == EOF )
	    return;
	if( b == '"' )
	{
	    return;
	}
    }
    return;
}

static char *directories[] = {
    "/etc/config",
    "/jffs/etc/config",
    "/mmc/etc/config"
};

struct variable **variables;
void Initnvramtab(  )
{
    struct dirent *entry;
    DIR *directory;
    FILE *in;
    int varcount = 0, len, i;
    char *tmpstr;
    struct variable *tmp;

    variables = NULL;
    char buf[1024];

    // format = VARNAME VARDESC VARVALID VARVALIDARGS FLAGS FLAGS
    // open config directory directory =
    int idx;

    for( idx = 0; idx < 3; idx++ )
    {
	directory = opendir( directories[idx] );
	if( directory == NULL )
	    continue;
	// list all files in this directory
	while( ( entry = readdir( directory ) ) != NULL )
	{
	    if( endswith( entry->d_name, ".nvramconfig" ) )
	    {
		sprintf( buf, "%s/%s", directories[idx], entry->d_name );
		in = fopen( buf, "rb" );
		if( in == NULL )
		{
		    return;
		}
		while( 1 )
		{
		    tmp =
			( struct variable * )
			malloc( sizeof( struct variable ) );
		    memset( tmp, 0, sizeof( struct variable ) );
		    tmp->name = getFileString( in );
		    if( tmp->name == NULL )
			break;
		    skipFileString( in );	// long string
		    tmpstr = getFileString( in );
		    tmp->argv = NULL;
		    if( !stricmp( tmpstr, "RANGE" ) )
		    {
			tmp->validatename = "validate_range";
			tmp->argv =
			    ( char ** )malloc( sizeof( char ** ) * 3 );
			tmp->argv[0] = getFileString( in );
			tmp->argv[1] = getFileString( in );
			tmp->argv[2] = NULL;
		    }
		    if( !stricmp( tmpstr, "CHOICE" ) )
		    {
			tmp->validatename = "validate_choice";
			free( tmpstr );
			tmpstr = getFileString( in );
			len = atoi( tmpstr );
			tmp->argv =
			    ( char ** )malloc( sizeof( char ** ) *
					       ( len + 1 ) );
			for( i = 0; i < len; i++ )
			{
			    tmp->argv[i] = getFileString( in );
			}
			tmp->argv[i] = NULL;
		    }
#ifdef HAVE_SPUTNIK_APD
		    if( !stricmp( tmpstr, "MJIDTYPE" ) )
		    {
			tmp->validatename = "validate_choice";
			free( tmpstr );
			tmpstr = getFileString( in );
			len = atoi( tmpstr );
			tmp->argv =
			    ( char ** )malloc( sizeof( char ** ) *
					       ( len + 1 ) );
			for( i = 0; i < len; i++ )
			{
			    tmp->argv[i] = getFileString( in );
			}
			tmp->argv[i] = NULL;
			nvram_set( "sputnik_rereg", "1" );
		    }
#endif
		    if( !stricmp( tmpstr, "NOACK" ) )
		    {
			tmp->validatename = "validate_noack";
			len = 2;
			tmp->argv =
			    ( char ** )malloc( sizeof( char ** ) *
					       ( len + 1 ) );
			for( i = 0; i < len; i++ )
			{
			    tmp->argv[i] = getFileString( in );
			}
			tmp->argv[i] = NULL;
		    }
		    if( !stricmp( tmpstr, "NAME" ) )
		    {
			tmp->validatename = "validate_name";
			tmp->argv =
			    ( char ** )malloc( sizeof( char ** ) * 2 );
			tmp->argv[0] = getFileString( in );
			tmp->argv[1] = NULL;
		    }
		    if( !stricmp( tmpstr, "NULL" ) )
		    {
		    }
		    if( !stricmp( tmpstr, "WMEPARAM" ) )
		    {
			tmp->validatename = "validate_wl_wme_params";
		    }
		    if( !stricmp( tmpstr, "WMETXPARAM" ) )
		    {
			tmp->validatename = "validate_wl_wme_tx_params";
		    }
		    if( !stricmp( tmpstr, "PASSWORD" ) )
		    {
			tmp->validatename = "validate_password";
			tmp->argv =
			    ( char ** )malloc( sizeof( char ** ) * 2 );
			tmp->argv[0] = getFileString( in );
			tmp->argv[1] = NULL;
		    }
		    if( !stricmp( tmpstr, "PASSWORD2" ) )
		    {
			tmp->validatename = "validate_password2";
			tmp->argv =
			    ( char ** )malloc( sizeof( char ** ) * 2 );
			tmp->argv[0] = getFileString( in );
			tmp->argv[1] = NULL;
		    }
		    if( !stricmp( tmpstr, "LANIPADDR" ) )
		    {
			tmp->validatename = "validate_lan_ipaddr";
			tmp->argv =
			    ( char ** )malloc( sizeof( char ** ) * 2 );
			tmp->argv[0] = getFileString( in );
			tmp->argv[1] = NULL;
		    }
		    if( !stricmp( tmpstr, "WANIPADDR" ) )
		    {
			tmp->validatename = "validate_wan_ipaddr";
		    }
		    if( !stricmp( tmpstr, "MERGEREMOTEIP" ) )
		    {
			tmp->validatename = "validate_remote_ip";
		    }
		    if( !stricmp( tmpstr, "MERGEIPADDRS" ) )
		    {
			tmp->validatename = "validate_merge_ipaddrs";
		    }
		    if( !stricmp( tmpstr, "DNS" ) )
		    {
			tmp->validatename = "validate_dns";
		    }
		    if( !stricmp( tmpstr, "SAVEWDS" ) )
		    {
			tmp->validate2name = "save_wds";
		    }
		    if( !stricmp( tmpstr, "DHCP" ) )
		    {
			tmp->validatename = "dhcp_check";
		    }
		    if( !stricmp( tmpstr, "WPAPSK" ) )
		    {
			tmp->validatename = "validate_wpa_psk";
			tmp->argv =
			    ( char ** )malloc( sizeof( char ** ) * 2 );
			tmp->argv[0] = getFileString( in );
			tmp->argv[1] = NULL;
		    }
		    if( !stricmp( tmpstr, "STATICS" ) )
		    {
			tmp->validatename = "validate_statics";
		    }
#ifdef HAVE_PORTSETUP
		    if( !stricmp( tmpstr, "PORTSETUP" ) )
		    {
			tmp->validatename = "validate_portsetup";
		    }
#endif
		    if( !stricmp( tmpstr, "REBOOT" ) )
		    {
			tmp->validatename = "validate_reboot";
		    }
		    if( !stricmp( tmpstr, "IPADDR" ) )
		    {
			tmp->validatename = "validate_ipaddr";
		    }
		    if( !stricmp( tmpstr, "STATICLEASES" ) )
		    {
			tmp->validatename = "validate_staticleases";
		    }
#ifdef HAVE_CHILLILOCAL
		    if( !stricmp( tmpstr, "USERLIST" ) )
		    {
			tmp->validatename = "validate_userlist";
		    }
#endif
#ifdef HAVE_RADLOCAL
		    if( !stricmp( tmpstr, "IRADIUSUSERLIST" ) )
		    {
			tmp->validatename = "validate_iradius";
		    }
#endif
		    if( !stricmp( tmpstr, "IPADDRS" ) )
		    {
			tmp->validatename = "validate_ipaddrs";
		    }
		    if( !stricmp( tmpstr, "NETMASK" ) )
		    {
			tmp->validatename = "validate_netmask";
		    }
		    if( !stricmp( tmpstr, "MERGENETMASK" ) )
		    {
			tmp->validatename = "validate_merge_netmask";
		    }
		    if( !stricmp( tmpstr, "WDS" ) )
		    {
			tmp->validatename = "validate_wds";
		    }
		    if( !stricmp( tmpstr, "STATICROUTE" ) )
		    {
			tmp->validatename = "validate_static_route";
		    }
		    if( !stricmp( tmpstr, "MERGEMAC" ) )
		    {
			tmp->validatename = "validate_merge_mac";
		    }
		    if( !stricmp( tmpstr, "FILTERPOLICY" ) )
		    {
			tmp->validatename = "validate_filter_policy";
		    }
		    if( !stricmp( tmpstr, "FILTERIPGRP" ) )
		    {
			tmp->validatename = "validate_filter_ip_grp";
		    }
		    if( !stricmp( tmpstr, "FILTERPORT" ) )
		    {
			tmp->validatename = "validate_filter_port";
		    }
		    if( !stricmp( tmpstr, "FILTERDPORTGRP" ) )
		    {
			tmp->validatename = "validate_filter_dport_grp";
		    }
		    if( !stricmp( tmpstr, "BLOCKEDSERVICE" ) )
		    {
			tmp->validatename = "validate_blocked_service";
		    }
		    if( !stricmp( tmpstr, "FILTERP2P" ) )
		    {
			tmp->validatename = "validate_catchall";
		    }
		    if( !stricmp( tmpstr, "FILTERMACGRP" ) )
		    {
			tmp->validatename = "validate_filter_mac_grp";
		    }
		    if( !stricmp( tmpstr, "FILTERWEB" ) )
		    {
			tmp->validatename = "validate_filter_web";
		    }
		    if( !stricmp( tmpstr, "WLHWADDRS" ) )
		    {
			tmp->validatename = "validate_wl_hwaddrs";
		    }
		    if( !stricmp( tmpstr, "FORWARDPROTO" ) )
		    {
			tmp->validatename = "validate_forward_proto";
		    }
		    if( !stricmp( tmpstr, "FORWARDSPEC" ) )
		    {
			tmp->validatename = "validate_forward_spec";
		    }
		    // changed by steve
		    /*
		     * if (!stricmp (tmpstr, "FORWARDUPNP")) { tmp->validate
		     * = validate_forward_upnp; } 
		     */
		    // end changed by steve
		    if( !stricmp( tmpstr, "PORTTRIGGER" ) )
		    {
			tmp->validatename = "validate_port_trigger";
		    }
		    if( !stricmp( tmpstr, "HWADDR" ) )
		    {
			tmp->validatename = "validate_hwaddr";
		    }
		    if( !stricmp( tmpstr, "HWADDRS" ) )
		    {
			tmp->validatename = "validate_hwaddrs";
		    }
		    if( !stricmp( tmpstr, "WLWEPKEY" ) )
		    {
			tmp->validatename = "validate_wl_wep_key";
		    }

		    if( !stricmp( tmpstr, "WLAUTH" ) )
		    {
			tmp->validatename = "validate_wl_auth";
			tmp->argv =
			    ( char ** )malloc( sizeof( char ** ) * 3 );
			tmp->argv[0] = getFileString( in );
			tmp->argv[1] = getFileString( in );
			tmp->argv[2] = NULL;
		    }
		    if( !stricmp( tmpstr, "WLWEP" ) )
		    {
			tmp->validatename = "validate_wl_wep";
			free( tmpstr );
			tmpstr = getFileString( in );
			len = atoi( tmpstr );
			tmp->argv =
			    ( char ** )malloc( sizeof( char ** ) *
					       ( len + 1 ) );
			for( i = 0; i < len; i++ )
			{
			    tmp->argv[i] = getFileString( in );
			}
			tmp->argv[i] = NULL;
		    }

		    if( !stricmp( tmpstr, "DYNAMICROUTE" ) )
		    {
			tmp->validatename = "validate_dynamic_route";
			free( tmpstr );
			tmpstr = getFileString( in );
			len = atoi( tmpstr );
			tmp->argv =
			    ( char ** )malloc( sizeof( char ** ) *
					       ( len + 1 ) );
			for( i = 0; i < len; i++ )
			{
			    tmp->argv[i] = getFileString( in );
			}
			tmp->argv[i] = NULL;
		    }
		    if( !stricmp( tmpstr, "WLGMODE" ) )
		    {
			tmp->validatename = "validate_wl_gmode";
			free( tmpstr );
			tmpstr = getFileString( in );
			len = atoi( tmpstr );
			tmp->argv =
			    ( char ** )malloc( sizeof( char ** ) *
					       ( len + 1 ) );
			for( i = 0; i < len; i++ )
			{
			    tmp->argv[i] = getFileString( in );
			}
			tmp->argv[i] = NULL;
		    }
		    if( !stricmp( tmpstr, "WLNETMODE" ) )
		    {
			tmp->validatename = "validate_wl_net_mode";
			free( tmpstr );
			tmpstr = getFileString( in );
			len = atoi( tmpstr );
			tmp->argv =
			    ( char ** )malloc( sizeof( char ** ) *
					       ( len + 1 ) );
			for( i = 0; i < len; i++ )
			{
			    tmp->argv[i] = getFileString( in );
			}
			tmp->argv[i] = NULL;
		    }
		    if( !stricmp( tmpstr, "AUTHMODE" ) )
		    {
			tmp->validatename = "validate_auth_mode";
			free( tmpstr );
			tmpstr = getFileString( in );
			len = atoi( tmpstr );
			tmp->argv =
			    ( char ** )malloc( sizeof( char ** ) *
					       ( len + 1 ) );
			for( i = 0; i < len; i++ )
			{
			    tmp->argv[i] = getFileString( in );
			}
			tmp->argv[i] = NULL;
		    }
#ifndef HAVE_MSSID
		    if( !stricmp( tmpstr, "SECURITYMODE" ) )
		    {
			tmp->validatename = "validate_security_mode";
			free( tmpstr );
			tmpstr = getFileString( in );
			len = atoi( tmpstr );
			tmp->argv =
			    ( char ** )malloc( sizeof( char ** ) *
					       ( len + 1 ) );
			for( i = 0; i < len; i++ )
			{
			    tmp->argv[i] = getFileString( in );
			}
			tmp->argv[i] = NULL;
		    }
#endif
#ifdef HAVE_PPPOESERVER
		    if( !stricmp( tmpstr, "CHAPTABLE" ) )
		    {
			tmp->validatename = "validate_chaps";
		    }
#endif

#ifdef HAVE_MILKFISH
		    if( !stricmp( tmpstr, "MFSUBSCRIBERS" ) )
		    {
			tmp->validatename = "validate_subscribers";
		    }
		    if( !stricmp( tmpstr, "MFALIASES" ) )
		    {
			tmp->validatename = "validate_aliases";
		    }
#endif

		    free( tmpstr );
		    tmpstr = getFileString( in );
		    if( !stricmp( tmpstr, "TRUE" ) )
		    {
			tmp->nullok = TRUE;
		    }
		    else
		    {
			tmp->nullok = FALSE;
		    }
		    free( tmpstr );
		    skipFileString( in );	// todo: remove it
		    // tmpstr = getFileString (in);
		    // tmp->ezc_flags = atoi (tmpstr);
		    // free (tmpstr);
		    variables =
			( struct variable ** )realloc( variables,
						       sizeof( struct variable
							       ** ) *
						       ( varcount + 2 ) );
		    variables[varcount++] = tmp;
		    variables[varcount] = NULL;
		}
		fclose( in );
	    }
	}
	closedir( directory );
    }
}

#ifdef HAVE_MACBIND
#include "../../../opt/mac.h"
#endif
// Added by Daniel(2004-07-29) for EZC
int variables_arraysize( void )
{
    int varcount = 0;

    if( variables == NULL )
	return 0;
    while( variables[varcount] != NULL )
    {
	varcount++;
    }
    // return ARRAYSIZE(variables);
    return varcount;
}

// and now the tricky part (more dirty as dirty)
void do_filtertable( struct mime_handler *handler, char *path, webs_t stream,
		     char *query )
{
    char *temp2 = &path[indexof( path, '-' ) + 1];
    char ifname[16];

    strcpy( ifname, temp2 );
    ifname[indexof( ifname, '.' )] = 0;
    FILE *web = getWebsFile( "WL_FilterTable.asp" );
    char temp[4096];

    memset( temp, 0, 4096 );
    unsigned int len = getWebsFileLen( "WL_FilterTable.asp" );
    char *webfile = ( char * )malloc( len + 1 );

    fread( webfile, len, 1, web );
    webfile[len] = 0;
    sprintf( temp, webfile, ifname, ifname, ifname, ifname );
    free( webfile );
    fclose( web );
    do_ej_buffer( temp, stream );
}

void do_wds( struct mime_handler *handler, char *path, webs_t stream,
	     char *query )
{
    char *temp2 = &path[indexof( path, '-' ) + 1];
    char ifname[16];

    strcpy( ifname, temp2 );
    ifname[indexof( ifname, '.' )] = 0;
    FILE *web = getWebsFile( "Wireless_WDS.asp" );
    unsigned int len = getWebsFileLen( "Wireless_WDS.asp" );
    char *webfile = ( char * )malloc( len + 1 );

    fread( webfile, len, 1, web );
    webfile[len] = 0;
    fclose( web );

    char temp[32768];

    memset( temp, 0, 32768 );
    int ai = 0;
    int i = 0;
    int weblen = strlen( webfile );

    for( i = 0; i < weblen; i++ )
    {
	if( webfile[i] == '%' )
	{
	    i++;
	    switch ( webfile[i] )
	    {
		case '%':
		    temp[ai++] = '%';
		    break;
		case 's':
		    strcpy( &temp[ai], ifname );
		    ai += strlen( ifname );
		    break;
		default:
		    temp[ai++] = webfile[i];
		    break;
	    }
	}
	else
	    temp[ai++] = webfile[i];
    }
    free( webfile );
    do_ej_buffer( temp, stream );
}

void do_wireless_adv( struct mime_handler *handler, char *path, webs_t stream,
	     char *query )
{
    char *temp2 = &path[indexof( path, '-' ) + 1];
    char ifname[16];

    strcpy( ifname, temp2 );
    ifname[indexof( ifname, '.' )] = 0;
    FILE *web = getWebsFile( "Wireless_Advanced.asp" );
    unsigned int len = getWebsFileLen( "Wireless_Advanced.asp" );
    char *webfile = ( char * )malloc( len + 1 );

	char index[2];
	substring (strlen (ifname)-1, strlen (ifname), ifname, index);
    
    fread( webfile, len, 1, web );
    webfile[len] = 0;
    fclose( web );

    char temp[65536];

    memset( temp, 0, 65536 );
    int ai = 0;
    int i = 0;
    int weblen = strlen( webfile );

    for( i = 0; i < weblen; i++ )
    {
	if( webfile[i] == '%' )
	{
	    i++;
	    switch ( webfile[i] )
	    {
		case '%':
		    temp[ai++] = '%';
		    break;
		case 'd':
		    strcpy( &temp[ai], index );
		    ai ++;
		    break;
		case 's':
		    strcpy( &temp[ai], ifname );
		    ai += strlen( ifname );
		    break;
		default:
		    temp[ai++] = webfile[i];
		    break;
	    }
	}
	else
	    temp[ai++] = webfile[i];
    }
    free( webfile );
    do_ej_buffer( temp, stream );
}

static void validate_cgi( webs_t wp )
{
    char *value;
    int i;

#ifdef HAVE_MACBIND
    if( !nvram_match( "et0macaddr", MACBRAND ) )
	return;
#endif
    int alen = variables_arraysize(  );
    void *handle = NULL;

    for( i = 0; i < alen; i++ )
    {
	if( variables[i] == NULL )
	    return;
	value = websGetVar( wp, variables[i]->name, NULL );
	if( !value )
	    continue;
	if( ( !*value && variables[i]->nullok )
	    || ( !variables[i]->validate2name
		 && !variables[i]->validatename ) )
	    nvram_set( variables[i]->name, value );
	else
	{
	    if( variables[i]->validatename )
	    {
		cprintf( "call validator_nofree %s\n",
			 variables[i]->validatename );
		handle =
		    start_validator_nofree( variables[i]->validatename,
					    handle, wp, value, variables[i] );
	    }
	    else if( variables[i]->validate2name )
	    {
		cprintf( "call gozila %s\n", variables[i]->validate2name );
		start_gozila( variables[i]->validate2name, wp );
		// fprintf(stderr,"validating %s =
		// %s\n",variables[i]->name,value);
		// variables[i]->validate (wp, value, variables[i]);
	    }
	    else
	    {
		// variables[i]->validate2 (wp);
	    }
	}

    }
    cprintf( "close handle\n" );
    if( handle )
	dlclose( handle );
    cprintf( "all vars validated\n" );
}

enum
{
    NOTHING,
    REBOOT,
    RESTART,
    SERVICE_RESTART,
    SYS_RESTART,
    REFRESH,
};

static struct gozila_action gozila_actions[] = {
    /*
     * SETUP 
     */
    {"index", "wan_proto", "", 1, REFRESH, "wan_proto"},
    {"index", "dhcpfwd", "", 1, REFRESH, "dhcpfwd"},
    // {"index", "clone_mac", "", 1, REFRESH, clone_mac}, //OBSOLETE
#ifdef HAVE_CCONTROL
    {"ccontrol", "execute", "", 1, REFRESH, "execute"},
#endif
    {"WanMAC", "clone_mac", "", 1, REFRESH, "clone_mac"},	// for cisco
    // style
    {"DHCPTable", "delete", "", 2, REFRESH, "delete_leases"},
    {"Info", "refresh", "", 0, REFRESH, "save_wifi"},
    {"Status_Wireless", "refresh", "", 0, REFRESH, "save_wifi"},
    // {"Status", "release", "dhcp_release", 0, SYS_RESTART, "dhcp_release"},
    // {"Status", "renew", "", 3, REFRESH, "dhcp_renew"},
    // {"Status", "Connect", "start_pppoe", 1, RESTART, NULL},
    {"Status_Internet", "release", "dhcp_release", 0, SERVICE_RESTART, "dhcp_release"},	// for 
    // cisco 
    // style
    {"Status_Internet", "renew", "", 3, REFRESH, "dhcp_renew"},	// for cisco
    // style
    {"Status_Internet", "Disconnect", "stop_pppoe", 2, SERVICE_RESTART, "stop_ppp"},	// for 
    // cisco 
    // style
    {"Status_Internet", "Connect_pppoe", "start_pppoe", 1, RESTART, NULL},	// for 
    // cisco 
    // style
    {"Status_Internet", "Disconnect_pppoe", "stop_pppoe", 2, SERVICE_RESTART, "stop_ppp"},	// for 
    // cisco 
    // style
    {"Status_Internet", "Connect_pptp", "start_pptp", 1, RESTART, NULL},	// for 
    // cisco 
    // style
    {"Status_Internet", "Disconnect_pptp", "stop_pptp", 2, SERVICE_RESTART, "stop_ppp"},	// for 
    // cisco 
    // style
    {"Status_Internet", "Connect_l2tp", "start_l2tp", 1, RESTART, NULL},	// for 
    // cisco 
    // style
    {"Status_Internet", "Disconnect_l2tp", "stop_l2tp", 2, SERVICE_RESTART, "stop_ppp"},	// for 
    // cisco 
    // style{ 
    // "Status_Router", 
    // "Connect_heartbeat", 
    // "start_heartbeat", 
    // 1, 
    // RESTART, 
    // NULL}, 
    // // 
    // for 
    // cisco 
    // style
    {"Status_Internet", "Disconnect_heartbeat", "stop_heartbeat", 2, SERVICE_RESTART, "stop_ppp"},	// for 
    // cisco 
    // style
    {"Status_Internet", "delete_ttraffdata", "", 0, REFRESH, "ttraff_erase"},
    // {"Status", "Disconnect", "stop_pppoe", 2, SYS_RESTART, "stop_ppp"},
    // {"Status", "Connect_pppoe", "start_pppoe", 1, RESTART, NULL},
    // {"Status", "Disconnect_pppoe", "stop_pppoe", 2, SYS_RESTART, "stop_ppp"},
    // {"Status", "Connect_pptp", "start_pptp", 1, RESTART, NULL},
    // {"Status", "Disconnect_pptp", "stop_pptp", 2, SYS_RESTART, "stop_ppp"},
    // {"Status", "Connect_heartbeat", "start_heartbeat", 1, RESTART, NULL},
    // {"Status", "Disconnect_heartbeat", "stop_heartbeat", 2, SYS_RESTART, "stop_ppp"},
    {"Filters", "save", "filters", 1, REFRESH, "save_policy"},
    {"Filters", "delete", "filters", 1, REFRESH, "single_delete_policy"},
    {"FilterSummary", "delete", "filters", 1, REFRESH,
     "summary_delete_policy"},
    {"Routing", "del", "static_route_del", 1, REFRESH,"delete_static_route"},
    {"RouteStatic", "del", "static_route_del", 1, REFRESH,"delete_static_route"},
    {"WL_WPATable", "wep_key_generate", "", 1, REFRESH, "generate_wep_key"},
    {"WL_WPATable", "security", "", 1, REFRESH, "set_security"},
#ifdef HAVE_MSSID
    {"WL_WPATable", "save", "wireless_2", 1, REFRESH, "security_save"},
    {"WL_WPATable", "keysize", "wireless_2", 1, REFRESH, "security_save"},
#endif
    {"WL_ActiveTable", "add_mac", "", 1, REFRESH, "add_active_mac"},
    /*
     * Siafu addition 
     */
    {"Ping", "wol", "", 1, REFRESH, "ping_wol"},
    /*
     * Sveasoft addition 
     */
    // {"Wireless_WDS", "save", "", 0, REFRESH, save_wds},
#ifndef HAVE_MADWIFI
    {"Wireless_WDS-wl0", "save", "wireless", 0, REFRESH, "save_wds"},
    {"Wireless_WDS-wl1", "save", "wireless", 0, REFRESH, "save_wds"},
#else
    {"Wireless_WDS-ath0", "save", "wireless", 0, REFRESH, "save_wds"},
    {"Wireless_WDS-ath1", "save", "wireless", 0, REFRESH, "save_wds"},
    {"Wireless_WDS-ath2", "save", "wireless", 0, REFRESH, "save_wds"},
    {"Wireless_WDS-ath3", "save", "wireless", 0, REFRESH, "save_wds"},
#endif
    {"Ping", "startup", "", 1, SYS_RESTART, "ping_startup"},
    {"Ping", "shutdown", "", 1, SYS_RESTART, "ping_shutdown"},
    {"Ping", "firewall", "", 1, SYS_RESTART, "ping_firewall"},
    {"Ping", "custom", "", 0, REFRESH, "ping_custom"},
    {"QoS", "add_svc", "", 0, REFRESH, "qos_add_svc"},
    {"QoS", "add_ip", "", 0, REFRESH, "qos_add_ip"},
    {"QoS", "add_mac", "", 0, REFRESH, "qos_add_mac"},
    {"QoS", "save", "filters", 1, REFRESH, "qos_save"},
    /*
     * end Sveasoft addition 
     */
    {"Forward", "add_forward", "", 0, REFRESH, "forward_add"},
    {"Forward", "remove_forward", "", 0, REFRESH, "forward_remove"},
#ifdef HAVE_MSSID
    {"Wireless_Basic", "add_vifs", "", 0, REFRESH, "add_vifs"},
    {"Wireless_Basic", "remove_vifs", "", 0, REFRESH, "remove_vifs"},
#endif
#ifdef HAVE_BONDING
    {"Networking", "add_bond", "", 0, REFRESH, "add_bond"},
    {"Networking", "del_bond", "", 0, REFRESH, "del_bond"},
#endif
#ifdef HAVE_OLSRD
    {"Routing", "add_olsrd", "", 0, REFRESH, "add_olsrd"},
    {"Routing", "del_olsrd", "", 0, REFRESH, "del_olsrd"},
#endif
#ifdef HAVE_VLANTAGGING
    {"Networking", "add_vlan", "", 0, REFRESH, "add_vlan"},
    {"Networking", "add_bridge", "", 0, REFRESH, "add_bridge"},
    {"Networking", "add_bridgeif", "", 0, REFRESH, "add_bridgeif"},
    {"Networking", "del_vlan", "", 0, REFRESH, "del_vlan"},
    {"Networking", "del_bridge", "", 0, REFRESH, "del_bridge"},
    {"Networking", "del_bridgeif", "", 0, REFRESH, "del_bridgeif"},
    {"Networking", "save_networking", "index", 0, REFRESH, "save_networking"},
    {"Networking", "add_mdhcp", "", 0, REFRESH, "add_mdhcp"},
    {"Networking", "del_mdhcp", "", 0, REFRESH, "del_mdhcp"},
#endif
    {"Wireless_Basic", "save", "wireless", 1, REFRESH, "wireless_save"},
#ifdef HAVE_WIVIZ
    {"Wiviz_Survey", "Set", "", 0, REFRESH, "set_wiviz"},
#endif
#ifdef HAVE_REGISTER
    {"Register", "activate", "", 1, RESTART, "reg_validate"},
#endif
    {"index", "changepass", "", 1, REFRESH, "changepass"},
#ifdef HAVE_SUPERCHANNEL
    {"SuperChannel", "activate", "", 1, REFRESH, "superchannel_validate"},
#endif
    {"Services", "add_lease", "", 0, REFRESH, "lease_add"},
    {"Services", "remove_lease", "", 0, REFRESH, "lease_remove"},
#ifdef HAVE_PPPOESERVER
    {"PPPoE_Server", "add_chap_user", "", 0, REFRESH, "chap_user_add"},
    {"PPPoE_Server", "remove_chap_user", "", 0, REFRESH, "chap_user_remove"},
#endif
#ifdef HAVE_CHILLILOCAL
    {"Hotspot", "add_user", "", 0, REFRESH, "user_add"},
    {"Hotspot", "remove_user", "", 0, REFRESH, "user_remove"},
#endif
#ifdef HAVE_RADLOCAL
    {"Hotspot", "add_iradius", "", 0, REFRESH, "raduser_add"},
#endif
    {"ForwardSpec", "add_forward_spec", "", 0, REFRESH, "forwardspec_add"},
    {"ForwardSpec", "remove_forward_spec", "", 0, REFRESH,
     "forwardspec_remove"},
    {"Triggering", "add_trigger", "", 0, REFRESH, "trigger_add"},
    {"Triggering", "remove_trigger", "", 0, REFRESH, "trigger_remove"},
    {"Port_Services", "save_services", "filters", 2, REFRESH,
     "save_services_port"},
    {"QOSPort_Services", "save_qosservices", "filters", 2, REFRESH,
     "save_services_port"},
    {"Ping", "start", "", 1, SERVICE_RESTART, "diag_ping_start"},
    {"Ping", "stop", "", 0, REFRESH, "diag_ping_stop"},
    {"Ping", "clear", "", 0, REFRESH, "diag_ping_clear"},
#ifdef HAVE_MILKFISH
    {"Milkfish_database", "add_milkfish_user", "", 0, REFRESH,
     "milkfish_user_add"},
    {"Milkfish_database", "remove_milkfish_user", "", 0, REFRESH,
     "milkfish_user_remove"},
    {"Milkfish_aliases", "add_milkfish_alias", "", 0, REFRESH,
     "milkfish_alias_add"},
    {"Milkfish_aliases", "remove_milkfish_alias", "", 0, REFRESH,
     "milkfish_alias_remove"},
    {"Milkfish_messaging", "send_message", "", 1, SERVICE_RESTART,
     "milkfish_sip_message"},
#endif
};

struct gozila_action *handle_gozila_action( char *name, char *type )
{
    struct gozila_action *v;

    if( !name || !type )
	return NULL;

    for( v = gozila_actions;
	 v < &gozila_actions[STRUCT_LEN( gozila_actions )]; v++ )
    {
	if( !strcmp( v->name, name ) && !strcmp( v->type, type ) )
	{
	    return v;
	}
    }
    return NULL;
}

char my_next_page[30] = "";
int
gozila_cgi( webs_t wp, char_t * urlPrefix, char_t * webDir, int arg,
	    char_t * url, char_t * path, char_t * query )
{
    char *submit_button, *submit_type, *next_page;
    int action = REFRESH;
    int sleep_time;
    struct gozila_action *act;

    nvram_set( "gozila_action", "1" );
    my_next_page[0] = '\0';
    submit_button = websGetVar( wp, "submit_button", NULL );	/* every html 
								 * must have
								 * the name */
    submit_type = websGetVar( wp, "submit_type", NULL );	/* add, del,
								 * renew,
								 * release
								 * ..... */

    fprintf( stderr, "submit_button=[%s] submit_type=[%s]\n", submit_button,
	     submit_type );
    act = handle_gozila_action( submit_button, submit_type );

    if( act )
    {
	fprintf( stderr,
		 "name=[%s] type=[%s] service=[%s] sleep=[%d] action=[%d]\n",
		 act->name, act->type, act->service, act->sleep_time,
		 act->action );
	addAction( act->service );
	sleep_time = act->sleep_time;
	action = act->action;
	if( act->goname )
	{
	    start_gozila( act->goname, wp );
	}
    }
    else
    {
	sleep_time = 0;
	action = REFRESH;
    }

    if( action == REFRESH )
    {
	sleep( sleep_time );
    }
    else if( action == SERVICE_RESTART )
    {
	sys_commit(  );
	service_restart(  );
	sleep( sleep_time );
    }
    else if( action == SYS_RESTART )
    {
	sys_commit(  );
	sys_restart(  );
    }
    else if( action == RESTART )
    {
	sys_commit(  );
	sys_restart(  );
    }

    if( my_next_page[0] != '\0' )
    {
	sprintf( path, "%s", my_next_page );
    }
    else
    {
	next_page = websGetVar( wp, "next_page", NULL );
	if( next_page )
	    sprintf( path, "%s", next_page );
	else
	    sprintf( path, "%s.asp", submit_button );
    }

    cprintf( "refresh to %s\n", path );
    if( !strncmp( path, "WL_FilterTable", 14 ) )
	do_filtertable( NULL, path, wp, NULL );	// refresh
    // #ifdef HAVE_MADWIFI
    else if( !strncmp( path, "Wireless_WDS", 12 ) )
	do_wds( NULL, path, wp, NULL );	// refresh
    // #endif
    else if( !strncmp( path, "Wireless_Advanced", 17 ) )
	do_wireless_adv( NULL, path, wp, NULL );	// refresh
    else
	do_ej( NULL, path, wp, NULL );	// refresh
    websDone( wp, 200 );

    nvram_set( "gozila_action", "0" );
    nvram_set( "generate_key", "0" );
    nvram_set( "clone_wan_mac", "0" );

    return 1;
}

struct apply_action apply_actions[] = {
    /*
     * name, service, sleep_time, action, function_to_execute 
     */

    /*
     * SETUP 
     */
    {"index", "index", 0, SERVICE_RESTART, NULL},
    {"DDNS", "ddns", 0, SERVICE_RESTART, "ddns_save_value"},
    {"Routing", "routing", 0, SERVICE_RESTART, NULL},
    {"Vlan", "", 0, SYS_RESTART, "port_vlan_table_save"},
    {"eop-tunnel", "eop", 0, SERVICE_RESTART, NULL},

    /*
     * WIRELESS 
     */
    {"Wireless_Basic", "wireless", 0, SERVICE_RESTART, NULL},	// Only for
    // V23, since 
    // V24 it's a 
    // gozilla
    // save
    {"Wireless_Advanced-wl0", "wireless_2", 0, SERVICE_RESTART, NULL},
    {"Wireless_Advanced-wl1", "wireless_2", 0, SERVICE_RESTART, NULL},
    {"Wireless_Advanced", "wireless_2", 0, SERVICE_RESTART, NULL},
    {"Wireless_MAC", "wireless_2", 0, SERVICE_RESTART, "save_macmode"},
    {"WL_FilterTable", "macfilter", 0, SERVICE_RESTART, NULL},
    {"Wireless_WDS", "wireless_2", 0, SERVICE_RESTART, NULL},
    {"WL_WPATable", "wireless_2", 0, SERVICE_RESTART, NULL},

    /*
     * MANAGEMENT 
     */
    {"Management", "management", 0, SYS_RESTART, NULL},
    {"Services", "services", 0, SERVICE_RESTART, NULL},
    {"Alive", "alive", 0, SERVICE_RESTART, NULL},

    /*
     * SERVICES 
     */
    {"PPPoE_Server", "services", 0, SERVICE_RESTART, NULL},
    {"PPTP", "services", 0, SERVICE_RESTART, NULL},
    {"USB", "", 0, REBOOT, NULL},     
    {"NAS", "nassrv", 0, SERVICE_RESTART, NULL},  
    {"Hotspot", "hotspot", 0, SERVICE_RESTART, NULL},
    {"AnchorFree", "anchorfree", 0, SERVICE_RESTART, NULL},

    /*
     * APP & GAMING 
     */
    {"Forward", "forward", 0, SERVICE_RESTART, NULL},
    {"ForwardSpec", "forward", 0, SERVICE_RESTART, NULL},
    {"Triggering", "filters", 0, SERVICE_RESTART, NULL},
    {"DMZ", "filters", 0, SERVICE_RESTART, NULL},
    {"Filters", "filters", 0, SERVICE_RESTART, NULL},
    {"FilterIPMAC", "filters", 0, SERVICE_RESTART, NULL},
#ifdef HAVE_UPNP
    {"UPnP", "forward_upnp", 0, SERVICE_RESTART, "tf_upnp"},
#endif
    /*
     * SECURITY 
     */
    {"Firewall", "filters", 0, SERVICE_RESTART, NULL},
    {"VPN", "filters", 0, SERVICE_RESTART, NULL},
#ifdef HAVE_MILKFISH
    {"Milkfish", "milkfish", 0, SERVICE_RESTART, NULL},
#endif
    /*
     * Obsolete {"WL_WEPTable", "", 0, SERVICE_RESTART, NULL}, {"Security",
     * "", 1, RESTART, NULL}, {"System", "", 0, RESTART, NULL}, {"DHCP",
     * "dhcp", 0, SERVICE_RESTART, NULL}, {"FilterIP", "filters", 0,
     * SERVICE_RESTART, NULL}, {"FilterMAC", "filters", 0, SERVICE_RESTART,
     * NULL}, {"FilterPort", "filters", 0, SERVICE_RESTART, NULL},
     * {"Wireless", "wireless", 0, SERVICE_RESTART, NULL}, {"Log", "logging", 
     * 0, SERVICE_RESTART, NULL}, //moved to Firewall {"QoS", "qos", 0,
     * SERVICE_RESTART, NULL}, //gozilla does the save 
     */

};

struct apply_action *handle_apply_action( char *name )
{
    struct apply_action *v;

    cprintf( "apply name = \n", name );
    if( !name )
	return NULL;

    for( v = apply_actions; v < &apply_actions[STRUCT_LEN( apply_actions )];
	 v++ )
    {
	if( !strcmp( v->name, name ) )
	{
	    return v;
	}
    }

    return NULL;
}

int getFileLen( FILE * in )
{
    int len;

    fseek( in, 0, SEEK_END );
    len = ftell( in );
    rewind( in );
    return len;
}

void do_logout( void )		// static functions are not exportable,
				// additionally this is no ej function
{
    send_authenticate( auth_realm );
}

static int
apply_cgi( webs_t wp, char_t * urlPrefix, char_t * webDir, int arg,
	   char_t * url, char_t * path, char_t * query )
{
    int action = NOTHING;
    char *value;
    char *submit_button, *next_page;
    int sleep_time = 0;
    int need_commit = 1;

    cprintf( "need reboot\n" );
    int need_reboot = atoi( websGetVar( wp, "need_reboot", "0" ) );

    cprintf( "apply" );

	/**********   get "change_action" and launch gozila_cgi if needed **********/

    value = websGetVar( wp, "change_action", "" );
    cprintf( "get change_action = %s\n", value );

    if( value && !strcmp( value, "gozila_cgi" ) )
    {
	gozila_cgi( wp, urlPrefix, webDir, arg, url, path, query );
	return 1;
    }

  /***************************************************************************/

    if( !query )
    {
	goto footer;
    }
    if( legal_ip_netmask
	( "lan_ipaddr", "lan_netmask",
	  nvram_safe_get( "http_client_ip" ) ) == TRUE )
	nvram_set( "browser_method", "USE_LAN" );
    else
	nvram_set( "browser_method", "USE_WAN" );

  /**********   get all webs var **********/

    submit_button = websGetVar( wp, "submit_button", "" );
    cprintf( "get submit_button = %s\n", submit_button );

    need_commit = atoi( websGetVar( wp, "commit", "1" ) );
    cprintf( "get need_commit = %d\n", need_commit );

    value = websGetVar( wp, "action", "" );
    cprintf( "get action = %s\n", value );

  /**********   check action to do **********/

  /** Apply **/
    if( !strcmp( value, "Apply" ) || !strcmp( value, "ApplyTake" ) )
    {
	struct apply_action *act;

	cprintf( "validate cgi" );
	validate_cgi( wp );
	cprintf( "handle apply action\n" );
	act = handle_apply_action( submit_button );
	cprintf( "done\n" );
	// If web page configuration is changed, the EZC configuration
	// function should be disabled.(2004-07-29)
	nvram_set( "is_default", "0" );
	nvram_set( "is_modified", "1" );
	if( act )
	{
	    fprintf( stderr,
		     "%s:submit_button=[%s] service=[%s] sleep_time=[%d] action=[%d]\n",
		     value, act->name, act->service, act->sleep_time,
		     act->action );

	    if( ( act->action == SYS_RESTART )
		|| ( act->action == SERVICE_RESTART ) )
	    {

		addAction( act->service );
	    }
	    sleep_time = act->sleep_time;
	    action = act->action;

	    if( act->goname )
		start_gozila( act->goname, wp );
	}
	else
	{
	    // nvram_set ("action_service", "");
	    sleep_time = 1;
	    action = RESTART;
	}
	diag_led( DIAG, STOP_LED );
	sys_commit(  );
    }

  /** Restore defaults **/
    else if( !strncmp( value, "Restore", 7 ) )
    {
	ACTION( "ACT_SW_RESTORE" );
	nvram_set( "sv_restore_defaults", "1" );
	killall( "udhcpc", SIGKILL );
	sys_commit(  );
#ifdef HAVE_X86
	eval( "mount", "/usr/local", "-o", "remount,rw" );
	eval( "rm", "-f", "/tmp/nvram/*" );	// delete nvram database
	eval( "rm", "-f", "/tmp/nvram/.lock" );	// delete nvram database
	eval( "rm", "-f", "/usr/local/nvram/*" );	// delete nvram
	// database
	eval( "mount", "/usr/local", "-o", "remount,ro" );
#elif HAVE_RB500
	eval( "rm", "-f", "/tmp/nvram/*" );	// delete nvram database
	eval( "rm", "-f", "/tmp/nvram/.lock" );	// delete nvram database
	eval( "rm", "-f", "/etc/nvram/*" );	// delete nvram database
#elif HAVE_MAGICBOX
	eval( "rm", "-f", "/tmp/nvram/*" );	// delete nvram database
	eval( "rm", "-f", "/tmp/nvram/.lock" );	// delete nvram database
	eval( "erase", "nvram" );
#else
	eval( "erase", "nvram" );
#endif
	action = REBOOT;
    }

  /** Reboot **/
    else if( !strncmp( value, "Reboot", 6 ) )
    {
	action = REBOOT;
    }

    /** GUI Logout **/// Experimental, not work yet ... 
    else if( !strncmp( value, "Logout", 6 ) )
    {
	do_ej( NULL, "Logout.asp", wp, NULL );
	websDone( wp, 200 );
	do_logout(  );
	return 1;
    }

    /*
     * DEBUG : Invalid action 
     */
    else
	websDebugWrite( wp, "Invalid action %s<br />", value );

  footer:

    if( nvram_match( "do_reboot", "1" ) )
    {
	action = REBOOT;
    }
    /*
     * The will let PC to re-get a new IP Address automatically 
     */
    if( need_reboot )
	action = REBOOT;

    if( !strcmp( value, "Apply" ) )
    {
	action = NOTHING;
    }

    if( action != REBOOT )
    {
	if( my_next_page[0] != '\0' )
	    sprintf( path, "%s", my_next_page );
	else
	{
	    next_page = websGetVar( wp, "next_page", NULL );
	    if( next_page )
		sprintf( path, "%s", next_page );
	    else
		sprintf( path, "%s.asp", submit_button );
	}

	cprintf( "refresh to %s\n", path );
	if( !strncmp( path, "WL_FilterTable", 14 ) )
	    do_filtertable( NULL, path, wp, NULL );	// refresh
	else if( !strncmp( path, "Wireless_WDS", 12 ) )
	    do_wds( NULL, path, wp, NULL );	// refresh
	else if( !strncmp( path, "Wireless_Advanced", 17 ) )
	    do_wireless_adv( NULL, path, wp, NULL );	// refresh
	else
	    do_ej( NULL, path, wp, NULL );	// refresh
	websDone( wp, 200 );
    }
    else
    {
#ifndef HAVE_WRK54G
	do_ej( NULL, "Reboot.asp", wp, NULL );
	websDone( wp, 200 );
#endif
	// sleep (5);
	sys_reboot(  );
	return 1;
    }

    nvram_set( "upnp_wan_proto", "" );
    sleep( sleep_time );
    if( ( action == RESTART ) || ( action == SYS_RESTART ) )
	sys_restart(  );
    else if( action == SERVICE_RESTART )
	service_restart(  );

    return 1;

}

#ifdef HAVE_SKYTRON
int do_auth( char *userid, char *passwd, char *realm )
{
    strncpy( userid, nvram_safe_get( "skyhttp_username" ), AUTH_MAX );
    strncpy( passwd, nvram_safe_get( "skyhttp_passwd" ), AUTH_MAX );
    // strncpy(realm, MODEL_NAME, AUTH_MAX);
    strncpy( realm, nvram_safe_get( "router_name" ), AUTH_MAX );
    return 0;
}

int do_auth2( char *userid, char *passwd, char *realm )
{
    strncpy( userid, nvram_safe_get( "http_username" ), AUTH_MAX );
    strncpy( passwd, nvram_safe_get( "http_passwd" ), AUTH_MAX );
    // strncpy(realm, MODEL_NAME, AUTH_MAX);
    strncpy( realm, nvram_safe_get( "router_name" ), AUTH_MAX );
    return 0;
}
#else

int do_auth( char *userid, char *passwd, char *realm )
{
    strncpy( userid, nvram_safe_get( "http_username" ), AUTH_MAX );
    strncpy( passwd, nvram_safe_get( "http_passwd" ), AUTH_MAX );
    // strncpy(realm, MODEL_NAME, AUTH_MAX);
    strncpy( realm, nvram_safe_get( "router_name" ), AUTH_MAX );
    return 0;
}

int do_cauth( char *userid, char *passwd, char *realm )
{
    if( nvram_match( "info_passwd", "0" ) )
	return -1;
    return do_auth( userid, passwd, realm );
}
#endif

#ifdef HAVE_REGISTER
int do_auth_reg( char *userid, char *passwd, char *realm )
{
    if( !isregistered(  ) )
	return -1;
    return do_auth( userid, passwd, realm );
}
#endif

#undef HAVE_DDLAN

#ifdef HAVE_DDLAN
int do_auth2( char *userid, char *passwd, char *realm )
{
    strncpy( userid, nvram_safe_get( "http2_username" ), AUTH_MAX );
    strncpy( passwd, nvram_safe_get( "http2_passwd" ), AUTH_MAX );
    // strncpy(realm, MODEL_NAME, AUTH_MAX);
    strncpy( realm, nvram_safe_get( "router_name" ), AUTH_MAX );
    return 0;
}
#endif
// #ifdef EZC_SUPPORT
char ezc_version[128];

// #endif

extern int post;

static char *post_buf = NULL;
static void			// support GET and POST 2003-08-22
do_apply_post( char *url, webs_t stream, int len, char *boundary )
{
    unsigned char buf[1024];
    int count;
    if( post == 1 )
    {
	if (post_buf)
	    post_buf=(char *)realloc(post_buf,len+1);
	else
	    post_buf = (char *)malloc(len+1);
	
	if( !post_buf )
	{
	    cprintf( "The POST data exceed length limit!\n" );
	    return;
	}
	/*
	 * Get query 
	 */
	if( !( count = wfread( post_buf, 1, len, stream ) ) )
	    return;
	post_buf[count] = '\0';;
	len -= strlen( post_buf );

	/*
	 * Slurp anything remaining in the request 
	 */
	while( --len > 0 )
#ifdef HAVE_HTTPS
	    if( do_ssl )
		wfgets( buf, 1, stream );
	    else
#endif
		( void )fgetc( stream );
	init_cgi( post_buf );
    }
}

#if !defined(HAVE_X86) && !defined(HAVE_MAGICBOX)
static void do_cfebackup( struct mime_handler *handler, char *url,
			  webs_t stream, char *query )
{
    system2( "cat /dev/mtd/0 > /tmp/cfe.bin" );
    do_file_attach( handler, "/tmp/cfe.bin", stream, NULL, "cfe.bin" );
    unlink( "/tmp/cfe.bin" );
}
#endif

#ifdef HAVE_ROUTERSTYLE
static void do_stylecss( struct mime_handler *handler, char *url,
			 webs_t stream, char *query )
{
    char *style = nvram_get( "router_style" );

    if( query != NULL )
	style = query;

    long sdata[30];

    long blue[30] =
	{ 0x36f, 0xfff, 0x68f, 0x24d, 0x24d, 0x68f, 0x57f, 0xccf, 0x78f,
	0x35d,
	0x35c, 0x78f,
	0x78f, 0xfff, 0x9af, 0x46e, 0x46e, 0x9af, 0x36f, 0xccf, 0xfff, 0x69f,
	0xfff, 0xfff,
	0x999, 0x69f, 0x69f, 0xccf, 0x78f, 0xfff
    };

    long cyan[30] =
	{ 0x099, 0xfff, 0x3bb, 0x066, 0x066, 0x3bb, 0x3bb, 0xcff, 0x4cc,
	0x1aa,
	0x1aa, 0x4cc,
	0x6cc, 0xfff, 0x8dd, 0x5bb, 0x5bb, 0x8dd, 0x099, 0xcff, 0xfff, 0x3bb,
	0xfff, 0xfff,
	0x999, 0x3bb, 0x3bb, 0xcff, 0x6cc, 0xfff
    };

    long elegant[30] =
	{ 0x30519c, 0xfff, 0x496fc7, 0x496fc7, 0x496fc7, 0x496fc7, 0x496fc7,
	0xfff, 0x6384cf, 0x6384cf, 0x6384cf, 0x6384cf,
	0x6384cf, 0xfff, 0x849dd9, 0x849dd9, 0x849dd9, 0x849dd9, 0x30519c,
	0xfff,
	0xfff, 0x496fc7, 0xfff, 0xfff,
	0x999, 0x496fc7, 0x496fc7, 0xfff, 0x6384cf, 0xfff
    };

    long green[30] =
	{ 0x090, 0xfff, 0x3b3, 0x060, 0x060, 0x3b3, 0x3b3, 0xcfc, 0x4c4,
	0x1a1,
	0x1a1, 0x4c4,
	0x6c6, 0xfff, 0x8d8, 0x5b5, 0x5b5, 0x8d8, 0x090, 0xcfc, 0xfff, 0x3b3,
	0xfff, 0xfff,
	0x999, 0x3b3, 0x3b3, 0xcfc, 0x6c6, 0xfff
    };

    long orange[30] =
	{ 0xf26522, 0xfff, 0xff8400, 0xff8400, 0xff8400, 0xff8400, 0xff8400,
	0xfff, 0xfeb311, 0xfeb311, 0xfeb311, 0xfeb311,
	0xff9000, 0xfff, 0xffa200, 0xffa200, 0xffa200, 0xffa200, 0xf26522,
	0xfff,
	0xfff, 0xff8400, 0xfff, 0xfff,
	0x999, 0xff8400, 0xff8400, 0xfff, 0xff9000, 0xfff
    };

    long red[30] =
	{ 0xc00, 0xfff, 0xe33, 0x800, 0x800, 0xe33, 0xd55, 0xfcc, 0xe77,
	0xc44,
	0xc44, 0xe77,
	0xe77, 0xfff, 0xf99, 0xd55, 0xd55, 0xf99, 0xc00, 0xfcc, 0xfff, 0xd55,
	0xfff, 0xfff,
	0x999, 0xd55, 0xd55, 0xfcc, 0xe77, 0xfff
    };

    long yellow[30] =
	{ 0xeec900, 0x000, 0xee3, 0x880, 0x880, 0xee3, 0xffd700, 0x660, 0xee7,
	0xbb4,
	0xbb4, 0xee7,
	0xeec900, 0x000, 0xff9, 0xcc5, 0xcc5, 0xff9, 0xeec900, 0x660, 0x000,
	    0xffd700,
	0x000, 0xfff,
	0x999, 0xffd700, 0xeec900, 0x660, 0xffd700, 0x000
    };

    if( !strcmp( style, "blue" ) )
	memcpy( sdata, blue, 30 * sizeof( long ) );
    else if( !strcmp( style, "cyan" ) )
	memcpy( sdata, cyan, 30 * sizeof( long ) );
    else if( !strcmp( style, "yellow" ) )
	memcpy( sdata, yellow, 30 * sizeof( long ) );
    else if( !strcmp( style, "green" ) )
	memcpy( sdata, green, 30 * sizeof( long ) );
    else if( !strcmp( style, "orange" ) )
	memcpy( sdata, orange, 30 * sizeof( long ) );
    else if( !strcmp( style, "red" ) )
	memcpy( sdata, red, 30 * sizeof( long ) );
    else			// default to elegant
	memcpy( sdata, elegant, 30 * sizeof( long ) );

    websWrite( stream, "@import url(../common.css);\n" );
    websWrite( stream, "#menuSub,\n" );
    websWrite( stream, "#menuMainList li span,\n" );
    websWrite( stream, "#help h2 {\n" );
    websWrite( stream, "background:#%03x;\n", sdata[0] );
    websWrite( stream, "color:#%03x;\n", sdata[1] );
    websWrite( stream, "border-color:#%03x #%03x #%03x #%03x;\n", sdata[2],
	       sdata[3], sdata[4], sdata[5] );
    websWrite( stream, "}\n" );
    websWrite( stream, "#menuSubList li a {\n" );
    websWrite( stream, "background:#%03x;\n", sdata[6] );
    websWrite( stream, "color:#%03x;\n", sdata[7] );
    websWrite( stream, "border-color:#%03x #%03x #%03x #%03x;\n", sdata[8],
	       sdata[9], sdata[10], sdata[11] );
    websWrite( stream, "}\n" );
    websWrite( stream, "#menuSubList li a:hover {\n" );
    websWrite( stream, "background:#%03x;\n", sdata[12] );
    websWrite( stream, "color:#%03x;\n", sdata[13] );
    websWrite( stream, "border-color:#%03x #%03x #%03x #%03x;\n", sdata[14],
	       sdata[15], sdata[16], sdata[17] );
    websWrite( stream, "}\n" );
    websWrite( stream, "fieldset legend {\n" );
    websWrite( stream, "color:#%03x;\n", sdata[18] );
    websWrite( stream, "}\n" );
    websWrite( stream, "#help a {\n" );
    websWrite( stream, "color:#%03x;\n", sdata[19] );
    websWrite( stream, "}\n" );
    websWrite( stream, "#help a:hover {\n" );
    websWrite( stream, "color:#%03x;\n", sdata[20] );
    websWrite( stream, "}\n" );
    websWrite( stream, ".meter .bar {\n" );
    websWrite( stream, "background-color: #%03x;\n", sdata[21] );
    websWrite( stream, "}\n" );
    websWrite( stream, ".meter .text {\n" );
    websWrite( stream, "color:#%03x;\n", sdata[22] );
    websWrite( stream, "}\n" );
    websWrite( stream, ".progressbar {\n" );
    websWrite( stream, "background-color: #%03x;\n", sdata[23] );
    websWrite( stream, "border-color: #%03x;\n", sdata[24] );
    websWrite( stream, "font-size:.09em;\n" );
    websWrite( stream, "border-width:.09em;\n" );
    websWrite( stream, "}\n" );
    websWrite( stream, ".progressbarblock {\n" );
    websWrite( stream, "background-color: #%03x;\n", sdata[25] );
    websWrite( stream, "font-size:.09em;\n" );
    websWrite( stream, "}\n" );
    websWrite( stream, "input.button {\n" );
    websWrite( stream, "background: #%03x;\n", sdata[26] );
    websWrite( stream, "color: #%03x;\n", sdata[27] );
    websWrite( stream, "}\n" );
    websWrite( stream, "input.button:hover {\n" );
    websWrite( stream, "background: #%03x;\n", sdata[28] );
    websWrite( stream, "color: #%03x;\n", sdata[29] );
    websWrite( stream, "}\n" );

}

static void do_stylecss_ie( struct mime_handler *handler, char *url,
			    webs_t stream, char *query )
{
    websWrite( stream, ".submitFooter input {\n"
	       "padding:.362em .453em;\n"
	       "}\n"
	       "fieldset {\n"
	       "padding-top:0;\n"
	       "}\n"
	       "fieldset legend {\n"
	       "margin-left:-9px;\n"
	       "margin-bottom:8px;\n" "padding:0 .09em;\n" "}\n" );
}
#endif

#ifdef HAVE_REGISTER
static void do_trial_logo( struct mime_handler *handler, char *url,
			   webs_t stream, char *query )
{
    if( !isregistered_real(  ) )
    {
	do_file( handler, "style/logo-trial.png", stream, query );
    }
    else
    {
	do_file( handler, url, stream, query );
    }
}

#endif
/*
 * static void do_style (char *url, webs_t stream, char *query) { char *style 
 * = nvram_get ("router_style"); if (style == NULL || strlen (style) == 0)
 * do_file ("kromo.css", stream, NULL); else do_file (style, stream, NULL); } 
 */

static void do_fetchif( struct mime_handler *handler, char *url,
			webs_t stream, char *query )
{
    char line[256];
    int i, llen;
    char buffer[256];

    if( query == NULL || strlen( query ) == 0 )
	return;
    int strbuffer = 0;
    time_t tm;
    struct tm tm_time;

    time( &tm );
    memcpy( &tm_time, localtime( &tm ), sizeof( tm_time ) );
    char *date_fmt = "%a %b %e %H:%M:%S %Z %Y";

    strftime( buffer, 200, date_fmt, &tm_time );
    strbuffer = strlen( buffer );
    buffer[strbuffer++] = '\n';
    FILE *in = fopen( "/proc/net/dev", "rb" );

    if( in == NULL )
	return;

    while( fgets( line, sizeof( line ), in ) != NULL )
    {
	if( !strchr( line, ':' ) )
	    continue;
	if( strstr( line, query ) )
	{
	    llen = strlen( line );
	    for( i = 0; i < llen; i++ )
	    {
		buffer[strbuffer++] = line[i];
	    }
	    break;
	}
    }
    buffer[strbuffer] = 0;
    fclose( in );
    websWrite( stream, "%s", buffer );
}

static char *getLanguageName(  )
{
    char *lang = nvram_get( "language" );

    cprintf( "get language %s\n", lang );
    char *l = malloc( 60 );

    if( lang == NULL )
    {
	cprintf( "return default\n" );
	sprintf( l, "lang_pack/english.js" );
	return l;
    }
    sprintf( l, "lang_pack/%s.js", lang );
    cprintf( "return %s\n", l );
    return l;
}

char *live_translate( char *tran )
{

    FILE *fp;
    static char temp[256], temp1[256];
    char *temp2;
    char *lang = getLanguageName(  );
    char buf[64];

    memset( temp, 0, sizeof( temp ) );
    memset( temp1, 0, sizeof( temp ) );

    sprintf( buf, "%s", lang );
    free( lang );

    strcpy( temp1, tran );
    strcat( temp1, "=\"" );

    int len = strlen( temp1 );

    fp = getWebsFile( buf );
    if( fp == NULL )
	return "Error";
    int start = ftell( fp );
    int filelen = getWebsFileLen( buf );

    while( fgets( temp, 256, fp ) != NULL )
    {
	int pos = ftell( fp );

	if( ( pos - start ) > filelen )
	    break;
	if( ( memcmp( temp, temp1, len ) ) == 0 )
	{
	    temp2 = strtok( temp, "\"" );
	    temp2 = strtok( NULL, "\"" );

	    fclose( fp );
	    return temp2;
	}
    }
    fclose( fp );

    fp = getWebsFile( "lang_pack/english.js" );	// if not found, try english
    if( fp == NULL )
	return "Error";

    while( fgets( temp, 256, fp ) != NULL )
    {
	if( ( memcmp( temp, temp1, len ) ) == 0 )
	{
	    temp2 = strtok( temp, "\"" );
	    temp2 = strtok( NULL, "\"" );

	    fclose( fp );
	    return temp2;
	}
    }
    fclose( fp );

    return "Error";		// not found

}

static void do_ttgraph( struct mime_handler *handler, char *url,
			webs_t stream, char *query )
{
#define COL_WIDTH 16		/* single column width */

    char *next;
    char var[80];

    unsigned int days;
    unsigned int month;
    unsigned int year;
    int wd;
    int i = 0;
    char months[12][12] =
	{ "share.jan", "share.feb", "share.mar", "share.apr", "share.may",
	"share.jun",
	"share.jul", "share.aug", "share.sep", "share.oct", "share.nov",
	"share.dec"
    };
    unsigned long rcvd[31] =
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0,
	0, 0, 0, 0, 0, 0, 0
    };
    unsigned long sent[31] =
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0,
	0, 0, 0, 0, 0, 0, 0
    };
    unsigned long max = 5, smax = 5, f = 1;
    unsigned long totin = 0;
    unsigned long totout = 0;

    if( sscanf( query, "%u-%u", &month, &year ) != 2 )
	return;

    days = daysformonth( month, year );
    wd = weekday( month, 1, year );	// first day in month (mon=0, tue=1,
    // ..., sun=6)

    char tq[32];

    sprintf( tq, "traff-%02u-%u", month, year );
    char *tdata = nvram_safe_get( tq );

    if( tdata != NULL || strlen( tdata ) )
    {
	foreach( var, tdata, next )
	{
	    sscanf( var, "%lu:%lu", &rcvd[i], &sent[i] );
	    totin += rcvd[i];
	    totout += sent[i];
	    if( rcvd[i] > max )
		max = rcvd[i];
	    if( sent[i] > max )
		max = sent[i];
	    i++;
	}
    }

    while( max > smax )
    {
	if( max > ( f * 5 ) )
	    smax = f * 10;
	if( max > ( f * 10 ) )
	    smax = f * 25;
	if( max > ( f * 25 ) )
	    smax = f * 50;
	f = f * 10;
    }

    char incom[32];

    sprintf( incom, "%s", live_translate( "status_inet.traffin" ) );
    char outcom[32];

    sprintf( outcom, "%s", live_translate( "status_inet.traffout" ) );
    char monthname[32];

    sprintf( monthname, "%s", live_translate( months[month - 1] ) );

    websWrite( stream,
	       "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n" );
    websWrite( stream, "<html>\n" );
    websWrite( stream, "<head>\n" );
    websWrite( stream,
	       "<meta http-equiv=\"Content-Type\" content=\"application/xhtml+xml; charset=%s\" />\n",
	       live_translate( "lang_charset.set" ) );
    websWrite( stream, "<title>dd-wrt traffic graph</title>\n" );

    websWrite( stream, "<script type=\"text/javascript\">\n" );
    websWrite( stream, "//<![CDATA[\n" );
    websWrite( stream, "function Show(label) {\n" );
    websWrite( stream,
	       "document.getElementById(\"label\").innerHTML = label;\n" );
    websWrite( stream, "}\n" );
    websWrite( stream, "//]]>\n" );
    websWrite( stream, "</script>\n" );

    websWrite( stream, "<style type=\"text/css\">\n\n" );
    websWrite( stream,
	       "#t-graph {position: relative; width: %upx; height: 300px;\n",
	       days * COL_WIDTH );
    websWrite( stream, "  margin: 1.1em 0 3.5em; padding: 0;\n" );
    websWrite( stream, "  border: 1px solid gray; list-style: none;\n" );
    websWrite( stream, "  font: 9px Tahoma, Arial, sans-serif;}\n" );
    websWrite( stream,
	       "#t-graph ul {margin: 0; padding: 0; list-style: none;}\n" );
    websWrite( stream,
	       "#t-graph li {position: absolute; bottom: 0; width: %dpx; z-index: 2;\n",
	       COL_WIDTH );
    websWrite( stream, "  margin: 0; padding: 0;\n" );
    websWrite( stream, "  text-align: center; list-style: none;}\n" );
    websWrite( stream,
	       "#t-graph li.day {height: 298px; padding-top: 2px; border-right: 1px dotted #C4C4C4; color: #AAA;}\n" );
    websWrite( stream,
	       "#t-graph li.day_sun {height: 298px; padding-top: 2px; border-right: 1px dotted #C4C4C4; color: #E00;}\n" );
    websWrite( stream,
	       "#t-graph li.bar {width: 4px; border: 1px solid; border-bottom: none; color: #000;}\n" );
    websWrite( stream, "#t-graph li.bar p {margin: 5px 0 0; padding: 0;}\n" );
    websWrite( stream, "#t-graph li.rcvd {left: 3px; background: #228B22;}\n" );	// set 
    // rcvd 
    // bar 
    // colour 
    // here 
    // (green)
    websWrite( stream, "#t-graph li.sent {left: 8px; background: #CD0000;}\n" );	// set 
    // sent 
    // bar 
    // colour 
    // here 
    // (red)

    for( i = 0; i < days - 1; i++ )
    {
	websWrite( stream, "#t-graph #d%d {left: %dpx;}\n", i + 1,
		   i * COL_WIDTH );
    }
    websWrite( stream, "#t-graph #d%u {left: %upx; border-right: none;}\n",
	       days, ( days - 1 ) * COL_WIDTH );

    websWrite( stream,
	       "#t-graph #ticks {width: %upx; height: 300px; z-index: 1;}\n",
	       days * COL_WIDTH );
    websWrite( stream,
	       "#t-graph #ticks .tick {position: relative; border-bottom: 1px solid #BBB; width: %upx;}\n",
	       days * COL_WIDTH );
    websWrite( stream,
	       "#t-graph #ticks .tick p {position: absolute; left: 100%%; top: -0.67em; margin: 0 0 0 0.5em;}\n" );
    websWrite( stream,
	       "#t-graph #label {width: 500px; bottom: -20px;  z-index: 1; font: 12px Tahoma, Arial, sans-serif; font-weight: bold;}\n" );
    websWrite( stream, "</style>\n" );
    websWrite( stream, "</head>\n\n" );
    websWrite( stream, "<body>\n" );
    websWrite( stream, "<ul id=\"t-graph\">\n" );

    for( i = 0; i < days; i++ )
    {
	websWrite( stream, "<li class=\"day%s\" id=\"d%d\" ",
		   ( wd % 7 ) == 6 ? "_sun" : "", i + 1 );
	wd++;
	websWrite( stream,
		   "onmouseover=\"Show(\'%s %d, %d (%s: %lu MB / %s: %lu MB)\')\" ",
		   monthname, i + 1, year, incom, rcvd[i], outcom, sent[i] );
	websWrite( stream,
		   "onmouseout=\"Show(\'%s %d (%s: %lu MB / %s: %lu MB)\')\"",
		   monthname, year, incom, totin, outcom, totout );
	websWrite( stream, ">%d\n", i + 1 );
	websWrite( stream, "<ul>\n" );
	websWrite( stream,
		   "<li class=\"rcvd bar\" style=\"height: %lupx;\"><p></p></li>\n",
		   rcvd[i] * 300 / smax );
	websWrite( stream,
		   "<li class=\"sent bar\" style=\"height: %lupx;\"><p></p></li>\n",
		   sent[i] * 300 / smax );
	websWrite( stream, "</ul>\n" );
	websWrite( stream, "</li>\n" );
    }

    websWrite( stream, "<li id=\"ticks\">\n" );
    for( i = 5; i; i-- )	// scale
    {
	websWrite( stream,
		   "<div class=\"tick\" style=\"height: 59px;\"><p>%d%sMB</p></div>\n",
		   smax * i / 5, ( smax > 10000 ) ? " " : "&nbsp;" );
    }
    websWrite( stream, "</li>\n\n" );

    websWrite( stream, "<li id=\"label\">\n" );
    websWrite( stream, "%s %d (%s: %lu MB / %s: %lu MB)\n", monthname, year,
	       incom, totin, outcom, totout );
    websWrite( stream, "</li>\n" );

    websWrite( stream, "</ul>\n\n" );
    websWrite( stream, "</body>\n" );
    websWrite( stream, "</html>\n" );

}

static void ttraff_backup( struct mime_handler *handler, char *url,
			   webs_t stream, char *query )
{
    system2( "echo TRAFF-DATA > /tmp/traffdata.bak" );
    system2( "nvram show | grep traff- >> /tmp/traffdata.bak" );
    do_file_attach( handler, "/tmp/traffdata.bak", stream, NULL,
		    "traffdata.bak" );
    unlink( "/tmp/traffdata.bak" );
}

static void do_apply_cgi( struct mime_handler *handler, char *url,
			  webs_t stream, char *q )
{
    char *path, *query;

    if( post == 1 )
    {
	query = post_buf;
	path = url;
    }
    else
    {
	query = url;
	path = strsep( &query, "?" ) ? : url;
	init_cgi( query );
    }

    if( !query )
	return;

    apply_cgi( stream, NULL, NULL, 0, url, path, query );
    init_cgi( NULL );
}

#ifdef HAVE_MADWIFI
extern int getdevicecount( void );
#endif

#ifdef HAVE_LANGUAGE
static void do_language( struct mime_handler *handler, char *path, webs_t stream, char *query )	// jimmy, 
									// https, 
									// 8/4/2003
{
    char *lang = getLanguageName(  );

    do_file( handler, lang, stream, NULL );
    free( lang );
    return;
}
#endif

extern int issuperchannel( void );

static char no_cache[] =
    "Cache-Control: no-cache\r\n" "Pragma: no-cache\r\n" "Expires: 0";

struct mime_handler mime_handlers[] = {
    // { "ezconfig.asp", "text/html", ezc_version, do_apply_ezconfig_post,
    // do_ezconfig_asp, do_auth },
#ifdef HAVE_SKYTRON
    {"setupindex*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
#endif
#ifdef HAVE_DDLAN
    {"Upgrade*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
    {"Management*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
    {"Services*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
    {"Hotspot*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
    {"Wireless*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
    {"WL_*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
    {"WPA*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
    {"Log*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
    {"Alive*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
    {"Diagnostics*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
    {"Wol*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
    {"Factory_Defaults*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
    {"config*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
#endif

    {"changepass.asp", "text/html", no_cache, NULL, do_ej, NULL, 1},
#ifdef HAVE_REGISTER
    {"register.asp", "text/html", no_cache, NULL, do_ej, do_auth_reg, 1},
#endif
    {"WL_FilterTable*", "text/html", no_cache, NULL, do_filtertable, do_auth,
     1},
    // #endif
    // #ifdef HAVE_MADWIFI
    {"Wireless_WDS*", "text/html", no_cache, NULL, do_wds, do_auth, 1},
    {"Wireless_Advanced*", "text/html", no_cache, NULL, do_wireless_adv, do_auth, 1},
    // #endif
    {"**.asp", "text/html", no_cache, NULL, do_ej, do_auth, 1},
    {"**.JPG", "image/jpeg", no_cache, NULL, do_file, NULL, 0},
    // {"style.css", "text/css", NULL, NULL, do_style, NULL},
    {"common.js", "text/javascript", NULL, NULL, do_file, NULL, 0},
#ifdef HAVE_LANGUAGE
    {"lang_pack/language.js", "text/javascript", NULL, NULL, do_language,
     NULL, 0},
#endif
    {"SysInfo.htm*", "text/plain", no_cache, NULL, do_ej, do_auth, 1},
#ifdef HAVE_SKYTRON
    {"Info.htm*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
    {"Info.live.htm", "text/html", no_cache, NULL, do_ej, do_auth, 1},
    {"**.htm", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
    {"**.html", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
#else
    {"Info.htm*", "text/html", no_cache, NULL, do_ej, do_cauth, 1},
    {"Info.live.htm", "text/html", no_cache, NULL, do_ej, do_cauth, 1},
    {"**.htm", "text/html", no_cache, NULL, do_ej, NULL, 1},
    {"**.html", "text/html", no_cache, NULL, do_ej, NULL, 1},

#endif
#ifdef HAVE_ROUTERSTYLE
    {"style/blue/style.css", "text/css", NULL, NULL, do_stylecss, NULL, 1},
    {"style/cyan/style.css", "text/css", NULL, NULL, do_stylecss, NULL, 1},
    {"style/elegant/style.css", "text/css", NULL, NULL, do_stylecss, NULL, 1},
    {"style/green/style.css", "text/css", NULL, NULL, do_stylecss, NULL, 1},
    {"style/orange/style.css", "text/css", NULL, NULL, do_stylecss, NULL, 1},
    {"style/red/style.css", "text/css", NULL, NULL, do_stylecss, NULL, 1},
    {"style/yellow/style.css", "text/css", NULL, NULL, do_stylecss, NULL, 1},
    {"style/blue/style_ie.css", "text/css", NULL, NULL, do_stylecss_ie, NULL,
     1},
    {"style/cyan/style_ie.css", "text/css", NULL, NULL, do_stylecss_ie, NULL,
     1},
    {"style/elegant/style_ie.css", "text/css", NULL, NULL, do_stylecss_ie,
     NULL, 1},
    {"style/green/style_ie.css", "text/css", NULL, NULL, do_stylecss_ie, NULL,
     1},
    {"style/orange/style_ie.css", "text/css", NULL, NULL, do_stylecss_ie,
     NULL, 1},
    {"style/red/style_ie.css", "text/css", NULL, NULL, do_stylecss_ie, NULL,
     1},
    {"style/yellow/style_ie.css", "text/css", NULL, NULL, do_stylecss_ie,
     NULL, 1},
#endif
#ifdef HAVE_REGISTER
    {"style/logo.png", "image/png", NULL, NULL, do_trial_logo, NULL, 0},
#endif
    {"**.css", "text/css", NULL, NULL, do_file, NULL, 0},
    {"**.svg", "image/svg+xml", NULL, NULL, do_file, NULL, 0},
    {"**.gif", "image/gif", NULL, NULL, do_file, NULL, 0},
    {"**.png", "image/png", NULL, NULL, do_file, NULL, 0},
    {"**.jpg", "image/jpeg", NULL, NULL, do_file, NULL, 0},
    {"**.ico", "image/x-icon", NULL, NULL, do_file, NULL, 0},
    {"**.js", "text/javascript", NULL, NULL, do_file, NULL, 0},
    {"**.swf", "application/x-shockwave-flash", NULL, NULL, do_file, NULL, 0},
    {"**.pdf", "application/pdf", NULL, NULL, do_file, NULL, 0},
#ifdef HAVE_SKYTRON
    {"applyuser.cgi*", "text/html", no_cache, do_apply_post, do_apply_cgi,
     do_auth2, 1},
#elif HAVE_DDLAN
    {"applyuser.cgi*", "text/html", no_cache, do_apply_post, do_apply_cgi,
     NULL, 1},
#else
    {"applyuser.cgi*", "text/html", no_cache, do_apply_post, do_apply_cgi,
     do_auth, 1},
#endif
    {"fetchif.cgi*", "text/html", no_cache, NULL, do_fetchif, do_auth, 1},
#ifdef HAVE_DDLAN
    {"apply.cgi*", "text/html", no_cache, do_apply_post, do_apply_cgi, NULL,
     1},
    {"upgrade.cgi*", "text/html", no_cache, do_upgrade_post, do_upgrade_cgi,
     NULL, 1},
#else
    {"apply.cgi*", "text/html", no_cache, do_apply_post, do_apply_cgi,
     do_auth, 1},
    {"upgrade.cgi*", "text/html", no_cache, do_upgrade_post, do_upgrade_cgi,
     do_auth, 1},
#endif
    // {"Gozila.cgi*", "text/html", no_cache, NULL, do_setup_wizard,
    // do_auth}, // for setup wizard
    /*
     * { "**.cfg", "application/octet-stream", no_cache, NULL, do_backup,
     * do_auth }, 
     */
#ifdef HAVE_DDLAN
    {"restore.cgi**", "text/html", no_cache, do_upgrade_post, do_upgrade_cgi,
     NULL, 1},
#else
    {"restore.cgi**", "text/html", no_cache, do_upgrade_post, do_upgrade_cgi,
     do_auth, 1},
#endif
    {"test.bin**", "application/octet-stream", no_cache, NULL, do_file,
     do_auth, 0},

#ifdef HAVE_DDLAN
    {"nvrambak.bin*", "application/octet-stream", no_cache, NULL, nv_file_out,
     do_auth2, 0},
    {"nvrambak**.bin*", "application/octet-stream", no_cache, NULL,
     nv_file_out,
     do_auth2, 0},
    {"nvram.cgi*", "text/html", no_cache, nv_file_in, sr_config_cgi, NULL, 1},
#else
    {"nvrambak.bin*", "application/octet-stream", no_cache, NULL, nv_file_out,
     do_auth, 0},
    {"nvrambak**.bin*", "application/octet-stream", no_cache, NULL,
     nv_file_out,
     do_auth, 0},
    {"nvram.cgi*", "text/html", no_cache, nv_file_in, sr_config_cgi, do_auth,
     1},
#endif
#if !defined(HAVE_X86) && !defined(HAVE_MAGICBOX)
    {"backup/cfe.bin", "application/octet-stream", no_cache, NULL,
     do_cfebackup,
     do_auth, 0},
#endif
    {"ttgraph.cgi*", "text/html", no_cache, NULL, do_ttgraph, do_auth, 1},
    {"traffdata.bak*", "text/html", no_cache, NULL, ttraff_backup,
     do_auth, 0},
    {"tadmin.cgi*", "text/html", no_cache, td_file_in, td_config_cgi,
     NULL, 1},
    // for ddm
    {NULL, NULL, NULL, NULL, NULL, NULL, 0}
};

/*
 * Format: type = SET : " " => "&nbsp;" , ":" => "&semi;" type = GET :
 * "&nbsp;" => " " , "&semi;" => ":" Example: name1 = test 123:abc
 * filter_name("name1", new_name, SET); new_name="test&nbsp;123&semi;abc"
 * name2 = test&nbsp;123&semi;abc filter_name("name2", new_name, GET);
 * new_name="test 123:abc" 
 */
int httpd_filter_name( char *old_name, char *new_name, size_t size, int type )
{
    int i, j, match;

    cprintf( "httpd_filter_name\n" );

    struct pattern
    {
	char ch;
	char *string;
    };

    struct pattern patterns[] = {
	{' ', "&nbsp;"},
	{':', "&semi;"},
    };

    struct pattern *v;

    strcpy( new_name, "" );

    switch ( type )
    {
	case SET:
	    for( i = 0; *( old_name + i ); i++ )
	    {
		match = 0;
		for( v = patterns; v < &patterns[STRUCT_LEN( patterns )];
		     v++ )
		{
		    if( *( old_name + i ) == v->ch )
		    {
			if( strlen( new_name ) + strlen( v->string ) > size )
			{	// avoid overflow
			    cprintf( "%s(): overflow\n", __FUNCTION__ );
			    new_name[strlen( new_name )] = '\0';
			    return 1;
			}
			sprintf( new_name + strlen( new_name ), "%s",
				 v->string );
			match = 1;
			break;
		    }
		}
		if( !match )
		{
		    if( strlen( new_name ) + 1 > size )
		    {
			cprintf( "%s(): overflow\n", __FUNCTION__ );	// avoid 
			// overflow
			new_name[strlen( new_name )] = '\0';
			return 1;
		    }
		    sprintf( new_name + strlen( new_name ), "%c",
			     *( old_name + i ) );
		}
	    }

	    break;
	case GET:
	    for( i = 0, j = 0; *( old_name + j ); j++ )
	    {
		match = 0;
		for( v = patterns; v < &patterns[STRUCT_LEN( patterns )];
		     v++ )
		{
		    if( !memcmp
			( old_name + j, v->string, strlen( v->string ) ) )
		    {
			*( new_name + i ) = v->ch;
			j = j + strlen( v->string ) - 1;
			match = 1;
			break;
		    }
		}
		if( !match )
		    *( new_name + i ) = *( old_name + j );

		i++;
	    }
	    *( new_name + i ) = '\0';
	    break;
	default:
	    cprintf( "%s():Invalid type!\n", __FUNCTION__ );
	    break;
    }
    // cprintf("%s():new_name=[%s]\n", __FUNCTION__, new_name);

    return 1;
}

#if 0
struct ej_handler ej_handlers[] = {
    /*
     * for all 
     */
    {"nvram_get", ej_nvram_get},
    /*
     * { "nvram_get_len", ej_nvram_get_len }, 
     */
    {"nvram_selget", ej_nvram_selget},
    {"nvram_match", ej_nvram_match},
    {"nvram_invmatch", ej_nvram_invmatch},
    {"nvram_selmatch", ej_nvram_selmatch},
    {"nvram_else_selmatch", ej_nvram_else_selmatch},
    {"nvram_else_match", ej_nvram_else_match},
    {"tran", ej_tran},
    {"nvram_list", ej_nvram_list},
    {"nvram_mac_get", ej_nvram_mac_get},
    {"nvram_gozila_get", ej_nvram_gozila_get},
    {"nvram_status_get", ej_nvram_status_get},
    {"nvram_real_get", ej_nvram_real_get},
    {"webs_get", ej_webs_get},
    {"get_firmware_version", ej_get_firmware_version},
    {"get_firmware_title", ej_get_firmware_title},
    {"get_firmware_svnrev", ej_get_firmware_svnrev},
    {"get_model_name", ej_get_model_name},
    {"showad", ej_showad},
    {"get_single_ip", ej_get_single_ip},
    {"get_single_mac", ej_get_single_mac},
    {"prefix_ip_get", ej_prefix_ip_get},
    {"no_cache", ej_no_cache},
    // {"scroll", ej_scroll},
    {"get_dns_ip", ej_get_dns_ip},
    {"onload", ej_onload},
    {"get_web_page_name", ej_get_web_page_name},
    {"show_logo", ej_show_logo},
    {"get_clone_mac", ej_get_clone_mac},
    /*
     * for index 
     */
    {"show_index_setting", ej_show_index_setting},
    {"compile_date", ej_compile_date},
    {"compile_time", ej_compile_time},
    {"get_wl_max_channel", ej_get_wl_max_channel},
    {"get_wl_domain", ej_get_wl_domain},
    /*
     * for status 
     */
    {"show_status", ej_show_status},
    {"show_status_setting", ej_show_status_setting},
    {"localtime", ej_localtime},
    {"dhcp_remaining_time", ej_dhcp_remaining_time},
    {"show_wan_domain", ej_show_wan_domain},
    {"show_wl_mac", ej_show_wl_mac},
    {"dumpip_conntrack", ej_dumpip_conntrack},
    {"ip_conntrack_table", ej_ip_conntrack_table},
    {"gethostnamebyip", ej_gethostnamebyip},
    /*
     * for dhcp 
     */
    {"dumpleases", ej_dumpleases},
    /*
     * for ddm 
     */
    /*
     * for log 
     */
    {"dumplog", ej_dumplog},
#ifdef HAVE_SPUTNIK_APD
    {"sputnik_apd_status", ej_sputnik_apd_status},
    // {"show_sputnik", ej_show_sputnik},
#endif
    // {"show_openvpn", ej_show_openvpn},
    {"show_openvpn_status", ej_show_openvpn_status},
    /*
     * for filter 
     */
    {"filter_init", ej_filter_init},
    {"filter_summary_show", ej_filter_summary_show},
    {"filter_ip_get", ej_filter_ip_get},
    {"filter_port_get", ej_filter_port_get},
    {"filter_dport_get", ej_filter_dport_get},
    {"filter_mac_get", ej_filter_mac_get},
    {"filter_policy_select", ej_filter_policy_select},
    {"filter_policy_get", ej_filter_policy_get},
    {"filter_tod_get", ej_filter_tod_get},
    {"filter_web_get", ej_filter_web_get},
    {"filter_port_services_get", ej_filter_port_services_get},
    /*
     * for forward 
     */
    // {"port_forward_table", ej_port_forward_table},
    // {"port_forward_spec", ej_port_forward_spec},
    // changed by steve
    // {"forward_upnp", ej_forward_upnp},
    // end changed by steve
    /*
     * for route 
     */
    {"static_route_table", ej_static_route_table},
    {"static_route_setting", ej_static_route_setting},
    {"dump_route_table", ej_dump_route_table},
    /*
     * for ddns 
     */
    {"show_ddns_status", ej_show_ddns_status},
    {"show_usb_diskinfo", ej_show_usb_diskinfo},
    // {"show_ddns_ip", ej_show_ddns_ip},
    /*
     * for wireless 
     */
    {"wireless_active_table", ej_wireless_active_table},
    {"wireless_filter_table", ej_wireless_filter_table},
    {"show_wl_wep_setting", ej_show_wl_wep_setting},
    {"get_wep_value", ej_get_wep_value},
    {"get_wl_active_mac", ej_get_wl_active_mac},
    {"get_wl_value", ej_get_wl_value},

    // {"show_wpa_setting", ej_show_wpa_setting},
    /*
     * for test 
     */
    {"wl_packet_get", ej_wl_packet_get},
    {"wl_ioctl", ej_wl_ioctl},
    {"dump_ping_log", ej_dump_ping_log},
    // {"dump_traceroute_log", ej_dump_traceroute_log},
    // {"get_http_method", ej_get_http_method},
    /*
     * { "get_backup_name", ej_get_backup_name }, 
     */
    /*
     * { "per_port_option", ej_per_port_option}, 
     */
    {"get_http_prefix", ej_get_http_prefix},
    {"dump_site_survey", ej_dump_site_survey},
    // {"show_meminfo", ej_show_meminfo},
    {"get_mtu", ej_get_mtu},
    {"get_url", ej_get_url},
    /*
     * Sveasoft additions 
     */
    {"get_wdsp2p", ej_get_wdsp2p},
    {"active_wireless", ej_active_wireless},
#ifdef HAVE_SKYTRON
    {"active_wireless2", ej_active_wireless2},
#endif
    {"active_wds", ej_active_wds},
    {"get_wds_mac", ej_get_wds_mac},
    {"get_wds_ip", ej_get_wds_ip},
    {"get_wds_netmask", ej_get_wds_netmask},
    {"get_wds_gw", ej_get_wds_gw},
    {"get_br1_ip", ej_get_br1_ip},
    {"get_br1_netmask", ej_get_br1_netmask},
    {"get_curchannel", ej_get_curchannel},
    {"get_currate", ej_get_currate},
    {"get_uptime", ej_get_uptime},
    {"get_wan_uptime", ej_get_wan_uptime},
    // {"get_services_options", ej_get_services_options},
    {"get_clone_wmac", ej_get_clone_wmac},
    {"show_modules", ej_show_modules},
    {"show_styles", ej_show_styles},
#ifdef HAVE_LANGUAGE
    {"show_languages", ej_show_languages},
#endif
    {"show_forward", ej_show_forward},
    {"show_forward_spec", ej_show_forward_spec},
    {"show_triggering", ej_show_triggering},
    {"nvram_selected", ej_nvram_selected},
    {"nvram_selected_js", ej_nvram_selected_js},
    {"nvram_checked", ej_nvram_checked},
    {"nvram_checked_js", ej_nvram_checked_js},
    {"get_qossvcs", ej_get_qossvcs},
    {"get_qosips", ej_get_qosips},
    {"get_qosmacs", ej_get_qosmacs},
    // { "if_config_table",ej_if_config_table},
    {"wme_match_op", ej_wme_match_op},
    // { "show_advanced_qos", ej_show_advanced_qos },
#ifdef FBNFW
    {"list_fbn", ej_list_fbn},
#endif
    {"show_control", ej_show_control},
    {"show_paypal", ej_show_paypal},
    {"show_default_level", ej_show_default_level},
    {"show_staticleases", ej_show_staticleases},
    {"show_security", ej_show_security},
    {"show_dhcpd_settings", ej_show_dhcpd_settings},
    {"show_wds_subnet", ej_show_wds_subnet},
    {"show_infopage", ej_show_infopage},
    {"show_connectiontype", ej_show_connectiontype},
    {"show_routing", ej_show_routing},
#ifdef HAVE_MSSID
    {"show_wireless", ej_show_wireless},
#endif
#ifdef HAVE_RADLOCAL
    {"show_iradius_check", ej_show_iradius_check},
    {"show_iradius", ej_show_iradius},
#endif

#ifdef HAVE_CHILLILOCAL
    {"show_userlist", ej_show_userlist},
#endif
    {"show_cpuinfo", ej_show_cpuinfo},
    {"get_clkfreq", ej_get_clkfreq},
    {"dumpmeminfo", ej_dumpmeminfo},

    /*
     * Added by Botho 10.May.06 
     */
    {"show_wan_to_switch", ej_show_wan_to_switch},
    {"statfs", ej_statfs},

    /*
     * lonewolf additions 
     */
    {"port_vlan_table", ej_port_vlan_table},
    /*
     * end lonewolf additions 
     */
#ifdef HAVE_UPNP
    {"tf_upnp", ej_tf_upnp},
#endif
    // {"charset", ej_charset},
    {"do_menu", ej_do_menu},	// Eko
    {"do_pagehead", ej_do_pagehead},	// Eko
    {"do_hpagehead", ej_do_hpagehead},	// Eko
    {"show_timeoptions", ej_show_timeoptions},	// Eko
    {"show_wanipinfo", ej_show_wanipinfo},	// Eko
#ifdef HAVE_OVERCLOCKING
    {"show_clocks", ej_show_clocks},
#endif
    {"make_time_list", ej_make_time_list},	// Eko
    {"getrebootflags", ej_getrebootflags},
    {"getwirelessmode", ej_getwirelessmode},
    {"getwirelessnetmode", ej_getwirelessnetmode},
    {"getwirelessssid", ej_getwirelessssid},
    {"radio_on", ej_radio_on},   
    {"get_radio_state", ej_get_radio_state},
    {"dumparptable", ej_dumparptable},
#ifdef HAVE_WIVIZ
    {"dump_wiviz_data", ej_dump_wiviz_data},	// Eko, for testing only
#endif
#ifdef HAVE_EOP_TUNNEL
    {"show_eop_tunnels", ej_show_eop_tunnels},
#endif
#ifdef HAVE_MSSID
    {"getwirelessstatus", ej_getwirelessstatus},
    {"getencryptionstatus", ej_getencryptionstatus},
    {"get_txpower", ej_get_txpower},
#endif
#ifdef HAVE_CPUTEMP
    {"get_cputemp", ej_get_cputemp},
    {"show_cpu_temperature", ej_show_cpu_temperature},
#endif
#ifdef HAVE_VOLT
    {"get_voltage", ej_get_voltage},
    {"show_voltage", ej_show_voltage},
#endif
#if defined(HAVE_REGISTER) || defined(HAVE_SUPERCHANNEL)
    {"getregcode", ej_getregcode},
#endif
#ifdef HAVE_RSTATS
    {"bandwidth", ej_bandwidth},
#endif
    {"show_bandwidth", ej_show_bandwidth},
    {"get_totaltraff", ej_get_totaltraff},
#ifdef HAVE_PORTSETUP
    {"portsetup", ej_portsetup},
#endif
    {"show_macfilter", ej_show_macfilter},
    {"list_mac_layers", ej_list_mac_layers},
#ifdef HAVE_VLANTAGGING
    {"show_vlantagging", ej_show_vlantagging},
    {"show_bridgenames", ej_show_bridgenames},
    {"show_bridgeifnames", ej_show_bridgeifnames},
    {"show_bridgetable", ej_show_bridgetable},
    {"show_mdhcp", ej_show_mdhcp},
#endif
#ifdef HAVE_BONDING
    {"show_bondings", ej_show_bondings},
#endif
#ifdef HAVE_MADWIFI
    {"show_wifiselect", ej_show_wifiselect},
#endif
#ifdef HAVE_PPPOESERVER
    {"show_chaps", ej_show_chaps},
#endif
#ifdef HAVE_OLSRD
    {"show_olsrd", ej_show_olsrd},
#endif
    {"show_routeif", ej_show_routeif},
#ifdef HAVE_MILKFISH
    {"exec_milkfish_service", ej_exec_milkfish_service},
    {"exec_milkfish_phonebook", ej_exec_milkfish_phonebook},
    {"exec_show_subscribers", ej_exec_show_subscribers},
    {"exec_show_aliases", ej_exec_show_aliases},
    {"exec_show_registrations", ej_exec_show_registrations},
#endif
#ifdef HAVE_CHILLI
    {"show_chilliif", ej_show_chilliif},
#endif
#ifdef HAVE_RFLOW
    {"show_rflowif", ej_show_rflowif},
#endif
    {"startswith", ej_startswith},
    {"ifdef", ej_ifdef},
    {"ifndef", ej_ifndef},
    {"show_countrylist", ej_show_countrylist},
#ifdef HAVE_MADWIFI
    {"show_acktiming", ej_show_acktiming},
    {"update_acktiming", ej_update_acktiming},
#endif
    {"showbridgesettings", ej_showbridgesettings},
    {NULL, NULL}
};

#endif
