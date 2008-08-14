/**
 * \file md2.h
 */
#ifndef _MD2_H
#define _MD2_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          MD2 context structure
 */
typedef struct
{
    unsigned char state[48];    /*!< intermediate digest state  */
    unsigned char cksum[16];    /*!< checksum of the data block */
    unsigned char buffer[16];   /*!< data block being processed */
    int left;                   /*!< amount of data in buffer   */
}
md2_context;

/**
 * \brief          MD2 context setup
 *
 * \param ctx      MD2 context to be initialized
 */
void md2_starts( md2_context *ctx );

/**
 * \brief          MD2 process buffer
 *
 * \param ctx      MD2 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void md2_update( md2_context *ctx, unsigned char *input, int ilen );

/**
 * \brief          MD2 final digest
 *
 * \param ctx      MD2 context
 * \param output   MD2 checksum result
 */
void md2_finish( md2_context *ctx, unsigned char output[16] );

/**
 * \brief          Output = MD2( input buffer )
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   MD2 checksum result
 */
void md2_csum( unsigned char *input, int ilen,
               unsigned char output[16] );

/**
 * \brief          Output = MD2( file contents )
 *
 * \param path     input file name
 * \param output   MD2 checksum result
 * \return         0 if successful, or 1 if fopen failed
 */
int md2_file( char *path, unsigned char output[16] );

/**
 * \brief          Output = HMAC-MD2( input buffer, hmac key )
 *
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   HMAC-MD2 result
 */
void md2_hmac( unsigned char *key, int keylen,
               unsigned char *input, int ilen,
               unsigned char output[16] );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int md2_self_test( void );

#ifdef __cplusplus
}
#endif

#endif /* md2.h */
