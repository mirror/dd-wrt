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
 * File       : NetworkInterfaces.c
 * Description: Functions to open and close sockets
 * Created    : 29 Jun 2006
 *
 * ------------------------------------------------------------------------- */

#include "NetworkInterfaces.h"

/* System includes */
#include <stddef.h> /* NULL */
#include <syslog.h> /* syslog() */
#include <string.h> /* strerror(), strchr(), strcmp() */
#include <errno.h> /* errno */
#include <unistd.h> /* close() */
#include <sys/ioctl.h> /* ioctl() */
#include <fcntl.h> /* fcntl() */
#include <assert.h> /* assert() */
#include <net/if.h> /* socket(), ifreq, if_indextoname(), if_nametoindex() */
#include <netinet/in.h> /* htons() */
#include <netinet/udp.h> /* struct udphdr */
#include <linux/if_ether.h> /* ETH_P_IP */
#include <linux/if_packet.h> /* packet_mreq, PACKET_MR_PROMISC, PACKET_ADD_MEMBERSHIP */
#include <linux/if_tun.h> /* IFF_TAP */
#include <netinet/ip.h> /* struct ip */
#include <netinet/udp.h> /* SOL_UDP */

/* OLSRD includes */
#include "olsr.h" /* olsr_printf() */
#include "ipcalc.h"
#include "defs.h" /* olsr_cnf */
#include "link_set.h" /* get_link_set() */
#include "tc_set.h" /* olsr_lookup_tc_entry(), olsr_lookup_tc_edge() */
#include "net_olsr.h" /* ipequal */
#include "lq_plugin.h"
#include "kernel_tunnel.h"

/* Plugin includes */
#include "Packet.h" /* IFHWADDRLEN */
#include "Bmf.h" /* PLUGIN_NAME, MainAddressOf() */
#include "Address.h" /* IsMulticast() */

/* List of network interface objects used by BMF plugin */
struct TBmfInterface* BmfInterfaces = NULL;
struct TBmfInterface* LastBmfInterface = NULL;

/* Highest-numbered open socket file descriptor. To be used as first
 * parameter in calls to select(...). */
int HighestSkfd = -1;

/* Set of socket file descriptors */
fd_set InputSet;

/* File descriptor of EtherTunTap interface */
int EtherTunTapFd = -1;

/* Network interface name of EtherTunTap interface. May be overruled by
 * setting the plugin parameter "BmfInterface". */
char EtherTunTapIfName[IFNAMSIZ] = "bmf0";

/* The underlying mechanism to forward multicast packets. Either:
 * - BM_BROADCAST: BMF uses the IP local broadcast as destination address
 * - BM_UNICAST_PROMISCUOUS: BMF uses the IP address of the best neighbor as
 *   destination address. The other neighbors listen promiscuously. */
enum TBmfMechanism BmfMechanism = BM_BROADCAST;

#define ETHERTUNTAPIPNOTSET 0

/* The IP address of the BMF network interface in host byte order.
 * May be overruled by setting the plugin parameter "BmfInterfaceIp". */
u_int32_t EtherTunTapIp = ETHERTUNTAPIPNOTSET;

/* 255.255.255.255 in host byte order. May be overruled by
 * setting the plugin parameter "BmfInterfaceIp". */
u_int32_t EtherTunTapIpMask = 0xFFFFFFFF;

/* The IP broadcast address of the BMF network interface in host byte order.
 * May be overruled by setting the plugin parameter "BmfinterfaceIp". */
u_int32_t EtherTunTapIpBroadcast = ETHERTUNTAPIPNOTSET;

/* Whether or not the BMF network interface must be marked as persistent */
int EtherTunTapPersistent = 1;

/* Whether or not the configuration has overruled the default IP
 * configuration of the EtherTunTap interface */
int TunTapIpOverruled = 0;

/* Whether or not to capture packets on the OLSR-enabled
 * interfaces (in promiscuous mode). May be overruled by setting the plugin
 * parameter "CapturePacketsOnOlsrInterfaces" to "yes". */
int CapturePacketsOnOlsrInterfaces = 0;

/* -------------------------------------------------------------------------
 * Function   : SetBmfInterfaceName
 * Description: Overrule the default network interface name ("bmf0") of the
 *              EtherTunTap interface
 * Input      : ifname - network interface name (e.g. "mybmf0")
 *              data - not used
 *              addon - not used
 * Output     : none
 * Return     : success (0) or fail (1)
 * Data Used  : EtherTunTapIfName
 * ------------------------------------------------------------------------- */
int SetBmfInterfaceName(
  const char* ifname,
  void* data __attribute__((unused)),
  set_plugin_parameter_addon addon __attribute__((unused)))
{
  strncpy(EtherTunTapIfName, ifname, IFNAMSIZ - 1);
  EtherTunTapIfName[IFNAMSIZ - 1] = '\0'; /* Ensures null termination */
  return 0;
} /* SetBmfInterfaceName */

/* -------------------------------------------------------------------------
 * Function   : SetBmfInterfaceIp
 * Description: Overrule the default IP address and prefix length
 *              ("10.255.255.253/30") of the EtherTunTap interface
 * Input      : ip - IP address string, followed by '/' and prefix length
 *              data - not used
 *              addon - not used
 * Output     : none
 * Return     : success (0) or fail (1)
 * Data Used  : EtherTunTapIp, EtherTunTapIpMask, EtherTunTapIpBroadcast,
 *              TunTapIpOverruled
 * ------------------------------------------------------------------------- */
int SetBmfInterfaceIp(
  const char* ip,
  void* data __attribute__((unused)),
  set_plugin_parameter_addon addon __attribute__((unused)))
{
#define IPV4_MAX_ADDRLEN 16
#define IPV4_MAX_PREFIXLEN 32
  char* slashAt;
  char ipAddr[IPV4_MAX_ADDRLEN];
  struct in_addr sinaddr;
  int prefixLen;
  int i;

  /* Inspired by function str2prefix_ipv4 as found in Quagga source
   * file lib/prefix.c */

  /* Find slash inside string. */
  slashAt = strchr(ip, '/');

  /* String doesn't contain slash. */
  if (slashAt == NULL || slashAt - ip >= IPV4_MAX_ADDRLEN)
  {
    /* No prefix length specified, or IP address too long */
    return 1;
  }

  strncpy(ipAddr, ip, slashAt - ip);
  *(ipAddr + (slashAt - ip)) = '\0';
  if (inet_aton(ipAddr, &sinaddr) == 0)
  {
    /* Invalid address passed */
    return 1;
  }

  EtherTunTapIp = ntohl(sinaddr.s_addr);

  /* Get prefix length. */
  prefixLen = atoi(++slashAt);
  if (prefixLen <= 0 || prefixLen > IPV4_MAX_PREFIXLEN)
  {
    return 1;
  }

  /* Compose IP subnet mask in host byte order */
  EtherTunTapIpMask = 0;
  for (i = 0; i < prefixLen; i++)
  {
    EtherTunTapIpMask |= (1u << (IPV4_MAX_PREFIXLEN - 1 - i));
  }

  /* Compose IP broadcast address in host byte order */
  EtherTunTapIpBroadcast = EtherTunTapIp;
  for (i = prefixLen; i < IPV4_MAX_PREFIXLEN; i++)
  {
    EtherTunTapIpBroadcast |= (1u << (IPV4_MAX_PREFIXLEN - 1 - i));
  }

  TunTapIpOverruled = 1;

  return 0;
} /* SetBmfInterfaceIp */

/* -------------------------------------------------------------------------
 * Function   : SetBmfInterfacePersistent
 * Description: Determine if the EtherTunTap interface must be marked
 *              persistent or not
 * Input      : value - yes/true/1  or no/false/0
 *              data  - not used
 *              addon - not used
 * Output     : none
 * Return     : success (0) or fail (1)
 * Data Used  : EtherTunTapPersistent
 * ------------------------------------------------------------------------- */
int SetBmfInterfacePersistent(
  const char* value,
  void* data __attribute__((unused)),
  set_plugin_parameter_addon addon __attribute__((unused)))
{
	if (strcasecmp(value, "yes") == 0 || strcasecmp(value, "true") == 0 || strcasecmp(value, "1") == 0) {
		EtherTunTapPersistent = 1;
	} else if (strcasecmp(value, "no") == 0 || strcasecmp(value, "false") == 0 || strcasecmp(value, "0") == 0) {
		EtherTunTapPersistent = 0;
	} else {
		return 1;
	}
	return 0;
} /* SetBmfInterfacePersistent */

/* -------------------------------------------------------------------------
 * Function   : SetCapturePacketsOnOlsrInterfaces
 * Description: Overrule the default setting, enabling or disabling the
 *              capturing of packets on OLSR-enabled interfaces.
 * Input      : enable - either "yes" or "no"
 *              data - not used
 *              addon - not used
 * Output     : none
 * Return     : success (0) or fail (1)
 * Data Used  : none
 * ------------------------------------------------------------------------- */
int SetCapturePacketsOnOlsrInterfaces(
  const char* enable,
  void* data __attribute__((unused)),
  set_plugin_parameter_addon addon __attribute__((unused)))
{
  if (strcmp(enable, "yes") == 0)
  {
    CapturePacketsOnOlsrInterfaces = 1;
    return 0;
  }
  else if (strcmp(enable, "no") == 0)
  {
    CapturePacketsOnOlsrInterfaces = 0;
    return 0;
  }

  /* Value not recognized */
  return 1;
} /* SetCapturePacketsOnOlsrInterfaces */

/* -------------------------------------------------------------------------
 * Function   : SetBmfMechanism
 * Description: Overrule the default BMF mechanism to either BM_BROADCAST or
 *              BM_UNICAST_PROMISCUOUS.
 * Input      : mechanism - either "Broadcast" or "UnicastPromiscuous"
 *              data - not used
 *              addon - not used
 * Output     : none
 * Return     : success (0) or fail (1)
 * Data Used  : none
 * ------------------------------------------------------------------------- */
int SetBmfMechanism(
  const char* mechanism,
  void* data __attribute__((unused)),
  set_plugin_parameter_addon addon __attribute__((unused)))
{
  if (strcasecmp(mechanism, "Broadcast") == 0)
  {
    BmfMechanism = BM_BROADCAST;
    return 0;
  }
  else if (strcasecmp(mechanism, "UnicastPromiscuous") == 0)
  {
    BmfMechanism = BM_UNICAST_PROMISCUOUS;
    return 0;
  }

  /* Value not recognized */
  return 1;
} /* SetBmfMechanism */

/* -------------------------------------------------------------------------
 * Function   : AddDescriptorToInputSet
 * Description: Add a socket descriptor to the global set of socket file descriptors
 * Input      : skfd - socket file descriptor
 * Output     : none
 * Return     : none
 * Data Used  : HighestSkfd, InputSet
 * Notes      : Keeps track of the highest-numbered descriptor
 * ------------------------------------------------------------------------- */
static void AddDescriptorToInputSet(int skfd)
{
  /* Keep the highest-numbered descriptor */
  if (skfd > HighestSkfd)
  {
    HighestSkfd = skfd;
  }

  /* Add descriptor to input set */
  FD_SET(skfd, &InputSet);
} /* AddDescriptorToInputSet */

/* To save the state of the IP spoof filter for the EtherTunTap interface */
static char EthTapSpoofState = '1';

/* -------------------------------------------------------------------------
 * Function   : DeactivateSpoofFilter
 * Description: Deactivates the Linux anti-spoofing filter for the tuntap
 *              interface
 * Input      : none
 * Output     : none
 * Return     : fail (0) or success (1)
 * Data Used  : EtherTunTapIfName, EthTapSpoofState
 * Notes      : Saves the current filter state for later restoring
 * ------------------------------------------------------------------------- */
int DeactivateSpoofFilter(void)
{
  FILE* procSpoof;
  char procFile[FILENAME_MAX];

  /* Generate the procfile name */
  sprintf(procFile, "/proc/sys/net/ipv4/conf/%s/rp_filter", EtherTunTapIfName);

  /* Open procfile for reading */
  procSpoof = fopen(procFile, "r");
  if (procSpoof == NULL)
  {
    fprintf(
      stderr,
      "WARNING! Could not open the %s file to check/disable the IP spoof filter!\n"
      "Are you using the procfile filesystem?\n"
      "Does your system support IPv4?\n"
      "I will continue (in 3 sec) - but you should manually ensure that IP spoof\n"
      "filtering is disabled!\n\n",
      procFile);
      
    sleep(3);
    return 0;
  }

  EthTapSpoofState = (char)fgetc(procSpoof);
  fclose(procSpoof);

  /* Open procfile for writing */
  procSpoof = fopen(procFile, "w");
  if (procSpoof == NULL)
  {
    fprintf(stderr, "Could not open %s for writing!\n", procFile);
    fprintf(
      stderr,
      "I will continue (in 3 sec) - but you should manually ensure that IP"
      " spoof filtering is disabled!\n\n");
    sleep(3);
    return 0;
  }

  syslog(LOG_INFO, "Writing \"0\" to %s", procFile);
  fputs("0", procSpoof);

  fclose(procSpoof);

  return 1;
} /* DeactivateSpoofFilter */

/* -------------------------------------------------------------------------
 * Function   : RestoreSpoofFilter
 * Description: Restores the Linux anti-spoofing filter setting for the tuntap
 *              interface
 * Input      : none
 * Output     : none
 * Return     : none
 * Data Used  : EtherTunTapIfName, EthTapSpoofState
 * ------------------------------------------------------------------------- */
void RestoreSpoofFilter(void)
{
  FILE* procSpoof;
  char procFile[FILENAME_MAX];

  /* Generate the procfile name */
  sprintf(procFile, "/proc/sys/net/ipv4/conf/%s/rp_filter", EtherTunTapIfName);

  /* Open procfile for writing */
  procSpoof = fopen(procFile, "w");
  if (procSpoof == NULL)
  {
    fprintf(stderr, "Could not open %s for writing!\nSettings not restored!\n", procFile);
  }
  else
  {
    syslog(LOG_INFO, "Resetting %s to %c\n", procFile, EthTapSpoofState);

    fputc(EthTapSpoofState, procSpoof);
    fclose(procSpoof);
  }
} /* RestoreSpoofFilter */

/* -------------------------------------------------------------------------
 * Function   : FindNeighbors
 * Description: Find the neighbors on a network interface to forward a BMF
 *              packet to
 * Input      : intf - the network interface
 *              source - the source IP address of the BMF packet, or NULL if
 *                unknown or not applicable
 *              forwardedBy - the IP address of the node that forwarded the BMF
 *                packet, or NULL if unknown or not applicable
 *              forwardedTo - the IP address of the node to which the BMF packet
 *                was directed, or NULL if unknown or not applicable
 * Output     : neighbors - list of (up to a number of 'FanOutLimit') neighbors.
 *              bestNeighbor - the best neighbor (in terms of lowest cost or ETX
 *                value)
 *              nPossibleNeighbors - number of found possible neighbors
 * Data Used  : FanOutLimit
 * ------------------------------------------------------------------------- */
void FindNeighbors(
  struct TBestNeighbors* neighbors,
  struct link_entry** bestNeighbor,
  struct TBmfInterface* intf,
  union olsr_ip_addr* source,
  union olsr_ip_addr* forwardedBy,
  union olsr_ip_addr* forwardedTo,
  int* nPossibleNeighbors)
{
#ifndef NODEBUG
  struct ipaddr_str buf;
#endif /* NODEBUG */
  int i;

  /* Initialize */
  *bestNeighbor = NULL;
  for (i = 0; i < MAX_UNICAST_NEIGHBORS; i++)
  {
    neighbors->links[i] = NULL;
  }
  *nPossibleNeighbors = 0;

  /* handle the non-LQ case */

  if (olsr_cnf->lq_level == 0)
  {
    struct link_entry* walker;

    OLSR_FOR_ALL_LINK_ENTRIES(walker) {
      union olsr_ip_addr* neighborMainIp;

      /* Consider only links from the specified interface */
      if (! ipequal(&intf->intAddr, &walker->local_iface_addr))
      {
        continue; /* for */
      }

      OLSR_PRINTF(
        8,
        "%s: ----> considering forwarding pkt on \"%s\" to %s\n",
        PLUGIN_NAME_SHORT,
        intf->ifName,
        olsr_ip_to_string(&buf, &walker->neighbor_iface_addr));

      neighborMainIp = MainAddressOf(&walker->neighbor_iface_addr);

      /* Consider only neighbors with an IP address that differs from the
       * passed IP addresses (if passed). Rely on short-circuit boolean evaluation. */
      if (source != NULL && ipequal(neighborMainIp, MainAddressOf(source)))
      {
        OLSR_PRINTF(
          9,
          "%s: ----> not forwarding to %s: is source of pkt\n",
          PLUGIN_NAME_SHORT,
          olsr_ip_to_string(&buf, &walker->neighbor_iface_addr));

        continue; /* for */
      }

      /* Rely on short-circuit boolean evaluation */
      if (forwardedBy != NULL && ipequal(neighborMainIp, MainAddressOf(forwardedBy)))
      {
        OLSR_PRINTF(
          9,
          "%s: ----> not forwarding to %s: is the node that forwarded the pkt\n",
          PLUGIN_NAME_SHORT,
          olsr_ip_to_string(&buf, &walker->neighbor_iface_addr));

        continue; /* for */
      }

      /* Rely on short-circuit boolean evaluation */
      if (forwardedTo != NULL && ipequal(neighborMainIp, MainAddressOf(forwardedTo)))
      {
        OLSR_PRINTF(
          9,
          "%s: ----> not forwarding to %s: is the node to which the pkt was forwarded\n",
          PLUGIN_NAME_SHORT,
          olsr_ip_to_string(&buf, &walker->neighbor_iface_addr));

        continue; /* for */
      }

      /* Found a candidate neighbor to direct our packet to */

      /* In the non-LQ case, it is not possible to select neigbors
       * by quality or cost. So just remember the first found link.
       * TODO: come on, there must be something better than to simply
       * select the first one found! */
      if (*bestNeighbor == NULL)
      {
        *bestNeighbor = walker;
      }

      /* Fill the list with up to 'FanOutLimit' neighbors. If there
       * are more neighbors, broadcast is used instead of unicast. In that
       * case we do not need the list of neighbors. */
      if (*nPossibleNeighbors < FanOutLimit)
      {
        neighbors->links[*nPossibleNeighbors] = walker;
      }

      *nPossibleNeighbors += 1;
    } OLSR_FOR_ALL_LINK_ENTRIES_END(walker);

  }
  /* handle the LQ case */
  else
  {
#ifdef USING_THALES_LINK_COST_ROUTING

    struct link_entry* walker;
    float previousLinkCost = 2 * INFINITE_COST;
    float bestLinkCost = 2 * INFINITE_COST;

    if (forwardedBy != NULL)
    {
      /* Retrieve the cost of the link from 'forwardedBy' to myself */
      struct link_entry* bestLinkFromForwarder = get_best_link_to_neighbor(forwardedBy);
      if (bestLinkFromForwarder != NULL)
      {
        previousLinkCost = bestLinkFromForwarder->link_cost;
      }
    }

    for (walker = get_link_set(); walker != NULL; walker = walker->next) 
    {
      union olsr_ip_addr* neighborMainIp;
      struct link_entry* bestLinkToNeighbor;
      struct tc_entry* tcLastHop;

      /* Consider only links from the specified interface */
      if (! ipequal(&intf->intAddr, &walker->local_iface_addr))
      {
        continue; /* for */
      }

      OLSR_PRINTF(
        9,
        "%s: ----> considering forwarding pkt on \"%s\" to %s\n",
        PLUGIN_NAME_SHORT,
        intf->ifName,
        olsr_ip_to_string(&buf, &walker->neighbor_iface_addr));

      neighborMainIp = MainAddressOf(&walker->neighbor_iface_addr);

      /* Consider only neighbors with an IP address that differs from the
       * passed IP addresses (if passed). Rely on short-circuit boolean evaluation. */
      if (source != NULL && ipequal(neighborMainIp, MainAddressOf(source)))
      {
        OLSR_PRINTF(
          9,
          "%s: ----> not forwarding to %s: is source of pkt\n",
          PLUGIN_NAME_SHORT,
          olsr_ip_to_string(&buf, &walker->neighbor_iface_addr));

        continue; /* for */
      }

      /* Rely on short-circuit boolean evaluation */
      if (forwardedBy != NULL && ipequal(neighborMainIp, MainAddressOf(forwardedBy)))
      {
        OLSR_PRINTF(
          9,
          "%s: ----> not forwarding to %s: is the node that forwarded the pkt\n",
          PLUGIN_NAME_SHORT,
          olsr_ip_to_string(&buf, &walker->neighbor_iface_addr));

        continue; /* for */
      }

      /* Rely on short-circuit boolean evaluation */
      if (forwardedTo != NULL && ipequal(neighborMainIp, MainAddressOf(forwardedTo)))
      {
        OLSR_PRINTF(
          9,
          "%s: ----> not forwarding to %s: is the node to which the pkt was forwarded\n",
          PLUGIN_NAME_SHORT,
          olsr_ip_to_string(&buf, &walker->neighbor_iface_addr));

        continue; /* for */
      }

      /* Found a candidate neighbor to direct our packet to */

      if (walker->link_cost >= INFINITE_COST)
      {
        OLSR_PRINTF(
          9,
          "%s: ----> not forwarding to %s: link is timing out\n",
          PLUGIN_NAME_SHORT,
          olsr_ip_to_string(&buf, &walker->neighbor_iface_addr));

        continue; /* for */
      }

      /* Compare costs to check if the candidate neighbor is best reached via 'intf' */
      OLSR_PRINTF(
        9,
        "%s: ----> forwarding pkt to %s will cost %5.2f\n",
        PLUGIN_NAME_SHORT,
        olsr_ip_to_string(&buf, &walker->neighbor_iface_addr),
        walker->link_cost);

      /* If the candidate neighbor is best reached via another interface, then skip 
       * the candidate neighbor; the candidate neighbor has been / will be selected via that
       * other interface. */
      bestLinkToNeighbor = get_best_link_to_neighbor(&walker->neighbor_iface_addr);

      if (walker != bestLinkToNeighbor)
      {
        if (bestLinkToNeighbor == NULL)
        {
          OLSR_PRINTF(
            9,
            "%s: ----> not forwarding to %s: no link found\n",
            PLUGIN_NAME_SHORT,
            olsr_ip_to_string(&buf, &walker->neighbor_iface_addr));
        }
        else
        {
          struct interface_olsr * bestIntf = if_ifwithaddr(&bestLinkToNeighbor->local_iface_addr);

          OLSR_PRINTF(
            9,
            "%s: ----> not forwarding to %s: \"%s\" gives a better link to this neighbor, costing %5.2f\n",
            PLUGIN_NAME_SHORT,
            olsr_ip_to_string(&buf, &walker->neighbor_iface_addr),
            bestIntf->int_name,
            bestLinkToNeighbor->link_cost);
        }

        continue; /* for */
      }

      if (forwardedBy != NULL)
      {
        OLSR_PRINTF(
          9,
          "%s: ----> 2-hop path from %s via me to %s will cost %5.2f\n",
          PLUGIN_NAME_SHORT,
          olsr_ip_to_string(&forwardedByBuf, forwardedBy),
          olsr_ip_to_string(&niaBuf, &walker->neighbor_iface_addr),
          previousLinkCost + walker->link_cost);
      }

      /* Check the topology table whether the 'forwardedBy' node is itself a direct
       * neighbor of the candidate neighbor, at a lower cost than the 2-hop route
       * via myself. If so, we do not need to forward the BMF packet to the candidate
       * neighbor, because the 'forwardedBy' node will forward the packet. */
      if (forwardedBy != NULL)
      {
        tcLastHop = olsr_lookup_tc_entry(MainAddressOf(forwardedBy));
        if (tcLastHop != NULL)
        {
          struct tc_edge_entry* tc_edge;

          tc_edge = olsr_lookup_tc_edge(tcLastHop, MainAddressOf(&walker->neighbor_iface_addr));
          
          /* We are not interested in dead-end or dying edges. */
          if (tc_edge != NULL && (tc_edge->flags & OLSR_TC_EDGE_DOWN) == 0)
          {
            if (previousLinkCost + walker->link_cost > tc_edge->link_cost)
            {
#ifndef NODEBUG
              struct ipaddr_str neighbor_iface_buf, forw_buf;
              olsr_ip_to_string(&neighbor_iface_buf, &walker->neighbor_iface_addr);
#endif /* NODEBUG */
              OLSR_PRINTF(
                9,
                "%s: ----> not forwarding to %s: I am not an MPR between %s and %s, direct link costs %5.2f\n",
                PLUGIN_NAME_SHORT,
                neighbor_iface_buf.buf,
                olsr_ip_to_string(&forw_buf, forwardedBy),
                neighbor_iface_buf.buf,
                tc_edge->link_cost);

              continue; /* for */
            } /* if */
          } /* if */
        } /* if */
      } /* if */

      /* Remember the best neighbor. If all are very bad, remember none. */
      if (walker->link_cost < bestLinkCost)
      {
        *bestNeighbor = walker;
        bestLinkCost = walker->link_cost;
      }

      /* Fill the list with up to 'FanOutLimit' neighbors. If there
       * are more neighbors, broadcast is used instead of unicast. In that
       * case we do not need the list of neighbors. */
      if (*nPossibleNeighbors < FanOutLimit)
      {
        neighbors->links[*nPossibleNeighbors] = walker;
      }

      *nPossibleNeighbors += 1;

    } /* for */

#else /* USING_THALES_LINK_COST_ROUTING */
        
    struct link_entry* walker;
    olsr_linkcost previousLinkEtx = LINK_COST_BROKEN;
    olsr_linkcost bestEtx = LINK_COST_BROKEN;

    if (forwardedBy != NULL)
    {
      /* Retrieve the cost of the link from 'forwardedBy' to myself */
      struct link_entry* bestLinkFromForwarder = get_best_link_to_neighbor(forwardedBy);
      if (bestLinkFromForwarder != NULL)
      {
        previousLinkEtx = bestLinkFromForwarder->linkcost;
      }
    }

    OLSR_FOR_ALL_LINK_ENTRIES(walker) {
      union olsr_ip_addr* neighborMainIp;
      struct link_entry* bestLinkToNeighbor;
      struct tc_entry* tcLastHop;
      olsr_linkcost currEtx;
#ifndef NODEBUG
      struct lqtextbuffer lqbuffer;
#endif /* NODEBUG */
 
      /* Consider only links from the specified interface */
      if (! ipequal(&intf->intAddr, &walker->local_iface_addr))
      {
        continue; /* for */
      }

      OLSR_PRINTF(
        9,
        "%s: ----> considering forwarding pkt on \"%s\" to %s\n",
        PLUGIN_NAME_SHORT,
        intf->ifName,
        olsr_ip_to_string(&buf, &walker->neighbor_iface_addr));

      neighborMainIp = MainAddressOf(&walker->neighbor_iface_addr);

      /* Consider only neighbors with an IP address that differs from the
       * passed IP addresses (if passed). Rely on short-circuit boolean evaluation. */
      if (source != NULL && ipequal(neighborMainIp, MainAddressOf(source)))
      {
        OLSR_PRINTF(
          9,
          "%s: ----> not forwarding to %s: is source of pkt\n",
          PLUGIN_NAME_SHORT,
          olsr_ip_to_string(&buf, &walker->neighbor_iface_addr));

        continue; /* for */
      }

      /* Rely on short-circuit boolean evaluation */
      if (forwardedBy != NULL && ipequal(neighborMainIp, MainAddressOf(forwardedBy)))
      {
        OLSR_PRINTF(
          9,
          "%s: ----> not forwarding to %s: is the node that forwarded the pkt\n",
          PLUGIN_NAME_SHORT,
          olsr_ip_to_string(&buf, &walker->neighbor_iface_addr));

        continue; /* for */
      }

      /* Rely on short-circuit boolean evaluation */
      if (forwardedTo != NULL && ipequal(neighborMainIp, MainAddressOf(forwardedTo)))
      {
        OLSR_PRINTF(
          9,
          "%s: ----> not forwarding to %s: is the node to which the pkt was forwarded\n",
          PLUGIN_NAME_SHORT,
          olsr_ip_to_string(&buf, &walker->neighbor_iface_addr));

        continue; /* for */
      }

      /* Found a candidate neighbor to direct our packet to */

      /* Calculate the link quality (ETX) of the link to the found neighbor */
      currEtx = walker->linkcost;
 
      if (currEtx >= LINK_COST_BROKEN)
      {
        OLSR_PRINTF(
          9,
          "%s: ----> not forwarding to %s: link is timing out\n",
          PLUGIN_NAME_SHORT,
          olsr_ip_to_string(&buf, &walker->neighbor_iface_addr));

        continue; /* for */
      }

      /* Compare costs to check if the candidate neighbor is best reached via 'intf' */
      OLSR_PRINTF(
        9,
        "%s: ----> forwarding pkt to %s will cost ETX %s\n",
        PLUGIN_NAME_SHORT,
        olsr_ip_to_string(&buf, &walker->neighbor_iface_addr),
        get_linkcost_text(currEtx, false, &lqbuffer));

      /* If the candidate neighbor is best reached via another interface, then skip 
       * the candidate neighbor; the candidate neighbor has been / will be selected via that
       * other interface. */
      bestLinkToNeighbor = get_best_link_to_neighbor(&walker->neighbor_iface_addr);

      if (walker != bestLinkToNeighbor)
      {
        if (bestLinkToNeighbor == NULL)
        {
          OLSR_PRINTF(
            9,
            "%s: ----> not forwarding to %s: no link found\n",
            PLUGIN_NAME_SHORT,
            olsr_ip_to_string(&buf, &walker->neighbor_iface_addr));
        }
        else
        {
#ifndef NODEBUG
          struct interface_olsr * bestIntf = if_ifwithaddr(&bestLinkToNeighbor->local_iface_addr);
#endif /* NODEBUG */
          OLSR_PRINTF(
            9,
            "%s: ----> not forwarding to %s: \"%s\" gives a better link to this neighbor, costing %s\n",
            PLUGIN_NAME_SHORT,
            olsr_ip_to_string(&buf, &walker->neighbor_iface_addr),
            bestIntf ? bestIntf->int_name : "NULL",
            get_linkcost_text(bestLinkToNeighbor->linkcost, false, &lqbuffer));
        }

        continue; /* for */
      }

      if (forwardedBy != NULL)
      {
#ifndef NODEBUG
        struct ipaddr_str forwardedByBuf, niaBuf;
#endif /* NODEBUG */
        OLSR_PRINTF(
          9,
          "%s: ----> 2-hop path from %s via me to %s will cost ETX %s\n",
          PLUGIN_NAME_SHORT,
          olsr_ip_to_string(&forwardedByBuf, forwardedBy),
          olsr_ip_to_string(&niaBuf, &walker->neighbor_iface_addr),
          get_linkcost_text(previousLinkEtx + currEtx, false, &lqbuffer));
      }

      /* Check the topology table whether the 'forwardedBy' node is itself a direct
       * neighbor of the candidate neighbor, at a lower cost than the 2-hop route
       * via myself. If so, we do not need to forward the BMF packet to the candidate
       * neighbor, because the 'forwardedBy' node will forward the packet. */
      if (forwardedBy != NULL)
      {
        tcLastHop = olsr_lookup_tc_entry(MainAddressOf(forwardedBy));
        if (tcLastHop != NULL)
        {
          struct tc_edge_entry* tc_edge;

          tc_edge = olsr_lookup_tc_edge(tcLastHop, MainAddressOf(&walker->neighbor_iface_addr));

          /* We are not interested in dead-end edges. */
          if (tc_edge) {
            olsr_linkcost tcEtx = tc_edge->cost;

            if (previousLinkEtx + currEtx > tcEtx)
            {
#ifndef NODEBUG
              struct ipaddr_str neighbor_iface_buf, forw_buf;
              olsr_ip_to_string(&neighbor_iface_buf, &walker->neighbor_iface_addr);
#endif /* NODEBUG */
              OLSR_PRINTF(
                9,
                "%s: ----> not forwarding to %s: I am not an MPR between %s and %s, direct link costs %s\n",
                PLUGIN_NAME_SHORT,
                neighbor_iface_buf.buf,
                olsr_ip_to_string(&forw_buf, forwardedBy),
                neighbor_iface_buf.buf,
                get_linkcost_text(tcEtx, true, &lqbuffer));

              continue; /* for */
            } /* if */
          } /* if */
        } /* if */
      } /* if */

      /* Remember the best neighbor. If all are very bad, remember none. */
      if (currEtx < bestEtx)
      {
        *bestNeighbor = walker;
        bestEtx = currEtx;
      }

      /* Fill the list with up to 'FanOutLimit' neighbors. If there
       * are more neighbors, broadcast is used instead of unicast. In that
       * case we do not need the list of neighbors. */
      if (*nPossibleNeighbors < FanOutLimit)
      {
        neighbors->links[*nPossibleNeighbors] = walker;
      }

      *nPossibleNeighbors += 1;
    } OLSR_FOR_ALL_LINK_ENTRIES_END(walker);

#endif /* USING_THALES_LINK_COST_ROUTING */

  } /* if */

  /* Display the result of the neighbor search */
  if (*nPossibleNeighbors == 0)
  {
    OLSR_PRINTF(
      9,
      "%s: ----> no suitable neighbor found to forward to on \"%s\"\n",
      PLUGIN_NAME_SHORT,
      intf->ifName);
  }
  else
  {
    OLSR_PRINTF(
      9,
      "%s: ----> %d neighbors found on \"%s\"; best neighbor to forward to: %s\n",
      PLUGIN_NAME_SHORT,
      *nPossibleNeighbors,
      intf->ifName,
      olsr_ip_to_string(&buf, &(*bestNeighbor)->neighbor_iface_addr));
  } /* if */

} /* FindNeighbors */

/* -------------------------------------------------------------------------
 * Function   : CreateCaptureSocket
 * Description: Create socket for promiscuously capturing multicast IP traffic
 * Input      : ifname - network interface (e.g. "eth0")
 * Output     : none
 * Return     : the socket descriptor ( >= 0), or -1 if an error occurred
 * Data Used  : none
 * Notes      : The socket is a cooked IP packet socket, bound to the specified
 *              network interface
 * ------------------------------------------------------------------------- */
static int CreateCaptureSocket(const char* ifName)
{
  int ifIndex = if_nametoindex(ifName);
  struct packet_mreq mreq;
  struct ifreq req;
  struct sockaddr_ll bindTo;

  /* Open cooked IP packet socket */
  int skfd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));
  if (skfd < 0)
  {
    BmfPError("socket(PF_PACKET) error");
    return -1;
  }

  /* Set interface to promiscuous mode */
  memset(&mreq, 0, sizeof(struct packet_mreq));
  mreq.mr_ifindex = ifIndex;
  mreq.mr_type = PACKET_MR_PROMISC;
  if (setsockopt(skfd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
  {
    BmfPError("setsockopt(PACKET_MR_PROMISC) error");
    close(skfd);
    return -1;
  }

  /* Get hardware (MAC) address */
  memset(&req, 0, sizeof(struct ifreq));
  strncpy(req.ifr_name, ifName, IFNAMSIZ - 1);
  req.ifr_name[IFNAMSIZ-1] = '\0'; /* Ensures null termination */
  if (ioctl(skfd, SIOCGIFHWADDR, &req) < 0)
  {
    BmfPError("error retrieving MAC address");
    close(skfd);
    return -1;
  }
   
  /* Bind the socket to the specified interface */
  memset(&bindTo, 0, sizeof(bindTo));
  bindTo.sll_family = AF_PACKET;
  bindTo.sll_protocol = htons(ETH_P_IP);
  bindTo.sll_ifindex = ifIndex;
  memcpy(bindTo.sll_addr, req.ifr_hwaddr.sa_data, IFHWADDRLEN);
  bindTo.sll_halen = IFHWADDRLEN;
    
  if (bind(skfd, (struct sockaddr*)&bindTo, sizeof(bindTo)) < 0)
  {
    BmfPError("bind() error");
    close(skfd);
    return -1;
  }

  /* Set socket to blocking operation */
  if (fcntl(skfd, F_SETFL, fcntl(skfd, F_GETFL, 0) & ~O_NONBLOCK) < 0)
  {
    BmfPError("fcntl() error");
    close(skfd);
    return -1;
  }

  AddDescriptorToInputSet(skfd);

  return skfd;
} /* CreateCaptureSocket */

/* -------------------------------------------------------------------------
 * Function   : CreateListeningSocket
 * Description: Create socket for promiscuously listening to BMF packets.
 *              Used only when 'BmfMechanism' is BM_UNICAST_PROMISCUOUS
 * Input      : ifname - network interface (e.g. "eth0")
 * Output     : none
 * Return     : the socket descriptor ( >= 0), or -1 if an error occurred
 * Data Used  : none
 * Notes      : The socket is a cooked IP packet socket, bound to the specified
 *              network interface
 * ------------------------------------------------------------------------- */
static int CreateListeningSocket(const char* ifName)
{
  int ifIndex = if_nametoindex(ifName);
  struct packet_mreq mreq;
  struct ifreq req;
  struct sockaddr_ll bindTo;

  /* Open cooked IP packet socket */
  int skfd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));
  if (skfd < 0)
  {
    BmfPError("socket(PF_PACKET) error");
    return -1;
  }

  /* Set interface to promiscuous mode */
  memset(&mreq, 0, sizeof(struct packet_mreq));
  mreq.mr_ifindex = ifIndex;
  mreq.mr_type = PACKET_MR_PROMISC;
  if (setsockopt(skfd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
  {
    BmfPError("setsockopt(PACKET_MR_PROMISC) error");
    close(skfd);
    return -1;
  }

  /* Get hardware (MAC) address */
  memset(&req, 0, sizeof(struct ifreq));
  strncpy(req.ifr_name, ifName, IFNAMSIZ - 1);
  req.ifr_name[IFNAMSIZ-1] = '\0'; /* Ensures null termination */
  if (ioctl(skfd, SIOCGIFHWADDR, &req) < 0)
  {
    BmfPError("error retrieving MAC address");
    close(skfd);
    return -1;
  }

  /* Bind the socket to the specified interface */
  memset(&bindTo, 0, sizeof(bindTo));
  bindTo.sll_family = AF_PACKET;
  bindTo.sll_protocol = htons(ETH_P_IP);
  bindTo.sll_ifindex = ifIndex;
  memcpy(bindTo.sll_addr, req.ifr_hwaddr.sa_data, IFHWADDRLEN);
  bindTo.sll_halen = IFHWADDRLEN;
    
  if (bind(skfd, (struct sockaddr*)&bindTo, sizeof(bindTo)) < 0)
  {
    BmfPError("bind() error");
    close(skfd);
    return -1;
  }

  /* Set socket to blocking operation */
  if (fcntl(skfd, F_SETFL, fcntl(skfd, F_GETFL, 0) & ~O_NONBLOCK) < 0)
  {
    BmfPError("fcntl() error");
    close(skfd);
    return -1;
  }

  AddDescriptorToInputSet(skfd);

  return skfd;
} /* CreateListeningSocket */

/* -------------------------------------------------------------------------
 * Function   : CreateEncapsulateSocket
 * Description: Create a socket for sending and receiving encapsulated
 *              multicast packets
 * Input      : ifname - network interface (e.g. "eth0")
 * Output     : none
 * Return     : the socket descriptor ( >= 0), or -1 if an error occurred
 * Data Used  : none
 * Notes      : The socket is an UDP (datagram) over IP socket, bound to the
 *              specified network interface
 * ------------------------------------------------------------------------- */
static int CreateEncapsulateSocket(const char* ifName)
{
  int on = 1;
  struct sockaddr_in bindTo;

  /* Open UDP-IP socket */
  int skfd = socket(PF_INET, SOCK_DGRAM, 0);
  if (skfd < 0)
  {
    BmfPError("socket(PF_INET) error");
    return -1;
  }

  /* Enable sending to broadcast addresses */
  if (setsockopt(skfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) < 0)
  {
    BmfPError("setsockopt(SO_BROADCAST) error");
    close(skfd);
    return -1;
  }
	
  /* Bind to the specific network interfaces indicated by ifName. */
  /* When using Kernel 2.6 this must happer prior to the port binding! */
  if (setsockopt(skfd, SOL_SOCKET, SO_BINDTODEVICE, ifName, strlen(ifName) + 1) < 0)
  {
    BmfPError("setsockopt(SO_BINDTODEVICE) error");
    close(skfd);
    return -1;
  }

  /* Bind to BMF port */
  memset(&bindTo, 0, sizeof(bindTo));
  bindTo.sin_family = AF_INET;
  bindTo.sin_port = htons(BMF_ENCAP_PORT);
  bindTo.sin_addr.s_addr = htonl(INADDR_ANY);
      
  if (bind(skfd, (struct sockaddr*)&bindTo, sizeof(bindTo)) < 0) 
  {
    BmfPError("bind() error");
    close(skfd);
    return -1;
  }

  /* Set socket to blocking operation */
  if (fcntl(skfd, F_SETFL, fcntl(skfd, F_GETFL, 0) & ~O_NONBLOCK) < 0)
  {
    BmfPError("fcntl() error");
    close(skfd);
    return -1;
  }

  AddDescriptorToInputSet(skfd);

  return skfd;
} /* CreateEncapsulateSocket */

/* -------------------------------------------------------------------------
 * Function   : CreateLocalEtherTunTap
 * Description: Creates and brings up an EtherTunTap interface
 * Input      : none
 * Output     : none
 * Return     : the socket file descriptor (>= 0), or -1 in case of failure
 * Data Used  : EtherTunTapIfName - name used for the tuntap interface (e.g.
 *                "bmf0")
 *              EtherTunTapIp
 *              EtherTunTapIpMask
 *              EtherTunTapIpBroadcast
 *              BmfInterfaces
 * Note       : Order dependency: call this function only if BmfInterfaces
 *              is filled with a list of network interfaces.
 * ------------------------------------------------------------------------- */
static int CreateLocalEtherTunTap(void)
{
  static const char * deviceName = OS_TUNNEL_PATH;
  struct ifreq ifreq;
  int etfd;
  int ioctlSkfd;
  int ioctlres;

  etfd = open(deviceName, O_RDWR | O_NONBLOCK);
  if (etfd < 0)
  {
    BmfPError("error opening %s", deviceName);
    return -1;
  }

  memset(&ifreq, 0, sizeof(ifreq));
  strncpy(ifreq.ifr_name, EtherTunTapIfName, IFNAMSIZ - 1);
  ifreq.ifr_name[IFNAMSIZ - 1] = '\0'; /* Ensures null termination */

  /* Specify the IFF_TUN flag for IP packets.
   * Specify IFF_NO_PI for not receiving extra meta packet information. */
  ifreq.ifr_flags = IFF_TUN;
  ifreq.ifr_flags |= IFF_NO_PI;

  if (ioctl(etfd, TUNSETIFF, (void *)&ifreq) < 0)
  {
    BmfPError("ioctl(TUNSETIFF) error on %s", deviceName);
    close(etfd);
    return -1;
  }

  memset(&ifreq, 0, sizeof(ifreq));
  strncpy(ifreq.ifr_name, EtherTunTapIfName, IFNAMSIZ - 1);
  ifreq.ifr_name[IFNAMSIZ - 1] = '\0'; /* Ensures null termination */
  ifreq.ifr_addr.sa_family = AF_INET;

  ioctlSkfd = socket(PF_INET, SOCK_DGRAM, 0);
  if (ioctlSkfd < 0)
  {
    BmfPError("socket(PF_INET) error on %s", deviceName);
    close(etfd);
    return -1;
  }

  /* Give the EtherTunTap interface an IP address.
   * The default IP address is the address of the first OLSR interface;
   * the default netmask is 255.255.255.255 . Having an all-ones netmask prevents
   * automatic entry of the BMF network interface in the routing table. */
  if (EtherTunTapIp == ETHERTUNTAPIPNOTSET)
  {
    struct TBmfInterface* nextBmfIf = BmfInterfaces;
    while (nextBmfIf != NULL)
    {
      struct TBmfInterface* bmfIf = nextBmfIf;
      nextBmfIf = bmfIf->next;

      if (bmfIf->olsrIntf != NULL)
      {
        EtherTunTapIp = ntohl(bmfIf->intAddr.v4.s_addr);
        EtherTunTapIpBroadcast = EtherTunTapIp;
      }
    }
  }

  if (EtherTunTapIp == ETHERTUNTAPIPNOTSET)
  {
    /* No IP address configured for BMF network interface, and no OLSR interface found to
     * copy IP address from. Fall back to default: 10.255.255.253 . */
    EtherTunTapIp = ETHERTUNTAPDEFAULTIP;
  }

  {
    struct sockaddr* ifra = &ifreq.ifr_addr;
    ((struct sockaddr_in*) ARM_NOWARN_ALIGN(ifra))->sin_addr.s_addr = htonl(EtherTunTapIp);
  }
  ioctlres = ioctl(ioctlSkfd, SIOCSIFADDR, &ifreq);
  if (ioctlres >= 0)
  {
    struct sockaddr* ifrn = &ifreq.ifr_netmask;
    struct sockaddr* ifrb = &ifreq.ifr_broadaddr;

    /* Set net mask */
    ((struct sockaddr_in*) ARM_NOWARN_ALIGN(ifrn))->sin_addr.s_addr = htonl(EtherTunTapIpMask);
    ioctlres = ioctl(ioctlSkfd, SIOCSIFNETMASK, &ifreq);
    if (ioctlres >= 0)
    {
      /* Set broadcast IP */
      ((struct sockaddr_in*) ARM_NOWARN_ALIGN(ifrb))->sin_addr.s_addr = htonl(EtherTunTapIpBroadcast);
      ioctlres = ioctl(ioctlSkfd, SIOCSIFBRDADDR, &ifreq);
      if (ioctlres >= 0)
      {
        /* Bring EtherTunTap interface up (if not already) */
        ioctlres = ioctl(ioctlSkfd, SIOCGIFFLAGS, &ifreq);
        if (ioctlres >= 0)
        {
          ifreq.ifr_flags |= (IFF_UP | IFF_RUNNING | IFF_BROADCAST);
          ioctlres = ioctl(ioctlSkfd, SIOCSIFFLAGS, &ifreq);
        }
      }
    }
  }

  if (ioctlres < 0)
  {
    /* Any of the above ioctl() calls failed */
    BmfPError("error bringing up EtherTunTap interface \"%s\"", EtherTunTapIfName);

    close(etfd);
    close(ioctlSkfd);
    return -1;
  } /* if (ioctlres < 0) */

  /* Set the multicast flag on the interface */
  memset(&ifreq, 0, sizeof(ifreq));
  strncpy(ifreq.ifr_name, EtherTunTapIfName, IFNAMSIZ - 1);
  ifreq.ifr_name[IFNAMSIZ - 1] = '\0'; /* Ensures null termination */

  ioctlres = ioctl(ioctlSkfd, SIOCGIFFLAGS, &ifreq);
  if (ioctlres >= 0)
  {
    ifreq.ifr_flags |= IFF_MULTICAST;
    ioctlres = ioctl(ioctlSkfd, SIOCSIFFLAGS, &ifreq);
  }
  if (ioctlres < 0)
  {
    /* Any of the two above ioctl() calls failed */
    BmfPError("error setting multicast flag on EtherTunTap interface \"%s\"", EtherTunTapIfName);

    /* Continue anyway */
  }

  /* Use ioctl to make the tuntap persistent. Otherwise it will disappear
   * when this program exits. That is not desirable, since a multicast
   * daemon (e.g. mrouted) may be using the tuntap interface. */
  if (ioctl(etfd, TUNSETPERSIST, EtherTunTapPersistent ? (void *)&ifreq : NULL) < 0)
  {
    BmfPError("error making EtherTunTap interface \"%s\" %spersistent", EtherTunTapIfName, !EtherTunTapPersistent ? "non-" : "");

    /* Continue anyway */
  }

  OLSR_PRINTF(8, "%s: opened 1 socket on \"%s\"\n", PLUGIN_NAME_SHORT, EtherTunTapIfName);

  AddDescriptorToInputSet(etfd);

  /* If the user configured a specific IP address for the BMF network interface,
   * help the user and advertise the IP address of the BMF network interface
   * on the OLSR network via HNA */
  if (TunTapIpOverruled != 0)
  {
    union olsr_ip_addr temp_net;

    temp_net.v4.s_addr = htonl(EtherTunTapIp);
    ip_prefix_list_add(&olsr_cnf->hna_entries, &temp_net, 32);
  }

  close(ioctlSkfd);

  return etfd;
} /* CreateLocalEtherTunTap */

/* -------------------------------------------------------------------------
 * Function   : CreateInterface
 * Description: Create a new TBmfInterface object and adds it to the global
 *              BmfInterfaces list
 * Input      : ifName - name of the network interface (e.g. "eth0")
 *            : olsrIntf - OLSR interface object of the network interface, or
 *                NULL if the network interface is not OLSR-enabled
 * Output     : none
 * Return     : the number of opened sockets
 * Data Used  : BmfInterfaces, LastBmfInterface
 * ------------------------------------------------------------------------- */
static int CreateInterface(
  const char* ifName,
  struct interface_olsr * olsrIntf)
{
  int capturingSkfd = -1;
  int encapsulatingSkfd = -1;
  int listeningSkfd = -1;
  int ioctlSkfd;
  struct ifreq ifr;
  int nOpened = 0;
  struct TBmfInterface* newIf = malloc(sizeof(struct TBmfInterface));

  assert(ifName != NULL);

  if (newIf == NULL)
  {
    return 0;
  }

  if (olsrIntf != NULL)
  {
    /* On OLSR-enabled interfaces, create socket for encapsulating and forwarding 
     * multicast packets */
    encapsulatingSkfd = CreateEncapsulateSocket(ifName);
    if (encapsulatingSkfd < 0)
    {
      free(newIf);
      return 0;
    }
    nOpened++;
  }

  /* Create socket for capturing and sending of multicast packets on
   * non-OLSR interfaces, and on OLSR-interfaces if configured. */
  if ((olsrIntf == NULL) || (CapturePacketsOnOlsrInterfaces != 0))
  {
    capturingSkfd = CreateCaptureSocket(ifName);
    if (capturingSkfd < 0)
    {
      if (encapsulatingSkfd >= 0) {
        close(encapsulatingSkfd);
      }
      free(newIf);
      return 0;
    }

    nOpened++;
  }

  /* Create promiscuous mode listening interface if BMF uses IP unicast
   * as underlying forwarding mechanism */
  if (BmfMechanism == BM_UNICAST_PROMISCUOUS)
  {
    listeningSkfd = CreateListeningSocket(ifName);
    if (listeningSkfd < 0)
    {
      if (encapsulatingSkfd >= 0) {
        close(encapsulatingSkfd); /* no problem if 'encapsulatingSkfd' is -1 */
      }
      free(newIf);
      return 0;
    }

    nOpened++;
  }

  /* For ioctl operations on the network interface, use either capturingSkfd
   * or encapsulatingSkfd, whichever is available */
  ioctlSkfd = (capturingSkfd >= 0) ? capturingSkfd : encapsulatingSkfd;

  /* Retrieve the MAC address of the interface. */
  memset(&ifr, 0, sizeof(struct ifreq));
  strncpy(ifr.ifr_name, ifName, IFNAMSIZ - 1);
  ifr.ifr_name[IFNAMSIZ - 1] = '\0'; /* Ensures null termination */
  if (ioctl(ioctlSkfd, SIOCGIFHWADDR, &ifr) < 0)
  {
    BmfPError("ioctl(SIOCGIFHWADDR) error for interface \"%s\"", ifName);
    if (capturingSkfd >= 0) {
      close(capturingSkfd);
    }
    if (encapsulatingSkfd >= 0) {
      close(encapsulatingSkfd);
    }
    free(newIf);
    return 0;
  }

  /* add listeners to sockets */
  if (capturingSkfd != -1) {
    add_olsr_socket(capturingSkfd, NULL, BMF_handle_captureFd, newIf, SP_IMM_READ);
  }
  if (encapsulatingSkfd != -1) {
    add_olsr_socket(encapsulatingSkfd, NULL, BMF_handle_encapsulatingFd, newIf, SP_IMM_READ);
  }
  if (listeningSkfd != -1) {
    add_olsr_socket(listeningSkfd, NULL, BMF_handle_listeningFd, newIf, SP_IMM_READ);
  }
  /* Copy data into TBmfInterface object */
  newIf->capturingSkfd = capturingSkfd;
  newIf->encapsulatingSkfd = encapsulatingSkfd;
  newIf->listeningSkfd = listeningSkfd;
  memcpy(newIf->macAddr, ifr.ifr_hwaddr.sa_data, IFHWADDRLEN);
  memcpy(newIf->ifName, ifName, IFNAMSIZ);
  newIf->olsrIntf = olsrIntf;
  if (olsrIntf != NULL)
  {
    /* For an OLSR-interface, copy the interface address and broadcast
     * address from the OLSR interface object. Downcast to correct sockaddr
     * subtype. */
    newIf->intAddr.v4 = ((struct sockaddr_in *)&olsrIntf->int_addr)->sin_addr;
    newIf->broadAddr.v4 = ((struct sockaddr_in *)&olsrIntf->int_broadaddr)->sin_addr;
  }
  else
  {
    /* For a non-OLSR interface, retrieve the IP address ourselves */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, ifName, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0'; /* Ensures null termination */
    if (ioctl(ioctlSkfd, SIOCGIFADDR, &ifr) < 0) 
    {
      BmfPError("ioctl(SIOCGIFADDR) error for interface \"%s\"", ifName);

      newIf->intAddr.v4.s_addr = inet_addr("0.0.0.0");
    }
    else
    {
      /* Downcast to correct sockaddr subtype */
      struct sockaddr* ifra = &ifr.ifr_addr;
      newIf->intAddr.v4 = ((struct sockaddr_in *) ARM_NOWARN_ALIGN(ifra))->sin_addr;
    }

    /* For a non-OLSR interface, retrieve the IP broadcast address ourselves */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, ifName, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0'; /* Ensures null termination */
    if (ioctl(ioctlSkfd, SIOCGIFBRDADDR, &ifr) < 0) 
    {
      BmfPError("ioctl(SIOCGIFBRDADDR) error for interface \"%s\"", ifName);

      newIf->broadAddr.v4.s_addr = inet_addr("0.0.0.0");
    }
    else
    {
      /* Downcast to correct sockaddr subtype */
      struct sockaddr* ifrb = &ifr.ifr_broadaddr;
      newIf->broadAddr.v4 = ((struct sockaddr_in *) ARM_NOWARN_ALIGN(ifrb))->sin_addr;
    }
  }

  /* Reset counters */
  newIf->nBmfPacketsRx = 0;
  newIf->nBmfPacketsRxDup = 0;
  newIf->nBmfPacketsTx = 0;

  /* Add new TBmfInterface object to global list. OLSR interfaces are
   * added at the front of the list, non-OLSR interfaces at the back. */
  if (BmfInterfaces == NULL)
  {
    /* First TBmfInterface object in list */
    newIf->next = NULL;
    BmfInterfaces = newIf;
    LastBmfInterface = newIf;
  }
  else if (olsrIntf != NULL)
  {
    /* Add new TBmfInterface object at front of list */
    newIf->next = BmfInterfaces;
    BmfInterfaces = newIf;
  }
  else
  {
    /* Add new TBmfInterface object at back of list */
    newIf->next = NULL;
    LastBmfInterface->next= newIf;
    LastBmfInterface = newIf;
  }

  OLSR_PRINTF(
    8,
    "%s: opened %d socket%s on %s interface \"%s\"\n",
    PLUGIN_NAME_SHORT,
    nOpened,
    nOpened == 1 ? "" : "s",
    olsrIntf != NULL ? "OLSR" : "non-OLSR",
    ifName);

  return nOpened;
} /* CreateInterface */

/* -------------------------------------------------------------------------
 * Function   : CreateBmfNetworkInterfaces
 * Description: Create a list of TBmfInterface objects, one for each network
 *              interface on which BMF runs
 * Input      : skipThisIntf - network interface to skip, if seen
 * Output     : none
 * Return     : fail (-1) or success (0)
 * Data Used  : none
 * ------------------------------------------------------------------------- */
int CreateBmfNetworkInterfaces(struct interface_olsr * skipThisIntf)
{
  int skfd;
  struct ifconf ifc;
  int numreqs = 30;
  struct ifreq* ifr;
  int n;
  int nOpenedSockets = 0;

  /* Clear input descriptor set */
  FD_ZERO(&InputSet);

  skfd = socket(PF_INET, SOCK_DGRAM, 0);
  if (skfd < 0)
  {
    BmfPError("no inet socket available to retrieve interface list");
    return -1;
  }

  /* Retrieve the network interface configuration list */
  ifc.ifc_buf = NULL;
  for (;;)
  {
    ifc.ifc_len = sizeof(struct ifreq) * numreqs;
    ifc.ifc_buf = olsr_realloc(ifc.ifc_buf, ifc.ifc_len, "BMF: CreateBmfNetworkInterfaces ifc");

    if (ioctl(skfd, SIOCGIFCONF, &ifc) < 0)
    {
      BmfPError("ioctl(SIOCGIFCONF) error");

      close(skfd);
      free(ifc.ifc_buf);
      return -1;
    }
    if ((unsigned)ifc.ifc_len == sizeof(struct ifreq) * numreqs)
    {
      /* Assume it overflowed; double the space and try again */
      numreqs *= 2;
      assert(numreqs < 1024);
      continue; /* for (;;) */
    }
    break; /* for (;;) */
  } /* for (;;) */

  close(skfd);

  /* For each item in the interface configuration list... */
  ifr = ifc.ifc_req;
  for (n = ifc.ifc_len / sizeof(struct ifreq); --n >= 0; ifr++)
  {
    struct interface_olsr * olsrIntf;
    union olsr_ip_addr ipAddr;

    /* Skip the BMF network interface itself */
    if (strncmp(ifr->ifr_name, EtherTunTapIfName, IFNAMSIZ) == 0)
    {
      continue; /* for (n = ...) */
    }

    /* ...find the OLSR interface structure, if any */
    {
      struct sockaddr* ifra = &ifr->ifr_addr;
      ipAddr.v4 =  ((struct sockaddr_in*) ARM_NOWARN_ALIGN(ifra))->sin_addr;
    }
    olsrIntf = if_ifwithaddr(&ipAddr);

    if (skipThisIntf != NULL && olsrIntf == skipThisIntf)
    {
      continue; /* for (n = ...) */
    }

    if (olsrIntf == NULL && ! IsNonOlsrBmfIf(ifr->ifr_name))
    {
      /* Interface is neither OLSR interface, nor specified as non-OLSR BMF
       * interface in the BMF plugin parameter list */
      continue; /* for (n = ...) */
    }

    nOpenedSockets += CreateInterface(ifr->ifr_name, olsrIntf);

  } /* for (n = ...) */

  free(ifc.ifc_buf);
  
  /* Create the BMF network interface */
  EtherTunTapFd = CreateLocalEtherTunTap();
  if (EtherTunTapFd >= 0)
  {
    add_olsr_socket(EtherTunTapFd, NULL, BMF_handle_tuntapFd, NULL, SP_IMM_READ);
    nOpenedSockets++;
  }

  if (BmfInterfaces == NULL)
  {
    olsr_printf(1, "%s: could not initialize any network interface\n", PLUGIN_NAME_SHORT);
  }
  else
  {
    olsr_printf(1, "%s: opened %d sockets\n", PLUGIN_NAME_SHORT, nOpenedSockets);
  }
  return 0;
} /* CreateBmfNetworkInterfaces */

/* -------------------------------------------------------------------------
 * Function   : CloseBmfNetworkInterfaces
 * Description: Closes every socket on each network interface used by BMF
 * Input      : none
 * Output     : none
 * Return     : none
 * Data Used  : BmfInterfaces, LastBmfInterface
 * Notes      : Closes
 *              - the local EtherTunTap interface (e.g. "tun0" or "tap0")
 *              - for each BMF-enabled interface, the socket used for
 *                capturing multicast packets
 *              - for each OLSR-enabled interface, the socket used for
 *                encapsulating packets
 *              Also restores the network state to the situation before BMF
 *              was started.
 * ------------------------------------------------------------------------- */
void CloseBmfNetworkInterfaces(void)
{
  int nClosed = 0;
  u_int32_t totalOlsrBmfPacketsRx = 0;
  u_int32_t totalOlsrBmfPacketsRxDup = 0;
  u_int32_t totalOlsrBmfPacketsTx = 0;
  u_int32_t totalNonOlsrBmfPacketsRx = 0;
  u_int32_t totalNonOlsrBmfPacketsRxDup = 0;
  u_int32_t totalNonOlsrBmfPacketsTx = 0;

  /* Close all opened sockets */
  struct TBmfInterface* nextBmfIf = BmfInterfaces;
  while (nextBmfIf != NULL) 
  {
    struct TBmfInterface* bmfIf = nextBmfIf;
    nextBmfIf = bmfIf->next;

    if (bmfIf->capturingSkfd >= 0)
    {
      close(bmfIf->capturingSkfd);
      remove_olsr_socket(bmfIf->capturingSkfd, NULL, BMF_handle_captureFd);
      nClosed++;
    }
    if (bmfIf->encapsulatingSkfd >= 0) 
    {
      close(bmfIf->encapsulatingSkfd);
      remove_olsr_socket(bmfIf->encapsulatingSkfd, NULL, BMF_handle_encapsulatingFd);
      nClosed++;
    }
    if (bmfIf->listeningSkfd >= 0)
    {
      close(bmfIf->listeningSkfd);
      remove_olsr_socket(bmfIf->listeningSkfd, NULL, BMF_handle_listeningFd);
      nClosed++;
    }

    OLSR_PRINTF(
      7,
      "%s: %s interface \"%s\": RX pkts %d (%d dups); TX pkts %d\n", 
      PLUGIN_NAME_SHORT,
      bmfIf->olsrIntf != NULL ? "OLSR" : "non-OLSR",
      bmfIf->ifName,
      bmfIf->nBmfPacketsRx,
      bmfIf->nBmfPacketsRxDup,
      bmfIf->nBmfPacketsTx);

    olsr_printf(
      1,
      "%s: closed %s interface \"%s\"\n", 
      PLUGIN_NAME_SHORT,
      bmfIf->olsrIntf != NULL ? "OLSR" : "non-OLSR",
      bmfIf->ifName);

    /* Add totals */
    if (bmfIf->olsrIntf != NULL)
    {
      totalOlsrBmfPacketsRx += bmfIf->nBmfPacketsRx;
      totalOlsrBmfPacketsRxDup += bmfIf->nBmfPacketsRxDup;
      totalOlsrBmfPacketsTx += bmfIf->nBmfPacketsTx;
    }
    else
    {
      totalNonOlsrBmfPacketsRx += bmfIf->nBmfPacketsRx;
      totalNonOlsrBmfPacketsRxDup += bmfIf->nBmfPacketsRxDup;
      totalNonOlsrBmfPacketsTx += bmfIf->nBmfPacketsTx;
    }

    free(bmfIf);
  } /* while */
  
  if (EtherTunTapFd >= 0)
  {
    close(EtherTunTapFd);
    remove_olsr_socket(EtherTunTapFd, NULL, BMF_handle_tuntapFd);
    nClosed++;

    OLSR_PRINTF(7, "%s: closed \"%s\"\n", PLUGIN_NAME_SHORT, EtherTunTapIfName);
  }

  BmfInterfaces = NULL;
  LastBmfInterface = NULL;

  /* Re-initialize the IP address for the BMF network interface. Thanks to
   * Daniele Lacamera for finding and solving this bug. */
  EtherTunTapIp = ETHERTUNTAPIPNOTSET;

  olsr_printf(1, "%s: closed %d sockets\n", PLUGIN_NAME_SHORT, nClosed);

  OLSR_PRINTF(
    7,
    "%s: total all OLSR interfaces    : RX pkts %d (%d dups); TX pkts %d\n",
    PLUGIN_NAME_SHORT,
    totalOlsrBmfPacketsRx,
    totalOlsrBmfPacketsRxDup,
    totalOlsrBmfPacketsTx);
  OLSR_PRINTF(
    7,
    "%s: total all non-OLSR interfaces: RX pkts %d (%d dups); TX pkts %d\n",
    PLUGIN_NAME_SHORT,
    totalNonOlsrBmfPacketsRx,
    totalNonOlsrBmfPacketsRxDup,
    totalNonOlsrBmfPacketsTx);
} /* CloseBmfNetworkInterfaces */

#define MAX_NON_OLSR_IFS 32
static char NonOlsrIfNames[MAX_NON_OLSR_IFS][IFNAMSIZ];
static int nNonOlsrIfs = 0;

/* -------------------------------------------------------------------------
 * Function   : AddNonOlsrBmfIf
 * Description: Add an non-OLSR enabled network interface to the list of BMF-enabled
 *              network interfaces
 * Input      : ifName - network interface (e.g. "eth0")
 *              data - not used
 *              addon - not used
 * Output     : none
 * Return     : success (0) or fail (1)
 * Data Used  : NonOlsrIfNames
 * ------------------------------------------------------------------------- */
int AddNonOlsrBmfIf(
  const char* ifName,
  void* data __attribute__((unused)),
  set_plugin_parameter_addon addon __attribute__((unused)))
{
  assert(ifName != NULL);

  if (nNonOlsrIfs >= MAX_NON_OLSR_IFS)
  {
    olsr_printf(
      1,
      "%s: too many non-OLSR interfaces specified, maximum is %d\n",
      PLUGIN_NAME_SHORT,
      MAX_NON_OLSR_IFS);
    return 1;
  }

  strncpy(NonOlsrIfNames[nNonOlsrIfs], ifName, IFNAMSIZ - 1);
  NonOlsrIfNames[nNonOlsrIfs][IFNAMSIZ - 1] = '\0'; /* Ensures null termination */
  nNonOlsrIfs++;
  return 0;
} /* AddNonOlsrBmfIf */

/* -------------------------------------------------------------------------
 * Function   : IsNonOlsrBmfIf
 * Description: Checks if a network interface is OLSR-enabled
 * Input      : ifName - network interface (e.g. "eth0")
 * Output     : none
 * Return     : true (1) or false (0)
 * Data Used  : NonOlsrIfNames
 * ------------------------------------------------------------------------- */
int IsNonOlsrBmfIf(const char* ifName)
{
  int i;

  assert(ifName != NULL);

  for (i = 0; i < nNonOlsrIfs; i++)
  {
    if (strncmp(NonOlsrIfNames[i], ifName, IFNAMSIZ) == 0) return 1;
  }
  return 0;
} /* IsNonOlsrBmfIf */

/* -------------------------------------------------------------------------
 * Function   : CheckAndUpdateLocalBroadcast
 * Description: For an IP packet, check if the destination address is not a
 *              multicast address. If it is not, the packet is assumed to be
 *              a local broadcast packet. In that case, set the destination
 *              address of the IP packet to the passed broadcast address.
 * Input      : ipPacket - the IP packet
 *              broadAddr - the broadcast address to fill in
 * Output     : none
 * Return     : none
 * Data Used  : none
 * Notes      : See also RFC1141
 * ------------------------------------------------------------------------- */
void CheckAndUpdateLocalBroadcast(unsigned char* ipPacket, union olsr_ip_addr* broadAddr)
{
  struct iphdr* iph;
  union olsr_ip_addr destIp;

  assert(ipPacket != NULL && broadAddr != NULL);

  iph = (struct iphdr*) ARM_NOWARN_ALIGN(ipPacket);
  destIp.v4.s_addr = iph->daddr;
  if (! IsMulticast(&destIp))
  {
    u_int32_t origDaddr, newDaddr;
    u_int32_t check;

    origDaddr = ntohl(iph->daddr);

    iph->daddr = broadAddr->v4.s_addr;
    newDaddr = ntohl(iph->daddr);

    /* Re-calculate IP header checksum for new destination */
    check = ntohs(iph->check);

    check = ~ (~ check - ((origDaddr >> 16) & 0xFFFF) + ((newDaddr >> 16) & 0xFFFF));
    check = ~ (~ check - (origDaddr & 0xFFFF) + (newDaddr & 0xFFFF));

    /* Add carry */
    check = check + (check >> 16);

    iph->check = htons(check);

    if (iph->protocol == SOL_UDP)
    {
      /* Re-calculate UDP/IP checksum for new destination */

      int ipHeaderLen = GetIpHeaderLength(ipPacket);
      struct udphdr* udph = (struct udphdr*) ARM_NOWARN_ALIGN((ipPacket + ipHeaderLen));

      /* RFC 1624, Eq. 3: HC' = ~(~HC - m + m') */

#if defined(__GLIBC__) || defined(__BIONIC__)
      check = ntohs(udph->check);
#else
      check = ntohs(udph->uh_sum);
#endif

      check = ~ (~ check - ((origDaddr >> 16) & 0xFFFF) + ((newDaddr >> 16) & 0xFFFF));
      check = ~ (~ check - (origDaddr & 0xFFFF) + (newDaddr & 0xFFFF));

      /* Add carry */
      check = check + (check >> 16);

#if defined(__GLIBC__) || defined(__BIONIC__)
      udph->check = htons(check);
#else
      udph->uh_sum = htons(check);
#endif
     } /* if */
  } /* if */
} /* CheckAndUpdateLocalBroadcast */

/* -------------------------------------------------------------------------
 * Function   : AddMulticastRoute
 * Description: Insert a route to all multicast addresses in the kernel
 *              routing table. The route will be via the BMF network interface.
 * Input      : none
 * Output     : none
 * Return     : none
 * Data Used  : none
 * ------------------------------------------------------------------------- */
void AddMulticastRoute(void)
{
  struct rtentry kernel_route;
  int ioctlSkfd = socket(PF_INET, SOCK_DGRAM, 0);
  if (ioctlSkfd < 0)
  {
    BmfPError("socket(PF_INET) error");
    return;
  }

  memset(&kernel_route, 0, sizeof(struct rtentry));

  ((struct sockaddr *) ARM_NOWARN_ALIGN(&kernel_route.rt_dst))->sa_family = AF_INET;
  ((struct sockaddr *) ARM_NOWARN_ALIGN(&kernel_route.rt_gateway))->sa_family = AF_INET;
  ((struct sockaddr *) ARM_NOWARN_ALIGN(&kernel_route.rt_genmask))->sa_family = AF_INET;

  /* 224.0.0.0/4 */
  {
    struct sockaddr* rdst = &kernel_route.rt_dst;
    struct sockaddr* rmask = &kernel_route.rt_genmask;
    ((struct sockaddr_in *) ARM_NOWARN_ALIGN(rdst))->sin_addr.s_addr = htonl(0xE0000000);
    ((struct sockaddr_in *) ARM_NOWARN_ALIGN(rmask))->sin_addr.s_addr = htonl(0xF0000000);
  }

  kernel_route.rt_metric = 0;
  kernel_route.rt_flags = RTF_UP;

  kernel_route.rt_dev = EtherTunTapIfName;

  if (ioctl(ioctlSkfd, SIOCADDRT, &kernel_route) < 0)
  {
    BmfPError("error setting multicast route via EtherTunTap interface \"%s\"", EtherTunTapIfName);

    /* Continue anyway */
  }
  close(ioctlSkfd);
} /* AddMulticastRoute */

/* -------------------------------------------------------------------------
 * Function   : DeleteMulticastRoute
 * Description: Delete the route to all multicast addresses from the kernel
 *              routing table
 * Input      : none
 * Output     : none
 * Return     : none
 * Data Used  : none
 * ------------------------------------------------------------------------- */
void DeleteMulticastRoute(void)
{
  if (EtherTunTapIp != ETHERTUNTAPDEFAULTIP)
  {
    struct rtentry kernel_route;
    int ioctlSkfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (ioctlSkfd < 0)
    {
      BmfPError("socket(PF_INET) error");
      return;
    }

    memset(&kernel_route, 0, sizeof(struct rtentry));

    ((struct sockaddr *) ARM_NOWARN_ALIGN(&kernel_route.rt_dst))->sa_family = AF_INET;
    ((struct sockaddr *) ARM_NOWARN_ALIGN(&kernel_route.rt_gateway))->sa_family = AF_INET;
    ((struct sockaddr *) ARM_NOWARN_ALIGN(&kernel_route.rt_genmask))->sa_family = AF_INET;

    /* 224.0.0.0/4 */
    {
      struct sockaddr* rdst = &kernel_route.rt_dst;
      struct sockaddr* rmask = &kernel_route.rt_genmask;
      ((struct sockaddr_in *) ARM_NOWARN_ALIGN(rdst))->sin_addr.s_addr = htonl(0xE0000000);
      ((struct sockaddr_in *) ARM_NOWARN_ALIGN(rmask))->sin_addr.s_addr = htonl(0xF0000000);
    }

    kernel_route.rt_metric = 0;
    kernel_route.rt_flags = RTF_UP;

    kernel_route.rt_dev = EtherTunTapIfName;

    if (ioctl(ioctlSkfd, SIOCDELRT, &kernel_route) < 0)
    {
      BmfPError("error deleting multicast route via EtherTunTap interface \"%s\"", EtherTunTapIfName);

      /* Continue anyway */
    }
    close(ioctlSkfd);
  } /* if */
} /* DeleteMulticastRoute */
