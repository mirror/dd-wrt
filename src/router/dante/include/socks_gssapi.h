/*
 * Copyright (c) 2009, 2011, 2012, 2013
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
  * This code was contributed by
  * Markus Moeller (markus_moeller at compuserve.com).
  */

#if HAVE_GSSAPI

gssapi_state_t *
socks_get_gssapi_state(const unsigned int fd, const int havelock);
/*
 * If "havelock" is true, it means the function has already taken
 * care of locking the addr object.
 *
 * Returns:
 *      On success:  the gssapi state associated with file descriptor "fd".
 *      On failure:  NULL.  (no socket address associated with "fd").
 */

ssize_t
gssapi_decode_read(int s, void *buf, size_t len, int flags,
                   struct sockaddr_storage *from, socklen_t *fromlen,
                   recvfrom_info_t *recvflags, gssapi_state_t *gs);
/*
 * Read data from socket s, assuming it is socks gssapi conforming.
 * Arguments are similar to socks_recvfrom().
 * Returns:
 *      On success:  The number of decoded bytes read.
 *      On failure:  -1
 */

ssize_t
gssapi_encode_write(int s, const void *msg, size_t len, int flags,
                    const struct sockaddr_storage *to, socklen_t tolen,
                    sendto_info_t *sendtoflags, gssapi_state_t *gs);
/*
 * Write data of len "len" to socket s, after gssapi-encapsulating it.
 * Returns:
 *      On success:  The number of unencoded bytes written.
 *      On failure:  -1
 */

int
gssapi_isencrypted(int s);
/*
 * If "s" refers to a gssapi socket using encryption, return true.
 * Otherwise, return false.
 */

int
gss_err_isset(OM_uint32 major_status, OM_uint32 minor_status,
              char *buf, size_t buflen);

int
gssapi_export_state(gss_ctx_id_t *id, gss_buffer_desc *state);
/*
 * Exports the gssapi security context given by "id" into "state".
 * Returns 0 on success, -1 on failure.
 */

int
gssapi_import_state(gss_ctx_id_t *id, gss_buffer_desc *state);
/*
 * Imports the gssapi security context given by "state".  On
 * successful import, the id of the new context is stored in "id".
 *
 * Returns 0 on success, -1 on failure.
 */

#define CLEAN_GSS_TOKEN(token) do {                                            \
   OM_uint32 major_status, minor_status;                                       \
   SIGSET_ALLOCATE(oldset);                                                    \
   char buf[1024];                                                             \
                                                                               \
   SOCKS_SIGBLOCK_IF_CLIENT(SIGIO, &oldset);                                   \
                                                                               \
   major_status = gss_release_buffer(&minor_status, &(token));                 \
   if (gss_err_isset(major_status, minor_status, buf, sizeof(buf)))            \
      swarnx("%s: gss_release_buffer() at %s:%d failed: %s",                   \
             function, __FILE__, __LINE__, buf);                               \
                                                                               \
   SOCKS_SIGUNBLOCK_IF_CLIENT(&oldset);                                        \
} while (/* CONSTCOND */ 0)

#define CLEAN_GSS_AUTH(client_name, server_name, server_creds) do {            \
   OM_uint32 major_status, minor_status;                                       \
   SIGSET_ALLOCATE(oldset);                                                    \
   char buf[1024];                                                             \
                                                                               \
   SOCKS_SIGBLOCK_IF_CLIENT(SIGIO, &oldset);                                   \
                                                                               \
   if ((client_name) != GSS_C_NO_NAME) {                                       \
      major_status = gss_release_name(&minor_status, &(client_name));          \
      if (gss_err_isset(major_status, minor_status, buf, sizeof(buf)))         \
         swarnx("%s: gss_release_name() at %s:%d failed: %s",                  \
                function, __FILE__, __LINE__, buf);                            \
   }                                                                           \
                                                                               \
   if ((server_name) != GSS_C_NO_NAME) {                                       \
      major_status = gss_release_name(&minor_status, &(server_name));          \
      if (gss_err_isset(major_status, minor_status, buf, sizeof(buf)))         \
         swarnx("%s: gss_release_name() at %s:%d failed: %s",                  \
                function, __FILE__, __LINE__, buf);                            \
   }                                                                           \
                                                                               \
   if ((server_creds) != GSS_C_NO_CREDENTIAL) {                                \
      major_status = gss_release_cred(&minor_status, &(server_creds));         \
      if (gss_err_isset(major_status, minor_status, buf, sizeof(buf)))         \
         swarnx("%s: gss_release_name() at %s:%d failed: %s",                  \
                function, __FILE__, __LINE__, buf);                            \
   }                                                                           \
                                                                               \
   SOCKS_SIGUNBLOCK_IF_CLIENT(&oldset);                                        \
} while (/* CONSTCOND */ 0)

#endif /* SOCKS_CLIENT && SOCKSLIBRARY_DYNAMIC */
