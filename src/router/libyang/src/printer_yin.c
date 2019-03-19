/**
 * @file printer_yin.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief YIN printer for libyang data model structure
 *
 * Copyright (c) 2016 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "common.h"
#include "context.h"
#include "printer.h"
#include "tree_schema.h"
#include "xml_internal.h"

#define INDENT ""
#define LEVEL (level*2)

static void yin_print_snode(struct lyout *out, int level, const struct lys_node *node, int mask);
static void yin_print_extension_instances(struct lyout *out, int level, const struct lys_module *module,
                                          LYEXT_SUBSTMT substmt, uint8_t substmt_index,
                                          struct lys_ext_instance **ext, unsigned int count);

/* endflag :
 * -1: />  - empty element
 *  0:     - no end
 *  1: >   - element with children
 */
static void
yin_print_open(struct lyout *out, int level, const char *elem_prefix, const char *elem_name,
               const char *attr_name, const char *attr_value, int endflag)
{
    if (elem_prefix) {
        ly_print(out, "%*s<%s:%s", LEVEL, INDENT, elem_prefix, elem_name);
    } else {
        ly_print(out, "%*s<%s", LEVEL, INDENT, elem_name);
    }

    if (attr_name) {
        ly_print(out, " %s=\"", attr_name);
        lyxml_dump_text(out, attr_value, LYXML_DATA_ATTR);
        ly_print(out, "\"%s", endflag == -1 ? "/>\n" : endflag == 1 ? ">\n" : "");
    } else if (endflag) {
        ly_print(out, endflag == -1 ? "/>\n" : ">\n");
    }
}

/*
 * endflag:
 * 0: />           - closing empty element
 * 1: </elem_name> - closing element with children
 */
static void
yin_print_close(struct lyout *out, int level, const char *elem_prefix, const char *elem_name, int endflag)
{
    if (endflag) {
        if (elem_prefix) {
            ly_print(out, "%*s</%s:%s>\n", LEVEL, INDENT, elem_prefix, elem_name);
        } else {
            ly_print(out, "%*s</%s>\n", LEVEL, INDENT, elem_name);
        }
    } else {
        ly_print(out, "/>\n");
    }
}

/*
 * par_close_flag
 * 0 - parent not yet closed, printing >\n, setting flag to 1
 * 1 or NULL - parent already closed, do nothing
 */
static void
yin_print_close_parent(struct lyout *out, int *par_close_flag)
{
    if (par_close_flag && !(*par_close_flag)) {
        (*par_close_flag) = 1;
        ly_print(out, ">\n");
    }
}

static void
yin_print_arg(struct lyout *out, int level, const char *arg, const char *text)
{
    ly_print(out, "%*s<%s>", LEVEL, INDENT, arg);
    lyxml_dump_text(out, text, LYXML_DATA_ELEM);
    ly_print(out, "</%s>\n", arg);
}

static void
yin_print_substmt(struct lyout *out, int level, LYEXT_SUBSTMT substmt, uint8_t substmt_index, const char *text,
                   const struct lys_module *module, struct lys_ext_instance **ext, unsigned int ext_size)
{
    int i, content = 0;

    if (!text) {
        /* nothing to print */
        return;
    }

    if (ext_substmt_info[substmt].flags & SUBST_FLAG_YIN) {
        content = 1;
        yin_print_open(out, level, NULL, ext_substmt_info[substmt].name,
                       NULL, NULL, content);
    } else {
        yin_print_open(out, level, NULL, ext_substmt_info[substmt].name,
                       ext_substmt_info[substmt].arg, text, content);
    }
    /* extensions */
    i = -1;
    do {
        i = lys_ext_iter(ext, ext_size, i + 1, substmt);
    } while (i != -1 && ext[i]->insubstmt_index != substmt_index);
    if (i != -1) {
        yin_print_close_parent(out, &content);
        do {
            yin_print_extension_instances(out, level + 1, module, substmt, substmt_index, &ext[i], 1);
            do {
                i = lys_ext_iter(ext, ext_size, i + 1, substmt);
            } while (i != -1 && ext[i]->insubstmt_index != substmt_index);
        } while (i != -1);
    }

    /* argument as yin-element */
    if (ext_substmt_info[substmt].flags & SUBST_FLAG_YIN) {
        yin_print_arg(out, level + 1, ext_substmt_info[substmt].arg, text);
    }

    yin_print_close(out, level, NULL, ext_substmt_info[substmt].name, content);
}

static void
yin_print_iffeature(struct lyout *out, int level, const struct lys_module *module, struct lys_iffeature *iffeature)
{
    ly_print(out, "%*s<if-feature name=\"", LEVEL, INDENT);
    ly_print_iffeature(out, module, iffeature, 0);

    /* extensions */
    if (iffeature->ext_size) {
        ly_print(out, "\">\n");
        yin_print_extension_instances(out, level + 1, module, LYEXT_SUBSTMT_SELF, 0, iffeature->ext, iffeature->ext_size);
        ly_print(out, "%*s</if-feature>\n", LEVEL, INDENT);
    } else {
        ly_print(out, "\"/>\n");
    }
}

/*
 * Covers:
 * extension (instances), if-features, config, mandatory, status, description, reference
 */
#define SNODE_COMMON_EXT    0x01
#define SNODE_COMMON_IFF    0x02
#define SNODE_COMMON_CONFIG 0x04
#define SNODE_COMMON_MAND   0x08
#define SNODE_COMMON_STATUS 0x10
#define SNODE_COMMON_DSC    0x20
#define SNODE_COMMON_REF    0x40
static void
yin_print_snode_common(struct lyout *out, int level, const struct lys_node *node, const struct lys_module *module,
                       int *par_close_flag, int mask)
{
    int i;
    const char *status = NULL;

    /* extensions */
    if ((mask & SNODE_COMMON_EXT) && node->ext_size) {
        yin_print_close_parent(out, par_close_flag);
        yin_print_extension_instances(out, level, module, LYEXT_SUBSTMT_SELF, 0, node->ext, node->ext_size);
    }

    /* if-features */
    if (mask & SNODE_COMMON_IFF) {
        for (i = 0; i < node->iffeature_size; ++i) {
            yin_print_close_parent(out, par_close_flag);
            yin_print_iffeature(out, level, module, &node->iffeature[i]);
        }
    }

    /* config */
    if (mask & SNODE_COMMON_CONFIG) {
        /* get info if there is an extension for the config statement */
        i = lys_ext_iter(node->ext, node->ext_size, 0, LYEXT_SUBSTMT_CONFIG);

        if (lys_parent(node)) {
            if ((node->flags & LYS_CONFIG_SET) || i != -1) {
                /* print config when it differs from the parent or if it has an extension instance ... */
                if (node->flags & LYS_CONFIG_W) {
                    yin_print_close_parent(out, par_close_flag);
                    yin_print_substmt(out, level, LYEXT_SUBSTMT_CONFIG, 0, "true",
                                      module, node->ext, node->ext_size);
                } else if (node->flags & LYS_CONFIG_R) {
                    yin_print_close_parent(out, par_close_flag);
                    yin_print_substmt(out, level, LYEXT_SUBSTMT_CONFIG, 0, "false",
                                      module, node->ext, node->ext_size);
                }
            }
        } else if (node->flags & LYS_CONFIG_R) {
            /* ... or it's a top-level state node */
            yin_print_close_parent(out, par_close_flag);
            yin_print_substmt(out, level, LYEXT_SUBSTMT_CONFIG, 0, "false",
                              module, node->ext, node->ext_size);
        } else if (i != -1) {
            /* the config has an extension, so we have to print it */
            yin_print_close_parent(out, par_close_flag);
            yin_print_substmt(out, level, LYEXT_SUBSTMT_CONFIG, 0, "true",
                              module, node->ext, node->ext_size);
        }
    }

    /* mandatory */
    if ((mask & SNODE_COMMON_MAND) && (node->nodetype & (LYS_LEAF | LYS_CHOICE | LYS_ANYDATA))) {
        if (node->flags & LYS_MAND_TRUE) {
            yin_print_close_parent(out, par_close_flag);
            yin_print_substmt(out, level, LYEXT_SUBSTMT_MANDATORY, 0, "true",
                              module, node->ext, node->ext_size);
        } else if (node->flags & LYS_MAND_FALSE) {
            yin_print_close_parent(out, par_close_flag);
            yin_print_substmt(out, level, LYEXT_SUBSTMT_MANDATORY, 0, "false",
                              module, node->ext, node->ext_size);
        }
    }

    /* status */
    if (mask & SNODE_COMMON_STATUS) {
        if (node->flags & LYS_STATUS_CURR) {
            yin_print_close_parent(out, par_close_flag);
            status = "current";
        } else if (node->flags & LYS_STATUS_DEPRC) {
            yin_print_close_parent(out, par_close_flag);
            status = "deprecated";
        } else if (node->flags & LYS_STATUS_OBSLT) {
            yin_print_close_parent(out, par_close_flag);
            status = "obsolete";
        }
        yin_print_substmt(out, level, LYEXT_SUBSTMT_STATUS, 0, status, module, node->ext, node->ext_size);
    }

    /* description */
    if ((mask & SNODE_COMMON_DSC) && node->dsc) {
        yin_print_close_parent(out, par_close_flag);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_DESCRIPTION, 0, node->dsc,
                           module, node->ext, node->ext_size);
    }

    /* reference */
    if ((mask & SNODE_COMMON_REF) && node->ref) {
        yin_print_close_parent(out, par_close_flag);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_REFERENCE, 0, node->ref,
                          module, node->ext, node->ext_size);
    }
}

static void
yin_print_extension(struct lyout *out, int level, const struct lys_ext *ext)
{
    int close = 0, close2 = 0, i;

    yin_print_open(out, level, NULL, "extension", "name", ext->name, close);
    level++;

    yin_print_snode_common(out, level, (struct lys_node *)ext, ext->module, &close,
                           SNODE_COMMON_EXT);

    if (ext->argument) {
        yin_print_close_parent(out, &close);
        yin_print_open(out, level, NULL, "argument", "name", ext->argument, close2);
        i = -1;
        while ((i = lys_ext_iter(ext->ext, ext->ext_size, i + 1, LYEXT_SUBSTMT_ARGUMENT)) != -1) {
            yin_print_close_parent(out, &close2);
            yin_print_extension_instances(out, level + 1, ext->module, LYEXT_SUBSTMT_ARGUMENT, 0, &ext->ext[i], 1);
        }
        if ((ext->flags & LYS_YINELEM) || lys_ext_iter(ext->ext, ext->ext_size, 0, LYEXT_SUBSTMT_YINELEM) != -1) {
            yin_print_close_parent(out, &close2);
            yin_print_substmt(out, level + 1, LYEXT_SUBSTMT_YINELEM, 0,
                              (ext->flags & LYS_YINELEM) ? "true" : "false", ext->module, ext->ext, ext->ext_size);
        }
        yin_print_close(out, level, NULL, "argument", close2);
    }

    yin_print_snode_common(out, level, (struct lys_node *)ext, ext->module, &close,
                           SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);

    level--;
    yin_print_close(out, level, NULL, "extension", close);
}

static void
yin_print_restr(struct lyout *out, int level, const struct lys_module *module, const struct lys_restr *restr,
                    int *flag)
{
    if (restr->ext_size) {
        yin_print_close_parent(out, flag);
        yin_print_extension_instances(out, level, module, LYEXT_SUBSTMT_SELF, 0,
                                      restr->ext, restr->ext_size);
    }
    if (restr->expr[0] == 0x15) {
        /* special byte value in pattern's expression: 0x15 - invert-match, 0x06 - match */
        yin_print_close_parent(out, flag);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_MODIFIER, 0, "invert-match", module,
                          restr->ext, restr->ext_size);
    }
    if (restr->emsg != NULL) {
        yin_print_close_parent(out, flag);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_ERRMSG, 0, restr->emsg,
                          module, restr->ext, restr->ext_size);
    }
    if (restr->eapptag != NULL) {
        yin_print_close_parent(out, flag);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_ERRTAG, 0, restr->eapptag,
                          module, restr->ext, restr->ext_size);
    }
    if (restr->dsc != NULL) {
        yin_print_close_parent(out, flag);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_DESCRIPTION, 0, restr->dsc,
                          module, restr->ext, restr->ext_size);
    }
    if (restr->ref != NULL) {
        yin_print_close_parent(out, flag);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_REFERENCE, 0, restr->ref,
                          module, restr->ext, restr->ext_size);
    }
}

/* covers length, range, pattern*/
static void
yin_print_typerestr(struct lyout *out, int level, const struct lys_module *module,
                    const struct lys_restr *restr, const char *elem_name)
{
    int content = 0;
    int pattern = 0;

    if (restr->expr[0] == 0x06 || restr->expr[0] == 0x15) {
        pattern = 1;
    }

    yin_print_open(out, level, NULL, elem_name, "value", pattern ? &restr->expr[1] : restr->expr , content);
    yin_print_restr(out, level + 1, module, restr, &content);
    yin_print_close(out, level, NULL, elem_name, content);
}

static void
yin_print_feature(struct lyout *out, int level, const struct lys_feature *feat)
{
    int close = 0;

    yin_print_open(out, level, NULL, "feature", "name", feat->name, close);
    yin_print_snode_common(out, level + 1, (struct lys_node *)feat, feat->module, &close, SNODE_COMMON_EXT |
                            SNODE_COMMON_IFF | SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);
    yin_print_close(out, level, NULL, "feature", close);
}

static void
yin_print_when(struct lyout *out, int level, const struct lys_module *module, const struct lys_when *when)
{
    int flag = 0;
    const char *str;

    str = transform_json2schema(module, when->cond);
    if (!str) {
        ly_print(out, "(!error!)");
        return;
    }

    ly_print(out, "%*s<when condition=\"", LEVEL, INDENT);
    lyxml_dump_text(out, str, LYXML_DATA_ATTR);
    ly_print(out, "\"");
    lydict_remove(module->ctx, str);

    level++;

    if (when->ext_size) {
        /* extension is stored in lys_when incompatible with lys_node, so we cannot use yang_print_snode_common() */
        yin_print_close_parent(out, &flag);
        yin_print_extension_instances(out, level, module, LYEXT_SUBSTMT_SELF, 0, when->ext, when->ext_size);
    }
    if (when->dsc != NULL) {
        yin_print_close_parent(out, &flag);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_DESCRIPTION, 0, when->dsc,
                          module, when->ext, when->ext_size);
    }
    if (when->ref != NULL) {
        yin_print_close_parent(out, &flag);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_REFERENCE, 0, when->ref,
                          module, when->ext, when->ext_size);
    }

    level--;
    yin_print_close(out, level, NULL, "when", flag);
}

static void
yin_print_unsigned(struct lyout *out, int level, LYEXT_SUBSTMT substmt, uint8_t substmt_index,
                   const struct lys_module *module, struct lys_ext_instance **ext, unsigned int ext_size,
                   unsigned int attr_value)
{
    char *str;

    if (asprintf(&str, "%u", attr_value) == -1) {
        LOGMEM(module->ctx);
    } else {
        yin_print_substmt(out, level, substmt, substmt_index, str, module, ext, ext_size);
        free(str);
    }
}

static void
yin_print_signed(struct lyout *out, int level, LYEXT_SUBSTMT substmt, uint8_t substmt_index,
                 const struct lys_module *module, struct lys_ext_instance **ext, unsigned int ext_size,
                 signed int attr_value)
{
    char *str;

    if (asprintf(&str, "%d", attr_value) == -1) {
        LOGMEM(module->ctx);
    } else {
        yin_print_substmt(out, level, substmt, substmt_index, str, module, ext, ext_size);
        free(str);
    }
}

static void
yin_print_type(struct lyout *out, int level, const struct lys_module *module, const struct lys_type *type)
{
    unsigned int i;
    int content = 0, content2 = 0;
    const char *str;
    char *s;
    struct lys_module *mod;

    if (!lys_type_is_local(type)) {
        ly_print(out, "%*s<type name=\"%s:%s\"", LEVEL, INDENT,
                 transform_module_name2import_prefix(module, lys_main_module(type->der->module)->name), type->der->name);
    } else {
        yin_print_open(out, level, NULL, "type", "name", type->der->name, content);
    }
    level++;

    /* extensions */
    if (type->ext_size) {
        yin_print_close_parent(out, &content);
        yin_print_extension_instances(out, level, module, LYEXT_SUBSTMT_SELF, 0, type->ext, type->ext_size);
    }


    switch (type->base) {
    case LY_TYPE_BINARY:
        if (type->info.binary.length) {
            yin_print_close_parent(out, &content);
            yin_print_typerestr(out, level, module, type->info.binary.length, "length");
        }
        break;
    case LY_TYPE_BITS:
        for (i = 0; i < type->info.bits.count; ++i) {
            content2 = 0;
            yin_print_close_parent(out, &content);
            yin_print_open(out, level, NULL, "bit", "name", type->info.bits.bit[i].name, content2);
            yin_print_snode_common(out, level + 1, (struct lys_node *)&type->info.bits.bit[i], module, &content2,
                                   SNODE_COMMON_EXT | SNODE_COMMON_IFF);
            if (!(type->info.bits.bit[i].flags & LYS_AUTOASSIGNED)) {
                yin_print_close_parent(out, &content2);
                yin_print_unsigned(out, level + 1, LYEXT_SUBSTMT_POSITION, 0, module,
                                   type->info.bits.bit[i].ext, type->info.bits.bit[i].ext_size,
                                   type->info.bits.bit[i].pos);
            }
            yin_print_snode_common(out, level + 1, (struct lys_node *)&type->info.bits.bit[i], module, &content2,
                                   SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);
            yin_print_close(out, level, NULL, "bit", content2);
        }
        break;
    case LY_TYPE_DEC64:
        if (!type->der->type.der) {
            yin_print_close_parent(out, &content);
            yin_print_unsigned(out, level, LYEXT_SUBSTMT_DIGITS, 0, module,
                               type->ext, type->ext_size, type->info.dec64.dig);
        }
        if (type->info.dec64.range) {
            yin_print_close_parent(out, &content);
            yin_print_typerestr(out, level, module, type->info.dec64.range, "range");
        }
        break;
    case LY_TYPE_ENUM:
        for (i = 0; i < type->info.enums.count; i++) {
            content2 = 0;
            yin_print_close_parent(out, &content);
            yin_print_open(out, level, NULL, "enum", "name", type->info.enums.enm[i].name, content2);
            yin_print_snode_common(out, level + 1, (struct lys_node *)&type->info.enums.enm[i], module, &content2,
                                   SNODE_COMMON_EXT | SNODE_COMMON_IFF);
            if (!(type->info.enums.enm[i].flags & LYS_AUTOASSIGNED)) {
                yin_print_close_parent(out, &content2);
                yin_print_signed(out, level + 1, LYEXT_SUBSTMT_VALUE, 0, module,
                                 type->info.enums.enm[i].ext, type->info.enums.enm[i].ext_size,
                                 type->info.enums.enm[i].value);
            }
            yin_print_snode_common(out, level + 1, (struct lys_node *)&type->info.enums.enm[i], module, &content2,
                                   SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);
            yin_print_close(out, level, NULL, "enum", content2);
        }
        break;
    case LY_TYPE_IDENT:
        if (type->info.ident.count) {
            yin_print_close_parent(out, &content);
            for (i = 0; i < type->info.ident.count; ++i) {
                mod = lys_main_module(type->info.ident.ref[i]->module);
                if (lys_main_module(module) == mod) {
                    yin_print_substmt(out, level, LYEXT_SUBSTMT_BASE, 0, type->info.ident.ref[i]->name,
                                      module, type->info.ident.ref[i]->ext, type->info.ident.ref[i]->ext_size);
                } else {
                    if (asprintf(&s, "%s:%s", transform_module_name2import_prefix(module, mod->name),
                                 type->info.ident.ref[i]->name) == -1) {
                        LOGMEM(module->ctx);
                    } else {
                        yin_print_substmt(out, level, LYEXT_SUBSTMT_BASE, 0, s,
                                          module, type->info.ident.ref[i]->ext, type->info.ident.ref[i]->ext_size);
                        free(s);
                    }
                }
            }
        }
        break;
    case LY_TYPE_INST:
        if (type->info.inst.req == 1) {
            yin_print_close_parent(out, &content);
            yin_print_substmt(out, level, LYEXT_SUBSTMT_REQINSTANCE, 0, "true", module, type->ext, type->ext_size);
        } else if (type->info.inst.req == -1) {
            yin_print_close_parent(out, &content);
            yin_print_substmt(out, level, LYEXT_SUBSTMT_REQINSTANCE, 0, "false", module, type->ext, type->ext_size);
        }
        break;
    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_UINT64:
        if (type->info.num.range) {
            yin_print_close_parent(out, &content);
            yin_print_typerestr(out, level, module, type->info.num.range, "range");
        }
        break;
    case LY_TYPE_LEAFREF:
        if (ly_strequal(type->der->name, "leafref", 0)) {
            yin_print_close_parent(out, &content);
            str = transform_json2schema(module, type->info.lref.path);
            yin_print_substmt(out, level, LYEXT_SUBSTMT_PATH, 0, str, module, type->ext, type->ext_size);
            lydict_remove(module->ctx, str);
        }
        if (type->info.lref.req == 1) {
            yin_print_close_parent(out, &content);
            yin_print_substmt(out, level, LYEXT_SUBSTMT_REQINSTANCE, 0, "true", module, type->ext, type->ext_size);
        } else if (type->info.lref.req == -1) {
            yin_print_close_parent(out, &content);
            yin_print_substmt(out, level, LYEXT_SUBSTMT_REQINSTANCE, 0, "false", module, type->ext, type->ext_size);
        }
        break;
    case LY_TYPE_STRING:
        if (type->info.str.length) {
            yin_print_close_parent(out, &content);
            yin_print_typerestr(out, level, module, type->info.str.length, "length");
        }
        for (i = 0; i < type->info.str.pat_count; i++) {
            yin_print_close_parent(out, &content);
            yin_print_typerestr(out, level, module, &type->info.str.patterns[i], "pattern");
        }
        break;
    case LY_TYPE_UNION:
        for (i = 0; i < type->info.uni.count; ++i) {
            yin_print_close_parent(out, &content);
            yin_print_type(out, level, module, &type->info.uni.types[i]);
        }
        break;
    default:
        /* other types do not have substatements */
        break;
    }

    level--;
    yin_print_close(out, level, NULL, "type", content);
}

static void
yin_print_must(struct lyout *out, int level, const struct lys_module *module, const struct lys_restr *must)
{
    const char *str;
    int content = 0;

    str = transform_json2schema(module, must->expr);
    if (!str) {
        ly_print(out, "(!error!)");
        return;
    }

    ly_print(out, "%*s<must condition=\"", LEVEL, INDENT);
    lyxml_dump_text(out, str, LYXML_DATA_ATTR);
    ly_print(out, "\"");
    lydict_remove(module->ctx, str);

    yin_print_restr(out, level + 1, module, must, &content);
    yin_print_close(out, level, NULL, "must", content);
}

static void
yin_print_unique(struct lyout *out, int level, const struct lys_module *module, const struct lys_unique *uniq)
{
    int i;
    const char *str;

    ly_print(out, "%*s<unique tag=\"", LEVEL, INDENT);
    for (i = 0; i < uniq->expr_size; i++) {
        str = transform_json2schema(module, uniq->expr[i]);
        ly_print(out, "%s%s", str, i + 1 < uniq->expr_size ? " " : "");
        lydict_remove(module->ctx, str);
    }
    ly_print(out, "\"");
}

static void
yin_print_refine(struct lyout *out, int level, const struct lys_module *module, const struct lys_refine *refine)
{
    int i, content = 0;
    const char *str;

    str = transform_json2xml(module, refine->target_name, 0, NULL, NULL, NULL);
    yin_print_open(out, level, NULL, "refine", "target-node", str, content);
    lydict_remove(module->ctx, str);

    level++;
    yin_print_snode_common(out, level, (struct lys_node *)refine, module, &content,
                           SNODE_COMMON_EXT | SNODE_COMMON_IFF);
    for (i = 0; i < refine->must_size; ++i) {
        yin_print_close_parent(out, &content);
        yin_print_must(out, level, module, &refine->must[i]);
    }
    if (refine->target_type == LYS_CONTAINER && refine->mod.presence) {
        yin_print_close_parent(out, &content);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_PRESENCE, 0, refine->mod.presence,
                          module, refine->ext, refine->ext_size);
    }
    for (i = 0; i < refine->dflt_size; ++i) {
        yin_print_close_parent(out, &content);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_DEFAULT, i, refine->dflt[i],
                          module, refine->ext, refine->ext_size);
    }
    if (refine->flags & LYS_CONFIG_W) {
        yin_print_close_parent(out, &content);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_CONFIG, 0, "true", module, refine->ext, refine->ext_size);
    } else if (refine->flags & LYS_CONFIG_R) {
        yin_print_close_parent(out, &content);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_CONFIG, 0, "false", module, refine->ext, refine->ext_size);
    }
    if (refine->flags & LYS_MAND_TRUE) {
        yin_print_close_parent(out, &content);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_MANDATORY, 0, "true", module, refine->ext, refine->ext_size);
    } else if (refine->flags & LYS_MAND_FALSE) {
        yin_print_close_parent(out, &content);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_MANDATORY, 0, "false", module, refine->ext, refine->ext_size);
    }
    if (refine->target_type & (LYS_LIST | LYS_LEAFLIST)) {
        if (refine->flags & LYS_RFN_MINSET) {
            yin_print_close_parent(out, &content);
            yin_print_unsigned(out, level, LYEXT_SUBSTMT_MIN, 0, module, refine->ext, refine->ext_size,
                               refine->mod.list.min);
        }
        if (refine->flags & LYS_RFN_MAXSET) {
            yin_print_close_parent(out, &content);
            if (refine->mod.list.max) {
                yin_print_unsigned(out, level, LYEXT_SUBSTMT_MAX, 0, module, refine->ext, refine->ext_size,
                                   refine->mod.list.max);
            } else {
                yin_print_substmt(out, level, LYEXT_SUBSTMT_MAX, 0, "unbounded", module,
                                  refine->ext, refine->ext_size);
            }
        }
    }
    yin_print_snode_common(out, level, (struct lys_node *)refine, module, &content,
                           SNODE_COMMON_DSC | SNODE_COMMON_REF);
    level--;

    yin_print_close(out, level, NULL, "refine", content);
}

static void
yin_print_deviation(struct lyout *out, int level, const struct lys_module *module,
                    const struct lys_deviation *deviation)
{
    int i, j, p, content;
    const char *str;

    str = transform_json2schema(module, deviation->target_name);
    yin_print_open(out, level, NULL, "deviation", "target-node", str, 1);
    lydict_remove(module->ctx, str);

    level++;

    if (deviation->ext_size) {
        yin_print_extension_instances(out, level, module, LYEXT_SUBSTMT_SELF, 0, deviation->ext, deviation->ext_size);
    }
    yin_print_substmt(out, level, LYEXT_SUBSTMT_DESCRIPTION, 0, deviation->dsc,
                      module, deviation->ext, deviation->ext_size);
    yin_print_substmt(out, level, LYEXT_SUBSTMT_REFERENCE, 0, deviation->ref,
                      module, deviation->ext, deviation->ext_size);

    for (i = 0; i < deviation->deviate_size; ++i) {
        ly_print(out, "%*s<deviate value=", LEVEL, INDENT);
        if (deviation->deviate[i].mod == LY_DEVIATE_NO) {
            if (deviation->deviate[i].ext_size) {
                ly_print(out, "\"not-supported\">\n");
            } else {
                ly_print(out, "\"not-supported\"/>\n");
                continue;
            }
        } else if (deviation->deviate[i].mod == LY_DEVIATE_ADD) {
            ly_print(out, "\"add\">\n");
        } else if (deviation->deviate[i].mod == LY_DEVIATE_RPL) {
            ly_print(out, "\"replace\">\n");
        } else if (deviation->deviate[i].mod == LY_DEVIATE_DEL) {
            ly_print(out, "\"delete\">\n");
        }
        level++;

        /* extensions */
        if (deviation->deviate[i].ext_size) {
            yin_print_extension_instances(out, level, module, LYEXT_SUBSTMT_SELF, 0,
                                          deviation->deviate[i].ext, deviation->deviate[i].ext_size);
        }

        /* type */
        if (deviation->deviate[i].type) {
            yin_print_type(out, level, module, deviation->deviate[i].type);
        }

        /* units */
        yin_print_substmt(out, level, LYEXT_SUBSTMT_UNITS, 0, deviation->deviate[i].units, module,
                          deviation->deviate[i].ext, deviation->deviate[i].ext_size);

        /* must */
        for (j = 0; j < deviation->deviate[i].must_size; ++j) {
            yin_print_must(out, level, module, &deviation->deviate[i].must[j]);
        }

        /* unique */
        for (j = 0; j < deviation->deviate[i].unique_size; ++j) {
            yin_print_unique(out, level, module, &deviation->deviate[i].unique[j]);
            content = 0;
            /* unique's extensions */
            p = -1;
            do {
                p = lys_ext_iter(deviation->deviate[i].ext, deviation->deviate[i].ext_size,
                                      p + 1, LYEXT_SUBSTMT_UNIQUE);
            } while (p != -1 && deviation->deviate[i].ext[p]->insubstmt_index != j);
            if (p != -1) {
                yin_print_close_parent(out, &content);
                do {
                    yin_print_extension_instances(out, level + 1, module, LYEXT_SUBSTMT_UNIQUE, j,
                                                  &deviation->deviate[i].ext[p], 1);
                    do {
                        p = lys_ext_iter(deviation->deviate[i].ext, deviation->deviate[i].ext_size,
                                              p + 1, LYEXT_SUBSTMT_UNIQUE);
                    } while (p != -1 && deviation->deviate[i].ext[p]->insubstmt_index != j);
                } while (p != -1);
            }
            yin_print_close(out, level, NULL, "unique", content);
        }

        /* default */
        for (j = 0; j < deviation->deviate[i].dflt_size; ++j) {
            yin_print_substmt(out, level, LYEXT_SUBSTMT_DEFAULT, j, deviation->deviate[i].dflt[j], module,
                              deviation->deviate[i].ext, deviation->deviate[i].ext_size);
        }

        /* config */
        if (deviation->deviate[i].flags & LYS_CONFIG_W) {
            yin_print_substmt(out, level, LYEXT_SUBSTMT_CONFIG, 0, "true", module,
                              deviation->deviate->ext, deviation->deviate[i].ext_size);
        } else if (deviation->deviate[i].flags & LYS_CONFIG_R) {
            yin_print_substmt(out, level, LYEXT_SUBSTMT_CONFIG, 0, "false", module,
                               deviation->deviate->ext, deviation->deviate[i].ext_size);
        }

        /* mandatory */
        if (deviation->deviate[i].flags & LYS_MAND_TRUE) {
            yin_print_substmt(out, level, LYEXT_SUBSTMT_MANDATORY, 0, "true", module,
                              deviation->deviate[i].ext, deviation->deviate[i].ext_size);
        } else if (deviation->deviate[i].flags & LYS_MAND_FALSE) {
            yin_print_substmt(out, level, LYEXT_SUBSTMT_MANDATORY, 0, "false", module,
                              deviation->deviate[i].ext, deviation->deviate[i].ext_size);
        }

        /* min-elements */
        if (deviation->deviate[i].min_set) {
            yin_print_unsigned(out, level, LYEXT_SUBSTMT_MIN, 0, module,
                               deviation->deviate[i].ext, deviation->deviate[i].ext_size,
                               deviation->deviate[i].min);
        }

        /* max-elements */
        if (deviation->deviate[i].max_set) {
            if (deviation->deviate[i].max) {
                yin_print_unsigned(out, level, LYEXT_SUBSTMT_MAX, 0, module,
                                   deviation->deviate[i].ext, deviation->deviate[i].ext_size,
                                   deviation->deviate[i].max);
            } else {
                yin_print_substmt(out, level, LYEXT_SUBSTMT_MAX, 0, "unbounded", module,
                                  deviation->deviate[i].ext, deviation->deviate[i].ext_size);
            }
        }
        level--;

        yin_print_close(out, level, NULL, "deviate", 1);
    }

    level--;
    yin_print_close(out, level, NULL, "deviation", 1);
}

static void
yin_print_augment(struct lyout *out, int level, const struct lys_module *module,
                  const struct lys_node_augment *augment)
{
    struct lys_node *sub;
    const char *str;

    str = transform_json2schema(module, augment->target_name);
    yin_print_open(out, level, NULL, "augment", "target-node", str, 1);
    lydict_remove(module->ctx, str);
    level++;

    yin_print_snode_common(out, level, (struct lys_node *)augment, augment->module, NULL, SNODE_COMMON_EXT);
    if (augment->when) {
        yin_print_when(out, level, module, augment->when);
    }
    yin_print_snode_common(out, level, (struct lys_node *)augment, augment->module, NULL,
                           SNODE_COMMON_IFF | SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);


    LY_TREE_FOR(augment->child, sub) {
        /* only our augment */
        if (sub->parent != (struct lys_node *)augment) {
            continue;
        }
        yin_print_snode(out, level, sub,
                        LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                        LYS_USES | LYS_ANYDATA | LYS_CASE | LYS_ACTION | LYS_NOTIF);
    }
    level--;

    yin_print_close(out, level, NULL, "augment", 1);
}

static void
yin_print_typedef(struct lyout *out, int level, const struct lys_module *module, const struct lys_tpdf *tpdf)
{
    const char *dflt;

    yin_print_open(out, level, NULL, "typedef", "name", tpdf->name, 1);
    level++;

    yin_print_snode_common(out, level, (struct lys_node *)tpdf, module, NULL, SNODE_COMMON_EXT);
    yin_print_type(out, level, module, &tpdf->type);
    if (tpdf->units) {
        yin_print_substmt(out, level, LYEXT_SUBSTMT_UNITS, 0, tpdf->units, module, tpdf->ext, tpdf->ext_size);
    }
    if (tpdf->dflt) {
        if (tpdf->flags & LYS_DFLTJSON) {
            assert(strchr(tpdf->dflt, ':'));
            if (!strncmp(tpdf->dflt, module->name, strchr(tpdf->dflt, ':') - tpdf->dflt)) {
                /* local module */
                dflt = lydict_insert(module->ctx, strchr(tpdf->dflt, ':') + 1, 0);
            } else {
                dflt = transform_json2schema(module, tpdf->dflt);
            }
        } else {
            dflt = tpdf->dflt;
        }
        yin_print_substmt(out, level, LYEXT_SUBSTMT_DEFAULT, 0, dflt, module, tpdf->ext, tpdf->ext_size);
        if (tpdf->flags & LYS_DFLTJSON) {
            lydict_remove(module->ctx, dflt);
        }
    }
    yin_print_snode_common(out, level, (struct lys_node *)tpdf, module, NULL,
                           SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);

    level--;
    yin_print_close(out, level, NULL, "typedef", 1);
}

static void
yin_print_identity(struct lyout *out, int level, const struct lys_ident *ident)
{
    int content = 0, i;
    struct lys_module *mod;
    char *str;

    yin_print_open(out, level, NULL, "identity", "name", ident->name, content);
    level++;

    yin_print_snode_common(out, level, (struct lys_node *)ident, ident->module, &content,
                           SNODE_COMMON_EXT | SNODE_COMMON_IFF);
    for (i = 0; i < ident->base_size; i++) {
        yin_print_close_parent(out, &content);
        mod = lys_main_module(ident->base[i]->module);
        if (lys_main_module(ident->module) == mod) {
            yin_print_substmt(out, level, LYEXT_SUBSTMT_BASE, i, ident->base[i]->name,
                              ident->module, ident->ext, ident->ext_size);
        } else {
            if (asprintf(&str, "%s:%s", transform_module_name2import_prefix(ident->module, mod->name), ident->base[i]->name) == -1) {
                LOGMEM(mod->ctx);
            } else {
                yin_print_substmt(out, level, LYEXT_SUBSTMT_BASE, i, str,
                                  ident->module, ident->ext, ident->ext_size);
                free(str);
            }
        }
    }
    yin_print_snode_common(out, level, (struct lys_node *)ident, ident->module, &content,
                           SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);

    level--;
    yin_print_close(out, level, NULL, "identity", content);
}

static void
yin_print_container(struct lyout *out, int level, const struct lys_node *node)
{
    int i, content = 0;
    struct lys_node *sub;
    struct lys_node_container *cont = (struct lys_node_container *)node;

    yin_print_open(out, level, NULL, "container", "name", node->name, content);
    level++;

    yin_print_snode_common(out, level, node, node->module, &content, SNODE_COMMON_EXT);
    if (cont->when) {
        yin_print_close_parent(out, &content);
        yin_print_when(out, level, node->module, cont->when);
    }

    for (i = 0; i < cont->iffeature_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_iffeature(out, level, node->module, &cont->iffeature[i]);
    }

    for (i = 0; i < cont->must_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_must(out, level, node->module, &cont->must[i]);
    }

    if (cont->presence) {
        yin_print_close_parent(out, &content);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_PRESENCE, 0, cont->presence,
                          node->module, node->ext, node->ext_size);
    }
    yin_print_snode_common(out, level, node, node->module, &content, SNODE_COMMON_CONFIG |
                           SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);

    for (i = 0; i < cont->tpdf_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_typedef(out, level, node->module, &cont->tpdf[i]);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_GROUPING);
    }
    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_ANYDATA );
    }
    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_ACTION);
    }
    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_NOTIF);
    }

    level--;
    yin_print_close(out, level, NULL, "container", content);
}

static void
yin_print_case(struct lyout *out, int level, const struct lys_node *node)
{
    int content = 0;
    struct lys_node *sub;
    struct lys_node_case *cas = (struct lys_node_case *)node;

    if (!(node->flags & LYS_IMPLICIT)) {
        yin_print_open(out, level, NULL, "case", "name", cas->name, content);
        level++;

        yin_print_snode_common(out, level, node, node->module, &content, SNODE_COMMON_EXT);
        if (cas->when) {
            yin_print_close_parent(out, &content);
            yin_print_when(out, level, node->module, cas->when);
        }
        yin_print_snode_common(out, level, node, node->module, &content,
                               SNODE_COMMON_IFF | SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);
    } else {
        content = 1;
    }

    /* print children */
    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub,
                        LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                        LYS_USES | LYS_ANYDATA);
    }

    if (node->flags & LYS_IMPLICIT) {
        /* do not print anything about the case, it was implicitely added by libyang */
        return;
    }

    level--;
    yin_print_close(out, level, NULL, "case", content);
}

static void
yin_print_choice(struct lyout *out, int level, const struct lys_node *node)
{
    int i, content = 0;
    struct lys_node *sub;
    struct lys_node_choice *choice = (struct lys_node_choice *)node;

    yin_print_open(out, level, NULL, "choice", "name", node->name, content);
    level++;

    yin_print_snode_common(out, level, node, node->module, &content, SNODE_COMMON_EXT);
    if (choice->when) {
        yin_print_close_parent(out, &content);
        yin_print_when(out, level, node->module, choice->when);
    }
    for (i = 0; i < choice->iffeature_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_iffeature(out, level, node->module, &choice->iffeature[i]);
    }
    if (choice->dflt) {
        yin_print_close_parent(out, &content);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_DEFAULT, 0, choice->dflt->name,
                          node->module, node->ext, node->ext_size);
    }

    yin_print_snode_common(out, level, node, node->module, &content, SNODE_COMMON_CONFIG | SNODE_COMMON_MAND |
                           SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub,
                        LYS_CHOICE |LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA | LYS_CASE);
    }

    level--;
    yin_print_close(out, level, NULL, "choice", content);
}

static void
yin_print_leaf(struct lyout *out, int level, const struct lys_node *node)
{
    int i;
    struct lys_node_leaf *leaf = (struct lys_node_leaf *)node;
    const char *dflt;

    yin_print_open(out, level, NULL, "leaf", "name", node->name, 1);
    level++;

    yin_print_snode_common(out, level, node, node->module, NULL, SNODE_COMMON_EXT);
    if (leaf->when) {
        yin_print_when(out, level, node->module, leaf->when);
    }
    for (i = 0; i < leaf->iffeature_size; i++) {
        yin_print_iffeature(out, level, node->module, &leaf->iffeature[i]);
    }
    yin_print_type(out, level, node->module, &leaf->type);
    yin_print_substmt(out, level, LYEXT_SUBSTMT_UNITS, 0, leaf->units,
                      node->module, node->ext, node->ext_size);
    for (i = 0; i < leaf->must_size; i++) {
        yin_print_must(out, level, node->module, &leaf->must[i]);
    }
    if (leaf->dflt) {
        if (leaf->flags & LYS_DFLTJSON) {
            assert(strchr(leaf->dflt, ':'));
            if (!strncmp(leaf->dflt, lys_node_module(node)->name, strchr(leaf->dflt, ':') - leaf->dflt)) {
                /* local module */
                dflt = lydict_insert(node->module->ctx, strchr(leaf->dflt, ':') + 1, 0);
            } else {
                dflt = transform_json2schema(node->module, leaf->dflt);
            }
        } else {
            dflt = leaf->dflt;
        }
        yin_print_substmt(out, level, LYEXT_SUBSTMT_DEFAULT, 0, dflt,
                          node->module, node->ext, node->ext_size);
        if (leaf->flags & LYS_DFLTJSON) {
            lydict_remove(node->module->ctx, dflt);
        }
    }
    yin_print_snode_common(out, level, node, node->module, NULL, SNODE_COMMON_CONFIG | SNODE_COMMON_MAND |
                           SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);

    level--;
    yin_print_close(out, level, NULL, "leaf", 1);
}

static void
yin_print_anydata(struct lyout *out, int level, const struct lys_node *node)
{
    int i, content = 0;
    struct lys_node_anydata *any = (struct lys_node_anydata *)node;
    const char *name;

    if (!lys_parent(node) && !strcmp(node->name, "config") && !strcmp(node->module->name, "ietf-netconf")) {
        /* node added by libyang, not actually in the model */
        return;
    }

    name = any->nodetype == LYS_ANYXML ? "anyxml" : "anydata";
    yin_print_open(out, level, NULL, name, "name", any->name, content);

    level++;
    yin_print_snode_common(out, level, node, node->module, &content, SNODE_COMMON_EXT);
    if (any->when) {
        yin_print_close_parent(out, &content);
        yin_print_when(out, level, node->module, any->when);
    }
    for (i = 0; i < any->iffeature_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_iffeature(out, level, node->module, &any->iffeature[i]);
    }
    for (i = 0; i < any->must_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_must(out, level, node->module, &any->must[i]);
    }
    yin_print_snode_common(out, level, node, node->module, &content, SNODE_COMMON_CONFIG | SNODE_COMMON_MAND |
                           SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);

    level--;
    yin_print_close(out, level, NULL, name, content);
}

static void
yin_print_leaflist(struct lyout *out, int level, const struct lys_node *node)
{
    int i;
    struct lys_node_leaflist *llist = (struct lys_node_leaflist *)node;
    const char *dflt;

    yin_print_open(out, level, NULL, "leaf-list", "name", node->name, 1);
    level++;

    yin_print_snode_common(out, level, node, node->module, NULL, SNODE_COMMON_EXT);
    if (llist->when) {
        yin_print_when(out, level, llist->module, llist->when);
    }
    for (i = 0; i < llist->iffeature_size; i++) {
        yin_print_iffeature(out, level, node->module, &llist->iffeature[i]);
    }
    yin_print_type(out, level, node->module, &llist->type);
    yin_print_substmt(out, level, LYEXT_SUBSTMT_UNITS, 0, llist->units,
                      node->module, node->ext, node->ext_size);
    for (i = 0; i < llist->must_size; i++) {
        yin_print_must(out, level, node->module, &llist->must[i]);
    }
    for (i = 0; i < llist->dflt_size; i++) {
        if (llist->flags & LYS_DFLTJSON) {
            assert(strchr(llist->dflt[i], ':'));
            if (!strncmp(llist->dflt[i], lys_node_module(node)->name, strchr(llist->dflt[i], ':') - llist->dflt[i])) {
                /* local module */
                dflt = lydict_insert(node->module->ctx, strchr(llist->dflt[i], ':') + 1, 0);
            } else {
                dflt = transform_json2schema(node->module, llist->dflt[i]);
            }
        } else {
            dflt = llist->dflt[i];
        }
        yin_print_substmt(out, level, LYEXT_SUBSTMT_DEFAULT, i, dflt,
                          node->module, node->ext, node->ext_size);
        if (llist->flags & LYS_DFLTJSON) {
            lydict_remove(node->module->ctx, dflt);
        }
    }
    yin_print_snode_common(out, level, node, node->module, NULL, SNODE_COMMON_CONFIG);
    if (llist->min > 0) {
        yin_print_unsigned(out, level, LYEXT_SUBSTMT_MIN, 0, node->module, node->ext, node->ext_size, llist->min);
    }
    if (llist->max > 0) {
        yin_print_unsigned(out, level, LYEXT_SUBSTMT_MAX, 0, node->module, node->ext, node->ext_size, llist->max);
    }
    if (llist->flags & LYS_USERORDERED) {
        yin_print_substmt(out, level, LYEXT_SUBSTMT_ORDEREDBY, 0, "user",
                          node->module, node->ext, node->ext_size);
    } else if (lys_ext_iter(node->ext, node->ext_size, 0, LYEXT_SUBSTMT_ORDEREDBY) != -1) {
        yin_print_substmt(out, level, LYEXT_SUBSTMT_ORDEREDBY, 0, "system",
                          node->module, node->ext, node->ext_size);
    }
    yin_print_snode_common(out, level, node, node->module, NULL,
                           SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);

    level--;
    yin_print_close(out, level, NULL, "leaf-list", 1);
}

static void
yin_print_list(struct lyout *out, int level, const struct lys_node *node)
{
    int i, p, content = 0, content2;
    struct lys_node *sub;
    struct lys_node_list *list = (struct lys_node_list *)node;

    yin_print_open(out, level, NULL, "list", "name", node->name, content);
    level++;

    yin_print_snode_common(out, level, node, node->module, &content, SNODE_COMMON_EXT);
    if (list->when) {
        yin_print_close_parent(out, &content);
        yin_print_when(out, level, list->module, list->when);
    }
    for (i = 0; i < list->iffeature_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_iffeature(out, level, node->module, &list->iffeature[i]);
    }
    for (i = 0; i < list->must_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_must(out, level, list->module, &list->must[i]);
    }
    if (list->keys_size) {
        yin_print_close_parent(out, &content);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_KEY, 0, list->keys_str,
                          node->module, node->ext, node->ext_size);
    }
    for (i = 0; i < list->unique_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_unique(out, level, node->module, &list->unique[i]);
        content2 = 0;
        /* unique's extensions */
        p = -1;
        do {
            p = lys_ext_iter(list->ext, list->ext_size, p + 1, LYEXT_SUBSTMT_UNIQUE);
        } while (p != -1 && list->ext[p]->insubstmt_index != i);
        if (p != -1) {
            yin_print_close_parent(out, &content2);
            do {
                yin_print_extension_instances(out, level + 1, list->module, LYEXT_SUBSTMT_UNIQUE, i, &list->ext[p], 1);
                do {
                    p = lys_ext_iter(list->ext, list->ext_size, p + 1, LYEXT_SUBSTMT_UNIQUE);
                } while (p != -1 && list->ext[p]->insubstmt_index != i);
            } while (p != -1);
        }
        yin_print_close(out, level, NULL, "unique", content2);
    }
    yin_print_snode_common(out, level, node, node->module, &content, SNODE_COMMON_CONFIG);
    if (list->min > 0) {
        yin_print_close_parent(out, &content);
        yin_print_unsigned(out, level, LYEXT_SUBSTMT_MIN, 0, node->module, node->ext, node->ext_size, list->min);
    }
    if (list->max > 0) {
        yin_print_close_parent(out, &content);
        yin_print_unsigned(out, level, LYEXT_SUBSTMT_MAX, 0, node->module, node->ext, node->ext_size, list->max);
    }
    if (list->flags & LYS_USERORDERED) {
        yin_print_close_parent(out, &content);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_ORDEREDBY, 0, "user",
                          node->module, node->ext, node->ext_size);
    } else if (lys_ext_iter(node->ext, node->ext_size, 0, LYEXT_SUBSTMT_ORDEREDBY) != -1) {
        yin_print_close_parent(out, &content);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_ORDEREDBY, 0, "system",
                          node->module, node->ext, node->ext_size);
    }
    yin_print_snode_common(out, level, node, node->module, &content,
                           SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);
    for (i = 0; i < list->tpdf_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_typedef(out, level, list->module, &list->tpdf[i]);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_GROUPING);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                        LYS_USES | LYS_ANYDATA);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_ACTION);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_NOTIF);
    }

    level--;
    yin_print_close(out, level, NULL, "list", content);
}

static void
yin_print_grouping(struct lyout *out, int level, const struct lys_node *node)
{
    int i, content = 0;
    struct lys_node *sub;
    struct lys_node_grp *grp = (struct lys_node_grp *)node;

    yin_print_open(out, level, NULL, "grouping", "name", node->name, content);
    level++;

    yin_print_snode_common(out, level, node, node->module, &content, SNODE_COMMON_EXT | SNODE_COMMON_STATUS |
                           SNODE_COMMON_DSC | SNODE_COMMON_REF);

    for (i = 0; i < grp->tpdf_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_typedef(out, level, node->module, &grp->tpdf[i]);
    }

    LY_TREE_FOR(node->child, sub) {
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_GROUPING);
    }

    LY_TREE_FOR(node->child, sub) {
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_ANYDATA);
    }

    LY_TREE_FOR(node->child, sub) {
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_ACTION);
    }

    LY_TREE_FOR(node->child, sub) {
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_NOTIF);
    }

    level--;
    yin_print_close(out, level, NULL, "grouping", content);
}

static void
yin_print_uses(struct lyout *out, int level, const struct lys_node *node)
{
    int i, content = 0;
    struct lys_node_uses *uses = (struct lys_node_uses *)node;
    struct lys_module *mod;

    ly_print(out, "%*s<uses name=\"", LEVEL, INDENT);
    if (node->child) {
        mod = lys_node_module(node->child);
        if (lys_node_module(node) != mod) {
            ly_print(out, "%s:", transform_module_name2import_prefix(node->module, mod->name));
        }
    }
    ly_print(out, "%s\"", uses->name);
    level++;

    yin_print_snode_common(out, level, node, node->module, &content, SNODE_COMMON_EXT);
    if (uses->when) {
        yin_print_close_parent(out, &content);
        yin_print_when(out, level, node->module, uses->when);
    }
    yin_print_snode_common(out, level, node, node->module, &content, SNODE_COMMON_IFF |
                           SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);
    for (i = 0; i < uses->refine_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_refine(out, level, node->module, &uses->refine[i]);
    }
    for (i = 0; i < uses->augment_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_augment(out, level, node->module, &uses->augment[i]);
    }

    level--;
    yin_print_close(out, level, NULL, "uses", content);
}

static void
yin_print_input_output(struct lyout *out, int level, const struct lys_node *node)
{
    int i;
    struct lys_node *sub;
    struct lys_node_inout *inout = (struct lys_node_inout *)node;

    yin_print_open(out, level, NULL, inout->nodetype == LYS_INPUT ? "input" : "output", NULL, NULL, 1);
    level++;

    if (inout->ext_size) {
        yin_print_extension_instances(out, level, node->module, LYEXT_SUBSTMT_SELF, 0, inout->ext, inout->ext_size);
    }
    for (i = 0; i < inout->must_size; i++) {
        yin_print_must(out, level, node->module, &inout->must[i]);
    }
    for (i = 0; i < inout->tpdf_size; i++) {
        yin_print_typedef(out, level, node->module, &inout->tpdf[i]);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_snode(out, level, sub, LYS_GROUPING);
    }
    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_snode(out, level, sub, LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                        LYS_USES | LYS_ANYDATA);
    }
    level--;

    yin_print_close(out, level, NULL, (inout->nodetype == LYS_INPUT ? "input" : "output"), 1);
}

static void
yin_print_rpc_action(struct lyout *out, int level, const struct lys_node *node)
{
    int i, content = 0;
    struct lys_node *sub;
    struct lys_node_rpc_action *rpc = (struct lys_node_rpc_action *)node;

    yin_print_open(out, level, NULL, (node->nodetype == LYS_RPC ? "rpc" : "action"), "name", node->name, content);
    level++;

    yin_print_snode_common(out, level, node, node->module, &content, SNODE_COMMON_EXT | SNODE_COMMON_IFF |
                           SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);

    for (i = 0; i < rpc->tpdf_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_typedef(out, level, node->module, &rpc->tpdf[i]);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if ((sub->parent != node) || (((sub->nodetype & (LYS_INPUT | LYS_OUTPUT)) && (sub->flags & LYS_IMPLICIT)))) {
            continue;
        }
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_INPUT | LYS_OUTPUT | LYS_GROUPING);
    }

    level--;
    yin_print_close(out, level, NULL, (node->nodetype == LYS_RPC ? "rpc" : "action"), content);
}

static void
yin_print_notif(struct lyout *out, int level, const struct lys_node *node)
{
    int i, content = 0;
    struct lys_node *sub;
    struct lys_node_notif *notif = (struct lys_node_notif *)node;

    yin_print_open(out, level, NULL, "notification", "name", node->name, content);
    level++;

    yin_print_snode_common(out, level, node, node->module, &content, SNODE_COMMON_EXT | SNODE_COMMON_IFF);
    for (i = 0; i < notif->must_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_must(out, level, node->module, &notif->must[i]);
    }
    yin_print_snode_common(out, level, node, node->module, &content,
                           SNODE_COMMON_STATUS | SNODE_COMMON_DSC | SNODE_COMMON_REF);
    for (i = 0; i < notif->tpdf_size; i++) {
        yin_print_close_parent(out, &content);
        yin_print_typedef(out, level, node->module, &notif->tpdf[i]);
    }

    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub, LYS_GROUPING);
    }
    LY_TREE_FOR(node->child, sub) {
        /* augments */
        if (sub->parent != node) {
            continue;
        }
        yin_print_close_parent(out, &content);
        yin_print_snode(out, level, sub,
                         LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                         LYS_USES | LYS_ANYDATA);
    }

    level--;
    yin_print_close(out, level, NULL, "notification", content);
}

static void
yin_print_snode(struct lyout *out, int level, const struct lys_node *node, int mask)
{
    switch (node->nodetype & mask) {
    case LYS_CONTAINER:
        yin_print_container(out, level, node);
        break;
    case LYS_CHOICE:
        yin_print_choice(out, level, node);
        break;
    case LYS_LEAF:
        yin_print_leaf(out, level, node);
        break;
    case LYS_LEAFLIST:
        yin_print_leaflist(out, level, node);
        break;
    case LYS_LIST:
        yin_print_list(out, level, node);
        break;
    case LYS_USES:
        yin_print_uses(out, level, node);
        break;
    case LYS_GROUPING:
        yin_print_grouping(out, level, node);
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        yin_print_anydata(out, level, node);
        break;
    case LYS_CASE:
        yin_print_case(out, level, node);
        break;
    case LYS_RPC:
    case LYS_ACTION:
        yin_print_rpc_action(out, level, node);
        break;
    case LYS_INPUT:
    case LYS_OUTPUT:
        yin_print_input_output(out, level, node);
        break;
    case LYS_NOTIF:
        yin_print_notif(out, level, node);
        break;
    default:
        break;
    }
}

