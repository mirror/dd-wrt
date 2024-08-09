#ifndef SSL_COMMON_H_INCLUDED
#define SSL_COMMON_H_INCLUDED
/*********************************************************************
*
* File        :  $Source: /cvsroot/ijbswa/current/ssl_common.h,v $
*
* Purpose     :  File with TLS/SSL extension. Contains methods for
*                creating, using and closing TLS/SSL connections that do
*                not depend on particular TLS/SSL library.
*
* Copyright   :  Written by and Copyright (c) 2017 Vaclav Svec. FIT CVUT.
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

#include "project.h"

#define RSA_KEY_PUBLIC_EXPONENT          65537             /* Public exponent for RSA private key generating */
#define RSA_KEYSIZE                      2048              /* Size of generated RSA keys */
#define ERROR_BUF_SIZE                   1024              /* Size of buffer for error messages */
#define INVALID_CERT_INFO_BUF_SIZE       2048              /* Size of buffer for message with information about reason of certificate invalidity. Data after the end of buffer will not be saved */
#define KEY_FILE_TYPE                    ".pem"
#define CERT_FILE_TYPE                   ".crt"

#define CERT_PARAM_COMMON_NAME_FCODE      "CN"
#define CERT_PARAM_ORGANIZATION_FCODE     "O"
#define CERT_PARAM_ORG_UNIT_FCODE         "OU"
#define CERT_PARAM_COUNTRY_FCODE          "C"
#define CERT_PARAM_COUNTRY_CODE           "CZ"
#define CERT_SUBJECT_PASSWORD            ""

/*
 * Properties of cert for generating
 */
typedef struct {
   char       *issuer_crt;                         /* filename of the issuer certificate       */
   char       *subject_key;                        /* filename of the subject key file         */
   char       *issuer_key;                         /* filename of the issuer key file          */
   const char *subject_pwd;                        /* password for the subject key file        */
   const char *issuer_pwd;                         /* password for the issuer key file         */
   char       *output_file;                        /* where to store the constructed key file  */
   const char *subject_name;                       /* subject name for certificate             */
   char       issuer_name[ISSUER_NAME_BUF_SIZE];   /* issuer name for certificate              */
   const char *not_before;                         /* validity period not before               */
   const char *not_after;                          /* validity period not after                */
   const char *serial;                             /* serial number string                     */
   int        is_ca;                               /* is a CA certificate                      */
   int        max_pathlen;                         /* maximum CA path length                   */
} cert_options;

extern void free_certificate_chain(struct client_state *csp);
extern int file_exists(const char *path);
extern char *make_certs_path(const char *conf_dir, const char *file_name,
   const char *suffix);
extern unsigned long get_certificate_serial(struct client_state *csp);
extern int get_certificate_valid_from_date(char *buffer, size_t buffer_size, const char *fmt);
extern int get_certificate_valid_to_date(char *buffer, size_t buffer_size, const char *fmt);
extern int enforce_sane_certificate_state(const char *certificate, const char *key);

#endif /* ndef SSL_COMMON_H_INCLUDED */
