/**
 * @file plugins_types.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief API for (user) types plugins
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PLUGINS_TYPES_H_
#define LY_PLUGINS_TYPES_H_

#include <stddef.h>
#include <stdint.h>

#include "config.h"
#include "log.h"
#include "plugins.h"
#include "tree.h"

#include "tree_edit.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ly_ctx;
struct ly_path;
struct lyd_node;
struct lyd_value;
struct lyd_value_xpath10;
struct lys_module;
struct lys_glob_unres;
struct lysc_ident;
struct lysc_node;
struct lysc_pattern;
struct lysc_range;
struct lysc_type;
struct lysc_type_bits;
struct lysc_type_leafref;

/**
 * @page howtoPluginsTypes Type Plugins
 *
 * Note that the part of the libyang API here is available only by including a separated `<libyang/plugins_types.h>` header
 * file. Also note that the type plugins API is versioned separately from libyang itself, so backward incompatible changes
 * can come even without changing libyang major version.
 *
 * YANG allows to define new data types via *typedef* statements or even in leaf's/leaf-list's *type* statements.
 * Such types are derived (directly or indirectly) from a set of [YANG built-in types](https://tools.ietf.org/html/rfc7950#section-4.2.4).
 * libyang implements all handling of the data values of the YANG types via the Type Plugins API. Internally, there is
 * implementation of the built-in types and others can be added as an external plugin (see @ref howtoPlugins).
 *
 * Type plugin is supposed to
 *  - store (and canonize) data value,
 *  - validate it according to the type's restrictions,
 *  - compare two values (::lyd_value) of the same type,
 *  - duplicate value (::lyd_value),
 *  - print it and
 *  - free the specific data inserted into ::lyd_value.
 *
 * These tasks are implemented as callbacks provided to libyang via ::lyplg_type_record structures defined as array using
 * ::LYPLG_TYPES macro.
 *
 * All the callbacks are supposed to do not log directly via libyang logger. Instead, they return ::LY_ERR value and
 * ::ly_err_item error structure(s) describing the detected error(s) (helper functions ::ly_err_new() and ::ly_err_free()
 * are available).
 *
 * The main functionality is provided via ::lyplg_type_store_clb callback responsible for canonizing and storing
 * provided string representation of the value in specified format (XML and JSON supported). Valid value is stored in
 * ::lyd_value structure - its union allows to store data as one of the predefined type or in a custom form behind
 * the void *ptr member of ::lyd_value structure. The callback is also responsible for storing canonized string
 * representation of the value as ::lyd_value._canonical. If the type does not define canonical representation, the original
 * representation is stored. In case there are any differences between the representation in specific input types, the plugin
 * is supposed to store the value in JSON representation - typically, the difference is in prefix representation and JSON
 * format uses directly the module names as prefixes.
 *
 * Usually, all the validation according to the type's restrictions is done in the store callback. However, in case the type
 * requires some validation referencing other entities in the data tree, the optional validation callback
 * ::lyplg_type_validate_clb can be implemented.
 *
 * The stored values can be compared in a specific way by providing ::lyplg_type_compare_clb. In case the best way to compare
 * the values is to compare their canonical string representations, the ::lyplg_type_compare_simple() function can be used.
 *
 * Data duplication is done with ::lyplg_type_dup_clb callbacks. Note that the callback is responsible even for duplicating
 * the ::lyd_value._canonical, so the callback must be always present (the canonical value is always present). If there is
 * nothing else to duplicate, the plugin can use the generic ::lyplg_type_dup_simple().
 *
 * The stored value can be printed into the required format via ::lyplg_type_print_clb implementation. Simple printing
 * canonical representation of the value is implemented by ::lyplg_type_print_simple().
 *
 * And finally freeing any data stored in the ::lyd_value by the plugin is done by implementation of ::lyplg_type_free_clb.
 * Freeing only the canonical string is implemented by ::lyplg_type_free_simple().
 *
 * The plugin information contains also the plugin identifier (::lyplg_type.id). This string can serve to identify the
 * specific plugin responsible to storing data value. In case the user can recognize the id string, it can access the
 * plugin specific data with the appropriate knowledge of its structure.
 *
 * Besides the mentioned `_simple` functions, libyang provides, as part of the type plugins API, all the callbacks
 * implementing the built-in types in the internal plugins:
 *
 *  - [simple callbacks](@ref pluginsTypesSimple) handling only the canonical strings in the value,
 *  - [binary built-in type](@ref pluginsTypesBinary)
 *  - [bits built-in type](@ref pluginsTypesBits)
 *  - [boolean built-in type](@ref pluginsTypesBoolean)
 *  - [decimal64 built-in type](@ref pluginsTypesDecimal64)
 *  - [empty built-in type](@ref pluginsTypesEmpty)
 *  - [enumeration built-in type](@ref pluginsTypesEnumeration)
 *  - [identityref built-in type](@ref pluginsTypesIdentityref)
 *  - [instance-identifier built-in type](@ref pluginsTypesInstanceid)
 *  - [integer built-in types](@ref pluginsTypesInteger)
 *  - [leafref built-in type](@ref pluginsTypesLeafref)
 *  - [string built-in type](@ref pluginsTypesString)
 *  - [union built-in type](@ref pluginsTypesUnion)
 *
 * And one derived type:
 *
 *  - [xpath1.0 `ietf-yang-types` type](@ref pluginsTypesXpath10)
 *
 * In addition to these callbacks, the API also provides several functions which can help to implement your own plugin for the
 * derived YANG types:
 *
 * - ::ly_err_new()
 * - ::ly_err_free()
 *
 * - ::lyplg_type_lypath_new()
 * - ::lyplg_type_lypath_free()
 *
 * - ::lyplg_type_prefix_data_new()
 * - ::lyplg_type_prefix_data_dup()
 * - ::lyplg_type_prefix_data_free()
 * - ::lyplg_type_get_prefix()
 *
 * - ::lyplg_type_check_hints()
 * - ::lyplg_type_check_status()
 * - ::lyplg_type_lypath_check_status()
 * - ::lyplg_type_identity_isderived()
 * - ::lyplg_type_identity_module()
 * - ::lyplg_type_make_implemented()
 * - ::lyplg_type_parse_dec64()
 * - ::lyplg_type_parse_int()
 * - ::lyplg_type_parse_uint()
 * - ::lyplg_type_resolve_leafref()
 */

/**
 * @defgroup pluginsTypes Plugins: Types
 * @{
 *
 * Structures and functions to for libyang plugins implementing specific YANG types defined in YANG modules. For more
 * information, see @ref howtoPluginsTypes.
 *
 * This part of libyang API is available by including `<libyang/plugins_types.h>` header file.
 */

/**
 * @brief Type API version
 */
#define LYPLG_TYPE_API_VERSION 1

/**
 * @brief Macro to define plugin information in external plugins
 *
 * Use as follows:
 * LYPLG_TYPES = {{<filled information of ::lyplg_type_record>}, ..., {0}};
 */
#define LYPLG_TYPES \
    uint32_t plugins_types_apiver__ = LYPLG_TYPE_API_VERSION; \
    const struct lyplg_type_record plugins_types__[]

/**
 * @brief Check whether specific type value needs to be allocated dynamically.
 *
 * @param[in] type_val Pointer to specific type value storage.
 */
#define LYPLG_TYPE_VAL_IS_DYN(type_val) \
    (sizeof *(type_val) > LYD_VALUE_FIXED_MEM_SIZE)

/**
 * @brief Prepare value memory for storing a specific type value, may be allocated dynamically.
 *
 * Must be called for values larger than 8 bytes.
 * To be used in ::lyplg_type_store_clb.
 *
 * @param[in] storage Pointer to the value storage to use (struct ::lyd_value *).
 * @param[in,out] type_val Pointer to specific type value structure.
 */
#define LYPLG_TYPE_VAL_INLINE_PREPARE(storage, type_val) \
    (LYPLG_TYPE_VAL_IS_DYN(type_val) \
     ? ((type_val) = ((storage)->dyn_mem = calloc(1, sizeof *(type_val)))) \
     : ((type_val) = memset((storage)->fixed_mem, 0, sizeof *(type_val))))

/**
 * @brief Destroy a prepared value.
 *
 * Must be called for values prepared with ::LYPLG_TYPE_VAL_INLINE_PREPARE.
 *
 * @param[in] type_val Pointer to specific type value structure.
 */
#define LYPLG_TYPE_VAL_INLINE_DESTROY(type_val) \
    do { if (LYPLG_TYPE_VAL_IS_DYN(type_val)) free(type_val); } while(0)

/**
 * @brief Create and fill error structure.
 *
 * Helper function for various plugin functions to generate error information structure.
 *
 * @param[in, out] err Pointer to store a new error structure filled according to the input parameters. If the storage
 * already contains error information, the new record is appended into the errors list.
 * @param[in] ecode Code of the error to fill. In case LY_SUCCESS value, nothing is done and LY_SUCCESS is returned.
 * @param[in] vecode Validity error code in case of LY_EVALID error code.
 * @param[in] path Path to the node causing the error.
 * @param[in] apptag Error-app-tag value.
 * @param[in] err_format Format string (same like at printf) or string literal.
 * If you want to print just an unknown string, use "%s" for the @p err_format, otherwise undefined behavior may occur
 * because the unknown string may contain the % character, which is interpreted as conversion specifier.
 * @return The given @p ecode value if the @p err is successfully created. The structure can be freed using ::ly_err_free()
 * or passed back from callback into libyang.
 * @return LY_EMEM If there is not enough memory for allocating error record, the @p err is not touched in that case.
 * @return LY_SUCCESS if @p ecode is LY_SUCCESS, the @p err is not touched in this case.
 */
LIBYANG_API_DECL LY_ERR ly_err_new(struct ly_err_item **err, LY_ERR ecode, LY_VECODE vecode, char *path, char *apptag,
        const char *err_format, ...) _FORMAT_PRINTF(6, 7);

/**
 * @brief Destructor for the error records created with ::ly_err_new().
 *
 * Compatible with the free(), so usable as a generic callback.
 *
 * @param[in] ptr Error record (::ly_err_item, the void pointer is here only for compatibility with a generic free()
 * function) to free. With the record, also all the records (if any) connected after this one are freed.
 */
LIBYANG_API_DECL void ly_err_free(void *ptr);

/**
 * @brief Check that the type is suitable for the parser's hints (if any) in the specified format
 *
 * Use only in implementations of ::lyplg_type_store_clb which provide all the necessary parameters for this function.
 *
 * @param[in] hints Bitmap of [value hints](@ref lydvalhints) of all the allowed value types provided by parsers
 *            to ::lyplg_type_store_clb.
 * @param[in] value Lexical representation of the value to be stored.
 * @param[in] value_len Length (number of bytes) of the given \p value.
 * @param[in] type Expected base type of the @p value by the caller.
 * @param[out] base Pointer to store the numeric base for parsing numeric values using strtol()/strtoll() function.
 * Returned (and required) only for numeric @p type values.
 * @param[out] err Pointer to store error information in case of failure.
 * @return LY_ERR value
 */
LIBYANG_API_DECL LY_ERR lyplg_type_check_hints(uint32_t hints, const char *value, size_t value_len, LY_DATA_TYPE type,
        int *base, struct ly_err_item **err);

/**
 * @brief Check that the value of a type is allowed based on its status.
 *
 * @param[in] ctx_node Context node (which references the value).
 * @param[in] val_flags Flags fo the value.
 * @param[in] format Format of the value.
 * @param[in] prefix_data Prefix data of the value.
 * @param[in] val_name Name of the value, only for logging.
 * @param[out] err Pointer to store error information in case of failure.
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_check_status(const struct lysc_node *ctx_node, uint16_t val_flags, LY_VALUE_FORMAT format,
        void *prefix_data, const char *val_name, struct ly_err_item **err);

/**
 * @brief Check that the lypath instance-identifier value is allowed based on the status of the nodes.
 *
 * @param[in] ctx_node Context node (which references the value).
 * @param[in] path Path of the instance-identifier.
 * @param[in] format Format of the value.
 * @param[in] prefix_data Prefix data of the value.
 * @param[out] err Pointer to store error information in case of failure.
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_lypath_check_status(const struct lysc_node *ctx_node, const struct ly_path *path,
        LY_VALUE_FORMAT format, void *prefix_data, struct ly_err_item **err);

/**
 * @brief Get the corresponding module for the identity value.
 *
 * Use only in implementations of ::lyplg_type_store_clb which provide all the necessary parameters for this function.
 *
 * @param[in] ctx libyang context.
 * @param[in] ctx_node Schema node where the value is instantiated to determine the module in case of unprefixed value
 * in specific @p format.
 * @param[in] prefix Prefix to resolve - identified beginning of a prefix in ::lyplg_type_store_clb's value parameter.
 * If NULL, an unprefixed identity is resolved.
 * @param[in] prefix_len Length of @p prefix.
 * @param[in] format Format of the prefix (::lyplg_type_store_clb's format parameter).
 * @param[in] prefix_data Format-specific data (::lyplg_type_store_clb's prefix_data parameter).
 * @return Resolved prefix module,
 * @return NULL otherwise.
 */
LIBYANG_API_DECL const struct lys_module *lyplg_type_identity_module(const struct ly_ctx *ctx,
        const struct lysc_node *ctx_node, const char *prefix, size_t prefix_len, LY_VALUE_FORMAT format,
        const void *prefix_data);

/**
 * @brief Implement a module (just like ::lys_set_implemented()), but keep maintaining unresolved items.
 *
 * Use only in implementations of ::lyplg_type_store_clb which provide all the necessary parameters for this function.
 *
 * @param[in] mod Module to implement.
 * @param[in] features Array of features to enable.
 * @param[in,out] unres Global unres to add to.
 * @return LY_ERECOMPILE if the context need to be recompiled, should be returned.
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_make_implemented(struct lys_module *mod, const char **features,
        struct lys_glob_unres *unres);

/**
 * @brief Get the bitmap size of a bits value bitmap.
 *
 * Bitmap size is rounded up to the smallest integer size (1, 2, 4, or 8 bytes).
 * If more than 8 bytes are needed to hold all the bit positions, no rounding is performed.
 *
 * @param[in] type Bits type.
 * @return Bitmap size in bytes.
 */
LIBYANG_API_DECL size_t lyplg_type_bits_bitmap_size(const struct lysc_type_bits *type);

/**
 * @brief Check whether a particular bit of a bitmap is set.
 *
 * @param[in] bitmap Bitmap to read from.
 * @param[in] size Size of @p bitmap.
 * @param[in] bit_position Bit position to check.
 * @return Whether the bit is set or not.
 */
LIBYANG_API_DECL ly_bool lyplg_type_bits_is_bit_set(const char *bitmap, size_t size, uint32_t bit_position);

/**
 * @brief Print xpath1.0 token in the specific format.
 *
 * @param[in] token Token to transform.
 * @param[in] tok_len Lenghth of @p token.
 * @param[in] is_nametest Whether the token is a nametest, it then always requires a prefix in XML @p get_format.
 * @param[in,out] context_mod Current context module, may be updated.
 * @param[in] resolve_ctx Context to use for resolving prefixes.
 * @param[in] resolve_format Format of the resolved prefixes.
 * @param[in] resolve_prefix_data Resolved prefixes prefix data.
 * @param[in] get_format Format of the output prefixes.
 * @param[in] get_prefix_data Format-specific prefix data for the output.
 * @param[out] token_p Printed token.
 * @param[out] err Error structure on error.
 * @return LY_ERR value.
 */
LIBYANG_API_DEF LY_ERR lyplg_type_xpath10_print_token(const char *token, uint16_t tok_len, ly_bool is_nametest,
        const struct lys_module **context_mod, const struct ly_ctx *resolve_ctx, LY_VALUE_FORMAT resolve_format,
        const void *resolve_prefix_data, LY_VALUE_FORMAT get_format, void *get_prefix_data, char **token_p,
        struct ly_err_item **err);

/**
 * @brief Get format-specific prefix for a module.
 *
 * Use only in implementations of ::lyplg_type_print_clb which provide all the necessary parameters for this function.
 *
 * @param[in] mod Module whose prefix to get - the module somehow connected with the value to print.
 * @param[in] format Format of the prefix (::lyplg_type_print_clb's format parameter).
 * @param[in] prefix_data Format-specific data (::lyplg_type_print_clb's prefix_data parameter).
 * @return Module prefix to print.
 * @return NULL on using the current module/namespace.
 */
LIBYANG_API_DECL const char *lyplg_type_get_prefix(const struct lys_module *mod, LY_VALUE_FORMAT format, void *prefix_data);

/**
 * @brief Store used prefixes in a string into an internal libyang structure used in ::lyd_value.
 *
 * Use only in implementations of ::lyplg_type_store_clb which provide all the necessary parameters for this function.
 *
 * If @p prefix_data_p are non-NULL, they are treated as valid according to the @p format_p and new possible
 * prefixes are simply added. This way it is possible to store prefix data for several strings together.
 *
 * @param[in] ctx libyang context.
 * @param[in] value Value to be parsed.
 * @param[in] value_len Length of @p value.
 * @param[in] format Format of the prefixes in the value.
 * @param[in] prefix_data Format-specific data for resolving any prefixes (see ly_resolve_prefix()).
 * @param[in,out] format_p Resulting format of the prefixes.
 * @param[in,out] prefix_data_p Resulting prefix data for the value in format @p format_p.
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_prefix_data_new(const struct ly_ctx *ctx, const void *value, size_t value_len,
        LY_VALUE_FORMAT format, const void *prefix_data, LY_VALUE_FORMAT *format_p, void **prefix_data_p);
/**
 * @brief Duplicate prefix data.
 *
 * Use only in implementations of ::lyplg_type_store_clb which provide all the necessary parameters for this function.
 *
 * @param[in] ctx libyang context.
 * @param[in] format Format of the prefixes in the value.
 * @param[in] orig Prefix data to duplicate.
 * @param[out] dup Duplicated prefix data.
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_prefix_data_dup(const struct ly_ctx *ctx, LY_VALUE_FORMAT format, const void *orig,
        void **dup);

/**
 * @brief Free internal prefix data.
 *
 * Use only in implementations of ::lyplg_type_store_clb which provide all the necessary parameters for this function.
 *
 * @param[in] format Format of the prefixes.
 * @param[in] prefix_data Format-specific data to free.
 */
LIBYANG_API_DECL void lyplg_type_prefix_data_free(LY_VALUE_FORMAT format, void *prefix_data);

/**
 * @brief Helper function to create internal schema path representation for instance-identifier value representation.
 *
 * Use only in implementations of ::lyplg_type_store_clb which provide all the necessary parameters for this function.
 *
 * @param[in] ctx libyang Context
 * @param[in] value Lexical representation of the value to be stored.
 * @param[in] value_len Length (number of bytes) of the given @p value.
 * @param[in] options [Type plugin store options](@ref plugintypestoreopts).
 * @param[in] format Input format of the value.
 * @param[in] prefix_data Format-specific data for resolving any prefixes (see ly_resolve_prefix()).
 * @param[in] ctx_node The @p value schema context node.
 * @param[in,out] unres Global unres structure for newly implemented modules.
 * @param[out] path Pointer to store the created structure representing the schema path from the @p value.
 * @param[out] err Pointer to store the error information provided in case of failure.
 * @return LY_SUCCESS on success,
 * @return LY_ERECOMPILE if the context need to be recompiled, should be returned.
 * @return LY_ERR value on error.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_lypath_new(const struct ly_ctx *ctx, const char *value, size_t value_len,
        uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, const struct lysc_node *ctx_node,
        struct lys_glob_unres *unres, struct ly_path **path, struct ly_err_item **err);

/**
 * @brief Free ly_path structure used by instanceid value representation.
 *
 * The ly_path representation can be created by ::lyplg_type_lypath_new().
 *
 * @param[in] ctx libyang context.
 * @param[in] path The structure ([sized array](@ref sizedarrays)) to free.
 */
