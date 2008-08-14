/**
 * \file bignum.h
 */
#ifndef _BIGNUM_H
#define _BIGNUM_H

#ifdef __cplusplus
extern "C" {
#endif

#define ERR_MPI_INVALID_CHARACTER               0x0002
#define ERR_MPI_INVALID_PARAMETER               0x0004
#define ERR_MPI_BUFFER_TOO_SMALL                0x0006
#define ERR_MPI_NEGATIVE_VALUE                  0x0008
#define ERR_MPI_DIVISION_BY_ZERO                0x000A
#define ERR_MPI_NOT_INVERTIBLE                  0x000C
#define ERR_MPI_IS_COMPOSITE                    0x000E

#define CHK(fc) if( ( ret = fc ) != 0 ) goto cleanup

/*
 * Define the base limb type
 */
#if defined(_MSC_VER) && _MSC_VER <= 800
/* 16-bit 8086 */
typedef unsigned int  t_int;
typedef unsigned long t_dbl;
#else
  typedef unsigned long t_int;
  #if defined(_MSC_VER) && defined(_M_IX86)
    typedef __int64 t_dbl;
  #else
    #if defined(__x86_64__)
      typedef unsigned int t_dbl __attribute__((mode(TI)));
    #else
      typedef unsigned long long t_dbl;
    #endif
  #endif
#endif

#include <stdio.h>

/**
 * \brief          MPI structure
 */
typedef struct
{
    int s;              /*!<  integer sign      */
    int n;              /*!<  total # of limbs  */
    t_int *p;           /*!<  pointer to limbs  */
}
mpi;

/**
 * \brief          Initialize one or more mpi
 */
void mpi_init( mpi *X, ... );

/**
 * \brief          Unallocate one or more mpi
 */
void mpi_free( mpi *X, ... );

/**
 * \brief          Enlarge X to the specified # of limbs
 *
 * \return         0 if successful,
 *                 1 if memory allocation failed
 */
int mpi_grow( mpi *X, int nblimbs );

/**
 * \brief          Copy the contents of Y into X
 *
 * \return         0 if successful,
 *                 1 if memory allocation failed
 */
int mpi_copy( mpi *X, mpi *Y );

/**
 * \brief          Swap the contents of X and Y
 */
void mpi_swap( mpi *X, mpi *Y );

/**
 * \brief          Set value from integer
 *
 * \return         0 if successful,
 *                 1 if memory allocation failed
 */
int mpi_lset( mpi *X, int z );

/**
 * \brief          Set value from string
 *
 * \param X        destination mpi
 * \param s        string to read the value from
 * \param radix    numeric base of "s"
 *
 * \return         0 if successful,
 *                 1 if memory allocation failed,
 *                 ERR_MPI_INVALID_PARAMETER if the radix is invalid
 *                 ERR_MPI_INVALID_CHARACTER if a non-digit is found
 */
int mpi_read( mpi *X, char *s, int radix );

/**
 * \brief          Print the value of X into fout
 *
 * \param name     string printed before the value
 * \param X        mpi to be printed
 * \param radix    chosen numeric base
 * \param fout     output file stream
 *
 * \return         0 if successful,
 *                 1 if memory allocation failed,
 *                 ERR_MPI_INVALID_PARAMETER if the radix is invalid
 */
int mpi_showf( char *name, mpi *X, int radix, FILE *fout );

/**
 * \brief          Print the value of X on the console
 *
 * \param name     string printed before the value
 * \param X        mpi to be printed
 * \param radix    chosen numeric base
 *
 * \return         0 if successful,
 *                 1 if memory allocation failed,
 *                 ERR_MPI_INVALID_PARAMETER if the radix is invalid
 */
int mpi_show( char *name, mpi *X, int radix );

/**
 * \brief          Import an unsigned value from binary data
 *
 * \param X        destination mpi
 * \param buf      input buffer
 * \param buflen   size of buffer
 *
 * \return         0 if successful,
 *                 1 if memory allocation failed
 */
int mpi_import( mpi *X, unsigned char *buf, int buflen );

/**
 * \brief          Export an unsigned value into binary data
 *
 * \param X        source mpi
 * \param buf      output buffer
 * \param buflen   size of buffer
 *
 * \return         0 if successful,
 *                 ERR_MPI_BUFFER_TOO_SMALL if buf isn't large enough
 *
 * \note           Call this function with *buflen = 0 to obtain the
 *                 required buffer size in *buflen.
 */
int mpi_export( mpi *X, unsigned char *buf, int *buflen );

/**
 * \brief          Return actual size in bits (without leading 0s)
 */
int mpi_size( mpi *X );

/**
 * \brief          Return number of least significant bits
 */
int mpi_lsb( mpi *X );

/**
 * \brief          Left-shift: X <<= count
 *
 * \return         0 if successful,
 *                 1 if memory allocation failed
 */
int mpi_shift_l( mpi *X, int count );

/**
 * \brief          Right-shift: X >>= count
 *
 * \return         0 if successful,
 *                 1 if memory allocation failed
 */
int mpi_shift_r( mpi *X, int count );

/**
 * \brief          Compare unsigned values
 *
 * \return         1 if |X| is greater than |Y|,
 *                -1 if |X| is lesser  than |Y| or
 *                 0 if |X| is equal to |Y|
 */
int mpi_cmp_abs( mpi *X, mpi *Y );

/**
 * \brief          Compare signed values
 *
 * \return         1 if X is greater than Y,
 *                -1 if X is lesser  than Y or
 *                 0 if X is equal to Y
 */
int mpi_cmp_mpi( mpi *X, mpi *Y );

/**
 * \brief          Compare signed values
 *
 * \return         1 if X is greater than z,
 *                -1 if X is lesser  than z or
 *                 0 if X is equal to z
 */
int mpi_cmp_int( mpi *X, int z );

/**
 * \brief          Unsigned addition: X = |A| + |B|
 *
 * \return         0 if successful,
 *                 1 if memory allocation failed
 */
int mpi_add_abs( mpi *X, mpi *A, mpi *B );

/**
 * \brief          Unsigned substraction: X = |A| - |B|
 *
 * \return         0 if successful,
 *                 ERR_MPI_NEGATIVE_VALUE if B is greater than A
 */
int mpi_sub_abs( mpi *X, mpi *A, mpi *B );

/**
 * \brief          Signed addition: X = A + B
 *
 * \return         0 if successful,
 *                 1 if memory allocation failed
 */
int mpi_add_mpi( mpi *X, mpi *A, mpi *B );

/**
 * \brief          Signed substraction: X = A - B
 *
 * \return         0 if successful,
 *                 1 if memory allocation failed
 */
int mpi_sub_mpi( mpi *X, mpi *A, mpi *B );

/**
 * \brief          Signed addition: X = A + b
 *
 * \return         0 if successful,
 *                 1 if memory allocation failed
 */
int mpi_add_int( mpi *X, mpi *A, int b );

/**
 * \brief          Signed substraction: X = A - b
 *
 * \return         0 if successful,
 *                 1 if memory allocation failed
 */
int mpi_sub_int( mpi *X, mpi *A, int b );

/**
 * \brief          Baseline multiplication: X = A * B
 *
 * \return         0 if successful,
 *                 1 if memory allocation failed
 */
int mpi_mul_mpi( mpi *X, mpi *A, mpi *B );

/**
 * \brief          Baseline multiplication: X = A * b
 *
 * \return         0 if successful,
 *                 1 if memory allocation failed
 */
int mpi_mul_int( mpi *X, mpi *A, t_int b );

/**
 * \brief          Division by mpi: A = Q * B + R
 *
 * \return         0 if successful,
 *                 1 if memory allocation failed,
 *                 ERR_MPI_DIVISION_BY_ZERO if B == 0
 *
 * \note           Either Q or R can be NULL.
 */
int mpi_div_mpi( mpi *Q, mpi *R, mpi *A, mpi *B );

/**
 * \brief          Division by int: A = Q * b + R
 *
 * \return         0 if successful,
 *                 1 if memory allocation failed,
 *                 ERR_MPI_DIVISION_BY_ZERO if b == 0
 *
 * \note           Either Q or R can be NULL.
 */
int mpi_div_int( mpi *Q, mpi *R, mpi *A, int b );

/**
 * \brief          Modulo: R = A mod B
 *
 * \return         0 if successful,
 *                 1 if memory allocation failed,
 *                 ERR_MPI_DIVISION_BY_ZERO if B == 0
 */
int mpi_mod_mpi( mpi *R, mpi *A, mpi *B );

/**
 * \brief          Modulo: r = A mod b
 *
 * \return         0 if successful,
 *                 1 if memory allocation failed,
 *                 ERR_MPI_DIVISION_BY_ZERO if b == 0,
 */
int mpi_mod_int( t_int *r, mpi *A, int b );

/**
 * \brief          Sliding-window exponentiation: X = A^E mod N
 *
 * \return         0 if successful,
 *                 1 if memory allocation failed,
 *                 ERR_MPI_INVALID_PARAMETER if N is negative or even
 *
 * \note           _RR is used to avoid re-computing R*R mod N across
 *                 multiple calls, which speeds up things a bit. It can
 *                 be set to NULL if the extra performance is unneeded.
 */
int mpi_exp_mod( mpi *X, mpi *A, mpi *E, mpi *N, mpi *_RR );

/**
 * \brief          Greatest common divisor: G = gcd(A, B)
 *
 * \return         0 if successful,
 *                 1 if memory allocation failed
 */
int mpi_gcd( mpi *G, mpi *A, mpi *B );

/**
 * \brief          Modular inverse: X = A^-1 mod N
 *
 * \return         0 if successful,
 *                 1 if memory allocation failed,
 *                 ERR_MPI_INVALID_PARAMETER if N is negative or nil
 *                 ERR_MPI_NOT_INVERTIBLE if A has no inverse mod N
 */
int mpi_inv_mod( mpi *X, mpi *A, mpi *N );

/**
 * \brief          Miller-Rabin primality test
 *
 * \return         0 if successful (probably prime),
 *                 1 if memory allocation failed,
 *                 ERR_MPI_IS_COMPOSITE if X is not prime
 */
int mpi_is_prime( mpi *X );

/**
 * \brief          Prime number generation
 *
 * \param X        destination mpi
 * \param nbits    required size of X in bits
 * \param dh_flag  set this to 1 if (X-1)/2 must also be prime
 *                 (needed for Diffie-Hellman)
 * \param rng_f    points to the RNG function
 * \param rng_d    points to the RNG data 
 *
 * \return         0 if successful (probably prime),
 *                 1 if memory allocation failed,
 *                 ERR_MPI_INVALID_PARAMETER if nbits is < 3
 */
int mpi_gen_prime( mpi *X, int nbits, int dh_flag,
                   int (*rng_f)(void *), void *rng_d );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int mpi_self_test( void );

#ifdef __cplusplus
}
#endif

#endif /* bignum.h */
