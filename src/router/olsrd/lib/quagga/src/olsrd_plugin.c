
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
#include "plugin_util.h"
#include "olsr.h"
#include "scheduler.h"
#include "defs.h"
#include "quagga.h"
#include "kernel_routes.h"
#include "net_olsr.h"

#define PLUGIN_NAME    "OLSRD quagga plugin"
#define PLUGIN_VERSION "0.2.2"
#define PLUGIN_AUTHOR  "Immo 'FaUl' Wehrenberg"
#define MOD_DESC PLUGIN_NAME " " PLUGIN_VERSION " by " PLUGIN_AUTHOR
#define PLUGIN_INTERFACE_VERSION 5

static void __attribute__ ((constructor)) my_init(void);
static void __attribute__ ((destructor)) my_fini(void);

static set_plugin_parameter set_redistribute;
static set_plugin_parameter set_exportroutes;
static set_plugin_parameter set_distance;
static set_plugin_parameter set_localpref;

static export_route_function orig_addroute_function;
static export_route_function orig_delroute_function;

int
olsrd_plugin_interface_version(void)
{
  return PLUGIN_INTERFACE_VERSION;
}

static const struct olsrd_plugin_parameters plugin_parameters[] = {
  {.name = "redistribute",.set_plugin_parameter = &set_redistribute,},
  {.name = "ExportRoutes",.set_plugin_parameter = &set_exportroutes,},
  {.name = "Distance",.set_plugin_parameter = &set_distance,},
  {.name = "LocalPref",.set_plugin_parameter = &set_localpref,},
};

void
olsrd_get_plugin_parameters(const struct olsrd_plugin_parameters **params, int *size)
{
  *params = plugin_parameters;
  *size = sizeof plugin_parameters / sizeof *plugin_parameters;
}

static int
set_redistribute(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  const char *zebra_route_types[] = { "system", "kernel", "connect",
    "static", "rip", "ripng", "ospf",
    "ospf6", "isis", "bgp", "hsls"
  };
  unsigned int i;

  for (i = 0; i < ARRAYSIZE(zebra_route_types); i++) {
    if (!strcmp(value, zebra_route_types[i])) {
      zebra_redistribute(i);
      return 0;
    }
  }
  return 1;
}

static int
set_exportroutes(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  if (!strcmp(value, "only")) {
    orig_addroute_function = NULL;
    orig_delroute_function = NULL;
    olsr_addroute_function = zebra_add_olsr_v4_route;
    olsr_delroute_function = zebra_del_olsr_v4_route;
    zebra_export_routes(1);
  } else if (!strcmp(value, "additional")) {
    orig_addroute_function = olsr_addroute_function;
    orig_delroute_function = olsr_delroute_function;
    olsr_addroute_function = zebra_add_olsr_v4_route;
    olsr_delroute_function = zebra_del_olsr_v4_route;
    zebra_export_routes(1);
  } else
    zebra_export_routes(0);
  return 0;
}

static int
set_distance(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  int distance;

  if (set_plugin_int(value, &distance, addon))
    return 1;
  if (distance < 0 || distance > 255)
    return 1;
  zebra_olsr_distance(distance);
  return 0;
}

static int
set_localpref(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  int b;

  if (set_plugin_boolean(value, &b, addon))
    return 1;
  if (b)
    zebra_olsr_localpref();
  return 0;
}

int
olsrd_plugin_init(void)
{
  if (olsr_cnf->ip_version != AF_INET) {
    fputs("see the source - ipv6 so far not supported\n", stderr);
    return 1;
  }

  olsr_start_timer(1 * MSEC_PER_SEC, 0, OLSR_TIMER_PERIODIC, &zebra_check, NULL, 0);

  return 0;
}

static void
my_init(void)
{
  init_zebra();
}

static void
my_fini(void)
{
  zebra_cleanup();
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
