/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * Copyright (C) Miroslav Lichvar  2009-2016, 2018-2025
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
#include "socket.h"
#include "sources.h"
#include "sourcestats.h"
#include "reference.h"
#include "manual.h"
#include "memory.h"
#include "nts_ke_server.h"
#include "local.h"
#include "addrfilt.h"
#include "conf.h"
#include "rtc.h"
#include "pktlength.h"
#include "clientlog.h"
#include "refclock.h"

/* ================================================== */

#define INVALID_SOCK_FD (-5)

/* File descriptors for command and monitoring sockets */
static int sock_fdu;
static int sock_fd4;
static int sock_fd6;

/* Flag indicating the IPv4 socket is bound to an address */
static int bound_sock_fd4;

/* Flag indicating whether this module has been initialised or not */
static int initialised = 0;

/* ================================================== */

/* This authorisation table is used for checking whether particular
   machines are allowed to make command and monitoring requests. */
static ADF_AuthTable access_auth_table;

/* ================================================== */
/* Forward prototypes */
static void read_from_cmd_socket(int sock_fd, int event, void *anything);

/* ================================================== */

static int
open_socket(int family)
{
  const char *local_path, *iface;
  IPSockAddr local_addr;
  int sock_fd, port;

  switch (family) {
    case IPADDR_INET4:
    case IPADDR_INET6:
      port = CNF_GetCommandPort();
      if (port == 0 || !SCK_IsIpFamilyEnabled(family))
        return INVALID_SOCK_FD;

      CNF_GetBindCommandAddress(family, &local_addr.ip_addr);
      local_addr.port = port;
      iface = CNF_GetBindCommandInterface();

      sock_fd = SCK_OpenUdpSocket(NULL, &local_addr, iface, SCK_FLAG_RX_DEST_ADDR);
      if (sock_fd < 0) {
        LOG(LOGS_ERR, "Could not open command socket on %s",
            UTI_IPSockAddrToString(&local_addr));
        return INVALID_SOCK_FD;
      }

      if (family == IPADDR_INET4)
        bound_sock_fd4 = local_addr.ip_addr.addr.in4 != INADDR_ANY;

      break;
    case IPADDR_UNSPEC:
      local_path = CNF_GetBindCommandPath();

      sock_fd = SCK_OpenUnixDatagramSocket(NULL, local_path, 0);
      if (sock_fd < 0) {
        LOG(LOGS_ERR, "Could not open command socket on %s", local_path);
        return INVALID_SOCK_FD;
      }

      break;
    default:
      assert(0);
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
    BRIEF_ASSERT(padding_length <= MAX_PADDING_LENGTH && padding_length <= request_length &&
                 request_length <= sizeof (CMD_Request) &&
                 (request_length == 0 || request_length >= offsetof(CMD_Request, data)));
  }

  for (i = 1; i < N_REPLY_TYPES; i++) {
    reply.reply = htons(i);
    reply.status = STT_SUCCESS;
    reply_length = PKL_ReplyLength(&reply);
    BRIEF_ASSERT((reply_length == 0 || reply_length >= offsetof(CMD_Reply, data)) &&
                 reply_length <= sizeof (CMD_Reply));
  }
}

/* ================================================== */

void
CAM_Initialise(void)
{
  assert(!initialised);
  do_size_checks();

  initialised = 1;

  bound_sock_fd4 = 0;

  sock_fdu = INVALID_SOCK_FD;
  sock_fd4 = open_socket(IPADDR_INET4);
  sock_fd6 = open_socket(IPADDR_INET6);

  access_auth_table = ADF_CreateTable();
}

/* ================================================== */

void
CAM_Finalise(void)
{
  if (sock_fdu != INVALID_SOCK_FD) {
    SCH_RemoveFileHandler(sock_fdu);
    SCK_RemoveSocket(sock_fdu);
    SCK_CloseSocket(sock_fdu);
    sock_fdu = INVALID_SOCK_FD;
  }

  if (sock_fd4 != INVALID_SOCK_FD) {
    SCH_RemoveFileHandler(sock_fd4);
    SCK_CloseSocket(sock_fd4);
    sock_fd4 = INVALID_SOCK_FD;
  }

  if (sock_fd6 != INVALID_SOCK_FD) {
    SCH_RemoveFileHandler(sock_fd6);
    SCK_CloseSocket(sock_fd6);
    sock_fd6 = INVALID_SOCK_FD;
  }

  ADF_DestroyTable(access_auth_table);

  initialised = 0;
}

