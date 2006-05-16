/*
 *  Host Resources MIB - Running Software group implementation - hr_swrun.c
 *      (also includes Running Software Performance group )
 *
 */

#include <net-snmp/net-snmp-config.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <fcntl.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sys/param.h>
#include <ctype.h>
#if HAVE_SYS_PSTAT_H
#include <sys/pstat.h>
#endif
#if HAVE_SYS_USER_H
#ifdef solaris2
#define _KMEMUSER
#endif
#include <sys/user.h>
#endif
#if HAVE_SYS_PROC_H
#include <sys/proc.h>
#endif
#if HAVE_KVM_H
#include <kvm.h>
#endif
#if HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif
#if HAVE_DIRENT_H && !defined(cygwin)
#include <dirent.h>
#else
# define dirent direct
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif
#ifdef cygwin
#include <windows.h>
#include <sys/cygwin.h>
#include <tlhelp32.h>
#include <psapi.h>
#endif

#if _SLASH_PROC_METHOD_
#include <procfs.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#include <stdio.h>

#include <net-snmp/output_api.h>
#include "host_res.h"
#include "hr_swrun.h"
#include <net-snmp/agent/auto_nlist.h>
#include "kernel.h"
#if solaris2
#include "kernel_sunos5.h"
#endif

        /*********************
	 *
	 *  Initialisation & common implementation functions
	 *
	 *********************/
void            Init_HR_SWRun(void);
int             Get_Next_HR_SWRun(void);
void            End_HR_SWRun(void);
int             header_hrswrun(struct variable *, oid *, size_t *, int,
                               size_t *, WriteMethod **);
int             header_hrswrunEntry(struct variable *, oid *, size_t *,
                                    int, size_t *, WriteMethod **);

#ifdef dynix
pid_t           nextproc;
static prpsinfo_t lowpsinfo, mypsinfo;
#endif
#ifdef cygwin
static struct external_pinfo *curproc;
static struct external_pinfo lowproc;
#elif !defined(linux)
static int      LowProcIndex;
#endif
#if defined(hpux10) || defined(hpux11)
struct pst_status *proc_table;
struct pst_dynamic pst_dyn;
#elif HAVE_KVM_GETPROCS
struct kinfo_proc *proc_table;
#elif defined(solaris2)
int            *proc_table;
#else
struct proc    *proc_table;
#endif
#ifndef dynix
int             current_proc_entry;
#endif


#define	HRSWRUN_OSINDEX		1

#define	HRSWRUN_INDEX		2
#define	HRSWRUN_NAME		3
#define	HRSWRUN_ID		4
#define	HRSWRUN_PATH		5
#define	HRSWRUN_PARAMS		6
#define	HRSWRUN_TYPE		7
#define	HRSWRUN_STATUS		8

#define	HRSWRUNPERF_CPU		9
#define	HRSWRUNPERF_MEM		10

struct variable4 hrswrun_variables[] = {
    {HRSWRUN_OSINDEX, ASN_INTEGER, RONLY, var_hrswrun, 1, {1}},
    {HRSWRUN_INDEX, ASN_INTEGER, RONLY, var_hrswrun, 3, {2, 1, 1}},
    {HRSWRUN_NAME, ASN_OCTET_STR, RONLY, var_hrswrun, 3, {2, 1, 2}},
    {HRSWRUN_ID, ASN_OBJECT_ID, RONLY, var_hrswrun, 3, {2, 1, 3}},
    {HRSWRUN_PATH, ASN_OCTET_STR, RONLY, var_hrswrun, 3, {2, 1, 4}},
    {HRSWRUN_PARAMS, ASN_OCTET_STR, RONLY, var_hrswrun, 3, {2, 1, 5}},
    {HRSWRUN_TYPE, ASN_INTEGER, RONLY, var_hrswrun, 3, {2, 1, 6}},
    {HRSWRUN_STATUS, ASN_INTEGER, RONLY, var_hrswrun, 3, {2, 1, 7}}
};

struct variable4 hrswrunperf_variables[] = {
    {HRSWRUNPERF_CPU, ASN_INTEGER, RONLY, var_hrswrun, 3, {1, 1, 1}},
    {HRSWRUNPERF_MEM, ASN_INTEGER, RONLY, var_hrswrun, 3, {1, 1, 2}}
};

oid             hrswrun_variables_oid[] = { 1, 3, 6, 1, 2, 1, 25, 4 };
oid             hrswrunperf_variables_oid[] = { 1, 3, 6, 1, 2, 1, 25, 5 };

#ifdef cygwin

/*
 * a lot of this is "stolen" from cygwin ps.cc
 */

typedef         BOOL(WINAPI * ENUMPROCESSMODULES) (HANDLE hProcess,
                                                   HMODULE * lphModule,
                                                   DWORD cb,
                                                   LPDWORD lpcbNeeded);

typedef         DWORD(WINAPI * GETMODULEFILENAME) (HANDLE hProcess,
                                                   HMODULE hModule,
                                                   LPTSTR lpstrFIleName,
                                                   DWORD nSize);

typedef         DWORD(WINAPI * GETPROCESSMEMORYINFO) (HANDLE hProcess,
                                                      PPROCESS_MEMORY_COUNTERS
                                                      pmc, DWORD nSize);

typedef         HANDLE(WINAPI * CREATESNAPSHOT) (DWORD dwFlags,
                                                 DWORD th32ProcessID);

typedef         BOOL(WINAPI * PROCESSWALK) (HANDLE hSnapshot,
                                            LPPROCESSENTRY32 lppe);

ENUMPROCESSMODULES myEnumProcessModules;
GETMODULEFILENAME myGetModuleFileNameEx;
CREATESNAPSHOT  myCreateToolhelp32Snapshot;
PROCESSWALK     myProcess32First;
PROCESSWALK     myProcess32Next;
GETPROCESSMEMORYINFO myGetProcessMemoryInfo = NULL;
cygwin_getinfo_types query = CW_GETPINFO;

static BOOL WINAPI
dummyprocessmodules(HANDLE hProcess,
                    HMODULE * lphModule, DWORD cb, LPDWORD lpcbNeeded)
{
    lphModule[0] = (HMODULE) * lpcbNeeded;
    *lpcbNeeded = 1;
    return 1;
}

static DWORD WINAPI
GetModuleFileNameEx95(HANDLE hProcess,
                      HMODULE hModule, LPTSTR lpstrFileName, DWORD n)
{
    HANDLE          h;
    DWORD           pid = (DWORD) hModule;
    PROCESSENTRY32  proc;

    h = myCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (!h)
        return 0;
    proc.dwSize = sizeof(proc);
    if (myProcess32First(h, &proc))
        do
            if (proc.th32ProcessID == pid) {
                CloseHandle(h);
                strcpy(lpstrFileName, proc.szExeFile);
                return 1;
            }
        while (myProcess32Next(h, &proc));
    CloseHandle(h);
    return 0;
}

#define FACTOR (0x19db1ded53ea710LL)
#define NSPERSEC 10000000LL
#define NSPERMSEC 10000LL

static time_t   __stdcall
to_time_t(PFILETIME ptr)
{
    long            rem;
    long long       x =
        ((long long) ptr->dwHighDateTime << 32) +
        ((unsigned) ptr->dwLowDateTime);
    x -= FACTOR;
    rem = x % NSPERSEC;
    rem += NSPERSEC / 2;
    x /= NSPERSEC;
    x += rem / NSPERSEC;
    return x;
}

static long
to_msec(PFILETIME ptr)
{
    long long       x =
        ((long long) ptr->dwHighDateTime << 32) +
        (unsigned) ptr->dwLowDateTime;
    x /= NSPERMSEC;
    return x;
}

#endif                          /* cygwin */


void
init_hr_swrun(void)
{
#ifdef cygwin
    OSVERSIONINFO   ver;
    HMODULE         h;

    memset(&ver, 0, sizeof ver);
    ver.dwOSVersionInfoSize = sizeof ver;
    GetVersionEx(&ver);

    if (ver.dwPlatformId == VER_PLATFORM_WIN32_NT) {
        h = LoadLibrary("psapi.dll");
        if (h) {
            myEnumProcessModules =
                (ENUMPROCESSMODULES) GetProcAddress(h,
                                                    "EnumProcessModules");
            myGetModuleFileNameEx =
                (GETMODULEFILENAME) GetProcAddress(h,
                                                   "GetModuleFileNameExA");
            myGetProcessMemoryInfo =
                (GETPROCESSMEMORYINFO) GetProcAddress(h,
                                                      "GetProcessMemoryInfo");
            if (myEnumProcessModules && myGetModuleFileNameEx)
                query = CW_GETPINFO_FULL;
            else
                snmp_log(LOG_ERR, "hr_swrun failed NT init\n");
        } else
            snmp_log(LOG_ERR, "hr_swrun failed to load psapi.dll\n");
    } else {
        h = GetModuleHandle("KERNEL32.DLL");
        myCreateToolhelp32Snapshot =
            (CREATESNAPSHOT) GetProcAddress(h, "CreateToolhelp32Snapshot");
        myProcess32First =
            (PROCESSWALK) GetProcAddress(h, "Process32First");
        myProcess32Next = (PROCESSWALK) GetProcAddress(h, "Process32Next");
        myEnumProcessModules = dummyprocessmodules;
        myGetModuleFileNameEx = GetModuleFileNameEx95;
        if (myCreateToolhelp32Snapshot && myProcess32First
            && myProcess32Next)
#if 0
            /*
             * This doesn't work after all on Win98 SE 
             */
            query = CW_GETPINFO_FULL;
#else
            query = CW_GETPINFO;
#endif
        else
            snmp_log(LOG_ERR, "hr_swrun failed non-NT init\n");
    }
#endif                          /* cygwin */
#ifdef PROC_SYMBOL
    auto_nlist(PROC_SYMBOL, 0, 0);
#endif
#ifdef NPROC_SYMBOL
    auto_nlist(NPROC_SYMBOL, 0, 0);
#endif

    proc_table = 0;

    REGISTER_MIB("host/hr_swrun", hrswrun_variables, variable4,
                 hrswrun_variables_oid);
    REGISTER_MIB("host/hr_swrun", hrswrunperf_variables, variable4,
                 hrswrunperf_variables_oid);
}

