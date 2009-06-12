//=============================================================================
//
//      sntp.c
//
//      Simple Network Time Protocol
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Andrew Lunn
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   andrew.lunn
// Contributors:
// Date:        2003-02-12
// Description: Provides a Simple Network Time Protocol Client
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/system.h>
#include <pkgconf/net_sntp.h>
#include <network.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/cyg_trac.h>
#include <cyg/sntp/sntp.h>
#include <time.h>

/* NTP/SNTPv4 Packet Format (RFC2030) */
typedef struct
{
  cyg_uint32    Seconds;    /* Since 00:00:00 Jan 01 1900 */
  cyg_uint32    Fraction;
} NTP_TIMESTAMP;

typedef struct
{
  /* Control combines LeapIndicator, Version, and Mode */
  cyg_uint8     Control;
  cyg_uint8     Stratum;
  cyg_uint8     Poll;
  cyg_uint8     Precision;

  cyg_uint32    RootDelay;
  cyg_uint32    RootDispersion;
  cyg_uint32    ReferenceIdentifier;

  NTP_TIMESTAMP ReferenceTimestamp;
  NTP_TIMESTAMP OriginateTimestamp;
  NTP_TIMESTAMP ReceiveTimestamp;
  NTP_TIMESTAMP TransmitTimestamp;

  cyg_uint32    KeyIdentifier;          /* Optional */
  cyg_uint8     MessageDigest[16];      /* Optional */
} NTP_PACKET;
#define NTP_PACKET_MINLEN       48      /* Packet size - optional fields */

/* Leap Indicator Field [Bits 7:6] */
#define NTP_LI_NOLEAP           0x00
#define NTP_LI_61SECS           0x40
#define NTP_LI_59SECS           0x80
#define NTP_LI_ALARM            0xC0

/* Version Field [Bits 5:3] */
#define NTP_VERSION_GET(pkt)    ((((pkt)->Control)>>3)&0x7)
#define NTP_VERSION_SET(ver)    (((ver)&0x7)<<3)

/* Mode Field [Bits 2:0] */
#define NTP_MODE_RESERVED       0
#define NTP_MODE_SYMACTIVE      1    /* Symmetric Active */
#define NTP_MODE_SYMPASSIVE     2    /* Symmetric Passive */
#define NTP_MODE_CLIENT         3
#define NTP_MODE_SERVER         4
#define NTP_MODE_BROADCAST      5
#define NTP_MODE_NTPCTRL        6    /* Reserved for NTP control message */
#define NTP_MODE_PRIVATE        7    /* Reserved for private use */
#define NTP_MODE_GET(pkt)       (((pkt)->Control)&0x7)
#define NTP_MODE_SET(mode)      ((mode)&0x7)

/* Time Base Conversion Macros
 *
 * The NTP timebase is 00:00 Jan 1 1900.  The local
 * time base is 00:00 Jan 1 1970.  Convert between
 * these two by added or substracting 70 years
 * worth of time.  Note that 17 of these years were
 * leap years.
 */
#define TIME_BASEDIFF           ((((cyg_uint32)70*365 + 17) * 24*3600))
#define TIME_NTP_TO_LOCAL(t)	((t)-TIME_BASEDIFF)
#define TIME_LOCAL_TO_NTP(t)	((t)+TIME_BASEDIFF)


struct sntp_srv_s {
  struct sockaddr addr;
  int stratum;
  int version;
  cyg_uint32 timestamp;
};
static int sntp_initialized = 0;


#ifdef CYGPKG_NET_SNTP_UNICAST
/* When using SNTP unicast mode, sntp_servers
 * points to an array of char pointers that
 * specify NTP server addresses to send
 * requests to.  sntp_num_servers specifies
 * the number of hostnames in the array.
 */
static struct sockaddr *sntp_servers = NULL;
static cyg_uint32 sntp_num_servers = 0;
static cyg_mutex_t sntp_mutex;
static time_t NextTimeUpdate = 0;

/* SNTP Timeouts
 *
 * SNTP_WAITPERIOD is the number of seconds to wait
 * before retransmitting unanswered NTP requests
 * whenever we are due for an update.
 *
 * SNTP_UPDATEPERIOD is the number of seconds to wait
 * after we get a good time update before we feel
 * like we should re-synchronize again with the
 * time server.
 */
#define SNTP_WAITPERIOD         10      /* Wait period in seconds */
#define SNTP_UPDATEPERIOD       (30*60) /* Update period in seconds */

#endif /* CYKPKG_NET_SNTP_UNICAST */

#ifndef	CYGNUM_SNTP_STACK_SIZE
/* Use a stack size of at least CYGNUM_SNTP_STACK_SIZE_MIN, but not less than
 * CYGNUM_HAL_STACK_SIZE_TYPICAL.
 */
#define	CYGNUM_SNTP_STACK_SIZE_MIN	4096
#if (CYGNUM_HAL_STACK_SIZE_TYPICAL < CYGNUM_SNTP_STACK_SIZE_MIN)
#define	CYGNUM_SNTP_STACK_SIZE	CYGNUM_SNTP_STACK_SIZE_MIN
#else
#define	CYGNUM_SNTP_STACK_SIZE	CYGNUM_HAL_STACK_SIZE_TYPICAL
#endif
#endif	/* CYGNUM_SNTP_STACK_SIZE */

/* Is the new server better than the current one? If its the same as
   the current, its always better. If the stratum is lower its better.
   If we have not heard from the old server for more than 10 minutes,
   the new server is better. */

static int is_better(struct sntp_srv_s *newer, struct sntp_srv_s *old) {
   
  time_t last_time, diff;
  
  if (!memcmp(&newer->addr, &old->addr, newer->addr.sa_len)) return 1;
  if (newer->stratum < old->stratum) return 1;

  if (old->timestamp != 0xffffffff) {
    last_time = TIME_NTP_TO_LOCAL(old->timestamp);
  
    diff = time(NULL) - last_time;
    if (diff > 600) return 1;
    
    return 0;
  }
  return 1;
}

const struct in6_addr in6addr_ntp_multicast = IN6ADDR_NTP_MULTICAST;

