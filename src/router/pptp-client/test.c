/*

Packet reordering test implementation, intended to cause packets to be
reordered for testing pptpd and other servers.  Avoids the use of
pqueue.c so that it can be tested independently.

*/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "test.h"

/* whether we are asked to test ordering, obtained from command line */
extern int test_type;

/* rate at which to do test ordering changes */
extern int test_rate;

/* trigger cycle */
static int test_ordering_cycle = 0;

/* phase of reordering */
static int test_ordering_phase = 0;

/* swap a packet every now and then */
static ssize_t write_reordered_swap(int fd, const void *buf, size_t count)
{
  static void *pocket_buf = NULL;
  static int pocket_count = 0;
  int stat;

  switch (test_ordering_phase) {
  case 0: /* between triggers, send as normal */
    test_ordering_cycle++;
    if (test_ordering_cycle == test_rate) test_ordering_phase++;
    return write(fd, buf, count);
  case 1: /* triggered, swap a packet */
    test_ordering_cycle++;
    if (test_ordering_cycle == (test_rate+1)) {
      /* pocket the packet */
      pocket_count = count;
      pocket_buf = malloc(count);
      memcpy(pocket_buf, buf, count);
      log("test order swap, packet buffered");
      /* lie about the result */
      return count;
    } else {
      /* after this, reset to normal */
      test_ordering_cycle = 0;
      test_ordering_phase = 0;
      /* send the new packet first */
      stat = write(fd, buf, count);
      if (stat != count) return stat;
      /* then send the old packet next */
      stat = write(fd, pocket_buf, pocket_count);
      free(pocket_buf);
      log("test order swap, packets sent");
      return count;
    }
  default:
    return write(fd, buf, count);
  }
}

/* hold ten packets and send the eleventh, then the ten in order */
static ssize_t write_reordered_retransmit(int fd, const void *buf, size_t count)
{
  int test_length = 10;
  static void *pocket_buf[10];
  static int pocket_count[10];
  int stat, n;

  switch (test_ordering_phase) {
  case 0: /* between triggers, send as normal */
    test_ordering_cycle++;
    if (test_ordering_cycle == test_rate) test_ordering_phase++;
    return write(fd, buf, count);
  case 1: /* triggered, buffer the packets */
    test_ordering_cycle++;
    if (test_ordering_cycle == (test_rate+test_length)) {
      test_ordering_phase = 2;
    }
    /* pocket the packet */
    n = test_ordering_cycle - test_rate - 1;
    pocket_count[n] = count;
    pocket_buf[n] = malloc(count);
    memcpy(pocket_buf[n], buf, count);
    log("test order retransmit, packet buffered");
    /* lie about the result */
    return count;
  case 2:
    /* after this, reset to normal */
    test_ordering_cycle = 0;
    test_ordering_phase = 0;
    /* send the new packet first */
    stat = write(fd, buf, count);
    if (stat != count) return stat;
    /* send the buffered packets in normal order */
    for (n=0; n<test_length; n++) {
      stat = write(fd, pocket_buf[n], pocket_count[n]);
      /* ignores failures */
      free(pocket_buf[n]);
    }
    log("test order retransmit, packets sent");
    return count;
  default:
    return write(fd, buf, count);
  }
}

/* hold ten packets and send them in reverse order */
static ssize_t write_reordered_reverse(int fd, const void *buf, size_t count)
{
  int test_length = 10;
  static void *pocket_buf[10];
  static int pocket_count[10];
  int stat, n;

  switch (test_ordering_phase) {
  case 0: /* between triggers, send as normal */
    test_ordering_cycle++;
    if (test_ordering_cycle == test_rate) test_ordering_phase++;
    return write(fd, buf, count);
  case 1: /* triggered, buffer the packets */
    test_ordering_cycle++;
    if (test_ordering_cycle == (test_rate+test_length)) {
      test_ordering_phase = 2;
    }
    /* pocket the packet */
    n = test_ordering_cycle - test_rate - 1;
    pocket_count[n] = count;
    pocket_buf[n] = malloc(count);
    memcpy(pocket_buf[n], buf, count);
    log("test order reverse, packet buffered");
    /* lie about the result */
    return count;
  case 2:
    /* after this, reset to normal */
    test_ordering_cycle = 0;
    test_ordering_phase = 0;
    /* send the new packet first */
    stat = write(fd, buf, count);
    if (stat != count) return stat;
    /* send the buffered packets in reverse order */
    for (n=test_length-1; n>0; n--) {
      stat = write(fd, pocket_buf[n], pocket_count[n]);
      /* ignores failures */
      free(pocket_buf[n]);
    }
    log("test order reverse, packets sent");
    return count;
  default:
    return write(fd, buf, count);
  }
}

/* dispatcher for write reordering tests */
static ssize_t write_reordered(int fd, const void *buf, size_t count)
{
  switch (test_type) {
  case 1: /* swap a packet every now and then */
    return write_reordered_swap(fd, buf, count);
  case 2: /* hold ten packets and send the eleventh, then the ten in order */
    return write_reordered_retransmit(fd, buf, count);
  case 3: /* hold ten packets and send them in reverse order */
    return write_reordered_reverse(fd, buf, count);
  default:
    return write(fd, buf, count);
  }
}

struct test_redirections *test_redirections()
{
  static struct test_redirections *my = NULL;

  if (my == NULL) my = malloc(sizeof(struct test_redirections));

  my->write = write;
  if (test_type) my->write = write_reordered;

  return my;
}
