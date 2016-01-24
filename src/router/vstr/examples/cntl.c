
#include "cntl.h"
#include <unistd.h>

#include <poll.h>
#include <socket_poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#define EX_UTILS_NO_FUNCS  1
#include "ex_utils.h"

#include "mk.h"

#include "bag.h"

struct Cntl_child_obj
{
 struct Evnt evnt[1];
 pid_t pid;
};

struct Cntl_waiter_obj
{
 struct Evnt *evnt;
 unsigned int num;
};

static Vlg *vlg = NULL;
static struct Evnt *acpt_cntl_evnt = NULL;
static struct Evnt *acpt_pipe_evnt = NULL;

static Bag *childs  = NULL;
static Bag *waiters = NULL;

static unsigned int potential_waiters = 0;


static void cntl__fin(Vstr_base *out)
{
  size_t ns1 = 0;
  size_t ns2 = 0;

  if (!(ns1 = vstr_add_netstr_beg(out, out->len)))
    return;
  if (!(ns2 = vstr_add_netstr_beg(out, out->len)))
    return;
  vstr_add_netstr_end(out, ns2, out->len);
  vstr_add_netstr_end(out, ns1, out->len);
}

static struct Cntl_waiter_obj *cntl_waiter_get_first(void)
{
  Bag_iter iter[1];
  const Bag_obj *obj = bag_iter_beg(waiters, iter);

  while (obj)
  {
    struct Cntl_waiter_obj *val = obj->val;
      
    if (val->num)
      return (val);
    
    obj = bag_iter_nxt(iter);
  }

  return (NULL);
}

static int cntl_waiter_add(struct Evnt *evnt, size_t pos, size_t len, int *stop)
{
  Bag_iter iter[1];
  const Bag_obj *obj = NULL;
  struct Cntl_waiter_obj *val = NULL;
  Bag *tmp = NULL;
  
  ASSERT(stop);
  
  if (!childs)
  {
    cntl__fin(evnt->io_w);
    return (TRUE);
  }
  
  if (!(val = MK(sizeof(struct Cntl_waiter_obj))))
    return (FALSE);
  
  if (!(tmp = bag_add_obj(waiters, NULL, val)))
  {
    F(val);
    VLG_WARNNOMEM_RET(FALSE, (vlg, "%s: %m\n", "cntl waiters"));
  }
  
  waiters = tmp;
  
  val->evnt = evnt;
  val->num  = childs->num;
  
  obj = bag_iter_beg(childs, iter);
  while (obj)
  {
    struct Cntl_child_obj *child = (void *)obj->val;
    Vstr_base *out = NULL;

    if (!child)
      return (FALSE);
    
    out = child->evnt->io_w;
    
    if (!vstr_add_vstr(out, out->len, evnt->io_r, pos, len, VSTR_TYPE_ADD_DEF))
    {
      out->conf->malloc_bad = FALSE;
      return (FALSE);
    }
    
    if (!evnt_send_add(child->evnt, FALSE, 32))
    {
      evnt_close(child->evnt);
      return (FALSE);
    }
    
    evnt_put_pkt(child->evnt);
    obj = bag_iter_nxt(iter);
  }
  
  evnt_wait_cntl_del(evnt, POLLIN);
  *stop = TRUE;
  
  return (TRUE);
}

static void cntl_waiter_del(struct Evnt *child_evnt,
                            struct Cntl_waiter_obj *val)
{
  evnt_got_pkt(child_evnt);

  if (!--val->num)
  {
    if (val->evnt)
    {
      cntl__fin(val->evnt->io_w);
      if (!evnt_send_add(val->evnt, FALSE, 32))
        evnt_close(val->evnt);
      else
        evnt_wait_cntl_add(val->evnt, POLLIN);
    }
    
    if (!cntl_waiter_get_first()) /* no more left... */
      bag_del_all(waiters);
  }
}

static void cntl__ns_out_cstr_ptr(Vstr_base *out, const char *ptr)
{
  size_t ns = 0;

  if (!(ns = vstr_add_netstr_beg(out, out->len)))
    return;
  
  vstr_add_cstr_ptr(out, out->len, ptr);
  
  vstr_add_netstr_end(out, ns, out->len);
}

static void cntl__ns_out_fmt(Vstr_base *out, const char *fmt, ...)
   VSTR__COMPILE_ATTR_FMT(2, 3);
static void cntl__ns_out_fmt(Vstr_base *out, const char *fmt, ...)
{
  va_list ap;
  size_t ns = 0;

  if (!(ns = vstr_add_netstr_beg(out, out->len)))
    return;

  va_start(ap, fmt);
  vstr_add_vfmt(out, out->len, fmt, ap);
  va_end(ap);

  vstr_add_netstr_end(out, ns, out->len);
}

static void cntl__close(Vstr_base *out)
{
  struct Evnt *evnt = evnt_queue("accept");

  if (!evnt)
    return;
  
  while (evnt)
  {
    size_t ns1 = 0;
    
    if (!(ns1 = vstr_add_netstr_beg(out, out->len)))
      return;

    cntl__ns_out_cstr_ptr(out, "CLOSE");
    cntl__ns_out_fmt(out, "from[$<sa:%p>]", EVNT_SA(evnt));
    cntl__ns_out_fmt(out, "ctime[%lu:%lu]",
                     (unsigned long)evnt->ctime.tv_sec,
                     (unsigned long)evnt->ctime.tv_usec);
    cntl__ns_out_fmt(out, "pid[%lu]", (unsigned long)getpid());
    
    vstr_add_netstr_end(out, ns1, out->len);

    vlg_dbg2(vlg, "evnt_close acpt %p\n", evnt);
    evnt_close(evnt);

    evnt = evnt->next;
  }

  if (evnt_is_child())
    evnt_shutdown_r(acpt_cntl_evnt, FALSE);
}

static void cntl__scan_events(Vstr_base *out, const char *tag, struct Evnt *beg)
{
  struct Evnt *ev = beg;
  
  while (ev)
  {
    size_t ns = 0;

    if (!(ns = vstr_add_netstr_beg(out, out->len)))
      return;

    cntl__ns_out_fmt(out, "EVNT %s", tag);
    cntl__ns_out_fmt(out, "from[$<sa:%p>]", EVNT_SA(ev));
    cntl__ns_out_fmt(out, "ctime[%lu:%lu]",
                     (unsigned long)ev->ctime.tv_sec,
                     (unsigned long)ev->ctime.tv_usec);
    cntl__ns_out_fmt(out, "pid[%lu]", (unsigned long)getpid());
    cntl__ns_out_fmt(out, "req_got[%'u:%u]",
                     ev->acct.req_got, ev->acct.req_got);
    cntl__ns_out_fmt(out, "req_put[%'u:%u]",
                     ev->acct.req_put, ev->acct.req_put);
    cntl__ns_out_fmt(out, "recv[${BKMG.ju:%ju}:%ju]",
                     ev->acct.bytes_r, ev->acct.bytes_r);
    cntl__ns_out_fmt(out, "send[${BKMG.ju:%ju}:%ju]",
                     ev->acct.bytes_w, ev->acct.bytes_w);

    vstr_add_netstr_end(out, ns, out->len);
    
    ev = ev->next;
  }
}

static void cntl__status(Vstr_base *out)
{
  struct Evnt *evnt = evnt_queue("accept");
  
  while (evnt)
  {
    size_t ns1 = 0;
    
    if (!(ns1 = vstr_add_netstr_beg(out, out->len)))
      return;

    cntl__ns_out_cstr_ptr(out, "STATUS");
    cntl__ns_out_fmt(out, "from[$<sa:%p>]", EVNT_SA(evnt));
    cntl__ns_out_fmt(out, "ctime[%lu:%lu]",
                     (unsigned long)evnt->ctime.tv_sec,
                     (unsigned long)evnt->ctime.tv_usec);
  
    cntl__ns_out_fmt(out, "pid[%lu]", (unsigned long)getpid());
  
    vstr_add_netstr_end(out, ns1, out->len);
    
    evnt = evnt->next;
  }
}

static void cntl__dbg(Vstr_base *out)
{
  size_t ns1 = 0;
  
  if (!(ns1 = vstr_add_netstr_beg(out, out->len)))
    return;

  cntl__ns_out_cstr_ptr(out, "DBG");
  cntl__ns_out_fmt(out, "pid[%lu]", (unsigned long)getpid());
  cntl__ns_out_fmt(out, "dbg[%u]", (unsigned)vlg->out_dbg);
  vstr_add_netstr_end(out, ns1, out->len);
}

static int cntl__srch_waiter_evnt(const Bag_obj *obj, const void *data)
{
  const struct Cntl_waiter_obj *val = obj->val;
  const struct Evnt *evnt = data;

  return (val->evnt == evnt);
}

static void cntl__cb_func_free(struct Evnt *evnt)
{
  evnt_vlg_stats_info(evnt, "CNTL FREE");

  if (waiters)
  {
    const Bag_obj *obj = bag_srch_eq(waiters, cntl__srch_waiter_evnt, evnt);
    
    if (obj)
    {
      struct Cntl_waiter_obj *val = obj->val;

      ASSERT(val->evnt == evnt);
      val->evnt = NULL;
    }
  }
  
  F(evnt);
  
  ASSERT(potential_waiters >= 1);
  --potential_waiters;

  if (childs && !potential_waiters && !acpt_cntl_evnt)
    bag_del_all(childs);
}

static int cntl__cb_func_recv(struct Evnt *evnt)
{
  int ret = evnt_cb_func_recv(evnt);
  int stop = FALSE;
  
  if (!ret)
    goto malloc_bad;

  vlg_dbg2(vlg, "CNTL recv %zu\n", evnt->io_r->len);

  while (evnt->io_r->len && !stop)
  {
    size_t pos = 0;
    size_t len = 0;
    size_t ns1 = 0;
    
    if (!(ns1 = vstr_parse_netstr(evnt->io_r, 1, evnt->io_r->len, &pos, &len)))
    {
      if (!(SOCKET_POLL_INDICATOR(evnt->ind)->events & POLLIN))
        return (FALSE);
      return (TRUE);
    }

    evnt_got_pkt(evnt);

    if (0){ }
    else if (vstr_cmp_cstr_eq(evnt->io_r, pos, len, "CLOSE"))
    {
      cntl__close(evnt->io_w);
      
      if (!cntl_waiter_add(evnt, 1, ns1, &stop))
        goto malloc_bad;
    }
    else if (vstr_cmp_cstr_eq(evnt->io_r, pos, len, "DBG"))
    {
      vlg_debug(vlg);
      cntl__dbg(evnt->io_w);

      if (!cntl_waiter_add(evnt, 1, ns1, &stop))
        goto malloc_bad;
    }
    else if (vstr_cmp_cstr_eq(evnt->io_r, pos, len, "UNDBG"))
    {
      vlg_undbg(vlg);
      cntl__dbg(evnt->io_w);

      if (!cntl_waiter_add(evnt, 1, ns1, &stop))
        goto malloc_bad;
    }
    else if (vstr_cmp_cstr_eq(evnt->io_r, pos, len, "LIST"))
    {
      cntl__scan_events(evnt->io_w, "CONNECT",   evnt_queue("connect"));
      cntl__scan_events(evnt->io_w, "ACCEPT",    evnt_queue("accept"));
      cntl__scan_events(evnt->io_w, "SEND/RECV", evnt_queue("send_recv"));
      cntl__scan_events(evnt->io_w, "RECV",      evnt_queue("recv"));
      cntl__scan_events(evnt->io_w, "NONE",      evnt_queue("none"));
      cntl__scan_events(evnt->io_w, "SEND_NOW",  evnt_queue("send_now"));
      
      if (!cntl_waiter_add(evnt, 1, ns1, &stop))
        goto malloc_bad;
    }
    else if (vstr_cmp_cstr_eq(evnt->io_r, pos, len, "STATUS"))
    {
      cntl__status(evnt->io_w);

      if (!cntl_waiter_add(evnt, 1, ns1, &stop))
        goto malloc_bad;
    }
    else
      return (FALSE);
  
    if (evnt->io_w->conf->malloc_bad)
      goto malloc_bad;
    
    evnt_put_pkt(evnt);
    if (!evnt_send_add(evnt, FALSE, 32))
      return (FALSE);
  
    vstr_del(evnt->io_r, 1, ns1);
  }
  
  return (TRUE);
  
 malloc_bad:
  evnt->io_r->conf->malloc_bad = FALSE;
  evnt->io_w->conf->malloc_bad = FALSE;
  return (FALSE);
}


