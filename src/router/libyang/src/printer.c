/**
 * @file printer.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Wrapper for all libyang printers.
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE /* vasprintf(), vdprintf() */
#define _POSIX_C_SOURCE 200809L

#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "tree_schema.h"
#include "tree_data.h"
#include "printer.h"

struct ext_substmt_info_s ext_substmt_info[] = {
  {NULL, NULL, 0},                              /**< LYEXT_SUBSTMT_SELF */
  {"argument", "name", SUBST_FLAG_ID},          /**< LYEXT_SUBSTMT_ARGUMENT */
  {"base", "name", SUBST_FLAG_ID},              /**< LYEXT_SUBSTMT_BASE */
  {"belongs-to", "module", SUBST_FLAG_ID},      /**< LYEXT_SUBSTMT_BELONGSTO */
  {"contact", "text", SUBST_FLAG_YIN},          /**< LYEXT_SUBSTMT_CONTACT */
  {"default", "value", 0},                      /**< LYEXT_SUBSTMT_DEFAULT */
  {"description", "text", SUBST_FLAG_YIN},      /**< LYEXT_SUBSTMT_DESCRIPTION */
  {"error-app-tag", "value", 0},                /**< LYEXT_SUBSTMT_ERRTAG */
  {"error-message", "value", SUBST_FLAG_YIN},   /**< LYEXT_SUBSTMT_ERRMSG */
  {"key", "value", 0},                          /**< LYEXT_SUBSTMT_KEY */
  {"namespace", "uri", 0},                      /**< LYEXT_SUBSTMT_NAMESPACE */
  {"organization", "text", SUBST_FLAG_YIN},     /**< LYEXT_SUBSTMT_ORGANIZATION */
  {"path", "value", 0},                         /**< LYEXT_SUBSTMT_PATH */
  {"prefix", "value", SUBST_FLAG_ID},           /**< LYEXT_SUBSTMT_PREFIX */
  {"presence", "value", 0},                     /**< LYEXT_SUBSTMT_PRESENCE */
  {"reference", "text", SUBST_FLAG_YIN},        /**< LYEXT_SUBSTMT_REFERENCE */
  {"revision-date", "date", SUBST_FLAG_ID},     /**< LYEXT_SUBSTMT_REVISIONDATE */
  {"units", "name", 0},                         /**< LYEXT_SUBSTMT_UNITS */
  {"value", "value", SUBST_FLAG_ID},            /**< LYEXT_SUBSTMT_VALUE */
  {"yang-version", "value", SUBST_FLAG_ID},     /**< LYEXT_SUBSTMT_VERSION */
  {"modifier", "value", SUBST_FLAG_ID},         /**< LYEXT_SUBSTMT_MODIFIER */
  {"require-instance", "value", SUBST_FLAG_ID}, /**< LYEXT_SUBSTMT_REQINST */
  {"yin-element", "value", SUBST_FLAG_ID},      /**< LYEXT_SUBSTMT_YINELEM */
  {"config", "value", SUBST_FLAG_ID},           /**< LYEXT_SUBSTMT_CONFIG */
  {"mandatory", "value", SUBST_FLAG_ID},        /**< LYEXT_SUBSTMT_MANDATORY */
  {"ordered-by", "value", SUBST_FLAG_ID},       /**< LYEXT_SUBSTMT_ORDEREDBY */
  {"status", "value", SUBST_FLAG_ID},           /**< LYEXT_SUBSTMT_STATUS */
  {"fraction-digits", "value", SUBST_FLAG_ID},  /**< LYEXT_SUBSTMT_DIGITS */
  {"max-elements", "value", SUBST_FLAG_ID},     /**< LYEXT_SUBSTMT_MAX */
  {"min-elements", "value", SUBST_FLAG_ID},     /**< LYEXT_SUBSTMT_MIN */
  {"position", "value", SUBST_FLAG_ID},         /**< LYEXT_SUBSTMT_POSITION */
  {"unique", "tag", 0},                         /**< LYEXT_SUBSTMT_UNIQUE */
};

/* 0 - same, 1 - different */
int
nscmp(const struct lyd_node *node1, const struct lyd_node *node2)
{
    /* we have to cover submodules belonging to the same module */
    if (lys_node_module(node1->schema) == lys_node_module(node2->schema)) {
        /* belongs to the same module */
        return 0;
    } else {
        /* different modules */
        return 1;
    }
}

int
ly_print(struct lyout *out, const char *format, ...)
{
    int count = 0;
    char *msg = NULL, *aux;
    va_list ap;

    va_start(ap, format);

    switch (out->type) {
    case LYOUT_FD:
        count = vdprintf(out->method.fd, format, ap);
        break;
    case LYOUT_STREAM:
        count = vfprintf(out->method.f, format, ap);
        break;
    case LYOUT_MEMORY:
        count = vasprintf(&msg, format, ap);
        if (out->method.mem.len + count + 1 > out->method.mem.size) {
            aux = ly_realloc(out->method.mem.buf, out->method.mem.len + count + 1);
            if (!aux) {
                out->method.mem.buf = NULL;
                out->method.mem.len = 0;
                out->method.mem.size = 0;
                LOGMEM(NULL);
                va_end(ap);
                return -1;
            }
            out->method.mem.buf = aux;
            out->method.mem.size = out->method.mem.len + count + 1;
        }
        memcpy(&out->method.mem.buf[out->method.mem.len], msg, count);
        out->method.mem.len += count;
        out->method.mem.buf[out->method.mem.len] = '\0';
        free(msg);
        break;
    case LYOUT_CALLBACK:
        count = vasprintf(&msg, format, ap);
        count = out->method.clb.f(out->method.clb.arg, msg, count);
        if (count >= 0) {
            /*
             * Depending on what the callback function does, errno might
             * contain non-zero values that are not real "errors" (EAGAIN or
             * EINTR). Reset errno if the callback returns a zero or positive
             * value.
             */
            errno = 0;
        }
        free(msg);
        break;
    }

    va_end(ap);
    return count;
}

void
ly_print_flush(struct lyout *out)
{
    switch (out->type) {
    case LYOUT_STREAM:
        fflush(out->method.f);
        break;
    case LYOUT_FD:
    case LYOUT_MEMORY:
    case LYOUT_CALLBACK:
        /* nothing to do */
        break;
    }
}

int
ly_write(struct lyout *out, const char *buf, size_t count)
{
    if (out->hole_count) {
        /* we are buffering data after a hole */
        if (out->buf_len + count > out->buf_size) {
            out->buffered = ly_realloc(out->buffered, out->buf_len + count);
            if (!out->buffered) {
                out->buf_len = 0;
                out->buf_size = 0;
                LOGMEM(NULL);
                return -1;
            }
            out->buf_size = out->buf_len + count;
        }

        memcpy(&out->buffered[out->buf_len], buf, count);
        out->buf_len += count;
        return count;
    }

    switch (out->type) {
    case LYOUT_MEMORY:
        if (out->method.mem.len + count + 1 > out->method.mem.size) {
            out->method.mem.buf = ly_realloc(out->method.mem.buf, out->method.mem.len + count + 1);
            if (!out->method.mem.buf) {
                out->method.mem.len = 0;
                out->method.mem.size = 0;
                LOGMEM(NULL);
                return -1;
            }
            out->method.mem.size = out->method.mem.len + count + 1;
        }
        memcpy(&out->method.mem.buf[out->method.mem.len], buf, count);
        out->method.mem.len += count;
        out->method.mem.buf[out->method.mem.len] = '\0';
        return count;
    case LYOUT_FD:
        return write(out->method.fd, buf, count);
    case LYOUT_STREAM:
        return fwrite(buf, sizeof *buf, count, out->method.f);
    case LYOUT_CALLBACK:
        return out->method.clb.f(out->method.clb.arg, buf, count);
    }

    return 0;
}

