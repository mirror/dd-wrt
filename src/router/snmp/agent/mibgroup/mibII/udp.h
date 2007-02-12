/*
 *  Template MIB group interface - udp.h
 *
 */
#ifndef _MIBGROUP_UDP_H
#define _MIBGROUP_UDP_H


config_require(mibII/udpTable util_funcs)
config_arch_require(solaris2, kernel_sunos5)
config_arch_require(linux, mibII/kernel_linux)

extern void     init_udp(void);
extern FindVarMethod var_udp;


#define UDPINDATAGRAMS	    0
#define UDPNOPORTS	    1
#define UDPINERRORS	    2
#define UDPOUTDATAGRAMS     3
#define UDPLOCALADDRESS     4
#define UDPLOCALPORT	    5

#endif                          /* _MIBGROUP_UDP_H */
