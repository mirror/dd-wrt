/*
 * uptime - uptime related functions - part of libproc2
 *
 * Copyright © 2015-2023 Craig Small <csmall@dropbear.xyz>
 * Copyright © 2015-2023 Jim Warner <james.warner@comcast.net>
 * Copyright © 1998-2003 Albert Cahalan
 * Copyright © 1992-1998 Michael K. Johnson <johnsonm@redhat.com>
 * Copyright © 1993      J. Cowley
 * Copyright © ????      Larry Greenfield <greenfie@gauss.rutgers.edu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <utmp.h>
#ifdef WITH_SYSTEMD
#include <systemd/sd-daemon.h>
#include <systemd/sd-login.h>
#endif
#ifdef WITH_ELOGIND
#include <elogind/sd-daemon.h>
#include <elogind/sd-login.h>
#endif

#include "misc.h"
#include "procps-private.h"
#include "pids.h"

#define UPTIME_FILE "/proc/uptime"

#define UPTIME_BUFLEN 256
static __thread char upbuf[UPTIME_BUFLEN];
static __thread char shortbuf[UPTIME_BUFLEN];

/*
 * users:
 *
 * Count the number of users on the system
 * Strictly speaking not a proc FS function but used in many
 * places.
 *
 * Returns: user count on success and <0 on failure
 * On some failures with utmp, 0 may be returned too.
 */
PROCPS_EXPORT int procps_users(void)
{
    int numuser = 0;
#ifdef HAVE_UTMP_X
    struct utmpx *ut;
#else
    struct utmp *ut;
#endif

#if defined(WITH_SYSTEMD) || defined(WITH_ELOGIND)
    if (sd_booted() > 0) {
        char **sessions_list;
        int sessions;

        numuser = 0;

        sessions = sd_get_sessions(&sessions_list);

        if (sessions > 0) {
            int i;

            for (i = 0; i < sessions; i++) {
                char *class;

                if (sd_session_get_class(sessions_list[i], &class) < 0)
                    continue;

                if (strncmp(class, "user", 4) == 0) // user, user-early, user-incomplete
                    numuser++;
                free(class);
            }
	    for (i = 0; i < sessions; i++)
	      free(sessions_list[i]);
	    free(sessions_list);
            return numuser;
        }
    }
#endif

#ifdef HAVE_UTMP_X
    setutxent();
    while ((ut = getutxent())) {
#else
    setutent();
    while ((ut = getutent())) {
#endif
        if ((ut->ut_type == USER_PROCESS) && (ut->ut_name[0] != '\0'))
            numuser++;
    }
    endutent();

    return numuser;
}

/*
 * uptime:
 *
 * Find the uptime and idle time of the system.
 * These numbers are found in /proc/uptime
 * Unlike other procps functions this closes the file each time
 * Either uptime_secs or idle_secs can be null
 *
 * Returns: 0 on success and <0 on failure
 */
PROCPS_EXPORT int procps_uptime(
        double *restrict uptime_secs,
        double *restrict idle_secs)
{
    double up=0, idle=0;
    locale_t tmplocale;
    FILE *fp;
    int rc;

    if ((fp = fopen(UPTIME_FILE, "r")) == NULL)
        return -errno;

    tmplocale = newlocale(LC_NUMERIC_MASK, "C", (locale_t)0);
    uselocale(tmplocale);
    rc = fscanf(fp, "%lf %lf", &up, &idle);
    fclose(fp);
    uselocale(LC_GLOBAL_LOCALE);
    freelocale(tmplocale);

    if (uptime_secs)
        *uptime_secs = up;
    if (idle_secs)
        *idle_secs = idle;

    if (rc < 2)
        return -ERANGE;
    return 0;
}

/*
 * procps_container_uptime:
 *
 * Find the uptime of a container.
 * This is derived from the start time of process 1
 * If hidepid is in action, this will return -1 with errno=ENOENT
 * uptime_secs can be null
 *
 * Returns: 0 on success and <0 on failure
 */
PROCPS_EXPORT int procps_container_uptime(
        double *restrict uptime_secs)
{
    int rv;
    double boot_time, start_time;
    struct pids_fetch *pids_fetch = NULL;
    struct pids_info *info = NULL;
    unsigned tgid = 1;
    struct timespec tp;

    enum pids_item items[] = {
        PIDS_TIME_START};

    if (!uptime_secs)
        return 0; //valid, but odd call

    if ( (rv = clock_gettime(CLOCK_BOOTTIME, &tp) < 0))
        return rv;

    if ( (rv = procps_pids_new(&info, items, 1) < 0))
        return rv;

    if ( (pids_fetch = procps_pids_select(info, &tgid, 1, PIDS_SELECT_PID)) == NULL)
        return -1;

    // Did we get anything? If not error out (probably hidepid>0)
    if (pids_fetch->stacks[0] == NULL)
       return -1;

    boot_time = (tp.tv_sec + tp.tv_nsec * 1.0e-9);
    start_time = PIDS_VAL(0, real, (pids_fetch->stacks[0]));

    if (boot_time > start_time)
        *uptime_secs = boot_time - start_time;
    else
        *uptime_secs = 0;

    procps_pids_unref(&info);
    return 0;
}

#define SECS_IN_DECADE  (60*60*24*365*10)
#define SECS_IN_YEAR    (60*60*24*365)
#define SECS_IN_WEEK    (60*60*24*7)
#define SECS_IN_DAY     (60*60*24)


/*
 * Print the actual uptime component only into str
 *
 * Normal: X days, hh:mm OR X days, mm min
 * Pretty: X decades, X years, X weeks, X days, X hours, X minutes
 *
 * Note the output may be truncated if it cannot fit into size.
 *
 * Returns:
 *  length of string or < 0 on error
 */

static int snprint_uptime_only(
        char *restrict str,
        size_t size,
        double uptime_secs,
        const int pretty)
{
#define print_this(VAL, UNITS) \
    if ( (l = snprintf(str + pos, size-pos, "%s%d %s", comma > 0 ? ", " : "", (VAL), (UNITS))) >= size) \
        return size; \
    else pos +=l
    int pos=0, l;
    int updecades = 0, upyears = 0, upweeks = 0, updays = 0, uphours = 0, upminutes = 0;
    int comma = 0;


    if (pretty) {
        if (uptime_secs > SECS_IN_DECADE) {
            updecades = (int) uptime_secs / SECS_IN_DECADE;
            uptime_secs -= updecades * SECS_IN_DECADE;
        }
        if (uptime_secs > SECS_IN_YEAR) {
            upyears = (int) uptime_secs / SECS_IN_YEAR;
            uptime_secs -= upyears * SECS_IN_YEAR;
        }
        if (uptime_secs > SECS_IN_WEEK) {
            upweeks = (int) uptime_secs / SECS_IN_WEEK;
            uptime_secs -= upweeks * SECS_IN_WEEK;
        }
    }
    /* all formats get the following */
    if (uptime_secs > SECS_IN_DAY) {
        updays = (int) uptime_secs / SECS_IN_DAY;
        uptime_secs -= updays * SECS_IN_DAY;
    }
    if (uptime_secs>60*60) {
        uphours = (int) uptime_secs / (60*60);
        uptime_secs -= uphours*60*60;
    }
    if (uptime_secs>60) {
        upminutes = (int) uptime_secs / 60;
        uptime_secs -= upminutes*60;
    }

    if (pretty) {
        if (updecades) {
            print_this(updecades, updecades > 1 ? "decades" : "decade");
            comma += 1;
        }

        if (upyears) {
            print_this(upyears, upyears > 1 ? "years" : "year");
            comma += 1;
        }

        if (upweeks) {
            print_this(upweeks, upweeks > 1 ? "weeks" : "week");
            comma += 1;
        }
    }
    /* Both formats get days */
    if (updays) {
        print_this(updays, updays != 1 ? "days" : "day");
        comma += 1;
    }

    if (pretty) {
        if (uphours) {
            print_this(uphours, uphours > 1 ? "hours" : "hour");
            comma += 1;
        }

        if (upminutes || (!upminutes && uptime_secs <= 60)) {
            print_this(upminutes, upminutes > 1 ? "minutes" : "minute");
            comma += 1;
        }
    } else {
        if (uphours) {
            if ( (l = snprintf(str + pos, size - pos, "%s%2d:%02d", comma > 0 ? ", " : "", uphours, upminutes)) >= size)
                return size;
            else pos +=l;
        } else {
            print_this(upminutes, "min");
        }
    }
    return pos;
}
/*
 * procps_uptime_snprint():
 *
 * print the standard formats of uptime into the buffer str up to
 * the given length
 *
 * @str: destination buffer
 * @size: length of destination buffer
 * @uptime_secs: Uptime of system/container, from procps_uptime() usually
 * @pretty: 0/1 sets pretty expanded format fo uptime component
 *
 * Returns:
 *  length of string
 *  < 0 on error
 */
PROCPS_EXPORT int procps_uptime_snprint(
        char *restrict str,
        size_t size,
        double uptime_secs,
        const int pretty)
{
    int l,pos=0;
    time_t realseconds;
    struct tm realtime;
    int users;
    double av1, av5, av15;

    if (str == NULL)
        return -EINVAL;
    str[0] = '\0';

    if ( time(&realseconds) < 0)
        return -errno;
    localtime_r(&realseconds, &realtime);

    if (pretty) {
        if ( (l = snprintf(str + pos, size-pos, "%s", "up ")) >= size-pos)
            return size;
        pos +=l;
    } else {
        if ( (l = snprintf(str + pos, size-pos, " %02d:%02d:%02d up ",
                        realtime.tm_hour, realtime.tm_min, realtime.tm_sec)) >= size-pos)
            return size;
        pos +=l;
    }
    l = snprint_uptime_only(str+pos, size-pos, uptime_secs, pretty);
    if (l >= size - pos)
        return size;
    if (l > 0)
        pos += l;

    if (pretty) // That's all it prints
        return pos;

    // print number of users, format ends with comma and two spaces
    users = procps_users();
    if (users < 0) {
        if ( (l = snprintf(str+pos, size-pos, ", ? users,  ")) >= size-pos)
            return size;
        pos += l;
    } else {
        if ( (l = snprintf(str+pos, size-pos, ", %2d %s,  ",
                        users,
                        users != 1 ? "users" : "user")) >= size-pos)
            return size;
        pos += l;
    }

    procps_loadavg(&av1, &av5, &av15);
    if ( (l = snprintf(str+pos, size-pos, "load average: %.2f, %.2f, %.2f",
                    av1, av5, av15)) >= size-pos)
        return size;
    pos += l;

    return pos;
}

/*
 * procps_uptime_sprint:
 *
 * Print current system uptime in nice format
 *
 * Returns a statically allocated upbuf
 */
PROCPS_EXPORT char *procps_uptime_sprint(void)
{
    double uptime_secs;

    upbuf[0] = '\0';
    if (procps_uptime(&uptime_secs, NULL) < 0)
        return shortbuf;

    procps_uptime_snprint( upbuf, UPTIME_BUFLEN, uptime_secs, 0);
    return upbuf;
}

/*
 * procps_uptime_sprint_short:
 *
 * Print current system uptime in nice format
 *
 * Returns a statically allocated buffer
 */
PROCPS_EXPORT char *procps_uptime_sprint_short(void)
{
    double uptime_secs;

    shortbuf[0] = '\0';
    if (procps_uptime(&uptime_secs, NULL) < 0)
        return shortbuf;

    procps_uptime_snprint( shortbuf, UPTIME_BUFLEN, uptime_secs, 1);
    return shortbuf;
}
