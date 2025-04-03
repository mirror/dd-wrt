/*********************************************************************
 *
 * File        :  $Source: /cvsroot/ijbswa/current/wolfssl.c,v $
 *
 * Purpose     :  File with TLS/SSL extension. Contains methods for
 *                creating, using and closing TLS/SSL connections
 *                using wolfSSL.
 *
 * Copyright   :  Copyright (C) 2018-2024 by Fabian Keil <fk@fabiankeil.de>
 *                Copyright (C) 2020 Maxim Antonov <mantonov@gmail.com>
 *                Copyright (C) 2017 Vaclav Svec. FIT CVUT.
 *
 *                This program is free software; you can redistribute it
 *                and/or modify it under the terms of the GNU General
 *                Public License as published by the Free Software
 *                Foundation; either version 2 of the License, or (at
 *                your option) any later version.
 *
 *                This program is distributed in the hope that it will
 *                be useful, but WITHOUT ANY WARRANTY; without even the
 *                implied warranty of MERCHANTABILITY or FITNESS FOR A
 *                PARTICULAR PURPOSE.  See the GNU General Public
 *                License for more details.
 *
 *                The GNU General Public License should be included with
 *                this file.  If not, you can view it at
 *                http://www.gnu.org/copyleft/gpl.html
 *                or write to the Free Software Foundation, Inc., 59
 *                Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *********************************************************************/

#include <string.h>
#include <unistd.h>
#include <assert.h>
#include "config.h"

#include <wolfssl/options.h>
#include <wolfssl/openssl/x509v3.h>
#include <wolfssl/openssl/pem.h>
#include <wolfssl/ssl.h>
#include <wolfssl/wolfcrypt/coding.h>
#include <wolfssl/wolfcrypt/rsa.h>

#include "project.h"
#include "miscutil.h"
#include "errlog.h"
#include "encode.h"
#include "jcc.h"
#include "jbsockets.h"
#include "ssl.h"
#include "ssl_common.h"

static int ssl_certificate_is_invalid(const char *cert_file);
static int generate_host_certificate(struct client_state *csp,
   const char *certificate_path, const char *key_path);
static void free_client_ssl_structures(struct client_state *csp);
static void free_server_ssl_structures(struct client_state *csp);
static int ssl_store_cert(struct client_state *csp, X509 *crt);
static void log_ssl_errors(int debuglevel, const char* fmt, ...) __attribute__((format(printf, 2, 3)));

static int wolfssl_initialized = 0;

/*
 * Whether or not sharing the RNG is thread-safe
 * doesn't matter because we only use it with
 * the certificate_mutex locked.
 */
static WC_RNG wolfssl_rng;

#ifndef WOLFSSL_ALT_CERT_CHAINS
/*
 * Without WOLFSSL_ALT_CERT_CHAINS wolfSSL will reject valid
 * certificates if the certificate chain contains CA certificates
 * that are "only" signed by trusted CA certificates but aren't
 * trusted CAs themselves.
 */
#warning wolfSSL has been compiled without WOLFSSL_ALT_CERT_CHAINS
#endif

/*********************************************************************
 *
 * Function    :  wolfssl_init
 *
 * Description :  Initializes wolfSSL library once
 *
 * Parameters  :  N/A
 *
 * Returns     :  N/A
 *
 *********************************************************************/
static void wolfssl_init(void)
{
   if (wolfssl_initialized == 0)
   {
      privoxy_mutex_lock(&ssl_init_mutex);
      if (wolfssl_initialized == 0)
      {
         if (wolfSSL_Init() != WOLFSSL_SUCCESS)
         {
            log_error(LOG_LEVEL_FATAL, "Failed to initialize wolfSSL");
         }
         wc_InitRng(&wolfssl_rng);
         wolfssl_initialized = 1;
      }
      privoxy_mutex_unlock(&ssl_init_mutex);
   }
}


/*********************************************************************
 *
 * Function    :  is_ssl_pending
 *
 * Description :  Tests if there are some waiting data on ssl connection.
 *                Only considers data that has actually been received
 *                locally and ignores data that is still on the fly
 *                or has not yet been sent by the remote end.
 *
 * Parameters  :
 *          1  :  ssl_attr = SSL context to test
 *
 * Returns     :   0 => No data are pending
 *                >0 => Pending data length. XXX: really?
 *
 *********************************************************************/
extern size_t is_ssl_pending(struct ssl_attr *ssl_attr)
{
   return (size_t)wolfSSL_pending(ssl_attr->wolfssl_attr.ssl);
}


/*********************************************************************
 *
 * Function    :  ssl_send_data
 *
 * Description :  Sends the content of buf (for n bytes) to given SSL
 *                connection context.
 *
 * Parameters  :
 *          1  :  ssl_attr = SSL context to send data to
 *          2  :  buf = Pointer to data to be sent
 *          3  :  len = Length of data to be sent to the SSL context
 *
 * Returns     :  Length of sent data or negative value on error.
 *
 *********************************************************************/
extern int ssl_send_data(struct ssl_attr *ssl_attr, const unsigned char *buf, size_t len)
{
   WOLFSSL *ssl;
   int ret = 0;
   int pos = 0; /* Position of unsent part in buffer */
   int fd = -1;

   if (len == 0)
   {
      return 0;
   }

   wolfSSL_ERR_clear_error();

   ssl = ssl_attr->wolfssl_attr.ssl;
   fd = wolfSSL_get_fd(ssl);

   while (pos < len)
   {
      int send_len = (int)len - pos;

      log_error(LOG_LEVEL_WRITING, "TLS on socket %d: %N",
         fd, send_len, buf+pos);

      ret = wolfSSL_write(ssl, buf+pos, send_len);
      if (ret <= 0)
      {
         log_ssl_errors(LOG_LEVEL_ERROR,
            "Sending data on socket %d over TLS failed", fd);
         return -1;
      }

      /* Adding count of sent bytes to position in buffer */
      pos = pos + ret;
   }

   return (int)len;
}


/*********************************************************************
 *
 * Function    :  ssl_recv_data
 *
 * Description :  Receives data from given SSL context and puts
 *                it into buffer.
 *
 * Parameters  :
 *          1  :  ssl_attr = SSL context to receive data from
 *          2  :  buf = Pointer to buffer where data will be written
 *          3  :  max_length = Maximum number of bytes to read
 *
 * Returns     :  Number of bytes read, 0 for EOF, or -1
 *                on error.
 *
 *********************************************************************/
extern int ssl_recv_data(struct ssl_attr *ssl_attr, unsigned char *buf, size_t max_length)
{
   WOLFSSL *ssl;
   int ret = 0;
   int fd = -1;

   memset(buf, 0, max_length);
   wolfSSL_ERR_clear_error();

   /*
    * Receiving data from SSL context into buffer
    */
   ssl = ssl_attr->wolfssl_attr.ssl;
   ret = wolfSSL_read(ssl, buf, (int)max_length);
   fd = wolfSSL_get_fd(ssl);

   if (ret < 0)
   {
      log_ssl_errors(LOG_LEVEL_ERROR,
         "Receiving data on socket %d over TLS failed", fd);

      return -1;
   }

   log_error(LOG_LEVEL_RECEIVED, "TLS from socket %d: %N",
      fd, ret, buf);

   return ret;
}


/*********************************************************************
 *
 * Function    :  get_public_key_size_string
 *
 * Description : Translates a public key type to a string.
 *
 * Parameters  :
 *          1  :  key_type = The public key type.
 *
 * Returns     :  String containing the translated key size.
 *
 *********************************************************************/
