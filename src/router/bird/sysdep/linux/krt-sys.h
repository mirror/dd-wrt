/*
 *	BIRD -- Linux Kernel Netlink Route Syncer
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_KRT_SYS_H_
#define _BIRD_KRT_SYS_H_


/* Kernel interfaces */

struct kif_params {
};

struct kif_state {
};


static inline void kif_sys_init(struct kif_proto *p UNUSED) { }
static inline int kif_sys_reconfigure(struct kif_proto *p UNUSED, struct kif_config *n UNUSED, struct kif_config *o UNUSED) { return 1; }

static inline void kif_sys_preconfig(struct config *c UNUSED) { }
static inline void kif_sys_postconfig(struct kif_config *c UNUSED) { }
static inline void kif_sys_init_config(struct kif_config *c UNUSED) { }
static inline void kif_sys_copy_config(struct kif_config *d UNUSED, struct kif_config *s UNUSED) { }

static inline struct ifa * kif_get_primary_ip(struct iface *i UNUSED) { return NULL; }


/* Kernel routes */

#define KRT_ALLOW_MERGE_PATHS	1

#define EA_KRT_PREFSRC		EA_CODE(EAP_KRT, 0x10)
#define EA_KRT_REALM		EA_CODE(EAP_KRT, 0x11)
#define EA_KRT_SCOPE		EA_CODE(EAP_KRT, 0x12)


#define KRT_METRICS_MAX		0x10	/* RTAX_QUICKACK+1 */
#define KRT_METRICS_OFFSET	0x20	/* Offset of EA_KRT_* vs RTAX_* */

#define KRT_FEATURES_MAX	4

/*
 * Following attributes are parts of RTA_METRICS kernel route attribute, their
 * ids must be consistent with their RTAX_* constants (+ KRT_METRICS_OFFSET)
 */
#define EA_KRT_METRICS		EA_CODE(EAP_KRT, 0x20)	/* Dummy one */
#define EA_KRT_LOCK		EA_CODE(EAP_KRT, 0x21)
#define EA_KRT_MTU		EA_CODE(EAP_KRT, 0x22)
#define EA_KRT_WINDOW		EA_CODE(EAP_KRT, 0x23)
#define EA_KRT_RTT		EA_CODE(EAP_KRT, 0x24)
#define EA_KRT_RTTVAR		EA_CODE(EAP_KRT, 0x25)
#define EA_KRT_SSTRESH		EA_CODE(EAP_KRT, 0x26)
#define EA_KRT_CWND		EA_CODE(EAP_KRT, 0x27)
#define EA_KRT_ADVMSS		EA_CODE(EAP_KRT, 0x28)
#define EA_KRT_REORDERING 	EA_CODE(EAP_KRT, 0x29)
#define EA_KRT_HOPLIMIT		EA_CODE(EAP_KRT, 0x2a)
#define EA_KRT_INITCWND		EA_CODE(EAP_KRT, 0x2b)
#define EA_KRT_FEATURES		EA_CODE(EAP_KRT, 0x2c)
#define EA_KRT_RTO_MIN		EA_CODE(EAP_KRT, 0x2d)
#define EA_KRT_INITRWND		EA_CODE(EAP_KRT, 0x2e)
#define EA_KRT_QUICKACK		EA_CODE(EAP_KRT, 0x2f)

/* Bits of EA_KRT_LOCK, also based on RTAX_* constants */
#define EA_KRT_LOCK_MTU		EA_KRT_LOCK | EA_BIT(0x2)
#define EA_KRT_LOCK_WINDOW	EA_KRT_LOCK | EA_BIT(0x3)
#define EA_KRT_LOCK_RTT		EA_KRT_LOCK | EA_BIT(0x4)
#define EA_KRT_LOCK_RTTVAR	EA_KRT_LOCK | EA_BIT(0x5)
#define EA_KRT_LOCK_SSTHRESH	EA_KRT_LOCK | EA_BIT(0x6)
#define EA_KRT_LOCK_CWND	EA_KRT_LOCK | EA_BIT(0x7)
#define EA_KRT_LOCK_ADVMSS	EA_KRT_LOCK | EA_BIT(0x8)
#define EA_KRT_LOCK_REORDERING 	EA_KRT_LOCK | EA_BIT(0x9)
#define EA_KRT_LOCK_HOPLIMIT	EA_KRT_LOCK | EA_BIT(0xa)
// define EA_KRT_LOCK_INITCWND	EA_KRT_LOCK | EA_BIT(0xb)
// define EA_KRT_LOCK_FEATURES	EA_KRT_LOCK | EA_BIT(0xc)
#define EA_KRT_LOCK_RTO_MIN	EA_KRT_LOCK | EA_BIT(0xd)
// define EA_KRT_LOCK_INITRWND	EA_KRT_LOCK | EA_BIT(0xe)

/* Bits of EA_KRT_FEATURES, based on RTAX_FEATURE_* constants */
#define EA_KRT_FEATURE_ECN	EA_KRT_FEATURES | EA_BIT(0x0)
// define EA_KRT_FEATURE_SACK	EA_KRT_FEATURES | EA_BIT(0x1)
// define EA_KRT_FEATURE_TSTAMP	EA_KRT_FEATURES | EA_BIT(0x2)
#define EA_KRT_FEATURE_ALLFRAG	EA_KRT_FEATURES | EA_BIT(0x3)


struct krt_params {
  u32 table_id;				/* Kernel table ID we sync with */
  u32 metric;				/* Kernel metric used for all routes */
};

struct krt_state {
  struct krt_proto *hash_next;
};


static inline void krt_sys_init(struct krt_proto *p UNUSED) { }
static inline void krt_sys_preconfig(struct config *c UNUSED) { }
static inline void krt_sys_postconfig(struct krt_config *x UNUSED) { }


#endif
