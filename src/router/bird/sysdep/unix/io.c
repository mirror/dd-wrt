/*
 *	BIRD Internet Routing Daemon -- Unix I/O
 *
 *	(c) 1998--2004 Martin Mares <mj@ucw.cz>
 *      (c) 2004       Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/* Unfortunately, some glibc versions hide parts of RFC 3542 API
   if _GNU_SOURCE is not defined. */
#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/icmp6.h>

#include "nest/bird.h"
#include "lib/lists.h"
#include "lib/resource.h"
#include "lib/timer.h"
#include "lib/socket.h"
#include "lib/event.h"
#include "lib/string.h"
#include "nest/iface.h"

#include "lib/unix.h"
#include "lib/sysio.h"

/* Maximum number of calls of tx handler for one socket in one
 * select iteration. Should be small enough to not monopolize CPU by
 * one protocol instance.
 */
#define MAX_STEPS 4

/* Maximum number of calls of rx handler for all sockets in one select
   iteration. RX callbacks are often much more costly so we limit
   this to gen small latencies */
#define MAX_RX_STEPS 4

/*
 *	Tracked Files
 */

struct rfile {
  resource r;
  FILE *f;
};

static void
rf_free(resource *r)
{
  struct rfile *a = (struct rfile *) r;

  fclose(a->f);
}

static void
rf_dump(resource *r)
{
  struct rfile *a = (struct rfile *) r;

  debug("(FILE *%p)\n", a->f);
}

static struct resclass rf_class = {
  "FILE",
  sizeof(struct rfile),
  rf_free,
  rf_dump,
  NULL,
  NULL
};

void *
tracked_fopen(pool *p, char *name, char *mode)
{
  FILE *f = fopen(name, mode);

  if (f)
    {
      struct rfile *r = ralloc(p, &rf_class);
      r->f = f;
    }
  return f;
}

/**
 * DOC: Timers
 *
 * Timers are resources which represent a wish of a module to call
 * a function at the specified time. The platform dependent code
 * doesn't guarantee exact timing, only that a timer function
 * won't be called before the requested time.
 *
 * In BIRD, time is represented by values of the &bird_clock_t type
 * which are integral numbers interpreted as a relative number of seconds since
 * some fixed time point in past. The current time can be read
 * from variable @now with reasonable accuracy and is monotonic. There is also
 * a current 'absolute' time in variable @now_real reported by OS.
 *
 * Each timer is described by a &timer structure containing a pointer
 * to the handler function (@hook), data private to this function (@data),
 * time the function should be called at (@expires, 0 for inactive timers),
 * for the other fields see |timer.h|.
 */

#define NEAR_TIMER_LIMIT 4

static list near_timers, far_timers;
static bird_clock_t first_far_timer = TIME_INFINITY;

/* now must be different from 0, because 0 is a special value in timer->expires */
bird_clock_t now = 1, now_real, boot_time;

static void
update_times_plain(void)
{
  bird_clock_t new_time = time(NULL);
  int delta = new_time - now_real;

  if ((delta >= 0) && (delta < 60))
    now += delta;
  else if (now_real != 0)
   log(L_WARN "Time jump, delta %d s", delta);

  now_real = new_time;
}

static void
update_times_gettime(void)
{
  struct timespec ts;
  int rv;

  rv = clock_gettime(CLOCK_MONOTONIC, &ts);
  if (rv != 0)
    die("clock_gettime: %m");

  if (ts.tv_sec != now) {
    if (ts.tv_sec < now)
      log(L_ERR "Monotonic timer is broken");

    now = ts.tv_sec;
    now_real = time(NULL);
  }
}

static int clock_monotonic_available;

static inline void
update_times(void)
{
  if (clock_monotonic_available)
    update_times_gettime();
  else
    update_times_plain();
}

static inline void
init_times(void)
{
 struct timespec ts;
 clock_monotonic_available = (clock_gettime(CLOCK_MONOTONIC, &ts) == 0);
 if (!clock_monotonic_available)
   log(L_WARN "Monotonic timer is missing");
}


static void
tm_free(resource *r)
{
  timer *t = (timer *) r;

  tm_stop(t);
}

static void
tm_dump(resource *r)
{
  timer *t = (timer *) r;

  debug("(code %p, data %p, ", t->hook, t->data);
  if (t->randomize)
    debug("rand %d, ", t->randomize);
  if (t->recurrent)
    debug("recur %d, ", t->recurrent);
  if (t->expires)
    debug("expires in %d sec)\n", t->expires - now);
  else
    debug("inactive)\n");
}

static struct resclass tm_class = {
  "Timer",
  sizeof(timer),
  tm_free,
  tm_dump,
  NULL,
  NULL
};

/**
 * tm_new - create a timer
 * @p: pool
 *
 * This function creates a new timer resource and returns
 * a pointer to it. To use the timer, you need to fill in
 * the structure fields and call tm_start() to start timing.
 */
timer *
tm_new(pool *p)
{
  timer *t = ralloc(p, &tm_class);
  return t;
}

static inline void
tm_insert_near(timer *t)
{
  node *n = HEAD(near_timers);

  while (n->next && (SKIP_BACK(timer, n, n)->expires < t->expires))
    n = n->next;
  insert_node(&t->n, n->prev);
}

/**
 * tm_start - start a timer
 * @t: timer
 * @after: number of seconds the timer should be run after
 *
 * This function schedules the hook function of the timer to
 * be called after @after seconds. If the timer has been already
 * started, it's @expire time is replaced by the new value.
 *
 * You can have set the @randomize field of @t, the timeout
 * will be increased by a random number of seconds chosen
 * uniformly from range 0 .. @randomize.
 *
 * You can call tm_start() from the handler function of the timer
 * to request another run of the timer. Also, you can set the @recurrent
 * field to have the timer re-added automatically with the same timeout.
 */
void
tm_start(timer *t, unsigned after)
{
  bird_clock_t when;

  if (t->randomize)
    after += random() % (t->randomize + 1);
  when = now + after;
  if (t->expires == when)
    return;
  if (t->expires)
    rem_node(&t->n);
  t->expires = when;
  if (after <= NEAR_TIMER_LIMIT)
    tm_insert_near(t);
  else
    {
      if (!first_far_timer || first_far_timer > when)
	first_far_timer = when;
      add_tail(&far_timers, &t->n);
    }
}

/**
 * tm_stop - stop a timer
 * @t: timer
 *
 * This function stops a timer. If the timer is already stopped,
 * nothing happens.
 */
void
tm_stop(timer *t)
{
  if (t->expires)
    {
      rem_node(&t->n);
      t->expires = 0;
    }
}

static void
tm_dump_them(char *name, list *l)
{
  node *n;
  timer *t;

  debug("%s timers:\n", name);
  WALK_LIST(n, *l)
    {
      t = SKIP_BACK(timer, n, n);
      debug("%p ", t);
      tm_dump(&t->r);
    }
  debug("\n");
}

void
tm_dump_all(void)
{
  tm_dump_them("Near", &near_timers);
  tm_dump_them("Far", &far_timers);
}

static inline time_t
tm_first_shot(void)
{
  time_t x = first_far_timer;

  if (!EMPTY_LIST(near_timers))
    {
      timer *t = SKIP_BACK(timer, n, HEAD(near_timers));
      if (t->expires < x)
	x = t->expires;
    }
  return x;
}

static void
tm_shot(void)
{
  timer *t;
  node *n, *m;

  if (first_far_timer <= now)
    {
      bird_clock_t limit = now + NEAR_TIMER_LIMIT;
      first_far_timer = TIME_INFINITY;
      n = HEAD(far_timers);
      while (m = n->next)
	{
	  t = SKIP_BACK(timer, n, n);
	  if (t->expires <= limit)
	    {
	      rem_node(n);
	      tm_insert_near(t);
	    }
	  else if (t->expires < first_far_timer)
	    first_far_timer = t->expires;
	  n = m;
	}
    }
  while ((n = HEAD(near_timers)) -> next)
    {
      int delay;
      t = SKIP_BACK(timer, n, n);
      if (t->expires > now)
	break;
      rem_node(n);
      delay = t->expires - now;
      t->expires = 0;
      if (t->recurrent)
	{
	  int i = t->recurrent - delay;
	  if (i < 0)
	    i = 0;
	  tm_start(t, i);
	}
      t->hook(t);
    }
}

/**
 * tm_parse_datetime - parse a date and time
 * @x: datetime string
 *
 * tm_parse_datetime() takes a textual representation of
 * a date and time (dd-mm-yyyy hh:mm:ss)
 * and converts it to the corresponding value of type &bird_clock_t.
 */
bird_clock_t
tm_parse_datetime(char *x)
{
  struct tm tm;
  int n;
  time_t t;

  if (sscanf(x, "%d-%d-%d %d:%d:%d%n", &tm.tm_mday, &tm.tm_mon, &tm.tm_year, &tm.tm_hour, &tm.tm_min, &tm.tm_sec, &n) != 6 || x[n])
    return tm_parse_date(x);
  tm.tm_mon--;
  tm.tm_year -= 1900;
  t = mktime(&tm);
  if (t == (time_t) -1)
    return 0;
  return t;
}
/**
 * tm_parse_date - parse a date
 * @x: date string
 *
 * tm_parse_date() takes a textual representation of a date (dd-mm-yyyy)
 * and converts it to the corresponding value of type &bird_clock_t.
 */
bird_clock_t
tm_parse_date(char *x)
{
  struct tm tm;
  int n;
  time_t t;

  if (sscanf(x, "%d-%d-%d%n", &tm.tm_mday, &tm.tm_mon, &tm.tm_year, &n) != 3 || x[n])
    return 0;
  tm.tm_mon--;
  tm.tm_year -= 1900;
  tm.tm_hour = tm.tm_min = tm.tm_sec = 0;
  t = mktime(&tm);
  if (t == (time_t) -1)
    return 0;
  return t;
}

