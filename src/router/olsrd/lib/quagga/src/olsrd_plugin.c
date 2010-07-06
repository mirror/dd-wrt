/*
 * OLSRd Quagga plugin
 *
 * Copyright (C) 2006-2008 Immo 'FaUl' Wehrenberg <immo@chaostreff-dortmund.de>
 * Copyright (C) 2007-2010 Vasilis Tsiligiannis <acinonyxs@yahoo.gr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation or - at your option - under
 * the terms of the GNU General Public Licence version 2 but can be
 * linked to any BSD-Licenced Software with public available sourcecode
 *
 */

/* -------------------------------------------------------------------------
 * File               : olsrd_plugin.c
 * Description        : functions to setup plugin
 * ------------------------------------------------------------------------- */

#include "olsrd_plugin.h"
#include "scheduler.h"
#include "defs.h"

#include "quagga.h"
#include "plugin.h"
#include "parse.h"

#define PLUGIN_NAME    "OLSRD quagga plugin"
#define PLUGIN_VERSION "0.2.2"
#define PLUGIN_AUTHOR  "Immo 'FaUl' Wehrenberg"
#define MOD_DESC PLUGIN_NAME " " PLUGIN_VERSION " by " PLUGIN_AUTHOR
#define PLUGIN_INTERFACE_VERSION 5

static void __attribute__ ((constructor)) my_init(void);
static void __attribute__ ((destructor)) my_fini(void);

int
olsrd_plugin_interface_version(void)
{

  return PLUGIN_INTERFACE_VERSION;
}

static const struct olsrd_plugin_parameters plugin_parameters[] = {
  {.name = "Redistribute",.set_plugin_parameter = &zplugin_redistribute,},
  {.name = "ExportRoutes",.set_plugin_parameter = &zplugin_exportroutes,},
  {.name = "Distance",.set_plugin_parameter = &zplugin_distance,},
  {.name = "LocalPref",.set_plugin_parameter = &zplugin_localpref,},
  {.name = "SockPath",.set_plugin_parameter = &zplugin_sockpath,.addon = {PATH_MAX},},
  {.name = "Port",.set_plugin_parameter = &zplugin_port,},
  {.name = "Version",.set_plugin_parameter = &zplugin_version,},
};

void
olsrd_get_plugin_parameters(const struct olsrd_plugin_parameters **params, int *size)
{

  *params = plugin_parameters;
  *size = ARRAYSIZE(plugin_parameters);

}

int
olsrd_plugin_init(void)
{

  olsr_start_timer(1 * MSEC_PER_SEC, 0, OLSR_TIMER_PERIODIC, &zparse, NULL, 0);

  return 0;
}

static void
my_init(void)
{

  zebra_init();

}

static void
my_fini(void)
{

  zebra_fini();

}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
