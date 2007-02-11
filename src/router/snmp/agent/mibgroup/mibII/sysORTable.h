/*
 *  Template MIB group interface - sysORTable.h
 *
 */
#ifndef _MIBGROUP_SYSORTABLE_H
#define _MIBGROUP_SYSORTABLE_H

config_require(util_funcs)
config_require(mibII/system_mib)

     struct sysORTable {
         char           *OR_descr;
         oid            *OR_oid;
         size_t          OR_oidlen;
         struct timeval  OR_uptime;
         netsnmp_session *OR_sess;
         struct sysORTable *next;
     };

     struct register_sysOR_parameters {
         oid            *name;
         int             namelen;
         const char     *descr;
     };

     extern void     init_sysORTable(void);
     extern FindVarMethod var_sysORTable;
     extern FindVarMethod var_sysORLastChange;
     extern int      register_sysORTable(oid *, size_t, const char *);
     extern int      unregister_sysORTable(oid *, size_t);
     extern int      register_sysORTable_sess(oid *, size_t, const char *,
                                              netsnmp_session *);
     extern int      unregister_sysORTable_sess(oid *, size_t,
                                                netsnmp_session *);
     extern void     unregister_sysORTable_by_session(netsnmp_session *);

#define	SYSORTABLEINDEX		        1
#define	SYSORTABLEID		        2
#define	SYSORTABLEDESCR		        3
#define	SYSORTABLEUPTIME	        4

#define SYS_ORTABLE_REGISTERED_OK              0
#define SYS_ORTABLE_REGISTRATION_FAILED       -1
#define SYS_ORTABLE_UNREGISTERED_OK            0
#define SYS_ORTABLE_NO_SUCH_REGISTRATION      -1

#ifdef  USING_MIBII_SYSORTABLE_MODULE
#define REGISTER_SYSOR_ENTRY(theoid, descr)                      \
  (void)register_sysORTable(theoid, sizeof(theoid)/sizeof(oid), descr);
#define REGISTER_SYSOR_TABLE(theoid, len, descr)                      \
  (void)register_sysORTable(theoid, len, descr);

#else
#define REGISTER_SYSOR_ENTRY(x,y)
#define REGISTER_SYSOR_TABLE(x,y,z)
#endif                          /* USING_MIBII_SYSORTABLE_MODULE */
#endif                          /* _MIBGROUP_SYSORTABLE_H */
