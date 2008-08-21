/*
 * jffs.c
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
#ifdef HAVE_JFFS2
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <sys/mount.h>

void stop_jffs2( void )
{
    eval( "umount", "/jffs" );
    rmmod( "jffs2" );
}

void start_jffs2( void )
{
    char *rwpart = "ddwrt";
    int itworked = 0;

    if( nvram_match( "sys_enable_jffs2", "1" ) )
    {
	if( nvram_match( "sys_clean_jffs2", "1" ) )
	{
	    nvram_set( "sys_clean_jffs2", "0" );
	    nvram_commit(  );
	    itworked = eval( "mtd", "erase", rwpart );
	    insmod( "crc32" );
	    insmod( "jffs2" );
	    char dev[64];

	    sprintf( dev, "/dev/mtdblock/%d", getMTD( "ddwrt" ) );
	    itworked += mount( dev, "/jffs", "jffs2", MS_MGC_VAL, NULL );
	    if( itworked )
	    {
		nvram_set( "jffs_mounted", "0" );
	    }
	    else
	    {
		nvram_set( "jffs_mounted", "1" );
	    }

	}
	else
	{
	    itworked = eval( "mtd", "unlock", rwpart );
	    insmod( "crc32" );
	    insmod( "jffs2" );
	    char dev[64];

	    sprintf( dev, "/dev/mtdblock/%d", getMTD( "ddwrt" ) );
	    itworked += mount( dev, "/jffs", "jffs2", MS_MGC_VAL, NULL );
	    if( itworked )
	    {
		nvram_set( "jffs_mounted", "0" );
	    }
	    else
	    {
		nvram_set( "jffs_mounted", "1" );
	    }

	}
    }

}
#endif