/*
 * header_hrswrun(...
 * Arguments:
 * vp     IN      - pointer to variable entry that points here
 * name    IN/OUT  - IN/name requested, OUT/name found
 * length  IN/OUT  - length of IN/OUT oid's 
 * exact   IN      - TRUE if an exact match was requested
 * var_len OUT     - length of variable or 0 if function returned
 * write_method
 * 
 */

int
header_hrswrun(struct variable *vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method)
{
#define HRSWRUN_NAME_LENGTH	9
    oid             newname[MAX_OID_LEN];
    int             result;

    DEBUGMSGTL(("host/hr_swrun", "var_hrswrun: "));
    DEBUGMSGOID(("host/hr_swrun", name, *length));
    DEBUGMSG(("host/hr_swrun", " %d\n", exact));

    memcpy((char *) newname, (char *) vp->name, vp->namelen * sizeof(oid));
    newname[HRSWRUN_NAME_LENGTH] = 0;
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

int
header_hrswrunEntry(struct variable *vp,
                    oid * name,
                    size_t * length,
                    int exact,
                    size_t * var_len, WriteMethod ** write_method)
{
#define HRSWRUN_ENTRY_NAME_LENGTH	11
    oid             newname[MAX_OID_LEN];
    int             pid, LowPid = -1;
    int             result;

    DEBUGMSGTL(("host/hr_swrun", "var_hrswrunEntry: "));
    DEBUGMSGOID(("host/hr_swrun", name, *length));
    DEBUGMSG(("host/hr_swrun", " %d\n", exact));

    memcpy((char *) newname, (char *) vp->name, vp->namelen * sizeof(oid));

    /*
     *  Find the "next" running process
     */
    Init_HR_SWRun();
    for (;;) {
        pid = Get_Next_HR_SWRun();
#ifndef linux
#ifndef dynix
        DEBUGMSG(("host/hr_swrun",
                  "(index %d (entry #%d) ....", pid, current_proc_entry));
#else
        DEBUGMSG(("host/hr_swrun", "pid %d; nextproc %d ....", pid,
                  nextproc));
#endif
#endif
        if (pid == -1)
            break;
        newname[HRSWRUN_ENTRY_NAME_LENGTH] = pid;
        DEBUGMSGOID(("host/hr_swrun", newname, *length));
        DEBUGMSG(("host/hr_swrun", "\n"));
        result = snmp_oid_compare(name, *length, newname, vp->namelen + 1);
        if (exact && (result == 0)) {
            LowPid = pid;
#ifdef cygwin
            lowproc = *curproc;
#elif  dynix
            memcpy(&lowpsinfo, &mypsinfo, sizeof(prpsinfo_t));
#elif !defined(linux)
            LowProcIndex = current_proc_entry - 1;
#endif
            DEBUGMSGTL(("host/hr_swrun", " saved\n"));
            /*
             * Save process status information 
             */
            break;
        }
        if ((!exact && (result < 0)) && (LowPid == -1 || pid < LowPid)) {
            LowPid = pid;
#ifdef cygwin
            lowproc = *curproc;
#elif !defined(linux)
            LowProcIndex = current_proc_entry - 1;
#endif
            /*
             * Save process status information 
             */
            DEBUGMSG(("host/hr_swrun", " saved"));
        }
        DEBUGMSG(("host/hr_swrun", "\n"));
    }
    End_HR_SWRun();

    if (LowPid == -1) {
        DEBUGMSGTL(("host/hr_swrun", "... index out of range\n"));
        return (MATCH_FAILED);
    }

    newname[HRSWRUN_ENTRY_NAME_LENGTH] = LowPid;
    memcpy((char *) name, (char *) newname,
           (vp->namelen + 1) * sizeof(oid));
    *length = vp->namelen + 1;
    *write_method = 0;
    *var_len = sizeof(long);    /* default to 'long' results */

    DEBUGMSGTL(("host/hr_swrun", "... get process stats "));
    DEBUGMSGOID(("host/hr_swrun", name, *length));
    DEBUGMSG(("host/hr_swrun", "\n"));
    return LowPid;
}

        /*********************
	 *
	 *  System specific implementation functions
	 *
	 *********************/


u_char         *
var_hrswrun(struct variable * vp,
            oid * name,
            size_t * length,
            int exact, size_t * var_len, WriteMethod ** write_method)
{
    int             pid = 0;
    static char     string[256];
#ifdef HAVE_SYS_PSTAT_H
    struct pst_status proc_buf;
#elif defined(solaris2)
#if _SLASH_PROC_METHOD_
    static psinfo_t psinfo;
    static psinfo_t *proc_buf;
    int             procfd;
    char            procfn[sizeof "/proc/00000/psinfo"];
#else
    static struct proc *proc_buf;
    char           *cp1;
#endif                          /* _SLASH_PROC_METHOD_ */
    static time_t   when = 0;
    time_t          now;
    static int      oldpid = -1;
#endif
#if HAVE_KVM_GETPROCS
    char          **argv;
#endif
#ifdef linux
    FILE           *fp;
    char            buf[256];
    int             i;
#endif
    char           *cp;

    if (vp->magic == HRSWRUN_OSINDEX) {
        if (header_hrswrun(vp, name, length, exact, var_len, write_method)
            == MATCH_FAILED)
            return NULL;
    } else {

        pid =
            header_hrswrunEntry(vp, name, length, exact, var_len,
                                write_method);
        if (pid == MATCH_FAILED)
            return NULL;
    }

#ifdef HAVE_SYS_PSTAT_H
    if (pstat_getproc(&proc_buf, sizeof(struct pst_status), 0, pid) == -1)
        return NULL;
#elif defined(solaris2)
    time(&now);
    if (pid == oldpid) {
        if (now != when)
            oldpid = -1;
    }
    if (oldpid != pid || proc_buf == NULL) {
#if _SLASH_PROC_METHOD_
        proc_buf = &psinfo;
        sprintf(procfn, "/proc/%.5d/psinfo", pid);
        if ((procfd = open(procfn, O_RDONLY)) != -1) {
            if (read(procfd, proc_buf, sizeof(*proc_buf)) !=
                sizeof(*proc_buf))
                abort();
            close(procfd);
        } else
            proc_buf = NULL;
#else
        if (kd == NULL)
            return NULL;
        if ((proc_buf = kvm_getproc(kd, pid)) == NULL)
            return NULL;
#endif
        oldpid = pid;
        when = now;
    }
#endif

    switch (vp->magic) {
    case HRSWRUN_OSINDEX:
#if NO_DUMMY_VALUES
        return NULL;
#else
        long_return = 1;        /* Probably! */
        return (u_char *) & long_return;
#endif

    case HRSWRUN_INDEX:
        long_return = pid;
        return (u_char *) & long_return;
    case HRSWRUN_NAME:
#ifdef HAVE_SYS_PSTAT_H
        snprintf(string, sizeof(string), "%s", proc_buf.pst_cmd);
        string[ sizeof(string)-1 ] = 0;
        cp = strchr(string, ' ');
        if (cp != NULL)
            *cp = '\0';
#elif defined(dynix)
        snprintf(string, sizeof(string), "%s", lowpsinfo.pr_fname);
        string[ sizeof(string)-1 ] = 0;
        cp = strchr(string, ' ');
        if (cp != NULL)
            *cp = '\0';
#elif defined(solaris2)
#if _SLASH_PROC_METHOD_
        if (proc_buf)
            strncpy(string, proc_buf->pr_fname, sizeof(string));
        else
            strcpy(string, "<exited>");
        string[ sizeof(string)-1 ] = 0;
#else
        strncpy(string, proc_buf->p_user.u_comm, sizeof(string));
        string[ sizeof(string)-1 ] = 0;
#endif
#elif HAVE_KVM_GETPROCS
        strcpy(string, proc_table[LowProcIndex].kp_proc.p_comm);
#elif defined(linux)
        sprintf(string, "/proc/%d/status", pid);
        if ((fp = fopen(string, "r")) == NULL)
            return NULL;
        fgets(buf, sizeof(buf), fp);    /* Name: process name */
        cp = buf;
        while (*cp != ':')
            ++cp;
        ++cp;
        while (isspace(*cp))
            ++cp;
        strcpy(string, cp);
        fclose(fp);
#elif defined(cygwin)
        if (lowproc.process_state & (PID_ZOMBIE | PID_EXITED))
            strcpy(string, "<defunct>");
        else if (lowproc.ppid) {
            cygwin_conv_to_posix_path(lowproc.progname, string);
            cp = strrchr(string, '/');
            if (cp)
                strcpy(string, cp + 1);
        } else if (query == CW_GETPINFO_FULL) {
            DWORD           n = lowproc.dwProcessId & 0xffff;
            HANDLE          h =
                OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                            FALSE, n);

            if (h) {
                HMODULE         hm[1000];
                if (!myEnumProcessModules(h, hm, sizeof hm, &n)) {
                    snmp_log(LOG_DEBUG, "no module handle for %lu\n", n);
                    n = 0;
                }
                if (n
                    && myGetModuleFileNameEx(h, hm[0], string,
                                             sizeof string)) {
                    cp = strrchr(string, '\\');
                    if (cp)
                        strcpy(string, cp + 1);
                } else
                    strcpy(string, "*** unknown");
                CloseHandle(h);
            } else {
                snmp_log(LOG_INFO, "no process handle for %lu\n", n);
                strcpy(string, "** unknown");
            }
        } else
            strcpy(string, "* unknown");
        cp = strchr(string, '\0') - 4;
        if (cp > string && strcasecmp(cp, ".exe") == 0)
            *cp = '\0';
#else
#if NO_DUMMY_VALUES
        return NULL;
#endif
        sprintf(string, "process name");
#endif
        *var_len = strlen(string);
        /*
         * remove trailing newline 
         */
        if (*var_len) {
            cp = string + *var_len - 1;
            if (*cp == '\n')
                --(*var_len);
        }
        return (u_char *) string;
    case HRSWRUN_ID:
        *var_len = nullOidLen;
        return (u_char *) nullOid;
    case HRSWRUN_PATH:
#ifdef HAVE_SYS_PSTAT_H
        /*
         * Path not available - use argv[0] 
         */
        sprintf(string, "%s", proc_buf.pst_cmd);
        cp = strchr(string, ' ');
        if (cp != NULL)
            *cp = '\0';
#elif defined(dynix)
        /*
         * Path not available - use argv[0] 
         */
        sprintf(string, "%s", lowpsinfo.pr_psargs);
        cp = strchr(string, ' ');
        if (cp != NULL)
            *cp = '\0';
#elif defined(solaris2)
#ifdef _SLASH_PROC_METHOD_
        if (proc_buf)
            strcpy(string, proc_buf->pr_psargs);
        else
            sprintf(string, "<exited>");
        cp = strchr(string, ' ');
        if (cp)
            *cp = 0;
#else
        cp = proc_buf->p_user.u_psargs;
        cp1 = string;
        while (*cp && *cp != ' ')
            *cp1++ = *cp++;
        *cp1 = 0;
#endif
#elif HAVE_KVM_GETPROCS
        strcpy(string, proc_table[LowProcIndex].kp_proc.p_comm);
#elif defined(linux)
        sprintf(string, "/proc/%d/cmdline", pid);
        if ((fp = fopen(string, "r")) == NULL)
            return NULL;
        if (fgets(buf, sizeof(buf) - 1, fp))    /* argv[0] '\0' argv[1] '\0' .... */
            strcpy(string, buf);
        else {
            /*
             * swapped out - no cmdline 
             */
            fclose(fp);
            sprintf(string, "/proc/%d/status", pid);
            if ((fp = fopen(string, "r")) == NULL)
                return NULL;
            fgets(buf, sizeof(buf), fp);        /* Name: process name */
            cp = strchr(buf, ':');
            ++cp;
            while (isspace(*cp))
                ++cp;
            strcpy(string, cp);
            cp = strchr(string, '\n');
            if (cp)
                *cp = 0;
        }
        fclose(fp);
#elif defined(cygwin)
        if (lowproc.process_state & (PID_ZOMBIE | PID_EXITED))
            strcpy(string, "<defunct>");
        else if (lowproc.ppid)
            cygwin_conv_to_posix_path(lowproc.progname, string);
        else if (query == CW_GETPINFO_FULL) {
            DWORD           n = lowproc.dwProcessId & 0xFFFF;
            HANDLE          h =
                OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                            FALSE, n);
            if (h) {
                HMODULE         hm[1000];
                if (!myEnumProcessModules(h, hm, sizeof hm, &n))
                    n = 0;
                if (!n
                    || !myGetModuleFileNameEx(h, hm[0], string,
                                              sizeof string))
                    strcpy(string, "*** unknown");
                CloseHandle(h);
            } else
                strcpy(string, "** unknown");
        } else
            strcpy(string, "* unknown");
#else
#if NO_DUMMY_VALUES
        return NULL;
#endif
        sprintf(string, "/bin/wombat");
#endif
        *var_len = strlen(string);
        return (u_char *) string;
    case HRSWRUN_PARAMS:
#ifdef HAVE_SYS_PSTAT_H
        cp = strchr(proc_buf.pst_cmd, ' ');
        if (cp != NULL) {
            cp++;
            sprintf(string, "%s", cp);
        } else
            string[0] = '\0';
#elif defined(dynix)
        cp = strchr(lowpsinfo.pr_psargs, ' ');
        if (cp != NULL) {
            cp++;
            sprintf(string, "%s", cp);
        } else
            string[0] = '\0';
#elif defined(solaris2)
#ifdef _SLASH_PROC_METHOD_
        if (proc_buf) {
            cp = strchr(proc_buf->pr_psargs, ' ');
            if (cp)
                strcpy(string, cp + 1);
            else
                string[0] = 0;
        } else
            string[0] = 0;
#else
        cp = proc_buf->p_user.u_psargs;
        while (*cp && *cp != ' ')
            cp++;
        if (*cp == ' ')
            cp++;
        strcpy(string, cp);
#endif
#elif HAVE_KVM_GETPROCS
        string[0] = 0;
        argv = kvm_getargv(kd, proc_table + LowProcIndex, sizeof(string));
        if (argv)
            argv++;
        while (argv && *argv) {
            if (string[0] != 0)
                strcat(string, " ");
            strcat(string, *argv);
            argv++;
        }
#elif defined(linux)
        sprintf(string, "/proc/%d/cmdline", pid);
        if ((fp = fopen(string, "r")) == NULL)
            return NULL;
        memset(buf, 0, sizeof(buf));

        /*
         * argv[0] '\0' argv[1] '\0' .... 
         */
        if (!fgets(buf, sizeof(buf) - 2, fp)) {
            /*
             * maybe be empty (even argv[0] is missing) 
             */
            string[0] = '\0';
            *var_len = 0;
            fclose(fp);
            return string;
        }

        /*
         * Skip over argv[0] 
         */
        cp = buf;
        while (*cp)
            ++cp;
        ++cp;
        /*
         * Now join together separate arguments. 
         */
        while (1) {
            while (*cp)
                ++cp;
            if (*(cp + 1) == '\0')
                break;          /* '\0''\0' => End of command line */
            *cp = ' ';
        }

        cp = buf;
        while (*cp)
            ++cp;
        ++cp;
        strcpy(string, cp);
        fclose(fp);
#elif defined(cygwin)
        string[0] = 0;
#else
#if NO_DUMMY_VALUES
        return NULL;
#endif
        sprintf(string, "-h -q -v");
#endif
        *var_len = strlen(string);
        return (u_char *) string;
    case HRSWRUN_TYPE:
#ifdef PID_MAXSYS
        if (pid < PID_MAXSYS)
            long_return = 2;    /* operatingSystem */
        else
#endif
            long_return = 4;    /* application */
        return (u_char *) & long_return;
    case HRSWRUN_STATUS:
#if defined(cygwin)
        if (lowproc.process_state & PID_STOPPED)
            long_return = 3;    /* notRunnable */
        else if (lowproc.process_state & PID_ZOMBIE)
            long_return = 4;    /* invalid */
        else
            long_return = 1;    /* running */
#elif !defined(linux)
#if defined(hpux10) || defined(hpux11)
        switch (proc_table[LowProcIndex].pst_stat) {
        case PS_STOP:
            long_return = 3;    /* notRunnable */
            break;
        case PS_SLEEP:
            long_return = 2;    /* runnable */
            break;
        case PS_RUN:
            long_return = 1;    /* running */
            break;
        case PS_ZOMBIE:
        case PS_IDLE:
        case PS_OTHER:
        default:
            long_return = 4;    /* invalid */
            break;
        }
#else
#if HAVE_KVM_GETPROCS
        switch (proc_table[LowProcIndex].kp_proc.p_stat) {
#elif defined(dynix)
        switch (lowpsinfo.pr_state) {
#elif defined(solaris2)
#if _SLASH_PROC_METHOD_
        switch (proc_buf ? proc_buf->pr_lwp.pr_state : SIDL) {
#else
        switch (proc_buf->p_stat) {
#endif
#else
        switch (proc_table[LowProcIndex].p_stat) {
#endif
        case SSTOP:
            long_return = 3;    /* notRunnable */
            break;
        case 0:
#ifdef SSWAP
        case SSWAP:
#endif
#ifdef SSLEEP
        case SSLEEP:
#endif
#ifdef SWAIT
        case SWAIT:
#endif
            long_return = 2;    /* runnable */
            break;
#ifdef SACTIVE
        case SACTIVE:
#endif
#ifdef SRUN
        case SRUN:
#endif
#ifdef SONPROC
        case SONPROC:
#endif
            long_return = 1;    /* running */
            break;
        case SIDL:
        case SZOMB:
        default:
            long_return = 4;    /* invalid */
            break;
        }
#endif
#else
        sprintf(string, "/proc/%d/stat", pid);
        if ((fp = fopen(string, "r")) != NULL) {
            fgets(buf, sizeof(buf), fp);
            cp = buf;
            for (i = 0; i < 2; ++i) {   /* skip two fields */
                while (*cp != ' ')
                    ++cp;
                ++cp;
            }

            switch (*cp) {
            case 'R':
                long_return = 1;        /* running */
                break;
            case 'S':
                long_return = 2;        /* runnable */
                break;
            case 'D':
            case 'T':
                long_return = 3;        /* notRunnable */
                break;
            case 'Z':
            default:
                long_return = 4;        /* invalid */
                break;
            }
            fclose(fp);
        } else
            long_return = 4;    /* invalid */
#endif
        return (u_char *) & long_return;

    case HRSWRUNPERF_CPU:
#ifdef HAVE_SYS_PSTAT_H
        long_return = proc_buf.pst_cptickstotal;
        /*
         * Not convinced this is right, but....
         */
#elif defined(dynix)
        long_return = lowpsinfo.pr_time.tv_sec * 100 +
            lowpsinfo.pr_time.tv_nsec / 10000000;
#elif defined(solaris2)
#if _SLASH_PROC_METHOD_
        long_return = proc_buf ? proc_buf->pr_time.tv_sec * 100 +
            proc_buf->pr_time.tv_nsec / 10000000 : 0;
#else
        long_return = proc_buf->p_utime * 100 + proc_buf->p_stime * 100;
#endif
#elif HAVE_KVM_GETPROCS
        long_return = proc_table[LowProcIndex].kp_proc.p_uticks +
            proc_table[LowProcIndex].kp_proc.p_sticks +
            proc_table[LowProcIndex].kp_proc.p_iticks;
#elif defined(linux)
        sprintf(string, "/proc/%d/stat", pid);
        if ((fp = fopen(string, "r")) == NULL)
            return NULL;
        fgets(buf, sizeof(buf), fp);
        cp = buf;
        for (i = 0; i < 13; ++i) {      /* skip 13 fields */
            while (*cp != ' ')
                ++cp;
            ++cp;
        }

        long_return = atoi(cp); /* utime */

        while (*cp != ' ')
            ++cp;
        ++cp;
        long_return += atoi(cp);        /* + stime */
        fclose(fp);
#elif defined(sunos4)
        long_return = proc_table[LowProcIndex].p_time;
#elif defined(cygwin)
        {
            DWORD           n = lowproc.dwProcessId;
            HANDLE          h =
                OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                            FALSE, n);
            FILETIME        ct, et, kt, ut;

            if (h) {
                if (GetProcessTimes(h, &ct, &et, &kt, &ut))
                    long_return = (to_msec(&kt) + to_msec(&ut)) / 10;
                else {
                    snmp_log(LOG_INFO, "no process times for %lu (%lu)\n",
                             lowproc.pid, n);
                    long_return = 0;
                }
                CloseHandle(h);
            } else {
                snmp_log(LOG_INFO, "no process handle for %lu (%lu)\n",
                         lowproc.pid, n);
                long_return = 0;
            }
        }
#elif defined(aix4)
        long_return = proc_table[LowProcIndex].p_ru.ru_utime.tv_sec * 100 +
            proc_table[LowProcIndex].p_ru.ru_utime.tv_usec / 10000 +
            proc_table[LowProcIndex].p_ru.ru_stime.tv_sec * 100 +
            proc_table[LowProcIndex].p_ru.ru_stime.tv_usec / 10000;
#else
        long_return = proc_table[LowProcIndex].p_utime.tv_sec * 100 +
            proc_table[LowProcIndex].p_utime.tv_usec / 10000 +
            proc_table[LowProcIndex].p_stime.tv_sec * 100 +
            proc_table[LowProcIndex].p_stime.tv_usec / 10000;
#endif
        return (u_char *) & long_return;
    case HRSWRUNPERF_MEM:
#ifdef HAVE_SYS_PSTAT_H
        long_return = (proc_buf.pst_rssize << PGSHIFT) / 1024;
#elif defined(dynix)
        long_return = (lowpsinfo.pr_rssize * MMU_PAGESIZE) / 1024;
#elif defined(solaris2)
#if _SLASH_PROC_METHOD_
        long_return = proc_buf ? proc_buf->pr_rssize : 0;
#else
        long_return = proc_buf->p_swrss;
#endif
#elif HAVE_KVM_GETPROCS
#if defined(freebsd3) && !defined(darwin)
        long_return =
            proc_table[LowProcIndex].kp_eproc.e_vm.vm_map.size / 1024;
#else
        long_return = proc_table[LowProcIndex].kp_eproc.e_vm.vm_tsize +
            proc_table[LowProcIndex].kp_eproc.e_vm.vm_ssize +
            proc_table[LowProcIndex].kp_eproc.e_vm.vm_dsize;
        long_return = long_return * (getpagesize() / 1024);
#endif
#elif defined(linux)
        sprintf(string, "/proc/%d/stat", pid);
        if ((fp = fopen(string, "r")) == NULL)
            return NULL;
        fgets(buf, sizeof(buf), fp);
        cp = buf;
        for (i = 0; i < 23; ++i) {      /* skip 23 fields */
            while (*cp != ' ')
                ++cp;
            ++cp;
        }
        long_return = atoi(cp) * (getpagesize() / 1024);        /* rss */
        fclose(fp);
#elif defined(cygwin)
        {
            DWORD           n = lowproc.dwProcessId;
            HANDLE          h =
                OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                            FALSE, n);
            PROCESS_MEMORY_COUNTERS pmc;

            if (h) {
                if (myGetProcessMemoryInfo
                    && myGetProcessMemoryInfo(h, &pmc, sizeof pmc))
                    long_return = pmc.WorkingSetSize / 1024;
                else {
                    snmp_log(LOG_INFO, "no process times for %lu (%lu)\n",
                             lowproc.pid, n);
                    long_return = 0;
                }
                CloseHandle(h);
            } else {
                snmp_log(LOG_INFO, "no process handle for %lu (%lu)\n",
                         lowproc.pid, n);
                long_return = 0;
            }
        }
#else
#if NO_DUMMY_VALUES
        return NULL;
#endif
        long_return = 16 * 1024;        /* XXX - 16M! */
#endif
        return (u_char *) & long_return;
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_hrswrun\n",
                    vp->magic));
    }
    return NULL;
}


        /*********************
	 *
	 *  Internal implementation functions
	 *
	 *********************/

#if defined(linux)

DIR            *procdir = NULL;
struct dirent  *procentry_p;

void
Init_HR_SWRun(void)
{
    if (procdir != NULL)
        closedir(procdir);
    procdir = opendir("/proc");
}

int
Get_Next_HR_SWRun(void)
{
    int             pid;
    procentry_p = readdir(procdir);

    if (procentry_p == NULL)
        return -1;

    pid = atoi(procentry_p->d_name);
    if (pid == 0)
        return (Get_Next_HR_SWRun());
    return pid;
}

void
End_HR_SWRun(void)
{
    if (procdir)
        closedir(procdir);
    procdir = NULL;
}

#elif defined(cygwin)

static pid_t    curpid;

void
Init_HR_SWRun(void)
{
    cygwin_internal(CW_LOCK_PINFO, 1000);
    curpid = 0;
}

int
Get_Next_HR_SWRun(void)
{
    curproc =
        (struct external_pinfo *) cygwin_internal(query,
                                                  curpid | CW_NEXTPID);
    if (curproc)
        curpid = curproc->pid;
    else {
        curpid = -1;
    }
    return curpid;
}

void
End_HR_SWRun(void)
{
    cygwin_internal(CW_UNLOCK_PINFO);
}

#elif defined(dynix)

void
Init_HR_SWRun(void)
{
    nextproc = 0;
}

int
Get_Next_HR_SWRun(void)
{
    getprpsinfo_t  *select = 0;

    DEBUGMSGTL(("host/hr_swrun::GetNextHR_SWRun",
                "nextproc == %d... &nextproc = %u\n", nextproc,
                &nextproc));
    if ((nextproc = getprpsinfo(nextproc, select, &mypsinfo)) < 0) {
        return -1;
    } else {
        DEBUGMSGTL(("host/hr_swrun::GetNextHR_SWRun",
                    "getprpsinfo returned %d\n", nextproc));
        return mypsinfo.pr_pid;
    }

}

void
End_HR_SWRun(void)
{
    /*
     * just a stub... because it's declared 
     */
}

#else                           /* linux */

static int      nproc;

void
Init_HR_SWRun(void)
{
    size_t          bytes;
    static time_t   iwhen = 0;
    time_t          now;

    time(&now);
    if (now == iwhen) {
        current_proc_entry = 0;
        return;
    }
    iwhen = now;

#if defined(hpux10) || defined(hpux11)
    pstat_getdynamic(&pst_dyn, sizeof(struct pst_dynamic), 1, 0);
    nproc = pst_dyn.psd_activeprocs;
    bytes = nproc * sizeof(struct pst_status);
    if ((proc_table =
         (struct pst_status *) realloc(proc_table, bytes)) == NULL) {
        current_proc_entry = nproc + 1;
        return;
    }
    pstat_getproc(proc_table, sizeof(struct pst_status), nproc, 0);

#elif defined(solaris2)
    if (!getKstatInt("unix", "system_misc", "nproc", &nproc)) {
        current_proc_entry = nproc + 1;
        return;
    }
    bytes = nproc * sizeof(int);
    if ((proc_table = (int *) realloc(proc_table, bytes)) == NULL) {
        current_proc_entry = nproc + 1;
        return;
    }
    {
        DIR            *f;
        struct dirent  *dp;
#if _SLASH_PROC_METHOD_ == 0
        if (kd == NULL) {
            current_proc_entry = nproc + 1;
            return;
        }
#endif
        f = opendir("/proc");
        current_proc_entry = 0;
        while ((dp = readdir(f)) != NULL && current_proc_entry < nproc)
            if (dp->d_name[0] != '.')
                proc_table[current_proc_entry++] = atoi(dp->d_name);
        closedir(f);
    }
#elif HAVE_KVM_GETPROCS
    {
        if (kd == NULL) {
            nproc = 0;
            return;
        }
        proc_table = kvm_getprocs(kd, KERN_PROC_ALL, 0, &nproc);
    }
#else

    current_proc_entry = 1;
#ifndef bsdi2
    nproc = 0;

    if (auto_nlist(NPROC_SYMBOL, (char *) &nproc, sizeof(int)) == 0) {
        snmp_log_perror("Init_HR_SWRun-auto_nlist NPROC");
        return;
    }
#endif
    bytes = nproc * sizeof(struct proc);

    if (proc_table)
        free((char *) proc_table);
    if ((proc_table = (struct proc *) malloc(bytes)) == NULL) {
        nproc = 0;
        snmp_log_perror("Init_HR_SWRun-malloc");
        return;
    }

    {
        int             proc_table_base;
        if (auto_nlist
            (PROC_SYMBOL, (char *) &proc_table_base,
             sizeof(proc_table_base)) == 0) {
            nproc = 0;
            snmp_log_perror("Init_HR_SWRun-auto_nlist PROC");
            return;
        }
        if (klookup(proc_table_base, (char *) proc_table, bytes) == 0) {
            nproc = 0;
            snmp_log_perror("Init_HR_SWRun-klookup");
            return;
        }
    }
#endif
    current_proc_entry = 0;
}

int
Get_Next_HR_SWRun(void)
{
    while (current_proc_entry < nproc) {
#if defined(hpux10) || defined(hpux11)
        return proc_table[current_proc_entry++].pst_pid;
#elif defined(solaris2)
        return proc_table[current_proc_entry++];
#elif HAVE_KVM_GETPROCS
        if (proc_table[current_proc_entry].kp_proc.p_stat != 0)
            return proc_table[current_proc_entry++].kp_proc.p_pid;
#else
        if (proc_table[current_proc_entry].p_stat != 0)
            return proc_table[current_proc_entry++].p_pid;
        else
            ++current_proc_entry;
#endif

    }
    return -1;
}

void
End_HR_SWRun(void)
{
    current_proc_entry = nproc + 1;
}
#endif

int
count_processes(void)
{
#if !(defined(linux) || defined(cygwin)) || defined(hpux10) || defined(hpux11) || defined(solaris2) || HAVE_KVM_GETPROCS
    int             i;
#endif
    int             total = 0;

    Init_HR_SWRun();
#if defined(hpux10) || defined(hpux11) || HAVE_KVM_GETPROCS || defined(solaris2)
    total = nproc;
#else
#if !defined(linux) && !defined(cygwin) && !defined(dynix)
    for (i = 0; i < nproc; ++i) {
        if (proc_table[i].p_stat != 0)
#else
    while (Get_Next_HR_SWRun() != -1) {
#endif
        ++total;
    }
#endif                          /* !hpux10 && !hpux11 && !HAVE_KVM_GETPROCS && !solaris2 */
    End_HR_SWRun();
    return total;
}
