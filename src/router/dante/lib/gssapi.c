/*
 * Copyright (c) 2009, 2010, 2011, 2012, 2013, 2014, 2020
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

 /*
  * This code was originally contributed by
  * Markus Moeller (markus_moeller at compuserve.com) a long time ago.
  * Since then we have added many bugs to this code for which Markus
  * cannot be blamed.
  */


#include "common.h"

static const char rcsid[] =
"$Id: gssapi.c,v 1.171.4.14.6.5 2020/11/11 17:02:25 karls Exp $";

/*
 *
 * All the calls to DNSCODE_{START,END}() here are to work around bugs in
 * socks_markas{native,normal}(), which was not working correctly in
 * 1.4.x for some cases.  Since those functions have been considerably
 * improved and changed in current/1.5.x, it has been deemed less risky
 * to change the calls to socks_markas{native,normal}() to
 * DNSCODE_{START,END}() for this release, rather than trying to merge
 * in changes from 1.5.x.
 * Fixes the test "-a gssapi td/server_iochurn/" which otherwise failed due
 * to memory being realloc(3)-ed under our feet by socks_addaddr() being
 * unexpectedly called after we have called socks_markasnative() to
 * indicate no interpositioning should be done.
 */
#define DNSCODE_START()                                                        \
do {                                                                           \
      slog(LOG_DEBUG, "DNSCODE_START: %d",                                     \
           (int)++sockscf.state.executingdnscode);                             \
} while (/* CONSTCOND */ 0)

#define DNSCODE_END()                                                          \
do {                                                                           \
      slog(LOG_DEBUG, "DNSCODE_END: %d",                                       \
           (int)--sockscf.state.executingdnscode);                             \
} while (/* CONSTCOND */ 0)


#if HAVE_GSSAPI

#if SOCKS_CLIENT

static void
drainsocket(iobuffer_t *iobuf, const int drainitall,
            void *buf, const size_t bufsize);
/*
 * Drains the socket belonging to iobuf for the appropriate number
 * of bytes, so that the kernel will either continue to mark the
 * socket as readable or not.
 *
 * If "drainitall" is true, the function will drain all previously peeked
 * at bytes.  If "drainitall" is false, it will leave at least one byte
 * in the socket buffer.
 *
 * "buf" is a buffer which can be used to temporarily hold the drained data,
 * and must be at least as large as iobuf->readalready.
 */

#endif /* SOCKS_CLIENT */

static ssize_t
gssapi_decode_read_udp(int s, void *buf, size_t len, int flags,
                       struct sockaddr_storage *from, socklen_t *fromlen,
                       recvfrom_info_t *recvflags, gssapi_state_t *gs,
                       unsigned char *token, const size_t tokensize);
/*
 * Reads and decodes a udp packet.  Similar to gssapi_decode_read()
 * and socks_recvfrom(), but takes two additional arguments:
 * token       - tmpbuffer to hold the gssapi token being worked on.
 * tokensize   - size of "token".  Should be big enough to hold the largest
 *               possible GSSAPI token, including the header.
 */

static ssize_t
gssapi_encode_write_udp(int s, const void *msg, size_t len, int flags,
                        const struct sockaddr_storage *to, socklen_t tolen,
                        sendto_info_t *sendtoflags, gssapi_state_t *gs,
                        unsigned char *token, const size_t tokensize);
/*
 * Similar to gssapi_encode_read_udp().
 */

static int
gssapi_headerisok(const unsigned char *headerbuf, const size_t len,
                  unsigned short *tokenlen,
                  char *emsg, size_t emsglen);
/*
 * Checks if the first few bytes of "headerbuf" match a valid socks gssapi
 * header.  If so, "tokenlen" is set to the length of the token.
 *
 * Returns true if it looks valid, false otherwise.
 */

int
gss_err_isset(major_status, minor_status, buf, buflen)
   OM_uint32 major_status;
   OM_uint32 minor_status;
   char *buf;
   size_t buflen;
{
   OM_uint32 maj_stat, min_stat, msg_ctx;
   gss_buffer_desc statstr;
#if SOCKS_CLIENT
   sigset_t oldset;
#endif /* SOCKS_CLIENT */
   size_t w;

   if (GSS_ERROR(major_status) == 0)
      return 0;

   if (buflen > 0)
      *buf = NUL;

   msg_ctx = 0;
   do {
      /*
       * convert major status code (GSSAPI error) to text.
       * Keep fetching errorstrings as long as gss_display_status()
       * does not fail.
       */

      SOCKS_SIGBLOCK_IF_CLIENT(SIGIO, &oldset);

      maj_stat = gss_display_status(&min_stat,
                                    major_status,
                                    GSS_C_GSS_CODE,
                                    GSS_C_NULL_OID,
                                    &msg_ctx,
                                    &statstr);

      SOCKS_SIGUNBLOCK_IF_CLIENT(&oldset);

      if (buflen > 0 && GSS_ERROR(maj_stat) != 0) {
         w = snprintf(buf, buflen,
                      "%.*s.  ", (int)statstr.length, (char *)statstr.value);
         buf    += w;
         buflen -= w;
      }

      SOCKS_SIGBLOCK_IF_CLIENT(SIGIO, &oldset);

      gss_release_buffer(&min_stat, &statstr);

      SOCKS_SIGUNBLOCK_IF_CLIENT(&oldset);

   } while (msg_ctx != 0 && GSS_ERROR(maj_stat) != 0);

   msg_ctx = 0;
   do {
      /*
       * convert minor status code (underlying routine error) to text.
       * Keep fetching errorstrings as long as gss_display_status()
       * does not fail.
       */

      SOCKS_SIGBLOCK_IF_CLIENT(SIGIO, &oldset);

      maj_stat = gss_display_status(&min_stat,
                                    minor_status,
                                    GSS_C_MECH_CODE,
                                    GSS_C_NULL_OID,
                                    &msg_ctx,
                                    &statstr);

      SOCKS_SIGUNBLOCK_IF_CLIENT(&oldset);

      if (buflen > 0 && GSS_ERROR(maj_stat) != 0) {
         w = snprintf(buf, buflen,
                      "%.*s.  ", (int)statstr.length, (char *)statstr.value);
         buf    += w;
         buflen -= w;
      }

      SOCKS_SIGBLOCK_IF_CLIENT(SIGIO, &oldset);

      gss_release_buffer(&min_stat, &statstr);

      SOCKS_SIGUNBLOCK_IF_CLIENT(&oldset);

   } while (msg_ctx != 0 && GSS_ERROR(maj_stat) != 0);

   if (ERRNOISTMP(errno))
      errno = ENETDOWN; /* at least indicate some error. */

   return 1;
}

int
gssapi_encode(input_token, gs, output_token)
   const gss_buffer_t input_token;
   gssapi_state_t *gs;
   gss_buffer_t output_token;
{
   const char *function = "gssapi_encode()";
   gss_buffer_desc encoded_token;
   OM_uint32 minor_status, major_status;
#if SOCKS_CLIENT
   sigset_t oldset;
#endif /* SOCKS_CLIENT */
   char emsg[1024];
   int conf_state;

   slog(LOG_DEBUG, "%s, input length %lu, max output length %lu",
        function,
        (long unsigned)input_token->length,
        (long unsigned)output_token->length);

#if SOCKS_CLIENT && SOCKSLIBRARY_DYNAMIC
   DNSCODE_START();
#endif /* SOCKS_CLIENT && SOCKSLIBRARY_DYNAMIC */

   SOCKS_SIGBLOCK_IF_CLIENT(SIGIO, &oldset);

   major_status = gss_wrap(&minor_status,
                           gs->id,
                           gs->protection == GSSAPI_CONFIDENTIALITY ?
                                 GSS_REQ_CONF : GSS_REQ_INT,
                           GSS_C_QOP_DEFAULT,
                           input_token,
                           &conf_state,
                           &encoded_token);

   SOCKS_SIGUNBLOCK_IF_CLIENT(&oldset);

#if SOCKS_CLIENT && SOCKSLIBRARY_DYNAMIC
   DNSCODE_END();
#endif /* SOCKS_CLIENT && SOCKSLIBRARY_DYNAMIC */

   if (gss_err_isset(major_status, minor_status, emsg, sizeof(emsg))) {
      swarnx("%s: gss_wrap(): %s", function, emsg);
      return -1;
   }

   if (encoded_token.length > input_token->length) {
      if (((encoded_token.length + GSSAPI_HLEN) - input_token->length)
      > gs->gssoverhead) {
        slog(LOG_DEBUG,
             "%s: max expected GSSAPI overhead increased from %lu to %lu",
             function,
             (unsigned long)gs->gssoverhead,
             (unsigned long)((encoded_token.length + GSSAPI_HLEN)
                             - input_token->length));

        gs->gssoverhead = (encoded_token.length + GSSAPI_HLEN)
                          - input_token->length;
      }
   }

   if (encoded_token.length > output_token->length) {
      slog(LOG_NOTICE,
           "%s: encoded token length (%lu) larger than buffer (%lu)",
           function,
           (long unsigned)encoded_token.length,
           (long unsigned)output_token->length);

      CLEAN_GSS_TOKEN(encoded_token);

      errno = EMSGSIZE; /* caller will have to retry with less data. */
      return -1;
   }

   output_token->length = encoded_token.length;
   memcpy(output_token->value, encoded_token.value, encoded_token.length);

   CLEAN_GSS_TOKEN(encoded_token);

   if (output_token->length >= 4)
      slog(LOG_DEBUG,
           "%s: gssapi packet encoded.  Decoded/encoded length %lu/%lu.  "
           "First encoded bytes: "
           "[%d]: 0x%x, [%d]: 0x%x [%d]: 0x%x, [%d]: 0x%x "
           "Last: "
           "[%d]: 0x%x, [%d]: 0x%x [%d]: 0x%x, [%d]: 0x%x",
           function,
           (unsigned long)input_token->length,
           (unsigned long)output_token->length,
           0,
           ((unsigned char *)(output_token->value))[0],
           1,
           ((unsigned char *)(output_token->value))[1],
           2,
           ((unsigned char *)(output_token->value))[2],
           3,
           ((unsigned char *)(output_token->value))[3],
           (int)(output_token->length - 4),
           ((unsigned char *)(output_token->value))[output_token->length - 4],
           (int)(output_token->length - 3),
           ((unsigned char *)(output_token->value))[output_token->length - 3],
           (int)(output_token->length - 2),
           ((unsigned char *)(output_token->value))[output_token->length - 2],
           (int)(output_token->length - 1),
           ((unsigned char *)(output_token->value))[output_token->length - 1]);

   return 0;
}

int
gssapi_decode(input_token, gs, output_token)
   const gss_buffer_t input_token;
   gssapi_state_t *gs;
   gss_buffer_t output_token;
{
   const char *function = "gssapi_decode()";
   gss_buffer_desc decoded_token;
   OM_uint32  minor_status, major_status;
#if SOCKS_CLIENT
   sigset_t oldset;
#endif /* SOCKS_CLIENT */
   char emsg[1024];
   int req_conf_state;

   slog(LOG_DEBUG, "%s, input length %lu, max output length %lu",
        function,
        (long unsigned)input_token->length,
        (long unsigned)output_token->length);

   if (gs->protection == GSSAPI_CONFIDENTIALITY)
      req_conf_state = GSS_REQ_CONF;
   else
      req_conf_state = GSS_REQ_INT;

   SOCKS_SIGBLOCK_IF_CLIENT(SIGIO, &oldset);

   major_status = gss_unwrap(&minor_status,
                             gs->id,
                             input_token,
                             &decoded_token,
                             &req_conf_state,
                             GSS_C_QOP_DEFAULT);

   SOCKS_SIGUNBLOCK_IF_CLIENT(&oldset);


   if (gss_err_isset(major_status, minor_status, emsg, sizeof(emsg))) {
      slog(GSSERR_IS_OK(major_status) ? LOG_DEBUG
                                        : SOCKS_CLIENT ?
                                                       LOG_WARNING : LOG_NOTICE,
           "%s: failed to decode GSSAPI-encapsulated token.  gss_unwrap() "
           "failed on token of length %lu: %s",
           function,
           (unsigned long)input_token->length,
           emsg);

#if DIAGNOSTIC
      if (!GSSERR_IS_OK(major_status))
         SWARNX(0);
#endif /* DIAGNOSTIC */

      errno = 0; /* make sure caller does not reuse some old leftover value. */
      return -1;
   }

   if (decoded_token.length > output_token->length) {
      swarnx("%s: output buffer too small.  Need %lu bytes, but have only %lu",
             function,
             (unsigned long)decoded_token.length,
             (unsigned long)output_token->length);

      CLEAN_GSS_TOKEN(decoded_token);

      errno = ENOMEM;
      return -1;
   }

   output_token->length = decoded_token.length;
   memcpy(output_token->value, decoded_token.value, decoded_token.length);

   CLEAN_GSS_TOKEN(decoded_token);

   slog(LOG_DEBUG, "%s: gssapi packet decoded.  Decoded/encoded length %lu/%lu",
        function,
        (unsigned long)output_token->length,
        (unsigned long)input_token->length);

   return 0;
}

/*
 * RFC1961: client request / server response
 *
 *   +------+------+------+.......................+
 *   + ver  | mtyp | len  |       token           |
 *   +------+------+------+.......................+
 *   + 0x01 | 0x03 | 0x02 | up to 2^16 - 1 octets |
 *   +------+------+------+.......................+
 *
 */
ssize_t
gssapi_decode_read(s, buf, len, flags, from, fromlen, recvflags, gs)
   int s;
   void *buf;
   size_t len;
   int flags;
   struct sockaddr_storage *from;
   socklen_t *fromlen;
   recvfrom_info_t *recvflags;
   gssapi_state_t *gs;
{
   const char *function = "gssapi_decode_read()";
   gss_buffer_desc input_token, output_token;
   iobuffer_t *iobuf = NULL;
   unsigned short encodedlen;
   unsigned char inputmem[sizeof(iobuf->buf[0])],
                 outputmem[sizeof(iobuf->buf[0])];
   ssize_t nread, encoded_left_to_read;
   size_t tokennumber, p, readfrombuf, mintoread;
   char emsg[512];

   if (iobuf == NULL && (iobuf = socks_getbuffer(s)) == NULL) {
      int stype;

      if (recvflags != NULL)
         stype = recvflags->type;
      else {
         socklen_t tlen = sizeof(stype);

         if (getsockopt(s, SOL_SOCKET, SO_TYPE, &stype, &tlen) != 0) {
            swarn("%s: getsockopt(SO_TYPE)", function);
            return -1;
         }
      }

      if ((iobuf = socks_allocbuffer(s, stype)) == NULL) {
         swarnx("%s: could not allocate iobuffer", function);

         errno = ENOMEM;
         return -1;
      }
   }

   if (iobuf->stype == SOCK_DGRAM)
      return gssapi_decode_read_udp(s,
                                    buf,
                                    len,
                                    flags,
                                    from,
                                    fromlen,
                                    recvflags,
                                    gs,
                                    outputmem,
                                    sizeof(outputmem));

   if (recvflags != NULL)
      recvflags->fromsocket = 0;

#if SOCKS_CLIENT

   /*
    * always flush before read.
    */
   if (socks_flushbuffer(s, -1, NULL) == -1)
      return -1;

#else

   /* not supported in the server, and not used at the moment either. */
   SASSERTX(!(flags & MSG_WAITALL));

#endif /* SOCKS_CLIENT */

   if (flags & MSG_WAITALL)
      mintoread = len;
   else if (fdisblocking(s))
      mintoread = 1;
   else
      mintoread = 0;

   /*
    * If this read operation is blocking we may need to retry the read.
    * The token buffers we allocate for this are however too large to
    * simply call ourselves recursively again, so we use goto instead.
    */
again:

   slog(LOG_DEBUG,
        "%s: fd %d, len %lu, flags %d, mintoread %lu, buffered: %lu + %lu",
        function,
        s,
        (long unsigned)len,
        flags,
        (unsigned long)mintoread,
        (unsigned long)socks_bytesinbuffer(s, READ_BUF, 0),
        (unsigned long)socks_bytesinbuffer(s, READ_BUF, 1));

   if (socks_bytesinbuffer(s, READ_BUF, 0) >= mintoread) {
      /*
       * Have enough decoded data buffered and available for read.
       * Return as much as caller wants, but do not try to read any
       * new data now; we do not know if we were called because there
       * was data to read from our buffer or from the socket.
       */
      readfrombuf = socks_getfrombuffer(s, flags, READ_BUF, 0, buf, len);
   }
   else
      readfrombuf = 0;

#if SOCKS_CLIENT
   /*
    * When called by the client, we have the considerably added complexity
    * that we cannot completely drain the socket if we have data buffered
    * for read.  If the client then select(2)'s on the socket to know when
    * there is more to read, select(2) will block forever as the data has
    * already been read and buffered by us, but select(2)/etc. of course
    * does not know anything about that.
    *
    * What we do instead is to only peek at the last byte in the socket
    * as long as we have buffered data which we have not yet returned to
    * the client, so as to not drain this last byte from the socket until
    * we can return all the data buffered to the client, and only then
    * do we completly drain the socket.
    *
    * This makes sure the socket remains readable until we have returned
    * all the data belonging to a given token to the caller, which should
    * let all the kernels select(2)/poll(2)/SIGIO/etc. stuff work.
    *
    * Sounds simple enough, but alas, there has been many bugs here. :-/
    *
    * We also need to handle a client only peeking at the data, with
    * MSG_PEEK. In this case we can do the same as for a normal read,
    * except we must of course leave all the bytes in our internal buffer
    * too, and not just in the socket.  socks_getfrombuffer() handles this
    * if we pass MSG_PEEK to it.
    */

   drainsocket(iobuf,
               socks_bytesinbuffer(s, READ_BUF, 0) == 0,
               inputmem,
               sizeof(inputmem));
#endif /* SOCKS_CLIENT */

   if (readfrombuf > 0) {
      slog(LOG_DEBUG,
           "%s: returning %lu bytes from buffer.  Only peeked-at is now: %lu",
           function,
           (unsigned long)readfrombuf,
#if SOCKS_CLIENT
           (unsigned long)iobuf->info[READ_BUF].readalready
#else  /* !SOCKS_CLIENT */
           (unsigned long)0
#endif /* !SOCKS_CLIENT */
          );

      return readfrombuf;
   }
   else
      slog(LOG_DEBUG, "%s: mintoread = %lu.  Will try to read more from fd %d",
           function, (unsigned long)mintoread, s);

   /*
    * No (or not enough) decoded data buffered.  Since we were called there
    * should be (encoded) data available for read from the socket.
    *
    * Make sure we read with something that does not step on our own toes
    * concerning using our iobufs, like e.g. socks_recvfrom() could do,
    * and that we do not read more than we can subsequently save in our
    * buffer after returning the amount caller wants.
    */

   if (mintoread > 0)
      SASSERTX(socks_bytesinbuffer(s, READ_BUF, 0) < mintoread);

/*
 * NOTE: use of cpp 'if/else/endif' statements inside recv() does not
 *       work with the AIX 'xlc' compiler
 */
#if SOCKS_CLIENT
               /*
                * only peek now.  We cannot risk draining the socket buffer
                * completely and then not being able to return all the data
                * to the client in this call (can max return len bytes).
                * If that had happened, the client would not know to call us
                * again to get the data we have buffered because the kernel
                * will not mark the fd as readable.
                */
   nread = recv(s,
                inputmem,
                MIN(sizeof(inputmem), socks_freeinbuffer(s, READ_BUF)),
                (flags | MSG_PEEK)
               );
#else /* !SOCKS_CLIENT */
                /*
                 * No such complications in the server, which uses our
                 * selectn() function.
                 */
   nread = recv(s,
                inputmem,
                MIN(sizeof(inputmem), socks_freeinbuffer(s, READ_BUF)),
                flags
               );
#endif /* !SOCKS_CLIENT */

   slog(LOG_DEBUG,
        "%s: read from fd %d for new encoded bytes returned %ld (%s)",
        function, s, (long)nread, strerror(errno));

   if (nread <= 0) {
      if (mintoread == 0)
         return nread;

      if (errno == EAGAIN) {
         slog(LOG_DEBUG,
              "%s: read nothing from fd %d, but error is temporary (%s), so "
              "will block and retry until we have enough to return %lu bytes",
              function, s, strerror(errno), (unsigned long)mintoread);

         nread = 0;
      }
      else
         return nread;
   }

   if (nread > 0) {
      if (recvflags != NULL)
         recvflags->fromsocket += nread;

#if SOCKS_CLIENT
      iobuf->info[READ_BUF].readalready += nread;
#endif /* SOCKS_CLIENT */

      socks_addtobuffer(s, READ_BUF, 1, inputmem, nread);
   }

   /*
    * Now, decode all tokens we can decode with the data we've read and
    * added to the encoded-data buffer, subsequently moving the data to the
    * decoded-data buffer.  This will make it easy to check whether we have
    * any data to return to caller (i.e., we have decoded data available),
    * or not (no decoded data, and not enough encoded data to decode a
    * complete token).
    */

   tokennumber          = 0;
   encoded_left_to_read = -1;

   while (socks_bytesinbuffer(s, READ_BUF, 1) >= GSSAPI_HLEN) {
      p = socks_getfrombuffer(s, MSG_PEEK, READ_BUF, 1, inputmem, GSSAPI_HLEN);

      SASSERTX(p == GSSAPI_HLEN);

      if (!gssapi_headerisok(inputmem, p, &encodedlen, emsg, sizeof(emsg))) {
         slog(LOG_NOTICE,
              "%s: invalid gssapi header received on fd %d from %s: %s",
              function, s, peername2string(s, NULL, 0), emsg);

         errno = EPROTO;
         return -1;
      }

      encoded_left_to_read =    (long)(GSSAPI_HLEN + encodedlen)
                              - socks_bytesinbuffer(s, READ_BUF, 1);

      slog(LOG_DEBUG,
           "%s: have %ld bytes now, %ld encoded bytes left to read for "
           "the whole token",
           function,
           (unsigned long)   socks_bytesinbuffer(s, READ_BUF, 1)
                           - GSSAPI_HLEN,
           (long)(encoded_left_to_read));

      if (encoded_left_to_read > 0)
         break; /* no token to decode now. */
      else
         /*
          * Have token to read now.  Don't know how much (if any) left
          * to read for the next token.
          */
         encoded_left_to_read = -1;

      SASSERTX((size_t)(GSSAPI_HLEN + encodedlen) <= sizeof(inputmem));

      readfrombuf = socks_getfrombuffer(s,
                                        0,
                                        READ_BUF,
                                        1,
                                        inputmem,
                                        GSSAPI_HLEN + encodedlen);

      SASSERTX(readfrombuf == (size_t)(GSSAPI_HLEN + encodedlen));

      if (sockscf.option.debug >= 2 && encodedlen >= 4)
         slog(LOG_DEBUG,
              "%s: read all we need to decode token #%lu of length %u.   "
              "First encoded bytes: "
              "[%d]: 0x%x, [%d]: 0x%x, [%d]: 0x%x, [%d]: 0x%x.  "
              "Last: "
              "[%d]: 0x%x, [%d]: 0x%x, [%d]: 0x%x, [%d]: 0x%x",
              function,
              (unsigned long)++tokennumber,
              encodedlen,
              0,
              inputmem[GSSAPI_HLEN + 0],
              1,
              inputmem[GSSAPI_HLEN + 1],
              2,
              inputmem[GSSAPI_HLEN + 2],
              3,
              inputmem[GSSAPI_HLEN + 3],
              (int)(encodedlen - 4),
              inputmem[readfrombuf - 4],
              (int)(encodedlen - 3),
              inputmem[readfrombuf - 3],
              (int)(encodedlen - 2),
              inputmem[readfrombuf - 2],
              (int)(encodedlen - 1),
              inputmem[readfrombuf - 1]);

      input_token.value   = inputmem    + GSSAPI_HLEN;
      input_token.length  = readfrombuf - GSSAPI_HLEN;

      output_token.value  = outputmem;
      output_token.length = sizeof(outputmem);

      if (gssapi_decode(&input_token, gs, &output_token) != 0) {
         slog(LOG_DEBUG,
              "%s: invalid GSSAPI-data received for token of length %u",
              function, encodedlen);

         errno = EPROTO;
         return -1;
      }

      /*
       * Ok, have successfully decoded a complete GSSAPI token.  Add it
       * to our buffer and later loop around to see if there is more data
       * already read to decode.
       */
      socks_addtobuffer(s,
                        READ_BUF,
                        0,
                        (char *)output_token.value,
                        output_token.length);
   }

   if (encoded_left_to_read == -1) {
      SASSERTX(socks_bytesinbuffer(s, READ_BUF, 1) < GSSAPI_HLEN);

      encoded_left_to_read = GSSAPI_HLEN - socks_bytesinbuffer(s, READ_BUF, 1);

      slog(LOG_DEBUG, "%s: encoded left to read was -1.  Changed to %ld",
           function, (long)encoded_left_to_read);
   }

   if (socks_bytesinbuffer(s, READ_BUF, 0) >= mintoread) {
      slog(LOG_DEBUG, "%s: enough decoded data buffered to return %lu now",
           function, (unsigned long)mintoread);

      goto again;
   }

   /*
    * Else; block and try to read more from socket, before looping around.
    */

#if SOCKS_CLIENT
   /*
    * First drain whatever we only peeked at before.
    */
   drainsocket(iobuf, 1, inputmem, sizeof(inputmem));
#endif /* SOCKS_CLIENT */

   if (sockscf.option.debug)
      slog(LOG_DEBUG,
           "%s: blocking for read on fd %d.  Buffered: %lu, need: %lu.  "
           "encoded_left_to_read: %ld",
           function,
           s,
           socks_bytesinbuffer(s, READ_BUF, 0),
           (unsigned long)mintoread,
           (long)encoded_left_to_read);

   nread = recv(s,
                inputmem,
                encoded_left_to_read,
                (flags | MSG_PEEK) | MSG_WAITALL);

   slog(LOG_DEBUG,
        "%s: recv(2) on fd %d returned %ld encoded bytes. "
        "Last bytes: 0x%x, 0x%x.  Errno = %d (%s)",
        function,
        s,
        (long)nread,
        nread >= 2 ? inputmem[nread - 2] : 0,
        nread >= 2 ? inputmem[nread - 1] : 0,
        errno,
        strerror(errno));

   if (nread <= 0)
      return nread;

   /*
    * Else; should have something new to read from the socket now, possibly
    * enough for us to return data to caller also.  Loop around to check
    * what the case is.
    */
   goto again;
}

