#ifndef _NAMESERVICE_COMPAT
#define _NAMESERVICE_COMPAT

#include <sys/types.h>

#if !defined(__linux__) && !defined(__GLIBC__)
char *strndup(const char *ptr, size_t size);
#endif /* !defined(__linux__) && !defined(__GLIBC__) */

#endif /* _NAMESERVICE_COMPAT */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
