/**
 * @file schema_mount.c
 * @author Tadeas Vintrlik <xvintr04@stud.fit.vutbr.cz>
 * @brief libyang extension plugin - Schema Mount (RFC 8528)
 *
 * Copyright (c) 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"
#include "dict.h"
#include "libyang.h"
#include "log.h"
#include "ly_common.h"
#include "parser_data.h"
#include "plugins_exts.h"
#include "plugins_types.h"
#include "tree_data.h"
#include "tree_schema.h"

/**
 * @brief Internal schema mount data structure for holding all the contexts of parsed data.
 */
struct lyplg_ext_sm {
    pthread_mutex_t lock;               /**< lock for accessing this shared structure */

    struct lyplg_ext_sm_shared {
        struct {
            struct ly_ctx *ctx;         /**< context shared between all data of this mount point */
            const char *mount_point;    /**< mount point name */
            const char *content_id;     /**< yang-library content-id (alternatively module-set-id),
                                             stored in the dictionary of the ext instance context */
        } *schemas;                     /**< array of shared schema schemas */
        uint32_t schema_count;          /**< length of schemas array */
        uint32_t ref_count;             /**< number of references to this structure (mount-points with the same name
                                             in the module) */
    } *shared;                          /**< shared schema mount points */

    struct lyplg_ext_sm_inln {
        struct {
            struct ly_ctx *ctx;         /**< context created for inline schema data, may be reused if possible */
        } *schemas;                     /**< array of inline schemas */
        uint32_t schema_count;          /**< length of schemas array */
    } inln;                             /**< inline mount points */
};

struct sprinter_tree_priv {
    struct ly_ctx *ext_ctx;
    struct ly_set *refs;
};

#define EXT_LOGERR_MEM_RET(cctx, ext) \
        lyplg_ext_compile_log(cctx, ext, LY_LLERR, LY_EMEM, "Memory allocation failed (%s:%d).", __FILE__, __LINE__); \
        return LY_EMEM

#define EXT_LOGERR_MEM_GOTO(cctx, ext, rc, label) \
        lyplg_ext_compile_log(cctx, ext, LY_LLERR, LY_EMEM, "Memory allocation failed (%s:%d).", __FILE__, __LINE__); \
        rc = LY_EMEM; \
        goto label

#define EXT_LOGERR_INT_RET(cctx, ext) \
        lyplg_ext_compile_log(cctx, ext, LY_LLERR, LY_EINT, "Internal error (%s:%d).", __FILE__, __LINE__); \
        return LY_EINT

/**
 * @brief Check if given mount point is unique among its siblings
 *
 * @param[in] pctx Parse context.
 * @param[in] ext Parsed extension instance.
 * @return LY_SUCCESS if is unique;
 * @return LY_EINVAL otherwise.
 */
static LY_ERR
schema_mount_parse_unique_mp(struct lysp_ctx *pctx, const struct lysp_ext_instance *ext)
{
    struct lysp_ext_instance *exts;
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_node *parent;

    /* check if it is the only instance of the mount-point among its siblings */
    parent = ext->parent;
    exts = parent->exts;
    LY_ARRAY_FOR(exts, u) {
        if (&exts[u] == ext) {
            continue;
        }

        if (!strcmp(exts[u].name, ext->name)) {
            lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Multiple extension \"%s\" instances.", ext->name);
            return LY_EINVAL;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Schema mount parse.
 * Checks if it can be a valid extension instance for yang schema mount.
 *
 * Implementation of ::lyplg_ext_parse_clb callback set as lyext_plugin::parse.
 */
static LY_ERR
schema_mount_parse(struct lysp_ctx *pctx, struct lysp_ext_instance *ext)
{
    /* check YANG version 1.1 */
    if (lyplg_ext_parse_get_cur_pmod(pctx)->version != LYS_VERSION_1_1) {
        lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Extension \"%s\" instance not allowed in YANG version 1 module.",
                ext->name);
        return LY_EINVAL;
    }

    /* check parent nodetype */
    if ((ext->parent_stmt != LY_STMT_CONTAINER) && (ext->parent_stmt != LY_STMT_LIST)) {
        lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Extension \"%s\" instance allowed only in container or list statement.",
                ext->name);
        return LY_EINVAL;
    }

    /* check uniqueness */
    if (schema_mount_parse_unique_mp(pctx, ext)) {
        return LY_EINVAL;
    }

    /* nothing to actually parse */
    return LY_SUCCESS;
}

struct lyplg_ext_sm_shared_cb_data {
    const struct lysc_ext_instance *ext;
    struct lyplg_ext_sm_shared *sm_shared;
};

static LY_ERR
schema_mount_compile_mod_dfs_cb(struct lysc_node *node, void *data, ly_bool *UNUSED(dfs_continue))
{
    struct lyplg_ext_sm_shared_cb_data *cb_data = data;
    struct lyplg_ext_sm *sm_data;
    struct lysc_ext_instance *exts;
    LY_ARRAY_COUNT_TYPE u;

    if (node == cb_data->ext->parent) {
        /* parent of the current compiled extension, skip */
        return LY_SUCCESS;
    }

    /* find the same mount point */
    exts = node->exts;
    LY_ARRAY_FOR(exts, u) {
        if (!strcmp(exts[u].def->module->name, "ietf-yang-schema-mount") && !strcmp(exts[u].def->name, "mount-point") &&
                (exts[u].argument == cb_data->ext->argument)) {
            /* same mount point, break the DFS search */
            sm_data = exts[u].compiled;
            cb_data->sm_shared = sm_data->shared;
            return LY_EEXIST;
        }
    }

    /* not found, continue search */
    return LY_SUCCESS;
}

