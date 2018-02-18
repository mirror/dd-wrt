/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
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
 */

#ifdef _WIN32

#if defined WINCE
#include <sys/types.h>          // for time_t
#endif /* defined WINCE */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#undef interface

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include "net_os.h"
#include "net_olsr.h"
#include "ipcalc.h"
#include "olsr.h"

#if defined WINCE
#define WIDE_STRING(s) L##s
#else /* defined WINCE */
#define WIDE_STRING(s) TEXT(s)
#endif /* defined WINCE */

void WinSockPError(const char *Str);
void PError(const char *);

void DisableIcmpRedirects(void);

int
gethemusocket(struct sockaddr_in *pin)
{
  int sock;

  OLSR_PRINTF(1, "       Connecting to switch daemon port 10150...");

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("hcsocket");
    return (-1);
  }

  /* connect to PORT on HOST */
  if (connect(sock, (struct sockaddr *)pin, sizeof(*pin)) < 0) {
    printf("FAILED\n");
    fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno));
    printf("connection refused\n");
    closesocket(sock);
    return (-1);
  }

  printf("OK\n");

  /* Keep TCP socket blocking */
  return (sock);
}

int
getsocket(int bufspace, struct interface_olsr *ifp __attribute__ ((unused)))
{
  struct sockaddr_in Addr;
  int On = 1;
  unsigned long Len;
  int Sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (Sock < 0) {
    WinSockPError("getsocket/socket()");
    return -1;
  }

  if (setsockopt(Sock, SOL_SOCKET, SO_BROADCAST, (char *)&On, sizeof(On)) < 0) {
    WinSockPError("getsocket/setsockopt(SO_BROADCAST)");
    closesocket(Sock);
    return -1;
  }

  if (setsockopt(Sock, SOL_SOCKET, SO_REUSEADDR, (char *)&On, sizeof(On)) < 0) {
    WinSockPError("getsocket/setsockopt(SO_REUSEADDR)");
    closesocket(Sock);
    return -1;
  }

  while (bufspace > 8192) {
    if (setsockopt(Sock, SOL_SOCKET, SO_RCVBUF, (char *)&bufspace, sizeof(bufspace)) == 0)
      break;

    bufspace -= 1024;
  }

  if (bufspace <= 8192) {
    OLSR_PRINTF(1, "Cannot set IPv4 socket receive buffer.\n");
  }
  memset(&Addr, 0, sizeof(Addr));
  Addr.sin_family = AF_INET;
  Addr.sin_port = htons(olsr_cnf->olsrport);

  if(bufspace <= 0) {
    Addr.sin_addr.s_addr = ifp->int_addr.sin_addr.s_addr;
  }

  if (bind(Sock, (struct sockaddr *)&Addr, sizeof(Addr)) < 0) {
    WinSockPError("getsocket/bind()");
    closesocket(Sock);
    return -1;
  }

  if (WSAIoctl(Sock, FIONBIO, &On, sizeof(On), NULL, 0, &Len, NULL, NULL) < 0) {
    WinSockPError("WSAIoctl");
    closesocket(Sock);
    return -1;
  }

  return Sock;
}

int
getsocket6(int bufspace, struct interface_olsr *ifp __attribute__ ((unused)))
{
  struct sockaddr_in6 Addr6;
  int On = 1;
  int Sock = socket(AF_INET6, SOCK_DGRAM, 0);
  if (Sock < 0) {
    WinSockPError("getsocket6/socket()");
    return -1;
  }

  if (setsockopt(Sock, SOL_SOCKET, SO_BROADCAST, (char *)&On, sizeof(On)) < 0) {
    WinSockPError("getsocket6/setsockopt(SO_BROADCAST)");
    closesocket(Sock);
    return -1;
  }

  if (setsockopt(Sock, SOL_SOCKET, SO_REUSEADDR, (char *)&On, sizeof(On)) < 0) {
    WinSockPError("getsocket6/setsockopt(SO_REUSEADDR)");
    closesocket(Sock);
    return -1;
  }

  while (bufspace > 8192) {
    if (setsockopt(Sock, SOL_SOCKET, SO_RCVBUF, (char *)&bufspace, sizeof(bufspace)) == 0)
      break;

    bufspace -= 1024;
  }

  if (bufspace <= 8192)
    fprintf(stderr, "Cannot set IPv6 socket receive buffer.\n");

  memset(&Addr6, 0, sizeof(Addr6));
  Addr6.sin6_family = AF_INET6;
  Addr6.sin6_port = htons(olsr_cnf->olsrport);

  if(bufspace <= 0) {
    memcpy(&Addr6.sin6_addr, &ifp->int6_addr.sin6_addr, sizeof(struct in6_addr));
  }

  if (bind(Sock, (struct sockaddr *)&Addr6, sizeof(Addr6)) < 0) {
    WinSockPError("getsocket6/bind()");
    closesocket(Sock);
    return -1;
  }

  return Sock;
}

static OVERLAPPED RouterOver;

void net_os_set_global_ifoptions(void)
{
  HMODULE Lib;
  unsigned int __stdcall(*enable_router)(HANDLE *, OVERLAPPED *);
  HANDLE Hand;

  Lib = LoadLibrary(WIDE_STRING("iphlpapi.dll"));

  if (Lib == NULL)
    return;

  enable_router = (unsigned int __stdcall(*)(HANDLE *, OVERLAPPED *))GetProcAddress(Lib, WIDE_STRING("EnableRouter"));

  if (enable_router == NULL)
    return;

  memset(&RouterOver, 0, sizeof(OVERLAPPED));

  RouterOver.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

  if (RouterOver.hEvent == NULL) {
    PError("CreateEvent()");
    return;
  }

  if (enable_router(&Hand, &RouterOver) != ERROR_IO_PENDING) {
    PError("EnableRouter()");
    return;
  }

  OLSR_PRINTF(3, "Routing enabled.\n");

  return;
}

