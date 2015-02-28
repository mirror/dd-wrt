/*
 *	BIRD -- Bidirectional Forwarding Detection (BFD)
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_BFD_H_
#define _BIRD_BFD_H_

#include <pthread.h>

#include "nest/bird.h"
#include "nest/cli.h"
#include "nest/iface.h"
#include "nest/protocol.h"
#include "nest/route.h"
#include "conf/conf.h"
#include "lib/hash.h"
#include "lib/resource.h"
#include "lib/socket.h"
#include "lib/string.h"

#include "nest/bfd.h"
#include "io.h"


#define BFD_CONTROL_PORT	3784
#define BFD_ECHO_PORT		3785
#define BFD_MULTI_CTL_PORT	4784

#define BFD_DEFAULT_MIN_RX_INT	(10 MS_)
#define BFD_DEFAULT_MIN_TX_INT	(100 MS_)
#define BFD_DEFAULT_IDLE_TX_INT	(1 S_)
#define BFD_DEFAULT_MULTIPLIER	5


struct bfd_iface_config;

struct bfd_config
{
  struct proto_config c;
  list patt_list;		/* List of iface configs (struct bfd_iface_config) */
  list neigh_list;		/* List of configured neighbors (struct bfd_neighbor) */
  struct bfd_iface_config *multihop; /* Multihop pseudoiface config */
};

struct bfd_iface_config
{
  struct iface_patt i;
  u32 min_rx_int;
  u32 min_tx_int;
  u32 idle_tx_int;
  u8 multiplier;
  u8 passive;
};

struct bfd_neighbor
{
  node n;
  ip_addr addr;
  ip_addr local;
  struct iface *iface;

  struct neighbor *neigh;
  struct bfd_request *req;

  u8 multihop;
  u8 active;
};

struct bfd_proto
{
  struct proto p;
  struct birdloop *loop;
  pool *tpool;
  pthread_spinlock_t lock;
  node bfd_node;

  slab *session_slab;
  HASH(struct bfd_session) session_hash_id;
  HASH(struct bfd_session) session_hash_ip;

  sock *notify_rs;
  sock *notify_ws;
  list notify_list;

  sock *rx_1;
  sock *rx_m;
  list iface_list;
};

struct bfd_iface
{
  node n;
  ip_addr local;
  struct iface *iface;
  struct bfd_iface_config *cf;
  struct bfd_proto *bfd;

  sock *sk;
  u32 uc;
  u8 changed;
};

struct bfd_session
{
  node n;
  ip_addr addr;				/* Address of session */
  struct bfd_iface *ifa;		/* Iface associated with session */
  struct bfd_session *next_id;		/* Next in bfd.session_hash_id */
  struct bfd_session *next_ip;		/* Next in bfd.session_hash_ip */

  u8 opened_unused;
  u8 passive;
  u8 poll_active;
  u8 poll_scheduled;
  
  u8 loc_state;
  u8 rem_state;
  u8 loc_diag;
  u8 rem_diag;
  u32 loc_id;				/* Local session ID (local discriminator) */
  u32 rem_id;				/* Remote session ID (remote discriminator) */
  u32 des_min_tx_int;			/* Desired min rx interval, local option */
  u32 des_min_tx_new;			/* Used for des_min_tx_int change */
  u32 req_min_rx_int;			/* Required min tx interval, local option */
  u32 req_min_rx_new;			/* Used for req_min_rx_int change */
  u32 rem_min_tx_int;			/* Last received des_min_tx_int */
  u32 rem_min_rx_int;			/* Last received req_min_rx_int */
  u8 demand_mode;			/* Currently unused */
  u8 rem_demand_mode;
  u8 detect_mult;			/* Announced detect_mult, local option */
  u8 rem_detect_mult;			/* Last received detect_mult */

  btime last_tx;			/* Time of last sent periodic control packet */
  btime last_rx;			/* Time of last received valid control packet */

  timer2 *tx_timer;			/* Periodic control packet timer */
  timer2 *hold_timer;			/* Timer for session down detection time */

  list request_list;			/* List of client requests (struct bfd_request) */
  bird_clock_t last_state_change;	/* Time of last state change */
  u8 notify_running;			/* 1 if notify hooks are running */
};


extern const char *bfd_state_names[];

#define BFD_STATE_ADMIN_DOWN	0
#define BFD_STATE_DOWN		1
#define BFD_STATE_INIT		2
#define BFD_STATE_UP		3

#define BFD_DIAG_NOTHING	0
#define BFD_DIAG_TIMEOUT	1
#define BFD_DIAG_ECHO_FAILED	2
#define BFD_DIAG_NEIGHBOR_DOWN	3
#define BFD_DIAG_FWD_RESET	4
#define BFD_DIAG_PATH_DOWN	5
#define BFD_DIAG_C_PATH_DOWN	6
#define BFD_DIAG_ADMIN_DOWN	7
#define BFD_DIAG_RC_PATH_DOWN	8

#define BFD_POLL_TX		1
#define BFD_POLL_RX		2

#define BFD_FLAGS		0x3f
#define BFD_FLAG_POLL		(1 << 5)
#define BFD_FLAG_FINAL		(1 << 4)
#define BFD_FLAG_CPI		(1 << 3)
#define BFD_FLAG_AP		(1 << 2)
#define BFD_FLAG_DEMAND		(1 << 1)
#define BFD_FLAG_MULTIPOINT	(1 << 0)


static inline void bfd_lock_sessions(struct bfd_proto *p) { pthread_spin_lock(&p->lock); }
static inline void bfd_unlock_sessions(struct bfd_proto *p) { pthread_spin_unlock(&p->lock); }

/* bfd.c */
struct bfd_session * bfd_find_session_by_id(struct bfd_proto *p, u32 id);
struct bfd_session * bfd_find_session_by_addr(struct bfd_proto *p, ip_addr addr);
void bfd_session_process_ctl(struct bfd_session *s, u8 flags, u32 old_tx_int, u32 old_rx_int);
void bfd_show_sessions(struct proto *P);

/* packets.c */
void bfd_send_ctl(struct bfd_proto *p, struct bfd_session *s, int final);
sock * bfd_open_rx_sk(struct bfd_proto *p, int multihop);
sock * bfd_open_tx_sk(struct bfd_proto *p, ip_addr local, struct iface *ifa);


#endif /* _BIRD_BFD_H_ */
