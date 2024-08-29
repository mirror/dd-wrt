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
#include "util.h"

/* UGLY */

int IPX_rprint(int options)
{
    /* int ext = options & FLAG_EXT; */
    int numeric = options & FLAG_NUM_HOST;
    char buff[1024];
    char net[128], router_net[128];
    char router_node[128];
    int num;
    FILE *fp;
    const struct aftype *ap;
    struct sockaddr_storage sas;

    fp = fopen(_PATH_PROCNET_IPX_ROUTE1, "r");

    if (!fp) {
        fp = fopen(_PATH_PROCNET_IPX_ROUTE2, "r");
    }

    if (!fp) {
        perror(NULL);
        printf(_("IPX routing not in file %s or %s found.\n"), _PATH_PROCNET_IPX_ROUTE1, _PATH_PROCNET_IPX_ROUTE2);
	return 1;
    }

    if ((ap = get_afntype(AF_IPX)) == NULL) {
	EINTERN("lib/ipx_rt.c", "AF_IPX missing");
	return (-1);
    }

    printf(_("Kernel IPX routing table\n"));	/* xxx */
    printf(_("Destination               Router Net                Router Node\n"));

    if (fgets(buff, 1023, fp))
	/* eat line */;

    while (fgets(buff, 1023, fp)) {
	num = sscanf(buff, "%s %s %s", net, router_net, router_node);
	if (num < 3)
	    continue;

	/* Fetch and resolve the Destination */
	(void) ap->input(1, net, &sas);
	safe_strncpy(net, ap->sprint(&sas, numeric), sizeof(net));

	/* Fetch and resolve the Router Net */
	(void) ap->input(1, router_net, &sas);
	safe_strncpy(router_net, ap->sprint(&sas, numeric), sizeof(router_net));

	/* Fetch and resolve the Router Node */
	(void) ap->input(2, router_node, &sas);
	safe_strncpy(router_node, ap->sprint(&sas, numeric), sizeof(router_node));

	printf("%-25s %-25s %-25s\n", net, router_net, router_node);
    }

    (void) fclose(fp);
    return (0);
}

#endif				/* HAVE_AFIPX */
