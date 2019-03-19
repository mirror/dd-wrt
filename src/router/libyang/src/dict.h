/**
 * @file dict.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang dictionary
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_DICT_H_
#define LY_DICT_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * structure definition from context.h
 */
struct ly_ctx;

/**
 * @defgroup dict Dictionary
 * @{
 *
 * Publicly visible functions and values of the libyang dictionary. They provide
 * access to the strings stored in the libyang context.
 */

/**
 * @brief Insert string into dictionary. If the string is already present,
 * only a reference counter is incremented and no memory allocation is
 * performed.
 *
 * @param[in] ctx libyang context handler
 * @param[in] value String to be stored in the dictionary.
 * @param[in] len Number of bytes to store. The value is not required to be
 * NULL terminated string, the len parameter says number of bytes stored in
 * dictionary. The specified number of bytes is duplicated and terminating NULL
 * byte is added automatically.
 * @return pointer to the string stored in the dictionary
 */
const char *lydict_insert(struct ly_ctx *ctx, const char *value, size_t len);

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
 * value address anymore.
 * @return pointer to the string stored in the dictionary
 */
const char *lydict_insert_zc(struct ly_ctx *ctx, char *value);

/**
 * @brief Remove specified string from the dictionary. It decrement reference
 * counter for the string and if it is zero, the string itself is freed.
 *
 * @param[in] ctx libyang context handler
 * @param[in] value String to be freed. Note, that not only the string itself
 * must match the stored value, but also the address is being compared and the
 * counter is decremented only if it matches.
 */
void lydict_remove(struct ly_ctx *ctx, const char *value);

/**@} dict */

#ifdef __cplusplus
}
#endif

#endif /* LY_DICT_H_ */
