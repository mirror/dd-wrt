/* BGP open message handling
 * Copyright (C) 1998, 1999 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#include <zebra.h>

#include "linklist.h"
#include "prefix.h"
#include "stream.h"
#include "thread.h"
#include "log.h"
#include "command.h"

#include "bgpd/bgpd.h"
#include "bgpd/bgp_attr.h"
#include "bgpd/bgp_debug.h"
#include "bgpd/bgp_fsm.h"
#include "bgpd/bgp_packet.h"
#include "bgpd/bgp_open.h"

/* BGP-4 Multiprotocol Extentions lead us to the complex world. We can
   negotiate remote peer supports extentions or not. But if
   remote-peer doesn't supports negotiation process itself.  We would
   like to do manual configuration.

   So there is many configurable point.  First of all we want set each
   peer whether we send capability negotiation to the peer or not.
   Next, if we send capability to the peer we want to set my capabilty
   inforation at each peer. */

/* MP Capability information. */
struct capability_mp
{
  u_int16_t afi;
  u_char reserved;
  u_char safi;
};

/* BGP open message capability. */
struct capability 
{
  u_char code;
  u_char length;
  struct capability_mp mpc;
};

/* For debug purpose. */
void
bgp_capability_mp_log (struct peer *peer, struct capability *cap, char *direct)
{
  if (BGP_DEBUG (normal, NORMAL))
    {
      zlog_info ("%s OPEN has CAPABILITY code: %u, length %u",
		 peer->host, cap->code, cap->length);
      zlog_info ("%s OPEN has MP_EXT CAP for afi/safi: %u/%u",
		 peer->host, ntohs(cap->mpc.afi) , cap->mpc.safi);
    }
}

void
bgp_capability_vty_out (struct vty *vty, struct peer *peer)
{
  u_char *pnt;
  u_char *end;
  struct capability cap;

  pnt = peer->notify.data;
  end = pnt + peer->notify.length;

  while (pnt < end)
    {
      memcpy(&cap, pnt, sizeof(struct capability));

      if (pnt + 2 > end)
	return;
      if (pnt + (cap.length + 2) > end)
	return;

      if (cap.code == CAPABILITY_CODE_MP)
	{
	  vty_out (vty, "  Capability error for: Multi protocol ");

	  switch (ntohs (cap.mpc.afi))
	    {
	    case AFI_IP:
	      vty_out (vty, "AFI IPv4, ");
	      break;
	    case AFI_IP6:
	      vty_out (vty, "AFI IPv6, ");
	      break;
	    default:
	      vty_out (vty, "AFI Unknown %d, ", ntohs (cap.mpc.afi));
	      break;
	    }
	  switch (cap.mpc.safi)
	    {
	    case SAFI_UNICAST:
	      vty_out (vty, "SAFI Unicast");
	      break;
	    case SAFI_MULTICAST:
	      vty_out (vty, "SAFI Multicast");
	      break;
	    case SAFI_UNICAST_MULTICAST:
	      vty_out (vty, "SAFI Unicast Multicast");
	      break;
	    case BGP_SAFI_VPNV4:
	      vty_out (vty, "SAFI MPLS-VPN");
	      break;
	    default:
	      vty_out (vty, "SAFI Unknown %d ", cap.mpc.safi);
	      break;
	    }
	  vty_out (vty, "%s", VTY_NEWLINE);
	}
      else if (cap.code >= 128)
	vty_out (vty, "  Capability error: vendor specific capability code %d",
		 cap.code);
      else
	vty_out (vty, "  Capability error: unknown capability code %d", 
		 cap.code);

      pnt += cap.length + 2;
    }
}

/* Set negotiated capability value. */
int
bgp_capability_mp (struct peer *peer, struct capability *cap)
{
  if (ntohs (cap->mpc.afi) == AFI_IP)
    {
      if (cap->mpc.safi == SAFI_UNICAST)
	{
	  peer->afc_recv[AFI_IP][SAFI_UNICAST] = 1;

	  if (peer->afc[AFI_IP][SAFI_UNICAST])
	    peer->afc_nego[AFI_IP][SAFI_UNICAST] = 1;
	  else
	    return -1;
	}
      else if (cap->mpc.safi == SAFI_MULTICAST) 
	{
	  peer->afc_recv[AFI_IP][SAFI_MULTICAST] = 1;

	  if (peer->afc[AFI_IP][SAFI_MULTICAST])
	    peer->afc_nego[AFI_IP][SAFI_MULTICAST] = 1;
	  else
	    return -1;
	}
      else if (cap->mpc.safi == BGP_SAFI_VPNV4)
	{
	  peer->afc_recv[AFI_IP][SAFI_MPLS_VPN] = 1;

	  if (peer->afc[AFI_IP][SAFI_MPLS_VPN])
	    peer->afc_nego[AFI_IP][SAFI_MPLS_VPN] = 1;
	  else
	    return -1;
	}
      else
	return -1;
    }
#ifdef HAVE_IPV6
  else if (ntohs (cap->mpc.afi) == AFI_IP6)
    {
      if (cap->mpc.safi == SAFI_UNICAST)
	{
	  peer->afc_recv[AFI_IP6][SAFI_UNICAST] = 1;

	  if (peer->afc[AFI_IP6][SAFI_UNICAST])
	    peer->afc_nego[AFI_IP6][SAFI_UNICAST] = 1;
	  else
	    return -1;
	}
      else if (cap->mpc.safi == SAFI_MULTICAST)
	{
	  peer->afc_recv[AFI_IP6][SAFI_MULTICAST] = 1;

	  if (peer->afc[AFI_IP6][SAFI_MULTICAST])
	    peer->afc_nego[AFI_IP6][SAFI_MULTICAST] = 1;
	  else
	    return -1;
	}
      else
	return -1;
    }
#endif /* HAVE_IPV6 */
  else
    {
      /* Unknown Address Family. */
      return -1;
    }

  return 0;
}

/* Parse given capability. */
int
bgp_capability_parse (struct peer *peer, u_char *pnt, u_char length,
		      u_char **error)
{
  int ret;
  u_char *end;
  struct capability cap;

  end = pnt + length;

  while (pnt < end)
    {
      /* Fetch structure to the byte stream. */
      memcpy(&cap, pnt, sizeof(struct capability));

      /* We need at least capability code and capability length. */
      if (pnt + 2 > end)
	{
	  zlog_info ("%s Capability length error", peer->host);
	  bgp_notify_send (peer, BGP_NOTIFY_CEASE, 0);
	  return -1;
	}

      /* Capability length check. */
      if (pnt + (cap.length + 2) > end)
	{
	  zlog_info ("%s Capability length error", peer->host);
	  bgp_notify_send (peer, BGP_NOTIFY_CEASE, 0);
	  return -1;
	}

      /* We know MP Capability Code. */
      if (cap.code == CAPABILITY_CODE_MP)
	{
	  /* For debug purpose. */
	  bgp_capability_mp_log (peer, &cap, "RECV");

	  /* Ignore capability when override-capability is set. */
	  if (! CHECK_FLAG (peer->flags, PEER_FLAG_OVERRIDE_CAPABILITY))
	    {
	      /* Set negotiated value. */
	      ret = bgp_capability_mp (peer, &cap);

	      /* Unsupported Capability. */
	      if (ret < 0)
		{
		  /* Store return data. */
		  memcpy (*error, &cap, cap.length + 2);
		  *error += cap.length + 2;
		}
	    }
	}
      else if (cap.code == CAPABILITY_CODE_REFRESH ||
	       cap.code == CAPABILITY_CODE_REFRESH_01)
	{
	  if (BGP_DEBUG (normal, NORMAL))
	    zlog_info ("%s OPEN has CAPABILITY code: %d, length %d",
		       peer->host, cap.code, cap.length);

	  /* Check length. */
	  if (cap.length != 0)
	    {
	      zlog_info ("%s Route Refresh Capability length error %d",
			 peer->host, cap.length);
	      bgp_notify_send (peer, BGP_NOTIFY_CEASE, 0);
	      return -1;
	    }

	  if (BGP_DEBUG (normal, NORMAL))
	    zlog_info ("%s OPEN has ROUTE-REFRESH capability(%s) for all address-families",
		       peer->host,
		       cap.code == CAPABILITY_CODE_REFRESH ? "old" : "new");

	  /* BGP refresh capability */
	  if (cap.code == CAPABILITY_CODE_REFRESH)
	    peer->refresh_nego_old = 1;
	  else
	    peer->refresh_nego_new = 1;
	}
      else if (cap.code > 128)
	{
	  /* We ignore sending Nofify's for vendor specific
	     capabilities. Seems reasonable for now...  */
	  zlog_warn ("%s Vendor specific capability %d",
		     peer->host, cap.code);
	}
      else
	{
	  zlog_warn ("%s unrecognized capability code: %d - ignored",
		     peer->host, cap.code);
	  memcpy (*error, &cap, cap.length + 2);
	  *error += cap.length + 2;
	}

      pnt += cap.length + 2;
    }
  return 0;
}

int
bgp_auth_parse (struct peer *peer, u_char *pnt, size_t length)
{
  bgp_notify_send (peer, 
		   BGP_NOTIFY_OPEN_ERR, 
		   BGP_NOTIFY_OPEN_AUTH_FAILURE); 
  return -1;
}

int
strict_capability_same (struct peer *peer)
{
  int i, j;

  for (i = AFI_IP; i < AFI_MAX; i++)
    for (j = SAFI_UNICAST; j < SAFI_MAX; j++)
      if (peer->afc[i][j] != peer->afc_nego[i][j])
	return 0;
  return 1;
}

/* Parse open option */
int
bgp_open_option_parse (struct peer *peer, u_char length, int *capability)
{
  int ret;
  u_char *end;
  u_char opt_type;
  u_char opt_length;
  u_char *pnt;
  u_char *error;
  u_char error_data[BGP_MAX_PACKET_SIZE];

  /* Fetch pointer. */
  pnt = stream_pnt (peer->ibuf);

  ret = 0;
  opt_type = 0;
  opt_length = 0;
  end = pnt + length;
  error = error_data;

  if (BGP_DEBUG (normal, NORMAL))
    zlog_info ("%s rcv OPEN w/ OPTION parameter len: %u",
	       peer->host, length);
  
  while (pnt < end) 
    {
      /* Check the length. */
      if (pnt + 2 > end)
	{
	  zlog_info ("%s Option length error", peer->host);
	  bgp_notify_send (peer, BGP_NOTIFY_CEASE, 0);
	  return -1;
	}

      /* Fetch option type and length. */
      opt_type = *pnt++;
      opt_length = *pnt++;
      
      /* Option length check. */
      if (pnt + opt_length > end)
	{
	  zlog_info ("%s Option length error", peer->host);
	  bgp_notify_send (peer, BGP_NOTIFY_CEASE, 0);
	  return -1;
	}

      if (BGP_DEBUG (normal, NORMAL))
	zlog_info ("%s rcvd OPEN w/ optional parameter type %u (%s) len %u",
		   peer->host, opt_type,
		   opt_type == BGP_OPEN_OPT_AUTH ? "Authentication" :
		   opt_type == BGP_OPEN_OPT_CAP ? "Capability" : "Unknown",
		   opt_length);
  
      switch (opt_type)
	{
	case BGP_OPEN_OPT_AUTH:
	  ret = bgp_auth_parse (peer, pnt, opt_length);
	  break;
	case BGP_OPEN_OPT_CAP:
	  ret = bgp_capability_parse (peer, pnt, opt_length, &error);
	  *capability = 1;
	  break;
	default:
	  bgp_notify_send (peer, 
			   BGP_NOTIFY_OPEN_ERR, 
			   BGP_NOTIFY_OPEN_UNSUP_PARAM); 
	  ret = -1;
	  break;
	}

      /* Parse error.  To accumulate all unsupported capability codes,
         bgp_capability_parse does not return -1 when encounter
         unsupported capability code.  To detect that, please check
         error and erro_data pointer, like below.  */
      if (ret < 0)
	return -1;

      /* Forward pointer. */
      pnt += opt_length;
    }

  /* All OPEN option is parsed.  Check capability when strict compare
     flag is enabled.*/
  if (CHECK_FLAG (peer->flags, PEER_FLAG_STRICT_CAP_MATCH))
    {
      /* If Unsupported Capability exists. */
      if (error != error_data)
	{
	  bgp_notify_send_with_data (peer, 
				     BGP_NOTIFY_OPEN_ERR, 
				     BGP_NOTIFY_OPEN_UNSUP_CAPBL, 
				     error_data, error - error_data);
	  return -1;
	}

      /* Check local capability does not negotiated with remote
         peer. */
      if (! strict_capability_same (peer))
	{
	  bgp_notify_send (peer, 
			   BGP_NOTIFY_OPEN_ERR, 
			   BGP_NOTIFY_OPEN_UNSUP_CAPBL);
	  return -1;
	}
    }

  /* Check there is no common capability send Unsupported Capability
     error. */
  if (*capability && ! CHECK_FLAG (peer->flags, PEER_FLAG_OVERRIDE_CAPABILITY))
    {
      if (! peer->afc_nego[AFI_IP][SAFI_UNICAST] 
	  && ! peer->afc_nego[AFI_IP][SAFI_MULTICAST]
	  && ! peer->afc_nego[AFI_IP][SAFI_MPLS_VPN]
	  && ! peer->afc_nego[AFI_IP6][SAFI_UNICAST]
	  && ! peer->afc_nego[AFI_IP6][SAFI_MULTICAST])
	{
	  plog_err (peer->log, "%s [Error] No common capability", peer->host);

	  if (error != error_data)

	    bgp_notify_send_with_data (peer, 
				       BGP_NOTIFY_OPEN_ERR, 
				       BGP_NOTIFY_OPEN_UNSUP_CAPBL, 
				       error_data, error - error_data);
	  else
	    bgp_notify_send (peer, 
			     BGP_NOTIFY_OPEN_ERR, 
			     BGP_NOTIFY_OPEN_UNSUP_CAPBL);
	  return -1;
	}
    }
  return 0;
}

