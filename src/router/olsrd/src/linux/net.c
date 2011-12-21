
/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tonnesen(andreto@olsr.org)
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

/*
 * Linux specific code
 */

#define __BSD_SOURCE 1

#include "../net_os.h"
#include "../ipcalc.h"
#include "../olsr.h"
#include "../log.h"
#include "kernel_tunnel.h"

#include <net/if.h>

#include <sys/ioctl.h>
#include <sys/utsname.h>

#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <syslog.h>
#include <unistd.h>

/**
 * Fix bug in GLIBC, see https://bugzilla.redhat.com/show_bug.cgi?id=635260
 */
#ifdef IPTOS_CLASS
#undef IPTOS_CLASS
#endif
#define IPTOS_CLASS(class)    ((class) & IPTOS_CLASS_MASK)

#define IPV6_ADDR_LOOPBACK      0x0010U
#define IPV6_ADDR_LINKLOCAL     0x0020U
#define IPV6_ADDR_SITELOCAL     0x0040U

/* ip forwarding */
#define PROC_IPFORWARD_V4 "/proc/sys/net/ipv4/ip_forward"
#define PROC_IPFORWARD_V6 "/proc/sys/net/ipv6/conf/all/forwarding"

/* Redirect proc entry */
#define PROC_IF_REDIRECT "/proc/sys/net/ipv4/conf/%s/send_redirects"
#define PROC_ALL_REDIRECT "/proc/sys/net/ipv4/conf/all/send_redirects"

/* IP spoof proc entry */
#define PROC_IF_SPOOF "/proc/sys/net/ipv4/conf/%s/rp_filter"
#define PROC_ALL_SPOOF "/proc/sys/net/ipv4/conf/all/rp_filter"


/* list of IPv6 interfaces */
#define PATH_PROCNET_IFINET6           "/proc/net/if_inet6"

/*
 *Wireless definitions for ioctl calls
 *(from linux/wireless.h)
 */
#define SIOCGIWNAME	0x8B01  /* get name == wireless protocol */
#define SIOCGIWRATE	0x8B21  /* get default bit rate (bps) */

/* The original state of the IP forwarding proc entry */
static char orig_fwd_state;
static char orig_global_redirect_state;
static char orig_global_rp_filter;
static char orig_tunnel_rp_filter;
#if 0 // should not be necessary for IPv6 */
static char orig_tunnel6_rp_filter;
#endif

/**
 *Bind a socket to a device
 *
 *@param sock the socket to bind
 *@param dev_name name of the device
 *
 *@return negative if error
 */

int
bind_socket_to_device(int sock, char *dev_name)
{
  /*
   *Bind to device using the SO_BINDTODEVICE flag
   */
  OLSR_PRINTF(3, "Binding socket %d to device %s\n", sock, dev_name);
  return setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, dev_name, strlen(dev_name) + 1);
}

static int writeToProc(const char *file, char *old, char value) {
  int fd;
  char rv;

  if ((fd = open(file, O_RDWR)) < 0) {
    OLSR_PRINTF(0, "Error, cannot open proc entry %s: %s (%d)\n", file, strerror(errno), errno);
    return -1;
  }

  if (read(fd, &rv, 1) != 1) {
    OLSR_PRINTF(0, "Error, cannot read proc entry %s: %s (%d)\n", file, strerror(errno), errno);
    return -1;
  }

  if (rv != value) {
    if (lseek(fd, SEEK_SET, 0) == -1) {
      OLSR_PRINTF(0, "Error, cannot rewind proc entry %s: %s (%d)\n", file, strerror(errno), errno);
      return -1;
    }

    if (write(fd, &value, 1) != 1) {
      OLSR_PRINTF(0, "Error, cannot write proc entry %s: %s (%d)\n", file, strerror(errno), errno);
      return -1;
    }
  }

  if (close(fd) != 0) {
    OLSR_PRINTF(0, "Error while closing proc entry %s: %s (%d)\n", file, strerror(errno), errno);
    return -1;
  }

  if (old) {
    *old = rv;
  }
  olsr_syslog(OLSR_LOG_INFO, "Writing '%c' (was %c) to %s", value, rv, file);
  return 0;
}

