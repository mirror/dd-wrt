/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2005, Andreas Tonnesen(andreto@olsr.org)
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

#include "olsrd_conf.h"
#include "../ipcalc.h"
#include "../net_olsr.h"
#include "../common/autobuf.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int olsrd_write_cnf(struct olsrd_config *cnf, const char *fname) {
  FILE *fd;
  struct autobuf abuf;

  fd = fopen(fname, "w");

  if (fd == NULL) {
    fprintf(stderr, "Could not open file %s for writing\n%s\n", fname, strerror(errno));
    return -1;
  }

  printf("Writing config to file \"%s\".... ", fname);
  abuf_init(&abuf, 1024);
  olsrd_write_cnf_autobuf(&abuf, cnf);
  if (fwrite(abuf.buf, abuf.len, 1, fd) < (size_t)abuf.len) {
    fprintf(stderr, "Error, could not write the complete config file.\n");
  }
  abuf_free(&abuf);
  fclose(fd);

  printf("DONE\n");

  return 1;
}

static int
if_appendf(struct autobuf *autobuf, bool comments, const char *fmt, ...)  __attribute__ ((format(printf, 3, 4)));

static int
if_appendf(struct autobuf *autobuf, bool comments, const char *fmt, ...)
{
  int rv;
  va_list ap;
  char *first;

  if (!comments) {
    va_start(ap, fmt);
    first = va_arg(ap, char*);
    va_end(ap);
    if (*first) {
      return 0;
    }
  }

  va_start(ap, fmt);
  rv = abuf_vappendf(autobuf, fmt, ap);
  va_end(ap);
  return rv;
}