static struct lyplg_ext_sm_shared *
schema_mount_compile_find_shared(const struct lys_module *mod, const struct lysc_ext_instance *ext)
{
    struct lyplg_ext_sm_shared_cb_data cb_data;
    LY_ERR r;

    /* prepare cb_data */
    cb_data.ext = ext;
    cb_data.sm_shared = NULL;

    /* try to find the same mount point */
    r = lysc_module_dfs_full(mod, schema_mount_compile_mod_dfs_cb, &cb_data);
    (void)r;
    assert((!r && !cb_data.sm_shared) || ((r == LY_EEXIST) && cb_data.sm_shared));

    return cb_data.sm_shared;
}

/**
 * @brief Schema mount compile.
 * Checks if it can be a valid extension instance for yang schema mount.
 *
 * Implementation of ::lyplg_ext_compile_clb callback set as lyext_plugin::compile.
 */
static LY_ERR
schema_mount_compile(struct lysc_ctx *cctx, const struct lysp_ext_instance *UNUSED(extp), struct lysc_ext_instance *ext)
{
    const struct lysc_node *node;
    struct lyplg_ext_sm *sm_data;

    /* init internal data */
    sm_data = calloc(1, sizeof *sm_data);
    if (!sm_data) {
        EXT_LOGERR_MEM_RET(cctx, ext);
    }
    pthread_mutex_init(&sm_data->lock, NULL);
    ext->compiled = sm_data;

    /* find the owner module */
    node = ext->parent;
    while (node->parent) {
        node = node->parent;
    }

    /* reuse/init shared schema */
    sm_data->shared = schema_mount_compile_find_shared(node->module, ext);
    if (sm_data->shared) {
        ++sm_data->shared->ref_count;
    } else {
        sm_data->shared = calloc(1, sizeof *sm_data->shared);
        if (!sm_data->shared) {
            free(sm_data);
            EXT_LOGERR_MEM_RET(cctx, ext);
        }
        sm_data->shared->ref_count = 1;
    }

    return LY_SUCCESS;
}

/**
 * @brief Learn details about the current mount point.
 *
 * @param[in] ext Compiled extension instance.
 * @param[in] ext_data Extension data retrieved by the callback.
 * @param[out] config Whether the whole schema should keep its config or be set to false.
 * @param[out] shared Optional flag whether the schema is shared or inline.
 * @return LY_ERR value.
 */
static LY_ERR
schema_mount_get_smount(const struct lysc_ext_instance *ext, const struct lyd_node *ext_data, ly_bool *config,
        ly_bool *shared)
{
    struct lyd_node *mpoint, *node;
    char *path = NULL;
    LY_ERR r;

    /* find the mount point */
    if (asprintf(&path, "/ietf-yang-schema-mount:schema-mounts/mount-point[module='%s'][label='%s']", ext->module->name,
            ext->argument) == -1) {
        EXT_LOGERR_MEM_RET(NULL, ext);
    }
    r = ext_data ? lyd_find_path(ext_data, path, 0, &mpoint) : LY_ENOTFOUND;
    free(path);
    if (r) {
        /* missing mount-point, cannot be data for this extension (https://datatracker.ietf.org/doc/html/rfc8528#page-10) */
        return LY_ENOT;
    }

    /* check config */
    *config = 1;
    if (!lyd_find_path(mpoint, "config", 0, &node) && !strcmp(lyd_get_value(node), "false")) {
        *config = 0;
    }
    assert((ext->parent_stmt == LY_STMT_CONTAINER) || (ext->parent_stmt == LY_STMT_LIST));
    if (((struct lysc_node *)ext->parent)->flags & LYS_CONFIG_R) {
        *config = 0;
    }

    if (shared) {
        /* check schema-ref */
        if (lyd_find_path(mpoint, "shared-schema", 0, NULL)) {
            if (lyd_find_path(mpoint, "inline", 0, NULL)) {
                EXT_LOGERR_INT_RET(NULL, ext);
            }
            *shared = 0;
        } else {
            *shared = 1;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Create schema (context) based on retrieved extension data.
 *
 * @param[in] ext Compiled extension instance.
 * @param[in] ext_data Extension data retrieved by the callback.
 * @param[in] config Whether the whole schema should keep its config or be set to false.
 * @param[out] ext_ctx Schema to use for parsing the data.
 * @return LY_ERR value.
 */
static LY_ERR
schema_mount_create_ctx(const struct lysc_ext_instance *ext, const struct lyd_node *ext_data, ly_bool config,
        struct ly_ctx **ext_ctx)
{
    LY_ERR rc = LY_SUCCESS;
    const char * const *searchdirs;
    char *sdirs = NULL;
    const struct lys_module *mod;
    struct lysc_node *root, *node;
    uint32_t i, idx = 0;

    /* get searchdirs from the current context */
    searchdirs = ly_ctx_get_searchdirs(ext->module->ctx);

    if (searchdirs) {
        /* append them all into a single string */
        for (i = 0; searchdirs[i]; ++i) {
            if ((rc = ly_strcat(&sdirs, "%s" PATH_SEPARATOR, searchdirs[i]))) {
                goto cleanup;
            }
        }
    }

    /* create the context based on the data */
    if ((rc = ly_ctx_new_yldata(sdirs, ext_data, ly_ctx_get_options(ext->module->ctx), ext_ctx))) {
        lyplg_ext_compile_log(NULL, ext, LY_LLERR, rc, "Failed to create context for the schema-mount data.");
        goto cleanup;
    }

    if (!config) {
        /* manually change the config of all schema nodes in all the modules */
        while ((mod = ly_ctx_get_module_iter(*ext_ctx, &idx))) {
            if (!mod->implemented) {
                continue;
            }

            LY_LIST_FOR(mod->compiled->data, root) {
                LYSC_TREE_DFS_BEGIN(root, node) {
                    node->flags &= ~LYS_CONFIG_W;
                    node->flags |= LYS_CONFIG_R;

                    LYSC_TREE_DFS_END(root, node);
                }
            }
        }
    }

cleanup:
    free(sdirs);
    return rc;
}

/**
 * @brief Get ietf-yang-library context-id from its data.
 *
 * @param[in] ext Compiled extension instance for logging.
 * @param[in] ext_data Extension data retrieved by the callback with the yang-library data.
 * @param[out] content_id Content ID in @p ext_data.
 * @return LY_ERR value.
 */
static LY_ERR
schema_mount_get_content_id(struct lysc_ext_instance *ext, const struct lyd_node *ext_data, const char **content_id)
{
    struct lyd_node *node = NULL;

    *content_id = NULL;

    /* get yang-library content-id or module-set-id */
    if (ext_data) {
        lyd_find_path(ext_data, "/ietf-yang-library:yang-library/content-id", 0, &node);
        if (!node) {
            lyd_find_path(ext_data, "/ietf-yang-library:modules-state/module-set-id", 0, &node);
        }
        if (node) {
            *content_id = lyd_get_value(node);
        }
    }

    if (!*content_id) {
        lyplg_ext_compile_log(NULL, ext, LY_LLERR, LY_EVALID,
                "Missing \"content-id\" or \"module-set-id\" in ietf-yang-library data.");
        return LY_EVALID;
    }
    return LY_SUCCESS;
}

/**
 * @brief Get schema (context) for a shared-schema mount point.
 *
 * @param[in] ext Compiled extension instance.
 * @param[in] ext_data Extension data retrieved by the callback.
 * @param[in] config Whether the whole schema should keep its config or be set to false.
 * @param[out] ext_ctx Schema to use for parsing the data.
 * @return LY_ERR value.
 */
static LY_ERR
schema_mount_get_ctx_shared(struct lysc_ext_instance *ext, const struct lyd_node *ext_data, ly_bool config,
        const struct ly_ctx **ext_ctx)
{
    struct lyplg_ext_sm *sm_data = ext->compiled;
    LY_ERR rc = LY_SUCCESS, r;
    struct ly_ctx *new_ctx = NULL;
    uint32_t i;
    const char *content_id;
    void *mem;

    assert(sm_data && sm_data->shared);

    /* get yang-library content-id or module-set-id */
    if ((r = schema_mount_get_content_id(ext, ext_data, &content_id))) {
        return r;
    }

    /* LOCK */
    if ((r = pthread_mutex_lock(&sm_data->lock))) {
        lyplg_ext_compile_log(NULL, ext, LY_LLERR, LY_ESYS, "Mutex lock failed (%s).", strerror(r));
        return LY_ESYS;
    }

    /* try to find this mount point */
    for (i = 0; i < sm_data->shared->schema_count; ++i) {
        if (ext->argument == sm_data->shared->schemas[i].mount_point) {
            break;
        }
    }

    if (i < sm_data->shared->schema_count) {
        /* schema exists already */
        if (strcmp(content_id, sm_data->shared->schemas[i].content_id)) {
            lyplg_ext_compile_log_path("/ietf-yang-library:yang-library/content-id", ext, LY_LLERR, LY_EVALID,
                    "Shared-schema yang-library content-id \"%s\" differs from \"%s\" used previously.",
                    content_id, sm_data->shared->schemas[i].content_id);
            rc = LY_EVALID;
            goto cleanup;
        }
    } else {
        /* no schema found, create it */
        if ((r = schema_mount_create_ctx(ext, ext_data, config, &new_ctx))) {
            rc = r;
            goto cleanup;
        }

        /* new entry */
        mem = realloc(sm_data->shared->schemas, (i + 1) * sizeof *sm_data->shared->schemas);
        if (!mem) {
            ly_ctx_destroy(new_ctx);
            EXT_LOGERR_MEM_GOTO(NULL, ext, rc, cleanup);
        }
        sm_data->shared->schemas = mem;
        ++sm_data->shared->schema_count;

        /* fill entry */
        sm_data->shared->schemas[i].ctx = new_ctx;
        sm_data->shared->schemas[i].mount_point = ext->argument;
        lydict_insert(ext->module->ctx, content_id, 0, &sm_data->shared->schemas[i].content_id);
    }

    /* use the context */
    *ext_ctx = sm_data->shared->schemas[i].ctx;

cleanup:
    /* UNLOCK */
    pthread_mutex_unlock(&sm_data->lock);

    return rc;
}

/**
 * @brief Check whether ietf-yang-library data describe an existing context meaning whether it includes
 * at least exactly all the mentioned modules.
 *
 * @param[in] ext Compiled extension instance for logging.
 * @param[in] ext_data Extension data retrieved by the callback with the yang-library data.
 * @param[in] ctx Context to consider.
 * @return LY_SUCCESS if the context matches.
 * @return LY_ENOT if the context differs.
 * @return LY_ERR on error.
 */
static LY_ERR
schema_mount_ctx_match(struct lysc_ext_instance *ext, const struct lyd_node *ext_data, const struct ly_ctx *ctx)
{
    struct ly_set *impl_mods = NULL, *imp_mods = NULL;
    struct lyd_node *node;
    const struct lys_module *mod;
    const char *name, *revision;
    LY_ERR rc = LY_ENOT, r;
    uint32_t i;

    /* collect all the implemented and imported modules, we do not really care about content-id */
    if (!lyd_find_path(ext_data, "/ietf-yang-library:yang-library/content-id", 0, NULL)) {
        if ((r = lyd_find_xpath(ext_data, "/ietf-yang-library:yang-library/module-set[1]/module", &impl_mods))) {
            rc = r;
            goto cleanup;
        }
        if ((r = lyd_find_xpath(ext_data, "/ietf-yang-library:yang-library/module-set[1]/import-only-module", &imp_mods))) {
            rc = r;
            goto cleanup;
        }
    } else {
        if ((r = lyd_find_xpath(ext_data, "/ietf-yang-library:modules-state/module[conformance-type='implement']", &impl_mods))) {
            rc = r;
            goto cleanup;
        }
        if ((r = lyd_find_xpath(ext_data, "/ietf-yang-library:modules-state/module[conformance-type='import']", &imp_mods))) {
            rc = r;
            goto cleanup;
        }
    }

    if (!impl_mods->count) {
        lyplg_ext_compile_log(NULL, ext, LY_LLERR, LY_EVALID, "No implemented modules included in ietf-yang-library data.");
        rc = LY_EVALID;
        goto cleanup;
    }

    /* check all the implemented modules */
    for (i = 0; i < impl_mods->count; ++i) {
        lyd_find_path(impl_mods->dnodes[i], "name", 0, &node);
        name = lyd_get_value(node);

        lyd_find_path(impl_mods->dnodes[i], "revision", 0, &node);
        if (node && strlen(lyd_get_value(node))) {
            revision = lyd_get_value(node);
        } else {
            revision = NULL;
        }

        if (!(mod = ly_ctx_get_module(ctx, name, revision)) || !mod->implemented) {
            /* unsuitable module */
            goto cleanup;
        }
    }

    /* check all the imported modules */
    for (i = 0; i < imp_mods->count; ++i) {
        lyd_find_path(imp_mods->dnodes[i], "name", 0, &node);
        name = lyd_get_value(node);

        lyd_find_path(imp_mods->dnodes[i], "revision", 0, &node);
        if (node && strlen(lyd_get_value(node))) {
            revision = lyd_get_value(node);
        } else {
            revision = NULL;
        }

        if (!ly_ctx_get_module(ctx, name, revision)) {
            /* unsuitable module */
            goto cleanup;
        }
    }

    /* context matches and can be reused */
    rc = LY_SUCCESS;

cleanup:
    ly_set_free(impl_mods, NULL);
    ly_set_free(imp_mods, NULL);
    return rc;
}

/**
 * @brief Get schema (context) for an inline mount point.
 *
 * @param[in] ext Compiled extension instance.
 * @param[in] ext_data Extension data retrieved by the callback.
 * @param[in] config Whether the whole schema should keep its config or be set to false.
 * @param[out] ext_ctx Schema to use for parsing the data.
 * @return LY_ERR value.
 */
static LY_ERR
schema_mount_get_ctx_inline(struct lysc_ext_instance *ext, const struct lyd_node *ext_data, ly_bool config,
        const struct ly_ctx **ext_ctx)
{
    struct lyplg_ext_sm *sm_data = ext->compiled;
    struct ly_ctx *new_ctx = NULL;
    uint32_t i;
    void *mem;
    LY_ERR rc = LY_SUCCESS, r;

    assert(sm_data && sm_data->shared);

    /* LOCK */
    if ((r = pthread_mutex_lock(&sm_data->lock))) {
        lyplg_ext_compile_log(NULL, ext, LY_LLERR, LY_ESYS, "Mutex lock failed (%s).", strerror(r));
        return LY_ESYS;
    }

    /* try to find a context we can reuse */
    for (i = 0; i < sm_data->inln.schema_count; ++i) {
        r = schema_mount_ctx_match(ext, ext_data, sm_data->inln.schemas[i].ctx);
        if (!r) {
            /* match */
            *ext_ctx = sm_data->inln.schemas[i].ctx;
            goto cleanup;
        } else if (r != LY_ENOT) {
            /* error */
            rc = r;
            goto cleanup;
        }
    }

    /* new schema required, create context */
    if ((r = schema_mount_create_ctx(ext, ext_data, config, &new_ctx))) {
        rc = r;
        goto cleanup;
    }

    /* new entry */
    mem = realloc(sm_data->inln.schemas, (i + 1) * sizeof *sm_data->inln.schemas);
    if (!mem) {
        ly_ctx_destroy(new_ctx);
        EXT_LOGERR_MEM_GOTO(NULL, ext, rc, cleanup);
    }
    sm_data->inln.schemas = mem;
    ++sm_data->inln.schema_count;

    /* fill entry */
    sm_data->inln.schemas[i].ctx = new_ctx;

    /* use the context */
    *ext_ctx = sm_data->inln.schemas[i].ctx;

cleanup:
    /* UNLOCK */
    pthread_mutex_unlock(&sm_data->lock);

    return rc;
}

/**
 * @brief Get schema (context) for a mount point.
 *
 * @param[in] ext Compiled extension instance.
 * @param[out] ext_ctx Schema to use for parsing the data.
 * @return LY_ERR value.
 */
static LY_ERR
schema_mount_get_ctx(struct lysc_ext_instance *ext, const struct ly_ctx **ext_ctx)
{
    LY_ERR ret = LY_SUCCESS, r;
    struct lyd_node *iter, *ext_data = NULL;
    ly_bool ext_data_free = 0, config, shared;

    *ext_ctx = NULL;

    /* get operational data with ietf-yang-library and ietf-yang-schema-mount data */
    if ((r = lyplg_ext_get_data(ext->module->ctx, ext, (void **)&ext_data, &ext_data_free))) {
        ret = r;
        goto cleanup;
    }

    LY_LIST_FOR(ext_data, iter) {
        if (iter->flags & LYD_NEW) {
            /* must be validated for the parent-reference prefix data to be stored */
            lyplg_ext_compile_log(NULL, ext, LY_LLERR, LY_EINVAL, "Provided ext data have not been validated.");
            ret = LY_EINVAL;
            goto cleanup;
        }
    }

    /* learn about this mount point */
    if ((r = schema_mount_get_smount(ext, ext_data, &config, &shared))) {
        ret = r;
        goto cleanup;
    }

    /* create/get the context for parsing the data */
    if (shared) {
        r = schema_mount_get_ctx_shared(ext, ext_data, config, ext_ctx);
    } else {
        r = schema_mount_get_ctx_inline(ext, ext_data, config, ext_ctx);
    }
    if (r) {
        ret = r;
        goto cleanup;
    }

cleanup:
    if (ext_data_free) {
        lyd_free_all(ext_data);
    }
    return ret;
}

/**
 * @brief Snode callback for schema mount.
 * Check if data are valid for schema mount and returns their schema node.
 */
static LY_ERR
schema_mount_snode(struct lysc_ext_instance *ext, const struct lyd_node *parent, const struct lysc_node *sparent,
        const char *prefix, size_t prefix_len, LY_VALUE_FORMAT format, void *prefix_data, const char *name, size_t name_len,
        const struct lysc_node **snode)
{
    LY_ERR r;
    const struct lys_module *mod;
    const struct ly_ctx *ext_ctx = NULL;

    /* get context based on ietf-yang-library data */
    if ((r = schema_mount_get_ctx(ext, &ext_ctx))) {
        return r;
    }

    /* get the module */
    mod = lyplg_type_identity_module(ext_ctx, parent ? parent->schema : sparent, prefix, prefix_len, format, prefix_data);
    if (!mod) {
        return LY_ENOT;
    }

    /* get the top-level schema node */
    *snode = lys_find_child(NULL, mod, name, name_len, 0, 0);
    return *snode ? LY_SUCCESS : LY_ENOT;
}

static LY_ERR
schema_mount_get_parent_ref(const struct lysc_ext_instance *ext, const struct lyd_node *ext_data,
        struct ly_set **set)
{
    LY_ERR ret = LY_SUCCESS;
    char *path = NULL;

    /* get all parent references of this mount point */
    if (asprintf(&path, "/ietf-yang-schema-mount:schema-mounts/mount-point[module='%s'][label='%s']"
            "/shared-schema/parent-reference", ext->module->name, ext->argument) == -1) {
        EXT_LOGERR_MEM_GOTO(NULL, ext, ret, cleanup);
    }
    if ((ret = lyd_find_xpath(ext_data, path, set))) {
        goto cleanup;
    }

cleanup:
    free(path);
    return ret;
}

/**
 * @brief Duplicate all accessible parent references for a shared-schema mount point.
 *
 * @param[in] ext Compiled extension instance.
 * @param[in] ctx_node Context node for evaluating the parent-reference XPath expressions.
 * @param[in] ext_data Extension data retrieved by the callback.
 * @param[in] trg_ctx Mounted data context to use for duplication.
 * @param[out] ref_set Set of all top-level parent-ref subtrees connected to each other, may be empty.
 * @return LY_ERR value.
 */
static LY_ERR
schema_mount_dup_parent_ref(const struct lysc_ext_instance *ext, const struct lyd_node *ctx_node,
        const struct lyd_node *ext_data, const struct ly_ctx *trg_ctx, struct ly_set **ref_set)
{
    LY_ERR ret = LY_SUCCESS;
    char *path = NULL;
    struct ly_set *set = NULL, *par_set = NULL;
    struct lyd_node_term *term;
    struct lyd_node *dup = NULL, *top_node, *first;
    struct lyd_value_xpath10 *xp_val;
    uint32_t i, j;

    *ref_set = NULL;

    if (!ext_data) {
        /* we expect the same ext data as before and there must be some for data to be parsed */
        lyplg_ext_compile_log(NULL, ext, LY_LLERR, LY_EINVAL, "No ext data provided.");
        ret = LY_EINVAL;
        goto cleanup;
    }

    if ((ret = schema_mount_get_parent_ref(ext, ext_data, &set))) {
        goto cleanup;
    }

    /* prepare result set */
    if ((ret = ly_set_new(ref_set))) {
        goto cleanup;
    }

    first = NULL;
    for (i = 0; i < set->count; ++i) {
        term = set->objs[i];

        /* get the referenced nodes (subtrees) */
        LYD_VALUE_GET(&term->value, xp_val);
        if ((ret = lyd_find_xpath3(ctx_node, ctx_node, lyxp_get_expr(xp_val->exp), xp_val->format, xp_val->prefix_data,
                NULL, &par_set))) {
            lyplg_ext_compile_log(NULL, ext, LY_LLERR, ret, "Parent reference \"%s\" evaluation failed.",
                    lyxp_get_expr(xp_val->exp));
            goto cleanup;
        }

        for (j = 0; j < par_set->count; ++j) {
            /* duplicate with parents in the context of the mounted data */
            if ((ret = lyd_dup_single_to_ctx(par_set->dnodes[j], trg_ctx, NULL,
                    LYD_DUP_RECURSIVE | LYD_DUP_WITH_PARENTS | LYD_DUP_WITH_FLAGS | LYD_DUP_NO_EXT, &dup))) {
                goto cleanup;
            }

            /* go top-level */
            while (dup->parent) {
                dup = lyd_parent(dup);
            }

            /* check whether the top-level node exists */
            if (first) {
                if ((ret = lyd_find_sibling_first(first, dup, &top_node)) && (ret != LY_ENOTFOUND)) {
                    goto cleanup;
                }
            } else {
                top_node = NULL;
            }

            if (top_node) {
                /* merge */
                ret = lyd_merge_tree(&first, dup, LYD_MERGE_DESTRUCT);
                dup = NULL;
                if (ret) {
                    goto cleanup;
                }
            } else {
                /* insert */
                if ((ret = lyd_insert_sibling(first, dup, &first))) {
                    goto cleanup;
                }

                /* add into the result set because a new top-level node was added */
                if ((ret = ly_set_add(*ref_set, dup, 1, NULL))) {
                    goto cleanup;
                }
                dup = NULL;
            }
        }
    }

cleanup:
    free(path);
    ly_set_free(set, NULL);
    ly_set_free(par_set, NULL);
    lyd_free_tree(dup);
    if (ret && *ref_set) {
        if ((*ref_set)->count) {
            lyd_free_siblings((*ref_set)->dnodes[0]);
        }
        ly_set_free(*ref_set, NULL);
        *ref_set = NULL;
    }
    return ret;
}

LY_ERR
lyplg_ext_schema_mount_get_parent_ref(const struct lysc_ext_instance *ext, struct ly_set **refs)
{
    LY_ERR rc;
    struct ly_set *pref_set = NULL;
    struct ly_set *snode_set = NULL;
    struct ly_set *results_set = NULL;
    struct lyd_node *ext_data;
    ly_bool ext_data_free;

    /* get operational data with ietf-yang-library and ietf-yang-schema-mount data */
    if ((rc = lyplg_ext_get_data(ext->module->ctx, ext, (void **)&ext_data, &ext_data_free))) {
        return rc;
    }

    LY_CHECK_GOTO(rc = schema_mount_get_parent_ref(ext, ext_data, &pref_set), cleanup);
    if (pref_set->count == 0) {
        goto cleanup;
    }

    LY_CHECK_GOTO(rc = ly_set_new(&results_set), cleanup);

    for (uint32_t i = 0; i < pref_set->count; ++i) {
        struct lyd_node_term *term;
        struct lyd_value_xpath10 *xp_val;
        char *value;
        struct ly_err_item *err;

        term = (struct lyd_node_term *)pref_set->dnodes[i];
        LYD_VALUE_GET(&term->value, xp_val);
        LY_CHECK_GOTO(rc = lyplg_type_print_xpath10_value(xp_val, LY_VALUE_JSON, NULL, &value, &err), cleanup);
        LY_CHECK_ERR_GOTO(rc = lys_find_xpath(ext->module->ctx, NULL, value, 0, &snode_set), free(value), cleanup);
        free(value);
        for (uint32_t sn = 0; sn < snode_set->count; sn++) {
            LY_CHECK_GOTO(rc = ly_set_add(results_set, snode_set->snodes[sn], 0, NULL), cleanup);
        }
        ly_set_free(snode_set, NULL);
        snode_set = NULL;
    }

    *refs = results_set;

cleanup:
    if (rc) {
        ly_set_free(results_set, NULL);
    }
    ly_set_free(snode_set, NULL);
    if (ext_data_free) {
        lyd_free_all(ext_data);
    }
    ly_set_free(pref_set, NULL);

    return rc;
}

/**
 * @brief Validate callback for schema mount.
 */
static LY_ERR
schema_mount_validate(struct lysc_ext_instance *ext, struct lyd_node *sibling, const struct lyd_node *dep_tree,
        enum lyd_type data_type, uint32_t val_opts, struct lyd_node **diff)
{
    LY_ERR ret = LY_SUCCESS;
    uint32_t *prev_lo, temp_lo = LY_LOSTORE_LAST, i;
    const struct ly_err_item *err;
    struct lyd_node *iter, *ext_data = NULL, *ref_first = NULL, *orig_parent = lyd_parent(sibling), *op_tree;
    struct lyd_node *ext_diff = NULL, *diff_parent = NULL;
    ly_bool ext_data_free = 0;
    struct ly_set *ref_set = NULL;

    if (!sibling) {
        /* some data had to be parsed for this callback to be called */
        EXT_LOGERR_INT_RET(NULL, ext);
    }

    /* get operational data with ietf-yang-library and ietf-yang-schema-mount data */
    if ((ret = lyplg_ext_get_data(ext->module->ctx, ext, (void **)&ext_data, &ext_data_free))) {
        goto cleanup;
    }

    LY_LIST_FOR(ext_data, iter) {
        if (iter->flags & LYD_NEW) {
            /* must be validated for the parent-reference prefix data to be stored */
            lyplg_ext_compile_log(NULL, ext, LY_LLERR, LY_EINVAL, "Provided ext data have not been validated.");
            ret = LY_EINVAL;
            goto cleanup;
        }
    }

    /* duplicate the referenced parent nodes into ext context */
    if ((ret = schema_mount_dup_parent_ref(ext, orig_parent, ext_data, LYD_CTX(sibling), &ref_set))) {
        goto cleanup;
    }

    if (data_type != LYD_TYPE_DATA_YANG) {
        /* remember the operation data tree, it may be moved */
        op_tree = sibling;
    }

    /* create accessible tree, remove LYD_EXT to not call this callback recursively */
    LY_CHECK_GOTO(lyd_unlink_siblings(sibling), cleanup);
    LY_LIST_FOR(sibling, iter) {
        iter->flags &= ~LYD_EXT;
    }
    if (ref_set->count) {
        if ((ret = lyd_insert_sibling(sibling, ref_set->dnodes[0], &sibling))) {
            goto cleanup;
        }
    }

    /* only store messages in the context, log as an extension */
    prev_lo = ly_temp_log_options(&temp_lo);

    if (data_type == LYD_TYPE_DATA_YANG) {
        /* validate all the modules with data */
        ret = lyd_validate_all(&sibling, NULL, val_opts | LYD_VALIDATE_PRESENT, diff ? &ext_diff : NULL);
    } else {
        /* validate the operation */
        ret = lyd_validate_op(op_tree, dep_tree, data_type, diff ? &ext_diff : NULL);
    }

    /* restore logging */
    ly_temp_log_options(prev_lo);

    /* restore sibling tree */
    for (i = 0; i < ref_set->count; ++i) {
        if (ref_set->dnodes[i] == sibling) {
            sibling = sibling->next;
        }
        lyd_free_tree(ref_set->dnodes[i]);
    }
    LY_LIST_FOR(sibling, iter) {
        iter->flags |= LYD_EXT;
    }
    lyplg_ext_insert(orig_parent, sibling);

    if (ret) {
        /* log the error in the original context */
        err = ly_err_first(LYD_CTX(sibling));
        if (!err) {
            lyplg_ext_compile_log(NULL, ext, LY_LLERR, ret, "Unknown validation error (err code %d).", ret);
        } else {
            lyplg_ext_compile_log_err(err, ext);
        }
        goto cleanup;
    }

    /* create proper diff */
    if (diff && ext_diff) {
        /* diff nodes from an extension instance */
        LY_LIST_FOR(ext_diff, iter) {
            iter->flags |= LYD_EXT;
        }

        /* create the parent and insert the diff */
        if ((ret = lyd_dup_single(lyd_parent(sibling), NULL, LYD_DUP_WITH_PARENTS | LYD_DUP_NO_META, &diff_parent))) {
            goto cleanup;
        }
        if ((ret = lyplg_ext_insert(diff_parent, ext_diff))) {
            goto cleanup;
        }
        ext_diff = NULL;

        /* go top-level and set the operation */
        while (lyd_parent(diff_parent)) {
            diff_parent = lyd_parent(diff_parent);
        }
        if ((ret = lyd_new_meta(LYD_CTX(diff_parent), diff_parent, NULL, "yang:operation", "none", 0, NULL))) {
            goto cleanup;
        }

        /* finally merge into the global diff */
        if ((ret = lyd_diff_merge_all(diff, diff_parent, LYD_DIFF_MERGE_DEFAULTS))) {
            goto cleanup;
        }
    }

cleanup:
    ly_set_free(ref_set, NULL);
    lyd_free_siblings(ref_first);
    lyd_free_tree(ext_diff);
    lyd_free_all(diff_parent);
    if (ext_data_free) {
        lyd_free_all(ext_data);
    }
    return ret;
}

/**
 * @brief Schema mount compile free.
 *
 * Implementation of ::lyplg_ext_compile_free_clb callback set as ::lyext_plugin::cfree.
 */
static void
schema_mount_cfree(const struct ly_ctx *ctx, struct lysc_ext_instance *ext)
{
    struct lyplg_ext_sm *sm_data = ext->compiled;
    uint32_t i;

    if (!sm_data) {
        return;
    }

    if (!--sm_data->shared->ref_count) {
        for (i = 0; i < sm_data->shared->schema_count; ++i) {
            ly_ctx_destroy(sm_data->shared->schemas[i].ctx);
            lydict_remove(ctx, sm_data->shared->schemas[i].content_id);
        }
        free(sm_data->shared->schemas);
        free(sm_data->shared);
    }

    for (i = 0; i < sm_data->inln.schema_count; ++i) {
        ly_ctx_destroy(sm_data->inln.schemas[i].ctx);
    }
    free(sm_data->inln.schemas);

    pthread_mutex_destroy(&sm_data->lock);
    free(sm_data);
}

LIBYANG_API_DEF LY_ERR
lyplg_ext_schema_mount_create_context(const struct lysc_ext_instance *ext, struct ly_ctx **ctx)
{
    struct lyd_node *ext_data = NULL;
    ly_bool ext_data_free = 0, config;
    LY_ERR rc = LY_SUCCESS;

    if (!ext->module->ctx->ext_clb) {
        return LY_EINVAL;
    }

    if (strcmp(ext->def->module->name, "ietf-yang-schema-mount") || strcmp(ext->def->name, "mount-point")) {
        return LY_EINVAL;
    }

    /* get operational data with ietf-yang-library and ietf-yang-schema-mount data */
    if ((rc = lyplg_ext_get_data(ext->module->ctx, ext, (void **)&ext_data, &ext_data_free))) {
        return rc;
    }

    /* learn about this mount point */
    if ((rc = schema_mount_get_smount(ext, ext_data, &config, NULL))) {
        goto cleanup;
    }

    /* create the context */
    rc = schema_mount_create_ctx(ext, ext_data, config, ctx);

cleanup:
    if (ext_data_free) {
        lyd_free_all(ext_data);
    }
    return rc;
}

static void
schema_mount_spriter_tree_free(void *priv)
{
    struct sprinter_tree_priv *st_priv;

    st_priv = priv;
    ly_set_free(st_priv->refs, NULL);
    ly_ctx_destroy(st_priv->ext_ctx);
    free(st_priv);
}

static LY_ERR
schema_mount_sprinter_tree_cnode_override_mounted(const struct lysc_node *node, const void *UNUSED(plugin_priv),
        ly_bool *UNUSED(skip), const char **UNUSED(flags), const char **add_opts)
{
    if (!node->parent) {
        *add_opts = "/";
    }

    return LY_SUCCESS;
}

static LY_ERR
schema_mount_sprinter_tree_pnode_override_mounted(const struct lysp_node *node, const void *UNUSED(plugin_priv),
        ly_bool *UNUSED(skip), const char **UNUSED(flags), const char **add_opts)
{
    if (!node->parent) {
        *add_opts = "/";
    }

    return LY_SUCCESS;
}

static LY_ERR
schema_mount_sprinter_tree_node_override_parent_refs(const struct lysc_node *node, const void *plugin_priv,
        ly_bool *skip, const char **UNUSED(flags), const char **add_opts)
{
    uint32_t i;
    const struct ly_set *refs;
    const struct lysc_module *mod;
    struct lysc_node *ref, *iter;

    refs = ((struct sprinter_tree_priv *)plugin_priv)->refs;
    mod = node->module->compiled;

    /* Assume the @p node will be skipped. */
    *skip = 1;
    for (i = 0; (i < refs->count) && *skip; i++) {
        ref = refs->snodes[i];
        if (ref->module->compiled != mod) {
            /* parent-reference points to different module */
            continue;
        }

        for (iter = ref; iter; iter = iter->parent) {
            if (iter == node) {
                /* @p node is not skipped because it is parent-rererence node or his parent */
                *skip = 0;
                break;
            }
        }
    }

    if (!*skip && !node->parent) {
        /* top-node has additional opts */
        *add_opts = "@";
    }

    return LY_SUCCESS;
}

/**
 * @brief Schema mount schema parsed tree printer.
 *
 * Implementation of ::lyplg_ext_sprinter_ptree_clb callback set as lyext_plugin::printer_ptree.
 */
static LY_ERR
schema_mount_sprinter_ptree(struct lysp_ext_instance *UNUSED(ext), const struct lyspr_tree_ctx *ctx,
        const char **flags, const char **UNUSED(add_opts))
{
    if (!ctx) {
        *flags = "mp";
    }

    return LY_SUCCESS;
}

/**
 * @brief Schema mount schema compiled tree printer.
 *
 * Implementation of ::lyplg_ext_sprinter_ctree_clb callback set as lyext_plugin::printer_ctree.
 */
static LY_ERR
schema_mount_sprinter_ctree(struct lysc_ext_instance *ext, const struct lyspr_tree_ctx *ctx,
        const char **flags, const char **UNUSED(add_opts))
{
    LY_ERR rc = LY_SUCCESS;
    struct ly_ctx *ext_ctx = NULL;
    const struct lys_module *mod;
    struct ly_set *refs = NULL;
    struct lysc_node *tree1, *tree2;
    uint32_t i, j;
    ly_bool from_parent_ref, is_first;
    struct sprinter_tree_priv *st_priv;

    if (!ctx) {
        *flags = "mp";
        return LY_SUCCESS;
    }

    if (lyplg_ext_schema_mount_create_context(ext, &ext_ctx)) {
        /* Void mount point */
        return LY_SUCCESS;
    }

    rc = lyplg_ext_schema_mount_get_parent_ref(ext, &refs);
    LY_CHECK_GOTO(rc, cleanup);

    /* build new list of modules to print. This list will omit internal
     * modules, modules with no nodes (e.g., iana-if-types) and modules
     * that were loaded as the result of a parent-reference.
     */
    i = ly_ctx_internal_modules_count(ext_ctx);
    while ((mod = ly_ctx_get_module_iter(ext_ctx, &i))) {
        from_parent_ref = 0;

        for (j = 0; refs && j < refs->count; j++) {
            if (!strcmp(mod->ns, refs->snodes[j]->module->ns)) {
                from_parent_ref = 1;
                break;
            }
        }
        if (from_parent_ref) {
            /* Modules loaded as the result of a parent-reference are added later. */
            continue;
        }

        /* Add data nodes, rpcs and notifications. */
        if ((ext_ctx->flags & LY_CTX_SET_PRIV_PARSED) && mod->compiled) {
            /* For compiled module. */
            rc = lyplg_ext_sprinter_ctree_add_nodes(ctx, mod->compiled->data,
                    schema_mount_sprinter_tree_cnode_override_mounted);
            LY_CHECK_GOTO(rc, cleanup);
            if (mod->compiled->rpcs) {
                rc = lyplg_ext_sprinter_ctree_add_nodes(ctx, &mod->compiled->rpcs->node,
                        schema_mount_sprinter_tree_cnode_override_mounted);
            }
            LY_CHECK_GOTO(rc, cleanup);
            if (mod->compiled->notifs) {
                rc = lyplg_ext_sprinter_ctree_add_nodes(ctx, &mod->compiled->notifs->node,
                        schema_mount_sprinter_tree_cnode_override_mounted);
            }
            LY_CHECK_GOTO(rc, cleanup);
        } else {
            /* For parsed module. */
            rc = lyplg_ext_sprinter_ptree_add_nodes(ctx, mod->parsed->data,
                    schema_mount_sprinter_tree_pnode_override_mounted);
            LY_CHECK_GOTO(rc, cleanup);
            if (mod->parsed->rpcs) {
                rc = lyplg_ext_sprinter_ptree_add_nodes(ctx, &mod->parsed->rpcs->node,
                        schema_mount_sprinter_tree_pnode_override_mounted);
            }
            LY_CHECK_GOTO(rc, cleanup);
            if (mod->parsed->notifs) {
                rc = lyplg_ext_sprinter_ptree_add_nodes(ctx, &mod->parsed->notifs->node,
                        schema_mount_sprinter_tree_pnode_override_mounted);
            }
            LY_CHECK_GOTO(rc, cleanup);
        }
    }

    /* Add modules loaded as the result of a parent-reference. */
    for (i = 0; refs && (i < refs->count); i++) {
        tree1 = refs->snodes[i]->module->compiled->data;

        /* Add data nodes from the module only once. */
        is_first = 1;
        for (j = 0; j < i; j++) {
            tree2 = refs->snodes[j]->module->compiled->data;
            if (tree1 == tree2) {
                is_first = 0;
                break;
            }
        }
        if (is_first) {
            /* Add all data nodes but unavailable nodes are skipped in the callback. */
            rc = lyplg_ext_sprinter_ctree_add_nodes(ctx, tree1, schema_mount_sprinter_tree_node_override_parent_refs);
            LY_CHECK_GOTO(rc, cleanup);
        }
    }

    /* add private plugin data */
    st_priv = calloc(1, sizeof(*st_priv));
    LY_CHECK_ERR_GOTO(!st_priv, rc = LY_EMEM, cleanup);
    st_priv->ext_ctx = ext_ctx;
    st_priv->refs = refs;
    rc = lyplg_ext_sprinter_tree_set_priv(ctx, st_priv, schema_mount_spriter_tree_free);

cleanup:
    if (rc) {
        ly_set_free(refs, NULL);
        ly_ctx_destroy(ext_ctx);
    }

    return rc;
}

/**
 * @brief Plugin descriptions for the Yang Schema Mount extension.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_EXTENSIONS = {
 */
const struct lyplg_ext_record plugins_schema_mount[] = {
    {
        .module = "ietf-yang-schema-mount",
        .revision = "2019-01-14",
        .name = "mount-point",

        .plugin.id = "ly2 schema mount v1",
        .plugin.parse = schema_mount_parse,
        .plugin.compile = schema_mount_compile,
        .plugin.printer_info = NULL,
        .plugin.printer_ctree = schema_mount_sprinter_ctree,
        .plugin.printer_ptree = schema_mount_sprinter_ptree,
        .plugin.node = NULL,
        .plugin.snode = schema_mount_snode,
        .plugin.validate = schema_mount_validate,
        .plugin.pfree = NULL,
        .plugin.cfree = schema_mount_cfree
    },
    {0} /* terminating zeroed item */
};