static void cntl__cb_func_acpt_free(struct Evnt *evnt)
{
  evnt_vlg_stats_info(evnt, "ACCEPT CNTL FREE");
  
  ASSERT(acpt_cntl_evnt == evnt);
  
  acpt_cntl_evnt = NULL;

  F(evnt);

  evnt_acpt_close_all();
  
  if (childs && !potential_waiters)
    bag_del_all(childs);
}

static struct Evnt *cntl__cb_func_accept(struct Evnt *from_evnt, int fd,
                                         struct sockaddr *sa, socklen_t len)
{
  struct Evnt *evnt = NULL;
  
  ASSERT(acpt_cntl_evnt);
  ASSERT(acpt_cntl_evnt == from_evnt);
  ASSERT(from_evnt->sa_ref);
  ASSERT(len >= 2);
  
  if (sa->sa_family != AF_LOCAL)
    goto make_acpt_fail;
  
  if (!(evnt = MK(sizeof(struct Evnt))))
    goto mk_acpt_fail;
  if (!evnt_make_acpt_ref(evnt, fd, from_evnt->sa_ref))
    goto make_acpt_fail;

  vlg_info(vlg, "CNTL CONNECT from[$<sa:%p>]\n", EVNT_SA(evnt));

  evnt->cbs->cb_func_recv = cntl__cb_func_recv;
  evnt->cbs->cb_func_free = cntl__cb_func_free;
  ++potential_waiters;
  
  return (evnt);

 make_acpt_fail:
  F(evnt);
  VLG_WARNNOMEM_RET(NULL, (vlg, "%s: %m\n", "accept"));
 mk_acpt_fail:
  VLG_WARN_RET(NULL, (vlg, "%s: %m\n", "accept"));
}

void cntl_make_file(Vlg *passed_vlg, const char *fname)
{
  struct Evnt *evnt = NULL;

  ASSERT(!vlg && passed_vlg);

  vlg = passed_vlg;
  
  ASSERT(fname);
  ASSERT(!acpt_cntl_evnt);
    
  if (!(evnt = MK(sizeof(struct Evnt))))
    VLG_ERRNOMEM((vlg, EXIT_FAILURE, "cntl file: %m\n"));
  
  if (!evnt_make_bind_local(evnt, fname, 8))
    vlg_err(vlg, EXIT_FAILURE, "cntl file: %m\n");
  
  evnt->cbs->cb_func_accept = cntl__cb_func_accept;
  evnt->cbs->cb_func_free   = cntl__cb_func_acpt_free;

  acpt_cntl_evnt = evnt;
}

static void cntl__cb_func_cntl_acpt_free(struct Evnt *evnt)
{
  evnt_vlg_stats_info(evnt, "CHILD CNTL FREE");
  
  ASSERT(acpt_cntl_evnt == evnt);
  
  acpt_cntl_evnt = NULL;

  F(evnt);
  
  evnt_acpt_close_all();
}

static void cntl__cb_func_pipe_acpt_free(struct Evnt *evnt)
{
  evnt_vlg_stats_info(evnt, "CHILD PIPE FREE");
  
  ASSERT(acpt_pipe_evnt == evnt);
  
  acpt_pipe_evnt = NULL;

  F(evnt);

  evnt_acpt_close_all();
}

/* used to get death sig or pass through cntl data */
static void cntl_pipe_acpt_fds(Vlg *passed_vlg, int fd, int allow_pdeathsig)
{
  ASSERT(fd != -1);

  if (acpt_cntl_evnt)
  {
    int old_fd = SOCKET_POLL_INDICATOR(acpt_cntl_evnt->ind)->fd;
    
    ASSERT(vlg       == passed_vlg);

    if (!evnt_poll_swap_accept_read(acpt_cntl_evnt, fd))
      vlg_abort(vlg, "%s: %m\n", "swap_acpt");

    close(old_fd);
    
    acpt_cntl_evnt->cbs->cb_func_recv = cntl__cb_func_recv;
    acpt_cntl_evnt->cbs->cb_func_free = cntl__cb_func_cntl_acpt_free;
    
    if (allow_pdeathsig)
      PROC_CNTL_PDEATHSIG(SIGCHLD);
  }
  else
  {
    if (!vlg)
      vlg = passed_vlg;
    ASSERT(vlg       == passed_vlg);

    if (allow_pdeathsig && (PROC_CNTL_PDEATHSIG(SIGCHLD) != -1))
    {
      close(fd);
      return;
    }
    
    if (!(acpt_pipe_evnt = MK(sizeof(struct Evnt))))
      VLG_ERRNOMEM((vlg, EXIT_FAILURE, "%s: %m\n", "pipe evnt"));
    
    if (!evnt_make_custom(acpt_pipe_evnt, fd, 0, 0))
      vlg_err(vlg, EXIT_FAILURE, "%s: %m\n", "pipe evnt");
    
    acpt_pipe_evnt->cbs->cb_func_free = cntl__cb_func_pipe_acpt_free;
  }
}

static void cntl__bag_cb_free_evnt(void *val)
{
  evnt_close(val);
}

static void cntl__bag_cb_free_F(void *val)
{
  F(val);
}

void cntl_child_make(unsigned int num)
{
  ASSERT(!childs && !waiters && acpt_cntl_evnt);
  
  if (!(childs  = bag_make(num, bag_cb_free_nothing, cntl__bag_cb_free_evnt)))
    VLG_ERRNOMEM((vlg, EXIT_FAILURE, "%s: %m\n", "cntl children"));
  if (!(waiters = bag_make(4,   bag_cb_free_nothing, cntl__bag_cb_free_F)))
    VLG_ERRNOMEM((vlg, EXIT_FAILURE, "%s: %m\n", "cntl children"));
}

void cntl_child_free(void)
{
  bag_free(childs);  childs  = NULL;
  bag_free(waiters); waiters = NULL;
}

static int cntl__cb_func_child_recv(struct Evnt *evnt)
{
  struct Cntl_waiter_obj *val = cntl_waiter_get_first();

  ASSERT(val);
  
  if (!evnt_cb_func_recv(evnt))
    goto malloc_bad;

  while (evnt->io_r->len)
  {
    size_t ns1  = 0;
    size_t pos  = 0;
    size_t len  = 0;
    size_t vpos = 0;
    size_t vlen = 0;
    size_t nse2 = 0;
    int done = FALSE;
    
    ASSERT(val);
  
    if (!(ns1 = vstr_parse_netstr(evnt->io_r, 1, evnt->io_r->len, &pos, &len)))
    {
      if (!(SOCKET_POLL_INDICATOR(evnt->ind)->events & POLLIN))
        return (FALSE);
      return (TRUE);
    }
    
    while ((nse2 = vstr_parse_netstr(evnt->io_r, pos, len, &vpos, &vlen)))
    {
      if (!done && !vlen && (nse2 == len))
      {
        cntl_waiter_del(evnt, val);
        vstr_del(evnt->io_r, 1, ns1);
        val = cntl_waiter_get_first();
        break;
      }
      
      done = TRUE;
      len -= nse2; pos += nse2;
    }
    if (nse2)
      continue;

    if (len)
      VLG_WARN_RET(FALSE, (vlg, "invalid entry\n"));

    if (!val->evnt)
      vstr_del(evnt->io_r, 1, ns1);
    else
    {
      struct Evnt *out = val->evnt;
      
      if (!vstr_mov(out->io_w, out->io_w->len, evnt->io_r, 1, ns1))
        goto malloc_bad;
    }
  }

  return (TRUE);
  
 malloc_bad:
  evnt_close(val->evnt);
  evnt->io_r->conf->malloc_bad = FALSE;
  evnt->io_w->conf->malloc_bad = FALSE;
  return (TRUE); /* this is "true" because the app. dies if we kill this con */
}