LIBYANG_API_DECL void lyplg_type_lypath_free(const struct ly_ctx *ctx, struct ly_path *path);

/**
 * @brief Print xpath1.0 value in the specific format.
 *
 * @param[in] xp_val xpath1.0 value structure.
 * @param[in] format Format to print in.
 * @param[in] prefix_data Format-specific prefix data.
 * @param[out] str_value Printed value.
 * @param[out] err Error structure on error.
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_print_xpath10_value(const struct lyd_value_xpath10 *xp_val, LY_VALUE_FORMAT format,
        void *prefix_data, char **str_value, struct ly_err_item **err);

/**
 * @defgroup plugintypestoreopts Plugins: Type store callback options.
 *
 * Options applicable to ::lyplg_type_store_clb().
 *
 * @{
 */
#define LYPLG_TYPE_STORE_DYNAMIC   0x01 /**< Value was dynamically allocated in its exact size and is supposed to be freed or
                                             directly inserted into the context's dictionary (e.g. in case of canonization).
                                             In any case, the caller of the callback does not free the provided
                                             value after calling the type's store callback with this option. */
#define LYPLG_TYPE_STORE_IMPLEMENT 0x02 /**< If a foreign module is needed to be implemented to successfully instantiate
                                             the value, make the module implemented. */
#define LYPLG_TYPE_STORE_IS_UTF8   0x04 /**< The value is guaranteed to be a valid UTF-8 string, if applicable for the type. */
/** @} plugintypestoreopts */

/**
 * @brief Callback to store the given @p value according to the given @p type.
 *
 * Value must always be correctly stored meaning all the other type callbacks (such as print or compare)
 * must function as expected. However, ::lyd_value._canonical can be left NULL and will be generated
 * and stored on-demand. But if @p format is ::LY_VALUE_CANON (or another, which must be equal to the canonical
 * value), the canonical value should be stored so that it does not have to be generated later.
 *
 * Note that the @p value is not necessarily used whole (may not be zero-terminated if a string). The provided
 * @p value_len is always correct. All store functions have to free a dynamically allocated @p value in all
 * cases (even on error).
 *
 * @param[in] ctx libyang context
 * @param[in] type Type of the value being stored.
 * @param[in] value Value to be stored.
 * @param[in] value_len Length (number of bytes) of the given @p value.
 * @param[in] options [Type plugin store options](@ref plugintypestoreopts).
 * @param[in] format Input format of the value, see the description for details.
 * @param[in] prefix_data Format-specific data for resolving any prefixes (see ly_resolve_prefix()).
 * @param[in] hints Bitmap of [value hints](@ref lydvalhints) of all the allowed value types.
 * @param[in] ctx_node Schema context node of @p value, may be NULL for metadata.
 * @param[out] storage Storage for the value in the type's specific encoding. Except for _canonical_, all the members
 * should be filled by the plugin (if it fills them at all).
 * @param[in,out] unres Global unres structure for newly implemented modules.
 * @param[out] err Optionally provided error information in case of failure. If not provided to the caller, a generic
 * error message is prepared instead. The error structure can be created by ::ly_err_new().
 * @return LY_SUCCESS on success,
 * @return LY_EINCOMPLETE in case the ::lyplg_type_validate_clb should be called to finish value validation in data,
 * @return LY_ERR value on error, @p storage must not have any pointers to dynamic memory.
 */
