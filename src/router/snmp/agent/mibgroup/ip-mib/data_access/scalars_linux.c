/*
 *  Arp MIB architecture support
 *
 * $Id: scalars_linux.c,v 1.3 2006/08/31 08:34:18 tanders Exp $
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include <net-snmp/data_access/ip_scalars.h>

const char *ipfw_name = "/proc/sys/net/ipv6/conf/all/forwarding";

int
netsnmp_arch_ip_scalars_ipv6IpForwarding_get(u_long *value)
{
    FILE *filep;
    int rc;

    if (NULL == value)
        return -1;


    filep = fopen(ipfw_name, "r");
    if (NULL == filep) {
        DEBUGMSGTL(("access:ipv6IpForwarding", "could not open %s\n",
                    ipfw_name));
        return -2;
    }

    rc = fscanf(filep, "%ld", value);
    fclose(filep);
    if (1 != rc) {
        DEBUGMSGTL(("access:ipv6IpForwarding", "could not read %s\n",
                    ipfw_name));
        return -3;
    }

    if ((0 != *value) && (1 != *value)) {
        DEBUGMSGTL(("access:ipv6IpForwarding", "unexpected value %ld in %s\n",
                    *value, ipfw_name));
        return -4;
    }

    return 0;
}

int
netsnmp_arch_ip_scalars_ipv6IpForwarding_set(u_long value)
{
    FILE *filep;
    int rc;

    if (1 == value)
        ;
    else if (2 == value)
        value = 0;
    else {
        DEBUGMSGTL(("access:ipv6IpForwarding", "bad value %ld for %s\n",
                    value));
        return SNMP_ERR_WRONGVALUE;
    }

    filep = fopen(ipfw_name, "w");
    if (NULL == filep) {
        DEBUGMSGTL(("access:ipv6IpForwarding", "could not open %s\n",
                    ipfw_name));
        return SNMP_ERR_RESOURCEUNAVAILABLE;
    }

    rc = fprintf(filep, "%ld", value);
    fclose(filep);
    if (1 != rc) {
        DEBUGMSGTL(("access:ipv6IpForwarding", "could not write %s\n",
                    ipfw_name));
        return SNMP_ERR_GENERR;
    }

    return 0;
}