static bool is_at_least_linuxkernel_2_6_31(void) {
  struct utsname uts;
  char *next;
  int first = 0, second = 0, third = 0;

  memset(&uts, 0, sizeof(uts));
  if (uname(&uts)) {
    OLSR_PRINTF(1, "Error, could not read kernel version: %s (%d)\n", strerror(errno), errno);
    return false;
  }

  first = strtol(uts.release, &next, 10);
  /* check for linux 3.x */
  if (first >= 3) {
    return true;
  }

  if (*next != '.') {
    goto kernel_parse_error;
  }

  second = strtol(next+1, &next, 10);
  if (*next != '.') {
    goto kernel_parse_error;
  }

  third = strtol(next+1, NULL, 10);

  /* better or equal than linux 2.6.31 ? */
  return first == 2 && second == 6 && third >= 31;

kernel_parse_error:
  OLSR_PRINTF(1, "Error, cannot parse kernel version: %s\n", uts.release);
  return false;
}

/**
 * Setup global interface options (icmp redirect, ip forwarding, rp_filter)
 * @return 1 on success 0 on failure
 */
void
net_os_set_global_ifoptions(void) {
  if (writeToProc(olsr_cnf->ip_version == AF_INET ? PROC_IPFORWARD_V4 : PROC_IPFORWARD_V6, &orig_fwd_state, '1')) {
    OLSR_PRINTF(1, "Warning, could not enable IP forwarding!\n"
        "you should manually ensure that IP forwarding is enabled!\n\n");
    olsr_startup_sleep(3);
  }

  if (olsr_cnf->smart_gw_active) {
    char procfile[FILENAME_MAX];

    /* Generate the procfile name */
    if (olsr_cnf->ip_version == AF_INET || olsr_cnf->use_niit) {
      snprintf(procfile, sizeof(procfile), PROC_IF_SPOOF, TUNNEL_ENDPOINT_IF);
      if (writeToProc(procfile, &orig_tunnel_rp_filter, '0')) {
        OLSR_PRINTF(0, "WARNING! Could not disable the IP spoof filter for tunnel!\n"
            "you should mannually ensure that IP spoof filtering is disabled!\n\n");

        olsr_startup_sleep(3);
      }
    }

#if 0 // should not be necessary for IPv6
    if (olsr_cnf->ip_version == AF_INET6) {
      snprintf(procfile, sizeof(procfile), PROC_IF_SPOOF, TUNNEL_ENDPOINT_IF6);
      if (writeToProc(procfile, &orig_tunnel6_rp_filter, '0')) {
        OLSR_PRINTF(0, "WARNING! Could not disable the IP spoof filter for tunnel6!\n"
            "you should mannually ensure that IP spoof filtering is disabled!\n\n");

        olsr_startup_sleep(3);
      }
    }
#endif
  }

  if (olsr_cnf->ip_version == AF_INET) {
    if (writeToProc(PROC_ALL_REDIRECT, &orig_global_redirect_state, '0')) {
      OLSR_PRINTF(1, "WARNING! Could not disable ICMP redirects!\n"
          "you should manually ensure that ICMP redirects are disabled!\n\n");

      olsr_startup_sleep(3);
    }

    /* check kernel version and disable global rp_filter */
    if (is_at_least_linuxkernel_2_6_31()) {
      if (writeToProc(PROC_ALL_SPOOF, &orig_global_rp_filter, '0')) {
        OLSR_PRINTF(1, "WARNING! Could not disable global rp_filter (necessary for kernel 2.6.31 and higher!\n"
            "you should manually ensure that rp_filter is disabled!\n\n");

        olsr_startup_sleep(3);
      }
    }
  }
  return;
}

/**
 *
 *@return 1 on sucess 0 on failiure
 */
int
net_os_set_ifoptions(const char *if_name, struct interface *iface)
{
  char procfile[FILENAME_MAX];
  if (olsr_cnf->ip_version == AF_INET6)
    return -1;

  /* Generate the procfile name */
  snprintf(procfile, sizeof(procfile), PROC_IF_REDIRECT, if_name);

  if (writeToProc(procfile, &iface->nic_state.redirect, '0')) {
    OLSR_PRINTF(0, "WARNING! Could not disable ICMP redirects!\n"
        "you should mannually ensure that ICMP redirects are disabled!\n\n");
    olsr_startup_sleep(3);
    return 0;
  }

  /* Generate the procfile name */
  snprintf(procfile, sizeof(procfile), PROC_IF_SPOOF, if_name);

  if (writeToProc(procfile, &iface->nic_state.spoof, '0')) {
    OLSR_PRINTF(0, "WARNING! Could not disable the IP spoof filter!\n"
        "you should mannually ensure that IP spoof filtering is disabled!\n\n");

    olsr_startup_sleep(3);
    return 0;
  }
  return 1;
}

