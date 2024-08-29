/*
 * Name Interfaces based on MAC address.
 * Writen 2000 by Andi Kleen.
 * Subject to the Gnu Public License, version 2.
 * TODO: make it support token ring etc.
 * $Id: nameif.c,v 1.4 2003/09/11 03:46:49 ak Exp $
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <getopt.h>
#include <sys/syslog.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <errno.h>
#include "intl.h"
#include "net-support.h"
#include "util.h"

/* Current limitation of Linux network device ioctl(2) interface */
#define MAC_ADDRESS_MAX_LENGTH (sizeof(((struct ifreq *)0)->ifr_hwaddr.sa_data))

static const char default_conf[] = "/etc/mactab";
static const char *fname = default_conf;
static int use_syslog;
static int ctl_sk = -1;

attribute_printf(1, 2)
static void complain(const char *fmt, ...)
{
	va_list ap;
	va_start(ap,fmt);
	if (use_syslog) {
		vsyslog(LOG_ERR,fmt,ap);
	} else {
		vfprintf(stderr,fmt,ap);
		fputc('\n',stderr);
	}
	va_end(ap);
	exit(1);
}

attribute_printf(1, 2)
static void warning(const char *fmt, ...)
{
	va_list ap;
	va_start(ap,fmt);
	if (use_syslog) {
		vsyslog(LOG_ERR,fmt,ap);
	} else {
		vfprintf(stderr,fmt,ap);
		fputc('\n',stderr);
	}
	va_end(ap);
}

static int parsemac(char *str, unsigned int *len, unsigned char *mac, const char *pos)
{
	char *s;
	while ((s = strsep(&str, ":")) != NULL) {
		unsigned byte;
		if (sscanf(s,"%x", &byte)!=1 || byte > 0xff)
			return -1;
		if (++(*len) > MAC_ADDRESS_MAX_LENGTH) {
			complain("MAC address at %s is larger than maximum allowed %zu bytes",
				pos, MAC_ADDRESS_MAX_LENGTH);
		}
		*mac++ = byte;
	}
	return 0;
}

static void opensock(void)
{
	if (ctl_sk < 0)
		ctl_sk = socket(PF_INET,SOCK_DGRAM,0);
}

#ifndef ifr_newname
#define ifr_newname ifr_ifru.ifru_slave
#endif

static int setname(const char *oldname, const char *newname)
{
	struct ifreq ifr;
	opensock();
	memset(&ifr,0,sizeof(struct ifreq));
	safe_strncpy(ifr.ifr_name, oldname, IFNAMSIZ);
	safe_strncpy(ifr.ifr_newname, newname, IFNAMSIZ);
	return ioctl(ctl_sk, SIOCSIFNAME, &ifr);
}

static int getmac(const char *name, unsigned char *mac)
{
	int r;
	struct ifreq ifr;
	opensock();
	memset(&ifr,0,sizeof(struct ifreq));
	safe_strncpy(ifr.ifr_name, name, IFNAMSIZ);
	r = ioctl(ctl_sk, SIOCGIFHWADDR, &ifr);
	memcpy(mac, ifr.ifr_hwaddr.sa_data, MAC_ADDRESS_MAX_LENGTH);
	return r;
}

struct change {
	struct change *next;
	int found;
	char ifname[IFNAMSIZ+1];
	unsigned int macaddrlen;
	unsigned char mac[MAC_ADDRESS_MAX_LENGTH];
};
static struct change *clist;

static struct change *lookupmac(unsigned char *mac)
{
	struct change *ch;
	for (ch = clist;ch;ch = ch->next)
		if (memcmp(ch->mac, mac, ch->macaddrlen) == 0)
			return ch;
	return NULL;
}

static int addchange(char *p, struct change *ch, const char *pos)
{
	if (strchr(ch->ifname, ':'))
		warning(_("alias device %s at %s probably has no mac"),
			ch->ifname, pos);
	ch->macaddrlen = 0;
	if (parsemac(p, &ch->macaddrlen, ch->mac, pos) < 0)
		complain(_("cannot parse MAC `%s' at %s"), p, pos);
	ch->next = clist;
	clist = ch;
	return 0;
}

