/*********************************************************************
 *
 * File        :  $Source: /cvsroot/ijbswa/current/openssl.c,v $
 *
 * Purpose     :  File with TLS/SSL extension. Contains methods for
 *                creating, using and closing TLS/SSL connections
 *                using OpenSSL (or LibreSSL).
 *
 * Copyright   :  Written by and Copyright (c) 2020 Maxim Antonov <mantonov@gmail.com>
 *                Copyright (C) 2017 Vaclav Svec. FIT CVUT.
 *                Copyright (C) 2018-2022 by Fabian Keil <fk@fabiankeil.de>
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

#include <openssl/bn.h>
#include <openssl/opensslv.h>
#include <openssl/pem.h>
#include <openssl/md5.h>
#include <openssl/x509v3.h>

#include "config.h"
#include "project.h"
#include "miscutil.h"
#include "errlog.h"
#include "encode.h"
#include "jcc.h"
#include "ssl.h"
#include "ssl_common.h"

/*
 * Macros for openssl.c
 */
#define CERTIFICATE_BASIC_CONSTRAINTS            "CA:FALSE"
#define CERTIFICATE_SUBJECT_KEY                  "hash"
#define CERTIFICATE_AUTHORITY_KEY                "keyid:always"
#define CERTIFICATE_ALT_NAME_PREFIX              "DNS:"
#define CERTIFICATE_VERSION                      2
#define VALID_DATETIME_FMT                       "%y%m%d%H%M%SZ"
#define VALID_DATETIME_BUFLEN                    16

static int generate_host_certificate(struct client_state *csp);
static void free_client_ssl_structures(struct client_state *csp);
static void free_server_ssl_structures(struct client_state *csp);
static int ssl_store_cert(struct client_state *csp, X509 *crt);
static void log_ssl_errors(int debuglevel, const char* fmt, ...) __attribute__((format(printf, 2, 3)));

static int ssl_inited = 0;

#if OPENSSL_VERSION_NUMBER < 0x10100000L
#define X509_set1_notBefore X509_set_notBefore
#define X509_set1_notAfter X509_set_notAfter
#define X509_get0_serialNumber X509_get_serialNumber
#define X509_get0_notBefore X509_get_notBefore
#define X509_get0_notAfter X509_get_notAfter
#endif

/*********************************************************************
 *
 * Function    :  openssl_init
 *
 * Description :  Initializes OpenSSL library once
 *
 * Parameters  :  N/A
 *
 * Returns     :  N/A
 *
 *********************************************************************/
static void openssl_init(void)
{
   if (ssl_inited == 0)
   {
      privoxy_mutex_lock(&ssl_init_mutex);
      if (ssl_inited == 0)
      {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
         SSL_library_init();
#else
         OPENSSL_init_ssl(0, NULL);
#endif
         SSL_load_error_strings();
         OpenSSL_add_ssl_algorithms();
         ssl_inited = 1;
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
 *                >0 => Pending data length
 *
 *********************************************************************/
extern size_t is_ssl_pending(struct ssl_attr *ssl_attr)
{
   BIO *bio = ssl_attr->openssl_attr.bio;
   if (bio == NULL)
   {
      return 0;
   }

   return (size_t)BIO_pending(bio);
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
   BIO *bio = ssl_attr->openssl_attr.bio;
   SSL *ssl;
   int ret = 0;
   int pos = 0; /* Position of unsent part in buffer */
   int fd = -1;

   if (len == 0)
   {
      return 0;
   }

   if (BIO_get_ssl(bio, &ssl) == 1)
   {
      fd = SSL_get_fd(ssl);
   }

   while (pos < len)
   {
      int send_len = (int)len - pos;

      log_error(LOG_LEVEL_WRITING, "TLS on socket %d: %N",
         fd, send_len, buf+pos);

      /*
       * Sending one part of the buffer
       */
      while ((ret = BIO_write(bio,
         (const unsigned char *)(buf + pos),
         send_len)) <= 0)
      {
         if (!BIO_should_retry(bio))
         {
            log_ssl_errors(LOG_LEVEL_ERROR,
               "Sending data on socket %d over TLS/SSL failed", fd);
            return -1;
         }
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
   BIO *bio = ssl_attr->openssl_attr.bio;
   SSL *ssl;
   int ret = 0;
   int fd = -1;

   memset(buf, 0, max_length);

   /*
    * Receiving data from SSL context into buffer
    */
   do
   {
      ret = BIO_read(bio, buf, (int)max_length);
   } while (ret <= 0 && BIO_should_retry(bio));

   if (BIO_get_ssl(bio, &ssl) == 1)
   {
      fd = SSL_get_fd(ssl);
   }

   if (ret < 0)
   {
      log_ssl_errors(LOG_LEVEL_ERROR,
         "Receiving data on socket %d over TLS/SSL failed", fd);

      return -1;
   }

   log_error(LOG_LEVEL_RECEIVED, "TLS from socket %d: %N",
      fd, ret, buf);

   return ret;
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
 *          2  :  crt = certificate from trusted chain
 *
 * Returns     :  0 on success and negative value on error
 *
 *********************************************************************/
static int ssl_store_cert(struct client_state *csp, X509 *crt)
{
   long len;
   struct certs_chain  *last = &(csp->server_certs_chain);
   int ret = 0;
   BIO *bio = BIO_new(BIO_s_mem());
   EVP_PKEY *pkey = NULL;
   char *bio_mem_data = NULL;
   char *encoded_text;
   long l;
   const ASN1_INTEGER *bs;
#if OPENSSL_VERSION_NUMBER > 0x10100000L
   const X509_ALGOR *tsig_alg;
#endif
   int loc;

   if (!bio)
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "BIO_new() failed");
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
   last->next = malloc_or_die(sizeof(struct certs_chain));
   last->next->next = NULL;
   memset(last->next->info_buf, 0, sizeof(last->next->info_buf));
   last->next->file_buf = NULL;

   /*
    * Saving certificate file into buffer
    */
   if (!PEM_write_bio_X509(bio, crt))
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "PEM_write_bio_X509() failed");
      ret = -1;
      goto exit;
   }

   len = BIO_get_mem_data(bio, &bio_mem_data);

   last->file_buf = malloc((size_t)len + 1);
   if (last->file_buf == NULL)
   {
      log_error(LOG_LEVEL_ERROR,
         "Failed to allocate %lu bytes to store the X509 PEM certificate",
         len + 1);
      ret = -1;
      goto exit;
   }

   strncpy(last->file_buf, bio_mem_data, (size_t)len);
   last->file_buf[len] = '\0';
   BIO_free(bio);
   bio = BIO_new(BIO_s_mem());
   if (!bio)
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "BIO_new() failed");
      ret = -1;
      goto exit;
   }

   /*
    * Saving certificate information into buffer
    */
   l = X509_get_version(crt);
   if (l >= 0 && l <= 2)
   {
      if (BIO_printf(bio, "cert. version     : %ld\n", l + 1) <= 0)
      {
         log_ssl_errors(LOG_LEVEL_ERROR, "BIO_printf() for version failed");
         ret = -1;
         goto exit;
      }
   }
   else
   {
      if (BIO_printf(bio, "cert. version     : Unknown (%ld)\n", l) <= 0)
      {
         log_ssl_errors(LOG_LEVEL_ERROR, "BIO_printf() for version failed");
         ret = -1;
         goto exit;
      }
   }

   if (BIO_puts(bio, "serial number     : ") <= 0)
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "BIO_puts() for serial failed");
      ret = -1;
      goto exit;
   }
   bs = X509_get0_serialNumber(crt);
   if (bs->length <= (int)sizeof(long))
   {
      ERR_set_mark();
      l = ASN1_INTEGER_get(bs);
      ERR_pop_to_mark();
   }
   else
   {
      l = -1;
   }
   if (l != -1)
   {
      unsigned long ul;
      const char *neg;
      if (bs->type == V_ASN1_NEG_INTEGER)
      {
         ul = 0 - (unsigned long)l;
         neg = "-";
      }
      else
      {
         ul = (unsigned long)l;
         neg = "";
      }
      if (BIO_printf(bio, "%s%lu (%s0x%lx)\n", neg, ul, neg, ul) <= 0)
      {
         log_ssl_errors(LOG_LEVEL_ERROR, "BIO_printf() for serial failed");
         ret = -1;
         goto exit;
      }
   }
   else
   {
      int i;
      if (bs->type == V_ASN1_NEG_INTEGER)
      {
         if (BIO_puts(bio, " (Negative)") < 0)
         {
            log_ssl_errors(LOG_LEVEL_ERROR, "BIO_puts() for serial failed");
            ret = -1;
            goto exit;
         }
      }
      for (i = 0; i < bs->length; i++)
      {
         if (BIO_printf(bio, "%02x%c", bs->data[i],
               ((i + 1 == bs->length) ? '\n' : ':')) <= 0)
         {
            log_ssl_errors(LOG_LEVEL_ERROR, "BIO_printf() for serial failed");
            ret = -1;
            goto exit;
         }
      }
   }

   if (BIO_puts(bio, "issuer name       : ") <= 0)
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "BIO_puts() for issuer failed");
      ret = -1;
      goto exit;
   }
   if (X509_NAME_print_ex(bio, X509_get_issuer_name(crt), 0, 0) < 0)
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "X509_NAME_print_ex() for issuer failed");
      ret = -1;
      goto exit;
   }

   if (BIO_puts(bio, "\nsubject name      : ") <= 0)
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "BIO_puts() for subject failed");
      ret = -1;
      goto exit;
   }
   if (X509_NAME_print_ex(bio, X509_get_subject_name(crt), 0, 0) < 0) {
      log_ssl_errors(LOG_LEVEL_ERROR, "X509_NAME_print_ex() for subject failed");
      ret = -1;
      goto exit;
   }

   if (BIO_puts(bio, "\nissued  on        : ") <= 0)
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "BIO_puts() for issued on failed");
      ret = -1;
      goto exit;
   }
   if (!ASN1_TIME_print(bio, X509_get0_notBefore(crt)))
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "ASN1_TIME_print() for issued on failed");
      ret = -1;
      goto exit;
   }

   if (BIO_puts(bio, "\nexpires on        : ") <= 0)
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "BIO_puts() for expires on failed");
      ret = -1;
      goto exit;
   }
   if (!ASN1_TIME_print(bio, X509_get0_notAfter(crt)))
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "ASN1_TIME_print() for expires on failed");
      ret = -1;
      goto exit;
   }

#if OPENSSL_VERSION_NUMBER > 0x10100000L
   if (BIO_puts(bio, "\nsigned using      : ") <= 0)
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "BIO_puts() for signed using failed");
      ret = -1;
      goto exit;
   }
   tsig_alg = X509_get0_tbs_sigalg(crt);
   if (!i2a_ASN1_OBJECT(bio, tsig_alg->algorithm))
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "i2a_ASN1_OBJECT() for signed using failed");
      ret = -1;
      goto exit;
   }
#endif
   pkey = X509_get_pubkey(crt);
   if (!pkey)
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "X509_get_pubkey() failed");
      ret = -1;
      goto exit;
   }
#define BC              "18"
   switch (EVP_PKEY_base_id(pkey))
   {
      case EVP_PKEY_RSA:
         ret = BIO_printf(bio, "\n%-" BC "s: %d bits", "RSA key size", EVP_PKEY_bits(pkey));
         break;
      case EVP_PKEY_DSA:
         ret = BIO_printf(bio, "\n%-" BC "s: %d bits", "DSA key size", EVP_PKEY_bits(pkey));
         break;
      case EVP_PKEY_EC:
         ret = BIO_printf(bio, "\n%-" BC "s: %d bits", "EC key size", EVP_PKEY_bits(pkey));
         break;
      default:
         ret = BIO_printf(bio, "\n%-" BC "s: %d bits", "non-RSA/DSA/EC key size",
            EVP_PKEY_bits(pkey));
         break;
   }
   if (ret <= 0)
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "BIO_printf() for key size failed");
      ret = -1;
      goto exit;
   }

   loc = X509_get_ext_by_NID(crt, NID_basic_constraints, -1);
   if (loc != -1)
   {
      X509_EXTENSION *ex = X509_get_ext(crt, loc);
      if (BIO_puts(bio, "\nbasic constraints : ") <= 0)
      {
         log_ssl_errors(LOG_LEVEL_ERROR,
            "BIO_printf() for basic constraints failed");
         ret = -1;
         goto exit;
      }
      if (!X509V3_EXT_print(bio, ex, 0, 0))
      {
         if (!ASN1_STRING_print_ex(bio, X509_EXTENSION_get_data(ex), ASN1_STRFLGS_RFC2253))
         {
            log_ssl_errors(LOG_LEVEL_ERROR,
               "ASN1_STRING_print_ex() for basic constraints failed");
            ret = -1;
            goto exit;
         }
      }
   }

   loc = X509_get_ext_by_NID(crt, NID_subject_alt_name, -1);
   if (loc != -1)
   {
      X509_EXTENSION *ex = X509_get_ext(crt, loc);
      if (BIO_puts(bio, "\nsubject alt name  : ") <= 0)
      {
         log_ssl_errors(LOG_LEVEL_ERROR, "BIO_printf() for alt name failed");
         ret = -1;
         goto exit;
      }
      if (!X509V3_EXT_print(bio, ex, 0, 0))
      {
         if (!ASN1_STRING_print_ex(bio, X509_EXTENSION_get_data(ex),
               ASN1_STRFLGS_RFC2253))
         {
            log_ssl_errors(LOG_LEVEL_ERROR,
               "ASN1_STRING_print_ex() for alt name failed");
            ret = -1;
            goto exit;
         }
      }
   }

   loc = X509_get_ext_by_NID(crt, NID_netscape_cert_type, -1);
   if (loc != -1)
   {
      X509_EXTENSION *ex = X509_get_ext(crt, loc);
      if (BIO_puts(bio, "\ncert. type        : ") <= 0)
      {
         log_ssl_errors(LOG_LEVEL_ERROR, "BIO_printf() for cert type failed");
         ret = -1;
         goto exit;
      }
      if (!X509V3_EXT_print(bio, ex, 0, 0))
      {
         if (!ASN1_STRING_print_ex(bio, X509_EXTENSION_get_data(ex),
               ASN1_STRFLGS_RFC2253))
         {
            log_ssl_errors(LOG_LEVEL_ERROR,
               "ASN1_STRING_print_ex() for cert type failed");
            ret = -1;
            goto exit;
         }
      }
   }

   loc = X509_get_ext_by_NID(crt, NID_key_usage, -1);
   if (loc != -1)
   {
      X509_EXTENSION *ex = X509_get_ext(crt, loc);
      if (BIO_puts(bio, "\nkey usage         : ") <= 0)
      {
         log_ssl_errors(LOG_LEVEL_ERROR, "BIO_printf() for key usage failed");
         ret = -1;
         goto exit;
      }
      if (!X509V3_EXT_print(bio, ex, 0, 0))
      {
         if (!ASN1_STRING_print_ex(bio, X509_EXTENSION_get_data(ex),
               ASN1_STRFLGS_RFC2253))
         {
            log_ssl_errors(LOG_LEVEL_ERROR,
               "ASN1_STRING_print_ex() for key usage failed");
            ret = -1;
            goto exit;
         }
      }
   }

   loc = X509_get_ext_by_NID(crt, NID_ext_key_usage, -1);
   if (loc != -1) {
      X509_EXTENSION *ex = X509_get_ext(crt, loc);
      if (BIO_puts(bio, "\next key usage     : ") <= 0)
      {
         log_ssl_errors(LOG_LEVEL_ERROR,
            "BIO_printf() for ext key usage failed");
         ret = -1;
         goto exit;
      }
      if (!X509V3_EXT_print(bio, ex, 0, 0))
      {
         if (!ASN1_STRING_print_ex(bio, X509_EXTENSION_get_data(ex),
               ASN1_STRFLGS_RFC2253))
         {
            log_ssl_errors(LOG_LEVEL_ERROR,
               "ASN1_STRING_print_ex() for ext key usage failed");
            ret = -1;
            goto exit;
         }
      }
   }

   loc = X509_get_ext_by_NID(crt, NID_certificate_policies, -1);
   if (loc != -1)
   {
      X509_EXTENSION *ex = X509_get_ext(crt, loc);
      if (BIO_puts(bio, "\ncertificate policies : ") <= 0)
      {
         log_ssl_errors(LOG_LEVEL_ERROR, "BIO_printf() for certificate policies failed");
         ret = -1;
         goto exit;
      }
      if (!X509V3_EXT_print(bio, ex, 0, 0))
      {
         if (!ASN1_STRING_print_ex(bio, X509_EXTENSION_get_data(ex),
               ASN1_STRFLGS_RFC2253))
         {
            log_ssl_errors(LOG_LEVEL_ERROR,
               "ASN1_STRING_print_ex() for certificate policies failed");
            ret = -1;
            goto exit;
         }
      }
   }

   /* make valgrind happy */
   static const char zero = 0;
   BIO_write(bio, &zero, 1);

   len = BIO_get_mem_data(bio, &bio_mem_data);
   if (len <= 0)
   {
      log_error(LOG_LEVEL_ERROR, "BIO_get_mem_data() returned %ld "
         "while gathering certificate information", len);
      ret = -1;
      goto exit;
   }
   encoded_text = html_encode(bio_mem_data);
   if (encoded_text == NULL)
   {
      log_error(LOG_LEVEL_ERROR,
         "Failed to HTML-encode the certificate information");
      ret = -1;
      goto exit;
   }

   strlcpy(last->info_buf, encoded_text, sizeof(last->info_buf));
   freez(encoded_text);
   ret = 0;

exit:
   if (bio)
   {
      BIO_free(bio);
   }
   if (pkey)
   {
      EVP_PKEY_free(pkey);
   }
   return ret;
}


/*********************************************************************
 *
 * Function    :  host_to_hash
 *
 * Description :  Creates MD5 hash from host name. Host name is loaded
 *                from structure csp and saved again into it.
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
   int ret = 0;

   memset(csp->http->hash_of_host, 0, sizeof(csp->http->hash_of_host));
   MD5((unsigned char *)csp->http->host, strlen(csp->http->host),
      csp->http->hash_of_host);

   /* Converting hash into string with hex */
   size_t i = 0;
   for (; i < 16; i++)
   {
      if ((ret = sprintf((char *)csp->http->hash_of_host_hex + 2 * i, "%02x",
         csp->http->hash_of_host[i])) < 0)
      {
         log_error(LOG_LEVEL_ERROR, "Sprintf return value: %d", ret);
         return -1;
      }
   }

   return 0;
}


/*********************************************************************
 *
 * Function    :  create_client_ssl_connection
 *
 * Description :  Creates TLS/SSL secured connection with client
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
   SSL *ssl;

   /*
    * Initializing OpenSSL structures for TLS/SSL connection
    */
   openssl_init();

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

   /*
    * Generating certificate for requested host. Mutex to prevent
    * certificate and key inconsistence must be locked.
    */
   privoxy_mutex_lock(&certificate_mutex);

   ret = generate_host_certificate(csp);
   if (ret < 0)
   {
      log_error(LOG_LEVEL_ERROR,
         "generate_host_certificate failed: %d", ret);
      privoxy_mutex_unlock(&certificate_mutex);
      ret = -1;
      goto exit;
   }
   privoxy_mutex_unlock(&certificate_mutex);

   if (!(ssl_attr->openssl_attr.ctx = SSL_CTX_new(SSLv23_server_method())))
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "Unable to create SSL context");
      ret = -1;
      goto exit;
   }

   /* Set the key and cert */
   if (SSL_CTX_use_certificate_file(ssl_attr->openssl_attr.ctx,
         cert_file, SSL_FILETYPE_PEM) != 1)
   {
      log_ssl_errors(LOG_LEVEL_ERROR,
         "Loading webpage certificate %s failed", cert_file);
      ret = -1;
      goto exit;
   }

   if (SSL_CTX_use_PrivateKey_file(ssl_attr->openssl_attr.ctx,
         key_file, SSL_FILETYPE_PEM) != 1)
   {
      log_ssl_errors(LOG_LEVEL_ERROR,
         "Loading webpage certificate private key %s failed", key_file);
      ret = -1;
      goto exit;
   }

   SSL_CTX_set_options(ssl_attr->openssl_attr.ctx, SSL_OP_NO_SSLv3);

   if (!(ssl_attr->openssl_attr.bio = BIO_new_ssl(ssl_attr->openssl_attr.ctx, 0)))
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "Unable to create BIO structure");
      ret = -1;
      goto exit;
   }

   if (BIO_get_ssl(ssl_attr->openssl_attr.bio, &ssl) != 1)
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "BIO_get_ssl failed");
      ret = -1;
      goto exit;
   }

   if (!SSL_set_fd(ssl, csp->cfd))
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "SSL_set_fd failed");
      ret = -1;
      goto exit;
   }

   if (csp->config->cipher_list != NULL)
   {
      if (!SSL_set_cipher_list(ssl, csp->config->cipher_list))
      {
         log_ssl_errors(LOG_LEVEL_ERROR,
            "Setting the cipher list '%s' for the client connection failed",
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
   if (BIO_do_handshake(ssl_attr->openssl_attr.bio) != 1)
   {
       log_ssl_errors(LOG_LEVEL_ERROR,
          "The TLS/SSL handshake with the client failed");
       ret = -1;
       goto exit;
   }

   log_error(LOG_LEVEL_CONNECT, "Client successfully connected over %s (%s).",
      SSL_get_version(ssl), SSL_get_cipher_name(ssl));

   csp->ssl_with_client_is_opened = 1;
   ret = 0;

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
 * Function    :  close_client_ssl_connection
 *
 * Description :  Closes TLS/SSL connection with client. This function
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
   SSL *ssl;

   if (csp->ssl_with_client_is_opened == 0)
   {
      return;
   }

   /*
    * Notifying the peer that the connection is being closed.
    */
   BIO_ssl_shutdown(ssl_attr->openssl_attr.bio);
   if (BIO_get_ssl(ssl_attr->openssl_attr.bio, &ssl) != 1)
   {
      log_ssl_errors(LOG_LEVEL_ERROR,
         "BIO_get_ssl() failed in close_client_ssl_connection()");
   }
   else
   {
      /*
       * Pretend we received a shutdown alert so
       * the BIO_free_all() call later on returns
       * quickly.
       */
      SSL_set_shutdown(ssl, SSL_RECEIVED_SHUTDOWN);
   }
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

   if (ssl_attr->openssl_attr.bio)
   {
      BIO_free_all(ssl_attr->openssl_attr.bio);
   }
   if (ssl_attr->openssl_attr.ctx)
   {
      SSL_CTX_free(ssl_attr->openssl_attr.ctx);
   }
}


/*********************************************************************
 *
 * Function    :  close_server_ssl_connection
 *
 * Description :  Closes TLS/SSL connection with server. This function
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
   SSL *ssl;

   if (csp->ssl_with_server_is_opened == 0)
   {
      return;
   }

   /*
   * Notifying the peer that the connection is being closed.
   */
   BIO_ssl_shutdown(ssl_attr->openssl_attr.bio);
   if (BIO_get_ssl(ssl_attr->openssl_attr.bio, &ssl) != 1)
   {
      log_ssl_errors(LOG_LEVEL_ERROR,
         "BIO_get_ssl() failed in close_server_ssl_connection()");
   }
   else
   {
      /*
       * Pretend we received a shutdown alert so
       * the BIO_free_all() call later on returns
       * quickly.
       */
      SSL_set_shutdown(ssl, SSL_RECEIVED_SHUTDOWN);
   }
   free_server_ssl_structures(csp);
   csp->ssl_with_server_is_opened = 0;
}


/*********************************************************************
 *
 * Function    :  create_server_ssl_connection
 *
 * Description :  Creates TLS/SSL secured connection with server.
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
   openssl_connection_attr *ssl_attrs = &csp->ssl_server_attr.openssl_attr;
   int ret = 0;
   char *trusted_cas_file = NULL;
   STACK_OF(X509) *chain;
   SSL *ssl;

   csp->server_cert_verification_result = SSL_CERT_NOT_VERIFIED;
   csp->server_certs_chain.next = NULL;

   /* Setting path to file with trusted CAs */
   trusted_cas_file = csp->config->trusted_cas_file;

   ssl_attrs->ctx = SSL_CTX_new(SSLv23_method());
   if (!ssl_attrs->ctx)
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "SSL context creation failed");
      ret = -1;
      goto exit;
   }

   /*
    * Loading file with trusted CAs
    */
   if (!SSL_CTX_load_verify_locations(ssl_attrs->ctx, trusted_cas_file, NULL))
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "Loading trusted CAs file %s failed",
         trusted_cas_file);
      ret = -1;
      goto exit;
   }

   SSL_CTX_set_verify(ssl_attrs->ctx, SSL_VERIFY_NONE, NULL);
   SSL_CTX_set_options(ssl_attrs->ctx, SSL_OP_NO_SSLv3);

   if (!(ssl_attrs->bio = BIO_new_ssl(ssl_attrs->ctx, 1)))
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "Unable to create BIO structure");
      ret = -1;
      goto exit;
   }

   if (BIO_get_ssl(ssl_attrs->bio, &ssl) != 1)
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "BIO_get_ssl failed");
      ret = -1;
      goto exit;
   }

   if (!SSL_set_fd(ssl, csp->server_connection.sfd))
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "SSL_set_fd failed");
      ret = -1;
      goto exit;
   }

   if (csp->config->cipher_list != NULL)
   {
      if (!SSL_set_cipher_list(ssl, csp->config->cipher_list))
      {
         log_ssl_errors(LOG_LEVEL_ERROR,
            "Setting the cipher list '%s' for the server connection failed",
            csp->config->cipher_list);
         ret = -1;
         goto exit;
      }
   }

   /*
    * Set the hostname to check against the received server certificate
    */
