/*
 * DHCP library functions.
 *
 * Copyright (c) 2006, Jens Jakobsen 
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 *   Neither the names of copyright holders nor the names of its contributors
 *   may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * Copyright (C) 2003, 2004, 2005, 2006 Mondru AB.
 *
 * The contents of this file may be used under the terms of the GNU
 * General Public License Version 2, provided that the above copyright
 * notice and this permission notice is included in all copies or
 * substantial portions of the software.
 *
 */


/* Usage
 *
 * The library is initialised by calling dhcp_new(), which
 * initialises a dhcp_t struct that is used for all subsequent calls
 * to the library. Ressources are freed by calling dhcp_free().
 * 
 */

/* TODO
 *
 * Port to FreeBSD. 
 * - Mainly concerns Ethernet stuff.
 * 
 * Move EAPOL stuff to separate files
 *
 * Change static memory allocation to malloc
 * - Mainly concerns newconn() and freeconn()
 * - Wait until code is bug free.
 */

#include <stdlib.h>
#include <syslog.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h> /* ISO C99 types */
#include <arpa/inet.h>
#include <sys/socket.h>

#if defined(__linux__)
#include <linux/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>

#elif defined (__FreeBSD__)   || defined (__OpenBSD__) || defined (__NetBSD__) || defined (__APPLE__)
#include <net/if.h>
#include <net/bpf.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <ifaddrs.h>
#endif

#ifdef HAVE_NET_ETHERNET_H
#include <net/ethernet.h>
#endif

#ifdef HAVE_ASM_TYPES_H
#include <asm/types.h>
#endif

#include <net/if_arp.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "../config.h"
#include "syserr.h"
#include "ippool.h"
#include "iphash.h"
#include "dhcp.h"
#include "lookup.h"

#ifdef NAIVE
const static int paranoid = 0; /* Trust that the program has no bugs */
#else
const static int paranoid = 1; /* Check for errors which cannot happen */
#endif


/*
 *    BrainSlayer: 
 *    wrapper for fixing the big endian bugs within dhcp server code.its surelly not the best.
 *    all dhcp packet fields must be handled in little endian
 */

static uint16_t swap16(uint16_t word) {
#if __BYTE_ORDER == __BIG_ENDIAN
  unsigned char low = word>>8;
  unsigned char high = word&0xff;
  return ((uint16_t)(high<<8))|low;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
  return word;
#else
#error "Could not determine the system's endianness"
#endif
}

/**
 * dhcp_ip_check()
 * Generates an IPv4 header checksum.
 **/
int dhcp_ip_check(struct dhcp_ippacket_t *pack) {
  int i;
  uint32_t sum = 0;
  pack->iph.check = 0;
  for (i=0; i<(pack->iph.ihl * 2); i++) {
    sum += swap16(((uint16_t*) &pack->iph)[i]); /* brainslayer */
  }
  while (sum>>16)
    sum = (sum & 0xFFFF)+(sum >> 16);
  pack->iph.check = swap16(~sum); /* brainslayer */
  return 0;
}

/**
 * dhcp_udp_check()
 * Generates an UDP header checksum.
 **/
int dhcp_udp_check(struct dhcp_fullpacket_t *pack) {
  int i;
  uint32_t sum = 0;
  int udp_len = ntohs(pack->udph.len);

  pack->udph.check = 0;
  
  if (udp_len > DHCP_UDP_HLEN + DHCP_LEN) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Length of dhcp packet larger then %d: %d",
	    DHCP_UDP_HLEN + DHCP_LEN, udp_len);
    return -1; /* Packet too long */
  }

  /* Sum UDP header and payload */
		
  for (i=0; i<(udp_len/2); i++) {
    sum += swap16(((uint16_t*) &pack->udph)[i]); /* brainslayer */
  }


  if (udp_len & 0x01) {
    sum += ((uint8_t*) &pack->udph)[udp_len-1];
  }

  /* Sum both source and destination address */
  for (i=0; i<4; i++) {
    sum += swap16(((uint16_t*) &pack->iph.saddr)[i]); /* brainslayer */
  }

  /* Sum both protocol and udp_len (again) */
  sum = sum + swap16(pack->udph.len) + ((pack->iph.protocol<<8)&0xFF00); /* brainslayer */

  while (sum>>16)
    sum = (sum & 0xFFFF)+(sum >> 16);

  pack->udph.check = swap16(~sum); /* brainslayer */

  return 0;
}


/**
 * dhcp_tcp_check()
 * Generates an TCP header checksum.
 **/
int dhcp_tcp_check(struct dhcp_ippacket_t *pack, int length) {
  int i;
  uint32_t sum = 0;
  struct dhcp_tcphdr_t *tcph;
  int tcp_len;

  if (ntohs(pack->iph.tot_len) > (length - DHCP_ETH_HLEN))
    return -1; /* Wrong length of packet */

  tcp_len = ntohs(pack->iph.tot_len) - pack->iph.ihl * 4;

  if (tcp_len < 20) /* TODO */
    return -1; /* Packet too short */

  tcph = (struct dhcp_tcphdr_t*) pack->payload;
  tcph->check = 0;

  /* Sum TCP header and payload */
  for (i=0; i<(tcp_len/2); i++) {
    sum += swap16(((uint16_t*) pack->payload)[i]); /* brainslayer */
  }

  /* Sum any uneven payload octet */
  if (tcp_len & 0x01) {
    sum += ((uint8_t*) pack->payload)[tcp_len-1];
  }

  /* Sum both source and destination address */
  for (i=0; i<4; i++) {
    sum += swap16(((uint16_t*) &pack->iph.saddr)[i]); /* brainslayer */
  }

  /* Sum both protocol and tcp_len */
  sum = sum + swap16(htons(tcp_len)) + ((pack->iph.protocol<<8)&0xFF00); /* brainslayer */

  while (sum>>16)
    sum = (sum & 0xFFFF)+(sum >> 16);

  tcph->check = swap16(~sum); /* brainslayer */

  return 0;
}


int dhcp_sifflags(char const *devname, int flags) {
  struct ifreq ifr;
  int fd;
  
  memset (&ifr, '\0', sizeof (ifr));
  ifr.ifr_flags = flags;
  strncpy(ifr.ifr_name, devname, IFNAMSIZ);
  ifr.ifr_name[IFNAMSIZ-1] = 0; /* Make sure to terminate */
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "socket() failed");
  }
  if (ioctl(fd, SIOCSIFFLAGS, &ifr)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "ioctl(SIOCSIFFLAGS) failed");
    close(fd);
    return -1;
  }
  close(fd);
  return 0;
}

int dhcp_gifflags(char const *devname, int *flags) {
  struct ifreq ifr;
  int fd;
  
  memset (&ifr, '\0', sizeof (ifr));
  strncpy(ifr.ifr_name, devname, IFNAMSIZ);
  ifr.ifr_name[IFNAMSIZ-1] = 0; /* Make sure to terminate */
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "socket() failed");
  }
  if (ioctl(fd, SIOCGIFFLAGS, &ifr)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "ioctl(SIOCSIFFLAGS) failed");
    close(fd);
    return -1;
  }
  close(fd);
  *flags = ifr.ifr_flags;

  return 0;
}

int dhcp_setaddr(char const *devname,
		 struct in_addr *addr,
		 struct in_addr *dstaddr,
		 struct in_addr *netmask) {

  struct ifreq ifr;
  int fd;

  memset (&ifr, '\0', sizeof (ifr));
  ifr.ifr_addr.sa_family = AF_INET;
  ifr.ifr_dstaddr.sa_family = AF_INET;

#if defined(__linux__)
  ifr.ifr_netmask.sa_family = AF_INET;

#elif defined(__FreeBSD__) || defined (__OpenBSD__) || defined (__NetBSD__) || defined (__APPLE__)
  ((struct sockaddr_in *) &ifr.ifr_addr)->sin_len = 
    sizeof (struct sockaddr_in);
  ((struct sockaddr_in *) &ifr.ifr_dstaddr)->sin_len = 
    sizeof (struct sockaddr_in);
#endif

  strncpy(ifr.ifr_name, devname, IFNAMSIZ);
  ifr.ifr_name[IFNAMSIZ-1] = 0; /* Make sure to terminate */

  /* Create a channel to the NET kernel. */
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "socket() failed");
    return -1;
  }

  if (addr) { /* Set the interface address */
    ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr.s_addr = addr->s_addr;
    if (ioctl(fd, SIOCSIFADDR, (void *) &ifr) < 0) {
      if (errno != EEXIST) {
	sys_err(LOG_ERR, __FILE__, __LINE__, errno,
		"ioctl(SIOCSIFADDR) failed");
      }
      else {
	sys_err(LOG_WARNING, __FILE__, __LINE__, errno,
		"ioctl(SIOCSIFADDR): Address already exists");
      }
      close(fd);
      return -1;
    }
  }

  if (dstaddr) { /* Set the destination address */
    ((struct sockaddr_in *) &ifr.ifr_dstaddr)->sin_addr.s_addr = 
      dstaddr->s_addr;
    if (ioctl(fd, SIOCSIFDSTADDR, (caddr_t) &ifr) < 0) {
      sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	      "ioctl(SIOCSIFDSTADDR) failed");
      close(fd);
      return -1; 
    }
  }

  if (netmask) { /* Set the netmask */
#if defined(__linux__)
    ((struct sockaddr_in *) &ifr.ifr_netmask)->sin_addr.s_addr = 
      netmask->s_addr;

#elif defined(__FreeBSD__) || defined (__OpenBSD__) || defined (__NetBSD__) || defined (__APPLE__)
    ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr.s_addr = 
      netmask->s_addr;

#elif defined(__sun__)
    ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr.s_addr = 
      netmask->s_addr;
#else
#error  "Unknown platform!" 
#endif

    if (ioctl(fd, SIOCSIFNETMASK, (void *) &ifr) < 0) {
      sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	      "ioctl(SIOCSIFNETMASK) failed");
      close(fd);
      return -1;
    }
  }
  
  close(fd);
  
  /* On linux the route to the interface is set automatically
     on FreeBSD we have to do this manually */

  /* TODO: How does it work on Solaris? */

#if defined(__FreeBSD__) || defined (__OpenBSD__) || defined (__NetBSD__) || defined (__APPLE__)
  (void)dhcp_sifflags(devname, IFF_UP | IFF_RUNNING);  /* TODO */
  /*return tun_addroute(this, addr, addr, netmask);*/
#else
  return dhcp_sifflags(devname, IFF_UP | IFF_RUNNING); 
#endif

}

#if defined(__linux__)

int dhcp_getmac(const char *ifname, char *macaddr) {
  int fd;
  int option=1;
  struct ifreq ifr;

  memset(&ifr, 0, sizeof(ifr));

  /* Create socket */
  if ((fd = socket(PF_PACKET, SOCK_RAW, htons(DHCP_ETH_IP))) < 0) {
    if (errno == EPERM) {
      sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	      "Cannot create raw socket. Must be root.");
    }
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "socket(domain=%d, protocol=%lx, protocol=%d) failed",
	    PF_PACKET, SOCK_RAW, DHCP_ETH_IP);
  }

  /* Get the MAC address of our interface */
  strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
  if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "ioctl(d=%d, request=%d) failed",
	    fd, SIOCGIFHWADDR);
  }
  memcpy(macaddr, ifr.ifr_hwaddr.sa_data, DHCP_ETH_ALEN);
  if (ifr.ifr_hwaddr.sa_family != ARPHRD_ETHER) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Not Ethernet: %.16s", ifname);
  }
  
  if (macaddr[0] & 0x01) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0, 
	    "Ethernet has broadcast or multicast address: %.16s", ifname);
  }
  
  close(fd);
  
}

/**
 * dhcp_open_eth()
 * Opens an Ethernet interface. As an option the interface can be set in
 * promisc mode. If not null macaddr and ifindex are filled with the
 * interface mac address and index
 **/
int dhcp_open_eth(char const *ifname, uint16_t protocol, int promisc,
		  int usemac, unsigned char *macaddr, int *ifindex) {
  int fd;
  int option=1;
  struct ifreq ifr;
  struct packet_mreq mr;
  struct sockaddr_ll sa;

  memset(&ifr, 0, sizeof(ifr));

  /* Create socket */
  if ((fd = socket(PF_PACKET, SOCK_RAW, htons(protocol))) < 0) {
    if (errno == EPERM) {
      sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	      "Cannot create raw socket. Must be root.");
    }
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "socket(domain=%d, protocol=%lx, protocol=%d) failed",
	    PF_PACKET, SOCK_RAW, protocol);
  }


  /* Enable reception and transmission of broadcast frames */
  if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &option, sizeof(option)) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "setsockopt(s=%d, level=%d, optname=%d, optlen=%d) failed",
	    fd, SOL_SOCKET, SO_BROADCAST, sizeof(option));
  }
  

  /* Get the MAC address of our interface */
  if ((!usemac) && (macaddr)) {
    strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
    if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) {
      sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	      "ioctl(d=%d, request=%d) failed",
	      fd, SIOCGIFHWADDR);
    }
    memcpy(macaddr, ifr.ifr_hwaddr.sa_data, DHCP_ETH_ALEN);
    if (ifr.ifr_hwaddr.sa_family != ARPHRD_ETHER) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Not Ethernet: %.16s", ifname);
    }
    
    if (macaddr[0] & 0x01) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0, 
	      "Ethernet has broadcast or multicast address: %.16s", ifname);
    }
  }


  /* Verify that MTU = ETH_DATA_LEN */
  strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
  if (ioctl(fd, SIOCGIFMTU, &ifr) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "ioctl(d=%d, request=%d) failed",
	    fd, SIOCGIFMTU);
  }
  if (ifr.ifr_mtu != ETH_DATA_LEN) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "MTU does not match EHT_DATA_LEN: %d %d", 
	    ifr.ifr_mtu, ETH_DATA_LEN);
  }

  
  /* Get ifindex */
  strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
  if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "ioctl(SIOCFIGINDEX) failed");
  }
  if (ifindex)
    *ifindex = ifr.ifr_ifindex;
  
  
  /* Set interface in promisc mode */
  if (promisc) {
    memset(&mr,0,sizeof(mr));
    mr.mr_ifindex = ifr.ifr_ifindex;
    mr.mr_type =  PACKET_MR_PROMISC;
    if(setsockopt(fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP,
		  (char *)&mr, sizeof(mr)) < 0) {
      sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	      "setsockopt(s=%d, level=%d, optname=%d, optlen=%d) failed",
	      fd, SOL_SOCKET, PACKET_ADD_MEMBERSHIP, sizeof(mr));
    }
  }


  /* Bind to particular interface */
  memset(&sa, 0, sizeof(sa));
  sa.sll_family = AF_PACKET;
  sa.sll_protocol = htons(protocol);
  sa.sll_ifindex = ifr.ifr_ifindex;
  if (bind(fd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "bind(sockfd=%d) failed", fd);
  }
  return fd;
}

#elif defined (__FreeBSD__) || defined (__OpenBSD__) || defined (__NetBSD__) || defined (__APPLE__)


int dhcp_getmac(const char *ifname, char *macaddr) {

  struct ifaddrs *ifap, *ifa;
  struct sockaddr_dl *sdl;

  if (getifaddrs(&ifap)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno, "getifaddrs() failed!");
    return -1;
  }

  ifa = ifap;
  while (ifa) {
    if ((strcmp(ifa->ifa_name, ifname) == 0) &&
	(ifa->ifa_addr->sa_family == AF_LINK)) {
      sdl = (struct sockaddr_dl *)ifa->ifa_addr;
      switch(sdl->sdl_type) {
      case IFT_ETHER:
#ifdef IFT_IEEE80211
      case IFT_IEEE80211:
#endif
	break;
      default:
	continue;
      }
      if (sdl->sdl_alen != DHCP_ETH_ALEN) {
	sys_err(LOG_ERR, __FILE__, __LINE__, errno, "Wrong sdl_alen!");
	freeifaddrs(ifap);
	return -1;
      }
      memcpy(macaddr, LLADDR(sdl), DHCP_ETH_ALEN);
      freeifaddrs(ifap);
      return 0;
    }
    ifa = ifa->ifa_next;
  }  
  freeifaddrs(ifap);
  return -1;
}

/**
 * dhcp_open_eth()
 * Opens an Ethernet interface. As an option the interface can be set in
 * promisc mode. If not null macaddr and ifindex are filled with the
 * interface mac address and index
 **/

/* Relevant IOCTLs
FIONREAD Get the number of bytes in input buffer
SIOCGIFADDR Get interface address (IP)
BIOCGBLEN, BIOCSBLEN Get and set required buffer length
BIOCGDLT Type of underlying data interface
BIOCPROMISC Set in promisc mode
BIOCFLUSH Flushes the buffer of incoming packets
BIOCGETIF, BIOCSETIF Set hardware interface. Uses ift_name
BIOCSRTIMEOUT, BIOCGRTIMEOUT Set and get timeout for reads
BIOCGSTATS Return stats for the interface
BIOCIMMEDIATE Return immediately from reads as soon as packet arrives.
BIOCSETF Set filter
BIOCVERSION Return the version of BPF
BIOCSHDRCMPLT BIOCGHDRCMPLT Set flag of wheather to fill in MAC address
BIOCSSEESENT BIOCGSEESENT Return locally generated packets */

int dhcp_open_eth(char const *ifname, uint16_t protocol, int promisc,
		  int usemac, unsigned char *macaddr, int *ifindex) {

  char devname[IFNAMSIZ+5]; /* "/dev/" + ifname */
  int devnum;
  struct ifreq ifr;
  struct ifaliasreq areq;
  int fd;
  int local_fd;
  struct bpf_version bv;

  u_int32_t ipaddr;
  struct sockaddr_dl hwaddr;
  unsigned int value;

  /* Find suitable device */
  for (devnum = 0; devnum < 255; devnum++) { /* TODO 255 */ 
    snprintf(devname, sizeof(devname), "/dev/bpf%d", devnum);
    devname[sizeof(devname)] = 0;
    if ((fd = open(devname, O_RDWR)) >= 0) break;
    if (errno != EBUSY) break;
  } 
  if (fd < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno, "Can't find bpf device");
    return -1;
  }

  /* Set the interface */
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
  if (ioctl(fd, BIOCSETIF, &ifr) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,"ioctl() failed");
    return -1;
  }

  /* Get and validate BPF version */
  if (ioctl(fd, BIOCVERSION, &bv) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,"ioctl() failed!");
    return -1;
  }  
  if (bv.bv_major != BPF_MAJOR_VERSION ||
      bv.bv_minor < BPF_MINOR_VERSION) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,"wrong BPF version!");
    return -1;
  }

  /* Get the MAC address of our interface */
  if ((!usemac) && (macaddr)) {

    if (dhcp_getmac(ifname, macaddr)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,"Did not find MAC address!");
      return -1;
    }
    
    if (0) printf("MAC Address %.2x %.2x %.2x %.2x %.2x %.2x\n",
		  macaddr[0], macaddr[1], macaddr[2],
		  macaddr[3], macaddr[4], macaddr[5]);
    
    if (macaddr[0] & 0x01) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0, 
	      "Ethernet has broadcast or multicast address: %.16s", ifname);
      return -1;
    }
  }

  /* Set interface in promisc mode */
  if (promisc) {
    value = 1;
    if (ioctl(fd, BIOCPROMISC, NULL) < 0) {
      sys_err(LOG_ERR, __FILE__, __LINE__, errno,"ioctl() failed!");
      return -1;
    }  
    value = 1;
    if (ioctl(fd, BIOCSHDRCMPLT, &value) < 0) {
      sys_err(LOG_ERR, __FILE__, __LINE__, errno,"ioctl() failed!");
      return -1;
    }  
  }
  else {
    value = 0;
    if (ioctl(fd, BIOCSHDRCMPLT, &value) < 0) {
      sys_err(LOG_ERR, __FILE__, __LINE__, errno,"ioctl() failed!");
      return -1;
    }  
  }

  /* Make sure reads return as soon as packet has been received */
  value = 1;
  if (ioctl(fd, BIOCIMMEDIATE, &value) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,"ioctl() failed!");
    return -1;
  }  

  return fd;
}

#endif

int dhcp_send(struct dhcp_t *this, 
				int fd, uint16_t protocol, unsigned char *hismac, int ifindex,
	      void *packet, int length)
{
if (this->debug) printf("Sending IP packet\n");

#if defined(__linux__)
  struct sockaddr_ll dest;

  memset(&dest, '\0', sizeof(dest));
  dest.sll_family = AF_PACKET;
  dest.sll_protocol = htons(protocol);
  dest.sll_ifindex = ifindex;
  dest.sll_halen = DHCP_ETH_ALEN;
  memcpy (dest.sll_addr, hismac, DHCP_ETH_ALEN);

  if (sendto(fd, packet, (length), 0,
	     (struct sockaddr *)&dest ,sizeof(dest)) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "sendto(fd=%d, len=%d) failed",
	    fd, length);
    return -1;
  }
#elif defined(__FreeBSD__) || defined (__OpenBSD__) || defined (__NetBSD__) || defined (__APPLE__)
  if (write(fd, packet, length) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno, "write() failed");
    return -1;
  }
#endif
  return 0;
}




/**
 * dhcp_hash()
 * Generates a 32 bit hash based on a mac address
 **/
unsigned long int dhcp_hash(uint8_t *hwaddr) {
  return lookup(hwaddr, DHCP_ETH_ALEN, 0);
}


/**
 * dhcp_hashinit()
 * Initialises hash tables
 **/
int dhcp_hashinit(struct dhcp_t *this, int listsize) {
  /* Determine hashlog */
  for ((this)->hashlog = 0; 
       ((1 << (this)->hashlog) < listsize);
       (this)->hashlog++);
  
  /* Determine hashsize */
  (this)->hashsize = 1 << (this)->hashlog;
  (this)->hashmask = (this)->hashsize -1;
  
  /* Allocate hash table */
  if (!((this)->hash = calloc(sizeof(struct dhcp_conn_t), (this)->hashsize))){
    /* Failed to allocate memory for hash members */
    return -1;
  }
  return 0;
}


/**
 * dhcp_hashadd()
 * Adds a connection to the hash table
 **/
int dhcp_hashadd(struct dhcp_t *this, struct dhcp_conn_t *conn) {
  uint32_t hash;
  struct dhcp_conn_t *p;
  struct dhcp_conn_t *p_prev = NULL; 

  /* Insert into hash table */
  hash = dhcp_hash(conn->hismac) & this->hashmask;
  for (p = this->hash[hash]; p; p = p->nexthash)
    p_prev = p;
  if (!p_prev)
    this->hash[hash] = conn;
  else 
    p_prev->nexthash = conn;
  return 0; /* Always OK to insert */
}


/**
 * dhcp_hashdel()
 * Removes a connection from the hash table
 **/
int dhcp_hashdel(struct dhcp_t *this, struct dhcp_conn_t *conn) {
  uint32_t hash;
  struct dhcp_conn_t *p;
  struct dhcp_conn_t *p_prev = NULL; 

  /* Find in hash table */
  hash = dhcp_hash(conn->hismac) & this->hashmask;
  for (p = this->hash[hash]; p; p = p->nexthash) {
    if (p == conn) {
      break;
    }
    p_prev = p;
  }

  if ((paranoid) && (p!= conn)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Tried to delete connection not in hash table");
  }

  if (!p_prev)
    this->hash[hash] = p->nexthash;
  else
    p_prev->nexthash = p->nexthash;
  
  return 0;
}


/**
 * dhcp_hashget()
 * Uses the hash tables to find a connection based on the mac address.
 * Returns -1 if not found.
 **/
int dhcp_hashget(struct dhcp_t *this, struct dhcp_conn_t **conn,
		 uint8_t *hwaddr) {
  struct dhcp_conn_t *p;
  uint32_t hash;

  /* Find in hash table */
  hash = dhcp_hash(hwaddr) & this->hashmask;
  for (p = this->hash[hash]; p; p = p->nexthash) {
    if ((!memcmp(p->hismac, hwaddr, DHCP_ETH_ALEN)) && (p->inuse)) {
      *conn = p;
      return 0;
    }
  }
  *conn = NULL;
  return -1; /* Address could not be found */
}


/**
 * dhcp_validate()
 * Valides reference structures of connections. 
 * Returns the number of active connections
 **/
int dhcp_validate(struct dhcp_t *this)
{
  int used = 0;
  int unused = 0;
  struct dhcp_conn_t *conn;
  struct dhcp_conn_t *hash_conn;

  /* Count the number of used connections */
  conn = this->firstusedconn;
  while (conn) {
    if (!conn->inuse) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Connection with inuse == 0!");
    }
    (void)dhcp_hashget(this, &hash_conn, conn->hismac);
    if (conn != hash_conn) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Connection could not be found by hashget!");
    }
    used ++;
    conn = conn->next;
  }
  
  /* Count the number of unused connections */
  conn = this->firstfreeconn;
  while (conn) {
    if (conn->inuse) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Connection with inuse != 0!");
    }
    unused ++;
    conn = conn->next;
  }

  if (this->numconn != (used + unused)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "The number of free and unused connections does not match!");
    if (this->debug) {
      printf("used %d unused %d\n", used, unused);
      conn = this->firstusedconn;
      while (conn) {
	printf("%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n", 
	       conn->hismac[0], conn->hismac[1], conn->hismac[2],
	       conn->hismac[3], conn->hismac[4], conn->hismac[5]);
	conn = conn->next;
      }
    }
  }
  
  return used;
}


/**
 * dhcp_initconn()
 * Initialises connection references
 **/
int dhcp_initconn(struct dhcp_t *this)
{
  int n;
  this->firstusedconn = NULL; /* Redundant */
  this->lastusedconn  = NULL; /* Redundant */

  for (n=0; n<this->numconn; n++) {
    this->conn[n].inuse = 0; /* Redundant */
    if (n == 0) {
      this->conn[n].prev = NULL; /* Redundant */
      this->firstfreeconn = &this->conn[n];

    }
    else {
      this->conn[n].prev = &this->conn[n-1];
      this->conn[n-1].next = &this->conn[n];
    }
    if (n == (this->numconn-1)) {
      this->conn[n].next = NULL; /* Redundant */
      this->lastfreeconn  = &this->conn[n];
    }
  }

  if (paranoid) dhcp_validate(this);

  return 0;
}

/**
 * dhcp_newconn()
 * Allocates a new connection from the pool. 
 * Returns -1 if unsuccessful.
 **/
int dhcp_newconn(struct dhcp_t *this, struct dhcp_conn_t **conn, 
		 uint8_t *hwaddr)
{

  if (this->debug) 
    printf("DHCP newconn: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n", 
	   hwaddr[0], hwaddr[1], hwaddr[2],
	   hwaddr[3], hwaddr[4], hwaddr[5]);


  if (!this->firstfreeconn) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Out of free connections");
    return -1;
  }

  *conn = this->firstfreeconn;

  /* Remove from link of free */
  if (this->firstfreeconn->next) {
    this->firstfreeconn->next->prev = NULL;
    this->firstfreeconn = this->firstfreeconn->next;
  }
  else { /* Took the last one */
    this->firstfreeconn = NULL; 
    this->lastfreeconn = NULL;
  }

  /* Initialise structures */
  memset(*conn, 0, sizeof(**conn));

  /* Insert into link of used */
  if (this->firstusedconn) {
    this->firstusedconn->prev = *conn;
    (*conn)->next = this->firstusedconn;
  }
  else { /* First insert */
    this->lastusedconn = *conn;
  }

  this->firstusedconn = *conn;

  (*conn)->inuse = 1;
  (*conn)->parent = this;

  /* Application specific initialisations */
  memcpy((*conn)->hismac, hwaddr, DHCP_ETH_ALEN);
  memcpy((*conn)->ourmac, this->hwaddr, DHCP_ETH_ALEN);
  gettimeofday(&(*conn)->lasttime, NULL);
  (void)dhcp_hashadd(this, *conn);
  
  if (paranoid) dhcp_validate(this);

  /* Inform application that connection was created */
  if (this ->cb_connect)
    this ->cb_connect(*conn);
  
  return 0; /* Success */
}


/**
 * dhcp_freeconn()
 * Returns a connection to the pool. 
 **/
int dhcp_freeconn(struct dhcp_conn_t *conn)
{
  /* TODO: Always returns success? */

  struct dhcp_t *this = conn->parent;

  /* Tell application that we disconnected */
  if (this->cb_disconnect)
    this->cb_disconnect(conn);

  if (this->debug)
    printf("DHCP freeconn: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n", 
	   conn->hismac[0], conn->hismac[1], conn->hismac[2],
	   conn->hismac[3], conn->hismac[4], conn->hismac[5]);



  /* Application specific code */
  /* First remove from hash table */
  (void)dhcp_hashdel(this, conn);


  /* Remove from link of used */
  if ((conn->next) && (conn->prev)) {
    conn->next->prev = conn->prev;
    conn->prev->next = conn->next;
  }
  else if (conn->next) { /* && prev == 0 */
    conn->next->prev = NULL;
    this->firstusedconn = conn->next;
  }
  else if (conn->prev) { /* && next == 0 */
    conn->prev->next = NULL;
    this->lastusedconn = conn->prev;
  }
  else { /* if ((next == 0) && (prev == 0)) */
    this->firstusedconn = NULL;
    this->lastusedconn = NULL;
  }

  /* Initialise structures */
  memset(conn, 0, sizeof(*conn));

  /* Insert into link of free */
  if (this->firstfreeconn) {
    this->firstfreeconn->prev = conn;
  }
  else { /* First insert */
    this->lastfreeconn = conn;
  }

  conn->next = this->firstfreeconn;
  this->firstfreeconn = conn;

  if (paranoid) dhcp_validate(this);

  return 0;
}


/**
 * dhcp_checkconn()
 * Checks connections to see if the lease has expired
 **/
int dhcp_checkconn(struct dhcp_t *this)
{
  struct dhcp_conn_t *conn;
  struct timeval now;

  gettimeofday(&now, NULL);
  now.tv_sec -= this->lease;

  conn = this->firstusedconn;
  while (conn) {
    if (timercmp(&now, &conn->lasttime, >)) {
      if (this->debug) printf("DHCP timeout: Removing connection\n");
      dhcp_freeconn(conn);
      return 0; /* Returning after first deletion */
    }
    conn = conn->next;
  }
  return 0;
}


/* API Functions */

/**
 * dhcp_version()
 * Returns the current version of the program
 **/
const char* dhcp_version()
{
  return VERSION;
}


/**
 * dhcp_new()
 * Allocates a new instance of the library
 **/

int
dhcp_new(struct dhcp_t **dhcp, int numconn, char *interface,
	 int usemac, uint8_t *mac, int promisc, 
	 struct in_addr *listen, int lease, int allowdyn,
	 struct in_addr *uamlisten, uint16_t uamport, int useeapol) {
  
  int i;
  struct in_addr noaddr;
  unsigned int blen;
  
  if (!(*dhcp = calloc(sizeof(struct dhcp_t), 1))) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "calloc() failed");
    return -1;
  }

  (*dhcp)->numconn = numconn;

  if (!((*dhcp)->conn = calloc(sizeof(struct dhcp_conn_t), numconn))) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "calloc() failed");
    free(*dhcp);
    return -1;
  }

  dhcp_initconn(*dhcp);

  strncpy((*dhcp)->devname, interface, IFNAMSIZ);
  (*dhcp)->devname[IFNAMSIZ] = 0;

  /* Bring network interface UP and RUNNING if currently down */
  (void)dhcp_gifflags((*dhcp)->devname, &(*dhcp)->devflags);
  if (!((*dhcp)->devflags & IFF_UP) || !((*dhcp)->devflags & IFF_RUNNING)) {
    (void)dhcp_sifflags((*dhcp)->devname, (*dhcp)->devflags | IFF_NOARP);
    memset(&noaddr, 0, sizeof(noaddr));
    (void)dhcp_setaddr((*dhcp)->devname, &noaddr, NULL, NULL);
  }
  
  if (usemac) memcpy(((*dhcp)->hwaddr), mac, DHCP_ETH_ALEN);
  if (((*dhcp)->fd = 
       dhcp_open_eth(interface, DHCP_ETH_IP, promisc, usemac,
		     ((*dhcp)->hwaddr),
		     &((*dhcp)->ifindex))) < 0) 
    {
      free((*dhcp)->conn);
      free(*dhcp);
      return -1; /* Error reporting done in dhcp_open_eth */
    }

#if defined(__FreeBSD__) || defined (__OpenBSD__) || defined (__APPLE__)
  if (ioctl((*dhcp)->fd, BIOCGBLEN, &blen) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,"ioctl() failed!");
  }
  (*dhcp)->rbuf_max = blen;
  if (!((*dhcp)->rbuf = malloc((*dhcp)->rbuf_max))) {
    /* TODO: Free malloc */
    sys_err(LOG_ERR, __FILE__, __LINE__, errno, "malloc() failed");
  }
  (*dhcp)->rbuf_offset = 0;
  (*dhcp)->rbuf_len = 0;
#endif

  if (usemac) memcpy(((*dhcp)->arp_hwaddr), mac, DHCP_ETH_ALEN);
  if (((*dhcp)->arp_fd = 
       dhcp_open_eth(interface, DHCP_ETH_ARP, promisc, usemac,
		     ((*dhcp)->arp_hwaddr),
		     &((*dhcp)->arp_ifindex))) < 0) 
    {
      close((*dhcp)->fd);
      free((*dhcp)->conn);
      free(*dhcp);
      return -1; /* Error reporting done in dhcp_open_eth */
    }

  if (!useeapol) {
    (*dhcp)->eapol_fd = 0;
  }
  else {
    if (usemac) memcpy(((*dhcp)->eapol_hwaddr), mac, DHCP_ETH_ALEN);
    if (((*dhcp)->eapol_fd = 
	 dhcp_open_eth(interface, DHCP_ETH_EAPOL, promisc, usemac,
		       ((*dhcp)->eapol_hwaddr), &((*dhcp)->eapol_ifindex))) < 0) {
      close((*dhcp)->fd);
      close((*dhcp)->arp_fd);
      free((*dhcp)->conn);
      free(*dhcp);
      return -1; /* Error reporting done in eapol_open_eth */
    }
  }

  if (dhcp_hashinit(*dhcp, (*dhcp)->numconn))
    return -1; /* Failed to allocate hash tables */

  /* Initialise various variables */
  (*dhcp)->ourip.s_addr = listen->s_addr;
  (*dhcp)->lease = lease;
  (*dhcp)->promisc = promisc;
  (*dhcp)->usemac = usemac;
  (*dhcp)->allowdyn = allowdyn;
  (*dhcp)->uamlisten.s_addr = uamlisten->s_addr;
  (*dhcp)->uamport = uamport;

  /* Initialise call back functions */
  (*dhcp)->cb_data_ind = 0;
  (*dhcp)->cb_eap_ind = 0;
  (*dhcp)->cb_request = 0;
  (*dhcp)->cb_disconnect = 0;
  (*dhcp)->cb_connect = 0;
  
  return 0;
}

/**
 * dhcp_set()
 * Set dhcp parameters which can be altered at runtime.
 **/
int
dhcp_set(struct dhcp_t *dhcp, int debug,
	 struct in_addr *authip, int authiplen, int anydns,
	 struct in_addr *uamokip, int uamokiplen, 
	 struct in_addr *uamokaddr,
	 struct in_addr *uamokmask, int uamoknetlen) {

  
  int i;

  dhcp->debug = debug;
  dhcp->anydns = anydns;

  /* Copy list of uamserver IP addresses */
  if ((dhcp)->authip) free((dhcp)->authip);
  dhcp->authiplen = authiplen;
  if (!(dhcp->authip = calloc(sizeof(struct in_addr), authiplen))) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "calloc() failed");
    dhcp->authip = 0;
    return -1;
  }
  memcpy(dhcp->authip, authip, sizeof(struct in_addr) * authiplen);

  /* Make hash table for allowed domains */
  if (dhcp->iphash) iphash_free(dhcp->iphash);
  if ((!uamokip) || (uamokiplen==0)) {
    dhcp->iphashm = NULL;
    dhcp->iphash = NULL;
  }
  else {
    if (!(dhcp->iphashm = calloc(uamokiplen, sizeof(struct ippoolm_t)))) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "calloc() failed");
      return -1;
    }
    for (i=0; i< uamokiplen; i++) {
      dhcp->iphashm[i].addr = uamokip[i];
    }
    (void)iphash_new(&dhcp->iphash, dhcp->iphashm, uamokiplen);
  }

  /* Copy allowed networks */
  if (dhcp->uamokaddr) free(dhcp->uamokaddr);
  if (dhcp->uamokmask) free(dhcp->uamokmask);
  if ((!uamokaddr) || (!uamokmask) || (uamoknetlen==0)) {
    dhcp->uamokaddr = NULL;
    dhcp->uamokmask = NULL;
    dhcp->uamoknetlen = 0;
  }
  else {
    dhcp->uamoknetlen = uamoknetlen;
    if (!(dhcp->uamokaddr = calloc(uamoknetlen, sizeof(struct in_addr)))) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "calloc() failed");
      return -1;
    }
    if (!(dhcp->uamokmask = calloc(uamoknetlen, sizeof(struct in_addr)))) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "calloc() failed");
      return -1;
    }
    memcpy(dhcp->uamokaddr, uamokaddr, uamoknetlen * sizeof(struct in_addr));
    memcpy(dhcp->uamokmask, uamokmask, uamoknetlen * sizeof(struct in_addr));
  }
  return 0;
}

/**
 * dhcp_free()
 * Releases ressources allocated to the instance of the library
 **/
int dhcp_free(struct dhcp_t *dhcp) {

  if (dhcp->hash) free(dhcp->hash);
  if (dhcp->iphash) iphash_free(dhcp->iphash);
  if (dhcp->iphashm) free(dhcp->iphashm);
  if (dhcp->authip) free(dhcp->authip);
  if (dhcp->uamokaddr) free(dhcp->uamokaddr);
  if (dhcp->uamokmask) free(dhcp->uamokmask);
  (void)dhcp_sifflags(dhcp->devname, dhcp->devflags);
  close(dhcp->fd);
  close(dhcp->arp_fd);
  if (dhcp->eapol_fd) close(dhcp->eapol_fd);
  free(dhcp->conn);
  free(dhcp);
  return 0;
}

/**
 * dhcp_timeout()
 * Need to call this function at regular intervals to clean up old connections.
 **/
int
dhcp_timeout(struct dhcp_t *this)
{
  if (paranoid) 
    dhcp_validate(this);

  dhcp_checkconn(this);
  
  return 0;
}

/**
 * dhcp_timeleft()
 * Use this function to find out when to call dhcp_timeout()
 * If service is needed after the value given by tvp then tvp
 * is left unchanged.
 **/
struct timeval*

dhcp_timeleft(struct dhcp_t *this, struct timeval *tvp)
{
  return tvp;
}


/**
 * dhcp_doDNAT()
 * Change destination address to authentication server.
 **/
int dhcp_doDNAT(struct dhcp_conn_t *conn, 
		struct dhcp_ippacket_t *pack, int len) {

  struct dhcp_t *this = conn->parent;
  struct dhcp_tcphdr_t *tcph = (struct dhcp_tcphdr_t*) pack->payload;
  struct dhcp_udphdr_t *udph = (struct dhcp_udphdr_t*) pack->payload;
  int i;

  /* Was it a DNS request? */
  if (((this->anydns) ||
       (pack->iph.daddr == conn->dns1.s_addr) ||
       (pack->iph.daddr == conn->dns2.s_addr)) &&
      (pack->iph.protocol == DHCP_IP_UDP) &&
      (udph->dst == htons(DHCP_DNS)))
    return 0; 
  
  /* Was it an ICMP request for us? */
  if ((pack->iph.daddr == conn->ourip.s_addr) &&
      (pack->iph.protocol == DHCP_IP_ICMP))
    return 0;

  /* Was it a http or https request for authentication server? */
  /* Was it a request for authentication server? */
  for (i = 0; i<this->authiplen; i++) {
    if ((pack->iph.daddr == this->authip[i].s_addr) /* &&
	(pack->iph.protocol == DHCP_IP_TCP) &&
	((tcph->dst == htons(DHCP_HTTP)) ||
	(tcph->dst == htons(DHCP_HTTPS)))*/)
      return 0; /* Destination was authentication server */
  }

  /* Was it a request for local redirection server? */
  if ((pack->iph.daddr == this->uamlisten.s_addr) &&
      (pack->iph.protocol == DHCP_IP_TCP) &&
      (tcph->dst == htons(this->uamport)))
    return 0; /* Destination was local redir server */

  /* Was it a request for an allowed domain? */
  if (this->iphash && 
      (!ippool_getip(this->iphash, NULL, (struct in_addr*) &pack->iph.daddr)))
    return 0;
  
  /* Was it a request for an allowed network? */
  for (i=0; i<this->uamoknetlen; i++) {
    if (this->uamokaddr[i].s_addr == 
	(pack->iph.daddr & this->uamokmask[i].s_addr))
      return 0;
  }

  /* Was it a http request for another server? */
  /* We are changing dest IP and dest port to local UAM server */
  if ((pack->iph.protocol == DHCP_IP_TCP) &&
      (tcph->dst == htons(DHCP_HTTP))) {
    int n;
    int pos=-1;
    for (n=0; n<DHCP_DNAT_MAX; n++) {
      if ((conn->dnatip[n] == pack->iph.daddr) && 
	  (conn->dnatport[n] == tcph->src)) {
	pos = n;
	break;
      }
    }
    if (pos==-1) { /* Save for undoing */
      conn->dnatip[conn->nextdnat] = pack->iph.daddr; 
      conn->dnatport[conn->nextdnat] = tcph->src;
      conn->nextdnat = (conn->nextdnat + 1) % DHCP_DNAT_MAX;
    }
    pack->iph.daddr = this->uamlisten.s_addr;
    tcph->dst  = htons(this->uamport);
    (void)dhcp_tcp_check(pack, len);
    (void)dhcp_ip_check((struct dhcp_ippacket_t*) pack);
    return 0;
  }

  return -1; /* Something else */

}

/**
 * dhcp_undoDNAT()
 * Change source address back to original server
 **/
int dhcp_undoDNAT(struct dhcp_conn_t *conn, 
		  struct dhcp_ippacket_t *pack, int len) {

  struct dhcp_t *this = conn->parent;
  struct dhcp_tcphdr_t *tcph = (struct dhcp_tcphdr_t*) pack->payload;
  struct dhcp_udphdr_t *udph = (struct dhcp_udphdr_t*) pack->payload;
  int i;

  
  /* Was it a DNS reply? */
  if (((this->anydns) ||
       (pack->iph.saddr == conn->dns1.s_addr) ||
       (pack->iph.saddr == conn->dns2.s_addr)) &&
      (pack->iph.protocol == DHCP_IP_UDP) &&
      (udph->src == htons(DHCP_DNS)))
    return 0; 
  
  /* Was it an ICMP reply from us? */ /* Added by Freddy */
  if ((pack->iph.saddr == conn->ourip.s_addr) &&
      (pack->iph.protocol == DHCP_IP_ICMP))
    return 0;

  /* Was it an ICMP reply from us? */
  if ((pack->iph.saddr == conn->ourip.s_addr) &&
      (pack->iph.protocol == DHCP_IP_ICMP))
    return 0;

  /* Was it a reply from redir server? */
  if ((pack->iph.saddr == this->uamlisten.s_addr) &&
      (pack->iph.protocol == DHCP_IP_TCP) &&
      (tcph->src == htons(this->uamport))) {
    int n;
    for (n=0; n<DHCP_DNAT_MAX; n++) {
      if (tcph->dst == conn->dnatport[n]) {
	pack->iph.saddr = conn->dnatip[n];
	tcph->src = htons(DHCP_HTTP);
	(void)dhcp_tcp_check(pack, len);
	(void)dhcp_ip_check((struct dhcp_ippacket_t*) pack);
	return 0; /* It was a DNAT reply */
      }
    }
    return 0; /* It was a normal reply from redir server */
  }
  
  /* Was it a normal http or https reply from authentication server? */
  /* Was it a normal reply from authentication server? */
  for (i = 0; i<this->authiplen; i++) {
    if ((pack->iph.saddr == this->authip[i].s_addr) /* &&
	(pack->iph.protocol == DHCP_IP_TCP) &&
	((tcph->src == htons(DHCP_HTTP)) ||
	(tcph->src == htons(DHCP_HTTPS)))*/)
      return 0; /* Destination was authentication server */
  }
  
  /* Was it a reply from an allowed domain? */
  if (this->iphash && 
      (!ippool_getip(this->iphash, NULL, (struct in_addr*) &pack->iph.saddr)))
    return 0;

  /* Was it a reply from for an allowed network? */
  for (i=0; i<this->uamoknetlen; i++) {
    if (this->uamokaddr[i].s_addr == 
	(pack->iph.saddr & this->uamokmask[i].s_addr))
      return 0;
  }


  return -1; /* Something else */

}

