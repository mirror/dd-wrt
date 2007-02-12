/*
 *  Host Resources MIB - processor device group interface - hr_proc.h
 *
 */
#ifndef _MIBGROUP_HRPROC_H
#define _MIBGROUP_HRPROC_H

config_require(ucd-snmp/loadave)

extern void     init_hr_proc(void);
extern FindVarMethod var_hrproc;

#endif                          /* _MIBGROUP_HRPROC_H */
