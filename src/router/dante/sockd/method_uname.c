/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2005, 2008, 2009, 2010,
 *               2011, 2012, 2013, 2014, 2019, 2020
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
"$Id: method_uname.c,v 1.114.4.2.6.6 2020/11/11 17:02:28 karls Exp $";

static negotiate_result_t
recv_unamever(int s, request_t *request, negotiate_state_t *state);

static negotiate_result_t
recv_ulen(int s, request_t *request, negotiate_state_t *state);

static negotiate_result_t
recv_uname(int s, request_t *request, negotiate_state_t *state);

static negotiate_result_t
recv_plen(int s, request_t *request, negotiate_state_t *state);

static negotiate_result_t
recv_passwd(int s, request_t *request, negotiate_state_t *state);

static int
passworddbisunique(void);
/*
 * If it's possible for us to fail username/password authentication
 * on one rule, and succeed at another, returns false.
 * Otherwise returns the unique authmethod that would be used.
 */


negotiate_result_t
method_uname(s, request, state)
   int s;
   request_t *request;
   negotiate_state_t *state;

{

   state->rcurrent = recv_unamever;
   return state->rcurrent(s, request, state);
}

static int
passworddbisunique(void)
{
   const char *function = "passworddbisunique()";
   int rc;

   if (methodisset(AUTHMETHOD_UNAME, sockscf.smethodv, sockscf.smethodc)) {
#if HAVE_PAM
     if (methodisset(AUTHMETHOD_PAM_USERNAME,
                     sockscf.smethodv,
                     sockscf.smethodc))
         rc = 0;
      else
#endif /* HAVE_PAM */

#if HAVE_BSDAUTH
     if (methodisset(AUTHMETHOD_BSDAUTH, sockscf.smethodv, sockscf.smethodc))
         rc = 0;
      else
#endif /* HAVE_BSDAUTH */

#if HAVE_LDAP
     if (methodisset(AUTHMETHOD_LDAPAUTH, sockscf.smethodv, sockscf.smethodc))
         rc = 0;
      else
#endif /* HAVE_LDAP */
         rc = AUTHMETHOD_UNAME;
   }

#if HAVE_PAM
   else if (methodisset(AUTHMETHOD_PAM_USERNAME,
                        sockscf.smethodv,
                        sockscf.smethodc)) {
      if (*sockscf.state.pamservicename == NUL)
         rc = 0;
      else if (methodisset(AUTHMETHOD_UNAME,
                           sockscf.smethodv,
                           sockscf.smethodc))
         rc = 0;
#if HAVE_BSDAUTH
      else if (methodisset(AUTHMETHOD_BSDAUTH,
                           sockscf.smethodv,
                           sockscf.smethodc))
         rc = 0;
#endif /* HAVE_BSDAUTH */
#if HAVE_LDAP
      else if (methodisset(AUTHMETHOD_LDAPAUTH,
                           sockscf.smethodv,
                           sockscf.smethodc))
         rc = 0;
#endif /* HAVE_LDAP */
      else
         rc = AUTHMETHOD_PAM_USERNAME;
   }
#endif /* HAVE_PAM */

#if HAVE_BSDAUTH
   else if (methodisset(AUTHMETHOD_BSDAUTH,
                        sockscf.smethodv,
                        sockscf.smethodc)) {
      if (sockscf.state.bsdauthstylename == NULL)
         rc = 0;
      else if (methodisset(AUTHMETHOD_UNAME,
                           sockscf.smethodv,
                           sockscf.smethodc))
         rc = 0;
#if HAVE_PAM
         else if (methodisset(AUTHMETHOD_PAM_USERNAME,
                              sockscf.smethodv,
                              sockscf.smethodc))
         rc = 0;
#endif /* HAVE_PAM */
#if HAVE_LDAP
         else if (methodisset(AUTHMETHOD_LDAPAUTH,
                              sockscf.smethodv,
                              sockscf.smethodc))
         rc = 0;
#endif /* HAVE_LDAP */
      else
         rc = AUTHMETHOD_BSDAUTH;
   }
#endif /* HAVE_BSDAUTH */

#if HAVE_LDAP
   else if (methodisset(AUTHMETHOD_LDAPAUTH,
                        sockscf.smethodv,
                        sockscf.smethodc)) {
      if (sockscf.state.ldapauthentication.ldapurl == NULL)
        rc = 0;
      else if (methodisset(AUTHMETHOD_UNAME,
                           sockscf.smethodv,
                           sockscf.smethodc))
         rc = 0;
#if HAVE_PAM
      else if (methodisset(AUTHMETHOD_PAM_USERNAME,
                              sockscf.smethodv,
                              sockscf.smethodc))
         rc = 0;
#endif /* HAVE_PAM */
#if HAVE_BSDAUTH
      else if (methodisset(AUTHMETHOD_BSDAUTH,
                              sockscf.smethodv,
                              sockscf.smethodc))
         rc = 0;
#endif /* HAVE_BSDAUTH */
      else
         rc = AUTHMETHOD_LDAPAUTH;
   }
#endif /* HAVE_LDAP */
   else {
      slog(LOG_DEBUG, "%s: no password-based methods configured", function);
      rc = 0;
   }

   slog(LOG_DEBUG, "%s: returning %d", function, rc);
   return rc;
}

static negotiate_result_t
recv_unamever(s, request, state)
   int s;
   request_t *request;
   negotiate_state_t *state;
{
   const char *function = "recv_unamever()";

   INIT(sizeof(request->auth->mdata.uname.version));
   CHECK(&request->auth->mdata.uname.version, request->auth, NULL);

   switch (request->auth->mdata.uname.version) {
      case SOCKS_UNAMEVERSION:
         break;

      default:
         snprintf(state->emsg, sizeof(state->emsg),
                  "%s: unknown %s version on username packet from client: %d",
                  function,
                  proxyprotocol2string(request->version),
                  request->auth->mdata.uname.version);

         return NEGOTIATE_ERROR;
   }

   state->rcurrent = recv_ulen;
   return state->rcurrent(s, request, state);
}

static negotiate_result_t
recv_ulen(s, request, state)
   int s;
   request_t *request;
   negotiate_state_t *state;
{

   INIT(sizeof(*request->auth->mdata.uname.name));
   CHECK(request->auth->mdata.uname.name, request->auth, NULL);

   /* LINTED conversion from 'int' may lose accuracy */
   OCTETIFY(*request->auth->mdata.uname.name);

   state->rcurrent = recv_uname;
   return state->rcurrent(s, request, state);
}

static negotiate_result_t
recv_uname(s, request, state)
   int s;
   request_t *request;
   negotiate_state_t *state;
{
   /* in the protocol, first byte gives length. */
   const size_t ulen = (size_t)*request->auth->mdata.uname.name;

   INIT(ulen);
   CHECK(request->auth->mdata.uname.name + 1, request->auth, NULL);

   /* convert to string. */
   memmove(request->auth->mdata.uname.name,
           request->auth->mdata.uname.name + 1,
           ulen);
   request->auth->mdata.uname.name[ulen] = NUL;

   state->rcurrent = recv_plen;
   return state->rcurrent(s, request, state);
}

static negotiate_result_t
recv_plen(s, request, state)
   int s;
   request_t *request;
   negotiate_state_t *state;
{

   INIT(sizeof(*request->auth->mdata.uname.password));
   CHECK(request->auth->mdata.uname.password, request->auth, NULL);

   /* LINTED conversion from 'int' may lose accuracy */
   OCTETIFY(*request->auth->mdata.uname.password);

   state->rcurrent = recv_passwd;
   return state->rcurrent(s, request, state);
}

static negotiate_result_t
recv_passwd(s, request, state)
   int s;
   request_t *request;
   negotiate_state_t *state;
{
/*   const char *function = "recv_passwd()"; */
   const size_t plen = (size_t)*request->auth->mdata.uname.password;
   sendto_info_t sendtoflags;
   unsigned char response[1 /* version. */ + 1 /* status.   */];
   int isunique;

   INIT(plen);
   CHECK(request->auth->mdata.uname.password + 1, request->auth, NULL);

   /* convert to C string. */
   memmove(request->auth->mdata.uname.password,
           request->auth->mdata.uname.password + 1,
           plen);
   request->auth->mdata.uname.password[plen] = NUL;

   /*
    * Very sadly we can't always do checking of the username/password here
    * since we don't know what authentication to use yet.  It could
    * be username, but it could also be PAM, or some future method.
    * It depends on what the socks request is.  We therefor would have
    * liked to give the client success status back no matter what
    * the username/password is, and later deny the connection if need be
    * after we have gotten the request.
    *
    * That however creates problems with clients that, naturally, cache
    * the wrong username/password if they get success.
    * We therefor check if we have a unique passworddb to use, and if so,
    * check the password here so we can return an immediate error to client.
    * This we can do because if the passworddb is unique there is
    * no chance of the result varying based to the client's request.
    *
    * If the database is not unique, we go with returning a success at
    * this point, and deny it later if need be, even though this might
    * create problems for the clients that cache the result.
   */
   response[UNAME_VERSION] = request->auth->mdata.uname.version;
   switch ((isunique = passworddbisunique())) {
      case 0:
         /*
          * not unique.  Return ok now, and check correct db later,
          * when we know what rules to use and what "correct" is.
          */
         response[UNAME_STATUS] = (unsigned char)UNAME_STATUS_ISOK;
         break;

#if HAVE_PAM
      case AUTHMETHOD_PAM_ANY:
      case AUTHMETHOD_PAM_ADDRESS:
      case AUTHMETHOD_PAM_USERNAME: {
         /*
          * it's a union, make a copy before moving into pam object.
          */
         const authmethod_uname_t uname = request->auth->mdata.uname;

         request->auth->method = isunique;

         STRCPY_ASSERTLEN(request->auth->mdata.pam.servicename,
                          sockscf.state.pamservicename);

         STRCPY_ASSERTSIZE(request->auth->mdata.pam.name, uname.name);

         STRCPY_ASSERTSIZE(request->auth->mdata.pam.password, uname.password);
         break;
      }
#endif /* HAVE_PAM */

#if HAVE_BSDAUTH
      case AUTHMETHOD_BSDAUTH: {
         /*
          * it's a union, make a copy before moving into bsd object.
          */
         const authmethod_uname_t uname = request->auth->mdata.uname;

         request->auth->method = AUTHMETHOD_BSDAUTH;
         if (sockscf.state.bsdauthstylename != NULL)
            STRCPY_ASSERTLEN(request->auth->mdata.bsd.style,
                             (const char *)sockscf.state.bsdauthstylename);
         else
            request->auth->mdata.bsd.style[0] = NUL;

         STRCPY_ASSERTSIZE(request->auth->mdata.bsd.name, uname.name);

         STRCPY_ASSERTSIZE(request->auth->mdata.bsd.password,
                           uname.password);
         break;
      }
#endif /* HAVE_BSDAUTH */

#if HAVE_LDAP
      case AUTHMETHOD_LDAPAUTH: {
         /*
          * it's a union, make a copy before moving into ldap object.
          */
         const authmethod_uname_t uname = request->auth->mdata.uname;

         request->auth->method = AUTHMETHOD_LDAPAUTH;

         STRCPY_ASSERTSIZE(request->auth->mdata.ldap.name,
                           uname.name);

         STRCPY_ASSERTSIZE(request->auth->mdata.ldap.password,
                           uname.password);

         /*
          * Use global LDAP settigs until we know what socks-rule
          * the request will match.
          */
         request->auth->mdata.ldap.ldapauthentication
         = sockscf.state.ldapauthentication;

         break;
      }
#endif /* HAVE_LDAP */

      case AUTHMETHOD_UNAME:
         break;

      default:
         SERRX(passworddbisunique());
   }

   if (isunique) {
      struct sockaddr_storage src, dst;

      sockshost2sockaddr(&state->src, &src);
      sockshost2sockaddr(&state->dst, &dst);

      if (accesscheck(s,
                      request->auth,
                      &src,
                      &dst,
                      state->emsg,
                      sizeof(state->emsg)))
         response[UNAME_STATUS] = (unsigned char)UNAME_STATUS_ISOK;
      else
         response[UNAME_STATUS] = (unsigned char)UNAME_STATUS_ISNOK;
   }

   bzero(&sendtoflags, sizeof(sendtoflags));
   sendtoflags.side = INTERNALIF;

   if (socks_sendton(s,
                     response,
                     sizeof(response),
                     0,
                     0,
                     NULL,
                     0,
                     &sendtoflags,
                     request->auth) != sizeof(response))
      return NEGOTIATE_ERROR;

   if (response[UNAME_STATUS] == (unsigned char)UNAME_STATUS_ISOK) {
      state->rcurrent = recv_sockspacket;

      /* presumably client is awaiting our response. */
      return NEGOTIATE_CONTINUE;
   }

   /* else; failed authentication. */
   return NEGOTIATE_ERROR;
}