/* Fill in capability open option to the packet. */
void
bgp_open_capability (struct stream *s, struct peer *peer)
{
  u_char len;
  unsigned long cp;

  /* Remember current pointer for Opt Parm Len. */
  cp = stream_get_putp (s);

  /* Opt Parm Len. */
  stream_putc (s, 0);

  /* Do not send capability. */
  if (! CHECK_FLAG (peer->sflags, PEER_STATUS_CAPABILITY_OPEN) 
      || CHECK_FLAG (peer->flags, PEER_FLAG_DONT_CAPABILITY))
    return;
    
  /* When the peer is IPv4 unicast only, do not send capability. */
  if (! peer->afc[AFI_IP][SAFI_MULTICAST] 
      && ! peer->afc[AFI_IP][SAFI_MPLS_VPN]
      && ! peer->afc[AFI_IP6][SAFI_UNICAST] 
      && ! peer->afc[AFI_IP6][SAFI_MULTICAST]
      && ! CHECK_FLAG (peer->flags, PEER_FLAG_CAPABILITY_ROUTE_REFRESH))
    return;

  /* IPv4 unicast. */
  if (peer->afc[AFI_IP][SAFI_UNICAST])
    {
      peer->afc_adv[AFI_IP][SAFI_UNICAST] = 1;
      stream_putc (s, BGP_OPEN_OPT_CAP);
      stream_putc (s, CAPABILITY_CODE_MP_LEN + 2);
      stream_putc (s, CAPABILITY_CODE_MP);
      stream_putc (s, CAPABILITY_CODE_MP_LEN);
      stream_putw (s, AFI_IP);
      stream_putc (s, 0);
      stream_putc (s, SAFI_UNICAST);
    }
  /* IPv4 multicast. */
  if (peer->afc[AFI_IP][SAFI_MULTICAST])
    {
      peer->afc_adv[AFI_IP][SAFI_MULTICAST] = 1;
      stream_putc (s, BGP_OPEN_OPT_CAP);
      stream_putc (s, CAPABILITY_CODE_MP_LEN + 2);
      stream_putc (s, CAPABILITY_CODE_MP);
      stream_putc (s, CAPABILITY_CODE_MP_LEN);
      stream_putw (s, AFI_IP);
      stream_putc (s, 0);
      stream_putc (s, SAFI_MULTICAST);
    }
  /* IPv4 VPN */
  if (peer->afc[AFI_IP][SAFI_MPLS_VPN])
    {
      peer->afc_adv[AFI_IP][SAFI_MPLS_VPN] = 1;
      stream_putc (s, BGP_OPEN_OPT_CAP);
      stream_putc (s, CAPABILITY_CODE_MP_LEN + 2);
      stream_putc (s, CAPABILITY_CODE_MP);
      stream_putc (s, CAPABILITY_CODE_MP_LEN);
      stream_putw (s, AFI_IP);
      stream_putc (s, 0);
      stream_putc (s, BGP_SAFI_VPNV4);
    }
#ifdef HAVE_IPV6
  /* IPv6 unicast. */
  if (peer->afc[AFI_IP6][SAFI_UNICAST])
    {
      peer->afc_adv[AFI_IP6][SAFI_UNICAST] = 1;
      stream_putc (s, BGP_OPEN_OPT_CAP);
      stream_putc (s, CAPABILITY_CODE_MP_LEN + 2);
      stream_putc (s, CAPABILITY_CODE_MP);
      stream_putc (s, CAPABILITY_CODE_MP_LEN);
      stream_putw (s, AFI_IP6);
      stream_putc (s, 0);
      stream_putc (s, SAFI_UNICAST);
    }
  /* IPv6 multicast. */
  if (peer->afc[AFI_IP6][SAFI_MULTICAST])
    {
      peer->afc_adv[AFI_IP6][SAFI_MULTICAST] = 1;
      stream_putc (s, BGP_OPEN_OPT_CAP);
      stream_putc (s, CAPABILITY_CODE_MP_LEN + 2);
      stream_putc (s, CAPABILITY_CODE_MP);
      stream_putc (s, CAPABILITY_CODE_MP_LEN);
      stream_putw (s, AFI_IP6);
      stream_putc (s, 0);
      stream_putc (s, SAFI_MULTICAST);
    }
#endif /* HAVE_IPV6 */

  /* Route refresh. (OLD) */
  if (CHECK_FLAG (peer->flags, PEER_FLAG_CAPABILITY_ROUTE_REFRESH))
    {
      peer->refresh_adv = 1;
      stream_putc (s, BGP_OPEN_OPT_CAP);
      stream_putc (s, CAPABILITY_CODE_REFRESH_LEN + 2);
      stream_putc (s, CAPABILITY_CODE_REFRESH);
      stream_putc (s, CAPABILITY_CODE_REFRESH_LEN);
    }
  
  /* Route refresh. (NEW) */
  if (CHECK_FLAG (peer->flags, PEER_FLAG_CAPABILITY_ROUTE_REFRESH))
    {
      peer->refresh_adv = 1;
      stream_putc (s, BGP_OPEN_OPT_CAP);
      stream_putc (s, CAPABILITY_CODE_REFRESH_LEN + 2);
      stream_putc (s, CAPABILITY_CODE_REFRESH_01);
      stream_putc (s, CAPABILITY_CODE_REFRESH_LEN);
    }

  /* Total Opt Parm Len. */
  len = stream_get_putp (s) - cp - 1;
  stream_putc_at (s, cp, len);
}
