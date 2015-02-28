/*
 *	BIRD -- Bidirectional Forwarding Detection (BFD)
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_NBFD_H_
#define _BIRD_NBFD_H_

#include "lib/lists.h"
#include "lib/resource.h"

struct bfd_session;

struct bfd_request {
  resource r;
  node n;

  ip_addr addr;
  ip_addr local;
  struct iface *iface;

  void (*hook)(struct bfd_request *);
  void *data;

  struct bfd_session *session;

  u8 state;
  u8 diag;
  u8 old_state;
  u8 down;
};


#ifdef CONFIG_BFD

struct bfd_request * bfd_request_session(pool *p, ip_addr addr, ip_addr local, struct iface *iface, void (*hook)(struct bfd_request *), void *data);

static inline void cf_check_bfd(int use) { }

#else

static inline struct bfd_request * bfd_request_session(pool *p, ip_addr addr, ip_addr local, struct iface *iface, void (*hook)(struct bfd_request *), void *data) { return NULL; }

static inline void cf_check_bfd(int use) { if (use) cf_error("BFD not available"); }

#endif /* CONFIG_BFD */



#endif /* _BIRD_NBFD_H_ */