#if OPENSSL_VERSION_NUMBER > 0x10100000L
   if (!SSL_set1_host(ssl, csp->http->host))
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "SSL_set1_host failed");
      ret = -1;
      goto exit;
   }
#else
   if (host_is_ip_address(csp->http->host))
   {
      if (X509_VERIFY_PARAM_set1_ip_asc(ssl->param,  csp->http->host) != 1)
      {
         log_ssl_errors(LOG_LEVEL_ERROR,
            "X509_VERIFY_PARAM_set1_ip_asc() failed");
         ret = -1;
         goto exit;
      }
   }
   else
   {
      if (X509_VERIFY_PARAM_set1_host(ssl->param,  csp->http->host, 0) != 1)
      {
         log_ssl_errors(LOG_LEVEL_ERROR,
            "X509_VERIFY_PARAM_set1_host() failed");
         ret = -1;
         goto exit;
      }
   }
#endif
   /* SNI extension */
   if (!SSL_set_tlsext_host_name(ssl, csp->http->host))
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "SSL_set_tlsext_host_name failed");
      ret = -1;
      goto exit;
   }

   /*
    * Handshake with server
    */
   log_error(LOG_LEVEL_CONNECT,
      "Performing the TLS/SSL handshake with the server");

   if (BIO_do_handshake(ssl_attrs->bio) != 1)
   {
      log_ssl_errors(LOG_LEVEL_ERROR,
         "The TLS/SSL handshake with the server failed");
      ret = -1;
      goto exit;
   }

   /*
    * XXX: Do we really have to do this always?
    *      Probably it's sufficient to do if the verification fails
    *      in which case we're sending the certificates to the client.
    */
   chain = SSL_get_peer_cert_chain(ssl);
   if (chain)
   {
      int i;
      for (i = 0; i < sk_X509_num(chain); i++)
      {
         if (ssl_store_cert(csp, sk_X509_value(chain, i)) != 0)
         {
            log_error(LOG_LEVEL_ERROR, "ssl_store_cert failed");
            ret = -1;
            goto exit;
         }
      }
   }

   if (!csp->dont_verify_certificate)
   {
      long verify_result = SSL_get_verify_result(ssl);
      if (verify_result == X509_V_OK)
      {
         ret = 0;
         csp->server_cert_verification_result = SSL_CERT_VALID;
      }
      else
      {
         csp->server_cert_verification_result = verify_result;
         log_error(LOG_LEVEL_ERROR,
            "X509 certificate verification for %s failed: %s",
            csp->http->hostport, X509_verify_cert_error_string(verify_result));
         ret = -1;
         goto exit;
      }
   }

   log_error(LOG_LEVEL_CONNECT, "Server successfully connected over %s (%s).",
     SSL_get_version(ssl), SSL_get_cipher_name(ssl));

   /*
    * Server certificate chain is valid, so we can clean
    * chain, because we will not send it to client.
    */
   free_certificate_chain(csp);

   csp->ssl_with_server_is_opened = 1;
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

   if (ssl_attr->openssl_attr.bio)
   {
      BIO_free_all(ssl_attr->openssl_attr.bio);
   }
   if (ssl_attr->openssl_attr.ctx)
   {
      SSL_CTX_free(ssl_attr->openssl_attr.ctx);
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

   while ((err_code = ERR_get_error()))
   {
      char err_buf[ERROR_BUF_SIZE];
      reported = 1;
      ERR_error_string_n(err_code, err_buf, sizeof(err_buf));
      log_error(debuglevel, "%s: %s", prefix, err_buf);
   }
   va_end(args);
   /*
    * In case if called by mistake and there were
    * no TLS/SSL errors let's report it to the log.
    */
   if (!reported)
   {
      log_error(debuglevel, "%s: no TLS/SSL errors detected", prefix);
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
   *olen = 4 * ((slen/3) + ((slen%3) ? 1 : 0)) + 1;
   if (*olen > dlen)
   {
      return ENOBUFS;
   }
   *olen = (size_t)EVP_EncodeBlock(dst, src, (int)slen) + 1;
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
 * Description :  Writes certificate into file.
 *
 * Parameters  :
 *          1  :  crt = certificate to write into file
 *          2  :  output_file = path to save certificate file
 *
 *                on error
 * Returns     :  1 on success success or negative value
 *
 *********************************************************************/
static int write_certificate(X509 *crt, const char *output_file)
{
   FILE *f = NULL;
   int ret = -1;

   /*
    * Saving certificate into file
    */
   if ((f = fopen(output_file, "w")) == NULL)
   {
      log_error(LOG_LEVEL_ERROR, "Opening file %s to save certificate failed",
         output_file);
      return ret;
   }

   ret = PEM_write_X509(f, crt);
   if (!ret)
   {
      log_ssl_errors(LOG_LEVEL_ERROR,
         "Writing certificate into file %s failed", output_file);
      ret = -1;
   }

   close_file_stream(f, output_file);

   return ret;
}

/*********************************************************************
 *
 * Function    :  write_private_key
 *
 * Description :  Writes private key into file and copies saved
 *                content into given pointer to string. If function
 *                returns 0 for success, this copy must be freed by
 *                caller.
 *
 * Parameters  :
 *          1  :  key = key to write into file
 *          2  :  ret_buf = pointer to string with created key file content
 *          3  :  key_file_path = path where to save key file
 *
 * Returns     :  Length of written private key on success or negative value
 *                on error
 *
 *********************************************************************/
static int write_private_key(EVP_PKEY *key, char **ret_buf,
                             const char *key_file_path)
{
   size_t len = 0;                /* Length of created key    */
   FILE *f = NULL;                /* File to save certificate */
   int ret = 0;
   BIO *bio_mem = BIO_new(BIO_s_mem());
   char *bio_mem_data = 0;

   if (bio_mem == NULL)
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "write_private_key memory allocation failure");
      return -1;
   }

   /*
    * Writing private key into PEM string
    */
   if (!PEM_write_bio_PrivateKey(bio_mem, key, NULL, NULL, 0, NULL, NULL))
   {
      log_ssl_errors(LOG_LEVEL_ERROR,
         "Writing private key into PEM string failed");
      ret = -1;
      goto exit;
   }

   len = (size_t)BIO_get_mem_data(bio_mem, &bio_mem_data);

   /* Initializing buffer for key file content */
   *ret_buf = zalloc_or_die(len + 1);
   (*ret_buf)[len] = 0;

   strncpy(*ret_buf, bio_mem_data, len);

   /*
    * Saving key into file
    */
   if ((f = fopen(key_file_path, "wb")) == NULL)
   {
      log_error(LOG_LEVEL_ERROR,
         "Opening file %s to save private key failed: %E",
         key_file_path);
      ret = -1;
      goto exit;
   }

   if (fwrite(*ret_buf, 1, len, f) != len)
   {
      log_error(LOG_LEVEL_ERROR,
         "Writing private key into file %s failed",
         key_file_path);
      close_file_stream(f, key_file_path);
      ret = -1;
      goto exit;
   }

   close_file_stream(f, key_file_path);

exit:
   BIO_free(bio_mem);
   if (ret < 0)
   {
      freez(*ret_buf);
      *ret_buf = NULL;
      return ret;
   }
   return (int)len;
}


