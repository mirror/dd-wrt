/*
 * Copyright (c) 2009, 2010, 2011, 2012, 2013, 2014, 2019, 2024
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

#include "common.h"

#if HAVE_GSSAPI

static const char rcsid[] =
   "$Id: method_gssapi.c,v 1.73.4.1.6.4.4.2 2024/11/20 22:05:39 karls Exp $";

static negotiate_result_t
recv_gssapi_auth_ver(int s, request_t *request, negotiate_state_t *state);

static negotiate_result_t
recv_gssapi_auth_type(int s, request_t *request, negotiate_state_t *state);

static negotiate_result_t
recv_gssapi_auth_len(int s, request_t *request, negotiate_state_t *state);

static negotiate_result_t
recv_gssapi_auth_token(int s, request_t *request, negotiate_state_t *state);

static negotiate_result_t
recv_gssapi_enc_ver(int s, request_t *request, negotiate_state_t *state);

static negotiate_result_t
recv_gssapi_enc_type(int s, request_t *request, negotiate_state_t *state);

static negotiate_result_t
recv_gssapi_enc_len(int s, request_t *request, negotiate_state_t *state);

static negotiate_result_t
recv_gssapi_enc_token(int s, request_t *request, negotiate_state_t *state);

static negotiate_result_t
recv_gssapi_packet(int s, request_t *request, negotiate_state_t *state);

static negotiate_result_t
recv_gssapi_packet_ver(int s, request_t *request, negotiate_state_t *state);

static negotiate_result_t
recv_gssapi_packet_type(int s, request_t *request, negotiate_state_t *state);

static negotiate_result_t
recv_gssapi_packet_len(int s, request_t *request, negotiate_state_t *state);

static negotiate_result_t
recv_gssapi_packet_token(int s, request_t *request, negotiate_state_t *state);


negotiate_result_t
method_gssapi(s, request, state)
   int s;
   request_t *request;
   negotiate_state_t *state;

{
   const char *function = "method_gssapi()";

   slog(LOG_DEBUG, "%s", function);

   request->auth->mdata.gssapi.state.id = GSS_C_NO_CONTEXT;

   state->rcurrent = recv_gssapi_auth_ver;
   return state->rcurrent(s, request, state);
}

/*
 *   RFC1961: client request
 *
 *   +------+------+------+.......................+
 *   + ver  | mtyp | len  |       token           |
 *   +------+------+------+.......................+
 *   + 0x01 | 0x01 | 0x02 | up to 2^16 - 1 octets |
 *   +------+------+------+.......................+
 *
 */

static negotiate_result_t
recv_gssapi_auth_ver(s, request, state)
   int s;
   request_t *request;
   negotiate_state_t *state;
{
   const char *function = "recv_gssapi_auth_ver()";
   unsigned char gssapi_auth_version;

   INIT(sizeof(gssapi_auth_version));
   CHECK(&gssapi_auth_version, request->auth, NULL);

   switch (gssapi_auth_version) {
      case SOCKS_GSSAPI_VERSION:
         break;

      default:
         snprintf(state->emsg, sizeof(state->emsg),
                  "%s: unknown %s version on gssapi packet from client: %d",
                  function,
                  proxyprotocol2string(request->version),
                  gssapi_auth_version);

         return NEGOTIATE_ERROR;
   }

   state->rcurrent = recv_gssapi_auth_type;
   return state->rcurrent(s, request, state);
}

static negotiate_result_t
recv_gssapi_auth_type(s, request, state)
   int s;
   request_t *request;
   negotiate_state_t *state;
{
   const char *function = "recv_gssapi_auth_type()";
   unsigned char gssapi_auth_type;

   INIT(sizeof(gssapi_auth_type));
   CHECK(&gssapi_auth_type, request->auth, NULL);

   switch (gssapi_auth_type) {
      case SOCKS_GSSAPI_AUTHENTICATION:
         break;

      case SOCKS_GSSAPI_ENCRYPTION:
      case SOCKS_GSSAPI_PACKET:
         snprintf(state->emsg, sizeof(state->emsg),
         "%s: received out of sequence exchange from client.  "
         "Got type %d, expected type %d",
         function, gssapi_auth_type, SOCKS_GSSAPI_AUTHENTICATION);

         return NEGOTIATE_ERROR;

      default:
         snprintf(state->emsg, sizeof(state->emsg),
         "%s: unknown type on gssapi packet from client: %d",
         function, gssapi_auth_type);

         return NEGOTIATE_ERROR;
   }

   state->rcurrent = recv_gssapi_auth_len;
   return state->rcurrent(s, request, state);
}

static negotiate_result_t
recv_gssapi_auth_len(s, request, state)
   int s;
   request_t *request;
   negotiate_state_t *state;
{
   INIT(sizeof(state->gssapitoken_len));
   CHECK(&state->gssapitoken_len, request->auth, NULL);

   state->gssapitoken_len
   = ntohs((short)state->gssapitoken_len);

   state->rcurrent = recv_gssapi_auth_token;
   return state->rcurrent(s, request, state);
}

static negotiate_result_t
recv_gssapi_auth_token(s, request, state)
   int s;
   request_t *request;
   negotiate_state_t *state;
{
   const char *function = "recv_gssapi_auth_token()";
   sendto_info_t sendtoflags;
   gss_name_t            client_name   = GSS_C_NO_NAME;
   gss_name_t            server_name   = GSS_C_NO_NAME;
   gss_cred_id_t         server_creds  = GSS_C_NO_CREDENTIAL;
   gss_buffer_desc       output_token  = GSS_C_EMPTY_BUFFER;
   gss_buffer_desc       input_token;
   OM_uint32             ret_flags, major_status, minor_status;
   unsigned short        token_length;
   ssize_t rc;
   size_t buflen;
   unsigned char buf[GSSAPI_HLEN + MAXGSSAPITOKENLEN];
   char env[sizeof(request->auth->mdata.gssapi.keytab)], emsg[1024];
#if HAVE_PAC
   krb5_context          k5_context = NULL;
   krb5_error_code ret;
   krb5_pac pac;
#if HAVE_HEIMDAL_KERBEROS
   gss_buffer_desc       data_set = GSS_C_EMPTY_BUFFER;
#else
   gss_buffer_desc       type_id  = GSS_C_EMPTY_BUFFER;
#endif /* HAVE_HEIMDAL_KERBEROS */
#endif /* HAVE_PAC */

   INIT(state->gssapitoken_len);

   input_token.length = state->gssapitoken_len;
   input_token.value  = buf;
   SASSERTX(input_token.length <= sizeof(buf));

   CHECK(input_token.value, request->auth, NULL);

   STRCPY_ASSERTLEN(env, request->auth->mdata.gssapi.keytab);
   setenv("KRB5_KTNAME", env, 1);

#if HAVE_HEIMDAL_KERBEROS
   gsskrb5_register_acceptor_identity(request->auth->mdata.gssapi.keytab);
#endif /* HAVE_HEIMDAL_KERBEROS */

   slog(LOG_DEBUG,"%s: using gssapi service name %s",
   function, request->auth->mdata.gssapi.servicename);

   if (strcasecmp(request->auth->mdata.gssapi.servicename, "GSS_C_NO_NAME")
   != 0) {
      gss_buffer_desc service;

      service.value  = request->auth->mdata.gssapi.servicename;
      service.length = strlen(service.value);

      major_status
      = gss_import_name(&minor_status,
                        &service,
                        strchr(request->auth->mdata.gssapi.servicename, '/')
                        != NULL ?
                                   (gss_OID)GSS_C_NULL_OID
                                 : (gss_OID)gss_nt_service_name,
                        &server_name);

      if (gss_err_isset(major_status, minor_status, emsg, sizeof(emsg))) {
         snprintf(state->emsg, sizeof(state->emsg),
         "%s: gss_import_name() %s", function, emsg);

         CLEAN_GSS_AUTH(client_name, server_name, server_creds);
         return NEGOTIATE_ERROR;
      }
   }

   sockd_priv(SOCKD_PRIV_GSSAPI, PRIV_ON);
   major_status = gss_acquire_cred(&minor_status,
                                   server_name,
                                   GSS_C_INDEFINITE,
                                   GSS_C_NO_OID_SET,
                                   GSS_C_ACCEPT,
                                   &server_creds,
                                   NULL,
                                   NULL);
   sockd_priv(SOCKD_PRIV_GSSAPI, PRIV_OFF);

   if (gss_err_isset(major_status, minor_status, emsg, sizeof(emsg))) {
      snprintf(state->emsg, sizeof(state->emsg), "%s: gss_acquire_cred(): %s",
               function, emsg);

      CLEAN_GSS_AUTH(client_name, server_name, server_creds);
      return NEGOTIATE_ERROR;
   }

   sockd_priv(SOCKD_PRIV_GSSAPI, PRIV_ON);
   major_status
   = gss_accept_sec_context(&minor_status,
                            &request->auth->mdata.gssapi.state.id,
                            server_creds,
                            &input_token,
                            GSS_C_NO_CHANNEL_BINDINGS,
                            &client_name,
                            NULL,
                            &output_token,
                            &ret_flags,
                            NULL,
                            NULL);
   sockd_priv(SOCKD_PRIV_GSSAPI, PRIV_OFF);

   slog(LOG_DEBUG, "%s: length of output_token is %lu",
        function, (unsigned long)output_token.length);

   if (gss_err_isset(major_status, minor_status, emsg, sizeof(emsg))) {
      snprintf(state->emsg, sizeof(state->emsg),
               "%s: gss_accept_sec_context() failed: %s", function, emsg);

      CLEAN_GSS_AUTH(client_name, server_name, server_creds);
      return NEGOTIATE_ERROR;
   }

   /*
    * Don't need the input token anymore, so use the token buffer
    * to hold the reply from now on.
    */

   /*
    * RFC1961: server reply
    *
    *   +------+------+------+.......................+
    *   + ver  | mtyp | len  |       token           |
    *   +------+------+------+.......................+
    *   + 0x01 | 0x01 | 0x02 | up to 2^16 - 1 octets |
    *   +------+------+------+.......................+
    *
    */

   SASSERTX(GSSAPI_HLEN + output_token.length <= sizeof(buf));

   buflen = 0;
   buf[buflen++] = SOCKS_GSSAPI_VERSION;
   buf[buflen++] = SOCKS_GSSAPI_AUTHENTICATION;

   token_length = htons(output_token.length);
   memcpy(&buf[buflen], &token_length, sizeof(token_length));
   buflen += sizeof(token_length);

   memcpy(&buf[buflen], output_token.value, output_token.length);
   buflen += output_token.length;

   CLEAN_GSS_TOKEN(output_token);

   bzero(&sendtoflags, sizeof(sendtoflags));
   sendtoflags.side = INTERNALIF;

   if ((rc = socks_sendton(s,
                           buf,
                           buflen,
                           0,
                           0,
                           NULL,
                           0,
                           NULL,
                           request->auth)) != (ssize_t)buflen)  {
      snprintf(state->emsg, sizeof(state->emsg),
              "socks_sendton() token: wrote %ld out of %lu byte%s: %s",
               (long)rc,
               (unsigned long)buflen,
               buflen == 1 ? "" : "s",
               strerror(errno));

      CLEAN_GSS_AUTH(client_name, server_name, server_creds);
      return NEGOTIATE_ERROR;
   }

   if (major_status == GSS_S_COMPLETE) {
      /* Get username */
      major_status = gss_display_name(&minor_status, client_name, &output_token,
                                      NULL);

      if (gss_err_isset(major_status, minor_status, emsg, sizeof(emsg))) {
         snprintf(state->emsg, sizeof(state->emsg),
                  "%s: gss_display_name(): %s", function, emsg);

         CLEAN_GSS_AUTH(client_name, server_name, server_creds);
         return NEGOTIATE_ERROR;
      }

      memcpy(request->auth->mdata.gssapi.name, output_token.value,
             output_token.length);
      request->auth->mdata.gssapi.name[output_token.length] = NUL;

#if HAVE_PAC
      ret = krb5_init_context(&k5_context);
      if (!krb5_err_isset(k5_context, emsg, sizeof(emsg), ret)) {
#define ADWIN2KPAC 128
#if HAVE_HEIMDAL_KERBEROS
         major_status = gsskrb5_extract_authz_data_from_sec_context(&minor_status,
                                                                    request->auth->mdata.gssapi.state.id,
                                                                    ADWIN2KPAC,
                                                                    &data_set);
         if (!gss_err_isset(major_status, minor_status, emsg, sizeof(emsg))) {
            ret = krb5_pac_parse(k5_context, data_set.value, data_set.length, &pac);
            gss_release_buffer(&minor_status, &data_set);
            if (!krb5_err_isset(k5_context, emsg, sizeof(emsg), ret)) {
               if (get_sids(request->auth->mdata.gssapi.sids, k5_context, pac, state)<0) {
                  slog(LOG_INFO, "%s: error in getting sids", function);
	       }
               krb5_pac_free(k5_context, pac);
            }
            krb5_free_context(k5_context);
         } else {
	    /* Not a major error - maybe not a MS AD setup */
            snprintf(state->emsg, sizeof(state->emsg),
                     "%s: gsskrb5_extract_authz_data_from_sec_context(): %s", function, emsg);
            slog(LOG_INFO, "%s: gsskrb5_extract_authz_data_from_sec_context(): %s", function, emsg);
         }
#else /* !HAVE_HEIMDAL_KERBEROS */
         type_id.value = (void *)"mspac";
         type_id.length = strlen((char *)type_id.value);
#define KRB5PACLOGONINFO 1
         major_status = gss_map_name_to_any(&minor_status, client_name, KRB5PACLOGONINFO, &type_id, (gss_any_t *)&pac);
         if (!gss_err_isset(major_status, minor_status, emsg, sizeof(emsg))){
            if (get_sids(request->auth->mdata.gssapi.sids, k5_context, pac, state)<0){
               slog(LOG_INFO, "%s: error in getting sids", function);
	    }
         } else {
	    /* Not a major error - maybe not a MS AD setup */
            snprintf(state->emsg, sizeof(state->emsg),
                     "%s: gss_map_name_to_any(): %s", function, emsg);
            slog(LOG_INFO, "%s: gss_map_name_to_any(): %s", function, emsg);
         }
         (void)gss_release_any_name_mapping(&minor_status, client_name, &type_id, (gss_any_t *)&pac);
         krb5_free_context(k5_context);
#endif /* !HAVE_HEIMDAL_KERBEROS */
   } else {
      snprintf(state->emsg, sizeof(state->emsg),
               "%s: krb5_init_context(): %s", function, emsg);
      slog(LOG_INFO, "%s: krb5_init_context(): %s", function, emsg);
   }
#endif /* HAVE_PAC */

      CLEAN_GSS_AUTH(client_name, server_name, server_creds);
      CLEAN_GSS_TOKEN(output_token);

      state->rcurrent = recv_gssapi_enc_ver;
      return state->rcurrent(s, request, state);
   }
   else if (major_status == GSS_S_CONTINUE_NEEDED) {
      CLEAN_GSS_AUTH(client_name, server_name, server_creds);

      /* expect a new token, with the version, length, etc. header. */
      state->rcurrent = recv_gssapi_auth_ver;

      /* presumably client is awaiting our response. */
      return NEGOTIATE_CONTINUE;
   }
   else {
      snprintf(state->emsg, sizeof(state->emsg),
               "%s: unknown gss major_status %d", function, major_status);

      CLEAN_GSS_AUTH(client_name, server_name, server_creds);
      return NEGOTIATE_ERROR;
   }

   /* NOTREACHED */
}

static negotiate_result_t
recv_gssapi_enc_ver(s, request, state)
   int s;
   request_t *request;
   negotiate_state_t *state;
{
   const char *function = "recv_gssapi_enc_ver()";
   unsigned char gssapi_enc_version;

   INIT(sizeof(gssapi_enc_version));
   CHECK(&gssapi_enc_version, request->auth, NULL);

   switch (gssapi_enc_version) {
      case SOCKS_GSSAPI_VERSION:
         break;

      default:
         snprintf(state->emsg, sizeof(state->emsg),
         "%s: unknown version on gssapi packet from client: %d",
         function, gssapi_enc_version);

         return NEGOTIATE_ERROR;
   }

   state->rcurrent = recv_gssapi_enc_type;
   return state->rcurrent(s, request, state);
}

/*
 * RFC1961: client request
 *
 *   +------+------+------+.......................+
 *   + ver  | mtyp | len  |       token           |
 *   +------+------+------+.......................+
 *   + 0x01 | 0x02 | 0x02 | up to 2^16 - 1 octets |
 *   +------+------+------+.......................+
 *
 */
static negotiate_result_t
recv_gssapi_enc_type(s, request, state)
   int s;
   request_t *request;
   negotiate_state_t *state;
{
   const char *function = "recv_gssapi_enc_type()";
   unsigned char gssapi_enc_type;

   INIT(sizeof(gssapi_enc_type));
   CHECK(&gssapi_enc_type, request->auth, NULL);

   switch (gssapi_enc_type) {
      case SOCKS_GSSAPI_INTEGRITY:
      case SOCKS_GSSAPI_CONFIDENTIALITY:
         break;

      case SOCKS_GSSAPI_PERMESSAGE:
         snprintf(state->emsg, sizeof(state->emsg),
          "%s: unsupported per message encryption on gssapi packet from client",
          function);

         return NEGOTIATE_ERROR;

      default:
         snprintf(state->emsg, sizeof(state->emsg),
         "%s: unknown type on gssapi packet from client: %d",
         function, gssapi_enc_type);

         return NEGOTIATE_ERROR;
   }

   state->rcurrent = recv_gssapi_enc_len;
   return state->rcurrent(s, request, state);
}

static negotiate_result_t
recv_gssapi_enc_len(s, request, state)
   int s;
   request_t *request;
   negotiate_state_t *state;
{

   INIT(sizeof(state->gssapitoken_len));
   CHECK(&state->gssapitoken_len, request->auth, NULL);

   state->gssapitoken_len
   = ntohs((short)state->gssapitoken_len);

   state->rcurrent = recv_gssapi_enc_token;
   return state->rcurrent(s, request, state);
}

static negotiate_result_t
recv_gssapi_enc_token(s, request, state)
   int s;
   request_t *request;
   negotiate_state_t *state;
{
   const char *function = "recv_gssapi_enc_token()";
   OM_uint32  minor_status, major_status;
   gss_buffer_desc input_token, output_token = GSS_C_EMPTY_BUFFER;
   unsigned short  unsigned_short;
   ssize_t rc;
   size_t buflen;
   unsigned char buf[GSSAPI_HLEN + MAXGSSAPITOKENLEN], gss_enc, gss_server_enc;
   int conf_state, do_rfc1961;
   char emsg[1024];

   INIT(state->gssapitoken_len);

   input_token.length = state->gssapitoken_len;
   input_token.value  = buf;

   CHECK(input_token.value, request->auth, NULL);

   if (state->gssapitoken_len == 1 /* clear text encryption selection packet */
   &&  request->auth->mdata.gssapi.encryption.nec)
      do_rfc1961 = 0; /*
                       * It seems the NEC reference implementation does not do
                       * this right.  Try to support it if configured as such.
                       */
   else
      do_rfc1961 = 1;

   slog(LOG_DEBUG, "%s: rule assumes client uses %s exchange",
        function, do_rfc1961? "rfc1961 encrypted" : "NEC-style unencrypted");

   if (do_rfc1961) {
      major_status = gss_unwrap(&minor_status,
                                request->auth->mdata.gssapi.state.id,
                                &input_token,
                                &output_token,
                                0,
                                GSS_C_QOP_DEFAULT);

      if (gss_err_isset(major_status, minor_status, emsg, sizeof(emsg))) {
         snprintf(state->emsg, sizeof(state->emsg), "%s: gss_unwrap(): %s",
                  function, emsg);

         CLEAN_GSS_TOKEN(output_token);
         return NEGOTIATE_ERROR;
      }

      if (output_token.length != 1) {
         snprintf(state->emsg, sizeof(state->emsg),
                  "%s: gssapi encryption unwrapped length is wrong: expected 1 "
                  "but is %lu.  If the client is a NEC-based client you can "
                  "try enabling NEC (NOTE: unencrypted exchange) "
                  "client-compatibility for this client-rule",
                  function, (unsigned long)output_token.length);

         CLEAN_GSS_TOKEN(output_token);
         return NEGOTIATE_ERROR;
      }

      SASSERTX(output_token.length == sizeof(gss_enc));
      memcpy(&gss_enc, output_token.value, output_token.length);
      CLEAN_GSS_TOKEN(output_token);
   }
   else {
      SASSERTX(input_token.length == sizeof(gss_enc));
      memcpy(&gss_enc, input_token.value, input_token.length);
   }

   if ((gss_enc == SOCKS_GSSAPI_CLEAR
    && !request->auth->mdata.gssapi.encryption.clear)
   ||  (gss_enc == SOCKS_GSSAPI_INTEGRITY
    && !request->auth->mdata.gssapi.encryption.integrity)
   ||  (gss_enc == SOCKS_GSSAPI_CONFIDENTIALITY
    && !request->auth->mdata.gssapi.encryption.confidentiality)
   ||  (gss_enc == SOCKS_GSSAPI_PERMESSAGE) ) {
      /*
       * enforce server encryption type, regardless of what client offers.
       */

      snprintf(state->emsg, sizeof(state->emsg),
      "the client requests different authentication from what we offer.  "
      "Client requests %s (0x%x), but we offer: clear/%d, integrity/%d, "
      "confidentiality/%d, per message/%d",
      gssapiprotection2string(gss_enc),
      gss_enc,
      request->auth->mdata.gssapi.encryption.clear,
      request->auth->mdata.gssapi.encryption.integrity,
      request->auth->mdata.gssapi.encryption.confidentiality,
      request->auth->mdata.gssapi.encryption.permessage);

      return NEGOTIATE_ERROR;
   }

   gss_server_enc = gss_enc;

   slog(LOG_DEBUG, "%s: using %s protection (%d)",
        function, gssapiprotection2string(gss_server_enc), gss_server_enc);

   if (do_rfc1961) {
      input_token.length = 1;
      input_token.value  = &gss_server_enc;

      major_status = gss_wrap(&minor_status,
                              request->auth->mdata.gssapi.state.id,
                              GSS_REQ_INT,
                              GSS_C_QOP_DEFAULT,
                              &input_token,
                              &conf_state,
                              &output_token);

      if (gss_err_isset(major_status, minor_status, emsg, sizeof(emsg))) {
         snprintf(state->emsg, sizeof(state->emsg), "%s: gss_wrap(): %s",
                  function, emsg);

         CLEAN_GSS_TOKEN(output_token);
         return NEGOTIATE_ERROR;
      }
   }
   else {
      output_token.length  = 1;
      output_token.value   = &gss_server_enc;
   }

   request->auth->mdata.gssapi.state.protection = gss_server_enc;

   /*
    * RFC1961: server reply
    *
    *   +------+------+------+.......................+
    *   + ver  | mtyp | len  |       token           |
    *   +------+------+------+.......................+
    *   + 0x01 | 0x02 | 0x02 | up to 2^16 - 1 octets |
    *   +------+------+------+.......................+
    *
    */

   SASSERTX(GSSAPI_HLEN + output_token.length <= sizeof(buf));

   buflen = 0;
   buf[buflen++] = SOCKS_GSSAPI_VERSION;
   buf[buflen++] = SOCKS_GSSAPI_ENCRYPTION;

   unsigned_short = htons(output_token.length);
   memcpy(&buf[buflen], &unsigned_short, sizeof(unsigned_short));
   buflen += sizeof(unsigned_short);

   memcpy(&buf[buflen], output_token.value, output_token.length);
   buflen += (output_token.length);

   if ((rc = socks_sendton(s, buf, buflen, 0, 0, NULL, 0, NULL, request->auth))
   != (ssize_t)buflen) {
      snprintf(state->emsg, sizeof(state->emsg),
               "socks_sendton() buf: wrote %ld out of %lu bytes: %s",
                (long)rc,
                (unsigned long)buflen,
                strerror(errno));

      if (do_rfc1961)
         CLEAN_GSS_TOKEN(output_token);

      return NEGOTIATE_ERROR;
   }

   if (do_rfc1961)
      CLEAN_GSS_TOKEN(output_token);

   if (gss_server_enc)
      state->rcurrent = recv_gssapi_packet;
   else
      /*
       * Continue with clear text communication.
       * Not RFC compliant but useful if authentication only is required.
       */
      state->rcurrent = recv_sockspacket;

   return NEGOTIATE_CONTINUE; /* presumably client is awaiting our response. */
}

/*
 * RFC1961: client request
 *
 *   +------+------+------+.......................+
 *   + ver  | mtyp | len  |       token           |
 *   +------+------+------+.......................+
 *   + 0x01 | 0x03 | 0x02 | up to 2^16 - 1 octets |
 *   +------+------+------+.......................+
 *
 */
static negotiate_result_t
recv_gssapi_packet(s, request, state)
   int s;
   request_t *request;
   negotiate_state_t *state;

{

   state->rcurrent = recv_gssapi_packet_ver;
   return state->rcurrent(s, request, state);
}

static negotiate_result_t
recv_gssapi_packet_ver(s, request, state)
   int s;
   request_t *request;
   negotiate_state_t *state;
{
   const char *function = "recv_gssapi_packet_ver()";
   unsigned char gssapi_packet_version;

   INIT(sizeof(gssapi_packet_version));
   CHECK(&gssapi_packet_version, request->auth, NULL);

   switch (gssapi_packet_version) {
      case SOCKS_GSSAPI_VERSION:
         break;

      default:
         snprintf(state->emsg, sizeof(state->emsg),
         "%s: unknown version on gssapi packet from client: %d",
         function, gssapi_packet_version);

         return NEGOTIATE_ERROR;
   }

   state->rcurrent = recv_gssapi_packet_type;
   return state->rcurrent(s, request, state);
}

static negotiate_result_t
recv_gssapi_packet_type(s, request, state)
   int s;
   request_t *request;
   negotiate_state_t *state;
{
   const char *function = "recv_gssapi_packet_type()";
   unsigned char gssapi_enc_type;

   INIT(sizeof(gssapi_enc_type));
   CHECK(&gssapi_enc_type, request->auth, NULL);

   switch (gssapi_enc_type) {
      case SOCKS_GSSAPI_PACKET:
         break;

      case SOCKS_GSSAPI_ENCRYPTION:
      case SOCKS_GSSAPI_AUTHENTICATION:
         snprintf(state->emsg, sizeof(state->emsg),
         "%s: received out of sequence exchange from client, type: %d",
         function, gssapi_enc_type);

         return NEGOTIATE_ERROR;

      default:
         snprintf(state->emsg, sizeof(state->emsg),
         "%s: unknown type on gssapi packet from client: %d",
         function, gssapi_enc_type);

         return NEGOTIATE_ERROR;
   }

   state->rcurrent = recv_gssapi_packet_len;
   return state->rcurrent(s, request, state);
}

static negotiate_result_t
recv_gssapi_packet_len(s, request, state)
   int s;
   request_t *request;
   negotiate_state_t *state;
{

   INIT(sizeof(state->gssapitoken_len));
   CHECK(&state->gssapitoken_len, request->auth, NULL);

   state->gssapitoken_len
   = ntohs((short)state->gssapitoken_len);

   state->rcurrent = recv_gssapi_packet_token;
   return state->rcurrent(s, request, state);
}

static negotiate_result_t
recv_gssapi_packet_token(s, request, state)
   int s;
   request_t *request;
   negotiate_state_t *state;
{
   const char *function = "recv_gssapi_packet_token()";
   OM_uint32        minor_status, major_status      = GSS_S_COMPLETE;
   gss_buffer_desc                input_token       = GSS_C_EMPTY_BUFFER;
   gss_buffer_desc                output_token      = GSS_C_EMPTY_BUFFER;
   unsigned char                  *data;
   unsigned char                  buf[GSSAPI_HLEN + MAXGSSAPITOKENLEN];
   char                           emsg[1024];
   int                            conf_state, offset;

   INIT(state->gssapitoken_len);

   input_token.length = state->gssapitoken_len;
   input_token.value  = buf;

   CHECK(input_token.value, request->auth, NULL);

   conf_state = (request->auth->mdata.gssapi.state.protection
   == GSSAPI_CONFIDENTIALITY) ? GSS_REQ_CONF : GSS_REQ_INT;

   major_status
   = gss_unwrap(&minor_status,
                request->auth->mdata.gssapi.state.id,
                &input_token, &output_token, &conf_state, GSS_C_QOP_DEFAULT);

   if (gss_err_isset(major_status, minor_status, emsg, sizeof(emsg))) {
      snprintf(state->emsg, sizeof(state->emsg),
      "%s: gss_unwrap(): %s", function, emsg);

      return NEGOTIATE_ERROR;
   }

   offset = 0;
   data   = output_token.value;

   if (offset + sizeof(request->version) > output_token.length) {
      snprintf(state->emsg, sizeof(state->emsg),
      "%s: token has short length: %lu",
      function, (unsigned long)output_token.length);

      CLEAN_GSS_TOKEN(output_token);
      return NEGOTIATE_ERROR;
   }

   memcpy(&request->version, &data[offset], sizeof(request->version));
   if (request->version != PROXY_SOCKS_V5) {
      snprintf(state->emsg, sizeof(state->emsg),
      "%s: invalid socks version %d in request", function, request->version);

      CLEAN_GSS_TOKEN(output_token);
      return NEGOTIATE_ERROR;
   }
   offset += sizeof(request->version);

   if (offset + sizeof(request->command) > output_token.length) {
      snprintf(state->emsg, sizeof(state->emsg),
      "%s: token has short length: %lu",
      function, (unsigned long)output_token.length);

      CLEAN_GSS_TOKEN(output_token);
      return NEGOTIATE_ERROR;
   }
   memcpy(&request->command, &data[offset], sizeof(request->command));
   offset += sizeof(request->command);

   switch (request->command) {
      case SOCKS_BIND:
      case SOCKS_CONNECT:
         request->protocol = SOCKS_TCP;
         break;

       case SOCKS_UDPASSOCIATE:
         request->protocol = SOCKS_UDP;
         break;

       default:
         snprintf(state->emsg, sizeof(state->emsg),
         "%s: unknown command received from client: %d",
         function, request->command);

         CLEAN_GSS_TOKEN(output_token);
         return NEGOTIATE_ERROR;
   }

   if (offset + sizeof(request->flag) > output_token.length) {
      CLEAN_GSS_TOKEN(output_token);
      return NEGOTIATE_ERROR;
   }
   memcpy(&request->flag, &data[offset], sizeof(request->flag));
   offset += sizeof(request->flag);

   if (mem2sockshost(&request->host, &data[offset],
   output_token.length - offset, request->version) == NULL) {
      CLEAN_GSS_TOKEN(output_token);
      return NEGOTIATE_ERROR;
   }

   CLEAN_GSS_TOKEN(output_token);

   /* Negotiation finished => set connection state as protected */
   if (request->auth->mdata.gssapi.state.protection)
      request->auth->mdata.gssapi.state.wrap = 1;

   return NEGOTIATE_FINISHED;
}

#endif /* HAVE_GSSAPI */
