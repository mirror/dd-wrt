/**
 * \file ssl.h
 */
#ifndef _SSL_H
#define _SSL_H

#ifdef __cplusplus
extern "C" {
#endif

#define ERR_SSL_INVALID_MAC                     0x1000
#define ERR_SSL_INVALID_RECORD                  0x1800
#define ERR_SSL_INVALID_MODULUS_SIZE            0x2000
#define ERR_SSL_UNKNOWN_CIPHER                  0x2800
#define ERR_SSL_NO_CIPHER_CHOSEN                0x3000
#define ERR_SSL_NO_SESSION_FOUND                0x3800
#define ERR_SSL_NO_CLIENT_CERTIFICATE           0x4000
#define ERR_SSL_CERTIFICATE_TOO_LARGE           0x4800
#define ERR_SSL_CERTIFICATE_REQUIRED            0x5000
#define ERR_SSL_PRIVATE_KEY_REQUIRED            0x5800
#define ERR_SSL_CA_CHAIN_REQUIRED               0x6000
#define ERR_SSL_UNEXPECTED_MESSAGE              0x6800
#define ERR_SSL_FATAL_ALERT_MESSAGE             0x7000
#define ERR_SSL_PEER_VERIFY_FAILED              0x7800
#define ERR_SSL_PEER_CLOSE_NOTIFY               0x8000
#define ERR_SSL_BAD_HS_CLIENT_HELLO             0x8800
#define ERR_SSL_BAD_HS_SERVER_HELLO             0x9000
#define ERR_SSL_BAD_HS_CERTIFICATE              0x9800
#define ERR_SSL_BAD_HS_CERTIFICATE_REQUEST      0xA000
#define ERR_SSL_BAD_HS_SERVER_KEY_EXCHANGE      0xA800
#define ERR_SSL_BAD_HS_SERVER_HELLO_DONE        0xB000
#define ERR_SSL_BAD_HS_CLIENT_KEY_EXCHANGE      0xB800
#define ERR_SSL_BAD_HS_CERTIFICATE_VERIFY       0xC000
#define ERR_SSL_BAD_HS_CHANGE_CIPHER_SPEC       0xC800
#define ERR_SSL_BAD_HS_FINISHED                 0xD000

/*
 * Various constants
 */
#define SSLV3_MAJOR_VERSION             3
#define SSLV3_MINOR_VERSION             0
#define TLS10_MINOR_VERSION             1
#define TLS11_MINOR_VERSION             2

#define SSL_IS_CLIENT                   0
#define SSL_IS_SERVER                   1
#define SSL_COMPRESS_NULL               0

#define SSL_VERIFY_NONE                 0
#define SSL_VERIFY_OPTIONAL             1
#define SSL_VERIFY_REQUIRED             2

#define SSL_SESSION_TBL_LEN          8192
#define SSL_MAX_CONTENT_LEN         16384
#define SSL_EXPIRATION_TIME         86400

/*
 * Allow an extra 512 bytes for the record header
 * and encryption overhead (counter + MAC + padding).
 */
#define SSL_BUFFER_LEN (SSL_MAX_CONTENT_LEN + 512)

/*
 * Supported ciphersuites
 */
#define SSL3_RSA_RC4_128_MD5            4
#define SSL3_RSA_RC4_128_SHA            5
#define SSL3_RSA_DES_168_SHA           10
#define SSL3_EDH_RSA_DES_168_SHA       22
#define TLS1_RSA_AES_256_SHA           53
#define TLS1_EDH_RSA_AES_256_SHA       57

#define SSL_MAX_BUF_SIZE               0xC800 + 2048
#define SSL_SEND_BUF_SZ                3 * 1024

extern int ssl_default_ciphers[];

/*
 * Message, alert and handshake types
 */
#define SSL_MSG_CHANGE_CIPHER_SPEC     20
#define SSL_MSG_ALERT                  21
#define SSL_MSG_HANDSHAKE              22
#define SSL_MSG_APPLICATION_DATA       23

#define SSL_ALERT_CLOSE_NOTIFY          0
#define SSL_ALERT_WARNING               1
#define SSL_ALERT_FATAL                 2
#define SSL_ALERT_NO_CERTIFICATE       41

#define SSL_HS_HELLO_REQUEST            0
#define SSL_HS_CLIENT_HELLO             1
#define SSL_HS_SERVER_HELLO             2
#define SSL_HS_CERTIFICATE             11
#define SSL_HS_SERVER_KEY_EXCHANGE     12
#define SSL_HS_CERTIFICATE_REQUEST     13
#define SSL_HS_SERVER_HELLO_DONE       14
#define SSL_HS_CERTIFICATE_VERIFY      15
#define SSL_HS_CLIENT_KEY_EXCHANGE     16
#define SSL_HS_FINISHED                20

/*
 * SSL state machine
 */
typedef enum
{
    SSL_HELLO_REQUEST,
    SSL_CLIENT_HELLO,
    SSL_SERVER_HELLO,
    SSL_SERVER_CERTIFICATE,
    SSL_SERVER_KEY_EXCHANGE,
    SSL_CERTIFICATE_REQUEST,
    SSL_SERVER_HELLO_DONE,
    SSL_CLIENT_CERTIFICATE,
    SSL_CLIENT_KEY_EXCHANGE,
    SSL_CERTIFICATE_VERIFY,
    SSL_CLIENT_CHANGE_CIPHER_SPEC,
    SSL_CLIENT_FINISHED,
    SSL_SERVER_CHANGE_CIPHER_SPEC,
    SSL_SERVER_FINISHED,
    SSL_HANDSHAKE_OVER
}
ssl_states;

#include "sha1.h"
#include "md5.h"
#include "rsa.h"
#include "dhm.h"
#include "x509.h"

typedef struct
{
    int state;                  /*!<  SSL handshake: current state    */

    /*
     * Negotiated protocol version
     */
    int major_ver;              /*!< equal to  SSLV3_MAJOR_VERSION    */
    int minor_ver;              /*!< either 0: SSLv3, or 1: TLSv1.0   */
    unsigned char max_ver[2];   /*!< max. version supported by client */

    /*
     * Record layer -- incoming data
     */
    unsigned char *in_ctr;      /*!< 64-bit incoming message counter  */
    unsigned char *in_hdr;      /*!< 5-byte record header (in_ctr+8)  */
    unsigned char *in_msg;      /*!< the message payload  (in_hdr+5)  */

    int read_fd;                /*!< descriptor for read operations   */
    int in_msgtype;             /*!< record header: message type      */
    int in_msglen;              /*!< record header: message length    */

    int in_left;                /*!< amount of data read so far       */
    int in_hslen;               /*!< current handshake message length */
    int nb_zero;                /*!< # of 0-length encrypted messages */

    /*
     * Record layer -- outgoing data
     */
    unsigned char *out_ctr;     /*!< 64-bit outgoing message counter  */
    unsigned char *out_hdr;     /*!< 5-byte record header (out_ctr+8) */
    unsigned char *out_msg;     /*!< the message payload  (out_hdr+5) */

    int write_fd;               /*!< descriptor for write operations  */
    int out_msgtype;            /*!< record header: message type      */
    int out_msglen;             /*!< record header: message length    */

    int out_left;               /*!< amount of data not yet written   */
    int out_uoff;               /*!< offset in user-supplied buffer   */

    /*
     * PKI stuff
     */
    rsa_context *own_key;               /*!<  own RSA private key     */
    x509_cert *own_cert;                /*!<  own X.509 certificate   */
    x509_cert *ca_chain;                /*!<  own trusted CA chain    */
    x509_cert *peer_cert;               /*!<  peer X.509 cert chain   */
    char *peer_cn;                      /*!<  expected peer CN        */

    dhm_context dhm_ctx;                /*!<  DHM key exchange        */
    char *dhm_P;                        /*!<  DHM modulus   (server)  */
    char *dhm_G;                        /*!<  DHM generator (server)  */

    int endpoint;                       /*!<  0: client, 1: server    */
    int authmode;                       /*!<  verification mode       */
    int client_auth;                    /*!<  flag for client auth.   */
    int verify_result;                  /*!<  verification result     */

    /*
     * Session stuff
     */
    int resumed;                        /*!<  session resuming flag   */
    int sidlen;                         /*!<  session id length       */
    unsigned char sessid[32];           /*!<  session id              */
    unsigned char *sidtable;            /*!<  table of session IDs    */

    /*
     * Crypto stuff
     */
     md5_context hs_md5;                /*!<   MD5( Handshake msgs )  */
    sha1_context hs_sha1;               /*!<  SHA1( Handshake msgs )  */

    int (*rng_f)(void *);               /*!<  RNG function            */
    void *rng_d;                        /*!<  RNG data                */

    int pmslen;                         /*!<  premaster length        */
    unsigned char premaster[256];       /*!<  premaster secret        */
    unsigned char randbytes[64];        /*!<  random bytes            */
    unsigned char master[48];           /*!<  master secret           */

    int *cipherlist;                    /*!<  accepted ciphersuites   */
    int cipher;                         /*!<  current chosen cipher   */
    int keylen;                         /*!<  symmetric key length    */
    int minlen;                         /*!<  min. ciphertext length  */

    int ctxlen;                         /*!<  cipher context length   */
    void *ctx_enc;                      /*!<  encryption context      */
    void *ctx_dec;                      /*!<  decryption context      */

    int ivlen;                          /*!<  IV length               */
    unsigned char iv_enc[16];           /*!<  IV (encryption)         */
    unsigned char iv_dec[16];           /*!<  IV (decryption)         */

    int maclen;                         /*!<  MAC length              */
    unsigned char mac_enc[32];          /*!<  MAC (encryption)        */
    unsigned char mac_dec[32];          /*!<  MAC (decryption)        */
    unsigned char ssl_buffer[SSL_MAX_BUF_SIZE];
    int ssl_buflen;
}
ssl_context;

/*
 * Internal functions (do not call directly)
 */
int ssl_client_start( ssl_context *ssl );
int ssl_server_start( ssl_context *ssl );

int ssl_derive_keys( ssl_context *ssl );
int ssl_calc_verify( ssl_context *ssl, unsigned char hash[36] );

int ssl_read_record(  ssl_context *ssl, int do_crypt );
int ssl_write_record( ssl_context *ssl, int do_crypt );
int ssl_flush_output( ssl_context *ssl );

int ssl_write_certificate( ssl_context *ssl );
int ssl_parse_certificate( ssl_context *ssl );

int ssl_write_change_cipher_spec( ssl_context *ssl );
int ssl_parse_change_cipher_spec( ssl_context *ssl );

int ssl_write_finished( ssl_context *ssl );
int ssl_parse_finished( ssl_context *ssl );

/**
 * \brief          Initialize the SSL context. If client_resume is
 *                 not null, the session id and premaster secret
 *                 are preserved (client-side only).
 *
 * \return         0 if successful, or 1 if memory allocation failed
 */
int ssl_init( ssl_context *ssl, int client_resume );

/**
 * \brief          Set the current endpoint type,
 *                 SSL_IS_CLIENT or SSL_IS_SERVER
 */
void ssl_set_endpoint( ssl_context *ssl, int endpoint );

/**
 * \brief          Set the certificate verification mode
 *
 * \param mode     can be:
 *
 *  SSL_VERIFY_NONE:      peer certificate is not checked (default).
 *
 *  SSL_VERIFY_OPTIONAL:  peer certificate is checked, however the
 *                        handshake continues even if verification failed;
 *                        you may want to check ssl->verify_result after.
 *
 *  SSL_VERIFY_REQUIRED:  peer *must* present a valid certificate,
 *                        handshake is aborted if verification failed.
 */
void ssl_set_authmode( ssl_context *ssl, int authmode );

/**
 * \brief          Set the random number generator function
 */
void ssl_set_rng_func( ssl_context *ssl,
                       int (*rng_f)(void *),
                       void *rng_d );

/**
 * \brief          Set the read and write file descriptors
 */
void ssl_set_io_files( ssl_context *ssl, int read_fd, int write_fd );

/**
 * \brief          Set the list of allowed ciphersuites
 */
void ssl_set_ciphlist( ssl_context *ssl, int *ciphers );

/**
 * \brief          Set the CA certificate chain used to verify peer
 *                 cert, and the peer's expected CommonName (or NULL)
 */
void ssl_set_ca_chain( ssl_context *ssl, x509_cert *ca, char *cn );

/**
 * \brief          Set own certificate and private RSA key
 */
void ssl_set_rsa_cert( ssl_context *ssl, x509_cert *own_cert,
                       rsa_context *own_key );

/**
 * \brief          Set the global session ID table (server-side only)
 */
void ssl_set_sidtable( ssl_context *ssl, unsigned char *sidtable );

/**
 * \brief          Set the Diffie-Hellman public P and G values,
 *                 provided as hexadecimal strings (server-side only)
 */
void ssl_set_dhm_vals( ssl_context *ssl, char *dhm_P, char *dhm_G );

/**
 * \brief          Return the result of the certificate verification
 */
int ssl_get_verify_result( ssl_context *ssl );

/**
 * \brief          Return the name of the current cipher
 */
char *ssl_get_cipher_name( ssl_context *ssl );

/**
 * \brief          Perform the SSL handshake
 *
 * \return         0 if successful, ERR_NET_WOULD_BLOCK (only when
 *                 the socket is set to non-blocking), or a specific
 *                 SSL error code.
 */
int ssl_handshake( ssl_context *ssl );

/**
 * \brief          Read at most 'len' application data bytes
 *
 * \return         0 if successful, ERR_NET_WOULD_BLOCK (only when
 *                 the socket is set to non-blocking), or a specific
 *                 SSL error code.
 *
 * \note           len is updated to reflect the actual number of
 *                 data bytes read.
 */
int ssl_read( ssl_context *ssl, unsigned char *buf, int *len );

/**
 * \brief          Write 'len' application data bytes
 *
 * \return         0 if successful, ERR_NET_WOULD_BLOCK (only when
 *                 the socket is set to non-blocking), or a specific
 *                 SSL error code.
 *
 * \note           When the socket is set to non-blocking and this
 *                 function returns ERR_NET_WOULD_BLOCK, it should
 *                 be called again with the *same* arguments until
 *                 it returns 0.
 */
int ssl_write( ssl_context *ssl, unsigned char *buf, int len );

int ssl_printf(ssl_context *ssl, const char *format, ...);

int ssl_read_line( ssl_context *ssl, unsigned char *buf, int *len );

void ssl_flush( ssl_context *ssl);


/**
 * \brief          Notify the peer that the connection is being closed
 */
int ssl_close_notify( ssl_context *ssl );

/**
 * \brief          Free an SSL context
 */
void ssl_free( ssl_context *ssl );

#ifdef __cplusplus
}
#endif

#endif /* ssl.h */
