#ifndef PQUEUE_H
#define PQUEUE_H

#include <time.h>
#include <sys/time.h>
#include <sys/types.h>

/* wait this many seconds for missing packets before forgetting about them */
#define DEFAULT_PACKET_TIMEOUT 0.3
extern int packet_timeout_usecs;

/* assume packet is bad/spoofed if it's more than this many seqs ahead */
/* default is NOT to check - command line override via '--missing-window <n>' */
/* default value is 300 - recommended is 6000 for high speed data rates */
#define MISSING_WINDOW -1
extern int missing_window;

/* Packet queue structure: linked list of packets received out-of-order */
typedef struct pqueue {
  struct pqueue *next;
  struct pqueue *prev;
  u_int32_t seq;
  struct timeval expires;
  unsigned char *packet;
  int packlen;
  int capacity;
} pqueue_t;

int       pqueue_add  (u_int32_t seq, unsigned char *packet, int packlen);
int       pqueue_del  (pqueue_t *point);
pqueue_t *pqueue_head (void);
int       pqueue_expiry_time (pqueue_t *entry);

#endif /* PQUEUE_H */
