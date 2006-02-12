/*
 *  Copyright 2002 Tobias Ringstrom <tobias@ringstrom.mine.nu>
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>

#include "ipsecadm.h"

struct ipsec_tunnel_parm
{
	char			name[IFNAMSIZ];
	int				link;
	struct iphdr	iph;
	int				spi;
};

static void
tunnel_usage(void)
{
	fputs("Usage:\n", stderr);
	fputs("    ipsecadm tunnel help\n", stderr);
	fputs("    ipsecadm tunnel add <dev> --local=<local> --remote=<remote>\n", stderr);
	fputs("                        [--spi=<spi>] [--nextdev=<dev>]\n", stderr);
	fputs("    ipsecadm tunnel modify <dev> --local=<local> --remote=<remote>\n", stderr);
	fputs("                        [--spi=<spi>] [--nextdev=<dev>]\n", stderr);
	fputs("    ipsecadm tunnel del --all | <dev>\n", stderr);
	fputs("    ipsecadm tunnel show [<dev>]\n", stderr);

	exit(1);
}

static int
ipsec_tunnel_add(const char *name,
				 uint32_t local,
				 uint32_t remote,
				 uint32_t spi,
				 int nextdev)
{
	struct ipsec_tunnel_parm ipsec;
	struct ifreq ifr;
	int fd, st;

	fd = ipsec_tunnel_open(IPSECDEVNAME, &ifr, 0);
	if (fd == -1)
		return -1;

	memset(&ipsec, 0, sizeof(struct ipsec_tunnel_parm));
	ifr.ifr_data = (char*)&ipsec;
	strncpy(ipsec.name, name, sizeof(ipsec.name));
	ipsec.link = nextdev;
	ipsec.iph.version = 4;
	ipsec.iph.ihl = 5;
	ipsec.iph.protocol = IPPROTO_ESP;
	ipsec.iph.daddr = remote;
	ipsec.iph.saddr = local;
	ipsec.spi = spi;

	st = ioctl(fd, SIOCIPSEC_ADD_TUNNEL, &ifr);
	if (st != 0)
	{
		error("Cannot add tunnel! [%s]",
			  errno == ENOBUFS ? "Tunnel already exists" : strerror(errno));
	}

	close(fd);

	return 0;
}

static int
ipsec_tunnel_modify(const char *name,
					int mod_local, uint32_t local,
					int mod_remote, uint32_t remote,
					int mod_spi, uint32_t spi,
					int mod_nextdev, int nextdev)
{
	struct ipsec_tunnel_parm ipsec;
	struct ifreq ifr;
	int fd, st;

	/* First get the old config. */
	fd = ipsec_tunnel_open(name, &ifr, 1);
	if (fd == -1)
		error("Cannot open tunnel! [%s]", strerror(errno));

	memset(&ipsec, 0, sizeof(struct ipsec_tunnel_parm));
	ifr.ifr_data = (char*)&ipsec;
	st = ioctl(fd, SIOCIPSEC_GET_TUNNEL, &ifr);
	if (st != 0)
		st = ioctl(fd, SIOCIPSEC_GET_TUNNEL_OLD, &ifr);
		
	if (st != 0)
		error("Cannot get tunnel! [%s]", strerror(errno));

	if (mod_nextdev)
		ipsec.link = nextdev;
	if (mod_remote)
		ipsec.iph.daddr = remote;
	if (mod_local)
		ipsec.iph.saddr = local;
	if (mod_spi)
		ipsec.spi = spi;

	st = ioctl(fd, SIOCIPSEC_CHG_TUNNEL, &ifr);
	if (st != 0)
		error("Cannot modify tunnel! [%s]", strerror(errno));

	close(fd);

	return 0;
}

static int
ipsec_tunnel_delete(const char *name)
{
	struct ifreq ifr;
	int fd, st;

	fd = ipsec_tunnel_open(name, &ifr, 0);
	if (fd == -1)
		return -1;

	st = ioctl(fd, SIOCIPSEC_DEL_TUNNEL, &ifr);
	if (st != 0)
		error("Cannot delete tunnel! [%s]", strerror(errno));

	close(fd);

	return 0;
}

