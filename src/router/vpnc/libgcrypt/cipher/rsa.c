/* rsa.c  -  RSA function
 *	Copyright (C) 1997, 1998, 1999 by Werner Koch (dd9jn)
 *	Copyright (C) 2000, 2001, 2002, 2003 Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

/* This code uses an algorithm protected by U.S. Patent #4,405,829
   which expired on September 20, 2000.  The patent holder placed that
   patent into the public domain on Sep 6th, 2000.
*/

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "g10lib.h"
#include "mpi.h"
#include "cipher.h"


typedef struct
{
  gcry_mpi_t n;	    /* modulus */
  gcry_mpi_t e;	    /* exponent */
} RSA_public_key;


typedef struct
{
  gcry_mpi_t n;	    /* public modulus */
  gcry_mpi_t e;	    /* public exponent */
  gcry_mpi_t d;	    /* exponent */
  gcry_mpi_t p;	    /* prime  p. */
  gcry_mpi_t q;	    /* prime  q. */
  gcry_mpi_t u;	    /* inverse of p mod q. */
} RSA_secret_key;


static void test_keys (RSA_secret_key *sk, unsigned nbits);
static void generate (RSA_secret_key *sk,
                      unsigned int nbits, unsigned long use_e);
static int  check_secret_key (RSA_secret_key *sk);
static void public (gcry_mpi_t output, gcry_mpi_t input, RSA_public_key *skey);
static void secret (gcry_mpi_t output, gcry_mpi_t input, RSA_secret_key *skey);


static void
test_keys( RSA_secret_key *sk, unsigned nbits )
{
  RSA_public_key pk;
  gcry_mpi_t test = gcry_mpi_new ( nbits );
  gcry_mpi_t out1 = gcry_mpi_new ( nbits );
  gcry_mpi_t out2 = gcry_mpi_new ( nbits );

  pk.n = sk->n;
  pk.e = sk->e;
  gcry_mpi_randomize( test, nbits, GCRY_WEAK_RANDOM );

  public( out1, test, &pk );
  secret( out2, out1, sk );
  if( mpi_cmp( test, out2 ) )
    log_fatal("RSA operation: public, secret failed\n");
  secret( out1, test, sk );
  public( out2, out1, &pk );
  if( mpi_cmp( test, out2 ) )
    log_fatal("RSA operation: secret, public failed\n");
  gcry_mpi_release ( test );
  gcry_mpi_release ( out1 );
  gcry_mpi_release ( out2 );
}


/* Callback used by the prime generation to test whether the exponent
   is suitable. Returns 0 if the test has been passed. */
static int
check_exponent (void *arg, gcry_mpi_t a)
{
  gcry_mpi_t e = arg;
  gcry_mpi_t tmp;
  int result;
  
  mpi_sub_ui (a, a, 1);
  tmp = _gcry_mpi_alloc_like (a);
  result = !gcry_mpi_gcd(tmp, e, a); /* GCD is not 1. */
  gcry_mpi_release (tmp);
  mpi_add_ui (a, a, 1);
  return result;
}

/****************
 * Generate a key pair with a key of size NBITS.  
 * USE_E = 0 let Libcgrypt decide what exponent to use.
 *       = 1 request the use of a "secure" exponent; this is required by some 
 *           specification to be 65537.
 *       > 2 Try starting at this value until a working exponent is found.
 * Returns: 2 structures filled with all needed values
 */
static void
generate (RSA_secret_key *sk, unsigned int nbits, unsigned long use_e)
{
  gcry_mpi_t p, q; /* the two primes */
  gcry_mpi_t d;    /* the private key */
  gcry_mpi_t u;
  gcry_mpi_t t1, t2;
  gcry_mpi_t n;    /* the public key */
  gcry_mpi_t e;    /* the exponent */
  gcry_mpi_t phi;  /* helper: (p-1)(q-1) */
  gcry_mpi_t g;
  gcry_mpi_t f;

  /* make sure that nbits is even so that we generate p, q of equal size */
  if ( (nbits&1) )
    nbits++; 

  if (use_e == 1)   /* Alias for a secure value. */
    use_e = 65537;  /* as demanded by Spinx. */

  /* Public exponent:
     In general we use 41 as this is quite fast and more secure than the
     commonly used 17.  Benchmarking the RSA verify function
     with a 1024 bit key yields (2001-11-08): 
     e=17    0.54 ms
     e=41    0.75 ms
     e=257   0.95 ms
     e=65537 1.80 ms
  */
  e = mpi_alloc( (32+BITS_PER_MPI_LIMB-1)/BITS_PER_MPI_LIMB );
  if (!use_e)
    mpi_set_ui (e, 41);     /* This is a reasonable secure and fast value */
  else 
    {
      use_e |= 1; /* make sure this is odd */
      mpi_set_ui (e, use_e); 
    }
    
  n = gcry_mpi_new (nbits);

  p = q = NULL;
  do
    {
      /* select two (very secret) primes */
      if (p)
        gcry_mpi_release (p);
      if (q)
        gcry_mpi_release (q);
      if (use_e)
        { /* Do an extra test to ensure that the given exponent is
             suitable. */
          p = _gcry_generate_secret_prime (nbits/2, check_exponent, e);
          q = _gcry_generate_secret_prime (nbits/2, check_exponent, e);
        }
      else
        { /* We check the exponent later. */
          p = _gcry_generate_secret_prime (nbits/2, NULL, NULL);
          q = _gcry_generate_secret_prime (nbits/2, NULL, NULL);
        }
      if (mpi_cmp (p, q) > 0 ) /* p shall be smaller than q (for calc of u)*/
        mpi_swap(p,q);
      /* calculate the modulus */
      mpi_mul( n, p, q );
    }
  while ( mpi_get_nbits(n) != nbits );

  /* calculate Euler totient: phi = (p-1)(q-1) */
  t1 = mpi_alloc_secure( mpi_get_nlimbs(p) );
  t2 = mpi_alloc_secure( mpi_get_nlimbs(p) );
  phi = gcry_mpi_snew ( nbits );
  g	= gcry_mpi_snew ( nbits );
  f	= gcry_mpi_snew ( nbits );
  mpi_sub_ui( t1, p, 1 );
  mpi_sub_ui( t2, q, 1 );
  mpi_mul( phi, t1, t2 );
  gcry_mpi_gcd(g, t1, t2);
  mpi_fdiv_q(f, phi, g);

  while (!gcry_mpi_gcd(t1, e, phi)) /* (while gcd is not 1) */
    {
      if (use_e)
        BUG (); /* The prime generator already made sure that we
                   never can get to here. */
      mpi_add_ui (e, e, 2);
    }

  /* calculate the secret key d = e^1 mod phi */
  d = gcry_mpi_snew ( nbits );
  mpi_invm(d, e, f );
  /* calculate the inverse of p and q (used for chinese remainder theorem)*/
  u = gcry_mpi_snew ( nbits );
  mpi_invm(u, p, q );

  if( DBG_CIPHER )
    {
      log_mpidump("  p= ", p );
      log_mpidump("  q= ", q );
      log_mpidump("phi= ", phi );
      log_mpidump("  g= ", g );
      log_mpidump("  f= ", f );
      log_mpidump("  n= ", n );
      log_mpidump("  e= ", e );
      log_mpidump("  d= ", d );
      log_mpidump("  u= ", u );
    }

  gcry_mpi_release (t1);
  gcry_mpi_release (t2);
  gcry_mpi_release (phi);
  gcry_mpi_release (f);
  gcry_mpi_release (g);

  sk->n = n;
  sk->e = e;
  sk->p = p;
  sk->q = q;
  sk->d = d;
  sk->u = u;

  /* now we can test our keys (this should never fail!) */
  test_keys( sk, nbits - 64 );
}


/****************
 * Test wether the secret key is valid.
 * Returns: true if this is a valid key.
 */
static int
check_secret_key( RSA_secret_key *sk )
{
  int rc;
  gcry_mpi_t temp = mpi_alloc( mpi_get_nlimbs(sk->p)*2 );
  
  mpi_mul(temp, sk->p, sk->q );
  rc = mpi_cmp( temp, sk->n );
  mpi_free(temp);
  return !rc;
}



/****************
 * Public key operation. Encrypt INPUT with PKEY and put result into OUTPUT.
 *
 *	c = m^e mod n
 *
 * Where c is OUTPUT, m is INPUT and e,n are elements of PKEY.
 */
