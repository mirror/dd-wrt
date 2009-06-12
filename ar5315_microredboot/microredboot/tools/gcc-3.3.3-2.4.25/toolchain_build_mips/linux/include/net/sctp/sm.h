/* SCTP kernel reference Implementation
 * Copyright (c) 1999-2000 Cisco, Inc.
 * Copyright (c) 1999-2001 Motorola, Inc.
 * Copyright (c) 2001 Intel Corp.
 * Copyright (c) 2001-2002 International Business Machines Corp.
 *
 * This file is part of the SCTP kernel reference Implementation
 *
 * This file is part of the implementation of the add-IP extension,
 * based on <draft-ietf-tsvwg-addip-sctp-02.txt> June 29, 2001,
 * for the SCTP kernel reference Implementation.
 *
 * These are definitions needed by the state machine.
 *
 * The SCTP reference implementation is free software;
 * you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * The SCTP reference implementation is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 *                 ************************
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU CC; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Please send any bug reports or fixes you make to the
 * email addresses:
 *    lksctp developers <lksctp-developers@lists.sourceforge.net>
 *
 * Or submit a bug report through the following website:
 *    http://www.sf.net/projects/lksctp
 *
 * Written or modified by:
 *    La Monte H.P. Yarroll <piggy@acm.org>
 *    Karl Knutson <karl@athena.chicago.il.us>
 *    Xingang Guo <xingang.guo@intel.com>
 *    Jon Grimm <jgrimm@us.ibm.com>
 *    Dajiang Zhang <dajiang.zhang@nokia.com>
 *    Sridhar Samudrala <sri@us.ibm.com>
 *    Daisy Chang <daisyc@us.ibm.com>
 *    Ardelle Fan <ardelle.fan@intel.com>
 *
 * Any bugs reported given to us we will try to fix... any fixes shared will
 * be incorporated into the next SCTP release.
 */


#include <linux/types.h>
#include <linux/compiler.h>
#include <linux/slab.h>
#include <linux/in.h>
#include <net/sctp/command.h>
#include <net/sctp/sctp.h>

#ifndef __sctp_sm_h__
#define __sctp_sm_h__

/*
 * Possible values for the disposition are:
 */
typedef enum {
	SCTP_DISPOSITION_DISCARD,	 /* No further processing.  */
	SCTP_DISPOSITION_CONSUME,	 /* Process return values normally.  */
	SCTP_DISPOSITION_NOMEM,		 /* We ran out of memory--recover.  */
	SCTP_DISPOSITION_DELETE_TCB,	 /* Close the association.  */
	SCTP_DISPOSITION_ABORT,		 /* Close the association NOW.  */
	SCTP_DISPOSITION_VIOLATION,	 /* The peer is misbehaving.  */
	SCTP_DISPOSITION_NOT_IMPL,	 /* This entry is not implemented.  */
	SCTP_DISPOSITION_ERROR,		 /* This is plain old user error.  */
	SCTP_DISPOSITION_BUG,		 /* This is a bug.  */
} sctp_disposition_t;

typedef struct {
	int name;
	int action;
} sctp_sm_command_t;

typedef sctp_disposition_t (sctp_state_fn_t) (const struct sctp_endpoint *,
					      const struct sctp_association *,
					      const sctp_subtype_t type,
					      void *arg,
					      sctp_cmd_seq_t *);
typedef void (sctp_timer_event_t) (unsigned long);
typedef struct {
	sctp_state_fn_t *fn;
	char *name;
} sctp_sm_table_entry_t;

/* A naming convention of "sctp_sf_xxx" applies to all the state functions
 * currently in use.
 */

/* Prototypes for generic state functions. */
sctp_state_fn_t sctp_sf_not_impl;
sctp_state_fn_t sctp_sf_bug;

/* Prototypes for gener timer state functions. */
sctp_state_fn_t sctp_sf_timer_ignore;

/* Prototypes for chunk state functions. */
sctp_state_fn_t sctp_sf_do_9_1_abort;
sctp_state_fn_t sctp_sf_cookie_wait_abort;
sctp_state_fn_t sctp_sf_cookie_echoed_abort;
sctp_state_fn_t sctp_sf_shutdown_pending_abort;
sctp_state_fn_t sctp_sf_shutdown_sent_abort;
sctp_state_fn_t sctp_sf_shutdown_ack_sent_abort;
sctp_state_fn_t sctp_sf_do_5_1B_init;
sctp_state_fn_t sctp_sf_do_5_1C_ack;
sctp_state_fn_t sctp_sf_do_5_1D_ce;
sctp_state_fn_t sctp_sf_do_5_1E_ca;
sctp_state_fn_t sctp_sf_do_4_C;
sctp_state_fn_t sctp_sf_eat_data_6_2;
sctp_state_fn_t sctp_sf_eat_data_fast_4_4;
sctp_state_fn_t sctp_sf_eat_sack_6_2;
sctp_state_fn_t sctp_sf_tabort_8_4_8;
sctp_state_fn_t sctp_sf_operr_notify;
sctp_state_fn_t sctp_sf_t1_timer_expire;
sctp_state_fn_t sctp_sf_t2_timer_expire;
sctp_state_fn_t sctp_sf_t5_timer_expire;
sctp_state_fn_t sctp_sf_sendbeat_8_3;
sctp_state_fn_t sctp_sf_beat_8_3;
sctp_state_fn_t sctp_sf_backbeat_8_3;
sctp_state_fn_t sctp_sf_do_9_2_final;
sctp_state_fn_t sctp_sf_do_9_2_shutdown;
sctp_state_fn_t sctp_sf_do_ecn_cwr;
sctp_state_fn_t sctp_sf_do_ecne;
sctp_state_fn_t sctp_sf_ootb;
sctp_state_fn_t sctp_sf_shut_8_4_5;
sctp_state_fn_t sctp_sf_pdiscard;
sctp_state_fn_t sctp_sf_violation;
sctp_state_fn_t sctp_sf_discard_chunk;
sctp_state_fn_t sctp_sf_do_5_2_1_siminit;
sctp_state_fn_t sctp_sf_do_5_2_2_dupinit;
sctp_state_fn_t sctp_sf_do_5_2_4_dupcook;
sctp_state_fn_t sctp_sf_unk_chunk;
sctp_state_fn_t sctp_sf_do_8_5_1_E_sa;
sctp_state_fn_t sctp_sf_cookie_echoed_err;
sctp_state_fn_t sctp_sf_do_5_2_6_stale;

/* Prototypes for primitive event state functions.  */
sctp_state_fn_t sctp_sf_do_prm_asoc;
sctp_state_fn_t sctp_sf_do_prm_send;
sctp_state_fn_t sctp_sf_do_9_2_prm_shutdown;
sctp_state_fn_t sctp_sf_cookie_wait_prm_shutdown;
sctp_state_fn_t sctp_sf_cookie_echoed_prm_shutdown;
sctp_state_fn_t sctp_sf_do_9_1_prm_abort;
sctp_state_fn_t sctp_sf_cookie_wait_prm_abort;
sctp_state_fn_t sctp_sf_cookie_echoed_prm_abort;
sctp_state_fn_t sctp_sf_shutdown_pending_prm_abort;
sctp_state_fn_t sctp_sf_shutdown_sent_prm_abort;
sctp_state_fn_t sctp_sf_shutdown_ack_sent_prm_abort;
sctp_state_fn_t sctp_sf_error_closed;
sctp_state_fn_t sctp_sf_error_shutdown;
sctp_state_fn_t sctp_sf_ignore_primitive;
sctp_state_fn_t sctp_sf_do_prm_requestheartbeat;

/* Prototypes for other event state functions.  */
sctp_state_fn_t sctp_sf_do_9_2_start_shutdown;
sctp_state_fn_t sctp_sf_do_9_2_shutdown_ack;
sctp_state_fn_t sctp_sf_ignore_other;

/* Prototypes for timeout event state functions.  */
sctp_state_fn_t sctp_sf_do_6_3_3_rtx;
sctp_state_fn_t sctp_sf_do_6_2_sack;
sctp_state_fn_t sctp_sf_autoclose_timer_expire;


/* These are state functions which are either obsolete or not in use yet.
 * If any of these functions needs to be revived, it should be renamed with
 * the "sctp_sf_xxx" prefix, and be moved to the above prototype groups.
 */

/* Prototypes for chunk state functions.  Not in use. */
sctp_state_fn_t sctp_sf_do_9_2_reshutack;
sctp_state_fn_t sctp_sf_do_9_2_reshut;
sctp_state_fn_t sctp_sf_do_9_2_shutack;

