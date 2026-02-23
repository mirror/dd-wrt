#include "testutils.h"

#include "knuth-lfib.h"

static void
progress(void *ctx UNUSED, int c)
{
  fputc(c, stderr);
}

void
test_main(void)
{
  struct dsa_params params;
  mpz_t pub;
  mpz_t key;

  struct knuth_lfib_ctx lfib;
  
  dsa_params_init(&params);
  mpz_init(pub);
  mpz_init(key);

  knuth_lfib_init(&lfib, 13);

  ASSERT (dsa_generate_params(&params,
			      &lfib,
			      (nettle_random_func *) knuth_lfib_random,
			      NULL, verbose ? progress : NULL,
			      1024, 160));
  dsa_generate_keypair(&params, pub, key,
		       &lfib,
		       (nettle_random_func *) knuth_lfib_random);

  test_dsa_key(&params, pub, key, 160);
  test_dsa160(&params, pub, key, NULL);

  ASSERT (dsa_generate_params(&params,
			       &lfib,
			       (nettle_random_func *) knuth_lfib_random,
			       NULL, verbose ? progress : NULL,
			       2048, 256));
  dsa_generate_keypair(&params, pub, key,
		       &lfib,
		       (nettle_random_func *) knuth_lfib_random);

  test_dsa_key(&params, pub, key, 256);
  test_dsa256(&params, pub, key, NULL);
  
  ASSERT (dsa_generate_params(&params,
			      &lfib,
			      (nettle_random_func *) knuth_lfib_random,
			      NULL, verbose ? progress : NULL,
			      2048, 224));
  dsa_generate_keypair(&params, pub, key,
		       &lfib,
		       (nettle_random_func *) knuth_lfib_random);

  test_dsa_key(&params, pub, key, 224);
  test_dsa256(&params, pub, key, NULL);

  /* Test with large q */
  if (!dsa_generate_params (&params,
			    &lfib,
			    (nettle_random_func *) knuth_lfib_random,
			    NULL, verbose ? progress : NULL,
			    1024, 768))
    FAIL();

  dsa_generate_keypair (&params, pub, key,
			&lfib,
			(nettle_random_func *) knuth_lfib_random);
  test_dsa_key(&params, pub, key, 768);
  test_dsa256(&params, pub, key, NULL);
  
  dsa_params_clear(&params);
  mpz_clear(pub);
  mpz_clear(key);
}