static const char *get_public_key_size_string(int key_type)
{
   switch (key_type)
   {
      case EVP_PKEY_RSA:
         return "RSA key size";
      case EVP_PKEY_DSA:
         return "DSA key size";
      case EVP_PKEY_EC:
         return "EC key size";
      default:
         return "non-RSA/DSA/EC key size";
   }
}


/*********************************************************************
 *
 * Function    :  ssl_store_cert
 *
 * Description : This function is called once for each certificate in the
 *               server's certificate trusted chain and prepares
 *               information about the certificate. The information can
 *               be used to inform the user about invalid certificates.
 *
 * Parameters  :
 *          1  :  csp = Current client state (buffers, headers, etc...)
 *          2  :  cert = certificate from trusted chain
 *
 * Returns     :  0 on success and negative value on error
 *
 *********************************************************************/
static int ssl_store_cert(struct client_state *csp, X509 *cert)
{
   long len;
   struct certs_chain *last = &(csp->server_certs_chain);
   int ret = 0;
   WOLFSSL_BIO *bio = BIO_new(BIO_s_mem());
   WOLFSSL_EVP_PKEY *pkey = NULL;
   char *bio_mem_data = NULL;
   char *encoded_text;
   long l;
   unsigned char serial_number[32];
   int  serial_number_size = sizeof(serial_number);
   WOLFSSL_X509_NAME *issuer_name;
   WOLFSSL_X509_NAME *subject_name;
   char *subject_alternative_name;
   int loc;
   int san_prefix_printed = 0;

   if (!bio)
   {
      log_error(LOG_LEVEL_ERROR, "BIO_new() failed.");
      return -1;
   }

   /*
    * Searching for last item in certificates linked list
    */
   while (last->next != NULL)
   {
      last = last->next;
   }

   /*
    * Preparing next item in linked list for next certificate
    */
   last->next = zalloc_or_die(sizeof(struct certs_chain));

   /*
    * Saving certificate file into buffer
    */
   if (wolfSSL_PEM_write_bio_X509(bio, cert) != WOLFSSL_SUCCESS)
   {
      log_error(LOG_LEVEL_ERROR, "wolfSSL_PEM_write_bio_X509() failed.");
      ret = -1;
      goto exit;
   }

   len = wolfSSL_BIO_get_mem_data(bio, &bio_mem_data);
   last->file_buf = malloc((size_t)len + 1);
   if (last->file_buf == NULL)
   {
      log_error(LOG_LEVEL_ERROR,
         "Failed to allocate %lu bytes to store the X509 PEM certificate.",
         len + 1);
      ret = -1;
      goto exit;
   }

   strncpy(last->file_buf, bio_mem_data, (size_t)len);
   last->file_buf[len] = '\0';
   wolfSSL_BIO_free(bio);
   bio = wolfSSL_BIO_new(wolfSSL_BIO_s_mem());
   if (!bio)
   {
      log_error(LOG_LEVEL_ERROR, "wolfSSL_BIO_new() failed.");
      ret = -1;
      goto exit;
   }

   /*
    * Saving certificate information into buffer
    */
   l = wolfSSL_X509_get_version(cert);
   if (l >= 0 && l <= 2)
   {
      if (wolfSSL_BIO_printf(bio, "cert. version     : %ld\n", l + 1) <= 0)
      {
         log_error(LOG_LEVEL_ERROR, "wolfSSL_BIO_printf() for version failed.");
         ret = -1;
         goto exit;
      }
   }
   else
   {
      if (wolfSSL_BIO_printf(bio, "cert. version     : Unknown (%ld)\n", l) <= 0)
      {
         log_error(LOG_LEVEL_ERROR, "wolfSSL_BIO_printf() for version failed.");
         ret = -1;
         goto exit;
      }
   }

   if (wolfSSL_BIO_puts(bio, "serial number     : ") <= 0)
   {
      log_error(LOG_LEVEL_ERROR,
         "wolfSSL_BIO_puts() for serial number failed.");
      ret = -1;
      goto exit;
   }
   if (wolfSSL_X509_get_serial_number(cert, serial_number, &serial_number_size)
      != WOLFSSL_SUCCESS)
   {
      log_error(LOG_LEVEL_ERROR, "wolfSSL_X509_get_serial_number() failed.");
      ret = -1;
      goto exit;
   }

   if (serial_number_size <= (int)sizeof(char))
   {
      if (wolfSSL_BIO_printf(bio, "%lu (0x%lx)\n", serial_number[0],
            serial_number[0]) <= 0)
      {
         log_error(LOG_LEVEL_ERROR,
            "wolfSSL_BIO_printf() for serial number as single byte failed.");
         ret = -1;
         goto exit;
      }
   }
   else
   {
      int i;
      for (i = 0; i < serial_number_size; i++)
      {
         if (wolfSSL_BIO_printf(bio, "%02x%c", serial_number[i],
               ((i + 1 == serial_number_size) ? '\n' : ':')) <= 0)
         {
            log_error(LOG_LEVEL_ERROR,
               "wolfSSL_BIO_printf() for serial number bytes failed.");
            ret = -1;
            goto exit;
         }
      }
   }

   if (wolfSSL_BIO_puts(bio, "issuer name       : ") <= 0)
   {
      log_error(LOG_LEVEL_ERROR, "wolfSSL_BIO_puts() for issuer failed.");
      ret = -1;
      goto exit;
   }
   issuer_name = wolfSSL_X509_get_issuer_name(cert);
   if (wolfSSL_X509_NAME_get_sz(issuer_name) <= 0)
   {
      if (wolfSSL_BIO_puts(bio, "none") <= 0)
      {
         log_error(LOG_LEVEL_ERROR,
            "wolfSSL_BIO_puts() for issuer name failed.");
         ret = -1;
         goto exit;
      }
   }
   else if (wolfSSL_X509_NAME_print_ex(bio, issuer_name, 0, 0) < 0)
   {
      log_error(LOG_LEVEL_ERROR,
         "wolfSSL_X509_NAME_print_ex() for issuer failed.");
      ret = -1;
      goto exit;
   }

   if (wolfSSL_BIO_puts(bio, "\nsubject name      : ") <= 0)
   {
      log_error(LOG_LEVEL_ERROR,
         "wolfSSL_BIO_puts() for subject name failed.");
      ret = -1;
      goto exit;
   }
   subject_name = wolfSSL_X509_get_subject_name(cert);
   if (wolfSSL_X509_NAME_get_sz(subject_name) <= 0)
   {
      if (wolfSSL_BIO_puts(bio, "none") <= 0)
      {
         log_error(LOG_LEVEL_ERROR,
            "wolfSSL_BIO_puts() for subject name failed.");
         ret = -1;
         goto exit;
      }
   }
   else if (wolfSSL_X509_NAME_print_ex(bio, subject_name, 0, 0) < 0)
   {
      log_error(LOG_LEVEL_ERROR,
         "wolfSSL_X509_NAME_print_ex() for subject name failed.");
      ret = -1;
      goto exit;
   }

   if (wolfSSL_BIO_puts(bio, "\nissued  on        : ") <= 0)
   {
      log_error(LOG_LEVEL_ERROR,
         "wolfSSL_BIO_puts() for issued on failed.");
      ret = -1;
      goto exit;
   }
   if (!wolfSSL_ASN1_TIME_print(bio, wolfSSL_X509_get_notBefore(cert)))
   {
      log_error(LOG_LEVEL_ERROR,
         "wolfSSL_ASN1_TIME_print() for issued on failed.");
      ret = -1;
      goto exit;
   }

   if (wolfSSL_BIO_puts(bio, "\nexpires on        : ") <= 0)
   {
      log_error(LOG_LEVEL_ERROR,
         "wolfSSL_BIO_puts() for expires on failed.");
      ret = -1;
      goto exit;
   }
   if (!wolfSSL_ASN1_TIME_print(bio, wolfSSL_X509_get_notAfter(cert)))
   {
      log_error(LOG_LEVEL_ERROR,
         "wolfSSL_ASN1_TIME_print() for expires on failed.");
      ret = -1;
      goto exit;
   }

   /* XXX: Show signature algorithm */

   pkey = wolfSSL_X509_get_pubkey(cert);
   if (!pkey)
   {
      log_error(LOG_LEVEL_ERROR, "wolfSSL_X509_get_pubkey() failed.");
      ret = -1;
      goto exit;
   }
   ret = wolfSSL_BIO_printf(bio, "\n%-18s: %d bits",
      get_public_key_size_string(wolfSSL_EVP_PKEY_base_id(pkey)),
      wolfSSL_EVP_PKEY_bits(pkey));
   if (ret <= 0)
   {
      log_error(LOG_LEVEL_ERROR,
         "wolfSSL_BIO_printf() for key size failed.");
      ret = -1;
      goto exit;
   }

   /*
    * XXX: Show cert usage, etc.
    */
   loc = wolfSSL_X509_get_ext_by_NID(cert, NID_basic_constraints, -1);
   if (loc != -1)
   {
      WOLFSSL_X509_EXTENSION *ex = wolfSSL_X509_get_ext(cert, loc);
      if (BIO_puts(bio, "\nbasic constraints : ") <= 0)
      {
         log_error(LOG_LEVEL_ERROR,
            "BIO_printf() for basic constraints failed.");
         ret = -1;
         goto exit;
      }
      if (!wolfSSL_X509V3_EXT_print(bio, ex, 0, 0))
      {
         if (!wolfSSL_ASN1_STRING_print_ex(bio,
               wolfSSL_X509_EXTENSION_get_data(ex),
               ASN1_STRFLGS_RFC2253))
         {
            log_error(LOG_LEVEL_ERROR,
               "wolfSSL_ASN1_STRING_print_ex() for basic constraints failed.");
            ret = -1;
            goto exit;
         }
      }
   }

   while ((subject_alternative_name = wolfSSL_X509_get_next_altname(cert))
      != NULL)
   {
      if (san_prefix_printed == 0)
      {
         ret = wolfSSL_BIO_printf(bio, "\nsubject alt name  : ");
         san_prefix_printed = 1;
      }
      if (ret > 0)
      {
         ret = wolfSSL_BIO_printf(bio, "%s ", subject_alternative_name);
      }
      if (ret <= 0)
      {
         log_error(LOG_LEVEL_ERROR,
            "wolfSSL_BIO_printf() for Subject Alternative Name failed.");
         ret = -1;
         goto exit;
      }
   }

#if 0
   /*
    * This code compiles but does not work because wolfSSL
    * sets NID_netscape_cert_type to NID_undef.
    */
   loc = wolfSSL_X509_get_ext_by_NID(cert, NID_netscape_cert_type, -1);
   if (loc != -1)
   {
      WOLFSSL_X509_EXTENSION *ex = wolfSSL_X509_get_ext(cert, loc);
      if (wolfSSL_BIO_puts(bio, "\ncert. type        : ") <= 0)
      {
         log_error(LOG_LEVEL_ERROR,
            "wolfSSL_BIO_printf() for cert type failed.");
         ret = -1;
         goto exit;
      }
      if (!wolfSSL_X509V3_EXT_print(bio, ex, 0, 0))
      {
         if (!wolfSSL_ASN1_STRING_print_ex(bio,
               wolfSSL_X509_EXTENSION_get_data(ex),
               ASN1_STRFLGS_RFC2253))
         {
            log_error(LOG_LEVEL_ERROR,
               "wolfSSL_ASN1_STRING_print_ex() for cert type failed.");
            ret = -1;
            goto exit;
         }
      }
   }
#endif

#if 0
   /*
    * This code compiles but does not work. wolfSSL_OBJ_obj2nid()
    * triggers a 'X509V3_EXT_print not yet implemented for ext type' message.
     */
   loc = wolfSSL_X509_get_ext_by_NID(cert, NID_key_usage, -1);
   if (loc != -1)
   {
      WOLFSSL_X509_EXTENSION *extension = wolfSSL_X509_get_ext(cert, loc);
      if (BIO_puts(bio, "\nkey usage         : ") <= 0)
      {
         log_error(LOG_LEVEL_ERROR,
            "wolfSSL_BIO_printf() for key usage failed.");
         ret = -1;
         goto exit;
      }
      if (!wolfSSL_X509V3_EXT_print(bio, extension, 0, 0))
      {
         if (!wolfSSL_ASN1_STRING_print_ex(bio,
               wolfSSL_X509_EXTENSION_get_data(extension),
               ASN1_STRFLGS_RFC2253))
         {
            log_error(LOG_LEVEL_ERROR,
               "wolfSSL_ASN1_STRING_print_ex() for key usage failed.");
            ret = -1;
            goto exit;
         }
      }
   }
#endif

#if 0
   /*
    * This compiles but doesn't work. wolfSSL_X509_ext_isSet_by_NID()
    * complains about "NID not in table".
    */
   loc = wolfSSL_X509_get_ext_by_NID(cert, NID_ext_key_usage, -1);
   if (loc != -1) {
      WOLFSSL_X509_EXTENSION *ex = wolfSSL_X509_get_ext(cert, loc);
      if (wolfSSL_BIO_puts(bio, "\next key usage     : ") <= 0)
      {
         log_error(LOG_LEVEL_ERROR,
            "wolfSSL_BIO_printf() for ext key usage failed.");
         ret = -1;
         goto exit;
      }
      if (!wolfSSL_X509V3_EXT_print(bio, ex, 0, 0))
      {
         if (!wolfSSL_ASN1_STRING_print_ex(bio,
               wolfSSL_X509_EXTENSION_get_data(ex),
               ASN1_STRFLGS_RFC2253))
         {
            log_error(LOG_LEVEL_ERROR,
               "wolfSSL_ASN1_STRING_print_ex() for ext key usage failed.");
            ret = -1;
            goto exit;
         }
      }
   }
#endif

#if 0
   /*
    * This compiles but doesn't work. wolfSSL_X509_ext_isSet_by_NID()
    * complains about "NID not in table". XXX: again?
    */
   loc = wolfSSL_X509_get_ext_by_NID(cert, NID_certificate_policies, -1);
   if (loc != -1)
   {
      WOLFSSL_X509_EXTENSION *ex = wolfSSL_X509_get_ext(cert, loc);
      if (wolfSSL_BIO_puts(bio, "\ncertificate policies : ") <= 0)
      {
         log_error(LOG_LEVEL_ERROR,
            "wolfSSL_BIO_printf() for certificate policies failed.");
         ret = -1;
         goto exit;
      }
      if (!wolfSSL_X509V3_EXT_print(bio, ex, 0, 0))
      {
         if (!wolfSSL_ASN1_STRING_print_ex(bio,
               wolfSSL_X509_EXTENSION_get_data(ex),
               ASN1_STRFLGS_RFC2253))
         {
            log_error(LOG_LEVEL_ERROR,
               "wolfSSL_ASN1_STRING_print_ex() for certificate policies failed.");
            ret = -1;
            goto exit;
         }
      }
   }
#endif

   /* make valgrind happy */
   static const char zero = 0;
   wolfSSL_BIO_write(bio, &zero, 1);

   len = wolfSSL_BIO_get_mem_data(bio, &bio_mem_data);
   if (len <= 0)
   {
      log_error(LOG_LEVEL_ERROR, "BIO_get_mem_data() returned %ld "
         "while gathering certificate information.", len);
      ret = -1;
      goto exit;
   }
   encoded_text = html_encode(bio_mem_data);
   if (encoded_text == NULL)
   {
      log_error(LOG_LEVEL_ERROR,
         "Failed to HTML-encode the certificate information.");
      ret = -1;
      goto exit;
   }

   strlcpy(last->info_buf, encoded_text, sizeof(last->info_buf));
   freez(encoded_text);
   ret = 0;

exit:
   if (bio)
   {
      wolfSSL_BIO_free(bio);
   }
   if (pkey)
   {
      wolfSSL_EVP_PKEY_free(pkey);
   }
   return ret;
}


