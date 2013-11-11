#ifndef IPTRAF_NG_COMPAT_H
#define IPTRAF_NG_COMPAT_H

#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>
#include <curses.h>
#include <panel.h>
#include <assert.h>
#include <stddef.h>
#include <poll.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/un.h>

#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <netinet/ip_icmp.h>

#include <arpa/inet.h>

#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/if_fddi.h>
#include <linux/types.h>

#include <linux/if.h>
#include <linux/if_arp.h>

#ifndef ETH_P_8021AD
#define ETH_P_8021AD	0x88A8          /* 802.1ad Service VLAN		*/
#endif

#ifndef ETH_P_QINQ1
#define ETH_P_QINQ1	0x9100		/* deprecated QinQ VLAN [ NOT AN OFFICIALLY REGISTERED ID ] */
#endif

#ifndef ETH_P_QINQ2
#define ETH_P_QINQ2	0x9200		/* deprecated QinQ VLAN [ NOT AN OFFICIALLY REGISTERED ID ] */
#endif

#ifndef ETH_P_QINQ3
#define ETH_P_QINQ3	0x9300		/* deprecated QinQ VLAN [ NOT AN OFFICIALLY REGISTERED ID ] */
#endif

#define debug(...)							\
	do {								\
                fprintf(stderr, "%s:%s():%d:",				\
			__FILE__, __func__, __LINE__);			\
                fprintf(stderr, __VA_ARGS__);				\
                fprintf(stderr, "\n");					\
	} while(0)

#define KBITS 0

#define dispmode(mode)				\
	(((mode) == KBITS) ? "kbps": "kBps")

#define __noreturn	__attribute__((noreturn))
#define __unused	__attribute__((unused))
#define __printf(x, y)	__attribute__((format(printf, (x), (y))))

/* screen delay (in msecs) if update rate == 0 */
#define DEFAULT_UPDATE_DELAY 50

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define alloc_nr(x) (((x)+16)*3/2)

/*
 * Realloc the buffer pointed at by variable 'x' so that it can hold
 * at least 'nr' entries; the number of entries currently allocated
 * is 'alloc', using the standard growing factor alloc_nr() macro.
 *
 * DO NOT USE any expression with side-effect for 'x', 'nr', or 'alloc'.
 */
#define ALLOC_GROW(x, nr, alloc)					\
	do {								\
		if ((nr) > alloc) {					\
			if (alloc_nr(alloc) < (nr))			\
				alloc = (nr);				\
			else						\
				alloc = alloc_nr(alloc);		\
			x = xrealloc((x), alloc * sizeof(*(x)));	\
		}							\
	} while (0)

extern int daemonized;
extern int exitloop;

extern void *xmalloc(size_t size);
extern void *xcalloc(size_t nmemb, size_t size);
extern void *xrealloc(void *ptr, size_t size);
extern void *xmallocz(size_t size);
extern char *xstrdup(const char *s);
extern int strtoul_ui(char const *s, int base, unsigned int *result);
extern int strtol_i(char const *s, int base, int *result);

extern void die(const char *err, ...) __noreturn __printf(1,2);
extern void die_errno(const char *fmt, ...) __noreturn __printf(1,2);
extern void error(const char *err, ...) __printf(1,2);

static inline char *skip_whitespace(char *str)
{
	while (isspace(*str))
		++str;

	return str;
}

static inline unsigned long timeval_diff_msec(const struct timeval *end,
					      const struct timeval *start)
{
	if (!start || !end)
		return 0UL;

	signed long secs = end->tv_sec - start->tv_sec;
	signed long usecs = end->tv_usec - start->tv_usec;

	if(usecs < 0) {
		usecs = 1000000 - usecs;
		secs -= 1;
	}
	if(secs >= 0)
		return secs * 1000UL + usecs / 1000UL;
	else
		return 0UL;
}

#endif	/* IPTRAF_NG_COMPAT_H */
