
/* doesn't use event.c ... isn't a full ypservd yet, just for tracing */
#define _GNU_SOURCE 1
#define _LARGEFILE64_SOURCE 1

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <unistd.h>
#include <assert.h>
#define ASSERT assert
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <sys/time.h>
#include <time.h>

#include <socket_poll.h>
#include <vstr.h>

#define TRUE 1
#define FALSE 0

#define MY_MAX_PROTO_EVTS 4

static Vstr_base *rcpt = NULL;
static Vstr_base *rqst = NULL;
static Vstr_base *vuio_r = NULL;
static Vstr_base *vuio_w = NULL;

/* rfc1057 -> rfc1831
 *         -> rfc1832
 *         -> rfc1833
 */

#define RPC_PORT_PORTMAP 111

#define RPC_MSG_VERSION 2

#define RPC_IS_CALL  0
#define RPC_IS_REPLY 1

#define RPC_MSG_RET_ACCEPTED 0
#define RPC_MSG_RET_DENIED   1

#define RPC_MSG_RET_A_SUCCESS       0 /* RPC executed successfully       */
#define RPC_MSG_RET_A_PROG_UNAVAIL  1 /* remote hasn't exported program  */
#define RPC_MSG_RET_A_PROG_MISMATCH 2 /* remote can't support version #  */
#define RPC_MSG_RET_A_PROC_UNAVAIL  3 /* program can't support procedure */
#define RPC_MSG_RET_A_GARBAGE_ARGS  4 /* procedure can't decode params   */

#define RPC_MSG_RET_D_VERS_MISMATCH 0 /* RPC version number != 2          */
#define RPC_MSG_RET_D_AUTH_ERROR    1 /* remote can't authenticate caller */

#define RPC_AUTH_FLAVOUR_NULL  0
#define RPC_AUTH_FLAVOUR_SYS   1
#define RPC_AUTH_FLAVOUR_SHORT 2
#define RPC_AUTH_FLAVOUR_DES   3

#define RPC_AUTH_RET_A_OK 0 /* success                          */

#define RPC_AUTH_RET_D_BADCRED      1 /* bad credentials (seal broken) */
#define RPC_AUTH_RET_D_REJECTEDCRED 2 /* client must begin new session */
#define RPC_AUTH_RET_D_BADVERF      3 /* bad verifier (seal broken)    */
#define RPC_AUTH_RET_D_REJECTEDVERF 4 /* verifier expired or replayed  */
#define RPC_AUTH_RET_D_TOOWEAK      5 /* rejected for security reasons */
#define RPC_AUTH_RET_D_INVALIDRESP  6 /* bogus response verifier          */
#define RPC_AUTH_RET_D_FAILED       7 /* reason unknown                   */

#define RPC_PROG_PORTMAP 0x000186A0 /* 100000 */
#define RPC_PROG_NFS     0x000186A3
#define RPC_PROG_YPSERV  0x000186A4
#define RPC_PROG_MOUNTD  0x000186A5
#define RPC_PROG_YPBIND  0x000186A7

#define RPC_PROC_NULL 0 /* std. for all programs */

/* portmap */
#define RPC_PROG_PORTMAP_VERSION 2

#define RPC_PROG_PORTMAP_PROC_SET 1
#define RPC_PROG_PORTMAP_PROC_UNSET 2
#define RPC_PROG_PORTMAP_PROC_GETPORT 3
#define RPC_PROG_PORTMAP_PROC_DUMP 4
#define RPC_PROG_PORTMAP_PROC_CALLIT 5


/* ypserv */
#define RPC_PROG_YPSERV_VERSION 2

#define RPC_PROG_YPSERV_PROC_DOMAIN 1
#define RPC_PROG_YPSERV_PROC_DOMAIN_NONACK 2
#define RPC_PROG_YPSERV_PROC_MATCH 3
#define RPC_PROG_YPSERV_PROC_FIRST 4
#define RPC_PROG_YPSERV_PROC_NEXT 5
#define RPC_PROG_YPSERV_PROC_XFR 6
#define RPC_PROG_YPSERV_PROC_CLEAR 7
#define RPC_PROG_YPSERV_PROC_ALL 8
#define RPC_PROG_YPSERV_PROC_MASTER 9
#define RPC_PROG_YPSERV_PROC_ORDER 10
#define RPC_PROG_YPSERV_PROC_MAPLIST 11

/* ypbind */
#define RPC_PROG_YPBIND_VERSION 2

#define RPC_PROG_YPBIND_PROC_DOMAIN 1
#define RPC_PROG_YPBIND_PROC_SETDOM 2

static long my_rpc_xid = 0;

#define REPLY_EVNT_PORTMAP_UNSET 0
#define REPLY_EVNT_PORTMAP_SET1  1
#define REPLY_EVNT_PORTMAP_SET2  2
#define REPLY_EVNT_LOG           3 /* just log it */

static unsigned int udp_reply_evnt = REPLY_EVNT_PORTMAP_UNSET;

static short my_udp_port = 0;
static short my_tcp_port = 0;

unsigned int uio_r = 0;
unsigned int uio_w = 0;

unsigned int my_ignore_evnt = 0;

static long xdr_get_long(Vstr_base *s1, size_t *pos, size_t *len)
{
  long ret = 0;
  char buf[4] = {0};
  
  assert(s1 && pos && len);
  
  if (*len < 4) /* hard coded */
    return (0);

  ret = vstr_sc_parse_b_uint32(s1, *pos);
  
  *pos += 4;
  *len -= 4;

  return (ret);
}

static int xdr_put_long(Vstr_base *s1, size_t *pos, long val)
{
  int ret = FALSE;
  
  ASSERT(s1 && pos);

  if ((ret = vstr_sc_add_b_uint32(s1, *pos, val)))
    *pos += 4;
  
  return (ret);
}

static size_t xdr_get_str_buf(Vstr_base *s1, size_t *pos, size_t *len,
                              char *buf, size_t max_len)
{
  size_t pad_len = 0;
  unsigned long buf_len = 0;
  
  ASSERT(s1 && pos && len && buf);

  buf_len = xdr_get_long(s1, pos, len);
  
  if (!buf_len || (buf_len > *len))
    return (0);

  vstr_export_cstr_buf(s1, *pos, buf_len, buf, max_len);

  *pos += buf_len;
  *len -= buf_len;
  
  if ((pad_len = (buf_len % 4)))
  {
    pad_len = 4 - pad_len; /*
                            * 1 % 4 = 1 == pad 3 bytes
                            * 2 % 4 = 2 == pad 2 bytes
                            * 3 % 4 = 3 == pad 1 bytes
                            */
    if (pad_len > *len)
      pad_len = *len;
    
    *pos += pad_len;
    *len -= pad_len;
  }
  
  return ((buf_len >= max_len) ? (max_len - 1) : buf_len);
}

static int xdr_put_str_buf(Vstr_base *s1, size_t *pos,
                           const char *buf, size_t len, size_t max_len)
{
  int ret = FALSE;
  size_t pad_len = 0;

  ASSERT(s1 && pos && ((!buf && !len) || buf));
  
  if (len > max_len)
    len = max_len;
  
  if (!xdr_put_long(s1, pos, len))
    return (FALSE);
  
  if (!len)
    return (TRUE);
  
  ret = vstr_add_buf(s1, *pos, buf, len); /* works negative */
  if (ret) *pos += len;
  if (ret && (pad_len = (len % 4)))
  {
    pad_len = 4 - pad_len; /*
                            * 1 % 4 = 1 == pad 3 bytes
                            * 2 % 4 = 2 == pad 2 bytes
                            * 3 % 4 = 3 == pad 1 bytes
                            */
    
    ret = vstr_add_rep_chr(s1, *pos, 0, pad_len);
  }
  
  if (ret) *pos += pad_len;

  ASSERT(ret);
  
  return (ret);
}
#define XDR_PUT_CSTR_BUF(x, y, z, ML) xdr_put_str_buf(x, y, z, strlen(z), ML)

static int xdr_put_vstr(Vstr_base *s1, size_t *pos,
                        Vstr_base *s2, size_t s2_pos, size_t len,
                        size_t max_len)
{
  int ret = FALSE;
  size_t pad_len = 0;

  ASSERT(s1 && pos && ((!s2 && !len) || s2) && s2_pos);

  if (len > max_len)
    len = max_len;

  if (!xdr_put_long(s1, pos, len))
    return (FALSE);
  
  ret = vstr_add_vstr(s1, *pos, s2, s2_pos, len, 0);
  if (ret) *pos += len;
  if (ret && (pad_len = (len % 4)))
  {
    pad_len = 4 - pad_len; 
    ret = vstr_add_rep_chr(s1, *pos, 0, pad_len);
  }
  if (ret) *pos += pad_len;

  ASSERT(ret);
  
  return (ret);
}

static int xdr_put_list_long(Vstr_base *s1, size_t *pos,
                             const long *list, size_t len, size_t max_len)
{
  int ret = FALSE;
  size_t scan = 0;
  
  ASSERT(s1 && pos && ((!list && !len) || list));

  if (len > max_len)
    len = max_len;
  
  if (!xdr_put_long(s1, pos, len))
    return (FALSE);

  while (scan < len)
  {
    if (!(ret = xdr_put_long(s1, pos, list[scan])))
      return (ret);
    ++scan;
  }
  
  ASSERT(ret);
    
  return (ret);
}

static int xdr_x_put_auth_null(Vstr_base *s1, size_t *pos)
{
  int ret = FALSE;
  
  ret |= !xdr_put_long(s1, pos, RPC_AUTH_FLAVOUR_NULL);
  ret |= !xdr_put_long(s1, pos, 0);
  ret |= !xdr_put_long(s1, pos, RPC_AUTH_FLAVOUR_NULL);
  ret |= !xdr_put_long(s1, pos, 0);

  return (!ret);
}

static int xdr_x_put_auth_sys_root(Vstr_base *s1, size_t *pos)
    __attribute__((unused));
static int xdr_x_put_auth_sys_root(Vstr_base *s1, size_t *pos)
{ /* pretend we are root */
  int ret = FALSE;
  Vstr_base *tmp = vstr_make_base(NULL);
  size_t tmp_pos = 0;
  long extra_gids[7] = {0, 1, 2, 3, 4, 6, 10};
  
  ret |= !xdr_put_long(s1, pos, RPC_AUTH_FLAVOUR_SYS);

  /* BEG: tmp */
  ret |= !xdr_put_long(tmp, &tmp_pos, time(NULL) ^ getpid()); /* stamp */
  ret |= !XDR_PUT_CSTR_BUF(tmp, &tmp_pos, "code.and.org", 255);
  ret |= !xdr_put_long(tmp, &tmp_pos, 0); /* euid */
  ret |= !xdr_put_long(tmp, &tmp_pos, 0); /* egid */
  ret |= !xdr_put_list_long(tmp, &tmp_pos, extra_gids, 7, 16);
  /* END: tmp */

  /* auth_length + above */
  ret |= !xdr_put_vstr(s1, pos, tmp, 1, tmp->len, UINT_MAX);
  vstr_free_base(tmp);

  return (!ret);
}

static int xdr_x_put_portmap(Vstr_base *s1, size_t *pos, long proc)
{
  int ret = FALSE;
  
  ret |= !xdr_put_long(s1, pos, ++my_rpc_xid);
  ret |= !xdr_put_long(s1, pos, RPC_IS_CALL);
  ret |= !xdr_put_long(s1, pos, RPC_MSG_VERSION);
  ret |= !xdr_put_long(s1, pos, RPC_PROG_PORTMAP);
  ret |= !xdr_put_long(s1, pos, RPC_PROG_PORTMAP_VERSION);
  ret |= !xdr_put_long(s1, pos, proc);

  return (!ret);
}

