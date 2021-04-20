/*********************************************************************
 *
 * File        :  $Source: /cvsroot/ijbswa/current/ssl.c,v $
 *
 * Purpose     :  File with TLS/SSL extension. Contains methods for
 *                creating, using and closing TLS/SSL connections.
 *
 * Copyright   :  Written by and Copyright (c) 2017-2020 Vaclav Svec. FIT CVUT.
 *                Copyright (C) 2018-2020 by Fabian Keil <fk@fabiankeil.de>
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

#if !defined(MBEDTLS_CONFIG_FILE)
#  include "mbedtls/config.h"
#else
#  include MBEDTLS_CONFIG_FILE
#endif

#include "mbedtls/md5.h"
#include "mbedtls/pem.h"
#include "mbedtls/base64.h"
#include "mbedtls/error.h"
#include "mbedtls/oid.h"
#include "mbedtls/asn1write.h"

#include "config.h"
#include "project.h"
#include "miscutil.h"
#include "errlog.h"
#include "jcc.h"
#include "ssl.h"
#include "ssl_common.h"
#include "encode.h"


/*
 * Macros for searching begin and end of certificates.
 * Necessary to convert structure mbedtls_x509_crt to crt file.
 */
#define PEM_BEGIN_CRT     "-----BEGIN CERTIFICATE-----\n"
#define PEM_END_CRT       "-----END CERTIFICATE-----\n"
#define VALID_DATETIME_FMT    "%Y%m%d%H%M%S"
#define VALID_DATETIME_BUFLEN 15

/*
 * Macros for ssl.c
 */
#define CERTIFICATE_BUF_SIZE             16384             /* Size of buffer to save certificate. Value 4096 is mbedtls library buffer size for certificate in DER form */
#define PRIVATE_KEY_BUF_SIZE             16000             /* Size of buffer to save private key. Value 16000 is taken from mbed TLS library examples. */
#define CERT_SIGNATURE_ALGORITHM         MBEDTLS_MD_SHA256 /* The MD algorithm to use for the signature */
#define CERT_PARAM_COMMON_NAME           CERT_PARAM_COMMON_NAME_FCODE"="
#define CERT_PARAM_ORGANIZATION          ","CERT_PARAM_ORGANIZATION_FCODE"="
#define CERT_PARAM_ORG_UNIT              ","CERT_PARAM_ORG_UNIT_FCODE"="
#define CERT_PARAM_COUNTRY               ","CERT_PARAM_COUNTRY_FCODE"="CERT_PARAM_COUNTRY_CODE

/*
 * Properties of key for generating
 */
typedef struct {
   mbedtls_pk_type_t type;   /* type of key to generate  */
   int  rsa_keysize;         /* length of key in bits    */
   char *key_file_path;      /* filename of the key file */
} key_options;

/* Variables for one common RNG for all SSL use */
static mbedtls_ctr_drbg_context ctr_drbg;
static mbedtls_entropy_context  entropy;
static int rng_seeded;

static int generate_host_certificate(struct client_state *csp);
static int host_to_hash(struct client_state *csp);
static int ssl_verify_callback(void *data, mbedtls_x509_crt *crt, int depth, uint32_t *flags);
static void free_client_ssl_structures(struct client_state *csp);
static void free_server_ssl_structures(struct client_state *csp);
static int seed_rng(struct client_state *csp);
static int *get_ciphersuites_from_string(const char *ciphersuites_string);

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
   mbedtls_ssl_context *ssl = &ssl_attr->mbedtls_attr.ssl;
   if (ssl == NULL)
   {
      return 0;
   }

   return mbedtls_ssl_get_bytes_avail(ssl);
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
   mbedtls_ssl_context *ssl = &ssl_attr->mbedtls_attr.ssl;
   int ret = 0;
   size_t max_fragment_size = 0;  /* Maximal length of data in one SSL fragment*/
   int send_len             = 0;  /* length of one data part to send */
   int pos                  = 0;  /* Position of unsent part in buffer */

   if (len == 0)
   {
      return 0;
   }

   /* Getting maximal length of data sent in one fragment */
   max_fragment_size = mbedtls_ssl_get_max_frag_len(ssl);

   /*
    * Whole buffer must be sent in many fragments, because each fragment
    * has its maximal length.
    */
   while (pos < len)
   {
      /* Compute length of data, that can be send in next fragment */
      if ((pos + (int)max_fragment_size) > len)
      {
         send_len = (int)len - pos;
      }
      else
      {
         send_len = (int)max_fragment_size;
      }

      log_error(LOG_LEVEL_WRITING, "TLS on socket %d: %N",
         ssl_attr->mbedtls_attr.socket_fd.fd, send_len, buf+pos);

      /*
       * Sending one part of the buffer
       */
      while ((ret = mbedtls_ssl_write(ssl,
         (const unsigned char *)(buf + pos),
         (size_t)send_len)) < 0)
      {
         if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
             ret != MBEDTLS_ERR_SSL_WANT_WRITE)
         {
            char err_buf[ERROR_BUF_SIZE];

            mbedtls_strerror(ret, err_buf, sizeof(err_buf));
            log_error(LOG_LEVEL_ERROR,
               "Sending data on socket %d over TLS/SSL failed: %s",
               ssl_attr->mbedtls_attr.socket_fd.fd, err_buf);
            return -1;
         }
      }
      /* Adding count of sent bytes to position in buffer */
      pos = pos + send_len;
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
   mbedtls_ssl_context *ssl = &ssl_attr->mbedtls_attr.ssl;
   int ret = 0;
   memset(buf, 0, max_length);

   /*
    * Receiving data from SSL context into buffer
    */
   do
   {
      ret = mbedtls_ssl_read(ssl, buf, max_length);
   } while (ret == MBEDTLS_ERR_SSL_WANT_READ
      || ret == MBEDTLS_ERR_SSL_WANT_WRITE);

   if (ret < 0)
   {
      char err_buf[ERROR_BUF_SIZE];

      if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
      {
         log_error(LOG_LEVEL_CONNECT, "The peer notified us that "
            "the connection on socket %d is going to be closed",
            ssl_attr->mbedtls_attr.socket_fd.fd);
         return 0;
      }
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR,
         "Receiving data on socket %d over TLS/SSL failed: %s",
         ssl_attr->mbedtls_attr.socket_fd.fd, err_buf);

      return -1;
   }

   log_error(LOG_LEVEL_RECEIVED, "TLS from socket %d: %N",
      ssl_attr->mbedtls_attr.socket_fd.fd, ret, buf);

   return ret;
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
   char *ca_file   = NULL;
   char *cert_file = NULL;
   int ret = 0;
   char err_buf[ERROR_BUF_SIZE];

   /*
    * Initializing mbedtls structures for TLS/SSL connection
    */
   mbedtls_net_init(&(ssl_attr->mbedtls_attr.socket_fd));
   mbedtls_ssl_init(&(ssl_attr->mbedtls_attr.ssl));
   mbedtls_ssl_config_init(&(ssl_attr->mbedtls_attr.conf));
   mbedtls_x509_crt_init(&(ssl_attr->mbedtls_attr.server_cert));
   mbedtls_pk_init(&(ssl_attr->mbedtls_attr.prim_key));
#if defined(MBEDTLS_SSL_CACHE_C)
   mbedtls_ssl_cache_init(&(ssl_attr->mbedtls_attr.cache));
#endif

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
   ca_file   = csp->config->ca_cert_file;
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

   /*
    * Seed the RNG
    */
   ret = seed_rng(csp);
   if (ret != 0)
   {
      ret = -1;
      goto exit;
   }

   /*
    * Loading CA file, webpage certificate and key files
    */
   ret = mbedtls_x509_crt_parse_file(&(ssl_attr->mbedtls_attr.server_cert),
      cert_file);
   if (ret != 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR,
         "Loading webpage certificate %s failed: %s", cert_file, err_buf);
      ret = -1;
      goto exit;
   }

   ret = mbedtls_x509_crt_parse_file(&(ssl_attr->mbedtls_attr.server_cert),
      ca_file);
   if (ret != 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR,
         "Loading CA certificate %s failed: %s", ca_file, err_buf);
      ret = -1;
      goto exit;
   }

   ret = mbedtls_pk_parse_keyfile(&(ssl_attr->mbedtls_attr.prim_key),
      key_file, NULL);
   if (ret != 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR,
         "Loading and parsing webpage certificate private key %s failed: %s",
         key_file, err_buf);
      ret = -1;
      goto exit;
   }

   /*
    * Setting SSL parameters
    */
   ret = mbedtls_ssl_config_defaults(&(ssl_attr->mbedtls_attr.conf),
      MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM,
      MBEDTLS_SSL_PRESET_DEFAULT);
   if (ret != 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR,
         "mbedtls_ssl_config_defaults failed: %s", err_buf);
      ret = -1;
      goto exit;
   }

   mbedtls_ssl_conf_rng(&(ssl_attr->mbedtls_attr.conf),
      mbedtls_ctr_drbg_random, &ctr_drbg);

#if defined(MBEDTLS_SSL_CACHE_C)
   mbedtls_ssl_conf_session_cache(&(ssl_attr->mbedtls_attr.conf),
      &(ssl_attr->mbedtls_attr.cache), mbedtls_ssl_cache_get,
      mbedtls_ssl_cache_set);
#endif

   /*
    * Setting certificates
    */
   ret = mbedtls_ssl_conf_own_cert(&(ssl_attr->mbedtls_attr.conf),
      &(ssl_attr->mbedtls_attr.server_cert),
      &(ssl_attr->mbedtls_attr.prim_key));
   if (ret != 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR,
         "mbedtls_ssl_conf_own_cert failed: %s", err_buf);
      ret = -1;
      goto exit;
   }

   if (csp->config->cipher_list != NULL)
   {
      ssl_attr->mbedtls_attr.ciphersuites_list =
         get_ciphersuites_from_string(csp->config->cipher_list);
      if (ssl_attr->mbedtls_attr.ciphersuites_list == NULL)
      {
         log_error(LOG_LEVEL_ERROR,
            "Setting the cipher list '%s' for the client connection failed",
            csp->config->cipher_list);
         ret = -1;
         goto exit;
      }
      mbedtls_ssl_conf_ciphersuites(&(ssl_attr->mbedtls_attr.conf),
         ssl_attr->mbedtls_attr.ciphersuites_list);
   }

   ret = mbedtls_ssl_setup(&(ssl_attr->mbedtls_attr.ssl),
      &(ssl_attr->mbedtls_attr.conf));
   if (ret != 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR, "mbedtls_ssl_setup failed: %s", err_buf);
      ret = -1;
      goto exit;
   }

   mbedtls_ssl_set_bio(&(ssl_attr->mbedtls_attr.ssl),
      &(ssl_attr->mbedtls_attr.socket_fd), mbedtls_net_send,
      mbedtls_net_recv, NULL);
   mbedtls_ssl_session_reset(&(ssl_attr->mbedtls_attr.ssl));

   /*
    * Setting socket fd in mbedtls_net_context structure. This structure
    * can't be set by mbedtls functions, because we already have created
    * a TCP connection when this function is called.
    */
   ssl_attr->mbedtls_attr.socket_fd.fd = csp->cfd;

   /*
    *  Handshake with client
    */
   log_error(LOG_LEVEL_CONNECT,
      "Performing the TLS/SSL handshake with client. Hash of host: %s",
      csp->http->hash_of_host_hex);
   while ((ret = mbedtls_ssl_handshake(&(ssl_attr->mbedtls_attr.ssl))) != 0)
   {
      if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
          ret != MBEDTLS_ERR_SSL_WANT_WRITE)
      {
         mbedtls_strerror(ret, err_buf, sizeof(err_buf));
         log_error(LOG_LEVEL_ERROR,
            "medtls_ssl_handshake with client failed: %s", err_buf);
         ret = -1;
         goto exit;
      }
   }

   log_error(LOG_LEVEL_CONNECT, "Client successfully connected over %s (%s).",
      mbedtls_ssl_get_version(&(ssl_attr->mbedtls_attr.ssl)),
      mbedtls_ssl_get_ciphersuite(&(ssl_attr->mbedtls_attr.ssl)));

   csp->ssl_with_client_is_opened = 1;

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
   int ret = 0;

   if (csp->ssl_with_client_is_opened == 0)
   {
      return;
   }

   /*
    * Notifying the peer that the connection is being closed.
    */
   do {
      ret = mbedtls_ssl_close_notify(&(ssl_attr->mbedtls_attr.ssl));
   } while (ret == MBEDTLS_ERR_SSL_WANT_WRITE);

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
   /*
   * We can't use function mbedtls_net_free, because this function
   * inter alia close TCP connection on set fd. Instead of this
   * function, we change fd to -1, which is the same what does
   * rest of mbedtls_net_free function.
   */
   ssl_attr->mbedtls_attr.socket_fd.fd = -1;

   /* Freeing mbedtls structures */
   mbedtls_x509_crt_free(&(ssl_attr->mbedtls_attr.server_cert));
   mbedtls_pk_free(&(ssl_attr->mbedtls_attr.prim_key));
   mbedtls_ssl_free(&(ssl_attr->mbedtls_attr.ssl));
   freez(ssl_attr->mbedtls_attr.ciphersuites_list);
   mbedtls_ssl_config_free(&(ssl_attr->mbedtls_attr.conf));
