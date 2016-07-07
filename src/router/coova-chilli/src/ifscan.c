/* -*- mode: c; c-basic-offset: 2 -*- */
/*
 * Copyright (C) 2007-2012 David Bird (Coova Technologies) <support@coova.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <net/if.h>

#include <sys/ioctl.h>
#include <net/if_arp.h>
#include <arpa/inet.h>

#define inaddrr(x) (*(struct in_addr *) &ifr->x[sizeof sa.sin_port])
#define IFRSIZE   ((int)(size * sizeof (struct ifreq)))

char *get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen)
{
  switch(sa->sa_family) {
    case AF_INET:
      inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
                s, maxlen);
      break;

    case AF_INET6:
      inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr),
                s, maxlen);
      break;

    default:
      strncpy(s, "Unknown AF", maxlen);
      break;
  }

  return s;
}

int main(void)
{
  unsigned char      *u;
  int                sockfd, size  = 1;
  struct ifreq       *ifr;
  struct ifconf      ifc;
  struct sockaddr_in sa;

  if (0 > (sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP))) {
    fprintf(stderr, "Cannot open socket.\n");
    exit(EXIT_FAILURE);
  }

  ifc.ifc_len = IFRSIZE;
  ifc.ifc_req = NULL;

  do {
    ++size;
    /* realloc buffer size until no overflow occurs  */
    if (NULL == (ifc.ifc_req = realloc(ifc.ifc_req, IFRSIZE))) {
      fprintf(stderr, "Out of memory.\n");
      exit(EXIT_FAILURE);
    }
    ifc.ifc_len = IFRSIZE;
    if (ioctl(sockfd, SIOCGIFCONF, &ifc)) {
      perror("ioctl SIOCFIFCONF");
      exit(EXIT_FAILURE);
    }
  } while  (IFRSIZE <= ifc.ifc_len);

  ifr = ifc.ifc_req;
  for (;(char *) ifr < (char *) ifc.ifc_req + ifc.ifc_len; ++ifr) {

    char ip[INET6_ADDRSTRLEN];

    if (ifr->ifr_addr.sa_data == (ifr+1)->ifr_addr.sa_data) {
      continue;  /* duplicate, skip it */
    }

    if (ioctl(sockfd, SIOCGIFFLAGS, ifr)) {
      continue;  /* failed to get flags, skip it */
    }

    printf("Interface:  %s\n", ifr->ifr_name);
    printf("IP Address: %s\n", get_ip_str(&ifr->ifr_addr, ip, sizeof ip));
    printf("IP Address: %s\n", inet_ntoa(inaddrr(ifr_addr.sa_data)));

    if (0 == ioctl(sockfd, SIOCGIFHWADDR, ifr)) {

      switch (ifr->ifr_hwaddr.sa_family) {
        default:
          printf("\n");
          continue;
        case  ARPHRD_NETROM:  case  ARPHRD_ETHER:  case  ARPHRD_PPP:
        case  ARPHRD_EETHER:  case  ARPHRD_IEEE802: break;
      }

      u = (unsigned char *) &ifr->ifr_addr.sa_data;

      if (u[0] + u[1] + u[2] + u[3] + u[4] + u[5]) {
        printf("HW Address: %2.2X-%2.2X-%2.2X-%2.2X-%2.2X-%2.2x\n",
	       u[0], u[1], u[2], u[3], u[4], u[5]);
      }
    }

    if (0 == ioctl(sockfd, SIOCGIFNETMASK, ifr) &&
	strcmp("255.255.255.255", inet_ntoa(inaddrr(ifr_addr.sa_data)))) {
      printf("Netmask:    %s\n", inet_ntoa(inaddrr(ifr_addr.sa_data)));
    }

    if (ifr->ifr_flags & IFF_BROADCAST) {
      if (0 == ioctl(sockfd, SIOCGIFBRDADDR, ifr) &&
	  strcmp("0.0.0.0", inet_ntoa(inaddrr(ifr_addr.sa_data)))) {
        printf("Broadcast:  %s\n", inet_ntoa(inaddrr(ifr_addr.sa_data)));
      }
    }

    if (0 == ioctl(sockfd, SIOCGIFMTU, ifr)) {
      printf("MTU:        %u\n",  ifr->ifr_mtu);
    }

    if (0 == ioctl(sockfd, SIOCGIFMETRIC, ifr)) {
      printf("Metric:     %u\n",  ifr->ifr_metric);
    } printf("\n");
  }

  close(sockfd);
  return EXIT_SUCCESS;
}
