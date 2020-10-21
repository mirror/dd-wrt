/**
 * @file context.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief context implementation for libyang
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>

#include "common.h"
#include "context.h"
#include "hash_table.h"
#include "parser.h"
#include "tree_internal.h"
#include "resolve.h"

/*
 * counter for references to the extensions plugins (for the number of contexts)
 * located in extensions.c
 */
extern unsigned int ext_plugins_ref;

#define IETF_YANG_METADATA_PATH "../models/ietf-yang-metadata@2016-08-05.h"
#define YANG_PATH "../models/yang@2017-02-20.h"
#define IETF_INET_TYPES_PATH "../models/ietf-inet-types@2013-07-15.h"
#define IETF_YANG_TYPES_PATH "../models/ietf-yang-types@2013-07-15.h"
#define IETF_DATASTORES "../models/ietf-datastores@2018-02-14.h"
#define IETF_YANG_LIB_PATH "../models/ietf-yang-library@2019-01-04.h"
#define IETF_YANG_LIB_REV "2019-01-04"

#include IETF_YANG_METADATA_PATH
#include YANG_PATH
#include IETF_INET_TYPES_PATH
#include IETF_YANG_TYPES_PATH
#include IETF_DATASTORES
#include IETF_YANG_LIB_PATH

#define LY_INTERNAL_MODULE_COUNT 6
static struct internal_modules_s {
    const char *name;
    const char *revision;
    const char *data;
    uint8_t implemented;
    LYS_INFORMAT format;
} internal_modules[LY_INTERNAL_MODULE_COUNT] = {
    {"ietf-yang-metadata", "2016-08-05", (const char*)ietf_yang_metadata_2016_08_05_yin, 0, LYS_IN_YIN},
    {"yang", "2017-02-20", (const char*)yang_2017_02_20_yin, 1, LYS_IN_YIN},
    {"ietf-inet-types", "2013-07-15", (const char*)ietf_inet_types_2013_07_15_yin, 0, LYS_IN_YIN},
    {"ietf-yang-types", "2013-07-15", (const char*)ietf_yang_types_2013_07_15_yin, 0, LYS_IN_YIN},
    /* ietf-datastores and ietf-yang-library must be right here at the end of the list! */
    {"ietf-datastores", "2018-02-14", (const char*)ietf_datastores_2018_02_14_yin, 0, LYS_IN_YIN},
    {"ietf-yang-library", IETF_YANG_LIB_REV, (const char*)ietf_yang_library_2019_01_04_yin, 1, LYS_IN_YIN}
};

API unsigned int
ly_ctx_internal_modules_count(struct ly_ctx *ctx)
{
    FUN_IN;

    if (!ctx) {
        return 0;
    }
    return ctx->internal_module_count;
}

API struct ly_ctx *
ly_ctx_new(const char *search_dir, int options)
{
    FUN_IN;

    struct ly_ctx *ctx = NULL;
    struct lys_module *module;
    char *search_dir_list;
    char *sep, *dir;
    int rc = EXIT_SUCCESS;
    int i;

    ctx = calloc(1, sizeof *ctx);
    LY_CHECK_ERR_RETURN(!ctx, LOGMEM(NULL), NULL);

    /* dictionary */
    lydict_init(&ctx->dict);

    /* plugins */
    ly_load_plugins();

    /* initialize thread-specific key */
    if (pthread_key_create(&ctx->errlist_key, ly_err_free) != 0) {
        LOGERR(NULL, LY_ESYS, "pthread_key_create() in ly_ctx_new() failed");
        goto error;
    }

    /* models list */
    ctx->models.list = calloc(16, sizeof *ctx->models.list);
    LY_CHECK_ERR_RETURN(!ctx->models.list, LOGMEM(NULL); free(ctx), NULL);
    ctx->models.flags = options;
    ctx->models.used = 0;
    ctx->models.size = 16;
    if (search_dir) {
        search_dir_list = strdup(search_dir);
        LY_CHECK_ERR_GOTO(!search_dir_list, LOGMEM(NULL), error);

        for (dir = search_dir_list; (sep = strchr(dir, ':')) != NULL && rc == EXIT_SUCCESS; dir = sep + 1) {
            *sep = 0;
            rc = ly_ctx_set_searchdir(ctx, dir);
        }
        if (*dir && rc == EXIT_SUCCESS) {
            rc = ly_ctx_set_searchdir(ctx, dir);
        }
        free(search_dir_list);
        /* If ly_ctx_set_searchdir() failed, the error is already logged. Just exit */
        if (rc != EXIT_SUCCESS) {
            goto error;
        }
    }
    ctx->models.module_set_id = 1;

    /* load internal modules */
    if (options & LY_CTX_NOYANGLIBRARY) {
        ctx->internal_module_count = LY_INTERNAL_MODULE_COUNT - 2;
    } else {
        ctx->internal_module_count = LY_INTERNAL_MODULE_COUNT;
    }
    for (i = 0; i < ctx->internal_module_count; i++) {
        module = (struct lys_module *)lys_parse_mem(ctx, internal_modules[i].data, internal_modules[i].format);
        if (!module) {
            goto error;
        }
        module->implemented = internal_modules[i].implemented;
    }

    return ctx;

error:
    /* cleanup */
    ly_ctx_destroy(ctx, NULL);
    return NULL;
}

static int
ly_ctx_new_yl_legacy(struct ly_ctx *ctx, struct lyd_node *yltree)
{
    unsigned int i, u;
    struct lyd_node *module, *node;
    struct ly_set *set;
    const char *name, *revision;
    struct ly_set features = {0, 0, {NULL}};
    const struct lys_module *mod;

    set = lyd_find_path(yltree, "/ietf-yang-library:yang-library/modules-state/module");
    if (!set) {
        return 1;
    }

    /* process the data tree */
    for (i = 0; i < set->number; ++i) {
        module = set->set.d[i];

        /* initiate */
        name = NULL;
        revision = NULL;
        ly_set_clean(&features);

        LY_TREE_FOR(module->child, node) {
            if (!strcmp(node->schema->name, "name")) {
                name = ((struct lyd_node_leaf_list*)node)->value_str;
            } else if (!strcmp(node->schema->name, "revision")) {
                revision = ((struct lyd_node_leaf_list*)node)->value_str;
            } else if (!strcmp(node->schema->name, "feature")) {
                ly_set_add(&features, node, LY_SET_OPT_USEASLIST);
            } else if (!strcmp(node->schema->name, "conformance-type") &&
                    ((struct lyd_node_leaf_list*)node)->value.enm->value) {
                /* imported module - skip it, it will be loaded as a side effect
                 * of loading another module */
                continue;
            }
        }

        /* use the gathered data to load the module */
        mod = ly_ctx_load_module(ctx, name, revision);
        if (!mod) {
            LOGERR(ctx, LY_EINVAL, "Unable to load module specified by yang library data.");
            ly_set_free(set);
            return 1;
        }

        /* set features */
        for (u = 0; u < features.number; u++) {
            lys_features_enable(mod, ((struct lyd_node_leaf_list*)features.set.d[u])->value_str);
        }
    }

    ly_set_free(set);
    return 0;
}

static struct ly_ctx *
ly_ctx_new_yl_common(const char *search_dir, const char *input, LYD_FORMAT format, int options,
                     struct lyd_node* (*parser_func)(struct ly_ctx*, const char*, LYD_FORMAT, int,...))
{
    unsigned int i, u;
    struct lyd_node *module, *node;
    const char *name, *revision;
    struct ly_set features = {0, 0, {NULL}};
    const struct lys_module *mod;
    struct lyd_node *yltree = NULL;
    struct ly_ctx *ctx = NULL;
    struct ly_set *set = NULL;
    int err = 0;

    /* create empty (with internal modules including ietf-yang-library) context */
    ctx = ly_ctx_new(search_dir, options);
    if (!ctx) {
        goto error;
    }

    /* parse yang library data tree */
    yltree = parser_func(ctx, input, format, LYD_OPT_DATA, NULL);
    if (!yltree) {
        goto error;
    }

    set = lyd_find_path(yltree, "/ietf-yang-library:yang-library/module-set[1]/module");
    if (!set) {
        goto error;
    }

    if (set->number == 0) {
        /* perhaps a legacy data tree? */
        if (ly_ctx_new_yl_legacy(ctx, yltree)) {
            goto error;
        }
    } else {
        /* process the data tree */
        for (i = 0; i < set->number; ++i) {
            module = set->set.d[i];

            /* initiate */
            name = NULL;
            revision = NULL;
            ly_set_clean(&features);

            LY_TREE_FOR(module->child, node) {
                if (!strcmp(node->schema->name, "name")) {
                    name = ((struct lyd_node_leaf_list*)node)->value_str;
                } else if (!strcmp(node->schema->name, "revision")) {
                    revision = ((struct lyd_node_leaf_list*)node)->value_str;
                } else if (!strcmp(node->schema->name, "feature")) {
                    ly_set_add(&features, node, LY_SET_OPT_USEASLIST);
                }
            }

            /* use the gathered data to load the module */
            mod = ly_ctx_load_module(ctx, name, revision);
            if (!mod) {
                LOGERR(NULL, LY_EINVAL, "Unable to load module specified by yang library data.");
                goto error;
            }

            /* set features */
            for (u = 0; u < features.number; u++) {
                lys_features_enable(mod, ((struct lyd_node_leaf_list*)features.set.d[u])->value_str);
            }
        }
    }

    if (0) {
        /* skip context destroy in case of success */
error:
        err = 1;
    }

    /* cleanup */
    if (yltree) {
        /* yang library data tree */
        lyd_free_withsiblings(yltree);
    }
    if (set) {
        ly_set_free(set);
    }
    if (err) {
        ly_ctx_destroy(ctx, NULL);
        ctx = NULL;
    }

    return ctx;
}

API struct ly_ctx *
ly_ctx_new_ylpath(const char *search_dir, const char *path, LYD_FORMAT format, int options)
{
    FUN_IN;

    return ly_ctx_new_yl_common(search_dir, path, format, options, lyd_parse_path);
}

API struct ly_ctx *
ly_ctx_new_ylmem(const char *search_dir, const char *data, LYD_FORMAT format, int options)
{
    FUN_IN;

    return ly_ctx_new_yl_common(search_dir, data, format, options, lyd_parse_mem);
}

static void
ly_ctx_set_option(struct ly_ctx *ctx, int options)
{
    if (!ctx) {
        return;
    }

    ctx->models.flags |= options;
}

static void
ly_ctx_unset_option(struct ly_ctx *ctx, int options)
{
    if (!ctx) {
        return;
    }

    ctx->models.flags &= ~options;
}

API void
ly_ctx_set_disable_searchdirs(struct ly_ctx *ctx)
{
    FUN_IN;

    ly_ctx_set_option(ctx, LY_CTX_DISABLE_SEARCHDIRS);
}

API void
ly_ctx_unset_disable_searchdirs(struct ly_ctx *ctx)
{
    FUN_IN;

    ly_ctx_unset_option(ctx, LY_CTX_DISABLE_SEARCHDIRS);
}

API void
ly_ctx_set_disable_searchdir_cwd(struct ly_ctx *ctx)
{
    FUN_IN;

    ly_ctx_set_option(ctx, LY_CTX_DISABLE_SEARCHDIR_CWD);
}

API void
ly_ctx_unset_disable_searchdir_cwd(struct ly_ctx *ctx)
{
    FUN_IN;

    ly_ctx_unset_option(ctx, LY_CTX_DISABLE_SEARCHDIR_CWD);
}

API void
ly_ctx_set_prefer_searchdirs(struct ly_ctx *ctx)
{
    FUN_IN;

    ly_ctx_set_option(ctx, LY_CTX_PREFER_SEARCHDIRS);
}

API void
ly_ctx_unset_prefer_searchdirs(struct ly_ctx *ctx)
{
    FUN_IN;

    ly_ctx_unset_option(ctx, LY_CTX_PREFER_SEARCHDIRS);
}

API void
ly_ctx_set_allimplemented(struct ly_ctx *ctx)
{
    FUN_IN;

    ly_ctx_set_option(ctx, LY_CTX_ALLIMPLEMENTED);
}

API void
ly_ctx_unset_allimplemented(struct ly_ctx *ctx)
{
    FUN_IN;

    ly_ctx_unset_option(ctx, LY_CTX_ALLIMPLEMENTED);
}

API void
ly_ctx_set_trusted(struct ly_ctx *ctx)
{
    FUN_IN;

    ly_ctx_set_option(ctx, LY_CTX_TRUSTED);
}

API void
ly_ctx_unset_trusted(struct ly_ctx *ctx)
{
    FUN_IN;

    ly_ctx_unset_option(ctx, LY_CTX_TRUSTED);
}

API int
ly_ctx_get_options(struct ly_ctx *ctx)
{
    FUN_IN;

    return ctx->models.flags;
}

API int
ly_ctx_set_searchdir(struct ly_ctx *ctx, const char *search_dir)
{
    FUN_IN;

    char *new_dir = NULL;
    int index = 0;
    void *r;
    int rc = EXIT_FAILURE;

    if (!ctx) {
        LOGARG;
        return EXIT_FAILURE;
    }

    if (search_dir) {
        if (access(search_dir, R_OK | X_OK)) {
            LOGERR(ctx, LY_ESYS, "Unable to use search directory \"%s\" (%s)",
                   search_dir, strerror(errno));
            return EXIT_FAILURE;
        }

        new_dir = realpath(search_dir, NULL);
        LY_CHECK_ERR_GOTO(!new_dir, LOGERR(ctx, LY_ESYS, "realpath() call failed (%s).", strerror(errno)), cleanup);
        if (!ctx->models.search_paths) {
            ctx->models.search_paths = malloc(2 * sizeof *ctx->models.search_paths);
            LY_CHECK_ERR_GOTO(!ctx->models.search_paths, LOGMEM(ctx), cleanup);
            index = 0;
        } else {
            for (index = 0; ctx->models.search_paths[index]; index++) {
                /* check for duplicities */
                if (!strcmp(new_dir, ctx->models.search_paths[index])) {
                    /* path is already present */
                    goto success;
                }
            }
            r = realloc(ctx->models.search_paths, (index + 2) * sizeof *ctx->models.search_paths);
            LY_CHECK_ERR_GOTO(!r, LOGMEM(ctx), cleanup);
            ctx->models.search_paths = r;
        }
        ctx->models.search_paths[index] = new_dir;
        new_dir = NULL;
        ctx->models.search_paths[index + 1] = NULL;

success:
        rc = EXIT_SUCCESS;
    } else {
        /* consider that no change is not actually an error */
        return EXIT_SUCCESS;
    }

cleanup:
    free(new_dir);
    return rc;
}

API const char * const *
ly_ctx_get_searchdirs(const struct ly_ctx *ctx)
{
    FUN_IN;

    if (!ctx) {
        LOGARG;
        return NULL;
    }
    return (const char * const *)ctx->models.search_paths;
}

API void
ly_ctx_unset_searchdirs(struct ly_ctx *ctx, int index)
{
    FUN_IN;

    int i;

    if (!ctx->models.search_paths) {
        return;
    }

    for (i = 0; ctx->models.search_paths[i]; i++) {
        if (index < 0 || index == i) {
            free(ctx->models.search_paths[i]);
            ctx->models.search_paths[i] = NULL;
        } else if (i > index) {
            ctx->models.search_paths[i - 1] = ctx->models.search_paths[i];
            ctx->models.search_paths[i] = NULL;
        }
    }
    if (index < 0 || !ctx->models.search_paths[0]) {
        free(ctx->models.search_paths);
        ctx->models.search_paths = NULL;
    }
}

API void
ly_ctx_destroy(struct ly_ctx *ctx, void (*private_destructor)(const struct lys_node *node, void *priv))
{
    FUN_IN;

    int i;

    if (!ctx) {
        return;
    }

    /* models list */
    for (; ctx->models.used > 0; ctx->models.used--) {
        /* remove the applied deviations and augments */
        lys_sub_module_remove_devs_augs(ctx->models.list[ctx->models.used - 1]);
        /* remove the module */
        lys_free(ctx->models.list[ctx->models.used - 1], private_destructor, 1, 0);
    }
    if (ctx->models.search_paths) {
        for(i = 0; ctx->models.search_paths[i]; i++) {
            free(ctx->models.search_paths[i]);
        }
        free(ctx->models.search_paths);
    }
    free(ctx->models.list);

    /* clean the error list */
    ly_err_clean(ctx, 0);
    pthread_key_delete(ctx->errlist_key);

    /* dictionary */
    lydict_clean(&ctx->dict);

    /* plugins - will be removed only if this is the last context */
    ly_clean_plugins();

    free(ctx);
}

API const struct lys_submodule *
ly_ctx_get_submodule2(const struct lys_module *main_module, const char *submodule)
{
    FUN_IN;

    const struct lys_submodule *result;
    int i;

    if (!main_module || !submodule) {
        LOGARG;
        return NULL;
    }

    /* search in submodules list */
    for (i = 0; i < main_module->inc_size; i++) {
        result = main_module->inc[i].submodule;
        if (ly_strequal(submodule, result->name, 0)) {
            return result;
        }

        /* in YANG 1.1 all the submodules must be included in the main module, so we are done.
         * YANG 1.0 allows (is unclear about denying it) to include a submodule only in another submodule
         * but when libyang parses such a module it adds the include into the main module so we are also done.
         */
    }

    return NULL;
}

API const struct lys_submodule *
ly_ctx_get_submodule(const struct ly_ctx *ctx, const char *module, const char *revision, const char *submodule,
                     const char *sub_revision)
{
    FUN_IN;

    const struct lys_module *mainmod;
    const struct lys_submodule *ret = NULL, *submod;
    uint32_t idx = 0;

    if (!ctx || !submodule || (revision && !module)) {
        LOGARG;
        return NULL;
    }

    while ((mainmod = ly_ctx_get_module_iter(ctx, &idx))) {
        if (module && strcmp(mainmod->name, module)) {
            /* main module name does not match */
            continue;
        }

        if (revision && (!mainmod->rev || strcmp(revision, mainmod->rev[0].date))) {
            /* main module revision does not match */
            continue;
        }

        submod = ly_ctx_get_submodule2(mainmod, submodule);
        if (!submod) {
            continue;
        }

        if (!sub_revision) {
            /* store only if newer */
            if (ret) {
                if (submod->rev && (!ret->rev || (strcmp(submod->rev[0].date, ret->rev[0].date) > 0))) {
                    ret = submod;
                }
            } else {
                ret = submod;
            }
        } else {
            /* store only if revision matches, we are done if it does */
            if (!submod->rev) {
                continue;
            } else if (!strcmp(sub_revision, submod->rev[0].date)) {
                ret = submod;
                break;
            }
        }
    }

    return ret;
}

static const struct lys_module *
ly_ctx_get_module_by(const struct ly_ctx *ctx, const char *key, size_t key_len, int offset, const char *revision,
                     int with_disabled, int implemented)
{
    int i;
    char *val;
    struct lys_module *result = NULL;

    if (!ctx || !key) {
        LOGARG;
        return NULL;
    }

    for (i = 0; i < ctx->models.used; i++) {
        if (!with_disabled && ctx->models.list[i]->disabled) {
            /* skip the disabled modules */
            continue;
        }
        /* use offset to get address of the pointer to string (char**), remember that offset is in
         * bytes, so we have to cast the pointer to the module to (char*), finally, we want to have
         * string not the pointer to string
         */
        val = *(char **)(((char *)ctx->models.list[i]) + offset);
        if (!ctx->models.list[i] || (!key_len && strcmp(key, val)) || (key_len && (strncmp(key, val, key_len) || val[key_len]))) {
            continue;
        }

        if (!revision) {
            /* compare revisons and remember the newest one */
            if (result) {
                if (!ctx->models.list[i]->rev_size) {
                    /* the current have no revision, keep the previous with some revision */
                    continue;
                }
                if (result->rev_size && strcmp(ctx->models.list[i]->rev[0].date, result->rev[0].date) < 0) {
                    /* the previous found matching module has a newer revision */
                    continue;
                }
            }
            if (implemented) {
                if (ctx->models.list[i]->implemented) {
                    /* we have the implemented revision */
                    result = ctx->models.list[i];
                    break;
                } else {
                    /* do not remember the result, we are supposed to return the implemented revision
                     * not the newest one */
                    continue;
                }
            }

            /* remember the current match and search for newer version */
            result = ctx->models.list[i];
        } else {
            if (ctx->models.list[i]->rev_size && !strcmp(revision, ctx->models.list[i]->rev[0].date)) {
                /* matching revision */
                result = ctx->models.list[i];
                break;
            }
        }
    }

    return result;

}

API const struct lys_module *
ly_ctx_get_module_by_ns(const struct ly_ctx *ctx, const char *ns, const char *revision, int implemented)
{
    FUN_IN;

    return ly_ctx_get_module_by(ctx, ns, 0, offsetof(struct lys_module, ns), revision, 0, implemented);
}

API const struct lys_module *
ly_ctx_get_module(const struct ly_ctx *ctx, const char *name, const char *revision, int implemented)
{
    FUN_IN;

    return ly_ctx_get_module_by(ctx, name, 0, offsetof(struct lys_module, name), revision, 0, implemented);
}

const struct lys_module *
ly_ctx_nget_module(const struct ly_ctx *ctx, const char *name, size_t name_len, const char *revision, int implemented)
{
    return ly_ctx_get_module_by(ctx, name, name_len, offsetof(struct lys_module, name), revision, 0, implemented);
}

API const struct lys_module *
ly_ctx_get_module_older(const struct ly_ctx *ctx, const struct lys_module *module)
{
    FUN_IN;

    int i;
    const struct lys_module *result = NULL, *iter;

    if (!ctx || !module || !module->rev_size) {
        LOGARG;
        return NULL;
    }


    for (i = 0; i < ctx->models.used; i++) {
        iter = ctx->models.list[i];
        if (iter->disabled) {
            /* skip the disabled modules */
            continue;
        }
        if (iter == module || !iter->rev_size) {
            /* iter is the module itself or iter has no revision */
            continue;
        }
        if (!ly_strequal(module->name, iter->name, 0)) {
            /* different module */
            continue;
        }
        if (strcmp(iter->rev[0].date, module->rev[0].date) < 0) {
            /* iter is older than module */
            if (result) {
                if (strcmp(iter->rev[0].date, result->rev[0].date) > 0) {
                    /* iter is newer than current result */
                    result = iter;
                }
            } else {
                result = iter;
            }
        }
    }

    return result;
}

API void
ly_ctx_set_module_imp_clb(struct ly_ctx *ctx, ly_module_imp_clb clb, void *user_data)
{
    FUN_IN;

    if (!ctx) {
        LOGARG;
        return;
    }

    ctx->imp_clb = clb;
    ctx->imp_clb_data = user_data;
}

API ly_module_imp_clb
ly_ctx_get_module_imp_clb(const struct ly_ctx *ctx, void **user_data)
{
    FUN_IN;

    if (!ctx) {
        LOGARG;
        return NULL;
    }

    if (user_data) {
        *user_data = ctx->imp_clb_data;
    }
    return ctx->imp_clb;
}

API void
ly_ctx_set_module_data_clb(struct ly_ctx *ctx, ly_module_data_clb clb, void *user_data)
{
    FUN_IN;

    if (!ctx) {
        LOGARG;
        return;
    }

    ctx->data_clb = clb;
    ctx->data_clb_data = user_data;
}

API ly_module_data_clb
ly_ctx_get_module_data_clb(const struct ly_ctx *ctx, void **user_data)
{
    FUN_IN;

    if (!ctx) {
        LOGARG;
        return NULL;
    }

    if (user_data) {
        *user_data = ctx->data_clb_data;
    }
    return ctx->data_clb;
}

#ifdef LY_ENABLED_LYD_PRIV

API void
ly_ctx_set_priv_dup_clb(struct ly_ctx *ctx, void *(*priv_dup_clb)(const void *priv))
{
    FUN_IN;

    ctx->priv_dup_clb = priv_dup_clb;
}

#endif

/* if module is !NULL, then the function searches for submodule */
static struct lys_module *
ly_ctx_load_localfile(struct ly_ctx *ctx, struct lys_module *module, const char *name, const char *revision,
                int implement, struct unres_schema *unres)
{
    size_t len;
    int fd, i;
    char *filepath = NULL, *dot, *rev, *filename;
    LYS_INFORMAT format;
    struct lys_module *result = NULL;

    if (lys_search_localfile(ly_ctx_get_searchdirs(ctx), !(ctx->models.flags & LY_CTX_DISABLE_SEARCHDIR_CWD), name, revision,
                             &filepath, &format)) {
        goto cleanup;
    } else if (!filepath) {
        if (!module && !revision) {
            /* otherwise the module would be already taken from the context */
            result = (struct lys_module *)ly_ctx_get_module(ctx, name, NULL, 0);
        }
        if (!result) {
            LOGERR(ctx, LY_ESYS, "Data model \"%s\" not found.", name);
        }
        return result;
    }

    LOGVRB("Loading schema from \"%s\" file.", filepath);

    /* cut the format for now */
    dot = strrchr(filepath, '.');
    dot[1] = '\0';

    /* check that the same file was not already loaded - it make sense only in case of loading the newest revision,
     * search also in disabled module - if the matching module is disabled, it will be enabled instead of loading it */
    if (!revision) {
        for (i = 0; i < ctx->models.used; ++i) {
            if (ctx->models.list[i]->filepath && !strcmp(name, ctx->models.list[i]->name)
                    && !strncmp(filepath, ctx->models.list[i]->filepath, strlen(filepath))) {
                result = ctx->models.list[i];
                if (implement && !result->implemented) {
                    /* make it implemented now */
                    if (lys_set_implemented(result)) {
                        result = NULL;
                    }
                } else if (result->disabled) {
                    lys_set_enabled(result);
                }

                goto cleanup;
            }
        }
    }

    /* add the format back */
    dot[1] = 'y';

    /* open the file */
    fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        LOGERR(ctx, LY_ESYS, "Unable to open data model file \"%s\" (%s).",
               filepath, strerror(errno));
        goto cleanup;
    }

    if (module) {
        result = (struct lys_module *)lys_sub_parse_fd(module, fd, format, unres);
    } else {
        result = (struct lys_module *)lys_parse_fd_(ctx, fd, format, revision, implement);
    }
    close(fd);

    if (!result) {
        goto cleanup;
    }

    /* check that name and revision match filename */
    filename = strrchr(filepath, '/');
    if (!filename) {
        filename = filepath;
    } else {
        filename++;
    }
    rev = strchr(filename, '@');
    /* name */
    len = strlen(result->name);
    if (strncmp(filename, result->name, len) ||
            ((rev && rev != &filename[len]) || (!rev && dot != &filename[len]))) {
        LOGWRN(ctx, "File name \"%s\" does not match module name \"%s\".", filename, result->name);
    }
    if (rev) {
        len = dot - ++rev;
        if (!result->rev_size || len != 10 || strncmp(result->rev[0].date, rev, len)) {
            LOGWRN(ctx, "File name \"%s\" does not match module revision \"%s\".", filename,
                   result->rev_size ? result->rev[0].date : "none");
        }
    }

    if (!result->filepath) {
        char rpath[PATH_MAX];
        if (realpath(filepath, rpath) != NULL) {
            result->filepath = lydict_insert(ctx, rpath, 0);
        } else {
            result->filepath = lydict_insert(ctx, filepath, 0);
        }
    }

    /* success */
cleanup:
    free(filepath);
    return result;
}

static struct lys_module *
ly_ctx_load_sub_module_clb(struct ly_ctx *ctx, struct lys_module *module, const char *name, const char *revision,
                           int implement, struct unres_schema *unres)
{
    struct lys_module *mod = NULL;
    const char *module_data = NULL;
    LYS_INFORMAT format = LYS_IN_UNKNOWN;
    void (*module_data_free)(void *module_data, void *user_data) = NULL;

    ly_errno = LY_SUCCESS;
    if (module) {
        mod = lys_main_module(module);
        module_data = ctx->imp_clb(mod->name, (mod->rev_size ? mod->rev[0].date : NULL), name, revision, ctx->imp_clb_data, &format, &module_data_free);
    } else {
        module_data = ctx->imp_clb(name, revision, NULL, NULL, ctx->imp_clb_data, &format, &module_data_free);
    }
    if (!module_data && (ly_errno != LY_SUCCESS)) {
        /* callback encountered an error, do not change it */
        LOGERR(ctx, ly_errno, "User module retrieval callback failed!");
        return NULL;
    }

    if (module_data) {
        /* we got the module from the callback */
        if (module) {
            mod = (struct lys_module *)lys_sub_parse_mem(module, module_data, format, unres);
        } else {
            mod = (struct lys_module *)lys_parse_mem_(ctx, module_data, format, NULL, 0, implement);
        }

        if (module_data_free) {
            module_data_free((char *)module_data, ctx->imp_clb_data);
        }
    }

    return mod;
}

const struct lys_module *
ly_ctx_load_sub_module(struct ly_ctx *ctx, struct lys_module *module, const char *name, const char *revision,
                       int implement, struct unres_schema *unres)
{
    struct lys_module *mod = NULL, *latest_mod = NULL;
    int i;

    if (!module) {
        /* try to get the schema from the context (with or without revision),
         * include the disabled modules in the search to avoid their duplication,
         * they are enabled by the subsequent call to lys_set_implemented() */
        for (i = 0, mod = NULL; i < ctx->models.used; i++) {
            mod = ctx->models.list[i]; /* shortcut */
            if (ly_strequal(name, mod->name, 0)) {
                /* first remember latest module if no other is found */
                if (!latest_mod) {
                    latest_mod = mod;
                } else {
                    if (mod->rev_size && latest_mod->rev_size && (strcmp(mod->rev[0].date, latest_mod->rev[0].date) > 0)) {
                        /* newer revision */
                        latest_mod = mod;
                    }
                }

                if (revision && mod->rev_size && !strcmp(revision, mod->rev[0].date)) {
                    /* the specific revision was already loaded */
                    break;
                } else if (!revision && mod->latest_revision) {
                    /* the latest revision of this module was already loaded */
                    break;
                } else if (implement && mod->implemented && !revision) {
                    /* we are not able to implement another module, so consider this module as the latest one */
                    break;
                }
            }
            mod = NULL;
        }
        if (mod) {
            /* module must be enabled */
            if (mod->disabled) {
                lys_set_enabled(mod);
            }
            /* module is supposed to be implemented */
            if (implement && lys_set_implemented(mod)) {
                /* the schema cannot be implemented */
                mod = NULL;
            }
            return mod;
        }
    }

    /* module is not yet in context, use the user callback or try to find the schema on our own */
    if (ctx->imp_clb && !(ctx->models.flags & LY_CTX_PREFER_SEARCHDIRS)) {
search_clb:
        if (ctx->imp_clb) {
            mod = ly_ctx_load_sub_module_clb(ctx, module, name, revision, implement, unres);
        }
        if (!mod && !(ctx->models.flags & LY_CTX_PREFER_SEARCHDIRS)) {
            goto search_file;
        }
    } else {
search_file:
        if (!(ctx->models.flags & LY_CTX_DISABLE_SEARCHDIRS)) {
            /* module was not received from the callback or there is no callback set */
            mod = ly_ctx_load_localfile(ctx, module, name, revision, implement, unres);
        }
        if (!mod && (ctx->models.flags & LY_CTX_PREFER_SEARCHDIRS)) {
            goto search_clb;
        }
    }

    if (mod && !revision && latest_mod && mod->rev_size && latest_mod->rev_size
                && (strcmp(mod->rev[0].date, latest_mod->rev[0].date) < 0)) {
        /* the found module has older revision as the one already in context and we are looking for the latest one, free it */
        lys_free(mod, NULL, 1, 1);
        mod = NULL;
    }

    if (!mod && latest_mod) {
        /* consider the latest mod found as the latest available */
        mod = latest_mod;
    }

#ifdef LY_ENABLED_LATEST_REVISIONS
    if (!revision && mod) {
        /* module is the latest revision found */
        mod->latest_revision = 1;
    }
#endif

    return mod;
}

API const struct lys_module *
ly_ctx_load_module(struct ly_ctx *ctx, const char *name, const char *revision)
{
    FUN_IN;

    if (!ctx || !name) {
        LOGARG;
        return NULL;
    }

    return ly_ctx_load_sub_module(ctx, NULL, name, revision && revision[0] ? revision : NULL, 1, NULL);
}

/*
 * mods - set of removed modules, if NULL all modules are supposed to be removed so any backlink is invalid
 */
static void
ctx_modules_undo_backlinks(struct ly_ctx *ctx, struct ly_set *mods)
{
    int o;
    uint8_t j;
    unsigned int u, v;
    struct lys_module *mod;

    /* maintain backlinks (start with internal ietf-yang-library which have leafs as possible targets of leafrefs */
    for (o = ctx->internal_module_count - 1; o < ctx->models.used; o++) {
        mod = ctx->models.list[o]; /* shortcut */

        /* 1) features */
        for (j = 0; j < mod->features_size; j++) {
            if (!mod->features[j].depfeatures) {
                continue;
            }
            for (v = 0; v < mod->features[j].depfeatures->number; v++) {
                if (!mods || ly_set_contains(mods, ((struct lys_feature *)mod->features[j].depfeatures->set.g[v])->module) != -1) {
                    /* depending feature is in module to remove */
                    ly_set_rm_index(mod->features[j].depfeatures, v);
                    v--;
                }
            }
            if (!mod->features[j].depfeatures->number) {
                /* all backlinks removed */
                ly_set_free(mod->features[j].depfeatures);
                mod->features[j].depfeatures = NULL;
            }
        }

        /* 2) identities */
        for (u = 0; u < mod->ident_size; u++) {
            if (!mod->ident[u].der) {
                continue;
            }
            for (v = 0; v < mod->ident[u].der->number; v++) {
                if (!mods || ly_set_contains(mods, ((struct lys_ident *)mod->ident[u].der->set.g[v])->module) != -1) {
                    /* derived identity is in module to remove */
                    ly_set_rm_index(mod->ident[u].der, v);
                    v--;
                }
            }
            if (!mod->ident[u].der->number) {
                /* all backlinks removed */
                ly_set_free(mod->ident[u].der);
                mod->ident[u].der = NULL;
            }
        }
    }
}

static int
ctx_modules_redo_backlinks(struct ly_set *mods)
{
    unsigned int i, j, k, s;
    struct lys_module *mod;
    struct lys_feature *feat;

    for (i = 0; i < mods->number; ++i) {
        mod = (struct lys_module *)mods->set.g[i]; /* shortcut */

        /* identities */
        if (mod->implemented) {
            for (j = 0; j < mod->ident_size; j++) {
                for (k = 0; k < mod->ident[j].base_size; k++) {
                    resolve_identity_backlink_update(&mod->ident[j], mod->ident[j].base[k]);
                }
            }
        }

        /* features */
        for (j = 0; j < mod->features_size; j++) {
            for (k = 0; k < mod->features[j].iffeature_size; k++) {
                resolve_iffeature_getsizes(&mod->features[j].iffeature[k], NULL, &s);
                while (s--) {
                    feat = mod->features[j].iffeature[k].features[s]; /* shortcut */
                    if (!feat->depfeatures) {
                        feat->depfeatures = ly_set_new();
                    }
                    ly_set_add(feat->depfeatures, &mod->features[j], LY_SET_OPT_USEASLIST);
                }
            }
        }
    }

    return 0;
}

