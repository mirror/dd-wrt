/* slh-dsa-test.c

   Copyright (C) 2025 Niels Möller

   This file is part of GNU Nettle.

   GNU Nettle is free software: you can redistribute it and/or
   modify it under the terms of either:

     * the GNU Lesser General Public License as published by the Free
       Software Foundation; either version 3 of the License, or (at your
       option) any later version.

   or

     * the GNU General Public License as published by the Free
       Software Foundation; either version 2 of the License, or (at your
       option) any later version.

   or both in parallel, as here.

   GNU Nettle is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received copies of the GNU General Public License and
   the GNU Lesser General Public License along with this program.  If
   not, see http://www.gnu.org/licenses/.
*/

#include "testutils.h"

#include "base16.h"
#include "sha3.h"
#include "slh-dsa.h"
#include "slh-dsa-internal.h"
#include "bswap-internal.h"

#include <errno.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static const struct tstring *
read_hex_file (const char *name, size_t max_size)
{
  char input_buf[1000];
  FILE *input = open_srcdir_file (name);
  size_t done;
  struct tstring *s;
  struct base16_decode_ctx ctx;
  base16_decode_init (&ctx);

  /* This function expects to read up to EOF, and fails if the amount
     of data exceeds the max size. */
  s = tstring_alloc (max_size);

  for (done = 0;;)
    {
      size_t left = s->length - done;
      size_t got = fread (input_buf, 1, MIN (left, sizeof (input_buf)), input);
      size_t res;

      if (!got)
	{
	  if (ferror (input))
	    {
	      fprintf (stderr, "reading %s failed: %s\n", name, strerror (errno));
	      FAIL ();
	    }
	  fclose (input);
	  ASSERT (base16_decode_final (&ctx));
	  s->length = done;
	  return s;
	}

      res = s->length - done;
      if (!base16_decode_update (&ctx, &res, s->data + done, got, input_buf))
	{
	  fprintf (stderr, "hex decoding %s failed (possibly exceeding max size)\n", name);
	  FAIL ();
	}
      done += res;
    }
}

static void
test_wots_gen (const struct tstring *public_seed, const struct tstring *secret_seed,
	       unsigned layer, uint64_t tree_idx, uint32_t keypair,
	       const struct tstring *exp_pub)
{
  struct sha3_ctx tree_ctx;
  struct sha3_ctx scratch_ctx;
  uint8_t pub[_SLH_DSA_128_SIZE];
  ASSERT (public_seed->length == _SLH_DSA_128_SIZE);
  ASSERT (secret_seed->length == _SLH_DSA_128_SIZE);
  ASSERT (exp_pub->length == _SLH_DSA_128_SIZE);

  _slh_hash_shake.init_tree (&tree_ctx, public_seed->data, layer, tree_idx);

  _wots_gen (&_slh_hash_shake, &tree_ctx, secret_seed->data, keypair, pub, &scratch_ctx);
  mark_bytes_defined (sizeof (pub), pub);
  ASSERT (MEMEQ (sizeof (pub), pub, exp_pub->data));
}

static void
test_wots_sign (const struct tstring *public_seed, const struct tstring *secret_seed,
		unsigned layer, uint64_t tree_idx, uint32_t keypair, const struct tstring *msg,
		const struct tstring *exp_pub, const struct tstring *exp_sig)
{
  struct sha3_ctx tree_ctx;
  struct sha3_ctx scratch_ctx;
  uint8_t sig[WOTS_SIGNATURE_SIZE];
  uint8_t pub[_SLH_DSA_128_SIZE];
  ASSERT (public_seed->length == _SLH_DSA_128_SIZE);
  ASSERT (secret_seed->length == _SLH_DSA_128_SIZE);
  ASSERT (msg->length == _SLH_DSA_128_SIZE);
  ASSERT (exp_pub->length == _SLH_DSA_128_SIZE);
  ASSERT (exp_sig->length == WOTS_SIGNATURE_SIZE);

  _slh_hash_shake.init_tree (&tree_ctx, public_seed->data, layer, tree_idx);

  _wots_sign (&_slh_hash_shake, &tree_ctx, secret_seed->data, keypair,
	      msg->data, sig, pub, &scratch_ctx);
  mark_bytes_defined (sizeof (sig), sig);
  mark_bytes_defined (sizeof (pub), pub);
  ASSERT (MEMEQ (sizeof (sig), sig, exp_sig->data));
  ASSERT (MEMEQ (sizeof (pub), pub, exp_pub->data));

  memset (pub, 0, sizeof (pub));
  _wots_verify (&_slh_hash_shake, &tree_ctx, keypair, msg->data, sig, pub, &scratch_ctx);
  ASSERT (MEMEQ (sizeof (pub), pub, exp_pub->data));
}

/* The xmss_leaf and xmss_node functions copied from slh-xmss.c */
static void
xmss_leaf (const struct slh_merkle_ctx_secret *ctx, unsigned idx, uint8_t *leaf)
{
  _wots_gen (ctx->pub.hash, ctx->pub.tree_ctx, ctx->secret_seed, idx, leaf, ctx->scratch_ctx);
  mark_bytes_defined (SLH_DSA_128_SEED_SIZE, leaf);
}

