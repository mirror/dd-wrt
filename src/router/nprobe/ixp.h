/*
 *  Copyright (C) 2005-06 Tom De Canio <tom@commlogicdesign.com>
 *  Copyright (C) 2006    Luca Deri <deri@ntop.org>
 *
 */

/*
 * nProbe netlink api
 */

#ifndef _IXP_H_
#define _IXP_H_

#if defined(linux) || defined(__linux__)

#include <asm/types.h>
#include <linux/netlink.h>
#include <stddef.h>

#define NL_NPROBE		12


typedef struct {
  struct nlmsghdr hdr;
  unsigned char	dat[0];
} evnt_t;


typedef struct {
  struct nlmsghdr hdr;
  struct nlmsgerr err;
} nlerr_t;


typedef union {
  struct nlmsghdr	hdr;
  evnt_t		evnt;
  nlerr_t		err;
} msg_t;


/* These are access macros for the netlink header
 */
#define NPROBE_TYPE(d)             (d)->hdr.nlmsg_type
#define NPROBE_LEN(d)              (d)->hdr.nlmsg_len
#define NPROBE_FLAGS(d)            (d)->hdr.nlmsg_flags
#define NPROBE_SEQ(d)              (d)->hdr.nlmsg_seq
#define NPROBE_PID(d)              (d)->hdr.nlmsg_pid


/* Message types
 */
#define NPROBE_TYPE_BASE	0x200
#define NPROBE_BIND_PID		(NPROBE_TYPE_BASE + 1)
#define NPROBE_EVENT		(NPROBE_TYPE_BASE + 2)
#define NPROBE_DEBUG		(NPROBE_TYPE_BASE + 3)
#define NPROBE_FORCE_INT	(NPROBE_TYPE_BASE + 4)

#define NPROBE_TYPE_MAX		(NPROBE_TYPE_BASE + 5)

typedef struct {
  u_int32_t		dest_mac_hi;	/* upper 32 bits of dest mac */
  u_int16_t		dest_mac_lo;	/* lower 16 bits of dest mac */
  u_int16_t		src_mac_hi;	/* upper 16 bits of src mac */
  u_int32_t		src_mac_lo;	/* lower 32 bits of src mac */
  u_int32_t		vlan_id;	/* vlan id */
  u_int32_t		src_ip;		/* souce ip address */
  u_int32_t		dest_ip;	/* dest ip address */
  union {
    struct {
      u_int8_t	icmp_type;
      u_int8_t	icmp_code;
      u_int16_t	unused;
    };
    struct {
      u_int16_t	tcp_udp_src_port;
      u_int16_t	tcp_udp_dest_port;
    };
    u_int32_t		src_dest_port;
  };
  union {
    struct {
      u_int32_t		zero	 	: 8;
      u_int32_t		protocol	: 8;
      u_int32_t		ingress_port	: 8;
      u_int32_t		tos		: 8;
    };
    u_int32_t			fields;
  };
} ixp_netflow_tuple_t;

typedef struct {
  unsigned 	high;	/* upper 32 bits */
  unsigned	low;	/* lower 32 bits */
} u64_t;

typedef struct {
  union {
    struct {
      ixp_netflow_tuple_t	netflow_tuple;
      u64_t	src2dest_start_timestamp;
      u64_t	src2dest_end_timestamp;
      u64_t	dest2src_start_timestamp;
      u64_t	dest2src_end_timestamp;
      u64_t	src2dest_packet_count;
      u64_t	src2dest_byte_count;
      u64_t	dest2src_packet_count;
      u64_t	dest2src_byte_count;
    };
    u_int32_t		word[1];
  };	
} ixp_netflow_record_t;

/* *********************************** */

#endif /* defined(linux) || defined(__linux__) */

extern int init_ixp();
extern void term_ixp(int ixp_id);
extern void get_ixp_record(int fd);

#endif /* !_IXP_H_ */

