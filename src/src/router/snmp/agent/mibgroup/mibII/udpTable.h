/*
 *  Template MIB group interface - udp.h
 *
 */
#ifndef _MIBGROUP_UDPTABLE_H
#define _MIBGROUP_UDPTABLE_H

config_arch_require(solaris2, kernel_sunos5)
config_require(mibII/ip util_funcs)

     extern FindVarMethod var_udpEntry;

#endif                          /* _MIBGROUP_UDPTABLE_H */
