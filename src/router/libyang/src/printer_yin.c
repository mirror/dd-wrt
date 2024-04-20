/**
 * @file printer_yin.c
 * @author Fred Gan <ganshaolong@vip.qq.com>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief YIN printer
 *
 * Copyright (c) 2015 - 2022 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "compat.h"
#include "log.h"
#include "ly_common.h"
#include "out.h"
#include "out_internal.h"
#include "printer_internal.h"
#include "tree.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "xml.h"
#include "xpath.h"

/**
 * @brief YIN printer context.
 */
struct lys_ypr_ctx {
    union {
        struct {
            struct ly_out *out;              /**< output specification */
            uint16_t level;                  /**< current indentation level: 0 - no formatting, >= 1 indentation levels */
            uint32_t options;                /**< Schema output options (see @ref schemaprinterflags). */
            const struct lys_module *module; /**< schema to print */
        };
        struct lyspr_ctx printer_ctx;
    };

    /* YIN printer specific members */
};

static void
ypr_open(struct lys_ypr_ctx *pctx, const char *elem_name, const char *attr_name, const char *attr_value, int8_t flag)
{
    ly_print_(pctx->out, "%*s<%s", INDENT, elem_name);

    if (attr_name) {
        ly_print_(pctx->out, " %s=\"", attr_name);
        lyxml_dump_text(pctx->out, attr_value, 1);
        ly_print_(pctx->out, "\"%s", flag == -1 ? "/>\n" : flag == 1 ? ">\n" : "");
    } else if (flag) {
        ly_print_(pctx->out, flag == -1 ? "/>\n" : ">\n");
    }
}

static void
ypr_close(struct lys_ypr_ctx *pctx, const char *elem_name, int8_t flag)
{
    if (flag) {
        ly_print_(pctx->out, "%*s</%s>\n", INDENT, elem_name);
    } else {
        ly_print_(pctx->out, "/>\n");
    }
}

/*
 * par_close_flag
 * 0 - parent not yet closed, printing >\n, setting flag to 1
 * 1 or NULL - parent already closed, do nothing
 */
static void
ypr_close_parent(struct lys_ypr_ctx *pctx, int8_t *par_close_flag)
{
    if (par_close_flag && !(*par_close_flag)) {
        (*par_close_flag) = 1;
        ly_print_(pctx->out, ">\n");
    }
}

static void
ypr_yin_arg(struct lys_ypr_ctx *pctx, const char *arg, const char *text)
{
    ly_print_(pctx->out, "%*s<%s>", INDENT, arg);
    lyxml_dump_text(pctx->out, text, 0);
    ly_print_(pctx->out, "</%s>\n", arg);
}

static void
yprp_stmt(struct lys_ypr_ctx *pctx, struct lysp_stmt *stmt)
{
    struct lysp_stmt *childstmt;
    int8_t flag = stmt->child ? 1 : -1;

    if (lys_stmt_str(stmt->kw)) {
        if (lys_stmt_flags(stmt->kw) & LY_STMT_FLAG_YIN) {
            ypr_open(pctx, stmt->stmt, NULL, NULL, flag);
            ypr_yin_arg(pctx, lys_stmt_arg(stmt->kw), stmt->arg);
        } else {
            ypr_open(pctx, stmt->stmt, lys_stmt_arg(stmt->kw), stmt->arg, flag);
        }
    } else if (stmt->kw == LY_STMT_EXTENSION_INSTANCE) {
        ypr_open(pctx, stmt->stmt, (stmt->arg && stmt->arg[0]) ? lys_stmt_arg(LY_STMT_VALUE) : NULL, stmt->arg, flag);
    }

    if (stmt->child) {
        LEVEL++;
        LY_LIST_FOR(stmt->child, childstmt) {
            yprp_stmt(pctx, childstmt);
        }
        LEVEL--;
        ypr_close(pctx, stmt->stmt, flag);
    }
}

static void
yprp_extension_instance(struct lys_ypr_ctx *pctx, enum ly_stmt substmt, uint8_t substmt_index,
        struct lysp_ext_instance *ext, int8_t *flag)
{
    struct lysp_stmt *stmt;
    int8_t inner_flag;

    if ((ext->flags & LYS_INTERNAL) || (ext->parent_stmt != substmt) || (ext->parent_stmt_index != substmt_index)) {
        return;
    }

    ypr_close_parent(pctx, flag);
    inner_flag = 0;

    ypr_open(pctx, ext->name, (ext->def->flags & LYS_YINELEM_TRUE) ? NULL : ext->def->argname, ext->argument, inner_flag);
    LEVEL++;
    if (ext->def->flags & LYS_YINELEM_TRUE) {
        const char *prefix, *name, *id;
        size_t prefix_len, name_len;

        ypr_close_parent(pctx, &inner_flag);

        /* we need to use the same namespace as for the extension instance element */
        id = ext->name;
        ly_parse_nodeid(&id, &prefix, &prefix_len, &name, &name_len);
        ly_print_(pctx->out, "%*s<%.*s:%s>", INDENT, (int)prefix_len, prefix, ext->def->argname);
        lyxml_dump_text(pctx->out, ext->argument, 0);
        ly_print_(pctx->out, "</%.*s:%s>\n", (int)prefix_len, prefix, ext->def->argname);
    }
    LY_LIST_FOR(ext->child, stmt) {
        if (stmt->flags & (LYS_YIN_ATTR | LYS_YIN_ARGUMENT)) {
            continue;
        }

        ypr_close_parent(pctx, &inner_flag);
        yprp_stmt(pctx, stmt);
    }
    LEVEL--;
    ypr_close(pctx, ext->name, inner_flag);
}

static void
yprp_extension_instances(struct lys_ypr_ctx *pctx, enum ly_stmt substmt, uint8_t substmt_index,
        struct lysp_ext_instance *exts, int8_t *flag)
{
    LY_ARRAY_COUNT_TYPE u;

    LY_ARRAY_FOR(exts, u) {
        yprp_extension_instance(pctx, substmt, substmt_index, &exts[u], flag);
    }
}

static void
ypr_substmt(struct lys_ypr_ctx *pctx, enum ly_stmt substmt, uint8_t substmt_index, const char *text, void *exts)
{
    int8_t extflag = 0;

    if (!text) {
        /* nothing to print */
        return;
    }

    if (lys_stmt_flags(substmt) & LY_STMT_FLAG_YIN) {
        extflag = 1;
        ypr_open(pctx, lys_stmt_str(substmt), NULL, NULL, extflag);
    } else {
        ypr_open(pctx, lys_stmt_str(substmt), lys_stmt_arg(substmt), text, extflag);
    }

    LEVEL++;
    yprp_extension_instances(pctx, substmt, substmt_index, exts, &extflag);

    /* argument as yin-element */
    if (lys_stmt_flags(substmt) & LY_STMT_FLAG_YIN) {
        ypr_yin_arg(pctx, lys_stmt_arg(substmt), text);
    }

    LEVEL--;
    ypr_close(pctx, lys_stmt_str(substmt), extflag);
}

static void
ypr_unsigned(struct lys_ypr_ctx *pctx, enum ly_stmt substmt, uint8_t substmt_index, void *exts, unsigned long attr_value)
{
    char *str;

    if (asprintf(&str, "%lu", attr_value) == -1) {
        LOGMEM(pctx->module->ctx);
        return;
    }
    ypr_substmt(pctx, substmt, substmt_index, str, exts);
    free(str);
}