static void
ipsec_tunnel_show_hdr(void)
{
	puts("Name            Local           Remote          SPI        Next device");
	puts("==========================================================================");
}

static int
ipsec_tunnel_show_one(const char *name)
{
	struct ipsec_tunnel_parm ipsec;
	struct ifreq ifr;
	char ifname[IFNAMSIZ+1];
	int fd, st;

	fd = ipsec_tunnel_open(name, &ifr, 1);
	if (fd == -1)
		return -1;

	memset(&ipsec, 0, sizeof(struct ipsec_tunnel_parm));
	ifr.ifr_data = (char*)&ipsec;
	st = ioctl(fd, SIOCIPSEC_GET_TUNNEL, &ifr);
	if (st != 0)
	{
		close(fd);
		return -1;
	}

	close(fd);

	printf("%-15s ", name);
	printf("%-15s ", ipv4_ntoa(ipsec.iph.saddr));
	printf("%-15s ", ipv4_ntoa(ipsec.iph.daddr));
	if (ipsec.spi == 0)
		printf("%-10s ", "any");
	else
		printf("0x%08x ", ipsec.spi);
	printf("%-15s ", ifindex_to_ifname(ifname, ipsec.link));
	putchar('\n');

	return 0;
}

static char**
ipsec_tunnel_find_all(int *n)
{
	FILE *f = fopen("/proc/net/dev", "r");
	char line[200], *p;
	struct ifreq ifr;
	int N = 1, fd;
	char **names;

	if (!f)
		error("Cannot open /proc/net/dev");

	fgets(line, sizeof(line) - 1, f);
	fgets(line, sizeof(line) - 1, f);

	*n = 0;
	names = malloc(N * sizeof(char*));

	for (;;)
	{
		fgets(line, sizeof(line) - 1, f);
		if (ferror(f))
			error("Error reading /proc/net/dev");
		if (feof(f))
			break;
		p = strchr(line, ':');
		if (!p)
			error("Error parsing /proc/net/dev");
		*p = '\0';
		p = line;
		while (*p == ' ')
			++p;
		/* Skip ipsec0 */
		if (strcmp(p, IPSECDEVNAME) == 0)
			continue;
		fd = ipsec_tunnel_open(p, &ifr, 1);
		if (fd != -1)
		{
			close(fd);
			if (*n == N)
			{
				N *= 2;
				names = realloc(names, N * sizeof(char*));
			}
			names[(*n)++] = strdup(p);
		}
	}
	fclose(f);

	return names;
}

static int
devname_cmp(const void *a, const void *b)
{
	return strcmp(*(char**)a, *(char**)b);
}

static int
ipsec_tunnel_show_all(void)
{
	char **list;
	int i, n;

	list = ipsec_tunnel_find_all(&n);

	qsort(list, n, sizeof(char*), devname_cmp);

	ipsec_tunnel_show_hdr();

	for (i = 0; i < n; ++i)
	{
		ipsec_tunnel_show_one(list[i]);
		free(list[i]);
	}

	free(list);

	return 0;
}

static int
tunnel_add(int argc, char *argv[])
{
	uint32_t local = 0, remote = 0, spi = IPSEC_SPI_ANY, nextdev = 0;
	int got_local = 0, got_remote = 0;
	int c;

	static struct option opts[] = {
		{ "local",            1, 0, 'l' },
		{ "remote",           1, 0, 'r' },
		{ "spi",              1, 0, 's' },
		{ "nextdev",          1, 0, 'n' },
		{0, 0, 0, 0}
	};

	if (argc < 1)
		tunnel_usage();

	for (;;)
	{
		c = getopt_long(argc - 1, argv + 1, "", opts, NULL);
		if (c == -1)
			break;

		switch (c)
		{
		case 'l':
			local = ipv4_aton(optarg);
			if (local == INADDR_ANY)
				error("Invalid local address!");
			got_local = 1;
			break;
		case 'r':
			remote = ipv4_aton(optarg);
			if (remote == INADDR_ANY)
				error("Invalid remote address!");
			got_remote = 1;
			break;
		case 's':
			spi = strtospi(optarg);
			break;
		case 'n':
			nextdev = ifname_to_ifindex(optarg);
			if (nextdev < 0)
				error("Invalid nextdev device!");
			break;
		case '?':
			tunnel_usage();
			break;
		default:
			printf("default: %d %c\n", c, c);
			break;
		}
	}

	if (optind != argc - 1)
		tunnel_usage();
	if (!got_local || !got_remote)
		error("Local and remote addresses must be specified!");

	return ipsec_tunnel_add(argv[1], local, remote, spi, nextdev);
}

