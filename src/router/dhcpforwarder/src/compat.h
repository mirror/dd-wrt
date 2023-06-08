// Copyright (C) 2002, 2003, 2004, 2008, 2014
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

#ifndef H_DHCP_FORWARDER_SRC_COMPAT_H
#define H_DHCP_FORWARDER_SRC_COMPAT_H

  /*@-namechecks@*/

#if defined(HAVE_STDBOOL_H) && !defined(__cplusplus)
#  include <stdbool.h>
#endif

#include <features.h>
#include <inttypes.h>
#include <unistd.h>

#ifndef ETH_ALEN
#  define ETH_ALEN		6
#endif

#ifndef ETHERTYPE_IP
#  define ETHERTYPE_IP		0x0800
#endif

#ifndef IP_DF
#  define IP_DF			0x4000
#endif

#ifndef S_SPLINT_S
#  if !defined(__bool_true_false_are_defined) && !defined(__cplusplus)
  typedef int			bool;
  enum {false = 0, true = 1 };
#  endif
#endif

#if (defined(__dietlibc__) && !defined(DIET_HAS_IN_ADDR_T)) ||	\
   (!defined(__dietlibc__) && !defined(HAVE_IN_ADDR_T))
  typedef uint32_t	in_addr_t;
#endif

#if defined(__dietlibc__) && !defined(DIET_HAS_STRUCT_ETHER_HEADER)
struct ether_header
{
  uint8_t  ether_dhost[ETH_ALEN];      /* destination eth addr */
  uint8_t  ether_shost[ETH_ALEN];      /* source ether addr    */
  uint16_t ether_type;                 /* packet type ID field */
} __attribute__ ((__packed__));
#endif

#if !defined(TEMP_FAILURE_RETRY ) && !defined(S_SPLINT_S)
#  define TEMP_FAILURE_RETRY(expression)		\
   (__extension__					\
     ({ long int m_result;				\
	do m_result = (long int) (expression);		\
	while (m_result == -1L && errno == EINTR);	\
	m_result; }))
#endif

  /*@=namechecks@*/

#endif	/* H_DHCP_FORWARDER_SRC_COMPAT_H */

  // Local Variables:
  // compile-command: "make -C .. -k"
  // fill-column: 80
  // End:
