/* -*- mode: c; c-basic-offset: 2 -*- */
/*
 * Copyright (C) 2003, 2004, 2005 Mondru AB.
 * Copyright (C) 2007-2013 David Bird (Coova Technologies) <support@coova.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "chilli.h"
#ifdef ENABLE_MODULES
#include "chilli_module.h"
#endif

#ifdef USING_MMAP
#include <sys/mman.h>
#include <linux/filter.h>
int tx_ring_bug = 1;
static int tx_ring(net_interface *iface, void *packet, size_t length);
static int rx_ring(net_interface *iface, net_handler func, void *ctx);
static void set_buffer(net_interface *iface, int what, int size);
static void setup_rings(net_interface *iface, unsigned size, int mtu);
static void setup_rings2(net_interface *iface);
static void destroy_one_ring(net_interface *iface, int what);
//static void setup_filter(net_interface *iface);
#endif

static int default_sndbuf = 0;
static int default_rcvbuf = 0;

int dev_set_flags(char const *dev, int flags) {
  struct ifreq ifr;
  int fd;

  memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_flags = flags;
  strlcpy(ifr.ifr_name, dev, IFNAMSIZ);

  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    syslog(LOG_ERR, "%s: socket() failed", strerror(errno));
    return -1;
  }

  if (ioctl(fd, SIOCSIFFLAGS, &ifr)) {
    syslog(LOG_ERR, "%s: ioctl(SIOCSIFFLAGS) failed", strerror(errno));
    close(fd);
    return -1;
  }

  close(fd);

  return 0;
}

int dev_get_flags(char const *dev, int *flags) {
  struct ifreq ifr;
  int fd;

  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name, dev, IFNAMSIZ);

  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    syslog(LOG_ERR, "%s: socket() failed", strerror(errno));
    return -1;
  }

  if (ioctl(fd, SIOCGIFFLAGS, &ifr)) {
    syslog(LOG_ERR, "%s: ioctl(SIOCSIFFLAGS) failed on %s", strerror(errno), dev);
    close(fd);
    return -1;
  }

  close(fd);

  *flags = ifr.ifr_flags;

  return 0;
}

int dev_set_address(char const *devname, struct in_addr *address,
		    struct in_addr *dstaddr, struct in_addr *netmask) {
#if defined(__linux__)
  struct ifreq ifr;
#elif defined(__FreeBSD__) || defined (__APPLE__) || defined (__OpenBSD__) || defined (__NetBSD__)
  struct ifaliasreq ifr;
#endif
  int fd;

  memset(&ifr, 0, sizeof (ifr));

  /* Create a channel to the NET kernel. */
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    syslog(LOG_ERR, "%s: socket() failed", strerror(errno));
    return -1;
  }

#if defined(__linux__)
  strlcpy(ifr.ifr_name, devname, IFNAMSIZ);
  ifr.ifr_addr.sa_family = AF_INET;
  ifr.ifr_dstaddr.sa_family = AF_INET;
  ifr.ifr_netmask.sa_family = AF_INET;
  ifr.ifr_name[IFNAMSIZ-1] = 0; /* Make sure to terminate */

  if (address) { /* Set the interface address */
    ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr.s_addr = address->s_addr;
    if (ioctl(fd, SIOCSIFADDR, (void *) &ifr) < 0) {
      if (errno != EEXIST) {
	syslog(LOG_ERR, "%s: ioctl(SIOCSIFADDR) failed", strerror(errno));
      }
      else {
	syslog(LOG_WARNING, "%d ioctl(SIOCSIFADDR): Address already exists",
               errno);
      }
      close(fd);
      return -1;
    }
  }

  if (dstaddr) { /* Set the destination address */
    ((struct sockaddr_in *) &ifr.ifr_dstaddr)->sin_addr.s_addr = dstaddr->s_addr;
    if (ioctl(fd, SIOCSIFDSTADDR, (caddr_t) &ifr) < 0) {
      syslog(LOG_ERR, "%s: ioctl(SIOCSIFDSTADDR) failed", strerror(errno));
      close(fd);
      return -1;
    }
  }

  if (netmask) { /* Set the netmask */
    ((struct sockaddr_in *) &ifr.ifr_netmask)->sin_addr.s_addr =  netmask->s_addr;
    if (ioctl(fd, SIOCSIFNETMASK, (void *) &ifr) < 0) {
      syslog(LOG_ERR, "%s: ioctl(SIOCSIFNETMASK) failed", strerror(errno));
      close(fd);
      return -1;
    }
  }


#elif defined(__FreeBSD__) || defined (__APPLE__) || defined (__OpenBSD__) || defined (__NetBSD__)
  strncpy(ifr.ifra_name, devname, IFNAMSIZ);
  ifr.ifra_name[IFNAMSIZ-1] = 0; /* Make sure to terminate */

  ((struct sockaddr_in*) &ifr.ifra_addr)->sin_family = AF_INET;
  ((struct sockaddr_in*) &ifr.ifra_addr)->sin_len = sizeof(ifr.ifra_addr);
  ((struct sockaddr_in*) &ifr.ifra_addr)->sin_addr.s_addr = address->s_addr;

  ((struct sockaddr_in*) &ifr.ifra_mask)->sin_family = AF_INET;
  ((struct sockaddr_in*) &ifr.ifra_mask)->sin_len    = sizeof(ifr.ifra_mask);
  ((struct sockaddr_in*) &ifr.ifra_mask)->sin_addr.s_addr = netmask->s_addr;

  /* For some reason FreeBSD uses ifra_broadcast for specifying dstaddr */
  ((struct sockaddr_in*) &ifr.ifra_broadaddr)->sin_family = AF_INET;
  ((struct sockaddr_in*) &ifr.ifra_broadaddr)->sin_len = sizeof(ifr.ifra_broadaddr);
  ((struct sockaddr_in*) &ifr.ifra_broadaddr)->sin_addr.s_addr = dstaddr->s_addr;

  if (ioctl(fd, SIOCAIFADDR, (void *) &ifr) < 0) {
    if (errno != EEXIST) {
      syslog(LOG_ERR, "%s: ioctl(SIOCAIFADDR) failed", strerror(errno));
    }
    else {
      syslog(LOG_WARNING, "%d ioctl(SIOCAIFADDR): Address already exists",
             errno);
    }
    close(fd);
    return -1;
  }
#else
#error  "Unknown platform!"
#endif

  close(fd);

  return dev_set_flags(devname, IFF_UP | IFF_RUNNING);
}

int net_init(net_interface *netif, char *ifname,
	     uint16_t protocol, int promisc, uint8_t *mac) {

  if (ifname) {
    memset(netif, 0, sizeof(net_interface));
    strlcpy(netif->devname, ifname, IFNAMSIZ);
  }

  netif->protocol = protocol;

  if (promisc) {
    netif->flags |= NET_PROMISC;
  }

  if (mac) {
    netif->flags |= NET_USEMAC;
    memcpy(netif->hwaddr, mac, PKT_ETH_ALEN);
  }

  return net_open(netif);
}

int net_open(net_interface *netif) {
  net_close(netif);
  net_gflags(netif);

  if (
#ifdef ENABLE_LAYER3
          !_options.layer3 &&
#endif
#ifdef HAVE_NETFILTER_COOVA
          (_options.uamlisten.s_addr == _options.dhcplisten.s_addr) && 
#endif
          ( !(netif->devflags & IFF_UP) || !(netif->devflags & IFF_RUNNING) )) {
    struct in_addr noaddr;
    net_sflags(netif, netif->devflags | IFF_NOARP);
    memset(&noaddr, 0, sizeof(noaddr));
    if (_options.debug)
      syslog(LOG_DEBUG, "removing ip address from %s", netif->devname);
    dev_set_address(netif->devname, &noaddr, NULL, NULL);
  }

  return net_open_eth(netif);
}

static int net_setsockopt(int s, int l, int op, void *v, socklen_t vl) {

  if (setsockopt(s, l, op, v, vl) < 0) {
    syslog(LOG_ERR, "%d setsockopt(s=%d, level=%d, optname=%d, optlen=%d) failed",
           errno, s, l, op, (int) vl);
    return -1;
  }

  return 0;
}

int net_reopen(net_interface *netif) {
  int previous_fd = netif->fd;
  int option;
  socklen_t len;

  if (_options.debug)
    syslog(LOG_DEBUG, "net_reopen(%s)", netif->devname);

  net_open(netif);

  option = (int)(default_sndbuf * 1.1);
  net_setsockopt(netif->fd, SOL_SOCKET, SO_SNDBUF, &option, sizeof(option));

  option = (int)(default_rcvbuf * 1.1);
  net_setsockopt(netif->fd, SOL_SOCKET, SO_RCVBUF, &option, sizeof(option));

  len = sizeof(default_sndbuf);
  getsockopt(netif->fd, SOL_SOCKET, SO_SNDBUF, &default_sndbuf, &len);
  if (_options.debug)
    syslog(LOG_DEBUG, "Net SNDBUF %d", default_sndbuf);

  len = sizeof(default_sndbuf);
  getsockopt(netif->fd, SOL_SOCKET, SO_RCVBUF, &default_rcvbuf, &len);
  if (_options.debug)
    syslog(LOG_DEBUG, "Net RCVBUF %d", default_rcvbuf);

  if (netif->sctx)
    net_select_rereg(netif->sctx, previous_fd, netif->fd);

  return 0;
}

int net_close(net_interface *netif) {
#ifdef USING_PCAP
  if (netif->pd) pcap_close(netif->pd);
  netif->pd = 0;
#endif
  if (netif->fd) close(netif->fd);
  netif->fd = 0;
  return 0;
}

int net_select_init(select_ctx *sctx) {
#if defined(USING_POLL) && defined(HAVE_SYS_EPOLL_H)
  sctx->efd = epoll_create(MAX_SELECT);
  if (sctx->efd <= 0) {
    syslog(LOG_ERR, "%s: !! could not create epoll !!", strerror(errno));
    return -1;
  }
#endif
  return 0;
}

int net_select_prepare(select_ctx *sctx) {

#ifdef ENABLE_MODULES
  {
    int i;
    for (i=0; i < MAX_MODULES; i++) {
      if (!_options.modules[i].name[0]) break;
      if (_options.modules[i].ctx) {
	struct chilli_module *m =
            (struct chilli_module *)_options.modules[i].ctx;
	if (m->net_select)
	  m->net_select(sctx);
      }
    }
  }
#endif

#if defined(USING_POLL) && defined(HAVE_SYS_EPOLL_H)
  /* nothing */
#else
#ifdef USING_POLL
  {
    int i;
    for (i=0; i < sctx->count; i++) {
      if (sctx->desc[i].fd) {
	sctx->pfds[i].fd = sctx->desc[i].fd;
	sctx->pfds[i].events = 0;
	if (sctx->desc[i].evts & SELECT_READ)
	  sctx->pfds[i].events |= POLLIN;
	if (sctx->desc[i].evts & SELECT_WRITE)
	  sctx->pfds[i].events |= POLLOUT;
      }
    }
  }
#else
  fd_zero(&sctx->rfds);
  fd_zero(&sctx->wfds);
  fd_zero(&sctx->efds);
  {
    int i;
    for (i=0; i < sctx->count; i++) {
      if (sctx->desc[i].fd) {
	if (sctx->desc[i].evts & SELECT_READ) {
	  fd_set(sctx->desc[i].fd, &sctx->rfds);
	  fd_set(sctx->desc[i].fd, &sctx->efds);
	}
	if (sctx->desc[i].evts & SELECT_WRITE)
	  fd_set(sctx->desc[i].fd, &sctx->wfds);
      } else if (sctx->desc[i].evts & SELECT_RESET) {
	sctx->desc[i].cb(&sctx->desc[i], -1);
      }
    }
  }
#endif
#endif
  return 0;
}

int net_select_dereg(select_ctx *sctx, int oldfd) {
  int i;
  for (i=0; i < sctx->count; i++) {
    if (sctx->desc[i].fd == oldfd) {
      for (; i < sctx->count - 1; i++)
	memcpy(&sctx->desc[i], &sctx->desc[i+1], sizeof(select_fd));
      memset(&sctx->desc[i], 0, sizeof(select_fd));
      sctx->count--;
      return 0;
    }
  }
  return -1;
}

int net_select_rereg(select_ctx *sctx, int oldfd, int newfd) {
  int i;
  for (i=0; i < sctx->count; i++) {
    if (sctx->desc[i].fd == oldfd) {
      sctx->desc[i].fd = newfd;
#if defined(USING_POLL) && defined(HAVE_SYS_EPOLL_H)
      {
	struct epoll_event event;

	memset(&event, 0, sizeof(event));
	event.data.fd = oldfd;
	event.events = EPOLLIN | EPOLLOUT;
	if (epoll_ctl(sctx->efd, EPOLL_CTL_DEL, oldfd, &event))
	  syslog(LOG_ERR, "%s: epoll fd %d not found", strerror(errno), oldfd);

	memset(&event, 0, sizeof(event));
	if (sctx->desc[i].evts & SELECT_READ) event.events |= EPOLLIN;
	if (sctx->desc[i].evts & SELECT_WRITE) event.events |= EPOLLOUT;
	event.data.ptr = &sctx->desc[i];
	if (epoll_ctl(sctx->efd, EPOLL_CTL_ADD, newfd, &event))
	  syslog(LOG_ERR, "%s: Failed to watch fd", strerror(errno));
      }
#endif
      return 0;
    }
  }
  return -1;
}

int net_select_reg(select_ctx *sctx, int fd, char evts,
		   select_callback cb, void *ctx, int idx) {
  if (!evts) return -3;
  if (fd <= 0) return -2;
  if (sctx->count == MAX_SELECT) return -1;
  sctx->desc[sctx->count].fd = fd;
  sctx->desc[sctx->count].cb = cb;
  sctx->desc[sctx->count].ctx = ctx;
  sctx->desc[sctx->count].idx = idx;
  sctx->desc[sctx->count].evts = evts;
#ifdef USING_POLL
#ifdef HAVE_SYS_EPOLL_H
  {
    struct epoll_event event;

    memset(&event, 0, sizeof(event));
    event.events = 0;
    if (evts & SELECT_READ) event.events |= EPOLLIN;
    if (evts & SELECT_WRITE) event.events |= EPOLLOUT;
    event.data.ptr = &sctx->desc[sctx->count];
    if (epoll_ctl(sctx->efd, EPOLL_CTL_ADD, fd, &event))
      syslog(LOG_ERR, "%s: Failed to watch fd", strerror(errno));
  }
#endif
#else
  if (fd > sctx->maxfd) sctx->maxfd = fd;
#endif
  sctx->count++;
  if (_options.debug)
    syslog(LOG_DEBUG, "net select count: %d", sctx->count);
  return 0;
}

int net_select_zero(select_ctx *sctx) {
#if defined(USING_POLL) && defined(HAVE_SYS_EPOLL_H)
  memset(&sctx->events, 0, sizeof(sctx->events));
#else
#ifdef USING_POLL
  int i;
  memset(&sctx->pfds, 0, sizeof(sctx->pfds));
  for (i=0; i < sctx->count; i++) {
    if (sctx->desc[i].fd) {
      sctx->pfds[i].fd = sctx->desc[i].fd;
      sctx->pfds[i].events = 0;
      if (sctx->desc[i].evts & SELECT_READ)
	sctx->pfds[i].events |= POLLIN;
      if (sctx->desc[i].evts & SELECT_WRITE)
	sctx->pfds[i].events |= POLLOUT;
    }
  }
#else
  fd_zero(&sctx->rfds);
  fd_zero(&sctx->wfds);
  fd_zero(&sctx->efds);
#endif
#endif
  sctx->count = 0;
  return 0;
}

int net_select_rmfd(select_ctx *sctx, int fd) {
#if defined(USING_POLL) && defined(HAVE_SYS_EPOLL_H)
  struct epoll_event event;
  memset(&event, 0, sizeof(event));
  event.data.fd = fd;
  if (_options.debug)
    syslog(LOG_DEBUG, "epoll rm %d", fd);

  if (epoll_ctl(sctx->efd, EPOLL_CTL_DEL, fd, &event))
    syslog(LOG_ERR, "%d Failed to remove fd %d (%d)",
           errno, fd, sctx->efd);
#endif
  return 0;
}

int net_select_addfd(select_ctx *sctx, int fd, int evts) {
#if defined(USING_POLL) && defined(HAVE_SYS_EPOLL_H)
  struct epoll_event event;
  memset(&event, 0, sizeof(event));
  event.data.fd = fd;
  if (evts & SELECT_READ) event.events |= EPOLLIN;
  if (evts & SELECT_WRITE) event.events |= EPOLLOUT;
  if (_options.debug)
    syslog(LOG_DEBUG, "epoll add %d (%d)", fd, sctx->efd);
  /*
   */
  if (epoll_ctl(sctx->efd, EPOLL_CTL_ADD, fd, &event))
    syslog(LOG_ERR, "%d Failed to add fd %d (%d)",
           errno, fd, sctx->efd);
#endif
  return 0;
}

int net_select_modfd(select_ctx *sctx, int fd, int evts) {
#if defined(USING_POLL) && defined(HAVE_SYS_EPOLL_H)
  struct epoll_event event;
  memset(&event, 0, sizeof(event));
  event.data.fd = fd;
  if (evts & SELECT_READ) event.events |= EPOLLIN;
  if (evts & SELECT_WRITE) event.events |= EPOLLOUT;
  /*syslog(LOG_DEBUG, "epoll mod %d", fd);*/
  if (epoll_ctl(sctx->efd, EPOLL_CTL_MOD, fd, &event))
    syslog(LOG_ERR, "%s: Failed to watch fd", strerror(errno));
#endif
  return 0;
}

