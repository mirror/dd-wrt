/**
 * @file printer/info.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Schema JSON printer for libyang data model structure
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "common.h"
#include "printer.h"
#include "tree_schema.h"
#include "resolve.h"

#define INDENT_LEN 2

static const char *
jsons_nodetype_str(LYS_NODE value) {
    switch (value) {
    case LYS_CONTAINER: return "container";
    case LYS_CHOICE: return "choice";
    case LYS_LEAF: return "leaf";
    case LYS_LEAFLIST: return "leaf-list";
    case LYS_LIST: return "list";
    case LYS_ANYXML: return "anyxml";
    case LYS_CASE: return "case";
    case LYS_NOTIF: return "notification";
    case LYS_RPC: return "rpc";
    case LYS_INPUT: return "input";
    case LYS_OUTPUT: return "output";
    case LYS_ACTION: return "action";
    case LYS_ANYDATA: return "anydata";
    default: return NULL;
    }
}

/* shared with printer_json.c */
int json_print_string(struct lyout *out, const char *text);

static void jsons_print_data(struct lyout *out, const struct lys_module *mod, struct lys_node *data, int *first);
static void jsons_print_notifs(struct lyout *out, struct lys_node *data, int *first);
static void jsons_print_actions(struct lyout *out, struct lys_node *data, int *first);

static void
jsons_print_text(struct lyout *out, const char *label, const char *arg, const char *text, int closeit, int *first)
{
    if (!text) {
        return;
    }
    ly_print(out, "%s\"%s\":{\"%s\":", (first && (*first)) ? "" : ",", label, arg);
    json_print_string(out, text);
    if (closeit) {
        ly_print(out, "}");
    }
    if (first) {
        (*first) = 0;
    }
}

static void
jsons_print_object(struct lyout *out, const char *label, const char *arg, const char *val, int closeit, int *first)
{
    if (!val) {
        return;
    }

    ly_print(out, "%s\"%s\":{\"%s\":\"%s\"%s", (first && (*first)) ? "" : ",", label, arg, val, closeit ? "}" : "");

    if (first) {
        (*first) = 0;
    }
}

static void
jsons_print_status(struct lyout *out, uint16_t flags, int *first)
{
    const char *str;

    if (flags & LYS_STATUS_MASK) {
        if (flags & LYS_STATUS_OBSLT) {
            str = "obsolete";
        } else if (flags & LYS_STATUS_DEPRC) {
            str = "deprecated";
        } else {
            str = "current";
        }
        jsons_print_object(out, "status", "value", str, 1, first);
    }
}

static void
jsons_print_config(struct lyout *out, uint16_t flags, int *first)
{
    const char *str = NULL;

    if (flags & LYS_CONFIG_MASK) {
        if (flags & LYS_CONFIG_R) {
            str = "false";
        } else if (flags & LYS_CONFIG_W) {
            str = "true";
        }
        jsons_print_object(out, "config", "value", str, 1, first);
    }
}

static void
jsons_print_mand(struct lyout *out, uint16_t flags, int *first)
{
    const char *str = NULL;

    if (flags & LYS_MAND_MASK) {
        if (flags & LYS_MAND_TRUE) {
            str = "true";
        } else if (flags & LYS_MAND_FALSE) {
            str = "false";
        }
        jsons_print_object(out, "mandatory", "value", str, 1, first);
    }
}

static void
jsons_print_ordering(struct lyout *out, uint16_t flags, int *first)
{
    if (flags & LYS_USERORDERED) {
        jsons_print_object(out, "ordered-by", "value", "user", 1, first);
    } else {
        jsons_print_object(out, "ordered-by", "value", "system", 1, first);
    }
}

static void
jsons_print_iffeatures(struct lyout *out, const struct lys_module *module,
                       struct lys_iffeature *iff, uint8_t iff_size, int *first)
{
    int i;

    if (!iff_size) {
        return;
    }

    ly_print(out, "%s\"if-features\":[", (first && (*first)) ? "" : ",");
    for (i = 0; i < iff_size; ++i) {
        ly_print(out, "%s\"", i ? "," : "");
        ly_print_iffeature(out, module, &iff[i], 3);
        ly_print(out, "\"");
    }
    ly_print(out, "]");
    if (first) {
        (*first) = 0;
    }
}

static void
jsons_print_when(struct lyout *out, const struct lys_when *when, int *first)
{
    if (!when) {
        return;
    }
    jsons_print_text(out, "when", "condition", when->cond, 0, first);
    jsons_print_text(out, "description", "text", when->dsc, 1, NULL);
    jsons_print_text(out, "reference", "text", when->ref, 1, NULL);
    ly_print(out, "}");
}

static void
jsons_print_typerestr(struct lyout *out, const struct lys_restr *restr, const char *label, int *first)
{
    int pattern = 0;

    if (!restr) {
        return;
    }
    if (restr->expr[0] == 0x06 || restr->expr[0] == 0x15) {
        pattern = 1;
    }

    if (label) {
        jsons_print_text(out, label, "value", pattern ? &restr->expr[1] : restr->expr, 0, first);
    } else {
        ly_print(out, "%s{\"%s\":", (first && (*first)) ? "" : ",", "value");
        json_print_string(out, pattern ? &restr->expr[1] : restr->expr);
    }
    if (pattern && restr->expr[0] == 0x15) {
        jsons_print_object(out, "modifier", "value", "invert-match", 1, NULL);
    }
    jsons_print_text(out, "description", "text", restr->dsc, 1, NULL);
    jsons_print_text(out, "reference", "text", restr->ref, 1, NULL);
    jsons_print_object(out, "error-app-tag", "value", restr->eapptag, 1, NULL);
    jsons_print_text(out, "error-message", "value", restr->emsg, 1, NULL);
    ly_print(out, "}");
    if (first) {
        (*first) = 0;
    }
}

static void
jsons_print_musts(struct lyout *out, const struct lys_restr *must, uint8_t must_size, int *first)
{
    int i, f;

    if (!must_size) {
        return;
    }

    ly_print(out, "%s\"musts\":[", (first && (*first)) ? "" : ",");
    f = 1;
    for (i = 0; i < must_size; ++i) {
        jsons_print_typerestr(out, &must[i], NULL, &f);
    }
    ly_print(out, "]");
}

static void
jsons_print_type_(struct lyout *out, const struct lys_type *type, int with_label, int *first)
{
    unsigned int i;
    int f;
    struct lys_module *mod;
    struct lys_node *node;

    if (!type) {
        return;
    }

    if (with_label) {
        ly_print(out, "%s\"type\":{", (first && (*first)) ? "" : ",");
    } else {
        ly_print(out, "%s{", (first && (*first)) ? "" : ",");
    }

    switch (type->base) {
    case LY_TYPE_BINARY:
        ly_print(out, "\"basetype\":\"binary\"");
        jsons_print_typerestr(out, type->info.binary.length, "length", NULL);
        break;
    case LY_TYPE_BITS:
        ly_print(out, "\"basetype\":\"bits\"");
        ly_print(out, ",\"bits\":[");
        for (i = 0; i < type->info.bits.count; ++i) {
            ly_print(out, "%s{\"position\":\"%u\",\"name\":\"%s\"", i ? "," : "",
                     type->info.bits.bit[i].pos, type->info.bits.bit[i].name);
            jsons_print_text(out, "description", "text", type->info.bits.bit[i].dsc, 1, NULL);
            jsons_print_text(out, "reference", "text", type->info.bits.bit[i].ref, 1, NULL);
            jsons_print_status(out, type->info.bits.bit[i].flags, NULL);
            jsons_print_iffeatures(out, type->parent->module,
                                   type->info.bits.bit[i].iffeature, type->info.bits.bit[i].iffeature_size, NULL);
            ly_print(out, "}");
        }
        ly_print(out, "]");
        break;
    case LY_TYPE_BOOL:
        ly_print(out, "\"basetype\":\"boolean\"");
        break;
    case LY_TYPE_DEC64:
        ly_print(out, "\"basetype\":\"decimal64\"");
        jsons_print_typerestr(out, type->info.dec64.range, "range", NULL);
        ly_print(out, ",\"fraction-digits\":{\"value\":\"%u\"}", type->info.dec64.dig);
        break;
    case LY_TYPE_EMPTY:
        ly_print(out, "\"basetype\":\"empty\"");
        break;
    case LY_TYPE_ENUM:
        ly_print(out, "\"basetype\":\"enumeration\"");
        ly_print(out, ",\"enums\":[");
        for (i = 0; i < type->info.enums.count; ++i) {
            ly_print(out, "%s{\"value\":\"%d\",\"name\":\"%s\"", i ? "," : "",
                     type->info.enums.enm[i].value, type->info.enums.enm[i].name);
            jsons_print_text(out, "description", "text", type->info.enums.enm[i].dsc, 1, NULL);
            jsons_print_text(out, "reference", "text", type->info.enums.enm[i].ref, 1, NULL);
            jsons_print_status(out, type->info.enums.enm[i].flags, NULL);
            jsons_print_iffeatures(out, type->parent->module,
                                   type->info.enums.enm[i].iffeature, type->info.enums.enm[i].iffeature_size, NULL);
            ly_print(out, "}");
        }
        ly_print(out, "]");
        break;
    case LY_TYPE_IDENT:
        ly_print(out, "\"basetype\":\"identityref\"");
        if (type->info.ident.count) {
            ly_print(out, ",\"bases\":[");
            for (i = 0; i < type->info.ident.count; ++i) {
                mod = type->info.ident.ref[i]->module;
                ly_print(out, "%s\"%s%s%s:%s\"", i ? "," : "",
                         mod->name, mod->rev_size ? "@" : "", mod->rev_size ? mod->rev[0].date : "",
                         type->info.ident.ref[i]->name);
            }
            ly_print(out, "]");
        }
        break;
    case LY_TYPE_INST:
        ly_print(out, "\"basetype\":\"instance-identifier\"");
        if (type->info.inst.req) {
            jsons_print_object(out, "require-instance", "value", type->info.inst.req == -1 ? "false" : "true", 1, NULL);
        }
        break;
    case LY_TYPE_INT8:
        ly_print(out, "\"basetype\":\"int8\"");
        goto int_range;
    case LY_TYPE_INT16:
        ly_print(out, "\"basetype\":\"int16\"");
        goto int_range;
    case LY_TYPE_INT32:
        ly_print(out, "\"basetype\":\"int32\"");
        goto int_range;
    case LY_TYPE_INT64:
        ly_print(out, "\"basetype\":\"int64\"");
        goto int_range;
    case LY_TYPE_UINT8:
        ly_print(out, "\"basetype\":\"uint8\"");
        goto int_range;
    case LY_TYPE_UINT16:
        ly_print(out, "\"basetype\":\"uint16\"");
        goto int_range;
    case LY_TYPE_UINT32:
        ly_print(out, "\"basetype\":\"uint32\"");
        goto int_range;
    case LY_TYPE_UINT64:
        ly_print(out, "\"basetype\":\"uint64\"");

int_range:
        jsons_print_typerestr(out, type->info.num.range, "range", NULL);
        break;
    case LY_TYPE_LEAFREF:
        ly_print(out, "\"basetype\":\"leafref\"");
        jsons_print_text(out, "path", "value", type->info.lref.path, 0, NULL);
        for (node = (struct lys_node*)type->info.lref.target; node && node->parent; node = lys_parent(node));
        if (node) {
            mod = node->module;
            ly_print(out, ",\"target-schema\":\"%s%s%s\"", mod->name, mod->rev_size ? "@" : "", mod->rev_size ? mod->rev[0].date : "");
        }
        ly_print(out, "}");
        if (type->info.lref.req) {
            jsons_print_object(out, "require-instance", "value", type->info.lref.req == -1 ? "false" : "true", 1, NULL);
        }
        break;
    case LY_TYPE_STRING:
        ly_print(out, "\"basetype\":\"string\"");
        jsons_print_typerestr(out, type->info.str.length, "length", NULL);
        if (type->info.str.pat_count) {
            ly_print(out, ",\"patterns\":[");
            f = 1;
            for (i = 0; i < type->info.str.pat_count; ++i) {
                jsons_print_typerestr(out, &type->info.str.patterns[i], NULL, &f);
            }
            ly_print(out, "]");
        }

        break;
    case LY_TYPE_UNION:
        ly_print(out, "\"basetype\":\"union\"");
        ly_print(out, ",\"types\":[");
        f = 1;
        for (i = 0; i < type->info.uni.count; ++i) {
            jsons_print_type_(out, &type->info.uni.types[i], 0, &f);
        }
        ly_print(out, "]");
        break;
    default:
        /* unused outside libyang, we never should be here */
        LOGINT(type->parent->module->ctx);
        break;
    }

    if (type->der) {
        ly_print(out, ",\"derived-from\":");
        if (type->der->module) {
            mod = type->der->module;
            ly_print(out, "\"%s%s%s:%s\"",
                     mod->name, mod->rev_size ? "@" : "", mod->rev_size ? mod->rev[0].date : "",
                     type->der->name);
        } else {
            ly_print(out, "\"%s\"", type->der->name);
        }
    }
    ly_print(out, "}");
    if (first) {
        (*first) = 0;
    }
}

static void
jsons_print_type(struct lyout *out, const struct lys_type *type, int *first)
{
    return jsons_print_type_(out, type, 1, first);
}

static void
jsons_print_typedef(struct lyout *out, const struct lys_tpdf *tpdf, int *first)
{
    int f;

    ly_print(out, "%s\"%s\":{", (first && (*first)) ? "" : ",", tpdf->name);
    f = 1;
    jsons_print_type(out, &tpdf->type, &f);
    jsons_print_text(out, "description", "text", tpdf->dsc, 1, NULL);
    jsons_print_text(out, "reference", "text", tpdf->ref, 1, NULL);
    jsons_print_status(out, tpdf->flags, NULL);
    jsons_print_object(out, "units", "name", tpdf->units, 1, NULL);
    jsons_print_object(out, "default", "value", tpdf->dflt, 1, NULL);
    ly_print(out, "}");

    if (first) {
        (*first) = 0;
    }
}

static void
jsons_print_typedefs(struct lyout *out, const struct lys_tpdf *tpdf, uint8_t tpdf_size, int *first)
{
    int i;

    if (!tpdf_size) {
        return;
    }

    ly_print(out, "%s\"typedefs\":[", (first && (*first)) ? "" : ",");
    for (i = 0; i < tpdf_size; ++i) {
        ly_print(out, "%s\"%s\"", i ? "," : "", tpdf[i].name);
    }
    ly_print(out, "]");
    if (first) {
        (*first) = 0;
    }
}

static void
jsons_print_min(struct lyout *out, uint32_t min, int *first)
{
    ly_print(out, "%s\"min-elements\":{\"value\":%u}", (first && (*first)) ? "" : ",", min);
}

static void
jsons_print_max(struct lyout *out, uint32_t max, int *first)
{
    ly_print(out, "%s\"max-elements\":{\"value\":%u}", (first && (*first)) ? "" : ",", max);
}

static void
jsons_print_uniques(struct lyout *out, const struct lys_unique *unique, uint8_t unique_size, int *first)
{
    int i, j;

    if (!unique_size) {
        return;
    }

    ly_print(out, "%s\"uniques\":[", (first && (*first)) ? "" : ",");
    for (i = 0; i < unique_size; ++i) {
        ly_print(out, "%s[", i ? "," : "");
        for (j = 0; j < unique[i].expr_size; ++j) {
            ly_print(out, "%s\"%s\"", j ? "," : "", unique[i].expr[j]);
        }
        ly_print(out, "]");
    }
    ly_print(out, "]");
    if (first) {
        (*first) = 0;
    }
}

static void
jsons_print_defaults(struct lyout *out, const char **dflts, uint8_t dflts_size, int *first)
{
    int i;

    if (!dflts_size) {
        return;
    }

    ly_print(out, "%s\"defaults\":[", (first && (*first)) ? "" : ",");
    for (i = 0; i < dflts_size; ++i) {
        ly_print(out, "%s\"%s\"", i ? "," : "", dflts[i]);
    }
    ly_print(out, "]");
    if (first) {
        (*first) = 0;
    }
}

static void
jsons_print_revisions(struct lyout *out, const struct lys_revision *rev, uint8_t rev_size, int *first)
{
    int i, f;

    if (!rev_size) {
        return;
    }

    ly_print(out, "%s\"revision\":{", (first && (*first)) ? "" : ",");
    for (i = 0; i < rev_size; ++i) {
        ly_print(out, "%s\"%s\":{", i ? "," : "", rev[i].date);
        f = 1;
        jsons_print_text(out, "description", "text", rev[i].dsc, 1, &f);
        jsons_print_text(out, "reference", "text", rev[i].ref, 1, &f);
        ly_print(out, "}");
    }
    ly_print(out, "}");
    if (first) {
        (*first) = 0;
    }
}

static void
jsons_print_imports_(struct lyout *out, const struct lys_submodule *submodule,
                     const struct lys_import *imp, uint8_t imp_size, char **label)
{
    int i, j = 1, f;
    char *str;

    if (imp_size && (*label)) {
        ly_print(out, *label);
        free(*label);
        (*label) = NULL;
        j = 0;
    }
    for (i = 0; i < imp_size; ++i) {
        ly_print(out, "%s\"%s%s%s\":{", i + j ? "," : "", imp[i].module->name,
                 imp[i].rev[0] ? "@" : "", imp[i].rev);
        f = 1;
        jsons_print_object(out, "prefix", "value", imp[i].prefix, 1, &f);
        jsons_print_text(out, "description", "text", imp[i].dsc, 1, &f);
        jsons_print_text(out, "reference", "text", imp[i].ref, 1, &f);
        if (submodule) {
            ly_print(out, ",\"from-submodule\":\"%s%s%s\"", submodule->name,
                     submodule->rev_size ? "@" : "", submodule->rev_size ? submodule->rev[0].date : "");
        }
        if (asprintf(&str, "%s%s%s", imp[i].module->name, imp[i].module->rev_size ? "@" : "", imp[i].module->rev_size ? imp[i].module->rev[0].date : "") == -1) {
            LOGMEM(NULL);
            return;
        }
        jsons_print_text(out, "resolves-to", "module", str, 1, &f);
        free(str);
        ly_print(out, "}");
    }
}

