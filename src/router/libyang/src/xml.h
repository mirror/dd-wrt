/**
 * @file xml.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Public API of libyang XML parser
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_XML_H_
#define LY_XML_H_

#include <sys/types.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xmlparser XML Parser
 *
 * Simplified libyang XML parser for XML data modeled by YANG.
 *
 * @{
 */

/*
 * Structures
 */

/*
 * structure definition from context.h
 */
struct ly_ctx;

/**
 * @brief enumeration of attribute types
 */
typedef enum lyxml_attr_type {
    LYXML_ATTR_STD = 1,              /**< standard XML attribute */
    LYXML_ATTR_NS = 2,               /**< XML namespace definition */
    LYXML_ATTR_STD_UNRES = 3         /**< standard XML attribute with unresolved namespace, its ns attribute is actually prefix */
} LYXML_ATTR_TYPE;

/**
 * @brief Namespace definition.
 *
 * The structure is actually casted lyxml_attr structure which covers all
 * attributes defined in an element. The namespace definition is in this case
 * also covered by lyxml_attr structure.
 */
struct lyxml_ns {
    LYXML_ATTR_TYPE type;            /**< type of the attribute = LYXML_ATTR_NS */
    struct lyxml_ns *next;           /**< next sibling attribute */
    struct lyxml_elem *parent;       /**< parent node of the attribute */
    const char *prefix;              /**< the namespace prefix if defined, NULL for default namespace */
    const char *value;               /**< the namespace value */
};

/**
 * @brief Element's attribute definition
 *
 * The structure actually covers both the attributes as well as namespace
 * definitions.
 *
 * Attributes are being connected only into a singly linked list (compare it
 * with the elements).
 */
struct lyxml_attr {
    LYXML_ATTR_TYPE type;            /**< type of the attribute */
    struct lyxml_attr *next;         /**< next sibling attribute */
    const struct lyxml_ns *ns;       /**< pointer to the namespace of the attribute if any */
    const char *name;                /**< name of the attribute (the LocalPart of the qualified name) */
    const char *value;               /**< data stored in the attribute */
};

/**
 * @brief Structure describing an element in an XML tree.
 *
 * If the name item is NULL, then the content is part of the mixed content.
 *
 * Children elements are connected in a half ring doubly linked list:
 * - first's prev pointer points to the last children
 * - last's next pointer is NULL
 */
struct lyxml_elem {
    char flags;                      /**< special flags */
#define LYXML_ELEM_MIXED 0x01 /* element contains mixed content */
/* 0x80 is reserved and cannot be set! */

    struct lyxml_elem *parent;       /**< parent node */
    struct lyxml_attr *attr;         /**< first attribute declared in the element */
    struct lyxml_elem *child;        /**< first children element */
    struct lyxml_elem *next;         /**< next sibling node */
    struct lyxml_elem *prev;         /**< previous sibling node */

    const char *name;                /**< name of the element */
    const struct lyxml_ns *ns;       /**< namespace of the element */
    const char *content;             /**< text content of the node if any */
};

/*
 * Functions
 * Parser
 */

/**
 * @defgroup xmlreadoptions XML parser options
 * @ingroup xmlparser
 *
 * Various options to change behavior of XML read functions (lyxml_parse_*()).
 *
 * @{
 */
#define LYXML_PARSE_MULTIROOT 0x01 /**< By default, XML is supposed to be well-formed so the input file or memory chunk
                                        contains only a single XML tree. This option make parser to read multiple XML
                                        trees from a single source (regular file terminated by EOF or memory chunk
                                        terminated by NULL byte). In such a case, the returned XML element has other
                                        siblings representing the other XML trees from the source. */
#define LYXML_PARSE_NOMIXEDCONTENT 0x02 /**< By default, the parser allows elements with mixed content (text content
                                        mixed with standard XML children). This option cases to handle such elements
                                        as invalid input (#LYVE_XML_INVAL). */

/**
 * @}
 */

/**
 * @brief Parse XML from in-memory string
 *
 * @param[in] ctx libyang context to use
 * @param[in] data Pointer to a NULL-terminated string containing XML data to
 * parse.
 * @param[in] options Parser options, see @ref xmlreadoptions.
 * @return Pointer to the root of the parsed XML document tree or NULL in case of empty \p data. To free the
 *         returned data, use lyxml_free(). In these cases, the function sets #ly_errno to LY_SUCCESS. In case
 *         of error, #ly_errno contains appropriate error code (see #LY_ERR).
 */
struct lyxml_elem *lyxml_parse_mem(struct ly_ctx *ctx, const char *data, int options);

/**
 * @brief Parse XML from filesystem
 *
 * @param[in] ctx libyang context to use
 * @param[in] filename Path to the file where read data to parse
 * @param[in] options Parser options, see @ref xmlreadoptions.
 * @return Pointer to the root of the parsed XML document tree or NULL in case of empty file. To free the
 *         returned data, use lyxml_free(). In these cases, the function sets #ly_errno to LY_SUCCESS. In case
 *         of error, #ly_errno contains appropriate error code (see #LY_ERR).
 */
struct lyxml_elem *lyxml_parse_path(struct ly_ctx *ctx, const char *filename, int options);

/**
 * @defgroup xmldumpoptions XML printer options
 * @ingroup xmlparser
 *
 * Various options to change behavior of XML dump functions (lyxml_print_*()).
 *
 * When no option is specified (value 0), dumper prints all the content at once.
 *
 * @{
 */
#define LYXML_PRINT_OPEN   0x01  /**< print only the open part of the XML element.
                                   If used in combination with #LYXML_PRINT_CLOSE, it prints the element without
                                   its children: \<element/\>. If none of these two options is used, the element
                                   is printed including all its children. */
#define LYXML_PRINT_FORMAT 0x02  /**< format the output.
                                   If option is not used, the element and its children are printed without indentantion.
                                   If used in combination with #LYXML_PRINT_CLOSE or LYXML_PRINT_ATTRS or LYXML_PRINT_OPEN,
                                   it has no effect.*/
#define LYXML_PRINT_CLOSE  0x04  /**< print only the closing part of the XML element.
                                   If used in combination with #LYXML_PRINT_OPEN, it prints the element without
                                   its children: \<element/\>. If none of these two options is used, the element
                                   is printed including all its children. */
#define LYXML_PRINT_ATTRS  0x08  /**< dump only attributes and namespace declarations of the element (element name
                                   is not printed). This option cannot be used in combination with
                                   #LYXML_PRINT_OPEN and/or #LYXML_PRINT_CLOSE */
#define LYXML_PRINT_SIBLINGS 0x10/**< dump all top-level siblings. By default, the given XML element is supposed to be
                                   the only root element (and document is supposed to be well-formed XML). With this
                                   option the printer consider that the given XML element can has some sibling
                                   elements and print them all (so the given element is not necessarily printed as
                                   the first one). */
#define LYXML_PRINT_NO_LAST_NEWLINE 0x20 /**< makes sense only combined with LYXML_PRINT_FORMAT and causes the very
                                           last newline not to be printed - necessary for correct anyxml XML structure
                                           print. */

/**
 * @}
 */

/**
 * @brief Dump XML tree to a IO stream
 *
 * To write data into a file descriptor instead of file stream, use lyxml_print_fd().
 *
 * @param[in] stream IO stream to print out the tree.
 * @param[in] elem Root element of the XML tree to print
 * @param[in] options Dump options, see @ref xmldumpoptions.
 * @return number of printed characters.
 */
int lyxml_print_file(FILE * stream, const struct lyxml_elem *elem, int options);

/**
 * @brief Dump XML tree to a IO stream
 *
 * Same as lyxml_dump(), but it writes data into the given file descriptor.
 *
 * @param[in] fd File descriptor to print out the tree.
 * @param[in] elem Root element of the XML tree to print
 * @param[in] options Dump options, see @ref xmldumpoptions.
 * @return number of printed characters.
 */
int lyxml_print_fd(int fd, const struct lyxml_elem *elem, int options);

/**
 * @brief Dump XML tree to a IO stream
 *
 * Same as lyxml_dump(), but it allocates memory and store the data into it.
 * It is up to caller to free the returned string by free().
 *
 * @param[out] strp Pointer to store the resulting dump.
 * @param[in] elem Root element of the XML tree to print
 * @param[in] options Dump options, see @ref xmldumpoptions.
 * @return number of printed characters.
 */
int lyxml_print_mem(char **strp, const struct lyxml_elem *elem, int options);

/**
 * @brief Dump XML tree to a IO stream
 *
 * Same as lyxml_dump(), but it writes data via the provided callback.
 *
 * @param[in] writeclb Callback function to write the data (see write(1)).
 * @param[in] arg Optional caller-specific argument to be passed to the \p writeclb callback.
 * @param[in] elem Root element of the XML tree to print
 * @param[in] options Dump options, see @ref xmldumpoptions.
 * @return number of printed characters.
 */
int lyxml_print_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg, const struct lyxml_elem *elem, int options);

/**
 * @brief Duplicate the XML tree into the different content.
 *
 * Date parser requires to have the input XML tree in the same context as the resulting data tree. Therefore,
 * if you need to parse a single XML tree into a different contexts, you have to duplicate the source XML
 * tree into the required context first.
 *
 * @param[in] ctx Target context for the result.
 * @param[in] root Root node of the XML tree to duplicate. If an internal node is provided,
 *            the parents are not duplicated and only the specified subtree is duplicated.
 * @result Pointer to the duplicated tree or NULL on error.
 */
struct lyxml_elem *lyxml_dup(struct ly_ctx *ctx, struct lyxml_elem *root);

/**
 * @brief Free (and unlink from the XML tree) the specified element with all
 * its attributes and namespace definitions.
 *
 * @param[in] ctx libyang context to use
 * @param[in] elem Pointer to the element to free.
 */
void lyxml_free(struct ly_ctx *ctx, struct lyxml_elem *elem);

/**
 * @brief Free (and unlink from the XML tree) the specified (sub)tree with all
 * its attributes and namespace definitions. In contrast to lyxml_free(), free also
 * all the element's siblings (preceding as well as following).
 *
 * @param[in] ctx libyang context to use
 * @param[in] elem Pointer to the element to free.
 */
void lyxml_free_withsiblings(struct ly_ctx *ctx, struct lyxml_elem *elem);

/**
 * @brief Unlink the element from its parent. In contrast to lyxml_free(),
 * after return the caller can still manipulate with the elem. Any namespaces
 * are corrected and copied, if needed.
 *
 * @param[in] ctx libyang context to use.
 * @param[in] elem Element to unlink from its parent (if any).
 */
void lyxml_unlink(struct ly_ctx *ctx, struct lyxml_elem *elem);

/**
 * @brief Get value of the attribute in the specified element.
 */
const char *lyxml_get_attr(const struct lyxml_elem *elem, const char *name, const char *ns);

/**
 * @brief Get namespace definition of the given prefix in context of the specified element.
 *
 * @param[in] elem Element where start namespace searching
 * @param[in] prefix Prefix of the namespace to search for
 * @return Namespace definition or NULL if no such namespace exists
 */
const struct lyxml_ns *lyxml_get_ns(const struct lyxml_elem *elem, const char *prefix);

/**@}*/

#ifdef __cplusplus
}
#endif

#endif /* LY_XML_H_ */
