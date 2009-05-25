/*
 *  Copyright (C) 2006    Luca Deri <deri@ntop.org>
 *  Copyright (C) 2005-06 Tom De Canio <tom@commlogicdesign.com>
 *
*/

#if defined(linux) || defined(__linux__)

#include "nprobe.h"

static struct sockaddr_nl peer;
static int pid;

/* ********************************* */

static void nl_header(struct nlmsghdr *nlh, int type,
		      int len, int flags, int seq) {
  nlh->nlmsg_type  = type;
  nlh->nlmsg_len   = len;
  nlh->nlmsg_flags = flags;
  nlh->nlmsg_seq   = seq;
  nlh->nlmsg_pid   = pid;
}

/* ********************************* */

static int simple_msg(int fd, int type) {
  struct nlmsghdr nlh;
  int status;
  static int sequence = 0;

  nl_header(&nlh, type, NLMSG_LENGTH(0), NLM_F_REQUEST, sequence++);
  status = sendto(fd, &nlh, sizeof(nlh), 0, (struct sockaddr *)&peer, 0);

  return(status);
}

/* ********************************* */

static int receive_ixp_msg(int fd, unsigned char *buf, int *len, size_t blen) {
  socklen_t addrlen;
  int rlen;

  addrlen = sizeof(peer);

  rlen = recvfrom(fd, buf, blen, 0,(struct sockaddr *)&peer, &addrlen);
  if(rlen < 0) {
    traceEvent(TRACE_INFO, "receive_msg() failed: %s", strerror(errno));
    return(-1);
  }

  return(rlen);
}

/* ********************************* */

int init_ixp() {
  int fd = socket(PF_NETLINK, SOCK_RAW, NL_NPROBE);

  if(fd < 0) {
    traceEvent(TRACE_INFO, "init_ixp() failed: %s", strerror(errno));
    return(-1);
  }

  memset(&peer, 0, sizeof(struct sockaddr_nl));
  peer.nl_family = AF_NETLINK;

  simple_msg(fd, NPROBE_FORCE_INT);

#ifdef DEBUG
  simple_msg(fd, NPROBE_DEBUG);
#endif

  pid = getpid();

  simple_msg(fd, NPROBE_BIND_PID);

  return(fd);
}

/* ********************************* */

void term_ixp(int ixp_id) {
  if(ixp_id >= 0)
    close(ixp_id);
}

/* ********************************* */

static void process_ixp_flow(register const ixp_netflow_record_t *ixp_netflow_rec,
			     register u_int length) {
  HashBucket dummyBucket;

  memset(&dummyBucket, 0, sizeof(dummyBucket));
  dummyBucket.firstSeenSent.tv_sec = ixp_netflow_rec->src2dest_start_timestamp.low;
  dummyBucket.firstSeenSent.tv_usec = ixp_netflow_rec->src2dest_start_timestamp.high;
  dummyBucket.lastSeenSent.tv_sec = ixp_netflow_rec->src2dest_end_timestamp.low;
  dummyBucket.lastSeenSent.tv_usec = ixp_netflow_rec->src2dest_end_timestamp.high;

  dummyBucket.firstSeenRcvd.tv_sec = ixp_netflow_rec->dest2src_start_timestamp.low;
  dummyBucket.firstSeenRcvd.tv_usec = ixp_netflow_rec->dest2src_start_timestamp.high;
  dummyBucket.lastSeenRcvd.tv_sec = ixp_netflow_rec->dest2src_end_timestamp.low;
  dummyBucket.lastSeenRcvd.tv_usec = ixp_netflow_rec->dest2src_end_timestamp.high;

  dummyBucket.pktSent   = ixp_netflow_rec->src2dest_packet_count.low;
  dummyBucket.pktRcvd   = ixp_netflow_rec->dest2src_packet_count.low;
  dummyBucket.bytesSent = ixp_netflow_rec->src2dest_byte_count.low;
  dummyBucket.bytesRcvd = ixp_netflow_rec->dest2src_byte_count.low;

  dummyBucket.srcMacAddress[0] = (ixp_netflow_rec->netflow_tuple.src_mac_hi >> 8)  & 0xff;
  dummyBucket.srcMacAddress[1] = (ixp_netflow_rec->netflow_tuple.src_mac_hi >> 0)  & 0xff;
  dummyBucket.srcMacAddress[2] = (ixp_netflow_rec->netflow_tuple.src_mac_lo >> 24) & 0xff;
  dummyBucket.srcMacAddress[3] = (ixp_netflow_rec->netflow_tuple.src_mac_lo >> 16) & 0xff;
  dummyBucket.srcMacAddress[4] = (ixp_netflow_rec->netflow_tuple.src_mac_lo >> 8)  & 0xff;
  dummyBucket.srcMacAddress[5] = (ixp_netflow_rec->netflow_tuple.src_mac_lo >> 0)  & 0xff;

  dummyBucket.dstMacAddress[0] = (ixp_netflow_rec->netflow_tuple.dest_mac_hi >> 8)  & 0xff;
  dummyBucket.dstMacAddress[1] = (ixp_netflow_rec->netflow_tuple.dest_mac_hi >> 0)  & 0xff;
  dummyBucket.dstMacAddress[2] = (ixp_netflow_rec->netflow_tuple.dest_mac_lo >> 24) & 0xff;
  dummyBucket.dstMacAddress[3] = (ixp_netflow_rec->netflow_tuple.dest_mac_lo >> 16) & 0xff;
  dummyBucket.dstMacAddress[4] = (ixp_netflow_rec->netflow_tuple.dest_mac_lo >> 8)  & 0xff;
  dummyBucket.dstMacAddress[5] = (ixp_netflow_rec->netflow_tuple.dest_mac_lo >> 0)  & 0xff;

  dummyBucket.vlanId = ixp_netflow_rec->netflow_tuple.vlan_id;

  dummyBucket.src.ipVersion = 4, dummyBucket.src.ipType.ipv4 = ixp_netflow_rec->netflow_tuple.src_ip;
  dummyBucket.dst.ipVersion = 4, dummyBucket.dst.ipType.ipv4 = ixp_netflow_rec->netflow_tuple.dest_ip;
  dummyBucket.proto = ixp_netflow_rec->netflow_tuple.protocol;
  dummyBucket.src2dstTos = ixp_netflow_rec->netflow_tuple.tos; /* FIX */
  dummyBucket.dst2srcTos = ixp_netflow_rec->netflow_tuple.tos; /* FIX */

  switch(ixp_netflow_rec->netflow_tuple.protocol) {
  case IPPROTO_TCP:
  case IPPROTO_UDP:
    dummyBucket.sport = ixp_netflow_rec->netflow_tuple.tcp_udp_src_port;
    dummyBucket.dport = ixp_netflow_rec->netflow_tuple.tcp_udp_dest_port;
    break;
  case IPPROTO_ICMP:
    dummyBucket.src2dstIcmpType  = ixp_netflow_rec->netflow_tuple.icmp_type;
    dummyBucket.dst2srcIcmpType  = ixp_netflow_rec->netflow_tuple.icmp_type;
    dummyBucket.src2dstIcmpFlags = ixp_netflow_rec->netflow_tuple.icmp_code;
    dummyBucket.dst2srcIcmpFlags = ixp_netflow_rec->netflow_tuple.icmp_code;
    break;
  }


  exportBucketToNetflow(&dummyBucket, 0 /* direct */, 0);
  exportBucketToNetflow(&dummyBucket, 1 /* reverse */, 0);
}

/* ********************************* */

void get_ixp_record(int fd) {
  unsigned char buf[2048];
  msg_t *msg =(msg_t *)buf;
  int len, rv=0;

  if((len = receive_ixp_msg(fd, buf, &len, sizeof(buf))) > 0) {
    switch(NPROBE_TYPE(msg)) {
    case NLMSG_ERROR:	// message response
      traceEvent(TRACE_WARNING, "IXP ack [error=%d][%s]", msg->err.err.error,
		 (msg->err.err.error < 0) ? strerror(-msg->err.err.error) : "");
      break;

    case NPROBE_EVENT:
      process_ixp_flow((ixp_netflow_record_t *)msg->evnt.dat, len-offsetof(evnt_t, dat));
      break;

    default:
      traceEvent(TRACE_WARNING, "Unknown Msg Type: %x", NPROBE_TYPE(msg));
      break;
    }
  }
}

/* ********************************* */

void test_ixp() {
  int ixp_id = init_ixp();

  if(ixp_id < 0) return;

  term_ixp(ixp_id);
}

#else
/* For non Linux platforms */

int init_ixp() { return(0); }
void term_ixp(int ixp_id) { ; }
void get_ixp_record(int fd) { ; }

#endif /* defined(linux) || defined(__linux__) */
