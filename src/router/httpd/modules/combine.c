#include <stdio.h>
#define BASEDEF 1
/* GoAhead 2.1 Embedded JavaScript compatibility */

#include "webs.h"
#include "nvramsr.c"
#ifdef HAVE_WIREGUARD
#include "wireguard.c"
#endif
#include "cgi.c"
#include "ej.c"
#include "base.c"

#ifdef STATIC_BUILD
#include "callvalidate_static.c"
#else
#include "callvalidate.c"
#endif
#ifdef HAVE_UPX86
#include "upgrade_x86.c"
#else
#include "upgrade.c"
#endif
