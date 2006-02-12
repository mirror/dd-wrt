// $Id: logging.c,v 1.11 2002/08/30 12:34:57 ensc Exp $    --*- c++ -*--

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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "splint.h"

#include <stdarg.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "output.h"
#include "logging.h"
#include "dhcp.h"


static void WRITE(/*@sef@*//*@observer@*/char const STR[])
  /*@globals internalState@*/
  /*@modifies internalState@*/ ;

static void WRITESTR(/*@sef@*//*@observer@*/char const *STR)
  /*@globals internalState@*/
  /*@modifies internalState@*/ ;

  /*@-sizeofformalarray@*/
#define WRITE(STR)	writeMsgStr((STR), sizeof(STR)-1)
  /*@=sizeofformalarray@*/
#define WRITESTR(STR)	writeMsgStr((STR), strlen(STR))


inline static char *
Xinet_ntop(sa_family_t af, /*@in@*/void const *src,
	   /*@returned@*//*@out@*/char *dst, size_t cnt)
    /*@requires cnt>=8 /\ maxSet(dst) >= cnt@*/
    /*@modifies *dst@*/
{
  if (inet_ntop(af, src, dst, cnt)==0) {
    strcpy(dst, "< ???? >");
  }

  return dst;
}

typedef /*@out@*/ char *	char_outptr;

inline static void
Xsnprintf(/*@out@*/char_outptr * const buffer, size_t * const len,
	  /*@in@*/char const * const format, ...)
    /*@requires notnull *buffer@*/
    /*@requires maxRead(*buffer) >= 0@*/
    /*@globals internalState@*/
    /*@modifies internalState, *buffer, *len@*/
{
  va_list	ap;
  int		l;
  
  va_start(ap, format);
  l = vsnprintf(*buffer, *len, format, ap);
  if (l<0 || static_cast(unsigned int)(l)>*len) {
    WRITE("\n\nBuffer not large enough for snprintf(\"");
    WRITESTR(format);
    WRITE("\");\nthere are ");
    writeMsgUInt(*len);
    WRITE(" chars available but ");
    writeMsgUInt(static_cast(unsigned int)(l));
    WRITE(" required\n\n");
  }
  else {
    *len    -= l;
    *buffer += l;
  }
}

inline static void
Xstrncat(/*@unique@*/char * const buffer,
	 /*@in@*/char const * const what, size_t *len)
    /*@modifies *buffer, *len@*/
{
  size_t const		what_len = strlen(what);
  
  if (what_len<*len) {
    strcat(buffer, what);
    *len -= what_len;
  }
}

void
logDHCPPackage(char const *data, size_t	len,
	       struct in_pktinfo const		*pkinfo,
	       void const			*addr)
{
  int				error = errno;
  /*@temp@*/char		buffer[256];
  char 				*buffer_ptr;
  char				addr_buffer[128];	/* adjust if needed */
  /*@dependent@*/char const	*msg = 0;
  size_t			avail;
  struct sockaddr const		*saddr = reinterpret_cast(struct sockaddr const *)(addr);
  struct DHCPHeader const	*header = reinterpret_cast(struct DHCPHeader const *)(data);
  
  writeMsgTimestamp();
  WRITESTR(": ");

  if (len==static_cast(size_t)(-1)) {
    msg = strerror(error);
  }
  else {
    void const		*ptr;
    switch (saddr->sa_family) {
      case AF_INET	 : ptr = &reinterpret_cast(struct sockaddr_in const  *)(addr)->sin_addr;  break;
      case AF_INET6	:  ptr = &reinterpret_cast(struct sockaddr_in6 const *)(addr)->sin6_addr; break;
      default		:  ptr = saddr->sa_data; break;
    }

    (void)Xinet_ntop(saddr->sa_family, ptr, addr_buffer, sizeof addr_buffer);

    buffer_ptr = buffer;
    avail      = sizeof(buffer)-1;
    
#if 1
    Xsnprintf(&buffer_ptr, &avail, "from %s (", addr_buffer) ;
    
    (void)Xinet_ntop(saddr->sa_family, &pkinfo->ipi_addr, addr_buffer, sizeof addr_buffer);
    Xsnprintf(&buffer_ptr, &avail, "%i, %s, ", pkinfo->ipi_ifindex, addr_buffer);

    (void)Xinet_ntop(saddr->sa_family, &pkinfo->ipi_spec_dst, addr_buffer, sizeof addr_buffer);
    Xsnprintf(&buffer_ptr, &avail, "%s)): ", addr_buffer);
#else
    Xsnprintf(&buffer_ptr, &avail, "from %s (if #%i): ", addr_buffer, pkinfo->ipi_ifindex);
#endif
    
    if (len<sizeof(struct DHCPHeader)) {
      Xsnprintf(&buffer_ptr, &avail, "Broken package with len %lu", len);
    }
    else {
      struct in_addr		ip;
      bool			is_faulty = false;

      Xsnprintf(&buffer_ptr, &avail, "%08x ", header->xid);
      switch (header->op) {
	case opBOOTREQUEST:
	  Xstrncat(buffer_ptr, "BOOTREQUEST from ", &avail);
	  ip.s_addr = header->ciaddr;
	  break;
	case opBOOTREPLY:
	  Xstrncat(buffer_ptr, "BOOTREPLY to ", &avail);
	  ip.s_addr = header->yiaddr;
	  break;
	default:
	  Xsnprintf(&buffer_ptr, &avail, "<UNKNOWN> (%u), ", header->op);
	  is_faulty = true;
	  break;
      }

      if (!is_faulty) {
	assertDefined(&ip);
	Xstrncat(buffer_ptr, inet_ntoa(ip), &avail);
      }
    }

    assertDefined(buffer);
    buffer[sizeof(buffer)-1] = '\0';
    msg                      = buffer;
  }
  
  writeMsgStr(msg, strlen(msg));
  writeMsgStr("\n", 1);

  /*@-mods@*/errno = error;/*@=mods@*/
}

  // Local Variables:
  // compile-command: "make -C .. -k"
  // fill-column: 80
  // End:
