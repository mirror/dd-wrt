//==========================================================================
//
//      src/chat.c
//
//      PPP CHAT scripting
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 eCosCentric Ltd.
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
// Alternative licenses for eCos may be arranged by contacting the
// copyright holder.
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    nickg
// Contributors: nickg
// Date:         2003-06-08
// Purpose:      Chat script handling
// Description: 
//
//####DESCRIPTIONEND####
//
//==========================================================================

//=====================================================================

#include <pkgconf/system.h>
#include <pkgconf/net.h>
#include <pkgconf/io_fileio.h>


#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "cyg/ppp/syslog.h"

#include <cyg/io/io.h>
#include <cyg/io/serial.h>

#include <cyg/kernel/kapi.h>

#include <cyg/ppp/ppp.h>

//=====================================================================

//#undef db_printf
//#define db_printf diag_printf

//=====================================================================
// Expect return values

#define CHAT_MATCH      0
#define CHAT_ABORT      1
#define CHAT_TIMEOUT    2
#define CHAT_FAIL       -1

//=====================================================================
// Configuration constants

#define CHAT_MAX_ABORTS CYGNUM_PPP_CHAT_ABORTS_MAX
#define CHAT_ABORT_SIZE CYGNUM_PPP_CHAT_ABORTS_SIZE

#define CHAT_STRING_LENGTH CYGNUM_PPP_CHAT_STRING_LENGTH

//=====================================================================
// Local variables

// Array of ABORT strings
static char cyg_ppp_chat_abort[CHAT_MAX_ABORTS][CHAT_ABORT_SIZE];
static int cyg_ppp_chat_abort_count;

// Response timeout
static int cyg_ppp_chat_timeout;
// timeout alarm
static cyg_alarm cyg_ppp_chat_alarm_obj;
static cyg_handle_t cyg_ppp_chat_alarm;
static cyg_handle_t cyg_ppp_chat_thread;

// Handle on serial device
static cyg_io_handle_t cyg_ppp_chat_handle;

// String buffers
char cyg_ppp_chat_buffer[CHAT_STRING_LENGTH];
static char cyg_ppp_chat_expect_buffer[CHAT_STRING_LENGTH];

//=====================================================================
// Timeout alarm function
//
// The alarm is set whenever we start looking for a match, if the alarm
// goes off before we find a match, then it kicks the thread out of any
// wait it is in.

static void chat_alarm(cyg_handle_t alarm, cyg_addrword_t data)
{
    db_printf("%s(%d)\n",__PRETTY_FUNCTION__,__LINE__);    
    cyg_thread_release( cyg_ppp_chat_thread );
}

//=====================================================================
// Match
//
// Looks for the supplied string in the input. Returns zero on timeout
// or other error, 1 if a match is found.