#if defined(MBEDTLS_SSL_CACHE_C)
   mbedtls_ssl_cache_free(&(ssl_attr->mbedtls_attr.cache));
#endif
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
   struct ssl_attr *ssl_attr = &csp->ssl_server_attr;
   int ret = 0;
   char err_buf[ERROR_BUF_SIZE];
   char *trusted_cas_file = NULL;
   int auth_mode = MBEDTLS_SSL_VERIFY_REQUIRED;

   csp->server_cert_verification_result = SSL_CERT_NOT_VERIFIED;
   csp->server_certs_chain.next = NULL;

   /* Setting path to file with trusted CAs */
   trusted_cas_file = csp->config->trusted_cas_file;

   /*
    * Initializing mbedtls structures for TLS/SSL connection
    */
   mbedtls_net_init(&(ssl_attr->mbedtls_attr.socket_fd));
   mbedtls_ssl_init(&(ssl_attr->mbedtls_attr.ssl));
   mbedtls_ssl_config_init(&(ssl_attr->mbedtls_attr.conf));
   mbedtls_x509_crt_init(&(ssl_attr->mbedtls_attr.ca_cert));

   /*
   * Setting socket fd in mbedtls_net_context structure. This structure
   * can't be set by mbedtls functions, because we already have created
   * TCP connection when calling this function.
   */
   ssl_attr->mbedtls_attr.socket_fd.fd = csp->server_connection.sfd;

   /*
    * Seed the RNG
    */
   ret = seed_rng(csp);
   if (ret != 0)
   {
      ret = -1;
      goto exit;
   }

   /*
    * Loading file with trusted CAs
    */
   ret = mbedtls_x509_crt_parse_file(&(ssl_attr->mbedtls_attr.ca_cert),
      trusted_cas_file);
   if (ret < 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR, "Loading trusted CAs file %s failed: %s",
         trusted_cas_file, err_buf);
      ret = -1;
      goto exit;
   }

   /*
    * Set TLS/SSL options
    */
   ret = mbedtls_ssl_config_defaults(&(ssl_attr->mbedtls_attr.conf),
      MBEDTLS_SSL_IS_CLIENT,
      MBEDTLS_SSL_TRANSPORT_STREAM,
      MBEDTLS_SSL_PRESET_DEFAULT);
   if (ret != 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR, "mbedtls_ssl_config_defaults failed: %s",
         err_buf);
      ret = -1;
      goto exit;
   }

   /*
    * Setting how strict should certificate verification be and other
    * parameters for certificate verification
    */
   if (csp->dont_verify_certificate)
   {
      auth_mode = MBEDTLS_SSL_VERIFY_NONE;
   }

   mbedtls_ssl_conf_authmode(&(ssl_attr->mbedtls_attr.conf), auth_mode);
   mbedtls_ssl_conf_ca_chain(&(ssl_attr->mbedtls_attr.conf),
      &(ssl_attr->mbedtls_attr.ca_cert), NULL);

   /* Setting callback function for certificates verification */
   mbedtls_ssl_conf_verify(&(ssl_attr->mbedtls_attr.conf),
      ssl_verify_callback, (void *)csp);

   mbedtls_ssl_conf_rng(&(ssl_attr->mbedtls_attr.conf),
      mbedtls_ctr_drbg_random, &ctr_drbg);

   if (csp->config->cipher_list != NULL)
   {
      ssl_attr->mbedtls_attr.ciphersuites_list =
         get_ciphersuites_from_string(csp->config->cipher_list);
      if (ssl_attr->mbedtls_attr.ciphersuites_list == NULL)
      {
         log_error(LOG_LEVEL_ERROR,
            "Setting the cipher list '%s' for the server connection failed",
            csp->config->cipher_list);
         ret = -1;
         goto exit;
      }
      mbedtls_ssl_conf_ciphersuites(&(ssl_attr->mbedtls_attr.conf),
         ssl_attr->mbedtls_attr.ciphersuites_list);
   }

   ret = mbedtls_ssl_setup(&(ssl_attr->mbedtls_attr.ssl),
      &(ssl_attr->mbedtls_attr.conf));
   if (ret != 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR, "mbedtls_ssl_setup failed: %s", err_buf);
      ret = -1;
      goto exit;
   }

   /*
    * Set the hostname to check against the received server certificate
    */
   ret = mbedtls_ssl_set_hostname(&(ssl_attr->mbedtls_attr.ssl),
      csp->http->host);
   if (ret != 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR, "mbedtls_ssl_set_hostname failed: %s",
         err_buf);
      ret = -1;
      goto exit;
   }

   mbedtls_ssl_set_bio(&(ssl_attr->mbedtls_attr.ssl),
      &(ssl_attr->mbedtls_attr.socket_fd), mbedtls_net_send,
      mbedtls_net_recv, NULL);

   /*
    * Handshake with server
    */
   log_error(LOG_LEVEL_CONNECT,
      "Performing the TLS/SSL handshake with the server");

   while ((ret = mbedtls_ssl_handshake(&(ssl_attr->mbedtls_attr.ssl))) != 0)
   {
      if (ret != MBEDTLS_ERR_SSL_WANT_READ
       && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
      {
         mbedtls_strerror(ret, err_buf, sizeof(err_buf));

         if (ret == MBEDTLS_ERR_X509_CERT_VERIFY_FAILED)
         {
            char reason[INVALID_CERT_INFO_BUF_SIZE];

            csp->server_cert_verification_result =
               mbedtls_ssl_get_verify_result(&(ssl_attr->mbedtls_attr.ssl));
            mbedtls_x509_crt_verify_info(reason, sizeof(reason), "",
               csp->server_cert_verification_result);

            /* Log the reason without the trailing new line */
            log_error(LOG_LEVEL_ERROR,
               "X509 certificate verification for %s failed: %N",
               csp->http->hostport, strlen(reason)-1, reason);
            ret = -1;
         }
         else
         {
            log_error(LOG_LEVEL_ERROR,
               "mbedtls_ssl_handshake with server failed: %s", err_buf);
            free_certificate_chain(csp);
            ret = -1;
         }
         goto exit;
      }
   }

   log_error(LOG_LEVEL_CONNECT, "Server successfully connected over %s (%s).",
      mbedtls_ssl_get_version(&(ssl_attr->mbedtls_attr.ssl)),
      mbedtls_ssl_get_ciphersuite(&(ssl_attr->mbedtls_attr.ssl)));

   /*
    * Server certificate chain is valid, so we can clean
    * chain, because we will not send it to client.
    */
   free_certificate_chain(csp);

   csp->ssl_with_server_is_opened = 1;
   csp->server_cert_verification_result =
      mbedtls_ssl_get_verify_result(&(ssl_attr->mbedtls_attr.ssl));

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
   int ret = 0;

   if (csp->ssl_with_server_is_opened == 0)
   {
      return;
   }

   /*
   * Notifying the peer that the connection is being closed.
   */
   do {
      ret = mbedtls_ssl_close_notify(&(ssl_attr->mbedtls_attr.ssl));
   } while (ret == MBEDTLS_ERR_SSL_WANT_WRITE);

   free_server_ssl_structures(csp);
   csp->ssl_with_server_is_opened = 0;
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
   /*
   * We can't use function mbedtls_net_free, because this function
   * inter alia close TCP connection on set fd. Instead of this
   * function, we change fd to -1, which is the same what does
   * rest of mbedtls_net_free function.
   */
   ssl_attr->mbedtls_attr.socket_fd.fd = -1;

   mbedtls_x509_crt_free(&(ssl_attr->mbedtls_attr.ca_cert));
   mbedtls_ssl_free(&(ssl_attr->mbedtls_attr.ssl));
   freez(ssl_attr->mbedtls_attr.ciphersuites_list);
   mbedtls_ssl_config_free(&(ssl_attr->mbedtls_attr.conf));
}


/*====================== Certificates ======================*/

/*********************************************************************
 *
 * Function    :  write_certificate
 *
 * Description :  Writes certificate into file.
 *
 * Parameters  :
 *          1  :  crt = certificate to write into file
 *          2  :  output_file = path to save certificate file
 *          3  :  f_rng = mbedtls_ctr_drbg_random
 *          4  :  p_rng = mbedtls_ctr_drbg_context
 *
 * Returns     :  Length of written certificate on success or negative value
 *                on error
 *
 *********************************************************************/
static int write_certificate(mbedtls_x509write_cert *crt, const char *output_file,
   int(*f_rng)(void *, unsigned char *, size_t), void *p_rng)
{
   FILE *f = NULL;
   size_t len = 0;
   unsigned char cert_buf[CERTIFICATE_BUF_SIZE + 1]; /* Buffer for certificate in PEM format + terminating NULL */
   int ret = 0;
   char err_buf[ERROR_BUF_SIZE];

   memset(cert_buf, 0, sizeof(cert_buf));

   /*
    * Writing certificate into PEM string. If buffer is too small, function
    * returns specific error and no buffer overflow can happen.
    */
   if ((ret = mbedtls_x509write_crt_pem(crt, cert_buf,
      sizeof(cert_buf) - 1, f_rng, p_rng)) != 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR,
         "Writing certificate into buffer failed: %s", err_buf);
      return -1;
   }

   len = strlen((char *)cert_buf);

   /*
    * Saving certificate into file
    */
   if ((f = fopen(output_file, "w")) == NULL)
   {
      log_error(LOG_LEVEL_ERROR, "Opening file %s to save certificate failed",
         output_file);
      return -1;
   }

   if (fwrite(cert_buf, 1, len, f) != len)
   {
      log_error(LOG_LEVEL_ERROR,
         "Writing certificate into file %s failed", output_file);
      fclose(f);
      return -1;
   }

   fclose(f);

   return (int)len;
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
static int write_private_key(mbedtls_pk_context *key, unsigned char **ret_buf,
   const char *key_file_path)
{
   size_t len = 0;                /* Length of created key    */
   FILE *f = NULL;                /* File to save certificate */
   int ret = 0;
   char err_buf[ERROR_BUF_SIZE];

   /* Initializing buffer for key file content */
   *ret_buf = zalloc_or_die(PRIVATE_KEY_BUF_SIZE + 1);

   /*
    * Writing private key into PEM string
    */
   if ((ret = mbedtls_pk_write_key_pem(key, *ret_buf, PRIVATE_KEY_BUF_SIZE)) != 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR,
         "Writing private key into PEM string failed: %s", err_buf);
      ret = -1;
      goto exit;
   }
   len = strlen((char *)*ret_buf);

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
      fclose(f);
      log_error(LOG_LEVEL_ERROR,
         "Writing private key into file %s failed",
         key_file_path);
      ret = -1;
      goto exit;
   }

   fclose(f);

exit:
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
static int generate_key(struct client_state *csp, unsigned char **key_buf)
{
   mbedtls_pk_context key;
   key_options key_opt;
   int ret = 0;
   char err_buf[ERROR_BUF_SIZE];

   key_opt.key_file_path = NULL;

   /*
    * Initializing structures for key generating
    */
   mbedtls_pk_init(&key);

   /*
    * Preparing path for key file and other properties for generating key
    */
   key_opt.type        = MBEDTLS_PK_RSA;
   key_opt.rsa_keysize = RSA_KEYSIZE;

   key_opt.key_file_path = make_certs_path(csp->config->certificate_directory,
      (char *)csp->http->hash_of_host_hex, KEY_FILE_TYPE);
   if (key_opt.key_file_path == NULL)
   {
      ret = -1;
      goto exit;
   }

   /*
    * Test if key already exists. If so, we don't have to create it again.
    */
   if (file_exists(key_opt.key_file_path) == 1)
   {
      ret = 0;
      goto exit;
   }

   /*
    * Seed the RNG
    */
   ret = seed_rng(csp);
   if (ret != 0)
   {
      ret = -1;
      goto exit;
   }

   /*
    * Setting attributes of private key and generating it
    */
   if ((ret = mbedtls_pk_setup(&key,
      mbedtls_pk_info_from_type(key_opt.type))) != 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR, "mbedtls_pk_setup failed: %s", err_buf);
      ret = -1;
      goto exit;
   }

   ret = mbedtls_rsa_gen_key(mbedtls_pk_rsa(key), mbedtls_ctr_drbg_random,
      &ctr_drbg, (unsigned)key_opt.rsa_keysize, RSA_KEY_PUBLIC_EXPONENT);
   if (ret != 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR, "Key generating failed: %s", err_buf);
      ret = -1;
      goto exit;
   }

   /*
    * Exporting private key into file
    */
   if ((ret = write_private_key(&key, key_buf, key_opt.key_file_path)) < 0)
   {
      log_error(LOG_LEVEL_ERROR,
         "Writing private key into file %s failed", key_opt.key_file_path);
      ret = -1;
      goto exit;
   }

exit:
   /*
    * Freeing used variables
    */
   freez(key_opt.key_file_path);

   mbedtls_pk_free(&key);

   return ret;
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
   mbedtls_x509_crt cert;
   int ret;

   mbedtls_x509_crt_init(&cert);

   ret = mbedtls_x509_crt_parse_file(&cert, cert_file);
   if (ret != 0)
   {
      char err_buf[ERROR_BUF_SIZE];

      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR,
         "Loading certificate %s to check validity failed: %s",
         cert_file, err_buf);
      mbedtls_x509_crt_free(&cert);

      return 1;
   }
   if (mbedtls_x509_time_is_past(&cert.valid_to))
   {
      mbedtls_x509_crt_free(&cert);

      return 1;
   }

   mbedtls_x509_crt_free(&cert);

   return 0;

}


/*********************************************************************
 *
 * Function    :  set_subject_alternative_name
 *
 * Description :  Sets the Subject Alternative Name extension to a cert
 *
 * Parameters  :
 *          1  :  cert = The certificate to modify
 *          2  :  hostname = The hostname to add
 *
 * Returns     :  <0 => Error while creating certificate.
 *                 0 => It worked
 *
 *********************************************************************/
static int set_subject_alternative_name(mbedtls_x509write_cert *cert, const char *hostname)
{
   char err_buf[ERROR_BUF_SIZE];
   int ret;
   char *subject_alternative_name;
   size_t subject_alternative_name_len;
#define MBEDTLS_SUBJECT_ALTERNATIVE_NAME_MAX_LEN 255
   unsigned char san_buf[MBEDTLS_SUBJECT_ALTERNATIVE_NAME_MAX_LEN + 1];
   unsigned char *c;
   int len;

   subject_alternative_name_len = strlen(hostname) + 1;
   subject_alternative_name = zalloc_or_die(subject_alternative_name_len);

   strlcpy(subject_alternative_name, hostname, subject_alternative_name_len);

   memset(san_buf, 0, sizeof(san_buf));

   c = san_buf + sizeof(san_buf);
   len = 0;

   ret = mbedtls_asn1_write_raw_buffer(&c, san_buf,
      (const unsigned char *)subject_alternative_name,
      strlen(subject_alternative_name));
   if (ret < 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR,
         "mbedtls_asn1_write_raw_buffer() failed: %s", err_buf);
      goto exit;
   }
   len += ret;

   ret = mbedtls_asn1_write_len(&c, san_buf, strlen(subject_alternative_name));
   if (ret < 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR,
         "mbedtls_asn1_write_len() failed: %s", err_buf);
      goto exit;
   }
   len += ret;

   ret = mbedtls_asn1_write_tag(&c, san_buf, MBEDTLS_ASN1_CONTEXT_SPECIFIC | 2);
   if (ret < 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR,
         "mbedtls_asn1_write_tag() failed: %s", err_buf);
      goto exit;
   }
   len += ret;

   ret = mbedtls_asn1_write_len(&c, san_buf, (size_t)len);
   if (ret < 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR,
         "mbedtls_asn1_write_len() failed: %s", err_buf);
      goto exit;
   }
   len += ret;

   ret = mbedtls_asn1_write_tag(&c, san_buf,
      MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
   if (ret < 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR,
         "mbedtls_asn1_write_tag() failed: %s", err_buf);
      goto exit;
   }
   len += ret;

   ret = mbedtls_x509write_crt_set_extension(cert,
      MBEDTLS_OID_SUBJECT_ALT_NAME,
      MBEDTLS_OID_SIZE(MBEDTLS_OID_SUBJECT_ALT_NAME),
      0, san_buf + sizeof(san_buf) - len, (size_t)len);
   if (ret < 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR,
         "mbedtls_x509write_crt_set_extension() failed: %s", err_buf);
   }

exit:
   freez(subject_alternative_name);

   return ret;

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
 *                >0 => Length of created certificate.
 *
 *********************************************************************/
static int generate_host_certificate(struct client_state *csp)
{
   mbedtls_x509_crt issuer_cert;
   mbedtls_pk_context loaded_issuer_key, loaded_subject_key;
   mbedtls_pk_context *issuer_key  = &loaded_issuer_key;
   mbedtls_pk_context *subject_key = &loaded_subject_key;
   mbedtls_x509write_cert cert;
   mbedtls_mpi serial;

   unsigned char *key_buf = NULL;    /* Buffer for created key */

   int ret = 0;
   char err_buf[ERROR_BUF_SIZE];
   cert_options cert_opt;
   char cert_valid_from[VALID_DATETIME_BUFLEN];
   char cert_valid_to[VALID_DATETIME_BUFLEN];

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
    * Initializing structures for certificate generating
    */
   mbedtls_x509write_crt_init(&cert);
   mbedtls_x509write_crt_set_md_alg(&cert, CERT_SIGNATURE_ALGORITHM);
   mbedtls_pk_init(&loaded_issuer_key);
   mbedtls_pk_init(&loaded_subject_key);
   mbedtls_mpi_init(&serial);
   mbedtls_x509_crt_init(&issuer_cert);

   /*
    * Presetting parameters for certificate. We must compute total length
    * of parameters.
    */
   size_t cert_params_len = strlen(CERT_PARAM_COMMON_NAME) +
      strlen(CERT_PARAM_ORGANIZATION) + strlen(CERT_PARAM_COUNTRY) +
      strlen(CERT_PARAM_ORG_UNIT) +
      3 * strlen(csp->http->host) + 1;
   char cert_params[cert_params_len];
   memset(cert_params, 0, cert_params_len);

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
   strlcpy(cert_params, CERT_PARAM_COMMON_NAME,  cert_params_len);
   strlcat(cert_params, csp->http->host,         cert_params_len);
   strlcat(cert_params, CERT_PARAM_ORGANIZATION, cert_params_len);
   strlcat(cert_params, csp->http->host,         cert_params_len);
   strlcat(cert_params, CERT_PARAM_ORG_UNIT,     cert_params_len);
   strlcat(cert_params, csp->http->host,         cert_params_len);
   strlcat(cert_params, CERT_PARAM_COUNTRY,      cert_params_len);

   cert_opt.issuer_crt = csp->config->ca_cert_file;
   cert_opt.issuer_key = csp->config->ca_key_file;

   if (get_certificate_valid_from_date(cert_valid_from, sizeof(cert_valid_from), VALID_DATETIME_FMT)
    || get_certificate_valid_to_date(cert_valid_to, sizeof(cert_valid_to), VALID_DATETIME_FMT))
   {
      log_error(LOG_LEVEL_ERROR, "Generating one of the validity dates failed");
      ret = -1;
      goto exit;
   }

   cert_opt.subject_pwd   = CERT_SUBJECT_PASSWORD;
   cert_opt.issuer_pwd    = csp->config->ca_password;
   cert_opt.subject_name  = cert_params;
   cert_opt.not_before    = cert_valid_from;
   cert_opt.not_after     = cert_valid_to;
   cert_opt.serial        = serial_num_text;
   cert_opt.is_ca         = 0;
   cert_opt.max_pathlen   = -1;

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
    * Seed the PRNG
    */
   ret = seed_rng(csp);
   if (ret != 0)
   {
      ret = -1;
      goto exit;
   }

   /*
    * Parse serial to MPI
    */
   ret = mbedtls_mpi_read_string(&serial, 10, cert_opt.serial);
   if (ret != 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR,
         "mbedtls_mpi_read_string failed: %s", err_buf);
      ret = -1;
      goto exit;
   }

   /*
    * Loading certificates
    */
   ret = mbedtls_x509_crt_parse_file(&issuer_cert, cert_opt.issuer_crt);
   if (ret != 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR, "Loading issuer certificate %s failed: %s",
         cert_opt.issuer_crt, err_buf);
      ret = -1;
      goto exit;
   }

   ret = mbedtls_x509_dn_gets(cert_opt.issuer_name,
      sizeof(cert_opt.issuer_name), &issuer_cert.subject);
   if (ret < 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR, "mbedtls_x509_dn_gets failed: %s", err_buf);
      ret = -1;
      goto exit;
   }

   /*
    * Loading keys from file or from buffer
    */
   if (key_buf != NULL && subject_key_len > 0)
   {
      /* Key was created in this function and is stored in buffer */
      ret = mbedtls_pk_parse_key(&loaded_subject_key, key_buf,
         (size_t)(subject_key_len + 1), (unsigned const char *)
         cert_opt.subject_pwd, strlen(cert_opt.subject_pwd));
   }
   else
   {
      /* Key wasn't created in this function, because it already existed */
      ret = mbedtls_pk_parse_keyfile(&loaded_subject_key,
         cert_opt.subject_key, cert_opt.subject_pwd);
   }

   if (ret != 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR, "Parsing subject key %s failed: %s",
         cert_opt.subject_key, err_buf);
      ret = -1;
      goto exit;
   }

   ret = mbedtls_pk_parse_keyfile(&loaded_issuer_key, cert_opt.issuer_key,
      cert_opt.issuer_pwd);
   if (ret != 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR,
         "Parsing issuer key %s failed: %s", cert_opt.issuer_key, err_buf);
      ret = -1;
      goto exit;
   }

   /*
    * Check if key and issuer certificate match
    */
   if (!mbedtls_pk_can_do(&issuer_cert.pk, MBEDTLS_PK_RSA) ||
      mbedtls_mpi_cmp_mpi(&mbedtls_pk_rsa(issuer_cert.pk)->N,
         &mbedtls_pk_rsa(*issuer_key)->N) != 0 ||
      mbedtls_mpi_cmp_mpi(&mbedtls_pk_rsa(issuer_cert.pk)->E,
         &mbedtls_pk_rsa(*issuer_key)->E) != 0)
   {
      log_error(LOG_LEVEL_ERROR,
         "Issuer key doesn't match issuer certificate");
      ret = -1;
      goto exit;
   }

   mbedtls_x509write_crt_set_subject_key(&cert, subject_key);
   mbedtls_x509write_crt_set_issuer_key(&cert, issuer_key);

   /*
    * Setting parameters of signed certificate
    */
   ret = mbedtls_x509write_crt_set_subject_name(&cert, cert_opt.subject_name);
   if (ret != 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR,
         "Setting subject name in signed certificate failed: %s", err_buf);
      ret = -1;
      goto exit;
   }

   ret = mbedtls_x509write_crt_set_issuer_name(&cert, cert_opt.issuer_name);
   if (ret != 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR,
         "Setting issuer name in signed certificate failed: %s", err_buf);
      ret = -1;
      goto exit;
   }

   ret = mbedtls_x509write_crt_set_serial(&cert, &serial);
   if (ret != 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR,
         "Setting serial number in signed certificate failed: %s", err_buf);
      ret = -1;
      goto exit;
   }

   ret = mbedtls_x509write_crt_set_validity(&cert, cert_opt.not_before,
      cert_opt.not_after);
   if (ret != 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR,
         "Setting validity in signed certificate failed: %s", err_buf);
      ret = -1;
      goto exit;
   }

   /*
    * Setting the basicConstraints extension for certificate
    */
   ret = mbedtls_x509write_crt_set_basic_constraints(&cert, cert_opt.is_ca,
      cert_opt.max_pathlen);
   if (ret != 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR, "Setting the basicConstraints extension "
         "in signed certificate failed: %s", err_buf);
      ret = -1;
      goto exit;
   }

