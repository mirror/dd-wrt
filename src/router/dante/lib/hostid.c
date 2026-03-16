/*
 * Copyright (c) 2012, 2013, 2016, 2017, 2024
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

static const char rcsid[] =
"$Id: hostid.c,v 1.18.6.4.8.19 2024/12/05 12:04:05 michaels Exp $";

#if SOCKS_HOSTID_TYPE != SOCKS_HOSTID_TYPE_NONE

/*
 * Customer has during it's history used two versions of hostid.
 * The original version is SOCKS_HOSTID_TYPE_TCP_IPA, which is being
 * deprecated in 2024.  This version supported multiple hostids being
 * set on a connection.
 *
 * The current versions is SOCKS_HOSTID_TYPE_TCP_EXP1, supporting only
 * one hostid being set.  
 *
 * Semantics are that we first check if SOCKS_HOSTID_TYPE_TCP_EXP1 is 
 * set on the incoming connection, and use those values if present.
 * If not present, we check for the older SOCKS_HOSTID_TYPE_TCP_IPA
 * type, and if set, use those values.
 *
 * On the outgoing connection, when setting hostids, we only use the
 * newest hostid version.
 */

#if HAVE_TCP_EXP1

static size_t
len2exidlen(const int s, u_int8_t len);
/*
 * For some reason, the ExId field can vary in length.  The "len" field
 * of the hostid-value determines what length the following exid field 
 * will have.
 * 
 * "s", if not -1, is the socket connection the "len" value was received on, 
 * and is only used for error reporting purposes.
 *
 * Returns: the predefined length the ExId field will have if the "len" 
 *          field of the hostid has the value "len".
 *
 * If no predifined ExId-length for the given "len" is known, 0 is returned,
 * but it is considered an error if no exidlen is known for the given length, 
 * so a SWARNX() assert will also be triggered.  Possibly it indicates 
 * changes have been done to the protocol that we are not aware of.
 */

#endif /* HAVE_TCP_EXP1 */


unsigned char
getsockethostid(
   const int s,
   struct hostid *hostid)
{
   const char *function = "getsockethostid()";
   socklen_t len;
   ssize_t i, max;

   bzero(hostid, sizeof(*hostid));

#if HAVE_TCP_EXP1

