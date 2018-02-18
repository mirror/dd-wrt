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
 * File       : Address.c
 * Description: IP packet characterization functions
 * Created    : 29 Jun 2006
 *
 * ------------------------------------------------------------------------- */
 
#include "Address.h"

/* System includes */
#include <stddef.h> /* NULL */
#include <assert.h> /* assert() */
#include <netinet/ip.h> /* struct ip */
#include <netinet/udp.h> /* struct udphdr */

/* OLSRD includes */
#include "defs.h" /* ipequal */

/* Plugin includes */
#include "Bmf.h" /* BMF_ENCAP_PORT */
#include "NetworkInterfaces.h" /* TBmfInterface */

/* Whether or not to flood local broadcast packets (e.g. packets with IP
 * destination 192.168.1.255). May be overruled by setting the plugin
 * parameter "DoLocalBroadcast" to "no" */
int EnableLocalBroadcast = 1;

/* -------------------------------------------------------------------------
 * Function   : DoLocalBroadcast
 * Description: Overrule the default setting, enabling or disabling the
 *              flooding of local broadcast packets
 * Input      : enable - either "yes" or "no"
 *              data - not used
 *              addon - not used
 * Output     : none
 * Return     : success (0) or fail (1)
 * Data Used  : none
 * ------------------------------------------------------------------------- */
int DoLocalBroadcast(
  const char* enable,
  void* data __attribute__((unused)),
  set_plugin_parameter_addon addon __attribute__((unused)))
{
  if (strcmp(enable, "yes") == 0)
  {
    EnableLocalBroadcast = 1;
    return 0;
  }
  else if (strcmp(enable, "no") == 0)
  {
    EnableLocalBroadcast = 0;
    return 0;
  }

  /* Value not recognized */
  return 1;
}

/* -------------------------------------------------------------------------
 * Function   : IsMulticast
 * Description: Check if an IP address is a multicast address
 * Input      : ipAddress
 * Output     : none
 * Return     : true (1) or false (0)
 * Data Used  : none
 * ------------------------------------------------------------------------- */
int IsMulticast(union olsr_ip_addr* ipAddress)
{
  assert(ipAddress != NULL);

  return IN_MULTICAST(ntohl(ipAddress->v4.s_addr));
}

/* -------------------------------------------------------------------------
 * Function   : IsOlsrOrBmfPacket
 * Description: Check if an IP packet is either an OLSR packet or a BMF packet
 * Input      : ipPacket
 * Output     : none
 * Return     : true (1) or false (0)
 * Data Used  : none
 * ------------------------------------------------------------------------- */
int IsOlsrOrBmfPacket(unsigned char* ipPacket)
{
  struct ip* ipHeader;
  unsigned int ipHeaderLen;
  struct udphdr* udpHeader;
  u_int16_t destPort;

  assert(ipPacket != NULL);

  /* OLSR packets are UDP - port 698
   * OLSR-BMF packets are UDP - port 50698
   * OLSR-Autodetect probe packets are UDP - port 51698 */

  /* Check if UDP */
  ipHeader = (struct ip*) ARM_NOWARN_ALIGN(ipPacket);
  if (ipHeader->ip_p != SOL_UDP)
  {
    /* Not UDP */
    return 0;
  }

  /* The total length must be at least large enough to store the UDP header */
  ipHeaderLen = GetIpHeaderLength(ipPacket);
  if (GetIpTotalLength(ipPacket) < ipHeaderLen + sizeof(struct udphdr))
  {
    /* Not long enough */
    return 0;
  }

  /* Go into the UDP header and check port number */
  udpHeader = (struct udphdr*) ARM_NOWARN_ALIGN((ipPacket + ipHeaderLen));
#if defined(__GLIBC__) || defined(__BIONIC__)
  destPort = ntohs(udpHeader->dest);
#else
  destPort = ntohs(udpHeader->uh_dport);
#endif

  if (destPort == olsr_cnf->olsrport || destPort == BMF_ENCAP_PORT || destPort == 51698)
      /* TODO: #define for 51698 */
  {
    return 1;
  }

  return 0;
}
