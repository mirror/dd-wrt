#ifndef CYGONCE_NS_DNS_DNS_PRIV_H
#define CYGONCE_NS_DNS_DNS_PRIV_H
//=============================================================================
//
//      dns-priv.h
//
//      Private DNS client definitions.
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   andrew.lunn
// Contributors:andrew.lunn, jskov
// Date:        2001-09-18
//
//####DESCRIPTIONEND####
//
//=============================================================================

struct dns_header {
#if (CYG_BYTEORDER == CYG_LSBFIRST)
  unsigned        id :16;         /* query identification number */
  /* fields in third byte */
  unsigned        rd :1;          /* recursion desired */
  unsigned        tc :1;          /* truncated message */
  unsigned        aa :1;          /* authoritive answer */
  unsigned        opcode :4;      /* purpose of message */
  unsigned        qr :1;          /* response flag */
  /* fields in fourth byte */
  unsigned        rcode :4;       /* response code */
  unsigned        cd: 1;          /* checking disabled by resolver */
  unsigned        ad: 1;          /* authentic data from named */
  unsigned        unused :1;      /* unused bits */
  unsigned        ra :1;          /* recursion available */
  /* remaining bytes */
  unsigned        qdcount :16;    /* number of question entries */
  unsigned        ancount :16;    /* number of answer entries */
  unsigned        nscount :16;    /* number of authority entries */
  unsigned        arcount :16;    /* number of resource entries */
#else
  unsigned        id :16;         /* query identification number */
  /* fields in third byte */
  unsigned        qr :1;          /* response flag */
  unsigned        opcode :4;      /* purpose of message */
  unsigned        aa :1;          /* authoritive answer */
  unsigned        tc :1;          /* truncated message */
  unsigned        rd :1;          /* recursion desired */
  /* fields in fourth byte */
  unsigned        ra :1;          /* recursion available */
  unsigned        unused :1;      /* unused bits */
  unsigned        ad: 1;          /* authentic data from named */
  unsigned        cd: 1;          /* checking disabled by resolver */
  unsigned        rcode :4;       /* response code */
  /* remaining bytes */
  unsigned        qdcount :16;    /* number of question entries */
  unsigned        ancount :16;    /* number of answer entries */
  unsigned        nscount :16;    /* number of authority entries */
  unsigned        arcount :16;    /* number of resource entries */
#endif
};

struct resource_record {
  unsigned rr_type : 16; /* Type of resourse */
  unsigned class   : 16; /* Class of resource */
  unsigned ttl     : 32; /* Time to live of this record */
  unsigned rdlength: 16; /* Lenght of data to follow */
  char     rdata [2];   /* Resource DATA */
};

/* Opcodes */
#define DNS_QUERY  0   /* Standard query */
#define DNS_IQUERY 1   /* Inverse query */
#define DNS_STATUS 2   /* Name server status */
#define DNS_NOTIFY 4   /* Zone change notification */
#define DNS_UPDATE 5   /* Zone update message */

/* DNS TYPEs */
#define DNS_TYPE_A     1   /* Host address */
#define DNS_TYPE_NS    2   /* Authoritative name server */
#define DNS_TYPE_CNAME 5   /* Canonical name for an alias */
#define DNS_TYPE_PTR   12  /* Domain name pointer */
#define DNS_TYPE_AAAA  28  /* IPv6 host address */

/* DNS CLASSs */
#define DNS_CLASS_IN   1   /* Internet */

/* DNS reply codes */
#define DNS_REPLY_NOERR      0
#define DNS_REPLY_NAME_ERROR 3

#define MAXDNSMSGSIZE 512

//-----------------------------------------------------------------------------
#endif // CYGONCE_NS_DNS_DNS_PRIV_H
// End of dns-priv.h
