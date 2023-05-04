/* $Id$ */
/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Copyright (C) 2002 Martin Roesch <roesch@sourcefire.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <dirent.h>
#include <fnmatch.h>
#endif /* !WIN32 */

#include <stdarg.h>
#include <syslog.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <pcap.h>
#include <timersub.h>
#include <pthread.h>
#include <string.h>

#ifdef HAVE_MALLINFO
#include <malloc.h>
static struct mallinfo mi;
#endif

#ifndef WIN32
#include <grp.h>
#include <pwd.h>
#include <netdb.h>
#include <limits.h>
#endif /* !WIN32 */

#include <fcntl.h>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include <zlib.h>

#include "snort.h"
#include "mstring.h"
#include "snort_debug.h"
#include "util.h"
#include "parser.h"
#include "sfdaq.h"
#include "build.h"
#include "plugbase.h"
#include "sf_types.h"
#include "sflsq.h"
#include "pcre.h"
#include "mpse.h"
#include "ppm.h"
#include "active.h"
#include "packet_time.h"
#include "control/sfcontrol.h"

#ifdef TARGET_BASED
#include "sftarget_reader.h"
#endif

#ifdef SIDE_CHANNEL
#include "sidechannel.h"
#endif

#ifdef WIN32
#include "win32/WIN32-Code/name.h"
#endif

#include "stream_common.h"
#include "memory_stats.h"

#ifdef PATH_MAX
#define PATH_MAX_UTIL PATH_MAX
#else
#define PATH_MAX_UTIL 1024
#endif /* PATH_MAX */

extern PreprocStatsFuncNode *preproc_stats_funcs;

// You may need to adjust this on the systems which don't have standard paths
// defined.
#ifndef _PATH_VARRUN
static char _PATH_VARRUN[STD_BUF];
#endif

/****************************************************************************
 * Store interesting data in memory that would not otherwise be visible
 * in a CORE(5) file
 ***************************************************************************/
#define SNORT_VERSION_STRING ("### Snort Version "VERSION" Build "BUILD"\n")
#define SNORT_VERSION_STRLEN sizeof(SNORT_VERSION_STRING)
char __snort_version_string[SNORT_VERSION_STRLEN];

void StoreSnortInfoStrings( void )
{
    strncpy(__snort_version_string, SNORT_VERSION_STRING,
            sizeof(__snort_version_string));
}
#undef SNORT_VERSION_STRING
#undef SNORT_VERSION_STRLEN

/****************************************************************************
 *
 * Function: CalcPct(uint64_t, uint64_t)
 *
 * Purpose:  Calculate the percentage of a value compared to a total
 *
 * Arguments: cnt => the numerator in the equation
 *            total => the denominator in the calculation
 *
 * Returns: pct -> the percentage of cnt to value
 *
 ****************************************************************************/
double CalcPct(uint64_t cnt, uint64_t total)
{
    double pct = 0.0;

    if (total == 0.0)
    {
        pct = (double)cnt;
    }
    else
    {
        pct = (double)cnt / (double)total;
    }

    pct *= 100.0;

    return pct;
}


/****************************************************************************
 *
 * Function: DisplayBanner()
 *
 * Purpose:  Show valuable proggie info
 *
 * Arguments: None.
 *
 * Returns: 0 all the time
 *
 ****************************************************************************/
int DisplayBanner(void)
{
    const char * info;
    const char * pcre_ver;
    const char * zlib_ver;

    info = getenv("HOSTTYPE");
    if( !info )
    {
        info="";
    }

    pcre_ver = pcre_version();
    zlib_ver = zlib_version;

    LogMessage("\n");
    LogMessage("   ,,_     -*> Snort! <*-\n");
    LogMessage("  o\"  )~   Version %s%s (Build %s) %s\n",
               VERSION,
#ifdef GRE
               " GRE",
#else
               "",
#endif
               BUILD,
               info);
    LogMessage("   ''''    By Martin Roesch & The Snort Team: http://www.snort.org/contact#team\n");
    LogMessage("           Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.\n");
    LogMessage("           Copyright (C) 1998-2013 Sourcefire, Inc., et al.\n");
#ifdef HAVE_PCAP_LIB_VERSION
    LogMessage("           Using %s\n", pcap_lib_version());
#endif
    LogMessage("           Using PCRE version: %s\n", pcre_ver);
    LogMessage("           Using ZLIB version: %s\n", zlib_ver);
    LogMessage("\n");

    return 0;
}



/****************************************************************************
 *
 * Function: ts_print(register const struct, char *)
 *
 * Purpose: Generate a time stamp and stuff it in a buffer.  This one has
 *          millisecond precision.  Oh yeah, I ripped this code off from
 *          TCPdump, props to those guys.
 *
 * Arguments: timeval => clock struct coming out of libpcap
 *            timebuf => buffer to stuff timestamp into
 *
 * Returns: void function
 *
 ****************************************************************************/
void ts_print(register const struct timeval *tvp, char *timebuf)
{
    register int s;
    int    localzone;
    time_t Time;
    struct timeval tv;
    struct timezone tz;
    struct tm *lt;    /* place to stick the adjusted clock data */

    /* if null was passed, we use current time */
    if(!tvp)
    {
        /* manual page (for linux) says tz is never used, so.. */
        memset((char *) &tz, 0, sizeof(tz));
        gettimeofday(&tv, &tz);
        tvp = &tv;
    }

    localzone = snort_conf->thiszone;

    /*
    **  If we're doing UTC, then make sure that the timezone is correct.
    */
    if (ScOutputUseUtc())
        localzone = 0;

    s = (tvp->tv_sec + localzone) % 86400;
    Time = (tvp->tv_sec + localzone) - s;

    lt = gmtime(&Time);

    if(!lt)
    {
         /* Invalid time, set to 0*/
         s = 0;
         Time = 0;
         lt = gmtime(&Time);
    }

    if (ScOutputIncludeYear())
    {
        int year = (lt->tm_year >= 100) ? (lt->tm_year - 100) : lt->tm_year;
        (void) SnortSnprintf(timebuf, TIMEBUF_SIZE,
                        "%02d/%02d/%02d-%02d:%02d:%02d.%06u ",
                        lt->tm_mon + 1, lt->tm_mday, year,
                        s / 3600, (s % 3600) / 60, s % 60,
                        (u_int) tvp->tv_usec);
    }
    else
    {
        (void) SnortSnprintf(timebuf, TIMEBUF_SIZE,
                        "%02d/%02d-%02d:%02d:%02d.%06u ", lt->tm_mon + 1,
                        lt->tm_mday, s / 3600, (s % 3600) / 60, s % 60,
                        (u_int) tvp->tv_usec);
    }
}



/****************************************************************************
 *
 * Function: gmt2local(time_t)
 *
 * Purpose: Figures out how to adjust the current clock reading based on the
 *          timezone you're in.  Ripped off from TCPdump.
 *
 * Arguments: time_t => offset from GMT
 *
 * Returns: offset seconds from GMT
 *
 ****************************************************************************/
int gmt2local(time_t t)
{
    register int dt, dir;
    register struct tm *gmt, *loc;
    struct tm sgmt;

    if(t == 0)
        t = time(NULL);

    gmt = &sgmt;
    *gmt = *gmtime(&t);
    loc = localtime(&t);

    dt = (loc->tm_hour - gmt->tm_hour) * 60 * 60 +
        (loc->tm_min - gmt->tm_min) * 60;

    dir = loc->tm_year - gmt->tm_year;

    if(dir == 0)
        dir = loc->tm_yday - gmt->tm_yday;

    dt += dir * 24 * 60 * 60;

    return(dt);
}




/****************************************************************************
 *
 * Function: copy_argv(u_char **)
 *
 * Purpose: Copies a 2D array (like argv) into a flat string.  Stolen from
 *          TCPDump.
 *
 * Arguments: argv => 2D array to flatten
 *
 * Returns: Pointer to the flat string
 *
 ****************************************************************************/
char *copy_argv(char **argv)
{
    char **p;
    u_int len = 0;
    char *buf;
    char *src, *dst;
    //void ftlerr(char *,...);

    p = argv;
    if(*p == 0)
        return 0;

    while(*p)
        len += strlen(*p++) + 1;

    buf = (char *) calloc(1,len);

    if(buf == NULL)
    {
        FatalError("calloc() failed: %s\n", strerror(errno));
    }
    p = argv;
    dst = buf;

    while((src = *p++) != NULL)
    {
        while((*dst++ = *src++) != '\0');
        dst[-1] = ' ';
    }

    dst[-1] = '\0';

    /* Check for an empty string */
    dst = buf;
    while (isspace((int)*dst))
        dst++;

    if (strlen(dst) == 0)
    {
        free(buf);
        buf = NULL;
    }

    return buf;
}


/****************************************************************************
 *
 * Function: strip(char *)
 *
 * Purpose: Strips a data buffer of CR/LF/TABs.  Replaces CR/LF's with
 *          NULL and TABs with spaces.
 *
 * Arguments: data => ptr to the data buf to be stripped
 *
 * Returns: void
 *
 * 3/7/07 - changed to return void - use strlen to get size of string
 *
 * Note that this function will turn all '\n' and '\r' into null chars
 * so, e.g. 'Hello\nWorld\n' => 'Hello\x00World\x00'
 * note that the string is now just 'Hello' and the length is shortened
 * by more than just an ending '\n' or '\r'
 ****************************************************************************/
