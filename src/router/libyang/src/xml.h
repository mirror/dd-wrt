/**
 * @file xml.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Generic XML parser routines.
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_XML_H_
#define LY_XML_H_

#include <stddef.h>
#include <stdint.h>

#include "log.h"
#include "set.h"

struct ly_ctx;
struct ly_in;
struct ly_out;

/* Macro to test if character is whitespace */
#define is_xmlws(c) (c == 0x20 || c == 0x9 || c == 0xa || c == 0xd)

/* Macro to test if character is allowed to be a first character of an qualified identifier */
#define is_xmlqnamestartchar(c) ((c >= 'a' && c <= 'z') || c == '_' || \
        (c >= 'A' && c <= 'Z') || /* c == ':' || */ \
        (c >= 0x370 && c <= 0x1fff && c != 0x37e ) || \
        (c >= 0xc0 && c <= 0x2ff && c != 0xd7 && c != 0xf7) || c == 0x200c || \
        c == 0x200d || (c >= 0x2070 && c <= 0x218f) || \
        (c >= 0x2c00 && c <= 0x2fef) || (c >= 0x3001 && c <= 0xd7ff) || \
        (c >= 0xf900 && c <= 0xfdcf) || (c >= 0xfdf0 && c <= 0xfffd) || \
        (c >= 0x10000 && c <= 0xeffff))

/* Macro to test if character is allowed to be used in an qualified identifier */
#define is_xmlqnamechar(c) ((c >= 'a' && c <= 'z') || c == '_' || c == '-' || \
        (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || /* c == ':' || */ \
        c == '.' || c == 0xb7 || (c >= 0x370 && c <= 0x1fff && c != 0x37e ) ||\
        (c >= 0xc0 && c <= 0x2ff && c != 0xd7 && c != 0xf7) || c == 0x200c || \
        c == 0x200d || (c >= 0x300 && c <= 0x36f) || \
        (c >= 0x2070 && c <= 0x218f) || (c >= 0x203f && c <= 0x2040) || \
        (c >= 0x2c00 && c <= 0x2fef) || (c >= 0x3001 && c <= 0xd7ff) || \
        (c >= 0xf900 && c <= 0xfdcf) || (c >= 0xfdf0 && c <= 0xfffd) || \
        (c >= 0x10000 && c <= 0xeffff))

struct lyxml_ns {
    char *prefix;         /* prefix of the namespace, NULL for the default namespace */
    char *uri;            /* namespace URI */
    uint32_t depth;       /* depth level of the element to maintain the list of accessible namespace definitions */
};

/* element tag identifier for matching opening and closing tags */
struct lyxml_elem {
    const char *prefix;
    const char *name;
    size_t prefix_len;
    size_t name_len;
};

/**
 * @brief Status of the parser providing information what is expected next (which function is supposed to be called).
 */
enum LYXML_PARSER_STATUS {
    LYXML_ELEMENT,        /* opening XML element parsed */
    LYXML_ELEM_CLOSE,     /* closing XML element parsed */
    LYXML_ELEM_CONTENT,   /* XML element context parsed */
    LYXML_ATTRIBUTE,      /* XML attribute parsed */
    LYXML_ATTR_CONTENT,   /* XML attribute content parsed */
    LYXML_END             /* end of input data */
};

struct lyxml_ctx {
    const struct ly_ctx *ctx;
    struct ly_in *in;       /* input structure */

    enum LYXML_PARSER_STATUS status; /* status providing information about the last parsed object, following attributes
                                        are filled based on it */
    union {
        const char *prefix; /* LYXML_ELEMENT, LYXML_ATTRIBUTE - elem/attr prefix */
        const char *value;  /* LYXML_ELEM_CONTENT, LYXML_ATTR_CONTENT - elem/attr value */
    };
    union {
        size_t prefix_len;  /* LYXML_ELEMENT, LYXML_ATTRIBUTE - elem/attr prefix length */
        size_t value_len;   /* LYXML_ELEM_CONTENT, LYXML_ATTR_CONTENT - elem/attr value length */
    };
    union {
        const char *name;   /* LYXML_ELEMENT, LYXML_ATTRIBUTE - elem/attr name */
        ly_bool ws_only;    /* LYXML_ELEM_CONTENT, LYXML_ATTR_CONTENT - whether elem/attr value is empty/white-space only */
    };
    union {
        size_t name_len;    /* LYXML_ELEMENT, LYXML_ATTRIBUTE - elem/attr name length */
        ly_bool dynamic;    /* LYXML_ELEM_CONTENT, LYXML_ATTR_CONTENT - whether elem/attr value is dynamically allocated */
    };

    struct ly_set elements; /* list of not-yet-closed elements */
    struct ly_set ns;       /* handled with LY_SET_OPT_USEASLIST */
};

/**
 * @brief Create a new XML parser context and start parsing.
 *
 * @param[in] ctx libyang context.
 * @param[in] in Input structure.
 * @param[out] xmlctx New XML context with status ::LYXML_ELEMENT.
 * @return LY_ERR value.
 */
LY_ERR lyxml_ctx_new(const struct ly_ctx *ctx, struct ly_in *in, struct lyxml_ctx **xmlctx);

/**
 * @brief Move to the next XML artefact and update parser status.
 *
 * LYXML_ELEMENT (-> LYXML_ATTRIBUTE -> LYXML_ATTR_CONTENT)* -> LYXML_ELEM_CONTENT -> LYXML_ELEM_CLOSE ...
 *                                                                                 -> LYXML_ELEMENT ...
 *
 * @param[in] xmlctx XML context to move.
 * @return LY_ERR value.
 */
LY_ERR lyxml_ctx_next(struct lyxml_ctx *xmlctx);

/**
 * @brief Peek at the next XML parser status without changing it.
 *
 * @param[in] xmlctx XML context to use.
 * @param[out] next Next XML parser status.
 * @return LY_ERR value.
 */
LY_ERR lyxml_ctx_peek(struct lyxml_ctx *xmlctx, enum LYXML_PARSER_STATUS *next);

/**
 * @brief Get a namespace record for the given prefix in the current context.
 *
 * @param[in] ns_set Set with namespaces from the XML context.
 * @param[in] prefix Pointer to the namespace prefix as taken from ::lyxml_get_attribute() or ::lyxml_get_element().
 * Can be NULL for default namespace.
 * @param[in] prefix_len Length of the prefix string (since it is not NULL-terminated when returned from ::lyxml_get_attribute() or
 * ::lyxml_get_element()).
 * @return The namespace record or NULL if the record for the specified prefix not found.
 */
const struct lyxml_ns *lyxml_ns_get(const struct ly_set *ns_set, const char *prefix, size_t prefix_len);

/**
 * @brief Print the given @p text as XML string which replaces some of the characters which cannot appear in XML data.
 *
 * @param[in] out Output structure for printing.
 * @param[in] text String to print.
 * @param[in] attribute Flag for attribute's value where a double quotes must be replaced.
 * @return LY_ERR values.
 */
LY_ERR lyxml_dump_text(struct ly_out *out, const char *text, ly_bool attribute);

/**
 * @brief Remove the allocated working memory of the context.
 *
 * @param[in] xmlctx XML context to clear.
 */
void lyxml_ctx_free(struct lyxml_ctx *xmlctx);

/**
 * @brief Compare values and their prefix mappings.
 *
 * @param[in] ctx1 Libyang context for resolving prefixes in @p value1.
 * @param[in] value1 First value.
 * @param[in] val_prefix_data1 First value prefix data.
 * @param[in] ctx2 Libyang context for resolving prefixes in @p value2.
 * Can be set to NULL if @p ctx1 is equal to @p ctx2.
 * @param[in] value2 Second value.
 * @param[in] val_prefix_data2 Second value prefix data.
 * @return LY_SUCCESS if values are equal.
 * @return LY_ENOT if values are not equal.
 * @return LY_ERR on error.
 */
LY_ERR lyxml_value_compare(const struct ly_ctx *ctx1, const char *value1, void *val_prefix_data1,
        const struct ly_ctx *ctx2, const char *value2, void *val_prefix_data2);

#endif /* LY_XML_H_ */
