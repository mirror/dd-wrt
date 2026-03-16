/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2003, 2006, 2008, 2009, 2010,
 *               2011, 2012, 2013, 2014
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

/* $Id: tostring.h,v 1.99.4.5.14.1 2024/12/05 11:55:59 michaels Exp $ */

#ifndef _TOSTRING_H_
#define _TOSTRING_H_

#define QUOTE(a)   a
#define QUOTE0()   ""

char *
aihints2string(const struct addrinfo *hints, char *string, size_t len);
/*
 * Returns a printable representation of "hints".
 *
 * If "string" and "len" is NULL and zero, the function returns a
 * string that will be overwritten on the next call to this function.
 */


char *
fdset2string(const int nfds, const fd_set *set, const int docheck,
             char *buf, size_t buflen);
/*
 * Returns a printable representation of the fd_set "set", which may
 * have up to "nfds" fds set.
 *
 * If "docheck" is true, will check each fd in the set for being a valid
 * fd.
 */

int32_t
string2portnumber(const char *string, char *emsg, size_t emsglen);
/*
 * Returns the integer corresponding to the portnumber represented by
 * the string "string", or -1 on error.
 * On error, "emsg" and "emsglen" contains the reason for error.
 */

enum operator_t
string2operator(const char *operator);
/*
 * Returns the enum for the string representation of a operator.
 * Can't fail.
 */

const char *
operator2string(enum operator_t operator);
/*
 * Returns the string representation of the operator.
 * Can't fail.
 */

/*
 * Bitmask of info to include when doing 2string conversion, in addition
 * to the IP-address or hostname.
 */
#define ADDRINFO_PORT     (1)
#define ADDRINFO_ATYPE    (2)
#define ADDRINFO_SCOPEID  (4)

const char *
ruleaddr2string(const ruleaddr_t *rule, const size_t includeinfo,
                char *string, size_t len)
                __ATTRIBUTE__((__BOUNDED__(__string__, 3, 4)));
/*
 * Writes "rule" out as a string.  The string is written to "string",
 * which is of size "len".
 *
 * If "includeinfo" is set, it specifies additional information,
 * apart from the ipaddress/hostname, to include in the string.
 *
 * If "string" and "len" is NULL and zero, the function returns a
 * string that will be overwritten on the next call to this function.
 *
 * Returns: "string".
 */

const char *
command2string(int command);
/*
 * Returns a printable representation of the socks command "command".
 * Can't fail.
 */

char *
commands2string(const command_t *command, char *str, size_t strsize)
      __ATTRIBUTE__((__NONNULL__(2)))
      __ATTRIBUTE__((__BOUNDED__(__string__, 2, 3)));
/*
 * Returns a printable representation of "commands".
 * "str" is the memory to write the printable representation into,
 * "strsize" is the size of the memory.
 *
 * Returns: "str", NUL terminated.
 */

char *
methods2string(size_t methodc, const int *methodv, char *str, size_t strsize)
      __ATTRIBUTE__((__BOUNDED__(__string__, 3, 4)));
/*
 * Returns a printable representation of the methods "methodv", of
 * length "methodc".
 * "str" is the memory to write the printable representation into,
 * "strsize" is the size of the memory.
 *
 * Returns: "str", NUL terminated.
 */

const char *
protocol2string(int protocol);
/*
 * Returns a printable representation of "protocol".
 * Can't fail.
 */

char *
protocols2string(const protocol_t *protocols, char *str, size_t strsize)
      __ATTRIBUTE__((__NONNULL__(2)))
      __ATTRIBUTE__((__BOUNDED__(__string__, 2, 3)));
/*
 * Returns a printable representation of "protocols".
 * "str" is the memory to write the printable representation into,
 * "strsize" is the size of the memory.
 *
 * Returns: "str", NUL terminated.
 */

char *
proxyprotocols2string(const proxyprotocol_t *proxyprotocols, char *str,
                      size_t strsize)
                      __ATTRIBUTE__((__BOUNDED__(__string__, 2, 3)));
/*
 * Returns a printable representation of "protocols".
 * "str" is the memory to write the printable representation into,
 * "strsize" is the size of the memory.
 *
 * If "str" and "strsize" is NULL and zero, the function returns a
 * string that will be overwritten on the next call to this function.
 *
 * Returns: "str", NUL terminated.
 */

const char *
method2string(int method);
/*
 * Returns a printable representation of the authmethod "method".
 */

const char *
gssapiprotection2string(const int protection);
/*
 * Returns a printable representation of the gssapi protection "protection".
 */

int
string2method(const char *methodname);
/*
 * If "methodname" is the name of a supported method, the protocol
 * value of that method is returned.
 * Otherwise, -1 is returned.
 */

char *
sockshost2string2(const sockshost_t *host, const size_t includeinfo,
                  char *string, size_t len)
      __ATTRIBUTE__((__BOUNDED__(__string__, 3, 4)));
/*
 * Writes "host" out as a string.  The string is written to "string",
 * which is of length "len", including NUL termination.
 *
 * If "includeinfo" is not zero, it indicates extra info to include.
 *
 * If "string" and "len" is NULL and zero, the function returns a
 * string that will be overwritten on the next call to this function.
 *
 * Returns: "string".
 */

char *
sockshost2string(const sockshost_t *host, char *string, size_t len)
      __ATTRIBUTE__((__BOUNDED__(__string__, 2, 3)));
/*
 * Wrapper around sockshost2string2() that does not include any extra info.
 */


char *
sockaddr2string2(const struct sockaddr_storage *addr, const size_t includeinfo,
                 char *string, size_t len)
                __ATTRIBUTE__((__BOUNDED__(__string__, 3, 4)));
/*
 * Returns the IP addr and port in "addr" on string form.
 *
 * "addr" is assumed to be on network form and it will be
 * converted to host form before written to "string".
 * "len" gives length of the NUL terminated string.
 *
 * If "includeinfo" is set, it specifies additional information,
 * apart from the ipaddress/hostname, to include in the string.
 *
 * If "string" and "len" is NULL and zero, the function returns a
 * string that will be overwritten on the next call to this function.
 *
 * Returns: "string".
 */

char *
sockaddr2string(const struct sockaddr_storage *addr, char *string, size_t len)
                __ATTRIBUTE__((__BOUNDED__(__string__, 2, 3)));
/*
 * Like sockaddr2string2(), but prints what we normally want to print.
 */


char *
addr2hexstring(const void *addr, const sa_family_t safamily,
               char *string, size_t len);
/*
 * prints "addr" of type "safamily", in hex format, to the string "string",
 * which should be of size "len".
 *
 * If "string" and "len" is zero, uses a statically allocated string.
 *
 * Returns "addr" on hex form, stored in "string".
 */

udpheader_t *
string2udpheader(const char *data, size_t len, udpheader_t *header)
      __ATTRIBUTE__((__NONNULL__(1)))
      __ATTRIBUTE__((__BOUNDED__(__string__, 1, 2)));
/*
 * Converts "data" to udpheader_t representation.
 * "len" is length of "data".
 * "data" is assumed to be in network order.
 * Returns:
 *      On success: pointer to a udpheader_t in static memory.
 *      On failure: NULL ("data" is not a valid udp packet).  In this case,
 *                  udpheader is bzero(3)-ed.
 */

const char *
socks_packet2string(const void *packet, int isrequest);
/*
 * debug function; dumps socks packet content.
 * "packet" is a socks packet, "isrequest" is set if it is a request
 * packet, false otherwise.
 *
 * Returns:
 *      On success: 0
 *      On failure: -1
 */

char *
extensions2string(const extension_t *extensions, char *str,
      size_t strsize)
      __ATTRIBUTE__((__NONNULL__(2)))
      __ATTRIBUTE__((__BOUNDED__(__string__, 2, 3)));
/*
 * Returns a printable representation of "extensions".
 * "str" is the memory to write the printable representation into,
 * "strsize" is the size of the memory.
 *
 * Returns: "str", NUL terminated.
 */

const char *
resolveprotocol2string(int resolveprotocol);
/*
 * Returns a printable representation of "resolveprotocol".
 */

char *
str2upper(char *string);
/*
 * converts all characters in "string" to uppercase.
 * returns "string".
 */

char *
sockname2string(const int s, char *buf, size_t buflen)
      __ATTRIBUTE__((__BOUNDED__(__string__, 2, 3)));
/*
 * Returns a printable representation of the local address of socket
 * "s", or NUL ("") on failure.
 */

char *
peername2string(const int s, char *buf, size_t buflen)
      __ATTRIBUTE__((__BOUNDED__(__string__, 2, 3)));
/*
 * Returns a printable representation of the remote address of socket
 * "s", or NUL ("") on failure.
 */

char *
socket2string(const int s, char *buf, size_t buflen)
      __ATTRIBUTE__((__BOUNDED__(__string__, 2, 3)));
/*
 * Prints out address info for the socket "s".
 * "buf" gives the buffer to write the address info to, "buflen" the
 * size of "buf".  If buflen is zero, a statically allocated buffer will
 * be used instead.
 *
 * Returns a pointer to buf.
 */

const char *
proxyprotocol2string(int version);
/*
 * Returns a printable representation of the proxy protocol version "version".
 */

const char *
atype2string(const unsigned int atype);
/*
 * Returns a printable representation of the atype "atype".
 */

const char *
safamily2string(const sa_family_t af);
/*
 * Returns a printable representation of the socket address family "af".
 */

const char *
socktype2string(const int socktype);
/*
 * Returns a printable representation of the socket type "socktype".
 */

char *
routeoptions2string(const routeoptions_t *options, char *str, size_t strsize);
/*
 * Returns a printable representation of the atype "atype".
 * If "str" and "strsize" is NULL and zero, the function returns a
 * string that will be overwritten on the next call to this function.
 */


const char *
loglevel2string(const int loglevel);
/*
 * returns a printable representation of the loglevel "loglevel".
 */

const char *
signal2string(const int sig);
/*
 * returns a printable representation of the signalnumber "sig".
 */


char *
logtypes2string(const logtype_t *logtypes, char *str, size_t strsize)
      __ATTRIBUTE__((__NONNULL__(2)))
      __ATTRIBUTE__((__BOUNDED__(__string__, 2, 3)));
/*
 * Returns a printable representation of "logtypes".
 * "str" is the memory to write the printable representation into,
 * "strsize" is the size of the memory.
 *
 * Returns: "str", NUL terminated.
 */

const char *
sockoptval2string(socketoptvalue_t value, socketoptvalue_type_t type,
                  char *str, size_t strsize);
/*
 * Returns a printable representation of socket option value "value",
 * type "type".
 */

const char *
sockoptlevel2string(int level);
/*
 * Returns a printable representation of the socket option "level" value.
 */

const char *
sockoptvaltype2string(socketoptvalue_type_t type);
/*
 * Returns a printable representation of the socket option "level" value.
 */

const char *
sockopt2string(const socketoption_t *opt, char *str, size_t strsize);
/*
 * Returns a printable representation of the socketoption_t "opt".
 */

char *
ltoa(long l, char *buf, size_t buflen);
/*
 * Returns the value of "l" as a string, stored in buf, which should
 * be of at least 22 bytes.
 * If "buf" is NULL, returns a pointer to a statically allocated string
 * instead of "buf".
 */

const char *
socketsettime2string(const int whichtime);
/*
 * Returns string representation of the socket option setting time "whichtime".
 */



#if !SOCKS_CLIENT
const char *
interfaceside2string(const interfaceside_t side);
/*
 * Returns a printable representation of the interface side "side".
 */

char *
interfaceprotocol2string(const interfaceprotocol_t *ifproto,
                         char *str, size_t strsize);
/*
 * Returns a printable representation of "if".
 */

char *
networktest2string(const networktest_t *test, char *str, size_t strlen);
/*
 * Returns a printable representation of "test".
 */

const char *
addrscope2string(const ipv6_addrscope_t scope);
/*
 * Returns string representation of the ipv6 address scope "scope".
 */

const char *
alarmside2string(const size_t alarmside);
/*
 * Returns string representation of the alarmside "alarmside".
 */

const char *
clientinfo2string(const clientinfo_t *cinfo, char *str, size_t strsize);
/*
 * Returns a printable representation of "cinfo".
 */

const char *
statekey2string(const statekey_t key);
/*
 * Returns a printable representation of "key".
 */

statekey_t
string2statekey(const char *string);
/*
 * Returns the statekey_t matching the string "string", or "unset" if
 * it does not match any valid key (unset is not a valid keyvalue).
 */

const char *
objecttype2string(const objecttype_t objecttype);
/*
 * returns a printable representation of "objecttype".
 */

char *
timeouts2string(const timeout_t *timeouts, const char *prefix,
      char *str, size_t strsize)
      __ATTRIBUTE__((__NONNULL__(3)))
      __ATTRIBUTE__((__BOUNDED__(__string__, 3, 4)));
/*
 * Returns a printable representation of "timeouts".
 * "prefix" is prefixed to every line written to "str".
 * "str" is the memory to write the printable representation into,
 * "strsize" is the size of the memory.
 *
 * Returns: "str", NUL terminated.
 */


const char *
timeouttype2string(const timeouttype_t type);
/*
 * Returns a printable representation of the timeout type "type".
 */

char *
logs2string(const log_t *logs, char *str, size_t strsize)
      __ATTRIBUTE__((__BOUNDED__(__string__, 2, 3)));
/*
 * Returns a printable representation of "logs".
 * "str" is the memory to write the printable representation into,
 * "strsize" is the size of the memory.
 *
 * Returns: "str", NUL terminated.
 */

#if !HAVE_PRIVILEGES
char *
userids2string(const userid_t *userids, const char *prefix, char *str,
      size_t strsize)
      __ATTRIBUTE__((__NONNULL__(3)))
      __ATTRIBUTE__((__BOUNDED__(__string__, 3, 4)));
/*
 * Returns a printable representation of "userids".
 * "prefix" is prefixed to every line written to "str".
 * "str" is the memory to write the printable representation into,
 * "strsize" is the size of the memory.
 *
 * Returns: "str", NUL terminated.
 */
#endif /* !HAVE_PRIVILEGES */

char *
options2string(const option_t *options, const char *prefix,
               char *str, size_t strsize)
      __ATTRIBUTE__((__NONNULL__(3)))
      __ATTRIBUTE__((__BOUNDED__(__string__, 3, 4)));
/*
 * Returns a printable representation of "options".
 * "prefix" is prefixed to every line written to "str".
 * "str" is the memory to write the printable representation into,
 * "strsize" is the size of the memory.
 * Returns: "str", NUL terminated.
 */

char *
compats2string(const compat_t *compats, char *str, size_t strsize)
      __ATTRIBUTE__((__NONNULL__(2)))
      __ATTRIBUTE__((__BOUNDED__(__string__, 2, 3)));
/*
 * Returns a printable representation of "compats".
 * "str" is the memory to write the printable representation into,
 * "strsize" is the size of the memory.
 *
 * Returns: "str", NUL terminated.
 */

char *
list2string(const linkedname_t *list, char *str, size_t strsize)
      __ATTRIBUTE__((__NONNULL__(2)))
      __ATTRIBUTE__((__BOUNDED__(__string__, 2, 3)));
/*
 * Returns a printable representation of "list".
 * "str" is the memory to write the printable representation into,
 * "strsize" is the size of the memory.
 *
 * Returns: "str", NUL terminated.
 */

const char *
childtype2string(int type);
/*
 * returns the string representation of "type".
 */

const char *
verdict2string(int verdict);
/*
 * returns the string representation of "verdict".
 */

char *
srchosts2string(const srchost_t *srchosts, const char *prefix,
      char *str, size_t strsize)
      __ATTRIBUTE__((__NONNULL__(3)))
      __ATTRIBUTE__((__BOUNDED__(__string__, 3, 4)));
/*
 * Returns a printable representation of "srchosts".
 * "prefix" is prefixed to every line written to "str".
 * "str" is the memory to write the printable representation into,
 * "strsize" is the size of the memory.
 *
 * Returns: "str", NUL terminated.
 */

const char *
uid2name(uid_t uid);
/*
 * If there is a mapping from "uid" to name, returns the name.
 * Otherwise returns NULL.
 */

const char *
rotation2string(int rotation);
/*
 * Returns a printable representation of "rotation".
 */

const char *
privop2string(const priv_op_t op);
/*
 * Returns a printable representation of "op".
 */

#if HAVE_SCHED_SETAFFINITY

char *
cpuset2string(const cpu_set_t *set, char *str, size_t strsize)
      __ATTRIBUTE__((__BOUNDED__(__string__, 2, 3)));
/*
 * returns a printable representation of the cpu_set_t "set".
 */

#endif /* HAVE_SCHED_SETAFFINITY */

#if COVENANT
const char *httpcode2string(const int version, const int code);
/*
 * Returns a short printable representation of the http version
 * "version" response code "code"
 */

#endif /* COVENANT */
#endif /* !SOCKS_CLIENT */

#endif /* !_TOSTRING_H_ */