static void olsrd_write_if_autobuf(struct autobuf *out, struct if_config_options *cnfi, bool comments) {
  struct ipaddr_str ipbuf;
  struct olsr_lq_mult *mult;

  abuf_puts(out, "{\n");
  if (comments) abuf_puts(out,
    "    # Interface Mode is used to prevent unnecessary\n"
    "    # packet forwarding on switched ethernet interfaces\n"
    "    # valid Modes are \"mesh\" and \"ether\"\n"
    "    # (default is \"mesh\")\n"
    "    \n");
  if_appendf(out, comments, "    %sMode \"%s\"\n",
      cnfi->mode == DEF_IF_MODE ? "# " : "",
      OLSR_IF_MODE[cnfi->mode]);
  if (comments) abuf_puts(out,
    "    \n"
    "    # IPv4 broadcast address for outgoing OLSR packets.\n"
    "    # One usefull example would be 255.255.255.255\n"
    "    # The second useful value would be to\n"
    "    # specify the peer adress of an ptp-tunnel.\n"
    "    # another name of this parameter is \"IPv4Multicast\"\n"
    "    # (default is 0.0.0.0, which triggers the usage of the\n"
    "    # interface broadcast IP)\n"
    "    \n");
  if_appendf(out, comments, "    %sIp4Broadcast      %s\n",
      cnfi->ipv4_multicast.v4.s_addr == 0 ? "# " : "",
      inet_ntop(AF_INET, &cnfi->ipv4_multicast, ipbuf.buf, sizeof(ipbuf)));
  if (comments) abuf_puts(out,
    "    \n"
    "    # IPv6 multicast address\n"
    "    # (default is FF02::6D, the manet-router linklocal multicast)\n"
    "    \n");
  if_appendf(out, comments, "    %sIPv6Multicast %s\n",
      memcmp(&cnfi->ipv6_multicast, &ipv6_def_multicast, sizeof(ipv6_def_multicast)) == 0 ? "# " : "",
      inet_ntop(AF_INET6, &cnfi->ipv6_multicast, ipbuf.buf, sizeof(ipbuf)));
  if (comments) abuf_puts(out,
    "    \n"
    "    # IPv4 src address for outgoing OLSR packages\n"
    "    # (default is 0.0.0.0, which triggers usage of the interface IP)\n"
    "    \n");
  if_appendf(out, comments, "    %sIPv4Src %s\n",
      cnfi->ipv4_src.v4.s_addr == 0 ? "# " : "",
      inet_ntop(AF_INET, &cnfi->ipv4_src, ipbuf.buf, sizeof(ipbuf)));
  if (comments) abuf_puts(out,
    "    \n"
    "    # IPv6 src prefix. OLSRd will choose one of the interface IPs\n"
    "    # which matches the prefix of this parameter.\n"
    "    # (default is 0::/0, which triggers the usage\n"
    "    # of a not-linklocal interface IP)\n"
    "    \n");
  if_appendf(out, comments, "    %sIPv6Src %s\n",
      cnfi->ipv6_src.prefix_len == 0 ? "# " : "",
      inet_ntop(AF_INET6, &cnfi->ipv6_src, ipbuf.buf, sizeof(ipbuf)));
  if (comments) abuf_puts(out,
    "    \n"
    "    # Emission intervals in seconds.\n"
    "    # If not defined, Freifunk network defaults are used\n"
    "    # (default is 2.0/20.0 for Hello and 5.0/300.0 for Tc/Mid/Hna)\n"
    "    \n");
  if_appendf(out, comments, "    %sHelloInterval       %3.1f\n",
      cnfi->hello_params.emission_interval == HELLO_INTERVAL ? "# " : "",
      cnfi->hello_params.emission_interval);
  if_appendf(out, comments, "    %sHelloValidityTime   %3.1f\n",
      cnfi->hello_params.validity_time == NEIGHB_HOLD_TIME ? "# " : "",
      cnfi->hello_params.validity_time);
  if_appendf(out, comments, "    %sTcInterval          %3.1f\n",
      cnfi->tc_params.emission_interval == TC_INTERVAL ? "# " : "",
      cnfi->tc_params.emission_interval);
  if_appendf(out, comments, "    %sTcValidityTime      %3.1f\n",
      cnfi->tc_params.validity_time == TOP_HOLD_TIME ? "# " : "",
      cnfi->tc_params.validity_time);
  if_appendf(out, comments, "    %sMidInterval         %3.1f\n",
      cnfi->mid_params.emission_interval == MID_INTERVAL ? "# " : "",
      cnfi->mid_params.emission_interval);
  if_appendf(out, comments, "    %sMidValidityTime     %3.1f\n",
      cnfi->mid_params.validity_time == MID_HOLD_TIME ? "# " : "",
      cnfi->mid_params.validity_time);
  if_appendf(out, comments, "    %sHnaInterval         %3.1f\n",
      cnfi->hna_params.emission_interval == HNA_INTERVAL ? "# " : "",
      cnfi->hna_params.emission_interval);
  if_appendf(out, comments, "    %sHnaValidityTime     %3.1f\n",
      cnfi->hna_params.validity_time == HNA_HOLD_TIME ? "# " : "",
      cnfi->hna_params.validity_time);
  if (comments) abuf_puts(out,
    "    \n"
    "    # When multiple links exist between hosts\n"
    "    # the weight of interface is used to determine\n"
    "    # the link to use. Normally the weight is\n"
    "    # automatically calculated by olsrd based\n"
    "    # on the characteristics of the interface,\n"
    "    # but here you can specify a fixed value.\n"
    "    # Olsrd will choose links with the lowest value.\n"
    "    # Note:\n"
    "    # Interface weight is used only when LinkQualityLevel is set to 0.\n"
    "    # For any other value of LinkQualityLevel, the interface ETX\n"
    "    # value is used instead.\n");
  if_appendf(out, comments, "    %sWeight %d\n",
      !cnfi->weight.fixed ? "# " : "",
      cnfi->weight.value);
  if (comments) abuf_puts(out,
    "    \n"
    "    # If a certain route should be preferred\n"
    "    # or ignored by the mesh, the Link Quality\n"
    "    # value of a node can be multiplied with a factor\n"
    "    # entered here. In the example the route\n"
    "    # using 192.168.0.1 would rather be ignored.\n"
    "    # A multiplier of 0.5 will result in a small\n"
    "    # (bad) LinkQuality value and a high (bad)\n"
    "    # ETX value.\n"
    "    # Note:\n"
    "    # Link quality multiplier is used only when\n"
    "    # LinkQualityLevel is > 0.\n"
    "    \n");
  mult = cnfi->lq_mult;

  if (mult == NULL) {
    if (comments) abuf_puts(out, "    # LinkQualityMult 192.168.0.1 0.5\n");
  } else {
    while (mult != NULL) {
      if_appendf(out, comments, "    LinkQualityMult    %s %0.2f\n",
          olsr_ip_to_string(&ipbuf, &mult->addr),
          (float)(mult->value) / 65536.0);
      mult = mult->next;
    }
  }
  abuf_puts(out, "}\n");
}

