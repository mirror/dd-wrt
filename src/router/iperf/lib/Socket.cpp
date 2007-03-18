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
 * by    Ajay Tirumala <tirumala@ncsa.uiuc.edu>
 * and   Mark Gates <mgates@nlanr.net>
 * -------------------------------------------------------------------
 * A parent class to hold socket information. Has wrappers around
 * the common listen, accept, connect, and close functions.
 * ------------------------------------------------------------------- */

#define HEADERS()

#include "headers.h"
#include "Socket.hpp"
#include "util.h"
#include "Timestamp.hpp"
#include "SocketAddr.hpp"

/* -------------------------------------------------------------------
 * Store the server port and UDP/TCP mode.
 * ------------------------------------------------------------------- */

Socket::Socket( unsigned short inPort, bool inUDP ) {
    mPort = inPort;
    mUDP  = inUDP;
    mSock = INVALID_SOCKET;
} // end Socket

/* -------------------------------------------------------------------
 * Close the socket, if it isn't already.
 * ------------------------------------------------------------------- */

Socket::~Socket() {
    Close();
} // end ~Socket


/* -------------------------------------------------------------------
 * Close the socket, if it isn't already.
 * ------------------------------------------------------------------- */

void Socket::Close( void ) {
    if ( mSock != INVALID_SOCKET ) {
        int rc = close( mSock );
        FAIL_errno( rc == SOCKET_ERROR, "close" );
        mSock = INVALID_SOCKET;
    }
} // end Close

/* -------------------------------------------------------------------
 * Setup a socket listening on a port.
 * For TCP, this calls bind() and listen().
 * For UDP, this just calls bind().
 * If inLocalhost is not null, bind to that address rather than the
 * wildcard server address, specifying what incoming interface to
 * accept connections on.
 * ------------------------------------------------------------------- */

void Socket::Listen( const char *inLocalhost, bool isIPv6 ) {
    int rc;

    SocketAddr serverAddr( inLocalhost, mPort, isIPv6 );

    // create an internet TCP socket
    int type = (mUDP  ?  SOCK_DGRAM  :  SOCK_STREAM);
    int domain = (serverAddr.isIPv6() ? 
#ifdef IPV6
                  AF_INET6
#else
                  AF_INET
#endif
                  : AF_INET);

#ifdef DBG_MJZ

    // DBG MJZ
    fprintf(stderr, "inLocalhost=%s domain=%d\n", inLocalhost, domain);
#endif /* DBG_MJZ */

#ifdef WIN32
    if ( mUDP  &&  serverAddr.isMulticast() ) {
        mSock = WSASocket( domain, type, 0, 0, 0, WSA_FLAG_MULTIPOINT_C_LEAF | WSA_FLAG_MULTIPOINT_D_LEAF );
        FAIL_errno( mSock == INVALID_SOCKET, "socket" );

    } else
#endif
    {
        mSock = socket( domain, type, 0 );
        FAIL_errno( mSock == INVALID_SOCKET, "socket" );
    } 

    SetSocketOptions();

    // reuse the address, so we can run if a former server was killed off
    int boolean = 1;
    Socklen_t len = sizeof(boolean);
    // this (char*) cast is for old headers that don't use (void*)
    setsockopt( mSock, SOL_SOCKET, SO_REUSEADDR, (char*) &boolean, len );

#ifdef DBG_MJZ
    {
        // DBG MJZ
        struct sockaddr *sa = serverAddr.get_sockaddr();
        int len = serverAddr.get_sizeof_sockaddr();
        fprintf(stderr, "len: %d salen: %d fam: %d addr chars %2x %2x %2x %2x ...\n",
                len, sa->sa_len, sa->sa_family, sa->sa_data[0], sa->sa_data[1],
                sa->sa_data[2], sa->sa_data[3]);

    }
#endif /* DBG_MJZ */
    // bind socket to server address
#ifdef WIN32
    if ( serverAddr.isMulticast() ) {
        rc = WSAJoinLeaf( mSock, serverAddr.get_sockaddr(), serverAddr.get_sizeof_sockaddr(),0,0,0,0,JL_BOTH);
        FAIL_errno( rc == SOCKET_ERROR, "bind" );
    } else
#endif
    {
        rc = bind( mSock, serverAddr.get_sockaddr(), serverAddr.get_sizeof_sockaddr());
        FAIL_errno( rc == SOCKET_ERROR, "bind" );
    }
    // listen for connections (TCP only).
    // default backlog traditionally 5
    if ( ! mUDP ) {
        rc = listen( mSock, 5 );
        FAIL_errno( rc == SOCKET_ERROR, "listen" );
    }

#ifndef WIN32
    // if multicast, join the group
    if ( mUDP  &&  serverAddr.isMulticast() ) {
        McastJoin( serverAddr );
    }
#endif
} // end Listen

/* -------------------------------------------------------------------
 * After Listen() has setup mSock, this will block
 * until a new connection arrives. Handles interupted accepts.
 * Returns the newly connected socket.
 * ------------------------------------------------------------------- */

int Socket::Accept( void ) {
    iperf_sockaddr clientAddr; 
    Socklen_t addrLen;
    int connectedSock;

    while ( true ) {
        // accept a connection
        addrLen = sizeof( clientAddr );
        connectedSock = accept( mSock, (struct sockaddr*) &clientAddr, &addrLen );

        // handle accept being interupted
        if ( connectedSock == INVALID_SOCKET  &&  errno == EINTR ) {
            continue;
        }

        return connectedSock;
    }

} // end Accept