static void cntl__cb_func_child_free(struct Evnt *evnt)
{
  if (childs)
  {
    const Bag_obj *obj = bag_srch_eq(childs, bag_cb_srch_eq_val_ptr, evnt);
    
    if (obj) /* FIXME: const cast */
      ((Bag_obj *)obj)->val = NULL;
  }
  
  F(evnt);
}

void cntl_child_pid(pid_t pid, int fd)
{
  struct Cntl_child_obj *obj = NULL;

  ASSERT(acpt_cntl_evnt);
    
  if (!(obj = MK(sizeof(struct Cntl_child_obj))))
    VLG_ERRNOMEM((vlg, EXIT_FAILURE, "%s: %m\n", "cntl children"));

  obj->pid = pid;
  
  if (!evnt_make_custom(obj->evnt, fd, 0, POLLIN))
    vlg_err(vlg, EXIT_FAILURE, "%s: %m\n", "cntl children");

  obj->evnt->cbs->cb_func_free = cntl__cb_func_child_free;
  obj->evnt->cbs->cb_func_recv = cntl__cb_func_child_recv;
  
  if (!(childs = bag_add_obj(childs, NULL, obj)))
    VLG_ERRNOMEM((vlg, EXIT_FAILURE, "%s: %m\n", "cntl children"));
}

/* Here we fork multiple procs. however:
 *
 *  If we have a cntl connection we keep two way communication open with the
 * children.
 *
 *  If not we "leak" the writable side of the pipe() fd in the parent,
 * which will cause an error on all the children's fd when the parent dies.
 *
 *  The children also kill themselves if the parent fd has an error.
 */
void cntl_sc_multiproc(Vlg *passed_vlg, 
                       unsigned int num, int use_cntl, int allow_pdeathsig)
{
  int pfds[2] = {-1, -1};
  
  if (!vlg)
    vlg = passed_vlg;

  ASSERT(vlg == passed_vlg);
  
  vlg_pid_set(vlg, TRUE);
  
  if (!use_cntl && (pipe(pfds) == -1))
    vlg_err(vlg, EXIT_FAILURE, "pipe(): %m\n");

  if (use_cntl)
    cntl_child_make(num - 1);
  
  while (--num)
  {
    pid_t cpid = -1;
      
    if (use_cntl && (socketpair(PF_LOCAL, SOCK_STREAM, IPPROTO_IP, pfds) == -1))
      vlg_err(vlg, EXIT_FAILURE, "socketpair(): %m\n");

    if ((cpid = evnt_make_child()) == -1)
      vlg_err(vlg, EXIT_FAILURE, "fork(): %m\n");

    if (use_cntl && cpid) /* parent */
    {
      close(pfds[0]);
      cntl_child_pid(cpid, pfds[1]);
    }
    else if (!cpid)
    { /* child */
      close(pfds[1]);
      cntl_child_free();
      cntl_pipe_acpt_fds(vlg, pfds[0], allow_pdeathsig);
      
      evnt_scan_q_close();
      return;
    }
  }

  if (!use_cntl)
    close(pfds[0]); /* close child pipe() */
  
  evnt_scan_q_close();
}

