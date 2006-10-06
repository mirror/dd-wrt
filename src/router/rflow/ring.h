#ifndef __RING_H__
#define __RING_H__

#include "include/linux/ring.h"

#define RING_ERRBUF_SIZE 1024

struct ring_pkthdr {
  struct timeval ts;    /* time stamp */
  u_int32_t caplen;     /* length of portion present */
  u_int32_t len;        /* length this packet (off wire) */
};

struct ring_stat {
  u_int ps_recv;    /* number of packets received */
  u_int ps_drop;    /* number of packets dropped */
  u_int ps_ifdrop;  /* drops by interface XXX not yet supported */
};


typedef struct ring_s {
  struct ring_stat stat;

  int snapshot;
  int timeout;
  char* device;
  int clear_promisc;
  int ifindex;
  
  char *ring_buffer;
  char *ring_slots;
  int  ring_fd;
  int fd;
  FlowSlotInfo *slots_info;
  u_int page_id;
  u_int slot_id;
  u_int pkts_per_page;
  int linktype;
  int offset;
  int break_loop;   /* flag set to force break from packet-reading loop */

} ring_t;

typedef void (*ring_handler)(void*, const struct ring_pkthdr *, const unsigned char*);

ring_t* ring_open (const char* device, int snaplen, int promisc, char* ebuf);
void ring_close (ring_t* handle);
int ring_read_packet (ring_t *handle, ring_handler callback, void *userdata);
int ring_fileno (ring_t *handle);
unsigned int ring_datalink (ring_t *handle);
int ring_stats (ring_t *handle, struct ring_stat*);
char* ring_strerror (int errnum);
int ring_dispatch(ring_t*, int cnt, ring_handler cb, void* ctx);
#endif
