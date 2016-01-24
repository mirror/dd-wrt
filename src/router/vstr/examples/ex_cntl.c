

#define EX_UTILS_NO_USE_INPUT 1
#define EX_UTILS_NO_USE_IO_FD 1
#define EX_UTILS_NO_USE_OPEN  1
#define EX_UTILS_NO_USE_INIT  1
#define EX_UTILS_NO_USE_EXIT  1
#include "ex_utils.h"

#include <stdlib.h>
#include <sys/socket.h>
#include <getopt.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>

#include <socket_poll.h>
#include <timer_q.h>

#define WAIT_SET_RECV_FLAG 1 /* work around ? */

#define CL_PACKET_MAX 0xFFFF
#define CL_MAX_CONNECT 128

/* made up ... 8bits */
#define CL_MSG_CLIENT_NONE INT_MAX
#define CL_MSG_CLIENT_ZERO (INT_MAX - 1)
#define CL_MAX_WAIT_SEND 16


#include "app.h"
#include "evnt.h"

#define EX_UTILS_NO_FUNCS  1
#include "ex_utils.h"

#include "mk.h"

MALLOC_CHECK_DECL();

#include "opt.h"

struct con
{
 struct Evnt ev[1];
};

static int io_r_fd = STDIN_FILENO;
unsigned int io_ind_r = 0; /* socket poll */
static Vstr_base *io_r = NULL;
static int io_w_fd = STDOUT_FILENO;
unsigned int io_ind_w = 0; /* socket poll */
static Vstr_base *io_w = NULL;

static Timer_q_base *cl_timeout_base = NULL;
static Timer_q_base *cl_timer_connect_base = NULL;

static int server_clients_count = 0; /* current number of clients */

static int server_clients = 1;
static unsigned int server_timeout = (2 * 60); /* 2 minutes */

static const char *server_filename = NULL;

static Vlg *vlg = NULL;

static void ui_out(void)
{
  if (!io_ind_w)
    return;
  
  SOCKET_POLL_INDICATOR(io_ind_w)->events |=  POLLOUT;
}

static int cl_recv(struct Evnt *evnt)
{
  int ret = evnt_cb_func_recv(evnt);
  
  if (!ret)
    goto malloc_bad;

  while (evnt->io_r->len)
  {
    size_t pos = 0;
    size_t len = 0;
    size_t ns1 = 0;
    size_t vpos = 0;
    size_t vlen = 0;
    size_t nse2 = 0;
    int done = FALSE;
  
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
        len = 0;
        evnt_got_pkt(evnt);
        break;
      }
      
      if (done)
        app_cstr_buf(io_w, " ");
      
      app_vstr(io_w, evnt->io_r, vpos, vlen, VSTR_TYPE_ADD_DEF);
    
      if (!done)
        app_cstr_buf(io_w, ":");
    
      done = TRUE;

      len -= nse2; pos += nse2;
    }
    if (done)
      app_cstr_buf(io_w, "\n");

    ui_out();
  
    if (len)
      VLG_WARN_RET(FALSE, (vlg, "invalid entry\n"));

  /*
  if (io_w->conf->malloc_bad)
    goto malloc_bad;
  */
  
    vstr_del(evnt->io_r, 1, ns1);
  }
  
  return (TRUE);
  
 malloc_bad:
  evnt->io_r->conf->malloc_bad = FALSE;
  evnt->io_w->conf->malloc_bad = FALSE;
  return (FALSE);
}

#define UI_CMD(x)                                                       \
    else if (vstr_cmp_case_cstr_eq(io_r, 1, len, x "\n")) do            \
    {                                                                   \
      size_t ns1 = 0;                                                   \
      Vstr_base *out = con->io_w;                                       \
                                                                        \
      if (!(ns1 = vstr_add_netstr_beg(out, out->len)) ||                \
          !vstr_add_cstr_ptr(out, out->len, x) ||                       \
          !vstr_add_netstr_end(out, ns1, out->len) ||                   \
          !evnt_send_add(con, FALSE, 0))                                \
      {                                                                 \
        evnt_close(con);                                                \
        return;                                                         \
      }                                                                 \
      evnt_put_pkt(con);                                                \
    } while (FALSE)

