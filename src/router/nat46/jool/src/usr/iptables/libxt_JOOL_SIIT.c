#include "common/iptables.h"

#define IPTABLES_MODULE_NAME IPTABLES_SIIT_MODULE_NAME
#define IPTABLES_MODULE_TYPE XT_SIIT
#define IPTABLES_MODULE_MAIN jool_siit_xtables_init
#include "common.c"
