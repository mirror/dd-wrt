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

#include <stdlib.h>
#include <winsock2.h>
#include "interfaces.h"
#include "olsr.h"
#include "parser.h"
#include "defs.h"
#include "net_os.h"
#include "ifnet.h"
#include "generate_msg.h"
#include "scheduler.h"
#include "mantissa.h"
#include "lq_packet.h"
#include "net_olsr.h"
#include "olsr_random.h"

#include <iphlpapi.h>
#include <iprtrmib.h>

#include <arpa/inet.h>

#define BUFSPACE  (127*1024)    /* max. input buffer size to request */

struct MibIpInterfaceRow {
  USHORT Family;
  ULONG64 InterfaceLuid;
  ULONG InterfaceIndex;
  ULONG MaxReassemblySize;
  ULONG64 InterfaceIdentifier;
  ULONG MinRouterAdvertisementInterval;
  ULONG MaxRouterAdvertisementInterval;
  BOOLEAN AdvertisingEnabled;
  BOOLEAN ForwardingEnabled;
  BOOLEAN WeakHostSend;
  BOOLEAN WeakHostReceive;
  BOOLEAN UseAutomaticMetric;
  BOOLEAN UseNeighborUnreachabilityDetection;
  BOOLEAN ManagedAddressConfigurationSupported;
  BOOLEAN OtherStatefulConfigurationSupported;
  BOOLEAN AdvertiseDefaultRoute;
  INT RouterDiscoveryBehavior;
  ULONG DadTransmits;
  ULONG BaseReachableTime;
  ULONG RetransmitTime;
  ULONG PathMtuDiscoveryTimeout;
  INT LinkLocalAddressBehavior;
  ULONG LinkLocalAddressTimeout;
  ULONG ZoneIndices[16];
  ULONG SitePrefixLength;
  ULONG Metric;
  ULONG NlMtu;
  BOOLEAN Connected;
  BOOLEAN SupportsWakeUpPatterns;
  BOOLEAN SupportsNeighborDiscovery;
  BOOLEAN SupportsRouterDiscovery;
  ULONG ReachableTime;
  BYTE TransmitOffload;
  BYTE ReceiveOffload;
  BOOLEAN DisableDefaultRoutes;
};

typedef DWORD(__stdcall * GETIPINTERFACEENTRY) (struct MibIpInterfaceRow * Row);

typedef DWORD(__stdcall * GETADAPTERSADDRESSES) (ULONG Family, DWORD Flags, PVOID Reserved, PIP_ADAPTER_ADDRESSES pAdapterAddresses,
                                                 PULONG pOutBufLen);

struct InterfaceInfo {
  unsigned int Index;
  int Mtu;
  int Metric;
  unsigned int Addr;
  unsigned int Mask;
  unsigned int Broad;
  char Guid[39];
};

void WinSockPError(char *);
char *StrError(unsigned int ErrNo);

void ListInterfaces(void);
int GetIntInfo(struct InterfaceInfo *Info, char *Name);

#define MAX_INTERFACES 100

static void
MiniIndexToIntName(char *String, int MiniIndex)
{
  const char *HexDigits = "0123456789abcdef";

  String[0] = 'i';
  String[1] = 'f';

  String[2] = HexDigits[(MiniIndex >> 4) & 15];
  String[3] = HexDigits[MiniIndex & 15];

  String[4] = 0;
}

static int
IntNameToMiniIndex(int *MiniIndex, char *String)
{
  const char *HexDigits = "0123456789abcdef";
  int i, k;
  char ch;

  if ((String[0] != 'i' && String[0] != 'I') || (String[1] != 'f' && String[1] != 'F'))
    return -1;

  *MiniIndex = 0;

  for (i = 2; i < 4; i++) {
    ch = String[i];

    if (ch >= 'A' && ch <= 'F')
      ch += 32;

    for (k = 0; k < 16 && ch != HexDigits[k]; k++);

    if (k == 16)
      return -1;

    *MiniIndex = (*MiniIndex << 4) | k;
  }

  return 0;
}

static int
FriendlyNameToMiniIndex(int *MiniIndex, char *String)
{
  unsigned long BuffLen;
  unsigned long Res;
  IP_ADAPTER_ADDRESSES AdAddr[MAX_INTERFACES], *WalkerAddr;
  char FriendlyName[MAX_INTERFACE_NAME_LEN];
  HMODULE h;
  GETADAPTERSADDRESSES pfGetAdaptersAddresses;

  h = LoadLibrary("iphlpapi.dll");

  if (h == NULL) {
    fprintf(stderr, "LoadLibrary() = %08lx", GetLastError());
    return -1;
  }

  pfGetAdaptersAddresses = (GETADAPTERSADDRESSES) GetProcAddress(h, "GetAdaptersAddresses");

  if (pfGetAdaptersAddresses == NULL) {
    fprintf(stderr, "Unable to use adapter friendly name (GetProcAddress() = %08lx)\n", GetLastError());
    return -1;
  }

  BuffLen = sizeof(AdAddr);

  Res = pfGetAdaptersAddresses(AF_INET, 0, NULL, AdAddr, &BuffLen);

  if (Res != NO_ERROR) {
    fprintf(stderr, "GetAdaptersAddresses() = %08lx", GetLastError());
    return -1;
  }

  for (WalkerAddr = AdAddr; WalkerAddr != NULL; WalkerAddr = WalkerAddr->Next) {
    OLSR_PRINTF(5, "Index = %08x - ", (int)WalkerAddr->IfIndex);

    wcstombs(FriendlyName, WalkerAddr->FriendlyName, MAX_INTERFACE_NAME_LEN);

    OLSR_PRINTF(5, "Friendly name = %s\n", FriendlyName);

    if (strncmp(FriendlyName, String, MAX_INTERFACE_NAME_LEN) == 0)
      break;
  }

  if (WalkerAddr == NULL) {
    fprintf(stderr, "No such interface: %s!\n", String);
    return -1;
  }

  *MiniIndex = WalkerAddr->IfIndex & 255;

  return 0;
}

