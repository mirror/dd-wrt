#ifndef CYGONCE_NET_FTPCLIENT_FTPCLIENT_H
#define CYGONCE_NET_FTPCLIENT_FTPCLIENT_H

//==========================================================================
//
//      ftpclient.h
//
//      A simple FTP client
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2004 Gary Thomas
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    andrew.lunn@ascom.ch
// Contributors: andrew.lunn@ascom.ch
// Date:         2001-11-4
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

// User-defined function for printing diagnostic messages
typedef void (*ftp_printf_t)(unsigned error, const char *, ...);
// User-defined function used to provide data
typedef int (*ftp_read_t)(char *buf, int bufsize, void *priv);
// User-defined function used to process data
typedef int (*ftp_write_t)(char *buf, int bufsize, void *priv);

/* Use the FTP protocol to retrieve a file from a server. Only binary
   mode is supported. The filename can include a directory name. Only
   use unix style / not M$'s \. The file is placed into buf. buf has
   maximum size buf_size. If the file is bigger than this, the
   transfer fails and FTP_TOOBIG is returned. Other error codes as
   listed below can also be returned. If the transfer is succseful the
   number of bytes received is returned. */

int ftp_get(char * hostname, 
            char * username, 
            char * passwd, 
            char * filename, 
            char * buf, 
            unsigned buf_size,
            ftp_printf_t ftp_printf);

//
// Just like 'ftp_get()' except that the "write()" function is called
// as data arrives instead of using a fixed buffer.  Returns the total
// amount of data read, or an error indication (negative codes)
//
int ftp_get_var(char *hostname,
                char *username,
                char *passwd,
                char *filename,
                ftp_write_t ftp_write,
                void *ftp_write_priv,
                ftp_printf_t ftp_printf);

/*Use the FTP protocol to send a file from a server. Only binary mode
  is supported. The filename can include a directory name. Only use
  unix style / not M$'s \. The contents of buf is placed into the file
  on the server. If an error occurs one of the codes as listed below
  will be returned. If the transfer is succseful zero is returned.*/

int ftp_put(char * hostname, 
            char * username, 
            char * passwd, 
            char * filename, 
            char * buf, 
            unsigned buf_size,
            ftp_printf_t ftp_printf);

//
// Just like 'ftp_put()' except that the "read()" function is called
// to fetch the data to write instead of using a fixed buffer.  Returns 
// the total amount of data written, or an error indication (negative codes)
//
int ftp_put_var(char *hostname,
                char *username,
                char *passwd,
                char *filename,
                ftp_read_t ftp_read,
                void *ftp_read_priv,
                ftp_printf_t ftp_printf);

/*ftp_get() and ftp_put take the name of a function to call to print
  out diagnostic and error messages. This is a sample implementation
  which can be used if you don't want to implement the function
  yourself. error will be true when the message to print is an error
  message. Otherwise the message is diagnostic, eg the commands sent
  and received from the server.*/

void ftpclient_printf(unsigned error, const char *fmt, ...);

/* Error codes */

#define FTP_BAD         -2 /* Catch all, socket errors etc. */
#define FTP_NOSUCHHOST  -3 /* The server does not exist. */
#define FTP_BADUSER     -4 /* Username/Password failed */
#define FTP_TOOBIG      -5 /* Out of buffer space or disk space */ 
#define FTP_BADFILENAME -6 /* The file does not exist */
#define FTP_NOMEMORY    -7 /* Unable to allocate memory for internal buffers */

#endif // CYGONCE_NET_FTPCLIENT_FTPCLIENT