static void sntp_fn(cyg_addrword_t data)
{
  int fd;
  int ret;
  struct sockaddr_in local;
  struct servent *serv;
  NTP_PACKET ntp_pkt;
  struct sntp_srv_s new_srv;
  struct sntp_srv_s best_srv;
  int mode;
  int len;
  time_t new_time, current_time, diff;
  fd_set readfds;
  int n;
#ifdef CYGPKG_NET_INET6
  int fd6 = -1;
  struct ipv6_mreq mreq;
  struct sockaddr_in6 local6;
#endif
#ifdef CYGPKG_NET_SNTP_UNICAST
  int i;
  struct timeval timeout;
#endif /* CYGPKG_NET_SNTP_UNICAST */
  struct timeval *ptimeout = NULL;

  memset(&best_srv,0xff,sizeof(best_srv));

  fd = socket(AF_INET,SOCK_DGRAM,0);
  CYG_ASSERT(-1 != fd,"Failed to open socket");

  serv = getservbyname("ntp","udp");
  CYG_ASSERT(serv,"getservbyname(sntp,udp)");

  memset(&local,0,sizeof(local));
  local.sin_family = AF_INET;
  local.sin_len = sizeof(local);
  local.sin_port = serv->s_port;
  local.sin_addr.s_addr = INADDR_ANY;

  ret=bind(fd,(struct sockaddr *)&local,sizeof(local));
  CYG_ASSERT(0 == ret, "Bind failed");

  n = fd;

#ifdef CYGPKG_NET_INET6
  fd6 = socket(AF_INET6, SOCK_DGRAM,0);
  CYG_ASSERT(-1 != fd,"Failed to open socket");
  mreq.ipv6mr_multiaddr = in6addr_ntp_multicast;
  mreq.ipv6mr_interface = 0;

  /* Join the well-known NTP multicast groups.  We will
   * try to join the link-local, site-local, and global
   * unicast groups.
   *
   * Note that we skip the node-local group since it
   * doesn't make any sense to join that group.  If we
   * had an NTP server running on the local node, we
   * wouldn't need to have an NTP client get the time
   * from it!
   */
#ifdef CYGHWR_NET_DRIVER_ETH0
  // Link-Local
  mreq.ipv6mr_multiaddr.s6_addr[1]=0x02;
  mreq.ipv6mr_interface = if_nametoindex("eth0");
  if (mreq.ipv6mr_interface != 0 ) {
    ret = setsockopt(fd6, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq, sizeof(mreq));
    CYG_ASSERT(0 == ret, "setsockopt(IPV6_JOIN_GROUP) Link-Local eth0");
  }
#endif
#ifdef CYGHWR_NET_DRIVER_ETH1
  // Link-Local
  mreq.ipv6mr_multiaddr.s6_addr[1]=0x02;
  mreq.ipv6mr_interface = if_nametoindex("eth1");
  if (mreq.ipv6mr_interface != 0 ) {
    ret = setsockopt(fd6, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq, sizeof(mreq));
    CYG_ASSERT(0 == ret, "setsockopt(IPV6_JOIN_GROUP) Link-Local eth1");
  }
#endif

  // Site-Local
  mreq.ipv6mr_multiaddr.s6_addr[1]=0x05;
  ret = setsockopt(fd6, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq, sizeof(mreq));
  CYG_ASSERT(0 == ret, "setsockopt(IPV6_JOIN_GROUP) Site-Local");

  // Global
  mreq.ipv6mr_multiaddr.s6_addr[1]=0x0e;
  ret = setsockopt(fd6, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq, sizeof(mreq));
  CYG_ASSERT(0 == ret, "setsockopt(IPV6_JOIN_GROUP) Global");
  
  memset(&local6,0,sizeof(local6));
  local6.sin6_family = AF_INET6;
  local6.sin6_len = sizeof(local6);
  local6.sin6_port = serv->s_port;
  local6.sin6_addr = in6addr_any;
  
  ret = bind(fd6, (struct sockaddr *)&local6,sizeof(local6));
  CYG_ASSERT(0 == ret, "Bind6 failed");
  
  n = (n > fd6 ? n : fd6);
#endif

  while (1) {
    FD_ZERO(&readfds);
    FD_SET(fd,&readfds);
#ifdef CYGPKG_NET_INET6
    FD_SET(fd6,&readfds);
#endif
    
#ifdef CYGPKG_NET_SNTP_UNICAST
    /* By default, we will let select() wait
     * for SNTP_WAITPERIOD to receive a packet.  This
     * allows us to periodically wake up and check
     * if new servers have been configured.  However,
     * if we are waiting to send an update request,
     * we will set ptimeout to something more
     * reasonable below.
     */
     timeout.tv_sec = SNTP_WAITPERIOD;
     timeout.tv_usec = 0;
     ptimeout = &timeout;

    /* If we've already set the time, then
     * check to see if it's time to try and
     * update it.
     */
    if (NextTimeUpdate != 0)
    {
        current_time = time(NULL);
        if (current_time < NextTimeUpdate)
        {
            /* Set the select() timeout to wake us
             * up when it's time to send more
             * requests.
             */
            timeout.tv_sec = (SNTP_WAITPERIOD > (NextTimeUpdate - current_time)?
							 (NextTimeUpdate - current_time):SNTP_WAITPERIOD);
        } else {
            /* It's already time for us to update our time */
			NextTimeUpdate = 0;
        }
    }

    /* If we need to update our time and we have
     * a list of NTP servers, then send out some
     * time requests.
     */
    if (NextTimeUpdate == 0 && (sntp_num_servers > 0))
    {
        /* Send an NTP request to each NTP server
         * in our server list.  Use version 3
         * for v3 and v4 compatibility.
         */
        memset(&ntp_pkt, 0, sizeof(ntp_pkt));
        ntp_pkt.Control =
			NTP_LI_NOLEAP |
			NTP_MODE_SET(NTP_MODE_CLIENT) |
			NTP_VERSION_SET(3);

        /* Send a request packet to each of our
         * configured servers.
         */
        cyg_mutex_lock(&sntp_mutex);
        for (i = 0; i < sntp_num_servers; i++)
        {
    		/* Send the request packet using the
    		 * appropriate protocol.
    		 */
            ntp_pkt.TransmitTimestamp.Seconds =
				htonl(TIME_LOCAL_TO_NTP(time(NULL)));
            if (sntp_servers[i].sa_family == AF_INET)
            {
                sendto(fd, &ntp_pkt, sizeof(ntp_pkt), 0,
                    &sntp_servers[i], sntp_servers[i].sa_len);
#ifdef CYGPKG_NET_INET6
            } else if (sntp_servers[i].sa_family == AF_INET6) {
                sendto(fd6, &ntp_pkt, sizeof(ntp_pkt), 0,
                    &sntp_servers[i], sntp_servers[i].sa_len);
#endif
            }
    	}
        cyg_mutex_unlock(&sntp_mutex);

        /* Set the NextTimeUpdate so that we don't
         * send any more requests until the next
         * poll period.  And we've already configured
         * the select() timeout above to wait for
         * replies.
         */
        NextTimeUpdate = time(NULL) + SNTP_WAITPERIOD;
    }
#endif /* CYGPKG_NET_SNTP_UNICAST */

    ret = select(n+1, &readfds, NULL, NULL, ptimeout);
    CYG_ASSERT(-1 != ret, "Select");

#ifdef CYGPKG_NET_SNTP_UNICAST
    /* If we timed out, then try resending requests */
    if (ret == 0)
        continue;
#endif /* CYGPKG_NET_SNTP_UNICAST */

    len = sizeof(new_srv.addr);
    if (FD_ISSET(fd,&readfds)) {
      ret=recvfrom(fd,&ntp_pkt,sizeof(ntp_pkt),0,(struct sockaddr *)&new_srv.addr,&len);
    }
#ifdef CYGPKG_NET_INET6
    if (FD_ISSET(fd6,&readfds)) {
      ret=recvfrom(fd6,&ntp_pkt,sizeof(ntp_pkt),0,(struct sockaddr *)&new_srv.addr,&len);
    }
#endif
    CYG_ASSERT(0 < ret,"recvfrom");

    /* We expect at least enough bytes to fill the buffer */
    if (ret < NTP_PACKET_MINLEN)
      continue;
    
    new_srv.version = NTP_VERSION_GET(&ntp_pkt);
    new_srv.stratum = ntp_pkt.Stratum;
    new_srv.timestamp = ntohl(ntp_pkt.TransmitTimestamp.Seconds);
    mode = NTP_MODE_GET(&ntp_pkt);
    
    /* Only support protocol versions 3 or 4 */
    if (new_srv.version < 3 || new_srv.version > 4) {
      CYG_TRACE1(1, "Unsupported version of NTP. Version %d",new_srv.version);
      continue;
    }
    
    /* Only process broadcast and server packets */
    if (mode != NTP_MODE_BROADCAST && mode != NTP_MODE_SERVER) 
      continue;
    
    /* Is the packet from a better server than our current one */
    if (is_better(&new_srv,&best_srv)) {
      best_srv = new_srv;

      /* Work out the difference between server and our time.
       * TODO: Implement RFC2030 recommendations for
       * calculating propagation delay between the client
       * and server.
       */
      new_time = TIME_NTP_TO_LOCAL(best_srv.timestamp);
      current_time = time(NULL);
      diff = current_time - new_time;
      
      if (diff < 0) 
          diff = -diff;
      
      if (diff > 2) 
          cyg_libc_time_settime(new_time);
    }
#ifdef CYGPKG_NET_SNTP_UNICAST
    NextTimeUpdate = time(NULL) + SNTP_UPDATEPERIOD;
#endif
  }
}

/* Start the SNTP server */
void cyg_sntp_start(void) {
  
  static char sntp_stack[CYGNUM_SNTP_STACK_SIZE];
  static cyg_thread sntp_thread_data;
  static cyg_handle_t sntp_handle;

  /* Only initialize things once */
  if (sntp_initialized)
      return;
  sntp_initialized = 1;

#ifdef CYGPKG_NET_SNTP_UNICAST
  /* Initialize the SNTP mutex */
  cyg_mutex_init(&sntp_mutex);
#endif

  cyg_thread_create(CYGPKG_NET_THREAD_PRIORITY+1, 
		    sntp_fn,               // entry
		    0,                     // entry parameter
		    "SNTP Client",         // Name
		    &sntp_stack,           // stack
		    sizeof(sntp_stack),    // stack size
		    &sntp_handle,          // Handle
		    &sntp_thread_data);    // Thread data structure

  cyg_thread_resume(sntp_handle);
}

#ifdef CYGPKG_NET_SNTP_UNICAST
/*
 *	FUNCTION cyg_sntp_set_servers
 *
 *	DESCRIPTION
 *		Sets the list of SNTP/NTP servers to use
 *		for SNTP unicast requests.  The list is
 *		specified as a list of sockaddr structures
 *		and can contain both IPv4 and IPv6
 *		addresses and UDP port numbers.
 *
 *		The server_list array must be maintained
 * 		by the caller and must not be modified after
 *		it is registered by this function.  The
 *		array can be unregistered by calling this
 *		function again with different parameters.
 *
 *      NOTE: If cyg_sntp_start() has not been called
 * 		already, and this function is called with a
 *		list of 1 or more servers, then cyg_sntp_start()
 *		will be called by this function to start the client.
 *
 *	PARAMETERS
 *		server_list - Array of IPv4 and/or IPv6 sockaddr's
 *		num_servers - Number of sockaddr's in array (0 to disable)
 *
 *	RETURN VALUES
 *		None
 */
void cyg_sntp_set_servers(struct sockaddr *server_list,
        cyg_uint32 num_servers)
{
	/* If we haven't already started the SNTP client, then
	 * start it now.
	 */
    if (!sntp_initialized)
	{
		/* If we haven't started already and we don't
		 * have a list of servers, then don't start
		 * anything up.
		 */
		if (num_servers == 0)
			return;
		cyg_sntp_start();
	}

    /* Get the server list mutex */
    cyg_mutex_lock(&sntp_mutex);

	/* Record the new server list */
	sntp_num_servers = num_servers;
	if (num_servers == 0) {
		server_list = NULL;
	} else {
		/* reset the waiting time to force a new update <= SNTP_WAITPERIOD*/
		NextTimeUpdate = 0;
	}
	sntp_servers = server_list;

	/* Free the mutex */
    cyg_mutex_unlock(&sntp_mutex);
}
#endif /* CYGPKG_NET_SNTP_UNICAST */