static void
ypr_signed(struct lys_ypr_ctx *pctx, enum ly_stmt substmt, uint8_t substmt_index, void *exts, long attr_value)
{
    char *str;

    if (asprintf(&str, "%ld", attr_value) == -1) {
        LOGMEM(pctx->module->ctx);
        return;
    }
    ypr_substmt(pctx, substmt, substmt_index, str, exts);
    free(str);
}

static void
yprp_revision(struct lys_ypr_ctx *pctx, const struct lysp_revision *rev)
{
    if (rev->dsc || rev->ref || rev->exts) {
        ypr_open(pctx, "revision", "date", rev->date, 1);
        LEVEL++;
        yprp_extension_instances(pctx, LY_STMT_REVISION, 0, rev->exts, NULL);
        ypr_substmt(pctx, LY_STMT_DESCRIPTION, 0, rev->dsc, rev->exts);
        ypr_substmt(pctx, LY_STMT_REFERENCE, 0, rev->ref, rev->exts);
        LEVEL--;
        ypr_close(pctx, "revision", 1);
    } else {
        ypr_open(pctx, "revision", "date", rev->date, -1);
    }
}

static void
ypr_mandatory(struct lys_ypr_ctx *pctx, uint16_t flags, void *exts, int8_t *flag)
{
    if (flags & LYS_MAND_MASK) {
        ypr_close_parent(pctx, flag);
        ypr_substmt(pctx, LY_STMT_MANDATORY, 0, (flags & LYS_MAND_TRUE) ? "true" : "false", exts);
    }
}

static void
ypr_config(struct lys_ypr_ctx *pctx, uint16_t flags, void *exts, int8_t *flag)
{
    if (flags & LYS_CONFIG_MASK) {
        ypr_close_parent(pctx, flag);
        ypr_substmt(pctx, LY_STMT_CONFIG, 0, (flags & LYS_CONFIG_W) ? "true" : "false", exts);
    }
}

static void
ypr_status(struct lys_ypr_ctx *pctx, uint16_t flags, void *exts, int8_t *flag)
{
    const char *status = NULL;

    if (flags & LYS_STATUS_CURR) {
        ypr_close_parent(pctx, flag);
        status = "current";
    } else if (flags & LYS_STATUS_DEPRC) {
        ypr_close_parent(pctx, flag);
        status = "deprecated";
    } else if (flags & LYS_STATUS_OBSLT) {
        ypr_close_parent(pctx, flag);
        status = "obsolete";
    }

    ypr_substmt(pctx, LY_STMT_STATUS, 0, status, exts);
}

static void
ypr_description(struct lys_ypr_ctx *pctx, const char *dsc, void *exts, int8_t *flag)
{
    if (dsc) {
        ypr_close_parent(pctx, flag);
        ypr_substmt(pctx, LY_STMT_DESCRIPTION, 0, dsc, exts);
    }
}

static void
ypr_reference(struct lys_ypr_ctx *pctx, const char *ref, void *exts, int8_t *flag)
{
    if (ref) {
        ypr_close_parent(pctx, flag);
        ypr_substmt(pctx, LY_STMT_REFERENCE, 0, ref, exts);
    }
}

static void
yprp_iffeatures(struct lys_ypr_ctx *pctx, struct lysp_qname *iffs, struct lysp_ext_instance *exts, int8_t *flag)
{
    LY_ARRAY_COUNT_TYPE u;
    int8_t extflag;

    LY_ARRAY_FOR(iffs, u) {
        ypr_close_parent(pctx, flag);
        extflag = 0;

        ly_print_(pctx->out, "%*s<if-feature name=\"%s",  INDENT, iffs[u].str);

        /* extensions */
        LEVEL++;
        yprp_extension_instances(pctx, LY_STMT_IF_FEATURE, u, exts, &extflag);
        LEVEL--;
        ly_print_(pctx->out, "\"/>\n");
    }
}

static void
yprp_extension(struct lys_ypr_ctx *pctx, const struct lysp_ext *ext)
{
    int8_t flag = 0, flag2 = 0;
    LY_ARRAY_COUNT_TYPE u;

    ypr_open(pctx, "extension", "name", ext->name, flag);
    LEVEL++;

    if (ext->exts) {
        ypr_close_parent(pctx, &flag);
        yprp_extension_instances(pctx, LY_STMT_EXTENSION, 0, ext->exts, &flag);
    }

    if (ext->argname) {
        ypr_close_parent(pctx, &flag);
        ypr_open(pctx, "argument", "name", ext->argname, flag2);

        LEVEL++;
        if (ext->exts) {
            u = -1;
            while ((u = lysp_ext_instance_iter(ext->exts, u + 1, LY_STMT_ARGUMENT)) != LY_ARRAY_COUNT(ext->exts)) {
                ypr_close_parent(pctx, &flag2);
                yprp_extension_instance(pctx, LY_STMT_ARGUMENT, 0, &ext->exts[u], &flag2);
            }
        }
        if ((ext->flags & LYS_YINELEM_MASK) ||
                (ext->exts && (lysp_ext_instance_iter(ext->exts, 0, LY_STMT_YIN_ELEMENT) != LY_ARRAY_COUNT(ext->exts)))) {
            ypr_close_parent(pctx, &flag2);
            ypr_substmt(pctx, LY_STMT_YIN_ELEMENT, 0, (ext->flags & LYS_YINELEM_TRUE) ? "true" : "false", ext->exts);
        }
        LEVEL--;
        ypr_close(pctx, "argument", flag2);
    }

    ypr_status(pctx, ext->flags, ext->exts, &flag);
    ypr_description(pctx, ext->dsc, ext->exts, &flag);
    ypr_reference(pctx, ext->ref, ext->exts, &flag);

    LEVEL--;
    ypr_close(pctx, "extension", flag);
}

static void
yprp_feature(struct lys_ypr_ctx *pctx, const struct lysp_feature *feat)
{
    int8_t flag = 0;

    ypr_open(pctx, "feature", "name", feat->name, flag);
    LEVEL++;
    yprp_extension_instances(pctx, LY_STMT_FEATURE, 0, feat->exts, &flag);
    yprp_iffeatures(pctx, feat->iffeatures, feat->exts, &flag);
    ypr_status(pctx, feat->flags, feat->exts, &flag);
    ypr_description(pctx, feat->dsc, feat->exts, &flag);
    ypr_reference(pctx, feat->ref, feat->exts, &flag);
    LEVEL--;
    ypr_close(pctx, "feature", flag);
}

static void
yprp_identity(struct lys_ypr_ctx *pctx, const struct lysp_ident *ident)
{
    int8_t flag = 0;
    LY_ARRAY_COUNT_TYPE u;

    ypr_open(pctx, "identity", "name", ident->name, flag);
    LEVEL++;

    yprp_extension_instances(pctx, LY_STMT_IDENTITY, 0, ident->exts, &flag);
    yprp_iffeatures(pctx, ident->iffeatures, ident->exts, &flag);

    LY_ARRAY_FOR(ident->bases, u) {
        ypr_close_parent(pctx, &flag);
        ypr_substmt(pctx, LY_STMT_BASE, u, ident->bases[u], ident->exts);
    }

    ypr_status(pctx, ident->flags, ident->exts, &flag);
    ypr_description(pctx, ident->dsc, ident->exts, &flag);
    ypr_reference(pctx, ident->ref, ident->exts, &flag);

    LEVEL--;
    ypr_close(pctx, "identity", flag);
}

static void
yprp_restr(struct lys_ypr_ctx *pctx, const struct lysp_restr *restr, enum ly_stmt stmt, const char *attr, int8_t *flag)
{
    (void)flag;
    int8_t inner_flag = 0;

    if (!restr) {
        return;
    }

    ly_print_(pctx->out, "%*s<%s %s=\"", INDENT, lyplg_ext_stmt2str(stmt), attr);
    lyxml_dump_text(pctx->out,
            (restr->arg.str[0] != LYSP_RESTR_PATTERN_NACK && restr->arg.str[0] != LYSP_RESTR_PATTERN_ACK) ?
            restr->arg.str : &restr->arg.str[1], 1);
    ly_print_(pctx->out, "\"");

    LEVEL++;
    yprp_extension_instances(pctx, stmt, 0, restr->exts, &inner_flag);
    if (restr->arg.str[0] == LYSP_RESTR_PATTERN_NACK) {
        ypr_close_parent(pctx, &inner_flag);
        /* special byte value in pattern's expression: 0x15 - invert-match, 0x06 - match */
        ypr_substmt(pctx, LY_STMT_MODIFIER, 0, "invert-match", restr->exts);
    }
    if (restr->emsg) {
        ypr_close_parent(pctx, &inner_flag);
        ypr_substmt(pctx, LY_STMT_ERROR_MESSAGE, 0, restr->emsg, restr->exts);
    }
    if (restr->eapptag) {
        ypr_close_parent(pctx, &inner_flag);
        ypr_substmt(pctx, LY_STMT_ERROR_APP_TAG, 0, restr->eapptag, restr->exts);
    }
    ypr_description(pctx, restr->dsc, restr->exts, &inner_flag);
    ypr_reference(pctx, restr->ref, restr->exts, &inner_flag);

    LEVEL--;
    ypr_close(pctx, lyplg_ext_stmt2str(stmt), inner_flag);
}

static void
yprp_when(struct lys_ypr_ctx *pctx, struct lysp_when *when, int8_t *flag)
{
    int8_t inner_flag = 0;

    if (!when) {
        return;
    }

    ypr_close_parent(pctx, flag);
    ly_print_(pctx->out, "%*s<when condition=\"", INDENT);
    lyxml_dump_text(pctx->out, when->cond, 1);
    ly_print_(pctx->out, "\"");

    LEVEL++;
    yprp_extension_instances(pctx, LY_STMT_WHEN, 0, when->exts, &inner_flag);
    ypr_description(pctx, when->dsc, when->exts, &inner_flag);
    ypr_reference(pctx, when->ref, when->exts, &inner_flag);
    LEVEL--;
    ypr_close(pctx, "when", inner_flag);
}

static void
yprp_enum(struct lys_ypr_ctx *pctx, const struct lysp_type_enum *items, LY_DATA_TYPE type, int8_t *flag)
{
    LY_ARRAY_COUNT_TYPE u;
    int8_t inner_flag;

    (void)flag;

    LY_ARRAY_FOR(items, u) {
        if (type == LY_TYPE_BITS) {
            ly_print_(pctx->out, "%*s<bit name=\"", INDENT);
            lyxml_dump_text(pctx->out, items[u].name, 1);
            ly_print_(pctx->out, "\"");
        } else { /* LY_TYPE_ENUM */
            ly_print_(pctx->out, "%*s<enum name=\"", INDENT);
            lyxml_dump_text(pctx->out, items[u].name, 1);
            ly_print_(pctx->out, "\"");
        }
        inner_flag = 0;
        LEVEL++;
        yprp_extension_instances(pctx, LY_STMT_ENUM, 0, items[u].exts, &inner_flag);
        yprp_iffeatures(pctx, items[u].iffeatures, items[u].exts, &inner_flag);
        if (items[u].flags & LYS_SET_VALUE) {
            if (type == LY_TYPE_BITS) {
                ypr_close_parent(pctx, &inner_flag);
                ypr_unsigned(pctx, LY_STMT_POSITION, 0, items[u].exts, items[u].value);
            } else { /* LY_TYPE_ENUM */
                ypr_close_parent(pctx, &inner_flag);
                ypr_signed(pctx, LY_STMT_VALUE, 0, items[u].exts, items[u].value);
            }
        }
        ypr_status(pctx, items[u].flags, items[u].exts, &inner_flag);
        ypr_description(pctx, items[u].dsc, items[u].exts, &inner_flag);
        ypr_reference(pctx, items[u].ref, items[u].exts, &inner_flag);
        LEVEL--;
        ypr_close(pctx, type == LY_TYPE_BITS ? "bit" : "enum", inner_flag);
    }
}

static void
yprp_type(struct lys_ypr_ctx *pctx, const struct lysp_type *type)
{
    LY_ARRAY_COUNT_TYPE u;
    int8_t flag = 0;

    if (!pctx || !type) {
        return;
    }

    ypr_open(pctx, "type", "name", type->name, flag);
    LEVEL++;

    yprp_extension_instances(pctx, LY_STMT_TYPE, 0, type->exts, &flag);

    if (type->range || type->length || type->patterns || type->bits || type->enums) {
        ypr_close_parent(pctx, &flag);
    }
    yprp_restr(pctx, type->range, LY_STMT_RANGE, "value", &flag);
    yprp_restr(pctx, type->length, LY_STMT_LENGTH, "value", &flag);
    LY_ARRAY_FOR(type->patterns, u) {
        yprp_restr(pctx, &type->patterns[u], LY_STMT_PATTERN, "value", &flag);
    }
    yprp_enum(pctx, type->bits, LY_TYPE_BITS, &flag);
    yprp_enum(pctx, type->enums, LY_TYPE_ENUM, &flag);

    if (type->path) {
        ypr_close_parent(pctx, &flag);
        ypr_substmt(pctx, LY_STMT_PATH, 0, type->path->expr, type->exts);
    }
    if (type->flags & LYS_SET_REQINST) {
        ypr_close_parent(pctx, &flag);
        ypr_substmt(pctx, LY_STMT_REQUIRE_INSTANCE, 0, type->require_instance ? "true" : "false", type->exts);
    }
    if (type->flags & LYS_SET_FRDIGITS) {
        ypr_close_parent(pctx, &flag);
        ypr_unsigned(pctx, LY_STMT_FRACTION_DIGITS, 0, type->exts, type->fraction_digits);
    }
    LY_ARRAY_FOR(type->bases, u) {
        ypr_close_parent(pctx, &flag);
        ypr_substmt(pctx, LY_STMT_BASE, u, type->bases[u], type->exts);
    }
    LY_ARRAY_FOR(type->types, u) {
        ypr_close_parent(pctx, &flag);
        yprp_type(pctx, &type->types[u]);
    }

    LEVEL--;
    ypr_close(pctx, "type", flag);
}

