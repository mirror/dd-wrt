/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
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

#include <stdbool.h>

int
zplugin_redistribute(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  const char *zroute_types[] = { "system", "kernel", "connect",
                                 "static", "rip", "ripng", "ospf", "ospf6", "isis", "bgp",
                                 "hsls", "olsr", "batman", "babel"
  };
  unsigned int i;

  unsigned int max = MIN(ARRAYSIZE(zroute_types), ZEBRA_ROUTE_MAX);
  for (i = 0; i < max; i++) {
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
    zebra.orig_addroute_function = olsr_addroute_function;
    zebra.orig_delroute_function = olsr_delroute_function;
    zebra.orig_addroute6_function = olsr_addroute6_function;
    zebra.orig_delroute6_function = olsr_delroute6_function;
    olsr_addroute_function = zebra_addroute;
    olsr_delroute_function = zebra_delroute;
    olsr_addroute6_function = zebra_addroute;
    olsr_delroute6_function = zebra_delroute;
    zebra.options |= OPTION_EXPORT | OPTION_ROUTE_ADDITIONAL;
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
  bool b;

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
  zebra.sockpath = olsr_realloc(zebra.sockpath, len, "QUAGGA: grow socket path");
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
  if (version < 0 || version > 2)
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