   if (SOCKS_HOSTID_TYPE & SOCKS_HOSTID_TYPE_TCP_EXP1) {
      /*
       *| ------------------------------------
       *| Field               | Length (bits)
       *| ------------------------------------
       *| length              | 8
       *| exid (0x348)        | 16 or 32
       *| data (IP-address)   | 32
       *--------------------------------------
       * 
       * Giving a total length of 7 or 8 bytes, with the "length" field thus
       * also set to eiter 7 or 8.  The sequence may then repeat.  
       *
       * The size of the "length" and "data" fields is fixed, but the size 
       * of the exid field can now unfortunately vary.  The exid field will 
       * be either 16 bits or 32 bits, depending on the whether the value of
       * "length" is 7 or 9. :-/
       * 
       * The format of "data" can presumably vary depending on the exid, but
       * we only know of the exid format above, used for IP-addresses.
       */
      struct tcp_exp1_raw tcp_exp1_raw;

      len = sizeof(tcp_exp1_raw);

      if (getsockopt(s, IPPROTO_TCP, TCP_EXP1, &tcp_exp1_raw, &len) == -1
      ||  len == 0) {
         slog(LOG_DEBUG, "%s: no TCP_EXP1 hostid data retreived on fd %d (%s)",
              function, s, strerror(errno));

         errno = 0;
         len   = 0;
      }
      else {
         const unsigned char *p;
         size_t lenused;

         slog(LOG_DEBUG,
              "%s: TCP_EXP1 data of length %lu (max: %lu) retrieved on fd %d",
              function, 
              (unsigned long)len, 
              (unsigned long)sizeof(tcp_exp1_raw), 
              s);

#define PARSE_TCP_EXP1(field)                                                  \
do {                                                                           \
      slog(LOG_DEBUG, "%s: parsing field %s of size %lu at offset %lu/%lu",    \
           function,                                                           \
           #field,                                                             \
           (unsigned long)sizeof(hostid->addrv[i].tcp_exp1.field),             \
           (unsigned long)lenused,                                             \
           (unsigned long)len);                                                \
                                                                               \
      if (lenused + sizeof(hostid->addrv[i].tcp_exp1.field) > len) {           \
         swarnx("%s: truncated TCP_EXP1 data detected while about to "         \
                "parse the next field.  Already parsed %lu/%lu bytes "         \
                "of data from TCP_EXP1 header, but the next field, "           \
                "field \"%s\", is of length %lu.  That means we did not "      \
                "receive enough TCP_EXP1 data on this connection to parse "    \
                "all the required fields",                                     \
                function,                                                      \
                (unsigned long)lenused,                                        \
                (unsigned long)len,                                            \
                #field,                                                        \
                sizeof(hostid->addrv[i].tcp_exp1.field));                      \
                                                                               \
         return 0;                                                             \
      }                                                                        \
                                                                               \
      memcpy(&hostid->addrv[i].tcp_exp1.field,                                 \
             p,                                                                \
             sizeof(hostid->addrv[i].tcp_exp1.field));                         \
                                                                               \
      p       += sizeof(hostid->addrv[i].tcp_exp1.field);                      \
      lenused += sizeof(hostid->addrv[i].tcp_exp1.field);                      \
} while (0) 

         /*
          * Lay the raw tcp_exp1_raw data out in our formated tcp_exp1 
          * struct for easier use.
          */

         p         = (unsigned char *)tcp_exp1_raw.data;
         lenused   = 0;
         i         = 0;

         /*
          * Customer specifies there should only be one IP here at the moment, 
          * but it does not cost much too put this in a loop to possibly 
          * support more IPs in the future.
          */
         while (lenused < len) {
            ssize_t remaining;
            u_int32_t exid; /* longest exid-length possible, four octets. */

            if (i + 1 > (ssize_t)ELEMENTS(hostid->addrv)) {
               slog(LOG_NOTICE, 
                    "%s: client %s: have already parsed %u TCP_EXP1 options, "
                    "and still have %u bytes left to parse, but the max "
                    "number of TCP_EXP1 options we have been compiled to "
                    "support is %u, so will ignore remaining %u bytes of data",
                    function, 
                    peername2string(s, NULL, 0),
                    (unsigned)(i + 1), 
                    (unsigned)(len - lenused), 
                    (unsigned)ELEMENTS(hostid->addrv),
                    (unsigned)(len - lenused));

               break;
            }

            slog(LOG_DEBUG,
                 "%s: parsing TCP_EXP1 option data #%u at offset %u", 
                 function, (unsigned)i + 1, (unsigned)lenused);

            PARSE_TCP_EXP1(len);

            if (hostid->addrv[i].tcp_exp1.len 
            <   sizeof(hostid->addrv[i].tcp_exp1.len)) {
               slog(LOG_NOTICE, 
                    "%s: client %s: the value of the length field in the "
                    "received TCP_EXP1 data is %u, but logically this value "
                    "cannot be less than %u, so this is too strange.  "
                    "Aborting further parsing of the TCP_EXP1 data on this "
                    "connection",
                    function,
                    peername2string(s, NULL, 0),
                    (unsigned)hostid->addrv[i].tcp_exp1.len,
                    (unsigned)sizeof(hostid->addrv[i].tcp_exp1.len));

               break;
            }

            slog(LOG_DEBUG, 
                 "%s: len field has value %u, indicating the following ExID "
                 "field should be be %u bytes long",
                 function, 
                 (u_int8_t)hostid->addrv[i].tcp_exp1.len,
                 (unsigned)len2exidlen(s, hostid->addrv[i].tcp_exp1.len));

            remaining =   hostid->addrv[i].tcp_exp1.len 
                        - sizeof(hostid->addrv[i].tcp_exp1.len); 

            switch (len2exidlen(s, hostid->addrv[i].tcp_exp1.len)) {
               case 2:
                  PARSE_TCP_EXP1(exid.exid_16);
                  exid       = ntohs(hostid->addrv[i].tcp_exp1.exid.exid_16);
                  remaining -= sizeof(hostid->addrv[i].tcp_exp1.exid.exid_16);
                  break;

               case 4:
                  PARSE_TCP_EXP1(exid.exid_32);
                  exid       = ntohl(hostid->addrv[i].tcp_exp1.exid.exid_32);
                  remaining -= sizeof(hostid->addrv[i].tcp_exp1.exid.exid_32);
                  break;

               default:
                  slog(LOG_NOTICE, 
                       "%s: client %s: unsupported ExID length of %u "
                       "received at offset %u/%u.  Skipping remaining %d "
                       "byte%s of this field",
                       (unsigned)len2exidlen(s, hostid->addrv[i].tcp_exp1.len),
                       (unsigned)remaining,
                       (unsigned)lenused,
                       (unsigned)len,
                       function, 
                       peername2string(s, NULL, 0),
                       remaining == 1 ? "" : "s");

                  p       += remaining;
                  lenused += remaining;
                  continue;
            }

            slog(LOG_DEBUG, 
                 "%s: exid field has value 0x%x (network order: 0x%x)", 
                 function, 
                 exid, 
                 len2exidlen(s, hostid->addrv[i].tcp_exp1.len) == 2 ? 
                      (unsigned)htons((u_int16_t)exid) : (unsigned)htonl(exid));

            switch (exid) {
#if 0
#warning "XXX should only happen in internal testing.  Remove in release"
               case TCP_EXP1_EXID_INFERNO_FOURBYTE_TEST:
#endif


               case TCP_EXP1_EXID_IP:
                  if (remaining != sizeof(hostid->addrv[i].tcp_exp1.data.ip)) {
                     slog(LOG_NOTICE, 
                          "%s: client %s: expecting the IP field of a "
                          "TCP_EXP1 ExID (0x%x) to be %u octets long, but "
                          "the header set by this client claims it is %u "
                          "octets long.  Skipping this field",
                          function, 
                          peername2string(s, NULL, 0),
                          (unsigned)exid,
                          (unsigned)sizeof(hostid->addrv[i].tcp_exp1.data.ip),
                          (unsigned)remaining);

                     p       += remaining;
                     lenused += remaining;
                     continue;
                  }

                  PARSE_TCP_EXP1(data.ip);           

                  slog(LOG_DEBUG, "%s: data.ip field has value 0x%x (%s)", 
                       function, 
                       (u_int32_t)hostid->addrv[i].tcp_exp1.data.ip,
                       inet_ntoa(*(struct in_addr *)
                                         &(hostid->addrv[i].tcp_exp1.data.ip)));

                  break;

               default: {
                  slog(LOG_NOTICE,
                       "%s: client %s: unknown TCP_EXP1 exid value: 0x%x.  "
                       "Values currently supported are: 0x%x and 0x%x.  "
                       "Skipping remaining %d byte%s of this field",
                       function, 
                       peername2string(s, NULL, 0),
                       (unsigned)exid,
                       TCP_EXP1_EXID_IP,
                       TCP_EXP1_EXID_INFERNO_FOURBYTE_TEST,
                       (unsigned)remaining,
                       remaining == 1 ? "" : "s");

                  p       += remaining;
                  lenused += remaining;
                  continue;
               }
            }

            ++i;
         }

         slog(LOG_DEBUG, "%s: %u TCP_EXP1 options parsed", 
              function, (unsigned)i);

         hostid->hostidtype = SOCKS_HOSTID_TYPE_TCP_EXP1;
         hostid->addrc      = i;

         return (unsigned char)i;
      }
   }

#else /* !HAVE_TCP_EXP1 */

   len = 0;

#endif /* HAVE_TCP_EXP1 */

#if HAVE_TCP_IPA

   if (len == 0 && (SOCKS_HOSTID_TYPE & SOCKS_HOSTID_TYPE_TCP_IPA)) {
      struct tcp_ipa_raw tcp_ipa_raw;

      len = sizeof(tcp_ipa_raw);

      if (getsockopt(s, IPPROTO_TCP, TCP_IPA, &tcp_ipa_raw, &len) == -1 
      ||  len == 0) {
         slog(LOG_DEBUG, "%s: no TCP_IPA hostid data retreived on fd %d (%s)",
              function, s, strerror(errno));

         errno = 0;
         len   = 0;
      }
      else {
         slog(LOG_DEBUG,
              "%s: TCP_IPA data of length %u (max: %u) retrieved on fd %d",
              function, 
              (unsigned)len, 
              (unsigned)sizeof(tcp_ipa_raw), 
              s);

         if (len != 0) {
            if (len < sizeof((*hostid->addrv).tcp_ipa.ip)) {
               swarnx("%s: TCP_IPA data of length %u received on connection "
                      "from client %s, but minimum expected is %u",
                      function, 
                      (unsigned)len, 
                      socket2string(s, NULL, 0),
                      (unsigned)sizeof((*hostid->addrv).tcp_ipa.ip));

               return 0;
            }

            /*
             * In the current (not original, though anyway marked as 
             * "IPA_VERSION 1") version of the API, it's no longer the 
             * length of data returned by getsockopt(2) that determines 
             * how many hostid values are set.  Instead the structure can 
             * be considered as an array of fixed length, where indices in 
             * the array having a value other than zero are to be considered 
             * set.  *However*: an indice having the value zero is also
             * considered set if there is a non-zero values in any of the 
             * following indices.
             */

#if DEBUG /* XXX use DEBUGLEVEL_ENABLED() when merging to Dante-current. */

            if (sockscf.option.debug) {
               char ntop[MAXSOCKADDRSTRING];

               max = len / sizeof(*tcp_ipa_raw.ipa_ipaddress);

               for (i = 0; i < max; ++i) {
                  if (inet_ntop(AF_INET, 
                                &tcp_ipa_raw.ipa_ipaddress[i], 
                                ntop, 
                                sizeof(ntop)) == NULL)
                     swarn("%s: inet_ntop(3) failed on %s hostid %x at "
                           "index #%u",
                           function, 
                           safamily2string(AF_INET), 
                           tcp_ipa_raw.ipa_ipaddress[i], 
                           (unsigned)i);
                  else
                     slog(LOG_DEBUG, "%s: hostid at index #%u: %s",
                          function, (unsigned)i, ntop);
               }
            }

#endif /* DEBUG */

            max = MIN(ELEMENTS(hostid->addrv), 
                      len / sizeof(*tcp_ipa_raw.ipa_ipaddress));

            for (i = max - 1; i >= 0; --i) {
               if (tcp_ipa_raw.ipa_ipaddress[i] != htonl(0)) {
                  slog(LOG_DEBUG, "%s: found hostid set at index #%u", 
                       function, (unsigned)i);

                  break;
               }
            }

            ++i; /* 'i' was last index set, so there are "i + 1" hostids set. */

            SASSERTX(i <= UCHAR_MAX);
            SASSERTX(i <= (ssize_t)ELEMENTS(hostid->addrv));

            if (i > 0) {
               for (hostid->addrc = 0; 
                    hostid->addrc < (size_t)i; 
                    ++hostid->addrc)
                  hostid->addrv[hostid->addrc].tcp_ipa.ip 
                  = tcp_ipa_raw.ipa_ipaddress[hostid->addrc];
            }

            slog(LOG_DEBUG, "%s: %u TCP_IPA hostids set on connection", 
                 function, (unsigned)i);
         }

         hostid->hostidtype = SOCKS_HOSTID_TYPE_TCP_IPA;

         return (unsigned char)hostid->addrc;
      }
   }

#endif /* HAVE_TCP_IPA */

   return 0;
}

int
setsockethostid(
   const int s,
   const struct hostid *hostid)
{
   const char *function = "setsockethostid()";
   size_t len, i;

   slog(LOG_DEBUG, "%s: hostidtype: %d, addrc: %u%s",
        function, 
        hostid->hostidtype, 
        (unsigned)hostid->addrc,
        hostid->addrc == 0 ? "( nothing to set)" : "");

   if (hostid->addrc == 0)
      return 0;

#if DEBUG /* XXX use DEBUGLEVEL_ENABLED() when merging to Dante-current. */

   for (i = 0; i < hostid->addrc; ++i) {
      const struct in_addr *addr;
      char ntop[MAXSOCKADDRSTRING];

      switch (hostid->hostidtype) {

#if HAVE_TCP_IPA

         case SOCKS_HOSTID_TYPE_TCP_IPA: 
            addr = (const struct in_addr *)&hostid->addrv[i].tcp_ipa.ip;
            break;

#endif /* HAVE_TCP_IPA */

#if HAVE_TCP_EXP1

         case SOCKS_HOSTID_TYPE_TCP_EXP1:
            addr = (const struct in_addr *)&hostid->addrv[i].tcp_exp1.data.ip;
            break;

#endif /* HAVE_TCP_EXP1 */

         default:
            SERRX(hostid->hostidtype);
      }

      if (inet_ntop(AF_INET, addr, ntop, sizeof(ntop)) == NULL)
         swarn("%s: inet_ntop(3) failed on %s %x",
               function, safamily2string(AF_INET), *(const unsigned *)addr);
      else
         slog(LOG_DEBUG, "%s: hostid at index #%lu: %s",
              function, (unsigned long)i, ntop);
   }

#endif /* DEBUG  */

   switch (hostid->hostidtype) {

#if HAVE_TCP_IPA

      case SOCKS_HOSTID_TYPE_TCP_IPA: {
         struct tcp_ipa_raw tcp_ipa_raw;

         /*
          * Unset hostid fields must be set to zero.
          */
         bzero(&tcp_ipa_raw, sizeof(tcp_ipa_raw));

         for (i = 0; i < hostid->addrc; ++i)
            tcp_ipa_raw.ipa_ipaddress[i] = hostid->addrv[i].tcp_ipa.ip;

         len = i * sizeof(*tcp_ipa_raw.ipa_ipaddress);

         if (setsockopt(s, IPPROTO_TCP, TCP_IPA, &tcp_ipa_raw, (socklen_t)len) 
         != 0) {
            swarn("%s: could not set %u TCP_IPA hostids (total length: %u) "
                  "on fd %d (%s)",
                  function, (unsigned)i, (unsigned)len, s, strerror(errno));

            return -1;
         }

         return 0;
      }

#endif /* HAVE_TCP_IPA */


#if HAVE_TCP_EXP1

      case SOCKS_HOSTID_TYPE_TCP_EXP1: {
         struct tcp_exp1_raw tcp_exp1_raw;
         ubits_8 *p;

#define HOSTID2MEM(rc, index, fieldname)                                       \
do {                                                                           \
   const size_t _len = sizeof(hostid->addrv[(index)].tcp_exp1.fieldname);      \
                                                                               \
   slog(LOG_DEBUG,                                                             \
        "%s: copying fieldname \"%s\", length %u bytes, network order "        \
        "value %lu (0x%lx) "                                                   \
        "for hostid array index #%u ...",                                      \
        function,                                                              \
        #fieldname,                                                            \
        (unsigned)_len,                                                        \
        (unsigned long)hostid->addrv[(index)].tcp_exp1.fieldname,              \
        (unsigned long)hostid->addrv[(index)].tcp_exp1.fieldname,              \
        (unsigned)i);                                                          \
                                                                               \
   if ((p - tcp_exp1_raw.data) + (_len) > sizeof(tcp_exp1_raw.data)) {         \
      swarnx("%s: %u TCP_EXP1 hostids to set, but ran out of space while "     \
             "trying to set the \"%s\" field of hostid #%u (%u bytes "         \
             "long).  Max amount of space for all hostids per connection was " \
             "at compiletime defined to %u bytes.  Remaining #%u hostids "     \
             "will be discarded and not set on outgoing connection",           \
              function,                                                        \
              (unsigned)hostid->addrc,                                         \
              #fieldname,                                                      \
              (unsigned)(index) + 1,                                           \
              (unsigned)_len,                                                  \
              (unsigned)sizeof(tcp_exp1_raw.data),                             \
              (unsigned)(hostid->addrc - (index)));                            \
                                                                               \
      *(rc) = -1;                                                              \
      break;                                                                   \
   }                                                                           \
                                                                               \
   memcpy(p, &hostid->addrv[(index)].tcp_exp1.fieldname, (_len));              \
   p      += _len;                                                             \
                                                                               \
   *(rc)  = 0;                                                                 \
} while (0)

         p = tcp_exp1_raw.data;

         for (i = 0; i < hostid->addrc; ++i) {
            int rc;

            HOSTID2MEM(&rc, i, len);

            if (rc != 0)
               break;

            /* exidlen can vary for for some reason.  Len determines. :-/ */
            switch (hostid->addrv[i].tcp_exp1.len) {
               case 7:
                  HOSTID2MEM(&rc, i, exid.exid_16);
                  break;

               case 9:
                  HOSTID2MEM(&rc, i, exid.exid_32);
                  break;

               default:
                  SERRX(hostid->addrv[i].tcp_exp1.len);
            }

            if (rc != 0)
               break;

            HOSTID2MEM(&rc, i, data.ip);

            if (rc != 0)
               break;
         }

         len = p - tcp_exp1_raw.data;

#if 0
#warning "XXX remove"

         if (len == 14)
            slog(LOG_DEBUG, 
                 "%s: will setsockopt(2) TCP_EXP1 with string of length %u.  "
                 "Bytes: "
                 "0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, "
                 "0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
                 function, 
                 (unsigned)len, 
                 ((char *)&tcp_exp1_raw)[0],
                 ((char *)&tcp_exp1_raw)[1],
                 ((char *)&tcp_exp1_raw)[2],
                 ((char *)&tcp_exp1_raw)[3],
                 ((char *)&tcp_exp1_raw)[4],
                 ((char *)&tcp_exp1_raw)[5],
                 ((char *)&tcp_exp1_raw)[6],
                 ((char *)&tcp_exp1_raw)[7],
                 ((char *)&tcp_exp1_raw)[8],
                 ((char *)&tcp_exp1_raw)[9],
                 ((char *)&tcp_exp1_raw)[10],
                 ((char *)&tcp_exp1_raw)[11],
                 ((char *)&tcp_exp1_raw)[12],
                 ((char *)&tcp_exp1_raw)[13]);
         else if (len == 7)
            slog(LOG_DEBUG, 
                 "%s: will setsockopt(2) TCP_EXP1 with string of length %u.  "
                 "Bytes: "
                 "0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
                 function, 
                 (unsigned)len, 
                 ((char *)&tcp_exp1_raw)[0],
                 ((char *)&tcp_exp1_raw)[1],
                 ((char *)&tcp_exp1_raw)[2],
                 ((char *)&tcp_exp1_raw)[3],
                 ((char *)&tcp_exp1_raw)[4],
                 ((char *)&tcp_exp1_raw)[5],
                 ((char *)&tcp_exp1_raw)[6]);
#endif 

         if (setsockopt(s, IPPROTO_TCP, TCP_EXP1, &tcp_exp1_raw, (socklen_t)len)
         == 0)
            slog(LOG_DEBUG, 
                 "%s: set %u bytes of TCP_EXP1 data for %u/%u hostid(s)", 
                 function, 
                 (unsigned)len, 
                 (unsigned)i, 
                 (unsigned)hostid->addrc);
         else {
            swarn("%s: could not set %u TCP_EXP1 hostids, in %u bytes of data, "
                  "on fd %d",
                  function, (unsigned)i, (unsigned)len, s);

            return -1;
         }

         return 0;
      }

#endif /* HAVE_TCP_EXP1 */

   }

   SERRX(hostid->hostidtype);
   /* NOTREACHED */
}

const struct in_addr *
gethostidip(
   const struct hostid *hostid,
   const size_t index)
{
   const char *function = "gethostidip()";
   const char s[]       = "<invalid IPv4 address>";
   struct in_addr *addr = NULL;
   char ntop[MAX(MAXSOCKADDRSTRING, sizeof(s))];

   SASSERTX(hostid->addrc <= ELEMENTS(hostid->addrv));
   SASSERTX(index < hostid->addrc);

   switch (hostid->hostidtype) {
      case SOCKS_HOSTID_TYPE_TCP_IPA:
          addr = (const struct in_addr *)&hostid->addrv[index].tcp_ipa.ip;
          break;

#if HAVE_TCP_EXP1

      case SOCKS_HOSTID_TYPE_TCP_EXP1:
          addr = (const struct in_addr *)&hostid->addrv[index].tcp_exp1.data.ip;
          break;

#endif /* HAVE_TCP_EXP1 */

   }

   if (addr == NULL)
      SERRX(hostid->hostidtype);

   /* XXX use DEBUGLEVEL_ENABLED() when merging to Dante-current. */

   if (sockscf.option.debug) {
      if (inet_ntop(AF_INET, addr, ntop, sizeof(ntop)) == NULL)
         strcpy(ntop, s);

      slog(LOG_DEBUG, "%s: hostid at index #%u is %s (0x%x)",
           function, (unsigned)index, ntop, addr->s_addr);
   }

   return addr;
}

size_t
gethostidipv(
   const struct hostid *hostid,
   struct in_addr *addrv,
   size_t addrc)
{

   for (addrc = 0; addrc < hostid->addrc; ++addrc)
      addrv[addrc] = *gethostidip(hostid, addrc);

   return addrc;
}



#if HAVE_TCP_EXP1

static size_t
len2exidlen(const int s, u_int8_t len);
/*
 * For some reason, the ExId field can vary in length.  The "len" field
 * of the hostid-value determines what length the following exid field 
 * will have.
 *
 * Returns: the predefined length the ExId field will have if the "len" 
 *          field of the hostid has the value "len".
 *
 * If no predifined ExId-length for the given "len" is known, 0 is returned,
 * but it is considered an error if no exidlen is known for the given
 * length, and SWARNX() assert will be triggered.
 * on the assumption that there should always be an ExId field.
 */



u_int8_t
tcp_exp1_len(
   const int exidlen)
{
   struct tcp_exp1 size;

   return (u_int8_t)(  sizeof(size.len) 
                     + (exidlen == 2 ? sizeof(size.exid.exid_16) 
                                     : sizeof(size.exid.exid_32))
                     + sizeof(size.data.ip));
}

static size_t
len2exidlen(
   const int s,
   u_int8_t len)
{
   const char *function = "len2exidlen()";

   switch (len) {
      case 7:
         return 2;

      case 9:
         return 4;
   }

   slog(LOG_NOTICE, 
        "%s: unknown hostid-length %d received from client %s.  "
        "No exid-length known for this hostid-length", 
        function, len, peername2string(s, NULL, 0));

   return 0;
}


#endif /* HAVE_TCP_EXP1 */

#endif /* SOCKS_HOSTID_TYPE != SOCKS_HOSTID_TYPE_NONE */
