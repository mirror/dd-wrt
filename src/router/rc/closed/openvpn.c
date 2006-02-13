/*
this source is published under no warranty and may be published without any fee
to third parties. 
The use of this source is permitted without any charge for PRIVATE and NON COMERCIAL usage
only unless other rights are granted by the firmware author only.

DD-WRT v23 (c) 2004 - 2005 by Sebastian Gottschall / Blueline AG
*/


#include <ctype.h>
#include <string.h>
#include <stdlib.h>
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
  eval ("killall", "-9", "openvpn");
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
  eval ("killall", "-9", "openvpn");
}

#endif
#endif
