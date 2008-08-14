/**
 * \file dhm.h
 */
#ifndef _DHM_H
#define _DHM_H

#ifdef __cplusplus
extern "C" {
#endif

#define ERR_DHM_READ_PARAMS_FAILED              0x0380
#define ERR_DHM_MAKE_PARAMS_FAILED              0x0390
#define ERR_DHM_READ_PUBLIC_FAILED              0x03A0
#define ERR_DHM_MAKE_PUBLIC_FAILED              0x03B0
#define ERR_DHM_CALC_SECRET_FAILED              0x03C0

#include "bignum.h"

typedef struct
{
    int len;    /*!<  size(P) in chars  */
    mpi P;      /*!<  prime modulus     */
    mpi G;      /*!<  generator         */
    mpi X;      /*!<  secret value      */
    mpi GX;     /*!<  self = G^X mod P  */
    mpi GY;     /*!<  peer = G^Y mod P  */
    mpi K;      /*!<  key = GY^X mod P  */
    mpi RP;     /*!<  recalc R*R mod P  */
}
dhm_context;

/**
 * \brief          Parse the ServerKeyExchange parameters
 *
 * \param ctx      DHM context
 * \param p        &(start of input buffer)
 * \param end      end of buffer
 *
 * \return         0 if successful, or ERR_DHM_READ_PARAMS_FAILED
 */
int dhm_ssl_read_params( dhm_context *ctx,
                         unsigned char **p,
                         unsigned char *end );

/**
 * \brief          Setup and write the ServerKeyExchange parameters
 *
 * \param ctx      DHM context
 * \param P        public modulus   (hex string)
 * \param G        public generator (hex string)
 * \param rng_f    points to the RNG function
 * \param rng_d    points to the RNG data 
 * \param output   destination buffer
 * \param olen     number of chars written
 *
 * \return         0 if successful, or an MPI error code
 */
int dhm_ssl_make_params( dhm_context *ctx, char *P, char *G,
                         int (*rng_f)(void *), void *rng_d,
                         unsigned char *output, int *olen );
                         
/**
 * \brief          Setup and write the ServerKeyExchange parameters
 *
 * \param ctx      DHM context
 * \param P        public modulus   (hex string)
 * \param G        public generator (hex string)
 * \param rng_f    points to the RNG function
 * \param rng_d    points to the RNG data
 *
 * \return         0 if successful, or an MPI error code
 */
int dhm_make_params( dhm_context *ctx, char *P, char *G,
                         int (*rng_f)(void *), void *rng_d);


/**
 * \brief          Import the peer's public value (G^Y)
 *
 * \param ctx      DHM context
 * \param input    input buffer
 * \param ilen     size of buffer
 *
 * \return         0 if successful, or ERR_DHM_READ_PUBLIC_FAILED
 */
int dhm_read_public( dhm_context *ctx,
                     unsigned char *input, int ilen );

/**
 * \brief          Create private value X and export G^X
 *
 * \param ctx      DHM context
 * \param output   destination buffer
 * \param olen     must be == ctx->P.len
 * \param rng_f    points to the RNG function
 * \param rng_d    points to the RNG data 
 *
 * \return         0 if successful, or ERR_DHM_MAKE_PUBLIC_FAILED
 */
int dhm_make_public( dhm_context *ctx,
                     unsigned char *output, int olen,
                     int (*rng_f)(void *), void *rng_d );

/**
 * \brief          Derive and export the shared secret (G^Y)^X mod P
 *
 * \param ctx      DHM context
 * \param output   destination buffer
 * \param olen     number of chars written
 *
 * \return         0 if successful, or ERR_DHM_MAKE_PUBLIC_FAILED
 */
int dhm_calc_secret( dhm_context *ctx,
                     unsigned char *output, int *olen );

/*
 * \brief          Free the components of a DHM key
 */
void dhm_free( dhm_context *ctx );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int dhm_self_test( void );

#ifdef __cplusplus
}
#endif

#endif