static int
disable_ip_forwarding(int Ver)
{
  HMODULE Lib;
  unsigned int __stdcall(*unenable_router)(OVERLAPPED *, unsigned int *);
  unsigned int Count;

  Ver = Ver;

  Lib = LoadLibrary(WIDE_STRING("iphlpapi.dll"));

  if (Lib == NULL)
    return 0;

  unenable_router = (unsigned int __stdcall(*)(OVERLAPPED *, unsigned int *))GetProcAddress(Lib, WIDE_STRING("UnenableRouter"));

  if (unenable_router == NULL)
    return 0;

  if (unenable_router(&RouterOver, &Count) != NO_ERROR) {
    PError("UnenableRouter()");
    return -1;
  }

  OLSR_PRINTF(3, "Routing disabled, count = %u.\n", Count);

  return 0;
}


int
net_os_restore_ifoptions(void)
{
  disable_ip_forwarding(olsr_cnf->ip_version);

  return 0;
}

static int
SetEnableRedirKey(unsigned long New)
{
#if !defined WINCE
  HKEY Key;
  unsigned long Type;
  unsigned long Len;
  unsigned long Old;

  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters", 0, KEY_READ | KEY_WRITE, &Key) !=
      ERROR_SUCCESS)
    return -1;

  Len = sizeof(Old);

  if (RegQueryValueEx(Key, "EnableICMPRedirect", NULL, &Type, (unsigned char *)&Old, &Len) != ERROR_SUCCESS || Type != REG_DWORD)
    Old = 1;

  if (RegSetValueEx(Key, "EnableICMPRedirect", 0, REG_DWORD, (unsigned char *)&New, sizeof(New))) {
    RegCloseKey(Key);
    return -1;
  }

  RegCloseKey(Key);
  return Old;
#else /* !defined WINCE */
  return 0;
#endif /* !defined WINCE */
}

void
DisableIcmpRedirects(void)
{
  int Res;

  Res = SetEnableRedirKey(0);

  if (Res != 1)
    return;

  fprintf(stderr, "\n*** IMPORTANT *** IMPORTANT *** IMPORTANT *** IMPORTANT *** IMPORTANT ***\n\n");

  fprintf(stderr, "I have disabled ICMP redirect processing in the registry for you.\n");
  fprintf(stderr, "REBOOT NOW, so that these changes take effect. Exiting...\n\n");

  olsr_exit(NULL, EXIT_SUCCESS);
}

int
join_mcast(struct interface_olsr *Nic, int Sock)
{
  /* See linux/in6.h */
  struct ipaddr_str buf;
  struct ipv6_mreq McastReq;

  McastReq.ipv6mr_multiaddr = Nic->int6_multaddr.sin6_addr;
  McastReq.ipv6mr_interface = Nic->if_index;

  OLSR_PRINTF(3, "Interface %s joining multicast %s...", Nic->int_name,
              olsr_ip_to_string(&buf, (union olsr_ip_addr *)&Nic->int6_multaddr.sin6_addr));
  /* Send multicast */
  if (setsockopt(Sock, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (char *)&McastReq, sizeof(struct ipv6_mreq)) < 0) {
    perror("Join multicast");
    return -1;
  }

  /* Old libc fix */
#ifdef IPV6_JOIN_GROUP
  /* Join receiver group */
  if (setsockopt(Sock, IPPROTO_IPV6, IPV6_JOIN_GROUP, (char *)&McastReq, sizeof(struct ipv6_mreq)) < 0)
#else /* IPV6_JOIN_GROUP */
  /* Join receiver group */
  if (setsockopt(Sock, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (char *)&McastReq, sizeof(struct ipv6_mreq)) < 0)
#endif /* IPV6_JOIN_GROUP */
  {
    perror("Join multicast send");
    return -1;
  }

  if (setsockopt(Sock, IPPROTO_IPV6, IPV6_MULTICAST_IF, (char *)&McastReq.ipv6mr_interface, sizeof(McastReq.ipv6mr_interface)) < 0) {
    perror("Set multicast if");
    return -1;
  }

  OLSR_PRINTF(3, "OK\n");
  return 0;
}

/**
 * Wrapper for sendto(2)
 */

ssize_t
olsr_sendto(int s, const void *buf, size_t len, int flags, const struct sockaddr * to, socklen_t tolen)
{
  return sendto(s, buf, len, flags, to, tolen);
}

/**
 * Wrapper for recvfrom(2)
 */

ssize_t
olsr_recvfrom(int s, void *buf, size_t len, int flags __attribute__ ((unused)), struct sockaddr * from, socklen_t * fromlen)
{
  return recvfrom(s, buf, len, 0, from, fromlen);
}

/**
 * Wrapper for select(2)
 */

int
olsr_select(int nfds, fd_set * readfds, fd_set * writefds, fd_set * exceptfds, struct timeval *timeout)
{
#ifdef _WIN32
  if (nfds == 0) {
    if (timeout) {
      Sleep(timeout->tv_sec * 1000 + timeout->tv_usec / 1000);
    }
    return 0;
  }
  else {
    return select(nfds, readfds, writefds, exceptfds, timeout);
  }
#else /* _WIN32 */
  return select(nfds, readfds, writefds, exceptfds, timeout);
#endif /* _WIN32 */
}

#endif /* _WIN32 */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
