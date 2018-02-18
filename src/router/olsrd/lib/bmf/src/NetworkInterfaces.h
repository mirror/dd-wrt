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

#ifndef _BMF_NETWORKINTERFACES_H
#define _BMF_NETWORKINTERFACES_H

/* -------------------------------------------------------------------------
 * File       : NetworkInterfaces.h
 * Description: Functions to open and close sockets
 * Created    : 29 Jun 2006
 *
 * ------------------------------------------------------------------------- */

/* System includes */
#include <netinet/in.h> /* struct in_addr */

/* OLSR includes */
#include "olsr_types.h" /* olsr_ip_addr */
#include "olsrd_plugin.h" /* union set_plugin_parameter_addon */

/* Plugin includes */
#include "Packet.h" /* IFHWADDRLEN */

/* Size of buffer in which packets are received */
#define BMF_BUFFER_SIZE 2048

struct TBmfInterface
{
  /* File descriptor of raw packet socket, used for capturing multicast packets */
  int capturingSkfd;

  /* File descriptor of UDP (datagram) socket for encapsulated multicast packets. 
   * Only used for OLSR-enabled interfaces; set to -1 if interface is not OLSR-enabled. */
  int encapsulatingSkfd;

  /* File descriptor of UDP packet socket, used for listening to encapsulation packets.
   * Used only when PlParam "BmfMechanism" is set to "UnicastPromiscuous". */
  int listeningSkfd;

  unsigned char macAddr[IFHWADDRLEN];

  char ifName[IFNAMSIZ];

  /* OLSRs idea of this network interface. NULL if this interface is not
   * OLSR-enabled. */
  struct interface_olsr * olsrIntf;

  /* IP address of this network interface */
  union olsr_ip_addr intAddr;

  /* Broadcast address of this network interface */
  union olsr_ip_addr broadAddr;

  /* Number of received and transmitted BMF packets on this interface */
  u_int32_t nBmfPacketsRx;
  u_int32_t nBmfPacketsRxDup;
  u_int32_t nBmfPacketsTx;

  /* Next element in list */
  struct TBmfInterface* next; 
};

extern struct TBmfInterface* BmfInterfaces;

extern int HighestSkfd;
extern fd_set InputSet;

extern int EtherTunTapFd;

extern char EtherTunTapIfName[];

/* 10.255.255.253 in host byte order */
#define ETHERTUNTAPDEFAULTIP 0x0AFFFFFD

extern u_int32_t EtherTunTapIp;
extern u_int32_t EtherTunTapIpMask;
extern u_int32_t EtherTunTapIpBroadcast;

extern int CapturePacketsOnOlsrInterfaces;

enum TBmfMechanism { BM_BROADCAST = 0, BM_UNICAST_PROMISCUOUS };
extern enum TBmfMechanism BmfMechanism;

int SetBmfInterfaceName(const char* ifname, void* data, set_plugin_parameter_addon addon);
int SetBmfInterfaceIp(const char* ip, void* data, set_plugin_parameter_addon addon);
int SetBmfInterfacePersistent(const char* value, void* data, set_plugin_parameter_addon addon);
int SetCapturePacketsOnOlsrInterfaces(const char* enable, void* data, set_plugin_parameter_addon addon);
int SetBmfMechanism(const char* mechanism, void* data, set_plugin_parameter_addon addon);
int DeactivateSpoofFilter(void);
void RestoreSpoofFilter(void);

#define MAX_UNICAST_NEIGHBORS 10
struct TBestNeighbors
{
  struct link_entry* links[MAX_UNICAST_NEIGHBORS];
};

void FindNeighbors(
  struct TBestNeighbors* neighbors,
  struct link_entry** bestNeighbor,
  struct TBmfInterface* intf,
  union olsr_ip_addr* source,
  union olsr_ip_addr* forwardedBy,
  union olsr_ip_addr* forwardedTo,
  int* nPossibleNeighbors);

int CreateBmfNetworkInterfaces(struct interface_olsr * skipThisIntf);
void CloseBmfNetworkInterfaces(void);
int AddNonOlsrBmfIf(const char* ifName, void* data, set_plugin_parameter_addon addon);
int IsNonOlsrBmfIf(const char* ifName);
void CheckAndUpdateLocalBroadcast(unsigned char* ipPacket, union olsr_ip_addr* broadAddr);
void AddMulticastRoute(void);
void DeleteMulticastRoute(void);

#endif /* _BMF_NETWORKINTERFACES_H */
