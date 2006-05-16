// $Id: splint.h,v 1.3 2002/07/10 09:40:13 ensc Exp $    --*- c++ -*--

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

#ifndef H_DHCP_FORWARDER_SPLINT_H
#define H_DHCP_FORWARDER_SPLINT_H

#include "splint_compat.h"

  /*@-declundef@*/
  /*@unused@*/
extern void assertDefined (/*@out@*/ /*@sef@*/ /*@unused@*/ void * ) /*@*/;
  /*@=declundef@*/

#ifndef S_SPLINT_S
#  define assertDefined(x)
#endif

#endif	//  H_DHCP_FORWARDER_SPLINT_H
