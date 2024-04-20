/**
 * @file tree_schema_helpers.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Parsing and validation helper functions for schema trees
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "compat.h"
#include "context.h"
#include "dict.h"
#include "hash_table.h"
#include "in.h"
#include "in_internal.h"
#include "log.h"
#include "parser_schema.h"
#include "schema_compile.h"
#include "schema_features.h"
#include "set.h"
#include "tree.h"
#include "tree_edit.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"

LY_ERR
lysp_check_prefix(struct lys_parser_ctx *ctx, struct lysp_import *imports, const char *module_prefix, const char **value)
{
    struct lysp_import *i;

    if (module_prefix && (&module_prefix != value) && !strcmp(module_prefix, *value)) {
        LOGVAL_PARSER(ctx, LYVE_REFERENCE, "Prefix \"%s\" already used as module prefix.", *value);
        return LY_EEXIST;
    }
    LY_ARRAY_FOR(imports, struct lysp_import, i) {
        if (i->prefix && (&i->prefix != value) && !strcmp(i->prefix, *value)) {
            LOGVAL_PARSER(ctx, LYVE_REFERENCE, "Prefix \"%s\" already used to import \"%s\" module.", *value, i->name);
            return LY_EEXIST;
        }
    }
    return LY_SUCCESS;
}

LY_ERR
lysp_check_date(struct lys_parser_ctx *ctx, const char *date, size_t date_len, const char *stmt)
{
    struct tm tm, tm_;
    char *r;

    LY_CHECK_ARG_RET(ctx ? PARSER_CTX(ctx) : NULL, date, LY_EINVAL);
    LY_CHECK_ERR_RET(date_len != LY_REV_SIZE - 1, LOGARG(ctx ? PARSER_CTX(ctx) : NULL, date_len), LY_EINVAL);

    /* check format: YYYY-MM-DD */
    for (uint8_t i = 0; i < date_len; i++) {
        if ((i == 4) || (i == 7)) {
            if (date[i] != '-') {
                goto error;
            }
        } else if (!isdigit(date[i])) {
            goto error;
        }
    }

    /* check content, e.g. 2018-02-31 */
    memset(&tm, 0, sizeof tm);
    r = strptime(date, "%Y-%m-%d", &tm);
    if (!r || (r != &date[LY_REV_SIZE - 1])) {
        goto error;
    }
    memcpy(&tm_, &tm, sizeof tm);
    mktime(&tm_); /* mktime modifies tm_ if it refers invalid date */
    if (tm.tm_mday != tm_.tm_mday) { /* e.g 2018-02-29 -> 2018-03-01 */
        /* checking days is enough, since other errors
         * have been checked by strptime() */
        goto error;
    }

    return LY_SUCCESS;

error:
    if (stmt) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, date_len, date, stmt);
    }
    return LY_EINVAL;
}

void
lysp_sort_revisions(struct lysp_revision *revs)
{
    LY_ARRAY_COUNT_TYPE i, r;
    struct lysp_revision rev;

    for (i = 1, r = 0; i < LY_ARRAY_COUNT(revs); i++) {
        if (strcmp(revs[i].date, revs[r].date) > 0) {
            r = i;
        }
    }

    if (r) {
        /* the newest revision is not on position 0, switch them */
        memcpy(&rev, &revs[0], sizeof rev);
        memcpy(&revs[0], &revs[r], sizeof rev);
        memcpy(&revs[r], &rev, sizeof rev);
    }
}

static const struct lysp_tpdf *
lysp_type_match(const char *name, struct lysp_node *node)
{
    const struct lysp_tpdf *typedefs;
    LY_ARRAY_COUNT_TYPE u;

    typedefs = lysp_node_typedefs(node);
    LY_ARRAY_FOR(typedefs, u) {
        if (!strcmp(name, typedefs[u].name)) {
            /* match */
            return &typedefs[u];
        }
    }

    return NULL;
}

static LY_DATA_TYPE
lysp_type_str2builtin(const char *name, size_t len)
{
    if (len >= 4) { /* otherwise it does not match any built-in type */
        if (name[0] == 'b') {
            if (name[1] == 'i') {
                if ((len == 6) && !strncmp(&name[2], "nary", 4)) {
                    return LY_TYPE_BINARY;
                } else if ((len == 4) && !strncmp(&name[2], "ts", 2)) {
                    return LY_TYPE_BITS;
                }
            } else if ((len == 7) && !strncmp(&name[1], "oolean", 6)) {
                return LY_TYPE_BOOL;
            }
        } else if (name[0] == 'd') {
            if ((len == 9) && !strncmp(&name[1], "ecimal64", 8)) {
                return LY_TYPE_DEC64;
            }
        } else if (name[0] == 'e') {
            if ((len == 5) && !strncmp(&name[1], "mpty", 4)) {
                return LY_TYPE_EMPTY;
            } else if ((len == 11) && !strncmp(&name[1], "numeration", 10)) {
                return LY_TYPE_ENUM;
            }
        } else if (name[0] == 'i') {
            if (name[1] == 'n') {
                if ((len == 4) && !strncmp(&name[2], "t8", 2)) {
                    return LY_TYPE_INT8;
                } else if (len == 5) {
                    if (!strncmp(&name[2], "t16", 3)) {
                        return LY_TYPE_INT16;
                    } else if (!strncmp(&name[2], "t32", 3)) {
                        return LY_TYPE_INT32;
                    } else if (!strncmp(&name[2], "t64", 3)) {
                        return LY_TYPE_INT64;
                    }
                } else if ((len == 19) && !strncmp(&name[2], "stance-identifier", 17)) {
                    return LY_TYPE_INST;
                }
            } else if ((len == 11) && !strncmp(&name[1], "dentityref", 10)) {
                return LY_TYPE_IDENT;
            }
        } else if (name[0] == 'l') {
            if ((len == 7) && !strncmp(&name[1], "eafref", 6)) {
                return LY_TYPE_LEAFREF;
            }
        } else if (name[0] == 's') {
            if ((len == 6) && !strncmp(&name[1], "tring", 5)) {
                return LY_TYPE_STRING;
            }
        } else if (name[0] == 'u') {
            if (name[1] == 'n') {
                if ((len == 5) && !strncmp(&name[2], "ion", 3)) {
                    return LY_TYPE_UNION;
                }
            } else if ((name[1] == 'i') && (name[2] == 'n') && (name[3] == 't')) {
                if ((len == 5) && (name[4] == '8')) {
                    return LY_TYPE_UINT8;
                } else if (len == 6) {
                    if (!strncmp(&name[4], "16", 2)) {
                        return LY_TYPE_UINT16;
                    } else if (!strncmp(&name[4], "32", 2)) {
                        return LY_TYPE_UINT32;
                    } else if (!strncmp(&name[4], "64", 2)) {
                        return LY_TYPE_UINT64;
                    }
                }
            }
        }
    }

    return LY_TYPE_UNKNOWN;
}

LY_ERR
lysp_type_find(const char *id, struct lysp_node *start_node, const struct lysp_module *start_module,
        LY_DATA_TYPE *type, const struct lysp_tpdf **tpdf, struct lysp_node **node)
{
    const char *str, *name;
    struct lysp_tpdf *typedefs;
    const struct lys_module *mod;
    const struct lysp_module *local_module;
    LY_ARRAY_COUNT_TYPE u, v;

    assert(id);
    assert(start_module);
    assert(tpdf);
    assert(node);

    *node = NULL;
    str = strchr(id, ':');
    if (str) {
        mod = ly_resolve_prefix(start_module->mod->ctx, id, str - id, LY_VALUE_SCHEMA, (void *)start_module);
        local_module = mod ? mod->parsed : NULL;
        name = str + 1;
        *type = LY_TYPE_UNKNOWN;
    } else {
        local_module = start_module;
        name = id;

        /* check for built-in types */
        *type = lysp_type_str2builtin(name, strlen(name));
        if (*type) {
            *tpdf = NULL;
            return LY_SUCCESS;
        }
    }
    LY_CHECK_RET(!local_module, LY_ENOTFOUND);

    if (start_node && (local_module == start_module)) {
        /* search typedefs in parent's nodes */
        *node = start_node;
        while (*node) {
            *tpdf = lysp_type_match(name, *node);
            if (*tpdf) {
                /* match */
                return LY_SUCCESS;
            }
            *node = (*node)->parent;
        }
    }

    /* search in top-level typedefs */
    if (local_module->typedefs) {
        LY_ARRAY_FOR(local_module->typedefs, u) {
            if (!strcmp(name, local_module->typedefs[u].name)) {
                /* match */
                *tpdf = &local_module->typedefs[u];
                return LY_SUCCESS;
            }
        }
    }

    /* search in submodules' typedefs */
    LY_ARRAY_FOR(local_module->includes, u) {
        typedefs = local_module->includes[u].submodule->typedefs;
        LY_ARRAY_FOR(typedefs, v) {
            if (!strcmp(name, typedefs[v].name)) {
                /* match */
                *tpdf = &typedefs[v];
                return LY_SUCCESS;
            }
        }
    }

    return LY_ENOTFOUND;
}

