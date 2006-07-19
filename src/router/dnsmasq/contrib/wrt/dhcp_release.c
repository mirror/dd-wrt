/* Copyright (c) 2006 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

/* dhcp_release <interface> <address> <MAC address> <client_id>
   MUST be run as root - will fail otherwise. */

/* Send a DHCPRELEASE message via the specified interface 
   to tell the local DHCP server to delete a particular lease. 
   
   The interface argument is the interface in which a DHCP
   request _would_ be received if it was coming from the client, 
   rather than being faked up here.
   
   The address argument is a dotted-quad IP addresses and mandatory. 
   
   The MAC address is colon separated hex, and is mandatory. It may be 
   prefixed by an address-type byte followed by -, eg

   10-11:22:33:44:55:66

   but if the address-type byte is missing it is assumed to be 1, the type 
   for ethernet. This encoding is the one used in dnsmasq lease files.

   The client-id is optional. If it is "*" then it treated as being missing.
*/

#include <sys/types.h> 
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>

#define DHCP_CHADDR_MAX          16
#define BOOTREQUEST              1
#define DHCP_COOKIE              0x63825363
#define OPTION_SERVER_IDENTIFIER 54
#define OPTION_CLIENT_ID         61
#define OPTION_MESSAGE_TYPE      53
#define OPTION_END               255
#define DHCPRELEASE              7
#define DHCP_SERVER_PORT         67

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

struct dhcp_packet {
  u8 op, htype, hlen, hops;
  u32 xid;
  u16 secs, flags;
  struct in_addr ciaddr, yiaddr, siaddr, giaddr;
  u8 chaddr[DHCP_CHADDR_MAX], sname[64], file[128];
  u32 cookie;
  unsigned char options[308];
};

static int parse_hex(char *in, unsigned char *out, int maxlen, int *mac_type)
{
  int i = 0;
  char *r;
    
  if (mac_type)
    *mac_type = 0;
  
  while (maxlen == -1 || i < maxlen)
    {
      for (r = in; *r != 0 && *r != ':' && *r != '-'; r++);
      if (*r == 0)
        maxlen = i;
      
      if (r != in )
        {
          if (*r == '-' && i == 0 && mac_type)
           {
              *r = 0;
              *mac_type = strtol(in, NULL, 16);
              mac_type = NULL;
           }
          else
            {
              *r = 0;
	      out[i] = strtol(in, NULL, 16);
              i++;
            }
        }
      in = r+1;
    }
    return i;
}

int main(int argc, char **argv)
{ 
  struct in_addr server, lease;
  int mac_type;
  struct dhcp_packet packet;
  unsigned char *p = packet.options;
  struct sockaddr_in dest;
  struct ifreq ifr;
  int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

  if (argc < 4 || argc > 5)
    { 
      fprintf(stderr, "usage: dhcp_release <interface> <addr> <mac> [<client_id>]\n");
      exit(1);
    }

  if (fd == -1)
    {
      perror("cannot create socket");
      exit(1);
    }
  
  /* This voodoo fakes up a packet coming from the correct interface, which really matters for 
     a DHCP server */
  strcpy(ifr.ifr_name, argv[1]);
  ifr.ifr_addr.sa_family = AF_INET;
  if (ioctl(fd, SIOCGIFADDR, &ifr) == -1 || setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr)) == -1)
    {
      perror("cannot setup interface");
      exit(1);
    }
  
  server = ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr;
  lease.s_addr = inet_addr(argv[2]);
  
  memset(&packet, 0, sizeof(packet));
 
  packet.hlen = parse_hex(argv[3], packet.chaddr, DHCP_CHADDR_MAX, &mac_type);
  if (mac_type == 0)
    packet.htype = ARPHRD_ETHER;
  else
    packet.htype = mac_type;

  packet.op = BOOTREQUEST;
  packet.ciaddr = lease;
  packet.cookie = htonl(DHCP_COOKIE);

  *(p++) = OPTION_MESSAGE_TYPE;
  *(p++) = 1;
  *(p++) = DHCPRELEASE;

  *(p++) = OPTION_SERVER_IDENTIFIER;
  *(p++) = sizeof(server);
  memcpy(p, &server, sizeof(server));
  p += sizeof(server);

  if (argc == 5 && strcmp(argv[4], "*") != 0)
    {
      unsigned int clid_len = parse_hex(argv[4], p+2, 255, NULL);
      *(p++) = OPTION_CLIENT_ID;
      *(p++) = clid_len;
      p += clid_len;
    }
  
  *(p++) = OPTION_END;
 
  dest.sin_family = AF_INET;
  dest.sin_port = ntohs(DHCP_SERVER_PORT);
  dest.sin_addr = server;

  if (sendto(fd, &packet, sizeof(packet), 0, 
	     (struct sockaddr *)&dest, sizeof(dest)) == 1)
    {
      perror("sendto failed");
      exit(1);
    }

  return 0;
}
