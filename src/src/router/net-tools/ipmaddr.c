/*
 * ipmaddr.c		"ip maddress".
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
 * Changes:	Arnaldo Carvalho de Melo <acme@conectiva.com.br>
 *		20010404 - use setlocale
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#if defined(__GLIBC__) && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 1))
#include <net/if.h>
#else
#include <linux/if.h>
#endif

#include "config.h"
#include "intl.h"
#include "util-ank.h"
#include "net-support.h"
#include "version.h"
#include "pathnames.h"

char filter_dev[16];
int  filter_family;

/* These have nothing to do with rtnetlink. :-) */
#define NEWADDR		1
#define DELADDR		2

char *Release = RELEASE,
     *Version = "ipmaddr 1.1",
     *Signature = "Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>";

static void version(void)
{
	printf("%s\n%s\n%s\n", Release, Version, Signature);
	exit(E_VERSION);
}

static void usage(void) __attribute__((noreturn));

static void usage(void)
{
	fprintf(stderr, _("Usage: ipmaddr [ add | del ] MULTIADDR dev STRING\n"));
	fprintf(stderr, _("       ipmaddr show [ dev STRING ] [ ipv4 | ipv6 | link | all ]\n"));
	fprintf(stderr, _("       ipmaddr -V | -version\n"));
	exit(-1);
}

static void print_lla(FILE *fp, int len, unsigned char *addr)
{
	int i;
	for (i=0; i<len; i++) {
		if (i==0)
			fprintf(fp, "%02x", addr[i]);
		else
			fprintf(fp, ":%02x", addr[i]);
	}
}

static int parse_lla(char *str, unsigned char *addr)
{
	int len=0;

	while (*str) {
		int tmp;
		if (str[0] == ':' || str[0] == '.') {
			str++;
			continue;
		}
		if (str[1] == 0)
			return -1;
		if (sscanf(str, "%02x", &tmp) != 1)
			return -1;
		addr[len] = tmp;
		len++;
		str += 2;
	}
	return len;
}

static int parse_hex(char *str, unsigned char *addr)
{
	int len=0;

	while (*str) {
		int tmp;
		if (str[1] == 0)
			return -1;
		if (sscanf(str, "%02x", &tmp) != 1)
			return -1;
		addr[len] = tmp;
		len++;
		str += 2;
	}
	return len;
}

struct ma_info
{
	struct ma_info *next;
	int		index;
	int		users;
	char		*features;
	char		name[IFNAMSIZ];
	inet_prefix	addr;
};

void maddr_ins(struct ma_info **lst, struct ma_info *m)
{
	struct ma_info *mp;

	for (; (mp=*lst) != NULL; lst = &mp->next) {
		if (mp->index > m->index)
			break;
	}
	m->next = *lst;
	*lst = m;
}

void read_dev_mcast(struct ma_info **result_p)
{
	char buf[256];
	FILE *fp = fopen(_PATH_PROCNET_DEV_MCAST, "r");

	if (!fp)
		return;

	while (fgets(buf, sizeof(buf), fp)) {
		char hexa[256];
		struct ma_info m;
		int len;
		int st;

		memset(&m, 0, sizeof(m));
		sscanf(buf, "%d%s%d%d%s", &m.index, m.name, &m.users, &st,
		       hexa);
		if (filter_dev[0] && strcmp(filter_dev, m.name))
			continue;

		m.addr.family = AF_PACKET;

		len = parse_hex(hexa, (unsigned char*)&m.addr.data);
		if (len >= 0) {
			struct ma_info *ma = malloc(sizeof(m));

			memcpy(ma, &m, sizeof(m));
			ma->addr.bytelen = len;
			ma->addr.bitlen = len<<3;
			if (st)
				ma->features = "static";
			maddr_ins(result_p, ma);
		}
	}
	fclose(fp);
}

void read_igmp(struct ma_info **result_p)
{
	struct ma_info m;
	char buf[256];
	FILE *fp = fopen(_PATH_PROCNET_IGMP, "r");

	if (!fp)
		return;
	memset(&m, 0, sizeof(m));
	fgets(buf, sizeof(buf), fp);

	m.addr.family = AF_INET;
	m.addr.bitlen = 32;
	m.addr.bytelen = 4;

	while (fgets(buf, sizeof(buf), fp)) {
		struct ma_info *ma = malloc(sizeof(m));

		if (buf[0] != '\t') {
			sscanf(buf, "%d%s", &m.index, m.name);
			continue;
		}

		if (filter_dev[0] && strcmp(filter_dev, m.name))
			continue;

		sscanf(buf, "%08x%d", (__u32*)&m.addr.data, &m.users);

		ma = malloc(sizeof(m));
		memcpy(ma, &m, sizeof(m));
		maddr_ins(result_p, ma);
	}
	fclose(fp);
}


void read_igmp6(struct ma_info **result_p)
{
	char buf[256];
	FILE *fp = fopen(_PATH_PROCNET_IGMP6, "r");

	if (!fp)
		return;

	while (fgets(buf, sizeof(buf), fp)) {
		char hexa[256];
		struct ma_info m;
		int len;

		memset(&m, 0, sizeof(m));
		sscanf(buf, "%d%s%s%d", &m.index, m.name, hexa, &m.users);

		if (filter_dev[0] && strcmp(filter_dev, m.name))
			continue;

		m.addr.family = AF_INET6;

		len = parse_hex(hexa, (unsigned char*)&m.addr.data);
		if (len >= 0) {
			struct ma_info *ma = malloc(sizeof(m));

			memcpy(ma, &m, sizeof(m));

			ma->addr.bytelen = len;
			ma->addr.bitlen = len<<3;
			maddr_ins(result_p, ma);
		}
	}
	fclose(fp);
}

static void print_maddr(FILE *fp, struct ma_info *list)
{
	fprintf(fp, "\t");

	if (list->addr.family == AF_PACKET) {
		fprintf(fp, "link  ");
		print_lla(fp, list->addr.bytelen, (unsigned char*)list->addr.data);
	} else {
		char abuf[256];
		switch(list->addr.family) {
		case AF_INET:
			fprintf(fp, "inet  ");
			break;
		case AF_INET6:
			fprintf(fp, "inet6 ");
			break;
		default:
			fprintf(fp, _("family %d "), list->addr.family);
			break;
		}
		if (format_host(list->addr.family, list->addr.data, abuf, sizeof(abuf)))
			fprintf(fp, "%s", abuf);
		else
			fprintf(fp, "?");
	}
	if (list->users != 1)
		fprintf(fp, _(" users %d"), list->users);
	if (list->features)
		fprintf(fp, " %s", list->features);
	fprintf(fp, "\n");
}

static void print_mlist(FILE *fp, struct ma_info *list)
{
	int cur_index = 0;

	for (; list; list = list->next) {
		if (cur_index != list->index) {
			cur_index = list->index;
			fprintf(fp, "%d:\t%s\n", cur_index, list->name);
		}
		print_maddr(fp, list);
	}
}

static int multiaddr_list(int argc, char **argv)
{
	struct ma_info *list = NULL;

	while (argc > 0) {
		if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			if (filter_dev[0])
				usage();
			strcpy(filter_dev, *argv);
		} else if (strcmp(*argv, "all") == 0) {
			filter_family = AF_UNSPEC;
		} else if (strcmp(*argv, "ipv4") == 0) {
			filter_family = AF_INET;
		} else if (strcmp(*argv, "ipv6") == 0) {
			filter_family = AF_INET6;
		} else if (strcmp(*argv, "link") == 0) {
			filter_family = AF_PACKET;
		} else {
			if (filter_dev[0])
				usage();
			strcpy(filter_dev, *argv);
		}
		argv++; argc--;
	}

	if (!filter_family || filter_family == AF_PACKET)
		read_dev_mcast(&list);
	if (!filter_family || filter_family == AF_INET)
		read_igmp(&list);
	if (!filter_family || filter_family == AF_INET6)
		read_igmp6(&list);
	print_mlist(stdout, list);
	return 0;
}

int multiaddr_modify(int cmd, int argc, char **argv)
{
	struct ifreq ifr;
	int fd;

	memset(&ifr, 0, sizeof(ifr));

	if (cmd == NEWADDR)
		cmd = SIOCADDMULTI;
	else
		cmd = SIOCDELMULTI;

	while (argc > 0) {
		if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			if (ifr.ifr_name[0])
				usage();
			strncpy(ifr.ifr_name, *argv, IFNAMSIZ);
		} else {
			if (ifr.ifr_hwaddr.sa_data[0])
				usage();
			if (parse_lla(*argv, ifr.ifr_hwaddr.sa_data) < 0)
				usage();
		}
		argc--; argv++;
	}
	if (ifr.ifr_name[0] == 0)
		usage();

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror(_("Cannot create socket"));
		exit(1);
	}
	if (ioctl(fd, cmd, (char*)&ifr) != 0) {
		perror("ioctl");
		exit(1);
	}
	close(fd);

	exit(0);
}


int do_multiaddr(int argc, char **argv)
{
	if (argc < 1)
		return multiaddr_list(0, NULL);
	if (matches(*argv, "add") == 0)
		return multiaddr_modify(NEWADDR, argc-1, argv+1);
	if (matches(*argv, "delete") == 0)
		return multiaddr_modify(DELADDR, argc-1, argv+1);
	if (matches(*argv, "list") == 0 || matches(*argv, "show") == 0
	    || matches(*argv, "lst") == 0)
		return multiaddr_list(argc-1, argv+1);
	usage();
}

int preferred_family = AF_UNSPEC;
int show_stats = 0;
int resolve_hosts = 0;

int main(int argc, char **argv)
{
	char *basename;

#if I18N
	setlocale (LC_ALL, "");
	bindtextdomain("net-tools", "/usr/share/locale");
	textdomain("net-tools");
#endif

	basename = strrchr(argv[0], '/');
	if (basename == NULL)
		basename = argv[0];
	else
		basename++;
	
	while (argc > 1) {
		if (argv[1][0] != '-')
			break;
		if (matches(argv[1], "-family") == 0) {
			argc--;
			argv++;
			if (argc <= 1)
				usage();
			if (strcmp(argv[1], "inet") == 0)
				preferred_family = AF_INET;
			else if (strcmp(argv[1], "inet6") == 0)
				preferred_family = AF_INET6;
			else
				usage();
		} else if (matches(argv[1], "-stats") == 0 ||
			   matches(argv[1], "-statistics") == 0) {
			++show_stats;
		} else if (matches(argv[1], "-resolve") == 0) {
			++resolve_hosts;
		} else if ((matches(argv[1], "-V") == 0) || matches(argv[1], "--version") == 0) {
			version();
		} else
			usage();
		argc--;	argv++;
	}

	return do_multiaddr(argc-1, argv+1);
}
