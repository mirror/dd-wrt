/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Bryan Christianson 2015
 * Copyright (C) Miroslav Lichvar  2017
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 **********************************************************************

  =======================================================================

  Perform privileged operations over a unix socket to a privileged fork.
  */

#include "config.h"

#include "sysincl.h"

#include "conf.h"
#include "nameserv.h"
#include "logging.h"
#include "privops.h"
#include "socket.h"
#include "util.h"

#define OP_ADJUSTTIME     1024
#define OP_ADJUSTTIMEX    1025
#define OP_SETTIME        1026
#define OP_BINDSOCKET     1027
#define OP_NAME2IPADDRESS 1028
#define OP_RELOADDNS      1029
#define OP_QUIT           1099

union sockaddr_in46 {
  struct sockaddr_in in4;
#ifdef FEAT_IPV6
  struct sockaddr_in6 in6;
#endif
  struct sockaddr u;
};

/* daemon request structs */

typedef struct {
  struct timeval tv;
} ReqAdjustTime;

#ifdef PRIVOPS_ADJUSTTIMEX
typedef struct {
  struct timex tmx;
} ReqAdjustTimex;
#endif

typedef struct {
  struct timeval tv;
} ReqSetTime;

typedef struct {
  int sock;
  socklen_t sa_len;
  union sockaddr_in46 sa;
} ReqBindSocket;

typedef struct {
  char name[256];
} ReqName2IPAddress;

typedef struct {
  int op;
  union {
    ReqAdjustTime adjust_time;
#ifdef PRIVOPS_ADJUSTTIMEX
    ReqAdjustTimex adjust_timex;
#endif
    ReqSetTime set_time;
    ReqBindSocket bind_socket;
#ifdef PRIVOPS_NAME2IPADDRESS
    ReqName2IPAddress name_to_ipaddress;
#endif
  } data;
} PrvRequest;

/* helper response structs */

typedef struct {
  struct timeval tv;
} ResAdjustTime;

#ifdef PRIVOPS_ADJUSTTIMEX
typedef struct {
  struct timex tmx;
} ResAdjustTimex;
#endif

typedef struct {
  IPAddr addresses[DNS_MAX_ADDRESSES];
} ResName2IPAddress;

typedef struct {
  char msg[256];
} ResFatalMsg;

typedef struct {
  int fatal_error;
  int rc;
  int res_errno;
  union {
    ResFatalMsg fatal_msg;
    ResAdjustTime adjust_time;
#ifdef PRIVOPS_ADJUSTTIMEX
    ResAdjustTimex adjust_timex;
#endif
#ifdef PRIVOPS_NAME2IPADDRESS
    ResName2IPAddress name_to_ipaddress;
#endif
  } data;
} PrvResponse;

static int helper_fd;
static pid_t helper_pid;

static int
have_helper(void)
{
  return helper_fd >= 0;
}

/* ======================================================================= */

/* HELPER - prepare fatal error for daemon */
FORMAT_ATTRIBUTE_PRINTF(2, 3)
static void
res_fatal(PrvResponse *res, const char *fmt, ...)
{
  va_list ap;

  res->fatal_error = 1;
  va_start(ap, fmt);
  vsnprintf(res->data.fatal_msg.msg, sizeof (res->data.fatal_msg.msg), fmt, ap);
  va_end(ap);
}

/* ======================================================================= */

/* HELPER - send response to the fd */

static int
send_response(int fd, const PrvResponse *res)
{
  if (SCK_Send(fd, res, sizeof (*res), 0) != sizeof (*res))
    return 0;

  return 1;
}

/* ======================================================================= */
/* receive daemon request plus optional file descriptor over a unix socket */

static int
receive_from_daemon(int fd, PrvRequest *req)
{
  SCK_Message *message;

  message = SCK_ReceiveMessage(fd, SCK_FLAG_MSG_DESCRIPTOR);
  if (!message || message->length != sizeof (*req))
    return 0;

  memcpy(req, message->data, sizeof (*req));

  if (req->op == OP_BINDSOCKET) {
    req->data.bind_socket.sock = message->descriptor;

    /* return error if valid descriptor not found */
    if (req->data.bind_socket.sock < 0)
      return 0;
  } else if (message->descriptor >= 0) {
    SCK_CloseSocket(message->descriptor);
    return 0;
  }

  return 1;
}

/* ======================================================================= */

