#include <net-snmp/net-snmp-config.h>

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <stdio.h>
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <ctype.h>
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include <dlfcn.h>


#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "struct.h"

#include "dlmod.h"

static struct dlmod *dlmods = NULL;

int             dlmod_next_index = 1;

static void     dlmod_parse_config(char *, char *);
static void     dlmod_free_config(void);
static char     dlmod_path[1024];

void
init_dlmod(void)
{
    char           *p;
    int             l;

    snmpd_register_config_handler("dlmod", dlmod_parse_config,
                                  dlmod_free_config, "dlmod-file-to-load");

    p = getenv("SNMP_DLMOD_PATH");
    strncpy(dlmod_path, DLMOD_DEFAULT_PATH, sizeof(dlmod_path));
    if (p) {
        if (p[0] == '+') {
            l = strlen(dlmod_path);
            if (dlmod_path[l - 1] != ':')
                strncat(dlmod_path, ":", sizeof(dlmod_path) - l);
            strncat(dlmod_path, p + 1,
                    sizeof(dlmod_path) - strlen(dlmod_path));
        } else
            strncpy(dlmod_path, p, sizeof(dlmod_path));
    }
#if 1
    DEBUGMSGTL(("dlmod", "dlmod_path: %s\n", dlmod_path));
#endif
}

void
deinit_dlmod(void)
{
    snmpd_unregister_config_handler("dlmod");
}

static void
dlmod_parse_config(char *token, char *cptr)
{
    char           *dlm_name, *dlm_path;
    struct dlmod   *dlm;

    if (cptr == NULL) {
        config_perror("Bad dlmod line");
        return;
    }
    /*
     * remove comments 
     */
    *(cptr + strcspn(cptr, "#;\r\n")) = '\0';

    dlm = dlmod_create_module();
    if (!dlm)
        return;

    /*
     * dynamic module name 
     */
    dlm_name = strtok(cptr, "\t ");
    if (dlm_name == NULL) {
        config_perror("Bad dlmod line");
        dlmod_delete_module(dlm);
        return;
    }
    strncpy(dlm->name, dlm_name, sizeof(dlm->name));

    /*
     * dynamic module path 
     */
    dlm_path = strtok(NULL, "\t ");
    if (dlm_path)
        strncpy(dlm->path, dlm_path, sizeof(dlm->path));
    else
        strncpy(dlm->path, dlm_name, sizeof(dlm->path));

    dlmod_load_module(dlm);

    if (dlm->status == DLMOD_ERROR)
        snmp_log(LOG_ERR, "%s\n", dlm->error);

}

static void
dlmod_free_config(void)
{
    struct dlmod   *dtmp, *dtmp2;

    for (dtmp = dlmods; dtmp != NULL;) {
        dtmp2 = dtmp;
        dtmp = dtmp->next;
        dlmod_unload_module(dtmp2);
        free(dtmp2);
    }
    dlmods = NULL;
    dlmod_next_index = 1;
}

struct dlmod   *
dlmod_create_module(void)
{
    struct dlmod  **pdlmod, *dlm;
#if 1
    DEBUGMSGTL(("dlmod", "dlmod_create_module\n"));
#endif
    dlm = calloc(1, sizeof(struct dlmod));
    if (dlm == NULL)
        return NULL;

    dlm->index = dlmod_next_index++;
    dlm->status = DLMOD_UNLOADED;

    for (pdlmod = &dlmods; *pdlmod != NULL; pdlmod = &((*pdlmod)->next));
    (*pdlmod) = dlm;

    return dlm;
}

void
dlmod_delete_module(struct dlmod *dlm)
{
    struct dlmod  **pdlmod;

#if 1
    DEBUGMSGTL(("dlmod", "dlmod_delete_module\n"));
#endif
    if (!dlm || dlm->status != DLMOD_UNLOADED)
        return;

    for (pdlmod = &dlmods; *pdlmod; pdlmod = &((*pdlmod)->next))
        if (*pdlmod == dlm) {
            *pdlmod = dlm->next;
            free(dlm);
            return;
        }
}

void
dlmod_load_module(struct dlmod *dlm)
{
    char            sym_init[64];
    char           *p, tmp_path[255];
    int             (*dl_init) (void);
#if 1
    DEBUGMSGTL(("dlmod", "dlmod_load_module\n"));
#endif

    if (!dlm || !dlm->path || !dlm->name || dlm->status != DLMOD_UNLOADED)
        return;


    if (dlm->path[0] == '/') {
        dlm->handle = dlopen(dlm->path, RTLD_NOW);
        if (dlm->handle == NULL) {
            snprintf(dlm->error, sizeof(dlm->error),
                     "dlopen failed: %s", dlerror());
            dlm->status = DLMOD_ERROR;
            return;
        }
    } else {
        for (p = strtok(dlmod_path, ":"); p; p = strtok(NULL, ":")) {
            snprintf(tmp_path, sizeof(tmp_path), "%s/%s.so", p, dlm->path);
#if 1
            DEBUGMSGTL(("dlmod", "p: %s tmp_path: %s\n", p, tmp_path));
#endif
            dlm->handle = dlopen(tmp_path, RTLD_NOW);
            if (dlm->handle == NULL) {
                snprintf(dlm->error, sizeof(dlm->error),
                         "dlopen failed: %s", dlerror());
                dlm->status = DLMOD_ERROR;
            }
        }
        strncpy(dlm->path, tmp_path, sizeof(dlm->path));
        if (dlm->status == DLMOD_ERROR)
            return;
    }
    snprintf(sym_init, sizeof(sym_init), "_dynamic_init_%s", dlm->name);
    dl_init = dlsym(dlm->handle, sym_init);
    if (dl_init == NULL) {
        snprintf(dlm->error, sizeof(dlm->error),
                 "dlsym failed: can't find \'%s\'", sym_init);
        dlm->status = DLMOD_ERROR;
        return;
    }

    if (dl_init()) {
        snprintf(dlm->error, sizeof(dlm->error), "\'%s\' failed",
                 sym_init);
        dlm->status = DLMOD_ERROR;
        return;
    }

    dlm->error[0] = '\0';
    dlm->status = DLMOD_LOADED;
}

void
dlmod_unload_module(struct dlmod *dlm)
{
    char            sym_deinit[64];
    char            buf[256];
    int             (*dl_deinit) (void);

    if (!dlm || dlm->status != DLMOD_LOADED)
        return;

    snprintf(sym_deinit, sizeof(sym_deinit), "_dynamic_deinit_%s",
             dlm->name);
    dl_deinit = dlsym(dlm->handle, sym_deinit);
    if (dl_deinit == NULL) {
        /** it's right way ? */
        dlm->status = DLMOD_ERROR;
        return;
    }
    dl_deinit();
    dlclose(dlm->handle);
    dlm->status = DLMOD_UNLOADED;
#if 1
    DEBUGMSGTL(("dlmod", "Module %s unloaded\n", dlm->name));
#endif
}

int
dlmod_get_next_index(void)
{
    return dlmod_next_index;
}

struct dlmod   *
dlmod_get_by_index(int iindex)
{
    struct dlmod   *dlmod;

    for (dlmod = dlmods; dlmod; dlmod = dlmod->next)
        if (dlmod->index == iindex)
            return dlmod;

    return NULL;
}

#if 0
struct dlmod   *
dlmod_get_next(struct dlmod *dlm)
{
    if (dlm)
        return dlmods;
    else
        return dlm->next;
}
#endif
