//=============================================================================
//
//      sntp.c
//
//      Helper functions to access the interface information
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Andrew Lunn
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   andrew.lunn
// Contributors:
// Date:        2003-02-22
// Description: Provides helper functions to access the network interface
//              information.
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/system.h>

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <net/if.h>
#include <net/if_types.h>

/* How many interfaces are there? */

#ifdef CYGPKG_NET_FREEBSD_STACK
extern struct ifaddr **ifnet_addrs;

long cyg_snmp_num_interfaces(void) {
  long long_ret=0;
  int cnt = if_index - 1;
  
  while (cnt >= 0) {
    if (ifnet_addrs[cnt] != 0) {
      long_ret++;
    }
    cnt--;
  }
  return long_ret;
}

struct ifnet *cyg_snmp_get_if(int if_num) {
  int index = 0;
  struct ifnet *ifp;
  
  do {
    while(0 == ifnet_addrs[index])
      index++;

    ifp = ifnet_addrs[index]->ifa_ifp;
    
    if_num--;	    
    index++;
  } while (if_num);

  return ifp;
}
#endif

#ifdef CYGPKG_NET_OPENBSD_STACK
long cyg_snmp_num_interfaces(void) {
  register struct ifnet *ifp;
  long long_ret = 0;

  for (ifp = ifnet.tqh_first; ifp != 0; ifp = ifp->if_list.tqe_next)
    long_ret++;
  
  return long_ret;
}

struct ifnet *cyg_snmp_get_if(int if_num) {
  struct ifnet *ifp;
  
  for ( ifp = ifnet.tqh_first;
	if_num > 1 && ifp != 0;
          if_num-- )
        ifp = ifp->if_list.tqe_next;
  
  return ifp;
}
#endif


