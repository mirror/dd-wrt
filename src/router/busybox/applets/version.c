#include "busybox.h"

#ifdef HAVE_BRANDING


#ifndef BB_EXTRA_VERSION
#define BANNER "VeryBusyBox v" BB_VER " (" BB_BT ")"
#else
#define BANNER "VeryBusyBox v" BB_VER " (" BB_EXTRA_VERSION ")"
#endif

#else

#ifndef BB_EXTRA_VERSION
#define BANNER "DD-WRT VeryBusyBox v" BB_VER " (" BB_BT ")"
#else
#define BANNER "DD-WRT VeryBusyBox v" BB_VER " (" BB_EXTRA_VERSION ")"
#endif

##endif


const char BB_BANNER[]=BANNER;
const char * const bb_msg_full_version = BANNER " multi-call binary";
