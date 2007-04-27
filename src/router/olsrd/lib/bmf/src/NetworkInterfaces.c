/*
 * OLSR Basic Multicast Forwarding (BMF) plugin.
 * Copyright (c) 2005, 2006, Thales Communications, Huizen, The Netherlands.
 * Written by Erik Tromp.
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
 * * Neither the name of Thales, BMF nor the names of its 
 *   contributors may be used to endorse or promote products derived 
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* -------------------------------------------------------------------------
 * File       : NetworkInterfaces.c
 * Description: Functions to open and close sockets
 * Created    : 29 Jun 2006
 *
 * $Id: NetworkInterfaces.c,v 1.3 2007/02/11 11:51:56 bernd67 Exp $ 
 * ------------------------------------------------------------------------- */

#include "NetworkInterfaces.h"

/* System includes */
#include <syslog.h> /* syslog() */
#include <string.h> /* strerror(), strchr(), strcmp() */
#include <errno.h> /* errno */
#include <unistd.h> /* close() */
#include <sys/ioctl.h> /* ioctl() */
#include <fcntl.h> /* fcntl() */
#include <assert.h> /* assert() */
#include <net/if.h> /* socket(), ifreq, if_indextoname(), if_nametoindex() */
#include <netinet/in.h> /* htons() */
#include <linux/if_ether.h> /* ETH_P_ALL */
#include <linux/if_packet.h> /* packet_mreq, PACKET_MR_PROMISC, PACKET_ADD_MEMBERSHIP */
#include <linux/if_tun.h> /* IFF_TAP */
#include <netinet/ip.h> /* struct ip */
#include <netinet/udp.h> /* SOL_UDP */

/* OLSRD includes */
#include "olsr.h" /* olsr_printf() */
#include "defs.h" /* olsr_cnf */
#include "local_hna_set.h" /* add_local_hna4_entry() */

/* Plugin includes */
#include "Packet.h" /* IFHWADDRLEN */
#include "Bmf.h" /* PLUGIN_NAME */
#include "Address.h" /* IsMulticast() */

/* List of network interface objects used by BMF plugin */
struct TBmfInterface* BmfInterfaces = NULL;

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

/* If the plugin parameter "BmfInterfaceType" is set to "tap", an
 * EtherTap interface will be used, and this variable will be set to TT_TAP. If
 * "BmfInterfaceType" is set to "tun" or not set at all, an IP tunnel interface 
 * used, and this variable will be set to TT_TUN. */
enum TTunOrTap TunOrTap = TT_TUN;

#define ETHERTUNTAPIPNOTSET 0

/* 10.255.255.253 in host byte order */
#define ETHERTUNTAPDEFAULTIP 0x0AFFFFFD

/* The IP address of the BMF network interface in host byte order.
 * May be overruled by setting the plugin parameter "BmfInterfaceIp". */
u_int32_t EtherTunTapIp = ETHERTUNTAPIPNOTSET;

/* 255.255.255.255 in host byte order. May be overruled by
 * setting the plugin parameter "BmfInterfaceIp". */
u_int32_t EtherTunTapIpMask = 0xFFFFFFFF;

/* The IP broadcast address of the BMF network interface in host byte order.
 * May be overruled by setting the plugin parameter "BmfinterfaceIp". */
u_int32_t EtherTunTapIpBroadcast = ETHERTUNTAPDEFAULTIP;

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
 * Output     : none
 * Return     : fail (0) or success (1)
 * Data Used  : none
 * ------------------------------------------------------------------------- */
int SetBmfInterfaceName(const char* ifname)
{
  strncpy(EtherTunTapIfName, ifname, IFNAMSIZ - 1);
  EtherTunTapIfName[IFNAMSIZ - 1] = '\0'; /* Ensures null termination */
  return 1;
}

/* -------------------------------------------------------------------------
 * Function   : SetBmfInterfaceType
 * Description: Overrule the default network interface type ("tun") of the
 *              EtherTunTap interface
 * Input      : iftype - network interface type, either "tun" or "tap"
 * Output     : none
 * Return     : fail (0) or success (1)
 * Data Used  : none
 * ------------------------------------------------------------------------- */
int SetBmfInterfaceType(const char* iftype)
{
  if (strcmp(iftype, "tun") == 0)
  {
    TunOrTap = TT_TUN;
    return 1;
  }
  else if (strcmp(iftype, "tap") == 0)
  {
    TunOrTap = TT_TAP;
    return 1;
  }

  /* Value not recognized */
  return 0;
}

/* -------------------------------------------------------------------------
 * Function   : SetBmfInterfaceIp
 * Description: Overrule the default IP address and prefix length
 *              ("10.255.255.253/30") of the EtherTunTap interface
 * Input      : ip - IP address, followed by '/' and prefix length
 * Output     : none
 * Return     : fail (0) or success (1)
 * Data Used  : none
 * ------------------------------------------------------------------------- */
int SetBmfInterfaceIp(const char* ip)
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
    return 0;
  }

  strncpy(ipAddr, ip, slashAt - ip);
  *(ipAddr + (slashAt - ip)) = '\0';
  if (inet_aton(ipAddr, &sinaddr) == 0)
  {
    /* Invalid address passed */
    return 0;
  }

  EtherTunTapIp = ntohl(sinaddr.s_addr);

  /* Get prefix length. */
  prefixLen = atoi(++slashAt);
  if (prefixLen <= 0 || prefixLen > IPV4_MAX_PREFIXLEN)
  {
	  return 0;
	}

  /* Compose IP subnet mask in host byte order */
  EtherTunTapIpMask = 0;
  for (i = 0; i < prefixLen; i++)
  {
    EtherTunTapIpMask |= (1 << (IPV4_MAX_PREFIXLEN - 1 - i));
  }

  /* Compose IP broadcast address in host byte order */
  EtherTunTapIpBroadcast = EtherTunTapIp;
  for (i=prefixLen; i < IPV4_MAX_PREFIXLEN; i++)
  {
    EtherTunTapIpBroadcast |= (1 << (IPV4_MAX_PREFIXLEN - 1 - i));
  }

  TunTapIpOverruled = 1;

  return 1;
}

/* -------------------------------------------------------------------------
 * Function   : SetCapturePacketsOnOlsrInterfaces
 * Description: Overrule the default setting, enabling or disabling the
 *              capturing of packets on OLSR-enabled interfaces.
 * Input      : enable - either "yes" or "no"
 * Output     : none
 * Return     : fail (0) or success (1)
 * Data Used  : none
 * ------------------------------------------------------------------------- */
int SetCapturePacketsOnOlsrInterfaces(const char* enable)
{
  if (strcmp(enable, "yes") == 0)
  {
    CapturePacketsOnOlsrInterfaces = 1;
    return 1;
  }
  else if (strcmp(enable, "no") == 0)
  {
    CapturePacketsOnOlsrInterfaces = 0;
    return 1;
  }

  /* Value not recognized */
  return 0;
}

/* To save the state of the IP spoof filter for the EtherTunTap interface */
static char EthTapSpoofState = '1';

/* -------------------------------------------------------------------------
 * Function   : DeactivateSpoofFilter
 * Description: Deactivates the Linux anti-spoofing filter for the tuntap
 *              interface
 * Input      : tunTapName - name used for the tuntap interface (e.g. "tun0" or "tap1")
 * Output     : none
 * Return     : fail (0) or success (1)
 * Data Used  : EthTapSpoofState
 * Notes      : Saves the current filter state for later restoring
 * ------------------------------------------------------------------------- */
static int DeactivateSpoofFilter(const char* tunTapName)
{
  FILE* procSpoof;
  char procFile[FILENAME_MAX];

  assert(tunTapName != NULL);

  /* Generate the procfile name */
  sprintf(procFile, "/proc/sys/net/ipv4/conf/%s/rp_filter", tunTapName);

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

  EthTapSpoofState = fgetc(procSpoof);
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
}

/* -------------------------------------------------------------------------
 * Function   : RestoreSpoofFilter
 * Description: Restores the Linux anti-spoofing filter setting for the tuntap
 *              interface
 * Input      : tunTapName - name used for the tuntap interface (e.g. "tun0" or "tap1")
 * Output     : none
 * Return     : none
 * Data Used  : EthTapSpoofState
 * ------------------------------------------------------------------------- */
static void RestoreSpoofFilter(const char* tunTapName)
{
  FILE* procSpoof;
  char procFile[FILENAME_MAX];

  assert(tunTapName != NULL);

  /* Generate the procfile name */
  sprintf(procFile, "/proc/sys/net/ipv4/conf/%s/rp_filter", tunTapName);

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
}

/* -------------------------------------------------------------------------
 * Function   : CreateCaptureSocket
 * Description: Create socket for promiscuously capturing multicast IP traffic
 * Input      : ifname - network interface (e.g. "eth0")
 * Output     : none
 * Return     : the socket descriptor ( >= 0), or -1 if an error occurred
 * Data Used  : none
 * Notes      : The socket is a raw packet socket, bound to the specified
 *              network interface
 * ------------------------------------------------------------------------- */
static int CreateCaptureSocket(const char* ifName)
{
  int ifIndex = if_nametoindex(ifName);
  struct packet_mreq mreq;
  struct ifreq req;
  struct sockaddr_ll bindTo;

  /* Open raw packet socket */
  int skfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (skfd < 0)
  {
    olsr_printf(1, "%s: socket(PF_PACKET) error: %s\n", PLUGIN_NAME, strerror(errno));
    return -1;
  }

  /* Set interface to promiscuous mode */
  memset(&mreq, 0, sizeof(struct packet_mreq));
  mreq.mr_ifindex = ifIndex;
  mreq.mr_type = PACKET_MR_PROMISC;
  if (setsockopt(skfd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
  {
    olsr_printf(1, "%s: setsockopt(PACKET_MR_PROMISC) error: %s\n", PLUGIN_NAME, strerror(errno));
    close(skfd);
    return -1;
  }

  /* Get hardware (MAC) address */
  memset(&req, 0, sizeof(struct ifreq));
  strncpy(req.ifr_name, ifName, IFNAMSIZ - 1);
  req.ifr_name[IFNAMSIZ-1] = '\0'; /* Ensures null termination */
  if (ioctl(skfd, SIOCGIFHWADDR, &req) < 0)
  {
    olsr_printf(1, "%s: error retrieving MAC address: %s\n", PLUGIN_NAME, strerror(errno));
    close(skfd);
    return -1;
  }
   
  /* Bind the socket to the specified interface */
  memset(&bindTo, 0, sizeof(bindTo));
  bindTo.sll_protocol = htons(ETH_P_ALL);
  bindTo.sll_ifindex = ifIndex;
  bindTo.sll_family = AF_PACKET;
  memcpy(bindTo.sll_addr, req.ifr_hwaddr.sa_data, IFHWADDRLEN);
  bindTo.sll_halen = IFHWADDRLEN;
    
  if (bind(skfd, (struct sockaddr*)&bindTo, sizeof(bindTo)) < 0)
  {
    olsr_printf(1, "%s: bind() error: %s\n", PLUGIN_NAME, strerror(errno));
    close(skfd);
    return -1;
  }

  /* Set socket to blocking operation */
  if (fcntl(skfd, F_SETFL, fcntl(skfd, F_GETFL, 0) & ~O_NONBLOCK) < 0)
  {
    olsr_printf(1, "%s: fcntl() error: %s\n", PLUGIN_NAME, strerror(errno));
    close(skfd);
    return -1;
  }

  /* Keep the highest-numbered descriptor */
  if (skfd > HighestSkfd)
  {
    HighestSkfd = skfd;
  }

  /* Add descriptor to input set */
  FD_SET(skfd, &InputSet);

  return skfd;
}

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
    olsr_printf(1, "%s: socket(PF_INET) error: %s\n", PLUGIN_NAME, strerror(errno));
    return -1;
  }

  /* Enable sending to broadcast addresses */
  if (setsockopt(skfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) < 0)
  {
    olsr_printf(1, "%s: setsockopt() error: %s\n", PLUGIN_NAME, strerror(errno));
    close(skfd);
    return -1;
  }
	
  /* Bind to the specific network interfaces indicated by ifName. */
  /* When using Kernel 2.6 this must happer prior to the port binding! */
  if (setsockopt(skfd, SOL_SOCKET, SO_BINDTODEVICE, ifName, strlen(ifName) + 1) < 0)
  {
    olsr_printf(1, "%s: setsockopt() error: %s\n", PLUGIN_NAME, strerror(errno));
    close(skfd);
    return -1;
  }

  /* Bind to port */
  memset(&bindTo, 0, sizeof(bindTo));
  bindTo.sin_family = AF_INET;
  bindTo.sin_port = htons(BMF_ENCAP_PORT);
  bindTo.sin_addr.s_addr = htonl(INADDR_ANY);
      
  if (bind(skfd, (struct sockaddr*)&bindTo, sizeof(bindTo)) < 0) 
  {
    olsr_printf(1, "%s: bind() error: %s\n", PLUGIN_NAME, strerror(errno));
    close(skfd);
    return -1;
  }

  /* Set socket to blocking operation */
  if (fcntl(skfd, F_SETFL, fcntl(skfd, F_GETFL, 0) & ~O_NONBLOCK) < 0)
  {
    olsr_printf(1, "%s: fcntl() error: %s\n", PLUGIN_NAME, strerror(errno));
    close(skfd);
    return -1;
  }

  /* Keep the highest-numbered descriptor */
  if (skfd > HighestSkfd)
  {
    HighestSkfd = skfd;
  }

  /* Add descriptor to input set */
  FD_SET(skfd, &InputSet);

  return skfd;
}

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
  static char* deviceName = "/dev/net/tun";
  struct ifreq ifreq;
  int etfd;
  int ioctl_s;
  int ioctlres;

  etfd = open(deviceName, O_RDWR | O_NONBLOCK);
  if (etfd < 0)
  {
    olsr_printf(1, "%s: error opening %s: %s\n", PLUGIN_NAME, deviceName, strerror(errno));
    return -1;
  }

  memset(&ifreq, 0, sizeof(ifreq));
  strncpy(ifreq.ifr_name, EtherTunTapIfName, IFNAMSIZ - 1);
  ifreq.ifr_name[IFNAMSIZ - 1] = '\0'; /* Ensures null termination */

  /* Specify either the IFF_TAP flag for Ethernet frames, or the IFF_TUN flag for IP.
   * Specify IFF_NO_PI for not receiving extra meta packet information. */
  if (TunOrTap == TT_TUN)
  {
    ifreq.ifr_flags = IFF_TUN;
  }
  else
  {
    ifreq.ifr_flags = IFF_TAP;
  }
  ifreq.ifr_flags |= IFF_NO_PI;

  if (ioctl(etfd, TUNSETIFF, (void *)&ifreq) < 0)
  {
    olsr_printf(1, "%s: ioctl(TUNSETIFF) error on %s: %s\n", PLUGIN_NAME, deviceName, strerror(errno));
    close(etfd);
    return -1;
  }

  memset(&ifreq, 0, sizeof(ifreq));
  strncpy(ifreq.ifr_name, EtherTunTapIfName, IFNAMSIZ - 1);
  ifreq.ifr_name[IFNAMSIZ - 1] = '\0'; /* Ensures null termination */
  ifreq.ifr_addr.sa_family = AF_INET;

  ioctl_s = socket(PF_INET, SOCK_DGRAM, 0);
  if (ioctl_s < 0)
  {
    olsr_printf(1, "%s: socket(PF_INET) error on %s: %s\n", PLUGIN_NAME, deviceName, strerror(errno));
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
        EtherTunTapIp = ntohl(((struct sockaddr_in*)&bmfIf->intAddr)->sin_addr.s_addr);
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

  ((struct sockaddr_in*)&ifreq.ifr_addr)->sin_addr.s_addr = htonl(EtherTunTapIp);
  ioctlres = ioctl(ioctl_s, SIOCSIFADDR, &ifreq);
  if (ioctlres >= 0)
  {
    /* Set net mask */
    ((struct sockaddr_in*)&ifreq.ifr_netmask)->sin_addr.s_addr = htonl(EtherTunTapIpMask);
    ioctlres = ioctl(ioctl_s, SIOCSIFNETMASK, &ifreq);
    if (ioctlres >= 0)
    {
      /* Set broadcast IP */
      ((struct sockaddr_in*)&ifreq.ifr_broadaddr)->sin_addr.s_addr = htonl(EtherTunTapIpBroadcast);
      ioctlres = ioctl(ioctl_s, SIOCSIFBRDADDR, &ifreq);
      if (ioctlres >= 0)
      {
        /* Bring EtherTunTap interface up (if not already) */
        ioctlres = ioctl(ioctl_s, SIOCGIFFLAGS, &ifreq);
        if (ioctlres >= 0)
        {
          ifreq.ifr_flags |= (IFF_UP | IFF_RUNNING | IFF_BROADCAST);
          ioctlres = ioctl(ioctl_s, SIOCSIFFLAGS, &ifreq);
        }
      }
    }
  }

  if (ioctlres < 0)
  {
    /* Any of the above ioctl() calls failed */
    olsr_printf(
      1,
      "%s: error bringing up EtherTunTap interface \"%s\": %s\n",
      PLUGIN_NAME,
      EtherTunTapIfName,
      strerror(errno));

    close(etfd);
    close(ioctl_s);
    return -1;
  } /* if (ioctlres < 0) */

  /* Set the multicast flag on the interface */
  memset(&ifreq, 0, sizeof(ifreq));
  strncpy(ifreq.ifr_name, EtherTunTapIfName, IFNAMSIZ - 1);
  ifreq.ifr_name[IFNAMSIZ - 1] = '\0'; /* Ensures null termination */

  ioctlres = ioctl(ioctl_s, SIOCGIFFLAGS, &ifreq);
  if (ioctlres >= 0)
  {
    ifreq.ifr_flags |= IFF_MULTICAST;
    ioctlres = ioctl(ioctl_s, SIOCSIFFLAGS, &ifreq);
  }
  if (ioctlres < 0)
  {
    /* Any of the two above ioctl() calls failed */
    olsr_printf(
      1,
      "%s: error setting multicast flag on EtherTunTap interface \"%s\": %s\n",
      PLUGIN_NAME,
      EtherTunTapIfName,
      strerror(errno));
    /* Continue anyway */
  }

  /* Use ioctl to make the tuntap persistent. Otherwise it will disappear
   * when this program exits. That is not desirable, since a multicast
   * daemon (e.g. mrouted) may be using the tuntap interface. */
  if (ioctl(etfd, TUNSETPERSIST, (void *)&ifreq) < 0)
  {
    olsr_printf(
      1,
      "%s: error making EtherTunTap interface \"%s\" persistent: %s\n",
      PLUGIN_NAME,
      EtherTunTapIfName,
      strerror(errno));
    /* Continue anyway */
  }

  /* Deactivate IP spoof filter for EtherTunTap interface */
  DeactivateSpoofFilter(EtherTunTapIfName);

  OLSR_PRINTF(9, "%s: opened 1 socket on \"%s\"\n", PLUGIN_NAME_SHORT, EtherTunTapIfName);

  /* Keep the highest-numbered descriptor */
  if (etfd > HighestSkfd)
  {
    HighestSkfd = etfd;
  }

  /* Add descriptor to input set */
  FD_SET(etfd, &InputSet);

  /* If the user configured a specific IP address for the BMF network interface,
   * help the user and advertise the IP address of the BMF network interface
   * on the OLSR network */
  if (TunTapIpOverruled != 0)
  {
    union olsr_ip_addr temp_net;
    union olsr_ip_addr temp_netmask;

    temp_net.v4 = htonl(EtherTunTapIp);
    temp_netmask.v4 = htonl(0xFFFFFFFF);
    add_local_hna4_entry(&temp_net, &temp_netmask);
  }

  /* If the BMF network interface has a sensible IP address, it is a good idea
   * to route all multicast traffic through that interface */
  if (EtherTunTapIp != ETHERTUNTAPDEFAULTIP)
  {
    struct rtentry kernel_route;

    memset(&kernel_route, 0, sizeof(struct rtentry));

    ((struct sockaddr_in*)&kernel_route.rt_dst)->sin_family = AF_INET;
    ((struct sockaddr_in*)&kernel_route.rt_gateway)->sin_family = AF_INET;
    ((struct sockaddr_in*)&kernel_route.rt_genmask)->sin_family = AF_INET;

    /* 224.0.0.0/4 */
    ((struct sockaddr_in *)&kernel_route.rt_dst)->sin_addr.s_addr = htonl(0xE0000000);
    ((struct sockaddr_in *)&kernel_route.rt_genmask)->sin_addr.s_addr = htonl(0xF0000000);

    kernel_route.rt_metric = 0;
    kernel_route.rt_flags = RTF_UP;

    kernel_route.rt_dev = EtherTunTapIfName;

    if (ioctl(ioctl_s, SIOCADDRT, &kernel_route) < 0)
    {
      olsr_printf(
        1,
        "%s: error setting multicast route via EtherTunTap interface \"%s\": %s\n",
        PLUGIN_NAME,
        EtherTunTapIfName,
        strerror(errno));
      /* Continue anyway */
    }
  }

  close(ioctl_s);

  return etfd;
}

#if 0
/* -------------------------------------------------------------------------
 * Function   : IsNullMacAddress
 * Description: Checks if a MAC address is all-zeroes
 * Input      : mac - address to check
 * Output     : none
 * Return     : true (1) or false (0)
 * Data Used  : none
 * ------------------------------------------------------------------------- */
static int IsNullMacAddress(char* mac)
{
  int i;

  assert(mac != NULL);

  for (i = 0; i < IFHWADDRLEN; i++)
  {
    if (mac[i] != 0) return 0;
  }
  return 1;
}
#endif

/* -------------------------------------------------------------------------
 * Function   : CreateInterface
 * Description: Create a new TBmfInterface object and adds it to the global
 *              BmfInterfaces list
 * Input      : ifName - name of the network interface (e.g. "eth0")
 *            : olsrIntf - OLSR interface object of the network interface, or
 *                NULL if the network interface is not OLSR-enabled
 * Output     : none
 * Return     : the number of opened sockets
 * Data Used  : none
 * ------------------------------------------------------------------------- */
static int CreateInterface(
  const char* ifName,
  struct interface* olsrIntf)
{
  int capturingSkfd = -1;
  int encapsulatingSkfd = -1;
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
    /* On OLSR interfaces, create socket for encapsulating and forwarding 
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
      close(encapsulatingSkfd);
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
    olsr_printf(
      1,
      "%s: ioctl(SIOCGIFHWADDR) error for interface \"%s\": %s\n",
      PLUGIN_NAME,
      ifName,
      strerror(errno));
    close(capturingSkfd);
    close(encapsulatingSkfd);
    free(newIf);
    return 0;
  }

  /* If null-interface, cancel the whole creation and return NULL */
  /* -- Not needed, all goes well with interfaces that have a
   * null-address, such as ppp interfaces. */
  /*
  if (IsNullMacAddress(ifr.ifr_hwaddr.sa_data))
  {
    close(capturingSkfd);
    close(encapsulatingSkfd);
    free(newIf);
    return 0;
  }
  */

  /* Copy data into TBmfInterface object */
  newIf->capturingSkfd = capturingSkfd;
  newIf->encapsulatingSkfd = encapsulatingSkfd;
  memcpy(newIf->macAddr, ifr.ifr_hwaddr.sa_data, IFHWADDRLEN);
  memcpy(newIf->ifName, ifName, IFNAMSIZ);
  newIf->olsrIntf = olsrIntf;
  if (olsrIntf != NULL)
  {
    /* For an OLSR-interface, copy the interface address and broadcast
     * address from the OLSR interface object */
    newIf->intAddr = olsrIntf->int_addr;
    newIf->broadAddr = olsrIntf->int_broadaddr;
  }
  else
  {
    /* For a non-OLSR interface, retrieve the IP address ourselves */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, ifName, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0'; /* Ensures null termination */
    if (ioctl(ioctlSkfd, SIOCGIFADDR, &ifr) < 0) 
    {
      olsr_printf(
        1,
        "%s: ioctl(SIOCGIFADDR) error for interface \"%s\": %s\n",
        PLUGIN_NAME,
        ifName,
        strerror(errno));

      ((struct sockaddr_in*)&newIf->intAddr)->sin_addr.s_addr = inet_addr("0.0.0.0");
	  }
	  else
	  {
      newIf->intAddr = ifr.ifr_addr;
    }

    /* For a non-OLSR interface, retrieve the IP broadcast address ourselves */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, ifName, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0'; /* Ensures null termination */
    if (ioctl(ioctlSkfd, SIOCGIFBRDADDR, &ifr) < 0) 
    {
      olsr_printf(
        1,
        "%s: ioctl(SIOCGIFBRDADDR) error for interface \"%s\": %s\n",
        PLUGIN_NAME,
        ifName,
        strerror(errno));

      ((struct sockaddr_in*)&newIf->broadAddr)->sin_addr.s_addr = inet_addr("0.0.0.0");
	  }
	  else
	  {
      newIf->broadAddr = ifr.ifr_broadaddr;
    }
  }