static void
yprp_typedef(struct lys_ypr_ctx *pctx, const struct lysp_tpdf *tpdf)
{
    ypr_open(pctx, "typedef", "name", tpdf->name, 1);
    LEVEL++;

    yprp_extension_instances(pctx, LY_STMT_TYPEDEF, 0, tpdf->exts, NULL);

    yprp_type(pctx, &tpdf->type);

    if (tpdf->units) {
        ypr_substmt(pctx, LY_STMT_UNITS, 0, tpdf->units, tpdf->exts);
    }
    if (tpdf->dflt.str) {
        ypr_substmt(pctx, LY_STMT_DEFAULT, 0, tpdf->dflt.str, tpdf->exts);
    }

    ypr_status(pctx, tpdf->flags, tpdf->exts, NULL);
    ypr_description(pctx, tpdf->dsc, tpdf->exts, NULL);
    ypr_reference(pctx, tpdf->ref, tpdf->exts, NULL);

    LEVEL--;
    ypr_close(pctx, "typedef", 1);
}

static void yprp_node(struct lys_ypr_ctx *pctx, const struct lysp_node *node);
static void yprp_action(struct lys_ypr_ctx *pctx, const struct lysp_node_action *action);
static void yprp_notification(struct lys_ypr_ctx *pctx, const struct lysp_node_notif *notif);

static void
yprp_grouping(struct lys_ypr_ctx *pctx, const struct lysp_node_grp *grp)
{
    LY_ARRAY_COUNT_TYPE u;
    int8_t flag = 0;
    struct lysp_node *data;
    struct lysp_node_action *action;
    struct lysp_node_notif *notif;
    struct lysp_node_grp *subgrp;

    ypr_open(pctx, "grouping", "name", grp->name, flag);
    LEVEL++;

    yprp_extension_instances(pctx, LY_STMT_GROUPING, 0, grp->exts, &flag);
    ypr_status(pctx, grp->flags, grp->exts, &flag);
    ypr_description(pctx, grp->dsc, grp->exts, &flag);
    ypr_reference(pctx, grp->ref, grp->exts, &flag);

    LY_ARRAY_FOR(grp->typedefs, u) {
        ypr_close_parent(pctx, &flag);
        yprp_typedef(pctx, &grp->typedefs[u]);
    }

    LY_LIST_FOR(grp->groupings, subgrp) {
        ypr_close_parent(pctx, &flag);
        yprp_grouping(pctx, subgrp);
    }

    LY_LIST_FOR(grp->child, data) {
        ypr_close_parent(pctx, &flag);
        yprp_node(pctx, data);
    }

    LY_LIST_FOR(grp->actions, action) {
        ypr_close_parent(pctx, &flag);
        yprp_action(pctx, action);
    }

    LY_LIST_FOR(grp->notifs, notif) {
        ypr_close_parent(pctx, &flag);
        yprp_notification(pctx, notif);
    }

    LEVEL--;
    ypr_close(pctx, "grouping", flag);
}

static void
yprp_inout(struct lys_ypr_ctx *pctx, const struct lysp_node_action_inout *inout, int8_t *flag)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_node *data;
    struct lysp_node_grp *grp;

    if (!inout->child) {
        /* input/output is empty */
        return;
    }
    ypr_close_parent(pctx, flag);

    ypr_open(pctx, inout->name, NULL, NULL, *flag);
    LEVEL++;

    yprp_extension_instances(pctx, lyplg_ext_nodetype2stmt(inout->nodetype), 0, inout->exts, NULL);
    LY_ARRAY_FOR(inout->musts, u) {
        yprp_restr(pctx, &inout->musts[u], LY_STMT_MUST, "condition", NULL);
    }
    LY_ARRAY_FOR(inout->typedefs, u) {
        yprp_typedef(pctx, &inout->typedefs[u]);
    }
    LY_LIST_FOR(inout->groupings, grp) {
        yprp_grouping(pctx, grp);
    }

    LY_LIST_FOR(inout->child, data) {
        yprp_node(pctx, data);
    }

    LEVEL--;
    ypr_close(pctx, inout->name, 1);
}

static void
yprp_notification(struct lys_ypr_ctx *pctx, const struct lysp_node_notif *notif)
{
    LY_ARRAY_COUNT_TYPE u;
    int8_t flag = 0;
    struct lysp_node *data;
    struct lysp_node_grp *grp;

    ypr_open(pctx, "notification", "name", notif->name, flag);

    LEVEL++;
    yprp_extension_instances(pctx, LY_STMT_NOTIFICATION, 0, notif->exts, &flag);
    yprp_iffeatures(pctx, notif->iffeatures, notif->exts, &flag);

    LY_ARRAY_FOR(notif->musts, u) {
        ypr_close_parent(pctx, &flag);
        yprp_restr(pctx, &notif->musts[u], LY_STMT_MUST, "condition", &flag);
    }
    ypr_status(pctx, notif->flags, notif->exts, &flag);
    ypr_description(pctx, notif->dsc, notif->exts, &flag);
    ypr_reference(pctx, notif->ref, notif->exts, &flag);

    LY_ARRAY_FOR(notif->typedefs, u) {
        ypr_close_parent(pctx, &flag);
        yprp_typedef(pctx, &notif->typedefs[u]);
    }

    LY_LIST_FOR(notif->groupings, grp) {
        ypr_close_parent(pctx, &flag);
        yprp_grouping(pctx, grp);
    }

    LY_LIST_FOR(notif->child, data) {
        ypr_close_parent(pctx, &flag);
        yprp_node(pctx, data);
    }

    LEVEL--;
    ypr_close(pctx, "notification", flag);
}

static void
yprp_action(struct lys_ypr_ctx *pctx, const struct lysp_node_action *action)
{
    LY_ARRAY_COUNT_TYPE u;
    int8_t flag = 0;
    struct lysp_node_grp *grp;

    ypr_open(pctx, action->parent ? "action" : "rpc", "name", action->name, flag);

    LEVEL++;
    yprp_extension_instances(pctx, lyplg_ext_nodetype2stmt(action->nodetype), 0, action->exts, &flag);
    yprp_iffeatures(pctx, action->iffeatures, action->exts, &flag);
    ypr_status(pctx, action->flags, action->exts, &flag);
    ypr_description(pctx, action->dsc, action->exts, &flag);
    ypr_reference(pctx, action->ref, action->exts, &flag);

    LY_ARRAY_FOR(action->typedefs, u) {
        ypr_close_parent(pctx, &flag);
        yprp_typedef(pctx, &action->typedefs[u]);
    }

    LY_LIST_FOR(action->groupings, grp) {
        ypr_close_parent(pctx, &flag);
        yprp_grouping(pctx, grp);
    }

    yprp_inout(pctx, &action->input, &flag);
    yprp_inout(pctx, &action->output, &flag);

    LEVEL--;
    ypr_close(pctx, action->parent ? "action" : "rpc", flag);
}

static void
yprp_node_common1(struct lys_ypr_ctx *pctx, const struct lysp_node *node, int8_t *flag)
{
    ypr_open(pctx, lys_nodetype2str(node->nodetype), "name", node->name, *flag);
    LEVEL++;

    yprp_extension_instances(pctx, lyplg_ext_nodetype2stmt(node->nodetype), 0, node->exts, flag);
    yprp_when(pctx, lysp_node_when(node), flag);
    yprp_iffeatures(pctx, node->iffeatures, node->exts, flag);
}

static void
yprp_node_common2(struct lys_ypr_ctx *pctx, const struct lysp_node *node, int8_t *flag)
{
    ypr_config(pctx, node->flags, node->exts, flag);
    if (node->nodetype & (LYS_CHOICE | LYS_LEAF | LYS_ANYDATA)) {
        ypr_mandatory(pctx, node->flags, node->exts, flag);
    }
    ypr_status(pctx, node->flags, node->exts, flag);
    ypr_description(pctx, node->dsc, node->exts, flag);
    ypr_reference(pctx, node->ref, node->exts, flag);
}

static void
yprp_container(struct lys_ypr_ctx *pctx, const struct lysp_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    int8_t flag = 0;
    struct lysp_node *child;
    struct lysp_node_action *action;
    struct lysp_node_notif *notif;
    struct lysp_node_grp *grp;
    struct lysp_node_container *cont = (struct lysp_node_container *)node;

    yprp_node_common1(pctx, node, &flag);

    LY_ARRAY_FOR(cont->musts, u) {
        ypr_close_parent(pctx, &flag);
        yprp_restr(pctx, &cont->musts[u], LY_STMT_MUST, "condition", &flag);
    }
    if (cont->presence) {
        ypr_close_parent(pctx, &flag);
        ypr_substmt(pctx, LY_STMT_PRESENCE, 0, cont->presence, cont->exts);
    }

    yprp_node_common2(pctx, node, &flag);

    LY_ARRAY_FOR(cont->typedefs, u) {
        ypr_close_parent(pctx, &flag);
        yprp_typedef(pctx, &cont->typedefs[u]);
    }

    LY_LIST_FOR(cont->groupings, grp) {
        ypr_close_parent(pctx, &flag);
        yprp_grouping(pctx, grp);
    }

    LY_LIST_FOR(cont->child, child) {
        ypr_close_parent(pctx, &flag);
        yprp_node(pctx, child);
    }

    LY_LIST_FOR(cont->actions, action) {
        ypr_close_parent(pctx, &flag);
        yprp_action(pctx, action);
    }

    LY_LIST_FOR(cont->notifs, notif) {
        ypr_close_parent(pctx, &flag);
        yprp_notification(pctx, notif);
    }

    LEVEL--;
    ypr_close(pctx, "container", flag);
}

static void
yprp_case(struct lys_ypr_ctx *pctx, const struct lysp_node *node)
{
    int8_t flag = 0;
    struct lysp_node *child;
    struct lysp_node_case *cas = (struct lysp_node_case *)node;

    yprp_node_common1(pctx, node, &flag);
    yprp_node_common2(pctx, node, &flag);

    LY_LIST_FOR(cas->child, child) {
        ypr_close_parent(pctx, &flag);
        yprp_node(pctx, child);
    }

    LEVEL--;
    ypr_close(pctx, "case", flag);
}

static void
yprp_choice(struct lys_ypr_ctx *pctx, const struct lysp_node *node)
{
    int8_t flag = 0;
    struct lysp_node *child;
    struct lysp_node_choice *choice = (struct lysp_node_choice *)node;

    yprp_node_common1(pctx, node, &flag);

    if (choice->dflt.str) {
        ypr_close_parent(pctx, &flag);
        ypr_substmt(pctx, LY_STMT_DEFAULT, 0, choice->dflt.str, choice->exts);
    }

    yprp_node_common2(pctx, node, &flag);

    LY_LIST_FOR(choice->child, child) {
        ypr_close_parent(pctx, &flag);
        yprp_node(pctx, child);
    }

    LEVEL--;
    ypr_close(pctx, "choice", flag);
}

static void
yprp_leaf(struct lys_ypr_ctx *pctx, const struct lysp_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_node_leaf *leaf = (struct lysp_node_leaf *)node;

    int8_t flag = 1;

    yprp_node_common1(pctx, node, &flag);

    yprp_type(pctx, &leaf->type);
    ypr_substmt(pctx, LY_STMT_UNITS, 0, leaf->units, leaf->exts);
    LY_ARRAY_FOR(leaf->musts, u) {
        yprp_restr(pctx, &leaf->musts[u], LY_STMT_MUST, "condition", &flag);
    }
    ypr_substmt(pctx, LY_STMT_DEFAULT, 0, leaf->dflt.str, leaf->exts);

    yprp_node_common2(pctx, node, &flag);

    LEVEL--;
    ypr_close(pctx, "leaf", flag);
}

static void
yprp_leaflist(struct lys_ypr_ctx *pctx, const struct lysp_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_node_leaflist *llist = (struct lysp_node_leaflist *)node;
    int8_t flag = 1;

    yprp_node_common1(pctx, node, &flag);

    yprp_type(pctx, &llist->type);
    ypr_substmt(pctx, LY_STMT_UNITS, 0, llist->units, llist->exts);
    LY_ARRAY_FOR(llist->musts, u) {
        yprp_restr(pctx, &llist->musts[u], LY_STMT_MUST, "condition", NULL);
    }
    LY_ARRAY_FOR(llist->dflts, u) {
        ypr_substmt(pctx, LY_STMT_DEFAULT, u, llist->dflts[u].str, llist->exts);
    }

    ypr_config(pctx, node->flags, node->exts, NULL);

    if (llist->flags & LYS_SET_MIN) {
        ypr_unsigned(pctx, LY_STMT_MIN_ELEMENTS, 0, llist->exts, llist->min);
    }
    if (llist->flags & LYS_SET_MAX) {
        if (llist->max) {
            ypr_unsigned(pctx, LY_STMT_MAX_ELEMENTS, 0, llist->exts, llist->max);
        } else {
            ypr_substmt(pctx, LY_STMT_MAX_ELEMENTS, 0, "unbounded", llist->exts);
        }
    }

    if (llist->flags & LYS_ORDBY_MASK) {
        ypr_substmt(pctx, LY_STMT_ORDERED_BY, 0, (llist->flags & LYS_ORDBY_USER) ? "user" : "system", llist->exts);
    }

    ypr_status(pctx, node->flags, node->exts, &flag);
    ypr_description(pctx, node->dsc, node->exts, &flag);
    ypr_reference(pctx, node->ref, node->exts, &flag);

    LEVEL--;
    ypr_close(pctx, "leaf-list", flag);
}

static void
yprp_list(struct lys_ypr_ctx *pctx, const struct lysp_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    int8_t flag = 0;
    struct lysp_node *child;
    struct lysp_node_action *action;
    struct lysp_node_notif *notif;
    struct lysp_node_grp *grp;
    struct lysp_node_list *list = (struct lysp_node_list *)node;

    yprp_node_common1(pctx, node, &flag);

    LY_ARRAY_FOR(list->musts, u) {
        ypr_close_parent(pctx, &flag);
        yprp_restr(pctx, &list->musts[u], LY_STMT_MUST, "condition", &flag);
    }
    if (list->key) {
        ypr_close_parent(pctx, &flag);
        ypr_substmt(pctx, LY_STMT_KEY, 0, list->key, list->exts);
    }
    LY_ARRAY_FOR(list->uniques, u) {
        ypr_close_parent(pctx, &flag);
        ypr_substmt(pctx, LY_STMT_UNIQUE, u, list->uniques[u].str, list->exts);
    }

    ypr_config(pctx, node->flags, node->exts, &flag);

    if (list->flags & LYS_SET_MIN) {
        ypr_close_parent(pctx, &flag);
        ypr_unsigned(pctx, LY_STMT_MIN_ELEMENTS, 0, list->exts, list->min);
    }
    if (list->flags & LYS_SET_MAX) {
        ypr_close_parent(pctx, &flag);
        if (list->max) {
            ypr_unsigned(pctx, LY_STMT_MAX_ELEMENTS, 0, list->exts, list->max);
        } else {
            ypr_substmt(pctx, LY_STMT_MAX_ELEMENTS, 0, "unbounded", list->exts);
        }
    }

    if (list->flags & LYS_ORDBY_MASK) {
        ypr_close_parent(pctx, &flag);
        ypr_substmt(pctx, LY_STMT_ORDERED_BY, 0, (list->flags & LYS_ORDBY_USER) ? "user" : "system", list->exts);
    }

    ypr_status(pctx, node->flags, node->exts, &flag);
    ypr_description(pctx, node->dsc, node->exts, &flag);
    ypr_reference(pctx, node->ref, node->exts, &flag);

    LY_ARRAY_FOR(list->typedefs, u) {
        ypr_close_parent(pctx, &flag);
        yprp_typedef(pctx, &list->typedefs[u]);
    }

    LY_LIST_FOR(list->groupings, grp) {
        ypr_close_parent(pctx, &flag);
        yprp_grouping(pctx, grp);
    }

    LY_LIST_FOR(list->child, child) {
        ypr_close_parent(pctx, &flag);
        yprp_node(pctx, child);
    }

    LY_LIST_FOR(list->actions, action) {
        ypr_close_parent(pctx, &flag);
        yprp_action(pctx, action);
    }

    LY_LIST_FOR(list->notifs, notif) {
        ypr_close_parent(pctx, &flag);
        yprp_notification(pctx, notif);
    }

    LEVEL--;
    ypr_close(pctx, "list", flag);
}

static void
yprp_refine(struct lys_ypr_ctx *pctx, struct lysp_refine *refine)
{
    LY_ARRAY_COUNT_TYPE u;
    int8_t flag = 0;

    ypr_open(pctx, "refine", "target-node", refine->nodeid, flag);
    LEVEL++;

    yprp_extension_instances(pctx, LY_STMT_REFINE, 0, refine->exts, &flag);
    yprp_iffeatures(pctx, refine->iffeatures, refine->exts, &flag);

    LY_ARRAY_FOR(refine->musts, u) {
        ypr_close_parent(pctx, &flag);
        yprp_restr(pctx, &refine->musts[u], LY_STMT_MUST, "condition", &flag);
    }

    if (refine->presence) {
        ypr_close_parent(pctx, &flag);
        ypr_substmt(pctx, LY_STMT_PRESENCE, 0, refine->presence, refine->exts);
    }

    LY_ARRAY_FOR(refine->dflts, u) {
        ypr_close_parent(pctx, &flag);
        ypr_substmt(pctx, LY_STMT_DEFAULT, u, refine->dflts[u].str, refine->exts);
    }

    ypr_config(pctx, refine->flags, refine->exts, &flag);
    ypr_mandatory(pctx, refine->flags, refine->exts, &flag);

    if (refine->flags & LYS_SET_MIN) {
        ypr_close_parent(pctx, &flag);
        ypr_unsigned(pctx, LY_STMT_MIN_ELEMENTS, 0, refine->exts, refine->min);
    }
    if (refine->flags & LYS_SET_MAX) {
        ypr_close_parent(pctx, &flag);
        if (refine->max) {
            ypr_unsigned(pctx, LY_STMT_MAX_ELEMENTS, 0, refine->exts, refine->max);
        } else {
            ypr_substmt(pctx, LY_STMT_MAX_ELEMENTS, 0, "unbounded", refine->exts);
        }
    }

    ypr_description(pctx, refine->dsc, refine->exts, &flag);
    ypr_reference(pctx, refine->ref, refine->exts, &flag);

    LEVEL--;
    ypr_close(pctx, "refine", flag);
}

static void
yprp_augment(struct lys_ypr_ctx *pctx, const struct lysp_node_augment *aug)
{
    struct lysp_node *child;
    struct lysp_node_action *action;
    struct lysp_node_notif *notif;

    ypr_open(pctx, "augment", "target-node", aug->nodeid, 1);
    LEVEL++;

    yprp_extension_instances(pctx, LY_STMT_AUGMENT, 0, aug->exts, NULL);
    yprp_when(pctx, aug->when, NULL);
    yprp_iffeatures(pctx, aug->iffeatures, aug->exts, NULL);
    ypr_status(pctx, aug->flags, aug->exts, NULL);
    ypr_description(pctx, aug->dsc, aug->exts, NULL);
    ypr_reference(pctx, aug->ref, aug->exts, NULL);

    LY_LIST_FOR(aug->child, child) {
        yprp_node(pctx, child);
    }

    LY_LIST_FOR(aug->actions, action) {
        yprp_action(pctx, action);
    }

    LY_LIST_FOR(aug->notifs, notif) {
        yprp_notification(pctx, notif);
    }

    LEVEL--;
    ypr_close(pctx, "augment", 1);
}

static void
yprp_uses(struct lys_ypr_ctx *pctx, const struct lysp_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    int8_t flag = 0;
    struct lysp_node_uses *uses = (struct lysp_node_uses *)node;
    struct lysp_node_augment *aug;

    yprp_node_common1(pctx, node, &flag);
    yprp_node_common2(pctx, node, &flag);

    LY_ARRAY_FOR(uses->refines, u) {
        ypr_close_parent(pctx, &flag);
        yprp_refine(pctx, &uses->refines[u]);
    }

    LY_LIST_FOR(uses->augments, aug) {
        ypr_close_parent(pctx, &flag);
        yprp_augment(pctx, aug);
    }

    LEVEL--;
    ypr_close(pctx, "uses", flag);
}

static void
yprp_anydata(struct lys_ypr_ctx *pctx, const struct lysp_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    int8_t flag = 0;
    struct lysp_node_anydata *any = (struct lysp_node_anydata *)node;

    yprp_node_common1(pctx, node, &flag);

    LY_ARRAY_FOR(any->musts, u) {
        ypr_close_parent(pctx, &flag);
        yprp_restr(pctx, &any->musts[u], LY_STMT_MUST, "condition", &flag);
    }

    yprp_node_common2(pctx, node, &flag);

    LEVEL--;
    ypr_close(pctx, lys_nodetype2str(node->nodetype), flag);
}

static void
yprp_node(struct lys_ypr_ctx *pctx, const struct lysp_node *node)
{
    switch (node->nodetype) {
    case LYS_CONTAINER:
        yprp_container(pctx, node);
        break;
    case LYS_CHOICE:
        yprp_choice(pctx, node);
        break;
    case LYS_LEAF:
        yprp_leaf(pctx, node);
        break;
    case LYS_LEAFLIST:
        yprp_leaflist(pctx, node);
        break;
    case LYS_LIST:
        yprp_list(pctx, node);
        break;
    case LYS_USES:
        yprp_uses(pctx, node);
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        yprp_anydata(pctx, node);
        break;
    case LYS_CASE:
        yprp_case(pctx, node);
        break;
    default:
        break;
    }
}

