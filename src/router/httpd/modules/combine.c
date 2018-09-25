#include <stdio.h>
#define BASEDEF 1
/* GoAhead 2.1 Embedded JavaScript compatibility */

#include "webs.h"
#include "nvramsr.c"
#include "cgi.c"
#include "ej.c"
#include "base.c"

#include "callvalidate.c"
#ifdef HAVE_UPX86
#include "upgrade_x86.c"
#else
#include "upgrade.c"
#endif
