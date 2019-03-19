/**
 * @file user_types.h
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang support for user YANG type implementations.
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_USER_TYPES_H_
#define LY_USER_TYPES_H_

#include "libyang.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup user_types User Types
 * @ingroup schematree
 * @{
 */

/**
 * @brief Callback for storing user type values.
 *
 * This callback should overwrite the value stored in \p value using some custom encoding. Be careful,
 * if the type is #LY_TYPE_BITS, the bits must be freed before overwritting the union value.
 *
 * @param[in] type_name Name of the type being stored.
 * @param[in] value_str String value to be stored.
 * @param[in,out] value Value union for the value to be stored in (already is but in the standard way).
 * @param[out] err_msg Can be filled on error. If not, a generic error message will be printed.
 * @return 0 on success, non-zero if an error occured and the value could not be stored for any reason.
 */
typedef int (*lytype_store_clb)(const char *type_name, const char *value_str, lyd_val *value, char **err_msg);

struct lytype_plugin_list {
    const char *module;          /**< Name of the module where the type is defined. */
    const char *revision;        /**< Optional module revision - if not specified, the plugin applies to any revision,
                                      which is not the best approach due to a possible future revisions of the module.
                                      Instead, there should be defined multiple items in the plugins list, each with the
                                      different revision, but all with the same store callback. The only valid use case
                                      for the NULL revision is the case when the module has no revision. */
    const char *name;            /**< Name of the type to be stored in a custom way. */
    lytype_store_clb store_clb;  /**< Callback used for storing values of this type. */
    void (*free_clb)(void *ptr); /**< Callback used for freeing values of this type. */
};

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LY_USER_TYPES_H_ */
