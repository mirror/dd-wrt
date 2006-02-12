/*
 *  System MIB group interface - system.h
 *
 */
#ifndef _MIBGROUP_SYSTEM_MIB_H
#define _MIBGROUP_SYSTEM_MIB_H

config_require(util_funcs)

     extern char     version_descr[];

     void            init_system_mib(void);
     extern FindVarMethod var_system;

/*
 * config file parsing routines 
 */
     void            system_parse_config_sysloc(const char *, char *);
     void            system_parse_config_syscon(const char *, char *);
     void            system_parse_config_sysname(const char *, char *);

#define	VERSION_DESCR		1
#define	VERSIONID		2
#define	UPTIME			3
#define SYSCONTACT		4
#define SYSTEMNAME		5
#define SYSLOCATION		6
#define SYSSERVICES		7
#define SYSORLASTCHANGE		8

#endif                          /* _MIBGROUP_SYSTEM_MIB_H */