static void cl_connect(void); /* fwd ref */
static void ui_parse(void)
{
  size_t len = 0;
  unsigned int count = 64;
  struct Evnt *con = NULL;

  vlg_dbg3(vlg, "ui_parse %zu\n", io_r->len);
  
  if (!io_r->len)
    return; /* don't start more connections for nothing */

  if (!(con = evnt_find_least_used()))
  {
    cl_connect();
    return;
  }
  
  while ((len = vstr_srch_chr_fwd(io_r, 1, io_r->len, '\n')) && --count)
  {
    size_t line_len = len;

    if (0) { /* not reached */ }
    UI_CMD("CLOSE");
    UI_CMD("DBG");
    UI_CMD("UNDBG");
    UI_CMD("LIST");
    UI_CMD("STATUS");

    vlg_dbg3(vlg, "bad input\n");
    /* ignore everything else */
    
    vstr_del(io_r, 1, line_len);
  }
  vlg_dbg3(vlg, "io_r left = %zu\n", io_r->len);
}
#undef UI_CMD

static int cl_cb_func_connect(struct Evnt *evnt)
{
  (void)evnt;
  
  vlg_dbg3(vlg, "connect\n");

  ui_parse();
  
  return (TRUE);
}

static int cl_cb_func_recv(struct Evnt *evnt)
{
  return (cl_recv(evnt));
}

static void cl_cb_func_free(struct Evnt *evnt)
{
  struct con *con = (struct con *)evnt;

  F(con);

  --server_clients_count;
}

static struct con *cl_make(const char *server_fname)
{
  struct con *ret = MK(sizeof(struct con));

  if (!ret)
    errno = ENOMEM, err(EXIT_FAILURE, "%s", __func__);
  if (!evnt_make_con_local(ret->ev, server_fname))
    err(EXIT_FAILURE, "%s", __func__);

  ret->ev->cbs->cb_func_connect = cl_cb_func_connect;
  ret->ev->cbs->cb_func_recv    = cl_cb_func_recv;
  ret->ev->cbs->cb_func_free    = cl_cb_func_free;
  
  ++server_clients_count;

  if (ret->ev->flag_q_none)
    cl_cb_func_connect(ret->ev);
  
  return (ret);
}

static void cl_connect(void)
{
  struct con *con = cl_make(server_filename);
  struct timeval tv;

  if (server_timeout)
  {
    gettimeofday(&tv, NULL);
    
    TIMER_Q_TIMEVAL_ADD_SECS(&tv, 0, rand() % server_timeout);
  
    if (!(con->ev->tm_o = timer_q_add_node(cl_timeout_base, con, &tv,
                                           TIMER_Q_FLAG_NODE_DEFAULT)))
      errno = ENOMEM, err(EXIT_FAILURE, "%s", __func__);
  }
}

static unsigned int cl_scan_io_fds(unsigned int ready)
{
  const int bad_poll_flags = (POLLERR | /* POLLHUP | */ POLLNVAL);

  vlg_dbg2(vlg, "BEG ready = %u\n", ready);
  if (io_ind_r &&
      SOCKET_POLL_INDICATOR(io_ind_r)->revents & bad_poll_flags)
  {
    --ready;
    
    close(SOCKET_POLL_INDICATOR(io_ind_r)->fd);
    vlg_dbg2(vlg, "ERROR-POLL-IO_R(%d):\n",
             SOCKET_POLL_INDICATOR(io_ind_r)->revents);
    socket_poll_del(io_ind_r);
    io_ind_r = 0;
  }
  if (SOCKET_POLL_INDICATOR(io_ind_w)->revents & bad_poll_flags)
  {
    --ready;
    
    close(SOCKET_POLL_INDICATOR(io_ind_w)->fd);
    vlg_dbg2(vlg, "ERROR-POLL-IO_W(%d):\n",
             SOCKET_POLL_INDICATOR(io_ind_w)->revents);
    socket_poll_del(io_ind_w);
    io_ind_w = 0;
  }
  if (io_ind_r && (SOCKET_POLL_INDICATOR(io_ind_r)->revents & POLLIN))
  {
    unsigned int ern;
    
    --ready;
    while (vstr_sc_read_iov_fd(io_r, io_r->len, io_r_fd, 4, 32, &ern))
    { /* do nothing */ }
    
    switch (ern)
    {
      case VSTR_TYPE_SC_READ_FD_ERR_EOF:
        close(SOCKET_POLL_INDICATOR(io_ind_r)->fd);        
        SOCKET_POLL_INDICATOR(io_ind_r)->fd = -1;        
        socket_poll_del(io_ind_r);
        io_ind_r = 0;
        errno = EAGAIN;
      case VSTR_TYPE_SC_READ_FD_ERR_READ_ERRNO:
        if (errno != EAGAIN)
          break;
      case VSTR_TYPE_SC_READ_FD_ERR_NONE:
        ui_parse();
      default:
        break;
    }
    vlg_dbg2(vlg, "READ UI\n");
  }
  else if (io_ind_w)
    ui_parse();
  
  if (io_ind_w && (SOCKET_POLL_INDICATOR(io_ind_w)->revents & POLLOUT))
  {
    unsigned int ern;
    
    --ready;
    
    while (io_w->len && vstr_sc_write_fd(io_w, 1, io_w->len, io_w_fd, &ern))
    { /* do nothing */ }
    
    if (!io_w->len)
      SOCKET_POLL_INDICATOR(io_ind_w)->events &= ~POLLOUT;
    vlg_dbg2(vlg, "WRITE UI\n");
  }

  return (ready);
}

static void usage(const char *program_name, int ret, const char *prefix)
{
  Vstr_base *out = vstr_make_base(NULL);

  if (!out)
    errno = ENOMEM, err(EXIT_FAILURE, "usage");

  vstr_add_fmt(out, 0, "%s\n"
          " Format: %s [-chmtwV] <server name>\n"
          " --help -h         - Print this message.\n"
          " --debug -d        - Enable debug info.\n"
          " --clients -c      - Number of connections to make.\n"
          " --nagle -n        - Enable/disable nagle TCP option.\n"
          " --timeout -t      - Timeout (usecs) between each message.\n"
          " --version -V      - Print the version string.\n",
          prefix, program_name);
  
  if (io_put_all(out, ret ? STDERR_FILENO : STDOUT_FILENO) == IO_FAIL)
    err(EXIT_FAILURE, "write");
  
  exit (ret);
}


static void cl_cmd_line(int argc, char *argv[])
{
  char optchar = 0;
  const char *program_name = NULL;
  struct option long_options[] =
  {
   {"help", no_argument, NULL, 'h'},
   {"clients", required_argument, NULL, 'c'},
   {"debug", required_argument, NULL, 'd'},
   {"execute", required_argument, NULL, 'e'},
   {"host", required_argument, NULL, 'H'},
   {"port", required_argument, NULL, 'P'},
   {"nagle", optional_argument, NULL, 'n'},
   {"timeout", required_argument, NULL, 't'},
   {"version", no_argument, NULL, 'V'},
   {NULL, 0, NULL, 0}
  };
  Vstr_base *out = vstr_make_base(NULL);

  if (!out)
    errno = ENOMEM, err(EXIT_FAILURE, "command line");
  
  program_name = opt_program_name(argv[0], "cntl");

  while ((optchar = getopt_long(argc, argv, "c:de:hH:nP:Rt:V",
                                long_options, NULL)) != -1)
  {
    switch (optchar)
    {
      case '?': usage(program_name, EXIT_FAILURE, "");
      case 'h': usage(program_name, EXIT_SUCCESS, "");
        
      case 'V':
          vstr_add_fmt(out, 0,"\
%s version 1.0.0, compiled on %s.\n\
Written by James Antill\n\
\n\
Uses Vstr string library.\n\
",
                       program_name, __DATE__);

        if (io_put_all(out, STDOUT_FILENO) == IO_FAIL)
          err(EXIT_FAILURE, "write");
        
        exit (EXIT_SUCCESS);

      case 'c': server_clients      = atoi(optarg); break;
      case 't': server_timeout      = atoi(optarg); break;

      case 'd': vlg_debug(vlg);                     break;
        
      case 'e':
        /* use cmd line instead of stdin */
        io_r_fd = -1;
        app_cstr_buf(io_r, optarg); app_cstr_buf(io_r, "\n");
        break;
        
      case 'n':
        if (!optarg)
        { evnt_opt_nagle = !evnt_opt_nagle; }
        else if (!strcasecmp("true", optarg))   evnt_opt_nagle = TRUE;
        else if (!strcasecmp("1", optarg))      evnt_opt_nagle = TRUE;
        else if (!strcasecmp("false", optarg))  evnt_opt_nagle = FALSE;
        else if (!strcasecmp("0", optarg))      evnt_opt_nagle = FALSE;
        break;

      default:
        abort();
    }
  }
  vstr_free_base(out); out = NULL;

  argc -= optind;
  argv += optind;

  if (argc != 1)
    usage(program_name, EXIT_FAILURE, "");

  server_filename = argv[0];
}

