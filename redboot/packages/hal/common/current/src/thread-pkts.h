//========================================================================
//
//      thread-pkts.h
//
//      Optional packet processing for thread aware debugging.
//
//========================================================================
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
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     Red Hat, nickg
// Contributors:  Red Hat, nickg
// Date:          1998-08-25
// Purpose:       
// Description:   Optional packet processing for thread aware debugging.
//                Externs as called by central packet switch routine
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================


/* The DEBUG_THREADS flag is usually set in board.h */

#if DEBUG_THREADS  /* activate this in board.h */

/* export the thread id's sent by gdb: */
extern int _gdb_cont_thread, _gdb_general_thread ;

extern char * stub_pack_Tpkt_threadid(char * pkt) ;

extern long stub_get_currthread(void) ;
extern int  stub_lock_scheduler(int lock, int kind, long id) ;

extern void stub_pkt_changethread(char * inbuf,
                                 char * outbuf,
                                 int bufmax) ;


extern void stub_pkt_getthreadlist(char * inbuf,
                                  char * outbuf,
                                  int bufmax) ;


extern void stub_pkt_getthreadinfo(
                  char * inbuf,
                  char * outbuf,
                  int bufmax) ;

extern void stub_pkt_thread_alive(
                                  char * inbuf,
                                  char * outbuf,
                                  int bufmax) ;

extern void stub_pkt_currthread(
                                char * inbuf,
                                char * outbuf,
                                int bufmax) ;

#define STUB_PKT_GETTHREADLIST(A,B,C) { stub_pkt_getthreadlist(A,B,C); }
#define STUB_PKT_GETTHREADINFO(A,B,C) { stub_pkt_getthreadinfo(A,B,C); }
#define STUB_PKT_CHANGETHREAD(A,B,C)  { stub_pkt_changethread(A,B,C);  }
#define STUB_PKT_THREAD_ALIVE(A,B,C)  { stub_pkt_thread_alive(A,B,C);  }
#define STUB_PKT_CURRTHREAD(A,B,C)    { stub_pkt_currthread(A,B,C);    }
#define PACK_THREAD_ID(PTR)           { PTR = stub_pack_Tpkt_threadid (PTR); }
#else
#define STUB_PKT_GETTHREADLIST(A,B,C) {}
#define STUB_PKT_GETTHREADINFO(A,B,C) {}
#define STUB_PKT_CHANGETHREAD(A,B,C)  {}
#define STUB_PKT_THREAD_ALIVE(A,B,C)  {}
#define STUB_PKT_CURRTHREAD(A,B,C)    {}
#define PACK_THREAD_ID(PTR)           {}
#endif