static int cyg_ppp_chat_match( char *str )
{
    cyg_uint64 trigger = (cyg_ppp_chat_timeout * 100) + cyg_current_time();
    char *s = cyg_ppp_chat_buffer;
    int matchlen = strlen(str);
    int matched = 0;
    int aborted = 0;

    db_printf("%s(%d) timeout %d trigger %d\n",__PRETTY_FUNCTION__,__LINE__,
              cyg_ppp_chat_timeout,(long)trigger);    
    db_printf(" current time %d\n",(long)cyg_current_time());
    
    if( matchlen == 0 )
        return 1;
    
    cyg_alarm_initialize( cyg_ppp_chat_alarm,
                          trigger,
                          0 );

    while(!(matched || aborted))
    {
        Cyg_ErrNo err;
        char c;
        cyg_uint32 len = 1;
        int i;

        err = cyg_io_read( cyg_ppp_chat_handle, &c, &len );

        if( err != 0 )
        {
            db_printf("%s(%d) err %d\n",__PRETTY_FUNCTION__,__LINE__,err);
            break;
        }

        db_printf("%c",c);        

        *s++ = c;

        // If we have enough characters to match the expect string,
        // and the current character matches the last character of the
        // expect string, then it is worth trying to compare the strings.
        
        if( s - cyg_ppp_chat_buffer >= matchlen &&
            c == str[matchlen - 1] &&
            strncmp( s - matchlen, str, matchlen ) == 0 )
        {
            matched = 1;
            break;
        }

        for( i = 0 ; i < cyg_ppp_chat_abort_count; i++ )
        {
            int abortlen = strlen( cyg_ppp_chat_abort[i] );

            if( s - cyg_ppp_chat_buffer >= abortlen &&
                strncmp( s - abortlen, cyg_ppp_chat_abort[i], abortlen ) == 0 )
            {
                syslog(LOG_ERR,"ABORT: %s\n",cyg_ppp_chat_abort[i]);
                aborted = 1;
                break;
            }
        }

        // If we have reached the end of the buffer, shuffle it down
        // leaving the last matchlen characters in place.
        if( s >= &cyg_ppp_chat_buffer[sizeof(cyg_ppp_chat_buffer)-1] )
        {
            strncpy( cyg_ppp_chat_buffer, s-matchlen, matchlen );
            s = cyg_ppp_chat_buffer + matchlen;
        }
    }

    cyg_alarm_disable( cyg_ppp_chat_alarm );

    return matched;
}

//=====================================================================
// Send string
//
// Send the string to the remote end, dealing with any escapes or
// special values. We currently implement the "EOT" and "BREAK"
// strings (or would if the device drivers did), and just the \c
// escape sequence.

static int cyg_ppp_chat_send( const char *s )
{
    cyg_bool crlf = true;
    
    db_printf("%s(%d) %s\n",__PRETTY_FUNCTION__,__LINE__,s);    
    
    if( strcmp( s, "EOT" ) == 0 )
    {
        s = "\x04";
        crlf = false;
    }
    else if( strcmp( s, "BREAK" ) == 0 )
    {
        // cannot do this at present
    }

    for( ; *s != 0 ; s++ )
    {
        char c = *s;
        cyg_uint32 one = 1;

        // Look for escape sequences and process them
        if( c == '\\' )
        {
            c = *++s;

            if( c == 0 )
                break;
            
            switch( c )
            {
            case 'c':
                crlf = false;
                continue;
            }
        }

        cyg_io_write( cyg_ppp_chat_handle, &c, &one );
    }

    if( crlf )
    {
        cyg_uint32 len = 2;
        s = "\r\n";
        cyg_io_write( cyg_ppp_chat_handle, s, &len );
    }

    return 0;
}

//=====================================================================
// find_sep
//
// Split the pointed-to string at the next '-' character.

static char *find_sep( char **str )
{
    char *s = *str;
    char *res = s;

    if( *s == 0 )
        return NULL;
    
    while( *s != 0 && *s != '-' )
        s++;

    if( *s == '-' )
        *s++ = 0;

    *str = s;
    return res;
}


//=====================================================================
// expect processing
//
// Parse the expect string. The return value indicates what it is: a
// match string, an ABORT, a TIMEOUT or a failure.

static int cyg_ppp_chat_expect( const char *str )
{
    char *expect;
    char *reply;
    char *s;

    db_printf("%s(%d) %s\n",__PRETTY_FUNCTION__,__LINE__,str);    

    // Look for command strings
    if( strcmp( str, "ABORT" ) == 0 )
        return CHAT_ABORT;

    if( strcmp( str, "TIMEOUT" ) == 0 )
        return CHAT_TIMEOUT;

    // Copy the string over to a writable buffer, so we can handle the
    // sub-expect strings
    strncpy( cyg_ppp_chat_expect_buffer, str, sizeof(cyg_ppp_chat_expect_buffer) );
    s = cyg_ppp_chat_expect_buffer;
    
    while(1)
    {
        // Split the string at the next '-'
        expect = find_sep( &s );

        // All done, it looks like we succeeded!
        if( expect == NULL )
            return CHAT_MATCH;

        db_printf("%s(%d) expect %s\n",__PRETTY_FUNCTION__,__LINE__,expect);

        // Get and response string in case the match fails.
        reply = find_sep( &s );

        db_printf("%s(%d) reply %s\n",__PRETTY_FUNCTION__,__LINE__,reply);            

        // Try a match.
        if( cyg_ppp_chat_match( expect ) )
            return CHAT_MATCH;

        // The match failed. If there is no reply string, then the
        // whole thing has failed.
        if( reply == NULL )
            break;

        // Send the reply string an loop around to pick up the next
        // expect string.
        cyg_ppp_chat_send( reply );
    }

    return CHAT_FAIL;
}

