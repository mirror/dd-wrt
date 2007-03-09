//========================================================================
//
//      thread-packets.c
//
//      Provides multi-threaded debug support
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
// Description:   Provides multi-threaded debug support
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================

// Define __ECOS__; allows all eCos specific additions to be easily identified.
#define __ECOS__


// #ifdef __ECOS__
#include <pkgconf/hal.h>

#if defined(CYGDBG_HAL_DEBUG_GDB_THREAD_SUPPORT) \
    && defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
// #endif // __ECOS__

/* FIXME: Scan this module for correct sizes of fields in packets */

#ifdef __ECOS__
#include <cyg/hal/dbg-threads-api.h>
#else  // __ECOS__
#include "dbg-threads-api.h"
#endif // __ECOS__

/* This file should ALWAYS define debug thread support */
/* Dont include the object in the link if you dont need the support */
/* This is NOT the internal unit debug flag, it is a feature control */
#if defined(DEBUG_THREADS)
#undef DEBUG_THREADS
#endif

#define DEBUG_THREADS 1
#define UNIT_TEST     0
#define GDB_MOCKUP    0


#define STUB_BUF_MAX 300 /* for range checking of packet lengths */
     
#include "thread-pkts.h"

#ifdef __ECOS__
// Use HAL rather than board.h in eCos
#include <cyg/hal/hal_stub.h>
#else  // __ECOS__
#include "board.h"
#endif // __ECOS__

/*
 * Export the continue and "general" (context) thread IDs from GDB.
 */
int _gdb_cont_thread ;
int _gdb_general_thread ;

#if !defined(PKT_DEBUG)
#define PKT_DEBUG 0
#endif
extern void output_string(char * message) ;

#if PKT_DEBUG
void output_threadid(char * title,threadref * ref) ;
#warning "PKT_DEBUG macros engaged"
#define PKT_TRACE(title,packet) \
{ output_string(title) ; output_string(packet) ; output_string("\n") ;}
#else
#define PKT_TRACE(title,packet) {}
#endif 


/* This is going to be irregular because the various implementations
   have adopted different names for registers.
   It would be nice to fix them to have a common convention
     _stub_registers
     stub_registers
     alt_stub_registers
 */

extern target_register_t * _registers ;
    /* A pointer to the current set of registers */
extern target_register_t registers[]; /* The current saved registers */
extern target_register_t alt_registers[] ;
    /* Thread or saved process state */


static void stub_copy_registers(
                           target_register_t * dest,
                           target_register_t *src
                           )
{
  target_register_t * limit ;
  limit = dest + NUMREGS ;

  while (dest < limit)  *dest++ = *src++ ;
}

#ifdef __ECOS__
void __stub_copy_registers(target_register_t * dest,
                           target_register_t *src)
{
    stub_copy_registers(dest, src);
}
#endif // __ECOS__

extern int stubhex(char ch) ;

/* ----- STUB_PACK_NAK ----------------------------------- */
/* Pack an error response into the response packet */

char * stub_pack_nak(char * outbuf)
{
  *outbuf++ = 'E' ;
  *outbuf++ = '0' ;
  *outbuf++ = '2' ;
  return outbuf ;
} /* stub_pack_nak */

/* ----- STUB_PACK_ACK -------------------------- */
/* Pack an OK achnowledgement */
char * stub_pack_ack(char * outbuf)
{
  *outbuf++ = 'O' ;
  *outbuf++ = 'K' ;
  return outbuf ;
} /* stub_pack_ack */

/* ------- STUB_UNPACK_INT ------------------------------- */
/* Unpack a few bytes and return its integer value         */
/* This is where I wish functions could return several values
   I would also advance the buffer pointer */

int stub_unpack_int(char * buff,int fieldlength)
{
  int retval = 0 ;
  int nibble ;
  while (fieldlength)
    { nibble = stubhex(*buff++) ;
      retval |= nibble ;
      fieldlength-- ;
      if (fieldlength) retval = retval << 4 ;
    }
  return retval ;
} /* stub_unpack_int */

static char * unpack_byte(char * buf, int * value)
{
  *value = stub_unpack_int(buf,2) ;
  return buf + 2 ;
}

static char * unpack_int(char * buf, int * value)
{
  *value = stub_unpack_int(buf,8) ;
  return buf + 8 ;
}

/* We are NOT depending upon extensive libraries */
static int ishex(char ch,int *val)
{
  if ((ch >= 'a') && (ch <= 'f'))
    { *val =ch - 'a' + 10 ; return 1 ; }
  if ((ch >= 'A') && (ch <= 'F'))
    { *val = ch - 'A' + 10 ; return 1 ;}
  if ((ch >= '0') && (ch <= '9'))
    { *val = ch - '0' ; return 1 ; }
  return 0 ;
} /* ishex */

static char * unpack_nibble(char * buf,int * val) 
{
  ishex(*buf++,val) ;
  return buf ;
}


static const char hexchars[] = "0123456789abcdef";

static char * pack_hex_byte(char * pkt, unsigned char byte)
{
  *pkt++ = hexchars[(byte >> 4) & 0xf] ;
  *pkt++ = hexchars[(byte & 0xf)] ;
  return pkt ;
} /* pack_hex_byte */

#ifndef __ECOS__
/* ---- STUB_PACK_VARLEN_HEX ------------------------------------- */
/* Format a variable length stream of hex bytes */

static char * pack_varlen_hex(
                       char * pkt,
                       unsigned int value)
{
  int i ;
  static unsigned char n[8] ;
  if (value == 0)
    {
      *pkt++ = '0' ;
      return pkt ;
    }
  else
    {
      i = 8 ;
      while (i-- >= 0 )  /* unpack nibbles into a char array */
        {
          n[i] = value & 0x0f ;
          value = value >> 4 ;
        }
      i = 0 ;                  /* we had decrmented it to -1 */
      while (n[i] == 0 ) i++ ; /* drop leading zeroes */
      while (i++ < 8) *pkt++ = hexchars[n[i]] ; /* pack the number */
    }
  return pkt ;
} /* pack_varlen_hex */
#endif // !__ECOS__


/* ------ STUB_UNPACK_VARLEN_HEX --------------------------------  */
/* Parse a stream of hex bytes which may be of variable length     */
/* return the pointer to the next char                             */
/* modify a varparm containing the result                          */
/* A failure would look like a non-increment of the buffer pointer */

/* This unpacks hex strings that may have been packed using sprintf(%x) */
/* We assume some non-hex delimits them */

char * unpack_varlen_hex(
                              char * buff,    /* packet to parse */
                              int * result)
{
  int nibble ;
  int retval ;
  retval = 0 ;

  while (ishex(*buff,&nibble))
    {
      buff++ ;
      retval = retval  << 4 ;
      retval |= nibble & 0x0f ;
    }
  *result = retval ;
  return buff ;
} /* stub_unpack_varlen_int */


/* ------ UNPACK_THREADID ------------------------------- */
/* A threadid is a 64 bit quantity                        */

#define BUFTHREADIDSIZ 16 /* encode 64 bits in 16 chars of hex */

static char * unpack_threadid(char * inbuf, threadref * id)
{
  char * altref ;
  char * limit  = inbuf + BUFTHREADIDSIZ ;
  int x,y ;
  altref = (char *) id ;

  while (inbuf < limit)
    {
      x = stubhex(*inbuf++) ;
      y = stubhex(*inbuf++) ;
      *altref++ = (x << 4) | y ;
    }
  return inbuf ;
} /* unpack_threadid */



/* Pack an integer use leading zeroes */
static char * pack_int(char * buf,int value)
{
  buf = pack_hex_byte(buf,(value>> 24)& 0xff) ;
  buf = pack_hex_byte(buf,(value >>16)& 0xff) ;
  buf = pack_hex_byte(buf,(value >>8) & 0x0ff) ;
  buf = pack_hex_byte(buf,(value & 0xff)) ;
  return buf ;
} /* pack_int */


/* -------- PACK_STRING ---------------------------------------------- */
/* This stupid string better not contain any funny characters */
/* Also, the GDB protocol will not cope with NULLs in the
   string or at the end of it.
   While is is posable to encapsulate the protocol in ays that
   preclude filtering for # I am assuming this is a constraint.
*/

static char * pack_raw_string(char * pkt,char * string)
{
  char ch ;
  while (0 != (ch = *string++)) *pkt++ = ch ;
  return pkt ;
}

static char * pack_string(
                          char * pkt,
                          char * string)
{
  char ch ;
#ifdef __ECOS__
  int len = 0;
  char *s = string;
  while( *s++ ) len++;
#else  // __ECOS__
  int len ;
  len = strlen(string) ;
#endif // __ECOS
  if (len > 200 ) len = 200 ; /* Bigger than most GDB packets, junk??? */
  pkt = pack_hex_byte(pkt,len) ;
  while (len-- > 0)
    { 
       ch = *string++ ;
       if ((ch == '\0') || (ch == '#')) ch = '*' ; /* Protect encapsulation */
       *pkt++ = ch ;
    }
  return pkt ;
} /* pack_string */


/* ----- STUB_PACK_THREADID  --------------------------------------------- */
/* Convert a binary 64 bit threadid  and pack it into a xmit buffer */
/* Return the advanced buffer pointer */

static char * pack_threadid(char * pkt, threadref * id)
{
  char * limit ;
  unsigned char * altid ;
  altid = (unsigned char *) id ;
  limit = pkt + BUFTHREADIDSIZ ;
  while (pkt < limit) pkt = pack_hex_byte(pkt,*altid++) ;
  return pkt ;
} /* stub_pack_threadid */

/* UNFORTUNATELY, not all of the extended debugging system has yet been
   converted to 64 but thread references and process identifiers.
   These routines do the conversion.
   An array of bytes is the correct treatment of an opaque identifier.
   ints have endian issues.
 */

static void int_to_threadref(threadref * id, int value)
{
  unsigned char * scan ;
  scan = (unsigned char *) id ;
   {
    int i = 4 ;
    while (i--) *scan++  = 0 ;
  }
  *scan++ = (value >> 24) & 0xff ;
  *scan++ = (value >> 16) & 0xff ;
  *scan++ = (value >> 8) & 0xff ;
  *scan++ = (value & 0xff) ;
}

static int threadref_to_int(threadref * ref)
{
  int value = 0 ;
  unsigned char * scan ;
  int i ;
  
  scan = (char *) ref ;
  scan += 4 ;
  i = 4 ;
  while (i-- > 0) value = (value << 8) | ((*scan++) & 0xff) ;
  return value ;
} /* threadref_to_int */

void copy_threadref(threadref * dest, threadref * src)
{
  int i ;
  unsigned char * csrc, * cdest ;
  csrc = (unsigned char *) src ;
  cdest = (unsigned char *) dest ;
  i = 8 ;
  while (i--) *cdest++ = *csrc++ ;
}


int threadmatch(
                threadref * dest ,
                threadref * src
                )
{
  unsigned char * srcp, * destp ;
  int i , result ;
  srcp = (char *) src ;
  destp = (char *) dest ;
  i = 8 ;
  result = 1 ;
  while (i-- > 0 ) result &= (*srcp++ == *destp++) ? 1 : 0 ;
  return result ;
} /* threadmatch */           



static char * Tpkt_threadtag = "thread:" ;

/* ----- STUB_PACK_TPKT_THREADID ------------------------------------ */
/* retreive, tag and insert a thread identifier into a T packet. */
/* Insert nothing if the thread identifier is not available */

char * stub_pack_Tpkt_threadid(char * pkt)
{
  static threadref thread ;
  int fmt = 0 ; /* old format */
  PKT_TRACE("Tpkt-id","---") ;
  if (dbg_currthread(&thread))
    {
      pkt = pack_raw_string(pkt,Tpkt_threadtag) ;
      if (fmt)
        pkt = pack_threadid(pkt,&thread) ;
      else
          /* Until GDB lengthens its thread ids, we have to MASH
             the threadid into somthing shorter. PLEASE FIX GDB */
          pkt = pack_int(pkt,threadref_to_int(&thread)) ;
      *pkt++ = ';'  ; /* terminate variable length int */
      *pkt = '\0' ; /* Null terminate to allow string to be printed, no++ */
    }
  PKT_TRACE("packedTpkt","--") ;
  return pkt ;
} /* stub_pack_Tpkt_threadid */

long stub_get_currthread (void)
{
  threadref thread ;
  
  if (dbg_currthread(&thread))
    return threadref_to_int(&thread) ;
  else
    return 0 ;
}

void stub_pkt_currthread(
                                char * inbuf,
                                char * outbuf,
                                int bufmax)
{
  threadref thread ;
  char * base_out ;
  base_out = outbuf ;
  
  if (dbg_currthread(&thread))
    {
      *outbuf++ = 'Q' ;
      *outbuf++ = 'C' ; /* FIXME: Is this a reasonable code */
      outbuf = pack_int(outbuf, threadref_to_int(&thread)) ; /* Short form */
    }
  else outbuf = stub_pack_nak(outbuf) ;
  *outbuf = '\0' ; /* terminate response packet */
  PKT_TRACE("stub_pkt_currthread(resp) ",base_out) ;
} /* stub_pkt_currthread */

/* ----- STUB_PKT_THREAD_ALIVE --------------------------------- */
/* Answer the thread alive query */

static int thread_alive (int id)
{
  threadref thread ;
  struct cygmon_thread_debug_info info ;

  int_to_threadref(&thread, id) ;
  if (dbg_threadinfo(&thread, &info) &&
      info.context_exists)
    return 1 ;
  else
    return 0 ;
}

void stub_pkt_thread_alive(char * inbuf,
                           char * outbuf,
                           int bufmax)
{
  char * prebuf = inbuf ;
  int result ;
  
  if (prebuf != (inbuf = unpack_varlen_hex(inbuf,&result)))
    {
      if (thread_alive(result))
        {
          outbuf = stub_pack_ack(outbuf) ;
          *outbuf = '\0' ;
          return ;
        }
    }
  outbuf = stub_pack_nak(outbuf) ;
  *outbuf = '\0' ; /* terminate the response message */
} /* stub_pkt_thread_alive */

 

/* ----- STUB_PKT_CHANGETHREAD ------------------------------- */
/* Switch the display of registers to that of a saved context */

/* Changing the context makes NO sense, although the packets define the
   capability. Therefore, the option to change the context back does
   call the function to change registers. Also, there is no
   forced context switch.
     'p' - New format, long long threadid, no special cases
     'c' - Old format, id for continue, 32 bit threadid max, possably less
                       -1 means continue all threads
     'g' - Old Format, id for general use (other than continue)

     replies:
          OK for success
          ENN for error
   */

void stub_pkt_changethread(
                           char * inbuf,
                           char * outbuf,
                           int bufmax)
{
  threadref id ;
  int idefined = -1 ;
  char ch ;
  PKT_TRACE("setthread-pkt ",inbuf) ;

  /* Parse the incoming packet for a thread identifier */
  switch (ch = *inbuf++ )  /* handle various packet formats */
    {
    case 'p' : /* New format: mode:8,threadid:64 */
      inbuf = unpack_nibble(inbuf,&idefined) ;
      inbuf = unpack_threadid(inbuf,&id) ; /* even if startflag */
      break ;  
    case 'c' : /* old format , specify thread for continue */
      if (inbuf[0] == '-' && inbuf[1] == '1')   /* Hc-1 */
        _gdb_cont_thread = 0 ;
      else
        inbuf = unpack_varlen_hex(inbuf, &_gdb_cont_thread) ;

      if (_gdb_cont_thread == 0 ||              /* revert to any old thread */
          thread_alive(_gdb_cont_thread))       /* specified thread is alive */
        outbuf = stub_pack_ack(outbuf) ;
      else
        outbuf = stub_pack_nak(outbuf) ;
      break ;
    case 'g' : /* old format, specify thread for general operations */
      /* OLD format: parse a variable length hex string */
      /* OLD format consider special thread ids */
      {
        inbuf = unpack_varlen_hex(inbuf, &_gdb_general_thread) ;
        int_to_threadref(&id, _gdb_general_thread) ;
        switch (_gdb_general_thread)
          {
          case  0 : /* pick a thread, any thread */
            idefined = 2 ; /* select original interrupted context */
            break ;
          case -1 : /* all threads */
            idefined = 2 ;
            break ;
          default :
            idefined = 1 ; /* select the specified thread */
            break ;
          }
      }
      break ;
    default:
      outbuf = stub_pack_nak(outbuf) ;
      break ;
    } /* handle various packet formats */

  switch (idefined)
    {
    case -1 :
      /* Packet not supported, already NAKed, no further action */
      break ;
    case 0 :
      /* Switch back to interrupted context */
      _registers = &registers[0] ;
      break ;
    case 1 :
      /* copy the saved registers into the backup registers */
      stub_copy_registers(alt_registers,registers) ;
      /* The OS will now update the values it has in a saved process context*/
      if (dbg_getthreadreg(&id,NUMREGS,&alt_registers[0]))
        {
          /* switch the registers pointer */
          _registers = &alt_registers[0] ;
          outbuf = stub_pack_ack(outbuf) ; 
        }
      else
          outbuf = stub_pack_nak(outbuf) ;
      break ;
    case 2 :
      /* switch to interrupted context */ 
      outbuf = stub_pack_ack(outbuf) ;
      break ;
    default:
      outbuf = stub_pack_nak(outbuf) ;
      break ;
    }
  *outbuf = '\0' ; /* Terminate response pkt */
} /* stub_pkt_changethread */


/* ---- STUB_PKT_GETTHREADLIST ------------------------------- */
/* Get a portion of the threadlist  or process list            */
/* This may be part of a multipacket transaction               */
/* It would be hard to tell in the response packet the difference
   between the end of list and an error in threadid encoding.
   */

void stub_pkt_getthreadlist(char * inbuf,
                            char * outbuf,
                            int bufmax)
{
  char * count_ptr ;
  char * done_ptr ;
  char * limit ;
  int start_flag , batchsize , result , count ;
  static threadref lastthread, nextthread ;

#if PKT_DEBUG
  char * r_base = outbuf ;
#endif  
  PKT_TRACE("pkt_getthreadlist: ",inbuf) ;

  count = 0 ;
  inbuf = unpack_nibble(inbuf,&start_flag) ;
  inbuf = unpack_byte(inbuf,&batchsize) ;
  inbuf = unpack_threadid(inbuf,&lastthread) ; /* even if startflag */

  /*   Start building response packet    */
  limit = outbuf + (bufmax - BUFTHREADIDSIZ - 10) ; /* id output packing limit */
  *outbuf++ = 'Q'  ;
  *outbuf++ = 'M'  ;
  
  /* Default values for count and done fields, save ptr to repatch */
  count_ptr = outbuf ; /* save to repatch count */
  outbuf = pack_hex_byte(outbuf,0) ;
  done_ptr = outbuf ;  /* Backpatched later */
  *outbuf++ = '0' ;    /* Done = 0 by default */
  outbuf = pack_threadid(outbuf,&lastthread) ;
  
  /* Loop through the threadid packing */
  while ((outbuf < limit) && (count < batchsize))
    {
      result = dbg_threadlist(start_flag,&lastthread,&nextthread) ;
      start_flag = 0 ; /* redundant but effective */
      if (!result)
        { *done_ptr = '1' ;   /* pack the done flag */
          break ;
        }
#if 0 /* DEBUG */
      if (threadmatch(&lastthread,&nextthread))
        {
          output_string("FAIL: Threadlist, not incrementing\n") ;
          *done_ptr = '1' ;
          break ;
        }
#endif      
      count++ ;
      outbuf = pack_threadid(outbuf,&nextthread) ;
      copy_threadref(&lastthread,&nextthread) ;
    }
  pack_hex_byte(count_ptr,count) ;/* backpatch, Pack the count field */
  *outbuf = '\0' ;
  PKT_TRACE("pkt_getthreadlist(resp) ",r_base) ;
} /* pkt_getthreadlist */




/* ----- STUB_PKT_GETTHREADINFO ---------------------------------------- */
/* Get the detailed information about a specific thread or process */

/*
Encoding:
 'Q':8,'P':8,mask:16

 Mask Fields
        threadid:1,        # always request threadid 
        context_exists:2,
        display:4,          
        unique_name:8,
        more_display:16
 */  

