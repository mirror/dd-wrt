/*
   Modifications:
   1998-07-01 - Arnaldo Carvalho de Melo - GNU gettext instead of catgets
 */

#include "config.h"

#if HAVE_AFINET6
#include <asm/types.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <resolv.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#ifdef __GLIBC__
#include <net/route.h>
#else
#include <netinet6/ipv6_route.h>	/* glibc does not have this */
#endif
#include "version.h"
#include "net-support.h"
#include "pathnames.h"
#include "intl.h"
#include "net-features.h"



extern struct aftype inet6_aftype;

static int skfd = -1;


static int usage(void)
{
    fprintf(stderr, _("Usage: inet6_route [-vF] del Target\n"));
    fprintf(stderr, _("       inet6_route [-vF] add Target [gw Gw] [metric M] [[dev] If]\n"));
    fprintf(stderr, _("       inet6_route [-FC] flush      NOT supported\n"));
    return (E_USAGE);
}


static int INET6_setroute(int action, int options, char **args)
{
    struct in6_rtmsg rt;
    struct ifreq ifr;
    struct sockaddr_in6 sa6;
    char target[128], gateway[128] = "NONE";
    int metric, prefix_len;
    char *devname = NULL;
    char *cp;

    if (*args == NULL)
	return (usage());

    strcpy(target, *args++);
    if (!strcmp(target, "default")) {
        prefix_len = 0;
	memset(&sa6, 0, sizeof(sa6));
    } else {
        if ((cp = strchr(target, '/'))) {
	    prefix_len = atol(cp + 1);
	    if ((prefix_len < 0) || (prefix_len > 128))
		usage();
	    *cp = 0;
	} else {
	    prefix_len = 128;
	}
	if (inet6_aftype.input(1, target, (struct sockaddr *) &sa6) < 0
	    && inet6_aftype.input(0, target, (struct sockaddr *) &sa6) < 0) {
	    inet6_aftype.herror(target);
	    return (1);
	}
    }

    /* Clean out the RTREQ structure. */
    memset((char *) &rt, 0, sizeof(struct in6_rtmsg));

    memcpy(&rt.rtmsg_dst, sa6.sin6_addr.s6_addr, sizeof(struct in6_addr));

    /* Fill in the other fields. */
    rt.rtmsg_flags = RTF_UP;
    if (prefix_len == 128)
	rt.rtmsg_flags |= RTF_HOST;
    rt.rtmsg_metric = 1;
    rt.rtmsg_dst_len = prefix_len;

    while (*args) {
	if (!strcmp(*args, "metric")) {

	    args++;
	    if (!*args || !isdigit(**args))
		return (usage());
	    metric = atoi(*args);
	    rt.rtmsg_metric = metric;
	    args++;
	    continue;
	}
	if (!strcmp(*args, "gw") || !strcmp(*args, "gateway")) {
	    args++;
	    if (!*args)
		return (usage());
	    if (rt.rtmsg_flags & RTF_GATEWAY)
		return (usage());
	    strcpy(gateway, *args);
	    if (inet6_aftype.input(1, gateway,
				   (struct sockaddr *) &sa6) < 0) {
		inet6_aftype.herror(gateway);
		return (E_LOOKUP);
	    }
	    memcpy(&rt.rtmsg_gateway, sa6.sin6_addr.s6_addr,
		   sizeof(struct in6_addr));
	    rt.rtmsg_flags |= RTF_GATEWAY;
	    args++;
	    continue;
	}
	if (!strcmp(*args, "mod")) {
	    args++;
	    rt.rtmsg_flags |= RTF_MODIFIED;
	    continue;
	}
	if (!strcmp(*args, "dyn")) {
	    args++;
	    rt.rtmsg_flags |= RTF_DYNAMIC;
	    continue;
	}
	if (!strcmp(*args, "device") || !strcmp(*args, "dev")) {
	    args++;
	    if (!*args)
		return (usage());
	} else if (args[1])
	    return (usage());

	devname = *args;
	args++;
    }

    /* Create a socket to the INET6 kernel. */
    if ((skfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
	perror("socket");
	return (E_SOCK);
    }
    if (devname) {
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, devname);

	if (ioctl(skfd, SIOGIFINDEX, &ifr) < 0) {
	    perror("SIOGIFINDEX");
	    return (E_SOCK);
	}
	rt.rtmsg_ifindex = ifr.ifr_ifindex;
    } else
	rt.rtmsg_ifindex = 0;

    /* Tell the kernel to accept this route. */
    if (action == RTACTION_DEL) {
	if (ioctl(skfd, SIOCDELRT, &rt) < 0) {
	    perror("SIOCDELRT");
	    close(skfd);
	    return (E_SOCK);
	}
    } else {
	if (ioctl(skfd, SIOCADDRT, &rt) < 0) {
	    perror("SIOCADDRT");
	    close(skfd);
	    return (E_SOCK);
	}
    }

    /* Close the socket. */
    (void) close(skfd);
    return (0);
}

int INET6_rinput(int action, int options, char **args)
{
    if (action == RTACTION_FLUSH) {
	fprintf(stderr, _("Flushing `inet6' routing table not supported\n"));
	return (usage());
    }
    if ((*args == NULL) || (action == RTACTION_HELP))
	return (usage());

    return (INET6_setroute(action, options, args));
}
#endif				/* HAVE_AFINET6 */
