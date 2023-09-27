/*
 * Declare our OpenSSL API usage, to avoid deprecation warnings.
 * This must be included before any OpenSSL header files.
 */

#ifndef NETSNMP_OPENSSL_COMPAT_H
#define NETSNMP_OPENSSL_COMPAT_H

/* Our interface matches OpenSSL 1.0.0 */
#define OPENSSL_API_COMPAT 0x10000000L

#endif /* NETSNMP_OPENSSL_COMPAT_H */