/*********************************************************************
 *
 * Function    :  generate_key
 *
 * Description : Tests if private key for host saved in csp already
 *               exists.  If this file doesn't exists, a new key is
 *               generated and saved in a file. The generated key is also
 *               copied into given parameter key_buf, which must be then
 *               freed by caller. If file with key exists, key_buf
 *               contain NULL and no private key is generated.
 *
 * Parameters  :
 *          1  :  csp = Current client state (buffers, headers, etc...)
 *          2  :  key_buf = buffer to save new generated key
 *
 * Returns     :  -1 => Error while generating private key
 *                 0 => Key already exists
 *                >0 => Length of generated private key
 *
 *********************************************************************/
static int generate_key(struct client_state *csp, char **key_buf)
{
   int ret = 0;
   char* key_file_path;
   BIGNUM *exp;
   RSA *rsa;
   EVP_PKEY *key;

   key_file_path = make_certs_path(csp->config->certificate_directory,
      (char *)csp->http->hash_of_host_hex, KEY_FILE_TYPE);
   if (key_file_path == NULL)
   {
      return -1;
   }

   /*
    * Test if key already exists. If so, we don't have to create it again.
    */
   if (file_exists(key_file_path) == 1)
   {
      freez(key_file_path);
      return 0;
   }

   exp = BN_new();
   rsa = RSA_new();
   key = EVP_PKEY_new();
   if (exp == NULL || rsa == NULL || key == NULL)
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "RSA key memory allocation failure");
      ret = -1;
      goto exit;
   }

   if (BN_set_word(exp, RSA_KEY_PUBLIC_EXPONENT) != 1)
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "Setting RSA key exponent failed");
      ret = -1;
      goto exit;
   }

   ret = RSA_generate_key_ex(rsa, RSA_KEYSIZE, exp, NULL);
   if (ret == 0)
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "RSA key generation failure");
      ret = -1;
      goto exit;
   }

   if (!EVP_PKEY_set1_RSA(key, rsa))
   {
      log_ssl_errors(LOG_LEVEL_ERROR,
         "Error assigning RSA key pair to PKEY structure");
      ret = -1;
      goto exit;
   }

   /*
    * Exporting private key into file
    */
   if ((ret = write_private_key(key, key_buf, key_file_path)) < 0)
   {
      log_error(LOG_LEVEL_ERROR,
         "Writing private key into file %s failed", key_file_path);
      ret = -1;
      goto exit;
   }

exit:
   /*
    * Freeing used variables
    */
   if (exp)
   {
      BN_free(exp);
   }
   if (rsa)
   {
      RSA_free(rsa);
   }
   if (key)
   {
      EVP_PKEY_free(key);
   }
   freez(key_file_path);

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

   ret = X509_cmp_current_time(X509_get_notAfter(cert));
   if (ret == 0)
   {
      log_ssl_errors(LOG_LEVEL_ERROR,
         "Error checking certificate %s validity", cert_file);
      ret = -1;
   }

   X509_free(cert);

   return ret == -1 ? 1 : 0;
}


/*********************************************************************
 *
 * Function    :  set_x509_ext
 *
 * Description :  Sets the X509V3 extension data
 *
 * Parameters  :
 *          1  :  cert = The certificate to modify
 *          2  :  issuer = Issuer certificate
 *          3  :  nid = OpenSSL NID
 *          4  :  value = extension value
 *
 * Returns     :   0 => Error while setting extension data
 *                 1 => It worked
 *
 *********************************************************************/
static int set_x509_ext(X509 *cert, X509 *issuer, int nid, char *value)
{
   X509_EXTENSION *ext = NULL;
   X509V3_CTX ctx;
   int ret = 0;

   X509V3_set_ctx(&ctx, issuer, cert, NULL, NULL, 0);
   ext = X509V3_EXT_conf_nid(NULL, &ctx, nid, value);
   if (!ext)
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "X509V3_EXT_conf_nid failure");
      goto exit;
   }

   if (!X509_add_ext(cert, ext, -1))
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "X509_add_ext failure");
      goto exit;
   }

   ret = 1;
exit:
   if (ext)
   {
      X509_EXTENSION_free(ext);
   }
   return ret;
}


/*********************************************************************
 *
 * Function    :  set_subject_alternative_name
 *
 * Description :  Sets the Subject Alternative Name extension to a cert
 *
 * Parameters  :
 *          1  :  cert = The certificate to modify
 *          2  :  issuer = Issuer certificate
 *          3  :  hostname = The hostname to add
 *
 * Returns     :   0 => Error while creating certificate.
 *                 1 => It worked
 *
 *********************************************************************/
static int set_subject_alternative_name(X509 *cert, X509 *issuer, const char *hostname)
{
   size_t altname_len = strlen(hostname) + sizeof(CERTIFICATE_ALT_NAME_PREFIX);
   char alt_name_buf[altname_len];

   snprintf(alt_name_buf, sizeof(alt_name_buf),
      CERTIFICATE_ALT_NAME_PREFIX"%s", hostname);
   return set_x509_ext(cert, issuer, NID_subject_alt_name, alt_name_buf);
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
 *
 * Returns     :  -1 => Error while creating certificate.
 *                 0 => Certificate already exists.
 *                 1 => Certificate created
 *
 *********************************************************************/
static int generate_host_certificate(struct client_state *csp)
{
   char *key_buf = NULL;    /* Buffer for created key */
   X509 *issuer_cert = NULL;
   X509 *cert = NULL;
   BIO *pk_bio = NULL;
   EVP_PKEY *loaded_subject_key = NULL;
   EVP_PKEY *loaded_issuer_key = NULL;
   X509_NAME *issuer_name;
   X509_NAME *subject_name = NULL;
   ASN1_TIME *asn_time = NULL;
   ASN1_INTEGER *serial = NULL;
   BIGNUM *serial_num = NULL;

   int ret = 0;
   cert_options cert_opt;
   char cert_valid_from[VALID_DATETIME_BUFLEN];
   char cert_valid_to[VALID_DATETIME_BUFLEN];
   const char *common_name;
   enum { CERT_PARAM_COMMON_NAME_MAX = 64 };

   /* Paths to keys and certificates needed to create certificate */
   cert_opt.issuer_key  = NULL;
   cert_opt.subject_key = NULL;
   cert_opt.issuer_crt  = NULL;

   cert_opt.output_file = make_certs_path(csp->config->certificate_directory,
      (const char *)csp->http->hash_of_host_hex, CERT_FILE_TYPE);
   if (cert_opt.output_file == NULL)
   {
      return -1;
   }

   cert_opt.subject_key = make_certs_path(csp->config->certificate_directory,
      (const char *)csp->http->hash_of_host_hex, KEY_FILE_TYPE);
   if (cert_opt.subject_key == NULL)
   {
      freez(cert_opt.output_file);
      return -1;
   }

   if (enforce_sane_certificate_state(cert_opt.output_file,
         cert_opt.subject_key))
   {
      freez(cert_opt.output_file);
      freez(cert_opt.subject_key);

      return -1;
   }

   if (file_exists(cert_opt.output_file) == 1)
   {
      /* The file exists, but is it valid? */
      if (ssl_certificate_is_invalid(cert_opt.output_file))
      {
         log_error(LOG_LEVEL_CONNECT,
            "Certificate %s is no longer valid. Removing it.",
            cert_opt.output_file);
         if (unlink(cert_opt.output_file))
         {
            log_error(LOG_LEVEL_ERROR, "Failed to unlink %s: %E",
               cert_opt.output_file);

            freez(cert_opt.output_file);
            freez(cert_opt.subject_key);

            return -1;
         }
         if (unlink(cert_opt.subject_key))
         {
            log_error(LOG_LEVEL_ERROR, "Failed to unlink %s: %E",
               cert_opt.subject_key);

            freez(cert_opt.output_file);
            freez(cert_opt.subject_key);

            return -1;
         }
      }
      else
      {
         freez(cert_opt.output_file);
         freez(cert_opt.subject_key);

         return 0;
      }
   }

   /*
    * Create key for requested host
    */
   int subject_key_len = generate_key(csp, &key_buf);
   if (subject_key_len < 0)
   {
      freez(cert_opt.output_file);
      freez(cert_opt.subject_key);
      log_error(LOG_LEVEL_ERROR, "Key generating failed");
      return -1;
   }

   /*
    * Converting unsigned long serial number to char * serial number.
    * We must compute length of serial number in string + terminating null.
    */
   unsigned long certificate_serial = get_certificate_serial(csp);
   unsigned long certificate_serial_time = (unsigned long)time(NULL);
   int serial_num_size = snprintf(NULL, 0, "%lu%lu",
      certificate_serial_time, certificate_serial) + 1;
   if (serial_num_size <= 0)
   {
      serial_num_size = 1;
   }

   char serial_num_text[serial_num_size];  /* Buffer for serial number */
   ret = snprintf(serial_num_text, (size_t)serial_num_size, "%lu%lu",
      certificate_serial_time, certificate_serial);
   if (ret < 0 || ret >= serial_num_size)
   {
      log_error(LOG_LEVEL_ERROR,
         "Converting certificate serial number into string failed");
      ret = -1;
      goto exit;
   }

   /*
    * Preparing parameters for certificate
    */
   subject_name = X509_NAME_new();
   if (!subject_name)
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "X509 memory allocation failure");
      ret = -1;
      goto exit;
   }

   /*
    * Make sure OpenSSL doesn't reject the common name due to its length.
    * The clients should only care about the Subject Alternative Name anyway
    * and we always use the real host name for that.
    */
   common_name = (strlen(csp->http->host) > CERT_PARAM_COMMON_NAME_MAX) ?
      CGI_SITE_2_HOST : csp->http->host;
   if (!X509_NAME_add_entry_by_txt(subject_name, CERT_PARAM_COMMON_NAME_FCODE,
         MBSTRING_ASC, (void *)common_name, -1, -1, 0))
   {
      log_ssl_errors(LOG_LEVEL_ERROR,
         "X509 subject name (code: %s, val: %s) error",
         CERT_PARAM_COMMON_NAME_FCODE, csp->http->host);
      ret = -1;
      goto exit;
   }
   if (!X509_NAME_add_entry_by_txt(subject_name, CERT_PARAM_ORGANIZATION_FCODE,
         MBSTRING_ASC, (void *)common_name, -1, -1, 0))
   {
      log_ssl_errors(LOG_LEVEL_ERROR,
         "X509 subject name (code: %s, val: %s) error",
         CERT_PARAM_ORGANIZATION_FCODE, csp->http->host);
      ret = -1;
      goto exit;
   }
   if (!X509_NAME_add_entry_by_txt(subject_name, CERT_PARAM_ORG_UNIT_FCODE,
         MBSTRING_ASC, (void *)common_name, -1, -1, 0))
   {
      log_ssl_errors(LOG_LEVEL_ERROR,
         "X509 subject name (code: %s, val: %s) error",
         CERT_PARAM_ORG_UNIT_FCODE, csp->http->host);
      ret = -1;
      goto exit;
   }
   if (!X509_NAME_add_entry_by_txt(subject_name, CERT_PARAM_COUNTRY_FCODE,
         MBSTRING_ASC, (void *)CERT_PARAM_COUNTRY_CODE, -1, -1, 0))
   {
      log_ssl_errors(LOG_LEVEL_ERROR,
         "X509 subject name (code: %s, val: %s) error",
         CERT_PARAM_COUNTRY_FCODE, CERT_PARAM_COUNTRY_CODE);
      ret = -1;
      goto exit;
   }

   cert_opt.issuer_crt = csp->config->ca_cert_file;
   cert_opt.issuer_key = csp->config->ca_key_file;

   if (get_certificate_valid_from_date(cert_valid_from,
         sizeof(cert_valid_from), VALID_DATETIME_FMT)
    || get_certificate_valid_to_date(cert_valid_to,
         sizeof(cert_valid_to), VALID_DATETIME_FMT))
   {
      log_error(LOG_LEVEL_ERROR, "Generating one of the validity dates failed");
      ret = -1;
      goto exit;
   }

   cert_opt.subject_pwd = CERT_SUBJECT_PASSWORD;
   cert_opt.issuer_pwd  = csp->config->ca_password;
   cert_opt.not_before  = cert_valid_from;
   cert_opt.not_after   = cert_valid_to;
   cert_opt.serial      = serial_num_text;
   cert_opt.max_pathlen = -1;

   /*
    * Test if the private key was already created.
    * XXX: Can this still happen?
    */
   if (subject_key_len == 0)
   {
      log_error(LOG_LEVEL_ERROR, "Subject key was already created");
      ret = 0;
      goto exit;
   }

   /*
    * Parse serial to MPI
    */
   serial_num = BN_new();
   if (!serial_num)
   {
      log_error(LOG_LEVEL_ERROR, "generate_host_certificate: memory error");
      ret = -1;
      goto exit;
   }
   if (!BN_dec2bn(&serial_num, cert_opt.serial))
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "Failed to parse serial %s", cert_opt.serial);
      ret = -1;
      goto exit;
   }

   if (!(serial = BN_to_ASN1_INTEGER(serial_num, NULL)))
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "Failed to generate serial ASN1 representation");
      ret = -1;
      goto exit;
   }

   /*
    * Loading certificates
    */
   if (!(issuer_cert = ssl_certificate_load(cert_opt.issuer_crt)))
   {
      log_error(LOG_LEVEL_ERROR, "Loading issuer certificate %s failed",
         cert_opt.issuer_crt);
      ret = -1;
      goto exit;
   }

   issuer_name = X509_get_subject_name(issuer_cert);

   /*
    * Loading keys from file or from buffer
    */
   if (key_buf != NULL && subject_key_len > 0)
   {
      pk_bio = BIO_new_mem_buf(key_buf, subject_key_len);
   }
   else if (!(pk_bio = BIO_new_file(cert_opt.subject_key, "r")))
   {
      log_ssl_errors(LOG_LEVEL_ERROR,
         "Failure opening subject key %s BIO", cert_opt.subject_key);
      ret = -1;
      goto exit;
   }

   loaded_subject_key = PEM_read_bio_PrivateKey(pk_bio, NULL, NULL,
      (void *)cert_opt.subject_pwd);
   if (!loaded_subject_key)
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "Parsing subject key %s failed",
         cert_opt.subject_key);
      ret = -1;
      goto exit;
   }

   if (!BIO_free(pk_bio))
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "Error closing subject key BIO");
   }

   if (!(pk_bio = BIO_new_file(cert_opt.issuer_key, "r")))
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "Failure opening issuer key %s BIO",
         cert_opt.issuer_key);
      ret = -1;
      goto exit;
   }

   loaded_issuer_key = PEM_read_bio_PrivateKey(pk_bio, NULL, NULL,
      (void *)cert_opt.issuer_pwd);
   if (!loaded_issuer_key)
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "Parsing issuer key %s failed",
         cert_opt.subject_key);
      ret = -1;
      goto exit;
   }

   cert = X509_new();
   if (!cert)
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "Certificate allocation error");
      ret = -1;
      goto exit;
   }

   if (!X509_set_version(cert, CERTIFICATE_VERSION))
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "X509_set_version failed");
      ret = -1;
      goto exit;
   }

   /*
    * Setting parameters of signed certificate
    */
   if (!X509_set_pubkey(cert, loaded_subject_key))
   {
      log_ssl_errors(LOG_LEVEL_ERROR,
         "Setting public key in signed certificate failed");
      ret = -1;
      goto exit;
   }

   if (!X509_set_subject_name(cert, subject_name))
   {
      log_ssl_errors(LOG_LEVEL_ERROR,
         "Setting subject name in signed certificate failed");
      ret = -1;
      goto exit;
   }

   if (!X509_set_issuer_name(cert, issuer_name))
   {
      log_ssl_errors(LOG_LEVEL_ERROR,
         "Setting issuer name in signed certificate failed");
      ret = -1;
      goto exit;
   }

   if (!X509_set_serialNumber(cert, serial))
   {
      log_ssl_errors(LOG_LEVEL_ERROR,
         "Setting serial number in signed certificate failed");
      ret = -1;
      goto exit;
   }

   asn_time = ASN1_TIME_new();
   if (!asn_time)
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "ASN1 time memory allocation failure");
      ret = -1;
      goto exit;
   }

   if (!ASN1_TIME_set_string(asn_time, cert_opt.not_after))
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "ASN1 time [%s] encode error", cert_opt.not_after);
      ret = -1;
      goto exit;
   }

   if (!X509_set1_notAfter(cert, asn_time))
   {
      log_ssl_errors(LOG_LEVEL_ERROR,
         "Setting valid not after in signed certificate failed");
      ret = -1;
      goto exit;
   }

   if (!ASN1_TIME_set_string(asn_time, cert_opt.not_before))
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "ASN1 time encode error");
      ret = -1;
      goto exit;
   }

   if (!X509_set1_notBefore(cert, asn_time))
   {
      log_ssl_errors(LOG_LEVEL_ERROR,
         "Setting valid not before in signed certificate failed");
      ret = -1;
      goto exit;
   }

   if (!set_x509_ext(cert, issuer_cert, NID_basic_constraints, CERTIFICATE_BASIC_CONSTRAINTS))
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "Setting the basicConstraints extension "
         "in signed certificate failed");
      ret = -1;
      goto exit;
   }

   if (!set_x509_ext(cert, issuer_cert, NID_subject_key_identifier, CERTIFICATE_SUBJECT_KEY))
   {
      log_ssl_errors(LOG_LEVEL_ERROR,
         "Setting the Subject Key Identifier extension failed");
      ret = -1;
      goto exit;
   }

   if (!set_x509_ext(cert, issuer_cert, NID_authority_key_identifier, CERTIFICATE_AUTHORITY_KEY))
   {
      log_ssl_errors(LOG_LEVEL_ERROR,
         "Setting the Authority Key Identifier extension failed");
      ret = -1;
      goto exit;
   }

   if (!host_is_ip_address(csp->http->host) &&
       !set_subject_alternative_name(cert, issuer_cert, csp->http->host))
   {
      log_ssl_errors(LOG_LEVEL_ERROR,
         "Setting the Subject Alt Name extension failed");
      ret = -1;
      goto exit;
   }

   if (!X509_sign(cert, loaded_issuer_key, EVP_sha256()))
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "Signing certificate failed");
      ret = -1;
      goto exit;
   }

   /*
    * Writing certificate into file
    */
   if (write_certificate(cert, cert_opt.output_file) < 0)
   {
      log_error(LOG_LEVEL_ERROR, "Writing certificate into file failed");
      ret = -1;
      goto exit;
   }

   ret = 1;

exit:
   /*
    * Freeing used structures
    */
   if (issuer_cert)
   {
      X509_free(issuer_cert);
   }
   if (cert)
   {
      X509_free(cert);
   }
   if (pk_bio && !BIO_free(pk_bio))
   {
      log_ssl_errors(LOG_LEVEL_ERROR, "Error closing pk BIO");
   }
   if (loaded_subject_key)
   {
      EVP_PKEY_free(loaded_subject_key);
   }
   if (loaded_issuer_key)
   {
      EVP_PKEY_free(loaded_issuer_key);
   }
   if (subject_name)
   {
      X509_NAME_free(subject_name);
   }
   if (asn_time)
   {
      ASN1_TIME_free(asn_time);
   }
   if (serial_num)
   {
      BN_free(serial_num);
   }
   if (serial)
   {
      ASN1_INTEGER_free(serial);
   }
   freez(cert_opt.subject_key);
   freez(cert_opt.output_file);
   freez(key_buf);

   return ret;
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
   strncpy(buf, X509_verify_cert_error_string(csp->server_cert_verification_result), size);
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
   if (ssl_inited == 1)
   {
#if OPENSSL_VERSION_NUMBER >= 0x1000200fL
#ifndef LIBRESSL_VERSION_NUMBER
#ifndef OPENSSL_NO_COMP
      SSL_COMP_free_compression_methods();
#endif
#endif
#endif
      CONF_modules_free();
      CONF_modules_unload(1);
#ifndef OPENSSL_NO_COMP
      COMP_zlib_cleanup();
#endif

      ERR_free_strings();
      EVP_cleanup();

      CRYPTO_cleanup_all_ex_data();
   }
}
#endif /* def FEATURE_GRACEFUL_TERMINATION */
