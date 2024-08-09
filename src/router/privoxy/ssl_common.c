/*********************************************************************
 *
 * File        :  $Source: /cvsroot/ijbswa/current/ssl_common.c,v $
 *
 * Purpose     :  File with TLS/SSL extension. Contains methods for
 *                creating, using and closing TLS/SSL connections that do
 *                not depend on particular TLS/SSL library.
 *
 * Copyright   :  Written by and Copyright (c) 2017 Vaclav Svec. FIT CVUT.
 *                Copyright (C) 2018-2021 by Fabian Keil <fk@fabiankeil.de>
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

#include <ctype.h>
#include <unistd.h>
#include "config.h"
#include "project.h"
#include "miscutil.h"
#include "errlog.h"
#include "ssl.h"
#include "ssl_common.h"

/*
 * Macros for ssl_common.c
 */
#define CERT_SERIAL_NUM_LENGTH           4                 /* Bytes of hash to be used for creating serial number of certificate. Min=2 and max=16 */

/*********************************************************************
 *
 * Function    :  client_use_ssl
 *
 * Description :  Tests if client in current client state structure
 *                should use SSL connection or standard connection.
 *
 * Parameters  :
 *          1  :  csp = Current client state (buffers, headers, etc...)
 *
 * Returns     :  If client should use TLS/SSL connection, 1 is returned.
 *                Otherwise 0 is returned.
 *
 *********************************************************************/
extern int client_use_ssl(const struct client_state *csp)
{
   return csp->http->client_ssl;
}


/*********************************************************************
 *
 * Function    :  server_use_ssl
 *
 * Description :  Tests if server in current client state structure
 *                should use SSL connection or standard connection.
 *
 * Parameters  :
 *          1  :  csp = Current client state (buffers, headers, etc...)
 *
 * Returns     :  If server should use TLS/SSL connection, 1 is returned.
 *                Otherwise 0 is returned.
 *
 *********************************************************************/
extern int server_use_ssl(const struct client_state *csp)
{
   return csp->http->server_ssl;
}


/*********************************************************************
 *
 * Function    :  ssl_send_data_delayed
 *
 * Description :  Sends the contents of buf (for n bytes) to given SSL
 *                connection, optionally delaying the operation.
 *
 * Parameters  :
 *          1  :  ssl_attr = SSL context to send data to
 *          2  :  buf = Pointer to data to be sent
 *          3  :  len = Length of data to be sent to the SSL context
 *          4  :  delay = Delay in milliseconds.
 *
 * Returns     :  0 on success (entire buffer sent).
 *                nonzero on error.
 *
 *********************************************************************/
extern int ssl_send_data_delayed(struct ssl_attr* ssl_attr,
                                 const unsigned char *buf, size_t len,
                                 unsigned int delay)
{
   size_t i = 0;

   if (delay == 0)
   {
      if (ssl_send_data(ssl_attr, buf, len) < 0)
      {
         return -1;
      }
      else
      {
         return 0;
      }
   }

   while (i < len)
   {
      size_t write_length;
      enum { MAX_WRITE_LENGTH = 10 };

      if ((i + MAX_WRITE_LENGTH) > len)
      {
         write_length = len - i;
      }
      else
      {
         write_length = MAX_WRITE_LENGTH;
      }

      privoxy_millisleep(delay);

      if (ssl_send_data(ssl_attr, buf + i, write_length) < 0)
      {
         return -1;
      }
      i += write_length;
   }

   return 0;

}


/*********************************************************************
 *
 * Function    :  ssl_flush_socket
 *
 * Description :  Send any pending "buffered" content with given
 *                SSL connection. Alternative to function flush_socket.
 *
 * Parameters  :
 *          1  :  ssl_attr = SSL context to send buffer to
 *          2  :  iob = The I/O buffer to flush, usually csp->iob.
 *
 * Returns     :  On success, the number of bytes send are returned (zero
 *                indicates nothing was sent).  On error, -1 is returned.
 *
 *********************************************************************/
extern long ssl_flush_socket(struct ssl_attr *ssl_attr, struct iob *iob)
{
   /* Computing length of buffer part to send */
   long len = iob->eod - iob->cur;

   if (len <= 0)
   {
      return(0);
   }

   /* Sending data to given SSl context */
   if (ssl_send_data(ssl_attr, (const unsigned char *)iob->cur, (size_t)len) < 0)
   {
      return -1;
   }
   iob->eod = iob->cur = iob->buf;
   return(len);
}


/*********************************************************************
 *
 * Function    :  close_client_and_server_ssl_connections
 *
 * Description :  Checks if client or server should use secured
 *                connection over SSL and if so, closes all of them.
 *
 * Parameters  :
 *          1  :  csp = Current client state (buffers, headers, etc...)
 *
 * Returns     :  N/A
 *
 *********************************************************************/
extern void close_client_and_server_ssl_connections(struct client_state *csp)
{
   if (client_use_ssl(csp) == 1)
   {
      close_client_ssl_connection(csp);
   }
   if (server_use_ssl(csp) == 1)
   {
      close_server_ssl_connection(csp);
   }
}


/*********************************************************************
 *
 * Function    :  tunnel_established_successfully
 *
 * Description :  Check if parent proxy server response contains
 *                information about successfully created connection with
 *                destination server. (HTTP/... 2xx ...)
 *
 * Parameters  :
 *          1  :  server_response = Buffer with parent proxy server response
 *          2  :  response_len = Length of server_response
 *
 * Returns     :  1 => Connection created successfully
 *                0 => Connection wasn't created successfully
 *
 *********************************************************************/
extern int tunnel_established_successfully(const char *server_response,
   unsigned int response_len)
{
   unsigned int pos = 0;

   if (server_response == NULL)
   {
      return 0;
   }

   /* Tests if "HTTP/" string is at the begin of received response */
   if (strncmp(server_response, "HTTP/", 5) != 0)
   {
      return 0;
   }

   for (pos = 0; pos < response_len; pos++)
   {
      if (server_response[pos] == ' ')
      {
         break;
      }
   }

   /*
    * response_len -3 because of buffer end, response structure and 200 code.
    * There must be at least 3 chars after space.
    * End of buffer: ... 2xx'\0'
    *             pos = |
    */
   if (pos >= (response_len - 3))
   {
      return 0;
   }

   /* Test HTTP status code */
   if (server_response[pos + 1] != '2')
   {
      return 0;
   }

   return 1;
}


/*********************************************************************
 *
 * Function    :  free_certificate_chain
 *
 * Description :  Frees certificates linked list. This linked list is
 *                used to save information about certificates in
 *                trusted chain.
 *
 * Parameters  :
 *          1  :  csp = Current client state (buffers, headers, etc...)
 *
 * Returns     :  N/A
 *
 *********************************************************************/
extern void free_certificate_chain(struct client_state *csp)
{
   struct certs_chain *cert = csp->server_certs_chain.next;

   /* Cleaning buffers */
   memset(csp->server_certs_chain.info_buf, 0,
      sizeof(csp->server_certs_chain.info_buf));
   freez(csp->server_certs_chain.file_buf);

   csp->server_certs_chain.next = NULL;

   /* Freeing memory in whole linked list */
   while (cert != NULL)
   {
      struct certs_chain *cert_for_free = cert;
      cert = cert->next;

      /* Cleaning buffers */
      memset(cert_for_free->info_buf, 0, sizeof(cert_for_free->info_buf));
      freez(cert_for_free->file_buf);

      freez(cert_for_free);
   }
}


/*********************************************************************
 *
 * Function    :  ssl_send_certificate_error
 *
 * Description :  Sends info about invalid server certificate to client.
 *                Sent message is including all trusted chain certificates,
 *                that can be downloaded in web browser.
 *
 * Parameters  :
 *          1  :  csp = Current client state (buffers, headers, etc...)
 *
 * Returns     :  N/A
 *
 *********************************************************************/
