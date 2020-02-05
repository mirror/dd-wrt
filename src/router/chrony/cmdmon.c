/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * Copyright (C) Miroslav Lichvar  2009-2016, 2018
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

  Command and monitoring module in the main program
  */

#include "config.h"

#include "sysincl.h"

#include "cmdmon.h"
#include "candm.h"
#include "sched.h"
#include "util.h"
#include "logging.h"
#include "keys.h"
#include "ntp_sources.h"
#include "ntp_core.h"
#include "smooth.h"
#include "sources.h"
#include "sourcestats.h"
#include "reference.h"
#include "manual.h"
#include "memory.h"
#include "local.h"
#include "addrfilt.h"
#include "conf.h"
#include "rtc.h"
#include "pktlength.h"
#include "clientlog.h"
#include "refclock.h"

/* ================================================== */

union sockaddr_all {
  struct sockaddr_in in4;
#ifdef FEAT_IPV6
  struct sockaddr_in6 in6;
#endif
  struct sockaddr_un un;
  struct sockaddr sa;
};

/* File descriptors for command and monitoring sockets */
static int sock_fdu;
static int sock_fd4;
#ifdef FEAT_IPV6
static int sock_fd6;
#endif

/* Flag indicating whether this module has been initialised or not */
static int initialised = 0;

/* ================================================== */
/* Array of permission levels for command types */

static const char permissions[] = {
  PERMIT_OPEN, /* NULL */
  PERMIT_AUTH, /* ONLINE */
  PERMIT_AUTH, /* OFFLINE */
  PERMIT_AUTH, /* BURST */
  PERMIT_AUTH, /* MODIFY_MINPOLL */
  PERMIT_AUTH, /* MODIFY_MAXPOLL */
  PERMIT_AUTH, /* DUMP */
  PERMIT_AUTH, /* MODIFY_MAXDELAY */
  PERMIT_AUTH, /* MODIFY_MAXDELAYRATIO */
  PERMIT_AUTH, /* MODIFY_MAXUPDATESKEW */
  PERMIT_OPEN, /* LOGON */
  PERMIT_AUTH, /* SETTIME */
  PERMIT_AUTH, /* LOCAL */
  PERMIT_AUTH, /* MANUAL */
  PERMIT_OPEN, /* N_SOURCES */
  PERMIT_OPEN, /* SOURCE_DATA */
  PERMIT_AUTH, /* REKEY */
  PERMIT_AUTH, /* ALLOW */
  PERMIT_AUTH, /* ALLOWALL */
  PERMIT_AUTH, /* DENY */
  PERMIT_AUTH, /* DENYALL */
  PERMIT_AUTH, /* CMDALLOW */
  PERMIT_AUTH, /* CMDALLOWALL */
  PERMIT_AUTH, /* CMDDENY */
  PERMIT_AUTH, /* CMDDENYALL */
  PERMIT_AUTH, /* ACCHECK */
  PERMIT_AUTH, /* CMDACCHECK */
  PERMIT_AUTH, /* ADD_SERVER */
  PERMIT_AUTH, /* ADD_PEER */
  PERMIT_AUTH, /* DEL_SOURCE */
  PERMIT_AUTH, /* WRITERTC */
  PERMIT_AUTH, /* DFREQ */
  PERMIT_AUTH, /* DOFFSET */
  PERMIT_OPEN, /* TRACKING */
  PERMIT_OPEN, /* SOURCESTATS */
  PERMIT_OPEN, /* RTCREPORT */
  PERMIT_AUTH, /* TRIMRTC */
  PERMIT_AUTH, /* CYCLELOGS */
  PERMIT_AUTH, /* SUBNETS_ACCESSED */
  PERMIT_AUTH, /* CLIENT_ACCESSES (by subnet) */
  PERMIT_AUTH, /* CLIENT_ACCESSES_BY_INDEX */
  PERMIT_OPEN, /* MANUAL_LIST */
  PERMIT_AUTH, /* MANUAL_DELETE */
  PERMIT_AUTH, /* MAKESTEP */
  PERMIT_OPEN, /* ACTIVITY */
  PERMIT_AUTH, /* MODIFY_MINSTRATUM */
  PERMIT_AUTH, /* MODIFY_POLLTARGET */
  PERMIT_AUTH, /* MODIFY_MAXDELAYDEVRATIO */
  PERMIT_AUTH, /* RESELECT */
  PERMIT_AUTH, /* RESELECTDISTANCE */
  PERMIT_AUTH, /* MODIFY_MAKESTEP */
  PERMIT_OPEN, /* SMOOTHING */
  PERMIT_AUTH, /* SMOOTHTIME */
  PERMIT_AUTH, /* REFRESH */
  PERMIT_AUTH, /* SERVER_STATS */
  PERMIT_AUTH, /* CLIENT_ACCESSES_BY_INDEX2 */
  PERMIT_AUTH, /* LOCAL2 */
  PERMIT_AUTH, /* NTP_DATA */
  PERMIT_AUTH, /* ADD_SERVER2 */
  PERMIT_AUTH, /* ADD_PEER2 */
  PERMIT_AUTH, /* ADD_SERVER3 */
  PERMIT_AUTH, /* ADD_PEER3 */
  PERMIT_AUTH, /* SHUTDOWN */
  PERMIT_AUTH, /* ONOFFLINE */
};

/* ================================================== */

/* This authorisation table is used for checking whether particular
   machines are allowed to make command and monitoring requests. */
static ADF_AuthTable access_auth_table;

/* ================================================== */
/* Forward prototypes */
static void read_from_cmd_socket(int sock_fd, int event, void *anything);

/* ================================================== */

static int
prepare_socket(int family, int port_number)
{
  int sock_fd;
  socklen_t my_addr_len;
  union sockaddr_all my_addr;
  IPAddr bind_address;
  int on_off = 1;

  sock_fd = socket(family, SOCK_DGRAM, 0);
  if (sock_fd < 0) {
    LOG(LOGS_ERR, "Could not open %s command socket : %s",
        UTI_SockaddrFamilyToString(family), strerror(errno));
    return -1;
  }

  /* Close on exec */
  UTI_FdSetCloexec(sock_fd);

  if (family != AF_UNIX) {
    /* Allow reuse of port number */
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &on_off, sizeof(on_off)) < 0) {
      LOG(LOGS_ERR, "Could not set reuseaddr socket options");
      /* Don't quit - we might survive anyway */
    }

#ifdef IP_FREEBIND
    /* Allow binding to address that doesn't exist yet */
    if (setsockopt(sock_fd, IPPROTO_IP, IP_FREEBIND, (char *)&on_off, sizeof(on_off)) < 0) {
      LOG(LOGS_ERR, "Could not set free bind socket option");
    }
#endif

#ifdef FEAT_IPV6
    if (family == AF_INET6) {
#ifdef IPV6_V6ONLY
      /* Receive IPv6 packets only */
      if (setsockopt(sock_fd, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&on_off, sizeof(on_off)) < 0) {
        LOG(LOGS_ERR, "Could not request IPV6_V6ONLY socket option");
      }
#endif
    }
