/**
 * @file resolve.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang resolve functions
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

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "libyang.h"
#include "resolve.h"
#include "common.h"
#include "xpath.h"
#include "parser.h"
#include "parser_yang.h"
#include "xml_internal.h"
#include "hash_table.h"
#include "tree_internal.h"
#include "extensions.h"
#include "validation.h"

/* internal parsed predicate structure */
struct parsed_pred {
    const struct lys_node *schema;
    int len;
    struct {
        const char *mod_name;
        int mod_name_len;
        const char *name;
        int nam_len;
        const char *value;
        int val_len;
    } *pred;
};

int
parse_range_dec64(const char **str_num, uint8_t dig, int64_t *num)
{
    const char *ptr;
    int minus = 0;
    int64_t ret = 0, prev_ret;
    int8_t str_exp, str_dig = -1, trailing_zeros = 0;

    ptr = *str_num;

    if (ptr[0] == '-') {
        minus = 1;
        ++ptr;
    } else if (ptr[0] == '+') {
        ++ptr;
    }

    if (!isdigit(ptr[0])) {
        /* there must be at least one */
        return 1;
    }

    for (str_exp = 0; isdigit(ptr[0]) || ((ptr[0] == '.') && (str_dig < 0)); ++ptr) {
        if (str_exp > 18) {
            return 1;
        }

        if (ptr[0] == '.') {
            if (ptr[1] == '.') {
                /* it's the next interval */
                break;
            }
            ++str_dig;
        } else {
            prev_ret = ret;
            if (minus) {
                ret = ret * 10 - (ptr[0] - '0');
                if (ret > prev_ret) {
                    return 1;
                }
            } else {
                ret = ret * 10 + (ptr[0] - '0');
                if (ret < prev_ret) {
                    return 1;
                }
            }
            if (str_dig > -1) {
                ++str_dig;
                if (ptr[0] == '0') {
                    /* possibly trailing zero */
                    trailing_zeros++;
                } else {
                    trailing_zeros = 0;
                }
            }
            ++str_exp;
        }
    }
    if (str_dig == 0) {
        /* no digits after '.' */
        return 1;
    } else if (str_dig == -1) {
        /* there are 0 numbers after the floating point */
        str_dig = 0;
    }
    /* remove trailing zeros */
    if (trailing_zeros) {
        str_dig -= trailing_zeros;
        str_exp -= trailing_zeros;
        ret = ret / dec_pow(trailing_zeros);
    }

    /* it's parsed, now adjust the number based on fraction-digits, if needed */
    if (str_dig < dig) {
        if ((str_exp - 1) + (dig - str_dig) > 18) {
            return 1;
        }
        prev_ret = ret;
        ret *= dec_pow(dig - str_dig);
        if ((minus && (ret > prev_ret)) || (!minus && (ret < prev_ret))) {
            return 1;
        }

    }
    if (str_dig > dig) {
        return 1;
    }

    *str_num = ptr;
    *num = ret;

    return 0;
}

/**
 * @brief Parse an identifier.
 *
 * ;; An identifier MUST NOT start with (('X'|'x') ('M'|'m') ('L'|'l'))
 * identifier          = (ALPHA / "_")
 *                       *(ALPHA / DIGIT / "_" / "-" / ".")
 *
 * @param[in] id Identifier to use.
 *
 * @return Number of characters successfully parsed.
 */
unsigned int
parse_identifier(const char *id)
{
    unsigned int parsed = 0;

    assert(id);

    if (!isalpha(id[0]) && (id[0] != '_')) {
        return -parsed;
    }

    ++parsed;
    ++id;

    while (isalnum(id[0]) || (id[0] == '_') || (id[0] == '-') || (id[0] == '.')) {
        ++parsed;
        ++id;
    }

    return parsed;
}

/**
 * @brief Parse a node-identifier.
 *
 * node-identifier     = [module-name ":"] identifier
 *
 * @param[in] id Identifier to use.
 * @param[out] mod_name Points to the module name, NULL if there is not any.
 * @param[out] mod_name_len Length of the module name, 0 if there is not any.
 * @param[out] name Points to the node name.
 * @param[out] nam_len Length of the node name.
 * @param[out] all_desc Whether the path starts with '/', only supported in extended paths.
 * @param[in] extended Whether to accept an extended path (support for [prefix:]*, /[prefix:]*, /[prefix:]., prefix:#identifier).
 *
 * @return Number of characters successfully parsed,
 *         positive on success, negative on failure.
 */
static int
parse_node_identifier(const char *id, const char **mod_name, int *mod_name_len, const char **name, int *nam_len,
                      int *all_desc, int extended)
{
    int parsed = 0, ret, all_desc_local = 0;

    assert(id);
    assert((mod_name && mod_name_len) || (!mod_name && !mod_name_len));
    assert((name && nam_len) || (!name && !nam_len));

    if (mod_name) {
        *mod_name = NULL;
        *mod_name_len = 0;
    }
    if (name) {
        *name = NULL;
        *nam_len = 0;
    }

    if (extended) {
        /* try to parse only the extended expressions */
        if (id[parsed] == '/') {
            if (all_desc) {
                *all_desc = 1;
            }
            all_desc_local = 1;
        } else {
            if (all_desc) {
                *all_desc = 0;
            }
        }

        /* is there a prefix? */
        ret = parse_identifier(id + all_desc_local);
        if (ret > 0) {
            if (id[all_desc_local + ret] != ':') {
                /* this is not a prefix, so not an extended id */
                goto standard_id;
            }

            if (mod_name) {
                *mod_name = id + all_desc_local;
                *mod_name_len = ret;
            }

            /* "/" and ":" */
            ret += all_desc_local + 1;
        } else {
            ret = all_desc_local;
        }

        /* parse either "*" or "." */
        if (*(id + ret) == '*') {
            if (name) {
                *name = id + ret;
                *nam_len = 1;
            }
            ++ret;

            return ret;
        } else if (*(id + ret) == '.') {
            if (!all_desc_local) {
                /* /. is redundant expression, we do not accept it */
                return -ret;
            }

            if (name) {
                *name = id + ret;
                *nam_len = 1;
            }
            ++ret;

            return ret;
        } else if (*(id + ret) == '#') {
            if (all_desc_local || !ret) {
                /* no prefix */
                return 0;
            }
            parsed = ret + 1;
            if ((ret = parse_identifier(id + parsed)) < 1) {
                return -parsed + ret;
            }
            *name = id + parsed - 1;
            *nam_len = ret + 1;
            return parsed + ret;
        }
        /* else a standard id, parse it all again */
    }

standard_id:
    if ((ret = parse_identifier(id)) < 1) {
        return ret;
    }

    if (mod_name) {
        *mod_name = id;
        *mod_name_len = ret;
    }

    parsed += ret;
    id += ret;

    /* there is prefix */
    if (id[0] == ':') {
        ++parsed;
        ++id;

    /* there isn't */
    } else {
        if (name && mod_name) {
            *name = *mod_name;
        }
        if (mod_name) {
            *mod_name = NULL;
        }

        if (nam_len && mod_name_len) {
            *nam_len = *mod_name_len;
        }
        if (mod_name_len) {
            *mod_name_len = 0;
        }

        return parsed;
    }

    /* identifier (node name) */
    if ((ret = parse_identifier(id)) < 1) {
        return -parsed+ret;
    }

    if (name) {
        *name = id;
        *nam_len = ret;
    }

    return parsed+ret;
}

/**
 * @brief Parse a path-predicate (leafref).
 *
 * path-predicate      = "[" *WSP path-equality-expr *WSP "]"
 * path-equality-expr  = node-identifier *WSP "=" *WSP path-key-expr
 *
 * @param[in] id Identifier to use.
 * @param[out] prefix Points to the prefix, NULL if there is not any.
 * @param[out] pref_len Length of the prefix, 0 if there is not any.
 * @param[out] name Points to the node name.
 * @param[out] nam_len Length of the node name.
 * @param[out] path_key_expr Points to the path-key-expr.
 * @param[out] pke_len Length of the path-key-expr.
 * @param[out] has_predicate Flag to mark whether there is another predicate following.
 *
 * @return Number of characters successfully parsed,
 *         positive on success, negative on failure.
 */
static int
parse_path_predicate(const char *id, const char **prefix, int *pref_len, const char **name, int *nam_len,
                     const char **path_key_expr, int *pke_len, int *has_predicate)
{
    const char *ptr;
    int parsed = 0, ret;

    assert(id);
    if (prefix) {
        *prefix = NULL;
    }
    if (pref_len) {
        *pref_len = 0;
    }
    if (name) {
        *name = NULL;
    }
    if (nam_len) {
        *nam_len = 0;
    }
    if (path_key_expr) {
        *path_key_expr = NULL;
    }
    if (pke_len) {
        *pke_len = 0;
    }
    if (has_predicate) {
        *has_predicate = 0;
    }

    if (id[0] != '[') {
        return -parsed;
    }

    ++parsed;
    ++id;

    while (isspace(id[0])) {
        ++parsed;
        ++id;
    }

    if ((ret = parse_node_identifier(id, prefix, pref_len, name, nam_len, NULL, 0)) < 1) {
        return -parsed+ret;
    }

    parsed += ret;
    id += ret;

    while (isspace(id[0])) {
        ++parsed;
        ++id;
    }

    if (id[0] != '=') {
        return -parsed;
    }

    ++parsed;
    ++id;

    while (isspace(id[0])) {
        ++parsed;
        ++id;
    }

    if ((ptr = strchr(id, ']')) == NULL) {
        return -parsed;
    }

    --ptr;
    while (isspace(ptr[0])) {
        --ptr;
    }
    ++ptr;

    ret = ptr-id;
    if (path_key_expr) {
        *path_key_expr = id;
    }
    if (pke_len) {
        *pke_len = ret;
    }

    parsed += ret;
    id += ret;

    while (isspace(id[0])) {
        ++parsed;
        ++id;
    }

    assert(id[0] == ']');

    if (id[1] == '[') {
        *has_predicate = 1;
    }

    return parsed+1;
}

/**
 * @brief Parse a path-key-expr (leafref). First call parses "current()", all
 *        the ".." and the first node-identifier, other calls parse a single
 *        node-identifier each.
 *
 * path-key-expr       = current-function-invocation *WSP "/" *WSP
 *                       rel-path-keyexpr
 * rel-path-keyexpr    = 1*(".." *WSP "/" *WSP)
 *                       *(node-identifier *WSP "/" *WSP)
 *                       node-identifier
 *
 * @param[in] id Identifier to use.
 * @param[out] prefix Points to the prefix, NULL if there is not any.
 * @param[out] pref_len Length of the prefix, 0 if there is not any.
 * @param[out] name Points to the node name.
 * @param[out] nam_len Length of the node name.
 * @param[out] parent_times Number of ".." in the path. Must be 0 on the first call,
 *                          must not be changed between consecutive calls.
 * @return Number of characters successfully parsed,
 *         positive on success, negative on failure.
 */
static int
parse_path_key_expr(const char *id, const char **prefix, int *pref_len, const char **name, int *nam_len,
                    int *parent_times)
{
    int parsed = 0, ret, par_times = 0;

    assert(id);
    assert(parent_times);
    if (prefix) {
        *prefix = NULL;
    }
    if (pref_len) {
        *pref_len = 0;
    }
    if (name) {
        *name = NULL;
    }
    if (nam_len) {
        *nam_len = 0;
    }

    if (!*parent_times) {
        /* current-function-invocation *WSP "/" *WSP rel-path-keyexpr */
        if (strncmp(id, "current()", 9)) {
            return -parsed;
        }

        parsed += 9;
        id += 9;

        while (isspace(id[0])) {
            ++parsed;
            ++id;
        }

        if (id[0] != '/') {
            return -parsed;
        }

        ++parsed;
        ++id;

        while (isspace(id[0])) {
            ++parsed;
            ++id;
        }

        /* rel-path-keyexpr */
        if (strncmp(id, "..", 2)) {
            return -parsed;
        }
        ++par_times;

        parsed += 2;
        id += 2;

        while (isspace(id[0])) {
            ++parsed;
            ++id;
        }
    }

    /* 1*(".." *WSP "/" *WSP) *(node-identifier *WSP "/" *WSP) node-identifier
     *
     * first parent reference with whitespaces already parsed
     */
    if (id[0] != '/') {
        return -parsed;
    }

    ++parsed;
    ++id;

    while (isspace(id[0])) {
        ++parsed;
        ++id;
    }

    while (!strncmp(id, "..", 2) && !*parent_times) {
        ++par_times;

        parsed += 2;
        id += 2;

        while (isspace(id[0])) {
            ++parsed;
            ++id;
        }

        if (id[0] != '/') {
            return -parsed;
        }

        ++parsed;
        ++id;

        while (isspace(id[0])) {
            ++parsed;
            ++id;
        }
    }

    if (!*parent_times) {
        *parent_times = par_times;
    }

    /* all parent references must be parsed at this point */
    if ((ret = parse_node_identifier(id, prefix, pref_len, name, nam_len, NULL, 0)) < 1) {
        return -parsed + ret;
    }

    parsed += ret;
    id += ret;

    return parsed;
}

/**
 * @brief Parse path-arg (leafref).
 *
 * path-arg            = absolute-path / relative-path
 * absolute-path       = 1*("/" (node-identifier *path-predicate))
 * relative-path       = 1*(".." "/") descendant-path
 *
 * @param[in] mod Module of the context node to get correct prefix in case it is not explicitly specified
 * @param[in] id Identifier to use.
 * @param[out] prefix Points to the prefix, NULL if there is not any.
 * @param[out] pref_len Length of the prefix, 0 if there is not any.
 * @param[out] name Points to the node name.
 * @param[out] nam_len Length of the node name.
 * @param[out] parent_times Number of ".." in the path. Must be 0 on the first call,
 *                          must not be changed between consecutive calls. -1 if the
 *                          path is relative.
 * @param[out] has_predicate Flag to mark whether there is a predicate specified.
 *
 * @return Number of characters successfully parsed,
 *         positive on success, negative on failure.
 */
static int
parse_path_arg(const struct lys_module *mod, const char *id, const char **prefix, int *pref_len,
               const char **name, int *nam_len, int *parent_times, int *has_predicate)
{
    int parsed = 0, ret, par_times = 0;

    assert(id);
    assert(parent_times);
    if (prefix) {
        *prefix = NULL;
    }
    if (pref_len) {
        *pref_len = 0;
    }
    if (name) {
        *name = NULL;
    }
    if (nam_len) {
        *nam_len = 0;
    }
    if (has_predicate) {
        *has_predicate = 0;
    }

    if (!*parent_times && !strncmp(id, "..", 2)) {
        ++par_times;

        parsed += 2;
        id += 2;

        while (!strncmp(id, "/..", 3)) {
            ++par_times;

            parsed += 3;
            id += 3;
        }
    }

    if (!*parent_times) {
        if (par_times) {
            *parent_times = par_times;
        } else {
            *parent_times = -1;
        }
    }

    if (id[0] != '/') {
        return -parsed;
    }

    /* skip '/' */
    ++parsed;
    ++id;

    /* node-identifier ([prefix:]identifier) */
    if ((ret = parse_node_identifier(id, prefix, pref_len, name, nam_len, NULL, 0)) < 1) {
        return -parsed - ret;
    }
    if (prefix && !(*prefix)) {
        /* actually we always need prefix even it is not specified */
        *prefix = lys_main_module(mod)->name;
        *pref_len = strlen(*prefix);
    }

    parsed += ret;
    id += ret;

    /* there is no predicate */
    if ((id[0] == '/') || !id[0]) {
        return parsed;
    } else if (id[0] != '[') {
        return -parsed;
    }

    if (has_predicate) {
        *has_predicate = 1;
    }

    return parsed;
}

/**
 * @brief Parse instance-identifier in JSON data format. That means that prefixes
 *        are actually model names.
 *
 * instance-identifier = 1*("/" (node-identifier *predicate))
 *
 * @param[in] id Identifier to use.
 * @param[out] model Points to the model name.
 * @param[out] mod_len Length of the model name.
 * @param[out] name Points to the node name.
 * @param[out] nam_len Length of the node name.
 * @param[out] has_predicate Flag to mark whether there is a predicate specified.
 *
 * @return Number of characters successfully parsed,
 *         positive on success, negative on failure.
 */
static int
parse_instance_identifier(const char *id, const char **model, int *mod_len, const char **name, int *nam_len,
                          int *has_predicate)
{
    int parsed = 0, ret;

    assert(id && model && mod_len && name && nam_len);

    if (has_predicate) {
        *has_predicate = 0;
    }

    if (id[0] != '/') {
        return -parsed;
    }

    ++parsed;
    ++id;

    if ((ret = parse_identifier(id)) < 1) {
        return ret;
    }

    *name = id;
    *nam_len = ret;

    parsed += ret;
    id += ret;

    if (id[0] == ':') {
        /* we have prefix */
        *model = *name;
        *mod_len = *nam_len;

        ++parsed;
        ++id;

        if ((ret = parse_identifier(id)) < 1) {
            return ret;
        }

        *name = id;
        *nam_len = ret;

        parsed += ret;
        id += ret;
    }

    if (id[0] == '[' && has_predicate) {
        *has_predicate = 1;
    }

    return parsed;
}

/**
 * @brief Parse predicate (instance-identifier) in JSON data format. That means that prefixes
 *        (which are mandatory) are actually model names.
 *
 * predicate           = "[" *WSP (predicate-expr / pos) *WSP "]"
 * predicate-expr      = (node-identifier / ".") *WSP "=" *WSP
 *                       ((DQUOTE string DQUOTE) /
 *                        (SQUOTE string SQUOTE))
 * pos                 = non-negative-integer-value
 *
 * @param[in] id Identifier to use.
 * @param[out] model Points to the model name.
 * @param[out] mod_len Length of the model name.
 * @param[out] name Points to the node name. Can be identifier (from node-identifier), "." or pos.
 * @param[out] nam_len Length of the node name.
 * @param[out] value Value the node-identifier must have (string from the grammar),
 *                   NULL if there is not any.
 * @param[out] val_len Length of the value, 0 if there is not any.
 * @param[out] has_predicate Flag to mark whether there is a predicate specified.
 *
 * @return Number of characters successfully parsed,
 *         positive on success, negative on failure.
 */
static int
parse_predicate(const char *id, const char **model, int *mod_len, const char **name, int *nam_len,
                const char **value, int *val_len, int *has_predicate)
{
    const char *ptr;
    int parsed = 0, ret;
    char quote;

    assert(id);
    if (model) {
        assert(mod_len);
        *model = NULL;
        *mod_len = 0;
    }
    if (name) {
        assert(nam_len);
        *name = NULL;
        *nam_len = 0;
    }
    if (value) {
        assert(val_len);
        *value = NULL;
        *val_len = 0;
    }
    if (has_predicate) {
        *has_predicate = 0;
    }

    if (id[0] != '[') {
        return -parsed;
    }

    ++parsed;
    ++id;

    while (isspace(id[0])) {
        ++parsed;
        ++id;
    }

    /* pos */
    if (isdigit(id[0])) {
        if (name) {
            *name = id;
        }

        if (id[0] == '0') {
            return -parsed;
        }

        while (isdigit(id[0])) {
            ++parsed;
            ++id;
        }

        if (nam_len) {
            *nam_len = id-(*name);
        }

    /* "." or node-identifier */
    } else {
        if (id[0] == '.') {
            if (name) {
                *name = id;
            }
            if (nam_len) {
                *nam_len = 1;
            }

            ++parsed;
            ++id;

        } else {
            if ((ret = parse_node_identifier(id, model, mod_len, name, nam_len, NULL, 0)) < 1) {
                return -parsed + ret;
            }

            parsed += ret;
            id += ret;
        }

        while (isspace(id[0])) {
            ++parsed;
            ++id;
        }

        if (id[0] != '=') {
            return -parsed;
        }

        ++parsed;
        ++id;

        while (isspace(id[0])) {
            ++parsed;
            ++id;
        }

        /* ((DQUOTE string DQUOTE) / (SQUOTE string SQUOTE)) */
        if ((id[0] == '\"') || (id[0] == '\'')) {
            quote = id[0];

            ++parsed;
            ++id;

            if ((ptr = strchr(id, quote)) == NULL) {
                return -parsed;
            }
            ret = ptr - id;

            if (value) {
                *value = id;
            }
            if (val_len) {
                *val_len = ret;
            }

            parsed += ret + 1;
            id += ret + 1;
        } else {
            return -parsed;
        }
    }

    while (isspace(id[0])) {
        ++parsed;
        ++id;
    }

    if (id[0] != ']') {
        return -parsed;
    }

    ++parsed;
    ++id;

    if ((id[0] == '[') && has_predicate) {
        *has_predicate = 1;
    }

    return parsed;
}

/**
 * @brief Parse schema-nodeid.
 *
 * schema-nodeid       = absolute-schema-nodeid /
 *                       descendant-schema-nodeid
 * absolute-schema-nodeid = 1*("/" node-identifier)
 * descendant-schema-nodeid = ["." "/"]
 *                       node-identifier
 *                       absolute-schema-nodeid
 *
 * @param[in] id Identifier to use.
 * @param[out] mod_name Points to the module name, NULL if there is not any.
 * @param[out] mod_name_len Length of the module name, 0 if there is not any.
 * @param[out] name Points to the node name.
 * @param[out] nam_len Length of the node name.
 * @param[out] is_relative Flag to mark whether the nodeid is absolute or descendant. Must be -1
 *                         on the first call, must not be changed between consecutive calls.
 * @param[out] has_predicate Flag to mark whether there is a predicate specified. It cannot be
 *                           based on the grammar, in those cases use NULL.
 * @param[in] extended Whether to accept an extended path (support for /[prefix:]*, //[prefix:]*, //[prefix:].).
 *
 * @return Number of characters successfully parsed,
 *         positive on success, negative on failure.
 */
int
parse_schema_nodeid(const char *id, const char **mod_name, int *mod_name_len, const char **name, int *nam_len,
                    int *is_relative, int *has_predicate, int *all_desc, int extended)
{
    int parsed = 0, ret;

    assert(id);
    assert(is_relative);

    if (has_predicate) {
        *has_predicate = 0;
    }

    if (id[0] != '/') {
        if (*is_relative != -1) {
            return -parsed;
        } else {
            *is_relative = 1;
        }
        if (!strncmp(id, "./", 2)) {
            parsed += 2;
            id += 2;
        }
    } else {
        if (*is_relative == -1) {
            *is_relative = 0;
        }
        ++parsed;
        ++id;
    }

    if ((ret = parse_node_identifier(id, mod_name, mod_name_len, name, nam_len, all_desc, extended)) < 1) {
        return -parsed + ret;
    }

    parsed += ret;
    id += ret;

    if ((id[0] == '[') && has_predicate) {
        *has_predicate = 1;
    }

    return parsed;
}

/**
 * @brief Parse schema predicate (special format internally used).
 *
 * predicate           = "[" *WSP predicate-expr *WSP "]"
 * predicate-expr      = "." / [prefix:]identifier / positive-integer / key-with-value
 * key-with-value      = identifier *WSP "=" *WSP
 *                       ((DQUOTE string DQUOTE) /
 *                        (SQUOTE string SQUOTE))
 *
 * @param[in] id Identifier to use.
 * @param[out] mod_name Points to the list key module name.
 * @param[out] mod_name_len Length of \p mod_name.
 * @param[out] name Points to the list key name.
 * @param[out] nam_len Length of \p name.
 * @param[out] value Points to the key value. If specified, key-with-value is expected.
 * @param[out] val_len Length of \p value.
 * @param[out] has_predicate Flag to mark whether there is another predicate specified.
 */
int
parse_schema_json_predicate(const char *id, const char **mod_name, int *mod_name_len, const char **name, int *nam_len,
                            const char **value, int *val_len, int *has_predicate)
{
    const char *ptr;
    int parsed = 0, ret;
    char quote;

    assert(id);
    if (mod_name) {
        *mod_name = NULL;
    }
    if (mod_name_len) {
        *mod_name_len = 0;
    }
    if (name) {
        *name = NULL;
    }
    if (nam_len) {
        *nam_len = 0;
    }
    if (value) {
        *value = NULL;
    }
    if (val_len) {
        *val_len = 0;
    }
    if (has_predicate) {
        *has_predicate = 0;
    }

    if (id[0] != '[') {
        return -parsed;
    }

    ++parsed;
    ++id;

    while (isspace(id[0])) {
        ++parsed;
        ++id;
    }

    /* identifier */
    if (id[0] == '.') {
        ret = 1;

        if (name) {
            *name = id;
        }
        if (nam_len) {
            *nam_len = ret;
        }
    } else if (isdigit(id[0])) {
        if (id[0] == '0') {
            return -parsed;
        }
        ret = 1;
        while (isdigit(id[ret])) {
            ++ret;
        }

        if (name) {
            *name = id;
        }
        if (nam_len) {
            *nam_len = ret;
        }
    } else if ((ret = parse_node_identifier(id, mod_name, mod_name_len, name, nam_len, NULL, 0)) < 1) {
        return -parsed + ret;
    }

    parsed += ret;
    id += ret;

    while (isspace(id[0])) {
        ++parsed;
        ++id;
    }

    /* there is value as well */
    if (id[0] == '=') {
        if (name && isdigit(**name)) {
            return -parsed;
        }

        ++parsed;
        ++id;

        while (isspace(id[0])) {
            ++parsed;
            ++id;
        }

        /* ((DQUOTE string DQUOTE) / (SQUOTE string SQUOTE)) */
        if ((id[0] == '\"') || (id[0] == '\'')) {
            quote = id[0];

            ++parsed;
            ++id;

            if ((ptr = strchr(id, quote)) == NULL) {
                return -parsed;
            }
            ret = ptr - id;

            if (value) {
                *value = id;
            }
            if (val_len) {
                *val_len = ret;
            }

            parsed += ret + 1;
            id += ret + 1;
        } else {
            return -parsed;
        }

        while (isspace(id[0])) {
            ++parsed;
            ++id;
        }
    }

    if (id[0] != ']') {
        return -parsed;
    }

    ++parsed;
    ++id;

    if ((id[0] == '[') && has_predicate) {
        *has_predicate = 1;
    }

    return parsed;
}

#ifdef LY_ENABLED_CACHE

static int
resolve_hash_table_find_equal(void *val1_p, void *val2_p, int mod, void *UNUSED(cb_data))
{
    struct lyd_node *val2, *elem2;
    struct parsed_pred pp;
    const char *str;
    int i;

    assert(!mod);
    (void)mod;

    pp = *((struct parsed_pred *)val1_p);
    val2 = *((struct lyd_node **)val2_p);

    if (val2->schema != pp.schema) {
        return 0;
    }

    switch (val2->schema->nodetype) {
    case LYS_CONTAINER:
    case LYS_LEAF:
    case LYS_ANYXML:
    case LYS_ANYDATA:
        return 1;
    case LYS_LEAFLIST:
        str = ((struct lyd_node_leaf_list *)val2)->value_str;
        if (!strncmp(str, pp.pred[0].value, pp.pred[0].val_len) && !str[pp.pred[0].val_len]) {
            return 1;
        }
        return 0;
    case LYS_LIST:
        assert(((struct lys_node_list *)val2->schema)->keys_size);
        assert(((struct lys_node_list *)val2->schema)->keys_size == pp.len);

        /* lists with keys, their equivalence is based on their keys */
        elem2 = val2->child;
        /* the exact data order is guaranteed */
        for (i = 0; elem2 && (i < pp.len); ++i) {
            /* module check */
            if (pp.pred[i].mod_name) {
                if (strncmp(lyd_node_module(elem2)->name, pp.pred[i].mod_name, pp.pred[i].mod_name_len)
                        || lyd_node_module(elem2)->name[pp.pred[i].mod_name_len]) {
                    break;
                }
            } else {
                if (lyd_node_module(elem2) != lys_node_module(pp.schema)) {
                    break;
                }
            }

            /* name check */
            if (strncmp(elem2->schema->name, pp.pred[i].name, pp.pred[i].nam_len) || elem2->schema->name[pp.pred[i].nam_len]) {
                break;
            }

            /* value check */
            str = ((struct lyd_node_leaf_list *)elem2)->value_str;
            if (strncmp(str, pp.pred[i].value, pp.pred[i].val_len) || str[pp.pred[i].val_len]) {
                break;
            }

            /* next key */
            elem2 = elem2->next;
        }
        if (i == pp.len) {
            return 1;
        }
        return 0;
    default:
        break;
    }

    LOGINT(val2->schema->module->ctx);
    return 0;
}

static struct lyd_node *
resolve_json_data_node_hash(struct lyd_node *parent, struct parsed_pred pp)
{
    values_equal_cb prev_cb;
    struct lyd_node **ret = NULL;
    uint32_t hash;
    int i;

    assert(parent && parent->hash);

    /* set our value equivalence callback that does not require data nodes */
    prev_cb = lyht_set_cb(parent->ht, resolve_hash_table_find_equal);

    /* get the hash of the searched node */
    hash = dict_hash_multi(0, lys_node_module(pp.schema)->name, strlen(lys_node_module(pp.schema)->name));
    hash = dict_hash_multi(hash, pp.schema->name, strlen(pp.schema->name));
    if (pp.schema->nodetype == LYS_LEAFLIST) {
        assert((pp.len == 1) && (pp.pred[0].name[0] == '.') && (pp.pred[0].nam_len == 1));
        /* leaf-list value in predicate */
        hash = dict_hash_multi(hash, pp.pred[0].value, pp.pred[0].val_len);
    } else if (pp.schema->nodetype == LYS_LIST) {
        /* list keys in predicates */
        for (i = 0; i < pp.len; ++i) {
            hash = dict_hash_multi(hash, pp.pred[i].value, pp.pred[i].val_len);
        }
    }
    hash = dict_hash_multi(hash, NULL, 0);

    /* try to find the node */
    i = lyht_find(parent->ht, &pp, hash, (void **)&ret);
    assert(i || *ret);

    /* restore the original callback */
    lyht_set_cb(parent->ht, prev_cb);

    return (i ? NULL : *ret);
}

#endif

/**
 * @brief Resolve (find) a feature definition. Logs directly.
 *
 * @param[in] feat_name Feature name to resolve.
 * @param[in] len Length of \p feat_name.
 * @param[in] node Node with the if-feature expression.
 * @param[out] feature Pointer to be set to point to the feature definition, if feature not found
 * (return code 1), the pointer is untouched.
 *
 * @return 0 on success, 1 on forward reference, -1 on error.
 */
static int
resolve_feature(const char *feat_name, uint16_t len, const struct lys_node *node, struct lys_feature **feature)
{
    char *str;
    const char *mod_name, *name;
    int mod_name_len, nam_len, i, j;
    const struct lys_module *module;

    assert(feature);

    /* check prefix */
    if ((i = parse_node_identifier(feat_name, &mod_name, &mod_name_len, &name, &nam_len, NULL, 0)) < 1) {
        LOGVAL(node->module->ctx, LYE_INCHAR, LY_VLOG_NONE, NULL, feat_name[-i], &feat_name[-i]);
        return -1;
    }

    module = lyp_get_module(lys_node_module(node), NULL, 0, mod_name, mod_name_len, 0);
    if (!module) {
        /* identity refers unknown data model */
        LOGVAL(node->module->ctx, LYE_INMOD_LEN, LY_VLOG_NONE, NULL, mod_name_len, mod_name);
        return -1;
    }

    if (module != node->module && module == lys_node_module(node)) {
        /* first, try to search directly in submodule where the feature was mentioned */
        for (j = 0; j < node->module->features_size; j++) {
            if (!strncmp(name, node->module->features[j].name, nam_len) && !node->module->features[j].name[nam_len]) {
                /* check status */
                if (lyp_check_status(node->flags, lys_node_module(node), node->name, node->module->features[j].flags,
                                     node->module->features[j].module, node->module->features[j].name, NULL)) {
                    return -1;
                }
                *feature = &node->module->features[j];
                return 0;
            }
        }
    }

    /* search in the identified module ... */
    for (j = 0; j < module->features_size; j++) {
        if (!strncmp(name, module->features[j].name, nam_len) && !module->features[j].name[nam_len]) {
            /* check status */
            if (lyp_check_status(node->flags, lys_node_module(node), node->name, module->features[j].flags,
                                 module->features[j].module, module->features[j].name, NULL)) {
                return -1;
            }
            *feature = &module->features[j];
            return 0;
        }
    }
    /* ... and all its submodules */
    for (i = 0; i < module->inc_size && module->inc[i].submodule; i++) {
        for (j = 0; j < module->inc[i].submodule->features_size; j++) {
            if (!strncmp(name, module->inc[i].submodule->features[j].name, nam_len)
                    && !module->inc[i].submodule->features[j].name[nam_len]) {
                /* check status */
                if (lyp_check_status(node->flags, lys_node_module(node), node->name,
                                     module->inc[i].submodule->features[j].flags,
                                     module->inc[i].submodule->features[j].module,
                                     module->inc[i].submodule->features[j].name, NULL)) {
                    return -1;
                }
                *feature = &module->inc[i].submodule->features[j];
                return 0;
            }
        }
    }

    /* not found */
    str = strndup(feat_name, len);
    LOGVAL(node->module->ctx, LYE_INRESOLV, LY_VLOG_NONE, NULL, "feature", str);
    free(str);
    return 1;
}

/*
 * @return
 *  -  1 if enabled
 *  -  0 if disabled
 */
static int
resolve_feature_value(const struct lys_feature *feat)
{
    int i;

    for (i = 0; i < feat->iffeature_size; i++) {
        if (!resolve_iffeature(&feat->iffeature[i])) {
            return 0;
        }
    }

    return feat->flags & LYS_FENABLED ? 1 : 0;
}

static int
resolve_iffeature_recursive(struct lys_iffeature *expr, int *index_e, int *index_f)
{
    uint8_t op;
    int a, b;

    op = iff_getop(expr->expr, *index_e);
    (*index_e)++;

    switch (op) {
    case LYS_IFF_F:
        /* resolve feature */
        return resolve_feature_value(expr->features[(*index_f)++]);
    case LYS_IFF_NOT:
        /* invert result */
        return resolve_iffeature_recursive(expr, index_e, index_f) ? 0 : 1;
    case LYS_IFF_AND:
    case LYS_IFF_OR:
        a = resolve_iffeature_recursive(expr, index_e, index_f);
        b = resolve_iffeature_recursive(expr, index_e, index_f);
        if (op == LYS_IFF_AND) {
            return a && b;
        } else { /* LYS_IFF_OR */
            return a || b;
        }
    }

    return 0;
}

int
resolve_iffeature(struct lys_iffeature *expr)
{
    int index_e = 0, index_f = 0;

    if (expr->expr) {
        return resolve_iffeature_recursive(expr, &index_e, &index_f);
    }
    return 0;
}

struct iff_stack {
    int size;
    int index;     /* first empty item */
    uint8_t *stack;
};

static int
iff_stack_push(struct iff_stack *stack, uint8_t value)
{
    if (stack->index == stack->size) {
        stack->size += 4;
        stack->stack = ly_realloc(stack->stack, stack->size * sizeof *stack->stack);
        LY_CHECK_ERR_RETURN(!stack->stack, LOGMEM(NULL); stack->size = 0, EXIT_FAILURE);
    }

    stack->stack[stack->index++] = value;
    return EXIT_SUCCESS;
}

static uint8_t
iff_stack_pop(struct iff_stack *stack)
{
    stack->index--;
    return stack->stack[stack->index];
}

static void
iff_stack_clean(struct iff_stack *stack)
{
    stack->size = 0;
    free(stack->stack);
}

static void
iff_setop(uint8_t *list, uint8_t op, int pos)
{
    uint8_t *item;
    uint8_t mask = 3;

    assert(pos >= 0);
    assert(op <= 3); /* max 2 bits */

    item = &list[pos / 4];
    mask = mask << 2 * (pos % 4);
    *item = (*item) & ~mask;
    *item = (*item) | (op << 2 * (pos % 4));
}

uint8_t
iff_getop(uint8_t *list, int pos)
{
    uint8_t *item;
    uint8_t mask = 3, result;

    assert(pos >= 0);

    item = &list[pos / 4];
    result = (*item) & (mask << 2 * (pos % 4));
    return result >> 2 * (pos % 4);
}

#define LYS_IFF_LP 0x04 /* ( */
#define LYS_IFF_RP 0x08 /* ) */

/* internal structure for passing data for UNRES_IFFEAT */
struct unres_iffeat_data {
    struct lys_node *node;
    const char *fname;
    int infeature;
};

void
resolve_iffeature_getsizes(struct lys_iffeature *iffeat, unsigned int *expr_size, unsigned int *feat_size)
{
    unsigned int e = 0, f = 0, r = 0;
    uint8_t op;

    assert(iffeat);

    if (!iffeat->expr) {
        goto result;
    }

    do {
        op = iff_getop(iffeat->expr, e++);
        switch (op) {
        case LYS_IFF_NOT:
            if (!r) {
                r += 1;
            }
            break;
        case LYS_IFF_AND:
        case LYS_IFF_OR:
            if (!r) {
                r += 2;
            } else {
                r += 1;
            }
            break;
        case LYS_IFF_F:
            f++;
            if (r) {
                r--;
            }
            break;
        }
    } while(r);

result:
    if (expr_size) {
        *expr_size = e;
    }
    if (feat_size) {
        *feat_size = f;
    }
}

int
resolve_iffeature_compile(struct lys_iffeature *iffeat_expr, const char *value, struct lys_node *node,
                          int infeature, struct unres_schema *unres)
{
    const char *c = value;
    int r, rc = EXIT_FAILURE;
    int i, j, last_not, checkversion = 0;
    unsigned int f_size = 0, expr_size = 0, f_exp = 1;
    uint8_t op;
    struct iff_stack stack = {0, 0, NULL};
    struct unres_iffeat_data *iff_data;
    struct ly_ctx *ctx = node->module->ctx;

    assert(c);

    if (isspace(c[0])) {
        LOGVAL(ctx, LYE_INCHAR, LY_VLOG_NONE, NULL, c[0], c);
        return EXIT_FAILURE;
    }

    /* pre-parse the expression to get sizes for arrays, also do some syntax checks of the expression */
    for (i = j = last_not = 0; c[i]; i++) {
        if (c[i] == '(') {
            checkversion = 1;
            j++;
            continue;
        } else if (c[i] == ')') {
            j--;
            continue;
        } else if (isspace(c[i])) {
            continue;
        }

        if (!strncmp(&c[i], "not", r = 3) || !strncmp(&c[i], "and", r = 3) || !strncmp(&c[i], "or", r = 2)) {
            if (c[i + r] == '\0') {
                LOGVAL(ctx, LYE_INARG, LY_VLOG_NONE, NULL, value, "if-feature");
                return EXIT_FAILURE;
            } else if (!isspace(c[i + r])) {
                /* feature name starting with the not/and/or */
                last_not = 0;
                f_size++;
            } else if (c[i] == 'n') { /* not operation */
                if (last_not) {
                    /* double not */
                    expr_size = expr_size - 2;
                    last_not = 0;
                } else {
                    last_not = 1;
                }
            } else { /* and, or */
                f_exp++;
                /* not a not operation */
                last_not = 0;
            }
            i += r;
        } else {
            f_size++;
            last_not = 0;
        }
        expr_size++;

        while (!isspace(c[i])) {
            if (!c[i] || c[i] == ')') {
                i--;
                break;
            }
            i++;
        }
    }
    if (j || f_exp != f_size) {
        /* not matching count of ( and ) */
        LOGVAL(ctx, LYE_INARG, LY_VLOG_NONE, NULL, value, "if-feature");
        return EXIT_FAILURE;
    }

    if (checkversion || expr_size > 1) {
        /* check that we have 1.1 module */
        if (node->module->version != LYS_VERSION_1_1) {
            LOGVAL(ctx, LYE_INARG, LY_VLOG_NONE, NULL, value, "if-feature");
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "YANG 1.1 if-feature expression found in 1.0 module.");
            return EXIT_FAILURE;
        }
    }

    /* allocate the memory */
    iffeat_expr->expr = calloc((j = (expr_size / 4) + ((expr_size % 4) ? 1 : 0)), sizeof *iffeat_expr->expr);
    iffeat_expr->features = calloc(f_size, sizeof *iffeat_expr->features);
    stack.stack = malloc(expr_size * sizeof *stack.stack);
    LY_CHECK_ERR_GOTO(!stack.stack || !iffeat_expr->expr || !iffeat_expr->features, LOGMEM(ctx), error);
    stack.size = expr_size;
    f_size--; expr_size--; /* used as indexes from now */

    for (i--; i >= 0; i--) {
        if (c[i] == ')') {
            /* push it on stack */
            iff_stack_push(&stack, LYS_IFF_RP);
            continue;
        } else if (c[i] == '(') {
            /* pop from the stack into result all operators until ) */
            while((op = iff_stack_pop(&stack)) != LYS_IFF_RP) {
                iff_setop(iffeat_expr->expr, op, expr_size--);
            }
            continue;
        } else if (isspace(c[i])) {
            continue;
        }

        /* end operator or operand -> find beginning and get what is it */
        j = i + 1;
        while (i >= 0 && !isspace(c[i]) && c[i] != '(') {
            i--;
        }
        i++; /* get back by one step */

        if (!strncmp(&c[i], "not", 3) && isspace(c[i + 3])) {
            if (stack.index && stack.stack[stack.index - 1] == LYS_IFF_NOT) {
                /* double not */
                iff_stack_pop(&stack);
            } else {
                /* not has the highest priority, so do not pop from the stack
                 * as in case of AND and OR */
                iff_stack_push(&stack, LYS_IFF_NOT);
            }
        } else if (!strncmp(&c[i], "and", 3) && isspace(c[i + 3])) {
            /* as for OR - pop from the stack all operators with the same or higher
             * priority and store them to the result, then push the AND to the stack */
            while (stack.index && stack.stack[stack.index - 1] <= LYS_IFF_AND) {
                op = iff_stack_pop(&stack);
                iff_setop(iffeat_expr->expr, op, expr_size--);
            }
            iff_stack_push(&stack, LYS_IFF_AND);
        } else if (!strncmp(&c[i], "or", 2) && isspace(c[i + 2])) {
            while (stack.index && stack.stack[stack.index - 1] <= LYS_IFF_OR) {
                op = iff_stack_pop(&stack);
                iff_setop(iffeat_expr->expr, op, expr_size--);
            }
            iff_stack_push(&stack, LYS_IFF_OR);
        } else {
            /* feature name, length is j - i */

            /* add it to the result */
            iff_setop(iffeat_expr->expr, LYS_IFF_F, expr_size--);

            /* now get the link to the feature definition. Since it can be
             * forward referenced, we have to keep the feature name in auxiliary
             * structure passed into unres */
            iff_data = malloc(sizeof *iff_data);
            LY_CHECK_ERR_GOTO(!iff_data, LOGMEM(ctx), error);
            iff_data->node = node;
            iff_data->fname = lydict_insert(node->module->ctx, &c[i], j - i);
            iff_data->infeature = infeature;
            r = unres_schema_add_node(node->module, unres, &iffeat_expr->features[f_size], UNRES_IFFEAT,
                                      (struct lys_node *)iff_data);
            f_size--;

            if (r == -1) {
                lydict_remove(node->module->ctx, iff_data->fname);
                free(iff_data);
                goto error;
            }
        }
    }
    while (stack.index) {
        op = iff_stack_pop(&stack);
        iff_setop(iffeat_expr->expr, op, expr_size--);
    }

    if (++expr_size || ++f_size) {
        /* not all expected operators and operands found */
        LOGVAL(ctx, LYE_INARG, LY_VLOG_NONE, NULL, value, "if-feature");
        rc = EXIT_FAILURE;
    } else {
        rc = EXIT_SUCCESS;
    }

error:
    /* cleanup */
    iff_stack_clean(&stack);

    return rc;
}

/**
 * @brief Resolve (find) a data node based on a schema-nodeid.
 *
 * Used for resolving unique statements - so id is expected to be relative and local (without reference to a different
 * module).
 *
 */
struct lyd_node *
resolve_data_descendant_schema_nodeid(const char *nodeid, struct lyd_node *start)
{
    char *str, *token, *p;
    struct lyd_node *result = NULL, *iter;
    const struct lys_node *schema = NULL;

    assert(nodeid && start);

    if (nodeid[0] == '/') {
        return NULL;
    }

    str = p = strdup(nodeid);
    LY_CHECK_ERR_RETURN(!str, LOGMEM(start->schema->module->ctx), NULL);

    while (p) {
        token = p;
        p = strchr(p, '/');
        if (p) {
            *p = '\0';
            p++;
        }

        if (p) {
            /* inner node */
            if (resolve_descendant_schema_nodeid(token, schema ? schema->child : start->schema,
                                                 LYS_CONTAINER | LYS_CHOICE | LYS_CASE | LYS_LEAF, 0, &schema)
                    || !schema) {
                result = NULL;
                break;
            }

            if (schema->nodetype & (LYS_CHOICE | LYS_CASE)) {
                continue;
            }
        } else {
            /* final node */
            if (resolve_descendant_schema_nodeid(token, schema ? schema->child : start->schema, LYS_LEAF, 0, &schema)
                    || !schema) {
                result = NULL;
                break;
            }
        }
        LY_TREE_FOR(result ? result->child : start, iter) {
            if (iter->schema == schema) {
                /* move in data tree according to returned schema */
                result = iter;
                break;
            }
        }
        if (!iter) {
            /* instance not found */
            result = NULL;
            break;
        }
    }
    free(str);

    return result;
}

int
schema_nodeid_siblingcheck(const struct lys_node *sibling, const struct lys_module *cur_module, const char *mod_name,
                           int mod_name_len, const char *name, int nam_len)
{
    const struct lys_module *prefix_mod;

    /* handle special names */
    if (name[0] == '*') {
        return 2;
    } else if (name[0] == '.') {
        return 3;
    }

    /* name check */
    if (strncmp(name, sibling->name, nam_len) || sibling->name[nam_len]) {
        return 1;
    }

    /* module check */
    if (mod_name) {
        prefix_mod = lyp_get_module(cur_module, NULL, 0, mod_name, mod_name_len, 0);
        if (!prefix_mod) {
            return -1;
        }
    } else {
        prefix_mod = cur_module;
    }
    if (prefix_mod != lys_node_module(sibling)) {
        return 1;
    }

    /* match */
    return 0;
}

/* keys do not have to be ordered and do not have to be all of them */
static int
resolve_extended_schema_nodeid_predicate(const char *nodeid, const struct lys_node *node,
                                         const struct lys_module *cur_module, int *nodeid_end)
{
    int mod_len, nam_len, has_predicate, r, i;
    const char *model, *name;
    struct lys_node_list *list;

    if (!(node->nodetype & (LYS_LIST | LYS_LEAFLIST))) {
        return 1;
    }

    list = (struct lys_node_list *)node;
    do {
        r = parse_schema_json_predicate(nodeid, &model, &mod_len, &name, &nam_len, NULL, NULL, &has_predicate);
        if (r < 1) {
            LOGVAL(cur_module->ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, nodeid[r], &nodeid[r]);
            return -1;
        }
        nodeid += r;

        if (node->nodetype == LYS_LEAFLIST) {
            /* just check syntax */
            if (model || !name || (name[0] != '.') || has_predicate) {
                return 1;
            }
            break;
        } else {
            /* check the key */
            for (i = 0; i < list->keys_size; ++i) {
                if (strncmp(list->keys[i]->name, name, nam_len) || list->keys[i]->name[nam_len]) {
                    continue;
                }
                if (model) {
                    if (strncmp(lys_node_module((struct lys_node *)list->keys[i])->name, model, mod_len)
                            || lys_node_module((struct lys_node *)list->keys[i])->name[mod_len]) {
                        continue;
                    }
                } else {
                    if (lys_node_module((struct lys_node *)list->keys[i]) != cur_module) {
                        continue;
                    }
                }

                /* match */
                break;
            }

            if (i == list->keys_size) {
                return 1;
            }
        }
    } while (has_predicate);

    if (!nodeid[0]) {
        *nodeid_end = 1;
    }
    return 0;
}

/* start_parent - relative, module - absolute, -1 error (logged), EXIT_SUCCESS ok
 */
int
resolve_schema_nodeid(const char *nodeid, const struct lys_node *start_parent, const struct lys_module *cur_module,
                      struct ly_set **ret, int extended, int no_node_error)
{
    const char *name, *mod_name, *id, *backup_mod_name = NULL, *yang_data_name = NULL;
    const struct lys_node *sibling, *next, *elem;
    struct lys_node_augment *last_aug;
    int r, nam_len, mod_name_len = 0, is_relative = -1, all_desc, has_predicate, nodeid_end = 0;
    int yang_data_name_len, backup_mod_name_len = 0;
    /* resolved import module from the start module, it must match the next node-name-match sibling */
    const struct lys_module *start_mod, *aux_mod = NULL;
    char *str;
    struct ly_ctx *ctx;

    assert(nodeid && (start_parent || cur_module) && ret);
    *ret = NULL;

    if (!cur_module) {
        cur_module = lys_node_module(start_parent);
    }
    ctx = cur_module->ctx;
    id = nodeid;

    r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, NULL, NULL, 1);
    if (r < 1) {
        LOGVAL(ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[r], &id[r]);
        return -1;
    }

    if (name[0] == '#') {
        if (is_relative) {
            LOGVAL(ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, '#', name);
            return -1;
        }
        yang_data_name = name + 1;
        yang_data_name_len = nam_len - 1;
        backup_mod_name = mod_name;
        backup_mod_name_len = mod_name_len;
        id += r;
    } else {
        is_relative = -1;
    }

    r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, &has_predicate,
                            (extended ? &all_desc : NULL), extended);
    if (r < 1) {
        LOGVAL(ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[r], &id[r]);
        return -1;
    }
    id += r;

    if (backup_mod_name) {
        mod_name = backup_mod_name;
        mod_name_len = backup_mod_name_len;
    }

    if (is_relative && !start_parent) {
        LOGVAL(ctx, LYE_SPEC, LY_VLOG_STR, nodeid, "Starting node must be provided for relative paths.");
        return -1;
    }

    /* descendant-schema-nodeid */
    if (is_relative) {
        cur_module = start_mod = lys_node_module(start_parent);

    /* absolute-schema-nodeid */
    } else {
        start_mod = lyp_get_module(cur_module, NULL, 0, mod_name, mod_name_len, 0);
        if (!start_mod) {
            str = strndup(mod_name, mod_name_len);
            LOGVAL(ctx, LYE_PATH_INMOD, LY_VLOG_STR, str);
            free(str);
            return -1;
        }
        start_parent = NULL;
        if (yang_data_name) {
            start_parent = lyp_get_yang_data_template(start_mod, yang_data_name, yang_data_name_len);
            if (!start_parent) {
                str = strndup(nodeid, (yang_data_name + yang_data_name_len) - nodeid);
                LOGVAL(ctx, LYE_PATH_INNODE, LY_VLOG_STR, str);
                free(str);
                return -1;
            }
        }
    }

    while (1) {
        sibling = NULL;
        last_aug = NULL;

        if (start_parent) {
            if (mod_name && (strncmp(mod_name, cur_module->name, mod_name_len)
                    || (mod_name_len != (signed)strlen(cur_module->name)))) {
                /* we are getting into another module (augment) */
                aux_mod = lyp_get_module(cur_module, NULL, 0, mod_name, mod_name_len, 0);
                if (!aux_mod) {
                    str = strndup(mod_name, mod_name_len);
                    LOGVAL(ctx, LYE_PATH_INMOD, LY_VLOG_STR, str);
                    free(str);
                    return -1;
                }
            } else {
                /* there is no mod_name, so why are we checking augments again?
                 * because this module may be not implemented and it augments something in another module and
                 * there is another augment augmenting that previous one */
                aux_mod = cur_module;
            }

            /* look into augments */
            if (!extended) {
get_next_augment:
                last_aug = lys_getnext_target_aug(last_aug, aux_mod, start_parent);
            }
        }

        while ((sibling = lys_getnext(sibling, (last_aug ? (struct lys_node *)last_aug : start_parent), start_mod,
                LYS_GETNEXT_WITHCHOICE | LYS_GETNEXT_WITHCASE | LYS_GETNEXT_WITHINOUT | LYS_GETNEXT_PARENTUSES | LYS_GETNEXT_NOSTATECHECK))) {
            r = schema_nodeid_siblingcheck(sibling, cur_module, mod_name, mod_name_len, name, nam_len);

            /* resolve predicate */
            if (extended && ((r == 0) || (r == 2) || (r == 3)) && has_predicate) {
                r = resolve_extended_schema_nodeid_predicate(id, sibling, cur_module, &nodeid_end);
                if (r == 1) {
                    continue;
                } else if (r == -1) {
                    return -1;
                }
            } else if (!id[0]) {
                nodeid_end = 1;
            }

            if (r == 0) {
                /* one matching result */
                if (nodeid_end) {
                    *ret = ly_set_new();
                    LY_CHECK_ERR_RETURN(!*ret, LOGMEM(ctx), -1);
                    ly_set_add(*ret, (void *)sibling, LY_SET_OPT_USEASLIST);
                } else {
                    if (sibling->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) {
                        return -1;
                    }
                    start_parent = sibling;
                }
                break;
            } else if (r == 1) {
                continue;
            } else if (r == 2) {
                /* "*" */
                if (!*ret) {
                    *ret = ly_set_new();
                    LY_CHECK_ERR_RETURN(!*ret, LOGMEM(ctx), -1);
                }
                ly_set_add(*ret, (void *)sibling, LY_SET_OPT_USEASLIST);
                if (all_desc) {
                    LY_TREE_DFS_BEGIN(sibling, next, elem) {
                        if (elem != sibling) {
                            ly_set_add(*ret, (void *)elem, LY_SET_OPT_USEASLIST);
                        }

                        LY_TREE_DFS_END(sibling, next, elem);
                    }
                }
            } else if (r == 3) {
                /* "." */
                if (!*ret) {
                    *ret = ly_set_new();
                    LY_CHECK_ERR_RETURN(!*ret, LOGMEM(ctx), -1);
                    ly_set_add(*ret, (void *)start_parent, LY_SET_OPT_USEASLIST);
                }
                ly_set_add(*ret, (void *)sibling, LY_SET_OPT_USEASLIST);
                if (all_desc) {
                    LY_TREE_DFS_BEGIN(sibling, next, elem) {
                        if (elem != sibling) {
                            ly_set_add(*ret, (void *)elem, LY_SET_OPT_USEASLIST);
                        }

                        LY_TREE_DFS_END(sibling, next, elem);
                    }
                }
            } else {
                LOGINT(ctx);
                return -1;
            }
        }

        /* skip predicate */
        if (extended && has_predicate) {
            while (id[0] == '[') {
                id = strchr(id, ']');
                if (!id) {
                    LOGINT(ctx);
                    return -1;
                }
                ++id;
            }
        }

        if (nodeid_end && ((r == 0) || (r == 2) || (r == 3))) {
            return EXIT_SUCCESS;
        }

        /* no match */
        if (!sibling) {
            if (last_aug) {
                /* it still could be in another augment */
                goto get_next_augment;
            }
            if (no_node_error) {
                str = strndup(nodeid, (name - nodeid) + nam_len);
                LOGVAL(ctx, LYE_PATH_INNODE, LY_VLOG_STR, str);
                free(str);
                return -1;
            }
            *ret = NULL;
            return EXIT_SUCCESS;
        }

        r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, &has_predicate,
                                (extended ? &all_desc : NULL), extended);
        if (r < 1) {
            LOGVAL(ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[r], &id[r]);
            return -1;
        }
        id += r;
    }

    /* cannot get here */
    LOGINT(ctx);
    return -1;
}

/* unique, refine,
 * >0  - unexpected char on position (ret - 1),
 *  0  - ok (but ret can still be NULL),
 * -1  - error,
 * -2  - violated no_innerlist  */
int
resolve_descendant_schema_nodeid(const char *nodeid, const struct lys_node *start, int ret_nodetype,
                                 int no_innerlist, const struct lys_node **ret)
{
    const char *name, *mod_name, *id;
    const struct lys_node *sibling, *start_parent;
    int r, nam_len, mod_name_len, is_relative = -1;
    /* resolved import module from the start module, it must match the next node-name-match sibling */
    const struct lys_module *module;

    assert(nodeid && ret);
    assert(!(ret_nodetype & (LYS_USES | LYS_AUGMENT | LYS_GROUPING)));

    if (!start) {
        /* leaf not found */
        return 0;
    }

    id = nodeid;
    module = lys_node_module(start);

    if ((r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, NULL, NULL, 0)) < 1) {
        return ((id - nodeid) - r) + 1;
    }
    id += r;

    if (!is_relative) {
        return -1;
    }

    start_parent = lys_parent(start);
    while ((start_parent->nodetype == LYS_USES) && lys_parent(start_parent)) {
        start_parent = lys_parent(start_parent);
    }

    while (1) {
        sibling = NULL;
        while ((sibling = lys_getnext(sibling, start_parent, module,
                LYS_GETNEXT_WITHCHOICE | LYS_GETNEXT_WITHCASE | LYS_GETNEXT_PARENTUSES | LYS_GETNEXT_NOSTATECHECK))) {
            r = schema_nodeid_siblingcheck(sibling, module, mod_name, mod_name_len, name, nam_len);
            if (r == 0) {
                if (!id[0]) {
                    if (!(sibling->nodetype & ret_nodetype)) {
                        /* wrong node type, too bad */
                        continue;
                    }
                    *ret = sibling;
                    return EXIT_SUCCESS;
                }
                start_parent = sibling;
                break;
            } else if (r == 1) {
                continue;
            } else {
                return -1;
            }
        }

        /* no match */
        if (!sibling) {
            *ret = NULL;
            return EXIT_SUCCESS;
        } else if (no_innerlist && sibling->nodetype == LYS_LIST) {
            *ret = NULL;
            return -2;
        }

        if ((r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, NULL, NULL, 0)) < 1) {
            return ((id - nodeid) - r) + 1;
        }
        id += r;
    }

    /* cannot get here */
    LOGINT(module->ctx);
    return -1;
}

/* choice default */
int
resolve_choice_default_schema_nodeid(const char *nodeid, const struct lys_node *start, const struct lys_node **ret)
{
    /* cannot actually be a path */
    if (strchr(nodeid, '/')) {
        return -1;
    }

    return resolve_descendant_schema_nodeid(nodeid, start, LYS_NO_RPC_NOTIF_NODE, 0, ret);
}

/* uses, -1 error, EXIT_SUCCESS ok (but ret can still be NULL), >0 unexpected char on ret - 1 */
static int
resolve_uses_schema_nodeid(const char *nodeid, const struct lys_node *start, const struct lys_node_grp **ret)
{
    const struct lys_module *module;
    const char *mod_prefix, *name;
    int i, mod_prefix_len, nam_len;

    /* parse the identifier, it must be parsed on one call */
    if (((i = parse_node_identifier(nodeid, &mod_prefix, &mod_prefix_len, &name, &nam_len, NULL, 0)) < 1) || nodeid[i]) {
        return -i + 1;
    }

    module = lyp_get_module(start->module, mod_prefix, mod_prefix_len, NULL, 0, 0);
    if (!module) {
        return -1;
    }
    if (module != lys_main_module(start->module)) {
        start = module->data;
    }

    *ret = lys_find_grouping_up(name, (struct lys_node *)start);

    return EXIT_SUCCESS;
}

int
resolve_absolute_schema_nodeid(const char *nodeid, const struct lys_module *module, int ret_nodetype,
                               const struct lys_node **ret)
{
    const char *name, *mod_name, *id;
    const struct lys_node *sibling, *start_parent;
    int r, nam_len, mod_name_len, is_relative = -1;
    const struct lys_module *abs_start_mod;

    assert(nodeid && module && ret);
    assert(!(ret_nodetype & (LYS_USES | LYS_AUGMENT)) && ((ret_nodetype == LYS_GROUPING) || !(ret_nodetype & LYS_GROUPING)));

    id = nodeid;
    start_parent = NULL;

    if ((r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, NULL, NULL, 0)) < 1) {
        return ((id - nodeid) - r) + 1;
    }
    id += r;

    if (is_relative) {
        return -1;
    }

    abs_start_mod = lyp_get_module(module, NULL, 0, mod_name, mod_name_len, 0);
    if (!abs_start_mod) {
        return -1;
    }

    while (1) {
        sibling = NULL;
        while ((sibling = lys_getnext(sibling, start_parent, abs_start_mod, LYS_GETNEXT_WITHCHOICE
                | LYS_GETNEXT_WITHCASE | LYS_GETNEXT_WITHINOUT | LYS_GETNEXT_WITHGROUPING | LYS_GETNEXT_NOSTATECHECK))) {
            r = schema_nodeid_siblingcheck(sibling, module, mod_name, mod_name_len, name, nam_len);
            if (r == 0) {
                if (!id[0]) {
                    if (!(sibling->nodetype & ret_nodetype)) {
                        /* wrong node type, too bad */
                        continue;
                    }
                    *ret = sibling;
                    return EXIT_SUCCESS;
                }
                start_parent = sibling;
                break;
            } else if (r == 1) {
                continue;
            } else {
                return -1;
            }
        }

        /* no match */
        if (!sibling) {
            *ret = NULL;
            return EXIT_SUCCESS;
        }

        if ((r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, NULL, NULL, 0)) < 1) {
            return ((id - nodeid) - r) + 1;
        }
        id += r;
    }

    /* cannot get here */
    LOGINT(module->ctx);
    return -1;
}

static int
resolve_json_schema_list_predicate(const char *predicate, const struct lys_node_list *list, int *parsed)
{
    const char *mod_name, *name;
    int mod_name_len, nam_len, has_predicate, i;
    struct lys_node *key;

    if (((i = parse_schema_json_predicate(predicate, &mod_name, &mod_name_len, &name, &nam_len, NULL, NULL, &has_predicate)) < 1)
            || !strncmp(name, ".", nam_len)) {
        LOGVAL(list->module->ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, predicate[-i], &predicate[-i]);
        return -1;
    }

    predicate += i;
    *parsed += i;

    if (!isdigit(name[0])) {
        for (i = 0; i < list->keys_size; ++i) {
            key = (struct lys_node *)list->keys[i];
            if (!strncmp(key->name, name, nam_len) && !key->name[nam_len]) {
                break;
            }
        }

        if (i == list->keys_size) {
            LOGVAL(list->module->ctx, LYE_PATH_INKEY, LY_VLOG_NONE, NULL, name);
            return -1;
        }
    }

    /* more predicates? */
    if (has_predicate) {
        return resolve_json_schema_list_predicate(predicate, list, parsed);
    }

    return 0;
}

/* cannot return LYS_GROUPING, LYS_AUGMENT, LYS_USES, logs directly */
const struct lys_node *
resolve_json_nodeid(const char *nodeid, struct ly_ctx *ctx, const struct lys_node *start, int output)
{
    char *str;
    const char *name, *mod_name, *id, *backup_mod_name = NULL, *yang_data_name = NULL;
    const struct lys_node *sibling, *start_parent, *parent;
    int r, nam_len, mod_name_len, is_relative = -1, has_predicate;
    int yang_data_name_len, backup_mod_name_len;
    /* resolved import module from the start module, it must match the next node-name-match sibling */
    const struct lys_module *prefix_mod, *module, *prev_mod;

    assert(nodeid && (ctx || start));
    if (!ctx) {
        ctx = start->module->ctx;
    }

    id = nodeid;

    if ((r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, NULL, NULL, 1)) < 1) {
        LOGVAL(ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[-r], &id[-r]);
        return NULL;
    }

    if (name[0] == '#') {
        if (is_relative) {
            LOGVAL(ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, '#', name);
            return NULL;
        }
        yang_data_name = name + 1;
        yang_data_name_len = nam_len - 1;
        backup_mod_name = mod_name;
        backup_mod_name_len = mod_name_len;
        id += r;
    } else {
        is_relative = -1;
    }

    if ((r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, &has_predicate, NULL, 0)) < 1) {
        LOGVAL(ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[-r], &id[-r]);
        return NULL;
    }
    id += r;

    if (backup_mod_name) {
        mod_name = backup_mod_name;
        mod_name_len = backup_mod_name_len;
    }

    if (is_relative) {
        assert(start);
        start_parent = start;
        while (start_parent && (start_parent->nodetype == LYS_USES)) {
            start_parent = lys_parent(start_parent);
        }
        module = start->module;
    } else {
        if (!mod_name) {
            str = strndup(nodeid, (name + nam_len) - nodeid);
            LOGVAL(ctx, LYE_PATH_MISSMOD, LY_VLOG_STR, nodeid);
            free(str);
            return NULL;
        }

        str = strndup(mod_name, mod_name_len);
        module = ly_ctx_get_module(ctx, str, NULL, 1);
        free(str);

        if (!module) {
            str = strndup(nodeid, (mod_name + mod_name_len) - nodeid);
            LOGVAL(ctx, LYE_PATH_INMOD, LY_VLOG_STR, str);
            free(str);
            return NULL;
        }
        start_parent = NULL;
        if (yang_data_name) {
            start_parent = lyp_get_yang_data_template(module, yang_data_name, yang_data_name_len);
            if (!start_parent) {
                str = strndup(nodeid, (yang_data_name + yang_data_name_len) - nodeid);
                LOGVAL(ctx, LYE_PATH_INNODE, LY_VLOG_STR, str);
                free(str);
                return NULL;
            }
        }

        /* now it's as if there was no module name */
        mod_name = NULL;
        mod_name_len = 0;
    }

    prev_mod = module;

    while (1) {
        sibling = NULL;
        while ((sibling = lys_getnext(sibling, start_parent, module, 0))) {
            /* name match */
            if (sibling->name && !strncmp(name, sibling->name, nam_len) && !sibling->name[nam_len]) {
                /* output check */
                for (parent = lys_parent(sibling); parent && !(parent->nodetype & (LYS_INPUT | LYS_OUTPUT)); parent = lys_parent(parent));
                if (parent) {
                    if (output && (parent->nodetype == LYS_INPUT)) {
                        continue;
                    } else if (!output && (parent->nodetype == LYS_OUTPUT)) {
                        continue;
                    }
                }

                /* module check */
                if (mod_name) {
                    /* will also find an augment module */
                    prefix_mod = ly_ctx_nget_module(ctx, mod_name, mod_name_len, NULL, 1);

                    if (!prefix_mod) {
                        str = strndup(nodeid, (mod_name + mod_name_len) - nodeid);
                        LOGVAL(ctx, LYE_PATH_INMOD, LY_VLOG_STR, str);
                        free(str);
                        return NULL;
                    }
                } else {
                    prefix_mod = prev_mod;
                }
                if (prefix_mod != lys_node_module(sibling)) {
                    continue;
                }

                /* do we have some predicates on it? */
                if (has_predicate) {
                    r = 0;
                    if (sibling->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
                        if ((r = parse_schema_json_predicate(id, NULL, NULL, NULL, NULL, NULL, NULL, &has_predicate)) < 1) {
                            LOGVAL(ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[-r], &id[-r]);
                            return NULL;
                        }
                    } else if (sibling->nodetype == LYS_LIST) {
                        if (resolve_json_schema_list_predicate(id, (const struct lys_node_list *)sibling, &r)) {
                            return NULL;
                        }
                    } else {
                        LOGVAL(ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[0], id);
                        return NULL;
                    }
                    id += r;
                }

                /* the result node? */
                if (!id[0]) {
                    return sibling;
                }

                /* move down the tree, if possible */
                if (sibling->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) {
                    LOGVAL(ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[0], id);
                    return NULL;
                }
                start_parent = sibling;

                /* update prev mod */
                prev_mod = (start_parent->child ? lys_node_module(start_parent->child) : module);
                break;
            }
        }

        /* no match */
        if (!sibling) {
            str = strndup(nodeid, (name + nam_len) - nodeid);
            LOGVAL(ctx, LYE_PATH_INNODE, LY_VLOG_STR, str);
            free(str);
            return NULL;
        }

        if ((r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, &has_predicate, NULL, 0)) < 1) {
            LOGVAL(ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[-r], &id[-r]);
            return NULL;
        }
        id += r;
    }

    /* cannot get here */
    LOGINT(ctx);
    return NULL;
}

static int
resolve_partial_json_data_list_predicate(struct parsed_pred pp, struct lyd_node *node, int position)
{
    uint16_t i;
    struct lyd_node_leaf_list *key;
    struct lys_node_list *slist;
    struct ly_ctx *ctx;

    assert(node);
    assert(node->schema->nodetype == LYS_LIST);
    assert(pp.len);

    ctx = node->schema->module->ctx;
    slist = (struct lys_node_list *)node->schema;

    /* is the predicate a number? */
    if (isdigit(pp.pred[0].name[0])) {
        if (position == atoi(pp.pred[0].name)) {
            /* match */
            return 0;
        } else {
            /* not a match */
            return 1;
        }
    }

    key = (struct lyd_node_leaf_list *)node->child;
    if (!key) {
        /* it is not a position, so we need a key for it to be a match */
        return 1;
    }

    /* go through all the keys */
    for (i = 0; i < slist->keys_size; ++i) {
        if (strncmp(key->schema->name, pp.pred[i].name, pp.pred[i].nam_len) || key->schema->name[pp.pred[i].nam_len]) {
            LOGVAL(ctx, LYE_PATH_INKEY, LY_VLOG_NONE, NULL, pp.pred[i].name);
            return -1;
        }

        if (pp.pred[i].mod_name) {
            /* specific module, check that the found key is from that module */
            if (strncmp(lyd_node_module((struct lyd_node *)key)->name, pp.pred[i].mod_name, pp.pred[i].mod_name_len)
                    || lyd_node_module((struct lyd_node *)key)->name[pp.pred[i].mod_name_len]) {
                LOGVAL(ctx, LYE_PATH_INKEY, LY_VLOG_NONE, NULL, pp.pred[i].name);
                return -1;
            }

            /* but if the module is the same as the parent, it should have been omitted */
            if (lyd_node_module((struct lyd_node *)key) == lyd_node_module(node)) {
                LOGVAL(ctx, LYE_PATH_INKEY, LY_VLOG_NONE, NULL, pp.pred[i].name);
                return -1;
            }
        } else {
            /* no module, so it must be the same as the list (parent) */
            if (lyd_node_module((struct lyd_node *)key) != lyd_node_module(node)) {
                LOGVAL(ctx, LYE_PATH_INKEY, LY_VLOG_NONE, NULL, pp.pred[i].name);
                return -1;
            }
        }

        /* value does not match */
        if (strncmp(key->value_str, pp.pred[i].value, pp.pred[i].val_len) || key->value_str[pp.pred[i].val_len]) {
            return 1;
        }

        key = (struct lyd_node_leaf_list *)key->next;
    }

    return 0;
}

/**
 * @brief get the closest parent of the node (or the node itself) identified by the nodeid (path)
 *
 * @param[in] nodeid Node data path to find
 * @param[in] llist_value If the \p nodeid identifies leaf-list, this is expected value of the leaf-list instance.
 * @param[in] options Bitmask of options flags, see @ref pathoptions.
 * @param[out] parsed Number of characters processed in \p id
 * @return The closes parent (or the node itself) from the path
 */
struct lyd_node *
resolve_partial_json_data_nodeid(const char *nodeid, const char *llist_value, struct lyd_node *start, int options,
                                 int *parsed)
{
    const char *id, *mod_name, *name, *data_val, *llval;
    int r, ret, mod_name_len, nam_len, is_relative = -1, list_instance_position;
    int has_predicate, last_parsed = 0, llval_len;
    struct lyd_node *sibling, *last_match = NULL;
    struct lyd_node_leaf_list *llist;
    const struct lys_module *prev_mod;
    struct ly_ctx *ctx;
    const struct lys_node *ssibling, *sparent;
    struct lys_node_list *slist;
    struct parsed_pred pp;

    assert(nodeid && start && parsed);

    memset(&pp, 0, sizeof pp);
    ctx = start->schema->module->ctx;
    id = nodeid;

    /* parse first nodeid in case it is yang-data extension */
    if ((r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, NULL, NULL, 1)) < 1) {
        LOGVAL(ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[-r], &id[-r]);
        goto error;
    }

    if (name[0] == '#') {
        if (is_relative) {
            LOGVAL(ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, '#', name);
            goto error;
        }
        id += r;
        last_parsed = r;
    } else {
        is_relative = -1;
    }

    /* parse first nodeid */
    if ((r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, &has_predicate, NULL, 0)) < 1) {
        LOGVAL(ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[-r], &id[-r]);
        goto error;
    }
    id += r;
    /* add it to parsed only after the data node was actually found */
    last_parsed += r;

    if (is_relative) {
        prev_mod = lyd_node_module(start);
        start = start->child;
    } else {
        for (; start->parent; start = start->parent);
        prev_mod = lyd_node_module(start);
    }

    /* do not duplicate code, use predicate parsing from the loop */
    goto parse_predicates;

    while (1) {
        /* find the correct schema node first */
        ssibling = NULL;
        sparent = (start && start->parent) ? start->parent->schema : NULL;
        while ((ssibling = lys_getnext(ssibling, sparent, prev_mod, 0))) {
            /* skip invalid input/output nodes */
            if (sparent && (sparent->nodetype & (LYS_RPC | LYS_ACTION))) {
                if (options & LYD_PATH_OPT_OUTPUT) {
                    if (lys_parent(ssibling)->nodetype == LYS_INPUT) {
                        continue;
                    }
                } else {
                    if (lys_parent(ssibling)->nodetype == LYS_OUTPUT) {
                        continue;
                    }
                }
            }

            if (!schema_nodeid_siblingcheck(ssibling, prev_mod, mod_name, mod_name_len, name, nam_len)) {
                break;
            }
        }
        if (!ssibling) {
            /* there is not even such a schema node */
            free(pp.pred);
            return last_match;
        }
        pp.schema = ssibling;

        /* unify leaf-list value - it is possible to specify last-node value as both a predicate or parameter if
         * is a leaf-list, unify both cases and the value will in both cases be in the predicate structure */
        if (!id[0] && !pp.len && (ssibling->nodetype == LYS_LEAFLIST)) {
            pp.len = 1;
            pp.pred = calloc(1, sizeof *pp.pred);
            LY_CHECK_ERR_GOTO(!pp.pred, LOGMEM(ctx), error);

            pp.pred[0].name = ".";
            pp.pred[0].nam_len = 1;
            pp.pred[0].value = (llist_value ? llist_value : "");
            pp.pred[0].val_len = strlen(pp.pred[0].value);
        }

        if (ssibling->nodetype & (LYS_LEAFLIST | LYS_LEAF)) {
            /* check leaf/leaf-list predicate */
            if (pp.len > 1) {
                LOGVAL(ctx, LYE_PATH_PREDTOOMANY, LY_VLOG_NONE, NULL);
                goto error;
            } else if (pp.len) {
                if ((pp.pred[0].name[0] != '.') || (pp.pred[0].nam_len != 1)) {
                    LOGVAL(ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, pp.pred[0].name[0], pp.pred[0].name);
                    goto error;
                }
                if ((((struct lys_node_leaf *)ssibling)->type.base == LY_TYPE_IDENT) && !strnchr(pp.pred[0].value, ':', pp.pred[0].val_len)) {
                    LOGVAL(ctx, LYE_PATH_INIDENTREF, LY_VLOG_LYS, ssibling, pp.pred[0].val_len, pp.pred[0].value);
                    goto error;
                }
            }
        } else if (ssibling->nodetype == LYS_LIST) {
            /* list should have predicates for all the keys or position */
            slist = (struct lys_node_list *)ssibling;
            if (!pp.len) {
                /* none match */
                return last_match;
            } else if (!isdigit(pp.pred[0].name[0])) {
                /* list predicate is not a position, so there must be all the keys */
                if (pp.len > slist->keys_size) {
                    LOGVAL(ctx, LYE_PATH_PREDTOOMANY, LY_VLOG_NONE, NULL);
                    goto error;
                } else if (pp.len < slist->keys_size) {
                    LOGVAL(ctx, LYE_PATH_MISSKEY, LY_VLOG_NONE, NULL, slist->keys[pp.len]->name);
                    goto error;
                }
                /* check that all identityrefs have module name, otherwise the hash of the list instance will never match!! */
                for (r = 0; r < pp.len; ++r) {
                    if ((slist->keys[r]->type.base == LY_TYPE_IDENT) && !strnchr(pp.pred[r].value, ':', pp.pred[r].val_len)) {
                        LOGVAL(ctx, LYE_PATH_INIDENTREF, LY_VLOG_LYS, slist->keys[r], pp.pred[r].val_len, pp.pred[r].value);
                        goto error;
                    }
                }
            }
        } else if (pp.pred) {
            /* no other nodes allow predicates */
            LOGVAL(ctx, LYE_PATH_PREDTOOMANY, LY_VLOG_NONE, NULL);
            goto error;
        }

#ifdef LY_ENABLED_CACHE
        /* we will not be matching keyless lists or state leaf-lists this way */
        if (start->parent && start->parent->ht && ((pp.schema->nodetype != LYS_LIST) || ((struct lys_node_list *)pp.schema)->keys_size)
                && ((pp.schema->nodetype != LYS_LEAFLIST) || (pp.schema->flags & LYS_CONFIG_W))) {
            sibling = resolve_json_data_node_hash(start->parent, pp);
        } else
#endif
        {
            list_instance_position = 0;
            LY_TREE_FOR(start, sibling) {
                /* RPC/action data check, return simply invalid argument, because the data tree is invalid */
                if (lys_parent(sibling->schema)) {
                    if (options & LYD_PATH_OPT_OUTPUT) {
                        if (lys_parent(sibling->schema)->nodetype == LYS_INPUT) {
                            LOGERR(ctx, LY_EINVAL, "Provided data tree includes some RPC input nodes (%s).", sibling->schema->name);
                            goto error;
                        }
                    } else {
                        if (lys_parent(sibling->schema)->nodetype == LYS_OUTPUT) {
                            LOGERR(ctx, LY_EINVAL, "Provided data tree includes some RPC output nodes (%s).", sibling->schema->name);
                            goto error;
                        }
                    }
                }

                if (sibling->schema != ssibling) {
                    /* wrong schema node */
                    continue;
                }

                /* leaf-list, did we find it with the correct value or not? */
                if (ssibling->nodetype == LYS_LEAFLIST) {
                    if (ssibling->flags & LYS_CONFIG_R) {
                        /* state leaf-lists will never match */
                        continue;
                    }

                    llist = (struct lyd_node_leaf_list *)sibling;

                    /* get the expected leaf-list value */
                    llval = NULL;
                    llval_len = 0;
                    if (pp.pred) {
                        /* it was already checked that it is correct */
                        llval = pp.pred[0].value;
                        llval_len = pp.pred[0].val_len;

                    }

                    /* make value canonical (remove module name prefix) unless it was specified with it */
                    if (llval && !strchr(llval, ':') && (llist->value_type & LY_TYPE_IDENT)
                            && !strncmp(llist->value_str, lyd_node_module(sibling)->name, strlen(lyd_node_module(sibling)->name))
                            && (llist->value_str[strlen(lyd_node_module(sibling)->name)] == ':')) {
                        data_val = llist->value_str + strlen(lyd_node_module(sibling)->name) + 1;
                    } else {
                        data_val = llist->value_str;
                    }

                    if ((!llval && data_val && data_val[0]) || (llval && (strncmp(llval, data_val, llval_len)
                            || data_val[llval_len]))) {
                        continue;
                    }

                } else if (ssibling->nodetype == LYS_LIST) {
                    /* list, we likely need predicates'n'stuff then, but if without a predicate, we are always creating it */
                    ++list_instance_position;
                    ret = resolve_partial_json_data_list_predicate(pp, sibling, list_instance_position);
                    if (ret == -1) {
                        goto error;
                    } else if (ret == 1) {
                        /* this list instance does not match */
                        continue;
                    }
                }

                break;
            }
        }

        /* no match, return last match */
        if (!sibling) {
            free(pp.pred);
            return last_match;
        }

        /* we found a next matching node */
        *parsed += last_parsed;
        last_match = sibling;
        prev_mod = lyd_node_module(sibling);

        /* the result node? */
        if (!id[0]) {
            free(pp.pred);
            return last_match;
        }

        /* move down the tree, if possible, and continue */
        if (ssibling->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) {
            /* there can be no children even through expected, error */
            LOGVAL(ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[0], id);
            goto error;
        } else if (!sibling->child) {
            /* there could be some children, but are not, return what we found so far */
            free(pp.pred);
            return last_match;
        }
        start = sibling->child;

        /* parse nodeid */
        if ((r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, &has_predicate, NULL, 0)) < 1) {
            LOGVAL(ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[-r], &id[-r]);
            goto error;
        }
        id += r;
        last_parsed = r;

parse_predicates:
        /* parse all the predicates */
        free(pp.pred);
        pp.schema = NULL;
        pp.len = 0;
        pp.pred = NULL;
        while (has_predicate) {
            ++pp.len;
            pp.pred = ly_realloc(pp.pred, pp.len * sizeof *pp.pred);
            LY_CHECK_ERR_GOTO(!pp.pred, LOGMEM(ctx), error);
            if ((r = parse_schema_json_predicate(id, &pp.pred[pp.len - 1].mod_name, &pp.pred[pp.len - 1].mod_name_len,
                                                 &pp.pred[pp.len - 1].name, &pp.pred[pp.len - 1].nam_len, &pp.pred[pp.len - 1].value,
                                                 &pp.pred[pp.len - 1].val_len, &has_predicate)) < 1) {
                LOGVAL(ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[0], id);
                goto error;
            }

            id += r;
            last_parsed += r;
        }
    }

error:
    *parsed = -1;
    free(pp.pred);
    return NULL;
}

/**
 * @brief Resolves length or range intervals. Does not log.
 * Syntax is assumed to be correct, *ret MUST be NULL.
 *
 * @param[in] ctx Context for errors.
 * @param[in] str_restr Restriction as a string.
 * @param[in] type Type of the restriction.
 * @param[out] ret Final interval structure that starts with
 * the interval of the initial type, continues with intervals
 * of any superior types derived from the initial one, and
 * finishes with intervals from our \p type.
 *
 * @return EXIT_SUCCESS on succes, -1 on error.
 */
int
resolve_len_ran_interval(struct ly_ctx *ctx, const char *str_restr, struct lys_type *type, struct len_ran_intv **ret)
{
    /* 0 - unsigned, 1 - signed, 2 - floating point */
    int kind;
    int64_t local_smin = 0, local_smax = 0, local_fmin, local_fmax;
    uint64_t local_umin, local_umax = 0;
    uint8_t local_fdig = 0;
    const char *seg_ptr, *ptr;
    struct len_ran_intv *local_intv = NULL, *tmp_local_intv = NULL, *tmp_intv, *intv = NULL;

    switch (type->base) {
    case LY_TYPE_BINARY:
        kind = 0;
        local_umin = 0;
        local_umax = 18446744073709551615UL;

        if (!str_restr && type->info.binary.length) {
            str_restr = type->info.binary.length->expr;
        }
        break;
    case LY_TYPE_DEC64:
        kind = 2;
        local_fmin = __INT64_C(-9223372036854775807) - __INT64_C(1);
        local_fmax = __INT64_C(9223372036854775807);
        local_fdig = type->info.dec64.dig;

        if (!str_restr && type->info.dec64.range) {
            str_restr = type->info.dec64.range->expr;
        }
        break;
    case LY_TYPE_INT8:
        kind = 1;
        local_smin = __INT64_C(-128);
        local_smax = __INT64_C(127);

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_INT16:
        kind = 1;
        local_smin = __INT64_C(-32768);
        local_smax = __INT64_C(32767);

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_INT32:
        kind = 1;
        local_smin = __INT64_C(-2147483648);
        local_smax = __INT64_C(2147483647);

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_INT64:
        kind = 1;
        local_smin = __INT64_C(-9223372036854775807) - __INT64_C(1);
        local_smax = __INT64_C(9223372036854775807);

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_UINT8:
        kind = 0;
        local_umin = __UINT64_C(0);
        local_umax = __UINT64_C(255);

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_UINT16:
        kind = 0;
        local_umin = __UINT64_C(0);
        local_umax = __UINT64_C(65535);

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_UINT32:
        kind = 0;
        local_umin = __UINT64_C(0);
        local_umax = __UINT64_C(4294967295);

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_UINT64:
        kind = 0;
        local_umin = __UINT64_C(0);
        local_umax = __UINT64_C(18446744073709551615);

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_STRING:
        kind = 0;
        local_umin = __UINT64_C(0);
        local_umax = __UINT64_C(18446744073709551615);

        if (!str_restr && type->info.str.length) {
            str_restr = type->info.str.length->expr;
        }
        break;
    default:
        return -1;
    }

    /* process superior types */
    if (type->der) {
        if (resolve_len_ran_interval(ctx, NULL, &type->der->type, &intv)) {
            return -1;
        }
        assert(!intv || (intv->kind == kind));
    }

    if (!str_restr) {
        /* we do not have any restriction, return superior ones */
        *ret = intv;
        return EXIT_SUCCESS;
    }

    /* adjust local min and max */
    if (intv) {
        tmp_intv = intv;

        if (kind == 0) {
            local_umin = tmp_intv->value.uval.min;
        } else if (kind == 1) {
            local_smin = tmp_intv->value.sval.min;
        } else if (kind == 2) {
            local_fmin = tmp_intv->value.fval.min;
        }

        while (tmp_intv->next) {
            tmp_intv = tmp_intv->next;
        }

        if (kind == 0) {
            local_umax = tmp_intv->value.uval.max;
        } else if (kind == 1) {
            local_smax = tmp_intv->value.sval.max;
        } else if (kind == 2) {
            local_fmax = tmp_intv->value.fval.max;
        }
    }

    /* finally parse our restriction */
    seg_ptr = str_restr;
    tmp_intv = NULL;
    while (1) {
        if (!tmp_local_intv) {
            assert(!local_intv);
            local_intv = malloc(sizeof *local_intv);
            tmp_local_intv = local_intv;
        } else {
            tmp_local_intv->next = malloc(sizeof *tmp_local_intv);
            tmp_local_intv = tmp_local_intv->next;
        }
        LY_CHECK_ERR_GOTO(!tmp_local_intv, LOGMEM(ctx), error);

        tmp_local_intv->kind = kind;
        tmp_local_intv->type = type;
        tmp_local_intv->next = NULL;

        /* min */
        ptr = seg_ptr;
        while (isspace(ptr[0])) {
            ++ptr;
        }
        if (isdigit(ptr[0]) || (ptr[0] == '+') || (ptr[0] == '-')) {
            if (kind == 0) {
                tmp_local_intv->value.uval.min = strtoll(ptr, (char **)&ptr, 10);
            } else if (kind == 1) {
                tmp_local_intv->value.sval.min = strtoll(ptr, (char **)&ptr, 10);
            } else if (kind == 2) {
                if (parse_range_dec64(&ptr, local_fdig, &tmp_local_intv->value.fval.min)) {
                    LOGVAL(ctx, LYE_INARG, LY_VLOG_NONE, NULL, ptr, "range");
                    goto error;
                }
            }
        } else if (!strncmp(ptr, "min", 3)) {
            if (kind == 0) {
                tmp_local_intv->value.uval.min = local_umin;
            } else if (kind == 1) {
                tmp_local_intv->value.sval.min = local_smin;
            } else if (kind == 2) {
                tmp_local_intv->value.fval.min = local_fmin;
            }

            ptr += 3;
        } else if (!strncmp(ptr, "max", 3)) {
            if (kind == 0) {
                tmp_local_intv->value.uval.min = local_umax;
            } else if (kind == 1) {
                tmp_local_intv->value.sval.min = local_smax;
            } else if (kind == 2) {
                tmp_local_intv->value.fval.min = local_fmax;
            }

            ptr += 3;
        } else {
            goto error;
        }

        while (isspace(ptr[0])) {
            ptr++;
        }

        /* no interval or interval */
        if ((ptr[0] == '|') || !ptr[0]) {
            if (kind == 0) {
                tmp_local_intv->value.uval.max = tmp_local_intv->value.uval.min;
            } else if (kind == 1) {
                tmp_local_intv->value.sval.max = tmp_local_intv->value.sval.min;
            } else if (kind == 2) {
                tmp_local_intv->value.fval.max = tmp_local_intv->value.fval.min;
            }
        } else if (!strncmp(ptr, "..", 2)) {
            /* skip ".." */
            ptr += 2;
            while (isspace(ptr[0])) {
                ++ptr;
            }

            /* max */
            if (isdigit(ptr[0]) || (ptr[0] == '+') || (ptr[0] == '-')) {
                if (kind == 0) {
                    tmp_local_intv->value.uval.max = strtoll(ptr, (char **)&ptr, 10);
                } else if (kind == 1) {
                    tmp_local_intv->value.sval.max = strtoll(ptr, (char **)&ptr, 10);
                } else if (kind == 2) {
                    if (parse_range_dec64(&ptr, local_fdig, &tmp_local_intv->value.fval.max)) {
                        LOGVAL(ctx, LYE_INARG, LY_VLOG_NONE, NULL, ptr, "range");
                        goto error;
                    }
                }
            } else if (!strncmp(ptr, "max", 3)) {
                if (kind == 0) {
                    tmp_local_intv->value.uval.max = local_umax;
                } else if (kind == 1) {
                    tmp_local_intv->value.sval.max = local_smax;
                } else if (kind == 2) {
                    tmp_local_intv->value.fval.max = local_fmax;
                }
            } else {
                goto error;
            }
        } else {
            goto error;
        }

        /* check min and max in correct order*/
        if (kind == 0) {
            /* current segment */
            if (tmp_local_intv->value.uval.min > tmp_local_intv->value.uval.max) {
                goto error;
            }
            if (tmp_local_intv->value.uval.min < local_umin || tmp_local_intv->value.uval.max > local_umax) {
                goto error;
            }
            /* segments sholud be ascending order */
            if (tmp_intv && (tmp_intv->value.uval.max >= tmp_local_intv->value.uval.min)) {
                goto error;
            }
        } else if (kind == 1) {
            if (tmp_local_intv->value.sval.min > tmp_local_intv->value.sval.max) {
                goto error;
            }
            if (tmp_local_intv->value.sval.min < local_smin || tmp_local_intv->value.sval.max > local_smax) {
                goto error;
            }
            if (tmp_intv && (tmp_intv->value.sval.max >= tmp_local_intv->value.sval.min)) {
                goto error;
            }
        } else if (kind == 2) {
            if (tmp_local_intv->value.fval.min > tmp_local_intv->value.fval.max) {
                goto error;
            }
            if (tmp_local_intv->value.fval.min < local_fmin || tmp_local_intv->value.fval.max > local_fmax) {
                goto error;
            }
            if (tmp_intv && (tmp_intv->value.fval.max >= tmp_local_intv->value.fval.min)) {
                /* fraction-digits value is always the same (it cannot be changed in derived types) */
                goto error;
            }
        }

        /* next segment (next OR) */
        seg_ptr = strchr(seg_ptr, '|');
        if (!seg_ptr) {
            break;
        }
        seg_ptr++;
        tmp_intv = tmp_local_intv;
    }

    /* check local restrictions against superior ones */
    if (intv) {
        tmp_intv = intv;
        tmp_local_intv = local_intv;

        while (tmp_local_intv && tmp_intv) {
            /* reuse local variables */
            if (kind == 0) {
                local_umin = tmp_local_intv->value.uval.min;
                local_umax = tmp_local_intv->value.uval.max;

                /* it must be in this interval */
                if ((local_umin >= tmp_intv->value.uval.min) && (local_umin <= tmp_intv->value.uval.max)) {
                    /* this interval is covered, next one */
                    if (local_umax <= tmp_intv->value.uval.max) {
                        tmp_local_intv = tmp_local_intv->next;
                        continue;
                    /* ascending order of restrictions -> fail */
                    } else {
                        goto error;
                    }
                }
            } else if (kind == 1) {
                local_smin = tmp_local_intv->value.sval.min;
                local_smax = tmp_local_intv->value.sval.max;

                if ((local_smin >= tmp_intv->value.sval.min) && (local_smin <= tmp_intv->value.sval.max)) {
                    if (local_smax <= tmp_intv->value.sval.max) {
                        tmp_local_intv = tmp_local_intv->next;
                        continue;
                    } else {
                        goto error;
                    }
                }
            } else if (kind == 2) {
                local_fmin = tmp_local_intv->value.fval.min;
                local_fmax = tmp_local_intv->value.fval.max;

                 if ((dec64cmp(local_fmin, local_fdig, tmp_intv->value.fval.min, local_fdig) > -1)
                        && (dec64cmp(local_fmin, local_fdig, tmp_intv->value.fval.max, local_fdig) < 1)) {
                    if (dec64cmp(local_fmax, local_fdig, tmp_intv->value.fval.max, local_fdig) < 1) {
                        tmp_local_intv = tmp_local_intv->next;
                        continue;
                    } else {
                        goto error;
                    }
                }
            }

            tmp_intv = tmp_intv->next;
        }

        /* some interval left uncovered -> fail */
        if (tmp_local_intv) {
            goto error;
        }
    }

    /* append the local intervals to all the intervals of the superior types, return it all */
    if (intv) {
        for (tmp_intv = intv; tmp_intv->next; tmp_intv = tmp_intv->next);
        tmp_intv->next = local_intv;
    } else {
        intv = local_intv;
    }
    *ret = intv;

    return EXIT_SUCCESS;

error:
    while (intv) {
        tmp_intv = intv->next;
        free(intv);
        intv = tmp_intv;
    }
    while (local_intv) {
        tmp_local_intv = local_intv->next;
        free(local_intv);
        local_intv = tmp_local_intv;
    }

    return -1;
}

/**
 * @brief Resolve a typedef, return only resolved typedefs if derived. If leafref, it must be
 * resolved for this function to return it. Does not log.
 *
 * @param[in] name Typedef name.
 * @param[in] mod_name Typedef name module name.
 * @param[in] module Main module.
 * @param[in] parent Parent of the resolved type definition.
 * @param[out] ret Pointer to the resolved typedef. Can be NULL.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
int
resolve_superior_type(const char *name, const char *mod_name, const struct lys_module *module,
                      const struct lys_node *parent, struct lys_tpdf **ret)
{
    int i, j;
    struct lys_tpdf *tpdf, *match;
    int tpdf_size;

    if (!mod_name) {
        /* no prefix, try built-in types */
        for (i = 1; i < LY_DATA_TYPE_COUNT; i++) {
            if (!strcmp(ly_types[i]->name, name)) {
                if (ret) {
                    *ret = ly_types[i];
                }
                return EXIT_SUCCESS;
            }
        }
    } else {
        if (!strcmp(mod_name, module->name)) {
            /* prefix refers to the current module, ignore it */
            mod_name = NULL;
        }
    }

    if (!mod_name && parent) {
        /* search in local typedefs */
        while (parent) {
            switch (parent->nodetype) {
            case LYS_CONTAINER:
                tpdf_size = ((struct lys_node_container *)parent)->tpdf_size;
                tpdf = ((struct lys_node_container *)parent)->tpdf;
                break;

            case LYS_LIST:
                tpdf_size = ((struct lys_node_list *)parent)->tpdf_size;
                tpdf = ((struct lys_node_list *)parent)->tpdf;
                break;

            case LYS_GROUPING:
                tpdf_size = ((struct lys_node_grp *)parent)->tpdf_size;
                tpdf = ((struct lys_node_grp *)parent)->tpdf;
                break;

            case LYS_RPC:
            case LYS_ACTION:
                tpdf_size = ((struct lys_node_rpc_action *)parent)->tpdf_size;
                tpdf = ((struct lys_node_rpc_action *)parent)->tpdf;
                break;

            case LYS_NOTIF:
                tpdf_size = ((struct lys_node_notif *)parent)->tpdf_size;
                tpdf = ((struct lys_node_notif *)parent)->tpdf;
                break;

            case LYS_INPUT:
            case LYS_OUTPUT:
                tpdf_size = ((struct lys_node_inout *)parent)->tpdf_size;
                tpdf = ((struct lys_node_inout *)parent)->tpdf;
                break;

            default:
                parent = lys_parent(parent);
                continue;
            }

            for (i = 0; i < tpdf_size; i++) {
                if (!strcmp(tpdf[i].name, name) && tpdf[i].type.base > 0) {
                    match = &tpdf[i];
                    goto check_leafref;
                }
            }

            parent = lys_parent(parent);
        }
    } else {
        /* get module where to search */
        module = lyp_get_module(module, NULL, 0, mod_name, 0, 0);
        if (!module) {
            return -1;
        }
    }

    /* search in top level typedefs */
    for (i = 0; i < module->tpdf_size; i++) {
        if (!strcmp(module->tpdf[i].name, name) && module->tpdf[i].type.base > 0) {
            match = &module->tpdf[i];
            goto check_leafref;
        }
    }

    /* search in submodules */
    for (i = 0; i < module->inc_size && module->inc[i].submodule; i++) {
        for (j = 0; j < module->inc[i].submodule->tpdf_size; j++) {
            if (!strcmp(module->inc[i].submodule->tpdf[j].name, name) && module->inc[i].submodule->tpdf[j].type.base > 0) {
                match = &module->inc[i].submodule->tpdf[j];
                goto check_leafref;
            }
        }
    }

    return EXIT_FAILURE;

check_leafref:
    if (ret) {
        *ret = match;
    }
    if (match->type.base == LY_TYPE_LEAFREF) {
        while (!match->type.info.lref.path) {
            match = match->type.der;
            assert(match);
        }
    }
    return EXIT_SUCCESS;
}

/**
 * @brief Check the default \p value of the \p type. Logs directly.
 *
 * @param[in] type Type definition to use.
 * @param[in] value Default value to check.
 * @param[in] module Type module.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
check_default(struct lys_type *type, const char **value, struct lys_module *module, int tpdf)
{
    struct lys_tpdf *base_tpdf = NULL;
    struct lyd_node_leaf_list node;
    const char *dflt = NULL;
    char *s;
    int ret = EXIT_SUCCESS, r;
    struct ly_ctx *ctx = module->ctx;

    assert(value);
    memset(&node, 0, sizeof node);

    if (type->base <= LY_TYPE_DER) {
        /* the type was not resolved yet, nothing to do for now */
        ret = EXIT_FAILURE;
        goto cleanup;
    } else if (!tpdf && !module->implemented) {
        /* do not check defaults in not implemented module's data */
        goto cleanup;
    } else if (tpdf && !module->implemented && type->base == LY_TYPE_IDENT) {
        /* identityrefs are checked when instantiated in data instead of typedef,
         * but in typedef the value has to be modified to include the prefix */
        if (*value) {
            if (strchr(*value, ':')) {
                dflt = transform_schema2json(module, *value);
            } else {
                /* default prefix of the module where the typedef is defined */
                if (asprintf(&s, "%s:%s", lys_main_module(module)->name, *value) == -1) {
                    LOGMEM(ctx);
                    ret = -1;
                    goto cleanup;
                }
                dflt = lydict_insert_zc(ctx, s);
            }
            lydict_remove(ctx, *value);
            *value = dflt;
            dflt = NULL;
        }
        goto cleanup;
    } else if (type->base == LY_TYPE_LEAFREF && tpdf) {
        /* leafref in typedef cannot be checked */
        goto cleanup;
    }

    dflt = lydict_insert(ctx, *value, 0);
    if (!dflt) {
        /* we do not have a new default value, so is there any to check even, in some base type? */
        for (base_tpdf = type->der; base_tpdf->type.der; base_tpdf = base_tpdf->type.der) {
            if (base_tpdf->dflt) {
                dflt = lydict_insert(ctx, base_tpdf->dflt, 0);
                break;
            }
        }

        if (!dflt) {
            /* no default value, nothing to check, all is well */
            goto cleanup;
        }

        /* so there is a default value in a base type, but can the default value be no longer valid (did we define some new restrictions)? */
        switch (type->base) {
        case LY_TYPE_IDENT:
            if (lys_main_module(base_tpdf->type.parent->module)->implemented) {
                goto cleanup;
            } else {
                /* check the default value from typedef, but use also the typedef's module
                 * due to possible searching in imported modules which is expected in
                 * typedef's module instead of module where the typedef is used */
                module = base_tpdf->module;
            }
            break;
        case LY_TYPE_INST:
        case LY_TYPE_LEAFREF:
        case LY_TYPE_BOOL:
        case LY_TYPE_EMPTY:
            /* these have no restrictions, so we would do the exact same work as the unres in the base typedef */
            goto cleanup;
        case LY_TYPE_BITS:
            /* the default value must match the restricted list of values, if the type was restricted */
            if (type->info.bits.count) {
                break;
            }
            goto cleanup;
        case LY_TYPE_ENUM:
            /* the default value must match the restricted list of values, if the type was restricted */
            if (type->info.enums.count) {
                break;
            }
            goto cleanup;
        case LY_TYPE_DEC64:
            if (type->info.dec64.range) {
                break;
            }
            goto cleanup;
        case LY_TYPE_BINARY:
            if (type->info.binary.length) {
                break;
            }
            goto cleanup;
        case LY_TYPE_INT8:
        case LY_TYPE_INT16:
        case LY_TYPE_INT32:
        case LY_TYPE_INT64:
        case LY_TYPE_UINT8:
        case LY_TYPE_UINT16:
        case LY_TYPE_UINT32:
        case LY_TYPE_UINT64:
            if (type->info.num.range) {
                break;
            }
            goto cleanup;
        case LY_TYPE_STRING:
            if (type->info.str.length || type->info.str.patterns) {
                break;
            }
            goto cleanup;
        case LY_TYPE_UNION:
            /* way too much trouble learning whether we need to check the default again, so just do it */
            break;
        default:
            LOGINT(ctx);
            ret = -1;
            goto cleanup;
        }
    } else if (type->base == LY_TYPE_EMPTY) {
        LOGVAL(ctx, LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, "default", type->parent->name);
        LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "The \"empty\" data type cannot have a default value.");
        ret = -1;
        goto cleanup;
    }

    /* dummy leaf */
    memset(&node, 0, sizeof node);
    node.value_str = lydict_insert(ctx, dflt, 0);
    node.value_type = type->base;

    if (tpdf) {
        node.schema = calloc(1, sizeof (struct lys_node_leaf));
        if (!node.schema) {
            LOGMEM(ctx);
            ret = -1;
            goto cleanup;
        }
        r = asprintf((char **)&node.schema->name, "typedef-%s-default", ((struct lys_tpdf *)type->parent)->name);
        if (r == -1) {
            LOGMEM(ctx);
            ret = -1;
            goto cleanup;
        }
        node.schema->module = module;
        memcpy(&((struct lys_node_leaf *)node.schema)->type, type, sizeof *type);
    } else {
        node.schema = (struct lys_node *)type->parent;
    }

    if (type->base == LY_TYPE_LEAFREF) {
        if (!type->info.lref.target) {
            ret = EXIT_FAILURE;
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Default value \"%s\" cannot be checked in an unresolved leafref.",
                   dflt);
            goto cleanup;
        }
        ret = check_default(&type->info.lref.target->type, &dflt, module, 0);
        if (!ret) {
            /* adopt possibly changed default value to its canonical form */
            if (*value) {
                lydict_remove(ctx, *value);
                *value = dflt;
                dflt = NULL;
            }
        }
    } else {
        if (!lyp_parse_value(type, &node.value_str, NULL, &node, NULL, module, 1, 1, 0)) {
            /* possible forward reference */
            ret = EXIT_FAILURE;
            if (base_tpdf) {
                /* default value is defined in some base typedef */
                if ((type->base == LY_TYPE_BITS && type->der->type.der) ||
                        (type->base == LY_TYPE_ENUM && type->der->type.der)) {
                    /* we have refined bits/enums */
                    LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL,
                           "Invalid value \"%s\" of the default statement inherited to \"%s\" from \"%s\" base type.",
                           dflt, type->parent->name, base_tpdf->name);
                }
            }
        } else {
            /* success - adopt canonical form from the node into the default value */
            if (!ly_strequal(dflt, node.value_str, 1)) {
                /* this can happen only if we have non-inherited default value,
                 * inherited default values are already in canonical form */
                assert(ly_strequal(dflt, *value, 1));

                lydict_remove(ctx, *value);
                *value = node.value_str;
                node.value_str = NULL;
            }
        }
    }

cleanup:
    lyd_free_value(node.value, node.value_type, node.value_flags, type, NULL, NULL, NULL);
    lydict_remove(ctx, node.value_str);
    if (tpdf && node.schema) {
        free((char *)node.schema->name);
        free(node.schema);
    }
    lydict_remove(ctx, dflt);

    return ret;
}

/**
 * @brief Check a key for mandatory attributes. Logs directly.
 *
 * @param[in] key The key to check.
 * @param[in] flags What flags to check.
 * @param[in] list The list of all the keys.
 * @param[in] index Index of the key in the key list.
 * @param[in] name The name of the keys.
 * @param[in] len The name length.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
check_key(struct lys_node_list *list, int index, const char *name, int len)
{
    struct lys_node_leaf *key = list->keys[index];
    char *dup = NULL;
    int j;
    struct ly_ctx *ctx = list->module->ctx;

    /* existence */
    if (!key) {
        if (name[len] != '\0') {
            dup = strdup(name);
            LY_CHECK_ERR_RETURN(!dup, LOGMEM(ctx), -1);
            dup[len] = '\0';
            name = dup;
        }
        LOGVAL(ctx, LYE_KEY_MISS, LY_VLOG_LYS, list, name);
        free(dup);
        return -1;
    }

    /* uniqueness */
    for (j = index - 1; j >= 0; j--) {
        if (key == list->keys[j]) {
            LOGVAL(ctx, LYE_KEY_DUP, LY_VLOG_LYS, list, key->name);
            return -1;
        }
    }

    /* key is a leaf */
    if (key->nodetype != LYS_LEAF) {
        LOGVAL(ctx, LYE_KEY_NLEAF, LY_VLOG_LYS, list, key->name);
        return -1;
    }

    /* type of the leaf is not built-in empty */
    if (key->type.base == LY_TYPE_EMPTY && key->module->version < LYS_VERSION_1_1) {
        LOGVAL(ctx, LYE_KEY_TYPE, LY_VLOG_LYS, list, key->name);
        return -1;
    }

    /* config attribute is the same as of the list */
    if ((key->flags & LYS_CONFIG_MASK) && (list->flags & LYS_CONFIG_MASK)
            && ((list->flags & LYS_CONFIG_MASK) != (key->flags & LYS_CONFIG_MASK))) {
        LOGVAL(ctx, LYE_KEY_CONFIG, LY_VLOG_LYS, list, key->name);
        return -1;
    }

    /* key is not placed from augment */
    if (key->parent->nodetype == LYS_AUGMENT) {
        LOGVAL(ctx, LYE_KEY_MISS, LY_VLOG_LYS, key, key->name);
        LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL, "Key inserted from augment.");
        return -1;
    }

    /* key is not when/if-feature -conditional */
    j = 0;
    if (key->when || (key->iffeature_size && (j = 1))) {
        LOGVAL(ctx, LYE_INCHILDSTMT, LY_VLOG_LYS, key, j ? "if-feature" : "when", "leaf");
        LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL, "Key definition cannot depend on a \"%s\" condition.",
               j ? "if-feature" : "when");
        return -1;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Resolve (test the target exists) unique. Logs directly.
 *
 * @param[in] parent The parent node of the unique structure.
 * @param[in] uniq_str_path One path from the unique string.
 *
 * @return EXIT_SUCCESS on succes, EXIT_FAILURE on forward reference, -1 on error.
 */
int
resolve_unique(struct lys_node *parent, const char *uniq_str_path, uint8_t *trg_type)
{
    int rc;
    const struct lys_node *leaf = NULL;
    struct ly_ctx *ctx = parent->module->ctx;

    rc = resolve_descendant_schema_nodeid(uniq_str_path, *lys_child(parent, LYS_LEAF), LYS_LEAF, 1, &leaf);
    if (rc || !leaf) {
        if (rc) {
            LOGVAL(ctx, LYE_INARG, LY_VLOG_LYS, parent, uniq_str_path, "unique");
            if (rc > 0) {
                LOGVAL(ctx, LYE_INCHAR, LY_VLOG_PREV, NULL, uniq_str_path[rc - 1], &uniq_str_path[rc - 1]);
            } else if (rc == -2) {
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL, "Unique argument references list.");
            }
            rc = -1;
        } else {
            LOGVAL(ctx, LYE_INARG, LY_VLOG_LYS, parent, uniq_str_path, "unique");
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL, "Target leaf not found.");
            rc = EXIT_FAILURE;
        }
        goto error;
    }
    if (leaf->nodetype != LYS_LEAF) {
        LOGVAL(ctx, LYE_INARG, LY_VLOG_LYS, parent, uniq_str_path, "unique");
        LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL, "Target is not a leaf.");
        return -1;
    }

    /* check status */
    if (parent->nodetype != LYS_EXT && lyp_check_status(parent->flags, parent->module, parent->name,
                                                        leaf->flags, leaf->module, leaf->name, leaf)) {
        return -1;
    }

    /* check that all unique's targets are of the same config type */
    if (*trg_type) {
        if (((*trg_type == 1) && (leaf->flags & LYS_CONFIG_R)) || ((*trg_type == 2) && (leaf->flags & LYS_CONFIG_W))) {
            LOGVAL(ctx, LYE_INARG, LY_VLOG_LYS, parent, uniq_str_path, "unique");
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL,
                   "Leaf \"%s\" referenced in unique statement is config %s, but previous referenced leaf is config %s.",
                   uniq_str_path, *trg_type == 1 ? "false" : "true", *trg_type == 1 ? "true" : "false");
            return -1;
        }
    } else {
        /* first unique */
        if (leaf->flags & LYS_CONFIG_W) {
            *trg_type = 1;
        } else {
            *trg_type = 2;
        }
    }

    /* set leaf's unique flag */
    ((struct lys_node_leaf *)leaf)->flags |= LYS_UNIQUE;

    return EXIT_SUCCESS;

error:

    return rc;
}

void
unres_data_del(struct unres_data *unres, uint32_t i)
{
    /* there are items after the one deleted */
    if (i+1 < unres->count) {
        /* we only move the data, memory is left allocated, why bother */
        memmove(&unres->node[i], &unres->node[i+1], (unres->count-(i+1)) * sizeof *unres->node);

    /* deleting the last item */
    } else if (i == 0) {
        free(unres->node);
        unres->node = NULL;
    }

    /* if there are no items after and it is not the last one, just move the counter */
    --unres->count;
}

/**
 * @brief Resolve (find) a data node from a specific module. Does not log.
 *
 * @param[in] mod Module to search in.
 * @param[in] name Name of the data node.
 * @param[in] nam_len Length of the name.
 * @param[in] start Data node to start the search from.
 * @param[in,out] parents Resolved nodes. If there are some parents,
 *                        they are replaced (!!) with the resolvents.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
resolve_data(const struct lys_module *mod, const char *name, int nam_len, struct lyd_node *start, struct unres_data *parents)
{
    struct lyd_node *node;
    int flag;
    uint32_t i;

    if (!parents->count) {
        parents->count = 1;
        parents->node = malloc(sizeof *parents->node);
        LY_CHECK_ERR_RETURN(!parents->node, LOGMEM(mod->ctx), -1);
        parents->node[0] = NULL;
    }
    for (i = 0; i < parents->count;) {
        if (parents->node[i] && (parents->node[i]->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA))) {
            /* skip */
            ++i;
            continue;
        }
        flag = 0;
        LY_TREE_FOR(parents->node[i] ? parents->node[i]->child : start, node) {
            if (lyd_node_module(node) == mod && !strncmp(node->schema->name, name, nam_len)
                    && node->schema->name[nam_len] == '\0') {
                /* matching target */
                if (!flag) {
                    /* put node instead of the current parent */
                    parents->node[i] = node;
                    flag = 1;
                } else {
                    /* multiple matching, so create a new node */
                    ++parents->count;
                    parents->node = ly_realloc(parents->node, parents->count * sizeof *parents->node);
                    LY_CHECK_ERR_RETURN(!parents->node, LOGMEM(mod->ctx), EXIT_FAILURE);
                    parents->node[parents->count-1] = node;
                    ++i;
                }
            }
        }

        if (!flag) {
            /* remove item from the parents list */
            unres_data_del(parents, i);
        } else {
            ++i;
        }
    }

    return parents->count ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int
resolve_schema_leafref_valid_dep_flag(const struct lys_node *op_node, const struct lys_module *local_mod,
                                      const struct lys_node *first_node, int abs_path)
{
    int dep1, dep2;
    const struct lys_node *node;

    if (!op_node) {
        /* leafref pointing to a different module */
        if (local_mod != lys_node_module(first_node)) {
            return 1;
        }
    } else if (lys_parent(op_node)) {
        /* inner operation (notif/action) */
        if (abs_path) {
            return 1;
        } else {
            /* compare depth of both nodes */
            for (dep1 = 0, node = op_node; lys_parent(node); node = lys_parent(node));
            for (dep2 = 0, node = first_node; lys_parent(node); node = lys_parent(node));
            if ((dep2 > dep1) || ((dep2 == dep1) && (op_node != first_node))) {
                return 1;
            }
        }
    } else {
        /* top-level operation (notif/rpc) */
        if (op_node != first_node) {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief Resolve a path (leafref) predicate in JSON schema context. Logs directly.
 *
 * @param[in] path Path to use.
 * @param[in] context_node Predicate context node (where the predicate is placed).
 * @param[in] parent Path context node (where the path begins/is placed).
 * @param[in] op_node Optional node if the leafref is in an operation (action/rpc/notif).
 *
 * @return 0 on forward reference, otherwise the number
 *         of characters successfully parsed,
 *         positive on success, negative on failure.
 */
static int
resolve_schema_leafref_predicate(const char *path, const struct lys_node *context_node, struct lys_node *parent)
{
    const struct lys_module *trg_mod;
    const struct lys_node *src_node, *dst_node;
    const char *path_key_expr, *source, *sour_pref, *dest, *dest_pref;
    int pke_len, sour_len, sour_pref_len, dest_len, dest_pref_len, pke_parsed, parsed = 0;
    int has_predicate, dest_parent_times, i, rc;
    struct ly_ctx *ctx = context_node->module->ctx;

    do {
        if ((i = parse_path_predicate(path, &sour_pref, &sour_pref_len, &source, &sour_len, &path_key_expr,
                                      &pke_len, &has_predicate)) < 1) {
            LOGVAL(ctx, LYE_INCHAR, LY_VLOG_LYS, parent, path[-i], path-i);
            return -parsed+i;
        }
        parsed += i;
        path += i;

        /* source (must be leaf) */
        if (sour_pref) {
            trg_mod = lyp_get_module(lys_node_module(parent), NULL, 0, sour_pref, sour_pref_len, 0);
        } else {
            trg_mod = lys_node_module(parent);
        }
        rc = lys_getnext_data(trg_mod, context_node, source, sour_len, LYS_LEAF | LYS_LEAFLIST, &src_node);
        if (rc) {
            LOGVAL(ctx, LYE_NORESOLV, LY_VLOG_LYS, parent, "leafref predicate", path-parsed);
            return 0;
        }

        /* destination */
        dest_parent_times = 0;
        pke_parsed = 0;
        if ((i = parse_path_key_expr(path_key_expr, &dest_pref, &dest_pref_len, &dest, &dest_len,
                                     &dest_parent_times)) < 1) {
            LOGVAL(ctx, LYE_INCHAR, LY_VLOG_LYS, parent, path_key_expr[-i], path_key_expr-i);
            return -parsed;
        }
        pke_parsed += i;

        for (i = 0, dst_node = parent; i < dest_parent_times; ++i) {
            if (!dst_node) {
                /* we went too much into parents, there is no parent anymore */
                LOGVAL(ctx, LYE_NORESOLV, LY_VLOG_LYS, parent, "leafref predicate", path_key_expr);
                return 0;
            }

            if (dst_node->parent && (dst_node->parent->nodetype == LYS_AUGMENT)
                    && !((struct lys_node_augment *)dst_node->parent)->target) {
                /* we are in an unresolved augment, cannot evaluate */
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_LYS, dst_node->parent,
                       "Cannot resolve leafref predicate \"%s\" because it is in an unresolved augment.", path_key_expr);
                return 0;
            }

            /* path is supposed to be evaluated in data tree, so we have to skip
             * all schema nodes that cannot be instantiated in data tree */
            for (dst_node = lys_parent(dst_node);
                 dst_node && !(dst_node->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_ACTION | LYS_NOTIF | LYS_RPC));
                 dst_node = lys_parent(dst_node));
        }
        while (1) {
            if (dest_pref) {
                trg_mod = lyp_get_module(lys_node_module(parent), NULL, 0, dest_pref, dest_pref_len, 0);
            } else {
                trg_mod = lys_node_module(parent);
            }
            rc = lys_getnext_data(trg_mod, dst_node, dest, dest_len, LYS_CONTAINER | LYS_LIST | LYS_LEAF, &dst_node);
            if (rc) {
                LOGVAL(ctx, LYE_NORESOLV, LY_VLOG_LYS, parent, "leafref predicate", path_key_expr);
                return 0;
            }

            if (pke_len == pke_parsed) {
                break;
            }

            if ((i = parse_path_key_expr(path_key_expr + pke_parsed, &dest_pref, &dest_pref_len, &dest, &dest_len,
                                         &dest_parent_times)) < 1) {
                LOGVAL(ctx, LYE_INCHAR, LY_VLOG_LYS, parent,
                       (path_key_expr + pke_parsed)[-i], (path_key_expr + pke_parsed)-i);
                return -parsed;
            }
            pke_parsed += i;
        }

        /* check source - dest match */
        if (dst_node->nodetype != src_node->nodetype) {
            LOGVAL(ctx, LYE_NORESOLV, LY_VLOG_LYS, parent, "leafref predicate", path - parsed);
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL, "Destination node is not a %s, but a %s.",
                   strnodetype(src_node->nodetype), strnodetype(dst_node->nodetype));
            return -parsed;
        }
    } while (has_predicate);

    return parsed;
}

static int
check_leafref_features(struct lys_type *type)
{
    struct lys_node *iter;
    struct ly_set *src_parents, *trg_parents, *features;
    struct lys_node_augment *aug;
    struct ly_ctx *ctx = ((struct lys_tpdf *)type->parent)->module->ctx;
    unsigned int i, j, size, x;
    int ret = EXIT_SUCCESS;

    assert(type->parent);

    src_parents = ly_set_new();
    trg_parents = ly_set_new();
    features = ly_set_new();

    /* get parents chain of source (leafref) */
    for (iter = (struct lys_node *)type->parent; iter; iter = lys_parent(iter)) {
        if (iter->nodetype & (LYS_INPUT | LYS_OUTPUT)) {
            continue;
        }
        if (iter->parent && (iter->parent->nodetype == LYS_AUGMENT)) {
            aug = (struct lys_node_augment *)iter->parent;
            if ((aug->module->implemented && (aug->flags & LYS_NOTAPPLIED)) || !aug->target) {
                /* unresolved augment, wait until it's resolved */
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_LYS, aug,
                       "Cannot check leafref \"%s\" if-feature consistency because of an unresolved augment.", type->info.lref.path);
                ret = EXIT_FAILURE;
                goto cleanup;
            }
            /* also add this augment */
            ly_set_add(src_parents, aug, LY_SET_OPT_USEASLIST);
        }
        ly_set_add(src_parents, iter, LY_SET_OPT_USEASLIST);
    }
    /* get parents chain of target */
    for (iter = (struct lys_node *)type->info.lref.target; iter; iter = lys_parent(iter)) {
        if (iter->nodetype & (LYS_INPUT | LYS_OUTPUT)) {
            continue;
        }
        if (iter->parent && (iter->parent->nodetype == LYS_AUGMENT)) {
            aug = (struct lys_node_augment *)iter->parent;
            if ((aug->module->implemented && (aug->flags & LYS_NOTAPPLIED)) || !aug->target) {
                /* unresolved augment, wait until it's resolved */
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_LYS, aug,
                       "Cannot check leafref \"%s\" if-feature consistency because of an unresolved augment.", type->info.lref.path);
                ret = EXIT_FAILURE;
                goto cleanup;
            }
        }
        ly_set_add(trg_parents, iter, LY_SET_OPT_USEASLIST);
    }

    /* compare the features used in if-feature statements in the rest of both
     * chains of parents. The set of features used for target must be a subset
     * of features used for the leafref. This is not a perfect, we should compare
     * the truth tables but it could require too much resources, so we simplify that */
    for (i = 0; i < src_parents->number; i++) {
        iter = src_parents->set.s[i]; /* shortcut */
        if (!iter->iffeature_size) {
            continue;
        }
        for (j = 0; j < iter->iffeature_size; j++) {
            resolve_iffeature_getsizes(&iter->iffeature[j], NULL, &size);
            for (; size; size--) {
                if (!iter->iffeature[j].features[size - 1]) {
                    /* not yet resolved feature, postpone this check */
                    ret = EXIT_FAILURE;
                    goto cleanup;
                }
                ly_set_add(features, iter->iffeature[j].features[size - 1], 0);
            }
        }
    }
    x = features->number;
    for (i = 0; i < trg_parents->number; i++) {
        iter = trg_parents->set.s[i]; /* shortcut */
        if (!iter->iffeature_size) {
            continue;
        }
        for (j = 0; j < iter->iffeature_size; j++) {
            resolve_iffeature_getsizes(&iter->iffeature[j], NULL, &size);
            for (; size; size--) {
                if (!iter->iffeature[j].features[size - 1]) {
                    /* not yet resolved feature, postpone this check */
                    ret = EXIT_FAILURE;
                    goto cleanup;
                }
                if ((unsigned)ly_set_add(features, iter->iffeature[j].features[size - 1], 0) >= x) {
                    /* the feature is not present in features set of target's parents chain */
                    LOGVAL(ctx, LYE_NORESOLV, LY_VLOG_LYS, type->parent, "leafref", type->info.lref.path);
                    LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL,
                           "Leafref is not conditional based on \"%s\" feature as its target.",
                           iter->iffeature[j].features[size - 1]->name);
                    ret = -1;
                    goto cleanup;
                }
            }
        }
    }

cleanup:
    ly_set_free(features);
    ly_set_free(src_parents);
    ly_set_free(trg_parents);

    return ret;
}

/**
 * @brief Resolve a path (leafref) in JSON schema context. Logs directly.
 *
 * @param[in] path Path to use.
 * @param[in] parent_node Parent of the leafref.
 * @param[out] ret Pointer to the resolved schema node. Can be NULL.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
resolve_schema_leafref(struct lys_type *type, struct lys_node *parent, struct unres_schema *unres)
{
    const struct lys_node *node, *op_node = NULL, *tmp_parent;
    struct lys_node_augment *last_aug;
    const struct lys_module *tmp_mod, *cur_module;
    const char *id, *prefix, *name;
    int pref_len, nam_len, parent_times, has_predicate;
    int i, first_iter;
    struct ly_ctx *ctx = parent->module->ctx;

    if (!type->info.lref.target) {
        first_iter = 1;
        parent_times = 0;
        id = type->info.lref.path;

        /* find operation schema we are in */
        for (op_node = lys_parent(parent);
            op_node && !(op_node->nodetype & (LYS_ACTION | LYS_NOTIF | LYS_RPC));
            op_node = lys_parent(op_node));

        cur_module = lys_node_module(parent);
        do {
            if ((i = parse_path_arg(cur_module, id, &prefix, &pref_len, &name, &nam_len, &parent_times, &has_predicate)) < 1) {
                LOGVAL(ctx, LYE_INCHAR, LY_VLOG_LYS, parent, id[-i], &id[-i]);
                return -1;
            }
            id += i;

            /* get the current module */
            tmp_mod = prefix ? lyp_get_module(cur_module, NULL, 0, prefix, pref_len, 0) : cur_module;
            if (!tmp_mod) {
                LOGVAL(ctx, LYE_NORESOLV, LY_VLOG_LYS, parent, "leafref", type->info.lref.path);
                return EXIT_FAILURE;
            }
            last_aug = NULL;

            if (first_iter) {
                if (parent_times == -1) {
                    /* use module data */
                    node = NULL;

                } else if (parent_times > 0) {
                    /* we are looking for the right parent */
                    for (i = 0, node = parent; i < parent_times; i++) {
                        if (node->parent && (node->parent->nodetype == LYS_AUGMENT)
                                && !((struct lys_node_augment *)node->parent)->target) {
                            /* we are in an unresolved augment, cannot evaluate */
                            LOGVAL(ctx, LYE_SPEC, LY_VLOG_LYS, node->parent,
                                "Cannot resolve leafref \"%s\" because it is in an unresolved augment.", type->info.lref.path);
                            return EXIT_FAILURE;
                        }

                        /* path is supposed to be evaluated in data tree, so we have to skip
                        * all schema nodes that cannot be instantiated in data tree */
                        for (node = lys_parent(node);
                            node && !(node->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_ACTION | LYS_NOTIF | LYS_RPC));
                            node = lys_parent(node));

                        if (!node) {
                            if (i == parent_times - 1) {
                                /* top-level */
                                break;
                            }

                            /* higher than top-level */
                            LOGVAL(ctx, LYE_NORESOLV, LY_VLOG_LYS, parent, "leafref", type->info.lref.path);
                            return EXIT_FAILURE;
                        }
                    }
                } else {
                    LOGINT(ctx);
                    return -1;
                }
            }

            /* find the next node (either in unconnected augment or as a schema sibling, node is NULL for top-level node -
            * - useless to search for that in augments) */
            if (!tmp_mod->implemented && node) {
    get_next_augment:
                last_aug = lys_getnext_target_aug(last_aug, tmp_mod, node);
            }

            tmp_parent = (last_aug ? (struct lys_node *)last_aug : node);
            node = NULL;
            while ((node = lys_getnext(node, tmp_parent, tmp_mod, LYS_GETNEXT_NOSTATECHECK))) {
                if (lys_node_module(node) != lys_main_module(tmp_mod)) {
                    continue;
                }
                if (strncmp(node->name, name, nam_len) || node->name[nam_len]) {
                    continue;
                }
                /* match */
                break;
            }
            if (!node) {
                if (last_aug) {
                    /* restore the correct augment target */
                    node = last_aug->target;
                    goto get_next_augment;
                }
                LOGVAL(ctx, LYE_NORESOLV, LY_VLOG_LYS, parent, "leafref", type->info.lref.path);
                return EXIT_FAILURE;
            }

            if (first_iter) {
                /* set external dependency flag, we can decide based on the first found node */
                if (resolve_schema_leafref_valid_dep_flag(op_node, cur_module, node, (parent_times == -1 ? 1 : 0))) {
                    parent->flags |= LYS_LEAFREF_DEP;
                }
                first_iter = 0;
            }

            if (has_predicate) {
                /* we have predicate, so the current result must be list */
                if (node->nodetype != LYS_LIST) {
                    LOGVAL(ctx, LYE_NORESOLV, LY_VLOG_LYS, parent, "leafref", type->info.lref.path);
                    return -1;
                }

                i = resolve_schema_leafref_predicate(id, node, parent);
                if (!i) {
                    return EXIT_FAILURE;
                } else if (i < 0) {
                    return -1;
                }
                id += i;
                has_predicate = 0;
            }
        } while (id[0]);

        /* the target must be leaf or leaf-list (in YANG 1.1 only) */
        if ((node->nodetype != LYS_LEAF) && (node->nodetype != LYS_LEAFLIST)) {
            LOGVAL(ctx, LYE_NORESOLV, LY_VLOG_LYS, parent, "leafref", type->info.lref.path);
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL, "Leafref target \"%s\" is not a leaf nor a leaf-list.", type->info.lref.path);
            return -1;
        }

        /* check status */
        if (lyp_check_status(parent->flags, parent->module, parent->name,
                        node->flags, node->module, node->name, node)) {
            return -1;
        }

        /* assign */
        type->info.lref.target = (struct lys_node_leaf *)node;
    }

    /* as the last thing traverse this leafref and make targets on the path implemented */
    if (lys_node_module(parent)->implemented) {
        /* make all the modules in the path implemented */
        for (node = (struct lys_node *)type->info.lref.target; node; node = lys_parent(node)) {
            if (!lys_node_module(node)->implemented) {
                lys_node_module(node)->implemented = 1;
                if (unres_schema_add_node(lys_node_module(node), unres, NULL, UNRES_MOD_IMPLEMENT, NULL) == -1) {
                    return -1;
                }
            }
        }

        /* store the backlink from leafref target */
        if (lys_leaf_add_leafref_target(type->info.lref.target, (struct lys_node *)type->parent)) {
            return -1;
        }
    }

    /* check if leafref and its target are under common if-features */
    return check_leafref_features(type);
}

/**
 * @brief Compare 2 data node values.
 *
 * Comparison performed on canonical forms, the first value
 * is first transformed into canonical form.
 *
 * @param[in] node Leaf/leaf-list with these values.
 * @param[in] noncan_val Non-canonical value.
 * @param[in] noncan_val_len Length of \p noncal_val.
 * @param[in] can_val Canonical value.
 * @return 1 if equal, 0 if not, -1 on error (logged).
 */
static int
valequal(struct lys_node *node, const char *noncan_val, int noncan_val_len, const char *can_val)
{
    int ret;
    struct lyd_node_leaf_list leaf;
    struct lys_node_leaf *sleaf = (struct lys_node_leaf*)node;

    /* dummy leaf */
    memset(&leaf, 0, sizeof leaf);
    leaf.value_str = lydict_insert(node->module->ctx, noncan_val, noncan_val_len);

repeat:
    leaf.value_type = sleaf->type.base;
    leaf.schema = node;

    if (leaf.value_type == LY_TYPE_LEAFREF) {
        if (!sleaf->type.info.lref.target) {
            /* it should either be unresolved leafref (leaf.value_type are ORed flags) or it will be resolved */
            LOGINT(node->module->ctx);
            ret = -1;
            goto finish;
        }
        sleaf = sleaf->type.info.lref.target;
        goto repeat;
    } else {
        if (!lyp_parse_value(&sleaf->type, &leaf.value_str, NULL, &leaf, NULL, NULL, 0, 0, 0)) {
            ret = -1;
            goto finish;
        }
    }

    if (!strcmp(leaf.value_str, can_val)) {
        ret = 1;
    } else {
        ret = 0;
    }

finish:
    lydict_remove(node->module->ctx, leaf.value_str);
    return ret;
}

/**
 * @brief Resolve instance-identifier predicate in JSON data format.
 *        Does not log.
 *
 * @param[in] prev_mod Previous module to use in case there is no prefix.
 * @param[in] pred Predicate to use.
 * @param[in,out] node Node matching the restriction without
 *                     the predicate. If it does not satisfy the predicate,
 *                     it is set to NULL.
 *
 * @return Number of characters successfully parsed,
 *         positive on success, negative on failure.
 */
static int
resolve_instid_predicate(const struct lys_module *prev_mod, const char *pred, struct lyd_node **node, int cur_idx)
{
    /* ... /node[key=value] ... */
    struct lyd_node_leaf_list *key;
    struct lys_node_leaf **list_keys = NULL;
    struct lys_node_list *slist = NULL;
    const char *model, *name, *value;
    int mod_len, nam_len, val_len, i, has_predicate, parsed;
    struct ly_ctx *ctx = prev_mod->ctx;

    assert(pred && node && *node);

    parsed = 0;
    do {
        if ((i = parse_predicate(pred + parsed, &model, &mod_len, &name, &nam_len, &value, &val_len, &has_predicate)) < 1) {
            return -parsed + i;
        }
        parsed += i;

        if (!(*node)) {
            /* just parse it all */
            continue;
        }

        /* target */
        if (name[0] == '.') {
            /* leaf-list value */
            if ((*node)->schema->nodetype != LYS_LEAFLIST) {
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Instance identifier expects leaf-list, but have %s \"%s\".",
                       strnodetype((*node)->schema->nodetype), (*node)->schema->name);
                parsed = -1;
                goto cleanup;
            }

            /* check the value */
            if (!valequal((*node)->schema, value, val_len, ((struct lyd_node_leaf_list *)*node)->value_str)) {
                *node = NULL;
                goto cleanup;
            }

        } else if (isdigit(name[0])) {
            assert(!value);

            /* keyless list position */
            if ((*node)->schema->nodetype != LYS_LIST) {
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Instance identifier expects list, but have %s \"%s\".",
                       strnodetype((*node)->schema->nodetype), (*node)->schema->name);
                parsed = -1;
                goto cleanup;
            }

            if (((struct lys_node_list *)(*node)->schema)->keys) {
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Instance identifier expects list without keys, but have list \"%s\".",
                       (*node)->schema->name);
                parsed = -1;
                goto cleanup;
            }

            /* check the index */
            if (atoi(name) != cur_idx) {
                *node = NULL;
                goto cleanup;
            }

        } else {
            /* list key value */
            if ((*node)->schema->nodetype != LYS_LIST) {
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Instance identifier expects list, but have %s \"%s\".",
                       strnodetype((*node)->schema->nodetype), (*node)->schema->name);
                parsed = -1;
                goto cleanup;
            }
            slist = (struct lys_node_list *)(*node)->schema;

            /* prepare key array */
            if (!list_keys) {
                list_keys = malloc(slist->keys_size * sizeof *list_keys);
                LY_CHECK_ERR_RETURN(!list_keys, LOGMEM(ctx), -1);
                for (i = 0; i < slist->keys_size; ++i) {
                    list_keys[i] = slist->keys[i];
                }
            }

            /* find the schema key leaf */
            for (i = 0; i < slist->keys_size; ++i) {
                if (list_keys[i] && !strncmp(list_keys[i]->name, name, nam_len) && !list_keys[i]->name[nam_len]) {
                    break;
                }
            }
            if (i == slist->keys_size) {
                /* this list has no such key */
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Instance identifier expects list with the key \"%.*s\","
                       " but list \"%s\" does not define it.", nam_len, name, slist->name);
                parsed = -1;
                goto cleanup;
            }

            /* check module */
            if (model) {
                if (strncmp(list_keys[i]->module->name, model, mod_len) || list_keys[i]->module->name[mod_len]) {
                    LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Instance identifier expects key \"%s\" from module \"%.*s\", not \"%s\".",
                           list_keys[i]->name, model, mod_len, list_keys[i]->module->name);
                    parsed = -1;
                    goto cleanup;
                }
            } else {
                if (list_keys[i]->module != prev_mod) {
                    LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Instance identifier expects key \"%s\" from module \"%s\", not \"%s\".",
                           list_keys[i]->name, prev_mod->name, list_keys[i]->module->name);
                    parsed = -1;
                    goto cleanup;
                }
            }

            /* find the actual data key */
            for (key = (struct lyd_node_leaf_list *)(*node)->child; key; key = (struct lyd_node_leaf_list *)key->next) {
                if (key->schema == (struct lys_node *)list_keys[i]) {
                    break;
                }
            }
            if (!key) {
                /* list instance is missing a key? definitely should not happen */
                LOGINT(ctx);
                parsed = -1;
                goto cleanup;
            }

            /* check the value */
            if (!valequal(key->schema, value, val_len, key->value_str)) {
                *node = NULL;
                /* we still want to parse the whole predicate */
                continue;
            }

            /* everything is fine, mark this key as resolved */
            list_keys[i] = NULL;
        }
    } while (has_predicate);

    /* check that all list keys were specified */
    if (*node && list_keys) {
        for (i = 0; i < slist->keys_size; ++i) {
            if (list_keys[i]) {
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Instance identifier is missing list key \"%s\".", list_keys[i]->name);
                parsed = -1;
                goto cleanup;
            }
        }
    }

cleanup:
    free(list_keys);
    return parsed;
}

static int
check_xpath(struct lys_node *node, int check_place)
{
    struct lys_node *parent;
    struct lyxp_set set;
    enum int_log_opts prev_ilo;

    if (check_place) {
        parent = node;
        while (parent) {
            if (parent->nodetype == LYS_GROUPING) {
                /* unresolved grouping, skip for now (will be checked later) */
                return EXIT_SUCCESS;
            }
            if (parent->nodetype == LYS_AUGMENT) {
                if (!((struct lys_node_augment *)parent)->target) {
                    /* unresolved augment, skip for now (will be checked later) */
                    return EXIT_FAILURE;
                } else {
                    parent = ((struct lys_node_augment *)parent)->target;
                    continue;
                }
            }
            parent = parent->parent;
        }
    }

    memset(&set, 0, sizeof set);

    /* produce just warnings */
    ly_ilo_change(NULL, ILO_ERR2WRN, &prev_ilo, NULL);
    lyxp_node_atomize(node, &set, 1);
    ly_ilo_restore(NULL, prev_ilo, NULL, 0);

    if (set.val.snodes) {
        free(set.val.snodes);
    }
    return EXIT_SUCCESS;
}

static int
check_leafref_config(struct lys_node_leaf *leaf, struct lys_type *type)
{
    unsigned int i;

    if (type->base == LY_TYPE_LEAFREF) {
        if ((leaf->flags & LYS_CONFIG_W) && type->info.lref.target && type->info.lref.req != -1 &&
                (type->info.lref.target->flags & LYS_CONFIG_R)) {
            LOGVAL(leaf->module->ctx, LYE_SPEC, LY_VLOG_LYS, leaf, "The leafref %s is config but refers to a non-config %s.",
                   strnodetype(leaf->nodetype), strnodetype(type->info.lref.target->nodetype));
            return -1;
        }
        /* we can skip the test in case the leafref is not yet resolved. In that case the test is done in the time
         * of leafref resolving (lys_leaf_add_leafref_target()) */
    } else if (type->base == LY_TYPE_UNION) {
        for (i = 0; i < type->info.uni.count; i++) {
            if (check_leafref_config(leaf, &type->info.uni.types[i])) {
                return -1;
            }
        }
    }
    return 0;
}

/**
 * @brief Passes config flag down to children, skips nodes without config flags.
 * Logs.
 *
 * @param[in] node Siblings and their children to have flags changed.
 * @param[in] clear Flag to clear all config flags if parent is LYS_NOTIF, LYS_INPUT, LYS_OUTPUT, LYS_RPC.
 * @param[in] flags Flags to assign to all the nodes.
 * @param[in,out] unres List of unresolved items.
 *
 * @return 0 on success, -1 on error.
 */
int
inherit_config_flag(struct lys_node *node, int flags, int clear)
{
    struct lys_node_leaf *leaf;
    struct ly_ctx *ctx;

    if (!node) {
        return 0;
    }

    assert(!(flags ^ (flags & LYS_CONFIG_MASK)));
    ctx = node->module->ctx;

    LY_TREE_FOR(node, node) {
        if (clear) {
            node->flags &= ~LYS_CONFIG_MASK;
            node->flags &= ~LYS_CONFIG_SET;
        } else {
            if (node->flags & LYS_CONFIG_SET) {
                /* skip nodes with an explicit config value */
                if ((flags & LYS_CONFIG_R) && (node->flags & LYS_CONFIG_W)) {
                    LOGVAL(ctx, LYE_INARG, LY_VLOG_LYS, node, "true", "config");
                    LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL, "State nodes cannot have configuration nodes as children.");
                    return -1;
                }
                continue;
            }

            if (!(node->nodetype & (LYS_USES | LYS_GROUPING))) {
                node->flags = (node->flags & ~LYS_CONFIG_MASK) | flags;
                /* check that configuration lists have keys */
                if ((node->nodetype == LYS_LIST) && (node->flags & LYS_CONFIG_W)
                        && !((struct lys_node_list *)node)->keys_size) {
                    LOGVAL(ctx, LYE_MISSCHILDSTMT, LY_VLOG_LYS, node, "key", "list");
                    return -1;
                }
            }
        }
        if (!(node->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA))) {
            if (inherit_config_flag(node->child, flags, clear)) {
                return -1;
            }
        } else if (node->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
            leaf = (struct lys_node_leaf *)node;
            if (check_leafref_config(leaf, &leaf->type)) {
                return -1;
            }
        }
    }

    return 0;
}

/**
 * @brief Resolve augment target. Logs directly.
 *
 * @param[in] aug Augment to use.
 * @param[in] uses Parent where to start the search in. If set, uses augment, if not, standalone augment.
 * @param[in,out] unres List of unresolved items.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
resolve_augment(struct lys_node_augment *aug, struct lys_node *uses, struct unres_schema *unres)
{
    int rc;
    struct lys_node *sub;
    struct lys_module *mod;
    struct ly_set *set;
    struct ly_ctx *ctx;

    assert(aug);
    mod = lys_main_module(aug->module);
    ctx = mod->ctx;

    /* set it as not applied for now */
    aug->flags |= LYS_NOTAPPLIED;

    /* it can already be resolved in case we returned EXIT_FAILURE from if block below */
    if (!aug->target) {
        /* resolve target node */
        rc = resolve_schema_nodeid(aug->target_name, uses, (uses ? NULL : lys_node_module((struct lys_node *)aug)), &set, 0, 0);
        if (rc == -1) {
            LOGVAL(ctx, LYE_PATH, LY_VLOG_LYS, aug);
            return -1;
        }
        if (!set) {
            LOGVAL(ctx, LYE_INRESOLV, LY_VLOG_LYS, aug, "augment", aug->target_name);
            return EXIT_FAILURE;
        }
        aug->target = set->set.s[0];
        ly_set_free(set);
    }

    /* make this module implemented if the target module is (if the target is in an unimplemented module,
     * it is fine because when we will be making that module implemented, its augment will be applied
     * and that augment target module made implemented, recursively) */
    if (mod->implemented && !lys_node_module(aug->target)->implemented) {
        lys_node_module(aug->target)->implemented = 1;
        if (unres_schema_add_node(lys_node_module(aug->target), unres, NULL, UNRES_MOD_IMPLEMENT, NULL) == -1) {
            return -1;
        }
    }

    /* check for mandatory nodes - if the target node is in another module
     * the added nodes cannot be mandatory
     */
    if (!aug->parent && (lys_node_module((struct lys_node *)aug) != lys_node_module(aug->target))
            && (rc = lyp_check_mandatory_augment(aug, aug->target))) {
        return rc;
    }

    /* check augment target type and then augment nodes type */
    if (aug->target->nodetype & (LYS_CONTAINER | LYS_LIST)) {
        LY_TREE_FOR(aug->child, sub) {
            if (!(sub->nodetype & (LYS_ANYDATA | LYS_CONTAINER | LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_USES
                                   | LYS_CHOICE | LYS_ACTION | LYS_NOTIF))) {
                LOGVAL(ctx, LYE_INCHILDSTMT, LY_VLOG_LYS, aug, strnodetype(sub->nodetype), "augment");
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL, "Cannot augment \"%s\" with a \"%s\".",
                       strnodetype(aug->target->nodetype), strnodetype(sub->nodetype));
                return -1;
            }
        }
    } else if (aug->target->nodetype & (LYS_CASE | LYS_INPUT | LYS_OUTPUT | LYS_NOTIF)) {
        LY_TREE_FOR(aug->child, sub) {
            if (!(sub->nodetype & (LYS_ANYDATA | LYS_CONTAINER | LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_USES | LYS_CHOICE))) {
                LOGVAL(ctx, LYE_INCHILDSTMT, LY_VLOG_LYS, aug, strnodetype(sub->nodetype), "augment");
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL, "Cannot augment \"%s\" with a \"%s\".",
                       strnodetype(aug->target->nodetype), strnodetype(sub->nodetype));
                return -1;
            }
        }
    } else if (aug->target->nodetype == LYS_CHOICE) {
        LY_TREE_FOR(aug->child, sub) {
            if (!(sub->nodetype & (LYS_CASE | LYS_ANYDATA | LYS_CONTAINER | LYS_LEAF | LYS_LIST | LYS_LEAFLIST))) {
                LOGVAL(ctx, LYE_INCHILDSTMT, LY_VLOG_LYS, aug, strnodetype(sub->nodetype), "augment");
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL, "Cannot augment \"%s\" with a \"%s\".",
                       strnodetype(aug->target->nodetype), strnodetype(sub->nodetype));
                return -1;
            }
        }
    } else {
        LOGVAL(ctx, LYE_INARG, LY_VLOG_LYS, aug, aug->target_name, "target-node");
        LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL, "Invalid augment target node type \"%s\".", strnodetype(aug->target->nodetype));
        return -1;
    }

    /* check identifier uniqueness as in lys_node_addchild() */
    LY_TREE_FOR(aug->child, sub) {
        if (lys_check_id(sub, aug->target, NULL)) {
            return -1;
        }
    }

    if (!aug->child) {
        /* empty augment, nothing to connect, but it is techincally applied */
        LOGWRN(ctx, "Augment \"%s\" without children.", aug->target_name);
        aug->flags &= ~LYS_NOTAPPLIED;
    } else if ((aug->parent || mod->implemented) && apply_aug(aug, unres)) {
        /* we try to connect the augment only in case the module is implemented or
         * the augment applies on the used grouping, anyway we failed here */
        return -1;
    }

    return EXIT_SUCCESS;
}

static int
resolve_extension(struct unres_ext *info, struct lys_ext_instance **ext, struct unres_schema *unres)
{
    enum LY_VLOG_ELEM vlog_type;
    void *vlog_node;
    unsigned int i, j;
    struct lys_ext *e;
    char *ext_name, *ext_prefix, *tmp;
    struct lyxml_elem *next_yin, *yin;
    const struct lys_module *mod;
    struct lys_ext_instance *tmp_ext;
    struct ly_ctx *ctx = NULL;
    LYEXT_TYPE etype;

    switch (info->parent_type) {
    case LYEXT_PAR_NODE:
        vlog_node = info->parent;
        vlog_type = LY_VLOG_LYS;
        break;
    case LYEXT_PAR_MODULE:
    case LYEXT_PAR_IMPORT:
    case LYEXT_PAR_INCLUDE:
        vlog_node = NULL;
        vlog_type = LY_VLOG_LYS;
        break;
    default:
        vlog_node = NULL;
        vlog_type = LY_VLOG_NONE;
        break;
    }

    if (info->datatype == LYS_IN_YIN) {
        /* YIN */

        /* get the module where the extension is supposed to be defined */
        mod = lyp_get_import_module_ns(info->mod, info->data.yin->ns->value);
        if (!mod) {
            LOGVAL(ctx, LYE_INSTMT, vlog_type, vlog_node, info->data.yin->name);
            return EXIT_FAILURE;
        }
        ctx = mod->ctx;

        /* find the extension definition */
        e = NULL;
        for (i = 0; i < mod->extensions_size; i++) {
            if (ly_strequal(mod->extensions[i].name, info->data.yin->name, 1)) {
                e = &mod->extensions[i];
                break;
            }
        }
        /* try submodules */
        for (j = 0; !e && j < mod->inc_size; j++) {
            for (i = 0; i < mod->inc[j].submodule->extensions_size; i++) {
                if (ly_strequal(mod->inc[j].submodule->extensions[i].name, info->data.yin->name, 1)) {
                    e = &mod->inc[j].submodule->extensions[i];
                    break;
                }
            }
        }
        if (!e) {
            LOGVAL(ctx, LYE_INSTMT, vlog_type, vlog_node, info->data.yin->name);
            return EXIT_FAILURE;
        }

        /* we have the extension definition, so now it cannot be forward referenced and error is always fatal */

        if (e->plugin && e->plugin->check_position) {
            /* common part - we have plugin with position checking function, use it first */
            if ((*e->plugin->check_position)(info->parent, info->parent_type, info->substmt)) {
                /* extension is not allowed here */
                LOGVAL(ctx, LYE_INSTMT, vlog_type, vlog_node, e->name);
                return -1;
            }
        }

        /* extension type-specific part - allocation */
        if (e->plugin) {
            etype = e->plugin->type;
        } else {
            /* default type */
            etype = LYEXT_FLAG;
        }
        switch (etype) {
        case LYEXT_FLAG:
            (*ext) = calloc(1, sizeof(struct lys_ext_instance));
            break;
        case LYEXT_COMPLEX:
            (*ext) = calloc(1, ((struct lyext_plugin_complex*)e->plugin)->instance_size);
            break;
        case LYEXT_ERR:
            /* we never should be here */
            LOGINT(ctx);
            return -1;
        }
        LY_CHECK_ERR_RETURN(!*ext, LOGMEM(ctx), -1);

        /* common part for all extension types */
        (*ext)->def = e;
        (*ext)->parent = info->parent;
        (*ext)->parent_type = info->parent_type;
        (*ext)->insubstmt = info->substmt;
        (*ext)->insubstmt_index = info->substmt_index;
        (*ext)->ext_type = e->plugin ? e->plugin->type : LYEXT_FLAG;
        (*ext)->flags |= e->plugin ? e->plugin->flags : 0;

        if (e->argument) {
            if (!(e->flags & LYS_YINELEM)) {
                (*ext)->arg_value = lyxml_get_attr(info->data.yin, e->argument, NULL);
                if (!(*ext)->arg_value) {
                    LOGVAL(ctx, LYE_MISSARG, LY_VLOG_NONE, NULL, e->argument, info->data.yin->name);
                    return -1;
                }

                (*ext)->arg_value = lydict_insert(mod->ctx, (*ext)->arg_value, 0);
            } else {
                LY_TREE_FOR_SAFE(info->data.yin->child, next_yin, yin) {
                    if (ly_strequal(yin->name, e->argument, 1)) {
                        (*ext)->arg_value = lydict_insert(mod->ctx, yin->content, 0);
                        lyxml_free(mod->ctx, yin);
                        break;
                    }
                }
            }
        }

        if ((*ext)->flags & LYEXT_OPT_VALID &&
            (info->parent_type == LYEXT_PAR_NODE || info->parent_type == LYEXT_PAR_TPDF)) {
            ((struct lys_node *)info->parent)->flags |= LYS_VALID_EXT;
        }

        (*ext)->nodetype = LYS_EXT;
        (*ext)->module = info->mod;

        /* extension type-specific part - parsing content */
        switch (etype) {
        case LYEXT_FLAG:
            LY_TREE_FOR_SAFE(info->data.yin->child, next_yin, yin) {
                if (!yin->ns) {
                    /* garbage */
                    lyxml_free(mod->ctx, yin);
                    continue;
                } else if (!strcmp(yin->ns->value, LY_NSYIN)) {
                    /* standard YANG statements are not expected here */
                    LOGVAL(ctx, LYE_INCHILDSTMT, vlog_type, vlog_node, yin->name, info->data.yin->name);
                    return -1;
                } else if (yin->ns == info->data.yin->ns &&
                        (e->flags & LYS_YINELEM) && ly_strequal(yin->name, e->argument, 1)) {
                    /* we have the extension's argument */
                    if ((*ext)->arg_value) {
                        LOGVAL(ctx, LYE_TOOMANY, vlog_type, vlog_node, yin->name, info->data.yin->name);
                        return -1;
                    }
                    (*ext)->arg_value = yin->content;
                    yin->content = NULL;
                    lyxml_free(mod->ctx, yin);
                } else {
                    /* extension instance */
                    if (lyp_yin_parse_subnode_ext(info->mod, *ext, LYEXT_PAR_EXTINST, yin,
                                                  LYEXT_SUBSTMT_SELF, 0, unres)) {
                        return -1;
                    }

                    continue;
                }
            }
            break;
        case LYEXT_COMPLEX:
            ((struct lys_ext_instance_complex*)(*ext))->substmt = ((struct lyext_plugin_complex*)e->plugin)->substmt;
            if (lyp_yin_parse_complex_ext(info->mod, (struct lys_ext_instance_complex*)(*ext), info->data.yin, unres)) {
                /* TODO memory cleanup */
                return -1;
            }
            break;
        default:
            break;
        }

        /* TODO - lyext_check_result_clb, other than LYEXT_FLAG plugins */

    } else {
        /* YANG */

        ext_prefix = (char *)(*ext)->def;
        tmp = strchr(ext_prefix, ':');
        if (!tmp) {
            LOGVAL(ctx, LYE_INSTMT, vlog_type, vlog_node, ext_prefix);
            goto error;
        }
        ext_name = tmp + 1;

        /* get the module where the extension is supposed to be defined */
        mod = lyp_get_module(info->mod, ext_prefix, tmp - ext_prefix, NULL, 0, 0);
        if (!mod) {
            LOGVAL(ctx, LYE_INSTMT, vlog_type, vlog_node, ext_prefix);
            return EXIT_FAILURE;
        }
        ctx = mod->ctx;

        /* find the extension definition */
        e = NULL;
        for (i = 0; i < mod->extensions_size; i++) {
            if (ly_strequal(mod->extensions[i].name, ext_name, 0)) {
                e = &mod->extensions[i];
                break;
            }
        }
        /* try submodules */
        for (j = 0; !e && j < mod->inc_size; j++) {
            for (i = 0; i < mod->inc[j].submodule->extensions_size; i++) {
                if (ly_strequal(mod->inc[j].submodule->extensions[i].name, ext_name, 0)) {
                    e = &mod->inc[j].submodule->extensions[i];
                    break;
                }
            }
        }
        if (!e) {
            LOGVAL(ctx, LYE_INSTMT, vlog_type, vlog_node, ext_prefix);
            return EXIT_FAILURE;
        }

        (*ext)->flags &= ~LYEXT_OPT_YANG;
        (*ext)->def = NULL;

        /* we have the extension definition, so now it cannot be forward referenced and error is always fatal */

        if (e->plugin && e->plugin->check_position) {
            /* common part - we have plugin with position checking function, use it first */
            if ((*e->plugin->check_position)(info->parent, info->parent_type, info->substmt)) {
                /* extension is not allowed here */
                LOGVAL(ctx, LYE_INSTMT, vlog_type, vlog_node, e->name);
                goto error;
            }
        }

        /* extension common part */
        (*ext)->def = e;
        (*ext)->parent = info->parent;
        (*ext)->ext_type = e->plugin ? e->plugin->type : LYEXT_FLAG;
        (*ext)->flags |= e->plugin ? e->plugin->flags : 0;

        if (e->argument && !(*ext)->arg_value) {
            LOGVAL(ctx, LYE_MISSARG, LY_VLOG_NONE, NULL, e->argument, ext_name);
            goto error;
        }

        if ((*ext)->flags & LYEXT_OPT_VALID &&
            (info->parent_type == LYEXT_PAR_NODE || info->parent_type == LYEXT_PAR_TPDF)) {
            ((struct lys_node *)info->parent)->flags |= LYS_VALID_EXT;
        }

        (*ext)->module = info->mod;
        (*ext)->nodetype = LYS_EXT;

        /* extension type-specific part */
        if (e->plugin) {
            etype = e->plugin->type;
        } else {
            /* default type */
            etype = LYEXT_FLAG;
        }
        switch (etype) {
        case LYEXT_FLAG:
            /* nothing change */
            break;
        case LYEXT_COMPLEX:
            tmp_ext = realloc(*ext, ((struct lyext_plugin_complex*)e->plugin)->instance_size);
            LY_CHECK_ERR_GOTO(!tmp_ext, LOGMEM(ctx), error);
            memset((char *)tmp_ext + offsetof(struct lys_ext_instance_complex, content), 0,
                   ((struct lyext_plugin_complex*)e->plugin)->instance_size - offsetof(struct lys_ext_instance_complex, content));
            (*ext) = tmp_ext;
            ((struct lys_ext_instance_complex*)(*ext))->substmt = ((struct lyext_plugin_complex*)e->plugin)->substmt;
            if (info->data.yang) {
                *tmp = ':';
                if (yang_parse_ext_substatement(info->mod, unres, info->data.yang->ext_substmt, ext_prefix,
                                                (struct lys_ext_instance_complex*)(*ext))) {
                    goto error;
                }
                if (yang_fill_extcomplex_module(info->mod->ctx, (struct lys_ext_instance_complex*)(*ext), ext_prefix,
                                                info->data.yang->ext_modules, info->mod->implemented)) {
                    goto error;
                }
            }
            if (lyp_mand_check_ext((struct lys_ext_instance_complex*)(*ext), ext_prefix)) {
                goto error;
            }
            break;
        case LYEXT_ERR:
            /* we never should be here */
            LOGINT(ctx);
            goto error;
        }

        if (yang_check_ext_instance(info->mod, &(*ext)->ext, (*ext)->ext_size, *ext, unres)) {
            goto error;
        }
        free(ext_prefix);
    }

    return EXIT_SUCCESS;
error:
    free(ext_prefix);
    return -1;
}

/**
 * @brief Resolve (find) choice default case. Does not log.
 *
 * @param[in] choic Choice to use.
 * @param[in] dflt Name of the default case.
 *
 * @return Pointer to the default node or NULL.
 */
static struct lys_node *
resolve_choice_dflt(struct lys_node_choice *choic, const char *dflt)
{
    struct lys_node *child, *ret;

    LY_TREE_FOR(choic->child, child) {
        if (child->nodetype == LYS_USES) {
            ret = resolve_choice_dflt((struct lys_node_choice *)child, dflt);
            if (ret) {
                return ret;
            }
        }

        if (ly_strequal(child->name, dflt, 1) && (child->nodetype & (LYS_ANYDATA | LYS_CASE
                | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_CHOICE))) {
            return child;
        }
    }

    return NULL;
}

/**
 * @brief Resolve uses, apply augments, refines. Logs directly.
 *
 * @param[in] uses Uses to use.
 * @param[in,out] unres List of unresolved items.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
resolve_uses(struct lys_node_uses *uses, struct unres_schema *unres)
{
    struct ly_ctx *ctx = uses->module->ctx; /* shortcut */
    struct lys_node *node = NULL, *next, *iter, **refine_nodes = NULL;
    struct lys_node *node_aux, *parent, *tmp;
    struct lys_node_leaflist *llist;
    struct lys_node_leaf *leaf;
    struct lys_refine *rfn;
    struct lys_restr *must, **old_must;
    struct lys_iffeature *iff, **old_iff;
    int i, j, k, rc;
    uint8_t size, *old_size;
    unsigned int usize, usize1, usize2;

    assert(uses->grp);

    /* check that the grouping is resolved (no unresolved uses inside) */
    assert(!uses->grp->unres_count);

    /* copy the data nodes from grouping into the uses context */
    LY_TREE_FOR(uses->grp->child, node_aux) {
        if (node_aux->nodetype & LYS_GROUPING) {
            /* do not instantiate groupings from groupings */
            continue;
        }
        node = lys_node_dup(uses->module, (struct lys_node *)uses, node_aux, unres, 0);
        if (!node) {
            LOGVAL(ctx, LYE_INARG, LY_VLOG_LYS, uses, uses->grp->name, "uses");
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL, "Copying data from grouping failed.");
            goto fail;
        }
        /* test the name of siblings */
        LY_TREE_FOR((uses->parent) ? *lys_child(uses->parent, LYS_USES) : lys_main_module(uses->module)->data, tmp) {
            if (!(tmp->nodetype & (LYS_USES | LYS_GROUPING | LYS_CASE)) && ly_strequal(tmp->name, node_aux->name, 1)) {
                goto fail;
            }
        }
    }

    /* we managed to copy the grouping, the rest must be possible to resolve */

    if (uses->refine_size) {
        refine_nodes = malloc(uses->refine_size * sizeof *refine_nodes);
        LY_CHECK_ERR_GOTO(!refine_nodes, LOGMEM(ctx), fail);
    }

    /* apply refines */
    for (i = 0; i < uses->refine_size; i++) {
        rfn = &uses->refine[i];
        rc = resolve_descendant_schema_nodeid(rfn->target_name, uses->child,
                                              LYS_NO_RPC_NOTIF_NODE | LYS_ACTION | LYS_NOTIF,
                                              0, (const struct lys_node **)&node);
        if (rc || !node) {
            LOGVAL(ctx, LYE_INARG, LY_VLOG_LYS, uses, rfn->target_name, "refine");
            goto fail;
        }

        if (rfn->target_type && !(node->nodetype & rfn->target_type)) {
            LOGVAL(ctx, LYE_INARG, LY_VLOG_LYS, uses, rfn->target_name, "refine");
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL, "Refine substatements not applicable to the target-node.");
            goto fail;
        }
        refine_nodes[i] = node;

        /* description on any nodetype */
        if (rfn->dsc) {
            lydict_remove(ctx, node->dsc);
            node->dsc = lydict_insert(ctx, rfn->dsc, 0);
        }

        /* reference on any nodetype */
        if (rfn->ref) {
            lydict_remove(ctx, node->ref);
            node->ref = lydict_insert(ctx, rfn->ref, 0);
        }

        /* config on any nodetype,
         * in case of notification or rpc/action, the config is not applicable (there is no config status) */
        if ((rfn->flags & LYS_CONFIG_MASK) && (node->flags & LYS_CONFIG_MASK)) {
            node->flags &= ~LYS_CONFIG_MASK;
            node->flags |= (rfn->flags & LYS_CONFIG_MASK);
        }

        /* default value ... */
        if (rfn->dflt_size) {
            if (node->nodetype == LYS_LEAF) {
                /* leaf */
                leaf = (struct lys_node_leaf *)node;

                /* replace default value */
                lydict_remove(ctx, leaf->dflt);
                leaf->dflt = lydict_insert(ctx, rfn->dflt[0], 0);

                /* check the default value */
                if (unres_schema_add_node(leaf->module, unres, &leaf->type, UNRES_TYPE_DFLT,
                                          (struct lys_node *)(&leaf->dflt)) == -1) {
                    goto fail;
                }
            } else if (node->nodetype == LYS_LEAFLIST) {
                /* leaf-list */
                llist = (struct lys_node_leaflist *)node;

                /* remove complete set of defaults in target */
                for (j = 0; j < llist->dflt_size; j++) {
                    lydict_remove(ctx, llist->dflt[j]);
                }
                free(llist->dflt);

                /* copy the default set from refine */
                llist->dflt = malloc(rfn->dflt_size * sizeof *llist->dflt);
                LY_CHECK_ERR_GOTO(!llist->dflt, LOGMEM(ctx), fail);
                llist->dflt_size = rfn->dflt_size;
                for (j = 0; j < llist->dflt_size; j++) {
                    llist->dflt[j] = lydict_insert(ctx, rfn->dflt[j], 0);
                }

                /* check default value */
                for (j = 0; j < llist->dflt_size; j++) {
                    if (unres_schema_add_node(llist->module, unres, &llist->type, UNRES_TYPE_DFLT,
                                              (struct lys_node *)(&llist->dflt[j])) == -1) {
                        goto fail;
                    }
                }
            }
        }

        /* mandatory on leaf, anyxml or choice */
        if (rfn->flags & LYS_MAND_MASK) {
            /* remove current value */
            node->flags &= ~LYS_MAND_MASK;

            /* set new value */
            node->flags |= (rfn->flags & LYS_MAND_MASK);

            if (rfn->flags & LYS_MAND_TRUE) {
                /* check if node has default value */
                if ((node->nodetype & LYS_LEAF) && ((struct lys_node_leaf *)node)->dflt) {
                    LOGVAL(ctx, LYE_SPEC, LY_VLOG_LYS, uses,
                           "The \"mandatory\" statement is forbidden on leaf with \"default\".");
                    goto fail;
                }
                if ((node->nodetype & LYS_CHOICE) && ((struct lys_node_choice *)node)->dflt) {
                    LOGVAL(ctx, LYE_SPEC, LY_VLOG_LYS, uses,
                           "The \"mandatory\" statement is forbidden on choices with \"default\".");
                    goto fail;
                }
            }
        }

        /* presence on container */
        if ((node->nodetype & LYS_CONTAINER) && rfn->mod.presence) {
            lydict_remove(ctx, ((struct lys_node_container *)node)->presence);
            ((struct lys_node_container *)node)->presence = lydict_insert(ctx, rfn->mod.presence, 0);
        }

        /* min/max-elements on list or leaf-list */
        if (node->nodetype == LYS_LIST) {
            if (rfn->flags & LYS_RFN_MINSET) {
                ((struct lys_node_list *)node)->min = rfn->mod.list.min;
            }
            if (rfn->flags & LYS_RFN_MAXSET) {
                ((struct lys_node_list *)node)->max = rfn->mod.list.max;
            }
        } else if (node->nodetype == LYS_LEAFLIST) {
            if (rfn->flags & LYS_RFN_MINSET) {
                ((struct lys_node_leaflist *)node)->min = rfn->mod.list.min;
            }
            if (rfn->flags & LYS_RFN_MAXSET) {
                ((struct lys_node_leaflist *)node)->max = rfn->mod.list.max;
            }
        }

        /* must in leaf, leaf-list, list, container or anyxml */
        if (rfn->must_size) {
            switch (node->nodetype) {
            case LYS_LEAF:
                old_size = &((struct lys_node_leaf *)node)->must_size;
                old_must = &((struct lys_node_leaf *)node)->must;
                break;
            case LYS_LEAFLIST:
                old_size = &((struct lys_node_leaflist *)node)->must_size;
                old_must = &((struct lys_node_leaflist *)node)->must;
                break;
            case LYS_LIST:
                old_size = &((struct lys_node_list *)node)->must_size;
                old_must = &((struct lys_node_list *)node)->must;
                break;
            case LYS_CONTAINER:
                old_size = &((struct lys_node_container *)node)->must_size;
                old_must = &((struct lys_node_container *)node)->must;
                break;
            case LYS_ANYXML:
            case LYS_ANYDATA:
                old_size = &((struct lys_node_anydata *)node)->must_size;
                old_must = &((struct lys_node_anydata *)node)->must;
                break;
            default:
                LOGINT(ctx);
                goto fail;
            }

            size = *old_size + rfn->must_size;
            must = realloc(*old_must, size * sizeof *rfn->must);
            LY_CHECK_ERR_GOTO(!must, LOGMEM(ctx), fail);
            for (k = 0, j = *old_size; k < rfn->must_size; k++, j++) {
                must[j].ext_size = rfn->must[k].ext_size;
                lys_ext_dup(ctx, rfn->module, rfn->must[k].ext, rfn->must[k].ext_size, &rfn->must[k], LYEXT_PAR_RESTR,
                            &must[j].ext, 0, unres);
                must[j].expr = lydict_insert(ctx, rfn->must[k].expr, 0);
                must[j].dsc = lydict_insert(ctx, rfn->must[k].dsc, 0);
                must[j].ref = lydict_insert(ctx, rfn->must[k].ref, 0);
                must[j].eapptag = lydict_insert(ctx, rfn->must[k].eapptag, 0);
                must[j].emsg = lydict_insert(ctx, rfn->must[k].emsg, 0);
                must[j].flags = rfn->must[k].flags;
            }

            *old_must = must;
            *old_size = size;

            /* check XPath dependencies again */
            if (unres_schema_add_node(node->module, unres, node, UNRES_XPATH, NULL) == -1) {
                goto fail;
            }
        }

        /* if-feature in leaf, leaf-list, list, container or anyxml */
        if (rfn->iffeature_size) {
            old_size = &node->iffeature_size;
            old_iff = &node->iffeature;

            size = *old_size + rfn->iffeature_size;
            iff = realloc(*old_iff, size * sizeof *rfn->iffeature);
            LY_CHECK_ERR_GOTO(!iff, LOGMEM(ctx), fail);
            *old_iff = iff;

            for (k = 0, j = *old_size; k < rfn->iffeature_size; k++, j++) {
                resolve_iffeature_getsizes(&rfn->iffeature[k], &usize1, &usize2);
                if (usize1) {
                    /* there is something to duplicate */
                    /* duplicate compiled expression */
                    usize = (usize1 / 4) + (usize1 % 4) ? 1 : 0;
                    iff[j].expr = malloc(usize * sizeof *iff[j].expr);
                    LY_CHECK_ERR_GOTO(!iff[j].expr, LOGMEM(ctx), fail);
                    memcpy(iff[j].expr, rfn->iffeature[k].expr, usize * sizeof *iff[j].expr);

                    /* duplicate list of feature pointers */
                    iff[j].features = malloc(usize2 * sizeof *iff[k].features);
                    LY_CHECK_ERR_GOTO(!iff[j].expr, LOGMEM(ctx), fail);
                    memcpy(iff[j].features, rfn->iffeature[k].features, usize2 * sizeof *iff[j].features);

                    /* duplicate extensions */
                    iff[j].ext_size = rfn->iffeature[k].ext_size;
                    lys_ext_dup(ctx, rfn->module, rfn->iffeature[k].ext, rfn->iffeature[k].ext_size,
                                &rfn->iffeature[k], LYEXT_PAR_IFFEATURE, &iff[j].ext, 0, unres);
                }
                (*old_size)++;
            }
            assert(*old_size == size);
        }
    }

    /* apply augments */
    for (i = 0; i < uses->augment_size; i++) {
        rc = resolve_augment(&uses->augment[i], (struct lys_node *)uses, unres);
        if (rc) {
            goto fail;
        }
    }

    /* check refines */
    for (i = 0; i < uses->refine_size; i++) {
        node = refine_nodes[i];
        rfn = &uses->refine[i];

        /* config on any nodetype */
        if ((rfn->flags & LYS_CONFIG_MASK) && (node->flags & LYS_CONFIG_MASK)) {
            for (parent = lys_parent(node); parent && parent->nodetype == LYS_USES; parent = lys_parent(parent));
            if (parent && parent->nodetype != LYS_GROUPING && (parent->flags & LYS_CONFIG_MASK) &&
                    ((parent->flags & LYS_CONFIG_MASK) != (rfn->flags & LYS_CONFIG_MASK)) &&
                    (rfn->flags & LYS_CONFIG_W)) {
                /* setting config true under config false is prohibited */
                LOGVAL(ctx, LYE_INARG, LY_VLOG_LYS, uses, "config", "refine");
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL,
                       "changing config from 'false' to 'true' is prohibited while "
                       "the target's parent is still config 'false'.");
                goto fail;
            }

            /* inherit config change to the target children */
            LY_TREE_DFS_BEGIN(node->child, next, iter) {
                if (rfn->flags & LYS_CONFIG_W) {
                    if (iter->flags & LYS_CONFIG_SET) {
                        /* config is set explicitely, go to next sibling */
                        next = NULL;
                        goto nextsibling;
                    }
                } else { /* LYS_CONFIG_R */
                    if ((iter->flags & LYS_CONFIG_SET) && (iter->flags & LYS_CONFIG_W)) {
                        /* error - we would have config data under status data */
                        LOGVAL(ctx, LYE_INARG, LY_VLOG_LYS, uses, "config", "refine");
                        LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL,
                               "changing config from 'true' to 'false' is prohibited while the target "
                               "has still a children with explicit config 'true'.");
                        goto fail;
                    }
                }
                /* change config */
                iter->flags &= ~LYS_CONFIG_MASK;
                iter->flags |= (rfn->flags & LYS_CONFIG_MASK);

                /* select next iter - modified LY_TREE_DFS_END */
                if (iter->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) {
                    next = NULL;
                } else {
                    next = iter->child;
                }
nextsibling:
                if (!next) {
                    /* try siblings */
                    next = iter->next;
                }
                while (!next) {
                    /* parent is already processed, go to its sibling */
                    iter = lys_parent(iter);

                    /* no siblings, go back through parents */
                    if (iter == node) {
                        /* we are done, no next element to process */
                        break;
                    }
                    next = iter->next;
                }
            }
        }

        /* default value */
        if (rfn->dflt_size) {
            if (node->nodetype == LYS_CHOICE) {
                /* choice */
                ((struct lys_node_choice *)node)->dflt = resolve_choice_dflt((struct lys_node_choice *)node,
                                                                             rfn->dflt[0]);
                if (!((struct lys_node_choice *)node)->dflt) {
                    LOGVAL(ctx, LYE_INARG, LY_VLOG_LYS, uses, rfn->dflt[0], "default");
                    goto fail;
                }
                if (lyp_check_mandatory_choice(node)) {
                    goto fail;
                }
            }
        }

        /* min/max-elements on list or leaf-list */
        if (node->nodetype == LYS_LIST && ((struct lys_node_list *)node)->max) {
            if (((struct lys_node_list *)node)->min > ((struct lys_node_list *)node)->max) {
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_LYS, uses, "Invalid value \"%d\" of \"%s\".", rfn->mod.list.min, "min-elements");
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL, "\"min-elements\" is bigger than \"max-elements\".");
                goto fail;
            }
        } else if (node->nodetype == LYS_LEAFLIST && ((struct lys_node_leaflist *)node)->max) {
            if (((struct lys_node_leaflist *)node)->min > ((struct lys_node_leaflist *)node)->max) {
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_LYS, uses, "Invalid value \"%d\" of \"%s\".", rfn->mod.list.min, "min-elements");
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL, "\"min-elements\" is bigger than \"max-elements\".");
                goto fail;
            }
        }

        /* additional checks */
        /* default value with mandatory/min-elements */
        if (node->nodetype == LYS_LEAFLIST) {
            llist = (struct lys_node_leaflist *)node;
            if (llist->dflt_size && llist->min) {
                LOGVAL(ctx, LYE_INCHILDSTMT, LY_VLOG_LYS, uses, rfn->dflt_size ? "default" : "min-elements", "refine");
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL,
                       "The \"min-elements\" statement with non-zero value is forbidden on leaf-lists with the \"default\" statement.");
                goto fail;
            }
        } else if (node->nodetype == LYS_LEAF) {
            leaf = (struct lys_node_leaf *)node;
            if (leaf->dflt && (leaf->flags & LYS_MAND_TRUE)) {
                LOGVAL(ctx, LYE_INCHILDSTMT, LY_VLOG_LYS, uses, rfn->dflt_size ? "default" : "mandatory", "refine");
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL,
                       "The \"mandatory\" statement is forbidden on leafs with the \"default\" statement.");
                goto fail;
            }
        }

        /* check for mandatory node in default case, first find the closest parent choice to the changed node */
        if ((rfn->flags & LYS_MAND_TRUE) || rfn->mod.list.min) {
            for (parent = node->parent;
                 parent && !(parent->nodetype & (LYS_CHOICE | LYS_GROUPING | LYS_ACTION | LYS_USES));
                 parent = parent->parent) {
                if (parent->nodetype == LYS_CONTAINER && ((struct lys_node_container *)parent)->presence) {
                    /* stop also on presence containers */
                    break;
                }
            }
            /* and if it is a choice with the default case, check it for presence of a mandatory node in it */
            if (parent && parent->nodetype == LYS_CHOICE && ((struct lys_node_choice *)parent)->dflt) {
                if (lyp_check_mandatory_choice(parent)) {
                    goto fail;
                }
            }
        }
    }
    free(refine_nodes);

    return EXIT_SUCCESS;

fail:
    LY_TREE_FOR_SAFE(uses->child, next, iter) {
        lys_node_free(iter, NULL, 0);
    }
    free(refine_nodes);
    return -1;
}

void
resolve_identity_backlink_update(struct lys_ident *der, struct lys_ident *base)
{
    int i;

    assert(der && base);

    if (!base->der) {
        /* create a set for backlinks if it does not exist */
        base->der = ly_set_new();
    }
    /* store backlink */
    ly_set_add(base->der, der, LY_SET_OPT_USEASLIST);

    /* do it recursively */
    for (i = 0; i < base->base_size; i++) {
        resolve_identity_backlink_update(der, base->base[i]);
    }
}

/**
 * @brief Resolve base identity recursively. Does not log.
 *
 * @param[in] module Main module.
 * @param[in] ident Identity to use.
 * @param[in] basename Base name of the identity.
 * @param[out] ret Pointer to the resolved identity. Can be NULL.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on crucial error.
 */
static int
resolve_base_ident_sub(const struct lys_module *module, struct lys_ident *ident, const char *basename,
                       struct unres_schema *unres, struct lys_ident **ret)
{
    uint32_t i, j;
    struct lys_ident *base = NULL;
    struct ly_ctx *ctx = module->ctx;

    assert(ret);

    /* search module */
    for (i = 0; i < module->ident_size; i++) {
        if (!strcmp(basename, module->ident[i].name)) {

            if (!ident) {
                /* just search for type, so do not modify anything, just return
                 * the base identity pointer */
                *ret = &module->ident[i];
                return EXIT_SUCCESS;
            }

            base = &module->ident[i];
            goto matchfound;
        }
    }

    /* search submodules */
    for (j = 0; j < module->inc_size && module->inc[j].submodule; j++) {
        for (i = 0; i < module->inc[j].submodule->ident_size; i++) {
            if (!strcmp(basename, module->inc[j].submodule->ident[i].name)) {

                if (!ident) {
                    *ret = &module->inc[j].submodule->ident[i];
                    return EXIT_SUCCESS;
                }

                base = &module->inc[j].submodule->ident[i];
                goto matchfound;
            }
        }
    }

matchfound:
    /* we found it somewhere */
    if (base) {
        /* is it already completely resolved? */
        for (i = 0; i < unres->count; i++) {
            if ((unres->item[i] == base) && (unres->type[i] == UNRES_IDENT)) {
                /* identity found, but not yet resolved, so do not return it in *res and try it again later */

                /* simple check for circular reference,
                 * the complete check is done as a side effect of using only completely
                 * resolved identities (previous check of unres content) */
                if (ly_strequal((const char *)unres->str_snode[i], ident->name, 1)) {
                    LOGVAL(ctx, LYE_INARG, LY_VLOG_NONE, NULL, basename, "base");
                    LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Circular reference of \"%s\" identity.", basename);
                    return -1;
                }

                return EXIT_FAILURE;
            }
        }

        /* checks done, store the result */
        *ret = base;
        return EXIT_SUCCESS;
    }

    /* base not found (maybe a forward reference) */
    return EXIT_FAILURE;
}

/**
 * @brief Resolve base identity. Logs directly.
 *
 * @param[in] module Main module.
 * @param[in] ident Identity to use.
 * @param[in] basename Base name of the identity.
 * @param[in] parent Either "type" or "identity".
 * @param[in,out] type Type structure where we want to resolve identity. Can be NULL.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
resolve_base_ident(const struct lys_module *module, struct lys_ident *ident, const char *basename, const char *parent,
                   struct lys_type *type, struct unres_schema *unres)
{
    const char *name;
    int mod_name_len = 0, rc;
    struct lys_ident *target, **ret;
    uint16_t flags;
    struct lys_module *mod;
    struct ly_ctx *ctx = module->ctx;

    assert((ident && !type) || (!ident && type));

    if (!type) {
        /* have ident to resolve */
        ret = &target;
        flags = ident->flags;
        mod = ident->module;
    } else {
        /* have type to fill */
        ++type->info.ident.count;
        type->info.ident.ref = ly_realloc(type->info.ident.ref, type->info.ident.count * sizeof *type->info.ident.ref);
        LY_CHECK_ERR_RETURN(!type->info.ident.ref, LOGMEM(ctx), -1);

        ret = &type->info.ident.ref[type->info.ident.count - 1];
        flags = type->parent->flags;
        mod = type->parent->module;
    }
    *ret = NULL;

    /* search for the base identity */
    name = strchr(basename, ':');
    if (name) {
        /* set name to correct position after colon */
        mod_name_len = name - basename;
        name++;

        if (!strncmp(basename, module->name, mod_name_len) && !module->name[mod_name_len]) {
            /* prefix refers to the current module, ignore it */
            mod_name_len = 0;
        }
    } else {
        name = basename;
    }

    /* get module where to search */
    module = lyp_get_module(module, NULL, 0, mod_name_len ? basename : NULL, mod_name_len, 0);
    if (!module) {
        /* identity refers unknown data model */
        LOGVAL(ctx, LYE_INMOD, LY_VLOG_NONE, NULL, basename);
        return -1;
    }

    /* search in the identified module ... */
    rc = resolve_base_ident_sub(module, ident, name, unres, ret);
    if (!rc) {
        assert(*ret);

        /* check status */
        if (lyp_check_status(flags, mod, ident ? ident->name : "of type",
                             (*ret)->flags, (*ret)->module, (*ret)->name, NULL)) {
            rc = -1;
        } else if (ident) {
            ident->base[ident->base_size++] = *ret;
            if (lys_main_module(mod)->implemented) {
                /* in case of the implemented identity, maintain backlinks to it
                 * from the base identities to make it available when resolving
                 * data with the identity values (not implemented identity is not
                 * allowed as an identityref value). */
                resolve_identity_backlink_update(ident, *ret);
            }
        }
    } else if (rc == EXIT_FAILURE) {
        LOGVAL(ctx, LYE_INRESOLV, LY_VLOG_NONE, NULL, parent, basename);
        if (type) {
            --type->info.ident.count;
        }
    }

    return rc;
}

/*
 * 1 - true (der is derived from base)
 * 0 - false (der is not derived from base)
 */
static int
search_base_identity(struct lys_ident *der, struct lys_ident *base)
{
    int i;

    if (der == base) {
        return 1;
    } else {
        for(i = 0; i < der->base_size; i++) {
            if (search_base_identity(der->base[i], base) == 1) {
                return 1;
            }
        }
    }

    return 0;
}

/**
 * @brief Resolve JSON data format identityref. Logs directly.
 *
 * @param[in] type Identityref type.
 * @param[in] ident_name Identityref name.
 * @param[in] node Node where the identityref is being resolved
 * @param[in] dflt flag if we are resolving default value in the schema
 *
 * @return Pointer to the identity resolvent, NULL on error.
 */
struct lys_ident *
resolve_identref(struct lys_type *type, const char *ident_name, struct lyd_node *node, struct lys_module *mod, int dflt)
{
    const char *mod_name, *name;
    char *str;
    int mod_name_len, nam_len, rc;
    int need_implemented = 0;
    unsigned int i, j;
    struct lys_ident *der, *cur;
    struct lys_module *imod = NULL, *m, *tmod;
    struct ly_ctx *ctx;

    assert(type && ident_name && mod);
    ctx = mod->ctx;

    if (!type || (!type->info.ident.count && !type->der) || !ident_name) {
        return NULL;
    }

    rc = parse_node_identifier(ident_name, &mod_name, &mod_name_len, &name, &nam_len, NULL, 0);
    if (rc < 1) {
        LOGVAL(ctx, LYE_INCHAR, node ? LY_VLOG_LYD : LY_VLOG_NONE, node, ident_name[-rc], &ident_name[-rc]);
        return NULL;
    } else if (rc < (signed)strlen(ident_name)) {
        LOGVAL(ctx, LYE_INCHAR, node ? LY_VLOG_LYD : LY_VLOG_NONE, node, ident_name[rc], &ident_name[rc]);
        return NULL;
    }

    m = lys_main_module(mod); /* shortcut */
    if (!mod_name || (!strncmp(mod_name, m->name, mod_name_len) && !m->name[mod_name_len])) {
        /* identity is defined in the same module as node */
        imod = m;
    } else if (dflt) {
        /* solving identityref in default definition in schema -
         * find the identity's module in the imported modules list to have a correct revision */
        for (i = 0; i < mod->imp_size; i++) {
            if (!strncmp(mod_name, mod->imp[i].module->name, mod_name_len) && !mod->imp[i].module->name[mod_name_len]) {
                imod = mod->imp[i].module;
                break;
            }
        }

        /* We may need to pull it from the module that the typedef came from */
        if (!imod && type && type->der) {
            tmod = type->der->module;
            for (i = 0; i < tmod->imp_size; i++) {
                if (!strncmp(mod_name, tmod->imp[i].module->name, mod_name_len) && !tmod->imp[i].module->name[mod_name_len]) {
                    imod = tmod->imp[i].module;
                    break;
                }
            }
        }
    } else {
        /* solving identityref in data - get the module from the context */
        for (i = 0; i < (unsigned)mod->ctx->models.used; ++i) {
            imod = mod->ctx->models.list[i];
            if (!strncmp(mod_name, imod->name, mod_name_len) && !imod->name[mod_name_len]) {
                break;
            }
            imod = NULL;
        }
        if (!imod && mod->ctx->models.parsing_sub_modules_count) {
            /* we are currently parsing some module and checking XPath or a default value,
             * so take this module into account */
            for (i = 0; i < mod->ctx->models.parsing_sub_modules_count; i++) {
                imod = mod->ctx->models.parsing_sub_modules[i];
                if (imod->type) {
                    /* skip submodules */
                    continue;
                }
                if (!strncmp(mod_name, imod->name, mod_name_len) && !imod->name[mod_name_len]) {
                    break;
                }
                imod = NULL;
            }
        }
    }

    if (!dflt && (!imod || !imod->implemented) && ctx->data_clb) {
        /* the needed module was not found, but it may have been expected so call the data callback */
        if (imod) {
            ctx->data_clb(ctx, imod->name, imod->ns, LY_MODCLB_NOT_IMPLEMENTED, ctx->data_clb_data);
        } else if (mod_name) {
            str = strndup(mod_name, mod_name_len);
            imod = (struct lys_module *)ctx->data_clb(ctx, str, NULL, 0, ctx->data_clb_data);
            free(str);
        }
    }
    if (!imod) {
        goto fail;
    }

    if (m != imod || lys_main_module(type->parent->module) != mod) {
        /* the type is not referencing the same schema,
         * THEN, we may need to make the module with the identity implemented, but only if it really
         * contains the identity */
        if (!imod->implemented) {
            cur = NULL;
            /* get the identity in the module */
            for (i = 0; i < imod->ident_size; i++) {
                if (!strcmp(name, imod->ident[i].name)) {
                    cur = &imod->ident[i];
                    break;
                }
            }
            if (!cur) {
                /* go through includes */
                for (j = 0; j < imod->inc_size; j++) {
                    for (i = 0; i < imod->inc[j].submodule->ident_size; i++) {
                        if (!strcmp(name, imod->inc[j].submodule->ident[i].name)) {
                            cur = &imod->inc[j].submodule->ident[i];
                            break;
                        }
                    }
                }
                if (!cur) {
                    goto fail;
                }
            }

            /* check that identity is derived from one of the type's base */
            while (type->der) {
                for (i = 0; i < type->info.ident.count; i++) {
                    if (search_base_identity(cur, type->info.ident.ref[i])) {
                        /* cur's base matches the type's base */
                        need_implemented = 1;
                        goto match;
                    }
                }
                type = &type->der->type;
            }
            /* matching base not found */
            LOGVAL(ctx, LYE_SPEC, node ? LY_VLOG_LYD : LY_VLOG_NONE, node, "Identity used as identityref value is not implemented.");
            goto fail;
        }
    }

    /* go through all the derived types of all the bases */
    while (type->der) {
        for (i = 0; i < type->info.ident.count; ++i) {
            cur = type->info.ident.ref[i];

            if (cur->der) {
                /* there are some derived identities */
                for (j = 0; j < cur->der->number; j++) {
                    der = (struct lys_ident *)cur->der->set.g[j]; /* shortcut */
                    if (!strcmp(der->name, name) && lys_main_module(der->module) == imod) {
                        /* we have match */
                        cur = der;
                        goto match;
                    }
                }
            }
        }
        type = &type->der->type;
    }

fail:
    LOGVAL(ctx, LYE_INRESOLV, node ? LY_VLOG_LYD : LY_VLOG_NONE, node, "identityref", ident_name);
    return NULL;

match:
    for (i = 0; i < cur->iffeature_size; i++) {
        if (!resolve_iffeature(&cur->iffeature[i])) {
            if (node) {
                LOGVAL(ctx, LYE_INVAL, LY_VLOG_LYD, node, cur->name, node->schema->name);
            }
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL, "Identity \"%s\" is disabled by its if-feature condition.", cur->name);
            return NULL;
        }
    }
    if (need_implemented) {
        if (dflt) {
            /* later try to make the module implemented */
            LOGVRB("Making \"%s\" module implemented because of identityref default value \"%s\" used in the implemented \"%s\" module",
                   imod->name, cur->name, mod->name);
            /* to be more effective we should use UNRES_MOD_IMPLEMENT but that would require changing prototype of
             * several functions with little gain */
            if (lys_set_implemented(imod)) {
                LOGERR(ctx, ly_errno, "Setting the module \"%s\" implemented because of used default identity \"%s\" failed.",
                       imod->name, cur->name);
                goto fail;
            }
        } else {
            /* just say that it was found, but in a non-implemented module */
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Identity found, but in a non-implemented module \"%s\".",
                   lys_main_module(cur->module)->name);
            goto fail;
        }
    }
    return cur;
}

/**
 * @brief Resolve unresolved uses. Logs directly.
 *
 * @param[in] uses Uses to use.
 * @param[in] unres Specific unres item.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
resolve_unres_schema_uses(struct lys_node_uses *uses, struct unres_schema *unres)
{
    int rc;
    struct lys_node *par_grp;
    struct ly_ctx *ctx = uses->module->ctx;

    /* HACK: when a grouping has uses inside, all such uses have to be resolved before the grouping itself is used
     *       in some uses. When we see such a uses, the grouping's unres counter is used to store number of so far
     *       unresolved uses. The grouping cannot be used unless this counter is decreased back to 0. To remember
     *       that the uses already increased grouping's counter, the LYS_USESGRP flag is used. */
    for (par_grp = lys_parent((struct lys_node *)uses); par_grp && (par_grp->nodetype != LYS_GROUPING); par_grp = lys_parent(par_grp));
    if (par_grp && ly_strequal(par_grp->name, uses->name, 1)) {
        LOGVAL(ctx, LYE_INRESOLV, LY_VLOG_LYS, uses, "uses", uses->name);
        return -1;
    }

    if (!uses->grp) {
        rc = resolve_uses_schema_nodeid(uses->name, (const struct lys_node *)uses, (const struct lys_node_grp **)&uses->grp);
        if (rc == -1) {
            LOGVAL(ctx, LYE_INRESOLV, LY_VLOG_LYS, uses, "uses", uses->name);
            return -1;
        } else if (rc > 0) {
            LOGVAL(ctx, LYE_INCHAR, LY_VLOG_LYS, uses, uses->name[rc - 1], &uses->name[rc - 1]);
            return -1;
        } else if (!uses->grp) {
            if (par_grp && !(uses->flags & LYS_USESGRP)) {
                if (++((struct lys_node_grp *)par_grp)->unres_count == 0) {
                    LOGERR(ctx, LY_EINT, "Too many unresolved items (uses) inside a grouping.");
                    return -1;
                }
                uses->flags |= LYS_USESGRP;
            }
            LOGVAL(ctx, LYE_INRESOLV, LY_VLOG_LYS, uses, "uses", uses->name);
            return EXIT_FAILURE;
        }
    }

    if (uses->grp->unres_count) {
        if (par_grp && !(uses->flags & LYS_USESGRP)) {
            if (++((struct lys_node_grp *)par_grp)->unres_count == 0) {
                LOGERR(ctx, LY_EINT, "Too many unresolved items (uses) inside a grouping.");
                return -1;
            }
            uses->flags |= LYS_USESGRP;
        } else {
            /* instantiate grouping only when it is completely resolved */
            uses->grp = NULL;
        }
        LOGVAL(ctx, LYE_INRESOLV, LY_VLOG_LYS, uses, "uses", uses->name);
        return EXIT_FAILURE;
    }

    rc = resolve_uses(uses, unres);
    if (!rc) {
        /* decrease unres count only if not first try */
        if (par_grp && (uses->flags & LYS_USESGRP)) {
            assert(((struct lys_node_grp *)par_grp)->unres_count);
            ((struct lys_node_grp *)par_grp)->unres_count--;
            uses->flags &= ~LYS_USESGRP;
        }

        /* check status */
        if (lyp_check_status(uses->flags, uses->module, "of uses",
                         uses->grp->flags, uses->grp->module, uses->grp->name,
                         (struct lys_node *)uses)) {
            return -1;
        }

        return EXIT_SUCCESS;
    }

    return rc;
}

/**
 * @brief Resolve list keys. Logs directly.
 *
 * @param[in] list List to use.
 * @param[in] keys_str Keys node value.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
resolve_list_keys(struct lys_node_list *list, const char *keys_str)
{
    int i, len, rc;
    const char *value;
    char *s = NULL;
    struct ly_ctx *ctx = list->module->ctx;

    for (i = 0; i < list->keys_size; ++i) {
        assert(keys_str);

        if (!list->child) {
            /* no child, possible forward reference */
            LOGVAL(ctx, LYE_INRESOLV, LY_VLOG_LYS, list, "list keys", keys_str);
            return EXIT_FAILURE;
        }
        /* get the key name */
        if ((value = strpbrk(keys_str, " \t\n"))) {
            len = value - keys_str;
            while (isspace(value[0])) {
                value++;
            }
        } else {
            len = strlen(keys_str);
        }

        rc = lys_getnext_data(lys_node_module((struct lys_node *)list), (struct lys_node *)list, keys_str, len, LYS_LEAF,
                              (const struct lys_node **)&list->keys[i]);
        if (rc) {
            LOGVAL(ctx, LYE_INRESOLV, LY_VLOG_LYS, list, "list key", keys_str);
            return EXIT_FAILURE;
        }

        if (check_key(list, i, keys_str, len)) {
            /* check_key logs */
            return -1;
        }

        /* check status */
        if (lyp_check_status(list->flags, list->module, list->name,
                             list->keys[i]->flags, list->keys[i]->module, list->keys[i]->name,
                             (struct lys_node *)list->keys[i])) {
            return -1;
        }

        /* default value - is ignored, keep it but print a warning */
        if (list->keys[i]->dflt) {
            /* log is not hidden only in case this resolving fails and in such a case
             * we cannot get here
             */
            assert(log_opt == ILO_STORE);
            log_opt = ILO_LOG;
            LOGWRN(ctx, "Default value \"%s\" in the list key \"%s\" is ignored. (%s)", list->keys[i]->dflt,
                   list->keys[i]->name, s = lys_path((struct lys_node*)list, LYS_PATH_FIRST_PREFIX));
            log_opt = ILO_STORE;
            free(s);
        }

        /* prepare for next iteration */
        while (value && isspace(value[0])) {
            value++;
        }
        keys_str = value;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Resolve (check) all must conditions of \p node.
 * Logs directly.
 *
 * @param[in] node Data node with optional must statements.
 * @param[in] inout_parent If set, must in input or output parent of node->schema will be resolved.
 *
 * @return EXIT_SUCCESS on pass, EXIT_FAILURE on fail, -1 on error.
 */
static int
resolve_must(struct lyd_node *node, int inout_parent, int ignore_fail)
{
    uint8_t i, must_size;
    struct lys_node *schema;
    struct lys_restr *must;
    struct lyxp_set set;
    struct ly_ctx *ctx = node->schema->module->ctx;

    assert(node);
    memset(&set, 0, sizeof set);

    if (inout_parent) {
        for (schema = lys_parent(node->schema);
             schema && (schema->nodetype & (LYS_CHOICE | LYS_CASE | LYS_USES));
             schema = lys_parent(schema));
        if (!schema || !(schema->nodetype & (LYS_INPUT | LYS_OUTPUT))) {
            LOGINT(ctx);
            return -1;
        }
        must_size = ((struct lys_node_inout *)schema)->must_size;
        must = ((struct lys_node_inout *)schema)->must;

        /* context node is the RPC/action */
        node = node->parent;
        if (!(node->schema->nodetype & (LYS_RPC | LYS_ACTION))) {
            LOGINT(ctx);
            return -1;
        }
    } else {
        switch (node->schema->nodetype) {
        case LYS_CONTAINER:
            must_size = ((struct lys_node_container *)node->schema)->must_size;
            must = ((struct lys_node_container *)node->schema)->must;
            break;
        case LYS_LEAF:
            must_size = ((struct lys_node_leaf *)node->schema)->must_size;
            must = ((struct lys_node_leaf *)node->schema)->must;
            break;
        case LYS_LEAFLIST:
            must_size = ((struct lys_node_leaflist *)node->schema)->must_size;
            must = ((struct lys_node_leaflist *)node->schema)->must;
            break;
        case LYS_LIST:
            must_size = ((struct lys_node_list *)node->schema)->must_size;
            must = ((struct lys_node_list *)node->schema)->must;
            break;
        case LYS_ANYXML:
        case LYS_ANYDATA:
            must_size = ((struct lys_node_anydata *)node->schema)->must_size;
            must = ((struct lys_node_anydata *)node->schema)->must;
            break;
        case LYS_NOTIF:
            must_size = ((struct lys_node_notif *)node->schema)->must_size;
            must = ((struct lys_node_notif *)node->schema)->must;
            break;
        default:
            must_size = 0;
            break;
        }
    }

    for (i = 0; i < must_size; ++i) {
        if (lyxp_eval(must[i].expr, node, LYXP_NODE_ELEM, lyd_node_module(node), &set, LYXP_MUST)) {
            return -1;
        }

        lyxp_set_cast(&set, LYXP_SET_BOOLEAN, node, lyd_node_module(node), LYXP_MUST);

        if (!set.val.bool) {
            if ((ignore_fail == 1) || ((must[i].flags & (LYS_XPCONF_DEP | LYS_XPSTATE_DEP)) && (ignore_fail == 2))) {
                LOGVRB("Must condition \"%s\" not satisfied, but it is not required.", must[i].expr);
            } else {
                LOGVAL(ctx, LYE_NOMUST, LY_VLOG_LYD, node, must[i].expr);
                if (must[i].emsg) {
                    ly_vlog_str(ctx, LY_VLOG_PREV, must[i].emsg);
                }
                if (must[i].eapptag) {
                    ly_err_last_set_apptag(ctx, must[i].eapptag);
                }
                return 1;
            }
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Resolve (find) when condition schema context node. Does not log.
 *
 * @param[in] schema Schema node with the when condition.
 * @param[out] ctx_snode When schema context node.
 * @param[out] ctx_snode_type Schema context node type.
 */
void
resolve_when_ctx_snode(const struct lys_node *schema, struct lys_node **ctx_snode, enum lyxp_node_type *ctx_snode_type)
{
    const struct lys_node *sparent;

    /* find a not schema-only node */
    *ctx_snode_type = LYXP_NODE_ELEM;
    while (schema->nodetype & (LYS_USES | LYS_CHOICE | LYS_CASE | LYS_AUGMENT | LYS_INPUT | LYS_OUTPUT)) {
        if (schema->nodetype == LYS_AUGMENT) {
            sparent = ((struct lys_node_augment *)schema)->target;
        } else {
            sparent = schema->parent;
        }
        if (!sparent) {
            /* context node is the document root (fake root in our case) */
            if (schema->flags & LYS_CONFIG_W) {
                *ctx_snode_type = LYXP_NODE_ROOT_CONFIG;
            } else {
                *ctx_snode_type = LYXP_NODE_ROOT;
            }
            /* we need the first top-level sibling, but no uses or groupings */
            schema = lys_getnext(NULL, NULL, lys_node_module(schema), LYS_GETNEXT_NOSTATECHECK);
            break;
        }
        schema = sparent;
    }

    *ctx_snode = (struct lys_node *)schema;
}

/**
 * @brief Resolve (find) when condition context node. Does not log.
 *
 * @param[in] node Data node, whose conditional definition is being decided.
 * @param[in] schema Schema node with the when condition.
 * @param[out] ctx_node Context node.
 * @param[out] ctx_node_type Context node type.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
resolve_when_ctx_node(struct lyd_node *node, struct lys_node *schema, struct lyd_node **ctx_node,
                      enum lyxp_node_type *ctx_node_type)
{
    struct lyd_node *parent;
    struct lys_node *sparent;
    enum lyxp_node_type node_type;
    uint16_t i, data_depth, schema_depth;

    resolve_when_ctx_snode(schema, &schema, &node_type);

    if (node_type == LYXP_NODE_ELEM) {
        /* standard element context node */
        for (parent = node, data_depth = 0; parent; parent = parent->parent, ++data_depth);
        for (sparent = schema, schema_depth = 0;
                sparent;
                sparent = (sparent->nodetype == LYS_AUGMENT ? ((struct lys_node_augment *)sparent)->target : sparent->parent)) {
            if (sparent->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA | LYS_NOTIF | LYS_RPC)) {
                ++schema_depth;
            }
        }
        if (data_depth < schema_depth) {
            return -1;
        }

        /* find the corresponding data node */
        for (i = 0; i < data_depth - schema_depth; ++i) {
            node = node->parent;
        }
        if (node->schema != schema) {
            return -1;
        }
    } else {
        /* root context node */
        while (node->parent) {
            node = node->parent;
        }
        while (node->prev->next) {
            node = node->prev;
        }
    }

    *ctx_node = node;
    *ctx_node_type = node_type;
    return EXIT_SUCCESS;
}

/**
 * @brief Temporarily unlink nodes as per YANG 1.1 RFC section 7.21.5 for when XPath evaluation.
 * The context node is adjusted if needed.
 *
 * @param[in] snode Schema node, whose children instances need to be unlinked.
 * @param[in,out] node Data siblings where to look for the children of \p snode. If it is unlinked,
 * it is moved to point to another sibling still in the original tree.
 * @param[in,out] ctx_node When context node, adjusted if needed.
 * @param[in] ctx_node_type Context node type, just for information to detect invalid situations.
 * @param[out] unlinked_nodes Unlinked siblings. Can be safely appended to \p node afterwards.
 * Ordering may change, but there will be no semantic change.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
resolve_when_unlink_nodes(struct lys_node *snode, struct lyd_node **node, struct lyd_node **ctx_node,
                          enum lyxp_node_type ctx_node_type, struct lyd_node **unlinked_nodes)
{
    struct lyd_node *next, *elem;
    const struct lys_node *slast;
    struct ly_ctx *ctx = snode->module->ctx;

    switch (snode->nodetype) {
    case LYS_AUGMENT:
    case LYS_USES:
    case LYS_CHOICE:
    case LYS_CASE:
        slast = NULL;
        while ((slast = lys_getnext(slast, snode, NULL, LYS_GETNEXT_PARENTUSES))) {
            if (slast->nodetype & (LYS_ACTION | LYS_NOTIF)) {
                continue;
            }

            if (resolve_when_unlink_nodes((struct lys_node *)slast, node, ctx_node, ctx_node_type, unlinked_nodes)) {
                return -1;
            }
        }
        break;
    case LYS_CONTAINER:
    case LYS_LIST:
    case LYS_LEAF:
    case LYS_LEAFLIST:
    case LYS_ANYXML:
    case LYS_ANYDATA:
        LY_TREE_FOR_SAFE(lyd_first_sibling(*node), next, elem) {
            if (elem->schema == snode) {

                if (elem == *ctx_node) {
                    /* We are going to unlink our context node! This normally cannot happen,
                     * but we use normal top-level data nodes for faking a document root node,
                     * so if this is the context node, we just use the next top-level node.
                     * Additionally, it can even happen that there are no top-level data nodes left,
                     * all were unlinked, so in this case we pass NULL as the context node/data tree,
                     * lyxp_eval() can handle this special situation.
                     */
                    if (ctx_node_type == LYXP_NODE_ELEM) {
                        LOGINT(ctx);
                        return -1;
                    }

                    if (elem->prev == elem) {
                        /* unlinking last top-level element, use an empty data tree */
                        *ctx_node = NULL;
                    } else {
                        /* in this case just use the previous/last top-level data node */
                        *ctx_node = elem->prev;
                    }
                } else if (elem == *node) {
                    /* We are going to unlink the currently processed node. This does not matter that
                     * much, but we would lose access to the original data tree, so just move our
                     * pointer somewhere still inside it.
                     */
                    if ((*node)->prev != *node) {
                        *node = (*node)->prev;
                    } else {
                        /* the processed node with sibings were all unlinked, oh well */
                        *node = NULL;
                    }
                }

                /* temporarily unlink the node */
                lyd_unlink_internal(elem, 0);
                if (*unlinked_nodes) {
                    if (lyd_insert_after((*unlinked_nodes)->prev, elem)) {
                        LOGINT(ctx);
                        return -1;
                    }
                } else {
                    *unlinked_nodes = elem;
                }

                if (snode->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_ANYDATA)) {
                    /* there can be only one instance */
                    break;
                }
            }
        }
        break;
    default:
        LOGINT(ctx);
        return -1;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Relink the unlinked nodes back.
 *
 * @param[in] node Data node to link the nodes back to. It can actually be the adjusted context node,
 * we simply need a sibling from the original data tree.
 * @param[in] unlinked_nodes Unlinked nodes to relink to \p node.
 * @param[in] ctx_node_type Context node type to distinguish between \p node being the parent
 * or the sibling of \p unlinked_nodes.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
resolve_when_relink_nodes(struct lyd_node *node, struct lyd_node *unlinked_nodes, enum lyxp_node_type ctx_node_type)
{
    struct lyd_node *elem;

    LY_TREE_FOR_SAFE(unlinked_nodes, unlinked_nodes, elem) {
        lyd_unlink_internal(elem, 0);
        if (ctx_node_type == LYXP_NODE_ELEM) {
            if (lyd_insert_common(node, NULL, elem, 0)) {
                return -1;
            }
        } else {
            if (lyd_insert_nextto(node, elem, 0, 0)) {
                return -1;
            }
        }
    }

    return EXIT_SUCCESS;
}

int
resolve_applies_must(const struct lyd_node *node)
{
    int ret = 0;
    uint8_t must_size;
    struct lys_node *schema, *iter;

    assert(node);

    schema = node->schema;

    /* their own must */
    switch (schema->nodetype) {
    case LYS_CONTAINER:
        must_size = ((struct lys_node_container *)schema)->must_size;
        break;
    case LYS_LEAF:
        must_size = ((struct lys_node_leaf *)schema)->must_size;
        break;
    case LYS_LEAFLIST:
        must_size = ((struct lys_node_leaflist *)schema)->must_size;
        break;
    case LYS_LIST:
        must_size = ((struct lys_node_list *)schema)->must_size;
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        must_size = ((struct lys_node_anydata *)schema)->must_size;
        break;
    case LYS_NOTIF:
        must_size = ((struct lys_node_notif *)schema)->must_size;
        break;
    default:
        must_size = 0;
        break;
    }

    if (must_size) {
        ++ret;
    }

    /* schema may be a direct data child of input/output with must (but it must be first, it needs to be evaluated only once) */
    if (!node->prev->next) {
        for (iter = lys_parent(schema); iter && (iter->nodetype & (LYS_CHOICE | LYS_CASE | LYS_USES)); iter = lys_parent(iter));
        if (iter && (iter->nodetype & (LYS_INPUT | LYS_OUTPUT))) {
            ret += 0x2;
        }
    }

    return ret;
}

static struct lys_when *
snode_get_when(const struct lys_node *schema)
{
    switch (schema->nodetype) {
    case LYS_CONTAINER:
        return ((struct lys_node_container *)schema)->when;
    case LYS_CHOICE:
        return ((struct lys_node_choice *)schema)->when;
    case LYS_LEAF:
        return ((struct lys_node_leaf *)schema)->when;
    case LYS_LEAFLIST:
        return ((struct lys_node_leaflist *)schema)->when;
    case LYS_LIST:
        return ((struct lys_node_list *)schema)->when;
    case LYS_ANYDATA:
    case LYS_ANYXML:
        return ((struct lys_node_anydata *)schema)->when;
    case LYS_CASE:
        return ((struct lys_node_case *)schema)->when;
    case LYS_USES:
        return ((struct lys_node_uses *)schema)->when;
    case LYS_AUGMENT:
        return ((struct lys_node_augment *)schema)->when;
    default:
        return NULL;
    }
}

int
resolve_applies_when(const struct lys_node *schema, int mode, const struct lys_node *stop)
{
    const struct lys_node *parent;

    assert(schema);

    if (!(schema->nodetype & (LYS_NOTIF | LYS_RPC)) && snode_get_when(schema)) {
        return 1;
    }

    parent = schema;
    goto check_augment;

    while (parent) {
        /* stop conditions */
        if (!mode) {
            /* stop on node that can be instantiated in data tree */
            if (!(parent->nodetype & (LYS_USES | LYS_CHOICE | LYS_CASE))) {
                break;
            }
        } else {
            /* stop on the specified node */
            if (parent == stop) {
                break;
            }
        }

        if (snode_get_when(parent)) {
            return 1;
        }
check_augment:

        if (parent->parent && (parent->parent->nodetype == LYS_AUGMENT) && snode_get_when(parent->parent)) {
            return 1;
        }
        parent = lys_parent(parent);
    }

    return 0;
}

/**
 * @brief Resolve (check) all when conditions relevant for \p node.
 * Logs directly.
 *
 * @param[in] node Data node, whose conditional reference, if such, is being decided.
 * @param[in] ignore_fail 1 if when does not have to be satisfied, 2 if it does not have to be satisfied
 * only when requiring external dependencies.
 *
 * @return
 *  -1 - error, ly_errno is set
 *   0 - all "when" statements true
 *   0, ly_vecode = LYVE_NOWHEN - some "when" statement false, returned in failed_when
 *   1, ly_vecode = LYVE_INWHEN - nodes needed to resolve are conditional and not yet resolved (under another "when")
 */
int
resolve_when(struct lyd_node *node, int ignore_fail, struct lys_when **failed_when)
{
    struct lyd_node *ctx_node = NULL, *unlinked_nodes, *tmp_node;
    struct lys_node *sparent;
    struct lyxp_set set;
    enum lyxp_node_type ctx_node_type;
    struct ly_ctx *ctx = node->schema->module->ctx;
    int rc = 0;

    assert(node);
    memset(&set, 0, sizeof set);

    if (!(node->schema->nodetype & (LYS_NOTIF | LYS_RPC | LYS_ACTION)) && snode_get_when(node->schema)) {
        /* make the node dummy for the evaluation */
        node->validity |= LYD_VAL_INUSE;
        rc = lyxp_eval(snode_get_when(node->schema)->cond, node, LYXP_NODE_ELEM, lyd_node_module(node),
                       &set, LYXP_WHEN);
        node->validity &= ~LYD_VAL_INUSE;
        if (rc) {
            if (rc == 1) {
                LOGVAL(ctx, LYE_INWHEN, LY_VLOG_LYD, node, snode_get_when(node->schema)->cond);
            }
            goto cleanup;
        }

        /* set boolean result of the condition */
        lyxp_set_cast(&set, LYXP_SET_BOOLEAN, node, lyd_node_module(node), LYXP_WHEN);
        if (!set.val.bool) {
            node->when_status |= LYD_WHEN_FALSE;
            if ((ignore_fail == 1) || ((snode_get_when(node->schema)->flags & (LYS_XPCONF_DEP | LYS_XPSTATE_DEP))
                    && (ignore_fail == 2))) {
                LOGVRB("When condition \"%s\" is not satisfied, but it is not required.", snode_get_when(node->schema)->cond);
            } else {
                LOGVAL(ctx, LYE_NOWHEN, LY_VLOG_LYD, node, snode_get_when(node->schema)->cond);
                if (failed_when) {
                    *failed_when = snode_get_when(node->schema);
                }
                goto cleanup;
            }
        }

        /* free xpath set content */
        lyxp_set_cast(&set, LYXP_SET_EMPTY, node, lyd_node_module(node), 0);
    }

    sparent = node->schema;
    goto check_augment;

    /* check when in every schema node that affects node */
    while (sparent && (sparent->nodetype & (LYS_USES | LYS_CHOICE | LYS_CASE))) {
        if (snode_get_when(sparent)) {
            if (!ctx_node) {
                rc = resolve_when_ctx_node(node, sparent, &ctx_node, &ctx_node_type);
                if (rc) {
                    LOGINT(ctx);
                    goto cleanup;
                }
            }

            unlinked_nodes = NULL;
            /* we do not want our node pointer to change */
            tmp_node = node;
            rc = resolve_when_unlink_nodes(sparent, &tmp_node, &ctx_node, ctx_node_type, &unlinked_nodes);
            if (rc) {
                goto cleanup;
            }

            rc = lyxp_eval(snode_get_when(sparent)->cond, ctx_node, ctx_node_type, lys_node_module(sparent),
                           &set, LYXP_WHEN);

            if (unlinked_nodes && ctx_node) {
                if (resolve_when_relink_nodes(ctx_node, unlinked_nodes, ctx_node_type)) {
                    rc = -1;
                    goto cleanup;
                }
            }

            if (rc) {
                if (rc == 1) {
                    LOGVAL(ctx, LYE_INWHEN, LY_VLOG_LYD, node, snode_get_when(sparent)->cond);
                }
                goto cleanup;
            }

            lyxp_set_cast(&set, LYXP_SET_BOOLEAN, ctx_node, lys_node_module(sparent), LYXP_WHEN);
            if (!set.val.bool) {
                if ((ignore_fail == 1) || ((snode_get_when(sparent)->flags & (LYS_XPCONF_DEP | LYS_XPSTATE_DEP))
                        && (ignore_fail == 2))) {
                    LOGVRB("When condition \"%s\" is not satisfied, but it is not required.", snode_get_when(sparent)->cond);
                } else {
                    node->when_status |= LYD_WHEN_FALSE;
                    LOGVAL(ctx, LYE_NOWHEN, LY_VLOG_LYD, node, snode_get_when(sparent)->cond);
                    if (failed_when) {
                        *failed_when = snode_get_when(sparent);
                    }
                    goto cleanup;
                }
            }

            /* free xpath set content */
            lyxp_set_cast(&set, LYXP_SET_EMPTY, ctx_node, lys_node_module(sparent), 0);
        }

check_augment:
        if ((sparent->parent && (sparent->parent->nodetype == LYS_AUGMENT) && snode_get_when(sparent->parent))) {
            if (!ctx_node) {
                rc = resolve_when_ctx_node(node, sparent->parent, &ctx_node, &ctx_node_type);
                if (rc) {
                    LOGINT(ctx);
                    goto cleanup;
                }
            }

            unlinked_nodes = NULL;
            tmp_node = node;
            rc = resolve_when_unlink_nodes(sparent->parent, &tmp_node, &ctx_node, ctx_node_type, &unlinked_nodes);
            if (rc) {
                goto cleanup;
            }

            rc = lyxp_eval(snode_get_when(sparent->parent)->cond, ctx_node, ctx_node_type,
                           lys_node_module(sparent->parent), &set, LYXP_WHEN);

            /* reconnect nodes, if ctx_node is NULL then all the nodes were unlinked, but linked together,
             * so the tree did not actually change and there is nothing for us to do
             */
            if (unlinked_nodes && ctx_node) {
                if (resolve_when_relink_nodes(ctx_node, unlinked_nodes, ctx_node_type)) {
                    rc = -1;
                    goto cleanup;
                }
            }

            if (rc) {
                if (rc == 1) {
                    LOGVAL(ctx, LYE_INWHEN, LY_VLOG_LYD, node, snode_get_when(sparent->parent)->cond);
                }
                goto cleanup;
            }

            lyxp_set_cast(&set, LYXP_SET_BOOLEAN, ctx_node, lys_node_module(sparent->parent), LYXP_WHEN);
            if (!set.val.bool) {
                node->when_status |= LYD_WHEN_FALSE;
                if ((ignore_fail == 1) || ((snode_get_when(sparent->parent)->flags & (LYS_XPCONF_DEP | LYS_XPSTATE_DEP))
                        && (ignore_fail == 2))) {
                    LOGVRB("When condition \"%s\" is not satisfied, but it is not required.",
                           snode_get_when(sparent->parent)->cond);
                } else {
                    LOGVAL(ctx, LYE_NOWHEN, LY_VLOG_LYD, node, snode_get_when(sparent->parent)->cond);
                    if (failed_when) {
                        *failed_when = snode_get_when(sparent->parent);
                    }
                    goto cleanup;
                }
            }

            /* free xpath set content */
            lyxp_set_cast(&set, LYXP_SET_EMPTY, ctx_node, lys_node_module(sparent->parent), 0);
        }

        sparent = lys_parent(sparent);
    }

    node->when_status |= LYD_WHEN_TRUE;

cleanup:
    /* free xpath set content */
    lyxp_set_cast(&set, LYXP_SET_EMPTY, ctx_node ? ctx_node : node, NULL, 0);
    return rc;
}

static int
check_type_union_leafref(struct lys_type *type)
{
    uint8_t i;

    if ((type->base == LY_TYPE_UNION) && type->info.uni.count) {
        /* go through unions and look for leafref */
        for (i = 0; i < type->info.uni.count; ++i) {
            switch (type->info.uni.types[i].base) {
            case LY_TYPE_LEAFREF:
                return 1;
            case LY_TYPE_UNION:
                if (check_type_union_leafref(&type->info.uni.types[i])) {
                    return 1;
                }
                break;
            default:
                break;
            }
        }

        return 0;
    }

    /* just inherit the flag value */
    return type->der->has_union_leafref;
}

/**
 * @brief Resolve a single unres schema item. Logs indirectly.
 *
 * @param[in] mod Main module.
 * @param[in] item Item to resolve. Type determined by \p type.
 * @param[in] type Type of the unresolved item.
 * @param[in] str_snode String, a schema node, or NULL.
 * @param[in] unres Unres schema structure to use.
 * @param[in] final_fail Whether we are just printing errors of the failed unres items.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
resolve_unres_schema_item(struct lys_module *mod, void *item, enum UNRES_ITEM type, void *str_snode,
                          struct unres_schema *unres)
{
    /* has_str - whether the str_snode is a string in a dictionary that needs to be freed */
    int rc = -1, has_str = 0, parent_type = 0, i, k;
    unsigned int j;
    struct ly_ctx * ctx = mod->ctx;
    struct lys_node *root, *next, *node, *par_grp;
    const char *expr;
    uint8_t *u;

    struct ly_set *refs, *procs;
    struct lys_feature *ref, *feat;
    struct lys_ident *ident;
    struct lys_type *stype;
    struct lys_node_choice *choic;
    struct lyxml_elem *yin;
    struct yang_type *yang;
    struct unres_list_uniq *unique_info;
    struct unres_iffeat_data *iff_data;
    struct unres_ext *ext_data;
    struct lys_ext_instance *ext, **extlist;
    struct lyext_plugin *eplugin;

    switch (type) {
    case UNRES_IDENT:
        expr = str_snode;
        has_str = 1;
        ident = item;

        rc = resolve_base_ident(mod, ident, expr, "identity", NULL, unres);
        break;
    case UNRES_TYPE_IDENTREF:
        expr = str_snode;
        has_str = 1;
        stype = item;

        rc = resolve_base_ident(mod, NULL, expr, "type", stype, unres);
        break;
    case UNRES_TYPE_LEAFREF:
        node = str_snode;
        stype = item;

        rc = resolve_schema_leafref(stype, node, unres);
        break;
    case UNRES_TYPE_DER_EXT:
        parent_type++;
        /* falls through */
    case UNRES_TYPE_DER_TPDF:
        parent_type++;
        /* falls through */
    case UNRES_TYPE_DER:
        /* parent */
        node = str_snode;
        stype = item;

        /* HACK type->der is temporarily unparsed type statement */
        yin = (struct lyxml_elem *)stype->der;
        stype->der = NULL;

        if (yin->flags & LY_YANG_STRUCTURE_FLAG) {
            yang = (struct yang_type *)yin;
            rc = yang_check_type(mod, node, yang, stype, parent_type, unres);

            if (rc) {
                /* may try again later */
                stype->der = (struct lys_tpdf *)yang;
            } else {
                /* we need to always be able to free this, it's safe only in this case */
                lydict_remove(ctx, yang->name);
                free(yang);
            }

        } else {
            rc = fill_yin_type(mod, node, yin, stype, parent_type, unres);
            if (!rc || rc == -1) {
                /* we need to always be able to free this, it's safe only in this case */
                lyxml_free(ctx, yin);
            } else {
                /* may try again later, put all back how it was */
                stype->der = (struct lys_tpdf *)yin;
            }
        }
        if (rc == EXIT_SUCCESS) {
            /* it does not make sense to have leaf-list of empty type */
            if (!parent_type && node->nodetype == LYS_LEAFLIST && stype->base == LY_TYPE_EMPTY) {
                LOGWRN(ctx, "The leaf-list \"%s\" is of \"empty\" type, which does not make sense.", node->name);
            }

            if ((type == UNRES_TYPE_DER_TPDF) && (stype->base == LY_TYPE_UNION)) {
                /* fill typedef union leafref flag */
                ((struct lys_tpdf *)stype->parent)->has_union_leafref = check_type_union_leafref(stype);
            } else if ((type == UNRES_TYPE_DER) && stype->der->has_union_leafref) {
                /* copy the type in case it has union leafref flag */
                if (lys_copy_union_leafrefs(mod, node, stype, NULL, unres)) {
                    LOGERR(ctx, LY_EINT, "Failed to duplicate type.");
                    return -1;
                }
            }
        } else if (rc == EXIT_FAILURE && !(stype->value_flags & LY_VALUE_UNRESGRP)) {
            /* forward reference - in case the type is in grouping, we have to make the grouping unusable
             * by uses statement until the type is resolved. We do that the same way as uses statements inside
             * grouping. The grouping cannot be used unless the unres counter is 0.
             * To remember that the grouping already increased the counter, the LYTYPE_GRP is used as value
             * of the type's base member. */
            for (par_grp = node; par_grp && (par_grp->nodetype != LYS_GROUPING); par_grp = lys_parent(par_grp));
            if (par_grp) {
                if (++((struct lys_node_grp *)par_grp)->unres_count == 0) {
                    LOGERR(ctx, LY_EINT, "Too many unresolved items (type) inside a grouping.");
                    return -1;
                }
                stype->value_flags |= LY_VALUE_UNRESGRP;
            }
        }
        break;
    case UNRES_IFFEAT:
        iff_data = str_snode;
        rc = resolve_feature(iff_data->fname, strlen(iff_data->fname), iff_data->node, item);
        if (!rc) {
            /* success */
            if (iff_data->infeature) {
                /* store backlink into the target feature to allow reverse changes in case of changing feature status */
                feat = *((struct lys_feature **)item);
                if (!feat->depfeatures) {
                    feat->depfeatures = ly_set_new();
                }
                ly_set_add(feat->depfeatures, iff_data->node, LY_SET_OPT_USEASLIST);
            }
            /* cleanup temporary data */
            lydict_remove(ctx, iff_data->fname);
            free(iff_data);
        }
        break;
    case UNRES_FEATURE:
        feat = (struct lys_feature *)item;

        if (feat->iffeature_size) {
            refs = ly_set_new();
            procs = ly_set_new();
            ly_set_add(procs, feat, 0);

            while (procs->number) {
                ref = procs->set.g[procs->number - 1];
                ly_set_rm_index(procs, procs->number - 1);

                for (i = 0; i < ref->iffeature_size; i++) {
                    resolve_iffeature_getsizes(&ref->iffeature[i], NULL, &j);
                    for (; j > 0 ; j--) {
                        if (ref->iffeature[i].features[j - 1]) {
                            if (ref->iffeature[i].features[j - 1] == feat) {
                                LOGVAL(ctx, LYE_CIRC_FEATURES, LY_VLOG_NONE, NULL, feat->name);
                                goto featurecheckdone;
                            }

                            if (ref->iffeature[i].features[j - 1]->iffeature_size) {
                                k = refs->number;
                                if (ly_set_add(refs, ref->iffeature[i].features[j - 1], 0) == k) {
                                    /* not yet seen feature, add it for processing */
                                    ly_set_add(procs, ref->iffeature[i].features[j - 1], 0);
                                }
                            }
                        } else {
                            /* forward reference */
                            rc = EXIT_FAILURE;
                            goto featurecheckdone;
                        }
                    }

                }
            }
            rc = EXIT_SUCCESS;

featurecheckdone:
            ly_set_free(refs);
            ly_set_free(procs);
        }

        break;
    case UNRES_USES:
        rc = resolve_unres_schema_uses(item, unres);
        break;
    case UNRES_TYPEDEF_DFLT:
        parent_type++;
        /* falls through */
    case UNRES_TYPE_DFLT:
        stype = item;
        rc = check_default(stype, (const char **)str_snode, mod, parent_type);
        if ((rc == EXIT_FAILURE) && !parent_type && (stype->base == LY_TYPE_LEAFREF)) {
            for (par_grp = (struct lys_node *)stype->parent;
                 par_grp && (par_grp->nodetype != LYS_GROUPING);
                 par_grp = lys_parent(par_grp));
            if (par_grp) {
                /* checking default value in a grouping finished with forward reference means we cannot check the value */
                rc = EXIT_SUCCESS;
            }
        }
        break;
    case UNRES_CHOICE_DFLT:
        expr = str_snode;
        has_str = 1;
        choic = item;

        if (!choic->dflt) {
            choic->dflt = resolve_choice_dflt(choic, expr);
        }
        if (choic->dflt) {
            rc = lyp_check_mandatory_choice((struct lys_node *)choic);
        } else {
            rc = EXIT_FAILURE;
        }
        break;
    case UNRES_LIST_KEYS:
        rc = resolve_list_keys(item, ((struct lys_node_list *)item)->keys_str);
        break;
    case UNRES_LIST_UNIQ:
        unique_info = (struct unres_list_uniq *)item;
        rc = resolve_unique(unique_info->list, unique_info->expr, unique_info->trg_type);
        break;
    case UNRES_AUGMENT:
        rc = resolve_augment(item, NULL, unres);
        break;
    case UNRES_XPATH:
        node = (struct lys_node *)item;
        rc = check_xpath(node, 1);
        break;
    case UNRES_MOD_IMPLEMENT:
        rc = lys_make_implemented_r(mod, unres);
        break;
    case UNRES_EXT:
        ext_data = (struct unres_ext *)str_snode;
        extlist = &(*(struct lys_ext_instance ***)item)[ext_data->ext_index];
        rc = resolve_extension(ext_data, extlist, unres);
        if (!rc) {
            /* success */
            /* is there a callback to be done to finalize the extension? */
            eplugin = extlist[0]->def->plugin;
            if (eplugin) {
                if (eplugin->check_result || (eplugin->flags & LYEXT_OPT_INHERIT)) {
                    u = malloc(sizeof *u);
                    LY_CHECK_ERR_RETURN(!u, LOGMEM(ctx), -1);
                    (*u) = ext_data->ext_index;
                    if (unres_schema_add_node(mod, unres, item, UNRES_EXT_FINALIZE, (struct lys_node *)u) == -1) {
                        /* something really bad happend since the extension finalization is not actually
                         * being resolved while adding into unres, so something more serious with the unres
                         * list itself must happened */
                        return -1;
                    }
                }
            }
        }
        if (!rc || rc == -1) {
            /* cleanup on success or fatal error */
            if (ext_data->datatype == LYS_IN_YIN) {
                /* YIN */
                lyxml_free(ctx, ext_data->data.yin);
            } else {
                /* YANG */
                yang_free_ext_data(ext_data->data.yang);
            }
            free(ext_data);
        }
        break;
    case UNRES_EXT_FINALIZE:
        u = (uint8_t *)str_snode;
        ext = (*(struct lys_ext_instance ***)item)[*u];
        free(u);

        eplugin = ext->def->plugin;

        /* inherit */
        if ((eplugin->flags & LYEXT_OPT_INHERIT) && (ext->parent_type == LYEXT_PAR_NODE)) {
            root = (struct lys_node *)ext->parent;
            if (!(root->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA))) {
                LY_TREE_DFS_BEGIN(root->child, next, node) {
                    /* first, check if the node already contain instance of the same extension,
                     * in such a case we won't inherit. In case the node was actually defined as
                     * augment data, we are supposed to check the same way also the augment node itself */
                    if (lys_ext_instance_presence(ext->def, node->ext, node->ext_size) != -1) {
                        goto inherit_dfs_sibling;
                    } else if (node->parent != root && node->parent->nodetype == LYS_AUGMENT &&
                            lys_ext_instance_presence(ext->def, node->parent->ext, node->parent->ext_size) != -1) {
                        goto inherit_dfs_sibling;
                    }

                    if (eplugin->check_inherit) {
                        /* we have a callback to check the inheritance, use it */
                        switch ((rc = (*eplugin->check_inherit)(ext, node))) {
                        case 0:
                            /* yes - continue with the inheriting code */
                            break;
                        case 1:
                            /* no - continue with the node's sibling */
                            goto inherit_dfs_sibling;
                        case 2:
                            /* no, but continue with the children, just skip the inheriting code for this node */
                            goto inherit_dfs_child;
                        default:
                            LOGERR(ctx, LY_EINT, "Plugin's (%s:%s) check_inherit callback returns invalid value (%d),",
                                   ext->def->module->name, ext->def->name, rc);
                        }
                    }

                    /* inherit the extension */
                    extlist = realloc(node->ext, (node->ext_size + 1) * sizeof *node->ext);
                    LY_CHECK_ERR_RETURN(!extlist, LOGMEM(ctx), -1);
                    extlist[node->ext_size] = malloc(sizeof **extlist);
                    LY_CHECK_ERR_RETURN(!extlist[node->ext_size], LOGMEM(ctx); node->ext = extlist, -1);
                    memcpy(extlist[node->ext_size], ext, sizeof *ext);
                    extlist[node->ext_size]->flags |= LYEXT_OPT_INHERIT;

                    node->ext = extlist;
                    node->ext_size++;

inherit_dfs_child:
                    /* modification of - select element for the next run - children first */
                    if (node->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) {
                        next = NULL;
                    } else {
                        next = node->child;
                    }
                    if (!next) {
inherit_dfs_sibling:
                        /* no children, try siblings */
                        next = node->next;
                    }
                    while (!next) {
                        /* go to the parent */
                        node = lys_parent(node);

                        /* we are done if we are back in the root (the starter's parent */
                        if (node == root) {
                            break;
                        }

                        /* parent is already processed, go to its sibling */
                        next = node->next;
                    }
                }
            }
        }

        /* final check */
        if (eplugin->check_result) {
            if ((*eplugin->check_result)(ext)) {
                LOGERR(ctx, LY_EPLUGIN, "Resolving extension failed.");
                return -1;
            }
        }

        rc = 0;
        break;
    default:
        LOGINT(ctx);
        break;
    }

    if (has_str && !rc) {
        /* the string is no more needed in case of success.
         * In case of forward reference, we will try to resolve the string later */
        lydict_remove(ctx, str_snode);
    }

    return rc;
}

/* logs directly */
static void
print_unres_schema_item_fail(void *item, enum UNRES_ITEM type, void *str_node)
{
    struct lyxml_elem *xml;
    struct lyxml_attr *attr;
    struct unres_iffeat_data *iff_data;
    const char *name = NULL;
    struct unres_ext *extinfo;

    switch (type) {
    case UNRES_IDENT:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later.", "identity", (char *)str_node);
        break;
    case UNRES_TYPE_IDENTREF:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later.", "identityref", (char *)str_node);
        break;
    case UNRES_TYPE_LEAFREF:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later.", "leafref",
               ((struct lys_type *)item)->info.lref.path);
        break;
    case UNRES_TYPE_DER_EXT:
    case UNRES_TYPE_DER_TPDF:
    case UNRES_TYPE_DER:
        xml = (struct lyxml_elem *)((struct lys_type *)item)->der;
        if (xml->flags & LY_YANG_STRUCTURE_FLAG) {
            name = ((struct yang_type *)xml)->name;
        } else {
            LY_TREE_FOR(xml->attr, attr) {
                if ((attr->type == LYXML_ATTR_STD) && !strcmp(attr->name, "name")) {
                    name = attr->value;
                    break;
                }
            }
            assert(attr);
        }
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later.", "derived type", name);
        break;
    case UNRES_IFFEAT:
        iff_data = str_node;
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later.", "if-feature", iff_data->fname);
        break;
    case UNRES_FEATURE:
        LOGVRB("There are unresolved if-features for \"%s\" feature circular dependency check, it will be attempted later",
               ((struct lys_feature *)item)->name);
        break;
    case UNRES_USES:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later.", "uses", ((struct lys_node_uses *)item)->name);
        break;
    case UNRES_TYPEDEF_DFLT:
    case UNRES_TYPE_DFLT:
        if (*(char **)str_node) {
            LOGVRB("Resolving %s \"%s\" failed, it will be attempted later.", "type default", *(char **)str_node);
        }   /* else no default value in the type itself, but we are checking some restrictions against
             *  possible default value of some base type. The failure is caused by not resolved base type,
             *  so it was already reported */
        break;
    case UNRES_CHOICE_DFLT:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later.", "choice default", (char *)str_node);
        break;
    case UNRES_LIST_KEYS:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later.", "list keys", (char *)str_node);
        break;
    case UNRES_LIST_UNIQ:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later.", "list unique", (char *)str_node);
        break;
    case UNRES_AUGMENT:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later.", "augment target",
               ((struct lys_node_augment *)item)->target_name);
        break;
    case UNRES_XPATH:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later.", "XPath expressions of",
               ((struct lys_node *)item)->name);
        break;
    case UNRES_EXT:
        extinfo = (struct unres_ext *)str_node;
        name = extinfo->datatype == LYS_IN_YIN ? extinfo->data.yin->name : NULL; /* TODO YANG extension */
        LOGVRB("Resolving extension \"%s\" failed, it will be attempted later.", name);
        break;
    default:
        LOGINT(NULL);
        break;
    }
}

static int
resolve_unres_schema_types(struct unres_schema *unres, enum UNRES_ITEM types, struct ly_ctx *ctx, int forward_ref,
                           int print_all_errors, uint32_t *resolved)
{
    uint32_t i, unres_count, res_count;
    int ret = 0, rc;
    struct ly_err_item *prev_eitem;
    enum int_log_opts prev_ilo;
    LY_ERR prev_ly_errno;

    /* if there can be no forward references, every failure is final, so we can print it directly */
    if (forward_ref) {
        prev_ly_errno = ly_errno;
        ly_ilo_change(ctx, ILO_STORE, &prev_ilo, &prev_eitem);
    }

    do {
        unres_count = 0;
        res_count = 0;

        for (i = 0; i < unres->count; ++i) {
            /* UNRES_TYPE_LEAFREF must be resolved (for storing leafref target pointers);
             * if-features are resolved here to make sure that we will have all if-features for
             * later check of feature circular dependency */
            if (unres->type[i] & types) {
                ++unres_count;
                rc = resolve_unres_schema_item(unres->module[i], unres->item[i], unres->type[i], unres->str_snode[i], unres);
                if (unres->type[i] == UNRES_EXT_FINALIZE) {
                    /* to avoid double free */
                    unres->type[i] = UNRES_RESOLVED;
                }
                if (!rc || (unres->type[i] == UNRES_XPATH)) {
                    /* invalid XPath can never cause an error, only a warning */
                    if (unres->type[i] == UNRES_LIST_UNIQ) {
                        /* free the allocated structure */
                        free(unres->item[i]);
                    }

                    unres->type[i] = UNRES_RESOLVED;
                    ++(*resolved);
                    ++res_count;
                } else if ((rc == EXIT_FAILURE) && forward_ref) {
                    /* forward reference, erase errors */
                    ly_err_free_next(ctx, prev_eitem);
                } else if (print_all_errors) {
                    /* just so that we quit the loop */
                    ++res_count;
                    ret = -1;
                } else {
                    if (forward_ref) {
                        ly_ilo_restore(ctx, prev_ilo, prev_eitem, 1);
                    }
                    return -1;
                }
            }
        }
    } while (res_count && (res_count < unres_count));

    if (res_count < unres_count) {
        assert(forward_ref);
        /* just print the errors (but we must free the ones we have and get them again :-/ ) */
        ly_ilo_restore(ctx, prev_ilo, prev_eitem, 0);

        for (i = 0; i < unres->count; ++i) {
            if (unres->type[i] & types) {
                resolve_unres_schema_item(unres->module[i], unres->item[i], unres->type[i], unres->str_snode[i], unres);
            }
        }
        return -1;
    }

    if (forward_ref) {
        /* restore log */
        ly_ilo_restore(ctx, prev_ilo, prev_eitem, 0);
        ly_errno = prev_ly_errno;
    }

    return ret;
}

/**
 * @brief Resolve every unres schema item in the structure. Logs directly.
 *
 * @param[in] mod Main module.
 * @param[in] unres Unres schema structure to use.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
int
resolve_unres_schema(struct lys_module *mod, struct unres_schema *unres)
{
    uint32_t resolved = 0;

    assert(unres);

    LOGVRB("Resolving \"%s\" unresolved schema nodes and their constraints...", mod->name);

    /* UNRES_TYPE_LEAFREF must be resolved (for storing leafref target pointers);
     * if-features are resolved here to make sure that we will have all if-features for
     * later check of feature circular dependency */
    if (resolve_unres_schema_types(unres, UNRES_USES | UNRES_IFFEAT | UNRES_TYPE_DER | UNRES_TYPE_DER_TPDF | UNRES_TYPE_DER_TPDF
                                   | UNRES_TYPE_LEAFREF | UNRES_MOD_IMPLEMENT | UNRES_AUGMENT | UNRES_CHOICE_DFLT | UNRES_IDENT,
                                   mod->ctx, 1, 0, &resolved)) {
        return -1;
    }

    /* another batch of resolved items */
    if (resolve_unres_schema_types(unres, UNRES_TYPE_IDENTREF | UNRES_FEATURE | UNRES_TYPEDEF_DFLT | UNRES_TYPE_DFLT
                                   | UNRES_LIST_KEYS | UNRES_LIST_UNIQ | UNRES_EXT, mod->ctx, 1, 0, &resolved)) {
        return -1;
    }

    /* print xpath warnings and finalize extensions, keep it last to provide the complete schema tree information to the plugin's checkers */
    if (resolve_unres_schema_types(unres, UNRES_XPATH | UNRES_EXT_FINALIZE, mod->ctx, 0, 1, &resolved)) {
        return -1;
    }

    LOGVRB("All \"%s\" schema nodes and constraints resolved.", mod->name);
    unres->count = 0;
    return EXIT_SUCCESS;
}

/**
 * @brief Try to resolve an unres schema item with a string argument. Logs indirectly.
 *
 * @param[in] mod Main module.
 * @param[in] unres Unres schema structure to use.
 * @param[in] item Item to resolve. Type determined by \p type.
 * @param[in] type Type of the unresolved item.
 * @param[in] str String argument.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on storing the item in unres, -1 on error.
 */
int
unres_schema_add_str(struct lys_module *mod, struct unres_schema *unres, void *item, enum UNRES_ITEM type,
                     const char *str)
{
    int rc;
    const char *dictstr;

    dictstr = lydict_insert(mod->ctx, str, 0);
    rc = unres_schema_add_node(mod, unres, item, type, (struct lys_node *)dictstr);

    if (rc < 0) {
        lydict_remove(mod->ctx, dictstr);
    }
    return rc;
}

/**
 * @brief Try to resolve an unres schema item with a schema node argument. Logs indirectly.
 *
 * @param[in] mod Main module.
 * @param[in] unres Unres schema structure to use.
 * @param[in] item Item to resolve. Type determined by \p type.
 * @param[in] type Type of the unresolved item. UNRES_TYPE_DER is handled specially!
 * @param[in] snode Schema node argument.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on storing the item in unres, -1 on error.
 */
int
unres_schema_add_node(struct lys_module *mod, struct unres_schema *unres, void *item, enum UNRES_ITEM type,
                      struct lys_node *snode)
{
    int rc;
    uint32_t u;
    enum int_log_opts prev_ilo;
    struct ly_err_item *prev_eitem;
    LY_ERR prev_ly_errno;
    struct lyxml_elem *yin;
    struct ly_ctx *ctx = mod->ctx;

    assert(unres && (item || (type == UNRES_MOD_IMPLEMENT)) && ((type != UNRES_LEAFREF) && (type != UNRES_INSTID)
           && (type != UNRES_WHEN) && (type != UNRES_MUST)));

    /* check for duplicities in unres */
    for (u = 0; u < unres->count; u++) {
        if (unres->type[u] == type && unres->item[u] == item &&
                unres->str_snode[u] == snode && unres->module[u] == mod) {
            /* duplication can happen when the node contains multiple statements of the same type to check,
             * this can happen for example when refinement is being applied, so we just postpone the processing
             * and do not duplicate the information */
            return EXIT_FAILURE;
        }
    }

    if ((type == UNRES_EXT_FINALIZE) || (type == UNRES_XPATH) || (type == UNRES_MOD_IMPLEMENT)) {
        /* extension finalization is not even tried when adding the item into the inres list,
         * xpath is not tried because it would hide some potential warnings,
         * implementing module must be deferred because some other nodes can be added that will need to be traversed
         * and their targets made implemented */
        rc = EXIT_FAILURE;
    } else {
        prev_ly_errno = ly_errno;
        ly_ilo_change(ctx, ILO_STORE, &prev_ilo, &prev_eitem);

        rc = resolve_unres_schema_item(mod, item, type, snode, unres);
        if (rc != EXIT_FAILURE) {
            ly_ilo_restore(ctx, prev_ilo, prev_eitem, rc == -1 ? 1 : 0);
            if (rc != -1) {
                ly_errno = prev_ly_errno;
            }

            if (type == UNRES_LIST_UNIQ) {
                /* free the allocated structure */
                free(item);
            } else if (rc == -1 && type == UNRES_IFFEAT) {
                /* free the allocated resources */
                free(*((char **)item));
            }
            return rc;
        } else {
            /* erase info about validation errors */
            ly_ilo_restore(ctx, prev_ilo, prev_eitem, 0);
            ly_errno = prev_ly_errno;
        }

        print_unres_schema_item_fail(item, type, snode);

        /* HACK unlinking is performed here so that we do not do any (NS) copying in vain */
        if (type == UNRES_TYPE_DER || type == UNRES_TYPE_DER_TPDF) {
            yin = (struct lyxml_elem *)((struct lys_type *)item)->der;
            if (!(yin->flags & LY_YANG_STRUCTURE_FLAG)) {
                lyxml_unlink_elem(mod->ctx, yin, 1);
                ((struct lys_type *)item)->der = (struct lys_tpdf *)yin;
            }
        }
    }

    unres->count++;
    unres->item = ly_realloc(unres->item, unres->count*sizeof *unres->item);
    LY_CHECK_ERR_RETURN(!unres->item, LOGMEM(ctx), -1);
    unres->item[unres->count-1] = item;
    unres->type = ly_realloc(unres->type, unres->count*sizeof *unres->type);
    LY_CHECK_ERR_RETURN(!unres->type, LOGMEM(ctx), -1);
    unres->type[unres->count-1] = type;
    unres->str_snode = ly_realloc(unres->str_snode, unres->count*sizeof *unres->str_snode);
    LY_CHECK_ERR_RETURN(!unres->str_snode, LOGMEM(ctx), -1);
    unres->str_snode[unres->count-1] = snode;
    unres->module = ly_realloc(unres->module, unres->count*sizeof *unres->module);
    LY_CHECK_ERR_RETURN(!unres->module, LOGMEM(ctx), -1);
    unres->module[unres->count-1] = mod;

    return rc;
}

/**
 * @brief Duplicate an unres schema item. Logs indirectly.
 *
 * @param[in] mod Main module.
 * @param[in] unres Unres schema structure to use.
 * @param[in] item Old item to be resolved.
 * @param[in] type Type of the old unresolved item.
 * @param[in] new_item New item to use in the duplicate.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE if item is not in unres, -1 on error.
 */
int
unres_schema_dup(struct lys_module *mod, struct unres_schema *unres, void *item, enum UNRES_ITEM type, void *new_item)
{
    int i;
    struct unres_list_uniq aux_uniq;
    struct unres_iffeat_data *iff_data;

    assert(item && new_item && ((type != UNRES_LEAFREF) && (type != UNRES_INSTID) && (type != UNRES_WHEN)));

    /* hack for UNRES_LIST_UNIQ, which stores multiple items behind its item */
    if (type == UNRES_LIST_UNIQ) {
        aux_uniq.list = item;
        aux_uniq.expr = ((struct unres_list_uniq *)new_item)->expr;
        item = &aux_uniq;
    }
    i = unres_schema_find(unres, -1, item, type);

    if (i == -1) {
        if (type == UNRES_LIST_UNIQ) {
            free(new_item);
        }
        return EXIT_FAILURE;
    }

    if ((type == UNRES_TYPE_LEAFREF) || (type == UNRES_USES) || (type == UNRES_TYPE_DFLT) ||
            (type == UNRES_FEATURE) || (type == UNRES_LIST_UNIQ)) {
        if (unres_schema_add_node(mod, unres, new_item, type, unres->str_snode[i]) == -1) {
            LOGINT(mod->ctx);
            return -1;
        }
    } else if (type == UNRES_IFFEAT) {
        /* duplicate unres_iffeature_data */
        iff_data = malloc(sizeof *iff_data);
        LY_CHECK_ERR_RETURN(!iff_data, LOGMEM(mod->ctx), -1);
        iff_data->fname = lydict_insert(mod->ctx, ((struct unres_iffeat_data *)unres->str_snode[i])->fname, 0);
        iff_data->node = ((struct unres_iffeat_data *)unres->str_snode[i])->node;
        if (unres_schema_add_node(mod, unres, new_item, type, (struct lys_node *)iff_data) == -1) {
            LOGINT(mod->ctx);
            return -1;
        }
    } else {
        if (unres_schema_add_str(mod, unres, new_item, type, unres->str_snode[i]) == -1) {
            LOGINT(mod->ctx);
            return -1;
        }
    }

    return EXIT_SUCCESS;
}

/* does not log */
int
unres_schema_find(struct unres_schema *unres, int start_on_backwards, void *item, enum UNRES_ITEM type)
{
    int i;
    struct unres_list_uniq *aux_uniq1, *aux_uniq2;

    if (!unres->count) {
        return -1;
    }

    if (start_on_backwards >= 0) {
        i = start_on_backwards;
    } else {
        i = unres->count - 1;
    }
    for (; i > -1; i--) {
        if (unres->type[i] != type) {
            continue;
        }
        if (type != UNRES_LIST_UNIQ) {
            if (unres->item[i] == item) {
                break;
            }
        } else {
            aux_uniq1 = (struct unres_list_uniq *)unres->item[i - 1];
            aux_uniq2 = (struct unres_list_uniq *)item;
            if ((aux_uniq1->list == aux_uniq2->list) && ly_strequal(aux_uniq1->expr, aux_uniq2->expr, 0)) {
                break;
            }
        }
    }

    return i;
}

static void
unres_schema_free_item(struct ly_ctx *ctx, struct unres_schema *unres, uint32_t i)
{
    struct lyxml_elem *yin;
    struct yang_type *yang;
    struct unres_iffeat_data *iff_data;

    switch (unres->type[i]) {
    case UNRES_TYPE_DER_TPDF:
    case UNRES_TYPE_DER:
        yin = (struct lyxml_elem *)((struct lys_type *)unres->item[i])->der;
        if (yin->flags & LY_YANG_STRUCTURE_FLAG) {
            yang =(struct yang_type *)yin;
            ((struct lys_type *)unres->item[i])->base = yang->base;
            lydict_remove(ctx, yang->name);
            free(yang);
            if (((struct lys_type *)unres->item[i])->base == LY_TYPE_UNION) {
                yang_free_type_union(ctx, (struct lys_type *)unres->item[i]);
            }
        } else {
            lyxml_free(ctx, yin);
        }
        break;
    case UNRES_IFFEAT:
        iff_data = (struct unres_iffeat_data *)unres->str_snode[i];
        lydict_remove(ctx, iff_data->fname);
        free(unres->str_snode[i]);
        break;
    case UNRES_IDENT:
    case UNRES_TYPE_IDENTREF:
    case UNRES_CHOICE_DFLT:
    case UNRES_LIST_KEYS:
        lydict_remove(ctx, (const char *)unres->str_snode[i]);
        break;
    case UNRES_LIST_UNIQ:
        free(unres->item[i]);
        break;
    case UNRES_EXT:
        free(unres->str_snode[i]);
        break;
    case UNRES_EXT_FINALIZE:
        free(unres->str_snode[i]);
    default:
        break;
    }
    unres->type[i] = UNRES_RESOLVED;
}

void
unres_schema_free(struct lys_module *module, struct unres_schema **unres, int all)
{
    uint32_t i;
    unsigned int unresolved = 0;

    if (!unres || !(*unres)) {
        return;
    }

    assert(module || ((*unres)->count == 0));

    for (i = 0; i < (*unres)->count; ++i) {
        if (!all && ((*unres)->module[i] != module)) {
            if ((*unres)->type[i] != UNRES_RESOLVED) {
                unresolved++;
            }
            continue;
        }

        /* free heap memory for the specific item */
        unres_schema_free_item(module->ctx, *unres, i);
    }

    /* free it all */
    if (!module || all || (!unresolved && !module->type)) {
        free((*unres)->item);
        free((*unres)->type);
        free((*unres)->str_snode);
        free((*unres)->module);
        free((*unres));
        (*unres) = NULL;
    }
}

/* check whether instance-identifier points outside its data subtree (for operation it is any node
 * outside the operation subtree, otherwise it is a node from a foreign model) */
static int
check_instid_ext_dep(const struct lys_node *sleaf, const char *json_instid)
{
    const struct lys_node *op_node, *first_node;
    enum int_log_opts prev_ilo;
    char *buf, *tmp;

    if (!json_instid || !json_instid[0]) {
        /* no/empty value */
        return 0;
    }

    for (op_node = lys_parent(sleaf);
         op_node && !(op_node->nodetype & (LYS_NOTIF | LYS_RPC | LYS_ACTION));
         op_node = lys_parent(op_node));

    if (op_node && lys_parent(op_node)) {
        /* nested operation - any absolute path is external */
        return 1;
    }

    /* get the first node from the instid */
    tmp = strchr(json_instid + 1, '/');
    buf = strndup(json_instid, tmp ? (size_t)(tmp - json_instid) : strlen(json_instid));
    if (!buf) {
        /* so that we do not have to bother with logging, say it is not external */
        return 0;
    }

    /* find the first schema node, do not log */
    ly_ilo_change(NULL, ILO_IGNORE, &prev_ilo, NULL);
    first_node = ly_ctx_get_node(NULL, sleaf, buf, 0);
    ly_ilo_restore(NULL, prev_ilo, NULL, 0);

    free(buf);
    if (!first_node) {
        /* unknown path, say it is external */
        return 1;
    }

    /* based on the first schema node in the path we can decide whether it points to an external tree or not */

    if (op_node) {
        if (op_node != first_node) {
            /* it is a top-level operation, so we're good if it points somewhere inside it */
            return 1;
        }
    } else {
        if (lys_node_module(sleaf) != lys_node_module(first_node)) {
            /* modules differ */
            return 1;
        }
    }

    return 0;
}

/**
 * @brief Resolve instance-identifier in JSON data format. Logs directly.
 *
 * @param[in] data Data node where the path is used
 * @param[in] path Instance-identifier node value.
 * @param[in,out] ret Resolved instance or NULL.
 *
 * @return 0 on success (even if unresolved and \p ret is NULL), -1 on error.
 */
static int
resolve_instid(struct lyd_node *data, const char *path, int req_inst, struct lyd_node **ret)
{
    int i = 0, j, parsed, cur_idx;
    const struct lys_module *mod, *prev_mod = NULL;
    struct ly_ctx *ctx = data->schema->module->ctx;
    struct lyd_node *root, *node;
    const char *model = NULL, *name;
    char *str;
    int mod_len, name_len, has_predicate;
    struct unres_data node_match;

    memset(&node_match, 0, sizeof node_match);
    *ret = NULL;

    /* we need root to resolve absolute path */
    for (root = data; root->parent; root = root->parent);
    /* we're still parsing it and the pointer is not correct yet */
    if (root->prev) {
        for (; root->prev->next; root = root->prev);
    }

    /* search for the instance node */
    while (path[i]) {
        j = parse_instance_identifier(&path[i], &model, &mod_len, &name, &name_len, &has_predicate);
        if (j <= 0) {
            LOGVAL(ctx, LYE_INCHAR, LY_VLOG_LYD, data, path[i-j], &path[i-j]);
            goto error;
        }
        i += j;

        if (model) {
            str = strndup(model, mod_len);
            if (!str) {
                LOGMEM(ctx);
                goto error;
            }
            mod = ly_ctx_get_module(ctx, str, NULL, 1);
            if (ctx->data_clb) {
                if (!mod) {
                    mod = ctx->data_clb(ctx, str, NULL, 0, ctx->data_clb_data);
                } else if (!mod->implemented) {
                    mod = ctx->data_clb(ctx, mod->name, mod->ns, LY_MODCLB_NOT_IMPLEMENTED, ctx->data_clb_data);
                }
            }
            free(str);

            if (!mod || !mod->implemented || mod->disabled) {
                break;
            }
        } else if (!prev_mod) {
            /* first iteration and we are missing module name */
            LOGVAL(ctx, LYE_INELEM_LEN, LY_VLOG_LYD, data, name_len, name);
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL, "Instance-identifier is missing prefix in the first node.");
            goto error;
        } else {
            mod = prev_mod;
        }

        if (resolve_data(mod, name, name_len, root, &node_match)) {
            /* no instance exists */
            break;
        }

        if (has_predicate) {
            /* we have predicate, so the current results must be list or leaf-list */
            parsed = j = 0;
            /* index of the current node (for lists with position predicates) */
            cur_idx = 1;
            while (j < (signed)node_match.count) {
                node = node_match.node[j];
                parsed = resolve_instid_predicate(mod, &path[i], &node, cur_idx);
                if (parsed < 1) {
                    LOGVAL(ctx, LYE_INPRED, LY_VLOG_LYD, data, &path[i - parsed]);
                    goto error;
                }

                if (!node) {
                    /* current node does not satisfy the predicate */
                    unres_data_del(&node_match, j);
                } else {
                    ++j;
                }
                ++cur_idx;
            }

            i += parsed;
        } else if (node_match.count) {
            /* check that we are not addressing lists */
            for (j = 0; (unsigned)j < node_match.count; ++j) {
                if (node_match.node[j]->schema->nodetype == LYS_LIST) {
                    unres_data_del(&node_match, j--);
                }
            }
            if (!node_match.count) {
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_LYD, data, "Instance identifier is missing list keys.");
            }
        }

        prev_mod = mod;
    }

    if (!node_match.count) {
        /* no instance exists */
        if (req_inst > -1) {
            LOGVAL(ctx, LYE_NOREQINS, LY_VLOG_LYD, data, path);
            return EXIT_FAILURE;
        }
        LOGVRB("There is no instance of \"%s\", but it is not required.", path);
        return EXIT_SUCCESS;
    } else if (node_match.count > 1) {
        /* instance identifier must resolve to a single node */
        LOGVAL(ctx, LYE_TOOMANY, LY_VLOG_LYD, data, path, "data tree");
        goto error;
    } else {
        /* we have required result, remember it and cleanup */
        *ret = node_match.node[0];
        free(node_match.node);
        return EXIT_SUCCESS;
    }

error:
    /* cleanup */
    free(node_match.node);
    return -1;
}

static int
resolve_leafref(struct lyd_node_leaf_list *leaf, const char *path, int req_inst, struct lyd_node **ret)
{
    struct lyxp_set xp_set;
    uint32_t i;

    memset(&xp_set, 0, sizeof xp_set);
    *ret = NULL;

    /* syntax was already checked, so just evaluate the path using standard XPath */
    if (lyxp_eval(path, (struct lyd_node *)leaf, LYXP_NODE_ELEM, lyd_node_module((struct lyd_node *)leaf), &xp_set, 0) != EXIT_SUCCESS) {
        return -1;
    }

    if (xp_set.type == LYXP_SET_NODE_SET) {
        for (i = 0; i < xp_set.used; ++i) {
            if ((xp_set.val.nodes[i].type != LYXP_NODE_ELEM) || !(xp_set.val.nodes[i].node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                continue;
            }

            /* not that the value is already in canonical form since the parsers does the conversion,
             * so we can simply compare just the values */
            if (ly_strequal(leaf->value_str, ((struct lyd_node_leaf_list *)xp_set.val.nodes[i].node)->value_str, 1)) {
                /* we have the match */
                *ret = xp_set.val.nodes[i].node;
                break;
            }
        }
    }

    lyxp_set_cast(&xp_set, LYXP_SET_EMPTY, (struct lyd_node *)leaf, NULL, 0);

    if (!*ret) {
        /* reference not found */
        if (req_inst > -1) {
            LOGVAL(leaf->schema->module->ctx, LYE_NOLEAFREF, LY_VLOG_LYD, leaf, path, leaf->value_str);
            return EXIT_FAILURE;
        } else {
            LOGVRB("There is no leafref \"%s\" with the value \"%s\", but it is not required.", path, leaf->value_str);
        }
    }

    return EXIT_SUCCESS;
}

/* ignore fail because we are parsing edit-config, get, or get-config - but only if the union includes leafref or instid */
int
resolve_union(struct lyd_node_leaf_list *leaf, struct lys_type *type, int store, int ignore_fail,
              struct lys_type **resolved_type)
{
    struct ly_ctx *ctx = leaf->schema->module->ctx;
    struct lys_type *t;
    struct lyd_node *ret;
    enum int_log_opts prev_ilo;
    int found, success = 0, ext_dep, req_inst;
    const char *json_val = NULL;

    assert(type->base == LY_TYPE_UNION);

    if ((leaf->value_type == LY_TYPE_UNION) || ((leaf->value_type == LY_TYPE_INST) && (leaf->value_flags & LY_VALUE_UNRES))) {
        /* either NULL or instid previously converted to JSON */
        json_val = lydict_insert(ctx, leaf->value.string, 0);
    }

    if (store) {
        lyd_free_value(leaf->value, leaf->value_type, leaf->value_flags, &((struct lys_node_leaf *)leaf->schema)->type,
                       NULL, NULL, NULL);
        memset(&leaf->value, 0, sizeof leaf->value);
    }

    /* turn logging off, we are going to try to validate the value with all the types in order */
    ly_ilo_change(NULL, ILO_IGNORE, &prev_ilo, 0);

    t = NULL;
    found = 0;
    while ((t = lyp_get_next_union_type(type, t, &found))) {
        found = 0;

        switch (t->base) {
        case LY_TYPE_LEAFREF:
            if ((ignore_fail == 1) || ((leaf->schema->flags & LYS_LEAFREF_DEP) && (ignore_fail == 2))) {
                req_inst = -1;
            } else {
                req_inst = t->info.lref.req;
            }

            if (!resolve_leafref(leaf, t->info.lref.path, req_inst, &ret)) {
                if (store) {
                    if (ret && !(leaf->schema->flags & LYS_LEAFREF_DEP)) {
                        /* valid resolved */
                        leaf->value.leafref = ret;
                        leaf->value_type = LY_TYPE_LEAFREF;
                    } else {
                        /* valid unresolved */
                        ly_ilo_restore(NULL, prev_ilo, NULL, 0);
                        if (!lyp_parse_value(t, &leaf->value_str, NULL, leaf, NULL, NULL, 1, 0, 0)) {
                            return -1;
                        }
                        ly_ilo_change(NULL, ILO_IGNORE, &prev_ilo, NULL);
                    }
                }

                success = 1;
            }
            break;
        case LY_TYPE_INST:
            ext_dep = check_instid_ext_dep(leaf->schema, (json_val ? json_val : leaf->value_str));
            if ((ignore_fail == 1) || (ext_dep && (ignore_fail == 2))) {
                req_inst = -1;
            } else {
                req_inst = t->info.inst.req;
            }

            if (!resolve_instid((struct lyd_node *)leaf, (json_val ? json_val : leaf->value_str), req_inst, &ret)) {
                if (store) {
                    if (ret && !ext_dep) {
                        /* valid resolved */
                        leaf->value.instance = ret;
                        leaf->value_type = LY_TYPE_INST;

                        if (json_val) {
                            lydict_remove(leaf->schema->module->ctx, leaf->value_str);
                            leaf->value_str = json_val;
                            json_val = NULL;
                        }
                    } else {
                        /* valid unresolved */
                        if (json_val) {
                            /* put the JSON val back */
                            leaf->value.string = json_val;
                            json_val = NULL;
                        } else {
                            leaf->value.instance = NULL;
                        }
                        leaf->value_type = LY_TYPE_INST;
                        leaf->value_flags |= LY_VALUE_UNRES;
                    }
                }

                success = 1;
            }
            break;
        default:
            if (lyp_parse_value(t, &leaf->value_str, NULL, leaf, NULL, NULL, store, 0, 0)) {
                success = 1;
            }
            break;
        }

        if (success) {
            break;
        }

        /* erase possible present and invalid value data */
        if (store) {
            lyd_free_value(leaf->value, leaf->value_type, leaf->value_flags, t, NULL, NULL, NULL);
            memset(&leaf->value, 0, sizeof leaf->value);
        }
    }

    /* turn logging back on */
    ly_ilo_restore(NULL, prev_ilo, NULL, 0);

    if (json_val) {
        if (!success) {
            /* put the value back for now */
            assert(leaf->value_type == LY_TYPE_UNION);
            leaf->value.string = json_val;
        } else {
            /* value was ultimately useless, but we could not have known */
            lydict_remove(leaf->schema->module->ctx, json_val);
        }
    }

    if (success) {
        if (resolved_type) {
            *resolved_type = t;
        }
    } else if (!ignore_fail || !type->info.uni.has_ptr_type) {
        /* not found and it is required */
        LOGVAL(ctx, LYE_INVAL, LY_VLOG_LYD, leaf, leaf->value_str ? leaf->value_str : "", leaf->schema->name);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

}

/**
 * @brief Resolve a single unres data item. Logs directly.
 *
 * @param[in] node Data node to resolve.
 * @param[in] type Type of the unresolved item.
 * @param[in] ignore_fail 0 - no, 1 - yes, 2 - yes, but only for external dependencies.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
int
resolve_unres_data_item(struct lyd_node *node, enum UNRES_ITEM type, int ignore_fail, struct lys_when **failed_when)
{
    int rc, req_inst, ext_dep;
    struct lyd_node_leaf_list *leaf;
    struct lyd_node *ret;
    struct lys_node_leaf *sleaf;

    leaf = (struct lyd_node_leaf_list *)node;
    sleaf = (struct lys_node_leaf *)leaf->schema;

    switch (type) {
    case UNRES_LEAFREF:
        assert(sleaf->type.base == LY_TYPE_LEAFREF);
        assert(leaf->validity & LYD_VAL_LEAFREF);
        if ((ignore_fail == 1) || ((leaf->schema->flags & LYS_LEAFREF_DEP) && (ignore_fail == 2))) {
            req_inst = -1;
        } else {
            req_inst = sleaf->type.info.lref.req;
        }
        rc = resolve_leafref(leaf, sleaf->type.info.lref.path, req_inst, &ret);
        if (!rc) {
            if (ret && !(leaf->schema->flags & LYS_LEAFREF_DEP)) {
                /* valid resolved */
                if (leaf->value_type == LY_TYPE_BITS) {
                    free(leaf->value.bit);
                }
                leaf->value.leafref = ret;
                leaf->value_type = LY_TYPE_LEAFREF;
                leaf->value_flags &= ~LY_VALUE_UNRES;
            } else {
                /* valid unresolved */
                if (!(leaf->value_flags & LY_VALUE_UNRES)) {
                    if (!lyp_parse_value(&sleaf->type, &leaf->value_str, NULL, leaf, NULL, NULL, 1, 0, 0)) {
                        return -1;
                    }
                }
            }
            leaf->validity &= ~LYD_VAL_LEAFREF;
        } else {
            return rc;
        }
        break;

    case UNRES_INSTID:
        assert(sleaf->type.base == LY_TYPE_INST);
        ext_dep = check_instid_ext_dep(leaf->schema, leaf->value_str);
        if (ext_dep == -1) {
            return -1;
        }

        if ((ignore_fail == 1) || (ext_dep && (ignore_fail == 2))) {
            req_inst = -1;
        } else {
            req_inst = sleaf->type.info.inst.req;
        }
        rc = resolve_instid(node, leaf->value_str, req_inst, &ret);
        if (!rc) {
            if (ret && !ext_dep) {
                /* valid resolved */
                leaf->value.instance = ret;
                leaf->value_type = LY_TYPE_INST;
                leaf->value_flags &= ~LY_VALUE_UNRES;
            } else {
                /* valid unresolved */
                leaf->value.instance = NULL;
                leaf->value_type = LY_TYPE_INST;
                leaf->value_flags |= LY_VALUE_UNRES;
            }
        } else {
            return rc;
        }
        break;

    case UNRES_UNION:
        assert(sleaf->type.base == LY_TYPE_UNION);
        return resolve_union(leaf, &sleaf->type, 1, ignore_fail, NULL);

    case UNRES_WHEN:
        if ((rc = resolve_when(node, ignore_fail, failed_when))) {
            return rc;
        }
        break;

    case UNRES_MUST:
        if ((rc = resolve_must(node, 0, ignore_fail))) {
            return rc;
        }
        break;

    case UNRES_MUST_INOUT:
        if ((rc = resolve_must(node, 1, ignore_fail))) {
            return rc;
        }
        break;

    case UNRES_UNIQ_LEAVES:
        if (lyv_data_unique(node)) {
            return -1;
        }
        break;

    default:
        LOGINT(NULL);
        return -1;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief add data unres item
 *
 * @param[in] unres Unres data structure to use.
 * @param[in] node Data node to use.
 *
 * @return 0 on success, -1 on error.
 */
int
unres_data_add(struct unres_data *unres, struct lyd_node *node, enum UNRES_ITEM type)
{
    assert(unres && node);
    assert((type == UNRES_LEAFREF) || (type == UNRES_INSTID) || (type == UNRES_WHEN) || (type == UNRES_MUST)
           || (type == UNRES_MUST_INOUT) || (type == UNRES_UNION) || (type == UNRES_UNIQ_LEAVES));

    unres->count++;
    unres->node = ly_realloc(unres->node, unres->count * sizeof *unres->node);
    LY_CHECK_ERR_RETURN(!unres->node, LOGMEM(NULL), -1);
    unres->node[unres->count - 1] = node;
    unres->type = ly_realloc(unres->type, unres->count * sizeof *unres->type);
    LY_CHECK_ERR_RETURN(!unres->type, LOGMEM(NULL), -1);
    unres->type[unres->count - 1] = type;

    return 0;
}

static void
resolve_unres_data_autodel_diff(struct unres_data *unres, uint32_t unres_i)
{
    struct lyd_node *next, *child, *parent;
    uint32_t i;

    for (i = 0; i < unres->diff_idx; ++i) {
        if (unres->diff->type[i] == LYD_DIFF_DELETED) {
            /* only leaf(-list) default could be removed and there is nothing to be checked in that case */
            continue;
        }

        if (unres->diff->second[i] == unres->node[unres_i]) {
            /* 1) default value was supposed to be created, but is disabled by when
             * -> remove it from diff altogether
             */
            unres_data_diff_rem(unres, i);
            /* if diff type is CREATED, the value was just a pointer, it can be freed normally (unlike in 4) */
            return;
        } else {
            parent = unres->diff->second[i]->parent;
            while (parent && (parent != unres->node[unres_i])) {
                parent = parent->parent;
            }
            if (parent) {
                /* 2) default value was supposed to be created but is disabled by when in some parent
                 * -> remove this default subtree and add the rest into diff as deleted instead in 4)
                 */
                unres_data_diff_rem(unres, i);
                break;
            }

            LY_TREE_DFS_BEGIN(unres->diff->second[i]->parent, next, child) {
                if (child == unres->node[unres_i]) {
                    /* 3) some default child of a default value was supposed to be created but has false when
                     * -> the subtree will be freed later and automatically disconnected from the diff parent node
                     */
                    return;
                }

                LY_TREE_DFS_END(unres->diff->second[i]->parent, next, child);
            }
        }
    }

    /* 4) it does not overlap with created default values in any way
     * -> just add it into diff as deleted
     */
    unres_data_diff_new(unres, unres->node[unres_i], unres->node[unres_i]->parent, 0);
    lyd_unlink(unres->node[unres_i]);

    /* should not be freed anymore */
    unres->node[unres_i] = NULL;
}

/**
 * @brief Resolve every unres data item in the structure. Logs directly.
 *
 * If options include #LYD_OPT_TRUSTED, the data are considered trusted (must conditions are not expected,
 * unresolved leafrefs/instids are accepted, when conditions are normally resolved because at least some implicit
 * non-presence containers may need to be deleted).
 *
 * If options includes #LYD_OPT_WHENAUTODEL, the non-default nodes with false when conditions are auto-deleted.
 *
 * @param[in] ctx Context used.
 * @param[in] unres Unres data structure to use.
 * @param[in,out] root Root node of the data tree, can be changed due to autodeletion.
 * @param[in] options Data options as described above.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
int
resolve_unres_data(struct ly_ctx *ctx, struct unres_data *unres, struct lyd_node **root, int options)
{
    uint32_t i, j, first, resolved, del_items, stmt_count;
    uint8_t prev_when_status;
    int rc, progress, ignore_fail;
    enum int_log_opts prev_ilo;
    struct ly_err_item *prev_eitem;
    LY_ERR prev_ly_errno = ly_errno;
    struct lyd_node *parent;
    struct lys_when *when;

    assert(root);
    assert(unres);

    if (!unres->count) {
        return EXIT_SUCCESS;
    }

    if (options & (LYD_OPT_NOTIF_FILTER | LYD_OPT_GET | LYD_OPT_GETCONFIG | LYD_OPT_EDIT)) {
        ignore_fail = 1;
    } else if (options & LYD_OPT_NOEXTDEPS) {
        ignore_fail = 2;
    } else {
        ignore_fail = 0;
    }

    LOGVRB("Resolving unresolved data nodes and their constraints...");
    if (!ignore_fail) {
        /* remember logging state only if errors are generated and valid */
        ly_ilo_change(ctx, ILO_STORE, &prev_ilo, &prev_eitem);
    }

    /*
     * when-stmt first
     */
    first = 1;
    stmt_count = 0;
    resolved = 0;
    del_items = 0;
    do {
        if (!ignore_fail) {
            ly_err_free_next(ctx, prev_eitem);
        }
        progress = 0;
        for (i = 0; i < unres->count; i++) {
            if (unres->type[i] != UNRES_WHEN) {
                continue;
            }
            if (first) {
                /* count when-stmt nodes in unres list */
                stmt_count++;
            }

            /* resolve when condition only when all parent when conditions are already resolved */
            for (parent = unres->node[i]->parent;
                 parent && LYD_WHEN_DONE(parent->when_status);
                 parent = parent->parent) {
                if (!parent->parent && (parent->when_status & LYD_WHEN_FALSE)) {
                    /* the parent node was already unlinked, do not resolve this node,
                     * it will be removed anyway, so just mark it as resolved
                     */
                    unres->node[i]->when_status |= LYD_WHEN_FALSE;
                    unres->type[i] = UNRES_RESOLVED;
                    resolved++;
                    break;
                }
            }
            if (parent) {
                continue;
            }

            prev_when_status = unres->node[i]->when_status;
            rc = resolve_unres_data_item(unres->node[i], unres->type[i], ignore_fail, &when);
            if (!rc) {
                /* finish with error/delete the node only if when was changed from true to false, an external
                 * dependency was not required, or it was not provided (the flag would not be passed down otherwise,
                 * checked in upper functions) */
                if ((unres->node[i]->when_status & LYD_WHEN_FALSE)
                        && (!(when->flags & (LYS_XPCONF_DEP | LYS_XPSTATE_DEP)) || !(options & LYD_OPT_NOEXTDEPS))) {
                    if ((!(prev_when_status & LYD_WHEN_TRUE) || !(options & LYD_OPT_WHENAUTODEL)) && !unres->node[i]->dflt) {
                        /* false when condition */
                        goto error;
                    } /* follows else */

                    /* auto-delete */
                    LOGVRB("Auto-deleting node \"%s\" due to when condition (%s)", ly_errpath(ctx), when->cond);

                    /* only unlink now, the subtree can contain another nodes stored in the unres list */
                    /* if it has parent non-presence containers that would be empty, we should actually
                     * remove the container
                     */
                    for (parent = unres->node[i];
                            parent->parent && parent->parent->schema->nodetype == LYS_CONTAINER;
                            parent = parent->parent) {
                        if (((struct lys_node_container *)parent->parent->schema)->presence) {
                            /* presence container */
                            break;
                        }
                        if (parent->next || parent->prev != parent) {
                            /* non empty (the child we are in and we are going to remove is not the only child) */
                            break;
                        }
                    }
                    unres->node[i] = parent;

                    if (*root && *root == unres->node[i]) {
                        *root = (*root)->next;
                    }

                    lyd_unlink(unres->node[i]);
                    unres->type[i] = UNRES_DELETE;
                    del_items++;

                    /* update the rest of unres items */
                    for (j = 0; j < unres->count; j++) {
                        if (unres->type[j] == UNRES_RESOLVED || unres->type[j] == UNRES_DELETE) {
                            continue;
                        }

                        /* test if the node is in subtree to be deleted */
                        for (parent = unres->node[j]; parent; parent = parent->parent) {
                            if (parent == unres->node[i]) {
                                /* yes, it is */
                                unres->type[j] = UNRES_RESOLVED;
                                resolved++;
                                break;
                            }
                        }
                    }
                } else {
                    unres->type[i] = UNRES_RESOLVED;
                }
                if (!ignore_fail) {
                    ly_err_free_next(ctx, prev_eitem);
                }
                resolved++;
                progress = 1;
            } else if (rc == -1) {
                goto error;
            } /* else forward reference */
        }
        first = 0;
    } while (progress && resolved < stmt_count);

    /* do we have some unresolved when-stmt? */
    if (stmt_count > resolved) {
        goto error;
    }

    for (i = 0; del_items && i < unres->count; i++) {
        /* we had some when-stmt resulted to false, so now we have to sanitize the unres list */
        if (unres->type[i] != UNRES_DELETE) {
            continue;
        }
        if (!unres->node[i]) {
            unres->type[i] = UNRES_RESOLVED;
            del_items--;
            continue;
        }

        if (unres->store_diff) {
            resolve_unres_data_autodel_diff(unres, i);
        }

        /* really remove the complete subtree */
        lyd_free(unres->node[i]);
        unres->type[i] = UNRES_RESOLVED;
        del_items--;
    }

    /*
     * now leafrefs
     */
    if (options & LYD_OPT_TRUSTED) {
        /* we want to attempt to resolve leafrefs */
        assert(!ignore_fail);
        ignore_fail = 1;

        ly_ilo_restore(ctx, prev_ilo, prev_eitem, 0);
        ly_errno = prev_ly_errno;
    }
    first = 1;
    stmt_count = 0;
    resolved = 0;
    do {
        progress = 0;
        for (i = 0; i < unres->count; i++) {
            if (unres->type[i] != UNRES_LEAFREF) {
                continue;
            }
            if (first) {
                /* count leafref nodes in unres list */
                stmt_count++;
            }

            rc = resolve_unres_data_item(unres->node[i], unres->type[i], ignore_fail, NULL);
            if (!rc) {
                unres->type[i] = UNRES_RESOLVED;
                if (!ignore_fail) {
                    ly_err_free_next(ctx, prev_eitem);
                }
                resolved++;
                progress = 1;
            } else if (rc == -1) {
                goto error;
            } /* else forward reference */
        }
        first = 0;
    } while (progress && resolved < stmt_count);

    /* do we have some unresolved leafrefs? */
    if (stmt_count > resolved) {
        goto error;
    }

    if (!ignore_fail) {
        /* log normally now, throw away irrelevant errors */
        ly_ilo_restore(ctx, prev_ilo, prev_eitem, 0);
        ly_errno = prev_ly_errno;
    }

    /*
     * rest
     */
    for (i = 0; i < unres->count; ++i) {
        if (unres->type[i] == UNRES_RESOLVED) {
            continue;
        }
        assert(!(options & LYD_OPT_TRUSTED) || ((unres->type[i] != UNRES_MUST) && (unres->type[i] != UNRES_MUST_INOUT)));

        rc = resolve_unres_data_item(unres->node[i], unres->type[i], ignore_fail, NULL);
        if (rc) {
            /* since when was already resolved, a forward reference is an error */
            return -1;
        }

        unres->type[i] = UNRES_RESOLVED;
    }

    LOGVRB("All data nodes and constraints resolved.");
    unres->count = 0;
    return EXIT_SUCCESS;

error:
    if (!ignore_fail) {
        /* print all the new errors */
        ly_ilo_restore(ctx, prev_ilo, prev_eitem, 1);
        /* do not restore ly_errno, it was udpated properly */
    }
    return -1;
}