LY_ERR
lysp_check_enum_name(struct lys_parser_ctx *ctx, const char *name, size_t name_len)
{
    if (!name_len) {
        LOGVAL_PARSER(ctx, LYVE_SYNTAX_YANG, "Enum name must not be zero-length.");
        return LY_EVALID;
    } else if (isspace(name[0]) || isspace(name[name_len - 1])) {
        LOGVAL_PARSER(ctx, LYVE_SYNTAX_YANG, "Enum name must not have any leading or trailing whitespaces (\"%.*s\").",
                (int)name_len, name);
        return LY_EVALID;
    } else {
        for (size_t u = 0; u < name_len; ++u) {
            if (iscntrl(name[u])) {
                LOGWRN(PARSER_CTX(ctx), "Control characters in enum name should be avoided (\"%.*s\", character number %d).",
                        (int)name_len, name, u + 1);
                break;
            }
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Check name of a new type to avoid name collisions.
 *
 * @param[in] ctx Parser context, module where the type is being defined is taken from here.
 * @param[in] node Schema node where the type is being defined, NULL in case of a top-level typedef.
 * @param[in] tpdf Typedef definition to check.
 * @param[in,out] tpdfs_global Initialized hash table to store temporary data between calls. When the module's
 *            typedefs are checked, caller is supposed to free the table.
 * @param[in,out] tpdfs_global Initialized hash table to store temporary data between calls. When the module's
 *            typedefs are checked, caller is supposed to free the table.
 * @return LY_EEXIST in case of collision, LY_SUCCESS otherwise.
 */
static LY_ERR
lysp_check_dup_typedef(struct lys_parser_ctx *ctx, struct lysp_node *node, const struct lysp_tpdf *tpdf,
        struct hash_table *tpdfs_global, struct hash_table *tpdfs_scoped)
{
    struct lysp_node *parent;
    uint32_t hash;
    size_t name_len;
    const char *name;
    LY_ARRAY_COUNT_TYPE u;
    const struct lysp_tpdf *typedefs;

    assert(ctx);
    assert(tpdf);

    name = tpdf->name;
    name_len = strlen(name);

    if (lysp_type_str2builtin(name, name_len)) {
        LOGVAL_PARSER(ctx, LYVE_SYNTAX_YANG, "Invalid name \"%s\" of typedef - name collision with a built-in type.", name);
        return LY_EEXIST;
    }

    /* check locally scoped typedefs (avoid name shadowing) */
    if (node) {
        typedefs = lysp_node_typedefs(node);
        LY_ARRAY_FOR(typedefs, u) {
            if (&typedefs[u] == tpdf) {
                break;
            }
            if (!strcmp(name, typedefs[u].name)) {
                LOGVAL_PARSER(ctx, LYVE_SYNTAX_YANG, "Invalid name \"%s\" of typedef - name collision with sibling type.", name);
                return LY_EEXIST;
            }
        }
        /* search typedefs in parent's nodes */
        for (parent = node->parent; parent; parent = parent->parent) {
            if (lysp_type_match(name, parent)) {
                LOGVAL_PARSER(ctx, LYVE_SYNTAX_YANG, "Invalid name \"%s\" of typedef - name collision with another scoped type.", name);
                return LY_EEXIST;
            }
        }
    }

    /* check collision with the top-level typedefs */
    hash = dict_hash(name, name_len);
    if (node) {
        lyht_insert(tpdfs_scoped, &name, hash, NULL);
        if (!lyht_find(tpdfs_global, &name, hash, NULL)) {
            LOGVAL_PARSER(ctx, LYVE_SYNTAX_YANG, "Invalid name \"%s\" of typedef - scoped type collide with a top-level type.", name);
            return LY_EEXIST;
        }
    } else {
        if (lyht_insert(tpdfs_global, &name, hash, NULL)) {
            LOGVAL_PARSER(ctx, LYVE_SYNTAX_YANG, "Invalid name \"%s\" of typedef - name collision with another top-level type.", name);
            return LY_EEXIST;
        }
        /* it is not necessary to test collision with the scoped types - in lysp_check_typedefs, all the
         * top-level typedefs are inserted into the tables before the scoped typedefs, so the collision
         * is detected in the first branch few lines above */
    }

    return LY_SUCCESS;
}

/**
 * @brief Compare identifiers.
 * Implementation of ::lyht_value_equal_cb.
 */
static ly_bool
lysp_id_cmp(void *val1, void *val2, ly_bool UNUSED(mod), void *UNUSED(cb_data))
{
    return strcmp(val1, val2) == 0 ? 1 : 0;
}

LY_ERR
lysp_check_dup_typedefs(struct lys_parser_ctx *ctx, struct lysp_module *mod)
{
    struct hash_table *ids_global;
    struct hash_table *ids_scoped;
    const struct lysp_tpdf *typedefs;
    LY_ARRAY_COUNT_TYPE u, v;
    uint32_t i;
    LY_ERR ret = LY_SUCCESS;

    /* check name collisions - typedefs and groupings */
    ids_global = lyht_new(LYHT_MIN_SIZE, sizeof(char *), lysp_id_cmp, NULL, 1);
    ids_scoped = lyht_new(LYHT_MIN_SIZE, sizeof(char *), lysp_id_cmp, NULL, 1);
    LY_ARRAY_FOR(mod->typedefs, v) {
        ret = lysp_check_dup_typedef(ctx, NULL, &mod->typedefs[v], ids_global, ids_scoped);
        LY_CHECK_GOTO(ret, cleanup);
    }
    LY_ARRAY_FOR(mod->includes, v) {
        LY_ARRAY_FOR(mod->includes[v].submodule->typedefs, u) {
            ret = lysp_check_dup_typedef(ctx, NULL, &mod->includes[v].submodule->typedefs[u], ids_global, ids_scoped);
            LY_CHECK_GOTO(ret, cleanup);
        }
    }
    for (i = 0; i < ctx->tpdfs_nodes.count; ++i) {
        typedefs = lysp_node_typedefs((struct lysp_node *)ctx->tpdfs_nodes.objs[i]);
        LY_ARRAY_FOR(typedefs, u) {
            ret = lysp_check_dup_typedef(ctx, (struct lysp_node *)ctx->tpdfs_nodes.objs[i], &typedefs[u], ids_global, ids_scoped);
            LY_CHECK_GOTO(ret, cleanup);
        }
    }

cleanup:
    lyht_free(ids_global);
    lyht_free(ids_scoped);
    return ret;
}

static ly_bool
ly_ptrequal_cb(void *val1_p, void *val2_p, ly_bool UNUSED(mod), void *UNUSED(cb_data))
{
    void *ptr1 = *((void **)val1_p), *ptr2 = *((void **)val2_p);

    return ptr1 == ptr2 ? 1 : 0;
}

LY_ERR
lysp_check_dup_features(struct lys_parser_ctx *ctx, struct lysp_module *mod)
{
    LY_ARRAY_COUNT_TYPE u;
    struct hash_table *ht;
    struct lysp_feature *f;
    uint32_t hash;
    LY_ERR ret = LY_SUCCESS, r;

    ht = lyht_new(1, sizeof(void *), ly_ptrequal_cb, NULL, 1);
    LY_CHECK_RET(!ht, LY_EMEM);

    /* add all module features into a hash table */
    LY_ARRAY_FOR(mod->features, struct lysp_feature, f) {
        hash = dict_hash(f->name, strlen(f->name));
        r = lyht_insert(ht, &f->name, hash, NULL);
        if (r == LY_EEXIST) {
            LOGVAL_PARSER(ctx, LY_VCODE_DUPIDENT, f->name, "feature");
            ret = LY_EVALID;
            goto cleanup;
        } else if (r) {
            ret = r;
            goto cleanup;
        }
    }

    /* add all submodule features into a hash table */
    LY_ARRAY_FOR(mod->includes, u) {
        LY_ARRAY_FOR(mod->includes[u].submodule->features, struct lysp_feature, f) {
            hash = dict_hash(f->name, strlen(f->name));
            r = lyht_insert(ht, &f->name, hash, NULL);
            if (r == LY_EEXIST) {
                LOGVAL_PARSER(ctx, LY_VCODE_DUPIDENT, f->name, "feature");
                ret = LY_EVALID;
                goto cleanup;
            } else if (r) {
                ret = r;
                goto cleanup;
            }
        }
    }

cleanup:
    lyht_free(ht);
    return ret;
}

LY_ERR
lysp_check_dup_identities(struct lys_parser_ctx *ctx, struct lysp_module *mod)
{
    LY_ARRAY_COUNT_TYPE u;
    struct hash_table *ht;
    struct lysp_ident *i;
    uint32_t hash;
    LY_ERR ret = LY_SUCCESS, r;

    ht = lyht_new(1, sizeof(void *), ly_ptrequal_cb, NULL, 1);
    LY_CHECK_RET(!ht, LY_EMEM);

    /* add all module identities into a hash table */
    LY_ARRAY_FOR(mod->identities, struct lysp_ident, i) {
        hash = dict_hash(i->name, strlen(i->name));
        r = lyht_insert(ht, &i->name, hash, NULL);
        if (r == LY_EEXIST) {
            LOGVAL_PARSER(ctx, LY_VCODE_DUPIDENT, i->name, "identity");
            ret = LY_EVALID;
            goto cleanup;
        } else if (r) {
            ret = r;
            goto cleanup;
        }
    }

    /* add all submodule identities into a hash table */
    LY_ARRAY_FOR(mod->includes, u) {
        LY_ARRAY_FOR(mod->includes[u].submodule->identities, struct lysp_ident, i) {
            hash = dict_hash(i->name, strlen(i->name));
            r = lyht_insert(ht, &i->name, hash, NULL);
            if (r == LY_EEXIST) {
                LOGVAL_PARSER(ctx, LY_VCODE_DUPIDENT, i->name, "identity");
                ret = LY_EVALID;
                goto cleanup;
            } else if (r) {
                ret = r;
                goto cleanup;
            }
        }
    }

cleanup:
    lyht_free(ht);
    return ret;
}

struct lysp_load_module_check_data {
    const char *name;
    const char *revision;
    const char *path;
    const char *submoduleof;
};

static LY_ERR
lysp_load_module_check(const struct ly_ctx *ctx, struct lysp_module *mod, struct lysp_submodule *submod, void *data)
{
    struct lysp_load_module_check_data *info = data;
    const char *filename, *dot, *rev, *name;
    uint8_t latest_revision;
    size_t len;
    struct lysp_revision *revs;

    name = mod ? mod->mod->name : submod->name;
    revs = mod ? mod->revs : submod->revs;
    latest_revision = mod ? mod->mod->latest_revision : submod->latest_revision;

    if (info->name) {
        /* check name of the parsed model */
        if (strcmp(info->name, name)) {
            LOGERR(ctx, LY_EINVAL, "Unexpected module \"%s\" parsed instead of \"%s\").", name, info->name);
            return LY_EINVAL;
        }
    }
    if (info->revision) {
        /* check revision of the parsed model */
        if (!revs || strcmp(info->revision, revs[0].date)) {
            LOGERR(ctx, LY_EINVAL, "Module \"%s\" parsed with the wrong revision (\"%s\" instead \"%s\").", name,
                    revs ? revs[0].date : "none", info->revision);
            return LY_EINVAL;
        }
    } else if (!latest_revision) {
        /* do not log, we just need to drop the schema and use the latest revision from the context */
        return LY_EEXIST;
    }
    if (submod) {
        assert(info->submoduleof);

        /* check that the submodule belongs-to our module */
        if (strcmp(info->submoduleof, submod->mod->name)) {
            LOGVAL(ctx, LYVE_REFERENCE, "Included \"%s\" submodule from \"%s\" belongs-to a different module \"%s\".",
                    submod->name, info->submoduleof, submod->mod->name);
            return LY_EVALID;
        }
        /* check circular dependency */
        if (submod->parsing) {
            LOGVAL(ctx, LYVE_REFERENCE, "A circular dependency (include) for module \"%s\".", submod->name);
            return LY_EVALID;
        }
    }
    if (info->path) {
        /* check that name and revision match filename */
        filename = strrchr(info->path, '/');
        if (!filename) {
            filename = info->path;
        } else {
            filename++;
        }
        /* name */
        len = strlen(name);
        rev = strchr(filename, '@');
        dot = strrchr(info->path, '.');
        if (strncmp(filename, name, len) ||
                ((rev && (rev != &filename[len])) || (!rev && (dot != &filename[len])))) {
            LOGWRN(ctx, "File name \"%s\" does not match module name \"%s\".", filename, name);
        }
        /* revision */
        if (rev) {
            len = dot - ++rev;
            if (!revs || (len != LY_REV_SIZE - 1) || strncmp(revs[0].date, rev, len)) {
                LOGWRN(ctx, "File name \"%s\" does not match module revision \"%s\".", filename,
                        revs ? revs[0].date : "none");
            }
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Load the (sub)module into the context.
 *
 * This function does not check the presence of the (sub)module in context, it should be done before calling this function.
 *
 * module_name and submodule_name are alternatives - only one of the
 *
 * @param[in] ctx libyang context where to work.
 * @param[in] name Name of the (sub)module to load.
 * @param[in] revision Optional revision of the (sub)module to load, if NULL the newest revision is being loaded.
 * @param[in] features Array of enabled features ended with NULL.
 * @param[in] need_implemented Whether the (sub)module is needed implemented or not.
 * @param[in] main_ctx Parser context of the main module in case of loading submodule.
 * @param[in] main_name Main module name in case of loading submodule.
 * @param[in] required Module is required so error (even if the input file not found) are important. If 0, there is some
 * backup and it is actually ok if the input data are not found. However, parser reports errors even in this case.
 * @param[in,out] unres Global unres structure for newly implemented modules.
 * @param[out] result Parsed YANG schema tree of the requested module (struct lys_module*) or submodule (struct lysp_submodule*).
 * If it is a module, it is already in the context!
 * @return LY_ERR value, in case of LY_SUCCESS, the \arg result is always provided.
 */
static LY_ERR
lys_module_localfile(struct ly_ctx *ctx, const char *name, const char *revision, const char **features,
        ly_bool need_implemented, struct lys_parser_ctx *main_ctx, const char *main_name, ly_bool required,
        struct lys_glob_unres *unres, void **result)
{
    struct ly_in *in;
    char *filepath = NULL;
    LYS_INFORMAT format;
    void *mod = NULL;
    LY_ERR ret = LY_SUCCESS;
    struct lysp_load_module_check_data check_data = {0};

    LY_CHECK_RET(lys_search_localfile(ly_ctx_get_searchdirs(ctx), !(ctx->flags & LY_CTX_DISABLE_SEARCHDIR_CWD), name, revision,
            &filepath, &format));
    if (!filepath) {
        if (required) {
            LOGERR(ctx, LY_ENOTFOUND, "Data model \"%s%s%s\" not found in local searchdirs.", name, revision ? "@" : "",
                    revision ? revision : "");
        }
        return LY_ENOTFOUND;
    }

    LOGVRB("Loading schema from \"%s\" file.", filepath);

    /* get the (sub)module */
    LY_CHECK_ERR_GOTO(ret = ly_in_new_filepath(filepath, 0, &in),
            LOGERR(ctx, ret, "Unable to create input handler for filepath %s.", filepath), cleanup);
    check_data.name = name;
    check_data.revision = revision;
    check_data.path = filepath;
    check_data.submoduleof = main_name;
    if (main_ctx) {
        ret = lys_parse_submodule(ctx, in, format, main_ctx, lysp_load_module_check, &check_data,
                (struct lysp_submodule **)&mod);
    } else {
        ret = lys_create_module(ctx, in, format, need_implemented, lysp_load_module_check, &check_data, features, unres,
                (struct lys_module **)&mod);

    }
    ly_in_free(in, 1);
    LY_CHECK_GOTO(ret, cleanup);

    *result = mod;

    /* success */

cleanup:
    free(filepath);
    return ret;
}

LY_ERR
lys_load_module(struct ly_ctx *ctx, const char *name, const char *revision, ly_bool need_implemented,
        const char **features, struct lys_glob_unres *unres, struct lys_module **mod)
{
    const char *module_data = NULL;
    LYS_INFORMAT format = LYS_IN_UNKNOWN;

    void (*module_data_free)(void *module_data, void *user_data) = NULL;
    struct lysp_load_module_check_data check_data = {0};
    struct lys_module *ctx_latest = NULL, *m;
    struct ly_in *in;
    LY_ERR ret;
    ly_bool implement;

    assert(mod && unres);

    if (ctx->flags & LY_CTX_ALL_IMPLEMENTED) {
        implement = 1;
    } else {
        implement = need_implemented;
    }

    /*
     * try to get the module from the context
     */
    if (!*mod) {
        if (revision) {
            /* get the specific revision */
            *mod = (struct lys_module *)ly_ctx_get_module(ctx, name, revision);
        } else {
            if (implement) {
                /* prefer the implemented module instead of the latest one */
                *mod = (struct lys_module *)ly_ctx_get_module_implemented(ctx, name);
            }
            if (!*mod) {
                /* get the requested module of the latest revision in the context */
                *mod = (struct lys_module *)ly_ctx_get_module_latest(ctx, name);
                if (*mod && ((*mod)->latest_revision == 1)) {
                    /* let us now search with callback and searchpaths to check if there is newer revision outside the context */
                    ctx_latest = *mod;
                    *mod = NULL;
                }
            }
        }
    }

    /* check collision with other implemented revision */
    if (implement) {
        m = ly_ctx_get_module_implemented(ctx, name);
        if (m && (!*mod || (*mod && (m != *mod)))) {
            LOGVAL(ctx, LYVE_REFERENCE, "Module \"%s\" is already present in other implemented revision.", name);
            *mod = NULL;
            return LY_EDENIED;
        }
    }

    /*
     * no suitable module in the context, try to load it
     */
    if (!*mod) {
        /* module not present in the context, get the input data and parse it */
        if (!(ctx->flags & LY_CTX_PREFER_SEARCHDIRS)) {
search_clb:
            if (ctx->imp_clb) {
                if (ctx->imp_clb(name, revision, NULL, NULL, ctx->imp_clb_data,
                        &format, &module_data, &module_data_free) == LY_SUCCESS) {
                    LY_CHECK_RET(ly_in_new_memory(module_data, &in));
                    check_data.name = name;
                    check_data.revision = revision;
                    lys_create_module(ctx, in, format, implement, lysp_load_module_check, &check_data, features, unres, mod);
                    ly_in_free(in, 0);
                    if (module_data_free) {
                        module_data_free((void *)module_data, ctx->imp_clb_data);
                    }
                }
            }
            if (!(*mod) && !(ctx->flags & LY_CTX_PREFER_SEARCHDIRS)) {
                goto search_file;
            }
        } else {
search_file:
            if (!(ctx->flags & LY_CTX_DISABLE_SEARCHDIRS)) {
                /* module was not received from the callback or there is no callback set */
                lys_module_localfile(ctx, name, revision, features, implement, NULL, NULL, ctx_latest ? 0 : 1, unres,
                        (void **)mod);
            }
            if (!*mod && (ctx->flags & LY_CTX_PREFER_SEARCHDIRS)) {
                goto search_clb;
            }
        }

        /* update the latest_revision flag - here we have selected the latest available schema,
         * consider that even the callback provides correct latest revision */
        if (!*mod && ctx_latest) {
            LOGVRB("Newer revision than \"%s@%s\" not found, using this as the latest revision.", ctx_latest->name,
                    ctx_latest->revision);
            ctx_latest->latest_revision = 2;
            *mod = ctx_latest;
        } else if (*mod && !revision && ((*mod)->latest_revision == 1)) {
            (*mod)->latest_revision = 2;
        }

        if (!*mod) {
            LOGVAL(ctx, LYVE_REFERENCE, "%s \"%s\" module failed.", implement ? "Loading" : "Importing", name);
            return LY_EVALID;
        }
    } else {
        /* we have module from the current context, circular check */
        if ((*mod)->parsed->parsing) {
            LOGVAL(ctx, LYVE_REFERENCE, "A circular dependency (import) for module \"%s\".", name);
            *mod = NULL;
            return LY_EVALID;
        }
    }

    /*
     * module found, make sure it is implemented if should be
     */
    if (implement) {
        if (!(*mod)->implemented) {
            /* implement */
            ret = lys_set_implemented_r(*mod, features, unres);
            if (ret) {
                *mod = NULL;
                return ret;
            }
        } else if (features) {
            /* set features if different */
            ret = lys_set_features((*mod)->parsed, features);
            if (!ret) {
                /* context need to be recompiled so that feature changes are properly applied */
                unres->recompile = 1;
            } else if (ret != LY_EEXIST) {
                /* error */
                return ret;
            } /* else no feature changes */
        }
    }

    return LY_SUCCESS;
}

LY_ERR
lysp_check_stringchar(struct lys_parser_ctx *ctx, uint32_t c)
{
    if (!is_yangutf8char(c)) {
        LOGVAL_PARSER(ctx, LY_VCODE_INCHAR, c);
        return LY_EVALID;
    }
    return LY_SUCCESS;
}

LY_ERR
lysp_check_identifierchar(struct lys_parser_ctx *ctx, uint32_t c, ly_bool first, uint8_t *prefix)
{
    if (first || (prefix && ((*prefix) == 1))) {
        if (!is_yangidentstartchar(c)) {
            if ((c < UCHAR_MAX) && isprint(c)) {
                LOGVAL_PARSER(ctx, LYVE_SYNTAX_YANG, "Invalid identifier first character '%c' (0x%04x).", (char)c, c);
            } else {
                LOGVAL_PARSER(ctx, LYVE_SYNTAX_YANG, "Invalid identifier first character 0x%04x.", c);
            }
            return LY_EVALID;
        }
        if (prefix) {
            if (first) {
                (*prefix) = 0;
            } else {
                (*prefix) = 2;
            }
        }
    } else if ((c == ':') && prefix && ((*prefix) == 0)) {
        (*prefix) = 1;
    } else if (!is_yangidentchar(c)) {
        LOGVAL_PARSER(ctx, LYVE_SYNTAX_YANG, "Invalid identifier character '%c' (0x%04x).", (char)c, c);
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

/**
 * @brief Try to find the parsed submodule in main module for the given include record.
 *
 * @param[in] pctx main parser context
 * @param[in] inc The include record with missing parsed submodule. According to include info try to find
 * the corresponding parsed submodule in main module's includes.
 * @return LY_SUCCESS - the parsed submodule was found and inserted into the @p inc record
 * @return LY_ENOT - the parsed module was not found.
 * @return LY_EVALID - YANG rule violation
 */
static LY_ERR
lysp_get_submodule(struct lys_parser_ctx *pctx, struct lysp_include *inc)
{
    LY_ARRAY_COUNT_TYPE i;
    struct lysp_module *main_pmod = pctx->parsed_mod->mod->parsed;

    LY_ARRAY_FOR(main_pmod->includes, i) {
        if (strcmp(main_pmod->includes[i].name, inc->name)) {
            continue;
        }

        if (inc->rev[0] && strncmp(inc->rev, main_pmod->includes[i].rev, LY_REV_SIZE)) {
            LOGVAL(PARSER_CTX(pctx), LYVE_REFERENCE,
                    "Submodule %s includes different revision (%s) of the submodule %s:%s included by the main module %s.",
                    ((struct lysp_submodule *)pctx->parsed_mod)->name, inc->rev,
                    main_pmod->includes[i].name, main_pmod->includes[i].rev, main_pmod->mod->name);
            return LY_EVALID;
        }

        inc->submodule = main_pmod->includes[i].submodule;
        return inc->submodule ? LY_SUCCESS : LY_ENOT;
    }

    if (main_pmod->version == LYS_VERSION_1_1) {
        LOGVAL(PARSER_CTX(pctx), LYVE_REFERENCE,
                "YANG 1.1 requires all submodules to be included from main module. "
                "But submodule \"%s\" includes submodule \"%s\" which is not included by main module \"%s\".",
                ((struct lysp_submodule *)pctx->parsed_mod)->name, inc->name, main_pmod->mod->name);
        return LY_EVALID;
    } else {
        return LY_ENOT;
    }
}

/**
 * @brief Make the copy of the given include record into the main module.
 *
 * YANG 1.0 does not require the main module to include all the submodules. Therefore, parsing submodules can cause
 * reallocating and extending the includes array in the main module by the submodules included only in submodules.
 *
 * @param[in] pctx main parser context
 * @param[in] inc Include record to copy into main module taken from @p pctx.
 * @return LY_ERR value.
 */
static LY_ERR
lysp_inject_submodule(struct lys_parser_ctx *pctx, struct lysp_include *inc)
{
    LY_ARRAY_COUNT_TYPE i;
    struct lysp_include *inc_new, *inc_tofill = NULL;
    struct lysp_module *main_pmod = pctx->parsed_mod->mod->parsed;

    /* first, try to find the corresponding record with missing parsed submodule */
    LY_ARRAY_FOR(main_pmod->includes, i) {
        if (strcmp(main_pmod->includes[i].name, inc->name)) {
            continue;
        }
        inc_tofill = &main_pmod->includes[i];
        break;
    }

    if (inc_tofill) {
        inc_tofill->submodule = inc->submodule;
    } else {
        LY_ARRAY_NEW_RET(PARSER_CTX(pctx), main_pmod->includes, inc_new, LY_EMEM);

        inc_new->submodule = inc->submodule;
        DUP_STRING_RET(PARSER_CTX(pctx), inc->name, inc_new->name);
        DUP_STRING_RET(PARSER_CTX(pctx), inc->dsc, inc_new->dsc);
        DUP_STRING_RET(PARSER_CTX(pctx), inc->ref, inc_new->ref);
        /* TODO duplicate extensions */
        memcpy(inc_new->rev, inc->rev, LY_REV_SIZE);
        inc_new->injected = 1;
    }
    return LY_SUCCESS;
}

LY_ERR
lysp_load_submodules(struct lys_parser_ctx *pctx, struct lysp_module *pmod)
{
    LY_ARRAY_COUNT_TYPE u;
    struct ly_ctx *ctx = PARSER_CTX(pctx);

    LY_ARRAY_FOR(pmod->includes, u) {
        LY_ERR ret = LY_SUCCESS;
        struct lysp_submodule *submod = NULL;
        struct lysp_include *inc = &pmod->includes[u];

        if (inc->submodule) {
            continue;
        }

        if (pmod->is_submod) {
            /* try to find the submodule in the main module or its submodules */
            ret = lysp_get_submodule(pctx, inc);
            LY_CHECK_RET(ret && ret != LY_ENOT, ret);
            LY_CHECK_RET(ret == LY_SUCCESS, LY_SUCCESS); /* submodule found in linked with the inc */
        }

        /* submodule not present in the main module, get the input data and parse it */
        if (!(ctx->flags & LY_CTX_PREFER_SEARCHDIRS)) {
search_clb:
            if (ctx->imp_clb) {
                const char *submodule_data = NULL;
                LYS_INFORMAT format = LYS_IN_UNKNOWN;
                void (*submodule_data_free)(void *module_data, void *user_data) = NULL;
                struct lysp_load_module_check_data check_data = {0};
                struct ly_in *in;

                if (ctx->imp_clb(pctx->parsed_mod->mod->name, NULL, inc->name,
                        inc->rev[0] ? inc->rev : NULL, ctx->imp_clb_data,
                        &format, &submodule_data, &submodule_data_free) == LY_SUCCESS) {
                    LY_CHECK_RET(ly_in_new_memory(submodule_data, &in));
                    check_data.name = inc->name;
                    check_data.revision = inc->rev[0] ? inc->rev : NULL;
                    check_data.submoduleof = pctx->parsed_mod->mod->name;
                    lys_parse_submodule(ctx, in, format, pctx, lysp_load_module_check, &check_data, &submod);

                    /* update inc pointer - parsing another (YANG 1.0) submodule can cause injecting
                     * submodule's include into main module, where it is missing */
                    inc = &pmod->includes[u];

                    ly_in_free(in, 0);
                    if (submodule_data_free) {
                        submodule_data_free((void *)submodule_data, ctx->imp_clb_data);
                    }
                }
            }
            if (!submod && !(ctx->flags & LY_CTX_PREFER_SEARCHDIRS)) {
                goto search_file;
            }
        } else {
search_file:
            if (!(ctx->flags & LY_CTX_DISABLE_SEARCHDIRS)) {
                /* submodule was not received from the callback or there is no callback set */
                lys_module_localfile(ctx, inc->name,
                        inc->rev[0] ? inc->rev : NULL, NULL, 0, pctx,
                        pctx->parsed_mod->mod->name, 1, NULL, (void **)&submod);

                /* update inc pointer - parsing another (YANG 1.0) submodule can cause injecting
                 * submodule's include into main module, where it is missing */
                inc = &pmod->includes[u];
            }
            if (!submod && (ctx->flags & LY_CTX_PREFER_SEARCHDIRS)) {
                goto search_clb;
            }
        }
        if (submod) {
            if (!inc->rev[0] && (submod->latest_revision == 1)) {
                /* update the latest_revision flag - here we have selected the latest available schema,
                 * consider that even the callback provides correct latest revision */
                submod->latest_revision = 2;
            }

            inc->submodule = submod;
            if (ret == LY_ENOT) {
                /* the submodule include is not present in YANG 1.0 main module - add it there */
                LY_CHECK_RET(lysp_inject_submodule(pctx, &pmod->includes[u]));
            }
        }
        if (!inc->submodule) {
            LOGVAL(ctx, LYVE_REFERENCE, "Including \"%s\" submodule into \"%s\" failed.", inc->name,
                    pctx->parsed_mod->is_submod ? ((struct lysp_submodule *)pctx->parsed_mod)->name : pctx->parsed_mod->mod->name);
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

API const struct lysc_when *
lysc_has_when(const struct lysc_node *node)
{
    struct lysc_when **when;

    if (!node) {
        return NULL;
    }

    do {
        when = lysc_node_when(node);
        if (when) {
            return when[0];
        }
        node = node->parent;
    } while (node && (node->nodetype & (LYS_CASE | LYS_CHOICE)));

    return NULL;
}

API const char *
lys_nodetype2str(uint16_t nodetype)
{
    switch (nodetype) {
    case LYS_CONTAINER:
        return "container";
    case LYS_CHOICE:
        return "choice";
    case LYS_LEAF:
        return "leaf";
    case LYS_LEAFLIST:
        return "leaf-list";
    case LYS_LIST:
        return "list";
    case LYS_ANYXML:
        return "anyxml";
    case LYS_ANYDATA:
        return "anydata";
    case LYS_CASE:
        return "case";
    case LYS_RPC:
        return "RPC";
    case LYS_ACTION:
        return "action";
    case LYS_NOTIF:
        return "notification";
    case LYS_USES:
        return "uses";
    default:
        return "unknown";
    }
}

API enum ly_stmt
lys_nodetype2stmt(uint16_t nodetype)
{
    switch (nodetype) {
    case LYS_CONTAINER:
        return LY_STMT_CONTAINER;
    case LYS_CHOICE:
        return LY_STMT_CHOICE;
    case LYS_LEAF:
        return LY_STMT_LEAF;
    case LYS_LEAFLIST:
        return LY_STMT_LEAF_LIST;
    case LYS_LIST:
        return LY_STMT_LIST;
    case LYS_ANYXML:
        return LY_STMT_ANYXML;
    case LYS_ANYDATA:
        return LY_STMT_ANYDATA;
    case LYS_CASE:
        return LY_STMT_CASE;
    case LYS_RPC:
        return LY_STMT_RPC;
    case LYS_ACTION:
        return LY_STMT_ACTION;
    case LYS_NOTIF:
        return LY_STMT_NOTIFICATION;
    case LYS_USES:
        return LY_STMT_USES;
    case LYS_INPUT:
        return LY_STMT_INPUT;
    case LYS_OUTPUT:
        return LY_STMT_OUTPUT;
    default:
        return LY_STMT_NONE;
    }
}

const char *
lys_datatype2str(LY_DATA_TYPE basetype)
{
    switch (basetype) {
    case LY_TYPE_BINARY:
        return "binary";
    case LY_TYPE_UINT8:
        return "uint8";
    case LY_TYPE_UINT16:
        return "uint16";
    case LY_TYPE_UINT32:
        return "uint32";
    case LY_TYPE_UINT64:
        return "uint64";
    case LY_TYPE_STRING:
        return "string";
    case LY_TYPE_BITS:
        return "bits";
    case LY_TYPE_BOOL:
        return "boolean";
    case LY_TYPE_DEC64:
        return "decimal64";
    case LY_TYPE_EMPTY:
        return "empty";
    case LY_TYPE_ENUM:
        return "enumeration";
    case LY_TYPE_IDENT:
        return "identityref";
    case LY_TYPE_INST:
        return "instance-identifier";
    case LY_TYPE_LEAFREF:
        return "leafref";
    case LY_TYPE_UNION:
        return "union";
    case LY_TYPE_INT8:
        return "int8";
    case LY_TYPE_INT16:
        return "int16";
    case LY_TYPE_INT32:
        return "int32";
    case LY_TYPE_INT64:
        return "int64";
    default:
        return "unknown";
    }
}

API const struct lysp_tpdf *
lysp_node_typedefs(const struct lysp_node *node)
{
    switch (node->nodetype) {
    case LYS_CONTAINER:
        return ((struct lysp_node_container *)node)->typedefs;
    case LYS_LIST:
        return ((struct lysp_node_list *)node)->typedefs;
    case LYS_GROUPING:
        return ((struct lysp_node_grp *)node)->typedefs;
    case LYS_RPC:
    case LYS_ACTION:
        return ((struct lysp_node_action *)node)->typedefs;
    case LYS_INPUT:
    case LYS_OUTPUT:
        return ((struct lysp_node_action_inout *)node)->typedefs;
    case LYS_NOTIF:
        return ((struct lysp_node_notif *)node)->typedefs;
    default:
        return NULL;
    }
}

API const struct lysp_node_grp *
lysp_node_groupings(const struct lysp_node *node)
{
    switch (node->nodetype) {
    case LYS_CONTAINER:
        return ((struct lysp_node_container *)node)->groupings;
    case LYS_LIST:
        return ((struct lysp_node_list *)node)->groupings;
    case LYS_GROUPING:
        return ((struct lysp_node_grp *)node)->groupings;
    case LYS_RPC:
    case LYS_ACTION:
        return ((struct lysp_node_action *)node)->groupings;
    case LYS_INPUT:
    case LYS_OUTPUT:
        return ((struct lysp_node_action_inout *)node)->groupings;
    case LYS_NOTIF:
        return ((struct lysp_node_notif *)node)->groupings;
    default:
        return NULL;
    }
}

struct lysp_node_action **
lysp_node_actions_p(struct lysp_node *node)
{
    assert(node);

    switch (node->nodetype) {
    case LYS_CONTAINER:
        return &((struct lysp_node_container *)node)->actions;
    case LYS_LIST:
        return &((struct lysp_node_list *)node)->actions;
    case LYS_GROUPING:
        return &((struct lysp_node_grp *)node)->actions;
    case LYS_AUGMENT:
        return &((struct lysp_node_augment *)node)->actions;
    default:
        return NULL;
    }
}

API const struct lysp_node_action *
lysp_node_actions(const struct lysp_node *node)
{
    struct lysp_node_action **actions;

    actions = lysp_node_actions_p((struct lysp_node *)node);
    if (actions) {
        return *actions;
    } else {
        return NULL;
    }
}

struct lysp_node_notif **
lysp_node_notifs_p(struct lysp_node *node)
{
    assert(node);
    switch (node->nodetype) {
    case LYS_CONTAINER:
        return &((struct lysp_node_container *)node)->notifs;
    case LYS_LIST:
        return &((struct lysp_node_list *)node)->notifs;
    case LYS_GROUPING:
        return &((struct lysp_node_grp *)node)->notifs;
    case LYS_AUGMENT:
        return &((struct lysp_node_augment *)node)->notifs;
    default:
        return NULL;
    }
}

API const struct lysp_node_notif *
lysp_node_notifs(const struct lysp_node *node)
{
    struct lysp_node_notif **notifs;

    notifs = lysp_node_notifs_p((struct lysp_node *)node);
    if (notifs) {
        return *notifs;
    } else {
        return NULL;
    }
}

struct lysp_node **
lysp_node_child_p(struct lysp_node *node)
{
    assert(node);
    switch (node->nodetype) {
    case LYS_CONTAINER:
        return &((struct lysp_node_container *)node)->child;
    case LYS_CHOICE:
        return &((struct lysp_node_choice *)node)->child;
    case LYS_LIST:
        return &((struct lysp_node_list *)node)->child;
    case LYS_CASE:
        return &((struct lysp_node_case *)node)->child;
    case LYS_GROUPING:
        return &((struct lysp_node_grp *)node)->child;
    case LYS_AUGMENT:
        return &((struct lysp_node_augment *)node)->child;
    case LYS_INPUT:
    case LYS_OUTPUT:
        return &((struct lysp_node_action_inout *)node)->child;
    case LYS_NOTIF:
        return &((struct lysp_node_notif *)node)->child;
    default:
        return NULL;
    }
}

API const struct lysp_node *
lysp_node_child(const struct lysp_node *node)
{
    struct lysp_node **child;

    if (!node) {
        return NULL;
    }

    child = lysp_node_child_p((struct lysp_node *)node);
    if (child) {
        return *child;
    } else {
        return NULL;
    }
}

struct lysp_restr **
lysp_node_musts_p(const struct lysp_node *node)
{
    if (!node) {
        return NULL;
    }

    switch (node->nodetype) {
    case LYS_CONTAINER:
        return &((struct lysp_node_container *)node)->musts;
    case LYS_LEAF:
        return &((struct lysp_node_leaf *)node)->musts;
    case LYS_LEAFLIST:
        return &((struct lysp_node_leaflist *)node)->musts;
    case LYS_LIST:
        return &((struct lysp_node_list *)node)->musts;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        return &((struct lysp_node_anydata *)node)->musts;
    case LYS_NOTIF:
        return &((struct lysp_node_notif *)node)->musts;
    case LYS_INPUT:
    case LYS_OUTPUT:
        return &((struct lysp_node_action_inout *)node)->musts;
    default:
        return NULL;
    }
}

struct lysp_restr *
lysp_node_musts(const struct lysp_node *node)
{
    struct lysp_restr **musts;

    musts = lysp_node_musts_p(node);
    if (musts) {
        return *musts;
    } else {
        return NULL;
    }
}

struct lysp_when **
lysp_node_when_p(const struct lysp_node *node)
{
    if (!node) {
        return NULL;
    }

    switch (node->nodetype) {
    case LYS_CONTAINER:
        return &((struct lysp_node_container *)node)->when;
    case LYS_CHOICE:
        return &((struct lysp_node_choice *)node)->when;
    case LYS_LEAF:
        return &((struct lysp_node_leaf *)node)->when;
    case LYS_LEAFLIST:
        return &((struct lysp_node_leaflist *)node)->when;
    case LYS_LIST:
        return &((struct lysp_node_list *)node)->when;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        return &((struct lysp_node_anydata *)node)->when;
    case LYS_CASE:
        return &((struct lysp_node_case *)node)->when;
    case LYS_USES:
        return &((struct lysp_node_uses *)node)->when;
    case LYS_AUGMENT:
        return &((struct lysp_node_augment *)node)->when;
    default:
        return NULL;
    }
}

struct lysp_when *
lysp_node_when(const struct lysp_node *node)
{
    struct lysp_when **when;

    when = lysp_node_when_p(node);
    if (when) {
        return *when;
    } else {
        return NULL;
    }
}

struct lysc_node_action **
lysc_node_actions_p(struct lysc_node *node)
{
    assert(node);
    switch (node->nodetype) {
    case LYS_CONTAINER:
        return &((struct lysc_node_container *)node)->actions;
    case LYS_LIST:
        return &((struct lysc_node_list *)node)->actions;
    default:
        return NULL;
    }
}

API const struct lysc_node_action *
lysc_node_actions(const struct lysc_node *node)
{
    struct lysc_node_action **actions;

    actions = lysc_node_actions_p((struct lysc_node *)node);
    if (actions) {
        return *actions;
    } else {
        return NULL;
    }
}

struct lysc_node_notif **
lysc_node_notifs_p(struct lysc_node *node)
{
    assert(node);
    switch (node->nodetype) {
    case LYS_CONTAINER:
        return &((struct lysc_node_container *)node)->notifs;
    case LYS_LIST:
        return &((struct lysc_node_list *)node)->notifs;
    default:
        return NULL;
    }
}

API const struct lysc_node_notif *
lysc_node_notifs(const struct lysc_node *node)
{
    struct lysc_node_notif **notifs;

    notifs = lysc_node_notifs_p((struct lysc_node *)node);
    if (notifs) {
        return *notifs;
    } else {
        return NULL;
    }
}

struct lysc_node **
lysc_node_child_p(const struct lysc_node *node)
{
    assert(node && !(node->nodetype & (LYS_RPC | LYS_ACTION)));

    switch (node->nodetype) {
    case LYS_CONTAINER:
        return &((struct lysc_node_container *)node)->child;
    case LYS_CHOICE:
        return (struct lysc_node **)&((struct lysc_node_choice *)node)->cases;
    case LYS_CASE:
        return &((struct lysc_node_case *)node)->child;
    case LYS_LIST:
        return &((struct lysc_node_list *)node)->child;
    case LYS_INPUT:
    case LYS_OUTPUT:
        return &((struct lysc_node_action_inout *)node)->child;
    case LYS_NOTIF:
        return &((struct lysc_node_notif *)node)->child;
    default:
        return NULL;
    }
}

API const struct lysc_node *
lysc_node_child(const struct lysc_node *node)
{
    struct lysc_node **child;

    if (!node) {
        return NULL;
    }

    if (node->nodetype & (LYS_RPC | LYS_ACTION)) {
        return &((struct lysc_node_action *)node)->input.node;
    } else {
        child = lysc_node_child_p(node);
        if (child) {
            return *child;
        }
    }

    return NULL;
}

struct lysc_must **
lysc_node_musts_p(const struct lysc_node *node)
{
    if (!node) {
        return NULL;
    }

    switch (node->nodetype) {
    case LYS_CONTAINER:
        return &((struct lysc_node_container *)node)->musts;
    case LYS_LEAF:
        return &((struct lysc_node_leaf *)node)->musts;
    case LYS_LEAFLIST:
        return &((struct lysc_node_leaflist *)node)->musts;
    case LYS_LIST:
        return &((struct lysc_node_list *)node)->musts;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        return &((struct lysc_node_anydata *)node)->musts;
    case LYS_NOTIF:
        return &((struct lysc_node_notif *)node)->musts;
    case LYS_INPUT:
    case LYS_OUTPUT:
        return &((struct lysc_node_action_inout *)node)->musts;
    default:
        return NULL;
    }
}

API struct lysc_must *
lysc_node_musts(const struct lysc_node *node)
{
    struct lysc_must **must_p;

    must_p = lysc_node_musts_p(node);
    if (must_p) {
        return *must_p;
    } else {
        return NULL;
    }
}

struct lysc_when ***
lysc_node_when_p(const struct lysc_node *node)
{
    if (!node) {
        return NULL;
    }

    switch (node->nodetype) {
    case LYS_CONTAINER:
        return &((struct lysc_node_container *)node)->when;
    case LYS_CHOICE:
        return &((struct lysc_node_choice *)node)->when;
    case LYS_LEAF:
        return &((struct lysc_node_leaf *)node)->when;
    case LYS_LEAFLIST:
        return &((struct lysc_node_leaflist *)node)->when;
    case LYS_LIST:
        return &((struct lysc_node_list *)node)->when;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        return &((struct lysc_node_anydata *)node)->when;
    case LYS_CASE:
        return &((struct lysc_node_case *)node)->when;
    case LYS_NOTIF:
        return &((struct lysc_node_notif *)node)->when;
    case LYS_RPC:
    case LYS_ACTION:
        return &((struct lysc_node_action *)node)->when;
    default:
        return NULL;
    }
}

API struct lysc_when **
lysc_node_when(const struct lysc_node *node)
{
    struct lysc_when ***when_p;

    when_p = lysc_node_when_p(node);
    if (when_p) {
        return *when_p;
    } else {
        return NULL;
    }
}

struct lys_module *
lysp_find_module(struct ly_ctx *ctx, const struct lysp_module *mod)
{
    for (uint32_t u = 0; u < ctx->list.count; ++u) {
        if (((struct lys_module *)ctx->list.objs[u])->parsed == mod) {
            return (struct lys_module *)ctx->list.objs[u];
        }
    }
    return NULL;
}

enum ly_stmt
lysp_match_kw(struct ly_in *in, uint64_t *indent)
{
/**
 * @brief Move the input by COUNT items. Also updates the indent value in yang parser context
 * @param[in] COUNT number of items for which the DATA pointer is supposed to move on.
 *
 * *INDENT-OFF*
 */
#define MOVE_IN(COUNT) \
    ly_in_skip(in, COUNT); \
    if (indent) { \
        (*indent)+=COUNT; \
    }
#define IF_KW(STR, LEN, STMT) \
    if (!strncmp(in->current, STR, LEN)) { \
        MOVE_IN(LEN); \
        (*kw)=STMT; \
    }
#define IF_KW_PREFIX(STR, LEN) \
    if (!strncmp(in->current, STR, LEN)) { \
        MOVE_IN(LEN);
#define IF_KW_PREFIX_END \
    }

    const char *start = in->current;
    enum ly_stmt result = LY_STMT_NONE;
    enum ly_stmt *kw = &result;
    /* read the keyword itself */
    switch (in->current[0]) {
    case 'a':
        MOVE_IN(1);
        IF_KW("rgument", 7, LY_STMT_ARGUMENT)
        else IF_KW("ugment", 6, LY_STMT_AUGMENT)
        else IF_KW("ction", 5, LY_STMT_ACTION)
        else IF_KW_PREFIX("ny", 2)
            IF_KW("data", 4, LY_STMT_ANYDATA)
            else IF_KW("xml", 3, LY_STMT_ANYXML)
        IF_KW_PREFIX_END
        break;
    case 'b':
        MOVE_IN(1);
        IF_KW("ase", 3, LY_STMT_BASE)
        else IF_KW("elongs-to", 9, LY_STMT_BELONGS_TO)
        else IF_KW("it", 2, LY_STMT_BIT)
        break;
    case 'c':
        MOVE_IN(1);
        IF_KW("ase", 3, LY_STMT_CASE)
        else IF_KW("hoice", 5, LY_STMT_CHOICE)
        else IF_KW_PREFIX("on", 2)
            IF_KW("fig", 3, LY_STMT_CONFIG)
            else IF_KW_PREFIX("ta", 2)
                IF_KW("ct", 2, LY_STMT_CONTACT)
                else IF_KW("iner", 4, LY_STMT_CONTAINER)
            IF_KW_PREFIX_END
        IF_KW_PREFIX_END
        break;
    case 'd':
        MOVE_IN(1);
        IF_KW_PREFIX("e", 1)
            IF_KW("fault", 5, LY_STMT_DEFAULT)
            else IF_KW("scription", 9, LY_STMT_DESCRIPTION)
            else IF_KW_PREFIX("viat", 4)
                IF_KW("e", 1, LY_STMT_DEVIATE)
                else IF_KW("ion", 3, LY_STMT_DEVIATION)
            IF_KW_PREFIX_END
        IF_KW_PREFIX_END
        break;
    case 'e':
        MOVE_IN(1);
        IF_KW("num", 3, LY_STMT_ENUM)
        else IF_KW_PREFIX("rror-", 5)
            IF_KW("app-tag", 7, LY_STMT_ERROR_APP_TAG)
            else IF_KW("message", 7, LY_STMT_ERROR_MESSAGE)
        IF_KW_PREFIX_END
        else IF_KW("xtension", 8, LY_STMT_EXTENSION)
        break;
    case 'f':
        MOVE_IN(1);
        IF_KW("eature", 6, LY_STMT_FEATURE)
        else IF_KW("raction-digits", 14, LY_STMT_FRACTION_DIGITS)
        break;
    case 'g':
        MOVE_IN(1);
        IF_KW("rouping", 7, LY_STMT_GROUPING)
        break;
    case 'i':
        MOVE_IN(1);
        IF_KW("dentity", 7, LY_STMT_IDENTITY)
        else IF_KW("f-feature", 9, LY_STMT_IF_FEATURE)
        else IF_KW("mport", 5, LY_STMT_IMPORT)
        else IF_KW_PREFIX("n", 1)
            IF_KW("clude", 5, LY_STMT_INCLUDE)
            else IF_KW("put", 3, LY_STMT_INPUT)
        IF_KW_PREFIX_END
        break;
    case 'k':
        MOVE_IN(1);
        IF_KW("ey", 2, LY_STMT_KEY)
        break;
    case 'l':
        MOVE_IN(1);
        IF_KW_PREFIX("e", 1)
            IF_KW("af-list", 7, LY_STMT_LEAF_LIST)
            else IF_KW("af", 2, LY_STMT_LEAF)
            else IF_KW("ngth", 4, LY_STMT_LENGTH)
        IF_KW_PREFIX_END
        else IF_KW("ist", 3, LY_STMT_LIST)
        break;
    case 'm':
        MOVE_IN(1);
        IF_KW_PREFIX("a", 1)
            IF_KW("ndatory", 7, LY_STMT_MANDATORY)
            else IF_KW("x-elements", 10, LY_STMT_MAX_ELEMENTS)
        IF_KW_PREFIX_END
        else IF_KW("in-elements", 11, LY_STMT_MIN_ELEMENTS)
        else IF_KW("ust", 3, LY_STMT_MUST)
        else IF_KW_PREFIX("od", 2)
            IF_KW("ule", 3, LY_STMT_MODULE)
            else IF_KW("ifier", 5, LY_STMT_MODIFIER)
        IF_KW_PREFIX_END
        break;
    case 'n':
        MOVE_IN(1);
        IF_KW("amespace", 8, LY_STMT_NAMESPACE)
        else IF_KW("otification", 11, LY_STMT_NOTIFICATION)
        break;
    case 'o':
        MOVE_IN(1);
        IF_KW_PREFIX("r", 1)
            IF_KW("dered-by", 8, LY_STMT_ORDERED_BY)
            else IF_KW("ganization", 10, LY_STMT_ORGANIZATION)
        IF_KW_PREFIX_END
        else IF_KW("utput", 5, LY_STMT_OUTPUT)
        break;
    case 'p':
        MOVE_IN(1);
        IF_KW("ath", 3, LY_STMT_PATH)
        else IF_KW("attern", 6, LY_STMT_PATTERN)
        else IF_KW("osition", 7, LY_STMT_POSITION)
        else IF_KW_PREFIX("re", 2)
            IF_KW("fix", 3, LY_STMT_PREFIX)
            else IF_KW("sence", 5, LY_STMT_PRESENCE)
        IF_KW_PREFIX_END
        break;
    case 'r':
        MOVE_IN(1);
        IF_KW("ange", 4, LY_STMT_RANGE)
        else IF_KW_PREFIX("e", 1)
            IF_KW_PREFIX("f", 1)
                IF_KW("erence", 6, LY_STMT_REFERENCE)
                else IF_KW("ine", 3, LY_STMT_REFINE)
            IF_KW_PREFIX_END
            else IF_KW("quire-instance", 14, LY_STMT_REQUIRE_INSTANCE)
            else IF_KW("vision-date", 11, LY_STMT_REVISION_DATE)
            else IF_KW("vision", 6, LY_STMT_REVISION)
        IF_KW_PREFIX_END
        else IF_KW("pc", 2, LY_STMT_RPC)
        break;
    case 's':
        MOVE_IN(1);
        IF_KW("tatus", 5, LY_STMT_STATUS)
        else IF_KW("ubmodule", 8, LY_STMT_SUBMODULE)
        break;
    case 't':
        MOVE_IN(1);
        IF_KW("ypedef", 6, LY_STMT_TYPEDEF)
        else IF_KW("ype", 3, LY_STMT_TYPE)
        break;
    case 'u':
        MOVE_IN(1);
        IF_KW_PREFIX("ni", 2)
            IF_KW("que", 3, LY_STMT_UNIQUE)
            else IF_KW("ts", 2, LY_STMT_UNITS)
        IF_KW_PREFIX_END
        else IF_KW("ses", 3, LY_STMT_USES)
        break;
    case 'v':
        MOVE_IN(1);
        IF_KW("alue", 4, LY_STMT_VALUE)
        break;
    case 'w':
        MOVE_IN(1);
        IF_KW("hen", 3, LY_STMT_WHEN)
        break;
    case 'y':
        MOVE_IN(1);
        IF_KW("ang-version", 11, LY_STMT_YANG_VERSION)
        else IF_KW("in-element", 10, LY_STMT_YIN_ELEMENT)
        break;
    default:
        /* if indent is not NULL we are matching keyword from YANG data */
        if (indent) {
            if (in->current[0] == ';') {
                MOVE_IN(1);
                *kw = LY_STMT_SYNTAX_SEMICOLON;
            } else if (in->current[0] == '{') {
                MOVE_IN(1);
                *kw = LY_STMT_SYNTAX_LEFT_BRACE;
            } else if (in->current[0] == '}') {
                MOVE_IN(1);
                *kw = LY_STMT_SYNTAX_RIGHT_BRACE;
            }
        }
        break;
    }

    if ((*kw < LY_STMT_SYNTAX_SEMICOLON) && isalnum(in->current[0])) {
        /* the keyword is not terminated */
        *kw = LY_STMT_NONE;
        in->current = start;
    }

#undef IF_KW
#undef IF_KW_PREFIX
#undef IF_KW_PREFIX_END
#undef MOVE_IN
    /* *INDENT-ON* */

    return result;
}

LY_ERR
lysp_ext_find_definition(const struct ly_ctx *ctx, const struct lysp_ext_instance *ext, const struct lys_module **ext_mod,
        struct lysp_ext **ext_def)
{
    const char *tmp, *name, *prefix;
    size_t pref_len, name_len;
    LY_ARRAY_COUNT_TYPE v;
    const struct lys_module *mod = NULL;

    assert(ext_def);

    *ext_def = NULL;
    if (ext_mod) {
        *ext_mod = NULL;
    }

    /* parse the prefix, the nodeid was previously already parsed and checked */
    tmp = ext->name;
    ly_parse_nodeid(&tmp, &prefix, &pref_len, &name, &name_len);

    /* get module where the extension definition should be placed */
    mod = ly_resolve_prefix(ctx, prefix, pref_len, ext->format, ext->prefix_data);
    if (!mod) {
        LOGVAL(ctx, LYVE_REFERENCE, "Invalid prefix \"%.*s\" used for extension instance identifier.", (int)pref_len, prefix);
        return LY_EVALID;
    } else if (!mod->parsed->extensions) {
        LOGVAL(ctx, LYVE_REFERENCE, "Extension instance \"%s\" refers \"%s\" module that does not contain extension definitions.",
                ext->name, mod->name);
        return LY_EVALID;
    }

    /* find the parsed extension definition there */
    LY_ARRAY_FOR(mod->parsed->extensions, v) {
        if (!strcmp(name, mod->parsed->extensions[v].name)) {
            *ext_def = &mod->parsed->extensions[v];
            break;
        }
    }

    if (!(*ext_def)) {
        LOGVAL(ctx, LYVE_REFERENCE, "Extension definition of extension instance \"%s\" not found.", ext->name);
        return LY_EVALID;
    }

    if (ext_mod) {
        *ext_mod = mod;
    }
    return LY_SUCCESS;
}

LY_ERR
lysp_ext_instance_resolve_argument(struct ly_ctx *ctx, struct lysp_ext_instance *ext_p, struct lysp_ext *ext_def)
{
    if (!ext_def->argname || ext_p->argument) {
        /* nothing to do */
        return LY_SUCCESS;
    }

    if (ext_p->format == LY_VALUE_XML) {
        /* Schema was parsed from YIN and an argument is expected, ... */
        struct lysp_stmt *stmt = NULL;

        if (ext_def->flags & LYS_YINELEM_TRUE) {
            /* ... argument was the first XML child element */
            for (stmt = ext_p->child; stmt && (stmt->flags & LYS_YIN_ATTR); stmt = stmt->next) {}
            if (stmt) {
                const char *arg, *ext, *name_arg, *name_ext, *prefix_arg, *prefix_ext;
                size_t name_arg_len, name_ext_len, prefix_arg_len, prefix_ext_len;

                stmt = ext_p->child;

                arg = stmt->stmt;
                ly_parse_nodeid(&arg, &prefix_arg, &prefix_arg_len, &name_arg, &name_arg_len);
                if (ly_strncmp(ext_def->argname, name_arg, name_arg_len)) {
                    LOGVAL(ctx, LYVE_SEMANTICS, "Extension instance \"%s\" expects argument element \"%s\" as its first XML child, "
                            "but \"%.*s\" element found.", ext_p->name, ext_def->argname, (int)name_arg_len, name_arg);
                    return LY_EVALID;
                }

                /* check namespace - all the extension instances must be qualified and argument element is expected in the same
                 * namespace. Do not check just prefixes, there can be different prefixes pointing to the same namespace */
                ext = ext_p->name; /* include prefix */
                ly_parse_nodeid(&ext, &prefix_ext, &prefix_ext_len, &name_ext, &name_ext_len);

                if (ly_resolve_prefix(ctx, prefix_ext, prefix_ext_len, ext_p->format, ext_p->prefix_data) !=
                        ly_resolve_prefix(ctx, prefix_arg, prefix_arg_len, stmt->format, stmt->prefix_data)) {
                    LOGVAL(ctx, LYVE_SEMANTICS, "Extension instance \"%s\" element and its argument element \"%s\" are "
                            "expected in the same namespace, but they differ.", ext_p->name, ext_def->argname);
                    return LY_EVALID;
                }
            }
        } else {
            /* ... argument was one of the XML attributes which are represented as child stmt
             * with LYS_YIN_ATTR flag */
            for (stmt = ext_p->child; stmt && (stmt->flags & LYS_YIN_ATTR); stmt = stmt->next) {
                if (!strcmp(stmt->stmt, ext_def->argname)) {
                    /* this is the extension's argument */
                    break;
                }
            }
        }

        if (stmt) {
            LY_CHECK_RET(lydict_insert(ctx, stmt->arg, 0, &ext_p->argument));
            stmt->flags |= LYS_YIN_ARGUMENT;
        }
    }

    if (!ext_p->argument) {
        /* missing extension's argument */
        LOGVAL(ctx, LYVE_SEMANTICS, "Extension instance \"%s\" misses argument %s\"%s\".",
                ext_p->name, (ext_def->flags & LYS_YINELEM_TRUE) ? "element " : "", ext_def->argname);
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

LY_ARRAY_COUNT_TYPE
lysp_ext_instance_iter(struct lysp_ext_instance *ext, LY_ARRAY_COUNT_TYPE index, enum ly_stmt substmt)
{
    LY_CHECK_ARG_RET(NULL, ext, LY_EINVAL);

    for ( ; index < LY_ARRAY_COUNT(ext); index++) {
        if (ext[index].parent_stmt == substmt) {
            return index;
        }
    }

    return LY_ARRAY_COUNT(ext);
}

const struct lysc_node *
lysc_data_node(const struct lysc_node *schema)
{
    const struct lysc_node *parent;

    parent = schema;
    while (parent && !(parent->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA | LYS_RPC |
            LYS_ACTION | LYS_NOTIF))) {
        parent = parent->parent;
    }

    return parent;
}
