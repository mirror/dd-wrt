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
 static bool usb_ufd_connected( char *str );
 static int usb_process_path( char *path );
 static int usb_add_ufd( void );

 
    // int hotplug_usb(void)
int main( int argc, char **argv ) 
{
     char *device, *interface;
     char *action;
     int class, subclass, protocol;

      if( !( action = getenv( "ACTION" ) )
	  ||  !( device = getenv( "TYPE" ) ) )
	 return EINVAL;
      sscanf( device, "%d/%d/%d", &class, &subclass, &protocol );
     if( class == 0 )
    {
	 if( !( interface = getenv( "INTERFACE" ) ) )
	     return EINVAL;
	 sscanf( interface, "%d/%d/%d", &class, &subclass, &protocol );
     }
     
	/* 
	 * If a new USB device is added and it is of storage class 
	 */ 
	if( class == 8 && subclass == 6   &&!strcmp( action, "add" ) )
	 return ( usb_add_ufd(  ) );
      return 0;
 }

  
    /* 
     *   Check if the UFD is still connected because the links  created in
     * /dev/discs are not removed when the UFD is  unplugged.  
     */ 
static bool   usb_ufd_connected( char *str ) 
{
     uint host_no;
     char proc_file[128];

     
	/* 
	 * Host no. assigned by scsi driver for this UFD 
	 */ 
	host_no = atoi( str );
      sprintf( proc_file, "/proc/scsi/usb-storage-%d/%d", host_no, host_no );
      if( eval( "/bin/grep", "-q", "Attached: Yes", proc_file ) == 0 )
	 return TRUE;
    
    else
	 return FALSE;
 }

  
    /* 
     *   Mount the path and look for the WCN configuration file.  If it
     * exists launch wcnparse to process the configuration.  
     */ 
 static int  usb_process_path( char *path ) 
{
     int ret = ENOENT;
     struct stat tmp_stat;

      eval( "/bin/mount", "-t", "vfat", path, "/mnt" );
    
	/* 
	 * if (stat("/mnt/SMRTNTKY/WSETTING.WFC", &tmp_stat) == 0) {
	 * eval("/usr/sbin/wcnparse", "-C", "/mnt",
	 * "SMRTNTKY/WSETTING.WFC");  ret = 0;  }  eval("/bin/umount",
	 * "/mnt");  
	 */ 
	return ret;
 }

  
    /* 
     * Handle hotplugging of UFD 
     */ 
static int  usb_add_ufd(  ) 
{
     DIR * dir;
     struct dirent *entry;
     char path[128];

      if( ( dir = opendir( "/dev/discs" ) ) == NULL )
	 return EINVAL;
     
	/* 
	 * Scan through entries in the directories 
	 */ 
	while( ( entry = readdir( dir ) ) != NULL )
    {
	 if( ( strncmp( entry->d_name, "disc", 4 ) ) )
	     continue;
	 
	    /* 
	     * Files created when the UFD is inserted are not  removed when
	     * it is removed. Verify the device  is still inserted.  Strip
	     * the "disc" and pass the rest of the string.  
	     */ 
	    if( usb_ufd_connected( entry->d_name + 4 ) == FALSE )
	     continue;
	  sprintf( path, "/dev/discs/%s/disc", entry->d_name );
	  eval( "/usr/sbin/disktype", path, ">/tmp/disktype.dump" );
	 eval( "/usr/sbin/disktype", path, ">/tmp/disktype.dump" );
	  
	    /* 
	     * Check if it has FAT file system 
	     */ 
	    if( eval( "/bin/grep", "-q", "FAT", "/tmp/disktype.dump" ) == 0 )
	{
	    
		/* 
		 * If it is partioned, mount first partition else raw disk 
		 */ 
		if( eval
		    ( "/bin/grep", "-q", "Partition",
		      "/tmp/disktype.dump" ) == 0 )
		
	    {
		 char part[10], *partitions, *next;
		 struct stat tmp_stat;

		  partitions = "part1 part2 part3 part4";
		 foreach( part, partitions, next )
		{
		     sprintf( path, "/dev/discs/%s/%s", entry->d_name, part );
		     if( stat( path, &tmp_stat ) )
			 continue;
		     if( usb_process_path( path ) == 0 )
			 return 0;
		 }
	     }
	    
	    else
	    {
		 if( usb_process_path( path ) == 0 )
		     return 0;
	     }
	 }
     }
      return 0;
 }


