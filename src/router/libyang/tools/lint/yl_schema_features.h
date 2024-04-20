/**
 * @file yl_schema_features.h
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief Control features for the schema.
 *
 * Copyright (c) 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef YL_SCHEMA_FEATURES_H_
#define YL_SCHEMA_FEATURES_H_

#include <stdint.h>

struct ly_set;
struct lys_module;
struct ly_out;
struct ly_ctx;

/**
 * @brief Storage for the list of the features (their names) in a specific YANG module.
 */
struct yl_schema_features {
    char *mod_name;
    char **features;
    uint8_t applied;
};

/**
 * @brief Free the schema features list (struct schema_features *)
 * @param[in,out] flist The (struct schema_features *) to free.
 */
void yl_schema_features_free(void *flist);

/**
 * @brief Get the list of features connected with the specific YANG module.
 *
 * @param[in] fset The set of features information (struct schema_features *).
 * @param[in] module Name of the YANG module which features should be found.
 * @param[out] features Pointer to the list of features being returned.
 */
void get_features(const struct ly_set *fset, const char *module, const char ***features);

/**
 * @brief Parse features being specified for the specific YANG module.
 *
 * Format of the input @p fstring is as follows: "<module_name>:[<feature>,]*"
 *
 * @param[in] fstring Input string to be parsed.
 * @param[in, out] fset Features information set (of struct schema_features *). The set is being filled.
 */
int parse_features(const char *fstring, struct ly_set *fset);

/**
 * @brief Print all features of a single module.
 *
 * @param[in] out The output handler for printing.
 * @param[in] mod Module which can contains the features.
 */
void print_features(struct ly_out *out, const struct lys_module *mod);

/**
 * @brief Print all features in the 'feature-param' format.
 *
 * @param[in] out The output handler for printing.
 * @param[in] mod Module which can contains the features.
 */
void print_feature_param(struct ly_out *out, const struct lys_module *mod);

/**
 * @brief Print all features of all implemented modules.
 *
 * @param[in] out The output handler for printing.
 * @param[in] ctx Libyang context.
 * @param[in] feature_param Flag expressing whether to print features parameter.
 */
void print_all_features(struct ly_out *out, const struct ly_ctx *ctx, uint8_t feature_param);

#endif /* YL_SCHEMA_FEATURES_H_ */
