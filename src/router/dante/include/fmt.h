/*
 * Copyright (c) 2012, 2013, 2014
 *      Inferno Nettverk A/S, Norway.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. The above copyright notice, this list of conditions and the following
 *    disclaimer must appear in all copies of the software, derivative works
 *    or modified versions, and any portions thereof, aswell as in all
 *    supporting documentation.
 * 2. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by
 *      Inferno Nettverk A/S, Norway.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Inferno Nettverk A/S requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  sdc@inet.no
 *  Inferno Nettverk A/S
 *  Oslo Research Park
 *  Gaustadalléen 21
 *  NO-0349 Oslo
 *  Norway
 *
 * any improvements or extensions that they make and grant Inferno Nettverk A/S
 * the rights to redistribute these changes.
 *
 */

/* $Id: fmt.h,v 1.25.4.3 2014/08/15 18:16:40 karls Exp $ */

#ifndef _FMT_H_
#define _FMT_H_

   /*
    *
    * Common, client and server.
    *
    */

char *
fmtresponseerror(const ssize_t received, const size_t expected,
                 char *emsg, const size_t emsglen);
/*
 * Formats "emsg", of length "emsglen", appropriately considering
 * we received only received a "received" byte response from proxy server,
 * but expected "expected" bytes.
 *
 * Returns "emsg".
 */

char *
fmtversionerror(const int expected, const int received,
                char *emsg, const size_t emsglen);
/*
 * Formats "emsg", of length "emsglen", appropriately considering
 * we received a version "received" reply from proxy server, but expected
 * version "expected".
 *
 * Returns "emsg".
 */

void
log_writefailed(const interfaceside_t side, const int s,
                const struct sockaddr_storage *dst);
/*
 * Logs that a write to "dst" on socket "s" failed on interfaceside "side".
 * If "dst" is NULL, the destination is retrieved via a getpeername(2)
 * call on "s".
 */

void
log_connectfailed(const interfaceside_t side, const char *dststr);
/*
 * Logs that a connect to "dst" failed on interfaceside "side".
 */

void
log_resolvefailed(const char *hostname, const interfaceside_t side,
                  const int gaierr);
/*
 * Logs that we could not resolve "hostname", related to a target on
 * the interface-side "side".
 *
 * "gaierr" is the resolver error returned.
 */


void
log_reversemapfailed(const struct sockaddr_storage *addr,
                     const interfaceside_t side, const int gaierr);

/*
 * Logs that we could not reversemap the IP-address in "addr", related to a
 * target on the interface-side "side".
 *
 * "gaierr" is the resolver error returned.
 */

#if SOCKS_CLIENT /* just client */


#else /* !SOCKS_CLIENT just server */

void
log_getsockopt_failed(const char *function, const char *option, const int fd,
                      const interfaceside_t interfaceside);
/*
 * Appropriately logs that we in function "function" could not fetch the
 * getsockopt(2)-option "option" on fd "fd", related to the interface on
 * the side "interfaceside".
 */

void
log_setsockopt_failed(const char *function, const char *option,
                      const int value, const int fd,
                      const interfaceside_t interfaceside);
/*
 * Similar to log_getsockopt() failed, but instead logs that setting the
 * option "option" to the value "value" failed.
 */


void
log_interfaceprotocol_set_too_late(const interfaceside_t side);
/*
 * logs protocol/address-famelies for interface set too late.
 */

void
log_addchild_failed(void);
/*
 * failed to create a new child.
 */

void
log_clientdropped(const struct sockaddr_storage *client);
/*
 * Client with address "client" was dropped due to lack of resources.
 */

void
log_clientsend(const struct sockaddr_storage *client,
               const sockd_child_t *child, const int isresend);
/*
 * Logs some information about sending a client object using protocol
 * "protocol" to child "child".
 */

void
log_sendfailed(const struct sockaddr_storage *client, const int s,
               const sockd_child_t *child, const int isfirsttime);
/*
 * Failed sending a clientobject relating to client "client", which
 * connected to us on socket "s", to child "child".
 *
 * "isfirsttime" indicates if it was the first time we failed sending
 * "client" to a child or not, and errno is used to determine if
 * the error is permanent or not.
 */

void
log_noclientrecv(const sockd_child_t *child);
/*
 * Logs some information about not trying to receive a new client object
 * from child "child" because we already have one.
 */

void
log_probablytimedout(const struct sockaddr_storage *client,
                     const sockd_child_t *child);
/*
 * Log that client "client" probably timed out while waiting for a
 * child of type "childtype" to handle it.
 */

void
log_truncatedudp(const char *function, const struct sockaddr_storage *from,
                 const ssize_t len);
/*
 * UDP packet from "from" was truncated.  We received "len" bytes.
 */


void
sockd_readmotherscontrolsocket(const char *prefix, const int s);
/*
 * Normally we will only receive EOF over the control channel to mother.
 * This function checks, and logs EOF as a normal event.  Anything else
 * is logged as something much more serious.
 */

void
log_ruleinfo_shmid(const rule_t *rule, const char *function,
                   const char *context);
/*
 * debug logs shmid-related info about rule "rule".
 * "function" is the function calling us, and "context" is some context
 * to add after the function-name.
 */

void
log_boundexternaladdress(const char *function,
                         const struct sockaddr_storage *addr);
/*
 * debug logs that we bound an address on the external side.
 * Used by many tests.
 */

void
log_unexpected_udprecv_error(const char *function, const int fd,
                             const int error, const interfaceside_t side);
/*
 * Logs we got an unexpected error ("error") related to reading from a
 * udp socket.
 */
void
log_bind_failed(const char *function, const int protocol,
                const struct sockaddr_storage *address);
/*
 * Logs that we failed to perform a local bind of the local address "address".
 */


void
log_mtuproblem(const int s, const interfaceside_t side, const int fromus);
/*
 * Logs that there is a MTU problem from us to peer, or from peer to us.
 * If "fromus" is true, the problem is sending from us to peer, if
 * "fromus" is false, the problem is sendng from peer to us.
 */

#endif /* !SOCKS_CLIENT */

#endif /* !_FMT_H_ */
