/**
 * @file yl_schema_features.c
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief Control features for the schema.
 *
 * Copyright (c) 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include <errno.h>
#include <stdlib.h> /* calloc */
#include <string.h> /* strcmp */

#include "compat.h" /* strndup */
#include "set.h" /* ly_set */

#include "common.h"
#include "yl_schema_features.h"

void
yl_schema_features_free(void *flist)
{
    struct yl_schema_features *rec = (struct yl_schema_features *)flist;

    if (rec) {
        free(rec->mod_name);
        if (rec->features) {
            for (uint32_t u = 0; rec->features[u]; ++u) {
                free(rec->features[u]);
            }
            free(rec->features);
        }
        free(rec);
    }
}

void
get_features(const struct ly_set *fset, const char *module, const char ***features)
{
    /* get features list for this module */
    for (uint32_t u = 0; u < fset->count; ++u) {
        struct yl_schema_features *sf = (struct yl_schema_features *)fset->objs[u];

        if (!strcmp(module, sf->mod_name)) {
            /* matched module - explicitly set features */
            *features = (const char **)sf->features;
            sf->applied = 1;
            return;
        }
    }

    /* features not set so disable all */
    *features = NULL;
}

int
parse_features(const char *fstring, struct ly_set *fset)
{
    struct yl_schema_features *rec = NULL;
    uint32_t count;
    char *p, **fp;

    rec = calloc(1, sizeof *rec);
    if (!rec) {
        YLMSG_E("Unable to allocate features information record (%s).", strerror(errno));
        goto error;
    }

    /* fill the record */
    p = strchr(fstring, ':');
    if (!p) {
        YLMSG_E("Invalid format of the features specification (%s).", fstring);
        goto error;
    }
    rec->mod_name = strndup(fstring, p - fstring);

    count = 0;
    while (p) {
        size_t len = 0;
        char *token = p + 1;

        p = strchr(token, ',');
        if (!p) {
            /* the last item, if any */
            len = strlen(token);
        } else {
            len = p - token;
        }

        if (len) {
            fp = realloc(rec->features, (count + 1) * sizeof *rec->features);
            if (!fp) {
                YLMSG_E("Unable to store features list information (%s).", strerror(errno));
                goto error;
            }
            rec->features = fp;
            fp = &rec->features[count++]; /* array item to set */
            (*fp) = strndup(token, len);
        }
    }

    /* terminating NULL */
    fp = realloc(rec->features, (count + 1) * sizeof *rec->features);
    if (!fp) {
        YLMSG_E("Unable to store features list information (%s).", strerror(errno));
        goto error;
    }
    rec->features = fp;
    rec->features[count++] = NULL;

    /* Store record to the output set. */
    if (ly_set_add(fset, rec, 1, NULL)) {
        YLMSG_E("Unable to store features information (%s).", strerror(errno));
        goto error;
    }
    rec = NULL;

    return 0;

error:
    yl_schema_features_free(rec);
    return -1;
}

void
print_features(struct ly_out *out, const struct lys_module *mod)
{
    struct lysp_feature *f;
    uint32_t idx;
    size_t max_len, len;

    ly_print(out, "%s:\n", mod->name);

    /* get max len, so the statuses of all the features will be aligned */
    max_len = 0, idx = 0, f = NULL;
    while ((f = lysp_feature_next(f, mod->parsed, &idx))) {
        len = strlen(f->name);
        max_len = (max_len > len) ? max_len : len;
    }
    if (!max_len) {
        ly_print(out, "\t(none)\n");
        return;
    }

    /* print features */
    idx = 0, f = NULL;
    while ((f = lysp_feature_next(f, mod->parsed, &idx))) {
        ly_print(out, "\t%-*s (%s)\n", (int)max_len, f->name, lys_feature_value(mod, f->name) ? "off" : "on");
    }
}

void
print_feature_param(struct ly_out *out, const struct lys_module *mod)
{
    struct lysp_feature *f = NULL;
    uint32_t idx = 0;
    uint8_t first = 1;

    ly_print(out, " -F %s:", mod->name);
    while ((f = lysp_feature_next(f, mod->parsed, &idx))) {
        if (first) {
            ly_print(out, "%s", f->name);
            first = 0;
        } else {
            ly_print(out, ",%s", f->name);
        }
    }
}

void
print_all_features(struct ly_out *out, const struct ly_ctx *ctx, uint8_t feature_param)
{
    uint32_t i;
    struct lys_module *mod;
    uint8_t first;

    /* Print features for all implemented modules. */
    first = 1;
    i = 0;
    while ((mod = ly_ctx_get_module_iter(ctx, &i)) != NULL) {
        if (!mod->implemented) {
            continue;
        }
        if (first) {
            print_features(out, mod);
            first = 0;
        } else {
            ly_print(out, "\n");
            print_features(out, mod);
        }
    }

    if (!feature_param) {
        return;
    }
    ly_print(out, "\n");

    /* Print features for all implemented modules in 'feature-param' format. */
    i = 0;
    while ((mod = ly_ctx_get_module_iter(ctx, &i)) != NULL) {
        if (mod->implemented) {
            print_feature_param(out, mod);
        }
    }
}
