#include <net-snmp/net-snmp-config.h>
#include <net-snmp/version.h>

/* expose ONLY for benefit of grandfathered code */
#ifndef UCD_COMPATIBLE
static
#endif
const char     *NetSnmpVersionInfo = "5.4";

const char     *
netsnmp_get_version()
{
    return NetSnmpVersionInfo;
}
