/**
 * @file context.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Context implementations
 *
 * Copyright (c) 2015 - 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _GNU_SOURCE /* asprintf, strdup */
#if defined (__NetBSD__) || defined (__OpenBSD__)
/* realpath */
#define _XOPEN_SOURCE 1
#define _XOPEN_SOURCE_EXTENDED 1
#endif

#include "context.h"

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common.h"
#include "compat.h"
#include "hash_table.h"
#include "in.h"
#include "parser_data.h"
#include "plugins_internal.h"
#include "plugins_types.h"
#include "schema_compile.h"
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_schema.h"
#include "tree_schema_free.h"
#include "tree_schema_internal.h"

#include "../models/ietf-datastores@2018-02-14.h"
#include "../models/ietf-inet-types@2013-07-15.h"
#include "../models/ietf-yang-library@2019-01-04.h"
#include "../models/ietf-yang-metadata@2016-08-05.h"
#include "../models/ietf-yang-schema-mount@2019-01-14.h"
#include "../models/ietf-yang-structure-ext@2020-06-17.h"
#include "../models/ietf-yang-types@2013-07-15.h"
#include "../models/yang@2022-06-16.h"
#define IETF_YANG_LIB_REV "2019-01-04"

static struct internal_modules_s {
    const char *name;
    const char *revision;
    const char *data;
    ly_bool implemented;
    LYS_INFORMAT format;
} internal_modules[] = {
    {"ietf-yang-metadata", "2016-08-05", (const char *)ietf_yang_metadata_2016_08_05_yang, 0, LYS_IN_YANG},
    {"yang", "2022-06-16", (const char *)yang_2022_06_16_yang, 1, LYS_IN_YANG},
    {"ietf-inet-types", "2013-07-15", (const char *)ietf_inet_types_2013_07_15_yang, 0, LYS_IN_YANG},
    {"ietf-yang-types", "2013-07-15", (const char *)ietf_yang_types_2013_07_15_yang, 0, LYS_IN_YANG},
    {"ietf-yang-schema-mount", "2019-01-14", (const char *)ietf_yang_schema_mount_2019_01_14_yang, 1, LYS_IN_YANG},
    {"ietf-yang-structure-ext", "2020-06-17", (const char *)ietf_yang_structure_ext_2020_06_17_yang, 0, LYS_IN_YANG},
    /* ietf-datastores and ietf-yang-library must be right here at the end of the list! */
    {"ietf-datastores", "2018-02-14", (const char *)ietf_datastores_2018_02_14_yang, 1, LYS_IN_YANG},
    {"ietf-yang-library", IETF_YANG_LIB_REV, (const char *)ietf_yang_library_2019_01_04_yang, 1, LYS_IN_YANG}
};

#define LY_INTERNAL_MODS_COUNT sizeof(internal_modules) / sizeof(struct internal_modules_s)

LIBYANG_API_DEF LY_ERR
ly_ctx_set_searchdir(struct ly_ctx *ctx, const char *search_dir)
{
    struct stat st;
    char *new_dir = NULL;

    LY_CHECK_ARG_RET(ctx, ctx, LY_EINVAL);

    if (search_dir) {
        new_dir = realpath(search_dir, NULL);
        LY_CHECK_ERR_RET(!new_dir,
                LOGERR(ctx, LY_ESYS, "Unable to use search directory \"%s\" (%s).", search_dir, strerror(errno)),
                LY_EINVAL);
        if (strcmp(search_dir, new_dir)) {
            LOGVRB("Search directory string \"%s\" canonized to \"%s\".", search_dir, new_dir);
        }
        LY_CHECK_ERR_RET(access(new_dir, R_OK | X_OK),
                LOGERR(ctx, LY_ESYS, "Unable to fully access search directory \"%s\" (%s).", new_dir, strerror(errno)); free(new_dir),
                LY_EINVAL);
        LY_CHECK_ERR_RET(stat(new_dir, &st),
                LOGERR(ctx, LY_ESYS, "stat() failed for \"%s\" (%s).", new_dir, strerror(errno)); free(new_dir),
                LY_ESYS);
        LY_CHECK_ERR_RET(!S_ISDIR(st.st_mode),
                LOGERR(ctx, LY_ESYS, "Given search directory \"%s\" is not a directory.", new_dir); free(new_dir),
                LY_EINVAL);
        /* avoid path duplication */
        for (uint32_t u = 0; u < ctx->search_paths.count; ++u) {
            if (!strcmp(new_dir, ctx->search_paths.objs[u])) {
                free(new_dir);
                return LY_EEXIST;
            }
        }
        if (ly_set_add(&ctx->search_paths, new_dir, 1, NULL)) {
            free(new_dir);
            return LY_EMEM;
        }

        /* new searchdir - possibly more latest revision available */
        ly_ctx_reset_latests(ctx);

        return LY_SUCCESS;
    } else {
        /* consider that no change is not actually an error */
        return LY_SUCCESS;
    }
}

LIBYANG_API_DEF const char * const *
ly_ctx_get_searchdirs(const struct ly_ctx *ctx)
{
#define LY_CTX_SEARCHDIRS_SIZE_STEP 8
    void **new;

    LY_CHECK_ARG_RET(ctx, ctx, NULL);

    if (ctx->search_paths.count == ctx->search_paths.size) {
        /* not enough space for terminating NULL byte */
        new = realloc(((struct ly_ctx *)ctx)->search_paths.objs,
                (ctx->search_paths.size + LY_CTX_SEARCHDIRS_SIZE_STEP) * sizeof *ctx->search_paths.objs);
        LY_CHECK_ERR_RET(!new, LOGMEM(NULL), NULL);
        ((struct ly_ctx *)ctx)->search_paths.size += LY_CTX_SEARCHDIRS_SIZE_STEP;
        ((struct ly_ctx *)ctx)->search_paths.objs = new;
    }
    /* set terminating NULL byte to the strings list */
    ctx->search_paths.objs[ctx->search_paths.count] = NULL;

    return (const char * const *)ctx->search_paths.objs;
}