/*
 * RFC1961: client request / server response
 *
 *   +------+------+------+.......................+
 *   + ver  | mtyp | len  |       token           |
 *   +------+------+------+.......................+
 *   + 0x01 | 0x03 | 0x02 | up to 2^16 - 1 octets |
 *   +------+------+------+.......................+
 *
 */

ssize_t
gssapi_encode_write(s, msg, len, flags, to, tolen, sendtoflags, gs)
   int s;
   const void *msg;
   size_t len;
   int flags;
   const struct sockaddr_storage *to;
   socklen_t tolen;
   sendto_info_t *sendtoflags;
   gssapi_state_t *gs;
{
   const char *function = "gssapi_encode_write()";
   gss_buffer_desc input_token, output_token;
   unsigned char outputmem[GSSAPI_HLEN + MAXGSSAPITOKENLEN];
   unsigned short pshort;
   iobuffer_t *iobuf;
   ssize_t towrite, written, p, encodedlen, addedtobuf;
   size_t i;

#if 0 /* for aid in debuging bufferproblems. */
   static size_t j;
   size_t lenv[] = { 60000, 60001, 60002, 60003, 60004, 60005, 60006, 60007,
                     60008, 60009, 60010, 60011, 60012, 60013, 60014, 60015 };

   len = MIN(lenv[j % ELEMENTS(lenv)], len);
   ++j;
#endif

   slog(LOG_DEBUG, "%s: fd %d, len %lu, gssoverhead %lu",
        function, s, (unsigned long)len, (unsigned long)gs->gssoverhead);

   if ((iobuf = socks_getbuffer(s)) == NULL) {
      /*
       * Allocate one.
       */
      int stype;
      socklen_t tlen = sizeof(stype);

      /*
       * In server we are only using pre-allocated buffers and we allocate
       * them before this function should ever be called.
       */
      SASSERTX(SOCKS_CLIENT);

      if (getsockopt(s, SOL_SOCKET, SO_TYPE, &stype, &tlen) != 0) {
         swarn("%s: getsockopt(SO_TYPE) on fd %d failed", function, s);
         return -1;
      }

      if ((iobuf = socks_allocbuffer(s, stype)) == NULL) {
         swarn("%s: could not allocate iobuffer for fd %d", function, s);

         errno = ENOMEM;
         return -1;
      }
   }

   if (iobuf->stype == SOCK_DGRAM)
      return gssapi_encode_write_udp(s,
                                     msg,
                                     len,
                                     flags,
                                     to,
                                     tolen,
                                     sendtoflags,
                                     gs,
                                     outputmem,
                                     sizeof(outputmem));

#if SOCKS_CLIENT
   /*
    * Two modes: Buffered and unbuffered.
    *
    * Unbuffered:
    *    We try to write upto "len" bytes, and if that fails, we
    *    store the remaining bytes in our internal iobuf, encoded.
    *    This makes us able to return either "len" or -1 (only if fatal
    *    error) to caller, so that caller understands we have accepted
    *    all data, even though we may not yet have written it to the socket.
    *
    * Buffered:
    *    We keep saving the data in the buffer, but _not_ encoded.
    *    Only upon flush (socks_flushbuffer()), we encode and write it.
    *    This is only used by the client, to simulate stdio-buffering.
    */

   if (iobuf->info[WRITE_BUF].mode != _IONBF) {
      /*
       * buffered mode.
       */

      if (flags & MSG_OOB)
         swarnx("%s: oob data is currently not handled for buffered writes",
                function);

      if (socks_freeinbuffer(s, WRITE_BUF) < len) {
         /*
          * try to flush some data, hoping it will free up space in our buffer.
          */
         if (socks_flushbuffer(s, -1, sendtoflags) == -1)
            return -1;
      }

      if (socks_freeinbuffer(s, WRITE_BUF) < len) {
         if (errno == 0)
            errno = EAGAIN;

         return -1;
      }

      SASSERTX(socks_freeinbuffer(s, WRITE_BUF) >= len);

      socks_addtobuffer(s, WRITE_BUF, 0, msg, len);

      if (len >= 2) {
         if (memchr((const unsigned char *)msg, '\r', len) != NULL
         ||  memchr((const unsigned char *)msg, '\n', len) != NULL) {
            /*
             * More correct would be to only flush up to the \r or \n,
             * but that is a bit of a hassle.  Since this is only
             * needed for stupid Linux glibc systems that don't allow us
             * to interpose on the real network-calls, don't bother for now.
             */
            (void)socks_flushbuffer(s, -1, sendtoflags);
         }
      }

      return len;
   }
   /*
    * else; unbuffered mode.  Same as server then.  Encode and write.
    */
#endif /* SOCKS_CLIENT */

   if ((towrite = socks_bytesinbuffer(s, WRITE_BUF, 1)) > 0) {
      /*
       * have encoded data for write buffered already.  Must always flush
       * that before we attempt to write anything more to the socket.
       */

      /* no (non-temporary) buffering for udp. */
      SASSERTX(iobuf->stype == SOCK_STREAM);

      if (socks_flushbuffer(s, -1, sendtoflags) == -1)
         return -1;
   }

   /*
    * Ok, no more unencoded data in the buffer.  Encode the data passed
    * us now and see how much we can write to the network.
    */

   SASSERTX(socks_bytesinbuffer(s, WRITE_BUF, 1) == 0);

   /*
    * Attempt to avoid writing a partial token with no room to buffer the
    * remainder.  Since gs->gssoverhead is not a constant size, we need to
    * handle the possibility of it increasing, by checking again later that
    * we actually have enough room to store the encoded token in our buffer.
    *
    * We save space for the SOCKS GSSAPI header too.
    */
   output_token.length = MIN(sizeof(outputmem) - GSSAPI_HLEN,
                             socks_freeinbuffer(s, WRITE_BUF) - GSSAPI_HLEN);

   /* will put the SOCKS GSSAPI header at the start. */
   output_token.value = outputmem + GSSAPI_HLEN;

   if (gs->maxgssdata != 0) /* is 0 if not yet determined. */
      len = MIN(len, gs->maxgssdata);

   p = MIN(len, socks_freeinbuffer(s, WRITE_BUF) - gs->gssoverhead);

   if (p <= 0 || output_token.length < gs->gssoverhead) {
      slog(LOG_DEBUG,
           "%s: not enough room in buffer.  Free space in buffer is only %lu, "
           "while expected gssapi-encapsulation overhead is %lu",
           function,
           (unsigned long)socks_freeinbuffer(s, WRITE_BUF),
           (unsigned long)gs->gssoverhead);

      errno = EAGAIN;
      return -1;
   }

   if (len != (size_t)p)
      slog(LOG_DEBUG, "%s: only room in buffer to attempt write of %ld/%lu",
           function, (long)p, (unsigned long)len);

   len                = p;
   input_token.value  = msg;
   input_token.length = len;

   if (gssapi_encode(&input_token, gs, &output_token) != 0) {
      if (errno == EMSGSIZE) {
         OM_uint32 minor_status, major_status, maxlen;
         char emsg[1024];

         major_status
         = gss_wrap_size_limit(&minor_status,
                               gs->id,
                               gs->protection == GSSAPI_CONFIDENTIALITY ?
                                       GSS_REQ_CONF : GSS_REQ_INT,
                               GSS_C_QOP_DEFAULT,
                               output_token.length,
                               &maxlen);

         if (gss_err_isset(major_status, minor_status, emsg, sizeof(emsg))) {
            swarnx("%s: gss_wrap_size_limit(): %lu is too big a token for "
                   "GSSAPI-encoding and we are unable to determine what the "
                   "maximum is: %s",
                   function, (long unsigned)len, emsg);

            return -1;
         }

         slog(LOG_DEBUG,
              "%s: data of length %lu too big for GSSAPI-encode.  Maximum "
              "determined to be %lu.  Reducing length and trying again",
              function, (long unsigned)len, (long unsigned)maxlen);

         len                = maxlen;
         input_token.length = len;

         if (gssapi_encode(&input_token, gs, &output_token) == 0)
            errno = 0;
         else {
            swarnx("%s: unexpected. gssapi_encode() failed with the shorter "
                   "message of length %lu too",
                   function, (unsigned long)input_token.length);

            errno = ECONNABORTED;
         }
      }
      else {
         slog(LOG_DEBUG,
              "%s: gssapi_encode() unexpectedly failed on data of length %lu",
              function, (unsigned long)input_token.length);
      }

      return -1;
   }

   if (output_token.length + GSSAPI_HLEN > socks_freeinbuffer(s, WRITE_BUF)) {
      slog(LOG_DEBUG,
           "%s: not enough free space in buffer to hold token of length %lu.  "
           "Will need to flush some from buffer first",
           function, (unsigned long)(output_token.length + GSSAPI_HLEN));

      errno = EAGAIN;
      return -1;
   }

   /*
    * Prefix the SOCKS GSSAPI header to the token.
    */

   output_token.value  = outputmem; /* shift back to start. */

   i = 0;
   ((unsigned char *)output_token.value)[i++] = SOCKS_GSSAPI_VERSION;
   ((unsigned char *)output_token.value)[i++] = SOCKS_GSSAPI_PACKET;

   pshort = htons(output_token.length);
   memcpy(&((unsigned char *)output_token.value)[i], &pshort, sizeof(pshort));
   i += sizeof(pshort);

   SASSERTX(i == GSSAPI_HLEN);

   output_token.length += i;

   addedtobuf = socks_addtobuffer(s,
                                  WRITE_BUF,
                                  1,
                                  output_token.value,
                                  output_token.length);

   encodedlen = (size_t)output_token.length;

   /*
    * Just peek first, then re-get later what we actually manage to write
    * to the socket.
    */
   towrite = socks_getfrombuffer(s,
                                 MSG_PEEK,
                                 WRITE_BUF,
                                 1,
                                 outputmem,
                                 MIN(encodedlen, (ssize_t)sizeof(outputmem)));

   if (towrite >= GSSAPI_HLEN + 2)
      slog(LOG_DEBUG,
           "%s: attempting to write %lu encoded bytes.  "
           "[%d]: 0x%x, [%d]: 0x%x, [%d]: 0x%x, [%d]: 0x%x",
           function,
           (unsigned long)towrite,
           GSSAPI_HLEN + 0,
           outputmem[GSSAPI_HLEN + 0],
           GSSAPI_HLEN + 1,
           outputmem[GSSAPI_HLEN + 1],
           (int)towrite - 2,
           outputmem[towrite - 2],
           (int)towrite - 1,
           outputmem[towrite - 1]);
   else
      slog(LOG_DEBUG, "%s: attempting to write %lu encoded bytes",
           function, (unsigned long)towrite);

   if ((written = sendto(s, outputmem, towrite, flags, TOCSA(to), tolen)) > 0) {
      slog(LOG_DEBUG, "%s: wrote %ld/%ld bytes",
           function, (long)written, (long)towrite);

      /*
       * First time we just peeked, now actually read the data out of the
       * buffer.
       */
      towrite = socks_getfrombuffer(s, 0, WRITE_BUF, 1, outputmem, written);

      if (sendtoflags != NULL)
         sendtoflags->tosocket += written;
   }

   if (sockscf.option.debug)
      slog(LOG_DEBUG,
           "%s: wrote %ld/%lu to fd %d, buffer now has %lu bytes free.  "
           "Errno is %d (%s)",
           function,
           (long)written,
           (long unsigned)towrite,
           s,
           (long unsigned)socks_freeinbuffer(s,  WRITE_BUF),
           errno,
           strerror(errno));

   if (written >= 0)
      /*
       * Wrote some and saved the rest in our own buffer.
       */
      return len;

   /*
    * Else, some error.  Did not manage to write anything now.
    *
    * In the server-case, we do not care about that as long as the error
    * is temporary, since we have accepted the data in to our buffer.
    *
    * In the client-case, we do not know whether the client cares or not,
    * so return the error.  That of course means we must remove the data
    * from our buffer also, so that we do not add the same data when
    * the client tries the same write again later.
    */

   if (!ERRNOISTMP(errno) || SOCKS_CLIENT) {
      slog(LOG_DEBUG,
           "%s: write failed error %d (%s).  Removing %ld bytes from "
           "buffer and returning %ld",
           function, errno, strerror(errno), (long)addedtobuf, (long)written);

      p = socks_getfrombuffer(s, 0, WRITE_BUF, 1, outputmem, addedtobuf);
      SASSERTX(p == addedtobuf);

      SASSERTX(written < 0);
      return written;
   }

   /*
    * Else: Some temporary error.  The server does not care about this
    * as the data has been added to our buffer.
    */
   SASSERTX(!SOCKS_CLIENT);

   return len;
}

int
gssapi_export_state(id, state)
   gss_ctx_id_t *id;
   gss_buffer_desc *state;
{
   const char *function = "gssapi_export_state()";
   const int errno_s = errno;
   OM_uint32 major_status, minor_status;
   gss_buffer_desc token;
   char emsg[512];
#if SOCKS_CLIENT
   sigset_t oldset;
#endif /* SOCKS_CLIENT */

#if SOCKS_CLIENT && SOCKSLIBRARY_DYNAMIC
   DNSCODE_START();
#endif /* SOCKS_CLIENT && SOCKSLIBRARY_DYNAMIC */

   slog(LOG_DEBUG, "%s", function);

   SOCKS_SIGBLOCK_IF_CLIENT(SIGIO, &oldset);

   major_status = gss_export_sec_context(&minor_status, id, &token);

   SOCKS_SIGUNBLOCK_IF_CLIENT(&oldset);

   if (gss_err_isset(major_status, minor_status, emsg, sizeof(emsg))) {
      swarnx("%s: gss_export_sec_context() failed: %s", function, emsg);

#if SOCKS_CLIENT && SOCKSLIBRARY_DYNAMIC
   DNSCODE_END();
#endif /* SOCKS_CLIENT && SOCKSLIBRARY_DYNAMIC */

      return -1;
   }

   if (token.length > state->length) {
      swarnx("%s: we depend on the size of the exported gssapi context not "
             "being larger than a predefined value (%lu), but unfortunately "
             "the value here (%lu) larger than that.  Please let us know",
             function,
             (unsigned long)state->length,
             (unsigned long)token.length);

      SWARNX(0);

#if SOCKS_CLIENT && SOCKSLIBRARY_DYNAMIC
   DNSCODE_START();
#endif /* SOCKS_CLIENT && SOCKSLIBRARY_DYNAMIC */

      return -1;
   }

   SASSERTX(token.length <= state->length);
   memcpy(state->value, token.value, token.length);
   state->length = token.length;

   SOCKS_SIGBLOCK_IF_CLIENT(SIGIO, &oldset);

   gss_release_buffer(&minor_status, &token);

   SOCKS_SIGUNBLOCK_IF_CLIENT(&oldset);

   slog(LOG_DEBUG,
        "%s: exported gssapistate at %p of length %lu (start: 0x%x, 0x%x)",
        function,
        state->value,
        (unsigned long)state->length,
        ((unsigned char *)state->value)[0],
        ((unsigned char *)state->value)[1]);

#if SOCKS_CLIENT && SOCKSLIBRARY_DYNAMIC
   DNSCODE_END();
#endif /* SOCKS_CLIENT && SOCKSLIBRARY_DYNAMIC */

   errno = errno_s; /* at least some gssapi libraries change errno. :-/ */
   return 0;
}

