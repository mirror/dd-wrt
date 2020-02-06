/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * Copyright (C) Lonnie Abelbeck  2016, 2018
 * Copyright (C) Miroslav Lichvar  2009-2018
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
#include "logging.h"
#include "memory.h"
#include "nameserv.h"
#include "getdate.h"
#include "cmdparse.h"
#include "pktlength.h"
#include "util.h"

#ifdef FEAT_READLINE
#ifdef USE_EDITLINE
#include <editline/readline.h>
#else
#include <readline/readline.h>
#include <readline/history.h>
#endif
#endif


/* ================================================== */

union sockaddr_all {
  struct sockaddr_in in4;
#ifdef FEAT_IPV6
  struct sockaddr_in6 in6;
#endif
  struct sockaddr_un un;
  struct sockaddr sa;
};

static ARR_Instance sockaddrs;

static int sock_fd = -1;

static int quit = 0;

static int on_terminal = 0;

static int no_dns = 0;

static int csv_mode = 0;

/* ================================================== */
/* Log a message. This is a minimalistic replacement of the logging.c
   implementation to avoid linking with it and other modules. */

int log_debug_enabled = 0;

void LOG_Message(LOG_Severity severity,
#if DEBUG > 0
                 int line_number, const char *filename, const char *function_name,
#endif
                 const char *format, ...)
{
  va_list ap;

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
      /* free the buffer allocated by readline */
      Free(cmd);
    } else {
      /* simulate the user has entered an empty line */
      *line = '\0';
    }
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
get_sockaddrs(const char *hostnames, int port)
{
  ARR_Instance addrs;
  char *hostname, *s1, *s2;
  IPAddr ip_addrs[DNS_MAX_ADDRESSES];
  union sockaddr_all *addr;
  int i;

  addrs = ARR_CreateInstance(sizeof (union sockaddr_all));
  s1 = Strdup(hostnames);

  /* Parse the comma-separated list of hostnames */
  for (hostname = s1; hostname && *hostname; hostname = s2) {
    s2 = strchr(hostname, ',');
    if (s2)
      *s2++ = '\0';

    /* hostname starting with / is considered a path of Unix domain socket */
    if (hostname[0] == '/') {
      addr = (union sockaddr_all *)ARR_GetNewElement(addrs);
      if (snprintf(addr->un.sun_path, sizeof (addr->un.sun_path), "%s", hostname) >=
          sizeof (addr->un.sun_path))
        LOG_FATAL("Unix socket path too long");
      addr->un.sun_family = AF_UNIX;
    } else {
      if (DNS_Name2IPAddress(hostname, ip_addrs, DNS_MAX_ADDRESSES) != DNS_Success) {
        DEBUG_LOG("Could not get IP address for %s", hostname);
        continue;
      }

      for (i = 0; i < DNS_MAX_ADDRESSES && ip_addrs[i].family != IPADDR_UNSPEC; i++) {
        addr = (union sockaddr_all *)ARR_GetNewElement(addrs);
        UTI_IPAndPortToSockaddr(&ip_addrs[i], port, (struct sockaddr *)addr);
        DEBUG_LOG("Resolved %s to %s", hostname, UTI_IPToString(&ip_addrs[i]));
      }
    }
  }

  Free(s1);
  return addrs;
}

/* ================================================== */
/* Initialise the socket used to talk to the daemon */

static int
prepare_socket(union sockaddr_all *addr)
{
  socklen_t addr_len;
  char *dir;

  switch (addr->sa.sa_family) {
    case AF_UNIX:
      addr_len = sizeof (addr->un);
      break;
    case AF_INET:
      addr_len = sizeof (addr->in4);
      break;
#ifdef FEAT_IPV6
    case AF_INET6:
      addr_len = sizeof (addr->in6);
      break;
#endif
    default:
      assert(0);
  }

  sock_fd = socket(addr->sa.sa_family, SOCK_DGRAM, 0);

  if (sock_fd < 0) {
    DEBUG_LOG("Could not create socket : %s", strerror(errno));
    return 0;
  }

  if (addr->sa.sa_family == AF_UNIX) {
    struct sockaddr_un sa_un;

    /* Construct path of our socket.  Use the same directory as the server
       socket and include our process ID to allow multiple chronyc instances
       running at the same time. */
    dir = UTI_PathToDir(addr->un.sun_path);
    if (snprintf(sa_un.sun_path, sizeof (sa_un.sun_path),
                 "%s/chronyc.%d.sock", dir, (int)getpid()) >= sizeof (sa_un.sun_path))
      LOG_FATAL("Unix socket path too long");
    Free(dir);

    sa_un.sun_family = AF_UNIX;
    unlink(sa_un.sun_path);

    /* Bind the socket to the path */
    if (bind(sock_fd, (struct sockaddr *)&sa_un, sizeof (sa_un)) < 0) {
      DEBUG_LOG("Could not bind socket : %s", strerror(errno));
      return 0;
    }

    /* Allow server without root privileges to send replies to our socket */
    if (chmod(sa_un.sun_path, 0666) < 0) {
      DEBUG_LOG("Could not change socket permissions : %s", strerror(errno));
      return 0;
    }
  }

  if (connect(sock_fd, &addr->sa, addr_len) < 0) {
    DEBUG_LOG("Could not connect socket : %s", strerror(errno));
    return 0;
  }

  return 1;
}

/* ================================================== */

static void
close_io(void)
{
  union sockaddr_all addr;
  socklen_t addr_len = sizeof (addr);

  if (sock_fd < 0)
    return;

  /* Remove our Unix domain socket */
  if (getsockname(sock_fd, &addr.sa, &addr_len) < 0)
    LOG_FATAL("getsockname() failed : %s", strerror(errno));
  if (addr_len <= sizeof (addr) && addr_len > sizeof (addr.sa.sa_family) &&
      addr.sa.sa_family == AF_UNIX)
    unlink(addr.un.sun_path);

  close(sock_fd);
  sock_fd = -1;
}

/* ================================================== */

static int
open_io(void)
{
  static unsigned int address_index = 0;
  union sockaddr_all *addr;

  /* If a socket is already opened, close it and try the next address */
  if (sock_fd >= 0) {
    close_io();
    address_index++;
  }

  /* Find an address for which a socket can be opened and connected */
  for (; address_index < ARR_GetSize(sockaddrs); address_index++) {
    addr = (union sockaddr_all *)ARR_GetElement(sockaddrs, address_index);
    DEBUG_LOG("Opening connection to %s", UTI_SockaddrToString(&addr->sa));

    if (prepare_socket(addr))
      return 1;

    close_io();
  }

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
    default:
      assert(0);
  }
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
      if (DNS_Name2IPAddress(p, address, 1) == DNS_Success) {
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
    if (DNS_Name2IPAddress(hostname, address, 1) != DNS_Success) {
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
    if (DNS_Name2IPAddress(hostname, address, 1) != DNS_Success) {
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
  int on_off, stratum = 0, orphan = 0;
  double distance = 0.0;

  if (!strcmp(line, "off")) {
    on_off = 0;
  } else if (CPS_ParseLocal(line, &stratum, &orphan, &distance)) {
    on_off = 1;
  } else {
    LOG(LOGS_ERR, "Invalid syntax for local command");
    return 0;
  }

  msg->command = htons(REQ_LOCAL2);
  msg->data.local.on_off = htonl(on_off);
  msg->data.local.stratum = htonl(stratum);
  msg->data.local.distance = UTI_FloatHostToNetwork(distance);
  msg->data.local.orphan = htonl(orphan);

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
parse_allow_deny(CMD_Request *msg, char *line)
{
  unsigned long a, b, c, d;
  int n, specified_subnet_bits;
  IPAddr ip;
  char *p;
  
  p = line;
  if (!*p) {
    /* blank line - applies to all addresses */
    ip.family = IPADDR_UNSPEC;
    UTI_IPHostToNetwork(&ip, &msg->data.allow_deny.ip);
    msg->data.allow_deny.subnet_bits = htonl(0);
  } else {
    char *slashpos;
    slashpos = strchr(p, '/');
    if (slashpos) *slashpos = 0;
    
    n = 0;
    if (!UTI_StringToIP(p, &ip) &&
        (n = sscanf(p, "%lu.%lu.%lu.%lu", &a, &b, &c, &d)) <= 0) {

      /* Try to parse as the name of a machine */
      if (slashpos || DNS_Name2IPAddress(p, &ip, 1) != DNS_Success) {
        LOG(LOGS_ERR, "Could not read address");
        return 0;
      } else {
        UTI_IPHostToNetwork(&ip, &msg->data.allow_deny.ip);
        if (ip.family == IPADDR_INET6)
          msg->data.allow_deny.subnet_bits = htonl(128);
        else
          msg->data.allow_deny.subnet_bits = htonl(32);
      }        
    } else {
      
      if (n == 0) {
        if (ip.family == IPADDR_INET6)
          msg->data.allow_deny.subnet_bits = htonl(128);
        else
          msg->data.allow_deny.subnet_bits = htonl(32);
      } else {
        ip.family = IPADDR_INET4;

        a &= 0xff;
        b &= 0xff;
        c &= 0xff;
        d &= 0xff;
        
        switch (n) {
          case 1:
            ip.addr.in4 = htonl((a<<24));
            msg->data.allow_deny.subnet_bits = htonl(8);
            break;
          case 2:
            ip.addr.in4 = htonl((a<<24) | (b<<16));
            msg->data.allow_deny.subnet_bits = htonl(16);
            break;
          case 3:
            ip.addr.in4 = htonl((a<<24) | (b<<16) | (c<<8));
            msg->data.allow_deny.subnet_bits = htonl(24);
            break;
          case 4:
            ip.addr.in4 = htonl((a<<24) | (b<<16) | (c<<8) | d);
            msg->data.allow_deny.subnet_bits = htonl(32);
            break;
          default:
            assert(0);
        }
      }

      UTI_IPHostToNetwork(&ip, &msg->data.allow_deny.ip);

      if (slashpos) {
        n = sscanf(slashpos+1, "%d", &specified_subnet_bits);
        if (n == 1) {
          msg->data.allow_deny.subnet_bits = htonl(specified_subnet_bits);
        } else {
          LOG(LOGS_WARN, "Warning: badly formatted subnet size, using %d",
              (int)ntohl(msg->data.allow_deny.subnet_bits));
        }
      } 
    }
  }
  return 1;
}

/* ================================================== */

static int
process_cmd_allow(CMD_Request *msg, char *line)
{
  int status;
  msg->command = htons(REQ_ALLOW);
  status = parse_allow_deny(msg, line);
  return status;
}

/* ================================================== */

static int
process_cmd_allowall(CMD_Request *msg, char *line)
{
  int status;
  msg->command = htons(REQ_ALLOWALL);
  status = parse_allow_deny(msg, line);
  return status;
}

/* ================================================== */

static int
process_cmd_deny(CMD_Request *msg, char *line)
{
  int status;
  msg->command = htons(REQ_DENY);
  status = parse_allow_deny(msg, line);
  return status;
}

/* ================================================== */

static int
process_cmd_denyall(CMD_Request *msg, char *line)
{
  int status;
  msg->command = htons(REQ_DENYALL);
  status = parse_allow_deny(msg, line);
  return status;
}

/* ================================================== */

static int
process_cmd_cmdallow(CMD_Request *msg, char *line)
{
  int status;
  msg->command = htons(REQ_CMDALLOW);
  status = parse_allow_deny(msg, line);
  return status;
}

/* ================================================== */

static int
process_cmd_cmdallowall(CMD_Request *msg, char *line)
{
  int status;
  msg->command = htons(REQ_CMDALLOWALL);
  status = parse_allow_deny(msg, line);
  return status;
}

/* ================================================== */

static int
process_cmd_cmddeny(CMD_Request *msg, char *line)
{
  int status;
  msg->command = htons(REQ_CMDDENY);
  status = parse_allow_deny(msg, line);
  return status;
}

/* ================================================== */

static int
process_cmd_cmddenyall(CMD_Request *msg, char *line)
{
  int status;
  msg->command = htons(REQ_CMDDENYALL);
  status = parse_allow_deny(msg, line);
  return status;
}

/* ================================================== */

static int
accheck_getaddr(char *line, IPAddr *addr)
{
  unsigned long a, b, c, d;
  IPAddr ip;
  char *p;
  p = line;
  if (!*p) {
    return 0;
  } else {
    if (sscanf(p, "%lu.%lu.%lu.%lu", &a, &b, &c, &d) == 4) {
      addr->family = IPADDR_INET4;
      addr->addr.in4 = (a<<24) | (b<<16) | (c<<8) | d;
      return 1;
    } else {
      if (DNS_Name2IPAddress(p, &ip, 1) != DNS_Success) {
        return 0;
      } else {
        *addr = ip;
        return 1;
      }
    }
  }
}

/* ================================================== */

static int
process_cmd_accheck(CMD_Request *msg, char *line)
{
  IPAddr ip;
  msg->command = htons(REQ_ACCHECK);
  if (accheck_getaddr(line, &ip)) {
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
  if (accheck_getaddr(line, &ip)) {
    UTI_IPHostToNetwork(&ip, &msg->data.ac_check.ip);
    return 1;
  } else {    
    LOG(LOGS_ERR, "Could not read address");
    return 0;
  }
}

/* ================================================== */

static void
process_cmd_dfreq(CMD_Request *msg, char *line)
{
  double dfreq;
  msg->command = htons(REQ_DFREQ);
  if (sscanf(line, "%lf", &dfreq) == 1) {
    msg->data.dfreq.dfreq = UTI_FloatHostToNetwork(dfreq);
  } else {
    msg->data.dfreq.dfreq = UTI_FloatHostToNetwork(0.0);
  }
}

/* ================================================== */

static void
cvt_to_sec_usec(double x, long *sec, long *usec) {
  long s, us;
  s = (long) x;
  us = (long)(0.5 + 1.0e6 * (x - (double) s));
  while (us >= 1000000) {
    us -= 1000000;
    s += 1;
  }
  while (us < 0) {
    us += 1000000;
    s -= 1;
  }
  
  *sec = s;
  *usec = us;
}

/* ================================================== */

static void
process_cmd_doffset(CMD_Request *msg, char *line)
{
  double doffset;
  long sec, usec;
  msg->command = htons(REQ_DOFFSET);
  if (sscanf(line, "%lf", &doffset) == 1) {
    cvt_to_sec_usec(doffset, &sec, &usec);
    msg->data.doffset.sec = htonl(sec);
    msg->data.doffset.usec = htonl(usec);
  } else {
    msg->data.doffset.sec = htonl(0);
    msg->data.doffset.usec = htonl(0);
  }
}

/* ================================================== */

static int
process_cmd_add_server_or_peer(CMD_Request *msg, char *line)
{
  CPS_NTP_Source data;
  IPAddr ip_addr;
  int result = 0, status;
  const char *opt_name;
  
  status = CPS_ParseNTPSourceAdd(line, &data);
  switch (status) {
    case 0:
      LOG(LOGS_ERR, "Invalid syntax for add command");
      break;
    default:
      if (DNS_Name2IPAddress(data.name, &ip_addr, 1) != DNS_Success) {
        LOG(LOGS_ERR, "Invalid host/IP address");
        break;
      }

      opt_name = NULL;
      if (opt_name) {
        LOG(LOGS_ERR, "%s can't be set in chronyc", opt_name);
        break;
      }

      msg->data.ntp_source.port = htonl((unsigned long) data.port);
      UTI_IPHostToNetwork(&ip_addr, &msg->data.ntp_source.ip_addr);
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
          (data.params.sel_options & SRC_SELECT_PREFER ? REQ_ADDSRC_PREFER : 0) |
          (data.params.sel_options & SRC_SELECT_NOSELECT ? REQ_ADDSRC_NOSELECT : 0) |
          (data.params.sel_options & SRC_SELECT_TRUST ? REQ_ADDSRC_TRUST : 0) |
          (data.params.sel_options & SRC_SELECT_REQUIRE ? REQ_ADDSRC_REQUIRE : 0));
      msg->data.ntp_source.filter_length = htonl(data.params.filter_length);
      memset(msg->data.ntp_source.reserved, 0, sizeof (msg->data.ntp_source.reserved));

      result = 1;

      break;
  }

  return result;
}

/* ================================================== */

static int
process_cmd_add_server(CMD_Request *msg, char *line)
{
  msg->command = htons(REQ_ADD_SERVER3);
  return process_cmd_add_server_or_peer(msg, line);
}

/* ================================================== */

static int
process_cmd_add_peer(CMD_Request *msg, char *line)
{
  msg->command = htons(REQ_ADD_PEER3);
  return process_cmd_add_server_or_peer(msg, line);
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
    if (DNS_Name2IPAddress(hostname, &address, 1) != DNS_Success) {
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
    "sources [-v]\0Display information about current sources\0"
    "sourcestats [-v]\0Display statistics about collected measurements\0"
    "reselect\0Force reselecting synchronisation source\0"
    "reselectdist <dist>\0Modify reselection distance\0"
    "\0\0"
    "NTP sources:\0\0"
    "activity\0Check how many NTP sources are online/offline\0"
    "ntpdata [<address>]\0Display information about last valid measurement\0"
    "add server <address> [options]\0Add new NTP server\0"
    "add peer <address> [options]\0Add new NTP peer\0"
    "delete <address>\0Remove server or peer\0"
    "burst <n-good>/<n-max> [<mask>/<address>]\0Start rapid set of measurements\0"
    "maxdelay <address> <delay>\0Modify maximum valid sample delay\0"
    "maxdelayratio <address> <ratio>\0Modify maximum valid delay/minimum ratio\0"
    "maxdelaydevratio <address> <ratio>\0Modify maximum valid delay/deviation ratio\0"
    "minpoll <address> <poll>\0Modify minimum polling interval\0"
    "maxpoll <address> <poll>\0Modify maximum polling interval\0"
    "minstratum <address> <stratum>\0Modify minimum stratum\0"
    "offline [<mask>/<address>]\0Set sources in subnet to offline status\0"
    "online [<mask>/<address>]\0Set sources in subnet to online status\0"
    "onoffline\0Set all sources to online or offline status\0"
    "\0according to network configuration\0"
    "polltarget <address> <target>\0Modify poll target\0"
    "refresh\0Refresh IP addresses\0"
    "\0\0"
    "Manual time input:\0\0"
    "manual off|on|reset\0Disable/enable/reset settime command\0"
    "manual list\0Show previous settime entries\0"
    "manual delete <index>\0Delete previous settime entry\0"
    "settime <time>\0Set daemon time\0"
    "\0(e.g. Sep 25, 2015 16:30:05 or 16:30:05)\0"
    "\0\0NTP access:\0\0"
    "accheck <address>\0Check whether address is allowed\0"
    "clients\0Report on clients that have accessed the server\0"
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
    "dump\0Dump all measurements to save files\0"
    "rekey\0Re-read keys from key file\0"
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
/* Tab-completion when editline/readline is available */

#ifdef FEAT_READLINE

enum {
  TAB_COMPLETE_BASE_CMDS,
  TAB_COMPLETE_ADD_OPTS,
  TAB_COMPLETE_MANUAL_OPTS,
  TAB_COMPLETE_SOURCES_OPTS,
  TAB_COMPLETE_SOURCESTATS_OPTS,
  TAB_COMPLETE_MAX_INDEX
};

static int tab_complete_index;

static char *
command_name_generator(const char *text, int state)
{
  const char *name, **names[TAB_COMPLETE_MAX_INDEX];
  const char *base_commands[] = {
    "accheck", "activity", "add", "allow", "burst",
    "clients", "cmdaccheck", "cmdallow", "cmddeny", "cyclelogs", "delete",
    "deny", "dns", "dump", "exit", "help", "keygen", "local", "makestep",
    "manual", "maxdelay", "maxdelaydevratio", "maxdelayratio", "maxpoll",
    "maxupdateskew", "minpoll", "minstratum", "ntpdata", "offline", "online", "onoffline",
    "polltarget", "quit", "refresh", "rekey", "reselect", "reselectdist",
    "retries", "rtcdata", "serverstats", "settime", "shutdown", "smoothing",
    "smoothtime", "sources", "sourcestats",
    "timeout", "tracking", "trimrtc", "waitsync", "writertc",
    NULL
  };
  const char *add_options[] = { "peer", "server", NULL };
  const char *manual_options[] = { "on", "off", "delete", "list", "reset", NULL };
  const char *sources_options[] = { "-v", NULL };
  const char *sourcestats_options[] = { "-v", NULL };
  static int list_index, len;

  names[TAB_COMPLETE_BASE_CMDS] = base_commands;
  names[TAB_COMPLETE_ADD_OPTS] = add_options;
  names[TAB_COMPLETE_MANUAL_OPTS] = manual_options;
  names[TAB_COMPLETE_SOURCES_OPTS] = sources_options;
  names[TAB_COMPLETE_SOURCESTATS_OPTS] = sourcestats_options;

  if (!state) {
    list_index = 0;
    len = strlen(text);
  }

  while ((name = names[tab_complete_index][list_index++])) {
    if (strncmp(name, text, len) == 0) {
      return strdup(name);
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
  } else if (!strcmp(first, "manual ")) {
    tab_complete_index = TAB_COMPLETE_MANUAL_OPTS;
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

      if (send(sock_fd, (void *)request, command_length, 0) < 0) {
        DEBUG_LOG("Could not send %d bytes : %s", command_length, strerror(errno));
        return 0;
      }

      DEBUG_LOG("Sent %d bytes", command_length);
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
      recv_status = recv(sock_fd, (void *)reply, sizeof(CMD_Reply), 0);
      
      if (recv_status < 0) {
        /* If we get connrefused here, it suggests the sendto is
           going to a dead port */
        DEBUG_LOG("Could not receive : %s", strerror(errno));
        new_attempt = 1;
      } else {
        DEBUG_LOG("Received %d bytes", recv_status);
        
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
print_seconds(unsigned long s)
{
  unsigned long d;

  if (s == (uint32_t)-1) {
    printf("   -");
  } else if (s < 1200) {
    printf("%4lu", s);
  } else if (s < 36000) {
    printf("%3lum", s / 60);
  } else if (s < 345600) {
    printf("%3luh", s / 3600);
  } else {
    d = s / 86400;
    if (d > 999) {
      printf("%3luy", d / 365);
    } else {
      printf("%3lud", d);
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
  unsigned long long_uinteger;
  unsigned int uinteger;
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
      case 'I': /* interval with unit */
        long_uinteger = va_arg(ap, unsigned long);
        print_seconds(long_uinteger);
        break;
      case 'L': /* leap status */
        integer = va_arg(ap, int);
        switch (integer) {
          case LEAP_Normal:
            string = "Normal";
            break;
          case LEAP_InsertSecond:
            string = "Insert second";
            break;
          case LEAP_DeleteSecond:
            string = "Delete second";
            break;
          case LEAP_Unsynchronised:
            string = "Not synchronised";
            break;
          default:
            string = "Invalid";
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
        long_uinteger = va_arg(ap, unsigned long);
        printf("%08lX", long_uinteger);
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
      case 'U': /* unsigned long in decimal */
        long_uinteger = va_arg(ap, unsigned long);
        printf("%*lu", width, long_uinteger);
        break;
      case 'V': /* timespec as seconds since epoch */
        ts = va_arg(ap, struct timespec *);
        printf("%s", UTI_TimespecToString(ts));
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

static void
format_name(char *buf, int size, int trunc_dns, int ref, uint32_t ref_id,
            IPAddr *ip_addr)
{
  if (ref) {
    snprintf(buf, size, "%s", UTI_RefidToString(ref_id));
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

static int
check_for_verbose_flag(char *line)
{
  if (!csv_mode && !strcmp(line, "-v"))
    return 1;
  return 0;
}

/* ================================================== */

static int
process_cmd_sources(char *line)
{
  CMD_Request request;
  CMD_Reply reply;
  IPAddr ip_addr;
  uint32_t i, mode, n_sources;
  char name[50], mode_ch, state_ch;
  int verbose;

  /* Check whether to output verbose headers */
  verbose = check_for_verbose_flag(line);
  
  request.command = htons(REQ_N_SOURCES);
  if (!request_reply(&request, &reply, RPY_N_SOURCES, 0))
    return 0;

  n_sources = ntohl(reply.data.n_sources.n_sources);
  print_info_field("210 Number of sources = %lu\n", (unsigned long)n_sources);

  if (verbose) {
    printf("\n");
    printf("  .-- Source mode  '^' = server, '=' = peer, '#' = local clock.\n");
    printf(" / .- Source state '*' = current synced, '+' = combined , '-' = not combined,\n");
    printf("| /   '?' = unreachable, 'x' = time may be in error, '~' = time too variable.\n");
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
    format_name(name, sizeof (name), 25,
                mode == RPY_SD_MD_REF && ip_addr.family == IPADDR_INET4,
                ip_addr.addr.in4, &ip_addr);

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
      case RPY_SD_ST_SYNC:
        state_ch = '*';
        break;
      case RPY_SD_ST_UNREACH:
        state_ch = '?';
        break;
      case RPY_SD_ST_FALSETICKER:
        state_ch = 'x';
        break;
      case RPY_SD_ST_JITTERY:
        state_ch = '~';
        break;
      case RPY_SD_ST_CANDIDATE:
        state_ch = '+';
        break;
      case RPY_SD_ST_OUTLIER:
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
                 (unsigned long)ntohl(reply.data.source_data.since_sample),
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
  int verbose = 0;
  char name[50];
  IPAddr ip_addr;

  verbose = check_for_verbose_flag(line);

  request.command = htons(REQ_N_SOURCES);
  if (!request_reply(&request, &reply, RPY_N_SOURCES, 0))
    return 0;

  n_sources = ntohl(reply.data.n_sources.n_sources);
  print_info_field("210 Number of sources = %lu\n", (unsigned long)n_sources);

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
    format_name(name, sizeof (name), 25, ip_addr.family == IPADDR_UNSPEC,
                ntohl(reply.data.sourcestats.ref_id), &ip_addr);

    print_report("%-25s %3U %3U  %I %+P %P  %+S  %S\n",
                 name,
                 (unsigned long)ntohl(reply.data.sourcestats.n_samples),
                 (unsigned long)ntohl(reply.data.sourcestats.n_runs),
                 (unsigned long)ntohl(reply.data.sourcestats.span_seconds),
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
  char name[50];
  struct timespec ref_time;
  
  request.command = htons(REQ_TRACKING);
  if (!request_reply(&request, &reply, RPY_TRACKING, 0))
    return 0;

  ref_id = ntohl(reply.data.tracking.ref_id);

  UTI_IPNetworkToHost(&reply.data.tracking.ip_addr, &ip_addr);
  format_name(name, sizeof (name), sizeof (name),
              ip_addr.family == IPADDR_UNSPEC, ref_id, &ip_addr);

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
               (unsigned long)ref_id, name,
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
      if (DNS_Name2IPAddress(line, &remote_addr, 1) != DNS_Success) {
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
    }

    request.command = htons(REQ_NTP_DATA);
    UTI_IPHostToNetwork(&remote_addr, &request.data.ntp_data.ip_addr);
    if (!request_reply(&request, &reply, RPY_NTP_DATA, 0))
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
                 "Total valid RX  : %U\n",
                 UTI_IPToString(&remote_addr), (unsigned long)UTI_IPToRefid(&remote_addr),
                 ntohs(reply.data.ntp_data.remote_port),
                 UTI_IPToString(&local_addr), (unsigned long)UTI_IPToRefid(&local_addr),
                 reply.data.ntp_data.leap, reply.data.ntp_data.version,
                 reply.data.ntp_data.mode, reply.data.ntp_data.stratum,
                 reply.data.ntp_data.poll, UTI_Log2ToDouble(reply.data.ntp_data.poll),
                 reply.data.ntp_data.precision, UTI_Log2ToDouble(reply.data.ntp_data.precision),
                 UTI_FloatNetworkToHost(reply.data.ntp_data.root_delay),
                 UTI_FloatNetworkToHost(reply.data.ntp_data.root_dispersion),
                 (unsigned long)ntohl(reply.data.ntp_data.ref_id),
                 reply.data.ntp_data.stratum <= 1 ?
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
                 (unsigned long)ntohl(reply.data.ntp_data.total_tx_count),
                 (unsigned long)ntohl(reply.data.ntp_data.total_rx_count),
                 (unsigned long)ntohl(reply.data.ntp_data.total_valid_count),
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
  if (!request_reply(&request, &reply, RPY_SERVER_STATS, 0))
    return 0;

  print_report("NTP packets received       : %U\n"
               "NTP packets dropped        : %U\n"
               "Command packets received   : %U\n"
               "Command packets dropped    : %U\n"
               "Client log records dropped : %U\n",
               (unsigned long)ntohl(reply.data.server_stats.ntp_hits),
               (unsigned long)ntohl(reply.data.server_stats.ntp_drops),
               (unsigned long)ntohl(reply.data.server_stats.cmd_hits),
               (unsigned long)ntohl(reply.data.server_stats.cmd_drops),
               (unsigned long)ntohl(reply.data.server_stats.log_drops),
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
               (unsigned long)ntohl(reply.data.rtc.span_seconds),
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
  uint32_t i, n_clients, next_index, n_indices;
  RPY_ClientAccesses_Client *client;
  char name[50];

  next_index = 0;

  print_header("Hostname                      NTP   Drop Int IntL Last     Cmd   Drop Int  Last");

  while (1) {
    request.command = htons(REQ_CLIENT_ACCESSES_BY_INDEX2);
    request.data.client_accesses_by_index.first_index = htonl(next_index);
    request.data.client_accesses_by_index.n_clients = htonl(MAX_CLIENT_ACCESSES);

    if (!request_reply(&request, &reply, RPY_CLIENT_ACCESSES_BY_INDEX2, 0))
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

      format_name(name, sizeof (name), 25, 0, 0, &ip);

      print_report("%-25s  %6U  %5U  %C  %C  %I  %6U  %5U  %C  %I\n",
                   name,
                   (unsigned long)ntohl(client->ntp_hits),
                   (unsigned long)ntohl(client->ntp_drops),
                   client->ntp_interval,
                   client->ntp_timeout_interval,
                   (unsigned long)ntohl(client->last_ntp_hit_ago),
                   (unsigned long)ntohl(client->cmd_hits),
                   (unsigned long)ntohl(client->cmd_drops),
                   client->cmd_interval,
                   (unsigned long)ntohl(client->last_cmd_hit_ago),
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
  print_info_field("210 n_samples = %lu\n", (unsigned long)n_samples);

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
               (unsigned long)ntohl(reply.data.activity.online),
               (unsigned long)ntohl(reply.data.activity.offline),
               (unsigned long)ntohl(reply.data.activity.burst_online),
               (unsigned long)ntohl(reply.data.activity.burst_offline),
               (unsigned long)ntohl(reply.data.activity.unresolved),
               REPORT_END);

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
                   i, (unsigned long)ref_id, correction, skew_ppm, REPORT_END);

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
  char hash_name[17];
  unsigned char key[512];
  unsigned int i, length, id = 1, bits = 160;

#ifdef FEAT_SECHASH
  snprintf(hash_name, sizeof (hash_name), "SHA1");
#else
  snprintf(hash_name, sizeof (hash_name), "MD5");
#endif

  if (sscanf(line, "%u %16s %u", &id, hash_name, &bits))
    ;

  length = CLAMP(10, (bits + 7) / 8, sizeof (key));
  if (HSH_GetHashId(hash_name) < 0) {
    LOG(LOGS_ERR, "Unknown hash function %s", hash_name);
    return 0;
  }

  UTI_GetRandomBytesUrandom(key, length);

  printf("%u %s HEX:", id, hash_name);
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
  } else if (!strcmp(command, "add") && !strncmp(line, "peer", 4)) {
    do_normal_submit = process_cmd_add_peer(&tx_message, CPS_SplitWord(line));
  } else if (!strcmp(command, "add") && !strncmp(line, "server", 6)) {
    do_normal_submit = process_cmd_add_server(&tx_message, CPS_SplitWord(line));
  } else if (!strcmp(command, "allow")) {
    if (!strncmp(line, "all", 3)) {
      do_normal_submit = process_cmd_allowall(&tx_message, CPS_SplitWord(line));
    } else {
      do_normal_submit = process_cmd_allow(&tx_message, line);
    }
  } else if (!strcmp(command, "burst")) {
    do_normal_submit = process_cmd_burst(&tx_message, line);
  } else if (!strcmp(command, "clients")) {
    ret = process_cmd_clients(line);
    do_normal_submit = 0;
  } else if (!strcmp(command, "cmdaccheck")) {
    do_normal_submit = process_cmd_cmdaccheck(&tx_message, line);
  } else if (!strcmp(command, "cmdallow")) {
    if (!strncmp(line, "all", 3)) {
      do_normal_submit = process_cmd_cmdallowall(&tx_message, CPS_SplitWord(line));
    } else {
      do_normal_submit = process_cmd_cmdallow(&tx_message, line);
    }
  } else if (!strcmp(command, "cmddeny")) {
    if (!strncmp(line, "all", 3)) {
      line = CPS_SplitWord(line);
      do_normal_submit = process_cmd_cmddenyall(&tx_message, line);
    } else {
      do_normal_submit = process_cmd_cmddeny(&tx_message, line);
    }
  } else if (!strcmp(command, "cyclelogs")) {
    process_cmd_cyclelogs(&tx_message, line);
  } else if (!strcmp(command, "delete")) {
    do_normal_submit = process_cmd_delete(&tx_message, line);
  } else if (!strcmp(command, "deny")) {
    if (!strncmp(line, "all", 3)) {
      do_normal_submit = process_cmd_denyall(&tx_message, CPS_SplitWord(line));
    } else {
      do_normal_submit = process_cmd_deny(&tx_message, line);
    }
  } else if (!strcmp(command, "dfreq")) {
    process_cmd_dfreq(&tx_message, line);
  } else if (!strcmp(command, "dns")) {
    ret = process_cmd_dns(line);
    do_normal_submit = 0;
  } else if (!strcmp(command, "doffset")) {
    process_cmd_doffset(&tx_message, line);
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
  } else if (!strcmp(command, "reselect")) {
    process_cmd_reselect(&tx_message, line);
  } else if (!strcmp(command, "reselectdist")) {
    do_normal_submit = process_cmd_reselectdist(&tx_message, line);
  } else if (!strcmp(command, "retries")) {
    ret = process_cmd_retries(line);
    do_normal_submit = 0;
  } else if (!strcmp(command, "rtcdata")) {
    do_normal_submit = 0;
    ret = process_cmd_rtcreport(line);
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
  fflush(stderr);
  fflush(stdout);
  return ret;
}

/* ================================================== */

static int
process_args(int argc, char **argv, int multi)
{
  int total_length, i, ret = 0;
  char *line;

  total_length = 0;
  for(i=0; i<argc; i++) {
    total_length += strlen(argv[i]) + 1;
  }

  line = (char *) Malloc((2 + total_length) * sizeof(char));

  for (i = 0; i < argc; i++) {
    line[0] = '\0';
    if (multi) {
      strcat(line, argv[i]);
    } else {
      for (; i < argc; i++) {
        strcat(line, argv[i]);
        if (i + 1 < argc)
          strcat(line, " ");
      }
    }

    ret = process_line(line);
    if (!ret || quit)
      break;
  }

  Free(line);

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
           "Copyright (C) 1997-2003, 2007, 2009-2019 Richard P. Curnow and others\n"
           "chrony comes with ABSOLUTELY NO WARRANTY.  This is free software, and\n"
           "you are welcome to redistribute it under certain conditions.  See the\n"
           "GNU General Public License version 2 for details.\n\n",
           CHRONY_VERSION);
}

/* ================================================== */

static void
print_help(const char *progname)
{
      printf("Usage: %s [-h HOST] [-p PORT] [-n] [-c] [-d] [-4|-6] [-m] [COMMAND]\n",
             progname);
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

  /* Parse (undocumented) long command-line options */
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
  while ((opt = getopt(argc, argv, "+46acdf:h:mnp:v")) != -1) {
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
        log_debug_enabled = 1;
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

  sockaddrs = get_sockaddrs(hostnames, port);

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

  ARR_DestroyInstance(sockaddrs);

  return !ret;
}


