/**
 * @file metadata.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief ietf-yang-metadata API
 *
 * Copyright (c) 2019 - 2022 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PLUGINS_EXTS_METADATA_H_
#define LY_PLUGINS_EXTS_METADATA_H_

#include "plugins_exts.h"
#include "tree_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Metadata structure.
 *
 * The structure provides information about metadata of a data element. Such attributes must map to
 * annotations as specified in RFC 7952. The only exception is the filter type (in NETCONF get operations)
 * and edit-config's operation attributes. In XML, they are represented as standard XML attributes. In JSON,
 * they are represented as JSON elements starting with the '@' character (for more information, see the
 * YANG metadata RFC.
 *
 */
struct lyd_meta {
    struct lyd_node *parent;         /**< data node where the metadata is placed */
    struct lyd_meta *next;           /**< pointer to the next metadata of the same element */
    struct lysc_ext_instance *annotation; /**< pointer to the annotation's definition */
    const char *name;                /**< metadata name */
    struct lyd_value value;          /**< metadata value representation */
};

/**
 * @brief Get the (canonical) value of a metadata node.
 *
 * @param[in] meta Metadata node to use.
 * @return Canonical value.
 */
static inline const char *
lyd_get_meta_value(const struct lyd_meta *meta)
{
    if (meta) {
        const struct lyd_value *value = &meta->value;

        return value->_canonical ? value->_canonical : lyd_value_get_canonical(meta->annotation->module->ctx, value);
    }

    return NULL;
}

#ifdef __cplusplus
}
#endif

#endif /* LY_PLUGINS_EXTS_METADATA_H_ */
