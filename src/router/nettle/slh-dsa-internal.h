/* slh-dsa-internal.h

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

#ifndef NETTLE_SLH_DSA_INTERNAL_H_INCLUDED
#define NETTLE_SLH_DSA_INTERNAL_H_INCLUDED

#include <stdint.h>

#include "nettle-types.h"

/* Name mangling */
#define _wots_gen _nettle_wots_gen
#define _wots_sign _nettle_wots_sign
#define _wots_verify _nettle_wots_verify
#define _merkle_root _nettle_merkle_root
#define _merkle_sign _nettle_merkle_sign
#define _merkle_verify _nettle_merkle_verify
#define _fors_gen _nettle_fors_gen
#define _fors_sign _nettle_fors_sign
#define _fors_verify _nettle_fors_verify
#define _xmss_gen _nettle_xmss_gen
#define _xmss_sign _nettle_xmss_sign
#define _xmss_verify _nettle_xmss_verify
#define _slh_dsa_pure_digest _nettle_slh_dsa_pure_digest
#define _slh_dsa_pure_rdigest _nettle_slh_dsa_pure_rdigest
#define _slh_dsa_sign _nettle_slh_dsa_sign
#define _slh_dsa_verify _nettle_slh_dsa_verify

#define _slh_dsa_128s_params _nettle_slh_dsa_128s_params
#define _slh_dsa_128f_params _nettle_slh_dsa_128f_params

#define _slh_hash_shake _nettle_slh_hash_shake
#define _slh_hash_sha256 _nettle_slh_hash_sha256

/* Size of a single hash, including the seed and prf parameters */
#define _SLH_DSA_128_SIZE 16

/* Fields always big-endian */
struct slh_address_hash
{
  uint32_t type;
  uint32_t keypair;
  /* height for XMSS_TREE and FORS_TREE, chain address for WOTS_HASH. */
  uint32_t height_chain;
  /* index for XMSS_TREE and FORS_TREE, hash address for WOTS_HASH. */
  uint32_t index_hash;
};

enum slh_addr_type
  {
    SLH_WOTS_HASH = 0,
    SLH_WOTS_PK = 1,
    SLH_XMSS_TREE = 2,
    SLH_FORS_TREE = 3,
    SLH_FORS_ROOTS = 4,
    SLH_WOTS_PRF = 5,
    SLH_FORS_PRF = 6,
  };

/* Compute the randomizer for a prefix + message, from secret PRF. */
typedef void slh_hash_randomizer_func (const uint8_t *public_seed, const uint8_t *secret_prf,
				       size_t prefix_length, const uint8_t *prefix,
				       size_t msg_length, const uint8_t *msg,
				       uint8_t *randomizer);

/* Compute the message digest, with the randomizer and public key
   (both seed and root) as input. */
typedef void slh_hash_msg_digest_func (const uint8_t *randomizer, const uint8_t *pub,
				       size_t prefix_length, const uint8_t *prefix,
				       size_t msg_length, const uint8_t *msg,
				       size_t digest_size, uint8_t *digest);

/* Initialize context with public seed and first part of the ADRS. */
typedef void slh_hash_init_tree_func (void *tree_ctx, const uint8_t *public_seed,
				      uint32_t layer, uint64_t tree_idx);
/* Initialize a new context starting from the tree_ctx, extending it
   with the rest of the ADRS. */
typedef void slh_hash_init_hash_func (const void *tree_ctx, void *ctx,
				      const struct slh_address_hash *ah);
/* Initialize a temporary context like above _init_hash, and hash a
   single value, e.g., the secret seed or a secret wots value. */
typedef void slh_hash_secret_func (const void *tree_ctx,
				   const struct slh_address_hash *ah,
				   const uint8_t *secret, uint8_t *out);
/* Initialize a temporary context like above _init_hash, and hash two
   values: The left and right child hashes of a merkle tree node. */
typedef void slh_hash_node_func (const void *tree_ctx,
				 const struct slh_address_hash *ah,
				 const uint8_t *left, const uint8_t *right,
				 uint8_t *out);

struct slh_hash
{
  slh_hash_init_tree_func *init_tree;
  slh_hash_init_hash_func *init_hash;
  nettle_hash_update_func *update;
  nettle_hash_digest_func *digest;
  slh_hash_secret_func *secret;
  slh_hash_node_func *node;
  slh_hash_randomizer_func *randomizer;
  slh_hash_msg_digest_func *msg_digest;
};

extern const struct slh_hash _slh_hash_shake;  /* For sha3_ctx. */
extern const struct slh_hash _slh_hash_sha256; /* For sha256_ctx. */

struct slh_merkle_ctx_public
{
  const struct slh_hash *hash;
  /* Initialized based on public seed and slh_address_tree. */
  const void *tree_ctx;
  unsigned keypair; /* Used only by fors_leaf and fors_node. */
};

struct slh_merkle_ctx_secret
{
  struct slh_merkle_ctx_public pub;
  const uint8_t *secret_seed;
  /* Scratch hashing context, used only by xmss_leaf and _xmss_sign
     (where it is passed on to wots operations). */
  void *scratch_ctx;
};

struct slh_xmss_params
{
  unsigned short d; /* Levels of xmss trees. */
  unsigned short h; /* Height of each tree. */
  unsigned short signature_size;
};

struct slh_fors_params
{
  unsigned short a; /* Height of tree. */
  unsigned short k; /* Number of trees. */
  unsigned short msg_size;
  unsigned short signature_size;
};

typedef void slh_parse_digest_func (const uint8_t *digest, uint64_t *tree_idx, unsigned *leaf_idx);

struct slh_dsa_params
{
  slh_parse_digest_func *parse_digest;
  struct slh_xmss_params xmss;
  struct slh_fors_params fors;
};

extern const struct slh_dsa_params _slh_dsa_128s_params;
extern const struct slh_dsa_params _slh_dsa_128f_params;

#define _WOTS_SIGNATURE_LENGTH 35
/* 560 bytes */
#define WOTS_SIGNATURE_SIZE (_WOTS_SIGNATURE_LENGTH*_SLH_DSA_128_SIZE)

void
_wots_gen (const struct slh_hash *hash, const void *tree_ctx,
	   const uint8_t *secret_seed,
	   uint32_t keypair, uint8_t *pub,
	   /* Allocated by caller, initialized and clobbered by callee. */
	   void *pub_ctx);

void
_wots_sign (const struct slh_hash *hash, const void *tree_ctx,
	    const uint8_t *secret_seed,
	    unsigned keypair, const uint8_t *msg,
	    uint8_t *signature, uint8_t *pub,
	    /* Allocated by caller, initialized and clobbered by callee. */
	    void *pub_ctx);

/* Computes candidate public key from signature. */
void
_wots_verify (const struct slh_hash *hash, const void *tree_ctx,
	      unsigned keypair, const uint8_t *msg, const uint8_t *signature, uint8_t *pub,
	      /* Allocated by caller, initialized and clobbered by callee. */
	      void *pub_ctx);

/* Merkle tree functions. Could be generalized for other merkle tree
   applications, by using const void* for the ctx argument. */
typedef void merkle_leaf_hash_func (const struct slh_merkle_ctx_secret *ctx, unsigned index, uint8_t *out);
typedef void merkle_node_hash_func (const struct slh_merkle_ctx_public *ctx, unsigned height, unsigned index,
				    const uint8_t *left, const uint8_t *right, uint8_t *out);

void
_merkle_root (const struct slh_merkle_ctx_secret *ctx,
	      merkle_leaf_hash_func *leaf_hash, merkle_node_hash_func *node_hash,
	      unsigned height, unsigned start, uint8_t *root,
	      /* Must have space for (height + 1) node hashes */
	      uint8_t *stack);

void
_merkle_sign (const struct slh_merkle_ctx_secret *ctx,
	      merkle_leaf_hash_func *leaf_hash, merkle_node_hash_func *node_hash,
	      unsigned height, unsigned idx, uint8_t *signature);

/* The hash argument is both input (leaf hash to be verified) and output (resulting root hash). */
void
_merkle_verify (const struct slh_merkle_ctx_public *ctx, merkle_node_hash_func *node_hash,
		unsigned height, unsigned idx, const uint8_t *signature, uint8_t *hash);

#define FORS_SIGNATURE_SIZE(a, k) ((k) * ((a) + 1) * _SLH_DSA_128_SIZE)

/* Generates a single secret value, and corresponding leaf hash. */
void
_fors_gen (const struct slh_merkle_ctx_secret *ctx, unsigned index, uint8_t *sk, uint8_t *leaf);

/* Computes a fors signature as well as the public key. */
void
_fors_sign (const struct slh_merkle_ctx_secret *ctx,
	    const struct slh_fors_params *fors,
	    const uint8_t *msg, uint8_t *signature, uint8_t *pub,
	    /* Allocated by caller, initialized and clobbered by callee. */
	    void *pub_ctx);

/* Computes candidate public key from signature. */
void
_fors_verify (const struct slh_merkle_ctx_public *ctx,
	      const struct slh_fors_params *fors,
	      const uint8_t *msg, const uint8_t *signature, uint8_t *pub,
	      /* Allocated by caller, initialized and clobbered by callee. */
	      void *pub_ctx);

/* Just the auth path, excluding the wots signature, 144 bytes. */
#define XMSS_AUTH_SIZE(h) ((h) * _SLH_DSA_128_SIZE)
#define XMSS_SIGNATURE_SIZE(h) (WOTS_SIGNATURE_SIZE + XMSS_AUTH_SIZE (h))

/* Provided scratch must be of size (xmss->h + 1) * _SLH_DSA_128_SIZE. */
void
_xmss_gen (const struct slh_hash *hash,
	   const uint8_t *public_seed, const uint8_t *secret_seed,
	   const struct slh_xmss_params *xmss, uint8_t *root,
	   void *tree_ctx, void *scratch_ctx, uint8_t *scratch);

/* Signs using wots, then signs wots public key using xmss. Also
   returns the xmss public key (i.e., root hash).*/
void
_xmss_sign (const struct slh_merkle_ctx_secret *ctx, unsigned h,
	    unsigned idx, const uint8_t *msg, uint8_t *signature, uint8_t *pub);

void
_xmss_verify (const struct slh_merkle_ctx_public *ctx, unsigned h,
	      unsigned idx, const uint8_t *msg, const uint8_t *signature, uint8_t *pub,
	      /* Allocated by caller, initialized and clobbered by callee. */
	      void *scratch_ctx);

void
_slh_dsa_pure_digest (const struct slh_hash *hash,
		      const uint8_t *pub,
		      size_t length, const uint8_t *msg,
		      const uint8_t *randomizer, size_t digest_size, uint8_t *digest);

void
_slh_dsa_pure_rdigest (const struct slh_hash *hash,
		       const uint8_t *pub, const uint8_t *prf,
		       size_t length, const uint8_t *msg,
		       uint8_t *randomizer, size_t digest_size, uint8_t *digest);

void
_slh_dsa_sign (const struct slh_dsa_params *params,
	       const struct slh_hash *hash,
	       const uint8_t *pub, const uint8_t *priv,
	       const uint8_t *digest, uint8_t *signature,
	       void *tree_ctx, void *scratch_ctx);
int
_slh_dsa_verify (const struct slh_dsa_params *params,
		 const struct slh_hash *hash,
		 const uint8_t *pub,
		 const uint8_t *digest, const uint8_t *signature,
		 void *tree_ctx, void *scratch_ctx);


#endif /* NETTLE_SLH_DSA_INTERNAL_H_INCLUDED */
