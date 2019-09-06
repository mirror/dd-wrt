#ifndef __BACKPORT_CACHE_H
#define __BACKPORT_CACHE_H
#include_next <linux/cache.h>

#ifndef __ro_after_init
#define __ro_after_init
#endif

#endif
