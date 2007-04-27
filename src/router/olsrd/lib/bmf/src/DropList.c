/*
 * OLSR Basic Multicast Forwarding (BMF) plugin.
 * Copyright (c) 2005, 2006, Thales Communications, Huizen, The Netherlands.
 * Written by Erik Tromp.
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
 * * Neither the name of Thales, BMF nor the names of its 
 *   contributors may be used to endorse or promote products derived 
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* -------------------------------------------------------------------------
 * File       : DropList.c
 * Description: List of MAC addresses of hosts from which received multicast
 *              and local broadcast packets are dropped.
 * Created    : 29 Jun 2006
 *
 * $Id: DropList.c,v 1.2 2007/02/10 17:05:55 bernd67 Exp $ 
 * ------------------------------------------------------------------------- */


#include "DropList.h"

/* System includes */
#include <assert.h> /* assert() */
#include <stdio.h> /* NULL */
#include <stdlib.h> /* malloc */
#include <string.h> /* memcmp */

/* OLSRD includes */
#include "olsr.h" /* olsr_printf */

/* Plugin includes */
#include "Bmf.h" /* PLUGIN_NAME */
#include "Packet.h" /* IFHWADDRLEN */

static struct TMacAddress* DroppedMacAddresses = NULL;

/* -------------------------------------------------------------------------
 * Function   : DropMac
 * Description: Register a MAC address in the drop list
 * Input      : macStr - MAC address as string
 * Output     : none
 * Return     : fail (0) or success (1)
 * Data Used  : DroppedMacAddresses
 * Notes      : The registered MAC address will be matched to the source MAC 
 *              address of incoming multicast packets. If matched, the multicast 
 *              packet will be dropped. 
 *              The drop list is needed only in lab environments, where hidden
 *              nodes are simulated by using iptables with the
 *              -m mac helper and --mac-source option (as in: 
 *              "iptables -A INPUT -m mac --mac-source 00:0C:29:EE:C9:D0 -j DROP") 
 *              The drop list is needed because network interfaces in promiscuous 
 *              mode will still capture packets even if they are specified to 
 *              be dropped by iptables.
 * ------------------------------------------------------------------------- */
int DropMac(const char* macStr)
{
  unsigned int mac[6];
  int n;
  struct TMacAddress* newMacAddress;
  int i;

  assert(macStr != NULL);

  n = sscanf(macStr, "%x:%x:%x:%x:%x:%x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
  if (n != 6)
  {
    olsr_printf(1, "%s: Invalid Ethernet address '%s'\n", PLUGIN_NAME, macStr);
    return 0;
  }

  newMacAddress = malloc(sizeof(struct TMacAddress));
  for (i = 0; i < 6; i++)
  {
    newMacAddress->addr[i] = (unsigned char) mac[i];
  }
  newMacAddress->next = DroppedMacAddresses;
  DroppedMacAddresses = newMacAddress;

  return 1;
}

/* -------------------------------------------------------------------------
 * Function   : IsInDropList
 * Description: Check if a MAC address is in the drop list
 * Input      : macAddress
 * Output     : none
 * Return     : true (1) or false (0)
 * Data Used  : DroppedMacAddresses
 * ------------------------------------------------------------------------- */
int IsInDropList(const unsigned char* macAddress)
{
  struct TMacAddress* ma = DroppedMacAddresses;

  assert(macAddress != NULL);

  while (ma != NULL)
  {
    if (memcmp(ma->addr, macAddress, IFHWADDRLEN) == 0) return 1;
    ma = ma->next;
  }
  return 0;
}