static void
yprp_deviation(struct lys_ypr_ctx *pctx, const struct lysp_deviation *deviation)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_deviate_add *add;
    struct lysp_deviate_rpl *rpl;
    struct lysp_deviate_del *del;
    struct lysp_deviate *elem;

    ypr_open(pctx, "deviation", "target-node", deviation->nodeid, 1);
    LEVEL++;

    yprp_extension_instances(pctx, LY_STMT_DEVIATION, 0, deviation->exts, NULL);
    ypr_description(pctx, deviation->dsc, deviation->exts, NULL);
    ypr_reference(pctx, deviation->ref, deviation->exts, NULL);

    LY_LIST_FOR(deviation->deviates, elem) {
        ly_print_(pctx->out, "%*s<deviate value=\"", INDENT);
        if (elem->mod == LYS_DEV_NOT_SUPPORTED) {
            if (elem->exts) {
                ly_print_(pctx->out, "not-supported\">\n");
                LEVEL++;

                yprp_extension_instances(pctx, LY_STMT_DEVIATE, 0, elem->exts, NULL);
            } else {
                ly_print_(pctx->out, "not-supported\"/>\n");
                continue;
            }
        } else if (elem->mod == LYS_DEV_ADD) {
            add = (struct lysp_deviate_add *)elem;
            ly_print_(pctx->out, "add\">\n");
            LEVEL++;

            yprp_extension_instances(pctx, LY_STMT_DEVIATE, 0, add->exts, NULL);
            ypr_substmt(pctx, LY_STMT_UNITS, 0, add->units, add->exts);
            LY_ARRAY_FOR(add->musts, u) {
                yprp_restr(pctx, &add->musts[u], LY_STMT_MUST, "condition", NULL);
            }
            LY_ARRAY_FOR(add->uniques, u) {
                ypr_substmt(pctx, LY_STMT_UNIQUE, u, add->uniques[u].str, add->exts);
            }
            LY_ARRAY_FOR(add->dflts, u) {
                ypr_substmt(pctx, LY_STMT_DEFAULT, u, add->dflts[u].str, add->exts);
            }
            ypr_config(pctx, add->flags, add->exts, NULL);
            ypr_mandatory(pctx, add->flags, add->exts, NULL);
            if (add->flags & LYS_SET_MIN) {
                ypr_unsigned(pctx, LY_STMT_MIN_ELEMENTS, 0, add->exts, add->min);
            }
            if (add->flags & LYS_SET_MAX) {
                if (add->max) {
                    ypr_unsigned(pctx, LY_STMT_MAX_ELEMENTS, 0, add->exts, add->max);
                } else {
                    ypr_substmt(pctx, LY_STMT_MAX_ELEMENTS, 0, "unbounded", add->exts);
                }
            }
        } else if (elem->mod == LYS_DEV_REPLACE) {
            rpl = (struct lysp_deviate_rpl *)elem;
            ly_print_(pctx->out, "replace\">\n");
            LEVEL++;

            yprp_extension_instances(pctx, LY_STMT_DEVIATE, 0, rpl->exts, NULL);
            if (rpl->type) {
                yprp_type(pctx, rpl->type);
            }
            ypr_substmt(pctx, LY_STMT_UNITS, 0, rpl->units, rpl->exts);
            ypr_substmt(pctx, LY_STMT_DEFAULT, 0, rpl->dflt.str, rpl->exts);
            ypr_config(pctx, rpl->flags, rpl->exts, NULL);
            ypr_mandatory(pctx, rpl->flags, rpl->exts, NULL);
            if (rpl->flags & LYS_SET_MIN) {
                ypr_unsigned(pctx, LY_STMT_MIN_ELEMENTS, 0, rpl->exts, rpl->min);
            }
            if (rpl->flags & LYS_SET_MAX) {
                if (rpl->max) {
                    ypr_unsigned(pctx, LY_STMT_MAX_ELEMENTS, 0, rpl->exts, rpl->max);
                } else {
                    ypr_substmt(pctx, LY_STMT_MAX_ELEMENTS, 0, "unbounded", rpl->exts);
                }
            }
        } else if (elem->mod == LYS_DEV_DELETE) {
            del = (struct lysp_deviate_del *)elem;
            ly_print_(pctx->out, "delete\">\n");
            LEVEL++;

            yprp_extension_instances(pctx, LY_STMT_DEVIATE, 0, del->exts, NULL);
            ypr_substmt(pctx, LY_STMT_UNITS, 0, del->units, del->exts);
            LY_ARRAY_FOR(del->musts, u) {
                yprp_restr(pctx, &del->musts[u], LY_STMT_MUST, "condition", NULL);
            }
            LY_ARRAY_FOR(del->uniques, u) {
                ypr_substmt(pctx, LY_STMT_UNIQUE, u, del->uniques[u].str, del->exts);
            }
            LY_ARRAY_FOR(del->dflts, u) {
                ypr_substmt(pctx, LY_STMT_DEFAULT, u, del->dflts[u].str, del->exts);
            }
        }

        LEVEL--;
        ypr_close(pctx, "deviate", 1);
    }

    LEVEL--;
    ypr_close(pctx, "deviation", 1);
}

static void
ypr_xmlns(struct lys_ypr_ctx *pctx, const struct lys_module *module, uint16_t indent)
{
    ly_print_(pctx->out, "%*sxmlns=\"%s\"", indent + INDENT, YIN_NS_URI);
    ly_print_(pctx->out, "\n%*sxmlns:%s=\"%s\"", indent + INDENT, module->prefix, module->ns);
}

static void
ypr_import_xmlns(struct lys_ypr_ctx *pctx, const struct lysp_module *modp, uint16_t indent)
{
    LY_ARRAY_COUNT_TYPE u;

    LY_ARRAY_FOR(modp->imports, u){
        if (!(modp->imports[u].flags & LYS_INTERNAL)) {
            ly_print_(pctx->out, "\n%*sxmlns:%s=\"%s\"", indent + INDENT, modp->imports[u].prefix, modp->imports[u].module->ns);
        }
    }
}

static void
yin_print_parsed_linkage(struct lys_ypr_ctx *pctx, const struct lysp_module *modp)
{
    LY_ARRAY_COUNT_TYPE u;

    LY_ARRAY_FOR(modp->imports, u) {
        if (modp->imports[u].flags & LYS_INTERNAL) {
            continue;
        }

        ypr_open(pctx, "import", "module", modp->imports[u].name, 1);
        LEVEL++;
        yprp_extension_instances(pctx, LY_STMT_IMPORT, 0, modp->imports[u].exts, NULL);
        ypr_substmt(pctx, LY_STMT_PREFIX, 0, modp->imports[u].prefix, modp->imports[u].exts);
        if (modp->imports[u].rev[0]) {
            ypr_substmt(pctx, LY_STMT_REVISION_DATE, 0, modp->imports[u].rev, modp->imports[u].exts);
        }
        ypr_substmt(pctx, LY_STMT_DESCRIPTION, 0, modp->imports[u].dsc, modp->imports[u].exts);
        ypr_substmt(pctx, LY_STMT_REFERENCE, 0, modp->imports[u].ref, modp->imports[u].exts);
        LEVEL--;
        ypr_close(pctx, "import", 1);
    }
    LY_ARRAY_FOR(modp->includes, u) {
        if (modp->includes[u].injected) {
            /* do not print the includes injected from submodules */
            continue;
        }
        if (modp->includes[u].rev[0] || modp->includes[u].dsc || modp->includes[u].ref || modp->includes[u].exts) {
            ypr_open(pctx, "include", "module", modp->includes[u].name, 1);
            LEVEL++;
            yprp_extension_instances(pctx, LY_STMT_INCLUDE, 0, modp->includes[u].exts, NULL);
            if (modp->includes[u].rev[0]) {
                ypr_substmt(pctx, LY_STMT_REVISION_DATE, 0, modp->includes[u].rev, modp->includes[u].exts);
            }
            ypr_substmt(pctx, LY_STMT_DESCRIPTION, 0, modp->includes[u].dsc, modp->includes[u].exts);
            ypr_substmt(pctx, LY_STMT_REFERENCE, 0, modp->includes[u].ref, modp->includes[u].exts);
            LEVEL--;
            ypr_close(pctx, "include", 1);
        } else {
            ypr_open(pctx, "include", "module", modp->includes[u].name, -1);
        }
    }
}

