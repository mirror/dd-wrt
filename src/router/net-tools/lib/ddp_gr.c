/*
 * lib/ddp_gr.c         Prinbting of DDP (AppleTalk) routing table
 *                      used by the NET-LIB.
 *
 * NET-LIB
 *
 * Version:     $Id: ddp_gr.c,v 1.4 2002/06/02 05:25:15 ecki Exp $
 *
 * Author:      Ajax <ajax@firest0rm.org>
 *
 * Modification:
 *  2002-06-02 integrated into main source by Bernd Eckenfels
 *
 */

/* TODO: name lookups (/etc/atalk.names?  NBP?) */

#include "config.h"

#if HAVE_AFATALK
#include <asm/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/atalk.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include "net-support.h"
#include "pathnames.h"
#include "intl.h"

/* stolen from inet_gr.c */
#define flags_decode(i,o) do {                  \
        o[0] = '\0';                            \
        if (i & RTF_UP) strcat(o, "U");         \
        if (i & RTF_GATEWAY) strcat(o, "G");    \
        if (i & RTF_REJECT) strcat(o, "!");     \
        if (i & RTF_HOST) strcat(o, "H");       \
        if (i & RTF_REINSTATE) strcat(o, "R");  \
        if (i & RTF_DYNAMIC) strcat(o, "D");    \
        if (i & RTF_MODIFIED) strcat(o, "M");   \
        if (i & RTF_DEFAULT) strcat(o, "d");    \
        if (i & RTF_ALLONLINK) strcat(o, "a");  \
        if (i & RTF_ADDRCONF) strcat(o, "c");   \
        if (i & RTF_NONEXTHOP) strcat(o, "o");  \
        if (i & RTF_EXPIRES) strcat(o, "e");    \
        if (i & RTF_CACHE) strcat(o, "c");      \
        if (i & RTF_FLOW) strcat(o, "f");       \
        if (i & RTF_POLICY) strcat(o, "p");     \
        if (i & RTF_LOCAL) strcat(o, "l");      \
        if (i & RTF_MTU) strcat(o, "u");        \
        if (i & RTF_WINDOW) strcat(o, "w");     \
        if (i & RTF_IRTT) strcat(o, "i");       \
        if (i & RTF_NOTCACHED) strcat(o, "n");  \
    } while (0)

int DDP_rprint(int options)
{
    FILE *fp;
    char *dest, *gw, *dev, *flags;
    char oflags[32];
    char *hdr = "Destination     Gateway         Device          Flags";

    fp = fopen(_PATH_PROCNET_ATALK_ROUTE, "r");

    if (!fp) {
        perror("Error opening " _PATH_PROCNET_ATALK_ROUTE);
        fprintf(stderr, "DDP (AppleTalk) not configured on this system.\n");
        return 1;
    }

    if (fscanf(fp, "%ms %ms %ms %ms\n", &dest, &gw, &flags, &dev))
		/* eat line */;
    free(dest); free(gw); free(flags); free(dev);

    printf("%s\n", hdr);

    while (fscanf(fp, "%ms %ms %ms %ms\n", &dest, &gw, &flags, &dev) == 4) {
        int iflags = atoi(flags);
        flags_decode(iflags, oflags);
        printf("%-16s%-16s%-16s%-s\n", dest, gw, dev, oflags);
        free(dest); free(gw); free(flags); free(dev);
    }

    fclose(fp);

    return 0;

}
#endif
