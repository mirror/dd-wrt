/* dsa.c  -  DSA signature scheme
 * Copyright (C) 1998, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser general Public License as
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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "g10lib.h"
#include "mpi.h"
#include "cipher.h"

typedef struct
{
  gcry_mpi_t p;	    /* prime */
  gcry_mpi_t q;	    /* group order */
  gcry_mpi_t g;	    /* group generator */
  gcry_mpi_t y;	    /* g^x mod p */
} DSA_public_key;


typedef struct
{
  gcry_mpi_t p;	    /* prime */
  gcry_mpi_t q;	    /* group order */
  gcry_mpi_t g;	    /* group generator */
  gcry_mpi_t y;	    /* g^x mod p */
  gcry_mpi_t x;	    /* secret exponent */
} DSA_secret_key;


static gcry_mpi_t gen_k (gcry_mpi_t q);
static void test_keys (DSA_secret_key *sk, unsigned qbits);
static int check_secret_key (DSA_secret_key *sk);
static void generate (DSA_secret_key *sk, unsigned nbits,
                      gcry_mpi_t **ret_factors);
static void sign (gcry_mpi_t r, gcry_mpi_t s, gcry_mpi_t input,
                  DSA_secret_key *skey);
static int verify (gcry_mpi_t r, gcry_mpi_t s, gcry_mpi_t input,
                   DSA_public_key *pkey);

static void (*progress_cb) (void *,const char *, int, int, int );
static void *progress_cb_data;


void
_gcry_register_pk_dsa_progress (void (*cb) (void *, const char *,
                                            int, int, int),
				void *cb_data)
{
  progress_cb = cb;
  progress_cb_data = cb_data;
}


static void
progress (int c)
{
  if (progress_cb)
    progress_cb (progress_cb_data, "pk_dsa", c, 0, 0);
}


/*
 * Generate a random secret exponent k less than q.
 */
static gcry_mpi_t
gen_k( gcry_mpi_t q )
{
  gcry_mpi_t k = mpi_alloc_secure( mpi_get_nlimbs(q) );
  unsigned int nbits = mpi_get_nbits(q);
  unsigned int nbytes = (nbits+7)/8;
  unsigned char *rndbuf = NULL;

  if ( DBG_CIPHER )
    log_debug("choosing a random k ");
  for (;;) 
    {
      if( DBG_CIPHER )
        progress('.');

      if ( !rndbuf || nbits < 32 ) 
        {
          gcry_free(rndbuf);
          rndbuf = gcry_random_bytes_secure( (nbits+7)/8, GCRY_STRONG_RANDOM );
	}
      else
        { /* Change only some of the higher bits.  We could improve
	     this by directly requesting more memory at the first call
	     to get_random_bytes() and use this the here maybe it is
	     easier to do this directly in random.c. */
          char *pp = gcry_random_bytes_secure( 4, GCRY_STRONG_RANDOM );
          memcpy( rndbuf,pp, 4 );
          gcry_free(pp);
	}
      _gcry_mpi_set_buffer( k, rndbuf, nbytes, 0 );
      if ( mpi_test_bit( k, nbits-1 ) )
        mpi_set_highbit( k, nbits-1 );
      else
        {
          mpi_set_highbit( k, nbits-1 );
          mpi_clear_bit( k, nbits-1 );
	}

      if( !(mpi_cmp( k, q ) < 0) ) /* check: k < q */
        {	
          if( DBG_CIPHER )
            progress('+');
          continue; /* no  */
        }
      if( !(mpi_cmp_ui( k, 0 ) > 0) )  /* check: k > 0 */
        {
          if( DBG_CIPHER )
            progress('-');
          continue; /* no */
        }
      break;	/* okay */
    }
  gcry_free(rndbuf);
  if( DBG_CIPHER )
    progress('\n');
  
  return k;
}


static void
test_keys( DSA_secret_key *sk, unsigned qbits )
{
  DSA_public_key pk;
  gcry_mpi_t test = gcry_mpi_new ( qbits  );
  gcry_mpi_t out1_a = gcry_mpi_new ( qbits );
  gcry_mpi_t out1_b = gcry_mpi_new ( qbits );

  pk.p = sk->p;
  pk.q = sk->q;
  pk.g = sk->g;
  pk.y = sk->y;
  gcry_mpi_randomize( test, qbits, GCRY_WEAK_RANDOM );

  sign( out1_a, out1_b, test, sk );
  if( !verify( out1_a, out1_b, test, &pk ) )
    log_fatal("DSA:: sign, verify failed\n");

  gcry_mpi_release ( test );
  gcry_mpi_release ( out1_a );
  gcry_mpi_release ( out1_b );
}



/*
   Generate a DSA key pair with a key of size NBITS.
   Returns: 2 structures filled with all needed values
 	    and an array with the n-1 factors of (p-1)
 */
static void
generate( DSA_secret_key *sk, unsigned nbits, gcry_mpi_t **ret_factors )
{
  gcry_mpi_t p;    /* the prime */
  gcry_mpi_t q;    /* the 160 bit prime factor */
  gcry_mpi_t g;    /* the generator */
  gcry_mpi_t y;    /* g^x mod p */
  gcry_mpi_t x;    /* the secret exponent */
  gcry_mpi_t h, e;  /* helper */
  unsigned qbits;
  byte *rndbuf;

  assert( nbits >= 512 && nbits <= 1024 );

  qbits = 160;
  p = _gcry_generate_elg_prime( 1, nbits, qbits, NULL, ret_factors );
  /* get q out of factors */
  q = mpi_copy((*ret_factors)[0]);
  if( mpi_get_nbits(q) != qbits )
    BUG();

  /* Find a generator g (h and e are helpers).
     e = (p-1)/q */
  e = mpi_alloc( mpi_get_nlimbs(p) );
  mpi_sub_ui( e, p, 1 );
  mpi_fdiv_q( e, e, q );
  g = mpi_alloc( mpi_get_nlimbs(p) );
  h = mpi_alloc_set_ui( 1 ); /* we start with 2 */
  do
    {
      mpi_add_ui( h, h, 1 );
      /* g = h^e mod p */
      gcry_mpi_powm( g, h, e, p );
    } 
  while( !mpi_cmp_ui( g, 1 ) );  /* continue until g != 1 */

  /* Select a random number which has these properties:
   *	 0 < x < q-1
   * This must be a very good random number because this
   * is the secret part. */
  if( DBG_CIPHER )
    log_debug("choosing a random x ");
  assert( qbits >= 160 );
  x = mpi_alloc_secure( mpi_get_nlimbs(q) );
  mpi_sub_ui( h, q, 1 );  /* put q-1 into h */
  rndbuf = NULL;
  do 
    {
      if( DBG_CIPHER )
        progress('.');
      if( !rndbuf )
        rndbuf = gcry_random_bytes_secure( (qbits+7)/8,
                                           GCRY_VERY_STRONG_RANDOM );
      else 
        { /* Change only some of the higher bits (= 2 bytes)*/
          char *r = gcry_random_bytes_secure (2, GCRY_VERY_STRONG_RANDOM);
          memcpy(rndbuf, r, 2 );
          gcry_free(r);
        }

      _gcry_mpi_set_buffer( x, rndbuf, (qbits+7)/8, 0 );
      mpi_clear_highbit( x, qbits+1 );
    } 
  while ( !( mpi_cmp_ui( x, 0 )>0 && mpi_cmp( x, h )<0 ) );
  gcry_free(rndbuf);
  mpi_free( e );
  mpi_free( h );

  /* y = g^x mod p */
  y = mpi_alloc( mpi_get_nlimbs(p) );
  gcry_mpi_powm( y, g, x, p );

  if( DBG_CIPHER ) 
    {
      progress('\n');
      log_mpidump("dsa  p= ", p );
      log_mpidump("dsa  q= ", q );
      log_mpidump("dsa  g= ", g );
      log_mpidump("dsa  y= ", y );
      log_mpidump("dsa  x= ", x );
    }

  /* Copy the stuff to the key structures. */
  sk->p = p;
  sk->q = q;
  sk->g = g;
  sk->y = y;
  sk->x = x;

  /* Now we can test our keys (this should never fail!). */
  test_keys( sk, qbits );
}



/*
   Test whether the secret key is valid.
   Returns: if this is a valid key.
 */
static int
check_secret_key( DSA_secret_key *sk )
{
  int rc;
  gcry_mpi_t y = mpi_alloc( mpi_get_nlimbs(sk->y) );

  gcry_mpi_powm( y, sk->g, sk->x, sk->p );
  rc = !mpi_cmp( y, sk->y );
  mpi_free( y );
  return rc;
}



/*
   Make a DSA signature from HASH and put it into r and s.
 */
static void
sign(gcry_mpi_t r, gcry_mpi_t s, gcry_mpi_t hash, DSA_secret_key *skey )
{
  gcry_mpi_t k;
  gcry_mpi_t kinv;
  gcry_mpi_t tmp;

  /* Select a random k with 0 < k < q */
  k = gen_k( skey->q );

  /* r = (a^k mod p) mod q */
  gcry_mpi_powm( r, skey->g, k, skey->p );
  mpi_fdiv_r( r, r, skey->q );

  /* kinv = k^(-1) mod q */
  kinv = mpi_alloc( mpi_get_nlimbs(k) );
  mpi_invm(kinv, k, skey->q );

  /* s = (kinv * ( hash + x * r)) mod q */
  tmp = mpi_alloc( mpi_get_nlimbs(skey->p) );
  mpi_mul( tmp, skey->x, r );
  mpi_add( tmp, tmp, hash );
  mpi_mulm( s , kinv, tmp, skey->q );

  mpi_free(k);
  mpi_free(kinv);
  mpi_free(tmp);
}


/*
   Returns true if the signature composed from R and S is valid.
 */
