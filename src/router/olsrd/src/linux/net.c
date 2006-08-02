/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 *
 * * Redistributions of source code must retain the above copyright 
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright 
 *   notice, this list of conditions and the following disclaimer in 
 *   the documentation and/or other materials provided with the 
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its 
 *   contributors may be used to endorse or promote products derived 
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 * $Id: net.c,v 1.29 2005/09/17 20:48:50 kattemat Exp $
 */


/*
 * Linux spesific code
 */

#include "net.h"
#include "../defs.h"
#include "../net_os.h"
#include "../parser.h"


/*
 *Wireless definitions for ioctl calls
 *(from linux/wireless.h)
 */
#define SIOCGIWNAME	0x8B01		/* get name == wireless protocol */
#define SIOCGIWRATE	0x8B21		/* get default bit rate (bps) */

/**
 *Bind a socket to a device
 *
 *@param sock the socket to bind
 *@param dev_name name of the device
 *
 *@return negative if error
 */

int
bind_socket_to_device(int sock, char *dev_name)
{
  /*
   *Bind to device using the SO_BINDTODEVICE flag
   */
  OLSR_PRINTF(3, "Binding socket %d to device %s\n", sock, dev_name)
  return setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, dev_name, strlen(dev_name)+1);

}




/**
 *Enable IP forwarding.
 *Just writing "1" to the /proc/sys/net/ipv4/ip_forward
 *if using IPv4 or /proc/sys/net/ipv6/conf/all/forwarding
 *if using IPv6.
 *Could probably drop the check for
 *"0" here and write "1" anyways.
 *
 *@param version IP version.
 *
 *@return 1 on sucess 0 on failiure
 */ 
int
enable_ip_forwarding(int version)
{
  FILE *proc_fwd;
  char procfile[FILENAME_MAX];

  if(version == AF_INET)
    {
      strncpy(procfile, "/proc/sys/net/ipv4/ip_forward", FILENAME_MAX);
    }
  else
    if(version == AF_INET6)
      {
	strncpy(procfile, "/proc/sys/net/ipv6/conf/all/forwarding", FILENAME_MAX);
      }
    else
      return -1;


  if ((proc_fwd=fopen(procfile, "r"))==NULL)
    {
      /* IPv4 */
      if(version == AF_INET)
	fprintf(stderr, "WARNING! Could not open the %s file to check/enable IP forwarding!\nAre you using the procfile filesystem?\nDoes your system support IPv4?\nI will continue(in 3 sec) - but you should mannually ensure that IP forwarding is enabeled!\n\n", procfile);
      /* IPv6 */
      else
	fprintf(stderr, "WARNING! Could not open the %s file to check/enable IP forwarding!\nAre you using the procfile filesystem?\nDoes your system support IPv6?\nI will continue(in 3 sec) - but you should mannually ensure that IP forwarding is enabeled!\n\n", procfile);
      
      sleep(3);
      return 0;
    }
  
  else
    {
      orig_fwd_state = fgetc(proc_fwd);
      fclose(proc_fwd);
      if(orig_fwd_state == '1')
	{
	  OLSR_PRINTF(3, "\nIP forwarding is enabled on this system\n")
	}
      else
	{
	  if ((proc_fwd=fopen(procfile, "w"))==NULL)
	    {
	      fprintf(stderr, "Could not open %s for writing!\n", procfile);
	      fprintf(stderr, "I will continue(in 3 sec) - but you should mannually ensure that IP forwarding is enabeled!\n\n");
	      sleep(3);
	      return 0;
	    }
	  else
	    {
	      syslog(LOG_INFO, "Writing \"1\" to %s\n", procfile);
	      fputs("1", proc_fwd);
	    }
	  fclose(proc_fwd);

	}
    }
  return 1;
      
}


int
disable_redirects_global(int version)
{
  FILE *proc_redirect;
  char procfile[FILENAME_MAX];

  if(version == AF_INET6)
    return -1;

  strcpy(procfile, "/proc/sys/net/ipv4/conf/all/send_redirects");

  if((proc_redirect = fopen(procfile, "r")) == NULL)
    {
      fprintf(stderr, "WARNING! Could not open the %s file to check/disable ICMP redirects!\nAre you using the procfile filesystem?\nDoes your system support IPv4?\nI will continue(in 3 sec) - but you should mannually ensure that ICMP redirects are disabled!\n\n", procfile);
      
      sleep(3);
      return -1;
    }
  else
    {
      orig_global_redirect_state = fgetc(proc_redirect);
      fclose(proc_redirect);
    }

  if(orig_global_redirect_state == '0')
    {
      return 0;
    }

  if ((proc_redirect = fopen(procfile, "w"))==NULL)
    {
      fprintf(stderr, "Could not open %s for writing!\n", procfile);
      fprintf(stderr, "I will continue(in 3 sec) - but you should mannually ensure that ICMP redirect is disabeled!\n\n");
      sleep(3);
      return 0;
    }
  else
    {
      syslog(LOG_INFO, "Writing \"0\" to %s", procfile);
      fputs("0", proc_redirect);
    }
  fclose(proc_redirect);
  
  return 1;
}


/**
 *
 *@return 1 on sucess 0 on failiure
 */ 
int
disable_redirects(char *if_name, int index, int version)
{
  FILE *proc_redirect;
  char procfile[FILENAME_MAX];

  if(version == AF_INET6)
    return -1;

  /* Generate the procfile name */
  sprintf(procfile, REDIRECT_PROC, if_name);


  if((proc_redirect = fopen(procfile, "r")) == NULL)
    {
      fprintf(stderr, "WARNING! Could not open the %s file to check/disable ICMP redirects!\nAre you using the procfile filesystem?\nDoes your system support IPv4?\nI will continue(in 3 sec) - but you should mannually ensure that ICMP redirects are disabled!\n\n", procfile);
      
      sleep(3);
      return 0;
    }
  else
    {
      nic_states[index].redirect = fgetc(proc_redirect);
      fclose(proc_redirect);
      
    }

  if ((proc_redirect = fopen(procfile, "w"))==NULL)
    {
      fprintf(stderr, "Could not open %s for writing!\n", procfile);
      fprintf(stderr, "I will continue(in 3 sec) - but you should mannually ensure that ICMP redirect is disabeled!\n\n");
      sleep(3);
      return 0;
    }
  else
    {
      syslog(LOG_INFO, "Writing \"0\" to %s", procfile);
      fputs("0", proc_redirect);
    }
  fclose(proc_redirect);

  return 1;
}



/**
 *
 *@return 1 on sucess 0 on failiure
 */ 
int
deactivate_spoof(char *if_name, int index, int version)
{
  FILE *proc_spoof;
  char procfile[FILENAME_MAX];

  if(version == AF_INET6)
    return -1;


  /* Generate the procfile name */
  sprintf(procfile, SPOOF_PROC, if_name);


  if((proc_spoof = fopen(procfile, "r")) == NULL)
    {
      fprintf(stderr, "WARNING! Could not open the %s file to check/disable the IP spoof filter!\nAre you using the procfile filesystem?\nDoes your system support IPv4?\nI will continue(in 3 sec) - but you should mannually ensure that IP spoof filtering is disabled!\n\n", procfile);
      
      sleep(3);
      return 0;
    }
  else
    {
      nic_states[index].spoof = fgetc(proc_spoof);
      fclose(proc_spoof);
      
    }

  if ((proc_spoof = fopen(procfile, "w")) == NULL)
    {
      fprintf(stderr, "Could not open %s for writing!\n", procfile);
      fprintf(stderr, "I will continue(in 3 sec) - but you should mannually ensure that IP spoof filtering is disabeled!\n\n");
      sleep(3);
      return 0;
    }
  else
    {
      syslog(LOG_INFO, "Writing \"0\" to %s", procfile);
      fputs("0", proc_spoof);
    }
  fclose(proc_spoof);

  return 1;
}



/**
 *Resets the spoof filter and ICMP redirect settings
 */

int
restore_settings(int version)
{
  FILE *proc_fd;
  char procfile[FILENAME_MAX];
  struct interface *ifs;

  OLSR_PRINTF(1, "Restoring network state\n")

  /* Restore IP forwarding to "off" */
  if(orig_fwd_state == '0')
    {
      if(version == AF_INET)
	{
	  strcpy(procfile, "/proc/sys/net/ipv4/ip_forward");
	}
      else if(version == AF_INET6)
	{
	  strcpy(procfile, "/proc/sys/net/ipv6/conf/all/forwarding");
	}

      if ((proc_fd = fopen(procfile, "w")) == NULL)
	{
	  fprintf(stderr, "Could not open %s for writing!\nSettings not restored!\n", procfile);
	}
      else
	{
	  syslog(LOG_INFO, "Resetting %s to %c\n", procfile, orig_fwd_state);
	  fputc(orig_fwd_state, proc_fd);
	  fclose(proc_fd);
	}

    }

  /* Restore global ICMP redirect setting */
  if(orig_global_redirect_state != '0')
    {
      if(version == AF_INET)
	{
	  strcpy(procfile, "/proc/sys/net/ipv4/conf/all/send_redirects");

	  if ((proc_fd = fopen(procfile, "w")) == NULL)
	    {
	      fprintf(stderr, "Could not open %s for writing!\nSettings not restored!\n", procfile);
	    }
	  else
	    {
	      syslog(LOG_INFO, "Resetting %s to %c\n", procfile, orig_global_redirect_state);
	      fputc(orig_global_redirect_state, proc_fd);
	      fclose(proc_fd);
	    }
	}
    }


  if(version == AF_INET6)
    return 0;

  for(ifs = ifnet; ifs != NULL; ifs = ifs->int_next)
    {
      /* Discard host-emulation interfaces */
      if(ifs->is_hcif)
	continue;
      /* ICMP redirects */
      
      /* Generate the procfile name */
      sprintf(procfile, REDIRECT_PROC, ifs->int_name);
      
      if ((proc_fd = fopen(procfile, "w")) == NULL)
	{
	  fprintf(stderr, "Could not open %s for writing!\nSettings not restored!\n", procfile);
	}
      else
	{
	  syslog(LOG_INFO, "Resetting %s to %c\n", procfile, nic_states[ifs->if_nr].redirect);

	  fputc(nic_states[ifs->if_nr].redirect, proc_fd);
	  fclose(proc_fd);
	}

      
      /* Spoof filter */
      
      /* Generate the procfile name */
      sprintf(procfile, SPOOF_PROC, ifs->int_name);

      if ((proc_fd = fopen(procfile, "w")) == NULL)
	{
	  fprintf(stderr, "Could not open %s for writing!\nSettings not restored!\n", procfile);
	}
      else
	{
	  syslog(LOG_INFO, "Resetting %s to %c\n", procfile, nic_states[ifs->if_nr].spoof);

	  fputc(nic_states[ifs->if_nr].spoof, proc_fd);
	  fclose(proc_fd);
	}

    }
  return 1;

}


/**
 *Creates a nonblocking broadcast socket.
 *@param sa sockaddr struct. Used for bind(2).
 *@return the FD of the socket or -1 on error.
 */
int
gethemusocket(struct sockaddr_in *pin)
{
  int sock, on = 1;

  OLSR_PRINTF(1, "       Connecting to switch daemon port 10150...");


  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
      perror("hcsocket");
      syslog(LOG_ERR, "hcsocket: %m");
      return (-1);
    }

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) 
    {
      perror("SO_REUSEADDR failed");
      return (-1);
    }
  /* connect to PORT on HOST */
  if (connect(sock,(struct sockaddr *) pin, sizeof(*pin)) < 0) 
    {
      printf("FAILED\n");
      fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno));
      printf("connection refused\n");
      return (-1);
    }

  printf("OK\n");

  /* Keep TCP socket blocking */  
  return (sock);
}


