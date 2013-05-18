#include "rflow.h"
#include "disp.h"
#include "sf_lite.h"
#include "opt.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <malloc.h>

static int skfd = -1;

struct l_ifstat {
	/* Receive */
	unsigned long ibytes;
	unsigned long ipackets;
	unsigned long ierrs;
	unsigned long idrop;
	unsigned long ififo;
	unsigned long iframe;
	unsigned long icompr;
	unsigned long imcast;

	/* Transmit */
	unsigned long obytes;
	unsigned long opackets;
	unsigned long oerrs;
	unsigned long odrop;
	unsigned long ofifo;
	unsigned long ocolls;
	unsigned long ocarrier;
	unsigned long ocompr;
};

static FILE *pnd = NULL;	/* Pre-opened /proc/net/dev */
static int uptime_fd = -1;
static pthread_mutex_t uptime_mutex;

#define S_OFFSET2(type,f) ((ptrdiff_t)&(((type *)0)->f) - (ptrdiff_t)((type *)0))
#define S_OFFSET(f) S_OFFSET2(struct l_ifstat, f)

static const ptrdiff_t offset_table[] =
{
	S_OFFSET(ibytes), S_OFFSET(ipackets), S_OFFSET(ierrs), S_OFFSET(idrop),
	S_OFFSET(ififo), S_OFFSET(iframe), S_OFFSET(icompr), S_OFFSET(imcast),
	S_OFFSET(obytes), S_OFFSET(opackets), S_OFFSET(oerrs), S_OFFSET(odrop),
	S_OFFSET(ofifo), S_OFFSET(ocolls), S_OFFSET(ocarrier), S_OFFSET(ocompr)
};

int status(FILE *f, char *ifname);


char *
get_encaps(int encaps) {
	static char buf[32];

	switch(encaps) {
		case ARPHRD_ETHER:
			return "Ethernet";
		case ARPHRD_PPP:
			return "PPP";
		case ARPHRD_SLIP:
			return "SLIP";
		case ARPHRD_CSLIP:
			return "CSLIP";
		case ARPHRD_LOOPBACK:
			return "Loopback";
		case ARPHRD_HDLC:
			return "HDLC";
	}

	snprintf(buf, sizeof(buf), "Unknown/%d", encaps);
	return buf;
}

void
ether_status(FILE *f, struct sockaddr *sa) {
	fprintf(f, "  Hardware is %s, ", get_encaps(sa->sa_family));

#define EA(foo)	((unsigned char)*((unsigned char *)sa->sa_data + (foo)))
	fprintf(f, "address is %02x%02x.%02x%02x.%02x%02x",
		EA(0), EA(1),
		EA(2), EA(3),
		EA(4), EA(5)
	);

#ifdef	ENABLE_BIA_OUTPUT
	fprintf(f, " (bia %02x%02x.%02x%02x.%02x%02x",
		EA(0), EA(1),
		EA(2), EA(3),
		EA(4), EA(5)
	);
#endif

	fprintf(f, "\n");
}

int
if_stat(FILE *f, char *ifname) {
	struct ifreq ifr;
	int encaps = -1;

	if(skfd == -1) {
		skfd = socket(AF_INET, SOCK_DGRAM, 0);
		if(skfd == -1) {
			fprintf(f, "System error\n");
			return -1;
		}
	}

	if(strlen(ifname) >= sizeof(ifr.ifr_name)) {
		fprintf(f, "No such interface %s\n", ifname);
		return -1;
	}

	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	if(ioctl(skfd, SIOCGIFNAME, &ifr) == 0) {
		fprintf(f, "No such interface %s\n", ifname);
		return -1;
	} else {
		ifname = ifr.ifr_name;
	}

	if(ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) {
		fprintf(f, "No such interface %s\n", ifname);
		return -1;
	}

	fprintf(f, "%s is %s, line protocol is %s\n",
		ifname,
		((ifr.ifr_flags & IFF_RUNNING) ? "up" : "down"),
		((ifr.ifr_flags & IFF_UP) ? "up" : "down")
	);


	if(ioctl(skfd, SIOCGIFHWADDR, &ifr) == 0) {
		encaps = ifr.ifr_hwaddr.sa_family;
		if(encaps == ARPHRD_ETHER) {
			ether_status(f, &ifr.ifr_hwaddr);
		};
	}

	ifr.ifr_addr.sa_family = AF_INET;
	if(ioctl(skfd, SIOCGIFADDR, &ifr) == 0) {
		fprintf(f, "  Internet address is ");
		print_ip(f, ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);

		if(ioctl(skfd, SIOCGIFNETMASK, &ifr) == 0) {
			fprintf(f, " ");
			print_ip(f, ((struct sockaddr_in *)&ifr.ifr_netmask)->sin_addr);
		}
		fprintf(f, "\n");

		if(ioctl(skfd, SIOCGIFDSTADDR, &ifr) == 0) {
			fprintf(f, "  Peer IP address is ");
			print_ip(f, ((struct sockaddr_in *)&ifr.ifr_dstaddr)->sin_addr);
			fprintf(f, "\n");
		}

		if(ioctl(skfd, SIOCGIFBRDADDR, &ifr) == 0) {
			fprintf(f, "  IP broadcast address is ");
			print_ip(f, ((struct sockaddr_in *)&ifr.ifr_broadaddr)->sin_addr);
			fprintf(f, "\n");
		}
	}

	fprintf(f, "  Encapsulation %s, looback %s\n",
		get_encaps(encaps),
		((ifr.ifr_flags & IFF_LOOPBACK) ? "set": "not set")
	);

	if(ioctl(skfd, SIOCGIFMTU, &ifr) == 0)
		fprintf(f, "  MTU %u\n", ifr.ifr_mtu);

	display_internal_averages(f, ifname);

	status(f, ifname);

	return 0;
}


void
display_ifstat(FILE *f, struct l_ifstat *ifs) {

	fprintf(f, "     %lu packets input, %lu bytes, %lu no buffer\n",
		ifs->ipackets, ifs->ibytes, ifs->idrop);

	fprintf(f, "     %lu input errors, %lu CRC, %lu frame, %lu overrun, %lu ignored\n",
		ifs->ierrs, 0L, ifs->iframe, 0L, ifs->ierrs);

	fprintf(f, "     %lu packets output, %lu bytes, %lu underruns\n",
		ifs->opackets, ifs->obytes, 0L);
	fprintf(f, "     %lu output errors, %lu collisions, %lu interface resets\n",
		ifs->oerrs, ifs->ocolls, ifs->ocarrier);
	fprintf(f, "     %lu output drops\n", ifs->odrop);

	return;
};

int
status(FILE *f, char *ifname) {
	char buf[1024];

	/* Lock all */
	pthread_mutex_lock(&pndev_lock);

	if(!pnd) {
		if(ifst_preopen() == -1) {
			fprintf(f, "     Verbose statistics unavailable: indefined source\n");
			goto finish;
		}
	} else {
		fseek(pnd, 0L, 0);
	}

	/*
	 * Eat two header lines.
	 */
	fgets(buf, sizeof(buf), pnd); /* Eat first header line */
	if(fgets(buf, sizeof(buf), pnd) == NULL) {
		fprintf(f, "     Verbose statistics unavailable: "
			"invalid format\n");
		goto finish;
	}

	while(fgets(buf, sizeof(buf), pnd) != NULL) {
		char *token, *tmps = 0;
		char *s = buf;
		struct l_ifstat lfs;
		unsigned int i;

		while (*s == ' ') ++s;
		
		token = strtok_r(s, ": \t", &tmps);
		
		if(strcmp(token, ifname))
			continue;

		for (i=0; i < sizeof(offset_table)/sizeof(offset_table[0]); ++i)
		{
			if ((token = strtok_r(0, ": \t", &tmps)) == 0) 
				break;
			*((unsigned long *) (&lfs + offset_table[i])) = strtoul (token, 0, 10);
		}

		if(i >= sizeof(offset_table)/sizeof(offset_table[0])) {
			display_ifstat(f, &lfs);
		} else {
			fprintf(f, "     Verbose statistics unavailable: resource shortage\n");
			goto finish;
		}
		pthread_mutex_unlock(&pndev_lock);
		return 0;
	}

finish:

	pthread_mutex_unlock(&pndev_lock);
	return -1;
}

#ifndef	IPCAD_IFLIST_USE_GETIFADDRS

slist *
get_interface_names() {
	char buf[1024];
	slist *sl;

	if(!pnd) {
		errno = ESRCH;
		return NULL;
	}

	pthread_mutex_lock(&pndev_lock);
	fseek(pnd, 0L, 0);

	fgets(buf, sizeof(buf), pnd); /* Eat first header line */
	if(fgets(buf, sizeof(buf), pnd) == NULL) {
		pthread_mutex_unlock(&pndev_lock);
		return NULL;
	}

	sl = sinit();
	while(fgets(buf, sizeof(buf), pnd)) {
		char *ifname, *p;
		for(ifname = buf; *ifname && *ifname == ' '; ifname++);
		for(p = ifname; *p; p++) {
			switch(*p) {
			default: continue;
			case ':': case ' ': case '\t': case '|':
				if(sadd2(sl, ifname, p - ifname) == -1) {
					sfree(sl);
					sl = NULL;
					break;
				}
			}
			break;
		}
	}

	pthread_mutex_unlock(&pndev_lock);
	return sl;
}

#endif	/* IPCAD_IFLIST_USE_GETIFADDRS */

#ifndef	_PATH_PROCNET_DEV
#define	_PATH_PROCNET_DEV	"/proc/net/dev"
#endif

#ifndef	_PATH_PROC_UPTIME
#define	_PATH_PROC_UPTIME	"/proc/uptime"
#endif

int
ifst_preopen() {
	uptime_fd = open(_PATH_PROC_UPTIME, O_RDONLY);

	pnd = fopen(_PATH_PROCNET_DEV, "r");
	if(pnd)	setbuf(pnd, NULL);
	else	return -1;

	return 0;
}

static double
system_uptime_d() {
	char buf[256];
	double uptime;

	if(uptime_fd == -1)
		return 0.0;

	pthread_mutex_lock(&uptime_mutex);
	if(lseek(uptime_fd, 0L, SEEK_SET) == 0
		&& read(uptime_fd, buf, sizeof(buf)) > 0) {
		buf[sizeof(buf) - 1] = '\0';	/* Just in case */
		errno = 0;
		uptime = (time_t)strtod(buf, NULL);
		if(errno)
			uptime = 0.0;
	} else {
		uptime = 0.0;
	}
	pthread_mutex_unlock(&uptime_mutex);

	return uptime;
}

#ifndef	MAXHOSTNAMELEN
#define	MAXHOSTNAMELEN 256
#endif

void
system_uptime(FILE *f) {
	char buf[MAXHOSTNAMELEN];
	double uptime;

	uptime = system_uptime_d();
	if(uptime <= 0.0) return;

	gethostname(buf, sizeof(buf));
	buf[sizeof(buf) - 1] = '\0';

	fprintf(f, "%s uptime is", buf);
	display_uptime(f, uptime);
}

