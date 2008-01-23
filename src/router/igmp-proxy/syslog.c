/*
**  igmpproxy - IGMP proxy based multicast router 
**  Copyright (C) 2005 Johnny Egeland <johnny@rlo.org>
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
**----------------------------------------------------------------------------
**
**  This software is derived work from the following software. The original
**  source code has been modified from it's original state by the author
**  of igmpproxy.
**
**  smcroute 0.92 - Copyright (C) 2001 Carsten Schill <carsten@cschill.de>
**  - Licensed under the GNU General Public License, version 2
**  
**  mrouted 3.9-beta3 - COPYRIGHT 1989 by The Board of Trustees of 
**  Leland Stanford Junior University.
**  - Original license can be found in the "doc/mrouted-LINCESE" file.
**
*/

#include "defs.h"

int  Log2Stderr = LOG_WARNING;

int  LogLastServerity;
int  LogLastErrno;
char LogLastMsg[ 128 ];

/*
** Writes the message 'FmtSt' with the parameters '...' to syslog.
** 'Serverity' is used for the syslog entry. For an 'Errno' value 
** other then 0, the correponding error string is appended to the
** message.
**
** For a 'Serverity' more important then 'LOG_WARNING' the message is 
** also logged to 'stderr' and the program is finished with a call to 
** 'exit()'.
**
** If the 'Serverity' is more important then 'Log2Stderr' the message
** is logged to 'stderr'.
**          
*/
void log( int Serverity, int Errno, const char *FmtSt, ... )
{
  const char ServVc[][ 5 ] = { "EMER", "ALER", "CRIT", "ERRO", 
			       "Warn", "Note", "Info", "Debu" };

  const char *ServPt = Serverity < 0 || Serverity >= VCMC( ServVc ) ? 
                       "!unknown serverity!" : ServVc[ Serverity ];
 
  const char *ErrSt = (Errno <= 0) ? NULL : (const char *)strerror( Errno ); 

  {
    va_list ArgPt;
    unsigned Ln;

    va_start( ArgPt, FmtSt );
    Ln  = snprintf( LogLastMsg, sizeof( LogLastMsg ), "%s: ", ServPt );
    Ln += vsnprintf( LogLastMsg + Ln, sizeof( LogLastMsg ) - Ln, FmtSt, ArgPt );
    if( ErrSt )
      snprintf( LogLastMsg + Ln, sizeof( LogLastMsg ) - Ln, "; Errno(%d): %s", Errno, ErrSt );
       
    va_end( ArgPt );
  }


  // update our global Last... variables
  LogLastServerity = Serverity;
  LogLastErrno = Errno;

  // control logging to stderr
  if(Serverity < LOG_WARNING || Serverity <= Log2Stderr )
    fprintf( stderr, "%s\n", LogLastMsg );

  // always to syslog
  syslog( Serverity, "%s", LogLastMsg );

  if( Serverity <= LOG_ERR )
    exit( -1 );
}
