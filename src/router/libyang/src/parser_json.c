/**
 * @file parser_json.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief JSON data parser for libyang
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
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
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"
#include "common.h"
#include "context.h"
#include "parser.h"
#include "printer.h"
#include "tree_internal.h"
#include "validation.h"
#include "xml_internal.h"

static int
lyjson_isspace(int c)
{
    switch(c) {
    case 0x20: /* space */
    case 0x09: /* horizontal tab */
    case 0x0a: /* line feed or new line */
    case 0x0d: /* carriage return */
        return 1;
    default:
        return 0;
    }
}

static unsigned int
skip_ws(const char *data)
{
    unsigned int len = 0;

    /* skip leading whitespaces */
    while (data[len] && lyjson_isspace(data[len])) {
        len++;
    }

    return len;
}

static char *
lyjson_parse_text(struct ly_ctx *ctx, const char *data, unsigned int *len)
{
#define BUFSIZE 1024

    char buf[BUFSIZE];
    char *result = NULL, *aux;
    int o, size = 0;
    unsigned int r, i;
    int32_t value;

    for (*len = o = 0; data[*len] && data[*len] != '"'; o++) {
        if (o > BUFSIZE - 4) {
            /* add buffer into the result */
            if (result) {
                size = size + o;
                aux = ly_realloc(result, size + 1);
                LY_CHECK_ERR_RETURN(!aux, LOGMEM(ctx), NULL);
                result = aux;
            } else {
                size = o;
                result = malloc((size + 1) * sizeof *result);
                LY_CHECK_ERR_RETURN(!result, LOGMEM(ctx), NULL);
            }
            memcpy(&result[size - o], buf, o);

            /* write again into the beginning of the buffer */
            o = 0;
        }

        if (data[*len] == '\\') {
            /* parse escape sequence */
            (*len)++;
            i = 1;
            switch (data[(*len)]) {
            case '"':
                /* quotation mark */
                value = 0x22;
                break;
            case '\\':
                /* reverse solidus */
                value = 0x5c;
                break;
            case '/':
                /* solidus */
                value = 0x2f;
                break;
            case 'b':
                /* backspace */
                value = 0x08;
                break;
            case 'f':
                /* form feed */
                value = 0x0c;
                break;
            case 'n':
                /* line feed */
                value = 0x0a;
                break;
            case 'r':
                /* carriage return */
                value = 0x0d;
                break;
            case 't':
                /* tab */
                value = 0x09;
                break;
            case 'u':
                /* Basic Multilingual Plane character \uXXXX */
                (*len)++;
                for (value = i = 0; i < 4; i++) {
                    if (isdigit(data[(*len) + i])) {
                        r = (data[(*len) + i] - '0');
                    } else if (data[(*len) + i] > 'F') {
                        r = 10 + (data[(*len) + i] - 'a');
                    } else {
                        r = 10 + (data[(*len) + i] - 'A');
                    }
                    value = (16 * value) + r;
                }
                break;
            default:
                /* invalid escape sequence */
                LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_NONE, NULL, "character escape sequence");
                goto error;

            }
            r = pututf8(ctx, &buf[o], value);
            if (!r) {
                LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_NONE, NULL, "character UTF8 character");
                goto error;
            }
            o += r - 1; /* o is ++ in for loop */
            (*len) += i; /* number of read characters */
        } else if ((unsigned char)(data[*len]) < 0x20) {
            /* In C, char != unsigned char != signed char, so let's work with ASCII explicitly */
            /* control characters must be escaped */
            LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_NONE, NULL, "control character (unescaped)");
            goto error;
        } else {
            /* unescaped character */
            r = copyutf8(ctx, &buf[o], &data[*len]);
            if (!r) {
                goto error;
            }

            o += r - 1;     /* o is ++ in for loop */
            (*len) += r;
        }
    }

#undef BUFSIZE

    if (o) {
        if (result) {
            size = size + o;
            aux = ly_realloc(result, size + 1);
            LY_CHECK_ERR_RETURN(!aux, LOGMEM(ctx), NULL);
            result = aux;
        } else {
            size = o;
            result = malloc((size + 1) * sizeof *result);
            LY_CHECK_ERR_RETURN(!result, LOGMEM(ctx), NULL);
        }
        memcpy(&result[size - o], buf, o);
    }
    if (result) {
        result[size] = '\0';
    } else {
        size = 0;
        result = strdup("");
        LY_CHECK_ERR_RETURN(!result, LOGMEM(ctx), NULL);
    }

    return result;

error:
    free(result);
    return NULL;
}

static unsigned int
lyjson_parse_number(struct ly_ctx *ctx, const char *data)
{
    unsigned int len = 0;

    if (data[len] == '-') {
        ++len;
    }

    if (data[len] == '0') {
        ++len;
    } else if (isdigit(data[len])) {
        ++len;
        while (isdigit(data[len])) {
            ++len;
        }
    } else {
        LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid character in JSON Number value ('%c').", data[len]);
        return 0;
    }

    if (data[len] == '.') {
        ++len;
        if (!isdigit(data[len])) {
            if (data[len]) {
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid character in JSON Number value ('%c').", data[len]);
            } else {
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid character in JSON Number value (EOF).");
            }
            return 0;
        }
        while (isdigit(data[len])) {
            ++len;
        }
    }

    if ((data[len] == 'e') || (data[len] == 'E')) {
        ++len;
        if ((data[len] == '+') || (data[len] == '-')) {
            ++len;
        }
        while (isdigit(data[len])) {
            ++len;
        }
    }

    if (data[len] && (data[len] != ',') && (data[len] != ']') && (data[len] != '}') && !lyjson_isspace(data[len])) {
        LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid character in JSON Number value ('%c').", data[len]);
        return 0;
    }

    return len;
}

static char *
lyjson_convert_enumber(struct ly_ctx *ctx, const char *number, unsigned int num_len, char *e_ptr)
{
    char *ptr, *num;
    const char *number_ptr;
    long int e_val;
    int dot_pos, chars_to_dot, minus;
    unsigned int num_len_no_e;

    if (*number == '-') {
        minus = 1;
        ++number;
        --num_len;
    } else {
        minus = 0;
    }

    num_len_no_e = e_ptr - number;

    errno = 0;
    ++e_ptr;
    e_val = strtol(e_ptr, &ptr, 10);
    if (errno) {
        LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Exponent out-of-bounds in a JSON Number value (%.*s).",
               num_len - (e_ptr - number), e_ptr);
        return NULL;
    } else if (ptr != number + num_len) {
        /* we checked this already */
        LOGINT(ctx);
        return NULL;
    }

    if ((ptr = strnchr(number, '.', num_len_no_e))) {
        dot_pos = ptr - number;
    } else {
        dot_pos = num_len_no_e;
    }

    dot_pos += e_val;

    /* allocate enough memory */
    if (dot_pos < 1) {
        /* (.XXX)XXX[.]XXXX */
        num = malloc((minus ? 1 : 0) + -dot_pos + 2 + (num_len_no_e - (ptr ? 1 : 0)) + 1);
    } else if (dot_pos < (signed)num_len_no_e) {
        /* XXXX(.)XX.XXX */
        num = malloc((minus ? 1 : 0) + num_len_no_e + (ptr ? 0 : 1) + 1);
    } else {
        /* XXX[.]XXXX(XXX.) */
        num = malloc((minus ? 1 : 0) + (dot_pos - (ptr ? 2 : 1)) + 1);
    }
    LY_CHECK_ERR_RETURN(!num, LOGMEM(ctx), NULL);
    if (minus) {
        strcpy(num, "-");
    } else {
        num[0] = '\0';
    }

    if (dot_pos < 1) {
        strcat(num, "0.");
    }
    if (dot_pos < 0) {
        sprintf(num + strlen(num), "%0*d", -dot_pos, 0);
    }

    chars_to_dot = dot_pos;
    for (ptr = num + strlen(num), number_ptr = number; (unsigned)(number_ptr - number) < num_len_no_e; ) {
        if (!chars_to_dot) {
            *ptr = '.';
            ++ptr;
            chars_to_dot = -1;
        } else if (isdigit(*number_ptr)) {
            *ptr = *number_ptr;
            ++ptr;
            ++number_ptr;
            if (chars_to_dot > 0) {
                --chars_to_dot;
            }
        } else if (*number_ptr == '.') {
            ++number_ptr;
        } else {
            LOGINT(ctx);
            free(num);
            return NULL;
        }
    }
    *ptr = '\0';

    if (dot_pos > (signed)num_len_no_e) {
        sprintf(num + strlen(num), "%0*d", dot_pos - num_len_no_e, 0);
    }

    return num;
}

static unsigned int
lyjson_parse_boolean(struct ly_ctx *ctx, const char *data)
{
    unsigned int len = 0;

    if (!strncmp(data, "false", 5)) {
        len = 5;
    } else if (!strncmp(data, "true", 4)) {
        len = 4;
    }

    if (data[len] && data[len] != ',' && data[len] != ']' && data[len] != '}' && !lyjson_isspace(data[len])) {
        LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_NONE, NULL, "JSON literal value (expected true or false)");
        return 0;
    }

    return len;
}

static unsigned int
json_get_anydata(struct lyd_node_anydata *any, const char *data)
{
    struct ly_ctx *ctx = any->schema->module->ctx;
    unsigned int len = 0, c = 0;
    char *str;

    if (data[len] == '"') {
        len = 1;
        str = lyjson_parse_text(ctx, &data[len], &c);
        if (!str) {
            return 0;
        }
        if (data[len + c] != '"') {
            free(str);
            LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_LYD, any,
                   "JSON data (missing quotation-mark at the end of string)");
            return 0;
        }

        any->value.str = lydict_insert_zc(ctx, str);
        any->value_type = LYD_ANYDATA_CONSTSTRING;
        return len + c + 1;
    } else if (data[len] != '{') {
        LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_LYD, any, "anydata/anyxml content (not an object nor string)");
        return 0;
    }

    /* count opening '{' and closing '}' brackets to get the end of the object without its parsing */
    c = len = 0;
    do {
        switch (data[len]) {
        case '{':
            c++;
            break;
        case '}':
            c--;
            break;
        default:
            break;
        }
        len++;
    } while (data[len] && c);
    if (c) {
        LOGVAL(ctx, LYE_EOF, LY_VLOG_LYD, any);
        return 0;
    }

    any->value_type = LYD_ANYDATA_JSON;
    any->value.str = lydict_insert(ctx, data, len);

    return len;
}

static unsigned int
json_get_value(struct lyd_node_leaf_list *leaf, struct lyd_node **first_sibling, const char *data, int options,
               struct unres_data *unres)
{
    struct lyd_node_leaf_list *new;
    struct lys_type *stype;
    struct ly_ctx *ctx;
    unsigned int len = 0, r;
    char *str;

    assert(leaf && data);
    ctx = leaf->schema->module->ctx;

    stype = &((struct lys_node_leaf *)leaf->schema)->type;

    if (leaf->schema->nodetype == LYS_LEAFLIST) {
        /* expecting begin-array */
        if (data[len++] != '[') {
            LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_LYD, leaf, "JSON data (expected begin-array)");
            return 0;
        }

repeat:
        len += skip_ws(&data[len]);
    }

    /* will be changed in case of union */
    leaf->value_type = stype->base;

    if (data[len] == '"') {
        /* string representations */
        ++len;
        str = lyjson_parse_text(ctx, &data[len], &r);
        if (!str) {
            LOGPATH(ctx, LY_VLOG_LYD, leaf);
            return 0;
        }
        leaf->value_str = lydict_insert_zc(ctx, str);
        if (data[len + r] != '"') {
            LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_LYD, leaf,
                   "JSON data (missing quotation-mark at the end of string)");
            return 0;
        }
        len += r + 1;
    } else if (data[len] == '-' || isdigit(data[len])) {
        /* numeric type */
        r = lyjson_parse_number(ctx, &data[len]);
        if (!r) {
            LOGPATH(ctx, LY_VLOG_LYD, leaf);
            return 0;
        }
        /* if it's a number with 'e' or 'E', get rid of it first */
        if ((str = strnchr(&data[len], 'e', r)) || (str = strnchr(&data[len], 'E', r))) {
            str = lyjson_convert_enumber(ctx, &data[len], r, str);
            if (!str) {
                return 0;
            }
            leaf->value_str = lydict_insert_zc(ctx, str);
        } else {
            leaf->value_str = lydict_insert(ctx, &data[len], r);
        }
        len += r;
    } else if (data[len] == 'f' || data[len] == 't') {
        /* boolean */
        r = lyjson_parse_boolean(ctx, &data[len]);
        if (!r) {
            LOGPATH(ctx, LY_VLOG_LYD, leaf);
            return 0;
        }
        leaf->value_str = lydict_insert(ctx, &data[len], r);
        len += r;
    } else if (data[len] == '[') {
        /* empty '[' WSP 'null' WSP ']' */
        for (r = len + 1; isspace(data[r]); ++r);
        if (strncmp(&data[r], "null", 4)) {
            goto inval;
        }
        for (r += 4; isspace(data[r]); ++r);
        if (data[r] != ']') {
            goto inval;
        }
        leaf->value_str = lydict_insert(ctx, "", 0);
        len = r + 1;
    } else {
inval:
        /* error */
        LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_LYD, leaf, "JSON data (unexpected value)");
        return 0;
    }

    /* the value is here converted to a JSON format if needed in case of LY_TYPE_IDENT and LY_TYPE_INST or to a
     * canonical form of the value */
    if (!lyp_parse_value(&((struct lys_node_leaf *)leaf->schema)->type, &leaf->value_str, NULL, leaf, NULL, NULL, 1, 0)) {
        return 0;
    }

#ifdef LY_ENABLED_CACHE
    /* calculate the hash and insert it into parent */
    lyd_hash((struct lyd_node *)leaf);
    lyd_insert_hash((struct lyd_node *)leaf);
#endif

    if (leaf->schema->nodetype == LYS_LEAFLIST) {
        /* repeat until end-array */
        len += skip_ws(&data[len]);
        if (data[len] == ',') {
            /* various validation checks */
            if (lyv_data_context((struct lyd_node*)leaf, options | LYD_OPT_TRUSTED, unres) ||
                    lyv_data_content((struct lyd_node*)leaf, options, unres) ||
                    lyv_multicases((struct lyd_node*)leaf, NULL, first_sibling, 0, NULL)) {
                return 0;
            }

            /* another instance of the leaf-list */
            new = calloc(1, sizeof(struct lyd_node_leaf_list));
            LY_CHECK_ERR_RETURN(!new, LOGMEM(ctx), 0);

            new->parent = leaf->parent;
            new->prev = (struct lyd_node *)leaf;
            leaf->next = (struct lyd_node *)new;

            /* copy the validity and when flags */
            new->validity = leaf->validity;
            new->when_status = leaf->when_status;

            /* fix the "last" pointer */
            (*first_sibling)->prev = (struct lyd_node *)new;

            new->schema = leaf->schema;

            /* repeat value parsing */
            leaf = new;
            len++;
            goto repeat;
        } else if (data[len] == ']') {
            len++;
            len += skip_ws(&data[len]);
        } else {
            /* something unexpected */
            LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_LYD, leaf, "JSON data (expecting value-separator or end-array)");
            return 0;
        }
    }

    len += skip_ws(&data[len]);
    return len;
}

static unsigned int
json_parse_attr(struct lys_module *parent_module, struct lyd_attr **attr, const char *data, int options)
{
    struct ly_ctx *ctx = parent_module->ctx;
    unsigned int len = 0, r;
    char *str = NULL, *name, *prefix = NULL, *value;
    struct lys_module *module = parent_module;
    struct lyd_attr *attr_new, *attr_last = NULL;
    int ret;

    *attr = NULL;

    if (data[len] != '{') {
        if (!strncmp(&data[len], "null", 4)) {
            len += 4;
            len += skip_ws(&data[len]);
            return len;
        }
        LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_NONE, NULL, "JSON data (missing begin-object)");
        goto error;
    }

repeat:
    prefix = NULL;
    len++;
    len += skip_ws(&data[len]);

    if (data[len] != '"') {
        LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_NONE, NULL, "JSON data (missing quotation-mark at the beginning of string)");
        return 0;
    }
    len++;
    str = lyjson_parse_text(ctx, &data[len], &r);
    if (!r) {
        goto error;
    } else if (data[len + r] != '"') {
        LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_NONE, NULL, "JSON data (missing quotation-mark at the end of string)");
        goto error;
    }
    if ((name = strchr(str, ':'))) {
        *name = '\0';
        name++;
        prefix = str;
        module = (struct lys_module *)ly_ctx_get_module(parent_module->ctx, prefix, NULL, 0);
        if (!module) {
            LOGVAL(ctx, LYE_INELEM, LY_VLOG_NONE, NULL, name);
            goto error;
        }
    } else {
        name = str;
    }

    /* prepare data for parsing node content */
    len += r + 1;
    len += skip_ws(&data[len]);
    if (data[len] != ':') {
        LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_NONE, NULL, "JSON data (missing name-separator)");
        goto error;
    }
    len++;
    len += skip_ws(&data[len]);

    if (data[len] != '"') {
        LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_NONE, NULL, "JSON data (missing quotation-mark at the beginning of string)");
        goto error;
    }
    len++;
    value = lyjson_parse_text(ctx, &data[len], &r);
    if (!r) {
        goto error;
    } else if (data[len + r] != '"') {
        LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_NONE, NULL, "JSON data (missing quotation-mark at the end of string)");
        free(value);
        goto error;
    }
    len += r + 1;
    len += skip_ws(&data[len]);

    ret = lyp_fill_attr(parent_module->ctx, NULL, NULL, prefix, name, value, NULL, &attr_new);
    if (ret == -1) {
        free(value);
        goto error;
    } else if (ret == 1) {
        if (options & LYD_OPT_STRICT) {
            LOGVAL(ctx, LYE_INMETA, LY_VLOG_NONE, NULL, prefix, name, value);
            free(value);
            goto error;
        }

        LOGWRN(ctx, "Unknown \"%s:%s\" metadata with value \"%s\", ignoring.",
               (prefix ? prefix : "<none>"), name, value);
        free(value);
        goto next;
    }
    free(value);

    if (!attr_last) {
        *attr = attr_last = attr_new;
    } else {
        attr_last->next = attr_new;
        attr_last = attr_new;
    }

next:
    free(str);
    str = NULL;

    if (data[len] == ',') {
        goto repeat;
    } else if (data[len] != '}') {
        LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_NONE, NULL, "JSON data (missing end-object)");
        goto error;
    }
    len++;
    len += skip_ws(&data[len]);

    return len;

error:
    free(str);
    if (*attr) {
        lyd_free_attr(module->ctx, NULL, *attr, 1);
        *attr = NULL;
    }
    return 0;
}

struct attr_cont {
    struct attr_cont *next;
    struct lyd_attr *attr;
    struct lys_node *schema;
    unsigned int index;    /** non-zero only in case of leaf-list */
};

static int
store_attrs(struct ly_ctx *ctx, struct attr_cont *attrs, struct lyd_node *first, int options)
{
    struct lyd_node *diter;
    struct attr_cont *iter;
    struct lyd_attr *aiter;
    unsigned int flag_leaflist = 0;

    while (attrs) {
        iter = attrs;
        attrs = attrs->next;

        if (iter->index) {
            flag_leaflist = 1;
        }

        LY_TREE_FOR(first, diter) {
            if (iter->schema != diter->schema) {
                continue;
            }

            if (flag_leaflist && flag_leaflist != iter->index) {
                flag_leaflist++;
                continue;
            }

            /* we have match */
            if (diter->attr) {
                LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_LYD, diter,
                       "attribute (multiple attribute definitions belong to a single element)");
                free(iter);
                goto error;
            }

            diter->attr = iter->attr;
            for (aiter = iter->attr; aiter; aiter = aiter->next) {
                aiter->parent = diter;
            }

            break;
        }

        if (!diter) {
            LOGVAL(ctx, LYE_XML_MISS, LY_VLOG_NONE, NULL, "element for the specified attribute", iter->attr->name);
            lyd_free_attr(iter->schema->module->ctx, NULL, iter->attr, 1);
            free(iter);
            goto error;
        }
        free(iter);

        /* check edit-config attribute correctness */
        if ((options & LYD_OPT_EDIT) && lyp_check_edit_attr(ctx, diter->attr, diter, NULL)) {
            goto error;
        }
    }

    return 0;

error:

    while (attrs) {
        iter = attrs;
        attrs = attrs->next;

        lyd_free_attr(ctx, NULL, iter->attr, 1);
        free(iter);
    }

    return -1;
}

/**
 * @brief Skip subtree (find its end in the input data) of the current JSON item.
 * @param[in] ctx libyang context for logging
 * @param[in] parent parent node for logging
 * @param[in] data input data (pointing to the beginning, @p len is used to go to the current position).
 * @param[in, out] len Current position in the @p data, will be updated to the end of the element's subtree in the @p data
 * @retun 0 on success
 * @return -1 on error.
 */
static int
json_skip_unknown(struct ly_ctx *ctx, struct lyd_node *parent, const char *data, unsigned int *len)
{
    int qstr = 0;
    int objects = 0;
    int arrays = 0;

    while (data[*len]) {
        switch (data[*len]) {
        case '\"':
            if (qstr) {
                if (data[(*len) - 1] != '\\')  {
                    qstr = 0;
                }
            } else if (data[(*len) - 1] != '\\') {
                qstr = 1;
            } else {
                LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_LYD, parent, "JSON data (missing quotation mark for a string data) ");
                return -1;
            }
            break;
        case '[':
            if (!qstr) {
                arrays++;
            }
            break;
        case '{':
            if (!qstr) {
                objects++;
            }
            break;
        case ']':
            if (!qstr) {
                arrays--;
            }
            break;
        case '}':
            if (!qstr) {
                objects--;
            }
            break;
        case ',':
            if (!qstr && !objects && !arrays) {
                /* do not eat the comma character */
                return 0;
            }
        }

        if (objects < 0) {
            if (arrays) {
                LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_LYD, parent, "JSON data (missing end-array)");
                return -1;
            }
            return 0;
        }
        if (arrays < 0) {
            if (objects) {
                LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_LYD, parent, "JSON data (missing end-object)");
                return -1;
            }
            return 0;
        }
        (*len)++;
    }

    return 0;
}

static unsigned int
json_parse_data(struct ly_ctx *ctx, const char *data, const struct lys_node *schema_parent, struct lyd_node **parent,
                struct lyd_node *first_sibling, struct lyd_node *prev, struct attr_cont **attrs, int options,
                struct unres_data *unres, struct lyd_node **act_notif, const char *yang_data_name)
{
    unsigned int len = 0;
    unsigned int r;
    unsigned int flag_leaflist = 0;
    int i;
    uint8_t pos;
    char *name, *prefix = NULL, *str = NULL;
    const struct lys_module *module = NULL;
    struct lys_node *schema = NULL;
    const struct lys_node *sparent = NULL;
    struct lyd_node *result = NULL, *new, *list, *diter = NULL;
    struct lyd_attr *attr;
    struct attr_cont *attrs_aux;

    /* each YANG data node representation starts with string (node identifier) */
    if (data[len] != '"') {
        LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_LYD, (*parent),
               "JSON data (missing quotation-mark at the beginning of string)");
        return 0;
    }
    len++;

    str = lyjson_parse_text(ctx, &data[len], &r);
    if (!str) {
        goto error;
    }

    if (!r) {
        goto error;
    } else if (data[len + r] != '"') {
        LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_LYD, (*parent),
               "JSON data (missing quotation-mark at the end of string)");
        goto error;
    }
    if ((name = strchr(str, ':'))) {
        *name = '\0';
        name++;
        prefix = str;
        if (prefix[0] == '@') {
            prefix++;
        }
    } else {
        name = str;
        if (name[0] == '@') {
            name++;
        }
    }

    /* prepare data for parsing node content */
    len += r + 1;
    len += skip_ws(&data[len]);
    if (data[len] != ':') {
        LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_LYD, (*parent), "JSON data (missing name-separator)");
        goto error;
    }
    len++;
    len += skip_ws(&data[len]);

    if (str[0] == '@' && !str[1]) {
        /* process attribute of the parent object (container or list) */
        if (!(*parent)) {
            LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_NONE, NULL, "attribute with no corresponding element to belongs to");
            goto error;
        }

        r = json_parse_attr((*parent)->schema->module, &attr, &data[len], options);
        if (!r) {
            LOGPATH(ctx, LY_VLOG_LYD, *parent);
            goto error;
        }
        len += r;

        if ((*parent)->attr) {
            lyd_free_attr(ctx, NULL, attr, 1);
        } else {
            (*parent)->attr = attr;
            for (; attr; attr = attr->next) {
                attr->parent = *parent;
            }
        }

        /* check edit-config attribute correctness */
        if ((options & LYD_OPT_EDIT) && lyp_check_edit_attr(ctx, (*parent)->attr, *parent, NULL)) {
            goto error;
        }

        free(str);
        return len;
    }

    /* find schema node */
    if (!(*parent)) {
        /* starting in root */
        /* get the proper schema */
        module = ly_ctx_get_module(ctx, prefix, NULL, 0);
        if (ctx->data_clb) {
            if (!module) {
                module = ctx->data_clb(ctx, prefix, NULL, 0, ctx->data_clb_data);
            } else if (!module->implemented) {
                module = ctx->data_clb(ctx, module->name, module->ns, LY_MODCLB_NOT_IMPLEMENTED, ctx->data_clb_data);
            }
        }
        if (module && module->implemented) {
            if (yang_data_name) {
                sparent = lyp_get_yang_data_template(module, yang_data_name, strlen(yang_data_name));
                schema = NULL;
                if (sparent) {
                    /* get the proper schema node */
                    while ((schema = (struct lys_node *) lys_getnext(schema, sparent, module, 0))) {
                        if (!strcmp(schema->name, name)) {
                            break;
                        }
                    }
                }
            } else {
                /* get the proper schema node */
                while ((schema = (struct lys_node *) lys_getnext(schema, NULL, module, 0))) {
                    if (!strcmp(schema->name, name)) {
                        break;
                    }
                }
            }
        }
    } else {
        if (prefix) {
            /* get the proper module to give the chance to load/implement it */
            module = ly_ctx_get_module(ctx, prefix, NULL, 1);
            if (ctx->data_clb) {
                if (!module) {
                    ctx->data_clb(ctx, prefix, NULL, 0, ctx->data_clb_data);
                } else if (!module->implemented) {
                    ctx->data_clb(ctx, module->name, module->ns, LY_MODCLB_NOT_IMPLEMENTED, ctx->data_clb_data);
                }
            }
        }

        /* go through RPC's input/output following the options' data type */
        if ((*parent)->schema->nodetype == LYS_RPC || (*parent)->schema->nodetype == LYS_ACTION) {
            while ((schema = (struct lys_node *)lys_getnext(schema, (*parent)->schema, NULL, LYS_GETNEXT_WITHINOUT))) {
                if ((options & LYD_OPT_RPC) && (schema->nodetype == LYS_INPUT)) {
                    break;
                } else if ((options & LYD_OPT_RPCREPLY) && (schema->nodetype == LYS_OUTPUT)) {
                    break;
                }
            }
            schema_parent = schema;
            schema = NULL;
        }

        if (schema_parent) {
            while ((schema = (struct lys_node *)lys_getnext(schema, schema_parent, NULL, 0))) {
                if (!strcmp(schema->name, name)
                        && ((prefix && !strcmp(lys_node_module(schema)->name, prefix))
                        || (!prefix && (lys_node_module(schema) == lys_node_module(schema_parent))))) {
                    break;
                }
            }
        } else {
            while ((schema = (struct lys_node *)lys_getnext(schema, (*parent)->schema, NULL, 0))) {
                if (!strcmp(schema->name, name)
                        && ((prefix && !strcmp(lys_node_module(schema)->name, prefix))
                        || (!prefix && (lys_node_module(schema) == lyd_node_module(*parent))))) {
                    break;
                }
            }
        }
    }

    module = lys_node_module(schema);
    if (!module || !module->implemented || module->disabled) {
        if (options & LYD_OPT_STRICT) {
            LOGVAL(ctx, LYE_INELEM, (*parent ? LY_VLOG_LYD : LY_VLOG_NONE), (*parent), name);
            goto error;
        } else {
            if (json_skip_unknown(ctx, *parent, data, &len)) {
                goto error;
            }
            free(str);
            return len;
        }
    }

    if (str[0] == '@') {
        /* attribute for some sibling node */
        if (data[len] == '[') {
            flag_leaflist = 1;
            len++;
            len += skip_ws(&data[len]);
        }

attr_repeat:
        r = json_parse_attr((struct lys_module *)module, &attr, &data[len], options);
        if (!r) {
            LOGPATH(ctx, LY_VLOG_LYD, (*parent));
            goto error;
        }
        len += r;

        if (attr) {
            attrs_aux = malloc(sizeof *attrs_aux);
            LY_CHECK_ERR_GOTO(!attrs_aux, LOGMEM(ctx), error);
            attrs_aux->attr = attr;
            attrs_aux->index = flag_leaflist;
            attrs_aux->schema = schema;
            attrs_aux->next = *attrs;
            *attrs = attrs_aux;
        }

        if (flag_leaflist) {
            if (data[len] == ',') {
                len++;
                len += skip_ws(&data[len]);
                flag_leaflist++;
                goto attr_repeat;
            } else if (data[len] != ']') {
                LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_LYD, (*parent), "JSON data (missing end-array)");
                goto error;
            }
            len++;
            len += skip_ws(&data[len]);
        }

        free(str);
        return len;
    }

    switch (schema->nodetype) {
    case LYS_CONTAINER:
    case LYS_LIST:
    case LYS_NOTIF:
    case LYS_RPC:
    case LYS_ACTION:
        result = calloc(1, sizeof *result);
        break;
    case LYS_LEAF:
    case LYS_LEAFLIST:
        result = calloc(1, sizeof(struct lyd_node_leaf_list));
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        result = calloc(1, sizeof(struct lyd_node_anydata));
        break;
    default:
        LOGINT(ctx);
        goto error;
    }
    LY_CHECK_ERR_GOTO(!result, LOGMEM(ctx), error);

    result->prev = result;
    result->schema = schema;
    result->parent = *parent;
    diter = NULL;
    if (schema->nodetype == LYS_LEAF && lys_is_key((struct lys_node_leaf *)schema, &pos)) {
        /* it is key and we need to insert it into a correct place (we must have parent then, a key cannot be top-level) */
        assert(*parent);
        for (i = 0, diter = (*parent)->child;
                diter && i < pos && diter->schema->nodetype == LYS_LEAF && lys_is_key((struct lys_node_leaf *)diter->schema, NULL);
                i++, diter = diter->next);
        if (diter) {
            /* out of order insertion - insert list's key to the correct position, before the diter */
            if ((*parent)->child == diter) {
                (*parent)->child = result;
                /* update first_sibling */
                first_sibling = result;
            }
            if (diter->prev->next) {
                diter->prev->next = result;
            }
            result->prev = diter->prev;
            diter->prev = result;
            result->next = diter;
        }
    }
    if (!diter) {
        /* simplified (faster) insert as the last node */
        if (*parent && !(*parent)->child) {
            (*parent)->child = result;
        }
        if (prev) {
            result->prev = prev;
            prev->next = result;

            /* fix the "last" pointer */
            first_sibling->prev = result;
        } else {
            result->prev = result;
            first_sibling = result;
        }
    }
    result->validity = ly_new_node_validity(result->schema);
    if (resolve_applies_when(schema, 0, NULL)) {
        result->when_status = LYD_WHEN;
    }

    /* type specific processing */
    switch (schema->nodetype) {
    case LYS_LEAF:
    case LYS_LEAFLIST:
        /* type detection and assigning the value */
        r = json_get_value((struct lyd_node_leaf_list *)result, &first_sibling, &data[len], options, unres);
        if (!r) {
            goto error;
        }
        /* only for leaf-list */
        while (result->next && (result->next->schema == result->schema)) {
            result = result->next;
        }

        len += r;
        len += skip_ws(&data[len]);
        break;
    case LYS_ANYDATA:
    case LYS_ANYXML:
        r = json_get_anydata((struct lyd_node_anydata *)result, &data[len]);
        if (!r) {
            goto error;
        }

#ifdef LY_ENABLED_CACHE
        /* calculate the hash and insert it into parent */
        lyd_hash(result);
        lyd_insert_hash(result);
#endif

        len += r;
        len += skip_ws(&data[len]);
        break;
    case LYS_CONTAINER:
    case LYS_RPC:
    case LYS_ACTION:
    case LYS_NOTIF:
        if (schema->nodetype & (LYS_RPC | LYS_ACTION)) {
            if (!(options & LYD_OPT_RPC) || *act_notif) {
                LOGVAL(ctx, LYE_INELEM, LY_VLOG_LYD, result, schema->name);
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL, "Unexpected %s node \"%s\".",
                       (schema->nodetype == LYS_RPC ? "rpc" : "action"), schema->name);
                goto error;
            }
            *act_notif = result;
        } else if (schema->nodetype == LYS_NOTIF) {
            if (!(options & LYD_OPT_NOTIF) || *act_notif) {
                LOGVAL(ctx, LYE_INELEM, LY_VLOG_LYD, result, schema->name);
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL, "Unexpected notification node \"%s\".", schema->name);
                goto error;
            }
            *act_notif = result;
        }

#ifdef LY_ENABLED_CACHE
        /* calculate the hash and insert it into parent */
        lyd_hash(result);
        lyd_insert_hash(result);
#endif

        if (data[len] != '{') {
            LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_LYD, result, "JSON data (missing begin-object)");
            goto error;
        }
        len++;
        len += skip_ws(&data[len]);

        if (data[len] != '}') {
            /* non-empty container */
            len--;
            diter = NULL;
            attrs_aux = NULL;
            do {
                len++;
                len += skip_ws(&data[len]);

                r = json_parse_data(ctx, &data[len], NULL, &result, result->child, diter, &attrs_aux, options, unres, act_notif, yang_data_name);
                if (!r) {
                    goto error;
                }
                len += r;

                if (result->child) {
                    diter = result->child->prev;
                }
            } while(data[len] == ',');

            /* store attributes */
            if (store_attrs(ctx, attrs_aux, result->child, options)) {
                goto error;
            }
        }

        if (data[len] != '}') {
            LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_LYD, result, "JSON data (missing end-object)");
            goto error;
        }
        len++;
        len += skip_ws(&data[len]);

        /* if we have empty non-presence container, mark it as default */
        if (schema->nodetype == LYS_CONTAINER && !result->child &&
                !result->attr && !((struct lys_node_container *)schema)->presence) {
            result->dflt = 1;
        }
        break;
    case LYS_LIST:
        if (data[len] != '[') {
            LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_LYD, result, "JSON data (missing begin-array)");
            goto error;
        }

        list = result;
        do {
            len++;
            len += skip_ws(&data[len]);

            if (data[len] != '{') {
                LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_LYD, result,
                       "JSON data (missing list instance's begin-object)");
                goto error;
            }
            diter = NULL;
            attrs_aux = NULL;
            do {
                len++;
                len += skip_ws(&data[len]);

                r = json_parse_data(ctx, &data[len], NULL, &list, list->child, diter, &attrs_aux, options, unres, act_notif, yang_data_name);
                if (!r) {
                    goto error;
                }
                len += r;

                if (list->child) {
                    diter = list->child->prev;
                }
            } while (data[len] == ',');

#ifdef LY_ENABLED_CACHE
            /* calculate the hash and insert it into parent */
            if (!((struct lys_node_list *)list->schema)->keys_size) {
                lyd_hash(list);
                lyd_insert_hash(list);
            }
#endif

            /* store attributes */
            if (store_attrs(ctx, attrs_aux, list->child, options)) {
                goto error;
            }

            if (data[len] != '}') {
                /* expecting end-object */
                LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_LYD, result,
                       "JSON data (missing list instance's end-object)");
                goto error;
            }
            len++;
            len += skip_ws(&data[len]);

            if (data[len] == ',') {
                /* various validation checks */
                if (lyv_data_context(list, options | LYD_OPT_TRUSTED, unres) ||
                        lyv_data_content(list, options, unres) ||
                        lyv_multicases(list, NULL, prev ? &first_sibling : NULL, 0, NULL)) {
                    goto error;
                }

                /* another instance of the list */
                new = calloc(1, sizeof *new);
                LY_CHECK_ERR_GOTO(!new, LOGMEM(ctx), error);
                new->parent = list->parent;
                new->prev = list;
                list->next = new;

                /* copy the validity and when flags */
                new->validity = list->validity;
                new->when_status = list->when_status;

                /* fix the "last" pointer */
                first_sibling->prev = new;

                new->schema = list->schema;
                list = new;
            }
        } while (data[len] == ',');
        result = list;

        if (data[len] != ']') {
            LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_LYD, result, "JSON data (missing end-array)");
            goto error;
        }
        len++;
        len += skip_ws(&data[len]);
        break;
    default:
        LOGINT(ctx);
        goto error;
    }

    /* various validation checks (LYD_OPT_TRUSTED is used just so that the order of elements is not checked) */
    if (lyv_data_context(result, options | LYD_OPT_TRUSTED, unres) ||
            lyv_data_content(result, options, unres) ||
            lyv_multicases(result, NULL, prev ? &first_sibling : NULL, 0, NULL)) {
        goto error;
    }

    /* validation successful */
    if (result->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
        /* postpone checking of unique when there will be all list/leaflist instances */
        result->validity |= LYD_VAL_DUP;
    }

    if (!(*parent)) {
        *parent = result;
    }

    free(str);
    return len;

error:
    /* cleanup */
    for (i = unres->count - 1; i >= 0; i--) {
        /* remove unres items connected with the node being removed */
        if (unres->node[i] == result) {
            unres_data_del(unres, i);
        }
    }
    while (*attrs) {
        attrs_aux = *attrs;
        *attrs = (*attrs)->next;

        lyd_free_attr(ctx, NULL, attrs_aux->attr, 1);
        free(attrs_aux);
    }

    lyd_free(result);
    free(str);

    return 0;
}

struct lyd_node *
lyd_parse_json(struct ly_ctx *ctx, const char *data, int options, const struct lyd_node *rpc_act,
               const struct lyd_node *data_tree, const char *yang_data_name)
{
    struct lyd_node *result = NULL, *next, *iter, *reply_parent = NULL, *reply_top = NULL, *act_notif = NULL;
    struct unres_data *unres = NULL;
    unsigned int len = 0, r;
    int act_cont = 0;
    struct attr_cont *attrs = NULL;

    if (!ctx || !data) {
        LOGARG;
        return NULL;
    }

    /* skip leading whitespaces */
    len += skip_ws(&data[len]);

    /* expect top-level { */
    if (data[len] != '{') {
        LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_NONE, NULL, "JSON data (missing top level begin-object)");
        return NULL;
    }

    /* check for empty object */
    r = len + 1;
    r += skip_ws(&data[r]);
    if (data[r] == '}') {
        if (options & LYD_OPT_DATA_ADD_YANGLIB) {
            result = ly_ctx_info(ctx);
        }
        lyd_validate(&result, options, ctx);
        return result;
    }

    unres = calloc(1, sizeof *unres);
    LY_CHECK_ERR_RETURN(!unres, LOGMEM(ctx), NULL);

    /* create RPC/action reply part that is not in the parsed data */
    if (rpc_act) {
        assert(options & LYD_OPT_RPCREPLY);
        if (rpc_act->schema->nodetype == LYS_RPC) {
            /* RPC request */
            reply_top = reply_parent = _lyd_new(NULL, rpc_act->schema, 0);
        } else {
            /* action request */
            reply_top = lyd_dup(rpc_act, 1);
            LY_TREE_DFS_BEGIN(reply_top, iter, reply_parent) {
                if (reply_parent->schema->nodetype == LYS_ACTION) {
                    break;
                }
                LY_TREE_DFS_END(reply_top, iter, reply_parent);
            }
            if (!reply_parent) {
                LOGERR(ctx, LY_EINVAL, "%s: invalid variable parameter (const struct lyd_node *rpc_act).", __func__);
                goto error;
            }
            lyd_free_withsiblings(reply_parent->child);
        }
    }

    iter = NULL;
    next = reply_parent;
    do {
        len++;
        len += skip_ws(&data[len]);

        if (!act_cont) {
            if (!strncmp(&data[len], "\"yang:action\"", 13)) {
                len += 13;
                len += skip_ws(&data[len]);
                if (data[len] != ':') {
                    LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_NONE, NULL, "JSON data (missing top-level begin-object)");
                    goto error;
                }
                ++len;
                len += skip_ws(&data[len]);
                if (data[len] != '{') {
                    LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_NONE, NULL, "JSON data (missing top level yang:action object)");
                    goto error;
                }
                ++len;
                len += skip_ws(&data[len]);

                act_cont = 1;
            } else {
                act_cont = -1;
            }
        }

        r = json_parse_data(ctx, &data[len], NULL, &next, result, iter, &attrs, options, unres, &act_notif, yang_data_name);
        if (!r) {
            goto error;
        }
        len += r;

        if (!result) {
            if (reply_parent) {
                result = next->child;
                iter = next->child ? next->child->prev : NULL;
            } else {
                for (iter = next; iter && iter->prev->next; iter = iter->prev);
                result = iter;
                if (iter && (options & LYD_OPT_DATA_ADD_YANGLIB) && iter->schema->module == ctx->models.list[ctx->internal_module_count - 1]) {
                    /* ietf-yang-library data present, so ignore the option to add them */
                    options &= ~LYD_OPT_DATA_ADD_YANGLIB;
                }
                iter = next;
            }
        } else {
            iter = result->prev;
        }
        if (!reply_parent) {
            next = NULL;
        }
    } while (data[len] == ',');

    if (data[len] != '}') {
        /* expecting end-object */
        LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_NONE, NULL, "JSON data (missing top-level end-object)");
        goto error;
    }
    len++;
    len += skip_ws(&data[len]);

    if (act_cont == 1) {
        if (data[len] != '}') {
            LOGVAL(ctx, LYE_XML_INVAL, LY_VLOG_NONE, NULL, "JSON data (missing top-level end-object)");
            goto error;
        }
        len++;
        len += skip_ws(&data[len]);
    }

    /* store attributes */
    if (store_attrs(ctx, attrs, result, options)) {
        goto error;
    }

    if (reply_top) {
        result = reply_top;
    }

    if (!result && (options & LYD_OPT_STRICT)) {
        LOGERR(ctx, LY_EVALID, "Model for the data to be linked with not found.");
        goto error;
    }

    /* order the elements by hand as it is not required of the JSON input */
    if ((options & (LYD_OPT_RPC | LYD_OPT_RPCREPLY))) {
        if (lyd_schema_sort(result, 1)) {
            goto error;
        }
    }

    if ((options & LYD_OPT_RPCREPLY) && (rpc_act->schema->nodetype != LYS_RPC)) {
        /* action reply */
        act_notif = reply_parent;
    } else if ((options & (LYD_OPT_RPC | LYD_OPT_NOTIF)) && !act_notif) {
        LOGVAL(ctx, LYE_MISSELEM, LY_VLOG_LYD, result, (options & LYD_OPT_RPC ? "action" : "notification"), result->schema->name);
        goto error;
    }

    /* add missing ietf-yang-library if requested */
    if (options & LYD_OPT_DATA_ADD_YANGLIB) {
        if (lyd_merge(result, ly_ctx_info(ctx), LYD_OPT_DESTRUCT | LYD_OPT_EXPLICIT)) {
            LOGERR(ctx, LY_EINT, "Adding ietf-yang-library data failed.");
            goto error;
        }
    }

    /* check for uniquness of top-level lists/leaflists because
     * only the inner instances were tested in lyv_data_content() */
    LY_TREE_FOR(result, iter) {
        if (!(iter->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) || !(iter->validity & LYD_VAL_DUP)) {
            continue;
        }

        if (lyv_data_dup(iter, result)) {
            goto error;
        }
    }

    /* add/validate default values, unres */
    if (lyd_defaults_add_unres(&result, options, ctx, NULL, 0, data_tree, act_notif, unres, 1)) {
        goto error;
    }

    /* check for missing top level mandatory nodes */
    if (!(options & (LYD_OPT_TRUSTED | LYD_OPT_NOTIF_FILTER))
            && lyd_check_mandatory_tree((act_notif ? act_notif : result), ctx, NULL, 0, options)) {
        goto error;
    }

    free(unres->node);
    free(unres->type);
    free(unres);

    return result;

error:
    lyd_free_withsiblings(result);
    if (reply_top && result != reply_top) {
        lyd_free_withsiblings(reply_top);
    }
    free(unres->node);
    free(unres->type);
    free(unres);

    return NULL;
}