/* HELPER - perform adjtime() */

#ifdef PRIVOPS_ADJUSTTIME
static void
do_adjust_time(const ReqAdjustTime *req, PrvResponse *res)
{
  res->rc = adjtime(&req->tv, &res->data.adjust_time.tv);
  if (res->rc)
    res->res_errno = errno;
}
#endif

/* ======================================================================= */

/* HELPER - perform ntp_adjtime() */

#ifdef PRIVOPS_ADJUSTTIMEX
static void
do_adjust_timex(const ReqAdjustTimex *req, PrvResponse *res)
{
  res->data.adjust_timex.tmx = req->tmx;
  res->rc = ntp_adjtime(&res->data.adjust_timex.tmx);
  if (res->rc < 0)
    res->res_errno = errno;
}
#endif

/* ======================================================================= */

/* HELPER - perform settimeofday() */

#ifdef PRIVOPS_SETTIME
static void
do_set_time(const ReqSetTime *req, PrvResponse *res)
{
  res->rc = settimeofday(&req->tv, NULL);
  if (res->rc)
    res->res_errno = errno;
}
#endif

/* ======================================================================= */

/* HELPER - perform bind() */

#ifdef PRIVOPS_BINDSOCKET
static void
do_bind_socket(ReqBindSocket *req, PrvResponse *res)
{
  IPSockAddr ip_saddr;
  int sock_fd;
  struct sockaddr *sa;
  socklen_t sa_len;

  sa = &req->sa.u;
  sa_len = req->sa_len;
  sock_fd = req->sock;

  SCK_SockaddrToIPSockAddr(sa, sa_len, &ip_saddr);
  if (ip_saddr.port != 0 && ip_saddr.port != CNF_GetNTPPort() &&
      ip_saddr.port != CNF_GetAcquisitionPort() && ip_saddr.port != CNF_GetPtpPort()) {
    SCK_CloseSocket(sock_fd);
    res_fatal(res, "Invalid port %d", ip_saddr.port);
    return;
  }

  res->rc = bind(sock_fd, sa, sa_len);
  if (res->rc)
    res->res_errno = errno;

  /* sock is still open on daemon side, but we're done with it in the helper */
  SCK_CloseSocket(sock_fd);
}
#endif

/* ======================================================================= */

/* HELPER - perform DNS_Name2IPAddress() */

#ifdef PRIVOPS_NAME2IPADDRESS
static void
do_name_to_ipaddress(ReqName2IPAddress *req, PrvResponse *res)
{
  /* make sure the string is terminated */
  req->name[sizeof (req->name) - 1] = '\0';

  res->rc = DNS_Name2IPAddress(req->name, res->data.name_to_ipaddress.addresses,
                               DNS_MAX_ADDRESSES);
}
#endif

/* ======================================================================= */

/* HELPER - perform DNS_Reload() */

#ifdef PRIVOPS_RELOADDNS
static void
do_reload_dns(PrvResponse *res)
{
  DNS_Reload();
  res->rc = 0;
}
#endif

/* ======================================================================= */

/* HELPER - main loop - action requests from the daemon */

static void
helper_main(int fd)
{
  PrvRequest req;
  PrvResponse res;
  int quit = 0;

  while (!quit) {
    if (!receive_from_daemon(fd, &req))
      /* read error or closed input - we cannot recover - give up */
      break;

    memset(&res, 0, sizeof (res));

    switch (req.op) {
#ifdef PRIVOPS_ADJUSTTIME
      case OP_ADJUSTTIME:
        do_adjust_time(&req.data.adjust_time, &res);
        break;
#endif
#ifdef PRIVOPS_ADJUSTTIMEX
      case OP_ADJUSTTIMEX:
        do_adjust_timex(&req.data.adjust_timex, &res);
        break;
#endif
#ifdef PRIVOPS_SETTIME
      case OP_SETTIME:
        do_set_time(&req.data.set_time, &res);
        break;
#endif
#ifdef PRIVOPS_BINDSOCKET
      case OP_BINDSOCKET:
        do_bind_socket(&req.data.bind_socket, &res);
        break;
#endif
#ifdef PRIVOPS_NAME2IPADDRESS
      case OP_NAME2IPADDRESS:
        do_name_to_ipaddress(&req.data.name_to_ipaddress, &res);
        break;
#endif
#ifdef PRIVOPS_RELOADDNS
      case OP_RELOADDNS:
        do_reload_dns(&res);
        break;
#endif
      case OP_QUIT:
        quit = 1;
        continue;

      default:
        res_fatal(&res, "Unexpected operator %d", req.op);
        break;
    }

    send_response(fd, &res);
  }

  SCK_CloseSocket(fd);
  exit(0);
}