sctp_state_fn_t lucky;
sctp_state_fn_t other_stupid;

/* Prototypes for timeout event state functions.  Not in use. */
sctp_state_fn_t sctp_do_4_2_reinit;
sctp_state_fn_t sctp_do_4_3_reecho;
sctp_state_fn_t sctp_do_9_2_reshut;
sctp_state_fn_t sctp_do_9_2_reshutack;
sctp_state_fn_t sctp_do_8_3_hb_err;
sctp_state_fn_t sctp_heartoff;

/* Prototypes for addip related state functions.  Not in use. */
sctp_state_fn_t sctp_addip_do_asconf;
sctp_state_fn_t sctp_addip_do_asconf_ack;

/* Prototypes for utility support functions.  */
__u8 sctp_get_chunk_type(struct sctp_chunk *chunk);
sctp_sm_table_entry_t *sctp_sm_lookup_event(sctp_event_t event_type,
					    sctp_state_t state,
					    sctp_subtype_t event_subtype);
int sctp_chunk_iif(const struct sctp_chunk *);
struct sctp_association *sctp_make_temp_asoc(const struct sctp_endpoint *,
					     struct sctp_chunk *,
					     int gfp);
__u32 sctp_generate_verification_tag(void);
void sctp_populate_tie_tags(__u8 *cookie, __u32 curTag, __u32 hisTag);

/* Prototypes for chunk-building functions.  */
sctp_chunk_t *sctp_make_init(const struct sctp_association *,
			     const sctp_bind_addr_t *,
			     int gfp, int vparam_len);
sctp_chunk_t *sctp_make_init_ack(const struct sctp_association *,
				 const sctp_chunk_t *,
				 const int gfp,
				 const int unkparam_len);
sctp_chunk_t *sctp_make_cookie_echo(const struct sctp_association *,
				    const sctp_chunk_t *);
sctp_chunk_t *sctp_make_cookie_ack(const struct sctp_association *,
				   const sctp_chunk_t *);
sctp_chunk_t *sctp_make_cwr(const struct sctp_association *,
				 const __u32 lowest_tsn,
				 const sctp_chunk_t *);
sctp_chunk_t *sctp_make_datafrag(struct sctp_association *,
				 const struct sctp_sndrcvinfo *sinfo,
				 int len, const __u8 *data,
				 __u8 flags, __u16 ssn);
sctp_chunk_t * sctp_make_datafrag_empty(struct sctp_association *,
					const struct sctp_sndrcvinfo *sinfo,
					int len, const __u8 flags,
					__u16 ssn);
sctp_chunk_t *sctp_make_data(struct sctp_association *,
			     const struct sctp_sndrcvinfo *sinfo,
			     int len, const __u8 *data);
sctp_chunk_t *sctp_make_data_empty(struct sctp_association *,
				   const struct sctp_sndrcvinfo *, int len);
sctp_chunk_t *sctp_make_ecne(const struct sctp_association *,
				  const __u32);
sctp_chunk_t *sctp_make_sack(const struct sctp_association *);
sctp_chunk_t *sctp_make_shutdown(const struct sctp_association *asoc);
sctp_chunk_t *sctp_make_shutdown_ack(const struct sctp_association *asoc,
					  const sctp_chunk_t *);
sctp_chunk_t *sctp_make_shutdown_complete(const struct sctp_association *,
					  const sctp_chunk_t *);
void sctp_init_cause(sctp_chunk_t *, __u16 cause, const void *, size_t);
sctp_chunk_t *sctp_make_abort(const struct sctp_association *,
			      const sctp_chunk_t *,
			      const size_t hint);
sctp_chunk_t *sctp_make_abort_no_data(const struct sctp_association *,
				      const sctp_chunk_t *,
				      __u32 tsn);
sctp_chunk_t *sctp_make_abort_user(const struct sctp_association *,
				   const sctp_chunk_t *,
				   const struct msghdr *);
sctp_chunk_t *sctp_make_heartbeat(const struct sctp_association *,
				  const struct sctp_transport *,
				  const void *payload,
				  const size_t paylen);
sctp_chunk_t *sctp_make_heartbeat_ack(const struct sctp_association *,
				      const sctp_chunk_t *,
				      const void *payload,
				      const size_t paylen);