void strip(char *data)
{
    int size;
    char *end;
    char *idx;

    idx = data;
    end = data + strlen(data);
    size = end - idx;

    while(idx != end)
    {
        if((*idx == '\n') ||
                (*idx == '\r'))
        {
            *idx = 0;
            size--;
        }
        if(*idx == '\t')
        {
            *idx = ' ';
        }
        idx++;
    }
}

/*
 * Function: ErrorMessage(const char *, ...)
 *
 * Purpose: Print a message to stderr.
 *
 * Arguments: format => the formatted error string to print out
 *            ... => format commands/fillers
 *
 * Returns: void function
 */
void ErrorMessage(const char *format,...)
{
    char buf[STD_BUF+1];
    va_list ap;

    if (snort_conf == NULL)
        return;

    va_start(ap, format);

    if (ScDaemonMode() || ScLogSyslog())
    {
        vsnprintf(buf, STD_BUF, format, ap);
        buf[STD_BUF] = '\0';
        syslog(LOG_CONS | LOG_DAEMON | LOG_ERR, "%s", buf);
    }
    else
    {
        vfprintf(stderr, format, ap);
    }
    va_end(ap);
}

/*
 * Function: ErrorMessageThrottled(ThrottleInfo *,const char *, ...)
 *
 * Purpose: Print a message to stderr, and throttle when
 *          too many messages are printed.
 *
 * Arguments: throttleInfo => point to the saved throttle state information
 *            format => the formatted error string to print out
 *            ... => format commands/fillers
 *
 * Returns: void function
 */

void ErrorMessageThrottled(ThrottleInfo *throttleInfo, const char *format,...)
{
    char buf[STD_BUF+1];
    va_list ap;
    time_t current_time = packet_time();

    if ((snort_conf == NULL)||(!throttleInfo))
        return;

    if (!ScCheckInternalLogLevel(INTERNAL_LOG_LEVEL__ERROR))
        return;

    throttleInfo->count++;
    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"current_time: %d, throttle (%p): count "STDu64", last update: %d\n",
            (int)current_time, throttleInfo, throttleInfo->count, (int)throttleInfo->lastUpdate );)
    /*Note: we only output the first error message,
     * and the statistics after at least duration_to_log seconds
     * when the same type of error message is printed out again */
    if (current_time - (time_t)throttleInfo->duration_to_log > throttleInfo->lastUpdate)
    {
        int index;
        va_start(ap, format);
        index = vsnprintf(buf, STD_BUF, format, ap);
        va_end(ap);

        if (index && (throttleInfo->count > 1))
        {
           snprintf(&buf[index - 1], STD_BUF-index,
                   " (suppressed "STDu64" times in the last %d seconds).\n",
                   throttleInfo->count, (int) (current_time - throttleInfo->lastUpdate));
        }

        ErrorMessage("%s",buf);
        throttleInfo->lastUpdate = current_time;
        throttleInfo->count = 0;
    }

}

/*
 * Function: LogThrottledByTimeCount(ThrottleInfo *,const char *, ...)
 *   
 * Purpose: Print a message based on time and count of messages.
 *      
 * Arguments: throttleInfo => point to the saved throttle state information
 *            format => the formatted message string to print out
 *            ... => format commands/fillers
 *          
 * Returns: void function
 */            
void LogThrottledByTimeCount(ThrottleInfo *throttleInfo, const char *format,...)
{
    char buf[STD_BUF+1];
    va_list ap;
    time_t current_time = packet_time();

    if ((!snort_conf) || (!throttleInfo))
        return;

    if (!ScCheckInternalLogLevel(INTERNAL_LOG_LEVEL__ERROR))
        return;

    throttleInfo->count++;
    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"current_time: %d, throttle (%p): count "STDu64", last update: %d\n",
                (int)current_time, throttleInfo, throttleInfo->count, (int)throttleInfo->lastUpdate );)

    if ((throttleInfo->lastUpdate == 0)
           || ((current_time - (time_t)throttleInfo->duration_to_log > throttleInfo->lastUpdate)
           && ((throttleInfo->count == 1)
           || throttleInfo->count > throttleInfo->count_to_log)))
    {
       int index;
       va_start(ap, format);
       index = vsnprintf(buf, STD_BUF, format, ap);
       va_end(ap);

       if (index) 
       {
           snprintf(&buf[index - 1], STD_BUF-index,
               " (suppressed "STDu64" times in the last %d seconds).\n",
               throttleInfo->count, throttleInfo->lastUpdate
               ? ((int)(current_time - throttleInfo->lastUpdate))
               : ((int)throttleInfo->lastUpdate));
       }

       LogMessage("%s",buf);
       throttleInfo->lastUpdate = current_time;
       throttleInfo->count = 0;
    }
}

/*
 * Function: LogMessage(const char *, ...)
 *
 * Purpose: Print a message to stderr or with logfacility.
 *
 * Arguments: format => the formatted error string to print out
 *            ... => format commands/fillers
 *
 * Returns: void function
 */
void LogMessage(const char *format,...)
{
    char buf[STD_BUF+1];
    va_list ap;

    if (snort_conf == NULL)
        return;

    if (!ScCheckInternalLogLevel(INTERNAL_LOG_LEVEL__MESSAGE))
        return;

    va_start(ap, format);

    if (ScDaemonMode() || ScLogSyslog())
    {
        vsnprintf(buf, STD_BUF, format, ap);
        buf[STD_BUF] = '\0';
        syslog(LOG_DAEMON | LOG_NOTICE, "%s", buf);
    }
    else
    {
        vfprintf(stderr, format, ap);
    }

    va_end(ap);
}

/*
 * Function: WarningMessage(const char *, ...)
 *
 * Purpose: Print a message to stderr or with logfacility.
 *
 * Arguments: format => the formatted error string to print out
 *            ... => format commands/fillers
 *
 * Returns: void function
 */
void WarningMessage(const char *format,...)
{
    char buf[STD_BUF+1];
    va_list ap;

    if (snort_conf == NULL)
        return;

    if (!ScCheckInternalLogLevel(INTERNAL_LOG_LEVEL__WARNING))
        return;

    va_start(ap, format);

    if (ScDaemonMode() || ScLogSyslog())
    {
        vsnprintf(buf, STD_BUF, format, ap);
        buf[STD_BUF] = '\0';
        syslog(LOG_DAEMON | LOG_WARNING, "%s", buf);
    }
    else
    {
        vfprintf(stderr, format, ap);
    }

    va_end(ap);
}

/*
 * Function: CreateApplicationEventLogEntry(const char *)
 *
 * Purpose: Add an entry to the Win32 "Application" EventLog
 *
 * Arguments: szMessage => the formatted error string to print out
 *
 * Returns: void function
 */
#if defined(WIN32) && defined(ENABLE_WIN32_SERVICE)
void CreateApplicationEventLogEntry(const char *msg)
{
    HANDLE hEventLog;
    char*  pEventSourceName = "SnortService";

    /* prepare to write to Application log on local host
      * with Event Source of SnortService
      */
    AddEventSource(pEventSourceName);
    hEventLog = RegisterEventSource(NULL, pEventSourceName);
    if (hEventLog == NULL)
    {
        /* Could not register the event source. */
        return;
    }

    if (!ReportEvent(hEventLog,   /* event log handle               */
            EVENTLOG_ERROR_TYPE,  /* event type                     */
            0,                    /* category zero                  */
            EVMSG_SIMPLE,         /* event identifier               */
            NULL,                 /* no user security identifier    */
            1,                    /* one substitution string        */
            0,                    /* no data                        */
            &msg,                 /* pointer to array of strings    */
            NULL))                /* pointer to data                */
    {
        /* Could not report the event. */
    }

    DeregisterEventSource(hEventLog);
}
#endif  /* WIN32 && ENABLE_WIN32_SERVICE */


static int already_fatal = 0;

/*
 * Function: SnortFatalExit(void)
 *
 * Purpose: When a fatal error occurs, this function cleanly
 *          shuts down the program
 *
 * Arguments: none
 *
 * Returns: void function
 */
NORETURN void SnortFatalExit(void)
{
    // -----------------------------
    // bail now if we are reentering
    if ( already_fatal )
        exit(1);
    else
        already_fatal = 1;

    if (!snort_conf || (!ScDaemonMode() && !ScLogSyslog()))
        fprintf(stderr,"Fatal Error, Quitting..\n");

    if ( InMainThread() || SnortIsInitializing() )
    {
        DAQ_Abort();
        exit(1);
    }
    else
    {
        DAQ_BreakLoop(1);
#ifndef WIN32
        pthread_exit(NULL);
#endif
    }
}

/*
 * Function: FatalError(const char *, ...)
 *
 * Purpose: When a fatal error occurs, this function prints the error message
 *          and cleanly shuts down the program
 *
 * Arguments: format => the formatted error string to print out
 *            ... => format commands/fillers
 *
 * Returns: void function
 */
NORETURN void FatalError(const char *format,...)
{
    char buf[STD_BUF+1];
    va_list ap;

    // -----------------------------
    // bail now if we are reentering
    if ( already_fatal )
        exit(1);
    else
        already_fatal = 1;
    // -----------------------------

    va_start(ap, format);
    vsnprintf(buf, STD_BUF, format, ap);
    va_end(ap);

    buf[STD_BUF] = '\0';

    if ((snort_conf != NULL) && (ScDaemonMode() || ScLogSyslog()))
    {
        syslog(LOG_CONS | LOG_DAEMON | LOG_ERR, "FATAL ERROR: %s", buf);
    }
    else
    {
        fprintf(stderr, "ERROR: %s", buf);
        fprintf(stderr,"Fatal Error, Quitting..\n");
#if defined(WIN32) && defined(ENABLE_WIN32_SERVICE)
        CreateApplicationEventLogEntry(buf);
#endif
    }

    if ( InMainThread() || SnortIsInitializing() )
    {
        if (!SnortIsInitializing()) 
        {
            /*
             * Shutdown the thread only when the snort is not 
             * initializing. Because FatalError api can be 
             * called during initialization as well.
             */
            SnortShutdownThreads(1);
        }
        DAQ_Abort();
        exit(1);
    }
    else
    {
        DAQ_BreakLoop(1);
#ifndef WIN32
        pthread_exit(NULL);
#endif
    }
}


/****************************************************************************
 *
 * Function: CreatePidFile(char *)
 *
 * Purpose:  Creates a PID file
 *
 * Arguments: Interface opened.
 *
 * Returns: void function
 *
 ****************************************************************************/
static FILE *pid_lockfile = NULL;
static FILE *pid_file = NULL;

void CreatePidFile(const char *intf, pid_t pid)
{
    struct stat pt;
#ifdef WIN32
    char dir[STD_BUF + 1];
#endif

    if (!ScReadMode())
    {
        LogMessage("Checking PID path...\n");

        if (strlen(snort_conf->pid_path) != 0)
        {
            if((stat(snort_conf->pid_path, &pt) == -1) ||
                !S_ISDIR(pt.st_mode) || access(snort_conf->pid_path, W_OK) == -1)
            {
#ifndef WIN32
                /* Save this just in case it's reset with LogMessage call */
                int err = errno;

                LogMessage("WARNING: %s is invalid, trying "
                           "/var/run...\n", snort_conf->pid_path);
                if (err)
                {
                    LogMessage("Previous Error, errno=%d, (%s)\n",
                               err, strerror(err) == NULL ? "Unknown error" : strerror(err));
                }
#endif
                memset(snort_conf->pid_path, 0, sizeof(snort_conf->pid_path));
            }
            else
            {
                LogMessage("PID path stat checked out ok, "
                           "PID path set to %s\n", snort_conf->pid_path);
            }
        }

        if (strlen(snort_conf->pid_path) == 0)
        {
#ifndef _PATH_VARRUN
# ifndef WIN32
            SnortStrncpy(_PATH_VARRUN, "/var/run/", sizeof(_PATH_VARRUN));
# else
            if (GetCurrentDirectory(sizeof(dir) - 1, dir))
                SnortStrncpy(_PATH_VARRUN, dir, sizeof(_PATH_VARRUN));
# endif  /* WIN32 */
#else
            LogMessage("PATH_VARRUN is set to %s on this operating "
                       "system\n", _PATH_VARRUN);
#endif  /* _PATH_VARRUN */

            if ((stat(_PATH_VARRUN, &pt) == -1) ||
                !S_ISDIR(pt.st_mode) || access(_PATH_VARRUN, W_OK) == -1)
            {
                LogMessage("WARNING: _PATH_VARRUN is invalid, trying "
                           "/var/log/ ...\n");
                SnortStrncpy(snort_conf->pid_path, "/var/log/", sizeof(snort_conf->pid_path));
                if ((stat(snort_conf->pid_path, &pt) == -1) ||
                    !S_ISDIR(pt.st_mode) || access(snort_conf->pid_path, W_OK) == -1)
                {
                    LogMessage("WARNING: %s is invalid, logging Snort "
                               "PID path to log directory (%s).\n", snort_conf->pid_path,
                               snort_conf->log_dir);
                    CheckLogDir();
                    SnortSnprintf(snort_conf->pid_path, sizeof(snort_conf->pid_path),
                                  "%s/", snort_conf->log_dir);
                }
            }
            else
            {
                LogMessage("PID path stat checked out ok, "
                           "PID path set to %s\n", _PATH_VARRUN);
                SnortStrncpy(snort_conf->pid_path, _PATH_VARRUN, sizeof(snort_conf->pid_path));
            }
        }
    }

    if(intf == NULL || strlen(snort_conf->pid_path) == 0)
    {
        /* snort_conf->pid_path should have some value by now
         * so let us just be sane. */
        FatalError("CreatePidFile() failed to lookup interface or pid_path is unknown!\n");
    }

    if (ScNoInterfacePidFile())
    {
        SnortSnprintf(snort_conf->pid_filename, sizeof(snort_conf->pid_filename),
                  "%s/snort%s.pid", snort_conf->pid_path, snort_conf->pidfile_suffix);
    }
    else
    {
        SnortSnprintf(snort_conf->pid_filename, sizeof(snort_conf->pid_filename),
                  "%s/snort_%s%s.pid", snort_conf->pid_path, intf, snort_conf->pidfile_suffix);
    }

#ifndef WIN32
    if (!ScNoLockPidFile())
    {
        char pid_lockfilename[STD_BUF+1];
        int lock_fd;

        /* First, lock the PID file */
        SnortSnprintf(pid_lockfilename, STD_BUF, "%s.lck", snort_conf->pid_filename);
        pid_lockfile = fopen(pid_lockfilename, "w");

        if (pid_lockfile)
        {
            struct flock lock;
            lock_fd = fileno(pid_lockfile);

            lock.l_type = F_WRLCK;
            lock.l_whence = SEEK_SET;
            lock.l_start = 0;
            lock.l_len = 0;

            if (fcntl(lock_fd, F_SETLK, &lock) == -1)
            {
                ClosePidFile();
                FatalError("Failed to Lock PID File \"%s\" for PID \"%d\"\n", snort_conf->pid_filename, (int)pid);
            }

            /* Give desired user control over lock file */
            fchown(fileno(pid_lockfile), ScUid(), ScGid());
        }
    }
#endif

    /* Okay, were able to lock PID file, now open and write PID */
    pid_file = fopen(snort_conf->pid_filename, "w");
    if(pid_file)
    {
        LogMessage("Writing PID \"%d\" to file \"%s\"\n", (int)pid, snort_conf->pid_filename);
        fprintf(pid_file, "%d\n", (int)pid);
        fflush(pid_file);
        
#ifndef WIN32
        /* Give desired user control over pid file */
        fchown(fileno(pid_file), ScUid(), ScGid());
#endif
    }
    else
    {
        char errBuf[STD_BUF];
#ifdef WIN32
		SnortSnprintf(errBuf, STD_BUF, "%s", strerror(errno));
#else
        strerror_r(errno, errBuf, STD_BUF);
#endif
        ErrorMessage("Failed to create pid file %s, Error: %s", snort_conf->pid_filename, errBuf);
        snort_conf->pid_filename[0] = 0;
    }
}

/****************************************************************************
 *
 * Function: ClosePidFile(char *)
 *
 * Purpose:  Releases lock on a PID file
 *
 * Arguments: None
 *
 * Returns: void function
 *
 ****************************************************************************/
void ClosePidFile(void)
{
    if (pid_file)
    {
        fclose(pid_file);
        pid_file = NULL;
    }
    if (pid_lockfile)
    {
        fclose(pid_lockfile);
        pid_lockfile = NULL;
    }
}

/****************************************************************************
 *
 * Function: SetUidGid()
 *
 * Purpose:  Sets safe UserID and GroupID if needed
 *
 * Arguments: none
 *
 * Returns: void function
 *
 ****************************************************************************/
void SetUidGid(int user_id, int group_id)
{
#ifndef WIN32

    if ((group_id != -1) && (getgid() != (gid_t)group_id))
    {
        if ( !DAQ_Unprivileged() )
        {
            LogMessage("WARNING: cannot set uid and gid - %s DAQ does not"
                " support unprivileged operation.\n", DAQ_GetType());
            return;
        }

        if (setgid(group_id) < 0)
            FatalError("Cannot set gid: %d\n", group_id);

        LogMessage("Set gid to %d\n", group_id);
    }

    if ((user_id != -1) && (getuid() != (uid_t)user_id))
    {
        if ( !DAQ_Unprivileged() )
        {
            LogMessage("WARNING: cannot set uid and gid - %s DAQ does not"
                " support unprivileged operation.\n", DAQ_GetType());
            return;
        }

        if (setuid(user_id) < 0)
            FatalError("Can not set uid: %d\n", user_id);

        LogMessage("Set uid to %d\n", user_id);
    }
#endif  /* WIN32 */
}

/****************************************************************************
 *
 * Function: InitGroups()
 *
 * Purpose:  Sets the groups of the process based on the UserID with the
 *           GroupID added
 *
 * Arguments: none
 *
 * Returns: void function
 *
 ****************************************************************************/
void InitGroups(int user_id, int group_id)
{
#ifndef WIN32

    if ((user_id != -1) && (getuid() == 0))
    {
        struct passwd *pw = getpwuid(user_id);

        if (pw != NULL)
        {
            /* getpwuid and initgroups may use the same static buffers */
            char *username = SnortStrdup(pw->pw_name);

            if (initgroups(username, group_id) < 0)
            {
                free(username);
                FatalError("Can not initgroups(%s,%d)", username, group_id);
            }

            free(username);
        }

        /** Just to be on the safe side... **/
        endgrent();
        endpwent();
    }
#endif  /* WIN32 */
}

//-------------------------------------------------------------------------

#define STATS_SEPARATOR \
    "==============================================================================="

static inline int DisplayCount (char *buf, const char* s, uint64_t n, int size)
{
    return snprintf(buf+size, CS_STATS_BUF_SIZE-size, "%11s: " FMTu64("12") "\n", s, n);
}

static inline int DisplayStat (char *buf, const char* s, uint64_t n, uint64_t tot, int size)
{
	return snprintf(buf+size, CS_STATS_BUF_SIZE-size,
        "%11s: " FMTu64("12") " (%7.3f%%)\n",
        s, n, CalcPct(n, tot));
}

static inline void LogCount (const char* s, uint64_t n)
{
    LogMessage("%11s: " FMTu64("12") "\n", s, n);
}

static inline void LogStat (const char* s, uint64_t n, uint64_t tot)
{
    LogMessage(
        "%11s: " FMTu64("12") " (%7.3f%%)\n",
        s, n, CalcPct(n, tot));
}

static struct timeval starttime;

void TimeStart (void)
{
    gettimeofday(&starttime, NULL);
}

void TimeStop (void)
{
    uint32_t days = 0, hrs = 0, mins = 0, secs = 0;
    uint32_t total_secs = 0, tmp = 0;
    uint64_t pps = 0;

    struct timeval endtime, difftime;
    gettimeofday(&endtime, NULL);
    TIMERSUB(&endtime, &starttime, &difftime);

    LogMessage("%s\n", STATS_SEPARATOR);

    LogMessage("Run time for packet processing was %lu.%lu seconds\n",
        (unsigned long)difftime.tv_sec, (unsigned long)difftime.tv_usec);

    LogMessage("Snort processed " STDu64 " packets.\n", pc.total_from_daq);

    tmp = total_secs = (uint32_t)difftime.tv_sec;
    if ( total_secs < 1 ) total_secs = 1;

    days = tmp / SECONDS_PER_DAY;
    tmp  = tmp % SECONDS_PER_DAY;

    hrs  = tmp / SECONDS_PER_HOUR;
    tmp  = tmp % SECONDS_PER_HOUR;

    mins = tmp / SECONDS_PER_MIN;
    secs = tmp % SECONDS_PER_MIN;

    LogMessage("Snort ran for %u days %u hours %u minutes %u seconds\n",
                 days, hrs, mins, secs);

    if ( days > 0 )
    {
        uint64_t n = (pc.total_from_daq / (total_secs / SECONDS_PER_DAY));
        LogCount("Pkts/day", n);
    }

    if ( hrs > 0 || days > 0 )
    {
        uint64_t n = (pc.total_from_daq / (total_secs / SECONDS_PER_HOUR));
        LogCount("Pkts/hr", n);
    }

    if ( mins > 0 || hrs > 0 || days > 0 )
    {
        uint64_t n = (pc.total_from_daq / (total_secs / SECONDS_PER_MIN));
        LogCount("Pkts/min", n);
    }

    pps = (pc.total_from_daq / total_secs);
    LogCount("Pkts/sec", pps);
}

//-------------------------------------------------------------------------

static const char* Verdicts[MAX_DAQ_VERDICT] = {
    "Allow",
    "Block",
    "Replace",
    "AllowFlow",
    "BlockFlow",
#ifdef HAVE_DAQ_VERDICT_RETRY
    "Ignore",
    "Retry"
#else
    "Ignore"
#endif
};

#ifdef HAVE_MALLINFO
static void display_mallinfo(void)
{
    mi = mallinfo();
    LogMessage("%s\n", STATS_SEPARATOR);
    LogMessage("Memory usage summary:\n");
    LogMessage("  Total non-mmapped bytes (arena):       %lu\n", (unsigned long)mi.arena);
    LogMessage("  Bytes in mapped regions (hblkhd):      %d\n", mi.hblkhd);
    LogMessage("  Total allocated space (uordblks):      %lu\n", (unsigned long)mi.uordblks);
    LogMessage("  Total free space (fordblks):           %d\n", mi.fordblks);
    LogMessage("  Topmost releasable block (keepcost):   %d\n", mi.keepcost);
#ifdef DEBUG
    LogMessage("  Number of free chunks (ordblks):       %d\n", mi.ordblks);
    LogMessage("  Number of free fastbin blocks (smblks):%d\n", mi.smblks);
    LogMessage("  Number of mapped regions (hblks):      %d\n", mi.hblks);
    LogMessage("  Max. total allocated space (usmblks):  %d\n", mi.usmblks);
    LogMessage("  Free bytes held in fastbins (fsmblks): %d\n", mi.fsmblks);
#endif

}
#endif

void DisplayActionStats (uint16_t type, void *old_context, struct _THREAD_ELEMENT *te, ControlDataSendFunc f)
{
    char buffer[CS_STATS_BUF_SIZE + 1];
    int len = 0;
    int i = 0;
    uint64_t total = pc.total_processed;
    uint64_t pkts_recv = pc.total_from_daq;
    const DAQ_Stats_t* pkt_stats = DAQ_GetStats();

    if (total) {
        // ensure proper counting of log_limit
        SnortEventqResetCounts();

        len += snprintf(buffer, CS_STATS_BUF_SIZE,  "Action Stats:\n");
        len += DisplayStat(buffer, "Alerts", pc.total_alert_pkts, total, len);
        len += DisplayStat(buffer, "Logged", pc.log_pkts, total, len);
        len += DisplayStat(buffer, "Passed", pc.pass_pkts, total, len);
        len += snprintf(buffer+len, CS_STATS_BUF_SIZE-len,  "Limits:\n");
        len += DisplayCount(buffer, "Match", pc.match_limit, len);
        len += DisplayCount(buffer, "Queue", pc.queue_limit, len);
        len += DisplayCount(buffer, "Log", pc.log_limit, len);
        len += DisplayCount(buffer, "Event", pc.event_limit, len);
        len += DisplayCount(buffer, "Alert", pc.alert_limit, len);
        len += snprintf(buffer+len, CS_STATS_BUF_SIZE-len,  "Verdicts:\n");

        for ( i = 0; i < MAX_DAQ_VERDICT && len < CS_STATS_BUF_SIZE; i++ ) {
            const char* s = Verdicts[i];
            len += DisplayStat(buffer, s, pkt_stats->verdicts[i], pkts_recv, len);
        }
        if ( pc.internal_blacklist )
            len += DisplayStat(buffer, "Int Blkflw", pc.internal_blacklist, pkts_recv, len);

        if ( pc.internal_whitelist )
            len += DisplayStat(buffer, "Int Alwflw", pc.internal_whitelist, pkts_recv, len);
    } else {
        len = snprintf(buffer, CS_STATS_BUF_SIZE,  "Action Stats are not available\n Total Action Processed:"FMTu64("12") "\n", total);
    }

    if (-1 == f(te, (const uint8_t *)buffer, len)) {
        LogMessage("Unable to send data to the frontend\n");
    }
}

/* exiting should be 0 for if not exiting, 1 if restarting, and 2 if exiting */
void DropStats(int exiting)
{
    PreprocStatsFuncNode *idx;
    uint64_t total = pc.total_processed;
    uint64_t pkts_recv = pc.total_from_daq;

    const DAQ_Stats_t* pkt_stats = DAQ_GetStats();

    /*Display all the memory usage in main arena*/
#ifdef HAVE_MALLINFO
    display_mallinfo();
#endif

#ifdef PPM_MGR
    PPM_PRINT_SUMMARY(&snort_conf->ppm_cfg);
#endif

    {
        uint64_t pkts_drop, pkts_out, pkts_inj;

        pkts_recv = pkt_stats->hw_packets_received;
        pkts_drop = pkt_stats->hw_packets_dropped;

        if ( pkts_recv > pkt_stats->packets_filtered
                       + pkt_stats->packets_received )
            pkts_out = pkts_recv - pkt_stats->packets_filtered
                     - pkt_stats->packets_received;
        else
            pkts_out = 0;

        pkts_inj = pkt_stats->packets_injected;
#ifdef ACTIVE_RESPONSE
        pkts_inj += Active_GetInjects();
#endif

        LogMessage("%s\n", STATS_SEPARATOR);
        LogMessage("Packet I/O Totals:\n");

        LogCount("Received", pkts_recv);
        LogStat("Analyzed", pkt_stats->packets_received, pkts_recv);
        LogStat("Dropped", pkts_drop, pkts_recv + pkts_drop);
        LogStat("Filtered", pkt_stats->packets_filtered, pkts_recv);
        LogStat("Outstanding", pkts_out, pkts_recv);
        LogCount("Injected", pkts_inj);

#ifdef REG_TEST
        if ( snort_conf->pkt_skip )
            LogCount("Skipped", snort_conf->pkt_skip);
#endif
    }

    LogMessage("%s\n", STATS_SEPARATOR);
    LogMessage("Breakdown by protocol (includes rebuilt packets):\n");

    LogStat("Eth", pc.eth, total);
    LogStat("VLAN", pc.vlan, total);

    if (pc.nested_vlan != 0)
        LogStat("Nested VLAN", pc.nested_vlan, total);

    LogStat("IP4", pc.ip, total);
    LogStat("Frag", pc.frags, total);
    LogStat("ICMP", pc.icmp, total);
    LogStat("UDP", pc.udp, total);
    LogStat("TCP", pc.tcp, total);

    LogStat("IP6", pc.ipv6, total);
    LogStat("IP6 Ext", pc.ip6ext, total);
    LogStat("IP6 Opts", pc.ipv6opts, total);
    LogStat("Frag6", pc.frag6, total);
    LogStat("ICMP6", pc.icmp6, total);
    LogStat("UDP6", pc.udp6, total);
    LogStat("TCP6", pc.tcp6, total);
    LogStat("Teredo", pc.teredo, total);

    LogStat("ICMP-IP", pc.embdip, total);

#ifndef NO_NON_ETHER_DECODER
    LogStat("EAPOL", pc.eapol, total);
#endif
#ifdef GRE
    LogStat("IP4/IP4", pc.ip4ip4, total);
    LogStat("IP4/IP6", pc.ip4ip6, total);
    LogStat("IP6/IP4", pc.ip6ip4, total);
    LogStat("IP6/IP6", pc.ip6ip6, total);

    LogStat("GRE", pc.gre, total);
    LogStat("GRE Eth", pc.gre_eth, total);
    LogStat("GRE VLAN", pc.gre_vlan, total);
    LogStat("GRE IP4", pc.gre_ip, total);
    LogStat("GRE IP6", pc.gre_ipv6, total);
    LogStat("GRE IP6 Ext", pc.gre_ipv6ext, total);
    LogStat("GRE PPTP", pc.gre_ppp, total);
    LogStat("GRE ARP", pc.gre_arp, total);
    LogStat("GRE IPX", pc.gre_ipx, total);
    LogStat("GRE Loop", pc.gre_loopback, total);
#endif  /* GRE */
#ifdef MPLS
    LogStat("MPLS", pc.mpls, total);
#endif
    LogStat("ARP", pc.arp, total);
    LogStat("IPX", pc.ipx, total);
    LogStat("Eth Loop", pc.ethloopback, total);
    LogStat("Eth Disc", pc.ethdisc, total);
    LogStat("IP4 Disc", pc.ipdisc, total);
    LogStat("IP6 Disc", pc.ipv6disc, total);
    LogStat("TCP Disc", pc.tdisc, total);
    LogStat("UDP Disc", pc.udisc, total);
    LogStat("ICMP Disc", pc.icmpdisc, total);
    LogStat("All Discard", pc.discards, total);

    LogStat("Other", pc.other, total);
    LogStat("Bad Chk Sum", pc.invalid_checksums, total);
    LogStat("Bad TTL", pc.bad_ttl, total);

    LogStat("S5 G 1", pc.s5tcp1, total);
    LogStat("S5 G 2", pc.s5tcp2, total);

    if ( InternalEventIsEnabled(snort_conf->rate_filter_config,
                INTERNAL_EVENT_SYN_RECEIVED) )
    {
        LogStat("SYN RL Evnt", pc.syn_rate_limit_events, total);
        LogStat("SYN RL Drop", pc.syn_rate_limit_drops, total);
    }

    LogCount("Total", total);

    if ( !ScPacketDumpMode() && !ScPacketLogMode() )
    {
        int i;

        // ensure proper counting of log_limit
        SnortEventqResetCounts();

        LogMessage("%s\n", STATS_SEPARATOR);
        LogMessage("Action Stats:\n");

        LogStat("Alerts", pc.total_alert_pkts, total);
        LogStat("Logged", pc.log_pkts, total);
        LogStat("Passed", pc.pass_pkts, total);

        LogMessage("Limits:\n");

        LogCount("Match", pc.match_limit);
        LogCount("Queue", pc.queue_limit);
        LogCount("Log", pc.log_limit);
        LogCount("Event", pc.event_limit);
        LogCount("Alert", pc.alert_limit);


        LogMessage("Verdicts:\n");

        for ( i = 0; i < MAX_DAQ_VERDICT; i++ )
        {
            const char* s = Verdicts[i];
            LogStat(s, pkt_stats->verdicts[i], pkts_recv);
        }
        if ( pc.internal_blacklist > 0 )
            LogStat("Int Blkflw", pc.internal_blacklist, pkts_recv);

        if ( pc.internal_whitelist > 0 )
            LogStat("Int Alwflw", pc.internal_whitelist, pkts_recv);
    }
#ifdef TARGET_BASED
    if (ScIdsMode() && IsAdaptiveConfigured())
    {
        LogMessage("%s\n", STATS_SEPARATOR);
        LogMessage("Attribute Table Stats:\n");

        LogCount("Number Entries", (uint64_t)SFAT_NumberOfHosts());
        LogCount("Table Reloaded", pc.attribute_table_reloads);
    }
#endif  /* TARGET_BASED */

    //mpse_print_qinfo();

#ifndef NO_NON_ETHER_DECODER
#ifdef DLT_IEEE802_11
    if(DAQ_GetBaseProtocol() == DLT_IEEE802_11)
    {
        LogMessage("%s\n", STATS_SEPARATOR);
        LogMessage("Wireless Stats:\n");
        LogMessage("Breakdown by type:\n");

        LogStat("Management Packets", pc.wifi_mgmt, total);
        LogStat("Control Packets", pc.wifi_control, total);
        LogStat("Data Packets", pc.wifi_data, total);
    }
#endif  /* DLT_IEEE802_11 */
#endif  // NO_NON_ETHER_DECODER

    for (idx = preproc_stats_funcs; idx != NULL; idx = idx->next)
    {
        LogMessage("%s\n", STATS_SEPARATOR);
        idx->func(exiting ? 1 : 0);
    }

#ifdef SIDE_CHANNEL
    SideChannelStats(exiting, STATS_SEPARATOR);
#endif /* SIDE_CHANNEL */

#ifndef REG_TEST
    memory_stats_periodic_handler(NULL, 0);
#endif /*REG_TEST*/

    LogMessage("%s\n", STATS_SEPARATOR);
}

/****************************************************************************
 *
 * Function: CleanupProtoNames()
 *
 * Purpose: Frees the protocol names
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
void CleanupProtoNames(void)
{
    int i;

    for(i = 0; i < 256; i++)
    {
        if( protocol_names[i] != NULL )
        {
            free( protocol_names[i] );
            protocol_names[i] = NULL;
        }
    }
}

/****************************************************************************
 *
 * Function: read_infile(char *)
 *
 * Purpose: Reads the BPF filters in from a file.  Ripped from tcpdump.
 *
 * Arguments: fname => the name of the file containing the BPF filters
 *
 * Returns: the processed BPF string
 *
 ****************************************************************************/
char *read_infile(char *fname)
{
    register int fd, cc;
    register char *cp, *cmt;
    struct stat buf;

    fd = open(fname, O_RDONLY);

    if(fd < 0)
        FatalError("can't open %s: %s\n", fname, strerror(errno));

    if(fstat(fd, &buf) < 0)
        FatalError("can't stat %s: %s\n", fname, strerror(errno));

    cp = (char *)SnortAlloc(((u_int)buf.st_size + 1) * sizeof(char));

    cc = read(fd, cp, (int) buf.st_size);

    if(cc < 0)
        FatalError("read %s: %s\n", fname, strerror(errno));

    if(cc != buf.st_size)
        FatalError("short read %s (%d != %d)\n", fname, cc, (int) buf.st_size);

    cp[(int) buf.st_size] = '\0';

    close(fd);

    /* Treat everything upto the end of the line as a space
     *  so that we can put comments in our BPF filters
     */

    while((cmt = strchr(cp, '#')) != NULL)
    {
        while (*cmt != '\r' && *cmt != '\n' && *cmt != '\0')
        {
            *cmt++ = ' ';
        }
    }

    /** LogMessage("BPF filter file: %s\n", fname); **/

    return(cp);
}


 /****************************************************************************
  *
  * Function: CheckLogDir()
  *
  * Purpose: CyberPsychotic sez: basically we only check if logdir exist and
  *          writable, since it might screw the whole thing in the middle. Any
  *          other checks could be performed here as well.
  *
  * Arguments: None.
  *
  * Returns: void function
  *
  ****************************************************************************/
void CheckLogDir(void)
{
    struct stat st;

    if (snort_conf->log_dir == NULL)
        return;

    if (stat(snort_conf->log_dir, &st) == -1)
        FatalError("Stat check on log dir failed: %s.\n", strerror(errno));

    if (!S_ISDIR(st.st_mode) || (access(snort_conf->log_dir, W_OK) == -1))
    {
        FatalError("Can not get write access to logging directory \"%s\". "
                   "(directory doesn't exist or permissions are set incorrectly "
                   "or it is not a directory at all)\n",
                   snort_conf->log_dir);
    }
}

/* Signal handler for child process signaling the parent
 * that is is ready */
static volatile int parent_wait = 1;
static void SigChildReadyHandler(int signal)
{
    parent_wait = 0;
}

/****************************************************************************
 *
 * Function: GoDaemon()
 *
 * Purpose: Puts the program into daemon mode, nice and quiet like....
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
void GoDaemon(void)
{
#ifndef WIN32

    int exit_val = 0;
    pid_t cpid;

    if (ScDaemonRestart())
        return;

    LogMessage("Initializing daemon mode\n");

    /* Don't daemonize if we've already daemonized and
     * received a SIGNAL_SNORT_RELOAD. */
    if(getppid() != 1)
    {
        /* Register signal handler that parent can trap signal */
        SnortAddSignal(SIGNAL_SNORT_CHILD_READY, SigChildReadyHandler, 1);

        if (errno != 0)
            errno = 0;

        /* now fork the child */
        printf("Spawning daemon child...\n");
        cpid = fork();

        if(cpid > 0)
        {
            /* Continue waiting until receiving signal from child */
            int status;
            /* Parent */
            printf("My daemon child %d lives...\n", cpid);

            /* Don't exit quite yet.  Wait for the child
             * to signal that is there and created the PID
             * file.
             */
            do
            {
#ifdef DEBUG
                printf("Parent waiting for child...\n");
#endif
                sleep(1);

            } while (parent_wait);

            if (waitpid(cpid, &status, WNOHANG) == cpid)
            {
                if (WIFEXITED(status))
                {
                    LogMessage("Child exited unexpectedly\n");
                    exit_val = -1;
                }

                else if (WIFSIGNALED(status))
                {
                    LogMessage("Child terminated unexpectedly\n");
                    exit_val = -2;
                }
            }
#ifdef DEBUG
            printf("Child terminated unexpectedly (%d)\n", status);
#endif
            printf("Daemon parent exiting (%d)\n", exit_val);

            exit(exit_val);                /* parent */
        }

        if(cpid < 0)
        {
            /* Daemonizing failed... */
            perror("fork");
            exit(1);
        }
    }
    /* Child */
    setsid();

    close(0);
    close(1);
    close(2);

#ifdef DEBUG
    /* redirect stdin/stdout/stderr to a file */
    open("/tmp/snort.debug", O_CREAT | O_RDWR);  /* stdin, fd 0 */

    /* Change ownership to that which we will drop privileges to */
    if ((snort_conf->user_id != -1) || (snort_conf->group_id != -1))
    {
        uid_t user_id = getuid();
        gid_t group_id = getgid();

        if (snort_conf->user_id != -1)
            user_id = snort_conf->user_id;
        if (snort_conf->group_id != -1)
            group_id = snort_conf->group_id;

        chown("/tmp/snort.debug", user_id, group_id);
    }
#else
    /* redirect stdin/stdout/stderr to /dev/null */
    (void)open("/dev/null", O_RDWR);  /* stdin, fd 0 */
#endif

    if (dup(0)) {}  /* stdout, fd 0 => fd 1 */
    if (dup(0)) {}  /* stderr, fd 0 => fd 2 */

    SignalWaitingParent();
#endif /* ! WIN32 */
}

/* Signal the parent that child is ready */
void SignalWaitingParent(void)
{
#ifndef WIN32
    pid_t ppid = getppid();
#ifdef DEBUG
    printf("Signaling parent %d from child %d\n", ppid, getpid());
#endif

    if (kill(ppid, SIGNAL_SNORT_CHILD_READY))
    {
        LogMessage("Daemon initialized, failed to signal parent pid: "
            "%d, failure: %d, %s\n", ppid, errno, strerror(errno));
    }
    else
    {
        LogMessage("Daemon initialized, signaled parent pid: %d\n", ppid);
    }
#endif
}

/* Guaranteed to be '\0' terminated even if truncation occurs.
 *
 * returns  SNORT_SNPRINTF_SUCCESS if successful
 * returns  SNORT_SNPRINTF_TRUNCATION on truncation
 * returns  SNORT_SNPRINTF_ERROR on error
 */
int SnortSnprintf(char *buf, size_t buf_size, const char *format, ...)
{
    va_list ap;
    int ret;

    if (buf == NULL || buf_size <= 0 || format == NULL)
        return SNORT_SNPRINTF_ERROR;

    /* zero first byte in case an error occurs with
     * vsnprintf, so buffer is null terminated with
     * zero length */
    buf[0] = '\0';
    buf[buf_size - 1] = '\0';

    va_start(ap, format);

    ret = vsnprintf(buf, buf_size, format, ap);

    va_end(ap);

    if (ret < 0)
        return SNORT_SNPRINTF_ERROR;

    if (buf[buf_size - 1] != '\0' || (size_t)ret >= buf_size)
    {
        /* result was truncated */
        buf[buf_size - 1] = '\0';
        return SNORT_SNPRINTF_TRUNCATION;
    }

    return SNORT_SNPRINTF_SUCCESS;
}

/* Appends to a given string
 * Guaranteed to be '\0' terminated even if truncation occurs.
 *
 * returns SNORT_SNPRINTF_SUCCESS if successful
 * returns SNORT_SNPRINTF_TRUNCATION on truncation
 * returns SNORT_SNPRINTF_ERROR on error
 */
int SnortSnprintfAppend(char *buf, size_t buf_size, const char *format, ...)
{
    int str_len;
    int ret;
    va_list ap;

    if (buf == NULL || buf_size <= 0 || format == NULL)
        return SNORT_SNPRINTF_ERROR;

    str_len = SnortStrnlen(buf, buf_size);

    /* since we've already checked buf and buf_size an error
     * indicates no null termination, so just start at
     * beginning of buffer */
    if (str_len == SNORT_STRNLEN_ERROR)
    {
        buf[0] = '\0';
        str_len = 0;
    }

    buf[buf_size - 1] = '\0';

    va_start(ap, format);

    ret = vsnprintf(buf + str_len, buf_size - (size_t)str_len, format, ap);

    va_end(ap);

    if (ret < 0)
        return SNORT_SNPRINTF_ERROR;

    if (buf[buf_size - 1] != '\0' || (size_t)ret >= buf_size)
    {
        /* truncation occured */
        buf[buf_size - 1] = '\0';
        return SNORT_SNPRINTF_TRUNCATION;
    }

    return SNORT_SNPRINTF_SUCCESS;
}

/* Guaranteed to be '\0' terminated even if truncation occurs.
 *
 * Arguments:  dst - the string to contain the copy
 *             src - the string to copy from
 *             dst_size - the size of the destination buffer
 *                        including the null byte.
 *
 * returns SNORT_STRNCPY_SUCCESS if successful
 * returns SNORT_STRNCPY_TRUNCATION on truncation
 * returns SNORT_STRNCPY_ERROR on error
 *
 * Note: Do not set dst[0] = '\0' on error since it's possible that
 * dst and src are the same pointer - it will at least be null
 * terminated in any case
 */
int SnortStrncpy(char *dst, const char *src, size_t dst_size)
{
    char *ret = NULL;

    if (dst == NULL || src == NULL || dst_size <= 0)
        return SNORT_STRNCPY_ERROR;

    dst[dst_size - 1] = '\0';

    ret = strncpy(dst, src, dst_size);

    /* Not sure if this ever happens but might as
     * well be on the safe side */
    if (ret == NULL)
        return SNORT_STRNCPY_ERROR;

    if (dst[dst_size - 1] != '\0')
    {
        /* result was truncated */
        dst[dst_size - 1] = '\0';
        return SNORT_STRNCPY_TRUNCATION;
    }

    return SNORT_STRNCPY_SUCCESS;
}

char *SnortStrndup(const char *src, size_t dst_size)
{
	char *ret = SnortAlloc(dst_size + 1);
    int ret_val;

	ret_val = SnortStrncpy(ret, src, dst_size + 1);

    if(ret_val == SNORT_STRNCPY_ERROR)
	{
		free(ret);
		return NULL;
	}

	return ret;
}

/* Determines whether a buffer is '\0' terminated and returns the
 * string length if so
 *
 * returns the string length if '\0' terminated
 * returns SNORT_STRNLEN_ERROR if not '\0' terminated
 */
int SnortStrnlen(const char *buf, int buf_size)
{
    int i = 0;

    if (buf == NULL || buf_size <= 0)
        return SNORT_STRNLEN_ERROR;

    for (i = 0; i < buf_size; i++)
    {
        if (buf[i] == '\0')
            break;
    }

    if (i == buf_size)
        return SNORT_STRNLEN_ERROR;

    return i;
}

char * SnortStrdup(const char *str)
{
    char *copy = NULL;

    if (!str)
    {
        FatalError("Unable to duplicate string: NULL!\n");
    }

    copy = strdup(str);

    if (copy == NULL)
    {
        FatalError("Unable to duplicate string: %s!\n", str);
    }

    return copy;
}

/*
 * Find first occurrence of char of accept in s, limited by slen.
 * A 'safe' version of strpbrk that won't read past end of buffer s
 * in cases that s is not NULL terminated.
 *
 * This code assumes 'accept' is a static string.
 */
const char *SnortStrnPbrk(const char *s, int slen, const char *accept)
{
    char ch;
    const char *s_end;
    if (!s || (slen == 0) || !*s || !accept)
        return NULL;

    s_end = s + slen;
    while (s < s_end)
    {
        ch = *s;
        if (strchr(accept, ch))
            return s;
        s++;
    }
    return NULL;
}

/*
 * Find first occurrence of searchstr in s, limited by slen.
 * A 'safe' version of strstr that won't read past end of buffer s
 * in cases that s is not NULL terminated.
 */
const char *SnortStrnStr(const char *s, int slen, const char *searchstr)
{
    char ch, nc;
    int len;
    if (!s || (slen == 0) || !*s || !searchstr)
        return NULL;

    if ((ch = *searchstr++) != 0)
    {
        len = strlen(searchstr);
        do
        {
            do
            {
                if ((nc = *s++) == 0)
                {
                    return NULL;
                }
                slen--;
                if (slen == 0)
                    return NULL;
            } while (nc != ch);
            if (slen - len < 0)
                return NULL;
        } while (memcmp(s, searchstr, len) != 0);
        s--;
    }
    return s;
}

/*
 * Find first occurrence of substring in s, ignore case.
*/
const char *SnortStrcasestr(const char *s, int slen, const char *substr)
{
    char ch, nc;
    int len;

    if (!s || (slen == 0) || !*s || !substr)
        return NULL;

    if ((ch = *substr++) != 0)
    {
        ch = tolower((char)ch);
        len = strlen(substr);
        do
        {
            do
            {
                if ((nc = *s++) == 0)
                {
                    return NULL;
                }
                slen--;
                if(slen == 0)
                    return NULL;
            } while ((char)tolower((uint8_t)nc) != ch);
            if(slen - len < 0)
                return NULL;
        } while (strncasecmp(s, substr, len) != 0);
        s--;
    }
    return s;
}

void * SnortAlloc2(size_t size, const char *format, ...)
{
    void *tmp;

    tmp = (void *)calloc(size, sizeof(char));

    if(tmp == NULL)
    {
        va_list ap;
        char buf[STD_BUF];

        buf[STD_BUF - 1] = '\0';

        va_start(ap, format);

        vsnprintf(buf, STD_BUF - 1, format, ap);

        va_end(ap);

        FatalError("%s", buf);
    }

    return tmp;
}

/**
 * Chroot and adjust the snort_conf->log_dir reference
 *
 * @param directory directory to chroot to
 * @param logstore ptr to snort_conf->log_dir which must be dynamically allocated
 */
void SetChroot(char *directory, char **logstore)
{
#ifdef WIN32
    FatalError("SetChroot() should not be called under Win32!\n");
#else
    char *absdir;
    size_t abslen;
    char *logdir;

    if(!directory || !logstore)
    {
        FatalError("Null parameter passed\n");
    }

    logdir = *logstore;

    if(logdir == NULL || *logdir == '\0')
    {
        FatalError("Null log directory\n");
    }

    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"SetChroot: %s\n",
                                       CurrentWorkingDir()););

    logdir = GetAbsolutePath(logdir);

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "SetChroot: %s\n",
                                       CurrentWorkingDir()));

    logdir = SnortStrdup(logdir);

    /* We're going to reset logstore, so free it now */
    free(*logstore);
    *logstore = NULL;

    /* change to the directory */
    if(chdir(directory) != 0)
    {
        FatalError("SetChroot: Can not chdir to \"%s\": %s\n", directory,
                   strerror(errno));
    }

    /* always returns an absolute pathname */
    absdir = CurrentWorkingDir();

    if(absdir == NULL)
    {
        FatalError("NULL Chroot found\n");
    }

    abslen = strlen(absdir);

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "ABS: %s %d\n", absdir, abslen););

    /* make the chroot call */
    if(chroot(absdir) < 0)
    {
        FatalError("Can not chroot to \"%s\": absolute: %s: %s\n",
                   directory, absdir, strerror(errno));
    }

    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"chroot success (%s ->", absdir););
    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"%s)\n ", CurrentWorkingDir()););

    /* change to "/" in the new directory */
    if(chdir("/") < 0)
    {
        FatalError("Can not chdir to \"/\" after chroot: %s\n",
                   strerror(errno));
    }

    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"chdir success (%s)\n",
                            CurrentWorkingDir()););


    if(strncmp(absdir, logdir, strlen(absdir)))
    {
        FatalError("Absdir is not a subset of the logdir");
    }

    if(abslen >= strlen(logdir))
    {
        *logstore = SnortStrdup("/");
    }
    else
    {
        *logstore = SnortStrdup(logdir + abslen);
    }

    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"new logdir from %s to %s\n",
                            logdir, *logstore));

    LogMessage("Chroot directory = %s\n", directory);

    if(logdir != NULL)
        free(logdir);

#if 0
    /* XXX XXX */
    /* install the I can't do this signal handler */
    signal(SIGNAL_SNORT_RELOAD, SigCantHupHandler);
#endif
#endif /* !WIN32 */
}


/**
 * Return a ptr to the absolute pathname of snort.  This memory must
 * be copied to another region if you wish to save it for later use.
 */
char *CurrentWorkingDir(void)
{
    static char buf[PATH_MAX_UTIL + 1];

    if(getcwd((char *) buf, PATH_MAX_UTIL) == NULL)
    {
        return NULL;
    }

    buf[PATH_MAX_UTIL] = '\0';

    return (char *) buf;
}

/**
 * Given a directory name, return a ptr to a static
 */
char *GetAbsolutePath(char *dir)
{
    char *savedir, *dirp;
    static char buf[PATH_MAX_UTIL + 1];

    if(dir == NULL)
    {
        return NULL;
    }

    savedir = strdup(CurrentWorkingDir());

    if(savedir == NULL)
    {
        return NULL;
    }

    if(chdir(dir) < 0)
    {
        LogMessage("Can't change to directory: %s\n", dir);
        free(savedir);
        return NULL;
    }

    dirp = CurrentWorkingDir();

    if(dirp == NULL)
    {
        LogMessage("Unable to access current directory\n");
        free(savedir);
        return NULL;
    }
    else
    {
        strncpy(buf, dirp, PATH_MAX_UTIL);
        buf[PATH_MAX_UTIL] = '\0';
    }

    if(chdir(savedir) < 0)
    {
        LogMessage("Can't change back to directory: %s\n", dir);
        free(savedir);
        return NULL;
    }

    free(savedir);
    return (char *) buf;
}


#ifndef WIN32
/* very slow sort - do not use at runtime! */
SF_LIST * SortDirectory(const char *path)
{
    SF_LIST *dir_entries;
    DIR *dir;
    struct dirent *direntry;
    int ret = 0;

    if (path == NULL)
        return NULL;

    dir_entries = sflist_new();
    if (dir_entries == NULL)
    {
        ErrorMessage("Could not allocate new list for directory entries\n");
        return NULL;
    }

    dir = opendir(path);
    if (dir == NULL)
    {
        ErrorMessage("Error opening directory: %s: %s\n",
                     path, strerror(errno));
        sflist_free_all(dir_entries, free);
        return NULL;
    }

    /* Reset errno since we'll be checking it unconditionally */
    errno = 0;

    while ((direntry = readdir(dir)) != NULL)
    {
        char *node_entry_name, *dir_entry_name;
        SF_LNODE *node;

        dir_entry_name = SnortStrdup(direntry->d_name);

        for (node = sflist_first_node(dir_entries);
             node != NULL;
             node = sflist_next_node(dir_entries))
        {
            node_entry_name = (char *)node->ndata;
            if (strcmp(dir_entry_name, node_entry_name) < 0)
                break;
        }

        if (node == NULL)
            ret = sflist_add_tail(dir_entries, (NODE_DATA)dir_entry_name);
        else
            ret = sflist_add_before(dir_entries, node, (NODE_DATA)dir_entry_name);

        if (ret == -1)
        {
            ErrorMessage("Error adding directory entry to list\n");
            sflist_free_all(dir_entries, free);
            closedir(dir);
            return NULL;
        }
    }

    if (errno != 0)
    {
        ErrorMessage("Error reading directory: %s: %s\n",
                     path, strerror(errno));
        errno = 0;
        sflist_free_all(dir_entries, free);
        closedir(dir);
        return NULL;
    }

    closedir(dir);

    return dir_entries;
}

int GetFilesUnderDir(const char *path, SF_QUEUE *dir_queue, const char *filter)
{
    SF_LIST *dir_entries;
    char *direntry;
    int ret = 0;
    int num_files = 0;

    if ((path == NULL) || (dir_queue == NULL))
        return -1;

    dir_entries = SortDirectory(path);
    if (dir_entries == NULL)
    {
        ErrorMessage("Error sorting entries in directory: %s\n", path);
        return -1;
    }

    for (direntry = (char *)sflist_first(dir_entries);
         direntry != NULL;
         direntry = (char *)sflist_next(dir_entries))
    {
        char path_buf[PATH_MAX];
        struct stat file_stat;

        /* Don't look at dot files */
        if (strncmp(".", direntry, 1) == 0)
            continue;

        ret = SnortSnprintf(path_buf, PATH_MAX, "%s%s%s",
                            path, path[strlen(path) - 1] == '/' ? "" : "/", direntry);
        if (ret == SNORT_SNPRINTF_TRUNCATION)
        {
            ErrorMessage("Error copying file to buffer: Path too long\n");
            sflist_free_all(dir_entries, free);
            return -1;
        }
        else if (ret != SNORT_SNPRINTF_SUCCESS)
        {
            ErrorMessage("Error copying file to buffer\n");
            sflist_free_all(dir_entries, free);
            return -1;
        }

        ret = stat(path_buf, &file_stat);
        if (ret == -1)
        {
            ErrorMessage("Could not stat file: %s: %s\n",
                         path_buf, strerror(errno));
            continue;
        }

        if (file_stat.st_mode & S_IFDIR)
        {
            ret = GetFilesUnderDir(path_buf, dir_queue, filter);
            if (ret == -1)
            {
                sflist_free_all(dir_entries, free);
                return -1;
            }

            num_files += ret;
        }
        else if (file_stat.st_mode & S_IFREG)
        {
            if ((filter == NULL) || (fnmatch(filter, direntry, 0) == 0))
            {
                char *file = SnortStrdup(path_buf);

                ret = sfqueue_add(dir_queue, (NODE_DATA)file);
                if (ret == -1)
                {
                    ErrorMessage("Could not append item to list: %s\n", file);
                    free(file);
                    sflist_free_all(dir_entries, free);
                    return -1;
                }

                num_files++;
            }
        }
    }

    sflist_free_all(dir_entries, free);

    return num_files;
}
#endif

/****************************************************************************
 *
 * Function: hex(u_char *xdata, int length)
 *
 * Purpose: This function takes takes a buffer "xdata" and its length then
 *          returns a string of hex with no spaces
 *
 * Arguments: xdata is the buffer, length is the length of the buffer in
 *            bytes
 *
 * Returns: char * -- You must free this char * when you are done with it.
 *
 ***************************************************************************/
char *hex(const u_char *xdata, int length)
{
    int x;
    char *rval = NULL;
    char *buf = NULL;

    if (xdata == NULL)
        return NULL;

    buf = (char *)calloc((length * 2) + 1, sizeof(char));

    if (buf != NULL)
    {
        rval = buf;

        for (x = 0; x < length; x++)
        {
            SnortSnprintf(buf, 3, "%02X", xdata[x]);
            buf += 2;
        }

        rval[length * 2] = '\0';
    }

    return rval;
}

/****************************************************************************
 *
 * Function: fasthex(u_char *xdata, int length)
 *
 * Purpose: Outputs a purdy fugly hex output, only used by mstring with
 * DEBUG_MSGS enabled.
 *
 * Arguments: xdata is the buffer, length is the length of the buffer in
 *            bytes
 *
 * Returns: char * -- You must free this char * when you are done with it.
 *
 ***************************************************************************/
char *fasthex(const u_char *xdata, int length)
{
    char conv[] = "0123456789ABCDEF";
    char *retbuf = NULL;
    const u_char *index;
    const u_char *end;
    char *ridx;

    index = xdata;
    end = xdata + length;
    retbuf = (char *)SnortAlloc(((length * 2) + 1) * sizeof(char));
    ridx = retbuf;

    while(index < end)
    {
        *ridx++ = conv[((*index & 0xFF)>>4)];
        *ridx++ = conv[((*index & 0xFF)&0x0F)];
        index++;
    }

    return retbuf;
}

/*
 *   Fatal Integer Parser
 *   Ascii to Integer conversion with fatal error support
 */
int xatol(const char *s , const char *etext)
{
    long int val;
    char *endptr;
    char *default_error = "xatol() error\n";

    if (etext == NULL)
        etext = default_error;

    if (s == NULL)
        FatalError("%s: String is NULL\n", etext);

    while (isspace((int)*s))
        s++;

    if (strlen(s) == 0)
        FatalError("%s: String is empty\n", etext);


    /*
     *  strtoul - errors on win32 : ERANGE (VS 6.0)
     *            errors on linux : ERANGE, EINVAL
     *               (for EINVAL, unsupported base which won't happen here)
     */
    val = SnortStrtol(s, &endptr, 0);

    if ((errno == ERANGE) || (val > INT_MAX) || (val < INT_MIN) || (*endptr != '\0'))
        FatalError("%s: Invalid integer input: %s\n", etext, s);

    return (int)val;
}

/*
 *   Fatal Integer Parser
 *   Ascii to Integer conversion with fatal error support
 */
unsigned int xatou(const char *s , const char *etext)
{
    unsigned long int val;
    char *endptr;
    char *default_error = "xatou() error\n";

    if (etext == NULL)
        etext = default_error;

    if (s == NULL)
        FatalError("%s: String is NULL\n", etext);

    while (isspace((int)*s))
        s++;

    if (strlen(s) == 0)
        FatalError("%s: String is empty\n", etext);

    if (*s == '-')
    {
        FatalError("%s: Invalid unsigned integer - negative sign found, "
                   "input: %s\n", etext, s);
    }


    /*
     *  strtoul - errors on win32 : ERANGE (VS 6.0)
     *            errors on linux : ERANGE, EINVAL
     */
    val = SnortStrtoul(s, &endptr, 0);

    if ((errno == ERANGE) || (val > UINT_MAX) || (*endptr != '\0'))
        FatalError("%s: Invalid integer input: %s\n", etext, s);

    return (unsigned int)val;
}

unsigned int xatoup(const char *s , const char *etext)
{
    unsigned long int val = xatou(s, etext);
    if ( !val )
        FatalError("%s: must be > 0\n", etext);
    return (unsigned int)val;
}

char * ObfuscateIpToText(sfaddr_t *ip)
{
    static char ip_buf1[INET6_ADDRSTRLEN];
    static char ip_buf2[INET6_ADDRSTRLEN];
    static int buf_num = 0;
    int buf_size = INET6_ADDRSTRLEN;
    char *ip_buf;

    if (buf_num)
        ip_buf = ip_buf2;
    else
        ip_buf = ip_buf1;

    buf_num ^= 1;
    ip_buf[0] = 0;

    if (ip == NULL)
        return ip_buf;

    if (!sfip_is_set(&snort_conf->obfuscation_net))
    {
        if (sfaddr_family(ip) == AF_INET6)
            SnortSnprintf(ip_buf, buf_size, "x:x:x:x::x:x:x:x");
        else
            SnortSnprintf(ip_buf, buf_size, "xxx.xxx.xxx.xxx");
    }
    else
    {
        sfaddr_t tmp;
        char *tmp_buf;

        IP_COPY_VALUE(tmp, ip);

        if (sfip_is_set(&snort_conf->homenet))
        {
            if (sfip_contains(&snort_conf->homenet, &tmp) == SFIP_CONTAINS)
                sfip_obfuscate(&snort_conf->obfuscation_net, &tmp);
        }
        else
        {
            sfip_obfuscate(&snort_conf->obfuscation_net, &tmp);
        }

        tmp_buf = sfip_to_str(&tmp);
        SnortSnprintf(ip_buf, buf_size, "%s", tmp_buf);
    }

    return ip_buf;
}

void PrintPacketData(const uint8_t *data, const uint32_t len)
{
    uint32_t i, j;
    uint32_t total_len = 0;
    uint8_t hex_buf[16];
    uint8_t char_buf[16];
    char *length_chars = "       0  1  2  3  4  5  6  7   8  9 10 11 12 13 14 15\n"
                         "------------------------------------------------------\n";

    LogMessage("%s", length_chars);

    for (i = 0; i <= len; i++)
    {
        if ((i%16 == 0) && (i != 0))
        {
            LogMessage("%04x  ", total_len);
            total_len += 16;

            for (j = 0; j < 16; j++)
            {
                LogMessage("%02x ", hex_buf[j]);
                if (j == 7)
                    LogMessage(" ");
            }

            LogMessage(" ");

            for (j = 0; j < 16; j++)
            {
                LogMessage("%c", char_buf[j]);
                if (j == 7)
                    LogMessage(" ");
            }

            LogMessage("\n");
        }

        if (i == len)
            break;

        hex_buf[i%16] = data[i];

        if (isprint((int)data[i]))
            char_buf[i%16] = data[i];
        else
            char_buf[i%16] = '.';
    }

    if ((i-total_len) > 0)
    {
        LogMessage("%04x  ", total_len);

        for (j = 0; j < i-total_len; j++)
        {
            LogMessage("%02x ", hex_buf[j]);
            if (j == 7)
                LogMessage(" ");
        }

        if (j < 8)
            LogMessage(" ");
        LogMessage("%*s", (16-j)*3, "");
        LogMessage(" ");

        for (j = 0; j < i-total_len; j++)
        {
            LogMessage("%c", char_buf[j]);
            if (j == 7)
                LogMessage(" ");
        }
    }

    LogMessage("\n");
}

int CheckValueInRange(const char *value_str, char *option,
        unsigned long lo, unsigned long hi, unsigned long *value)
{
    char *endptr;
    uint32_t val;

    if ( value_str == NULL )
    {
        ParseError("Invalid format for %s.", option);
        return -1;
    }

    if (SnortStrToU32(value_str, &endptr, &val, 10))
    {
        ParseError("Invalid format for %s.", option);
        return -1;
    }

    if(*endptr)
    {
        ParseError("Invalid format for %s.", option);
        return -1;
    }

    *value = val;

    if ( (errno == ERANGE) || (*value) < lo || (*value) > hi)
    {
        ParseError("Invalid value for %s. "
                "It should range between %u and %u.", option,
                lo, hi);
        return -1;
    }

    return 0;
}