  /* Initialize fragment history table */
  memset(&newIf->fragmentHistory, 0, sizeof(newIf->fragmentHistory));
  newIf->nextFragmentHistoryEntry = 0;

  /* Add new TBmfInterface object to global list */
  newIf->next = BmfInterfaces;
  BmfInterfaces = newIf;

  OLSR_PRINTF(
    9,
    "%s: opened %d socket%s on %s interface \"%s\"\n",
    PLUGIN_NAME_SHORT,
    nOpened,
    nOpened == 1 ? "" : "s",
    olsrIntf != NULL ? "OLSR" : "non-OLSR",
    ifName);

  return nOpened;
}

/* -------------------------------------------------------------------------
 * Function   : CreateBmfNetworkInterfaces
 * Description: Create a list of TBmfInterface objects, one for each network
 *              interface on which BMF runs
 * Input      : skipThisIntf - network interface to skip, if seen
 * Output     : none
 * Return     : fail (-1) or success (0)
 * Data Used  : none
 * ------------------------------------------------------------------------- */
int CreateBmfNetworkInterfaces(struct interface* skipThisIntf)
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
    olsr_printf(
      1,
      "%s: no inet socket available to retrieve interface list: %s\n",
      PLUGIN_NAME,
      strerror(errno));
    return -1;
  }

  /* Retrieve the network interface configuration list */
  ifc.ifc_buf = NULL;
  for (;;)
  {
    ifc.ifc_len = sizeof(struct ifreq) * numreqs;
    ifc.ifc_buf = realloc(ifc.ifc_buf, ifc.ifc_len);

    if (ioctl(skfd, SIOCGIFCONF, &ifc) < 0)
    {
      olsr_printf(1, "%s: ioctl(SIOCGIFCONF) error: %s\n", PLUGIN_NAME, strerror(errno));

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
    struct interface* olsrIntf;

    /* ...find the OLSR interface structure, if any */
    union olsr_ip_addr ipAddr;
    COPY_IP(&ipAddr, &((struct sockaddr_in*)&ifr->ifr_addr)->sin_addr.s_addr);
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
    nOpenedSockets++;
  }

  if (BmfInterfaces == NULL)
  {
    olsr_printf(1, "%s: could not initialize any network interface\n", PLUGIN_NAME);
  }
  else
  {
    olsr_printf(1, "%s: opened %d sockets\n", PLUGIN_NAME, nOpenedSockets);
  }
  return 0;
}

