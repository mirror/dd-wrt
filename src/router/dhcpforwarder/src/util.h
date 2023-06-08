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

#ifndef DHCP_FORWARDER_UTIL_H
#define DHCP_FORWARDER_UTIL_H

  /*@-namechecks@*/
#ifndef __cplusplus
#  define cAsT_(X)		(X))
#  define reinterpret_cast(X)	((X) cAsT_
#  define static_cast(X)	((X) cAsT_
#  define const_cast(X)		((X) cAsT_
#else	/* __cplusplus */
#  define reinterpret_cast(X)	reinterpret_cast<X>
#  define static_cast(X)	static_cast<X>
#  define const_cast(X)		const_cast<X>
#endif
  /*@=namechecks@*/

#endif	/* DHCP_FORWARDER_UTIL_H */

  // Local Variables:
  // compile-command: "make -C .. -k"
  // fill-column: 80
  // End:
