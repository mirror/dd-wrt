/**
 * \file md4.h
 */
#ifndef _MD4_H
#define _MD4_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          MD4 context structure
 */
typedef struct
{
    unsigned long total[2];     /*!< number of bytes processed  */
    unsigned long state[4];     /*!< intermediate digest state  */
    unsigned char buffer[64];   /*!< data block being processed */
}
md4_context;

/**
 * \brief          MD4 context setup
 *
 * \param ctx      MD4 context to be initialized
 */
void md4_starts( md4_context *ctx );

/**
 * \brief          MD4 process buffer
 *
 * \param ctx      MD4 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void md4_update( md4_context *ctx, unsigned char *input, int ilen );

/**
 * \brief          MD4 final digest
 *
 * \param ctx      MD4 context
 * \param output   MD4 checksum result
 */
void md4_finish( md4_context *ctx, unsigned char output[16] );

/**
 * \brief          Output = MD4( input buffer )
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   MD4 checksum result
 */
void md4_csum( unsigned char *input, int ilen,
               unsigned char output[16] );

/**
 * \brief          Output = MD4( file contents )
 *
 * \param path     input file name
 * \param output   MD4 checksum result
 * \return         0 if successful, or 1 if fopen failed
 */
int md4_file( char *path, unsigned char output[16] );

/**
 * \brief          Output = HMAC-MD4( input buffer, hmac key )
 *
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   HMAC-MD4 result
 */
void md4_hmac( unsigned char *key, int keylen,
               unsigned char *input, int ilen,
               unsigned char output[16] );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int md4_self_test( void );

#ifdef __cplusplus
}
#endif

#endif /* md4.h */
