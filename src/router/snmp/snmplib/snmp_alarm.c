/*
 * snmp_alarm.c:
 */
/* Portions of this file are subject to the following copyright(s).  See
 * the Net-SNMP's COPYING file for more details and other copyrights
 * that may apply:
 */
/*
 * Portions of this file are copyrighted by:
 * Copyright © 2003 Sun Microsystems, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */
/** @defgroup snmp_alarm  generic library based alarm timers for various parts of an application 
 *  @ingroup library
 * 
 *  @{
 */
#include <net-snmp/net-snmp-config.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <signal.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <sys/types.h>
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_STRING_H
#include <string.h>
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
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include <net-snmp/types.h>
#include <net-snmp/output_api.h>
#include <net-snmp/config_api.h>
#include <net-snmp/utilities.h>

#include <net-snmp/library/snmp_api.h>
#include <net-snmp/library/callback.h>
#include <net-snmp/library/snmp_alarm.h>

static struct snmp_alarm *thealarms = NULL;
static int      start_alarms = 0;
static unsigned int regnum = 1;

int
init_alarm_post_config(int majorid, int minorid, void *serverarg,
                       void *clientarg)
{
    start_alarms = 1;
    set_an_alarm();
    return SNMPERR_SUCCESS;
}

void
init_snmp_alarm(void)
{
    start_alarms = 0;
    snmp_register_callback(SNMP_CALLBACK_LIBRARY,
                           SNMP_CALLBACK_POST_READ_CONFIG,
                           init_alarm_post_config, NULL);
}

void
sa_update_entry(struct snmp_alarm *a)
{
    if (a->t_last.tv_sec == 0 && a->t_last.tv_usec == 0) {
        struct timeval  t_now;
        /*
         * Never been called yet, call time `t' from now.  
         */
        gettimeofday(&t_now, NULL);

        a->t_last.tv_sec = t_now.tv_sec;
        a->t_last.tv_usec = t_now.tv_usec;

        a->t_next.tv_sec = t_now.tv_sec + a->t.tv_sec;
        a->t_next.tv_usec = t_now.tv_usec + a->t.tv_usec;

        while (a->t_next.tv_usec >= 1000000) {
            a->t_next.tv_usec -= 1000000;
            a->t_next.tv_sec += 1;
        }
    } else if (a->t_next.tv_sec == 0 && a->t_next.tv_usec == 0) {
        /*
         * We've been called but not reset for the next call.  
         */
        if (a->flags & SA_REPEAT) {
            if (a->t.tv_sec == 0 && a->t.tv_usec == 0) {
                DEBUGMSGTL(("snmp_alarm",
                            "update_entry: illegal interval specified\n"));
                snmp_alarm_unregister(a->clientreg);
                return;
            }

            a->t_next.tv_sec = a->t_last.tv_sec + a->t.tv_sec;
            a->t_next.tv_usec = a->t_last.tv_usec + a->t.tv_usec;

            while (a->t_next.tv_usec >= 1000000) {
                a->t_next.tv_usec -= 1000000;
                a->t_next.tv_sec += 1;
            }
        } else {
            /*
             * Single time call, remove it.  
             */
            snmp_alarm_unregister(a->clientreg);
        }
    }
}

/**
 * This function removes the callback function from a list of registered
 * alarms, unregistering the alarm.
 *
 * @param clientreg is a unique unsigned integer representing a registered
 *	alarm which the client wants to unregister.
 *
 * @return void
 *
 * @see snmp_alarm_register
 * @see snmp_alarm_register_hr
 * @see snmp_alarm_unregister_all
 */
void
snmp_alarm_unregister(unsigned int clientreg)
{
    struct snmp_alarm *sa_ptr, **prevNext = &thealarms;

    for (sa_ptr = thealarms;
         sa_ptr != NULL && sa_ptr->clientreg != clientreg;
         sa_ptr = sa_ptr->next) {
        prevNext = &(sa_ptr->next);
    }

    if (sa_ptr != NULL) {
        *prevNext = sa_ptr->next;
        DEBUGMSGTL(("snmp_alarm", "unregistered alarm %d\n", 
		    sa_ptr->clientreg));
        /*
         * Note:  do not free the clientarg, its the clients responsibility 
         */
        free(sa_ptr);
    } else {
        DEBUGMSGTL(("snmp_alarm", "no alarm %d to unregister\n", clientreg));
    }
}

