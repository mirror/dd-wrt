/**
 * @file metadata.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief ietf-yang-metadata API
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PLUGINS_EXTS_METADATA_H_
#define LY_PLUGINS_EXTS_METADATA_H_

#ifdef __cplusplus
extern "C" {
#endif

#define ANNOTATION_SUBSTMT_IFF     0 /**< index for the LY_STMT_IF_FEATURE substatement in annotation's ::lysc_ext_instance.substmts */
#define ANNOTATION_SUBSTMT_UNITS   1 /**< index for the LY_STMT_UNITS substatement in annotation's ::lysc_ext_instance.substmts */
#define ANNOTATION_SUBSTMT_STATUS  2 /**< index for the LY_STMT_STATUS substatement in annotation's ::lysc_ext_instance.substmts */
#define ANNOTATION_SUBSTMT_TYPE    3 /**< index for the LY_STMT_TYPE substatement in annotation's ::lysc_ext_instance.substmts */
#define ANNOTATION_SUBSTMT_DSC     4 /**< index for the LY_STMT_DSC substatement in annotation's ::lysc_ext_instance.substmts */
#define ANNOTATION_SUBSTMT_REF     5 /**< index for the LY_STMT_REF substatement in annotation's ::lysc_ext_instance.substmts */

#define LYEXT_PLUGIN_INTERNAL_ANNOTATION "ietf-yang-metadata", "2016-08-05", "annotation"

#ifdef __cplusplus
}
#endif

#endif /* LY_PLUGINS_EXTS_METADATA_H_ */
