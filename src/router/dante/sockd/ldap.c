/*
 * Copyright (c) 2009, 2010, 2011, 2012, 2013
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
 * $Id: ldap.c,v 1.15.4.1 2014/05/09 15:31:10 michaels Exp $
 */

 /*
  * This code was contributed by
  * Markus Moeller (markus_moeller at compuserve.com).
  */

#include "common.h"

static const char rcsid[] =
"$Id: ldap.c,v 1.15.4.1 2014/05/09 15:31:10 michaels Exp $";

const char module_ldap_version[] =
"$Id: ldap.c,v 1.15.4.1 2014/05/09 15:31:10 michaels Exp $";

#if HAVE_LDAP

#define LDAP_DEPRECATED 1
#if HAVE_LDAP_REBIND_FUNCTION
#define LDAP_REFERRALS
#endif /* HAVE_LDAP_REBIND_FUNCTION */
#if HAVE_KRB5_H
#include <krb5.h>
#endif /* HAVE_KRB5_H */
#if !HAVE_COM_ERR_IN_KRB5
#if HAVE_COM_ERR_H
#include <com_err.h>
#elif HAVE_ET_COM_ERR_H
#include <et/com_err.h>
#endif /* HAVE_COM_ERR_H */
#endif /* !HAVE_COM_ERR_IN_KRB5 */
#if !HAVE_ERROR_MESSAGE && HAVE_KRB5_GET_ERR_TEXT
#define error_message(code) krb5_get_err_text(kparam.context, code)
#elif !HAVE_ERROR_MESSAGE && HAVE_KRB5_GET_ERROR_MESSAGE
#define error_message(code) krb5_get_error_message(kparam.context, code)
#elif !HAVE_ERROR_MESSAGE
   static char err_code[17];
   const char *KRB5_CALLCONV
   error_message(long code) {
      snprintf(err_code, 16, "%ld", code);
      return err_code;
   }
#endif /* !HAVE_ERROR_MESSAGE */

/*
#if HAVE_NAS_KERBEROS
#include <ibm_svc/krb5_svc.h>
const char *KRB5_CALLCONV error_message(long code) {
 char *msg = NULL;
 krb5_svc_get_msg(code, &msg);
 return msg;
}
#endif
*/

#if HAVE_LBER_H
#include <lber.h>
#endif /* HAVE_LBER_H */
#if HAVE_LDAP_H
#include <ldap.h>
#elif HAVE_MOZLDAP_LDAP_H
#include <mozldap/ldap.h>
#endif /* HAVE_MOZLDAP_LDAP_H */
#if HAVE_SASL_H
#include <sasl.h>
#elif HAVE_SASL_SASL_H
#include <sasl/sasl.h>
#elif HAVE_SASL_DARWIN
typedef struct sasl_interact {
   unsigned long id;           /* same as client/user callback ID.            */
   const char *challenge;      /* presented to user (e.g. OTP challenge).     */
   const char *prompt;         /* presented to user (e.g. "Username: ").      */
   const char *defresult;      /* default result string.                      */
   const void *result;         /* set to point to result.                     */
   unsigned len;               /* set to length of result.                    */
} sasl_interact_t;
#define SASL_CB_USER         (0x4001) /* client user identity to login as.    */
#define SASL_CB_AUTHNAME     (0x4002) /* client authentication name.          */
#define SASL_CB_PASS         (0x4004) /* client passphrase-based secret.      */
#define SASL_CB_ECHOPROMPT   (0x4005) /* challenge and client entered result.*/
#define SASL_CB_NOECHOPROMPT (0x4006) /* challenge and client entered result.*/
#define SASL_CB_GETREALM     (0x4008) /* realm to attempt authentication in.  */
#define SASL_CB_LIST_END     (0)      /* end of list.                         */
#endif /* HAVE_SASL_DARWIN */

#ifndef LDAP_OPT_NETWORK_TIMEOUT
#define LDAP_OPT_NETWORK_TIMEOUT   (0)
#endif /* !LDAP_OPT_NETWORK_TIMEOUT */
#ifndef LDAP_X_OPT_CONNECT_TIMEOUT
#define LDAP_X_OPT_CONNECT_TIMEOUT (0)
#endif /* !LDAP_X_OPT_CONNECT_TIMEOUT */

void
ldapcachesetup(void)
{
   const char *function = "ldapcachesetup()";

   if ((sockscf.ldapfd = socks_mklock(SOCKD_SHMEMFILE, NULL, 0)) == -1)
      serr("%s: socks_mklock() failed to create shmemfile using base %s",
           function, SOCKD_SHMEMFILE);

}

int
ldapgroupmatches(username, userdomain, group, groupdomain, rule)
   const char *username;
   const char *userdomain;
   const char *group;
   const char *groupdomain;
   const struct rule_t *rule;
{
   const char *function = "ldapgroupmatches()";

   (void)rule;

   slog(LOG_DEBUG, "%s: user: %s user domain: %s", function, username,
      userdomain != NULL ? userdomain : "NULL");
   slog(LOG_DEBUG, "%s: group: %s group domain: %s", function, group,
      groupdomain != NULL ? groupdomain: "NULL");

   return 0;
}

void
cache_ldap_user(username, retval)
   const char *username;
   int retval;
{

   (void)username;
   (void)retval;
}

int
ldap_user_is_cached(username)
   const char *username;
{

   (void)username;

   return 0;
}

#endif /* HAVE_LDAP */