LIBYANG_API_DEF LY_ERR
ly_ctx_unset_searchdir(struct ly_ctx *ctx, const char *value)
{
    LY_CHECK_ARG_RET(ctx, ctx, LY_EINVAL);

    if (!ctx->search_paths.count) {
        return LY_SUCCESS;
    }

    if (value) {
        /* remove specific search directory */
        uint32_t index;

        for (index = 0; index < ctx->search_paths.count; ++index) {
            if (!strcmp(value, ctx->search_paths.objs[index])) {
                break;
            }
        }
        if (index == ctx->search_paths.count) {
            LOGARG(ctx, value);
            return LY_EINVAL;
        } else {
            return ly_set_rm_index(&ctx->search_paths, index, free);
        }
    } else {
        /* remove them all */
        ly_set_erase(&ctx->search_paths, free);
        memset(&ctx->search_paths, 0, sizeof ctx->search_paths);
    }

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
ly_ctx_unset_searchdir_last(struct ly_ctx *ctx, uint32_t count)
{
    LY_CHECK_ARG_RET(ctx, ctx, LY_EINVAL);

    for ( ; count > 0 && ctx->search_paths.count; --count) {
        LY_CHECK_RET(ly_set_rm_index(&ctx->search_paths, ctx->search_paths.count - 1, free))
    }

    return LY_SUCCESS;
}

LIBYANG_API_DEF struct lys_module *
ly_ctx_load_module(struct ly_ctx *ctx, const char *name, const char *revision, const char **features)
{
    struct lys_module *mod = NULL;
    LY_ERR ret = LY_SUCCESS;

    LY_CHECK_ARG_RET(ctx, ctx, name, NULL);

    /* load and parse */
    ret = lys_parse_load(ctx, name, revision, &ctx->unres.creating, &mod);
    LY_CHECK_GOTO(ret, cleanup);

    /* implement */
    ret = _lys_set_implemented(mod, features, &ctx->unres);
    LY_CHECK_GOTO(ret, cleanup);

    if (!(ctx->flags & LY_CTX_EXPLICIT_COMPILE)) {
        /* create dep set for the module and mark all the modules that will be (re)compiled */
        LY_CHECK_GOTO(ret = lys_unres_dep_sets_create(ctx, &ctx->unres.dep_sets, mod), cleanup);

        /* (re)compile the whole dep set (other dep sets will have no modules marked for compilation) */
        LY_CHECK_GOTO(ret = lys_compile_depset_all(ctx, &ctx->unres), cleanup);

        /* unres resolved */
        lys_unres_glob_erase(&ctx->unres);
    }

cleanup:
    if (ret) {
        lys_unres_glob_revert(ctx, &ctx->unres);
        lys_unres_glob_erase(&ctx->unres);
        mod = NULL;
    }
    return mod;
}

/**
 * @brief Hash table value-equal callback for comparing context error hash table record.
 */
static ly_bool
ly_ctx_ht_err_equal_cb(void *val1_p, void *val2_p, ly_bool UNUSED(mod), void *UNUSED(cb_data))
{
    struct ly_ctx_err_rec *err1 = val1_p, *err2 = val2_p;

    return !memcmp(&err1->tid, &err2->tid, sizeof err1->tid);
}

LIBYANG_API_DEF LY_ERR
ly_ctx_new(const char *search_dir, uint16_t options, struct ly_ctx **new_ctx)
{
    struct ly_ctx *ctx = NULL;
    struct lys_module *module;
    char *search_dir_list, *sep, *dir;
    const char **imp_f, *all_f[] = {"*", NULL};
    uint32_t i;
    struct ly_in *in = NULL;
    LY_ERR rc = LY_SUCCESS;
    struct lys_glob_unres unres = {0};

    LY_CHECK_ARG_RET(NULL, new_ctx, LY_EINVAL);

    ctx = calloc(1, sizeof *ctx);
    LY_CHECK_ERR_GOTO(!ctx, LOGMEM(NULL); rc = LY_EMEM, cleanup);

    /* dictionary */
    lydict_init(&ctx->dict);

    /* plugins */
    LY_CHECK_ERR_GOTO(lyplg_init(), LOGINT(NULL); rc = LY_EINT, cleanup);

    /* initialize thread-specific error hash table */
    ctx->err_ht = lyht_new(1, sizeof(struct ly_ctx_err_rec), ly_ctx_ht_err_equal_cb, NULL, 1);
    LY_CHECK_ERR_GOTO(!ctx->err_ht, rc = LY_EMEM, cleanup);

    /* init LYB hash lock */
    pthread_mutex_init(&ctx->lyb_hash_lock, NULL);

    /* models list */
    ctx->flags = options;
    if (search_dir) {
        search_dir_list = strdup(search_dir);
        LY_CHECK_ERR_GOTO(!search_dir_list, LOGMEM(NULL); rc = LY_EMEM, cleanup);

        for (dir = search_dir_list; (sep = strchr(dir, PATH_SEPARATOR[0])) != NULL && rc == LY_SUCCESS; dir = sep + 1) {
            *sep = 0;
            rc = ly_ctx_set_searchdir(ctx, dir);
            if (rc == LY_EEXIST) {
                /* ignore duplication */
                rc = LY_SUCCESS;
            }
        }
        if (*dir && (rc == LY_SUCCESS)) {
            rc = ly_ctx_set_searchdir(ctx, dir);
            if (rc == LY_EEXIST) {
                /* ignore duplication */
                rc = LY_SUCCESS;
            }
        }
        free(search_dir_list);

        /* If ly_ctx_set_searchdir() failed, the error is already logged. Just exit */
        LY_CHECK_GOTO(rc, cleanup);
    }
    ctx->change_count = 1;

    if (!(options & LY_CTX_EXPLICIT_COMPILE)) {
        /* use it for creating the initial context */
        ctx->flags |= LY_CTX_EXPLICIT_COMPILE;
    }

    /* create dummy in */
    rc = ly_in_new_memory(internal_modules[0].data, &in);
    LY_CHECK_GOTO(rc, cleanup);

    /* load internal modules */
    for (i = 0; i < ((options & LY_CTX_NO_YANGLIBRARY) ? (LY_INTERNAL_MODS_COUNT - 2) : LY_INTERNAL_MODS_COUNT); i++) {
        ly_in_memory(in, internal_modules[i].data);
        LY_CHECK_GOTO(rc = lys_parse_in(ctx, in, internal_modules[i].format, NULL, NULL, &unres.creating, &module), cleanup);
        if (internal_modules[i].implemented || (ctx->flags & LY_CTX_ALL_IMPLEMENTED)) {
            imp_f = (ctx->flags & LY_CTX_ENABLE_IMP_FEATURES) ? all_f : NULL;
            LY_CHECK_GOTO(rc = lys_implement(module, imp_f, &unres), cleanup);
        }
    }

    if (!(options & LY_CTX_EXPLICIT_COMPILE)) {
        /* compile now */
        LY_CHECK_GOTO(rc = ly_ctx_compile(ctx), cleanup);
        ctx->flags &= ~LY_CTX_EXPLICIT_COMPILE;
    }

cleanup:
    ly_in_free(in, 0);
    lys_unres_glob_erase(&unres);
    if (rc) {
        ly_ctx_destroy(ctx);
    } else {
        *new_ctx = ctx;
    }
    return rc;
}

static LY_ERR
ly_ctx_new_yl_legacy(struct ly_ctx *ctx, const struct lyd_node *yltree)
{
    struct lyd_node *module, *node;
    struct ly_set *set;
    const char **feature_arr = NULL;
    const char *name = NULL, *revision = NULL;
    struct ly_set features = {0};
    ly_bool imported = 0;
    const struct lys_module *mod;
    LY_ERR ret = LY_SUCCESS;
    uint32_t i, j;

    LY_CHECK_RET(ret = lyd_find_xpath(yltree, "/ietf-yang-library:yang-library/modules-state/module", &set));

    /* process the data tree */
    for (i = 0; i < set->count; ++i) {
        module = set->dnodes[i];

        /* initiate */
        revision = NULL;
        name = NULL;
        imported = 0;

        LY_LIST_FOR(lyd_child(module), node) {
            if (!strcmp(node->schema->name, "name")) {
                name = lyd_get_value(node);
            } else if (!strcmp(node->schema->name, "revision")) {
                revision = lyd_get_value(node);
            } else if (!strcmp(node->schema->name, "feature")) {
                LY_CHECK_GOTO(ret = ly_set_add(&features, node, 0, NULL), cleanup);
            } else if (!strcmp(node->schema->name, "conformance-type") &&
                    !strcmp(lyd_get_value(node), "import")) {
                /* imported module - skip it, it will be loaded as a side effect
                 * of loading another module */
                imported = 1;
                break;
            }
        }

        if (imported) {
            continue;
        }

        feature_arr = malloc((features.count + 1) * sizeof *feature_arr);
        LY_CHECK_ERR_GOTO(!feature_arr, ret = LY_EMEM, cleanup);

        /* Parse features into an array of strings */
        for (j = 0; j < features.count; ++j) {
            feature_arr[j] = lyd_get_value(features.dnodes[j]);
        }
        feature_arr[features.count] = NULL;
        ly_set_clean(&features, free);

        /* use the gathered data to load the module */
        mod = ly_ctx_load_module(ctx, name, revision, feature_arr);
        free(feature_arr);
        if (!mod) {
            LOGERR(ctx, LY_EINVAL, "Unable to load module specified by yang library data.");
            ly_set_free(set, free);
            return LY_EINVAL;
        }
    }

cleanup:
    ly_set_clean(&features, free);
    ly_set_free(set, free);
    return ret;
}

LIBYANG_API_DEF LY_ERR
ly_ctx_new_ylpath(const char *search_dir, const char *path, LYD_FORMAT format, int options, struct ly_ctx **ctx)
{
    LY_ERR ret = LY_SUCCESS;
    struct ly_ctx *ctx_yl = NULL;
    struct lyd_node *data_yl = NULL;

    LY_CHECK_ARG_RET(NULL, path, ctx, LY_EINVAL);

    /* create a seperate context for the data */
    LY_CHECK_GOTO(ret = ly_ctx_new(search_dir, 0, &ctx_yl), cleanup);

    /* parse yang library data tree */
    LY_CHECK_GOTO(ret = lyd_parse_data_path(ctx_yl, path, format, 0, LYD_VALIDATE_PRESENT, &data_yl), cleanup);

    /* create the new context */
    ret = ly_ctx_new_yldata(search_dir, data_yl, options, ctx);

cleanup:
    lyd_free_all(data_yl);
    ly_ctx_destroy(ctx_yl);
    return ret;
}

LIBYANG_API_DEF LY_ERR
ly_ctx_new_ylmem(const char *search_dir, const char *data, LYD_FORMAT format, int options, struct ly_ctx **ctx)
{
    LY_ERR ret = LY_SUCCESS;
    struct ly_ctx *ctx_yl = NULL;
    struct lyd_node *data_yl = NULL;

    LY_CHECK_ARG_RET(NULL, data, ctx, LY_EINVAL);

    /* create a seperate context for the data */
    LY_CHECK_GOTO(ret = ly_ctx_new(search_dir, 0, &ctx_yl), cleanup);

    /* parse yang library data tree */
    LY_CHECK_GOTO(ret = lyd_parse_data_mem(ctx_yl, data, format, 0, LYD_VALIDATE_PRESENT, &data_yl), cleanup);

    /* create the new context */
    ret = ly_ctx_new_yldata(search_dir, data_yl, options, ctx);

cleanup:
    lyd_free_all(data_yl);
    ly_ctx_destroy(ctx_yl);
    return ret;
}

LIBYANG_API_DEF LY_ERR
ly_ctx_new_yldata(const char *search_dir, const struct lyd_node *tree, int options, struct ly_ctx **ctx)
{
    const char *name = NULL, *revision = NULL;
    struct lyd_node *module, *node;
    struct ly_set *set = NULL;
    const char **feature_arr = NULL;
    struct ly_set features = {0};
    LY_ERR ret = LY_SUCCESS;
    const struct lys_module *mod;
    struct ly_ctx *ctx_new = NULL;
    ly_bool no_expl_compile = 0;
    uint32_t i, j;

    LY_CHECK_ARG_RET(NULL, tree, ctx, LY_EINVAL);

    /* create a new context */
    if (*ctx == NULL) {
        LY_CHECK_GOTO(ret = ly_ctx_new(search_dir, options, &ctx_new), cleanup);
    } else {
        ctx_new = *ctx;
    }

    /* redundant to compile modules one-by-one */
    if (!(options & LY_CTX_EXPLICIT_COMPILE)) {
        ctx_new->flags |= LY_CTX_EXPLICIT_COMPILE;
        no_expl_compile = 1;
    }

    LY_CHECK_GOTO(ret = lyd_find_xpath(tree, "/ietf-yang-library:yang-library/module-set[1]/module", &set), cleanup);
    if (set->count == 0) {
        /* perhaps a legacy data tree? */
        LY_CHECK_GOTO(ret = ly_ctx_new_yl_legacy(ctx_new, tree), cleanup);
    } else {
        /* process the data tree */
        for (i = 0; i < set->count; ++i) {
            module = set->dnodes[i];

            /* initiate */
            name = NULL;
            revision = NULL;

            /* iterate over data */
            LY_LIST_FOR(lyd_child(module), node) {
                if (!strcmp(node->schema->name, "name")) {
                    name = lyd_get_value(node);
                } else if (!strcmp(node->schema->name, "revision")) {
                    revision = lyd_get_value(node);
                } else if (!strcmp(node->schema->name, "feature")) {
                    LY_CHECK_GOTO(ret = ly_set_add(&features, node, 0, NULL), cleanup);
                }
            }

            feature_arr = malloc((features.count + 1) * sizeof *feature_arr);
            LY_CHECK_ERR_GOTO(!feature_arr, ret = LY_EMEM, cleanup);

            /* parse features into an array of strings */
            for (j = 0; j < features.count; ++j) {
                feature_arr[j] = lyd_get_value(features.dnodes[j]);
            }
            feature_arr[features.count] = NULL;
            ly_set_clean(&features, NULL);

            /* use the gathered data to load the module */
            mod = ly_ctx_load_module(ctx_new, name, revision, feature_arr);
            free(feature_arr);
            if (!mod) {
                LOGERR(*ctx ? *ctx : LYD_CTX(tree), LY_EINVAL, "Unable to load module %s@%s specified by yang library data.",
                        name, revision ? revision : "<none>");
                ret = LY_EINVAL;
                goto cleanup;
            }
        }
    }

    /* compile */
    LY_CHECK_GOTO(ret = ly_ctx_compile(ctx_new), cleanup);

    if (no_expl_compile) {
        /* unset flag */
        ctx_new->flags &= ~LY_CTX_EXPLICIT_COMPILE;
    }

cleanup:
    ly_set_free(set, NULL);
    ly_set_erase(&features, NULL);
    if (*ctx == NULL) {
        *ctx = ctx_new;
        if (ret) {
            ly_ctx_destroy(*ctx);
            *ctx = NULL;
        }
    }
    return ret;
}

LIBYANG_API_DEF LY_ERR
ly_ctx_compile(struct ly_ctx *ctx)
{
    LY_ERR ret = LY_SUCCESS;

    LY_CHECK_ARG_RET(NULL, ctx, LY_EINVAL);

    /* create dep sets and mark all the modules that will be (re)compiled */
    LY_CHECK_GOTO(ret = lys_unres_dep_sets_create(ctx, &ctx->unres.dep_sets, NULL), cleanup);

    /* (re)compile all the dep sets */
    LY_CHECK_GOTO(ret = lys_compile_depset_all(ctx, &ctx->unres), cleanup);

cleanup:
    if (ret) {
        /* revert changes of modules */
        lys_unres_glob_revert(ctx, &ctx->unres);
    }
    lys_unres_glob_erase(&ctx->unres);
    return ret;
}

LIBYANG_API_DEF uint16_t
ly_ctx_get_options(const struct ly_ctx *ctx)
{
    LY_CHECK_ARG_RET(ctx, ctx, 0);

    return ctx->flags;
}

LIBYANG_API_DEF LY_ERR
ly_ctx_set_options(struct ly_ctx *ctx, uint16_t option)
{
    LY_ERR lyrc = LY_SUCCESS;
    struct lys_module *mod;
    uint32_t i;

    LY_CHECK_ARG_RET(ctx, ctx, LY_EINVAL);
    LY_CHECK_ERR_RET((option & LY_CTX_NO_YANGLIBRARY) && !(ctx->flags & LY_CTX_NO_YANGLIBRARY),
            LOGARG(ctx, option), LY_EINVAL);

    if (!(ctx->flags & LY_CTX_SET_PRIV_PARSED) && (option & LY_CTX_SET_PRIV_PARSED)) {
        ctx->flags |= LY_CTX_SET_PRIV_PARSED;
        /* recompile the whole context to set the priv pointers */
        for (i = 0; i < ctx->list.count; ++i) {
            mod = ctx->list.objs[i];
            if (mod->implemented) {
                mod->to_compile = 1;
            }
        }
        lyrc = ly_ctx_compile(ctx);
        if (lyrc) {
            ly_ctx_unset_options(ctx, LY_CTX_SET_PRIV_PARSED);
        }
    }

    /* set the option(s) */
    if (!lyrc) {
        ctx->flags |= option;
    }

    return lyrc;
}

static LY_ERR
lysc_node_clear_priv_dfs_cb(struct lysc_node *node, void *UNUSED(data), ly_bool *UNUSED(dfs_continue))
{
    node->priv = NULL;
    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
ly_ctx_unset_options(struct ly_ctx *ctx, uint16_t option)
{
    LY_ARRAY_COUNT_TYPE u, v;
    const struct lysc_ext_instance *ext;
    struct lysc_node *root;

    LY_CHECK_ARG_RET(ctx, ctx, LY_EINVAL);
    LY_CHECK_ERR_RET(option & LY_CTX_NO_YANGLIBRARY, LOGARG(ctx, option), LY_EINVAL);

    if ((ctx->flags & LY_CTX_SET_PRIV_PARSED) && (option & LY_CTX_SET_PRIV_PARSED)) {
        struct lys_module *mod;
        uint32_t index;

        index = 0;
        while ((mod = ly_ctx_get_module_iter(ctx, &index))) {
            if (!mod->compiled) {
                continue;
            }

            /* set NULL for all ::lysc_node.priv pointers in module */
            lysc_module_dfs_full(mod, lysc_node_clear_priv_dfs_cb, NULL);

            /* set NULL for all ::lysc_node.priv pointers in compiled extension instances */
            LY_ARRAY_FOR(mod->compiled->exts, u) {
                ext = &mod->compiled->exts[u];
                LY_ARRAY_FOR(ext->substmts, v) {
                    if (ext->substmts[v].stmt & LY_STMT_DATA_NODE_MASK) {
                        LY_LIST_FOR(*(struct lysc_node **)ext->substmts[v].storage, root) {
                            lysc_tree_dfs_full(root, lysc_node_clear_priv_dfs_cb, NULL);
                        }
                    }
                }
            }
        }
    }

    /* unset the option(s) */
    ctx->flags &= ~option;

    return LY_SUCCESS;
}

LIBYANG_API_DEF uint16_t
ly_ctx_get_change_count(const struct ly_ctx *ctx)
{
    LY_CHECK_ARG_RET(ctx, ctx, 0);

    return ctx->change_count;
}

LIBYANG_API_DEF uint32_t
ly_ctx_get_modules_hash(const struct ly_ctx *ctx)
{
    const struct lys_module *mod;
    uint32_t i = ly_ctx_internal_modules_count(ctx), hash = 0, fi = 0;
    struct lysp_feature *f = NULL;

    LY_CHECK_ARG_RET(ctx, ctx, 0);

    while ((mod = ly_ctx_get_module_iter(ctx, &i))) {
        /* name */
        hash = lyht_hash_multi(hash, mod->name, strlen(mod->name));

        /* revision */
        if (mod->revision) {
            hash = lyht_hash_multi(hash, mod->revision, strlen(mod->revision));
        }

        /* enabled features */
        while ((f = lysp_feature_next(f, mod->parsed, &fi))) {
            if (f->flags & LYS_FENABLED) {
                hash = lyht_hash_multi(hash, f->name, strlen(f->name));
            }
        }

        /* imported/implemented */
        hash = lyht_hash_multi(hash, (char *)&mod->implemented, sizeof mod->implemented);
    }

    hash = lyht_hash_multi(hash, NULL, 0);
    return hash;
}

LIBYANG_API_DEF ly_module_imp_clb
ly_ctx_get_module_imp_clb(const struct ly_ctx *ctx, void **user_data)
{
    LY_CHECK_ARG_RET(ctx, ctx, NULL);

    if (user_data) {
        *user_data = ctx->imp_clb_data;
    }
    return ctx->imp_clb;
}

LIBYANG_API_DEF void
ly_ctx_set_module_imp_clb(struct ly_ctx *ctx, ly_module_imp_clb clb, void *user_data)
{
    LY_CHECK_ARG_RET(ctx, ctx, );

    ctx->imp_clb = clb;
    ctx->imp_clb_data = user_data;
}

LIBYANG_API_DEF ly_ext_data_clb
ly_ctx_set_ext_data_clb(struct ly_ctx *ctx, ly_ext_data_clb clb, void *user_data)
{
    ly_ext_data_clb prev;

    LY_CHECK_ARG_RET(ctx, ctx, NULL);

    prev = ctx->ext_clb;
    ctx->ext_clb = clb;
    ctx->ext_clb_data = user_data;

    return prev;
}

LIBYANG_API_DEF struct lys_module *
ly_ctx_get_module_iter(const struct ly_ctx *ctx, uint32_t *index)
{
    LY_CHECK_ARG_RET(ctx, ctx, index, NULL);

    if (*index < ctx->list.count) {
        return ctx->list.objs[(*index)++];
    } else {
        return NULL;
    }
}

/**
 * @brief Iterate over the modules in the given context. Returned modules must match the given key at the offset of
 * lysp_module and lysc_module structures (they are supposed to be placed at the same offset in both structures).
 *
 * @param[in] ctx Context where to iterate.
 * @param[in] key Key value to search for.
 * @param[in] key_size Optional length of the @p key. If zero, NULL-terminated key is expected.
 * @param[in] key_offset Key's offset in struct lys_module to get value from the context's modules to match with the key.
 * @param[in,out] Iterator to pass between the function calls. On the first call, the variable is supposed to be
 * initiated to 0. After each call returning a module, the value is greater by 1 than the index of the returned
 * module in the context.
 * @return Module matching the given key, NULL if no such module found.
 */
static struct lys_module *
ly_ctx_get_module_by_iter(const struct ly_ctx *ctx, const char *key, size_t key_size, size_t key_offset, uint32_t *index)
{
    struct lys_module *mod;
    const char *value;

    for ( ; *index < ctx->list.count; ++(*index)) {
        mod = ctx->list.objs[*index];
        value = *(const char **)(((int8_t *)(mod)) + key_offset);
        if ((!key_size && !strcmp(key, value)) || (key_size && !strncmp(key, value, key_size) && (value[key_size] == '\0'))) {
            /* increment index for the next run */
            ++(*index);
            return mod;
        }
    }
    /* done */
    return NULL;
}

/**
 * @brief Unifying function for ly_ctx_get_module() and ly_ctx_get_module_ns()
 * @param[in] ctx Context where to search.
 * @param[in] key Name or Namespace as a search key.
 * @param[in] key_offset Key's offset in struct lys_module to get value from the context's modules to match with the key.
 * @param[in] revision Revision date to match. If NULL, the matching module must have no revision. To search for the latest
 * revision module, use ly_ctx_get_module_latest_by().
 * @return Matching module if any.
 */
static struct lys_module *
ly_ctx_get_module_by(const struct ly_ctx *ctx, const char *key, size_t key_offset, const char *revision)
{
    struct lys_module *mod;
    uint32_t index = 0;

    while ((mod = ly_ctx_get_module_by_iter(ctx, key, 0, key_offset, &index))) {
        if (!revision) {
            if (!mod->revision) {
                /* found requested module without revision */
                return mod;
            }
        } else {
            if (mod->revision && !strcmp(mod->revision, revision)) {
                /* found requested module of the specific revision */
                return mod;
            }
        }
    }

    return NULL;
}

LIBYANG_API_DEF struct lys_module *
ly_ctx_get_module_ns(const struct ly_ctx *ctx, const char *ns, const char *revision)
{
    LY_CHECK_ARG_RET(ctx, ctx, ns, NULL);
    return ly_ctx_get_module_by(ctx, ns, offsetof(struct lys_module, ns), revision);
}

LIBYANG_API_DEF struct lys_module *
ly_ctx_get_module(const struct ly_ctx *ctx, const char *name, const char *revision)
{
    LY_CHECK_ARG_RET(ctx, ctx, name, NULL);
    return ly_ctx_get_module_by(ctx, name, offsetof(struct lys_module, name), revision);
}

/**
 * @brief Unifying function for ly_ctx_get_module_latest() and ly_ctx_get_module_latest_ns()
 * @param[in] ctx Context where to search.
 * @param[in] key Name or Namespace as a search key.
 * @param[in] key_offset Key's offset in struct lys_module to get value from the context's modules to match with the key.
 * @return Matching module if any.
 */
static struct lys_module *
ly_ctx_get_module_latest_by(const struct ly_ctx *ctx, const char *key, size_t key_offset)
{
    struct lys_module *mod;
    uint32_t index = 0;

    while ((mod = ly_ctx_get_module_by_iter(ctx, key, 0, key_offset, &index))) {
        if (mod->latest_revision & LYS_MOD_LATEST_REV) {
            return mod;
        }
    }

    return NULL;
}

LIBYANG_API_DEF struct lys_module *
ly_ctx_get_module_latest(const struct ly_ctx *ctx, const char *name)
{
    LY_CHECK_ARG_RET(ctx, ctx, name, NULL);
    return ly_ctx_get_module_latest_by(ctx, name, offsetof(struct lys_module, name));
}

LIBYANG_API_DEF struct lys_module *
ly_ctx_get_module_latest_ns(const struct ly_ctx *ctx, const char *ns)
{
    LY_CHECK_ARG_RET(ctx, ctx, ns, NULL);
    return ly_ctx_get_module_latest_by(ctx, ns, offsetof(struct lys_module, ns));
}

/**
 * @brief Unifying function for ly_ctx_get_module_implemented() and ly_ctx_get_module_implemented_ns()
 * @param[in] ctx Context where to search.
 * @param[in] key Name or Namespace as a search key.
 * @param[in] key_size Optional length of the @p key. If zero, NULL-terminated key is expected.
 * @param[in] key_offset Key's offset in struct lys_module to get value from the context's modules to match with the key.
 * @return Matching module if any.
 */
static struct lys_module *
ly_ctx_get_module_implemented_by(const struct ly_ctx *ctx, const char *key, size_t key_size, size_t key_offset)
{
    struct lys_module *mod;
    uint32_t index = 0;

    while ((mod = ly_ctx_get_module_by_iter(ctx, key, key_size, key_offset, &index))) {
        if (mod->implemented) {
            return mod;
        }
    }

    return NULL;
}

LIBYANG_API_DEF struct lys_module *
ly_ctx_get_module_implemented(const struct ly_ctx *ctx, const char *name)
{
    LY_CHECK_ARG_RET(ctx, ctx, name, NULL);
    return ly_ctx_get_module_implemented_by(ctx, name, 0, offsetof(struct lys_module, name));
}

struct lys_module *
ly_ctx_get_module_implemented2(const struct ly_ctx *ctx, const char *name, size_t name_len)
{
    LY_CHECK_ARG_RET(ctx, ctx, name, NULL);
    return ly_ctx_get_module_implemented_by(ctx, name, name_len, offsetof(struct lys_module, name));
}

LIBYANG_API_DEF struct lys_module *
ly_ctx_get_module_implemented_ns(const struct ly_ctx *ctx, const char *ns)
{
    LY_CHECK_ARG_RET(ctx, ctx, ns, NULL);
    return ly_ctx_get_module_implemented_by(ctx, ns, 0, offsetof(struct lys_module, ns));
}

/**
 * @brief Try to find a submodule in a module.
 *
 * @param[in] module Module where to search in.
 * @param[in] submodule Name of the submodule to find.
 * @param[in] revision Revision of the submodule to find. NULL for submodule with no revision.
 * @param[in] latest Ignore @p revision and look for the latest revision.
 * @return Pointer to the specified submodule if it is present in the context.
 */
static const struct lysp_submodule *
_ly_ctx_get_submodule2(const struct lys_module *module, const char *submodule, const char *revision, ly_bool latest)
{
    struct lysp_include *inc;
    LY_ARRAY_COUNT_TYPE u;

    LY_CHECK_ARG_RET(NULL, module, module->parsed, submodule, NULL);

    LY_ARRAY_FOR(module->parsed->includes, u) {
        if (module->parsed->includes[u].submodule && !strcmp(submodule, module->parsed->includes[u].submodule->name)) {
            inc = &module->parsed->includes[u];

            if (latest && inc->submodule->latest_revision) {
                /* latest revision */
                return inc->submodule;
            } else if (!revision && !inc->submodule->revs) {
                /* no revision */
                return inc->submodule;
            } else if (revision && inc->submodule->revs && !strcmp(revision, inc->submodule->revs[0].date)) {
                /* specific revision */
                return inc->submodule;
            }
        }
    }

    return NULL;
}

/**
 * @brief Try to find a submodule in the context.
 *
 * @param[in] ctx Context where to search in.
 * @param[in] submodule Name of the submodule to find.
 * @param[in] revision Revision of the submodule to find. NULL for submodule with no revision.
 * @param[in] latest Ignore @p revision and look for the latest revision.
 * @return Pointer to the specified submodule if it is present in the context.
 */
static const struct lysp_submodule *
_ly_ctx_get_submodule(const struct ly_ctx *ctx, const char *submodule, const char *revision, ly_bool latest)
{
    const struct lys_module *mod;
    const struct lysp_submodule *submod = NULL;
    uint32_t v;

    LY_CHECK_ARG_RET(ctx, ctx, submodule, NULL);

    for (v = 0; v < ctx->list.count; ++v) {
        mod = ctx->list.objs[v];
        if (!mod->parsed) {
            continue;
        }

        submod = _ly_ctx_get_submodule2(mod, submodule, revision, latest);
        if (submod) {
            break;
        }
    }

    return submod;
}

LIBYANG_API_DEF const struct lysp_submodule *
ly_ctx_get_submodule(const struct ly_ctx *ctx, const char *submodule, const char *revision)
{
    return _ly_ctx_get_submodule(ctx, submodule, revision, 0);
}

LIBYANG_API_DEF const struct lysp_submodule *
ly_ctx_get_submodule_latest(const struct ly_ctx *ctx, const char *submodule)
{
    return _ly_ctx_get_submodule(ctx, submodule, NULL, 1);
}

LIBYANG_API_DEF const struct lysp_submodule *
ly_ctx_get_submodule2(const struct lys_module *module, const char *submodule, const char *revision)
{
    return _ly_ctx_get_submodule2(module, submodule, revision, 0);
}

LIBYANG_API_DEF const struct lysp_submodule *
ly_ctx_get_submodule2_latest(const struct lys_module *module, const char *submodule)
{
    return _ly_ctx_get_submodule2(module, submodule, NULL, 1);
}

LIBYANG_API_DEF void
ly_ctx_reset_latests(struct ly_ctx *ctx)
{
    struct lys_module *mod;

    for (uint32_t v = 0; v < ctx->list.count; ++v) {
        mod = ctx->list.objs[v];
        mod->latest_revision &= ~(LYS_MOD_LATEST_SEARCHDIRS | LYS_MOD_LATEST_IMPCLB);
        if (mod->parsed && mod->parsed->includes) {
            for (LY_ARRAY_COUNT_TYPE u = 0; u < LY_ARRAY_COUNT(mod->parsed->includes); ++u) {
                mod->parsed->includes[u].submodule->latest_revision &= ~(LYS_MOD_LATEST_SEARCHDIRS | LYS_MOD_LATEST_IMPCLB);
            }
        }
    }
}

LIBYANG_API_DEF uint32_t
ly_ctx_internal_modules_count(const struct ly_ctx *ctx)
{
    if (!ctx) {
        return 0;
    }

    if (ctx->flags & LY_CTX_NO_YANGLIBRARY) {
        return LY_INTERNAL_MODS_COUNT - 2;
    } else {
        return LY_INTERNAL_MODS_COUNT;
    }
}

static LY_ERR
ylib_feature(struct lyd_node *parent, const struct lysp_module *pmod)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_feature *f;

    if (!pmod->mod->implemented) {
        /* no features can be enabled */
        return LY_SUCCESS;
    }

    LY_ARRAY_FOR(pmod->features, struct lysp_feature, f) {
        if (!(f->flags & LYS_FENABLED)) {
            continue;
        }

        LY_CHECK_RET(lyd_new_term(parent, NULL, "feature", f->name, 0, NULL));
    }

    LY_ARRAY_FOR(pmod->includes, u) {
        LY_ARRAY_FOR(pmod->includes[u].submodule->features, struct lysp_feature, f) {
            if (!(f->flags & LYS_FENABLED)) {
                continue;
            }

            LY_CHECK_RET(lyd_new_term(parent, NULL, "feature", f->name, 0, NULL));
        }
    }

    return LY_SUCCESS;
}

static LY_ERR
ylib_deviation(struct lyd_node *parent, const struct lys_module *cur_mod, ly_bool bis)
{
    LY_ARRAY_COUNT_TYPE i;
    struct lys_module *mod;

    if (!cur_mod->implemented) {
        /* no deviations of the module for certain */
        return LY_SUCCESS;
    }

    LY_ARRAY_FOR(cur_mod->deviated_by, i) {
        mod = cur_mod->deviated_by[i];

        if (bis) {
            LY_CHECK_RET(lyd_new_term(parent, NULL, "deviation", mod->name, 0, NULL));
        } else {
            LY_CHECK_RET(lyd_new_list(parent, NULL, "deviation", 0, NULL, mod->name,
                    (mod->parsed->revs ? mod->parsed->revs[0].date : "")));
        }
    }

    return LY_SUCCESS;
}

static LY_ERR
ylib_submodules(struct lyd_node *parent, const struct lysp_module *pmod, ly_bool bis)
{
    LY_ERR ret;
    LY_ARRAY_COUNT_TYPE i;
    struct lyd_node *cont;
    struct lysp_submodule *submod;
    int r;
    char *str;

    LY_ARRAY_FOR(pmod->includes, i) {
        submod = pmod->includes[i].submodule;

        if (bis) {
            LY_CHECK_RET(lyd_new_list(parent, NULL, "submodule", 0, &cont, submod->name));

            if (submod->revs) {
                LY_CHECK_RET(lyd_new_term(cont, NULL, "revision", submod->revs[0].date, 0, NULL));
            }
        } else {
            LY_CHECK_RET(lyd_new_list(parent, NULL, "submodule", 0, &cont, submod->name,
                    (submod->revs ? submod->revs[0].date : "")));
        }

        if (submod->filepath) {
            r = asprintf(&str, "file://%s", submod->filepath);
            LY_CHECK_ERR_RET(r == -1, LOGMEM(pmod->mod->ctx), LY_EMEM);

            ret = lyd_new_term(cont, NULL, bis ? "location" : "schema", str, 0, NULL);
            free(str);
            LY_CHECK_RET(ret);
        }
    }

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
ly_ctx_get_yanglib_data(const struct ly_ctx *ctx, struct lyd_node **root_p, const char *content_id_format, ...)
{
    LY_ERR ret;
    uint32_t i;
    ly_bool bis = 0;
    int r;
    char *str;
    const struct lys_module *mod;
    struct lyd_node *root = NULL, *root_bis = NULL, *cont, *set_bis = NULL;
    va_list ap;

    LY_CHECK_ARG_RET(ctx, ctx, root_p, LY_EINVAL);

    mod = ly_ctx_get_module_implemented(ctx, "ietf-yang-library");
    LY_CHECK_ERR_RET(!mod, LOGERR(ctx, LY_EINVAL, "Module \"ietf-yang-library\" is not implemented."), LY_EINVAL);

    if (mod->parsed->revs && !strcmp(mod->parsed->revs[0].date, "2016-06-21")) {
        bis = 0;
    } else if (mod->parsed->revs && !strcmp(mod->parsed->revs[0].date, IETF_YANG_LIB_REV)) {
        bis = 1;
    } else {
        LOGERR(ctx, LY_EINVAL, "Incompatible ietf-yang-library version in context.");
        return LY_EINVAL;
    }

    LY_CHECK_GOTO(ret = lyd_new_inner(NULL, mod, "modules-state", 0, &root), error);

    if (bis) {
        LY_CHECK_GOTO(ret = lyd_new_inner(NULL, mod, "yang-library", 0, &root_bis), error);
        LY_CHECK_GOTO(ret = lyd_new_list(root_bis, NULL, "module-set", 0, &set_bis, "complete"), error);
    }

    for (i = 0; i < ctx->list.count; ++i) {
        mod = ctx->list.objs[i];
        if (!mod->parsed) {
            LOGERR(ctx, LY_ENOTFOUND, "Parsed module \"%s\" missing in the context.", mod->name);
            goto error;
        }

        /*
         * deprecated legacy
         */
        LY_CHECK_GOTO(ret = lyd_new_list(root, NULL, "module", 0, &cont, mod->name,
                (mod->parsed->revs ? mod->parsed->revs[0].date : "")), error);

        /* schema */
        if (mod->filepath) {
            r = asprintf(&str, "file://%s", mod->filepath);
            LY_CHECK_ERR_GOTO(r == -1, LOGMEM(ctx); ret = LY_EMEM, error);

            ret = lyd_new_term(cont, NULL, "schema", str, 0, NULL);
            free(str);
            LY_CHECK_GOTO(ret, error);
        }

        /* namespace */
        LY_CHECK_GOTO(ret = lyd_new_term(cont, NULL, "namespace", mod->ns, 0, NULL), error);

        /* feature leaf-list */
        LY_CHECK_GOTO(ret = ylib_feature(cont, mod->parsed), error);

        /* deviation list */
        LY_CHECK_GOTO(ret = ylib_deviation(cont, mod, 0), error);

        /* conformance-type */
        LY_CHECK_GOTO(ret = lyd_new_term(cont, NULL, "conformance-type", mod->implemented ? "implement" : "import", 0,
                NULL), error);

        /* submodule list */
        LY_CHECK_GOTO(ret = ylib_submodules(cont, mod->parsed, 0), error);

        /*
         * current revision
         */
        if (bis) {
            /* name and revision */
            if (mod->implemented) {
                LY_CHECK_GOTO(ret = lyd_new_list(set_bis, NULL, "module", 0, &cont, mod->name), error);

                if (mod->parsed->revs) {
                    LY_CHECK_GOTO(ret = lyd_new_term(cont, NULL, "revision", mod->parsed->revs[0].date, 0, NULL), error);
                }
            } else {
                LY_CHECK_GOTO(ret = lyd_new_list(set_bis, NULL, "import-only-module", 0, &cont, mod->name,
                        (mod->parsed->revs ? mod->parsed->revs[0].date : "")), error);
            }

            /* namespace */
            LY_CHECK_GOTO(ret = lyd_new_term(cont, NULL, "namespace", mod->ns, 0, NULL), error);

            /* location */
            if (mod->filepath) {
                r = asprintf(&str, "file://%s", mod->filepath);
                LY_CHECK_ERR_GOTO(r == -1, LOGMEM(ctx); ret = LY_EMEM, error);

                ret = lyd_new_term(cont, NULL, "location", str, 0, NULL);
                free(str);
                LY_CHECK_GOTO(ret, error);
            }

            /* submodule list */
            LY_CHECK_GOTO(ret = ylib_submodules(cont, mod->parsed, 1), error);

            /* feature list */
            LY_CHECK_GOTO(ret = ylib_feature(cont, mod->parsed), error);

            /* deviation */
            LY_CHECK_GOTO(ret = ylib_deviation(cont, mod, 1), error);
        }
    }

    /* IDs */
    va_start(ap, content_id_format);
    r = vasprintf(&str, content_id_format, ap);
    va_end(ap);
    LY_CHECK_ERR_GOTO(r == -1, LOGMEM(ctx); ret = LY_EMEM, error);
    ret = lyd_new_term(root, NULL, "module-set-id", str, 0, NULL);
    LY_CHECK_ERR_GOTO(ret, free(str), error);

    if (bis) {
        /* create one complete schema */
        LY_CHECK_ERR_GOTO(ret = lyd_new_list(root_bis, NULL, "schema", 0, &cont, "complete"), free(str), error);

        LY_CHECK_ERR_GOTO(ret = lyd_new_term(cont, NULL, "module-set", "complete", 0, NULL), free(str), error);

        /* content-id */
        LY_CHECK_ERR_GOTO(ret = lyd_new_term(root_bis, NULL, "content-id", str, 0, NULL), free(str), error);
    }
    free(str);

    if (root_bis) {
        if (lyd_insert_sibling(root, root_bis, &root)) {
            goto error;
        }
        root_bis = NULL;
    }

    LY_CHECK_GOTO(ret = lyd_validate_all(&root, NULL, LYD_VALIDATE_PRESENT, NULL), error);

    *root_p = root;
    return LY_SUCCESS;

error:
    lyd_free_all(root);
    lyd_free_all(root_bis);
    return ret;
}

/**
 * @brief Callback for freeing context error hash table values.
 *
 * @param[in] val_p Pointer to a pointer to an error item to free with all the siblings.
 */
static void
ly_ctx_ht_err_rec_free(void *val_p)
{
    struct ly_ctx_err_rec *err = val_p;

    ly_err_free(err->err);
}

LIBYANG_API_DEF void
ly_ctx_destroy(struct ly_ctx *ctx)
{
    struct lysf_ctx fctx = {.ctx = ctx};

    if (!ctx) {
        return;
    }

    /* models list */
    for ( ; ctx->list.count; ctx->list.count--) {
        fctx.mod = ctx->list.objs[ctx->list.count - 1];

        /* remove the module */
        if (fctx.mod->implemented) {
            fctx.mod->implemented = 0;
            lysc_module_free(&fctx, fctx.mod->compiled);
            fctx.mod->compiled = NULL;
        }
        lys_module_free(&fctx, fctx.mod, 0);
    }
    free(ctx->list.objs);

    /* free extensions */
    lysf_ctx_erase(&fctx);

    /* search paths list */
    ly_set_erase(&ctx->search_paths, free);

    /* leftover unres */
    lys_unres_glob_erase(&ctx->unres);

    /* clean the error hash table */
    lyht_free(ctx->err_ht, ly_ctx_ht_err_rec_free);

    /* dictionary */
    lydict_clean(&ctx->dict);

    /* LYB hash lock */
    pthread_mutex_destroy(&ctx->lyb_hash_lock);

    /* plugins - will be removed only if this is the last context */
    lyplg_clean();

    free(ctx);
}
