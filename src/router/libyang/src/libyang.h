/**
 * @file libyang.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief The main libyang public header.
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_LIBYANG_H_
#define LY_LIBYANG_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "context.h"
#include "dict.h"
#include "in.h"
#include "log.h"
#include "out.h"
#include "parser_data.h"
#include "parser_schema.h"
#include "printer_data.h"
#include "printer_schema.h"
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_schema.h"

/*
 * The following headers are supposed to be included explicitly:
 * - plugins_types.h
 * - plugins_exts.h
 */

/**
 * @mainpage About
 *
 * libyang is a library implementing processing of the YANG schemas and data modeled by the YANG language. The
 * library is implemented in C for GNU/Linux and provides C API.
 *
 * @section about-features Main Features
 *
 * - [Parsing (and validating) schemas](@ref howtoSchema) in YANG format.
 * - [Parsing (and validating) schemas](@ref howtoSchema) in YIN format.
 * - [Parsing, validating and printing instance data](@ref howtoData) in XML format.
 * - [Parsing, validating and printing instance data](@ref howtoData) in JSON format
 *   ([RFC 7951](https://tools.ietf.org/html/rfc7951)).
 * - [Manipulation with the instance data](@ref howtoDataManipulation).
 * - Support for [default values in the instance data](@ref howtoDataWD) ([RFC 6243](https://tools.ietf.org/html/rfc6243)).
 * - Support for [YANG extensions and user types](@ref howtoPlugins).
 * - Support for [YANG Metadata](@ref howtoDataMetadata) ([RFC 7952](https://tools.ietf.org/html/rfc6243)).
 *
 * The current implementation covers YANG 1.0 ([RFC 6020](https://tools.ietf.org/html/rfc6020)) as well as
 * YANG 1.1 ([RFC 7950](https://tools.ietf.org/html/rfc7950)).
 *
 * @section about-license License
 *
 * Copyright (c) 2015-2021 CESNET, z.s.p.o.
 *
 * (The BSD 3-Clause License)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 */

/**
 * @page howto libyang API Overview
 *
 * @section howtoGeneral General notes
 *
 * libyang is primarily intended for handling data modeled by YANG modeling language, so the library is supposed to be optimized
 * for this purpose. However, as a side effect, the library has to be able precisely process YANG modules. Thus, it is usable by
 * YANG module authors to validate their modules and schemas in the development process.
 *
 * - @subpage howtoStructures
 * - @subpage howtoErrors
 * - @subpage howtoLogger
 * - @subpage howtoThreads
 * - @subpage howtoContext
 * - @subpage howtoInput
 * - @subpage howtoOutput
 * - @subpage howtoSchema
 * - @subpage howtoData
 * - @subpage howtoXPath
 * - @subpage howtoPlugins
 */

/**
 * @page howtoStructures Data Structures
 *
 * @section sizedarrays Sized Arrays
 *
 * The structure starts with 32bit number storing size of the array - the number of the items inside. The size is part of the
 * array to have it allocated together with the array itself only when it is needed. However, the pointers to the array always
 * points after the 32b number, so items can be accessed directly as for standard C arrays. Because of a known size (available
 * via ::LY_ARRAY_COUNT macro), it is not terminated by any special byte (sequence), so there is also no limitation for specific
 * content of the stored records (e.g. that first byte must not be NULL).
 *
 * The sized arrays must be carefully freed (which should be done anyway only internally), since pointers to the sized arrays used
 * in libyang structures, does not point to the beginning of the allocated space.
 *
 * - ::LY_ARRAY_COUNT
 * - ::LY_ARRAY_FOR
 *
 * @section struct_lists Lists
 *
 * The lists are structures connected via a `next` and `prev` pointers. Iterating over the siblings can be simply done by
 * ::LY_LIST_FOR macro. Examples of such structures are ::lyd_node or ::lysc_node.
 *
 * The `prev` pointer is always filled. In case there is just a single item in the list, the `prev` pointer points to the
 * item itself. Otherwise, the `prev` pointer of the first item points to the last item of the list. In contrast, the
 * `next` pointer of the last item in the list is always NULL.
 */

/**
 * @page howtoThreads Threading Limitations
 *
 * @section context Context
 *
 * It is safe to read from ::ly_ctx structure concurrently and use its dictionary, which is protected by a lock.
 * Thread-safe functions include any ones working with data trees (only context dictionary is accessed) and all
 * the `ly_ctx_get_*()` functions. Generally, they are the functions with `const` context parameter.
 *
 * @section data Data Trees
 *
 * Data trees are not internally synchronized so the general safe practice of a single writer **or** several concurrent
 * readers should be followed. Specifically, only the functions with non-const ::lyd_node parameters modify the node(s)
 * and no concurrent execution of such functions should be allowed on a single data tree or subtrees of one.
 */

/**
 * @internal
 * @page internals Developers' Notes
 * @tableofcontents
 *
 * Following texts describes various internal subsystems and mechanism in libyang which are hidden from external users, but important
 * for libyang developers. The texts should explain various decisions made and internal processes utilized in libyang.
 */

#ifdef __cplusplus
}
#endif

#endif /* LY_LIBYANG_H_ */