#ifdef DHCP_CHECKDNS
/**
 * dhcp_checkDNS()
 * Check if it was request for known domain name.
 * In case it was a request for a known keyword then
 * redirect to the login/logout page
 * 2005-09-19: This stuff is highly experimental.
 **/
int dhcp_checkDNS(struct dhcp_conn_t *conn, 
		  struct dhcp_ippacket_t *pack, int len) {

  struct dhcp_t *this = conn->parent;
  struct dhcp_udphdr_t *udph = (struct dhcp_udphdr_t*) pack->payload;
  struct dhcp_dns_packet_t *dnsp = (struct dhcp_dns_packet_t*) 
    (pack->payload + sizeof(struct dhcp_udphdr_t));
  int i;
  uint8_t *p1 = NULL;
  uint8_t *p2 = NULL;
  struct dhcp_dns_fullpacket_t answer;
  int length;
  int udp_len;
  uint8_t query[256];
  int query_len = 0;
  int n;

  printf("DNS ID: \n");

  printf("DNS ID:    %d\n", ntohs(dnsp->id));
  printf("DNS flags: %d\n", ntohs(dnsp->flags));

  if ((ntohs(dnsp->flags)   == 0x0100) &&
      (ntohs(dnsp->qdcount) == 0x0001) &&
      (ntohs(dnsp->ancount) == 0x0000) &&
      (ntohs(dnsp->nscount) == 0x0000) &&
      (ntohs(dnsp->arcount) == 0x0000)) {
    printf("It was a query %s: \n", dnsp->records);
    p1 = dnsp->records + 1 + dnsp->records[0];
    p2 = dnsp->records;
    do {
      if (query_len < 256)
	query[query_len++] = *p2;
    } while (*p2++!=0); /* TODO */
    for (n=0; n<4; n++) {
      if (query_len < 256)
	query[query_len++] = *p2++;
    }

    query[query_len++] = 0xc0;
    query[query_len++] = 0x0c;
    query[query_len++] = 0x00;
    query[query_len++] = 0x01;
    query[query_len++] = 0x00;
    query[query_len++] = 0x01;
    query[query_len++] = 0x00;
    query[query_len++] = 0x00;
    query[query_len++] = 0x01;
    query[query_len++] = 0x2c;
    query[query_len++] = 0x00;
    query[query_len++] = 0x04;
    memcpy(&query[query_len], &conn->ourip.s_addr, 4);
    query_len += 4;

    if (!memcmp(p1, 
		"\3key\12chillispot\3org", 
		sizeof("\3key\12chillispot\3org"))) {
      printf("It was a matching query %s: \n", dnsp->records);
      memcpy(&answer, pack, len); /* TODO */
      
      /* DNS Header */
      answer.dns.id      = dnsp->id;
      answer.dns.flags   = htons(0x8000);
      answer.dns.qdcount = htons(0x0001);
      answer.dns.ancount = htons(0x0001);
      answer.dns.nscount = htons(0x0000);
      answer.dns.arcount = htons(0x0000);
      memcpy(answer.dns.records, query, query_len);
      
      /* UDP header */
      udp_len = query_len + DHCP_DNS_HLEN + DHCP_UDP_HLEN;
      answer.udph.len = htons(udp_len);
      answer.udph.src = udph->dst;
      answer.udph.dst = udph->src;
      
      /* IP header */
      answer.iph.ihl = 5;
      answer.iph.version = 4;
      answer.iph.tos = 0;
      answer.iph.tot_len = htons(udp_len + DHCP_IP_HLEN);
      answer.iph.id = 0;
      answer.iph.frag_off = 0;
      answer.iph.ttl = 0x10;
      answer.iph.protocol = 0x11;
      answer.iph.check = 0; /* Calculate at end of packet */      
      memcpy(&answer.iph.daddr, &pack->iph.saddr, DHCP_IP_ALEN);
      memcpy(&answer.iph.saddr, &pack->iph.saddr, DHCP_IP_ALEN);

      /* Ethernet header */
      memcpy(&answer.ethh.dst, &pack->ethh.src, DHCP_ETH_ALEN);
      memcpy(&answer.ethh.src, &pack->ethh.dst, DHCP_ETH_ALEN);
      answer.ethh.prot = htons(DHCP_ETH_IP);

      /* Work out checksums */
      (void)dhcp_udp_check((struct dhcp_fullpacket_t*) &answer);
      (void)dhcp_ip_check((struct dhcp_ippacket_t*) &answer);

      /* Calculate total length */
      length = udp_len + DHCP_IP_HLEN + DHCP_ETH_HLEN;
      
      return dhcp_send(this, this->fd, DHCP_ETH_IP, conn->hismac, this->ifindex,
		       &answer, length);


    }
  }
  return -0; /* Something else */
}
#endif

/**
 * dhcp_getdefault()
 * Fill in a DHCP packet with most essential values
 **/
int
dhcp_getdefault(struct dhcp_fullpacket_t *pack)
{

  /* Initialise reply packet with request */
  memset(pack, 0, sizeof(struct dhcp_fullpacket_t));

  /* DHCP Payload */
  pack->dhcp.op     = DHCP_BOOTREPLY;
  pack->dhcp.htype  = DHCP_HTYPE_ETH;
  pack->dhcp.hlen   = DHCP_ETH_ALEN;

  /* UDP header */
  pack->udph.src = htons(DHCP_BOOTPS);
  pack->udph.dst = htons(DHCP_BOOTPC);

  /* IP header */
  pack->iph.ihl = 5;
  pack->iph.version = 4;
  pack->iph.tos = 0;
  pack->iph.tot_len = 0; /* Calculate at end of packet */
  pack->iph.id = 0;
  pack->iph.frag_off = 0;
  pack->iph.ttl = 0x10;
  pack->iph.protocol = 0x11;
  pack->iph.check = 0; /* Calculate at end of packet */

  /* Ethernet header */
  pack->ethh.prot = htons(DHCP_ETH_IP);

  return 0;
}


/**
 * dhcp_gettag()
 * Search a DHCP packet for a particular tag.
 * Returns -1 if not found.
 **/
int dhcp_gettag(struct dhcp_packet_t *pack, int length,
		struct dhcp_tag_t **tag, uint8_t tagtype) {
  struct dhcp_tag_t *t;
  int offset = DHCP_MIN_LEN + DHCP_OPTION_MAGIC_LEN;

  
  /* if (length > DHCP_LEN) {
    sys_err(LOG_WARNING, __FILE__, __LINE__, 0,
	    "Length of dhcp packet larger then %d: %d", DHCP_LEN, length);
    length = DHCP_LEN;
  } */
  
  while ((offset + 2) < length) {
    t = (struct dhcp_tag_t*) (((void*) pack) + offset);
    if (t->t == tagtype) {
      if ((offset +  2 + t->l) > length)
	return -1; /* Tag length too long */
      *tag = t;
      return 0;
    }
    offset +=  2 + t->l;
  }
  
  return -1; /* Not found  */
  
}


/**
 * dhcp_sendOFFER()
 * Send of a DHCP offer message to a peer.
 **/
int dhcp_sendOFFER(struct dhcp_conn_t *conn, 
		   struct dhcp_fullpacket_t *pack, int len) {

  struct dhcp_t *this = conn->parent;
  struct dhcp_fullpacket_t packet;
  uint16_t length = 576 + 4; /* Maximum length */
  uint16_t udp_len = 576 - 20; /* Maximum length */
  int pos = 0;

  /* Get packet default values */
  dhcp_getdefault(&packet);
  
  /* DHCP Payload */
  packet.dhcp.xid    = pack->dhcp.xid;
  packet.dhcp.yiaddr = conn->hisip.s_addr;
  packet.dhcp.flags  = pack->dhcp.flags;
  packet.dhcp.giaddr = pack->dhcp.giaddr;
  memcpy(&packet.dhcp.chaddr, &pack->dhcp.chaddr, DHCP_CHADDR_LEN);

  /* Magic cookie */
  packet.dhcp.options[pos++] = 0x63;
  packet.dhcp.options[pos++] = 0x82;
  packet.dhcp.options[pos++] = 0x53;
  packet.dhcp.options[pos++] = 0x63;

  packet.dhcp.options[pos++] = DHCP_OPTION_MESSAGE_TYPE;
  packet.dhcp.options[pos++] = 1;
  packet.dhcp.options[pos++] = DHCPOFFER;

  packet.dhcp.options[pos++] = DHCP_OPTION_SUBNET_MASK;
  packet.dhcp.options[pos++] = 4;
  memcpy(&packet.dhcp.options[pos], &conn->hismask.s_addr, 4);
  pos += 4;

  packet.dhcp.options[pos++] = DHCP_OPTION_ROUTER_OPTION;
  packet.dhcp.options[pos++] = 4;
  memcpy(&packet.dhcp.options[pos], &conn->ourip.s_addr, 4);
  pos += 4;

  /* Insert DNS Servers if given */
  if (conn->dns1.s_addr && conn->dns2.s_addr) {
    packet.dhcp.options[pos++] = DHCP_OPTION_DNS;
    packet.dhcp.options[pos++] = 8;
    memcpy(&packet.dhcp.options[pos], &conn->dns1.s_addr, 4);
    pos += 4;
    memcpy(&packet.dhcp.options[pos], &conn->dns2.s_addr, 4);
    pos += 4;
  }
  else if (conn->dns1.s_addr) {
    packet.dhcp.options[pos++] = DHCP_OPTION_DNS;
    packet.dhcp.options[pos++] = 4;
    memcpy(&packet.dhcp.options[pos], &conn->dns1.s_addr, 4);
    pos += 4;
  }
  else if (conn->dns2.s_addr) {
    packet.dhcp.options[pos++] = DHCP_OPTION_DNS;
    packet.dhcp.options[pos++] = 4;
    memcpy(&packet.dhcp.options[pos], &conn->dns2.s_addr, 4);
    pos += 4;
  }

  /* Insert Domain Name if present */
  if (strlen(conn->domain)) {
    packet.dhcp.options[pos++] = DHCP_OPTION_DOMAIN_NAME;
    packet.dhcp.options[pos++] = strlen(conn->domain);
    memcpy(&packet.dhcp.options[pos], &conn->domain, strlen(conn->domain));
    pos += strlen(conn->domain);
  }

  packet.dhcp.options[pos++] = DHCP_OPTION_LEASE_TIME;
  packet.dhcp.options[pos++] = 4;
  packet.dhcp.options[pos++] = (this->lease >> 24) & 0xFF;
  packet.dhcp.options[pos++] = (this->lease >> 16) & 0xFF;
  packet.dhcp.options[pos++] = (this->lease >>  8) & 0xFF;
  packet.dhcp.options[pos++] = (this->lease >>  0) & 0xFF;

  /* Must be listening address */
  packet.dhcp.options[pos++] = DHCP_OPTION_SERVER_ID;
  packet.dhcp.options[pos++] = 4;
  memcpy(&packet.dhcp.options[pos], &conn->ourip.s_addr, 4);
  pos += 4;

  packet.dhcp.options[pos++] = DHCP_OPTION_END;

  /* UDP header */
  udp_len = pos + DHCP_MIN_LEN + DHCP_UDP_HLEN;
  packet.udph.len = htons(udp_len);

  /* IP header */
  packet.iph.tot_len = htons(udp_len + DHCP_IP_HLEN);
  packet.iph.daddr = ~0; /* TODO: Always sending to broadcast address */
  packet.iph.saddr = conn->ourip.s_addr;

  /* Work out checksums */
  (void)dhcp_udp_check(&packet);
  (void)dhcp_ip_check((struct dhcp_ippacket_t*) &packet); 

  /* Ethernet header */
  memcpy(packet.ethh.dst, conn->hismac, DHCP_ETH_ALEN);
  memcpy(packet.ethh.src, this->hwaddr, DHCP_ETH_ALEN);

  /* Calculate total length */
  length = udp_len + DHCP_IP_HLEN + DHCP_ETH_HLEN;

  return dhcp_send(this, this->fd, DHCP_ETH_IP, conn->hismac, this->ifindex,
		   &packet, length);
}