int
ly_write_skip(struct lyout *out, size_t count, size_t *position)
{
    switch (out->type) {
    case LYOUT_MEMORY:
        if (out->method.mem.len + count > out->method.mem.size) {
            out->method.mem.buf = ly_realloc(out->method.mem.buf, out->method.mem.len + count);
            if (!out->method.mem.buf) {
                out->method.mem.len = 0;
                out->method.mem.size = 0;
                LOGMEM(NULL);
                return -1;
            }
            out->method.mem.size = out->method.mem.len + count;
        }

        /* save the current position */
        *position = out->method.mem.len;

        /* skip the memory */
        out->method.mem.len += count;
        break;
    case LYOUT_FD:
    case LYOUT_STREAM:
    case LYOUT_CALLBACK:
        /* buffer the hole */
        if (out->buf_len + count > out->buf_size) {
            out->buffered = ly_realloc(out->buffered, out->buf_len + count);
            if (!out->buffered) {
                out->buf_len = 0;
                out->buf_size = 0;
                LOGMEM(NULL);
                return -1;
            }
            out->buf_size = out->buf_len + count;
        }

        /* save the current position */
        *position = out->buf_len;

        /* skip the memory */
        out->buf_len += count;

        /* increase hole counter */
        ++out->hole_count;
    }

    return count;
}

int
ly_write_skipped(struct lyout *out, size_t position, const char *buf, size_t count)
{
    switch (out->type) {
    case LYOUT_MEMORY:
        /* write */
        memcpy(&out->method.mem.buf[position], buf, count);
        break;
    case LYOUT_FD:
    case LYOUT_STREAM:
    case LYOUT_CALLBACK:
        if (out->buf_len < position + count) {
            LOGINT(NULL);
            return -1;
        }

        /* write into the hole */
        memcpy(&out->buffered[position], buf, count);

        /* decrease hole counter */
        --out->hole_count;

        if (!out->hole_count) {
            /* all holes filled, we can write the buffer */
            count = ly_write(out, out->buffered, out->buf_len);
            out->buf_len = 0;
        }
        break;
    }

    return count;
}

static int
write_iff(struct lyout *out, const struct lys_module *module, struct lys_iffeature *expr, int prefix_kind,
          int *index_e, int *index_f)
{
    int count = 0, brackets_flag = *index_e;
    uint8_t op;
    struct lys_module *mod;

    op = iff_getop(expr->expr, *index_e);
    (*index_e)++;

    switch (op) {
    case LYS_IFF_F:
        if (lys_main_module(expr->features[*index_f]->module) != lys_main_module(module)) {
            if (prefix_kind == 0) {
                count += ly_print(out, "%s:", transform_module_name2import_prefix(module,
                                  lys_main_module(expr->features[*index_f]->module)->name));
            } else if (prefix_kind == 1) {
                count += ly_print(out, "%s:", lys_main_module(expr->features[*index_f]->module)->name);
            } else if (prefix_kind == 2) {
                count += ly_print(out, "%s:", lys_main_module(expr->features[*index_f]->module)->prefix);
            } else if (prefix_kind == 3) {
                mod =  lys_main_module(expr->features[*index_f]->module);
                count += ly_print(out, "%s%s%s:", mod->name, mod->rev_size ? "@" : "", mod->rev_size ? mod->rev[0].date : "");
            }
        }
        count += ly_print(out, expr->features[*index_f]->name);
        (*index_f)++;
        break;
    case LYS_IFF_NOT:
        count += ly_print(out, "not ");
        count += write_iff(out, module, expr, prefix_kind, index_e, index_f);
        break;
    case LYS_IFF_AND:
        if (brackets_flag) {
            /* AND need brackets only if previous op was not */
            if (*index_e < 2 || iff_getop(expr->expr, *index_e - 2) != LYS_IFF_NOT) {
                brackets_flag = 0;
            }
        }
        /* falls through */
    case LYS_IFF_OR:
        if (brackets_flag) {
            count += ly_print(out, "(");
        }
        count += write_iff(out, module, expr, prefix_kind, index_e, index_f);
        count += ly_print(out, " %s ", op == LYS_IFF_OR ? "or" : "and");
        count += write_iff(out, module, expr, prefix_kind, index_e, index_f);
        if (brackets_flag) {
            count += ly_print(out, ")");
        }
    }

    return count;
}

int
ly_print_iffeature(struct lyout *out, const struct lys_module *module, struct lys_iffeature *expr, int prefix_kind)
{
    int index_e = 0, index_f = 0;

    if (expr->expr) {
        return write_iff(out, module, expr, prefix_kind, &index_e, &index_f);
    }

    return 0;
}

static int
lys_print_(struct lyout *out, const struct lys_module *module, LYS_OUTFORMAT format, const char *target_node,
           int line_length, int options)
{
    int ret;

    switch (format) {
    case LYS_OUT_YIN:
        lys_disable_deviations((struct lys_module *)module);
        ret = yin_print_model(out, module);
        lys_enable_deviations((struct lys_module *)module);
        break;
    case LYS_OUT_YANG:
        lys_disable_deviations((struct lys_module *)module);
        ret = yang_print_model(out, module);
        lys_enable_deviations((struct lys_module *)module);
        break;
    case LYS_OUT_TREE:
        ret = tree_print_model(out, module, target_node, line_length, options);
        break;
    case LYS_OUT_INFO:
        ret = info_print_model(out, module, target_node);
        break;
    case LYS_OUT_JSON:
        ret = jsons_print_model(out, module, target_node);
        break;
    default:
        LOGERR(module->ctx, LY_EINVAL, "Unknown output format.");
        ret = EXIT_FAILURE;
        break;
    }

    return ret;
}

API int
lys_print_file(FILE *f, const struct lys_module *module, LYS_OUTFORMAT format, const char *target_node,
               int line_length, int options)
{
    struct lyout out;

    if (!f || !module) {
        LOGARG;
        return EXIT_FAILURE;
    }

    memset(&out, 0, sizeof out);

    out.type = LYOUT_STREAM;
    out.method.f = f;

    return lys_print_(&out, module, format, target_node, line_length, options);
}

API int
lys_print_path(const char *path, const struct lys_module *module, LYS_OUTFORMAT format, const char *target_node,
               int line_length, int options)
{
    FILE *f;
    int ret;

    if (!path || !module) {
        LOGARG;
        return EXIT_FAILURE;
    }

    f = fopen(path, "w");
    if (!f) {
        LOGERR(module->ctx, LY_ESYS, "Failed to open file \"%s\" (%s).", path, strerror(errno));
        return EXIT_FAILURE;
    }

    ret = lys_print_file(f, module, format, target_node, line_length, options);
    fclose(f);
    return ret;
}

API int
lys_print_fd(int fd, const struct lys_module *module, LYS_OUTFORMAT format, const char *target_node,
             int line_length, int options)
{
    struct lyout out;

    if (fd < 0 || !module) {
        LOGARG;
        return EXIT_FAILURE;
    }

    memset(&out, 0, sizeof out);

    out.type = LYOUT_FD;
    out.method.fd = fd;

    return lys_print_(&out, module, format, target_node, line_length, options);
}

API int
lys_print_mem(char **strp, const struct lys_module *module, LYS_OUTFORMAT format, const char *target_node,
              int line_length, int options)
{
    struct lyout out;
    int r;

    if (!strp || !module) {
        LOGARG;
        return EXIT_FAILURE;
    }

    memset(&out, 0, sizeof out);

    out.type = LYOUT_MEMORY;

    r = lys_print_(&out, module, format, target_node, line_length, options);

    *strp = out.method.mem.buf;
    return r;
}

API int
lys_print_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg, const struct lys_module *module,
              LYS_OUTFORMAT format, const char *target_node, int line_length, int options)
{
    struct lyout out;

    if (!writeclb || !module) {
        LOGARG;
        return EXIT_FAILURE;
    }

    memset(&out, 0, sizeof out);

    out.type = LYOUT_CALLBACK;
    out.method.clb.f = writeclb;
    out.method.clb.arg = arg;

    return lys_print_(&out, module, format, target_node, line_length, options);
}

int
lys_print_target(struct lyout *out, const struct lys_module *module, const char *target_schema_path,
                 void (*clb_print_typedef)(struct lyout*, const struct lys_tpdf*, int*),
                 void (*clb_print_identity)(struct lyout*, const struct lys_ident*, int*),
                 void (*clb_print_feature)(struct lyout*, const struct lys_feature*, int*),
                 void (*clb_print_type)(struct lyout*, const struct lys_type*, int*),
                 void (*clb_print_grouping)(struct lyout*, const struct lys_node*, int*),
                 void (*clb_print_container)(struct lyout*, const struct lys_node*, int*),
                 void (*clb_print_choice)(struct lyout*, const struct lys_node*, int*),
                 void (*clb_print_leaf)(struct lyout*, const struct lys_node*, int*),
                 void (*clb_print_leaflist)(struct lyout*, const struct lys_node*, int*),
                 void (*clb_print_list)(struct lyout*, const struct lys_node*, int*),
                 void (*clb_print_anydata)(struct lyout*, const struct lys_node*, int*),
                 void (*clb_print_case)(struct lyout*, const struct lys_node*, int*),
                 void (*clb_print_notif)(struct lyout*, const struct lys_node*, int*),
                 void (*clb_print_rpc)(struct lyout*, const struct lys_node*, int*),
                 void (*clb_print_action)(struct lyout*, const struct lys_node*, int*),
                 void (*clb_print_input)(struct lyout*, const struct lys_node*, int*),
                 void (*clb_print_output)(struct lyout*, const struct lys_node*, int*))
{
    int rc, i, f = 1;
    char *spec_target = NULL;
    struct lys_node *target = NULL;
    struct lys_tpdf *tpdf = NULL;
    uint8_t tpdf_size = 0;

    if ((target_schema_path[0] == '/') || !strncmp(target_schema_path, "type/", 5)) {
        rc = resolve_absolute_schema_nodeid((target_schema_path[0] == '/' ? target_schema_path : target_schema_path + 4), module,
                                            LYS_ANY & ~(LYS_USES | LYS_AUGMENT | LYS_GROUPING), (const struct lys_node **)&target);
        if (rc || !target) {
            LOGERR(module->ctx, LY_EINVAL, "Target %s could not be resolved.",
                   (target_schema_path[0] == '/' ? target_schema_path : target_schema_path + 4));
            return EXIT_FAILURE;
        }
    } else if (!strncmp(target_schema_path, "grouping/", 9)) {
        /* cut the data part off */
        if ((spec_target = strchr(target_schema_path + 9, '/'))) {
            /* HACK only temporary */
            spec_target[0] = '\0';
            ++spec_target;
        }
        rc = resolve_absolute_schema_nodeid(target_schema_path + 8, module, LYS_GROUPING, (const struct lys_node **)&target);
        if (rc || !target) {
            ly_print(out, "Grouping %s not found.\n", target_schema_path + 8);
            return EXIT_FAILURE;
        }
    } else if (!strncmp(target_schema_path, "typedef/", 8)) {
        if ((spec_target = strrchr(target_schema_path + 8, '/'))) {
            /* schema node typedef */
            /* HACK only temporary */
            spec_target[0] = '\0';
            ++spec_target;

            rc = resolve_absolute_schema_nodeid(target_schema_path + 7, module,
                                                LYS_CONTAINER | LYS_LIST | LYS_NOTIF | LYS_RPC | LYS_ACTION,
                                                (const struct lys_node **)&target);
            if (rc || !target) {
                /* perhaps it's in a grouping */
                rc = resolve_absolute_schema_nodeid(target_schema_path + 7, module, LYS_GROUPING,
                                                    (const struct lys_node **)&target);
            }
            if (!rc && target) {
                switch (target->nodetype) {
                case LYS_CONTAINER:
                    tpdf = ((struct lys_node_container *)target)->tpdf;
                    tpdf_size = ((struct lys_node_container *)target)->tpdf_size;
                    break;
                case LYS_LIST:
                    tpdf = ((struct lys_node_list *)target)->tpdf;
                    tpdf_size = ((struct lys_node_list *)target)->tpdf_size;
                    break;
                case LYS_NOTIF:
                    tpdf = ((struct lys_node_notif *)target)->tpdf;
                    tpdf_size = ((struct lys_node_notif *)target)->tpdf_size;
                    break;
                case LYS_RPC:
                case LYS_ACTION:
                    tpdf = ((struct lys_node_rpc_action *)target)->tpdf;
                    tpdf_size = ((struct lys_node_rpc_action *)target)->tpdf_size;
                    break;
                case LYS_GROUPING:
                    tpdf = ((struct lys_node_grp *)target)->tpdf;
                    tpdf_size = ((struct lys_node_grp *)target)->tpdf_size;
                    break;
                default:
                    LOGINT(module->ctx);
                    return EXIT_FAILURE;
                }
            }
        } else {
            /* module typedef */
            spec_target = (char *)target_schema_path + 8;
            tpdf = module->tpdf;
            tpdf_size = module->tpdf_size;
        }

        for (i = 0; i < tpdf_size; ++i) {
            if (!strcmp(tpdf[i].name, spec_target)) {
                clb_print_typedef(out, &tpdf[i], &f);
                break;
            }
        }
        /* HACK return previous hack */
        --spec_target;
        spec_target[0] = '/';

        if (i == tpdf_size) {
            ly_print(out, "Typedef %s not found.\n", target_schema_path);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;

    } else if (!strncmp(target_schema_path, "identity/", 9)) {
        target_schema_path += 9;
        for (i = 0; i < (signed)module->ident_size; ++i) {
            if (!strcmp(module->ident[i].name, target_schema_path)) {
                break;
            }
        }
        if (i == (signed)module->ident_size) {
            ly_print(out, "Identity %s not found.\n", target_schema_path);
            return EXIT_FAILURE;
        }

        clb_print_identity(out, &module->ident[i], &f);
        return EXIT_SUCCESS;

    } else if (!strncmp(target_schema_path, "feature/", 8)) {
        target_schema_path += 8;
        for (i = 0; i < module->features_size; ++i) {
            if (!strcmp(module->features[i].name, target_schema_path)) {
                break;
            }
        }
        if (i == module->features_size) {
            ly_print(out, "Feature %s not found.\n", target_schema_path);
            return EXIT_FAILURE;
        }

        clb_print_feature(out, &module->features[i], &f);
        return EXIT_SUCCESS;
    } else {
        ly_print(out, "Target could not be resolved.\n");
        return EXIT_FAILURE;
    }

    if (!strncmp(target_schema_path, "type/", 5)) {
        if (!(target->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
            LOGERR(module->ctx, LY_EINVAL, "Target is not a leaf or a leaf-list.");
            return EXIT_FAILURE;
        }
        clb_print_type(out, &((struct lys_node_leaf *)target)->type, &f);
        return EXIT_SUCCESS;
    } else if (!strncmp(target_schema_path, "grouping/", 9) && !spec_target) {
        clb_print_grouping(out, target, &f);
        return EXIT_SUCCESS;
    }

    /* find the node in the grouping */
    if (spec_target) {
        rc = resolve_descendant_schema_nodeid(spec_target, target->child, LYS_NO_RPC_NOTIF_NODE,
                                              0, (const struct lys_node **)&target);
        if (rc || !target) {
            ly_print(out, "Grouping %s child \"%s\" not found.\n", target_schema_path + 9, spec_target);
            return EXIT_FAILURE;
        }
        /* HACK return previous hack */
        --spec_target;
        spec_target[0] = '/';
    }
    switch (target->nodetype) {
    case LYS_CONTAINER:
        clb_print_container(out, target, &f);
        break;
    case LYS_CHOICE:
        clb_print_choice(out, target, &f);
        break;
    case LYS_LEAF:
        clb_print_leaf(out, target, &f);
        break;
    case LYS_LEAFLIST:
        clb_print_leaflist(out, target, &f);
        break;
    case LYS_LIST:
        clb_print_list(out, target, &f);
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        clb_print_anydata(out, target, &f);
        break;
    case LYS_CASE:
        clb_print_case(out, target, &f);
        break;
    case LYS_NOTIF:
        clb_print_notif(out, target, &f);
        break;
    case LYS_RPC:
        clb_print_rpc(out, target, &f);
        break;
    case LYS_ACTION:
        clb_print_action(out, target, &f);
        break;
    case LYS_INPUT:
        clb_print_input(out, target, &f);
        break;
    case LYS_OUTPUT:
        clb_print_output(out, target, &f);
        break;
    default:
        ly_print(out, "Nodetype %s not supported.\n", strnodetype(target->nodetype));
        break;
    }

    return EXIT_SUCCESS;
}

static int
lyd_print_(struct lyout *out, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    switch (format) {
    case LYD_XML:
        return xml_print_data(out, root, options);
    case LYD_JSON:
        return json_print_data(out, root, options);
    case LYD_LYB:
        return lyb_print_data(out, root, options);
    default:
        LOGERR(root->schema->module->ctx, LY_EINVAL, "Unknown output format.");
        return EXIT_FAILURE;
    }
}

API int
lyd_print_file(FILE *f, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    int r;
    struct lyout out;

    if (!f) {
        LOGARG;
        return EXIT_FAILURE;
    }

    memset(&out, 0, sizeof out);

    out.type = LYOUT_STREAM;
    out.method.f = f;

    r = lyd_print_(&out, root, format, options);

    free(out.buffered);
    return r;
}

API int
lyd_print_path(const char *path, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    FILE *f;
    int ret;

    if (!path) {
        LOGARG;
        return EXIT_FAILURE;
    }

    f = fopen(path, "w");
    if (!f) {
        LOGERR(root->schema->module->ctx, LY_EINVAL, "Cannot open file \"%s\" for writing.", path);
        return EXIT_FAILURE;
    }

    ret = lyd_print_file(f, root, format, options);

    fclose(f);
    return ret;
}

API int
lyd_print_fd(int fd, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    int r;
    struct lyout out;

    if (fd < 0) {
        LOGARG;
        return EXIT_FAILURE;
    }

    memset(&out, 0, sizeof out);

    out.type = LYOUT_FD;
    out.method.fd = fd;

    r = lyd_print_(&out, root, format, options);

    free(out.buffered);
    return r;
}

API int
lyd_print_mem(char **strp, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    struct lyout out;
    int r;

    if (!strp) {
        LOGARG;
        return EXIT_FAILURE;
    }

    memset(&out, 0, sizeof out);

    out.type = LYOUT_MEMORY;

    r = lyd_print_(&out, root, format, options);

    *strp = out.method.mem.buf;
    free(out.buffered);
    return r;
}

API int
lyd_print_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg, const struct lyd_node *root,
              LYD_FORMAT format, int options)
{
    int r;
    struct lyout out;

    if (!writeclb) {
        LOGARG;
        return EXIT_FAILURE;
    }

    memset(&out, 0, sizeof out);

    out.type = LYOUT_CALLBACK;
    out.method.clb.f = writeclb;
    out.method.clb.arg = arg;

    r = lyd_print_(&out, root, format, options);

    free(out.buffered);
    return r;
}

static int
lyd_wd_toprint(const struct lyd_node *node, int options)
{
    const struct lyd_node *subroot, *next, *elem;
    int flag = 0;

    if (options & LYP_WD_TRIM) {
        /* do not print default nodes */
        if (node->dflt) {
            /* implicit default node */
            return 0;
        } else if (node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
            if (lyd_wd_default((struct lyd_node_leaf_list *)node)) {
                /* explicit default node */
                return 0;
            }
        } else if ((node->schema->nodetype & (LYS_CONTAINER)) && !((struct lys_node_container *)node->schema)->presence) {
            /* get know if non-presence container contains non-default node */
            for (subroot = node->child; subroot && !flag; subroot = subroot->next) {
                LY_TREE_DFS_BEGIN(subroot, next, elem) {
                    if (elem->dflt) {
                        /* skip subtree */
                        goto trim_dfs_nextsibling;
                    }
                    switch (elem->schema->nodetype) {
                    case LYS_LEAF:
                    case LYS_LEAFLIST:
                        if (!lyd_wd_default((struct lyd_node_leaf_list *)elem)) {
                            /* non-default node */
                            flag = 1;
                        }
                        break;
                    case LYS_ANYDATA:
                    case LYS_ANYXML:
                    case LYS_NOTIF:
                    case LYS_ACTION:
                    case LYS_LIST:
                        /* non-default nodes */
                        flag = 1;
                        break;
                    case LYS_CONTAINER:
                        if (((struct lys_node_container *)elem->schema)->presence) {
                            /* non-default node */
                            flag = 1;
                        }
                        break;
                    default:
                        break;
                    }
                    if (flag) {
                        break;
                    }

                    /* modified LY_TREE_DFS_END */
                    /* select element for the next run - children first */
                    /* child exception for leafs, leaflists and anyxml without children */
                    if (elem->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) {
                        next = NULL;
                    } else {
                        next = elem->child;
                    }
                    if (!next) {
trim_dfs_nextsibling:
                        /* no children */
                        if (elem == subroot) {
                            /* we are done, (START) has no children */
                            break;
                        }
                        /* try siblings */
                        next = elem->next;
                    }
                    while (!next) {
                        /* parent is already processed, go to its sibling */
                        elem = elem->parent;
                        /* no siblings, go back through parents */
                        if (elem->parent == subroot->parent) {
                            /* we are done, no next element to process */
                            break;
                        }
                        next = elem->next;
                    }
                }
            }
            if (!flag) {
                /* only default nodes in subtree, do not print the container */
                return 0;
            }
        }
    } else if (node->dflt && !(options & LYP_WD_MASK) && !(node->schema->flags & LYS_CONFIG_R)) {
        /* LYP_WD_EXPLICIT
         * - print only if it contains status data in its subtree */
        LY_TREE_DFS_BEGIN(node, next, elem) {
            if ((elem->schema->nodetype != LYS_CONTAINER) || ((struct lys_node_container *)elem->schema)->presence) {
                if (elem->schema->flags & LYS_CONFIG_R) {
                    flag = 1;
                    break;
                }
            }
            LY_TREE_DFS_END(node, next, elem)
        }
        if (!flag) {
            return 0;
        }
    } else if (node->dflt && node->schema->nodetype == LYS_CONTAINER && !(options & LYP_KEEPEMPTYCONT)) {
        /* avoid empty default containers */
        LY_TREE_DFS_BEGIN(node, next, elem) {
            if (elem->schema->nodetype != LYS_CONTAINER) {
                flag = 1;
                break;
            }
            LY_TREE_DFS_END(node, next, elem)
        }
        if (!flag) {
            return 0;
        }
    }

    return 1;
}

API int
lyd_node_should_print(const struct lyd_node *node, int options)
{
    struct lys_node *scase, *sparent;
    struct lyd_node *first;

    if (!lyd_wd_toprint(node, options)) {
        /* wd says do not print, but make exception for direct descendants of case nodes without other printable nodes */
        for (sparent = lys_parent(node->schema); sparent && (sparent->nodetype == LYS_USES); sparent = lys_parent(sparent));
        if (!sparent || (sparent->nodetype != LYS_CASE)) {
            /* parent not a case */
            return 0;
        }
        scase = sparent;

        for (sparent = lys_parent(scase); sparent && (sparent->nodetype == LYS_USES); sparent = lys_parent(sparent));
        if (!sparent || (sparent->nodetype != LYS_CHOICE)) {
            /* weird */
            LOGINT(lyd_node_module(node)->ctx);
            return 0;
        }
        if (((struct lys_node_choice *)sparent)->dflt == scase) {
            /* this is a default case, respect the previous original toprint flag */
            return 0;
        }

        /* try to find a sibling that will be printed */
        for (first = node->prev; first->prev->next; first = first->prev);
        LY_TREE_FOR(first, first) {
            if (first == node) {
                /* skip this node */
                continue;
            }

            /* find schema parent, whether it is the same case */
            for (sparent = lys_parent(first->schema); sparent && (sparent->nodetype == LYS_USES); sparent = lys_parent(sparent));
            if ((sparent == scase) && lyd_wd_toprint(first, options)) {
                /* this other node will be printed, we do not have to print the current one */
                return 0;
            }
        }

        /* there is no case child that will be printed, print this node */
        return 1;
    }

    return 1;
}
