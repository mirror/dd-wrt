// $Id: inet.h,v 1.6 2002/07/10 09:40:13 ensc Exp $    --*- c++ -*--

// Copyright (C) 2002 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 2 of the License.
//  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//  

#ifndef DHCP_FORWARDER_SRC_INET_H
#define DHCP_FORWARDER_SRC_INET_H 1

#include "splint.h"

#include <netinet/ip.h>
#include <netinet/ether.h>
#include <netinet/udp.h>

#include "compat.h"

  /*@-exportconst@*/
  /*@constant uint16_t DHCP_PORT_CLIENT@*/
  /*@constant uint16_t DHCP_PORT_SERVER@*/
  /*@=exportconst@*/

#ifndef S_SPLINT_S
enum {
  DHCP_PORT_SERVER = 67u,
  DHCP_PORT_CLIENT = 68u
};
#endif // S_SPLINT_S

struct DHCPllPacket
{
    struct ether_header         eth;
    struct iphdr		ip;
    struct udphdr		udp;
    __extension__ char		data __flexarr;
} __attribute__((__packed__));

#endif	/* DHCP_FORWARDER_SRC_INET_H */

  // Local Variables:
  // compile-command: "make -C .. -k"
  // fill-column: 80
  // End:
