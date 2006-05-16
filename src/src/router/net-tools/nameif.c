/* 
 * Name Interfaces based on MAC address.
 * Writen 2000 by Andi Kleen.
 * Subject to the Gnu Public License, version 2.  
 * TODO: make it support token ring etc.
 * $Id: nameif.c,v 1.1 2000/10/18 17:26:29 ak Exp $
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

const char default_conf[] = "/etc/mactab"; 
const char *fname = default_conf; 
int use_syslog; 
int ctl_sk = -1; 

void err(char *msg) 
{ 
	if (use_syslog) { 
		syslog(LOG_ERR,"%s: %m", msg); 
	} else { 
		perror(msg); 
	} 
	exit(1); 
}

void complain(char *fmt, ...) 
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

void warning(char *fmt, ...) 
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

int parsemac(char *str, unsigned char *mac)
{ 
	char *s; 
	while ((s = strsep(&str, ":")) != NULL) { 
		unsigned byte;
		if (sscanf(s,"%x", &byte)!=1 || byte > 0xff) 
			return -1;
		*mac++ = byte; 
	}  
	return 0;
} 

void *xmalloc(unsigned sz)
{ 
	void *p = calloc(sz,1);
	if (!p) errno=ENOMEM, err("xmalloc"); 
	return p; 
} 

void opensock(void)
{
	if (ctl_sk < 0) 
		ctl_sk = socket(PF_INET,SOCK_DGRAM,0); 
}

#ifndef ifr_newname
#define ifr_newname ifr_ifru.ifru_slave
#endif

int  setname(char *oldname, char *newname)
{
	struct ifreq ifr;
	opensock(); 
	memset(&ifr,0,sizeof(struct ifreq));
	strcpy(ifr.ifr_name, oldname); 
	strcpy(ifr.ifr_newname, newname); 
	return ioctl(ctl_sk, SIOCSIFNAME, &ifr);
}

int getmac(char *name, unsigned char *mac)
{
	int r;
	struct ifreq ifr;
	opensock(); 
	memset(&ifr,0,sizeof(struct ifreq));
	strcpy(ifr.ifr_name, name); 
	r = ioctl(ctl_sk, SIOCGIFHWADDR, &ifr);
	memcpy(mac, ifr.ifr_hwaddr.sa_data, 6); 
	return r; 
}

struct change { 
	struct change *next,**pprev;
	char ifname[IFNAMSIZ+1];
	unsigned char mac[6];
}; 
struct change *clist;

struct change *lookupmac(unsigned char *mac) 
{ 
	struct change *ch;
	for (ch = clist;ch;ch = ch->next) 
		if (!memcmp(ch->mac, mac, 6))
			return ch;
	return NULL; 
} 

int addchange(char *p, struct change *ch, char *pos)
{
	if (strchr(ch->ifname, ':'))
		warning(_("alias device %s at %s probably has no mac"), 
			ch->ifname, pos); 
	if (parsemac(p,ch->mac) < 0) 
		complain(_("cannot parse MAC `%s' at %s"), p, pos); 
	if (clist) 
		clist->pprev = &ch->next;
	ch->next = clist;
	ch->pprev = &clist;
	clist = ch;
	return 0; 
}

void readconf(void)
{
	char *line; 
	size_t linel; 
	int linenum; 
	FILE *ifh;
	char *p;
	int n;

	ifh = fopen(fname, "r");
	if (!ifh) 
		complain(_("opening configuration file %s: %s"),fname,strerror(errno)); 

	line = NULL; 
	linel = 0;
	linenum = 1; 
	while (getdelim(&line, &linel, '\n', ifh) > 0) {
		struct change *ch = xmalloc(sizeof(struct change)); 
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
		if (n > IFNAMSIZ) 
			complain(_("interface name too long at line %d"), line);  
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

struct option lopt[] = { 
	{"syslog", 0, NULL, 's' },
	{"config-file", 1, NULL, 'c' },
	{"help", 0, NULL, '?' }, 
	{NULL}, 
}; 

void usage(void)
{
	fprintf(stderr, _("usage: nameif [-c configurationfile] [-s] {ifname macaddress}")); 
	exit(1); 
}

int main(int ac, char **av) 
{ 
	FILE *ifh; 
	char *p;
	int n;
	int linenum; 
	char *line = NULL;
	size_t linel = 0;

	for (;;) {
		int c = getopt_long(ac,av,"c:s",lopt,NULL);
		if (c == -1) break;
		switch (c) { 
		default:
		case '?':
			usage(); 
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
			usage();
		if (strlen(av[optind])+1>IFNAMSIZ) 
			complain(_("interface name `%s' too long"), av[optind]);
		strcpy(ch->ifname, av[optind]); 
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
		unsigned char mac[6];

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
			
		*ch->pprev = ch->next;
		if (strcmp(p, ch->ifname)) { 
			if (setname(p, ch->ifname) < 0)  
				complain(_("cannot change name of %s to %s: %s"),
						p, ch->ifname, strerror(errno)); 
		} 
		free(ch);
	} 
	fclose(ifh); 
	
	while (clist) { 
		struct change *ch = clist;
		clist = clist->next;
		warning(_("interface '%s' not found"), ch->ifname); 
		free(ch); 
	}

	if (use_syslog)
		closelog();
	return 0;
} 

