/*
 *  Host Resources MIB - system group implementation - hr_system.c
 *
 */

#include <net-snmp/net-snmp-config.h>
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "host.h"
#include "host_res.h"
#include "hr_system.h"
#include <net-snmp/agent/auto_nlist.h>

#ifdef HAVE_SYS_PROC_H
#include <sys/param.h>
#include "sys/proc.h"
#endif
#if HAVE_UTMPX_H
#include <utmpx.h>
#else
#include <utmp.h>
#endif

#ifdef linux
#ifdef HAVE_LINUX_TASKS_H
#include <linux/tasks.h>
#else
/*
 * If this file doesn't exist, then there is no hard limit on the number
 * of processes, so return 0 for hrSystemMaxProcesses.  
 */
#define NR_TASKS	0
#endif
#endif

#if defined(hpux10) || defined(hpux11)
#include <sys/pstat.h>
#endif

#ifdef HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif

#if !defined(UTMP_FILE) && defined(_PATH_UTMP)
#define UTMP_FILE _PATH_UTMP
#endif

#if defined(UTMP_FILE) && !HAVE_UTMPX_H
void            setutent(void);
void            endutent(void);
struct utmp    *getutent(void);
#endif                          /* UTMP_FILE */


        /*********************
	 *
	 *  Kernel & interface information,
	 *   and internal forward declarations
	 *
	 *********************/

static int      get_load_dev(void);
static int      count_users(void);
extern int      count_processes(void);


        /*********************
	 *
	 *  Initialisation & common implementation functions
	 *
	 *********************/

#define	HRSYS_UPTIME		1
#define	HRSYS_DATE		2
#define	HRSYS_LOAD_DEV		3
#define	HRSYS_LOAD_PARAM	4
#define	HRSYS_USERS		5
#define	HRSYS_PROCS		6
#define	HRSYS_MAXPROCS		7

struct variable2 hrsystem_variables[] = {
    {HRSYS_UPTIME, ASN_TIMETICKS, RONLY, var_hrsys, 1, {1}},
    {HRSYS_DATE, ASN_OCTET_STR, RONLY, var_hrsys, 1, {2}},
    {HRSYS_LOAD_DEV, ASN_INTEGER, RONLY, var_hrsys, 1, {3}},
    {HRSYS_LOAD_PARAM, ASN_OCTET_STR, RONLY, var_hrsys, 1, {4}},
    {HRSYS_USERS, ASN_GAUGE, RONLY, var_hrsys, 1, {5}},
    {HRSYS_PROCS, ASN_GAUGE, RONLY, var_hrsys, 1, {6}},
    {HRSYS_MAXPROCS, ASN_INTEGER, RONLY, var_hrsys, 1, {7}}
};
oid             hrsystem_variables_oid[] = { 1, 3, 6, 1, 2, 1, 25, 1 };


void
init_hr_system(void)
{
#ifdef NPROC_SYMBOL
    auto_nlist(NPROC_SYMBOL, 0, 0);
#endif

    REGISTER_MIB("host/hr_system", hrsystem_variables, variable2,
                 hrsystem_variables_oid);
}

/*
 * header_hrsys(...
 * Arguments:
 * vp     IN      - pointer to variable entry that points here
 * name    IN/OUT  - IN/name requested, OUT/name found
 * length  IN/OUT  - length of IN/OUT oid's 
 * exact   IN      - TRUE if an exact match was requested
 * var_len OUT     - length of variable or 0 if function returned
 * write_method
 */

int
header_hrsys(struct variable *vp,
             oid * name,
             size_t * length,
             int exact, size_t * var_len, WriteMethod ** write_method)
{
#define HRSYS_NAME_LENGTH	9
    oid             newname[MAX_OID_LEN];
    int             result;

    DEBUGMSGTL(("host/hr_system", "var_hrsys: "));
    DEBUGMSGOID(("host/hr_system", name, *length));
    DEBUGMSG(("host/hr_system", " %d\n", exact));

    memcpy((char *) newname, (char *) vp->name, vp->namelen * sizeof(oid));
    newname[HRSYS_NAME_LENGTH] = 0;
    result = snmp_oid_compare(name, *length, newname, vp->namelen + 1);
    if ((exact && (result != 0)) || (!exact && (result >= 0)))
        return (MATCH_FAILED);
    memcpy((char *) name, (char *) newname,
           (vp->namelen + 1) * sizeof(oid));
    *length = vp->namelen + 1;

    *write_method = 0;
    *var_len = sizeof(long);    /* default to 'long' results */
    return (MATCH_SUCCEEDED);
}


        /*********************
	 *
	 *  System specific implementation functions
	 *
	 *********************/

u_char         *
var_hrsys(struct variable * vp,
          oid * name,
          size_t * length,
          int exact, size_t * var_len, WriteMethod ** write_method)
{
    static char     string[100];
    time_t          now;
#ifndef NR_TASKS
    int             nproc = 0;
#endif
#ifdef linux
    FILE           *fp;
#endif
#if CAN_USE_SYSCTL && defined(CTL_KERN) && defined(KERN_MAXPROC)
    static int      maxproc_mib[] = { CTL_KERN, KERN_MAXPROC };
    int             buf_size;
#endif
#if defined(hpux10) || defined(hpux11)
    struct pst_static pst_buf;
#endif

    if (header_hrsys(vp, name, length, exact, var_len, write_method) ==
        MATCH_FAILED)
        return NULL;

    switch (vp->magic) {
    case HRSYS_UPTIME:
        long_return = get_uptime();
        return (u_char *) & long_return;
    case HRSYS_DATE:
        (void *) time(&now);
        return (u_char *) date_n_time(&now, var_len);
    case HRSYS_LOAD_DEV:
        long_return = get_load_dev();
        return (u_char *) & long_return;
    case HRSYS_LOAD_PARAM:
#ifdef linux
        fp = fopen("/proc/cmdline", "r");
        fgets(string, sizeof(string), fp);
        fclose(fp);
#else
#if NO_DUMMY_VALUES
        return NULL;
#endif
        sprintf(string, "ask Dave");    /* XXX */
#endif
        *var_len = strlen(string);
        return (u_char *) string;
    case HRSYS_USERS:
        long_return = count_users();
        return (u_char *) & long_return;
    case HRSYS_PROCS:
#if USING_HOST_HR_SWRUN_MODULE
        long_return = count_processes();
#else
#if NO_DUMMY_VALUES
        return NULL;
#endif
        long_return = 0;
#endif
        return (u_char *) & long_return;
    case HRSYS_MAXPROCS:
#if defined(NR_TASKS)
        long_return = NR_TASKS; /* <linux/tasks.h> */
#elif CAN_USE_SYSCTL && defined(CTL_KERN) && defined(KERN_MAXPROC)
        buf_size = sizeof(nproc);
        if (sysctl(maxproc_mib, 2, &nproc, &buf_size, NULL, 0) < 0)
            return NULL;
        long_return = nproc;
#elif defined(hpux10) || defined(hpux11)
        pstat_getstatic(&pst_buf, sizeof(struct pst_static), 1, 0);
        long_return = pst_buf.max_proc;
#elif defined(NPROC_SYMBOL)
        auto_nlist(NPROC_SYMBOL, (char *) &nproc, sizeof(int));
        long_return = nproc;
#else
#if NO_DUMMY_VALUES
        return NULL;
#endif
        long_return = 0;
#endif
        return (u_char *) & long_return;
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_hrsys\n",
                    vp->magic));
    }
    return NULL;
}


        /*********************
	 *
	 *  Internal implementation functions
	 *
	 *********************/

                /*
                 *  Return the DeviceIndex corresponding
                 *   to the boot device
                 */
static int
get_load_dev(void)
{
    return (HRDEV_DISK << HRDEV_TYPE_SHIFT);    /* XXX */
}

static int
count_users(void)
{
    int             total = 0;
#if HAVE_UTMPX_H
#define setutent setutxent
#define getutent getutxent
#define endutent endutxent
    struct utmpx   *utmp_p;
#else
    struct utmp    *utmp_p;
#endif

    setutent();
    while ((utmp_p = getutent()) != NULL) {
#ifndef UTMP_HAS_NO_TYPE
        if (utmp_p->ut_type == USER_PROCESS)
#endif
            ++total;
    }
    endutent();
    return total;
}

#if defined(UTMP_FILE) && !HAVE_UTMPX_H

static FILE    *utmp_file;
static struct utmp utmp_rec;

void
setutent(void)
{
    if (utmp_file)
        fclose(utmp_file);
    utmp_file = fopen(UTMP_FILE, "r");
}

void
endutent(void)
{
    if (utmp_file) {
        fclose(utmp_file);
        utmp_file = NULL;
    }
}

struct utmp    *
getutent(void)
{
    if (!utmp_file)
        return NULL;
    while (fread(&utmp_rec, sizeof(utmp_rec), 1, utmp_file) == 1)
        if (*utmp_rec.ut_name && *utmp_rec.ut_line)
            return &utmp_rec;
    return NULL;
}

#endif                          /* UTMP_FILE */
