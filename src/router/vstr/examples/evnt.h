#ifndef EVENT_H
#define EVENT_H

#include "vlg.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>

#include <timer_q.h>

#ifdef __linux__
# include <sys/prctl.h>
#endif

#ifdef PR_SET_PDEATHSIG
# define PROC_CNTL_PDEATHSIG(x1) prctl(PR_SET_PDEATHSIG, x1, 0, 0, 0)
#else
# define PROC_CNTL_PDEATHSIG(x1) (-1)
#endif

#ifdef __GNUC__
# define EVNT__ATTR_UNUSED(x) vstr__UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define EVNT__ATTR_UNUSED(x) /*@unused@*/ vstr__UNUSED_ ## x
#else
# define EVNT__ATTR_UNUSED(x) vstr__UNUSED_ ## x
#endif

#ifdef __GNUC__
# define EVNT__ATTR_USED() __attribute__((used))
#else
# define EVNT__ATTR_USED() 
#endif

#ifndef EVNT_COMPILE_INLINE
#define EVNT_COMPILE_INLINE 1
#endif

#ifndef EVNT_CONF_NAGLE
#define EVNT_CONF_NAGLE FALSE /* configurable */
#endif

#define EVNT_IO_OK 0
#define EVNT_IO_READ_EOF 1
#define EVNT_IO_READ_FIN 2
#define EVNT_IO_READ_ERR 3
#define EVNT_IO_SEND_ERR 4

struct Evnt;

struct Evnt_cbs
{
 struct Evnt *(*cb_func_accept)    (struct Evnt *,
                                    int, struct sockaddr *, socklen_t);
 int          (*cb_func_connect)   (struct Evnt *);
 int          (*cb_func_recv)      (struct Evnt *);
 int          (*cb_func_send)      (struct Evnt *);
 void         (*cb_func_free)      (struct Evnt *);
 int          (*cb_func_shutdown_r)(struct Evnt *);
};

struct Evnt
{
 struct Evnt *next;
 struct Evnt *prev;

 struct Evnt_cbs cbs[1];
 
 unsigned int ind; /* socket poll */

 Vstr_ref *sa_ref;
 
 Vstr_base *io_r;
 Vstr_base *io_w;

 struct Evnt *s_next;
 struct Evnt *c_next;
 
 Timer_q_node *tm_o;

 struct timeval ctime;
 struct timeval mtime;

 unsigned long msecs_tm_mtime;
 
 VSTR_AUTOCONF_uintmax_t prev_bytes_r;
 struct
 {
  unsigned int            req_put;
  unsigned int            req_got;
  VSTR_AUTOCONF_uintmax_t bytes_r;
  VSTR_AUTOCONF_uintmax_t bytes_w;
 } acct;
 
 unsigned int flag_q_accept    : 1;
 unsigned int flag_q_connect   : 1;
 unsigned int flag_q_recv      : 1;
 unsigned int flag_q_send_recv : 1;
 unsigned int flag_q_none      : 1;
 
 unsigned int flag_q_send_now  : 1;
 unsigned int flag_q_closed    : 1;

 unsigned int flag_q_pkt_move  : 1;

 unsigned int flag_io_nagle    : 1;
 unsigned int flag_io_cork     : 1;

 unsigned int flag_io_filter   : 1;

 unsigned int flag_fully_acpt  : 1;

 unsigned int io_r_shutdown    : 1;
 unsigned int io_w_shutdown    : 1;
};

#if 1 /* ndef VSTR_AUTOCONF_NDEBUG */
# define EVNT_SA(x)     ((struct sockaddr     *)(x)->sa_ref->ptr)
# define EVNT_SA_IN4(x) ((struct sockaddr_in  *)(x)->sa_ref->ptr)
# define EVNT_SA_IN6(x) ((struct sockaddr_in6 *)(x)->sa_ref->ptr)
# define EVNT_SA_UN(x)  ((struct sockaddr_un  *)(x)->sa_ref->ptr)
#else
static struct sockaddr *evnt___chk_sa(void *ptr)
{
  struct sockaddr *sa = ptr;
  if ((sa->sa_family != AF_INET) &&
      (sa->sa_family != AF_INET6) &&
      (sa->sa_family != AF_LOCAL))
    abort();
  return (sa);
}
static struct sockaddr *evnt___chk_in(void *ptr)
{
  struct sockaddr_in *sa = ptr;
  if (sa->sin_family != AF_INET)
    abort();
  return (sa);
}
static struct sockaddr *evnt___chk_in6(void *ptr)
{
  struct sockaddr_in6 *sa = ptr;
  if (sa->sin6_family != AF_INET6)
    abort();
  return (sa);
}
static struct sockaddr *evnt___chk_un(void *ptr)
{
  struct sockaddr_un *sa = ptr;
  if (sa->sun_family != AF_LOCAL)
    abort();
  return (sa);
}

# define EVNT_SA(x)     evnt___chk_sa((x)->sa_ref->ptr)
# define EVNT_SA_IN4(x) evnt___chk_in((x)->sa_ref->ptr)
# define EVNT_SA_IN6(x) evnt___chk_in6((x)->sa_ref->ptr)
# define EVNT_SA_UN(x)  evnt___chk_un((x)->sa_ref->ptr)
#endif

/* FIXME: bad namespace */
typedef struct Acpt_data
{
 struct Evnt *evnt;
 Vstr_ref *sa;
} Acpt_data;
#if 1 /* ndef VSTR_AUTOCONF_NDEBUG */
# define ACPT_SA(x)     ((struct sockaddr     *)(x)->sa->ptr)
# define ACPT_SA_IN4(x) ((struct sockaddr_in  *)(x)->sa->ptr)
# define ACPT_SA_IN6(x) ((struct sockaddr_in6 *)(x)->sa->ptr)
# define ACPT_SA_UN(x)  ((struct sockaddr_un  *)(x)->sa->ptr)
#else
# define ACPT_SA(x)     evnt___chk_sa((x)->sa->ptr)
# define ACPT_SA_IN4(x) evnt___chk_in((x)->sa->ptr)
# define ACPT_SA_IN6(x) evnt___chk_in6((x)->sa->ptr)
# define ACPT_SA_UN(x)  evnt___chk_un((x)->sa->ptr)
#endif

typedef struct Acpt_listener
{
 struct Evnt evnt[1];
 unsigned int max_connections;
 Vstr_ref *ref;
} Acpt_listener;

extern volatile sig_atomic_t evnt_child_exited;

extern int evnt_opt_nagle;

extern void evnt_logger(Vlg *);

extern void evnt_fd__set_nonblock(int, int);

extern int evnt_fd(struct Evnt *);

extern void evnt_wait_cntl_add(struct Evnt *, int);
extern void evnt_wait_cntl_del(struct Evnt *, int);

extern int evnt_cb_func_connect(struct Evnt *);
extern struct Evnt *evnt_cb_func_accept(struct Evnt *,
                                        int, struct sockaddr *, socklen_t);
extern int evnt_cb_func_recv(struct Evnt *);
extern int evnt_cb_func_send(struct Evnt *);
extern void evnt_cb_func_free(struct Evnt *);
extern void evnt_cb_func_F(struct Evnt *);
extern int evnt_cb_func_shutdown_r(struct Evnt *);

extern int evnt_make_con_ipv4(struct Evnt *, const char *, short);
extern int evnt_make_con_local(struct Evnt *, const char *);
extern int evnt_make_bind_ipv4(struct Evnt *, const char *, short,unsigned int);
extern int evnt_make_bind_local(struct Evnt *, const char *, unsigned int);
extern int evnt_make_acpt_ref(struct Evnt *, int, Vstr_ref *);
extern int evnt_make_acpt_dup(struct Evnt *, int, struct sockaddr *, socklen_t);
extern int evnt_make_custom(struct Evnt *, int, Vstr_ref *, int);

extern void evnt_free(struct Evnt *);
extern void evnt_close(struct Evnt *);
extern void evnt_close_all(void);

extern void evnt_add(struct Evnt **, struct Evnt *);
extern void evnt_del(struct Evnt **, struct Evnt *);
extern void evnt_put_pkt(struct Evnt *);
extern void evnt_got_pkt(struct Evnt *);
extern int evnt_shutdown_r(struct Evnt *, int);
extern int evnt_shutdown_w(struct Evnt *);
extern int evnt_recv(struct Evnt *, unsigned int *);
extern int evnt_send(struct Evnt *);
extern int evnt_sendfile(struct Evnt *, int,
                         VSTR_AUTOCONF_uintmax_t *, VSTR_AUTOCONF_uintmax_t *,
                         unsigned int *);
extern int evnt_sc_read_send(struct Evnt *, int, VSTR_AUTOCONF_uintmax_t *);
extern int  evnt_send_add(struct Evnt *, int, size_t);
extern void evnt_send_del(struct Evnt *);
extern void evnt_scan_fds(unsigned int, size_t);
extern void evnt_scan_send_fds(void);
extern void evnt_scan_q_close(void);
extern void evnt_acpt_close_all(void);


extern void evnt_stats_add(struct Evnt *, const struct Evnt *);

extern unsigned int evnt_num_all(void);
extern int evnt_waiting(void);

extern void evnt_fd_set_cork(struct Evnt *, int);
extern void evnt_fd_set_defer_accept(struct Evnt *, int);
extern int  evnt_fd_set_filter(struct Evnt *, const char *);

extern void evnt_timeout_init(void);
extern void evnt_timeout_exit(void);

extern pid_t evnt_make_child(void);
extern int evnt_is_child(void);
extern int evnt_child_block_beg(void);
extern int evnt_child_block_end(void);

extern int evnt_sc_timeout_via_mtime(struct Evnt *, unsigned long);
extern void evnt_sc_main_loop(size_t);

extern time_t evnt_sc_time(void);

extern void evnt_sc_serv_cb_func_acpt_free(struct Evnt *);
extern struct Evnt *evnt_sc_serv_make_bind(const char *, unsigned short,
                                           unsigned int, unsigned int,
                                           unsigned int, const char *);

extern void evnt_vlg_stats_info(struct Evnt *, const char *);

extern int evnt_poll_init(void);
extern int evnt_poll_direct_enabled(void);
extern int evnt_poll_child_init(void);

extern unsigned int evnt_poll_add(struct Evnt *, int);
extern void evnt_poll_del(struct Evnt *);
extern int evnt_poll_swap_accept_read(struct Evnt *, int);
extern int evnt_poll(void);

extern struct Evnt *evnt_find_least_used(void);

extern struct Evnt *evnt_queue(const char *);

extern void evnt_out_dbg3(const char *);

#endif
