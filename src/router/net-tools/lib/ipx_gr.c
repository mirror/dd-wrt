/* support for ap->rresolv missing */
/*
   Modifications:
   1998-07-01 - Arnaldo Carvalho de Melo - GNU gettext instead of catgets,
   snprintf instead of sprintf
 */

#include "config.h"

#if HAVE_AFIPX
#include <asm/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#if (__GLIBC__ > 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 1)
#include <netipx/ipx.h>
#else
#include "ipx.h"
#endif
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include "version.h"
#include "net-support.h"
#include "pathnames.h"
#include "intl.h"

/* UGLY */

int IPX_rprint(int options)
{
    /* int ext = options & FLAG_EXT; */
    int numeric = options & FLAG_NUM_HOST;
    char buff[1024];
    char net[128], router_net[128];
    char router_node[128];
    int num;
    FILE *fp = fopen(_PATH_PROCNET_IPX_ROUTE, "r");
    struct aftype *ap;
    struct sockaddr sa;

    if ((ap = get_afntype(AF_IPX)) == NULL) {
	EINTERN("lib/ipx_rt.c", "AF_IPX missing");
	return (-1);
    }

    if (!fp) {
        perror(_PATH_PROCNET_IPX_ROUTE);
        printf(_("IPX not configured in this system.\n"));
	return 1;
    }

    printf(_("Kernel IPX routing table\n"));	/* xxx */
    printf(_("Destination               Router Net                Router Node\n"));

    fgets(buff, 1023, fp);

    while (fgets(buff, 1023, fp)) {
	num = sscanf(buff, "%s %s %s", net, router_net, router_node);
	if (num < 3)
	    continue;

	/* Fetch and resolve the Destination */
	(void) ap->input(5, net, &sa);
	strcpy(net, ap->sprint(&sa, numeric));

	/* Fetch and resolve the Router Net */
	(void) ap->input(5, router_net, &sa);
	strcpy(router_net, ap->sprint(&sa, numeric));

	/* Fetch and resolve the Router Node */
	(void) ap->input(2, router_node, &sa);
	strcpy(router_node, ap->sprint(&sa, numeric));

	printf("%-25s %-25s %-25s\n", net, router_net, router_node);
    }

    (void) fclose(fp);
    return (0);
}

#endif				/* HAVE_AFIPX */
