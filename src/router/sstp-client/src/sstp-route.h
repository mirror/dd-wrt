/*!
 * @brief This is the sstp-client code
 *
 * @file sstp-route.h
 *
 * @author Copyright (C) 2011 Eivind Naess, 
 *      All Rights Reserved
 *
 * @par License:
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __SSTP_ROUTE_H__
#define __SSTP_ROUTE_H__

#include <netinet/in.h>

struct sstp_route_ctx;
typedef struct sstp_route_ctx sstp_route_ctx_st;


#if defined(HAVE_NETLINK) || defined(__APPLE__)

/*! 
 * @brief Holds either a ipv4 or an ipv6 address
 */
typedef union 
{
    /* Access the IPv4 address */
    struct in_addr in4;

    /* Access the IPv6 address */
    struct in6_addr in6;

} inet_addr_t;

/*!
 * @brief Data structure to hold the route information
 */
typedef struct 
{
    /* Specify which attributes are set in this structure */
    struct {
        int src : 1;
        int dst : 1;
        int gwy : 1;
        int oif : 1;
    } have;

    /* The family, AF_UNSPEC, AF_INET, AF_INET6 */
    int family;

    /* The byte length of the addresse in this structure */
    int rt_blen;

    /* A IPv4 or IPv6 source address */
    inet_addr_t src;

    /* A IPv4 or IPv6 dst address */
    inet_addr_t dst;

    /* A IPv4 or IPv6 gateway address */
    inet_addr_t gwy;

    /* The interface index */
    int oif;

    /* The interface name */
    char ifname[32];

} sstp_route_st;

#else   /* #ifdef HAVE_NETLINK */

typedef struct
{
    char ipcmd[512];

} sstp_route_st;

#endif  /* #ifdef HAVE_NETLINK */


/*!
 * @brief Create or replace an existing route to a destination
 */
int sstp_route_replace(sstp_route_ctx_st *ctx, sstp_route_st *route);


/*!
 * @brief Delete a route from the route table
 */
int sstp_route_delete(sstp_route_ctx_st *ctx, sstp_route_st *route);


/*! 
 * @brief Obtain a particular route to a destination
 */
int sstp_route_get(sstp_route_ctx_st *ctx, struct sockaddr *dst,
        sstp_route_st *route);


/*!
 * @brief Initialize the route module
 */
int sstp_route_init(sstp_route_ctx_st **ctx);


/*!
 * @brief Cleanup the route context
 */
void sstp_route_done(sstp_route_ctx_st *ctx);


#endif /* #ifndef __SSTP_ROUTE_H__ */

