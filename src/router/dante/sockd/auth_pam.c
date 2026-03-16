/*
 * Copyright (c) 2001, 2002, 2004, 2005, 2006, 2008, 2009, 2010, 2011, 2012,
 *               2013
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
 * Based on code originally from
 * Patrick Bihan-Faou, MindStep Corporation, patrick@mindstep.com.
 */

#include "common.h"

#if HAVE_PAM

static const char rcsid[] =
"$Id: auth_pam.c,v 1.100 2013/10/27 15:24:42 karls Exp $";

static int
pam_conversation(int msgc, const struct pam_message **msgv,
                 struct pam_response **rspv, void *authdata);
/*
 * Called by the pam system to fetch username and password info.
 */


int
pam_passwordcheck(s, src, dst, auth, emsg, emsgsize)
   int s;
   const struct sockaddr_storage *src, *dst;
   const authmethod_pam_t *auth;
   char *emsg;
   size_t emsgsize;
{
   const char *function = "pam_passwordcheck()";
   authmethod_pam_t authdata = *auth;
   struct pam_conv pamconv;
   pam_handle_t *pamh;
   size_t i;
   char srcstr[MAXSOCKADDRSTRING], visbuf[MAXNAMELEN * 4];
   int rc;

   /*
    * unfortunately we can not set password here, that needs to be set
    * "from a module", i.e. in the conversion function, at least with
    * one Linux pam implementation.
    */
   struct {
      int         item;
      const char  *itemname;
      const void  *value;
      int         printable;

   } pamval[] = {
      { (int)PAM_CONV,  "PAM_CONV",  &pamconv,                              0 },
      { (int)PAM_RHOST, "PAM_RHOST",
                  src == NULL ?
                     "" : sockaddr2string2(src, 0, srcstr, sizeof(srcstr)), 1 },
      { (int)PAM_USER,  "PAM_USER",  (*auth->name == NUL) ?
                              DEFAULT_PAM_USER : (const char *)auth->name,  1 },
      { (int)PAM_RUSER, "PAM_RUSER", DEFAULT_PAM_RUSER,                     1 },
   };

   slog(LOG_DEBUG, "%s: src %s, user \"%s\", servicename \"%s\", emsgsize %ld",
        function,
        src == NULL ? "N/A" : sockaddr2string(src, NULL, 0),
        str2vis((const char *)auth->name,
                strlen((const char *)auth->name),
                visbuf,
                sizeof(visbuf)),
        auth->servicename,
        (long)emsgsize);

   if (src == NULL) {
      snprintf(emsg, emsgsize, "%s: NULL src address: not supported", function);
      return  -1;
   }
   /*
    * Note: we can not save the state of pam after pam_start(3), as
    * e.g. Solaris 5.11 pam does not allow setting PAM_SERVICE
    * except during pam_start(3), while we may need to change it
    * depending on the client/rule.
    * Some Linux pam-implementations on the other hand can enter
    * some sort of busy-loop if we don't call pam_end(3) ever so
    * often.
    *
    * Therefor, disregard all possible optimization stuff for now and
    * call pam_start(3) and pam_end(3) every time.
    */

   pamconv.conv        = pam_conversation;
   pamconv.appdata_ptr = &authdata;

   sockd_priv(SOCKD_PRIV_PAM, PRIV_ON);
   rc = pam_start(auth->servicename, NULL, &pamconv, &pamh);
   sockd_priv(SOCKD_PRIV_PAM, PRIV_OFF);

   if (rc != (int)PAM_SUCCESS) {
      snprintf(emsg, emsgsize, "pam_start() failed: %s",
               pam_strerror(pamh, rc));

      return -1;
   }

   for (i = 0; i < ELEMENTS(pamval); ++i) {
      if (pamval[i].printable) {
         str2vis((const char *)pamval[i].value,
                 strlen((const char *)pamval[i].value),
                 visbuf,
                 sizeof(visbuf));

         slog(LOG_DEBUG, "%s: setting item \"%s\" to value \"%s\"",
              function, pamval[i].itemname, visbuf);
      }
      else
         slog(LOG_DEBUG, "%s: setting item %s", function, pamval[i].itemname);

      if ((rc = pam_set_item(pamh, pamval[i].item, pamval[i].value))
      != (int)PAM_SUCCESS) {
         snprintf(emsg, emsgsize, "pam_set_item(%s) to \"%s\" failed: %s",
                  pamval[i].itemname, visbuf, pam_strerror(pamh, rc));

         pam_end(pamh, rc);
         return -1;
      }
   }

   sockd_priv(SOCKD_PRIV_PAM, PRIV_ON);

   if ((rc = pam_authenticate(pamh, 0)) != (int)PAM_SUCCESS) {
      sockd_priv(SOCKD_PRIV_PAM, PRIV_OFF);

      slog(LOG_DEBUG, "%s: pam_authenticate() failed: %s",
           function, pam_strerror(pamh, rc));

      snprintf(emsg, emsgsize, "pam_authenticate() for user \"%s\" failed: %s",
               *auth->name == NUL ?
                  "<no user specified>" : str2vis((const char *)auth->name,
                                               strlen((const char *)auth->name),
                                                  visbuf,
                                                  sizeof(visbuf)),
               pam_strerror(pamh, rc));

      pam_end(pamh, rc);
      return -1;
   }

   /* LINTED passing const, expecting non-const (PAM_SILENT) */
   rc = pam_acct_mgmt(pamh, PAM_SILENT);

   sockd_priv(SOCKD_PRIV_PAM, PRIV_OFF);

   if (rc != PAM_SUCCESS) {
      slog(LOG_DEBUG, "%s: pam_acct_mgmt() failed: %s",
           function, pam_strerror(pamh, rc));

      snprintf(emsg, emsgsize, "pam_acct_mgmt(): %s", pam_strerror(pamh, rc));

      pam_end(pamh, rc);
      return -1;
   }

   if ((rc = pam_end(pamh, rc)) != (int)PAM_SUCCESS)
      swarnx("%s: strange ... pam_end() failed: %s",
             function, pam_strerror(pamh, rc));

   slog(LOG_DEBUG, "%s: pam authentication succeeded", function);
   return 0;
}

static int
pam_conversation(msgc, msgv, rspv, authdata)
   int msgc;
   const struct pam_message **msgv;
   struct pam_response **rspv;
   void *authdata;
{
   const authmethod_pam_t *auth = authdata;
   const char *function = "pam_conversation()";
   int i, rc;

   if (rspv == NULL || msgv == NULL || auth == NULL || msgc < 1) {
      swarnx("%s: called with invalid/unexpected input", function);
      return (int)PAM_CONV_ERR;
   }

   if (((*rspv) = malloc(msgc * sizeof(struct pam_response))) == NULL) {
      swarn("%s: malloc(%d * %lu)",
            function, msgc, (unsigned long)sizeof(struct pam_response));

      return (int)PAM_CONV_ERR;
   }

   /* initialize all to NULL so we can easily free on error. */
   for (i = 0; i < msgc; ++i) {
      (*rspv)[i].resp_retcode = 0; /* according to sun not used, should be 0. */
      (*rspv)[i].resp         = NULL;
   }

   rc = (int)PAM_SUCCESS;
   for (i = 0; i < msgc; ++i) {
      slog(LOG_DEBUG, "%s: msg_style = %d", function, msgv[i]->msg_style);

      switch(msgv[i]->msg_style) {
         case PAM_PROMPT_ECHO_OFF:
            if (((*rspv)[i].resp = strdup((const char *)auth->password))
            == NULL) {
               swarn("%s: strdup() of password, length %lu, failed",
                     function,
                     (unsigned long)strlen((const char *)auth->password));

               rc = (int)PAM_CONV_ERR;
            }
            break;

         case PAM_ERROR_MSG:
            slog(LOG_INFO, "%s: got a pam error msg: %s",
                 function, msgv[i]->msg);
            break;

         case PAM_TEXT_INFO:
            /*
             * not expecting this, and where it has been seen (some versions
             * of FreeBSD), the string has been empty.
             * Seen it on Linux also.  Don't know what it's for.
             */
            slog(LOG_DEBUG, "%s: got unexpected PAM_TEXT_INFO: \"%s\"",
                 function, msgv[i]->msg);
            break;

         default:
            swarnx("%s: unknown msg_style %d, ignored ...",
                   function, msgv[i]->msg_style);
            break;
      }
   }

   if (rc != (int)PAM_SUCCESS) { /* failed; free the memory ourselves */
      for (i = 0; i < msgc; ++i)
         free((*rspv)[i].resp);

      free(*rspv);
   }

   return rc;
}

#endif /* HAVE_PAM */