/**
 * dhcp_sendACK()
 * Send of a DHCP acknowledge message to a peer.
 **/
int dhcp_sendACK(struct dhcp_conn_t *conn, 
		 struct dhcp_fullpacket_t *pack, int len) {

  struct dhcp_t *this = conn->parent;
  struct dhcp_fullpacket_t packet;
  uint16_t length = 576 + 4; /* Maximum length */
  uint16_t udp_len = 576 - 20; /* Maximum length */
  int pos = 0;

  /* Get packet default values */
  dhcp_getdefault(&packet);
  
  /* DHCP Payload */
  packet.dhcp.xid    = pack->dhcp.xid;
  packet.dhcp.ciaddr = pack->dhcp.ciaddr;
  packet.dhcp.yiaddr = conn->hisip.s_addr;
  packet.dhcp.flags  = pack->dhcp.flags;
  packet.dhcp.giaddr = pack->dhcp.giaddr;
  memcpy(&packet.dhcp.chaddr, &pack->dhcp.chaddr, DHCP_CHADDR_LEN);

  /* Magic cookie */
  packet.dhcp.options[pos++] = 0x63;
  packet.dhcp.options[pos++] = 0x82;
  packet.dhcp.options[pos++] = 0x53;
  packet.dhcp.options[pos++] = 0x63;

  packet.dhcp.options[pos++] = DHCP_OPTION_MESSAGE_TYPE;
  packet.dhcp.options[pos++] = 1;
  packet.dhcp.options[pos++] = DHCPACK;

  packet.dhcp.options[pos++] = DHCP_OPTION_SUBNET_MASK;
  packet.dhcp.options[pos++] = 4;
  memcpy(&packet.dhcp.options[pos], &conn->hismask.s_addr, 4);
  pos += 4;

  packet.dhcp.options[pos++] = DHCP_OPTION_ROUTER_OPTION;
  packet.dhcp.options[pos++] = 4;
  memcpy(&packet.dhcp.options[pos], &conn->ourip.s_addr, 4);
  pos += 4;

  /* Insert DNS Servers if given */
  if (conn->dns1.s_addr && conn->dns2.s_addr) {
    packet.dhcp.options[pos++] = DHCP_OPTION_DNS;
    packet.dhcp.options[pos++] = 8;
    memcpy(&packet.dhcp.options[pos], &conn->dns1.s_addr, 4);
    pos += 4;
    memcpy(&packet.dhcp.options[pos], &conn->dns2.s_addr, 4);
    pos += 4;
  }
  else if (conn->dns1.s_addr) {
    packet.dhcp.options[pos++] = DHCP_OPTION_DNS;
    packet.dhcp.options[pos++] = 4;
    memcpy(&packet.dhcp.options[pos], &conn->dns1.s_addr, 4);
    pos += 4;
  }
  else if (conn->dns2.s_addr) {
    packet.dhcp.options[pos++] = DHCP_OPTION_DNS;
    packet.dhcp.options[pos++] = 4;
    memcpy(&packet.dhcp.options[pos], &conn->dns2.s_addr, 4);
    pos += 4;
  }

  /* Insert Domain Name if present */
  if (strlen(conn->domain)) {
    packet.dhcp.options[pos++] = DHCP_OPTION_DOMAIN_NAME;
    packet.dhcp.options[pos++] = strlen(conn->domain);
    memcpy(&packet.dhcp.options[pos], &conn->domain, strlen(conn->domain));
    pos += strlen(conn->domain);
  }

  packet.dhcp.options[pos++] = DHCP_OPTION_LEASE_TIME;
  packet.dhcp.options[pos++] = 4;
  packet.dhcp.options[pos++] = (this->lease >> 24) & 0xFF;
  packet.dhcp.options[pos++] = (this->lease >> 16) & 0xFF;
  packet.dhcp.options[pos++] = (this->lease >>  8) & 0xFF;
  packet.dhcp.options[pos++] = (this->lease >>  0) & 0xFF;

  /*
  packet.dhcp.options[pos++] = DHCP_OPTION_INTERFACE_MTU;
  packet.dhcp.options[pos++] = 2;
  packet.dhcp.options[pos++] = (conn->mtu >> 8) & 0xFF;
  packet.dhcp.options[pos++] = (conn->mtu >> 0) & 0xFF;
  */

  /* Must be listening address */
  packet.dhcp.options[pos++] = DHCP_OPTION_SERVER_ID;
  packet.dhcp.options[pos++] = 4;
  memcpy(&packet.dhcp.options[pos], &conn->ourip.s_addr, 4);
  pos += 4;

  packet.dhcp.options[pos++] = DHCP_OPTION_END;

  /* UDP header */
  udp_len = pos + DHCP_MIN_LEN + DHCP_UDP_HLEN;
  packet.udph.len = htons(udp_len);

  /* IP header */
  packet.iph.tot_len = htons(udp_len + DHCP_IP_HLEN);
  packet.iph.daddr = ~0; /* TODO: Always sending to broadcast address */
  packet.iph.saddr = conn->ourip.s_addr;

  /* Work out checksums */
  (void)dhcp_udp_check(&packet);
  (void)dhcp_ip_check((struct dhcp_ippacket_t*) &packet); 

  /* Ethernet header */
  memcpy(packet.ethh.dst, conn->hismac, DHCP_ETH_ALEN);
  memcpy(packet.ethh.src, this->hwaddr, DHCP_ETH_ALEN);

  /* Calculate total length */
  length = udp_len + DHCP_IP_HLEN + DHCP_ETH_HLEN;

  return dhcp_send(this, this->fd, DHCP_ETH_IP, conn->hismac, this->ifindex,
		   &packet, length);
}

/**
 * dhcp_sendNAK()
 * Send of a DHCP negative acknowledge message to a peer.
 * NAK messages are always sent to broadcast IP address (
 * except when using a DHCP relay server)
 **/
int dhcp_sendNAK(struct dhcp_conn_t *conn, 
		 struct dhcp_fullpacket_t *pack, int len) {

  struct dhcp_t *this = conn->parent;
  struct dhcp_fullpacket_t packet;
  uint16_t length = 576 + 4; /* Maximum length */
  uint16_t udp_len = 576 - 20; /* Maximum length */
  int pos = 0;

  /* Get packet default values */
  dhcp_getdefault(&packet);

  
  /* DHCP Payload */
  packet.dhcp.xid    = pack->dhcp.xid;
  packet.dhcp.flags  = pack->dhcp.flags;
  packet.dhcp.giaddr = pack->dhcp.giaddr;
  memcpy(&packet.dhcp.chaddr, &pack->dhcp.chaddr, DHCP_CHADDR_LEN);

  /* Magic cookie */
  packet.dhcp.options[pos++] = 0x63;
  packet.dhcp.options[pos++] = 0x82;
  packet.dhcp.options[pos++] = 0x53;
  packet.dhcp.options[pos++] = 0x63;

  packet.dhcp.options[pos++] = DHCP_OPTION_MESSAGE_TYPE;
  packet.dhcp.options[pos++] = 1;
  packet.dhcp.options[pos++] = DHCPNAK;

  /* Must be listening address */
  packet.dhcp.options[pos++] = DHCP_OPTION_SERVER_ID;
  packet.dhcp.options[pos++] = 4;
  memcpy(&packet.dhcp.options[pos], &conn->ourip.s_addr, 4);
  pos += 4;

  packet.dhcp.options[pos++] = DHCP_OPTION_END;

  /* UDP header */
  udp_len = pos + DHCP_MIN_LEN + DHCP_UDP_HLEN;
  packet.udph.len = htons(udp_len);

  /* IP header */
  packet.iph.tot_len = htons(udp_len + DHCP_IP_HLEN);
  packet.iph.daddr = ~0; /* TODO: Always sending to broadcast address */
  packet.iph.saddr = conn->ourip.s_addr;

  /* Work out checksums */
  (void)dhcp_udp_check(&packet);
  (void)dhcp_ip_check((struct dhcp_ippacket_t*) &packet); 

  /* Ethernet header */
  memcpy(packet.ethh.dst, conn->hismac, DHCP_ETH_ALEN);
  memcpy(packet.ethh.src, this->hwaddr, DHCP_ETH_ALEN);

  /* Calculate total length */
  length = udp_len + DHCP_IP_HLEN + DHCP_ETH_HLEN;

  return dhcp_send(this, this->fd, DHCP_ETH_IP, conn->hismac, this->ifindex,
		   &packet, length);

}


/**
 * dhcp_getreq()
 * Process a received DHCP request MESSAGE.
 **/
int dhcp_getreq(struct dhcp_t *this,
		struct dhcp_fullpacket_t *pack, int len) {
  struct dhcp_conn_t *conn;

  struct dhcp_tag_t *message_type = 0;
  struct dhcp_tag_t *requested_ip = 0;
  struct in_addr addr;

  if (pack->udph.dst != htons(DHCP_BOOTPS)) 
    return 0; /* Not a DHCP packet */

  if (dhcp_gettag(&pack->dhcp, ntohs(pack->udph.len)-DHCP_UDP_HLEN, 
		  &message_type, DHCP_OPTION_MESSAGE_TYPE)) {
    return -1;
  }

  if (message_type->l != 1)
    return -1; /* Wrong length of message type */

  if ((message_type->v[0] != DHCPDISCOVER) && 
      (message_type->v[0] != DHCPREQUEST) &&
      (message_type->v[0] != DHCPRELEASE)) {
    return 0; /* Unsupported message type */
  }
  
  /* Release message */
  /* If connection exists: Release it. No Reply to client is sent */
  if (message_type->v[0] == DHCPRELEASE) {
    if (!dhcp_hashget(this, &conn, pack->ethh.src)) {
      dhcp_freeconn(conn);
    }
    return 0;
  }

  /* Check to see if we know MAC address. If not allocate new conn */
  if (dhcp_hashget(this, &conn, pack->ethh.src)) {
    
    /* Do we allow dynamic allocation of IP addresses? */
    if (!this->allowdyn) /* TODO: Should be deleted! */
      return 0; 

    /* Allocate new connection */
    if (dhcp_newconn(this, &conn, pack->ethh.src)) /* TODO: Delete! */
      return 0; /* Out of connections */
  }

  /* Request an IP address */
  if (conn->authstate == DHCP_AUTH_NONE) {
    addr.s_addr = pack->dhcp.ciaddr;
    if (this ->cb_request)
      if (this->cb_request(conn, &addr)) {
	return 0; /* Ignore request if IP address was not allocated */
      }
  }
  
  gettimeofday(&conn->lasttime, NULL);

  /* Discover message */
  /* If an IP address was assigned offer it to the client */
  /* Otherwise ignore the request */
  if (message_type->v[0] == DHCPDISCOVER) {
    if (conn->hisip.s_addr) (void)dhcp_sendOFFER(conn, pack, len);
    return 0;
  }
  
  /* Request message */
  if (message_type->v[0] == DHCPREQUEST) {
    
    if (!conn->hisip.s_addr) {
      if (this->debug) printf("hisip not set\n");
      return dhcp_sendNAK(conn, pack, len);
    }

    if (!memcmp(&conn->hisip.s_addr, &pack->dhcp.ciaddr, 4)) {
      if (this->debug) printf("hisip match ciaddr\n");
      return dhcp_sendACK(conn, pack, len);
    }

    if (!dhcp_gettag(&pack->dhcp, ntohs(pack->udph.len)-DHCP_UDP_HLEN, 
		     &requested_ip, DHCP_OPTION_REQUESTED_IP)) {
      if (!memcmp(&conn->hisip.s_addr, requested_ip->v, 4))
	return dhcp_sendACK(conn, pack, len);
    }

    if (this->debug) printf("Sending NAK to client\n");
    return dhcp_sendNAK(conn, pack, len);
  }
  
  /* Unsupported DHCP message: Ignore */
  return 0;
}


