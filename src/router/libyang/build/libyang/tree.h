/**
 * @file tree.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang generic macros and functions to work with YANG schema or data trees.
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_TREE_H_
#define LY_TREE_H_

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @page howtoXPath XPath Addressing
 *
 * Internally, XPath evaluation is performed on __when__ and __must__ conditions in the schema. For that almost
 * a full [XPath 1.0](http://www.w3.org/TR/1999/REC-xpath-19991116/) evaluator was implemented.
 * In YANG models you can also find paths identifying __augment__ targets, __leafref__ targets, and trivial paths in
 * __choice default__ and __unique__ statements argument. The exact format of all those paths can be found in the
 * relevant RFCs. Further will only be discussed paths that are used directly in libyang API functions.
 *
 * XPath
 * =====
 *
 * Generally, any xpath argument expects an expression similar to _when_ or _must_ as the same evaluator is used. As for
 * the format of any prefixes, the standardized JSON ([RFC 7951](https://tools.ietf.org/html/rfc7951#section-6.11))
 * was used. Summarized, xpath follows these conventions:
 *   - full XPath can be used, but only data nodes (node sets) will always be returned,
 *   - as per the specification, prefixes are actually __module names__,
 *   - also in the specification, for _absolute_ paths, the first (leftmost) node _MUST_ have a prefix,
 *   - for _relative_ paths, you specify the __context node__, which then acts as a parent for the first node in the path,
 *   - nodes always inherit their module (prefix) from their __parent node__ so whenever a node is from a different
 *     module than its parent, it _MUST_ have a prefix,
 *   - nodes from the same module as their __parent__ _MUST NOT_ have a prefix,
 *   - note that non-data nodes/schema-only node (choice, case, uses, input, output) are skipped and _MUST_ not be
 *     included in the path.
 *
 * Functions List
 * --------------
 * - ::lyd_find_xpath()
 * - ::lys_find_xpath()
 *
 * Path
 * ====
 *
 * The term path is used when a simplified (subset of) XPath is expected. Path is always a valid XPath but not
 * the other way around. In short, paths only identify a specific (set of) nodes based on their ancestors in the
 * schema. Predicates are allowed the same as for an [instance-identifier](https://tools.ietf.org/html/rfc7950#section-9.13).
 * Specifically, key values of a list, leaf-list value, or position of lists without keys can be used.
 *
 * Examples
 * --------
 *
 * - get __list__ instance with __key1__ of value __1__ and __key2__ of value __2__ (this can return more __list__ instances if there are more keys than __key1__ and __key2__)
 *
 *       /module-name:container/list[key1='1'][key2='2']
 *
 * - get __leaf-list__ instance with the value __val__
 *
 *       /module-name:container/leaf-list[.='val']
 *
 * - get __3rd list-without-keys__ instance with no keys defined
 *
 *       /module-name:container/list-without-keys[3]
 *
 * - get __aug-list__ with __aug-list-key__, which was added to __module-name__ from an augment module __augment-module__
 *
 *       /module-name:container/container2/augment-module:aug-cont/aug-list[aug-list-key='value']
 *
 * Functions List
 * --------------
 * - ::lyd_new_path()
 * - ::lyd_new_path2()
 * - ::lyd_path()
 * - ::lyd_find_path()
 * - ::lys_find_path()
 *
 */

/**
 * @defgroup trees Trees
 *
 * Generic macros, functions, etc. to work with both [schema](@ref schematree) and [data](@ref datatree) trees.
 *
 * @{
 */

/**
 * @brief Type (i.e. size) of the [sized array](@ref sizedarrays)'s size counter.
 *
 * To print the value via a print format, use LY_PRI_ARRAY_COUNT_TYPE specifier.
 */
#define LY_ARRAY_COUNT_TYPE uint64_t

/**
 * @brief Printing format specifier macro for LY_ARRAY_SIZE_TYPE values.
 */
#define LY_PRI_ARRAY_COUNT_TYPE PRIu64

/**
 * @brief Macro selector for other LY_ARRAY_* macros, do not use directly!
 */
#define LY_ARRAY_SELECT(_1, _2, NAME, ...) NAME

/**
 * @brief Helper macro to go through sized-arrays with a pointer iterator.
 *
 * Use with opening curly bracket (`{`).
 *
 * @param[in] ARRAY Array to go through
 * @param[in] TYPE Type of the records in the ARRAY
 * @param[out] ITER Iterating pointer to the item being processed in each loop
 */