static void
tm_format_reltime(char *x, struct tm *tm, bird_clock_t delta)
{
  static char *month_names[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
				   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

  if (delta < 20*3600)
    bsprintf(x, "%02d:%02d", tm->tm_hour, tm->tm_min);
  else if (delta < 360*86400)
    bsprintf(x, "%s%02d", month_names[tm->tm_mon], tm->tm_mday);
  else
    bsprintf(x, "%d", tm->tm_year+1900);
}

#include "conf/conf.h"

/**
 * tm_format_datetime - convert date and time to textual representation
 * @x: destination buffer of size %TM_DATETIME_BUFFER_SIZE
 * @t: time
 *
 * This function formats the given relative time value @t to a textual
 * date/time representation (dd-mm-yyyy hh:mm:ss) in real time.
 */
void
tm_format_datetime(char *x, struct timeformat *fmt_spec, bird_clock_t t)
{
  const char *fmt_used;
  struct tm *tm;
  bird_clock_t delta = now - t;
  t = now_real - delta;
  tm = localtime(&t);

  if (fmt_spec->fmt1 == NULL)
    return tm_format_reltime(x, tm, delta);

  if ((fmt_spec->limit == 0) || (delta < fmt_spec->limit))
    fmt_used = fmt_spec->fmt1;
  else
    fmt_used = fmt_spec->fmt2;

  int rv = strftime(x, TM_DATETIME_BUFFER_SIZE, fmt_used, tm);
  if (((rv == 0) && fmt_used[0]) || (rv == TM_DATETIME_BUFFER_SIZE))
    strcpy(x, "<too-long>");
}

/**
 * DOC: Sockets
 *
 * Socket resources represent network connections. Their data structure (&socket)
 * contains a lot of fields defining the exact type of the socket, the local and
 * remote addresses and ports, pointers to socket buffers and finally pointers to
 * hook functions to be called when new data have arrived to the receive buffer
 * (@rx_hook), when the contents of the transmit buffer have been transmitted
 * (@tx_hook) and when an error or connection close occurs (@err_hook).
 *
 * Freeing of sockets from inside socket hooks is perfectly safe.
 */

#ifndef SOL_IP
#define SOL_IP IPPROTO_IP
#endif

#ifndef SOL_IPV6
#define SOL_IPV6 IPPROTO_IPV6
#endif

static list sock_list;
static struct birdsock *current_sock;
static struct birdsock *stored_sock;
static int sock_recalc_fdsets_p;

static inline sock *
sk_next(sock *s)
{
  if (!s->n.next->next)
    return NULL;
  else
    return SKIP_BACK(sock, n, s->n.next);
}

static void
sk_alloc_bufs(sock *s)
{
  if (!s->rbuf && s->rbsize)
    s->rbuf = s->rbuf_alloc = xmalloc(s->rbsize);
  s->rpos = s->rbuf;
  if (!s->tbuf && s->tbsize)
    s->tbuf = s->tbuf_alloc = xmalloc(s->tbsize);
  s->tpos = s->ttx = s->tbuf;
}

static void
sk_free_bufs(sock *s)
{
  if (s->rbuf_alloc)
    {
      xfree(s->rbuf_alloc);
      s->rbuf = s->rbuf_alloc = NULL;
    }
  if (s->tbuf_alloc)
    {
      xfree(s->tbuf_alloc);
      s->tbuf = s->tbuf_alloc = NULL;
    }
}

static void
sk_free(resource *r)
{
  sock *s = (sock *) r;

  sk_free_bufs(s);
  if (s->fd >= 0)
    {
      close(s->fd);
      if (s == current_sock)
	current_sock = sk_next(s);
      if (s == stored_sock)
	stored_sock = sk_next(s);
      rem_node(&s->n);
      sock_recalc_fdsets_p = 1;
    }
}

void
sk_reallocate(sock *s)
{
  sk_free_bufs(s);
  sk_alloc_bufs(s);
}

static void
sk_dump(resource *r)
{
  sock *s = (sock *) r;
  static char *sk_type_names[] = { "TCP<", "TCP>", "TCP", "UDP", "UDP/MC", "IP", "IP/MC", "MAGIC", "UNIX<", "UNIX", "DEL!" };

  debug("(%s, ud=%p, sa=%08x, sp=%d, da=%08x, dp=%d, tos=%d, ttl=%d, if=%s)\n",
	sk_type_names[s->type],
	s->data,
	s->saddr,
	s->sport,
	s->daddr,
	s->dport,
	s->tos,
	s->ttl,
	s->iface ? s->iface->name : "none");
}

static struct resclass sk_class = {
  "Socket",
  sizeof(sock),
  sk_free,
  sk_dump,
  NULL,
  NULL
};

/**
 * sk_new - create a socket
 * @p: pool
 *
 * This function creates a new socket resource. If you want to use it,
 * you need to fill in all the required fields of the structure and
 * call sk_open() to do the actual opening of the socket.
 *
 * The real function name is sock_new(), sk_new() is a macro wrapper
 * to avoid collision with OpenSSL.
 */
sock *
sock_new(pool *p)
{
  sock *s = ralloc(p, &sk_class);
  s->pool = p;
  // s->saddr = s->daddr = IPA_NONE;
  s->tos = s->ttl = -1;
  s->fd = -1;
  return s;
}

static void
sk_insert(sock *s)
{
  add_tail(&sock_list, &s->n);
  sock_recalc_fdsets_p = 1;
}

#ifdef IPV6

void
fill_in_sockaddr(struct sockaddr_in6 *sa, ip_addr a, struct iface *ifa, unsigned port)
{
  memset(sa, 0, sizeof (struct sockaddr_in6));
  sa->sin6_family = AF_INET6;
  sa->sin6_port = htons(port);
  sa->sin6_flowinfo = 0;
#ifdef HAVE_SIN_LEN
  sa->sin6_len = sizeof(struct sockaddr_in6);
#endif
  set_inaddr(&sa->sin6_addr, a);

  if (ifa && ipa_has_link_scope(a))
    sa->sin6_scope_id = ifa->index;
}

void
get_sockaddr(struct sockaddr_in6 *sa, ip_addr *a, struct iface **ifa, unsigned *port, int check)
{
  if (check && sa->sin6_family != AF_INET6)
    bug("get_sockaddr called for wrong address family (%d)", sa->sin6_family);
  if (port)
    *port = ntohs(sa->sin6_port);
  memcpy(a, &sa->sin6_addr, sizeof(*a));
  ipa_ntoh(*a);

  if (ifa && ipa_has_link_scope(*a))
    *ifa = if_find_by_index(sa->sin6_scope_id);
}

#else

void
fill_in_sockaddr(struct sockaddr_in *sa, ip_addr a, struct iface *ifa, unsigned port)
{
  memset (sa, 0, sizeof (struct sockaddr_in));
  sa->sin_family = AF_INET;
  sa->sin_port = htons(port);
#ifdef HAVE_SIN_LEN
  sa->sin_len = sizeof(struct sockaddr_in);
#endif
  set_inaddr(&sa->sin_addr, a);
}

void
get_sockaddr(struct sockaddr_in *sa, ip_addr *a, struct iface **ifa, unsigned *port, int check)
{
  if (check && sa->sin_family != AF_INET)
    bug("get_sockaddr called for wrong address family (%d)", sa->sin_family);
  if (port)
    *port = ntohs(sa->sin_port);
  memcpy(a, &sa->sin_addr.s_addr, sizeof(*a));
  ipa_ntoh(*a);
}

#endif


#ifdef IPV6

/* PKTINFO handling is also standardized in IPv6 */
#define CMSG_RX_SPACE CMSG_SPACE(sizeof(struct in6_pktinfo))
#define CMSG_TX_SPACE CMSG_SPACE(sizeof(struct in6_pktinfo))

/*
 * RFC 2292 uses IPV6_PKTINFO for both the socket option and the cmsg
 * type, RFC 3542 changed the socket option to IPV6_RECVPKTINFO. If we
 * don't have IPV6_RECVPKTINFO we suppose the OS implements the older
 * RFC and we use IPV6_PKTINFO.
 */
#ifndef IPV6_RECVPKTINFO
#define IPV6_RECVPKTINFO IPV6_PKTINFO
#endif

static char *
sysio_register_cmsgs(sock *s)
{
  int ok = 1;
  if ((s->flags & SKF_LADDR_RX) &&
      setsockopt(s->fd, IPPROTO_IPV6, IPV6_RECVPKTINFO, &ok, sizeof(ok)) < 0)
    return "IPV6_RECVPKTINFO";

  return NULL;
}

static void
sysio_process_rx_cmsgs(sock *s, struct msghdr *msg)
{
  struct cmsghdr *cm;
  struct in6_pktinfo *pi = NULL;

  if (!(s->flags & SKF_LADDR_RX))
    return;

  for (cm = CMSG_FIRSTHDR(msg); cm != NULL; cm = CMSG_NXTHDR(msg, cm))
    {
      if (cm->cmsg_level == IPPROTO_IPV6 && cm->cmsg_type == IPV6_PKTINFO)
	pi = (struct in6_pktinfo *) CMSG_DATA(cm);
    }

  if (!pi)
    {
      s->laddr = IPA_NONE;
      s->lifindex = 0;
      return;
    }

  get_inaddr(&s->laddr, &pi->ipi6_addr);
  s->lifindex = pi->ipi6_ifindex;
  return;
}

/*
static void
sysio_prepare_tx_cmsgs(sock *s, struct msghdr *msg, void *cbuf, size_t cbuflen)
{
  struct cmsghdr *cm;
  struct in6_pktinfo *pi;

  if (!(s->flags & SKF_LADDR_TX))
    return;

  msg->msg_control = cbuf;
  msg->msg_controllen = cbuflen;

  cm = CMSG_FIRSTHDR(msg);
  cm->cmsg_level = IPPROTO_IPV6;
  cm->cmsg_type = IPV6_PKTINFO;
  cm->cmsg_len = CMSG_LEN(sizeof(*pi));

  pi = (struct in6_pktinfo *) CMSG_DATA(cm);
  set_inaddr(&pi->ipi6_addr, s->saddr);
  pi->ipi6_ifindex = s->iface ? s->iface->index : 0;

  msg->msg_controllen = cm->cmsg_len;
  return;
}
*/
#endif

static char *
sk_set_ttl_int(sock *s)
{
#ifdef IPV6
  if (setsockopt(s->fd, SOL_IPV6, IPV6_UNICAST_HOPS, &s->ttl, sizeof(s->ttl)) < 0)
    return "IPV6_UNICAST_HOPS";
#else
  if (setsockopt(s->fd, SOL_IP, IP_TTL, &s->ttl, sizeof(s->ttl)) < 0)
    return "IP_TTL";
#ifdef CONFIG_UNIX_DONTROUTE
  int one = 1;
  if (s->ttl == 1 && setsockopt(s->fd, SOL_SOCKET, SO_DONTROUTE, &one, sizeof(one)) < 0)
    return "SO_DONTROUTE";
#endif 
#endif
  return NULL;
}

#define ERR(x) do { err = x; goto bad; } while(0)
#define WARN(x) log(L_WARN "sk_setup: %s: %m", x)

static char *
sk_setup(sock *s)
{
  int fd = s->fd;
  char *err = NULL;

  if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
    ERR("fcntl(O_NONBLOCK)");
  if (s->type == SK_UNIX)
    return NULL;
#ifndef IPV6
  if ((s->tos >= 0) && setsockopt(fd, SOL_IP, IP_TOS, &s->tos, sizeof(s->tos)) < 0)
    WARN("IP_TOS");
#endif

#ifdef IPV6
  int v = 1;
  if ((s->flags & SKF_V6ONLY) && setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &v, sizeof(v)) < 0)
    WARN("IPV6_V6ONLY");
#endif

  if (s->ttl >= 0)
    err = sk_set_ttl_int(s);

  sysio_register_cmsgs(s);
bad:
  return err;
}

/**
 * sk_set_ttl - set transmit TTL for given socket.
 * @s: socket
 * @ttl: TTL value
 *
 * Set TTL for already opened connections when TTL was not set before.
 * Useful for accepted connections when different ones should have 
 * different TTL.
 *
 * Result: 0 for success, -1 for an error.
 */

int
sk_set_ttl(sock *s, int ttl)
{
  char *err;

  s->ttl = ttl;
  if (err = sk_set_ttl_int(s))
    log(L_ERR "sk_set_ttl: %s: %m", err);

  return (err ? -1 : 0);
}

/**
 * sk_set_min_ttl - set minimal accepted TTL for given socket.
 * @s: socket
 * @ttl: TTL value
 *
 * Can be used in TTL security implementation
 *
 * Result: 0 for success, -1 for an error.
 */

int
sk_set_min_ttl(sock *s, int ttl)
{
  int err;
#ifdef IPV6
  err = sk_set_min_ttl6(s, ttl);
#else
  err = sk_set_min_ttl4(s, ttl);
#endif

  return err;
}

/**
 * sk_set_md5_auth - add / remove MD5 security association for given socket.
 * @s: socket
 * @a: IP address of the other side
 * @ifa: Interface for link-local IP address
 * @passwd: password used for MD5 authentication
 *
 * In TCP MD5 handling code in kernel, there is a set of pairs
 * (address, password) used to choose password according to
 * address of the other side. This function is useful for
 * listening socket, for active sockets it is enough to set
 * s->password field.
 *
 * When called with passwd != NULL, the new pair is added,
 * When called with passwd == NULL, the existing pair is removed.
 *
 * Result: 0 for success, -1 for an error.
 */

int
sk_set_md5_auth(sock *s, ip_addr a, struct iface *ifa, char *passwd)
{
  sockaddr sa;
  fill_in_sockaddr(&sa, a, ifa, 0);
  return sk_set_md5_auth_int(s, &sa, passwd);
}

int
sk_set_broadcast(sock *s, int enable)
{
  if (setsockopt(s->fd, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable)) < 0)
    {
      log(L_ERR "sk_set_broadcast: SO_BROADCAST: %m");
      return -1;
    }

  return 0;
}


#ifdef IPV6

int
sk_set_ipv6_checksum(sock *s, int offset)
{
  if (setsockopt(s->fd, IPPROTO_IPV6, IPV6_CHECKSUM, &offset, sizeof(offset)) < 0)
    {
      log(L_ERR "sk_set_ipv6_checksum: IPV6_CHECKSUM: %m");
      return -1;
    }

  return 0;
}

int
sk_set_icmp_filter(sock *s, int p1, int p2)
{
  /* a bit of lame interface, but it is here only for Radv */
  struct icmp6_filter f;

  ICMP6_FILTER_SETBLOCKALL(&f);
  ICMP6_FILTER_SETPASS(p1, &f);
  ICMP6_FILTER_SETPASS(p2, &f);

  if (setsockopt(s->fd, IPPROTO_ICMPV6, ICMP6_FILTER, &f, sizeof(f)) < 0)
    {
      log(L_ERR "sk_setup_icmp_filter: ICMP6_FILTER: %m");
      return -1;
    }

  return 0;
}

int
sk_setup_multicast(sock *s)
{
  char *err;
  int zero = 0;
  int index;

  ASSERT(s->iface && s->iface->addr);

  index = s->iface->index;
  if (setsockopt(s->fd, SOL_IPV6, IPV6_MULTICAST_HOPS, &s->ttl, sizeof(s->ttl)) < 0)
    ERR("IPV6_MULTICAST_HOPS");
  if (setsockopt(s->fd, SOL_IPV6, IPV6_MULTICAST_LOOP, &zero, sizeof(zero)) < 0)
    ERR("IPV6_MULTICAST_LOOP");
  if (setsockopt(s->fd, SOL_IPV6, IPV6_MULTICAST_IF, &index, sizeof(index)) < 0)
    ERR("IPV6_MULTICAST_IF");

  if (err = sysio_bind_to_iface(s))
    goto bad;

  return 0;

bad:
  log(L_ERR "sk_setup_multicast: %s: %m", err);
  return -1;
}

int
sk_join_group(sock *s, ip_addr maddr)
{
  struct ipv6_mreq mreq;

  set_inaddr(&mreq.ipv6mr_multiaddr, maddr);

#ifdef CONFIG_IPV6_GLIBC_20
  mreq.ipv6mr_ifindex = s->iface->index;
#else
  mreq.ipv6mr_interface = s->iface->index;
#endif

  if (setsockopt(s->fd, SOL_IPV6, IPV6_JOIN_GROUP, &mreq, sizeof(mreq)) < 0)
    {
      log(L_ERR "sk_join_group: IPV6_JOIN_GROUP: %m");
      return -1;
    }

  return 0;
}

int
sk_leave_group(sock *s, ip_addr maddr)
{
  struct ipv6_mreq mreq;
	
  set_inaddr(&mreq.ipv6mr_multiaddr, maddr);

#ifdef CONFIG_IPV6_GLIBC_20
  mreq.ipv6mr_ifindex = s->iface->index;
#else
  mreq.ipv6mr_interface = s->iface->index;
#endif

  if (setsockopt(s->fd, SOL_IPV6, IPV6_LEAVE_GROUP, &mreq, sizeof(mreq)) < 0)
    {
      log(L_ERR "sk_leave_group: IPV6_LEAVE_GROUP: %m");
      return -1;
    }

  return 0;
}

#else /* IPV4 */

int
sk_setup_multicast(sock *s)
{
  char *err;

  ASSERT(s->iface && s->iface->addr);

  if (err = sysio_setup_multicast(s))
    {
      log(L_ERR "sk_setup_multicast: %s: %m", err);
      return -1;
    }

  return 0;
}

int
sk_join_group(sock *s, ip_addr maddr)
{
 char *err;

 if (err = sysio_join_group(s, maddr))
    {
      log(L_ERR "sk_join_group: %s: %m", err);
      return -1;
    }

  return 0;
}

int
sk_leave_group(sock *s, ip_addr maddr)
{
 char *err;

 if (err = sysio_leave_group(s, maddr))
    {
      log(L_ERR "sk_leave_group: %s: %m", err);
      return -1;
    }

  return 0;
}

#endif 


static void
sk_tcp_connected(sock *s)
{
  sockaddr lsa;
  int lsa_len = sizeof(lsa);
  if (getsockname(s->fd, (struct sockaddr *) &lsa, &lsa_len) == 0)
    get_sockaddr(&lsa, &s->saddr, &s->iface, &s->sport, 1);

  s->type = SK_TCP;
  sk_alloc_bufs(s);
  s->tx_hook(s);
}

static int
sk_passive_connected(sock *s, struct sockaddr *sa, int al, int type)
{
  int fd = accept(s->fd, sa, &al);
  if (fd >= 0)
    {
      sock *t = sk_new(s->pool);
      char *err;
      t->type = type;
      t->fd = fd;
      t->ttl = s->ttl;
      t->tos = s->tos;
      t->rbsize = s->rbsize;
      t->tbsize = s->tbsize;
      if (type == SK_TCP)
	{
	  sockaddr lsa;
	  int lsa_len = sizeof(lsa);
	  if (getsockname(fd, (struct sockaddr *) &lsa, &lsa_len) == 0)
	    get_sockaddr(&lsa, &t->saddr, &t->iface, &t->sport, 1);

	  get_sockaddr((sockaddr *) sa, &t->daddr, &t->iface, &t->dport, 1);
	}
      sk_insert(t);
      if (err = sk_setup(t))
	{
	  log(L_ERR "Incoming connection: %s: %m", err);
	  rfree(t);
	  return 1;
	}
      sk_alloc_bufs(t);
      s->rx_hook(t, 0);
      return 1;
    }
  else if (errno != EINTR && errno != EAGAIN)
    {
      s->err_hook(s, errno);
    }
  return 0;
}

/**
 * sk_open - open a socket
 * @s: socket
 *
 * This function takes a socket resource created by sk_new() and
 * initialized by the user and binds a corresponding network connection
 * to it.
 *
 * Result: 0 for success, -1 for an error.
 */
int
sk_open(sock *s)
{
  int fd;
  sockaddr sa;
  int one = 1;
  int type = s->type;
  int has_src = ipa_nonzero(s->saddr) || s->sport;
  char *err;

  switch (type)
    {
    case SK_TCP_ACTIVE:
      s->ttx = "";			/* Force s->ttx != s->tpos */
      /* Fall thru */
    case SK_TCP_PASSIVE:
      fd = socket(BIRD_PF, SOCK_STREAM, IPPROTO_TCP);
      break;
    case SK_UDP:
      fd = socket(BIRD_PF, SOCK_DGRAM, IPPROTO_UDP);
      break;
    case SK_IP:
      fd = socket(BIRD_PF, SOCK_RAW, s->dport);
      break;
    case SK_MAGIC:
      fd = s->fd;
      break;
    default:
      bug("sk_open() called for invalid sock type %d", type);
    }
  if (fd < 0)
    die("sk_open: socket: %m");
  s->fd = fd;

  if (err = sk_setup(s))
    goto bad;

  if (has_src)
    {
      int port;

      if (type == SK_IP)
	port = 0;
      else
	{
	  port = s->sport;
	  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0)
	    ERR("SO_REUSEADDR");
	}
      fill_in_sockaddr(&sa, s->saddr, s->iface, port);
      if (bind(fd, (struct sockaddr *) &sa, sizeof(sa)) < 0)
	ERR("bind");
    }
  fill_in_sockaddr(&sa, s->daddr, s->iface, s->dport);

  if (s->password)
    {
      int rv = sk_set_md5_auth_int(s, &sa, s->password);
      if (rv < 0)
	goto bad_no_log;
    }

  switch (type)
    {
    case SK_TCP_ACTIVE:
      if (connect(fd, (struct sockaddr *) &sa, sizeof(sa)) >= 0)
	sk_tcp_connected(s);
      else if (errno != EINTR && errno != EAGAIN && errno != EINPROGRESS &&
	       errno != ECONNREFUSED && errno != EHOSTUNREACH && errno != ENETUNREACH)
	ERR("connect");
      break;
    case SK_TCP_PASSIVE:
      if (listen(fd, 8))
	ERR("listen");
      break;
    case SK_MAGIC:
      break;
    default:
      sk_alloc_bufs(s);
#ifdef IPV6
#ifdef IPV6_MTU_DISCOVER
      {
	int dont = IPV6_PMTUDISC_DONT;
	if (setsockopt(fd, SOL_IPV6, IPV6_MTU_DISCOVER, &dont, sizeof(dont)) < 0)
	  ERR("IPV6_MTU_DISCOVER");
      }
#endif
#else
#ifdef IP_PMTUDISC
      {
	int dont = IP_PMTUDISC_DONT;
	if (setsockopt(fd, SOL_IP, IP_PMTUDISC, &dont, sizeof(dont)) < 0)
	  ERR("IP_PMTUDISC");
      }
#endif
#endif
    }

  sk_insert(s);
  return 0;

bad:
  log(L_ERR "sk_open: %s: %m", err);
bad_no_log:
  close(fd);
  s->fd = -1;
  return -1;
}

void
sk_open_unix(sock *s, char *name)
{
  int fd;
  struct sockaddr_un sa;
  char *err;

  fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd < 0)
    ERR("socket");
  s->fd = fd;
  if (err = sk_setup(s))
    goto bad;
  unlink(name);

  /* Path length checked in test_old_bird() */
  sa.sun_family = AF_UNIX;
  strcpy(sa.sun_path, name);
  if (bind(fd, (struct sockaddr *) &sa, SUN_LEN(&sa)) < 0)
    ERR("bind");
  if (listen(fd, 8))
    ERR("listen");
  sk_insert(s);
  return;

 bad:
  log(L_ERR "sk_open_unix: %s: %m", err);
  die("Unable to create control socket %s", name);
}

static inline void reset_tx_buffer(sock *s) { s->ttx = s->tpos = s->tbuf; }

static int
sk_maybe_write(sock *s)
{
  int e;

  switch (s->type)
    {
    case SK_TCP:
    case SK_MAGIC:
    case SK_UNIX:
      while (s->ttx != s->tpos)
	{
	  e = write(s->fd, s->ttx, s->tpos - s->ttx);
	  if (e < 0)
	    {
	      if (errno != EINTR && errno != EAGAIN)
		{
		  reset_tx_buffer(s);
		  /* EPIPE is just a connection close notification during TX */
		  s->err_hook(s, (errno != EPIPE) ? errno : 0);
		  return -1;
		}
	      return 0;
	    }
	  s->ttx += e;
	}
      reset_tx_buffer(s);
      return 1;
    case SK_UDP:
    case SK_IP:
      {
	if (s->tbuf == s->tpos)
	  return 1;

	sockaddr sa;
	fill_in_sockaddr(&sa, s->daddr, s->iface, s->dport);

	struct iovec iov = {s->tbuf, s->tpos - s->tbuf};
	// byte cmsg_buf[CMSG_TX_SPACE];

	struct msghdr msg = {
	  .msg_name = &sa,
	  .msg_namelen = sizeof(sa),
	  .msg_iov = &iov,
	  .msg_iovlen = 1};

	// sysio_prepare_tx_cmsgs(s, &msg, cmsg_buf, sizeof(cmsg_buf));
	e = sendmsg(s->fd, &msg, 0);

	if (e < 0)
	  {
	    if (errno != EINTR && errno != EAGAIN)
	      {
		reset_tx_buffer(s);
		s->err_hook(s, errno);
		return -1;
	      }
	    return 0;
	  }
	reset_tx_buffer(s);
	return 1;
      }
    default:
      bug("sk_maybe_write: unknown socket type %d", s->type);
    }
}

int
sk_rx_ready(sock *s)
{
  fd_set rd, wr;
  struct timeval timo;
  int rv;

  FD_ZERO(&rd);
  FD_ZERO(&wr);
  FD_SET(s->fd, &rd);

  timo.tv_sec = 0;
  timo.tv_usec = 0;

 redo:
  rv = select(s->fd+1, &rd, &wr, NULL, &timo);
  
  if ((rv < 0) && (errno == EINTR || errno == EAGAIN))
    goto redo;

  return rv;
}

/**
 * sk_send - send data to a socket
 * @s: socket
 * @len: number of bytes to send
 *
 * This function sends @len bytes of data prepared in the
 * transmit buffer of the socket @s to the network connection.
 * If the packet can be sent immediately, it does so and returns
 * 1, else it queues the packet for later processing, returns 0
 * and calls the @tx_hook of the socket when the tranmission
 * takes place.
 */
int
sk_send(sock *s, unsigned len)
{
  s->ttx = s->tbuf;
  s->tpos = s->tbuf + len;
  return sk_maybe_write(s);
}

/**
 * sk_send_to - send data to a specific destination
 * @s: socket
 * @len: number of bytes to send
 * @addr: IP address to send the packet to
 * @port: port to send the packet to
 *
 * This is a sk_send() replacement for connection-less packet sockets
 * which allows destination of the packet to be chosen dynamically.
 */
int
sk_send_to(sock *s, unsigned len, ip_addr addr, unsigned port)
{
  s->daddr = addr;
  s->dport = port;
  s->ttx = s->tbuf;
  s->tpos = s->tbuf + len;
  return sk_maybe_write(s);
}

/*
int
sk_send_full(sock *s, unsigned len, struct iface *ifa,
	     ip_addr saddr, ip_addr daddr, unsigned dport)
{
  s->iface = ifa;
  s->saddr = saddr;
  s->daddr = daddr;
  s->dport = dport;
  s->ttx = s->tbuf;
  s->tpos = s->tbuf + len;
  return sk_maybe_write(s);
}
*/

static int
sk_read(sock *s)
{
  switch (s->type)
    {
    case SK_TCP_PASSIVE:
      {
	sockaddr sa;
	return sk_passive_connected(s, (struct sockaddr *) &sa, sizeof(sa), SK_TCP);
      }
    case SK_UNIX_PASSIVE:
      {
	struct sockaddr_un sa;
	return sk_passive_connected(s, (struct sockaddr *) &sa, sizeof(sa), SK_UNIX);
      }
    case SK_TCP:
    case SK_UNIX:
      {
	int c = read(s->fd, s->rpos, s->rbuf + s->rbsize - s->rpos);

	if (c < 0)
	  {
	    if (errno != EINTR && errno != EAGAIN)
	      s->err_hook(s, errno);
	  }
	else if (!c)
	  s->err_hook(s, 0);
	else
	  {
	    s->rpos += c;
	    if (s->rx_hook(s, s->rpos - s->rbuf))
	      {
		/* We need to be careful since the socket could have been deleted by the hook */
		if (current_sock == s)
		  s->rpos = s->rbuf;
	      }
	    return 1;
	  }
	return 0;
      }
    case SK_MAGIC:
      return s->rx_hook(s, 0);
    default:
      {
	sockaddr sa;
	int e;

	struct iovec iov = {s->rbuf, s->rbsize};
	byte cmsg_buf[CMSG_RX_SPACE];

	struct msghdr msg = {
	  .msg_name = &sa,
	  .msg_namelen = sizeof(sa),
	  .msg_iov = &iov,
	  .msg_iovlen = 1,
	  .msg_control = cmsg_buf,
	  .msg_controllen = sizeof(cmsg_buf),
	  .msg_flags = 0};

	e = recvmsg(s->fd, &msg, 0);

	if (e < 0)
	  {
	    if (errno != EINTR && errno != EAGAIN)
	      s->err_hook(s, errno);
	    return 0;
	  }
	s->rpos = s->rbuf + e;
	get_sockaddr(&sa, &s->faddr, NULL, &s->fport, 1);
	sysio_process_rx_cmsgs(s, &msg);

	s->rx_hook(s, e);
	return 1;
      }
    }
}

