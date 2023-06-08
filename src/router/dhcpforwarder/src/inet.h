// Copyright (C) 2002, 2008, 2014
//               Enrico Scholz <enrico.scholz@ensc.de>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see http://www.gnu.org/licenses/.

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
    char			data[];
} __attribute__((__packed__));

#endif	/* DHCP_FORWARDER_SRC_INET_H */

  // Local Variables:
  // compile-command: "make -C .. -k"
  // fill-column: 80
  // End:
