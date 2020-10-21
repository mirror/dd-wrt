/**
 * @file metadata.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang extension test plugin - special plugin for internal cmocka tests of extensions implementation.
 * The extension definition can be found in libyang/tests/schema/yang/files/ext-def.yang
 *
 * Copyright (c) 2016-2017 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_ ## x
#endif

#include "../extensions.h"

/**
 * @brief Storage for ID used to check plugin API version compatibility.
 */
LYEXT_VERSION_CHECK

/**
 * @brief Callback to check that the annotation can be instantiated inside the provided node
 *
 * @param[in] parent The parent of the instantiated extension.
 * @param[in] parent_type The type of the structure provided as \p parent.
 * @param[in] substmt_type libyang does not store all the extension instances in the structures where they are
 *                         instantiated in the module. In some cases (see #LYEXT_SUBSTMT) they are stored in parent
 *                         structure and marked with flag to know in which substatement of the parent the extension
 *                         was originally instantiated.
 * @return 0 - ok
 *         1 - error
 */
int libyang_ext_test_position(const void * UNUSED(parent), LYEXT_PAR UNUSED(parent_type),
                              LYEXT_SUBSTMT UNUSED(substmt_type))
{
    /* allow extension instance anywhere */
    return 0;
}

struct lyext_substmt libyang_ext_test_substmt[] = {
    {LY_STMT_ARGUMENT,      0,                       LY_STMT_CARD_OPT}, /* const char* + uint8_t */
    {LY_STMT_BASE,          1 * sizeof(const char*) + 1 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_BELONGSTO,     2 * sizeof(const char*) + 1 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* const char*[2] */
    {LY_STMT_CONTACT,       4 * sizeof(const char*) + 1 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_DEFAULT,       5 * sizeof(const char*) + 1 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_DESCRIPTION,   6 * sizeof(const char*) + 1 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_ERRTAG,        7 * sizeof(const char*) + 1 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_ERRMSG,        8 * sizeof(const char*) + 1 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_KEY,           9 * sizeof(const char*) + 1 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_NAMESPACE,    10 * sizeof(const char*) + 1 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_ORGANIZATION, 11 * sizeof(const char*) + 1 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_PATH,         12 * sizeof(const char*) + 1 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_PREFIX,       13 * sizeof(const char*) + 1 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_PRESENCE,     14 * sizeof(const char*) + 1 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_REFERENCE,    15 * sizeof(const char*) + 1 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_REVISIONDATE, 16 * sizeof(const char*) + 1 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_UNITS,        17 * sizeof(const char*) + 1 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* const char* */
    {LY_STMT_MODIFIER,     18 * sizeof(const char*) + 1 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* uint8_t */
    {LY_STMT_REQINSTANCE,  18 * sizeof(const char*) + 2 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* uint8_t */
    {LY_STMT_CONFIG,       18 * sizeof(const char*) + 3 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared uint16_t */
    {LY_STMT_MANDATORY,    18 * sizeof(const char*) + 3 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared uint16_t */
    {LY_STMT_ORDEREDBY,    18 * sizeof(const char*) + 3 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared uint16_t */
    {LY_STMT_STATUS,       18 * sizeof(const char*) + 3 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared uint16_t */
    {LY_STMT_DIGITS,       18 * sizeof(const char*) + 3 * sizeof(uint8_t) + sizeof(uint16_t), LY_STMT_CARD_OPT}, /* uint8_t */
    {LY_STMT_MAX,          18 * sizeof(const char*) + 4 * sizeof(uint8_t) + sizeof(uint16_t), LY_STMT_CARD_OPT}, /* uint32_t* */
    {LY_STMT_MIN,          18 * sizeof(const char*) + 4 * sizeof(uint8_t) +
                           sizeof(uint16_t) + 1 * sizeof(uint32_t*), LY_STMT_CARD_OPT}, /* uint32_t* */
    {LY_STMT_POSITION,     18 * sizeof(const char*) + 4 * sizeof(uint8_t) +
                           sizeof(uint16_t) + 2 * sizeof(uint32_t*), LY_STMT_CARD_OPT}, /* uint32_t* */
    {LY_STMT_VALUE,        18 * sizeof(const char*) + 4 * sizeof(uint8_t) +
                           sizeof(uint16_t) + 2 * sizeof(uint32_t*) + 1 * sizeof(int32_t*), LY_STMT_CARD_OPT}, /* uint32_t* */
/* compress the offset calculation */
    {LY_STMT_UNIQUE,       22 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* struct lys_unique* */
    {LY_STMT_MODULE,       23 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* struct lys_module* */
    {LY_STMT_ACTION,       24 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared struct lys_node* */
    {LY_STMT_ANYDATA,      24 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared struct lys_node* */
    {LY_STMT_ANYXML,       24 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared struct lys_node* */
    {LY_STMT_CASE,         24 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared struct lys_node* */
    {LY_STMT_CHOICE,       24 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared struct lys_node* */
    {LY_STMT_CONTAINER,    24 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared struct lys_node* */
    {LY_STMT_GROUPING,     24 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared struct lys_node* */
    {LY_STMT_INPUT,        24 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared struct lys_node* */
    {LY_STMT_LEAF,         24 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared struct lys_node* */
    {LY_STMT_LEAFLIST,     24 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared struct lys_node* */
    {LY_STMT_LIST,         24 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared struct lys_node* */
    {LY_STMT_NOTIFICATION, 24 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared struct lys_node* */
    {LY_STMT_OUTPUT,       24 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared struct lys_node* */
    {LY_STMT_USES,         24 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* shared struct lys_node* */
    {LY_STMT_TYPEDEF,      25 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* struct lys_tpdf* */
    {LY_STMT_TYPE,         26 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* struct lys_type* */
    {LY_STMT_IFFEATURE,    27 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* struct lys_iffeature* */
    {LY_STMT_LENGTH,       28 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* struct lys_restr* */
    {LY_STMT_MUST,         29 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* struct lys_restr* */
    {LY_STMT_PATTERN,      30 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* struct lys_restr* */
    {LY_STMT_RANGE,        31 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* struct lys_restr* */
    {LY_STMT_WHEN,         32 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* struct lys_when* */
    {LY_STMT_REVISION,     33 * sizeof(void*) + 6 * sizeof(uint8_t), LY_STMT_CARD_OPT}, /* struct lys_revision* */
    {0, 0, 0} /* terminating item */
};

struct lyext_substmt libyang_ext_test_substmt_arrays[] = {
    {LY_STMT_ARGUMENT,      0,                       LY_STMT_CARD_ANY}, /* const char** + uint8_t* */
    {LY_STMT_BASE,          1 * sizeof(const char*) + 1 * sizeof(uint8_t*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_BELONGSTO,     2 * sizeof(const char*) + 1 * sizeof(uint8_t*), LY_STMT_CARD_ANY}, /* const char**[2] */
    {LY_STMT_CONTACT,       4 * sizeof(const char*) + 1 * sizeof(uint8_t*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_DEFAULT,       5 * sizeof(const char*) + 1 * sizeof(uint8_t*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_DESCRIPTION,   6 * sizeof(const char*) + 1 * sizeof(uint8_t*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_ERRTAG,        7 * sizeof(const char*) + 1 * sizeof(uint8_t*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_ERRMSG,        8 * sizeof(const char*) + 1 * sizeof(uint8_t*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_KEY,           9 * sizeof(const char*) + 1 * sizeof(uint8_t*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_NAMESPACE,    10 * sizeof(const char*) + 1 * sizeof(uint8_t*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_ORGANIZATION, 11 * sizeof(const char*) + 1 * sizeof(uint8_t*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_PATH,         12 * sizeof(const char*) + 1 * sizeof(uint8_t*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_PREFIX,       13 * sizeof(const char*) + 1 * sizeof(uint8_t*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_PRESENCE,     14 * sizeof(const char*) + 1 * sizeof(uint8_t*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_REFERENCE,    15 * sizeof(const char*) + 1 * sizeof(uint8_t*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_REVISIONDATE, 16 * sizeof(const char*) + 1 * sizeof(uint8_t*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_UNITS,        17 * sizeof(const char*) + 1 * sizeof(uint8_t*), LY_STMT_CARD_ANY}, /* const char** */
    {LY_STMT_DIGITS,       18 * sizeof(const char*) + 1 * sizeof(uint8_t*), LY_STMT_CARD_ANY}, /* uint8_t* */
    {LY_STMT_MAX,          18 * sizeof(const char*) + 2 * sizeof(uint8_t*), LY_STMT_CARD_ANY}, /* uint32_t* */
    {LY_STMT_MIN,          18 * sizeof(const char*) + 2 * sizeof(uint8_t*) +
                            1 * sizeof(uint32_t*), LY_STMT_CARD_ANY}, /* uint32_t* */
    {LY_STMT_POSITION,     18 * sizeof(const char*) + 2 * sizeof(uint8_t*) +
                            2 * sizeof(uint32_t*), LY_STMT_CARD_ANY}, /* uint32_t* */
    {LY_STMT_VALUE,        18 * sizeof(const char*) + 2 * sizeof(uint8_t*) +
                            2 * sizeof(uint32_t*) + 1 * sizeof(int32_t*), LY_STMT_CARD_ANY}, /* int32_t* */
/* compress the offset calculation */
    {LY_STMT_UNIQUE,       24 * sizeof(void*), LY_STMT_CARD_ANY}, /* struct lys_unique** */
    {LY_STMT_MODULE,       25 * sizeof(void*), LY_STMT_CARD_ANY}, /* struct lys_module** */
    {LY_STMT_ACTION,       26 * sizeof(void*), LY_STMT_CARD_ANY}, /* shared struct lys_node* */
    {LY_STMT_ANYDATA,      26 * sizeof(void*), LY_STMT_CARD_ANY}, /* shared struct lys_node* */
    {LY_STMT_ANYXML,       26 * sizeof(void*), LY_STMT_CARD_ANY}, /* shared struct lys_node* */
    {LY_STMT_CASE,         26 * sizeof(void*), LY_STMT_CARD_ANY}, /* shared struct lys_node* */
    {LY_STMT_CHOICE,       26 * sizeof(void*), LY_STMT_CARD_ANY}, /* shared struct lys_node* */
    {LY_STMT_CONTAINER,    26 * sizeof(void*), LY_STMT_CARD_ANY}, /* shared struct lys_node* */
    {LY_STMT_GROUPING,     26 * sizeof(void*), LY_STMT_CARD_ANY}, /* shared struct lys_node* */
    {LY_STMT_INPUT,        26 * sizeof(void*), LY_STMT_CARD_ANY}, /* shared struct lys_node* */
    {LY_STMT_LEAF,         26 * sizeof(void*), LY_STMT_CARD_ANY}, /* shared struct lys_node* */
    {LY_STMT_LEAFLIST,     26 * sizeof(void*), LY_STMT_CARD_ANY}, /* shared struct lys_node* */
    {LY_STMT_LIST,         26 * sizeof(void*), LY_STMT_CARD_ANY}, /* shared struct lys_node* */
    {LY_STMT_NOTIFICATION, 26 * sizeof(void*), LY_STMT_CARD_ANY}, /* shared struct lys_node* */
    {LY_STMT_OUTPUT,       26 * sizeof(void*), LY_STMT_CARD_ANY}, /* shared struct lys_node* */
    {LY_STMT_USES,         26 * sizeof(void*), LY_STMT_CARD_ANY}, /* shared struct lys_node* */
    {LY_STMT_TYPEDEF,      27 * sizeof(void*), LY_STMT_CARD_ANY}, /* struct lys_tpdf** */
    {LY_STMT_TYPE,         28 * sizeof(void*), LY_STMT_CARD_ANY}, /* struct lys_type** */
    {LY_STMT_IFFEATURE,    29 * sizeof(void*), LY_STMT_CARD_ANY}, /* struct lys_iffeature** */
    {LY_STMT_LENGTH,       30 * sizeof(void*), LY_STMT_CARD_ANY}, /* struct lys_restr** */
    {LY_STMT_MUST,         31 * sizeof(void*), LY_STMT_CARD_ANY}, /* struct lys_restr** */
    {LY_STMT_PATTERN,      32 * sizeof(void*), LY_STMT_CARD_ANY}, /* struct lys_restr** */
    {LY_STMT_RANGE,        33 * sizeof(void*), LY_STMT_CARD_ANY}, /* struct lys_restr** */
    {LY_STMT_WHEN,         34 * sizeof(void*), LY_STMT_CARD_ANY}, /* struct lys_when** */
    {LY_STMT_REVISION,     35 * sizeof(void*), LY_STMT_CARD_ANY}, /* struct lys_revision** */
    {0, 0, 0} /* terminating item */
};

struct lyext_substmt libyang_ext_test_substmt_mand[] = {
    {LY_STMT_DESCRIPTION,   0,                 LY_STMT_CARD_MAND}, /* const char* */
    {LY_STMT_DEFAULT,       1 * sizeof(void*), LY_STMT_CARD_SOME}, /* const char* */
    {LY_STMT_CONFIG,        2 * sizeof(void*), LY_STMT_CARD_MAND}, /* shared uint16_t */
    {LY_STMT_MANDATORY,     2 * sizeof(void*), LY_STMT_CARD_MAND}, /* shared uint16_t */
    {LY_STMT_STATUS,        2 * sizeof(void*), LY_STMT_CARD_MAND}, /* shared uint16_t */
    {LY_STMT_DIGITS,        2 * sizeof(void*) + sizeof(uint16_t), LY_STMT_CARD_MAND}, /* uint8_t */
    {LY_STMT_MIN,           2 * sizeof(void*) + sizeof(uint8_t) + sizeof(uint16_t), LY_STMT_CARD_SOME}, /* uint32_t* */
    {LY_STMT_LEAF,          3 * sizeof(void*) + sizeof(uint8_t) + sizeof(uint16_t), LY_STMT_CARD_SOME}, /* shared struct lys_node* */
    {LY_STMT_MUST,          4 * sizeof(void*) + sizeof(uint8_t) + sizeof(uint16_t), LY_STMT_CARD_SOME}, /* struct lys_restr* */
    {0, 0, 0} /* terminating item */
};

/**
 * @brief Plugin structure with cardinalities up to 1
 */
struct lyext_plugin_complex libyang_ext_test_p = {
    .type = LYEXT_COMPLEX,
    .flags = 0,
    .check_position = &libyang_ext_test_position,
    .check_result = NULL,
    .check_inherit = NULL,

    /* specification of allowed substatements of the extension instance */
    .substmt = libyang_ext_test_substmt,

    /* final size of the extension instance structure with the space for storing the substatements */
    .instance_size = (sizeof(struct lys_ext_instance_complex) - 1) + 34 * sizeof(void*) + 6 * sizeof(uint8_t)
};

/**
 * @brief Plugin structure with cardinalities higher than 1
 */
struct lyext_plugin_complex libyang_ext_test_arrays_p = {
    .type = LYEXT_COMPLEX,
    .flags = 0,
    .check_position = &libyang_ext_test_position,
    .check_result = NULL,
    .check_inherit = NULL,

    /* specification of allowed substatements of the extension instance */
    .substmt = libyang_ext_test_substmt_arrays,

    /* final size of the extension instance structure with the space for storing the substatements */
    .instance_size = (sizeof(struct lys_ext_instance_complex) - 1) + 36 * sizeof(void*)
};

/**
 * @brief Plugin structure with mandatory tests
 */
struct lyext_plugin_complex libyang_ext_test_mand_p = {
    .type = LYEXT_COMPLEX,
    .flags = 0,
    .check_position = &libyang_ext_test_position,
    .check_result = NULL,
    .check_inherit = NULL,

    /* specification of allowed substatements of the extension instance */
    .substmt = libyang_ext_test_substmt_mand,

    /* final size of the extension instance structure with the space for storing the substatements */
    .instance_size = (sizeof(struct lys_ext_instance_complex) - 1) + 5 * sizeof(void*) + sizeof(uint8_t) + sizeof(uint16_t)
};

/**
 * @brief list of all extension plugins implemented here
 *
 * MANDATORY object for all libyang extension plugins, the name must match the <name>.so
 */
struct lyext_plugin_list libyang_ext_test[] = {
    {"ext-def", "2017-01-18", "complex", (struct lyext_plugin*)&libyang_ext_test_p},
    {"ext-def", "2017-01-18", "complex-arrays", (struct lyext_plugin*)&libyang_ext_test_arrays_p},
    {"ext-def", "2017-01-18", "complex-mand", (struct lyext_plugin*)&libyang_ext_test_mand_p},
    {NULL, NULL, NULL, NULL} /* terminating item */
};
