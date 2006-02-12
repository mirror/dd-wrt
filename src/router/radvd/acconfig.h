/*
 *   $Id: acconfig.h,v 1.3 2001/11/14 19:58:10 lutchann Exp $
 *
 *   Authors:
 *    Lars Fenneberg		<lf@elemental.net>	 
 *
 *   This software is Copyright 1996,1997 by the above mentioned author(s), 
 *   All Rights Reserved.
 *
 *   The license which is distributed with this software in the file COPYRIGHT
 *   applies to this software. If your distribution is missing this file, you
 *   may request it from <lutchann@litech.org>.
 *
 */

@TOP@

/* Whether struct sockaddr_in6 has sin6_scope_id */
#undef HAVE_SIN6_SCOPE_ID

/* Whether struct in6_addr has u6_addrXX and s6_addrXX is defined */
#undef HAVE_IN6_ADDR_S6_ADDR