static int q_sendto(int fd,
                    Vstr_base *s1, size_t pos, size_t len,
                    struct sockaddr_in *to)
{
  int ret = -1;
  
  if (!vstr_export_cstr_ptr(s1, pos, len))
    return (FALSE);
  
  ret = sendto(fd, vstr_export_cstr_ptr(s1, pos, len), len, 0,
               (struct sockaddr *)to, sizeof(*to));
  if (ret == -1)
    return (FALSE);
  
  return (TRUE);
}

static int udp_bind(const char *name, int port)
{
  struct protoent *the_protocol = getprotobyname("udp");
  struct sockaddr_in socket_bind_in;
  int fd = -1;
  int dummy = 1;
  
  memset(&socket_bind_in, 0, sizeof(struct sockaddr_in));

  socket_bind_in.sin_family = AF_INET;
  {
    struct hostent *tmp = gethostbyname(name);

    if (!tmp)
      exit (EXIT_FAILURE);
    socket_bind_in.sin_addr = *(struct in_addr *) tmp->h_addr_list[0];
  }

  socket_bind_in.sin_port = htons(port);

  if ((fd = socket(PF_INET, SOCK_DGRAM, the_protocol->p_proto)) == -1)
    exit (EXIT_FAILURE);
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &dummy, sizeof(dummy)) == -1)
    exit (EXIT_FAILURE);
  if (fcntl(fd, F_SETFD, 1) == -1)
    exit (EXIT_FAILURE);
  if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
    exit (EXIT_FAILURE);
  
  if (bind(fd, (struct sockaddr *) &socket_bind_in,
           sizeof(struct sockaddr_in)) == -1)
    exit (EXIT_FAILURE);

  return (fd);
}

static int tcp_bind(const char *name, int port)
{
  struct protoent *the_protocol = getprotobyname("ip"); /* generic */
  struct sockaddr_in socket_bind_in;
  int fd = -1;
  int dummy = 1;

  if (!the_protocol)
    exit (EXIT_FAILURE);
  
  memset(&socket_bind_in, 0, sizeof(struct sockaddr_in));

  socket_bind_in.sin_family = AF_INET;
  {
    struct hostent *tmp = gethostbyname(name);

    if (!tmp)
      exit (EXIT_FAILURE);
    socket_bind_in.sin_addr = *(struct in_addr *) tmp->h_addr_list[0];
  }

  socket_bind_in.sin_port = htons(port);

  if ((fd = socket(PF_INET, SOCK_STREAM, the_protocol->p_proto)) == -1)
    exit (EXIT_FAILURE);
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &dummy, sizeof(dummy)) == -1)
    exit (EXIT_FAILURE);
  if (fcntl(fd, F_SETFD, 1) == -1)
    exit (EXIT_FAILURE);
  if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
    exit (EXIT_FAILURE);
  
  if (bind(fd, (struct sockaddr *) &socket_bind_in,
           sizeof(struct sockaddr_in)) == -1)
    exit (EXIT_FAILURE);

  if (listen(fd, 64) == -1)
    exit (EXIT_FAILURE);

  return (fd);
}

static int rpc_call_unset_ypserv(int fd, struct sockaddr_in *to)
{
  size_t rqst_pos = 0;

  vstr_del(rqst, 1, rqst->len);
  
  xdr_x_put_portmap(rqst, &rqst_pos, RPC_PROG_PORTMAP_PROC_UNSET);
  xdr_x_put_auth_null(rqst, &rqst_pos);
  xdr_put_long(rqst, &rqst_pos, RPC_PROG_YPSERV);
  xdr_put_long(rqst, &rqst_pos, RPC_PROG_YPSERV_VERSION);
  xdr_put_long(rqst, &rqst_pos, 0);
  xdr_put_long(rqst, &rqst_pos, 0);

  if (!q_sendto(fd, rqst, 1, rqst->len, to))
    return (FALSE);

  vstr_del(rqst, 1, rqst->len);
  
  return (TRUE);
}

static int rpc_call_set_ypserv(int fd, struct sockaddr_in *to,
                               long ypserv_proto, long ypserv_port)
{
  size_t rqst_pos = 0;

  vstr_del(rqst, 1, rqst->len);
  
  xdr_x_put_portmap(rqst, &rqst_pos, RPC_PROG_PORTMAP_PROC_SET);
  xdr_x_put_auth_null(rqst, &rqst_pos);
  xdr_put_long(rqst, &rqst_pos, RPC_PROG_YPSERV);
  xdr_put_long(rqst, &rqst_pos, RPC_PROG_YPSERV_VERSION);
  xdr_put_long(rqst, &rqst_pos, ypserv_proto);
  xdr_put_long(rqst, &rqst_pos, ypserv_port);

  if (!q_sendto(fd, rqst, 1, rqst->len, to))
    return (FALSE);

  vstr_del(rqst, 1, rqst->len);
  
  return (TRUE);
}

static unsigned int srch_evnt(unsigned int *p_evt, unsigned int max_evts,
                              unsigned int evnt)
{
  unsigned int ret = 0;

  while (ret < max_evts)
  {
    if (evnt == p_evt[ret])
      return (ret);
    ++ret;
  }

  ASSERT(FALSE);

  return (0xFFFFFFFF);
}

static void del_evnt(unsigned int *p_evt, unsigned int *max_evts,
                     unsigned int num)
{
  close(SOCKET_POLL_INDICATOR(p_evt[num])->fd);
  socket_poll_del(p_evt[num]);
  --*max_evts;
  
  if (num < *max_evts)
    memmove(p_evt + num, p_evt + num + 1,
            (*max_evts - num) * sizeof(unsigned int));
}

static void user_evts(unsigned int *passed_evts,
                      unsigned int *p_u_evts, unsigned int *max_u_evts,
                      unsigned int *p_t_evts, unsigned int *max_t_evts)
{
  unsigned int evts = *passed_evts;
  struct sockaddr_in to_portmap;

  to_portmap.sin_family = AF_INET;
  to_portmap.sin_port = htons(RPC_PORT_PORTMAP);
  to_portmap.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  if ((SOCKET_POLL_INDICATOR(uio_r)->revents &
       (POLLERR|POLLHUP|POLLNVAL)) ||
      (SOCKET_POLL_INDICATOR(uio_w)->revents &
       (POLLERR|POLLHUP|POLLNVAL)))
  {
    
    if (!rpc_call_unset_ypserv(SOCKET_POLL_INDICATOR(p_u_evts[0])->fd,
                               &to_portmap))
      return;

    while (*max_u_evts)
      del_evnt(p_u_evts, max_u_evts, *max_u_evts - 1);
    while (*max_t_evts)
      del_evnt(p_t_evts, max_t_evts, *max_t_evts - 1);

    evts = 0;
  }

  if (evts && (SOCKET_POLL_INDICATOR(uio_r)->revents & POLLIN))
  {
    size_t endln_pos = 0;
    
    --evts;
    
    vstr_add_fmt(vuio_w, vuio_w->len, "\t User io event\n");
    
    vstr_sc_read_iov_fd(vuio_r, vuio_r->len, SOCKET_POLL_INDICATOR(uio_r)->fd,
                        3, 32, NULL);

    if ((endln_pos = vstr_srch_chr_fwd(vuio_r, 1, vuio_r->len, '\n')))
    {
      size_t vs_len = endln_pos - 1;
      size_t endw_len = 0;
      size_t tmp = 0;

      tmp = VSTR_SPN_CSTR_CHRS_FWD(vuio_r, 1, vs_len, " \t");
      vstr_del(vuio_r, 1, tmp);
      vs_len -= tmp;

      tmp = VSTR_CSPN_CSTR_CHRS_FWD(vuio_r, 1, vs_len, " \t");
      endw_len = tmp;

      if (0)
      { ASSERT(FALSE); }
      else if (VSTR_CMP_CSTR_EQ(vuio_r, 1, endw_len, "addudp"))
      {
        int fd = -1;
        
        vstr_del(vuio_r, 1, endw_len);
        vs_len -= endw_len;
        
        tmp = VSTR_SPN_CSTR_CHRS_FWD(vuio_r, 1, vs_len, " \t");
        vstr_del(vuio_r, 1, tmp);
        vs_len -= tmp;
      
        fd = udp_bind(vstr_export_cstr_ptr(vuio_r, 1, vs_len), my_udp_port);
        p_u_evts[*max_u_evts] = socket_poll_add(fd);
        SOCKET_POLL_INDICATOR(p_u_evts[*max_u_evts])->events = POLLIN;
        ++*max_u_evts;
        
        vstr_add_fmt(vuio_w, vuio_w->len, "\t Added udp interface -- %s -- "
                     "now has evnt -- %u --\n",
                     vstr_export_cstr_ptr(vuio_r, 1, vs_len),
                     p_u_evts[*max_u_evts - 1]);
      }
      else if (VSTR_CMP_CSTR_EQ(vuio_r, 1, endw_len, "addtcp"))
      {
        int fd = -1;

        vstr_del(vuio_r, 1, endw_len);
        vs_len -= endw_len;
        
        tmp = VSTR_SPN_CSTR_CHRS_FWD(vuio_r, 1, vs_len, " \t");
        vstr_del(vuio_r, 1, tmp);
        vs_len -= tmp;

        fd = tcp_bind(vstr_export_cstr_ptr(vuio_r, 1, vs_len), my_tcp_port);
        p_t_evts[*max_t_evts] = socket_poll_add(fd);
        SOCKET_POLL_INDICATOR(p_t_evts[*max_t_evts])->events = POLLIN;
        ++*max_t_evts;
        
        vstr_add_fmt(vuio_w, vuio_w->len, "\t Added tcp interface -- %s -- "
                     "now has evnt -- %u --\n",
                     vstr_export_cstr_ptr(vuio_r, 1, vs_len),
                     p_t_evts[*max_t_evts - 1]);
      }
      else if (VSTR_CMP_CSTR_EQ(vuio_r, 1, endw_len, "deludp"))
      {
        unsigned int evnt_num = 0;
        unsigned int off_num = 0;

        vstr_del(vuio_r, 1, endw_len);
        vs_len -= endw_len;
        
        tmp = VSTR_SPN_CSTR_CHRS_FWD(vuio_r, 1, vs_len, " \t");
        vstr_del(vuio_r, 1, tmp);
        vs_len -= tmp;

        evnt_num = vstr_parse_uint(vuio_r, 1, vs_len, 10, NULL, NULL);
        
        if ((off_num = srch_evnt(p_u_evts, *max_u_evts, evnt_num)) != 0xFFFFFFFF)
        {
          vstr_add_fmt(vuio_w, vuio_w->len, "\t Deleted udp evnt -- %d --\n",
                       evnt_num);          
          del_evnt(p_u_evts, max_u_evts, off_num);
        }
        else
          vstr_add_fmt(vuio_w, vuio_w->len,
                       "\t **** Can't delete evnt -- %u -- "
                       "not a UDP event\n",
                       evnt_num);
      }
      else if (VSTR_CMP_CSTR_EQ(vuio_r, 1, endw_len, "deltcp"))
      {
        unsigned int evnt_num = 0;
        unsigned int off_num = 0;

        vstr_del(vuio_r, 1, endw_len);
        vs_len -= endw_len;
        
        tmp = VSTR_SPN_CSTR_CHRS_FWD(vuio_r, 1, vs_len, " \t");
        vstr_del(vuio_r, 1, tmp);
        vs_len -= tmp;

        evnt_num = vstr_parse_uint(vuio_r, 1, vs_len, 10, NULL, NULL);
        
        if ((off_num = srch_evnt(p_t_evts, *max_t_evts, evnt_num)) != 0xFFFFFFFF)
        {
          vstr_add_fmt(vuio_w, vuio_w->len, "\t Deleted tcp evnt -- %d --\n",
                       evnt_num);
          del_evnt(p_t_evts, max_t_evts, off_num);
        }
        else
          vstr_add_fmt(vuio_w, vuio_w->len,
                       "\t **** Can't delete evnt -- %u -- "
                       "not a TCP event\n",
                       evnt_num);          
      }
      else if (VSTR_CMP_CSTR_EQ(vuio_r, 1, endw_len, "ignore"))
      {
        unsigned int evnt_num = 0;

        vstr_del(vuio_r, 1, endw_len);
        vs_len -= endw_len;
        
        tmp = VSTR_SPN_CSTR_CHRS_FWD(vuio_r, 1, vs_len, " \t");
        vstr_del(vuio_r, 1, tmp);
        vs_len -= tmp;

        evnt_num = vstr_parse_uint(vuio_r, 1, vs_len, 10, NULL, NULL);
        
        if (my_ignore_evnt)
          vstr_add_fmt(vuio_w, vuio_w->len,
                       "\t **** Can't ignore evnt -- %u -- "
                       "already ignoring evnt -- %u --\n",
                       evnt_num, my_ignore_evnt);
        else
        {
          vstr_add_fmt(vuio_w, vuio_w->len, "\t Ignoring evnt -- %d --\n",
                       evnt_num);
          my_ignore_evnt = evnt_num;
        }
        
      }
      else if (VSTR_CMP_CSTR_EQ(vuio_r, 1, endw_len, "unignore"))
      {
        unsigned int evnt_num = 0;

        vstr_del(vuio_r, 1, endw_len);
        vs_len -= endw_len;
        
        tmp = VSTR_SPN_CSTR_CHRS_FWD(vuio_r, 1, vs_len, " \t");
        vstr_del(vuio_r, 1, tmp);
        vs_len -= tmp;

        evnt_num = vstr_parse_uint(vuio_r, 1, vs_len, 10, NULL, NULL);
        
        if (!my_ignore_evnt)
          vstr_add_fmt(vuio_w, vuio_w->len,
                       "\t **** Can't unignore evnt -- %u -- "
                       "not ignoring any evnts.\n",
                       evnt_num);
        else if (my_ignore_evnt != evnt_num)
          vstr_add_fmt(vuio_w, vuio_w->len,
                       "\t **** Can't unignore evnt -- %u -- "
                       "ignoring evnt -- %u --\n",
                       evnt_num, my_ignore_evnt);
        else
        {
          vstr_add_fmt(vuio_w, vuio_w->len, "\t Unignoring evnt -- %d --\n",
                       evnt_num);
          my_ignore_evnt = 0;
        }
        
      }
      else if (VSTR_CMP_CSTR_EQ(vuio_r, 1, endw_len, "help"))
      {
        vstr_add_fmt(vuio_w, vuio_w->len, "%s", "\t List of all commands:\n");
        vstr_add_fmt(vuio_w, vuio_w->len, "%s", "\t\t help\n");
        vstr_add_fmt(vuio_w, vuio_w->len, "%s", "\t\t list\n");
        vstr_add_fmt(vuio_w, vuio_w->len, "%s", "\t\t ignore   <event>\n");
        vstr_add_fmt(vuio_w, vuio_w->len, "%s", "\t\t unignore <event>\n");
        vstr_add_fmt(vuio_w, vuio_w->len, "%s", "\t\t deludp   <udpevent>\n");
        vstr_add_fmt(vuio_w, vuio_w->len, "%s", "\t\t deltcp   <tcpevent>\n");
        vstr_add_fmt(vuio_w, vuio_w->len, "%s", "\t\t addudp   <interface>\n");
        vstr_add_fmt(vuio_w, vuio_w->len, "%s", "\t\t addtcp   <interface>\n");
        vstr_add_fmt(vuio_w, vuio_w->len, "%s", "\t\t quit\n");
      }
      else if (VSTR_CMP_CSTR_EQ(vuio_r, 1, endw_len, "list"))
      {
        unsigned int scan = 0;

        vstr_add_fmt(vuio_w, vuio_w->len, "%s", "\t List of all evnts:\n");

        vstr_add_fmt(vuio_w, vuio_w->len, "\t\t %s%*s %4u\n",
                     uio_r == my_ignore_evnt ? "*" : " ",
                     -16, "USER_IO_R", uio_r);
        vstr_add_fmt(vuio_w, vuio_w->len, "\t\t %s%*s %4u\n",
                     uio_w == my_ignore_evnt ? "*" : " ",
                     -16, "USER_IO_W", uio_w);

        scan = 0;
        while (scan < *max_u_evts)
        {
          struct sockaddr_in sa[1];
          socklen_t socklen = sizeof(sa);
          
          getpeername(SOCKET_POLL_INDICATOR(p_u_evts[scan])->fd, sa, &socklen);
          vstr_add_fmt(vuio_w, vuio_w->len, "\t\t %s%*s %4u ${ipv4.p:%p}:%hu\n",
                       p_u_evts[scan] == my_ignore_evnt ? "*" : " ",
                       -16, "UDP", p_u_evts[scan],
                       &sa->sin_addr, ntohs(sa->sin_port));
          ++scan;
        }

        scan = 0;
        while (scan < *max_t_evts)
        {
          struct sockaddr_in sa[1];
          socklen_t socklen = sizeof(sa);

          getsockname(SOCKET_POLL_INDICATOR(p_t_evts[scan])->fd, sa, &socklen);
          vstr_add_fmt(vuio_w, vuio_w->len, "\t\t %s%*s %4u ${ipv4.p:%p}:%hu\n",
                       p_t_evts[scan] == my_ignore_evnt ? "*" : " ",
                       -16, "TCP", p_t_evts[scan],
                       &sa->sin_addr, ntohs(sa->sin_port));
          ++scan;
        }
      }
      else if (VSTR_CMP_CSTR_EQ(vuio_r, 1, endw_len, "quit"))
      {
        if (!rpc_call_unset_ypserv(SOCKET_POLL_INDICATOR(p_u_evts[0])->fd,
                                   &to_portmap))
          return;
        
        while (*max_u_evts)
          del_evnt(p_u_evts, max_u_evts, *max_u_evts - 1);
        while (*max_t_evts)
          del_evnt(p_t_evts, max_t_evts, *max_t_evts - 1);

        vstr_add_fmt(vuio_w, vuio_w->len, "\t QUIT\n");
        
        evts = 0;
      }
      
      vstr_del(vuio_r, 1, vs_len + 1);
    }
  }

  *passed_evts = evts;
}

static void loop_do_proto_evts(unsigned int *passed_evts,
                               unsigned int *p_evt, 
                               unsigned int *passed_max_evts, int tcp)
{
  unsigned int scan = 0;
  unsigned int max_evts = *passed_max_evts;
  unsigned int evts = *passed_evts;
  
    
  while (evts && (scan < max_evts))
  {
    if (SOCKET_POLL_INDICATOR(p_evt[scan])->revents &
        (POLLERR|POLLHUP|POLLNVAL))
    {
      --evts;
      vstr_add_fmt(vuio_w, vuio_w->len, "%s",
                   "\tevent = (POLLERR|POLLHUP|POLLNVAL)\n");

      del_evnt(p_evt, &max_evts, scan);
      continue;
    }        
    
    if (SOCKET_POLL_INDICATOR(p_evt[scan])->revents & POLLIN)
    {
      char buf[1024];
      struct sockaddr_in from;
      socklen_t len = sizeof(from);
      ssize_t ret = -1;
      int fd = SOCKET_POLL_INDICATOR(p_evt[scan])->fd;
      
      --evts;
      if (tcp)
        while ((fd = accept(fd, (struct sockaddr *)&from, &len)) != -1)
        {
          size_t data_len = 0;
          
          while ((ret = read(fd, buf, sizeof(buf))) != -1)
            data_len += ret;
          close(fd);
          
          vstr_add_fmt(vuio_w, vuio_w->len, "\tevent = POLLIN_tcp(%d), len = %zu\n",
                       p_evt[scan], (size_t)ret);
        }
      else
        while ((ret = recvfrom(fd, buf, sizeof(buf), 0,
                               (struct sockaddr *)&from, &len)) != -1)
        {
          vstr_add_fmt(vuio_w, vuio_w->len, "\tevent = POLLIN_udp(%d), len = %zu\n",
                       p_evt[scan], (size_t)ret);
          /* deal with RPC request ... */

          if (ret < 16) continue; /* min size */

          if (my_ignore_evnt == p_evt[scan])
            continue; /* just drop the request */
          
          assert(!rcpt->len);
          vstr_del(rcpt, 1, rcpt->len);
          vstr_add_ptr(rcpt, rcpt->len, buf, ret);

          {
            size_t rcpt_len = rcpt->len;
            size_t rcpt_pos = 1;
            size_t rqst_pos = 0;
            /* BEG: generic RPC header... */
            long xid = xdr_get_long(rcpt, &rcpt_pos, &rcpt_len);
            long direction = xdr_get_long(rcpt, &rcpt_pos, &rcpt_len);
            long rpcvers = 0;
            long prog = 0;
            long progvers = 0;
            long proc = 0;
            long auth_flavour = 0;
            unsigned long auth_length = 0;

            if (direction == RPC_IS_REPLY)
            {
              
              vstr_add_fmt(vuio_w, vuio_w->len, "\t got reply %ld ",
                           my_rpc_xid - xid);

              switch (udp_reply_evnt)
              {
                case REPLY_EVNT_PORTMAP_UNSET:
                  VSTR_ADD_CSTR_BUF(vuio_w, vuio_w->len, "PORTMAP_UNSET\n");
                  if (!rpc_call_set_ypserv(fd, &from, IPPROTO_UDP, my_udp_port))
                    abort();
                  if (!rpc_call_set_ypserv(fd, &from, IPPROTO_TCP, my_tcp_port))
                    abort();
                  udp_reply_evnt = REPLY_EVNT_PORTMAP_SET1;
                  break;

                case REPLY_EVNT_PORTMAP_SET1:
                  VSTR_ADD_CSTR_BUF(vuio_w, vuio_w->len, "PORTMAP_SET1\n");
                  udp_reply_evnt = REPLY_EVNT_PORTMAP_SET2;
                  break;
                case REPLY_EVNT_PORTMAP_SET2:
                  VSTR_ADD_CSTR_BUF(vuio_w, vuio_w->len, "PORTMAP_SET2\n");
                  udp_reply_evnt = REPLY_EVNT_LOG;
                  break;
                  
                case REPLY_EVNT_LOG:
                  VSTR_ADD_CSTR_BUF(vuio_w, vuio_w->len, "UNKNOWN\n");
                  break;

                default:
                  abort();
              }
              
              goto fin; /* reply from portmapper */
            }

            ASSERT(direction == RPC_IS_CALL);
            
            rpcvers = xdr_get_long(rcpt, &rcpt_pos, &rcpt_len);
            prog = xdr_get_long(rcpt, &rcpt_pos, &rcpt_len);
            progvers = xdr_get_long(rcpt, &rcpt_pos, &rcpt_len);
            proc = xdr_get_long(rcpt, &rcpt_pos, &rcpt_len);

            ASSERT(rpcvers == RPC_MSG_VERSION);
            ASSERT(prog == RPC_PROG_YPSERV);
            ASSERT(progvers == RPC_PROG_YPSERV_VERSION);

            auth_flavour = xdr_get_long(rcpt, &rcpt_pos, &rcpt_len);
            auth_length = xdr_get_long(rcpt, &rcpt_pos, &rcpt_len);
            vstr_add_fmt(vuio_w, vuio_w->len, "\t got AUTH cred %lu %zu\n",
                         auth_length, rcpt_len);
            if (auth_length > rcpt_len)
              auth_length = rcpt_len;
            rcpt_pos += auth_length;
            rcpt_len -= auth_length;
            
            auth_flavour = xdr_get_long(rcpt, &rcpt_pos, &rcpt_len);
            auth_length = xdr_get_long(rcpt, &rcpt_pos, &rcpt_len);
            vstr_add_fmt(vuio_w, vuio_w->len, "\t got AUTH %lu %zu\n",
                         auth_length, rcpt_len);
            if (auth_length > rcpt_len)
              auth_length = rcpt_len;
            rcpt_pos += auth_length;
            rcpt_len -= auth_length;
            /* END: generic RPC header... */

            rqst_pos = 0;
            switch (proc)
            {
              case RPC_PROG_YPSERV_PROC_DOMAIN:
              {
                char dom[128 + 1];
                size_t dom_len = 0;
                
                dom_len = xdr_get_str_buf(rcpt, &rcpt_pos, &rcpt_len,
                                          dom, sizeof(dom));

                vstr_add_fmt(vuio_w, vuio_w->len, "\t got procedure %s\n",
                             "DOMAIN");
                vstr_add_fmt(vuio_w, vuio_w->len, "\t\t dom = %zu:%s\n",
                             dom_len, dom);                
                vstr_add_fmt(vuio_w, vuio_w->len, "\t\t remain = %zu\n", rcpt_len);
                /* BEG: send RPC reply */
                xdr_put_long(rqst, &rqst_pos, xid);
                xdr_put_long(rqst, &rqst_pos, RPC_IS_REPLY);
                xdr_put_long(rqst, &rqst_pos, 0);
                xdr_put_long(rqst, &rqst_pos, 0);
                xdr_put_long(rqst, &rqst_pos, 0);
                xdr_put_long(rqst, &rqst_pos, 0);
                xdr_put_long(rqst, &rqst_pos, 1); /* YPBIND_SUCC_VAL */
                /* END: send RPC reply */
              }
              break;
                
              case RPC_PROG_YPSERV_PROC_DOMAIN_NONACK:
              {
                char dom[128 + 1];
                size_t dom_len = 0;
                
                dom_len = xdr_get_str_buf(rcpt, &rcpt_pos, &rcpt_len,
                                          dom, sizeof(dom));
                
                vstr_add_fmt(vuio_w, vuio_w->len, "\t got procedure %s\n",
                             "DOMAIN_NONACK");
                vstr_add_fmt(vuio_w, vuio_w->len, "\t\t dom = %zu:%s\n",
                             dom_len, dom);
                vstr_add_fmt(vuio_w, vuio_w->len, "\t\t remain = %zu\n", rcpt_len);
                
                /* BEG: send RPC reply */
                xdr_put_long(rqst, &rqst_pos, xid);
                xdr_put_long(rqst, &rqst_pos, RPC_IS_REPLY);
                xdr_put_long(rqst, &rqst_pos, 0);
                xdr_put_long(rqst, &rqst_pos, 0);
                xdr_put_long(rqst, &rqst_pos, 0);
                xdr_put_long(rqst, &rqst_pos, 0);
                xdr_put_long(rqst, &rqst_pos, 1);
                /* END: send RPC reply */
              }
              break;
                
              case RPC_PROG_YPSERV_PROC_MASTER:
              {
                char dom[128 + 1];
                size_t dom_len = 0;
                char serv[128 + 1];
                size_t serv_len = 0;

                dom_len = xdr_get_str_buf(rcpt, &rcpt_pos, &rcpt_len,
                                          dom, sizeof(dom));
                serv_len = xdr_get_str_buf(rcpt, &rcpt_pos, &rcpt_len,
                                           serv, sizeof(serv));
                
                vstr_add_fmt(vuio_w, vuio_w->len, "\t got procedure %s\n",
                             "MASTER");
                vstr_add_fmt(vuio_w, vuio_w->len, "\t\t dom = %zu:%s\n",
                             dom_len, dom);
                vstr_add_fmt(vuio_w, vuio_w->len, "\t\t serv = %zu:%s\n",
                             serv_len, serv);
                vstr_add_fmt(vuio_w, vuio_w->len, "\t\t remain = %zu\n", rcpt_len);
                
                /* BEG: send RPC reply */
                xdr_put_long(rqst, &rqst_pos, xid);
                xdr_put_long(rqst, &rqst_pos, RPC_IS_REPLY);
                xdr_put_long(rqst, &rqst_pos, 0);
                xdr_put_long(rqst, &rqst_pos, 0);
                xdr_put_long(rqst, &rqst_pos, 0);
                xdr_put_long(rqst, &rqst_pos, 0);
                xdr_put_long(rqst, &rqst_pos, -1);
                xdr_put_long(rqst, &rqst_pos, 0);
                /* END: send RPC reply */
              }
              break;

              case RPC_PROG_YPSERV_PROC_MAPLIST:
              {
                char dom[128 + 1];
                size_t dom_len = 0;
                
                dom_len = xdr_get_str_buf(rcpt, &rcpt_pos, &rcpt_len,
                                          dom, sizeof(dom));

                vstr_add_fmt(vuio_w, vuio_w->len, "\t got procedure %s\n",
                             "MAPLIST");
                vstr_add_fmt(vuio_w, vuio_w->len, "\t\t dom = %zu:%s\n",
                             dom_len, dom);                
                vstr_add_fmt(vuio_w, vuio_w->len, "\t\t remain = %zu\n",
                             rcpt_len);
              }
              break;
              
              default:
                vstr_add_fmt(vuio_w, vuio_w->len, "\t got UNKNOWN procedure %ld\n",
                             proc);
                vstr_add_fmt(vuio_w, vuio_w->len, "\t\t remain = %zu\n", rcpt_len);
                break;
            }
            
            vstr_del(rcpt, 1, rcpt->len);
            
            if (!q_sendto(fd, rqst, 1, rqst->len, &from))
              goto fin;
          }
         fin:
          vstr_del(rcpt, 1, rcpt->len);
          vstr_del(rqst, 1, rqst->len);
        }
    }
    
    ++scan;
  }

  *passed_max_evts = max_evts;
  *passed_evts = evts;
}

