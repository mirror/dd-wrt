//==========================================================================
//
//      tests/windows_telnet.c
//
//      Test program showing how to create a client that dials into
//      a Windows PPP server and waits for a telnet session.
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Portions created by Nick Garnett are
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
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD, 
// FreeBSD or other sources, and are covered by the appropriate
// copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    oyvind
// Contributors: oyvind harboe
// Date:         2004-06-24
// Purpose:      
// Description:  Example/test on how to connect to a Windows PPP server
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================


#include <pkgconf/system.h>
#include <pkgconf/net.h>
#include <network.h>
#include <cyg/ppp/ppp.h>

static char *windows_script[] =
{
     "TIMEOUT",         "2",
     "",                "CLIENTCLIENT\\c",
     "CLIENTSERVER",    "\\c",
     0
};

void telnet(void)
{
     struct   sockaddr_in sin;
     struct   sockaddr_in pin;
     
     /* get an internet domain socket */
     int sd;
     if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
     {
          return;
     }
     
     /* complete the socket structure */
     memset(&sin, 0, sizeof(sin));
     sin.sin_len = sizeof(sin);
     sin.sin_family = AF_INET;
     sin.sin_addr.s_addr = INADDR_ANY;
     sin.sin_port = htons(23);
          
     unsigned int opt = 1;
     if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) 
     {
          goto out;
     }
     
     /* bind the socket to the port number */
     if (bind(sd, (struct sockaddr *) &sin, sizeof(sin)) == -1) 
     {
          goto out;
     }
     
     /* show that we are willing to listen */
     if (listen(sd, SOMAXCONN) == -1) 
     {
          goto out;
     }
     
     int sd_current;
     /* wait for a client to talk to us */
     socklen_t addrlen = sizeof(pin);
     if ((sd_current = accept(sd, (struct sockaddr *)  &pin, &addrlen)) == -1) 
     {
          goto out;
     }
     
     for (;;)
     {
          char *prompt="eCos>";
          int promptlen = strlen(prompt);
           
          if (write(sd_current, prompt, promptlen) != promptlen) 
          {
               goto AbortSession;
          }
          /* get a message from the client */
          char dir[256];
          int len;
          size_t i;
          for (i=0; i<sizeof(dir)-1; i++)
          {
               // returns when a full line has been collected
               len=read(sd_current, dir+i, 1);
               if (len != 1) 
               {
                    goto AbortSession;
               }
               if (write(sd_current, dir+i, 1)!=1)
               {
                    goto AbortSession;
               }
               
               // ignore CR
               if (dir[i]=='\r')
               {
                    i--;
               }
               
               // Break out on a new line
               if (dir[i]=='\n')
               {
                    break;
               }
               dir[i]=0;
          }
     }
 AbortSession:
     /* close up both sockets */
     close(sd_current); 
 out:
     close(sd);
}
        


int main(int argc, char **argv)
{
     // Bring up the TCP/IP network
     init_all_network_interfaces();
     
     for (;;)
     {
          cyg_ppp_options_t options;
          cyg_ppp_handle_t ppp_handle;
          
          // Initialize the options
          cyg_ppp_options_init( &options );
          
          options.script=windows_script;
          options.baud = CYGNUM_SERIAL_BAUD_38400;
          options.flowctl = CYG_PPP_FLOWCTL_NONE;
          options.idle_time_limit = 0; // never shut down.      
          
          // Start up PPP
          ppp_handle = cyg_ppp_up( "/dev/ser0", &options );
          
          // Wait for it to get running
          if( cyg_ppp_wait_up( ppp_handle ) == 0 )
          {
               // Make use of PPP
               for (;;)
               {
                    telnet();
               }
               
               // never reached, but  for illustration:
               
               // Bring PPP link down
               cyg_ppp_down( ppp_handle );
               
               // Wait for connection to go down.
               cyg_ppp_wait_down( ppp_handle );
          }
     }
}