/*********************************************************************
 *
 * Function    :  host_to_hash
 *
 * Description :  Creates a sha256 hash from host name. The host name
 *                is taken from the csp structure and stored into it.
 *
 * Parameters  :
 *          1  :  csp = Current client state (buffers, headers, etc...)
 *
 * Returns     : -1 => Error while creating hash
 *                0 => Hash created successfully
 *
 *********************************************************************/
static int host_to_hash(struct client_state *csp)
{
   int ret;

   ret = wc_Sha256Hash((const byte *)csp->http->host,
      (word32)strlen(csp->http->host), (byte *)csp->http->hash_of_host);
   if (ret != 0)
   {
        return -1;
   }

   return create_hexadecimal_hash_of_host(csp);

}


/*********************************************************************
 *
 * Function    :  create_client_ssl_connection
 *
 * Description :  Creates a TLS-secured connection with the client.
 *
 * Parameters  :
 *          1  :  csp = Current client state (buffers, headers, etc...)
 *
 * Returns     :  0 on success, negative value if connection wasn't created
 *                successfully.
 *
 *********************************************************************/
extern int create_client_ssl_connection(struct client_state *csp)
{
   struct ssl_attr *ssl_attr = &csp->ssl_client_attr;
   /* Paths to certificates file and key file */
   char *key_file  = NULL;
   char *cert_file = NULL;
   int ret = 0;
   WOLFSSL *ssl;

   /* Should probably be called from somewhere else. */
   wolfssl_init();

   /*
    * Preparing hash of host for creating certificates
    */
   ret = host_to_hash(csp);
   if (ret != 0)
   {
      log_error(LOG_LEVEL_ERROR, "Generating hash of host failed: %d", ret);
      ret = -1;
      goto exit;
   }

   /*
    * Preparing paths to certificates files and key file
    */
   cert_file = make_certs_path(csp->config->certificate_directory,
      (const char *)csp->http->hash_of_host_hex, CERT_FILE_TYPE);
   key_file  = make_certs_path(csp->config->certificate_directory,
      (const char *)csp->http->hash_of_host_hex, KEY_FILE_TYPE);

   if (cert_file == NULL || key_file == NULL)
   {
      ret = -1;
      goto exit;
   }

   /* Do we need to generate a new host certificate and key? */
   if (!file_exists(cert_file) || !file_exists(key_file) ||
       ssl_certificate_is_invalid(cert_file))
   {
      /*
       * Yes we do. Lock mutex to prevent certificate and
       * key inconsistencies.
       */
      privoxy_mutex_lock(&certificate_mutex);
      ret = generate_host_certificate(csp, cert_file, key_file);
      privoxy_mutex_unlock(&certificate_mutex);
      if (ret < 0)
      {
         /*
          * No need to log something, generate_host_certificate()
          * took care of it.
          */
         ret = -1;
         goto exit;
      }
   }
   ssl_attr->wolfssl_attr.ctx = wolfSSL_CTX_new(wolfSSLv23_method());
   if (ssl_attr->wolfssl_attr.ctx == NULL)
   {
      log_error(LOG_LEVEL_ERROR, "Unable to create TLS context.");
      ret = -1;
      goto exit;
   }

   /* Set the key and cert */
   if (wolfSSL_CTX_use_certificate_file(ssl_attr->wolfssl_attr.ctx,
         cert_file, SSL_FILETYPE_PEM) != WOLFSSL_SUCCESS)
   {
      log_error(LOG_LEVEL_ERROR,
         "Loading host certificate %s failed.", cert_file);
      ret = -1;
      goto exit;
   }

   if (wolfSSL_CTX_use_PrivateKey_file(ssl_attr->wolfssl_attr.ctx,
         key_file, SSL_FILETYPE_PEM) != WOLFSSL_SUCCESS)
   {
      log_error(LOG_LEVEL_ERROR,
         "Loading host certificate private key %s failed.", key_file);
      ret = -1;
      goto exit;
   }

   wolfSSL_CTX_set_options(ssl_attr->wolfssl_attr.ctx, WOLFSSL_OP_NO_SSLv3);

   ssl = ssl_attr->wolfssl_attr.ssl = wolfSSL_new(ssl_attr->wolfssl_attr.ctx);

   if (wolfSSL_set_fd(ssl, csp->cfd) != WOLFSSL_SUCCESS)
   {
      log_error(LOG_LEVEL_ERROR,
         "wolfSSL_set_fd() failed to set the client socket.");
      ret = -1;
      goto exit;
   }

   if (csp->config->cipher_list != NULL)
   {
      if (!wolfSSL_set_cipher_list(ssl, csp->config->cipher_list))
      {
         log_error(LOG_LEVEL_ERROR,
            "Setting the cipher list '%s' for the client connection failed.",
            csp->config->cipher_list);
         ret = -1;
         goto exit;
      }
   }

   /*
    *  Handshake with client
    */
   log_error(LOG_LEVEL_CONNECT,
      "Performing the TLS/SSL handshake with client. Hash of host: %s",
      csp->http->hash_of_host_hex);

   ret = wolfSSL_accept(ssl);
   if (ret == WOLFSSL_SUCCESS)
   {
      log_error(LOG_LEVEL_CONNECT,
         "Client successfully connected over %s (%s).",
         wolfSSL_get_version(ssl), wolfSSL_get_cipher_name(ssl));
      csp->ssl_with_client_is_opened = 1;
      ret = 0;
   }
   else
   {
      char buffer[80];
      int error = wolfSSL_get_error(ssl, ret);
      log_error(LOG_LEVEL_ERROR,
         "The TLS handshake with the client failed. error = %d, %s",
         error, wolfSSL_ERR_error_string((unsigned long)error, buffer));
      ret = -1;
   }

exit:
   /*
    * Freeing allocated paths to files
    */
   freez(cert_file);
   freez(key_file);

   /* Freeing structures if connection wasn't created successfully */
   if (ret < 0)
   {
      free_client_ssl_structures(csp);
   }
   return ret;
}


/*********************************************************************
 *
 * Function    :  shutdown_connection
 *
 * Description :  Shuts down a TLS connection if the socket is still
 *                alive.
 *
 * Parameters  :
 *          1  :  csp = Current client state (buffers, headers, etc...)
 *
 * Returns     :  N/A
 *
 *********************************************************************/
static void shutdown_connection(WOLFSSL *ssl, const char *type)
{
   int shutdown_attempts = 0;
   int ret;
   int fd;
   enum { MAX_SHUTDOWN_ATTEMPTS = 5 };

   fd = wolfSSL_get_fd(ssl);

   do
   {
      if (!socket_is_still_alive(fd))
      {
         log_error(LOG_LEVEL_CONNECT, "Not shutting down %s connection "
            "on socket %d. The socket is no longer alive.", type, fd);
         return;
      }
      ret = wolfSSL_shutdown(ssl);
      shutdown_attempts++;
      if (WOLFSSL_SUCCESS != ret)
      {
         log_error(LOG_LEVEL_CONNECT, "Failed to shutdown %s connection "
            "on socket %d. Attempts so far: %d.", type, fd, shutdown_attempts);
      }
   } while (ret == WOLFSSL_SHUTDOWN_NOT_DONE &&
      shutdown_attempts < MAX_SHUTDOWN_ATTEMPTS);
   if (WOLFSSL_SUCCESS != ret)
   {
      char buffer[80];
      int error = wolfSSL_get_error(ssl, ret);
      log_error(LOG_LEVEL_CONNECT, "Failed to shutdown %s connection "
         "on socket %d after %d attempts. ret: %d, error: %d, %s",
         type, fd, shutdown_attempts, ret, error,
         wolfSSL_ERR_error_string((unsigned long)error, buffer));
   }
   else if (shutdown_attempts > 1)
   {
      log_error(LOG_LEVEL_CONNECT, "Succeeded to shutdown %s connection "
         "on socket %d after %d attempts.", type, fd, shutdown_attempts);
   }
}


/*********************************************************************
 *
 * Function    :  close_client_ssl_connection
 *
 * Description :  Closes TLS connection with client. This function
 *                checks if this connection is already created.
 *
 * Parameters  :
 *          1  :  csp = Current client state (buffers, headers, etc...)
 *
 * Returns     :  N/A
 *
 *********************************************************************/
extern void close_client_ssl_connection(struct client_state *csp)
{
   struct ssl_attr *ssl_attr = &csp->ssl_client_attr;

   if (csp->ssl_with_client_is_opened == 0)
   {
      return;
   }

   /*
    * Notify the peer that the connection is being closed.
    */
   shutdown_connection(ssl_attr->wolfssl_attr.ssl, "client");

   free_client_ssl_structures(csp);
   csp->ssl_with_client_is_opened = 0;
}


/*********************************************************************
 *
 * Function    :  free_client_ssl_structures
 *
 * Description :  Frees structures used for SSL communication with
 *                client.
 *
 * Parameters  :
 *          1  :  csp = Current client state (buffers, headers, etc...)
 *
 * Returns     :  N/A
 *
 *********************************************************************/
static void free_client_ssl_structures(struct client_state *csp)
{
   struct ssl_attr *ssl_attr = &csp->ssl_client_attr;

   if (ssl_attr->wolfssl_attr.ssl)
   {
      wolfSSL_free(ssl_attr->wolfssl_attr.ssl);
   }
   if (ssl_attr->wolfssl_attr.ctx)
   {
      wolfSSL_CTX_free(ssl_attr->wolfssl_attr.ctx);
   }
}


/*********************************************************************
 *
 * Function    :  close_server_ssl_connection
 *
 * Description :  Closes TLS connection with server. This function
 *                checks if this connection is already opened.
 *
 * Parameters  :
 *          1  :  csp = Current client state (buffers, headers, etc...)
 *
 * Returns     :  N/A
 *
 *********************************************************************/
extern void close_server_ssl_connection(struct client_state *csp)
{
   struct ssl_attr *ssl_attr = &csp->ssl_server_attr;

   if (csp->ssl_with_server_is_opened == 0)
   {
      return;
   }

   /*
   * Notify the peer that the connection is being closed.
   */
   shutdown_connection(ssl_attr->wolfssl_attr.ssl, "server");

   free_server_ssl_structures(csp);
   csp->ssl_with_server_is_opened = 0;
}


/*********************************************************************
 *
 * Function    :  create_server_ssl_connection
 *
 * Description :  Creates TLS-secured connection with the server.
 *
 * Parameters  :
 *          1  :  csp = Current client state (buffers, headers, etc...)
 *
 * Returns     :  0 on success, negative value if connection wasn't created
 *                successfully.
 *
 *********************************************************************/
extern int create_server_ssl_connection(struct client_state *csp)
{
   wolfssl_connection_attr *ssl_attrs = &csp->ssl_server_attr.wolfssl_attr;
   int ret = 0;
   WOLFSSL *ssl;
   int connect_ret = 0;

   csp->server_cert_verification_result = SSL_CERT_NOT_VERIFIED;
   csp->server_certs_chain.next = NULL;

   ssl_attrs->ctx = wolfSSL_CTX_new(wolfSSLv23_method());
   if (ssl_attrs->ctx == NULL)
   {
      log_error(LOG_LEVEL_ERROR, "TLS context creation failed");
      ret = -1;
      goto exit;
   }

   if (csp->dont_verify_certificate)
   {
      wolfSSL_CTX_set_verify(ssl_attrs->ctx, WOLFSSL_VERIFY_NONE, NULL);
   }
   else if (wolfSSL_CTX_load_verify_locations(ssl_attrs->ctx,
      csp->config->trusted_cas_file, NULL) != WOLFSSL_SUCCESS)
   {
      log_error(LOG_LEVEL_ERROR, "Loading trusted-cas-file '%s' failed.",
         csp->config->trusted_cas_file);
      ret = -1;
      goto exit;
   }

   wolfSSL_CTX_set_options(ssl_attrs->ctx, WOLFSSL_OP_NO_SSLv3);

   ssl = ssl_attrs->ssl = wolfSSL_new(ssl_attrs->ctx);

   if (wolfSSL_set_fd(ssl, csp->server_connection.sfd) != WOLFSSL_SUCCESS)
   {
      log_error(LOG_LEVEL_ERROR,
         "wolfSSL_set_fd() failed to set the server socket.");
      ret = -1;
      goto exit;
   }

   if (csp->config->cipher_list != NULL)
   {
      if (wolfSSL_set_cipher_list(ssl, csp->config->cipher_list) != WOLFSSL_SUCCESS)
      {
         log_error(LOG_LEVEL_ERROR,
            "Setting the cipher list '%s' for the server connection failed.",
            csp->config->cipher_list);
         ret = -1;
         goto exit;
      }
   }

   ret = wolfSSL_UseSNI(ssl, WOLFSSL_SNI_HOST_NAME,
      csp->http->host, (unsigned short)strlen(csp->http->host));
   if (ret != WOLFSSL_SUCCESS)
   {
      log_error(LOG_LEVEL_ERROR, "Failed to set use of SNI.");
      ret = -1;
      goto exit;
   }

   ret = wolfSSL_check_domain_name(ssl, csp->http->host);
   if (ret != WOLFSSL_SUCCESS)
   {
      char buffer[80];
      int error = wolfSSL_get_error(ssl, ret);
      log_error(LOG_LEVEL_FATAL,
         "Failed to set check domain name. error = %d, %s",
         error, wolfSSL_ERR_error_string((unsigned long)error, buffer));
      ret = -1;
      goto exit;
   }

#ifdef HAVE_SECURE_RENEGOTIATION
#warning wolfssl has been compiled with HAVE_SECURE_RENEGOTIATION while you probably want HAVE_RENEGOTIATION_INDICATION
   if(wolfSSL_UseSecureRenegotiation(ssl) != WOLFSSL_SUCCESS)
   {
      log_error(LOG_LEVEL_ERROR,
         "Failed to enable 'Secure' Renegotiation. Continuing anyway.");
   }
#endif
#ifndef HAVE_RENEGOTIATION_INDICATION
#warning Looks like wolfssl has been compiled without HAVE_RENEGOTIATION_INDICATION
#endif

   log_error(LOG_LEVEL_CONNECT,
      "Performing the TLS/SSL handshake with the server");

   /* wolfSSL_Debugging_ON(); */
   connect_ret = wolfSSL_connect(ssl);
   /* wolfSSL_Debugging_OFF(); */

   /*
   wolfSSL_Debugging_ON();
   */
   if (!csp->dont_verify_certificate)
   {
      long verify_result = wolfSSL_get_error(ssl, connect_ret);

#if LIBWOLFSSL_VERSION_HEX > 0x05005004
      if (verify_result == WOLFSSL_X509_V_OK)
#else
      if (verify_result == X509_V_OK)
#endif
      {
         ret = 0;
         csp->server_cert_verification_result = SSL_CERT_VALID;
      }
      else
      {
         WOLF_STACK_OF(WOLFSSL_X509) *chain;

         csp->server_cert_verification_result = verify_result;
         log_error(LOG_LEVEL_ERROR,
            "X509 certificate verification for %s failed with error %ld: %s",
            csp->http->hostport, verify_result,
            wolfSSL_X509_verify_cert_error_string(verify_result));

         chain = wolfSSL_get_peer_cert_chain(ssl);
         if (chain != NULL)
         {
            int i;
            for (i = 0; i < wolfSSL_sk_X509_num(chain); i++)
            {
               if (ssl_store_cert(csp, wolfSSL_sk_X509_value(chain, i)) != 0)
               {
                  log_error(LOG_LEVEL_ERROR,
                     "ssl_store_cert() failed for cert %d", i);
                  /*
                   * ssl_send_certificate_error() wil not be able to show
                   * the certificate but the user will stil get the error
                   * description.
                   */
               }
            }
         }

         ret = -1;
         goto exit;
      }
   }
   /*
   wolfSSL_Debugging_OFF();
   */
   if (connect_ret == WOLFSSL_SUCCESS)
   {
      log_error(LOG_LEVEL_CONNECT,
         "Server successfully connected over %s (%s).",
         wolfSSL_get_version(ssl), wolfSSL_get_cipher_name(ssl));
      csp->ssl_with_server_is_opened = 1;
      ret = 0;
   }
   else
   {
      char buffer[80];
      int error = wolfSSL_get_error(ssl, ret);
      log_error(LOG_LEVEL_ERROR,
         "The TLS handshake with the server %s failed. error = %d, %s",
         csp->http->hostport,
         error, wolfSSL_ERR_error_string((unsigned long)error, buffer));
      ret = -1;
   }

exit:
   /* Freeing structures if connection wasn't created successfully */
   if (ret < 0)
   {
      free_server_ssl_structures(csp);
   }

   return ret;
}


/*********************************************************************
 *
 * Function    :  free_server_ssl_structures
 *
 * Description :  Frees structures used for SSL communication with server
 *
 * Parameters  :
 *          1  :  csp = Current client state (buffers, headers, etc...)
 *
 * Returns     :  N/A
 *
 *********************************************************************/
static void free_server_ssl_structures(struct client_state *csp)
{
   struct ssl_attr *ssl_attr = &csp->ssl_server_attr;

   if (ssl_attr->wolfssl_attr.ssl)
   {
      wolfSSL_free(ssl_attr->wolfssl_attr.ssl);
   }
   if (ssl_attr->wolfssl_attr.ctx)
   {
      wolfSSL_CTX_free(ssl_attr->wolfssl_attr.ctx);
   }
}


/*********************************************************************
 *
 * Function    :  log_ssl_errors
 *
 * Description :  Log SSL errors
 *
 * Parameters  :
 *          1  :  debuglevel = Debug level
 *          2  :  desc = Error description
 *
 * Returns     :  N/A
 *
 *********************************************************************/
static void log_ssl_errors(int debuglevel, const char* fmt, ...)
{
   unsigned long err_code;
   char prefix[ERROR_BUF_SIZE];
   va_list args;
   va_start(args, fmt);
   vsnprintf(prefix, sizeof(prefix), fmt, args);
   int reported = 0;

   while ((err_code = wolfSSL_ERR_get_error()))
   {
      char err_buf[ERROR_BUF_SIZE];
      reported = 1;
      wolfSSL_ERR_error_string_n(err_code, err_buf, sizeof(err_buf));
      log_error(debuglevel, "%s: %s", prefix, err_buf);
   }
   va_end(args);
   /*
    * In case if called by mistake and there were
    * no TLS errors let's report it to the log.
    */
   if (!reported)
   {
      log_error(debuglevel, "%s: no TLS errors detected", prefix);
   }
}


/*********************************************************************
 *
 * Function    :  ssl_base64_encode
 *
 * Description :  Encode a buffer into base64 format.
 *
 * Parameters  :
 *          1  :  dst = Destination buffer
 *          2  :  dlen = Destination buffer length
 *          3  :  olen = Number of bytes written
 *          4  :  src = Source buffer
 *          5  :  slen = Amount of data to be encoded
 *
 * Returns     :  0 on success, error code othervise
 *
 *********************************************************************/
extern int ssl_base64_encode(unsigned char *dst, size_t dlen, size_t *olen,
                             const unsigned char *src, size_t slen)
{
   word32 output_length;
   int ret;

   *olen = 4 * ((slen/3) + ((slen%3) ? 1 : 0)) + 1;
   if (*olen > dlen)
   {
      return ENOBUFS;
   }

   output_length = (word32)dlen;
   ret = Base64_Encode_NoNl(src, (word32)slen, dst, &output_length);
   if (ret != 0)
   {
      log_error(LOG_LEVEL_ERROR, "base64 encoding failed with %d", ret);
      return ret;
   }
   *olen = output_length;

   return 0;

}


/*********************************************************************
 *
 * Function    :  close_file_stream
 *
 * Description :  Close file stream, report error on close error
 *
 * Parameters  :
 *          1  :  f = file stream to close
 *          2  :  path = path for error report
 *
 * Returns     :  N/A
 *
 *********************************************************************/
static void close_file_stream(FILE *f, const char *path)
{
   if (fclose(f) != 0)
   {
      log_error(LOG_LEVEL_ERROR,
         "Error closing file %s: %s", path, strerror(errno));
   }
}


/*********************************************************************
 *
 * Function    :  write_certificate
 *
 * Description :  Writes a PEM-encoded certificate to a file.
 *
 * Parameters  :
 *          1  :  certificate_path = Path to the file to create
 *          2  :  certificate = PEM-encoded certificate to write.
 *
 * Returns     :  NULL => Error. Otherwise a key;
 *
 *********************************************************************/
static int write_certificate(const char *certificate_path, const char *certificate)
{
   FILE *fp;

   assert(certificate_path != NULL);
   assert(certificate != NULL);

   fp = fopen(certificate_path, "wb");
   if (NULL == fp)
   {
      log_error(LOG_LEVEL_ERROR,
         "Failed to open %s to write the certificate: %E",
         certificate_path);
      return -1;
   }
   if (fputs(certificate, fp) < 0)
   {
      log_error(LOG_LEVEL_ERROR, "Failed to write certificate to %s: %E",
         certificate_path);
      fclose(fp);
      return -1;
   }
   fclose(fp);

   return 0;

}


/*********************************************************************
 *
 * Function    :  generate_rsa_key
 *
 * Description : Generates a new RSA key and saves it in a file.
 *
 * Parameters  :
 *          1  :  rsa_key_path = Path to the key that should be written.
 *
 * Returns     :  -1 => Error while generating private key
 *                 0 => Success.
 *
 *********************************************************************/
static int generate_rsa_key(const char *rsa_key_path)
{
   RsaKey rsa_key;
   byte rsa_key_der[4096];
   int ret;
   byte key_pem[4096];
   int der_key_size;
   int pem_key_size;
   FILE *f = NULL;

   assert(file_exists(rsa_key_path) != 1);

   wc_InitRsaKey(&rsa_key, NULL);

   log_error(LOG_LEVEL_CONNECT, "Making RSA key %s ...", rsa_key_path);
   ret = wc_MakeRsaKey(&rsa_key, RSA_KEYSIZE, RSA_KEY_PUBLIC_EXPONENT,
      &wolfssl_rng);
   if (ret != 0)
   {
      log_error(LOG_LEVEL_ERROR, "RSA key generation failed");
      ret = -1;
      goto exit;
   }
   log_error(LOG_LEVEL_CONNECT, "Done making RSA key %s", rsa_key_path);

   der_key_size = wc_RsaKeyToDer(&rsa_key, rsa_key_der, sizeof(rsa_key_der));
   wc_FreeRsaKey(&rsa_key);
   if (der_key_size < 0)
   {
      log_error(LOG_LEVEL_ERROR, "RSA key conversion to DER format failed");
      ret = -1;
      goto exit;
   }
   pem_key_size = wc_DerToPem(rsa_key_der, (word32)der_key_size,
      key_pem, sizeof(key_pem), PRIVATEKEY_TYPE);
   if (pem_key_size < 0)
   {
      log_error(LOG_LEVEL_ERROR, "RSA key conversion to PEM format failed");
      ret = -1;
      goto exit;
   }

   /*
    * Saving key into file
    */
   if ((f = fopen(rsa_key_path, "wb")) == NULL)
   {
      log_error(LOG_LEVEL_ERROR,
         "Opening file %s to save private key failed: %E",
         rsa_key_path);
      ret = -1;
      goto exit;
   }

   if (fwrite(key_pem, 1, (size_t)pem_key_size, f) != pem_key_size)
   {
      log_error(LOG_LEVEL_ERROR,
         "Writing private key into file %s failed",
         rsa_key_path);
      close_file_stream(f, rsa_key_path);
      ret = -1;
      goto exit;
   }

   close_file_stream(f, rsa_key_path);

exit:

   return ret;

}


/*********************************************************************
 *
 * Function    :  ssl_certificate_load
 *
 * Description :  Loads certificate from file.
 *
 * Parameters  :
 *          1  :  cert_path = The certificate path to load
 *
 * Returns     :   NULL => error loading certificate,
 *                   pointer to certificate instance otherwise
 *
 *********************************************************************/
static X509 *ssl_certificate_load(const char *cert_path)
{
   X509 *cert = NULL;
   FILE *cert_f = NULL;

   if (!(cert_f = fopen(cert_path, "r")))
   {
      log_error(LOG_LEVEL_ERROR,
         "Error opening certificate file %s: %s", cert_path, strerror(errno));
      return NULL;
   }

   if (!(cert = PEM_read_X509(cert_f, NULL, NULL, NULL)))
   {
      log_ssl_errors(LOG_LEVEL_ERROR,
         "Error reading certificate file %s", cert_path);
   }

   close_file_stream(cert_f, cert_path);
   return cert;
}


/*********************************************************************
 *
 * Function    :  ssl_certificate_is_invalid
 *
 * Description :  Checks whether or not a certificate is valid.
 *                Currently only checks that the certificate can be
 *                parsed and that the "valid to" date is in the future.
 *
 * Parameters  :
 *          1  :  cert_file = The certificate to check
 *
 * Returns     :   0 => The certificate is valid.
 *                 1 => The certificate is invalid
 *
 *********************************************************************/
static int ssl_certificate_is_invalid(const char *cert_file)
{
   int ret;

   X509 *cert = NULL;

   if (!(cert = ssl_certificate_load(cert_file)))
   {
      return 1;
   }

   ret = wolfSSL_X509_cmp_current_time(wolfSSL_X509_get_notAfter(cert));
   if (ret == 0)
   {
      log_ssl_errors(LOG_LEVEL_ERROR,
         "Error checking certificate %s validity", cert_file);
      ret = -1;
   }

   wolfSSL_X509_free(cert);

   return ret == -1 ? 1 : 0;
}


/*********************************************************************
 *
 * Function    :  load_rsa_key
 *
 * Description :  Load a PEM-encoded RSA file into memory.
 *
 * Parameters  :
 *          1  :  rsa_key_path = Path to the file that holds the key.
 *          2  :  password = Password to unlock the key. NULL if no
 *                           password is required.
 *          3  :  rsa_key = Initialized RSA key storage.
 *
 * Returns     :   0 => Error while creating the key.
 *                 1 => It worked
 *
 *********************************************************************/
static int load_rsa_key(const char *rsa_key_path, const char *password, RsaKey *rsa_key)
{
   FILE *fp;
   size_t length;
   long ret;
   unsigned char *key_pem;
   DerBuffer *der_buffer;
   word32 der_index = 0;
   DerBuffer decrypted_der_buffer;
   unsigned char der_data[4096];

   fp = fopen(rsa_key_path, "rb");
   if (NULL == fp)
   {
      log_error(LOG_LEVEL_ERROR, "Failed to open %s: %E", rsa_key_path);
      return 0;
   }

   /* Get file length */
   if (fseek(fp, 0, SEEK_END))
   {
      log_error(LOG_LEVEL_ERROR,
         "Unexpected error while fseek()ing to the end of %s: %E",
         rsa_key_path);
      fclose(fp);
      return 0;
   }
   ret = ftell(fp);
   if (-1 == ret)
   {
      log_error(LOG_LEVEL_ERROR,
         "Unexpected ftell() error while loading %s: %E",
         rsa_key_path);
      fclose(fp);
      return 0;
   }
   length = (size_t)ret;

   /* Go back to the beginning. */
   if (fseek(fp, 0, SEEK_SET))
   {
      log_error(LOG_LEVEL_ERROR,
         "Unexpected error while fseek()ing to the beginning of %s: %E",
         rsa_key_path);
      fclose(fp);
      return 0;
   }

   key_pem = malloc_or_die(length);

   if (1 != fread(key_pem, length, 1, fp))
   {
      log_error(LOG_LEVEL_ERROR,
         "Couldn't completely read file %s.", rsa_key_path);
      fclose(fp);
      freez(key_pem);
      return 0;
   }

   fclose(fp);

   if (password == NULL)
   {
      ret = wc_PemToDer(key_pem, (long)length, PRIVATEKEY_TYPE,
         &der_buffer, NULL, NULL, NULL);
   }
   else
   {
      der_buffer = &decrypted_der_buffer;
      der_buffer->buffer = der_data;
      ret = wc_KeyPemToDer(key_pem, (int)length, der_buffer->buffer,
         sizeof(der_data), password);
      if (ret < 0)
      {
         log_error(LOG_LEVEL_ERROR,
            "Failed to convert PEM key %s into DER format. Error: %ld",
            rsa_key_path, ret);
         freez(key_pem);
         return 0;
      }
      der_buffer->length = (word32)ret;
   }

   freez(key_pem);

   if (ret < 0)
   {
      log_error(LOG_LEVEL_ERROR,
         "Failed to convert buffer into DER format for file %s. Error = %ld",
         rsa_key_path, ret);
      return 0;
   }

   ret = wc_RsaPrivateKeyDecode(der_buffer->buffer, &der_index, rsa_key,
      der_buffer->length);
   if (password == NULL)
   {
      freez(der_buffer);
   }
   if (ret < 0)
   {
      log_error(LOG_LEVEL_ERROR,
         "Failed to decode DER buffer into RSA key structure for %s",
         rsa_key_path);
      return 0;
   }

   return 1;
}

