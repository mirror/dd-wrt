/*
 * sth_h for HP-UX 10.30 and above
 *
 * This header file defines the stream head structure, sth_t, for lsof.  Lsof
 * uses the stream head structure to obtain the stream's read and write queue
 * structure pointers.
 *
 * V. Abell
 * February, 1998
 */

#if	!defined(LSOF_STH_H)
#define	LSOF_STH_H

#include "kernbits.h"
#include <sys/types.h>

typedef struct streams_queue {
	KA_T q_qinfo;			/* queue info pointer */
	KA_T q_first;
	KA_T q_last;
	KA_T q_next;
	KA_T q_link;
	KA_T q_ptr;			/* queue private data pointer */
	ulong q_count;
	ulong q_flag;
	int q_minpsz;
	int q_maxpsz;
	ulong q_hiwat;
	ulong q_lowat;
	KA_T q_bandp;
	u_char q_nband;
	u_char q_pad1[3];
	KA_T q_other;
	KA_T queue_sth;
} streams_queue_t;

typedef struct sth_s {
	streams_queue_t *sth_rq;	/* pointer to stream's read queue
					 * structure chain */
	streams_queue_t *sth_wq;	/* pointer to stream's write queue
					 * structure chain */
/*
 * These q4 elements are ignored.

	dev_t sth_dev;
	ulong sth_read_mode;
	ulong sth_write_mode;
	int sth_close_wait_timeout;
	u_char sth_read_error;
	u_char sth_write_error;
	short sth_prim_ack;
	short sth_prim_nak;
	short sth_ext_flags;
	ulong sth_flags;
	int sth_ioc_id;
	KA_T sth_ioc_mp;
	OSRQ sth_ioctl_osrq;
	OSRQ sth_read_osrq;
	OSRQ sth_write_osrq;
	ulong sth_wroff;
	int sth_muxid;
	KA_T sth_mux_link;
	KA_T sth_mux_top;
	gid_t sth_pgid;
	KA_T sth_session;
	KA_T sth_next;
	POLLQ sth_pollq;
	SIGSQ sth_sigsq;
	KA_T sth_ttyp;
	int sth_push_cnt;
	OSR sth_osr;
	KA_T sth_pipestatp;
	KA_T sth_ext_flags_lock;
	uint qlen;
	struct sth_func_reg sth_f_reg;
	spu_t sth_bindspu;

* Those q4 elements were ignored.
*/

} sth_s_t;

#endif	/* !defined(LSOF_STH_H) */