static void
public(gcry_mpi_t output, gcry_mpi_t input, RSA_public_key *pkey )
{
  if( output == input )  /* powm doesn't like output and input the same */
    {
      gcry_mpi_t x = mpi_alloc( mpi_get_nlimbs(input)*2 );
      mpi_powm( x, input, pkey->e, pkey->n );
      mpi_set(output, x);
      mpi_free(x);
    }
  else
    mpi_powm( output, input, pkey->e, pkey->n );
}

#if 0
static void
stronger_key_check ( RSA_secret_key *skey )
{
  gcry_mpi_t t = mpi_alloc_secure ( 0 );
  gcry_mpi_t t1 = mpi_alloc_secure ( 0 );
  gcry_mpi_t t2 = mpi_alloc_secure ( 0 );
  gcry_mpi_t phi = mpi_alloc_secure ( 0 );

  /* check that n == p * q */
  mpi_mul( t, skey->p, skey->q);
  if (mpi_cmp( t, skey->n) )
    log_info ( "RSA Oops: n != p * q\n" );

  /* check that p is less than q */
  if( mpi_cmp( skey->p, skey->q ) > 0 )
    {
      log_info ("RSA Oops: p >= q - fixed\n");
      _gcry_mpi_swap ( skey->p, skey->q);
    }

    /* check that e divides neither p-1 nor q-1 */
    mpi_sub_ui(t, skey->p, 1 );
    mpi_fdiv_r(t, t, skey->e );
    if ( !mpi_cmp_ui( t, 0) )
        log_info ( "RSA Oops: e divides p-1\n" );
    mpi_sub_ui(t, skey->q, 1 );
    mpi_fdiv_r(t, t, skey->e );
    if ( !mpi_cmp_ui( t, 0) )
        log_info ( "RSA Oops: e divides q-1\n" );

    /* check that d is correct */
    mpi_sub_ui( t1, skey->p, 1 );
    mpi_sub_ui( t2, skey->q, 1 );
    mpi_mul( phi, t1, t2 );
    gcry_mpi_gcd(t, t1, t2);
    mpi_fdiv_q(t, phi, t);
    mpi_invm(t, skey->e, t );
    if ( mpi_cmp(t, skey->d ) )
      {
        log_info ( "RSA Oops: d is wrong - fixed\n");
        mpi_set (skey->d, t);
        _gcry_log_mpidump ("  fixed d", skey->d);
      }

    /* check for correctness of u */
    mpi_invm(t, skey->p, skey->q );
    if ( mpi_cmp(t, skey->u ) )
      {
        log_info ( "RSA Oops: u is wrong - fixed\n");
        mpi_set (skey->u, t);
        _gcry_log_mpidump ("  fixed u", skey->u);
      }

    log_info ( "RSA secret key check finished\n");

    mpi_free (t);
    mpi_free (t1);
    mpi_free (t2);
    mpi_free (phi);
}
#endif



/****************
 * Secret key operation. Encrypt INPUT with SKEY and put result into OUTPUT.
 *
 *	m = c^d mod n
 *
 * Or faster:
 *
 *      m1 = c ^ (d mod (p-1)) mod p 
 *      m2 = c ^ (d mod (q-1)) mod q 
 *      h = u * (m2 - m1) mod q 
 *      m = m1 + h * p
 *
 * Where m is OUTPUT, c is INPUT and d,n,p,q,u are elements of SKEY.
 */
static void
secret(gcry_mpi_t output, gcry_mpi_t input, RSA_secret_key *skey )
{
  if (!skey->p && !skey->q && !skey->u)
    {
      mpi_powm (output, input, skey->d, skey->n);
    }
  else
    {
      gcry_mpi_t m1 = mpi_alloc_secure( mpi_get_nlimbs(skey->n)+1 );
      gcry_mpi_t m2 = mpi_alloc_secure( mpi_get_nlimbs(skey->n)+1 );
      gcry_mpi_t h  = mpi_alloc_secure( mpi_get_nlimbs(skey->n)+1 );
      
      /* m1 = c ^ (d mod (p-1)) mod p */
      mpi_sub_ui( h, skey->p, 1  );
      mpi_fdiv_r( h, skey->d, h );   
      mpi_powm( m1, input, h, skey->p );
      /* m2 = c ^ (d mod (q-1)) mod q */
      mpi_sub_ui( h, skey->q, 1  );
      mpi_fdiv_r( h, skey->d, h );
      mpi_powm( m2, input, h, skey->q );
      /* h = u * ( m2 - m1 ) mod q */
      mpi_sub( h, m2, m1 );
      if ( mpi_is_neg( h ) ) 
        mpi_add ( h, h, skey->q );
      mpi_mulm( h, skey->u, h, skey->q ); 
      /* m = m2 + h * p */
      mpi_mul ( h, h, skey->p );
      mpi_add ( output, m1, h );
    
      mpi_free ( h );
      mpi_free ( m1 );
      mpi_free ( m2 );
    }
}