static int
tunnel_modify(int argc, char *argv[])
{
	uint32_t local = 0, remote = 0, spi = IPSEC_SPI_ANY, nextdev = 0;
	int got_local = 0, got_remote = 0, got_spi = 0, got_nextdev = 0;
	int c;

	static struct option opts[] = {
		{ "local",            1, 0, 'l' },
		{ "remote",           1, 0, 'r' },
		{ "spi",              1, 0, 's' },
		{ "nextdev",          1, 0, 'n' },
		{0, 0, 0, 0}
	};

	if (argc < 1)
			tunnel_usage();

	for (;;)
	{
		c = getopt_long(argc - 1, argv + 1, "", opts, NULL);
		if (c == -1)
			break;

		switch (c)
		{
		case 'l':
			local = ipv4_aton(optarg);
			if (local == INADDR_ANY)
				error("Invalid local address!");
			got_local = 1;
			break;
		case 'r':
			remote = ipv4_aton(optarg);
			if (remote == INADDR_ANY)
				error("Invalid remote address!");
			got_remote = 1;
			break;
		case 's':
			spi = strtospi(optarg);
			got_spi = 1;
			break;
		case 'n':
			nextdev = ifname_to_ifindex(optarg);
			if (nextdev < 0)
				error("Invalid nextdev device!");
			got_nextdev = 1;
			break;
		case '?':
			tunnel_usage();
			break;
		default:
			printf("default: %d %c\n", c, c);
			break;
		}
	}

	if (optind != argc - 1)
		tunnel_usage();

	if (!got_local && !got_remote && !got_spi && !got_nextdev)
		error("Nothing is changed.");

	return ipsec_tunnel_modify(argv[1],
							   got_local, local,
							   got_remote, remote,
							   got_spi, spi,
							   got_nextdev, nextdev);
}

static int
tunnel_delete(int argc, char *argv[])
{
	char **list;
	int i, n;

	if (argc != 2)
		tunnel_usage();

	if (strcmp(argv[1], "--all") != 0)
		return ipsec_tunnel_delete(argv[1]);

	list = ipsec_tunnel_find_all(&n);

	for (i = 0; i < n; ++i)
	{
		ipsec_tunnel_delete(list[i]);
		free(list[i]);
	}

	free(list);

	return 0;
}

static int
tunnel_show(int argc, char *argv[])
{
	struct ifreq ifr;
	int ret, fd;

	/* Make sure the module is loaded by checking that IPSECDEVNAME is
	 * available. We do this here (non-quietly) since it's only done
	 * quietly later on. */
	fd = ipsec_tunnel_open(IPSECDEVNAME, &ifr, 0);
	if (fd == -1)
		return -1;
	close(fd);

	if (argc == 1)
		return ipsec_tunnel_show_all();
	else if (argc == 2)
	{
		ipsec_tunnel_show_hdr();
		ret = ipsec_tunnel_show_one(argv[1]);
		if (ret != 0)
			error("Cannot show %s!", argv[1]);
		return ret;
	}

	tunnel_usage();

	return 1;
}

int
tunnel_main(int argc, char *argv[])
{
	const char *modes[] = {
		"help", "usage",
		"add", "change", "modify", "delete", "show", NULL };
	int mode;

	if (argc < 2)
		tunnel_usage();

	mode = find_unambiguous_string(modes, argv[1]);
	switch (mode)
	{
	case 0:
	case 1:
		tunnel_usage();
		return 0;
	case 2:
		return tunnel_add(argc - 1, argv + 1);
	case 3:
	case 4:
		return tunnel_modify(argc - 1, argv + 1);
	case 5:
		return tunnel_delete(argc - 1, argv + 1);
	case 6:
		return tunnel_show(argc - 1, argv + 1);
	}

	tunnel_usage();

	return 1;
}