/**
 * dhcp_set_addrs()
 * Set various IP addresses of a connection.
 **/
int dhcp_set_addrs(struct dhcp_conn_t *conn,
		   struct in_addr *hisip,
		   struct in_addr *hismask,
		   struct in_addr *ourip,
		   struct in_addr *dns1,
		   struct in_addr *dns2,
		   char *domain) {

  conn->hisip.s_addr = hisip->s_addr;
  conn->hismask.s_addr = hismask->s_addr;
  conn->ourip.s_addr = ourip->s_addr;
  conn->dns1.s_addr = dns1->s_addr;
  conn->dns2.s_addr = dns2->s_addr;

  if (domain) {
    strncpy(conn->domain, domain, DHCP_DOMAIN_LEN);
    conn->domain[DHCP_DOMAIN_LEN-1] = 0;
  }
  else {
    conn->domain[0] = 0;
  }
  
  return 0;
}


int dhcp_receive_ip(struct dhcp_t *this, struct dhcp_ippacket_t *pack,
		    int len)
{
  struct dhcp_conn_t *conn;
  struct in_addr ourip;
  unsigned char const bmac[DHCP_ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  struct in_addr addr;
  struct dhcp_tcphdr_t *tcph = (struct dhcp_tcphdr_t*) pack->payload;
  struct dhcp_udphdr_t *udph = (struct dhcp_udphdr_t*) pack->payload;

  if (this->debug) printf("DHCP packet received\n");
  
  /* Check that MAC address is our MAC or Broadcast */
  if ((memcmp(pack->ethh.dst, this->hwaddr, DHCP_ETH_ALEN)) && (memcmp(pack->ethh.dst, bmac, DHCP_ETH_ALEN)))
    return 0;

  /* Check to see if we know MAC address. */
  if (!dhcp_hashget(this, &conn, pack->ethh.src)) {
    if (this->debug) printf("Address found\n");
    ourip.s_addr = conn->ourip.s_addr;
  }
  else {
    if (this->debug) printf("Address not found\n");
    ourip.s_addr = this->ourip.s_addr;
    
    /* Do we allow dynamic allocation of IP addresses? */
    if (!this->allowdyn)
      return 0; 
    
    /* Allocate new connection */
    if (dhcp_newconn(this, &conn, pack->ethh.src))
      return 0; /* Out of connections */
  }

  /* Request an IP address */
  if ((conn->authstate == DHCP_AUTH_NONE) && 
      (pack->iph.daddr != 0) && (pack->iph.daddr != 0xffffffff)) {
    addr.s_addr = pack->iph.saddr;
    if (this ->cb_request)
      if (this->cb_request(conn, &addr)) {
	return 0; /* Ignore request if IP address was not allocated */
      }
  }


  /* Check to see if it is a packet for us */
  /* TODO: Handle IP packets with options. Currently these are just ignored */
  if (((pack->iph.daddr == 0) || 
       (pack->iph.daddr == 0xffffffff) ||
       (pack->iph.daddr == ourip.s_addr)) &&
      ((pack->iph.ihl == 5) && (pack->iph.protocol == DHCP_IP_UDP) &&
       (((struct dhcp_fullpacket_t*)pack)->udph.dst == htons(DHCP_BOOTPS)))) {
    (void)dhcp_getreq(this, (struct dhcp_fullpacket_t*) pack, len);
  }

  /* Return if we do not know peer */
  if (!conn) 
    return 0;

  gettimeofday(&conn->lasttime, NULL);

  /* Was it a DNS request? 
  if (((pack->iph.daddr == conn->dns1.s_addr) ||
       (pack->iph.daddr == conn->dns2.s_addr)) &&
      (pack->iph.protocol == DHCP_IP_UDP) &&
      (udph->dst == htons(DHCP_DNS))) {
    if (dhcp_checkDNS(conn, pack, len)) return 0;
  } */

  switch (conn->authstate) {
  case DHCP_AUTH_PASS:
    /* Pass packets unmodified */
    break; 
  case DHCP_AUTH_UNAUTH_TOS:
    /* Set TOS to specified value (unauthenticated) */
    pack->iph.tos = conn->unauth_cp;
    (void)dhcp_ip_check(pack);
    break;
  case DHCP_AUTH_AUTH_TOS:
    /* Set TOS to specified value (authenticated) */
    pack->iph.tos = conn->auth_cp;
    (void)dhcp_ip_check(pack);
    break;
  case DHCP_AUTH_DNAT:
    /* Destination NAT if request to unknown web server */
    if (dhcp_doDNAT(conn, pack, len))
      return 0; /* Drop is not http or dns */
    break;
  case DHCP_AUTH_DROP: 
  default:
    return 0;
  }

  if ((conn->hisip.s_addr) && (this ->cb_data_ind)) {
    this ->cb_data_ind(conn, &pack->iph, len-DHCP_ETH_HLEN);
  }
  
  return 0;
}


/**
 * dhcp_decaps()
 * Call this function when a new IP packet has arrived. This function
 * should be part of a select() loop in the application.
 **/
int dhcp_decaps(struct dhcp_t *this)  /* DHCP Indication */
{
  struct dhcp_ippacket_t packet;
  int length;
  
  struct dhcp_conn_t *conn;
  struct in_addr ourip;
  unsigned char const bmac[DHCP_ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

  if (this->debug) printf("DHCP packet received\n");

  if ((length = recv(this->fd, &packet, sizeof(packet), 0)) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "recv(fd=%d, len=%d) failed",
	    this->fd, sizeof(packet));
    return -1;
  }

  return dhcp_receive_ip(this, &packet, length);
}

/**
 * dhcp_data_req()
 * Call this function to send an IP packet to the peer.
 **/
int dhcp_data_req(struct dhcp_conn_t *conn, void *pack, unsigned len)
{
  struct dhcp_t *this = conn->parent;
  int length = len + DHCP_ETH_HLEN;
  
  struct dhcp_ippacket_t packet;


  /* Ethernet header */
  memcpy(packet.ethh.dst, conn->hismac, DHCP_ETH_ALEN);
  memcpy(packet.ethh.src, this->hwaddr, DHCP_ETH_ALEN);
  packet.ethh.prot = htons(DHCP_ETH_IP);

  /* IP Packet */
  memcpy(&packet.iph, pack, len);
  
  switch (conn->authstate) {
  case DHCP_AUTH_PASS:
  case DHCP_AUTH_UNAUTH_TOS:
  case DHCP_AUTH_AUTH_TOS:
    /* Pass packets unmodified */
    break; 
  case DHCP_AUTH_DNAT:
    /* Undo destination NAT */
    if (dhcp_undoDNAT(conn, &packet, length))
      return 0;
    break;
  case DHCP_AUTH_DROP: 
  default:
    return 0;
  }

  return dhcp_send(this, this->fd, DHCP_ETH_IP, conn->hismac, this->ifindex,
		   &packet, length);
}




/**
 * dhcp_sendARP()
 * Send ARP message to peer
 **/
int dhcp_sendARP(struct dhcp_conn_t *conn, 
		 struct dhcp_arp_fullpacket_t *pack, int len) {

  struct dhcp_t *this = conn->parent;
  struct dhcp_arp_fullpacket_t packet;
  uint16_t length = sizeof(packet);
  struct in_addr reqaddr;

  /* Get local copy */
  memcpy(&reqaddr.s_addr, pack->arp.tpa, DHCP_IP_ALEN);

  /* Check that request is within limits */

  
  /* Is ARP request for clients own address: Ignore */
  if (conn->hisip.s_addr == reqaddr.s_addr)
    return 0;

  /* If ARP request outside of mask: Ignore */
  if ((conn->hisip.s_addr & conn->hismask.s_addr) !=
      (reqaddr.s_addr & conn->hismask.s_addr))
    return 0;

  /* Get packet default values */
  memset(&packet, 0, sizeof(packet));
	 
  /* ARP Payload */
  packet.arp.hrd = htons(DHCP_HTYPE_ETH);
  packet.arp.pro = htons(DHCP_ETH_IP);
  packet.arp.hln = DHCP_ETH_ALEN;
  packet.arp.pln = DHCP_IP_ALEN;
  packet.arp.op  = htons(DHCP_ARP_REPLY);

  /* Source address */
  memcpy(packet.arp.sha, this->arp_hwaddr, DHCP_ETH_ALEN);
  memcpy(packet.arp.spa, &reqaddr.s_addr, DHCP_IP_ALEN);

  /* Target address */
  memcpy(packet.arp.tha, &conn->hismac, DHCP_ETH_ALEN);
  memcpy(packet.arp.tpa, &conn->hisip.s_addr, DHCP_IP_ALEN);
  

  /* Ethernet header */
  memcpy(packet.ethh.dst, conn->hismac, DHCP_ETH_ALEN);
  memcpy(packet.ethh.src, this->hwaddr, DHCP_ETH_ALEN);
  packet.ethh.prot = htons(DHCP_ETH_ARP);

  return dhcp_send(this, this->arp_fd, DHCP_ETH_ARP, conn->hismac, this->arp_ifindex,
		   &packet, length);
}


int dhcp_receive_arp(struct dhcp_t *this, 
		     struct dhcp_arp_fullpacket_t *pack, int len) {
  
  struct dhcp_conn_t *conn;
  unsigned char const bmac[DHCP_ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  struct in_addr addr;

  /* Check that this is ARP request */
  if (pack->arp.op != htons(DHCP_ARP_REQUEST)) {
    if (this->debug) printf("Received other ARP than request!\n");
    return 0;
  }


  /* Check that MAC address is our MAC or Broadcast */
  if ((memcmp(pack->ethh.dst, this->hwaddr, DHCP_ETH_ALEN)) && (memcmp(pack->ethh.dst, bmac, DHCP_ETH_ALEN))) {
    if (this->debug) printf("Received ARP request for other destination!\n");
    return 0;
  }

  /* Check to see if we know MAC address. */
  if (dhcp_hashget(this, &conn, pack->ethh.src)) {
    if (this->debug) printf("Address not found\n");
    
    /* Do we allow dynamic allocation of IP addresses? */
    if (!this->allowdyn)  /* TODO: Experimental */
      return 0; 
    
    /* Allocate new connection */
    if (dhcp_newconn(this, &conn, pack->ethh.src)) /* TODO: Experimental */
      return 0; /* Out of connections */
  }
  
  
  gettimeofday(&conn->lasttime, NULL);

  if (!conn->hisip.s_addr) {
    if (this->debug) printf("ARP request did not come from known client!\n");
    return 0; /* Only reply if he was allocated an address */
  }
  
  if (memcmp(&conn->ourip.s_addr, pack->arp.tpa, 4)) {
    if (this->debug) printf("Did not ask for router address: %.8x - %.2x%.2x%.2x%.2x\n", conn->ourip.s_addr, 
			    pack->arp.tpa[0],
			    pack->arp.tpa[1],
			    pack->arp.tpa[2],
			    pack->arp.tpa[3]);
    return 0; /* Only reply if he asked for his router address */
  }
  
  (void)dhcp_sendARP(conn, pack, len);

  return 0;
}


/**
 * dhcp_arp_ind()
 * Call this function when a new ARP packet has arrived. This function
 * should be part of a select() loop in the application.
 **/
int dhcp_arp_ind(struct dhcp_t *this)  /* ARP Indication */
{
  struct dhcp_arp_fullpacket_t packet;
  int length;
  
  struct dhcp_conn_t *conn;
  unsigned char const bmac[DHCP_ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

  if (this->debug) printf("ARP Packet Received!\n");

  if ((length = recv(this->arp_fd, &packet, sizeof(packet), 0)) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "recv(fd=%d, len=%d) failed",
	    this->arp_fd, sizeof(packet));
    return -1;
  }

  dhcp_receive_arp(this, &packet, length);

  return 0;
}




/**
 * eapol_sendNAK()
 * Send of a EAPOL negative acknowledge message to a peer.
 * NAK messages are always sent to broadcast IP address (
 * except when using a EAPOL relay server)
 **/
int dhcp_senddot1x(struct dhcp_conn_t *conn,  
		   struct dhcp_dot1xpacket_t *pack, int len) {

  struct dhcp_t *this = conn->parent;
  
  return dhcp_send(this, this->fd, DHCP_ETH_EAPOL, conn->hismac, this->ifindex,
		   pack, len);
}

/**
 * eapol_sendNAK()
 * Send of a EAPOL negative acknowledge message to a peer.
 * NAK messages are always sent to broadcast IP address (
 * except when using a EAPOL relay server)
 **/
int dhcp_sendEAP(struct dhcp_conn_t *conn, void *pack, int len) {

  struct dhcp_t *this = conn->parent;
  struct dhcp_dot1xpacket_t packet;

  /* Ethernet header */
  memcpy(packet.ethh.dst, conn->hismac, DHCP_ETH_ALEN);
  memcpy(packet.ethh.src, this->hwaddr, DHCP_ETH_ALEN);
  packet.ethh.prot = htons(DHCP_ETH_EAPOL);
  
  /* 802.1x header */
  packet.dot1x.ver  = 1;
  packet.dot1x.type = 0; /* EAP */
  packet.dot1x.len =  ntohs(len);

  memcpy(&packet.eap, pack, len);
  
  return dhcp_send(this, this->fd, DHCP_ETH_EAPOL, conn->hismac, this->ifindex,
		   &packet, (DHCP_ETH_HLEN + 4 + len));
}

int dhcp_sendEAPreject(struct dhcp_conn_t *conn, void *pack, int len) {

  /*struct dhcp_t *this = conn->parent;*/

  struct dhcp_eap_t packet;


  if (pack) {
    (void)dhcp_sendEAP(conn, pack, len);
  }
  else {
    memset(&packet, 0, sizeof(packet));
    packet.code      =  4;
    packet.id        =  1; /* TODO ??? */
    packet.length    =  ntohs(4);
  
    dhcp_sendEAP(conn, &packet, 4);
  }

  return 0;

}


/**
 * dhcp_eapol_ind()
 * Call this function when a new EAPOL packet has arrived. This function
 * should be part of a select() loop in the application.
 **/
int dhcp_eapol_ind(struct dhcp_t *this)  /* EAPOL Indication */
{
  struct dhcp_dot1xpacket_t packet;
  int length;
  
  struct dhcp_conn_t *conn = NULL;
  unsigned char const bmac[DHCP_ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  unsigned char const amac[DHCP_ETH_ALEN] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x03};

  if (this->debug) printf("EAPOL packet received\n");
  
  if ((length = recv(this->eapol_fd, &packet, sizeof(packet), 0)) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "recv(fd=%d, len=%d) failed",
	    this->fd, sizeof(packet));
    return -1;
  }
  
  /* Check to see if we know MAC address. */
  if (!dhcp_hashget(this, &conn, packet.ethh.src)) {
    if (this->debug) printf("Address found\n");
  }
  else {
    if (this->debug) printf("Address not found\n");
  }
  
  /* Check that MAC address is our MAC, Broadcast or authentication MAC */
  if ((memcmp(packet.ethh.dst, this->hwaddr, DHCP_ETH_ALEN)) && (memcmp(packet.ethh.dst, bmac, DHCP_ETH_ALEN)) && (memcmp(packet.ethh.dst, amac, DHCP_ETH_ALEN)))
    return 0;
  
  if (this->debug) printf("IEEE 802.1x Packet: %.2x, %.2x %d\n",
			  packet.dot1x.ver, packet.dot1x.type,
			  ntohs(packet.dot1x.len));
  
  if (packet.dot1x.type == 1) { /* Start */
    struct dhcp_dot1xpacket_t pack;
    memset(&pack, 0, sizeof(pack));
    
    /* Allocate new connection */
    if (conn == NULL) {
      if (dhcp_newconn(this, &conn, packet.ethh.src))
	return 0; /* Out of connections */
    }

    /* Ethernet header */
    memcpy(pack.ethh.dst, packet.ethh.src, DHCP_ETH_ALEN);
    memcpy(pack.ethh.src, this->hwaddr, DHCP_ETH_ALEN);
    pack.ethh.prot = htons(DHCP_ETH_EAPOL);

    /* 802.1x header */
    pack.dot1x.ver  = 1;
    pack.dot1x.type = 0; /* EAP */
    pack.dot1x.len =  ntohs(5);
    
    /* EAP Packet */
    pack.eap.code      =  1;
    pack.eap.id        =  1;
    pack.eap.length    =  ntohs(5);
    pack.eap.type      =  1; /* Identity */
    (void)dhcp_senddot1x(conn, &pack, DHCP_ETH_HLEN + 4 + 5);
    return 0;
  }
  else if (packet.dot1x.type == 0) { /* EAP */

    /* TODO: Currently we only support authentications starting with a
       client sending a EAPOL start message. Need to also support
       authenticator initiated communications. */
    if (!conn)
      return 0;

    gettimeofday(&conn->lasttime, NULL);
    
    if (this ->cb_eap_ind)
      this ->cb_eap_ind(conn, &packet.eap, ntohs(packet.eap.length));
    return 0;
  }
  else { /* Check for logoff */
    return 0;
  }
}


/**
 * dhcp_set_cb_eap_ind()
 * Set callback function which is called when packet has arrived
 * Used for eap packets
 **/
extern int dhcp_set_cb_eap_ind(struct dhcp_t *this, 
  int (*cb_eap_ind) (struct dhcp_conn_t *conn, void *pack, unsigned len))
{
  this ->cb_eap_ind = cb_eap_ind;
  return 0;
}


/**
 * dhcp_set_cb_data_ind()
 * Set callback function which is called when packet has arrived
 **/
extern int dhcp_set_cb_data_ind(struct dhcp_t *this, 
  int (*cb_data_ind) (struct dhcp_conn_t *conn, void *pack, unsigned len))
{
  this ->cb_data_ind = cb_data_ind;
  return 0;
}


/**
 * dhcp_set_cb_data_ind()
 * Set callback function which is called when a dhcp request is received
 **/
extern int dhcp_set_cb_request(struct dhcp_t *this, 
  int (*cb_request) (struct dhcp_conn_t *conn, struct in_addr *addr))
{
  this ->cb_request = cb_request;
  return 0;
}


/**
 * dhcp_set_cb_connect()
 * Set callback function which is called when a connection is created
 **/
extern int dhcp_set_cb_connect(struct dhcp_t *this, 
             int (*cb_connect) (struct dhcp_conn_t *conn))
{
  this ->cb_connect = cb_connect;
  return 0;
}

/**
 * dhcp_set_cb_disconnect()
 * Set callback function which is called when a connection is deleted
 **/
extern int dhcp_set_cb_disconnect(struct dhcp_t *this, 
  int (*cb_disconnect) (struct dhcp_conn_t *conn))
{
  this ->cb_disconnect = cb_disconnect;
  return 0;
}



#if defined(__FreeBSD__) || defined (__OpenBSD__) || defined (__APPLE__)

int dhcp_receive(struct dhcp_t *this) {
  /*
	       struct interface_info *interface, unsigned char *buf,
	       size_t len, struct sockaddr_in *from, struct hardware *hfrom)
{*/
  int length = 0, offset = 0;
  struct bpf_hdr *hdrp;
  struct dhcp_ethhdr_t *ethhdr;
  
  if (this->rbuf_offset == this->rbuf_len) {
    length = read(this->fd, this->rbuf, this->rbuf_max);
    if (length <= 0)
      return (length);
    this->rbuf_offset = 0;
    this->rbuf_len = length;
  }
  
  while (this->rbuf_offset != this->rbuf_len) {
    
    if (this->rbuf_len - this->rbuf_offset < sizeof(struct bpf_hdr)) {
      this->rbuf_offset = this->rbuf_len;
      continue;
    }
    
    hdrp = (struct bpf_hdr *) &this->rbuf[this->rbuf_offset];
    
    if (this->rbuf_offset + hdrp->bh_hdrlen + hdrp->bh_caplen > 
	this->rbuf_len) {
      this->rbuf_offset = this->rbuf_len;
      continue;
    }

    if (hdrp->bh_caplen != hdrp->bh_datalen) {
      this->rbuf_offset += hdrp->bh_hdrlen + hdrp->bh_caplen;
      continue;
    }

    ethhdr = (struct dhcp_ethhdr_t *) 
      (this->rbuf + this->rbuf_offset + hdrp->bh_hdrlen);

    switch (ntohs(ethhdr->prot)) {
    case DHCP_ETH_IP:
      dhcp_receive_ip(this, (struct dhcp_ippacket_t*) ethhdr, hdrp->bh_caplen);
      break;
    case DHCP_ETH_ARP:
      dhcp_receive_arp(this, (struct dhcp_arp_fullpacket_t*) ethhdr, 
		      hdrp->bh_caplen);
      break;
    case DHCP_ETH_EAPOL:
    default:
      break;
    }
    this->rbuf_offset += hdrp->bh_hdrlen + hdrp->bh_caplen;
  };
  return (0);
}
#endif
