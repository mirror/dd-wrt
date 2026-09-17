/*
 * ProFTPD - mod_sftp OpenSSL provider
 * Copyright (c) 2026 TJ Saunders
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 * As a special exemption, TJ Saunders and other respective copyright holders
 * give permission to link this program with OpenSSL, and distribute the
 * resulting executable, without including the source code for OpenSSL in the
 * source distribution.
 */

#include "mod_sftp.h"
#include "crypto.h"
#include "provider.h"
#include "umac.h"

#if OPENSSL_VERSION_NUMBER >= 0x40000000L && !defined(HAVE_LIBRESSL)
# include <openssl/core.h>
# include <openssl/core_dispatch.h>
# include <openssl/core_names.h>
# include <openssl/params.h>
# include <openssl/provider.h>

static OSSL_PROVIDER *umac_provider = NULL;

static const char *trace_channel = "ssh2";

/* Our custom algorithm provider implementation. */

/* UMAC */

typedef struct umac_ctx_st {
  struct umac_ctx *umac;
} UMAC_CTX;

static void *umac_ctx_new(void *vctx) {
  UMAC_CTX *ctx;

  ctx = OPENSSL_zalloc(sizeof(UMAC_CTX));
  return ctx;
}

static void umac_ctx_free(void *vctx) {
  UMAC_CTX *ctx;

  ctx = vctx;
  if (ctx->umac != NULL) {
    umac_delete(ctx->umac);
    ctx->umac = NULL;
  }

  OPENSSL_free(ctx);
}

/* The Provider interface for digests expects an "init" callback, even though
 * it is not functionally needed for our situation.
 */
static int umac_md_init(void *vctx) {
  (void) vctx;

  return 1;
}

static const OSSL_PARAM umac_params[] = {
  OSSL_PARAM_size_t(OSSL_DIGEST_PARAM_BLOCK_SIZE, NULL),
  OSSL_PARAM_size_t(OSSL_DIGEST_PARAM_SIZE, NULL),
  OSSL_PARAM_END
};

/* UMAC64 */

static int umac64_md_update(void *vctx, const unsigned char *data, size_t len) {
  UMAC_CTX *ctx;
  struct umac_ctx *umac;

  ctx = vctx;
  umac = ctx->umac;

  /* The allocation of the umac_ctx is deliberately delayed until the first
   * update, since the computation of keys depends on the initial bytes
   * provided.
   */
  if (umac == NULL) {
    umac = umac_new((unsigned char *) data);
    if (umac == NULL) {
      return 0;
    }

    ctx->umac = umac;
    return 1;
  }

  return umac_update(umac, (unsigned char *) data, (long) len);
}

static int umac64_md_final(void *vctx, unsigned char *out, size_t *out_len,
    size_t outsz) {
  int res = 1;
  struct umac_ctx *ctx;
  unsigned char nonce[8];

  ctx = vctx;

  *out_len = outsz;

  if (outsz != 0) {
    res = umac_final(ctx, out, nonce);
  }

  return res;
}

static int umac64_get_params(void *provctx, OSSL_PARAM params[]) {
  OSSL_PARAM *p;
  int ok = 1;

  p = OSSL_PARAM_locate(params, OSSL_DIGEST_PARAM_BLOCK_SIZE);
  if (p != NULL) {
    if (OSSL_PARAM_set_size_t(p, 32) != 1) {
      ok = 0;
    }
  }

  if (ok == 1) {
    p = OSSL_PARAM_locate(params, OSSL_DIGEST_PARAM_SIZE);
    if (p != NULL) {
      if (OSSL_PARAM_set_size_t(p, 8) != 1) {
        ok = 0;
      }
    }
  }

  return ok;
}

static const OSSL_PARAM *umac64_gettable_params(void) {
  return umac_params;
}

static const OSSL_DISPATCH umac64_functions[] = {
  { OSSL_FUNC_DIGEST_NEWCTX, (void (*)(void)) umac_ctx_new },
  { OSSL_FUNC_DIGEST_FREECTX, (void (*)(void)) umac_ctx_free },
  { OSSL_FUNC_DIGEST_INIT, (void (*)(void)) umac_md_init },
  { OSSL_FUNC_DIGEST_UPDATE, (void (*)(void)) umac64_md_update },
  { OSSL_FUNC_DIGEST_FINAL, (void (*)(void)) umac64_md_final },
  { OSSL_FUNC_DIGEST_GET_PARAMS, (void (*)(void)) umac64_get_params },
  { OSSL_FUNC_DIGEST_GETTABLE_PARAMS, (void (*)(void)) umac64_gettable_params },

  { 0, NULL }
};

/* UMAC128 */

static int umac128_md_update(void *vctx, const unsigned char *data,
    size_t len) {
  UMAC_CTX *ctx;
  struct umac_ctx *umac;

  ctx = vctx;
  umac = ctx->umac;

  /* The allocation of the umac_ctx is deliberately delayed until the first
   * update, since the computation of keys depends on the initial bytes
   * provided.
   */
  if (umac == NULL) {
    umac = umac128_new((unsigned char *) data);
    if (umac == NULL) {
      return 0;
    }

    ctx->umac = umac;
    return 1;
  }

  return umac128_update(umac, (unsigned char *) data, (long) len);
}

static int umac128_md_final(void *vctx, unsigned char *out, size_t *out_len,
    size_t outsz) {
  int res = 1;
  struct umac_ctx *ctx;
  unsigned char nonce[8];

  ctx = vctx;

  *out_len = outsz;

  if (outsz != 0) {
    res = umac128_final(ctx, out, nonce);
  }

  return res;
}

static int umac128_get_params(void *provctx, OSSL_PARAM params[]) {
  OSSL_PARAM *p;
  int ok = 1;

  p = OSSL_PARAM_locate(params, OSSL_DIGEST_PARAM_BLOCK_SIZE);
  if (p != NULL) {
    if (OSSL_PARAM_set_size_t(p, 64) != 1) {
      ok = 0;
    }
  }

  if (ok == 1) {
    p = OSSL_PARAM_locate(params, OSSL_DIGEST_PARAM_SIZE);
    if (p != NULL) {
      if (OSSL_PARAM_set_size_t(p, 16) != 1) {
        ok = 0;
      }
    }
  }

  return ok;
}

static const OSSL_PARAM *umac128_gettable_params(void) {
  return umac_params;
}

static const OSSL_DISPATCH umac128_functions[] = {
  { OSSL_FUNC_DIGEST_NEWCTX, (void (*)(void)) umac_ctx_new },
  { OSSL_FUNC_DIGEST_FREECTX, (void (*)(void)) umac_ctx_free },
  { OSSL_FUNC_DIGEST_INIT, (void (*)(void)) umac_md_init },
  { OSSL_FUNC_DIGEST_UPDATE, (void (*)(void)) umac128_md_update },
  { OSSL_FUNC_DIGEST_FINAL, (void (*)(void)) umac128_md_final },
  { OSSL_FUNC_DIGEST_GET_PARAMS, (void (*)(void)) umac128_get_params },
  { OSSL_FUNC_DIGEST_GETTABLE_PARAMS, (void (*)(void)) umac128_gettable_params },

  { 0, NULL }
};

static const OSSL_ALGORITHM umac_digests[] = {
  { "umac64", NULL, umac64_functions },
  { "umac128", NULL, umac128_functions },

  { NULL, NULL, NULL }
};

static const OSSL_ALGORITHM *umac_provider_operations(void *provctx,
    int operation_id, int *no_cache) {
  *no_cache = 0;

  if (operation_id == OSSL_OP_DIGEST) {
    return umac_digests;
  }

  return NULL;
}

static const OSSL_DISPATCH umac_provider_functions[] = {
  { OSSL_FUNC_PROVIDER_QUERY_OPERATION, (void (*)(void)) umac_provider_operations },

  { 0, NULL }
};

static int umac_provider_init(const OSSL_CORE_HANDLE *core,
    const OSSL_DISPATCH *in, const OSSL_DISPATCH **out, void **provctx) {
  *out = umac_provider_functions;
  *provctx = (void *) core;

  return 1;
}

#endif /* OpenSSL 4.x and later */

int sftp_provider_init(void) {
#if OPENSSL_VERSION_NUMBER >= 0x40000000L && !defined(HAVE_LIBRESSL)
  if (OSSL_PROVIDER_add_builtin(NULL, "umac", umac_provider_init) != 1) {
    pr_log_debug(DEBUG1, MOD_SFTP_VERSION
      ": error registering 'umac' OpenSSL provider: %s",
      sftp_crypto_get_errors());

  } else {
    pr_trace_msg(trace_channel, 9, "%s", "registered 'umac' OpenSSL provider");
  }

  /* Load our custom OpenSSL algorithm provider. */
  umac_provider = OSSL_PROVIDER_load(NULL, "umac");
  if (umac_provider == NULL) {
    pr_log_pri(PR_LOG_NOTICE, MOD_SFTP_VERSION
      ": error loading 'umac' OpenSSL provider: %s", sftp_crypto_get_errors());
 
  } else {
    pr_trace_msg(trace_channel, 9, "%s", "loaded 'umac' OpenSSL provider");
  }
#endif /* OpenSSL 4.x and later */

  return 0;
}

void sftp_provider_free(void) {
#if OPENSSL_VERSION_NUMBER >= 0x40000000L && !defined(HAVE_LIBRESSL)
  if (umac_provider != NULL) { 
    OSSL_PROVIDER_unload(umac_provider);
    umac_provider = NULL;
  }
#endif /* OpenSSL 4.x and later */
}