#endif
  }

  memset(&my_addr, 0, sizeof (my_addr));

  switch (family) {
    case AF_INET:
      my_addr_len = sizeof (my_addr.in4);
      my_addr.in4.sin_family = family;
      my_addr.in4.sin_port = htons((unsigned short)port_number);

      CNF_GetBindCommandAddress(IPADDR_INET4, &bind_address);

      if (bind_address.family == IPADDR_INET4)
        my_addr.in4.sin_addr.s_addr = htonl(bind_address.addr.in4);
      else
        my_addr.in4.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
      break;
#ifdef FEAT_IPV6
    case AF_INET6:
      my_addr_len = sizeof (my_addr.in6);
      my_addr.in6.sin6_family = family;
      my_addr.in6.sin6_port = htons((unsigned short)port_number);

      CNF_GetBindCommandAddress(IPADDR_INET6, &bind_address);

      if (bind_address.family == IPADDR_INET6)
        memcpy(my_addr.in6.sin6_addr.s6_addr, bind_address.addr.in6,
            sizeof (my_addr.in6.sin6_addr.s6_addr));
      else
        my_addr.in6.sin6_addr = in6addr_loopback;
      break;
#endif
    case AF_UNIX:
      my_addr_len = sizeof (my_addr.un);
      my_addr.un.sun_family = family;
      if (snprintf(my_addr.un.sun_path, sizeof (my_addr.un.sun_path), "%s",
                   CNF_GetBindCommandPath()) >= sizeof (my_addr.un.sun_path))
        LOG_FATAL("Unix socket path too long");
      unlink(my_addr.un.sun_path);
      break;
    default:
      assert(0);
  }

  if (bind(sock_fd, &my_addr.sa, my_addr_len) < 0) {
    LOG(LOGS_ERR, "Could not bind %s command socket : %s",
        UTI_SockaddrFamilyToString(family), strerror(errno));
    close(sock_fd);
    return -1;
  }

  /* Register handler for read events on the socket */
  SCH_AddFileHandler(sock_fd, SCH_FILE_INPUT, read_from_cmd_socket, NULL);

  return sock_fd;
}

/* ================================================== */

static void
do_size_checks(void)
{
  int i, request_length, padding_length, reply_length;
  CMD_Request request;
  CMD_Reply reply;

  assert(offsetof(CMD_Request, data) == 20);
  assert(offsetof(CMD_Reply, data) == 28);

  for (i = 0; i < N_REQUEST_TYPES; i++) {
    request.version = PROTO_VERSION_NUMBER;
    request.command = htons(i);
    request_length = PKL_CommandLength(&request);
    padding_length = PKL_CommandPaddingLength(&request);
    if (padding_length > MAX_PADDING_LENGTH || padding_length > request_length ||
        request_length > sizeof (CMD_Request) ||
        (request_length && request_length < offsetof(CMD_Request, data)))
      assert(0);
  }

  for (i = 1; i < N_REPLY_TYPES; i++) {
    reply.reply = htons(i);
    reply.status = STT_SUCCESS;
    reply_length = PKL_ReplyLength(&reply);
    if ((reply_length && reply_length < offsetof(CMD_Reply, data)) ||
        reply_length > sizeof (CMD_Reply))
      assert(0);
  }
}

/* ================================================== */

void
CAM_Initialise(int family)
{
  int port_number;

  assert(!initialised);
  assert(sizeof (permissions) / sizeof (permissions[0]) == N_REQUEST_TYPES);
  do_size_checks();

  initialised = 1;
  sock_fdu = -1;
  port_number = CNF_GetCommandPort();

  if (port_number && (family == IPADDR_UNSPEC || family == IPADDR_INET4))
    sock_fd4 = prepare_socket(AF_INET, port_number);
  else
    sock_fd4 = -1;
#ifdef FEAT_IPV6
  if (port_number && (family == IPADDR_UNSPEC || family == IPADDR_INET6))
    sock_fd6 = prepare_socket(AF_INET6, port_number);
  else
    sock_fd6 = -1;
#endif

  if (port_number && sock_fd4 < 0
#ifdef FEAT_IPV6
      && sock_fd6 < 0
#endif
      ) {
    LOG_FATAL("Could not open any command socket");
  }

  access_auth_table = ADF_CreateTable();

}

/* ================================================== */

void
CAM_Finalise(void)
{
  if (sock_fdu >= 0) {
    SCH_RemoveFileHandler(sock_fdu);
    close(sock_fdu);
    unlink(CNF_GetBindCommandPath());
  }
  sock_fdu = -1;
  if (sock_fd4 >= 0) {
    SCH_RemoveFileHandler(sock_fd4);
    close(sock_fd4);
  }
  sock_fd4 = -1;
#ifdef FEAT_IPV6
  if (sock_fd6 >= 0) {
    SCH_RemoveFileHandler(sock_fd6);
    close(sock_fd6);
  }
  sock_fd6 = -1;
#endif

  ADF_DestroyTable(access_auth_table);

  initialised = 0;
}

/* ================================================== */

void
CAM_OpenUnixSocket(void)
{
  /* This is separated from CAM_Initialise() as it needs to be called when
     the process has already dropped the root privileges */
  if (CNF_GetBindCommandPath()[0])
    sock_fdu = prepare_socket(AF_UNIX, 0);
}

/* ================================================== */

static void
transmit_reply(CMD_Reply *msg, union sockaddr_all *where_to)
{
  int status;
  int tx_message_length;
  int sock_fd;
  socklen_t addrlen;

  switch (where_to->sa.sa_family) {
    case AF_INET:
      sock_fd = sock_fd4;
      addrlen = sizeof (where_to->in4);
      break;
#ifdef FEAT_IPV6
    case AF_INET6:
      sock_fd = sock_fd6;
      addrlen = sizeof (where_to->in6);
      break;
#endif
    case AF_UNIX:
      sock_fd = sock_fdu;
      addrlen = sizeof (where_to->un);
      break;
    default:
      assert(0);
  }

  tx_message_length = PKL_ReplyLength(msg);
  status = sendto(sock_fd, (void *) msg, tx_message_length, 0,
                  &where_to->sa, addrlen);

  if (status < 0) {
    DEBUG_LOG("Could not send to %s fd %d : %s",
              UTI_SockaddrToString(&where_to->sa), sock_fd, strerror(errno));
    return;
  }

  DEBUG_LOG("Sent %d bytes to %s fd %d", status,
            UTI_SockaddrToString(&where_to->sa), sock_fd);
}
  
/* ================================================== */

static void
handle_dump(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  SRC_DumpSources();
}

/* ================================================== */

static void
handle_online(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  IPAddr address, mask;

  UTI_IPNetworkToHost(&rx_message->data.online.mask, &mask);
  UTI_IPNetworkToHost(&rx_message->data.online.address, &address);
  if (!NSR_SetConnectivity(&mask, &address, SRC_ONLINE))
    tx_message->status = htons(STT_NOSUCHSOURCE);
}

/* ================================================== */

static void
handle_offline(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  IPAddr address, mask;

  UTI_IPNetworkToHost(&rx_message->data.offline.mask, &mask);
  UTI_IPNetworkToHost(&rx_message->data.offline.address, &address);
  if (!NSR_SetConnectivity(&mask, &address, SRC_OFFLINE))
    tx_message->status = htons(STT_NOSUCHSOURCE);
}

/* ================================================== */

static void
handle_onoffline(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  IPAddr address, mask;

  address.family = mask.family = IPADDR_UNSPEC;
  if (!NSR_SetConnectivity(&mask, &address, SRC_MAYBE_ONLINE))
    ;
}

/* ================================================== */

static void
handle_burst(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  IPAddr address, mask;

  UTI_IPNetworkToHost(&rx_message->data.burst.mask, &mask);
  UTI_IPNetworkToHost(&rx_message->data.burst.address, &address);
  if (!NSR_InitiateSampleBurst(ntohl(rx_message->data.burst.n_good_samples),
                               ntohl(rx_message->data.burst.n_total_samples),
                               &mask, &address))
    tx_message->status = htons(STT_NOSUCHSOURCE);
}

/* ================================================== */

static void
handle_modify_minpoll(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  IPAddr address;

  UTI_IPNetworkToHost(&rx_message->data.modify_minpoll.address, &address);
  if (!NSR_ModifyMinpoll(&address,
                         ntohl(rx_message->data.modify_minpoll.new_minpoll)))
    tx_message->status = htons(STT_NOSUCHSOURCE);
}

/* ================================================== */

static void
handle_modify_maxpoll(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  IPAddr address;

  UTI_IPNetworkToHost(&rx_message->data.modify_minpoll.address, &address);
  if (!NSR_ModifyMaxpoll(&address,
                         ntohl(rx_message->data.modify_minpoll.new_minpoll)))
    tx_message->status = htons(STT_NOSUCHSOURCE);
}

/* ================================================== */

static void
handle_modify_maxdelay(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  IPAddr address;

  UTI_IPNetworkToHost(&rx_message->data.modify_maxdelay.address, &address);
  if (!NSR_ModifyMaxdelay(&address,
        UTI_FloatNetworkToHost(rx_message->data.modify_maxdelay.new_max_delay)))
    tx_message->status = htons(STT_NOSUCHSOURCE);
}

/* ================================================== */

static void
handle_modify_maxdelayratio(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  IPAddr address;

  UTI_IPNetworkToHost(&rx_message->data.modify_maxdelayratio.address, &address);
  if (!NSR_ModifyMaxdelayratio(&address,
        UTI_FloatNetworkToHost(rx_message->data.modify_maxdelayratio.new_max_delay_ratio)))
    tx_message->status = htons(STT_NOSUCHSOURCE);
}

/* ================================================== */

static void
handle_modify_maxdelaydevratio(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  IPAddr address;

  UTI_IPNetworkToHost(&rx_message->data.modify_maxdelaydevratio.address, &address);
  if (!NSR_ModifyMaxdelaydevratio(&address,
        UTI_FloatNetworkToHost(rx_message->data.modify_maxdelaydevratio.new_max_delay_dev_ratio)))
    tx_message->status = htons(STT_NOSUCHSOURCE);
}

/* ================================================== */

static void
handle_modify_minstratum(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  IPAddr address;

  UTI_IPNetworkToHost(&rx_message->data.modify_minpoll.address, &address);
  if (!NSR_ModifyMinstratum(&address,
                            ntohl(rx_message->data.modify_minstratum.new_min_stratum)))
    tx_message->status = htons(STT_NOSUCHSOURCE);
}

/* ================================================== */

static void
handle_modify_polltarget(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  IPAddr address;

  UTI_IPNetworkToHost(&rx_message->data.modify_polltarget.address, &address);
  if (!NSR_ModifyPolltarget(&address,
                            ntohl(rx_message->data.modify_polltarget.new_poll_target)))
    tx_message->status = htons(STT_NOSUCHSOURCE);
}

/* ================================================== */

static void
handle_modify_maxupdateskew(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  REF_ModifyMaxupdateskew(UTI_FloatNetworkToHost(rx_message->data.modify_maxupdateskew.new_max_update_skew));
}

/* ================================================== */

static void
handle_modify_makestep(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  REF_ModifyMakestep(ntohl(rx_message->data.modify_makestep.limit),
                     UTI_FloatNetworkToHost(rx_message->data.modify_makestep.threshold));
}

/* ================================================== */

static void
handle_settime(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  struct timespec ts;
  double offset, dfreq_ppm, new_afreq_ppm;
  UTI_TimespecNetworkToHost(&rx_message->data.settime.ts, &ts);
  if (!MNL_IsEnabled()) {
    tx_message->status = htons(STT_NOTENABLED);
  } else if (MNL_AcceptTimestamp(&ts, &offset, &dfreq_ppm, &new_afreq_ppm)) {
    tx_message->reply = htons(RPY_MANUAL_TIMESTAMP2);
    tx_message->data.manual_timestamp.offset = UTI_FloatHostToNetwork(offset);
    tx_message->data.manual_timestamp.dfreq_ppm = UTI_FloatHostToNetwork(dfreq_ppm);
    tx_message->data.manual_timestamp.new_afreq_ppm = UTI_FloatHostToNetwork(new_afreq_ppm);
  } else {
    tx_message->status = htons(STT_FAILED);
  }
}

/* ================================================== */

static void
handle_local(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  if (ntohl(rx_message->data.local.on_off)) {
    REF_EnableLocal(ntohl(rx_message->data.local.stratum),
                    UTI_FloatNetworkToHost(rx_message->data.local.distance),
                    ntohl(rx_message->data.local.orphan));
  } else {
    REF_DisableLocal();
  }
}

/* ================================================== */

static void
handle_manual(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  int option;
  option = ntohl(rx_message->data.manual.option);
  switch (option) {
    case 0:
      MNL_Disable();
      break;
    case 1:
      MNL_Enable();
      break;
    case 2:
      MNL_Reset();
      break;
    default:
      tx_message->status = htons(STT_INVALID);
      break;
  }
}

/* ================================================== */

static void
handle_n_sources(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  int n_sources;
  n_sources = SRC_ReadNumberOfSources();
  tx_message->reply = htons(RPY_N_SOURCES);
  tx_message->data.n_sources.n_sources = htonl(n_sources);
}

/* ================================================== */

