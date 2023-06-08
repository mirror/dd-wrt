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

#ifndef DHCP_FORWARDER_SRC_DHCP_H
#define DHCP_FORWARDER_SRC_DHCP_H

#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>

#include "compat.h"
#include "util.h"

struct DHCPHeader  {
    uint8_t	op;
    uint8_t	htype;
    uint8_t	hlen;
    uint8_t	hops;

    uint32_t	xid;
    uint16_t	secs;

    uint16_t	flags;

    in_addr_t	ciaddr;
    in_addr_t	yiaddr;
    in_addr_t	siaddr;
    in_addr_t	giaddr;

    uint8_t	chaddr[16];
    uint8_t	sname[64];
    uint8_t	file[128];
} __attribute__((__packed__));

struct DHCPOptions {
    uint32_t		cookie;
    char		data[];
} __attribute__((__packed__));

struct DHCPSingleOption {
    uint8_t		code;
    uint8_t		len;
    uint8_t		data[];
} __attribute__((__packed__));


  /*@-exportconst@*/
  /*@constant unsigned int flgDHCP_BCAST@*/
  /*@constant unsigned int MAX_HOPS@*/
  /*@constant unsigned int optDHCP_COOKIE@*/

  /*@constant unsigned int opBOOTREQUEST@*/
  /*@constant unsigned int opBOOTREPLY@*/

  /*@constant uint8_t cdPAD@*/
  /*@constant uint8_t cdRELAY_AGENT@*/
  /*@constant uint8_t cdEND@*/

  /*@constant uint8_t agCIRCUITID@*/
  /*@constant uint8_t agREMOTEID@*/

  /*@=exportconst@*/


#ifndef S_SPLINT_S

enum {
  MAX_HOPS	= 16u
};

enum {
#if __BYTE_ORDER == __LITTLE_ENDIAN
  optDHCP_COOKIE = 0x63538263u,
  flgDHCP_BCAST	 = 0x0080u
#else
  optDHCP_COOKIE = 0x63825363u,
  flgDHCP_BCAST	 = 0x8000u
#endif
};

enum {
  opBOOTREQUEST	= 1u,
  opBOOTREPLY	= 2u
};

enum {
  cdPAD	= 0u,
  cdRELAY_AGENT = 82u,
  cdEND	= 255u
};

enum {
  agCIRCUITID	  = 1u,
  agREMOTEID	  = 2u,
  /* Link Selection  RFC3527 */
  agLINKSELECT	  = 5u,
  /* Server Identifier Override Suboption Definition - RFC 5107 */
  agREPLACESERVER = 11u,
};
#endif

inline static size_t
DHCP_ptrdiff(struct DHCPSingleOption const *a,
	     struct DHCPSingleOption const *b)
{
  return reinterpret_cast(uintptr_t)(a) - reinterpret_cast(uintptr_t)(b);
}

/*@unused@*/
inline static size_t
DHCP_getOptionLength(/*@sef@*//*@in@*/struct DHCPSingleOption const *opt)
    /*@*/
{
  switch (opt->code) {
    case cdPAD	:
    case cdEND	:  return 1;
    default	:  return opt->len + 2;
  }
}

/*@unused@*/
inline static size_t
DHCP_removeOption(struct DHCPSingleOption *opt,
		  struct DHCPSingleOption **end_opt)
    /*@requires opt <= end_opt@*/
    /*@modifies *opt@*/
{
  size_t		len = DHCP_getOptionLength(opt);
  char * const		start = reinterpret_cast(char *)(opt);
  char const * const	end   = reinterpret_cast(char const *)(*end_opt);

  assert(opt <= *end_opt);
  if (start+len > end)
    // TODO: broken option-list ... what to do?
    return 0;

    // end-start  -->  character count between opt and end_opt without end_opt
    // -len       -->  size of the option to be removed
    // +1         -->  the end_opt
    /*@-strictops@*/
  memmove(start, start+len, end-(start+len) + 1);
    /*@=strictops@*/

  *end_opt = reinterpret_cast(struct DHCPSingleOption *)(end - len);

  return len;
}

/*@unused@*/
inline static void
DHCP_zeroOption(struct DHCPSingleOption *opt)
    /*@modifies *opt@*/
{
  size_t	len = DHCP_getOptionLength(opt);
  size_t	i;


  for (i=0; i<len; ++i) {
    reinterpret_cast(uint8_t *)(opt)[i] = cdPAD;
  }
}

/*@unused@*/
inline static struct DHCPSingleOption *
DHCP_nextSingleOption(/*@sef@*//*@in@*//*@returned@*/struct DHCPSingleOption *opt)
    /*@*/
    /*@ensures result >= opt@*/
{
  size_t cnt = DHCP_getOptionLength(opt);

    /*@-ptrarith@*/
  return (reinterpret_cast(struct DHCPSingleOption *)
	  (reinterpret_cast(char *)(opt) + cnt));
    /*@=ptrarith@*/
}

/*@unused@*/
inline static struct DHCPSingleOption const *
DHCP_nextSingleOptionConst(/*@sef@*//*@in@*//*@returned@*/struct DHCPSingleOption const *opt)
    /*@*/
    /*@ensures result >= opt@*/
{
  return DHCP_nextSingleOption(const_cast(struct DHCPSingleOption *)(opt));
}

#endif	/* DHCP_FORWARDER_SRC_DHCP_H */

  // Local Variables:
  // compile-command: "make -C .. -k"
  // fill-column: 80
  // End:
