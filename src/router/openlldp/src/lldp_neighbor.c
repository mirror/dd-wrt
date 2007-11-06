/*******************************************************************
 *
 * OpenLLDP Neighbor 
 *
 * Licensed under a dual GPL/Proprietary license.  
 * See LICENSE file for more info.
 * 
 * File: lldp_neighbor.c
 * 
 * Authors: Jason Peterson (condurre@users.sourceforge.net)
 *
 *******************************************************************/

#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/utsname.h>
#include <unistd.h>

#include <string.h>

#include "lldp_debug.h"
#include "lldp_neighbor.h"

int get_sys_desc() {
  
  int retval;
  struct utsname sysinfo;

  bzero(&lldp_systemdesc[0], 512);
 
  retval = uname(&sysinfo);

  if (retval < 0) { 
    
    debug_printf(DEBUG_NORMAL, "Call to uname failed!\n");
    exit(0);    
    
  }

  debug_printf(DEBUG_NORMAL, "sysinfo.machine: %s\n", sysinfo.machine);  
  debug_printf(DEBUG_NORMAL, "sysinfo.sysname: %s\n", sysinfo.sysname);
  debug_printf(DEBUG_NORMAL, "sysinfo.release: %s\n", sysinfo.release);

  strcpy(lldp_systemdesc, sysinfo.machine);
  strcat(lldp_systemdesc, "/");
  strcat(lldp_systemdesc, "DD-WRT Linux");  
  strcat(lldp_systemdesc, " ");
  strcat(lldp_systemdesc, sysinfo.release);

  debug_printf(DEBUG_NORMAL, "lldp_systemdesc: %s\n", lldp_systemdesc);

  return(0);
  
}

int get_sys_fqdn() {

int retval;

bzero(&lldp_systemname[0], 512);

retval = gethostname(lldp_systemname, 255);

if (retval < 0) 
	{
	debug_printf(DEBUG_NORMAL, "gethostname() failed! retval = %d.\n", retval);
	}

strcat(lldp_systemname, ".");

retval = getdomainname(&lldp_systemname[strlen(lldp_systemname)], 255 - strlen(lldp_systemname));

if (retval < 0)
	{
	debug_printf(DEBUG_NORMAL, "getdomainname() failed! retval = %d.\n", retval);
	}

debug_printf(DEBUG_NORMAL, "lldp_systemname: %s\n", lldp_systemname);

return(0);

}
