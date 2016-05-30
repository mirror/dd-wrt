/* src/native/jvmti/cacaodbgserver.h - contains cacao specifics for 
                                       debugging support

   Copyright (C) 1996-2005, 2006 R. Grafl, A. Krall, C. Kruegel,
   C. Oates, R. Obermaisser, M. Platter, M. Probst, S. Ring,
   E. Steiner, C. Thalinger, D. Thuernbeck, P. Tomsich, C. Ullrich,
   J. Wenninger, Institut f. Computersprachen - TU Wien

   This file is part of CACAO.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.

   Contact: cacao@complang.tuwien.ac.at

   Authors: Martin Platter

   Changes: 



*/

#ifndef _CACAODBGSERVER_H
#define _CACAODBGSERVER_H

#define INBUFLEN 8192

#define CONTINUE      "-exec-continue\n"

#define HCSIGTRAP "*stopped,reason=\"signal-received\",signal-name=\"SIGTRAP\"" 
#define GDBBREAKPOINT       "*stopped,reason=\"breakpoint-hit\""
#define EXITEDNORMALLY      "*stopped,reason=\"exited-normally\""
#define REGNAMES            "^done,register-names=["
#define CURRENTTHREAD       "~\"*"
#define SIGADDR             "frame={addr=\""
#define DATAEVALUATE        "value=\""
#define LOGSTREAMOUTPUT     '&'
#define CONSOLESTREAMOUTPUT '~'
#define OUTPUTEND           "(gdb)"
#define OUTPUTENDSIZE       5


#endif

/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 */
