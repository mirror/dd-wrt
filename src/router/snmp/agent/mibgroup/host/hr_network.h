/*
 *  Host Resources MIB - network device group interface - hr_network.h
 *
 */
#ifndef _MIBGROUP_HRNET_H
#define _MIBGROUP_HRNET_H

extern void     init_hr_network(void);
extern FindVarMethod var_hrnet;

config_require(mibII/interfaces)
#endif                          /* _MIBGROUP_HRNET_H */
