/*
 * util_funcs.c
 */
/*
 * Portions of this file are copyrighted by:
 * Copyright Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */

#include <net-snmp/net-snmp-config.h>

#if HAVE_IO_H
#include <io.h>
#endif
#include <stdio.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <sys/types.h>
#ifdef __alpha
#ifndef _BSD
#define _BSD
#define _myBSD
#endif
#endif
#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#ifdef __alpha
#ifdef _myBSD
#undef _BSD
#undef _myBSD
#endif
#endif
#ifndef WEXITSTATUS
# define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif
#ifndef WIFEXITED
# define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
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
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <errno.h>
#include <signal.h>
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <ctype.h>
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_BASETSD_H
#include <basetsd.h>
#define ssize_t SSIZE_T
#endif
#if HAVE_RAISE
#define alarm raise
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "struct.h"
#include "util_funcs.h"
#include "utilities/execute.h"

#if HAVE_LIMITS_H
#include "limits.h"
#endif
#ifdef USING_UCD_SNMP_ERRORMIB_MODULE
#include "ucd-snmp/errormib.h"
#else
#define setPerrorstatus(x) snmp_log_perror(x)
#endif


#ifdef NETSNMP_EXCACHETIME
static long     cachetime;
#endif

extern int      numprocs, numextens;

void
Exit(int var)
{
    snmp_log(LOG_ERR, "Server Exiting with code %d\n", var);
    exit(var);
}

const char *
make_tempfile(void)
{
    static char     name[32];
    int             fd = -1;

    strcpy(name, get_temp_file_pattern());
#ifdef HAVE_MKSTEMP
    fd = mkstemp(name);
#else
    if (mktemp(name)) {
# ifndef WIN32        
        fd = open(name, O_CREAT | O_EXCL | O_WRONLY);
# else
        /*
          Win32 needs _S_IREAD | _S_IWRITE to set permissions on file after closing
         */
        fd = _open(name, _O_CREAT, _S_IREAD | _S_IWRITE | _O_EXCL | _O_WRONLY);
# endif
    }
#endif
    if (fd >= 0) {
        close(fd);
        DEBUGMSGTL(("make_tempfile", "temp file created: %s\n", name));
        return name;
    }
    snmp_log(LOG_ERR,"make_tempfile: error creating file %s\n", name);
    return NULL;
}

int
shell_command(struct extensible *ex)
{
#if HAVE_SYSTEM
    const char     *ofname;
    char            shellline[STRMAX];
    FILE           *shellout;

    ofname = make_tempfile();
    if (ofname == NULL) {
        ex->output[0] = 0;
        ex->result = 127;
        return ex->result;
    }

    snprintf(shellline, sizeof(shellline), "%s > %s", ex->command, ofname);
    shellline[ sizeof(shellline)-1 ] = 0;
    ex->result = system(shellline);
    ex->result = WEXITSTATUS(ex->result);
    shellout = fopen(ofname, "r");
    if (shellout != NULL) {
        if (fgets(ex->output, sizeof(ex->output), shellout) == NULL) {
            ex->output[0] = 0;
        }
        fclose(shellout);
    }
    unlink(ofname);
#else
    ex->output[0] = 0;
    ex->result = 0;
#endif
    return (ex->result);
}

#define MAXOUTPUT 300

int
exec_command(struct extensible *ex)
{
#if defined (HAVE_EXECV) || defined (WIN32)
    int             fd;
    FILE           *file;

    if ((fd = get_exec_output(ex)) != -1) {
        file = fdopen(fd, "r");
        if (fgets(ex->output, sizeof(ex->output), file) == NULL) {
            ex->output[0] = 0;
        }
        fclose(file);
        wait_on_exec(ex);
    } else
#endif /* HAVE_EXECV */
    {
        ex->output[0] = 0;
        ex->result = 0;
    }
    return (ex->result);
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

void
wait_on_exec(struct extensible *ex)
{
#ifndef NETSNMP_EXCACHETIME
    if (ex->pid && waitpid(ex->pid, &ex->result, 0) < 0) {
        setPerrorstatus("waitpid");
    }
    ex->pid = 0;
#endif
}

#define MAXARGS 30

int
get_exec_output(struct extensible *ex)
{
#if HAVE_EXECV
    char            cachefile[STRMAX];
    char            cache[NETSNMP_MAXCACHESIZE];
    ssize_t         cachebytes;
    int             cfd;
#ifdef NETSNMP_EXCACHETIME
    long            curtime;
    static char     lastcmd[STRMAX];
    static int      lastresult;
#endif

    DEBUGMSGTL(("exec:get_exec_output","calling %s\n", ex->command));

    sprintf(cachefile, "%s/%s", get_persistent_directory(), NETSNMP_CACHEFILE);
#ifdef NETSNMP_EXCACHETIME
    curtime = time(NULL);
    if (curtime > (cachetime + NETSNMP_EXCACHETIME) ||
        strcmp(ex->command, lastcmd) != 0) {
        strcpy(lastcmd, ex->command);
        cachetime = curtime;
#endif

        cachebytes = NETSNMP_MAXCACHESIZE;
        ex->result = run_exec_command( ex->command, NULL, cache, &cachebytes );

        unlink(cachefile);
            /*
             * XXX  Use SNMP_FILEMODE_CLOSED instead of 644? 
             */
        if ((cfd = open(cachefile, O_WRONLY | O_TRUNC | O_CREAT, 0644)) < 0) {
                snmp_log(LOG_ERR,"can not create cache file\n");
                setPerrorstatus(cachefile);
                cachetime = 0;
                return -1;
        }
        if (cachebytes > 0)
            write(cfd, (void *) cache, cachebytes);
        close(cfd);
#ifdef NETSNMP_EXCACHETIME
        lastresult = ex->result;
    } else {
        ex->result = lastresult;
    }
#endif
    DEBUGMSGTL(("exec:get_exec_output","using cached value\n"));
    if ((cfd = open(cachefile, O_RDONLY)) < 0) {
        snmp_log(LOG_ERR,"can not open cache file\n");
        setPerrorstatus(cachefile);
        return -1;
    }
    return (cfd);
#else                           /* !HAVE_EXECV */
#if defined(WIN32) && !defined(HAVE_EXECV)
/* MSVC and MinGW.  Cygwin already works as it has execv and fork */
    int         fd;   
    
    /* Reference:  MS tech note: 190351 */
    HANDLE hOutputReadTmp, hOutputRead, hOutputWrite = NULL;
        
    HANDLE hErrorWrite;
    SECURITY_ATTRIBUTES sa;
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    
    sa.nLength= sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    DEBUGMSGTL(("exec:get_exec_output","calling %s\n", ex->command));
    
    /* Child temporary output pipe with Inheritance on (sa.bInheritHandle is true) */    
    if (!CreatePipe(&hOutputReadTmp,&hOutputWrite,&sa,0)) {
      DEBUGMSGTL(("util_funcs", "get_exec_pipes CreatePipe ChildOut: %d\n",
            GetLastError()));
      return -1;
    }
    
    /* Copy the stdout handle to the stderr handle in case the child closes one of 
     * its stdout handles. */
    if (!DuplicateHandle(GetCurrentProcess(),hOutputWrite, GetCurrentProcess(),
          &hErrorWrite,0, TRUE,DUPLICATE_SAME_ACCESS)) {
      DEBUGMSGTL(("util_funcs", "get_exec_output DuplicateHandle: %d\n", GetLastError()));
      return -1;
    }

    /* Create new copies of the input and output handles but set bInheritHandle to 
     * FALSE so the new handle can not be inherited.  Otherwise the handles can not
     * be closed.  */
    if (!DuplicateHandle(GetCurrentProcess(), hOutputReadTmp, GetCurrentProcess(),
          &hOutputRead, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
      DEBUGMSGTL(("util_funcs", "get_exec_output DupliateHandle ChildOut: %d\n", GetLastError()));
      CloseHandle(hErrorWrite);
      return -1;
    }   

    /* Close the temporary output and input handles */
    if (!CloseHandle(hOutputReadTmp)) {
      DEBUGMSGTL(("util_funcs", "get_exec_output CloseHandle (hOutputReadTmp): %d\n", GetLastError()));
      CloseHandle(hErrorWrite);
      CloseHandle(hOutputRead);
      return -1;
    }
    
    /* Associates a C run-time file descriptor with an existing operating-system file handle. */
    fd = _open_osfhandle((long) hOutputRead, 0);
    
    /* Set up STARTUPINFO for CreateProcess with the handles and have it hide the window
     * for the new process. */
    ZeroMemory(&si,sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = hOutputWrite;
    si.hStdError = hErrorWrite;
    si.wShowWindow = SW_HIDE;
   
    /* Launch the process that you want to redirect.  Example snmpd.conf pass_persist:
     * pass_persist    .1.3.6.1.4.1.2021.255  c:/perl/bin/perl c:/temp/pass_persisttest
    */
    if (!CreateProcess(NULL, ex->command, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
      DEBUGMSGTL(("util_funcs","get_exec_output CreateProcess:'%s' %d\n",ex->command, GetLastError()));
      CloseHandle(hErrorWrite);
      CloseHandle(hOutputRead);
      return -1;
    }
    
    /* Set global child process handle */
    ex->pid = (int)pi.hProcess;

    /* Close pipe handles to make sure that no handles to the write end of the
     * output pipe are maintained in this process or else the pipe will
     * not close when the child process exits and any calls to ReadFile 
     * will hang.
     */

    if (!CloseHandle(hOutputWrite)){
      DEBUGMSGTL(("util_funcs","get_exec_output CloseHandle hOutputWrite: %d\n",ex->command, GetLastError()));
      return -1;
    }
    if (!CloseHandle(hErrorWrite)) {
      DEBUGMSGTL(("util_funcs","get_exec_output CloseHandle hErrorWrite: %d\n",ex->command, GetLastError()));
      return -1;
    }
    return fd;
#endif                          /* WIN32 */
    return -1;
#endif
}
int
get_exec_pipes(char *cmd, int *fdIn, int *fdOut, int *pid)
{
/* 	Alexander Prömel, alexander@proemel.de 08/24/2006
	The following code, is tested on picotux rev. 1.01.
	I think, it will be better to put the named pipes, into /var/run or make it selectable via CONFIG file.
	If the pipe file already exist, the creation will fail.
	I put the pipes into /flash, the pipepath has to change in ucd-snmp/pass_persist.c too, if you change it here.
*/
#if HAVE_EXECV
#ifdef __uClinux__ /* HAVE uClinux */
	int in,out;
	char fifo_in_path[256];
	char fifo_out_path[256];
	pid_t tpid;
        
    if ((tpid = vfork()) == 0) { /*temp child*/
        execve(cmd, NULL,NULL);
        perror(cmd);
        exit(1);
    } else { 
		if(tpid > 0) {
			/*initialize workspace*/
			snprintf(fifo_in_path, 256, "/flash/cp_%d", tpid);
			snprintf(fifo_out_path, 256, "/flash/pc_%d", tpid);

			in = mkfifo(fifo_in_path, S_IRWXU);	/*Create Input Pipe, 700*/
			if ( in ) {
				perror("parent: inpipe");
				exit(0);
			}
			out = mkfifo(fifo_out_path, S_IRWXU);	/*Create Output Pipe, 700*/
			if ( out ) {
				perror("parent: outpipe");
				exit(0);
			}
						
			in = open(fifo_in_path,O_RDONLY);	/*open the Input Pipe read Only*/
			if(in < 0) {
				perror("parent: input");
				exit(0);
			}
			out = open(fifo_out_path,O_WRONLY); 	/*open the Output Pipe write Only*/
			if(out < 0) {
				perror("parent: output");
				exit(0);
			}
			
			*fdIn = in;	/*read*/
			*fdOut = out;	/*write*/
			*pid = tpid;	
			return (1);     /* We are returning 0 for error... */
		} else { /*pid < 0*/
			setPerrorstatus("vfork");
			return 0;
		}

    }
#else /*HAVE x86*/
    int             fd[2][2], i, cnt;
    char            ctmp[STRMAX], *cptr1, *cptr2, argvs[STRMAX], **argv,
        **aptr;
    /*
     * Setup our pipes 
     */
    if (pipe(fd[0]) || pipe(fd[1])) {
        setPerrorstatus("pipe");
        return 0;
    }
    if ((*pid = fork()) == 0) { /* First handle for the child */
        close(0);
        if (dup(fd[0][0]) != 0) {
            setPerrorstatus("dup 0");
            return 0;
        }
        close(1);
        if (dup(fd[1][1]) != 1) {
            setPerrorstatus("dup 1");
            return 0;
        }

        /*
         * write standard output and standard error to pipe. 
         */
        /*
         * close all non-standard open file descriptors 
         */
        for (cnt = getdtablesize() - 1; cnt >= 2; --cnt)
            (void) close(cnt);
        (void) dup(1);          /* stderr */

        for (cnt = 1, cptr1 = cmd, cptr2 = argvs; *cptr1 != 0;
             cptr2++, cptr1++) {
            *cptr2 = *cptr1;
            if (*cptr1 == ' ') {
                *(cptr2++) = 0;
                if ((cptr1 = skip_white(cptr1)) == NULL)
                    break;
                *cptr2 = *cptr1;
                if (*cptr1 != 0)
                    cnt++;
            }
        }
        *cptr2 = 0;
        *(cptr2 + 1) = 0;
        argv = (char **) malloc((cnt + 2) * sizeof(char *));
        if (argv == NULL)
            return 0;           /* memory alloc error */
        aptr = argv;
        *(aptr++) = argvs;
        for (cptr2 = argvs, i = 1; i != cnt; cptr2++)
            if (*cptr2 == 0) {
                *(aptr++) = cptr2 + 1;
                i++;
            }
        while (*cptr2 != 0)
            cptr2++;
        *(aptr++) = NULL;
        copy_nword(cmd, ctmp, sizeof(ctmp));
        execv(ctmp, argv);
        perror(ctmp);
        exit(1);
    } else {
        close(fd[0][0]);
        close(fd[1][1]);
        if (*pid < 0) {
            close(fd[0][1]);
            close(fd[1][0]);
            setPerrorstatus("fork");
            return 0;
        }
        *fdIn = fd[1][0];
        *fdOut = fd[0][1];
        return (1);             /* We are returning 0 for error... */
    }
#endif				/* uClinux or x86 */
#endif                          /* !HAVE_EXECV */
#if defined(WIN32) && !defined (mingw32) && !defined(HAVE_EXECV)
/* MSVC (MinGW not working but should use this code).  Cygwin already works as it has execv and fork */
    /* Reference:  MS tech note: 190351 */
    HANDLE hInputWriteTmp, hInputRead, hInputWrite = NULL;
    HANDLE hOutputReadTmp, hOutputRead, hOutputWrite = NULL;
        
    HANDLE hErrorWrite;
    SECURITY_ATTRIBUTES sa;
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    
    sa.nLength= sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    /* Child temporary output pipe with Inheritance on (sa.bInheritHandle is true) */    
    if (!CreatePipe(&hOutputReadTmp,&hOutputWrite,&sa,0)) {
      DEBUGMSGTL(("util_funcs", "get_exec_pipes CreatePipe ChildOut: %d\n",
            GetLastError()));
      return 0;
    }
    /* Child temporary input pipe with Inheritance on (sa.bInheritHandle is true) */    
    if (!CreatePipe(&hInputRead,&hInputWriteTmp,&sa,0)) {
      DEBUGMSGTL(("util_funcs", "get_exec_pipes CreatePipe ChildIn: %d\n", GetLastError()));
      return 0;
    }
    
    /* Copy the stdout handle to the stderr handle in case the child closes one of 
     * its stdout handles. */
    if (!DuplicateHandle(GetCurrentProcess(),hOutputWrite, GetCurrentProcess(),
          &hErrorWrite,0, TRUE,DUPLICATE_SAME_ACCESS)) {
      DEBUGMSGTL(("util_funcs", "get_exec_pipes DuplicateHandle: %d\n", GetLastError()));
      return 0;
    }

    /* Create new copies of the input and output handles but set bInheritHandle to 
     * FALSE so the new handle can not be inherited.  Otherwise the handles can not
     * be closed.  */
    if (!DuplicateHandle(GetCurrentProcess(), hOutputReadTmp, GetCurrentProcess(),
          &hOutputRead, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
      DEBUGMSGTL(("util_funcs", "get_exec_pipes DupliateHandle ChildOut: %d\n", GetLastError()));
      CloseHandle(hErrorWrite);
      return 0;
    }   
    if (!DuplicateHandle(GetCurrentProcess(),hInputWriteTmp,
          GetCurrentProcess(), &hInputWrite, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
      DEBUGMSGTL(("util_funcs","get_exec_pipes DupliateHandle ChildIn: %d\n", GetLastError()));
      CloseHandle(hErrorWrite);
      CloseHandle(hOutputRead);
      return 0;
    }

    /* Close the temporary output and input handles */
    if (!CloseHandle(hOutputReadTmp)) {
      DEBUGMSGTL(("util_funcs", "get_exec_pipes CloseHandle (hOutputReadTmp): %d\n", GetLastError()));
      CloseHandle(hErrorWrite);
      CloseHandle(hOutputRead);
      CloseHandle(hInputWrite);
      return 0;
    }
    if (!CloseHandle(hInputWriteTmp)) {
      DEBUGMSGTL(("util_funcs", "get_exec_pipes CloseHandle (hInputWriteTmp): %d\n", GetLastError()));
      CloseHandle(hErrorWrite);
      CloseHandle(hOutputRead);
      CloseHandle(hInputWrite);
      return 0;
    }
    
    /* Associates a C run-time file descriptor with an existing operating-system file handle. */
    *fdIn = _open_osfhandle((long) hOutputRead, 0);
    *fdOut = _open_osfhandle((long) hInputWrite, 0);
    
    /* Set up STARTUPINFO for CreateProcess with the handles and have it hide the window
     * for the new process. */
    ZeroMemory(&si,sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = hOutputWrite;
    si.hStdInput = hInputRead;
    si.hStdError = hErrorWrite;
    si.wShowWindow = SW_HIDE;
   
    /* Launch the process that you want to redirect.  Example snmpd.conf pass_persist:
     * pass_persist    .1.3.6.1.4.1.2021.255  c:/perl/bin/perl c:/temp/pass_persisttest
    */
    if (!CreateProcess(NULL, cmd, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
      DEBUGMSGTL(("util_funcs","get_exec_pipes CreateProcess:'%s' %d\n",cmd, GetLastError()));
      CloseHandle(hErrorWrite);
      CloseHandle(hOutputRead);
      CloseHandle(hInputWrite);
      return 0;
    }
    
    /* Set global child process handle */
    *pid = (int)pi.hProcess;

    /* Close pipe handles to make sure that no handles to the write end of the
     * output pipe are maintained in this process or else the pipe will
     * not close when the child process exits and any calls to ReadFile 
     * will hang.
     */

    if (!CloseHandle(hOutputWrite)){
      DEBUGMSGTL(("util_funcs","get_exec_pipes CloseHandle hOutputWrite: %d\n",cmd, GetLastError()));
      return 0;
    }
    if (!CloseHandle(hInputRead)) {
      DEBUGMSGTL(("util_funcs","get_exec_pipes CloseHandle hInputRead: %d\n",cmd, GetLastError()));
      return 0;
    }
    if (!CloseHandle(hErrorWrite)) {
      DEBUGMSGTL(("util_funcs","get_exec_pipes CloseHandle hErrorWrite: %d\n",cmd, GetLastError()));
      return 0;
    }
    return 1;
#endif                          /* WIN32 */
    return 0;
}

int
clear_cache(int action,
            u_char * var_val,
            u_char var_val_type,
            size_t var_val_len,
            u_char * statP, oid * name, size_t name_len)
{

    long            tmp = 0;

    if (var_val_type != ASN_INTEGER) {
        snmp_log(LOG_NOTICE, "Wrong type != int\n");
        return SNMP_ERR_WRONGTYPE;
    }
    tmp = *((long *) var_val);
    if (tmp == 1 && action == COMMIT) {
#ifdef NETSNMP_EXCACHETIME
        cachetime = 0;          /* reset the cache next read */
#endif
    }
    return SNMP_ERR_NOERROR;
}

char          **argvrestartp, *argvrestartname, *argvrestart;

RETSIGTYPE
restart_doit(int a)
{
    char * name = netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, 
                                        NETSNMP_DS_LIB_APPTYPE);
    snmp_shutdown(name);

    /*
     * do the exec 
     */
#if HAVE_EXECV
    execv(argvrestartname, argvrestartp);
    setPerrorstatus(argvrestartname);
#endif
}

int
restart_hook(int action,
             u_char * var_val,
             u_char var_val_type,
             size_t var_val_len,
             u_char * statP, oid * name, size_t name_len)
{

    long            tmp = 0;

    if (var_val_type != ASN_INTEGER) {
        snmp_log(LOG_NOTICE, "Wrong type != int\n");
        return SNMP_ERR_WRONGTYPE;
    }
    tmp = *((long *) var_val);
    if (tmp == 1 && action == COMMIT) {
#ifdef SIGALRM
        signal(SIGALRM, restart_doit);
#endif
        alarm(NETSNMP_RESTARTSLEEP);
    }
    return SNMP_ERR_NOERROR;
}

void
print_mib_oid(oid name[], size_t len)
{
    char           *buffer;
    buffer = (char *) malloc(11 * len); /* maximum digit lengths for int32 + a '.' */
    if (!buffer) {
        snmp_log(LOG_ERR, "Malloc failed - out of memory?");
        return;
    }
    sprint_mib_oid(buffer, name, len);
    snmp_log(LOG_NOTICE, "Mib: %s\n", buffer);
    free(buffer);
}

void
sprint_mib_oid(char *buf, oid name[], size_t len)
{
    int             i;
    for (i = 0; i < (int) len; i++) {
        sprintf(buf, ".%d", (int) name[i]);
        while (*buf != 0)
            buf++;
    }
}

/*******************************************************************-o-******
 * header_simple_table
 *
 * Parameters:
 *	  *vp		 Variable data.
 *	  *name		 Fully instantiated OID name.
 *	  *length	 Length of name.
 *	   exact	 TRUE if an exact match is desired.
 *	  *var_len	 Hook for size of returned data type.
 *	(**write_method) Hook for write method (UNUSED).
 *	   max
 *      
 * Returns:
 *	0	If name matches vp->name (accounting for 'exact') and is
 *			not greater in length than 'max'.
 *	1	Otherwise.
 *
 *
 * Compare 'name' to vp->name for the best match or an exact match (if
 *	requested).  Also check that 'name' is not longer than 'max' if
 *	max is greater-than/equal 0.
 * Store a successful match in 'name', and increment the OID instance if
 *	the match was not exact.  
 *
 * 'name' and 'length' are undefined upon failure.
 *
 */
int
header_simple_table(struct variable *vp, oid * name, size_t * length,
                    int exact, size_t * var_len,
                    WriteMethod ** write_method, int max)
{
    int             i, rtest;   /* Set to:      -1      If name < vp->name,
                                 *              1       If name > vp->name,
                                 *              0       Otherwise.
                                 */
    oid             newname[MAX_OID_LEN];

    for (i = 0, rtest = 0;
         i < (int) vp->namelen && i < (int) (*length) && !rtest; i++) {
        if (name[i] != vp->name[i]) {
            if (name[i] < vp->name[i])
                rtest = -1;
            else
                rtest = 1;
        }
    }
    if (rtest > 0 ||
        (exact == 1
         && (rtest || (int) *length != (int) (vp->namelen + 1)))) {
        if (var_len)
            *var_len = 0;
        return MATCH_FAILED;
    }

    memset(newname, 0, sizeof(newname));

    if (((int) *length) <= (int) vp->namelen || rtest == -1) {
        memmove(newname, vp->name, (int) vp->namelen * sizeof(oid));
        newname[vp->namelen] = 1;
        *length = vp->namelen + 1;
    } else if (((int) *length) > (int) vp->namelen + 1) {       /* exact case checked earlier */
        *length = vp->namelen + 1;
        memmove(newname, name, (*length) * sizeof(oid));
        if (name[*length - 1] < ULONG_MAX) {
            newname[*length - 1] = name[*length - 1] + 1;
        } else {
            /*
             * Careful not to overflow...  
             */
            newname[*length - 1] = name[*length - 1];
        }
    } else {
        *length = vp->namelen + 1;
        memmove(newname, name, (*length) * sizeof(oid));
        if (!exact) {
            if (name[*length - 1] < ULONG_MAX) {
                newname[*length - 1] = name[*length - 1] + 1;
            } else {
                /*
                 * Careful not to overflow...  
                 */
                newname[*length - 1] = name[*length - 1];
            }
        } else {
            newname[*length - 1] = name[*length - 1];
        }
    }
    if ((max >= 0 && ((int)newname[*length - 1] > max)) ||
               ( 0 == newname[*length - 1] )) {
        if (var_len)
            *var_len = 0;
        return MATCH_FAILED;
    }

    memmove(name, newname, (*length) * sizeof(oid));
    if (write_method)
        *write_method = 0;
    if (var_len)
        *var_len = sizeof(long);        /* default */
    return (MATCH_SUCCEEDED);
}

/*
 * header_generic(...
 * Arguments:
 * vp     IN      - pointer to variable entry that points here
 * name    IN/OUT  - IN/name requested, OUT/name found
 * length  IN/OUT  - length of IN/OUT oid's 
 * exact   IN      - TRUE if an exact match was requested
 * var_len OUT     - length of variable or 0 if function returned
 * write_method
 * 
 */

/*******************************************************************-o-******
 * generic_header
 *
 * Parameters:
 *	  *vp	   (I)     Pointer to variable entry that points here.
 *	  *name	   (I/O)   Input name requested, output name found.
 *	  *length  (I/O)   Length of input and output oid's.
 *	   exact   (I)     TRUE if an exact match was requested.
 *	  *var_len (O)     Length of variable or 0 if function returned.
 *	(**write_method)   Hook to name a write method (UNUSED).
 *      
 * Returns:
 *	MATCH_SUCCEEDED	If vp->name matches name (accounting for exact bit).
 *	MATCH_FAILED	Otherwise,
 *
 *
 * Check whether variable (vp) matches name.
 */
int
header_generic(struct variable *vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method)
{
    oid             newname[MAX_OID_LEN];
    int             result;

    DEBUGMSGTL(("util_funcs", "header_generic: "));
    DEBUGMSGOID(("util_funcs", name, *length));
    DEBUGMSG(("util_funcs", " exact=%d\n", exact));

    memcpy((char *) newname, (char *) vp->name,
           (int) vp->namelen * sizeof(oid));
    newname[vp->namelen] = 0;
    result = snmp_oid_compare(name, *length, newname, vp->namelen + 1);
    DEBUGMSGTL(("util_funcs", "  result: %d\n", result));
    if ((exact && (result != 0)) || (!exact && (result >= 0)))
        return (MATCH_FAILED);
    memcpy((char *) name, (char *) newname,
           ((int) vp->namelen + 1) * sizeof(oid));
    *length = vp->namelen + 1;

    *write_method = 0;
    *var_len = sizeof(long);    /* default to 'long' results */
    return (MATCH_SUCCEEDED);
}

/*
 * checkmib(): provided for backwards compatibility, do not use: 
 */
int
checkmib(struct variable *vp, oid * name, size_t * length,
         int exact, size_t * var_len, WriteMethod ** write_method, int max)
{
    /*
     * checkmib used to be header_simple_table, with reveresed boolean
     * return output.  header_simple_table() was created to match
     * header_generic(). 
     */
    return (!header_simple_table(vp, name, length, exact, var_len,
                                 write_method, max));
}

char           *
find_field(char *ptr, int field)
{
    int             i;
    char           *init = ptr;

    if (field == NETSNMP_LASTFIELD) {
        /*
         * skip to end 
         */
        while (*ptr++);
        ptr = ptr - 2;
        /*
         * rewind a field length 
         */
        while (*ptr != 0 && isspace(*ptr) && init <= ptr)
            ptr--;
        while (*ptr != 0 && !isspace(*ptr) && init <= ptr)
            ptr--;
        if (isspace(*ptr))
            ptr++;              /* past space */
        if (ptr < init)
            ptr = init;
        if (!isspace(*ptr) && *ptr != 0)
            return (ptr);
    } else {
        if ((ptr = skip_white(ptr)) == NULL)
            return (NULL);
        for (i = 1; *ptr != 0 && i != field; i++) {
            if ((ptr = skip_not_white(ptr)) == NULL)
                return (NULL);
            if ((ptr = skip_white(ptr)) == NULL)
                return (NULL);
        }
        if (*ptr != 0 && i == field)
            return (ptr);
        return (NULL);
    }
    return (NULL);
}

int
parse_miboid(const char *buf, oid * oidout)
{
    int             i;

    if (!buf)
        return 0;
    if (*buf == '.')
        buf++;
    for (i = 0; isdigit(*buf); i++) {
        oidout[i] = atoi(buf);
        while (isdigit(*buf++));
        if (*buf == '.')
            buf++;
    }
    /*
     * oidout[i] = -1; hmmm 
     */
    return i;
}

void
string_append_int(char *s, int val)
{
    char            textVal[16];

    if (val < 10) {
        *s++ = '0' + val;
        *s = '\0';
        return;
    }
    sprintf(textVal, "%d", val);
    strcpy(s, textVal);
    return;
}

struct internal_mib_table {
    int             max_size;   /* Size of the current data table */
    int             next_index; /* Index of the next free entry */
    int             current_index;      /* Index of the 'current' entry */
    int             cache_timeout;
    marker_t        cache_marker;
    RELOAD         *reload;     /* Routine to read in the data */
    COMPARE        *compare;    /* Routine to compare two entries */
    int             data_size;  /* Size of an individual entry */
    void           *data;       /* The table itself */
};

mib_table_t
Initialise_Table(int size, int timeout, RELOAD *reload, COMPARE *compare)
{
    struct internal_mib_table *t;

    t = (struct internal_mib_table *)
        malloc(sizeof(struct internal_mib_table));
    if (t == NULL)
        return NULL;

    t->max_size = 0;
    t->next_index = 1;          /* Don't use index 0 */
    t->current_index = 1;
    t->cache_timeout = timeout;
    t->cache_marker = NULL;
    t->reload = reload;
    t->compare = compare;
    t->data_size = size;
    t->data = NULL;

    return (mib_table_t) t;
}

#define TABLE_ADD( x, y )	((void*)((char*)(x) + y))
#define TABLE_INDEX(t, i)	(TABLE_ADD(t->data, i * t->data_size))
#define TABLE_START(t)		(TABLE_INDEX(t, 1))
#define TABLE_NEXT(t)		(TABLE_INDEX(t, t->next_index))
#define TABLE_CURRENT(t)	(TABLE_INDEX(t, t->current_index))

int
check_and_reload_table(struct internal_mib_table *table)
{
    /*
     * If the saved data is fairly recent,
     *    we don't need to reload it
     */
    if (table->cache_marker &&
        !(atime_ready(table->cache_marker, table->cache_timeout * 1000)))
        return 1;


    /*
     * Call the routine provided to read in the data
     *
     * N.B:  Update the cache marker *before* calling
     *   this routine, to avoid problems with recursion
     */
    if (!table->cache_marker)
        table->cache_marker = atime_newMarker();
    else
        atime_setMarker(table->cache_marker);

    table->next_index = 1;
    if (table->reload((mib_table_t) table) < 0) {
        free(table->cache_marker);
        table->cache_marker = NULL;
        return 0;
    }
    table->current_index = 1;
    if (table->compare != NULL) /* Sort the table */
        qsort(TABLE_START(table), table->next_index,
              table->data_size, table->compare);
    return 1;
}

int
Search_Table(mib_table_t t, void *entry, int exact)
{
    struct internal_mib_table *table = (struct internal_mib_table *) t;
    void           *entry2;
    int             res;

    if (!check_and_reload_table(table))
        return -1;

    if (table->compare == NULL) {
        /*
         * XXX - not sure this is right ? 
         */
        memcpy(entry, table->data, table->data_size);
        return 0;
    }

    if (table->next_index == table->current_index)
        table->current_index = 1;

    entry2 = TABLE_CURRENT(table);
    res = table->compare(entry, entry2);
    if ((res < 0) && (table->current_index != 1)) {
        table->current_index = 1;
        entry2 = TABLE_CURRENT(table);
        res = table->compare(entry, entry2);
    }

    while (res > 0) {
        table->current_index++;
        if (table->next_index == table->current_index)
            return -1;
        entry2 = TABLE_CURRENT(table);
        res = table->compare(entry, entry2);
    }

    if (exact && res != 0)
        return -1;

    if (!exact && res == 0) {
        table->current_index++;
        if (table->next_index == table->current_index)
            return -1;
        entry2 = TABLE_CURRENT(table);
    }
    memcpy(entry, entry2, table->data_size);
    return 0;
}

int
Add_Entry(mib_table_t t, void *entry)
{
    struct internal_mib_table *table = (struct internal_mib_table *) t;
    int             new_max;
    void           *new_data;   /* Used for
                                 *      a) extending the data table
                                 *      b) the next entry to use
                                 */

    if (table->max_size <= table->next_index) {
        /*
         * Table is full, so extend it to double the size
         */
        new_max = 2 * table->max_size;
        if (new_max == 0)
            new_max = 10;       /* Start with 10 entries */

        new_data = (void *) malloc(new_max * table->data_size);
        if (new_data == NULL)
            return -1;

        if (table->data) {
            memcpy(new_data, table->data,
                   table->max_size * table->data_size);
            free(table->data);
        }
        table->data = new_data;
        table->max_size = new_max;
    }

    /*
     * Insert the new entry into the data array
     */
    new_data = TABLE_NEXT(table);
    memcpy(new_data, entry, table->data_size);
    table->next_index++;
    return 0;
}

void           *
Retrieve_Table_Data(mib_table_t t, int *max_idx)
{
    struct internal_mib_table *table = (struct internal_mib_table *) t;

    if (!check_and_reload_table(table))
        return NULL;
    *max_idx = table->next_index - 1;
    return table->data;
}
