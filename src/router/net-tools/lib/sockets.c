/* sockets.c. Rewriten by Andi Kleen. Subject to the GPL. */

/* philb 14/11/98: we now stash the socket file descriptor inside
   the `aftype' structure rather than keeping it in a pile of separate
   variables.  This is necessary so that "ifconfig eth0 broadcast ..."
   issues ioctls to the right socket for the address family in use;
   picking one at random doesn't always work.  */

#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>

#include "config.h"
#include "sockets.h"
#include "intl.h"
#include "util.h"
#include "net-support.h"

int skfd = -1;			/* generic raw socket desc.     */

int sockets_open(int family)
{
    struct aftype **aft;
    int sfd = -1;
    static int force = -1;

    if (force < 0) {
	force = 0;
	if (kernel_version() < KRELEASE(2, 1, 0))
	    force = 1;
	if (access("/proc/net", R_OK))
	    force = 1;
    }
    for (aft = aftypes; *aft; aft++) {
	struct aftype *af = *aft;
	int type = SOCK_DGRAM;
	if (af->af == AF_UNSPEC)
	    continue;
	if (family && family != af->af)
	    continue;
	if (af->fd != -1) {
	    sfd = af->fd;
	    continue;
	}
	/* Check some /proc file first to not stress kmod */
	if (!family && !force && af->flag_file) {
	    if (access(af->flag_file, R_OK))
		continue;
	}
#if HAVE_AFNETROM
	if (af->af == AF_NETROM)
	    type = SOCK_SEQPACKET;
#endif
#if HAVE_AFX25
       if (af->af == AF_X25)
           type = SOCK_SEQPACKET;
#endif
	af->fd = socket(af->af, type, 0);
	if (af->fd >= 0)
	    sfd = af->fd;
    }
    if (sfd < 0)
	fprintf(stderr, _("No usable address families found.\n"));
    return sfd;
}