#if defined(MBEDTLS_SHA1_C)
   /* Setting the subjectKeyIdentifier extension for certificate */
   ret = mbedtls_x509write_crt_set_subject_key_identifier(&cert);
   if (ret != 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR, "mbedtls_x509write_crt_set_subject_key_"
         "identifier failed: %s", err_buf);
      ret = -1;
      goto exit;
   }

   /* Setting the authorityKeyIdentifier extension for certificate */
   ret = mbedtls_x509write_crt_set_authority_key_identifier(&cert);
   if (ret != 0)
   {
      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR, "mbedtls_x509write_crt_set_authority_key_"
         "identifier failed: %s", err_buf);
      ret = -1;
      goto exit;
   }
#endif /* MBEDTLS_SHA1_C */

   if (!host_is_ip_address(csp->http->host) &&
      set_subject_alternative_name(&cert, csp->http->host))
   {
      /* Errors are already logged by set_subject_alternative_name() */
      ret = -1;
      goto exit;
   }

   /*
    * Writing certificate into file
    */
   ret = write_certificate(&cert, cert_opt.output_file,
      mbedtls_ctr_drbg_random, &ctr_drbg);
   if (ret < 0)
   {
      log_error(LOG_LEVEL_ERROR, "Writing certificate into file failed");
      goto exit;
   }

exit:
   /*
    * Freeing used structures
    */
   mbedtls_x509write_crt_free(&cert);
   mbedtls_pk_free(&loaded_subject_key);
   mbedtls_pk_free(&loaded_issuer_key);
   mbedtls_mpi_free(&serial);
   mbedtls_x509_crt_free(&issuer_cert);

   freez(cert_opt.subject_key);
   freez(cert_opt.output_file);
   freez(key_buf);

   return ret;
}

/*********************************************************************
 *
 * Function    :  ssl_verify_callback
 *
 * Description :  This is a callback function for certificate verification.
 *                It's called once for each certificate in the server's
 *                certificate trusted chain and prepares information about
 *                the certificate. The information can be used to inform
 *                the user about invalid certificates.
 *
 * Parameters  :
 *          1  :  csp_void = Current client state (buffers, headers, etc...)
 *          2  :  crt   = certificate from trusted chain
 *          3  :  depth = depth in trusted chain
 *          4  :  flags = certificate flags
 *
 * Returns     :  0 on success and negative value on error
 *
 *********************************************************************/
static int ssl_verify_callback(void *csp_void, mbedtls_x509_crt *crt,
   int depth, uint32_t *flags)
{
   struct client_state *csp  = (struct client_state *)csp_void;
   struct certs_chain  *last = &(csp->server_certs_chain);
   size_t olen = 0;
   int ret = 0;

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
   memset(last->next->file_buf, 0, sizeof(last->next->file_buf));

   /*
    * Saving certificate file into buffer
    */
   if ((ret = mbedtls_pem_write_buffer(PEM_BEGIN_CRT, PEM_END_CRT,
      crt->raw.p, crt->raw.len, (unsigned char *)last->file_buf,
      sizeof(last->file_buf)-1, &olen)) != 0)
   {
      char err_buf[ERROR_BUF_SIZE];

      mbedtls_strerror(ret, err_buf, sizeof(err_buf));
      log_error(LOG_LEVEL_ERROR, "mbedtls_pem_write_buffer() failed: %s",
         err_buf);

      return(ret);
   }

   /*
    * Saving certificate information into buffer
    */
   {
      char buf[CERT_INFO_BUF_SIZE];
      char *encoded_text;
#define CERT_INFO_PREFIX                 ""

      mbedtls_x509_crt_info(buf, sizeof(buf), CERT_INFO_PREFIX, crt);
      encoded_text = html_encode(buf);
      if (encoded_text == NULL)
      {
         log_error(LOG_LEVEL_ERROR,
            "Failed to HTML-encode the certificate information");
         return -1;
      }
      strlcpy(last->info_buf, encoded_text, sizeof(last->info_buf));
      freez(encoded_text);
   }

   return 0;
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

#if !defined(MBEDTLS_MD5_C)
#error mbedTLS needs to be compiled with md5 support
#else
   memset(csp->http->hash_of_host, 0, sizeof(csp->http->hash_of_host));
   ret = mbedtls_md5_ret((unsigned char *)csp->http->host,
      strlen(csp->http->host), csp->http->hash_of_host);
   if (ret != 0)
   {
      log_error(LOG_LEVEL_ERROR,
         "Failed to generate md5 hash of host %s: %d",
         csp->http->host, ret);
      return -1;
   }

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
#endif /* MBEDTLS_MD5_C */
}

/*********************************************************************
 *
 * Function    :  seed_rng
 *
 * Description :  Seeding the RNG for all SSL uses
 *
 * Parameters  :
 *          1  :  csp = Current client state (buffers, headers, etc...)
 *
 * Returns     : -1 => RNG wasn't seed successfully
 *                0 => RNG is seeded successfully
 *
 *********************************************************************/
static int seed_rng(struct client_state *csp)
{
   int ret = 0;
   char err_buf[ERROR_BUF_SIZE];

   if (rng_seeded == 0)
   {
      privoxy_mutex_lock(&ssl_init_mutex);
      if (rng_seeded == 0)
      {
         mbedtls_ctr_drbg_init(&ctr_drbg);
         mbedtls_entropy_init(&entropy);
         ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func,
            &entropy, NULL, 0);
         if (ret != 0)
         {
            mbedtls_strerror(ret, err_buf, sizeof(err_buf));
            log_error(LOG_LEVEL_ERROR,
               "mbedtls_ctr_drbg_seed failed: %s", err_buf);
            privoxy_mutex_unlock(&ssl_init_mutex);
            return -1;
         }
         rng_seeded = 1;
      }
      privoxy_mutex_unlock(&ssl_init_mutex);
   }
   return 0;
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
   return mbedtls_base64_encode(dst, dlen, olen, src, slen);
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
   char *last_byte;

   mbedtls_x509_crt_verify_info(buf, size, "",
      csp->server_cert_verification_result);
   last_byte = buf + strlen(buf)-1;
   if (*last_byte == '\n')
   {
      /* Overwrite trailing new line character */
      *last_byte = '\0';
   }
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
   if (rng_seeded == 1)
   {
      mbedtls_ctr_drbg_free(&ctr_drbg);
      mbedtls_entropy_free(&entropy);
   }
}
#endif /* def FEATURE_GRACEFUL_TERMINATION */


/*********************************************************************
 *
 * Function    :  get_ciphersuites_from_string
 *
 * Description :  Converts a string of ciphersuite names to
 *                an array of ciphersuite ids.
 *
 * Parameters  :
 *          1  :  ciphersuites_string = String containing allowed
 *                ciphersuites.
 *
 * Returns     :  Array of ciphersuite ids
 *
 *********************************************************************/
static int *get_ciphersuites_from_string(const char *parameter_string)
{
   char *ciphersuites_index;
   char *item_end;
   char *ciphersuites_string;
   int *ciphersuite_ids;
   size_t count = 2;
   int index = 0;
   const char separator = ':';
   size_t parameter_len = strlen(parameter_string);

   ciphersuites_string = zalloc_or_die(parameter_len + 1);
   strncpy(ciphersuites_string, parameter_string, parameter_len);
   ciphersuites_index = ciphersuites_string;

   while (*ciphersuites_index)
   {
      if (*ciphersuites_index++ == separator)
      {
         ++count;
      }
   }

   ciphersuite_ids = zalloc_or_die(count * sizeof(int));

   ciphersuites_index = ciphersuites_string;
   do
   {
      item_end = strchr(ciphersuites_index, separator);
      if (item_end != NULL)
      {
         *item_end = '\0';
      }

      ciphersuite_ids[index] =
         mbedtls_ssl_get_ciphersuite_id(ciphersuites_index);
      if (ciphersuite_ids[index] == 0)
      {
         log_error(LOG_LEVEL_ERROR,
            "Failed to get ciphersuite id for %s", ciphersuites_index);
         freez(ciphersuite_ids);
         freez(ciphersuites_string);
         return NULL;
      }
      ciphersuites_index = item_end + 1;
      index++;
   } while (item_end != NULL);

   ciphersuite_ids[index] = 0;
   freez(ciphersuites_string);

   return ciphersuite_ids;

}
