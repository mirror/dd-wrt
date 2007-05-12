/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 * Copyright (c) 2004, Thomas Lopatic (thomas@lopatic.de)
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
 * $Id: ifnet.c,v 1.37 2007/05/01 21:50:41 bernd67 Exp $
 */

#include "interfaces.h"
#include "olsr.h"
#include "parser.h"
#include "socket_parser.h"
#include "defs.h"
#include "net_os.h"
#include "ifnet.h"
#include "generate_msg.h"
#include "scheduler.h"
#include "mantissa.h"
#include "lq_packet.h"

#include <iphlpapi.h>
#include <iprtrmib.h>

struct MibIpInterfaceRow
{
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

typedef DWORD (__stdcall *GETIPINTERFACEENTRY)
  (struct MibIpInterfaceRow *Row);

typedef DWORD (__stdcall *GETADAPTERSADDRESSES)
  (ULONG Family, DWORD Flags, PVOID Reserved,
   PIP_ADAPTER_ADDRESSES pAdapterAddresses, PULONG pOutBufLen);

struct InterfaceInfo
{
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
int inet_pton(int af, const char *src, void *dst);

void ListInterfaces(void);
int GetIntInfo(struct InterfaceInfo *Info, char *Name);
void RemoveInterface(struct olsr_if *IntConf);

#define MAX_INTERFACES 100

int __stdcall SignalHandler(unsigned long Signal);

static unsigned long __stdcall SignalHandlerWrapper(void *Dummy __attribute__((unused)))
{
  SignalHandler(0);
  return 0;
}

static void CallSignalHandler(void)
{
  unsigned long ThreadId;

  CreateThread(NULL, 0, SignalHandlerWrapper, NULL, 0, &ThreadId);
}

static void MiniIndexToIntName(char *String, int MiniIndex)
{
  char *HexDigits = "0123456789abcdef";

  String[0] = 'i';
  String[1] = 'f';

  String[2] = HexDigits[(MiniIndex >> 4) & 15];
  String[3] = HexDigits[MiniIndex & 15];

  String[4] = 0;
}

static int IntNameToMiniIndex(int *MiniIndex, char *String)
{
  char *HexDigits = "0123456789abcdef";
  int i, k;
  char ch;

  if ((String[0] != 'i' && String[0] != 'I') ||
      (String[1] != 'f' && String[1] != 'F'))
    return -1;

  *MiniIndex = 0;

  for (i = 2; i < 4; i++)
  {
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

static int FriendlyNameToMiniIndex(int *MiniIndex, char *String)
{
  unsigned long BuffLen;
  unsigned long Res;
  IP_ADAPTER_ADDRESSES AdAddr[MAX_INTERFACES], *WalkerAddr;
  char FriendlyName[MAX_INTERFACE_NAME_LEN];
  HMODULE h;
  GETADAPTERSADDRESSES pfGetAdaptersAddresses;
  
  h = LoadLibrary("iphlpapi.dll");

  if (h == NULL)
  {
    fprintf(stderr, "LoadLibrary() = %08lx", GetLastError());
    return -1;
  }

  pfGetAdaptersAddresses = (GETADAPTERSADDRESSES) GetProcAddress(h, "GetAdaptersAddresses");

  if (pfGetAdaptersAddresses == NULL)
  {
    fprintf(stderr, "Unable to use adapter friendly name (GetProcAddress() = %08lx)\n", GetLastError());
    return -1;
  }
  
  BuffLen = sizeof (AdAddr);

  Res = pfGetAdaptersAddresses(AF_INET, 0, NULL, AdAddr, &BuffLen);

  if (Res != NO_ERROR)
  {  
    fprintf(stderr, "GetAdaptersAddresses() = %08lx", GetLastError());
    return -1;
  }

  for (WalkerAddr = AdAddr; WalkerAddr != NULL; WalkerAddr = WalkerAddr->Next)
  {
    OLSR_PRINTF(5, "Index = %08x - ", (int)WalkerAddr->IfIndex);

    wcstombs(FriendlyName, WalkerAddr->FriendlyName, MAX_INTERFACE_NAME_LEN); 

    OLSR_PRINTF(5, "Friendly name = %s\n", FriendlyName);

    if (strncmp(FriendlyName, String, MAX_INTERFACE_NAME_LEN) == 0)
      break;
  }

  if (WalkerAddr == NULL)
  {
    fprintf(stderr, "No such interface: %s!\n", String);
    return -1;
  }

  *MiniIndex = WalkerAddr->IfIndex & 255;
}

int GetIntInfo(struct InterfaceInfo *Info, char *Name)
{
  int MiniIndex;
  unsigned char Buff[MAX_INTERFACES * sizeof (MIB_IFROW) + 4];
  MIB_IFTABLE *IfTable;
  unsigned long BuffLen;
  unsigned long Res;
  int TabIdx;
  IP_ADAPTER_INFO AdInfo[MAX_INTERFACES], *Walker;
  HMODULE Lib;
  struct MibIpInterfaceRow Row;
  GETIPINTERFACEENTRY InterfaceEntry;

  if (olsr_cnf->ip_version == AF_INET6)
  {
    fprintf(stderr, "IPv6 not supported by GetIntInfo()!\n");
    return -1;
  }

  if ((Name[0] != 'i' && Name[0] != 'I') ||
      (Name[1] != 'f' && Name[1] != 'F'))
  {
    if (FriendlyNameToMiniIndex(&MiniIndex, Name) < 0)
    {
      fprintf(stderr, "No such interface: %s!\n", Name);
      return -1;
    }
  }

  else
  {
    if (IntNameToMiniIndex(&MiniIndex, Name) < 0)
    {
      fprintf(stderr, "No such interface: %s!\n", Name);
      return -1;
    }
  }

  IfTable = (MIB_IFTABLE *)Buff;

  BuffLen = sizeof (Buff);

  Res = GetIfTable(IfTable, &BuffLen, FALSE);

  if (Res != NO_ERROR)
  {
    fprintf(stderr, "GetIfTable() = %08lx, %s", Res, StrError(Res));
    return -1;
  }

  for (TabIdx = 0; TabIdx < (int)IfTable->dwNumEntries; TabIdx++)
  {
    OLSR_PRINTF(5, "Index = %08x\n", (int)IfTable->table[TabIdx].dwIndex);

    if ((int)(IfTable->table[TabIdx].dwIndex & 255) == MiniIndex)
      break;
  }

  if (TabIdx == (int)IfTable->dwNumEntries)
  {
    fprintf(stderr, "No such interface: %s!\n", Name);
    return -1;
  }
    
  Info->Index = IfTable->table[TabIdx].dwIndex;
  Info->Mtu = (int)IfTable->table[TabIdx].dwMtu;

  Info->Mtu -= (olsr_cnf->ip_version == AF_INET6) ?
    UDP_IPV6_HDRSIZE : UDP_IPV4_HDRSIZE;

  Lib = LoadLibrary("iphlpapi.dll");

  if (Lib == NULL)
  {
    fprintf(stderr, "Cannot load iphlpapi.dll: %08lx\n", GetLastError());
    return -1;
  }

  InterfaceEntry = (GETIPINTERFACEENTRY)GetProcAddress(Lib, "GetIpInterfaceEntry");

  if (InterfaceEntry == NULL)
  {
    OLSR_PRINTF(5, "Not running on Vista - setting interface metric to 0.\n");

    Info->Metric = 0;
  }

  else
  {
    memset(&Row, 0, sizeof (struct MibIpInterfaceRow));

    Row.Family = AF_INET;
    Row.InterfaceIndex = Info->Index;

    Res = InterfaceEntry(&Row);

    if (Res != NO_ERROR)
    {
      fprintf(stderr, "GetIpInterfaceEntry() = %08lx", Res);
      FreeLibrary(Lib);
      return -1;
    }

    Info->Metric = Row.Metric;

    OLSR_PRINTF(5, "Running on Vista - interface metric is %d.\n", Info->Metric);
  }

  FreeLibrary(Lib);

  BuffLen = sizeof (AdInfo);

  Res = GetAdaptersInfo(AdInfo, &BuffLen);

  if (Res != NO_ERROR)
  {
    fprintf(stderr, "GetAdaptersInfo() = %08lx, %s", GetLastError(),
            StrError(Res));
    return -1;
  }

  for (Walker = AdInfo; Walker != NULL; Walker = Walker->Next)
  {
    OLSR_PRINTF(5, "Index = %08x\n", (int)Walker->Index);

    if ((int)(Walker->Index & 255) == MiniIndex)
      break;
  }

  if (Walker == NULL)
  {
    fprintf(stderr, "No such interface: %s!\n", Name);
    return -1;
  }

  inet_pton(AF_INET, Walker->IpAddressList.IpAddress.String, &Info->Addr);
  inet_pton(AF_INET, Walker->IpAddressList.IpMask.String, &Info->Mask);

  Info->Broad = Info->Addr | ~Info->Mask;

  strcpy(Info->Guid, Walker->AdapterName);

  if ((IfTable->table[TabIdx].dwOperStatus != MIB_IF_OPER_STATUS_CONNECTED &&
      IfTable->table[TabIdx].dwOperStatus != MIB_IF_OPER_STATUS_OPERATIONAL) ||
      Info->Addr == 0)
  {
    OLSR_PRINTF(3, "Interface %s not up!\n", Name);
    return -1;
  }

  return 0;
}

#if !defined OID_802_11_CONFIGURATION
#define OID_802_11_CONFIGURATION 0x0d010211
#endif

#if !defined IOCTL_NDIS_QUERY_GLOBAL_STATS
#define IOCTL_NDIS_QUERY_GLOBAL_STATS 0x00170002
#endif

static int IsWireless(char *IntName)
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

  strcpy(DevName + 4, Info.Guid);

  OLSR_PRINTF(5, "Checking whether interface %s is wireless.\n", DevName);

  DevHand = CreateFile(DevName, GENERIC_READ,
                       FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL, NULL);

  if (DevHand == INVALID_HANDLE_VALUE)
  {
    ErrNo = GetLastError();

    OLSR_PRINTF(5, "CreateFile() = %08x, %s\n", ErrNo, StrError(ErrNo));
    return -1;
  }

  Oid = OID_802_11_CONFIGURATION;

  if (!DeviceIoControl(DevHand, IOCTL_NDIS_QUERY_GLOBAL_STATS,
                       &Oid, sizeof (Oid),
                       OutBuff, sizeof (OutBuff),
                       &OutBytes, NULL))
  {
    ErrNo = GetLastError();

    CloseHandle(DevHand);

    if (ErrNo == ERROR_GEN_FAILURE || ErrNo == ERROR_INVALID_PARAMETER)
    {
      OLSR_PRINTF(5, "OID not supported. Device probably not wireless.\n");
      return 0;
    }

    OLSR_PRINTF(5, "DeviceIoControl() = %08x, %s\n", ErrNo, StrError(ErrNo));
    return -1;
  }

  CloseHandle(DevHand);
#endif
  return 1;
}

void ListInterfaces(void)
{
  IP_ADAPTER_INFO AdInfo[MAX_INTERFACES], *Walker;
  unsigned long AdInfoLen;
  char IntName[5];
  IP_ADDR_STRING *Walker2;
  unsigned long Res;
  int IsWlan;
  
  if (olsr_cnf->ip_version == AF_INET6)
  {
    fprintf(stderr, "IPv6 not supported by ListInterfaces()!\n");
    return;
  }

  AdInfoLen = sizeof (AdInfo);

  Res = GetAdaptersInfo(AdInfo, &AdInfoLen);

  if (Res == ERROR_NO_DATA)
  {
    printf("No interfaces detected.\n");
    return;
  }
  
  if (Res != NO_ERROR)
  {
    fprintf(stderr, "GetAdaptersInfo() = %08lx, %s", Res, StrError(Res));
    return;
  }

  for (Walker = AdInfo; Walker != NULL; Walker = Walker->Next)
  {
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

    for (Walker2 = &Walker->IpAddressList; Walker2 != NULL;
         Walker2 = Walker2->Next)
      printf(" %s", Walker2->IpAddress.String);

    printf("\n");
  }
}

void RemoveInterface(struct olsr_if *IntConf)
{
  struct interface *Int, *Prev;

  OLSR_PRINTF(1, "Removing interface %s.\n", IntConf->name);
  
  Int = IntConf->interf;

  run_ifchg_cbs(Int, IFCHG_IF_ADD);

  if (Int == ifnet)
    ifnet = Int->int_next;

  else
  {
    for (Prev = ifnet; Prev->int_next != Int; Prev = Prev->int_next);

    Prev->int_next = Int->int_next;
  }

  if(COMP_IP(&olsr_cnf->main_addr, &Int->ip_addr))
  {
    if(ifnet == NULL)
    {
      memset(&olsr_cnf->main_addr, 0, olsr_cnf->ipsize);
      OLSR_PRINTF(1, "Removed last interface. Cleared main address.\n");
    }

    else
    {
      COPY_IP(&olsr_cnf->main_addr, &ifnet->ip_addr);
      OLSR_PRINTF(1, "New main address: %s.\n", olsr_ip_to_string(&olsr_cnf->main_addr));
    }
  }

  if (olsr_cnf->lq_level == 0)
    {
      olsr_remove_scheduler_event(&generate_hello, Int,
                                  IntConf->cnf->hello_params.emission_interval,
                                  0, NULL);

      olsr_remove_scheduler_event(&generate_tc, Int,
                                  IntConf->cnf->tc_params.emission_interval,
                                  0, NULL);
    }

  else
    {
      olsr_remove_scheduler_event(&olsr_output_lq_hello, Int,
                                  IntConf->cnf->hello_params.emission_interval,
                                  0, NULL);

      olsr_remove_scheduler_event(&olsr_output_lq_tc, Int,
                                  IntConf->cnf->tc_params.emission_interval,
                                  0, NULL);
    }

  olsr_remove_scheduler_event(&generate_mid, Int,
                              IntConf->cnf->mid_params.emission_interval,
                              0, NULL);

  olsr_remove_scheduler_event(&generate_hna, Int,
                              IntConf->cnf->hna_params.emission_interval,
                              0, NULL);

  net_remove_buffer(Int);

  IntConf->configured = 0;
  IntConf->interf = NULL;

  closesocket(Int->olsr_socket);
  remove_olsr_socket(Int->olsr_socket, &olsr_input);

  free(Int->int_name);
  free(Int);

  if (ifnet == NULL && !olsr_cnf->allow_no_interfaces)
  {
    OLSR_PRINTF(1, "No more active interfaces - exiting.\n");
    olsr_cnf->exit_value = EXIT_FAILURE;
    CallSignalHandler();
  }
}

int add_hemu_if(struct olsr_if *iface)
{
  struct interface *ifp;
  union olsr_ip_addr null_addr;
  olsr_u32_t addr[4];

  if(!iface->host_emul)
    return -1;

  ifp = olsr_malloc(sizeof (struct interface), "Interface update 2");

  memset(ifp, 0, sizeof (struct interface));

  iface->configured = OLSR_TRUE;
  iface->interf = ifp;

  ifp->is_hcif = OLSR_TRUE;
  ifp->int_name = olsr_malloc(strlen("hcif01") + 1, "Interface update 3");
  ifp->int_metric = 0;

  strcpy(ifp->int_name, "hcif01");

  OLSR_PRINTF(1, "Adding %s(host emulation):\n", ifp->int_name);

  OLSR_PRINTF(1, "       Address:%s\n", olsr_ip_to_string(&iface->hemu_ip));

  OLSR_PRINTF(1, "       Index:%d\n", iface->index);

  OLSR_PRINTF(1, "       NB! This is a emulated interface\n       that does not exist in the kernel!\n");

  ifp->int_next = ifnet;
  ifnet = ifp;

  memset(&null_addr, 0, olsr_cnf->ipsize);
  if(COMP_IP(&null_addr, &olsr_cnf->main_addr))
    {
      COPY_IP(&olsr_cnf->main_addr, &iface->hemu_ip);
      OLSR_PRINTF(1, "New main address: %s\n", olsr_ip_to_string(&olsr_cnf->main_addr));
    }

  /* setting the interfaces number*/
  ifp->if_nr = iface->index;

  ifp->int_mtu = OLSR_DEFAULT_MTU;

  ifp->int_mtu -= (olsr_cnf->ip_version == AF_INET6) ?
    UDP_IPV6_HDRSIZE : UDP_IPV4_HDRSIZE;

  /* Set up buffer */
  net_add_buffer(ifp);


  if(olsr_cnf->ip_version == AF_INET)
    {
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
      
      if (ifp->olsr_socket < 0)
	{
	  fprintf(stderr, "Could not initialize socket... exiting!\n\n");
	  exit(1);
	}

    }
  else
    {
      /* IP version 6 */
      memcpy(&ifp->ip_addr, &iface->hemu_ip, olsr_cnf->ipsize);

#if 0      
      /*
       *We create one socket for each interface and bind
       *the socket to it. This to ensure that we can control
       *on what interface the message is transmitted
       */
      
      ifp->olsr_socket = gethcsocket6(&addrsock6, bufspace, ifp->int_name);
      
      join_mcast(ifp, ifp->olsr_socket);
      
      if (ifp->olsr_socket < 0)
	{
	  fprintf(stderr, "Could not initialize socket... exiting!\n\n");
	  exit(1);
	}
      
#endif
    }

  /* Send IP as first 4/16 bytes on socket */
  memcpy(addr, iface->hemu_ip.v6.s6_addr, olsr_cnf->ipsize);
  addr[0] = htonl(addr[0]);
  addr[1] = htonl(addr[1]);
  addr[2] = htonl(addr[2]);
  addr[3] = htonl(addr[3]);

  if(send(ifp->olsr_socket, (char *)addr, olsr_cnf->ipsize, 0) != (int)olsr_cnf->ipsize)
    {
      fprintf(stderr, "Error sending IP!");
    }  
  
  /* Register socket */
  add_olsr_socket(ifp->olsr_socket, &olsr_input_hostemu);


  if (olsr_cnf->lq_level == 0)
    {
      olsr_register_scheduler_event(&generate_hello, 
                                    ifp, 
                                    iface->cnf->hello_params.emission_interval, 
                                    0, 
                                    NULL);
      olsr_register_scheduler_event(&generate_tc, 
                                    ifp, 
                                    iface->cnf->tc_params.emission_interval,
                                    0, 
                                    NULL);
    }

  else
    {
      olsr_register_scheduler_event(&olsr_output_lq_hello, 
                                    ifp, 
                                    iface->cnf->hello_params.emission_interval, 
                                    0, 
                                    NULL);
      olsr_register_scheduler_event(&olsr_output_lq_tc, 
                                    ifp, 
                                    iface->cnf->tc_params.emission_interval,
                                    0, 
                                    NULL);
    }

  olsr_register_scheduler_event(&generate_mid, 
				ifp, 
				iface->cnf->mid_params.emission_interval,
				0, 
				NULL);
  olsr_register_scheduler_event(&generate_hna, 
				ifp, 
				iface->cnf->hna_params.emission_interval,
				0, 
				NULL);

  /* Recalculate max jitter */

  if((olsr_cnf->max_jitter == 0) || 
     ((iface->cnf->hello_params.emission_interval / 4) < olsr_cnf->max_jitter))
    olsr_cnf->max_jitter = iface->cnf->hello_params.emission_interval / 4;

  /* Recalculate max topology hold time */
  if(olsr_cnf->max_tc_vtime < iface->cnf->tc_params.emission_interval)
    olsr_cnf->max_tc_vtime = iface->cnf->tc_params.emission_interval;

  ifp->hello_etime = iface->cnf->hello_params.emission_interval;
  ifp->valtimes.hello = double_to_me(iface->cnf->hello_params.validity_time);
  ifp->valtimes.tc = double_to_me(iface->cnf->tc_params.validity_time);
  ifp->valtimes.mid = double_to_me(iface->cnf->mid_params.validity_time);
  ifp->valtimes.hna = double_to_me(iface->cnf->hna_params.validity_time);

  return 1;
}

int chk_if_changed(struct olsr_if *IntConf)
{
  struct interface *Int;
  struct InterfaceInfo Info;
  int Res;
  int IsWlan;
  union olsr_ip_addr OldVal, NewVal;
  struct sockaddr_in *AddrIn;

  if (olsr_cnf->ip_version == AF_INET6)
  {
    fprintf(stderr, "IPv6 not supported by chk_if_changed()!\n");
    return 0;
  }

#ifdef DEBUG
  OLSR_PRINTF(3, "Checking if %s is set down or changed\n", IntConf->name);
#endif

  Int = IntConf->interf;

  if (GetIntInfo(&Info, IntConf->name) < 0)
  {
    RemoveInterface(IntConf);
    return 1;
  }

  Res = 0;

  IsWlan = IsWireless(IntConf->name);

  if (IsWlan < 0)
    IsWlan = 1;

  if (Int->is_wireless != IsWlan)
  {
    OLSR_PRINTF(1, "\tLAN/WLAN change: %d -> %d.\n", Int->is_wireless, IsWlan);

    Int->is_wireless = IsWlan;

    if (IntConf->cnf->weight.fixed)
      Int->int_metric = IntConf->cnf->weight.value;

    else
      Int->int_metric = Info.Metric;

    Res = 1;
  }

  if (Int->int_mtu != Info.Mtu)
  {
    OLSR_PRINTF(1, "\tMTU change: %d -> %d.\n", (int)Int->int_mtu,
                Info.Mtu);

    Int->int_mtu = Info.Mtu;

    net_remove_buffer(Int);
    net_add_buffer(Int);

    Res = 1;
  }

  OldVal.v4 = ((struct sockaddr_in *)&Int->int_addr)->sin_addr.s_addr;
  NewVal.v4 = Info.Addr;

#ifdef DEBUG
  OLSR_PRINTF(3, "\tAddress: %s\n", olsr_ip_to_string(&NewVal));
#endif

  if (NewVal.v4 != OldVal.v4)
  {
    OLSR_PRINTF(1, "\tAddress change.\n");
    OLSR_PRINTF(1, "\tOld: %s\n", olsr_ip_to_string(&OldVal));
    OLSR_PRINTF(1, "\tNew: %s\n", olsr_ip_to_string(&NewVal));

    Int->ip_addr.v4 = NewVal.v4;

    AddrIn = (struct sockaddr_in *)&Int->int_addr;

    AddrIn->sin_family = AF_INET;
    AddrIn->sin_port = 0;
    AddrIn->sin_addr.s_addr = NewVal.v4;

    if (olsr_cnf->main_addr.v4 == OldVal.v4)
    {
      OLSR_PRINTF(1, "\tMain address change.\n");

      olsr_cnf->main_addr.v4 = NewVal.v4;
    }

    Res = 1;
  }

  else
    OLSR_PRINTF(3, "\tNo address change.\n");

  OldVal.v4 = ((struct sockaddr_in *)&Int->int_netmask)->sin_addr.s_addr;
  NewVal.v4 = Info.Mask;

#ifdef DEBUG
  OLSR_PRINTF(3, "\tNetmask: %s\n", olsr_ip_to_string(&NewVal));
#endif

  if (NewVal.v4 != OldVal.v4)
  {
    OLSR_PRINTF(1, "\tNetmask change.\n");
    OLSR_PRINTF(1, "\tOld: %s\n", olsr_ip_to_string(&OldVal));
    OLSR_PRINTF(1, "\tNew: %s\n", olsr_ip_to_string(&NewVal));

    AddrIn = (struct sockaddr_in *)&Int->int_netmask;

    AddrIn->sin_family = AF_INET;
    AddrIn->sin_port = 0;
    AddrIn->sin_addr.s_addr = NewVal.v4;

    Res = 1;
  }

  else
    OLSR_PRINTF(3, "\tNo netmask change.\n");

  OldVal.v4 = ((struct sockaddr_in *)&Int->int_broadaddr)->sin_addr.s_addr;
  NewVal.v4 = Info.Broad;

#ifdef DEBUG
  OLSR_PRINTF(3, "\tBroadcast address: %s\n", olsr_ip_to_string(&NewVal));
#endif

  if (NewVal.v4 != OldVal.v4)
  {
    OLSR_PRINTF(1, "\tBroadcast address change.\n");
    OLSR_PRINTF(1, "\tOld: %s\n", olsr_ip_to_string(&OldVal));
    OLSR_PRINTF(1, "\tNew: %s\n", olsr_ip_to_string(&NewVal));

    AddrIn = (struct sockaddr_in *)&Int->int_broadaddr;

    AddrIn->sin_family = AF_INET;
    AddrIn->sin_port = 0;
    AddrIn->sin_addr.s_addr = NewVal.v4;

    Res = 1;
  }

  else
    OLSR_PRINTF(3, "\tNo broadcast address change.\n");

  if (Res != 0)
    run_ifchg_cbs(Int, IFCHG_IF_UPDATE);

  return Res;
}

int chk_if_up(struct olsr_if *IntConf, int DebugLevel __attribute__((unused)))
{
  struct InterfaceInfo Info;
  struct interface *New;
  union olsr_ip_addr NullAddr;
  unsigned int AddrSockAddr;
  int IsWlan;
  struct sockaddr_in *AddrIn;
  
  if (olsr_cnf->ip_version == AF_INET6)
  {
    fprintf(stderr, "IPv6 not supported by chk_if_up()!\n");
    return 0;
  }

  if (GetIntInfo(&Info, IntConf->name) < 0)
    return 0;

  New = olsr_malloc(sizeof (struct interface), "Interface 1");
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

  if (IntConf->cnf->ipv4_broadcast.v4 != 0)
    AddrIn->sin_addr.s_addr = IntConf->cnf->ipv4_broadcast.v4;

  New->int_flags = 0;

  New->is_hcif = OLSR_FALSE;

  New->int_mtu = Info.Mtu;

  New->int_name = olsr_malloc(strlen (IntConf->name) + 1, "Interface 2");
  strcpy(New->int_name, IntConf->name);

  New->if_nr = IntConf->index;

  IsWlan = IsWireless(IntConf->name);

  if (IsWlan < 0)
    IsWlan = 1;

  New->is_wireless = IsWlan;

  if (IntConf->cnf->weight.fixed)
    New->int_metric = IntConf->cnf->weight.value;

  else
    New->int_metric = Info.Metric;

  New->olsr_seqnum = random() & 0xffff;

  New->ttl_index = 0;
    
  OLSR_PRINTF(1, "\tInterface %s set up for use with index %d\n\n",
              IntConf->name, New->if_nr);
      
  OLSR_PRINTF(1, "\tMTU: %d\n", New->int_mtu);
  OLSR_PRINTF(1, "\tAddress: %s\n", sockaddr_to_string(&New->int_addr));
  OLSR_PRINTF(1, "\tNetmask: %s\n", sockaddr_to_string(&New->int_netmask));
  OLSR_PRINTF(1, "\tBroadcast address: %s\n",
              sockaddr_to_string(&New->int_broadaddr));

  New->ip_addr.v4 =
    ((struct sockaddr_in *)&New->int_addr)->sin_addr.s_addr;
      
  New->if_index = Info.Index;

  OLSR_PRINTF(3, "\tKernel index: %08x\n", New->if_index);

  AddrSockAddr = addrsock.sin_addr.s_addr;
  addrsock.sin_addr.s_addr = New->ip_addr.v4;

  New->olsr_socket = getsocket((struct sockaddr *)&addrsock,
                               127 * 1024, New->int_name);
      
  addrsock.sin_addr.s_addr = AddrSockAddr;

  if (New->olsr_socket < 0)
  {
    fprintf(stderr, "Could not initialize socket... exiting!\n\n");
    exit(1);
  }

  add_olsr_socket(New->olsr_socket, &olsr_input);

  New->int_next = ifnet;
  ifnet = New;

  IntConf->interf = New;
  IntConf->configured = 1;

  memset(&NullAddr, 0, olsr_cnf->ipsize);
  
  if(COMP_IP(&NullAddr, &olsr_cnf->main_addr))
  {
    COPY_IP(&olsr_cnf->main_addr, &New->ip_addr);
    OLSR_PRINTF(1, "New main address: %s\n", olsr_ip_to_string(&olsr_cnf->main_addr));
  }

  net_add_buffer(New);

  if (olsr_cnf->lq_level == 0)
  {
    olsr_register_scheduler_event(&generate_hello, New,
                                  IntConf->cnf->hello_params.emission_interval,
                                  0, NULL);

    olsr_register_scheduler_event(&generate_tc, New,
                                  IntConf->cnf->tc_params.emission_interval,
                                  0, NULL);
  }

  else
  {
    olsr_register_scheduler_event(&olsr_output_lq_hello, New,
                                  IntConf->cnf->hello_params.emission_interval,
                                  0, NULL);

    olsr_register_scheduler_event(&olsr_output_lq_tc, New,
                                  IntConf->cnf->tc_params.emission_interval,
                                  0, NULL);
  }

  olsr_register_scheduler_event(&generate_mid, New,
                                IntConf->cnf->mid_params.emission_interval,
                                0, NULL);

  olsr_register_scheduler_event(&generate_hna, New,
                                IntConf->cnf->hna_params.emission_interval,
                                0, NULL);

  if(olsr_cnf->max_jitter == 0 ||
     IntConf->cnf->hello_params.emission_interval / 4 < olsr_cnf->max_jitter)
    olsr_cnf->max_jitter = IntConf->cnf->hello_params.emission_interval / 4;

  if(olsr_cnf->max_tc_vtime < IntConf->cnf->tc_params.emission_interval)
    olsr_cnf->max_tc_vtime = IntConf->cnf->tc_params.emission_interval;

  New->hello_etime = IntConf->cnf->hello_params.emission_interval;

  New->valtimes.hello = double_to_me(IntConf->cnf->hello_params.validity_time);
  New->valtimes.tc = double_to_me(IntConf->cnf->tc_params.validity_time);
  New->valtimes.mid = double_to_me(IntConf->cnf->mid_params.validity_time);
  New->valtimes.hna = double_to_me(IntConf->cnf->hna_params.validity_time);

  run_ifchg_cbs(New, IFCHG_IF_ADD);

  return 1;
}

void check_interface_updates(void *dummy __attribute__((unused)))
{
  struct olsr_if *IntConf;

#ifdef DEBUG
  OLSR_PRINTF(3, "Checking for updates in the interface set\n");
#endif

  for(IntConf = olsr_cnf->interfaces; IntConf != NULL; IntConf = IntConf->next)
  {
    if(IntConf->host_emul)
      continue;
      
    if(olsr_cnf->host_emul) /* XXX: TEMPORARY! */
      continue;
 
    if(IntConf->configured)    
      chk_if_changed(IntConf);

    else
      chk_if_up(IntConf, 3);
  }
}