static void
yin_print_revision(struct lyout *out, int level, const struct lys_module *module, const struct lys_revision *rev)
{
    if (rev->dsc || rev->ref || rev->ext_size) {
        yin_print_open(out, level, NULL, "revision", "date", rev->date, 1);
        level++;
        yin_print_extension_instances(out, level, module, LYEXT_SUBSTMT_SELF, 0, rev->ext, rev->ext_size);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_DESCRIPTION, 0, rev->dsc, module, rev->ext, rev->ext_size);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_REFERENCE, 0, rev->ref, module, rev->ext, rev->ext_size);
        level--;
        yin_print_close(out, level, NULL, "revision", 1);
    } else {
        yin_print_open(out, level, NULL, "revision", "date", rev->date, -1);
    }
}

static void
yin_print_xmlns(struct lyout *out, const struct lys_module *module)
{
    unsigned int i, lvl;

    if (module->type) {
        lvl = 11;
    } else {
        lvl = 8;
    }

    ly_print(out, "%*sxmlns=\"%s\"", lvl, INDENT, LY_NSYIN);
    if (!module->type) {
        ly_print(out, "\n%*sxmlns:%s=\"%s\"", lvl, INDENT, module->prefix, module->ns);
    } else {
        ly_print(out, "\n%*sxmlns:%s=\"%s\"", lvl, INDENT, module->prefix, ((struct lys_submodule*)module)->belongsto->ns);
    }
    for (i = 0; i < module->imp_size; ++i) {
        ly_print(out, "\n%*sxmlns:%s=\"%s\"", lvl, INDENT, module->imp[i].prefix, module->imp[i].module->ns);
    }
}

static int
yin_print_model_(struct lyout *out, int level, const struct lys_module *module)
{
    unsigned int i;
    int p;
    struct lys_node *node;

    if (module->deviated == 1) {
        ly_print(out, "<!-- DEVIATED -->\n");
    }

    /* (sub)module-header-stmts */
    if (module->type) {
        ly_print(out, "%*s<submodule name=\"%s\"\n", LEVEL, INDENT, module->name);
        yin_print_xmlns(out, module);
        ly_print(out, ">\n");

        level++;
        if (module->version) {
            yin_print_substmt(out, level, LYEXT_SUBSTMT_VERSION, 0, module->version == LYS_VERSION_1_1 ? "1.1" : "1",
                              module, module->ext, module->ext_size);
        }
        yin_print_open(out, level, NULL, "belongs-to", "module", ((struct lys_submodule *)module)->belongsto->name, 1);
        p = -1;
        while ((p = lys_ext_iter(module->ext, module->ext_size, p + 1, LYEXT_SUBSTMT_BELONGSTO)) != -1) {
            yin_print_extension_instances(out, level + 1, module, LYEXT_SUBSTMT_BELONGSTO, 0, &module->ext[p], 1);
        }
        yin_print_substmt(out, level + 1, LYEXT_SUBSTMT_PREFIX, 0, module->prefix,
                          module, module->ext, module->ext_size);
        yin_print_close(out, level, NULL, "belongs-to", 1);
    } else {
        ly_print(out, "%*s<module name=\"%s\"\n", LEVEL, INDENT, module->name);
        yin_print_xmlns(out, module);
        ly_print(out, ">\n");

        level++;
        if (module->version) {
            yin_print_substmt(out, level, LYEXT_SUBSTMT_VERSION, 0, module->version == LYS_VERSION_1_1 ? "1.1" : "1",
                              module, module->ext, module->ext_size);
        }
        yin_print_substmt(out, level, LYEXT_SUBSTMT_NAMESPACE, 0, module->ns,
                          module, module->ext, module->ext_size);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_PREFIX, 0, module->prefix,
                           module, module->ext, module->ext_size);
    }

    /* linkage-stmts */
    for (i = 0; i < module->imp_size; i++) {
        yin_print_open(out, level, NULL, "import", "module", module->imp[i].module->name, 1);
        level++;

        yin_print_extension_instances(out, level, module, LYEXT_SUBSTMT_SELF, 0,
                                       module->imp[i].ext, module->imp[i].ext_size);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_PREFIX, 0, module->imp[i].prefix,
                          module, module->imp[i].ext, module->imp[i].ext_size);
        if (module->imp[i].rev[0]) {
            yin_print_substmt(out, level, LYEXT_SUBSTMT_REVISIONDATE, 0, module->imp[i].rev,
                              module, module->imp[i].ext, module->imp[i].ext_size);
        }
        yin_print_substmt(out, level, LYEXT_SUBSTMT_DESCRIPTION, 0, module->imp[i].dsc,
                          module, module->imp[i].ext, module->imp[i].ext_size);
        yin_print_substmt(out, level, LYEXT_SUBSTMT_REFERENCE, 0, module->imp[i].ref,
                          module, module->imp[i].ext, module->imp[i].ext_size);

        level--;
        yin_print_close(out, level, NULL, "import", 1);
    }
    for (i = 0; i < module->inc_size; i++) {
        if (module->inc[i].rev[0] || module->inc[i].dsc || module->inc[i].ref || module->inc[i].ext_size) {
            yin_print_open(out, level, NULL, "include", "module", module->inc[i].submodule->name, 1);
            level++;

            yin_print_extension_instances(out, level, module, LYEXT_SUBSTMT_SELF, 0,
                                          module->inc[i].ext, module->inc[i].ext_size);
            if (module->inc[i].rev[0]) {
                yin_print_substmt(out, level, LYEXT_SUBSTMT_REVISIONDATE, 0, module->inc[i].rev,
                                  module, module->inc[i].ext, module->inc[i].ext_size);
            }
            yin_print_substmt(out, level, LYEXT_SUBSTMT_DESCRIPTION, 0, module->inc[i].dsc,
                               module, module->inc[i].ext, module->inc[i].ext_size);
            yin_print_substmt(out, level, LYEXT_SUBSTMT_REFERENCE, 0, module->inc[i].ref,
                              module, module->inc[i].ext, module->inc[i].ext_size);

            level--;
            yin_print_close(out, level, NULL, "include", 1);
        } else {
            yin_print_open(out, level, NULL, "include", "module", module->inc[i].submodule->name, -1);
        }
    }

    /* meta-stmts */
    yin_print_substmt(out, level, LYEXT_SUBSTMT_ORGANIZATION, 0, module->org,
                      module, module->ext, module->ext_size);
    yin_print_substmt(out, level, LYEXT_SUBSTMT_CONTACT, 0, module->contact,
                      module, module->ext, module->ext_size);
    yin_print_substmt(out, level, LYEXT_SUBSTMT_DESCRIPTION, 0, module->dsc,
                      module, module->ext, module->ext_size);
    yin_print_substmt(out, level, LYEXT_SUBSTMT_REFERENCE, 0, module->ref,
                      module, module->ext, module->ext_size);

    /* revision-stmts */
    for (i = 0; i < module->rev_size; i++) {
        yin_print_revision(out, level, module, &module->rev[i]);
    }

    /* body-stmts */
    for (i = 0; i < module->extensions_size; ++i) {
        yin_print_extension(out, level, &module->extensions[i]);
    }
    if (module->ext_size) {
        yin_print_extension_instances(out, level, module, LYEXT_SUBSTMT_SELF, 0, module->ext, module->ext_size);
    }

    for (i = 0; i < module->features_size; i++) {
        yin_print_feature(out, level, &module->features[i]);
    }

    for (i = 0; i < module->ident_size; i++) {
        yin_print_identity(out, level, &module->ident[i]);
    }

    for (i = 0; i < module->tpdf_size; i++) {
        yin_print_typedef(out, level, module, &module->tpdf[i]);
    }

    LY_TREE_FOR(lys_main_module(module)->data, node) {
        if (node->module != module) {
            /* data from submodules */
            continue;
        }
        yin_print_snode(out, level, node, LYS_GROUPING);
    }

    LY_TREE_FOR(lys_main_module(module)->data, node) {
        if (node->module != module) {
            /* data from submodules */
            continue;
        }
        yin_print_snode(out, level, node, LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST |
                        LYS_USES | LYS_ANYDATA);
    }

    for (i = 0; i < module->augment_size; i++) {
        yin_print_augment(out, level, module, &module->augment[i]);
    }

    LY_TREE_FOR(lys_main_module(module)->data, node) {
        if (node->module != module) {
            /* data from submodules */
            continue;
        }
        yin_print_snode(out, level, node, LYS_RPC | LYS_ACTION);
    }

    LY_TREE_FOR(lys_main_module(module)->data, node) {
        if (node->module != module) {
            /* data from submodules */
            continue;
        }
        yin_print_snode(out, level, node, LYS_NOTIF);
    }

    for (i = 0; i < module->deviation_size; ++i) {
        yin_print_deviation(out, level, module, &module->deviation[i]);
    }

    level--;
    if (module->type) {
        ly_print(out, "%*s</submodule>\n", LEVEL, INDENT);
    } else {
        ly_print(out, "%*s</module>\n", LEVEL, INDENT);
    }
    ly_print_flush(out);

    return EXIT_SUCCESS;
}

