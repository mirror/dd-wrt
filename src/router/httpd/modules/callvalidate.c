/*
 * DD-WRT servicemanager.c
 *
 * Copyright (C) 2005 - 2006 Sebastian Gottschall <sebastian.gottschall@newmedia-net.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License
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
#include <stdio.h>
// #include <shutils.h>

#ifdef HAVE_ADM5120
#define SERVICE_MODULE "/lib/validate.so"
#define VISSERVICE_MODULE "/lib/visuals.so"
#else
#define SERVICE_MODULE "/usr/lib/validate.so"
#define VISSERVICE_MODULE "/usr/lib/visuals.so"
#endif
// #define SERVICE_MODULE "/tmp/validate.so"
// #define VISSERVICE_MODULE "/tmp/visuals.so"

// #define SERVICE_MODULE "/tmp/validate.so"
// #define cprintf(fmt, args...)

#ifndef cprintf
#define cprintf(fmt, args...) do { \
	FILE *fp = fopen("/dev/console", "w"); \
	if (fp) { \
		fprintf(fp,"%s (%d):%s ",__FILE__,__LINE__,__func__); \
		fprintf(fp, fmt, ## args); \
		fclose(fp); \
	} \
} while (0)
#endif

int websWrite( webs_t wp, char *fmt, ... );
char *websGetVar( webs_t wp, char *var, char *d )
{
    return get_cgi( var ) ? : d;
}

char *GOZILA_GET( webs_t wp, char *name )
{
    return nvram_match( "gozila_action", "1" ) ? websGetVar( wp, name,
							     NULL ) :
	nvram_safe_get( name );
}

void *load_visual_service( char *name )
{
    cprintf( "load service %s\n", name );
    void *handle = dlopen( VISSERVICE_MODULE, RTLD_LAZY );

    cprintf( "done()\n" );
    if( handle == NULL && name != NULL )
    {
	cprintf( "not found, try to load alternate\n" );
	char dl[64];

	sprintf( dl, "/lib/%s_visual.so", name );
	cprintf( "try to load %s\n", dl );
	handle = dlopen( dl, RTLD_LAZY );
	if( handle == NULL )
	{
	    fprintf( stderr, "cannot load %s\n", dl );
	    return NULL;
	}
    }
    cprintf( "found it, returning handle\n" );
    return handle;
}

void *load_service( char *name )
{
    cprintf( "load service %s\n", name );
    void *handle = dlopen( SERVICE_MODULE, RTLD_LAZY );

    cprintf( "done()\n" );
    if( handle == NULL && name != NULL )
    {
	cprintf( "not found, try to load alternate\n" );
	char dl[64];

	sprintf( dl, "/lib/%s_validate.so", name );
	cprintf( "try to load %s\n", dl );
	handle = dlopen( dl, RTLD_LAZY );
	if( handle == NULL )
	{
	    fprintf( stderr, "cannot load %s\n", dl );
	    return NULL;
	}
    }
    cprintf( "found it, returning handle\n" );
    return handle;
}
extern websRomPageIndexType websRomPageIndex[];
struct wl_client_mac wl_client_macs[MAX_LEASES];

// extern struct wl_client_mac *wl_client_macs;

extern char *live_translate( char *tran );

static int initWeb( void *handle )
{
    struct Webenvironment *env;
    env =
	( struct Webenvironment * )malloc( sizeof( struct Webenvironment ) );
    void ( *init ) ( struct Webenvironment * env );

    init =
	( void ( * )( struct Webenvironment * env ) )dlsym( handle,
							    "initWeb" );
    if( !init )
    {
	fprintf( stderr, "error, initWeb not found\n" );
	return -1;
    }
    env->PwebsGetVar = websGetVar;
    env->PwebsWrite = websWrite;
    cprintf( "httpd_filter_name %p:%d\n", httpd_filter_name,
	     sizeof( struct Webenvironment ) );
    env->Phttpd_filter_name = httpd_filter_name;
    env->Pwl_client_macs = wl_client_macs;
    env->Pdo_ej_buffer = do_ej_buffer;
    env->Pdo_ej = do_ej;
#ifdef HAVE_HTTPS
    env->Pdo_ssl = do_ssl;
#endif
    env->PejArgs = ejArgs;
    env->PgetWebsFile = getWebsFile;
    env->Pwfputc = wfputc;
    env->Pwfputs = wfputs;
    env->Pwfflush = wfflush;
    env->PwebsRomPageIndex = websRomPageIndex;
    env->Plive_translate = live_translate;
    env->PGOZILA_GET = GOZILA_GET;
    cprintf( "call initWeb\n" );
    init( env );
    free( env );
    return 0;
}

void start_gozila( char *name, webs_t wp )
{
    // lcdmessaged("Starting Service",name);
    cprintf( "start_gozila %s\n", name );
    char service[64];
    void *handle = load_service( name );

    if( handle == NULL )
    {
	return;
    }
    if( initWeb( handle ) != 0 )
    {
	return;
    }

    int ( *fptr ) ( webs_t wp );

    sprintf( service, "%s", name );
    cprintf( "resolving %s\n", service );
    fptr = ( int ( * )( webs_t wp ) )dlsym( handle, service );
    if( fptr )
	( *fptr ) ( wp );
    else
	fprintf( stderr, "function %s not found \n", service );
    dlclose( handle );
    cprintf( "start_sevice done()\n" );
}

int start_validator( char *name, webs_t wp, char *value, struct variable *v )
{
    // lcdmessaged("Starting Service",name);
    cprintf( "start_validator %s\n", name );
    char service[64];
    void *handle = load_service( name );

    if( handle == NULL )
    {
	return FALSE;
    }

    if( initWeb( handle ) != 0 )
    {
	return FALSE;
    }
    int ret = FALSE;

    int ( *fptr ) ( webs_t wp, char *value, struct variable * v );

    sprintf( service, "%s", name );
    cprintf( "resolving %s\n", service );
    fptr =
	( int ( * )( webs_t wp, char *value, struct variable * v ) )
	dlsym( handle, service );
    if( fptr )
	ret = ( *fptr ) ( wp, value, v );
    else
	fprintf( stderr, "function %s not found \n", service );
    dlclose( handle );
    cprintf( "start_sevice done()\n" );
    return ret;
}

void *start_validator_nofree( char *name, void *handle, webs_t wp,
			      char *value, struct variable *v )
{
    // lcdmessaged("Starting Service",name);
    cprintf( "start_service_nofree %s\n", name );
    char service[64];
    int nohandle = 0;

    if( !handle )
    {
	handle = load_service( name );
	nohandle = 1;
    }
    if( handle == NULL )
    {
	return NULL;
    }
    void ( *fptr ) ( webs_t wp, char *value, struct variable * v );

    sprintf( service, "%s", name );
    if( nohandle )
    {
	if( initWeb( handle ) != 0 )
	{
	    return handle;
	}
    }
    cprintf( "resolving %s\n", service );
    fptr =
	( void ( * )( webs_t wp, char *value, struct variable * v ) )
	dlsym( handle, service );
    cprintf( "found. pointer is %p\n", fptr );
    if( fptr )
	( *fptr ) ( wp, value, v );
    else
	fprintf( stderr, "function %s not found \n", service );
    cprintf( "start_sevice_nofree done()\n" );
    return handle;
}

void *call_ej( char *name, void *handle, webs_t wp, int argc, char_t ** argv )
{
    // fprintf (stderr,"call_ej %s\n", name);
    char service[64];
    int nohandle = 0;

    if( !handle )
    {
	cprintf( "load visual_service\n" );
	handle = load_visual_service( name );
	nohandle = 1;
    }
    if( handle == NULL )
    {
	cprintf( "handle null\n" );
	return NULL;
    }
    cprintf( "pointer init\n" );
    void ( *fptr ) ( webs_t wp, int argc, char_t ** argv );

    sprintf( service, "ej_%s", name );
    if( nohandle )
    {
	cprintf( "init web\n" );
	if( initWeb( handle ) != 0 )
	{
	    return handle;
	}
    }
    cprintf( "resolving %s\n", service );
    fptr = ( void ( * )( webs_t wp, int argc, char_t ** argv ) )dlsym( handle,
								       service );
    cprintf( "found. pointer is %p\n", fptr );
    if( fptr )
	( *fptr ) ( wp, argc, argv );
    else
	fprintf( stderr, "function %s not found \n", service );
    cprintf( "start_sevice_nofree done()\n" );
    return handle;

}