sctp_chunk_t *sctp_make_op_error(const struct sctp_association *,
				 const sctp_chunk_t *chunk,
				 __u16 cause_code,
				 const void *payload,
				 size_t paylen);
void sctp_chunk_assign_tsn(sctp_chunk_t *);
void sctp_chunk_assign_ssn(sctp_chunk_t *);
int sctp_datachunks_from_user(struct sctp_association *,
			      const struct sctp_sndrcvinfo *,
			      struct msghdr *, int len,
			      struct sk_buff_head *);


/* Prototypes for statetable processing. */

int sctp_do_sm(sctp_event_t event_type, sctp_subtype_t subtype,
	       sctp_state_t state,
               struct sctp_endpoint *,
               struct sctp_association *asoc,
               void *event_arg,
               int gfp);

int sctp_side_effects(sctp_event_t event_type, sctp_subtype_t subtype,
		      sctp_state_t state,
                      struct sctp_endpoint *,
                      struct sctp_association *asoc,
                      void *event_arg,
                      sctp_disposition_t status,
		      sctp_cmd_seq_t *commands,
                      int gfp);

/* 2nd level prototypes */
int
sctp_cmd_interpreter(sctp_event_t event_type, sctp_subtype_t subtype,
		     sctp_state_t state,
		     struct sctp_endpoint *ep,
		     struct sctp_association *asoc,
		     void *event_arg,
		     sctp_disposition_t status,
		     sctp_cmd_seq_t *retval,
		     int gfp);


int sctp_gen_sack(struct sctp_association *, int force, sctp_cmd_seq_t *);
void sctp_do_TSNdup(struct sctp_association *, sctp_chunk_t *, long gap);

void sctp_generate_t3_rtx_event(unsigned long peer);
void sctp_generate_heartbeat_event(unsigned long peer);

sctp_sackhdr_t *sctp_sm_pull_sack(sctp_chunk_t *);
struct sctp_packet *sctp_abort_pkt_new(const struct sctp_endpoint *,
				       const struct sctp_association *,
				       struct sctp_chunk *chunk,
				       const void *payload,
				       size_t paylen);
struct sctp_packet *sctp_ootb_pkt_new(const struct sctp_association *,
				      const struct sctp_chunk *);
void sctp_ootb_pkt_free(struct sctp_packet *);

sctp_cookie_param_t *
sctp_pack_cookie(const struct sctp_endpoint *, const struct sctp_association *,
		 const struct sctp_chunk *, int *cookie_len,
		 const __u8 *, int addrs_len);
struct sctp_association *sctp_unpack_cookie(const struct sctp_endpoint *,
				       const struct sctp_association *,
				       sctp_chunk_t *, int gfp, int *err,
				       sctp_chunk_t **err_chk_p);
int sctp_addip_addr_config(struct sctp_association *, sctp_param_t,
			   struct sockaddr_storage*, int);
void sctp_send_stale_cookie_err(const struct sctp_endpoint *ep,
				const struct sctp_association *asoc,
				const sctp_chunk_t *chunk,
				sctp_cmd_seq_t *commands,
				sctp_chunk_t *err_chunk);

/* 3rd level prototypes */
__u32 sctp_generate_tag(const struct sctp_endpoint *);
__u32 sctp_generate_tsn(const struct sctp_endpoint *);

/* 4th level prototypes */
void sctp_param2sockaddr(union sctp_addr *addr, sctp_addr_param_t *,
			 __u16 port, int iif);
int sctp_addr2sockaddr(const union sctp_params, union sctp_addr *);
int sockaddr2sctp_addr(const union sctp_addr *, sctp_addr_param_t *);

/* Extern declarations for major data structures.  */
sctp_sm_table_entry_t *sctp_chunk_event_lookup(sctp_cid_t, sctp_state_t);
extern sctp_sm_table_entry_t
primitive_event_table[SCTP_NUM_PRIMITIVE_TYPES][SCTP_STATE_NUM_STATES];
extern sctp_sm_table_entry_t
other_event_table[SCTP_NUM_OTHER_TYPES][SCTP_STATE_NUM_STATES];
extern sctp_sm_table_entry_t
timeout_event_table[SCTP_NUM_TIMEOUT_TYPES][SCTP_STATE_NUM_STATES];
extern sctp_timer_event_t *sctp_timer_events[SCTP_NUM_TIMEOUT_TYPES];