static void cl_timer_cli(int type, void *data)
{
  struct con *con = data;
  struct timeval tv;
  unsigned long diff = 0;
  
  if (!con)
    return;
  
  ASSERT(evnt_fd(con->ev) != -1);
  
  if (type == TIMER_Q_TYPE_CALL_RUN_ALL)
    return;

  con->ev->tm_o = NULL;

  if (type == TIMER_Q_TYPE_CALL_DEL)
    return;

  gettimeofday(&tv, NULL);
  diff = timer_q_timeval_udiff_secs(&tv, &con->ev->mtime);
  if (diff > server_timeout)
  {
    vlg_dbg2(vlg, "timeout = %p (%lu, %lu)\n",
             con, diff, (unsigned long)server_timeout);
    close(SOCKET_POLL_INDICATOR(con->ev->ind)->fd);
    return;
  }
  
  if (type == TIMER_Q_TYPE_CALL_RUN_ALL)
    return;
  
  TIMER_Q_TIMEVAL_ADD_SECS(&tv, (server_timeout - diff) + 1, 0);
  if (!(con->ev->tm_o = timer_q_add_node(cl_timeout_base, con, &tv,
                                         TIMER_Q_FLAG_NODE_DEFAULT)))
    errno = ENOMEM, err(EXIT_FAILURE, "%s", __func__);
}

static void cl_timer_con(int type, void *data)
{
  int count = 0;
  
  if (type == TIMER_Q_TYPE_CALL_DEL)
    return;
  
  while ((server_clients_count < server_clients) && (count < CL_MAX_CONNECT))
  {
    cl_connect();
    ++count;
  }
  
  if (type == TIMER_Q_TYPE_CALL_RUN_ALL)
    return;
  
  if (server_clients_count < server_clients)
  {
    struct timeval tv;
    
    gettimeofday(&tv, NULL);
    TIMER_Q_TIMEVAL_ADD_SECS(&tv, 1, 0);
    if (!timer_q_add_node(cl_timer_connect_base, NULL, &tv,
                          TIMER_Q_FLAG_NODE_DEFAULT))
      errno = ENOMEM, err(EXIT_FAILURE, "%s", __func__);
  }
}

static void cl_init(void)
{
  cl_timeout_base       = timer_q_add_base(cl_timer_cli,
                                           TIMER_Q_FLAG_BASE_DEFAULT);
  cl_timer_connect_base = timer_q_add_base(cl_timer_con,
                                           TIMER_Q_FLAG_BASE_DEFAULT);
  if (!cl_timeout_base)
    errno = ENOMEM, err(EXIT_FAILURE, "%s", __func__);
  if (!cl_timer_connect_base)
    errno = ENOMEM, err(EXIT_FAILURE, "%s", __func__);

  /* FIXME: massive hack 1.0.5 is broken */
  timer_q_cntl_base(cl_timeout_base,
                    TIMER_Q_CNTL_BASE_SET_FLAG_INSERT_FROM_END, FALSE);
  timer_q_cntl_base(cl_timer_connect_base,
                    TIMER_Q_CNTL_BASE_SET_FLAG_INSERT_FROM_END, FALSE);
  
  vlg_init();

  if (!(vlg = vlg_make()))
    errno = ENOMEM, err(EXIT_FAILURE, "init");

  evnt_logger(vlg);
}