/**
 *Creates a nonblocking broadcast socket.
 *@param sa sockaddr struct. Used for bind(2).
 *@return the FD of the socket or -1 on error.
 */
int
getsocket(struct sockaddr *sa, int bufspace, char *int_name)
{
  struct sockaddr_in *sin=(struct sockaddr_in *)sa;
  int sock, on = 1;



  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
      perror("socket");
      syslog(LOG_ERR, "socket: %m");
      return (-1);
    }



#ifdef SO_BROADCAST
  if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &on, sizeof (on)) < 0)
    {
      perror("setsockopt");
      syslog(LOG_ERR, "setsockopt SO_BROADCAST: %m");
      close(sock);
      return (-1);
    }
#endif

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) 
    {
      perror("SO_REUSEADDR failed");
      return (-1);
    }



#ifdef SO_RCVBUF

  for (on = bufspace; ; on -= 1024) 
    {
      if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF,
		     &on, sizeof (on)) == 0)
	break;
      if (on <= 8*1024) 
	{
	  perror("setsockopt");
	  syslog(LOG_ERR, "setsockopt SO_RCVBUF: %m");
	  break;
	}
    }


#endif


  /*
   * WHEN USING KERNEL 2.6 THIS MUST HAPPEN PRIOR TO THE PORT BINDING!!!!
   */

  /* Bind to device */
  if(bind_socket_to_device(sock, int_name) < 0)
    {
      fprintf(stderr, "Could not bind socket to device... exiting!\n\n");
      syslog(LOG_ERR, "Could not bind socket to device... exiting!\n\n");
      return -1;
    }


  if (bind(sock, (struct sockaddr *)sin, sizeof (*sin)) < 0) 
    {
      perror("bind");
      syslog(LOG_ERR, "bind: %m");
      close(sock);
      return (-1);
    }

  /*
   *One should probably fetch the flags first
   *using F_GETFL....
   */
  if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1)
    syslog(LOG_ERR, "fcntl O_NONBLOCK: %m\n");

  return (sock);
}


/**
 *Creates a nonblocking IPv6 socket
 *@param sin sockaddr_in6 struct. Used for bind(2).
 *@return the FD of the socket or -1 on error.
 */
int
getsocket6(struct sockaddr_in6 *sin, int bufspace, char *int_name)
{
  int sock, on = 1;



  if ((sock = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) 
    {
      perror("socket");
      syslog(LOG_ERR, "socket: %m");
      return (-1);
    }

#ifdef IPV6_V6ONLY
  if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on)) < 0) 
    {
      perror("setsockopt(IPV6_V6ONLY)");
      syslog(LOG_ERR, "setsockopt(IPV6_V6ONLY): %m");
    }
#endif


  //#ifdef SO_BROADCAST
  /*
  if (setsockopt(sock, SOL_SOCKET, SO_MULTICAST, &on, sizeof (on)) < 0)
    {
      perror("setsockopt");
      syslog(LOG_ERR, "setsockopt SO_BROADCAST: %m");
      close(sock);
      return (-1);
    }
  */
  //#endif




#ifdef SO_RCVBUF
  for (on = bufspace; ; on -= 1024) 
    {
      if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF,
		     &on, sizeof (on)) == 0)
	break;
      if (on <= 8*1024) 
	{
	  perror("setsockopt");
	  syslog(LOG_ERR, "setsockopt SO_RCVBUF: %m");
	  break;
	}
    }


#endif

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) 
    {
      perror("SO_REUSEADDR failed");
      return (-1);
    }


  /*
   * WHEN USING KERNEL 2.6 THIS MUST HAPPEN PRIOR TO THE PORT BINDING!!!!
   */

  /* Bind to device */
  if(bind_socket_to_device(sock, int_name) < 0)
    {
      fprintf(stderr, "Could not bind socket to device... exiting!\n\n");
      syslog(LOG_ERR, "Could not bind socket to device... exiting!\n\n");
      return -1;
    }


  if (bind(sock, (struct sockaddr *)sin, sizeof (*sin)) < 0) 
    {
      perror("bind");
      syslog(LOG_ERR, "bind: %m");
      close(sock);
      return (-1);
    }

  /*
   *One should probably fetch the flags first
   *using F_GETFL....
   */
  if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1)
    syslog(LOG_ERR, "fcntl O_NONBLOCK: %m\n");



  return (sock);
}


int
join_mcast(struct interface *ifs, int sock)
{
  /* See linux/in6.h */

  struct ipv6_mreq mcastreq;

  COPY_IP(&mcastreq.ipv6mr_multiaddr, &ifs->int6_multaddr.sin6_addr);
  mcastreq.ipv6mr_interface = ifs->if_index;

#if !defined __FreeBSD__ && !defined __MacOSX__ && !defined __NetBSD__
  OLSR_PRINTF(3, "Interface %s joining multicast %s...",	ifs->int_name, olsr_ip_to_string((union olsr_ip_addr *)&ifs->int6_multaddr.sin6_addr))
  /* Send multicast */
  if(setsockopt(sock, 
		IPPROTO_IPV6, 
		IPV6_ADD_MEMBERSHIP, 
		(char *)&mcastreq, 
		sizeof(struct ipv6_mreq)) 
     < 0)
    {
      perror("Join multicast");
      return -1;
    }
#else
#warning implement IPV6_ADD_MEMBERSHIP
#endif

  /* Old libc fix */
#ifdef IPV6_JOIN_GROUP
  /* Join reciever group */
  if(setsockopt(sock, 
		IPPROTO_IPV6, 
		IPV6_JOIN_GROUP, 
		(char *)&mcastreq, 
		sizeof(struct ipv6_mreq)) 
     < 0)
#else
  /* Join reciever group */
  if(setsockopt(sock, 
		IPPROTO_IPV6, 
		IPV6_ADD_MEMBERSHIP, 
		(char *)&mcastreq, 
		sizeof(struct ipv6_mreq)) 
     < 0)
#endif 
    {
      perror("Join multicast send");
      return -1;
    }

  
  if(setsockopt(sock, 
		IPPROTO_IPV6, 
		IPV6_MULTICAST_IF, 
		(char *)&mcastreq.ipv6mr_interface, 
		sizeof(mcastreq.ipv6mr_interface)) 
     < 0)
    {
      perror("Set multicast if");
      return -1;
    }


  OLSR_PRINTF(3, "OK\n")
  return 0;
}



/*
 *From net-tools lib/interface.c
 *
 */

int
get_ipv6_address(char *ifname, struct sockaddr_in6 *saddr6, int scope_in)
{
  char addr6[40], devname[IFNAMSIZ];
  char addr6p[8][5];
  int plen, scope, dad_status, if_idx;
  FILE *f;
  struct sockaddr_in6 tmp_sockaddr6;

  if ((f = fopen(_PATH_PROCNET_IFINET6, "r")) != NULL) 
    {
      while (fscanf(f, "%4s%4s%4s%4s%4s%4s%4s%4s %02x %02x %02x %02x %20s\n",
		    addr6p[0], addr6p[1], addr6p[2], addr6p[3],
		    addr6p[4], addr6p[5], addr6p[6], addr6p[7],
		    &if_idx, &plen, &scope, &dad_status, devname) != EOF) 
	{
	  if (!strcmp(devname, ifname)) 
	    {
	      sprintf(addr6, "%s:%s:%s:%s:%s:%s:%s:%s",
		      addr6p[0], addr6p[1], addr6p[2], addr6p[3],
		      addr6p[4], addr6p[5], addr6p[6], addr6p[7]);
	      OLSR_PRINTF(5, "\tinet6 addr: %s\n", addr6)
	      OLSR_PRINTF(5, "\tScope: %d\n", scope)
	      if(scope == scope_in)
		{
		  OLSR_PRINTF(4, "Found addr: %s:%s:%s:%s:%s:%s:%s:%s\n",
			      addr6p[0], addr6p[1], addr6p[2], addr6p[3],
			      addr6p[4], addr6p[5], addr6p[6], addr6p[7])
		  inet_pton(AF_INET6,addr6,&tmp_sockaddr6);
		  memcpy(&saddr6->sin6_addr, &tmp_sockaddr6, sizeof(struct in6_addr));	  
		  fclose(f);
		  return 1;
		}
	    }
	}
      fclose(f);
    }
  
  return 0;
}


/**
 * Wrapper for sendto(2)
 */

ssize_t
olsr_sendto(int s, 
	    const void *buf, 
	    size_t len, 
	    int flags, 
	    const struct sockaddr *to, 
	    socklen_t tolen)
{
  return sendto(s, buf, len, flags, to, tolen);
}

/**
 * Wrapper for recvfrom(2)
 */

ssize_t  
olsr_recvfrom(int  s, 
	      void *buf, 
	      size_t len, 
	      int flags, 
	      struct sockaddr *from,
	      socklen_t *fromlen)
{
  return recvfrom(s, 
		  buf, 
		  len, 
		  flags, 
		  from, 
		  fromlen);
}

/**
 * Wrapper for select(2)
 */

int
olsr_select(int nfds,
            fd_set *readfds,
            fd_set *writefds,
            fd_set *exceptfds,
            struct timeval *timeout)
{
  return select(nfds,
                readfds,
                writefds,
                exceptfds,
                timeout);
}

int
check_wireless_interface(char *ifname)
{
  struct ifreq ifr;

  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));

  return (ioctl(ioctl_s, SIOCGIWNAME, &ifr) >= 0) ? 1 : 0;
}

#if 0

#include <linux/sockios.h>
#include <linux/types.h>

/* This data structure is used for all the MII ioctl's */
struct mii_data {
    __u16	phy_id;
    __u16	reg_num;
    __u16	val_in;
    __u16	val_out;
};


/* Basic Mode Control Register */
#define MII_BMCR		0x00
#define  MII_BMCR_RESET		0x8000
#define  MII_BMCR_LOOPBACK	0x4000
#define  MII_BMCR_100MBIT	0x2000
#define  MII_BMCR_AN_ENA	0x1000
#define  MII_BMCR_ISOLATE	0x0400
#define  MII_BMCR_RESTART	0x0200
#define  MII_BMCR_DUPLEX	0x0100
#define  MII_BMCR_COLTEST	0x0080

/* Basic Mode Status Register */
#define MII_BMSR		0x01
#define  MII_BMSR_CAP_MASK	0xf800
#define  MII_BMSR_100BASET4	0x8000
#define  MII_BMSR_100BASETX_FD	0x4000
#define  MII_BMSR_100BASETX_HD	0x2000
#define  MII_BMSR_10BASET_FD	0x1000
#define  MII_BMSR_10BASET_HD	0x0800
#define  MII_BMSR_NO_PREAMBLE	0x0040
#define  MII_BMSR_AN_COMPLETE	0x0020
#define  MII_BMSR_REMOTE_FAULT	0x0010
#define  MII_BMSR_AN_ABLE	0x0008
#define  MII_BMSR_LINK_VALID	0x0004
#define  MII_BMSR_JABBER	0x0002
#define  MII_BMSR_EXT_CAP	0x0001

int
calculate_if_metric(char *ifname)
{
  if(check_wireless_interface(ifname))
    {
      struct ifreq ifr;
      strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
      
      /* Get bit rate */
      if(ioctl(ioctl_s, SIOCGIWRATE, &ifr) < 0)
	{
	  OLSR_PRINTF(1, "Not able to find rate for WLAN interface %s\n", ifname)
	  return WEIGHT_WLAN_11MB;
	}
      
      OLSR_PRINTF(1, "Bitrate %d\n", ifr.ifr_ifru.ifru_ivalue)

      //WEIGHT_WLAN_LOW,          /* <11Mb WLAN     */
      //WEIGHT_WLAN_11MB,         /* 11Mb 802.11b   */
      //WEIGHT_WLAN_54MB,         /* 54Mb 802.11g   */
      return WEIGHT_WLAN_LOW;
    }
  else
    {
      /* Ethernet */
      /* Mii wizardry */
      struct ifreq ifr;
      struct mii_data *mii = (struct mii_data *)&ifr.ifr_data;
      int bmcr;
      memset(&ifr, 0, sizeof(ifr));
      strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));

      if (ioctl(ioctl_s, SIOCGMIIPHY, &ifr) < 0) {
	if (errno != ENODEV)
	  OLSR_PRINTF(1, "SIOCGMIIPHY on '%s' failed: %s\n",
		      ifr.ifr_name, strerror(errno))
	return WEIGHT_ETHERNET_DEFAULT;
      }

      mii->reg_num = MII_BMCR;
      if (ioctl(ioctl_s, SIOCGMIIREG, &ifr) < 0) {
	OLSR_PRINTF(1, "SIOCGMIIREG on %s failed: %s\n", ifr.ifr_name,
		    strerror(errno))
	return WEIGHT_ETHERNET_DEFAULT;
      }
      bmcr = mii->val_out;


      OLSR_PRINTF(1, "%s: ", ifr.ifr_name)
      OLSR_PRINTF(1, "%s Mbit, %s duplex\n",
		  (bmcr & MII_BMCR_100MBIT) ? "100" : "10",
		  (bmcr & MII_BMCR_DUPLEX) ? "full" : "half")
    
      is_if_link_up(ifname);

      if(mii->val_out & MII_BMCR_100MBIT)
	return WEIGHT_ETHERNET_100MB;
      else
	return WEIGHT_ETHERNET_10MB;
      //WEIGHT_ETHERNET_1GB,      /* Ethernet 1Gb   */

    }
}


olsr_bool
is_if_link_up(char *ifname)
{
  if(check_wireless_interface(ifname))
    {
      /* No link checking on wireless devices */
      return OLSR_TRUE;
    }
  else
    {
      /* Mii wizardry */
      struct ifreq ifr;
      struct mii_data *mii = (struct mii_data *)&ifr.ifr_data;
      int bmsr;
      memset(&ifr, 0, sizeof(ifr));
      strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));

      if (ioctl(ioctl_s, SIOCGMIIPHY, &ifr) < 0) {
	if (errno != ENODEV)
	  OLSR_PRINTF(1, "SIOCGMIIPHY on '%s' failed: %s\n",
		      ifr.ifr_name, strerror(errno))
	return WEIGHT_ETHERNET_DEFAULT;
      }
      mii->reg_num = MII_BMSR;
      if (ioctl(ioctl_s, SIOCGMIIREG, &ifr) < 0) {
	OLSR_PRINTF(1, "SIOCGMIIREG on %s failed: %s\n", ifr.ifr_name,
		    strerror(errno))
	return WEIGHT_ETHERNET_DEFAULT;
      }
      bmsr = mii->val_out;

      OLSR_PRINTF(1, "%s: ", ifr.ifr_name)
      OLSR_PRINTF(1, "%s\n", (bmsr & MII_BMSR_LINK_VALID) ? "link ok " : "no link ")
    
      return (bmsr & MII_BMSR_LINK_VALID);

    }
}

#else
int
calculate_if_metric(char *ifname)
{
  return check_wireless_interface(ifname);
}
#endif
  
