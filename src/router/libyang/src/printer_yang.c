/**
 * @file printer_yang.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief YANG printer
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

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "compat.h"
#include "log.h"
#include "ly_common.h"
#include "out.h"
#include "out_internal.h"
#include "plugins_exts.h"
#include "plugins_types.h"
#include "printer_internal.h"
#include "printer_schema.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "xpath.h"

/**
 * @brief Types of the YANG printers
 */
enum lys_ypr_schema_type {
    LYS_YPR_PARSED,   /**< YANG printer of the parsed schema */
    LYS_YPR_COMPILED  /**< YANG printer of the compiled schema */
};

enum lys_ypr_text_flags {
    LYS_YPR_TEXT_SINGLELINE = 0x01,     /**< print 'text' on the same line as 'name' */
    LYS_YPR_TEXT_SINGLEQUOTED = 0x02    /**< do not encode 'text' and print it in single quotes */
};

#define YPR_CTX_FLAG_EXTRA_LINE 0x01 /**< Flag for ::ypr_ctx::flags to print extra line in schema */

#define YPR_EXTRA_LINE(COND, PCTX) if (COND) { (PCTX)->flags |= YPR_CTX_FLAG_EXTRA_LINE; }
#define YPR_EXTRA_LINE_PRINT(PCTX) \
    if ((PCTX)->flags & YPR_CTX_FLAG_EXTRA_LINE) { \
        (PCTX)->flags &= ~YPR_CTX_FLAG_EXTRA_LINE; \
        if (DO_FORMAT) { \
            ly_print_((PCTX)->out, "\n"); \
        } \
    }

#define YPR_IS_LYS_SINGLEQUOTED(FLAGS) (((FLAGS) & LYS_SINGLEQUOTED) ? 1 : 0)

/**
 * @brief Compiled YANG printer context
 *
 * Note that the YANG extensions API provides getter to the members for the extension plugins.
 */
struct lys_ypr_ctx {
    union {
        struct {
            struct ly_out *out;              /**< output specification */
            uint16_t level;                  /**< current indentation level: 0 - no formatting, >= 1 indentation levels */
            uint16_t flags;                  /**< internal flags for use by printer */
            uint32_t options;                /**< Schema output options (see @ref schemaprinterflags). */
            const struct lys_module *module; /**< schema to print */
        };
        struct lyspr_ctx printer_ctx;
    };

    /* YANG printer specific members */
    enum lys_ypr_schema_type schema; /**< type of the schema to print */
};

/**
 * @brief Print the given text as content of a double quoted YANG string,
 * including encoding characters that have special meanings. The quotation marks
 * are not printed.
 *
 * Follows RFC 7950, section 6.1.3.
 *
 * @param[in] out Output specification.
 * @param[in] text String to be printed.
 * @param[in] len Length of the string from @p text to be printed. In case of -1,
 * the @p text is printed completely as a NULL-terminated string.
 */
static void
ypr_encode(struct ly_out *out, const char *text, ssize_t len)
{
    size_t i, start_len;
    const char *start;
    char special = 0;

    if (!len) {
        return;
    }

    if (len < 0) {
        len = strlen(text);
    }

    start = text;
    start_len = 0;
    for (i = 0; i < (size_t)len; ++i) {
        switch (text[i]) {
        case '\n':
        case '\t':
        case '\"':
        case '\\':
            special = text[i];
            break;
        default:
            ++start_len;
            break;
        }

        if (special) {
            ly_write_(out, start, start_len);
            switch (special) {
            case '\n':
                ly_write_(out, "\\n", 2);
                break;
            case '\t':
                ly_write_(out, "\\t", 2);
                break;
            case '\"':
                ly_write_(out, "\\\"", 2);
                break;
            case '\\':
                ly_write_(out, "\\\\", 2);
                break;
            }

            start += start_len + 1;
            start_len = 0;

            special = 0;
        }
    }

    ly_write_(out, start, start_len);
}

static void
ypr_open(struct ly_out *out, ly_bool *flag)
{
    if (flag && !*flag) {
        *flag = 1;
        ly_print_(out, " {\n");
    }
}

static void
ypr_close(struct lys_ypr_ctx *pctx, ly_bool flag)
{
    if (flag) {
        ly_print_(pctx->out, "%*s}\n", INDENT);
    } else {
        ly_print_(pctx->out, ";\n");
    }
}

static void
ypr_text(struct lys_ypr_ctx *pctx, const char *name, const char *text, enum lys_ypr_text_flags flags)
{
    const char *s, *t;
    char quot;

    if (flags & LYS_YPR_TEXT_SINGLEQUOTED) {
        quot = '\'';
    } else {
        quot = '\"';
    }

    if (flags & LYS_YPR_TEXT_SINGLELINE) {
        ly_print_(pctx->out, "%*s%s %c", INDENT, name, quot);
    } else {
        ly_print_(pctx->out, "%*s%s\n", INDENT, name);
        LEVEL++;

        ly_print_(pctx->out, "%*s%c", INDENT, quot);
    }

    t = text;
    while ((s = strchr(t, '\n'))) {
        if (flags & LYS_YPR_TEXT_SINGLEQUOTED) {
            ly_print_(pctx->out, "%.*s", (int)(s - t), t);
        } else {
            ypr_encode(pctx->out, t, s - t);
        }
        ly_print_(pctx->out, "\n");

        t = s + 1;
        if (*t != '\n') {
            ly_print_(pctx->out, "%*s ", INDENT);
        }
    }

    if (flags & LYS_YPR_TEXT_SINGLEQUOTED) {
        ly_print_(pctx->out, "%s", t);
    } else {
        ypr_encode(pctx->out, t, strlen(t));
    }
    ly_print_(pctx->out, "%c", quot);
    if (!(flags & LYS_YPR_TEXT_SINGLELINE)) {
        LEVEL--;
    }
}

static void
yprp_stmt(struct lys_ypr_ctx *pctx, struct lysp_stmt *stmt)
{
    struct lysp_stmt *childstmt;
    uint16_t tflags = 0;

    if (stmt->arg) {
        if (stmt->flags) {
            if (stmt->flags & LYS_SINGLEQUOTED) {
                tflags |= LYS_YPR_TEXT_SINGLEQUOTED;
            }
            ypr_text(pctx, stmt->stmt, stmt->arg, tflags);
            ly_print_(pctx->out, "%s", stmt->child ? " {\n" : ";\n");
        } else {
            ly_print_(pctx->out, "%*s%s %s%s", INDENT, stmt->stmt, stmt->arg, stmt->child ? " {\n" : ";\n");
        }
    } else {
        ly_print_(pctx->out, "%*s%s%s", INDENT, stmt->stmt, stmt->child ? " {\n" : ";\n");
    }

    if (stmt->child) {
        LEVEL++;
        LY_LIST_FOR(stmt->child, childstmt) {
            yprp_stmt(pctx, childstmt);
        }
        LEVEL--;
        ly_print_(pctx->out, "%*s}\n", INDENT);
    }
}

static void
yprp_extension_instance(struct lys_ypr_ctx *pctx, enum ly_stmt substmt, uint8_t substmt_index,
        struct lysp_ext_instance *ext, ly_bool *flag)
{
    struct lysp_stmt *stmt;
    ly_bool child_presence;

    if ((ext->flags & LYS_INTERNAL) || (ext->parent_stmt != substmt) || (ext->parent_stmt_index != substmt_index)) {
        return;
    }

    ypr_open(pctx->out, flag);

    if (ext->def->argname) {
        ly_print_(pctx->out, "%*s%s \"", INDENT, ext->name);
        ypr_encode(pctx->out, ext->argument, -1);
        ly_print_(pctx->out, "\"");
    } else {
        ly_print_(pctx->out, "%*s%s", INDENT, ext->name);
    }

    child_presence = 0;
    LEVEL++;
    LY_LIST_FOR(ext->child, stmt) {
        if (stmt->flags & (LYS_YIN_ATTR | LYS_YIN_ARGUMENT)) {
            continue;
        }
        if (!child_presence) {
            ly_print_(pctx->out, " {\n");
            child_presence = 1;
        }
        yprp_stmt(pctx, stmt);
    }
    LEVEL--;
    if (child_presence) {
        ly_print_(pctx->out, "%*s}\n", INDENT);
    } else {
        ly_print_(pctx->out, ";\n");
    }
}

static void
yprp_extension_instances(struct lys_ypr_ctx *pctx, enum ly_stmt substmt, uint8_t substmt_index,
        struct lysp_ext_instance *exts, ly_bool *flag)
{
    LY_ARRAY_COUNT_TYPE u;

    LY_ARRAY_FOR(exts, u) {
        if (exts[u].flags & LYS_INTERNAL) {
            continue;
        }
        yprp_extension_instance(pctx, substmt, substmt_index, &exts[u], flag);
    }
}

static ly_bool
yprp_extension_has_printable_instances(struct lysp_ext_instance *exts)
{
    LY_ARRAY_COUNT_TYPE u;

    LY_ARRAY_FOR(exts, u) {
        if (!(exts[u].flags & LYS_INTERNAL)) {
            return 1;
        }
    }

    return 0;
}

static void
yprc_extension_instances(struct lys_ypr_ctx *pctx, enum ly_stmt substmt, uint8_t substmt_index,
        struct lysc_ext_instance *exts, ly_bool *flag)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool inner_flag;

    LY_ARRAY_FOR(exts, u) {
        if ((exts[u].parent_stmt != substmt) || (exts[u].parent_stmt_index != substmt_index)) {
            return;
        }

        ypr_open(pctx->out, flag);
        if (exts[u].argument) {
            ly_print_(pctx->out, "%*s%s:%s \"", INDENT, exts[u].def->module->name, exts[u].def->name);
            ypr_encode(pctx->out, exts[u].argument, -1);
            ly_print_(pctx->out, "\"");
        } else {
            ly_print_(pctx->out, "%*s%s:%s", INDENT, exts[u].def->module->name, exts[u].def->name);
        }

        LEVEL++;
        inner_flag = 0;
        yprc_extension_instances(pctx, LY_STMT_EXTENSION_INSTANCE, 0, exts[u].exts, &inner_flag);

        if (exts[u].def->plugin && exts[u].def->plugin->printer_info) {
            exts[u].def->plugin->printer_info(&pctx->printer_ctx, &exts[u], &inner_flag);
        }

        LEVEL--;
        ypr_close(pctx, inner_flag);
    }
}

static void
ypr_substmt(struct lys_ypr_ctx *pctx, enum ly_stmt substmt, uint8_t substmt_index, const char *text,
        ly_bool singlequoted, void *exts)
{
    ly_bool extflag = 0;
    uint16_t flags = 0;

    if (!text) {
        /* nothing to print */
        return;
    }

    if (lys_stmt_flags(substmt) & LY_STMT_FLAG_ID) {
        ly_print_(pctx->out, "%*s%s %s", INDENT, lys_stmt_str(substmt), text);
    } else {
        if (!(lys_stmt_flags(substmt) & LY_STMT_FLAG_YIN)) {
            flags |= LYS_YPR_TEXT_SINGLELINE;
        }
        if (singlequoted) {
            flags |= LYS_YPR_TEXT_SINGLEQUOTED;
        }
        ypr_text(pctx, lys_stmt_str(substmt), text, flags);
    }

    LEVEL++;
    if (pctx->schema == LYS_YPR_PARSED) {
        yprp_extension_instances(pctx, substmt, substmt_index, exts, &extflag);
    } else {
        yprc_extension_instances(pctx, substmt, substmt_index, exts, &extflag);
    }
    LEVEL--;
    ypr_close(pctx, extflag);
}

static void
ypr_unsigned(struct lys_ypr_ctx *pctx, enum ly_stmt substmt, uint8_t substmt_index, void *exts,
        unsigned long attr_value, ly_bool *flag)
{
    char *str;

    if (asprintf(&str, "%lu", attr_value) == -1) {
        LOGMEM(pctx->module->ctx);
        return;
    }
    ypr_open(pctx->out, flag);
    ypr_substmt(pctx, substmt, substmt_index, str, 0, exts);
    free(str);
}

static void
ypr_signed(struct lys_ypr_ctx *pctx, enum ly_stmt substmt, uint8_t substmt_index, void *exts, long attr_value,
        ly_bool *flag)
{
    char *str;

    if (asprintf(&str, "%ld", attr_value) == -1) {
        LOGMEM(pctx->module->ctx);
        return;
    }
    ypr_open(pctx->out, flag);
    ypr_substmt(pctx, substmt, substmt_index, str, 0, exts);
    free(str);
}

static void
yprp_revision(struct lys_ypr_ctx *pctx, const struct lysp_revision *rev)
{
    if (rev->dsc || rev->ref || rev->exts) {
        ly_print_(pctx->out, "%*srevision %s {\n", INDENT, rev->date);
        LEVEL++;
        yprp_extension_instances(pctx, LY_STMT_REVISION, 0, rev->exts, NULL);
        ypr_substmt(pctx, LY_STMT_DESCRIPTION, 0, rev->dsc, 0, rev->exts);
        ypr_substmt(pctx, LY_STMT_REFERENCE, 0, rev->ref, 0, rev->exts);
        LEVEL--;
        ly_print_(pctx->out, "%*s}\n", INDENT);
    } else {
        ly_print_(pctx->out, "%*srevision %s;\n", INDENT, rev->date);
    }
}

static void
ypr_mandatory(struct lys_ypr_ctx *pctx, uint16_t flags, void *exts, ly_bool *flag)
{
    if (flags & LYS_MAND_MASK) {
        ypr_open(pctx->out, flag);
        ypr_substmt(pctx, LY_STMT_MANDATORY, 0, (flags & LYS_MAND_TRUE) ? "true" : "false", 0, exts);
    }
}

static void
ypr_config(struct lys_ypr_ctx *pctx, uint16_t flags, void *exts, ly_bool *flag)
{
    if (flags & LYS_CONFIG_MASK) {
        ypr_open(pctx->out, flag);
        ypr_substmt(pctx, LY_STMT_CONFIG, 0, (flags & LYS_CONFIG_W) ? "true" : "false", 0, exts);
    }
}

static void
ypr_status(struct lys_ypr_ctx *pctx, uint16_t flags, void *exts, ly_bool *flag)
{
    const char *status = NULL;

    if (flags & LYS_STATUS_CURR) {
        ypr_open(pctx->out, flag);
        status = "current";
    } else if (flags & LYS_STATUS_DEPRC) {
        ypr_open(pctx->out, flag);
        status = "deprecated";
    } else if (flags & LYS_STATUS_OBSLT) {
        ypr_open(pctx->out, flag);
        status = "obsolete";
    }

    ypr_substmt(pctx, LY_STMT_STATUS, 0, status, 0, exts);
}

static void
ypr_description(struct lys_ypr_ctx *pctx, const char *dsc, void *exts, ly_bool *flag)
{
    if (dsc) {
        ypr_open(pctx->out, flag);
        ypr_substmt(pctx, LY_STMT_DESCRIPTION, 0, dsc, 0, exts);
    }
}

static void
ypr_reference(struct lys_ypr_ctx *pctx, const char *ref, void *exts, ly_bool *flag)
{
    if (ref) {
        ypr_open(pctx->out, flag);
        ypr_substmt(pctx, LY_STMT_REFERENCE, 0, ref, 0, exts);
    }
}

static void
yprp_iffeatures(struct lys_ypr_ctx *pctx, struct lysp_qname *iffs, struct lysp_ext_instance *exts, ly_bool *flag)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool extflag;
    uint16_t flags;

    LY_ARRAY_FOR(iffs, u) {
        ypr_open(pctx->out, flag);
        extflag = 0;

        flags = LYS_YPR_TEXT_SINGLELINE | ((iffs[u].flags & LYS_SINGLEQUOTED) ? LYS_YPR_TEXT_SINGLEQUOTED : 0);
        ypr_text(pctx, "if-feature", iffs[u].str, flags);

        /* extensions */
        LEVEL++;
        yprp_extension_instances(pctx, LY_STMT_IF_FEATURE, u, exts, &extflag);
        LEVEL--;
        ypr_close(pctx, extflag);
    }
}

static void
yprp_extension(struct lys_ypr_ctx *pctx, const struct lysp_ext *ext)
{
    ly_bool flag = 0, flag2 = 0;
    LY_ARRAY_COUNT_TYPE u;

    ly_print_(pctx->out, "%*sextension %s", INDENT, ext->name);
    LEVEL++;

    yprp_extension_instances(pctx, LY_STMT_EXTENSION, 0, ext->exts, &flag);

    if (ext->argname) {
        ypr_open(pctx->out, &flag);
        ly_print_(pctx->out, "%*sargument %s", INDENT, ext->argname);
        LEVEL++;
        if (ext->exts) {
            u = -1;
            while ((u = lysp_ext_instance_iter(ext->exts, u + 1, LY_STMT_ARGUMENT)) != LY_ARRAY_COUNT(ext->exts)) {
                yprp_extension_instance(pctx, LY_STMT_ARGUMENT, 0, &ext->exts[u], &flag2);
            }
        }
        if ((ext->flags & LYS_YINELEM_MASK) ||
                (ext->exts && (lysp_ext_instance_iter(ext->exts, 0, LY_STMT_YIN_ELEMENT) != LY_ARRAY_COUNT(ext->exts)))) {
            ypr_open(pctx->out, &flag2);
            ypr_substmt(pctx, LY_STMT_YIN_ELEMENT, 0, (ext->flags & LYS_YINELEM_TRUE) ? "true" : "false", 0, ext->exts);
        }
        LEVEL--;
        ypr_close(pctx, flag2);
    }

    ypr_status(pctx, ext->flags, ext->exts, &flag);
    ypr_description(pctx, ext->dsc, ext->exts, &flag);
    ypr_reference(pctx, ext->ref, ext->exts, &flag);

    LEVEL--;
    ypr_close(pctx, flag);
}

static void
yprp_feature(struct lys_ypr_ctx *pctx, const struct lysp_feature *feat)
{
    ly_bool flag = 0;

    ly_print_(pctx->out, "%*sfeature %s", INDENT, feat->name);
    LEVEL++;
    yprp_extension_instances(pctx, LY_STMT_FEATURE, 0, feat->exts, &flag);
    yprp_iffeatures(pctx, feat->iffeatures, feat->exts, &flag);
    ypr_status(pctx, feat->flags, feat->exts, &flag);
    ypr_description(pctx, feat->dsc, feat->exts, &flag);
    ypr_reference(pctx, feat->ref, feat->exts, &flag);
    LEVEL--;
    ypr_close(pctx, flag);
}

static void
yprp_identity(struct lys_ypr_ctx *pctx, const struct lysp_ident *ident)
{
    ly_bool flag = 0;
    LY_ARRAY_COUNT_TYPE u;

    ly_print_(pctx->out, "%*sidentity %s", INDENT, ident->name);
    LEVEL++;

    yprp_extension_instances(pctx, LY_STMT_IDENTITY, 0, ident->exts, &flag);
    yprp_iffeatures(pctx, ident->iffeatures, ident->exts, &flag);

    LY_ARRAY_FOR(ident->bases, u) {
        ypr_open(pctx->out, &flag);
        ypr_substmt(pctx, LY_STMT_BASE, u, ident->bases[u], 0, ident->exts);
    }

    ypr_status(pctx, ident->flags, ident->exts, &flag);
    ypr_description(pctx, ident->dsc, ident->exts, &flag);
    ypr_reference(pctx, ident->ref, ident->exts, &flag);

    LEVEL--;
    ypr_close(pctx, flag);
}

static void
yprc_identity(struct lys_ypr_ctx *pctx, const struct lysc_ident *ident)
{
    ly_bool flag = 0;
    LY_ARRAY_COUNT_TYPE u;

    ly_print_(pctx->out, "%*sidentity %s", INDENT, ident->name);
    LEVEL++;

    yprc_extension_instances(pctx, LY_STMT_IDENTITY, 0, ident->exts, &flag);

    ypr_open(pctx->out, &flag);
    if (lys_identity_iffeature_value(ident) == LY_ENOT) {
        ly_print_(pctx->out, "%*s/* identity \"%s\" is disabled by if-feature(s) */\n", INDENT, ident->name);
    }

    LY_ARRAY_FOR(ident->derived, u) {
        if (pctx->module != ident->derived[u]->module) {
            ly_print_(pctx->out, "%*sderived %s:%s;\n", INDENT, ident->derived[u]->module->prefix, ident->derived[u]->name);
        } else {
            ly_print_(pctx->out, "%*sderived %s;\n", INDENT, ident->derived[u]->name);
        }
    }

    ypr_status(pctx, ident->flags, ident->exts, &flag);
    ypr_description(pctx, ident->dsc, ident->exts, &flag);
    ypr_reference(pctx, ident->ref, ident->exts, &flag);

    LEVEL--;
    ypr_close(pctx, flag);
}

static void
yprp_restr(struct lys_ypr_ctx *pctx, const struct lysp_restr *restr, enum ly_stmt stmt, ly_bool *flag)
{
    ly_bool inner_flag = 0;
    const char *text;
    uint16_t flags = 0;

    if (!restr) {
        return;
    }

    ypr_open(pctx->out, flag);
    text = ((restr->arg.str[0] != LYSP_RESTR_PATTERN_NACK) && (restr->arg.str[0] != LYSP_RESTR_PATTERN_ACK)) ?
            restr->arg.str : restr->arg.str + 1;
    if (!strchr(text, '\n')) {
        flags |= LYS_YPR_TEXT_SINGLELINE;
    }
    if (restr->arg.flags & LYS_SINGLEQUOTED) {
        flags |= LYS_YPR_TEXT_SINGLEQUOTED;
    }
    ypr_text(pctx, lyplg_ext_stmt2str(stmt), text, flags);

    LEVEL++;
    yprp_extension_instances(pctx, stmt, 0, restr->exts, &inner_flag);
    if (restr->arg.str[0] == LYSP_RESTR_PATTERN_NACK) {
        /* special byte value in pattern's expression: 0x15 - invert-match, 0x06 - match */
        ypr_open(pctx->out, &inner_flag);
        ypr_substmt(pctx, LY_STMT_MODIFIER, 0, "invert-match", 0, restr->exts);
    }
    if (restr->emsg) {
        ypr_open(pctx->out, &inner_flag);
        ypr_substmt(pctx, LY_STMT_ERROR_MESSAGE, 0, restr->emsg, 0, restr->exts);
    }
    if (restr->eapptag) {
        ypr_open(pctx->out, &inner_flag);
        ypr_substmt(pctx, LY_STMT_ERROR_APP_TAG, 0, restr->eapptag, 0, restr->exts);
    }
    ypr_description(pctx, restr->dsc, restr->exts, &inner_flag);
    ypr_reference(pctx, restr->ref, restr->exts, &inner_flag);

    LEVEL--;
    ypr_close(pctx, inner_flag);
}

static void
yprc_must(struct lys_ypr_ctx *pctx, const struct lysc_must *must, ly_bool *flag)
{
    ly_bool inner_flag = 0;

    ypr_open(pctx->out, flag);
    ypr_text(pctx, "must", must->cond->expr, LYS_YPR_TEXT_SINGLELINE);

    LEVEL++;
    yprc_extension_instances(pctx, LY_STMT_MUST, 0, must->exts, &inner_flag);
    if (must->emsg) {
        ypr_open(pctx->out, &inner_flag);
        ypr_substmt(pctx, LY_STMT_ERROR_MESSAGE, 0, must->emsg, 0, must->exts);
    }
    if (must->eapptag) {
        ypr_open(pctx->out, &inner_flag);
        ypr_substmt(pctx, LY_STMT_ERROR_APP_TAG, 0, must->eapptag, 0, must->exts);
    }
    ypr_description(pctx, must->dsc, must->exts, &inner_flag);
    ypr_reference(pctx, must->ref, must->exts, &inner_flag);

    LEVEL--;
    ypr_close(pctx, inner_flag);
}

static void
yprc_range(struct lys_ypr_ctx *pctx, const struct lysc_range *range, LY_DATA_TYPE basetype, ly_bool *flag)
{
    ly_bool inner_flag = 0;
    LY_ARRAY_COUNT_TYPE u;

    if (!range) {
        return;
    }

    ypr_open(pctx->out, flag);
    ly_print_(pctx->out, "%*s%s \"", INDENT, (basetype == LY_TYPE_STRING || basetype == LY_TYPE_BINARY) ? "length" : "range");
    LY_ARRAY_FOR(range->parts, u) {
        if (u > 0) {
            ly_print_(pctx->out, " | ");
        }
        if (range->parts[u].max_64 == range->parts[u].min_64) {
            if (basetype <= LY_TYPE_STRING) { /* unsigned values */
                ly_print_(pctx->out, "%" PRIu64, range->parts[u].max_u64);
            } else { /* signed values */
                ly_print_(pctx->out, "%" PRId64, range->parts[u].max_64);
            }
        } else {
            if (basetype <= LY_TYPE_STRING) { /* unsigned values */
                ly_print_(pctx->out, "%" PRIu64 "..%" PRIu64, range->parts[u].min_u64, range->parts[u].max_u64);
            } else { /* signed values */
                ly_print_(pctx->out, "%" PRId64 "..%" PRId64, range->parts[u].min_64, range->parts[u].max_64);
            }
        }
    }
    ly_print_(pctx->out, "\"");

    LEVEL++;
    yprc_extension_instances(pctx, LY_STMT_RANGE, 0, range->exts, &inner_flag);
    if (range->emsg) {
        ypr_open(pctx->out, &inner_flag);
        ypr_substmt(pctx, LY_STMT_ERROR_MESSAGE, 0, range->emsg, 0, range->exts);
    }
    if (range->eapptag) {
        ypr_open(pctx->out, &inner_flag);
        ypr_substmt(pctx, LY_STMT_ERROR_APP_TAG, 0, range->eapptag, 0, range->exts);
    }
    ypr_description(pctx, range->dsc, range->exts, &inner_flag);
    ypr_reference(pctx, range->ref, range->exts, &inner_flag);

    LEVEL--;
    ypr_close(pctx, inner_flag);
}

static void
yprc_pattern(struct lys_ypr_ctx *pctx, const struct lysc_pattern *pattern, ly_bool *flag)
{
    ly_bool inner_flag = 0;

    ypr_open(pctx->out, flag);
    ly_print_(pctx->out, "%*spattern \"", INDENT);
    ypr_encode(pctx->out, pattern->expr, -1);
    ly_print_(pctx->out, "\"");

    LEVEL++;
    yprc_extension_instances(pctx, LY_STMT_PATTERN, 0, pattern->exts, &inner_flag);
    if (pattern->inverted) {
        /* special byte value in pattern's expression: 0x15 - invert-match, 0x06 - match */
        ypr_open(pctx->out, &inner_flag);
        ypr_substmt(pctx, LY_STMT_MODIFIER, 0, "invert-match", 0, pattern->exts);
    }
    if (pattern->emsg) {
        ypr_open(pctx->out, &inner_flag);
        ypr_substmt(pctx, LY_STMT_ERROR_MESSAGE, 0, pattern->emsg, 0, pattern->exts);
    }
    if (pattern->eapptag) {
        ypr_open(pctx->out, &inner_flag);
        ypr_substmt(pctx, LY_STMT_ERROR_APP_TAG, 0, pattern->eapptag, 0, pattern->exts);
    }
    ypr_description(pctx, pattern->dsc, pattern->exts, &inner_flag);
    ypr_reference(pctx, pattern->ref, pattern->exts, &inner_flag);

    LEVEL--;
    ypr_close(pctx, inner_flag);
}

static void
yprc_bits_enum(struct lys_ypr_ctx *pctx, const struct lysc_type_bitenum_item *items, LY_DATA_TYPE basetype, ly_bool *flag)
{
    LY_ARRAY_COUNT_TYPE u;
    const struct lysc_type_bitenum_item *item;
    ly_bool inner_flag;

    assert((basetype == LY_TYPE_BITS) || (basetype == LY_TYPE_ENUM));

    LY_ARRAY_FOR(items, u) {
        item = &items[u];
        inner_flag = 0;

        ypr_open(pctx->out, flag);
        ly_print_(pctx->out, "%*s%s \"", INDENT, basetype == LY_TYPE_BITS ? "bit" : "enum");
        ypr_encode(pctx->out, item->name, -1);
        ly_print_(pctx->out, "\"");
        LEVEL++;
        if (basetype == LY_TYPE_BITS) {
            yprc_extension_instances(pctx, LY_STMT_BIT, 0, item->exts, &inner_flag);
            ypr_unsigned(pctx, LY_STMT_POSITION, 0, item->exts, item->position, &inner_flag);
        } else { /* LY_TYPE_ENUM */
            yprc_extension_instances(pctx, LY_STMT_ENUM, 0, item->exts, &inner_flag);
            ypr_signed(pctx, LY_STMT_VALUE, 0, item->exts, item->value, &inner_flag);
        }
        ypr_status(pctx, item->flags, item->exts, &inner_flag);
        ypr_description(pctx, item->dsc, item->exts, &inner_flag);
        ypr_reference(pctx, item->ref, item->exts, &inner_flag);
        LEVEL--;
        ypr_close(pctx, inner_flag);
    }
}

static void
yprp_when(struct lys_ypr_ctx *pctx, const struct lysp_when *when, ly_bool *flag)
{
    ly_bool inner_flag = 0;

    if (!when) {
        return;
    }
    ypr_open(pctx->out, flag);

    ypr_text(pctx, "when", when->cond, LYS_YPR_TEXT_SINGLELINE);

    LEVEL++;
    yprp_extension_instances(pctx, LY_STMT_WHEN, 0, when->exts, &inner_flag);
    ypr_description(pctx, when->dsc, when->exts, &inner_flag);
    ypr_reference(pctx, when->ref, when->exts, &inner_flag);
    LEVEL--;
    ypr_close(pctx, inner_flag);
}

static void
yprc_when(struct lys_ypr_ctx *pctx, const struct lysc_when *when, ly_bool *flag)
{
    ly_bool inner_flag = 0;

    if (!when) {
        return;
    }
    ypr_open(pctx->out, flag);

    ypr_text(pctx, "when", when->cond->expr, LYS_YPR_TEXT_SINGLELINE);

    LEVEL++;
    yprc_extension_instances(pctx, LY_STMT_WHEN, 0, when->exts, &inner_flag);
    ypr_description(pctx, when->dsc, when->exts, &inner_flag);
    ypr_reference(pctx, when->ref, when->exts, &inner_flag);
    LEVEL--;
    ypr_close(pctx, inner_flag);
}

static void
yprp_bits_enum(struct lys_ypr_ctx *pctx, const struct lysp_type_enum *items, LY_DATA_TYPE type, ly_bool *flag)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool inner_flag;

    LY_ARRAY_FOR(items, u) {
        ypr_open(pctx->out, flag);
        if (type == LY_TYPE_BITS) {
            ly_print_(pctx->out, "%*sbit %s", INDENT, items[u].name);
        } else { /* LY_TYPE_ENUM */
            ly_print_(pctx->out, "%*senum \"", INDENT);
            ypr_encode(pctx->out, items[u].name, -1);
            ly_print_(pctx->out, "\"");
        }
        inner_flag = 0;
        LEVEL++;
        yprp_extension_instances(pctx, LY_STMT_ENUM, 0, items[u].exts, &inner_flag);
        yprp_iffeatures(pctx, items[u].iffeatures, items[u].exts, &inner_flag);
        if (items[u].flags & LYS_SET_VALUE) {
            if (type == LY_TYPE_BITS) {
                ypr_unsigned(pctx, LY_STMT_POSITION, 0, items[u].exts, items[u].value, &inner_flag);
            } else { /* LY_TYPE_ENUM */
                ypr_signed(pctx, LY_STMT_VALUE, 0, items[u].exts, items[u].value, &inner_flag);
            }
        }
        ypr_status(pctx, items[u].flags, items[u].exts, &inner_flag);
        ypr_description(pctx, items[u].dsc, items[u].exts, &inner_flag);
        ypr_reference(pctx, items[u].ref, items[u].exts, &inner_flag);
        LEVEL--;
        ypr_close(pctx, inner_flag);
    }
}

static void
yprp_type(struct lys_ypr_ctx *pctx, const struct lysp_type *type)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool flag = 0;

    ly_print_(pctx->out, "%*stype %s", INDENT, type->name);
    LEVEL++;

    yprp_extension_instances(pctx, LY_STMT_TYPE, 0, type->exts, &flag);

    yprp_restr(pctx, type->range, LY_STMT_RANGE, &flag);
    yprp_restr(pctx, type->length, LY_STMT_LENGTH, &flag);
    LY_ARRAY_FOR(type->patterns, u) {
        yprp_restr(pctx, &type->patterns[u], LY_STMT_PATTERN, &flag);
    }
    yprp_bits_enum(pctx, type->bits, LY_TYPE_BITS, &flag);
    yprp_bits_enum(pctx, type->enums, LY_TYPE_ENUM, &flag);

    if (type->path) {
        ypr_open(pctx->out, &flag);
        ypr_substmt(pctx, LY_STMT_PATH, 0, type->path->expr, 0, type->exts);
    }
    if (type->flags & LYS_SET_REQINST) {
        ypr_open(pctx->out, &flag);
        ypr_substmt(pctx, LY_STMT_REQUIRE_INSTANCE, 0, type->require_instance ? "true" : "false", 0, type->exts);
    }
    if (type->flags & LYS_SET_FRDIGITS) {
        ypr_unsigned(pctx, LY_STMT_FRACTION_DIGITS, 0, type->exts, type->fraction_digits, &flag);
    }
    LY_ARRAY_FOR(type->bases, u) {
        ypr_open(pctx->out, &flag);
        ypr_substmt(pctx, LY_STMT_BASE, u, type->bases[u], 0, type->exts);
    }
    LY_ARRAY_FOR(type->types, u) {
        ypr_open(pctx->out, &flag);
        yprp_type(pctx, &type->types[u]);
    }

    LEVEL--;
    ypr_close(pctx, flag);
}

static void
yprc_dflt_value(struct lys_ypr_ctx *pctx, const struct ly_ctx *ly_pctx, const struct lyd_value *value,
        struct lysc_ext_instance *exts)
{
    ly_bool dynamic;
    const char *str;

    str = value->realtype->plugin->print(ly_pctx, value, LY_VALUE_JSON, NULL, &dynamic, NULL);
    ypr_substmt(pctx, LY_STMT_DEFAULT, 0, str, 0, exts);
    if (dynamic) {
        free((void *)str);
    }
}

static void
yprc_type(struct lys_ypr_ctx *pctx, const struct lysc_type *type)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool flag = 0;

    ly_print_(pctx->out, "%*stype %s", INDENT, lys_datatype2str(type->basetype));
    LEVEL++;

    yprc_extension_instances(pctx, LY_STMT_TYPE, 0, type->exts, &flag);

    switch (type->basetype) {
    case LY_TYPE_BINARY: {
        struct lysc_type_bin *bin = (struct lysc_type_bin *)type;

        yprc_range(pctx, bin->length, type->basetype, &flag);
        break;
    }
    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_UINT64:
    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32:
    case LY_TYPE_INT64: {
        struct lysc_type_num *num = (struct lysc_type_num *)type;

        yprc_range(pctx, num->range, type->basetype, &flag);
        break;
    }
    case LY_TYPE_STRING: {
        struct lysc_type_str *str = (struct lysc_type_str *)type;

        yprc_range(pctx, str->length, type->basetype, &flag);
        LY_ARRAY_FOR(str->patterns, u) {
            yprc_pattern(pctx, str->patterns[u], &flag);
        }
        break;
    }
    case LY_TYPE_BITS:
    case LY_TYPE_ENUM: {
        /* bits and enums structures are compatible */
        struct lysc_type_bits *bits = (struct lysc_type_bits *)type;

        yprc_bits_enum(pctx, bits->bits, type->basetype, &flag);
        break;
    }
    case LY_TYPE_BOOL:
    case LY_TYPE_EMPTY:
        /* nothing to do */
        break;
    case LY_TYPE_DEC64: {
        struct lysc_type_dec *dec = (struct lysc_type_dec *)type;

        ypr_open(pctx->out, &flag);
        ypr_unsigned(pctx, LY_STMT_FRACTION_DIGITS, 0, type->exts, dec->fraction_digits, &flag);
        yprc_range(pctx, dec->range, dec->basetype, &flag);
        break;
    }
    case LY_TYPE_IDENT: {
        struct lysc_type_identityref *ident = (struct lysc_type_identityref *)type;

        LY_ARRAY_FOR(ident->bases, u) {
            ypr_open(pctx->out, &flag);
            ypr_substmt(pctx, LY_STMT_BASE, u, ident->bases[u]->name, 0, type->exts);
        }
        break;
    }
    case LY_TYPE_INST: {
        struct lysc_type_instanceid *inst = (struct lysc_type_instanceid *)type;

        ypr_open(pctx->out, &flag);
        ypr_substmt(pctx, LY_STMT_REQUIRE_INSTANCE, 0, inst->require_instance ? "true" : "false", 0, inst->exts);
        break;
    }
    case LY_TYPE_LEAFREF: {
        struct lysc_type_leafref *lr = (struct lysc_type_leafref *)type;

        ypr_open(pctx->out, &flag);
        ypr_substmt(pctx, LY_STMT_PATH, 0, lr->path->expr, 0, lr->exts);
        ypr_substmt(pctx, LY_STMT_REQUIRE_INSTANCE, 0, lr->require_instance ? "true" : "false", 0, lr->exts);
        yprc_type(pctx, lr->realtype);
        break;
    }
    case LY_TYPE_UNION: {
        struct lysc_type_union *un = (struct lysc_type_union *)type;

        LY_ARRAY_FOR(un->types, u) {
            ypr_open(pctx->out, &flag);
            yprc_type(pctx, un->types[u]);
        }
        break;
    }
    default:
        LOGINT(pctx->module->ctx);
    }

    LEVEL--;
    ypr_close(pctx, flag);
}

static void
yprp_typedef(struct lys_ypr_ctx *pctx, const struct lysp_tpdf *tpdf)
{
    ly_print_(pctx->out, "%*stypedef %s {\n", INDENT, tpdf->name);
    LEVEL++;

    yprp_extension_instances(pctx, LY_STMT_TYPEDEF, 0, tpdf->exts, NULL);

    yprp_type(pctx, &tpdf->type);

    if (tpdf->units) {
        ypr_substmt(pctx, LY_STMT_UNITS, 0, tpdf->units, 0, tpdf->exts);
    }
    if (tpdf->dflt.str) {
        ypr_substmt(pctx, LY_STMT_DEFAULT, 0, tpdf->dflt.str, YPR_IS_LYS_SINGLEQUOTED(tpdf->dflt.flags), tpdf->exts);
    }

    ypr_status(pctx, tpdf->flags, tpdf->exts, NULL);
    ypr_description(pctx, tpdf->dsc, tpdf->exts, NULL);
    ypr_reference(pctx, tpdf->ref, tpdf->exts, NULL);

    LEVEL--;
    ly_print_(pctx->out, "%*s}\n", INDENT);
}

static void yprp_node(struct lys_ypr_ctx *pctx, const struct lysp_node *node);
static void yprc_node(struct lys_ypr_ctx *pctx, const struct lysc_node *node);
static void yprp_action(struct lys_ypr_ctx *pctx, const struct lysp_node_action *action);
static void yprp_notification(struct lys_ypr_ctx *pctx, const struct lysp_node_notif *notif);

static void
yprp_grouping(struct lys_ypr_ctx *pctx, const struct lysp_node_grp *grp)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool flag = 0;
    struct lysp_node *data;
    struct lysp_node_action *action;
    struct lysp_node_notif *notif;
    struct lysp_node_grp *subgrp;

    ly_print_(pctx->out, "%*sgrouping %s", INDENT, grp->name);
    LEVEL++;

    yprp_extension_instances(pctx, LY_STMT_GROUPING, 0, grp->exts, &flag);
    ypr_status(pctx, grp->flags, grp->exts, &flag);
    ypr_description(pctx, grp->dsc, grp->exts, &flag);
    ypr_reference(pctx, grp->ref, grp->exts, &flag);

    LY_ARRAY_FOR(grp->typedefs, u) {
        ypr_open(pctx->out, &flag);
        yprp_typedef(pctx, &grp->typedefs[u]);
    }

    LY_LIST_FOR(grp->groupings, subgrp) {
        ypr_open(pctx->out, &flag);
        yprp_grouping(pctx, subgrp);
    }

    LY_LIST_FOR(grp->child, data) {
        ypr_open(pctx->out, &flag);
        yprp_node(pctx, data);
    }

    LY_LIST_FOR(grp->actions, action) {
        ypr_open(pctx->out, &flag);
        yprp_action(pctx, action);
    }

    LY_LIST_FOR(grp->notifs, notif) {
        ypr_open(pctx->out, &flag);
        yprp_notification(pctx, notif);
    }

    LEVEL--;
    ypr_close(pctx, flag);
}

static void
yprp_inout(struct lys_ypr_ctx *pctx, const struct lysp_node_action_inout *inout, ly_bool *flag)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_node *data;
    struct lysp_node_grp *grp;

    if (!inout->child) {
        /* no children */
        return;
    }
    ypr_open(pctx->out, flag);
    YPR_EXTRA_LINE_PRINT(pctx);

    ly_print_(pctx->out, "%*s%s {\n", INDENT, inout->name);
    LEVEL++;

    yprp_extension_instances(pctx, LY_STMT_MUST, 0, inout->exts, NULL);
    LY_ARRAY_FOR(inout->musts, u) {
        yprp_restr(pctx, &inout->musts[u], LY_STMT_MUST, NULL);
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
    ypr_close(pctx, 1);
}

static void
yprc_inout(struct lys_ypr_ctx *pctx, const struct lysc_node_action_inout *inout, ly_bool *flag)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_node *data;

    if (!inout->child) {
        /* input/output is empty */
        return;
    }
    ypr_open(pctx->out, flag);

    ly_print_(pctx->out, "\n%*s%s {\n", INDENT, inout->name);
    LEVEL++;

    yprc_extension_instances(pctx, lyplg_ext_nodetype2stmt(inout->nodetype), 0, inout->exts, NULL);
    LY_ARRAY_FOR(inout->musts, u) {
        yprc_must(pctx, &inout->musts[u], NULL);
    }

    if (!(pctx->options & LYS_PRINT_NO_SUBSTMT)) {
        LY_LIST_FOR(inout->child, data) {
            yprc_node(pctx, data);
        }
    }

    LEVEL--;
    ypr_close(pctx, 1);
}

static void
yprp_notification(struct lys_ypr_ctx *pctx, const struct lysp_node_notif *notif)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool flag = 0;
    struct lysp_node *data;
    struct lysp_node_grp *grp;

    ly_print_(pctx->out, "%*snotification %s", INDENT, notif->name);

    LEVEL++;
    yprp_extension_instances(pctx, LY_STMT_NOTIFICATION, 0, notif->exts, &flag);
    yprp_iffeatures(pctx, notif->iffeatures, notif->exts, &flag);

    LY_ARRAY_FOR(notif->musts, u) {
        yprp_restr(pctx, &notif->musts[u], LY_STMT_MUST, &flag);
    }
    ypr_status(pctx, notif->flags, notif->exts, &flag);
    ypr_description(pctx, notif->dsc, notif->exts, &flag);
    ypr_reference(pctx, notif->ref, notif->exts, &flag);

    LY_ARRAY_FOR(notif->typedefs, u) {
        ypr_open(pctx->out, &flag);
        yprp_typedef(pctx, &notif->typedefs[u]);
    }

    LY_LIST_FOR(notif->groupings, grp) {
        ypr_open(pctx->out, &flag);
        yprp_grouping(pctx, grp);
    }

    LY_LIST_FOR(notif->child, data) {
        ypr_open(pctx->out, &flag);
        yprp_node(pctx, data);
    }

    LEVEL--;
    ypr_close(pctx, flag);
}

static void
yprc_notification(struct lys_ypr_ctx *pctx, const struct lysc_node_notif *notif)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool flag = 0;
    struct lysc_node *data;

    ly_print_(pctx->out, "%*snotification %s", INDENT, notif->name);

    LEVEL++;
    yprc_extension_instances(pctx, LY_STMT_NOTIFICATION, 0, notif->exts, &flag);

    LY_ARRAY_FOR(notif->musts, u) {
        yprc_must(pctx, &notif->musts[u], &flag);
    }
    ypr_status(pctx, notif->flags, notif->exts, &flag);
    ypr_description(pctx, notif->dsc, notif->exts, &flag);
    ypr_reference(pctx, notif->ref, notif->exts, &flag);

    if (!(pctx->options & LYS_PRINT_NO_SUBSTMT)) {
        LY_LIST_FOR(notif->child, data) {
            ypr_open(pctx->out, &flag);
            yprc_node(pctx, data);
        }
    }

    LEVEL--;
    ypr_close(pctx, flag);
}

static void
yprp_action(struct lys_ypr_ctx *pctx, const struct lysp_node_action *action)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool flag = 0;
    struct lysp_node_grp *grp;

    ly_print_(pctx->out, "%*s%s %s", INDENT, action->parent ? "action" : "rpc", action->name);

    LEVEL++;
    yprp_extension_instances(pctx, lyplg_ext_nodetype2stmt(action->nodetype), 0, action->exts, &flag);
    yprp_iffeatures(pctx, action->iffeatures, action->exts, &flag);
    ypr_status(pctx, action->flags, action->exts, &flag);
    ypr_description(pctx, action->dsc, action->exts, &flag);
    ypr_reference(pctx, action->ref, action->exts, &flag);

    YPR_EXTRA_LINE(flag, pctx);

    LY_ARRAY_FOR(action->typedefs, u) {
        ypr_open(pctx->out, &flag);
        YPR_EXTRA_LINE_PRINT(pctx);
        yprp_typedef(pctx, &action->typedefs[u]);
    }

    YPR_EXTRA_LINE(action->typedefs, pctx);

    LY_LIST_FOR(action->groupings, grp) {
        ypr_open(pctx->out, &flag);
        YPR_EXTRA_LINE_PRINT(pctx);
        yprp_grouping(pctx, grp);
    }

    YPR_EXTRA_LINE(action->groupings, pctx);

    yprp_inout(pctx, &action->input, &flag);
    yprp_inout(pctx, &action->output, &flag);

    LEVEL--;
    ypr_close(pctx, flag);
}

static void
yprc_action(struct lys_ypr_ctx *pctx, const struct lysc_node_action *action)
{
    ly_bool flag = 0;

    ly_print_(pctx->out, "%*s%s %s", INDENT, action->parent ? "action" : "rpc", action->name);

    LEVEL++;
    yprc_extension_instances(pctx, lyplg_ext_nodetype2stmt(action->nodetype), 0, action->exts, &flag);
    ypr_status(pctx, action->flags, action->exts, &flag);
    ypr_description(pctx, action->dsc, action->exts, &flag);
    ypr_reference(pctx, action->ref, action->exts, &flag);

    yprc_inout(pctx, &action->input, &flag);
    yprc_inout(pctx, &action->output, &flag);

    LEVEL--;
    ypr_close(pctx, flag);
}

static void
yprp_node_common1(struct lys_ypr_ctx *pctx, const struct lysp_node *node, ly_bool *flag)
{
    ly_print_(pctx->out, "%*s%s %s%s", INDENT, lys_nodetype2str(node->nodetype), node->name, flag ? "" : " {\n");
    LEVEL++;

    yprp_extension_instances(pctx, lyplg_ext_nodetype2stmt(node->nodetype), 0, node->exts, flag);
    yprp_when(pctx, lysp_node_when(node), flag);
    yprp_iffeatures(pctx, node->iffeatures, node->exts, flag);
}

static void
yprc_node_common1(struct lys_ypr_ctx *pctx, const struct lysc_node *node, ly_bool *flag)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_when **when;

    ly_print_(pctx->out, "%*s%s %s%s", INDENT, lys_nodetype2str(node->nodetype), node->name, flag ? "" : " {\n");
    LEVEL++;

    yprc_extension_instances(pctx, lyplg_ext_nodetype2stmt(node->nodetype), 0, node->exts, flag);

    when = lysc_node_when(node);
    LY_ARRAY_FOR(when, u) {
        yprc_when(pctx, when[u], flag);
    }
}

/* macro to unify the code */
#define YPR_NODE_COMMON2 \
    ypr_config(pctx, node->flags, node->exts, flag); \
    if (node->nodetype & (LYS_CHOICE | LYS_LEAF | LYS_ANYDATA)) { \
        ypr_mandatory(pctx, node->flags, node->exts, flag); \
    } \
    ypr_status(pctx, node->flags, node->exts, flag); \
    ypr_description(pctx, node->dsc, node->exts, flag); \
    ypr_reference(pctx, node->ref, node->exts, flag)

static void
yprp_node_common2(struct lys_ypr_ctx *pctx, const struct lysp_node *node, ly_bool *flag)
{
    YPR_NODE_COMMON2;
}

static void
yprc_node_common2(struct lys_ypr_ctx *pctx, const struct lysc_node *node, ly_bool *flag)
{
    YPR_NODE_COMMON2;
}

#undef YPR_NODE_COMMON2

