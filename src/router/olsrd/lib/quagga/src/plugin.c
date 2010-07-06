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
 * File               : plugin.c
 * Description        : functions to set zebra plugin parameters
 * ------------------------------------------------------------------------- */

#include "defs.h"
#include "olsr.h"
#include "log.h"
#include "olsrd_plugin.h"
#include "plugin_util.h"
#include "net_olsr.h"

#include "common.h"
#include "quagga.h"
#include "packet.h"
#include "plugin.h"

static void *my_realloc(void *, size_t, const char *);

static void
*my_realloc(void *buf, size_t s, const char *c)
{

  buf = realloc(buf, s);
  if (!buf) {
    OLSR_PRINTF(1, "(QUAGGA) Out of memory: %s!\n", strerror(errno));
    olsr_syslog(OLSR_LOG_ERR, "(QUAGGA) Out of memory!\n");
    olsr_exit(c, EXIT_FAILURE);
  }

  return buf;
}

int
zplugin_redistribute(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  const char *zroute_types[] = { "system", "kernel", "connect",
    "static", "rip", "ripng", "ospf", "ospf6", "isis", "bgp",
    "hsls", "olsr", "batman"
  };
  unsigned int i;

  for (i = 0; i < ARRAYSIZE(zroute_types) && i < ZEBRA_ROUTE_MAX; i++) {
    if (!strcmp(value, zroute_types[i]))
      zebra.redistribute[i] = 1;
  }

  return 0;
}

int
zplugin_exportroutes(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)))
{

  if (!strcmp(value, "only")) {
    olsr_addroute_function = zebra_addroute;
    olsr_delroute_function = zebra_delroute;
    olsr_addroute6_function = zebra_addroute;
    olsr_delroute6_function = zebra_delroute;
    zebra.options |= OPTION_EXPORT;
  } else if (!strcmp(value, "additional")) {
    olsr_addroute_function = zebra_addroute;
    olsr_delroute_function = zebra_delroute;
    olsr_addroute6_function = zebra_addroute;
    olsr_delroute6_function = zebra_delroute;
    zebra.options |= OPTION_EXPORT;
  }

  return 0;
}

int
zplugin_distance(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  int distance;

  if (set_plugin_int(value, &distance, addon))
    return 1;
  if (distance < 0 || distance > 255)
    return 1;
  zebra.distance = distance;

  return 0;
}

int
zplugin_localpref(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  int b;

  if (set_plugin_boolean(value, &b, addon))
    return 1;
  if (b)
    zebra.flags &= ZEBRA_FLAG_SELECTED;

  return 0;
}

int
zplugin_sockpath(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon)
{
  size_t len;
  char sockpath[PATH_MAX];

  if (set_plugin_string(value, &sockpath, addon))
    return 1;
  len = strlen(sockpath) + 1;
  zebra.sockpath = my_realloc(zebra.sockpath, len, "QUAGGA: Grow socket path");
  memcpy(zebra.sockpath, sockpath, len);

  return 0;
}

int
zplugin_port(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  unsigned int port;

  if (set_plugin_port(value, &port, addon))
    return 1;
  zebra.port = port;

  return 0;
}

int
zplugin_version(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  int version;

  if (set_plugin_int(value, &version, addon))
    return 1;
  if (version < 0 || version > 1)
    return 1;
  zebra.version = version;

  return 0;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
