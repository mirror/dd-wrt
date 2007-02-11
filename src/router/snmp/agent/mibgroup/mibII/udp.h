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
extern Netsnmp_Node_Handler udp_handler;
extern NetsnmpCacheLoad udp_load;
extern NetsnmpCacheFree udp_free;


#define UDPINDATAGRAMS	    1
#define UDPNOPORTS	    2
#define UDPINERRORS	    3
#define UDPOUTDATAGRAMS     4

#endif                          /* _MIBGROUP_UDP_H */