/* -------------------------------------------------------------------------
 * Function   : AddInterface
 * Description: Add an OLSR-enabled network interface to the list of BMF-enabled
 *              network interfaces
 * Input      : newIntf - network interface to add
 * Output     : none
 * Return     : none
 * Data Used  : none
 * ------------------------------------------------------------------------- */
void AddInterface(struct interface* newIntf)
{
  int nOpened;

  assert(newIntf != NULL);

  nOpened = CreateInterface(newIntf->int_name, newIntf);

  olsr_printf(1, "%s: opened %d sockets\n", PLUGIN_NAME, nOpened);
}

/* -------------------------------------------------------------------------
 * Function   : CloseBmfNetworkInterfaces
 * Description: Closes every socket on each network interface used by BMF
 * Input      : newIntf - network interface to add
 * Output     : none
 * Return     : none
 * Data Used  : none
 * Notes      : Closes
 *              - the local EtherTunTap interface (e.g. "tun0" or "tap0")
 *              - for each BMF-enabled interface, the socket used for
 *                capturing multicast packets
 *              - for each OLSR-enabled interface, the socket used for
 *                encapsulating packets
 *              Also restores the network state to the situation before BMF
 *              was started.
 * ------------------------------------------------------------------------- */

void CloseBmfNetworkInterfaces()
{
  int nClosed = 0;
  
  /* Close all opened sockets */
  struct TBmfInterface* nextBmfIf = BmfInterfaces;
  while (nextBmfIf != NULL)
  {
    struct TBmfInterface* bmfIf = nextBmfIf;
    nextBmfIf = bmfIf->next;

    if (bmfIf->capturingSkfd >= 0)
    {
      close(bmfIf->capturingSkfd);
      nClosed++;
    }
    if (bmfIf->encapsulatingSkfd >= 0) 
    {
      close(bmfIf->encapsulatingSkfd);
      nClosed++;
    }

    OLSR_PRINTF(
      9,
      "%s: closed %s interface \"%s\"\n", 
      PLUGIN_NAME_SHORT,
      bmfIf->olsrIntf != NULL ? "OLSR" : "non-OLSR",
      bmfIf->ifName);

    free(bmfIf);
  }
  
  if (EtherTunTapFd >= 0)
  {
    /* Restore IP spoof filter for EtherTunTap interface */
    RestoreSpoofFilter(EtherTunTapIfName);

    close(EtherTunTapFd);
    nClosed++;

    OLSR_PRINTF(9, "%s: closed \"%s\"\n", PLUGIN_NAME_SHORT, EtherTunTapIfName);
  }

  BmfInterfaces = NULL;

  olsr_printf(1, "%s: closed %d sockets\n", PLUGIN_NAME, nClosed);

  /* If there is a multicast route, delete it */
  if (EtherTunTapIp != ETHERTUNTAPDEFAULTIP)
  {
    struct rtentry kernel_route;

    memset(&kernel_route, 0, sizeof(struct rtentry));

    ((struct sockaddr_in*)&kernel_route.rt_dst)->sin_family = AF_INET;
    ((struct sockaddr_in*)&kernel_route.rt_gateway)->sin_family = AF_INET;
    ((struct sockaddr_in*)&kernel_route.rt_genmask)->sin_family = AF_INET;

    /* 224.0.0.0/4 */
    ((struct sockaddr_in *)&kernel_route.rt_dst)->sin_addr.s_addr = htonl(0xE0000000);
    ((struct sockaddr_in *)&kernel_route.rt_genmask)->sin_addr.s_addr = htonl(0xF0000000);

    kernel_route.rt_metric = 0;
    kernel_route.rt_flags = RTF_UP;

    kernel_route.rt_dev = EtherTunTapIfName;

    if (ioctl(olsr_cnf->ioctl_s, SIOCDELRT, &kernel_route) < 0)
    {
      olsr_printf(
        1,
        "%s: error deleting multicast route via EtherTunTap interface \"%s\": %s\n",
        PLUGIN_NAME,
        EtherTunTapIfName,
        strerror(errno));
      /* Continue anyway */
    }
  }
}

