/*
 *  Template MIB group interface - ipAddr.h
 *
 */
#ifndef _MIBGROUP_IPADDR_H
#define _MIBGROUP_IPADDR_H

config_require(mibII/ip util_funcs)

     extern FindVarMethod var_ipAddrEntry;

#endif                          /* _MIBGROUP_IPADDR_H */