static int
verify (gcry_mpi_t r, gcry_mpi_t s, gcry_mpi_t hash, DSA_public_key *pkey )
{
  int rc;
  gcry_mpi_t w, u1, u2, v;
  gcry_mpi_t base[3];
  gcry_mpi_t ex[3];

  if( !(mpi_cmp_ui( r, 0 ) > 0 && mpi_cmp( r, pkey->q ) < 0) )
    return 0; /* assertion	0 < r < q  failed */
  if( !(mpi_cmp_ui( s, 0 ) > 0 && mpi_cmp( s, pkey->q ) < 0) )
    return 0; /* assertion	0 < s < q  failed */

  w  = mpi_alloc( mpi_get_nlimbs(pkey->q) );
  u1 = mpi_alloc( mpi_get_nlimbs(pkey->q) );
  u2 = mpi_alloc( mpi_get_nlimbs(pkey->q) );
  v  = mpi_alloc( mpi_get_nlimbs(pkey->p) );

  /* w = s^(-1) mod q */
  mpi_invm( w, s, pkey->q );

  /* u1 = (hash * w) mod q */
  mpi_mulm( u1, hash, w, pkey->q );

  /* u2 = r * w mod q  */
  mpi_mulm( u2, r, w, pkey->q );

  /* v =  g^u1 * y^u2 mod p mod q */
  base[0] = pkey->g; ex[0] = u1;
  base[1] = pkey->y; ex[1] = u2;
  base[2] = NULL;    ex[2] = NULL;
  mpi_mulpowm( v, base, ex, pkey->p );
  mpi_fdiv_r( v, v, pkey->q );

  rc = !mpi_cmp( v, r );

  mpi_free(w);
  mpi_free(u1);
  mpi_free(u2);
  mpi_free(v);

  return rc;
}


/*********************************************
 **************  interface  ******************
 *********************************************/

gcry_err_code_t
_gcry_dsa_generate (int algo, unsigned nbits, unsigned long dummy,
                    gcry_mpi_t *skey, gcry_mpi_t **retfactors)
{
  DSA_secret_key sk;

  generate (&sk, nbits, retfactors);
  skey[0] = sk.p;
  skey[1] = sk.q;
  skey[2] = sk.g;
  skey[3] = sk.y;
  skey[4] = sk.x;

  return GPG_ERR_NO_ERROR;
}


gcry_err_code_t
_gcry_dsa_check_secret_key (int algo, gcry_mpi_t *skey)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;
  DSA_secret_key sk;

  if ((! skey[0]) || (! skey[1]) || (! skey[2]) || (! skey[3]) || (! skey[4]))
    err = GPG_ERR_BAD_MPI;
  else
    {
      sk.p = skey[0];
      sk.q = skey[1];
      sk.g = skey[2];
      sk.y = skey[3];
      sk.x = skey[4];
      if (! check_secret_key (&sk))
	err = GPG_ERR_BAD_SECKEY;
    }

  return err;
}


gcry_err_code_t
_gcry_dsa_sign (int algo, gcry_mpi_t *resarr, gcry_mpi_t data, gcry_mpi_t *skey)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;
  DSA_secret_key sk;

  if ((! data)
      || (! skey[0]) || (! skey[1]) || (! skey[2])
      || (! skey[3]) || (! skey[4]))
    err = GPG_ERR_BAD_MPI;
  else
    {
      sk.p = skey[0];
      sk.q = skey[1];
      sk.g = skey[2];
      sk.y = skey[3];
      sk.x = skey[4];
      resarr[0] = mpi_alloc (mpi_get_nlimbs (sk.p));
      resarr[1] = mpi_alloc (mpi_get_nlimbs (sk.p));
      sign (resarr[0], resarr[1], data, &sk);
    }
  return err;
}

gcry_err_code_t
_gcry_dsa_verify (int algo, gcry_mpi_t hash, gcry_mpi_t *data, gcry_mpi_t *pkey,
		  int (*cmp) (void *, gcry_mpi_t), void *opaquev)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;
  DSA_public_key pk;

  if ((! data[0]) || (! data[1]) || (! hash)
      || (! pkey[0]) || (! pkey[1]) || (! pkey[2]) || (! pkey[3]))
    err = GPG_ERR_BAD_MPI;
  else
    {
      pk.p = pkey[0];
      pk.q = pkey[1];
      pk.g = pkey[2];
      pk.y = pkey[3];
      if (! verify (data[0], data[1], hash, &pk))
	err = GPG_ERR_BAD_SIGNATURE;
    }
  return err;
}


unsigned int
_gcry_dsa_get_nbits (int algo, gcry_mpi_t *pkey)
{
  return mpi_get_nbits (pkey[0]);
}

static char *dsa_names[] =
  {
    "dsa",
    "openpgp-dsa",
    NULL,
  };

gcry_pk_spec_t _gcry_pubkey_spec_dsa =
  {
    "DSA", dsa_names, 
    "pqgy", "pqgyx", "", "rs", "pqgy",
    GCRY_PK_USAGE_SIGN,
    _gcry_dsa_generate,
    _gcry_dsa_check_secret_key,
    NULL,
    NULL,
    _gcry_dsa_sign,
    _gcry_dsa_verify,
    _gcry_dsa_get_nbits,
  };

