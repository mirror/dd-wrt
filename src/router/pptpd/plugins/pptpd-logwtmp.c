/*
 * $Id: pptpd-logwtmp.c,v 1.2 2004/04/29 09:54:01 quozl Exp $
 * pptpd-logwtmp.c - pppd plugin to update wtmp for a pptpd user
 *
 * Copyright 2004 James Cameron.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 */
#include <unistd.h>
#include <utmp.h>
#include "pppd.h"

char pppd_version[] = VERSION;

static char pptpd_original_ip[PATH_MAX+1];

static option_t options[] = {
  { "pptpd-original-ip", o_string, pptpd_original_ip,
    "Original IP address of the PPTP connection",
    OPT_STATIC, NULL, PATH_MAX },
  { NULL }
};

static void ip_up(void *opaque, int arg)
{
  if (debug)
    notice("pptpd-logwtmp.so ip-up %s %s %s", ifname, peer_authname, 
	   pptpd_original_ip);
  logwtmp(ifname, peer_authname, pptpd_original_ip);
}

static void ip_down(void *opaque, int arg)
{
  if (debug) 
    notice("pptpd-logwtmp.so ip-down %s", ifname);
  logwtmp(ifname, "", "");
}

void plugin_init(void)
{
  add_options(options);
  add_notifier(&ip_up_notifier, ip_up, NULL);
  add_notifier(&ip_down_notifier, ip_down, NULL);
  if (debug) 
    notice("pptpd-logwtmp: $Version$");
}
