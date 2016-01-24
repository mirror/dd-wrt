/* dns stub resolver...
 * rfc1034 rfc1035 rfc1183 rfc1535 rfc1536 rfc1995 rfc1996 rfc2182
 * rfc2219 rfc2308 rfc2309 rfc2606 rfc2671 rfc2782
 * ipv6: rfc1886 rfc2874 rfc3363 rfc3364 rfc3596 */

#define VSTR_COMPILE_INCLUDE 1
#include <vstr.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <netdb.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>

#include <err.h>


#include <socket_poll.h>
#include <timer_q.h>

#define TRUE 1
#define FALSE 0


#define WAIT_SET_RECV_FLAG 1 /* work around ? */

#define CL_PACKET_MAX 0xFFFF
#define CL_MAX_CONNECT 128

/* made up ... 8bits */
#define CL_MSG_CLIENT_NONE INT_MAX
#define CL_MSG_CLIENT_ZERO (INT_MAX - 1)
#define CL_MAX_WAIT_SEND 16


#include "dns.h"
#include "app.h"

#define EX_UTILS_NO_FUNCS  1
#include "ex_utils.h"

#define CL_DNS_INIT(x, y) do {                  \
      (x)->io_w_serv = (y);                     \
      (x)->io_w_user = io_w;                    \
      (x)->io_dbg    = vlg;                     \
      (x)->opt_recur = cl_opt_recur;            \
    } while (FALSE)


#include "evnt.h"

#include "opt.h"

#include "mk.h"

MALLOC_CHECK_DECL();

struct con
{
 struct Evnt ev[1];
 
 Dns_base d1[1];
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

static const char *server_ipv4_address = "127.0.0.1";
static short server_port = 53;

static unsigned int cl_opt_recur = TRUE;

static Vlg *vlg = NULL;

static void ui_out(Dns_base *d1, Vstr_base *pkt)
{
  if (!io_ind_w)
    return;
  
  dns_sc_ui_out(d1, pkt);
  /* SOCKET_POLL_INDICATOR(io_ind_w)->events |=  POLLOUT; */
}

static void cl_parse(struct con *con, size_t msg_len)
{
  Vstr_base *pkt = vstr_dup_vstr(con->ev->io_r->conf,
                                 con->ev->io_r, 1, msg_len,
                                 VSTR_TYPE_ADD_BUF_PTR);

  if (!pkt)
    errno = ENOMEM, err(EXIT_FAILURE, "%s", __func__);

  vlg_dbg1(vlg, "\n${rep_chr:%c%zu} recv ${BKMG.u:%u} ${rep_chr:%c%zu}\n",
           '=', 33, msg_len, '=', 33);
  dns_dbg_prnt_pkt(con->d1, pkt);
  vlg_dbg1(vlg, "\n${rep_chr:%c%zu}\n", '-', 79);
  
  ui_out(con->d1, pkt);
  vstr_free_base(pkt);
  vstr_del(con->ev->io_r, 1, msg_len);

  evnt_got_pkt(con->ev);
}

static int cl_recv(struct con *con)
{
  unsigned int ern = 0;
  int ret = evnt_recv(con->ev, &ern);
  unsigned int msg_len = 0;
  
  /* parse data */
  while ((msg_len = dns_get_msg_len(con->ev->io_r, 1)))
  {
    if (msg_len > con->ev->io_r->len)
    {
      if (msg_len > CL_PACKET_MAX)
      {
        vlg_dbg2(vlg, "ERROR-RECV-MAX: fd=%d len=%zu\n",
                 SOCKET_POLL_INDICATOR(con->ev->ind)->fd, msg_len);
        return (FALSE);
      }
      
      return (TRUE);
    }

    vstr_del(con->ev->io_r, 1, 2);
    
    cl_parse(con, msg_len - 2);
  }

  if (!ret)
    evnt_close(con->ev);
    
  return (ret);
}

static void cl_app_recq1_pkt(struct con *con,
                             const char *name,
                             unsigned int dns_class,
                             unsigned int dns_type)
{
  dns_app_recq_pkt(con->d1, 1, name, dns_class, dns_type);
  
  evnt_put_pkt(con->ev);
  
  if (!con->ev->flag_q_connect)
    evnt_send_add(con->ev, FALSE, CL_MAX_WAIT_SEND); /* does write */
}


#define UI_CMD(x, c, t)                                                 \
    else if (vstr_cmp_case_bod_cstr_eq(io_r, 1, len, x " ")) do         \
    {                                                                   \
      size_t pos = 1;                                                   \
      size_t tmp = strlen(x " ");                                       \
      const char *name = NULL;                                          \
                                                                        \
      pos += tmp; len -= tmp;                                           \
      tmp = vstr_spn_cstr_chrs_fwd(io_r, pos, len, " ");                \
      pos += tmp; len -= tmp;                                           \
      name = vstr_export_cstr_ptr(io_r, pos, len - 1);                  \
                                                                        \
      cl_app_recq1_pkt((struct con *)con, name, c, t);                  \
    } while (FALSE)

static void cl_connect(void); /* fwd ref */
static void ui_parse(void)
{
  size_t len = 0;
  unsigned int count = 64;
  struct Evnt *con = NULL;

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
    UI_CMD("*.*",      DNS_CLASS_ALL, DNS_TYPE_ALL);
    UI_CMD("all.all",  DNS_CLASS_ALL, DNS_TYPE_ALL);

    UI_CMD("in.*",     DNS_CLASS_IN, DNS_TYPE_ALL);
    UI_CMD("in.all",   DNS_CLASS_IN, DNS_TYPE_ALL);
    
    UI_CMD("in.a",     DNS_CLASS_IN, DNS_TYPE_IN_A);
    UI_CMD("in.aaaa",  DNS_CLASS_IN, DNS_TYPE_IN_AAAA);
    UI_CMD("in.cname", DNS_CLASS_IN, DNS_TYPE_IN_CNAME);
    UI_CMD("in.hinfo", DNS_CLASS_IN, DNS_TYPE_IN_HINFO);
    UI_CMD("in.mx",    DNS_CLASS_IN, DNS_TYPE_IN_MX);
    UI_CMD("in.ns",    DNS_CLASS_IN, DNS_TYPE_IN_NS);
    UI_CMD("in.ptr",   DNS_CLASS_IN, DNS_TYPE_IN_PTR);
    UI_CMD("in.soa",   DNS_CLASS_IN, DNS_TYPE_IN_SOA);
    UI_CMD("in.srv",   DNS_CLASS_IN, DNS_TYPE_IN_SRV);
    UI_CMD("in.txt",   DNS_CLASS_IN, DNS_TYPE_IN_TXT);
    
    UI_CMD("ch.*",     DNS_CLASS_CH, DNS_TYPE_ALL);
    UI_CMD("ch.all",   DNS_CLASS_CH, DNS_TYPE_ALL);
    UI_CMD("ch.txt",   DNS_CLASS_CH, DNS_TYPE_CH_TXT);

    /* ignore everything else */
    
    vstr_del(io_r, 1, line_len);
  }
}
#undef UI_CMD