int
GetIntInfo(struct InterfaceInfo *Info, char *Name)
{
  int MiniIndex;
  unsigned char Buff[MAX_INTERFACES * sizeof(MIB_IFROW) + 4];
  MIB_IFTABLE *IfTable;
  unsigned long BuffLen;
  unsigned long Res;
  int TabIdx;
  IP_ADAPTER_INFO AdInfo[MAX_INTERFACES], *Walker;
  HMODULE Lib;
  struct MibIpInterfaceRow Row;
  GETIPINTERFACEENTRY InterfaceEntry;

  if (olsr_cnf->ip_version == AF_INET6) {
    fprintf(stderr, "IPv6 not supported by GetIntInfo()!\n");
    return -1;
  }

  if ((Name[0] != 'i' && Name[0] != 'I') || (Name[1] != 'f' && Name[1] != 'F')) {
    if (FriendlyNameToMiniIndex(&MiniIndex, Name) < 0) {
      fprintf(stderr, "No such interface: %s!\n", Name);
      return -1;
    }
  }

  else {
    if (IntNameToMiniIndex(&MiniIndex, Name) < 0) {
      fprintf(stderr, "No such interface: %s!\n", Name);
      return -1;
    }
  }

  IfTable = (MIB_IFTABLE *) Buff;

  BuffLen = sizeof(Buff);

  Res = GetIfTable(IfTable, &BuffLen, FALSE);

  if (Res != NO_ERROR) {
    fprintf(stderr, "GetIfTable() = %08lx, %s", Res, StrError(Res));
    return -1;
  }

  for (TabIdx = 0; TabIdx < (int)IfTable->dwNumEntries; TabIdx++) {
    OLSR_PRINTF(5, "Index = %08x\n", (int)IfTable->table[TabIdx].dwIndex);

    if ((int)(IfTable->table[TabIdx].dwIndex & 255) == MiniIndex)
      break;
  }

  if (TabIdx == (int)IfTable->dwNumEntries) {
    fprintf(stderr, "No such interface: %s!\n", Name);
    return -1;
  }

  Info->Index = IfTable->table[TabIdx].dwIndex;
  Info->Mtu = (int)IfTable->table[TabIdx].dwMtu;

  Info->Mtu -= (olsr_cnf->ip_version == AF_INET6) ? UDP_IPV6_HDRSIZE : UDP_IPV4_HDRSIZE;

  Lib = LoadLibrary("iphlpapi.dll");

  if (Lib == NULL) {
    fprintf(stderr, "Cannot load iphlpapi.dll: %08lx\n", GetLastError());
    return -1;
  }

  InterfaceEntry = (GETIPINTERFACEENTRY) GetProcAddress(Lib, "GetIpInterfaceEntry");

  if (InterfaceEntry == NULL) {
    OLSR_PRINTF(5, "Not running on Vista - setting interface metric to 0.\n");

    Info->Metric = 0;
  }

  else {
    memset(&Row, 0, sizeof(struct MibIpInterfaceRow));

    Row.Family = AF_INET;
    Row.InterfaceIndex = Info->Index;

    Res = InterfaceEntry(&Row);

    if (Res != NO_ERROR) {
      fprintf(stderr, "GetIpInterfaceEntry() = %08lx", Res);
      FreeLibrary(Lib);
      return -1;
    }

    Info->Metric = Row.Metric;

    OLSR_PRINTF(5, "Running on Vista - interface metric is %d.\n", Info->Metric);
  }

  FreeLibrary(Lib);

  BuffLen = sizeof(AdInfo);

  Res = GetAdaptersInfo(AdInfo, &BuffLen);

  if (Res != NO_ERROR) {
    fprintf(stderr, "GetAdaptersInfo() = %08lx, %s", GetLastError(), StrError(Res));
    return -1;
  }

  for (Walker = AdInfo; Walker != NULL; Walker = Walker->Next) {
    OLSR_PRINTF(5, "Index = %08x\n", (int)Walker->Index);

    if ((int)(Walker->Index & 255) == MiniIndex)
      break;
  }

  if (Walker == NULL) {
    fprintf(stderr, "No such interface: %s!\n", Name);
    return -1;
  }

  inet_pton(AF_INET, Walker->IpAddressList.IpAddress.String, &Info->Addr);
  inet_pton(AF_INET, Walker->IpAddressList.IpMask.String, &Info->Mask);

  Info->Broad = Info->Addr | ~Info->Mask;

  strscpy(Info->Guid, Walker->AdapterName, sizeof(Info->Guid));

  if ((IfTable->table[TabIdx].dwOperStatus != MIB_IF_OPER_STATUS_CONNECTED
       && IfTable->table[TabIdx].dwOperStatus != MIB_IF_OPER_STATUS_OPERATIONAL) || Info->Addr == 0) {
    OLSR_PRINTF(3, "Interface %s not up!\n", Name);
    return -1;
  }

  return 0;
}

#if !defined OID_802_11_CONFIGURATION
#define OID_802_11_CONFIGURATION 0x0d010211
#endif /* !defined OID_802_11_CONFIGURATION */

#if !defined IOCTL_NDIS_QUERY_GLOBAL_STATS
#define IOCTL_NDIS_QUERY_GLOBAL_STATS 0x00170002
#endif /* !defined IOCTL_NDIS_QUERY_GLOBAL_STATS */

