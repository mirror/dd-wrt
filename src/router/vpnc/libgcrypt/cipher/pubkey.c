/* pubkey.c  -	pubkey dispatcher
 * Copyright (C) 1998,1999,2000,2002,2003 Free Software Foundation, Inc.
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
#include <errno.h>
#include <assert.h>

#include "g10lib.h"
#include "mpi.h"
#include "cipher.h"
#include "ath.h"

static gcry_err_code_t pubkey_decrypt (int algo, gcry_mpi_t *result,
                                       gcry_mpi_t *data, gcry_mpi_t *skey,
                                       int flags);
static gcry_err_code_t pubkey_sign (int algo, gcry_mpi_t *resarr,
                                    gcry_mpi_t hash, gcry_mpi_t *skey);
static gcry_err_code_t pubkey_verify (int algo, gcry_mpi_t hash,
                                      gcry_mpi_t *data, gcry_mpi_t *pkey,
				     int (*cmp) (void *, gcry_mpi_t),
                                      void *opaque);

/* This is the list of the default public-key ciphers included in
   libgcrypt.  */
static struct pubkey_table_entry
{
  gcry_pk_spec_t *pubkey;
  unsigned int algorithm;
} pubkey_table[] =
  {
#if USE_RSA
    { &_gcry_pubkey_spec_rsa, GCRY_PK_RSA },
#endif
#if USE_ELGAMAL
    { &_gcry_pubkey_spec_elg, GCRY_PK_ELG },
    { &_gcry_pubkey_spec_elg, GCRY_PK_ELG_E },
#endif
#if USE_DSA
    { &_gcry_pubkey_spec_dsa, GCRY_PK_DSA },
#endif
    { NULL, 0 },
  };

/* List of registered ciphers.  */
static gcry_module_t pubkeys_registered;

/* This is the lock protecting PUBKEYS_REGISTERED.  */
static ath_mutex_t pubkeys_registered_lock;

/* Flag to check wether the default pubkeys have already been
   registered.  */
static int default_pubkeys_registered;

/* Convenient macro for registering the default digests.  */
#define REGISTER_DEFAULT_PUBKEYS                   \
  do                                               \
    {                                              \
      ath_mutex_lock (&pubkeys_registered_lock);   \
      if (! default_pubkeys_registered)            \
        {                                          \
          gcry_pk_register_default ();         \
          default_pubkeys_registered = 1;          \
        }                                          \
      ath_mutex_unlock (&pubkeys_registered_lock); \
    }                                              \
  while (0)

/* These dummy functions are used in case a cipher implementation
   refuses to provide it's own functions.  */

static gcry_err_code_t
dummy_generate (int algorithm, unsigned int nbits, unsigned long dummy,
                gcry_mpi_t *skey, gcry_mpi_t **retfactors)
{
  log_bug ("no generate() for %d\n", algorithm);
  return GPG_ERR_PUBKEY_ALGO;
}

static gcry_err_code_t
dummy_check_secret_key (int algorithm, gcry_mpi_t *skey)
{
  log_bug ("no check_secret_key() for %d\n", algorithm);
  return GPG_ERR_PUBKEY_ALGO;
}

static gcry_err_code_t
dummy_encrypt (int algorithm, gcry_mpi_t *resarr, gcry_mpi_t data,
               gcry_mpi_t *pkey, int flags)
{
  log_bug ("no encrypt() for %d\n", algorithm);
  return GPG_ERR_PUBKEY_ALGO;
}

static gcry_err_code_t
dummy_decrypt (int algorithm, gcry_mpi_t *result, gcry_mpi_t *data,
               gcry_mpi_t *skey, int flags)
{
  log_bug ("no decrypt() for %d\n", algorithm);
  return GPG_ERR_PUBKEY_ALGO;
}

static gcry_err_code_t
dummy_sign (int algorithm, gcry_mpi_t *resarr, gcry_mpi_t data,
            gcry_mpi_t *skey)
{
  log_bug ("no sign() for %d\n", algorithm);
  return GPG_ERR_PUBKEY_ALGO;
}

static gcry_err_code_t
dummy_verify (int algorithm, gcry_mpi_t hash, gcry_mpi_t *data,
              gcry_mpi_t *pkey,
	      int (*cmp) (void *, gcry_mpi_t), void *opaquev)
{
  log_bug ("no verify() for %d\n", algorithm);
  return GPG_ERR_PUBKEY_ALGO;
}

static unsigned
dummy_get_nbits (int algorithm, gcry_mpi_t *pkey)
{
  log_bug ("no get_nbits() for %d\n", algorithm);
  return 0;
}

/* Internal function.  Register all the pubkeys included in
   PUBKEY_TABLE.  Returns zero on success or an error code.  */
static void
gcry_pk_register_default (void)
{
  gcry_err_code_t err = 0;
  int i;
  
  for (i = 0; (! err) && pubkey_table[i].pubkey; i++)
    {
#define pubkey_use_dummy(func)                       \
      if (! pubkey_table[i].pubkey->func)            \
	pubkey_table[i].pubkey->func = dummy_##func;

      pubkey_use_dummy (generate);
      pubkey_use_dummy (check_secret_key);
      pubkey_use_dummy (encrypt);
      pubkey_use_dummy (decrypt);
      pubkey_use_dummy (sign);
      pubkey_use_dummy (verify);
      pubkey_use_dummy (get_nbits);
#undef pubkey_use_dummy
      err = _gcry_module_add (&pubkeys_registered,
			      pubkey_table[i].algorithm,
			      (void *) pubkey_table[i].pubkey, NULL);
    }

  if (err)
    BUG ();
}

/* Internal callback function.  Used via _gcry_module_lookup.  */
static int
gcry_pk_lookup_func_name (void *spec, void *data)
{
  gcry_pk_spec_t *pubkey = (gcry_pk_spec_t *) spec;
  char *name = (char *) data;
  char **aliases = pubkey->aliases;
  int ret = stricmp (name, pubkey->name);

  while (ret && *aliases)
    ret = stricmp (name, *aliases++);

  return ! ret;
}

/* Internal function.  Lookup a pubkey entry by it's name.  */
static gcry_module_t 
gcry_pk_lookup_name (const char *name)
{
  gcry_module_t pubkey;

  pubkey = _gcry_module_lookup (pubkeys_registered, (void *) name,
				gcry_pk_lookup_func_name);

  return pubkey;
}

/* Register a new pubkey module whose specification can be found in
   PUBKEY.  On success, a new algorithm ID is stored in ALGORITHM_ID
   and a pointer representhing this module is stored in MODULE.  */
gcry_error_t
gcry_pk_register (gcry_pk_spec_t *pubkey,
		  unsigned int *algorithm_id,
		  gcry_module_t *module)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;
  gcry_module_t mod;

  ath_mutex_lock (&pubkeys_registered_lock);
  err = _gcry_module_add (&pubkeys_registered, 0,
			  (void *) pubkey, &mod);
  ath_mutex_unlock (&pubkeys_registered_lock);

  if (! err)
    {
      *module = mod;
      *algorithm_id = mod->mod_id;
    }

  return err;
}

/* Unregister the pubkey identified by ID, which must have been
   registered with gcry_pk_register.  */
void
gcry_pk_unregister (gcry_module_t module)
{
  ath_mutex_lock (&pubkeys_registered_lock);
  _gcry_module_release (module);
  ath_mutex_unlock (&pubkeys_registered_lock);
}

static void
release_mpi_array (gcry_mpi_t *array)
{
  for (; *array; array++)
    {
      mpi_free(*array);
      *array = NULL;
    }
}

/****************
 * Map a string to the pubkey algo
 */
int
gcry_pk_map_name (const char *string)
{
  gcry_module_t pubkey;
  int algorithm = 0;

  if (!string)
    return 0;

  REGISTER_DEFAULT_PUBKEYS;

  ath_mutex_lock (&pubkeys_registered_lock);
  pubkey = gcry_pk_lookup_name (string);
  if (pubkey)
    {
      algorithm = pubkey->mod_id;
      _gcry_module_release (pubkey);
    }
  ath_mutex_unlock (&pubkeys_registered_lock);

  return algorithm;
}


/****************
 * Map a pubkey algo to a string
 */
const char *
gcry_pk_algo_name (int algorithm)
{
  const char *name = NULL;
  gcry_module_t pubkey;

  REGISTER_DEFAULT_PUBKEYS;

  ath_mutex_lock (&pubkeys_registered_lock);
  pubkey = _gcry_module_lookup_id (pubkeys_registered, algorithm);
  if (pubkey)
    {
      name = ((gcry_pk_spec_t *) pubkey->spec)->name;
      _gcry_module_release (pubkey);
    }
  ath_mutex_unlock (&pubkeys_registered_lock);

  return name;
}


/* A special version of gcry_pk_algo name to return the first aliased
   name of the algorithm.  This is required to adhere to the spki
   specs where the algorithm names are lowercase. */
const char *
_gcry_pk_aliased_algo_name (int algorithm)
{
  const char *name = NULL;
  gcry_module_t module;

  REGISTER_DEFAULT_PUBKEYS;

  ath_mutex_lock (&pubkeys_registered_lock);
  module = _gcry_module_lookup_id (pubkeys_registered, algorithm);
  if (module)
    {
      gcry_pk_spec_t *pubkey = (gcry_pk_spec_t *) module->spec;

      name = pubkey->aliases? *pubkey->aliases : NULL;
      if (!name || !*name)
        name = pubkey->name;
      _gcry_module_release (module);
    }
  ath_mutex_unlock (&pubkeys_registered_lock);

  return name;
}


static void
disable_pubkey_algo (int algorithm)
{
  gcry_module_t pubkey;

  ath_mutex_lock (&pubkeys_registered_lock);
  pubkey = _gcry_module_lookup_id (pubkeys_registered, algorithm);
  if (pubkey)
    {
      if (! (pubkey-> flags & FLAG_MODULE_DISABLED))
	pubkey->flags |= FLAG_MODULE_DISABLED;
      _gcry_module_release (pubkey);
    }
  ath_mutex_unlock (&pubkeys_registered_lock);
}


/****************
 * A USE of 0 means: don't care.
 */
static gcry_err_code_t
check_pubkey_algo (int algorithm, unsigned use)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;
  gcry_pk_spec_t *pubkey;
  gcry_module_t module;

  REGISTER_DEFAULT_PUBKEYS;

  ath_mutex_lock (&pubkeys_registered_lock);
  module = _gcry_module_lookup_id (pubkeys_registered, algorithm);
  if (module)
    {
      pubkey = (gcry_pk_spec_t *) module->spec;

      if (((use & GCRY_PK_USAGE_SIGN)
	   && (! (pubkey->use & GCRY_PK_USAGE_SIGN)))
	  || ((use & GCRY_PK_USAGE_ENCR)
	      && (! (pubkey->use & GCRY_PK_USAGE_ENCR))))
	err = GPG_ERR_WRONG_PUBKEY_ALGO;
      else if (module->flags & FLAG_MODULE_DISABLED)
	err = GPG_ERR_PUBKEY_ALGO;
      _gcry_module_release (module);
    }
  else
    err = GPG_ERR_PUBKEY_ALGO;
  ath_mutex_unlock (&pubkeys_registered_lock);

  return err;
}


/****************
 * Return the number of public key material numbers
 */
static int
pubkey_get_npkey (int algorithm)
{
  gcry_module_t pubkey;
  int npkey = 0;

  REGISTER_DEFAULT_PUBKEYS;

  ath_mutex_lock (&pubkeys_registered_lock);
  pubkey = _gcry_module_lookup_id (pubkeys_registered, algorithm);
  if (pubkey)
    {
      npkey = strlen (((gcry_pk_spec_t *) pubkey->spec)->elements_pkey);
      _gcry_module_release (pubkey);
    }
  ath_mutex_unlock (&pubkeys_registered_lock);

  return npkey;
}

/****************
 * Return the number of secret key material numbers
 */
static int
pubkey_get_nskey (int algorithm)
{
  gcry_module_t pubkey;
  int nskey = 0;

  REGISTER_DEFAULT_PUBKEYS;

  ath_mutex_lock (&pubkeys_registered_lock);
  pubkey = _gcry_module_lookup_id (pubkeys_registered, algorithm);
  if (pubkey)
    {
      nskey = strlen (((gcry_pk_spec_t *) pubkey->spec)->elements_skey);
      _gcry_module_release (pubkey);
    }
  ath_mutex_unlock (&pubkeys_registered_lock);

  return nskey;
}

/****************
 * Return the number of signature material numbers
 */
static int
pubkey_get_nsig (int algorithm)
{
  gcry_module_t pubkey;
  int nsig = 0;

  REGISTER_DEFAULT_PUBKEYS;

  ath_mutex_lock (&pubkeys_registered_lock);
  pubkey = _gcry_module_lookup_id (pubkeys_registered, algorithm);
  if (pubkey)
    {
      nsig = strlen (((gcry_pk_spec_t *) pubkey->spec)->elements_sig);
      _gcry_module_release (pubkey);
    }
  ath_mutex_unlock (&pubkeys_registered_lock);

  return nsig;
}

/****************
 * Return the number of encryption material numbers
 */
static int
pubkey_get_nenc (int algorithm)
{
  gcry_module_t pubkey;
  int nenc = 0;

  REGISTER_DEFAULT_PUBKEYS;

  ath_mutex_lock (&pubkeys_registered_lock);
  pubkey = _gcry_module_lookup_id (pubkeys_registered, algorithm);
  if (pubkey)
    {
      nenc = strlen (((gcry_pk_spec_t *) pubkey->spec)->elements_enc);
      _gcry_module_release (pubkey);
    }
  ath_mutex_unlock (&pubkeys_registered_lock);

  return nenc;
}


static gcry_err_code_t
pubkey_generate (int algorithm, unsigned int nbits, unsigned long use_e,
                 gcry_mpi_t *skey, gcry_mpi_t **retfactors)
{
  gcry_err_code_t err = GPG_ERR_PUBKEY_ALGO;
  gcry_module_t pubkey;

  REGISTER_DEFAULT_PUBKEYS;

  ath_mutex_lock (&pubkeys_registered_lock);
  pubkey = _gcry_module_lookup_id (pubkeys_registered, algorithm);
  if (pubkey)
    {
      err = ((gcry_pk_spec_t *) pubkey->spec)->generate 
        (algorithm, nbits, use_e, skey, retfactors);
      _gcry_module_release (pubkey);
    }
  ath_mutex_unlock (&pubkeys_registered_lock);

  return err;
}

static gcry_err_code_t
pubkey_check_secret_key (int algorithm, gcry_mpi_t *skey)
{
  gcry_err_code_t err = GPG_ERR_PUBKEY_ALGO;
  gcry_module_t pubkey;

  REGISTER_DEFAULT_PUBKEYS;

  ath_mutex_lock (&pubkeys_registered_lock);
  pubkey = _gcry_module_lookup_id (pubkeys_registered, algorithm);
  if (pubkey)
    {
      err = ((gcry_pk_spec_t *) pubkey->spec)->check_secret_key
        (algorithm, skey);
      _gcry_module_release (pubkey);
    }
  ath_mutex_unlock (&pubkeys_registered_lock);

  return err;
}


/****************
 * This is the interface to the public key encryption.  Encrypt DATA
 * with PKEY and put it into RESARR which should be an array of MPIs
 * of size PUBKEY_MAX_NENC (or less if the algorithm allows this -
 * check with pubkey_get_nenc() )
 */
static gcry_err_code_t
pubkey_encrypt (int algorithm, gcry_mpi_t *resarr, gcry_mpi_t data,
                gcry_mpi_t *pkey, int flags)
{
  gcry_pk_spec_t *pubkey;
  gcry_module_t module;
  gcry_err_code_t rc;
  int i;

  if (DBG_CIPHER)
    {
      log_debug ("pubkey_encrypt: algo=%d\n", algorithm);
      for(i = 0; i < pubkey_get_npkey (algorithm); i++)
	log_mpidump ("  pkey:", pkey[i]);
      log_mpidump ("  data:", data);
    }

  ath_mutex_lock (&pubkeys_registered_lock);
  module = _gcry_module_lookup_id (pubkeys_registered, algorithm);
  if (module)
    {
      pubkey = (gcry_pk_spec_t *) module->spec;
      rc = pubkey->encrypt (algorithm, resarr, data, pkey, flags);
      _gcry_module_release (module);
      goto ready;
    }
  rc = GPG_ERR_PUBKEY_ALGO;

 ready:
  ath_mutex_unlock (&pubkeys_registered_lock);

  if (!rc && DBG_CIPHER)
    {
      for(i = 0; i < pubkey_get_nenc (algorithm); i++)
	log_mpidump("  encr:", resarr[i] );
    }
  return rc;
}


/****************
 * This is the interface to the public key decryption.
 * ALGO gives the algorithm to use and this implicitly determines
 * the size of the arrays.
 * result is a pointer to a mpi variable which will receive a
 * newly allocated mpi or NULL in case of an error.
 */
static gcry_err_code_t
pubkey_decrypt (int algorithm, gcry_mpi_t *result, gcry_mpi_t *data,
                gcry_mpi_t *skey, int flags)
{
  gcry_pk_spec_t *pubkey;
  gcry_module_t module;
  gcry_err_code_t rc;
  int i;

  *result = NULL; /* so the caller can always do a mpi_free */
  if (DBG_CIPHER)
    {
      log_debug ("pubkey_decrypt: algo=%d\n", algorithm);
      for(i = 0; i < pubkey_get_nskey (algorithm); i++)
	log_mpidump ("  skey:", skey[i]);
      for(i = 0; i < pubkey_get_nenc (algorithm); i++)
	log_mpidump ("  data:", data[i]);
    }

  ath_mutex_lock (&pubkeys_registered_lock);
  module = _gcry_module_lookup_id (pubkeys_registered, algorithm);
  if (module)
    {
      pubkey = (gcry_pk_spec_t *) module->spec;
      rc = pubkey->decrypt (algorithm, result, data, skey, flags);
      _gcry_module_release (module);
      goto ready;
    }

  rc = GPG_ERR_PUBKEY_ALGO;
  
 ready:
  ath_mutex_unlock (&pubkeys_registered_lock);

  if (! rc && DBG_CIPHER)
    log_mpidump (" plain:", *result);

  return rc;
}


/****************
 * This is the interface to the public key signing.
 * Sign data with skey and put the result into resarr which
 * should be an array of MPIs of size PUBKEY_MAX_NSIG (or less if the
 * algorithm allows this - check with pubkey_get_nsig() )
 */
static gcry_err_code_t
pubkey_sign (int algorithm, gcry_mpi_t *resarr, gcry_mpi_t data,
             gcry_mpi_t *skey)
{
  gcry_pk_spec_t *pubkey;
  gcry_module_t module;
  gcry_err_code_t rc;
  int i;

  if (DBG_CIPHER)
    {
      log_debug ("pubkey_sign: algo=%d\n", algorithm);
      for(i = 0; i < pubkey_get_nskey (algorithm); i++)
	log_mpidump ("  skey:", skey[i]);
      log_mpidump("  data:", data );
    }

  ath_mutex_lock (&pubkeys_registered_lock);
  module = _gcry_module_lookup_id (pubkeys_registered, algorithm);
  if (module)
    {
      pubkey = (gcry_pk_spec_t *) module->spec;
      rc = pubkey->sign (algorithm, resarr, data, skey);
      _gcry_module_release (module);
      goto ready;
    }

  rc = GPG_ERR_PUBKEY_ALGO;

 ready:
  ath_mutex_unlock (&pubkeys_registered_lock);

  if (! rc && DBG_CIPHER)
    for (i = 0; i < pubkey_get_nsig (algorithm); i++)
      log_mpidump ("   sig:", resarr[i]);

  return rc;
}

/****************
 * Verify a public key signature.
 * Return 0 if the signature is good
 */
static gcry_err_code_t
pubkey_verify (int algorithm, gcry_mpi_t hash, gcry_mpi_t *data,
               gcry_mpi_t *pkey,
	       int (*cmp)(void *, gcry_mpi_t), void *opaquev)
{
  gcry_pk_spec_t *pubkey;
  gcry_module_t module;
  gcry_err_code_t rc;
  int i;

  if (DBG_CIPHER)
    {
      log_debug ("pubkey_verify: algo=%d\n", algorithm);
      for (i = 0; i < pubkey_get_npkey (algorithm); i++)
	log_mpidump ("  pkey:", pkey[i]);
      for (i = 0; i < pubkey_get_nsig (algorithm); i++)
	log_mpidump ("   sig:", data[i]);
      log_mpidump ("  hash:", hash);
    }

  ath_mutex_lock (&pubkeys_registered_lock);
  module = _gcry_module_lookup_id (pubkeys_registered, algorithm);
  if (module)
    {
      pubkey = (gcry_pk_spec_t *) module->spec;
      rc = pubkey->verify (algorithm, hash, data, pkey, cmp, opaquev);
      _gcry_module_release (module);
      goto ready;
    }

  rc = GPG_ERR_PUBKEY_ALGO;

 ready:
  ath_mutex_unlock (&pubkeys_registered_lock);
  return rc;
}


/* Internal function.   */
static gcry_err_code_t
sexp_elements_extract (gcry_sexp_t key_sexp, const char *element_names,
		       gcry_mpi_t *elements)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;
  int i, idx;
  const char *name;
  gcry_sexp_t list;

  for (name = element_names, idx = 0; *name && !err; name++, idx++)
    {
      list = gcry_sexp_find_token (key_sexp, name, 1);
      if (! list)
	err = GPG_ERR_NO_OBJ;
      else
	{
	  elements[idx] = gcry_sexp_nth_mpi (list, 1, GCRYMPI_FMT_USG);
	  gcry_sexp_release (list);
	  if (! elements[idx])
	    err = GPG_ERR_INV_OBJ;
	}
    }

  if (err)
    {
      for (i = 0; i < idx; i++)
        if (elements[i])
          gcry_free (elements[i]);
    }
  return err;
}

/****************
 * Convert a S-Exp with either a private or a public key to our
 * internal format. Currently we do only support the following
 * algorithms:
 *    dsa
 *    rsa
 *    openpgp-dsa
 *    openpgp-rsa
 *    openpgp-elg
 *    openpgp-elg-sig
 * Provide a SE with the first element be either "private-key" or
 * or "public-key". It is followed by a list with its first element
 * be one of the above algorithm identifiers and the remaning
 * elements are pairs with parameter-id and value.
 * NOTE: we look through the list to find a list beginning with
 * "private-key" or "public-key" - the first one found is used.
 *
 * FIXME: Allow for encrypted secret keys here.
 *
 * Returns: A pointer to an allocated array of MPIs if the return value is
 *	    zero; the caller has to release this array.
 *
 * Example of a DSA public key:
 *  (private-key
 *    (dsa
 *	(p <mpi>)
 *	(g <mpi>)
 *	(y <mpi>)
 *	(x <mpi>)
 *    )
 *  )
 * The <mpi> are expected to be in GCRYMPI_FMT_USG
 */
static gcry_err_code_t
sexp_to_key (gcry_sexp_t sexp, int want_private, gcry_mpi_t **retarray,
             gcry_module_t *retalgo)
{
    gcry_sexp_t list, l2;
    const char *name;
    size_t n;
    const char *elems;
    gcry_mpi_t *array;
    gcry_err_code_t err = GPG_ERR_NO_ERROR;
    gcry_module_t module;
    gcry_pk_spec_t *pubkey;

    /* check that the first element is valid */
    list = gcry_sexp_find_token( sexp, want_private? "private-key"
						    :"public-key", 0 );
    if( !list )
	return GPG_ERR_INV_OBJ; /* Does not contain a public-
                                   or private-key object */
    l2 = gcry_sexp_cadr( list );
    gcry_sexp_release ( list );
    list = l2;
    name = gcry_sexp_nth_data( list, 0, &n );
    if( !name ) {
	gcry_sexp_release ( list );
	return GPG_ERR_INV_OBJ; /* invalid structure of object */
    }

    {
      char *name_terminated = gcry_xmalloc (n + 1);
      memcpy (name_terminated, name, n);
      name_terminated[n] = 0;

      ath_mutex_lock (&pubkeys_registered_lock);
      module = gcry_pk_lookup_name (name_terminated);
      ath_mutex_unlock (&pubkeys_registered_lock);

      gcry_free (name_terminated);
    }

    if (! module)
      {
	gcry_sexp_release (list);
	return GPG_ERR_PUBKEY_ALGO; /* unknown algorithm */
      }
    else
      pubkey = (gcry_pk_spec_t *) module->spec;

    elems = want_private ? pubkey->elements_skey : pubkey->elements_pkey;
    array = gcry_calloc (strlen (elems) + 1, sizeof (*array));
    if (! array)
      err = gpg_err_code_from_errno (errno);
    if (! err)
      err = sexp_elements_extract (list, elems, array);

    if (list)
      gcry_sexp_release (list);

    if (err)
      {
	if (array)
	  gcry_free (array);

	ath_mutex_lock (&pubkeys_registered_lock);
	_gcry_module_release (module);
	ath_mutex_unlock (&pubkeys_registered_lock);
      }
    else
      {
	*retarray = array;
	*retalgo = module;
      }

    return err;
}

static gcry_err_code_t
sexp_to_sig (gcry_sexp_t sexp, gcry_mpi_t **retarray,
	     gcry_module_t *retalgo)
{
    gcry_sexp_t list, l2;
    const char *name;
    size_t n;
    const char *elems;
    gcry_mpi_t *array;
    gcry_err_code_t err = GPG_ERR_NO_ERROR;
    gcry_module_t module;
    gcry_pk_spec_t *pubkey;

    /* check that the first element is valid */
    list = gcry_sexp_find_token( sexp, "sig-val" , 0 );
    if( !list )
	return GPG_ERR_INV_OBJ; /* Does not contain a signature value object */
    l2 = gcry_sexp_nth (list, 1);
    if(! l2)
      {
	gcry_sexp_release (list);
	return GPG_ERR_NO_OBJ; /* no cadr for the sig object */
      }
    name = gcry_sexp_nth_data( l2, 0, &n );
    if( !name ) {
      	gcry_sexp_release ( list );
      	gcry_sexp_release ( l2 );
	return GPG_ERR_INV_OBJ; /* invalid structure of object */
    }
    else if (n == 5 && (! memcmp (name, "flags", 5))) {
      /* Skip flags, since they are not used but just here for the
	 sake of consistent S-expressions.  */
      gcry_sexp_release (l2);
      l2 = gcry_sexp_nth (list, 2);
      if (! l2)
	{
	  gcry_sexp_release (list);
	  return GPG_ERR_INV_OBJ;
	}
      name = gcry_sexp_nth_data (l2, 0, &n);
    }
      
    {
      char *name_terminated = gcry_xmalloc (n + 1);
      memcpy (name_terminated, name, n);
      name_terminated[n] = 0;
      
      ath_mutex_lock (&pubkeys_registered_lock);
      module = gcry_pk_lookup_name (name_terminated);
      ath_mutex_unlock (&pubkeys_registered_lock);

      gcry_free (name_terminated);
    }

    if (! module)
      {
	gcry_sexp_release (l2);
	gcry_sexp_release (list);
	return GPG_ERR_PUBKEY_ALGO; /* unknown algorithm */
      }
    else
      pubkey = (gcry_pk_spec_t *) module->spec;

    elems = pubkey->elements_sig;
    array = gcry_calloc (strlen (elems) + 1 , sizeof (*array));
    if (! array)
      err = gpg_err_code_from_errno (errno);

    if (! err)
      err = sexp_elements_extract (list, elems, array);

    gcry_sexp_release (l2);
    gcry_sexp_release (list);

    if (err)
      {
	ath_mutex_lock (&pubkeys_registered_lock);
	_gcry_module_release (module);
	ath_mutex_unlock (&pubkeys_registered_lock);

	if (array)
	  gcry_free (array);
      }
    else
      {
	*retarray = array;
	*retalgo = module;
      }

    return err;
}


/****************
 * Take sexp and return an array of MPI as used for our internal decrypt
 * function.
 * s_data = (enc-val
 *           [(flags [pkcs1])
 *	      (<algo>
 *		(<param_name1> <mpi>)
 *		...
 *		(<param_namen> <mpi>)
 *	      ))
 * RET_MODERN is set to true when at least an empty flags list has been found.
 */
static gcry_err_code_t
sexp_to_enc (gcry_sexp_t sexp, gcry_mpi_t **retarray, gcry_module_t *retalgo,
             int *ret_modern, int *ret_want_pkcs1, int *flags)
{
  gcry_sexp_t list = NULL, l2 = NULL;
  gcry_pk_spec_t *pubkey = NULL;
  gcry_module_t module = NULL;
  const char *name;
  size_t n;
  int parsed_flags = 0;
  const char *elems;
  gcry_mpi_t *array = NULL;
  gcry_err_code_t err = GPG_ERR_NO_ERROR;

  *ret_want_pkcs1 = 0;
  *ret_modern = 0;

  /* check that the first element is valid */
  list = gcry_sexp_find_token (sexp, "enc-val" , 0);
  if (! list)
    {
      err = GPG_ERR_INV_OBJ; /* Does not contain an encrypted value object */
      goto leave;
    }

  l2 = gcry_sexp_nth (list, 1);
  if (! l2)
    {
      err = GPG_ERR_NO_OBJ; /* no cdr for the data object */
      goto leave;
    }

  /* Extract identifier of sublist.  */
  name = gcry_sexp_nth_data (l2, 0, &n);
  if (! name)
    {
      err = GPG_ERR_INV_OBJ; /* invalid structure of object */
      goto leave;
    }
  
  if ((n == 5) && (! memcmp (name, "flags", 5)))
    {
      /* There is a flags element - process it */
      const char *s;
      int i;
      
      *ret_modern = 1;
      for (i = gcry_sexp_length (l2) - 1; i > 0; i--)
        {
          s = gcry_sexp_nth_data (l2, i, &n);
          if (! s)
            ; /* not a data element - ignore */
          else if (n == 3 && ! memcmp (s, "raw", 3))
            ; /* just a dummy because it is the default */
          else if (n == 5 && ! memcmp (s, "pkcs1", 5))
            *ret_want_pkcs1 = 1;
          else if (n == 11 && ! memcmp (s, "no-blinding", 11))
            parsed_flags |= PUBKEY_FLAG_NO_BLINDING;
          else
            {
              err = GPG_ERR_INV_FLAG;
              goto leave;
            }
        }
      
      /* Get the next which has the actual data */
      gcry_sexp_release (l2);
      l2 = gcry_sexp_nth (list, 2);
      if (! l2)
        {
          err = GPG_ERR_NO_OBJ; /* no cdr for the data object */
          goto leave;
        }

      /* Extract sublist identifier.  */
      name = gcry_sexp_nth_data (l2, 0, &n);
      if (! name)
        {
          err = GPG_ERR_INV_OBJ; /* invalid structure of object */
          goto leave;
        }

      gcry_sexp_release (list);
      list = l2;
      l2 = NULL;
    }

  {
    char *name_terminated = gcry_xmalloc (n + 1);
    memcpy (name_terminated, name, n);
    name_terminated[n] = 0;
    
    ath_mutex_lock (&pubkeys_registered_lock);
    module = gcry_pk_lookup_name (name_terminated);
    ath_mutex_unlock (&pubkeys_registered_lock);
    
    gcry_free (name_terminated);
    
    if (! module)
      {
        err = GPG_ERR_PUBKEY_ALGO; /* unknown algorithm */
        goto leave;
      }
    pubkey = (gcry_pk_spec_t *) module->spec;
  }

  elems = pubkey->elements_enc;
  array = gcry_calloc (strlen (elems) + 1, sizeof (*array));
  if (! array)
    {
      err = gpg_err_code_from_errno (errno);
      goto leave;
    }

  err = sexp_elements_extract (list, elems, array);

 leave:
  if (list)
    gcry_sexp_release (list);
  if (l2)
    gcry_sexp_release (l2);

  if (err)
    {
      ath_mutex_lock (&pubkeys_registered_lock);
      _gcry_module_release (module);
      ath_mutex_unlock (&pubkeys_registered_lock);
      if (array)
	gcry_free (array);
    }
  else
    {
      *retarray = array;
      *retalgo = module;
      *flags = parsed_flags;
    }

  return err;
}

/* Take the hash value and convert into an MPI, suitable for for
   passing to the low level functions.  We currently support the
   old style way of passing just a MPI and the modern interface which
   allows to pass flags so that we can choose between raw and pkcs1
   padding - may be more padding options later. 

   (<mpi>)
   or
   (data
    [(flags [pkcs1])]
    [(hash <algo> <value>)]
    [(value <text>)]
   )
   
   Either the VALUE or the HASH element must be present for use
   with signatures.  VALUE is used for encryption.

   NBITS is the length of the key in bits. 

*/
static gcry_err_code_t
sexp_data_to_mpi (gcry_sexp_t input, unsigned int nbits, gcry_mpi_t *ret_mpi,
                  int for_encryption, int *flags)
{
  gcry_err_code_t rc = 0;
  gcry_sexp_t ldata, lhash, lvalue;
  int i;
  size_t n;
  const char *s;
  int is_raw = 0, is_pkcs1 = 0, unknown_flag=0; 
  int parsed_flags = 0, dummy_flags;

  if (! flags)
    flags = &dummy_flags;
  
  *ret_mpi = NULL;
  ldata = gcry_sexp_find_token (input, "data", 0);
  if (!ldata)
    { /* assume old style */
      *ret_mpi = gcry_sexp_nth_mpi (input, 0, 0);
      return *ret_mpi ? GPG_ERR_NO_ERROR : GPG_ERR_INV_OBJ;
    }

  /* see whether there is a flags object */
  {
    gcry_sexp_t lflags = gcry_sexp_find_token (ldata, "flags", 0);
    if (lflags)
      { /* parse the flags list. */
        for (i=gcry_sexp_length (lflags)-1; i > 0; i--)
          {
            s = gcry_sexp_nth_data (lflags, i, &n);
            if (!s)
              ; /* not a data element*/
            else if ( n == 3 && !memcmp (s, "raw", 3))
              is_raw = 1;
            else if ( n == 5 && !memcmp (s, "pkcs1", 5))
              is_pkcs1 = 1;
	    else if (n == 11 && ! memcmp (s, "no-blinding", 11))
	      parsed_flags |= PUBKEY_FLAG_NO_BLINDING;
            else
              unknown_flag = 1;
          }
        gcry_sexp_release (lflags);
      }
  }

  if (!is_pkcs1 && !is_raw)
    is_raw = 1; /* default to raw */

  /* Get HASH or MPI */
  lhash = gcry_sexp_find_token (ldata, "hash", 0);
  lvalue = lhash? NULL : gcry_sexp_find_token (ldata, "value", 0);

  if (!(!lhash ^ !lvalue))
    rc = GPG_ERR_INV_OBJ; /* none or both given */
  else if (unknown_flag)
    rc = GPG_ERR_INV_FLAG;
  else if (is_raw && is_pkcs1 && !for_encryption)
    rc = GPG_ERR_CONFLICT;
  else if (is_raw && lvalue)
    {
      *ret_mpi = gcry_sexp_nth_mpi (lvalue, 1, 0);
      if (!*ret_mpi)
        rc = GPG_ERR_INV_OBJ;
    }
  else if (is_pkcs1 && lvalue && for_encryption)
    { /* Create pkcs#1 block type 2 padding. */
      unsigned char *frame = NULL;
      size_t nframe = (nbits+7) / 8;
      const void * value;
      size_t valuelen;
      unsigned char *p;

      if ( !(value=gcry_sexp_nth_data (lvalue, 1, &valuelen)) || !valuelen )
        rc = GPG_ERR_INV_OBJ;
      else if (valuelen + 7 > nframe || !nframe)
        {
          /* Can't encode a VALUELEN value in a NFRAME bytes frame. */
          rc = GPG_ERR_TOO_SHORT; /* the key is too short */
        }
      else if ( !(frame = gcry_malloc_secure (nframe)))
        rc = gpg_err_code_from_errno (errno);
      else
        {
          n = 0;
          frame[n++] = 0;
          frame[n++] = 2; /* block type */
          i = nframe - 3 - valuelen;
          assert (i > 0);
          p = gcry_random_bytes_secure (i, GCRY_STRONG_RANDOM);
          /* Replace zero bytes by new values. */
          for (;;)
            {
              int j, k;
              unsigned char *pp;
              
              /* Count the zero bytes. */
              for (j=k=0; j < i; j++)
                {
                  if (!p[j])
                    k++;
                }
              if (!k)
                break; /* Okay: no (more) zero bytes. */
              
              k += k/128 + 3; /* Better get some more. */
              pp = gcry_random_bytes_secure (k, GCRY_STRONG_RANDOM);
              for (j=0; j < i && k;)
                {
                  if (!p[j])
                    p[j] = pp[--k];
                  if (p[j])
                    j++;
                }
              gcry_free (pp);
            }
          memcpy (frame+n, p, i);
          n += i;
          gcry_free (p);
          
          frame[n++] = 0;
          memcpy (frame+n, value, valuelen);
          n += valuelen;
          assert (n == nframe);

	  /* FIXME, error checking?  */
          gcry_mpi_scan (ret_mpi, GCRYMPI_FMT_USG, frame, n, &nframe);
        }

      gcry_free(frame);
    }
  else if (is_pkcs1 && lhash && !for_encryption)
    { /* Create pkcs#1 block type 1 padding. */
      if (gcry_sexp_length (lhash) != 3)
        rc = GPG_ERR_INV_OBJ;
      else if ( !(s=gcry_sexp_nth_data (lhash, 1, &n)) || !n )
        rc = GPG_ERR_INV_OBJ;
      else
        {
          static struct { const char *name; int algo; } hashnames[] = 
          { { "sha1",   GCRY_MD_SHA1 },
            { "md5",    GCRY_MD_MD5 },
            { "sha256", GCRY_MD_SHA256 },
            { "ripemd160", GCRY_MD_RMD160 },
            { "rmd160", GCRY_MD_RMD160 },
            { "sha384", GCRY_MD_SHA384 },
            { "sha512", GCRY_MD_SHA512 },
            { "md2",    GCRY_MD_MD2 },
            { "md4",    GCRY_MD_MD4 },
            { "tiger",  GCRY_MD_TIGER },
            { "haval",  GCRY_MD_HAVAL },
            { NULL }
          };
          int algo;
          byte asn[100];
          byte *frame = NULL;
          size_t nframe = (nbits+7) / 8;
          const void * value;
          size_t valuelen;
          size_t asnlen, dlen;
            
          for (i=0; hashnames[i].name; i++)
            {
              if ( strlen (hashnames[i].name) == n
                   && !memcmp (hashnames[i].name, s, n))
                break;
            }

          algo = hashnames[i].algo;
          asnlen = DIM(asn);
          dlen = gcry_md_get_algo_dlen (algo);

          if (!hashnames[i].name)
            rc = GPG_ERR_DIGEST_ALGO;
          else if ( !(value=gcry_sexp_nth_data (lhash, 2, &valuelen))
                    || !valuelen )
            rc = GPG_ERR_INV_OBJ;
          else if (gcry_md_algo_info (algo, GCRYCTL_GET_ASNOID, asn, &asnlen))
            {
              /* We don't have yet all of the above algorithms.  */
              rc = GPG_ERR_NOT_IMPLEMENTED;
            }
          else if ( valuelen != dlen )
            {
              /* Hash value does not match the length of digest for
                 the given algorithm. */
              rc = GPG_ERR_CONFLICT;
            }
          else if( !dlen || dlen + asnlen + 4 > nframe)
            {
              /* Can't encode an DLEN byte digest MD into a NFRAME
                 byte frame. */
              rc = GPG_ERR_TOO_SHORT;
            }
          else if ( !(frame = gcry_malloc (nframe)) )
            rc = gpg_err_code_from_errno (errno);
          else
            { /* Assemble the pkcs#1 block type 1. */
              n = 0;
              frame[n++] = 0;
              frame[n++] = 1; /* block type */
              i = nframe - valuelen - asnlen - 3 ;
              assert (i > 1);
              memset (frame+n, 0xff, i );
              n += i;
              frame[n++] = 0;
              memcpy (frame+n, asn, asnlen);
              n += asnlen;
              memcpy (frame+n, value, valuelen );
              n += valuelen;
              assert (n == nframe);
      
              /* convert it into an MPI, FIXME: error checking?  */
              gcry_mpi_scan (ret_mpi, GCRYMPI_FMT_USG, frame, n, &nframe);
            }
          
          gcry_free (frame);
        }
    }
  else
    rc = GPG_ERR_CONFLICT;
   
  gcry_sexp_release (ldata);
  gcry_sexp_release (lhash);
  gcry_sexp_release (lvalue);

  if (!rc)
    *flags = parsed_flags;

  return rc;
}


/*
   Do a PK encrypt operation
  
   Caller has to provide a public key as the SEXP pkey and data as a
   SEXP with just one MPI in it. Alternativly S_DATA might be a
   complex S-Expression, similar to the one used for signature
   verification.  This provides a flag which allows to handle PKCS#1
   block type 2 padding.  The function returns a a sexp which may be
   passed to to pk_decrypt.
  
   Returns: 0 or an errorcode.
  
   s_data = See comment for sexp_data_to_mpi
   s_pkey = <key-as-defined-in-sexp_to_key>
   r_ciph = (enc-val
               (<algo>
                 (<param_name1> <mpi>)
                 ...
                 (<param_namen> <mpi>)
               ))

*/
gcry_error_t
gcry_pk_encrypt (gcry_sexp_t *r_ciph, gcry_sexp_t s_data, gcry_sexp_t s_pkey)
{
  gcry_mpi_t *pkey = NULL, data = NULL, *ciph = NULL;
  const char *algo_name, *algo_elems;
  int flags;
  gcry_err_code_t rc;
  gcry_pk_spec_t *pubkey = NULL;
  gcry_module_t module = NULL;

  REGISTER_DEFAULT_PUBKEYS;

  *r_ciph = NULL;
  /* get the key */
  rc = sexp_to_key (s_pkey, 0, &pkey, &module);
  if (rc)
    goto leave;

  assert (module);
  pubkey = (gcry_pk_spec_t *) module->spec;

  /* If aliases for the algorithm name exists, take the first one
     instead of the regular name to adhere to SPKI conventions.  We
     assume that the first alias name is the lowercase version of the
     regular one.  This change is required for compatibility with
     1.1.12 generated S-expressions. */
  algo_name = pubkey->aliases? *pubkey->aliases : NULL;
  if (!algo_name || !*algo_name)
    algo_name = pubkey->name;
  
  algo_elems = pubkey->elements_enc;
  
  /* Get the stuff we want to encrypt. */
  rc = sexp_data_to_mpi (s_data, gcry_pk_get_nbits (s_pkey), &data, 1,
                         &flags);
  if (rc)
    goto leave;

  /* Now we can encrypt DATA to CIPH. */
  ciph = gcry_xcalloc (strlen (algo_elems) + 1, sizeof (*ciph));
  rc = pubkey_encrypt (module->mod_id, ciph, data, pkey, flags);
  mpi_free (data);
  data = NULL;
  if (rc)
    goto leave;

  /* We did it.  Now build the return list */
  {
    char *string, *p;
    int i;
    size_t nelem = strlen (algo_elems);
    size_t needed = 19 + strlen (algo_name) + (nelem * 5);
    void **arg_list;
    
    /* Build the string.  */
    string = p = gcry_xmalloc (needed);
    p = stpcpy ( p, "(enc-val(" );
    p = stpcpy ( p, algo_name );
    for (i=0; algo_elems[i]; i++ )
      {
        *p++ = '(';
        *p++ = algo_elems[i];
        p = stpcpy ( p, "%m)" );
      }
    strcpy ( p, "))" );
    
    /* And now the ugly part: We don't have a function to pass an
     * array to a format string, so we have to do it this way :-(.  */
    /* FIXME: There is now such a format spefier, so we can could
       change the code to be more clear. */
    arg_list = malloc (nelem * sizeof *arg_list);
    if (!arg_list)
      {
        rc = gpg_err_code_from_errno (errno);
        goto leave;
      }

    for (i = 0; i < nelem; i++)
      arg_list[i] = ciph + i;
    
    rc = gcry_sexp_build_array (r_ciph, NULL, string, arg_list);
    free (arg_list);
    if (rc)
      BUG ();
    gcry_free (string);
  }

 leave:
  if (pkey)
    {
      release_mpi_array (pkey);
      gcry_free (pkey);
    }

  if (ciph)
    {
      release_mpi_array (ciph);
      gcry_free (ciph);
    }

  if (module)
    {
      ath_mutex_lock (&pubkeys_registered_lock);
      _gcry_module_release (module);
      ath_mutex_unlock (&pubkeys_registered_lock);
    }

  return gcry_error (rc);
}

/* 
   Do a PK decrypt operation
  
   Caller has to provide a secret key as the SEXP skey and data in a
   format as created by gcry_pk_encrypt.  For historic reasons the
   function returns simply an MPI as an S-expression part; this is
   deprecated and the new method should be used which returns a real
   S-expressionl this is selected by adding at least an empty flags
   list to S_DATA.
   
   Returns: 0 or an errorcode.
  
   s_data = (enc-val
              [(flags)]
              (<algo>
                (<param_name1> <mpi>)
                ...
                (<param_namen> <mpi>)
              ))
   s_skey = <key-as-defined-in-sexp_to_key>
   r_plain= Either an incomplete S-expression without the parentheses
            or if the flags list is used (even if empty) a real S-expression:
            (value PLAIN). 
 */
gcry_error_t
gcry_pk_decrypt (gcry_sexp_t *r_plain, gcry_sexp_t s_data, gcry_sexp_t s_skey)
{
  gcry_mpi_t *skey = NULL, *data = NULL, plain = NULL;
  int modern, want_pkcs1, flags;
  gcry_err_code_t rc;
  gcry_module_t module_enc = NULL, module_key = NULL;
  gcry_pk_spec_t *pubkey = NULL;

  REGISTER_DEFAULT_PUBKEYS;

  *r_plain = NULL;
  rc = sexp_to_key (s_skey, 1, &skey, &module_key);
  if (rc)
    goto leave;

  rc = sexp_to_enc (s_data, &data, &module_enc, &modern, &want_pkcs1, &flags);
  if (rc)
    goto leave;
  
  if (module_key->mod_id != module_enc->mod_id)
    {
      rc = GPG_ERR_CONFLICT; /* Key algo does not match data algo. */
      goto leave;
    }

  pubkey = (gcry_pk_spec_t *) module_key->spec;

  rc = pubkey_decrypt (module_key->mod_id, &plain, data, skey, flags);
  if (rc)
    goto leave;

  if (gcry_sexp_build (r_plain, NULL, modern? "(value %m)" : "%m", plain))
    BUG ();
  
 leave:
  if (skey)
    {
      release_mpi_array (skey);
      gcry_free (skey);
    }

  if (plain)
    mpi_free (plain);

  if (data)
    {
      release_mpi_array (data);
      gcry_free (data);
    }

  if (module_key || module_enc)
    {
      ath_mutex_lock (&pubkeys_registered_lock);
      if (module_key)
	_gcry_module_release (module_key);
      if (module_enc)
	_gcry_module_release (module_enc);
      ath_mutex_unlock (&pubkeys_registered_lock);
    }

  return gcry_error (rc);
}



/*
   Create a signature.
  
   Caller has to provide a secret key as the SEXP skey and data
   expressed as a SEXP list hash with only one element which should
   instantly be available as a MPI. Alternatively the structure given
   below may be used for S_HASH, it provides the abiliy to pass flags
   to the operation; the only flag defined by now is "pkcs1" which
   does PKCS#1 block type 1 style padding.
  
   Returns: 0 or an errorcode.
            In case of 0 the function returns a new SEXP with the
            signature value; the structure of this signature depends on the
            other arguments but is always suitable to be passed to
            gcry_pk_verify
  
   s_hash = See comment for sexp_data_to_mpi
               
   s_skey = <key-as-defined-in-sexp_to_key>
   r_sig  = (sig-val
              (<algo>
                (<param_name1> <mpi>)
                ...
                (<param_namen> <mpi>))) 
*/
gcry_error_t
gcry_pk_sign (gcry_sexp_t *r_sig, gcry_sexp_t s_hash, gcry_sexp_t s_skey)
{
  gcry_mpi_t *skey = NULL, hash = NULL, *result = NULL;
  gcry_pk_spec_t *pubkey = NULL;
  gcry_module_t module = NULL;
  const char *algo_name, *algo_elems;
  int i;
  gcry_err_code_t rc;

  REGISTER_DEFAULT_PUBKEYS;

  *r_sig = NULL;
  rc = sexp_to_key (s_skey, 1, &skey, &module);
  if (rc)
    goto leave;

  assert (module);
  pubkey = (gcry_pk_spec_t *) module->spec;
  algo_name = pubkey->aliases? *pubkey->aliases : NULL;
  if (!algo_name || !*algo_name)
    algo_name = pubkey->name;
  
  algo_elems = pubkey->elements_sig;

  /* Get the stuff we want to sign.  Note that pk_get_nbits does also
      work on a private key. */
  rc = sexp_data_to_mpi (s_hash, gcry_pk_get_nbits (s_skey),
                             &hash, 0, NULL);
  if (rc)
    goto leave;

  result = gcry_xcalloc (strlen (algo_elems) + 1, sizeof (*result));
  rc = pubkey_sign (module->mod_id, result, hash, skey);
  if (rc)
    goto leave;

  {
    char *string, *p;
    size_t nelem, needed = strlen (algo_name) + 20;
    void **arg_list;

    nelem = strlen (algo_elems);
    
    /* Count elements, so that we can allocate enough space. */
    needed += 10 * nelem;

    /* Build the string. */
    string = p = gcry_xmalloc (needed);
    p = stpcpy (p, "(sig-val(");
    p = stpcpy (p, algo_name);
    for (i = 0; algo_elems[i]; i++)
      {
        *p++ = '(';
        *p++ = algo_elems[i];
        p = stpcpy (p, "%m)");
      }
    strcpy (p, "))");

    arg_list = malloc (nelem * sizeof *arg_list);
    if (!arg_list)
      {
        rc = gpg_err_code_from_errno (errno);
        goto leave;
      }

    for (i = 0; i < nelem; i++)
      arg_list[i] = result + i;

    rc = gcry_sexp_build_array (r_sig, NULL, string, arg_list);
    free (arg_list);
    if (rc)
      BUG ();
    gcry_free (string);
  }

 leave:
  if (skey)
    {
      release_mpi_array (skey);
      gcry_free (skey);
    }

  if (hash)
    mpi_free (hash);

  if (result)
    {
      release_mpi_array (result);
      gcry_free (result);
    }

  return gcry_error (rc);
}


/*
   Verify a signature.

   Caller has to supply the public key pkey, the signature sig and his
   hashvalue data.  Public key has to be a standard public key given
   as an S-Exp, sig is a S-Exp as returned from gcry_pk_sign and data
   must be an S-Exp like the one in sign too.  */
gcry_error_t
gcry_pk_verify (gcry_sexp_t s_sig, gcry_sexp_t s_hash, gcry_sexp_t s_pkey)
{
  gcry_module_t module_key = NULL, module_sig = NULL;
  gcry_mpi_t *pkey = NULL, hash = NULL, *sig = NULL;
  gcry_err_code_t rc;

  REGISTER_DEFAULT_PUBKEYS;
 
  rc = sexp_to_key (s_pkey, 0, &pkey, &module_key);
  if (rc)
    goto leave;

  rc = sexp_to_sig (s_sig, &sig, &module_sig);
  if (rc)
    goto leave;

  if (module_key->mod_id != module_sig->mod_id)
    {
      rc = GPG_ERR_CONFLICT;
      goto leave;
    }

  rc = sexp_data_to_mpi (s_hash, gcry_pk_get_nbits (s_pkey), &hash, 0, 0);
  if (rc)
    goto leave;

  rc = pubkey_verify (module_key->mod_id, hash, sig, pkey, NULL, NULL);

 leave:
  if (pkey)
    {
      release_mpi_array (pkey);
      gcry_free (pkey);
    }
  if (sig)
    {
      release_mpi_array (sig);
      gcry_free (sig);
    }
  if (hash)
    mpi_free (hash);

  if (module_key || module_sig)
    {
      ath_mutex_lock (&pubkeys_registered_lock);
      if (module_key)
	_gcry_module_release (module_key);
      if (module_sig)
	_gcry_module_release (module_sig);
      ath_mutex_unlock (&pubkeys_registered_lock);
    }

  return gcry_error (rc);
}


/*
   Test a key.

   This may be used either for a public or a secret key to see whether
   internal structre is valid.
  
   Returns: 0 or an errorcode.
  
   s_key = <key-as-defined-in-sexp_to_key> */
gcry_error_t
gcry_pk_testkey (gcry_sexp_t s_key)
{
  gcry_module_t module = NULL;
  gcry_mpi_t *key = NULL;
  gcry_err_code_t rc;
  
  REGISTER_DEFAULT_PUBKEYS;

  /* Note we currently support only secret key checking. */
  rc = sexp_to_key (s_key, 1, &key, &module);
  if (! rc)
    {
      rc = pubkey_check_secret_key (module->mod_id, key);
      release_mpi_array (key);
      gcry_free (key);
    }
  return gcry_error (rc);
}


/*
  Create a public key pair and return it in r_key.
  How the key is created depends on s_parms:
  (genkey
   (algo
     (parameter_name_1 ....)
      ....
     (parameter_name_n ....)
  ))
  The key is returned in a format depending on the
  algorithm. Both, private and secret keys are returned
  and optionally some additional informatin.
  For elgamal we return this structure:
  (key-data
   (public-key
     (elg
 	(p <mpi>)
 	(g <mpi>)
 	(y <mpi>)
     )
   )
   (private-key
     (elg
 	(p <mpi>)
 	(g <mpi>)
 	(y <mpi>)
 	(x <mpi>)
     )
   )
   (misc-key-info
      (pm1-factors n1 n2 ... nn)
   ))
 */
gcry_error_t
gcry_pk_genkey (gcry_sexp_t *r_key, gcry_sexp_t s_parms)
{
  gcry_pk_spec_t *pubkey = NULL;
  gcry_module_t module = NULL;
  gcry_sexp_t list = NULL, l2 = NULL;
  const char *name;
  size_t n;
  gcry_err_code_t rc = GPG_ERR_NO_ERROR;
  int i;
  const char *algo_name = NULL;
  int algo;
  const char *sec_elems = NULL, *pub_elems = NULL;
  gcry_mpi_t skey[10], *factors = NULL;
  unsigned int nbits = 0;
  unsigned long use_e = 0;
  char *name_terminated;

  REGISTER_DEFAULT_PUBKEYS;

  skey[0] = NULL;
  *r_key = NULL;

  list = gcry_sexp_find_token (s_parms, "genkey", 0);
  if (!list)
    {
      rc = GPG_ERR_INV_OBJ; /* Does not contain genkey data. */
      goto leave;
    }

  l2 = gcry_sexp_cadr (list);
  gcry_sexp_release (list);
  list = l2;
  l2 = NULL;
  if (! list)
    {
      rc = GPG_ERR_NO_OBJ; /* No cdr for the genkey. */
      goto leave;
    }

  name = gcry_sexp_nth_data (list, 0, &n);
  if (! name)
    {
      rc = GPG_ERR_INV_OBJ; /* Algo string missing. */
      goto leave;
    }

  name_terminated = gcry_xmalloc (n + 1);
  memcpy (name_terminated, name, n);
  name_terminated[n] = 0;
  ath_mutex_lock (&pubkeys_registered_lock);
  module = gcry_pk_lookup_name (name_terminated);
  ath_mutex_unlock (&pubkeys_registered_lock);
  gcry_free (name_terminated);

  if (! module)
    {
      rc = GPG_ERR_PUBKEY_ALGO; /* Unknown algorithm. */
      goto leave;
    }
  
  pubkey = (gcry_pk_spec_t *) module->spec;
  algo = module->mod_id;
  algo_name = pubkey->aliases? *pubkey->aliases : NULL;
  if (!algo_name || !*algo_name)
    algo_name = pubkey->name;
  pub_elems = pubkey->elements_pkey;
  sec_elems = pubkey->elements_skey;

  /* Handle the optional rsa-use-e element. */
  l2 = gcry_sexp_find_token (list, "rsa-use-e", 0);
  if (l2)
    {
      char buf[50];

      name = gcry_sexp_nth_data (l2, 1, &n);
      if ((! name) || (n >= DIM (buf) - 1))
        {
          rc = GPG_ERR_INV_OBJ; /* No value or value too large. */
          goto leave;
        }
      memcpy (buf, name, n);
      buf[n] = 0;
      use_e = strtoul (buf, NULL, 0);
      gcry_sexp_release (l2);
      l2 = NULL;
    }
  else
    use_e = 65537; /* Not given, use the value generated by old versions. */

  l2 = gcry_sexp_find_token (list, "nbits", 0);
  gcry_sexp_release (list);
  list = l2;
  l2 = NULL;
  
  if (! list)
    {
      rc = GPG_ERR_NO_OBJ; /* No nbits parameter. */
      goto leave;
    }

  name = gcry_sexp_nth_data (list, 1, &n);
  if (! name)
    {
      rc = GPG_ERR_INV_OBJ; /* nbits without a cdr. */
      goto leave;
    }
  
  name_terminated = gcry_xmalloc (n + 1);
  memcpy (name_terminated, name, n);
  name_terminated[n] = 0;
  nbits = (unsigned int) strtoul (name_terminated, NULL, 0);
  gcry_free (name_terminated);

  rc = pubkey_generate (module->mod_id, nbits, use_e, skey, &factors);
  if (rc)
    goto leave;

  {
    char *string, *p;
    size_t nelem=0, nelem_cp = 0, needed=0;
    gcry_mpi_t mpis[30];
    
    nelem = strlen (pub_elems) + strlen (sec_elems);
    for (i = 0; factors[i]; i++)
      nelem++;
    nelem_cp = nelem;

    needed += nelem * 10;
    needed += 2 * strlen (algo_name) + 300;
    if (nelem > DIM (mpis))
      BUG ();

    /* Build the string. */
    nelem = 0;
    string = p = gcry_xmalloc (needed);
    p = stpcpy (p, "(key-data");
    p = stpcpy (p, "(public-key(");
    p = stpcpy (p, algo_name);
    for(i = 0; pub_elems[i]; i++)
      {
        *p++ = '(';
        *p++ = pub_elems[i];
        p = stpcpy (p, "%m)");
        mpis[nelem++] = skey[i];
      }
    p = stpcpy (p, "))");
    p = stpcpy (p, "(private-key(");
    p = stpcpy (p, algo_name);
    for (i = 0; sec_elems[i]; i++)
      {
        *p++ = '(';
        *p++ = sec_elems[i];
        p = stpcpy (p, "%m)");
        mpis[nelem++] = skey[i];
      }
    p = stpcpy (p, "))");

    /* Very ugly hack to make release_mpi_array() work FIXME */
    skey[i] = NULL;

    p = stpcpy (p, "(misc-key-info(pm1-factors");
    for(i = 0; factors[i]; i++)
      {
        p = stpcpy (p, "%m");
        mpis[nelem++] = factors[i];
      }
    strcpy (p, ")))");

    while (nelem < DIM (mpis))
      mpis[nelem++] = NULL;

    {
      int elem_n = strlen (pub_elems) + strlen (sec_elems);
      void **arg_list;

      arg_list = malloc (nelem_cp * sizeof *arg_list);
      if (!arg_list)
        {
          rc = gpg_err_code_from_errno (errno);
          goto leave;
        }
      for (i = 0; i < elem_n; i++)
        arg_list[i] = mpis + i;
      for (; i < nelem_cp; i++)
        arg_list[i] = factors + i - elem_n;
      
      rc = gcry_sexp_build_array (r_key, NULL, string, arg_list);
      free (arg_list);
      if (rc)
	BUG ();
      assert (DIM (mpis) == 30); /* Reminder to make sure that the
                                    array gets increased if new
                                    parameters are added. */
    }
    gcry_free (string);
  }

 leave:
  release_mpi_array (skey);
  /* Don't free SKEY itself, it is a static array. */
    
  if (factors)
    {
      release_mpi_array ( factors );
      gcry_free (factors);
    }
  
  if (l2)
    gcry_sexp_release (l2);
  if (list)
    gcry_sexp_release (list);
  
  if (module)
    {
      ath_mutex_lock (&pubkeys_registered_lock);
      _gcry_module_release (module);
      ath_mutex_unlock (&pubkeys_registered_lock);
    }

  return gcry_error (rc);
}


/* 
   Get the number of nbits from the public key.

   Hmmm: Should we have really this function or is it better to have a
   more general function to retrieve different propoerties of the key?  */
unsigned int
gcry_pk_get_nbits (gcry_sexp_t key)
{
  gcry_module_t module = NULL;
  gcry_pk_spec_t *pubkey;
  gcry_mpi_t *keyarr = NULL;
  unsigned int nbits = 0;
  gcry_err_code_t rc;

  REGISTER_DEFAULT_PUBKEYS;

  rc = sexp_to_key (key, 0, &keyarr, &module);
  if (rc == GPG_ERR_INV_OBJ)
    rc = sexp_to_key (key, 1, &keyarr, &module);
  if (rc)
    return 0; /* Error - 0 is a suitable indication for that. */

  pubkey = (gcry_pk_spec_t *) module->spec;
  nbits = (*pubkey->get_nbits) (module->mod_id, keyarr);
  
  ath_mutex_lock (&pubkeys_registered_lock);
  _gcry_module_release (module);
  ath_mutex_unlock (&pubkeys_registered_lock);

  release_mpi_array (keyarr);
  gcry_free (keyarr);

  return nbits;
}


