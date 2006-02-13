/*
this source is published under no warranty and may be published without any fee
to third parties. 
The use of this source is permitted without any charge for PRIVATE and NON COMERCIAL usage
only unless other rights are granted by the firmware author only.

DD-WRT v23 (c) 2004 - 2005 by Sebastian Gottschall / Blueline AG
*/


#ifdef HAVE_MSSID
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <shutils.h>


void
do_mssid (char *lan_ifname)
{
  //bridge the virtual interfaces too
  char *next;
  char var[80];
  char *vifs = nvram_safe_get ("wl0_vifs");
  if (vifs != NULL)
    foreach (var, vifs, next)
    {
      eval ("brctl", "addif", lan_ifname, var);
    }
}
#endif
