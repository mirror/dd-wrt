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

#ifndef DHCP_FORWARDER_ASSERTIONS_H
#define DHCP_FORWARDER_ASSERTIONS_H

#include "dhcp.h"
#include "inet.h"

#ifndef NDEBUG
extern void	iphdr_not_20();
extern void	ether_header_not_14();
extern void	udphdr_not_8();
extern void	dhcpheader_not_236();
extern void	dhcpoptions_not_4();
extern void	dhcpsingleoption_not_2();
extern void	dhcpllpacket_not_42();
#endif

  /*@unused@*//*@maynotreturn@*/
inline static void
checkCompileTimeAssertions() /*@*/
{
#ifndef NDEBUG
#  ifdef __OPTIMIZE__
  if (sizeof(struct iphdr)!=20)           iphdr_not_20();
  if (sizeof(struct ether_header)!=14)    ether_header_not_14();
  if (sizeof(struct udphdr)!=8)           udphdr_not_8();
  if (sizeof(struct DHCPHeader)!=236)     dhcpheader_not_236();
  if (sizeof(struct DHCPOptions)!=4)      dhcpoptions_not_4();
  if (sizeof(struct DHCPSingleOption)!=2) dhcpsingleoption_not_2();
  if (sizeof(struct DHCPllPacket)!=42)    dhcpllpacket_not_42();
#  endif	/* __OPTIMIZE__ */
#endif		/* NDEBUG */
}

#endif	/* DHCP_FORWARDER_ASSERTIONS_H */

  // Local Variables:
  // compile-command: "make -C .. -k"
  // fill-column: 80
  // End:
