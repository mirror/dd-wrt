// SPDX-License-Identifier: Apache-2.0 AND MIT

#include "test_common.h"

#include <string.h>

void hexdump(const void *ptr, size_t len) {
    const unsigned char *p = ptr;
    size_t i, j;

    for (i = 0; i < len; i += j) {
        for (j = 0; j < 16 && i + j < len; j++)
            printf("%s%02x", j ? "" : " ", p[i + j]);
    }
    printf("\n");
}

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
int alg_is_enabled(const char *algname) {
    char *alglist = getenv("OQS_SKIP_TESTS");
    char *comma = NULL;
    char totest[200];

    if (alglist == NULL)
        return 1;

    while ((comma = strchr(alglist, ','))) {
        memcpy(totest, alglist, MIN(200, comma - alglist));
        totest[comma - alglist] = '\0';
        if (strstr(algname, totest))
            return 0;
        alglist = comma + 1;
    }
    return strstr(algname, alglist) == NULL;
}

OSSL_PROVIDER *load_default_provider(OSSL_LIB_CTX *libctx) {
    OSSL_PROVIDER *provider;
    T((provider = OSSL_PROVIDER_load(libctx, "default")));
    return provider;
}

#ifdef OQS_PROVIDER_STATIC
#define OQS_PROVIDER_ENTRYPOINT_NAME oqs_provider_init
#else
#define OQS_PROVIDER_ENTRYPOINT_NAME OSSL_provider_init
#endif // ifdef OQS_PROVIDER_STATIC

#ifndef OQS_PROVIDER_STATIC

/* Loads the oqs-provider from a shared module (.so). */
void load_oqs_provider(OSSL_LIB_CTX *libctx, const char *modulename,
                       const char *configfile) {
    T(OSSL_LIB_CTX_load_config(libctx, configfile));
    T(OSSL_PROVIDER_available(libctx, modulename));
}

#else

extern OSSL_provider_init_fn OQS_PROVIDER_ENTRYPOINT_NAME;

/* Loads the statically linked oqs-provider. */
void load_oqs_provider(OSSL_LIB_CTX *libctx, const char *modulename,
                       const char *configfile) {
    (void)configfile;
    T(OSSL_PROVIDER_add_builtin(libctx, modulename,
                                OQS_PROVIDER_ENTRYPOINT_NAME));
    T(OSSL_PROVIDER_load(libctx, "default"));
}

#endif // ifndef OQS_PROVIDER_STATIC
