/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2016-2019
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

  Functions for NTP I/O specific to Linux
  */

#include "config.h"

#include "sysincl.h"

#include <ifaddrs.h>
#include <linux/errqueue.h>
#include <linux/ethtool.h>
#include <linux/net_tstamp.h>
#include <linux/sockios.h>
#include <net/if.h>

#include "array.h"
#include "conf.h"
#include "hwclock.h"
#include "local.h"
#include "logging.h"
#include "ntp_core.h"
#include "ntp_io.h"
#include "ntp_io_linux.h"
#include "ntp_sources.h"
#include "sched.h"
#include "sys_linux.h"
#include "util.h"

union sockaddr_in46 {
  struct sockaddr_in in4;
#ifdef FEAT_IPV6
  struct sockaddr_in6 in6;
#endif
  struct sockaddr u;
};

struct Interface {
  char name[IF_NAMESIZE];
  int if_index;
  int phc_fd;
  int phc_mode;
  int phc_nocrossts;
  /* Link speed in mbit/s */
  int link_speed;
  /* Start of UDP data at layer 2 for IPv4 and IPv6 */
  int l2_udp4_ntp_start;
  int l2_udp6_ntp_start;
  /* Precision of PHC readings */
  double precision;
  /* Compensation of errors in TX and RX timestamping */
  double tx_comp;
  double rx_comp;
  HCL_Instance clock;
};

/* Number of PHC readings per HW clock sample */
#define PHC_READINGS 10

/* Minimum interval between PHC readings */
#define MIN_PHC_POLL -6

/* Maximum acceptable offset between HW and daemon/kernel timestamp */
#define MAX_TS_DELAY 1.0

/* Array of Interfaces */
static ARR_Instance interfaces;

/* RX/TX and TX-specific timestamping socket options */
static int ts_flags;
static int ts_tx_flags;

/* Flag indicating the socket options can't be changed in control messages */
static int permanent_ts_options;

/* When sending client requests to a close and fast server, it is possible that
   a response will be received before the HW transmit timestamp of the request
   itself.  To avoid processing of the response without the HW timestamp, we
   monitor events returned by select() and suspend reading of packets from the
   receive queue for up to 200 microseconds.  As the requests are normally
   separated by at least 200 milliseconds, it is sufficient to monitor and
   suspend one socket at a time. */
static int monitored_socket;
static int suspended_socket;
static SCH_TimeoutID resume_timeout_id;

#define RESUME_TIMEOUT 200.0e-6

/* Unbound socket keeping the kernel RX timestamping permanently enabled
   in order to avoid a race condition between receiving a server response
   and the kernel actually starting to timestamp received packets after
   enabling the timestamping and sending a request */
static int dummy_rxts_socket;

#define INVALID_SOCK_FD -3

/* ================================================== */

