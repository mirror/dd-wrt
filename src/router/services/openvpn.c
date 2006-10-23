/*
 * openvpn.c
 *
 * Copyright (C) 2005 - 2006 Sebastian Gottschall <gottschall@dd-wrt.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */


#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <shutils.h>


#ifdef HAVE_NEWMEDIA
static int
start_openvpnserver (void)
{

  if (nvram_invmatch ("openvpn_enable", "1"))
    return -1;
  mkdir ("/tmp/openvpn", 0700);
  write_nvram ("/tmp/openvpn/dh.pem", "openvpn_dh");
  write_nvram ("/tmp/openvpn/ca.crt", "openvpn_ca");
  write_nvram ("/tmp/openvpn/ca.crl", "openvpn_crl");
  write_nvram ("/tmp/openvpn/cert.pem", "openvpn_client");
  write_nvram ("/tmp/openvpn/key.pem", "openvpn_key");
  write_nvram ("/tmp/openvpn/ta.key", "openvpn_tlsauth");
  write_nvram ("/tmp/openvpn/openvpn.conf", "openvpn_config");

  FILE *fp = fopen ("/tmp/openvpn/route-up.sh", "wb");
  if (fp == NULL)
    return -1;
  fprintf (fp, "iptables -I INPUT -i tun0 -j ACCEPT\n");
  fclose (fp);
  fp = fopen ("/tmp/openvpn/route-down.sh", "wb");
  if (fp == NULL)
    return -1;
  fprintf (fp, "iptables -D INPUT -i tun0 -j ACCEPT\n");
  fclose (fp);
  chmod ("/tmp/openvpn/route-up.sh", 0700);
  chmod ("/tmp/openvpn/route-down.sh", 0700);
  eval ("openvpn", "--config", "/tmp/openvpn/openvpn.conf", "--route-up",
	"/tmp/openvpn/route-up.sh", "--down", "/tmp/openvpn/route-down.sh",
	"--daemon");
  return 0;
}

static int
stop_openvpnserver (void)
{
  killall("openvpn",SIGKILL);
  return 0;
}

int
start_openvpn (void)
{
  return 0;
}

int
stop_openvpn (void)
{
  return 0;
}

int
start_openvpnserverwan (void)
{
  if (nvram_match ("openvpn_onwan", "1"))
    return start_openvpnserver ();
  return 0;
}

int
stop_openvpnserverwan (void)
{
  if (nvram_match ("openvpn_onwan", "1"))
    return stop_openvpnserver ();
  return 0;
}

int
start_openvpnserversys (void)
{
  if (nvram_match ("openvpn_onwan", "0"))
    return start_openvpnserver ();
  return 0;
}

int
stop_openvpnserversys (void)
{
  if (nvram_match ("openvpn_onwan", "0"))
    return stop_openvpnserver ();
  return 0;
}

#else
#ifdef HAVE_OPENVPN
int
start_openvpn (void)
{
  if (nvram_invmatch ("openvpn_enable", "1"))
    return -1;
  mkdir ("/tmp/openvpn", 0700);
  FILE *fp = fopen ("/tmp/openvpn/openvpn.conf", "wb");
  if (fp == NULL)
    return -1;
  fprintf (fp, "client\n");
  fprintf (fp, "dev tun\n");
  fprintf (fp, "proto %s\n", nvram_safe_get ("openvpn_proto"));
  fprintf (fp, "remote %s %s\n", nvram_safe_get ("openvpn_remoteip"),
	   nvram_safe_get ("openvpn_remoteport"));
  fprintf (fp, "resolv-retry infinite\n");
  fprintf (fp, "nobind\n");
//fprintf(fp,"user nobody\n");
//fprintf(fp,"group nobody\n");
  fprintf (fp, "persist-key\n");
  fprintf (fp, "persist-tun\n");
  if (nvram_invmatch ("openvpn_mtu", ""))
    fprintf (fp, "tun-mtu %s\n", nvram_safe_get ("openvpn_mtu"));
  if (nvram_invmatch ("openvpn_extramtu", ""))
    fprintf (fp, "tun-mtu-extra %s\n", nvram_safe_get ("openvpn_extramtu"));
  if (nvram_invmatch ("openvpn_mssfix", ""))
    fprintf (fp, "mssfix %s\n", nvram_safe_get ("openvpn_mssfix"));

  fprintf (fp, "ca /tmp/openvpn/ca.crt\n");
  fprintf (fp, "cert /tmp/openvpn/client.crt\n");

  // Botho 22/05/2006 - start
  if (nvram_match ("openvpn_certtype", "1"))
    fprintf (fp, "ns-cert-type server\n");
  // Botho 22/05/2006 - end

  fprintf (fp, "key /tmp/openvpn/client.key\n");

  if (nvram_match ("openvpn_lzo", "1"))
    fprintf (fp, "comp-lzo\n");

  fclose (fp);
  fp = fopen ("/tmp/openvpn/route-up.sh", "wb");
  if (fp == NULL)
    return -1;
  fprintf (fp, "iptables -A POSTROUTING -t nat -o tun0 -j MASQUERADE\n");
  fclose (fp);
  fp = fopen ("/tmp/openvpn/route-down.sh", "wb");
  if (fp == NULL)
    return -1;
  fprintf (fp, "iptables -D POSTROUTING -t nat -o tun0 -j MASQUERADE\n");
  fclose (fp);
  chmod ("/tmp/openvpn/route-up.sh", 0700);
  chmod ("/tmp/openvpn/route-down.sh", 0700);

  write_nvram ("/tmp/openvpn/ca.crt", "openvpn_ca");
  write_nvram ("/tmp/openvpn/client.crt", "openvpn_client");
  write_nvram ("/tmp/openvpn/client.key", "openvpn_key");
  eval ("openvpn", "--config", "/tmp/openvpn/openvpn.conf", "--route-up",
	"/tmp/openvpn/route-up.sh", "--down", "/tmp/openvpn/route-down.sh",
	"--daemon");
  return 0;
}

int
stop_openvpn (void)
{
  killall("openvpn",SIGKILL);
}

#endif
#endif