/**
 *Resets the spoof filter and ICMP redirect settings
 */
int
net_os_restore_ifoptions(void)
{
  struct interface *ifs;
  char procfile[FILENAME_MAX];

  OLSR_PRINTF(1, "Restoring network state\n");

  /* Restore IP forwarding to "off" */
  if (writeToProc(olsr_cnf->ip_version == AF_INET ? PROC_IPFORWARD_V4 : PROC_IPFORWARD_V6, NULL, orig_fwd_state)) {
    OLSR_PRINTF(1, "Error, could not restore ip_forward settings\n");
  }

  if (olsr_cnf->smart_gw_active && (olsr_cnf->ip_version == AF_INET || olsr_cnf->use_niit)) {
    /* Generate the procfile name */
    snprintf(procfile, sizeof(procfile), PROC_IF_SPOOF, TUNNEL_ENDPOINT_IF);
    if (writeToProc(procfile, NULL, orig_tunnel_rp_filter)) {
      OLSR_PRINTF(0, "WARNING! Could not restore the IP spoof filter for tunnel!\n");
    }

#if 0 // should not be necessary for IPv6
    if (olsr_cnf->ip_version == AF_INET6) {
      snprintf(procfile, sizeof(procfile), PROC_IF_SPOOF, TUNNEL_ENDPOINT_IF6);
      if (writeToProc(procfile, NULL, orig_tunnel6_rp_filter)) {
        OLSR_PRINTF(0, "WARNING! Could not restore the IP spoof filter for tunnel6!\n");
      }
    }
#endif
  }

  if (olsr_cnf->ip_version == AF_INET) {
    /* Restore global ICMP redirect setting */
    if (writeToProc(PROC_ALL_REDIRECT, NULL, orig_global_redirect_state)) {
      OLSR_PRINTF(1, "Error, could not restore global icmp_redirect setting\n");
    }

    /* Restore global rp_filter setting for linux 2.6.31+ */
    if (is_at_least_linuxkernel_2_6_31()) {
      if (writeToProc(PROC_ALL_SPOOF, NULL, orig_global_rp_filter)) {
        OLSR_PRINTF(1, "Error, could not restore global rp_filter setting\n");
      }
    }
    for (ifs = ifnet; ifs != NULL; ifs = ifs->int_next) {
      /* Discard host-emulation interfaces */
      if (ifs->is_hcif)
        continue;

      /* ICMP redirects */
      snprintf(procfile, sizeof(procfile), PROC_IF_REDIRECT, ifs->int_name);
      if (writeToProc(procfile, NULL, ifs->nic_state.redirect)) {
        OLSR_PRINTF(1, "Error, could not restore icmp_redirect for interface %s\n", ifs->int_name);
      }

      /* Spoof filter */
      sprintf(procfile, PROC_IF_SPOOF, ifs->int_name);
      if (writeToProc(procfile, NULL, ifs->nic_state.spoof)) {
        OLSR_PRINTF(1, "Error, could not restore rp_filter for interface %s\n", ifs->int_name);
      }
    }
  }
  return 1;
}

/**
 *Creates a blocking tcp socket for communication with switch daemon.
 *@param sa sockaddr struct. Used for bind(2).
 *@return the FD of the socket or -1 on error.
 */
int
gethemusocket(struct sockaddr_in *pin)
{
  int sock, on = 1;

  OLSR_PRINTF(1, "       Connecting to switch daemon port 10150...");
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("hcsocket");
    syslog(LOG_ERR, "hcsocket: %m");
    return -1;
  }

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
    perror("SO_REUSEADDR failed");
    close(sock);
    return -1;
  }
  /* connect to PORT on HOST */
  if (connect(sock, (struct sockaddr *)pin, sizeof(*pin)) < 0) {
    printf("FAILED\n");
    fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno));
    printf("connection refused\n");
    close(sock);
    return -1;
  }

  printf("OK\n");

  /* Keep TCP socket blocking */
  return sock;
}

/**
 *Creates a nonblocking broadcast socket.
 *@param sa sockaddr struct. Used for bind(2).
 *@return the FD of the socket or -1 on error.
 */
int
getsocket(int bufspace, struct interface *ifp)
{
  struct sockaddr_in sin;
  int on;
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    perror("socket");
    syslog(LOG_ERR, "socket: %m");
    return -1;
  }

  on = 1;
#ifdef SO_BROADCAST
  if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) < 0) {
    perror("setsockopt");
    syslog(LOG_ERR, "setsockopt SO_BROADCAST: %m");
    close(sock);
    return -1;
  }
#endif

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
    perror("SO_REUSEADDR failed");
    close(sock);
    return -1;
  }
#ifdef SO_RCVBUF
  if(bufspace > 0) {
    for (on = bufspace;; on -= 1024) {
      if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &on, sizeof(on)) == 0)
        break;
      if (on <= 8 * 1024) {
        perror("setsockopt");
        syslog(LOG_ERR, "setsockopt SO_RCVBUF: %m");
        break;
      }
    }
  }
#endif

  /*
   * WHEN USING KERNEL 2.6 THIS MUST HAPPEN PRIOR TO THE PORT BINDING!!!!
   */

  /* Bind to device */
  if (bind_socket_to_device(sock, ifp->int_name) < 0) {
    fprintf(stderr, "Could not bind socket to device... exiting!\n\n");
    syslog(LOG_ERR, "Could not bind socket to device... exiting!\n\n");
    close(sock);
    return -1;
  }

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(olsr_cnf->olsrport);

  if(bufspace <= 0) {
    sin.sin_addr.s_addr = ifp->int_addr.sin_addr.s_addr;
  }

  if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
    perror("bind");
    syslog(LOG_ERR, "bind: %m");
    close(sock);
    return -1;
  }

  on = fcntl(sock, F_GETFL);
  if (on == -1) {
    syslog(LOG_ERR, "fcntl (F_GETFL): %m\n");
  } else {
    if (fcntl(sock, F_SETFL, on | O_NONBLOCK) == -1) {
      syslog(LOG_ERR, "fcntl O_NONBLOCK: %m\n");
    }
  }
  return sock;
}

/**
 *Creates a nonblocking IPv6 socket
 *@param sin sockaddr_in6 struct. Used for bind(2).
 *@return the FD of the socket or -1 on error.
 */
int
getsocket6(int bufspace, struct interface *ifp)
{
  struct sockaddr_in6 sin;
  int on;
  int sock = socket(AF_INET6, SOCK_DGRAM, 0);
  if (sock < 0) {
    perror("socket");
    syslog(LOG_ERR, "socket: %m");
    return (-1);
  }
#ifdef IPV6_V6ONLY
  on = 1;
  if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on)) < 0) {
    perror("setsockopt(IPV6_V6ONLY)");
    syslog(LOG_ERR, "setsockopt(IPV6_V6ONLY): %m");
  }
#endif

  //#ifdef SO_BROADCAST
  /*
     if (setsockopt(sock, SOL_SOCKET, SO_MULTICAST, &on, sizeof (on)) < 0)
     {
     perror("setsockopt");
     syslog(LOG_ERR, "setsockopt SO_BROADCAST: %m");
     close(sock);
     return (-1);
     }
   */
  //#endif

#ifdef SO_RCVBUF
  if(bufspace > 0) {
    for (on = bufspace;; on -= 1024) {
      if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &on, sizeof(on)) == 0)
        break;
      if (on <= 8 * 1024) {
        perror("setsockopt");
        syslog(LOG_ERR, "setsockopt SO_RCVBUF: %m");
        break;
      }
    }
  }
#endif

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
    perror("SO_REUSEADDR failed");
    close(sock);
    return (-1);
  }

  /*
   * WHEN USING KERNEL 2.6 THIS MUST HAPPEN PRIOR TO THE PORT BINDING!!!!
   */

  /* Bind to device */
  if (bind_socket_to_device(sock, ifp->int_name) < 0) {
    fprintf(stderr, "Could not bind socket to device... exiting!\n\n");
    syslog(LOG_ERR, "Could not bind socket to device... exiting!\n\n");
    close(sock);
    return -1;
  }

  memset(&sin, 0, sizeof(sin));
  sin.sin6_family = AF_INET6;
  sin.sin6_port = htons(olsr_cnf->olsrport);
  sin.sin6_scope_id = ifp->if_index;

  if(bufspace <= 0) {
    memcpy(&sin.sin6_addr, &ifp->int6_addr.sin6_addr, sizeof(struct in6_addr));
  }

  if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
    struct ipaddr_str buf;
    OLSR_PRINTF(1, "Error, cannot bind address %s to %s-socket: %s (%d)\n",
        inet_ntop(sin.sin6_family, &sin.sin6_addr, buf.buf, sizeof(buf)),
        bufspace <= 0 ? "transmit" : "receive",
        strerror(errno), errno);
    syslog(LOG_ERR, "bind: %m");
    close(sock);
    return (-1);
  }

  on = fcntl(sock, F_GETFL);
  if (on == -1) {
    syslog(LOG_ERR, "fcntl (F_GETFL): %m\n");
  } else {
    if (fcntl(sock, F_SETFL, on | O_NONBLOCK) == -1) {
      syslog(LOG_ERR, "fcntl O_NONBLOCK: %m\n");
    }
  }
  return sock;
}

int
join_mcast(struct interface *ifs, int sock)
{
  /* See linux/in6.h */
  struct ipaddr_str buf;
  struct ipv6_mreq mcastreq;

  mcastreq.ipv6mr_multiaddr = ifs->int6_multaddr.sin6_addr;
  mcastreq.ipv6mr_interface = ifs->if_index;

#if !defined __FreeBSD__ && !defined __FreeBSD_kernel__ && !defined __MacOSX__ && !defined __NetBSD__
  OLSR_PRINTF(3, "Interface %s joining multicast %s...", ifs->int_name, ip6_to_string(&buf, &ifs->int6_multaddr.sin6_addr));
  /* Send multicast */
  if (setsockopt(sock, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (char *)&mcastreq, sizeof(struct ipv6_mreq)) < 0) {
    perror("Join multicast");
    return -1;
  }
#else
#warning implement IPV6_ADD_MEMBERSHIP
#endif

  /* Old libc fix */
#ifdef IPV6_JOIN_GROUP
  /* Join reciever group */
  if (setsockopt(sock, IPPROTO_IPV6, IPV6_JOIN_GROUP, (char *)&mcastreq, sizeof(struct ipv6_mreq)) < 0)
#else
  /* Join reciever group */
  if (setsockopt(sock, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (char *)&mcastreq, sizeof(struct ipv6_mreq)) < 0)
#endif
  {
    perror("Join multicast send");
    return -1;
  }

  if (setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_IF, (char *)&mcastreq.ipv6mr_interface, sizeof(mcastreq.ipv6mr_interface)) < 0) {
    perror("Set multicast if");
    return -1;
  }

  OLSR_PRINTF(3, "OK\n");
  return 0;
}

/*
 *From net-tools lib/interface.c
 *
 */
int
get_ipv6_address(char *ifname, struct sockaddr_in6 *saddr6, struct olsr_ip_prefix *prefix)
{
  char addr6[40], devname[IFNAMSIZ];
  char addr6p[8][5];
  int plen, scope, dad_status, if_idx;
  FILE *f;
  union olsr_ip_addr tmp_ip;

  if ((f = fopen(PATH_PROCNET_IFINET6, "r")) != NULL) {
    while (fscanf
           (f, "%4s%4s%4s%4s%4s%4s%4s%4s %x %02x %02x %02x %20s\n", addr6p[0], addr6p[1], addr6p[2], addr6p[3], addr6p[4],
            addr6p[5], addr6p[6], addr6p[7], &if_idx, &plen, &scope, &dad_status, devname) != EOF) {
      if (!strcmp(devname, ifname)) {
        bool isNetWide = false;
        sprintf(addr6, "%s:%s:%s:%s:%s:%s:%s:%s", addr6p[0], addr6p[1], addr6p[2], addr6p[3], addr6p[4], addr6p[5], addr6p[6],
                addr6p[7]);
        OLSR_PRINTF(5, "\tinet6 addr: %s\n", addr6);
        OLSR_PRINTF(5, "\tScope: %d\n", scope);

        inet_pton(AF_INET6, addr6, &tmp_ip.v6);

        isNetWide = (scope != IPV6_ADDR_LOOPBACK) && (scope != IPV6_ADDR_LINKLOCAL) && (scope != IPV6_ADDR_SITELOCAL);

        if ((prefix == NULL && isNetWide) || (prefix != NULL && ip_in_net(&tmp_ip, prefix))) {
          OLSR_PRINTF(4, "Found addr: %s:%s:%s:%s:%s:%s:%s:%s\n", addr6p[0], addr6p[1], addr6p[2], addr6p[3], addr6p[4], addr6p[5],
                      addr6p[6], addr6p[7]);
          memcpy(&saddr6->sin6_addr, &tmp_ip.v6, sizeof(struct in6_addr));
          fclose(f);
          return 1;
        }
      }
    }
    fclose(f);
  }
  return 0;
}

