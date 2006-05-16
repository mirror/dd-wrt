/*
 * BGP network related fucntions
 * Copyright (C) 1999 Kunihiro Ishiguro
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

#include "thread.h"
#include "sockunion.h"
#include "memory.h"
#include "log.h"
#include "if.h"
#include "prefix.h"
#include "command.h"

#include "bgpd/bgpd.h"
#include "bgpd/bgp_fsm.h"
#include "bgpd/bgp_attr.h"
#include "bgpd/bgp_debug.h"

/* BGP socket bind. */
int
bgp_bind (struct peer *peer)
{
#ifdef SO_BINDTODEVICE
  int ret;
  struct ifreq ifreq;

  if (!peer->ifname)
    return 0;

  strncpy ((char *)&ifreq.ifr_name, peer->ifname, sizeof (ifreq.ifr_name));

  ret = setsockopt (peer->fd, SOL_SOCKET, SO_BINDTODEVICE, 
		    &ifreq, sizeof (ifreq));
  if (ret < 0)
    {
      zlog (peer->log, LOG_INFO, "bind to interface %s failed", peer->ifname);
      return ret;
    }
#endif /* SO_BINDTODEVICE */
  return 0;
}

int
bgp_bind_address (int sock, struct in_addr *addr)
{
  int ret;
  struct sockaddr_in local;

  memset (&local, 0, sizeof (struct sockaddr_in));
  local.sin_family = AF_INET;
#ifdef HAVE_SIN_LEN
  local.sin_len = sizeof(struct sockaddr_in);
#endif /* HAVE_SIN_LEN */
  memcpy (&local.sin_addr, addr, sizeof (struct in_addr));

  ret = bind (sock, (struct sockaddr *)&local, sizeof (struct sockaddr_in));
  if (ret < 0)
    ;
  return 0;
}

struct in_addr *
bgp_update_address (struct interface *ifp)
{
  struct prefix_ipv4 *p;
  struct connected *connected;
  listnode node;

  for (node = listhead (ifp->connected); node; nextnode (node))
    {
      connected = getdata (node);

      p = (struct prefix_ipv4 *) connected->address;

      if (p->family == AF_INET)
	return &p->prefix;
    }
  return NULL;
}

int
bgp_update_source (struct peer *peer)
{
  struct interface *ifp;
  struct in_addr *addr;

  /* Ifname is exist. */
  if (peer->update_if)
    {
      ifp = if_lookup_by_name (peer->update_if);
      if (!ifp)
	return -1;

      addr = bgp_update_address (ifp);
      if (!addr)
	return -1;

      return bgp_bind_address (peer->fd, addr);
    }

  if (peer->update_source)
    return sockunion_bind (peer->fd, peer->update_source, 
			   0, peer->update_source);

  return 0;
}

/* BGP try to connect to the peer.  */
int
bgp_connect (struct peer *peer)
{
  unsigned int ifindex = 0;

  /* Make socket for the peer. */
  peer->fd = sockunion_socket (&peer->su);
  if (peer->fd < 0)
    return -1;

  /* If we can get socket for the peer, adjest TTL and make connection. */
  if (peer_sort (peer) == BGP_PEER_EBGP)
    sockopt_ttl (peer->su.sa.sa_family, peer->fd, peer->ttl);

  sockopt_reuseaddr (peer->fd);
  sockopt_reuseport (peer->fd);

  /* Bind socket. */
  bgp_bind (peer);

  /* Update source bind. */
  bgp_update_source (peer);

#ifdef HAVE_IPV6
  if (peer->ifname)
    ifindex = if_nametoindex (peer->ifname);
#endif /* HAVE_IPV6 */

  if (BGP_DEBUG (events, EVENTS))
    plog_info (peer->log, "%s [Event] Connect start to %s fd %d",
	       peer->host, peer->host, peer->fd);

  /* Connect to the remote peer. */
  return sockunion_connect (peer->fd, &peer->su, htons (peer->port), ifindex);
}

/* Accept bgp connection. */
int
bgp_accept (struct thread *thread)
{
  int bgp_sock;
  int accept_sock;
  union sockunion su;
  struct peer *peer;
  struct peer *peer1;
  char buf[SU_ADDRSTRLEN];

  /* Regiser accept thread. */
  accept_sock = THREAD_FD (thread);

  if (accept_sock < 0)
    {
      zlog_err ("accept_sock is nevative value %d", accept_sock);
      return -1;
    }
  thread_add_read (master, bgp_accept, NULL, accept_sock);

  /* Accept client connection. */
  bgp_sock = sockunion_accept (accept_sock, &su);
  if (bgp_sock < 0)
    {
      zlog_err ("[Error] BGP socket accept failed (%s)", strerror (errno));
      return -1;
    }

  if (BGP_DEBUG (events, EVENTS))
    zlog_info ("[Event] BGP connection from host %s", inet_sutop (&su, buf));
  
  /* Check remote IP address */
  peer1 = peer_lookup_by_su (&su);
  if (! peer1 || peer1->status == Idle)
    {
      if (BGP_DEBUG (events, EVENTS))
	{
	  if (! peer1)
	    zlog_info ("[Event] BGP connection IP address %s is not configured",
		       inet_sutop (&su, buf));
	  else
	    zlog_info ("[Event] BGP connection IP address %s is Idle state",
		       inet_sutop (&su, buf));
	}
      close (bgp_sock);
      return -1;
    }

  /* Make dummy peer until read Open packet. */
  if (BGP_DEBUG (events, EVENTS))
    zlog_info ("[Event] Make dummy peer structure until read Open packet");

  {
    char buf[SU_ADDRSTRLEN + 1];

    peer = peer_create_accept ();
    SET_FLAG (peer->sflags, PEER_STATUS_ACCEPT_PEER);
    peer->su = su;
    peer->fd = bgp_sock;
    peer->status = Active;
    peer->local_id = peer1->local_id;

    /* Make peer's address string. */
    sockunion2str (&su, buf, SU_ADDRSTRLEN);
    peer->host = strdup (buf);
  }

  BGP_EVENT_ADD (peer, TCP_connection_open);

  return 0;
}

#if defined(HAVE_IPV6) && !defined(NRL)
void
bgp_serv_sock_addrinfo (unsigned short port)
{
  int ret;
  struct addrinfo req;
  struct addrinfo *ainfo;
  struct addrinfo *ainfo_save;
  int sock;
  char port_str[BUFSIZ];

  memset (&req, 0, sizeof (struct addrinfo));
  req.ai_flags = AI_PASSIVE;
  req.ai_family = AF_UNSPEC;
  req.ai_socktype = SOCK_STREAM;
  sprintf (port_str, "%d", port);
  port_str[sizeof(port_str)-1] = '\0';

  ret = getaddrinfo (NULL, port_str, &req, &ainfo);

  if (ret != 0)
    {
      fprintf (stderr, "getaddrinfo failed: %s\n", gai_strerror (ret));
      exit (1);
    }

  ainfo_save = ainfo;

  do
    {
      if (ainfo->ai_family != AF_INET
#ifdef HAVE_IPV6
	  && ainfo->ai_family != AF_INET6
#endif /* HAVE_IPV6 */
	  )
	continue;
     
      sock = socket (ainfo->ai_family, ainfo->ai_socktype, ainfo->ai_protocol);
      if (sock < 0)
	continue;

      sockopt_reuseaddr (sock);
      sockopt_reuseport (sock);

      ret = bind (sock, ainfo->ai_addr, ainfo->ai_addrlen);
      if (ret < 0)
	continue;

      ret = listen (sock, 3);
      if (ret < 0) 
	continue;

      if (sock < 0)
	{
	  zlog_err ("bgp_sock_addr_info sock is nevative value %d", sock);
	  continue;
	}
      thread_add_read (master, bgp_accept, NULL, sock);
    }
  while ((ainfo = ainfo->ai_next) != NULL);

  freeaddrinfo (ainfo_save);
}
#endif /* HAVE_IPV6 && !NRL */

/* Make bgpd's server socket. */
void
bgp_serv_sock_family (unsigned short port, int family)
{
  int ret;
  int bgp_sock;
  union sockunion su;

  memset (&su, 0, sizeof (union sockunion));

  /* Specify address family. */
  su.sa.sa_family = family;
  bgp_sock = sockunion_stream_socket (&su);

  sockopt_reuseaddr (bgp_sock);
  sockopt_reuseport (bgp_sock);

  ret = sockunion_bind (bgp_sock, &su, port, NULL);

  ret = listen (bgp_sock, 3);
  if (ret < 0) 
    {
      zlog (NULL, LOG_INFO, "Can't listen bgp server socket : %s",
	    strerror (errno));
      return;
    }
  
  if (bgp_sock < 0)
    {
      zlog_err ("bgp_serv_sock_family bgp_sock is nevative value %d",
		bgp_sock);
      return;
    }
  thread_add_read (master, bgp_accept, NULL, bgp_sock);
}

void
bgp_serv_sock (unsigned short port)
{
#if defined(HAVE_IPV6) && !defined(NRL)
  bgp_serv_sock_addrinfo (port);
#else
  bgp_serv_sock_family (port, AF_INET);
#endif /* HAVE_IPV6 && !defined(NRL) */
}

/* After TCP connection is established.  Get local address and port. */
void
bgp_getsockname (struct peer *peer)
{
  if (peer->su_local)
    {
      XFREE (MTYPE_TMP, peer->su_local);
      peer->su_local = NULL;
    }

  if (peer->su_remote)
    {
      XFREE (MTYPE_TMP, peer->su_remote);
      peer->su_remote = NULL;
    }

  peer->su_local = sockunion_getsockname (peer->fd);
  peer->su_remote = sockunion_getpeername (peer->fd);

  bgp_nexthop_set (peer->su_local, peer->su_remote, &peer->nexthop, peer);
}
