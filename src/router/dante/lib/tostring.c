/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2003, 2005, 2006, 2008, 2009,
 *               2010, 2011, 2012, 2013, 2014, 2019, 2024
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

#include "common.h"
#include "config_parse.h"

static const char rcsid[] =
"$Id: tostring.c,v 1.225.4.9.6.2.4.15 2024/12/05 11:55:59 michaels Exp $";

static const char *stripstring = ", \t\n";

char *
aihints2string(hints, buf, buflen)
   const struct addrinfo *hints;
   char *buf;
   size_t buflen;
{
   if (buf == NULL || buflen == 0) {
      static char _buf[64];

      buf    = _buf;
      buflen = sizeof(_buf);
   }

   *buf    = NUL;

   snprintf(buf, buflen,
            "ai_flags: %d, ai_family: %d, ai_socktype: %d, ai_protocol: %d",
            hints->ai_flags,
            hints->ai_family,
            hints->ai_socktype,
            hints->ai_protocol);

   return buf;
}

char *
fdset2string(nfds, set, docheck, buf, buflen)
   const int nfds;
   const fd_set *set;
   const int docheck;
   char *buf;
   size_t buflen;
{
   size_t bufused;
   int i, rc;

   if (buf == NULL || buflen == 0) {
      static char _buf[10240];

      buf    = _buf;
      buflen = sizeof(_buf);
   }

   bufused = 0;
   *buf    = NUL;

   if (set == NULL)
      return buf;

   for (i = 0; i < nfds; ++i) {
      if (FD_ISSET(i, set)) {
         rc = (int)snprintf(&buf[bufused], buflen - bufused,
                            "%d%s, ",
                            i, docheck ? (fdisopen(i) ? "" : "-invalid") : "");
         bufused += rc;
      }
   }

   return buf;
}

int32_t
string2portnumber(string, emsg, emsglen)
   const char *string;
   char *emsg;
   size_t emsglen;
{
   char *endptr, visbuf[256];
   long portnumber;

   if (emsg == NULL || emsglen == 0) {
      static char _emsg[256];

      emsg    = _emsg;
      emsglen = sizeof(_emsg);
   }

   portnumber = strtol(string, &endptr, 10);

   if (*endptr == NUL || *endptr == '/' || isspace(*endptr)) {
      if (portnumber >= 0 && portnumber <= IP_MAXPORT)
         return (int32_t)portnumber;
      else {
         snprintf(emsg, emsglen,
                  "portnumber given (%ld) is out of range.  Must be in the "
                  "range 0 - %u",
                  portnumber, IP_MAXPORT);

         return -1;
      }
   }

   snprintf(emsg, emsglen,
            "\"%s\" does not appear to be a valid portnumber in the range "
            "0 - %u",
            str2vis(string, strlen(string), visbuf, sizeof(visbuf)),
            IP_MAXPORT);

   return -1;
}

char *
proxyprotocols2string(proxyprotocols, str, strsize)
   const proxyprotocol_t *proxyprotocols;
   char *str;
   size_t strsize;
{
   size_t strused;

   if (strsize == 0) {
      static char buf[256];

      str = buf;
      strsize = sizeof(buf);
   }

   *str    = NUL;
   strused = 0;

   if (proxyprotocols->socks_v4)
      strused += snprintf(&str[strused], strsize - strused, "%s, ",
      QUOTE(PROXY_SOCKS_V4s));

   if (proxyprotocols->socks_v5)
      strused += snprintf(&str[strused], strsize - strused, "%s, ",
      QUOTE(PROXY_SOCKS_V5s));

   if (proxyprotocols->http)
      strused += snprintf(&str[strused], strsize - strused, "%s, ",
      QUOTE("HTTP"));

   if (proxyprotocols->upnp)
      strused += snprintf(&str[strused], strsize - strused, "%s, ",
      QUOTE(PROXY_UPNPs));

   if (proxyprotocols->direct)
      strused += snprintf(&str[strused], strsize - strused, "%s, ",
      QUOTE(PROXY_DIRECTs));

   STRIPTRAILING(str, strused, stripstring);
   return str;
}

char *
protocols2string(protocols, str, strsize)
   const protocol_t *protocols;
   char *str;
   size_t strsize;
{
   size_t strused;

   if (strsize == 0) {
      static char buf[16];

      str = buf;
      strsize = sizeof(buf);
   }

   *str    = NUL;
   strused = 0;

   if (protocols->tcp)
      strused += snprintf(&str[strused], strsize - strused, "%s, ",
      QUOTE(PROTOCOL_TCPs));

   if (protocols->udp)
      strused += snprintf(&str[strused], strsize - strused, "%s, ",
      QUOTE(PROTOCOL_UDPs));

   STRIPTRAILING(str, strused, stripstring);
   return str;
}

const char *
socks_packet2string(packet, isrequest)
     const void *packet;
     int isrequest;
{
   static char buf[1024];
   char hstr[MAXSOCKSHOSTSTRING];
   unsigned char version;
   const request_t *request = NULL;
   const response_t *response = NULL;

   if (isrequest) {
      request = (const request_t *)packet;
      version = request->version;
   }
   else {
      response = (const response_t *)packet;
      version  = response->version;
   }

   switch (version) {
      case PROXY_SOCKS_V4:
      case PROXY_SOCKS_V4REPLY_VERSION:
         if (isrequest) {
            SASSERTX(version == PROXY_SOCKS_V4);

            snprintf(buf, sizeof(buf),
                     "VER: %d CMD: %d address: %s",
                     request->version,
                     request->command,
                     sockshost2string(&request->host, hstr, sizeof(hstr)));
         }
         else {
            SASSERTX(version == PROXY_SOCKS_V4REPLY_VERSION);

            snprintf(buf, sizeof(buf),
                     "VER: %d REP: %d address: %s",
                     response->version,
                     response->reply.socks,
                     sockshost2string(&response->host, hstr, sizeof(hstr)));
         }

         break;

      case PROXY_SOCKS_V5:
         if (isrequest)
            snprintf(buf, sizeof(buf),
                     "VER: %d CMD: %d FLAG: %d ATYP: %d address: %s",
                     request->version,
                     request->command,
                     request->flag,
                     request->host.atype,
                     sockshost2string(&request->host, hstr, sizeof(hstr)));
         else
            snprintf(buf, sizeof(buf),
                     "VER: %d REP: %d FLAG: %d ATYP: %d address: %s",
                     response->version,
                     response->reply.socks,
                     response->flag,
                     response->host.atype,
                     sockshost2string(&response->host, hstr, sizeof(hstr)));

         break;

      case PROXY_HTTP_10:
      case PROXY_HTTP_11:
         if (isrequest)
            snprintf(buf, sizeof(buf),
                     "VER: %d CMD: %d ATYP: %d address: %s",
                     request->version,
                     request->command,
                     request->host.atype,
                     sockshost2string(&request->host, hstr, sizeof(hstr)));
         else
            snprintf(buf, sizeof(buf),
                     "VER: %d REP: %d ATYP: %d address: %s",
                     response->version,
                     response->reply.http,
                     response->host.atype,
                     sockshost2string(&response->host, hstr, sizeof(hstr)));

         break;

      default:
         SERRX(version);
  }

  return buf;
}

enum operator_t
string2operator(string)
   const char *string;
{

   if (strcmp(string, "eq") == 0 || strcmp(string, "=") == 0)
      return eq;

   if (strcmp(string, "ne") == 0 || strcmp(string, "!=") == 0)
      return neq;

   if (strcmp(string, "ge") == 0 || strcmp(string, ">=") == 0)
      return ge;

   if (strcmp(string, "le") == 0 || strcmp(string, "<=") == 0)
      return le;

   if (strcmp(string, "gt") == 0 || strcmp(string, ">") == 0)
      return gt;

   if (strcmp(string, "lt") == 0 || strcmp(string, "<") == 0)
      return lt;

   /* parser should make sure this never happens. */
   SERRX(0);

   /* NOTREACHED */
}

const char *
operator2string(operator)
   enum operator_t operator;
{

   switch (operator) {
      case none:
         return QUOTE("none");

      case eq:
         return QUOTE("eq");

      case neq:
         return QUOTE("neq");

      case ge:
         return QUOTE("ge");

      case le:
         return QUOTE("le");

      case gt:
         return QUOTE("gt");

      case lt:
         return QUOTE("lt");

      case range:
         return QUOTE("range");

      default:
         SERRX(operator);
   }

   /* NOTREACHED */
}

const char *
ruleaddr2string(address, includeinfo, string, len)
   const ruleaddr_t *address;
   const size_t includeinfo;
   char *string;
   size_t len;
{
   const char *function = "ruleaddr2string()";
   ssize_t lenused;

   if (string == NULL || len == 0) {
      static char addrstring[MAXRULEADDRSTRING];

      string = addrstring;
      len    = sizeof(addrstring);
   }

   lenused = 0;

   if (includeinfo & ADDRINFO_ATYPE)
      lenused += snprintf(string, len, "%s ", atype2string(address->atype));

   switch (address->atype) {
      case SOCKS_ADDR_IPV4: {
         char ntop[MAXSOCKADDRSTRING];

         if (inet_ntop(AF_INET, &address->addr.ipv4.ip, ntop, sizeof(ntop))
         == NULL)
            serr("%s: inet_ntop(3) failed on %s %x",
                 function,
                 atype2string(address->atype),
                 address->addr.ipv4.ip.s_addr);

         lenused += snprintf(&string[lenused], len - lenused, "%s/%d",
                             ntop,
                             bitcount((unsigned long)
                                               address->addr.ipv4.mask.s_addr));
         break;
      }

      case SOCKS_ADDR_IPV6: {
         char ntop[MAXSOCKADDRSTRING];

         if (inet_ntop(AF_INET6, &address->addr.ipv6.ip, ntop, sizeof(ntop))
         == NULL)
            serr("%s: inet_ntop(3) failed on %s " IP6_FMTSTR,
                 function,
                 atype2string(address->atype),
                 IP6_ELEMENTS(&address->addr.ipv6.ip));

         lenused += snprintf(&string[lenused], len - lenused, "%s/%u",
                             ntop, address->addr.ipv6.maskbits);
         break;
      }

      case SOCKS_ADDR_IPVANY:
         SASSERTX(address->addr.ipvany.ip.s_addr   == htonl(0));
         SASSERTX(address->addr.ipvany.mask.s_addr == htonl(0));

         lenused += snprintf(&string[lenused], len - lenused, "%d/%d",
                             ntohl(address->addr.ipvany.ip.s_addr),
                             bitcount((unsigned long)
                                           address->addr.ipvany.mask.s_addr));
         break;

      case SOCKS_ADDR_DOMAIN:
         lenused += snprintf(&string[lenused], len - lenused,
                             "%s", address->addr.domain);
         break;

      case SOCKS_ADDR_IFNAME:
         lenused += snprintf(&string[lenused], len - lenused,
                            "%s", address->addr.ifname);
         break;

      default:
         SERRX(address->atype);
   }

   if (includeinfo & ADDRINFO_PORT) {
      switch (address->operator) {
         case none:
            break;

         case eq:
         case neq:
         case ge:
         case le:
         case gt:
         case lt:
            if (address->port.tcp == address->port.udp)
               lenused += snprintf(&string[lenused], len - lenused,
                                   " port %s %u",
                                   operator2string(address->operator),
                                   ntohs(address->port.tcp));
            else
               lenused += snprintf(&string[lenused], len - lenused,
                                   " port %s %u (tcp) / %u (udp)",
                                   operator2string(address->operator),
                                   ntohs(address->port.tcp),
                                   ntohs(address->port.udp));
            break;

         case range:
            SASSERTX(address->port.tcp == address->port.udp);
            lenused += snprintf(&string[lenused], len - lenused,
                                " port %s %u - %u",
                                operator2string(address->operator),
                                ntohs(address->port.tcp),
                                ntohs(address->portend));
            break;

         default:
            SERRX(address->operator);
      }
   }

   return string;
}

const char *
protocol2string(protocol)
   int protocol;
{

   switch (protocol) {
      case SOCKS_TCP:
         return QUOTE(PROTOCOL_TCPs);

      case SOCKS_UDP:
         return QUOTE(PROTOCOL_UDPs);

      default:
         SERRX(protocol);
   }

   /* NOTREACHED */
}

const char *
resolveprotocol2string(resolveprotocol)
   int resolveprotocol;
{
   switch (resolveprotocol) {
      case RESOLVEPROTOCOL_TCP:
         return QUOTE(PROTOCOL_TCPs);

      case RESOLVEPROTOCOL_UDP:
         return QUOTE(PROTOCOL_UDPs);

      case RESOLVEPROTOCOL_FAKE:
         return QUOTE("fake");

      default:
         SERRX(resolveprotocol);
   }

   /* NOTREACHED */
}

const char *
command2string(command)
   int command;
{

   switch (command) {
      case SOCKS_BIND:
         return QUOTE(SOCKS_BINDs);

      case SOCKS_CONNECT:
         return QUOTE(SOCKS_CONNECTs);

      case SOCKS_UDPASSOCIATE:
         return QUOTE(SOCKS_UDPASSOCIATEs);

      /* pseudo commands. */
      case SOCKS_ACCEPT:
         return QUOTE(SOCKS_ACCEPTs);

      case SOCKS_BINDREPLY:
         return QUOTE(SOCKS_BINDREPLYs);

      case SOCKS_UDPREPLY:
         return QUOTE(SOCKS_UDPREPLYs);

      case SOCKS_DISCONNECT:
         return QUOTE(SOCKS_DISCONNECTs);

      case SOCKS_BOUNCETO:
         return QUOTE(SOCKS_BOUNCETOs);

      case SOCKS_HOSTID:
         return QUOTE(SOCKS_HOSTIDs);

      case SOCKS_UNKNOWN:
         return QUOTE(SOCKS_UNKNOWNs);

      default:
         SERRX(command);
   }

   /* NOTREACHED */
}

char *
commands2string(command, str, strsize)
   const command_t *command;
   char *str;
   size_t strsize;
{
   size_t strused;

   if (strsize == 0) {
      static char buf[128];

      str = buf;
      strsize = sizeof(buf);
   }

   *str    = NUL;
   strused = 0;

   if (command->bind)
      strused += snprintf(&str[strused], strsize - strused, "%s, ",
      command2string(SOCKS_BIND));

   if (command->bindreply)
      strused += snprintf(&str[strused], strsize - strused, "%s, ",
      command2string(SOCKS_BINDREPLY));

   if (command->connect)
      strused += snprintf(&str[strused], strsize - strused, "%s, ",
      command2string(SOCKS_CONNECT));

   if (command->udpassociate)
      strused += snprintf(&str[strused], strsize - strused, "%s, ",
      command2string(SOCKS_UDPASSOCIATE));

   if (command->udpreply)
      strused += snprintf(&str[strused], strsize - strused, "%s, ",
      command2string(SOCKS_UDPREPLY));

   STRIPTRAILING(str, strused, stripstring);
   return str;
}

const char *
method2string(method)
   int method;
{

   switch (method) {
      case AUTHMETHOD_NOTSET:
         return QUOTE(AUTHMETHOD_NOTSETs);

      case AUTHMETHOD_NONE:
         return QUOTE(AUTHMETHOD_NONEs);

      case AUTHMETHOD_GSSAPI:
         return QUOTE(AUTHMETHOD_GSSAPIs);

      case AUTHMETHOD_UNAME:
         return QUOTE(AUTHMETHOD_UNAMEs);

      case AUTHMETHOD_NOACCEPT:
         return QUOTE(AUTHMETHOD_NOACCEPTs);

      case AUTHMETHOD_RFC931:
         return QUOTE(AUTHMETHOD_RFC931s);

      case AUTHMETHOD_PAM_ANY:
         return QUOTE(AUTHMETHOD_PAM_ANYs);

      case AUTHMETHOD_PAM_ADDRESS:
         return QUOTE(AUTHMETHOD_PAM_ADDRESSs);

      case AUTHMETHOD_PAM_USERNAME:
         return QUOTE(AUTHMETHOD_PAM_USERNAMEs);

      case AUTHMETHOD_BSDAUTH:
         return QUOTE(AUTHMETHOD_BSDAUTHs);

      case AUTHMETHOD_LDAPAUTH:
         return QUOTE(AUTHMETHOD_LDAPAUTHs);

      default:
         return "<unknown>";
   }

   /* NOTREACHED */
}

const char *
proxyprotocol2string(version)
   int version;
{

   switch (version) {
      case PROXY_SOCKS_V4:
         return QUOTE(PROXY_SOCKS_V4s);

      case PROXY_SOCKS_V5:
         return QUOTE(PROXY_SOCKS_V5s);

      case PROXY_HTTP_10:
         return QUOTE(PROXY_HTTP_10s);

      case PROXY_HTTP_11:
         return QUOTE(PROXY_HTTP_11s);

      case PROXY_UPNP:
         return QUOTE(PROXY_UPNPs);

      case PROXY_DIRECT:
         return QUOTE(PROXY_DIRECTs);

      default:
         SERRX(version);
   }

   /* NOTREACHED */
}

char *
methods2string(methodc, methodv, str, strsize)
   size_t methodc;
   const int methodv[];
   char *str;
   size_t strsize;
{
   size_t strused;
   size_t i;

   if (strsize == 0) {
      static char buf[512];

      str     = buf;
      strsize = sizeof(buf);
   }

   *str    = NUL;
   strused = 0;

   for (i = 0; i < methodc; ++i)
      strused += snprintf(&str[strused], strsize - strused,
                          "%s, ", method2string(methodv[i]));

   STRIPTRAILING(str, strused, stripstring);
   return str;
}

int
string2method(methodname)
   const char *methodname;
{
   struct {
      const char *methodname;
      int        method;
   } method[] = {
      { AUTHMETHOD_NONEs,         AUTHMETHOD_NONE         },
      { AUTHMETHOD_UNAMEs,        AUTHMETHOD_UNAME        },
      { AUTHMETHOD_GSSAPIs,       AUTHMETHOD_GSSAPI       },
      { AUTHMETHOD_RFC931s,       AUTHMETHOD_RFC931       },
      { AUTHMETHOD_PAM_ANYs,      AUTHMETHOD_PAM_ANY      },
      { AUTHMETHOD_PAM_ADDRESSs,  AUTHMETHOD_PAM_ADDRESS  },
      { AUTHMETHOD_PAM_USERNAMEs, AUTHMETHOD_PAM_USERNAME },
      { AUTHMETHOD_BSDAUTHs,      AUTHMETHOD_BSDAUTH      },
      { AUTHMETHOD_LDAPAUTHs,     AUTHMETHOD_LDAPAUTH     }
   };
   size_t i;

   for (i = 0; i < ELEMENTS(method); ++i)
      if (strcmp(method[i].methodname, methodname) == 0)
         return method[i].method;

   return -1;
}

char *
sockshost2string2(host, includeinfo, string, len)
   const sockshost_t *host;
   const size_t includeinfo;
   char *string;
   size_t len;
{
   size_t lenused;
   char visbuf[sizeof(*host) * 4];

   if (string == NULL || len == 0) {
      static char hstr[sizeof(visbuf)];

      string = hstr;
      len    = sizeof(hstr);
   }

   lenused = 0;

   if (includeinfo & ADDRINFO_ATYPE)
      lenused += snprintf(&string[lenused], len - lenused, "%s ",
                          atype2string(host->atype));

   switch (host->atype) {
      case SOCKS_ADDR_IPV4: {
         char b[MAX(INET_ADDRSTRLEN, 32)];

         if (inet_ntop(AF_INET, &host->addr.ipv4, b, sizeof(b)) == NULL)
            STRCPY_ASSERTSIZE(b, "<nonsense address>");

         lenused += snprintf(&string[lenused], len - lenused, "%s", b);
         break;
      }

      case SOCKS_ADDR_IPV6: {
         char b[MAX(INET6_ADDRSTRLEN, 32)];

         if (inet_ntop(AF_INET6, &host->addr.ipv6, b, sizeof(b)) == NULL)
            STRCPY_ASSERTSIZE(b, "<nonsense address>");

         lenused += snprintf(&string[lenused], len - lenused, "%s", b);
         break;
      }

      case SOCKS_ADDR_DOMAIN:
         lenused += snprintf(&string[lenused], len - lenused, "%s",
                             str2vis(host->addr.domain,
                                     strlen(host->addr.domain),
                                     visbuf,
                                     sizeof(visbuf)));
         break;

      case SOCKS_ADDR_IFNAME:
         lenused += snprintf(&string[lenused], len - lenused, "%s",
                             str2vis(host->addr.ifname,
                                      strlen(host->addr.ifname),
                                      visbuf,
                                      sizeof(visbuf)));
         break;

      case SOCKS_ADDR_URL:
         lenused += snprintf(&string[lenused], len - lenused, "%s",
                             str2vis(host->addr.urlname,
                                     strlen(host->addr.urlname),
                                     visbuf,
                                     sizeof(visbuf)));
         break;

      default:
         SERRX(host->atype);
   }

   if (includeinfo & ADDRINFO_PORT) {
      switch (host->atype) {
         case SOCKS_ADDR_IPV4:
         case SOCKS_ADDR_IPV6:
         case SOCKS_ADDR_DOMAIN:
            lenused += snprintf(&string[lenused], len - lenused,
                                ".%d", ntohs(host->port));
            break;
      }
   }

   return string;
}

char *
sockshost2string(host, string, len)
   const sockshost_t *host;
   char *string;
   size_t len;
{

   return sockshost2string2(host, ADDRINFO_PORT, string, len);
}

char *
sockaddr2string(addr, string, len)
   const struct sockaddr_storage *addr;
   char *string;
   size_t len;
{
   if (string == NULL || len == 0) {
      static char addrstring[MAXSOCKADDRSTRING];

      string = addrstring;
      len    = sizeof(addrstring);
   }


   return sockaddr2string2(addr, ADDRINFO_PORT, string, len);
}


char *
sockaddr2string2(addr, includeinfo, string, len)
   const struct sockaddr_storage *addr;
   const size_t includeinfo;
   char *string;
   size_t len;
{
   size_t lenused = 0;

   if (string == NULL || len == 0) {
      static char addrstring[MAX(MAXSOCKADDRSTRING, 256)];

      string = addrstring;
      len    = sizeof(addrstring);
   }

   if (includeinfo & ADDRINFO_ATYPE)
      lenused += snprintf(&string[lenused], len - lenused, "%s ",
                          safamily2string(addr->ss_family));

   switch (addr->ss_family) {
      case AF_INET:
      case AF_INET6: {
         if (inet_ntop(addr->ss_family,
                       GET_SOCKADDRADDR(addr),
                       &string[lenused],
                       (socklen_t)(len - lenused)) != NULL) {
            if (addr->ss_family == AF_INET6
            &&  includeinfo & ADDRINFO_SCOPEID
            &&  TOCIN6(addr)->sin6_scope_id != 0) {
               lenused = strlen(string);

               snprintf(&string[lenused], len - lenused,
                         "%u", (unsigned int)TOCIN6(addr)->sin6_scope_id);
            }

            if (includeinfo & ADDRINFO_PORT) {
               lenused = strlen(string);

               snprintf(&string[lenused], len - lenused,
                        ".%d", ntohs(GET_SOCKADDRPORT(addr)));
            }
         }
         else {
            char addrstr[MAXSOCKADDRSTRING];

            switch (addr->ss_family) {
               case AF_INET:
                  snprintf(addrstr, sizeof(addrstr),
                           "0x%x", TOCIN(addr)->sin_addr.s_addr);
                  break;

               case AF_INET6:
                  snprintf(addrstr, sizeof(addrstr),
                           IP6_FMTSTR, IP6_ELEMENTS(&TOCIN6(addr)->sin6_addr));
                  break;

               default:
                  SERRX(addr->ss_family);
            }

            snprintf(string, len,
                     "<inet_ntop(3) on af %d, addr %s, failed: %s>",
                     addr->ss_family,
                     strerror(errno),
                     addrstr);

            errno = 0;
         }

         break;
      }

      default:
         snprintf(string, len, "<undecoded af %d>", addr->ss_family);
   }

   return string;
}

char *
addr2hexstring(addr, safamily, string, len)
   const void *addr;
   const sa_family_t safamily;
   char *string;
   size_t len;
{
   if (string == NULL || len == 0) {
      static char stringmem[sizeof(IP6_FMTSTR)];

      string = stringmem;
      len    = sizeof(stringmem);
   }

   switch (safamily) {
      case AF_INET:
         snprintf(string, len, "0x%x", ((const struct in_addr *)addr)->s_addr);
         break;

      case AF_INET6:
         snprintf(string, len,
                  IP6_FMTSTR,
                  IP6_ELEMENTS(((const struct in6_addr *)addr)));
         break;

      default:
         SERRX(safamily);
   }

   return string;
}

udpheader_t *
string2udpheader(data, len, header)
   const char *data;
   size_t len;
   udpheader_t *header;
{

   bzero(header, sizeof(*header));

   if (len < MINSOCKSUDPHLEN)
      return NULL;

   if (len < sizeof(header->flag))
      return NULL;

   memcpy(&header->flag, data, sizeof(header->flag));
   data += sizeof(header->flag);
   len -= sizeof(header->flag);

   if (len < sizeof(header->frag))
      return NULL;

   memcpy(&header->frag, data, sizeof(header->frag));
   data += sizeof(header->frag);
   len  -= sizeof(header->frag);

   if (mem2sockshost(&header->host,
                     (const unsigned char *)data,
                     len,
                     PROXY_SOCKS_V5) == NULL) {
      bzero(header, sizeof(*header));
      return NULL;
   }

   return header;
}

char *
extensions2string(extensions, str, strsize)
   const extension_t *extensions;
   char *str;
   size_t strsize;
{
   size_t strused;

   if (strsize == 0) {
      static char buf[16];

      str = buf;
      strsize = sizeof(buf);
   }

   *str    = NUL;
   strused = 0;

   if (extensions->bind)
      strused += snprintf(&str[strused], strsize - strused, "%s, ",
      QUOTE("bind"));

   STRIPTRAILING(str, strused, stripstring);
   return str;
}

char *
str2upper(string)
   char *string;
{

   while (*string != NUL) {
      *string = (char)toupper(*string);
      ++string;
   }

   return string;
}

char *
sockname2string(
   const int s,
   char *buf,
   size_t buflen)
{
   const char *function = "sockname2string()";
   struct sockaddr_storage addr;
   socklen_t len;

   if (buflen == 0) {
      static char sbuf[256];

      buf    = sbuf;
      buflen = sizeof(sbuf);
   }
   else
      SASSERTX(buflen >= MAXSOCKADDRSTRING);

   if (s == -1) {
      snprintf(buf, buflen, "<N/A>");
      return buf;
   }

   len = sizeof(addr);
   if (getsockname(s, TOSA(&addr), &len) == -1) {
      snprintf(buf, buflen, "<N/A>");
      return buf;
   }

   return sockaddr2string(&addr, buf, buflen);
}

char *
peername2string(
   const int s,
   char *buf,
   size_t buflen)
{
   const char *function = "peername2string()";
   struct sockaddr_storage addr;
   socklen_t len;

   if (buflen == 0) {
      static char sbuf[256];

      buf    = sbuf;
      buflen = sizeof(sbuf);
   }
   else
      SASSERTX(buflen >= MAXSOCKADDRSTRING);

   if (s == -1) {
      snprintf(buf, buflen, "<N/A>");
      return buf;
   }

   len = sizeof(addr);
   if (getpeername(s, TOSA(&addr), &len) == -1) {
      snprintf(buf, buflen, "<N/A>");
      return buf;
   }

   return sockaddr2string(&addr, buf, buflen);
}

char *
socket2string(s, buf, buflen)
   const int s;
   char *buf;
   size_t buflen;
{
   const char *function = "socket2string()";
   const int errno_s    = errno;
   const char *protocol;
   socklen_t len;
   char src[MAXSOCKADDRSTRING], dst[MAXSOCKADDRSTRING];
   int val;

   if (buflen == 0) {
      static char sbuf[256];

      buf    = sbuf;
      buflen = sizeof(sbuf);
   }

   sockname2string(s, src, sizeof(src));
   peername2string(s, dst, sizeof(dst));

   len = sizeof(val);
   if (getsockopt(s, SOL_SOCKET, SO_TYPE, &val, &len) == -1)
      protocol = NULL;
   else
      switch (val) {
         case SOCK_DGRAM:
            protocol = PROTOCOL_UDPs;
            break;

         case SOCK_STREAM:
            protocol = PROTOCOL_TCPs;
            break;

         default:
            protocol = "unknown";
      }

   snprintf(buf, buflen,
            "laddr: %s, raddr: %s, protocol: %s",
            *src     == NUL  ? "N/A" : src,
            *dst     == NUL  ? "N/A" : dst,
            protocol == NULL ? "N/A" : protocol);

   errno = errno_s;
   return buf;
}

const char *
atype2string(atype)
   const unsigned int atype;
{

   switch (atype) {
      case SOCKS_ADDR_IPV4:
         return "IPv4 address";

      case SOCKS_ADDR_IPV6:
         return "IPv6 address";

      case SOCKS_ADDR_IPVANY:
         return "<IPvAny> address";

      case SOCKS_ADDR_IFNAME:
         return "interfacename";

      case SOCKS_ADDR_DOMAIN:
         return "host/domain-name";

      case SOCKS_ADDR_URL:
         return "url";

      default:
         SERRX(atype);
   }

   /* NOTREACHED */
}

const char *
safamily2string(af)
   const sa_family_t af;
{

   switch (af) {
      case AF_INET:
         return atype2string(SOCKS_ADDR_IPV4);

      case AF_INET6:
         return atype2string(SOCKS_ADDR_IPV6);

      case AF_UNSPEC:
         return "AF_UNSPEC";

      case AF_LOCAL:
         return "AF_LOCAL";

      default: {
         static char buf[sizeof("unknown socket address family: 65535")];

         snprintf(buf, sizeof(buf), "<unknown socket address family: %d>", af);
         return buf;
      }
   }

   /* NOTREACHED */
}

const char *
socktype2string(socktype)
   const int socktype;
{

   switch (socktype) {
      case SOCK_STREAM:
         return "SOCK_STREAM";

      case SOCK_DGRAM:
         return "SOCK_DGRAM";

      default:
         return "<UNKNOWN>";
   }

   /* NOTREACHED */
}

char *
ltoa(l, buf, buflen)
   long l;
   char *buf;
   size_t buflen;
{
   char *p;
   size_t bufused;
   int add_minus;

   if (buf == NULL || buflen == 0) {
      /* for a 64 bits integer. */
      static char bufmem[(sizeof("-") - 1) + sizeof("18446744073709551616")];

      buf    = bufmem;
      buflen = sizeof(bufmem);
   }
   else if (buflen == 1) {
      *buf = NUL;
      return buf;
   }

   p  = &buf[buflen - 1]; /* start at end and go backwards. */
   *p = NUL;

   if (l < 0) {
      l         = -l;
      add_minus = 1;
   }
   else
      add_minus = 0;

   do {
      *(--p)  = (char)((l % 10) + '0');
      l      /= 10;
   } while (l != 0 && p > buf);

   if (l != 0
   || (p == buf && add_minus)) { /* buf too small / number too large. */
      SASSERTX(p == buf);

      errno = ERANGE;
      *buf  = NUL;

      return buf;
   }

   if (add_minus)
      *(--p) = '-';

   bufused = (&buf[buflen - 1] - p) + 1;
   SASSERTX(p + (bufused - 1) <= &buf[buflen - 1]);

   memmove(buf, p, bufused);
   SASSERTX(buf[bufused - 1] == NUL);

   return buf;
}

#if HAVE_GSSAPI
const char *
gssapiprotection2string(protection)
   const int protection;
{
   switch (protection) {
      case SOCKS_GSSAPI_CLEAR:
         return "clear";

      case SOCKS_GSSAPI_INTEGRITY:
         return "integrity";

      case SOCKS_GSSAPI_CONFIDENTIALITY:
         return "confidentiality";

      case SOCKS_GSSAPI_PERMESSAGE:
         return "per-message";
   }

   return "unknown gssapi protection";
}
#endif /* HAVE_GSSAPI */

char *
routeoptions2string(options, str, strsize)
   const routeoptions_t *options;
   char *str;
   size_t strsize;
{
   size_t strused;

   if (strsize == 0) {
      static char buf[512];

      str = buf;
      strsize = sizeof(buf);
   }

   *str    = NUL;
   strused = 0;

   strused += snprintf(&str[strused], strsize - strused,
                       "\"badexpire: %lu\", ",
                       (long)options->badexpire);

   strused += snprintf(&str[strused], strsize - strused,
                       "\"maxfail: %lu\"",
                       (unsigned long)options->maxfail);

   return str;
}

char *
logtypes2string(logtypes, str, strsize)
   const logtype_t *logtypes;
   char *str;
   size_t strsize;
{
   size_t strused;
   size_t i;

   if (strsize == 0) {
      static char buf[512];

      str = buf;
      strsize = sizeof(buf);
   }

   *str    = NUL;
   strused = 0;

   if (logtypes->type & LOGTYPE_SYSLOG)
      strused += snprintf(&str[strused], strsize - strused, "\"syslog.%s\", ",
      logtypes->facilityname);

   if (logtypes->type & LOGTYPE_FILE)
      for (i = 0; i < logtypes->filenoc; ++i)
         strused += snprintf(&str[strused], strsize - strused, "\"%s\", ",
         logtypes->fnamev[i]);

   STRIPTRAILING(str, strused, stripstring);
   return str;
}

const char *
loglevel2string(loglevel)
   const int loglevel;
{

   switch (loglevel) {
      case LOG_EMERG:
         return "emergency";

      case LOG_ALERT:
         return "alert";

      case LOG_CRIT:
         return "critical";

      case LOG_ERR:
         return "error";

      case LOG_WARNING:
         return "warning";

      case LOG_NOTICE:
         return "notice";

      case LOG_INFO:
         return "info";

      case LOG_DEBUG:
         return "debug";

      default:
         SWARNX(loglevel);
         return "unknown loglevel";
   }
}

const char *
signal2string(sig)
   const int sig;
{

   switch (sig) {
#ifdef SIGABRT
      case SIGABRT:
         return "SIGABRT";
#endif /* SIGABRT */

#ifdef SIGALRM
      case SIGALRM:
         return "SIGALRM";
#endif /* SIGALRM */

#ifdef SIGBUS
      case SIGBUS:
         return "SIGBUS";
#endif /* SIGBUS */

#ifdef SIGCANCEL
      case SIGCANCEL:
         return "SIGCANCEL";
#endif /* SIGCANCEL */

#ifdef SIGCHLD
      case SIGCHLD:
         return "SIGCHLD";
#endif /* SIGCHLD */

#if (defined SIGCLD) && SIGCLD != SIGCHLD
      case SIGCLD:
         return "SIGCLD";
#endif /* SIGCLD */

#ifdef SIGCONT
      case SIGCONT:
         return "SIGCONT";
#endif /* SIGCONT */

#ifdef SIGEMT
      case SIGEMT:
         return "SIGEMT";
#endif /* SIGEMT */

#ifdef SIGFPE
      case SIGFPE:
         return "SIGFPE";
#endif /* SIGFPE */

#ifdef SIGFREEZE
      case SIGFREEZE:
         return "SIGFREEZE";
#endif /* SIGFREEZE */

#ifdef SIGHUP
      case SIGHUP:
         return "SIGHUP";
#endif /* SIGHUP */

#ifdef SIGILL
      case SIGILL:
         return "SIGILL";
#endif /* SIGILL */

#ifdef SIGINFO
      case SIGINFO:
         return "SIGINFO";
#endif /* SIGINFO */

#ifdef SIGINT
      case SIGINT:
         return "SIGINT";
#endif /* SIGINT */

#ifdef SIGIO
      case SIGIO:
         return "SIGIO";
#endif /* SIGIO */

#if (defined SIGIOT) && SIGIOT != SIGABRT
      case SIGIOT:
         return "SIGIOT";
#endif /* SIGIOT && SIGIOT != SIGABRT */

#ifdef SIGJVM1
      case SIGJVM1:
         return "SIGJVM1";
#endif /* SIGJVM1 */

#ifdef SIGJVM2
      case SIGJVM2:
         return "SIGJVM2";
#endif /* SIGJVM2 */

#ifdef SIGKILL
      case SIGKILL:
         return "SIGKILL";
#endif /* SIGKILL */

#if (defined SIGLOST) && (!defined SIGABRT || SIGLOST != SIGABRT)
      case SIGLOST:
         return "SIGLOST";
#endif /* SIGLOST */

#if (defined SIGLWP) && (!defined SIGTHR || SIGLWP != SIGTHR)
      case SIGLWP:
         return "SIGLWP";
#endif /* SIGLWP */

#ifdef SIGPIPE
      case SIGPIPE:
         return "SIGPIPE";
#endif /* SIGPIPE */

#if (defined SIGPOLL) && SIGPOLL != SIGIO
      case SIGPOLL:
         return "SIGPOLL";
#endif /* SIGPOLL */

#ifdef SIGPROF
      case SIGPROF:
         return "SIGPROF";
#endif /* SIGPROF */

#ifdef SIGPWR
      case SIGPWR:
         return "SIGPWR";
#endif /* SIGPWR */

#ifdef SIGQUIT
      case SIGQUIT:
         return "SIGQUIT";
#endif /* SIGQUIT */

#ifdef SIGSEGV
      case SIGSEGV:
         return "SIGSEGV";
#endif /* SIGSEGV */

#ifdef SIGSTKFLT
      case SIGSTKFLT:
         return "SIGSTKFLT";
#endif /* SIGSTKFLT */

#ifdef SIGSTOP
      case SIGSTOP:
         return "SIGSTOP";
#endif /* SIGSTOP */

#ifdef SIGSYS
      case SIGSYS:
         return "SIGSYS";
#endif /* SIGSYS */

#ifdef SIGTERM
      case SIGTERM:
         return "SIGTERM";
#endif /* SIGTERM */

#ifdef SIGTHAW
      case SIGTHAW:
         return "SIGTHAW";
#endif /* SIGTHAW */

#ifdef SIGTHR
      case SIGTHR:
         return "SIGTHR";
#endif /* SIGTHR */

#ifdef SIGTRAP
      case SIGTRAP:
         return "SIGTRAP";
#endif /* SIGTRAP */

#ifdef SIGTSTP
      case SIGTSTP:
         return "SIGTSTP";
#endif /* SIGTSTP */

#ifdef SIGTTIN
      case SIGTTIN:
         return "SIGTTIN";
#endif /* SIGTTIN */

#ifdef SIGTTOU
      case SIGTTOU:
         return "SIGTTOU";
#endif /* SIGTTOU */

#ifdef SIGURG
      case SIGURG:
         return "SIGURG";
#endif /* SIGURG */

#ifdef SIGUSR1
      case SIGUSR1:
         return "SIGUSR1";
#endif /* SIGUSR1 */

#ifdef SIGUSR2
      case SIGUSR2:
         return "SIGUSR2";
#endif /* SIGUSR2 */

#ifdef SIGVTALRM
      case SIGVTALRM:
         return "SIGVTALRM";
#endif /* SIGVTALRM */

#ifdef SIGWAITING
      case SIGWAITING:
         return "SIGWAITING";
#endif /* SIGWAITING */

#ifdef SIGWINCH
      case SIGWINCH:
         return "SIGWINCH";
#endif /* SIGWINCH */

#ifdef SIGXCPU
      case SIGXCPU:
         return "SIGXCPU";
#endif /* SIGXCPU */

#ifdef SIGXFSZ
      case SIGXFSZ:
         return "SIGXFSZ";
#endif /* SIGXFSZ */

#ifdef SIGXRES
      case SIGXRES:
         return "SIGXRES";
#endif /* SIGXRES */
   }

   return "<unknown signal>";
}


