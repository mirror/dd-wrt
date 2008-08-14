/**
 * \file base64.h
 */
#ifndef _BASE64_H
#define _BASE64_H

#ifdef __cplusplus
extern "C" {
#endif

#define ERR_BASE64_BUFFER_TOO_SMALL             0x0010
#define ERR_BASE64_INVALID_CHARACTER            0x0012

/**
 * \brief          Encode a buffer into base64 format
 *
 * \param dst      destination buffer
 * \param dlen     size of the buffer (updated after call)
 * \param src      source buffer
 * \param slen     amount of data to be encoded
 *
 * \return         0 if successful, or ERR_BASE64_BUFFER_TOO_SMALL.
 *                 *dlen is always updated to reflect to amount of
 *                 data that was written (or would have been written)
 *
 * \note           Call this function with *dlen = 0 to obtain the
 *                 required buffer size in *dlen
 */
int base64_encode( unsigned char *dst, int *dlen,
                   unsigned char *src, int  slen );

/**
 * \brief          Decode a base64-formatted buffer
 *
 * \param dst      destination buffer
 * \param dlen     size of the buffer (updated after call)
 * \param src      source buffer
 * \param slen     amount of data to be decoded
 *
 * \return         0 if successful, ERR_BASE64_BUFFER_TOO_SMALL, or
 *                 ERR_BASE64_INVALID_DATA if an invalid char is found.
 *                 *dlen is always updated to reflect to amount of
 *                 data that was written (or would have been written)
 *
 * \note           Call this function with *dlen = 0 to obtain the
 *                 required buffer size in *dlen
 */
int base64_decode( unsigned char *dst, int *dlen,
                   unsigned char *src, int  slen );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int base64_self_test( void );

#ifdef __cplusplus
}
#endif

#endif /* base64.h */