static void
yprp_container(struct lys_ypr_ctx *pctx, const struct lysp_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool flag = 0;
    struct lysp_node *child;
    struct lysp_node_action *action;
    struct lysp_node_notif *notif;
    struct lysp_node_grp *grp;
    struct lysp_node_container *cont = (struct lysp_node_container *)node;

    yprp_node_common1(pctx, node, &flag);

    LY_ARRAY_FOR(cont->musts, u) {
        yprp_restr(pctx, &cont->musts[u], LY_STMT_MUST, &flag);
    }
    if (cont->presence) {
        ypr_open(pctx->out, &flag);
        ypr_substmt(pctx, LY_STMT_PRESENCE, 0, cont->presence, 0, cont->exts);
    }

    yprp_node_common2(pctx, node, &flag);

    LY_ARRAY_FOR(cont->typedefs, u) {
        ypr_open(pctx->out, &flag);
        yprp_typedef(pctx, &cont->typedefs[u]);
    }

    LY_LIST_FOR(cont->groupings, grp) {
        ypr_open(pctx->out, &flag);
        yprp_grouping(pctx, grp);
    }

    LY_LIST_FOR(cont->child, child) {
        ypr_open(pctx->out, &flag);
        yprp_node(pctx, child);
    }

    LY_LIST_FOR(cont->actions, action) {
        ypr_open(pctx->out, &flag);
        yprp_action(pctx, action);
    }

    LY_LIST_FOR(cont->notifs, notif) {
        ypr_open(pctx->out, &flag);
        yprp_notification(pctx, notif);
    }

    LEVEL--;
    ypr_close(pctx, flag);
}

static void
yprc_container(struct lys_ypr_ctx *pctx, const struct lysc_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool flag = 0;
    struct lysc_node *child;
    struct lysc_node_action *action;
    struct lysc_node_notif *notif;
    struct lysc_node_container *cont = (struct lysc_node_container *)node;

    yprc_node_common1(pctx, node, &flag);

    LY_ARRAY_FOR(cont->musts, u) {
        yprc_must(pctx, &cont->musts[u], &flag);
    }
    if (cont->flags & LYS_PRESENCE) {
        ypr_open(pctx->out, &flag);
        ypr_substmt(pctx, LY_STMT_PRESENCE, 0, "true", 0, cont->exts);
    }

    yprc_node_common2(pctx, node, &flag);

    if (!(pctx->options & LYS_PRINT_NO_SUBSTMT)) {
        LY_LIST_FOR(cont->child, child) {
            ypr_open(pctx->out, &flag);
            yprc_node(pctx, child);
        }

        LY_LIST_FOR(cont->actions, action) {
            ypr_open(pctx->out, &flag);
            yprc_action(pctx, action);
        }

        LY_LIST_FOR(cont->notifs, notif) {
            ypr_open(pctx->out, &flag);
            yprc_notification(pctx, notif);
        }
    }

    LEVEL--;
    ypr_close(pctx, flag);
}

static void
yprp_case(struct lys_ypr_ctx *pctx, const struct lysp_node *node)
{
    ly_bool flag = 0;
    struct lysp_node *child;
    struct lysp_node_case *cas = (struct lysp_node_case *)node;

    yprp_node_common1(pctx, node, &flag);
    yprp_node_common2(pctx, node, &flag);

    LY_LIST_FOR(cas->child, child) {
        ypr_open(pctx->out, &flag);
        yprp_node(pctx, child);
    }

    LEVEL--;
    ypr_close(pctx, flag);
}

static void
yprc_case(struct lys_ypr_ctx *pctx, const struct lysc_node_case *cs)
{
    ly_bool flag = 0;
    struct lysc_node *child;

    yprc_node_common1(pctx, &cs->node, &flag);
    yprc_node_common2(pctx, &cs->node, &flag);

    if (!(pctx->options & LYS_PRINT_NO_SUBSTMT)) {
        for (child = cs->child; child && child->parent == (struct lysc_node *)cs; child = child->next) {
            ypr_open(pctx->out, &flag);
            yprc_node(pctx, child);
        }
    }

    LEVEL--;
    ypr_close(pctx, flag);
}

static void
yprp_choice(struct lys_ypr_ctx *pctx, const struct lysp_node *node)
{
    ly_bool flag = 0;
    struct lysp_node *child;
    struct lysp_node_choice *choice = (struct lysp_node_choice *)node;

    yprp_node_common1(pctx, node, &flag);

    if (choice->dflt.str) {
        ypr_open(pctx->out, &flag);
        ypr_substmt(pctx, LY_STMT_DEFAULT, 0, choice->dflt.str, YPR_IS_LYS_SINGLEQUOTED(choice->dflt.flags), choice->exts);
    }

    yprp_node_common2(pctx, node, &flag);

    LY_LIST_FOR(choice->child, child) {
        ypr_open(pctx->out, &flag);
        yprp_node(pctx, child);
    }

    LEVEL--;
    ypr_close(pctx, flag);
}

static void
yprc_choice(struct lys_ypr_ctx *pctx, const struct lysc_node *node)
{
    ly_bool flag = 0;
    struct lysc_node_case *cs;
    struct lysc_node_choice *choice = (struct lysc_node_choice *)node;

    yprc_node_common1(pctx, node, &flag);

    if (choice->dflt) {
        ypr_open(pctx->out, &flag);
        ypr_substmt(pctx, LY_STMT_DEFAULT, 0, choice->dflt->name, 0, choice->exts);
    }

    yprc_node_common2(pctx, node, &flag);

    for (cs = choice->cases; cs; cs = (struct lysc_node_case *)cs->next) {
        ypr_open(pctx->out, &flag);
        yprc_case(pctx, cs);
    }

    LEVEL--;
    ypr_close(pctx, flag);
}

static void
yprp_leaf(struct lys_ypr_ctx *pctx, const struct lysp_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_node_leaf *leaf = (struct lysp_node_leaf *)node;

    yprp_node_common1(pctx, node, NULL);

    yprp_type(pctx, &leaf->type);
    ypr_substmt(pctx, LY_STMT_UNITS, 0, leaf->units, 0, leaf->exts);
    LY_ARRAY_FOR(leaf->musts, u) {
        yprp_restr(pctx, &leaf->musts[u], LY_STMT_MUST, NULL);
    }
    ypr_substmt(pctx, LY_STMT_DEFAULT, 0, leaf->dflt.str, YPR_IS_LYS_SINGLEQUOTED(leaf->dflt.flags), leaf->exts);

    yprp_node_common2(pctx, node, NULL);

    LEVEL--;
    ly_print_(pctx->out, "%*s}\n", INDENT);
}

static void
yprc_leaf(struct lys_ypr_ctx *pctx, const struct lysc_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_node_leaf *leaf = (struct lysc_node_leaf *)node;

    yprc_node_common1(pctx, node, NULL);

    yprc_type(pctx, leaf->type);
    ypr_substmt(pctx, LY_STMT_UNITS, 0, leaf->units, 0, leaf->exts);
    LY_ARRAY_FOR(leaf->musts, u) {
        yprc_must(pctx, &leaf->musts[u], NULL);
    }

    if (leaf->dflt) {
        yprc_dflt_value(pctx, node->module->ctx, leaf->dflt, leaf->exts);
    }

    yprc_node_common2(pctx, node, NULL);

    LEVEL--;
    ly_print_(pctx->out, "%*s}\n", INDENT);
}

static void
yprp_leaflist(struct lys_ypr_ctx *pctx, const struct lysp_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_node_leaflist *llist = (struct lysp_node_leaflist *)node;

    yprp_node_common1(pctx, node, NULL);

    yprp_type(pctx, &llist->type);
    ypr_substmt(pctx, LY_STMT_UNITS, 0, llist->units, 0, llist->exts);
    LY_ARRAY_FOR(llist->musts, u) {
        yprp_restr(pctx, &llist->musts[u], LY_STMT_MUST, NULL);
    }
    LY_ARRAY_FOR(llist->dflts, u) {
        ypr_substmt(pctx, LY_STMT_DEFAULT, u, llist->dflts[u].str, YPR_IS_LYS_SINGLEQUOTED(llist->dflts[u].flags),
                llist->exts);
    }

    ypr_config(pctx, node->flags, node->exts, NULL);

    if (llist->flags & LYS_SET_MIN) {
        ypr_unsigned(pctx, LY_STMT_MIN_ELEMENTS, 0, llist->exts, llist->min, NULL);
    }
    if (llist->flags & LYS_SET_MAX) {
        if (llist->max) {
            ypr_unsigned(pctx, LY_STMT_MAX_ELEMENTS, 0, llist->exts, llist->max, NULL);
        } else {
            ypr_substmt(pctx, LY_STMT_MAX_ELEMENTS, 0, "unbounded", 0, llist->exts);
        }
    }

    if (llist->flags & LYS_ORDBY_MASK) {
        ypr_substmt(pctx, LY_STMT_ORDERED_BY, 0, (llist->flags & LYS_ORDBY_USER) ? "user" : "system", 0, llist->exts);
    }

    ypr_status(pctx, node->flags, node->exts, NULL);
    ypr_description(pctx, node->dsc, node->exts, NULL);
    ypr_reference(pctx, node->ref, node->exts, NULL);

    LEVEL--;
    ly_print_(pctx->out, "%*s}\n", INDENT);
}

static void
yprc_leaflist(struct lys_ypr_ctx *pctx, const struct lysc_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_node_leaflist *llist = (struct lysc_node_leaflist *)node;

    yprc_node_common1(pctx, node, NULL);

    yprc_type(pctx, llist->type);
    ypr_substmt(pctx, LY_STMT_UNITS, 0, llist->units, 0, llist->exts);
    LY_ARRAY_FOR(llist->musts, u) {
        yprc_must(pctx, &llist->musts[u], NULL);
    }
    LY_ARRAY_FOR(llist->dflts, u) {
        yprc_dflt_value(pctx, node->module->ctx, llist->dflts[u], llist->exts);
    }

    ypr_config(pctx, node->flags, node->exts, NULL);

    ypr_unsigned(pctx, LY_STMT_MIN_ELEMENTS, 0, llist->exts, llist->min, NULL);
    if (llist->max) {
        ypr_unsigned(pctx, LY_STMT_MAX_ELEMENTS, 0, llist->exts, llist->max, NULL);
    } else {
        ypr_substmt(pctx, LY_STMT_MAX_ELEMENTS, 0, "unbounded", 0, llist->exts);
    }

    ypr_substmt(pctx, LY_STMT_ORDERED_BY, 0, (llist->flags & LYS_ORDBY_USER) ? "user" : "system", 0, llist->exts);

    ypr_status(pctx, node->flags, node->exts, NULL);
    ypr_description(pctx, node->dsc, node->exts, NULL);
    ypr_reference(pctx, node->ref, node->exts, NULL);

    LEVEL--;
    ly_print_(pctx->out, "%*s}\n", INDENT);
}

static void
yprp_list(struct lys_ypr_ctx *pctx, const struct lysp_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool flag = 0;
    struct lysp_node *child;
    struct lysp_node_action *action;
    struct lysp_node_notif *notif;
    struct lysp_node_grp *grp;
    struct lysp_node_list *list = (struct lysp_node_list *)node;

    yprp_node_common1(pctx, node, &flag);

    LY_ARRAY_FOR(list->musts, u) {
        yprp_restr(pctx, &list->musts[u], LY_STMT_MUST, &flag);
    }
    if (list->key) {
        ypr_open(pctx->out, &flag);
        ypr_substmt(pctx, LY_STMT_KEY, 0, list->key, 0, list->exts);
    }
    LY_ARRAY_FOR(list->uniques, u) {
        ypr_open(pctx->out, &flag);
        ypr_substmt(pctx, LY_STMT_UNIQUE, u, list->uniques[u].str, YPR_IS_LYS_SINGLEQUOTED(list->uniques[u].flags),
                list->exts);
    }

    ypr_config(pctx, node->flags, node->exts, &flag);

    if (list->flags & LYS_SET_MIN) {
        ypr_unsigned(pctx, LY_STMT_MIN_ELEMENTS, 0, list->exts, list->min, &flag);
    }
    if (list->flags & LYS_SET_MAX) {
        if (list->max) {
            ypr_unsigned(pctx, LY_STMT_MAX_ELEMENTS, 0, list->exts, list->max, &flag);
        } else {
            ypr_open(pctx->out, &flag);
            ypr_substmt(pctx, LY_STMT_MAX_ELEMENTS, 0, "unbounded", 0, list->exts);
        }
    }

    if (list->flags & LYS_ORDBY_MASK) {
        ypr_open(pctx->out, &flag);
        ypr_substmt(pctx, LY_STMT_ORDERED_BY, 0, (list->flags & LYS_ORDBY_USER) ? "user" : "system", 0, list->exts);
    }

    ypr_status(pctx, node->flags, node->exts, &flag);
    ypr_description(pctx, node->dsc, node->exts, &flag);
    ypr_reference(pctx, node->ref, node->exts, &flag);

    LY_ARRAY_FOR(list->typedefs, u) {
        ypr_open(pctx->out, &flag);
        yprp_typedef(pctx, &list->typedefs[u]);
    }

    LY_LIST_FOR(list->groupings, grp) {
        ypr_open(pctx->out, &flag);
        yprp_grouping(pctx, grp);
    }

    LY_LIST_FOR(list->child, child) {
        ypr_open(pctx->out, &flag);
        yprp_node(pctx, child);
    }

    LY_LIST_FOR(list->actions, action) {
        ypr_open(pctx->out, &flag);
        yprp_action(pctx, action);
    }

    LY_LIST_FOR(list->notifs, notif) {
        ypr_open(pctx->out, &flag);
        yprp_notification(pctx, notif);
    }

    LEVEL--;
    ypr_close(pctx, flag);
}

