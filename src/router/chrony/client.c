/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * Copyright (C) Lonnie Abelbeck  2016, 2018
 * Copyright (C) Miroslav Lichvar  2009-2024
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

  Command line client for configuring the daemon and obtaining status
  from it whilst running.
  */

#include "config.h"

#include "sysincl.h"

#include "array.h"
#include "candm.h"
#include "cmac.h"
#include "logging.h"
#include "memory.h"
#include "nameserv.h"
#include "getdate.h"
#include "cmdparse.h"
#include "pktlength.h"
#include "socket.h"
#include "util.h"

#ifdef FEAT_READLINE
#include <editline/readline.h>
#endif

/* ================================================== */

struct Address {
  SCK_AddressType type;
  union {
    IPSockAddr ip;
    char *path;
  } addr;
};

static ARR_Instance server_addresses;

static int sock_fd = -1;

static volatile int quit = 0;

static int on_terminal = 0;

static int no_dns = 0;

static int source_names = 0;

static int csv_mode = 0;

static int end_dot = 0;

/* ================================================== */
/* Log a message. This is a minimalistic replacement of the logging.c
   implementation to avoid linking with it and other modules. */

LOG_Severity log_min_severity = LOGS_INFO;

void LOG_Message(LOG_Severity severity,
#if DEBUG > 0
                 int line_number, const char *filename, const char *function_name,
#endif
                 const char *format, ...)
{
  va_list ap;

  if (severity < log_min_severity)
    return;

  va_start(ap, format);
  vfprintf(stderr, format, ap);
  putc('\n', stderr);
  va_end(ap);
}

/* ================================================== */
/* Read a single line of commands from standard input */

#ifdef FEAT_READLINE
static char **command_name_completion(const char *text, int start, int end);
#endif

static char *
read_line(void)
{
  static char line[2048];
  static const char *prompt = "chronyc> ";

  if (on_terminal) {
#ifdef FEAT_READLINE
    char *cmd;

    rl_attempted_completion_function = command_name_completion;
    rl_basic_word_break_characters = " \t\n\r";

    /* save line only if not empty */
    cmd = readline(prompt);
    if( cmd == NULL ) return( NULL );
    
    /* user pressed return */
    if( *cmd != '\0' ) {
      strncpy(line, cmd, sizeof(line) - 1);
      line[sizeof(line) - 1] = '\0';
      add_history(cmd);
    } else {
      /* simulate the user has entered an empty line */
      *line = '\0';
    }
    Free(cmd);
    return( line );
#else
    printf("%s", prompt);
    fflush(stdout);
#endif
  }
  if (fgets(line, sizeof(line), stdin)) {
    return line;
  } else {
    return NULL;
  }
  
}

/* ================================================== */

static ARR_Instance
get_addresses(const char *hostnames, int port)
{
  struct Address *addr;
  ARR_Instance addrs;
  char *hostname, *s1, *s2;
  IPAddr ip_addrs[DNS_MAX_ADDRESSES];
  int i;

  addrs = ARR_CreateInstance(sizeof (*addr));
  s1 = Strdup(hostnames);

  /* Parse the comma-separated list of hostnames */
  for (hostname = s1; hostname && *hostname; hostname = s2) {
    s2 = strchr(hostname, ',');
    if (s2)
      *s2++ = '\0';

    /* hostname starting with / is considered a path of Unix domain socket */
    if (hostname[0] == '/') {
      addr = ARR_GetNewElement(addrs);
      addr->type = SCK_ADDR_UNIX;
      addr->addr.path = Strdup(hostname);
    } else {
      if (DNS_Name2IPAddress(hostname, ip_addrs, DNS_MAX_ADDRESSES) != DNS_Success) {
        DEBUG_LOG("Could not get IP address for %s", hostname);
        continue;
      }

      for (i = 0; i < DNS_MAX_ADDRESSES && ip_addrs[i].family != IPADDR_UNSPEC; i++) {
        addr = ARR_GetNewElement(addrs);
        addr->type = SCK_ADDR_IP;
        addr->addr.ip.ip_addr = ip_addrs[i];
        addr->addr.ip.port = port;
        DEBUG_LOG("Resolved %s to %s", hostname, UTI_IPToString(&ip_addrs[i]));
      }
    }
  }

  Free(s1);
  return addrs;
}

/* ================================================== */

static void
free_addresses(ARR_Instance addresses)
{
  struct Address *addr;
  unsigned int i;

  for (i = 0; i < ARR_GetSize(addresses); i++) {
    addr = ARR_GetElement(addresses, i);

    if (addr->type == SCK_ADDR_UNIX)
      Free(addr->addr.path);
  }

  ARR_DestroyInstance(addresses);
}

/* ================================================== */
/* Initialise the socket used to talk to the daemon */

static int
open_socket(struct Address *addr)
{
  char *dir, *local_addr;
  size_t local_addr_len;

  switch (addr->type) {
    case SCK_ADDR_IP:
      sock_fd = SCK_OpenUdpSocket(&addr->addr.ip, NULL, NULL, 0);
      break;
    case SCK_ADDR_UNIX:
      /* Construct path of our socket.  Use the same directory as the server
         socket and include our process ID to allow multiple chronyc instances
         running at the same time. */

      dir = UTI_PathToDir(addr->addr.path);
      local_addr_len = strlen(dir) + 50;
      local_addr = Malloc(local_addr_len);

      snprintf(local_addr, local_addr_len, "%s/chronyc.%d.sock", dir, (int)getpid());

      sock_fd = SCK_OpenUnixDatagramSocket(addr->addr.path, local_addr,
                                           SCK_FLAG_ALL_PERMISSIONS);
      Free(dir);
      Free(local_addr);

      break;
    default:
      assert(0);
  }

  if (sock_fd < 0)
    return 0;

  return 1;
}

/* ================================================== */

static void
close_io(void)
{
  if (sock_fd < 0)
    return;

  SCK_RemoveSocket(sock_fd);
  SCK_CloseSocket(sock_fd);
  sock_fd = -1;
}

/* ================================================== */

static int
open_io(void)
{
  static unsigned int address_index = 0;
  struct Address *addr;

  /* If a socket is already opened, close it and try the next address */
  if (sock_fd >= 0) {
    close_io();
    address_index++;
  }

  /* Find an address for which a socket can be opened and connected */
  for (; address_index < ARR_GetSize(server_addresses); address_index++) {
    addr = ARR_GetElement(server_addresses, address_index);

    if (open_socket(addr))
      return 1;

    close_io();
  }

  /* Start from the first address if called again */
  address_index = 0;

  return 0;
}

/* ================================================== */

static void
bits_to_mask(int bits, int family, IPAddr *mask)
{
  int i;

  mask->family = family;
  switch (family) {
    case IPADDR_INET4:
      if (bits > 32 || bits < 0)
        bits = 32;
      if (bits > 0) {
        mask->addr.in4 = -1;
        mask->addr.in4 <<= 32 - bits;
      } else {
        mask->addr.in4 = 0;
      }
      break;
    case IPADDR_INET6:
      if (bits > 128 || bits < 0)
        bits = 128;
      for (i = 0; i < bits / 8; i++)
        mask->addr.in6[i] = 0xff;
      if (i < 16)
        mask->addr.in6[i++] = (0xff << (8 - bits % 8)) & 0xff;
      for (; i < 16; i++)
        mask->addr.in6[i] = 0x0;
      break;
    case IPADDR_ID:
      mask->family = IPADDR_UNSPEC;
      break;
    default:
      assert(0);
  }
}

/* ================================================== */

static int
parse_source_address(char *word, IPAddr *address)
{
  if (UTI_StringToIdIP(word, address))
    return 1;

  if (DNS_Name2IPAddress(word, address, 1) == DNS_Success)
    return 1;

  return 0;
}

/* ================================================== */

static int
parse_source_address_or_refid(char *s, IPAddr *address, uint32_t *ref_id)
{
  address->family = IPADDR_UNSPEC;
  *ref_id = 0;

  /* Don't allow hostnames to avoid conflicts with reference IDs */
  if (UTI_StringToIdIP(s, address) || UTI_StringToIP(s, address))
    return 1;

  if (CPS_ParseRefid(s, ref_id) > 0)
    return 1;

  return 0;
}

/* ================================================== */

static int
read_mask_address(char *line, IPAddr *mask, IPAddr *address)
{
  unsigned int bits;
  char *p, *q;

  p = line;
  if (!*p) {
    mask->family = address->family = IPADDR_UNSPEC;
    return 1;
  } else {
    q = strchr(p, '/');
    if (q) {
      *q++ = 0;
      if (UTI_StringToIP(p, mask)) {
        p = q;
        if (UTI_StringToIP(p, address)) {
          if (address->family == mask->family)
            return 1;
        } else if (sscanf(p, "%u", &bits) == 1) {
          *address = *mask;
          bits_to_mask(bits, address->family, mask);
          return 1;
        }
      }
    } else {
      if (parse_source_address(p, address)) {
        bits_to_mask(-1, address->family, mask);
        return 1;
      } else {
        LOG(LOGS_ERR, "Could not get address for hostname");
        return 0;
      }
    }
  }

  LOG(LOGS_ERR, "Invalid syntax for mask/address");
  return 0;
}

/* ================================================== */

static int
process_cmd_offline(CMD_Request *msg, char *line)
{
  IPAddr mask, address;
  int ok;

  if (read_mask_address(line, &mask, &address)) {
    UTI_IPHostToNetwork(&mask, &msg->data.offline.mask);
    UTI_IPHostToNetwork(&address, &msg->data.offline.address);
    msg->command = htons(REQ_OFFLINE);
    ok = 1;
  } else {
    ok = 0;
  }

  return ok;

}

/* ================================================== */


static int
process_cmd_online(CMD_Request *msg, char *line)
{
  IPAddr mask, address;
  int ok;

  if (read_mask_address(line, &mask, &address)) {
    UTI_IPHostToNetwork(&mask, &msg->data.online.mask);
    UTI_IPHostToNetwork(&address, &msg->data.online.address);
    msg->command = htons(REQ_ONLINE);
    ok = 1;
  } else {
    ok = 0;
  }

  return ok;

}

/* ================================================== */

static void
process_cmd_onoffline(CMD_Request *msg, char *line)
{
  msg->command = htons(REQ_ONOFFLINE);
}

/* ================================================== */

static int
read_address_integer(char *line, IPAddr *address, int *value)
{
  char *hostname;
  int ok = 0;

  hostname = line;
  line = CPS_SplitWord(line);

  if (sscanf(line, "%d", value) != 1) {
    LOG(LOGS_ERR, "Invalid syntax for address value");
    ok = 0;
  } else {
    if (!parse_source_address(hostname, address)) {
      LOG(LOGS_ERR, "Could not get address for hostname");
      ok = 0;
    } else {
      ok = 1;
    }
  }

  return ok;

}


/* ================================================== */

static int
read_address_double(char *line, IPAddr *address, double *value)
{
  char *hostname;
  int ok = 0;

  hostname = line;
  line = CPS_SplitWord(line);

  if (sscanf(line, "%lf", value) != 1) {
    LOG(LOGS_ERR, "Invalid syntax for address value");
    ok = 0;
  } else {
    if (!parse_source_address(hostname, address)) {
      LOG(LOGS_ERR, "Could not get address for hostname");
      ok = 0;
    } else {
      ok = 1;
    }
  }

  return ok;

}


/* ================================================== */

static int
process_cmd_minpoll(CMD_Request *msg, char *line)
{
  IPAddr address;
  int minpoll;
  int ok;
  
  if (read_address_integer(line, &address, &minpoll)) {
    UTI_IPHostToNetwork(&address, &msg->data.modify_minpoll.address);
    msg->data.modify_minpoll.new_minpoll = htonl(minpoll);
    msg->command = htons(REQ_MODIFY_MINPOLL);
    ok = 1;
  } else {
    ok = 0;
  }

  return ok;

}

/* ================================================== */

static int
process_cmd_maxpoll(CMD_Request *msg, char *line)
{
  IPAddr address;
  int maxpoll;
  int ok;
  
  if (read_address_integer(line, &address, &maxpoll)) {
    UTI_IPHostToNetwork(&address, &msg->data.modify_maxpoll.address);
    msg->data.modify_maxpoll.new_maxpoll = htonl(maxpoll);
    msg->command = htons(REQ_MODIFY_MAXPOLL);
    ok = 1;
  } else {
    ok = 0;
  }

  return ok;

}

/* ================================================== */

static int
process_cmd_maxdelay(CMD_Request *msg, char *line)
{
  IPAddr address;
  double max_delay;
  int ok;
  
  if (read_address_double(line, &address, &max_delay)) {
    UTI_IPHostToNetwork(&address, &msg->data.modify_maxdelay.address);
    msg->data.modify_maxdelay.new_max_delay = UTI_FloatHostToNetwork(max_delay);
    msg->command = htons(REQ_MODIFY_MAXDELAY);
    ok = 1;
  } else {
    ok = 0;
  }

  return ok;

}

/* ================================================== */

static int
process_cmd_maxdelaydevratio(CMD_Request *msg, char *line)
{
  IPAddr address;
  double max_delay_dev_ratio;
  int ok;
  
  if (read_address_double(line, &address, &max_delay_dev_ratio)) {
    UTI_IPHostToNetwork(&address, &msg->data.modify_maxdelaydevratio.address);
    msg->data.modify_maxdelayratio.new_max_delay_ratio = UTI_FloatHostToNetwork(max_delay_dev_ratio);
    msg->command = htons(REQ_MODIFY_MAXDELAYDEVRATIO);
    ok = 1;
  } else {
    ok = 0;
  }

  return ok;

}

/* ================================================== */

static int
process_cmd_maxdelayratio(CMD_Request *msg, char *line)
{
  IPAddr address;
  double max_delay_ratio;
  int ok;
  
  if (read_address_double(line, &address, &max_delay_ratio)) {
    UTI_IPHostToNetwork(&address, &msg->data.modify_maxdelayratio.address);
    msg->data.modify_maxdelayratio.new_max_delay_ratio = UTI_FloatHostToNetwork(max_delay_ratio);
    msg->command = htons(REQ_MODIFY_MAXDELAYRATIO);
    ok = 1;
  } else {
    ok = 0;
  }

  return ok;

}

/* ================================================== */

static int
process_cmd_minstratum(CMD_Request *msg, char *line)
{
  IPAddr address;
  int min_stratum;
  int ok;
  
  if (read_address_integer(line, &address, &min_stratum)) {
    UTI_IPHostToNetwork(&address, &msg->data.modify_minstratum.address);
    msg->data.modify_minstratum.new_min_stratum = htonl(min_stratum);
    msg->command = htons(REQ_MODIFY_MINSTRATUM);
    ok = 1;
  } else {
    ok = 0;
  }

  return ok;

}

/* ================================================== */

static int
process_cmd_polltarget(CMD_Request *msg, char *line)
{
  IPAddr address;
  int poll_target;
  int ok;
  
  if (read_address_integer(line, &address, &poll_target)) {
    UTI_IPHostToNetwork(&address, &msg->data.modify_polltarget.address);
    msg->data.modify_polltarget.new_poll_target = htonl(poll_target);
    msg->command = htons(REQ_MODIFY_POLLTARGET);
    ok = 1;
  } else {
    ok = 0;
  }

  return ok;

}

/* ================================================== */

static int
process_cmd_maxupdateskew(CMD_Request *msg, char *line)
{
  int ok;
  double new_max_update_skew;
  
  if (sscanf(line, "%lf", &new_max_update_skew) == 1) {
    msg->data.modify_maxupdateskew.new_max_update_skew = UTI_FloatHostToNetwork(new_max_update_skew);
    msg->command = htons(REQ_MODIFY_MAXUPDATESKEW);
    ok = 1;
  } else {
    ok = 0;
  }

  return ok;

}

/* ================================================== */

static void
process_cmd_dump(CMD_Request *msg, char *line)
{
  msg->command = htons(REQ_DUMP);
  msg->data.dump.pad = htonl(0);
}

/* ================================================== */

static void
process_cmd_writertc(CMD_Request *msg, char *line)
{
  msg->command = htons(REQ_WRITERTC);
}

/* ================================================== */

static void
process_cmd_trimrtc(CMD_Request *msg, char *line)
{
  msg->command = htons(REQ_TRIMRTC);
}

/* ================================================== */

static void
process_cmd_cyclelogs(CMD_Request *msg, char *line)
{
  msg->command = htons(REQ_CYCLELOGS);
}

/* ================================================== */

static int
process_cmd_burst(CMD_Request *msg, char *line)
{
  int n_good_samples, n_total_samples;
  char *s1, *s2;
  IPAddr address, mask;

  s1 = line;
  s2 = CPS_SplitWord(s1);
  CPS_SplitWord(s2);

  if (sscanf(s1, "%d/%d", &n_good_samples, &n_total_samples) != 2) {
    LOG(LOGS_ERR, "Invalid syntax for burst command");
    return 0;
  }

  mask.family = address.family = IPADDR_UNSPEC;
  if (*s2 && !read_mask_address(s2, &mask, &address)) {
    return 0;
  }

  msg->command = htons(REQ_BURST);
  msg->data.burst.n_good_samples = ntohl(n_good_samples);
  msg->data.burst.n_total_samples = ntohl(n_total_samples);

  UTI_IPHostToNetwork(&mask, &msg->data.burst.mask);
  UTI_IPHostToNetwork(&address, &msg->data.burst.address);

  return 1;
}

/* ================================================== */

static int
process_cmd_local(CMD_Request *msg, char *line)
{
  double distance = 0.0, activate = 0.0, wait_synced = 0.0, wait_unsynced = 0.0;
  int on_off, stratum = 0, orphan = 0;

  if (!strcmp(line, "off")) {
    on_off = 0;
  } else if (CPS_ParseLocal(line, &stratum, &orphan, &distance, &activate,
                            &wait_synced, &wait_unsynced) == CPS_Success) {
    on_off = 1;
  } else {
    LOG(LOGS_ERR, "Invalid syntax for local command");
    return 0;
  }

  msg->command = htons(REQ_LOCAL3);
  msg->data.local.on_off = htonl(on_off);
  msg->data.local.stratum = htonl(stratum);
  msg->data.local.distance = UTI_FloatHostToNetwork(distance);
  msg->data.local.orphan = htonl(orphan);
  msg->data.local.activate = UTI_FloatHostToNetwork(activate);
  msg->data.local.wait_synced = UTI_FloatHostToNetwork(wait_synced);
  msg->data.local.wait_unsynced = UTI_FloatHostToNetwork(wait_unsynced);

  return 1;
}

/* ================================================== */

static int
process_cmd_manual(CMD_Request *msg, const char *line)
{
  const char *p;

  p = line;

  if (!strcmp(p, "off")) {
    msg->data.manual.option = htonl(0);
  } else if (!strcmp(p, "on")) {
    msg->data.manual.option = htonl(1);
  } else if (!strcmp(p, "reset")) {
    msg->data.manual.option = htonl(2);
  } else {
    LOG(LOGS_ERR, "Invalid syntax for manual command");
    return 0;
  }
  msg->command = htons(REQ_MANUAL);

  return 1;
}

/* ================================================== */

static int
process_cmd_allowdeny(CMD_Request *msg, char *line, int cmd, int allcmd)
{
  int all, subnet_bits;
  IPAddr ip;
  
  if (!CPS_ParseAllowDeny(line, &all, &ip, &subnet_bits)) {
    LOG(LOGS_ERR, "Could not read address");
    return 0;
  }

  msg->command = htons(all ? allcmd : cmd);
  UTI_IPHostToNetwork(&ip, &msg->data.allow_deny.ip);
  msg->data.allow_deny.subnet_bits = htonl(subnet_bits);

  return 1;
}

/* ================================================== */

static int
process_cmd_accheck(CMD_Request *msg, char *line)
{
  IPAddr ip;
  msg->command = htons(REQ_ACCHECK);
  if (DNS_Name2IPAddress(line, &ip, 1) == DNS_Success) {
    UTI_IPHostToNetwork(&ip, &msg->data.ac_check.ip);
    return 1;
  } else {    
    LOG(LOGS_ERR, "Could not read address");
    return 0;
  }
}

/* ================================================== */

static int
process_cmd_cmdaccheck(CMD_Request *msg, char *line)
{
  IPAddr ip;
  msg->command = htons(REQ_CMDACCHECK);
  if (DNS_Name2IPAddress(line, &ip, 1) == DNS_Success) {
    UTI_IPHostToNetwork(&ip, &msg->data.ac_check.ip);
    return 1;
  } else {    
    LOG(LOGS_ERR, "Could not read address");
    return 0;
  }
}

/* ================================================== */

static int
process_cmd_dfreq(CMD_Request *msg, char *line)
{
  double dfreq;

  msg->command = htons(REQ_DFREQ);

  if (sscanf(line, "%lf", &dfreq) != 1) {
    LOG(LOGS_ERR, "Invalid value");
    return 0;
  }

  msg->data.dfreq.dfreq = UTI_FloatHostToNetwork(dfreq);
  return 1;
}

/* ================================================== */

static int
process_cmd_doffset(CMD_Request *msg, char *line)
{
  double doffset;

  msg->command = htons(REQ_DOFFSET2);

  if (sscanf(line, "%lf", &doffset) != 1) {
    LOG(LOGS_ERR, "Invalid value");
    return 0;
  }

  msg->data.doffset.doffset = UTI_FloatHostToNetwork(doffset);
  return 1;
}

/* ================================================== */

static int
convert_addsrc_sel_options(int options)
{
  return (options & SRC_SELECT_PREFER ? REQ_ADDSRC_PREFER : 0) |
         (options & SRC_SELECT_NOSELECT ? REQ_ADDSRC_NOSELECT : 0) |
         (options & SRC_SELECT_TRUST ? REQ_ADDSRC_TRUST : 0) |
         (options & SRC_SELECT_REQUIRE ? REQ_ADDSRC_REQUIRE : 0);
}

/* ================================================== */

static int
process_cmd_add_source(CMD_Request *msg, char *line)
{
  CPS_NTP_Source data;
  CPS_Status status;
  IPAddr ip_addr;
  int result = 0, type;
  const char *opt_name, *word;
  
  msg->command = htons(REQ_ADD_SOURCE);

  word = line;
  line = CPS_SplitWord(line);

  if (!strcasecmp(word, "server")) {
    type = REQ_ADDSRC_SERVER;
  } else if (!strcasecmp(word, "peer")) {
    type = REQ_ADDSRC_PEER;
  } else if (!strcasecmp(word, "pool")) {
    type = REQ_ADDSRC_POOL;
  } else {
    LOG(LOGS_ERR, "Invalid syntax for add command");
    return 0;
  }

  status = CPS_ParseNTPSourceAdd(line, &data);
  switch (status) {
    case CPS_Success:
      /* Verify that the address is resolvable (chronyc and chronyd are
         assumed to be running on the same host) */
      if (strlen(data.name) >= sizeof (msg->data.ntp_source.name) ||
          DNS_Name2IPAddress(data.name, &ip_addr, 1) != DNS_Success) {
        LOG(LOGS_ERR, "Invalid host/IP address");
        break;
      }

      opt_name = NULL;
      if (opt_name) {
        LOG(LOGS_ERR, "%s can't be set in chronyc", opt_name);
        break;
      }

      msg->data.ntp_source.type = htonl(type);
      BRIEF_ASSERT(strlen(data.name) < sizeof (msg->data.ntp_source.name));
      strncpy((char *)msg->data.ntp_source.name, data.name,
              sizeof (msg->data.ntp_source.name));
      msg->data.ntp_source.port = htonl(data.port);
      msg->data.ntp_source.minpoll = htonl(data.params.minpoll);
      msg->data.ntp_source.maxpoll = htonl(data.params.maxpoll);
      msg->data.ntp_source.presend_minpoll = htonl(data.params.presend_minpoll);
      msg->data.ntp_source.min_stratum = htonl(data.params.min_stratum);
      msg->data.ntp_source.poll_target = htonl(data.params.poll_target);
      msg->data.ntp_source.version = htonl(data.params.version);
      msg->data.ntp_source.max_sources = htonl(data.params.max_sources);
      msg->data.ntp_source.min_samples = htonl(data.params.min_samples);
      msg->data.ntp_source.max_samples = htonl(data.params.max_samples);
      msg->data.ntp_source.authkey = htonl(data.params.authkey);
      msg->data.ntp_source.nts_port = htonl(data.params.nts_port);
      msg->data.ntp_source.max_delay = UTI_FloatHostToNetwork(data.params.max_delay);
      msg->data.ntp_source.max_delay_ratio = UTI_FloatHostToNetwork(data.params.max_delay_ratio);
      msg->data.ntp_source.max_delay_dev_ratio =
        UTI_FloatHostToNetwork(data.params.max_delay_dev_ratio);
      msg->data.ntp_source.min_delay = UTI_FloatHostToNetwork(data.params.min_delay);
      msg->data.ntp_source.asymmetry = UTI_FloatHostToNetwork(data.params.asymmetry);
      msg->data.ntp_source.offset = UTI_FloatHostToNetwork(data.params.offset);
      msg->data.ntp_source.flags = htonl(
          (data.params.connectivity == SRC_ONLINE ? REQ_ADDSRC_ONLINE : 0) |
          (data.params.auto_offline ? REQ_ADDSRC_AUTOOFFLINE : 0) |
          (data.params.iburst ? REQ_ADDSRC_IBURST : 0) |
          (data.params.interleaved ? REQ_ADDSRC_INTERLEAVED : 0) |
          (data.params.burst ? REQ_ADDSRC_BURST : 0) |
          (data.params.nts ? REQ_ADDSRC_NTS : 0) |
          (data.params.copy ? REQ_ADDSRC_COPY : 0) |
          (data.params.ext_fields & NTP_EF_FLAG_EXP_MONO_ROOT ?
           REQ_ADDSRC_EF_EXP_MONO_ROOT : 0) |
          (data.params.ext_fields & NTP_EF_FLAG_EXP_NET_CORRECTION ?
           REQ_ADDSRC_EF_EXP_NET_CORRECTION : 0) |
          (data.family == IPADDR_INET4 ? REQ_ADDSRC_IPV4 : 0) |
          (data.family == IPADDR_INET6 ? REQ_ADDSRC_IPV6 : 0) |
          convert_addsrc_sel_options(data.params.sel_options));
      msg->data.ntp_source.filter_length = htonl(data.params.filter_length);
      msg->data.ntp_source.cert_set = htonl(data.params.cert_set);
      msg->data.ntp_source.max_delay_quant =
        UTI_FloatHostToNetwork(data.params.max_delay_quant);
      memset(msg->data.ntp_source.reserved, 0, sizeof (msg->data.ntp_source.reserved));

      result = 1;

      break;
    case CPS_InvalidOption:
      LOG(LOGS_ERR, "Invalid %s add command", "option in");
      break;
    case CPS_InvalidValue:
      LOG(LOGS_ERR, "Invalid %s add command", "value in");
      break;
    default:
      LOG(LOGS_ERR, "Invalid %s add command", "syntax for");
      break;
  }

  return result;
}

/* ================================================== */

static int
process_cmd_delete(CMD_Request *msg, char *line)
{
  char *hostname;
  int ok = 0;
  IPAddr address;

  msg->command = htons(REQ_DEL_SOURCE);
  hostname = line;
  CPS_SplitWord(line);

  if (!*hostname) {
    LOG(LOGS_ERR, "Invalid syntax for address");
    ok = 0;
  } else {
    if (!parse_source_address(hostname, &address)) {
      LOG(LOGS_ERR, "Could not get address for hostname");
      ok = 0;
    } else {
      UTI_IPHostToNetwork(&address, &msg->data.del_source.ip_addr);
      ok = 1;
    }
  }

  return ok;
  
}

/* ================================================== */

