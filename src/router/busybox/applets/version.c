#include "busybox.h"


#ifndef BB_EXTRA_VERSION
#define BB_BANNER "Now DD-WRT VeryBusyBox v" BB_VER " (" BB_BT ")"
#else
#define BB_BANNER "Now DD-WRT VeryBusyBox v" BB_VER " (" BB_EXTRA_VERSION ")"
#endif

const char BB_BANNER[]=BANNER;
const char * const bb_msg_full_version = BANNER " multi-call binary";