/* -------------------------------------------------------------------
 * Setup a socket connected to a server.
 * If inLocalhost is not null, bind to that address, specifying
 * which outgoing interface to use.
 * ------------------------------------------------------------------- */

void Socket::Connect( const char *inHostname, const char *inLocalhost ) {
    int rc;
    SocketAddr serverAddr( inHostname, mPort );

    assert( inHostname != NULL );

    // create an internet socket
    int type = (mUDP  ?  SOCK_DGRAM : SOCK_STREAM);

    int domain = (serverAddr.isIPv6() ? 
#ifdef IPV6
                  AF_INET6
#else
                  AF_INET
#endif
                  : AF_INET);

    // DBG MJZ
#ifdef DBG_MJZ
    fprintf(stderr, "inHostname=%s domain=%d\n", inHostname, domain);
#endif /* DBG_MJZ */

    mSock = socket( domain, type, 0 );
    FAIL_errno( mSock == INVALID_SOCKET, "socket" );

    SetSocketOptions();


    if ( inLocalhost != NULL ) {
        SocketAddr localAddr( inLocalhost );

#ifdef DBG_MJZ

        // DBG MJZ
        fprintf(stderr, "inLocalhost=%s sockaddrlen=%d\n", inHostname, localAddr.get_sizeof_sockaddr());

        {
            // DBG MJZ
            struct sockaddr *sa = localAddr.get_sockaddr();
            int len = localAddr.get_sizeof_sockaddr();
            fprintf(stderr, "LOC len: %d salen: %d fam: %d data chars %2x %2x %2x %2x ...\n",
                    len, sa->sa_len, sa->sa_family, sa->sa_data[0], sa->sa_data[1],
                    sa->sa_data[2], sa->sa_data[3]);

        }
#endif /* DBG_MJZ */
        // bind socket to local address
        rc = bind( mSock, localAddr.get_sockaddr(), localAddr.get_sizeof_sockaddr());
        FAIL_errno( rc == SOCKET_ERROR, "bind" );
    }

#ifdef DBG_MJZ
    {
        // DBG MJZ
        struct sockaddr *sa = serverAddr.get_sockaddr();
        int len = serverAddr.get_sizeof_sockaddr();
        fprintf(stderr, "SRV len: %d salen: %d fam: %d data chars %2x %2x %2x %2x ...\n",
                len, sa->sa_len, sa->sa_family, sa->sa_data[0], sa->sa_data[1],
                sa->sa_data[2], sa->sa_data[3]);

    }
#endif /* DBG_MJZ */
    // connect socket
    rc = connect( mSock, serverAddr.get_sockaddr(), serverAddr.get_sizeof_sockaddr());
    FAIL_errno( rc == SOCKET_ERROR, "connect" );

} // end Connect

/* -------------------------------------------------------------------
 * Joins the multicast group, with the default interface.
 * ------------------------------------------------------------------- */

void Socket::McastJoin( SocketAddr &inAddr ) {
#ifdef MCAST

    if ( !inAddr.isIPv6() ) {
        struct ip_mreq mreq;

        memcpy( &mreq.imr_multiaddr, inAddr.get_in_addr(), sizeof(mreq.imr_multiaddr));

        mreq.imr_interface.s_addr = htonl( INADDR_ANY );

        int rc = setsockopt( mSock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                             (char*) &mreq, sizeof(mreq));
        FAIL_errno( rc == SOCKET_ERROR, "multicast join" );
    }
#ifdef IPV6_ADD_MEMBERSHIP
      else {
        struct ipv6_mreq mreq;

        memcpy( &mreq.ipv6mr_multiaddr, inAddr.get_in6_addr(), sizeof(mreq.ipv6mr_multiaddr));

        mreq.ipv6mr_interface = 0;

        int rc = setsockopt( mSock, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP,
                             (char*) &mreq, sizeof(mreq));
        FAIL_errno( rc == SOCKET_ERROR, "multicast join" );
    }
#endif
#endif
}
// end McastJoin

/* -------------------------------------------------------------------
 * Sets the Multicast TTL for outgoing packets.
 * ------------------------------------------------------------------- */

void Socket::McastSetTTL( int val, SocketAddr &inAddr ) {
#ifdef MCAST
    if ( !inAddr.isIPv6() ) {
        int rc = setsockopt( mSock, IPPROTO_IP, IP_MULTICAST_TTL,
                             (char*) &val, sizeof(val));
        WARN_errno( rc == SOCKET_ERROR, "multicast ttl" );
    }
#ifdef IPV6_MULTICAST_HOPS
      else {
        int rc = setsockopt( mSock, IPPROTO_IPV6, IPV6_MULTICAST_HOPS,
                             (char*) &val, sizeof(val));
        WARN_errno( rc == SOCKET_ERROR, "multicast ttl" );
    }
#endif
#endif
}
// end McastSetTTL


// get local address
SocketAddr Socket::getLocalAddress( void ) {
    iperf_sockaddr sock; 
    Socklen_t len = sizeof(sock);
    int rc = getsockname( mSock, (struct sockaddr*) &sock, &len );
    FAIL_errno( rc == SOCKET_ERROR, "getsockname" );

    return SocketAddr( (struct sockaddr*) &sock, len );
}

// get remote address
SocketAddr Socket::getRemoteAddress( void ) {
    iperf_sockaddr peer; 

    Socklen_t len = sizeof(peer);
    int rc = getpeername( mSock, (struct sockaddr*) &peer, &len );
    FAIL_errno( rc == SOCKET_ERROR, "getpeername" );

    return SocketAddr( (struct sockaddr*) &peer, len );
}
