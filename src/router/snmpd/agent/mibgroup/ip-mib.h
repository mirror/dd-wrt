/*
 * module to include the modules
 */

config_require(ip-mib/ipAddressTable);
config_require(ip-mib/inetNetToMediaTable);
config_require(ip-mib/ipSystemStatsTable);
config_require(ip-mib/ip_scalars);
config_add_mib(IP-MIB)
