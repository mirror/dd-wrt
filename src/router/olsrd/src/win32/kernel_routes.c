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
 * $Id: kernel_routes.c,v 1.15 2005/02/27 10:48:05 kattemat Exp $
 */

#include <stdio.h>
#include "net/route.h"

#include "kernel_routes.h"
#include "defs.h"

#define WIN32_LEAN_AND_MEAN
#include <iprtrmib.h>
#include <iphlpapi.h>
#undef interface

char *StrError(unsigned int ErrNo);

int olsr_ioctl_add_route(struct rt_entry *Dest)
{
  MIB_IPFORWARDROW Row;
  unsigned long Res;
  char Str1[16], Str2[16], Str3[16];

  inet_ntop(AF_INET, &Dest->rt_dst.v4, Str1, 16);
  inet_ntop(AF_INET, &Dest->rt_mask.v4, Str2, 16);
  inet_ntop(AF_INET, &Dest->rt_router.v4, Str3, 16);

  OLSR_PRINTF(1, "Adding IPv4 route with metric %d to %s/%s via %s and I/F 0x%x.\n",
              Dest->rt_metric, Str1, Str2, Str3, Dest->rt_if->if_index)

  memset(&Row, 0, sizeof (MIB_IPFORWARDROW));

  Row.dwForwardDest = Dest->rt_dst.v4;
  Row.dwForwardMask = Dest->rt_mask.v4;
  Row.dwForwardNextHop = Dest->rt_router.v4;
  Row.dwForwardIfIndex = Dest->rt_if->if_index;
  Row.dwForwardType = (Dest->rt_dst.v4 == Dest->rt_router.v4) ? 3 : 4;
  Row.dwForwardProto = 3; // PROTO_IP_NETMGMT
  Row.dwForwardMetric1 = Dest->rt_metric;

  Res = SetIpForwardEntry(&Row);

  if (Res != NO_ERROR)
  {
    fprintf(stderr, "SetIpForwardEntry() = %08lx, %s", Res, StrError(Res));
    Res = CreateIpForwardEntry(&Row);
  }

  if (Res != NO_ERROR)
  {
    fprintf(stderr, "CreateIpForwardEntry() = %08lx, %s", Res, StrError(Res));

    // XXX - report error in a different way

    errno = Res;

    return -1;
  }

  if(olsr_cnf->open_ipc)
    ipc_route_send_rtentry(&Dest->rt_dst, &Dest->rt_router, Dest->rt_metric,
                           1, Dest->rt_if->int_name);

  return 0;
}

// XXX - to be implemented

int olsr_ioctl_add_route6(struct rt_entry *Dest)
{
  return 0;
}

int olsr_ioctl_del_route(struct rt_entry *Dest)
{
  MIB_IPFORWARDROW Row;
  unsigned long Res;
  char Str1[16], Str2[16], Str3[16];

  inet_ntop(AF_INET, &Dest->rt_dst.v4, Str1, 16);
  inet_ntop(AF_INET, &Dest->rt_mask.v4, Str2, 16);
  inet_ntop(AF_INET, &Dest->rt_router.v4, Str3, 16);

  OLSR_PRINTF(1, "Deleting IPv4 route with metric %d to %s/%s via %s and I/F 0x%x.\n",
              Dest->rt_metric, Str1, Str2, Str3, Dest->rt_if->if_index)

  memset(&Row, 0, sizeof (MIB_IPFORWARDROW));

  Row.dwForwardDest = Dest->rt_dst.v4;
  Row.dwForwardMask = Dest->rt_mask.v4;
  Row.dwForwardNextHop = Dest->rt_router.v4;
  Row.dwForwardIfIndex = Dest->rt_if->if_index;
  Row.dwForwardType = (Dest->rt_dst.v4 == Dest->rt_router.v4) ? 3 : 4;
  Row.dwForwardProto = 3; // PROTO_IP_NETMGMT
  Row.dwForwardMetric1 = Dest->rt_metric;

  Res = DeleteIpForwardEntry(&Row);

  if (Res != NO_ERROR)
  {
    fprintf(stderr, "DeleteIpForwardEntry() = %08lx, %s", Res, StrError(Res));

    // XXX - report error in a different way

    errno = Res;

    return -1;
  }

  if(olsr_cnf->open_ipc)
    ipc_route_send_rtentry(&Dest->rt_dst, NULL, Dest->rt_metric, 0, NULL);

  return 0;
}

// XXX - to be implemented

int olsr_ioctl_del_route6(struct rt_entry *Dest)
{
  return 0;
}
