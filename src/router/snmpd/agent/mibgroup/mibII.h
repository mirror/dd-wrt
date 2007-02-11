/*
 * module to include the modules relavent to the mib-II mib(s) 
 */

config_require(mibII/system_mib)
config_require(mibII/sysORTable)
config_require(mibII/at)
config_require(mibII/ifTable)
config_require(mibII/ip)
config_require(mibII/snmp_mib)
config_require(mibII/tcp)
config_require(mibII/icmp)
config_require(mibII/udp)
config_require(mibII/vacm_vars)
config_require(mibII/setSerialNo)

#ifdef NETSNMP_INCLUDE_IFTABLE_REWRITES
config_require(if-mib)
#endif

/*
 * these new module re-rewrites have only been implemented for
 * linux.
 */
#if defined( linux )
config_require(ip-mib ip-forward-mib tcp-mib udp-mib)
#endif
