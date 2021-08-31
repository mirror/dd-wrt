/**
 * @file invalid.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Invalid testing plugins implementation
 *
 * Copyright (c) 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdint.h>

#include "libyang.h"
#include "plugins_exts.h"
#include "plugins_types.h"

/*
 * EXTENSION PLUGIN
 */

/**
 * @brief Instead of plugin description, only the API version is declared.
 *
 * Here should be LY_PLUGINS_EXTENSIONS used.
 */
uint32_t plugins_extensions_apiver__ = LYPLG_EXT_API_VERSION;

/*
 * TYPE PLUGIN
 */

/**
 * @brief Instead of plugin description, only the API version is declared.
 *
 * Here should be LYPLG_TYPES used.
 */
uint32_t plugins_types_apiver__ = LYPLG_TYPE_API_VERSION;
