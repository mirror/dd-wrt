/**
 * @file xml_internal.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Internal part of libyang XML parser
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_XML_INTERNAL_H_
#define LY_XML_INTERNAL_H_

#include <stdio.h>
#include "xml.h"
#include "printer.h"

/*
 * Macro to test if character is #x20 | #x9 | #xA | #xD (whitespace)
 */
#define is_xmlws(c) (c == 0x20 || c == 0x9 || c == 0xa || c == 0xd)

#define is_xmlnamestartchar(c) ((c >= 'a' && c <= 'z') || c == '_' || \
        (c >= 'A' && c <= 'Z') || c == ':' || \
        (c >= 0x370 && c <= 0x1fff && c != 0x37e ) || \
        (c >= 0xc0 && c <= 0x2ff && c != 0xd7 && c != 0xf7) || c == 0x200c || \
        c == 0x200d || (c >= 0x2070 && c <= 0x218f) || \
        (c >= 0x2c00 && c <= 0x2fef) || (c >= 0x3001 && c <= 0xd7ff) || \
        (c >= 0xf900 && c <= 0xfdcf) || (c >= 0xfdf0 && c <= 0xfffd) || \
        (c >= 0x10000 && c <= 0xeffff))

#define is_xmlnamechar(c) ((c >= 'a' && c <= 'z') || c == '_' || c == '-' || \
        (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == ':' || \
        c == '.' || c == 0xb7 || (c >= 0x370 && c <= 0x1fff && c != 0x37e ) ||\
        (c >= 0xc0 && c <= 0x2ff && c != 0xd7 && c != 0xf7) || c == 0x200c || \
        c == 0x200d || (c >= 0x300 && c <= 0x36f) || \
        (c >= 0x2070 && c <= 0x218f) || (c >= 0x2030f && c <= 0x2040) || \
        (c >= 0x2c00 && c <= 0x2fef) || (c >= 0x3001 && c <= 0xd7ff) || \
        (c >= 0xf900 && c <= 0xfdcf) || (c >= 0xfdf0 && c <= 0xfffd) || \
        (c >= 0x10000 && c <= 0xeffff))

/*
 * Functions
 * Tree Manipulation
 */

/**
 * @brief Add a child element into a parent element.
 *
 * The child is added as a last child.
 *
 * @param[in] ctx libyang context to use.
 * @param[in] parent Element where to add the child.
 * @param[in] child Element to be added as a last child of the parent.
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int lyxml_add_child(struct ly_ctx *ctx, struct lyxml_elem *parent, struct lyxml_elem *child);

/* copy_ns: 0 - set invalid namespaces to NULL, 1 - copy them into this subtree */
void lyxml_correct_elem_ns(struct ly_ctx *ctx, struct lyxml_elem *elem, struct lyxml_elem *orig, int copy_ns,
                           int correct_attrs);

struct lyxml_elem *lyxml_dup_elem(struct ly_ctx *ctx, struct lyxml_elem *elem,
                                  struct lyxml_elem *parent, int recursive, int with_siblings);

/**
 * @brief Free attribute. Includes unlinking from an element if the attribute
 * is placed anywhere.
 *
 * @param[in] ctx libyang context to use
 * @param[in] parent Parent element where the attribute is placed
 * @param[in] attr Attribute to free.
 */
void lyxml_free_attr(struct ly_ctx *ctx, struct lyxml_elem *parent, struct lyxml_attr *attr);

/**
 * @brief Free (and unlink from their element) all attributes (including
 * namespace definitions) of the specified element.
 *
 * @param[in] elem Element to modify.
 */
void lyxml_free_attrs(struct ly_ctx *ctx, struct lyxml_elem *elem);

/**
 * @brief Unlink the attribute from its parent element. In contrast to
 * lyxml_free_attr(), after return the caller can still manipulate with the
 * attr.
 *
 * @param[in] attr Attribute to unlink from its parent (if any).
 */
void lyxml_unlink_attr(struct lyxml_attr *attr);

/**
 * @brief Unlink the element from its parent. In contrast to lyxml_free_elem(),
 * after return the caller can still manipulate with the elem.
 *
 * @param[in] ctx libyang context to use.
 * @param[in] elem Element to unlink from its parent (if any).
 * @param[in] copy_ns 0 sets NS of \p elem and children that are defined
 * outside \p elem subtree to NULL,
 * 1 corrects NS of \p elem and children that are defined outside \p elem
 * subtree (copy NS and update pointer),
 * 2 skips any NS checking.
 *
 */
void lyxml_unlink_elem(struct ly_ctx *ctx, struct lyxml_elem *elem, int copy_ns);

/**
 * @brief Get the first UTF-8 character value (4bytes) from buffer
 * @param[in] ctx Context to store errors in.
 * @param[in] buf Pointer to the current position in input buffer.
 * @param[out] read Number of processed bytes in buf (length of UTF-8
 * character).
 * @return UTF-8 value as 4 byte number. 0 means error, only UTF-8 characters
 * valid for XML are returned, so:
 * #x9 | #xA | #xD | [#x20-#xD7FF] | [#xE000-#xFFFD] | [#x10000-#x10FFFF]
 * = any Unicode character, excluding the surrogate blocks, FFFE, and FFFF.
 *
 * UTF-8 mapping:
 * 00000000 -- 0000007F:    0xxxxxxx
 * 00000080 -- 000007FF:    110xxxxx 10xxxxxx
 * 00000800 -- 0000FFFF:    1110xxxx 10xxxxxx 10xxxxxx
 * 00010000 -- 001FFFFF:    11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 */
int lyxml_getutf8(struct ly_ctx *ctx, const char *buf, unsigned int *read);

/**
 * @brief Types of the XML data
 */
typedef enum lyxml_data_type {
    LYXML_DATA_ATTR = 1,   /**< XML attribute data */
    LYXML_DATA_ELEM = 2    /**< XML element data */
} LYXML_DATA_TYPE;

/**
 * @brief Dump XML text. Converts special characters to their equivalent
 * starting with '&'.
 * @param[in] out Output structure.
 * @param[in] text Text to dump.
 * @return Number of dumped characters.
 */
int lyxml_dump_text(struct lyout *out, const char *text, LYXML_DATA_TYPE type);

#endif /* LY_XML_INTERNAL_H_ */