/* ================================================== */

void
CAM_OpenUnixSocket(void)
{
  /* This is separated from CAM_Initialise() as it needs to be called when
     the process has already dropped the root privileges */
  if (CNF_GetBindCommandPath())
    sock_fdu = open_socket(IPADDR_UNSPEC);
}

/* ================================================== */

static void
transmit_reply(int sock_fd, int request_length, SCK_Message *message)
{
  message->length = PKL_ReplyLength((CMD_Reply *)message->data);

  if (request_length < message->length) {
    DEBUG_LOG("Response longer than request req_len=%d res_len=%d",
              request_length, message->length);
    return;
  }

  /* Don't require responses to non-link-local addresses to use the same
     interface */
  if (message->addr_type == SCK_ADDR_IP &&
      !SCK_IsLinkLocalIPAddress(&message->remote_addr.ip.ip_addr))
    message->if_index = INVALID_IF_INDEX;

#if !defined(HAVE_IN_PKTINFO) && defined(IP_SENDSRCADDR)
  /* On FreeBSD a local IPv4 address cannot be specified on bound socket */
  if (message->addr_type == SCK_ADDR_IP && message->local_addr.ip.family == IPADDR_INET4 &&
      (sock_fd != sock_fd4 || bound_sock_fd4))
    message->local_addr.ip.family = IPADDR_UNSPEC;
#endif

  if (!SCK_SendMessage(sock_fd, message, 0))
    return;
}
  
/* ================================================== */

static void
handle_dump(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  SRC_DumpSources();
  NSR_DumpAuthData();
  NKS_DumpKeys();
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
                    ntohl(rx_message->data.local.orphan),
                    UTI_FloatNetworkToHost(rx_message->data.local.activate),
                    UTI_FloatNetworkToHost(rx_message->data.local.wait_synced),
                    UTI_FloatNetworkToHost(rx_message->data.local.wait_unsynced));
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
      case RPT_NONSELECTABLE:
        tx_message->data.source_data.state   = htons(RPY_SD_ST_NONSELECTABLE);
        break;
      case RPT_FALSETICKER:
        tx_message->data.source_data.state   = htons(RPY_SD_ST_FALSETICKER);
        break;
      case RPT_JITTERY:
        tx_message->data.source_data.state   = htons(RPY_SD_ST_JITTERY);
        break;
      case RPT_SELECTABLE:
        tx_message->data.source_data.state   = htons(RPY_SD_ST_SELECTABLE);
        break;
      case RPT_UNSELECTED:
        tx_message->data.source_data.state   = htons(RPY_SD_ST_UNSELECTED);
        break;
      case RPT_SELECTED:
        tx_message->data.source_data.state   = htons(RPY_SD_ST_SELECTED);
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
    tx_message->data.source_data.flags = htons(0);
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
  NKS_ReloadKeys();
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

static int
convert_addsrc_select_options(int flags)
{
    return (flags & REQ_ADDSRC_PREFER ? SRC_SELECT_PREFER : 0) |
           (flags & REQ_ADDSRC_NOSELECT ? SRC_SELECT_NOSELECT : 0) |
           (flags & REQ_ADDSRC_TRUST ? SRC_SELECT_TRUST : 0) |
           (flags & REQ_ADDSRC_REQUIRE ? SRC_SELECT_REQUIRE : 0);
}

/* ================================================== */

static void
handle_add_source(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  NTP_Source_Type type;
  SourceParameters params;
  int family, pool, port;
  NSR_Status status;
  uint32_t flags;
  char *name;
  
  switch (ntohl(rx_message->data.ntp_source.type)) {
    case REQ_ADDSRC_SERVER:
      type = NTP_SERVER;
      pool = 0;
      break;
    case REQ_ADDSRC_PEER:
      type = NTP_PEER;
      pool = 0;
      break;
    case REQ_ADDSRC_POOL:
      type = NTP_SERVER;
      pool = 1;
      break;
    default:
      tx_message->status = htons(STT_INVALID);
      return;
  }

  name = (char *)rx_message->data.ntp_source.name;

  /* Make sure the name is terminated */
  if (name[sizeof (rx_message->data.ntp_source.name) - 1] != '\0') {
      tx_message->status = htons(STT_INVALIDNAME);
      return;
  }

  flags = ntohl(rx_message->data.ntp_source.flags);

  family = flags & REQ_ADDSRC_IPV4 ? IPADDR_INET4 :
           flags & REQ_ADDSRC_IPV6 ? IPADDR_INET6 : IPADDR_UNSPEC;
  port = ntohl(rx_message->data.ntp_source.port);
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
  params.nts_port = ntohl(rx_message->data.ntp_source.nts_port);
  params.cert_set = ntohl(rx_message->data.ntp_source.cert_set);
  params.max_delay = UTI_FloatNetworkToHost(rx_message->data.ntp_source.max_delay);
  params.max_delay_ratio =
    UTI_FloatNetworkToHost(rx_message->data.ntp_source.max_delay_ratio);
  params.max_delay_dev_ratio =
    UTI_FloatNetworkToHost(rx_message->data.ntp_source.max_delay_dev_ratio);
  params.max_delay_quant =
    UTI_FloatNetworkToHost(rx_message->data.ntp_source.max_delay_quant);
  params.min_delay = UTI_FloatNetworkToHost(rx_message->data.ntp_source.min_delay);
  params.asymmetry = UTI_FloatNetworkToHost(rx_message->data.ntp_source.asymmetry);
  params.offset = UTI_FloatNetworkToHost(rx_message->data.ntp_source.offset);

  params.connectivity = flags & REQ_ADDSRC_ONLINE ? SRC_ONLINE : SRC_OFFLINE;
  params.auto_offline = !!(flags & REQ_ADDSRC_AUTOOFFLINE);
  params.iburst = !!(flags & REQ_ADDSRC_IBURST);
  params.interleaved = !!(flags & REQ_ADDSRC_INTERLEAVED);
  params.burst = !!(flags & REQ_ADDSRC_BURST);
  params.nts = !!(flags & REQ_ADDSRC_NTS);
  params.copy = !!(flags & REQ_ADDSRC_COPY);
  params.ext_fields = (flags & REQ_ADDSRC_EF_EXP_MONO_ROOT ? NTP_EF_FLAG_EXP_MONO_ROOT : 0) |
                      (flags & REQ_ADDSRC_EF_EXP_NET_CORRECTION ?
                       NTP_EF_FLAG_EXP_NET_CORRECTION : 0);
  params.sel_options = convert_addsrc_select_options(ntohl(rx_message->data.ntp_source.flags));

  status = NSR_AddSourceByName(name, family, port, pool, type, &params, NULL);
  switch (status) {
    case NSR_Success:
      break;
    case NSR_UnresolvedName:
      /* Try to resolve the name now */
      NSR_ResolveSources();
      break;
    case NSR_AlreadyInUse:
      tx_message->status = htons(STT_SOURCEALREADYKNOWN);
      break;
    case NSR_TooManySources:
      tx_message->status = htons(STT_TOOMANYSOURCES);
      break;
    case NSR_InvalidName:
      tx_message->status = htons(STT_INVALIDNAME);
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
  NSR_Status status;
  IPAddr ip_addr;
  
  UTI_IPNetworkToHost(&rx_message->data.del_source.ip_addr, &ip_addr);
  
  status = NSR_RemoveSource(&ip_addr);
  switch (status) {
    case NSR_Success:
      break;
    case NSR_NoSuchSource:
      tx_message->status = htons(STT_NOSUCHSOURCE);
      break;
    case NSR_TooManySources:
    case NSR_AlreadyInUse:
    case NSR_InvalidAF:
    case NSR_InvalidName:
    case NSR_UnresolvedName:
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
  double doffset;

  doffset = UTI_FloatNetworkToHost(rx_message->data.doffset.doffset);
  if (!LCL_AccumulateOffset(doffset, 0.0)) {
    tx_message->status = htons(STT_FAILED);
  } else {
    LOG(LOGS_INFO, "Accumulated delta offset of %.6f seconds", doffset);
  }
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
  uint32_t i, j, req_first_index, req_n_clients, req_min_hits, req_reset;
  struct timespec now;

  SCH_GetLastEventTime(&now, NULL, NULL);

  req_first_index = ntohl(rx_message->data.client_accesses_by_index.first_index);
  req_n_clients = ntohl(rx_message->data.client_accesses_by_index.n_clients);
  if (req_n_clients > MAX_CLIENT_ACCESSES)
    req_n_clients = MAX_CLIENT_ACCESSES;
  req_min_hits = ntohl(rx_message->data.client_accesses_by_index.min_hits);
  req_reset = ntohl(rx_message->data.client_accesses_by_index.reset);

  n_indices = CLG_GetNumberOfIndices();
  if (n_indices < 0) {
    tx_message->status = htons(STT_INACTIVE);
    return;
  }

  tx_message->reply = htons(RPY_CLIENT_ACCESSES_BY_INDEX3);
  tx_message->data.client_accesses_by_index.n_indices = htonl(n_indices);

  for (i = req_first_index, j = 0; i < (uint32_t)n_indices && j < req_n_clients; i++) {
    if (!CLG_GetClientAccessReportByIndex(i, req_reset, req_min_hits, &report, &now))
      continue;

    client = &tx_message->data.client_accesses_by_index.clients[j++];

    UTI_IPHostToNetwork(&report.ip_addr, &client->ip);
    client->ntp_hits = htonl(report.ntp_hits);
    client->nke_hits = htonl(report.nke_hits);
    client->cmd_hits = htonl(report.cmd_hits);
    client->ntp_drops = htonl(report.ntp_drops);
    client->nke_drops = htonl(report.nke_drops);
    client->cmd_drops = htonl(report.cmd_drops);
    client->ntp_interval = report.ntp_interval;
    client->nke_interval = report.nke_interval;
    client->cmd_interval = report.cmd_interval;
    client->ntp_timeout_interval = report.ntp_timeout_interval;
    client->last_ntp_hit_ago = htonl(report.last_ntp_hit_ago);
    client->last_nke_hit_ago = htonl(report.last_nke_hit_ago);
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
  tx_message->reply = htons(RPY_SERVER_STATS4);
  tx_message->data.server_stats.ntp_hits = UTI_Integer64HostToNetwork(report.ntp_hits);
  tx_message->data.server_stats.nke_hits = UTI_Integer64HostToNetwork(report.nke_hits);
  tx_message->data.server_stats.cmd_hits = UTI_Integer64HostToNetwork(report.cmd_hits);
  tx_message->data.server_stats.ntp_drops = UTI_Integer64HostToNetwork(report.ntp_drops);
  tx_message->data.server_stats.nke_drops = UTI_Integer64HostToNetwork(report.nke_drops);
  tx_message->data.server_stats.cmd_drops = UTI_Integer64HostToNetwork(report.cmd_drops);
  tx_message->data.server_stats.log_drops = UTI_Integer64HostToNetwork(report.log_drops);
  tx_message->data.server_stats.ntp_auth_hits =
    UTI_Integer64HostToNetwork(report.ntp_auth_hits);
  tx_message->data.server_stats.ntp_interleaved_hits =
    UTI_Integer64HostToNetwork(report.ntp_interleaved_hits);
  tx_message->data.server_stats.ntp_timestamps =
    UTI_Integer64HostToNetwork(report.ntp_timestamps);
  tx_message->data.server_stats.ntp_span_seconds =
    UTI_Integer64HostToNetwork(report.ntp_span_seconds);
  tx_message->data.server_stats.ntp_daemon_rx_timestamps =
    UTI_Integer64HostToNetwork(report.ntp_daemon_rx_timestamps);
  tx_message->data.server_stats.ntp_daemon_tx_timestamps =
    UTI_Integer64HostToNetwork(report.ntp_daemon_tx_timestamps);
  tx_message->data.server_stats.ntp_kernel_rx_timestamps =
    UTI_Integer64HostToNetwork(report.ntp_kernel_rx_timestamps);
  tx_message->data.server_stats.ntp_kernel_tx_timestamps =
    UTI_Integer64HostToNetwork(report.ntp_kernel_tx_timestamps);
  tx_message->data.server_stats.ntp_hw_rx_timestamps =
    UTI_Integer64HostToNetwork(report.ntp_hw_rx_timestamps);
  tx_message->data.server_stats.ntp_hw_tx_timestamps =
    UTI_Integer64HostToNetwork(report.ntp_hw_tx_timestamps);
  memset(tx_message->data.server_stats.reserved, 0xff,
         sizeof (tx_message->data.server_stats.reserved));
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

  tx_message->reply = htons(RPY_NTP_DATA2);
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
  tx_message->data.ntp_data.total_good_count = htonl(report.total_good_count);
  tx_message->data.ntp_data.total_kernel_tx_ts = htonl(report.total_kernel_tx_ts);
  tx_message->data.ntp_data.total_kernel_rx_ts = htonl(report.total_kernel_rx_ts);
  tx_message->data.ntp_data.total_hw_tx_ts = htonl(report.total_hw_tx_ts);
  tx_message->data.ntp_data.total_hw_rx_ts = htonl(report.total_hw_rx_ts);
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

static void
handle_ntp_source_name(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  IPAddr addr;
  char *name;

  UTI_IPNetworkToHost(&rx_message->data.ntp_source_name.ip_addr, &addr);
  name = NSR_GetName(&addr);

  if (!name) {
    tx_message->status = htons(STT_NOSUCHSOURCE);
    return;
  }

  tx_message->reply = htons(RPY_NTP_SOURCE_NAME);

  /* Avoid compiler warning */
  if (strlen(name) >= sizeof (tx_message->data.ntp_source_name.name))
    memcpy(tx_message->data.ntp_source_name.name, name,
           sizeof (tx_message->data.ntp_source_name.name));
  else
    strncpy((char *)tx_message->data.ntp_source_name.name, name,
            sizeof (tx_message->data.ntp_source_name.name));
}

/* ================================================== */

static void
handle_reload_sources(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  CNF_ReloadSources();
}

/* ================================================== */

static void
handle_reset_sources(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  struct timespec cooked_now, now;

  SRC_ResetSources();
  SCH_GetLastEventTime(&cooked_now, NULL, &now);
  LCL_NotifyExternalTimeStep(&now, &cooked_now, 0.0, 0.0);
}

/* ================================================== */

static void
handle_auth_data(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  RPT_AuthReport report;
  IPAddr ip_addr;

  UTI_IPNetworkToHost(&rx_message->data.auth_data.ip_addr, &ip_addr);

  if (!NSR_GetAuthReport(&ip_addr, &report)) {
    tx_message->status = htons(STT_NOSUCHSOURCE);
    return;
  }

  tx_message->reply = htons(RPY_AUTH_DATA);

  switch (report.mode) {
    case NTP_AUTH_NONE:
      tx_message->data.auth_data.mode = htons(RPY_AD_MD_NONE);
      break;
    case NTP_AUTH_SYMMETRIC:
      tx_message->data.auth_data.mode = htons(RPY_AD_MD_SYMMETRIC);
      break;
    case NTP_AUTH_NTS:
      tx_message->data.auth_data.mode = htons(RPY_AD_MD_NTS);
      break;
    default:
      break;
  }

  tx_message->data.auth_data.key_type = htons(report.key_type);
  tx_message->data.auth_data.key_id = htonl(report.key_id);
  tx_message->data.auth_data.key_length = htons(report.key_length);
  tx_message->data.auth_data.ke_attempts = htons(report.ke_attempts);
  tx_message->data.auth_data.last_ke_ago = htonl(report.last_ke_ago);
  tx_message->data.auth_data.cookies = htons(report.cookies);
  tx_message->data.auth_data.cookie_length = htons(report.cookie_length);
  tx_message->data.auth_data.nak = htons(report.nak);
}

/* ================================================== */

static uint16_t
convert_sd_sel_options(int options)
{
  return (options & SRC_SELECT_PREFER ? RPY_SD_OPTION_PREFER : 0) |
         (options & SRC_SELECT_NOSELECT ? RPY_SD_OPTION_NOSELECT : 0) |
         (options & SRC_SELECT_TRUST ? RPY_SD_OPTION_TRUST : 0) |
         (options & SRC_SELECT_REQUIRE ? RPY_SD_OPTION_REQUIRE : 0);
}

/* ================================================== */

static void
handle_select_data(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  RPT_SelectReport report;

  if (!SRC_GetSelectReport(ntohl(rx_message->data.select_data.index), &report)) {
    tx_message->status = htons(STT_NOSUCHSOURCE);
    return;
  }

  tx_message->reply = htons(RPY_SELECT_DATA);

  tx_message->data.select_data.ref_id = htonl(report.ref_id);
  UTI_IPHostToNetwork(&report.ip_addr, &tx_message->data.select_data.ip_addr);
  tx_message->data.select_data.state_char = report.state_char;
  tx_message->data.select_data.authentication = report.authentication;
  tx_message->data.select_data.leap = report.leap;
  tx_message->data.select_data.conf_options = htons(convert_sd_sel_options(report.conf_options));
  tx_message->data.select_data.eff_options = htons(convert_sd_sel_options(report.eff_options));
  tx_message->data.select_data.last_sample_ago = htonl(report.last_sample_ago);
  tx_message->data.select_data.score = UTI_FloatHostToNetwork(report.score);
  tx_message->data.select_data.hi_limit = UTI_FloatHostToNetwork(report.hi_limit);
  tx_message->data.select_data.lo_limit = UTI_FloatHostToNetwork(report.lo_limit);
}

/* ================================================== */

static void
handle_modify_selectopts(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  int mask, options;
  uint32_t ref_id;
  IPAddr ip_addr;

  UTI_IPNetworkToHost(&rx_message->data.modify_select_opts.address, &ip_addr);
  ref_id = ntohl(rx_message->data.modify_select_opts.ref_id);
  mask = ntohl(rx_message->data.modify_select_opts.mask);
  options = convert_addsrc_select_options(ntohl(rx_message->data.modify_select_opts.options));

  if (!SRC_ModifySelectOptions(&ip_addr, ref_id, options, mask))
    tx_message->status = htons(STT_NOSUCHSOURCE);
}

/* ================================================== */

static void
handle_modify_offset(CMD_Request *rx_message, CMD_Reply *tx_message)
{
  uint32_t ref_id;
  IPAddr ip_addr;
  double offset;

  UTI_IPNetworkToHost(&rx_message->data.modify_offset.address, &ip_addr);
  ref_id = ntohl(rx_message->data.modify_offset.ref_id);
  offset = UTI_FloatNetworkToHost(rx_message->data.modify_offset.new_offset);

  if ((ip_addr.family != IPADDR_UNSPEC && !NSR_ModifyOffset(&ip_addr, offset)) ||
      (ip_addr.family == IPADDR_UNSPEC && !RCL_ModifyOffset(ref_id, offset)))
    tx_message->status = htons(STT_NOSUCHSOURCE);
}

/* ================================================== */

static int
handle_readwrite_commands(int command, CMD_Request *request, CMD_Reply *reply)
{
  switch (command) {
    case REQ_ADD_SOURCE:
      handle_add_source(request, reply);
      break;
    case REQ_ALLOW:
      handle_allowdeny(request, reply, 1, 0);
      break;
    case REQ_ALLOWALL:
      handle_allowdeny(request, reply, 1, 1);
      break;
    case REQ_BURST:
      handle_burst(request, reply);
      break;
    case REQ_CMDALLOW:
      handle_cmdallowdeny(request, reply, 1, 0);
      break;
    case REQ_CMDALLOWALL:
      handle_cmdallowdeny(request, reply, 1, 1);
      break;
    case REQ_CMDDENY:
      handle_cmdallowdeny(request, reply, 0, 0);
      break;
    case REQ_CMDDENYALL:
      handle_cmdallowdeny(request, reply, 0, 1);
      break;
    case REQ_CYCLELOGS:
      handle_cyclelogs(request, reply);
      break;
    case REQ_DEL_SOURCE:
      handle_del_source(request, reply);
      break;
    case REQ_DENY:
      handle_allowdeny(request, reply, 0, 0);
      break;
    case REQ_DENYALL:
      handle_allowdeny(request, reply, 0, 1);
      break;
    case REQ_DFREQ:
      handle_dfreq(request, reply);
      break;
    case REQ_DOFFSET2:
      handle_doffset(request, reply);
      break;
    case REQ_DUMP:
      handle_dump(request, reply);
      break;
    case REQ_LOCAL3:
      handle_local(request, reply);
      break;
    case REQ_MAKESTEP:
      handle_make_step(request, reply);
      break;
    case REQ_MANUAL:
      handle_manual(request, reply);
      break;
    case REQ_MANUAL_DELETE:
      handle_manual_delete(request, reply);
      break;
    case REQ_MODIFY_MAKESTEP:
      handle_modify_makestep(request, reply);
      break;
    case REQ_MODIFY_MAXDELAY:
      handle_modify_maxdelay(request, reply);
      break;
    case REQ_MODIFY_MAXDELAYDEVRATIO:
      handle_modify_maxdelaydevratio(request, reply);
      break;
    case REQ_MODIFY_MAXDELAYRATIO:
      handle_modify_maxdelayratio(request, reply);
      break;
    case REQ_MODIFY_MAXPOLL:
      handle_modify_maxpoll(request, reply);
      break;
    case REQ_MODIFY_MAXUPDATESKEW:
      handle_modify_maxupdateskew(request, reply);
      break;
    case REQ_MODIFY_MINPOLL:
      handle_modify_minpoll(request, reply);
      break;
    case REQ_MODIFY_MINSTRATUM:
      handle_modify_minstratum(request, reply);
      break;
    case REQ_MODIFY_OFFSET:
      handle_modify_offset(request, reply);
      break;
    case REQ_MODIFY_POLLTARGET:
      handle_modify_polltarget(request, reply);
      break;
    case REQ_MODIFY_SELECTOPTS:
      handle_modify_selectopts(request, reply);
      break;
    case REQ_OFFLINE:
      handle_offline(request, reply);
      break;
    case REQ_ONLINE:
      handle_online(request, reply);
      break;
    case REQ_ONOFFLINE:
      handle_onoffline(request, reply);
      break;
    case REQ_REFRESH:
      handle_refresh(request, reply);
      break;
    case REQ_REKEY:
      handle_rekey(request, reply);
      break;
    case REQ_RELOAD_SOURCES:
      handle_reload_sources(request, reply);
      break;
    case REQ_RESELECT:
      handle_reselect(request, reply);
      break;
    case REQ_RESELECTDISTANCE:
      handle_reselect_distance(request, reply);
      break;
    case REQ_RESET_SOURCES:
      handle_reset_sources(request, reply);
      break;
    case REQ_SETTIME:
      handle_settime(request, reply);
      break;
    case REQ_SHUTDOWN:
      handle_shutdown(request, reply);
      break;
    case REQ_SMOOTHTIME:
      handle_smoothtime(request, reply);
      break;
    case REQ_TRIMRTC:
      handle_trimrtc(request, reply);
      break;
    case REQ_WRITERTC:
      handle_writertc(request, reply);
      break;
    default:
      return 0;
  }

  return 1;
}

/* ================================================== */

static int
handle_readonly_commands(int command, int full_access, CMD_Request *request, CMD_Reply *reply)
{
  ARR_Instance open_commands;
  int i, allowed = 0;

  if (full_access) {
    allowed = 1;
  } else {
    open_commands = CNF_GetOpenCommands();

    for (i = 0; i < ARR_GetSize(open_commands); i++) {
      if (*(int *)ARR_GetElement(open_commands, i) == command) {
        allowed = 1;
        break;
      }
    }
  }

  if (!allowed)
    return 0;

  switch (command) {
    case REQ_ACCHECK:
      handle_accheck(request, reply);
      break;
    case REQ_ACTIVITY:
      handle_activity(request, reply);
      break;
    case REQ_AUTH_DATA:
      handle_auth_data(request, reply);
      break;
    case REQ_CLIENT_ACCESSES_BY_INDEX3:
      handle_client_accesses_by_index(request, reply);
      break;
    case REQ_CMDACCHECK:
      handle_cmdaccheck(request, reply);
      break;
    case REQ_MANUAL_LIST:
      handle_manual_list(request, reply);
      break;
    case REQ_NTP_DATA:
      handle_ntp_data(request, reply);
      break;
    case REQ_NTP_SOURCE_NAME:
      handle_ntp_source_name(request, reply);
      break;
    case REQ_N_SOURCES:
      handle_n_sources(request, reply);
      break;
    case REQ_RTCREPORT:
      handle_rtcreport(request, reply);
      break;
    case REQ_SELECT_DATA:
      handle_select_data(request, reply);
      break;
    case REQ_SERVER_STATS:
      handle_server_stats(request, reply);
      break;
    case REQ_SMOOTHING:
      handle_smoothing(request, reply);
      break;
    case REQ_SOURCESTATS:
      handle_sourcestats(request, reply);
      break;
    case REQ_SOURCE_DATA:
      handle_source_data(request, reply);
      break;
    case REQ_TRACKING:
      handle_tracking(request, reply);
      break;
    default:
      return 0;
  }

  return 1;
}

/* ================================================== */
/* Read a packet and process it */

static void
read_from_cmd_socket(int sock_fd, int event, void *anything)
{
  int read_length, expected_length, localhost, log_index, full_access, handled;
  SCK_Message *sck_message;
  CMD_Request rx_message;
  CMD_Reply tx_message;
  IPAddr loopback_addr, remote_ip;
  uint16_t rx_command;
  struct timespec now, cooked_now;

  sck_message = SCK_ReceiveMessage(sock_fd, 0);
  if (!sck_message)
    return;

  read_length = sck_message->length;

  /* Get current time cheaply */
  SCH_GetLastEventTime(&cooked_now, NULL, &now);

  /* Check if the request came from the Unix domain socket, or network and
     whether the address is allowed (127.0.0.1 and ::1 is always allowed) */
  if ((sock_fd == sock_fd4 || sock_fd == sock_fd6) && sck_message->addr_type == SCK_ADDR_IP) {
    remote_ip = sck_message->remote_addr.ip.ip_addr;
    SCK_GetLoopbackIPAddress(remote_ip.family, &loopback_addr);
    localhost = UTI_CompareIPs(&remote_ip, &loopback_addr, NULL) == 0;

    if (!localhost && !ADF_IsAllowed(access_auth_table, &remote_ip)) {
      DEBUG_LOG("Unauthorised host %s",
                UTI_IPSockAddrToString(&sck_message->remote_addr.ip));
      return;
    }

    full_access = 0;
  } else if (sock_fd == sock_fdu && sck_message->addr_type == SCK_ADDR_UNIX) {
    remote_ip.family = IPADDR_UNSPEC;
    localhost = 1;
    full_access = 1;
  } else {
    DEBUG_LOG("Unexpected socket/address");
    return;
  }

  if (read_length < offsetof(CMD_Request, data) ||
      read_length < offsetof(CMD_Reply, data) ||
      read_length > sizeof (CMD_Request)) {
    /* We don't know how to process anything like this or an error reply
       would be larger than the request */
    DEBUG_LOG("Unexpected length");
    return;
  }

  memcpy(&rx_message, sck_message->data, read_length);

  if (rx_message.pkt_type != PKT_TYPE_CMD_REQUEST ||
      rx_message.res1 != 0 ||
      rx_message.res2 != 0) {
    DEBUG_LOG("Command packet dropped");
    return;
  }

  log_index = CLG_LogServiceAccess(CLG_CMDMON, &remote_ip, &cooked_now);

  /* Don't reply to all requests from hosts other than localhost if the rate
     is excessive */
  if (!localhost && log_index >= 0 &&
      CLG_LimitServiceRate(CLG_CMDMON, log_index) != CLG_PASS) {
    DEBUG_LOG("Command packet discarded to limit response rate");
    return;
  }

  expected_length = PKL_CommandLength(&rx_message);
  rx_command = ntohs(rx_message.command);

  memset(&tx_message, 0, sizeof (tx_message));
  sck_message->data = &tx_message;
  sck_message->length = 0;

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
      transmit_reply(sock_fd, read_length, sck_message);
    }
    return;
  }

  if (rx_command >= N_REQUEST_TYPES ||
      expected_length < (int)offsetof(CMD_Request, data)) {
    DEBUG_LOG("Command packet has invalid command %d", rx_command);

    tx_message.status = htons(STT_INVALID);
    transmit_reply(sock_fd, read_length, sck_message);
    return;
  }

  if (read_length < expected_length) {
    DEBUG_LOG("Command packet is too short (%d < %d)", read_length,
              expected_length);

    tx_message.status = htons(STT_BADPKTLENGTH);
    transmit_reply(sock_fd, read_length, sck_message);
    return;
  }

  /* OK, we have a valid message.  Now dispatch on message type and process it. */

  LOG_SetContext(LOGC_Command);

  if (full_access)
    handled = handle_readwrite_commands(rx_command, &rx_message, &tx_message);
  else
    handled = 0;

  if (!handled)
    handled = handle_readonly_commands(rx_command, full_access, &rx_message, &tx_message);

  if (!handled)
    tx_message.status = htons(STT_UNAUTH);

  LOG_UnsetContext(LOGC_Command);

  /* Transmit the response */
  transmit_reply(sock_fd, read_length, sck_message);
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
    LOG(LOG_GetContextSeverity(LOGC_Command), "%s%s %s access from %s",
        allow ? "Allowed" : "Denied", all ? " all" : "", "command",
        UTI_IPSubnetToString(ip_addr, subnet_bits));
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
