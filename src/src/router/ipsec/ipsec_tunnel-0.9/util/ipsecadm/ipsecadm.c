/*
 *  Copyright 2002 Tobias Ringstrom <tobias@ringstrom.mine.nu>
 *  Authentication Copyright 2002 Arcturus Networks Inc.
 *      by Norman Shulman <norm@arcturusnetworks.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#include "ipsecadm.h"

#define VERSION "0.9-pre"

void
usage(void)
{
	fprintf(stderr, "Usage: ipsecadm <mode> [options]\n");
	fprintf(stderr, "    (<mode> can be either key, sa, tunnel or stats)\n");
	fprintf(stderr, "Run \"ipsecadm <mode> help\" for mode specific help!\n");
	exit(1);
}

void
error(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fputc('\n', stderr);
	exit(1);
}

int
find_unambiguous_string(const char *const strlist[], const char *str)
{
	int i, match = -1;

	for (i = 0; strlist[i] != NULL; ++i)
	{
		if (strncmp(str, strlist[i], strlen(str)) == 0)
		{
			if (match != -1)
				return -2; /* Ambiguous */
			match = i;
		}
	}

	return match;
}

const char*
ipv4_ntoa(uint32_t addr)
{
	struct in_addr a;

	if (addr == INADDR_ANY)
		return "any";

	a.s_addr = addr;

	return inet_ntoa(a);
}

uint32_t
ipv4_aton(const char *str)
{
	if (strcmp(str, "any") == 0)
		return INADDR_ANY;

	return inet_addr(str);
}

uint32_t
strtospi(const char *str)
{
	if (strcmp(str, "any") == 0)
		return IPSEC_SPI_ANY;

	return strtoul(str, NULL, 0);
}

int
ifname_to_ifindex(const char *name)
{
	struct ifreq ifr;
	int fd, st;

	if (strcmp(name, "any") == 0)
		return 0;

	fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (fd == -1)
		error("Cannot open socket!");

	memset(&ifr, 0, sizeof(struct ifreq));
	strcpy(ifr.ifr_name, name);
	st = ioctl(fd, SIOCGIFINDEX, &ifr);
	close(fd);

	return st == 0 ? ifr.ifr_ifindex : -1;
}

char*
ifindex_to_ifname(char *ifname, int ifindex)
{
	struct ifreq ifr;
	int fd, st;

	if (ifindex == 0)
	{
		strcpy(ifname, "any");
		return ifname;
	}

	fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (fd == -1)
		error("Cannot open socket!");

	memset(&ifr, 0, sizeof(struct ifreq));
	ifr.ifr_ifindex = ifindex;
	st = ioctl(fd, SIOCGIFNAME, &ifr);
	close(fd);

	strcpy(ifname, st == 0 ? ifr.ifr_name : "unknown");

	return ifname;
}

int
ipsec_tunnel_open(const char *name, struct ifreq *ifr, int quiet)
{
	int fd, st;

	fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (fd == -1)
		error("Cannot open socket!");

	memset(ifr, 0, sizeof(struct ifreq));
	strcpy(ifr->ifr_name, name);
	st = ioctl(fd, SIOCGIFHWADDR, ifr);
	if (st != 0)
	{
		if (!quiet)
		{
			int err = errno;
			fprintf(stderr, "Cannot open %s! [%s]\n",
					name, strerror(err));
			if (err == ENODEV && strcmp(name, IPSECDEVNAME) == 0)
				fprintf(stderr, "Make sure the kernel module is loaded!\n");
		}
		close(fd);
		return -1;
	}

	if (ifr->ifr_hwaddr.sa_family != ARPHRD_IPSEC)
	{
		if (!quiet)
			fprintf(stderr, "Not an IPsec device!\n");
		close(fd);
		return -1;
	}

	return fd;
}

int
hex2dec(int c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

int
parse_key(const char *str, void *key, int maxsize)
{
	int pos, n1, n0, keysize;
	unsigned char *ckey = key;

	keysize = strlen(str);
	if (keysize % 2 != 0)
		error("Key length is not an even number!");
	keysize /= 2;
	if (keysize > maxsize)
		error("Key is too long!");

	for (pos = 0; pos < keysize; ++pos)
	{
		n1 = hex2dec(str[2*pos]);
		n0 = hex2dec(str[2*pos+1]);
		if (n0 == -1 || n1 == -1)
			error("Key contains non-hexadecimal characters!");
		ckey[pos] = 16 * n1 + n0;
	}

	return keysize;
}

int
read_key_file(const char *filename, void *key, int maxsize)
{
	FILE *f = fopen(filename, "rb");
	int n;

	if (!f)
		return -1;

	n = fread(key, 1, maxsize, f);
	if (n == maxsize && fread(key, 1, maxsize, f) > 0)
		error("Key is too long!");
	fclose(f);

	return n;
}

int
help_main(int argc, char *argv[])
{
	usage();

	return 0;
}

int
main(int argc, char *argv[])
{
	const char *modes[] = {
		"help", "usage",
		"version",
		"sa", "tunnel", "stats", "key",
		NULL };
	int mode;

	if (argc < 2)
		usage();

	mode = find_unambiguous_string(modes, argv[1]);
	switch (mode)
	{
	case 0:
	case 1:
		usage();
		return 0;
	case 2:
		printf("ipsec_tunnel ipsecadm version: "VERSION"\n");
		return 0;
	case 3:
		return sa_main(argc - 1, argv + 1);
	case 4:
		return tunnel_main(argc - 1, argv + 1);
	case 5:
		return stats_main(argc - 1, argv + 1);
	case 6:
		return key_main(argc - 1, argv + 1);
	}

	usage();

	return 1;
}