static int
IsWireless(char *IntName)
{
#if !defined WINCE
  struct InterfaceInfo Info;
  char DevName[43];
  HANDLE DevHand;
  unsigned int ErrNo;
  unsigned int Oid;
  unsigned char OutBuff[100];
  unsigned long OutBytes;

  if (GetIntInfo(&Info, IntName) < 0)
    return -1;

  DevName[0] = '\\';
  DevName[1] = '\\';
  DevName[2] = '.';
  DevName[3] = '\\';

  strscpy(DevName + 4, Info.Guid, sizeof(DevName) - 4);

  OLSR_PRINTF(5, "Checking whether interface %s is wireless.\n", DevName);

  DevHand = CreateFile(DevName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (DevHand == INVALID_HANDLE_VALUE) {
    ErrNo = GetLastError();

    OLSR_PRINTF(5, "CreateFile() = %08x, %s\n", ErrNo, StrError(ErrNo));
    return -1;
  }

  Oid = OID_802_11_CONFIGURATION;

  if (!DeviceIoControl(DevHand, IOCTL_NDIS_QUERY_GLOBAL_STATS, &Oid, sizeof(Oid), OutBuff, sizeof(OutBuff), &OutBytes, NULL)) {
    ErrNo = GetLastError();

    CloseHandle(DevHand);

    if (ErrNo == ERROR_GEN_FAILURE || ErrNo == ERROR_INVALID_PARAMETER) {
      OLSR_PRINTF(5, "OID not supported. Device probably not wireless.\n");
      return 0;
    }

    OLSR_PRINTF(5, "DeviceIoControl() = %08x, %s\n", ErrNo, StrError(ErrNo));
    return -1;
  }

  CloseHandle(DevHand);
#endif /* !defined WINCE */
  return 1;
}

void
ListInterfaces(void)
{
  IP_ADAPTER_INFO AdInfo[MAX_INTERFACES], *Walker;
  unsigned long AdInfoLen;
  char IntName[5];
  IP_ADDR_STRING *Walker2;
  unsigned long Res;
  int IsWlan;

  if (olsr_cnf->ip_version == AF_INET6) {
    fprintf(stderr, "IPv6 not supported by ListInterfaces()!\n");
    return;
  }

  AdInfoLen = sizeof(AdInfo);

  Res = GetAdaptersInfo(AdInfo, &AdInfoLen);

  if (Res == ERROR_NO_DATA) {
    printf("No interfaces detected.\n");
    return;
  }

  if (Res != NO_ERROR) {
    fprintf(stderr, "GetAdaptersInfo() = %08lx, %s", Res, StrError(Res));
    return;
  }

  for (Walker = AdInfo; Walker != NULL; Walker = Walker->Next) {
    OLSR_PRINTF(5, "Index = %08x\n", (int)Walker->Index);

    MiniIndexToIntName(IntName, Walker->Index);

    printf("%s: ", IntName);

    IsWlan = IsWireless(IntName);

    if (IsWlan < 0)
      printf("?");

    else if (IsWlan == 0)
      printf("-");

    else
      printf("+");

    for (Walker2 = &Walker->IpAddressList; Walker2 != NULL; Walker2 = Walker2->Next)
      printf(" %s", Walker2->IpAddress.String);

    printf("\n");
  }
}

int
add_hemu_if(struct olsr_if *iface)
{
  struct interface_olsr *ifp;
  union olsr_ip_addr null_addr;
  uint32_t addr[4];
  struct ipaddr_str buf;
  size_t name_size;

  if (!iface->host_emul)
    return -1;

  ifp = olsr_malloc(sizeof(struct interface_olsr), "Interface update 2");

  memset(ifp, 0, sizeof(struct interface_olsr));

  /* initialize backpointer */
  ifp->olsr_if = iface;

  iface->configured = true;
  iface->interf = ifp;

  name_size = strlen("hcif01") + 1;
  ifp->is_hcif = true;
  ifp->int_name = olsr_malloc(name_size, "Interface update 3");
  ifp->int_metric = 0;

  strscpy(ifp->int_name, "hcif01", name_size);

  OLSR_PRINTF(1, "Adding %s(host emulation):\n", ifp->int_name);

  OLSR_PRINTF(1, "       Address:%s\n", olsr_ip_to_string(&buf, &iface->hemu_ip));

  OLSR_PRINTF(1, "       NB! This is a emulated interface\n       that does not exist in the kernel!\n");

  ifp->int_next = ifnet;
  ifnet = ifp;

  memset(&null_addr, 0, olsr_cnf->ipsize);
  if (ipequal(&null_addr, &olsr_cnf->main_addr)) {
    olsr_cnf->main_addr = iface->hemu_ip;
    olsr_cnf->unicast_src_ip = iface->hemu_ip;
    OLSR_PRINTF(1, "New main address: %s\n", olsr_ip_to_string(&buf, &olsr_cnf->main_addr));
  }

  ifp->int_mtu = OLSR_DEFAULT_MTU;

  ifp->int_mtu -= (olsr_cnf->ip_version == AF_INET6) ? UDP_IPV6_HDRSIZE : UDP_IPV4_HDRSIZE;

  /* Set up buffer */
  net_add_buffer(ifp);

  if (olsr_cnf->ip_version == AF_INET) {
    struct sockaddr_in sin;

    memset(&sin, 0, sizeof(sin));

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    sin.sin_port = htons(10150);

    /* IP version 4 */
    ifp->ip_addr.v4 = iface->hemu_ip.v4;

    memcpy(&((struct sockaddr_in *)&ifp->int_addr)->sin_addr, &iface->hemu_ip, olsr_cnf->ipsize);

    /*
     *We create one socket for each interface and bind
     *the socket to it. This to ensure that we can control
     *on what interface the message is transmitted
     */

    ifp->olsr_socket = gethemusocket(&sin);

    if (ifp->olsr_socket < 0) {
      olsr_exit("Could not initialize socket", EXIT_FAILURE);
    }

  } else {
    /* IP version 6 */
    memcpy(&ifp->ip_addr, &iface->hemu_ip, olsr_cnf->ipsize);
  }

  /* Send IP as first 4/16 bytes on socket */
  memcpy(addr, iface->hemu_ip.v6.s6_addr, olsr_cnf->ipsize);
  addr[0] = htonl(addr[0]);
  addr[1] = htonl(addr[1]);
  addr[2] = htonl(addr[2]);
  addr[3] = htonl(addr[3]);

  if (send(ifp->olsr_socket, (char *)addr, olsr_cnf->ipsize, 0) != (int)olsr_cnf->ipsize) {
    fprintf(stderr, "Error sending IP!");
  }

  /* Register socket */
  add_olsr_socket(ifp->olsr_socket, &olsr_input_hostemu, NULL, NULL, SP_PR_READ);

  /*
   * Register functions for periodic message generation
   */
  ifp->hello_gen_timer =
    olsr_start_timer(iface->cnf->hello_params.emission_interval * MSEC_PER_SEC, HELLO_JITTER, OLSR_TIMER_PERIODIC,
                     olsr_cnf->lq_level == 0 ? &generate_hello : &olsr_output_lq_hello, ifp, hello_gen_timer_cookie);
  ifp->tc_gen_timer =
    olsr_start_timer(iface->cnf->tc_params.emission_interval * MSEC_PER_SEC, TC_JITTER, OLSR_TIMER_PERIODIC,
                     olsr_cnf->lq_level == 0 ? &generate_tc : &olsr_output_lq_tc, ifp, tc_gen_timer_cookie);
  ifp->mid_gen_timer =
    olsr_start_timer(iface->cnf->mid_params.emission_interval * MSEC_PER_SEC, MID_JITTER, OLSR_TIMER_PERIODIC, &generate_mid, ifp,
                     mid_gen_timer_cookie);
  ifp->hna_gen_timer =
    olsr_start_timer(iface->cnf->hna_params.emission_interval * MSEC_PER_SEC, HNA_JITTER, OLSR_TIMER_PERIODIC, &generate_hna, ifp,
                     hna_gen_timer_cookie);

  /* Recalculate max topology hold time */
  if (olsr_cnf->max_tc_vtime < iface->cnf->tc_params.emission_interval)
    olsr_cnf->max_tc_vtime = iface->cnf->tc_params.emission_interval;

  ifp->hello_etime = (olsr_reltime) (iface->cnf->hello_params.emission_interval * MSEC_PER_SEC);
  ifp->valtimes.hello = reltime_to_me(iface->cnf->hello_params.validity_time * MSEC_PER_SEC);
  ifp->valtimes.tc = reltime_to_me(iface->cnf->tc_params.validity_time * MSEC_PER_SEC);
  ifp->valtimes.mid = reltime_to_me(iface->cnf->mid_params.validity_time * MSEC_PER_SEC);
  ifp->valtimes.hna = reltime_to_me(iface->cnf->hna_params.validity_time * MSEC_PER_SEC);

  ifp->mode = iface->cnf->mode;

  return 1;
}

int
chk_if_changed(struct olsr_if *iface)
{
  struct ipaddr_str buf;
  struct interface_olsr *Int;
  struct InterfaceInfo Info;
  int Res;
  int IsWlan;
  union olsr_ip_addr OldVal, NewVal;
  struct sockaddr_in *AddrIn;

  if (olsr_cnf->ip_version == AF_INET6) {
    fprintf(stderr, "IPv6 not supported by chk_if_changed()!\n");
    return 0;
  }
#ifdef DEBUG
  OLSR_PRINTF(3, "Checking if %s is set down or changed\n", iface->name);
#endif /* DEBUG */

  Int = iface->interf;

  if (GetIntInfo(&Info, iface->name) < 0) {
    olsr_remove_interface(iface);
    return 1;
  }

  Res = 0;

  IsWlan = IsWireless(iface->name);

  if (IsWlan < 0)
    IsWlan = 1;

  if (Int->is_wireless != IsWlan) {
    OLSR_PRINTF(1, "\tLAN/WLAN change: %d -> %d.\n", Int->is_wireless, IsWlan);

    Int->is_wireless = IsWlan;

    if (iface->cnf->weight.fixed)
      Int->int_metric = iface->cnf->weight.value;

    else
      Int->int_metric = Info.Metric;

    Res = 1;
  }

  if (Int->int_mtu != Info.Mtu) {
    OLSR_PRINTF(1, "\tMTU change: %d -> %d.\n", (int)Int->int_mtu, Info.Mtu);

    Int->int_mtu = Info.Mtu;

    net_remove_buffer(Int);
    net_add_buffer(Int);

    Res = 1;
  }

  OldVal.v4 = ((struct sockaddr_in *)&Int->int_addr)->sin_addr;
  NewVal.v4.s_addr = Info.Addr;

#ifdef DEBUG
  OLSR_PRINTF(3, "\tAddress: %s\n", olsr_ip_to_string(&buf, &NewVal));
#endif /* DEBUG */

  if (NewVal.v4.s_addr != OldVal.v4.s_addr) {
    OLSR_PRINTF(1, "\tAddress change.\n");
    OLSR_PRINTF(1, "\tOld: %s\n", olsr_ip_to_string(&buf, &OldVal));
    OLSR_PRINTF(1, "\tNew: %s\n", olsr_ip_to_string(&buf, &NewVal));

    Int->ip_addr.v4 = NewVal.v4;

    AddrIn = (struct sockaddr_in *)&Int->int_addr;

    AddrIn->sin_family = AF_INET;
    AddrIn->sin_port = 0;
    AddrIn->sin_addr = NewVal.v4;

    if (olsr_cnf->main_addr.v4.s_addr == OldVal.v4.s_addr) {
      OLSR_PRINTF(1, "\tMain address change.\n");

      olsr_cnf->main_addr.v4 = NewVal.v4;
    }

    Res = 1;
  }

  else
    OLSR_PRINTF(3, "\tNo address change.\n");

  OldVal.v4 = ((struct sockaddr_in *)&Int->int_netmask)->sin_addr;
  NewVal.v4.s_addr = Info.Mask;

#ifdef DEBUG
  OLSR_PRINTF(3, "\tNetmask: %s\n", olsr_ip_to_string(&buf, &NewVal));
#endif /* DEBUG */

  if (NewVal.v4.s_addr != OldVal.v4.s_addr) {
    OLSR_PRINTF(1, "\tNetmask change.\n");
    OLSR_PRINTF(1, "\tOld: %s\n", olsr_ip_to_string(&buf, &OldVal));
    OLSR_PRINTF(1, "\tNew: %s\n", olsr_ip_to_string(&buf, &NewVal));

    AddrIn = (struct sockaddr_in *)&Int->int_netmask;

    AddrIn->sin_family = AF_INET;
    AddrIn->sin_port = 0;
    AddrIn->sin_addr = NewVal.v4;

    Res = 1;
  }

  else
    OLSR_PRINTF(3, "\tNo netmask change.\n");

  OldVal.v4 = ((struct sockaddr_in *)&Int->int_broadaddr)->sin_addr;
  NewVal.v4.s_addr = Info.Broad;

#ifdef DEBUG
  OLSR_PRINTF(3, "\tBroadcast address: %s\n", olsr_ip_to_string(&buf, &NewVal));
#endif /* DEBUG */

  if (NewVal.v4.s_addr != OldVal.v4.s_addr) {
    OLSR_PRINTF(1, "\tBroadcast address change.\n");
    OLSR_PRINTF(1, "\tOld: %s\n", olsr_ip_to_string(&buf, &OldVal));
    OLSR_PRINTF(1, "\tNew: %s\n", olsr_ip_to_string(&buf, &NewVal));

    AddrIn = (struct sockaddr_in *)&Int->int_broadaddr;

    AddrIn->sin_family = AF_INET;
    AddrIn->sin_port = 0;
    AddrIn->sin_addr = NewVal.v4;

    Res = 1;
  }

  else
    OLSR_PRINTF(3, "\tNo broadcast address change.\n");

  if (Res != 0)
    olsr_trigger_ifchange(Int->if_index, Int, IFCHG_IF_UPDATE);

  return Res;
}

int
chk_if_up(struct olsr_if *iface, int debuglvl __attribute__ ((unused)))
{
  struct ipaddr_str buf;
  struct InterfaceInfo Info;
  struct interface_olsr *New;
  union olsr_ip_addr NullAddr;
  int IsWlan;
  struct sockaddr_in *AddrIn;
  size_t name_size;

  if (olsr_cnf->ip_version == AF_INET6) {
    fprintf(stderr, "IPv6 not supported by chk_if_up()!\n");
    return 0;
  }

  if (GetIntInfo(&Info, iface->name) < 0)
    return 0;

  New = olsr_malloc(sizeof(struct interface_olsr), "Interface 1");
  /* initialize backpointer */
  New->olsr_if = iface;


  New->immediate_send_tc = (iface->cnf->tc_params.emission_interval < iface->cnf->hello_params.emission_interval);
  if (olsr_cnf->max_jitter == 0) {
    /* max_jitter determines the max time to store to-be-send-messages, correlated with random() */
    olsr_cnf->max_jitter =
      New->immediate_send_tc ? iface->cnf->tc_params.emission_interval : iface->cnf->hello_params.emission_interval;
  }

  New->gen_properties = NULL;

  AddrIn = (struct sockaddr_in *)&New->int_addr;

  AddrIn->sin_family = AF_INET;
  AddrIn->sin_port = 0;
  AddrIn->sin_addr.s_addr = Info.Addr;

  AddrIn = (struct sockaddr_in *)&New->int_netmask;

  AddrIn->sin_family = AF_INET;
  AddrIn->sin_port = 0;
  AddrIn->sin_addr.s_addr = Info.Mask;

  AddrIn = (struct sockaddr_in *)&New->int_broadaddr;

  AddrIn->sin_family = AF_INET;
  AddrIn->sin_port = 0;
  AddrIn->sin_addr.s_addr = Info.Broad;

  if (iface->cnf->ipv4_multicast.v4.s_addr != 0)
    AddrIn->sin_addr = iface->cnf->ipv4_multicast.v4;

  New->int_flags = 0;

  New->is_hcif = false;

  New->int_mtu = Info.Mtu;

  name_size = strlen(iface->name) + 1;
  New->int_name = olsr_malloc(name_size, "Interface 2");
  strscpy(New->int_name, iface->name, name_size);

  IsWlan = IsWireless(iface->name);

  if (IsWlan < 0)
    IsWlan = 1;

  New->is_wireless = IsWlan;

  if (iface->cnf->weight.fixed)
    New->int_metric = iface->cnf->weight.value;

  else
    New->int_metric = Info.Metric;

  New->olsr_seqnum = olsr_random() & 0xffff;

  New->ttl_index = -32;         /* For the first 32 TC's, fish-eye is disabled */

  OLSR_PRINTF(1, "\tInterface %s set up for use with index %d\n\n", iface->name, New->if_index);

  OLSR_PRINTF(1, "\tMTU: %d\n", New->int_mtu);
  OLSR_PRINTF(1, "\tAddress: %s\n", sockaddr4_to_string(&buf, (const struct sockaddr *)&New->int_addr));
  OLSR_PRINTF(1, "\tNetmask: %s\n", sockaddr4_to_string(&buf, (const struct sockaddr *)&New->int_netmask));
  OLSR_PRINTF(1, "\tBroadcast address: %s\n", sockaddr4_to_string(&buf, (const struct sockaddr *)&New->int_broadaddr));

  New->ip_addr.v4 = New->int_addr.sin_addr;

  New->if_index = Info.Index;

  OLSR_PRINTF(3, "\tKernel index: %08x\n", New->if_index);

  New->olsr_socket = getsocket(BUFSPACE, New);
  New->send_socket = getsocket(0, New);

  if (New->olsr_socket < 0) {
    olsr_exit("Could not initialize socket", EXIT_FAILURE);
  }

  add_olsr_socket(New->olsr_socket, &olsr_input, NULL, NULL, SP_PR_READ);

  New->int_next = ifnet;
  ifnet = New;

  iface->interf = New;
  iface->configured = 1;

  memset(&NullAddr, 0, olsr_cnf->ipsize);

  if (ipequal(&NullAddr, &olsr_cnf->main_addr)) {
    olsr_cnf->main_addr = New->ip_addr;
    olsr_cnf->unicast_src_ip = New->ip_addr;
    OLSR_PRINTF(1, "New main address: %s\n", olsr_ip_to_string(&buf, &olsr_cnf->main_addr));
  }

  net_add_buffer(New);

  /*
   * Register functions for periodic message generation
   */
  New->hello_gen_timer =
    olsr_start_timer(iface->cnf->hello_params.emission_interval * MSEC_PER_SEC, HELLO_JITTER, OLSR_TIMER_PERIODIC,
                     olsr_cnf->lq_level == 0 ? &generate_hello : &olsr_output_lq_hello, New, hello_gen_timer_cookie);
  New->tc_gen_timer =
    olsr_start_timer(iface->cnf->tc_params.emission_interval * MSEC_PER_SEC, TC_JITTER, OLSR_TIMER_PERIODIC,
                     olsr_cnf->lq_level == 0 ? &generate_tc : &olsr_output_lq_tc, New, tc_gen_timer_cookie);
  New->mid_gen_timer =
    olsr_start_timer(iface->cnf->mid_params.emission_interval * MSEC_PER_SEC, MID_JITTER, OLSR_TIMER_PERIODIC, &generate_mid, New,
                     mid_gen_timer_cookie);
  New->hna_gen_timer =
    olsr_start_timer(iface->cnf->hna_params.emission_interval * MSEC_PER_SEC, HNA_JITTER, OLSR_TIMER_PERIODIC, &generate_hna, New,
                     hna_gen_timer_cookie);

  if (olsr_cnf->max_tc_vtime < iface->cnf->tc_params.emission_interval)
    olsr_cnf->max_tc_vtime = iface->cnf->tc_params.emission_interval;

  New->hello_etime = (olsr_reltime) (iface->cnf->hello_params.emission_interval * MSEC_PER_SEC);
  New->valtimes.hello = reltime_to_me(iface->cnf->hello_params.validity_time * MSEC_PER_SEC);
  New->valtimes.tc = reltime_to_me(iface->cnf->tc_params.validity_time * MSEC_PER_SEC);
  New->valtimes.mid = reltime_to_me(iface->cnf->mid_params.validity_time * MSEC_PER_SEC);
  New->valtimes.hna = reltime_to_me(iface->cnf->hna_params.validity_time * MSEC_PER_SEC);

  New->mode = iface->cnf->mode;

  olsr_trigger_ifchange(New->if_index, New, IFCHG_IF_ADD);

  return 1;
}

void
check_interface_updates(void *dummy __attribute__ ((unused)))
{
  struct olsr_if *IntConf;

#ifdef DEBUG
  OLSR_PRINTF(3, "Checking for updates in the interface set\n");
#endif /* DEBUG */

  for (IntConf = olsr_cnf->interfaces; IntConf != NULL; IntConf = IntConf->next) {
    if (IntConf->host_emul)
      continue;

    if (olsr_cnf->host_emul)    /* XXX: TEMPORARY! */
      continue;

    if (IntConf->configured)
      chk_if_changed(IntConf);

    else
      chk_if_up(IntConf, 3);
  }
}

#endif /* _WIN32 */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
