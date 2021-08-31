/**
 * @file schema_features.h
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Header for schema features.
 *
 * Copyright (c) 2015 - 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_SCHEMA_FEATURES_H_
#define LY_SCHEMA_FEATURES_H_

#include "log.h"

struct ly_ctx;
struct lysp_module;
struct lysp_qname;

/**
 * @brief Evaluate if-features array.
 *
 * @param[in] ctx libyang context.
 * @param[in] iffeatures Sized array of if-features to evaluate.
 * @param[out] enabled Whether if-features evaluated to true or false.
 * @return LY_SUCCESS on success.
 * @return LY_ERR on error.
 */
LY_ERR lys_eval_iffeatures(const struct ly_ctx *ctx, struct lysp_qname *iffeatures, ly_bool *enabled);

/**
 * @brief Enable features in a parsed module with their consolidation and checking that they really
 * can be enabled and have all their if-features true.
 *
 * @param[in] pmod Parsed module to modify.
 * @param[in] features Array of features ended with NULL to enable. NULL for all features disabled, '*' for all enabled.
 * @return LY_SUCCESS on success.
 * @return LY_ERR on error.
 */
LY_ERR lys_enable_features(struct lysp_module *pmod, const char **features);

/**
 * @brief Set the specified features of a parsed module, with all the checks.
 *
 * @param[in] pmod Parsed module to modify.
 * @param[in] features Array of features ended with NULL to be enabled if the module is being implemented.
 * The feature string '*' enables all and array of length 1 with only the terminating NULL explicitly disables all
 * the features. In case the parameter is NULL, the features are untouched - left disabled in newly loaded module or
 * with the current features settings in case the module is already present in the context.
 * @return LY_SUCCESS on success.
 * @return LY_EEXIST if the specified features were already set.
 * @return LY_ERR on error.
 */
LY_ERR lys_set_features(struct lysp_module *pmod, const char **features);

/**
 * @brief Compile if-features of features in the current module and all its submodules.
 *
 * @param[in] ctx Compile context.
 * @return LY_ERR value.
 */
LY_ERR lys_compile_feature_iffeatures(struct lysp_module *pmod);

/**
 * @brief Free all auxiliary if-feature structures in a parsed module used for compilation.
 *
 * @param[in] pmod Module to update.
 */
void lys_free_feature_iffeatures(struct lysp_module *pmod);

#endif /* LY_SCHEMA_FEATURES_H_ */
