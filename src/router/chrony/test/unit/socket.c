/*
 **********************************************************************
 * Copyright (C) Luke Valenta  2023
 * Copyright (C) Miroslav Lichvar  2024
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
 */

#include <socket.c>
#include "test.h"

static void
test_preinitialise(void)
{
#ifdef LINUX
  /* Test LISTEN_FDS environment variable parsing */

  /* normal */
  putenv("LISTEN_FDS=2");
  SCK_PreInitialise();
  TEST_CHECK(reusable_fds == 2);

  /* negative */
  putenv("LISTEN_FDS=-2");
  SCK_PreInitialise();
  TEST_CHECK(reusable_fds == 0);

  /* trailing characters */
  putenv("LISTEN_FDS=2a");
  SCK_PreInitialise();
  TEST_CHECK(reusable_fds == 0);

  /* non-integer */
  putenv("LISTEN_FDS=a2");
  SCK_PreInitialise();
  TEST_CHECK(reusable_fds == 0);

  /* not set */
  unsetenv("LISTEN_FDS");
  SCK_PreInitialise();
  TEST_CHECK(reusable_fds == 0);
#endif
}

static void
send_and_recv(int type, int is_stream, int is_client_bound, int server_fd, int client_fd)
{
  SCK_Message *msg1, msg2;
  char buf1[16], buf2[16];

  TEST_CHECK(!SCK_IsReusable(server_fd));
  TEST_CHECK(!SCK_IsReusable(client_fd));

  if (random() % 2 && !SCK_EnableKernelRxTimestamping(client_fd))
    ;

  UTI_GetRandomBytes(buf1, sizeof (buf1));

  TEST_CHECK(SCK_Send(client_fd, buf1, sizeof (buf1), 0) == sizeof (buf1));
  msg1 = SCK_ReceiveMessage(server_fd, 0);

  TEST_CHECK(msg1);
  TEST_CHECK(msg1->data);
  TEST_CHECK(msg1->length == sizeof (buf1));
  if (is_stream) {
    TEST_CHECK(msg1->addr_type == type || msg1->addr_type == SCK_ADDR_UNSPEC);
  } else if (is_client_bound) {
    TEST_CHECK(msg1->addr_type == type);
    TEST_CHECK(msg1->remote_addr.ip.ip_addr.family != IPADDR_UNSPEC);
  } else {
    TEST_CHECK(msg1->addr_type == SCK_ADDR_UNSPEC);
  }

  TEST_CHECK(memcmp(buf1, msg1->data, sizeof (buf1)) == 0);

  UTI_GetRandomBytes(buf2, sizeof (buf2));
  SCK_InitMessage(&msg2, is_stream ? SCK_ADDR_UNSPEC : type);
  if (!is_stream)
    msg2.remote_addr = msg1->remote_addr;
  msg2.data = buf2;
  msg2.length = sizeof (buf1);

  TEST_CHECK(SCK_SendMessage(server_fd, &msg2, 0));
  TEST_CHECK(SCK_Receive(client_fd, buf1, sizeof (buf1), 0) == sizeof (buf1));
  TEST_CHECK(memcmp(buf1, buf2, sizeof (buf1)) == 0);
}

void
test_unit(void)
{
  int i, family, s1, s2, s3;
  IPSockAddr sa1, sa2;
  IPAddr ip1, ip2;

  test_preinitialise();

  SCK_PreInitialise();

  SCK_Initialise(IPADDR_INET4);
  TEST_CHECK(SCK_IsIpFamilyEnabled(IPADDR_INET4));
  TEST_CHECK(!SCK_IsIpFamilyEnabled(IPADDR_INET6));
  SCK_Finalise();

  SCK_Initialise(IPADDR_INET6);
  TEST_CHECK(!SCK_IsIpFamilyEnabled(IPADDR_INET4));
#ifdef FEAT_IPV6
  TEST_CHECK(SCK_IsIpFamilyEnabled(IPADDR_INET6));
#else
  TEST_CHECK(!SCK_IsIpFamilyEnabled(IPADDR_INET6));
#endif
  SCK_Finalise();

  SCK_Initialise(IPADDR_UNSPEC);
  TEST_CHECK(SCK_IsIpFamilyEnabled(IPADDR_INET4));
#ifdef FEAT_IPV6
  TEST_CHECK(SCK_IsIpFamilyEnabled(IPADDR_INET6));
#else
  TEST_CHECK(!SCK_IsIpFamilyEnabled(IPADDR_INET6));
#endif

  SCK_GetAnyLocalIPAddress(IPADDR_INET4, &ip1);
  if (UTI_StringToIP("0.0.0.0", &ip2))
    TEST_CHECK(UTI_CompareIPs(&ip1, &ip2, NULL) == 0);
  SCK_GetAnyLocalIPAddress(IPADDR_INET6, &ip1);
  if (UTI_StringToIP("::", &ip2))
    TEST_CHECK(UTI_CompareIPs(&ip1, &ip2, NULL) == 0);

  SCK_GetLoopbackIPAddress(IPADDR_INET4, &ip1);
  if (UTI_StringToIP("127.0.0.1", &ip2))
    TEST_CHECK(UTI_CompareIPs(&ip1, &ip2, NULL) == 0);
  SCK_GetLoopbackIPAddress(IPADDR_INET6, &ip1);
  if (UTI_StringToIP("::1", &ip2))
    TEST_CHECK(UTI_CompareIPs(&ip1, &ip2, NULL) == 0);

  if (UTI_StringToIP("169.254.100.100", &ip1))
    TEST_CHECK(SCK_IsLinkLocalIPAddress(&ip1));
  if (UTI_StringToIP("169.255.100.100", &ip1))
    TEST_CHECK(!SCK_IsLinkLocalIPAddress(&ip2));
  if (UTI_StringToIP("fe80::", &ip1))
    TEST_CHECK(SCK_IsLinkLocalIPAddress(&ip1));
  if (UTI_StringToIP("fe81::", &ip1))
    TEST_CHECK(!SCK_IsLinkLocalIPAddress(&ip2));

  for (family = IPADDR_INET4; family <= IPADDR_INET6; family++) {
    SCK_GetLoopbackIPAddress(family, &sa1.ip_addr);

    for (i = 0; i < 16; i++) {
      sa1.port = 1024 + random() % 60000;
      s1 = SCK_OpenUdpSocket(NULL, &sa1, NULL, SCK_FLAG_BLOCK);
      if (s1 >= 0) {
        s2 = SCK_OpenUdpSocket(&sa1, NULL, NULL, SCK_FLAG_BLOCK);
        TEST_CHECK(s2 >= 0);

        send_and_recv(SCK_ADDR_IP, 0, 1, s1, s2);

        SCK_CloseSocket(s1);
        SCK_CloseSocket(s2);
      }
    }

    for (i = 0; i < 16; i++) {
      sa1.port = 1024 + random() % 60000;
      s1 = SCK_OpenTcpSocket(NULL, &sa1, NULL, SCK_FLAG_BLOCK);
      if (s1 >= 0) {
        TEST_CHECK(SCK_ListenOnSocket(s1, 1));
        if (i % 2) {
          sa2 = sa1;
          sa2.port = 1024 + random() % 60000;
          s2 = SCK_OpenTcpSocket(&sa1, &sa2, NULL, SCK_FLAG_BLOCK);
          if (s2 < 0) {
            SCK_CloseSocket(s1);
            continue;
          }
        } else {
          s2 = SCK_OpenTcpSocket(&sa1, NULL, NULL, SCK_FLAG_BLOCK);
        }
        TEST_CHECK(s2 >= 0);
        s3 = SCK_AcceptConnection(s1, &sa2);
        TEST_CHECK(UTI_CompareIPs(&sa1.ip_addr, &sa2.ip_addr, NULL) == 0);

        send_and_recv(SCK_ADDR_IP, 1, 1, s3, s2);

        SCK_ShutdownConnection(s2);
        SCK_ShutdownConnection(s3);
        SCK_CloseSocket(s1);
        SCK_CloseSocket(s2);
        SCK_CloseSocket(s3);
      }
    }
  }

  for (i = 0; i < 16; i++) {
    s1 = SCK_OpenUnixDatagramSocket(NULL, "testsocket1", SCK_FLAG_BLOCK);
    TEST_CHECK(s1 >= 0);
    s2 = SCK_OpenUnixDatagramSocket("testsocket1", "testsocket2", SCK_FLAG_BLOCK);
    TEST_CHECK(s2 >= 0);

    send_and_recv(SCK_ADDR_UNIX, 0, 1, s1, s2);

    if (i % 2)
      TEST_CHECK(SCK_RemoveSocket(s1));
    if (i % 2)
      TEST_CHECK(SCK_RemoveSocket(s2));
    SCK_CloseSocket(s1);
    SCK_CloseSocket(s2);
  }

  for (i = 0; i < 16; i++) {
    s1 = SCK_OpenUnixStreamSocket(NULL, "testsocket1", SCK_FLAG_BLOCK);
    TEST_CHECK(s1 >= 0);
    TEST_CHECK(SCK_ListenOnSocket(s1, 1));
    s2 = SCK_OpenUnixStreamSocket("testsocket1", i % 2 ? "testsocket2" : NULL, SCK_FLAG_BLOCK);
    TEST_CHECK(s2 >= 0);
    s3 = SCK_AcceptConnection(s1, &sa2);
    TEST_CHECK(sa2.ip_addr.family == IPADDR_UNSPEC);

    send_and_recv(SCK_ADDR_UNIX, 1, i % 2, s3, s2);

    if (i % 4)
      TEST_CHECK(SCK_RemoveSocket(s1));
    SCK_ShutdownConnection(s2);
    SCK_ShutdownConnection(s3);
    SCK_CloseSocket(s1);
    SCK_CloseSocket(s2);
    SCK_CloseSocket(s3);
  }

  for (i = 0; i < 16; i++) {
    s1 = SCK_OpenUnixSocketPair(SCK_FLAG_BLOCK, &s2);
    TEST_CHECK(s1 >= 0);
    TEST_CHECK(s2 >= 0);

    send_and_recv(SCK_ADDR_UNIX, 1, 0, s1, s2);

    SCK_CloseSocket(s1);
    SCK_CloseSocket(s2);
  }

  SCK_Finalise();
}