extern void ssl_send_certificate_error(struct client_state *csp)
{
   struct ssl_attr *ssl_attr = &csp->ssl_client_attr;
   size_t message_len = 0;
   int ret = 0;
   struct certs_chain *cert = NULL;
   const size_t head_length = 63;

   /* Header of message with certificate information */
   const char message_begin[] =
      "HTTP/1.1 403 Certificate validation failed\r\n"
      "Content-Type: text/html\r\n"
      "Connection: close\r\n\r\n"
      "<!DOCTYPE html>\n"
      "<html><head><title>Server certificate verification failed</title></head>\n"
      "<body><h1>Server certificate verification failed</h1>\n"
      "<p><a href=\"https://" CGI_SITE_2_HOST "/\">Privoxy</a> was unable "
      "to securely connect to the destination server.</p>"
      "<p>Reason: ";
   const char message_end[] = "</body></html>\n";
   char reason[INVALID_CERT_INFO_BUF_SIZE];
   memset(reason, 0, sizeof(reason));

   /* Get verification message from verification return code */
   ssl_crt_verify_info(reason, sizeof(reason), csp);

   /*
    * Computing total length of message with all certificates inside
    */
   message_len = strlen(message_begin) + strlen(message_end)
                 + strlen(reason) + strlen("</p>") + 1;

   cert = &(csp->server_certs_chain);
   while (cert->next != NULL)
   {
      size_t base64_len;

      if (cert->file_buf != NULL)
      {
         base64_len = 4 * ((strlen(cert->file_buf) + 2) / 3) + 1;

         message_len += strlen(cert->info_buf) + strlen("<pre></pre>\n")
            +  base64_len + strlen("<a href=\"data:application"
            "/x-x509-ca-cert;base64,\">Download certificate</a>");
      }
      else
      {
         log_error(LOG_LEVEL_ERROR,
            "Incomplete certificate information for %s.",
            csp->http->hostport);
      }
      cert = cert->next;
   }

   /*
    * Joining all blocks in one long message
    */
   char message[message_len];
   memset(message, 0, message_len);

   strlcpy(message, message_begin, message_len);
   strlcat(message, reason       , message_len);
   strlcat(message, "</p>"       , message_len);

   cert = &(csp->server_certs_chain);
   while (cert->next != NULL)
   {
      if (cert->file_buf != NULL)
      {
                                                       /* +1 for terminating null */
         size_t base64_len = base64_len = 4 * ((strlen(cert->file_buf) + 2) / 3) + 1;
         size_t olen = 0;
         char base64_buf[base64_len];

         memset(base64_buf, 0, base64_len);

         /* Encoding certificate into base64 code */
         ret = ssl_base64_encode((unsigned char*)base64_buf,
                  base64_len, &olen, (const unsigned char*)cert->file_buf,
                  strlen(cert->file_buf));
         if (ret != 0)
         {
            log_error(LOG_LEVEL_ERROR,
               "Encoding to base64 failed, buffer is to small");
         }

         strlcat(message, "<pre>",        message_len);
         strlcat(message, cert->info_buf, message_len);
         strlcat(message, "</pre>\n",     message_len);

         if (ret == 0)
         {
            strlcat(message, "<a href=\"data:application/x-x509-ca-cert;base64,",
               message_len);
            strlcat(message, base64_buf, message_len);
            strlcat(message, "\">Download certificate</a>", message_len);
         }
      }

      cert = cert->next;
   }
   strlcat(message, message_end, message_len);

   if (0 == strcmpic(csp->http->gpc, "HEAD"))
   {
      /* Cut off body */
      char *header_end = strstr(message, "\r\n\r\n");
      if (header_end != NULL)
      {
         header_end[3] = '\0';
      }
   }

   /*
    * Sending final message to client
    */
   (void)ssl_send_data(ssl_attr, (const unsigned char *)message, strlen(message));

   free_certificate_chain(csp);

   log_error(LOG_LEVEL_CRUNCH, "Certificate error: %s: https://%s%s",
      reason, csp->http->hostport, csp->http->path);
   log_error(LOG_LEVEL_CLF, "%s - - [%T] \"%s https://%s%s %s\" 403 %lu",
      csp->ip_addr_str, csp->http->gpc, csp->http->hostport, csp->http->path,
      csp->http->version, message_len-head_length);

#ifdef FEATURE_CONNECTION_KEEP_ALIVE
   csp->flags &= ~CSP_FLAG_CLIENT_CONNECTION_KEEP_ALIVE;
   csp->flags |= CSP_FLAG_SERVER_SOCKET_TAINTED;
#endif
}


