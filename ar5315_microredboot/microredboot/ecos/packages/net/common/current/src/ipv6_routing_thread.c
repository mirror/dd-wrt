//==========================================================================
//
//      ipv6_routing_thread.c
//
//      IPv6 routing support
//
//==========================================================================
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD or other sources,
// and are covered by the appropriate copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas, lhamilton, andrew.lunn@ascom.ch
// Date:         2002-04-16
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// IPv6 routing support

#include <pkgconf/net.h>
#undef _KERNEL
#include <sys/param.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <errno.h>

#include <cyg/kernel/kapi.h>
#include <cyg/infra/diag.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <net/if_var.h>
#include <netinet/in.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <netdb.h>
#include <netinet6/in6_var.h>
#include <netinet6/nd6.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void _show_all_interfaces(void);

//#define DBG_PRINT                          1
#define ALLROUTER                  "ff02::2"

#define STACK_SIZE    CYGNUM_HAL_STACK_SIZE_TYPICAL+2048
#define _1_MILLION                   1000000
#define STATE_DELAY                        0
#define STATE_PROBE                        1
#define STATE_IDLE                         2

#define MAX_RTR_SOLICITATIONS              3
#define SEL_TIMEOUT_IDLE                 145
#define SECONDS_TILL_RS_SEND      (12*60*60)

static char rs_stack[STACK_SIZE];
static cyg_thread rs_thread_data;
static cyg_handle_t rs_thread_handle;
static int s;
static int probe=0;
static int racount=0;
static int state;
static int badra;
static int cc = sizeof(struct icmp6_hdr);
static u_char outpack[sizeof(struct icmp6_hdr)];
struct sockaddr_in6 to;
static struct timeval select_timeout;
static struct timeval last_ra_time;
static struct timeval idle_expire;
static struct icmp6stat *istat;
extern struct icmp6stat cyg_icmp6stat;
static struct sockaddr_in6 router;
static int router_valid = 0;

// Return the address of the last router to send us a router advertisement 
// Copy the address into addr and return true. If we have not received an
// advertisement yet, return false
int cyg_net_get_ipv6_advrouter(struct sockaddr_in6 * addr) {
  
  if (addr) {
    memcpy(addr,&router,sizeof(*addr));
  }
  return router_valid;
}

static void
cyg_rs_exit(void)
{
  diag_printf("<%s>\n", __FUNCTION__);
  cyg_thread_exit();
}

static void
dprnt(const char *msg, ...)
{
#ifdef DBG_PRINT
  va_list ap;

  va_start(ap, msg);
  (void) vfprintf(stdout, msg, ap);
  (void) fprintf(stdout, "\n");
  va_end(ap);
#endif
}

#ifdef DBG_PRINT
void
sleep_msg(char *str, int total)
{
  int hours, minutes;

  hours = total / 3600;
  total -= hours * 3600;
  minutes = total / 60;
  total -= minutes * 60;

  if (hours > 0)
    dprnt("%s: %d hr, %d min", str, hours, minutes);
  else if (minutes > 0)
    dprnt("%s: %d min, %d sec", str, minutes, total);
  else
    dprnt("%s: %d sec", str, total);
}
#endif

void
get_realtime(struct timeval *now)
{
  struct timespec tp;
  clock_gettime(CLOCK_REALTIME, &tp);
  now->tv_sec = tp.tv_sec;
  now->tv_usec = 0;
}

void
send_rs_packet(void)
{
  probe++;

  dprnt("%s: probe=%d", __FUNCTION__, probe);

  if (sendto(s, outpack, cc, 0, (struct sockaddr *)&to, to.sin6_len) < 0) {
    dprnt("%s: can't send RS: %s", __FUNCTION__, strerror(errno));
  }
}

void
receive_ra_packet(void)
{
  int len;
  char msg[1024];
  struct nd_router_advert *advert;
  cyg_uint8 *opts;
  int optlen;
  int addrlen = sizeof(router);
  char buf[64];

  dprnt("%s", __FUNCTION__);

  len = recvfrom(s, msg, sizeof(msg),0,(struct sockaddr *)&router,&addrlen);
  if (len < 0) {
    dprnt("  Can't read RA data: %s", strerror(errno));
    return;
  }
  router_valid = 1;

  inet_ntop(AF_INET6,(void *)&router.sin6_addr,buf,64);  
  dprnt("Router Advertisement from %s",buf);
  dprnt("packet data (buf len=%d) :", len);
#ifdef DBG_PRINT  
  diag_dump_buf(msg, len);
#endif

  advert = (struct nd_router_advert *)msg;

  dprnt("AdvCurHopLimit: %d", (int) advert->nd_ra_curhoplimit);
  dprnt("AdvManagedFlag: %s",
	(advert->nd_ra_flags_reserved & ND_RA_FLAG_MANAGED) ? "yes" : "no");
  dprnt("AdvOtherConfigFlag: %s",
	(advert->nd_ra_flags_reserved & ND_RA_FLAG_OTHER) ? "yes":"no");
  dprnt("AdvHomeAgentFlag: %s",
	 (advert->nd_ra_flags_reserved & ND_RA_FLAG_HOME_AGENT) ? 
	 "yes" : "no");
  dprnt("AdvReachableTime: %lu", 
	 (unsigned long)ntohl(advert->nd_ra_reachable));
  dprnt("AdvRetransTimer: %lu", 
	(unsigned long)ntohl(advert->nd_ra_retransmit));
  
  // Now process the optinal fields 
  len -= sizeof(*advert);
  opts = (cyg_uint8 *)(msg + sizeof(struct nd_router_advert));

  while (len > 0) {
     optlen = (opts[1] << 3);
     switch (*opts) {
     case ND_OPT_MTU: {
       struct nd_opt_mtu *mtu = (struct nd_opt_mtu *) opts;

       dprnt("AdvLinkMTU: %lu", 
	     (unsigned long)ntohl(mtu->nd_opt_mtu_mtu));
       break;
     }
     case ND_OPT_PREFIX_INFORMATION: {
       struct nd_opt_prefix_info *pinfo = (struct nd_opt_prefix_info *) opts;

       inet_ntop(AF_INET6,(void *)&pinfo->nd_opt_pi_prefix,buf,64);
       dprnt("Prefix:  %s/%d",buf,pinfo->nd_opt_pi_prefix_len);
       break;
     }
     case ND_OPT_SOURCE_LINKADDR: {
       char *cp = buf;
       int i;
       
       for (i = 2 ; i < optlen ; i++) {
	 cp += diag_sprintf(cp,"%02x ", (unsigned int) opts[i]);
       }
      
       dprnt("AdvSourceLinkAddress: %s",buf);
       break;
     }
     case ND_OPT_ADVINTERVAL:
     case ND_OPT_HOMEAGENT_INFO:
       dprnt("Mobile IPv6 entension options (not decoded)");
       break;
     default:
       dprnt("Unknown option %d\n", (int)*opts);
       break;
     }
     len -= optlen;
     opts += optlen;
  }
       
  racount++;
  get_realtime(&last_ra_time);
  dprnt("racount=%d, timestamp=%d sec",
	racount, last_ra_time.tv_sec);

  if (badra != istat->icp6s_badra) {
    /* BAD RA received by ipv6 stack (nd6_ra_input).
       Resend RS packet. */
    badra = istat->icp6s_badra;
    dprnt("<Bad RA (Router Advertisement)>");
    return;
  }
  /* Check the ICMPv6 RA Code */
  if (msg[1] != 0) {
    len = (int)msg[1];
    dprnt("<Bad ICMPv6 Code: %d>", len);
    return;
  }

  /* RA received, go idle for SECONDS_TILL_RS_SEND */
  idle_expire.tv_sec = last_ra_time.tv_sec +
    SECONDS_TILL_RS_SEND;
  probe = 0;
  select_timeout.tv_sec = SEL_TIMEOUT_IDLE;
  if (state != STATE_IDLE)
    dprnt("Going IDLE, thread listening");
  state = STATE_IDLE;
}

void
check_timer(void)
{
  struct timeval now;

  if (state == STATE_DELAY) {
    probe = 0;
    select_timeout.tv_sec = RTR_SOLICITATION_INTERVAL;
    state = STATE_PROBE;
    send_rs_packet();
  } else if (state == STATE_PROBE) {
    dprnt("%s: state: PROBE", __FUNCTION__);
    if (probe < MAX_RTR_SOLICITATIONS) {
      send_rs_packet();
    } else {
      dprnt("Maximum %d rtr solicitations sent out",
	    MAX_RTR_SOLICITATIONS);
      get_realtime(&now);
      probe = 0;
      idle_expire.tv_sec = now.tv_sec +
	SECONDS_TILL_RS_SEND;
      dprnt("Going IDLE");
      select_timeout.tv_sec = SEL_TIMEOUT_IDLE;
      state = STATE_IDLE;
#ifdef DBG_PRINT
      sleep_msg("Thread listening (state=IDLE)",
		SECONDS_TILL_RS_SEND);
#endif
    }
  } else {
    /* STATE_IDLE */
    get_realtime(&now);
    if (now.tv_sec >= idle_expire.tv_sec) {
      dprnt("<Leaving IDLE; Woke up to retry RS send>");
      select_timeout.tv_sec = 1;
      state = STATE_DELAY;
    }
#ifdef DBG_PRINT
    else if ((idle_expire.tv_sec - now.tv_sec) < (5 * 60)) {
      sleep_msg("Coming out of IDLE in",
		idle_expire.tv_sec - now.tv_sec);
    }
#endif
  }
}

static void
cyg_rs(cyg_addrword_t param)
{
  struct addrinfo hints, *res;
  struct icmp6_hdr *icp;
  struct icmp6_filter filt;
  struct timeval loop_delay;
  int err, status;
  u_int hlim = 255;
  fd_set fdset;

#ifdef DBG_PRINT
  _show_all_interfaces();
#endif

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET6;
  hints.ai_flags = AI_NUMERICHOST;
  err = getaddrinfo(ALLROUTER, NULL, &hints, &res);
  if (err) {
    dprnt("%s - failed to get ALL ROUTER: %s",
		__FUNCTION__, gai_strerror(err));
    cyg_rs_exit();
  }
  memcpy(&to, res->ai_addr, res->ai_addrlen);
  *(u_int16_t *)&to.sin6_addr.s6_addr[2] = htons(1);

  if ((s = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6)) < 0) {
    dprnt("%s - can't open socket: %s",
		__FUNCTION__, strerror(errno));
    cyg_rs_exit();
  }
  if (setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_HOPS,
		 &hlim, sizeof(hlim)) < 0) {
    dprnt("%s - can't set IPV6 MULTICAST HOPS: %s",
		__FUNCTION__, strerror(errno));
    cyg_rs_exit();
  }

  /* Specify to accept only router advertisements on the socket */
  ICMP6_FILTER_SETBLOCKALL(&filt);
  ICMP6_FILTER_SETPASS(ND_ROUTER_ADVERT, &filt);
  if (setsockopt(s, IPPROTO_ICMPV6, ICMP6_FILTER,
		 &filt, sizeof(filt)) < 0) {
    dprnt("%s - can't set ICMP6 filter: %s",
		__FUNCTION__, strerror(errno));
    cyg_rs_exit();
  }

  idle_expire.tv_usec = 0;
  select_timeout.tv_usec = 0;
  last_ra_time.tv_usec = 0;
  last_ra_time.tv_sec = 2;
  select(0, 0, 0, 0, &last_ra_time);

  racount = 0;
  istat = &(cyg_icmp6stat);
  badra = istat->icp6s_badra;

  icp = (struct icmp6_hdr *)outpack;
  icp->icmp6_type = ND_ROUTER_SOLICIT;
  icp->icmp6_code = 0;
  icp->icmp6_cksum = 0;
  icp->icmp6_data32[0] = 0; /* RS reserved field */
  state = STATE_DELAY;

  while (true) {
    FD_ZERO(&fdset);
    FD_SET(s, &fdset);

    check_timer();

    status = select(s + 1, &fdset, NULL, NULL, &select_timeout);
    if (status < 0) {
      dprnt("%s - select: %s", strerror(errno));
      continue;
    }
    if (status == 1)
      receive_ra_packet();

    loop_delay.tv_sec = 2;
    loop_delay.tv_usec = 0;
    select(0, 0, 0, 0, &loop_delay);
  }
  /* NOTREACHED */
}

void
ipv6_start_routing_thread(void)
{
  static int started = 0;
  
  started++;

  if (started == 1) {
  
    cyg_thread_create(
        CYGINT_NET_IPV6_ROUTING_THREAD_PRIORITY, // Priority
        cyg_rs,                                  // entry
        0,                                       // parameter
        "IPv6 routing",                          // Name
        &rs_stack[0],                            // Stack
        STACK_SIZE,                              // Size
        &rs_thread_handle,                       // Handle
        &rs_thread_data                          // Thread data structure
        );
    cyg_thread_resume(rs_thread_handle);         // Start it
    diag_printf("IPv6 routing thread started\n");
  }
}