static int cl_cb_func_connect(struct Evnt *evnt)
{
  (void)evnt;
  
  ui_parse();
  
  return (TRUE);
}

static int cl_cb_func_recv(struct Evnt *evnt)
{
  return (cl_recv((struct con *)evnt));
}

static void cl_cb_func_free(struct Evnt *evnt)
{
  struct con *con = (struct con *)evnt;

  free(con);

  --server_clients_count;
}

static struct con *cl_make(const char *ipv4_string, short port)
{
  struct con *ret = malloc(sizeof(struct con));

  if (!ret)
    errno = ENOMEM, err(EXIT_FAILURE, "%s", __func__);
  if (!evnt_make_con_ipv4(ret->ev, ipv4_string, port))
    err(EXIT_FAILURE, "%s", __func__);

  ret->ev->cbs->cb_func_connect = cl_cb_func_connect;
  ret->ev->cbs->cb_func_recv = cl_cb_func_recv;
  ret->ev->cbs->cb_func_free = cl_cb_func_free;
  
  ++server_clients_count;  

  CL_DNS_INIT(ret->d1, ret->ev->io_w);
  
  if (ret->ev->flag_q_none)
    cl_cb_func_connect(ret->ev);

  return (ret);
}

static void cl_connect(void)
{
  struct con *con = cl_make(server_ipv4_address, server_port);
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

static void usage(const char *program_name, int tst_err)
{
  fprintf(tst_err ? stderr : stdout, "\n Format: %s [-chmtwV] <?>\n"
          " --help -h         - Print this message.\n"
          " --debug -d        - Enable debug info.\n"
          " --clients -c      - Number of connections to make.\n"
          " --host -H         - IPv4 host address to send DNS queries to.\n"
          " --port -P         - Port to send DNS queries to.\n"
          " --nagle -n        - Enable/disable nagle TCP option.\n"
          " --timeout -t      - Timeout (usecs) between each message.\n"
          " --version -V      - Print the version string.\n",
          program_name);
  if (tst_err)
    exit (EXIT_FAILURE);
  else
    exit (EXIT_SUCCESS);
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
   {"recursive", optional_argument, NULL, 'R'},
   {"timeout", required_argument, NULL, 't'},
   {"version", no_argument, NULL, 'V'},
   {NULL, 0, NULL, 0}
  };
  
  program_name = opt_program_name(argv[0], "dns");

  while ((optchar = getopt_long(argc, argv, "c:de:hH:nP:Rt:V",
                                long_options, NULL)) != EOF)
  {
    switch (optchar)
    {
      case '?':
        fprintf(stderr, " That option is not valid.\n");
      case 'h':
        usage(program_name, '?' == optchar);
        
      case 'V':
        printf(" %s version 0.0.1, compiled on %s.\n", program_name, __DATE__);
        exit (EXIT_SUCCESS);

      case 'c': server_clients      = atoi(optarg); break;
      case 't': server_timeout      = atoi(optarg); break;
      case 'H': server_ipv4_address = optarg;       break;
      case 'P': server_port         = atoi(optarg); break;

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

      case 'R':
        if (!optarg)
        { cl_opt_recur = !cl_opt_recur; }
        else if (!strcasecmp("true", optarg))   cl_opt_recur = TRUE;
        else if (!strcasecmp("1", optarg))      cl_opt_recur = TRUE;
        else if (!strcasecmp("false", optarg))  cl_opt_recur = FALSE;
        else if (!strcasecmp("0", optarg))      cl_opt_recur = FALSE;
        break;

      default:
        abort();
    }
  }

  argc -= optind;
  argv += optind;

  //  if (argc != 1)
  //    usage(program_name, TRUE);

  //  ip_addr_input_file = argv[0];
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
  
  if (io_r_fd != -1)
  {
    evnt_fd__set_nonblock(io_r_fd,  TRUE);
    if (!(io_ind_r = socket_poll_add(io_r_fd)))
      errno = ENOMEM, err(EXIT_FAILURE, "%s", __func__);
    SOCKET_POLL_INDICATOR(io_ind_r)->events |= POLLIN; /* FIXME: */
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
  
  while (io_ind_w && (io_w->len || (evnt_waiting() || io_ind_r || io_r->len)))
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
         