#ifndef WOLFSSL_ALT_NAMES
#error wolfSSL lacks Subject Alternative Name support (WOLFSSL_ALT_NAMES) which is mandatory
#endif
/*********************************************************************
 *
 * Function    :  set_subject_alternative_name
 *
 * Description :  Sets the Subject Alternative Name extension to
 *                a cert using the awesome "API" provided by wolfSSL.
 *
 * Parameters  :
 *          1  :  cert = The certificate to modify
 *          2  :  hostname = The hostname to add
 *
 * Returns     :  <0 => Error.
 *                 0 => It worked
 *
 *********************************************************************/
static int set_subject_alternative_name(struct Cert *certificate, const char *hostname)
{
   const size_t hostname_length = strlen(hostname);

   if (hostname_length >= 253)
   {
      /*
       * We apparently only have a byte to represent the length
       * of the sequence.
       */
      log_error(LOG_LEVEL_ERROR,
         "Hostname '%s' is too long to set Subject Alternative Name",
         hostname);
      return -1;
   }
   certificate->altNames[0] = 0x30; /* Sequence */
   certificate->altNames[1] = (unsigned char)hostname_length + 2;

   certificate->altNames[2] = 0x82; /* DNS name */
   certificate->altNames[3] = (unsigned char)hostname_length;
   memcpy(&certificate->altNames[4], hostname, hostname_length);

   certificate->altNamesSz = (int)hostname_length + 4;

   return 0;
}


/*********************************************************************
 *
 * Function    :  generate_host_certificate
 *
 * Description :  Creates certificate file in presetted directory.
 *                If certificate already exists, no other certificate
 *                will be created. Subject of certificate is named
 *                by csp->http->host from parameter. This function also
 *                triggers generating of private key for new certificate.
 *
 * Parameters  :
 *          1  :  csp = Current client state (buffers, headers, etc...)
 *          2  :  certificate_path = Path to the certficate to generate.
 *          3  :  rsa_key_path = Path to the key to generate for the
 *                               certificate.
 *
 * Returns     :  -1 => Error while creating certificate.
 *                 0 => Certificate already exists.
 *                 1 => Certificate created
 *
 *********************************************************************/
static int generate_host_certificate(struct client_state *csp,
   const char *certificate_path, const char *rsa_key_path)
{
   struct Cert certificate;
   RsaKey ca_key;
   RsaKey rsa_key;
   int ret;
   byte certificate_der[4096];
   int der_certificate_length;
   byte certificate_pem[4096];
   int pem_certificate_length;

   if (file_exists(certificate_path) == 1)
   {
      /* The file exists, but is it valid? */
      if (ssl_certificate_is_invalid(certificate_path))
      {
         log_error(LOG_LEVEL_CONNECT,
            "Certificate %s is no longer valid. Removing it.",
            certificate_path);
         if (unlink(certificate_path))
         {
            log_error(LOG_LEVEL_ERROR, "Failed to unlink %s: %E",
               certificate_path);
            return -1;
         }
         if (unlink(rsa_key_path))
         {
            log_error(LOG_LEVEL_ERROR, "Failed to unlink %s: %E",
               rsa_key_path);
            return -1;
         }
      }
      else
      {
         return 0;
      }
   }
   else
   {
      log_error(LOG_LEVEL_CONNECT, "Creating new certificate %s",
         certificate_path);
   }
   if (enforce_sane_certificate_state(certificate_path, rsa_key_path))
   {
      return -1;
   }

   wc_InitRsaKey(&rsa_key, NULL);
   wc_InitRsaKey(&ca_key, NULL);

   if (generate_rsa_key(rsa_key_path) == -1)
   {
      return -1;
   }

   wc_InitCert(&certificate);

   strncpy(certificate.subject.country, CERT_PARAM_COUNTRY_CODE, CTC_NAME_SIZE);
   strncpy(certificate.subject.org, "Privoxy", CTC_NAME_SIZE);
   strncpy(certificate.subject.unit, "Development", CTC_NAME_SIZE);
   strncpy(certificate.subject.commonName, csp->http->host, CTC_NAME_SIZE);
   certificate.daysValid = 90;
   certificate.selfSigned = 0;
   certificate.sigType = CTC_SHA256wRSA;
   if (!host_is_ip_address(csp->http->host) &&
       set_subject_alternative_name(&certificate, csp->http->host))
   {
      ret = -1;
      goto exit;
   }

   ret = wc_SetIssuer(&certificate, csp->config->ca_cert_file);
   if (ret < 0)
   {
      log_error(LOG_LEVEL_ERROR,
         "Failed to set Issuer file %s", csp->config->ca_cert_file);
      ret = -1;
      goto exit;
   }

   if (load_rsa_key(rsa_key_path, NULL, &rsa_key) != 1)
   {
      log_error(LOG_LEVEL_ERROR,
         "Failed to load RSA key %s", rsa_key_path);
      ret = -1;
      goto exit;
   }

   /* wolfSSL_Debugging_ON(); */
   der_certificate_length = wc_MakeCert(&certificate, certificate_der,
      sizeof(certificate_der), &rsa_key, NULL, &wolfssl_rng);
   /* wolfSSL_Debugging_OFF(); */

   if (der_certificate_length < 0)
   {
      log_error(LOG_LEVEL_ERROR, "Failed to make certificate");
      ret = -1;
      goto exit;
   }

   if (load_rsa_key(csp->config->ca_key_file, csp->config->ca_password,
      &ca_key) != 1)
   {
      log_error(LOG_LEVEL_ERROR,
         "Failed to load CA key %s", csp->config->ca_key_file);
      ret = -1;
      goto exit;
   }

   der_certificate_length = wc_SignCert(certificate.bodySz, certificate.sigType,
      certificate_der, sizeof(certificate_der), &ca_key, NULL, &wolfssl_rng);
   wc_FreeRsaKey(&ca_key);
   if (der_certificate_length < 0)
   {
      log_error(LOG_LEVEL_ERROR, "Failed to sign certificate");
      ret = -1;
      goto exit;
   }

   pem_certificate_length = wc_DerToPem(certificate_der,
      (word32)der_certificate_length, certificate_pem,
      sizeof(certificate_pem), CERT_TYPE);
   if (pem_certificate_length < 0)
   {
      log_error(LOG_LEVEL_ERROR,
         "Failed to convert certificate from DER to PEM");
      ret = -1;
      goto exit;
   }
   certificate_pem[pem_certificate_length] = '\0';

   if (write_certificate(certificate_path, (const char*)certificate_pem))
   {
      ret = -1;
      goto exit;
   }

   ret = 1;

exit:
   wc_FreeRsaKey(&rsa_key);
   wc_FreeRsaKey(&ca_key);

   return 1;

}


/*********************************************************************
 *
 * Function    :  ssl_crt_verify_info
 *
 * Description :  Returns an informational string about the verification
 *                status of a certificate.
 *
 * Parameters  :
 *          1  :  buf = Buffer to write to
 *          2  :  size = Maximum size of buffer
 *          3  :  csp = client state
 *
 * Returns     :  N/A
 *
 *********************************************************************/
extern void ssl_crt_verify_info(char *buf, size_t size, struct client_state *csp)
{
   strncpy(buf, wolfSSL_X509_verify_cert_error_string(
      csp->server_cert_verification_result), size);
   buf[size - 1] = 0;
}


#ifdef FEATURE_GRACEFUL_TERMINATION
/*********************************************************************
 *
 * Function    :  ssl_release
 *
 * Description :  Release all SSL resources
 *
 * Parameters  :
 *
 * Returns     :  N/A
 *
 *********************************************************************/
extern void ssl_release(void)
{
   if (wolfssl_initialized == 1)
   {
      wc_FreeRng(&wolfssl_rng);
      wolfSSL_Cleanup();
   }
}
#endif /* def FEATURE_GRACEFUL_TERMINATION */
