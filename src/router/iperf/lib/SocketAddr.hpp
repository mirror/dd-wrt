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
 * Socket.cpp
 * by Ajay Tirumala <tirumala@ncsa.uiuc.edu> 
 *    and Mark Gates <mgates@nlanr.net>
 * -------------------------------------------------------------------
 * A parent class to hold socket information. Has wrappers around
 * the common listen, accept, connect, and close functions.
 * ------------------------------------------------------------------- */


#ifndef SOCKET_ADDR_H
#define SOCKET_ADDR_H

#include "headers.h"

/* ------------------------------------------------------------------- */
class SocketAddr {
public:
    SocketAddr( const char* inHostname = NULL, unsigned short inPort = 0, bool isIPv6 = false );
    SocketAddr( const struct sockaddr* inAddr, Socklen_t inSize );

    ~SocketAddr();

    void setHostname( const char* inHostname );          // DNS lookup
    void getHostname( char* outHostname, size_t len );   // reverse DNS lookup
    void getHostAddress( char* outAddress, size_t len ); // dotted decimal

    void setPort( unsigned short inPort );
    void setPortAny( void );
    unsigned short getPort( void );

    void setAddressAny( void );

    // return pointer to the struct sockaddr
    struct sockaddr* get_sockaddr( void );

    // return pointer to the struct sockaddr_in (IPv4)
    struct sockaddr_in* get_sockaddr_in( void );

    // return pointer to the struct in_addr
    struct in_addr* get_in_addr( void );
#ifdef IPV6
    // return pointer to the struct in_addr
    struct in6_addr* get_in6_addr( void );
#endif
    // return the sizeof the addess structure (struct sockaddr_in)
    Socklen_t get_sizeof_sockaddr( void );

    bool isMulticast( void );

    bool isIPv6(void) {
        return mIsIPv6; 
    };

    static bool are_Equal(sockaddr *first, sockaddr *second);
    static bool Hostare_Equal(sockaddr *first, sockaddr *second);

    static Socklen_t mAddress_size;


protected:
    void zeroAddress( void );
    iperf_sockaddr mAddress;
        
    bool mIsIPv6;
};
// end class SocketAddr

#endif /* SOCKET_ADDR_H */