#undef strerror
const char *
socks_strerror(err)
   const int err;
{
   const int errno_s = errno;
   char *errstr;

   if (sockscf.state.insignal)
      return "<cannot retrieve errno string while in signalhandler>";

   if (err == 0)
      return "no system error";

   errstr = strerror(err);

   if (errno != errno_s
   &&  errno != EINVAL)
      /*
       * don't expect strerror(3) to change errno normally, but on
       * OpenBSD it for some reason can. :-(
       */
      errno  = errno_s;

   return errstr;
}

const char *
sockoptval2string(value, type, str, strsize)
   socketoptvalue_t value;
   socketoptvalue_type_t type;
   char *str;
   size_t strsize;
{
   size_t strused;

   if (strsize == 0) {
      static char buf[100];

      str     = buf;
      strsize = sizeof(buf);
   }

   *str    = NUL;
   strused = 0;

   switch (type) {
      case int_val:
         strused += snprintf(&str[strused], strsize - strused, "%d",
                             value.int_val);
         break;

      case uchar_val:
         strused += snprintf(&str[strused], strsize - strused, "%u",
                             (unsigned)value.uchar_val);
         break;

      case linger_val:
      case timeval_val:
      case in_addr_val:
      case sockaddr_val:
      case ipoption_val:
#if HAVE_TCP_IPA
      case option28_val:
#endif /* HAVE_TCP_IPA */
#if HAVE_TCP_EXP1
      case option253_val:
#endif /* HAVE_TCP_EXP1 */

         strused += snprintf(&str[strused], strsize - strused,
                             "<value-decoding unimplemented>");
         break;

      default:
         SERRX(type);
   }

   STRIPTRAILING(str, strused, stripstring);
   return str;
}

const char *
sockoptlevel2string(level)
   int level;
{

   switch (level) {
#ifdef SOL_SOCKET
      case SOL_SOCKET:
         return "socket";
#endif /* SOL_SOCKET */

#ifdef IPPROTO_TCP
      case IPPROTO_TCP:
         return "tcp";
#endif /* IPPROTO_TCP */

#ifdef IPPROTO_UDP
      case IPPROTO_UDP:
         return "udp";
#endif /* IPPROTO_UDP */

#ifdef IPPROTO_IP
      case IPPROTO_IP:
         return "ip";
#endif /* IPPROTO_IP */

      default:
         SERRX(level);
   }

   /* NOTREACHED */
}

const char *
sockoptvaltype2string(type)
   socketoptvalue_type_t type;
{

   switch (type) {
      case int_val:
         return "int_val";

      case linger_val:
         return "linger_val";

      case timeval_val:
         return "timeval_val";

      case in_addr_val:
         return "in_addr_val";

      case uchar_val:
         return "uchar_val";

      case sockaddr_val:
         return "sockaddr_val";

      case ipoption_val:
         return "ipoption_val";

      case option28_val:
         return "option28_val";

      case option253_val:
         return "option253_val";
   }

   SERRX(type);
   /* NOTREACHED */
}

const char *
sockopt2string(opt, str, strsize)
   const socketoption_t *opt;
   char *str;
   size_t strsize;
{
   size_t strused;

   if (strsize == 0) {
      static char buf[1024];

      str     = buf;
      strsize = sizeof(buf);
   }

   strused = snprintf(str, strsize,
                      "%s (%d), level %s (%d), calltype %d, %s-side",
                      opt->info == NULL ? "<unknown>" : opt->info->name,
                      opt->optname,
                      sockoptlevel2string(opt->info == NULL ?
                                                 opt->level : opt->info->level),
                      opt->info == NULL ?        opt->level : opt->info->level,
                      opt->info == NULL ?        -1 : (int)opt->info->calltype,
                      opt->info == NULL ?        "<unknown>" 
                                                : opt->isinternalside ?
                                                "internal" : "external");

#if 1

   strused += snprintf(&str[strused], strsize - strused, " value: %s (%s)",
                       opt->opttype == 0 ? "<unknown>" 
                                         : sockoptval2string(opt->optval, 
                                                             opt->opttype, 
                                                             NULL, 
                                                             0),
                       opt->opttype == 0 ? "<unknown>" 
                                         : sockoptvaltype2string(opt->opttype));

#endif

   STRIPTRAILING(str, strused, stripstring);
   return str;
}


const char *
socketsettime2string(whichtime)
   const int whichtime;
{
   const char *function = "socketsettime2string()";

   switch (whichtime) {
      case SOCKETOPT_PRE:
         return "pre-establishment time";

      case SOCKETOPT_POST:
         return "post-establishment time";

      case SOCKETOPT_ANYTIME:
         return "any time";
   }

   if (whichtime == (SOCKETOPT_PRE | SOCKETOPT_POST))
      return "pre/post-establishment time";

   if (whichtime == (SOCKETOPT_PRE | SOCKETOPT_ANYTIME))
      return "pre-establishment or any time";

   if (whichtime == (SOCKETOPT_POST | SOCKETOPT_ANYTIME))
      return "post-establishment or any time";

   swarnx("%s: unknown value: %d", function, whichtime);
   return "<unknown value>";
}

#if !SOCKS_CLIENT

const char *
interfaceside2string(side)
   const interfaceside_t side;
{

   switch (side) {
      case INTERNALIF:
         return "internal/client";

      case EXTERNALIF:
         return "external/target";

      default:
         SERRX(side);
         /* NOTREACHED */
   }

}

char *
interfaceprotocol2string(ifproto, str, strsize)
   const interfaceprotocol_t *ifproto;
   char *str;
   size_t strsize;
{

   if (strsize == 0) {
      static char buf[1024];

      str     = buf;
      strsize = sizeof(buf);
   }

   snprintf(str, strsize, "%s: %s, %s: %s",
            safamily2string(AF_INET),  ifproto->ipv4 ? "yes" : "no",
            safamily2string(AF_INET6), ifproto->ipv6 ? "yes" : "no");


   return str;
}


char *
networktest2string(test, str, strsize)
   const networktest_t *test;
   char *str;
   size_t strsize;
{

   if (strsize == 0) {
      static char buf[1024];

      str     = buf;
      strsize = sizeof(buf);
   }

   snprintf(str, strsize, "mtu.dotest: %d", test->mtu.dotest);

   return str;
}

const char *
addrscope2string(scope)
   const ipv6_addrscope_t scope;
{
   switch (scope) {
      case addrscope_global:
         return "global";

      case addrscope_linklocal:
         return "linklocal";

      case addrscope_nodelocal:
         return "nodelocal";
   }

   SERRX(scope);
   /* NOTREACHED */
}

const char *
alarmside2string(alarmside)
   const size_t alarmside;
{

   switch (alarmside) {
      case ALARM_INTERNAL:
         return "internal";

      case ALARM_INTERNAL_RECV:
         return "internal.recv";

      case ALARM_INTERNAL_SEND:
         return "internal.send";

      case ALARM_EXTERNAL:
         return "external";

      case ALARM_EXTERNAL_RECV:
         return "external.recv";

      case ALARM_EXTERNAL_SEND:
         return "external.send";

      case ALARM_RECV:
         return "recv";

      case ALARM_SEND:
         return "send";

      default:
         SERRX(alarmside);
   }
}

const char *
clientinfo2string(cinfo, str, strsize)
   const clientinfo_t *cinfo;
   char *str;
   size_t strsize;
{
   const char *function = "clientinfo2string()";
   size_t strused;

   if (strsize == 0) {
      static char buf[1024];

      str     = buf;
      strsize = sizeof(buf);
   }

   strused = 0;

#if HAVE_SOCKS_HOSTID

{
   size_t i;

   for (i = 0; i < (size_t)cinfo->hostid.addrc; ++i) {
      char ntop[MAXSOCKADDRSTRING];

      if (inet_ntop(AF_INET, 
                    gethostidip(&cinfo->hostid, i),
                    ntop, 
                    sizeof(ntop)) == NULL) {
         slog(LOG_DEBUG, "%s: inet_ntop(3) failed on %s %x: %s",
              function,
              atype2string(SOCKS_ADDR_IPV4),
              gethostidip(&cinfo->hostid, i)->s_addr, 
              strerror(errno));

         snprintf(ntop, sizeof(ntop), "<unknown>");
         errno = 0;
      }

      strused += snprintf(&str[strused], strsize - strused, "[%s] ", ntop);
   }
}

#endif /* HAVE_SOCKS_HOSTID */

   strused += snprintf(&str[strused], strsize - strused, "%s",
                       sockaddr2string(&cinfo->from, NULL, 0));

   return str;
}

const char *
statekey2string(key)
   const statekey_t key;
{

   switch (key) {
      case key_unset:
         return "unset";

      case key_from:
         return "from (client IP-address)";

      case key_hostid:
         return "hostid (client hostid)";

      default:
         SERRX(key);
   }

   /* NOTREACHED */
}

statekey_t
string2statekey(string)
   const char *string;
{

   if (strcmp(string, "from") == 0)
      return key_from;
   else if (strcmp(string, "hostid") == 0)
      return key_hostid;

   return key_unset;
}

const char *
objecttype2string(type)
   const objecttype_t type;
{

   switch (type) {
      case object_sockaddr:
         return "sockaddr";

      case object_sockshost:
         return "sockshost";

      case object_crule:
         return "client-rule";

      case object_hrule:
         return "hostid-rule";

      case object_srule:
         return "socks-rule";

      case object_monitor:
         return "monitor";

      default:
         SERRX(type);
   }

   /* NOTREACHED */
   return NULL;
}


char *
options2string(options, prefix, str, strsize)
   const option_t *options;
   const char *prefix;
   char *str;
   size_t strsize;
{
   size_t strused;

   if (strsize == 0) {
      static char buf[1024];

      str = buf;
      strsize = sizeof(buf);
   }

   *str    = NUL;
   strused = 0;

   strused += snprintf(&str[strused], strsize - strused,
                       "\"%sdaemon\": \"%d\",\n",
                       prefix, options->daemon);

   strused += snprintf(&str[strused], strsize - strused,
                       "\"%sservercount\": \"%lu\",\n",
                       prefix, (unsigned long)options->serverc);

   strused += snprintf(&str[strused], strsize - strused,
                       "\"%sverifyonly\": \"%s\",\n",
                       prefix, options->verifyonly ? "yes" : "no");

   strused += snprintf(&str[strused], strsize - strused,
                       "\"%sdebug\": \"%d\",\n",
                       prefix, options->debug);

   strused += snprintf(&str[strused], strsize - strused,
                       "\"%sconfigfile\": \"%s\",\n",
                       prefix,
                       options->configfile == NULL ?
                          SOCKD_CONFIGFILE : options->configfile);

   strused += snprintf(&str[strused], strsize - strused,
                       "\"%sdebuglevel\": \"%d\",\n",
                       prefix, options->debug);

   strused += snprintf(&str[strused], strsize - strused,
                       "\"%skeepalive\": \"%d\",\n",
                       prefix, options->keepalive);

   strused += snprintf(&str[strused], strsize - strused,
                       "\"%spidfile\": \"%s\",\n",
                       prefix,
                       options->pidfile == NULL ?
                          SOCKD_PIDFILE : options->pidfile);

   STRIPTRAILING(str, strused, stripstring);
   return str;
}

char *
logs2string(logs, str, strsize)
   const log_t *logs;
   char *str;
   size_t strsize;
{
   size_t strused;

   if (strsize == 0) {
      static char buf[128];

      str = buf;
      strsize = sizeof(buf);
   }

   *str    = NUL;
   strused = 0;

   if (logs->connect)
      strused += snprintf(&str[strused], strsize - strused, "%s, ",
      QUOTE(SOCKS_LOG_CONNECTs));

   if (logs->disconnect)
      strused += snprintf(&str[strused], strsize - strused, "%s, ",
      QUOTE(SOCKS_LOG_DISCONNECTs));

   if (logs->data)
      strused += snprintf(&str[strused], strsize - strused, "%s, ",
      QUOTE(SOCKS_LOG_DATAs));

   if (logs->error)
      strused += snprintf(&str[strused], strsize - strused, "%s, ",
      QUOTE(SOCKS_LOG_ERRORs));

   if (logs->iooperation)
      strused += snprintf(&str[strused], strsize - strused, "%s, ",
      QUOTE(SOCKS_LOG_IOOPERATIONs));

   if (logs->tcpinfo)
      strused += snprintf(&str[strused], strsize - strused, "%s, ",
      QUOTE(SOCKS_LOG_TCPINFOs));

   STRIPTRAILING(str, strused, stripstring);
   return str;
}