static int
add_interface(CNF_HwTsInterface *conf_iface)
{
  struct ethtool_ts_info ts_info;
  struct hwtstamp_config ts_config;
  struct ifreq req;
  int sock_fd, if_index, phc_fd, req_hwts_flags, rx_filter;
  unsigned int i;
  struct Interface *iface;

  /* Check if the interface was not already added */
  for (i = 0; i < ARR_GetSize(interfaces); i++) {
    if (!strcmp(conf_iface->name, ((struct Interface *)ARR_GetElement(interfaces, i))->name))
      return 1;
  }

  sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock_fd < 0)
    return 0;

  memset(&req, 0, sizeof (req));
  memset(&ts_info, 0, sizeof (ts_info));

  if (snprintf(req.ifr_name, sizeof (req.ifr_name), "%s", conf_iface->name) >=
      sizeof (req.ifr_name)) {
    close(sock_fd);
    return 0;
  }

  if (ioctl(sock_fd, SIOCGIFINDEX, &req)) {
    DEBUG_LOG("ioctl(%s) failed : %s", "SIOCGIFINDEX", strerror(errno));
    close(sock_fd);
    return 0;
  }

  if_index = req.ifr_ifindex;

  ts_info.cmd = ETHTOOL_GET_TS_INFO;
  req.ifr_data = (char *)&ts_info;

  if (ioctl(sock_fd, SIOCETHTOOL, &req)) {
    DEBUG_LOG("ioctl(%s) failed : %s", "SIOCETHTOOL", strerror(errno));
    close(sock_fd);
    return 0;
  }

  req_hwts_flags = SOF_TIMESTAMPING_RX_HARDWARE | SOF_TIMESTAMPING_TX_HARDWARE |
                   SOF_TIMESTAMPING_RAW_HARDWARE;
  if ((ts_info.so_timestamping & req_hwts_flags) != req_hwts_flags) {
    DEBUG_LOG("HW timestamping not supported on %s", req.ifr_name);
    close(sock_fd);
    return 0;
  }

  if (ts_info.phc_index < 0) {
    DEBUG_LOG("PHC missing on %s", req.ifr_name);
    close(sock_fd);
    return 0;
  }

  switch (conf_iface->rxfilter) {
    case CNF_HWTS_RXFILTER_ANY:
#ifdef HAVE_LINUX_TIMESTAMPING_RXFILTER_NTP
      if (ts_info.rx_filters & (1 << HWTSTAMP_FILTER_NTP_ALL))
        rx_filter = HWTSTAMP_FILTER_NTP_ALL;
      else
#endif
      if (ts_info.rx_filters & (1 << HWTSTAMP_FILTER_ALL))
        rx_filter = HWTSTAMP_FILTER_ALL;
      else
        rx_filter = HWTSTAMP_FILTER_NONE;
      break;
    case CNF_HWTS_RXFILTER_NONE:
      rx_filter = HWTSTAMP_FILTER_NONE;
      break;
#ifdef HAVE_LINUX_TIMESTAMPING_RXFILTER_NTP
    case CNF_HWTS_RXFILTER_NTP:
      rx_filter = HWTSTAMP_FILTER_NTP_ALL;
      break;
#endif
    default:
      rx_filter = HWTSTAMP_FILTER_ALL;
      break;
  }

  ts_config.flags = 0;
  ts_config.tx_type = HWTSTAMP_TX_ON;
  ts_config.rx_filter = rx_filter;
  req.ifr_data = (char *)&ts_config;

  if (ioctl(sock_fd, SIOCSHWTSTAMP, &req)) {
    DEBUG_LOG("ioctl(%s) failed : %s", "SIOCSHWTSTAMP", strerror(errno));

    /* Check the current timestamping configuration in case this interface
       allows only reading of the configuration and it was already configured
       as requested */
    req.ifr_data = (char *)&ts_config;
#ifdef SIOCGHWTSTAMP
    if (ioctl(sock_fd, SIOCGHWTSTAMP, &req) ||
        ts_config.tx_type != HWTSTAMP_TX_ON || ts_config.rx_filter != rx_filter)
#endif
    {
      close(sock_fd);
      return 0;
    }
  }

  close(sock_fd);

  phc_fd = SYS_Linux_OpenPHC(NULL, ts_info.phc_index);
  if (phc_fd < 0)
    return 0;

  iface = ARR_GetNewElement(interfaces);

  snprintf(iface->name, sizeof (iface->name), "%s", conf_iface->name);
  iface->if_index = if_index;
  iface->phc_fd = phc_fd;
  iface->phc_mode = 0;
  iface->phc_nocrossts = conf_iface->nocrossts;

  /* Start with 1 gbit and no VLANs or IPv4/IPv6 options */
  iface->link_speed = 1000;
  iface->l2_udp4_ntp_start = 42;
  iface->l2_udp6_ntp_start = 62;

  iface->precision = conf_iface->precision;
  iface->tx_comp = conf_iface->tx_comp;
  iface->rx_comp = conf_iface->rx_comp;

  iface->clock = HCL_CreateInstance(conf_iface->min_samples, conf_iface->max_samples,
                                    UTI_Log2ToDouble(MAX(conf_iface->minpoll, MIN_PHC_POLL)));

  LOG(LOGS_INFO, "Enabled HW timestamping %son %s",
      ts_config.rx_filter == HWTSTAMP_FILTER_NONE ? "(TX only) " : "", iface->name);

  return 1;
}

/* ================================================== */

static int
add_all_interfaces(CNF_HwTsInterface *conf_iface_all)
{
  CNF_HwTsInterface conf_iface;
  struct ifaddrs *ifaddr, *ifa;
  int r;

  conf_iface = *conf_iface_all;

  if (getifaddrs(&ifaddr)) {
    DEBUG_LOG("getifaddrs() failed : %s", strerror(errno));
    return 0;
  }

  for (r = 0, ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
    conf_iface.name = ifa->ifa_name;
    if (add_interface(&conf_iface))
      r = 1;
  }
  
  freeifaddrs(ifaddr);

  /* Return success if at least one interface was added */
  return r;
}

/* ================================================== */

static void
update_interface_speed(struct Interface *iface)
{
  struct ethtool_cmd cmd;
  struct ifreq req;
  int sock_fd, link_speed;

  sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock_fd < 0)
    return;

  memset(&req, 0, sizeof (req));
  memset(&cmd, 0, sizeof (cmd));

  snprintf(req.ifr_name, sizeof (req.ifr_name), "%s", iface->name);
  cmd.cmd = ETHTOOL_GSET;
  req.ifr_data = (char *)&cmd;

  if (ioctl(sock_fd, SIOCETHTOOL, &req)) {
    DEBUG_LOG("ioctl(%s) failed : %s", "SIOCETHTOOL", strerror(errno));
    close(sock_fd);
    return;
  }

  close(sock_fd);

  link_speed = ethtool_cmd_speed(&cmd);

  if (iface->link_speed != link_speed) {
    iface->link_speed = link_speed;
    DEBUG_LOG("Updated speed of %s to %d Mb/s", iface->name, link_speed);
  }
}

/* ================================================== */

#if defined(HAVE_LINUX_TIMESTAMPING_OPT_PKTINFO) || defined(HAVE_LINUX_TIMESTAMPING_OPT_TX_SWHW)
static int
check_timestamping_option(int option)
{
  int sock_fd;

  sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock_fd < 0)
    return 0;

  if (setsockopt(sock_fd, SOL_SOCKET, SO_TIMESTAMPING, &option, sizeof (option)) < 0) {
    DEBUG_LOG("Could not enable timestamping option %x", (unsigned int)option);
    close(sock_fd);
    return 0;
  }

  close(sock_fd);
  return 1;
}
#endif

/* ================================================== */

static int
open_dummy_socket(void)
{
  int sock_fd, events = 0;

  if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0
#ifdef FEAT_IPV6
      && (sock_fd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0
#endif
     )
    return INVALID_SOCK_FD;

  if (!NIO_Linux_SetTimestampSocketOptions(sock_fd, 1, &events)) {
    close(sock_fd);
    return INVALID_SOCK_FD;
  }

  UTI_FdSetCloexec(sock_fd);
  return sock_fd;
}

/* ================================================== */

void
NIO_Linux_Initialise(void)
{
  CNF_HwTsInterface *conf_iface;
  unsigned int i;
  int hwts;

  interfaces = ARR_CreateInstance(sizeof (struct Interface));

  /* Enable HW timestamping on specified interfaces.  If "*" was specified, try
     all interfaces.  If no interface was specified, enable SW timestamping. */

  for (i = hwts = 0; CNF_GetHwTsInterface(i, &conf_iface); i++) {
    if (!strcmp("*", conf_iface->name))
      continue;
    if (!add_interface(conf_iface))
      LOG_FATAL("Could not enable HW timestamping on %s", conf_iface->name);
    hwts = 1;
  }

  for (i = 0; CNF_GetHwTsInterface(i, &conf_iface); i++) {
    if (strcmp("*", conf_iface->name))
      continue;
    if (add_all_interfaces(conf_iface))
      hwts = 1;
    break;
  }

  ts_flags = SOF_TIMESTAMPING_SOFTWARE | SOF_TIMESTAMPING_RX_SOFTWARE;
  ts_tx_flags = SOF_TIMESTAMPING_TX_SOFTWARE;

  if (hwts) {
    ts_flags |= SOF_TIMESTAMPING_RAW_HARDWARE | SOF_TIMESTAMPING_RX_HARDWARE;
    ts_tx_flags |= SOF_TIMESTAMPING_TX_HARDWARE;
#ifdef HAVE_LINUX_TIMESTAMPING_OPT_PKTINFO
    if (check_timestamping_option(SOF_TIMESTAMPING_OPT_PKTINFO))
      ts_flags |= SOF_TIMESTAMPING_OPT_PKTINFO;
#endif
#ifdef HAVE_LINUX_TIMESTAMPING_OPT_TX_SWHW
    if (check_timestamping_option(SOF_TIMESTAMPING_OPT_TX_SWHW))
      ts_flags |= SOF_TIMESTAMPING_OPT_TX_SWHW;
#endif
  }

  /* Enable IP_PKTINFO in messages looped back to the error queue */
  ts_flags |= SOF_TIMESTAMPING_OPT_CMSG;

  /* Kernels before 4.7 ignore timestamping flags set in control messages */
  permanent_ts_options = !SYS_Linux_CheckKernelVersion(4, 7);

  monitored_socket = INVALID_SOCK_FD;
  suspended_socket = INVALID_SOCK_FD;
  dummy_rxts_socket = INVALID_SOCK_FD;
}

/* ================================================== */

void
NIO_Linux_Finalise(void)
{
  struct Interface *iface;
  unsigned int i;

  if (dummy_rxts_socket != INVALID_SOCK_FD)
    close(dummy_rxts_socket);

  for (i = 0; i < ARR_GetSize(interfaces); i++) {
    iface = ARR_GetElement(interfaces, i);
    HCL_DestroyInstance(iface->clock);
    close(iface->phc_fd);
  }

  ARR_DestroyInstance(interfaces);
}

/* ================================================== */

int
NIO_Linux_SetTimestampSocketOptions(int sock_fd, int client_only, int *events)
{
  int val, flags;

  if (!ts_flags)
    return 0;

  /* Enable SCM_TIMESTAMPING control messages and the socket's error queue in
     order to receive our transmitted packets with more accurate timestamps */

  val = 1;
  flags = ts_flags;

  if (client_only || permanent_ts_options)
    flags |= ts_tx_flags;

  if (setsockopt(sock_fd, SOL_SOCKET, SO_SELECT_ERR_QUEUE, &val, sizeof (val)) < 0) {
    LOG(LOGS_ERR, "Could not set %s socket option", "SO_SELECT_ERR_QUEUE");
    ts_flags = 0;
    return 0;
  }

  if (setsockopt(sock_fd, SOL_SOCKET, SO_TIMESTAMPING, &flags, sizeof (flags)) < 0) {
    LOG(LOGS_ERR, "Could not set %s socket option", "SO_TIMESTAMPING");
    ts_flags = 0;
    return 0;
  }

  *events |= SCH_FILE_EXCEPTION;
  return 1;
}

/* ================================================== */

static void
resume_socket(int sock_fd)
{
  if (monitored_socket == sock_fd)
    monitored_socket = INVALID_SOCK_FD;

  if (sock_fd == INVALID_SOCK_FD || sock_fd != suspended_socket)
    return;

  suspended_socket = INVALID_SOCK_FD;

  SCH_SetFileHandlerEvent(sock_fd, SCH_FILE_INPUT, 1);

  DEBUG_LOG("Resumed RX processing %s timeout fd=%d",
            resume_timeout_id ? "before" : "on", sock_fd);

  if (resume_timeout_id) {
    SCH_RemoveTimeout(resume_timeout_id);
    resume_timeout_id = 0;
  }
}

/* ================================================== */

static void
resume_timeout(void *arg)
{
  resume_timeout_id = 0;
  resume_socket(suspended_socket);
}

/* ================================================== */

static void
suspend_socket(int sock_fd)
{
  resume_socket(suspended_socket);

  suspended_socket = sock_fd;

  SCH_SetFileHandlerEvent(suspended_socket, SCH_FILE_INPUT, 0);
  resume_timeout_id = SCH_AddTimeoutByDelay(RESUME_TIMEOUT, resume_timeout, NULL);

  DEBUG_LOG("Suspended RX processing fd=%d", sock_fd);
}

/* ================================================== */

int
NIO_Linux_ProcessEvent(int sock_fd, int event)
{
  if (sock_fd != monitored_socket)
    return 0;

  if (event == SCH_FILE_INPUT) {
    suspend_socket(monitored_socket);
    monitored_socket = INVALID_SOCK_FD;

    /* Don't process the message yet */
    return 1;
  }

  return 0;
}

/* ================================================== */

static struct Interface *
get_interface(int if_index)
{
  struct Interface *iface;
  unsigned int i;

  for (i = 0; i < ARR_GetSize(interfaces); i++) {
    iface = ARR_GetElement(interfaces, i);
    if (iface->if_index != if_index)
      continue;

    return iface;
  }

  return NULL;
}

/* ================================================== */

static void
process_hw_timestamp(struct Interface *iface, struct timespec *hw_ts,
                     NTP_Local_Timestamp *local_ts, int rx_ntp_length, int family,
                     int l2_length)
{
  struct timespec sample_phc_ts, sample_sys_ts, sample_local_ts, ts;
  double rx_correction, ts_delay, phc_err, local_err;

  if (HCL_NeedsNewSample(iface->clock, &local_ts->ts)) {
    if (!SYS_Linux_GetPHCSample(iface->phc_fd, iface->phc_nocrossts, iface->precision,
                                &iface->phc_mode, &sample_phc_ts, &sample_sys_ts,
                                &phc_err))
      return;

    LCL_CookTime(&sample_sys_ts, &sample_local_ts, &local_err);
    HCL_AccumulateSample(iface->clock, &sample_phc_ts, &sample_local_ts,
                         phc_err + local_err);

    update_interface_speed(iface);
  }

  /* We need to transpose RX timestamps as hardware timestamps are normally
     preamble timestamps and RX timestamps in NTP are supposed to be trailer
     timestamps.  If we don't know the length of the packet at layer 2, we
     make an assumption that UDP data start at the same position as in the
     last transmitted packet which had a HW TX timestamp. */
  if (rx_ntp_length && iface->link_speed) {
    if (!l2_length)
      l2_length = (family == IPADDR_INET4 ? iface->l2_udp4_ntp_start :
                   iface->l2_udp6_ntp_start) + rx_ntp_length;

    /* Include the frame check sequence (FCS) */
    l2_length += 4;

    rx_correction = l2_length / (1.0e6 / 8 * iface->link_speed);

    UTI_AddDoubleToTimespec(hw_ts, rx_correction, hw_ts);
  }

  if (!HCL_CookTime(iface->clock, hw_ts, &ts, &local_err))
    return;

  if (!rx_ntp_length && iface->tx_comp)
    UTI_AddDoubleToTimespec(&ts, iface->tx_comp, &ts);
  else if (rx_ntp_length && iface->rx_comp)
    UTI_AddDoubleToTimespec(&ts, -iface->rx_comp, &ts);

  ts_delay = UTI_DiffTimespecsToDouble(&local_ts->ts, &ts);

  if (fabs(ts_delay) > MAX_TS_DELAY) {
    DEBUG_LOG("Unacceptable timestamp delay %.9f", ts_delay);
    return;
  }

  local_ts->ts = ts;
  local_ts->err = local_err;
  local_ts->source = NTP_TS_HARDWARE;
}

/* ================================================== */
/* Extract UDP data from a layer 2 message.  Supported is Ethernet
   with optional VLAN tags. */

static int
extract_udp_data(unsigned char *msg, NTP_Remote_Address *remote_addr, int len)
{
  unsigned char *msg_start = msg;
  union sockaddr_in46 addr;

  remote_addr->ip_addr.family = IPADDR_UNSPEC;
  remote_addr->port = 0;

  /* Skip MACs */
  if (len < 12)
    return 0;
  len -= 12, msg += 12;

  /* Skip VLAN tag(s) if present */
  while (len >= 4 && msg[0] == 0x81 && msg[1] == 0x00)
    len -= 4, msg += 4;

  /* Skip IPv4 or IPv6 ethertype */
  if (len < 2 || !((msg[0] == 0x08 && msg[1] == 0x00) ||
                   (msg[0] == 0x86 && msg[1] == 0xdd)))
    return 0;
  len -= 2, msg += 2;

  /* Parse destination address and port from IPv4/IPv6 and UDP headers */
  if (len >= 20 && msg[0] >> 4 == 4) {
    int ihl = (msg[0] & 0xf) * 4;

    if (len < ihl + 8 || msg[9] != 17)
      return 0;

    memcpy(&addr.in4.sin_addr.s_addr, msg + 16, sizeof (uint32_t));
    addr.in4.sin_port = *(uint16_t *)(msg + ihl + 2);
    addr.in4.sin_family = AF_INET;
    len -= ihl + 8, msg += ihl + 8;
#ifdef FEAT_IPV6
  } else if (len >= 48 && msg[0] >> 4 == 6) {
    int eh_len, next_header = msg[6];

    memcpy(&addr.in6.sin6_addr.s6_addr, msg + 24, 16);
    len -= 40, msg += 40;

    /* Skip IPv6 extension headers if present */
    while (next_header != 17) {
      switch (next_header) {
        case 44:  /* Fragment Header */
          /* Process only the first fragment */
          if (ntohs(*(uint16_t *)(msg + 2)) >> 3 != 0)
            return 0;
          eh_len = 8;
          break;
        case 0:   /* Hop-by-Hop Options */
        case 43:  /* Routing Header */
        case 60:  /* Destination Options */
        case 135: /* Mobility Header */
          eh_len = 8 * (msg[1] + 1);
          break;
        case 51:  /* Authentication Header */
          eh_len = 4 * (msg[1] + 2);
          break;
        default:
          return 0;
      }

      if (eh_len < 8 || len < eh_len + 8)
        return 0;

      next_header = msg[0];
      len -= eh_len, msg += eh_len;
    }

    addr.in6.sin6_port = *(uint16_t *)(msg + 2);
    addr.in6.sin6_family = AF_INET6;
    len -= 8, msg += 8;
#endif
  } else {
    return 0;
  }

  UTI_SockaddrToIPAndPort(&addr.u, &remote_addr->ip_addr, &remote_addr->port);

  /* Move the message to fix alignment of its fields */
  if (len > 0)
    memmove(msg_start, msg, len);

  return len;
}

/* ================================================== */

int
NIO_Linux_ProcessMessage(NTP_Remote_Address *remote_addr, NTP_Local_Address *local_addr,
                         NTP_Local_Timestamp *local_ts, struct msghdr *hdr, int length)
{
  struct Interface *iface;
  struct cmsghdr *cmsg;
  int is_tx, ts_if_index, l2_length;

  is_tx = hdr->msg_flags & MSG_ERRQUEUE;
  iface = NULL;
  ts_if_index = local_addr->if_index;
  l2_length = 0;

  for (cmsg = CMSG_FIRSTHDR(hdr); cmsg; cmsg = CMSG_NXTHDR(hdr, cmsg)) {
#ifdef HAVE_LINUX_TIMESTAMPING_OPT_PKTINFO
    if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_TIMESTAMPING_PKTINFO) {
      struct scm_ts_pktinfo ts_pktinfo;

      memcpy(&ts_pktinfo, CMSG_DATA(cmsg), sizeof (ts_pktinfo));

      ts_if_index = ts_pktinfo.if_index;
      l2_length = ts_pktinfo.pkt_length;

      DEBUG_LOG("Received HW timestamp info if=%d length=%d", ts_if_index, l2_length);
    }
#endif

    if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_TIMESTAMPING) {
      struct scm_timestamping ts3;

      memcpy(&ts3, CMSG_DATA(cmsg), sizeof (ts3));

      if (!UTI_IsZeroTimespec(&ts3.ts[2])) {
        iface = get_interface(ts_if_index);
        if (iface) {
          process_hw_timestamp(iface, &ts3.ts[2], local_ts, !is_tx ? length : 0,
                               remote_addr->ip_addr.family, l2_length);
        } else {
          DEBUG_LOG("HW clock not found for interface %d", ts_if_index);
        }

        /* If a HW transmit timestamp was received, resume processing
           of non-error messages on this socket */
        if (is_tx)
          resume_socket(local_addr->sock_fd);
      }

      if (local_ts->source == NTP_TS_DAEMON && !UTI_IsZeroTimespec(&ts3.ts[0]) &&
          (!is_tx || UTI_IsZeroTimespec(&ts3.ts[2]))) {
        LCL_CookTime(&ts3.ts[0], &local_ts->ts, &local_ts->err);
        local_ts->source = NTP_TS_KERNEL;
      }
    }

    if ((cmsg->cmsg_level == SOL_IP && cmsg->cmsg_type == IP_RECVERR) ||
        (cmsg->cmsg_level == SOL_IPV6 && cmsg->cmsg_type == IPV6_RECVERR)) {
      struct sock_extended_err err;

      memcpy(&err, CMSG_DATA(cmsg), sizeof (err));

      if (err.ee_errno != ENOMSG || err.ee_info != SCM_TSTAMP_SND ||
          err.ee_origin != SO_EE_ORIGIN_TIMESTAMPING) {
        DEBUG_LOG("Unknown extended error");
        /* Drop the message */
        return 1;
      }
    }
  }

  /* If the kernel is slow with enabling RX timestamping, open a dummy
     socket to keep the kernel RX timestamping permanently enabled */
  if (!is_tx && local_ts->source == NTP_TS_DAEMON && ts_flags) {
    DEBUG_LOG("Missing kernel RX timestamp");
    if (dummy_rxts_socket == INVALID_SOCK_FD)
      dummy_rxts_socket = open_dummy_socket();
  }

  /* Return the message if it's not received from the error queue */
  if (!is_tx)
    return 0;

  /* The data from the error queue includes all layers up to UDP.  We have to
     extract the UDP data and also the destination address with port as there
     currently doesn't seem to be a better way to get them both. */
  l2_length = length;
  length = extract_udp_data(hdr->msg_iov[0].iov_base, remote_addr, length);

  DEBUG_LOG("Received %d (%d) bytes from error queue for %s:%d fd=%d if=%d tss=%u",
            l2_length, length, UTI_IPToString(&remote_addr->ip_addr), remote_addr->port,
            local_addr->sock_fd, local_addr->if_index, local_ts->source);

  /* Update assumed position of UDP data at layer 2 for next received packet */
  if (iface && length) {
    if (remote_addr->ip_addr.family == IPADDR_INET4)
      iface->l2_udp4_ntp_start = l2_length - length;
    else if (remote_addr->ip_addr.family == IPADDR_INET6)
      iface->l2_udp6_ntp_start = l2_length - length;
  }

  /* Drop the message if it has no timestamp or its processing failed */
  if (local_ts->source == NTP_TS_DAEMON) {
    DEBUG_LOG("Missing TX timestamp");
    return 1;
  }

  if (length < NTP_NORMAL_PACKET_LENGTH)
    return 1;

  NSR_ProcessTx(remote_addr, local_addr, local_ts,
                (NTP_Packet *)hdr->msg_iov[0].iov_base, length);

  return 1;
}

/* ================================================== */

int
NIO_Linux_RequestTxTimestamp(struct msghdr *msg, int cmsglen, int sock_fd)
{
  struct cmsghdr *cmsg;

  if (!ts_flags)
    return cmsglen;

  /* If a HW transmit timestamp is requested on a client socket, monitor
     events on the socket in order to avoid processing of a fast response
     without the HW timestamp of the request */
  if (ts_tx_flags & SOF_TIMESTAMPING_TX_HARDWARE && !NIO_IsServerSocket(sock_fd))
    monitored_socket = sock_fd;

  /* Check if TX timestamping is disabled on this socket */
  if (permanent_ts_options || !NIO_IsServerSocket(sock_fd))
    return cmsglen;

  /* Add control message that will enable TX timestamping for this message.
     Don't use CMSG_NXTHDR as the one in glibc is buggy for creating new
     control messages. */

  cmsg = CMSG_FIRSTHDR(msg);
  if (!cmsg || cmsglen + CMSG_SPACE(sizeof (ts_tx_flags)) > msg->msg_controllen)
    return cmsglen;

  cmsg = (struct cmsghdr *)((char *)cmsg + cmsglen);
  memset(cmsg, 0, CMSG_SPACE(sizeof (ts_tx_flags)));
  cmsglen += CMSG_SPACE(sizeof (ts_tx_flags));

  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SO_TIMESTAMPING;
  cmsg->cmsg_len = CMSG_LEN(sizeof (ts_tx_flags));

  memcpy(CMSG_DATA(cmsg), &ts_tx_flags, sizeof (ts_tx_flags));

  return cmsglen;
}

/* ================================================== */

void
NIO_Linux_NotifySocketClosing(int sock_fd)
{
  resume_socket(sock_fd);
}