int net_select_fd(select_ctx *sctx, int fd, char evts) {
  if (!evts) return -3;
  if (fd <= 0) return -2;
#ifdef USING_POLL
#ifdef HAVE_SYS_EPOLL_H
#else
  sctx->pfds[sctx->count].fd = fd;
  sctx->pfds[sctx->count].events = 0;
  if (evts & SELECT_READ)
    sctx->pfds[sctx->count].events |= POLLIN;
  if (evts & SELECT_WRITE)
    sctx->pfds[sctx->count].events |= POLLOUT;
#endif
#else
  if (evts & SELECT_READ) {
    fd_set(fd, &sctx->rfds);
    fd_set(fd, &sctx->efds);
  }
  if (evts & SELECT_WRITE) {
    fd_set(fd, &sctx->wfds);
  }
  if (fd > sctx->maxfd) {
    sctx->maxfd = fd;
  }
#endif
  sctx->count++;
  return 0;
}

int net_select(select_ctx *sctx) {
  int status;

  do {

#ifdef USING_POLL
#ifdef HAVE_SYS_EPOLL_H
    status = epoll_wait(sctx->efd, sctx->events, MAX_SELECT, 1000);
#else
    status = poll(sctx->pfds, sctx->count, 1000);
#endif
#else
    sctx->idleTime.tv_sec = 1;
    sctx->idleTime.tv_usec = 0;

    status = select(sctx->maxfd + 1,
		    &sctx->rfds,
		    &sctx->wfds,
		    &sctx->efds,
		    &sctx->idleTime);

#if(0)
    if (status) syslog(LOG_DEBUG, "select() == %d", status);
    {int i;
      for (i=0; i<FD_SETSIZE; i++)
	if (FD_ISSET(i, &sctx->rfds))
	  syslog(LOG_DEBUG, "rfds[%d]",i);
      for (i=0; i<FD_SETSIZE; i++)
	if (FD_ISSET(i, &sctx->wfds))
	  syslog(LOG_DEBUG, "wfds[%d]",i);
      for (i=0; i<FD_SETSIZE; i++)
	if (FD_ISSET(i, &sctx->efds))
	  syslog(LOG_DEBUG, "efds[%d]",i);
    }
#endif

    if (status == -1) net_select_prepare(sctx); /* reset */
#endif
  } while (status == -1 && errno == EINTR);
  return status;
}

int net_select_read_fd(select_ctx *sctx, int fd) {
#ifdef USING_POLL
  int idx;
#ifdef HAVE_SYS_EPOLL_H
  for (idx=0; idx < sctx->count; idx++)
    if (sctx->events[idx].data.fd == fd) {
      if (sctx->events[idx].events & EPOLLIN)
	return 1;
    }
#else
  for (idx=0; idx < sctx->count; idx++)
    if (sctx->pfds[idx].fd == fd)
      if (sctx->pfds[idx].events & POLLIN)
	return 1;
#endif
#else
  if (FD_ISSET(fd, &sctx->efds))
    return -1;
  if (FD_ISSET(fd, &sctx->rfds))
    return 1;
#endif
  return 0;
}

int net_select_write_fd(select_ctx *sctx, int fd) {
#ifdef USING_POLL
  int idx;
#ifdef HAVE_SYS_EPOLL_H
  for (idx=0; idx < sctx->count; idx++)
    if (sctx->events[idx].data.fd == fd) {
#if(_debug_ > 1)
      if (_options.debug)
        syslog(LOG_DEBUG, "write %d", (sctx->events[idx].events & EPOLLOUT) != 0);
#endif
      if (sctx->events[idx].events & EPOLLOUT)
	return 1;
    }
#else
  for (idx=0; idx < sctx->count; idx++)
    if (sctx->pfds[idx].fd == fd) {
      if (sctx->pfds[idx].events & POLLOUT)
	return 1;
    }
#endif
#else
  if (FD_ISSET(fd, &sctx->efds))
    return -1;
  if (FD_ISSET(fd, &sctx->wfds))
    return 1;
#endif
  return 0;
}

int net_run_selected(select_ctx *sctx, int status) {
  int i;
#if defined(USING_POLL) && defined(HAVE_SYS_EPOLL_H)
  for (i=0; i < status; i++) {
    select_fd *sfd = (select_fd *)sctx->events[i].data.ptr;
    sfd->cb(sfd->ctx, sfd->idx);
  }
#else
  for (i=0; i < sctx->count; i++) {
    if (sctx->desc[i].fd) {
#ifdef USING_POLL
      char has_read = !!(sctx->pfds[i].revents & POLLIN);
#else
      char has_read = fd_isset(sctx->desc[i].fd, &sctx->rfds);
#endif
      if (has_read) {
	sctx->desc[i].cb(sctx->desc[i].ctx, sctx->desc[i].idx);
      }
    }
  }
#endif
  return 0;
}

int net_set_address(net_interface *netif, struct in_addr *address,
		    struct in_addr *dstaddr, struct in_addr *netmask) {
  netif->address.s_addr = address->s_addr;
  netif->gateway.s_addr = dstaddr->s_addr;
  netif->netmask.s_addr = netmask->s_addr;

  return dev_set_address(netif->devname, address, dstaddr, netmask);
}

#if defined(USING_PCAP)
struct netpcap {
  void *d;
  size_t dlen;
  size_t read;
  net_handler func;
};

static void
net_pcap_read(u_char *user, const struct pcap_pkthdr *hdr,
	      const u_char *bytes) {
  struct netpcap *np = (struct netpcap *)user;
  if (!bytes || hdr->caplen < sizeof(struct pkt_ethhdr_t) ||
      hdr->caplen > np->dlen)
    np->read = -1;
  else {
    memcpy(np->d, bytes, hdr->caplen);
    np->read = hdr->caplen;
  }
}

static void
net_pcap_handler(u_char *user, const struct pcap_pkthdr *hdr,
		 const u_char *bytes) {
  struct netpcap *np = (struct netpcap *)user;
  if (!bytes || hdr->caplen < sizeof(struct pkt_ethhdr_t))
    np->read = -1;
  else {
    struct pkt_buffer pb;
    pkt_buffer_init(&pb, (uint8_t *)bytes, hdr->caplen, 0);
    np->read = np->func(np->d, &pb);
  }
}
#endif

ssize_t
net_read_dispatch_eth(net_interface *netif, net_handler func, void *ctx) {

#if defined(USING_PCAP)
  if (netif->pd) {
    struct netpcap np;
    int cnt;

    np.func = func;
    np.d = ctx;
    np.dlen = 0;
    np.read = 0;

    cnt = pcap_dispatch(netif->pd, 1, net_pcap_handler, (u_char *)&np);

    return cnt ? np.read : -1;
  }
#endif

#ifdef USING_MMAP
  if (netif->rx_ring.frames) {
    return rx_ring(netif, func, ctx);
  } else
#endif
  {
    struct pkt_buffer pb;
    uint8_t packet[PKT_MAX_LEN];
    ssize_t length;
    pkt_buffer_init(&pb, packet, sizeof(packet), PKT_BUFFER_IPOFF);
    length = net_read_eth(netif,
			  pkt_buffer_head(&pb),
			  pkt_buffer_size(&pb));
    if (length <= 0) return length;
    pb.length = length;
    return func(ctx, &pb);
  }
}

ssize_t
net_read_dispatch(net_interface *netif, net_handler func, void *ctx) {
  struct pkt_buffer pb;
  uint8_t packet[PKT_MAX_LEN];
  ssize_t length;
  pkt_buffer_init(&pb, packet, sizeof(packet), PKT_BUFFER_IPOFF);
  length = safe_read(netif->fd,
		     pkt_buffer_head(&pb),
		     pkt_buffer_size(&pb));
  if (length <= 0) return length;
  pb.length = length;
  return func(ctx, &pb);
}