static void
yin_print_parsed_body(struct lys_ypr_ctx *pctx, const struct lysp_module *modp)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_node *data;
    struct lysp_node_action *action;
    struct lysp_node_notif *notif;
    struct lysp_node_grp *grp;
    struct lysp_node_augment *aug;

    LY_ARRAY_FOR(modp->extensions, u) {
        yprp_extension(pctx, &modp->extensions[u]);
    }
    if (modp->exts) {
        yprp_extension_instances(pctx, LY_STMT_MODULE, 0, modp->exts, NULL);
    }

    LY_ARRAY_FOR(modp->features, u) {
        yprp_feature(pctx, &modp->features[u]);
    }

    LY_ARRAY_FOR(modp->identities, u) {
        yprp_identity(pctx, &modp->identities[u]);
    }

    LY_ARRAY_FOR(modp->typedefs, u) {
        yprp_typedef(pctx, &modp->typedefs[u]);
    }

    LY_LIST_FOR(modp->groupings, grp) {
        yprp_grouping(pctx, grp);
    }

    LY_LIST_FOR(modp->data, data) {
        yprp_node(pctx, data);
    }

    LY_LIST_FOR(modp->augments, aug) {
        yprp_augment(pctx, aug);
    }

    LY_LIST_FOR(modp->rpcs, action) {
        yprp_action(pctx, action);
    }

    LY_LIST_FOR(modp->notifs, notif) {
        yprp_notification(pctx, notif);
    }

    LY_ARRAY_FOR(modp->deviations, u) {
        yprp_deviation(pctx, &modp->deviations[u]);
    }
}

LY_ERR
yin_print_parsed_module(struct ly_out *out, const struct lysp_module *modp, uint32_t options)
{
    LY_ARRAY_COUNT_TYPE u;
    const struct lys_module *module = modp->mod;
    struct lys_ypr_ctx pctx_ = {.out = out, .level = 0, .module = module, .options = options}, *pctx = &pctx_;

    ly_print_(pctx->out, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    ly_print_(pctx->out, "%*s<module name=\"%s\"\n", INDENT, module->name);
    ypr_xmlns(pctx, module, XML_NS_INDENT);
    ypr_import_xmlns(pctx, modp, XML_NS_INDENT);
    ly_print_(pctx->out, ">\n");

    LEVEL++;

    /* module-header-stmts */
    if (modp->version) {
        ypr_substmt(pctx, LY_STMT_YANG_VERSION, 0, modp->version == LYS_VERSION_1_1 ? "1.1" : "1", modp->exts);
    }
    ypr_substmt(pctx, LY_STMT_NAMESPACE, 0, module->ns, modp->exts);
    ypr_substmt(pctx, LY_STMT_PREFIX, 0, module->prefix, modp->exts);

    /* linkage-stmts (import/include) */
    yin_print_parsed_linkage(pctx, modp);

    /* meta-stmts */
    if (module->org || module->contact || module->dsc || module->ref) {
        ly_print_(out, "\n");
    }
    ypr_substmt(pctx, LY_STMT_ORGANIZATION, 0, module->org, modp->exts);
    ypr_substmt(pctx, LY_STMT_CONTACT, 0, module->contact, modp->exts);
    ypr_substmt(pctx, LY_STMT_DESCRIPTION, 0, module->dsc, modp->exts);
    ypr_substmt(pctx, LY_STMT_REFERENCE, 0, module->ref, modp->exts);

    /* revision-stmts */
    if (modp->revs) {
        ly_print_(out, "\n");
    }
    LY_ARRAY_FOR(modp->revs, u) {
        yprp_revision(pctx, &modp->revs[u]);
    }

    /* body-stmts */
    yin_print_parsed_body(pctx, modp);

    LEVEL--;
    ly_print_(out, "%*s</module>\n", INDENT);
    ly_print_flush(out);

    return LY_SUCCESS;
}

static void
yprp_belongsto(struct lys_ypr_ctx *pctx, const struct lysp_submodule *submodp)
{
    ypr_open(pctx, "belongs-to", "module", submodp->mod->name, 1);
    LEVEL++;
    yprp_extension_instances(pctx, LY_STMT_BELONGS_TO, 0, submodp->exts, NULL);
    ypr_substmt(pctx, LY_STMT_PREFIX, 0, submodp->prefix, submodp->exts);
    LEVEL--;
    ypr_close(pctx, "belongs-to", 1);
}

LY_ERR
yin_print_parsed_submodule(struct ly_out *out, const struct lysp_submodule *submodp, uint32_t options)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lys_ypr_ctx pctx_ = {.out = out, .level = 0, .module = submodp->mod, .options = options}, *pctx = &pctx_;

    ly_print_(pctx->out, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    ly_print_(pctx->out, "%*s<submodule name=\"%s\"\n", INDENT, submodp->name);
    ypr_xmlns(pctx, submodp->mod, XML_NS_INDENT);
    ypr_import_xmlns(pctx, (struct lysp_module *)submodp, XML_NS_INDENT);
    ly_print_(pctx->out, ">\n");

    LEVEL++;

    /* submodule-header-stmts */
    if (submodp->version) {
        ypr_substmt(pctx, LY_STMT_YANG_VERSION, 0, submodp->version == LYS_VERSION_1_1 ? "1.1" : "1", submodp->exts);
    }
    yprp_belongsto(pctx, submodp);

    /* linkage-stmts (import/include) */
    yin_print_parsed_linkage(pctx, (struct lysp_module *)submodp);

    /* meta-stmts */
    if (submodp->org || submodp->contact || submodp->dsc || submodp->ref) {
        ly_print_(out, "\n");
    }
    ypr_substmt(pctx, LY_STMT_ORGANIZATION, 0, submodp->org, submodp->exts);
    ypr_substmt(pctx, LY_STMT_CONTACT, 0, submodp->contact, submodp->exts);
    ypr_substmt(pctx, LY_STMT_DESCRIPTION, 0, submodp->dsc, submodp->exts);
    ypr_substmt(pctx, LY_STMT_REFERENCE, 0, submodp->ref, submodp->exts);

    /* revision-stmts */
    if (submodp->revs) {
        ly_print_(out, "\n");
    }
    LY_ARRAY_FOR(submodp->revs, u) {
        yprp_revision(pctx, &submodp->revs[u]);
    }

    /* body-stmts */
    yin_print_parsed_body(pctx, (struct lysp_module *)submodp);

    LEVEL--;
    ly_print_(out, "%*s</submodule>\n", INDENT);
    ly_print_flush(out);

    return LY_SUCCESS;
}