static void
jsons_print_imports(struct lyout *out, const struct lys_module *mod, int *first)
{
    char *str;

    if (!mod->imp_size && !mod->inc_size) {
        return;
    }

    if (asprintf(&str, "%s\"import\":{", (first && (*first)) ? "" : ",") == -1) {
        LOGMEM(mod->ctx);
        return;
    }
    jsons_print_imports_(out, NULL, mod->imp, mod->imp_size, &str);
    /* FIXME key duplication in case multiple submodules imports the same module,
     * but the question is if it is needed to print imports even from submodules -
     * similar code should be then added even for typedefs or identities
    for (i = 0; i < mod->inc_size; ++i) {
        jsons_print_imports_(out, mod->inc[i].submodule, mod->inc[i].submodule->imp, mod->inc[i].submodule->imp_size, &str);
    }
    */
    if (str) {
        free(str);
    } else {
        ly_print(out, "}");
    }
    if (first) {
        (*first) = 0;
    }
}

static void
jsons_print_includes(struct lyout *out, const struct lys_include *inc, uint8_t inc_size, int *first)
{
    int i, f;

    if (!inc_size) {
        return;
    }

    ly_print(out, "%s\"include\":{", (first && (*first)) ? "" : ",");
    for (i = 0; i < inc_size; ++i) {
        ly_print(out, "%s\"%s%s%s\":{", i ? "," : "", inc[i].submodule->name,
                 inc[i].rev[0] ? "@" : "", inc[i].rev);
        f = 1;
        jsons_print_text(out, "description", "text", inc[i].dsc, 1, &f);
        jsons_print_text(out, "reference", "text", inc[i].ref, 1, &f);
        ly_print(out, "}");
    }
    ly_print(out, "}");
    if (first) {
        (*first) = 0;
    }
}

static void
jsons_print_augment(struct lyout *out, const struct lys_node_augment *aug, uint8_t aug_size, int *first)
{
    int i, f;

    if (!aug_size) {
        return;
    }

    ly_print(out, "%s\"augment\":{", (first && (*first)) ? "" : ",");
    for (i = 0; i < aug_size; ++i) {
        ly_print(out, "%s\"%s\":{", i ? "," : "", aug[i].target_name);
        f = 1;
        jsons_print_text(out, "description", "text", aug[i].dsc, 1, &f);
        jsons_print_text(out, "reference", "text", aug[i].ref, 1, &f);
        jsons_print_status(out, aug[i].flags, &f);
        jsons_print_iffeatures(out, aug[i].module, aug[i].iffeature, aug[i].iffeature_size, &f);
        jsons_print_when(out, aug[i].when, &f);
        jsons_print_data(out, aug->module, aug->child, &f);
        jsons_print_actions(out, aug->child, &f);
        jsons_print_notifs(out, aug->child, &f);
        ly_print(out, "}");
    }
    ly_print(out, "}");
    if (first) {
        (*first) = 0;
    }

    if (first) {
        ly_print(out, "\n");
    }
}

static void
jsons_print_deviation(struct lyout *out, const struct lys_deviation *dev, uint8_t dev_size, int *first)
{
    int i, j, f, f2;

    if (!dev_size) {
        return;
    }

    ly_print(out, "%s\"deviations\":{", (first && (*first)) ? "" : ",");
    for (i = 0; i < dev_size; ++i) {
        ly_print(out, "%s\"%s\":{", i ? "," : "", dev[i].target_name);
        f = 1;
        jsons_print_text(out, "description", "text", dev[i].dsc, 1, &f);
        jsons_print_text(out, "reference", "text", dev[i].ref, 1, &f);
        if (dev[i].deviate_size) {
            ly_print(out, "%s\"deviates\":[", f ? "" : ",");
            f = 0;
            f2 = 1;
            for (j = 0; j < dev[i].deviate_size; ++j) {
                ly_print(out, "%s{", j ? "" : ",");
                jsons_print_config(out, dev[i].deviate[j].flags, &f2);
                jsons_print_defaults(out, dev[i].deviate[j].dflt, dev[i].deviate[j].dflt_size, &f2);
                jsons_print_mand(out, dev[i].deviate[j].flags, &f2);
                if (dev[i].deviate[j].min_set) {
                    ly_print(out, "%s\"min-elements\":{\"value\":%u}", f2 ? "" : ",", dev[i].deviate[j].min);
                    f2 = 0;
                }
                if (dev[i].deviate[j].max_set) {
                    ly_print(out, "%s\"max-elements\":{\"value\":%u}", f2 ? "" : ",", dev[i].deviate[j].max);
                    f2 = 0;
                }
                jsons_print_musts(out, dev[i].deviate[j].must, dev[i].deviate[j].must_size, &f2);
                jsons_print_type(out, dev[i].deviate[j].type, &f2);
                jsons_print_uniques(out, dev[i].deviate[j].unique, dev[i].deviate[j].unique_size, &f2);
                jsons_print_text(out, "units", "name", dev[i].deviate[j].units, 1, &f2);
                ly_print(out, "}");
            }
            ly_print(out, "]");
        }
        ly_print(out, "}");
    }
    ly_print(out, "}");
    if (first) {
        (*first) = 0;
    }
}

static void
jsons_print_identity(struct lyout *out, const struct lys_ident *ident, int *first)
{
    int f = 1, j;
    struct lys_module *mod;

    ly_print(out, "%s\"%s\":{", (first && (*first)) ? "" : ",", ident->name);
    if (ident->base_size) {
        ly_print(out, "\"bases\":[");
        f = 0;
        for (j = 0; j < ident->base_size; ++j) {
            mod = ident->base[j]->module;
            ly_print(out, "%s\"%s%s%s:%s\"", j ? "," : "",
                     mod->name, mod->rev_size ? "@" : "", mod->rev_size ? mod->rev[0].date : "",
                     ident->base[j]->name);
        }
        ly_print(out, "]");
    }
    jsons_print_text(out, "description", "text", ident->dsc, 1, &f);
    jsons_print_text(out, "reference", "text", ident->ref, 1, &f);
    jsons_print_status(out, ident->flags, &f);
    jsons_print_iffeatures(out, ident->module, ident->iffeature, ident->iffeature_size, &f);

    ly_print(out, "}");
    if (first) {
        (*first) = 0;
    }
}