API int
lys_set_disabled(const struct lys_module *module)
{
    FUN_IN;

    struct ly_ctx *ctx; /* shortcut */
    struct lys_module *mod;
    struct ly_set *mods;
    uint8_t j, imported;
    int i, o;
    unsigned int u, v;

    if (!module) {
        LOGARG;
        return EXIT_FAILURE;
    } else if (module->disabled) {
        /* already disabled module */
        return EXIT_SUCCESS;
    }
    mod = (struct lys_module *)module;
    ctx = mod->ctx;

    /* avoid disabling internal modules */
    for (i = 0; i < ctx->internal_module_count; i++) {
        if (mod == ctx->models.list[i]) {
            LOGERR(ctx, LY_EINVAL, "Internal module \"%s\" cannot be disabled.", mod->name);
            return EXIT_FAILURE;
        }
    }

    /* disable the module */
    mod->disabled = 1;

    /* get the complete list of modules to disable because of dependencies,
     * we are going also to disable all the imported (not implemented) modules
     * that are not used in any other module */
    mods = ly_set_new();
    ly_set_add(mods, mod, 0);
checkdependency:
    for (i = ctx->internal_module_count; i < ctx->models.used; i++) {
        mod = ctx->models.list[i]; /* shortcut */
        if (mod->disabled) {
            /* skip the already disabled modules */
            continue;
        }

        /* check depndency of imported modules */
        for (j = 0; j < mod->imp_size; j++) {
            for (u = 0; u < mods->number; u++) {
                if (mod->imp[j].module == mods->set.g[u]) {
                    /* module is importing some module to disable, so it must be also disabled */
                    mod->disabled = 1;
                    ly_set_add(mods, mod, 0);
                    /* we have to start again because some of the already checked modules can
                     * depend on the one we have just decided to disable */
                    goto checkdependency;
                }
            }
        }
        /* check if the imported module is used in any module supposed to be kept */
        if (!mod->implemented) {
            imported = 0;
            for (o = ctx->internal_module_count; o < ctx->models.used; o++) {
                if (ctx->models.list[o]->disabled) {
                    /* skip modules already disabled */
                    continue;
                }
                for (j = 0; j < ctx->models.list[o]->imp_size; j++) {
                    if (ctx->models.list[o]->imp[j].module == mod) {
                        /* the module is used in some other module not yet selected to be disabled */
                        imported = 1;
                        goto imported;
                    }
                }
            }
imported:
            if (!imported) {
                /* module is not implemented and neither imported by any other module in context
                 * which is supposed to be kept enabled after this operation, so we are going to disable also
                 * this module */
                mod->disabled = 1;
                ly_set_add(mods, mod, 0);
                /* we have to start again, this time not because other module can depend on this one
                 * (we know that there is no such module), but because the module can import module
                 * that could became useless. If there are no imports, we can continue */
                if (mod->imp_size) {
                    goto checkdependency;
                }
            }
        }
    }

    /* before removing applied deviations, augments and updating leafrefs, we have to enable the modules
     * to disable to allow all that operations */
    for (u = 0; u < mods->number; u++) {
        ((struct lys_module *)mods->set.g[u])->disabled = 0;
    }

    /* maintain backlinks (start with internal ietf-yang-library which have leafs as possible targets of leafrefs */
    ctx_modules_undo_backlinks(ctx, mods);

    /* remove the applied deviations and augments */
    u = mods->number;
    while (u--) {
        lys_sub_module_remove_devs_augs((struct lys_module *)mods->set.g[u]);
    }

    /* now again disable the modules to disable and disable also all its submodules */
    for (u = 0; u < mods->number; u++) {
        mod = (struct lys_module *)mods->set.g[u];
        mod->disabled = 1;
        for (v = 0; v < mod->inc_size; v++) {
            mod->inc[v].submodule->disabled = 1;
        }
    }

    /* free the set */
    ly_set_free(mods);

    /* update the module-set-id */
    ctx->models.module_set_id++;

    return EXIT_SUCCESS;
}

static void
lys_set_enabled_(struct ly_set *mods, struct lys_module *mod)
{
    unsigned int i;

    ly_set_add(mods, mod, 0);
    mod->disabled = 0;

    for (i = 0; i < mod->inc_size; i++) {
        mod->inc[i].submodule->disabled = 0;
    }

    /* go recursively */
    for (i = 0; i < mod->imp_size; i++) {
        if (!mod->imp[i].module->disabled) {
            continue;
        }

        lys_set_enabled_(mods, mod->imp[i].module);
    }
}

API int
lys_set_enabled(const struct lys_module *module)
{
    FUN_IN;

    struct ly_ctx *ctx; /* shortcut */
    struct lys_module *mod;
    struct ly_set *mods, *disabled;
    int i;
    unsigned int u, v, w;

    if (!module) {
        LOGARG;
        return EXIT_FAILURE;
    } else if (!module->disabled) {
        /* already enabled module */
        return EXIT_SUCCESS;
    }
    mod = (struct lys_module *)module;
    ctx = mod->ctx;

    /* avoid disabling internal modules */
    for (i = 0; i < ctx->internal_module_count; i++) {
        if (mod == ctx->models.list[i]) {
            LOGERR(ctx, LY_EINVAL, "Internal module \"%s\" cannot be removed.", mod->name);
            return EXIT_FAILURE;
        }
    }

    mods = ly_set_new();
    disabled = ly_set_new();

    /* enable the module, including its dependencies */
    lys_set_enabled_(mods, mod);

    /* we will go through the all disabled modules in the context, if the module has no dependency (import)
     * that is still disabled AND at least one of its imported module is from the set we are enabling now,
     * it is going to be also enabled. This way we try to revert everething that was possibly done by
     * lys_set_disabled(). */
checkdependency:
    for (i = ctx->internal_module_count; i < ctx->models.used; i++) {
        mod = ctx->models.list[i]; /* shortcut */
        if (!mod->disabled || ly_set_contains(disabled, mod) != -1) {
            /* skip the enabled modules */
            continue;
        }

        /* check imported modules */
        for (u = 0; u < mod->imp_size; u++) {
            if (mod->imp[u].module->disabled) {
                /* it has disabled dependency so it must stay disabled */
                break;
            }
        }
        if (u < mod->imp_size) {
            /* it has disabled dependency, continue with the next module in the context */
            continue;
        }

        /* get know if at least one of the imported modules is being enabled this time */
        for (u = 0; u < mod->imp_size; u++) {
            for (v = 0; v < mods->number; v++) {
                if (mod->imp[u].module == mods->set.g[v]) {
                    /* yes, it is, so they are connected and we are going to enable it as well,
                     * it is not necessary to call recursive lys_set_enable_() because we already
                     * know that there is no disabled import to enable */
                    mod->disabled = 0;
                    ly_set_add(mods, mod, 0);
                    for (w = 0; w < mod->inc_size; w++) {
                        mod->inc[w].submodule->disabled = 0;
                    }
                    /* we have to start again because some of the already checked modules can
                     * depend on the one we have just decided to enable */
                    goto checkdependency;
                }
            }
        }

        /* this module is disabled, but it does not depend on any other disabled module and none
         * of its imports was not enabled in this call. No future enabling of the disabled module
         * will change this so we can remember the module and skip it next time we will have to go
         * through the all context because of the checkdependency goto.
         */
        ly_set_add(disabled, mod, 0);
    }

    /* maintain backlinks */
    ctx_modules_redo_backlinks(mods);

    /* re-apply the deviations and augments */
    for (v = 0; v < mods->number; v++) {
        if (((struct lys_module *)mods->set.g[v])->implemented) {
            lys_sub_module_apply_devs_augs((struct lys_module *)mods->set.g[v]);
        }
    }

    /* free the sets */
    ly_set_free(mods);
    ly_set_free(disabled);

    /* update the module-set-id */
    ctx->models.module_set_id++;

    return EXIT_SUCCESS;
}

API int
ly_ctx_remove_module(const struct lys_module *module,
                     void (*private_destructor)(const struct lys_node *node, void *priv))
{
    FUN_IN;

    struct ly_ctx *ctx; /* shortcut */
    struct lys_module *mod = NULL;
    struct ly_set *mods;
    uint8_t j, imported;
    int i, o;
    unsigned int u;

    if (!module) {
        LOGARG;
        return EXIT_FAILURE;
    }

    mod = (struct lys_module *)module;
    ctx = mod->ctx;

    /* avoid removing internal modules ... */
    for (i = 0; i < ctx->internal_module_count; i++) {
        if (mod == ctx->models.list[i]) {
            LOGERR(ctx, LY_EINVAL, "Internal module \"%s\" cannot be removed.", mod->name);
            return EXIT_FAILURE;
        }
    }
    /* ... and hide the module from the further processing of the context modules list */
    for (i = ctx->internal_module_count; i < ctx->models.used; i++) {
        if (mod == ctx->models.list[i]) {
            ctx->models.list[i] = NULL;
            break;
        }
    }

    /* get the complete list of modules to remove because of dependencies,
     * we are going also to remove all the imported (not implemented) modules
     * that are not used in any other module */
    mods = ly_set_new();
    ly_set_add(mods, mod, 0);
checkdependency:
    for (i = ctx->internal_module_count; i < ctx->models.used; i++) {
        mod = ctx->models.list[i]; /* shortcut */
        if (!mod) {
            /* skip modules already selected for removing */
            continue;
        }

        /* check depndency of imported modules */
        for (j = 0; j < mod->imp_size; j++) {
            for (u = 0; u < mods->number; u++) {
                if (mod->imp[j].module == mods->set.g[u]) {
                    /* module is importing some module to remove, so it must be also removed */
                    ly_set_add(mods, mod, 0);
                    ctx->models.list[i] = NULL;
                    /* we have to start again because some of the already checked modules can
                     * depend on the one we have just decided to remove */
                    goto checkdependency;
                }
            }
        }
        /* check if the imported module is used in any module supposed to be kept */
        if (!mod->implemented) {
            imported = 0;
            for (o = ctx->internal_module_count; o < ctx->models.used; o++) {
                if (!ctx->models.list[o]) {
                    /* skip modules already selected for removing */
                    continue;
                }
                for (j = 0; j < ctx->models.list[o]->imp_size; j++) {
                    if (ctx->models.list[o]->imp[j].module == mod) {
                        /* the module is used in some other module not yet selected to be deleted */
                        imported = 1;
                        goto imported;
                    }
                }
            }
imported:
            if (!imported) {
                /* module is not implemented and neither imported by any other module in context
                 * which is supposed to be kept after this operation, so we are going to remove also
                 * this useless module */
                ly_set_add(mods, mod, 0);
                ctx->models.list[i] = NULL;
                /* we have to start again, this time not because other module can depend on this one
                 * (we know that there is no such module), but because the module can import module
                 * that could became useless. If there are no imports, we can continue */
                if (mod->imp_size) {
                    goto checkdependency;
                }
            }
        }
    }


    /* consolidate the modules list */
    for (i = o = ctx->internal_module_count; i < ctx->models.used; i++) {
        if (ctx->models.list[o]) {
            /* used cell */
            o++;
        } else {
            /* the current output cell is empty, move here an input cell */
            ctx->models.list[o] = ctx->models.list[i];
            ctx->models.list[i] = NULL;
        }
    }
    /* get the last used cell to get know the number of used */
    while (!ctx->models.list[o]) {
        o--;
    }
    ctx->models.used = o + 1;
    ctx->models.module_set_id++;

    /* maintain backlinks (start with internal ietf-yang-library which have leafs as possible targets of leafrefs */
    ctx_modules_undo_backlinks(ctx, mods);

    /* free the modules */
    for (u = 0; u < mods->number; u++) {
        /* remove the applied deviations and augments */
        lys_sub_module_remove_devs_augs((struct lys_module *)mods->set.g[u]);
        /* remove the module */
        lys_free((struct lys_module *)mods->set.g[u], private_destructor, 1, 0);
    }
    ly_set_free(mods);

    return EXIT_SUCCESS;
}

API void
ly_ctx_clean(struct ly_ctx *ctx, void (*private_destructor)(const struct lys_node *node, void *priv))
{
    FUN_IN;

    if (!ctx) {
        return;
    }

    /* models list */
    for (; ctx->models.used > ctx->internal_module_count; ctx->models.used--) {
        /* remove the applied deviations and augments */
        lys_sub_module_remove_devs_augs(ctx->models.list[ctx->models.used - 1]);
        /* remove the module */
        lys_free(ctx->models.list[ctx->models.used - 1], private_destructor, 1, 0);
        /* clean it for safer future use */
        ctx->models.list[ctx->models.used - 1] = NULL;
    }
    ctx->models.module_set_id++;

    /* maintain backlinks (actually done only with ietf-yang-library since its leafs can be target of leafref) */
    ctx_modules_undo_backlinks(ctx, NULL);
}

API const struct lys_module *
ly_ctx_get_module_iter(const struct ly_ctx *ctx, uint32_t *idx)
{
    FUN_IN;

    if (!ctx || !idx) {
        LOGARG;
        return NULL;
    }

    for ( ; *idx < (unsigned)ctx->models.used; (*idx)++) {
        if (!ctx->models.list[(*idx)]->disabled) {
            return ctx->models.list[(*idx)++];
        }
    }

    return NULL;
}

API const struct lys_module *
ly_ctx_get_disabled_module_iter(const struct ly_ctx *ctx, uint32_t *idx)
{
    FUN_IN;

    if (!ctx || !idx) {
        LOGARG;
        return NULL;
    }

    for ( ; *idx < (unsigned)ctx->models.used; (*idx)++) {
        if (ctx->models.list[(*idx)]->disabled) {
            return ctx->models.list[(*idx)++];
        }
    }

    return NULL;
}

