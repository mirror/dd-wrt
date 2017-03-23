
#include <config.h>
#include <sys/types.h>
#include <sys/time.h>

#ifdef HAVE_SANDBOX_H
# pragma GCC diagnostic ignored "-Wdeprecated-declarations"
# include <sandbox.h>
#endif

#include "sandboxes.h"

int
sandboxes_app(void)
{
    return 0;
}

int
sandboxes_pidproc(void)
{
#ifdef HAVE_SANDBOX_INIT
    char *errmsg;

    if (sandbox_init != NULL &&
        sandbox_init(kSBXProfileNoNetwork, SANDBOX_NAMED, &errmsg) != 0) {
        return -1;
    }
#endif
    return 0;
}
