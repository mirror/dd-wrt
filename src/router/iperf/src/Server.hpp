/*--------------------------------------------------------------- 
 * Copyright (c) 1999,2000,2001,2002,2003                              
 * The Board of Trustees of the University of Illinois            
 * All Rights Reserved.                                           
 *--------------------------------------------------------------- 
 * Permission is hereby granted, free of charge, to any person    
 * obtaining a copy of this software (Iperf) and associated       
 * documentation files (the "Software"), to deal in the Software  
 * without restriction, including without limitation the          
 * rights to use, copy, modify, merge, publish, distribute,        
 * sublicense, and/or sell copies of the Software, and to permit     
 * persons to whom the Software is furnished to do
 * so, subject to the following conditions: 
 *
 *     
 * Redistributions of source code must retain the above 
 * copyright notice, this list of conditions and 
 * the following disclaimers. 
 *
 *     
 * Redistributions in binary form must reproduce the above 
 * copyright notice, this list of conditions and the following 
 * disclaimers in the documentation and/or other materials 
 * provided with the distribution. 
 * 
 *     
 * Neither the names of the University of Illinois, NCSA, 
 * nor the names of its contributors may be used to endorse 
 * or promote products derived from this Software without
 * specific prior written permission. 
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE CONTIBUTORS OR COPYRIGHT 
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 * ________________________________________________________________
 * National Laboratory for Applied Network Research 
 * National Center for Supercomputing Applications 
 * University of Illinois at Urbana-Champaign 
 * http://www.ncsa.uiuc.edu
 * ________________________________________________________________ 
 *
 * Server.hpp
 * by Mark Gates <mgates@nlanr.net>
 * -------------------------------------------------------------------
 * A server thread is initiated for each connection accept() returns.
 * Handles sending and receiving data, and then closes socket.
 * ------------------------------------------------------------------- */

#ifndef SERVER_H
#define SERVER_H


#include "Settings.hpp"
#include "PerfSocket.hpp"
#include "Thread.hpp"
#include "Notify.hpp"

/* ------------------------------------------------------------------- */
class Server : public PerfSocket, public Thread {
public:
    // stores server socket, port and TCP/UDP mode
    Server( ext_Settings *inSettings, int inSock,
            Notify* toNotify = NULL );

    // destroy the server object
    ~Server();

    virtual int set_tcp_windowsize( int a, int b ) {
        return setsock_tcp_windowsize( a, b, 0 );
    };
    virtual int get_tcp_windowsize( int a ) {
        return getsock_tcp_windowsize( a, 0 );
    };

    // accepts connection and receives data
    virtual void Run( void );

}; // end class Server

#endif // SERVER_H
