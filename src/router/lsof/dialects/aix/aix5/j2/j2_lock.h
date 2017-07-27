/*
 * j2_lock.h -- lsof private copy
 *
 * Needed for:
 *
 * 	AIX 5L, because it's missing there;
 *	AIX 5.2, because it includes <proc/proc_public.h> and that header
 *	    file is missing from AIX 5.2.
 *
 * V. Abell <abe@purdue.edu>
 * Purdue University
 */

#if	!defined(LSOF_J2_LOCK_H)
#define	LSOF_J2_LOCK_H
typedef	long		event_t;
#define	MUTEXLOCK_T	Simple_lock
#define	RDWRLOCK_T	Complex_lock
#endif	/* !defined(LSOF_J2_LOCK_H) */