#define LY_ARRAY_FOR_ITER(ARRAY, TYPE, ITER) \
    for (ITER = ARRAY; \
         (ARRAY) && ((char *)ITER - (char *)ARRAY)/(sizeof(TYPE)) < (*((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1)); \
         ITER = (TYPE*)ITER + 1)

/**
 * @brief Helper macro to go through sized-arrays with a numeric iterator.
 *
 * Use with opening curly bracket (`{`).
 *
 * The item on the current INDEX in the ARRAY can be accessed in a standard C way as ARRAY[INDEX].
 *
 * @param[in] ARRAY Array to go through
 * @param[out] INDEX Variable for the iterating index of the item being processed in each loop
 */
#define LY_ARRAY_FOR_INDEX(ARRAY, INDEX) \
    for (INDEX = 0; \
         INDEX < LY_ARRAY_COUNT(ARRAY); \
         ++INDEX)

/**
 * @brief Get the number of records in the ARRAY.
 */
#define LY_ARRAY_COUNT(ARRAY) (ARRAY ? (*((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1)) : 0)

/**
 * @brief Sized-array iterator (for-loop).
 *
 * Use with opening curly bracket (`{`).
 *
 * There are 2 variants:
 *
 *     LY_ARRAY_FOR(ARRAY, TYPE, ITER)
 *
 * Where ARRAY is a sized-array to go through, TYPE is the type of the items in the ARRAY and ITER is a pointer variable
 * providing the items of the ARRAY in the loops. This functionality is provided by LY_ARRAY_FOR_ITER macro
 *
 *     LY_ARRAY_FOR(ARRAY, INDEX)
 *
 * The ARRAY is again a sized-array to go through, the INDEX is a variable (LY_ARRAY_COUNT_TYPE) for storing iterating ARRAY's index
 * to access the items of ARRAY in the loops. This functionality is provided by LY_ARRAY_FOR_INDEX macro.
 */
#define LY_ARRAY_FOR(ARRAY, ...) LY_ARRAY_SELECT(__VA_ARGS__, LY_ARRAY_FOR_ITER, LY_ARRAY_FOR_INDEX, LY_UNDEF)(ARRAY, __VA_ARGS__)

/**
 * @brief Macro to iterate via all sibling elements without affecting the list itself
 *
 * Works for all types of nodes despite it is data or schema tree, but all the
 * parameters must be pointers to the same type.
 *
 * Use with opening curly bracket (`{`). All parameters must be of the same type.
 *
 * @param START Pointer to the starting element.
 * @param ELEM Iterator.
 */
#define LY_LIST_FOR(START, ELEM) \
    for ((ELEM) = (START); \
         (ELEM); \
         (ELEM) = (ELEM)->next)

/**
 * @brief Macro to iterate via all sibling elements allowing to modify the list itself (e.g. removing elements)
 *
 * Use with opening curly bracket (`{`). All parameters must be of the same type.
 *
 * @param START Pointer to the starting element.
 * @param NEXT Temporary storage to allow removing of the current iterator content.
 * @param ELEM Iterator.
 */
#define LY_LIST_FOR_SAFE(START, NEXT, ELEM) \
    for ((ELEM) = (START); \
         (ELEM) ? (NEXT = (ELEM)->next, 1) : 0; \
         (ELEM) = (NEXT))

/**
 * @brief YANG built-in types
 */
typedef enum {
    LY_TYPE_UNKNOWN = 0, /**< Unknown type */
    LY_TYPE_BINARY, /**< Any binary data ([RFC 6020 sec 9.8](http://tools.ietf.org/html/rfc6020#section-9.8)) */
    LY_TYPE_UINT8, /**< 8-bit unsigned integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_UINT16, /**< 16-bit unsigned integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_UINT32, /**< 32-bit unsigned integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_UINT64, /**< 64-bit unsigned integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_STRING, /**< Human-readable string ([RFC 6020 sec 9.4](http://tools.ietf.org/html/rfc6020#section-9.4)) */
    LY_TYPE_BITS, /**< A set of bits or flags ([RFC 6020 sec 9.7](http://tools.ietf.org/html/rfc6020#section-9.7)) */
    LY_TYPE_BOOL, /**< "true" or "false" ([RFC 6020 sec 9.5](http://tools.ietf.org/html/rfc6020#section-9.5)) */
    LY_TYPE_DEC64, /**< 64-bit signed decimal number ([RFC 6020 sec 9.3](http://tools.ietf.org/html/rfc6020#section-9.3))*/
    LY_TYPE_EMPTY, /**< A leaf that does not have any value ([RFC 6020 sec 9.11](http://tools.ietf.org/html/rfc6020#section-9.11)) */
    LY_TYPE_ENUM, /**< Enumerated strings ([RFC 6020 sec 9.6](http://tools.ietf.org/html/rfc6020#section-9.6)) */
    LY_TYPE_IDENT, /**< A reference to an abstract identity ([RFC 6020 sec 9.10](http://tools.ietf.org/html/rfc6020#section-9.10)) */
    LY_TYPE_INST, /**< References a data tree node ([RFC 6020 sec 9.13](http://tools.ietf.org/html/rfc6020#section-9.13)) */
    LY_TYPE_LEAFREF, /**< A reference to a leaf instance ([RFC 6020 sec 9.9](http://tools.ietf.org/html/rfc6020#section-9.9))*/
    LY_TYPE_UNION, /**< Choice of member types ([RFC 6020 sec 9.12](http://tools.ietf.org/html/rfc6020#section-9.12)) */
    LY_TYPE_INT8, /**< 8-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_INT16, /**< 16-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_INT32, /**< 32-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_INT64  /**< 64-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
} LY_DATA_TYPE;
#define LY_DATA_TYPE_COUNT 20 /**< Number of different types */

/**
 * @brief Stringfield YANG built-in data types
 */
extern const char *ly_data_type2str[LY_DATA_TYPE_COUNT];

/**
 * @brief All kinds of supported value formats and prefix mappings to modules.
 */
typedef enum {
    LY_VALUE_CANON,           /**< canonical value, prefix mapping is type-specific */
    LY_VALUE_SCHEMA,          /**< YANG schema value, prefixes map to YANG import prefixes */
    LY_VALUE_SCHEMA_RESOLVED, /**< resolved YANG schema value, prefixes map to module structures directly */
    LY_VALUE_XML,             /**< XML data value, prefixes map to XML namespace prefixes */
    LY_VALUE_JSON,            /**< JSON data value, prefixes map to module names */
    LY_VALUE_LYB,             /**< LYB data binary value, prefix mapping is type-specific (but usually like JSON) */
    LY_VALUE_STR_NS           /**< any data format value, prefixes map to XML namespace prefixes */
} LY_VALUE_FORMAT;

/** @} trees */

#ifdef __cplusplus
}
#endif

#endif /* LY_TREE_H_ */