static int rpc_setup(char *argv[],
                     unsigned int p_u_evt[static 4],
                     unsigned int p_t_evt[static 4])
{
  struct sockaddr_in to;
  
  my_rpc_xid = getpid() ^ time(NULL);

  p_u_evt[0] = socket_poll_add(udp_bind(argv[1], my_udp_port = atoi(argv[2])));
  SOCKET_POLL_INDICATOR(p_u_evt[0])->events = POLLIN;
  
  p_t_evt[0] = socket_poll_add(tcp_bind(argv[1], my_tcp_port = atoi(argv[3])));
  SOCKET_POLL_INDICATOR(p_t_evt[0])->events = POLLIN;
  
  to.sin_family = AF_INET;
  to.sin_port = htons(RPC_PORT_PORTMAP);
  to.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  if (!rpc_call_unset_ypserv(SOCKET_POLL_INDICATOR(p_u_evt[0])->fd, &to))
    goto fin;
  
  return (TRUE);

 fin:
  abort();

  return (FALSE);
}

int main(int argc, char *argv[])
{
  unsigned int p_t_evt[MY_MAX_PROTO_EVTS];
  unsigned int p_u_evt[MY_MAX_PROTO_EVTS];
  unsigned int max_t_evts = 0;
  unsigned int max_u_evts = 0;
  struct stat stat_buf;
  
  if (!socket_poll_init(4, SOCKET_POLL_TYPE_MAP_COMPRESSED))
    exit (EXIT_FAILURE);

  if (!vstr_init())
    exit (EXIT_FAILURE);

  if (fstat(1, &stat_buf) == -1)
    stat_buf.st_blksize = 4 * 1024;
  if (!stat_buf.st_blksize)
    stat_buf.st_blksize = 4 * 1024; /* defualt 4k -- proc etc. */

  if (!vstr_sc_fmt_add_all(NULL) ||
      !vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$') ||
      !vstr_cntl_conf(NULL,
                      VSTR_CNTL_CONF_SET_NUM_BUF_SZ, stat_buf.st_blksize) ||
      !vstr_make_spare_nodes(NULL, VSTR_TYPE_NODE_BUF, 32))
    exit (EXIT_FAILURE);
  
  if (!(rcpt = vstr_make_base(NULL)))
    exit (EXIT_FAILURE);
  
  if (!(rqst = vstr_make_base(NULL)))
    exit (EXIT_FAILURE);
  
  if (!(vuio_r = vstr_make_base(NULL)))
    exit (EXIT_FAILURE);
  if (!(vuio_w = vstr_make_base(NULL)))
    exit (EXIT_FAILURE);
  
  if (argc < 4)
  {
    vstr_add_fmt(vuio_w, vuio_w->len, " Format: ex_ypservd <host> <udpport> <tcpport>\n");
    goto boot_failed;
  }

  if (!(uio_r = socket_poll_add(0)))
  {
    vstr_add_fmt(vuio_w, vuio_w->len, "Can't speak to user via. stdin\n");
    goto boot_failed;
  }
  SOCKET_POLL_INDICATOR(uio_r)->events = POLLIN;
  if (!(uio_w = socket_poll_add(1)))
  {
    vstr_add_fmt(vuio_w, vuio_w->len, "Can't speak to user via. stdout\n");
    goto boot_failed;
  }
  
  if (!rpc_setup(argv, p_u_evt, p_t_evt))
    goto boot_failed;
  max_t_evts = 1;
  max_u_evts = 1;
  
  vstr_add_fmt(vuio_w, vuio_w->len, "STARTED\n");
  while (max_u_evts && max_t_evts)
  {
    int evts = socket_poll_update_all(-1);

    vstr_add_fmt(vuio_w, vuio_w->len, "events = %d\n", evts);

    user_evts(&evts, p_u_evt, &max_u_evts, p_t_evt, &max_t_evts);
    
    loop_do_proto_evts(&evts, p_u_evt, &max_u_evts, FALSE);
    loop_do_proto_evts(&evts, p_t_evt, &max_t_evts, TRUE);
    
    while (vuio_w->len)
      vstr_sc_write_fd(vuio_w, 1, vuio_w->len, 1, NULL);
  }
  vstr_add_fmt(vuio_w, vuio_w->len, "ENDED\n");

  while (vuio_w->len)
    vstr_sc_write_fd(vuio_w, 1, vuio_w->len, 1, NULL);
  
  exit (EXIT_SUCCESS);
  
 boot_failed:
  while (vuio_w->len)
    vstr_sc_write_fd(vuio_w, 1, vuio_w->len, 1, NULL);
  
  exit (EXIT_FAILURE);
}
