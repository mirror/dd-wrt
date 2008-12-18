/* 
 * USB hotplug service    Copyright 2007, Broadcom Corporation  All Rights
 * Reserved.    THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO
 * WARRANTIES OF ANY  KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR
 * OTHERWISE. BROADCOM  SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS  FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT
 * CONCERNING THIS SOFTWARE.    $Id$  
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <typedefs.h>
#include <shutils.h>
#include <bcmnvram.h>

static bool usb_ufd_connected( char *str );
static int usb_process_path( char *path, char *fs );
static int usb_add_ufd( void );

#define DUMPFILE	"/tmp/disktype.dump"

void start_hotplug_usb( void )
{
    char *device, *interface;
    char *action;
    int class, subclass, protocol;
    fprintf(stderr,"%s:%d\n",__FILE__,__LINE__);
    if( !(nvram_match ("usb_automnt", "1") ) )
    return;
//    fprintf(stderr,"%s:%d\n",__FILE__,__LINE__);

    if( !( action = getenv( "ACTION" ) ) || !( device = getenv( "TYPE" ) ) )
	return;
//    fprintf(stderr,"%s:%d\n",__FILE__,__LINE__);
    sscanf( device, "%d/%d/%d", &class, &subclass, &protocol );
//    fprintf(stderr,"%s:%d %s\n",__FILE__,__LINE__,device);
    if( class == 0 )
    {
	if( !( interface = getenv( "INTERFACE" ) ) )
	    return;
	sscanf( interface, "%d/%d/%d", &class, &subclass, &protocol );
    }
//    fprintf(stderr,"%s:%d\n",__FILE__,__LINE__);

    /* 
     * If a new USB device is added and it is of storage class 
     */
    if( class == 8 && subclass == 6 && !strcmp( action, "add" ) )
	usb_add_ufd(  );
//    fprintf(stderr,"%s:%d %s\n",__FILE__,__LINE__,device);
    return;
}

    /* 
     *   Check if the UFD is still connected because the links created in
     * /dev/discs are not removed when the UFD is  unplugged.  
     */
static bool usb_ufd_connected( char *str )
{
    uint host_no;
    char proc_file[128];
    FILE *fp;
    char line[256];
    /* 
     * Host no. assigned by scsi driver for this UFD 
     */
    host_no = atoi( str );
    sprintf( proc_file, "/proc/scsi/usb-storage-%d/%d", host_no, host_no );
 
	if( ( fp = fopen( proc_file, "r" ) ) )
	{
	while( fgets( line, sizeof( line ), fp ) != NULL )
	{
	if (strstr( line, "Attached: Yes" ) )
	{
//	fprintf(stderr,"%s:%d\n",__FILE__,__LINE__);
	fclose( fp );
	return TRUE;
	}
	}
	fclose( fp );
	}
	//in 2.6 kernels its a little bit different
	sprintf( proc_file, "/proc/scsi/usb-storage/%d",host_no );
	if( ( fp = fopen( proc_file, "r" ) ) )
	    {
//    fprintf(stderr,"%s:%d\n",__FILE__,__LINE__);
	    fclose(fp);
	    return TRUE;
	    }
//    fprintf(stderr,"%s:%d\n",__FILE__,__LINE__);
	return FALSE;
    
}

    /* 
     *   Mount the path and look for the WCN configuration file.  If it
     * exists launch wcnparse to process the configuration.  
     */
static int usb_process_path( char *path, char *fs)
{
    int ret = ENOENT;
    char mount_point[32];
//    fprintf(stderr,"%s:%d\n",__FILE__,__LINE__);
    
    sprintf( mount_point, "/%s", nvram_default_get( "usb_mntpoint", "mnt" ) ); 

    ret = eval( "/bin/mount", "-t", fs, path, mount_point );
    
    if( ret != 0 ) //give it another try
        ret = eval( "/bin/mount", path, mount_point );  //guess fs

    return ret;
}

    /* 
     * Handle hotplugging of UFD 
     */
