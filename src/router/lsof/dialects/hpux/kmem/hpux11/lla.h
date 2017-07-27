/*
 * lla.h for HP-UX 10.30 and above
 *
 * This header file defines the lla_cb structure for lsof.  Lsof uses it to
 * to read the Link Level Access (LLA) control block.  Link level access means
 * access to the network link layer access protocol -- e.g., Ethernet 802.5.
 *
 * V. Abell <abe@purdue.edu>
 * February, 1998
 */

#if	!defined(LSOF_LLA_H)
#define	LSOF_LLA_H

#include "kernbits.h"
#include <sys/types.h>

#define	LLA_IS_ETHER	0x1
#define	LLA_FWRITE	0x100
#define	LLA_FREAD	0x200
#define	LLA_IS_8025	0x800
#define	LLA_IS_SNAP8025	0x1000
#define	LLA_IS_FA8025	0x4000

typedef struct lla_hdr {
	union {
	    struct {
		u_char destaddr[6];
		u_char sourceaddr[6];
		u_short length;
		u_char dsap;
		u_char ssap;
		u_char ctrl;
		u_char pad[3];
		u_short dxsap;
		u_short sxsap;
	    } ieee;
	    struct {
		u_char destaddr[6];
		u_char sourceaddr[6];
		u_short log_type;
		u_short dxsap;
		u_short sxsap;
	    } ether;
	    struct {
		u_char access_ctl;
		u_char frame_ctl;
		u_char destaddr[6];
		u_char sourceaddr[6];
		u_char rif_plus[26];
		u_char dsap;
		u_char ssap;
		u_char ctrl;
		u_char orgid[3];
		u_short etype;
	    } snap8025;
	    struct {
		u_char access_ctl;
		u_char frame_ctl;
		u_char destaddr[6];
		u_char sourceaddr[6];
		u_char rif_plus[26];
		u_char dsap;
		u_char ssap;
		u_char ctrl;
	    } ieee8025;
	} proto;
} lla_hdr_t;

typedef struct lla_cb {
	KA_T so_pcb;
	KA_T pktheader;
	KA_T head_packet;
	KA_T last_packet;
	KA_T lla_ifp;
	u_int lan_signal_mask;
	u_int lan_signal_pid;
	int lan_pkt_size;
	int lla_timeo;
	KA_T lla_rsel;
	struct lla_hdr packet_header;
	short lla_msgsqd;
	short lla_maxmsgs;
	u_short lla_flags;		/* flags, including type  -- i.e.,
					 * the LLA_* symbols defined above */
	short hdr_size;
	int func_addr;
	KA_T lla_lock;
} lla_cb_t;

#endif