static void
give_help(void)
{
  int line, len;
  const char *s, cols[] =
    "System clock:\0\0"
    "tracking\0Display system time information\0"
    "makestep\0Correct clock by stepping immediately\0"
    "makestep <threshold> <updates>\0Configure automatic clock stepping\0"
    "maxupdateskew <skew>\0Modify maximum valid skew to update frequency\0"
    "waitsync [<max-tries> [<max-correction> [<max-skew> [<interval>]]]]\0"
                          "Wait until synchronised in specified limits\0"
    "\0\0"
    "Time sources:\0\0"
    "sources [-a] [-v]\0Display information about current sources\0"
    "sourcestats [-a] [-v]\0Display statistics about collected measurements\0"
    "selectdata [-a] [-v]\0Display information about source selection\0"
    "selectopts <address|refid> <+|-options>\0Modify selection options\0"
    "reselect\0Force reselecting synchronisation source\0"
    "reselectdist <dist>\0Modify reselection distance\0"
    "offset <address|refid> <offset>\0Modify offset correction\0"
    "\0\0"
    "NTP sources:\0\0"
    "activity\0Check how many NTP sources are online/offline\0"
    "authdata [-a] [-v]\0Display information about authentication\0"
    "ntpdata [<address>]\0Display information about last valid measurement\0"
    "add server <name> [options]\0Add new NTP server\0"
    "add pool <name> [options]\0Add new pool of NTP servers\0"
    "add peer <name> [options]\0Add new NTP peer\0"
    "delete <address>\0Remove server or peer\0"
    "burst <n-good>/<n-max> [[<mask>/]<address>]\0Start rapid set of measurements\0"
    "maxdelay <address> <delay>\0Modify maximum valid sample delay\0"
    "maxdelayratio <address> <ratio>\0Modify maximum valid delay/minimum ratio\0"
    "maxdelaydevratio <address> <ratio>\0Modify maximum valid delay/deviation ratio\0"
    "minpoll <address> <poll>\0Modify minimum polling interval\0"
    "maxpoll <address> <poll>\0Modify maximum polling interval\0"
    "minstratum <address> <stratum>\0Modify minimum stratum\0"
    "offline [[<mask>/]<address>]\0Set sources in subnet to offline status\0"
    "online [[<mask>/]<address>]\0Set sources in subnet to online status\0"
    "onoffline\0Set all sources to online or offline status\0"
    "\0according to network configuration\0"
    "polltarget <address> <target>\0Modify poll target\0"
    "refresh\0Refresh IP addresses\0"
    "reload sources\0Re-read *.sources files\0"
    "sourcename <address>\0Display original name\0"
    "\0\0"
    "Manual time input:\0\0"
    "manual off|on|reset\0Disable/enable/reset settime command\0"
    "manual list\0Show previous settime entries\0"
    "manual delete <index>\0Delete previous settime entry\0"
    "settime <time>\0Set daemon time\0"
    "\0(e.g. Sep 25, 2015 16:30:05 or 16:30:05)\0"
    "\0\0NTP access:\0\0"
    "accheck <address>\0Check whether address is allowed\0"
    "clients [-p <packets>] [-k] [-r]\0Report on clients that accessed the server\0"
    "serverstats\0Display statistics of the server\0"
    "allow [<subnet>]\0Allow access to subnet as a default\0"
    "allow all [<subnet>]\0Allow access to subnet and all children\0"
    "deny [<subnet>]\0Deny access to subnet as a default\0"
    "deny all [<subnet>]\0Deny access to subnet and all children\0"
    "local [options]\0Serve time even when not synchronised\0"
    "local off\0Don't serve time when not synchronised\0"
    "smoothtime reset|activate\0Reset/activate time smoothing\0"
    "smoothing\0Display current time smoothing state\0"
    "\0\0"
    "Monitoring access:\0\0"
    "cmdaccheck <address>\0Check whether address is allowed\0"
    "cmdallow [<subnet>]\0Allow access to subnet as a default\0"
    "cmdallow all [<subnet>]\0Allow access to subnet and all children\0"
    "cmddeny [<subnet>]\0Deny access to subnet as a default\0"
    "cmddeny all [<subnet>]\0Deny access to subnet and all children\0"
    "\0\0"
    "Real-time clock:\0\0"
    "rtcdata\0Print current RTC performance parameters\0"
    "trimrtc\0Correct RTC relative to system clock\0"
    "writertc\0Save RTC performance parameters to file\0"
    "\0\0"
    "Other daemon commands:\0\0"
    "cyclelogs\0Close and re-open log files\0"
    "dump\0Dump measurements and NTS keys/cookies\0"
    "rekey\0Re-read keys\0"
    "reset sources\0Drop all measurements\0"
    "shutdown\0Stop daemon\0"
    "\0\0"
    "Client commands:\0\0"
    "dns -n|+n\0Disable/enable resolving IP addresses to hostnames\0"
    "dns -4|-6|-46\0Resolve hostnames only to IPv4/IPv6/both addresses\0"
    "timeout <milliseconds>\0Set initial response timeout\0"
    "retries <retries>\0Set maximum number of retries\0"
    "keygen [<id> [<type> [<bits>]]]\0Generate key for key file\0"
    "exit|quit\0Leave the program\0"
    "help\0Generate this help\0"
    "\0";

  /* Indent the second column */
  for (s = cols, line = 0; s < cols + sizeof (cols); s += len + 1, line++) {
    len = strlen(s);
    printf(line % 2 == 0 ? (len >= 28 ? "%s\n%28s" : "%-28s%s") : "%s%s\n",
           s, "");
  }
}

/* ================================================== */
/* Tab-completion when editline is available */

#ifdef FEAT_READLINE

enum {
  TAB_COMPLETE_BASE_CMDS,
  TAB_COMPLETE_ADD_OPTS,
  TAB_COMPLETE_MANUAL_OPTS,
  TAB_COMPLETE_RELOAD_OPTS,
  TAB_COMPLETE_RESET_OPTS,
  TAB_COMPLETE_SOURCES_OPTS,
  TAB_COMPLETE_SOURCESTATS_OPTS,
  TAB_COMPLETE_AUTHDATA_OPTS,
  TAB_COMPLETE_SELECTDATA_OPTS,
  TAB_COMPLETE_MAX_INDEX
};

static int tab_complete_index;

static char *
command_name_generator(const char *text, int state)
{
  const char *name, **names[TAB_COMPLETE_MAX_INDEX];
  const char *base_commands[] = {
    "accheck", "activity", "add", "allow", "authdata", "burst",
    "clients", "cmdaccheck", "cmdallow", "cmddeny", "cyclelogs", "delete",
    "deny", "dns", "dump", "exit", "help", "keygen", "local", "makestep",
    "manual", "maxdelay", "maxdelaydevratio", "maxdelayratio", "maxpoll",
    "maxupdateskew", "minpoll", "minstratum", "ntpdata",
    "offline", "offset", "online", "onoffline",
    "polltarget", "quit", "refresh", "rekey", "reload", "reselect", "reselectdist", "reset",
    "retries", "rtcdata", "selectdata", "selectopts", "serverstats", "settime",
    "shutdown", "smoothing", "smoothtime", "sourcename", "sources", "sourcestats",
    "timeout", "tracking", "trimrtc", "waitsync", "writertc",
    NULL
  };
  const char *add_options[] = { "peer", "pool", "server", NULL };
  const char *manual_options[] = { "on", "off", "delete", "list", "reset", NULL };
  const char *reset_options[] = { "sources", NULL };
  const char *reload_options[] = { "sources", NULL };
  const char *common_source_options[] = { "-a", "-v", NULL };
  static int list_index, len;

  names[TAB_COMPLETE_BASE_CMDS] = base_commands;
  names[TAB_COMPLETE_ADD_OPTS] = add_options;
  names[TAB_COMPLETE_MANUAL_OPTS] = manual_options;
  names[TAB_COMPLETE_RELOAD_OPTS] = reload_options;
  names[TAB_COMPLETE_RESET_OPTS] = reset_options;
  names[TAB_COMPLETE_AUTHDATA_OPTS] = common_source_options;
  names[TAB_COMPLETE_SELECTDATA_OPTS] = common_source_options;
  names[TAB_COMPLETE_SOURCES_OPTS] = common_source_options;
  names[TAB_COMPLETE_SOURCESTATS_OPTS] = common_source_options;

  if (!state) {
    list_index = 0;
    len = strlen(text);
  }

  while ((name = names[tab_complete_index][list_index++])) {
    if (strncmp(name, text, len) == 0) {
      return Strdup(name);
    }
  }

  return NULL;
}

/* ================================================== */

static char **
command_name_completion(const char *text, int start, int end)
{
  char first[32];

  snprintf(first, MIN(sizeof (first), start + 1), "%s", rl_line_buffer);
  rl_attempted_completion_over = 1;

  if (!strcmp(first, "add ")) {
    tab_complete_index = TAB_COMPLETE_ADD_OPTS;
  } else if (!strcmp(first, "authdata ")) {
    tab_complete_index = TAB_COMPLETE_AUTHDATA_OPTS;
  } else if (!strcmp(first, "manual ")) {
    tab_complete_index = TAB_COMPLETE_MANUAL_OPTS;
  } else if (!strcmp(first, "reload ")) {
    tab_complete_index = TAB_COMPLETE_RELOAD_OPTS;
  } else if (!strcmp(first, "reset ")) {
    tab_complete_index = TAB_COMPLETE_RESET_OPTS;
  } else if (!strcmp(first, "selectdata ")) {
    tab_complete_index = TAB_COMPLETE_SELECTDATA_OPTS;
  } else if (!strcmp(first, "sources ")) {
    tab_complete_index = TAB_COMPLETE_SOURCES_OPTS;
  } else if (!strcmp(first, "sourcestats ")) {
    tab_complete_index = TAB_COMPLETE_SOURCESTATS_OPTS;
  } else if (first[0] == '\0') {
    tab_complete_index = TAB_COMPLETE_BASE_CMDS;
  } else {
    return NULL;
  }

  return rl_completion_matches(text, command_name_generator);
}
#endif

/* ================================================== */

static int max_retries = 2;
static int initial_timeout = 1000;
static int proto_version = PROTO_VERSION_NUMBER;

/* This is the core protocol module.  Complete particular fields in
   the outgoing packet, send it, wait for a response, handle retries,
   etc.  Returns a Boolean indicating whether the protocol was
   successful or not.*/

static int
submit_request(CMD_Request *request, CMD_Reply *reply)
{
  int select_status;
  int recv_status;
  int read_length;
  int command_length;
  int padding_length;
  struct timespec ts_now, ts_start;
  struct timeval tv;
  int n_attempts, new_attempt;
  double timeout;
  fd_set rdfd;

  request->pkt_type = PKT_TYPE_CMD_REQUEST;
  request->res1 = 0;
  request->res2 = 0;
  request->pad1 = 0;
  request->pad2 = 0;

  n_attempts = 0;
  new_attempt = 1;

  do {
    if (gettimeofday(&tv, NULL))
      return 0;

    if (new_attempt) {
      new_attempt = 0;

      if (n_attempts > max_retries)
        return 0;

      UTI_TimevalToTimespec(&tv, &ts_start);

      UTI_GetRandomBytes(&request->sequence, sizeof (request->sequence));
      request->attempt = htons(n_attempts);
      request->version = proto_version;
      command_length = PKL_CommandLength(request);
      padding_length = PKL_CommandPaddingLength(request);
      assert(command_length > 0 && command_length > padding_length);

      n_attempts++;

      /* Zero the padding to not send any uninitialized data */
      memset(((char *)request) + command_length - padding_length, 0, padding_length);

      if (sock_fd < 0) {
        DEBUG_LOG("No socket to send request");
        return 0;
      }

      if (SCK_Send(sock_fd, (void *)request, command_length, 0) < 0)
        return 0;
    }

    UTI_TimevalToTimespec(&tv, &ts_now);

    /* Check if the clock wasn't stepped back */
    if (UTI_CompareTimespecs(&ts_now, &ts_start) < 0)
      ts_start = ts_now;

    timeout = initial_timeout / 1000.0 * (1U << (n_attempts - 1)) -
              UTI_DiffTimespecsToDouble(&ts_now, &ts_start);
    DEBUG_LOG("Timeout %f seconds", timeout);

    /* Avoid calling select() with an invalid timeout */
    if (timeout <= 0.0) {
      new_attempt = 1;
      continue;
    }

    UTI_DoubleToTimeval(timeout, &tv);

    FD_ZERO(&rdfd);
    FD_SET(sock_fd, &rdfd);

    if (quit)
      return 0;

    select_status = select(sock_fd + 1, &rdfd, NULL, NULL, &tv);

    if (select_status < 0) {
      DEBUG_LOG("select failed : %s", strerror(errno));
      return 0;
    } else if (select_status == 0) {
      /* Timeout must have elapsed, try a resend? */
      new_attempt = 1;
    } else {
      recv_status = SCK_Receive(sock_fd, reply, sizeof (*reply), 0);
      
      if (recv_status < 0) {
        new_attempt = 1;
      } else {
        read_length = recv_status;
        
        /* Check if the header is valid */
        if (read_length < offsetof(CMD_Reply, data) ||
            (reply->version != proto_version &&
             !(reply->version >= PROTO_VERSION_MISMATCH_COMPAT_CLIENT &&
               ntohs(reply->status) == STT_BADPKTVERSION)) ||
            reply->pkt_type != PKT_TYPE_CMD_REPLY ||
            reply->res1 != 0 ||
            reply->res2 != 0 ||
            reply->command != request->command ||
            reply->sequence != request->sequence) {
          DEBUG_LOG("Invalid reply");
          continue;
        }
        
#if PROTO_VERSION_NUMBER == 6
        /* Protocol version 5 is similar to 6 except there is no padding.
           If a version 5 reply with STT_BADPKTVERSION is received,
           switch our version and try again. */
        if (proto_version == PROTO_VERSION_NUMBER &&
            reply->version == PROTO_VERSION_NUMBER - 1) {
          proto_version = PROTO_VERSION_NUMBER - 1;
          n_attempts--;
          new_attempt = 1;
          continue;
        }
#else
#error unknown compatibility with PROTO_VERSION - 1
#endif

        /* Check that the packet contains all data it is supposed to have.
           Unknown responses will always pass this test as their expected
           length is zero. */
        if (read_length < PKL_ReplyLength(reply)) {
          DEBUG_LOG("Reply too short");
          new_attempt = 1;
          continue;
        }

        /* Good packet received, print out results */
        DEBUG_LOG("Reply cmd=%d reply=%d stat=%d",
                  ntohs(reply->command), ntohs(reply->reply), ntohs(reply->status));
        break;
      }
    }
  } while (1);

  return 1;
}

/* ================================================== */

