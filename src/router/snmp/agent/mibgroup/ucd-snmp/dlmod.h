/*
 *  Dynamic Loadable Agent Modules MIB (UCD-DLMOD-MIB) - dlmod.h
 *
 */

#ifndef _MIBGROUP_DLMOD_H
#define _MIBGROUP_DLMOD_H

/*
 * TODO #include "mibdefs.h" 
 */

config_add_mib(UCD-DLMOD-MIB)
#ifndef SNMPDLMODPATH
#define SNMPDLMODPATH "/usr/local/lib/snmp/dlmod"
#endif
     struct dlmod {
         struct dlmod   *next;
         int             index;
         char            name[64 + 1];
         char            path[255 + 1];
         char            error[255 + 1];
         void           *handle;
         int             status;
     };

     void            dlmod_load_module(struct dlmod *);
     void            dlmod_unload_module(struct dlmod *);
     struct dlmod   *dlmod_create_module(void);
     void            dlmod_delete_module(struct dlmod *);
     struct dlmod   *dlmod_get_by_index(int);

     void            dlmod_init(void);
     void            dlmod_deinit(void);

     extern void     init_dlmod(void);
     extern void     deinit_dlmod(void);

     extern FindVarMethod var_dlmod;
     extern FindVarMethod var_dlmodEntry;
     extern WriteMethod write_dlmodName;
     extern WriteMethod write_dlmodPath;
     extern WriteMethod write_dlmodStatus;

#define DLMODNEXTINDEX 		1
#define DLMODINDEX     		2
#define DLMODNAME      		3
#define DLMODPATH      		4
#define DLMODERROR     		5
#define DLMODSTATUS    		6

#define DLMOD_LOADED		1
#define DLMOD_UNLOADED		2
#define DLMOD_ERROR		3
#define DLMOD_LOAD		4
#define	DLMOD_UNLOAD		5
#define DLMOD_CREATE		6
#define DLMOD_DELETE		7

#endif                          /* _MIBGROUP_DLMOD_H */
