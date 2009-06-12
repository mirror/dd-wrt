#ifndef CYGONCE_ISO_NETDB_H
#define CYGONCE_ISO_NETDB_H
/*==========================================================================
//
//      netdb.h
//
//      Network database functions
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jskov,jlarmour
// Contributors: 
// Date:          2001-09-28
// Purpose:       Provides network database types and function API.
// Description:   Much of the real contents of this file get set from the
//                configuration (set by the implementation)
// Usage:         #include <netdb.h>
//              
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/isoinfra.h>

#ifdef CYGINT_ISO_DNS
# ifdef CYGBLD_ISO_DNS_HEADER
#  include CYGBLD_ISO_DNS_HEADER
# endif
#else

/* Provide dummy entries so that legacy code that relies on the presence
  of these types/functions still works. */

/* Host name / IP mapping fallback when not using a real DNS implementation */
struct hostent {
  char    *h_name;        /* official name of host */
  char    **h_aliases;    /* alias list */
  int     h_addrtype;     /* host address type */
  int     h_length;       /* length of address */
  char    **h_addr_list;  /* list of addresses */
};
#define h_addr  h_addr_list[0]  /* for backward compatibility */

static inline struct hostent *
gethostbyname( const char *__host )
{
  return NULL;
}

static inline struct hostent *
gethostbyaddr( const char * __addr, int __len, int __type )
{
  return NULL;
}

#endif /* ifdef CYGINT_ISO_DNS */

#ifdef CYGINT_ISO_NETDB_PROTO
# ifdef CYGBLD_ISO_NETDB_PROTO_HEADER
#  include CYGBLD_ISO_NETDB_PROTO_HEADER
# endif
#endif

#ifdef CYGINT_ISO_NETDB_SERV
# ifdef CYGBLD_ISO_NETDB_SERV_HEADER
#  include CYGBLD_ISO_NETDB_SERV_HEADER
# endif
#endif

#endif /* CYGONCE_ISO_NETDB_H multiple inclusion protection */

/* EOF netdb.h */