/**
 * Wrapper for sendto(2)
 */
ssize_t
olsr_sendto(int s, const void *buf, size_t len, int flags, const struct sockaddr * to, socklen_t tolen)
{
  return sendto(s, buf, len, flags, to, tolen);
}

/**
 * Wrapper for recvfrom(2)
 */

ssize_t
olsr_recvfrom(int s, void *buf, size_t len, int flags, struct sockaddr * from, socklen_t * fromlen)
{
  return recvfrom(s, buf, len, flags, from, fromlen);
}

/**
 * Wrapper for select(2)
 */

int
olsr_select(int nfds, fd_set * readfds, fd_set * writefds, fd_set * exceptfds, struct timeval *timeout)
{
  return select(nfds, readfds, writefds, exceptfds, timeout);
}

int
check_wireless_interface(char *ifname)
{
  struct ifreq ifr;

  memset(&ifr, 0, sizeof(ifr));
  strscpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));

  return (ioctl(olsr_cnf->ioctl_s, SIOCGIWNAME, &ifr) >= 0) ? 1 : 0;
}

#if 0

#include <linux/sockios.h>
#include <linux/types.h>

/* This data structure is used for all the MII ioctl's */
struct mii_data {
  __u16 phy_id;
  __u16 reg_num;
  __u16 val_in;
  __u16 val_out;
};

/* Basic Mode Control Register */
#define MII_BMCR		0x00
#define  MII_BMCR_RESET		0x8000
#define  MII_BMCR_LOOPBACK	0x4000
#define  MII_BMCR_100MBIT	0x2000
#define  MII_BMCR_AN_ENA	0x1000
#define  MII_BMCR_ISOLATE	0x0400
#define  MII_BMCR_RESTART	0x0200
#define  MII_BMCR_DUPLEX	0x0100
#define  MII_BMCR_COLTEST	0x0080

/* Basic Mode Status Register */
#define MII_BMSR		0x01
#define  MII_BMSR_CAP_MASK	0xf800
#define  MII_BMSR_100BASET4	0x8000
#define  MII_BMSR_100BASETX_FD	0x4000
#define  MII_BMSR_100BASETX_HD	0x2000
#define  MII_BMSR_10BASET_FD	0x1000
#define  MII_BMSR_10BASET_HD	0x0800
#define  MII_BMSR_NO_PREAMBLE	0x0040
#define  MII_BMSR_AN_COMPLETE	0x0020
#define  MII_BMSR_REMOTE_FAULT	0x0010
#define  MII_BMSR_AN_ABLE	0x0008
#define  MII_BMSR_LINK_VALID	0x0004
#define  MII_BMSR_JABBER	0x0002
#define  MII_BMSR_EXT_CAP	0x0001

int
calculate_if_metric(char *ifname)
{
  if (check_wireless_interface(ifname)) {
    struct ifreq ifr;
    strscpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));

    /* Get bit rate */
    if (ioctl(olsr_cnf->ioctl_s, SIOCGIWRATE, &ifr) < 0) {
      OLSR_PRINTF(1, "Not able to find rate for WLAN interface %s\n", ifname);
      return WEIGHT_WLAN_11MB;
    }

    OLSR_PRINTF(1, "Bitrate %d\n", ifr.ifr_ifru.ifru_ivalue);

    //WEIGHT_WLAN_LOW,          /* <11Mb WLAN     */
    //WEIGHT_WLAN_11MB,         /* 11Mb 802.11b   */
    //WEIGHT_WLAN_54MB,         /* 54Mb 802.11g   */
    return WEIGHT_WLAN_LOW;
  } else {
    /* Ethernet */
    /* Mii wizardry */
    struct ifreq ifr;
    struct mii_data *mii = (struct mii_data *)&ifr.ifr_data;
    int bmcr;
    memset(&ifr, 0, sizeof(ifr));
    strscpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));

    if (ioctl(olsr_cnf->ioctl_s, SIOCGMIIPHY, &ifr) < 0) {
      if (errno != ENODEV)
        OLSR_PRINTF(1, "SIOCGMIIPHY on '%s' failed: %s\n", ifr.ifr_name, strerror(errno));
      return WEIGHT_ETHERNET_DEFAULT;
    }

    mii->reg_num = MII_BMCR;
    if (ioctl(olsr_cnf->ioctl_s, SIOCGMIIREG, &ifr) < 0) {
      OLSR_PRINTF(1, "SIOCGMIIREG on %s failed: %s\n", ifr.ifr_name, strerror(errno));
      return WEIGHT_ETHERNET_DEFAULT;
    }
    bmcr = mii->val_out;

    OLSR_PRINTF(1, "%s: ", ifr.ifr_name);
    OLSR_PRINTF(1, "%s Mbit, %s duplex\n", (bmcr & MII_BMCR_100MBIT) ? "100" : "10", (bmcr & MII_BMCR_DUPLEX) ? "full" : "half");

    is_if_link_up(ifname);

    if (mii->val_out & MII_BMCR_100MBIT)
      return WEIGHT_ETHERNET_100MB;
    else
      return WEIGHT_ETHERNET_10MB;
    //WEIGHT_ETHERNET_1GB,      /* Ethernet 1Gb   */

  }
}

