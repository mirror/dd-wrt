/**
 * @file dict.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang dictionary
 *
 * Copyright (c) 2015-2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_DICT_H_
#define LY_DICT_H_

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "log.h"

#ifdef __cplusplus
extern "C" {
#endif

/* dummy context structure */
struct ly_ctx;

/**
 * @page howtoContextDict Context Dictionary
 *
 * Context includes dictionary to store strings more effectively. The most of strings repeats quite often in schema
 * as well as data trees. Therefore, instead of allocating those strings each time they appear, libyang stores them
 * as records in the dictionary. The basic API to the context dictionary is public, so even a caller application can
 * use the dictionary.
 *
 * To insert a string into the dictionary, caller can use ::lydict_insert() (adding a constant string) or
 * ::lydict_insert_zc() (for dynamically allocated strings that won't be used by the caller after its insertion into
 * the dictionary). Both functions provide the pointer to the inserted string in the dictionary record.
 *
 * To remove (reference of the) string from the context dictionary, ::lydict_remove() is supposed to be used.
 *
 * \note Incorrect usage of the dictionary can break libyang functionality.
 *
 * \note API for this group of functions is described in the [Dictionary module](@ref dict).
 *
 * Functions List
 * --------------
 * - ::lydict_insert()
 * - ::lydict_insert_zc()
 * - ::lydict_remove()
 */

/**
 * @defgroup dict Dictionary
 * @{
 *
 * Publicly visible functions and values of the libyang dictionary. They provide
 * access to the strings stored in the libyang context. More detailed information can be found at
 * @ref howtoContextDict page.
 */

/**
 * @brief Insert string into dictionary. If the string is already present,
 * only a reference counter is incremented and no memory allocation is
 * performed.
 *
 * @param[in] ctx libyang context handler
 * @param[in] value String to be stored in the dictionary. If NULL, function does nothing.
 * @param[in] len Number of bytes to store. The value is not required to be
 * NULL terminated string, the len parameter says number of bytes stored in
 * dictionary. The specified number of bytes is duplicated and terminating NULL
 * byte is added automatically. If \p len is 0, it is count automatically using strlen().
 * @param[out] str_p Optional parameter to get pointer to the string corresponding to the @p value and stored in dictionary.
 * @return LY_SUCCESS in case of successful insertion into dictionary, note that the function does not return LY_EEXIST.
 * @return LY_EINVAL in case of invalid input parameters.
 * @return LY_EMEM in case of memory allocation failure.
 */
LY_ERR lydict_insert(const struct ly_ctx *ctx, const char *value, size_t len, const char **str_p);

/**
 * @brief Insert string into dictionary - zerocopy version. If the string is
 * already present, only a reference counter is incremented and no memory
 * allocation is performed. This insert function variant avoids duplication of
 * specified value - it is inserted into the dictionary directly.
 *
 * @param[in] ctx libyang context handler
 * @param[in] value NULL-terminated string to be stored in the dictionary. If
 * the string is not present in dictionary, the pointer is directly used by the
 * dictionary. Otherwise, the reference counter is incremented and the value is
 * freed. So, after calling the function, caller is supposed to not use the
 * value address anymore. If NULL, function does nothing.
 * @param[out] str_p Optional parameter to get pointer to the string corresponding to the @p value and stored in dictionary.
 * @return LY_SUCCESS in case of successful insertion into dictionary, note that the function does not return LY_EEXIST.
 * @return LY_EINVAL in case of invalid input parameters.
 * @return LY_EMEM in case of memory allocation failure.
 */
LY_ERR lydict_insert_zc(const struct ly_ctx *ctx, char *value, const char **str_p);

/**
 * @brief Remove specified string from the dictionary. It decrement reference
 * counter for the string and if it is zero, the string itself is freed.
 *
 * @param[in] ctx libyang context handler
 * @param[in] value String to be freed. Note, that not only the string itself
 * must match the stored value, but also the address is being compared and the
 * counter is decremented only if it matches. If NULL, function does nothing.
 * @return LY_SUCCESS if the value was found and removed (or refcount decreased).
 * @return LY_ENOTFOUND if the value was not found.
 * @return LY_ERR on other errors.
 */
LY_ERR lydict_remove(const struct ly_ctx *ctx, const char *value);

/** @} dict */

#ifdef __cplusplus
}
#endif

#endif /* LY_DICT_H_ */
