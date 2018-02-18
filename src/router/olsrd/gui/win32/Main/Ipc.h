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

#if !defined TL_IPC_H
#define TL_IPC_H

#define MSG_TYPE_OLSR_HELLO 1
#define MSG_TYPE_OLSR_TC 2
#define MSG_TYPE_OLSR_MID 3
#define MSG_TYPE_OLSR_HNA 4
#define MSG_TYPE_OLSR_LQ_HELLO 201
#define MSG_TYPE_OLSR_LQ_TC 202

#define MSG_TYPE_IPC_ROUTE 11
#define MSG_TYPE_IPC_CONFIG 12

#pragma pack(push, BeforeIpcMessages, 1)

struct OlsrHeader {
  unsigned char Type;
  unsigned char VTime;
  unsigned short Size;
  unsigned int Orig;
  unsigned char Ttl;
  unsigned char Hops;
  unsigned short SeqNo;
};

struct OlsrHello {
  struct OlsrHeader Header;

  unsigned short Reserved;
  unsigned char HTime;
  unsigned char Will;
};

struct OlsrHelloLink {
  unsigned char LinkCode;
  unsigned char Reserved;
  unsigned short Size;
};

struct OlsrTc {
  struct OlsrHeader Header;

  unsigned short Ansn;
  unsigned short Reserved;
};

union IpcIpAddr {
  unsigned int v4;
  unsigned char v6[16];
};

struct IpcHeader {
  unsigned char Type;
  unsigned char Reserved;
  unsigned short Size;
};

struct IpcRoute {
  struct IpcHeader Header;

  unsigned char Metric;
  unsigned char Add;
  unsigned char Reserved[2];
  union IpcIpAddr Dest;
  union IpcIpAddr Gate;
  char Int[4];
};

struct IpcConfig {
  struct IpcHeader Header;

  unsigned char NumMid;
  unsigned char NumHna;
  unsigned char Reserved1[2];
  unsigned short HelloInt;
  unsigned short WiredHelloInt;
  unsigned short TcInt;
  unsigned short HelloHold;
  unsigned short TcHold;
  unsigned char Ipv6;
  unsigned char Reserved2;
  IpcIpAddr MainAddr;
};

#pragma pack (pop, BeforeIpcMessages)

#endif /* !defined TL_IPC_H */

#endif /* _WIN32 */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