#define MAX_NON_OLSR_IFS 32
static char NonOlsrIfNames[MAX_NON_OLSR_IFS][IFNAMSIZ];
static int nNonOlsrIfs = 0;

/* -------------------------------------------------------------------------
 * Function   : AddNonOlsrBmfIf
 * Description: Add an non-OLSR enabled network interface to the list of BMF-enabled
 *              network interfaces
 * Input      : ifName - network interface (e.g. "eth0")
 * Output     : none
 * Return     : fail (0) or success (1)
 * Data Used  : none
 * ------------------------------------------------------------------------- */
int AddNonOlsrBmfIf(const char* ifName)
{
  assert(ifName != NULL);

  if (nNonOlsrIfs >= MAX_NON_OLSR_IFS)
  {
    olsr_printf(
      1,
      "%s: too many non-OLSR interfaces specified, maximum is %d\n",
      PLUGIN_NAME,
      MAX_NON_OLSR_IFS);
    return 0;
  }

  strncpy(NonOlsrIfNames[nNonOlsrIfs], ifName, IFNAMSIZ - 1);
  NonOlsrIfNames[nNonOlsrIfs][IFNAMSIZ - 1] = '\0'; /* Ensures null termination */
  nNonOlsrIfs++;
  return 1;
}

/* -------------------------------------------------------------------------
 * Function   : IsNonOlsrBmfIf
 * Description: Checks if a network interface is OLSR-enabled
 * Input      : ifName - network interface (e.g. "eth0")
 * Output     : none
 * Return     : true (1) or false (0)
 * Data Used  : none
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
}

/* -------------------------------------------------------------------------
 * Function   : CheckAndUpdateLocalBroadcast
 * Description: For an IP packet, check if the destination address is not a
 *              multicast address. If it is not, the packet is assumed to be
 *              a local broadcast packet. Update the destination address to
 *              match the passed network interface.
 * Input      : buffer - the ethernet-IP packet
 *              broadAddr - the broadcast address to fill in
 * Output     : none
 * Return     : none
 * Data Used  : none
 * Notes      : See also RFC1141
 * ------------------------------------------------------------------------- */
void CheckAndUpdateLocalBroadcast(unsigned char* buffer, struct sockaddr* broadAddr)
{
  struct iphdr* iph;
  union olsr_ip_addr destIp;

  assert(buffer != NULL && broadAddr != NULL);

  iph = (struct iphdr*) (buffer + IP_HDR_OFFSET);
  COPY_IP(&destIp, &iph->daddr);
  if (! IsMulticast(&destIp))
  {
    u_int32_t origDaddr, newDaddr;
    struct sockaddr_in* sin;
    u_int32_t check;

    /* Cast down to correct sockaddr subtype */
    sin = (struct sockaddr_in*)broadAddr;
    
    origDaddr = ntohl(iph->daddr);

    iph->daddr = sin->sin_addr.s_addr;
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

      int ipHeaderLen = GetIpHeaderLength(buffer);
      struct udphdr* udph = (struct udphdr*) (buffer + IP_HDR_OFFSET + ipHeaderLen);

      /* RFC 1624, Eq. 3: HC' = ~(~HC - m + m') */

      check = ntohs(udph->check);

      check = ~ (~ check - ((origDaddr >> 16) & 0xFFFF) + ((newDaddr >> 16) & 0xFFFF));
      check = ~ (~ check - (origDaddr & 0xFFFF) + (newDaddr & 0xFFFF));

      /* Add carry */
      check = check + (check >> 16);

      udph->check = htons(check);
     }
  }
}

