#include "common/iptables.h"

#define IPTABLES_MODULE_NAME IPTABLES_NAT64_MODULE_NAME
#define IPTABLES_MODULE_TYPE XT_NAT64
#define IPTABLES_MODULE_MAIN jool_nat64_xtables_init
#include "common.c"
