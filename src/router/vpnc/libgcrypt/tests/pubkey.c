/* pubkey.c - Public key encryption/decryption tests
 *	Copyright (C) 2001, 2002, 2003 Free Software Foundation, Inc.
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "../src/gcrypt.h"

/* Sample RSA keys, taken from basic.c.  */

static const char sample_private_key_1[] =
"(private-key\n"
" (openpgp-rsa\n"
"  (n #00e0ce96f90b6c9e02f3922beada93fe50a875eac6bcc18bb9a9cf2e84965caa"
      "2d1ff95a7f542465c6c0c19d276e4526ce048868a7a914fd343cc3a87dd74291"
      "ffc565506d5bbb25cbac6a0e2dd1f8bcaab0d4a29c2f37c950f363484bf269f7"
      "891440464baf79827e03a36e70b814938eebdc63e964247be75dc58b014b7ea251#)\n"
"  (e #010001#)\n"
"  (d #046129F2489D71579BE0A75FE029BD6CDB574EBF57EA8A5B0FDA942CAB943B11"
      "7D7BB95E5D28875E0F9FC5FCC06A72F6D502464DABDED78EF6B716177B83D5BD"
      "C543DC5D3FED932E59F5897E92E6F58A0F33424106A3B6FA2CBF877510E4AC21"
      "C3EE47851E97D12996222AC3566D4CCB0B83D164074ABF7DE655FC2446DA1781#)\n"
"  (p #00e861b700e17e8afe6837e7512e35b6ca11d0ae47d8b85161c67baf64377213"
      "fe52d772f2035b3ca830af41d8a4120e1c1c70d12cc22f00d28d31dd48a8d424f1#)\n"
"  (q #00f7a7ca5367c661f8e62df34f0d05c10c88e5492348dd7bddc942c9a8f369f9"
      "35a07785d2db805215ed786e4285df1658eed3ce84f469b81b50d358407b4ad361#)\n"
"  (u #304559a9ead56d2309d203811a641bb1a09626bc8eb36fffa23c968ec5bd891e"
      "ebbafc73ae666e01ba7c8990bae06cc2bbe10b75e69fcacb353a6473079d8e9b#)\n"
" )\n"
")\n";

static const char sample_public_key_1[] =
"(public-key\n"
" (rsa\n"
"  (n #00e0ce96f90b6c9e02f3922beada93fe50a875eac6bcc18bb9a9cf2e84965caa"
      "2d1ff95a7f542465c6c0c19d276e4526ce048868a7a914fd343cc3a87dd74291"
      "ffc565506d5bbb25cbac6a0e2dd1f8bcaab0d4a29c2f37c950f363484bf269f7"
      "891440464baf79827e03a36e70b814938eebdc63e964247be75dc58b014b7ea251#)\n"
"  (e #010001#)\n"
" )\n"
")\n";

#define RANDOM_DATA_NBITS 800

static int verbose;

static void
die (const char *format, ...)
{
  va_list arg_ptr ;

  va_start( arg_ptr, format ) ;
  vfprintf (stderr, format, arg_ptr );
  va_end(arg_ptr);
  exit (1);
}

static void
check_keys_crypt (gcry_sexp_t pkey, gcry_sexp_t skey,
		  gcry_sexp_t plain0)
{
  gcry_sexp_t plain1, cipher, l;
  gcry_mpi_t x0, x1;
  int rc;
  int have_flags;

  /* Extract data from plaintext.  */
  l = gcry_sexp_find_token (plain0, "value", 0);
  x0 = gcry_sexp_nth_mpi (l, 1, GCRYMPI_FMT_USG);

  /* Encrypt data.  */
  rc = gcry_pk_encrypt (&cipher, plain0, pkey);
  if (rc)
    die ("encryption failed: %s\n", gcry_strerror (rc));

  l = gcry_sexp_find_token (cipher, "flags", 0);
  have_flags = !!l;
  gcry_sexp_release (l);

  /* Decrypt data.  */
  rc = gcry_pk_decrypt (&plain1, cipher, skey);
  gcry_sexp_release (cipher);
  if (rc)
    die ("decryption failed: %s\n", gcry_strerror (rc));

  /* Extract decrypted data.  Note that for compatibility reasons, the
     output of gcry_pk_decrypt depends on whether a flags lists (even
     if empty) occurs in its input data.  Because we passed the output
     of encrypt directly to decrypt, such a flag value won't be there
     as of today.  We check it anyway. */
  l = gcry_sexp_find_token (plain1, "value", 0);
  if (l)
    {
      if (!have_flags)
        die ("compatibility mode of pk_decrypt broken\n");
      gcry_sexp_release (plain1);
      x1 = gcry_sexp_nth_mpi (l, 1, GCRYMPI_FMT_USG);
      gcry_sexp_release (l);
    }
  else
    {
      if (have_flags)
        die ("compatibility mode of pk_decrypt broken\n");
      x1 = gcry_sexp_nth_mpi (plain1, 0, GCRYMPI_FMT_USG);
      gcry_sexp_release (plain1);
    }

  /* Compare.  */
  if (gcry_mpi_cmp (x0, x1))
    die ("data corrupted\n");
}