static int
request_reply(CMD_Request *request, CMD_Reply *reply, int requested_reply, int verbose)
{
  int status;

  while (!submit_request(request, reply)) {
    /* Try connecting to other addresses before giving up */
    if (open_io())
      continue;
    printf("506 Cannot talk to daemon\n");
    return 0;
  }

  status = ntohs(reply->status);
        
  if (verbose || status != STT_SUCCESS) {
    switch (status) {
      case STT_SUCCESS:
        printf("200 OK");
        break;
      case STT_ACCESSALLOWED:
        printf("208 Access allowed");
        break;
      case STT_ACCESSDENIED:
        printf("209 Access denied");
        break;
      case STT_FAILED:
        printf("500 Failure");
        break;
      case STT_UNAUTH:
        printf("501 Not authorised");
        break;
      case STT_INVALID:
        printf("502 Invalid command");
        break;
      case STT_NOSUCHSOURCE:
        printf("503 No such source");
        break;
      case STT_INVALIDTS:
        printf("504 Duplicate or stale logon detected");
        break;
      case STT_NOTENABLED:
        printf("505 Facility not enabled in daemon");
        break;
      case STT_BADSUBNET:
        printf("507 Bad subnet");
        break;
      case STT_NOHOSTACCESS:
        printf("510 No command access from this host");
        break;
      case STT_SOURCEALREADYKNOWN:
        printf("511 Source already present");
        break;
      case STT_TOOMANYSOURCES:
        printf("512 Too many sources present");
        break;
      case STT_NORTC:
        printf("513 RTC driver not running");
        break;
      case STT_BADRTCFILE:
        printf("514 Can't write RTC parameters");
        break;
      case STT_INVALIDAF:
        printf("515 Invalid address family");
        break;
      case STT_BADSAMPLE:
        printf("516 Sample index out of range");
        break;
      case STT_BADPKTVERSION:
        printf("517 Protocol version mismatch");
        break;
      case STT_BADPKTLENGTH:
        printf("518 Packet length mismatch");
        break;
      case STT_INACTIVE:
        printf("519 Client logging is not active in the daemon");
        break;
      case STT_INVALIDNAME:
        printf("521 Invalid name");
        break;
      default:
        printf("520 Got unexpected error from daemon");
    }
    printf("\n");
  }
  
  if (status != STT_SUCCESS &&
      status != STT_ACCESSALLOWED && status != STT_ACCESSDENIED) {
    return 0;
  } 

  if (ntohs(reply->reply) != requested_reply) {
    printf("508 Bad reply from daemon\n");
    return 0;
  }

  /* Make sure an unknown response was not requested */
  assert(PKL_ReplyLength(reply));

  return 1;
}

/* ================================================== */

static void
print_seconds(uint32_t s)
{
  uint32_t d;

  if (s == (uint32_t)-1) {
    printf("   -");
  } else if (s < 1200) {
    printf("%4"PRIu32, s);
  } else if (s < 36000) {
    printf("%3"PRIu32"m", s / 60);
  } else if (s < 345600) {
    printf("%3"PRIu32"h", s / 3600);
  } else {
    d = s / 86400;
    if (d > 999) {
      printf("%3"PRIu32"y", d / 365);
    } else {
      printf("%3"PRIu32"d", d);
    }
  }
}

/* ================================================== */

static void
print_nanoseconds(double s)
{
  s = fabs(s);

  if (s < 9999.5e-9) {
    printf("%4.0fns", s * 1e9);
  } else if (s < 9999.5e-6) {
    printf("%4.0fus", s * 1e6);
  } else if (s < 9999.5e-3) {
    printf("%4.0fms", s * 1e3);
  } else if (s < 999.5) {
    printf("%5.1fs", s);
  } else if (s < 99999.5) {
    printf("%5.0fs", s);
  } else if (s < 99999.5 * 60) {
    printf("%5.0fm", s / 60);
  } else if (s < 99999.5 * 3600) {
    printf("%5.0fh", s / 3600);
  } else if (s < 99999.5 * 3600 * 24) {
    printf("%5.0fd", s / (3600 * 24));
  } else {
    printf("%5.0fy", s / (3600 * 24 * 365));
  }
}

/* ================================================== */

static void
print_signed_nanoseconds(double s)
{
  double x;

  x = fabs(s);

  if (x < 9999.5e-9) {
    printf("%+5.0fns", s * 1e9);
  } else if (x < 9999.5e-6) {
    printf("%+5.0fus", s * 1e6);
  } else if (x < 9999.5e-3) {
    printf("%+5.0fms", s * 1e3);
  } else if (x < 999.5) {
    printf("%+6.1fs", s);
  } else if (x < 99999.5) {
    printf("%+6.0fs", s);
  } else if (x < 99999.5 * 60) {
    printf("%+6.0fm", s / 60);
  } else if (x < 99999.5 * 3600) {
    printf("%+6.0fh", s / 3600);
  } else if (x < 99999.5 * 3600 * 24) {
    printf("%+6.0fd", s / (3600 * 24));
  } else {
    printf("%+6.0fy", s / (3600 * 24 * 365));
  }
}

/* ================================================== */

static void
print_freq_ppm(double f)
{
  if (fabs(f) < 99999.5) {
    printf("%10.3f", f);
  } else {
    printf("%10.0f", f);
  }
}

/* ================================================== */

static void
print_signed_freq_ppm(double f)
{
  if (fabs(f) < 99999.5) {
    printf("%+10.3f", f);
  } else {
    printf("%+10.0f", f);
  }
}

/* ================================================== */

static void
print_clientlog_interval(int rate)
{
  if (rate >= 127) {
    printf(" -");
  } else {
    printf("%2d", rate);
  }
}

/* ================================================== */

static void
print_header(const char *header)
{
  int len;

  if (csv_mode)
    return;

  printf("%s\n", header);

  len = strlen(header);
  while (len--)
    printf("=");
  printf("\n");
}

/* ================================================== */

#define REPORT_END 0x1234

/* Print a report. The syntax of the format is similar to printf(), but not all
   specifiers are supported and some are different! */

static void
print_report(const char *format, ...)
{
  char buf[256];
  va_list ap;
  int i, field, sign, width, prec, spec;
  const char *string;
  unsigned int uinteger;
  uint64_t uinteger64;
  uint32_t uinteger32;
  int integer;
  struct timespec *ts;
  struct tm *tm;
  double dbl;

  va_start(ap, format);

  for (field = 0; ; field++) {
    /* Search for text between format specifiers and print it
       if not in the CSV mode */
    for (i = 0; i < sizeof (buf) && format[i] != '%' && format[i] != '\0'; i++)
      buf[i] = format[i];

    if (i >= sizeof (buf))
      break;

    buf[i] = '\0';

    if (!csv_mode)
      printf("%s", buf);

    if (format[i] == '\0' || format[i + 1] == '\0')
      break;

    format += i + 1;

    sign = 0;
    width = 0;
    prec = 5;

    if (*format == '+' || *format == '-') {
      sign = 1;
      format++;
    }

    if (isdigit((unsigned char)*format)) {
      width = atoi(format);
      while (isdigit((unsigned char)*format))
        format++;
    }

    if (*format == '.') {
      format++;
      prec = atoi(format);
      while (isdigit((unsigned char)*format))
        format++;
    }

    spec = *format;
    format++;

    /* Disable human-readable formatting in the CSV mode */
    if (csv_mode) {
      sign = width = 0;

      if (field > 0)
        printf(",");

      switch (spec) {
        case 'C':
          spec = 'd';
          break;
        case 'F':
        case 'P':
          prec = 3;
          spec = 'f';
          break;
        case 'O':
        case 'S':
          prec = 9;
          spec = 'f';
          break;
        case 'I':
          spec = 'U';
          break;
        case 'T':
          spec = 'V';
          break;
      }
    }

    switch (spec) {
      case 'B': /* boolean */
        integer = va_arg(ap, int);
        printf("%s", integer ? "Yes" : "No");
        break;
      case 'C': /* clientlog interval */
        integer = va_arg(ap, int);
        print_clientlog_interval(integer);
        break;
      case 'F': /* absolute frequency in ppm with fast/slow keyword */
      case 'O': /* absolute offset in seconds with fast/slow keyword */
        dbl = va_arg(ap, double);
        printf("%*.*f %s %s", width, prec, fabs(dbl),
               spec == 'O' ? "seconds" : "ppm",
               (dbl > 0.0) ^ (spec != 'O') ? "slow" : "fast");
        break;
      case 'I': /* uint32_t interval with unit */
        uinteger32 = va_arg(ap, uint32_t);
        print_seconds(uinteger32);
        break;
      case 'L': /* leap status */
        integer = va_arg(ap, int);
        switch (integer) {
          case LEAP_Normal:
            string = width != 1 ? "Normal" : "N";
            break;
          case LEAP_InsertSecond:
            string = width != 1 ? "Insert second" : "+";
            break;
          case LEAP_DeleteSecond:
            string = width != 1 ? "Delete second" : "-";
            break;
          case LEAP_Unsynchronised:
            string = width != 1 ? "Not synchronised" : "?";
            break;
          default:
            string = width != 1 ? "Invalid" : "?";
            break;
        }
        printf("%s", string);
        break;
      case 'M': /* NTP mode */
        integer = va_arg(ap, int);
        switch (integer) {
          case MODE_ACTIVE:
            string = "Symmetric active";
            break;
          case MODE_PASSIVE:
            string = "Symmetric passive";
            break;
          case MODE_SERVER:
            string = "Server";
            break;
          default:
            string = "Invalid";
            break;
        }
        printf("%s", string);
        break;
      case 'N': /* Timestamp source */
        integer = va_arg(ap, int);
        switch (integer) {
          case 'D':
            string = "Daemon";
            break;
          case 'K':
            string = "Kernel";
            break;
          case 'H':
            string = "Hardware";
            break;
          default:
            string = "Invalid";
            break;
        }
        printf("%s", string);
        break;
      case 'P': /* frequency in ppm */
        dbl = va_arg(ap, double);
        if (sign)
          print_signed_freq_ppm(dbl);
        else
          print_freq_ppm(dbl);
        break;
      case 'R': /* reference ID in hexdecimal */
        uinteger32 = va_arg(ap, uint32_t);
        printf("%08"PRIX32, uinteger32);
        break;
      case 'S': /* offset with unit */
        dbl = va_arg(ap, double);
        if (sign)
          print_signed_nanoseconds(dbl);
        else
          print_nanoseconds(dbl);
        break;
      case 'T': /* timespec as date and time in UTC */
        ts = va_arg(ap, struct timespec *);
        tm = gmtime(&ts->tv_sec);
        if (!tm)
          break;
        strftime(buf, sizeof (buf), "%a %b %d %T %Y", tm);
        printf("%s", buf);
        break;
      case 'U': /* uint32_t in decimal */
        uinteger32 = va_arg(ap, uint32_t);
        printf("%*"PRIu32, width, uinteger32);
        break;
      case 'V': /* timespec as seconds since epoch */
        ts = va_arg(ap, struct timespec *);
        printf("%s", UTI_TimespecToString(ts));
        break;
      case 'Q': /* uint64_t in decimal */
        uinteger64 = va_arg(ap, uint64_t);
        printf("%*"PRIu64, width, uinteger64);
        break;
      case 'b': /* unsigned int in binary */
        uinteger = va_arg(ap, unsigned int);
        for (i = prec - 1; i >= 0; i--)
          printf("%c", uinteger & 1U << i ? '1' : '0');
        break;

      /* Classic printf specifiers */
      case 'c': /* character */
        integer = va_arg(ap, int);
        printf("%c", integer);
        break;
      case 'd': /* signed int in decimal */
        integer = va_arg(ap, int);
        printf("%*d", width, integer);
        break;
      case 'f': /* double */
        dbl = va_arg(ap, double);
        printf(sign ? "%+*.*f" : "%*.*f", width, prec, dbl);
        break;
      case 'o': /* unsigned int in octal */
        uinteger = va_arg(ap, unsigned int);
        printf("%*o", width, uinteger);
        break;
      case 's': /* string */
        string = va_arg(ap, const char *);
        if (sign)
          printf("%-*s", width, string);
        else
          printf("%*s", width, string);
        break;
      case 'u': /* unsigned int in decimal */
        uinteger = va_arg(ap, unsigned int);
        printf("%*u", width, uinteger);
        break;
    }
  }

  /* Require terminating argument to catch bad type conversions */
  if (va_arg(ap, int) != REPORT_END)
    assert(0);

  va_end(ap);

  if (csv_mode)
    printf("\n");
}

/* ================================================== */

static void
print_info_field(const char *format, ...)
{
  va_list ap;

  if (csv_mode)
    return;

  va_start(ap, format);
  vprintf(format, ap);
  va_end(ap);
}

/* ================================================== */

static int
get_source_name(IPAddr *ip_addr, char *buf, int size)
{
  CMD_Request request;
  CMD_Reply reply;
  int i;

  request.command = htons(REQ_NTP_SOURCE_NAME);
  UTI_IPHostToNetwork(ip_addr, &request.data.ntp_source_name.ip_addr);
  if (!request_reply(&request, &reply, RPY_NTP_SOURCE_NAME, 0) ||
      reply.data.ntp_source_name.name[sizeof (reply.data.ntp_source_name.name) - 1] != '\0' ||
      snprintf(buf, size, "%s", (char *)reply.data.ntp_source_name.name) >= size)
    return 0;

  /* Make sure the name is printable */
  for (i = 0; i < size && buf[i] != '\0'; i++) {
    if (!isgraph((unsigned char)buf[i]))
      return 0;
  }

  return 1;
}

/* ================================================== */

static void
format_name(char *buf, int size, int trunc_dns, int ref, uint32_t ref_id,
            int source, IPAddr *ip_addr)
{
  if (ref) {
    snprintf(buf, size, "%s", UTI_RefidToString(ref_id));
  } else if (source && source_names) {
    if (!get_source_name(ip_addr, buf, size))
      snprintf(buf, size, "?");
  } else if (no_dns || csv_mode) {
    snprintf(buf, size, "%s", UTI_IPToString(ip_addr));
  } else {
    DNS_IPAddress2Name(ip_addr, buf, size);
    if (trunc_dns > 0 && strlen(buf) > trunc_dns) {
      buf[trunc_dns - 1] = '>';
      buf[trunc_dns] = '\0';
    }
  }
}

/* ================================================== */

static void
parse_sources_options(char *line, int *all, int *verbose)
{
  char *opt;

  *all = *verbose = 0;

  while (*line) {
    opt = line;
    line = CPS_SplitWord(line);
    if (!strcmp(opt, "-a"))
      *all = 1;
    else if (!strcmp(opt, "-v"))
      *verbose = !csv_mode;
  }
}

/* ================================================== */

static int
process_cmd_sourcename(char *line)
{
  IPAddr ip_addr;
  char name[256];

  if (!parse_source_address(line, &ip_addr)) {
    LOG(LOGS_ERR, "Could not read address");
    return 0;
  }

  if (!get_source_name(&ip_addr, name, sizeof (name)))
    return 0;

  print_report("%s\n", name, REPORT_END);

  return 1;
}

/* ================================================== */

static int
process_cmd_sources(char *line)
{
  CMD_Request request;
  CMD_Reply reply;
  IPAddr ip_addr;
  uint32_t i, mode, n_sources;
  char name[256], mode_ch, state_ch;
  int all, verbose, ref;

  parse_sources_options(line, &all, &verbose);
  
  request.command = htons(REQ_N_SOURCES);
  if (!request_reply(&request, &reply, RPY_N_SOURCES, 0))
    return 0;

  n_sources = ntohl(reply.data.n_sources.n_sources);

  if (verbose) {
    printf("\n");
    printf("  .-- Source mode  '^' = server, '=' = peer, '#' = local clock.\n");
    printf(" / .- Source state '*' = current best, '+' = combined, '-' = not combined,\n");
    printf("| /             'x' = may be in error, '~' = too variable, '?' = unusable.\n");
    printf("||                                                 .- xxxx [ yyyy ] +/- zzzz\n");
    printf("||      Reachability register (octal) -.           |  xxxx = adjusted offset,\n");
    printf("||      Log2(Polling interval) --.      |          |  yyyy = measured offset,\n");
    printf("||                                \\     |          |  zzzz = estimated error.\n");
    printf("||                                 |    |           \\\n");
  }

  print_header("MS Name/IP address         Stratum Poll Reach LastRx Last sample               ");

  /*           "MS NNNNNNNNNNNNNNNNNNNNNNNNNNN  SS  PP   RRR  RRRR  SSSSSSS[SSSSSSS] +/- SSSSSS" */

  for (i = 0; i < n_sources; i++) {
    request.command = htons(REQ_SOURCE_DATA);
    request.data.source_data.index = htonl(i);
    if (!request_reply(&request, &reply, RPY_SOURCE_DATA, 0))
      return 0;

    mode = ntohs(reply.data.source_data.mode);
    UTI_IPNetworkToHost(&reply.data.source_data.ip_addr, &ip_addr);
    if (!all && ip_addr.family == IPADDR_ID)
      continue;

    ref = mode == RPY_SD_MD_REF && ip_addr.family == IPADDR_INET4;
    format_name(name, sizeof (name), 25, ref, ref ? ip_addr.addr.in4 : 0, 1, &ip_addr);

    switch (mode) {
      case RPY_SD_MD_CLIENT:
        mode_ch = '^';
        break;
      case RPY_SD_MD_PEER:
        mode_ch = '=';
        break;
      case RPY_SD_MD_REF:
        mode_ch = '#';
        break;
      default:
        mode_ch = ' ';
    }

    switch (ntohs(reply.data.source_data.state)) {
      case RPY_SD_ST_SELECTED:
        state_ch = '*';
        break;
      case RPY_SD_ST_NONSELECTABLE:
        state_ch = '?';
        break;
      case RPY_SD_ST_FALSETICKER:
        state_ch = 'x';
        break;
      case RPY_SD_ST_JITTERY:
        state_ch = '~';
        break;
      case RPY_SD_ST_UNSELECTED:
        state_ch = '+';
        break;
      case RPY_SD_ST_SELECTABLE:
        state_ch = '-';
        break;
      default:
        state_ch = ' ';
    }

    switch (ntohs(reply.data.source_data.flags)) {
      default:
        break;
    }

    print_report("%c%c %-27s  %2d  %2d   %3o  %I  %+S[%+S] +/- %S\n",
                 mode_ch, state_ch, name,
                 ntohs(reply.data.source_data.stratum),
                 (int16_t)ntohs(reply.data.source_data.poll),
                 ntohs(reply.data.source_data.reachability),
                 ntohl(reply.data.source_data.since_sample),
                 UTI_FloatNetworkToHost(reply.data.source_data.latest_meas),
                 UTI_FloatNetworkToHost(reply.data.source_data.orig_latest_meas),
                 UTI_FloatNetworkToHost(reply.data.source_data.latest_meas_err),
                 REPORT_END);
  }

  return 1;
}

/* ================================================== */

static int
process_cmd_sourcestats(char *line)
{
  CMD_Request request;
  CMD_Reply reply;
  uint32_t i, n_sources;
  int all, verbose;
  char name[256];
  IPAddr ip_addr;

  parse_sources_options(line, &all, &verbose);

  request.command = htons(REQ_N_SOURCES);
  if (!request_reply(&request, &reply, RPY_N_SOURCES, 0))
    return 0;

  n_sources = ntohl(reply.data.n_sources.n_sources);

  if (verbose) {
    printf("                             .- Number of sample points in measurement set.\n");
    printf("                            /    .- Number of residual runs with same sign.\n");
    printf("                           |    /    .- Length of measurement set (time).\n");
    printf("                           |   |    /      .- Est. clock freq error (ppm).\n");
    printf("                           |   |   |      /           .- Est. error in freq.\n");
    printf("                           |   |   |     |           /         .- Est. offset.\n");
    printf("                           |   |   |     |          |          |   On the -.\n");
    printf("                           |   |   |     |          |          |   samples. \\\n");
    printf("                           |   |   |     |          |          |             |\n");
  }

  print_header("Name/IP Address            NP  NR  Span  Frequency  Freq Skew  Offset  Std Dev");

  /*           "NNNNNNNNNNNNNNNNNNNNNNNNN  NP  NR  SSSS FFFFFFFFFF SSSSSSSSSS  SSSSSSS  SSSSSS" */

  for (i = 0; i < n_sources; i++) {
    request.command = htons(REQ_SOURCESTATS);
    request.data.source_data.index = htonl(i);
    if (!request_reply(&request, &reply, RPY_SOURCESTATS, 0))
      return 0;

    UTI_IPNetworkToHost(&reply.data.sourcestats.ip_addr, &ip_addr);
    if (!all && ip_addr.family == IPADDR_ID)
      continue;

    format_name(name, sizeof (name), 25, ip_addr.family == IPADDR_UNSPEC,
                ntohl(reply.data.sourcestats.ref_id), 1, &ip_addr);

    print_report("%-25s %3U %3U  %I %+P %P  %+S  %S\n",
                 name,
                 ntohl(reply.data.sourcestats.n_samples),
                 ntohl(reply.data.sourcestats.n_runs),
                 ntohl(reply.data.sourcestats.span_seconds),
                 UTI_FloatNetworkToHost(reply.data.sourcestats.resid_freq_ppm),
                 UTI_FloatNetworkToHost(reply.data.sourcestats.skew_ppm),
                 UTI_FloatNetworkToHost(reply.data.sourcestats.est_offset),
                 UTI_FloatNetworkToHost(reply.data.sourcestats.sd),
                 REPORT_END);
  }

  return 1;
}

/* ================================================== */

static int
process_cmd_tracking(char *line)
{
  CMD_Request request;
  CMD_Reply reply;
  IPAddr ip_addr;
  uint32_t ref_id;
  char name[256];
  struct timespec ref_time;
  
  request.command = htons(REQ_TRACKING);
  if (!request_reply(&request, &reply, RPY_TRACKING, 0))
    return 0;

  ref_id = ntohl(reply.data.tracking.ref_id);

  UTI_IPNetworkToHost(&reply.data.tracking.ip_addr, &ip_addr);
  format_name(name, sizeof (name), sizeof (name),
              ip_addr.family == IPADDR_UNSPEC, ref_id, 1, &ip_addr);

  UTI_TimespecNetworkToHost(&reply.data.tracking.ref_time, &ref_time);

  print_report("Reference ID    : %R (%s)\n"
               "Stratum         : %u\n"
               "Ref time (UTC)  : %T\n"
               "System time     : %.9O of NTP time\n"
               "Last offset     : %+.9f seconds\n"
               "RMS offset      : %.9f seconds\n"
               "Frequency       : %.3F\n"
               "Residual freq   : %+.3f ppm\n"
               "Skew            : %.3f ppm\n"
               "Root delay      : %.9f seconds\n"
               "Root dispersion : %.9f seconds\n"
               "Update interval : %.1f seconds\n"
               "Leap status     : %L\n",
               ref_id, name,
               ntohs(reply.data.tracking.stratum),
               &ref_time,
               UTI_FloatNetworkToHost(reply.data.tracking.current_correction),
               UTI_FloatNetworkToHost(reply.data.tracking.last_offset),
               UTI_FloatNetworkToHost(reply.data.tracking.rms_offset),
               UTI_FloatNetworkToHost(reply.data.tracking.freq_ppm),
               UTI_FloatNetworkToHost(reply.data.tracking.resid_freq_ppm),
               UTI_FloatNetworkToHost(reply.data.tracking.skew_ppm),
               UTI_FloatNetworkToHost(reply.data.tracking.root_delay),
               UTI_FloatNetworkToHost(reply.data.tracking.root_dispersion),
               UTI_FloatNetworkToHost(reply.data.tracking.last_update_interval),
               ntohs(reply.data.tracking.leap_status), REPORT_END);

  return 1;
}

/* ================================================== */

static int
process_cmd_authdata(char *line)
{
  CMD_Request request;
  CMD_Reply reply;
  IPAddr ip_addr;
  uint32_t i, source_mode, n_sources;
  int all, verbose;
  const char *mode_str;
  char name[256];

  parse_sources_options(line, &all, &verbose);

  request.command = htons(REQ_N_SOURCES);
  if (!request_reply(&request, &reply, RPY_N_SOURCES, 0))
    return 0;

  n_sources = ntohl(reply.data.n_sources.n_sources);

  if (verbose) {
    printf(    "                             .- Auth. mechanism (NTS, SK - symmetric key)\n");
    printf(    "                            |   Key length -.  Cookie length (bytes) -.\n");
    printf(    "                            |       (bits)  |  Num. of cookies --.    |\n");
    printf(    "                            |               |  Key est. attempts  |   |\n");
    printf(    "                            |               |           |         |   |\n");
  }

  print_header("Name/IP address             Mode KeyID Type KLen Last Atmp  NAK Cook CLen");

  /*           "NNNNNNNNNNNNNNNNNNNNNNNNNNN MMMM KKKKK AAAA LLLL LLLL AAAA NNNN CCCC LLLL" */

  for (i = 0; i < n_sources; i++) {
    request.command = htons(REQ_SOURCE_DATA);
    request.data.source_data.index = htonl(i);
    if (!request_reply(&request, &reply, RPY_SOURCE_DATA, 0))
      return 0;

    source_mode = ntohs(reply.data.source_data.mode);
    if (source_mode != RPY_SD_MD_CLIENT && source_mode != RPY_SD_MD_PEER)
      continue;

    UTI_IPNetworkToHost(&reply.data.source_data.ip_addr, &ip_addr);
    if (!all && ip_addr.family == IPADDR_ID)
      continue;

    request.command = htons(REQ_AUTH_DATA);
    request.data.auth_data.ip_addr = reply.data.source_data.ip_addr;
    if (!request_reply(&request, &reply, RPY_AUTH_DATA, 0))
      return 0;

    format_name(name, sizeof (name), 25, 0, 0, 1, &ip_addr);

    switch (ntohs(reply.data.auth_data.mode)) {
      case RPY_AD_MD_NONE:
        mode_str = "-";
        break;
      case RPY_AD_MD_SYMMETRIC:
        mode_str = "SK";
        break;
      case RPY_AD_MD_NTS:
        mode_str = "NTS";
        break;
      default:
        mode_str = "?";
        break;
    }

    print_report("%-27s %4s %5U %4d %4d %I %4d %4d %4d %4d\n",
                 name, mode_str,
                 ntohl(reply.data.auth_data.key_id),
                 ntohs(reply.data.auth_data.key_type),
                 ntohs(reply.data.auth_data.key_length),
                 ntohl(reply.data.auth_data.last_ke_ago),
                 ntohs(reply.data.auth_data.ke_attempts),
                 ntohs(reply.data.auth_data.nak),
                 ntohs(reply.data.auth_data.cookies),
                 ntohs(reply.data.auth_data.cookie_length),
                 REPORT_END);
  }

  return 1;
}

/* ================================================== */