void stub_pkt_getthreadinfo(
                            char * inbuf,
                            char * outbuf,
                            int bufmax)
{
  int mask ;
  int result ;
  threadref thread ;
  struct cygmon_thread_debug_info info ;

  info.context_exists = 0 ;
  info.thread_display = 0 ;
  info.unique_thread_name = 0 ;
  info.more_display = 0 ;

  /* Assume the packet identification chars have already been
     discarded by the packet demultiples routines */
  PKT_TRACE("PKT getthreadinfo",inbuf) ;
  
  inbuf = unpack_int(inbuf,&mask) ;
  inbuf = unpack_threadid(inbuf,&thread) ;
  
  result = dbg_threadinfo(&thread,&info) ; /* Make system call */

  if (result)
    {
      *outbuf++ = 'q' ;
      *outbuf++ = 'p' ;
      outbuf = pack_int(outbuf,mask) ;
      outbuf = pack_threadid(outbuf,&info.thread_id) ; /* echo threadid */
      if (mask & 2)   /* context-exists */
        {
          outbuf = pack_int(outbuf,2) ; /* tag */
          outbuf = pack_hex_byte(outbuf,2) ; /* length */
          outbuf = pack_hex_byte(outbuf,info.context_exists) ;
        }
      if ((mask & 4)  && info.thread_display)/* display */
        {
          outbuf = pack_int(outbuf,4) ; /* tag */
          outbuf = pack_string(outbuf,info.thread_display) ;
        }
      if ((mask & 8) && info.unique_thread_name) /* unique_name */
        {
          outbuf = pack_int(outbuf,8) ;
          outbuf = pack_string(outbuf,info.unique_thread_name) ;
        }
      if ((mask & 16) && info.more_display)  /* more display */
        {
          outbuf = pack_int(outbuf,16) ; /* tag 16 */
          outbuf = pack_string(outbuf,info.more_display) ;
        }
    }
  else
    {
      PKT_TRACE("FAIL: dbg_threadinfo\n", "") ;
      outbuf = stub_pack_nak(outbuf) ;
    }
  *outbuf = '\0' ;
} /* stub_pkt_getthreadinfo */

int stub_lock_scheduler(int lock,       /* 0 to unlock, 1 to lock */
                        int mode,       /* 0 for step,  1 for continue */
                        long id)        /* current thread */
{
  threadref thread;

  int_to_threadref(&thread, id) ;
  return dbg_scheduler(&thread, lock, mode) ;
}


#if GDB_MOCKUP
/* ------ MATCHED GDB SIDE PACKET ENCODING AND PARSING ----------------- */

char * pack_nibble(char * buf, int nibble)
{
  *buf++ =  hexchars[(nibble & 0x0f)] ;
  return buf ;
} /* pack_nibble */

#if 0
static char * unpack_short(char * buf,int * value)
{
  *value = stub_unpack_int(buf,4) ;
  return buf + 4 ;
}

static char * pack_short(
                  char * buf,
                  unsigned int value)
{
  buf = pack_hex_byte(buf,(value >> 8) & 0xff) ;
  buf = pack_hex_byte(buf,(value & 0xff)) ;
  return buf ;
} /* pack_short */
#endif


/* Generally, I dont bury xmit and receive calls inside packet formatters
   and parsers
   */




/* ----- PACK_SETTHREAD_REQUEST ------------------------------------- */
/*      Encoding: ??? decode gdb/remote.c
        'Q':8,'p':8,idefined:8,threadid:32 ;
        */

char * pack_setthread_request(
                       char * buf,
                       char fmt,   /* c,g or, p */
                       int idformat ,
                       threadref *  threadid )
{
  *buf++ = fmt ;
  
  if (fmt == 'p')
    {  /* pack the long form */
      buf = pack_nibble(buf,idformat) ;
      buf = pack_threadid(buf,threadid)  ;
    }
  else
    {  /* pack the shorter form - Serious truncation */
      /* There are reserved identifieds 0 , -1 */
      int quickref = threadref_to_int(threadid) ;
      buf = pack_varlen_hex(buf,quickref) ;
    }
  *buf++ = '\0' ; /* end_of_packet  */
  return buf ;
} /* pack_setthread_request */



/* -------- PACK_THREADLIST-REQUEST --------------------------------- */
/*    Format: i'Q':8,i"L":8,initflag:8,batchsize:16,lastthreadid:32   */


char * pack_threadlist_request(
                               char * pkt,
                               int startflag,
                               int threadcount,
                               threadref * nextthread 
                               )
{
  *pkt++ = 'q' ;
  *pkt++ = 'L' ;
  pkt = pack_nibble(pkt,startflag) ;     /* initflag 1 bytes */
  pkt = pack_hex_byte(pkt,threadcount) ;   /* threadcount 2 bytes */
  pkt = pack_threadid(pkt,nextthread) ;        /* 64 bit thread identifier */
  *pkt = '\0' ;
  return pkt ;
} /* remote_threadlist_request */




/* ---------- PARSE_THREADLIST_RESPONSE ------------------------------------ */
/* Encoding:   'q':8,'M':8,count:16,done:8,argthreadid:64,(threadid:64)* */

int parse_threadlist_response(
                              char * pkt,
                              threadref * original_echo,
                              threadref * resultlist,
                              int * doneflag)
{
  char * limit ;
  int count, resultcount , done ;
  resultcount = 0 ;

  /* assume the 'q' and 'M chars have been stripped */
  PKT_TRACE("parse-threadlist-response ",pkt) ;
  limit = pkt + (STUB_BUF_MAX - BUFTHREADIDSIZ) ; /* done parse past here */
  pkt = unpack_byte(pkt,&count)  ;                /* count field */
  pkt = unpack_nibble(pkt,&done) ;
  /* The first threadid is the argument threadid */
  pkt = unpack_threadid(pkt,original_echo) ; /* should match query packet */
  while ((count-- > 0) && (pkt < limit))
    {
      pkt = unpack_threadid(pkt,resultlist++) ;
      resultcount++ ;
    }
  if (doneflag) *doneflag = done ;
  return resultcount ; /* successvalue */
} /* parse_threadlist_response */
 
struct gdb_ext_thread_info
{
  threadref threadid ;
  int active ;
  char display[256] ;
  char shortname[32] ;
  char more_display[256] ;
} ;


/* ----- PACK_THREAD_INFO_REQUEST -------------------------------- */

/* 
   threadid:1,        # always request threadid
   context_exists:2,
   display:4,
   unique_name:8,
   more_display:16
 */

/* Encoding:  'Q':8,'P':8,mask:32,threadid:64 */

char * pack_threadinfo_request(char * pkt,
                                int mode,
                                threadref * id 
                                )
{
  *pkt++ = 'Q' ;
  *pkt++ = 'P' ;
  pkt = pack_int(pkt,mode) ; /* mode */
  pkt = pack_threadid(pkt,id) ; /* threadid */
  *pkt = '\0' ; /* terminate */
  return pkt ;
} /* pack_thread_info_request */



static char * unpack_string(
                            char * src,
                            char * dest,
                            int length)
{
  while (length--) *dest++ = *src++ ;
  *dest = '\0' ;
  return src ;
} /* unpack_string */


void output_threadid(char * title,threadref * ref)
{
  char hexid[20] ;
  pack_threadid(&hexid[0],ref) ; /* Convert threead id into hex */
  hexid[16] = 0 ;
  output_string(title) ; 
  output_string(&hexid[0]) ; 
  output_string("\n") ;
}

/* ------ REMOTE_UPK_THREAD_INFO_RESPONSE ------------------------------- */
/* Unpack the response of a detailed thread info packet */
/* Encoding:  i'Q':8,i'R':8,argmask:16,threadid:64,(tag:8,length:16,data:x)* */

#define TAG_THREADID 1
#define TAG_EXISTS 2
#define TAG_DISPLAY 4
#define TAG_THREADNAME 8
#define TAG_MOREDISPLAY 16 


int remote_upk_thread_info_response(
                               char * pkt,
                               threadref * expectedref ,
                               struct gdb_ext_thread_info * info)
{
  int mask, length ;
  unsigned int tag ;
  threadref ref ;
  char * limit = pkt + 500 ; /* plausable parsing limit */
  int retval = 1 ;

  PKT_TRACE("upk-threadinfo ",pkt) ;

  /* info->threadid = 0 ; FIXME: implement zero_threadref */
  info->active = 0 ;
  info->display[0] = '\0' ;
  info->shortname[0] = '\0' ;
  info->more_display[0] = '\0' ;

  /* Assume the characters indicating the packet type have been stripped */
  pkt = unpack_int(pkt,&mask) ;  /* arg mask */
  pkt = unpack_threadid(pkt , &ref) ;
                      
  if (! threadmatch(&ref,expectedref))
    { /* This is an answer to a different request */
      output_string("FAIL Thread mismatch\n") ;
      output_threadid("ref ",&ref) ;
      output_threadid("expected ",expectedref) ;
      return 0 ;
    }
  copy_threadref(&info->threadid,&ref) ;
  
  /* Loop on tagged fields , try to bail if somthing goes wrong */
  if (mask==0)  output_string("OOPS NO MASK \n") ;

  while ((pkt < limit) && mask && *pkt)  /* packets are terminated with nulls */
    {
      pkt = unpack_int(pkt,&tag) ;            /* tag */
      pkt = unpack_byte(pkt,&length) ;   /* length */
      if (! (tag & mask))  /* tags out of synch with mask */
        {
          output_string("FAIL: threadinfo tag mismatch\n") ;
          retval = 0 ;
          break ;
        }
      if (tag == TAG_THREADID)
        {
          output_string("unpack THREADID\n") ;
          if (length != 16)
            {
              output_string("FAIL: length of threadid is not 16\n") ;
              retval = 0 ;
              break ;
            }
          pkt = unpack_threadid(pkt,&ref) ;
          mask = mask & ~ TAG_THREADID ;
          continue ;
        }
      if (tag == TAG_EXISTS)
        {
          info->active = stub_unpack_int(pkt,length) ;
          pkt += length ;
          mask = mask & ~(TAG_EXISTS) ;
          if (length > 8)
            {
              output_string("FAIL: 'exists' length too long\n") ;
              retval = 0 ;
              break ;
            }
          continue ;
        }
      if (tag == TAG_THREADNAME)
        {
          pkt = unpack_string(pkt,&info->shortname[0],length) ;
          mask = mask & ~TAG_THREADNAME ;
          continue ;
        }
      if (tag == TAG_DISPLAY)
        { 
          pkt = unpack_string(pkt,&info->display[0],length) ;
          mask = mask & ~TAG_DISPLAY ;
          continue ;
        }
      if (tag == TAG_MOREDISPLAY)
        { 
          pkt = unpack_string(pkt,&info->more_display[0],length) ;
          mask = mask & ~TAG_MOREDISPLAY ;
          continue ;
        }
      output_string("FAIL: unknown info tag\n") ;
      break ; /* Not a tag we know about */
    }
  return retval  ;
} /* parse-thread_info_response */


/* ---- REMOTE_PACK_CURRTHREAD_REQUEST ---------------------------- */
/* This is a request to emit the T packet */

/* FORMAT: 'q':8,'C' */

char * remote_pack_currthread_request(char * pkt )
{
  *pkt++ = 'q' ;
  *pkt++ = 'C' ;
  *pkt = '\0' ;
  return pkt ;
} /* remote_pack_currthread_request */


/* ------- REMOTE_UPK_CURTHREAD_RESPONSE ----------------------- */
/* Unpack the interesting part of a T packet */


int remote_upk_currthread_response(
                               char * pkt,
                               int *thr )  /* Parse a T packet */
{
  int retval = 0 ;
  PKT_TRACE("upk-currthreadresp ",pkt) ;

#if 0
  {
    static char threadtag[8] =  "thread" ;
    int retval = 0 ;
    int i , found ;
    char ch ;
    int quickid ;

  /* Unpack as a t packet */
  while (((ch = *pkt++) != ':')    /* scan for : thread */
         && (ch != '\0'))          /* stop at end of packet */

    {
      found = 0 ;
      i = 0 ;
      while ((ch = *pkt++) == threadtag[i++]) ;
      if (i == 8) /* string match "thread" */
        {
          pkt = unpack_varlen_hex(pkt,&quickid) ;
          retval = 1;
          break ;
        }
      retval = 0 ;
    }
  }
#else
  pkt = unpack_threadid(pkt, thr) ;
  retval = 1 ;
#endif  
  return retval ;
} /* remote_upk_currthread_response */


/* -------- REMOTE_UPK-SIMPLE_ACK --------------------------------- */
/* Decode a response which is eother "OK" or "Enn"
   fillin error code,  fillin pkfag-1== undef, 0==nak, 1 == ack ;
   return advanced packet pointer */


char * remote_upk_simple_ack(
                             char * buf,
                             int * pkflag,
                             int * errcode)
{
  int lclerr = 0 ;
  char ch = *buf++ ;
  int retval = -1 ;  /* Undefined ACK , a protocol error */
  if (ch == 'E')     /* NAK */
    {
      buf = unpack_byte(buf,&lclerr) ;
      retval = 0 ;   /* transaction failed, explicitly */
    }
  else
    if ((ch == 'O') && (*buf++ == 'K')) /* ACK */
      retval = 1 ; /* transaction succeeded */
  *pkflag = retval ;
  *errcode = lclerr ;
  return buf ;
} /* remote-upk_simple_ack */


/* -------- PACK_THREADALIVE_REQUEST ------------------------------- */

char * pack_threadalive_request(
                         char * buf,
                         threadref * threadid)
{
  *buf++ = 'T' ;
  buf = pack_threadid(buf,threadid) ;
  *buf = '\0' ;
  return buf ;
} /* pack_threadalive_request */
     
#endif /* GDB_MOCKUP */

/* ---------------------------------------------------------------------- */
/* UNIT_TESTS SUBSECTION                                                  */
/* ---------------------------------------------------------------------- */


#if UNIT_TEST
extern void output_string(char * message) ;
static char test_req[400] ;
static char t_response[400] ;



/* ----- DISPLAY_THREAD_INFO ---------------------------------------------- */
/*  Use local cygmon string output utiities */

void display_thread_info(struct gdb_ext_thread_info * info)
{

  output_threadid("Threadid: ",&info->threadid) ;
  /* short name */
  output_string("Name: ") ; output_string(info->shortname) ; output_string("\n");
  /* format display state */
  output_string("State: ") ; output_string(info->display) ; output_string("\n") ;
  /* additional data */
  output_string("other: ");output_string(info->more_display);
   output_string("\n\n");
} /* display_thread_info */


/* --- CURRTHREAD-TEST -------------------------------------------- */
static int currthread_test(threadref * thread)
{
  int result ;
  int threadid ;
  output_string("TEST: currthread\n") ;
  remote_pack_currthread_request(test_req) ;
  stub_pkt_currthread(test_req+2,t_response,STUB_BUF_MAX) ;
  result = remote_upk_currthread_response(t_response+2, &threadid) ;
  if (result)
    {
      output_string("PASS getcurthread\n") ;
      /* FIXME: print the thread */
    }
  else
    output_string("FAIL getcurrthread\n") ;
  return result ;
} /* currthread_test */

/* ------ SETTHREAD_TEST ------------------------------------------- */
  /* use a known thread from previous test */

static int setthread_test(threadref * thread)
{
  int result, errcode ;
  output_string("TEST: setthread\n") ;
  
  pack_setthread_request(test_req,'p',1,thread) ;
  stub_pkt_changethread(test_req,t_response,STUB_BUF_MAX) ;
  remote_upk_simple_ack(t_response,&result,&errcode) ;
  switch (result)
    {
    case 0 :
      output_string("FAIL setthread\n") ;
      break ;
    case 1 :
      output_string("PASS setthread\n") ;
      break ;
    default :
      output_string("FAIL setthread -unrecognized response\n") ;
      break ;
    }
  return result ;
} /* setthread_test */


