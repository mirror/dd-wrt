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

int DDP_rprint(int options)
{
    fprintf(stderr, _("Routing table for `ddp' not yet supported.\n"));
    return (1);
}
#endif
