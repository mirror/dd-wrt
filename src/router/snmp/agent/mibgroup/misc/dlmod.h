/*
 *  pass: pass through extensiblity
 */
#ifndef _MIBGROUP_DLMOD_H
#define _MIBGROUP_DLMOD_H



#include "mibdefs.h"

#ifndef DLMOD_PATH
#define DLMOD_DEFAULT_PATH "/usr/local/lib/snmp/dlmod"
#endif

struct dlmod {
    struct dlmod   *next;
    int             index;
    char            name[64];
    char            path[255];
    char            error[255];
    void           *handle;
    int             status;
};

#define DLMOD_LOADED 	1
#define DLMOD_UNLOADED	2
#define DLMOD_ERROR		3
#define DLMOD_LOAD		4
#define	DLMOD_UNLOAD	5
#define DLMOD_CREATE	6
#define DLMOD_DELETE	7

void            dlmod_load_module(struct dlmod *);
void            dlmod_unload_module(struct dlmod *);
struct dlmod   *dlmod_create_module(void);
void            dlmod_delete_module(struct dlmod *);
struct dlmod   *dlmod_get_by_index(int);

void            dlmod_init(void);
void            dlmod_deinit(void);

extern int      dlmod_next_index;
#endif                          /* _MIBGROUP_DLMOD_H */