/**
 * This function unregisters all alarms currently stored.
 *
 * @return void
 *
 * @see snmp_alarm_register
 * @see snmp_alarm_register_hr
 * @see snmp_alarm_unregister
 */
void
snmp_alarm_unregister_all(void)
{
  struct snmp_alarm *sa_ptr, *sa_tmp;

  for (sa_ptr = thealarms; sa_ptr != NULL; sa_ptr = sa_tmp) {
    sa_tmp = sa_ptr->next;
    free(sa_ptr);
  }
  DEBUGMSGTL(("snmp_alarm", "ALL alarms unregistered\n"));
  thealarms = NULL;
}  

struct snmp_alarm *
sa_find_next(void)
{
    struct snmp_alarm *a, *lowest = NULL;

    for (a = thealarms; a != NULL; a = a->next) {
        if (lowest == NULL) {
            lowest = a;
        } else if (a->t_next.tv_sec == lowest->t_next.tv_sec) {
            if (a->t_next.tv_usec < lowest->t_next.tv_usec) {
                lowest = a;
            }
        } else if (a->t_next.tv_sec < lowest->t_next.tv_sec) {
            lowest = a;
        }
    }
    return lowest;
}

struct snmp_alarm *
sa_find_specific(unsigned int clientreg)
{
    struct snmp_alarm *sa_ptr;
    for (sa_ptr = thealarms; sa_ptr != NULL; sa_ptr = sa_ptr->next) {
        if (sa_ptr->clientreg == clientreg) {
            return sa_ptr;
        }
    }
    return NULL;
}

void
run_alarms(void)
{
    int             done = 0;
    struct snmp_alarm *a = NULL;
    unsigned int    clientreg;
    struct timeval  t_now;

    /*
     * Loop through everything we have repeatedly looking for the next thing to
     * call until all events are finally in the future again.  
     */

    while (!done) {
        if ((a = sa_find_next()) == NULL) {
            return;
        }

        gettimeofday(&t_now, NULL);

        if ((a->t_next.tv_sec < t_now.tv_sec) ||
            ((a->t_next.tv_sec == t_now.tv_sec) &&
             (a->t_next.tv_usec < t_now.tv_usec))) {
            clientreg = a->clientreg;
            DEBUGMSGTL(("snmp_alarm", "run alarm %d\n", clientreg));
            (*(a->thecallback)) (clientreg, a->clientarg);
            DEBUGMSGTL(("snmp_alarm", "alarm %d completed\n", clientreg));

            if ((a = sa_find_specific(clientreg)) != NULL) {
                a->t_last.tv_sec = t_now.tv_sec;
                a->t_last.tv_usec = t_now.tv_usec;
                a->t_next.tv_sec = 0;
                a->t_next.tv_usec = 0;
                sa_update_entry(a);
            } else {
                DEBUGMSGTL(("snmp_alarm", "alarm %d deleted itself\n",
                            clientreg));
            }
        } else {
            done = 1;
        }
    }
}



RETSIGTYPE
alarm_handler(int a)
{
    run_alarms();
    set_an_alarm();
}



int
get_next_alarm_delay_time(struct timeval *delta)
{
    struct snmp_alarm *sa_ptr;
    struct timeval  t_diff, t_now;

    sa_ptr = sa_find_next();

    if (sa_ptr) {
        gettimeofday(&t_now, 0);

        if ((t_now.tv_sec > sa_ptr->t_next.tv_sec) ||
            ((t_now.tv_sec == sa_ptr->t_next.tv_sec) &&
             (t_now.tv_usec > sa_ptr->t_next.tv_usec))) {
            /*
             * Time has already passed.  Return the smallest possible amount of
             * time.  
             */
            delta->tv_sec = 0;
            delta->tv_usec = 1;
            return sa_ptr->clientreg;
        } else {
            /*
             * Time is still in the future.  
             */
            t_diff.tv_sec = sa_ptr->t_next.tv_sec - t_now.tv_sec;
            t_diff.tv_usec = sa_ptr->t_next.tv_usec - t_now.tv_usec;

            while (t_diff.tv_usec < 0) {
                t_diff.tv_sec -= 1;
                t_diff.tv_usec += 1000000;
            }

            delta->tv_sec = t_diff.tv_sec;
            delta->tv_usec = t_diff.tv_usec;
            return sa_ptr->clientreg;
        }
    }

    /*
     * Nothing Left.  
     */
    return 0;
}


void
set_an_alarm(void)
{
    struct timeval  delta;
    int             nextalarm = get_next_alarm_delay_time(&delta);

    /*
     * We don't use signals if they asked us nicely not to.  It's expected
     * they'll check the next alarm time and do their own calling of
     * run_alarms().  
     */

    if (nextalarm && !netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
					NETSNMP_DS_LIB_ALARM_DONT_USE_SIG)) {
#ifndef WIN32
# ifdef HAVE_SETITIMER
        struct itimerval it;

        it.it_value.tv_sec = delta.tv_sec;
        it.it_value.tv_usec = delta.tv_usec;
        it.it_interval.tv_sec = 0;
        it.it_interval.tv_usec = 0;

        signal(SIGALRM, alarm_handler);
        setitimer(ITIMER_REAL, &it, NULL);
        DEBUGMSGTL(("snmp_alarm", "schedule alarm %d in %d.%03d seconds\n",
                    nextalarm, delta.tv_sec, (delta.tv_usec / 1000)));
# else  /* HAVE_SETITIMER */
#  ifdef SIGALRM
        signal(SIGALRM, alarm_handler);
        alarm(delta.tv_sec);
        DEBUGMSGTL(("snmp_alarm",
                    "schedule alarm %d in roughly %d seconds\n", nextalarm,
                    delta.tv_sec));
#  endif  /* SIGALRM */
# endif  /* HAVE_SETITIMER */
#endif  /* WIN32 */

    } else {
        DEBUGMSGTL(("snmp_alarm", "no alarms found to schedule\n"));
    }
}


/**
 * This function registers function callbacks to occur at a speciifc time
 * in the future.
 *
 * @param when is an unsigned integer specifying when the callback function
 *             will be called in seconds.
 *
 * @param flags is an unsigned integer that specifies how frequent the callback
 *	function is called in seconds.  Should be SA_REPEAT or 0.  If  
 *	flags  is  set with SA_REPEAT, then the registered callback function
 *	will be called every SA_REPEAT seconds.  If flags is 0 then the 
 *	function will only be called once and then removed from the 
 *	registered alarm list.
 *
 * @param thecallback is a pointer SNMPAlarmCallback which is the callback 
 *	function being stored and registered.
 *
 * @param clientarg is a void pointer used by the callback function.  This 
 *	pointer is assigned to snmp_alarm->clientarg and passed into the
 *	callback function for the client's specifc needs.
 *
 * @return Returns a unique unsigned integer(which is also passed as the first 
 *	argument of each callback), which can then be used to remove the
 *	callback from the list at a later point in the future using the
 *	snmp_alarm_unregister() function.  If memory could not be allocated
 *	for the snmp_alarm struct 0 is returned.
 *
 * @see snmp_alarm_unregister
 * @see snmp_alarm_register_hr
 * @see snmp_alarm_unregister_all
 */
unsigned int
snmp_alarm_register(unsigned int when, unsigned int flags,
                    SNMPAlarmCallback * thecallback, void *clientarg)
{
    struct snmp_alarm **sa_pptr;
    if (thealarms != NULL) {
        for (sa_pptr = &thealarms; (*sa_pptr) != NULL;
             sa_pptr = &((*sa_pptr)->next));
    } else {
        sa_pptr = &thealarms;
    }

    *sa_pptr = SNMP_MALLOC_STRUCT(snmp_alarm);
    if (*sa_pptr == NULL)
        return 0;

    if (0 == when) {
        (*sa_pptr)->t.tv_sec = 0;
        (*sa_pptr)->t.tv_usec = 1;
    } else {
        (*sa_pptr)->t.tv_sec = when;
        (*sa_pptr)->t.tv_usec = 0;
    }
    (*sa_pptr)->flags = flags;
    (*sa_pptr)->clientarg = clientarg;
    (*sa_pptr)->thecallback = thecallback;
    (*sa_pptr)->clientreg = regnum++;
    (*sa_pptr)->next = NULL;
    sa_update_entry(*sa_pptr);

    DEBUGMSGTL(("snmp_alarm",
		"registered alarm %d, t = %d.%03d, flags=0x%02x\n",
                (*sa_pptr)->clientreg, (*sa_pptr)->t.tv_sec,
                ((*sa_pptr)->t.tv_usec / 1000), (*sa_pptr)->flags));

    if (start_alarms)
        set_an_alarm();
    return (*sa_pptr)->clientreg;
}


/**
 * This function offers finer granularity as to when the callback 
 * function is called by making use of t->tv_usec value forming the 
 * "when" aspect of snmp_alarm_register().
 *
 * @param t is a timeval structure used to specify when the callback 
 *	function(alarm) will be called.  Adds the ability to specify
 *	microseconds.  t.tv_sec and t.tv_usec are assigned
 *	to snmp_alarm->tv_sec and snmp_alarm->tv_usec respectively internally.
 *	The snmp_alarm_register function only assigns seconds(it's when 
 *	argument).
 *
 * @param flags is an unsigned integer that specifies how frequent the callback
 *	function is called in seconds.  Should be SA_REPEAT or NULL.  If  
 *	flags  is  set with SA_REPEAT, then the registered callback function
 *	will be called every SA_REPEAT seconds.  If flags is NULL then the 
 *	function will only be called once and then removed from the 
 *	registered alarm list.
 *
 * @param cb is a pointer SNMPAlarmCallback which is the callback 
 *	function being stored and registered.
 *
 * @param cd is a void pointer used by the callback function.  This 
 *	pointer is assigned to snmp_alarm->clientarg and passed into the
 *	callback function for the client's specifc needs.
 *
 * @return Returns a unique unsigned integer(which is also passed as the first 
 *	argument of each callback), which can then be used to remove the
 *	callback from the list at a later point in the future using the
 *	snmp_alarm_unregister() function.  If memory could not be allocated
 *	for the snmp_alarm struct 0 is returned.
 *
 * @see snmp_alarm_register
 * @see snmp_alarm_unregister
 * @see snmp_alarm_unregister_all
 */
unsigned int
snmp_alarm_register_hr(struct timeval t, unsigned int flags,
                       SNMPAlarmCallback * cb, void *cd)
{
    struct snmp_alarm **s = NULL;

    for (s = &(thealarms); *s != NULL; s = &((*s)->next));

    *s = SNMP_MALLOC_STRUCT(snmp_alarm);
    if (*s == NULL) {
        return 0;
    }

    (*s)->t.tv_sec = t.tv_sec;
    (*s)->t.tv_usec = t.tv_usec;
    (*s)->flags = flags;
    (*s)->clientarg = cd;
    (*s)->thecallback = cb;
    (*s)->clientreg = regnum++;
    (*s)->next = NULL;

    sa_update_entry(*s);

    DEBUGMSGTL(("snmp_alarm",
                "registered alarm %d, t = %d.%03d, flags=0x%02x\n",
                (*s)->clientreg, (*s)->t.tv_sec, ((*s)->t.tv_usec / 1000),
                (*s)->flags));

    if (start_alarms) {
        set_an_alarm();
    }

    return (*s)->clientreg;
}
/**  @} */