static void
yprc_list(struct lys_ypr_ctx *pctx, const struct lysc_node *node)
{
    LY_ARRAY_COUNT_TYPE u, v;
    struct lysc_node_list *list = (struct lysc_node_list *)node;

    yprc_node_common1(pctx, node, NULL);

    LY_ARRAY_FOR(list->musts, u) {
        yprc_must(pctx, &list->musts[u], NULL);
    }
    if (!(list->flags & LYS_KEYLESS)) {
        ly_print_(pctx->out, "%*skey \"", INDENT);
        for (struct lysc_node *key = list->child; key && key->nodetype == LYS_LEAF && (key->flags & LYS_KEY); key = key->next) {
            ly_print_(pctx->out, "%s%s", u > 0 ? ", " : "", key->name);
        }
        ly_print_(pctx->out, "\";\n");
    }
    LY_ARRAY_FOR(list->uniques, u) {
        ly_print_(pctx->out, "%*sunique \"", INDENT);
        LY_ARRAY_FOR(list->uniques[u], v) {
            ly_print_(pctx->out, "%s%s", v > 0 ? ", " : "", list->uniques[u][v]->name);
        }
        ypr_close(pctx, 0);
    }

    ypr_config(pctx, node->flags, node->exts, NULL);

    ypr_unsigned(pctx, LY_STMT_MIN_ELEMENTS, 0, list->exts, list->min, NULL);
    if (list->max) {
        ypr_unsigned(pctx, LY_STMT_MAX_ELEMENTS, 0, list->exts, list->max, NULL);
    } else {
        ypr_substmt(pctx, LY_STMT_MAX_ELEMENTS, 0, "unbounded", 0, list->exts);
    }

    ypr_substmt(pctx, LY_STMT_ORDERED_BY, 0, (list->flags & LYS_ORDBY_USER) ? "user" : "system", 0, list->exts);

    ypr_status(pctx, node->flags, node->exts, NULL);
    ypr_description(pctx, node->dsc, node->exts, NULL);
    ypr_reference(pctx, node->ref, node->exts, NULL);

    if (!(pctx->options & LYS_PRINT_NO_SUBSTMT)) {
        struct lysc_node *child;
        struct lysc_node_action *action;
        struct lysc_node_notif *notif;

        LY_LIST_FOR(list->child, child) {
            yprc_node(pctx, child);
        }

        LY_LIST_FOR(list->actions, action) {
            yprc_action(pctx, action);
        }

        LY_LIST_FOR(list->notifs, notif) {
            yprc_notification(pctx, notif);
        }
    }

    LEVEL--;
    ypr_close(pctx, 1);
}

static void
yprp_refine(struct lys_ypr_ctx *pctx, struct lysp_refine *refine)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool flag = 0;

    ly_print_(pctx->out, "%*srefine \"%s\"", INDENT, refine->nodeid);
    LEVEL++;

    yprp_extension_instances(pctx, LY_STMT_REFINE, 0, refine->exts, &flag);
    yprp_iffeatures(pctx, refine->iffeatures, refine->exts, &flag);

    LY_ARRAY_FOR(refine->musts, u) {
        ypr_open(pctx->out, &flag);
        yprp_restr(pctx, &refine->musts[u], LY_STMT_MUST, NULL);
    }

    if (refine->presence) {
        ypr_open(pctx->out, &flag);
        ypr_substmt(pctx, LY_STMT_PRESENCE, 0, refine->presence, 0, refine->exts);
    }

    LY_ARRAY_FOR(refine->dflts, u) {
        ypr_open(pctx->out, &flag);
        ypr_substmt(pctx, LY_STMT_DEFAULT, u, refine->dflts[u].str, YPR_IS_LYS_SINGLEQUOTED(refine->dflts[u].flags),
                refine->exts);
    }

    ypr_config(pctx, refine->flags, refine->exts, &flag);
    ypr_mandatory(pctx, refine->flags, refine->exts, &flag);

    if (refine->flags & LYS_SET_MIN) {
        ypr_open(pctx->out, &flag);
        ypr_unsigned(pctx, LY_STMT_MIN_ELEMENTS, 0, refine->exts, refine->min, NULL);
    }
    if (refine->flags & LYS_SET_MAX) {
        ypr_open(pctx->out, &flag);
        if (refine->max) {
            ypr_unsigned(pctx, LY_STMT_MAX_ELEMENTS, 0, refine->exts, refine->max, NULL);
        } else {
            ypr_substmt(pctx, LY_STMT_MAX_ELEMENTS, 0, "unbounded", 0, refine->exts);
        }
    }

    ypr_description(pctx, refine->dsc, refine->exts, &flag);
    ypr_reference(pctx, refine->ref, refine->exts, &flag);

    LEVEL--;
    ypr_close(pctx, flag);
}

static void
yprp_augment(struct lys_ypr_ctx *pctx, const struct lysp_node_augment *aug)
{
    struct lysp_node *child;
    struct lysp_node_action *action;
    struct lysp_node_notif *notif;

    ly_print_(pctx->out, "%*saugment \"%s\" {\n", INDENT, aug->nodeid);
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
    ypr_close(pctx, 1);
}

static void
yprp_uses(struct lys_ypr_ctx *pctx, const struct lysp_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool flag = 0;
    struct lysp_node_uses *uses = (struct lysp_node_uses *)node;
    struct lysp_node_augment *aug;

    yprp_node_common1(pctx, node, &flag);
    yprp_node_common2(pctx, node, &flag);

    LY_ARRAY_FOR(uses->refines, u) {
        ypr_open(pctx->out, &flag);
        yprp_refine(pctx, &uses->refines[u]);
    }

    LY_LIST_FOR(uses->augments, aug) {
        ypr_open(pctx->out, &flag);
        yprp_augment(pctx, aug);
    }

    LEVEL--;
    ypr_close(pctx, flag);
}

static void
yprp_anydata(struct lys_ypr_ctx *pctx, const struct lysp_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool flag = 0;
    struct lysp_node_anydata *any = (struct lysp_node_anydata *)node;

    yprp_node_common1(pctx, node, &flag);

    LY_ARRAY_FOR(any->musts, u) {
        ypr_open(pctx->out, &flag);
        yprp_restr(pctx, &any->musts[u], LY_STMT_MUST, NULL);
    }

    yprp_node_common2(pctx, node, &flag);

    LEVEL--;
    ypr_close(pctx, flag);
}

static void
yprc_anydata(struct lys_ypr_ctx *pctx, const struct lysc_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool flag = 0;
    struct lysc_node_anydata *any = (struct lysc_node_anydata *)node;

    yprc_node_common1(pctx, node, &flag);

    LY_ARRAY_FOR(any->musts, u) {
        ypr_open(pctx->out, &flag);
        yprc_must(pctx, &any->musts[u], NULL);
    }

    yprc_node_common2(pctx, node, &flag);

    LEVEL--;
    ypr_close(pctx, flag);
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
yprc_node(struct lys_ypr_ctx *pctx, const struct lysc_node *node)
{
    switch (node->nodetype) {
    case LYS_CONTAINER:
        yprc_container(pctx, node);
        break;
    case LYS_CHOICE:
        yprc_choice(pctx, node);
        break;
    case LYS_LEAF:
        yprc_leaf(pctx, node);
        break;
    case LYS_LEAFLIST:
        yprc_leaflist(pctx, node);
        break;
    case LYS_LIST:
        yprc_list(pctx, node);
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        yprc_anydata(pctx, node);
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

    ly_print_(pctx->out, "%*sdeviation \"%s\" {\n", INDENT, deviation->nodeid);
    LEVEL++;

    yprp_extension_instances(pctx, LY_STMT_DEVIATION, 0, deviation->exts, NULL);
    ypr_description(pctx, deviation->dsc, deviation->exts, NULL);
    ypr_reference(pctx, deviation->ref, deviation->exts, NULL);

    LY_LIST_FOR(deviation->deviates, elem) {
        ly_print_(pctx->out, "%*sdeviate ", INDENT);
        if (elem->mod == LYS_DEV_NOT_SUPPORTED) {
            if (elem->exts) {
                ly_print_(pctx->out, "not-supported {\n");
                LEVEL++;

                yprp_extension_instances(pctx, LY_STMT_DEVIATE, 0, elem->exts, NULL);
            } else {
                ly_print_(pctx->out, "not-supported;\n");
                continue;
            }
        } else if (elem->mod == LYS_DEV_ADD) {
            add = (struct lysp_deviate_add *)elem;
            ly_print_(pctx->out, "add {\n");
            LEVEL++;

            yprp_extension_instances(pctx, LY_STMT_DEVIATE, 0, add->exts, NULL);
            ypr_substmt(pctx, LY_STMT_UNITS, 0, add->units, 0, add->exts);
            LY_ARRAY_FOR(add->musts, u) {
                yprp_restr(pctx, &add->musts[u], LY_STMT_MUST, NULL);
            }
            LY_ARRAY_FOR(add->uniques, u) {
                ypr_substmt(pctx, LY_STMT_UNIQUE, u, add->uniques[u].str, YPR_IS_LYS_SINGLEQUOTED(add->uniques[u].flags),
                        add->exts);
            }
            LY_ARRAY_FOR(add->dflts, u) {
                ypr_substmt(pctx, LY_STMT_DEFAULT, u, add->dflts[u].str, YPR_IS_LYS_SINGLEQUOTED(add->dflts[u].flags),
                        add->exts);
            }
            ypr_config(pctx, add->flags, add->exts, NULL);
            ypr_mandatory(pctx, add->flags, add->exts, NULL);
            if (add->flags & LYS_SET_MIN) {
                ypr_unsigned(pctx, LY_STMT_MIN_ELEMENTS, 0, add->exts, add->min, NULL);
            }
            if (add->flags & LYS_SET_MAX) {
                if (add->max) {
                    ypr_unsigned(pctx, LY_STMT_MAX_ELEMENTS, 0, add->exts, add->max, NULL);
                } else {
                    ypr_substmt(pctx, LY_STMT_MAX_ELEMENTS, 0, "unbounded", 0, add->exts);
                }
            }
        } else if (elem->mod == LYS_DEV_REPLACE) {
            rpl = (struct lysp_deviate_rpl *)elem;
            ly_print_(pctx->out, "replace {\n");
            LEVEL++;

            yprp_extension_instances(pctx, LY_STMT_DEVIATE, 0, rpl->exts, NULL);
            if (rpl->type) {
                yprp_type(pctx, rpl->type);
            }
            ypr_substmt(pctx, LY_STMT_UNITS, 0, rpl->units, 0, rpl->exts);
            ypr_substmt(pctx, LY_STMT_DEFAULT, 0, rpl->dflt.str, YPR_IS_LYS_SINGLEQUOTED(rpl->dflt.flags), rpl->exts);
            ypr_config(pctx, rpl->flags, rpl->exts, NULL);
            ypr_mandatory(pctx, rpl->flags, rpl->exts, NULL);
            if (rpl->flags & LYS_SET_MIN) {
                ypr_unsigned(pctx, LY_STMT_MIN_ELEMENTS, 0, rpl->exts, rpl->min, NULL);
            }
            if (rpl->flags & LYS_SET_MAX) {
                if (rpl->max) {
                    ypr_unsigned(pctx, LY_STMT_MAX_ELEMENTS, 0, rpl->exts, rpl->max, NULL);
                } else {
                    ypr_substmt(pctx, LY_STMT_MAX_ELEMENTS, 0, "unbounded", 0, rpl->exts);
                }
            }
        } else if (elem->mod == LYS_DEV_DELETE) {
            del = (struct lysp_deviate_del *)elem;
            ly_print_(pctx->out, "delete {\n");
            LEVEL++;

            yprp_extension_instances(pctx, LY_STMT_DEVIATE, 0, del->exts, NULL);
            ypr_substmt(pctx, LY_STMT_UNITS, 0, del->units, 0, del->exts);
            LY_ARRAY_FOR(del->musts, u) {
                yprp_restr(pctx, &del->musts[u], LY_STMT_MUST, NULL);
            }
            LY_ARRAY_FOR(del->uniques, u) {
                ypr_substmt(pctx, LY_STMT_UNIQUE, u, del->uniques[u].str, YPR_IS_LYS_SINGLEQUOTED(del->uniques[u].flags),
                        del->exts);
            }
            LY_ARRAY_FOR(del->dflts, u) {
                ypr_substmt(pctx, LY_STMT_DEFAULT, u, del->dflts[u].str, YPR_IS_LYS_SINGLEQUOTED(del->dflts[u].flags),
                        del->exts);
            }
        }

        LEVEL--;
        ypr_close(pctx, 1);
    }

    LEVEL--;
    ypr_close(pctx, 1);
}

static void
yang_print_parsed_linkage(struct lys_ypr_ctx *pctx, const struct lysp_module *modp)
{
    LY_ARRAY_COUNT_TYPE u;

    LY_ARRAY_FOR(modp->imports, u) {
        if (modp->imports[u].flags & LYS_INTERNAL) {
            continue;
        }

        YPR_EXTRA_LINE_PRINT(pctx);
        ly_print_(pctx->out, "%*simport %s {\n", INDENT, modp->imports[u].name);
        LEVEL++;
        yprp_extension_instances(pctx, LY_STMT_IMPORT, 0, modp->imports[u].exts, NULL);
        ypr_substmt(pctx, LY_STMT_PREFIX, 0, modp->imports[u].prefix, 0, modp->imports[u].exts);
        if (modp->imports[u].rev[0]) {
            ypr_substmt(pctx, LY_STMT_REVISION_DATE, 0, modp->imports[u].rev, 0, modp->imports[u].exts);
        }
        ypr_substmt(pctx, LY_STMT_DESCRIPTION, 0, modp->imports[u].dsc, 0, modp->imports[u].exts);
        ypr_substmt(pctx, LY_STMT_REFERENCE, 0, modp->imports[u].ref, 0, modp->imports[u].exts);
        LEVEL--;
        ly_print_(pctx->out, "%*s}\n", INDENT);
    }
    YPR_EXTRA_LINE(modp->imports, pctx);

    LY_ARRAY_FOR(modp->includes, u) {
        if (modp->includes[u].injected) {
            /* do not print the includes injected from submodules */
            continue;
        }
        YPR_EXTRA_LINE_PRINT(pctx);
        if (modp->includes[u].rev[0] || modp->includes[u].dsc || modp->includes[u].ref || modp->includes[u].exts) {
            ly_print_(pctx->out, "%*sinclude %s {\n", INDENT, modp->includes[u].name);
            LEVEL++;
            yprp_extension_instances(pctx, LY_STMT_INCLUDE, 0, modp->includes[u].exts, NULL);
            if (modp->includes[u].rev[0]) {
                ypr_substmt(pctx, LY_STMT_REVISION_DATE, 0, modp->includes[u].rev, 0, modp->includes[u].exts);
            }
            ypr_substmt(pctx, LY_STMT_DESCRIPTION, 0, modp->includes[u].dsc, 0, modp->includes[u].exts);
            ypr_substmt(pctx, LY_STMT_REFERENCE, 0, modp->includes[u].ref, 0, modp->includes[u].exts);
            LEVEL--;
            ly_print_(pctx->out, "%*s}\n", INDENT);
        } else {
            ly_print_(pctx->out, "\n%*sinclude \"%s\";\n", INDENT, modp->includes[u].name);
        }
    }
    YPR_EXTRA_LINE(modp->includes, pctx);
}

static void
yang_print_parsed_body(struct lys_ypr_ctx *pctx, const struct lysp_module *modp)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_node *data;
    struct lysp_node_action *action;
    struct lysp_node_notif *notif;
    struct lysp_node_grp *grp;
    struct lysp_node_augment *aug;

    LY_ARRAY_FOR(modp->extensions, u) {
        YPR_EXTRA_LINE_PRINT(pctx);
        yprp_extension(pctx, &modp->extensions[u]);
    }

    YPR_EXTRA_LINE(modp->extensions, pctx);

    if (yprp_extension_has_printable_instances(modp->exts)) {
        YPR_EXTRA_LINE_PRINT(pctx);
        yprp_extension_instances(pctx, LY_STMT_MODULE, 0, modp->exts, NULL);
    }

    YPR_EXTRA_LINE(modp->exts, pctx);

    LY_ARRAY_FOR(modp->features, u) {
        YPR_EXTRA_LINE_PRINT(pctx);
        yprp_feature(pctx, &modp->features[u]);
        YPR_EXTRA_LINE(1, pctx);
    }

    YPR_EXTRA_LINE(modp->features, pctx);

    LY_ARRAY_FOR(modp->identities, u) {
        YPR_EXTRA_LINE_PRINT(pctx);
        yprp_identity(pctx, &modp->identities[u]);
        YPR_EXTRA_LINE(1, pctx);
    }

    YPR_EXTRA_LINE(modp->identities, pctx);

    LY_ARRAY_FOR(modp->typedefs, u) {
        YPR_EXTRA_LINE_PRINT(pctx);
        yprp_typedef(pctx, &modp->typedefs[u]);
        YPR_EXTRA_LINE(1, pctx);
    }

    YPR_EXTRA_LINE(modp->typedefs, pctx);

    LY_LIST_FOR(modp->groupings, grp) {
        YPR_EXTRA_LINE_PRINT(pctx);
        yprp_grouping(pctx, grp);
        YPR_EXTRA_LINE(1, pctx);
    }

    YPR_EXTRA_LINE(modp->groupings, pctx);

    LY_LIST_FOR(modp->data, data) {
        YPR_EXTRA_LINE_PRINT(pctx);
        yprp_node(pctx, data);
    }

    YPR_EXTRA_LINE(modp->data, pctx);

    LY_LIST_FOR(modp->augments, aug) {
        YPR_EXTRA_LINE_PRINT(pctx);
        yprp_augment(pctx, aug);
    }

    YPR_EXTRA_LINE(modp->augments, pctx);

    LY_LIST_FOR(modp->rpcs, action) {
        YPR_EXTRA_LINE_PRINT(pctx);
        yprp_action(pctx, action);
    }

    YPR_EXTRA_LINE(modp->rpcs, pctx);

    LY_LIST_FOR(modp->notifs, notif) {
        YPR_EXTRA_LINE_PRINT(pctx);
        yprp_notification(pctx, notif);
    }

    YPR_EXTRA_LINE(modp->notifs, pctx);

    LY_ARRAY_FOR(modp->deviations, u) {
        YPR_EXTRA_LINE_PRINT(pctx);
        yprp_deviation(pctx, &modp->deviations[u]);
    }

    YPR_EXTRA_LINE(modp->deviations, pctx);
}

LY_ERR
yang_print_parsed_module(struct ly_out *out, const struct lysp_module *modp, uint32_t options)
{
    LY_ARRAY_COUNT_TYPE u;
    const struct lys_module *module = modp->mod;
    struct lys_ypr_ctx pctx_ = {
        .out = out,
        .level = 0,
        .options = options,
        .module = module,
        .schema = LYS_YPR_PARSED
    }, *pctx = &pctx_;

    ly_print_(pctx->out, "%*smodule %s {\n", INDENT, module->name);
    LEVEL++;

    /* module-header-stmts */
    if (modp->version) {
        ypr_substmt(pctx, LY_STMT_YANG_VERSION, 0, modp->version == LYS_VERSION_1_1 ? "1.1" : "1", 0, modp->exts);
    }
    ypr_substmt(pctx, LY_STMT_NAMESPACE, 0, module->ns, 0, modp->exts);
    ypr_substmt(pctx, LY_STMT_PREFIX, 0, module->prefix, 0, modp->exts);

    YPR_EXTRA_LINE(1, pctx);

    /* linkage-stmts (import/include) */
    yang_print_parsed_linkage(pctx, modp);

    /* meta-stmts */
    if (module->org || module->contact || module->dsc || module->ref) {
        YPR_EXTRA_LINE_PRINT(pctx);
    }
    ypr_substmt(pctx, LY_STMT_ORGANIZATION, 0, module->org, 0, modp->exts);
    ypr_substmt(pctx, LY_STMT_CONTACT, 0, module->contact, 0, modp->exts);
    ypr_substmt(pctx, LY_STMT_DESCRIPTION, 0, module->dsc, 0, modp->exts);
    ypr_substmt(pctx, LY_STMT_REFERENCE, 0, module->ref, 0, modp->exts);

    YPR_EXTRA_LINE(module->org || module->contact || module->dsc || module->ref, pctx);

    /* revision-stmts */
    LY_ARRAY_FOR(modp->revs, u) {
        YPR_EXTRA_LINE_PRINT(pctx);
        yprp_revision(pctx, &modp->revs[u]);
    }

    YPR_EXTRA_LINE(modp->revs, pctx);

    /* body-stmts */
    yang_print_parsed_body(pctx, modp);

    LEVEL--;
    ly_print_(out, "%*s}\n", INDENT);
    ly_print_flush(out);

    return LY_SUCCESS;
}

static void
yprp_belongsto(struct lys_ypr_ctx *pctx, const struct lysp_submodule *submodp)
{
    ly_print_(pctx->out, "%*sbelongs-to %s {\n", INDENT, submodp->mod->name);
    LEVEL++;
    yprp_extension_instances(pctx, LY_STMT_BELONGS_TO, 0, submodp->exts, NULL);
    ypr_substmt(pctx, LY_STMT_PREFIX, 0, submodp->prefix, 0, submodp->exts);
    LEVEL--;
    ly_print_(pctx->out, "%*s}\n", INDENT);
}

LY_ERR
yang_print_parsed_submodule(struct ly_out *out, const struct lysp_submodule *submodp, uint32_t options)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lys_ypr_ctx pctx_ = {
        .out = out,
        .level = 0,
        .options = options,
        .module = submodp->mod,
        .schema = LYS_YPR_PARSED
    }, *pctx = &pctx_;

    ly_print_(pctx->out, "%*ssubmodule %s {\n", INDENT, submodp->name);
    LEVEL++;

    /* submodule-header-stmts */
    if (submodp->version) {
        ypr_substmt(pctx, LY_STMT_YANG_VERSION, 0, submodp->version == LYS_VERSION_1_1 ? "1.1" : "1", 0, submodp->exts);
    }

    yprp_belongsto(pctx, submodp);

    YPR_EXTRA_LINE(1, pctx);

    /* linkage-stmts (import/include) */
    yang_print_parsed_linkage(pctx, (struct lysp_module *)submodp);

    /* meta-stmts */
    if (submodp->org || submodp->contact || submodp->dsc || submodp->ref) {
        YPR_EXTRA_LINE_PRINT(pctx);
    }
    ypr_substmt(pctx, LY_STMT_ORGANIZATION, 0, submodp->org, 0, submodp->exts);
    ypr_substmt(pctx, LY_STMT_CONTACT, 0, submodp->contact, 0, submodp->exts);
    ypr_substmt(pctx, LY_STMT_DESCRIPTION, 0, submodp->dsc, 0, submodp->exts);
    ypr_substmt(pctx, LY_STMT_REFERENCE, 0, submodp->ref, 0, submodp->exts);

    YPR_EXTRA_LINE(submodp->org || submodp->contact || submodp->dsc || submodp->ref, pctx);

    /* revision-stmts */
    LY_ARRAY_FOR(submodp->revs, u) {
        YPR_EXTRA_LINE_PRINT(pctx);
        yprp_revision(pctx, &submodp->revs[u]);
    }

    YPR_EXTRA_LINE(submodp->revs, pctx);

    /* body-stmts */
    yang_print_parsed_body(pctx, (struct lysp_module *)submodp);

    LEVEL--;
    ly_print_(out, "%*s}\n", INDENT);
    ly_print_flush(out);

    return LY_SUCCESS;
}

LY_ERR
yang_print_compiled_node(struct ly_out *out, const struct lysc_node *node, uint32_t options)
{
    struct lys_ypr_ctx pctx_ = {
        .out = out,
        .level = 0,
        .options = options,
        .module = node->module,
        .schema = LYS_YPR_COMPILED
    }, *pctx = &pctx_;

    yprc_node(pctx, node);

    ly_print_flush(out);
    return LY_SUCCESS;
}

LY_ERR
yang_print_compiled(struct ly_out *out, const struct lys_module *module, uint32_t options)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_module *modc = module->compiled;
    struct lys_ypr_ctx pctx_ = {
        .out = out,
        .level = 0,
        .options = options,
        .module = module,
        .schema = LYS_YPR_COMPILED
    }, *pctx = &pctx_;

    ly_print_(pctx->out, "%*smodule %s {\n", INDENT, module->name);
    LEVEL++;

    /* module-header-stmts */
    ypr_substmt(pctx, LY_STMT_NAMESPACE, 0, module->ns, 0, modc->exts);
    ypr_substmt(pctx, LY_STMT_PREFIX, 0, module->prefix, 0, modc->exts);

    YPR_EXTRA_LINE(1, pctx);

    /* no linkage-stmts */

    /* meta-stmts */
    if (module->org || module->contact || module->dsc || module->ref) {
        YPR_EXTRA_LINE_PRINT(pctx);
    }
    ypr_substmt(pctx, LY_STMT_ORGANIZATION, 0, module->org, 0, modc->exts);
    ypr_substmt(pctx, LY_STMT_CONTACT, 0, module->contact, 0, modc->exts);
    ypr_substmt(pctx, LY_STMT_DESCRIPTION, 0, module->dsc, 0, modc->exts);
    ypr_substmt(pctx, LY_STMT_REFERENCE, 0, module->ref, 0, modc->exts);

    YPR_EXTRA_LINE(module->org || module->contact || module->dsc || module->ref, pctx);

    /* revision-stmts */
    if (module->revision) {
        YPR_EXTRA_LINE_PRINT(pctx);
        ly_print_(pctx->out, "%*srevision %s;\n", INDENT, module->revision);
        YPR_EXTRA_LINE(1, pctx);
    }

    /* body-stmts */
    if (modc->exts) {
        YPR_EXTRA_LINE_PRINT(pctx);
        yprc_extension_instances(pctx, LY_STMT_MODULE, 0, module->compiled->exts, NULL);
        YPR_EXTRA_LINE(1, pctx);
    }

    LY_ARRAY_FOR(module->identities, u) {
        YPR_EXTRA_LINE_PRINT(pctx);
        yprc_identity(pctx, &module->identities[u]);
        YPR_EXTRA_LINE(1, pctx);
    }

    if (!(pctx->options & LYS_PRINT_NO_SUBSTMT)) {
        struct lysc_node *data;
        struct lysc_node_action *rpc;
        struct lysc_node_notif *notif;

        LY_LIST_FOR(modc->data, data) {
            YPR_EXTRA_LINE_PRINT(pctx);
            yprc_node(pctx, data);
        }

        YPR_EXTRA_LINE(modc->data, pctx);

        LY_LIST_FOR(modc->rpcs, rpc) {
            YPR_EXTRA_LINE_PRINT(pctx);
            yprc_action(pctx, rpc);
        }

        YPR_EXTRA_LINE(modc->rpcs, pctx);

        LY_LIST_FOR(modc->notifs, notif) {
            YPR_EXTRA_LINE_PRINT(pctx);
            yprc_notification(pctx, notif);
        }

        YPR_EXTRA_LINE(modc->notifs, pctx);
    }

    LEVEL--;
    ly_print_(out, "%*s}\n", INDENT);
    ly_print_flush(out);

    return LY_SUCCESS;
}

LIBYANG_API_DEF void
lyplg_ext_print_info_extension_instance(struct lyspr_ctx *ctx, const struct lysc_ext_instance *ext, ly_bool *flag)
{
    struct lys_ypr_ctx *pctx = (struct lys_ypr_ctx *)ctx;
    LY_ARRAY_COUNT_TYPE u, v;
    ly_bool data_printed = 0;

    LY_ARRAY_FOR(ext->substmts, u) {
        switch (ext->substmts[u].stmt) {
        case LY_STMT_NOTIFICATION:
        case LY_STMT_INPUT:
        case LY_STMT_OUTPUT:
        case LY_STMT_ACTION:
        case LY_STMT_RPC:
        case LY_STMT_ANYDATA:
        case LY_STMT_ANYXML:
        case LY_STMT_CASE:
        case LY_STMT_CHOICE:
        case LY_STMT_CONTAINER:
        case LY_STMT_LEAF:
        case LY_STMT_LEAF_LIST:
        case LY_STMT_LIST:
        case LY_STMT_USES: {
            const struct lysc_node *node;

            if (data_printed) {
                break;
            }

            LY_LIST_FOR(*(const struct lysc_node **)ext->substmts[u].storage, node) {
                ypr_open(pctx->out, flag);
                if (ext->substmts[u].stmt == LY_STMT_NOTIFICATION) {
                    yprc_notification(pctx, (struct lysc_node_notif *)node);
                } else if (ext->substmts[u].stmt & (LY_STMT_INPUT | LY_STMT_OUTPUT)) {
                    yprc_inout(pctx, (struct lysc_node_action_inout *)node, flag);
                } else if (ext->substmts[u].stmt & (LY_STMT_ACTION | LY_STMT_RPC)) {
                    yprc_action(pctx, (struct lysc_node_action *)node);
                } else {
                    yprc_node(pctx, node);
                }
            }

            /* all data nodes are stored in a linked list so all were printed */
            data_printed = 1;
            break;
        }
        case LY_STMT_ARGUMENT:
        case LY_STMT_CONTACT:
        case LY_STMT_DESCRIPTION:
        case LY_STMT_ERROR_APP_TAG:
        case LY_STMT_ERROR_MESSAGE:
        case LY_STMT_KEY:
        case LY_STMT_MODIFIER:
        case LY_STMT_NAMESPACE:
        case LY_STMT_ORGANIZATION:
        case LY_STMT_PRESENCE:
        case LY_STMT_REFERENCE:
        case LY_STMT_UNITS:
            if (*(const char **)ext->substmts[u].storage) {
                ypr_open(pctx->out, flag);
                ypr_substmt(pctx, ext->substmts[u].stmt, 0, *(const char **)ext->substmts[u].storage, 0, ext->exts);
            }
            break;
        case LY_STMT_BIT:
        case LY_STMT_ENUM: {
            const struct lysc_type_bitenum_item *items = *(struct lysc_type_bitenum_item **)ext->substmts[u].storage;

            yprc_bits_enum(pctx, items, ext->substmts[u].stmt == LY_STMT_BIT ? LY_TYPE_BITS : LY_TYPE_ENUM, flag);
            break;
        }
        case LY_STMT_CONFIG:
            ypr_config(pctx, *(uint16_t *)ext->substmts[u].storage, ext->exts, flag);
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            yprc_extension_instances(pctx, LY_STMT_EXTENSION_INSTANCE, 0,
                    *(struct lysc_ext_instance **)ext->substmts[u].storage, flag);
            break;
        case LY_STMT_FRACTION_DIGITS:
            if (*(uint8_t *)ext->substmts[u].storage) {
                ypr_unsigned(pctx, LY_STMT_FRACTION_DIGITS, 0, ext->exts, *(uint8_t *)ext->substmts[u].storage, flag);
            }
            break;
        case LY_STMT_IDENTITY: {
            const struct lysc_ident *idents = *(struct lysc_ident **)ext->substmts[u].storage;

            LY_ARRAY_FOR(idents, v) {
                yprc_identity(pctx, &idents[v]);
            }
            break;
        }
        case LY_STMT_LENGTH:
            if (*(struct lysc_range **)ext->substmts[u].storage) {
                yprc_range(pctx, *(struct lysc_range **)ext->substmts[u].storage, LY_TYPE_STRING, flag);
            }
            break;
        case LY_STMT_MANDATORY:
            ypr_mandatory(pctx, *(uint16_t *)ext->substmts[u].storage, ext->exts, flag);
            break;
        case LY_STMT_MAX_ELEMENTS: {
            uint32_t max = *(uint32_t *)ext->substmts[u].storage;

            if (max) {
                ypr_unsigned(pctx, LY_STMT_MAX_ELEMENTS, 0, ext->exts, max, flag);
            } else {
                ypr_open(pctx->out, flag);
                ypr_substmt(pctx, LY_STMT_MAX_ELEMENTS, 0, "unbounded", 0, ext->exts);
            }
            break;
        }
        case LY_STMT_MIN_ELEMENTS:
            ypr_unsigned(pctx, LY_STMT_MIN_ELEMENTS, 0, ext->exts, *(uint32_t *)ext->substmts[u].storage, flag);
            break;
        case LY_STMT_ORDERED_BY:
            ypr_open(pctx->out, flag);
            ypr_substmt(pctx, LY_STMT_ORDERED_BY, 0,
                    (*(uint16_t *)ext->substmts[u].storage & LYS_ORDBY_USER) ? "user" : "system", 0, ext->exts);
            break;
        case LY_STMT_MUST: {
            const struct lysc_must *musts = *(struct lysc_must **)ext->substmts[u].storage;

            LY_ARRAY_FOR(musts, v) {
                yprc_must(pctx, &musts[v], flag);
            }
            break;
        }
        case LY_STMT_PATTERN: {
            const struct lysc_pattern *patterns = *(struct lysc_pattern **)ext->substmts[u].storage;

            LY_ARRAY_FOR(patterns, v) {
                yprc_pattern(pctx, &patterns[v], flag);
            }
            break;
        }
        case LY_STMT_POSITION:
            if (*(int64_t *)ext->substmts[u].storage) {
                ypr_unsigned(pctx, ext->substmts[u].stmt, 0, ext->exts, *(int64_t *)ext->substmts[u].storage, flag);
            }
            break;
        case LY_STMT_VALUE:
            if (*(int64_t *)ext->substmts[u].storage) {
                ypr_signed(pctx, ext->substmts[u].stmt, 0, ext->exts, *(int64_t *)ext->substmts[u].storage, flag);
            }
            break;
        case LY_STMT_RANGE:
            if (*(struct lysc_range **)ext->substmts[u].storage) {
                yprc_range(pctx, *(struct lysc_range **)ext->substmts[u].storage, LY_TYPE_UINT64, flag);
            }
            break;
        case LY_STMT_REQUIRE_INSTANCE:
            ypr_open(pctx->out, flag);
            ypr_substmt(pctx, LY_STMT_REQUIRE_INSTANCE, 0, *(uint8_t *)ext->substmts[u].storage ? "true" : "false",
                    0, ext->exts);
            break;
        case LY_STMT_STATUS:
            ypr_status(pctx, *(uint16_t *)ext->substmts[u].storage, ext->exts, flag);
            break;
        case LY_STMT_TYPE:
            if (*(const struct lysc_type **)ext->substmts[u].storage) {
                ypr_open(pctx->out, flag);
                yprc_type(pctx, *(const struct lysc_type **)ext->substmts[u].storage);
            }
            break;
        case LY_STMT_WHEN:
            yprc_when(pctx, *(struct lysc_when **)ext->substmts[u].storage, flag);
            break;
        case LY_STMT_AUGMENT:
        case LY_STMT_BASE:
        case LY_STMT_BELONGS_TO:
        case LY_STMT_DEFAULT:
        case LY_STMT_DEVIATE:
        case LY_STMT_DEVIATION:
        case LY_STMT_EXTENSION:
        case LY_STMT_FEATURE:
        case LY_STMT_GROUPING:
        case LY_STMT_IF_FEATURE:
        case LY_STMT_IMPORT:
        case LY_STMT_INCLUDE:
        case LY_STMT_MODULE:
        case LY_STMT_PATH:
        case LY_STMT_PREFIX:
        case LY_STMT_REFINE:
        case LY_STMT_REVISION:
        case LY_STMT_REVISION_DATE:
        case LY_STMT_SUBMODULE:
        case LY_STMT_TYPEDEF:
        case LY_STMT_UNIQUE:
        case LY_STMT_YANG_VERSION:
        case LY_STMT_YIN_ELEMENT:
            /* nothing to do */
            break;
        default:
            LOGINT(pctx->module->ctx);
            break;
        }
    }
}
