/**
 * \file rsa.h
 */
#ifndef _RSA_H
#define _RSA_H

#ifdef __cplusplus
extern "C" {
#endif

#define ERR_RSA_KEYGEN_FAILED                   0x0300
#define ERR_RSA_PUBLIC_FAILED                   0x0310
#define ERR_RSA_PRIVATE_FAILED                  0x0320
#define ERR_RSA_KEY_CHECK_FAILED                0x0330
#define ERR_RSA_ENCRYPT_FAILED                  0x0340
#define ERR_RSA_DECRYPT_FAILED                  0x0350
#define ERR_RSA_SIGN_FAILED                     0x0360
#define ERR_RSA_VERIFY_FAILED                   0x0370

/*
 * PKCS#1 stuff
 */
#define RSA_NONE            0
#define RSA_MD2             2
#define RSA_MD4             3
#define RSA_MD5             4
#define RSA_SHA1            5

#define RSA_SIGN            0x01
#define RSA_CRYPT           0x02

/*
 * DigestInfo ::= SEQUENCE {
 *   digestAlgorithm DigestAlgorithmIdentifier,
 *   digest Digest }
 *
 * DigestAlgorithmIdentifier ::= AlgorithmIdentifier
 *
 * Digest ::= OCTET STRING
 */
#define ASN1_HASH_MDX                       \
    "\x30\x20\x30\x0C\x06\x08\x2A\x86\x48"  \
    "\x86\xF7\x0D\x02\x00\x05\x00\x04\x10"

#define ASN1_HASH_SHA1                      \
    "\x30\x21\x30\x09\x06\x05\x2B\x0E\x03"  \
    "\x02\x1A\x05\x00\x04\x14"

#include "bignum.h"

typedef struct
{
    int ver;    /*!<  should be 0       */
    int len;    /*!<  size(N) in chars  */
    mpi N;      /*!<  public modulus    */
    mpi E;      /*!<  public exponent   */
    mpi D;      /*!<  private exponent  */

    mpi P;      /*!<  1st prime factor  */
    mpi Q;      /*!<  2nd prime factor  */
    mpi DP;     /*!<  D mod (P - 1)     */
    mpi DQ;     /*!<  D mod (Q - 1)     */
    mpi QP;     /*!<  inverse of Q % P  */

    mpi RN;     /*!<  R^2 mod N         */
    mpi RP;     /*!<  R^2 mod P         */
    mpi RQ;     /*!<  R^2 mod Q         */
}
rsa_context;

/**
 * \brief          Generate an RSA keypair
 *
 * \param ctx      RSA context to be initialized
 * \param nbits    size of the public key in bits
 * \param exponent public exponent (e.g., 65537)
 * \param rng_f    points to the RNG function
 * \param rng_d    points to the RNG data 
 *
 * \return         0 if successful, or ERR_RSA_KEYGEN_FAILED
 */
int rsa_gen_key( rsa_context *ctx, int nbits, int exponent,
                 int (*rng_f)(void *), void *rng_d );

/**
 * \brief          Perform an RSA public key operation
 *
 * \return         0 if successful, or ERR_RSA_PUBLIC_FAILED
 *
 * \note           This function does not take care of message
 *                 padding: both ilen and olen must be equal to
 *                 the modulus size (ctx->len). Also, be sure
 *                 to set input[0] = 0.
 */
int rsa_public( rsa_context *ctx, unsigned char *input,  int ilen,
                                  unsigned char *output, int olen );

/**
 * \brief          Perform an RSA private key operation
 *
 * \return         0 if successful, or ERR_RSA_PRIVATE_FAILED
 *
 * \note           This function does not take care of message
 *                 padding: both ilen and olen must be equal to
 *                 the modulus size (ctx->len). Also, be sure
 *                 to set input[0] = 0.
 */
int rsa_private( rsa_context *ctx, unsigned char *input,  int ilen,
                                   unsigned char *output, int olen );

/**
 * \brief          Return 0 if the public key is valid,
 *                 or ERR_RSA_KEY_CHECK_FAILED
 */
int rsa_check_pubkey( rsa_context *ctx );

/**
 * \brief          Return 0 if the private key is valid,
 *                 or ERR_RSA_KEY_CHECK_FAILED
 */
int rsa_check_privkey( rsa_context *ctx );

/**
 * \brief          Add the PKCS1 v1.5 padding and do a public RSA
 *
 * \param ctx      RSA context
 * \param input    buffer holding the data to be encrypted
 * \param ilen     length of the plaintext; cannot be longer
 *                 than the modulus, minus 3+8 for padding
 * \param output   buffer that will hold the ciphertext
 * \param olen     must be the same as the modulus size
 *                 (for example, 128 if RSA-1024 is used)
 *
 * \return         0 if successful, or ERR_RSA_ENCRYPT_FAILED
 */
int rsa_pkcs1_encrypt( rsa_context *ctx,
                       unsigned char *input,  int ilen,
                       unsigned char *output, int olen );

/**
 * \brief          Do a private RSA and remove the PKCS1 v1.5 padding
 *
 * \param ctx      RSA context
 * \param input    buffer holding the encrypted data
 * \param ilen     must be the same as the modulus size
 * \param output   buffer that will hold the plaintext
 * \param olen     size of output buffer, will be updated
 *                 to contain the length of the plaintext
 *
 * \return         0 if successful, or ERR_RSA_DECRYPT_FAILED
 */
int rsa_pkcs1_decrypt( rsa_context *ctx,
                       unsigned char *input,  int  ilen,
                       unsigned char *output, int *olen );

/**
 * \brief          Perform a private RSA to sign a message digest
 *
 * \param ctx      RSA context
 * \param alg_id   RSA_MD2/4/5, RSA_SHA1 or RSA_NONE
 * \param hash     buffer holding the message digest
 * \param hashlen  message digest length (only for RSA_NONE)
 * \param sig      buffer that will hold the ciphertext
 * \param siglen   must be the same as the modulus size
 *                 (for example, 128 if RSA-1024 is used)
 *
 * \return         0 if successful, or ERR_RSA_SIGN_FAILED
 */
int rsa_pkcs1_sign( rsa_context *ctx, int alg_id,
                    unsigned char *hash, int hashlen,
                    unsigned char *sig,  int siglen );

/**
 * \brief          Perform a public RSA and check the message digest
 *
 * \param ctx      points to an RSA public key
 * \param alg_id   RSA_MD2/4/5, RSA_SHA1 or RSA_NONE
 * \param hash     buffer holding the message digest
 * \param hashlen  message digest length (only for RSA_NONE)
 * \param sig      buffer holding the ciphertext
 * \param siglen   must be the same as the modulus size
 *
 * Returns 0 if successful, or ERR_RSA_VERIFY_FAILED
 */
int rsa_pkcs1_verify( rsa_context *ctx, int alg_id,
                      unsigned char *hash, int hashlen,
                      unsigned char *sig,  int siglen );

/**
 * \brief          Free the components of an RSA key
 */
void rsa_free( rsa_context *ctx );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int rsa_self_test( void );

#ifdef __cplusplus
}
#endif

#endif
