/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2016-2019, 2021-2023
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
#include <linux/ethtool.h>
#include <linux/net_tstamp.h>
#include <linux/sockios.h>
#include <net/if.h>

#include "array.h"
#include "conf.h"
#include "hwclock.h"
#include "local.h"
#include "logging.h"
#include "memory.h"
#include "ntp_core.h"
#include "ntp_io.h"
#include "ntp_io_linux.h"
#include "ntp_sources.h"
#include "sched.h"
#include "socket.h"
#include "sys_linux.h"
#include "util.h"

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
  /* Compensation of errors in TX and RX timestamping */
  double tx_comp;
  double rx_comp;
  HCL_Instance clock;
  int maxpoll;
  SCH_TimeoutID poll_timeout_id;
};

/* Number of PHC readings per HW clock sample */
#define PHC_READINGS 25

/* Minimum and maximum interval between PHC readings */
#define MIN_PHC_POLL -6
#define MAX_PHC_POLL 20

/* Maximum acceptable offset between SW/HW and daemon timestamp */
#define MAX_TS_DELAY 1.0

/* Array of Interfaces */
static ARR_Instance interfaces;

/* RX/TX and TX-specific timestamping socket options */
static int ts_flags;
static int ts_tx_flags;

/* Flag indicating the socket options can't be changed in control messages */
static int permanent_ts_options;

/* Unbound socket keeping the kernel RX timestamping permanently enabled
   in order to avoid a race condition between receiving a server response
   and the kernel actually starting to timestamp received packets after
   enabling the timestamping and sending a request */
static int dummy_rxts_socket;

#define INVALID_SOCK_FD -3

/* ================================================== */

static void poll_phc(struct Interface *iface, struct timespec *now);

/* ================================================== */

static int
add_interface(CNF_HwTsInterface *conf_iface)
{
  int sock_fd, if_index, minpoll, phc_fd, req_hwts_flags, rx_filter;
  struct ethtool_ts_info ts_info;
  struct hwtstamp_config ts_config;
  struct ifreq req;
  unsigned int i;
  struct Interface *iface;

  /* Check if the interface was not already added */
  for (i = 0; i < ARR_GetSize(interfaces); i++) {
    if (!strcmp(conf_iface->name, ((struct Interface *)ARR_GetElement(interfaces, i))->name))
      return 1;
  }

  sock_fd = SCK_OpenUdpSocket(NULL, NULL, NULL, 0);
  if (sock_fd < 0)
    return 0;

  memset(&req, 0, sizeof (req));
  memset(&ts_info, 0, sizeof (ts_info));

  if (snprintf(req.ifr_name, sizeof (req.ifr_name), "%s", conf_iface->name) >=
      sizeof (req.ifr_name)) {
    SCK_CloseSocket(sock_fd);
    return 0;
  }

  if (ioctl(sock_fd, SIOCGIFINDEX, &req)) {
    DEBUG_LOG("ioctl(%s) failed : %s", "SIOCGIFINDEX", strerror(errno));
    SCK_CloseSocket(sock_fd);
    return 0;
  }

  if_index = req.ifr_ifindex;

  ts_info.cmd = ETHTOOL_GET_TS_INFO;
  req.ifr_data = (char *)&ts_info;

  if (ioctl(sock_fd, SIOCETHTOOL, &req)) {
    DEBUG_LOG("ioctl(%s) failed : %s", "SIOCETHTOOL", strerror(errno));
    SCK_CloseSocket(sock_fd);
    return 0;
  }

  req_hwts_flags = SOF_TIMESTAMPING_RX_HARDWARE | SOF_TIMESTAMPING_TX_HARDWARE |
                   SOF_TIMESTAMPING_RAW_HARDWARE;
  if ((ts_info.so_timestamping & req_hwts_flags) != req_hwts_flags) {
    DEBUG_LOG("HW timestamping not supported on %s", req.ifr_name);
    SCK_CloseSocket(sock_fd);
    return 0;
  }

  if (ts_info.phc_index < 0) {
    DEBUG_LOG("PHC missing on %s", req.ifr_name);
    SCK_CloseSocket(sock_fd);
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
    case CNF_HWTS_RXFILTER_PTP:
      if (ts_info.rx_filters & (1 << HWTSTAMP_FILTER_PTP_V2_L4_EVENT))
        rx_filter = HWTSTAMP_FILTER_PTP_V2_L4_EVENT;
      else if (ts_info.rx_filters & (1 << HWTSTAMP_FILTER_PTP_V2_EVENT))
        rx_filter = HWTSTAMP_FILTER_PTP_V2_EVENT;
      else
        rx_filter = HWTSTAMP_FILTER_NONE;
      break;
    default:
      rx_filter = HWTSTAMP_FILTER_ALL;
      break;
  }

  ts_config.flags = 0;
  ts_config.tx_type = HWTSTAMP_TX_ON;
  ts_config.rx_filter = rx_filter;
  req.ifr_data = (char *)&ts_config;

  if (ioctl(sock_fd, SIOCSHWTSTAMP, &req)) {
    LOG(errno == EPERM ? LOGS_ERR : LOGS_DEBUG,
        "ioctl(%s) failed : %s", "SIOCSHWTSTAMP", strerror(errno));

    /* Check the current timestamping configuration in case this interface
       allows only reading of the configuration and it was already configured
       as requested */
    req.ifr_data = (char *)&ts_config;
#ifdef SIOCGHWTSTAMP
    if (ioctl(sock_fd, SIOCGHWTSTAMP, &req) ||
        ts_config.tx_type != HWTSTAMP_TX_ON || ts_config.rx_filter != rx_filter)
#endif
    {
      SCK_CloseSocket(sock_fd);
      return 0;
    }
  }

  SCK_CloseSocket(sock_fd);

  phc_fd = SYS_Linux_OpenPHC(req.ifr_name);
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

  iface->tx_comp = conf_iface->tx_comp;
  iface->rx_comp = conf_iface->rx_comp;

  minpoll = CLAMP(MIN_PHC_POLL, conf_iface->minpoll, MAX_PHC_POLL);
  iface->clock = HCL_CreateInstance(conf_iface->min_samples, conf_iface->max_samples,
                                    UTI_Log2ToDouble(minpoll), conf_iface->precision);

  iface->maxpoll = CLAMP(minpoll, conf_iface->maxpoll, MAX_PHC_POLL);

  /* Do not schedule the first poll timeout here!  The argument (interface) can
     move until all interfaces are added.  Wait for the first HW timestamp. */
  iface->poll_timeout_id = 0;

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

  sock_fd = SCK_OpenUdpSocket(NULL, NULL, NULL, 0);
  if (sock_fd < 0)
    return;

  memset(&req, 0, sizeof (req));
  memset(&cmd, 0, sizeof (cmd));

  snprintf(req.ifr_name, sizeof (req.ifr_name), "%s", iface->name);
  cmd.cmd = ETHTOOL_GSET;
  req.ifr_data = (char *)&cmd;

  if (ioctl(sock_fd, SIOCETHTOOL, &req)) {
    DEBUG_LOG("ioctl(%s) failed : %s", "SIOCETHTOOL", strerror(errno));
    SCK_CloseSocket(sock_fd);
    return;
  }

  SCK_CloseSocket(sock_fd);

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

  sock_fd = SCK_OpenUdpSocket(NULL, NULL, NULL, 0);
  if (sock_fd < 0)
    return 0;

  if (!SCK_SetIntOption(sock_fd, SOL_SOCKET, SO_TIMESTAMPING, option)) {
    SCK_CloseSocket(sock_fd);
    return 0;
  }

  SCK_CloseSocket(sock_fd);
  return 1;
}
#endif

/* ================================================== */

static int
open_dummy_socket(void)
{
  int sock_fd, events = 0;

  sock_fd = SCK_OpenUdpSocket(NULL, NULL, NULL, 0);
  if (sock_fd < 0)
    return INVALID_SOCK_FD;

  if (!NIO_Linux_SetTimestampSocketOptions(sock_fd, 1, &events)) {
    SCK_CloseSocket(sock_fd);
    return INVALID_SOCK_FD;
  }

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

  dummy_rxts_socket = INVALID_SOCK_FD;
}

/* ================================================== */

void
NIO_Linux_Finalise(void)
{
  struct Interface *iface;
  unsigned int i;

  if (dummy_rxts_socket != INVALID_SOCK_FD)
    SCK_CloseSocket(dummy_rxts_socket);

  for (i = 0; i < ARR_GetSize(interfaces); i++) {
    iface = ARR_GetElement(interfaces, i);
    SCH_RemoveTimeout(iface->poll_timeout_id);
    HCL_DestroyInstance(iface->clock);
    close(iface->phc_fd);
  }

  ARR_DestroyInstance(interfaces);
}

/* ================================================== */

int
NIO_Linux_IsHwTsEnabled(void)
{
  return ARR_GetSize(interfaces) > 0;
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

  if (!SCK_SetIntOption(sock_fd, SOL_SOCKET, SO_SELECT_ERR_QUEUE, val)) {
    ts_flags = 0;
    return 0;
  }

  if (!SCK_SetIntOption(sock_fd, SOL_SOCKET, SO_TIMESTAMPING, flags)) {
    ts_flags = 0;
    return 0;
  }

  *events |= SCH_FILE_EXCEPTION;
  return 1;
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
poll_timeout(void *arg)
{
  struct Interface *iface = arg;
  struct timespec now;

  iface->poll_timeout_id = 0;

  SCH_GetLastEventTime(&now, NULL, NULL);
  poll_phc(iface, &now);
}

/* ================================================== */

static void
poll_phc(struct Interface *iface, struct timespec *now)
{
  struct timespec sample_phc_ts, sample_sys_ts, sample_local_ts;
  struct timespec phc_readings[PHC_READINGS][3];
  double phc_err, local_err, interval;
  int n_readings;

  if (!HCL_NeedsNewSample(iface->clock, now))
    return;

  DEBUG_LOG("Polling PHC on %s%s",
            iface->name, iface->poll_timeout_id != 0 ? " before timeout" : "");

  n_readings = SYS_Linux_GetPHCReadings(iface->phc_fd, iface->phc_nocrossts,
                                        &iface->phc_mode, PHC_READINGS, phc_readings);

  /* Add timeout for the next poll in case no HW timestamp will be captured
     between the minpoll and maxpoll.  Separate reading of different PHCs to
     avoid long intervals between handling I/O events. */
  SCH_RemoveTimeout(iface->poll_timeout_id);
  interval = UTI_Log2ToDouble(iface->maxpoll);
  iface->poll_timeout_id = SCH_AddTimeoutInClass(interval, interval /
                                                   ARR_GetSize(interfaces) / 4, 0.1,
                                                 SCH_PhcPollClass, poll_timeout, iface);

  if (n_readings <= 0)
    return;

  if (!HCL_ProcessReadings(iface->clock, n_readings, phc_readings,
                           &sample_phc_ts, &sample_sys_ts, &phc_err))
    return;

  LCL_CookTime(&sample_sys_ts, &sample_local_ts, &local_err);
  HCL_AccumulateSample(iface->clock, &sample_phc_ts, &sample_local_ts, phc_err + local_err);

  update_interface_speed(iface);
}

/* ================================================== */

static void
process_hw_timestamp(struct Interface *iface, struct timespec *hw_ts,
                     NTP_Local_Timestamp *local_ts, int rx_ntp_length, int family,
                     int l2_length)
{
  double rx_correction = 0.0, ts_delay, local_err;
  struct timespec ts;

  poll_phc(iface, &local_ts->ts);

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
  local_ts->rx_duration = rx_correction;
  /* Network correction needs to include the RX duration to avoid
     asymmetric correction with asymmetric link speeds */
  local_ts->net_correction = rx_correction;
}

/* ================================================== */

static void
process_sw_timestamp(struct timespec *sw_ts, NTP_Local_Timestamp *local_ts)
{
  double ts_delay, local_err;
  struct timespec ts;

  LCL_CookTime(sw_ts, &ts, &local_err);

  ts_delay = UTI_DiffTimespecsToDouble(&local_ts->ts, &ts);

  if (fabs(ts_delay) > MAX_TS_DELAY) {
    DEBUG_LOG("Unacceptable timestamp delay %.9f", ts_delay);
    return;
  }

  local_ts->ts = ts;
  local_ts->err = local_err;
  local_ts->source = NTP_TS_KERNEL;
}

/* ================================================== */
/* Extract UDP data from a layer 2 message.  Supported is Ethernet
   with optional VLAN tags. */

static int
extract_udp_data(unsigned char *msg, NTP_Remote_Address *remote_addr, int len)
{
  unsigned char *msg_start = msg;

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
    uint32_t addr;

    if (len < ihl + 8 || msg[9] != 17)
      return 0;

    memcpy(&addr, msg + 16, sizeof (addr));
    remote_addr->ip_addr.addr.in4 = ntohl(addr);
    remote_addr->port = ntohs(*(uint16_t *)(msg + ihl + 2));
    remote_addr->ip_addr.family = IPADDR_INET4;
    len -= ihl + 8, msg += ihl + 8;
#ifdef FEAT_IPV6
  } else if (len >= 48 && msg[0] >> 4 == 6) {
    int eh_len, next_header = msg[6];

    memcpy(&remote_addr->ip_addr.addr.in6, msg + 24, sizeof (remote_addr->ip_addr.addr.in6));
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

    remote_addr->port = ntohs(*(uint16_t *)(msg + 2));
    remote_addr->ip_addr.family = IPADDR_INET6;
    len -= 8, msg += 8;
#endif
  } else {
    return 0;
  }

  /* Move the message to fix alignment of its fields */
  if (len > 0)
    memmove(msg_start, msg, len);

  return len;
}

/* ================================================== */

int
NIO_Linux_ProcessMessage(SCK_Message *message, NTP_Local_Address *local_addr,
                         NTP_Local_Timestamp *local_ts, int event)
{
  struct Interface *iface;
  int is_tx, ts_if_index, l2_length;
  double c = 0.0;

  is_tx = event == SCH_FILE_EXCEPTION;
  iface = NULL;

  ts_if_index = message->timestamp.if_index;
  if (ts_if_index == INVALID_IF_INDEX)
    ts_if_index = message->if_index;
  l2_length = message->timestamp.l2_length;

  if (!UTI_IsZeroTimespec(&message->timestamp.hw)) {
    iface = get_interface(ts_if_index);
    if (iface) {
      process_hw_timestamp(iface, &message->timestamp.hw, local_ts, !is_tx ? message->length : 0,
                           message->remote_addr.ip.ip_addr.family, l2_length);
    } else {
      DEBUG_LOG("HW clock not found for interface %d", ts_if_index);
    }
  }

  if (local_ts->source == NTP_TS_DAEMON && !UTI_IsZeroTimespec(&message->timestamp.kernel) &&
      (!is_tx || UTI_IsZeroTimespec(&message->timestamp.hw))) {
    process_sw_timestamp(&message->timestamp.kernel, local_ts);
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
  l2_length = message->length;
  message->length = extract_udp_data(message->data, &message->remote_addr.ip, message->length);

  DEBUG_LOG("Extracted message for %s fd=%d len=%d",
            UTI_IPSockAddrToString(&message->remote_addr.ip),
            local_addr->sock_fd, message->length);

  /* Update assumed position of UDP data at layer 2 for next received packet */
  if (iface && message->length) {
    if (message->remote_addr.ip.ip_addr.family == IPADDR_INET4)
      iface->l2_udp4_ntp_start = l2_length - message->length;
    else if (message->remote_addr.ip.ip_addr.family == IPADDR_INET6)
      iface->l2_udp6_ntp_start = l2_length - message->length;
  }

  /* Drop the message if it has no timestamp or its processing failed */
  if (local_ts->source == NTP_TS_DAEMON) {
    DEBUG_LOG("Missing TX timestamp");
    return 1;
  }

  if (!NIO_UnwrapMessage(message, local_addr->sock_fd, &c))
    return 1;

  if (message->length < NTP_HEADER_LENGTH || message->length > sizeof (NTP_Packet))
    return 1;

  NSR_ProcessTx(&message->remote_addr.ip, local_addr, local_ts, message->data, message->length);

  return 1;
}

/* ================================================== */

void
NIO_Linux_RequestTxTimestamp(SCK_Message *message, int sock_fd)
{
  if (!ts_flags)
    return;

  /* Check if TX timestamping is disabled on this socket */
  if (permanent_ts_options || !NIO_IsServerSocket(sock_fd))
    return;

  message->timestamp.tx_flags = ts_tx_flags;
}
