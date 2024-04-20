#include <compat.h>

#ifndef HAVE_STRCASECMP
#ifdef _MSC_VER
#define strcasecmp _stricmp
#else
#error No strcasecmp() implementation for this platform is available.
#endif
#endif

#ifndef HAVE_STRNCASECMP
#ifdef _MSC_VER
#define strncasecmp _strnicmp
#else
#error No strncasecmp() implementation for this platform is available.
#endif
#endif