/* Return the so called KEYGRIP which is the SHA-1 hash of the public
   key parameters expressed in a way depended on the algorithm.

   ARRAY must either be 20 bytes long or NULL; in the latter case a
   newly allocated array of that size is returned, otherwise ARRAY or
   NULL is returned to indicate an error which is most likely an
   unknown algorithm.  The function accepts public or secret keys. */
unsigned char *
gcry_pk_get_keygrip (gcry_sexp_t key, unsigned char *array)
{
  gcry_sexp_t list = NULL, l2 = NULL;
  gcry_pk_spec_t *pubkey = NULL;
  gcry_module_t module = NULL;
  const char *s, *name;
  size_t n;
  int idx;
  int is_rsa;
  const char *elems;
  gcry_md_hd_t md = NULL;

  REGISTER_DEFAULT_PUBKEYS;

  /* Check that the first element is valid. */
  list = gcry_sexp_find_token (key, "public-key", 0);
  if (!list)
    list = gcry_sexp_find_token (key, "private-key", 0);
  if (!list)
    list = gcry_sexp_find_token (key, "protected-private-key", 0);
  if (!list)
    list = gcry_sexp_find_token (key, "shadowed-private-key", 0);
  if (!list)
    return NULL; /* No public- or private-key object. */

  l2 = gcry_sexp_cadr (list);
  gcry_sexp_release (list);
  list = l2;
  l2 = NULL;

  name = gcry_sexp_nth_data (list, 0, &n);
  if (! name)
    goto fail; /* Invalid structure of object. */

  {
    char *name_terminated = gcry_xmalloc (n + 1);
    memcpy (name_terminated, name, n);
    name_terminated[n] = 0;
    ath_mutex_lock (&pubkeys_registered_lock);
    module = gcry_pk_lookup_name (name_terminated);
    ath_mutex_unlock (&pubkeys_registered_lock);
    gcry_free (name_terminated);
  }

  if (! module)
    goto fail; /* unknown algorithm */

  pubkey = (gcry_pk_spec_t *) module->spec;

  /* FIXME, special handling should be implemented by the algorithms,
     not by the libgcrypt core.  */
  is_rsa = module->mod_id == GCRY_PK_RSA;
  elems = pubkey->elements_grip;
  if (! elems)
    goto fail; /* no grip parameter */
    
  if (gcry_md_open (&md, GCRY_MD_SHA1, 0))
    goto fail;

  for (idx = 0, s = elems; *s; s++, idx++)
    {
      const char *data;
      size_t datalen;

      l2 = gcry_sexp_find_token (list, s, 1);
      if (! l2)
        goto fail;
      data = gcry_sexp_nth_data (l2, 1, &datalen);
      if (! data)
        goto fail;
      if (!is_rsa)
        {
          char buf[30];

          sprintf (buf, "(1:%c%u:", *s, (unsigned int)datalen);
          gcry_md_write (md, buf, strlen (buf));
        }
  
      /* PKCS-15 says that for RSA only the modulus should be hashed -
         however, it is not clear wether this is meant to has the raw
         bytes assuming this is an unsigned integer or whether the DER
         required 0 should be prefixed. We hash the raw bytes.  For
         non-RSA we hash S-expressions. */
      gcry_md_write (md, data, datalen);
      gcry_sexp_release (l2);
      if (!is_rsa)
        gcry_md_write (md, ")", 1);
    }

  if (!array)
    {
      array = gcry_malloc (20);
      if (! array)
        goto fail;
    }

  memcpy (array, gcry_md_read (md, GCRY_MD_SHA1), 20);
  gcry_md_close (md);
  gcry_sexp_release (list);
  return array;

 fail:
  if (l2)
    gcry_sexp_release (l2);
  if (md)
    gcry_md_close (md);
  gcry_sexp_release (list);
  return NULL;
}


gcry_error_t
gcry_pk_ctl (int cmd, void *buffer, size_t buflen)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;

  REGISTER_DEFAULT_PUBKEYS;

  switch (cmd)
    {
    case GCRYCTL_DISABLE_ALGO:
      /* This one expects a buffer pointing to an integer with the
         algo number.  */
      if ((! buffer) || (buflen != sizeof (int)))
	err = GPG_ERR_INV_ARG;
      else
	disable_pubkey_algo (*((int *) buffer));
      break;

    default:
      err = GPG_ERR_INV_OP;
    }

  return gcry_error (err);
}


/*
   Return information about the given algorithm
   WHAT select the kind of information returned:
    GCRYCTL_TEST_ALGO:
        Returns 0 when the specified algorithm is available for use.
        Buffer must be NULL, nbytes  may have the address of a variable
        with the required usage of the algorithm. It may be 0 for don't
        care or a combination of the GCRY_PK_USAGE_xxx flags;
    GCRYCTL_GET_ALGO_USAGE:
        Return the usage glafs for the give algo.  An invalid alog
        does return 0.  Disabled algos are ignored here becuase we
        only want to know whether the algo is at all capable of
        the usage.
  
   Note: Because this function is in most cases used to return an
   integer value, we can make it easier for the caller to just look at
   the return value.  The caller will in all cases consult the value
   and thereby detecting whether a error occured or not (i.e. while
   checking the block size) */
gcry_error_t
gcry_pk_algo_info (int algorithm, int what, void *buffer, size_t *nbytes)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;

  switch (what)
    {
    case GCRYCTL_TEST_ALGO:
      {
	int use = nbytes ? *nbytes : 0;
	if (buffer)
	  err = GPG_ERR_INV_ARG;
	else if (check_pubkey_algo (algorithm, use))
	  err = GPG_ERR_PUBKEY_ALGO;
	break;
      }

    case GCRYCTL_GET_ALGO_USAGE:
      {
	gcry_module_t pubkey;
	int use = 0;

	REGISTER_DEFAULT_PUBKEYS;

	ath_mutex_lock (&pubkeys_registered_lock);
	pubkey = _gcry_module_lookup_id (pubkeys_registered, algorithm);
	if (pubkey)
	  {
	    use = ((gcry_pk_spec_t *) pubkey->spec)->use;
	    _gcry_module_release (pubkey);
	  }
	ath_mutex_unlock (&pubkeys_registered_lock);

	/* FIXME? */
	*nbytes = use;
      }

    case GCRYCTL_GET_ALGO_NPKEY:
      {
	/* FIXME?  */
	int npkey = pubkey_get_npkey (algorithm);
	*nbytes = npkey;
	break;
      }
    case GCRYCTL_GET_ALGO_NSKEY:
      {
	/* FIXME?  */
	int nskey = pubkey_get_nskey (algorithm);
	*nbytes = nskey;
	break;
      }
    case GCRYCTL_GET_ALGO_NSIGN:
      {
	/* FIXME?  */
	int nsign = pubkey_get_nsig (algorithm);
	*nbytes = nsign;
	break;
      }
    case GCRYCTL_GET_ALGO_NENCR:
      {
	/* FIXME?  */
	int nencr = pubkey_get_nenc (algorithm);
	*nbytes = nencr;
	break;
      }

    default:
      err = GPG_ERR_INV_OP;
    }

  return gcry_error (err);
}


gcry_err_code_t
_gcry_pk_init (void)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;

  REGISTER_DEFAULT_PUBKEYS;

  return err;
}


gcry_err_code_t
_gcry_pk_module_lookup (int algorithm, gcry_module_t *module)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;
  gcry_module_t pubkey;

  REGISTER_DEFAULT_PUBKEYS;

  ath_mutex_lock (&pubkeys_registered_lock);
  pubkey = _gcry_module_lookup_id (pubkeys_registered, algorithm);
  if (pubkey)
    *module = pubkey;
  else
    err = GPG_ERR_PUBKEY_ALGO;
  ath_mutex_unlock (&pubkeys_registered_lock);

  return err;
}


void
_gcry_pk_module_release (gcry_module_t module)
{
  ath_mutex_lock (&pubkeys_registered_lock);
  _gcry_module_release (module);
  ath_mutex_unlock (&pubkeys_registered_lock);
}

/* Get a list consisting of the IDs of the loaded pubkey modules.  If
   LIST is zero, write the number of loaded pubkey modules to
   LIST_LENGTH and return.  If LIST is non-zero, the first
   *LIST_LENGTH algorithm IDs are stored in LIST, which must be of
   according size.  In case there are less pubkey modules than
   *LIST_LENGTH, *LIST_LENGTH is updated to the correct number.  */
gcry_error_t
gcry_pk_list (int *list, int *list_length)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;

  ath_mutex_lock (&pubkeys_registered_lock);
  err = _gcry_module_list (pubkeys_registered, list, list_length);
  ath_mutex_unlock (&pubkeys_registered_lock);

  return err;
}