static int
ylib_feature(struct lyd_node *parent, struct lys_module *cur_mod)
{
    int i, j;

    /* module features */
    for (i = 0; i < cur_mod->features_size; ++i) {
        if (!(cur_mod->features[i].flags & LYS_FENABLED)) {
            continue;
        }

        if (!lyd_new_leaf(parent, NULL, "feature", cur_mod->features[i].name)) {
            return EXIT_FAILURE;
        }
    }

    /* submodule features */
    for (i = 0; i < cur_mod->inc_size && cur_mod->inc[i].submodule; ++i) {
        for (j = 0; j < cur_mod->inc[i].submodule->features_size; ++j) {
            if (!(cur_mod->inc[i].submodule->features[j].flags & LYS_FENABLED)) {
                continue;
            }

            if (!lyd_new_leaf(parent, NULL, "feature", cur_mod->inc[i].submodule->features[j].name)) {
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}

static int
ylib_deviation(struct lyd_node *parent, struct lys_module *cur_mod, int bis)
{
    uint32_t i = 0, j;
    const struct lys_module *mod;
    struct lyd_node *cont;
    const char *ptr;

    if (cur_mod->deviated) {
        while ((mod = ly_ctx_get_module_iter(cur_mod->ctx, &i))) {
            if (mod == cur_mod) {
                continue;
            }

            for (j = 0; j < mod->deviation_size; ++j) {
                ptr = strstr(mod->deviation[j].target_name, cur_mod->name);
                if (ptr && ptr[strlen(cur_mod->name)] == ':') {
                    if (bis) {
                        if (!lyd_new_leaf(parent, NULL, "deviation", mod->name)) {
                            return EXIT_FAILURE;
                        }
                    } else {
                        cont = lyd_new(parent, NULL, "deviation");
                        if (!cont) {
                            return EXIT_FAILURE;
                        }
                        if (!lyd_new_leaf(cont, NULL, "name", mod->name)) {
                            return EXIT_FAILURE;
                        }
                        if (!lyd_new_leaf(cont, NULL, "revision", (mod->rev_size ? mod->rev[0].date : ""))) {
                            return EXIT_FAILURE;
                        }
                    }

                    break;
                }
            }
        }
    }

    return EXIT_SUCCESS;
}

static int
ylib_submodules(struct lyd_node *parent, struct lys_module *cur_mod, int bis)
{
    int i;
    char *str;
    struct lyd_node *item;

    for (i = 0; i < cur_mod->inc_size && cur_mod->inc[i].submodule; ++i) {
        item = lyd_new(parent, NULL, "submodule");
        if (!item) {
            return EXIT_FAILURE;
        }

        if (!lyd_new_leaf(item, NULL, "name", cur_mod->inc[i].submodule->name)) {
            return EXIT_FAILURE;
        }
        if ((!bis || cur_mod->inc[i].submodule->rev_size)
                && !lyd_new_leaf(item, NULL, "revision",
                    (cur_mod->inc[i].submodule->rev_size ? cur_mod->inc[i].submodule->rev[0].date : ""))) {
            return EXIT_FAILURE;
        }
        if (cur_mod->inc[i].submodule->filepath) {
            if (asprintf(&str, "file://%s", cur_mod->inc[i].submodule->filepath) == -1) {
                LOGMEM(cur_mod->ctx);
                return EXIT_FAILURE;
            } else if (!lyd_new_leaf(item, NULL, bis ? "location" : "schema", str)) {
                free(str);
                return EXIT_FAILURE;
            }
            free(str);
        }
    }

    return EXIT_SUCCESS;
}

API uint16_t
ly_ctx_get_module_set_id(const struct ly_ctx *ctx)
{
    FUN_IN;

    return ctx->models.module_set_id;
}

API struct lyd_node *
ly_ctx_info(struct ly_ctx *ctx)
{
    FUN_IN;

    int i, bis = 0;
    char id[8];
    char *str;
    const struct lys_module *mod;
    struct lyd_node *root, *root_bis = NULL, *cont = NULL, *set_bis = NULL;

    if (!ctx) {
        LOGARG;
        return NULL;
    }

    mod = ly_ctx_get_module(ctx, "ietf-yang-library", NULL, 1);
    if (!mod || !mod->data) {
        LOGERR(ctx, LY_EINVAL, "ietf-yang-library is not implemented.");
        return NULL;
    }
    if (mod->rev && !strcmp(mod->rev[0].date, "2016-06-21")) {
        bis = 0;
    } else if (mod->rev && !strcmp(mod->rev[0].date, IETF_YANG_LIB_REV)) {
        bis = 1;
    } else {
        LOGERR(ctx, LY_EINVAL, "Incompatible ietf-yang-library version in context.");
        return NULL;
    }

    root = lyd_new(NULL, mod, "modules-state");
    if (!root) {
        return NULL;
    }

    if (bis) {
        if (!(root_bis = lyd_new(NULL, mod, "yang-library"))) {
            goto error;
        }

        if (!(set_bis = lyd_new(root_bis, NULL, "module-set"))) {
            goto error;
        }

        if (!lyd_new_leaf(set_bis, NULL, "name", "complete")) {
            goto error;
        }
    }

    for (i = 0; i < ctx->models.used; ++i) {
        if (ctx->models.list[i]->disabled) {
            /* skip the disabled modules */
            continue;
        }

        /*
         * deprecated legacy
         */
        cont = lyd_new(root, NULL, "module");
        if (!cont) {
            goto error;
        }
        /* name */
        if (!lyd_new_leaf(cont, NULL, "name", ctx->models.list[i]->name)) {
            goto error;
        }
        /* revision */
        if (!lyd_new_leaf(cont, NULL, "revision", (ctx->models.list[i]->rev_size ? ctx->models.list[i]->rev[0].date : ""))) {
            goto error;
        }
        /* schema */
        if (ctx->models.list[i]->filepath) {
            if (asprintf(&str, "file://%s", ctx->models.list[i]->filepath) == -1) {
                LOGMEM(ctx);
                goto error;
            }
            if (!lyd_new_leaf(cont, NULL, "schema", str)) {
                free(str);
                goto error;
            }
            free(str);
        }
        /* namespace */
        if (!lyd_new_leaf(cont, NULL, "namespace", ctx->models.list[i]->ns)) {
            goto error;
        }
        /* feature leaf-list */
        if (ylib_feature(cont, ctx->models.list[i])) {
            goto error;
        }
        /* deviation list */
        if (ylib_deviation(cont, ctx->models.list[i], 0)) {
            goto error;
        }
        /* conformance-type */
        if (!lyd_new_leaf(cont, NULL, "conformance-type", ctx->models.list[i]->implemented ? "implement" : "import")) {
            goto error;
        }
        /* submodule list */
        if (ylib_submodules(cont, ctx->models.list[i], 0)) {
            goto error;
        }

        /*
         * current revision
         */
        if (bis) {
            if (ctx->models.list[i]->implemented) {
                if (!(cont = lyd_new(set_bis, NULL, "module"))) {
                    goto error;
                }
            } else {
                if (!(cont = lyd_new(set_bis, NULL, "import-only-module"))) {
                    goto error;
                }
            }
            /* name */
            if (!lyd_new_leaf(cont, NULL, "name", ctx->models.list[i]->name)) {
                goto error;
            }
            /* revision */
            if ((!ctx->models.list[i]->implemented || ctx->models.list[i]->rev_size)
                    && !lyd_new_leaf(cont, NULL, "revision", ctx->models.list[i]->rev[0].date)) {
                goto error;
            }
            /* namespace */
            if (!lyd_new_leaf(cont, NULL, "namespace", ctx->models.list[i]->ns)) {
                goto error;
            }
            /* location */
            if (ctx->models.list[i]->filepath) {
                if (asprintf(&str, "file://%s", ctx->models.list[i]->filepath) == -1) {
                    LOGMEM(ctx);
                    goto error;
                }
                if (!lyd_new_leaf(cont, NULL, "location", str)) {
                    free(str);
                    goto error;
                }
                free(str);
            }
            /* submodule list */
            if (ylib_submodules(cont, ctx->models.list[i], 1)) {
                goto error;
            }
            if (ctx->models.list[i]->implemented) {
                /* feature list */
                if (ylib_feature(cont, ctx->models.list[i])) {
                    goto error;
                }
                /* deviation */
                if (ylib_deviation(cont, ctx->models.list[i], 1)) {
                    goto error;
                }
            }
        }
    }

    /* IDs */
    sprintf(id, "%u", ctx->models.module_set_id);
    if (!lyd_new_leaf(root, NULL, "module-set-id", id)) {
        goto error;
    }
    if (bis) {
        /* create one complete schema */
        if (!(cont = lyd_new(root_bis, NULL, "schema"))) {
            goto error;
        }

        if (!lyd_new_leaf(cont, NULL, "name", "complete")) {
            goto error;
        }

        if (!lyd_new_leaf(cont, NULL, "module-set", "complete")) {
            goto error;
        }

        /* content-id */
        if (!lyd_new_leaf(root_bis, NULL, "content-id", id)) {
            goto error;
        }
    }

    if (root_bis) {
        if (lyd_insert_sibling(&root_bis, root)) {
            goto error;
        }
        root = root_bis;
        root_bis = 0;
    }

    if (lyd_validate(&root, LYD_OPT_NOSIBLINGS, NULL)) {
        goto error;
    }

    return root;

error:
    lyd_free_withsiblings(root);
    lyd_free_withsiblings(root_bis);
    return NULL;
}

API const struct lys_node *
ly_ctx_get_node(const struct ly_ctx *ctx, const struct lys_node *start, const char *nodeid, int output)
{
    FUN_IN;

    const struct lys_node *node;

    if ((!ctx && !start) || !nodeid || ((nodeid[0] != '/') && !start)) {
        LOGARG;
        return NULL;
    }

    if (!ctx) {
        ctx = start->module->ctx;
    }

    /* sets error and everything */
    node = resolve_json_nodeid(nodeid, ctx, start, output);

    return node;
}

API struct ly_set *
ly_ctx_find_path(struct ly_ctx *ctx, const char *path)
{
    FUN_IN;

    struct ly_set *resultset = NULL;

    if (!ctx || !path) {
        LOGARG;
        return NULL;
    }

    /* start in internal module without data to make sure that all the nodes are prefixed */
    resolve_schema_nodeid(path, NULL, ctx->models.list[0], &resultset, 1, 1);
    return resultset;
}
