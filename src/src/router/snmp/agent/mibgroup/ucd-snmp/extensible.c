#include <net-snmp/net-snmp-config.h>

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#if TIME_WITH_SYS_TIME
# ifdef WIN32
#  include <sys/timeb.h>
# else
#  include <sys/time.h>
# endif
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#include <signal.h>
#if HAVE_MACHINE_PARAM_H
#include <machine/param.h>
#endif
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#if HAVE_SYS_VMMETER_H
#if !(defined(bsdi2) || defined(netbsd1))
#include <sys/vmmeter.h>
#endif
#endif
#if HAVE_SYS_CONF_H
#include <sys/conf.h>
#endif
#if HAVE_ASM_PAGE_H
#include <asm/page.h>
#endif
#if HAVE_SYS_SWAP_H
#include <sys/swap.h>
#endif
#if HAVE_SYS_FS_H
#include <sys/fs.h>
#else
#if HAVE_UFS_FS_H
#include <ufs/fs.h>
#else
#if HAVE_UFS_UFS_DINODE_H
#include <ufs/ufs/dinode.h>
#endif
#if HAVE_UFS_FFS_FS_H
#include <ufs/ffs/fs.h>
#endif
#endif
#endif
#if HAVE_MTAB_H
#include <mtab.h>
#endif
#include <sys/stat.h>
#include <errno.h>
#if HAVE_FSTAB_H
#include <fstab.h>
#endif
#if HAVE_SYS_STATFS_H
#include <sys/statfs.h>
#endif
#if HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#endif
#if HAVE_SYS_VFS_H
#include <sys/vfs.h>
#endif
#if (!defined(HAVE_STATVFS)) && defined(HAVE_STATFS)
#if HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
#if HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif
#define statvfs statfs
#endif
#if HAVE_VM_VM_H
#include <vm/vm.h>
#endif
#if HAVE_VM_SWAP_PAGER_H
#include <vm/swap_pager.h>
#endif
#if HAVE_SYS_FIXPOINT_H
#include <sys/fixpoint.h>
#endif
#if HAVE_MALLOC_H
#include <malloc.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#endif
#include <ctype.h>
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#ifndef HAVE_STRNCASECMP
int             strncasecmp(const char *s1, const char *s2, size_t n);
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/auto_nlist.h>

#include "struct.h"
#include "extensible.h"
#include "util_funcs.h"

extern struct myproc *procwatch;        /* moved to proc.c */
extern int      numprocs;       /* ditto */
extern struct extensible *extens;       /* In exec.c */
extern struct extensible *relocs;       /* In exec.c */
extern int      numextens;      /* ditto */
extern int      numrelocs;      /* ditto */
extern struct extensible *passthrus;    /* In pass.c */
extern int      numpassthrus;   /* ditto */
extern char     sysName[];
extern netsnmp_subtree *subtrees;
extern struct variable2 extensible_relocatable_variables[];
extern struct variable2 extensible_passthru_variables[];

void
init_extensible(void)
{

    struct variable2 extensible_extensible_variables[] = {
        {MIBINDEX, ASN_INTEGER, RONLY, var_extensible_shell, 1,
         {MIBINDEX}},
        {ERRORNAME, ASN_OCTET_STR, RONLY, var_extensible_shell, 1,
         {ERRORNAME}},
        {SHELLCOMMAND, ASN_OCTET_STR, RONLY, var_extensible_shell, 1,
         {SHELLCOMMAND}},
        {ERRORFLAG, ASN_INTEGER, RONLY, var_extensible_shell, 1,
         {ERRORFLAG}},
        {ERRORMSG, ASN_OCTET_STR, RONLY, var_extensible_shell, 1,
         {ERRORMSG}},
        {ERRORFIX, ASN_INTEGER, RWRITE, var_extensible_shell, 1,
         {ERRORFIX}},
        {ERRORFIXCMD, ASN_OCTET_STR, RONLY, var_extensible_shell, 1,
         {ERRORFIXCMD}}
    };

    /*
     * Define the OID pointer to the top of the mib tree that we're
     * registering underneath 
     */
    oid             extensible_variables_oid[] =
        { UCDAVIS_MIB, SHELLMIBNUM, 1 };

    /*
     * register ourselves with the agent to handle our mib tree 
     */
    REGISTER_MIB("ucd-snmp/extensible", extensible_extensible_variables,
                 variable2, extensible_variables_oid);

    snmpd_register_config_handler("exec", extensible_parse_config,
                                  extensible_free_config,
                                  "[miboid] name program arguments");
    snmpd_register_config_handler("sh", extensible_parse_config,
                                  extensible_free_config,
                                  "[miboid] name program-or-script arguments");
    snmpd_register_config_handler("execfix", execfix_parse_config, NULL,
                                  "exec-or-sh-name program [arguments...]");
}

void
extensible_parse_config(const char *token, char *cptr)
{
    struct extensible *ptmp, **pp;
    char           *tcptr;

    /*
     * allocate and clear memory structure 
     */
    ptmp = (struct extensible *) calloc(1, sizeof(struct extensible));
    if (ptmp == NULL)
        return;                 /* XXX memory alloc error */

    if (*cptr == '.')
        cptr++;
    if (isdigit(*cptr)) {
        /*
         * its a relocatable extensible mib 
         */
        for (pp = &relocs, numrelocs++; *pp; pp = &((*pp)->next));
        (*pp) = ptmp;
    } else {
        /*
         * it goes in with the general extensible table 
         */
        for (pp = &extens, numextens++; *pp; pp = &((*pp)->next));
        (*pp) = ptmp;
    }

    /*
     * the rest is pretty much handled the same 
     */
    if (!strncasecmp(token, "sh", 2))
        ptmp->type = SHPROC;
    else
        ptmp->type = EXECPROC;
    if (isdigit(*cptr)) {
        ptmp->miblen = parse_miboid(cptr, ptmp->miboid);
        while (isdigit(*cptr) || *cptr == '.')
            cptr++;
    }

    /*
     * name 
     */
    cptr = skip_white(cptr);
    copy_nword(cptr, ptmp->name, sizeof(ptmp->name));
    cptr = skip_not_white(cptr);
    cptr = skip_white(cptr);
    /*
     * command 
     */
    if (cptr == NULL) {
        config_perror("No command specified on line");
    } else {
        for (tcptr = cptr; *tcptr != 0 && *tcptr != '#' && *tcptr != ';';
             tcptr++);
        strncpy(ptmp->command, cptr, tcptr - cptr);
        ptmp->command[tcptr - cptr] = 0;
    }
#ifdef EXECFIXCMD
    sprintf(ptmp->fixcmd, EXECFIXCMD, ptmp->name);
#endif
    if (ptmp->miblen > 0) {
        register_mib(token,
                     (struct variable *) extensible_relocatable_variables,
                     sizeof(struct variable2), 6, ptmp->miboid,
                     ptmp->miblen);
    }
}

void
extensible_free_config(void)
{
    struct extensible *etmp, *etmp2;

    for (etmp = extens; etmp != NULL;) {
        etmp2 = etmp;
        etmp = etmp->next;
        free(etmp2);
    }

    for (etmp = relocs; etmp != NULL;) {
        etmp2 = etmp;
        etmp = etmp->next;
        unregister_mib(etmp2->miboid, etmp2->miblen);
        free(etmp2);
    }

    relocs = NULL;
    extens = NULL;
    numextens = 0;
    numrelocs = 0;
}


struct extensible *
get_exten_instance(struct extensible *exten, size_t inst)
{
    int             i;

    if (exten == NULL)
        return (NULL);
    for (i = 1; i != (int) inst && exten != NULL; i++)
        exten = exten->next;
    return (exten);
}

#define MAXMSGLINES 1000

struct extensible *extens = NULL;       /* In exec.c */
struct extensible *relocs = NULL;       /* In exec.c */
int             numextens = 0, numrelocs = 0;   /* ditto */


/*
 * var_extensible_shell(...
 * Arguments:
 * vp     IN      - pointer to variable entry that points here
 * name    IN/OUT  - IN/name requested, OUT/name found
 * length  IN/OUT  - length of IN/OUT oid's 
 * exact   IN      - TRUE if an exact match was requested
 * var_len OUT     - length of variable or 0 if function returned
 * write_method
 * 
 */

/*
 * find a give entry in the linked list associated with a proc name 
 */
struct extensible *
get_exec_by_name(char *name)
{
    struct extensible *etmp;

    if (name == NULL)
        return NULL;

    for (etmp = extens; etmp != NULL && strcmp(etmp->name, name) != 0;
         etmp = etmp->next);
    return etmp;
}

void
execfix_parse_config(const char *token, char *cptr)
{
    char            tmpname[STRMAX];
    struct extensible *execp;

    /*
     * don't allow two entries with the same name 
     */
    cptr = copy_nword(cptr, tmpname, sizeof(tmpname));
    if ((execp = get_exec_by_name(tmpname)) == NULL) {
        config_perror("No exec entry registered for this exec name yet.");
        return;
    }

    if (strlen(cptr) > sizeof(execp->fixcmd)) {
        config_perror("fix command too long.");
        return;
    }

    strncpy(execp->fixcmd, cptr, sizeof(execp->fixcmd));
    execp->fixcmd[ sizeof(execp->fixcmd)-1 ] = 0;
}

u_char         *
var_extensible_shell(struct variable * vp,
                     oid * name,
                     size_t * length,
                     int exact,
                     size_t * var_len, WriteMethod ** write_method)
{

    static struct extensible *exten = 0;
    static long     long_ret;

    if (header_simple_table
        (vp, name, length, exact, var_len, write_method, numextens))
        return (NULL);

    if ((exten = get_exten_instance(extens, name[*length - 1]))) {
        switch (vp->magic) {
        case MIBINDEX:
            long_ret = name[*length - 1];
            return ((u_char *) (&long_ret));
        case ERRORNAME:        /* name defined in config file */
            *var_len = strlen(exten->name);
            return ((u_char *) (exten->name));
        case SHELLCOMMAND:
            *var_len = strlen(exten->command);
            return ((u_char *) (exten->command));
        case ERRORFLAG:        /* return code from the process */
            if (exten->type == EXECPROC)
                exec_command(exten);
            else
                shell_command(exten);
            long_ret = exten->result;
            return ((u_char *) (&long_ret));
        case ERRORMSG:         /* first line of text returned from the process */
            if (exten->type == EXECPROC)
                exec_command(exten);
            else
                shell_command(exten);
            *var_len = strlen(exten->output);
            if (exten->output[*var_len - 1] == '\n')
                exten->output[--(*var_len)] = '\0';
            return ((u_char *) (exten->output));
        case ERRORFIX:
            *write_method = fixExecError;
            long_return = 0;
            return ((u_char *) & long_return);

        case ERRORFIXCMD:
            *var_len = strlen(exten->fixcmd);
            return ((u_char *) exten->fixcmd);
        }
        return NULL;
    }
    return NULL;
}

int
fixExecError(int action,
             u_char * var_val,
             u_char var_val_type,
             size_t var_val_len,
             u_char * statP, oid * name, size_t name_len)
{

    struct extensible *exten;
    long            tmp = 0;
    int             fd;
    static struct extensible ex;
    FILE           *file;

    if ((exten = get_exten_instance(extens, name[10]))) {
        if (var_val_type != ASN_INTEGER) {
            snmp_log(LOG_ERR, "Wrong type != int\n");
            return SNMP_ERR_WRONGTYPE;
        }
        tmp = *((long *) var_val);
        if ((tmp == 1) && (action == COMMIT) && (exten->fixcmd[0] != 0)) {
            sprintf(ex.command, exten->fixcmd);
            if ((fd = get_exec_output(&ex)) != -1) {
                file = fdopen(fd, "r");
                while (fgets(ex.output, sizeof(ex.output), file) != NULL);
                fclose(file);
                wait_on_exec(&ex);
            }
        }
        return SNMP_ERR_NOERROR;
    }
    return SNMP_ERR_WRONGTYPE;
}

/*
 * the relocatable extensible commands variables 
 */
struct variable2 extensible_relocatable_variables[] = {
    {MIBINDEX, ASN_INTEGER, RONLY, var_extensible_relocatable, 1,
     {MIBINDEX}},
    {ERRORNAME, ASN_OCTET_STR, RONLY, var_extensible_relocatable, 1,
     {ERRORNAME}},
    {SHELLCOMMAND, ASN_OCTET_STR, RONLY, var_extensible_relocatable, 1,
     {SHELLCOMMAND}},
    {ERRORFLAG, ASN_INTEGER, RONLY, var_extensible_relocatable, 1,
     {ERRORFLAG}},
    {ERRORMSG, ASN_OCTET_STR, RONLY, var_extensible_relocatable, 1,
     {ERRORMSG}},
    {ERRORFIX, ASN_INTEGER, RWRITE, var_extensible_relocatable, 1,
     {ERRORFIX}}
};