void olsrd_write_cnf_autobuf(struct autobuf *out, struct olsrd_config *cnf) {
  struct ip_prefix_list *hna = cnf->hna_entries;
  struct olsr_if *interf = cnf->interfaces;
  struct plugin_entry *plugins = cnf->plugins;
  struct plugin_param *pl_param;
  struct ip_prefix_list *ipc_nets = cnf->ipc_nets;

  struct ipaddr_str ipbuf;
  bool first;

  abuf_appendf(out, "#\n"
      "# Configuration file for %s\n"
      "# automatically generated by olsrd-cnf parser v. %s\n"
      "#\n"
      "\n",
      olsrd_version, PARSER_VERSION);
  abuf_puts(out,
    "# OLSR.org routing daemon config file\n"
    "# This file contains ALL available options and explanations about them\n"
    "#\n"
    "# Lines starting with a # are discarded\n"
    "#\n"
    "\n"
    "#### ATTENTION for IPv6 users ####\n"
    "# Because of limitations in the parser IPv6 addresses must NOT\n"
    "# begin with a \":\", so please add a \"0\" as a prefix.\n"
    "\n"
    "###########################\n"
    "### Basic configuration ###\n"
    "###########################\n"
    "# keep this settings at the beginning of your first configuration file\n"
    "\n"
    "# Debug level (0-9)\n"
    "# If set to 0 the daemon runs in the background, unless \"NoFork\" is set to true\n"
    "# (Default is 1)\n"
    "\n");
  abuf_appendf(out, "%sDebugLevel  %d\n",
      cnf->debug_level == DEF_DEBUGLVL ? "# " : "",
      cnf->debug_level);
  abuf_puts(out,
    "\n"
    "# IP version to use (4 or 6)\n"
    "# (Default is 4)\n"
    "\n");
  abuf_appendf(out, "%sIpVersion %d\n",
      cnf->ip_version == DEF_IP_VERSION ? "# " : "",
      cnf->ip_version == AF_INET ? 4 : 6);
  abuf_puts(out,
    "\n"
    "#################################\n"
    "### OLSRd agent configuration ###\n"
    "#################################\n"
    "# this parameters control the settings of the routing agent which are not\n"
    "# related to the OLSR protocol and it's extensions\n"
    "\n"
    "# Clear the screen each time the internal state changes\n"
    "# (Default is yes)\n"
    "\n");
  abuf_appendf(out, "%sClearScreen     %s\n",
      cnf->clear_screen == DEF_CLEAR_SCREEN ? "# " : "",
      cnf->clear_screen ? "yes" : "no");
  abuf_puts(out,
    "\n"
    "# Should olsrd keep on running even if there are\n"
    "# no interfaces available? This is a good idea\n"
    "# for a PCMCIA/USB hotswap environment.\n"
    "# (Default is yes)\n"
    "\n");
  abuf_appendf(out, "%sAllowNoInt  %s\n",
      cnf->allow_no_interfaces == DEF_ALLOW_NO_INTS ? "# " : "",
      cnf->allow_no_interfaces ? "yes" : "no");
  abuf_puts(out,
    "\n"
    "# LockFile\n"
    "# The lockfile is used to prevent multiple OLSR instances running at the same\n"
    "# time.\n"
    "# (Linux/BSD default is \"/var/run/olsrd-ipv(4/6).lock\")\n"
    "# (Win32 default is \"<configfile>-ipv(4/6).lock\")\n"
    "\n");
  abuf_appendf(out, "%sLockFile \"%s\"\n",
      cnf->lock_file == NULL ? "# " : "",
      cnf->lock_file ? cnf->lock_file : "lockfile");
  abuf_puts(out,
    "\n"
    "# Polling rate for OLSR sockets in seconds (float). \n"
    "# (Default is 0.05)\n"
    "\n");
  abuf_appendf(out, "%sPollrate  %.2f\n",
      cnf->pollrate == DEF_POLLRATE ? "# " : "",
      cnf->pollrate);
  abuf_puts(out,
    "\n"
    "# Interval to poll network interfaces for configuration changes (in seconds).\n"
    "# Linux systems can detect interface statechange via netlink sockets.\n"
    "# (Defaults is 2.5)\n"
    "\n");
  abuf_appendf(out, "%sNicChgsPollInt  %.1f\n",
      cnf->nic_chgs_pollrate == DEF_NICCHGPOLLRT ? "# " : "",
      cnf->nic_chgs_pollrate);
  abuf_puts(out,
    "\n"
    "# TOS(type of service) value for the IP header of control traffic.\n"
    "# (Default is 16)\n"
    "\n");
  abuf_appendf(out, "%sTosValue %u\n",
      cnf->tos == DEF_TOS ? "# " : "",
      cnf->tos);
  abuf_puts(out,
    "\n"
    "# FIBMetric controls the metric value of the host-routes OLSRd sets.\n"
    "# - \"flat\" means that the metric value is always 2. This is the preferred value\n"
    "#   because it helps the linux kernel routing to clean up older routes\n"
    "# - \"correct\" use the hopcount as the metric value.\n"
    "# - \"approx\" use the hopcount as the metric value too, but does only update the\n"
    "#   hopcount if the nexthop changes too\n"
    "# (Default is \"flat\")\n"
    "\n");
  abuf_appendf(out, "%sFIBMetric \"%s\"\n",
      cnf->fib_metric == DEF_FIB_METRIC ? "# " : "",
      FIB_METRIC_TXT[cnf->fib_metric]);
  abuf_puts(out,
    "\n"
    "#######################################\n"
    "### Linux specific OLSRd extensions ###\n"
    "#######################################\n"
    "# these parameters are only working on linux at the moment, but might become\n"
    "# useful on BSD in the future\n"
    "\n"
    "# SrcIpRoutes tells OLSRd to set the Src flag of host routes to the originator-ip\n"
    "# of the node. In addition to this an additional localhost device is created\n"
    "# to make sure the returning traffic can be received.\n"
    "# (Default is \"no\")\n"
    "\n");
  abuf_appendf(out, "%sSrcIpRoutes %s\n",
      cnf->use_src_ip_routes == DEF_USE_SRCIP_ROUTES ? "# " : "",
      cnf->use_src_ip_routes ? "yes" : "no");
  abuf_puts(out,
    "\n"
    "# Specify the proto tag to be used for routes olsr inserts into kernel\n"
    "# currently only implemented for linux\n"
    "# valid values under linux are 1 .. 254\n"
    "# 1 gets remapped by olsrd to 0 UNSPECIFIED (1 is reserved for ICMP redirects)\n"
    "# 2 KERNEL routes (not very wise to use)\n"
    "# 3 BOOT (should in fact not be used by routing daemons)\n"
    "# 4 STATIC \n"
    "# 8 .. 15 various routing daemons (gated, zebra, bird, & co)\n"
    "# (defaults to 0 which gets replaced by an OS-specific default value\n"
    "# under linux 3 (BOOT) (for backward compatibility)\n"
    "\n");
  abuf_appendf(out, "%sRtProto %u\n",
      cnf->rt_proto == DEF_RTPROTO ? "# " : "",
      cnf->rt_proto);
  abuf_puts(out,
    "\n"
    "# Specifies the routing Table olsr uses\n"
    "# RtTable is for host routes, RtTableDefault for the route to the default\n"
    "# internet gateway (2 in case of IPv6+NIIT) and RtTableTunnel is for\n"
    "# routes to the ipip tunnels, valid values are 1 to 254\n"
    "# There is a special parameter \"auto\" (choose default below)\n"
    "# (with smartgw: default is 254/223/224)\n"
    "# (without smartgw: default is 254/254/254, linux main table)\n"
    "\n");
  abuf_appendf(out, "RtTable %u\n",
      cnf->rt_table);
  abuf_appendf(out, "RtTableDefault %u\n",
      cnf->rt_table_default);
  abuf_appendf(out, "RtTableTunnel %u\n",
      cnf->rt_table_tunnel);
  abuf_puts(out,
    "\n"
    "# Specifies the policy rule priorities for the three routing tables and\n"
    "# a special rule for smartgateway routing (see README-Olsr-Extensions)\n"
    "# Priorities can only be set if three different routing tables are set.\n"
    "# if set the values must obey to condition\n"
    "# RtTablePriority < RtTableDefaultOlsrPriority\n"
    "# < RtTableTunnelPriority < RtTableDefaultPriority\n"
    "# There are two special parameters, \"auto\" (choose fitting to SmartGW\n"
    "# mode) and \"none\" (do not set policy rule)\n"
    "# (with smartgw: default is none/32776/32786/32796)\n"
    "# (without smartgw: default is none/none/none/none)\n"
    "\n");
  abuf_appendf(out, "RtTablePriority %u\n",
      cnf->rt_table_pri);
  abuf_appendf(out, "RtTableDefaultOlsrPriority %u\n",
      cnf->rt_table_default_pri);
  abuf_appendf(out, "RtTableTunnelPriority %u\n",
      cnf->rt_table_tunnel_pri);
  abuf_appendf(out, "RtTableDefaultPriority %u\n",
      cnf->rt_table_defaultolsr_pri);
  abuf_puts(out,
    "\n"
    "# Activates (in IPv6 mode) the automatic use of NIIT\n"
    "# (see README-Olsr-Extensions)\n"
    "# (default is \"yes\")\n"
    "\n");
  abuf_appendf(out, "%sUseNiit %s\n",
      cnf->use_niit == DEF_USE_NIIT ? "# " : "",
      cnf->use_niit ? "yes" : "no");
  abuf_puts(out,
    "\n"
    "# Activates the smartgateway ipip tunnel feature.\n"
    "# See README-Olsr-Extensions for a description of smartgateways.\n"
    "# (default is \"yes\")\n"
    "\n");
  abuf_appendf(out, "%sSmartGateway %s\n",
      cnf->smart_gw_active == DEF_SMART_GW ? "# " : "",
      cnf->smart_gw_active ? "yes" : "no");
  abuf_puts(out,
    "\n"
    "# Allows the selection of a smartgateway with NAT (only for IPv4)\n"
    "# (default is \"yes\")\n"
    "\n");
  abuf_appendf(out, "%sSmartGatewayAllowNAT %s\n",
      cnf->smart_gw_allow_nat == DEF_GW_ALLOW_NAT ? "# " : "",
      cnf->smart_gw_allow_nat ? "yes" : "no");
  abuf_puts(out,
    "\n"
    "# Defines what kind of Uplink this node will publish as a\n"
    "# smartgateway. The existence of the uplink is detected by\n"
    "# a route to 0.0.0.0/0, ::ffff:0:0/96 and/or 2000::/3.\n"
    "# possible values are \"none\", \"ipv4\", \"ipv6\", \"both\"\n"
    "# (default is \"both\")\n"
    "\n");
  abuf_appendf(out, "%sSmartGatewayUplink \"%s\"\n",
      cnf->smart_gw_type == DEF_GW_TYPE ? "# " : "",
      GW_UPLINK_TXT[cnf->smart_gw_type]);
  abuf_puts(out,
    "\n"
    "# Specifies if the local ipv4 uplink use NAT\n"
    "# (default is \"yes\")\n"
    "\n");
  abuf_appendf(out, "%sSmartGatewayUplinkNAT %s\n",
      cnf->smart_gw_uplink_nat == DEF_GW_UPLINK_NAT ? "# " : "",
      cnf->smart_gw_uplink_nat ? "yes" : "no");
  abuf_puts(out,
    "\n"
    "# Specifies the speed of the uplink in kilobit/s.\n"
    "# First parameter is upstream, second parameter is downstream\n"
    "# (default is 128/1024)\n"
    "\n");
  abuf_appendf(out, "%sSmartGatewaySpeed %d %d\n",
      cnf->smart_gw_uplink == DEF_UPLINK_SPEED && cnf->smart_gw_downlink == DEF_DOWNLINK_SPEED ? "# " : "",
      cnf->smart_gw_uplink, cnf->smart_gw_downlink);
  abuf_puts(out,
    "\n"
    "# Specifies the EXTERNAL ipv6 prefix of the uplink. A prefix\n"
    "# length of more than 64 is not allowed.\n"
    "# (default is 0::/0\n"
    "\n");
  abuf_appendf(out, "%sSmartGatewayPrefix %s\n",
      cnf->smart_gw_prefix.prefix_len == 0 ? "# " : "",
      olsr_ip_prefix_to_string(&cnf->smart_gw_prefix));
  abuf_puts(out,
    "\n"
    "##############################\n"
    "### OLSR protocol settings ###\n"
    "##############################\n"
    "\n"
    "# For testing purposes it may be nice to use another port for olsrd\n"
    "# for using another port than the IANA assigned one \n"
    "# for a production network, there should be a good reason!!\n"
    "# valid values are integers >1, please be careful with using reserved\n"
    "# port numbers\n"
    "# (default is 698, the IANA assigned olsr-port)\n"
    "\n");
  abuf_appendf(out, "%sOlsrPort %u\n",
      cnf->olsrport == DEF_OLSRPORT ? "# " : "",
      cnf->olsrport);
  abuf_puts(out,
    "\n"
    "# Sets the main IP (originator ip) of the router. This IP will NEVER\n"
    "# change during the uptime of olsrd.\n"
    "# (default is 0.0.0.0, which triggers usage of the IP of the first interface)\n"
    "\n");
  abuf_appendf(out, "MainIp %s\n",
      olsr_ip_to_string(&ipbuf, &cnf->main_addr));
  abuf_puts(out,
    "\n"
    "# The fixed willingness to use (0-7)\n"
    "# If not set willingness will be calculated\n"
    "# dynamically based on battery/power status\n"
    "# (default is 3)\n"
    "\n");
  abuf_appendf(out, "%sWillingness     %u\n",
      cnf->willingness == DEF_WILLINGNESS ? "# " : "",
      cnf->willingness);
  abuf_puts(out,
    "\n"
    "# HNA (Host network association) allows the OLSR to announce\n"
    "# additional IPs or IP subnets to the net that are reachable\n"
    "# through this node.\n"
    "# Syntax for HNA4 is \"network-address    network-mask\"\n"
    "# Syntax for HNA6 is \"network-address    prefix-length\"\n"
    "# (default is no HNA)\n");
  abuf_appendf(out, "Hna%u\n"
    "{\n",
    cnf->ip_version == AF_INET ? 4 : 6);
  while (hna) {
    struct ipaddr_str strbuf;
    abuf_appendf(out, "    %s\n", olsr_ip_prefix_to_string(&hna->net));
    hna = hna->next;
  }
  abuf_puts(out,
    "}\n"
    "\n"
    "# Hysteresis for link sensing (only for hopcount metric)\n"
    "# Hysteresis adds more robustness to the link sensing\n"
    "# but delays neighbor registration.\n"
    "# (defaults to yes)\n"
    "\n");
  abuf_appendf(out, "%sUseHysteresis %s\n",
      cnf->use_hysteresis == DEF_USE_HYST ? "# " : "",
      cnf->use_hysteresis ? "yes" : "no");
  abuf_puts(out,
    "\n"
    "# Hysteresis parameters (only for hopcount metric)\n"
    "# Do not alter these unless you know what you are doing!\n"
    "# Set to auto by default. Allowed values are floating point\n"
    "# values in the interval 0,1\n"
    "# THR_LOW must always be lower than THR_HIGH!!\n"
    "# (default is 0.5/0.8/0.3)\n"
    "\n");
  abuf_appendf(out, "%sHystScaling  %.2f\n",
      cnf->hysteresis_param.scaling == HYST_SCALING ? "# " : "",
      cnf->hysteresis_param.scaling);
  abuf_appendf(out, "%sHystThrHigh  %.2f\n",
      cnf->hysteresis_param.thr_high == HYST_THRESHOLD_HIGH ? "# " : "",
      cnf->hysteresis_param.thr_high);
  abuf_appendf(out, "%sHystThrLow  %.2f\n",
      cnf->hysteresis_param.thr_low == HYST_THRESHOLD_LOW ? "# " : "",
      cnf->hysteresis_param.thr_low);
  abuf_puts(out,
    "\n"
    "# TC redundancy\n"
    "# Specifies how much neighbor info should be sent in\n"
    "# TC messages. Because of a design problem in the 0.5.x\n"
    "# dijkstra implementation this value must be set to 2.\n"
    "# 2 - send all neighbors\n"
    "# (default is 2)\n"
    "\n");
  abuf_appendf(out, "%sTcRedundancy  %d\n",
      cnf->tc_redundancy == TC_REDUNDANCY ? "# " : "",
      cnf->tc_redundancy);
  abuf_puts(out,
    "\n"
    "# MPR coverage specifies how many MPRs a node should\n"
    "# try select to reach every 2 hop neighbor. Because of\n"
    "# a design problem in the 0.5.x dijkstra algorithm this\n"
    "# value should be set to 7.\n"
    "# (default is 7)\n"
    "\n");
  abuf_appendf(out, "%sMprCoverage %d\n",
      cnf->mpr_coverage == MPR_COVERAGE ? "# " : "",
      cnf->mpr_coverage);
  abuf_puts(out,
    "\n"
    "################################\n"
    "### OLSR protocol extensions ###\n"
    "################################\n"
    "\n"
    "# Link quality level switch between hopcount and \n"
    "# cost-based (mostly ETX) routing. Because of\n"
    "# a design problem in the 0.5.x dijkstra algorithm this\n"
    "# value should not be set to 1.\n"
    "# 0 = do not use link quality\n"
    "# 2 = use link quality for MPR selection and routing\n"
    "# (default is 2)\n"
    "\n");
  abuf_appendf(out, "%sLinkQualityLevel %d\n",
      cnf->lq_level == DEF_LQ_LEVEL ? "# " : "",
      cnf->lq_level);
  abuf_puts(out,
    "\n"
    "# Link quality algorithm (only for lq level 2)\n"
    "# (see README-Olsr-Extensions)\n"
    "# - \"etx_float\", a floating point  ETX with exponential aging\n"
    "# - \"etx_fpm\", same as ext_float, but with integer arithmetic\n"
    "# - \"etx_ff\" (ETX freifunk), an etx variant which use all OLSR\n"
    "#   traffic (instead of only hellos) for ETX calculation\n"
    "# - \"etx_ffeth\", an incompatible variant of etx_ff that allows\n"
    "#   ethernet links with ETX 0.1.\n"
    "# (defaults to \"etx_ff\")\n"
    "\n");
  abuf_appendf(out, "%sLinkQualityAlgorithm    \"%s\"\n",
      cnf->lq_algorithm == NULL ? "# " : "",
      cnf->lq_algorithm == NULL ? DEF_LQ_ALGORITHM : cnf->lq_algorithm);
  abuf_puts(out,
    "\n"
    "# Link quality aging factor (only for lq level 2)\n"
    "# Tuning parameter for etx_float and etx_fpm, smaller values\n"
    "# mean slower changes of ETX value. (allowed values are\n"
    "# between 0.01 and 1.0)\n"
    "# (default is 0.05)\n"
    "\n");
  abuf_appendf(out, "%sLinkQualityAging %.2f\n",
      cnf->lq_aging == DEF_LQ_AGING ? "# " : "",
      cnf->lq_aging);
  abuf_puts(out,
    "\n"
    "# Fisheye mechanism for TCs (0 meansoff, 1 means on)\n"
    "# (default is 1)\n"
    "\n");
  abuf_appendf(out, "%sLinkQualityFishEye  %d\n",
      cnf->lq_fish == DEF_LQ_FISH ? "# " : "",
      cnf->lq_fish);
  abuf_puts(out,
    "\n"
    "#\n"
    "# NatThreshold \n"
    "#\n"
    "# (currently this is only in the freifunk firmware)\n"
    "# If the NAT-Endpoint (the preferred 0/0 HNA emitting node)\n"
    "# is to be changed, the ETX value of the current 0/0 is \n"
    "# multiplied with the NatThreshold value before being\n"
    "# compared to the new one.\n"
    "# The parameter can be a value between 0.1 and 1.0, but\n"
    "# should be close to 1.0 if changed.\n"
    "# WARNING: This parameter should not be used together with\n"
    "# the etx_ffeth metric !!\n"
    "# (defaults to 1.0)\n"
    "\n");
  abuf_appendf(out, "%sNatThreshold  %.1f\n",
      cnf->lq_nat_thresh == DEF_LQ_NAT_THRESH ? "# " : "",
      cnf->lq_nat_thresh);

  abuf_puts(out,
    "\n"
    "#############################################################\n"
    "### Configuration of the IPC to the windows GUI interface ###\n"
    "#############################################################\n"
    "\n"
    "IpcConnect\n"
    "{\n"
    "    # Determines how many simultaneously\n"
    "    # IPC connections that will be allowed\n"
    "    # Setting this to 0 disables IPC\n"
    "\n");
  abuf_appendf(out, "  %sMaxConnections  %d\n",
      cnf->ipc_connections == DEF_IPC_CONNECTIONS ? "# " : "",
      cnf->ipc_connections);
  abuf_puts(out,
    "\n"
    "    # By default only 127.0.0.1 is allowed\n"
    "    # to connect. Here allowed hosts and networks can\n"
    "    # be added\n"
    "\n");

  while (ipc_nets) {
    if (ipc_nets->net.prefix_len == olsr_cnf->maxplen) {
      abuf_appendf(out, "    Host %s\n", olsr_ip_to_string(&ipbuf, &ipc_nets->net.prefix));
    } else {
      abuf_appendf(out, "    Net  %s\n", olsr_ip_prefix_to_string(&ipc_nets->net));
    }
    ipc_nets = ipc_nets->next;
  }
  abuf_puts(out,
    "}\n"
    "\n"
    "#####################################\n"
    "### Example plugin configurations ###\n"
    "#####################################\n"
    "# Olsrd plugins to load\n"
    "# This must be the absolute path to the file\n"
    "# or the loader will use the following scheme:\n"
    "# - Try the paths in the LD_LIBRARY_PATH \n"
    "#   environment variable.\n"
    "# - The list of libraries cached in /etc/ld.so.cache\n"
    "# - /lib, followed by /usr/lib\n"
    "\n");

  while (plugins) {
    abuf_appendf(out, "LoadPlugin \"%s\" {\n", plugins->name);
    pl_param = plugins->params;
    while (pl_param) {
      abuf_appendf(out, "    PlParam \"%s\"\t\"%s\"\n", pl_param->key, pl_param->value);
      pl_param = pl_param->next;
    }
    abuf_puts(out, "}\n"
        "\n");
    plugins = plugins->next;
  }

  abuf_puts(out,
    "#############################################\n"
    "### OLSRD default interface configuration ###\n"
    "#############################################\n"
    "# the default interface section can have the same values as the following\n"
    "# interface configuration. It will allow you so set common options for all\n"
    "# interfaces.\n"
    "\n"
    "InterfaceDefaults\n");
  olsrd_write_if_autobuf(out, cnf->interface_defaults, false);
  abuf_puts(out,
    "\n"
    "######################################\n"
    "### OLSRd Interfaces configuration ###\n"
    "######################################\n"
    "# multiple interfaces can be specified for a single configuration block\n"
    "# multiple configuration blocks can be specified\n"
    "\n");
  first = true;
  while (interf) {
    abuf_appendf(out, "Interface \"%s\"\n", interf->name);
    olsrd_write_if_autobuf(out, interf->cnf, first);

    first = false;
    interf = interf->next;
  }

  abuf_puts(out,
      "\n"
      "# END AUTOGENERATED CONFIG\n");
}
/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