/* Perform RSA blinding.  */
static gcry_mpi_t
rsa_blind (gcry_mpi_t x, gcry_mpi_t r, gcry_mpi_t e, gcry_mpi_t n)
{
  /* A helper.  */
  gcry_mpi_t a;

  /* Result.  */
  gcry_mpi_t y;

  a = gcry_mpi_snew (gcry_mpi_get_nbits (n));
  y = gcry_mpi_snew (gcry_mpi_get_nbits (n));
  
  /* Now we calculate: y = (x * r^e) mod n, where r is the random
     number, e is the public exponent, x is the non-blinded data and n
     is the RSA modulus.  */
  gcry_mpi_powm (a, r, e, n);
  gcry_mpi_mulm (y, a, x, n);

  gcry_mpi_release (a);

  return y;
}

/* Undo RSA blinding.  */
static gcry_mpi_t
rsa_unblind (gcry_mpi_t x, gcry_mpi_t ri, gcry_mpi_t n)
{
  gcry_mpi_t y;

  y = gcry_mpi_snew (gcry_mpi_get_nbits (n));

  /* Here we calculate: y = (x * r^-1) mod n, where x is the blinded
     decrypted data, ri is the modular multiplicative inverse of r and
     n is the RSA modulus.  */

  gcry_mpi_mulm (y, ri, x, n);

  return y;
}

/*********************************************
 **************  interface  ******************
 *********************************************/

gcry_err_code_t
_gcry_rsa_generate (int algo, unsigned int nbits, unsigned long use_e,
                    gcry_mpi_t *skey, gcry_mpi_t **retfactors)
{
  RSA_secret_key sk;

  generate (&sk, nbits, use_e);
  skey[0] = sk.n;
  skey[1] = sk.e;
  skey[2] = sk.d;
  skey[3] = sk.p;
  skey[4] = sk.q;
  skey[5] = sk.u;
  
  /* make an empty list of factors */
  *retfactors = gcry_xcalloc( 1, sizeof **retfactors );
  
  return GPG_ERR_NO_ERROR;
}


gcry_err_code_t
_gcry_rsa_check_secret_key( int algo, gcry_mpi_t *skey )
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;
  RSA_secret_key sk;

  sk.n = skey[0];
  sk.e = skey[1];
  sk.d = skey[2];
  sk.p = skey[3];
  sk.q = skey[4];
  sk.u = skey[5];

  if (! check_secret_key (&sk))
    err = GPG_ERR_PUBKEY_ALGO;

  return err;
}


gcry_err_code_t
_gcry_rsa_encrypt (int algo, gcry_mpi_t *resarr, gcry_mpi_t data,
                   gcry_mpi_t *pkey, int flags)
{
  RSA_public_key pk;
  
  pk.n = pkey[0];
  pk.e = pkey[1];
  resarr[0] = mpi_alloc (mpi_get_nlimbs (pk.n));
  public (resarr[0], data, &pk);
  
  return GPG_ERR_NO_ERROR;
}

gcry_err_code_t
_gcry_rsa_decrypt (int algo, gcry_mpi_t *result, gcry_mpi_t *data,
                   gcry_mpi_t *skey, int flags)
{
  RSA_secret_key sk;
  gcry_mpi_t r = MPI_NULL;	/* Random number needed for blinding.  */
  gcry_mpi_t ri = MPI_NULL;	/* Modular multiplicative inverse of
				   r.  */
  gcry_mpi_t x = MPI_NULL;	/* Data to decrypt.  */
  gcry_mpi_t y;			/* Result.  */

  /* Extract private key.  */
  sk.n = skey[0];
  sk.e = skey[1];
  sk.d = skey[2];
  sk.p = skey[3];
  sk.q = skey[4];
  sk.u = skey[5];

  y = gcry_mpi_snew (gcry_mpi_get_nbits (sk.n));

  if (! (flags & PUBKEY_FLAG_NO_BLINDING))
    {
      /* Initialize blinding.  */
      
      /* First, we need a random number r between 0 and n - 1, which
	 is relatively prime to n (i.e. it is neither p nor q).  */
      r = gcry_mpi_snew (gcry_mpi_get_nbits (sk.n));
      ri = gcry_mpi_snew (gcry_mpi_get_nbits (sk.n));
      
      gcry_mpi_randomize (r, gcry_mpi_get_nbits (sk.n),
			  GCRY_STRONG_RANDOM);
      gcry_mpi_mod (r, r, sk.n);

      /* Actually it should be okay to skip the check for equality
	 with either p or q here.  */

      /* Calculate inverse of r.  */
      if (! gcry_mpi_invm (ri, r, sk.n))
	BUG ();
    }

  if (! (flags & PUBKEY_FLAG_NO_BLINDING))
    x = rsa_blind (data[0], r, sk.e, sk.n);
  else
    x = data[0];

  /* Do the encryption.  */
  secret (y, x, &sk);

  if (! (flags & PUBKEY_FLAG_NO_BLINDING))
    {
      /* Undo blinding.  */
      gcry_mpi_t a = gcry_mpi_copy (y);
      
      gcry_mpi_release (y);
      y = rsa_unblind (a, ri, sk.n);

      gcry_mpi_release (a);
    }

  if (! (flags & PUBKEY_FLAG_NO_BLINDING))
    {
      /* Deallocate resources needed for blinding.  */
      gcry_mpi_release (x);
      gcry_mpi_release (r);
      gcry_mpi_release (ri);
    }

  /* Copy out result.  */
  *result = y;
  
  return GPG_ERR_NO_ERROR;
}

gcry_err_code_t
_gcry_rsa_sign (int algo, gcry_mpi_t *resarr, gcry_mpi_t data, gcry_mpi_t *skey)
{
  RSA_secret_key sk;
  
  sk.n = skey[0];
  sk.e = skey[1];
  sk.d = skey[2];
  sk.p = skey[3];
  sk.q = skey[4];
  sk.u = skey[5];
  resarr[0] = mpi_alloc( mpi_get_nlimbs (sk.n));
  secret (resarr[0], data, &sk);

  return GPG_ERR_NO_ERROR;
}

gcry_err_code_t
_gcry_rsa_verify (int algo, gcry_mpi_t hash, gcry_mpi_t *data, gcry_mpi_t *pkey,
		  int (*cmp) (void *opaque, gcry_mpi_t tmp),
		  void *opaquev)
{
  RSA_public_key pk;
  gcry_mpi_t result;
  gcry_err_code_t rc;

  pk.n = pkey[0];
  pk.e = pkey[1];
  result = gcry_mpi_new ( 160 );
  public( result, data[0], &pk );
  /*rc = (*cmp)( opaquev, result );*/
  rc = mpi_cmp (result, hash) ? GPG_ERR_BAD_SIGNATURE : GPG_ERR_NO_ERROR;
  gcry_mpi_release (result);
  
  return rc;
}


unsigned int
_gcry_rsa_get_nbits (int algo, gcry_mpi_t *pkey)
{
  return mpi_get_nbits (pkey[0]);
}

static char *rsa_names[] =
  {
    "rsa",
    "openpgp-rsa",
    "oid.1.2.840.113549.1.1.1",
    NULL,
  };

gcry_pk_spec_t _gcry_pubkey_spec_rsa =
  {
    "RSA", rsa_names,
    "ne", "nedpqu", "a", "s", "n",
    GCRY_PK_USAGE_SIGN | GCRY_PK_USAGE_ENCR,
    _gcry_rsa_generate,
    _gcry_rsa_check_secret_key,
    _gcry_rsa_encrypt,
    _gcry_rsa_decrypt,
    _gcry_rsa_sign,
    _gcry_rsa_verify,
    _gcry_rsa_get_nbits,
  };