static int
sk_write(sock *s)
{
  switch (s->type)
    {
    case SK_TCP_ACTIVE:
      {
	sockaddr sa;
	fill_in_sockaddr(&sa, s->daddr, s->iface, s->dport);
	if (connect(s->fd, (struct sockaddr *) &sa, sizeof(sa)) >= 0 || errno == EISCONN)
	  sk_tcp_connected(s);
	else if (errno != EINTR && errno != EAGAIN && errno != EINPROGRESS)
	  s->err_hook(s, errno);
	return 0;
      }
    default:
      if (s->ttx != s->tpos && sk_maybe_write(s) > 0)
	{
	  s->tx_hook(s);
	  return 1;
	}
      return 0;
    }
}

void
sk_dump_all(void)
{
  node *n;
  sock *s;

  debug("Open sockets:\n");
  WALK_LIST(n, sock_list)
    {
      s = SKIP_BACK(sock, n, n);
      debug("%p ", s);
      sk_dump(&s->r);
    }
  debug("\n");
}

#undef ERR
#undef WARN

/*
 *	Main I/O Loop
 */

volatile int async_config_flag;		/* Asynchronous reconfiguration/dump scheduled */
volatile int async_dump_flag;

void
io_init(void)
{
  init_list(&near_timers);
  init_list(&far_timers);
  init_list(&sock_list);
  init_list(&global_event_list);
  krt_io_init();
  init_times();
  update_times();
  boot_time = now;
  srandom((int) now_real);
}

static int short_loops = 0;
#define SHORT_LOOP_MAX 10

void
io_loop(void)
{
  fd_set rd, wr;
  struct timeval timo;
  time_t tout;
  int hi, events;
  sock *s;
  node *n;

  sock_recalc_fdsets_p = 1;
  for(;;)
    {
      events = ev_run_list(&global_event_list);
      update_times();
      tout = tm_first_shot();
      if (tout <= now)
	{
	  tm_shot();
	  continue;
	}
      timo.tv_sec = events ? 0 : MIN(tout - now, 3);
      timo.tv_usec = 0;

      if (sock_recalc_fdsets_p)
	{
	  sock_recalc_fdsets_p = 0;
	  FD_ZERO(&rd);
	  FD_ZERO(&wr);
	}

      hi = 0;
      WALK_LIST(n, sock_list)
	{
	  s = SKIP_BACK(sock, n, n);
	  if (s->rx_hook)
	    {
	      FD_SET(s->fd, &rd);
	      if (s->fd > hi)
		hi = s->fd;
	    }
	  else
	    FD_CLR(s->fd, &rd);
	  if (s->tx_hook && s->ttx != s->tpos)
	    {
	      FD_SET(s->fd, &wr);
	      if (s->fd > hi)
		hi = s->fd;
	    }
	  else
	    FD_CLR(s->fd, &wr);
	}

      /*
       * Yes, this is racy. But even if the signal comes before this test
       * and entering select(), it gets caught on the next timer tick.
       */

      if (async_config_flag)
	{
	  async_config();
	  async_config_flag = 0;
	  continue;
	}
      if (async_dump_flag)
	{
	  async_dump();
	  async_dump_flag = 0;
	  continue;
	}
      if (async_shutdown_flag)
	{
	  async_shutdown();
	  async_shutdown_flag = 0;
	  continue;
	}

      /* And finally enter select() to find active sockets */
      hi = select(hi+1, &rd, &wr, NULL, &timo);

      if (hi < 0)
	{
	  if (errno == EINTR || errno == EAGAIN)
	    continue;
	  die("select: %m");
	}
      if (hi)
	{
	  /* guaranteed to be non-empty */
	  current_sock = SKIP_BACK(sock, n, HEAD(sock_list));

	  while (current_sock)
	    {
	      sock *s = current_sock;
	      int e;
	      int steps;

	      steps = MAX_STEPS;
	      if ((s->type >= SK_MAGIC) && FD_ISSET(s->fd, &rd) && s->rx_hook)
		do
		  {
		    steps--;
		    e = sk_read(s);
		    if (s != current_sock)
		      goto next;
		  }
		while (e && s->rx_hook && steps);

	      steps = MAX_STEPS;
	      if (FD_ISSET(s->fd, &wr))
		do
		  {
		    steps--;
		    e = sk_write(s);
		    if (s != current_sock)
		      goto next;
		  }
		while (e && steps);
	      current_sock = sk_next(s);
	    next: ;
	    }

	  short_loops++;
	  if (events && (short_loops < SHORT_LOOP_MAX))
	    continue;
	  short_loops = 0;

	  int count = 0;
	  current_sock = stored_sock;
	  if (current_sock == NULL)
	    current_sock = SKIP_BACK(sock, n, HEAD(sock_list));

	  while (current_sock && count < MAX_RX_STEPS)
	    {
	      sock *s = current_sock;
	      int e;

	      if ((s->type < SK_MAGIC) && FD_ISSET(s->fd, &rd) && s->rx_hook)
		{
		  count++;
		  e = sk_read(s);
		  if (s != current_sock)
		      goto next2;
		}
	      current_sock = sk_next(s);
	    next2: ;
	    }

	  stored_sock = current_sock;
	}
    }
}

void
test_old_bird(char *path)
{
  int fd;
  struct sockaddr_un sa;

  fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd < 0)
    die("Cannot create socket: %m");
  if (strlen(path) >= sizeof(sa.sun_path))
    die("Socket path too long");
  bzero(&sa, sizeof(sa));
  sa.sun_family = AF_UNIX;
  strcpy(sa.sun_path, path);
  if (connect(fd, (struct sockaddr *) &sa, SUN_LEN(&sa)) == 0)
    die("I found another BIRD running.");
  close(fd);
}