//=====================================================================
// Chat entry point
//
// This is the main CHAT entry point. It is given the name of a device
// plus a pointer to the expect script in argv format. The device
// should already have been set up to the correct baud rate, bits,
// parity, flow control etc.

externC cyg_int32 cyg_ppp_chat( const char *devname,
                                const char *script[] )
{
    const char *s;
    Cyg_ErrNo err;
    cyg_int32 result = 1;
    cyg_uint32 zero = 0;
    cyg_uint32 len = sizeof(zero);    

    cyg_ppp_chat_thread = cyg_thread_self();
    cyg_ppp_chat_abort_count = 0;
    cyg_ppp_chat_timeout = 45;
    
    // Clear the result area
    memset(cyg_ppp_chat_buffer, 0, CHAT_STRING_LENGTH);
       
    while ((err = cyg_io_lookup(devname, &cyg_ppp_chat_handle)) < 0) {
        if (err != 0)
            syslog(LOG_ERR, "Failed to open %s: %d", devname,err);
    }

    // Flush the serial input before starting
    cyg_io_get_config( cyg_ppp_chat_handle,
                       CYG_IO_GET_CONFIG_SERIAL_INPUT_FLUSH,
                       &zero, &len);

    // Set up timeout alarm.
    cyg_alarm_create( cyg_real_time_clock(),
                      chat_alarm,
                      (cyg_addrword_t)0,
                      &cyg_ppp_chat_alarm,
                      &cyg_ppp_chat_alarm_obj);


    // Now loop over script handling the elements in turn
    while( (s = *script++) != NULL )
    {
        int what = cyg_ppp_chat_expect( s );

        db_printf("%s(%d) what %d\n",__PRETTY_FUNCTION__,__LINE__,what);    

        if( what == CHAT_FAIL )
        {
            result = 0;
            break;
        }
        
        if( (s = *script++) != NULL )
        {
            if( what == CHAT_MATCH )
                cyg_ppp_chat_send( s );
            else switch( what )
            {
            case CHAT_ABORT:
                // An abort string, add it to the table.
                if( cyg_ppp_chat_abort_count >= CHAT_MAX_ABORTS )
                {
                    syslog(LOG_ERR,"Too many ABORT strings\n");
                }
                else if( strlen( s ) >= CHAT_ABORT_SIZE )
                    syslog(LOG_ERR,"Abort string too long\n");
                else
                {
                    strncpy( cyg_ppp_chat_abort[cyg_ppp_chat_abort_count++],
                             s, CHAT_ABORT_SIZE );
                }
                break;
                
            case CHAT_TIMEOUT:
                // A new timeout value
                cyg_ppp_chat_timeout = atoi( s );
                db_printf("New timeout >%s< %d\n",s,cyg_ppp_chat_timeout);
                break;
            }
        }
    }

    if (s==NULL)
    {
	// the script ran to completion
        result = 1; 
    }
    
    // Finally, wait for the serial device to drain 
        cyg_io_get_config( cyg_ppp_chat_handle,
                           CYG_IO_GET_CONFIG_SERIAL_OUTPUT_DRAIN,
                           &zero, &len);
        
    return result;
}

//=====================================================================
// End of chat.c