static void
jsons_print_identities(struct lyout *out, const struct lys_ident *ident, uint16_t ident_size, int *first)
{
    int i;

    if (!ident_size) {
        return;
    }

    ly_print(out, "%s\"identities\":[", (first && (*first)) ? "" : ",");
    for (i = 0; i < ident_size; ++i) {
        ly_print(out, "%s\"%s\"", i ? "," : "", ident[i].name);
    }
    ly_print(out, "]");
    if (first) {
        (*first) = 0;
    }
}

static void
jsons_print_feature(struct lyout *out, const struct lys_feature *feat, int *first)
{
    int f = 1;
    unsigned int j;

    ly_print(out, "%s\"%s\":{", (first && (*first)) ? "" : ",", feat->name);
    jsons_print_text(out, "description", "text", feat->dsc, 1, &f);
    jsons_print_text(out, "reference", "text", feat->ref, 1, &f);
    jsons_print_status(out, feat->flags, &f);
    jsons_print_iffeatures(out, feat->module, feat->iffeature, feat->iffeature_size, &f);
    if (feat->depfeatures && feat->depfeatures->number) {
        ly_print(out, "%s\"depending-features\":[", f ? "" : ",");
        for (j = 0; j < feat->depfeatures->number; ++j) {
            ly_print(out, "%s\"%s\"", j ? "," : "", ((struct lys_feature*)(feat->depfeatures->set.g[j]))->name);
        }
        ly_print(out, "]");
    }
    ly_print(out, "}");
    if (first) {
        (*first) = 0;
    }
}

static void
jsons_print_features(struct lyout *out, const struct lys_feature *feat, uint8_t feat_size, int *first)
{
    int i;

    if (!feat_size) {
        return;
    }

    ly_print(out, "%s\"features\":[", (first && (*first)) ? "" : ",");
    for (i = 0; i < feat_size; ++i) {
        ly_print(out, "%s\"%s\"", i ? "," : "", feat[i].name);
    }
    ly_print(out, "]");
    if (first) {
        (*first) = 0;
    }
}

static void
jsons_print_data_(struct lyout *out, const struct lys_module *mod, struct lys_node *data, int *first)
{
    struct lys_node *node;
    int mask = LYS_CONTAINER | LYS_CHOICE | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYXML | LYS_CASE | LYS_USES | LYS_ANYDATA;

    LY_TREE_FOR(data, node) {
        if (!(node->nodetype & mask)) {
            continue;
        }
        if (node->nodetype & (LYS_USES)) {
            jsons_print_data_(out, mod, node->child, first);
        } else if (lys_main_module(mod) == lys_main_module(node->module)) {
            jsons_print_object(out, node->name, "nodetype", jsons_nodetype_str(node->nodetype), 0, first);
            if (node->module->type) {
                ly_print(out, ",\"included-from\":\"%s\"", node->module->name);
            }
            ly_print(out, "}");
        } else {
            ly_print(out, "\"%s:%s\":{\"nodetype\":\"%s\"}", lys_main_module(node->module)->name, node->name,
                     jsons_nodetype_str(node->nodetype));
            (*first) = 0;
        }
    }
}

static void
jsons_print_data(struct lyout *out, const struct lys_module *mod, struct lys_node *data, int *first)
{
    int f;

    ly_print(out, "%s\"data\":{", (first && (*first)) ? "" : ",");
    f = 1;
    jsons_print_data_(out, mod, data, &f);
    ly_print(out, "}");
    if (first) {
        (*first) = 0;
    }
}

static void
jsons_print_nodes_uses_(struct lyout *out, struct lys_node *data, const char *label, int mask, int *top_first, int *first)
{
    struct lys_node *node;
    LY_TREE_FOR(data, node) {
        if (!(node->nodetype & mask)) {
            continue;
        }
        if (node->nodetype & (LYS_USES)) {
            jsons_print_nodes_uses_(out, node->child, label, mask, top_first, first);
        } else {
            if (*first) {
                ly_print(out, "%s\"%s\":[", (top_first && (*top_first)) ? "" : ",", label);
            }
            ly_print(out, "%s\"%s\"", (*first) ? "" : ",", node->name);
            (*first) = 0;
        }
    }
}

static void
jsons_print_nodes_(struct lyout *out, struct lys_node *data, const char *label, int mask, int *first)
{
    int f = 1;
    struct lys_node *node;

    LY_TREE_FOR(data, node) {
        if (!(node->nodetype & mask)) {
            continue;
        }
        if (node->nodetype & (LYS_USES)) {
            jsons_print_nodes_uses_(out, node->child, label, mask, first, &f);
        } else {
            if (f) {
                ly_print(out, "%s\"%s\":[", (first && (*first)) ? "" : ",", label);
            }
            ly_print(out, "%s\"%s\"", f ? "" : ",", node->name);
            f = 0;
        }
    }
    if (!f) {
        ly_print(out, "]");
        if (first) {
            (*first) = 0;
        }
    }
}

static void
jsons_print_groupings(struct lyout *out, struct lys_node *data, int *first)
{
    jsons_print_nodes_(out, data, "groupings", LYS_GROUPING, first);
}

static void
jsons_print_rpcs(struct lyout *out, struct lys_node *data, int *first)
{
    jsons_print_nodes_(out, data, "rpcs", LYS_RPC, first);
}

static void
jsons_print_actions(struct lyout *out, struct lys_node *data, int *first)
{
    jsons_print_nodes_(out, data, "actions", LYS_ACTION, first);
}

static void
jsons_print_notifs(struct lyout *out, struct lys_node *data, int *first)
{
    jsons_print_nodes_(out, data, "notifications", LYS_NOTIF, first);
}