int
gssapi_import_state(id, state)
   gss_ctx_id_t *id;
   gss_buffer_desc *state;
{
   const char *function = "gssapi_import_state()";
   const int errno_s = errno;
   OM_uint32 major_status, minor_status;
   char emsg[512];
#if SOCKS_CLIENT
   sigset_t oldset;
#endif /* SOCKS_CLIENT */

#if SOCKS_CLIENT && SOCKSLIBRARY_DYNAMIC

   DNSCODE_START();

#endif /* SOCKS_CLIENT && SOCKSLIBRARY_DYNAMIC */

   slog(LOG_DEBUG,
        "%s: importing gssapistate at %p of length %lu " "(start: 0x%x, 0x%x)",
         function,
         state->value,
         (unsigned long)state->length,
         ((unsigned char *)state->value)[0],
         ((unsigned char *)state->value)[1]);

   SOCKS_SIGBLOCK_IF_CLIENT(SIGIO, &oldset);

   major_status = gss_import_sec_context(&minor_status, state, id);

   SOCKS_SIGUNBLOCK_IF_CLIENT(&oldset);

   if (gss_err_isset(major_status, minor_status, emsg, sizeof(emsg))) {
      swarnx("%s: gss_import_sec_context() failed: %s", function, emsg);

#if SOCKS_CLIENT && SOCKSLIBRARY_DYNAMIC

      DNSCODE_END();

#endif /* SOCKS_CLIENT && SOCKSLIBRARY_DYNAMIC */

      return -1;
   }
   else
      slog(LOG_DEBUG, "%s: gss_import_sec_context() complete", function);

#if SOCKS_CLIENT && SOCKSLIBRARY_DYNAMIC

   DNSCODE_END();

#endif /* SOCKS_CLIENT && SOCKSLIBRARY_DYNAMIC */

   errno = errno_s; /* at least some gssapi libraries change errno. :-/ */

   return 0;
}

static int
gssapi_headerisok(headerbuf, len, tokenlen, emsg, emsglen)
   const unsigned char *headerbuf;
   const size_t len;
   unsigned short *tokenlen;
   char *emsg;
   const size_t emsglen;
{
   const char *function = "gssapi_headerisok()";

   if (len < GSSAPI_HLEN) {
      snprintf(emsg, emsglen,
               "gssapi packet of length %lu is too short.  Minimum is %lu",
               (unsigned long)len, (unsigned long)GSSAPI_HLEN);
      return 0;
   }

   if (headerbuf[GSSAPI_VERSION] != SOCKS_GSSAPI_VERSION
   ||  headerbuf[GSSAPI_STATUS]  != SOCKS_GSSAPI_PACKET) {
      snprintf(emsg, emsglen,
               "invalid socks gssapi header (0x%x, 0x%x, not 0x%x, 0x%x)",
               (unsigned char)headerbuf[GSSAPI_VERSION],
               (unsigned char)headerbuf[GSSAPI_STATUS],
               SOCKS_GSSAPI_VERSION,
               SOCKS_GSSAPI_PACKET);

      return 0;
   }

   memcpy(tokenlen, &headerbuf[GSSAPI_TOKEN_LENGTH], sizeof(*tokenlen));
   *tokenlen = ntohs(*tokenlen);

   slog(LOG_DEBUG, "%s: SOCKS header for GSSAPI token of length %u is ok",
        function, *tokenlen);

   return 1;
}

#if SOCKS_CLIENT
int
gssapi_isencrypted(s)
   const int s;
{
   socksfd_t socksfd;

   if (!sockscf.state.havegssapisockets)
      return 0;

   /* XXX this takes too long. */
   if (!socks_addrisours(s, &socksfd, 1)) {
      socks_rmaddr(s, 1);
      return 0;
   }

   if (socksfd.state.auth.method != AUTHMETHOD_GSSAPI)
      return 0;

   return socksfd.state.auth.mdata.gssapi.state.wrap;
}

#endif /* SOCKS_CLIENT */

