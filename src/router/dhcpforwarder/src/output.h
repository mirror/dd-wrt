// $Id: output.h,v 1.4 2002/07/11 16:59:42 ensc Exp $    --*- c++ -*--

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

#ifndef DHCP_FORWARDER_OUTPUT_H
#define DHCP_FORWARDER_OUTPUT_H

void		openMsgfile(/*@in@*//*@null@*/char const *filename)
  /*@globals fileSystem, internalState@*/
  /*@modifies fileSystem, internalState@*/ ;

void		writeMsg(char const *msg, size_t len)
  /*@globals internalState, errno@*/
  /*@modifies internalState, errno@*/ ;

void		writeMsgStr(char const *msg, size_t len)
  /*@globals internalState, errno@*/
  /*@modifies internalState, errno@*/ ;

void		writeMsgUInt(unsigned int val)
  /*@globals internalState, errno@*/
  /*@modifies internalState, errno@*/ ;

void		writeMsgTimestamp()
  /*@globals internalState, errno@*/
  /*@modifies internalState, errno@*/ ;


void		writeUInt(int fd, unsigned int val)
  /*@globals internalState, errno@*/
  /*@modifies internalState, errno@*/ ;

#endif	//  DHCP_FORWARDER_OUTPUT_H

  // Local Variables:
  // compile-command: "make -C .. -k"
  // fill-column: 80
  // End:
