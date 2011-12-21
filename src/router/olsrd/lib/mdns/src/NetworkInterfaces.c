/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004-2009, the olsr.org team - see HISTORY file
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


#include "NetworkInterfaces.h"

/* System includes */
#include <stddef.h>             /* NULL */
#include <syslog.h>             /* syslog() */
#include <string.h>             /* strerror(), strchr(), strcmp() */
#include <errno.h>              /* errno */
#include <unistd.h>             /* close() */
#include <sys/ioctl.h>          /* ioctl() */
#include <fcntl.h>              /* fcntl() */
#include <assert.h>             /* assert() */
#include <net/if.h>             /* socket(), ifreq, if_indextoname(), if_nametoindex() */
#include <netinet/in.h>         /* htons() */
#if defined linux
#include <linux/if_ether.h>     /* ETH_P_IP */
#include <linux/if_packet.h>    /* packet_mreq, PACKET_MR_PROMISC, PACKET_ADD_MEMBERSHIP */
#include <linux/if_tun.h>       /* IFF_TAP */
#endif
#include <netinet/ip.h>         /* struct ip */
#include <netinet/udp.h>        /* SOL_UDP */
#include <stdlib.h>             /* atoi, malloc */

/* OLSRD includes */
#include "olsr.h"               /* OLSR_PRINTF() */
#include "ipcalc.h"
#include "defs.h"               /* olsr_cnf */
#include "link_set.h"           /* get_link_set() */
#include "tc_set.h"             /* olsr_lookup_tc_entry(), olsr_lookup_tc_edge() */
#include "net_olsr.h"           /* ipequal */
#include "lq_plugin.h"
//#include "olsr_ip_prefix_list.h"

/* Plugin includes */
#include "Packet.h"             /* IFHWADDRLEN */
#include "mdns.h"               /* PLUGIN_NAME, MainAddressOf() */
#include "Address.h"            /* IsMulticast() */

int my_MDNS_TTL = 0;

/* List of network interface objects used by BMF plugin */
struct TBmfInterface *BmfInterfaces = NULL;
struct TBmfInterface *LastBmfInterface = NULL;

/* Highest-numbered open socket file descriptor. To be used as first
 * parameter in calls to select(...). */
int HighestSkfd = -1;

/* Set of socket file descriptors */
fd_set InputSet;


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
static int
CreateCaptureSocket(const char *ifName)
{
  int ifIndex = if_nametoindex(ifName);
  struct packet_mreq mreq;
  struct ifreq req;
  struct sockaddr_ll bindTo;
  int skfd = 0;
  /* Open cooked IP packet socket */
  if (olsr_cnf->ip_version == AF_INET) {
    skfd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));
  } else {
    skfd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IPV6));
  }
  if (skfd < 0) {
    BmfPError("socket(PF_PACKET) error");
    return -1;
  }

  /* Set interface to promiscuous mode */
  memset(&mreq, 0, sizeof(struct packet_mreq));
  mreq.mr_ifindex = ifIndex;
  mreq.mr_type = PACKET_MR_PROMISC;
  if (setsockopt(skfd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
    BmfPError("setsockopt(PACKET_MR_PROMISC) error");
    close(skfd);
    return -1;
  }

  /* Get hardware (MAC) address */
  memset(&req, 0, sizeof(struct ifreq));
  strncpy(req.ifr_name, ifName, IFNAMSIZ - 1);
  req.ifr_name[IFNAMSIZ - 1] = '\0';    /* Ensures null termination */
  if (ioctl(skfd, SIOCGIFHWADDR, &req) < 0) {
    BmfPError("error retrieving MAC address");
    close(skfd);
    return -1;
  }

  /* Bind the socket to the specified interface */
  memset(&bindTo, 0, sizeof(bindTo));
  bindTo.sll_family = AF_PACKET;
  if (olsr_cnf->ip_version == AF_INET) {
    bindTo.sll_protocol = htons(ETH_P_IP);
  } else {
    bindTo.sll_protocol = htons(ETH_P_IPV6);
  }
  bindTo.sll_ifindex = ifIndex;
  memcpy(bindTo.sll_addr, req.ifr_hwaddr.sa_data, IFHWADDRLEN);
  bindTo.sll_halen = IFHWADDRLEN;

  if (bind(skfd, (struct sockaddr *)&bindTo, sizeof(bindTo)) < 0) {
    BmfPError("bind() error");
    close(skfd);
    return -1;
  }

  /* Set socket to blocking operation */
  if (fcntl(skfd, F_SETFL, fcntl(skfd, F_GETFL, 0) & ~O_NONBLOCK) < 0) {
    BmfPError("fcntl() error");
    close(skfd);
    return -1;
  }
  //AddDescriptorToInputSet(skfd);
  add_olsr_socket(skfd, &DoMDNS,NULL, NULL, SP_PR_READ);

  return skfd;
}                               /* CreateCaptureSocket */

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

//FOR MDNS IS ALWAYS CALLED WITH NULL AS SECOND ARG

static int
CreateInterface(const char *ifName, struct interface *olsrIntf)
{
  int capturingSkfd = -1;
  int encapsulatingSkfd = -1;
  int listeningSkfd = -1;
  int ioctlSkfd;
  struct ifreq ifr;
  int nOpened = 0;
  struct TBmfInterface *newIf = olsr_malloc(sizeof(struct TBmfInterface), "TBMFInterface (mdns)");

  assert(ifName != NULL);

  if (newIf == NULL) {
    return 0;
  }
//TODO: assert interface is not talking OLSR


  /* Create socket for capturing and sending of multicast packets on
   * non-OLSR interfaces, and on OLSR-interfaces if configured. */
  if ((olsrIntf == NULL)) {
    capturingSkfd = CreateCaptureSocket(ifName);
    if (capturingSkfd < 0) {
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
  ifr.ifr_name[IFNAMSIZ - 1] = '\0';    /* Ensures null termination */
  if (ioctl(ioctlSkfd, SIOCGIFHWADDR, &ifr) < 0) {
    BmfPError("ioctl(SIOCGIFHWADDR) error for interface \"%s\"", ifName);
    close(capturingSkfd);
    close(encapsulatingSkfd);
    free(newIf);
    return 0;
  }

  /* Copy data into TBmfInterface object */
  newIf->capturingSkfd = capturingSkfd;
  newIf->encapsulatingSkfd = encapsulatingSkfd;
  newIf->listeningSkfd = listeningSkfd;
  memcpy(newIf->macAddr, ifr.ifr_hwaddr.sa_data, IFHWADDRLEN);
  memcpy(newIf->ifName, ifName, IFNAMSIZ);
  newIf->olsrIntf = olsrIntf;
  if (olsrIntf != NULL) {
    /* For an OLSR-interface, copy the interface address and broadcast
     * address from the OLSR interface object. Downcast to correct sockaddr
     * subtype. */
    newIf->intAddr.v4 = olsrIntf->int_addr.sin_addr;
    newIf->broadAddr.v4 = olsrIntf->int_broadaddr.sin_addr;
  } else {
    /* For a non-OLSR interface, retrieve the IP address ourselves */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, ifName, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';  /* Ensures null termination */
    if (ioctl(ioctlSkfd, SIOCGIFADDR, &ifr) < 0) {
      BmfPError("ioctl(SIOCGIFADDR) error for interface \"%s\"", ifName);

      newIf->intAddr.v4.s_addr = inet_addr("0.0.0.0");
    } else {
      /* Downcast to correct sockaddr subtype */
      newIf->intAddr.v4 = ((struct sockaddr_in *)ARM_NOWARN_ALIGN(&ifr.ifr_addr))->sin_addr;
    }

    /* For a non-OLSR interface, retrieve the IP broadcast address ourselves */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, ifName, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';  /* Ensures null termination */
    if (ioctl(ioctlSkfd, SIOCGIFBRDADDR, &ifr) < 0) {
      BmfPError("ioctl(SIOCGIFBRDADDR) error for interface \"%s\"", ifName);

      newIf->broadAddr.v4.s_addr = inet_addr("0.0.0.0");
    } else {
      /* Downcast to correct sockaddr subtype */
      newIf->broadAddr.v4 = ((struct sockaddr_in *)ARM_NOWARN_ALIGN(&ifr.ifr_broadaddr))->sin_addr;
    }
  }

  /* Initialize fragment history table */
  //memset(&newIf->fragmentHistory, 0, sizeof(newIf->fragmentHistory));
  //newIf->nextFragmentHistoryEntry = 0;

  /* Reset counters */
  //newIf->nBmfPacketsRx = 0;
  //newIf->nBmfPacketsRxDup = 0;
  //newIf->nBmfPacketsTx = 0;

  /* Add new TBmfInterface object to global list. OLSR interfaces are
   * added at the front of the list, non-OLSR interfaces at the back. */
  if (BmfInterfaces == NULL) {
    /* First TBmfInterface object in list */
    newIf->next = NULL;
    BmfInterfaces = newIf;
    LastBmfInterface = newIf;
  } else if (olsrIntf != NULL) {
    /* Add new TBmfInterface object at front of list */
    newIf->next = BmfInterfaces;
    BmfInterfaces = newIf;
  } else {
    /* Add new TBmfInterface object at back of list */
    newIf->next = NULL;
    LastBmfInterface->next = newIf;
    LastBmfInterface = newIf;
  }

  //OLSR_PRINTF(
  //  8,
  //  "%s: opened %d socket%s on %s interface \"%s\"\n",
  //  PLUGIN_NAME_SHORT,
  //  nOpened,
  //  nOpened == 1 ? "" : "s",
  //  olsrIntf != NULL ? "OLSR" : "non-OLSR",
  //  ifName);

  return nOpened;
}                               /* CreateInterface */

/* -------------------------------------------------------------------------
 * Function   : CreateBmfNetworkInterfaces
 * Description: Create a list of TBmfInterface objects, one for each network
 *              interface on which BMF runs
 * Input      : skipThisIntf - network interface to skip, if seen
 * Output     : none
 * Return     : fail (-1) or success (0)
 * Data Used  : none
 * ------------------------------------------------------------------------- */
int
CreateBmfNetworkInterfaces(struct interface *skipThisIntf)
{
  int skfd;
  struct ifconf ifc;
  int numreqs = 30;
  struct ifreq *ifr;
  int n;
  int nOpenedSockets = 0;

  /* Clear input descriptor set */
  FD_ZERO(&InputSet);

  skfd = socket(PF_INET, SOCK_DGRAM, 0);
  if (skfd < 0) {
    BmfPError("no inet socket available to retrieve interface list");
    return -1;
  }

  /* Retrieve the network interface configuration list */
  ifc.ifc_buf = NULL;
  for (;;) {
    ifc.ifc_len = sizeof(struct ifreq) * numreqs;
    ifc.ifc_buf = realloc(ifc.ifc_buf, ifc.ifc_len);

    if (ioctl(skfd, SIOCGIFCONF, &ifc) < 0) {
      BmfPError("ioctl(SIOCGIFCONF) error");

      close(skfd);
      free(ifc.ifc_buf);
      return -1;
    }
    if ((unsigned)ifc.ifc_len == sizeof(struct ifreq) * numreqs) {
      /* Assume it overflowed; double the space and try again */
      numreqs *= 2;
      assert(numreqs < 1024);
      continue;                 /* for (;;) */
    }
    break;                      /* for (;;) */
  }                             /* for (;;) */

  close(skfd);

  /* For each item in the interface configuration list... */
  ifr = ifc.ifc_req;
  for (n = ifc.ifc_len / sizeof(struct ifreq); --n >= 0; ifr++) {
    struct interface *olsrIntf;
    union olsr_ip_addr ipAddr;

    /* Skip the BMF network interface itself */
    //if (strncmp(ifr->ifr_name, EtherTunTapIfName, IFNAMSIZ) == 0)
    //{
    //  continue; /* for (n = ...) */
    //}

    /* ...find the OLSR interface structure, if any */
    ipAddr.v4 = ((struct sockaddr_in *)ARM_NOWARN_ALIGN(&ifr->ifr_addr))->sin_addr;
    olsrIntf = if_ifwithaddr(&ipAddr);

    if (skipThisIntf != NULL && olsrIntf == skipThisIntf) {
      continue;                 /* for (n = ...) */
    }

    if (olsrIntf == NULL && !IsNonOlsrBmfIf(ifr->ifr_name)) {
      /* Interface is neither OLSR interface, nor specified as non-OLSR BMF
       * interface in the BMF plugin parameter list */
      continue;                 /* for (n = ...) */
    }

    if (!IsNonOlsrBmfIf(ifr->ifr_name)) {
      //If the interface is not specified in the configuration file then go ahead
      continue;                 /* for (n = ...) */
    }
    //TODO: asser if->ifr_name is not talking OLSR
    //nOpenedSockets += CreateInterface(ifr->ifr_name, olsrIntf);
    nOpenedSockets += CreateInterface(ifr->ifr_name, NULL);

  }                             /* for (n = ...) */

  free(ifc.ifc_buf);

  if (BmfInterfaces == NULL) {
    //OLSR_PRINTF(1, "%s: could not initialize any network interface\n", PLUGIN_NAME);
  } else {
    //OLSR_PRINTF(1, "%s: opened %d sockets\n", PLUGIN_NAME, nOpenedSockets);
  }
  return 0;
}                               /* CreateBmfNetworkInterfaces */

/* -------------------------------------------------------------------------
 * Function   : AddInterface
 * Description: Add an OLSR-enabled network interface to the list of BMF-enabled
 *              network interfaces
 * Input      : newIntf - network interface to add
 * Output     : none
 * Return     : none
 * Data Used  : none
 * ------------------------------------------------------------------------- */
void
AddInterface(struct interface *newIntf)
{
  /* int nOpened; */

  assert(newIntf != NULL);

  /* nOpened = */ (void)CreateInterface(newIntf->int_name, newIntf);

  //OLSR_PRINTF(1, "%s: opened %d sockets\n", PLUGIN_NAME, nOpened);
}                               /* AddInterface */

/* -------------------------------------------------------------------------
 * Function   : CloseBmfNetworkInterfaces
 * Description: Closes every socket on each network interface used by BMF
 * Input      : none
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
void
CloseBmfNetworkInterfaces(void)
{
  int nClosed = 0;
  u_int32_t totalOlsrBmfPacketsRx = 0;
  u_int32_t totalOlsrBmfPacketsRxDup = 0;
  u_int32_t totalOlsrBmfPacketsTx = 0;
  u_int32_t totalNonOlsrBmfPacketsRx = 0;
  u_int32_t totalNonOlsrBmfPacketsRxDup = 0;
  u_int32_t totalNonOlsrBmfPacketsTx = 0;

  /* Close all opened sockets */
  struct TBmfInterface *nextBmfIf = BmfInterfaces;
  while (nextBmfIf != NULL) {
    struct TBmfInterface *bmfIf = nextBmfIf;
    nextBmfIf = bmfIf->next;

    if (bmfIf->capturingSkfd >= 0) {
      close(bmfIf->capturingSkfd);
      nClosed++;
    }
    if (bmfIf->encapsulatingSkfd >= 0) {
      close(bmfIf->encapsulatingSkfd);
      nClosed++;
    }
    //OLSR_PRINTF(
    //  7,
    //  "%s: %s interface \"%s\": RX pkts %u (%u dups); TX pkts %u\n",
    //  PLUGIN_NAME_SHORT,
    //  bmfIf->olsrIntf != NULL ? "OLSR" : "non-OLSR",
    //  bmfIf->ifName,
    //  bmfIf->nBmfPacketsRx,
    //  bmfIf->nBmfPacketsRxDup,
    //  bmfIf->nBmfPacketsTx);

    //OLSR_PRINTF(
    //  1,
    //  "%s: closed %s interface \"%s\"\n",
    //  PLUGIN_NAME_SHORT,
    //  bmfIf->olsrIntf != NULL ? "OLSR" : "non-OLSR",
    //  bmfIf->ifName);

    /* Add totals */
    if (bmfIf->olsrIntf != NULL) {
      totalOlsrBmfPacketsRx += bmfIf->nBmfPacketsRx;
      totalOlsrBmfPacketsRxDup += bmfIf->nBmfPacketsRxDup;
      totalOlsrBmfPacketsTx += bmfIf->nBmfPacketsTx;
    } else {
      totalNonOlsrBmfPacketsRx += bmfIf->nBmfPacketsRx;
      totalNonOlsrBmfPacketsRxDup += bmfIf->nBmfPacketsRxDup;
      totalNonOlsrBmfPacketsTx += bmfIf->nBmfPacketsTx;
    }

    free(bmfIf);
  }                             /* while */

  BmfInterfaces = NULL;

  //OLSR_PRINTF(1, "%s: closed %d sockets\n", PLUGIN_NAME_SHORT, nClosed);

}                               /* CloseBmfNetworkInterfaces */

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
int
AddNonOlsrBmfIf(const char *ifName, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  assert(ifName != NULL);

  if (nNonOlsrIfs >= MAX_NON_OLSR_IFS) {
    //OLSR_PRINTF(
    //  1,
    //  "%s: too many non-OLSR interfaces specified, maximum is %d\n",
    //  PLUGIN_NAME,
    //  MAX_NON_OLSR_IFS);
    return 1;
  }

  strncpy(NonOlsrIfNames[nNonOlsrIfs], ifName, IFNAMSIZ - 1);
  NonOlsrIfNames[nNonOlsrIfs][IFNAMSIZ - 1] = '\0';     /* Ensures null termination */
  nNonOlsrIfs++;
  return 0;
}                               /* AddNonOlsrBmfIf */


int
set_MDNS_TTL(const char *MDNS_TTL, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  assert(MDNS_TTL!= NULL);
  my_MDNS_TTL = atoi(MDNS_TTL);
  return 0;
}                               /* set_MDNS_TTL */
/* -------------------------------------------------------------------------
 * Function   : IsNonOlsrBmfIf
 * Description: Checks if a network interface is OLSR-enabled
 * Input      : ifName - network interface (e.g. "eth0")
 * Output     : none
 * Return     : true (1) or false (0)
 * Data Used  : NonOlsrIfNames
 * ------------------------------------------------------------------------- */
int
IsNonOlsrBmfIf(const char *ifName)
{
  int i;

  assert(ifName != NULL);

  for (i = 0; i < nNonOlsrIfs; i++) {
    if (strncmp(NonOlsrIfNames[i], ifName, IFNAMSIZ) == 0)
      return 1;
  }
  return 0;
}                               /* IsNonOlsrBmfIf */
