/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * Copyright (C) Timo Teras  2009
 * Copyright (C) Miroslav Lichvar  2009, 2013-2016, 2018
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

  This file deals with the IO aspects of reading and writing NTP packets
  */

#include "config.h"

#include "sysincl.h"

#include "array.h"
#include "ntp_io.h"
#include "ntp_core.h"
#include "ntp_sources.h"
#include "sched.h"
#include "local.h"
#include "logging.h"
#include "conf.h"
#include "privops.h"
#include "util.h"

#ifdef HAVE_LINUX_TIMESTAMPING
#include "ntp_io_linux.h"
#endif

#define INVALID_SOCK_FD -1
#define CMSGBUF_SIZE 256

union sockaddr_in46 {
  struct sockaddr_in in4;
#ifdef FEAT_IPV6
  struct sockaddr_in6 in6;
#endif
  struct sockaddr u;
};

struct Message {
  union sockaddr_in46 name;
  struct iovec iov;
  NTP_Receive_Buffer buf;
  /* Aligned buffer for control messages */
  struct cmsghdr cmsgbuf[CMSGBUF_SIZE / sizeof (struct cmsghdr)];
};

#ifdef HAVE_RECVMMSG
#define MAX_RECV_MESSAGES 4
#define MessageHeader mmsghdr
#else
/* Compatible with mmsghdr */
struct MessageHeader {
  struct msghdr msg_hdr;
  unsigned int msg_len;
};

#define MAX_RECV_MESSAGES 1
#endif

/* Arrays of Message and MessageHeader */
static ARR_Instance recv_messages;
static ARR_Instance recv_headers;

/* The server/peer and client sockets for IPv4 and IPv6 */
static int server_sock_fd4;
static int client_sock_fd4;
#ifdef FEAT_IPV6
static int server_sock_fd6;
static int client_sock_fd6;
#endif

/* Reference counters for server sockets to keep them open only when needed */
static int server_sock_ref4;
#ifdef FEAT_IPV6
static int server_sock_ref6;
#endif

/* Flag indicating we create a new connected client socket for each
   server instead of sharing client_sock_fd4 and client_sock_fd6 */
static int separate_client_sockets;

/* Flag indicating the server sockets are not created dynamically when needed,
   either to have a socket for client requests when separate client sockets
   are disabled and client port is equal to server port, or the server port is
   disabled */
static int permanent_server_sockets;

/* Flag indicating the server IPv4 socket is bound to an address */
static int bound_server_sock_fd4;

/* Flag indicating that we have been initialised */
static int initialised=0;

/* ================================================== */

/* Forward prototypes */
static void read_from_socket(int sock_fd, int event, void *anything);

/* ================================================== */

static int
prepare_socket(int family, int port_number, int client_only)
{
  union sockaddr_in46 my_addr;
  socklen_t my_addr_len;
  int sock_fd;
  IPAddr bind_address;
  int events = SCH_FILE_INPUT, on_off = 1;

  /* Open Internet domain UDP socket for NTP message transmissions */

  sock_fd = socket(family, SOCK_DGRAM, 0);

  if (sock_fd < 0) {
    if (!client_only) {
      LOG(LOGS_ERR, "Could not open %s NTP socket : %s",
          UTI_SockaddrFamilyToString(family), strerror(errno));
    } else {
      DEBUG_LOG("Could not open %s NTP socket : %s",
                UTI_SockaddrFamilyToString(family), strerror(errno));
    }
    return INVALID_SOCK_FD;
  }

  /* Close on exec */
  UTI_FdSetCloexec(sock_fd);

  /* Enable non-blocking mode on server sockets */
  if (!client_only && fcntl(sock_fd, F_SETFL, O_NONBLOCK))
    DEBUG_LOG("Could not set O_NONBLOCK : %s", strerror(errno));

  /* Prepare local address */
  memset(&my_addr, 0, sizeof (my_addr));
  my_addr_len = 0;

  switch (family) {
    case AF_INET:
      if (!client_only)
        CNF_GetBindAddress(IPADDR_INET4, &bind_address);
      else
        CNF_GetBindAcquisitionAddress(IPADDR_INET4, &bind_address);

      if (bind_address.family == IPADDR_INET4)
        my_addr.in4.sin_addr.s_addr = htonl(bind_address.addr.in4);
      else if (port_number)
        my_addr.in4.sin_addr.s_addr = htonl(INADDR_ANY);
      else
        break;

      my_addr.in4.sin_family = family;
      my_addr.in4.sin_port = htons(port_number);
      my_addr_len = sizeof (my_addr.in4);

      if (!client_only)
        bound_server_sock_fd4 = my_addr.in4.sin_addr.s_addr != htonl(INADDR_ANY);

      break;
#ifdef FEAT_IPV6
    case AF_INET6:
      if (!client_only)
        CNF_GetBindAddress(IPADDR_INET6, &bind_address);
      else
        CNF_GetBindAcquisitionAddress(IPADDR_INET6, &bind_address);

      if (bind_address.family == IPADDR_INET6)
        memcpy(my_addr.in6.sin6_addr.s6_addr, bind_address.addr.in6,
            sizeof (my_addr.in6.sin6_addr.s6_addr));
      else if (port_number)
        my_addr.in6.sin6_addr = in6addr_any;
      else
        break;

      my_addr.in6.sin6_family = family;
      my_addr.in6.sin6_port = htons(port_number);
      my_addr_len = sizeof (my_addr.in6);

      break;
#endif
    default:
      assert(0);
  }

  /* Make the socket capable of re-using an old address if binding to a specific port */
  if (port_number &&
      setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&on_off, sizeof(on_off)) < 0) {
    LOG(LOGS_ERR, "Could not set %s socket option", "SO_REUSEADDR");
    /* Don't quit - we might survive anyway */
  }
  
  /* Make the socket capable of sending broadcast pkts - needed for NTP broadcast mode */
  if (!client_only &&
      setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, (char *)&on_off, sizeof(on_off)) < 0) {
    LOG(LOGS_ERR, "Could not set %s socket option", "SO_BROADCAST");
    /* Don't quit - we might survive anyway */
  }

  /* Enable kernel/HW timestamping of packets */
#ifdef HAVE_LINUX_TIMESTAMPING
  if (!NIO_Linux_SetTimestampSocketOptions(sock_fd, client_only, &events))
#endif
#ifdef SO_TIMESTAMPNS
    if (setsockopt(sock_fd, SOL_SOCKET, SO_TIMESTAMPNS, (char *)&on_off, sizeof(on_off)) < 0)
#endif
#ifdef SO_TIMESTAMP
      if (setsockopt(sock_fd, SOL_SOCKET, SO_TIMESTAMP, (char *)&on_off, sizeof(on_off)) < 0)
        LOG(LOGS_ERR, "Could not set %s socket option", "SO_TIMESTAMP");
#endif
      ;

#ifdef IP_FREEBIND
  /* Allow binding to address that doesn't exist yet */
  if (my_addr_len > 0 &&
      setsockopt(sock_fd, IPPROTO_IP, IP_FREEBIND, (char *)&on_off, sizeof(on_off)) < 0) {
    LOG(LOGS_ERR, "Could not set %s socket option", "IP_FREEBIND");
  }
#endif

  if (family == AF_INET) {
#ifdef HAVE_IN_PKTINFO
    if (setsockopt(sock_fd, IPPROTO_IP, IP_PKTINFO, (char *)&on_off, sizeof(on_off)) < 0)
      LOG(LOGS_ERR, "Could not set %s socket option", "IP_PKTINFO");
#elif defined(IP_RECVDSTADDR)
    if (setsockopt(sock_fd, IPPROTO_IP, IP_RECVDSTADDR, (char *)&on_off, sizeof(on_off)) < 0)
      LOG(LOGS_ERR, "Could not set %s socket option", "IP_RECVDSTADDR");
#endif
  }