static void cl_beg(void)
{
  int count = 0;

  vlg_dbg3(vlg, "cl_beg\n");
  
  if (io_r_fd != -1)
  {
    vlg_dbg3(vlg, "cl_beg io_r beg\n");
    evnt_fd__set_nonblock(io_r_fd,  TRUE);
    if (!(io_ind_r = socket_poll_add(io_r_fd)))
      errno = ENOMEM, err(EXIT_FAILURE, "%s", __func__);
    SOCKET_POLL_INDICATOR(io_ind_r)->events |= POLLIN;
  }
  
  evnt_fd__set_nonblock(io_w_fd, TRUE);
  if (!(io_ind_w = socket_poll_add(io_w_fd)))
    errno = ENOMEM, err(EXIT_FAILURE, "%s", __func__);

  while ((server_clients_count < server_clients) && (count < CL_MAX_CONNECT))
  {
    cl_connect();
    ++count;
  }
  
  if (server_clients_count < server_clients)
  {
    struct timeval tv;
    
    gettimeofday(&tv, NULL);
    TIMER_Q_TIMEVAL_ADD_SECS(&tv, 1, 0);
    if (!timer_q_add_node(cl_timer_connect_base, NULL, &tv,
                          TIMER_Q_FLAG_NODE_DEFAULT))
      errno = ENOMEM, err(EXIT_FAILURE, "%s", __func__);
  }
}

static void cl_signals(void)
{
  struct sigaction sa;
  
  if (sigemptyset(&sa.sa_mask) == -1)
    err(EXIT_FAILURE, "%s", __func__);
  
  /* don't use SA_RESTART ... */
  sa.sa_flags = 0;
  /* ignore it... we don't have a use for it */
  sa.sa_handler = SIG_IGN;
  
  if (sigaction(SIGPIPE, &sa, NULL) == -1)
    err(EXIT_FAILURE, "%s", __func__);
}

int main(int argc, char *argv[])
{
  if (!vstr_init())
    errno = ENOMEM, err(EXIT_FAILURE, "%s", __func__);

  vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$');
  vstr_sc_fmt_add_all(NULL);
  
  if (!(io_r = vstr_make_base(NULL))) /* used in cmd line */
    errno = ENOMEM, err(EXIT_FAILURE, "%s", __func__);
  if (!(io_w = vstr_make_base(NULL)))
    errno = ENOMEM, err(EXIT_FAILURE, "%s", __func__);
  
  if (!socket_poll_init(0, SOCKET_POLL_TYPE_MAP_DIRECT))
    errno = ENOMEM, err(EXIT_FAILURE, "%s", __func__);

  srand(getpid() ^ time(NULL)); /* doesn't need to be secure... just different
                                 * for different runs */
  
  cl_signals();
  
  cl_init();
  
  cl_cmd_line(argc, argv);

  cl_beg();
  
  while (io_ind_w && (io_w->len || evnt_waiting() || io_ind_r || io_r->len))
  {
    int ready = evnt_poll();
    struct timeval tv;
    
    if ((ready == -1) && (errno != EINTR))
      err(EXIT_FAILURE, "%s", __func__);
    if (ready == -1)
      continue;

    evnt_out_dbg3("1");
    ready = cl_scan_io_fds(ready);
    evnt_out_dbg3("2");
    evnt_scan_fds(ready, CL_MAX_WAIT_SEND);
    evnt_out_dbg3("3");
    evnt_scan_send_fds();
    evnt_out_dbg3("4");
    
    gettimeofday(&tv, NULL);
    timer_q_run_norm(&tv);

    evnt_out_dbg3("5");
    evnt_scan_send_fds();
    evnt_out_dbg3("6");
  }
  evnt_out_dbg3("E");

  vstr_free_base(io_r);
  vstr_free_base(io_w);

  timer_q_del_base(cl_timeout_base);
  timer_q_del_base(cl_timer_connect_base);
  
  evnt_close_all();
  
  vlg_free(vlg);
  
  vlg_exit();
  
  vstr_exit();
  
  MALLOC_CHECK_EMPTY();
  
  exit (EXIT_SUCCESS);
}
         