static void readconf(void)
{
	char *line;
	size_t linel;
	int linenum;
	FILE *ifh;
	char *p;
	int n;
	struct change *ch = NULL;

	ifh = fopen(fname, "r");
	if (!ifh)
		complain(_("opening configuration file %s: %s"),fname,strerror(errno));

	line = NULL;
	linel = 0;
	linenum = 1;
	while (getdelim(&line, &linel, '\n', ifh) > 0) {
		char pos[20];

		sprintf(pos, _("line %d"), linenum);

		if ((p = strchr(line,'#')) != NULL)
			*p = '\0';
		p = line;
		while (isspace(*p))
			++p;
		if (*p == '\0')
			continue;
		n = strcspn(p, " \t");
		if (n > IFNAMSIZ-1)
			complain(_("interface name too long at %s"), line);
		ch = xmalloc(sizeof(struct change));
		memcpy(ch->ifname, p, n);
		ch->ifname[n] = 0;
		p += n;
		p += strspn(p, " \t");
		n = strspn(p, "0123456789ABCDEFabcdef:");
		p[n] = 0;
		addchange(p, ch, pos);
		linenum++;
	}
	fclose(ifh);
}

static const struct option lopt[] = {
	{"syslog", 0, NULL, 's' },
	{"config-file", 1, NULL, 'c' },
	{"help", 0, NULL, 'h' },
	{NULL},
};

static void usage(int rc)
{
	FILE *fp = rc ? stderr : stdout;
	fprintf(fp, _("usage: nameif [-c configurationfile] [-s] {ifname macaddress}\n"));
	exit(E_USAGE);
}

int main(int ac, char **av)
{
	FILE *ifh;
	char *p;
	int n;
	int linenum;
	char *line = NULL;
	size_t linel = 0;
	int ret = 0;

	for (;;) {
		int c = getopt_long(ac,av,"c:sh",lopt,NULL);
		if (c == -1) break;
		switch (c) {
		default:
			usage(E_OPTERR);
		case 'h':
			usage(E_USAGE);
		case 'c':
			fname = optarg;
			break;
		case 's':
			use_syslog = 1;
			break;
		}
	}

	if (use_syslog)
		openlog("nameif",0,LOG_LOCAL0);

	while (optind < ac) {
		struct change *ch = xmalloc(sizeof(struct change));
		char pos[30];

		if ((ac-optind) & 1)
			usage(E_OPTERR);
		if (strlen(av[optind])+1>IFNAMSIZ)
			complain(_("interface name `%s' too long"), av[optind]);
		safe_strncpy(ch->ifname, av[optind], sizeof(ch->ifname));
		optind++;
		sprintf(pos,_("argument %d"),optind);
		addchange(av[optind], ch, pos);
		optind++;
	}

	if (!clist || fname != default_conf)
		readconf();

	ifh = fopen("/proc/net/dev", "r");
	if (!ifh)  complain(_("open of /proc/net/dev: %s"), strerror(errno));


	linenum = 0;
	while (getdelim(&line, &linel, '\n', ifh) > 0) {
		struct change *ch;
		unsigned char mac[MAC_ADDRESS_MAX_LENGTH];

		if (linenum++ < 2)
			continue;

		p = line;
		while (isspace(*p))
			++p;
		n = strcspn(p, ": \t");
		p[n] = 0;

		if (n > IFNAMSIZ-1)
			complain(_("interface name `%s' too long"), p);

		if (getmac(p, mac) < 0)
			continue;

		ch = lookupmac(mac);
		if (!ch)
			continue;

		ch->found = 1;
		if (strcmp(p, ch->ifname)) {
			if (setname(p, ch->ifname) < 0)
				complain(_("cannot change name of %s to %s: %s"),
						p, ch->ifname, strerror(errno));
		}
	}
	fclose(ifh);

	while (clist) {
		struct change *ch = clist;
		clist = clist->next;
		if (!ch->found){
			warning(_("interface '%s' not found"), ch->ifname);
			ret = 1;
		}
		free(ch);
	}

	if (use_syslog)
		closelog();
	return ret;
}