bool
is_if_link_up(char *ifname)
{
  if (check_wireless_interface(ifname)) {
    /* No link checking on wireless devices */
    return true;
  } else {
    /* Mii wizardry */
    struct ifreq ifr;
    struct mii_data *mii = (struct mii_data *)&ifr.ifr_data;
    int bmsr;
    memset(&ifr, 0, sizeof(ifr));
    strscpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));

    if (ioctl(olsr_cnf->ioctl_s, SIOCGMIIPHY, &ifr) < 0) {
      if (errno != ENODEV)
        OLSR_PRINTF(1, "SIOCGMIIPHY on '%s' failed: %s\n", ifr.ifr_name, strerror(errno));
      return WEIGHT_ETHERNET_DEFAULT;
    }
    mii->reg_num = MII_BMSR;
    if (ioctl(olsr_cnf->ioctl_s, SIOCGMIIREG, &ifr) < 0) {
      OLSR_PRINTF(1, "SIOCGMIIREG on %s failed: %s\n", ifr.ifr_name, strerror(errno));
      return WEIGHT_ETHERNET_DEFAULT;
    }
    bmsr = mii->val_out;

    OLSR_PRINTF(1, "%s: ", ifr.ifr_name);
    OLSR_PRINTF(1, "%s\n", (bmsr & MII_BMSR_LINK_VALID) ? "link ok " : "no link ");

    return (bmsr & MII_BMSR_LINK_VALID);

  }
}

#else
int
calculate_if_metric(char *ifname)
{
  return check_wireless_interface(ifname);
}
#endif

bool olsr_if_isup(const char * dev)
{
  struct ifreq ifr;

  memset(&ifr, 0, sizeof(ifr));
  strscpy(ifr.ifr_name, dev, IFNAMSIZ);

  if (ioctl(olsr_cnf->ioctl_s, SIOCGIFFLAGS, &ifr) < 0) {
    OLSR_PRINTF(1, "ioctl SIOCGIFFLAGS (get flags) error on device %s: %s (%d)\n",
        dev, strerror(errno), errno);
    return 1;
  }
  return (ifr.ifr_flags & IFF_UP) != 0;
}

int olsr_if_set_state(const char *dev, bool up) {
  int oldflags;
  struct ifreq ifr;

  memset(&ifr, 0, sizeof(ifr));
  strscpy(ifr.ifr_name, dev, IFNAMSIZ);

  if (ioctl(olsr_cnf->ioctl_s, SIOCGIFFLAGS, &ifr) < 0) {
    OLSR_PRINTF(1, "ioctl SIOCGIFFLAGS (get flags) error on device %s: %s (%d)\n",
        dev, strerror(errno), errno);
    return 1;
  }

  oldflags = ifr.ifr_flags;
  if (up) {
    ifr.ifr_flags |= IFF_UP;
  }
  else {
    ifr.ifr_flags &= ~IFF_UP;
  }

  if (oldflags == ifr.ifr_flags) {
    /* interface is already up/down */
    return 0;
  }

  if (ioctl(olsr_cnf->ioctl_s, SIOCSIFFLAGS, &ifr) < 0) {
    OLSR_PRINTF(1, "ioctl SIOCSIFFLAGS (set flags %s) error on device %s: %s (%d)\n",
        up ? "up" : "down", dev, strerror(errno), errno);
    return 1;
  }
  return 0;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
