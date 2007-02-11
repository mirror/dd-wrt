/*
 *  Host Resources MIB - system group interface - hr_system.h
 *
 */
#ifndef _MIBGROUP_HRSYSTEM_H
#define _MIBGROUP_HRSYSTEM_H

extern void     init_hr_system(void);
extern FindVarMethod var_hrsys;

#if defined(HAVE_MKTIME) && defined(HAVE_STIME)
int ns_set_time(int action, u_char * var_val, u_char var_val_type, size_t var_val_len, u_char * statP, oid * name, size_t name_len);
#endif

#endif                          /* _MIBGROUP_HRSYSTEM_H */
