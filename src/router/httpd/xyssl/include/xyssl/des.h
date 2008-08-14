/**
 * \file des.h
 */
#ifndef _DES_H
#define _DES_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          DES context structure
 */
typedef struct
{
    unsigned long esk[32];     /*!< DES encryption subkeys */
    unsigned long dsk[32];     /*!< DES decryption subkeys */
}
des_context;

/**
 * \brief          Triple-DES context structure
 */
typedef struct
{
    unsigned long esk[96];     /*!< Triple-DES encryption subkeys */
    unsigned long dsk[96];     /*!< Triple-DES decryption subkeys */
}
des3_context;

/**
 * \brief          DES key schedule (56-bit)
 *
 * \param ctx      DES context to be initialized
 * \param key      8-byte secret key
 */
void des_set_key( des_context *ctx, unsigned char key[8] );

/**
 * \brief          DES block encryption (ECB mode)
 *
 * \param ctx      DES context
 * \param input    plaintext  block
 * \param output   ciphertext block
 */
void des_encrypt( des_context *ctx,
                  unsigned char input[8],
                  unsigned char output[8] );

/**
 * \brief          DES block decryption (ECB mode)
 *
 * \param ctx      DES context
 * \param input    ciphertext block
 * \param output   plaintext  block
 */
void des_decrypt( des_context *ctx,
                  unsigned char input[8],
                  unsigned char output[8] );

/**
 * \brief          DES-CBC buffer encryption
 *
 * \param ctx      DES context
 * \param iv       initialization vector (modified after use)
 * \param input    buffer holding the plaintext
 * \param output   buffer holding the ciphertext
 * \param len      length of the data to be encrypted
 */
void des_cbc_encrypt( des_context *ctx,
                      unsigned char iv[8],
                      unsigned char *input,
                      unsigned char *output,
                      int len );

/**
 * \brief          DES-CBC buffer decryption
 *
 * \param ctx      DES context
 * \param iv       initialization vector (modified after use)
 * \param input    buffer holding the ciphertext
 * \param output   buffer holding the plaintext
 * \param len      length of the data to be decrypted
 */
void des_cbc_decrypt( des_context *ctx,
                      unsigned char iv[8],
                      unsigned char *input,
                      unsigned char *output,
                      int len );

/**
 * \brief          Triple-DES key schedule (112-bit)
 *
 * \param ctx      3DES context to be initialized
 * \param key      16-byte secret key
 */
void des3_set_2keys( des3_context *ctx, unsigned char key[16] );

/**
 * \brief          Triple-DES key schedule (168-bit)
 *
 * \param ctx      3DES context to be initialized
 * \param key      24-byte secret key
 */
void des3_set_3keys( des3_context *ctx, unsigned char key[24] );

/**
 * \brief          Triple-DES block encryption (ECB mode)
 *
 * \param ctx      3DES context
 * \param input    plaintext  block
 * \param output   ciphertext block
 */
void des3_encrypt( des3_context *ctx,
                   unsigned char input[8],
                   unsigned char output[8] );

/**
 * \brief          Triple-DES block decryption (ECB mode)
 *
 * \param ctx      3DES context
 * \param input    ciphertext block
 * \param output   plaintext  block
 */
void des3_decrypt( des3_context *ctx,
                   unsigned char input[8],
                   unsigned char output[8] );

/**
 * \brief          3DES-CBC buffer encryption
 *
 * \param ctx      3DES context
 * \param iv       initialization vector (modified after use)
 * \param input    buffer holding the plaintext
 * \param output   buffer holding the ciphertext
 * \param len      length of the data to be encrypted
 */
void des3_cbc_encrypt( des3_context *ctx,
                       unsigned char iv[8],
                       unsigned char *input,
                       unsigned char *output,
                       int len );

/**
 * \brief          3DES-CBC buffer decryption
 *
 * \param ctx      3DES context
 * \param iv       initialization vector (modified after use)
 * \param input    buffer holding the ciphertext
 * \param output   buffer holding the plaintext
 * \param len      length of the data to be decrypted
 */
void des3_cbc_decrypt( des3_context *ctx,
                       unsigned char iv[8],
                       unsigned char *input,
                       unsigned char *output,
                       int len );

/*
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int des_self_test( void );

#ifdef __cplusplus
}
#endif

#endif /* des.h */