u_char         *
var_extensible_relocatable(struct variable *vp,
                           oid * name,
                           size_t * length,
                           int exact,
                           size_t * var_len, WriteMethod ** write_method)
{

    int             fd;
    int             i;
    FILE           *file;
    struct extensible *exten = 0;
    static long     long_ret;
    static char     errmsg[STRMAX];
    struct variable myvp;
    oid             tname[MAX_OID_LEN];

    memcpy(&myvp, vp, sizeof(struct variable));

    long_ret = *length;
    for (i = 1; i <= (int) numrelocs; i++) {
        exten = get_exten_instance(relocs, i);
        if ((int) exten->miblen == (int) vp->namelen - 1) {
            memcpy(myvp.name, exten->miboid, exten->miblen * sizeof(oid));
            myvp.namelen = exten->miblen;
            *length = vp->namelen;
            memcpy(tname, vp->name, vp->namelen * sizeof(oid));
            if (!header_simple_table
                (&myvp, tname, length, -1, var_len, write_method, -1))
                break;
            else
                exten = NULL;
        }
    }
    if (i > (int) numrelocs || exten == NULL) {
        *length = long_ret;
        *var_len = 0;
        *write_method = NULL;
        return (NULL);
    }

    *length = long_ret;
    if (header_simple_table(vp, name, length, exact, var_len, write_method,
                            ((vp->magic == ERRORMSG) ? MAXMSGLINES : 1)))
        return (NULL);

    switch (vp->magic) {
    case MIBINDEX:
        long_ret = name[*length - 1];
        return ((u_char *) (&long_ret));
    case ERRORNAME:            /* name defined in config file */
        *var_len = strlen(exten->name);
        return ((u_char *) (exten->name));
    case SHELLCOMMAND:
        *var_len = strlen(exten->command);
        return ((u_char *) (exten->command));
    case ERRORFLAG:            /* return code from the process */
        if (exten->type == EXECPROC)
            exec_command(exten);
        else
            shell_command(exten);
        long_ret = exten->result;
        return ((u_char *) (&long_ret));
    case ERRORMSG:             /* first line of text returned from the process */
        if (exten->type == EXECPROC) {
            if ((fd = get_exec_output(exten)) != -1) {
                file = fdopen(fd, "r");
                for (i = 0; i != (int) name[*length - 1]; i++) {
                    if (fgets(errmsg, sizeof(errmsg), file) == NULL) {
                        *var_len = 0;
                        fclose(file);
                        wait_on_exec(exten);
                        return (NULL);
                    }
                }
                fclose(file);
                wait_on_exec(exten);
            } else
                errmsg[0] = 0;
        } else {
            if (*length > 1) {
                *var_len = 0;
                return (NULL);
            }
            shell_command(exten);
            strncpy(errmsg, exten->output, sizeof(errmsg));
            errmsg[ sizeof(errmsg)-1 ] = 0;
        }
        *var_len = strlen(errmsg);
        if (errmsg[*var_len - 1] == '\n')
            errmsg[--(*var_len)] = '\0';
        return ((u_char *) (errmsg));
    case ERRORFIX:
        *write_method = fixExecError;
        long_return = 0;
        return ((u_char *) & long_return);
    }
    return NULL;
}

netsnmp_subtree *
find_extensible(netsnmp_subtree *tp, oid *tname, size_t tnamelen, int exact)
{
    size_t          tmp;
    int             i;
    struct extensible *exten = 0;
    struct variable myvp;
    oid             name[MAX_OID_LEN];
    static netsnmp_subtree mysubtree[2] =
	{ { NULL, 0, NULL, 0, NULL, 0, NULL, 0, 0, NULL, NULL, 0, 0, 0,
	    NULL, NULL, NULL, 0, 0, NULL, 0, 0 },
	  { NULL, 0, NULL, 0, NULL, 0, NULL, 0, 0, NULL, NULL, 0, 0, 0,
	    NULL, NULL, NULL, 0, 0, NULL, 0, 0 } };

    for (i = 1; i <= (int) numrelocs; i++) {
        exten = get_exten_instance(relocs, i);
        if (exten->miblen != 0) {
            memcpy(myvp.name, exten->miboid, exten->miblen * sizeof(oid));
            memcpy(name, tname, tnamelen * sizeof(oid));
            myvp.name[exten->miblen] = name[exten->miblen];
            myvp.namelen = exten->miblen + 1;
            tmp = exten->miblen + 1;
            if (!header_simple_table(&myvp, name, &tmp, -1, 
				     NULL, NULL, numrelocs)) {
                break;
	    }
        }
    }
    if (i > (int)numrelocs || exten == NULL) {
        return (tp);
    }

    if (mysubtree[0].name_a != NULL) {
	free(mysubtree[0].name_a);
	mysubtree[0].name_a = NULL;
    }
    mysubtree[0].name_a  = snmp_duplicate_objid(exten->miboid, exten->miblen);
    mysubtree[0].namelen = exten->miblen;
    mysubtree[0].variables = (struct variable *)extensible_relocatable_variables;
    mysubtree[0].variables_len = sizeof(extensible_relocatable_variables) /
        sizeof(*extensible_relocatable_variables);
    mysubtree[0].variables_width = sizeof(*extensible_relocatable_variables);
    mysubtree[1].namelen = 0;
    return (mysubtree);
}