/* ======================================================================= */

/* DAEMON - receive helper response */

static void
receive_response(PrvResponse *res)
{
  int resp_len;

  resp_len = SCK_Receive(helper_fd, res, sizeof (*res), 0);
  if (resp_len < 0)
    LOG_FATAL("Could not read from helper : %s", strerror(errno));
  if (resp_len != sizeof (*res))
    LOG_FATAL("Invalid helper response");

  if (res->fatal_error)
    LOG_FATAL("Error in helper : %s", res->data.fatal_msg.msg);

  DEBUG_LOG("Received response rc=%d", res->rc);

  /* if operation failed in the helper, set errno so daemon can print log message */
  if (res->res_errno)
    errno = res->res_errno;
}

/* ======================================================================= */

/* DAEMON - send daemon request to the helper */

static void
send_request(PrvRequest *req)
{
  SCK_Message message;
  int flags;

  SCK_InitMessage(&message, SCK_ADDR_UNSPEC);

  message.data = req;
  message.length = sizeof (*req);
  flags = 0;

  if (req->op == OP_BINDSOCKET) {
    /* send file descriptor as a control message */
    message.descriptor = req->data.bind_socket.sock;
    flags |= SCK_FLAG_MSG_DESCRIPTOR;
  }

  if (!SCK_SendMessage(helper_fd, &message, flags)) {
    /* don't try to send another request from exit() */
    helper_fd = -1;
    LOG_FATAL("Could not send to helper : %s", strerror(errno));
  }

  DEBUG_LOG("Sent request op=%d", req->op);
}

/* ======================================================================= */

/* DAEMON - send daemon request and wait for response */

static void
submit_request(PrvRequest *req, PrvResponse *res)
{
  send_request(req);
  receive_response(res);
}

/* ======================================================================= */

/* DAEMON - send the helper a request to exit and wait until it exits */

static void
stop_helper(void)
{
  PrvRequest req;
  int status;

  if (!have_helper())
    return;

  memset(&req, 0, sizeof (req));
  req.op = OP_QUIT;
  send_request(&req);

  waitpid(helper_pid, &status, 0);
}

/* ======================================================================= */

/* DAEMON - request adjtime() */

#ifdef PRIVOPS_ADJUSTTIME
int
PRV_AdjustTime(const struct timeval *delta, struct timeval *olddelta)
{
  PrvRequest req;
  PrvResponse res;

  if (!have_helper() || delta == NULL)
    /* helper is not running or read adjustment call */
    return adjtime(delta, olddelta);

  memset(&req, 0, sizeof (req));
  req.op = OP_ADJUSTTIME;
  req.data.adjust_time.tv = *delta;

  submit_request(&req, &res);

  if (olddelta)
    *olddelta = res.data.adjust_time.tv;

  return res.rc;
}
#endif

/* ======================================================================= */

/* DAEMON - request ntp_adjtime() */

#ifdef PRIVOPS_ADJUSTTIMEX
int
PRV_AdjustTimex(struct timex *tmx)
{
  PrvRequest req;
  PrvResponse res;

  if (!have_helper())
    return ntp_adjtime(tmx);

  memset(&req, 0, sizeof (req));
  req.op = OP_ADJUSTTIMEX;
  req.data.adjust_timex.tmx = *tmx;

  submit_request(&req, &res);

  *tmx = res.data.adjust_timex.tmx;

  return res.rc;
}
#endif

/* ======================================================================= */

/* DAEMON - request settimeofday() */

#ifdef PRIVOPS_SETTIME
int
PRV_SetTime(const struct timeval *tp, const struct timezone *tzp)
{
  PrvRequest req;
  PrvResponse res;

  /* only support setting the time */
  assert(tp != NULL);
  assert(tzp == NULL);

  if (!have_helper())
    return settimeofday(tp, NULL);

  memset(&req, 0, sizeof (req));
  req.op = OP_SETTIME;
  req.data.set_time.tv = *tp;

  submit_request(&req, &res);

  return res.rc;
}
#endif