ssize_t
net_read_eth(net_interface *netif, void *d, size_t dlen) {
  ssize_t len = 0;

#if defined(USING_PCAP)
  if (netif->pd) {
    struct netpcap np;
    int cnt;

    np.d = d;
    np.dlen = dlen;
    np.read = 0;

    cnt = pcap_dispatch(netif->pd, 1, net_pcap_read, (u_char *)&np);

    return cnt ? np.read : -1;
  }
#endif

#ifdef USING_MMAP
  if (netif->rx_ring.frames) {
    syslog(LOG_ERR, "shouldn't be reading a mmap'ed interface this way, use dispatch");
    return -1;
  } else
#endif

    if (netif->fd) {

#if defined(__linux__)
      struct sockaddr_ll s_addr;

#if defined(HAVE_LINUX_TPACKET_AUXDATA_TP_VLAN_TCI)
      struct iovec iov;
      struct msghdr msg;
      union {
        struct cmsghdr cmsg;
        char buf[CMSG_SPACE(sizeof(struct tpacket_auxdata))];
      } cmsg_buf;
#ifdef ENABLE_IEEE8021Q
      struct cmsghdr *cmsg;
      struct vlan_tag {
        u_int16_t tpid;
        u_int16_t tci;
      };
#endif
      msg.msg_name = &s_addr;
      msg.msg_namelen = sizeof(s_addr);
      msg.msg_iov = &iov;
      msg.msg_iovlen = 1;
      msg.msg_control = &cmsg_buf;
      msg.msg_controllen = sizeof(cmsg_buf);
      msg.msg_flags = 0;

      iov.iov_len = dlen;
      iov.iov_base = d;
#else
      int addr_len;
#endif

      memset (&s_addr, 0, sizeof (struct sockaddr_ll));

#if defined(HAVE_LINUX_TPACKET_AUXDATA_TP_VLAN_TCI)

      len = safe_recvmsg(netif->fd, &msg, MSG_TRUNC);

#else

      addr_len = sizeof (s_addr);

      len = safe_recvfrom(netif->fd, d, dlen,
                          MSG_DONTWAIT | MSG_TRUNC,
                          (struct sockaddr *) &s_addr,
                          (socklen_t *) &addr_len);

#endif
      if (len < 0) {

        syslog(LOG_ERR, "%s: could not read packet", strerror(errno));

      } else {

        if (len == 0) {
          if (_options.debug)
            syslog(LOG_DEBUG, "read zero, enable ieee8021q?");
        }

        if (len > dlen) {
          syslog(LOG_WARNING, "data truncated %zu/%zd, sending ICMP error",
		 len, dlen);
          return -1;
        }
      }

#elif defined (__FreeBSD__) || defined (__APPLE__) || defined (__OpenBSD__) || defined (__NetBSD__)

      len = safe_read(netif->fd, d, dlen);

#else

      len = safe_read(netif->fd, d, dlen);

#endif

      if (len < 0) {
        syslog(LOG_ERR, "%d net_read_eth(fd=%d, len=%zu, mtu=%d) == %zd",
               errno, netif->fd, dlen, netif->mtu, len);
        return -1;
      }

#if defined(HAVE_LINUX_TPACKET_AUXDATA_TP_VLAN_TCI) && defined(ENABLE_IEEE8021Q)
      if (_options.ieee8021q) {
        for (cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
          struct tpacket_auxdata *aux;
          struct vlan_tag *tag;
          unsigned int ulen;

          if (cmsg->cmsg_len < CMSG_LEN(sizeof(struct tpacket_auxdata)) ||
              cmsg->cmsg_level != SOL_PACKET ||
              cmsg->cmsg_type != PACKET_AUXDATA)
            continue;

          aux = (struct tpacket_auxdata *)CMSG_DATA(cmsg);
          if (aux->tp_vlan_tci == 0)
            continue;

          ulen = len > iov.iov_len ? iov.iov_len : len;

          if (ulen < 2 * PKT_ETH_ALEN ||
              len >= (dlen - 4)) {
            syslog(LOG_ERR, "bad pkt length to add 802.1q header %d/%zd",
                   ulen, len);
            break;
          }

#if(_debug_ > 1)
          if (_options.debug)
            syslog(LOG_DEBUG, "adding 8021q header from auxdata");
#endif

          memmove(d + (2 * PKT_ETH_ALEN) + 4,
                  d + (2 * PKT_ETH_ALEN),
                  len - (2 * PKT_ETH_ALEN));

          tag = (struct vlan_tag *)(d + 2 * PKT_ETH_ALEN);
          tag->tpid = htons(ETH_P_8021Q);
          tag->tci = htons(aux->tp_vlan_tci);
          len += 4;
        }
      }
#endif
    }

  return len;
}

ssize_t net_write(int sock, void *d, size_t dlen) {
  int left = dlen;
  int w, off = 0;

  while (left > 0) {
    w = safe_send(sock, d + off, left,
#ifdef MSG_NOSIGNAL
		  MSG_NOSIGNAL
#else
		  0
#endif
		  );
    if (w < 0) {
      syslog(LOG_ERR, "%s: safe_send(%d, d+%d,%d)", strerror(errno), sock, off, left);
      return (errno == EWOULDBLOCK || errno == EAGAIN) ? off : -1;
    }
    left -= w;
    off += w;
  }

  return off;
}

#if defined (__linux__)
ssize_t net_write_eth(net_interface *netif, void *d, size_t dlen, struct sockaddr_ll *dest) {
  int fd = netif->fd;
  ssize_t len;

#ifdef USING_MMAP
  if (netif->tx_ring.frames)
    return tx_ring(netif, d, dlen);
#endif

  len = safe_sendto(fd, d, dlen, 0,
		    (struct sockaddr *)dest,
		    sizeof(struct sockaddr_ll));

  if (len < 0) {
    switch (errno) {
      case EWOULDBLOCK:
        syslog(LOG_ERR, "%s: packet dropped due to congestion", strerror(errno));
        break;

#ifdef ENETDOWN
      case ENETDOWN:
        net_reopen(netif);
        break;
#endif
#ifdef ENXIO
      case ENXIO:
        net_reopen(netif);
        break;
#endif
#ifdef EMSGSIZE
      case EMSGSIZE:
        if (dlen > netif->mtu)
          net_set_mtu(netif, dlen);
        break;
#endif
    }

    syslog(LOG_ERR, "%s: net_write_eth(fd=%d, len=%zu) failed", strerror(errno), netif->fd, dlen);
    return -1;
  }

  return len;
}
#endif

int net_set_mtu(net_interface *netif, size_t mtu) {
#if !defined(USING_PCAP)
  struct ifreq ifr;
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0) return -1;
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name, netif->devname, sizeof(ifr.ifr_name));
  ifr.ifr_mtu = mtu;
  if (ioctl(fd, SIOCSIFMTU, &ifr) < 0) {
    syslog(LOG_ERR, "%d could not set MTU of %zu on dev=%s",
           errno, mtu, netif->devname);
    close(fd);
    return -1;
  }
  close(fd);
#endif
  return 0;
}

int net_route(struct in_addr *dst, struct in_addr *gateway,
	      struct in_addr *mask, int delete) {

  /* TODO: solaris!  */

#if defined(__linux__)
  struct rtentry r;
  int fd;

  memset (&r, 0, sizeof (r));
  r.rt_flags = RTF_UP | RTF_GATEWAY; /* RTF_HOST not set */

  /* Create a channel to the NET kernel. */
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    syslog(LOG_ERR, "%s: socket() failed", strerror(errno));
    return -1;
  }

  r.rt_dst.sa_family     = AF_INET;
  r.rt_gateway.sa_family = AF_INET;
  r.rt_genmask.sa_family = AF_INET;
  ((struct sockaddr_in *) &r.rt_dst)->sin_addr.s_addr = dst->s_addr;
  ((struct sockaddr_in *) &r.rt_gateway)->sin_addr.s_addr = gateway->s_addr;
  ((struct sockaddr_in *) &r.rt_genmask)->sin_addr.s_addr = mask->s_addr;

  if (delete) {
    if (ioctl(fd, SIOCDELRT, (void *) &r) < 0) {
      syslog(LOG_ERR, "%s: ioctl(SIOCDELRT) failed", strerror(errno));
      close(fd);
      return -1;
    }
  }
  else {
    if (ioctl(fd, SIOCADDRT, (void *) &r) < 0) {
      syslog(LOG_ERR, "%s: ioctl(SIOCADDRT) failed", strerror(errno));
      close(fd);
      return -1;
    }
  }
  close(fd);
  return 0;

#elif defined(__FreeBSD__) || defined (__APPLE__) || defined (__OpenBSD__) || defined (__NetBSD__)

  struct {
    struct rt_msghdr rt;
    struct sockaddr_in dst;
    struct sockaddr_in gate;
    struct sockaddr_in mask;
  } req;

  int fd;
  struct rt_msghdr *rtm;

  if ((fd = socket(AF_ROUTE, SOCK_RAW, 0)) == -1) {
    syslog(LOG_ERR, "%s: socket() failed", strerror(errno));
    return -1;
  }

  memset(&req, 0, sizeof(req));

  rtm  = &req.rt;

  rtm->rtm_msglen = sizeof(req);
  rtm->rtm_version = RTM_VERSION;
  if (delete) {
    rtm->rtm_type = RTM_DELETE;
  }
  else {
    rtm->rtm_type = RTM_ADD;
  }
  rtm->rtm_flags = RTF_UP | RTF_GATEWAY | RTF_STATIC;  /* TODO */
  rtm->rtm_addrs = RTA_DST | RTA_GATEWAY | RTA_NETMASK;
  rtm->rtm_pid = getpid();
  rtm->rtm_seq = 0044;                                 /* TODO */

  req.dst.sin_family       = AF_INET;
  req.dst.sin_len          = sizeof(req.dst);
  req.mask.sin_family      = AF_INET;
  req.mask.sin_len         = sizeof(req.mask);
  req.gate.sin_family      = AF_INET;
  req.gate.sin_len         = sizeof(req.gate);

  req.dst.sin_addr.s_addr  = dst->s_addr;
  req.mask.sin_addr.s_addr = mask->s_addr;
  req.gate.sin_addr.s_addr = gateway->s_addr;

  if (safe_write(fd, rtm, rtm->rtm_msglen) < 0) {
    syslog(LOG_ERR, "%s: write() failed", strerror(errno));
    close(fd);
    return -1;
  }
  close(fd);
  return 0;

#else
#error  "Unknown platform!"
#endif
}

int net_open_nfqueue(net_interface *netif, u_int16_t q, int (*cb)()) {
#ifdef HAVE_NETFILTER_QUEUE
  netif->h = nfq_open();

  if (_options.debug)
    syslog(LOG_DEBUG, "netif nfqueue %d", (int)q);

  if (!netif->h) {
    syslog(LOG_ERR, "%s: nfq_open() failed", strerror(errno));
    return -1;
  }

  if (nfq_unbind_pf(netif->h, AF_INET) < 0) {
    syslog(LOG_ERR, "%s: error during nfq_unbind_pf()", strerror(errno));
  }

  if (nfq_bind_pf(netif->h, AF_INET) < 0) {
    syslog(LOG_ERR, "%s: error during nfq_bind_pf()", strerror(errno));
    return -1;
  }

  netif->qh = nfq_create_queue(netif->h,  q, cb, NULL);
  if (!netif->qh) {
    syslog(LOG_ERR, "%s: error during nfq_create_queue(%d)", strerror(errno), (int)q);
    return -1;
  }

  if (nfq_set_mode(netif->qh, NFQNL_COPY_PACKET, 21 /*0xffff*/) < 0) {
    syslog(LOG_ERR, "%s: error during nfq_set_mode()", strerror(errno));
    return -1;
  }

  netif->fd = nfq_fd(netif->h);

  return 0;
#else
  syslog(LOG_ERR, "Not implemeneted; build with --with-nfqueue");
  return -1;
#endif
}

int net_getip(char *dev, struct in_addr *addr) {
  struct ifreq ifr;
  int ret = -1;
  int fd=socket(AF_INET, SOCK_DGRAM, 0);
  strlcpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name));
  if (ioctl(fd, SIOCGIFADDR, (caddr_t)&ifr) >= 0) {
    struct sockaddr *sa = (struct sockaddr *)&(ifr.ifr_addr);
    memcpy(addr, &((struct sockaddr_in *)sa)->sin_addr, sizeof(struct in_addr));
    ret = 0;
  }
  close(fd);
  return ret;
}

#ifdef ENABLE_IPV6
int net_getip6(char *dev, struct in6_addr *addr) {
  int ret = -1;
  struct ifaddrs *ifaddr, *ifa;
  int family;

  if (getifaddrs(&ifaddr) == 0) {
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
      if (ifa->ifa_addr) {
	family = ifa->ifa_addr->sa_family;
	if (family == AF_INET6 && !strcmp(ifa->ifa_name, dev)) {
	  struct sockaddr_in6 *sa = (struct sockaddr_in6 *) ifa->ifa_addr;
	  memcpy(addr, &((struct sockaddr_in6 *)sa)->sin6_addr, sizeof(struct in6_addr));
	  ret = 0;
	  break;
	}
      }
    }
    freeifaddrs(ifaddr);
  }
  return ret;
}
#endif

#if defined(USING_PCAP)

int net_open_eth(net_interface *netif) {
  struct ifreq ifr;
  char errbuf[PCAP_ERRBUF_SIZE];

  netif->pd = pcap_open_live(netif->devname, 2500, 1, 10, errbuf);

  syslog(LOG_INFO, "opening pcap device: %s", netif->devname);

  if (!netif->pd) {
    syslog(LOG_ERR, "%s: pcap: %s", strerror(errno), errbuf);
    return -1;
  }

  netif->fd = pcap_get_selectable_fd(netif->pd);

  memset(&ifr, 0, sizeof(ifr));

  /* Get ifindex */
  strlcpy(ifr.ifr_name, netif->devname, sizeof(ifr.ifr_name));
  if (ioctl(netif->fd, SIOCGIFINDEX, (caddr_t)&ifr) < 0) {
    syslog(LOG_ERR, "%s: ioctl(SIOCFIGINDEX) failed", strerror(errno));
  }

  netif->ifindex = ifr.ifr_ifindex;

  /* Get the MAC address of our interface */
  strlcpy(ifr.ifr_name, netif->devname, sizeof(ifr.ifr_name));
  if (ioctl(netif->fd, SIOCGIFHWADDR, (caddr_t)&ifr) < 0) {
    syslog(LOG_ERR, "%s: ioctl(d=%d, request=%d) failed", strerror(errno), netif->fd, SIOCGIFHWADDR);
    return -1;
  }

  if (ifr.ifr_hwaddr.sa_family == ARPHRD_ETHER) {
    netif->flags |= NET_ETHHDR;
    if ((netif->flags & NET_USEMAC) == 0) {
      memcpy(netif->hwaddr, ifr.ifr_hwaddr.sa_data, PKT_ETH_ALEN);
    } else if (_options.dhcpmacset) {
      strlcpy(ifr.ifr_name, netif->devname, sizeof(ifr.ifr_name));
      memcpy(ifr.ifr_hwaddr.sa_data, netif->hwaddr, PKT_ETH_ALEN);
      if (ioctl(netif->fd, SIOCSIFHWADDR, (caddr_t)&ifr) < 0) {
	syslog(LOG_ERR, "%s: ioctl(d=%d, request=%d) failed", strerror(errno), netif->fd, SIOCSIFHWADDR);
	return -1;
      }
    }
  }

#if defined(__linux__)
  memset(&netif->dest, 0, sizeof(netif->dest));
  netif->dest.sll_family = AF_PACKET;
  netif->dest.sll_protocol = htons(netif->protocol);
  netif->dest.sll_ifindex = netif->ifindex;
#endif

  return 0;
}

#elif defined(__linux__)

/**
 * Opens an Ethernet interface. As an option the interface can be set in
 * promisc mode. If not null macaddr and ifindex are filled with the
 * interface mac address and index
 **/
int net_open_eth(net_interface *netif) {
  struct ifreq ifr;
  struct sockaddr_ll sa;
  int option;

  memset(&ifr, 0, sizeof(ifr));

  /* Create socket */
  if ((netif->fd = socket(PF_PACKET,
			  /*XXX netif->idx ? SOCK_DGRAM : */SOCK_RAW,
			  htons(netif->protocol))) < 0) {
    if (errno == EPERM) {
      syslog(LOG_ERR, "%s: Cannot create raw socket. Must be root.", strerror(errno));
    }

    syslog(LOG_ERR, "%d socket(domain=%d, type=%d, protocol=%d) failed",
           errno, PF_PACKET, SOCK_RAW, netif->protocol);

    return -1;
  }

#ifdef USING_MMAP
  if (!_options.mmapring) {
#endif

    /* Let's make this non-blocking */
    ndelay_on(netif->fd);
    coe(netif->fd);

    /* Enable reception and transmission of broadcast frames */
    option = 1;
    if (net_setsockopt(netif->fd, SOL_SOCKET, SO_BROADCAST,
		       &option, sizeof(option)) < 0)
      return -1;

    if (_options.sndbuf > 0) {
      option = _options.sndbuf;
      net_setsockopt(netif->fd, SOL_SOCKET, SO_SNDBUF, &option, sizeof(option));
    }

    if (_options.rcvbuf > 0) {
      option = _options.rcvbuf;
      net_setsockopt(netif->fd, SOL_SOCKET, SO_RCVBUF, &option, sizeof(option));
    }

    {
      socklen_t len;
      len = sizeof(default_sndbuf);
      getsockopt(netif->fd, SOL_SOCKET, SO_SNDBUF, &default_sndbuf, &len);
      if (_options.debug)
        syslog(LOG_DEBUG, "Net SNDBUF %d", default_sndbuf);
      len = sizeof(default_sndbuf);
      getsockopt(netif->fd, SOL_SOCKET, SO_RCVBUF, &default_rcvbuf, &len);
      if (_options.debug)
        syslog(LOG_DEBUG, "Net RCVBUF %d", default_rcvbuf);
    }
#ifdef USING_MMAP
  }
#endif

  /* Get the MAC address of our interface */
  strlcpy(ifr.ifr_name, netif->devname, sizeof(ifr.ifr_name));
  if (ioctl(netif->fd, SIOCGIFHWADDR, (caddr_t)&ifr) < 0) {
    syslog(LOG_ERR, "%s: ioctl(d=%d, request=%d) failed", strerror(errno), netif->fd, SIOCGIFHWADDR);
    return -1;
  }

  if (ifr.ifr_hwaddr.sa_family == ARPHRD_ETHER) {
    netif->flags |= NET_ETHHDR;
    if ((netif->flags & NET_USEMAC) == 0) {
      memcpy(netif->hwaddr, ifr.ifr_hwaddr.sa_data, PKT_ETH_ALEN);
    } else if (_options.dhcpmacset) {
      strlcpy(ifr.ifr_name, netif->devname, sizeof(ifr.ifr_name));
      memcpy(ifr.ifr_hwaddr.sa_data, netif->hwaddr, PKT_ETH_ALEN);
      if (ioctl(netif->fd, SIOCSIFHWADDR, (caddr_t)&ifr) < 0) {
	syslog(LOG_ERR, "%s: ioctl(d=%d, request=%d) failed", strerror(errno), netif->fd, SIOCSIFHWADDR);
	return -1;
      }
    }
  }

  if (netif->hwaddr[0] & 0x01) {
    syslog(LOG_ERR, "Ethernet has broadcast or multicast address: %.16s",
           netif->devname);
  }

  /* Get the current interface address, network, and any destination address */
  /* Get the IP address of our interface */

  /* Verify that MTU = ETH_DATA_LEN */
  strlcpy(ifr.ifr_name, netif->devname, sizeof(ifr.ifr_name));
  if (ioctl(netif->fd, SIOCGIFMTU, (caddr_t)&ifr) < 0) {
    syslog(LOG_ERR, "%s: ioctl(d=%d, request=%d) failed", strerror(errno), netif->fd, SIOCGIFMTU);
    return -1;
  }
  if (ifr.ifr_mtu > PKT_BUFFER) {
    syslog(LOG_ERR, "MTU is larger than PKT_BUFFER: %d > %d",
           ifr.ifr_mtu, PKT_BUFFER);
    return -1;
  }
  netif->mtu = ifr.ifr_mtu;

  /* Get ifindex */
  strlcpy(ifr.ifr_name, netif->devname, sizeof(ifr.ifr_name));
  if (ioctl(netif->fd, SIOCGIFINDEX, (caddr_t)&ifr) < 0) {
    syslog(LOG_ERR, "%s: ioctl(SIOCFIGINDEX) failed", strerror(errno));
  }
  netif->ifindex = ifr.ifr_ifindex;

  if (_options.debug)
    syslog(LOG_DEBUG, "device %s ifindex %d", netif->devname, netif->ifindex);

#ifdef ENABLE_IPV6
  {
    struct ifaddrs *ifaddr, *ifa;
    char host[NI_MAXHOST];
    int family, s;
    if (getifaddrs(&ifaddr) == 0) {
      for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
	if (!ifa->ifa_addr) continue;
	family = ifa->ifa_addr->sa_family;

        if (_options.debug)
          syslog(LOG_DEBUG, "%s  address family: %d%s",
                 ifa->ifa_name, family,
                 (family == AF_PACKET) ? " (AF_PACKET)" :
                 (family == AF_INET) ?   " (AF_INET)" :
                 (family == AF_INET6) ?  " (AF_INET6)" : "");

	if (/*family == AF_INET || */family == AF_INET6 &&
	    !strcmp(netif->devname, ifa->ifa_name)) {
	  struct sockaddr_in6 *in6 = (struct sockaddr_in6 *)ifa->ifa_addr;
	  memcpy(&netif->address_v6, &in6->sin6_addr,
		 sizeof(struct in6_addr));
	  s = getnameinfo(ifa->ifa_addr,
			  /*(family == AF_INET) ?
			    sizeof(struct sockaddr_in) :*/
			  sizeof(struct sockaddr_in6),
			  host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
	  if (s != 0) {
            if (_options.debug)
              syslog(LOG_DEBUG, "getnameinfo() failed: %s\n", strerror(s));
	  } else {
            if (_options.debug)
              syslog(LOG_DEBUG, "address: <%s>\n", host);
	  }
	}
      }
      freeifaddrs(ifaddr);
    }
  }
#endif

#ifdef USING_MMAP
  if (_options.mmapring) {
    setup_rings(netif,
		_options.ringsize ? _options.ringsize : DEF_RING_SIZE,
		netif->mtu + 20);
  }
#endif

  /* Set interface in promisc mode */
  if (netif->flags & NET_PROMISC) {
    struct packet_mreq mr;

    memset(&ifr, 0, sizeof(ifr));
    strlcpy(ifr.ifr_name, netif->devname, sizeof(ifr.ifr_name));
    if (ioctl(netif->fd, SIOCGIFFLAGS, (caddr_t)&ifr) == -1) {
      syslog(LOG_ERR, "%s: ioctl(SIOCGIFFLAGS)", strerror(errno));
    } else {
      netif->devflags = ifr.ifr_flags;
      ifr.ifr_flags |= IFF_PROMISC;
      if (ioctl (netif->fd, SIOCSIFFLAGS, (caddr_t)&ifr) == -1) {
	syslog(LOG_ERR, "%s: Could not set flag IFF_PROMISC", strerror(errno));
      }
    }

    memset(&mr,0,sizeof(mr));
    mr.mr_ifindex = netif->ifindex;
    mr.mr_type = PACKET_MR_PROMISC;

    if (net_setsockopt(netif->fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP,
		       (char *)&mr, sizeof(mr)) < 0)
      return -1;
  }

  /* Bind to particular interface */
  memset(&sa, 0, sizeof(sa));
  sa.sll_family = AF_PACKET;
  sa.sll_protocol = htons(netif->protocol);
  sa.sll_ifindex = netif->ifindex;

  if (bind(netif->fd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
    syslog(LOG_ERR, "%s: bind(sockfd=%d) failed", strerror(errno), netif->fd);
    return -1;
  }

#if defined(__linux__)
  memset(&netif->dest, 0, sizeof(netif->dest));
  netif->dest.sll_family = AF_PACKET;
  netif->dest.sll_protocol = htons(netif->protocol);
  netif->dest.sll_ifindex = netif->ifindex;
#endif

#if defined(HAVE_LINUX_TPACKET_AUXDATA_TP_VLAN_TCI)
  option = 1;
  if (setsockopt(netif->fd, SOL_PACKET, PACKET_AUXDATA, &option,
		 sizeof(option)) == -1 && errno != ENOPROTOOPT) {
    syslog(LOG_ERR, "%s: auxdata", strerror(errno));
  }
#endif

#ifdef USING_MMAP
  if (_options.mmapring) {
    setup_rings2(netif);
    /*setup_filter(netif);*/

    if (_options.sndbuf > 0)
      set_buffer(netif, SO_SNDBUF, _options.sndbuf * 1024);

    if (_options.rcvbuf > 0)
      set_buffer(netif, SO_RCVBUF, _options.rcvbuf * 1024);
  }
#endif

  net_set_mtu(netif, _options.mtu);

  return 0;
}

#elif defined (__FreeBSD__) || defined (__APPLE__) || defined (__OpenBSD__) || defined (__NetBSD__)

int net_getmac(const char *ifname, char *macaddr) {

  struct ifaddrs *ifap, *ifa;
  struct sockaddr_dl *sdl;

  if (getifaddrs(&ifap)) {
    syslog(LOG_ERR, "%s: getifaddrs() failed!", strerror(errno));
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
      if (sdl->sdl_alen != PKT_ETH_ALEN) {
	syslog(LOG_ERR, "%s: Wrong sdl_alen!", strerror(errno));
	freeifaddrs(ifap);
	return -1;
      }
      memcpy(macaddr, LLADDR(sdl), PKT_ETH_ALEN);
      freeifaddrs(ifap);
      return 0;
    }
    ifa = ifa->ifa_next;
  }
  freeifaddrs(ifap);
  return -1;
}

/**
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

int net_open_eth(net_interface *netif) {
  char devname[IFNAMSIZ+5]; /* "/dev/" + ifname */
  int devnum;
  struct ifreq ifr;
  /*struct ifaliasreq areq;*/
  /*int local_fd;*/
  struct bpf_version bv;

  /*u_int32_t ipaddr;*/
  /*struct sockaddr_dl hwaddr;*/
  unsigned int value;

  /* Find suitable device */
  for (devnum = 0; devnum < 255; devnum++) { /* TODO 255 */
    snprintf(devname, sizeof(devname), "/dev/bpf%d", devnum);
    if ((netif->fd = open(devname, O_RDWR)) >= 0) break;
    if (errno != EBUSY) break;
  }
  if (netif->fd < 0) {
    syslog(LOG_ERR, "%s: Can't find bpf device", strerror(errno));
    return -1;
  }

  /* Set the interface */
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name, netif->devname, sizeof(ifr.ifr_name));
  if (ioctl(netif->fd, BIOCSETIF, &ifr) < 0) {
    syslog(LOG_ERR, "%s: ioctl() failed", strerror(errno));
    return -1;
  }

  /* Get and validate BPF version */
  if (ioctl(netif->fd, BIOCVERSION, &bv) < 0) {
    syslog(LOG_ERR, "%s: ioctl() failed!", strerror(errno));
    return -1;
  }
  if (bv.bv_major != BPF_MAJOR_VERSION ||
      bv.bv_minor < BPF_MINOR_VERSION) {
    syslog(LOG_ERR, "%s: wrong BPF version!", strerror(errno));
    return -1;
  }

  /* Get the MAC address of our interface */
  if (net_getmac(netif->devname, (char *)netif->hwaddr)) {
    syslog(LOG_ERR,"Did not find MAC address!");
  }
  else {
    netif->flags |= NET_ETHHDR;
  }

  if (netif->hwaddr[0] & 0x01) {
    syslog(LOG_ERR, "Ethernet has broadcast or multicast address: %.16s",
           netif->devname);
    return -1;
  }

  /* Set interface in promisc mode */
  if (netif->flags & NET_PROMISC) {
    value = 1;
    if (ioctl(netif->fd, BIOCPROMISC, NULL) < 0) {
      syslog(LOG_ERR, "%s: ioctl() failed!", strerror(errno));
      return -1;
    }
    value = 1;
    if (ioctl(netif->fd, BIOCSHDRCMPLT, &value) < 0) {
      syslog(LOG_ERR, "%s: ioctl() failed!", strerror(errno));
      return -1;
    }
  }
  else {
    value = 0;
    if (ioctl(netif->fd, BIOCSHDRCMPLT, &value) < 0) {
      syslog(LOG_ERR, "%s: ioctl() failed!", strerror(errno));
      return -1;
    }
  }

  /* Make sure reads return as soon as packet has been received */
  value = 1;
  if (ioctl(netif->fd, BIOCIMMEDIATE, &value) < 0) {
    syslog(LOG_ERR, "%s: ioctl() failed!", strerror(errno));
    return -1;
  }

  return 0;
}

#endif

void net_run(net_interface *iface) {
#ifdef USING_MMAP
  if (iface->is_active) {
    int ret = send(iface->fd, NULL, 0, MSG_DONTWAIT | MSG_NOSIGNAL);

    if (ret == -1 && errno != EAGAIN)
      syslog(LOG_ERR, "Async write error");
    else
      ++iface->stats.tx_runs;

    iface->is_active = 0;
  }
#endif
}

#ifdef USING_MMAP

static int rx_ring(net_interface *iface, net_handler func, void *ctx) {
  unsigned cnt, was_drop;
  struct tpacket2_hdr *h;
  /*  struct timespec tv;*/
  struct pkt_buffer pb;
  void *data;

  was_drop = 0;
  for (cnt = 0; cnt < iface->rx_ring.cnt; ++cnt) {
    data = h = iface->rx_ring.frames[iface->rx_ring.idx];

    if (!h->tp_status)
      break;

    if (++iface->rx_ring.idx >= iface->rx_ring.cnt)
      iface->rx_ring.idx = 0;

    if (h->tp_snaplen < (int)sizeof(struct pkt_ethhdr_t)) {
      syslog(LOG_ERR, "Packet too short");
      ++iface->stats.dropped;
      goto next;
    }

    /* Use the receiving time of the packet as the start time of
     * the request
     tv.tv_sec = h->tp_sec;
     tv.tv_nsec = h->tp_nsec;*/

    if (_options.debug > 100)
      syslog(LOG_DEBUG, "RX len=%d spanlen=%d (idx %d)", h->tp_len, h->tp_snaplen, iface->ifindex);

    pkt_buffer_init(&pb, (uint8_t *)data, h->tp_snaplen, h->tp_mac);
    pb.length = h->tp_len;

    func(ctx, &pb);

    was_drop |= h->tp_status & TP_STATUS_LOSING;

 next:
    h->tp_status = TP_STATUS_KERNEL;
  }

  if (cnt >= iface->rx_ring.cnt)
    ++iface->stats.rx_buffers_full;

  if (was_drop) {
    struct tpacket_stats stats;
    socklen_t len;

    len = sizeof(stats);
    if (!getsockopt(iface->fd, SOL_PACKET, PACKET_STATISTICS, &stats, &len))
      iface->stats.dropped += stats.tp_drops;

    if (_options.logfacility > 100)
      syslog(LOG_DEBUG, "RX drops %d", iface->stats.dropped);
  }

  ++iface->stats.rx_runs;

  return 1;
}

static int tx_ring(net_interface *iface, void *packet, size_t length) {
  struct tpacket2_hdr *h;
  unsigned cnt;
  void *data;

  /*int hdrlen = sizeofeth(packet);*/

  /* This may happen if the MTU changes while requests are
   * in flight */
#if (0)
  if (length > (unsigned)iface->mtu) {
    /*drop_request(q);*/
    syslog(LOG_ERR, "dropping packet len=%d", length);
    /*return -1;*/
  }
#endif

  for (cnt = 0; cnt < iface->tx_ring.cnt; ++cnt) {
    h = iface->tx_ring.frames[iface->tx_ring.idx++];
    if (iface->tx_ring.idx >= iface->tx_ring.cnt)
      iface->tx_ring.idx = 0;
    if (h->tp_status == TP_STATUS_AVAILABLE ||
	h->tp_status == TP_STATUS_WRONG_FORMAT)
      break;
  }
  if (cnt >= iface->tx_ring.cnt) {
    ++iface->stats.tx_buffers_full;
    /*g_ptr_array_add(iface->deferred, q);
      if (!iface->congested)
      {
      modify_fd(iface->fd, &iface->event_ctx, EPOLLIN | EPOLLOUT);
      iface->congested = TRUE;
      }
    */
    syslog(LOG_WARNING, "dropped packet, buffer full");
    return -1;
  }

  /* Should not happen */
  if (h->tp_status == TP_STATUS_WRONG_FORMAT)
    syslog(LOG_ERR, "Bad packet format on send");

  /* Fill the frame */
  data = (void *)h + iface->tp_hdrlen;
  memcpy(data, packet, length);
  h->tp_len = length;

  iface->stats.tx_bytes += h->tp_len;
  ++iface->stats.tx_cnt;

  h->tp_status = TP_STATUS_SEND_REQUEST;

  if (_options.debug > 100)
    syslog(LOG_DEBUG, "TX sent=%zu (idx %d)", length, iface->ifindex);

  if (!iface->is_active) {
    iface->is_active = 1;
  }

  return length;
}

static void setup_one_ring(net_interface *iface, unsigned ring_size, int mtu, int what) {
  unsigned page_size, max_blocks;
  struct tpacket_req req;
  struct ring *ring = NULL;
  const char *name = NULL;
  int ret = -1;
  memset(&req, 0, sizeof(struct tpacket_req));

  name = what == PACKET_RX_RING ? "RX" : "TX";
  ring = what == PACKET_RX_RING ? &iface->rx_ring : &iface->tx_ring;

  page_size = sysconf(_SC_PAGESIZE);

  if (_options.debug)
    syslog(LOG_DEBUG, "Creating %s ring: ring_size=%d; page_size=%d; mtu=%d",
           name, ring_size, page_size, mtu);

  /* For RX, the frame looks like:
   * - struct tpacket2_hdr
   * - padding to 16-byte boundary (this is included in iface->tp_hdrlen)
   * - padding: 16 - sizeof(struct ether_hdr)
   * - raw packet
   * - padding to 16-byte boundary
   *
   * So the raw packet is aligned so that the data part starts on a
   * 16-byte boundary, not the packet header. This means we need an extra
   * 16 bytes for the frame size.
   *
   * The TX frame is simpler:
   * - struct tpacket2_hdr
   * - padding to 16-byte boundary (this is included in iface->tp_hdrlen)
   * - raw packet
   * - padding to 16-byte boundary
   */
  ring->frame_size = TPACKET_ALIGN(iface->tp_hdrlen) + TPACKET_ALIGN(mtu);

  if (what == PACKET_RX_RING)
    ring->frame_size += TPACKET_ALIGNMENT;

  if (what == PACKET_TX_RING && tx_ring_bug) {
    unsigned maxsect;

    /* Kernel 2.6.31 has a bug and it requires a larger frame size
     * than what is in fact used */
    maxsect = (mtu - sizeof(struct pkt_ethhdr_t)) / 512;
    ring->frame_size = TPACKET_ALIGN(sizeof(struct pkt_ethhdr_t) +
				     maxsect * 512 + /*576 + 32*/1000);
  }

  req.tp_frame_size = ring->frame_size;

  /* The number of blocks is limited by the kernel implementation */
  max_blocks = page_size / sizeof(void *);

  /* Start with a large block size and if that fails try to lower it */
#ifdef ENABLE_LARGELIMITS
  req.tp_block_size = 64 * 1024;
#else
  req.tp_block_size = 4 * 1024;
#endif

  ret = -1;
  while (req.tp_block_size > req.tp_frame_size && req.tp_block_size >= page_size) {
    req.tp_block_nr = ring_size / req.tp_block_size;

    if (req.tp_block_nr > max_blocks)
      req.tp_block_nr = max_blocks;

    req.tp_frame_nr = (req.tp_block_size / req.tp_frame_size) * req.tp_block_nr;

    ret = net_setsockopt(iface->fd, SOL_PACKET, what, &req, sizeof(req));
    if (!ret)
      break;

    req.tp_block_size >>= 1;
  }
  if (ret) {
    syslog(LOG_ERR, "%d Failed to set up the %s ring buffer; "
           "block_sz=%d block_nr=%d frame_sz=%d frame_nr=%d page_size=%d", errno, name,
           req.tp_block_size, req.tp_block_nr, req.tp_frame_size, req.tp_frame_nr, page_size);
    memset(ring, 0, sizeof(*ring));
    return;
  }

  ring->len = req.tp_block_size * req.tp_block_nr;
  ring->block_size = req.tp_block_size;
  ring->cnt = req.tp_frame_nr;
  ring->frames = calloc(sizeof(void *), req.tp_frame_nr);

  syslog(LOG_INFO, "Created %s ring: len=%d; block size=%d; frame size=%d, cnt=%d",
         name, ring->len, req.tp_block_size, req.tp_frame_size, ring->cnt);
}

static void destroy_one_ring(net_interface *iface, int what) {
  struct tpacket_req req;
  struct ring *ring;

  ring = what == PACKET_RX_RING ? &iface->rx_ring : &iface->tx_ring;

  memset(&req, 0, sizeof(req));
  net_setsockopt(iface->fd, SOL_PACKET, what, &req, sizeof(req));
  free(ring->frames);
  memset(ring, 0, sizeof(*ring));
}

/* Set up pointers to the individual frames */
static void setup_frames(struct ring *ring, void *data) {
  unsigned i, j, cnt, blocks, frames;

  /* Number of blocks in the ring */
  blocks = ring->len / ring->block_size;
  /* Number of frames in a block */
  frames = ring->block_size / ring->frame_size;

  for (i = cnt = 0; i < blocks; i++)
    for (j = 0; j < frames; j++)
      ring->frames[cnt++] = data + i * ring->block_size + j * ring->frame_size;
}

/* Allocate and map the shared ring buffer */
static void setup_rings(net_interface *iface, unsigned size, int mtu) {
  socklen_t len;
  int ret, val;

  /* The function can be called on MTU change, so destroy the previous ring
   * if any */
  if (iface->ring_ptr) {
    munmap(iface->ring_ptr, iface->ring_len);
    iface->ring_ptr = NULL;
    iface->ring_len = 0;
    destroy_one_ring(iface, PACKET_RX_RING);
    destroy_one_ring(iface, PACKET_TX_RING);
  }

  if (!size)
    return;

  /* We want version 2 ring buffers to avoid 64-bit uncleanness */
  val = TPACKET_V2;
  ret = net_setsockopt(iface->fd, SOL_PACKET, PACKET_VERSION, &val, sizeof(val));

  if (ret) {
    syslog(LOG_ERR, "%s: Failed to set version 2 ring buffer format", strerror(errno));
    return;
  }

  val = TPACKET_V2;
  len = sizeof(val);
  ret = getsockopt(iface->fd, SOL_PACKET, PACKET_HDRLEN, &val, &len);

  if (ret) {
    syslog(LOG_ERR, "%s: Failed to determine the header length of the ring buffer", strerror(errno));
    return;
  }

  iface->tp_hdrlen = TPACKET_ALIGN(val);

  /* Drop badly formatted packets */
  val = 1;
  ret = net_setsockopt(iface->fd, SOL_PACKET, PACKET_LOSS, &val, sizeof(val));

  if (ret)
    syslog(LOG_ERR, "%s: Failed to set packet drop mode", strerror(errno));

  /* The RX and TX rings share the memory mapped area, so give
   * half the requested size to each */
  setup_one_ring(iface, size * 1024 / 2, mtu, PACKET_RX_RING);
  setup_one_ring(iface, size * 1024 / 2, mtu, PACKET_TX_RING);

  /* Both rings must be mapped using a single mmap() call */
  iface->ring_len = iface->rx_ring.len + iface->tx_ring.len;
}

static void setup_rings2(net_interface *iface) {
  socklen_t len;

  if (!iface->ring_len)
    return;

  iface->ring_ptr = mmap(NULL, iface->ring_len, PROT_READ | PROT_WRITE,
			 MAP_SHARED, iface->fd, 0);

  if (iface->ring_ptr == MAP_FAILED) {
    syslog(LOG_ERR, "%s: Failed to mmap the ring buffer", strerror(errno));
    destroy_one_ring(iface, PACKET_RX_RING);
    destroy_one_ring(iface, PACKET_TX_RING);
    iface->ring_ptr = NULL;
    iface->ring_len = 0;
    return;
  }

  len = 0;

  if (iface->rx_ring.len) {
    setup_frames(&iface->rx_ring, iface->ring_ptr);
    len = iface->rx_ring.len;
  }

  if (iface->tx_ring.len)
    setup_frames(&iface->tx_ring, iface->ring_ptr + len);

  /*len = human_format(iface->ring_len, &unit);*/
  syslog(LOG_INFO, "Set up ring buffer (%u RX/%u TX packets)",
         iface->rx_ring.cnt, iface->tx_ring.cnt);
}

/* Setting SO_SNDBUF/SO_RCVBUF is just advisory, so report the real value being
 * used */
static void set_buffer(net_interface *iface, int what, int size) {
  socklen_t len;
  int ret, val;

  ret = net_setsockopt(iface->fd, SOL_SOCKET, what, &size, sizeof(size));
  if (ret) {
    syslog(LOG_ERR, "%d Failed to set the %s buffer size",
           errno, what == SO_SNDBUF ? "send" : "receive");
    return;
  }

  len = sizeof(val);
  if (getsockopt(iface->fd, SOL_SOCKET, what, &val, &len))
    val = size;

  /*ret = human_format(val, &unit);
    syslog(LOG_INFO, "The %s buffer is %d %s",
    what == SO_SNDBUF ? "send" : "receive", ret, unit);*/
}

#if(0)
static void setup_filter(net_interface *iface) {

  if (iface->idx > 0) {

    static struct sock_filter filter[] = {

      { 0x28, 0, 0, 0x0000000c },
      { 0x15, 0, 6, 0x00000800 },
      { 0x20, 0, 0, 0x0000001e },
      { 0x54, 0, 0, 0xff000000 },
      { 0x15, 11, 0, 0x0a000000 },
      { 0x20, 0, 0, 0x0000001e },
      { 0x54, 0, 0, 0xff000000 },
      { 0x15, 8, 9, 0x65000000 },
      { 0x15, 1, 0, 0x00000806 },
      { 0x15, 0, 7, 0x00008035 },
      { 0x20, 0, 0, 0x00000026 },
      { 0x54, 0, 0, 0xff000000 },
      { 0x15, 3, 0, 0x0a000000 },
      { 0x20, 0, 0, 0x00000026 },
      { 0x54, 0, 0, 0xff000000 },
      { 0x15, 0, 1, 0x65000000 },
      { 0x6, 0, 0, 0x00000800 },

      /* Load the type into register */
      /*BPF_STMT(BPF_LD+BPF_H+BPF_ABS, 12),*/
      /* Does it match AoE (0x88a2)? */
      /*BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, 0x88a2, 0, 4),*/
      /* Load the flags into register */
      /*BPF_STMT(BPF_LD+BPF_B+BPF_ABS, 14),*/
      /* Check to see if the Resp flag is set */
      /*BPF_STMT(BPF_ALU+BPF_AND+BPF_K, (1 << 3)),*/
      /* Yes, goto INVALID */
      /*BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, 0, 0, 1),*/
      /* VALID: return -1 (allow the packet to be read) */
      /*BPF_STMT(BPF_RET+BPF_K, -1),*/
      /* INVALID: return 0 (ignore the packet) */
      /*BPF_STMT(BPF_RET+BPF_K, 0),*/
    };

    static struct sock_fprog prog = {
      .filter = filter,
      .len = sizeof(filter) / sizeof(struct sock_filter)
    };

    if (net_setsockopt(iface->fd, SOL_SOCKET, SO_ATTACH_FILTER, &prog, sizeof(prog)))
      syslog(LOG_ERR, "%s: Failed to set up the socket filter", strerror(errno));
  }
}
#endif

#endif