/*********************************************************************
 *
 * Function    :  file_exists
 *
 * Description :  Tests if file exists and is readable.
 *
 * Parameters  :
 *          1  :  path = Path to tested file.
 *
 * Returns     :  1 => File exists and is readable.
 *                0 => File doesn't exist or is not readable.
 *
 *********************************************************************/
extern int file_exists(const char *path)
{
   FILE *f;
   if ((f = fopen(path, "r")) != NULL)
   {
      fclose(f);
      return 1;
   }

   return 0;
}


/*********************************************************************
 *
 * Function    :  make_certs_path
 *
 * Description : Creates path to file from three pieces. This function
 *               takes parameters and puts them in one new mallocated
 *               char * in correct order. Returned variable must be freed
 *               by caller. This function is mainly used for creating
 *               paths of certificates and keys files.
 *
 * Parameters  :
 *          1  :  conf_dir  = Name/path of directory where is the file.
 *                            '.' can be used for current directory.
 *          2  :  file_name = Name of file in conf_dir without suffix.
 *          3  :  suffix    = Suffix of given file_name.
 *
 * Returns     :  path => Path was built up successfully
 *                NULL => Path can't be built up
 *
 *********************************************************************/
extern char *make_certs_path(const char *conf_dir, const char *file_name,
   const char *suffix)
{
   /* Test if all given parameters are valid */
   if (conf_dir == NULL || *conf_dir == '\0' || file_name == NULL ||
      *file_name == '\0' || suffix == NULL || *suffix == '\0')
   {
      log_error(LOG_LEVEL_ERROR,
         "make_certs_path failed: bad input parameters");
      return NULL;
   }

   char *path = NULL;
   size_t path_size = strlen(conf_dir)
      + strlen(file_name) + strlen(suffix) + 2;

   /* Setting delimiter and editing path length */
#if defined(_WIN32)
   char delim[] = "\\";
   path_size += 1;
#else /* ifndef _WIN32 */
   char delim[] = "/";
#endif /* ifndef _WIN32 */

   /*
    * Building up path from many parts
    */
#if defined(unix)
   if (*conf_dir != '/' && basedir && *basedir)
   {
      /*
       * Replacing conf_dir with basedir. This new variable contains
       * absolute path to cwd.
       */
      path_size += strlen(basedir) + 2;
      path = zalloc_or_die(path_size);

      strlcpy(path, basedir,   path_size);
      strlcat(path, delim,     path_size);
      strlcat(path, conf_dir,  path_size);
      strlcat(path, delim,     path_size);
      strlcat(path, file_name, path_size);
      strlcat(path, suffix,    path_size);
   }
   else
#endif /* defined unix */
   {
      path = zalloc_or_die(path_size);

      strlcpy(path, conf_dir,  path_size);
      strlcat(path, delim,     path_size);
      strlcat(path, file_name, path_size);
      strlcat(path, suffix,    path_size);
   }

   return path;
}


/*********************************************************************
 *
 * Function    :  get_certificate_serial
 *
 * Description :  Computes serial number for new certificate from host
 *                name hash. This hash must be already saved in csp
 *                structure.
 *
 * Parameters  :
 *          1  :  csp = Current client state (buffers, headers, etc...)
 *
 * Returns     :  Serial number for new certificate
 *
 *********************************************************************/
extern unsigned long get_certificate_serial(struct client_state *csp)
{
   unsigned long exp    = 1;
   unsigned long serial = 0;

   int i = CERT_SERIAL_NUM_LENGTH;

   for (; i >= 0; i--)
   {
      serial += exp * (unsigned)csp->http->hash_of_host[i];
      exp *= 256;
   }
   return serial;
}


/*********************************************************************
 *
 * Function    :  generate_certificate_valid_date
 *
 * Description :  Turns a time_t into the format expected by mbedTLS.
 *
 * Parameters  :
 *          1  :  time_spec = The timestamp to convert
 *          2  :  buffer = The buffer to write the date to
 *          3  :  buffer_size = The size of the buffer
 *          4  :  fmt = format
 *
 * Returns     :   0 => The conversion worked
 *                 1 => The conversion failed
 *
 *********************************************************************/
static int generate_certificate_valid_date(time_t time_spec, char *buffer,
                                           size_t buffer_size, const char *fmt)
{
   struct tm valid_date;
   struct tm *timeptr;
   size_t ret;

   timeptr = privoxy_gmtime_r(&time_spec, &valid_date);
   if (NULL == timeptr)
   {
      return 1;
   }

   ret = strftime(buffer, buffer_size, fmt, timeptr);
   if (ret <= 0)
   {
      return 1;
   }

   return 0;

}


/*********************************************************************
 *
 * Function    :  get_certificate_valid_from_date
 *
 * Description :  Generates a "valid from" date in the format
 *                expected by mbedTLS.
 *
 * Parameters  :
 *          1  :  buffer = The buffer to write the date to
 *          2  :  buffer_size = The size of the buffer
 *          3  :  fmt = format
 *
 * Returns     :   0 => The generation worked
 *                 1 => The generation failed
 *
 *********************************************************************/
extern int get_certificate_valid_from_date(char *buffer, size_t buffer_size, const char *fmt)
{
   time_t time_spec;

   time_spec = time(NULL);
   /* 1 month in the past */
   time_spec -= 30 * 24 * 60 * 60;

   return generate_certificate_valid_date(time_spec, buffer, buffer_size, fmt);

}


/*********************************************************************
 *
 * Function    :  get_certificate_valid_to_date
 *
 * Description :  Generates a "valid to" date in the format
 *                expected by mbedTLS.
 *
 * Parameters  :
 *          1  :  buffer = The buffer to write the date to
 *          2  :  buffer_size = The size of the buffer
 *          3  :  fmt = format
 *
 * Returns     :   0 => The generation worked
 *                 1 => The generation failed
 *
 *********************************************************************/
extern int get_certificate_valid_to_date(char *buffer, size_t buffer_size, const char *fmt)
{
   time_t time_spec;

   time_spec = time(NULL);
   /* Three months in the future */
   time_spec += 90 * 24 * 60 * 60;

   return generate_certificate_valid_date(time_spec, buffer, buffer_size, fmt);

}


/*********************************************************************
 *
 * Function    :  enforce_sane_certificate_state
 *
 * Description :  Makes sure the certificate state is sane.
 *
 * Parameters  :
 *          1  :  certificate = Path to the potentionally existing certifcate.
 *          2  :  key = Path to the potentionally existing key.
 *
 * Returns     :   -1 => Error
 *                 0 => Certificate state is sane
 *
 *********************************************************************/
extern int enforce_sane_certificate_state(const char *certificate, const char *key)
{
   const int certificate_exists = file_exists(certificate);
   const int key_exists = file_exists(key);

   if (!certificate_exists && key_exists)
   {
      log_error(LOG_LEVEL_ERROR,
         "A website key already exists but there's no matching certificate. "
         "Removing %s before creating a new key and certificate.", key);
      if (unlink(key))
      {
         log_error(LOG_LEVEL_ERROR, "Failed to unlink %s: %E", key);

         return -1;
      }
   }
   if (certificate_exists && !key_exists)
   {
      log_error(LOG_LEVEL_ERROR,
         "A certificate exists but there's no matching key. "
         "Removing %s before creating a new key and certificate.", certificate);
      if (unlink(certificate))
      {
         log_error(LOG_LEVEL_ERROR, "Failed to unlink %s: %E", certificate);

         return -1;
      }
   }

   return 0;

}
