#ifndef _common_h
#define _common_h

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#ifdef HAVE_MATRIXSSL
#include <matrixSsl.h>
#endif

#define CA_LIST "/etc/root.pem"
#define HOST "localhost"
#define RANDOM "random.pem"
#define PORT 443
#define BUFSIZZ 1024

#ifdef HAVE_OPENSSL

#include <openssl/ssl.h>

extern BIO *bio_err;
int berr_exit(char *string);
int err_exit(char *string);

SSL_CTX *initialize_ctx(char *keyfile, char *password);
void destroy_ctx(SSL_CTX *ctx);

#ifndef ALLOW_OLD_VERSIONS
#if (OPENSSL_VERSION_NUMBER < 0x00905100L)
#error "Must use OpenSSL 0.9.6 or later"
#endif /* OPENSSL_VERSION_NUMBER */
#endif /* ALLOW_OLD_VERSIONS */
#endif /* HAVE_OPENSSL */

#endif /* _common_h */