static void
handle_source_data(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  RPT_SourceReport report;
  struct timespec now_corr;

  /* Get data */
  SCH_GetLastEventTime(&now_corr, NULL, NULL);
  if (SRC_ReportSource(ntohl(rx_message->data.source_data.index), &report, &now_corr)) {
    switch (SRC_GetType(ntohl(rx_message->data.source_data.index))) {
      case SRC_NTP:
        NSR_ReportSource(&report, &now_corr);
        break;
      case SRC_REFCLOCK:
        RCL_ReportSource(&report, &now_corr);
        break;
    }
    
    tx_message->reply  = htons(RPY_SOURCE_DATA);
    
    UTI_IPHostToNetwork(&report.ip_addr, &tx_message->data.source_data.ip_addr);
    tx_message->data.source_data.stratum = htons(report.stratum);
    tx_message->data.source_data.poll    = htons(report.poll);
    switch (report.state) {
      case RPT_SYNC:
        tx_message->data.source_data.state   = htons(RPY_SD_ST_SYNC);
        break;
      case RPT_UNREACH:
        tx_message->data.source_data.state   = htons(RPY_SD_ST_UNREACH);
        break;
      case RPT_FALSETICKER:
        tx_message->data.source_data.state   = htons(RPY_SD_ST_FALSETICKER);
        break;
      case RPT_JITTERY:
        tx_message->data.source_data.state   = htons(RPY_SD_ST_JITTERY);
        break;
      case RPT_CANDIDATE:
        tx_message->data.source_data.state   = htons(RPY_SD_ST_CANDIDATE);
        break;
      case RPT_OUTLIER:
        tx_message->data.source_data.state   = htons(RPY_SD_ST_OUTLIER);
        break;
    }
    switch (report.mode) {
      case RPT_NTP_CLIENT:
        tx_message->data.source_data.mode    = htons(RPY_SD_MD_CLIENT);
        break;
      case RPT_NTP_PEER:
        tx_message->data.source_data.mode    = htons(RPY_SD_MD_PEER);
        break;
      case RPT_LOCAL_REFERENCE:
        tx_message->data.source_data.mode    = htons(RPY_SD_MD_REF);
        break;
    }
    tx_message->data.source_data.flags =
                  htons((report.sel_options & SRC_SELECT_PREFER ? RPY_SD_FLAG_PREFER : 0) |
                        (report.sel_options & SRC_SELECT_NOSELECT ? RPY_SD_FLAG_NOSELECT : 0) |
                        (report.sel_options & SRC_SELECT_TRUST ? RPY_SD_FLAG_TRUST : 0) |
                        (report.sel_options & SRC_SELECT_REQUIRE ? RPY_SD_FLAG_REQUIRE : 0));
    tx_message->data.source_data.reachability = htons(report.reachability);
    tx_message->data.source_data.since_sample = htonl(report.latest_meas_ago);
    tx_message->data.source_data.orig_latest_meas = UTI_FloatHostToNetwork(report.orig_latest_meas);
    tx_message->data.source_data.latest_meas = UTI_FloatHostToNetwork(report.latest_meas);
    tx_message->data.source_data.latest_meas_err = UTI_FloatHostToNetwork(report.latest_meas_err);
  } else {
    tx_message->status = htons(STT_NOSUCHSOURCE);
  }
}

/* ================================================== */

static void
handle_rekey(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  KEY_Reload();
}

/* ================================================== */

static void
handle_allowdeny(CMD_Request *rx_message, CMD_Reply *tx_message, int allow, int all)
{
  IPAddr ip;
  int subnet_bits;

  UTI_IPNetworkToHost(&rx_message->data.allow_deny.ip, &ip);
  subnet_bits = ntohl(rx_message->data.allow_deny.subnet_bits);
  if (!NCR_AddAccessRestriction(&ip, subnet_bits, allow, all))
    tx_message->status = htons(STT_BADSUBNET);
}

/* ================================================== */

static void
handle_cmdallowdeny(CMD_Request *rx_message, CMD_Reply *tx_message, int allow, int all)
{
  IPAddr ip;
  int subnet_bits;

  UTI_IPNetworkToHost(&rx_message->data.allow_deny.ip, &ip);
  subnet_bits = ntohl(rx_message->data.allow_deny.subnet_bits);
  if (!CAM_AddAccessRestriction(&ip, subnet_bits, allow, all))
    tx_message->status = htons(STT_BADSUBNET);
}

/* ================================================== */

static void
handle_accheck(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  IPAddr ip;
  UTI_IPNetworkToHost(&rx_message->data.ac_check.ip, &ip);
  if (NCR_CheckAccessRestriction(&ip)) {
    tx_message->status = htons(STT_ACCESSALLOWED);
  } else {
    tx_message->status = htons(STT_ACCESSDENIED);
  }
}

/* ================================================== */

static void
handle_cmdaccheck(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  IPAddr ip;
  UTI_IPNetworkToHost(&rx_message->data.ac_check.ip, &ip);
  if (CAM_CheckAccessRestriction(&ip)) {
    tx_message->status = htons(STT_ACCESSALLOWED);
  } else {
    tx_message->status = htons(STT_ACCESSDENIED);
  }
}

/* ================================================== */

static void
handle_add_source(NTP_Source_Type type, CMD_Request *rx_message, CMD_Reply *tx_message)
{
  NTP_Remote_Address rem_addr;
  SourceParameters params;
  NSR_Status status;
  
  UTI_IPNetworkToHost(&rx_message->data.ntp_source.ip_addr, &rem_addr.ip_addr);
  rem_addr.port = (unsigned short)(ntohl(rx_message->data.ntp_source.port));
  params.minpoll = ntohl(rx_message->data.ntp_source.minpoll);
  params.maxpoll = ntohl(rx_message->data.ntp_source.maxpoll);
  params.presend_minpoll = ntohl(rx_message->data.ntp_source.presend_minpoll);
  params.min_stratum = ntohl(rx_message->data.ntp_source.min_stratum);
  params.poll_target = ntohl(rx_message->data.ntp_source.poll_target);
  params.version = ntohl(rx_message->data.ntp_source.version);
  params.max_sources = ntohl(rx_message->data.ntp_source.max_sources);
  params.min_samples = ntohl(rx_message->data.ntp_source.min_samples);
  params.max_samples = ntohl(rx_message->data.ntp_source.max_samples);
  params.filter_length = ntohl(rx_message->data.ntp_source.filter_length);
  params.authkey = ntohl(rx_message->data.ntp_source.authkey);
  params.max_delay = UTI_FloatNetworkToHost(rx_message->data.ntp_source.max_delay);
  params.max_delay_ratio =
    UTI_FloatNetworkToHost(rx_message->data.ntp_source.max_delay_ratio);
  params.max_delay_dev_ratio =
    UTI_FloatNetworkToHost(rx_message->data.ntp_source.max_delay_dev_ratio);
  params.min_delay = UTI_FloatNetworkToHost(rx_message->data.ntp_source.min_delay);
  params.asymmetry = UTI_FloatNetworkToHost(rx_message->data.ntp_source.asymmetry);
  params.offset = UTI_FloatNetworkToHost(rx_message->data.ntp_source.offset);

  params.connectivity = ntohl(rx_message->data.ntp_source.flags) & REQ_ADDSRC_ONLINE ?
                        SRC_ONLINE : SRC_OFFLINE;
  params.auto_offline = ntohl(rx_message->data.ntp_source.flags) & REQ_ADDSRC_AUTOOFFLINE ? 1 : 0;
  params.iburst = ntohl(rx_message->data.ntp_source.flags) & REQ_ADDSRC_IBURST ? 1 : 0;
  params.interleaved = ntohl(rx_message->data.ntp_source.flags) & REQ_ADDSRC_INTERLEAVED ? 1 : 0;
  params.burst = ntohl(rx_message->data.ntp_source.flags) & REQ_ADDSRC_BURST ? 1 : 0;
  params.sel_options =
    (ntohl(rx_message->data.ntp_source.flags) & REQ_ADDSRC_PREFER ? SRC_SELECT_PREFER : 0) |
    (ntohl(rx_message->data.ntp_source.flags) & REQ_ADDSRC_NOSELECT ? SRC_SELECT_NOSELECT : 0) |
    (ntohl(rx_message->data.ntp_source.flags) & REQ_ADDSRC_TRUST ? SRC_SELECT_TRUST : 0) |
    (ntohl(rx_message->data.ntp_source.flags) & REQ_ADDSRC_REQUIRE ? SRC_SELECT_REQUIRE : 0);

  status = NSR_AddSource(&rem_addr, type, &params);
  switch (status) {
    case NSR_Success:
      break;
    case NSR_AlreadyInUse:
      tx_message->status = htons(STT_SOURCEALREADYKNOWN);
      break;
    case NSR_TooManySources:
      tx_message->status = htons(STT_TOOMANYSOURCES);
      break;
    case NSR_InvalidAF:
      tx_message->status = htons(STT_INVALIDAF);
      break;
    case NSR_NoSuchSource:
      assert(0);
      break;
  }
}

/* ================================================== */

static void
handle_del_source(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  NTP_Remote_Address rem_addr;
  NSR_Status status;
  
  UTI_IPNetworkToHost(&rx_message->data.del_source.ip_addr, &rem_addr.ip_addr);
  rem_addr.port = 0;
  
  status = NSR_RemoveSource(&rem_addr);
  switch (status) {
    case NSR_Success:
      break;
    case NSR_NoSuchSource:
      tx_message->status = htons(STT_NOSUCHSOURCE);
      break;
    case NSR_TooManySources:
    case NSR_AlreadyInUse:
    case NSR_InvalidAF:
      assert(0);
      break;
  }
}

/* ================================================== */

static void
handle_writertc(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  switch (RTC_WriteParameters()) {
    case RTC_ST_OK:
      break;
    case RTC_ST_NODRV:
      tx_message->status = htons(STT_NORTC);
      break;
    case RTC_ST_BADFILE:
      tx_message->status = htons(STT_BADRTCFILE);
      break;
  }
}

/* ================================================== */

static void
handle_dfreq(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  double dfreq;
  dfreq = UTI_FloatNetworkToHost(rx_message->data.dfreq.dfreq);
  LCL_AccumulateDeltaFrequency(dfreq * 1.0e-6);
  LOG(LOGS_INFO, "Accumulated delta freq of %.3fppm", dfreq);
}

/* ================================================== */

static void
handle_doffset(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  long sec, usec;
  double doffset;
  sec = (int32_t)ntohl(rx_message->data.doffset.sec);
  usec = (int32_t)ntohl(rx_message->data.doffset.usec);
  doffset = (double) sec + 1.0e-6 * (double) usec;
  LOG(LOGS_INFO, "Accumulated delta offset of %.6f seconds", doffset);
  LCL_AccumulateOffset(doffset, 0.0);
}

/* ================================================== */

static void
handle_tracking(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  RPT_TrackingReport rpt;

  REF_GetTrackingReport(&rpt);
  tx_message->reply  = htons(RPY_TRACKING);
  tx_message->data.tracking.ref_id = htonl(rpt.ref_id);
  UTI_IPHostToNetwork(&rpt.ip_addr, &tx_message->data.tracking.ip_addr);
  tx_message->data.tracking.stratum = htons(rpt.stratum);
  tx_message->data.tracking.leap_status = htons(rpt.leap_status);
  UTI_TimespecHostToNetwork(&rpt.ref_time, &tx_message->data.tracking.ref_time);
  tx_message->data.tracking.current_correction = UTI_FloatHostToNetwork(rpt.current_correction);
  tx_message->data.tracking.last_offset = UTI_FloatHostToNetwork(rpt.last_offset);
  tx_message->data.tracking.rms_offset = UTI_FloatHostToNetwork(rpt.rms_offset);
  tx_message->data.tracking.freq_ppm = UTI_FloatHostToNetwork(rpt.freq_ppm);
  tx_message->data.tracking.resid_freq_ppm = UTI_FloatHostToNetwork(rpt.resid_freq_ppm);
  tx_message->data.tracking.skew_ppm = UTI_FloatHostToNetwork(rpt.skew_ppm);
  tx_message->data.tracking.root_delay = UTI_FloatHostToNetwork(rpt.root_delay);
  tx_message->data.tracking.root_dispersion = UTI_FloatHostToNetwork(rpt.root_dispersion);
  tx_message->data.tracking.last_update_interval = UTI_FloatHostToNetwork(rpt.last_update_interval);
}

/* ================================================== */

static void
handle_smoothing(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  RPT_SmoothingReport report;
  struct timespec now;

  SCH_GetLastEventTime(&now, NULL, NULL);

  if (!SMT_GetSmoothingReport(&report, &now)) {
    tx_message->status = htons(STT_NOTENABLED);
    return;
  }

  tx_message->reply  = htons(RPY_SMOOTHING);
  tx_message->data.smoothing.flags = htonl((report.active ? RPY_SMT_FLAG_ACTIVE : 0) |
                                           (report.leap_only ? RPY_SMT_FLAG_LEAPONLY : 0));
  tx_message->data.smoothing.offset = UTI_FloatHostToNetwork(report.offset);
  tx_message->data.smoothing.freq_ppm = UTI_FloatHostToNetwork(report.freq_ppm);
  tx_message->data.smoothing.wander_ppm = UTI_FloatHostToNetwork(report.wander_ppm);
  tx_message->data.smoothing.last_update_ago = UTI_FloatHostToNetwork(report.last_update_ago);
  tx_message->data.smoothing.remaining_time = UTI_FloatHostToNetwork(report.remaining_time);
}

/* ================================================== */

static void
handle_smoothtime(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  struct timespec now;
  int option;

  if (!SMT_IsEnabled()) {
    tx_message->status = htons(STT_NOTENABLED);
    return;
  }

  option = ntohl(rx_message->data.smoothtime.option);
  SCH_GetLastEventTime(&now, NULL, NULL);

  switch (option) {
    case REQ_SMOOTHTIME_RESET:
      SMT_Reset(&now);
      break;
    case REQ_SMOOTHTIME_ACTIVATE:
      SMT_Activate(&now);
      break;
    default:
      tx_message->status = htons(STT_INVALID);
      break;
  }
}

/* ================================================== */

static void
handle_sourcestats(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  int status;
  RPT_SourcestatsReport report;
  struct timespec now_corr;

  SCH_GetLastEventTime(&now_corr, NULL, NULL);
  status = SRC_ReportSourcestats(ntohl(rx_message->data.sourcestats.index),
                                 &report, &now_corr);

  if (status) {
    tx_message->reply = htons(RPY_SOURCESTATS);
    tx_message->data.sourcestats.ref_id = htonl(report.ref_id);
    UTI_IPHostToNetwork(&report.ip_addr, &tx_message->data.sourcestats.ip_addr);
    tx_message->data.sourcestats.n_samples = htonl(report.n_samples);
    tx_message->data.sourcestats.n_runs = htonl(report.n_runs);
    tx_message->data.sourcestats.span_seconds = htonl(report.span_seconds);
    tx_message->data.sourcestats.resid_freq_ppm = UTI_FloatHostToNetwork(report.resid_freq_ppm);
    tx_message->data.sourcestats.skew_ppm = UTI_FloatHostToNetwork(report.skew_ppm);
    tx_message->data.sourcestats.sd = UTI_FloatHostToNetwork(report.sd);
    tx_message->data.sourcestats.est_offset = UTI_FloatHostToNetwork(report.est_offset);
    tx_message->data.sourcestats.est_offset_err = UTI_FloatHostToNetwork(report.est_offset_err);
  } else {
    tx_message->status = htons(STT_NOSUCHSOURCE);
  }
}

/* ================================================== */

static void
handle_rtcreport(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  int status;
  RPT_RTC_Report report;
  status = RTC_GetReport(&report);
  if (status) {
    tx_message->reply  = htons(RPY_RTC);
    UTI_TimespecHostToNetwork(&report.ref_time, &tx_message->data.rtc.ref_time);
    tx_message->data.rtc.n_samples = htons(report.n_samples);
    tx_message->data.rtc.n_runs = htons(report.n_runs);
    tx_message->data.rtc.span_seconds = htonl(report.span_seconds);
    tx_message->data.rtc.rtc_seconds_fast = UTI_FloatHostToNetwork(report.rtc_seconds_fast);
    tx_message->data.rtc.rtc_gain_rate_ppm = UTI_FloatHostToNetwork(report.rtc_gain_rate_ppm);
  } else {
    tx_message->status = htons(STT_NORTC);
  }
}

/* ================================================== */

static void
handle_trimrtc(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  if (!RTC_Trim())
    tx_message->status = htons(STT_NORTC);
}

/* ================================================== */

static void
handle_cyclelogs(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  LOG_CycleLogFiles();
}

/* ================================================== */

static void
handle_client_accesses_by_index(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  RPT_ClientAccessByIndex_Report report;
  RPY_ClientAccesses_Client *client;
  int n_indices;
  uint32_t i, j, req_first_index, req_n_clients;
  struct timespec now;

  SCH_GetLastEventTime(&now, NULL, NULL);

  req_first_index = ntohl(rx_message->data.client_accesses_by_index.first_index);
  req_n_clients = ntohl(rx_message->data.client_accesses_by_index.n_clients);
  if (req_n_clients > MAX_CLIENT_ACCESSES)
    req_n_clients = MAX_CLIENT_ACCESSES;

  n_indices = CLG_GetNumberOfIndices();
  if (n_indices < 0) {
    tx_message->status = htons(STT_INACTIVE);
    return;
  }

  tx_message->reply = htons(RPY_CLIENT_ACCESSES_BY_INDEX2);
  tx_message->data.client_accesses_by_index.n_indices = htonl(n_indices);

  for (i = req_first_index, j = 0; i < (uint32_t)n_indices && j < req_n_clients; i++) {
    if (!CLG_GetClientAccessReportByIndex(i, &report, &now))
      continue;

    client = &tx_message->data.client_accesses_by_index.clients[j++];

    UTI_IPHostToNetwork(&report.ip_addr, &client->ip);
    client->ntp_hits = htonl(report.ntp_hits);
    client->cmd_hits = htonl(report.cmd_hits);
    client->ntp_drops = htonl(report.ntp_drops);
    client->cmd_drops = htonl(report.cmd_drops);
    client->ntp_interval = report.ntp_interval;
    client->cmd_interval = report.cmd_interval;
    client->ntp_timeout_interval = report.ntp_timeout_interval;
    client->last_ntp_hit_ago = htonl(report.last_ntp_hit_ago);
    client->last_cmd_hit_ago = htonl(report.last_cmd_hit_ago);
  }

  tx_message->data.client_accesses_by_index.next_index = htonl(i);
  tx_message->data.client_accesses_by_index.n_clients = htonl(j);
}

/* ================================================== */

static void
handle_manual_list(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  int n_samples;
  int i;
  RPY_ManualListSample *sample;
  RPT_ManualSamplesReport report[MAX_MANUAL_LIST_SAMPLES];

  tx_message->reply = htons(RPY_MANUAL_LIST2);
  
  MNL_ReportSamples(report, MAX_MANUAL_LIST_SAMPLES, &n_samples);
  tx_message->data.manual_list.n_samples = htonl(n_samples);

  for (i=0; i<n_samples; i++) {
    sample = &tx_message->data.manual_list.samples[i];
    UTI_TimespecHostToNetwork(&report[i].when, &sample->when);
    sample->slewed_offset = UTI_FloatHostToNetwork(report[i].slewed_offset);
    sample->orig_offset = UTI_FloatHostToNetwork(report[i].orig_offset);
    sample->residual = UTI_FloatHostToNetwork(report[i].residual);
  }
}

/* ================================================== */

static void
handle_manual_delete(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  int index;

  index = ntohl(rx_message->data.manual_delete.index);
  if (!MNL_DeleteSample(index))
    tx_message->status = htons(STT_BADSAMPLE);
}  

/* ================================================== */

static void
handle_make_step(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  if (!LCL_MakeStep())
    tx_message->status = htons(STT_FAILED);
}

/* ================================================== */

static void
handle_activity(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  RPT_ActivityReport report;
  NSR_GetActivityReport(&report);
  tx_message->data.activity.online = htonl(report.online);
  tx_message->data.activity.offline = htonl(report.offline);
  tx_message->data.activity.burst_online = htonl(report.burst_online);
  tx_message->data.activity.burst_offline = htonl(report.burst_offline);
  tx_message->data.activity.unresolved = htonl(report.unresolved);
  tx_message->reply = htons(RPY_ACTIVITY);
}

/* ================================================== */

static void
handle_reselect_distance(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  double dist;
  dist = UTI_FloatNetworkToHost(rx_message->data.reselect_distance.distance);
  SRC_SetReselectDistance(dist);
}

/* ================================================== */

static void
handle_reselect(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  SRC_ReselectSource();
}

/* ================================================== */

static void
handle_refresh(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  NSR_RefreshAddresses();
}

/* ================================================== */

static void
handle_server_stats(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  RPT_ServerStatsReport report;

  CLG_GetServerStatsReport(&report);
  tx_message->reply = htons(RPY_SERVER_STATS);
  tx_message->data.server_stats.ntp_hits = htonl(report.ntp_hits);
  tx_message->data.server_stats.cmd_hits = htonl(report.cmd_hits);
  tx_message->data.server_stats.ntp_drops = htonl(report.ntp_drops);
  tx_message->data.server_stats.cmd_drops = htonl(report.cmd_drops);
  tx_message->data.server_stats.log_drops = htonl(report.log_drops);
}

/* ================================================== */

static void
handle_ntp_data(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  RPT_NTPReport report;

  UTI_IPNetworkToHost(&rx_message->data.ntp_data.ip_addr, &report.remote_addr);

  if (!NSR_GetNTPReport(&report)) {
    tx_message->status = htons(STT_NOSUCHSOURCE);
    return;
  }

  tx_message->reply = htons(RPY_NTP_DATA);
  UTI_IPHostToNetwork(&report.remote_addr, &tx_message->data.ntp_data.remote_addr);
  UTI_IPHostToNetwork(&report.local_addr, &tx_message->data.ntp_data.local_addr);
  tx_message->data.ntp_data.remote_port = htons(report.remote_port);
  tx_message->data.ntp_data.leap = report.leap;
  tx_message->data.ntp_data.version = report.version;
  tx_message->data.ntp_data.mode = report.mode;
  tx_message->data.ntp_data.stratum = report.stratum;
  tx_message->data.ntp_data.poll = report.poll;
  tx_message->data.ntp_data.precision = report.precision;
  tx_message->data.ntp_data.root_delay = UTI_FloatHostToNetwork(report.root_delay);
  tx_message->data.ntp_data.root_dispersion = UTI_FloatHostToNetwork(report.root_dispersion);
  tx_message->data.ntp_data.ref_id = htonl(report.ref_id);
  UTI_TimespecHostToNetwork(&report.ref_time, &tx_message->data.ntp_data.ref_time);
  tx_message->data.ntp_data.offset = UTI_FloatHostToNetwork(report.offset);
  tx_message->data.ntp_data.peer_delay = UTI_FloatHostToNetwork(report.peer_delay);
  tx_message->data.ntp_data.peer_dispersion = UTI_FloatHostToNetwork(report.peer_dispersion);
  tx_message->data.ntp_data.response_time = UTI_FloatHostToNetwork(report.response_time);
  tx_message->data.ntp_data.jitter_asymmetry = UTI_FloatHostToNetwork(report.jitter_asymmetry);
  tx_message->data.ntp_data.flags = htons((report.tests & RPY_NTP_FLAGS_TESTS) |
                                          (report.interleaved ? RPY_NTP_FLAG_INTERLEAVED : 0) |
                                          (report.authenticated ? RPY_NTP_FLAG_AUTHENTICATED : 0));
  tx_message->data.ntp_data.tx_tss_char = report.tx_tss_char;
  tx_message->data.ntp_data.rx_tss_char = report.rx_tss_char;
  tx_message->data.ntp_data.total_tx_count = htonl(report.total_tx_count);
  tx_message->data.ntp_data.total_rx_count = htonl(report.total_rx_count);
  tx_message->data.ntp_data.total_valid_count = htonl(report.total_valid_count);
  memset(tx_message->data.ntp_data.reserved, 0xff, sizeof (tx_message->data.ntp_data.reserved));
}