#ifdef FEAT_IPV6
  else if (family == AF_INET6) {
#ifdef IPV6_V6ONLY
    /* Receive IPv6 packets only */
    if (setsockopt(sock_fd, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&on_off, sizeof(on_off)) < 0) {
      LOG(LOGS_ERR, "Could not set %s socket option", "IPV6_V6ONLY");
    }
#endif

#ifdef HAVE_IN6_PKTINFO
#ifdef IPV6_RECVPKTINFO
    if (setsockopt(sock_fd, IPPROTO_IPV6, IPV6_RECVPKTINFO, (char *)&on_off, sizeof(on_off)) < 0) {
      LOG(LOGS_ERR, "Could not set %s socket option", "IPV6_RECVPKTINFO");
    }
#else
    if (setsockopt(sock_fd, IPPROTO_IPV6, IPV6_PKTINFO, (char *)&on_off, sizeof(on_off)) < 0) {
      LOG(LOGS_ERR, "Could not set %s socket option", "IPV6_PKTINFO");
    }
#endif
#endif
  }
#endif

  /* Bind the socket if a port or address was specified */
  if (my_addr_len > 0 && PRV_BindSocket(sock_fd, &my_addr.u, my_addr_len) < 0) {
    LOG(LOGS_ERR, "Could not bind %s NTP socket : %s",
        UTI_SockaddrFamilyToString(family), strerror(errno));
    close(sock_fd);
    return INVALID_SOCK_FD;
  }

  /* Register handler for read and possibly exception events on the socket */
  SCH_AddFileHandler(sock_fd, events, read_from_socket, NULL);

  return sock_fd;
}

/* ================================================== */

static int
prepare_separate_client_socket(int family)
{
  switch (family) {
    case IPADDR_INET4:
      return prepare_socket(AF_INET, 0, 1);
#ifdef FEAT_IPV6
    case IPADDR_INET6:
      return prepare_socket(AF_INET6, 0, 1);
#endif
    default:
      return INVALID_SOCK_FD;
  }
}

/* ================================================== */

static int
connect_socket(int sock_fd, NTP_Remote_Address *remote_addr)
{
  union sockaddr_in46 addr;
  socklen_t addr_len;

  addr_len = UTI_IPAndPortToSockaddr(&remote_addr->ip_addr, remote_addr->port, &addr.u);

  assert(addr_len);

  if (connect(sock_fd, &addr.u, addr_len) < 0) {
    DEBUG_LOG("Could not connect NTP socket to %s:%d : %s",
        UTI_IPToString(&remote_addr->ip_addr), remote_addr->port,
        strerror(errno));
    return 0;
  }

  return 1;
}

/* ================================================== */

static void
close_socket(int sock_fd)
{
  if (sock_fd == INVALID_SOCK_FD)
    return;

#ifdef HAVE_LINUX_TIMESTAMPING
  NIO_Linux_NotifySocketClosing(sock_fd);
#endif
  SCH_RemoveFileHandler(sock_fd);
  close(sock_fd);
}

/* ================================================== */

static void
prepare_buffers(unsigned int n)
{
  struct MessageHeader *hdr;
  struct Message *msg;
  unsigned int i;

  for (i = 0; i < n; i++) {
    msg = ARR_GetElement(recv_messages, i);
    hdr = ARR_GetElement(recv_headers, i);

    msg->iov.iov_base = &msg->buf;
    msg->iov.iov_len = sizeof (msg->buf);
    hdr->msg_hdr.msg_name = &msg->name;
    hdr->msg_hdr.msg_namelen = sizeof (msg->name);
    hdr->msg_hdr.msg_iov = &msg->iov;
    hdr->msg_hdr.msg_iovlen = 1;
    hdr->msg_hdr.msg_control = &msg->cmsgbuf;
    hdr->msg_hdr.msg_controllen = sizeof (msg->cmsgbuf);
    hdr->msg_hdr.msg_flags = 0;
    hdr->msg_len = 0;
  }
}

