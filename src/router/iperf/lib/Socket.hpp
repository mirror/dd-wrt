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
 * Socket.hpp
 * by Mark Gates <mgates@nlanr.net>
 * -------------------------------------------------------------------
 * A parent class to hold socket information. Has wrappers around
 * the common listen, accept, connect, and close functions.
 * ------------------------------------------------------------------- */

#ifndef SOCKET_H
#define SOCKET_H

#include "headers.h"
#include "SocketAddr.hpp"

/* ------------------------------------------------------------------- */
class Socket {
public:
    // stores server port and TCP/UDP mode
    Socket( unsigned short inPort, bool inUDP = false );

    // destructor
    virtual ~Socket();

protected:
    // get local address
    SocketAddr getLocalAddress( void );

    // get remote address
    SocketAddr getRemoteAddress( void );

    // server bind and listen
    void Listen( const char *inLocalhost = NULL, bool isIPv6 = false );

    // server accept
    int Accept( void );

    // client connect
    void Connect( const char *inHostname, const char *inLocalhost = NULL );

    // close the socket
    void Close( void );

    // to put setsockopt calls before the listen() and connect() calls
    virtual void SetSocketOptions( void ) {
    }

    // join the multicast group
    void McastJoin( SocketAddr &inAddr );

    // set the multicast ttl
    void McastSetTTL( int val, SocketAddr &inAddr );

    int   mSock;             // socket file descriptor (sockfd)
    unsigned short mPort;    // port to listen to
    bool  mUDP;              // true for UDP, false for TCP

}; // end class Socket

#endif // SOCKET_H