/* These are some handy utility macros... */


/* Get the size of a DATA chunk payload. */
static inline __u16 sctp_data_size(sctp_chunk_t *chunk)
{
	__u16 size;

	size = ntohs(chunk->chunk_hdr->length);
	size -= sizeof(sctp_data_chunk_t);

	return size;
}

/* Compare two TSNs */

/* RFC 1982 - Serial Number Arithmetic
 *
 * 2. Comparison
 *  Then, s1 is said to be equal to s2 if and only if i1 is equal to i2,
 *  in all other cases, s1 is not equal to s2.
 *
 * s1 is said to be less than s2 if, and only if, s1 is not equal to s2,
 * and
 *
 *      (i1 < i2 and i2 - i1 < 2^(SERIAL_BITS - 1)) or
 *      (i1 > i2 and i1 - i2 > 2^(SERIAL_BITS - 1))
 *
 * s1 is said to be greater than s2 if, and only if, s1 is not equal to
 * s2, and
 *
 *      (i1 < i2 and i2 - i1 > 2^(SERIAL_BITS - 1)) or
 *      (i1 > i2 and i1 - i2 < 2^(SERIAL_BITS - 1))
 */

/*
 * RFC 2960
 *  1.6 Serial Number Arithmetic
 *
 * Comparisons and arithmetic on TSNs in this document SHOULD use Serial
 * Number Arithmetic as defined in [RFC1982] where SERIAL_BITS = 32.
 */

enum {
	TSN_SIGN_BIT = (1<<31)
};

static inline int TSN_lt(__u32 s, __u32 t)
{
	return (((s) - (t)) & TSN_SIGN_BIT);
}

static inline int TSN_lte(__u32 s, __u32 t)
{
	return (((s) == (t)) || (((s) - (t)) & TSN_SIGN_BIT));
}

/* Compare two SSNs */

/*
 * RFC 2960
 *  1.6 Serial Number Arithmetic
 *
 * Comparisons and arithmetic on Stream Sequence Numbers in this document
 * SHOULD use Serial Number Arithmetic as defined in [RFC1982] where
 * SERIAL_BITS = 16.
 */
enum {
	SSN_SIGN_BIT = (1<<15)
};

static inline int SSN_lt(__u16 s, __u16 t)
{
	return (((s) - (t)) & SSN_SIGN_BIT);
}

static inline int SSN_lte(__u16 s, __u16 t)
{
	return (((s) == (t)) || (((s) - (t)) & SSN_SIGN_BIT));
}

/* Run sctp_add_cmd() generating a BUG() if there is a failure.  */
static inline void sctp_add_cmd_sf(sctp_cmd_seq_t *seq, sctp_verb_t verb, sctp_arg_t obj)
{
	if (unlikely(!sctp_add_cmd(seq, verb, obj)))
		BUG();
}

/* Check VTAG of the packet matches the sender's own tag OR its peer's
 * tag and the T bit is set in the Chunk Flags.
 */
static inline int
sctp_vtag_verify_either(const sctp_chunk_t *chunk,
			const struct sctp_association *asoc)
{
        /* RFC 2960 Section 8.5.1, sctpimpguide-06 Section 2.13.2
	 *
	 * B) The receiver of a ABORT shall accept the packet if the
	 * Verification Tag field of the packet matches its own tag OR it
	 * is set to its peer's tag and the T bit is set in the Chunk
	 * Flags. Otherwise, the receiver MUST silently discard the packet
	 * and take no further action.
	 *
	 * (C) The receiver of a SHUTDOWN COMPLETE shall accept the
	 * packet if the Verification Tag field of the packet
	 * matches its own tag OR it is set to its peer's tag and
	 * the T bit is set in the Chunk Flags.  Otherwise, the
	 * receiver MUST silently discard the packet and take no
	 * further action....
	 *
	 */
        if ((ntohl(chunk->sctp_hdr->vtag) == asoc->c.my_vtag) ||
	    (sctp_test_T_bit(chunk) && (ntohl(chunk->sctp_hdr->vtag)
	    == asoc->c.peer_vtag))) {
                return 1;
	}

	return 0;
}

#endif /* __sctp_sm_h__ */