static int usb_add_ufd(  )
{
    DIR *dir;
    FILE *fp;
    char line[256];
    struct dirent *entry;
    char path[128];
    char *fs = NULL;
    int is_part = 0;
    int is_mounted = 0;
	char part[10], *partitions, *next;
	struct stat tmp_stat;

	if( ( dir = opendir( "/dev/discs" ) ) == NULL )
	return EINVAL;

    /* 
     * Scan through entries in the directories 
     */
//    fprintf(stderr,"%s:%d\n",__FILE__,__LINE__);
    while( ( entry = readdir( dir ) ) != NULL )
    {
	if( ( strncmp( entry->d_name, "disc", 4 ) ) )
	    continue;

	/* 
	 * Files created when the UFD is inserted are not  removed when
	 * it is removed. Verify the device  is still inserted.  Strip
	 * the "disc" and pass the rest of the string.  
	 */
//    fprintf(stderr,"%s:%d\n",__FILE__,__LINE__);
	if( usb_ufd_connected( entry->d_name + 4 ) == FALSE )
	    continue;
//    fprintf(stderr,"%s:%d\n",__FILE__,__LINE__);
	sprintf( path, "/dev/discs/%s/disc", entry->d_name );
//    fprintf(stderr,"%s:%d\n",__FILE__,__LINE__);
	sysprintf ("/usr/sbin/disktype %s > %s", path, DUMPFILE);

	/* 
	 * Check if it has file system 
	 */
	if( ( fp = fopen( DUMPFILE, "r" ) ) )
	{
	while( fgets( line, sizeof( line ), fp ) != NULL )
	{
	if( strstr( line, "Partition" ) )
	is_part = 1;
	
	if (strstr( line, "file system" ) )
	{
	if (strstr( line, "FAT" ) )
	{
		fs = "vfat";
		break;
	}
	else if( strstr( line, "Ext2" ) )
	{
		fs = "ext2";
		break;
	}
	else if( strstr( line, "Ext3" ) )
	{
#ifdef HAVE_USB_ADVANCED
		fs = "ext3";
#else
		fs = "ext2";
#endif
		break;
	}
	}
	
	}
	fclose( fp );
	}
	 
	if( fs )
	{
	    /* 
	     * If it is partioned, mount first partition else raw disk 
	     */
	    if( is_part )
	    {
		partitions = "part1 part2 part3 part4 part5 part6";
		foreach( part, partitions, next )
		{
		    sprintf( path, "/dev/discs/%s/%s", entry->d_name, part );
		    if( stat( path, &tmp_stat ) )
			continue;
		    if( usb_process_path( path, fs ) == 0 )
		    	{
				is_mounted = 1;
				break;
				}
		}
	    }
	    else
	    {
		if( usb_process_path( path, fs ) == 0 )
			is_mounted = 1;
	    }
	    

	}
	
	if( ( fp = fopen( DUMPFILE, "a" ) ) )
	{
	    if( fs && is_mounted )
	    	fprintf( fp, "Status: <b>Mounted on /%s</b>\n", nvram_safe_get ("usb_mntpoint" ) );
	    else if ( fs )
	    	fprintf( fp, "Status: <b>Not mounted</b>\n" );
	    else 
	    	fprintf( fp, "Status: <b>Not mounted - Unsupported file system or disk not formated</b>\n" );	    
	    fclose( fp );
	}
	
	if( is_mounted && !nvram_match( "usb_runonmount", "" ) )
	{
	sprintf( path, "%s", nvram_safe_get( "usb_runonmount" ) );
	if( stat( path, &tmp_stat ) == 0 ) //file exists
		{
		system( path );
		}
	}	

	if( is_mounted ) //temp. fix: only mount 1st mountable part, then exit
		return 0;
	}
	return 0;
}