/* ======================================================================= */

/* DAEMON - request bind() */

#ifdef PRIVOPS_BINDSOCKET
int
PRV_BindSocket(int sock, struct sockaddr *address, socklen_t address_len)
{
  IPSockAddr ip_saddr;
  PrvRequest req;
  PrvResponse res;

  SCK_SockaddrToIPSockAddr(address, address_len, &ip_saddr);
  BRIEF_ASSERT(ip_saddr.port == 0 || ip_saddr.port == CNF_GetNTPPort() ||
               ip_saddr.port == CNF_GetAcquisitionPort() ||
               ip_saddr.port == CNF_GetPtpPort());

  if (!have_helper())
    return bind(sock, address, address_len);

  memset(&req, 0, sizeof (req));
  req.op = OP_BINDSOCKET;
  req.data.bind_socket.sock = sock;
  req.data.bind_socket.sa_len = address_len;
  assert(address_len <= sizeof (req.data.bind_socket.sa));
  memcpy(&req.data.bind_socket.sa.u, address, address_len);

  submit_request(&req, &res);

  return res.rc;
}
#endif

/* ======================================================================= */

/* DAEMON - request DNS_Name2IPAddress() */

#ifdef PRIVOPS_NAME2IPADDRESS
int
PRV_Name2IPAddress(const char *name, IPAddr *ip_addrs, int max_addrs)
{
  PrvRequest req;
  PrvResponse res;
  int i;

  if (!have_helper())
    return DNS_Name2IPAddress(name, ip_addrs, max_addrs);

  memset(&req, 0, sizeof (req));
  req.op = OP_NAME2IPADDRESS;
  if (snprintf(req.data.name_to_ipaddress.name, sizeof (req.data.name_to_ipaddress.name),
               "%s", name) >= sizeof (req.data.name_to_ipaddress.name)) {
    return DNS_Failure;
  }

  submit_request(&req, &res);

  for (i = 0; i < max_addrs && i < DNS_MAX_ADDRESSES; i++)
    ip_addrs[i] = res.data.name_to_ipaddress.addresses[i];

  return res.rc;
}
#endif

/* ======================================================================= */

/* DAEMON - request res_init() */

#ifdef PRIVOPS_RELOADDNS
void
PRV_ReloadDNS(void)
{
  PrvRequest req;
  PrvResponse res;

  if (!have_helper()) {
    DNS_Reload();
    return;
  }

  memset(&req, 0, sizeof (req));
  req.op = OP_RELOADDNS;

  submit_request(&req, &res);
  assert(!res.rc);
}
#endif

/* ======================================================================= */

void
PRV_Initialise(void)
{
  helper_fd = -1;
}

/* ======================================================================= */

/* DAEMON - setup socket(s) then fork to run the helper */
/* must be called before privileges are dropped */

void
PRV_StartHelper(void)
{
  pid_t pid;
  int fd, sock_fd1, sock_fd2;

  if (have_helper())
    LOG_FATAL("Helper already running");

  sock_fd1 = SCK_OpenUnixSocketPair(SCK_FLAG_BLOCK, &sock_fd2);
  if (sock_fd1 < 0)
    LOG_FATAL("Could not open socket pair");

  pid = fork();
  if (pid < 0)
    LOG_FATAL("fork() failed : %s", strerror(errno));

  if (pid == 0) {
    /* child process */
    SCK_CloseSocket(sock_fd1);

    /* close other descriptors inherited from the parent process, except
       stdin, stdout, and stderr */
    for (fd = STDERR_FILENO + 1; fd < 1024; fd++) {
      if (fd != sock_fd2)
        close(fd);
    }

    UTI_ResetGetRandomFunctions();

    /* ignore signals, the process will exit on OP_QUIT request */
    UTI_SetQuitSignalsHandler(SIG_IGN, 1);

    helper_main(sock_fd2);

  } else {
    /* parent process */
    SCK_CloseSocket(sock_fd2);
    helper_fd = sock_fd1;
    helper_pid = pid;

    /* stop the helper even when not exiting cleanly from the main function */
    atexit(stop_helper);
  }
}

/* ======================================================================= */

/* DAEMON - graceful shutdown of the helper */

void
PRV_Finalise(void)
{
  if (!have_helper())
    return;

  stop_helper();
  close(helper_fd);
  helper_fd = -1;
}
