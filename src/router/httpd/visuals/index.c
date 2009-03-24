#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <broadcom.h>
#include <cymac.h>

void ej_show_index_setting( webs_t wp, int argc, char_t ** argv )
{
    char *type;

    type = GOZILA_GET( wp, "wan_proto" );
    if (type==NULL)
	type=nvram_safe_get("wan_proto");
    if( !strcmp( type, "static" ) )
	do_ej(NULL, "index_static.asp", wp, NULL );
#ifdef HAVE_PPPOE
    else if( !strcmp( type, "pppoe" ) )
	do_ej(NULL, "index_pppoe.asp", wp, NULL );
#endif
#ifdef HAVE_PPTP
    else if( !strcmp( type, "pptp" ) )
	do_ej(NULL, "index_pptp.asp", wp, NULL );
#endif
#ifdef HAVE_L2TP
    else if( !strcmp( type, "l2tp" ) )
	do_ej(NULL, "index_l2tp.asp", wp, NULL );
#endif
#ifdef HAVE_HEARTBEAT
    else if( !strcmp( type, "heartbeat" ) )
	do_ej(NULL, "index_heartbeat.asp", wp, NULL );
#endif
#ifdef HAVE_3G
    else if( !strcmp( type, "3g" ) )
	do_ej(NULL, "index_3g.asp", wp, NULL );
#endif
}

void ej_get_wl_max_channel( webs_t wp, int argc, char_t ** argv )
{

    websWrite( wp, "%s", WL_MAX_CHANNEL );
}

void ej_get_wl_domain( webs_t wp, int argc, char_t ** argv )
{

#if COUNTRY == EUROPE
    websWrite( wp, "ETSI" );
#elif COUNTRY == JAPAN
    websWrite( wp, "JP" );
#else
    websWrite( wp, "US" );
#endif
}

void ej_get_clone_mac( webs_t wp, int argc, char_t ** argv )
{
    char *c;
    int mac, which;
    int dofree = 0;

#ifdef FASTWEB
    ejArgs( argc, argv, "%d", &which );
#else
    if( ejArgs( argc, argv, "%d", &which ) < 1 )
    {
	websError( wp, 400, "Insufficient args\n" );
    }
#endif

    if( nvram_match( "clone_wan_mac", "1" ) )
	c = nvram_safe_get( "http_client_mac" );
    else
    {
	if( nvram_match( "def_hwaddr", "00:00:00:00:00:00" ) )
	{
	    if( nvram_match( "port_swap", "1" ) )
		c = strdup( nvram_safe_get( "et1macaddr" ) );
	    else
		c = strdup( nvram_safe_get( "et0macaddr" ) );
	    if( c )
	    {
		MAC_ADD( c );
		dofree = 1;
	    }
	}
	else
	    c = nvram_safe_get( "def_hwaddr" );
    }

    if( c )
    {
	mac = get_single_mac( c, which );
	websWrite( wp, "%02X", mac );
	if( dofree )
	    free( c );
    }
    else
	websWrite( wp, "00" );
}

void macclone_onload( webs_t wp, char *arg )
{

    if( nvram_match( "clone_wan_mac", "1" ) )
	websWrite( wp, arg );

    return;
}
