/*
 * ipc_s.h for HP-UX 10.30 and above
 *
 * This header file defines the ipc_s structure for lsof.  The ipc_s structure
 * is the streams equivalent of a Berkeley-style inpcb (Internet Protocol
 * Control Block).  The ipc_s holds the TCP/IP address for a stream.
 *
 * The original HP-UX 11 distribution has a flat ipc_s structure, with hash
 * links to other ipc_s structures, and direct links to the the read and write
 * sections of the stream.
 *
 * After patch bundle B11.00.43 the ipc_s structure definition requires
 * two other Q4-derived structures, mirg_s and ipis_s.  The ipis_s structure
 * contains the hash and stream links formerly contained in ipc_s.
 *
 * V. Abell <abe@purdue.edu>
 * February, 1998
 *
 * Patch bundle update supplied by: Kevin Vajk <kvajk@cup.hp.com>
 *				    February, 1999
 */

#if	!defined(LSOF_IPC_S_H)
#define	LSOF_IPC_S_H

#include "kernbits.h"
#include <sys/types.h>

typedef struct mirg_s {
	uint mirg_gen;
} mirg_t;

# if	defined(HAS_IPC_S_PATCH)
typedef struct ipis_s {
	union {
	    KA_T u_ipc_hash_next;
	    KA_T u_ill_hash_next;
	    KA_T u_ipis_hash_next;
	} ipis_hash_next_u;
	union {
	    KA_T u_ipc_ptphn;
	    KA_T u_ill_ptphn;
	    KA_T u_ipis_ptphn;
	} ipis_ptphn_u;
	KA_T ipis_readers_next;
	KA_T ipis_readers_ptpn;
	KA_T ipis_ptr_hash_next;
	KA_T ipis_ptr_ptphn;
	KA_T ipis_rq;
	KA_T ipis_wq;
	mirg_t ipis_mirg;
#  if	HAS_IPC_S_PATCH==2
	uint ipis_msgsqueued;
#  endif	/* HAS_IPC_S_PATCH==2 */
} ipis_t;
# endif	/* defined(HAS_IPC_S_PATCH) */

typedef struct ipc_s {

# if	defined(HAS_IPC_S_PATCH)
	ipis_t ipc_ipis;
# else	/* !defined(HAS_IPC_S_PATCH) */
	KA_T ipc_hash_next;			/* hash link -- ipc_s
						 * structures are hashed in
						 * ipc_tcp_conn[] and
						 * ipc_udp_conn[] */
	mirg_t ipc_mirg;
	KA_T ipc_readers_next;
	KA_T ipc_readers_ptpn;
	KA_T ipc_ptphn;
	KA_T ipc_rq;				/* stream's read queue */
	KA_T ipc_wq;				/* stream's write queue */
# endif	/* defined(HAS_IPC_S_PATCH) */

	int ipc_ioctl_pended;
	union {
	    struct {
		uint32_t ipcu_lcl_addr;		/* local IP address */
		uint32_t ipcu_rem_addr;		/* remote IP address */
		uint16_t ipcu_rem_port;		/* remote port */
		uint16_t ipcu_lcl_port;		/* local port */
	    } ipcu_addrs;
	    uint16_t ipcu_tcp_addr[6];
	} ipc_ipcu;
/*
 * The rest of the q4 elements are ignored.
 */

} ipc_s_t;

#define	ipc_udp_port	ipc_ipcu.ipcu_addrs.ipcu_lcl_port
#define	ipc_udp_addr	ipc_ipcu.ipcu_addrs.ipcu_lcl_addr
#define	ipc_tcp_lport	ipc_ipcu.ipcu_addrs.ipcu_lcl_port
#define	ipc_tcp_laddr	ipc_ipcu.ipcu_addrs.ipcu_lcl_addr
#define	ipc_tcp_fport	ipc_ipcu.ipcu_addrs.ipcu_rem_port
#define	ipc_tcp_faddr	ipc_ipcu.ipcu_addrs.ipcu_rem_addr

#endif	/* !defined(LSOF_IPC_S_H) */
