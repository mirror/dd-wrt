/*
 * beep.c
 *
 * Copyright (C) 2008 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <utils.h>
#include <wlutils.h>
#include <bcmnvram.h>

static char *filter[] = { "lan_ifnames",
    "lan_ifname",
    "wan_ifnames",
    "wan_ifname",
    "et0macaddr",
    "et1macaddr",
    "il0macaddr",
    "boardnum",
    "boardtype",
    "boardrev",
    "melco_id",
    "product_name",
    "phyid_num",
    "cardbus",
    "CFEver",
    "clkfreq",
    "boardflags",
    "boardflags2",
    "sromrev",
    "sdram_config",
    "sdram_init",
    "sdram_refresh",
    "sdram_ncdl",
    "boot_wait",
    "wait_time",
    "et0phyaddr",
    "et0mdcport",
    "vlan0ports",
    "vlan1ports",
    "vlan2ports",
    "vlan0hwname",
    "vlan1hwname",
    "vlan2hwname",
    "wl_use_coregpio",
    "wl0gpio0",
    "wl0gpio1",
    "wl0gpio2",
    "wl0gpio3",
    "wl0gpio4",
    "reset_gpio",
    "af_hash",
    "wan_ifname",
    "lan_ifname",
    "lan_ifnames",
    "wan_ifnames",
    "wan_default",
    NULL
};
extern struct nvram_tuple srouter_defaults[];

static int isCritical( char *name )
{
    int a = 0;

    while( filter[a] != NULL )
    {
	if( !strcmp( name, filter[a++] ) )
	{
	    return 1;
	}
    }
    return 0;
}

void start_defaults( void )
{
    fprintf( stderr, "restore nvram to defaults\n" );
    char *buf = ( char * )malloc( NVRAM_SPACE );
    int i;
    struct nvram_tuple *t;

    nvram_getall( buf, NVRAM_SPACE );
    char *p = buf;

    //clean old values
    while( strlen( p ) != 0 )
    {
	int len = strlen( p );

	for( i = 0; i < len; i++ )
	    if( p[i] == '=' )
		p[i] = 0;
	char *name = p;

	if( !isCritical( name ) )
	    nvram_unset( name );
	p += len + 1;
    }
    for( t = srouter_defaults; t->name; t++ )
    {
	nvram_set( t->name, t->value );
    }
    free(buf);
    nvram_commit(  );
}
