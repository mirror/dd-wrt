// Copyright (c) 2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: (GPL-2.0)

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <linux/types.h>
#include <net/if.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <linux/if_ether.h>

#include "utils.h"

static int netsock = 0;

int if_get_mac(int ifindex, unsigned char *mac)
{
	char name[IF_NAMESIZE];
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(ifr));
	if (!if_indextoname(ifindex, name))
		return 0;

	strncpy(ifr.ifr_name, name, IFNAMSIZ - 1);
	if(ioctl(netsock, SIOCGIFHWADDR, &ifr))
		return 0;

	memcpy(mac, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
	return 0;
}

int if_get_link(int ifindex)
{
	char name[IF_NAMESIZE];
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(ifr));
	if (!if_indextoname(ifindex, name))
		return 0;

	strncpy(ifr.ifr_name, name, IFNAMSIZ - 1);
	if(ioctl(netsock, SIOCGIFFLAGS, &ifr))
		return 0;

	return ifr.ifr_flags & IFF_RUNNING;
}

int if_init(void)
{
	netsock = socket(AF_INET, SOCK_DGRAM, 0);
	if (!netsock) {
		printf("netsock was not open!\n");
		return -1;
	}

	return 0;
}

struct frame_buf *fb_alloc(uint32_t size)
{
	struct frame_buf *fb;

	fb = malloc(sizeof(*fb));
	if (!fb)
		return NULL;

	fb->start = malloc(size);
	if (!fb->start) {
		free(fb);
		return NULL;
	}
	memset(fb->start, 0x0, size);

	fb->data = fb->start;
	fb->size = 0;
	return fb;
};

void *fb_put(struct frame_buf *fb, uint32_t size) {
	void *res = fb->data;
	fb->data += size;
	fb->size += size;
	return res;
}

void ether_addr_copy(uint8_t *dst, const uint8_t *src)
{
	uint16_t *a = (uint16_t *)dst;
	const uint16_t *b = (const uint16_t *)src;

	a[0] = b[0];
	a[1] = b[1];
	a[2] = b[2];
}

bool ether_addr_equal(const uint8_t *addr1, const uint8_t *addr2)
{
	const uint16_t *a = (const uint16_t *)addr1;
	const uint16_t *b = (const uint16_t *)addr2;

	return ((a[0] ^ b[0]) | (a[1] ^ b[1]) | (a[2] ^ b[2])) == 0;
}

uint64_t ether_addr_to_u64(const uint8_t *addr)
{
	uint64_t u = 0;
	int i;

	for (i = 0; i < ETH_ALEN; i++)
		u = u << 8 | addr[i];

	return u;
}

uint32_t get_ms(void)
{
	struct timeval  tv;
	gettimeofday(&tv, NULL);

	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void if_cleanup(void)
{
	close(netsock);
}