static void
check_keys (gcry_sexp_t pkey, gcry_sexp_t skey)
{
  gcry_sexp_t plain;
  gcry_mpi_t x;
  int rc;
  
  /* Create plain text.  */
  x = gcry_mpi_new (RANDOM_DATA_NBITS);
  gcry_mpi_randomize (x, RANDOM_DATA_NBITS, GCRY_WEAK_RANDOM);
  
  rc = gcry_sexp_build (&plain, NULL, "(data (flags raw) (value %m))", x);
  if (rc)
    die ("converting data for encryption failed: %s\n",
	 gcry_strerror (rc));

  check_keys_crypt (pkey, skey, plain);
  gcry_sexp_release (plain);
  gcry_mpi_release (x);

  /* Create plain text.  */
  x = gcry_mpi_new (RANDOM_DATA_NBITS);
  gcry_mpi_randomize (x, RANDOM_DATA_NBITS, GCRY_WEAK_RANDOM);
  
  rc = gcry_sexp_build (&plain, NULL, "(data (flags raw no-blinding) (value %m))", x);
  if (rc)
    die ("converting data for encryption failed: %s\n",
	 gcry_strerror (rc));

  check_keys_crypt (pkey, skey, plain);
  gcry_sexp_release (plain);
}

static void
get_keys_sample (gcry_sexp_t *pkey, gcry_sexp_t *skey)
{
  gcry_sexp_t pub_key, sec_key;
  int rc;

  rc = gcry_sexp_sscan (&pub_key, NULL, sample_public_key_1,
			strlen (sample_public_key_1));
  if (! rc)
    rc = gcry_sexp_sscan (&sec_key, NULL, sample_private_key_1,
			  strlen (sample_private_key_1));
  if (rc)
    die ("converting sample keys failed: %s\n", gcry_strerror (rc));

  *pkey = pub_key;
  *skey = sec_key;
}

static void
get_keys_new (gcry_sexp_t *pkey, gcry_sexp_t *skey)
{
  gcry_sexp_t key_spec, key, pub_key, sec_key;
  int rc;
  
  rc = gcry_sexp_new (&key_spec,
		      "(genkey (rsa (nbits 4:1024)))", 0, 1);
  if (rc)
    die ("error creating S-expression: %s\n", gcry_strerror (rc));
  rc = gcry_pk_genkey (&key, key_spec);
  gcry_sexp_release (key_spec);
  if (rc)
    die ("error generating RSA key: %s\n", gcry_strerror (rc));
    
  pub_key = gcry_sexp_find_token (key, "public-key", 0);
  if (! pub_key)
    die ("public part missing in key\n");

  sec_key = gcry_sexp_find_token (key, "private-key", 0);
  if (! sec_key)
    die ("private part missing in key\n");

  gcry_sexp_release (key);
  *pkey = pub_key;
  *skey = sec_key;
}

static void
check_run (void)
{
  gcry_sexp_t pkey, skey;

  /* Check sample keys.  */
  get_keys_sample (&pkey, &skey);
  check_keys (pkey, skey);
  gcry_sexp_release (pkey);
  gcry_sexp_release (skey);
  
  /* Check newly generated keys.  */
  get_keys_new (&pkey, &skey);
  check_keys (pkey, skey);
  gcry_sexp_release (pkey);
  gcry_sexp_release (skey);
}

int
main (int argc, char **argv)
{
  int debug = 0;
  int i = 10;

  if (argc > 1 && !strcmp (argv[1], "--verbose"))
    verbose = 1;
  else if (argc > 1 && !strcmp (argv[1], "--debug"))
    verbose = debug = 1;

  gcry_control (GCRYCTL_DISABLE_SECMEM, 0);
  if (!gcry_check_version (GCRYPT_VERSION))
    die ("version mismatch\n");
  gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);
  if (debug)
    gcry_control (GCRYCTL_SET_DEBUG_FLAGS, 1u , 0);
  gcry_control (GCRYCTL_ENABLE_QUICK_RANDOM, 0);

  for (; i > 0; i--)
    check_run ();
  
  return 0;
}
