// $Id: parser.h,v 1.6 2002/07/10 09:40:13 ensc Exp $    --*- c++ -*--

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

#ifndef H_DHCP_FORWARDER_SRC_PARSER
#define H_DHCP_FORWARDER_SRC_PARSER

struct ConfigInfo;

void	parse(/*@in@*/char const		*filename,
	      /*@dependent@*/struct ConfigInfo	*cfg)
  /*@globals fileSystem, internalState@*/
  /*@modifies fileSystem, internalState, *cfg@*/ ;

#endif	//  H_DHCP_FORWARDER_SRC_PARSER

  // Local Variables:
  // compile-command: "make -C .. -k"
  // fill-column: 80
  // End:
