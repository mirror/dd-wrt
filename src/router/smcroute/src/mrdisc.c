/* Multicast Router Discovery Protocol, RFC4286 (IPv4 only)
 *
 * Copyright (C) 2017-2021  Joachim Wiberg <troglobit@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef __linux__
/* We use SO_BINDTODEVICE to be able to send MRDISC on interfaces that
 * may not have an IP address yet.  I have not found any way of doing
 * this on FreeBSD.  Best I could find was an aging patch for IP_SENDIF
 * https://forums.freebsd.org/threads/so_bindtodevice-undeclared-on-freebsd-12.73731/
 * that never got merged.  It's possible there are other ways to do the
 * same, but now you know as much as I do.
 */
#error Currently only works on Linux, patches for FreeBSD are most welcome.
#endif

#include "config.h"
#include "queue.h"

#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/igmp.h>
#include <sys/socket.h>

#include "log.h"
#include "mrdisc.h"
#include "socket.h"
#include "timer.h"
#include "util.h"

#define MC_ALL_ROUTERS       "224.0.0.2"
#define MC_ALL_SNOOPERS      "224.0.0.106"

#define IGMP_MRDISC_ANNOUNCE 0x30
#define IGMP_MRDISC_SOLICIT  0x31
#define IGMP_MRDISC_TERM     0x32

struct ifsock {
	LIST_ENTRY(ifsock) link;

	int    sd;
	char   ifname[IFNAMSIZ];
	vifi_t vif;
};

static uint8_t interval       = 20;
static LIST_HEAD(ifslist, ifsock) ifsock_list = LIST_HEAD_INITIALIZER();


static struct ifsock *find(int sd)
{
	struct ifsock *entry;

	LIST_FOREACH(entry, &ifsock_list, link) {
		if (entry->sd == sd)
			return entry;
	}

	return NULL;
}

/* Checksum routine for Internet Protocol family headers */
static unsigned short in_cksum(unsigned short *addr, int len)
{
	unsigned short answer = 0;
	unsigned short *w = addr;
	int nleft = len;
	int sum = 0;

	/*
	 * Our algorithm is simple, using a 32 bit accumulator (sum), we add
	 * sequential 16 bit words to it, and at the end, fold back all the
	 * carry bits from the top 16 bits into the lower 16 bits.
	 */
	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1) {
		*(unsigned char *)(&answer) = *(unsigned char *)w;
		sum += answer;
	}

	/* add back carry outs from top 16 bits to low 16 bits */
	sum  = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */

	return answer;
}

static void compose_addr(struct sockaddr_in *sin, char *group)
{
	memset(sin, 0, sizeof(*sin));
	sin->sin_family      = AF_INET;
	sin->sin_addr.s_addr = inet_addr(group);
}

static int inet_send(int sd, uint8_t type, uint8_t interval)
{
	struct sockaddr dest;
	struct igmp igmp;
	ssize_t num;

	memset(&igmp, 0, sizeof(igmp));
	igmp.igmp_type = type;
	igmp.igmp_code = interval;
	igmp.igmp_cksum = in_cksum((unsigned short *)&igmp, sizeof(igmp));

	compose_addr((struct sockaddr_in *)&dest, MC_ALL_SNOOPERS);

	num = sendto(sd, &igmp, sizeof(igmp), 0, &dest, sizeof(dest));
	if (num < 0)
		return -1;

	return 0;
}

/* If called with interval=0, only read() */
static int inet_recv(int sd, uint8_t interval)
{
	struct igmp *igmp;
	char buf[1530];
	struct ip *ip;
	ssize_t num;

	memset(buf, 0, sizeof(buf));
	num = read(sd, buf, sizeof(buf));
	if (num < 0)
		return -1;

	ip = (struct ip *)buf;
	igmp = (struct igmp *)(buf + (ip->ip_hl << 2));
	if (igmp->igmp_type == IGMP_MRDISC_SOLICIT && interval > 0) {
		smclog(LOG_DEBUG, "Received mrdisc solicitation");
		return inet_send(sd, IGMP_MRDISC_ANNOUNCE, interval);
	}

	return 0;
}

static int inet_open(char *ifname)
{
	unsigned char ra[4] = { IPOPT_RA, 0x04, 0x00, 0x00 };
	struct ip_mreqn mreq;
	struct ifreq ifr;
	int sd, val, rc;
	char loop;

	sd = socket_create(AF_INET, SOCK_RAW, IPPROTO_IGMP, mrdisc_recv, NULL);
	if (sd < 0) {
		smclog(LOG_ERR, "Cannot open socket: %s", strerror(errno));
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", ifname);
	if (setsockopt(sd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
		if (ENODEV == errno) {
			smclog(LOG_WARNING, "Not a valid interface, %s, skipping ...", ifname);
			socket_close(sd);
			return -1;
		}

		smclog(LOG_ERR, "Cannot bind socket to interface %s: %s", ifname, strerror(errno));
		socket_close(sd);
		return -1;
	}

	memset(&mreq, 0, sizeof(mreq));
	mreq.imr_multiaddr.s_addr = inet_addr(MC_ALL_SNOOPERS);
	mreq.imr_ifindex = if_nametoindex(ifname);
        if (setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq))) {
		smclog(LOG_ERR, "Failed joining group %s: %s", MC_ALL_SNOOPERS, strerror(errno));
		return -1;
	}

	/* mrdisc solicitation messages goes to the All-Routers group */
	mreq.imr_multiaddr.s_addr = inet_addr(MC_ALL_ROUTERS);
	mreq.imr_ifindex = if_nametoindex(ifname);
        if (setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq))) {
		smclog(LOG_ERR, "Failed joining group %s: %s", MC_ALL_ROUTERS, strerror(errno));
		return -1;
	}

	val = 1;
	rc = setsockopt(sd, IPPROTO_IP, IP_MULTICAST_TTL, &val, sizeof(val));
	if (rc < 0) {
		smclog(LOG_ERR, "Cannot set TTL: %s", strerror(errno));
		socket_close(sd);
		return -1;
	}

	loop = 0;
	rc = setsockopt(sd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
	if (rc < 0) {
		smclog(LOG_ERR, "Cannot disable MC loop: %s", strerror(errno));
		socket_close(sd);
		return -1;
	}

	rc = setsockopt(sd, IPPROTO_IP, IP_OPTIONS, &ra, sizeof(ra));
	if (rc < 0) {
		smclog(LOG_ERR, "Cannot set IP OPTIONS: %s", strerror(errno));
		socket_close(sd);
		return -1;
	}

	return sd;
}

static int inet_close(int sd)
{
	return  inet_send(sd, IGMP_MRDISC_TERM, 0) ||
		socket_close(sd);
}

static void announce(struct ifsock *entry)
{
	if (!entry)
		return;

	smclog(LOG_DEBUG, "Sending mrdisc announcement on %s", entry->ifname);
	if (inet_send(entry->sd, IGMP_MRDISC_ANNOUNCE, interval)) {
		if (ENETUNREACH == errno || ENETDOWN == errno)
			return;	/* Link down, ignore. */

		smclog(LOG_WARNING, "Failed sending IGMP control message 0x%x on %s, error %d: %s",
		       IGMP_MRDISC_ANNOUNCE, entry->ifname, errno, strerror(errno));
	}
}

int mrdisc_init(int period)
{
	interval = period;
	if (timer_add(interval, mrdisc_send, NULL) < 0 && errno != EEXIST) {
		smclog(LOG_ERR, "Failed starting mrdisc announcement timer.");
		return -1;
	}

	return 0;
}

int mrdisc_exit(void)
{
	struct ifsock *entry, *tmp;

	LIST_FOREACH_SAFE(entry, &ifsock_list, link, tmp) {
		inet_close(entry->sd);
		LIST_REMOVE(entry, link);
		free(entry);
	}

	return 0;
}

/*
 * Register possible interface for mrdisc
 */
int mrdisc_register(char *ifname, short vif)
{
	struct ifsock *entry;

	LIST_FOREACH(entry, &ifsock_list, link) {
		if (!strcmp(entry->ifname, ifname))
			goto reload;
	}

	entry = malloc(sizeof(*entry));
	if (!entry) {
		smclog(LOG_ERR, "Out of memory in %s()", __func__);
		return -1;
	}

	entry->vif = vif;
	strlcpy(entry->ifname, ifname, sizeof(entry->ifname));
	LIST_INSERT_HEAD(&ifsock_list, entry, link);

	entry->sd = inet_open(entry->ifname);
	if (entry->sd < 0)
		return -1;
reload:
	announce(entry);
	return 0;
}

/*
 * Unregister mrdisc interface, regardless of refcnt
 */
int mrdisc_deregister(short vif)
{
	struct ifsock *entry;

	LIST_FOREACH(entry, &ifsock_list, link) {
		if (entry->vif != vif)
			continue;

		inet_close(entry->sd);
		LIST_REMOVE(entry, link);
		free(entry);
		return 0;
	}

	return 0;
}

void mrdisc_send(void *arg)
{
	struct ifsock *entry;

	(void)arg;
	LIST_FOREACH(entry, &ifsock_list, link)
		announce(entry);
}

void mrdisc_recv(int sd, void *arg)
{
	struct ifsock *entry;

	(void)arg;

	/* Verify we are reading from an active socket */
	entry = find(sd);
	if (!entry) {
		smclog(LOG_WARNING, "Bug in mrdisc, received frame on unknown socket %d", sd);
		return;
	}

	/* Only do a "dummy" read on inactive interfaces */
	if (inet_recv(sd, interval))
		smclog(LOG_WARNING, "Failed receiving IGMP control message from %s", entry->ifname);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