const char *
childtype2string(type)
   int type;
{

   switch (type) {
      case PROC_MOTHER:
         return "mother";

      case PROC_MONITOR:
         return "monitor-child";

      case PROC_NEGOTIATE:
         return "negotiate-child";

      case PROC_REQUEST:
         return "request-child";

      case PROC_IO:
         return "io-child";

      default:
         SERRX(type);
   }

   /* NOTREACHED */
}

const char *
verdict2string(verdict)
   int verdict;
{

   switch (verdict) {
      case VERDICT_PASS:
         return QUOTE(VERDICT_PASSs);

      case VERDICT_BLOCK:
         return QUOTE(VERDICT_BLOCKs);
   }

   SERRX(verdict);
   /* NOTREACHED */
}

char *
list2string(list, str, strsize)
   const linkedname_t *list;
   char *str;
   size_t strsize;
{
   size_t strused;

   if (strsize)
      *str = NUL; /* make sure we return a NUL terminated string. */
   else
      return str;

   strused = 0;
   for (; list != NULL; list = list->next)
      strused += snprintf(&str[strused], strsize - strused, "\"%s\", ",
                          list->name);

   return str;
}

char *
compats2string(compats, str, strsize)
   const compat_t *compats;
   char *str;
   size_t strsize;
{
   size_t strused;

   if (strsize == 0) {
      static char buf[32];

      str = buf;
      strsize = sizeof(buf);
   }

   *str    = NUL;
   strused = 0;

   if (compats->sameport)
      strused += snprintf(&str[strused], strsize - strused, "%s, ",
      QUOTE("sameport"));

   STRIPTRAILING(str, strused, stripstring);
   return str;
}

char *
srchosts2string(srchost, prefix, str, strsize)
   const srchost_t *srchost;
   const char *prefix;
   char *str;
   size_t strsize;
{
   size_t strused;

   if (strsize == 0) {
      static char buf[32];

      str = buf;
      strsize = sizeof(buf);
   }

   *str    = NUL;
   strused = 0;

   if (srchost->nodnsmismatch)
      strused += snprintf(&str[strused], strsize - strused,
      "\"%snodnsmismatch\", ", prefix);

   if (srchost->nodnsunknown)
      strused += snprintf(&str[strused], strsize - strused,
      "\"%snodnsunknown\",", prefix);

   STRIPTRAILING(str, strused, stripstring);
   return str;
}

const char *
uid2name(uid)
   uid_t uid;
{
   struct passwd *pw;

   if ((pw = getpwuid(uid)) == NULL)
      return NULL;

   return pw->pw_name;
}

const char *
timeouttype2string(type)
   timeouttype_t type;
{

   switch (type) {
      case TIMEOUT_NEGOTIATE:
         return "negotiate timeout";

      case TIMEOUT_CONNECT:
         return "connect timeout";

      case TIMEOUT_IO:
         return "i/o timeout";

      case TIMEOUT_TCP_FIN_WAIT:
         return "tcp-fin-wait timeout";

      default:
         SERRX(type);
   }

   /* NOTREACHED */
}

char *
timeouts2string(timeouts, prefix, str, strsize)
   const timeout_t *timeouts;
   const char *prefix;
   char *str;
   size_t strsize;
{
   size_t strused;

   if (strsize == 0) {
      static char buf[64];

      str = buf;
      strsize = sizeof(buf);
   }

   *str    = NUL;
   strused = 0;

   strused += snprintf(&str[strused], strsize - strused,
                       "\"%sconnecttimeout\": \"%ld\",\n",
                       prefix, (unsigned long)timeouts->negotiate);

   strused += snprintf(&str[strused], strsize - strused,
                       "\"%siotimeout\": tcp: \"%lu\", udp: \"%lu\" \n",
                       prefix,
                       (unsigned long)timeouts->tcpio,
                       (unsigned long)timeouts->udpio);

   STRIPTRAILING(str, strused, stripstring);
   return str;
}

const char *
rotation2string(rotation)
   int rotation;
{

   switch (rotation) {
      case ROTATION_NONE:
         return "none";

      case ROTATION_SAMESAME:
         return "same-same";

      case ROTATION_ROUTE:
         return "route";

      default:
         SERRX(rotation);
   }

   /* NOTREACHED */
}

const char *
privop2string(op)
   const priv_op_t op;
{
   switch (op) {
      case PRIV_ON:
         return "on";

      case PRIV_OFF:
         return "off";
   }


   /* NOTREACHED */
   SERRX(op);
}



#if HAVE_SCHED_SETAFFINITY
char *
cpuset2string(set, str, strsize)
   const cpu_set_t *set;
   char *str;
   size_t strsize;
{
   const size_t setsize = cpu_get_setsize();
   size_t i, strused;

   if (strsize == 0) {
      static char buf[2048];

      str     = buf;
      strsize = sizeof(buf);
   }

   *str    = NUL;
   strused = 0;

   for (i = 0; i < setsize; ++i)
      if (cpu_isset(i, set))
         strused += snprintf(&str[strused], strsize - strused, "%ld ", (long)i);

   return str;
}
#endif /* HAVE_SCHED_SETAFFINITY */


#if !HAVE_PRIVILEGES
char *
userids2string(userids, prefix, str, strsize)
   const userid_t *userids;
   const char *prefix;
   char *str;
   size_t strsize;
{
   size_t strused;

   if (strsize == 0) {
      static char buf[128];

      str = buf;
      strsize = sizeof(buf);
   }

   *str    = NUL;
   strused = 0;

   strused += snprintf(&str[strused], strsize - strused,
                       "\"%sprivileged\": \"%s\",\n",
                       prefix, uid2name(userids->privileged_uid));

   strused += snprintf(&str[strused], strsize - strused,
                       "\"%sunprivileged\": \"%s\",\n",
                       prefix, uid2name(userids->unprivileged_uid));

   strused += snprintf(&str[strused], strsize - strused,
                       "\"%slibwrap\": \"%s\",\n",
                       prefix, uid2name(userids->libwrap_uid));

   STRIPTRAILING(str, strused, stripstring);
   return str;
}
#endif /* !HAVE_PRIVILEGES */

#if COVENANT
const char *
httpcode2string(version, code)
   const int version;
   const int code;
{
   static char prefix[16], buf[64];

   SASSERTX(version == PROXY_HTTP_10
   ||       version == PROXY_HTTP_11);

   snprintf(prefix, sizeof(prefix), "HTTP/1.%d %d",
   version == PROXY_HTTP_10 ? 0 : 1, code);

   switch (code) {
      case HTTP_SUCCESS:
         snprintf(buf, sizeof(buf), "%s Success", prefix);
         break;

      case HTTP_FORBIDDEN:
         snprintf(buf, sizeof(buf), "%s Not allowed", prefix);
         break;

      case HTTP_NOTALLOWED:
         snprintf(buf, sizeof(buf), "%s Not authorized", prefix);
         break;

      case HTTP_PROXYAUTHREQUIRED:
         snprintf(buf, sizeof(buf), "%s Not authorized", prefix);
         break;

      case HTTP_HOSTUNREACH:
         snprintf(buf, sizeof(buf), "%s Not reachable", prefix);
         break;

      case HTTP_FAILURE:
         snprintf(buf, sizeof(buf), "%s Unknown proxy server error", prefix);
         break;

      default:
         SERRX(code);
   }

   return buf;
}
#endif /* COVENANT */

#endif /* !SOCKS_CLIENT */
