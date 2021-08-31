/**
 * @file printer_yin.c
 * @author Fred Gan <ganshaolong@vip.qq.com>
 * @brief YIN printer
 *
 * Copyright (c) 2015 - 2019 CESNET, z.s.p.o.
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

#include "common.h"
#include "compat.h"
#include "log.h"
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

static void yprp_extension_instances(struct lys_ypr_ctx *ctx, enum ly_stmt substmt, uint8_t substmt_index,
        struct lysp_ext_instance *ext, int8_t *flag, LY_ARRAY_COUNT_TYPE count);

static void
ypr_open(struct lys_ypr_ctx *ctx, const char *elem_name, const char *attr_name, const char *attr_value, int8_t flag)
{
    ly_print_(ctx->out, "%*s<%s", INDENT, elem_name);

    if (attr_name) {
        ly_print_(ctx->out, " %s=\"", attr_name);
        lyxml_dump_text(ctx->out, attr_value, 1);
        ly_print_(ctx->out, "\"%s", flag == -1 ? "/>\n" : flag == 1 ? ">\n" : "");
    } else if (flag) {
        ly_print_(ctx->out, flag == -1 ? "/>\n" : ">\n");
    }
}

static void
ypr_close(struct lys_ypr_ctx *ctx, const char *elem_name, int8_t flag)
{
    if (flag) {
        ly_print_(ctx->out, "%*s</%s>\n", INDENT, elem_name);
    } else {
        ly_print_(ctx->out, "/>\n");
    }
}

/*
 * par_close_flag
 * 0 - parent not yet closed, printing >\n, setting flag to 1
 * 1 or NULL - parent already closed, do nothing
 */
static void
ypr_close_parent(struct lys_ypr_ctx *ctx, int8_t *par_close_flag)
{
    if (par_close_flag && !(*par_close_flag)) {
        (*par_close_flag) = 1;
        ly_print_(ctx->out, ">\n");
    }
}

static void
ypr_yin_arg(struct lys_ypr_ctx *ctx, const char *arg, const char *text)
{
    ly_print_(ctx->out, "%*s<%s>", INDENT, arg);
    lyxml_dump_text(ctx->out, text, 0);
    ly_print_(ctx->out, "</%s>\n", arg);
}

static void
ypr_substmt(struct lys_ypr_ctx *ctx, enum ly_stmt substmt, uint8_t substmt_index, const char *text, void *ext)
{
    LY_ARRAY_COUNT_TYPE u;
    int8_t extflag = 0;

    if (!text) {
        /* nothing to print */
        return;
    }

    if (stmt_attr_info[substmt].flags & STMT_FLAG_YIN) {
        extflag = 1;
        ypr_open(ctx, stmt_attr_info[substmt].name, NULL, NULL, extflag);
    } else {
        ypr_open(ctx, stmt_attr_info[substmt].name, stmt_attr_info[substmt].arg, text, extflag);
    }

    LEVEL++;
    LY_ARRAY_FOR(ext, u) {
        if ((((struct lysp_ext_instance *)ext)[u].parent_stmt != substmt) || (((struct lysp_ext_instance *)ext)[u].parent_stmt_index != substmt_index)) {
            continue;
        }
        yprp_extension_instances(ctx, substmt, substmt_index, &((struct lysp_ext_instance *)ext)[u], &extflag, 1);
    }

    /* argument as yin-element */
    if (stmt_attr_info[substmt].flags & STMT_FLAG_YIN) {
        ypr_yin_arg(ctx, stmt_attr_info[substmt].arg, text);
    }

    LEVEL--;
    ypr_close(ctx, stmt_attr_info[substmt].name, extflag);
}

static void
ypr_unsigned(struct lys_ypr_ctx *ctx, enum ly_stmt substmt, uint8_t substmt_index, void *exts, unsigned long int attr_value)
{
    char *str;

    if (asprintf(&str, "%lu", attr_value) == -1) {
        LOGMEM(ctx->module->ctx);
        return;
    }
    ypr_substmt(ctx, substmt, substmt_index, str, exts);
    free(str);
}

static void
ypr_signed(struct lys_ypr_ctx *ctx, enum ly_stmt substmt, uint8_t substmt_index, void *exts, signed long int attr_value)
{
    char *str;

    if (asprintf(&str, "%ld", attr_value) == -1) {
        LOGMEM(ctx->module->ctx);
        return;
    }
    ypr_substmt(ctx, substmt, substmt_index, str, exts);
    free(str);
}

static void
yprp_revision(struct lys_ypr_ctx *ctx, const struct lysp_revision *rev)
{
    if (rev->dsc || rev->ref || rev->exts) {
        ypr_open(ctx, "revision", "date", rev->date, 1);
        LEVEL++;
        yprp_extension_instances(ctx, LY_STMT_REVISION, 0, rev->exts, NULL, 0);
        ypr_substmt(ctx, LY_STMT_DESCRIPTION, 0, rev->dsc, rev->exts);
        ypr_substmt(ctx, LY_STMT_REFERENCE, 0, rev->ref, rev->exts);
        LEVEL--;
        ypr_close(ctx, "revision", 1);
    } else {
        ypr_open(ctx, "revision", "date", rev->date, -1);
    }
}

static void
ypr_mandatory(struct lys_ypr_ctx *ctx, uint16_t flags, void *exts, int8_t *flag)
{
    if (flags & LYS_MAND_MASK) {
        ypr_close_parent(ctx, flag);
        ypr_substmt(ctx, LY_STMT_MANDATORY, 0, (flags & LYS_MAND_TRUE) ? "true" : "false", exts);
    }
}

static void
ypr_config(struct lys_ypr_ctx *ctx, uint16_t flags, void *exts, int8_t *flag)
{
    if (flags & LYS_CONFIG_MASK) {
        ypr_close_parent(ctx, flag);
        ypr_substmt(ctx, LY_STMT_CONFIG, 0, (flags & LYS_CONFIG_W) ? "true" : "false", exts);
    }
}

static void
ypr_status(struct lys_ypr_ctx *ctx, uint16_t flags, void *exts, int8_t *flag)
{
    const char *status = NULL;

    if (flags & LYS_STATUS_CURR) {
        ypr_close_parent(ctx, flag);
        status = "current";
    } else if (flags & LYS_STATUS_DEPRC) {
        ypr_close_parent(ctx, flag);
        status = "deprecated";
    } else if (flags & LYS_STATUS_OBSLT) {
        ypr_close_parent(ctx, flag);
        status = "obsolete";
    }

    ypr_substmt(ctx, LY_STMT_STATUS, 0, status, exts);
}

static void
ypr_description(struct lys_ypr_ctx *ctx, const char *dsc, void *exts, int8_t *flag)
{
    if (dsc) {
        ypr_close_parent(ctx, flag);
        ypr_substmt(ctx, LY_STMT_DESCRIPTION, 0, dsc, exts);
    }
}

static void
ypr_reference(struct lys_ypr_ctx *ctx, const char *ref, void *exts, int8_t *flag)
{
    if (ref) {
        ypr_close_parent(ctx, flag);
        ypr_substmt(ctx, LY_STMT_REFERENCE, 0, ref, exts);
    }
}

static void
yprp_iffeatures(struct lys_ypr_ctx *ctx, struct lysp_qname *iffs, struct lysp_ext_instance *exts, int8_t *flag)
{
    LY_ARRAY_COUNT_TYPE u, v;
    int8_t extflag;

    LY_ARRAY_FOR(iffs, u) {
        ypr_close_parent(ctx, flag);
        extflag = 0;

        ly_print_(ctx->out, "%*s<if-feature name=\"%s",  INDENT, iffs[u].str);

        /* extensions */
        LEVEL++;
        LY_ARRAY_FOR(exts, v) {
            yprp_extension_instances(ctx, LY_STMT_IF_FEATURE, u, &exts[v], &extflag, 1);
        }
        LEVEL--;
        ly_print_(ctx->out, "\"/>\n");
    }
}

static void
yprp_extension(struct lys_ypr_ctx *ctx, const struct lysp_ext *ext)
{
    int8_t flag = 0, flag2 = 0;
    LY_ARRAY_COUNT_TYPE u;

    ypr_open(ctx, "extension", "name", ext->name, flag);
    LEVEL++;

    if (ext->exts) {
        ypr_close_parent(ctx, &flag);
        yprp_extension_instances(ctx, LY_STMT_EXTENSION, 0, ext->exts, &flag, 0);
    }

    if (ext->argname) {
        ypr_close_parent(ctx, &flag);
        ypr_open(ctx, "argument", "name", ext->argname, flag2);

        LEVEL++;
        if (ext->exts) {
            u = -1;
            while ((u = lysp_ext_instance_iter(ext->exts, u + 1, LY_STMT_ARGUMENT)) != LY_ARRAY_COUNT(ext->exts)) {
                ypr_close_parent(ctx, &flag2);
                yprp_extension_instances(ctx, LY_STMT_ARGUMENT, 0, &ext->exts[u], &flag2, 1);
            }
        }
        if ((ext->flags & LYS_YINELEM_MASK) ||
                (ext->exts && (lysp_ext_instance_iter(ext->exts, 0, LY_STMT_YIN_ELEMENT) != LY_ARRAY_COUNT(ext->exts)))) {
            ypr_close_parent(ctx, &flag2);
            ypr_substmt(ctx, LY_STMT_YIN_ELEMENT, 0, (ext->flags & LYS_YINELEM_TRUE) ? "true" : "false", ext->exts);
        }
        LEVEL--;
        ypr_close(ctx, "argument", flag2);
    }

    ypr_status(ctx, ext->flags, ext->exts, &flag);
    ypr_description(ctx, ext->dsc, ext->exts, &flag);
    ypr_reference(ctx, ext->ref, ext->exts, &flag);

    LEVEL--;
    ypr_close(ctx, "extension", flag);
}

static void
yprp_feature(struct lys_ypr_ctx *ctx, const struct lysp_feature *feat)
{
    int8_t flag = 0;

    ypr_open(ctx, "feature", "name", feat->name, flag);
    LEVEL++;
    yprp_extension_instances(ctx, LY_STMT_FEATURE, 0, feat->exts, &flag, 0);
    yprp_iffeatures(ctx, feat->iffeatures, feat->exts, &flag);
    ypr_status(ctx, feat->flags, feat->exts, &flag);
    ypr_description(ctx, feat->dsc, feat->exts, &flag);
    ypr_reference(ctx, feat->ref, feat->exts, &flag);
    LEVEL--;
    ypr_close(ctx, "feature", flag);
}

static void
yprp_identity(struct lys_ypr_ctx *ctx, const struct lysp_ident *ident)
{
    int8_t flag = 0;
    LY_ARRAY_COUNT_TYPE u;

    ypr_open(ctx, "identity", "name", ident->name, flag);
    LEVEL++;

    yprp_extension_instances(ctx, LY_STMT_IDENTITY, 0, ident->exts, &flag, 0);
    yprp_iffeatures(ctx, ident->iffeatures, ident->exts, &flag);

    LY_ARRAY_FOR(ident->bases, u) {
        ypr_close_parent(ctx, &flag);
        ypr_substmt(ctx, LY_STMT_BASE, u, ident->bases[u], ident->exts);
    }

    ypr_status(ctx, ident->flags, ident->exts, &flag);
    ypr_description(ctx, ident->dsc, ident->exts, &flag);
    ypr_reference(ctx, ident->ref, ident->exts, &flag);

    LEVEL--;
    ypr_close(ctx, "identity", flag);
}

static void
yprp_restr(struct lys_ypr_ctx *ctx, const struct lysp_restr *restr, enum ly_stmt stmt, const char *attr, int8_t *flag)
{
    (void)flag;
    int8_t inner_flag = 0;

    if (!restr) {
        return;
    }

    ly_print_(ctx->out, "%*s<%s %s=\"", INDENT, ly_stmt2str(stmt), attr);
    lyxml_dump_text(ctx->out,
            (restr->arg.str[0] != LYSP_RESTR_PATTERN_NACK && restr->arg.str[0] != LYSP_RESTR_PATTERN_ACK) ?
            restr->arg.str : &restr->arg.str[1], 1);
    ly_print_(ctx->out, "\"");

    LEVEL++;
    yprp_extension_instances(ctx, stmt, 0, restr->exts, &inner_flag, 0);
    if (restr->arg.str[0] == LYSP_RESTR_PATTERN_NACK) {
        ypr_close_parent(ctx, &inner_flag);
        /* special byte value in pattern's expression: 0x15 - invert-match, 0x06 - match */
        ypr_substmt(ctx, LY_STMT_MODIFIER, 0, "invert-match", restr->exts);
    }
    if (restr->emsg) {
        ypr_close_parent(ctx, &inner_flag);
        ypr_substmt(ctx, LY_STMT_ERROR_MESSAGE, 0, restr->emsg, restr->exts);
    }
    if (restr->eapptag) {
        ypr_close_parent(ctx, &inner_flag);
        ypr_substmt(ctx, LY_STMT_ERROR_APP_TAG, 0, restr->eapptag, restr->exts);
    }
    ypr_description(ctx, restr->dsc, restr->exts, &inner_flag);
    ypr_reference(ctx, restr->ref, restr->exts, &inner_flag);

    LEVEL--;
    ypr_close(ctx, ly_stmt2str(stmt), inner_flag);
}

static void
yprp_when(struct lys_ypr_ctx *ctx, struct lysp_when *when, int8_t *flag)
{
    int8_t inner_flag = 0;

    (void)flag;

    if (!when) {
        return;
    }

    ly_print_(ctx->out, "%*s<when condition=\"", INDENT);
    lyxml_dump_text(ctx->out, when->cond, 1);
    ly_print_(ctx->out, "\"");

    LEVEL++;
    yprp_extension_instances(ctx, LY_STMT_WHEN, 0, when->exts, &inner_flag, 0);
    ypr_description(ctx, when->dsc, when->exts, &inner_flag);
    ypr_reference(ctx, when->ref, when->exts, &inner_flag);
    LEVEL--;
    ypr_close(ctx, "when", inner_flag);
}

static void
yprp_enum(struct lys_ypr_ctx *ctx, const struct lysp_type_enum *items, LY_DATA_TYPE type, int8_t *flag)
{
    LY_ARRAY_COUNT_TYPE u;
    int8_t inner_flag;

    (void)flag;

    LY_ARRAY_FOR(items, u) {
        if (type == LY_TYPE_BITS) {
            ly_print_(ctx->out, "%*s<bit name=\"", INDENT);
            lyxml_dump_text(ctx->out, items[u].name, 1);
            ly_print_(ctx->out, "\"");
        } else { /* LY_TYPE_ENUM */
            ly_print_(ctx->out, "%*s<enum name=\"", INDENT);
            lyxml_dump_text(ctx->out, items[u].name, 1);
            ly_print_(ctx->out, "\"");
        }
        inner_flag = 0;
        LEVEL++;
        yprp_extension_instances(ctx, LY_STMT_ENUM, 0, items[u].exts, &inner_flag, 0);
        yprp_iffeatures(ctx, items[u].iffeatures, items[u].exts, &inner_flag);
        if (items[u].flags & LYS_SET_VALUE) {
            if (type == LY_TYPE_BITS) {
                ypr_close_parent(ctx, &inner_flag);
                ypr_unsigned(ctx, LY_STMT_POSITION, 0, items[u].exts, items[u].value);
            } else { /* LY_TYPE_ENUM */
                ypr_close_parent(ctx, &inner_flag);
                ypr_signed(ctx, LY_STMT_VALUE, 0, items[u].exts, items[u].value);
            }
        }
        ypr_status(ctx, items[u].flags, items[u].exts, &inner_flag);
        ypr_description(ctx, items[u].dsc, items[u].exts, &inner_flag);
        ypr_reference(ctx, items[u].ref, items[u].exts, &inner_flag);
        LEVEL--;
        ypr_close(ctx, type == LY_TYPE_BITS ? "bit" : "enum", inner_flag);
    }
}

static void
yprp_type(struct lys_ypr_ctx *ctx, const struct lysp_type *type)
{
    LY_ARRAY_COUNT_TYPE u;
    int8_t flag = 0;

    if (!ctx || !type) {
        return;
    }

    ypr_open(ctx, "type", "name", type->name, flag);
    LEVEL++;

    yprp_extension_instances(ctx, LY_STMT_TYPE, 0, type->exts, &flag, 0);

    if (type->range || type->length || type->patterns || type->bits || type->enums) {
        ypr_close_parent(ctx, &flag);
    }
    yprp_restr(ctx, type->range, LY_STMT_RANGE, "value", &flag);
    yprp_restr(ctx, type->length, LY_STMT_LENGTH, "value", &flag);
    LY_ARRAY_FOR(type->patterns, u) {
        yprp_restr(ctx, &type->patterns[u], LY_STMT_PATTERN, "value", &flag);
    }
    yprp_enum(ctx, type->bits, LY_TYPE_BITS, &flag);
    yprp_enum(ctx, type->enums, LY_TYPE_ENUM, &flag);

    if (type->path) {
        ypr_close_parent(ctx, &flag);
        ypr_substmt(ctx, LY_STMT_PATH, 0, type->path->expr, type->exts);
    }
    if (type->flags & LYS_SET_REQINST) {
        ypr_close_parent(ctx, &flag);
        ypr_substmt(ctx, LY_STMT_REQUIRE_INSTANCE, 0, type->require_instance ? "true" : "false", type->exts);
    }
    if (type->flags & LYS_SET_FRDIGITS) {
        ypr_close_parent(ctx, &flag);
        ypr_unsigned(ctx, LY_STMT_FRACTION_DIGITS, 0, type->exts, type->fraction_digits);
    }
    LY_ARRAY_FOR(type->bases, u) {
        ypr_close_parent(ctx, &flag);
        ypr_substmt(ctx, LY_STMT_BASE, u, type->bases[u], type->exts);
    }
    LY_ARRAY_FOR(type->types, u) {
        ypr_close_parent(ctx, &flag);
        yprp_type(ctx, &type->types[u]);
    }

    LEVEL--;
    ypr_close(ctx, "type", flag);
}

static void
yprp_typedef(struct lys_ypr_ctx *ctx, const struct lysp_tpdf *tpdf)
{
    ypr_open(ctx, "typedef", "name", tpdf->name, 1);
    LEVEL++;

    yprp_extension_instances(ctx, LY_STMT_TYPEDEF, 0, tpdf->exts, NULL, 0);

    yprp_type(ctx, &tpdf->type);

    if (tpdf->units) {
        ypr_substmt(ctx, LY_STMT_UNITS, 0, tpdf->units, tpdf->exts);
    }
    if (tpdf->dflt.str) {
        ypr_substmt(ctx, LY_STMT_DEFAULT, 0, tpdf->dflt.str, tpdf->exts);
    }

    ypr_status(ctx, tpdf->flags, tpdf->exts, NULL);
    ypr_description(ctx, tpdf->dsc, tpdf->exts, NULL);
    ypr_reference(ctx, tpdf->ref, tpdf->exts, NULL);

    LEVEL--;
    ypr_close(ctx, "typedef", 1);
}

static void yprp_node(struct lys_ypr_ctx *ctx, const struct lysp_node *node);
static void yprp_action(struct lys_ypr_ctx *ctx, const struct lysp_node_action *action);
static void yprp_notification(struct lys_ypr_ctx *ctx, const struct lysp_node_notif *notif);

static void
yprp_grouping(struct lys_ypr_ctx *ctx, const struct lysp_node_grp *grp)
{
    LY_ARRAY_COUNT_TYPE u;
    int8_t flag = 0;
    struct lysp_node *data;
    struct lysp_node_action *action;
    struct lysp_node_notif *notif;
    struct lysp_node_grp *subgrp;

    ypr_open(ctx, "grouping", "name", grp->name, flag);
    LEVEL++;

    yprp_extension_instances(ctx, LY_STMT_GROUPING, 0, grp->exts, &flag, 0);
    ypr_status(ctx, grp->flags, grp->exts, &flag);
    ypr_description(ctx, grp->dsc, grp->exts, &flag);
    ypr_reference(ctx, grp->ref, grp->exts, &flag);

    LY_ARRAY_FOR(grp->typedefs, u) {
        ypr_close_parent(ctx, &flag);
        yprp_typedef(ctx, &grp->typedefs[u]);
    }

    LY_LIST_FOR(grp->groupings, subgrp) {
        ypr_close_parent(ctx, &flag);
        yprp_grouping(ctx, subgrp);
    }

    LY_LIST_FOR(grp->child, data) {
        ypr_close_parent(ctx, &flag);
        yprp_node(ctx, data);
    }

    LY_LIST_FOR(grp->actions, action) {
        ypr_close_parent(ctx, &flag);
        yprp_action(ctx, action);
    }

    LY_LIST_FOR(grp->notifs, notif) {
        ypr_close_parent(ctx, &flag);
        yprp_notification(ctx, notif);
    }

    LEVEL--;
    ypr_close(ctx, "grouping", flag);
}

static void
yprp_inout(struct lys_ypr_ctx *ctx, const struct lysp_node_action_inout *inout, int8_t *flag)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_node *data;
    struct lysp_node_grp *grp;

    if (!inout->child) {
        /* input/output is empty */
        return;
    }
    ypr_close_parent(ctx, flag);

    ypr_open(ctx, inout->name, NULL, NULL, *flag);
    LEVEL++;

    yprp_extension_instances(ctx, lys_nodetype2stmt(inout->nodetype), 0, inout->exts, NULL, 0);
    LY_ARRAY_FOR(inout->musts, u) {
        yprp_restr(ctx, &inout->musts[u], LY_STMT_MUST, "condition", NULL);
    }
    LY_ARRAY_FOR(inout->typedefs, u) {
        yprp_typedef(ctx, &inout->typedefs[u]);
    }
    LY_LIST_FOR(inout->groupings, grp) {
        yprp_grouping(ctx, grp);
    }

    LY_LIST_FOR(inout->child, data) {
        yprp_node(ctx, data);
    }

    LEVEL--;
    ypr_close(ctx, inout->name, 1);
}

static void
yprp_notification(struct lys_ypr_ctx *ctx, const struct lysp_node_notif *notif)
{
    LY_ARRAY_COUNT_TYPE u;
    int8_t flag = 0;
    struct lysp_node *data;
    struct lysp_node_grp *grp;

    ypr_open(ctx, "notification", "name", notif->name, flag);

    LEVEL++;
    yprp_extension_instances(ctx, LY_STMT_NOTIFICATION, 0, notif->exts, &flag, 0);
    yprp_iffeatures(ctx, notif->iffeatures, notif->exts, &flag);

    LY_ARRAY_FOR(notif->musts, u) {
        ypr_close_parent(ctx, &flag);
        yprp_restr(ctx, &notif->musts[u], LY_STMT_MUST, "condition", &flag);
    }
    ypr_status(ctx, notif->flags, notif->exts, &flag);
    ypr_description(ctx, notif->dsc, notif->exts, &flag);
    ypr_reference(ctx, notif->ref, notif->exts, &flag);

    LY_ARRAY_FOR(notif->typedefs, u) {
        ypr_close_parent(ctx, &flag);
        yprp_typedef(ctx, &notif->typedefs[u]);
    }

    LY_LIST_FOR(notif->groupings, grp) {
        ypr_close_parent(ctx, &flag);
        yprp_grouping(ctx, grp);
    }

    LY_LIST_FOR(notif->child, data) {
        ypr_close_parent(ctx, &flag);
        yprp_node(ctx, data);
    }

    LEVEL--;
    ypr_close(ctx, "notification", flag);
}

static void
yprp_action(struct lys_ypr_ctx *ctx, const struct lysp_node_action *action)
{
    LY_ARRAY_COUNT_TYPE u;
    int8_t flag = 0;
    struct lysp_node_grp *grp;

    ypr_open(ctx, action->parent ? "action" : "rpc", "name", action->name, flag);

    LEVEL++;
    yprp_extension_instances(ctx, lys_nodetype2stmt(action->nodetype), 0, action->exts, &flag, 0);
    yprp_iffeatures(ctx, action->iffeatures, action->exts, &flag);
    ypr_status(ctx, action->flags, action->exts, &flag);
    ypr_description(ctx, action->dsc, action->exts, &flag);
    ypr_reference(ctx, action->ref, action->exts, &flag);

    LY_ARRAY_FOR(action->typedefs, u) {
        ypr_close_parent(ctx, &flag);
        yprp_typedef(ctx, &action->typedefs[u]);
    }

    LY_LIST_FOR(action->groupings, grp) {
        ypr_close_parent(ctx, &flag);
        yprp_grouping(ctx, grp);
    }

    yprp_inout(ctx, &action->input, &flag);
    yprp_inout(ctx, &action->output, &flag);

    LEVEL--;
    ypr_close(ctx, action->parent ? "action" : "rpc", flag);
}

static void
yprp_node_common1(struct lys_ypr_ctx *ctx, const struct lysp_node *node, int8_t *flag)
{
    ypr_open(ctx, lys_nodetype2str(node->nodetype), "name", node->name, *flag);
    LEVEL++;

    yprp_extension_instances(ctx, lys_nodetype2stmt(node->nodetype), 0, node->exts, flag, 0);
    yprp_when(ctx, lysp_node_when(node), flag);
    yprp_iffeatures(ctx, node->iffeatures, node->exts, flag);
}

static void
yprp_node_common2(struct lys_ypr_ctx *ctx, const struct lysp_node *node, int8_t *flag)
{
    ypr_config(ctx, node->flags, node->exts, flag);
    if (node->nodetype & (LYS_CHOICE | LYS_LEAF | LYS_ANYDATA)) {
        ypr_mandatory(ctx, node->flags, node->exts, flag);
    }
    ypr_status(ctx, node->flags, node->exts, flag);
    ypr_description(ctx, node->dsc, node->exts, flag);
    ypr_reference(ctx, node->ref, node->exts, flag);
}

static void
yprp_container(struct lys_ypr_ctx *ctx, const struct lysp_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    int8_t flag = 0;
    struct lysp_node *child;
    struct lysp_node_action *action;
    struct lysp_node_notif *notif;
    struct lysp_node_grp *grp;
    struct lysp_node_container *cont = (struct lysp_node_container *)node;

    yprp_node_common1(ctx, node, &flag);

    LY_ARRAY_FOR(cont->musts, u) {
        ypr_close_parent(ctx, &flag);
        yprp_restr(ctx, &cont->musts[u], LY_STMT_MUST, "condition", &flag);
    }
    if (cont->presence) {
        ypr_close_parent(ctx, &flag);
        ypr_substmt(ctx, LY_STMT_PRESENCE, 0, cont->presence, cont->exts);
    }

    yprp_node_common2(ctx, node, &flag);

    LY_ARRAY_FOR(cont->typedefs, u) {
        ypr_close_parent(ctx, &flag);
        yprp_typedef(ctx, &cont->typedefs[u]);
    }

    LY_LIST_FOR(cont->groupings, grp) {
        ypr_close_parent(ctx, &flag);
        yprp_grouping(ctx, grp);
    }

    LY_LIST_FOR(cont->child, child) {
        ypr_close_parent(ctx, &flag);
        yprp_node(ctx, child);
    }

    LY_LIST_FOR(cont->actions, action) {
        ypr_close_parent(ctx, &flag);
        yprp_action(ctx, action);
    }

    LY_LIST_FOR(cont->notifs, notif) {
        ypr_close_parent(ctx, &flag);
        yprp_notification(ctx, notif);
    }

    LEVEL--;
    ypr_close(ctx, "container", flag);
}

static void
yprp_case(struct lys_ypr_ctx *ctx, const struct lysp_node *node)
{
    int8_t flag = 0;
    struct lysp_node *child;
    struct lysp_node_case *cas = (struct lysp_node_case *)node;

    yprp_node_common1(ctx, node, &flag);
    yprp_node_common2(ctx, node, &flag);

    LY_LIST_FOR(cas->child, child) {
        ypr_close_parent(ctx, &flag);
        yprp_node(ctx, child);
    }

    LEVEL--;
    ypr_close(ctx, "case", flag);
}

static void
yprp_choice(struct lys_ypr_ctx *ctx, const struct lysp_node *node)
{
    int8_t flag = 0;
    struct lysp_node *child;
    struct lysp_node_choice *choice = (struct lysp_node_choice *)node;

    yprp_node_common1(ctx, node, &flag);

    if (choice->dflt.str) {
        ypr_close_parent(ctx, &flag);
        ypr_substmt(ctx, LY_STMT_DEFAULT, 0, choice->dflt.str, choice->exts);
    }

    yprp_node_common2(ctx, node, &flag);

    LY_LIST_FOR(choice->child, child) {
        ypr_close_parent(ctx, &flag);
        yprp_node(ctx, child);
    }

    LEVEL--;
    ypr_close(ctx, "choice", flag);
}

static void
yprp_leaf(struct lys_ypr_ctx *ctx, const struct lysp_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_node_leaf *leaf = (struct lysp_node_leaf *)node;

    int8_t flag = 1;

    yprp_node_common1(ctx, node, &flag);

    yprp_type(ctx, &leaf->type);
    ypr_substmt(ctx, LY_STMT_UNITS, 0, leaf->units, leaf->exts);
    LY_ARRAY_FOR(leaf->musts, u) {
        yprp_restr(ctx, &leaf->musts[u], LY_STMT_MUST, "condition", &flag);
    }
    ypr_substmt(ctx, LY_STMT_DEFAULT, 0, leaf->dflt.str, leaf->exts);

    yprp_node_common2(ctx, node, &flag);

    LEVEL--;
    ypr_close(ctx, "leaf", flag);
}

static void
yprp_leaflist(struct lys_ypr_ctx *ctx, const struct lysp_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_node_leaflist *llist = (struct lysp_node_leaflist *)node;
    int8_t flag = 1;

    yprp_node_common1(ctx, node, &flag);

    yprp_type(ctx, &llist->type);
    ypr_substmt(ctx, LY_STMT_UNITS, 0, llist->units, llist->exts);
    LY_ARRAY_FOR(llist->musts, u) {
        yprp_restr(ctx, &llist->musts[u], LY_STMT_MUST, "condition", NULL);
    }
    LY_ARRAY_FOR(llist->dflts, u) {
        ypr_substmt(ctx, LY_STMT_DEFAULT, u, llist->dflts[u].str, llist->exts);
    }

    ypr_config(ctx, node->flags, node->exts, NULL);

    if (llist->flags & LYS_SET_MIN) {
        ypr_unsigned(ctx, LY_STMT_MIN_ELEMENTS, 0, llist->exts, llist->min);
    }
    if (llist->flags & LYS_SET_MAX) {
        if (llist->max) {
            ypr_unsigned(ctx, LY_STMT_MAX_ELEMENTS, 0, llist->exts, llist->max);
        } else {
            ypr_substmt(ctx, LY_STMT_MAX_ELEMENTS, 0, "unbounded", llist->exts);
        }
    }

    if (llist->flags & LYS_ORDBY_MASK) {
        ypr_substmt(ctx, LY_STMT_ORDERED_BY, 0, (llist->flags & LYS_ORDBY_USER) ? "user" : "system", llist->exts);
    }

    ypr_status(ctx, node->flags, node->exts, &flag);
    ypr_description(ctx, node->dsc, node->exts, &flag);
    ypr_reference(ctx, node->ref, node->exts, &flag);

    LEVEL--;
    ypr_close(ctx, "leaf-list", flag);
}

static void
yprp_list(struct lys_ypr_ctx *ctx, const struct lysp_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    int8_t flag = 0;
    struct lysp_node *child;
    struct lysp_node_action *action;
    struct lysp_node_notif *notif;
    struct lysp_node_grp *grp;
    struct lysp_node_list *list = (struct lysp_node_list *)node;

    yprp_node_common1(ctx, node, &flag);

    LY_ARRAY_FOR(list->musts, u) {
        ypr_close_parent(ctx, &flag);
        yprp_restr(ctx, &list->musts[u], LY_STMT_MUST, "condition", &flag);
    }
    if (list->key) {
        ypr_close_parent(ctx, &flag);
        ypr_substmt(ctx, LY_STMT_KEY, 0, list->key, list->exts);
    }
    LY_ARRAY_FOR(list->uniques, u) {
        ypr_close_parent(ctx, &flag);
        ypr_substmt(ctx, LY_STMT_UNIQUE, u, list->uniques[u].str, list->exts);
    }

    ypr_config(ctx, node->flags, node->exts, NULL);

    if (list->flags & LYS_SET_MIN) {
        ypr_unsigned(ctx, LY_STMT_MIN_ELEMENTS, 0, list->exts, list->min);
    }
    if (list->flags & LYS_SET_MAX) {
        if (list->max) {
            ypr_unsigned(ctx, LY_STMT_MAX_ELEMENTS, 0, list->exts, list->max);
        } else {
            ypr_substmt(ctx, LY_STMT_MAX_ELEMENTS, 0, "unbounded", list->exts);
        }
    }

    if (list->flags & LYS_ORDBY_MASK) {
        ypr_close_parent(ctx, &flag);
        ypr_substmt(ctx, LY_STMT_ORDERED_BY, 0, (list->flags & LYS_ORDBY_USER) ? "user" : "system", list->exts);
    }

    ypr_status(ctx, node->flags, node->exts, &flag);
    ypr_description(ctx, node->dsc, node->exts, &flag);
    ypr_reference(ctx, node->ref, node->exts, &flag);

    LY_ARRAY_FOR(list->typedefs, u) {
        ypr_close_parent(ctx, &flag);
        yprp_typedef(ctx, &list->typedefs[u]);
    }

    LY_LIST_FOR(list->groupings, grp) {
        ypr_close_parent(ctx, &flag);
        yprp_grouping(ctx, grp);
    }

    LY_LIST_FOR(list->child, child) {
        ypr_close_parent(ctx, &flag);
        yprp_node(ctx, child);
    }

    LY_LIST_FOR(list->actions, action) {
        ypr_close_parent(ctx, &flag);
        yprp_action(ctx, action);
    }

    LY_LIST_FOR(list->notifs, notif) {
        ypr_close_parent(ctx, &flag);
        yprp_notification(ctx, notif);
    }

    LEVEL--;
    ypr_close(ctx, "list", flag);
}

static void
yprp_refine(struct lys_ypr_ctx *ctx, struct lysp_refine *refine)
{
    LY_ARRAY_COUNT_TYPE u;
    int8_t flag = 0;

    ypr_open(ctx, "refine", "target-node", refine->nodeid, flag);
    LEVEL++;

    yprp_extension_instances(ctx, LY_STMT_REFINE, 0, refine->exts, &flag, 0);
    yprp_iffeatures(ctx, refine->iffeatures, refine->exts, &flag);

    LY_ARRAY_FOR(refine->musts, u) {
        ypr_close_parent(ctx, &flag);
        yprp_restr(ctx, &refine->musts[u], LY_STMT_MUST, "condition", &flag);
    }

    if (refine->presence) {
        ypr_close_parent(ctx, &flag);
        ypr_substmt(ctx, LY_STMT_PRESENCE, 0, refine->presence, refine->exts);
    }

    LY_ARRAY_FOR(refine->dflts, u) {
        ypr_close_parent(ctx, &flag);
        ypr_substmt(ctx, LY_STMT_DEFAULT, u, refine->dflts[u].str, refine->exts);
    }

    ypr_config(ctx, refine->flags, refine->exts, &flag);
    ypr_mandatory(ctx, refine->flags, refine->exts, &flag);

    if (refine->flags & LYS_SET_MIN) {
        ypr_close_parent(ctx, &flag);
        ypr_unsigned(ctx, LY_STMT_MIN_ELEMENTS, 0, refine->exts, refine->min);
    }
    if (refine->flags & LYS_SET_MAX) {
        ypr_close_parent(ctx, &flag);
        if (refine->max) {
            ypr_unsigned(ctx, LY_STMT_MAX_ELEMENTS, 0, refine->exts, refine->max);
        } else {
            ypr_substmt(ctx, LY_STMT_MAX_ELEMENTS, 0, "unbounded", refine->exts);
        }
    }

    ypr_description(ctx, refine->dsc, refine->exts, &flag);
    ypr_reference(ctx, refine->ref, refine->exts, &flag);

    LEVEL--;
    ypr_close(ctx, "refine", flag);
}

static void
yprp_augment(struct lys_ypr_ctx *ctx, const struct lysp_node_augment *aug)
{
    struct lysp_node *child;
    struct lysp_node_action *action;
    struct lysp_node_notif *notif;

    ypr_open(ctx, "augment", "target-node", aug->nodeid, 1);
    LEVEL++;

    yprp_extension_instances(ctx, LY_STMT_AUGMENT, 0, aug->exts, NULL, 0);
    yprp_when(ctx, aug->when, NULL);
    yprp_iffeatures(ctx, aug->iffeatures, aug->exts, NULL);
    ypr_status(ctx, aug->flags, aug->exts, NULL);
    ypr_description(ctx, aug->dsc, aug->exts, NULL);
    ypr_reference(ctx, aug->ref, aug->exts, NULL);

    LY_LIST_FOR(aug->child, child) {
        yprp_node(ctx, child);
    }

    LY_LIST_FOR(aug->actions, action) {
        yprp_action(ctx, action);
    }

    LY_LIST_FOR(aug->notifs, notif) {
        yprp_notification(ctx, notif);
    }

    LEVEL--;
    ypr_close(ctx, "augment", 1);
}

static void
yprp_uses(struct lys_ypr_ctx *ctx, const struct lysp_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    int8_t flag = 0;
    struct lysp_node_uses *uses = (struct lysp_node_uses *)node;
    struct lysp_node_augment *aug;

    yprp_node_common1(ctx, node, &flag);
    yprp_node_common2(ctx, node, &flag);

    LY_ARRAY_FOR(uses->refines, u) {
        ypr_close_parent(ctx, &flag);
        yprp_refine(ctx, &uses->refines[u]);
    }

    LY_LIST_FOR(uses->augments, aug) {
        ypr_close_parent(ctx, &flag);
        yprp_augment(ctx, aug);
    }

    LEVEL--;
    ypr_close(ctx, "uses", flag);
}

static void
yprp_anydata(struct lys_ypr_ctx *ctx, const struct lysp_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    int8_t flag = 0;
    struct lysp_node_anydata *any = (struct lysp_node_anydata *)node;

    yprp_node_common1(ctx, node, &flag);

    LY_ARRAY_FOR(any->musts, u) {
        ypr_close_parent(ctx, &flag);
        yprp_restr(ctx, &any->musts[u], LY_STMT_MUST, "condition", &flag);
    }

    yprp_node_common2(ctx, node, &flag);

    LEVEL--;
    ypr_close(ctx, lys_nodetype2str(node->nodetype), flag);
}

static void
yprp_node(struct lys_ypr_ctx *ctx, const struct lysp_node *node)
{
    switch (node->nodetype) {
    case LYS_CONTAINER:
        yprp_container(ctx, node);
        break;
    case LYS_CHOICE:
        yprp_choice(ctx, node);
        break;
    case LYS_LEAF:
        yprp_leaf(ctx, node);
        break;
    case LYS_LEAFLIST:
        yprp_leaflist(ctx, node);
        break;
    case LYS_LIST:
        yprp_list(ctx, node);
        break;
    case LYS_USES:
        yprp_uses(ctx, node);
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        yprp_anydata(ctx, node);
        break;
    case LYS_CASE:
        yprp_case(ctx, node);
        break;
    default:
        break;
    }
}

static void
yprp_deviation(struct lys_ypr_ctx *ctx, const struct lysp_deviation *deviation)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_deviate_add *add;
    struct lysp_deviate_rpl *rpl;
    struct lysp_deviate_del *del;
    struct lysp_deviate *elem;

    ypr_open(ctx, "deviation", "target-node", deviation->nodeid, 1);
    LEVEL++;

    yprp_extension_instances(ctx, LY_STMT_DEVIATION, 0, deviation->exts, NULL, 0);
    ypr_description(ctx, deviation->dsc, deviation->exts, NULL);
    ypr_reference(ctx, deviation->ref, deviation->exts, NULL);

    LY_LIST_FOR(deviation->deviates, elem) {
        ly_print_(ctx->out, "%*s<deviate value=\"", INDENT);
        if (elem->mod == LYS_DEV_NOT_SUPPORTED) {
            if (elem->exts) {
                ly_print_(ctx->out, "not-supported\"/>\n");
                LEVEL++;

                yprp_extension_instances(ctx, LY_STMT_DEVIATE, 0, elem->exts, NULL, 0);
            } else {
                ly_print_(ctx->out, "not-supported\"/>\n");
                continue;
            }
        } else if (elem->mod == LYS_DEV_ADD) {
            add = (struct lysp_deviate_add *)elem;
            ly_print_(ctx->out, "add\">\n");
            LEVEL++;

            yprp_extension_instances(ctx, LY_STMT_DEVIATE, 0, add->exts, NULL, 0);
            ypr_substmt(ctx, LY_STMT_UNITS, 0, add->units, add->exts);
            LY_ARRAY_FOR(add->musts, u) {
                yprp_restr(ctx, &add->musts[u], LY_STMT_MUST, "condition", NULL);
            }
            LY_ARRAY_FOR(add->uniques, u) {
                ypr_substmt(ctx, LY_STMT_UNIQUE, u, add->uniques[u].str, add->exts);
            }
            LY_ARRAY_FOR(add->dflts, u) {
                ypr_substmt(ctx, LY_STMT_DEFAULT, u, add->dflts[u].str, add->exts);
            }
            ypr_config(ctx, add->flags, add->exts, NULL);
            ypr_mandatory(ctx, add->flags, add->exts, NULL);
            if (add->flags & LYS_SET_MIN) {
                ypr_unsigned(ctx, LY_STMT_MIN_ELEMENTS, 0, add->exts, add->min);
            }
            if (add->flags & LYS_SET_MAX) {
                if (add->max) {
                    ypr_unsigned(ctx, LY_STMT_MAX_ELEMENTS, 0, add->exts, add->max);
                } else {
                    ypr_substmt(ctx, LY_STMT_MAX_ELEMENTS, 0, "unbounded", add->exts);
                }
            }
        } else if (elem->mod == LYS_DEV_REPLACE) {
            rpl = (struct lysp_deviate_rpl *)elem;
            ly_print_(ctx->out, "replace\">\n");
            LEVEL++;

            yprp_extension_instances(ctx, LY_STMT_DEVIATE, 0, rpl->exts, NULL, 0);
            if (rpl->type) {
                yprp_type(ctx, rpl->type);
            }
            ypr_substmt(ctx, LY_STMT_UNITS, 0, rpl->units, rpl->exts);
            ypr_substmt(ctx, LY_STMT_DEFAULT, 0, rpl->dflt.str, rpl->exts);
            ypr_config(ctx, rpl->flags, rpl->exts, NULL);
            ypr_mandatory(ctx, rpl->flags, rpl->exts, NULL);
            if (rpl->flags & LYS_SET_MIN) {
                ypr_unsigned(ctx, LY_STMT_MIN_ELEMENTS, 0, rpl->exts, rpl->min);
            }
            if (rpl->flags & LYS_SET_MAX) {
                if (rpl->max) {
                    ypr_unsigned(ctx, LY_STMT_MAX_ELEMENTS, 0, rpl->exts, rpl->max);
                } else {
                    ypr_substmt(ctx, LY_STMT_MAX_ELEMENTS, 0, "unbounded", rpl->exts);
                }
            }
        } else if (elem->mod == LYS_DEV_DELETE) {
            del = (struct lysp_deviate_del *)elem;
            ly_print_(ctx->out, "delete\">\n");
            LEVEL++;

            yprp_extension_instances(ctx, LY_STMT_DEVIATE, 0, del->exts, NULL, 0);
            ypr_substmt(ctx, LY_STMT_UNITS, 0, del->units, del->exts);
            LY_ARRAY_FOR(del->musts, u) {
                yprp_restr(ctx, &del->musts[u], LY_STMT_MUST, "condition", NULL);
            }
            LY_ARRAY_FOR(del->uniques, u) {
                ypr_substmt(ctx, LY_STMT_UNIQUE, u, del->uniques[u].str, del->exts);
            }
            LY_ARRAY_FOR(del->dflts, u) {
                ypr_substmt(ctx, LY_STMT_DEFAULT, u, del->dflts[u].str, del->exts);
            }
        }

        LEVEL--;
        ypr_close(ctx, "deviate", 1);
    }

    LEVEL--;
    ypr_close(ctx, "deviation", 1);
}

static void
ypr_xmlns(struct lys_ypr_ctx *ctx, const struct lys_module *module, uint16_t indent)
{
    ly_print_(ctx->out, "%*sxmlns=\"%s\"", indent + INDENT, YIN_NS_URI);
    ly_print_(ctx->out, "\n%*sxmlns:%s=\"%s\"", indent + INDENT, module->prefix, module->ns);
}

static void
ypr_import_xmlns(struct lys_ypr_ctx *ctx, const struct lysp_module *modp, uint16_t indent)
{
    LY_ARRAY_COUNT_TYPE u;

    LY_ARRAY_FOR(modp->imports, u){
        ly_print_(ctx->out, "\n%*sxmlns:%s=\"%s\"", indent + INDENT, modp->imports[u].prefix, modp->imports[u].module->ns);
    }
}

static void
yprp_stmt(struct lys_ypr_ctx *ctx, struct lysp_stmt *stmt)
{
    struct lysp_stmt *childstmt;
    int8_t flag = stmt->child ? 1 : -1;

    /* TODO:
             the extension instance substatements in extension instances (LY_STMT_EXTENSION_INSTANCE)
             cannot find the compiled information, so it is needed to be done,
             currently it is ignored */
    if (stmt_attr_info[stmt->kw].name) {
        if (stmt_attr_info[stmt->kw].flags & STMT_FLAG_YIN) {
            ypr_open(ctx, stmt->stmt, NULL, NULL, flag);
            ypr_yin_arg(ctx, stmt_attr_info[stmt->kw].arg, stmt->arg);
        } else {
            ypr_open(ctx, stmt->stmt, stmt_attr_info[stmt->kw].arg, stmt->arg, flag);
        }
    }

    if (stmt->child) {
        LEVEL++;
        LY_LIST_FOR(stmt->child, childstmt) {
            yprp_stmt(ctx, childstmt);
        }
        LEVEL--;
        ypr_close(ctx, stmt->stmt, flag);
    }
}

/**
 * @param[in] count Number of extensions to print, 0 to print them all.
 */
static void
yprp_extension_instances(struct lys_ypr_ctx *ctx, enum ly_stmt substmt, uint8_t substmt_index,
        struct lysp_ext_instance *ext, int8_t *flag, LY_ARRAY_COUNT_TYPE count)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_stmt *stmt;
    int8_t inner_flag = 0;

    if (!count && ext) {
        count = LY_ARRAY_COUNT(ext);
    }
    LY_ARRAY_FOR(ext, u) {
        struct lysp_ext *ext_def = NULL;

        if (!count) {
            break;
        }

        count--;
        if ((ext->flags & LYS_INTERNAL) || (ext->parent_stmt != substmt) || (ext->parent_stmt_index != substmt_index)) {
            continue;
        }

        lysp_ext_find_definition(ctx->module->ctx, &ext[u], NULL, &ext_def);
        if (!ext_def) {
            continue;
        }

        ypr_close_parent(ctx, flag);
        inner_flag = 0;

        if (ext_def->argname) {
            lysp_ext_instance_resolve_argument(ctx->module->ctx, &ext[u], ext_def);
        }

        ypr_open(ctx, ext[u].name, (ext_def->flags & LYS_YINELEM_TRUE) ? NULL : ext_def->argname, ext[u].argument, inner_flag);
        LEVEL++;
        if (ext_def->flags & LYS_YINELEM_TRUE) {
            const char *prefix, *name, *id;
            size_t prefix_len, name_len;

            ypr_close_parent(ctx, &inner_flag);

            /* we need to use the same namespace as for the extension instance element */
            id = ext[u].name;
            ly_parse_nodeid(&id, &prefix, &prefix_len, &name, &name_len);
            ly_print_(ctx->out, "%*s<%.*s:%s>", INDENT, (int)prefix_len, prefix, ext_def->argname);
            lyxml_dump_text(ctx->out, ext[u].argument, 0);
            ly_print_(ctx->out, "</%.*s:%s>\n", (int)prefix_len, prefix, ext_def->argname);
        }
        LY_LIST_FOR(ext[u].child, stmt) {
            if (stmt->flags & (LYS_YIN_ATTR | LYS_YIN_ARGUMENT)) {
                continue;
            }

            ypr_close_parent(ctx, &inner_flag);
            yprp_stmt(ctx, stmt);
        }
        LEVEL--;
        ypr_close(ctx, ext[u].name, inner_flag);
    }
}

static void
yin_print_parsed_linkage(struct lys_ypr_ctx *ctx, const struct lysp_module *modp)
{
    LY_ARRAY_COUNT_TYPE u;

    LY_ARRAY_FOR(modp->imports, u) {
        if (modp->imports[u].flags & LYS_INTERNAL) {
            continue;
        }

        ypr_open(ctx, "import", "module", modp->imports[u].name, 1);
        LEVEL++;
        yprp_extension_instances(ctx, LY_STMT_IMPORT, 0, modp->imports[u].exts, NULL, 0);
        ypr_substmt(ctx, LY_STMT_PREFIX, 0, modp->imports[u].prefix, modp->imports[u].exts);
        if (modp->imports[u].rev[0]) {
            ypr_substmt(ctx, LY_STMT_REVISION_DATE, 0, modp->imports[u].rev, modp->imports[u].exts);
        }
        ypr_substmt(ctx, LY_STMT_DESCRIPTION, 0, modp->imports[u].dsc, modp->imports[u].exts);
        ypr_substmt(ctx, LY_STMT_REFERENCE, 0, modp->imports[u].ref, modp->imports[u].exts);
        LEVEL--;
        ypr_close(ctx, "import", 1);
    }
    LY_ARRAY_FOR(modp->includes, u) {
        if (modp->includes[u].injected) {
            /* do not print the includes injected from submodules */
            continue;
        }
        if (modp->includes[u].rev[0] || modp->includes[u].dsc || modp->includes[u].ref || modp->includes[u].exts) {
            ypr_open(ctx, "include", "module", modp->includes[u].name, 1);
            LEVEL++;
            yprp_extension_instances(ctx, LY_STMT_INCLUDE, 0, modp->includes[u].exts, NULL, 0);
            if (modp->includes[u].rev[0]) {
                ypr_substmt(ctx, LY_STMT_REVISION_DATE, 0, modp->includes[u].rev, modp->includes[u].exts);
            }
            ypr_substmt(ctx, LY_STMT_DESCRIPTION, 0, modp->includes[u].dsc, modp->includes[u].exts);
            ypr_substmt(ctx, LY_STMT_REFERENCE, 0, modp->includes[u].ref, modp->includes[u].exts);
            LEVEL--;
            ly_print_(ctx->out, "%*s}\n", INDENT);
        } else {
            ypr_open(ctx, "include", "module", modp->includes[u].name, -1);
        }
    }
}

static void
yin_print_parsed_body(struct lys_ypr_ctx *ctx, const struct lysp_module *modp)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_node *data;
    struct lysp_node_action *action;
    struct lysp_node_notif *notif;
    struct lysp_node_grp *grp;
    struct lysp_node_augment *aug;

    LY_ARRAY_FOR(modp->extensions, u) {
        ly_print_(ctx->out, "\n");
        yprp_extension(ctx, &modp->extensions[u]);
    }
    if (modp->exts) {
        ly_print_(ctx->out, "\n");
        yprp_extension_instances(ctx, LY_STMT_MODULE, 0, modp->exts, NULL, 0);
    }

    LY_ARRAY_FOR(modp->features, u) {
        yprp_feature(ctx, &modp->features[u]);
    }

    LY_ARRAY_FOR(modp->identities, u) {
        yprp_identity(ctx, &modp->identities[u]);
    }

    LY_ARRAY_FOR(modp->typedefs, u) {
        yprp_typedef(ctx, &modp->typedefs[u]);
    }

    LY_LIST_FOR(modp->groupings, grp) {
        yprp_grouping(ctx, grp);
    }

    LY_LIST_FOR(modp->data, data) {
        yprp_node(ctx, data);
    }

    LY_LIST_FOR(modp->augments, aug) {
        yprp_augment(ctx, aug);
    }

    LY_LIST_FOR(modp->rpcs, action) {
        yprp_action(ctx, action);
    }

    LY_LIST_FOR(modp->notifs, notif) {
        yprp_notification(ctx, notif);
    }

    LY_ARRAY_FOR(modp->deviations, u) {
        yprp_deviation(ctx, &modp->deviations[u]);
    }
}

LY_ERR
yin_print_parsed_module(struct ly_out *out, const struct lysp_module *modp, uint32_t options)
{
    LY_ARRAY_COUNT_TYPE u;
    const struct lys_module *module = modp->mod;
    struct lys_ypr_ctx ctx_ = {.out = out, .level = 0, .module = module, .options = options}, *ctx = &ctx_;

    ly_print_(ctx->out, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    ly_print_(ctx->out, "%*s<module name=\"%s\"\n", INDENT, module->name);
    ypr_xmlns(ctx, module, XML_NS_INDENT);
    ypr_import_xmlns(ctx, modp, XML_NS_INDENT);
    ly_print_(ctx->out, ">\n");

    LEVEL++;

    /* module-header-stmts */
    if (modp->version) {
        ypr_substmt(ctx, LY_STMT_YANG_VERSION, 0, modp->version == LYS_VERSION_1_1 ? "1.1" : "1", modp->exts);
    }
    ypr_substmt(ctx, LY_STMT_NAMESPACE, 0, module->ns, modp->exts);
    ypr_substmt(ctx, LY_STMT_PREFIX, 0, module->prefix, modp->exts);

    /* linkage-stmts (import/include) */
    yin_print_parsed_linkage(ctx, modp);

    /* meta-stmts */
    if (module->org || module->contact || module->dsc || module->ref) {
        ly_print_(out, "\n");
    }
    ypr_substmt(ctx, LY_STMT_ORGANIZATION, 0, module->org, modp->exts);
    ypr_substmt(ctx, LY_STMT_CONTACT, 0, module->contact, modp->exts);
    ypr_substmt(ctx, LY_STMT_DESCRIPTION, 0, module->dsc, modp->exts);
    ypr_substmt(ctx, LY_STMT_REFERENCE, 0, module->ref, modp->exts);

    /* revision-stmts */
    if (modp->revs) {
        ly_print_(out, "\n");
    }
    LY_ARRAY_FOR(modp->revs, u) {
        yprp_revision(ctx, &modp->revs[u]);
    }

    /* body-stmts */
    yin_print_parsed_body(ctx, modp);

    LEVEL--;
    ly_print_(out, "%*s</module>\n", INDENT);
    ly_print_flush(out);

    return LY_SUCCESS;
}

static void
yprp_belongsto(struct lys_ypr_ctx *ctx, const struct lysp_submodule *submodp)
{
    ypr_open(ctx, "belongs-to", "module", submodp->mod->name, 1);
    LEVEL++;
    yprp_extension_instances(ctx, LY_STMT_BELONGS_TO, 0, submodp->exts, NULL, 0);
    ypr_substmt(ctx, LY_STMT_PREFIX, 0, submodp->prefix, submodp->exts);
    LEVEL--;
    ypr_close(ctx, "belongs-to", 1);
}

LY_ERR
yin_print_parsed_submodule(struct ly_out *out, const struct lysp_submodule *submodp, uint32_t options)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lys_ypr_ctx ctx_ = {.out = out, .level = 0, .module = submodp->mod, .options = options}, *ctx = &ctx_;

    ly_print_(ctx->out, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    ly_print_(ctx->out, "%*s<submodule name=\"%s\"\n", INDENT, submodp->name);
    ypr_xmlns(ctx, submodp->mod, XML_NS_INDENT);
    ypr_import_xmlns(ctx, (struct lysp_module *)submodp, XML_NS_INDENT);
    ly_print_(ctx->out, ">\n");

    LEVEL++;

    /* submodule-header-stmts */
    if (submodp->version) {
        ypr_substmt(ctx, LY_STMT_YANG_VERSION, 0, submodp->version == LYS_VERSION_1_1 ? "1.1" : "1", submodp->exts);
    }
    yprp_belongsto(ctx, submodp);

    /* linkage-stmts (import/include) */
    yin_print_parsed_linkage(ctx, (struct lysp_module *)submodp);

    /* meta-stmts */
    if (submodp->org || submodp->contact || submodp->dsc || submodp->ref) {
        ly_print_(out, "\n");
    }
    ypr_substmt(ctx, LY_STMT_ORGANIZATION, 0, submodp->org, submodp->exts);
    ypr_substmt(ctx, LY_STMT_CONTACT, 0, submodp->contact, submodp->exts);
    ypr_substmt(ctx, LY_STMT_DESCRIPTION, 0, submodp->dsc, submodp->exts);
    ypr_substmt(ctx, LY_STMT_REFERENCE, 0, submodp->ref, submodp->exts);

    /* revision-stmts */
    if (submodp->revs) {
        ly_print_(out, "\n");
    }
    LY_ARRAY_FOR(submodp->revs, u) {
        yprp_revision(ctx, &submodp->revs[u]);
    }

    /* body-stmts */
    yin_print_parsed_body(ctx, (struct lysp_module *)submodp);

    LEVEL--;
    ly_print_(out, "%*s</submodule>\n", INDENT);
    ly_print_flush(out);

    return LY_SUCCESS;
}