/* ================================================== */

void
NIO_Initialise(int family)
{
  int server_port, client_port;

  assert(!initialised);
  initialised = 1;

#ifdef HAVE_LINUX_TIMESTAMPING
  NIO_Linux_Initialise();
#else
  if (1) {
    CNF_HwTsInterface *conf_iface;
    if (CNF_GetHwTsInterface(0, &conf_iface))
      LOG_FATAL("HW timestamping not supported");
  }
#endif

  recv_messages = ARR_CreateInstance(sizeof (struct Message));
  ARR_SetSize(recv_messages, MAX_RECV_MESSAGES);
  recv_headers = ARR_CreateInstance(sizeof (struct MessageHeader));
  ARR_SetSize(recv_headers, MAX_RECV_MESSAGES);
  prepare_buffers(MAX_RECV_MESSAGES);

  server_port = CNF_GetNTPPort();
  client_port = CNF_GetAcquisitionPort();

  /* Use separate connected sockets if client port is negative */
  separate_client_sockets = client_port < 0;
  if (client_port < 0)
    client_port = 0;

  permanent_server_sockets = !server_port || (!separate_client_sockets &&
                                              client_port == server_port);

  server_sock_fd4 = INVALID_SOCK_FD;
  client_sock_fd4 = INVALID_SOCK_FD;
  server_sock_ref4 = 0;
#ifdef FEAT_IPV6
  server_sock_fd6 = INVALID_SOCK_FD;
  client_sock_fd6 = INVALID_SOCK_FD;
  server_sock_ref6 = 0;
#endif

  if (family == IPADDR_UNSPEC || family == IPADDR_INET4) {
    if (permanent_server_sockets && server_port)
      server_sock_fd4 = prepare_socket(AF_INET, server_port, 0);
    if (!separate_client_sockets) {
      if (client_port != server_port || !server_port)
        client_sock_fd4 = prepare_socket(AF_INET, client_port, 1);
      else
        client_sock_fd4 = server_sock_fd4;
    }
  }
#ifdef FEAT_IPV6
  if (family == IPADDR_UNSPEC || family == IPADDR_INET6) {
    if (permanent_server_sockets && server_port)
      server_sock_fd6 = prepare_socket(AF_INET6, server_port, 0);
    if (!separate_client_sockets) {
      if (client_port != server_port || !server_port)
        client_sock_fd6 = prepare_socket(AF_INET6, client_port, 1);
      else
        client_sock_fd6 = server_sock_fd6;
    }
  }
#endif

  if ((server_port && server_sock_fd4 == INVALID_SOCK_FD &&
       permanent_server_sockets 
#ifdef FEAT_IPV6
       && server_sock_fd6 == INVALID_SOCK_FD
#endif
      ) || (!separate_client_sockets && client_sock_fd4 == INVALID_SOCK_FD
#ifdef FEAT_IPV6
       && client_sock_fd6 == INVALID_SOCK_FD
#endif
      )) {
    LOG_FATAL("Could not open NTP sockets");
  }
}

/* ================================================== */

void
NIO_Finalise(void)
{
  if (server_sock_fd4 != client_sock_fd4)
    close_socket(client_sock_fd4);
  close_socket(server_sock_fd4);
  server_sock_fd4 = client_sock_fd4 = INVALID_SOCK_FD;
#ifdef FEAT_IPV6
  if (server_sock_fd6 != client_sock_fd6)
    close_socket(client_sock_fd6);
  close_socket(server_sock_fd6);
  server_sock_fd6 = client_sock_fd6 = INVALID_SOCK_FD;
#endif
  ARR_DestroyInstance(recv_headers);
  ARR_DestroyInstance(recv_messages);

#ifdef HAVE_LINUX_TIMESTAMPING
  NIO_Linux_Finalise();
#endif

  initialised = 0;
}

/* ================================================== */

int
NIO_OpenClientSocket(NTP_Remote_Address *remote_addr)
{
  if (separate_client_sockets) {
    int sock_fd = prepare_separate_client_socket(remote_addr->ip_addr.family);

    if (sock_fd == INVALID_SOCK_FD)
      return INVALID_SOCK_FD;

    if (!connect_socket(sock_fd, remote_addr)) {
      close_socket(sock_fd);
      return INVALID_SOCK_FD;
    }

    return sock_fd;
  } else {
    switch (remote_addr->ip_addr.family) {
      case IPADDR_INET4:
        return client_sock_fd4;
#ifdef FEAT_IPV6
      case IPADDR_INET6:
        return client_sock_fd6;
#endif
      default:
        return INVALID_SOCK_FD;
    }
  }
}

/* ================================================== */

int
NIO_OpenServerSocket(NTP_Remote_Address *remote_addr)
{
  switch (remote_addr->ip_addr.family) {
    case IPADDR_INET4:
      if (permanent_server_sockets)
        return server_sock_fd4;
      if (server_sock_fd4 == INVALID_SOCK_FD)
        server_sock_fd4 = prepare_socket(AF_INET, CNF_GetNTPPort(), 0);
      if (server_sock_fd4 != INVALID_SOCK_FD)
        server_sock_ref4++;
      return server_sock_fd4;
#ifdef FEAT_IPV6
    case IPADDR_INET6:
      if (permanent_server_sockets)
        return server_sock_fd6;
      if (server_sock_fd6 == INVALID_SOCK_FD)
        server_sock_fd6 = prepare_socket(AF_INET6, CNF_GetNTPPort(), 0);
      if (server_sock_fd6 != INVALID_SOCK_FD)
        server_sock_ref6++;
      return server_sock_fd6;
#endif
    default:
      return INVALID_SOCK_FD;
  }
}

/* ================================================== */

void
NIO_CloseClientSocket(int sock_fd)
{
  if (separate_client_sockets)
    close_socket(sock_fd);
}

/* ================================================== */

void
NIO_CloseServerSocket(int sock_fd)
{
  if (permanent_server_sockets || sock_fd == INVALID_SOCK_FD)
    return;

  if (sock_fd == server_sock_fd4) {
    if (--server_sock_ref4 <= 0) {
      close_socket(server_sock_fd4);
      server_sock_fd4 = INVALID_SOCK_FD;
    }
  }
#ifdef FEAT_IPV6
  else if (sock_fd == server_sock_fd6) {
    if (--server_sock_ref6 <= 0) {
      close_socket(server_sock_fd6);
      server_sock_fd6 = INVALID_SOCK_FD;
    }
  }
#endif
  else {
    assert(0);
  }
}

/* ================================================== */

int
NIO_IsServerSocket(int sock_fd)
{
  return sock_fd != INVALID_SOCK_FD &&
    (sock_fd == server_sock_fd4
#ifdef FEAT_IPV6
     || sock_fd == server_sock_fd6
#endif
    );
}

/* ================================================== */

int
NIO_IsServerConnectable(NTP_Remote_Address *remote_addr)
{
  int sock_fd, r;

  sock_fd = prepare_separate_client_socket(remote_addr->ip_addr.family);
  if (sock_fd == INVALID_SOCK_FD)
    return 0;

  r = connect_socket(sock_fd, remote_addr);
  close_socket(sock_fd);

  return r;
}

/* ================================================== */

