#include "webs.h"
#include "nvramsr.c"
#include "cgi.c"
#include "base.c"
#include "callvalidate.c"
#include "ej.c"
#ifdef HAVE_UPX86
#include "upgrade_x86.c"
#else
#include "upgrade.c"
#endif