/* ================================================== */

static void
handle_shutdown(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  LOG(LOGS_INFO, "Received shutdown command");
  SCH_QuitProgram();
}

/* ================================================== */
/* Read a packet and process it */

static void
read_from_cmd_socket(int sock_fd, int event, void *anything)
{
  CMD_Request rx_message;
  CMD_Reply tx_message;
  int status, read_length, expected_length, rx_message_length;
  int localhost, allowed, log_index;
  union sockaddr_all where_from;
  socklen_t from_length;
  IPAddr remote_ip;
  unsigned short remote_port, rx_command;
  struct timespec now, cooked_now;

  rx_message_length = sizeof(rx_message);
  from_length = sizeof(where_from);

  status = recvfrom(sock_fd, (char *)&rx_message, rx_message_length, 0,
                    &where_from.sa, &from_length);

  if (status < 0) {
    LOG(LOGS_WARN, "Error [%s] reading from control socket %d",
        strerror(errno), sock_fd);
    return;
  }

  if (from_length > sizeof (where_from) ||
      from_length <= sizeof (where_from.sa.sa_family)) {
    DEBUG_LOG("Read command packet without source address");
    return;
  }

  read_length = status;

  /* Get current time cheaply */
  SCH_GetLastEventTime(&cooked_now, NULL, &now);

  UTI_SockaddrToIPAndPort(&where_from.sa, &remote_ip, &remote_port);

  /* Check if it's from localhost (127.0.0.1, ::1, or Unix domain) */
  switch (remote_ip.family) {
    case IPADDR_INET4:
      assert(sock_fd == sock_fd4);
      localhost = remote_ip.addr.in4 == INADDR_LOOPBACK;
      break;
#ifdef FEAT_IPV6
    case IPADDR_INET6:
      assert(sock_fd == sock_fd6);
      localhost = !memcmp(remote_ip.addr.in6, &in6addr_loopback,
                          sizeof (in6addr_loopback));
      break;
#endif
    case IPADDR_UNSPEC:
      /* This should be the Unix domain socket */
      if (where_from.sa.sa_family != AF_UNIX)
        return;
      assert(sock_fd == sock_fdu);
      localhost = 1;
      break;
    default:
      assert(0);
  }

  DEBUG_LOG("Received %d bytes from %s fd %d",
            status, UTI_SockaddrToString(&where_from.sa), sock_fd);

  if (!(localhost || ADF_IsAllowed(access_auth_table, &remote_ip))) {
    /* The client is not allowed access, so don't waste any more time
       on him.  Note that localhost is always allowed access
       regardless of the defined access rules - otherwise, we could
       shut ourselves out completely! */
    return;
  }

  if (read_length < offsetof(CMD_Request, data) ||
      read_length < offsetof(CMD_Reply, data) ||
      rx_message.pkt_type != PKT_TYPE_CMD_REQUEST ||
      rx_message.res1 != 0 ||
      rx_message.res2 != 0) {

    /* We don't know how to process anything like this or an error reply
       would be larger than the request */
    DEBUG_LOG("Command packet dropped");
    return;
  }

  expected_length = PKL_CommandLength(&rx_message);
  rx_command = ntohs(rx_message.command);

  memset(&tx_message, 0, sizeof (tx_message));

  tx_message.version = PROTO_VERSION_NUMBER;
  tx_message.pkt_type = PKT_TYPE_CMD_REPLY;
  tx_message.command = rx_message.command;
  tx_message.reply = htons(RPY_NULL);
  tx_message.status = htons(STT_SUCCESS);
  tx_message.sequence = rx_message.sequence;

  if (rx_message.version != PROTO_VERSION_NUMBER) {
    DEBUG_LOG("Command packet has invalid version (%d != %d)",
              rx_message.version, PROTO_VERSION_NUMBER);

    if (rx_message.version >= PROTO_VERSION_MISMATCH_COMPAT_SERVER) {
      tx_message.status = htons(STT_BADPKTVERSION);
      transmit_reply(&tx_message, &where_from);
    }
    return;
  }

  if (rx_command >= N_REQUEST_TYPES ||
      expected_length < (int)offsetof(CMD_Request, data)) {
    DEBUG_LOG("Command packet has invalid command %d", rx_command);

    tx_message.status = htons(STT_INVALID);
    transmit_reply(&tx_message, &where_from);
    return;
  }

  if (read_length < expected_length) {
    DEBUG_LOG("Command packet is too short (%d < %d)", read_length,
              expected_length);

    tx_message.status = htons(STT_BADPKTLENGTH);
    transmit_reply(&tx_message, &where_from);
    return;
  }

  /* OK, we have a valid message.  Now dispatch on message type and process it. */

  log_index = CLG_LogCommandAccess(&remote_ip, &cooked_now);

  /* Don't reply to all requests from hosts other than localhost if the rate
     is excessive */
  if (!localhost && log_index >= 0 && CLG_LimitCommandResponseRate(log_index)) {
      DEBUG_LOG("Command packet discarded to limit response rate");
      return;
  }

  if (rx_command >= N_REQUEST_TYPES) {
    /* This should be already handled */
    assert(0);
  } else {
    /* Check level of authority required to issue the command.  All commands
       from the Unix domain socket (which is accessible only by the root and
       chrony user/group) are allowed. */
    if (where_from.sa.sa_family == AF_UNIX) {
      assert(sock_fd == sock_fdu);
      allowed = 1;
    } else {
      switch (permissions[rx_command]) {
        case PERMIT_AUTH:
          allowed = 0;
          break;
        case PERMIT_LOCAL:
          allowed = localhost;
          break;
        case PERMIT_OPEN:
          allowed = 1;
          break;
        default:
          assert(0);
          allowed = 0;
      }
    }

    if (allowed) {
      switch(rx_command) {
        case REQ_NULL:
          /* Do nothing */
          break;

        case REQ_DUMP:
          handle_dump(&rx_message, &tx_message);
          break;

        case REQ_ONLINE:
          handle_online(&rx_message, &tx_message);
          break;

        case REQ_OFFLINE:
          handle_offline(&rx_message, &tx_message);
          break;

        case REQ_BURST:
          handle_burst(&rx_message, &tx_message);
          break;

        case REQ_MODIFY_MINPOLL:
          handle_modify_minpoll(&rx_message, &tx_message);
          break;

        case REQ_MODIFY_MAXPOLL:
          handle_modify_maxpoll(&rx_message, &tx_message);
          break;

        case REQ_MODIFY_MAXDELAY:
          handle_modify_maxdelay(&rx_message, &tx_message);
          break;

        case REQ_MODIFY_MAXDELAYRATIO:
          handle_modify_maxdelayratio(&rx_message, &tx_message);
          break;

        case REQ_MODIFY_MAXDELAYDEVRATIO:
          handle_modify_maxdelaydevratio(&rx_message, &tx_message);
          break;

        case REQ_MODIFY_MAXUPDATESKEW:
          handle_modify_maxupdateskew(&rx_message, &tx_message);
          break;

        case REQ_MODIFY_MAKESTEP:
          handle_modify_makestep(&rx_message, &tx_message);
          break;

        case REQ_LOGON:
          /* Authentication is no longer supported, log-on always fails */
          tx_message.status = htons(STT_FAILED);
          break;

        case REQ_SETTIME:
          handle_settime(&rx_message, &tx_message);
          break;
        
        case REQ_LOCAL2:
          handle_local(&rx_message, &tx_message);
          break;

        case REQ_MANUAL:
          handle_manual(&rx_message, &tx_message);
          break;

        case REQ_N_SOURCES:
          handle_n_sources(&rx_message, &tx_message);
          break;

        case REQ_SOURCE_DATA:
          handle_source_data(&rx_message, &tx_message);
          break;

        case REQ_REKEY:
          handle_rekey(&rx_message, &tx_message);
          break;

        case REQ_ALLOW:
          handle_allowdeny(&rx_message, &tx_message, 1, 0);
          break;

        case REQ_ALLOWALL:
          handle_allowdeny(&rx_message, &tx_message, 1, 1);
          break;

        case REQ_DENY:
          handle_allowdeny(&rx_message, &tx_message, 0, 0);
          break;

        case REQ_DENYALL:
          handle_allowdeny(&rx_message, &tx_message, 0, 1);
          break;

        case REQ_CMDALLOW:
          handle_cmdallowdeny(&rx_message, &tx_message, 1, 0);
          break;

        case REQ_CMDALLOWALL:
          handle_cmdallowdeny(&rx_message, &tx_message, 1, 1);
          break;

        case REQ_CMDDENY:
          handle_cmdallowdeny(&rx_message, &tx_message, 0, 0);
          break;

        case REQ_CMDDENYALL:
          handle_cmdallowdeny(&rx_message, &tx_message, 0, 1);
          break;

        case REQ_ACCHECK:
          handle_accheck(&rx_message, &tx_message);
          break;

        case REQ_CMDACCHECK:
          handle_cmdaccheck(&rx_message, &tx_message);
          break;

        case REQ_ADD_SERVER3:
          handle_add_source(NTP_SERVER, &rx_message, &tx_message);
          break;

        case REQ_ADD_PEER3:
          handle_add_source(NTP_PEER, &rx_message, &tx_message);
          break;

        case REQ_DEL_SOURCE:
          handle_del_source(&rx_message, &tx_message);
          break;

        case REQ_WRITERTC:
          handle_writertc(&rx_message, &tx_message);
          break;
          
        case REQ_DFREQ:
          handle_dfreq(&rx_message, &tx_message);
          break;

        case REQ_DOFFSET:
          handle_doffset(&rx_message, &tx_message);
          break;

        case REQ_TRACKING:
          handle_tracking(&rx_message, &tx_message);
          break;

        case REQ_SMOOTHING:
          handle_smoothing(&rx_message, &tx_message);
          break;

        case REQ_SMOOTHTIME:
          handle_smoothtime(&rx_message, &tx_message);
          break;

        case REQ_SOURCESTATS:
          handle_sourcestats(&rx_message, &tx_message);
          break;

        case REQ_RTCREPORT:
          handle_rtcreport(&rx_message, &tx_message);
          break;
          
        case REQ_TRIMRTC:
          handle_trimrtc(&rx_message, &tx_message);
          break;

        case REQ_CYCLELOGS:
          handle_cyclelogs(&rx_message, &tx_message);
          break;

        case REQ_CLIENT_ACCESSES_BY_INDEX2:
          handle_client_accesses_by_index(&rx_message, &tx_message);
          break;

        case REQ_MANUAL_LIST:
          handle_manual_list(&rx_message, &tx_message);
          break;

        case REQ_MANUAL_DELETE:
          handle_manual_delete(&rx_message, &tx_message);
          break;

        case REQ_MAKESTEP:
          handle_make_step(&rx_message, &tx_message);
          break;

        case REQ_ACTIVITY:
          handle_activity(&rx_message, &tx_message);
          break;

        case REQ_RESELECTDISTANCE:
          handle_reselect_distance(&rx_message, &tx_message);
          break;

        case REQ_RESELECT:
          handle_reselect(&rx_message, &tx_message);
          break;

        case REQ_MODIFY_MINSTRATUM:
          handle_modify_minstratum(&rx_message, &tx_message);
          break;

        case REQ_MODIFY_POLLTARGET:
          handle_modify_polltarget(&rx_message, &tx_message);
          break;

        case REQ_REFRESH:
          handle_refresh(&rx_message, &tx_message);
          break;

        case REQ_SERVER_STATS:
          handle_server_stats(&rx_message, &tx_message);
          break;

        case REQ_NTP_DATA:
          handle_ntp_data(&rx_message, &tx_message);
          break;

        case REQ_SHUTDOWN:
          handle_shutdown(&rx_message, &tx_message);
          break;

        case REQ_ONOFFLINE:
          handle_onoffline(&rx_message, &tx_message);
          break;

        default:
          DEBUG_LOG("Unhandled command %d", rx_command);
          tx_message.status = htons(STT_FAILED);
          break;
      }
    } else {
      tx_message.status = htons(STT_UNAUTH);
    }
  }

  /* Transmit the response */
  {
    /* Include a simple way to lose one message in three to test resend */

    static int do_it=1;

    if (do_it) {
      transmit_reply(&tx_message, &where_from);
    }

#if 0
    do_it = ((do_it + 1) % 3);
#endif
  }
}

/* ================================================== */

int
CAM_AddAccessRestriction(IPAddr *ip_addr, int subnet_bits, int allow, int all)
 {
  ADF_Status status;

  if (allow) {
    if (all) {
      status = ADF_AllowAll(access_auth_table, ip_addr, subnet_bits);
    } else {
      status = ADF_Allow(access_auth_table, ip_addr, subnet_bits);
    }
  } else {
    if (all) {
      status = ADF_DenyAll(access_auth_table, ip_addr, subnet_bits);
    } else {
      status = ADF_Deny(access_auth_table, ip_addr, subnet_bits);
    }
  }

  if (status == ADF_BADSUBNET) {
    return 0;
  } else if (status == ADF_SUCCESS) {
    return 1;
  } else {
    return 0;
  }
}

/* ================================================== */

int
CAM_CheckAccessRestriction(IPAddr *ip_addr)
{
  return ADF_IsAllowed(access_auth_table, ip_addr);
}


/* ================================================== */
/* ================================================== */