static void
jsons_print_module(struct lyout *out, const struct lys_module *module)
{
    ly_print(out, "{\"%s\":{", module->name);
    ly_print(out, "\"namespace\":\"%s\"", module->ns);
    ly_print(out, ",\"prefix\":\"%s\"", module->prefix);
    jsons_print_text(out, "description", "text", module->dsc, 1, NULL);
    jsons_print_text(out, "reference", "text", module->ref, 1, NULL);
    jsons_print_text(out, "organization", "text", module->org, 1, NULL);
    jsons_print_text(out, "contact", "text", module->contact, 1, NULL);
    jsons_print_object(out, "yang-version", "value", (module->version == LYS_VERSION_1_1) ? "1.1" : "1.0", 1, NULL);
    /* TODO deviated-by */

    jsons_print_revisions(out, module->rev, module->rev_size, NULL);
    jsons_print_includes(out, module->inc, module->inc_size, NULL);
    jsons_print_imports(out, module, NULL);
    jsons_print_typedefs(out, module->tpdf, module->tpdf_size, NULL);
    jsons_print_identities(out, module->ident, module->ident_size, NULL);
    jsons_print_features(out, module->features, module->features_size, NULL);
    jsons_print_augment(out, module->augment, module->augment_size, NULL);
    jsons_print_deviation(out, module->deviation, module->deviation_size, NULL);

    jsons_print_groupings(out, module->data, NULL);
    jsons_print_data(out, module, module->data, NULL);
    jsons_print_rpcs(out, module->data, NULL);
    jsons_print_notifs(out, module->data, NULL);

    /* close the module */
    ly_print(out, "}}");
}

static void
jsons_print_submodule(struct lyout *out, const struct lys_submodule *module)
{
    ly_print(out, "{\"%s\":{", module->name);
    ly_print(out, "\"belongs-to\":\"%s\"", module->belongsto->name);
    jsons_print_text(out, "description", "text", module->dsc, 1, NULL);
    jsons_print_text(out, "reference", "text", module->ref, 1, NULL);
    jsons_print_text(out, "organization", "text", module->org, 1, NULL);
    jsons_print_text(out, "contact", "text", module->contact, 1, NULL);
    jsons_print_object(out, "yang-version", "value", (module->version == LYS_VERSION_1_1) ? "1.1" : "1.0", 1, NULL);
    /* TODO deviated-by */

    jsons_print_revisions(out, module->rev, module->rev_size, NULL);
    jsons_print_includes(out, module->inc, module->inc_size, NULL);
    jsons_print_imports(out, (struct lys_module *)module, NULL);
    jsons_print_typedefs(out, module->tpdf, module->tpdf_size, NULL);
    jsons_print_identities(out, module->ident, module->ident_size, NULL);
    jsons_print_features(out, module->features, module->features_size, NULL);
    jsons_print_augment(out, module->augment, module->augment_size, NULL);
    jsons_print_deviation(out, module->deviation, module->deviation_size, NULL);

    /* close the module */
    ly_print(out, "}}");
}

static void
jsons_print_container(struct lyout *out, const struct lys_node *node, int *first)
{
    struct lys_node_container *cont = (struct lys_node_container *)node;

    jsons_print_object(out, node->name, "nodetype", jsons_nodetype_str(node->nodetype), 0, first);
    ly_print(out, ",\"module\":\"%s\"", lys_main_module(node->module)->name);
    if (node->module->type) {
        ly_print(out, ",\"included-from\":\"%s\"", node->module->name);
    }
    jsons_print_text(out, "description", "text", cont->dsc, 1, NULL);
    jsons_print_text(out, "reference", "text", cont->ref, 1, NULL);
    jsons_print_config(out, cont->flags, NULL);
    jsons_print_status(out, cont->flags, NULL);
    jsons_print_text(out, "presence", "value", cont->presence, 1, NULL);
    jsons_print_iffeatures(out, cont->module, cont->iffeature, cont->iffeature_size, NULL);
    jsons_print_when(out, cont->when, NULL);
    jsons_print_musts(out, cont->must, cont->must_size, NULL);
    jsons_print_typedefs(out, cont->tpdf, cont->tpdf_size, NULL);

    jsons_print_groupings(out, cont->child, NULL);
    jsons_print_data(out, cont->module, cont->child, NULL);
    jsons_print_actions(out, cont->child, NULL);
    jsons_print_notifs(out, cont->child, NULL);
    ly_print(out, "}");
}

static void
jsons_print_choice(struct lyout *out, const struct lys_node *node, int *first)
{
    struct lys_node_choice *choice = (struct lys_node_choice *)node;

    jsons_print_object(out, node->name, "nodetype", jsons_nodetype_str(node->nodetype), 0, first);
    ly_print(out, ",\"module\":\"%s\"", lys_main_module(node->module)->name);
    if (node->module->type) {
        ly_print(out, ",\"included-from\":\"%s\"", node->module->name);
    }
    jsons_print_text(out, "description", "text", choice->dsc, 1, NULL);
    jsons_print_text(out, "reference", "text", choice->ref, 1, NULL);
    jsons_print_config(out, choice->flags, NULL);
    jsons_print_status(out, choice->flags, NULL);
    jsons_print_mand(out, choice->flags, NULL);
    if (choice->dflt) {
        jsons_print_defaults(out, &choice->dflt->name, 1, NULL);
    }
    jsons_print_iffeatures(out, choice->module, choice->iffeature, choice->iffeature_size, NULL);
    jsons_print_when(out, choice->when, NULL);

    jsons_print_data(out, choice->module, choice->child, NULL);
    ly_print(out, "}");
}

static void
jsons_print_leaf(struct lyout *out, const struct lys_node *node, int *first)
{
    struct lys_node_leaf *leaf = (struct lys_node_leaf *)node;

    jsons_print_object(out, node->name, "nodetype", jsons_nodetype_str(node->nodetype), 0, first);
    ly_print(out, ",\"module\":\"%s\"", lys_main_module(node->module)->name);
    if (node->module->type) {
        ly_print(out, ",\"included-from\":\"%s\"", node->module->name);
    }
    jsons_print_text(out, "description", "text", leaf->dsc, 1, NULL);
    jsons_print_text(out, "reference", "text", leaf->ref, 1, NULL);
    jsons_print_status(out, leaf->flags, NULL);
    jsons_print_config(out, leaf->flags, NULL);
    jsons_print_mand(out, leaf->flags, NULL);
    jsons_print_type(out, &leaf->type, NULL);
    jsons_print_text(out, "units", "name", leaf->units, 1, NULL);
    if (leaf->dflt) {
        jsons_print_defaults(out, &leaf->dflt, 1, NULL);
    }
    jsons_print_iffeatures(out, leaf->module, leaf->iffeature, leaf->iffeature_size, NULL);
    jsons_print_when(out, leaf->when, NULL);
    jsons_print_musts(out, leaf->must, leaf->must_size, NULL);
    ly_print(out, "}");
}

static void
jsons_print_leaflist(struct lyout *out, const struct lys_node *node, int *first)
{
    struct lys_node_leaflist *llist = (struct lys_node_leaflist *)node;

    jsons_print_object(out, node->name, "nodetype", jsons_nodetype_str(node->nodetype), 0, first);
    ly_print(out, ",\"module\":\"%s\"", lys_main_module(node->module)->name);
    if (node->module->type) {
        ly_print(out, ",\"included-from\":\"%s\"", node->module->name);
    }
    jsons_print_text(out, "description", "text", llist->dsc, 1, NULL);
    jsons_print_text(out, "reference", "text", llist->ref, 1, NULL);
    jsons_print_status(out, llist->flags, NULL);
    jsons_print_config(out, llist->flags, NULL);
    jsons_print_ordering(out, llist->flags, NULL);
    jsons_print_type(out, &llist->type, NULL);
    jsons_print_text(out, "units", "name", llist->units, 1, NULL);
    jsons_print_defaults(out, llist->dflt, llist->dflt_size, NULL);
    if (llist->min) {
        jsons_print_min(out, llist->min, NULL);
    }
    if (llist->max) {
        jsons_print_max(out, llist->max, NULL);
    }
    jsons_print_iffeatures(out, llist->module, llist->iffeature, llist->iffeature_size, NULL);
    jsons_print_when(out, llist->when, NULL);
    jsons_print_musts(out, llist->must, llist->must_size, NULL);
    ly_print(out, "}");
}

static void
jsons_print_list(struct lyout *out, const struct lys_node *node, int *first)
{
    uint8_t i;
    struct lys_node_list *list = (struct lys_node_list *)node;

    jsons_print_object(out, node->name, "nodetype", jsons_nodetype_str(node->nodetype), 0, first);
    ly_print(out, ",\"module\":\"%s\"", lys_main_module(node->module)->name);
    if (node->module->type) {
        ly_print(out, ",\"included-from\":\"%s\"", node->module->name);
    }
    jsons_print_text(out, "description", "text", list->dsc, 1, NULL);
    jsons_print_text(out, "reference", "text", list->ref, 1, NULL);
    jsons_print_status(out, list->flags, NULL);
    jsons_print_config(out, list->flags, NULL);
    jsons_print_ordering(out, list->flags, NULL);
    if (list->min) {
        jsons_print_min(out, list->min, NULL);
    }
    if (list->max) {
        jsons_print_max(out, list->max, NULL);
    }
    jsons_print_iffeatures(out, list->module, list->iffeature, list->iffeature_size, NULL);
    jsons_print_when(out, list->when, NULL);
    jsons_print_musts(out, list->must, list->must_size, NULL);
    ly_print(out, ",\"keys\":[");
    for (i = 0; i < list->keys_size; ++i) {
        ly_print(out, "%s\"%s\"", i ? "," : "", list->keys[i]->name);
    }
    ly_print(out, "]");
    jsons_print_uniques(out, list->unique, list->unique_size, NULL);
    jsons_print_typedefs(out, list->tpdf, list->tpdf_size, NULL);

    jsons_print_groupings(out, list->child, NULL);
    jsons_print_data(out, list->module, list->child, NULL);
    jsons_print_actions(out, list->child, NULL);
    jsons_print_notifs(out, list->child, NULL);
    ly_print(out, "}");
}

static void
jsons_print_anydata(struct lyout *out, const struct lys_node *node, int *first)
{
    struct lys_node_anydata *any = (struct lys_node_anydata *)node;

    if (!lys_parent(node) && !strcmp(node->name, "config") && !strcmp(node->module->name, "ietf-netconf")) {
        /* node added by libyang, not actually in the model */
        return;
    }

    jsons_print_object(out, node->name, "nodetype", jsons_nodetype_str(node->nodetype), 0, first);
    ly_print(out, ",\"module\":\"%s\"", lys_main_module(node->module)->name);
    if (node->module->type) {
        ly_print(out, ",\"included-from\":\"%s\"", node->module->name);
    }
    jsons_print_text(out, "description", "text", any->dsc, 1, NULL);
    jsons_print_text(out, "reference", "text", any->ref, 1, NULL);
    jsons_print_config(out, any->flags, NULL);
    jsons_print_status(out, any->flags, NULL);
    jsons_print_mand(out, any->flags, NULL);
    jsons_print_iffeatures(out, any->module, any->iffeature, any->iffeature_size, NULL);
    jsons_print_when(out, any->when, NULL);
    jsons_print_musts(out, any->must, any->must_size, NULL);

    /* TODO print content */

    ly_print(out, "}");
}

static void
jsons_print_grouping(struct lyout *out, const struct lys_node *node, int *first)
{
    struct lys_node_grp *group = (struct lys_node_grp *)node;

    jsons_print_object(out, node->name, "module", lys_main_module(node->module)->name, 0, first);
    if (node->module->type) {
        ly_print(out, ",\"included-from\":\"%s\"", node->module->name);
    }
    jsons_print_text(out, "description", "text", group->dsc, 1, NULL);
    jsons_print_text(out, "reference", "text", group->ref, 1, NULL);
    jsons_print_status(out, group->flags, NULL);
    jsons_print_typedefs(out, group->tpdf, group->tpdf_size, NULL);

    jsons_print_groupings(out, group->child, NULL);
    jsons_print_data(out, group->module, group->child, NULL);
    jsons_print_actions(out, group->child, NULL);
    jsons_print_notifs(out, group->child, NULL);
    ly_print(out, "}");
}

static void
jsons_print_case(struct lyout *out, const struct lys_node *node, int *first)
{
    struct lys_node_case *cas = (struct lys_node_case *)node;

    jsons_print_object(out, node->name, "nodetype", jsons_nodetype_str(node->nodetype), 0, first);
    ly_print(out, ",\"module\":\"%s\"", lys_main_module(node->module)->name);
    if (node->module->type) {
        ly_print(out, ",\"included-from\":\"%s\"", node->module->name);
    }
    jsons_print_text(out, "description", "text", cas->dsc, 1, NULL);
    jsons_print_text(out, "reference", "text", cas->ref, 1, NULL);
    jsons_print_config(out, cas->flags, NULL);
    jsons_print_status(out, cas->flags, NULL);
    jsons_print_iffeatures(out, cas->module, cas->iffeature, cas->iffeature_size, NULL);
    jsons_print_when(out, cas->when, NULL);

    jsons_print_data(out, cas->module, cas->child, NULL);
    ly_print(out, "}");
}

static void
jsons_print_input(struct lyout *out, const struct lys_node *node, int *first)
{
    struct lys_node_inout *input = (struct lys_node_inout *)node;

    jsons_print_object(out, "input", "module", lys_main_module(node->module)->name, 0, first);
    jsons_print_typedefs(out, input->tpdf, input->tpdf_size, NULL);
    jsons_print_musts(out, input->must, input->must_size, NULL);
    jsons_print_groupings(out, input->child, NULL);
    jsons_print_data(out, input->module, input->child, NULL);
    ly_print(out, "}");
}

static void
jsons_print_output(struct lyout *out, const struct lys_node *node, int *first)
{
    struct lys_node_inout *output = (struct lys_node_inout *)node;

    jsons_print_object(out, "output", "module", lys_main_module(node->module)->name, 0, first);
    jsons_print_typedefs(out, output->tpdf, output->tpdf_size, NULL);
    jsons_print_musts(out, output->must, output->must_size, NULL);
    jsons_print_groupings(out, output->child, NULL);
    jsons_print_data(out, output->module, output->child, NULL);
    ly_print(out, "}");
}

static void
jsons_print_notif(struct lyout *out, const struct lys_node *node, int *first)
{
    struct lys_node_notif *ntf = (struct lys_node_notif *)node;

    jsons_print_object(out, node->name, "nodetype", jsons_nodetype_str(node->nodetype), 0, first);
    ly_print(out, ",\"module\":\"%s\"", lys_main_module(node->module)->name);
    if (node->module->type) {
        ly_print(out, ",\"included-from\":\"%s\"", node->module->name);
    }
    jsons_print_text(out, "description", "text", ntf->dsc, 1, NULL);
    jsons_print_text(out, "reference", "text", ntf->ref, 1, NULL);
    jsons_print_status(out, ntf->flags, NULL);
    jsons_print_iffeatures(out, ntf->module, ntf->iffeature, ntf->iffeature_size, NULL);
    jsons_print_typedefs(out, ntf->tpdf, ntf->tpdf_size, NULL);
    jsons_print_musts(out, ntf->must, ntf->must_size, NULL);

    jsons_print_groupings(out, ntf->child, NULL);
    jsons_print_data(out, ntf->module, ntf->child, NULL);
    ly_print(out, "}");
}

static void
jsons_print_rpc(struct lyout *out, const struct lys_node *node, int *first)
{
    const struct lys_node *child;
    struct lys_node_rpc_action *rpc = (struct lys_node_rpc_action *)node;

    jsons_print_object(out, node->name, "nodetype", jsons_nodetype_str(node->nodetype), 0, first);
    ly_print(out, ",\"module\":\"%s\"", lys_main_module(node->module)->name);
    if (node->module->type) {
        ly_print(out, ",\"included-from\":\"%s\"", node->module->name);
    }
    jsons_print_text(out, "description", "text", rpc->dsc, 1, NULL);
    jsons_print_text(out, "reference", "text", rpc->ref, 1, NULL);
    jsons_print_status(out, rpc->flags, NULL);
    jsons_print_iffeatures(out, rpc->module, rpc->iffeature, rpc->iffeature_size, NULL);
    jsons_print_typedefs(out, rpc->tpdf, rpc->tpdf_size, NULL);
    jsons_print_groupings(out, rpc->child, NULL);

    LY_TREE_FOR(node->child, child) {
        if (!(child->nodetype & LYS_INPUT)) {
            continue;
        }
        jsons_print_input(out, child, NULL);
        break;
    }

    LY_TREE_FOR(node->child, child) {
        if (!(child->nodetype & LYS_OUTPUT)) {
            continue;
        }
        jsons_print_output(out, child, NULL);
        break;
    }
    ly_print(out, "}");
}

static void
jsons_print_action(struct lyout *out, const struct lys_node *node, int *first)
{
    const struct lys_node *child;
    struct lys_node_rpc_action *act = (struct lys_node_rpc_action *)node;

    jsons_print_object(out, node->name, "nodetype", jsons_nodetype_str(node->nodetype), 0, first);
    ly_print(out, ",\"module\":\"%s\"", lys_main_module(node->module)->name);
    if (node->module->type) {
        ly_print(out, ",\"included-from\":\"%s\"", node->module->name);
    }
    jsons_print_text(out, "description", "text", act->dsc, 1, NULL);
    jsons_print_text(out, "reference", "text", act->ref, 1, NULL);
    jsons_print_status(out, act->flags, NULL);
    jsons_print_iffeatures(out, act->module, act->iffeature, act->iffeature_size, NULL);
    jsons_print_typedefs(out, act->tpdf, act->tpdf_size, NULL);
    jsons_print_groupings(out, act->child, NULL);

    LY_TREE_FOR(node->child, child) {
        if (!(child->nodetype & LYS_INPUT)) {
            continue;
        }
        jsons_print_input(out, child, NULL);
        break;
    }

    LY_TREE_FOR(node->child, child) {
        if (!(child->nodetype & LYS_OUTPUT)) {
            continue;
        }
        jsons_print_output(out, child, NULL);
        break;
    }
    ly_print(out, "}");
}

int
jsons_print_model(struct lyout *out, const struct lys_module *module, const char *target_schema_path)
{
    int rc = EXIT_SUCCESS;

    if (!target_schema_path) {
        if (module->type == 0) {
            jsons_print_module(out, module);
        } else {
            jsons_print_submodule(out, (struct lys_submodule *)module);
        }
    } else {
        ly_print(out, "{");
        rc = lys_print_target(out, module, target_schema_path,
                              jsons_print_typedef,
                              jsons_print_identity,
                              jsons_print_feature,
                              jsons_print_type,
                              jsons_print_grouping,
                              jsons_print_container,
                              jsons_print_choice,
                              jsons_print_leaf,
                              jsons_print_leaflist,
                              jsons_print_list,
                              jsons_print_anydata,
                              jsons_print_case,
                              jsons_print_notif,
                              jsons_print_rpc,
                              jsons_print_action,
                              jsons_print_input,
                              jsons_print_output);
        ly_print(out, "}");
    }
    ly_print_flush(out);

    return rc;
}