static int
process_cmd_ntpdata(char *line)
{
  CMD_Request request;
  CMD_Reply reply;
  IPAddr remote_addr, local_addr;
  struct timespec ref_time;
  uint32_t i, n_sources;
  uint16_t mode;
  int specified_addr;

  if (*line) {
    specified_addr = 1;
    n_sources = 1;
  } else {
    specified_addr = 0;
    request.command = htons(REQ_N_SOURCES);
    if (!request_reply(&request, &reply, RPY_N_SOURCES, 0))
      return 0;
    n_sources = ntohl(reply.data.n_sources.n_sources);
  }

  for (i = 0; i < n_sources; i++) {
    if (specified_addr) {
      if (!parse_source_address(line, &remote_addr)) {
        LOG(LOGS_ERR, "Could not get address for hostname");
        return 0;
      }
    } else {
      request.command = htons(REQ_SOURCE_DATA);
      request.data.source_data.index = htonl(i);
      if (!request_reply(&request, &reply, RPY_SOURCE_DATA, 0))
        return 0;

      mode = ntohs(reply.data.source_data.mode);
      if (mode != RPY_SD_MD_CLIENT && mode != RPY_SD_MD_PEER)
        continue;

      UTI_IPNetworkToHost(&reply.data.source_data.ip_addr, &remote_addr);
      if (!UTI_IsIPReal(&remote_addr))
        continue;
    }

    request.command = htons(REQ_NTP_DATA);
    UTI_IPHostToNetwork(&remote_addr, &request.data.ntp_data.ip_addr);
    if (!request_reply(&request, &reply, RPY_NTP_DATA2, 0))
      return 0;

    UTI_IPNetworkToHost(&reply.data.ntp_data.remote_addr, &remote_addr);
    UTI_IPNetworkToHost(&reply.data.ntp_data.local_addr, &local_addr);
    UTI_TimespecNetworkToHost(&reply.data.ntp_data.ref_time, &ref_time);

    if (!specified_addr && !csv_mode)
      printf("\n");

    print_report("Remote address  : %s (%R)\n"
                 "Remote port     : %u\n"
                 "Local address   : %s (%R)\n"
                 "Leap status     : %L\n"
                 "Version         : %u\n"
                 "Mode            : %M\n"
                 "Stratum         : %u\n"
                 "Poll interval   : %d (%.0f seconds)\n"
                 "Precision       : %d (%.9f seconds)\n"
                 "Root delay      : %.6f seconds\n"
                 "Root dispersion : %.6f seconds\n"
                 "Reference ID    : %R (%s)\n"
                 "Reference time  : %T\n"
                 "Offset          : %+.9f seconds\n"
                 "Peer delay      : %.9f seconds\n"
                 "Peer dispersion : %.9f seconds\n"
                 "Response time   : %.9f seconds\n"
                 "Jitter asymmetry: %+.2f\n"
                 "NTP tests       : %.3b %.3b %.4b\n"
                 "Interleaved     : %B\n"
                 "Authenticated   : %B\n"
                 "TX timestamping : %N\n"
                 "RX timestamping : %N\n"
                 "Total TX        : %U\n"
                 "Total RX        : %U\n"
                 "Total valid RX  : %U\n"
                 "Total good RX   : %U\n"
                 "Total kernel TX : %U\n"
                 "Total kernel RX : %U\n"
                 "Total HW TX     : %U\n"
                 "Total HW RX     : %U\n",
                 UTI_IPToString(&remote_addr), UTI_IPToRefid(&remote_addr),
                 ntohs(reply.data.ntp_data.remote_port),
                 UTI_IPToString(&local_addr), UTI_IPToRefid(&local_addr),
                 reply.data.ntp_data.leap, reply.data.ntp_data.version,
                 reply.data.ntp_data.mode, reply.data.ntp_data.stratum,
                 reply.data.ntp_data.poll, UTI_Log2ToDouble(reply.data.ntp_data.poll),
                 reply.data.ntp_data.precision, UTI_Log2ToDouble(reply.data.ntp_data.precision),
                 UTI_FloatNetworkToHost(reply.data.ntp_data.root_delay),
                 UTI_FloatNetworkToHost(reply.data.ntp_data.root_dispersion),
                 ntohl(reply.data.ntp_data.ref_id), reply.data.ntp_data.stratum <= 1 ?
                   UTI_RefidToString(ntohl(reply.data.ntp_data.ref_id)) : "",
                 &ref_time,
                 UTI_FloatNetworkToHost(reply.data.ntp_data.offset),
                 UTI_FloatNetworkToHost(reply.data.ntp_data.peer_delay),
                 UTI_FloatNetworkToHost(reply.data.ntp_data.peer_dispersion),
                 UTI_FloatNetworkToHost(reply.data.ntp_data.response_time),
                 UTI_FloatNetworkToHost(reply.data.ntp_data.jitter_asymmetry),
                 ntohs(reply.data.ntp_data.flags) >> 7,
                 ntohs(reply.data.ntp_data.flags) >> 4,
                 ntohs(reply.data.ntp_data.flags),
                 ntohs(reply.data.ntp_data.flags) & RPY_NTP_FLAG_INTERLEAVED,
                 ntohs(reply.data.ntp_data.flags) & RPY_NTP_FLAG_AUTHENTICATED,
                 reply.data.ntp_data.tx_tss_char, reply.data.ntp_data.rx_tss_char,
                 ntohl(reply.data.ntp_data.total_tx_count),
                 ntohl(reply.data.ntp_data.total_rx_count),
                 ntohl(reply.data.ntp_data.total_valid_count),
                 ntohl(reply.data.ntp_data.total_good_count),
                 ntohl(reply.data.ntp_data.total_kernel_tx_ts),
                 ntohl(reply.data.ntp_data.total_kernel_rx_ts),
                 ntohl(reply.data.ntp_data.total_hw_tx_ts),
                 ntohl(reply.data.ntp_data.total_hw_rx_ts),
                 REPORT_END);
  }

  return 1;
}

/* ================================================== */

static int
process_cmd_selectdata(char *line)
{
  CMD_Request request;
  CMD_Reply reply;
  uint32_t i, n_sources;
  int all, verbose, conf_options, eff_options;
  char name[256];
  IPAddr ip_addr;

  parse_sources_options(line, &all, &verbose);

  request.command = htons(REQ_N_SOURCES);
  if (!request_reply(&request, &reply, RPY_N_SOURCES, 0))
    return 0;

  n_sources = ntohl(reply.data.n_sources.n_sources);

  if (verbose) {
    printf(    "  . State: N - noselect, s - unsynchronised, M - missing samples,\n");
    printf(    " /         d/D - large distance, ~ - jittery, w/W - waits for others,\n");
    printf(    "|          S - stale, O - orphan, T - not trusted, P - not preferred,\n");
    printf(    "|          U - waits for update,, x - falseticker, + - combined, * - best.\n");
    printf(    "|   Effective options   ---------.  (N - noselect, P - prefer\n");
    printf(    "|   Configured options  ----.     \\  T - trust, R - require)\n");
    printf(    "|   Auth. enabled (Y/N) -.   \\     \\     Offset interval --.\n");
    printf(    "|                        |    |     |                       |\n");
  }

  print_header("S Name/IP Address        Auth COpts EOpts Last Score     Interval  Leap");

  /*           "S NNNNNNNNNNNNNNNNNNNNNNNNN A OOOO- OOOO- LLLL SSSSS IIIIIII IIIIIII  L" */

  for (i = 0; i < n_sources; i++) {
    request.command = htons(REQ_SELECT_DATA);
    request.data.source_data.index = htonl(i);
    if (!request_reply(&request, &reply, RPY_SELECT_DATA, 0))
      return 0;

    UTI_IPNetworkToHost(&reply.data.select_data.ip_addr, &ip_addr);
    if (!all && ip_addr.family == IPADDR_ID)
      continue;

    format_name(name, sizeof (name), 25, ip_addr.family == IPADDR_UNSPEC,
                ntohl(reply.data.select_data.ref_id), 1, &ip_addr);

    conf_options = ntohs(reply.data.select_data.conf_options);
    eff_options = ntohs(reply.data.select_data.eff_options);

    print_report("%c %-25s %c %c%c%c%c%c %c%c%c%c%c %I %5.1f %+S %+S  %1L\n",
                 reply.data.select_data.state_char,
                 name,
                 reply.data.select_data.authentication ? 'Y' : 'N',
                 conf_options & RPY_SD_OPTION_NOSELECT ? 'N' : '-',
                 conf_options & RPY_SD_OPTION_PREFER ? 'P' : '-',
                 conf_options & RPY_SD_OPTION_TRUST ? 'T' : '-',
                 conf_options & RPY_SD_OPTION_REQUIRE ? 'R' : '-',
                 '-',
                 eff_options & RPY_SD_OPTION_NOSELECT ? 'N' : '-',
                 eff_options & RPY_SD_OPTION_PREFER ? 'P' : '-',
                 eff_options & RPY_SD_OPTION_TRUST ? 'T' : '-',
                 eff_options & RPY_SD_OPTION_REQUIRE ? 'R' : '-',
                 '-',
                 ntohl(reply.data.select_data.last_sample_ago),
                 UTI_FloatNetworkToHost(reply.data.select_data.score),
                 UTI_FloatNetworkToHost(reply.data.select_data.lo_limit),
                 UTI_FloatNetworkToHost(reply.data.select_data.hi_limit),
                 reply.data.select_data.leap,
                 REPORT_END);
  }

  return 1;
}

/* ================================================== */

static int
process_cmd_serverstats(char *line)
{
  CMD_Request request;
  CMD_Reply reply;

  request.command = htons(REQ_SERVER_STATS);
  if (!request_reply(&request, &reply, RPY_SERVER_STATS4, 0))
    return 0;

  print_report("NTP packets received       : %Q\n"
               "NTP packets dropped        : %Q\n"
               "Command packets received   : %Q\n"
               "Command packets dropped    : %Q\n"
               "Client log records dropped : %Q\n"
               "NTS-KE connections accepted: %Q\n"
               "NTS-KE connections dropped : %Q\n"
               "Authenticated NTP packets  : %Q\n"
               "Interleaved NTP packets    : %Q\n"
               "NTP timestamps held        : %Q\n"
               "NTP timestamp span         : %Q\n"
               "NTP daemon RX timestamps   : %Q\n"
               "NTP daemon TX timestamps   : %Q\n"
               "NTP kernel RX timestamps   : %Q\n"
               "NTP kernel TX timestamps   : %Q\n"
               "NTP hardware RX timestamps : %Q\n"
               "NTP hardware TX timestamps : %Q\n",
               UTI_Integer64NetworkToHost(reply.data.server_stats.ntp_hits),
               UTI_Integer64NetworkToHost(reply.data.server_stats.ntp_drops),
               UTI_Integer64NetworkToHost(reply.data.server_stats.cmd_hits),
               UTI_Integer64NetworkToHost(reply.data.server_stats.cmd_drops),
               UTI_Integer64NetworkToHost(reply.data.server_stats.log_drops),
               UTI_Integer64NetworkToHost(reply.data.server_stats.nke_hits),
               UTI_Integer64NetworkToHost(reply.data.server_stats.nke_drops),
               UTI_Integer64NetworkToHost(reply.data.server_stats.ntp_auth_hits),
               UTI_Integer64NetworkToHost(reply.data.server_stats.ntp_interleaved_hits),
               UTI_Integer64NetworkToHost(reply.data.server_stats.ntp_timestamps),
               UTI_Integer64NetworkToHost(reply.data.server_stats.ntp_span_seconds),
               UTI_Integer64NetworkToHost(reply.data.server_stats.ntp_daemon_rx_timestamps),
               UTI_Integer64NetworkToHost(reply.data.server_stats.ntp_daemon_tx_timestamps),
               UTI_Integer64NetworkToHost(reply.data.server_stats.ntp_kernel_rx_timestamps),
               UTI_Integer64NetworkToHost(reply.data.server_stats.ntp_kernel_tx_timestamps),
               UTI_Integer64NetworkToHost(reply.data.server_stats.ntp_hw_rx_timestamps),
               UTI_Integer64NetworkToHost(reply.data.server_stats.ntp_hw_tx_timestamps),
               REPORT_END);

  return 1;
}

/* ================================================== */

static int
process_cmd_smoothing(char *line)
{
  CMD_Request request;
  CMD_Reply reply;
  uint32_t flags;

  request.command = htons(REQ_SMOOTHING);
  if (!request_reply(&request, &reply, RPY_SMOOTHING, 0))
    return 0;

  flags = ntohl(reply.data.smoothing.flags);

  print_report("Active         : %B %s\n"
               "Offset         : %+.9f seconds\n"
               "Frequency      : %+.6f ppm\n"
               "Wander         : %+.6f ppm per second\n"
               "Last update    : %.1f seconds ago\n"
               "Remaining time : %.1f seconds\n",
               !!(flags & RPY_SMT_FLAG_ACTIVE),
               flags & RPY_SMT_FLAG_LEAPONLY ? "(leap second only)" : "",
               UTI_FloatNetworkToHost(reply.data.smoothing.offset),
               UTI_FloatNetworkToHost(reply.data.smoothing.freq_ppm),
               UTI_FloatNetworkToHost(reply.data.smoothing.wander_ppm),
               UTI_FloatNetworkToHost(reply.data.smoothing.last_update_ago),
               UTI_FloatNetworkToHost(reply.data.smoothing.remaining_time),
               REPORT_END);

  return 1;
}

/* ================================================== */

static int
process_cmd_smoothtime(CMD_Request *msg, const char *line)
{
  if (!strcmp(line, "reset")) {
    msg->data.smoothtime.option = htonl(REQ_SMOOTHTIME_RESET);
  } else if (!strcmp(line, "activate")) {
    msg->data.smoothtime.option = htonl(REQ_SMOOTHTIME_ACTIVATE);
  } else {
    LOG(LOGS_ERR, "Invalid syntax for smoothtime command");
    return 0;
  }

  msg->command = htons(REQ_SMOOTHTIME);

  return 1;
}

/* ================================================== */

static int
process_cmd_rtcreport(char *line)
{
  CMD_Request request;
  CMD_Reply reply;
  struct timespec ref_time;
  
  request.command = htons(REQ_RTCREPORT);
  if (!request_reply(&request, &reply, RPY_RTC, 0))
    return 0;

  UTI_TimespecNetworkToHost(&reply.data.rtc.ref_time, &ref_time);

  print_report("RTC ref time (UTC) : %T\n"
               "Number of samples  : %u\n"
               "Number of runs     : %u\n"
               "Sample span period : %I\n"
               "RTC is fast by     : %12.6f seconds\n"
               "RTC gains time at  : %9.3f ppm\n",
               &ref_time,
               ntohs(reply.data.rtc.n_samples),
               ntohs(reply.data.rtc.n_runs),
               ntohl(reply.data.rtc.span_seconds),
               UTI_FloatNetworkToHost(reply.data.rtc.rtc_seconds_fast),
               UTI_FloatNetworkToHost(reply.data.rtc.rtc_gain_rate_ppm),
               REPORT_END);

  return 1;
}

/* ================================================== */

static int
process_cmd_clients(char *line)
{
  CMD_Request request;
  CMD_Reply reply;
  IPAddr ip;
  uint32_t i, n_clients, next_index, n_indices, min_hits, reset;
  RPY_ClientAccesses_Client *client;
  char header[80], name[50], *opt, *arg;
  int nke;

  next_index = 0;
  min_hits = 0;
  reset = 0;
  nke = 0;

  while (*line) {
    opt = line;
    line = CPS_SplitWord(line);
    if (strcmp(opt, "-k") == 0) {
      nke = 1;
    } else if (strcmp(opt, "-p") == 0) {
      arg = line;
      line = CPS_SplitWord(line);
      if (sscanf(arg, "%"SCNu32, &min_hits) != 1) {
        LOG(LOGS_ERR, "Invalid syntax for clients command");
        return 0;
      }
    } else if (strcmp(opt, "-r") == 0) {
      reset = 1;
    }
  }

  snprintf(header, sizeof (header),
           "Hostname                      NTP   Drop Int IntL Last  %6s   Drop Int  Last",
           nke ? "NTS-KE" : "Cmd");
  print_header(header);

  while (1) {
    request.command = htons(REQ_CLIENT_ACCESSES_BY_INDEX3);
    request.data.client_accesses_by_index.first_index = htonl(next_index);
    request.data.client_accesses_by_index.n_clients = htonl(MAX_CLIENT_ACCESSES);
    request.data.client_accesses_by_index.min_hits = htonl(min_hits);
    request.data.client_accesses_by_index.reset = htonl(reset);

    if (!request_reply(&request, &reply, RPY_CLIENT_ACCESSES_BY_INDEX3, 0))
      return 0;

    n_clients = ntohl(reply.data.client_accesses_by_index.n_clients);
    n_indices = ntohl(reply.data.client_accesses_by_index.n_indices);

    for (i = 0; i < n_clients && i < MAX_CLIENT_ACCESSES; i++) {
      client = &reply.data.client_accesses_by_index.clients[i];

      UTI_IPNetworkToHost(&client->ip, &ip);

      /* UNSPEC means the record could not be found in the daemon's tables.
         We shouldn't ever generate this case, but ignore it if we do. */
      if (ip.family == IPADDR_UNSPEC)
        continue;

      format_name(name, sizeof (name), 25, 0, 0, 0, &ip);

      print_report("%-25s  %6U  %5U  %C  %C  %I  %6U  %5U  %C  %I\n",
                   name,
                   ntohl(client->ntp_hits),
                   ntohl(client->ntp_drops),
                   client->ntp_interval,
                   client->ntp_timeout_interval,
                   ntohl(client->last_ntp_hit_ago),
                   ntohl(nke ? client->nke_hits : client->cmd_hits),
                   ntohl(nke ? client->nke_drops : client->cmd_drops),
                   nke ? client->nke_interval : client->cmd_interval,
                   ntohl(nke ? client->last_nke_hit_ago : client->last_cmd_hit_ago),
                   REPORT_END);
    }

    /* Set the next index to probe based on what the server tells us */
    next_index = ntohl(reply.data.client_accesses_by_index.next_index);

    if (next_index >= n_indices || n_clients < MAX_CLIENT_ACCESSES)
      break;
  }

  return 1;
}


/* ================================================== */
/* Process the manual list command */
static int
process_cmd_manual_list(const char *line)
{
  CMD_Request request;
  CMD_Reply reply;
  uint32_t i, n_samples;
  RPY_ManualListSample *sample;
  struct timespec when;

  request.command = htons(REQ_MANUAL_LIST);
  if (!request_reply(&request, &reply, RPY_MANUAL_LIST2, 0))
    return 0;

  n_samples = ntohl(reply.data.manual_list.n_samples);
  print_info_field("210 n_samples = %"PRIu32"\n", n_samples);

  print_header("#    Date     Time(UTC)    Slewed   Original   Residual");

  for (i = 0; i < n_samples && i < MAX_MANUAL_LIST_SAMPLES; i++) {
    sample = &reply.data.manual_list.samples[i];
    UTI_TimespecNetworkToHost(&sample->when, &when);

    print_report("%2d %s %10.2f %10.2f %10.2f\n",
                 i, UTI_TimeToLogForm(when.tv_sec),
                 UTI_FloatNetworkToHost(sample->slewed_offset),
                 UTI_FloatNetworkToHost(sample->orig_offset),
                 UTI_FloatNetworkToHost(sample->residual),
                 REPORT_END);
  }

  return 1;
}

/* ================================================== */

static int
process_cmd_manual_delete(CMD_Request *msg, const char *line)
{
  int index;

  if (sscanf(line, "%d", &index) != 1) {
    LOG(LOGS_ERR, "Bad syntax for manual delete command");
    return 0;
  }

  msg->command = htons(REQ_MANUAL_DELETE);
  msg->data.manual_delete.index = htonl(index);
  return 1;
}

/* ================================================== */

static int
process_cmd_settime(char *line)
{
  struct timespec ts;
  time_t now, new_time;
  CMD_Request request;
  CMD_Reply reply;
  double dfreq_ppm, new_afreq_ppm;
  double offset;

  now = time(NULL);
  new_time = get_date(line, &now);

  if (new_time == -1) {
    printf("510 - Could not parse date string\n");
  } else {
    ts.tv_sec = new_time;
    ts.tv_nsec = 0;
    UTI_TimespecHostToNetwork(&ts, &request.data.settime.ts);
    request.command = htons(REQ_SETTIME);
    if (request_reply(&request, &reply, RPY_MANUAL_TIMESTAMP2, 1)) {
          offset = UTI_FloatNetworkToHost(reply.data.manual_timestamp.offset);
          dfreq_ppm = UTI_FloatNetworkToHost(reply.data.manual_timestamp.dfreq_ppm);
          new_afreq_ppm = UTI_FloatNetworkToHost(reply.data.manual_timestamp.new_afreq_ppm);
          printf("Clock was %.2f seconds fast.  Frequency change = %.2fppm, new frequency = %.2fppm\n",
              offset, dfreq_ppm, new_afreq_ppm);
          return 1;
    }
  }
  return 0;
}

/* ================================================== */

static void
process_cmd_rekey(CMD_Request *msg, char *line)
{
  msg->command = htons(REQ_REKEY);
}

/* ================================================== */

static int
process_cmd_makestep(CMD_Request *msg, char *line)
{
  int limit;
  double threshold;

  if (*line) {
    if (sscanf(line, "%lf %d", &threshold, &limit) != 2) {
      LOG(LOGS_ERR, "Bad syntax for makestep command");
      return 0;
    }
    msg->command = htons(REQ_MODIFY_MAKESTEP);
    msg->data.modify_makestep.limit = htonl(limit);
    msg->data.modify_makestep.threshold = UTI_FloatHostToNetwork(threshold);
  } else {
    msg->command = htons(REQ_MAKESTEP);
  }

  return 1;
}

/* ================================================== */

static int
process_cmd_activity(const char *line)
{
  CMD_Request request;
  CMD_Reply reply;

  request.command = htons(REQ_ACTIVITY);
  if (!request_reply(&request, &reply, RPY_ACTIVITY, 0))
    return 0;

  print_info_field("200 OK\n");

  print_report("%U sources online\n"
               "%U sources offline\n"
               "%U sources doing burst (return to online)\n"
               "%U sources doing burst (return to offline)\n"
               "%U sources with unknown address\n",
               ntohl(reply.data.activity.online),
               ntohl(reply.data.activity.offline),
               ntohl(reply.data.activity.burst_online),
               ntohl(reply.data.activity.burst_offline),
               ntohl(reply.data.activity.unresolved),
               REPORT_END);

  return 1;
}

/* ================================================== */

static int
process_cmd_offset(CMD_Request *msg, char *line)
{
  uint32_t ref_id;
  IPAddr ip_addr;
  double offset;
  char *src;

  src = line;
  line = CPS_SplitWord(line);

  if (!parse_source_address_or_refid(src, &ip_addr, &ref_id) ||
      sscanf(line, "%lf", &offset) != 1) {
    LOG(LOGS_ERR, "Invalid syntax for offset command");
    return 0;
  }

  UTI_IPHostToNetwork(&ip_addr, &msg->data.modify_offset.address);
  msg->data.modify_offset.ref_id = htonl(ref_id);
  msg->data.modify_offset.new_offset = UTI_FloatHostToNetwork(offset);

  msg->command = htons(REQ_MODIFY_OFFSET);

  return 1;
}

/* ================================================== */

static int
process_cmd_reselectdist(CMD_Request *msg, char *line)
{
  double dist;
  int ok;
  msg->command = htons(REQ_RESELECTDISTANCE);
  if (sscanf(line, "%lf", &dist) == 1) {
    msg->data.reselect_distance.distance = UTI_FloatHostToNetwork(dist);
    ok = 1;
  } else {
    ok = 0;
  }
  return ok;
}

/* ================================================== */

static void
process_cmd_reselect(CMD_Request *msg, char *line)
{
  msg->command = htons(REQ_RESELECT);
}

/* ================================================== */

static void
process_cmd_refresh(CMD_Request *msg, char *line)
{
  msg->command = htons(REQ_REFRESH);
}

/* ================================================== */

static void
process_cmd_shutdown(CMD_Request *msg, char *line)
{
  msg->command = htons(REQ_SHUTDOWN);
}

/* ================================================== */

static int
process_cmd_reload(CMD_Request *msg, char *line)
{
  if (!strcmp(line, "sources")) {
    msg->command = htons(REQ_RELOAD_SOURCES);
  } else {
    LOG(LOGS_ERR, "Invalid syntax for reload command");
    return 0;
  }

  return 1;
}

/* ================================================== */

static int
process_cmd_reset(CMD_Request *msg, char *line)
{
  if (!strcmp(line, "sources")) {
    msg->command = htons(REQ_RESET_SOURCES);
  } else {
    LOG(LOGS_ERR, "Invalid syntax for reset command");
    return 0;
  }

  return 1;
}

/* ================================================== */

static int
process_cmd_selectopts(CMD_Request *msg, char *line)
{
  int mask, options, option;
  uint32_t ref_id;
  IPAddr ip_addr;
  char *src, *opt;

  src = line;
  line = CPS_SplitWord(line);

  if (!parse_source_address_or_refid(src, &ip_addr, &ref_id)) {
    LOG(LOGS_ERR, "Invalid syntax for selectopts command");
    return 0;
  }

  mask = options = 0;

  while (*line != '\0') {
    opt = line;
    line = CPS_SplitWord(line);

    if ((opt[0] != '+' && opt[0] != '-') || (option = CPS_GetSelectOption(opt + 1)) == 0) {
      LOG(LOGS_ERR, "Invalid syntax for selectopts command");
      return 0;
    }

    mask |= option;
    if (opt[0] == '+')
      options |= option;
  }

  UTI_IPHostToNetwork(&ip_addr, &msg->data.modify_select_opts.address);
  msg->data.modify_select_opts.ref_id = htonl(ref_id);
  msg->data.modify_select_opts.mask = htonl(mask);
  msg->data.modify_select_opts.options = htonl(convert_addsrc_sel_options(options));

  msg->command = htons(REQ_MODIFY_SELECTOPTS);

  return 1;
}

/* ================================================== */

static int
process_cmd_waitsync(char *line)
{
  CMD_Request request;
  CMD_Reply reply;
  IPAddr ip_addr;
  uint32_t ref_id;
  double correction, skew_ppm, max_correction, max_skew_ppm, interval;
  int ret = 0, max_tries, i;
  struct timeval timeout;

  max_tries = 0;
  max_correction = 0.0;
  max_skew_ppm = 0.0;
  interval = 10.0;

  if (sscanf(line, "%d %lf %lf %lf", &max_tries, &max_correction, &max_skew_ppm, &interval))
    ;

  /* Don't allow shorter interval than 0.1 seconds */
  if (interval < 0.1)
    interval = 0.1;

  request.command = htons(REQ_TRACKING);

  for (i = 1; ; i++) {
    if (request_reply(&request, &reply, RPY_TRACKING, 0)) {
      ref_id = ntohl(reply.data.tracking.ref_id);
      UTI_IPNetworkToHost(&reply.data.tracking.ip_addr, &ip_addr);

      correction = UTI_FloatNetworkToHost(reply.data.tracking.current_correction);
      correction = fabs(correction);
      skew_ppm = UTI_FloatNetworkToHost(reply.data.tracking.skew_ppm);

      print_report("try: %d, refid: %R, correction: %.9f, skew: %.3f\n",
                   i, ref_id, correction, skew_ppm, REPORT_END);

      if ((ip_addr.family != IPADDR_UNSPEC ||
           (ref_id != 0 && ref_id != 0x7f7f0101L /* LOCAL refid */)) &&
          (max_correction == 0.0 || correction <= max_correction) &&
          (max_skew_ppm == 0.0 || skew_ppm <= max_skew_ppm)) {
        ret = 1;
      }
    }

    if (!ret && (!max_tries || i < max_tries) && !quit) {
      UTI_DoubleToTimeval(interval, &timeout);
      if (select(0, NULL, NULL, NULL, &timeout))
        break;
    } else {
      break;
    }
  }
  return ret;
}

/* ================================================== */

static int
process_cmd_dns(const char *line)
{
  if (!strcmp(line, "-46")) {
    DNS_SetAddressFamily(IPADDR_UNSPEC);
  } else if (!strcmp(line, "-4")) {
    DNS_SetAddressFamily(IPADDR_INET4);
  } else if (!strcmp(line, "-6")) {
    DNS_SetAddressFamily(IPADDR_INET6);
  } else if (!strcmp(line, "-n")) {
    no_dns = 1;
  } else if (!strcmp(line, "+n")) {
    no_dns = 0;
  } else {
    LOG(LOGS_ERR, "Unrecognized dns command");
    return 0;
  }
  return 1;
}

/* ================================================== */

static int
process_cmd_timeout(const char *line)
{
  int timeout;

  timeout = atoi(line);
  if (timeout < 100) {
    LOG(LOGS_ERR, "Timeout %d is too short", timeout);
    return 0;
  }
  initial_timeout = timeout;
  return 1;
}

/* ================================================== */

static int
process_cmd_retries(const char *line)
{
  int retries;

  retries = atoi(line);
  if (retries < 0 || retries > 30) {
    LOG(LOGS_ERR, "Invalid maximum number of retries");
    return 0;
  }
  max_retries = retries;
  return 1;
}

/* ================================================== */

static int
process_cmd_keygen(char *line)
{
  unsigned int i, args, cmac_length, length, id = 1, bits = 160;
  unsigned char key[512];
  const char *type;
  char *words[3];

#ifdef FEAT_SECHASH
  type = "SHA1";
#else
  type = "MD5";
#endif

  args = UTI_SplitString(line, words, 3);
  if (args >= 2)
    type = words[1];

  if (args > 3 ||
      (args >= 1 && sscanf(words[0], "%u", &id) != 1) ||
      (args >= 3 && sscanf(words[2], "%u", &bits) != 1)) {
    LOG(LOGS_ERR, "Invalid syntax for keygen command");
    return 0;
  }

#ifdef HAVE_CMAC
  cmac_length = CMC_GetKeyLength(UTI_CmacNameToAlgorithm(type));
#else
  cmac_length = 0;
#endif

  if (HSH_GetHashId(UTI_HashNameToAlgorithm(type)) >= 0) {
    length = (bits + 7) / 8;
  } else if (cmac_length > 0) {
    length = cmac_length;
  } else {
    LOG(LOGS_ERR, "Unknown hash function or cipher %s", type);
    return 0;
  }

  length = CLAMP(10, length, sizeof (key));

  UTI_GetRandomBytesUrandom(key, length);

  printf("%u %s HEX:", id, type);
  for (i = 0; i < length; i++)
    printf("%02hhX", key[i]);
  printf("\n");

  return 1;
}

/* ================================================== */

static int
process_line(char *line)
{
  char *command;
  int do_normal_submit;
  int ret;
  CMD_Request tx_message;
  CMD_Reply rx_message;

  ret = 0;

  do_normal_submit = 1;

  CPS_NormalizeLine(line);

  if (!*line) {
    fflush(stderr);
    fflush(stdout);
    return 1;
  };

  command = line;
  line = CPS_SplitWord(line);

  if (!strcmp(command, "accheck")) {
    do_normal_submit = process_cmd_accheck(&tx_message, line);
  } else if (!strcmp(command, "activity")) {
    do_normal_submit = 0;
    ret = process_cmd_activity(line);
  } else if (!strcmp(command, "add")) {
    do_normal_submit = process_cmd_add_source(&tx_message, line);
  } else if (!strcmp(command, "allow")) {
    do_normal_submit = process_cmd_allowdeny(&tx_message, line, REQ_ALLOW, REQ_ALLOWALL);
  } else if (!strcmp(command, "authdata")) {
    do_normal_submit = 0;
    ret = process_cmd_authdata(line);
  } else if (!strcmp(command, "burst")) {
    do_normal_submit = process_cmd_burst(&tx_message, line);
  } else if (!strcmp(command, "clients")) {
    ret = process_cmd_clients(line);
    do_normal_submit = 0;
  } else if (!strcmp(command, "cmdaccheck")) {
    do_normal_submit = process_cmd_cmdaccheck(&tx_message, line);
  } else if (!strcmp(command, "cmdallow")) {
    do_normal_submit = process_cmd_allowdeny(&tx_message, line, REQ_CMDALLOW, REQ_CMDALLOWALL);
  } else if (!strcmp(command, "cmddeny")) {
    do_normal_submit = process_cmd_allowdeny(&tx_message, line, REQ_CMDDENY, REQ_CMDDENYALL);
  } else if (!strcmp(command, "cyclelogs")) {
    process_cmd_cyclelogs(&tx_message, line);
  } else if (!strcmp(command, "delete")) {
    do_normal_submit = process_cmd_delete(&tx_message, line);
  } else if (!strcmp(command, "deny")) {
    do_normal_submit = process_cmd_allowdeny(&tx_message, line, REQ_DENY, REQ_DENYALL);
  } else if (!strcmp(command, "dfreq")) {
    do_normal_submit = process_cmd_dfreq(&tx_message, line);
  } else if (!strcmp(command, "dns")) {
    ret = process_cmd_dns(line);
    do_normal_submit = 0;
  } else if (!strcmp(command, "doffset")) {
    do_normal_submit = process_cmd_doffset(&tx_message, line);
  } else if (!strcmp(command, "dump")) {
    process_cmd_dump(&tx_message, line);
  } else if (!strcmp(command, "exit")) {
    do_normal_submit = 0;
    quit = 1;
    ret = 1;
  } else if (!strcmp(command, "help")) {
    do_normal_submit = 0;
    give_help();
    ret = 1;
  } else if (!strcmp(command, "keygen")) {
    ret = process_cmd_keygen(line);
    do_normal_submit = 0;
  } else if (!strcmp(command, "local")) {
    do_normal_submit = process_cmd_local(&tx_message, line);
  } else if (!strcmp(command, "makestep")) {
    do_normal_submit = process_cmd_makestep(&tx_message, line);
  } else if (!strcmp(command, "manual")) {
    if (!strncmp(line, "list", 4)) {
      do_normal_submit = 0;
      ret = process_cmd_manual_list(CPS_SplitWord(line));
    } else if (!strncmp(line, "delete", 6)) {
      do_normal_submit = process_cmd_manual_delete(&tx_message, CPS_SplitWord(line));
    } else {
      do_normal_submit = process_cmd_manual(&tx_message, line);
    }
  } else if (!strcmp(command, "maxdelay")) {
    do_normal_submit = process_cmd_maxdelay(&tx_message, line);
  } else if (!strcmp(command, "maxdelaydevratio")) {
    do_normal_submit = process_cmd_maxdelaydevratio(&tx_message, line);
  } else if (!strcmp(command, "maxdelayratio")) {
    do_normal_submit = process_cmd_maxdelayratio(&tx_message, line);
  } else if (!strcmp(command, "maxpoll")) {
    do_normal_submit = process_cmd_maxpoll(&tx_message, line);
  } else if (!strcmp(command, "maxupdateskew")) {
    do_normal_submit = process_cmd_maxupdateskew(&tx_message, line);
  } else if (!strcmp(command, "minpoll")) {
    do_normal_submit = process_cmd_minpoll(&tx_message, line);
  } else if (!strcmp(command, "minstratum")) {
    do_normal_submit = process_cmd_minstratum(&tx_message, line);
  } else if (!strcmp(command, "ntpdata")) {
    do_normal_submit = 0;
    ret = process_cmd_ntpdata(line);
  } else if (!strcmp(command, "offline")) {
    do_normal_submit = process_cmd_offline(&tx_message, line);
  } else if (!strcmp(command, "offset")) {
    do_normal_submit = process_cmd_offset(&tx_message, line);
  } else if (!strcmp(command, "online")) {
    do_normal_submit = process_cmd_online(&tx_message, line);
  } else if (!strcmp(command, "onoffline")) {
    process_cmd_onoffline(&tx_message, line);
  } else if (!strcmp(command, "polltarget")) {
    do_normal_submit = process_cmd_polltarget(&tx_message, line);
  } else if (!strcmp(command, "quit")) {
    do_normal_submit = 0;
    quit = 1;
    ret = 1;
  } else if (!strcmp(command, "refresh")) {
    process_cmd_refresh(&tx_message, line);
  } else if (!strcmp(command, "rekey")) {
    process_cmd_rekey(&tx_message, line);
  } else if (!strcmp(command, "reload")) {
    do_normal_submit = process_cmd_reload(&tx_message, line);
  } else if (!strcmp(command, "reselect")) {
    process_cmd_reselect(&tx_message, line);
  } else if (!strcmp(command, "reselectdist")) {
    do_normal_submit = process_cmd_reselectdist(&tx_message, line);
  } else if (!strcmp(command, "reset")) {
    do_normal_submit = process_cmd_reset(&tx_message, line);
  } else if (!strcmp(command, "retries")) {
    ret = process_cmd_retries(line);
    do_normal_submit = 0;
  } else if (!strcmp(command, "rtcdata")) {
    do_normal_submit = 0;
    ret = process_cmd_rtcreport(line);
  } else if (!strcmp(command, "selectdata")) {
    do_normal_submit = 0;
    ret = process_cmd_selectdata(line);
  } else if (!strcmp(command, "selectopts")) {
    do_normal_submit = process_cmd_selectopts(&tx_message, line);
  } else if (!strcmp(command, "serverstats")) {
    do_normal_submit = 0;
    ret = process_cmd_serverstats(line);
  } else if (!strcmp(command, "settime")) {
    do_normal_submit = 0;
    ret = process_cmd_settime(line);
  } else if (!strcmp(command, "shutdown")) {
    process_cmd_shutdown(&tx_message, line);
  } else if (!strcmp(command, "smoothing")) {
    do_normal_submit = 0;
    ret = process_cmd_smoothing(line);
  } else if (!strcmp(command, "smoothtime")) {
    do_normal_submit = process_cmd_smoothtime(&tx_message, line);
  } else if (!strcmp(command, "sourcename")) {
    do_normal_submit = 0;
    ret = process_cmd_sourcename(line);
  } else if (!strcmp(command, "sources")) {
    do_normal_submit = 0;
    ret = process_cmd_sources(line);
  } else if (!strcmp(command, "sourcestats")) {
    do_normal_submit = 0;
    ret = process_cmd_sourcestats(line);
  } else if (!strcmp(command, "timeout")) {
    ret = process_cmd_timeout(line);
    do_normal_submit = 0;
  } else if (!strcmp(command, "tracking")) {
    ret = process_cmd_tracking(line);
    do_normal_submit = 0;
  } else if (!strcmp(command, "trimrtc")) {
    process_cmd_trimrtc(&tx_message, line);
  } else if (!strcmp(command, "waitsync")) {
    ret = process_cmd_waitsync(line);
    do_normal_submit = 0;
  } else if (!strcmp(command, "writertc")) {
    process_cmd_writertc(&tx_message, line);
  } else if (!strcmp(command, "authhash") ||
             !strcmp(command, "password")) {
    /* Warn, but don't return error to not break scripts */
    LOG(LOGS_WARN, "Authentication is no longer supported");
    do_normal_submit = 0;
    ret = 1;
  } else {
    LOG(LOGS_ERR, "Unrecognized command");
    do_normal_submit = 0;
  }
    
  if (do_normal_submit) {
    ret = request_reply(&tx_message, &rx_message, RPY_NULL, 1);
  }

  if (end_dot) {
    printf(".\n");
  }

  fflush(stderr);

  if (fflush(stdout) != 0 || ferror(stdout) != 0) {
    LOG(LOGS_ERR, "Could not write to stdout");

    /* Return error for commands that print data */
    if (!do_normal_submit)
      return 0;
  }

  return ret;
}

/* ================================================== */

#define MAX_LINE_LENGTH 2048

static int
process_args(int argc, char **argv, int multi)
{
  char line[MAX_LINE_LENGTH];
  int i, l, ret = 0;

  for (i = l = 0; i < argc; i++) {
    l += snprintf(line + l, sizeof (line) - l, "%s ", argv[i]);
    if (l >= sizeof (line)) {
      LOG(LOGS_ERR, "Command too long");
      return 0;
    }

    if (!multi && i + 1 < argc)
      continue;

    ret = process_line(line);
    if (!ret || quit)
      break;

    l = 0;
  }

  return ret;
}

/* ================================================== */

static void
signal_handler(int signum)
{
  quit = 1;
}

/* ================================================== */

static void
display_gpl(void)
{
    printf("chrony version %s\n"
           "Copyright (C) 1997-2003, 2007, 2009-2025 Richard P. Curnow and others\n"
           "chrony comes with ABSOLUTELY NO WARRANTY.  This is free software, and\n"
           "you are welcome to redistribute it under certain conditions.  See the\n"
           "GNU General Public License version 2 for details.\n\n",
           CHRONY_VERSION);
}

/* ================================================== */

static void
print_help(const char *progname)
{
      printf("Usage: %s [OPTION]... [COMMAND]...\n\n"
             "Options:\n"
             "  -4\t\tUse IPv4 addresses only\n"
             "  -6\t\tUse IPv6 addresses only\n"
             "  -n\t\tDon't resolve hostnames\n"
             "  -N\t\tPrint original source names\n"
             "  -c\t\tEnable CSV format\n"
             "  -e\t\tEnd responses with dot\n"
#if DEBUG > 0
             "  -d\t\tEnable debug messages\n"
#endif
             "  -m\t\tAccept multiple commands\n"
             "  -h HOST\tSpecify server (%s)\n"
             "  -p PORT\tSpecify UDP port (%d)\n"
             "  -v, --version\tPrint version and exit\n"
             "      --help\tPrint usage and exit\n",
             progname, DEFAULT_COMMAND_SOCKET",127.0.0.1,::1", DEFAULT_CANDM_PORT);
}

/* ================================================== */

static void
print_version(void)
{
      printf("chronyc (chrony) version %s (%s)\n", CHRONY_VERSION, CHRONYC_FEATURES);
}

/* ================================================== */

int
main(int argc, char **argv)
{
  char *line;
  const char *progname = argv[0];
  const char *hostnames = NULL;
  int opt, ret = 1, multi = 0, family = IPADDR_UNSPEC;
  int port = DEFAULT_CANDM_PORT;

  /* Parse long command-line options */
  for (optind = 1; optind < argc; optind++) {
    if (!strcmp("--help", argv[optind])) {
      print_help(progname);
      return 0;
    } else if (!strcmp("--version", argv[optind])) {
      print_version();
      return 0;
    }
  }

  optind = 1;

  /* Parse short command-line options */
  while ((opt = getopt(argc, argv, "+46acdef:h:mnNp:v")) != -1) {
    switch (opt) {
      case '4':
      case '6':
        family = opt == '4' ? IPADDR_INET4 : IPADDR_INET6;
        break;
      case 'a':
      case 'f':
        /* For compatibility only */
        break;
      case 'c':
        csv_mode = 1;
        break;
      case 'd':
#if DEBUG > 0
        log_min_severity = LOGS_DEBUG;
#endif
        break;
      case 'e':
        end_dot = 1;
        break;
      case 'h':
        hostnames = optarg;
        break;
      case 'm':
        multi = 1;
        break;
      case 'n':
        no_dns = 1;
        break;
      case 'N':
        source_names = 1;
        break;
      case 'p':
        port = atoi(optarg);
        break;
      case 'v':
        print_version();
        return 0;
      default:
        print_help(progname);
        return 1;
    }
  }

  if (isatty(0) && isatty(1) && isatty(2)) {
    on_terminal = 1;
  }

  if (on_terminal && optind == argc) {
    display_gpl();
  }
  
  DNS_SetAddressFamily(family);

  if (!hostnames) {
    hostnames = DEFAULT_COMMAND_SOCKET",127.0.0.1,::1";
  }

  UTI_SetQuitSignalsHandler(signal_handler, 0);

  SCK_Initialise(IPADDR_UNSPEC);
  server_addresses = get_addresses(hostnames, port);

  if (!open_io())
    LOG_FATAL("Could not open connection to daemon");

  if (optind < argc) {
    ret = process_args(argc - optind, argv + optind, multi);
  } else {
    do {
      line = read_line();
      if (line && !quit) {
        ret = process_line(line);
      }else {
	/* supply the final '\n' when user exits via ^D */
        if( on_terminal ) printf("\n");
      }
    } while (line && !quit);
  }

  close_io();
  free_addresses(server_addresses);
  SCK_Finalise();
  UTI_ResetGetRandomFunctions();

  return !ret;
}