int
yin_print_model(struct lyout *out, const struct lys_module *module)
{
    int level = 0;

    ly_print(out, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    return yin_print_model_(out, level, module);
}

static void
yin_print_extcomplex_bool(struct lyout *out, int level, const struct lys_module *module,
                          struct lys_ext_instance_complex *ext, LY_STMT stmt,
                          const char *true_val, const char *false_val, int *content)
{
    struct lyext_substmt *info;
    uint8_t *val;

    val = lys_ext_complex_get_substmt(stmt, ext, &info);
    if (!val || !(*val)) {
        return;
    }

    yin_print_close_parent(out, content);
    if (*val == 1) {
        yin_print_substmt(out, level, (LYEXT_SUBSTMT)stmt, 0, true_val, module, ext->ext, ext->ext_size);
    } else if (*val == 2) {
        yin_print_substmt(out, level, (LYEXT_SUBSTMT)stmt, 0, false_val, module, ext->ext, ext->ext_size);
    } else {
        LOGINT(module->ctx);
    }
}

static void
yin_print_extcomplex_str(struct lyout *out, int level, const struct lys_module *module,
                         struct lys_ext_instance_complex *ext, LY_STMT stmt, int *content)
{
    struct lyext_substmt *info;
    const char **str;
    int c;

    str = lys_ext_complex_get_substmt(stmt, ext, &info);
    if (!str || !(*str)) {
        return;
    }
    if (info->cardinality >= LY_STMT_CARD_SOME) {
        /* we have array */
        for (str = (const char **)(*str), c = 0; *str; str++, c++) {
            yin_print_close_parent(out, content);
            yin_print_substmt(out, level, (LYEXT_SUBSTMT)stmt, c, *str, module, ext->ext, ext->ext_size);
        }
    } else {
        yin_print_close_parent(out, content);
        yin_print_substmt(out, level, (LYEXT_SUBSTMT)stmt, 0, *str, module, ext->ext, ext->ext_size);
    }
}

/* val1 is supposed to be the default value */
static void
yin_print_extcomplex_flags(struct lyout *out, int level, const struct lys_module *module,
                           struct lys_ext_instance_complex *ext, LY_STMT stmt,
                           const char *val1_str, const char *val2_str, uint16_t val1, uint16_t val2,
                           int *content)
{
    const char *str;
    uint16_t *flags;

    flags = lys_ext_complex_get_substmt(stmt, ext, NULL);
    if (!flags) {
        return;
    }

    if (val1 & *flags) {
        str = val1_str;
    } else if (val2 & *flags) {
        str = val2_str;
    } else if (lys_ext_iter(ext->ext, ext->ext_size, 0, (LYEXT_SUBSTMT)stmt) != -1) {
        /* flag not set, but since there are some extension, we are going to print the default value */
        str = val1_str;
    } else {
        return;
    }

    yin_print_close_parent(out, content);
    yin_print_substmt(out, level, (LYEXT_SUBSTMT)stmt, 0, str, module, ext->ext, ext->ext_size);
}

static void
yin_print_extension_instances(struct lyout *out, int level, const struct lys_module *module,
                              LYEXT_SUBSTMT substmt, uint8_t substmt_index,
                              struct lys_ext_instance **ext, unsigned int count)
{
    unsigned int u, x;
    struct lys_module *mod;
    const char *prefix = NULL;
    struct lyext_substmt *info;
    int content, content2, i, j, c;
    uint16_t *flags;
    const char *str;
    void **pp, *p;
    struct lys_node *siter;

#define YIN_PRINT_EXTCOMPLEX_SNODE(STMT)                                                      \
    pp = lys_ext_complex_get_substmt(STMT, (struct lys_ext_instance_complex *)ext[u], NULL);  \
    if (!pp || !(*pp)) { break; }                                                             \
    LY_TREE_FOR((struct lys_node*)(*pp), siter) {                                             \
        if (lys_snode2stmt(siter->nodetype) == STMT) {                                        \
            yin_print_close_parent(out, &content);                                            \
            yin_print_snode(out, level, siter, LYS_ANY);                                      \
        }                                                                                     \
    }
#define YIN_PRINT_EXTCOMPLEX_STRUCT(STMT, TYPE, FUNC)                                         \
    pp = lys_ext_complex_get_substmt(STMT, (struct lys_ext_instance_complex *)ext[u], NULL);  \
    if (!pp || !(*pp)) { break; }                                                             \
    if (info[i].cardinality >= LY_STMT_CARD_SOME) { /* process array */                       \
        for (pp = *pp; *pp; pp++) {                                                           \
            yin_print_close_parent(out, &content);                                            \
            FUNC(out, level, (TYPE *)(*pp));                                                  \
        }                                                                                     \
    } else { /* single item */                                                                \
        yin_print_close_parent(out, &content);                                                \
        FUNC(out, level, (TYPE *)(*pp));                                                      \
    }
#define YIN_PRINT_EXTCOMPLEX_STRUCT_M(STMT, TYPE, FUNC, ARGS...)                              \
    pp = lys_ext_complex_get_substmt(STMT, (struct lys_ext_instance_complex *)ext[u], NULL);  \
    if (!pp || !(*pp)) { break; }                                                             \
    if (info[i].cardinality >= LY_STMT_CARD_SOME) { /* process array */                       \
        for (pp = *pp; *pp; pp++) {                                                           \
            yin_print_close_parent(out, &content);                                            \
            FUNC(out, level, module, (TYPE *)(*pp), ##ARGS);                                  \
        }                                                                                     \
    } else { /* single item */                                                                \
        yin_print_close_parent(out, &content);                                                \
        FUNC(out, level, module, (TYPE *)(*pp), ##ARGS);                                      \
    }
#define YIN_PRINT_EXTCOMPLEX_INT(STMT, TYPE, SIGN)                                 \
    p = &((struct lys_ext_instance_complex*)ext[u])->content[info[i].offset];      \
    if (!p || !*(TYPE**)p) { break; }                                              \
    if (info[i].cardinality >= LY_STMT_CARD_SOME) { /* we have array */            \
        for (c = 0; (*(TYPE***)p)[c]; c++) {                                       \
            yin_print_close_parent(out, &content);                                 \
            yin_print_##SIGN(out, level, STMT, c, module,                          \
                             ext[u]->ext, ext[u]->ext_size, *(*(TYPE***)p)[c]);    \
        }                                                                          \
    } else {                                                                       \
        yin_print_close_parent(out, &content);                                     \
        yin_print_##SIGN(out, level, STMT, 0, module,                              \
                         ext[u]->ext, ext[u]->ext_size, (**(TYPE**)p));            \
    }

    for (u = 0; u < count; u++) {
        if (ext[u]->flags & LYEXT_OPT_INHERIT) {
            /* ignore the inherited extensions which were not explicitely instantiated in the module */
            continue;
        } else if (ext[u]->insubstmt != substmt || ext[u]->insubstmt_index != substmt_index) {
            /* do not print the other substatement than the required */
            continue;
        } else if (ext[u]->def->module == module->ctx->models.list[0] &&
                 (!strcmp(ext[u]->arg_value, "operation") ||
                  !strcmp(ext[u]->arg_value, "select") ||
                  !strcmp(ext[u]->arg_value, "type"))) {
            /* hack for NETCONF's edit-config's operation and filter's attributes
             * - the annotation definition is only internal, do not print it */
            continue;
        }

        mod = lys_main_module(ext[u]->def->module);
        if (mod == lys_main_module(module)) {
            prefix = module->prefix;
        } else {
            for (x = 0; x < module->imp_size; x++) {
                if (mod == module->imp[x].module) {
                    prefix = module->imp[x].prefix;
                    break;
                }
            }
        }
        assert(prefix);

        content = 0;
        if (ext[u]->arg_value) {
            if (ext[u]->def->flags & LYS_YINELEM) {
                /* argument as element */
                content = 1;
                yin_print_open(out, level, prefix, ext[u]->def->name, NULL, NULL, content);
                level++;
                ly_print(out, "%*s<%s:%s>", LEVEL, INDENT, prefix, ext[u]->def->argument);
                lyxml_dump_text(out, ext[u]->arg_value, LYXML_DATA_ELEM);
                ly_print(out, "</%s:%s>\n", prefix, ext[u]->def->argument);
                level--;
            } else {
                /* argument as attribute */
                yin_print_open(out, level, prefix, ext[u]->def->name,
                               ext[u]->def->argument, ext[u]->arg_value, content);
            }
        } else {
            yin_print_open(out, level, prefix, ext[u]->def->name, NULL, NULL, content);
        }

        /* extensions */
        if (ext[u]->ext_size) {
            yin_print_close_parent(out, &content);
            yin_print_extension_instances(out, level + 1, module, LYEXT_SUBSTMT_SELF, 0,
                                          ext[u]->ext, ext[u]->ext_size);
        }

        /* extension - type-specific part */
        switch (ext[u]->ext_type) {
        case LYEXT_FLAG:
            /* flag extension - nothing special */
            break;
        case LYEXT_COMPLEX:
            info = ((struct lys_ext_instance_complex*)ext[u])->substmt; /* shortcut */
            if (!info) {
                /* no content */
                break;
            }
            level++;
            for (i = 0; info[i].stmt; i++) {
                switch(info[i].stmt) {
                case LY_STMT_DESCRIPTION:
                case LY_STMT_REFERENCE:
                case LY_STMT_UNITS:
                case LY_STMT_DEFAULT:
                case LY_STMT_ERRTAG:
                case LY_STMT_ERRMSG:
                case LY_STMT_PREFIX:
                case LY_STMT_NAMESPACE:
                case LY_STMT_PRESENCE:
                case LY_STMT_REVISIONDATE:
                case LY_STMT_KEY:
                case LY_STMT_BASE:
                case LY_STMT_CONTACT:
                case LY_STMT_ORGANIZATION:
                case LY_STMT_PATH:
                    yin_print_extcomplex_str(out, level, module, (struct lys_ext_instance_complex*)ext[u],
                                             info[i].stmt, &content);
                    break;
                case LY_STMT_ARGUMENT:
                    pp = lys_ext_complex_get_substmt(LY_STMT_ARGUMENT, (struct lys_ext_instance_complex*)ext[u], NULL);
                    if (!pp || !(*pp)) {
                        break;
                    }
                    yin_print_close_parent(out, &content);
                    if (info->cardinality >= LY_STMT_CARD_SOME) {
                        /* we have array */
                        for (c = 0; ((const char***)pp)[0][c]; c++) {
                            content2 = 0;
                            yin_print_open(out, level, NULL, "argument", "name", ((const char ***)pp)[0][c], 0);
                            j = -1;
                            while ((j = lys_ext_iter(ext[u]->ext, ext[u]->ext_size, j + 1, LYEXT_SUBSTMT_ARGUMENT)) != -1) {
                                if (ext[u]->ext[j]->insubstmt_index != c) {
                                    continue;
                                }
                                yin_print_close_parent(out, &content2);
                                yin_print_extension_instances(out, level + 1, module, LYEXT_SUBSTMT_ARGUMENT, c,
                                                              &ext[u]->ext[j], 1);
                            }
                            if (((uint8_t *)pp[1])[c] == 1) {
                                yin_print_close_parent(out, &content2);
                                yin_print_substmt(out, level + 1, LYEXT_SUBSTMT_YINELEM, c,
                                                 (((uint8_t *)pp[1])[c] == 1) ? "true" : "false", module, ext[u]->ext, ext[u]->ext_size);
                            } else {
                                j = -1;
                                while ((j = lys_ext_iter(ext[u]->ext, ext[u]->ext_size, j + 1, LYEXT_SUBSTMT_YINELEM)) != -1) {
                                    if (ext[u]->ext[j]->insubstmt_index == c) {
                                        yin_print_close_parent(out, &content2);
                                        yin_print_substmt(out, level + 1, LYEXT_SUBSTMT_YINELEM, c, (((uint8_t *)pp[1])[c] == 1) ? "true" : "false",
                                                          module, ext[u]->ext + j, ext[u]->ext_size - j);
                                        break;
                                    }
                                }
                            }
                            yin_print_close(out, level, NULL, "argument", content2);
                        }
                    } else {
                        content2 = 0;
                        yin_print_open(out, level, NULL, "argument", "name", (const char *)pp[0], 0);
                        j = -1;
                        while ((j = lys_ext_iter(ext[u]->ext, ext[u]->ext_size, j + 1, LYEXT_SUBSTMT_ARGUMENT)) != -1) {
                            yin_print_close_parent(out, &content2);
                            yin_print_extension_instances(out, level + 1, module, LYEXT_SUBSTMT_ARGUMENT, 0,
                                                          &ext[u]->ext[j], 1);
                        }
                        if (*(uint8_t*)(pp + 1) == 1 || lys_ext_iter(ext[u]->ext, ext[u]->ext_size, 0, LYEXT_SUBSTMT_YINELEM) != -1) {
                            yin_print_close_parent(out, &content2);
                            yin_print_substmt(out, level + 1, LYEXT_SUBSTMT_YINELEM, 0,
                                             (*(uint8_t*)(pp + 1) == 1) ? "true" : "false", module, ext[u]->ext, ext[u]->ext_size);
                        }
                        yin_print_close(out, level, NULL, "argument", content2);
                    }
                    break;
                case LY_STMT_BELONGSTO:
                    pp = lys_ext_complex_get_substmt(LY_STMT_BELONGSTO, (struct lys_ext_instance_complex*)ext[u], NULL);
                    if (!pp || !(*pp)) {
                        break;
                    }
                    if (info->cardinality >= LY_STMT_CARD_SOME) {
                        /* we have array */
                        for (c = 0; ((const char***)pp)[0][c]; c++) {
                            yin_print_close_parent(out, &content);
                            yin_print_open(out, level, NULL, "belongs-to", "module", ((const char ***)pp)[0][c], 1);
                            j = -1;
                            while ((j = lys_ext_iter(ext[u]->ext, ext[u]->ext_size, j + 1, LYEXT_SUBSTMT_BELONGSTO)) != -1) {
                                yin_print_extension_instances(out, level + 1, module, LYEXT_SUBSTMT_BELONGSTO, c,
                                                              &ext[u]->ext[j], 1);
                            }
                            yin_print_substmt(out, level + 1, LYEXT_SUBSTMT_PREFIX, c, ((const char ***)pp)[1][c],
                                              module, ext[u]->ext, ext[u]->ext_size);
                            yin_print_close(out, level, NULL, "belongs-to", 1);
                        }
                    } else {
                        yin_print_close_parent(out, &content);
                        yin_print_open(out, level, NULL, "belongs-to", "module", (const char *)pp[0], 1);
                        j = -1;
                        while ((j = lys_ext_iter(ext[u]->ext, ext[u]->ext_size, j + 1, LYEXT_SUBSTMT_BELONGSTO)) != -1) {
                            yin_print_extension_instances(out, level + 1, module, LYEXT_SUBSTMT_BELONGSTO, 0,
                                                          &ext[u]->ext[j], 1);
                        }
                        yin_print_substmt(out, level + 1, LYEXT_SUBSTMT_PREFIX, 0, (const char *)pp[1],
                                          module, ext[u]->ext, ext[u]->ext_size);
                        yin_print_close(out, level, NULL, "belongs-to", 1);
                    }
                    break;
                case LY_STMT_TYPE:
                    YIN_PRINT_EXTCOMPLEX_STRUCT_M(LY_STMT_TYPE, struct lys_type, yin_print_type);
                    break;
                case LY_STMT_TYPEDEF:
                    YIN_PRINT_EXTCOMPLEX_STRUCT_M(LY_STMT_TYPEDEF, struct lys_tpdf, yin_print_typedef);
                    break;
                case LY_STMT_IFFEATURE:
                    YIN_PRINT_EXTCOMPLEX_STRUCT_M(LY_STMT_IFFEATURE, struct lys_iffeature, yin_print_iffeature);
                    break;
                case LY_STMT_STATUS:
                    flags = (uint16_t*)&((struct lys_ext_instance_complex*)ext[u])->content[info[i].offset];
                    if (!flags || !(*flags)) {
                        break;
                    }

                    if (*flags & LYS_STATUS_CURR) {
                        yin_print_close_parent(out, &content);
                        str = "current";
                    } else if (*flags & LYS_STATUS_DEPRC) {
                        yin_print_close_parent(out, &content);
                        str = "deprecated";
                    } else if (*flags & LYS_STATUS_OBSLT) {
                        yin_print_close_parent(out, &content);
                        str = "obsolete";
                    } else {
                        /* no status flag */
                        break;
                    }
                    yin_print_substmt(out, level, LYEXT_SUBSTMT_STATUS, 0, str, module, ext[u]->ext, ext[u]->ext_size);
                    break;
                case LY_STMT_CONFIG:
                    yin_print_extcomplex_flags(out, level, module, (struct lys_ext_instance_complex*)ext[u],
                                               LY_STMT_CONFIG, "true", "false",
                                               LYS_CONFIG_W | LYS_CONFIG_SET, LYS_CONFIG_R | LYS_CONFIG_SET, &content);
                    break;
                case LY_STMT_MANDATORY:
                    yin_print_extcomplex_flags(out, level, module, (struct lys_ext_instance_complex*)ext[u],
                                               LY_STMT_MANDATORY, "false", "true", LYS_MAND_FALSE, LYS_MAND_TRUE,
                                               &content);
                    break;
                case LY_STMT_ORDEREDBY:
                    yin_print_extcomplex_flags(out, level, module, (struct lys_ext_instance_complex*)ext[u],
                                               LY_STMT_ORDEREDBY, "system", "user", 0, LYS_USERORDERED, &content);
                    break;
                case LY_STMT_REQINSTANCE:
                    yin_print_extcomplex_bool(out, level, module, (struct lys_ext_instance_complex*)ext[u],
                                              info[i].stmt, "true", "false", &content);
                    break;
                case LY_STMT_MODIFIER:
                    yin_print_extcomplex_bool(out, level, module, (struct lys_ext_instance_complex*)ext[u],
                                              LY_STMT_MODIFIER, "invert-match", NULL, &content);
                    break;
                case LY_STMT_DIGITS:
                    p = &((struct lys_ext_instance_complex*)ext[u])->content[info[i].offset];
                    if (!p) {
                        break;
                    }
                    if (info->cardinality >= LY_STMT_CARD_SOME && *(uint8_t**)p) { /* we have array */
                        for (c = 0; (*(uint8_t**)p)[c]; c++) {
                            yin_print_close_parent(out, &content);
                            yin_print_unsigned(out, level, LYEXT_SUBSTMT_DIGITS, c, module,
                                               ext[u]->ext, ext[u]->ext_size, (*(uint8_t**)p)[c]);
                        }
                    } else if ((*(uint8_t*)p)) {
                        yin_print_close_parent(out, &content);
                        yin_print_unsigned(out, level, LYEXT_SUBSTMT_DIGITS, 0, module,
                                           ext[u]->ext, ext[u]->ext_size, (*(uint8_t*)p));
                    }
                    break;
                case LY_STMT_MAX:
                    YIN_PRINT_EXTCOMPLEX_INT(LYEXT_SUBSTMT_MAX, uint32_t, unsigned);
                    break;
                case LY_STMT_MIN:
                    YIN_PRINT_EXTCOMPLEX_INT(LYEXT_SUBSTMT_MIN, uint32_t, unsigned);
                    break;
                case LY_STMT_POSITION:
                    YIN_PRINT_EXTCOMPLEX_INT(LYEXT_SUBSTMT_POSITION, uint32_t, unsigned);
                    break;
                case LY_STMT_VALUE:
                    YIN_PRINT_EXTCOMPLEX_INT(LYEXT_SUBSTMT_VALUE, int32_t, signed);
                    break;
                case LY_STMT_UNIQUE:
                    pp = lys_ext_complex_get_substmt(LY_STMT_UNIQUE, (struct lys_ext_instance_complex *)ext[u], NULL);
                    if (!pp || !(*pp)) {
                        break;
                    }
                    if (info[i].cardinality >= LY_STMT_CARD_SOME) { /* process array */
                        for (pp = *pp, c = 0; *pp; pp++, c++) {
                            yin_print_close_parent(out, &content);
                            yin_print_unique(out, level, module, (struct lys_unique*)(*pp));
                            /* unique's extensions */
                            j = -1; content2 = 0;
                            do {
                                j = lys_ext_iter(ext[u]->ext, ext[u]->ext_size, j + 1, LYEXT_SUBSTMT_UNIQUE);
                            } while (j != -1 && ext[u]->ext[j]->insubstmt_index != c);
                            if (j != -1) {
                                yin_print_close_parent(out, &content2);
                                do {
                                    yin_print_extension_instances(out, level + 1, module, LYEXT_SUBSTMT_UNIQUE, c, &ext[u]->ext[j], 1);
                                    do {
                                        j = lys_ext_iter(ext[u]->ext, ext[u]->ext_size, j + 1, LYEXT_SUBSTMT_UNIQUE);
                                    } while (j != -1 && ext[u]->ext[j]->insubstmt_index != c);
                                } while (j != -1);
                            }
                            yin_print_close(out, level, NULL, "unique", content2);
                        }
                    } else { /* single item */
                        yin_print_close_parent(out, &content);
                        yin_print_unique(out, level, module, (struct lys_unique*)(*pp));
                        /* unique's extensions */
                        j = -1; content2 = 0;
                        while ((j = lys_ext_iter(ext[u]->ext, ext[u]->ext_size, j + 1, LYEXT_SUBSTMT_UNIQUE)) != -1) {
                            yin_print_close_parent(out, &content2);
                            yin_print_extension_instances(out, level + 1, module, LYEXT_SUBSTMT_UNIQUE, 0, &ext[u]->ext[j], 1);
                        }
                        yin_print_close(out, level, NULL, "unique", content2);
                    }
                    break;
                case LY_STMT_MODULE:
                    YIN_PRINT_EXTCOMPLEX_STRUCT(LY_STMT_MODULE, struct lys_module, yin_print_model_);
                    break;
                case LY_STMT_ACTION:
                case LY_STMT_ANYDATA:
                case LY_STMT_ANYXML:
                case LY_STMT_CASE:
                case LY_STMT_CHOICE:
                case LY_STMT_CONTAINER:
                case LY_STMT_GROUPING:
                case LY_STMT_INPUT:
                case LY_STMT_OUTPUT:
                case LY_STMT_LEAF:
                case LY_STMT_LEAFLIST:
                case LY_STMT_LIST:
                case LY_STMT_NOTIFICATION:
                case LY_STMT_USES:
                    YIN_PRINT_EXTCOMPLEX_SNODE(info[i].stmt);
                    break;
                case LY_STMT_LENGTH:
                    YIN_PRINT_EXTCOMPLEX_STRUCT_M(LY_STMT_LENGTH, struct lys_restr, yin_print_typerestr, "length");
                    break;
                case LY_STMT_MUST:
                    YIN_PRINT_EXTCOMPLEX_STRUCT_M(LY_STMT_MUST, struct lys_restr, yin_print_must);
                    break;
                case LY_STMT_PATTERN:
                    YIN_PRINT_EXTCOMPLEX_STRUCT_M(LY_STMT_PATTERN, struct lys_restr, yin_print_typerestr, "pattern");
                    break;
                case LY_STMT_RANGE:
                    YIN_PRINT_EXTCOMPLEX_STRUCT_M(LY_STMT_RANGE, struct lys_restr, yin_print_typerestr, "range");
                    break;
                case LY_STMT_WHEN:
                    YIN_PRINT_EXTCOMPLEX_STRUCT_M(LY_STMT_WHEN, struct lys_when, yin_print_when);
                    break;
                case LY_STMT_REVISION:
                    YIN_PRINT_EXTCOMPLEX_STRUCT_M(LY_STMT_REVISION, struct lys_revision, yin_print_revision);
                    break;
                default:
                    /* TODO */
                    break;
                }
            }
            level--;
            break;
        }

        /* close extension */
        yin_print_close(out, level, prefix, ext[u]->def->name, content);
    }

#undef YIN_PRINT_EXTCOMPLEX_STRUCT
#undef YIN_PRINT_EXTCOMPLEX_STRUCT_M
#undef YIN_PRINT_EXTCOMPLEX_INT
}
