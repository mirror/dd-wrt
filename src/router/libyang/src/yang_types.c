/**
 * @file yang_types.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Static YANG built-in-types
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdlib.h>

#include "tree_internal.h"

struct lys_tpdf ly_type_binary = {
    .name = "binary",
    .module = NULL,
    .dsc = "Any binary data",
    .ref = "RFC 6020, section 4.2.4",
    .flags = 0,
    .type = {.base = LY_TYPE_BINARY}
};

struct lys_tpdf ly_type_bits = {
    .name = "bits",
    .module = NULL,
    .dsc = "A set of bits or flags",
    .ref = "RFC 6020, section 4.2.4",
    .flags = 0,
    .type = {.base = LY_TYPE_BITS}
};

struct lys_tpdf ly_type_bool = {
    .name = "boolean",
    .module = NULL,
    .dsc = "true or false",
    .ref = "RFC 6020, section 4.2.4",
    .flags = 0,
    .type = {.base = LY_TYPE_BOOL}
};

struct lys_tpdf ly_type_dec64 = {
    .name = "decimal64",
    .module = NULL,
    .dsc = "64-bit signed decimal number",
    .ref = "RFC 6020, section 4.2.4",
    .flags = 0,
    .type = {.base = LY_TYPE_DEC64}
};

struct lys_tpdf ly_type_empty = {
    .name = "empty",
    .module = NULL,
    .dsc = "A leaf that does not have any value",
    .ref = "RFC 6020, section 4.2.4",
    .flags = 0,
    .type = {.base = LY_TYPE_EMPTY}
};

struct lys_tpdf ly_type_enum = {
    .name = "enumeration",
    .module = NULL,
    .dsc = "Enumerated strings",
    .ref = "RFC 6020, section 4.2.4",
    .flags = 0,
    .type = {.base = LY_TYPE_ENUM}
};

struct lys_tpdf ly_type_ident = {
    .name = "identityref",
    .module = NULL,
    .dsc = "A reference to an abstract identity",
    .ref = "RFC 6020, section 4.2.4",
    .flags = 0,
    .type = {.base = LY_TYPE_IDENT}
};

struct lys_tpdf ly_type_inst = {
    .name = "instance-identifier",
    .module = NULL,
    .dsc = "References a data tree node",
    .ref = "RFC 6020, section 4.2.4",
    .flags = 0,
    .type = {.base = LY_TYPE_INST}
};

struct lys_tpdf ly_type_int8 = {
    .name = "int8",
    .module = NULL,
    .dsc = "8-bit signed integer",
    .ref = "RFC 6020, section 4.2.4",
    .flags = 0,
    .type = {.base = LY_TYPE_INT8}
};

struct lys_tpdf ly_type_int16 = {
    .name = "int16",
    .module = NULL,
    .dsc = "16-bit signed integer",
    .ref = "RFC 6020, section 4.2.4",
    .flags = 0,
    .type = {.base = LY_TYPE_INT16}
};

struct lys_tpdf ly_type_int32 = {
    .name = "int32",
    .module = NULL,
    .dsc = "32-bit signed integer",
    .ref = "RFC 6020, section 4.2.4",
    .flags = 0,
    .type = {.base = LY_TYPE_INT32}
};

struct lys_tpdf ly_type_int64 = {
    .name = "int64",
    .module = NULL,
    .dsc = "64-bit signed integer",
    .ref = "RFC 6020, section 4.2.4",
    .flags = 0,
    .type = {.base = LY_TYPE_INT64}
};

struct lys_tpdf ly_type_leafref = {
    .name = "leafref",
    .module = NULL,
    .dsc = "A reference to a leaf instance",
    .ref = "RFC 6020, section 4.2.4",
    .flags = 0,
    .type = {.base = LY_TYPE_LEAFREF}
};

struct lys_tpdf ly_type_string = {
    .name = "string",
    .module = NULL,
    .dsc = "Human-readable string",
    .ref = "RFC 6020, section 4.2.4",
    .flags = 0,
    .type = {.base = LY_TYPE_STRING}
};

struct lys_tpdf ly_type_uint8 = {
    .name = "uint8",
    .module = NULL,
    .dsc = "8-bit unsigned integer",
    .ref = "RFC 6020, section 4.2.4",
    .flags = 0,
    .type = {.base = LY_TYPE_UINT8}
};

struct lys_tpdf ly_type_uint16 = {
    .name = "uint16",
    .module = NULL,
    .dsc = "16-bit unsigned integer",
    .ref = "RFC 6020, section 4.2.4",
    .flags = 0,
    .type = {.base = LY_TYPE_UINT16}
};

struct lys_tpdf ly_type_uint32 = {
    .name = "uint32",
    .module = NULL,
    .dsc = "32-bit unsigned integer",
    .ref = "RFC 6020, section 4.2.4",
    .flags = 0,
    .type = {.base = LY_TYPE_UINT32}
};

struct lys_tpdf ly_type_uint64 = {
    .name = "uint64",
    .module = NULL,
    .dsc = "64-bit unsigned integer",
    .ref = "RFC 6020, section 4.2.4",
    .flags = 0,
    .type = {.base = LY_TYPE_UINT64}
};

struct lys_tpdf ly_type_union = {
    .name = "union",
    .module = NULL,
    .dsc = "Choice of member types",
    .ref = "RFC 6020, section 4.2.4",
    .flags = 0,
    .type = {.base = LY_TYPE_UNION}
};

struct lys_tpdf *ly_types[LY_DATA_TYPE_COUNT] = {
    [LY_TYPE_DER] = NULL,
    [LY_TYPE_BINARY] = &ly_type_binary,
    [LY_TYPE_BITS] = &ly_type_bits,
    [LY_TYPE_BOOL] = &ly_type_bool,
    [LY_TYPE_DEC64] = &ly_type_dec64,
    [LY_TYPE_EMPTY] = &ly_type_empty,
    [LY_TYPE_ENUM] = &ly_type_enum,
    [LY_TYPE_IDENT] = &ly_type_ident,
    [LY_TYPE_INST] = &ly_type_inst,
    [LY_TYPE_INT8] = &ly_type_int8,
    [LY_TYPE_INT16] = &ly_type_int16,
    [LY_TYPE_INT32] = &ly_type_int32,
    [LY_TYPE_INT64] = &ly_type_int64,
    [LY_TYPE_LEAFREF] = &ly_type_leafref,
    [LY_TYPE_STRING] = &ly_type_string,
    [LY_TYPE_UINT8] = &ly_type_uint8,
    [LY_TYPE_UINT16] = &ly_type_uint16,
    [LY_TYPE_UINT32] = &ly_type_uint32,
    [LY_TYPE_UINT64] = &ly_type_uint64,
    [LY_TYPE_UNION] = &ly_type_union
};

