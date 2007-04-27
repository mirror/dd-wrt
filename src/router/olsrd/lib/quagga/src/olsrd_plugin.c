/***************************************************************************
 projekt              : olsrd-quagga
 file                 : olsrd_plugin.c  
 usage                : olsrd-plugin-handler-stuff 
 copyright            : (C) 2006 by Immo 'FaUl' Wehrenberg
 e-mail               : immo@chaostreff-dortmund.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 ***************************************************************************/


#include <stdio.h>
#include <string.h>

#include "olsrd_plugin.h"
#include "olsr.h"
#include "scheduler.h"
#include "defs.h"
#include "quagga.h"
#include "kernel_routes.h"

#define PLUGIN_NAME    "OLSRD quagga plugin"
#define PLUGIN_VERSION "0.2.2"
#define PLUGIN_AUTHOR  "Immo 'FaUl' Wehrenberg"
#define MOD_DESC PLUGIN_NAME " " PLUGIN_VERSION " by " PLUGIN_AUTHOR

static void __attribute__ ((constructor)) my_init(void);
static void __attribute__ ((destructor)) my_fini(void);
static void redist_hna (void);


int olsrd_plugin_interface_version() {
  return OLSRD_PLUGIN_INTERFACE_VERSION;
}


int olsrd_plugin_register_param(char *key, char *value) {
  const char *zebra_route_types[] = {"system","kernel","connect","static",
			      "rip","ripng","ospf","ospf6","isis",
 			      "bgp","hsls", NULL};
  unsigned char i = 0;

  if(!strcmp(key, "redistribute")) {
    for (i = 0; zebra_route_types[i]; i++)
      if (!strcmp(value, zebra_route_types[i])) {
	zebra_redistribute(i);
	return 1;
      }
  }
  else if(!strcmp(key, "ExportRoutes")) {
    if (!strcmp(value, "only")) {
      if (!olsr_addroute_remove_function(&olsr_ioctl_add_route, AF_INET))
	puts ("AIII, could not remove the kernel route exporter");
      if (!olsr_delroute_remove_function(&olsr_ioctl_del_route, AF_INET))
	puts ("AIII, could not remove the kernel route deleter");
      olsr_addroute_add_function(&zebra_add_olsr_v4_route, AF_INET);
      olsr_delroute_add_function(&zebra_del_olsr_v4_route, AF_INET);
      return 1;
    }
    else if (!strcmp(value, "additional")) {
      olsr_addroute_add_function(&zebra_add_olsr_v4_route, AF_INET);
      olsr_delroute_add_function(&zebra_del_olsr_v4_route, AF_INET);
      return 1;
    }
  }
  else if (!strcmp(key, "Distance")) {
    unsigned int distance = atoi (value);
    if (distance < 255)
      zebra_olsr_distance(distance);
      return 1;
  }
  
  else if (!strcmp(key, "LocalPref")) {
    if (!strcmp(value, "true")) 
      zebra_olsr_localpref();
    else if (strcmp (value, "false"))
      return -1;
    return 1;
  }
  return -1;
}


int olsrd_plugin_init() {
  if(olsr_cnf->ip_version != AF_INET) {
    fputs("see the source - ipv6 so far not supportet\n" ,stderr);
    return 1;
  }

  //  olsr_register_timeout_function(&olsr_timeout);
  olsr_register_scheduler_event(&zebra_check, NULL, 1, 0, NULL);
  return 0;
}

static void my_init(void) {
  init_zebra();
}

static void my_fini(void) {
}