/* ------ THREADACTIVE_TEST ---------------------- */
  /* use known thread */
  /* pack threadactive packet */
  /* process threadactive packet */
  /* parse threadactive response */
  /* check results */


int threadactive_test(threadref * thread)
{
  int result ;
  int errcode ;
  output_string("TEST: threadactive\n") ;
  pack_threadalive_request(test_req,thread) ;
  stub_pkt_thread_alive(test_req+1,t_response,STUB_BUF_MAX);
  remote_upk_simple_ack(t_response,&result,&errcode) ;
  switch (result)
    {
    case 0 :
      output_string("FAIL threadalive\n") ;
      break ;
    case 1 :
      output_string("PASS threadalive\n") ;
      break ;
    default :
      output_string("FAIL threadalive -unrecognized response\n") ;
      break ;
    }
  return result ;
} /* threadactive_test */

/* ------ REMOTE_GET_THREADINFO -------------------------------------- */
int remote_get_threadinfo(
                           threadref * threadid,
                           int fieldset , /* TAG mask */
                           struct gdb_ext_thread_info * info
                          )
{
  int result ;
  pack_threadinfo_request(test_req,fieldset,threadid) ;
  stub_pkt_getthreadinfo(test_req+2,t_response,STUB_BUF_MAX) ;
  result = remote_upk_thread_info_response(t_response+2,threadid,info) ;
  return result ;
} /* remote_get-thrreadinfo */


static struct gdb_ext_thread_info test_info ;

static int get_and_display_threadinfo(threadref * thread)
{
  int mode ;
  int result ;
  /* output_string("TEST: get and display threadinfo\n") ; */

  mode = TAG_THREADID | TAG_EXISTS | TAG_THREADNAME
    | TAG_MOREDISPLAY | TAG_DISPLAY ;
  result = remote_get_threadinfo(thread,mode,&test_info) ;
  if (result) display_thread_info(&test_info) ;
#if 0  /* silent subtest */
  if (result)
      output_string("PASS: get_and_display threadinfo\n") ;
  else
      output_string("FAIL: get_and_display threadinfo\n") ;
#endif  
  return result ;
} /* get-and-display-threadinfo */



/* ----- THREADLIST_TEST ------------------------------------------ */
#define TESTLISTSIZE 16
#define TLRSIZ 2
static threadref test_threadlist[TESTLISTSIZE] ;

static int threadlist_test(void)
{
  int done, i , result_count ;
  int startflag = 1 ;
  int result = 1 ;
  int loopcount = 0 ;
  static threadref nextthread ;
  static threadref echo_nextthread ;

  output_string("TEST: threadlist\n") ;

  done = 0 ;
  while (! done)
    {
      if (loopcount++ > 10)
        {
          result = 0 ;
          output_string("FAIL: Threadlist test -infinite loop-\n") ;
          break ;
        }
      pack_threadlist_request(test_req,startflag,TLRSIZ,&nextthread) ;
      startflag = 0 ; /* clear for later iterations */
      stub_pkt_getthreadlist(test_req+2,t_response,STUB_BUF_MAX);
      result_count = parse_threadlist_response(t_response+2,
                                               &echo_nextthread,
                                               &test_threadlist[0],&done) ;
      if (! threadmatch(&echo_nextthread,&nextthread))
        {
          output_string("FAIL: threadlist did not echo arg thread\n");
          result = 0 ;
          break ;
        }
      if (result_count <= 0)
        {
          if (done != 0)
              { output_string("FAIL threadlist_test, failed to get list");
                result = 0 ;
              }
          break ;
        }
      if (result_count > TLRSIZ)
        {
          output_string("FAIL: threadlist response longer than requested\n") ;
          result = 0 ;
          break ;
        }
      /* Setup to resume next batch of thread references , set nestthread */
      copy_threadref(&nextthread,&test_threadlist[result_count-1]) ;
      /* output_threadid("last-of-batch",&nextthread) ; */
      i = 0 ;
      while (result_count--)
          {
            if (0)  /* two display alternatives */
              output_threadid("truncatedisplay",&test_threadlist[i++]) ;
            else
              get_and_display_threadinfo(&test_threadlist[i++]) ; 
          }

    }
  if (!result)
    output_string("FAIL: threadlist test\n") ;
  else output_string("PASS: Threadlist test\n") ;
  return result ;
} /* threadlist_test */


static threadref testthread ;


int test_thread_support(void)
{
  int result = 1 ;
  output_string("TESTING Thread support infrastructure\n") ;
  stub_pack_Tpkt_threadid(test_req) ;
  PKT_TRACE("packing the threadid  -> ",test_req) ;
  result &= currthread_test(&testthread) ;
  result &= get_and_display_threadinfo(&testthread) ;
  result &= threadlist_test() ;
  result &= setthread_test(&testthread) ;
  if (result)
    output_string("PASS: UNITTEST Thread support\n") ;
  else
        output_string("FAIL: UNITTEST Thread support\n") ;
  return result ;
} /* test-thread_support */
#endif /* UNIT_TEST */

// #ifdef __ECOS__
#endif // ifdef CYGDBG_HAL_DEBUG_GDB_THREAD_SUPPORT...
// #endif // __ECOS__
