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
 * by       Ajay Tirumala <tirumala@ncsa.uiuc.edu>
 * and      Mark Gates <mgates@nlanr.net>
 * -------------------------------------------------------------------
 * A parent class to hold socket information. Has wrappers around
 * the common listen, accept, connect, and close functions.
 * ------------------------------------------------------------------- */

#define HEADERS()

#include "headers.h"

#include "SocketAddr.hpp"

/* Occasionally the /usr/include/netdb.h doesn't define this
 * (e.g. HP-UX without _XOPEN_SOURCE_EXTENDED).
 * Should be okay to declare it multiple times though
 * except on Windows which #defines this is to a function (gack!). */


#ifndef WIN32
extern int h_errno;
#endif

Socklen_t SocketAddr::mAddress_size = sizeof(iperf_sockaddr);
/* -------------------------------------------------------------------
 * Create a socket address. If inHostname is not null, resolve that
 * address and fill it in. Fill in the port number. Use IPv6 ADDR_ANY
 * if that is what is desired.
 * ------------------------------------------------------------------- */

SocketAddr::SocketAddr( const char* inHostname, unsigned short inPort, bool isIPv6 ) {
    zeroAddress();
    if ( inHostname != NULL ) {
        setHostname( inHostname );
    } else {
#ifdef IPV6
        if ( isIPv6 ) {
            ((sockaddr*)&mAddress)->sa_family = AF_INET6;
        } else {
            ((sockaddr*)&mAddress)->sa_family = AF_INET;
        }
        mIsIPv6 = isIPv6;
#else
        mIsIPv6 = false;
        ((sockaddr*)&mAddress)->sa_family = AF_INET;
#endif
    }
    setPort( inPort );
}
// end SocketAddr

/* -------------------------------------------------------------------
 * Copy the given socket address.
 * ------------------------------------------------------------------- */

SocketAddr::SocketAddr( const struct sockaddr* inAddr, Socklen_t inSize ) {
    zeroAddress();
    if ( inSize > mAddress_size ) {
        inSize = mAddress_size;
    }

    if ( inAddr->sa_family == AF_INET ) {
        mIsIPv6 = false;
    } else {
        mIsIPv6 = true;
    }

    memcpy( &mAddress, inAddr, inSize);
}
// end SocketAddr

/* -------------------------------------------------------------------
 * Empty destructor.
 * ------------------------------------------------------------------- */

SocketAddr::~SocketAddr( ) {
}
// end ~SocketAddr

/* -------------------------------------------------------------------
 * Resolve the hostname address and fill it in.
 * ------------------------------------------------------------------- */

void SocketAddr::setHostname( const char* inHostname ) {

    assert( inHostname != NULL );

    // trouble with below on FreeBSD 4.3-RELEASE (for one thing
    // inet_aton is ipv4 only... and thengethostbyname doesn't
    // appear to handle ipv4 addresses.
    // ..I think this works for both ipv6 & ipv4... we'll see
#if defined(IPV6)
    {
        struct addrinfo *res;
        int ret_ga;
#ifdef DBG_MJZ
        fprintf(stderr, "about to getaddrinfo on '%s'\n", inHostname);
#endif      
        ret_ga = getaddrinfo(inHostname, NULL, NULL, &res);
        if ( ret_ga ) {
            fprintf(stderr, "error: %s\n", gai_strerror(ret_ga));
            exit(1);
        }
        if ( !res->ai_addr ) {
            fprintf(stderr, "getaddrinfo failed to get an address... target was '%s'\n", inHostname);
            exit(1);
        }

#ifdef DBG_MJZ
        fprintf(stderr, "done with gai, ai_fam=%d ai_alen=%d addr=0x%08x...\n", res->ai_family, res->ai_addrlen, *((int *)(res->ai_addr)));
#endif

        // Check address type before filling in the address
        // ai_family = PF_xxx; ai_protocol = IPPROTO_xxx, see netdb.h
        // ...but AF_INET6 == PF_INET6
        if ( res->ai_family == AF_INET ) {
            mIsIPv6 = false;
        } else {
            mIsIPv6 = true;
        }

        memcpy(&mAddress, (res->ai_addr),
               (res->ai_addrlen));

        freeaddrinfo(res);

        return;
    }
#else
    mIsIPv6 = false;
    mAddress.sin_family = AF_INET;
    // first try just converting dotted decimal
    // on Windows gethostbyname doesn't understand dotted decimal
    int rc = inet_pton( AF_INET, inHostname, (unsigned char*)&(mAddress.sin_addr) );
    if ( rc == 0 ) {
        struct hostent *hostP = gethostbyname( inHostname );
        if ( hostP == NULL ) {
            /* this is the same as herror() but works on more systems */
            const char* format;
            switch ( h_errno ) {
                case HOST_NOT_FOUND:
                    format = "%s: Unknown host\n";
                    break;
                case NO_ADDRESS:
                    format = "%s: No address associated with name\n";
                    break;
                case NO_RECOVERY:
                    format = "%s: Unknown server error\n";
                    break;
                case TRY_AGAIN:
                    format = "%s: Host name lookup failure\n";
                    break;

                default:
                    format = "%s: Unknown resolver error\n";
                    break;
            }
            fprintf( stderr, format, inHostname );
            exit(1);

            return; // TODO throw
        }

        memcpy(&(mAddress.sin_addr), *(hostP->h_addr_list),
               (hostP->h_length));
    }
#endif
}
// end setHostname

/* -------------------------------------------------------------------
 * Copy the hostname into the given string. This generally does a
 * reverse DNS lookup on the IP address; if that fails fill in the
 * IP address itself.
 * ------------------------------------------------------------------- */

void SocketAddr::getHostname( char* outHostname, size_t len ) {
    struct hostent *hostP = NULL;

    if ( !mIsIPv6 ) {
        hostP = gethostbyaddr( (char*) &(((sockaddr_in*) &mAddress)->sin_addr),
                               sizeof( in_addr ),
                               ((sockaddr*) &mAddress)->sa_family );
    }
#if defined(IPV6)
    else {

        hostP = gethostbyaddr( (char*) &(((sockaddr_in6*) &mAddress)->sin6_addr), 
                               sizeof( in6_addr ),
                               ((sockaddr*) &mAddress)->sa_family ); 
    }
#endif

    if ( hostP == NULL ) {
        // in the worst case, just use the dotted decimal IP address
        getHostAddress( outHostname, len );
    } else {
        strncpy( outHostname, hostP->h_name, len );
    }
}
// end getHostname

/* -------------------------------------------------------------------
 * Copy the IP address in dotted decimal format into the string.
 * TODO make this thread safe, since inet_ntoa's string is shared.
 * ------------------------------------------------------------------- */
void SocketAddr::getHostAddress( char* outAddress, size_t len ) {
    if ( !mIsIPv6 ) {
        inet_ntop( AF_INET, &(((sockaddr_in*) &mAddress)->sin_addr), 
                   outAddress, len);
    }
#ifdef IPV6
    else {
        inet_ntop( AF_INET6, &(((sockaddr_in6*) &mAddress)->sin6_addr), 
                   outAddress, len);
    }
#endif
}
// end getHostAddress

/* -------------------------------------------------------------------
 * Set the address to any (generally all zeros).
 * ------------------------------------------------------------------- */

void SocketAddr::setAddressAny( void ) {
    if ( !mIsIPv6 )
        memset( &(((sockaddr_in*) &mAddress)->sin_addr), 0, 
                sizeof( in_addr ));
#if defined(IPV6)  
    else
        memset( &(((sockaddr_in6*) &mAddress)->sin6_addr), 0, 
                sizeof( in6_addr ));
#endif
}
// end setAddressAny

/* -------------------------------------------------------------------
 * Set the port to the given port. Handles the byte swapping.
 * ------------------------------------------------------------------- */

void SocketAddr::setPort( unsigned short inPort ) {
    if ( !mIsIPv6 )
        ((sockaddr_in*) &mAddress)->sin_port = htons( inPort );
#if defined(IPV6)  
    else
        ((sockaddr_in6*) &mAddress)->sin6_port = htons( inPort );
#endif

}
// end setPort

/* -------------------------------------------------------------------
 * Set the port to zero, which lets the OS pick the port.
 * ------------------------------------------------------------------- */

void SocketAddr::setPortAny( void ) {
    setPort( 0 );
}
// end setPortAny

/* -------------------------------------------------------------------
 * Return the port. Handles the byte swapping.
 * ------------------------------------------------------------------- */

unsigned short SocketAddr::getPort( void ) {
#if defined(IPV6)
    if ( mIsIPv6 )
        return ntohs( ((sockaddr_in6*) &mAddress)->sin6_port);
#endif

    return ntohs( ((sockaddr_in*) &mAddress)->sin_port );
}
// end getPort

/* -------------------------------------------------------------------
 * Return the IPv4 or IPv6 sockaddr structure
 * ------------------------------------------------------------------- */

struct sockaddr* SocketAddr::get_sockaddr( void ) {
    return(sockaddr*)&mAddress;
}

/* -------------------------------------------------------------------
 * Return the IPv4 sockaddr_in structure
 * ------------------------------------------------------------------- */

struct sockaddr_in* SocketAddr::get_sockaddr_in( void ) {
    if ( !mIsIPv6 )
        return(sockaddr_in*)&mAddress;
    else {
        fprintf(stderr, "FATAL: get_sockaddr_in called with IPv6 address\n");
        return NULL;
    }
}

/* -------------------------------------------------------------------
 * Return the IPv4 Internet Address from the sockaddr_in structure
 * ------------------------------------------------------------------- */

struct in_addr* SocketAddr::get_in_addr( void ) {
    if ( !mIsIPv6 )
        return &(((sockaddr_in*) &mAddress)->sin_addr);

    fprintf(stderr, "FATAL: get_in_addr called on IPv6 address\n");
    return NULL;
}

/* -------------------------------------------------------------------
 * Return the IPv6 Internet Address from the sockaddr_in6 structure
 * ------------------------------------------------------------------- */
#ifdef IPV6
struct in6_addr* SocketAddr::get_in6_addr( void ) {
    if ( mIsIPv6 )
        return &(((sockaddr_in6*) &mAddress)->sin6_addr);

    fprintf(stderr, "FATAL: get_in6_addr called on IPv4 address\n");
    return NULL;
}
#endif


/* -------------------------------------------------------------------
 * Return the size of the appropriate address structure.
 * ------------------------------------------------------------------- */

Socklen_t SocketAddr::get_sizeof_sockaddr( void ) {

#if defined(IPV6)
    if ( mIsIPv6 ) {
        return(sizeof(sockaddr_in6));
    }
#endif
    return(sizeof(sockaddr_in));
}
// end getAddressLen

/* -------------------------------------------------------------------
 * Return true if the address is a IPv4 multicast address.
 * ------------------------------------------------------------------- */

bool SocketAddr::isMulticast( void ) {

#if defined(IPV6)
    if ( mIsIPv6 ) {
        return( 0 != IN6_IS_ADDR_MULTICAST(&(((sockaddr_in6*) &mAddress)->sin6_addr)) );
    } else
#endif
    {
        // 224.0.0.0 to 239.255.255.255 (e0.00.00.00 to ef.ff.ff.ff)
        const unsigned long kMulticast_Mask = 0xe0000000L;

        return(kMulticast_Mask ==
               (ntohl( ((sockaddr_in*) &mAddress)->sin_addr.s_addr) & kMulticast_Mask));
    }
}
// end isMulticast

/* -------------------------------------------------------------------
 * Zero out the address structure.
 * ------------------------------------------------------------------- */

void SocketAddr::zeroAddress( void ) {
    memset( &mAddress, 0, sizeof( mAddress ));
}
// zeroAddress

/* -------------------------------------------------------------------
 * Compare two sockaddrs and return true if they are equal
 * ------------------------------------------------------------------- */
bool SocketAddr::are_Equal( sockaddr* first, sockaddr* second ) {
    if ( first->sa_family == AF_INET && second->sa_family == AF_INET ) {
        // compare IPv4 adresses
        return( ((long) ((sockaddr_in*)first)->sin_addr.s_addr == (long) ((sockaddr_in*)second)->sin_addr.s_addr)
                && ( ((sockaddr_in*)first)->sin_port == ((sockaddr_in*)second)->sin_port) );
    }
#if defined(IPV6)      
    if ( first->sa_family == AF_INET6 && second->sa_family == AF_INET6 ) {
        // compare IPv6 addresses
        return( !memcmp(((sockaddr_in6*)first)->sin6_addr.s6_addr, ((sockaddr_in6*)second)->sin6_addr.s6_addr, sizeof(in6_addr)) 
                && (((sockaddr_in6*)first)->sin6_port == ((sockaddr_in6*)second)->sin6_port) );
    }
#endif 
    return false;

}

/* -------------------------------------------------------------------
 * Compare two sockaddrs and return true if the hosts are equal
 * ------------------------------------------------------------------- */
bool SocketAddr::Hostare_Equal( sockaddr* first, sockaddr* second ) {
    if ( first->sa_family == AF_INET && second->sa_family == AF_INET ) {
        // compare IPv4 adresses
        return( (long) ((sockaddr_in*)first)->sin_addr.s_addr == 
                (long) ((sockaddr_in*)second)->sin_addr.s_addr);
    }
#if defined(IPV6)      
    if ( first->sa_family == AF_INET6 && second->sa_family == AF_INET6 ) {
        // compare IPv6 addresses
        return( !memcmp(((sockaddr_in6*)first)->sin6_addr.s6_addr, 
                        ((sockaddr_in6*)second)->sin6_addr.s6_addr, sizeof(in6_addr)));
    }
#endif 
    return false;

}
