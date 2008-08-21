/*
 * Support for router statistics gathering
 *
 * Copyright 2001-2003, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: stats.c,v 1.2 2005/11/11 09:26:18 seg Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#include <bcmnvram.h>
#include <shutils.h>

extern int http_post( const char *server, char *buf, size_t count );

#define BUFSPACE 50000

int http_stats( const char *url )
{
    char *buf, *s;
    char **cur;
    char *secrets[] = { "os_server", "stats_server", "http_passwd", NULL };
    char *files[] =
	{ "/proc/version", "/proc/meminfo", "/proc/cpuinfo",
"/proc/interrupts",
	"/proc/net/dev", "/proc/net/pppoe", "/proc/net/snmp", NULL
    };
    char *contents;

    if( !( buf = malloc( BUFSPACE ) ) )
	return errno;

    /*
     * Get NVRAM variables 
     */
    nvram_getall( buf, NVRAM_SPACE );
    for( s = buf; *s; s++ )
    {
	for( cur = secrets; *cur; cur++ )
	{
	    if( !strncmp( s, *cur, strlen( *cur ) ) )
	    {
		s += strlen( *cur ) + 1;
		while( *s )
		    *s++ = '*';
		break;
	    }
	}
	*( s += strlen( s ) ) = '&';
    }

    /*
     * Dump interesting /proc entries 
     */
    for( cur = files; *cur; cur++ )
    {
	if( ( contents = file2str( *cur ) ) )
	{
	    s += snprintf( s, buf + BUFSPACE - s, "%s=%s&", *cur, contents );
	    free( contents );
	}
    }

    /*
     * Report uptime 
     */
    s += snprintf( s, buf + BUFSPACE - s, "uptime=%lu&",
		   ( unsigned long )time( NULL ) );

    /*
     * Save 
     */
    s += snprintf( s, buf + BUFSPACE - s, "action=save" );
    buf[BUFSPACE - 1] = '\0';

    /*
     * Post to server 
     */
    http_post( url ? : nvram_safe_get( "stats_server" ), buf, BUFSPACE );

    free( buf );
    return 0;
}