static void
process_message(struct msghdr *hdr, int length, int sock_fd)
{
  NTP_Remote_Address remote_addr;
  NTP_Local_Address local_addr;
  NTP_Local_Timestamp local_ts;
  struct timespec sched_ts;
  struct cmsghdr *cmsg;

  SCH_GetLastEventTime(&local_ts.ts, &local_ts.err, NULL);
  local_ts.source = NTP_TS_DAEMON;
  sched_ts = local_ts.ts;

  if (hdr->msg_namelen > sizeof (union sockaddr_in46)) {
    DEBUG_LOG("Truncated source address");
    return;
  }

  if (hdr->msg_namelen >= sizeof (((struct sockaddr *)hdr->msg_name)->sa_family)) {
    UTI_SockaddrToIPAndPort((struct sockaddr *)hdr->msg_name,
                            &remote_addr.ip_addr, &remote_addr.port);
  } else {
    remote_addr.ip_addr.family = IPADDR_UNSPEC;
    remote_addr.port = 0;
  }

  local_addr.ip_addr.family = IPADDR_UNSPEC;
  local_addr.if_index = INVALID_IF_INDEX;
  local_addr.sock_fd = sock_fd;

  if (hdr->msg_flags & MSG_TRUNC) {
    DEBUG_LOG("Received truncated message from %s:%d",
              UTI_IPToString(&remote_addr.ip_addr), remote_addr.port);
    return;
  }

  if (hdr->msg_flags & MSG_CTRUNC) {
    DEBUG_LOG("Truncated control message");
    /* Continue */
  }

  for (cmsg = CMSG_FIRSTHDR(hdr); cmsg; cmsg = CMSG_NXTHDR(hdr, cmsg)) {
#ifdef HAVE_IN_PKTINFO
    if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_PKTINFO) {
      struct in_pktinfo ipi;

      memcpy(&ipi, CMSG_DATA(cmsg), sizeof(ipi));
      local_addr.ip_addr.addr.in4 = ntohl(ipi.ipi_addr.s_addr);
      local_addr.ip_addr.family = IPADDR_INET4;
      local_addr.if_index = ipi.ipi_ifindex;
    }
#elif defined(IP_RECVDSTADDR)
    if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_RECVDSTADDR) {
      struct in_addr addr;

      memcpy(&addr, CMSG_DATA(cmsg), sizeof (addr));
      local_addr.ip_addr.addr.in4 = ntohl(addr.s_addr);
      local_addr.ip_addr.family = IPADDR_INET4;
    }
#endif

#ifdef HAVE_IN6_PKTINFO
    if (cmsg->cmsg_level == IPPROTO_IPV6 && cmsg->cmsg_type == IPV6_PKTINFO) {
      struct in6_pktinfo ipi;

      memcpy(&ipi, CMSG_DATA(cmsg), sizeof(ipi));
      memcpy(&local_addr.ip_addr.addr.in6, &ipi.ipi6_addr.s6_addr,
             sizeof (local_addr.ip_addr.addr.in6));
      local_addr.ip_addr.family = IPADDR_INET6;
      local_addr.if_index = ipi.ipi6_ifindex;
    }
#endif

#ifdef SCM_TIMESTAMP
    if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_TIMESTAMP) {
      struct timeval tv;
      struct timespec ts;

      memcpy(&tv, CMSG_DATA(cmsg), sizeof(tv));
      UTI_TimevalToTimespec(&tv, &ts);
      LCL_CookTime(&ts, &local_ts.ts, &local_ts.err);
      local_ts.source = NTP_TS_KERNEL;
    }
#endif

#ifdef SCM_TIMESTAMPNS
    if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_TIMESTAMPNS) {
      struct timespec ts;

      memcpy(&ts, CMSG_DATA(cmsg), sizeof (ts));
      LCL_CookTime(&ts, &local_ts.ts, &local_ts.err);
      local_ts.source = NTP_TS_KERNEL;
    }
#endif
  }

#ifdef HAVE_LINUX_TIMESTAMPING
  if (NIO_Linux_ProcessMessage(&remote_addr, &local_addr, &local_ts, hdr, length))
    return;
#endif

  DEBUG_LOG("Received %d bytes from %s:%d to %s fd=%d if=%d tss=%u delay=%.9f",
            length, UTI_IPToString(&remote_addr.ip_addr), remote_addr.port,
            UTI_IPToString(&local_addr.ip_addr), local_addr.sock_fd, local_addr.if_index,
            local_ts.source, UTI_DiffTimespecsToDouble(&sched_ts, &local_ts.ts));

  /* Just ignore the packet if it's not of a recognized length */
  if (length < NTP_NORMAL_PACKET_LENGTH || length > sizeof (NTP_Receive_Buffer))
    return;

  NSR_ProcessRx(&remote_addr, &local_addr, &local_ts,
                (NTP_Packet *)hdr->msg_iov[0].iov_base, length);
}

/* ================================================== */

static void
read_from_socket(int sock_fd, int event, void *anything)
{
  /* This should only be called when there is something
     to read, otherwise it may block */

  struct MessageHeader *hdr;
  unsigned int i, n;
  int status, flags = 0;

#ifdef HAVE_LINUX_TIMESTAMPING
  if (NIO_Linux_ProcessEvent(sock_fd, event))
    return;
#endif

  hdr = ARR_GetElements(recv_headers);
  n = ARR_GetSize(recv_headers);
  assert(n >= 1);

  if (event == SCH_FILE_EXCEPTION) {
#ifdef HAVE_LINUX_TIMESTAMPING
    flags |= MSG_ERRQUEUE;
#else
    assert(0);
#endif
  }

#ifdef HAVE_RECVMMSG
  status = recvmmsg(sock_fd, hdr, n, flags | MSG_DONTWAIT, NULL);
  if (status >= 0)
    n = status;
#else
  n = 1;
  status = recvmsg(sock_fd, &hdr[0].msg_hdr, flags);
  if (status >= 0)
    hdr[0].msg_len = status;
#endif

  if (status < 0) {
#ifdef HAVE_LINUX_TIMESTAMPING
    /* If reading from the error queue failed, the exception should be
       for a socket error.  Clear the error to avoid a busy loop. */
    if (flags & MSG_ERRQUEUE) {
      int error = 0;
      socklen_t len = sizeof (error);

      if (getsockopt(sock_fd, SOL_SOCKET, SO_ERROR, &error, &len))
        DEBUG_LOG("Could not get SO_ERROR");
      if (error)
        errno = error;
    }
#endif

    DEBUG_LOG("Could not receive from fd %d : %s", sock_fd,
              strerror(errno));
    return;
  }

  for (i = 0; i < n; i++) {
    hdr = ARR_GetElement(recv_headers, i);
    process_message(&hdr->msg_hdr, hdr->msg_len, sock_fd);
  }

  /* Restore the buffers to their original state */
  prepare_buffers(n);
}

/* ================================================== */
/* Send a packet to remote address from local address */

int
NIO_SendPacket(NTP_Packet *packet, NTP_Remote_Address *remote_addr,
               NTP_Local_Address *local_addr, int length, int process_tx)
{
  union sockaddr_in46 remote;
  struct msghdr msg;
  struct iovec iov;
  struct cmsghdr *cmsg, cmsgbuf[CMSGBUF_SIZE / sizeof (struct cmsghdr)];
  int cmsglen;
  socklen_t addrlen = 0;

  assert(initialised);

  if (local_addr->sock_fd == INVALID_SOCK_FD) {
    DEBUG_LOG("No socket to send to %s:%d",
              UTI_IPToString(&remote_addr->ip_addr), remote_addr->port);
    return 0;
  }

  /* Don't set address with connected socket */
  if (NIO_IsServerSocket(local_addr->sock_fd) || !separate_client_sockets) {
    addrlen = UTI_IPAndPortToSockaddr(&remote_addr->ip_addr, remote_addr->port,
                                      &remote.u);
    if (!addrlen)
      return 0;
  }

  if (addrlen) {
    msg.msg_name = &remote.u;
    msg.msg_namelen = addrlen;
  } else {
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
  }

  iov.iov_base = packet;
  iov.iov_len = length;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = cmsgbuf;
  msg.msg_controllen = sizeof(cmsgbuf);
  msg.msg_flags = 0;
  cmsglen = 0;

#ifdef HAVE_IN_PKTINFO
  if (local_addr->ip_addr.family == IPADDR_INET4) {
    struct in_pktinfo *ipi;

    cmsg = CMSG_FIRSTHDR(&msg);
    memset(cmsg, 0, CMSG_SPACE(sizeof(struct in_pktinfo)));
    cmsglen += CMSG_SPACE(sizeof(struct in_pktinfo));

    cmsg->cmsg_level = IPPROTO_IP;
    cmsg->cmsg_type = IP_PKTINFO;
    cmsg->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));

    ipi = (struct in_pktinfo *) CMSG_DATA(cmsg);
    ipi->ipi_spec_dst.s_addr = htonl(local_addr->ip_addr.addr.in4);
    if (local_addr->if_index != INVALID_IF_INDEX)
      ipi->ipi_ifindex = local_addr->if_index;
  }
#elif defined(IP_SENDSRCADDR)
  /* Specify the IPv4 source address only if the socket is not bound */
  if (local_addr->ip_addr.family == IPADDR_INET4 &&
      local_addr->sock_fd == server_sock_fd4 && !bound_server_sock_fd4) {
    struct in_addr *addr;

    cmsg = CMSG_FIRSTHDR(&msg);
    memset(cmsg, 0, CMSG_SPACE(sizeof (struct in_addr)));
    cmsglen += CMSG_SPACE(sizeof (struct in_addr));

    cmsg->cmsg_level = IPPROTO_IP;
    cmsg->cmsg_type = IP_SENDSRCADDR;
    cmsg->cmsg_len = CMSG_LEN(sizeof (struct in_addr));

    addr = (struct in_addr *)CMSG_DATA(cmsg);
    addr->s_addr = htonl(local_addr->ip_addr.addr.in4);
  }
#endif

#ifdef HAVE_IN6_PKTINFO
  if (local_addr->ip_addr.family == IPADDR_INET6) {
    struct in6_pktinfo *ipi;

    cmsg = CMSG_FIRSTHDR(&msg);
    memset(cmsg, 0, CMSG_SPACE(sizeof(struct in6_pktinfo)));
    cmsglen += CMSG_SPACE(sizeof(struct in6_pktinfo));

    cmsg->cmsg_level = IPPROTO_IPV6;
    cmsg->cmsg_type = IPV6_PKTINFO;
    cmsg->cmsg_len = CMSG_LEN(sizeof(struct in6_pktinfo));

    ipi = (struct in6_pktinfo *) CMSG_DATA(cmsg);
    memcpy(&ipi->ipi6_addr.s6_addr, &local_addr->ip_addr.addr.in6,
        sizeof(ipi->ipi6_addr.s6_addr));
    if (local_addr->if_index != INVALID_IF_INDEX)
      ipi->ipi6_ifindex = local_addr->if_index;
  }
#endif

#ifdef HAVE_LINUX_TIMESTAMPING
  if (process_tx)
   cmsglen = NIO_Linux_RequestTxTimestamp(&msg, cmsglen, local_addr->sock_fd);
#endif

  msg.msg_controllen = cmsglen;
  /* This is apparently required on some systems */
  if (!cmsglen)
    msg.msg_control = NULL;

  if (sendmsg(local_addr->sock_fd, &msg, 0) < 0) {
    DEBUG_LOG("Could not send to %s:%d from %s fd %d : %s",
        UTI_IPToString(&remote_addr->ip_addr), remote_addr->port,
        UTI_IPToString(&local_addr->ip_addr), local_addr->sock_fd,
        strerror(errno));
    return 0;
  }

  DEBUG_LOG("Sent %d bytes to %s:%d from %s fd %d", length,
      UTI_IPToString(&remote_addr->ip_addr), remote_addr->port,
      UTI_IPToString(&local_addr->ip_addr), local_addr->sock_fd);

  return 1;
}