static void
xmss_node (const struct slh_merkle_ctx_public *ctx, unsigned height, unsigned index,
	   const uint8_t *left, const uint8_t *right, uint8_t *out)
{
  struct slh_address_hash ah =
    {
      bswap32_if_le (SLH_XMSS_TREE),
      0,
      bswap32_if_le (height),
      bswap32_if_le (index),
    };

  ctx->hash->node (ctx->tree_ctx, &ah, left, right, out);
}

static void
test_merkle (const struct tstring *public_seed, const struct tstring *secret_seed,
	     unsigned h,
	     unsigned layer, uint64_t tree_idx, uint32_t idx, const struct tstring *msg,
	     const struct tstring *exp_pub, const struct tstring *exp_sig)
{
  struct sha3_ctx tree_ctx;
  struct sha3_ctx scratch_ctx;
  const struct slh_merkle_ctx_secret ctx =
    {
      { &_slh_hash_shake, &tree_ctx, 0 },
      secret_seed->data, &scratch_ctx
    };

  uint8_t *sig = xalloc (XMSS_AUTH_SIZE (h));
  uint8_t pub[_SLH_DSA_128_SIZE];

  ASSERT (public_seed->length == _SLH_DSA_128_SIZE);
  ASSERT (secret_seed->length == _SLH_DSA_128_SIZE);
  ASSERT (msg->length == _SLH_DSA_128_SIZE);
  ASSERT (exp_pub->length == _SLH_DSA_128_SIZE);
  ASSERT (exp_sig->length == XMSS_AUTH_SIZE (h));

  _slh_hash_shake.init_tree (&tree_ctx, public_seed->data, layer, tree_idx);

  _merkle_sign (&ctx, xmss_leaf, xmss_node, h, idx, sig);
  ASSERT (MEMEQ (exp_sig->length, sig, exp_sig->data));

  memcpy (pub, msg->data, sizeof (pub));
  _merkle_verify (&ctx.pub, xmss_node, h, idx, sig, pub);
  ASSERT (MEMEQ (sizeof (pub), pub, exp_pub->data));
  free (sig);
}

static void
test_fors_gen (const struct tstring *public_seed, const struct tstring *secret_seed,
	       unsigned layer, uint64_t tree_idx, unsigned keypair, unsigned idx,
	       const struct tstring *exp_sk, const struct tstring *exp_leaf)
{
  struct sha3_ctx tree_ctx;
  const struct slh_merkle_ctx_secret ctx =
    {
      { &_slh_hash_shake, &tree_ctx, keypair },
      secret_seed->data, NULL
    };
  uint8_t sk[_SLH_DSA_128_SIZE];
  uint8_t leaf[_SLH_DSA_128_SIZE];
  ASSERT (public_seed->length == _SLH_DSA_128_SIZE);
  ASSERT (secret_seed->length == _SLH_DSA_128_SIZE);
  ASSERT (exp_sk->length == _SLH_DSA_128_SIZE);
  ASSERT (exp_leaf->length == _SLH_DSA_128_SIZE);

  _slh_hash_shake.init_tree (&tree_ctx, public_seed->data, layer, tree_idx);

  _fors_gen (&ctx, idx, sk, leaf);
  mark_bytes_defined (sizeof (sk), sk);
  mark_bytes_defined (sizeof (sk), leaf);
  ASSERT (MEMEQ (sizeof (sk), sk, exp_sk->data));
  ASSERT (MEMEQ (sizeof (leaf), leaf, exp_leaf->data));
}

static void
test_fors_sign (const struct tstring *public_seed, const struct tstring *secret_seed,
		const struct slh_fors_params *fors,
		unsigned layer, uint64_t tree_idx, unsigned keypair, const struct tstring *msg,
		const struct tstring *exp_pub, const struct tstring *exp_sig)
{
  struct sha3_ctx tree_ctx, scratch_ctx;
  const struct slh_merkle_ctx_secret ctx =
    {
      { &_slh_hash_shake, &tree_ctx, keypair },
      secret_seed->data, NULL
    };
  uint8_t pub[_SLH_DSA_128_SIZE];
  uint8_t *sig = xalloc (fors->signature_size);
  ASSERT (public_seed->length == _SLH_DSA_128_SIZE);
  ASSERT (secret_seed->length == _SLH_DSA_128_SIZE);
  ASSERT (msg->length == fors->msg_size);
  ASSERT (exp_pub->length == _SLH_DSA_128_SIZE);
  ASSERT (exp_sig->length == fors->signature_size);

  _slh_hash_shake.init_tree (&tree_ctx, public_seed->data, layer, tree_idx);

  _fors_sign (&ctx, fors, msg->data, sig, pub, &scratch_ctx);
  mark_bytes_defined (exp_sig->length, sig);
  mark_bytes_defined (sizeof (pub), pub);
  ASSERT (MEMEQ (exp_sig->length, sig, exp_sig->data));
  ASSERT (MEMEQ (sizeof (pub), pub, exp_pub->data));

  memset (pub, 0, sizeof (pub));
  _fors_verify (&ctx.pub, fors, msg->data, sig, pub, &scratch_ctx);
  ASSERT (MEMEQ (sizeof (pub), pub, exp_pub->data));
  free (sig);
}

static void
test_xmss_sign (const struct tstring *public_seed, const struct tstring *secret_seed,
		unsigned xmss_h,
		unsigned layer, uint64_t tree_idx, uint32_t idx, const struct tstring *msg,
		const struct tstring *exp_pub, const struct tstring *exp_sig)
{
  struct sha3_ctx tree_ctx;
  struct sha3_ctx scratch_ctx;
  const struct slh_merkle_ctx_secret ctx =
    {
      { &_slh_hash_shake, &tree_ctx, 0 },
      secret_seed->data, &scratch_ctx
    };

  uint8_t *sig = xalloc (XMSS_SIGNATURE_SIZE (xmss_h));
  uint8_t pub[_SLH_DSA_128_SIZE];
  ASSERT (public_seed->length == _SLH_DSA_128_SIZE);
  ASSERT (secret_seed->length == _SLH_DSA_128_SIZE);
  ASSERT (msg->length == _SLH_DSA_128_SIZE);
  ASSERT (exp_pub->length == _SLH_DSA_128_SIZE);
  ASSERT (exp_sig->length == XMSS_SIGNATURE_SIZE (xmss_h));

  _slh_hash_shake.init_tree (&tree_ctx, public_seed->data, layer, tree_idx);

  _xmss_sign (&ctx, xmss_h, idx, msg->data, sig, pub);
  mark_bytes_defined (sizeof (pub), pub);
  mark_bytes_defined (exp_sig->length, sig);
  ASSERT (MEMEQ (exp_sig->length, sig, exp_sig->data));
  ASSERT (MEMEQ (sizeof (pub), pub, exp_pub->data));

  memset (pub, 0, sizeof (pub));
  _xmss_verify (&ctx.pub, xmss_h, idx, msg->data, sig, pub, &scratch_ctx);
  ASSERT (MEMEQ (sizeof (pub), pub, exp_pub->data));
  free (sig);
}

typedef void root_func (const uint8_t *public_seed, const uint8_t *secret_seed, uint8_t *root);

static void
test_slh_dsa_128_root (root_func *f,
		       const struct tstring *public_seed, const struct tstring *secret_seed,
		       const struct tstring *exp_pub)
{
  uint8_t pub[_SLH_DSA_128_SIZE];
  ASSERT (public_seed->length == _SLH_DSA_128_SIZE);
  ASSERT (secret_seed->length == _SLH_DSA_128_SIZE);
  ASSERT (exp_pub->length == _SLH_DSA_128_SIZE);

  f (public_seed->data, secret_seed->data, pub);
  mark_bytes_defined (sizeof (pub), pub);
  ASSERT (MEMEQ (sizeof (pub), pub, exp_pub->data));
}

typedef void sign_func (const uint8_t *pub, const uint8_t *priv,
			size_t length, const uint8_t *msg,
			uint8_t *signature);
typedef int verify_func (const uint8_t *pub,
			 size_t length, const uint8_t *msg,
			 const uint8_t *signature);
struct slh_dsa_alg
{
  const char *name;
  size_t key_size;
  size_t signature_size;
  sign_func *sign;
  verify_func *verify;
};

static const struct slh_dsa_alg
slh_dsa_shake_128s = {
  "slh_dsa_shake_128s",
  SLH_DSA_128_KEY_SIZE,
  SLH_DSA_128S_SIGNATURE_SIZE,
  slh_dsa_shake_128s_sign,
  slh_dsa_shake_128s_verify,
};

static const struct slh_dsa_alg
slh_dsa_shake_128f = {
  "slh_dsa_shake_128f",
  SLH_DSA_128_KEY_SIZE,
  SLH_DSA_128F_SIGNATURE_SIZE,
  slh_dsa_shake_128f_sign,
  slh_dsa_shake_128f_verify,
};

static const struct slh_dsa_alg
slh_dsa_sha2_128s = {
  "slh_dsa_sha2_128s",
  SLH_DSA_128_KEY_SIZE,
  SLH_DSA_128S_SIGNATURE_SIZE,
  slh_dsa_sha2_128s_sign,
  slh_dsa_sha2_128s_verify,
};

static const struct slh_dsa_alg
slh_dsa_sha2_128f = {
  "slh_dsa_sha2_128f",
  SLH_DSA_128_KEY_SIZE,
  SLH_DSA_128F_SIGNATURE_SIZE,
  slh_dsa_sha2_128f_sign,
  slh_dsa_sha2_128f_verify,
};

static void
test_slh_dsa (const struct slh_dsa_alg *alg,
	      const struct tstring *pub, const struct tstring *priv,
	      const struct tstring *msg, const struct tstring *ref)
{
  uint8_t *sig = xalloc (alg->signature_size);
  ASSERT (pub->length == alg->key_size);
  ASSERT (priv->length == alg->key_size);
  ASSERT (ref->length == alg->signature_size);

  alg->sign (pub->data, priv->data, msg->length, msg->data, sig);
  if (! MEMEQ (alg->signature_size, sig, ref->data))
    {
      size_t i;
      for (i = 0; i < alg->signature_size; i++)
	if (sig[i] != ref->data[i])
	  break;

      fprintf (stderr, "failed %s sign, first diff at %zd\n", alg->name, i);
      abort ();
    }
  ASSERT (alg->verify (pub->data, msg->length, msg->data, sig));

  if (msg->length > 0)
    ASSERT (!alg->verify (pub->data, msg->length-1, msg->data, sig));
  sig[alg->signature_size-1] ^= 1;
  ASSERT (!alg->verify (pub->data, msg->length, msg->data, sig));

  free (sig);
}

void
test_main (void)
{
  const struct tstring *public_seed =
    SHEX ("b505d7cfad1b497499323c8686325e47");

  const struct tstring *secret_seed =
    SHEX ("7c9935a0b07694aa0c6d10e4db6b1add");

  mark_bytes_undefined (2*SLH_DSA_128_SEED_SIZE, secret_seed->data);

  test_wots_gen (public_seed, secret_seed, 6, 0, 0,
		 SHEX ("38c9077d76d1e32933fb58a53e769ed7"));
  test_wots_gen (public_seed, secret_seed, 6, 0, 1,
		 SHEX ("a026afacc77c7d97eebe6f88c70fec2d"));
  test_wots_gen (public_seed, secret_seed, 0, UINT64_C(0x29877722d7c079), 0x156,
		 SHEX ("99747c3547770fa288a628ed15122d3e"));

  test_wots_sign (public_seed, secret_seed, 0, UINT64_C(0x29877722d7c079), 0x156,
		  SHEX ("3961b2cab15e08c633be827744a07f01"),
		  SHEX ("99747c3547770fa288a628ed15122d3e"),
		  SHEX ("e1933de10e3fface 5fb8f8707c35ac13 74dc14ee8518481c 7e63d936ecc62f50"
			"c7f951b87bc716dc 45e9bcfec6f6d97e 7fafdacb6db05ed3 778f21851f325e25"
			"470da8dd81c41223 6d66cbee9ffa9c50 b86aa40baf213494 dfacca22aa0fb479"
			"53928735ca4212cf 53a09ab0335d20a8 e62ede797c8e7493 54d636f15f3150c5"
			"52797b76c091a41f 949f7fb57b42f744 1cca410264d6421f 4aa2c7e2ff4834a8"
			"db0e6e7750b2e11f f1c89a42d1fbc271 8358e38325886ad1 2346cd694f9eab73"
			"46c9a23b5ebe7637 bfd834a412318b01 188b0f29e3bd979f 8ae734acf1563af3"
			"03d3c095e9eaeba3 5207b9df3acf9ee4 7da5c1e2652f3b86 41698f3d2260591b"
			"07d00565e5d6be18 36033d2b7ef2c33b dc5cf3bba95b42df 6f73345b835341b2"
			"50e2862c9f2f9cef 77cfa74cfb04c560 d8a0038c4e96cb0d a2b3e9b2cd3cecf5"
			"22fda0d67e5f62b2 ee23bd42a61c7da4 8f0ea30b81af7ccb 6bb02cde272d2574"
			"1325e9d91535615c 0184f2d7f226141d 79b42412721fd345 61d93663650b3c1b"
			"6901872bc4c0bb15 bcd9038950b7717f 7f448b6126592076 a2bad2d63c55399c"
			"243fdbdb0c8d676b 2ae455e7f0a9b18d 3fc889c43387f2cb c4dc73d7c85bfab6"
			"b4b04463a3dd359c 3a8f61bfa6c4b042 4aeba4dd8a95ec12 43b2e36c29f82e1d"
			"711281599b3e05e7 5492ae3425eaa7f1 4ff8c6a9630bba6e bd236f195269a481"
			"e87eb3d444825ba4 424ee5b2d9efb595 d5a338f4c253f79d e9d04535206ca6db"
			"c2d4c9a1ec20849b 0db3fbe10c1446d5"));

  test_merkle (public_seed, secret_seed, 9, 0, UINT64_C(0x29877722d7c079), 0x156,
	       /* The message signed is the wots public key. */
	       SHEX ("99747c3547770fa288a628ed15122d3e"),
	       SHEX ("1be9523f2c90cd553ef5be5aa1c5c4fa"),
	       SHEX ("612d5bac915a3996 2cdbcacee0969dcf 8ecfb830cea2206c 37749c65b8f673db"
		     "090b1e2ade6c2a2f 349b5915103a3ac7 8482c39e99ffc462 6fb4cf4a116804ab"
		     "9d93d7104660fefa 0753cf875cb22fd6 0e55dc2f303de036 47712b12067a55f7"
		     "a467897bbed0d3a0 9d50e9deaadff78d e9ac65c1fd05d076 10a79c8c465141ad"
		     "65e60340531fab08 f1f433ef823283fe"));

  test_fors_gen (public_seed, secret_seed, 0, UINT64_C(0x29877722d7c079), 0x156, 0x203,
		 SHEX ("1ba66d6f782bdd2485589ea15d2b8ff0"),
		 SHEX ("4d9783fd544a53ee7a485ef229b35965"));
  test_fors_gen (public_seed, secret_seed, 0, UINT64_C(0x29877722d7c079), 0x156, 0,
		 SHEX ("be19da5abd01818bbcae2fc2d728c83b"),
		 SHEX ("40b0edc79104214adda356341b3950ab"));
  test_fors_gen (public_seed, secret_seed, 0, UINT64_C(0x29877722d7c079), 0x156, 1,
		 SHEX ("ed98099d2fd9d94ac48cae4c142a4c78"),
		 SHEX ("64fccb8a3cf088faeb39353aad5f624c"));
  test_fors_gen (public_seed, secret_seed, 0, UINT64_C(0x29877722d7c079), 0x156, 0x4e1e,
		 SHEX ("17f55905e41a6dc6e5bab2c9f0c1d5d3"),
		 SHEX ("15325ef3d2914cbd401327244cdb633d"));
  test_fors_sign (public_seed, secret_seed, &_slh_dsa_128s_params.fors,
		  0, UINT64_C(0x29877722d7c079), 0x156,
		  SHEX ("2033c1a4df6fc230c699522a21bed913"
			"0dda231526"),
		  SHEX ("3961b2cab15e08c633be827744a07f01"),
		  SHEX ("1ba66d6f782bdd24 85589ea15d2b8ff0 a00c06eaedc8c22d cb86f3df8b52a3bd"
			"144d4ed6f1167431 a95dc6018879b6b0 f9813797204ec2b0 558bad17b32e6dd9"
			"88086a032c0acbcf 2c1349ffc16c4af7 59365ff74afe4b8d c3fac5b2cda7ba65"
			"6c36c086e58468c0 1eddfc959fbdc853 2d75e79cf3374756 cc0491cfef555921"
			"ec8567bff0b6f216 ac4a4f200da63b5c 6d3a5c3273aa7a42 66adf083d3126103"
			"c73fe63a6e05e47e 8b9a520f00f32a69 7d0ff3a5ee840931 3773188f300b39e5"
			"b4967febf77c0f23 226785ee9dce335c efb1ce84b0673058 1bcef4d45f24aee4"
			"96a60bd4759b7b20 692241850eae1de7 0c7c4287b9f3b962 a66e0f23d1301b84"
			"48bb3dc545be0ef8 d0ec0be045d33ae4 b2dc0c5d002c2699 e8f49bf3bcb13676"
			"beefe11186a20a95 7027ac48ee6dd33b c0895df9847fd1c6 7a753777d21ac464"
			"2751139061cca836 99822c13567833b9 41fe5954bff0969a 4b20e0829d77e24e"
			"d0e02a00a2ce9f7e 64923fa61f0da1af dc5978dc063afeac b7108ee08aaa55b7"
			"11df00bbf1c71d69 8b389e6ad0ee2af4 fad1d8f8c87d53ee ac1f82a162a95cd5"
			"cce6dfc9908a1de3 3a2b26b41bbc4ffd a8e136879f10341a 713d62c107f3238c"
			"38693aff2e1fe15a dd8380671b2fac8f db3a4ffacd143f5a 00e21caccbe7d95d"
			"4c31c4daf7110529 de599fb6e8aa4f71 8c6172f4f10c4c1d d7310f8e44d18fb9"
			"bb6b906ae7ce973d eadfc82d6704762e d61165f6ca118313 4b6c834bf6b4e4ce"
			"19700bb54fb2d0f2 82b11ee5b7f68c72 cf32ff9e7d1356bd 53fdcce0d03c43d0"
			"ebf6f7d8841a16dd b49944d01374ecb9 45e6c5f0659d5f51 de0b27a834e2e7be"
			"a78a609da75d7f2c 6a40ba9110a2c331 9db7775bf6226b9a 8e324dc4411824a8"
			"8db95cc2fd96e4bc 24f1ecb6ce2b9293 020c28deacec1eb3 313d4e3dfd24b403"
			"686f16272cac3aca 95080257071a54f7 45ffb4708ff2d02d e94d7e9bf8d45f64"
			"5917c7135d6bc0a1 ca0a99bc4e33a689 07aa65a58b586c56 e1d81af6cb57fa5d"
			"56b3567687ecef53 2bb5aaaf2041b510 1538294296ae4c11 89a5100eb19b5531"
			"2016c575cbbb688f 20ba186dd48e4161 64c29b2eb7b59979 814b5a8e76553997"
			"99bf79eeab3ee76d 4c97df282265564f 2fa8971a1ecca0c4 6b59cc6ba253531c"
			"17ab7125cf2aad60 a120c7d4b631b1ba f187182c7d7582da 3232251215ffd6a2"
			"a55c627ba8d5cafe 761504d8341f293e 713987d6e0ca2eab 373c5131c2d38051"
			"c35b17918937b9fe e98382c277640de0 ccec45ba22d9d189 eea505a21c8594dd"
			"9b12e69a7faf58ed 269b718abeea4621 391d7fb4c6e0037b daf4a9ac73191674"
			"9e2a17d704cf5616 8d97c17b257e2483 16aa9da15d822ee3 c325bc0519173641"
			"7007ea82088618d7 531ffcef255b2de2 bf9fcaeb29d83e56 7a08dd3d3c229209"
			"af96ba71d8274fda e324702878d99ac0 5e990e0d6f34c879 d19279f57541f294"
			"96645cad4a636793 385b0a5dc21d5659 37fc36384dea4beb b5746c10748efcbd"
			"6b1925a74e3ac467 d7af456e0ba1e47a 2fab24e8311c14d8 40b499c9140e99a4"
			"993379b9b762b3ad c9499d5c86d07bc6 a159876a9962d8a4 43514e812f75c60d"
			"c50028388c627329 6e7208a3fa618256 2d10d7142a99da06 86ef8f05e564446c"
			"6bb32ffdee9edd13 aee58027d29e7195 48b67d75efe9581a 3374c66f65a1cd9e"
			"f9e98e6b57c40321 2739df6fd2de6c8a 39decc7cd33e37db 3a0f43296cb987e8"
			"756d4b29dc227733 bdee1d2f01679dab 92ce506e2fc77a70 798787b2e95be8e9"
			"bf80d0b64af8eaa6 ceda80fe85a0ceaf 81f335b99a1899a3 d9d609e7ba606eb2"
			"ababf2bbce1bc8f9 9eaf6074cf1c7e07 9896eb09827c16d0 cd4833377c46a337"
			"a7950b31b6566624 02e8ba838668a315 ac531315a9a56af1 8729ee25f53711c0"
			"9d25c173aa0e4d2b ec72db4b9cb4210d 52a8fb2f8b2671b1 ec711a4da8a357df"
			"bb0d2ec9734a50e1 db92352ace0f26f5 0cfb76fd17a08dec bb19c2417a9dc719"
			"f2ecac4a8e7c4827 5533def5c08788dc 4b47ec81960b25a5 7dba2762f5a07003"
			"7c50a4883fe902eb cb1574998dd5e8b1 e34ea5aea20bbbef fdb5d6163688e4e1"
			"bdc9619f12b78d20 e8c073f81da8bbe4 8bde8934bc7186da 9d29d1f670a322bf"
			"9febca92915e393c 1878895c04b8c365 e4d399ac551a55c5 4264e3fc6176cbcd"
			"101790863cdab395 74a4dd5c9edd69a0 1df20a10e5abff31 b4e204f5cf7e1dc9"
			"a27626ec3bf06d28 fad08c10674830d7 abc54772d95ace66 765757340007a353"
			"63d270f410a6bcf2 0f2ca54dfdb00d9b e8fa7ea5b79bf818 2f16b95f9850ce4c"
			"acff1e66bec202b5 7b85b37cdd2c3900 1d2950666368afa2 1de5ce68f54833a8"
			"8da17b49c4e66243 560ec61a6efd5d3a 2966a76df2dc08c4 e5f02f8b8cd71b90"
			"4ddd4bfd73a5c848 9b7eb813ca3da6b8 dbea536354e01428 dd6dc42db23257a2"
			"0e322f685bb82b20 f0edc48351c22b75 e0aa8adc567f172e 654360e094c19754"
			"2f39965bd9004621 c9ee3297870ed818 f980a71ec4a8f818 1e9be5be1ef6a660"
			"cbf68637e54b5afa bbc5f9dc61933014 cb52b4d2624a24ac a3c6f5ca80dd5aee"
			"93d0155af703c0ac a4a9266cd9b56f3f 152fc4fca8e7dce3 21a188682fb36e6f"
			"7a736fd4e9972a9f 71f11d50c351551e 3c455f1b051befcb c1fd83239b748951"
			"f7e18c2027627339 712df2772dcd57de 9a15f218e25a4493 ce20d039e2880881"
			"69445f244f14d56e 6efe9ed005094333 1a4ef297119cf5c0 e21e2bbc535daebf"
			"3fce3caf9d86b62c 37a4c9bd8991b8ff 01e992f26a77e987 ca8ddf6cf47d47d5"
			"439eb6622b241172 a8d5a251dcb5d4d2 26a68bef9d2e77df e4db3ebd4342f49b"
			"ee82b28fc35063e9 36589f86f8ff2db0 f2a7fcbf0d461484 184f64bf18e5bff6"
			"84545e6112f87662 60987bcfe76bed5a 17dfb88a9b7d7cac cb4283afb4ee21ef"
			"b43d698c413de813 48309bc1ec10cdb3 3a7e2e4aaed41cfe bf808b08e7f64f8f"
			"6f250960375c3a3e d0617000ac6e54a6 12727861daf4d893 7ae133a5e99c607e"
			"09e8097f876ef8cd 75e244b78eaabf83 1db9efd0ba405b52 715825974579a627"
			"9f7775ab87de6e26 9979530e3fff6d8f f6421ab3ba1ec61b 9ebc1a2a7aa59002"
			"ac916c26f55bc369 b2e11030f3346548 28285930228ad081 2500c822bd41ead4"
			"80b530331f8642f2 6d5454fe75cc3870 d807ef92496b27e0 45b3317f10e98533"
			"59875ec041117f3b b37d88c526ef1a34 2b6ea289fa69bc91 4d8fef84a27329f3"
			"0a7326c84710f972 5432a525f3bf9af3 d93f9faded5766f9 067a5b1b7a0dac92"
			"75207b6776c404b1 7801a7372666f153 78cdf91bb4c29d6a cf79eed16918947c"
			"769283e829ec1e97 cb90630473224d88 95f2a0219d309507 173f42594372696f"
			"6ef8468b843d4ad5 81ad78c221bbb877 0ca2323858016dc6 f9c311bd451a5b68"
			"ce23c6feb8c1f543 82a8512d286e6bd5 62ada1c6c8c7c46d 7a9722d7a909b7cc"
			"fce3258bb37b78c0 d076e4bb587bfe05 95257c988543edeb d2f24f9e124dd0e3"
			"35ea2add17201df7 f2e68fbcc02da7d4 3b7a9a8f83de7375 be2c61b4c2b872bb"
			"de25ea659a59b1a6 3cbc9c5efb6c449d 9818245291c6c232 17ae6cb018cdf7a9"
			"a49240f37a484361 b450ba8fccedd4f4 556ca8423fd1e907 6a876306958ee264"
			"4646633c2777280a c7a82e441d79b556 c629d7c97b4c7895 4bae0e76cb4ab1b2"
			"0b51126ac8f125e2 f01c266df31b2ae6 d50eb02f96b39044 81a32254799bc233"
			"88f7d86b6b60876d 20cf9e8a4468fb3e be4883fb90765a50 5d6ae99827a0ff96"
			"d5eb284ac7df815c 0fd5aa2bdffa560b dc37beb9a7a6a4e3 fc074a9f812132a8"
			"6be3a1f73433a198 0a168bbe54910ff5 95a47b6747f43a67 8fe5a7c96e636b4b"
			"874f348d24b79337 db4315cb10fd0e56 2431511c323353cf 1e59fd5a55357e5f"
			"6b7cce60f1f8211f d1f5be68f7c8bd70 c29f03c0a6613c64 dd10a65db5e0c546"
			"f5382403ff8ba36b ad49879231912a4b 219a08a19858b12c 2744fd65603775b5"
			"6bf4459512e79188 92da55f87d7cc02c 6885c0ec02550b60 9e3fa7d9fb0d13ab"));

  test_xmss_sign (public_seed, secret_seed, 9, 0, UINT64_C(0x29877722d7c079), 0x156,
		  /* The message signed is a fors public key. */
		  SHEX ("3961b2cab15e08c633be827744a07f01"),
		  SHEX ("1be9523f2c90cd553ef5be5aa1c5c4fa"),
		  SHEX (/* Embedded wots signature. */
			"e1933de10e3fface 5fb8f8707c35ac13 74dc14ee8518481c 7e63d936ecc62f50"
			"c7f951b87bc716dc 45e9bcfec6f6d97e 7fafdacb6db05ed3 778f21851f325e25"
			"470da8dd81c41223 6d66cbee9ffa9c50 b86aa40baf213494 dfacca22aa0fb479"
			"53928735ca4212cf 53a09ab0335d20a8 e62ede797c8e7493 54d636f15f3150c5"
			"52797b76c091a41f 949f7fb57b42f744 1cca410264d6421f 4aa2c7e2ff4834a8"
			"db0e6e7750b2e11f f1c89a42d1fbc271 8358e38325886ad1 2346cd694f9eab73"
			"46c9a23b5ebe7637 bfd834a412318b01 188b0f29e3bd979f 8ae734acf1563af3"
			"03d3c095e9eaeba3 5207b9df3acf9ee4 7da5c1e2652f3b86 41698f3d2260591b"
			"07d00565e5d6be18 36033d2b7ef2c33b dc5cf3bba95b42df 6f73345b835341b2"
			"50e2862c9f2f9cef 77cfa74cfb04c560 d8a0038c4e96cb0d a2b3e9b2cd3cecf5"
			"22fda0d67e5f62b2 ee23bd42a61c7da4 8f0ea30b81af7ccb 6bb02cde272d2574"
			"1325e9d91535615c 0184f2d7f226141d 79b42412721fd345 61d93663650b3c1b"
			"6901872bc4c0bb15 bcd9038950b7717f 7f448b6126592076 a2bad2d63c55399c"
			"243fdbdb0c8d676b 2ae455e7f0a9b18d 3fc889c43387f2cb c4dc73d7c85bfab6"
			"b4b04463a3dd359c 3a8f61bfa6c4b042 4aeba4dd8a95ec12 43b2e36c29f82e1d"
			"711281599b3e05e7 5492ae3425eaa7f1 4ff8c6a9630bba6e bd236f195269a481"
			"e87eb3d444825ba4 424ee5b2d9efb595 d5a338f4c253f79d e9d04535206ca6db"
			"c2d4c9a1ec20849b 0db3fbe10c1446d5"
			/* Auth path aka inclusion proof. */
			"612d5bac915a3996 2cdbcacee0969dcf 8ecfb830cea2206c 37749c65b8f673db"
			"090b1e2ade6c2a2f 349b5915103a3ac7 8482c39e99ffc462 6fb4cf4a116804ab"
			"9d93d7104660fefa 0753cf875cb22fd6 0e55dc2f303de036 47712b12067a55f7"
			"a467897bbed0d3a0 9d50e9deaadff78d e9ac65c1fd05d076 10a79c8c465141ad"
			"65e60340531fab08 f1f433ef823283fe"));

  test_slh_dsa_128_root (slh_dsa_shake_128s_root, public_seed, secret_seed,
			 SHEX ("ac524902fc81f503 2bc27b17d9261ebd"));

  /* From
     https://github.com/usnistgov/ACVP-Server/blob/master/gen-val/json-files/SLH-DSA-keyGen-FIPS205/internalProjection.json */
  test_slh_dsa_128_root (slh_dsa_sha2_128s_root, /* tcId 1 */
			 SHEX ("0D794777914C99766827F0F09CA972BE"),
			 SHEX ("173D04C938C1C36BF289C3C022D04B14"),
			 SHEX ("0162C10219D422ADBA1359E6AA65299C"));

  test_slh_dsa_128_root (slh_dsa_sha2_128f_root, /* tcId 31 */
			 SHEX ("A868F1BD5DEBC12D4C9FAD66AABD0A94"),
			 SHEX ("C42BCB3B5A6F331F5CCE899253C6D9E2"),
			 SHEX ("B546DF247BE4C457F3D467CDFCFABD39"));

  test_slh_dsa_128_root (slh_dsa_shake_128s_root, /* tcId 11 */
			 SHEX ("529FFE86200D1F32C2B60D0CD909F190"),
			 SHEX ("C151951F3811029239B74ADD24C506AF"),
			 SHEX ("0761F9B727AFA724B47223016BB5B2BA"));

  test_slh_dsa_128_root (slh_dsa_shake_128s_root, /* tcId 12 */
			 SHEX ("B64302C8D20FB89AA2414307D44E1F9C"),
			 SHEX ("D3ADF41FF57EED108BEF2D8733F4C2B0"),
			 SHEX ("6EFA39EBBA94B0633C900644B81DE2B9"));

  test_slh_dsa_128_root (slh_dsa_shake_128f_root, /* tcId 31 */
			 SHEX ("56505C229F4E7FA6B201714C7DCC9DA3"),
			 SHEX ("3956AB391B4D22FC907AF0740326D061"),
			 SHEX ("66578F1F24C3FE371C97C14CE0E79CDC"));

  test_slh_dsa_128_root (slh_dsa_shake_128f_root, /* tcId 32 */
			 SHEX ("F8B2314A9ABB09E72509F14A742035BA"),
			 SHEX ("57250E2880AF25BC0D8DBA76A8FBB666"),
			 SHEX ("6B5F4A0CC172672BBE8DF3F86CB58F51"));

  /* If we mark the private key for the top-level
     slh_dsa_shake_128s_sign call as undefined, then we get valgrind
     errors from the branches in wots_chain, when signing the derived
     public keys. We'd need further instrumentation to make such a
     test work. */
  if (test_side_channel)
    return;

  /* Test vector from
     https://github.com/smuellerDD/leancrypto/blob/master/slh-dsa/tests/sphincs_tester_vectors_shake_128s.h */
  test_slh_dsa (&slh_dsa_shake_128s,
		SHEX ("B505D7CFAD1B4974 99323C8686325E47"
		      "AC524902FC81F503 2BC27B17D9261EBD"),
		SHEX ("7C9935A0B07694AA 0C6D10E4DB6B1ADD"
		      "2FD81A25CCB14803 2DCD739936737F2D"),
		SHEX ("D81C4D8D734FCBFB EADE3D3F8A039FAA"
		      "2A2C9957E835AD55 B22E75BF57BB556A"
		      "C8"),
		read_hex_file ("slh-dsa-shake-128s.ref", SLH_DSA_128S_SIGNATURE_SIZE));

  /* Test vector from
     https://github.com/smuellerDD/leancrypto/blob/master/slh-dsa/tests/sphincs_tester_vectors_shake_128f.h */
  test_slh_dsa (&slh_dsa_shake_128f,
		SHEX ("B505D7CFAD1B4974 99323C8686325E47"
		      "AFBC007BA1E2B4A1 38F03AA9A6195AC8"),
		SHEX ("7C9935A0B07694AA 0C6D10E4DB6B1ADD"
		      "2FD81A25CCB14803 2DCD739936737F2D"),
		SHEX ("D81C4D8D734FCBFB EADE3D3F8A039FAA"
		      "2A2C9957E835AD55 B22E75BF57BB556A"
		      "C8"),
		read_hex_file ("slh-dsa-shake-128f.ref", SLH_DSA_128F_SIGNATURE_SIZE));

  /* From
     https://github.com/usnistgov/ACVP-Server/blob/master/gen-val/json-files/SLH-DSA-sigGen-FIPS205/internalProjection.json */
  test_slh_dsa (&slh_dsa_sha2_128f, /* tcId 7 */
		SHEX ("0C04FABC4FCA7F356AC36C28B99D7A1FCFEF78F38B167CA9D0AB8772910C3945"),
		SHEX ("704555B4E5DD1B979A4C3B7A0A0E4EE241D59AE0779CAF0DF58300F21066DDA7"),
		read_hex_file ("slh-dsa-sha2-128f-tc7.msg", 5024),
		read_hex_file ("slh-dsa-sha2-128f-tc7.sig", SLH_DSA_128F_SIGNATURE_SIZE));

  test_slh_dsa (&slh_dsa_shake_128f, /* tcId 64 */
		SHEX ("C9A7900E931AFBA2B52A5BC55A2DC4D12DDC9BF8E0B2ED0BDE83E674F1ECE7AA"),
		SHEX ("0E87FF20256E0E499A53B52DF91467C01F0431C07250AFE93DE814117B5D66D3"),
		read_hex_file ("slh-dsa-shake-128f-tc64.msg", 2280),
		read_hex_file ("slh-dsa-shake-128f-tc64.sig", SLH_DSA_128F_SIGNATURE_SIZE));

  test_slh_dsa (&slh_dsa_sha2_128s, /* tcId 162 */
		SHEX ("0FD12C3F990748CF9B1426413B64128EDF9242E50B9E29378BD24CAD4D547540"),
		SHEX ("438E444071BD643C2407BD9FEB0071EC21DAA14113518133D6161EF420EE629D"),
		read_hex_file ("slh-dsa-sha2-128s-tc162.msg", 5746),
		read_hex_file ("slh-dsa-sha2-128s-tc162.sig", SLH_DSA_128S_SIGNATURE_SIZE));

  test_slh_dsa (&slh_dsa_shake_128s, /* tcId 215 */
		SHEX ("DD286FF370CB50BC1B23894AA3F7025A534A788E697B94942AB845EFB753A30B"),
		SHEX ("4738AC60C561FFBE15AB96EFFA1A09291A79332E1CA3C38B2FEF40ACA7CFE285"),
		read_hex_file ("slh-dsa-shake-128s-tc215.msg", 5377),
		read_hex_file ("slh-dsa-shake-128s-tc215.sig", SLH_DSA_128S_SIGNATURE_SIZE));
}