LIBYANG_API_DECL typedef LY_ERR (*lyplg_type_store_clb)(const struct ly_ctx *ctx, const struct lysc_type *type,
        const void *value, size_t value_len, uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints,
        const struct lysc_node *ctx_node, struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/**
 * @brief Callback to validate the stored value in data.
 *
 * This callback is optional for types that can only be validated in a data tree. It must be called and succeed
 * in case the ::lyplg_type_store_clb callback returned ::LY_EINCOMPLETE for the value to be valid. However, this
 * callback can be called even in other cases (such as separate/repeated validation).
 *
 * @param[in] ctx libyang context
 * @param[in] type Original type of the value (not necessarily the stored one) being validated.
 * @param[in] ctx_node The value data context node for validation.
 * @param[in] tree External data tree (e.g. when validating RPC/Notification) with possibly referenced data.
 * @param[in,out] storage Storage of the value successfully filled by ::lyplg_type_store_clb. May be modified.
 * @param[out] err Optionally provided error information in case of failure. If not provided to the caller, a generic
 * error message is prepared instead. The error structure can be created by ::ly_err_new().
 * @return LY_SUCCESS on success,
 * @return LY_ERR value on error.
 */
LIBYANG_API_DECL typedef LY_ERR (*lyplg_type_validate_clb)(const struct ly_ctx *ctx, const struct lysc_type *type,
        const struct lyd_node *ctx_node, const struct lyd_node *tree, struct lyd_value *storage, struct ly_err_item **err);

/**
 * @brief Callback for comparing 2 values of the same type.
 *
 * It can be assumed that the same context (dictionary) was used for storing both values and the realtype
 * member of both the values is the same.
 *
 * @param[in] val1 First value to compare.
 * @param[in] val2 Second value to compare.
 * @return LY_SUCCESS if values are considered equal.
 * @return LY_ENOT if values differ.
 */
typedef LY_ERR (*lyplg_type_compare_clb)(const struct lyd_value *val1, const struct lyd_value *val2);

/**
 * @brief Unused callback for sorting values.
 *
 * @param[in] val1 First value to compare.
 * @param[in] val2 Second value to compare.
 * @return -1 if val1 < val2,
 * @return 0 if val1 == val2,
 * @return 1 if val1 > val2.
 */
typedef int (*lyplg_type_sort_clb)(const struct lyd_value *val1, const struct lyd_value *val2);

/**
 * @brief Callback for getting the value of the data stored in @p value.
 *
 * Canonical value (@p format of ::LY_VALUE_CANON) must always be a zero-terminated const string stored in
 * the dictionary. The ::lyd_value._canonical member should be used for storing (caching) it.
 *
 * @param[in] ctx libyang context for storing the canonical value. May not be set for ::LY_VALUE_LYB format.
 * @param[in] value Value to print.
 * @param[in] format Format in which the data are supposed to be printed. Formats ::LY_VALUE_SCHEMA and
 * ::LY_VALUE_SCHEMA_RESOLVED are not supported and should not be implemented.
 * @param[in] prefix_data Format-specific data for processing prefixes. In case of using one of the built-in's print
 * callback (or ::lyplg_type_print_simple()), the argument is just simply passed in. If you need to handle prefixes
 * in the value on your own, there is ::lyplg_type_get_prefix() function to help.
 * @param[out] dynamic Flag if the returned value is dynamically allocated. In such a case the caller is responsible
 * for freeing it. Will not be set and should be ignored for @p format ::LY_VALUE_CANON.
 * @param[out] value_len Optional returned value length in bytes. For strings it EXCLUDES the terminating zero.
 * @return Pointer to @p value in the specified @p format. According to the returned @p dynamic flag, caller
 * can be responsible for freeing allocated memory.
 * @return NULL in case of error.
 */
typedef const void *(*lyplg_type_print_clb)(const struct ly_ctx *ctx, const struct lyd_value *value,
        LY_VALUE_FORMAT format, void *prefix_data, ly_bool *dynamic, size_t *value_len);

/**
 * @brief Callback to duplicate data in the data structure.
 *
 * @param[in] ctx libyang context of the @p dup. Note that the context of @p original and @p dup might not be the same.
 * @param[in] original Original data structure to be duplicated.
 * @param[in,out] dup Prepared data structure to be filled with the duplicated data of @p original.
 * @return LY_SUCCESS after successful duplication.
 * @return LY_ERR value on error.
 */
typedef LY_ERR (*lyplg_type_dup_clb)(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup);

/**
 * @brief Callback for freeing the user type values stored by ::lyplg_type_store_clb.
 *
 * Note that this callback is responsible also for freeing the canonized member in the @p value.
 *
 * @param[in] ctx libyang ctx to enable correct manipulation with values that are in the dictionary.
 * @param[in,out] value Value structure to free the data stored there by the plugin's ::lyplg_type_store_clb callback
 */
typedef void (*lyplg_type_free_clb)(const struct ly_ctx *ctx, struct lyd_value *value);

/**
 * @brief Hold type-specific functions for various operations with the data values.
 *
 * libyang includes set of plugins for all the built-in types. They are, by default, inherited to the derived types.
 * However, if the user type plugin for the specific type is loaded, the plugin can provide it's own functions.
 * The built-in types plugin callbacks are public, so even the user type plugins can use them to do part of their own
 * functionality.
 */
struct lyplg_type {
    const char *id;                     /**< Plugin identification (mainly for distinguish incompatible versions when
                                             used by external tools) */
    lyplg_type_store_clb store;         /**< store and canonize the value in the type-specific way */
    lyplg_type_validate_clb validate;   /**< optional, validate the value in the type-specific way in data */
    lyplg_type_compare_clb compare;     /**< comparison callback to compare 2 values of the same type */
    lyplg_type_sort_clb sort;           /**< unused comparison callback for sorting values */
    lyplg_type_print_clb print;         /**< printer callback to get string representing the value */
    lyplg_type_dup_clb duplicate;       /**< data duplication callback */
    lyplg_type_free_clb free;           /**< optional function to free the type-spceific way stored value */
    int32_t lyb_data_len;               /**< Length of the data in [LYB format](@ref howtoDataLYB).
                                             For variable-length is set to -1. */
};

struct lyplg_type_record {
    /* plugin identification */
    const char *module;          /**< name of the module where the type is defined (top-level typedef) */
    const char *revision;        /**< optional module revision - if not specified, the plugin applies to any revision,
                                      which is not an optimal approach due to a possible future revisions of the module.
                                      Instead, there should be defined multiple items in the plugins list, each with the
                                      different revision, but all with the same pointer to the plugin functions. The
                                      only valid use case for the NULL revision is the case the module has no revision. */
    const char *name;            /**< name of the typedef */

    /* runtime data */
    struct lyplg_type plugin; /**< data to utilize plugin implementation */
};

/**
 * @defgroup pluginsTypesSimple Plugins: Simple Types Callbacks
 * @ingroup pluginsTypes
 * @{
 *
 * Simple functions implementing @ref howtoPluginsTypes callbacks handling types that allocate no dynamic
 * value and always generate their canonical value (::lyd_value._canonical).
 */

/**
 * @brief Implementation of ::lyplg_type_compare_clb for a generic simple type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_compare_simple(const struct lyd_value *val1, const struct lyd_value *val2);

/**
 * @brief Implementation of ::lyplg_type_print_clb for a generic simple type.
 */
LIBYANG_API_DECL const void *lyplg_type_print_simple(const struct ly_ctx *ctx, const struct lyd_value *value,
        LY_VALUE_FORMAT format, void *prefix_data, ly_bool *dynamic, size_t *value_len);

/**
 * @brief Implementation of ::lyplg_type_dup_clb for a generic simple type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_dup_simple(const struct ly_ctx *ctx, const struct lyd_value *original,
        struct lyd_value *dup);

/**
 * @brief Implementation of ::lyplg_type_free_clb for a generic simple type.
 */
LIBYANG_API_DECL void lyplg_type_free_simple(const struct ly_ctx *ctx, struct lyd_value *value);

/** @} pluginsTypesSimple */

/**
 * @defgroup pluginsTypesBinary Plugins: Binary built-in type callbacks
 * @ingroup pluginsTypes
 * @{
 *
 * Callbacks used to implement binary built-in type.
 */

/**
 * @brief Implementation of ::lyplg_type_store_clb for the built-in binary type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_store_binary(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value,
        size_t value_len, uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints,
        const struct lysc_node *ctx_node, struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/**
 * @brief Implementation of ::lyplg_type_compare_clb for the built-in binary type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_compare_binary(const struct lyd_value *val1, const struct lyd_value *val2);

/**
 * @brief Implementation of ::lyplg_type_print_clb for the built-in binary type.
 */
LIBYANG_API_DECL const void *lyplg_type_print_binary(const struct ly_ctx *ctx, const struct lyd_value *value,
        LY_VALUE_FORMAT format, void *prefix_data, ly_bool *dynamic, size_t *value_len);

/**
 * @brief Implementation of ::lyplg_type_dup_clb for the built-in binary type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_dup_binary(const struct ly_ctx *ctx, const struct lyd_value *original,
        struct lyd_value *dup);

/**
 * @brief Implementation of ::lyplg_type_free_clb for the built-in binary type.
 */
LIBYANG_API_DECL void lyplg_type_free_binary(const struct ly_ctx *ctx, struct lyd_value *value);

/** @} pluginsTypesBinary */

/**
 * @defgroup pluginsTypesBits Plugins: Bits built-in type callbacks
 * @ingroup pluginsTypes
 * @{
 *
 * Callbacks used (besides the [simple callbacks](@ref pluginsTypesSimple)) to implement bits built-in type.
 */

/**
 * @brief Implementation of the ::lyplg_type_store_clb for the built-in bits type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_store_bits(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value,
        size_t value_len, uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints,
        const struct lysc_node *ctx_node, struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/**
 * @brief Implementation of the ::lyplg_type_compare_clb for the built-in bits type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_compare_bits(const struct lyd_value *val1, const struct lyd_value *val2);

/**
 * @brief Implementation of the ::lyplg_type_print_clb for the built-in bits type.
 */
LIBYANG_API_DECL const void *lyplg_type_print_bits(const struct ly_ctx *ctx, const struct lyd_value *value,
        LY_VALUE_FORMAT format, void *prefix_data, ly_bool *dynamic, size_t *value_len);

/**
 * @brief Implementation of the ::lyplg_type_dup_clb for the built-in bits type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_dup_bits(const struct ly_ctx *ctx, const struct lyd_value *original,
        struct lyd_value *dup);

/**
 * @brief Implementation of the ::lyplg_type_free_clb for the built-in bits type.
 */
LIBYANG_API_DECL void lyplg_type_free_bits(const struct ly_ctx *ctx, struct lyd_value *value);

/** @} pluginsTypesBits */

/**
 * @defgroup pluginsTypesBoolean Plugins: Boolean built-in type callbacks
 * @ingroup pluginsTypes
 * @{
 *
 * Callbacks used (besides the [simple callbacks](@ref pluginsTypesSimple)) to implement boolean built-in type.
 */

/**
 * @brief Implementation of ::lyplg_type_store_clb for the built-in boolean type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_store_boolean(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value,
        size_t value_len, uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints,
        const struct lysc_node *ctx_node, struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/**
 * @brief Implementation of ::lyplg_type_compare_clb for the built-in boolean type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_compare_boolean(const struct lyd_value *val1, const struct lyd_value *val2);

/**
 * @brief Implementation of ::lyplg_type_print_clb for the built-in boolean type.
 */
LIBYANG_API_DECL const void *lyplg_type_print_boolean(const struct ly_ctx *ctx, const struct lyd_value *value,
        LY_VALUE_FORMAT format, void *prefix_data, ly_bool *dynamic, size_t *value_len);

/** @} pluginsTypesBoolean */

/**
 * @defgroup pluginsTypesDecimal64 Plugins: Decimal64 built-in type callbacks
 * @ingroup pluginsTypes
 * @{
 *
 * Callbacks used (besides the [simple callbacks](@ref pluginsTypesSimple)) to implement decimal64 built-in type.
 */

/**
 * @brief Implementation of ::lyplg_type_store_clb for the built-in decimal64 type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_store_decimal64(const struct ly_ctx *ctx, const struct lysc_type *type,
        const void *value, size_t value_len, uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints,
        const struct lysc_node *ctx_node, struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/**
 * @brief Implementation of ::lyplg_type_compare_clb for the built-in decimal64 type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_compare_decimal64(const struct lyd_value *val1, const struct lyd_value *val2);

/**
 * @brief Implementation of ::lyplg_type_print_clb for the built-in decimal64 type.
 */
LIBYANG_API_DECL const void *lyplg_type_print_decimal64(const struct ly_ctx *ctx, const struct lyd_value *value,
        LY_VALUE_FORMAT format, void *prefix_data, ly_bool *dynamic, size_t *value_len);

/** @} pluginsTypesDecimal64 */

/**
 * @defgroup pluginsTypesEmpty Plugins: Empty built-in type callbacks
 * @ingroup pluginsTypes
 * @{
 *
 * Callbacks used (besides the [simple callbacks](@ref pluginsTypesSimple)) to implement empty built-in type.
 */

/**
 * @brief Implementation of ::lyplg_type_store_clb for the built-in empty type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_store_empty(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value,
        size_t value_len, uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints,
        const struct lysc_node *ctx_node, struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/** @} pluginsTypesEmpty */

/**
 * @defgroup pluginsTypesEnumeration Plugins: Enumeration built-in type callbacks
 * @ingroup pluginsTypes
 * @{
 *
 * Callbacks used (besides the [simple callbacks](@ref pluginsTypesSimple)) to implement enumeration built-in type.
 */

/**
 * @brief Implementation of ::lyplg_type_store_clb for the built-in enumeration type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_store_enum(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value,
        size_t value_len, uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints,
        const struct lysc_node *ctx_node, struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/**
 * @brief Implementation of ::lyplg_type_print_clb for the built-in enumeration type.
 */
LIBYANG_API_DECL const void *lyplg_type_print_enum(const struct ly_ctx *ctx, const struct lyd_value *value,
        LY_VALUE_FORMAT format, void *prefix_data, ly_bool *dynamic, size_t *value_len);

/** @} pluginsTypesEnumeration */

/**
 * @defgroup pluginsTypesIdentityref Plugins: Identityref built-in type callbacks
 * @ingroup pluginsTypes
 * @{
 *
 * Callbacks used (besides the [simple callbacks](@ref pluginsTypesSimple)) to implement identityref built-in type.
 */

/**
 * @brief Implementation of ::lyplg_type_store_clb for the built-in identityref type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_store_identityref(const struct ly_ctx *ctx, const struct lysc_type *type,
        const void *value, size_t value_len, uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints,
        const struct lysc_node *ctx_node, struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/**
 * @brief Implementation of ::lyplg_type_compare_clb for the built-in identityref type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_compare_identityref(const struct lyd_value *val1, const struct lyd_value *val2);

/**
 * @brief Implementation of ::lyplg_type_print_clb for the built-in identityref type.
 */
LIBYANG_API_DECL const void *lyplg_type_print_identityref(const struct ly_ctx *ctx, const struct lyd_value *value,
        LY_VALUE_FORMAT format, void *prefix_data, ly_bool *dynamic, size_t *value_len);

/** @} pluginsTypesIdentityref */

/**
 * @defgroup pluginsTypesInstanceid Plugins: Instance-identifier built-in type callbacks
 * @ingroup pluginsTypes
 * @{
 *
 * Callbacks used (besides the [simple callbacks](@ref pluginsTypesSimple)) to implement instance-identifier built-in type.
 */

/**
 * @brief Implementation of ::lyplg_type_store_clb for the built-in instance-identifier type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_store_instanceid(const struct ly_ctx *ctx, const struct lysc_type *type,
        const void *value, size_t value_len, uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints,
        const struct lysc_node *ctx_node, struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/**
 * @brief Implementation of ::lyplg_type_validate_clb for the built-in instance-identifier type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_validate_instanceid(const struct ly_ctx *ctx, const struct lysc_type *type,
        const struct lyd_node *ctx_node, const struct lyd_node *tree, struct lyd_value *storage, struct ly_err_item **err);

/**
 * @brief Implementation of ::lyplg_type_compare_clb for the built-in instance-identifier type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_compare_instanceid(const struct lyd_value *val1, const struct lyd_value *val2);

/**
 * @brief Implementation of ::lyplg_type_print_clb for the built-in instance-identifier type.
 */
LIBYANG_API_DECL const void *lyplg_type_print_instanceid(const struct ly_ctx *ctx, const struct lyd_value *value,
        LY_VALUE_FORMAT format, void *prefix_data, ly_bool *dynamic, size_t *value_len);

/**
 * @brief Implementation of ::lyplg_type_dup_clb for the built-in instance-identifier type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_dup_instanceid(const struct ly_ctx *ctx, const struct lyd_value *original,
        struct lyd_value *dup);

/**
 * @brief Implementation of ::lyplg_type_free_clb for the built-in instance-identifier type.
 */
LIBYANG_API_DECL void lyplg_type_free_instanceid(const struct ly_ctx *ctx, struct lyd_value *value);

/** @} pluginsTypesInstanceid */

/**
 * @defgroup pluginsTypesInteger Plugins: Integer built-in types callbacks
 * @ingroup pluginsTypes
 * @{
 *
 * Callbacks used (besides the [simple callbacks](@ref pluginsTypesSimple)) to implement integer built-in types.
 */

/**
 * @brief Implementation of ::lyplg_type_store_clb for the built-in signed integer types.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_store_int(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value,
        size_t value_len, uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints,
        const struct lysc_node *ctx_node, struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/**
 * @brief Implementation of ::lyplg_type_compare_clb for the built-in signed integer types.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_compare_int(const struct lyd_value *val1, const struct lyd_value *val2);

/**
 * @brief Implementation of ::lyplg_type_print_clb for the built-in signed integer types.
 */
LIBYANG_API_DECL const void *lyplg_type_print_int(const struct ly_ctx *ctx, const struct lyd_value *value,
        LY_VALUE_FORMAT format, void *prefix_data, ly_bool *dynamic, size_t *value_len);

/**
 * @brief Implementation of ::lyplg_type_store_clb for the built-in unsigned integer types.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_store_uint(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value,
        size_t value_len, uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints,
        const struct lysc_node *ctx_node, struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/**
 * @brief Implementation of ::lyplg_type_compare_clb for the built-in unsigned integer types.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_compare_uint(const struct lyd_value *val1, const struct lyd_value *val2);

/**
 * @brief Implementation of ::lyplg_type_print_clb for the built-in unsigned integer types.
 */
LIBYANG_API_DECL const void *lyplg_type_print_uint(const struct ly_ctx *ctx, const struct lyd_value *value,
        LY_VALUE_FORMAT format, void *prefix_data, ly_bool *dynamic, size_t *value_len);

/** @} pluginsTypesInteger */

/**
 * @defgroup pluginsTypesLeafref Plugins: Leafref built-in type callbacks
 * @ingroup pluginsTypes
 * @{
 *
 * Callbacks used (besides the [simple callbacks](@ref pluginsTypesSimple)) to implement leafref built-in type.
 */

/**
 * @brief Implementation of ::lyplg_type_store_clb for the built-in leafref type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_store_leafref(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value,
        size_t value_len, uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints,
        const struct lysc_node *ctx_node, struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/**
 * @brief Implementation of ::lyplg_type_compare_clb for the built-in leafref type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_compare_leafref(const struct lyd_value *val1, const struct lyd_value *val2);

/**
 * @brief Implementation of ::lyplg_type_print_clb for the built-in leafref type.
 */
LIBYANG_API_DECL const void *lyplg_type_print_leafref(const struct ly_ctx *ctx, const struct lyd_value *value,
        LY_VALUE_FORMAT format, void *prefix_data, ly_bool *dynamic, size_t *value_len);

/**
 * @brief Implementation of ::lyplg_type_dup_clb for the built-in leafref type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_dup_leafref(const struct ly_ctx *ctx, const struct lyd_value *original,
        struct lyd_value *dup);

/**
 * @brief Implementation of ::lyplg_type_validate_clb for the built-in leafref type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_validate_leafref(const struct ly_ctx *ctx, const struct lysc_type *type,
        const struct lyd_node *ctx_node, const struct lyd_node *tree, struct lyd_value *storage, struct ly_err_item **err);

/**
 * @brief Implementation of ::lyplg_type_free_clb for the built-in leafref type.
 */
LIBYANG_API_DECL void lyplg_type_free_leafref(const struct ly_ctx *ctx, struct lyd_value *value);

/** @} pluginsTypesLeafref */

/**
 * @defgroup pluginsTypesString Plugins: String built-in type callbacks
 * @ingroup pluginsTypes
 * @{
 *
 * Callbacks used (besides the [simple callbacks](@ref pluginsTypesSimple)) to implement string built-in type.
 */

/**
 * @brief Implementation of ::lyplg_type_store_clb for the built-in string type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_store_string(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value,
        size_t value_len, uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints,
        const struct lysc_node *ctx_node, struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/** @} pluginsTypesString */

/**
 * @defgroup pluginsTypesUnion Plugins: Union built-in type callbacks
 * @ingroup pluginsTypes
 * @{
 *
 * Callbacks used (besides the [simple callbacks](@ref pluginsTypesSimple)) to implement union built-in type.
 */

/**
 * @brief Implementation of ::lyplg_type_store_clb for the built-in union type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_store_union(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value,
        size_t value_len, uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints,
        const struct lysc_node *ctx_node, struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/**
 * @brief Implementation of ::lyplg_type_compare_clb for the built-in union type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_compare_union(const struct lyd_value *val1, const struct lyd_value *val2);

/**
 * @brief Implementation of ::lyplg_type_print_clb for the built-in union type.
 */
LIBYANG_API_DECL const void *lyplg_type_print_union(const struct ly_ctx *ctx, const struct lyd_value *value,
        LY_VALUE_FORMAT format, void *prefix_data, ly_bool *dynamic, size_t *value_len);

/**
 * @brief Implementation of ::lyplg_type_dup_clb for the built-in union type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_dup_union(const struct ly_ctx *ctx, const struct lyd_value *original,
        struct lyd_value *dup);

/**
 * @brief Implementation of ::lyplg_type_validate_clb for the built-in union type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_validate_union(const struct ly_ctx *ctx, const struct lysc_type *type,
        const struct lyd_node *ctx_node, const struct lyd_node *tree, struct lyd_value *storage, struct ly_err_item **err);

/**
 * @brief Implementation of ::lyplg_type_free_clb for the built-in union type.
 */
LIBYANG_API_DECL void lyplg_type_free_union(const struct ly_ctx *ctx, struct lyd_value *value);

/** @} pluginsTypesUnion */

/**
 * @defgroup pluginsTypesXpath10 Plugins: xpath1.0 `ietf-yang-types` type callbacks
 * @ingroup pluginsTypes
 * @{
 *
 * Callbacks used (besides the [simple callbacks](@ref pluginsTypesSimple)) to implement xpath1.0 derived type.
 */

/**
 * @brief Implementation of ::lyplg_type_store_clb for the ietf-yang-types xpath1.0 type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_store_xpath10(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value,
        size_t value_len, uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints,
        const struct lysc_node *ctx_node, struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err);

/**
 * @brief Implementation of ::lyplg_type_compare_clb for the ietf-yang-types xpath1.0 type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_compare_xpath10(const struct lyd_value *val1, const struct lyd_value *val2);

/**
 * @brief Implementation of ::lyplg_type_print_clb for the ietf-yang-types xpath1.0 type.
 */
LIBYANG_API_DECL const void *lyplg_type_print_xpath10(const struct ly_ctx *ctx, const struct lyd_value *value,
        LY_VALUE_FORMAT format, void *prefix_data, ly_bool *dynamic, size_t *value_len);

/**
 * @brief Implementation of ::lyplg_type_dup_clb for the ietf-yang-types xpath1.0 type.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_dup_xpath10(const struct ly_ctx *ctx, const struct lyd_value *original,
        struct lyd_value *dup);

/**
 * @brief Implementation of ::lyplg_type_free_clb for the ietf-yang-types xpath1.0 type.
 */
LIBYANG_API_DECL void lyplg_type_free_xpath10(const struct ly_ctx *ctx, struct lyd_value *value);

/** @} pluginsTypesXpath10 */

/**
 * @brief Unsigned integer value parser and validator.
 *
 * @param[in] datatype Type of the integer for logging.
 * @param[in] base Base of the integer's lexical representation. In case of built-in types, data must be represented in decimal format (base 10),
 * but default values in schemas can be represented also as hexadecimal or octal values (base 0).
 * @param[in] min Lower bound of the type.
 * @param[in] max Upper bound of the type.
 * @param[in] value Value string to parse.
 * @param[in] value_len Length of the @p value (mandatory parameter).
 * @param[out] ret Parsed integer value (optional).
 * @param[out] err Error information in case of failure. The error structure can be freed by ::ly_err_free().
 * @return LY_ERR value according to the result of the parsing and validation.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_parse_int(const char *datatype, int base, int64_t min, int64_t max, const char *value,
        size_t value_len, int64_t *ret, struct ly_err_item **err);

/**
 * @brief Unsigned integer value parser and validator.
 *
 * @param[in] datatype Type of the unsigned integer for logging.
 * @param[in] base Base of the integer's lexical representation. In case of built-in types, data must be represented in decimal format (base 10),
 * but default values in schemas can be represented also as hexadecimal or octal values (base 0).
 * @param[in] max Upper bound of the type.
 * @param[in] value Value string to parse.
 * @param[in] value_len Length of the @p value (mandatory parameter).
 * @param[out] ret Parsed unsigned integer value (optional).
 * @param[out] err Error information in case of failure. The error structure can be freed by ::ly_err_free().
 * @return LY_ERR value according to the result of the parsing and validation.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_parse_uint(const char *datatype, int base, uint64_t max, const char *value,
        size_t value_len, uint64_t *ret, struct ly_err_item **err);

/**
 * @brief Convert a string with a decimal64 value into libyang representation:
 * ret = value * 10^fraction-digits
 *
 * @param[in] fraction_digits Fraction-digits of the decimal64 type.
 * @param[in] value Value string to parse.
 * @param[in] value_len Length of the @p value (mandatory parameter).
 * @param[out] ret Parsed decimal64 value representing original value * 10^fraction-digits (optional).
 * @param[out] err Error information in case of failure. The error structure can be freed by ::ly_err_free().
 * @return LY_ERR value according to the result of the parsing and validation.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_parse_dec64(uint8_t fraction_digits, const char *value, size_t value_len, int64_t *ret,
        struct ly_err_item **err);

/**
 * @brief Decide if the @p derived identity is derived from (based on) the @p base identity.
 *
 * @param[in] base Expected base identity.
 * @param[in] derived Expected derived identity.
 * @return LY_SUCCESS if @p derived IS based on the @p base identity.
 * @return LY_ENOTFOUND if @p derived IS NOT not based on the @p base identity.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_identity_isderived(const struct lysc_ident *base, const struct lysc_ident *derived);

/**
 * @brief Data type validator for a range/length-restricted values.
 *
 * @param[in] basetype Base built-in type of the type with the range specified to get know if the @p range structure represents range or length restriction.
 * @param[in] range Range (length) restriction information.
 * @param[in] value Value to check. In case of basetypes using unsigned integer values, the value is actually cast to uint64_t.
 * @param[in] strval String representation of the @p value for error logging.
 * @param[in] strval_len Length of @p strval.
 * @param[out] err Error information in case of failure. The error structure can be freed by ::ly_err_free().
 * @return LY_ERR value according to the result of the validation.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_validate_range(LY_DATA_TYPE basetype, struct lysc_range *range, int64_t value,
        const char *strval, size_t strval_len, struct ly_err_item **err);

/**
 * @brief Data type validator for pattern-restricted string values.
 *
 * @param[in] patterns ([Sized array](@ref sizedarrays)) of the compiled list of pointers to the pattern restrictions.
 * The array can be found in the ::lysc_type_str.patterns structure.
 * @param[in] str String to validate.
 * @param[in] str_len Length (number of bytes) of the string to validate (mandatory).
 * @param[out] err Error information in case of failure or non-matching @p str. The error structure can be freed by ::ly_err_free().
 * @return LY_SUCCESS when @p matches all the patterns.
 * @return LY_EVALID when @p does not match any of the patterns.
 * @return LY_ESYS in case of PCRE2 error.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_validate_patterns(struct lysc_pattern **patterns, const char *str, size_t str_len,
        struct ly_err_item **err);

/**
 * @brief Find leafref target in data.
 *
 * @param[in] lref Leafref type.
 * @param[in] node Context node.
 * @param[in] value Target value.
 * @param[in] tree Full data tree to search in.
 * @param[out] target Optional found target.
 * @param[out] errmsg Error message in case of error.
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR lyplg_type_resolve_leafref(const struct lysc_type_leafref *lref, const struct lyd_node *node,
        struct lyd_value *value, const struct lyd_node *tree, struct lyd_node **target, char **errmsg);

/** @} pluginsTypes */

#ifdef __cplusplus
}
#endif

#endif /* LY_PLUGINS_TYPES_H_ */