static ssize_t
gssapi_decode_read_udp(s, buf, len, flags, from, fromlen, recvflags, gs,
                       token, tokensize)
   int s;
   void *buf;
   size_t len;
   int flags;
   struct sockaddr_storage *from;
   socklen_t *fromlen;
   recvfrom_info_t *recvflags;
   gssapi_state_t *gs;
   unsigned char *token;
   const size_t tokensize;
{
   const char *function = "gssapi_decode_read_udp()";
   gss_buffer_desc input_token, output_token;
   unsigned short encodedlen;
   ssize_t nread;
   char emsg[512];

   slog(LOG_DEBUG, "%s: fd %d, len %lu, flags %d",
        function, s, (long unsigned)len, flags);

   /*
    * Ok since we don't buffer udp data.  But if that is ever added, we
    * must make sure socks_recvfrom() does not step on our toes as far
    * as using our iobuffer is concerned (e.g. by adding data to the iobuf,
    * saying it is not encrypted since we pass NULL for auth to it).
    */
   if ((nread = socks_recvfrom(s,
                               token,
                               tokensize,
                               flags,
                               from,
                               fromlen,
                               recvflags,
                               NULL)) <= 0) {
      slog(LOG_DEBUG, "%s: read from fd %d returned %ld: %s",
           function, s, (long)nread, strerror(errno));

      return nread;
   }

   slog(LOG_DEBUG, "%s: read %ld/%lu from socket",
        function, (long)nread, (long unsigned)tokensize);

   if (nread < GSSAPI_HLEN) {
      slog(LOG_NOTICE,
           "%s: packet read on fd %d (%s) is shorter than minimal gssapi "
           "header length (%ld < %lu)",
           function,
           s,
           socket2string(s, NULL, 0),
           (long)nread,
           (unsigned long)GSSAPI_HLEN);

      errno = ENOMSG;
      return -1;
   }

   if (!gssapi_headerisok(token, nread, &encodedlen, emsg, sizeof(emsg))) {
      slog(LOG_NOTICE,
           "%s: invalid gssapi header on fd %d (packet from %s): %s",
           function, s, socket2string(s, NULL, 0), emsg);

      errno = ENOMSG;
      return -1;
   }

   if (nread < GSSAPI_HLEN + encodedlen) {
      slog(LOG_NOTICE,
           "%s: short packet on fd %d (packet from %s).  Should be %lu bytes, "
           "but received only %ld",
           function,
           s,
           socket2string(s, NULL, 0),
           (unsigned long)(GSSAPI_HLEN + encodedlen),
           (long)nread);

      errno = ENOMSG;
      return -1;
   }

   slog(LOG_DEBUG, "%s: read complete token of encoded size %d + %u",
        function, GSSAPI_HLEN, encodedlen);

   input_token.value  = token + GSSAPI_HLEN;
   input_token.length = nread - GSSAPI_HLEN;

   output_token.value  = buf;
   output_token.length = len;

   if (gssapi_decode(&input_token, gs, &output_token) != 0) {
      slog(LOG_NOTICE,
           "%s: udp token of length %u failed decode - discarded: %s",
           function,
           encodedlen,
           errno == ENOMEM ? "output buffer too small" : strerror(errno));

      if (errno == ENOMEM) {
#if !SOCKS_CLIENT
         SWARNX(len);
#endif /* !SOCKS_CLIENT */
      }

      errno = ENOMSG;
      return -1;
   }

   return output_token.length;
}

static ssize_t
gssapi_encode_write_udp(s, msg, len, flags, to, tolen, sendtoflags, gs,
                        token, tokensize)
   int s;
   const void *msg;
   size_t len;
   int flags;
   const struct sockaddr_storage *to;
   socklen_t tolen;
   sendto_info_t *sendtoflags;
   gssapi_state_t *gs;
   unsigned char *token;
   const size_t tokensize;
{
   const char *function = "gssapi_encode_write_udp()";
   gss_buffer_desc input_token, output_token;
   unsigned short pshort;
   ssize_t towrite, written;
   size_t i;

   slog(LOG_DEBUG, "%s: fd %d, len %lu, gssoverhead %lu",
        function, s, (unsigned long)len, (unsigned long)gs->gssoverhead);

   /* save space for SOCKS GSSAPI header too. */
   output_token.length = tokensize - GSSAPI_HLEN;

   /* will put the SOCKS GSSAPI header at the start. */
   output_token.value = token + GSSAPI_HLEN;

   input_token.value  = msg;
   input_token.length = len;

   if (gssapi_encode(&input_token, gs, &output_token) != 0)
      return -1;

   /*
    * Prefix the SOCKS GSSAPI header to the token.
    */

   output_token.value  = token; /* shift back to start. */

   i = 0;
   ((unsigned char *)output_token.value)[i++] = SOCKS_GSSAPI_VERSION;
   ((unsigned char *)output_token.value)[i++] = SOCKS_GSSAPI_PACKET;

   pshort = htons(output_token.length);
   memcpy(&((unsigned char *)output_token.value)[i], &pshort, sizeof(pshort));
   i += sizeof(pshort);

   SASSERTX(i == GSSAPI_HLEN);

   output_token.length += i;
   towrite              = output_token.length;


   if (towrite >= GSSAPI_HLEN + 2)
      slog(LOG_DEBUG,
           "%s: attempting to write %lu encoded bytes.  "
           "[0]: 0x%x, [1]: 0x%x, [%d]: 0x%x, [%d]: 0x%x",
           function,
           (unsigned long)towrite,
           token[0],
           token[1],
           (int)towrite - 2,
           token[towrite - 2],
           (int)towrite - 1,
           token[towrite - 1]);
   else
      slog(LOG_DEBUG, "%s: attempting to write %lu encoded bytes",
           function, (unsigned long)towrite);

   written = sendto(s, token, towrite, flags, TOCSA(to), tolen);

   if (sendtoflags != NULL && written > 0)
      sendtoflags->tosocket += written;

   slog(LOG_DEBUG, "%s: wrote %ld/%lu (%lu unencoded) to fd %d",
        function, (long)written, (unsigned long)towrite, (unsigned long)len, s);

   return len;
}

#if SOCKS_CLIENT

static void
drainsocket(iobuf, drainitall, buf, bufsize)
   iobuffer_t *iobuf;
   const int drainitall;
   void *buf;
   const size_t bufsize;
{
   const char *function = "drainsocket()";
   ssize_t nread, drain;

   /*
    * Drain the bytes we only peeked at before, so that select(2)
    * and similar won't mark the fd as readable unless there is more
    * data to read from the socket, as there is nothing we can return
    * until we have enough data to start decoding.
    *
    * If we were requested to only peek however, the client might get a
    * problem as if select(2) says the fd was readable, there should
    * be data to read.  We don't have any data to return yet however,
    * and there isn't anything we can do about that.  The client may
    * rightfully get confused about this, but we have to hope it will
    * handle an EAGAIN error for this ok.
    */

   if (drainitall)
      drain = iobuf->info[READ_BUF].readalready;
   else
      /*
       * still have data buffered.  Make sure kernel keeps marking
       * this fd as readable.
       */
      drain = iobuf->info[READ_BUF].readalready - 1;

   slog(LOG_DEBUG, "%s: draining socket for %ld peeked at bytes",
        function, (long)drain);

   SASSERTX(drain <= (ssize_t)bufsize);
   SASSERTX(drain >= 0);

   if (drain == 0)
      return;

   while ((nread = read(iobuf->s, buf, (size_t)drain)) == -1 && errno == EINTR)
      ;

   iobuf->info[READ_BUF].readalready -= (nread == -1 ? 0 : nread);

   if (nread != drain) {
      slog(LOG_INFO,
           "%s: strange ... could not re-read %ld bytes from fd %d.  "
           "Read only %ld (%s).  Removing %ld bytes from our buffer",
           function,
           (long)drain,
           iobuf->s,
           (long)nread,
           strerror(errno),
           nread == -1 ? drain : drain - nread);

      /*
       * Need to remove the data we could not re-read from the socket
       * from our own buffer also, so we do not add it again next time.
       */
      socks_getfrombuffer(iobuf->s,
                          0,
                          READ_BUF,
                          1,
                          buf,
                          nread == -1 ? drain : drain - nread);
   }
}

#endif /* SOCKS_CLIENT */

#endif /* HAVE_GSSAPI */
