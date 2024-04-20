/**
 * @file   utests.h
 * @author Radek Iša <isa@cesnet.cz>
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief  this file contains macros for simplification test writing
 *
 * Copyright (c) 2021 - 2022 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef _UTESTS_H_
#define _UTESTS_H_

#define _POSIX_C_SOURCE 200809L /* strdup */

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <cmocka.h>

#include <string.h>

#include "libyang.h"
#include "plugins_exts/metadata.h"
#include "plugins_internal.h"
#include "plugins_types.h"
#include "tests_config.h"
#include "tree_schema_internal.h"

/**
 * TESTS OVERVIEW
 *
 * To include utest's environment, just include "utests.h" in the test's source
 * code. In case it is the main source code for a cmocka test group (there is a
 * main() function), define _UTEST_MAIN_ before including this header.
 *
 * TESTS VARIABLES
 *
 * Checking macros use internal storage to store various variables necessary
 * during the checking. It is possible to access these variables using the
 * following macros:
 *
 * UTEST_LYCTX    - libyang context
 * UTEST_IN       - input handler
 * UTEST_OUT      - output handler
 *
 * All these variables are cleaned with test's teardown.
 *
 * TESTS SETUP
 *
 * CMocka's CMUnitTest list definition macros (cmoka_unit_test*()) are replaced
 * by UTEST macro with possibility to specify own setup and teardown functions:
 *
 * UTEST(test_func) - only implicit setup and teardown functions are used
 * UTEST(test_func, setup) - implicit teardown but own setup
 * UTEST(test_func, setup, teardown) - both setup and teardown are test-specific
 *
 * Note that the tests environment always provide (and need) internal setup and
 * teardown functions. In case the test-specific setup or teardown are used, they
 * are supposed to include UTEST_SETUP at the setup beginning and UTEST_TEARDOWN
 * at the teardown end.
 *
 * Libyang context is part of the prepared environment. To add a schema into the
 * context (despite it is in the test-specific setup or in test function itself),
 * use UTEST_ADD_MODULE macro.
 *
 * LOGGING
 *
 * There are 2 macros to check content of the log from the previously called
 * libyang function. CHECK_LOG macro test only the last error message and path
 * stored directly via logging callback. CHECK_LOG_CTX gets error message and
 * path from the libyang context (in case the function does not store the error
 * information into the libyang context, the message cannot be checked this way).
 * libyang is set to store multiple error information, so multiple couples of
 * error message and path can be provided to be checked (the first couple
 * corresponds to the latest error). The macro also cleanups the errors list, so
 * it is fine to check that there is no error after succeeding successful
 * function call.
 */

/**
 * @brief Test's context to provide common storage for various variables.
 */
struct utest_context {
    struct ly_ctx *ctx;  /**< libyang context */

    char *err_msg;       /**< Directly logged error message */
    char *err_path;      /**< Directly logged error path */

    struct ly_in *in;    /**< Input handler */
    struct ly_out *out;  /**< Outpu handler */

    char *orig_tz;       /**< Original "TZ" environment variable value */
};

/**
 * @brief Shortcut to access utest_context.
 */
#define _UC ((struct utest_context *)*state)

/**
 * @brief libyang context provider.
 */
#define UTEST_LYCTX (_UC->ctx)

/**
 * @brief Context's input handler provider
 */
#define UTEST_IN (_UC->in)

/**
 * @brief Context's input handler provider
 */
#define UTEST_OUT (_UC->out)

/**
 * @brief Parse (and validate) data from the input handler as a YANG data tree.
 *
 * @param[in] INPUT The input data in the specified @p format to parse (and validate)
 * @param[in] INPUT_FORMAT Format of the input data to be parsed. Can be 0 to try to detect format from the input handler.
 * @param[in] PARSE_OPTIONS Options for parser, see @ref dataparseroptions.
 * @param[in] VALIDATE_OPTIONS Options for the validation phase, see @ref datavalidationoptions.
 * @param[in] RET expected return status
 * @param[out] OUT_NODE Resulting data tree built from the input data. Note that NULL can be a valid result as a
 * representation of an empty YANG data tree.
 */
#define CHECK_PARSE_LYD_PARAM(INPUT, INPUT_FORMAT, PARSE_OPTIONS, VALIDATE_OPTIONS, RET, OUT_NODE) \
    { \
        LY_ERR _r = lyd_parse_data_mem(_UC->ctx, INPUT, INPUT_FORMAT, PARSE_OPTIONS, VALIDATE_OPTIONS, &OUT_NODE); \
        if (_r != RET) { \
            if (_r) { \
                fail_msg("%s != 0x%d; MSG: %s", #RET, _r, ly_err_last(_UC->ctx)->msg); \
            } else { \
                fail_msg("%s != 0x%d", #RET, _r); \
            } \
        } \
    }

/**
 * @brief Check if lyd_node and his subnodes have correct values. Print lyd_node and his sunodes int o string in json or xml format.
 * @param[in] NODE pointer to lyd_node
 * @param[in] TEXT expected output string in json or xml format.
 * @param[in] FORMAT format of input text. LYD_JSON, LYD_XML
 * @param[in] PARAM  options [Data printer flags](@ref dataprinterflags).
 */
#define CHECK_LYD_STRING_PARAM(NODE, TEXT, FORMAT, PARAM) \
    { \
        char *str; \
        LY_ERR _r = lyd_print_mem(&str, NODE, FORMAT, PARAM); \
        if (_r) { \
            fail_msg("Print err 0x%d; MSG: %s", _r, ly_err_last(_UC->ctx)->msg); \
        } \
        assert_string_equal(str, TEXT); \
        free(str); \
    }

/**
 * @brief Compare two lyd_node structure. Macro print lyd_node structure into string and then compare string. Print function use these two parameters. LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK;
 * @param[in] NODE_1 pointer to lyd_node
 * @param[in] NODE_2 pointer to lyd_node
 */
#define CHECK_LYD(NODE_1, NODE_2) \
    { \
        char *str1; \
        char *str2; \
        assert_int_equal(LY_SUCCESS, lyd_print_mem(&str1, NODE_1, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK)); \
        assert_int_equal(LY_SUCCESS, lyd_print_mem(&str2, NODE_2, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK)); \
        assert_non_null(str1); \
        assert_non_null(str2); \
        assert_string_equal(str1, str2); \
        free(str1); \
        free(str2); \
    }

/*
 * SUPPORT MACROS
 */

/**
 * @brief Internal macro witch assert that two given string are equal or are both null.
 *
 * @param[in] STRING string to check
 * @param[in] TEXT   string to compare
 */
#define CHECK_STRING(STRING, TEXT)\
    if (TEXT == NULL) { \
        assert_null(STRING); \
    } else { \
        assert_non_null(STRING); \
        assert_string_equal(STRING, TEXT); \
    }

/**
 * @brief Internal macro witch assert that pointer is null when flag is 0.
 *
 * @param[in] POINTER pointer to check
 * @param[in] FLAG    0 -> pointer is NULL, 1 -> pointer is not null
 */
#define CHECK_POINTER(POINTER, FLAG) \
    assert_true(FLAG == 0 ? POINTER == NULL : POINTER != NULL)

/**
 * @brief Internal macro check size of [sized array](@ref sizedarrays)'s
 *
 * @param[in] ARRAY pointer to [sized array](@ref sizedarrays)
 * @param[in] SIZE  expected [sized array](@ref sizedarrays) size of array
 */
#define CHECK_ARRAY(ARRAY, SIZE) \
    assert_true((SIZE == 0) ? \
                (ARRAY == NULL) : \
                (ARRAY != NULL && SIZE == LY_ARRAY_COUNT(ARRAY)));

/*
 *   LIBYANG NODE CHECKING
 */

/**
 * @brief check compileted type
 *
 * @param[in] NODE pointer to lysc_type value
 * @param[in] TYPE expected type [LY_DATA_TYPE](@ref LY_DATA_TYPE)
 * @param[in] EXTS expected [sized array](@ref sizedarrays) size of extens list
 */
#define CHECK_LYSC_TYPE(NODE, TYPE, EXTS) \
    assert_non_null(NODE); \
    assert_int_equal((NODE)->basetype, TYPE); \
    CHECK_ARRAY((NODE)->exts, EXTS); \
    assert_ptr_equal((NODE)->plugin, lyplg_type_plugin_find("", NULL, ly_data_type2str[TYPE]))

/**
 * @brief check compileted numeric type
 *
 * @param[in] NODE pointer to lysc_type_num value
 * @param[in] TYPE expected type [LY_DATA_TYPE](@ref LY_DATA_TYPE)
 * @param[in] EXTS expected [sized array](@ref sizedarrays) size of extens list
 * @warning only integer types INT, UINT, NUM
 */
#define CHECK_LYSC_TYPE_NUM(NODE, TYPE, EXTS, RANGE) \
    CHECK_LYSC_TYPE(NODE, TYPE, EXTS);\
    CHECK_POINTER((NODE)->range, RANGE)

/**
 * @brief check compiled string type
 *
 * @param[in] NODE     pointer to lysc_type_num value
 * @param[in] EXTS     expected [sized array](@ref sizedarrays) size of extens list
 * @param[in] LENGTH   0 -> node dosnt have length limitation, 1 -> node have length limitation
 * @param[in] PATTERNS expected number of patterns [sized array](@ref sizedarrays)
 * @warning only integer types INT, UINT, NUM
 */
#define CHECK_LYSC_TYPE_STR(NODE, EXTS, LENGTH, PATTERNS) \
    CHECK_LYSC_TYPE(NODE, LY_TYPE_STRING, EXTS); \
    CHECK_POINTER((NODE)->length, LENGTH); \
    CHECK_ARRAY((NODE)->patterns, PATTERNS)

/**
 * @brief check compiled bits type
 *
 * @param[in] NODE     pointer to lysc_type_num value
 * @param[in] EXTS     expected [sized array](@ref sizedarrays) size of extens list
 * @param[in] BITS     expected number of bits
 * @warning only integer types INT, UINT, NUM
 */
#define CHECK_LYSC_TYPE_BITS(NODE, EXTS, BITS) \
    CHECK_LYSC_TYPE(NODE, LY_TYPE_BITS, EXTS); \
    CHECK_ARRAY((NODE)->bits, BITS)

#define CHECK_LYSC_TYPE_BITENUM_ITEM(NODE, POSITION, DSC, EXTS, FLAGS, NAME, REF)\
    assert_non_null(NODE); \
    assert_int_equal((NODE)->position, POSITION); \
    CHECK_STRING((NODE)->dsc, DSC); \
    CHECK_ARRAY((NODE)->exts, EXTS); \
    assert_int_equal((NODE)->flags, FLAGS); \
    CHECK_STRING((NODE)->name, NAME); \
    CHECK_STRING((NODE)->ref, REF) \

/**
 * @brief check range
 *
 * @param[in] NODE     pointer to lysc_range value
 * @param[in] DSC      expected descriptin (string)
 * @param[in] EAPPTAG  expected string reprezenting error-app-tag value
 * @param[in] EMSG     expected string reprezenting error message
 * @param[in] EXTS     expected [sized array](@ref sizedarrays) size of extens list
 * @param[in] PARTS    expected [sized array](@ref sizedarrays) number of rang limitations
 * @param[in] REF      expected reference
 */
#define CHECK_LYSC_RANGE(NODE, DSC, EAPPTAG, EMSG, EXTS, PARTS, REF) \
    assert_non_null(NODE); \
    CHECK_STRING((NODE)->dsc, DSC); \
    CHECK_STRING((NODE)->eapptag, EAPPTAG); \
    CHECK_STRING((NODE)->emsg, EMSG); \
    CHECK_ARRAY((NODE)->exts, EXTS); \
    CHECK_ARRAY((NODE)->parts, PARTS); \
    CHECK_STRING((NODE)->ref, REF)

/**
 * @brief check pattern
 *
 * @param[in] NODE     pointer to lysc_pattern value
 * @param[in] DSC      expected descriptin (string)
 * @param[in] EAPPTAG  expected string reprezenting error-app-tag value
 * @param[in] EMSG     expected string reprezenting error message
 * @param[in] EEXPR    expected string reprezenting original, not compiled, regular expression
 * @param[in] EXTS     expected [sized array](@ref sizedarrays) size of extens list
 * @param[in] INVERTED if regular expression is inverted.
 * @param[in] REF      expected reference
 */
#define CHECK_LYSC_PATTERN(NODE, DSC, EAPPTAG, EMSG, EXPR, EXTS, INVERTED, REF) \
    assert_non_null(NODE); \
    assert_non_null((NODE)->code); \
    CHECK_STRING((NODE)->dsc, DSC); \
    CHECK_STRING((NODE)->eapptag, EAPPTAG); \
    CHECK_STRING((NODE)->emsg, EMSG); \
    CHECK_STRING((NODE)->expr, EXPR); \
    CHECK_ARRAY((NODE)->exts, EXTS); \
    assert_int_equal((NODE)->inverted, INVERTED); \
    CHECK_STRING((NODE)->ref, REF)

/**
 * @brief assert that lysp_action_inout structure members are correct
 *
 * @param[in] NODE      pointer to lysp_action_inout variable
 * @param[in] DATA      0 -> check if pointer to data is NULL, 1 -> check if pointer to data is not null
 * @param[in] EXTS      expected [sized array](@ref sizedarrays) size of extens list
 * @param[in] GROUPINGS expected [sized array](@ref sizedarrays) size of grouping list
 * @param[in] MUSTS     expected [sized array](@ref sizedarrays) size of must restriction list
 * @param[in] NODETYPE  node type. LYS_INPUT or LYS_OUTPUT
 * @param[in] PARENT    0 -> check if node is root, 1 -> check if node is not root
 * @param[in] TYPEDEFS  expected [sized array](@ref sizedarrays) size of typedefs list
 */
#define CHECK_LYSP_ACTION_INOUT(NODE, DATA, EXTS, GROUPINGS, MUSTS, NODETYPE, PARENT, TYPEDEFS) \
    assert_non_null(NODE); \
    CHECK_POINTER((NODE)->child, DATA); \
    CHECK_ARRAY((NODE)->exts, EXTS); \
    CHECK_POINTER((NODE)->groupings, GROUPINGS); \
    CHECK_ARRAY((NODE)->musts, MUSTS); \
    assert_int_equal((NODE)->nodetype, NODETYPE); \
    CHECK_POINTER((NODE)->parent, PARENT); \
    CHECK_ARRAY((NODE)->typedefs, TYPEDEFS);

/**
 * @brief assert that lysp_action structure members are correct
 *
 * @param[in] NODE    pointer to lysp_action variable
 * @param[in] DSC     expected description
 * @param[in] EXTS    expected [sized array](@ref sizedarrays) size of extension list
 * @param[in] FLAGS   expected [schema node flags](@ref snodeflags)
 * @param[in] GROUPINGS expected [sized array](@ref sizedarrays) size of grouping list
 * @param[in] IFFEATURES expected [sized array](@ref sizedarrays) size of if-feature expressions list
 * @param[in] INPUT_*    ::LYSP_ACTION_INOUT_CHECK
 * @param[in] NAME     expected name
 * @param[in] NODETYPE node type. LYS_RPC or LYS_ACTION
 * @param[in] OUTPUT_*    ::LYSP_ACTION_INOUT_CHECK
 * @param[in] PARENT   0-> check if node is root, 1-> check if node is not root
 * @param[in] REF      expected reference
 * @param[in] TYPEDEFS expected [sized array](@ref sizedarrays) size of list of typedefs
 */
#define CHECK_LYSP_ACTION(NODE, DSC, EXTS, FLAGS, GROUPINGS, IFFEATURES, \
                INPUT_DATA, INPUT_EXTS, INPUT_GROUPINGS, INPUT_MUSTS, \
                INPUT_PARENT, INPUT_TYPEDEFS, \
                NAME, NODETYPE, \
                OUTPUT_DATA, OUTPUT_EXTS, OUTPUT_GROUPINGS, OUTPUT_MUSTS, \
                OUTPUT_PARENT, OUTPUT_TYPEDEFS, \
                PARENT, REF, TYPEDEFS) \
    assert_non_null(NODE); \
    CHECK_STRING((NODE)->dsc, DSC); \
    CHECK_ARRAY((NODE)->exts, EXTS); \
    assert_int_equal((NODE)->flags, FLAGS); \
    CHECK_POINTER((NODE)->groupings, GROUPINGS); \
    CHECK_ARRAY((NODE)->iffeatures, IFFEATURES); \
    CHECK_LYSP_ACTION_INOUT(&((NODE)->input), INPUT_DATA, INPUT_EXTS, INPUT_GROUPINGS, \
                INPUT_MUSTS, LYS_INPUT, INPUT_PARENT, INPUT_TYPEDEFS); \
    assert_string_equal((NODE)->name, NAME); \
    assert_int_equal((NODE)->nodetype, NODETYPE); \
    CHECK_LYSP_ACTION_INOUT(&((NODE)->output), OUTPUT_DATA, OUTPUT_EXTS, OUTPUT_GROUPINGS, \
                OUTPUT_MUSTS, LYS_OUTPUT, OUTPUT_PARENT, OUTPUT_TYPEDEFS); \
    CHECK_POINTER((NODE)->parent, PARENT); \
    CHECK_STRING((NODE)->ref, REF); \
    CHECK_ARRAY((NODE)->typedefs, TYPEDEFS) \

/**
 * @brief assert that lysp_when structure members are correct
 *
 * @param[in] NODE pointer to lysp_when variable
 * @param[in] COND expected string specifid condition
 * @param[in] DSC  expected string description statement
 * @param[in] EXTS expected [sized array](@ref sizedarrays) size of list of extension array
 * @param[in] REF  expected string reference
 */
#define CHECK_LYSP_WHEN(NODE, COND, DSC, EXTS, REF) \
    assert_non_null(NODE); \
    assert_string_equal((NODE)->cond, COND); \
    CHECK_STRING((NODE)->dsc, DSC); \
    CHECK_ARRAY((NODE)->exts, EXTS); \
    if (REF == NULL) { \
        assert_null((NODE)->ref); \
    } else { \
        assert_non_null((NODE)->ref); \
        assert_string_equal((NODE)->ref, REF); \
    }

/**
 * @brief assert that lysp_restr structure members are correct
 *
 * @param[in] NODE pointer to lysp_restr variable
 * @param[in] ARG_STR expected string. The restriction expression/value
 * @param[in] DSC     expected descrition
 * @param[in] EAPPTAG expected string reprezenting error-app-tag value
 * @param[in] EMSG    expected string reprezenting error message
 * @param[in] EXTS    expected [sized array](@ref sizedarrays) size of list of extension array
 * @param[in] REF     expected reference
 */

#define CHECK_LYSP_RESTR(NODE, ARG_STR, DSC, EAPPTAG, EMSG, EXTS, REF) \
    assert_non_null(NODE); \
    assert_non_null((NODE)->arg.mod); \
    assert_string_equal((NODE)->arg.str, ARG_STR); \
    CHECK_STRING((NODE)->dsc, DSC); \
    CHECK_STRING((NODE)->eapptag, EAPPTAG); \
    CHECK_STRING((NODE)->emsg, EMSG); \
    CHECK_ARRAY((NODE)->exts, EXTS); \
    CHECK_STRING((NODE)->ref, REF);

/**
 * @brief assert that lysp_import structure members are correct
 *
 * @param[in] NODE   pointer to lysp_import variable
 * @param[in] DSC    expected description or NULL
 * @param[in] EXTS   expected [sized array](@ref sizedarrays) size of list of extensions
 * @param[in] NAME   expected name of imported module
 * @param[in] PREFIX expected prefix for the data from the imported schema
 * @param[in] REF    expected reference
 * @param[in] REV    expected reprezenting date in format "11-10-2020"
 */
#define CHECK_LYSP_IMPORT(NODE, DSC, EXTS, NAME, PREFIX, REF, REV) \
    assert_non_null(NODE); \
    CHECK_STRING((NODE)->dsc, DSC); \
    CHECK_ARRAY((NODE)->exts, EXTS); \
    assert_string_equal((NODE)->name, NAME); \
    assert_string_equal((NODE)->prefix, PREFIX); \
    CHECK_STRING((NODE)->ref, REF); \
    CHECK_STRING((NODE)->rev, REV); \

/**
 * @brief assert that lysp_ext structure members are correct
 *
 * @param[in] NODE pointer to lysp_ext_instance variable
 * @param[in] ARGNAME expected argument name
 * @param[in] COMPILED 0 -> compiled data dosnt exists, 1 -> compiled data exists
 * @param[in] DSC      expected string reprezent description
 * @param[in] EXTS     expected [sized array](@ref sizedarrays) size of list of extension instances
 * @param[in] FLAGS    expected LYS_STATUS_* and LYS_YINELEM_* values (@ref snodeflags)
 * @param[in] NAME     expected name
 * @param[in] REF      expected ref
 */
#define CHECK_LYSP_EXT(NODE, ARGNAME, COMPILED, DSC, EXTS, FLAGS, NAME, REF) \
    assert_non_null(NODE); \
    CHECK_STRING((NODE)->argname, ARGNAME); \
    CHECK_POINTER((NODE)->compiled, COMPILED); \
    CHECK_STRING((NODE)->dsc, DSC); \
    CHECK_ARRAY((NODE)->exts, EXTS); \
    assert_int_equal((NODE)->flags, FLAGS); \
    assert_string_equal((NODE)->name, NAME); \
    CHECK_STRING((NODE)->ref, REF);

/**
 * @brief assert that lysp_ext_instance structure members are correct
 *
 * @param[in] NODE      pointer to lysp_ext_instance variable
 * @param[in] ARGUMENT  expected optional value of the extension's argument
 * @param[in] CHILD     0 -> node doesnt have child, 1 -> node have children
 * @param[in] PARENT_STMT expected value identifying placement of the extension instance
 * @param[in] PARENT_STMT_INDEX expected indentifi index
 * @param[in] FORMAT    expected format
 */
#define CHECK_LYSP_EXT_INSTANCE(NODE, ARGUMENT, CHILD, PARENT_STMT, PARENT_STMT_INDEX, NAME, FORMAT) \
    assert_non_null(NODE); \
    CHECK_STRING((NODE)->argument, ARGUMENT); \
    CHECK_POINTER((NODE)->child, CHILD); \
    assert_int_equal((NODE)->parent_stmt, PARENT_STMT); \
    assert_int_equal((NODE)->parent_stmt_index, PARENT_STMT_INDEX); \
    assert_string_equal((NODE)->name, NAME); \
    assert_int_equal((NODE)->format, FORMAT);

/**
 * @brief assert that lysp_stmt structure members are correct
 *
 * @param[in] NODE  pointer to lysp_stmt variable
 * @param[in] ARG   expected statemet argumet
 * @param[in] CHILD 0 -> node doesnt have child, 1 -> node have children
 * @param[in] FLAGS expected statement flags, can be set to LYS_YIN_ATTR
 * @param[in] KW    expected numeric respresentation of the stmt value
 * @param[in] NEXT  0 -> pointer is NULL, 1 -> pointer is not null
 * @param[in] STMS  expected identifier of the statement
 */
#define CHECK_LYSP_STMT(NODE, ARG, CHILD, FLAGS, KW, NEXT, STMT) \
    assert_non_null(NODE); \
    CHECK_STRING((NODE)->arg, ARG); \
    CHECK_POINTER((NODE)->child, CHILD); \
    assert_int_equal((NODE)->flags, FLAGS); \
    assert_int_equal((NODE)->kw, KW); \
    CHECK_POINTER((NODE)->next, NEXT); \
    assert_string_equal((NODE)->stmt, STMT); \

/**
 * @brief assert that lysp_type_enum structure members are correct
 *
 * @param[in] NODE pointer to lysp_type_enum variable
 * @param[in] DSC   expected description
 * @param[in] EXTS  expected [sized array](@ref sizedarrays) size of list of the extension instances
 * @param[in] FLAGS only LYS_STATUS_ and LYS_SET_VALUE values are allowed
 * @param[in] IFFEATURES expected [sized array](@ref sizedarrays) size of list of the extension instances
 * @param[in] NAME  expected name
 * @param[in] REF   expected reference statement
 * @param[in] VALUE expected enum's value or bit's position
 */
#define CHECK_LYSP_TYPE_ENUM(NODE, DSC, EXTS, FLAGS, IFFEATURES, NAME, REF, VALUE) \
    assert_non_null(NODE); \
    CHECK_STRING((NODE)->dsc, DSC); \
    CHECK_ARRAY((NODE)->exts, EXTS); \
    assert_int_equal((NODE)->flags, FLAGS); \
    CHECK_ARRAY((NODE)->iffeatures, IFFEATURES); \
    CHECK_STRING((NODE)->name, NAME); \
    CHECK_STRING((NODE)->ref, REF); \
    assert_int_equal(VALUE, (NODE)->value);

/**
 * @brief assert that lysp_type_enum structure members are correct
 *
 * @param[in] NODE pointer to lysp_type variable
 * @param[in] BASES  expected [sized array](@ref sizedarrays) size of list of indentifiers
 * @param[in] BITS   expected [sized array](@ref sizedarrays) size of list of bits
 * @param[in] COMPILED 0 -> pointer to compiled type is null, 1 -> pointer to compilet type is valid
 * @param[in] ENUMS  expected [sized array](@ref sizedarrays) size of list of enums-stmts
 * @param[in] EXTS   expected [sized array](@ref sizedarrays) size of list of extension instances
 * @param[in] FLAGS  expected flags
 * @param[in] FRACTION_DIGITS expected number of fraction digits decimal64
 * @param[in] LENGTH expected 0 -> there isnt any restriction on length, 1 -> type is restricted on length (string, binary)
 * @param[in] NAME   expected name of type
 * @param[in] PATH   0 -> no pointer to parsed path, 1 -> pointer to parsed path is valid
 * @param[in] PATTERNS expected [sized array](@ref sizedarrays) size of list of patterns for string
 * @param[in] PMOD   expected submodule where type is defined 0 -> pointer is null, 1 -> pointer is not null
 * @param[in] RANGE   expected [sized array](@ref sizedarrays) size of list of range restriction
 * @param[in] REQUIRE_INSTANCE expected require instance flag
 * @param[in] TYPES   expected [sized array](@ref sizedarrays) size of list of sub-types
 */
#define CHECK_LYSP_TYPE(NODE, BASES, BITS, COMPILED, ENUMS, EXTS, FLAGS, FRACTIONS_DIGITS, \
                LENGTH, NAME, PATH, PATTERNS, PMOD, RANGE, REQUIRE_INSTANCE, TYPES) \
    assert_non_null(NODE);\
    CHECK_ARRAY((NODE)->bases, BASES); \
    CHECK_ARRAY((NODE)->bits, BITS); \
    CHECK_POINTER((NODE)->compiled, COMPILED); \
    CHECK_ARRAY((NODE)->enums, ENUMS); \
    CHECK_ARRAY((NODE)->exts, EXTS); \
    assert_int_equal((NODE)->flags, FLAGS); \
    assert_int_equal((NODE)->fraction_digits, FRACTIONS_DIGITS); \
    CHECK_POINTER((NODE)->length, LENGTH); \
    CHECK_STRING((NODE)->name, NAME); \
    CHECK_POINTER((NODE)->path, PATH); \
    CHECK_ARRAY((NODE)->patterns, PATTERNS); \
    CHECK_POINTER((NODE)->pmod, PMOD); \
    CHECK_POINTER((NODE)->range, RANGE); \
    assert_int_equal((NODE)->require_instance, REQUIRE_INSTANCE); \
    CHECK_ARRAY((NODE)->types , TYPES)

/**
 * @brief assert that lysp_node structure members are correct
 *
 * @param[in] NODE  pointer to lysp_node variable
 * @param[in] DSC   expected description statement
 * @param[in] EXTS  expected [sized array](@ref sizedarrays) size of list of the extension instances
 * @param[in] FLAGS [schema node flags](@ref snodeflags)
 * @param[in] IFFEATURES expected [sized array](@ref sizedarrays) size of list of the extension instances
 * @param[in] NAME  expected name
 * @param[in] NEXT  0 pointer is null, 1 pointer is not null
 * @param[in] NODETYPE  node type LYS_UNKNOWN, LYS_CONTAINER, LYS_CHOICE, LYS_LEAF, LYS_LEAFLIST,
 *                      LYS_LIST, LYS_ANYXML, LYS_ANYDATA, LYS_CASE, LYS_RPC, LYS_ACTION, LYS_NOTIF,
 *                      LYS_USES, LYS_INPUT, LYS_OUTPUT, LYS_GROUPING, LYS_AUGMENT
 * @param[in] PARENT    0-> check if node is root, 1-> check if node is not root
 * @param[in] REF       expected reference statement
 * @param[in] WHEN      0-> pointer is null, 1 -> pointer is not null
 */
#define CHECK_LYSP_NODE(NODE, DSC, EXTS, FLAGS, IFFEATURES, NAME, NEXT, NODETYPE, PARENT, REF, WHEN) \
    assert_non_null(NODE); \
    CHECK_STRING((NODE)->dsc, DSC); \
    CHECK_ARRAY((NODE)->exts, EXTS); \
    assert_int_equal((NODE)->flags, FLAGS); \
    CHECK_ARRAY((NODE)->iffeatures, IFFEATURES); \
    CHECK_STRING((NODE)->name, NAME); \
    CHECK_POINTER((NODE)->next, NEXT); \
    assert_int_equal((NODE)->nodetype, NODETYPE); \
    CHECK_POINTER((NODE)->parent, PARENT); \
    CHECK_STRING((NODE)->ref, REF); \
    CHECK_POINTER(lysp_node_when((struct lysp_node *)NODE), WHEN);

/**
 * @brief assert that lysp_node structure members are correct
 *
 * @param[in] NODE  pointer to lysp_node variable
 * @param[in] DSC   expected description statement
 * @param[in] EXTS  expected [sized array](@ref sizedarrays) size of list of the extension instances
 * @param[in] FLAGS [schema node flags](@ref snodeflags)
 * @param[in] IFFEATURES expected [sized array](@ref sizedarrays) size of list of the extension instances
 * @param[in] NAME  expected name
 * @param[in] NEXT  0 pointer is null, 1 pointer is not null
 * @param[in] PARENT    0-> check if node is root, 1-> check if node is not root
 * @param[in] REF       expected reference statement
 * @param[in] WHEN      0-> pointer is null, 1 -> pointer is not null
 * @param[in] MUSTS     expected [sized array](@ref sizedarrays) size of list of must restriction
 * @param[in] UNITS     expected string reprezenting units
 * @param[in] DFLT      0-> node dosn't have default value. 1 -> node have default value
 */
#define CHECK_LYSP_NODE_LEAF(NODE, DSC, EXTS, FLAGS, IFFEATURES, NAME, NEXT, \
                PARENT, REF, WHEN, MUSTS, UNITS, DFLT) \
    CHECK_LYSP_NODE(NODE, DSC, EXTS, FLAGS, IFFEATURES, NAME, NEXT, LYS_LEAF, PARENT, REF, WHEN); \
    CHECK_ARRAY((NODE)->musts, MUSTS); \
    CHECK_STRING((NODE)->units, UNITS); \
    CHECK_STRING((NODE)->dflt.str, DFLT);

/**
 * @brief assert that lysc_notif structure members are correct
 *
 * @param[in] NODE    pointer to lysp_notif variable
 * @param[in] DATA    0 pointer is null, 1 pointer is not null
 * @param[in] DSC     expected description
 * @param[in] EXTS    expected [sized array](@ref sizedarrays) size of list of the extension instances
 * @param[in] FLAGS   [schema node flags](@ref snodeflags)
 * @param[in] MODULE  0 pointer is null, 1 pointer is not null
 * @param[in] MUSTS   expected [sized array](@ref sizedarrays) size of list of must restriction
 * @param[in] NAME    expected name
 * @param[in] PARENT  0-> check if node is root, 1-> check if node is not root
 * @param[in] PRIV    0-> pointer is null, 1-> pointer is not null
 * @param[in] REF     expected reference
 * @param[in] WHEN    expected [sized array](@ref sizedarrays) size of list of pointers to when statements
 */
#define CHECK_LYSC_NOTIF(NODE, DATA, DSC, EXTS, FLAGS, MODULE, MUSTS, NAME, PARENT, PRIV, REF, WHEN) \
    assert_non_null(NODE); \
    CHECK_POINTER((NODE)->child, DATA); \
    CHECK_STRING((NODE)->dsc, DSC); \
    CHECK_ARRAY((NODE)->exts, EXTS); \
    assert_int_equal((NODE)->flags, FLAGS); \
    CHECK_POINTER((NODE)->module, MODULE); \
    CHECK_ARRAY((NODE)->musts, MUSTS); \
    assert_string_equal((NODE)->name, NAME); \
    assert_int_equal((NODE)->nodetype, LYS_NOTIF); \
    CHECK_POINTER((NODE)->parent, PARENT); \
    CHECK_POINTER((NODE)->priv, PRIV); \
    CHECK_STRING((NODE)->ref, REF); \
    CHECK_ARRAY(lysc_node_when((const struct lysc_node *)NODE), WHEN);

/**
 * @brief assert that lysc_action_inout structure members are correct
 *
 * @param[in] NODE      pointer to lysp_notif variable
 * @param[in] DATA      0 pointer is null, 1 pointer is not null
 * @param[in] MUST      expected [sized array](@ref sizedarrays) size of list of must restrictions
 * @param[in] NODETYPE  LYS_INPUT or LYS_OUTPUT
 */
#define CHECK_LYSC_ACTION_INOUT(NODE, DATA, MUST, NODETYPE) \
    assert_non_null(NODE); \
    CHECK_POINTER((NODE)->child, DATA); \
    CHECK_ARRAY((NODE)->musts, MUST); \
    assert_int_equal((NODE)->nodetype, NODETYPE);

/**
 * @brief assert that lysc_action structure members are correct
 *
 * @param[in] NODE    pointer to lysp_action variable
 * @param[in] DSC     string description statement
 * @param[in] EXTS    expected [sized array](@ref sizedarrays) size of list of the extension instances
 * @param[in] FLAGS   [schema node flags](@ref snodeflags)
 * @param[in] INPUT_DATA 0 pointer is null, 1 pointer is not null
 * @param[in] INPUT_MUST expected [sized array](@ref sizedarrays) size of input list of must restrictions
 * @param[in] INPUT_EXTS expected [sized array](@ref sizedarrays) size of the input extension instances of input
 * @param[in] MODULE  0 pointer is null, 1 pointer is not null
 * @param[in] NAME    expected name
 * @param[in] NODETYPE    LYS_RPC, LYS_ACTION
 * @param[in] OUTPUT_DATA 0 pointer is null, 1 pointer is not null
 * @param[in] OUTPUT_MUST expected [sized array](@ref sizedarrays) size of output list of must restrictions
 * @param[in] OUTPUT_EXTS expected [sized array](@ref sizedarrays) size of the output extension instances of input
 * @param[in] PARENT  0-> check if node is root, 1-> check if node is not root
 * @param[in] PRIV    0-> pointer is null, 1-> pointer is not null
 * @param[in] REF     expected reference
 * @param[in] WHEN    expected [sized array](@ref sizedarrays) size of list of pointers to when statements
 */
#define CHECK_LYSC_ACTION(NODE, DSC, EXTS, FLAGS, INPUT_DATA, INPUT_MUST, INPUT_EXTS, MODULE, NAME, NODETYPE, \
                OUTPUT_DATA, OUTPUT_MUST, OUTPUT_EXTS, PARENT, PRIV, REF, WHEN) \
    assert_non_null(NODE); \
    CHECK_STRING((NODE)->dsc, DSC); \
    CHECK_ARRAY((NODE)->exts, EXTS); \
    assert_int_equal((NODE)->flags, FLAGS); \
    CHECK_LYSC_ACTION_INOUT(&(NODE)->input, INPUT_DATA, INPUT_MUST, LYS_INPUT); \
    CHECK_ARRAY((NODE)->input.exts, INPUT_EXTS); \
    CHECK_POINTER((NODE)->module, MODULE); \
    assert_string_equal((NODE)->name, NAME); \
    assert_int_equal((NODE)->nodetype, NODETYPE); \
    CHECK_LYSC_ACTION_INOUT(&(NODE)->output, OUTPUT_DATA, OUTPUT_MUST, LYS_OUTPUT); \
    CHECK_ARRAY((NODE)->output.exts, OUTPUT_EXTS); \
    CHECK_POINTER((NODE)->parent, PARENT); \
    CHECK_POINTER((NODE)->priv, PRIV); \
    CHECK_STRING((NODE)->ref, REF); \
    CHECK_ARRAY(lysc_node_when((const struct lysc_node *)NODE), WHEN);

/**
 * @brief assert that lysc_node structure members are correct
 *
 * @param[in] NODE    pointer to lysc_node variable
 * @param[in] DSC     expected description
 * @param[in] EXTS    expected [sized array](@ref sizedarrays) size of list of the extension instances
 * @param[in] FLAGS   [schema node flags](@ref snodeflags)
 * @param[in] MODULE  0 pointer is null, 1 pointer is not null
 * @param[in] NAME    expected name
 * @param[in] NEXT    0 pointer is null, 1 pointer is not null
 * @param[in] NODETYPE    type of the node  LYS_UNKNOWN, LYS_CONTAINER, LYS_CHOICE, LYS_LEAF,
 *                    LYS_LEAFLIST, LYS_LIST, LYS_ANYXML, LYS_ANYDATA, LYS_CASE, LYS_RPC,
 *                    LYS_ACTION, LYS_NOTIF, LYS_USES, LYS_INPUT, LYS_OUTPUT, LYS_GROUPING,
 *                    LYS_AUGMENT
 * @param[in] PARENT  0-> check if node is root, 1-> check if node is not root
 * @param[in] PRIV    0-> pointer is null, 1-> pointer is not null
 * @param[in] REF     expected reference
 * @param[in] WHEN    expected [sized array](@ref sizedarrays) size of list of pointers to when statements
 */
#define CHECK_LYSC_NODE(NODE, DSC, EXTS, FLAGS, MODULE, NAME, NEXT, NODETYPE, PARENT, PRIV, REF, WHEN) \
    assert_non_null(NODE); \
    CHECK_STRING((NODE)->dsc, DSC); \
    CHECK_ARRAY((NODE)->exts, EXTS); \
    assert_int_equal((NODE)->flags, FLAGS); \
    CHECK_POINTER((NODE)->module, MODULE); \
    assert_string_equal((NODE)->name, NAME); \
    CHECK_POINTER((NODE)->next, NEXT); \
    assert_int_equal((NODE)->nodetype, NODETYPE); \
    CHECK_POINTER((NODE)->parent, PARENT); \
    assert_non_null((NODE)->prev); \
    CHECK_POINTER((NODE)->priv, PRIV); \
    CHECK_STRING((NODE)->ref, REF); \
    CHECK_ARRAY(lysc_node_when((const struct lysc_node *)NODE), WHEN);

/**
 * @brief assert that lysc_node_leaf structure members are correct
 *
 * @param[in] NODE    pointer to lysc_node variable
 * @param[in] DSC     expected description
 * @param[in] EXTS    expected [sized array](@ref sizedarrays) size of list of the extension instances
 * @param[in] FLAGS   [schema node flags](@ref snodeflags)
 * @param[in] MODULE  0 pointer is null, 1 pointer is not null
 * @param[in] NAME    expected name
 * @param[in] NEXT    0 pointer is null, 1 pointer is not null
 * @param[in] PARENT  0-> check if node is root, 1-> check if node is not root
 * @param[in] PRIV    0-> pointer is null, 1-> pointer is not null
 * @param[in] REF     expected reference
 * @param[in] ACTIONS 1 if is set pointer to structure lysc_node_action other 0
 * @param[in] CHILD   1 if is set pointer to child other 0
 * @param[in] MAX     possible maximum elements in list
 * @param[in] MIN     possible minimum elements in list
 * @param[in] MUSTS   [sized array](@ref sizedarrays) number of must node elements in array
 * @param[in] NOTIFS  1 if is set pointer to any notifs node
 * @param[in] UNIQUES [sized array](@ref sizedarrays) number of unique nodes element in array
 * @param[in] WHEN    [sized array](@ref sizedarrays) size of when node array
 */
#define CHECK_LYSC_NODE_LIST(NODE, DSC, EXTS, FLAGS, MODULE, NAME, NEXT, \
                PARENT, PRIV, REF, ACTIONS, CHILD, MAX, MIN, MUSTS, NOTIFS, UNIQUES, WHEN) \
        CHECK_LYSC_NODE(NODE, DSC, EXTS, FLAGS, MODULE, NAME, NEXT, LYS_LIST, PARENT, PRIV, REF, WHEN); \
        CHECK_POINTER((NODE)->actions, ACTIONS); \
        CHECK_POINTER((NODE)->child, CHILD); \
        assert_int_equal((NODE)->max, MAX); \
        assert_int_equal((NODE)->min, MIN); \
        CHECK_ARRAY((NODE)->musts, MUSTS); \
        CHECK_POINTER((NODE)->notifs, NOTIFS); \
        CHECK_ARRAY((NODE)->uniques, UNIQUES); \
        CHECK_ARRAY(lysc_node_when((const struct lysc_node *)NODE), WHEN)

/**
 * @brief assert that lysc_node_leaf structure members are correct
 *
 * @param[in] NODE    pointer to lysc_node variable
 * @param[in] DSC     expected description
 * @param[in] EXTS    expected [sized array](@ref sizedarrays) size of list of the extension instances
 * @param[in] FLAGS   [schema node flags](@ref snodeflags)
 * @param[in] MODULE  0 pointer is null, 1 pointer is not null
 * @param[in] NAME    expected name
 * @param[in] NEXT    0 pointer is null, 1 pointer is not null
 * @param[in] PARENT  0-> check if node is root, 1-> check if node is not root
 * @param[in] PRIV    0-> pointer is null, 1-> pointer is not null
 * @param[in] REF     expected reference
 * @param[in] WHEN    expected [sized array](@ref sizedarrays) size of list of pointers to when statements
 * @param[in] MUSTS     expected [sized array](@ref sizedarrays) size of list of must restriction
 * @param[in] UNITS     expected string reprezenting units
 * @param[in] DFLT      0-> node dosn't have default value. 1 -> node have default value
 */
#define CHECK_LYSC_NODE_LEAF(NODE, DSC, EXTS, FLAGS, MODULE, NAME, NEXT, \
                PARENT, PRIV, REF, WHEN, MUSTS, UNITS, DFLT) \
    CHECK_LYSC_NODE(NODE, DSC, EXTS, FLAGS, MODULE, NAME, NEXT, LYS_LEAF, PARENT, PRIV, REF, WHEN); \
    CHECK_ARRAY((NODE)->musts, MUSTS); \
    assert_non_null((NODE)->type); \
    CHECK_STRING((NODE)->units, UNITS); \
    CHECK_POINTER((NODE)->dflt, DFLT);

/**
 * @brief assert that lyd_meta structure members are correct
 *
 * @param[in] NODE       pointer to lyd_meta variable
 * @param[in] ANNOTATION 0 pointer is null, 1 pointer is not null
 * @param[in] NAME       expected name
 * @param[in] NEXT       0 pointer is null, 1 pointer is not null
 * @param[in] TYPE_VAL value type. EMPTY, UNION, BITS, INST, ENUM, INT8, INT16, UINT8, STRING, LEAFREF, DEC64, BINARY, BOOL, IDENT
 *                     part of text reprezenting LY_DATA_TYPE.
 * @param[in] ...              ::CHECK_LYD_VALUE
 */
#define CHECK_LYD_META(NODE, ANNOTATION, NAME, NEXT, PARENT, TYPE_VAL, ...) \
    assert_non_null(NODE); \
    CHECK_POINTER((NODE)->annotation, ANNOTATION); \
    assert_string_equal((NODE)->name, NAME); \
    CHECK_POINTER((NODE)->next, NEXT); \
    CHECK_POINTER((NODE)->parent, PARENT); \
    CHECK_LYD_VALUE((NODE)->value, TYPE_VAL, __VA_ARGS__);

/**
 * @brief assert that lyd_node_term structure members are correct
 *
 * @param[in] NODE             pointer to lyd_node_term variable
 * @param[in] FLAGS            expected [data node flags](@ref dnodeflags)
 * @param[in] META             0 -> meta is not prezent, 1 -> meta is prezent
 * @param[in] NEXT             0 -> next node is not prezent, 1 -> next node is prezent
 * @param[in] TYPE_VAL value type. EMPTY, UNION, BITS, INST, ENUM, INT8, INT16, UINT8, STRING, LEAFREF, DEC64, BINARY, BOOL, IDENT
 *                     part of text reprezenting LY_DATA_TYPE.
 * @param[in] ...              ::CHECK_LYD_VALUE
 */
#define CHECK_LYD_NODE_TERM(NODE, FLAGS, META, NEXT, PARENT, SCHEMA, VALUE_TYPE, ...) \
    assert_non_null(NODE); \
    assert_int_equal((NODE)->flags, FLAGS); \
    CHECK_POINTER((NODE)->meta, META); \
    CHECK_POINTER((NODE)->next, NEXT); \
    CHECK_POINTER((NODE)->parent, PARENT); \
    assert_non_null((NODE)->prev); \
    CHECK_POINTER((NODE)->schema, SCHEMA); \
    CHECK_LYD_VALUE((NODE)->value, VALUE_TYPE, __VA_ARGS__);

/**
 * @brief assert that lyd_node_any structure members are correct
 *
 * @param[in] NODE       pointer to lyd_node_term variable
 * @param[in] FLAGS      expected [data node flags](@ref dnodeflags)
 * @param[in] META       0 meta isnt present , 1 meta is present
 * @param[in] PARENT     0 it is root node , 1 node have parent
 * @param[in] VALUE_TYPE value type ::lyd_node_any.value
 */
#define CHECK_LYD_NODE_ANY(NODE, FLAGS, META, PARENT, VALUE_TYPE) \
    assert_non_null(NODE); \
    assert_int_equal((NODE)->flags, FLAGS); \
    CHECK_POINTER((NODE)->meta, META); \
    CHECK_POINTER((NODE)->meta, PARENT); \
    assert_non_null((NODE)->prev); \
    assert_non_null((NODE)->schema); \
    assert_int_equal((NODE)->value_type, VALUE_TYPE);

/**
 * @brief assert that lyd_node_opaq structure members are correct
 *
 * @param[in] NODE     pointer to lyd_node_opaq variable
 * @param[in] ATTR     0 if pointer is null ,1 if pointer is not null
 * @param[in] CHILD    0 if pointer is null ,1 if pointer is not null
 * @param[in] FORMAT   LY_PREF_XML or LY_PREF_JSON
 * @param[in] VAL_PREFS 0 if pointer is null ,1 if pointer is not null
 * @param[in] NAME     expected name
 * @param[in] value    expected orignal value
 */
#define CHECK_LYD_NODE_OPAQ(NODE, ATTR, CHILD, FORMAT, NAME, NEXT, PARENT, PREFIX, VAL_PREFS, VALUE) \
    assert_non_null(NODE); \
    CHECK_POINTER((NODE)->attr, ATTR); \
    CHECK_POINTER((NODE)->child, CHILD); \
    assert_ptr_equal((NODE)->ctx, UTEST_LYCTX); \
    assert_int_equal((NODE)->flags, 0); \
    assert_true((NODE)->format == FORMAT); \
    assert_int_equal((NODE)->hash, 0); \
    assert_string_equal((NODE)->name.name, NAME); \
    assert_non_null((NODE)->prev); \
    assert_null((NODE)->schema); \
    CHECK_POINTER((NODE)->val_prefix_data, VAL_PREFS); \
    assert_string_equal((NODE)->value, VALUE);

/**
 * @brief assert that lyd_node_opaq structure members are correct
 *
 * @param[in] NODE     pointer to lyd_node_opaq variable
 * @param[in] CHILD    1 if node has children other 0
 * @param[in] HILD_HT  1 if node has children hash table other 0
 * @param[in] META     1 if node has metadata other 0
 * @param[in] FLAGS    expected flag
 * @param[in] NEXT     1 if next node is present other 0
 * @param[in] PARENT   1 if node has parent other 0
 * @param[in] PRIV     1 if node has private data other 0
 * @param[in] SCHEMA   1 if node has schema other 0
*/
#define CHECK_LYD_NODE_INNER(NODE, CHILD, CHILD_HT, META, FLAGS, NEXT, PARENT, PRIV, SCHEMA) \
    assert_non_null(NODE); \
    CHECK_POINTER((NODE)->child, CHILD); \
    CHECK_POINTER((NODE)->children_ht, CHILD_HT); \
    CHECK_POINTER((NODE)->meta, META); \
    assert_int_equal((NODE)->flags, FLAGS); \
    CHECK_POINTER((NODE)->parent, PARENT); \
    assert_non_null((NODE)->prev); \
    CHECK_POINTER((NODE)->next, NEXT); \
    CHECK_POINTER((NODE)->priv, PRIV); \
    CHECK_POINTER((NODE)->schema, SCHEMA)

/**
 * @brief assert that lyd_value structure members are correct
 *
 * @param[in] NODE     lyd_value
 * @param[in] TYPE_VAL value type. EMPTY, UNION, BITS, INST, ENUM, INT8, INT16, UINT8, STRING, LEAFREF, DEC64, BINARY, BOOL, IDENT
 *                     part of text reprezenting LY_DATA_TYPE.
 * @param[in] ...      Unspecified parameters. Type and numbers of parameters are specified
 *                     by type of value. These parameters are passed to macro
 *                     CHECK_LYD_VALUE_ ## TYPE_VAL.
 */
#define CHECK_LYD_VALUE(NODE, TYPE_VAL, ...) \
    CHECK_LYD_VALUE_ ## TYPE_VAL (NODE, __VA_ARGS__);

/*
 * LYD VALUES CHECKING SPECIALIZATION
 */

/**
 * @brief Internal macro. Assert that lyd_value structure members are correct. Lyd value is type EMPTY
 *        Example CHECK_LYD_VALUE(node->value, EMPTY, "");
 *
 * @param[in] NODE           lyd_value variable
 * @param[in] CANNONICAL_VAL expected cannonical value
 */
#define CHECK_LYD_VALUE_EMPTY(NODE, CANNONICAL_VAL) \
    assert_non_null((NODE).realtype->plugin->print(UTEST_LYCTX, &(NODE), LY_VALUE_CANON, NULL, NULL, NULL)); \
    assert_string_equal((NODE)._canonical, CANNONICAL_VAL); \
    assert_non_null((NODE).realtype); \
    assert_int_equal((NODE).realtype->basetype, LY_TYPE_EMPTY);

/**
 * @brief Internal macro. Assert that lyd_value structure members are correct. Lyd value is type UNION
 *        Example CHECK_LYD_VALUE(node->value, UNION, "12", INT8, "12", 12);
 * @warning   type of subvalue cannot be UNION. Example of calling
 *
 * @param[in] NODE           lyd_value variable
 * @param[in] CANNONICAL_VAL expected cannonical value
 * @param[in] TYPE_VAL value type. EMPTY, UNION, BITS, INST, ENUM, INT8, INT16, UINT8, STRING, LEAFREF, DEC64, BINARY, BOOL, IDENT
 * @param[in] ...      Unspecified parameters. Type and numbers of parameters are specified
 *                     by type of value. These parameters are passed to macro
 *                     CHECK_LYD_VALUE_ ## TYPE_VAL.
 */
#define CHECK_LYD_VALUE_UNION(NODE, CANNONICAL_VAL, TYPE_VAL, ...) \
    assert_non_null((NODE).realtype->plugin->print(UTEST_LYCTX, &(NODE), LY_VALUE_CANON, NULL, NULL, NULL)); \
    assert_string_equal((NODE)._canonical, CANNONICAL_VAL); \
    assert_non_null((NODE).realtype); \
    assert_int_equal(LY_TYPE_UNION, (NODE).realtype->basetype); \
    assert_non_null((NODE).subvalue); \
    assert_non_null((NODE).subvalue->prefix_data); \
    CHECK_LYD_VALUE_ ## TYPE_VAL ((NODE).subvalue->value, __VA_ARGS__)

/**
 * @brief Internal macro. Get 1st variadic argument.
 */
#define _GETARG1(ARG1, ...) ARG1

/**
 * @brief Internal macro. Assert that lyd_value structure members are correct. Lyd value is type BITS
 *        Example arr[] = {"a", "b"}; CHECK_LYD_VALUE(node->value, BITS, "a b", arr);
 *
 * @param[in] NODE           lyd_value variable
 * @param[in] CANNONICAL_VAL expected cannonical value
 * @param[in] VALUE          expected array of bits names
 */
#define CHECK_LYD_VALUE_BITS(NODE, ...) \
    assert_non_null((NODE).realtype->plugin->print(UTEST_LYCTX, &(NODE), LY_VALUE_CANON, NULL, NULL, NULL)); \
    assert_string_equal((NODE)._canonical, _GETARG1(__VA_ARGS__, DUMMY)); \
    assert_non_null((NODE).realtype); \
    assert_int_equal(LY_TYPE_BITS, (NODE).realtype->basetype); \
    { \
        const char *arr[] = { __VA_ARGS__ }; \
        LY_ARRAY_COUNT_TYPE arr_size = (sizeof(arr) / sizeof(arr[0])) - 1; \
        struct lyd_value_bits *_val; \
        LYD_VALUE_GET(&(NODE), _val); \
        assert_int_equal(arr_size, LY_ARRAY_COUNT(_val->items)); \
        for (LY_ARRAY_COUNT_TYPE it = 0; it < arr_size; it++) { \
            assert_string_equal(arr[it + 1], _val->items[it]->name); \
        } \
    }

/**
 * @brief Internal macro. Assert that lyd_value structure members are correct. Lyd value is type INST
 *
 * @param[in] NODE           lyd_value variable
 * @param[in] CANNONICAL_VAL expected cannonical value
 * @param[in] VALUE          expected array of enum ly_path_pred_type
 * @brief Example enum arr[] = {0x0, 0x1}; CHECK_LYD_VALUE(node->value, INST, "test/d", arr);
 */
#define CHECK_LYD_VALUE_INST(NODE, CANNONICAL_VAL, VALUE) \
    assert_non_null((NODE).realtype->plugin->print(UTEST_LYCTX, &(NODE), LY_VALUE_CANON, NULL, NULL, NULL)); \
    assert_string_equal((NODE)._canonical, CANNONICAL_VAL); \
    assert_non_null((NODE).realtype); \
    assert_int_equal(LY_TYPE_INST, (NODE).realtype->basetype); \
    { \
        LY_ARRAY_COUNT_TYPE arr_size = sizeof(VALUE) / sizeof(VALUE[0]); \
        assert_int_equal(arr_size, LY_ARRAY_COUNT((NODE).target)); \
        for (LY_ARRAY_COUNT_TYPE it = 0; it < arr_size; it++) { \
            if ((NODE).target[it].predicates) { \
                assert_int_equal(VALUE[it], (NODE).target[it].predicates[0].type); \
            } \
        } \
    }

/**
 * @brief Internal macro. Assert that lyd_value structure members are correct. Lyd value is type ENUM.
 *        Example CHECK_LYD_VALUE(node->value, ENUM, "item_name", "item_name");
 *
 * @param[in] NODE           lyd_value variable
 * @param[in] CANNONICAL_VAL expected cannonical value
 * @param[in] VALUE          expected enum item name
 */
#define CHECK_LYD_VALUE_ENUM(NODE, CANNONICAL_VAL, VALUE) \
    assert_non_null((NODE).realtype->plugin->print(UTEST_LYCTX, &(NODE), LY_VALUE_CANON, NULL, NULL, NULL)); \
    assert_string_equal((NODE)._canonical, CANNONICAL_VAL); \
    assert_non_null((NODE).realtype); \
    assert_int_equal(LY_TYPE_ENUM, (NODE).realtype->basetype); \
    assert_string_equal(VALUE, (NODE).enum_item->name);

/**
 * @brief Internal macro. Assert that lyd_value structure members are correct. Lyd value is type INT8
 *        Example CHECK_LYD_VALUE(node->value, INT8, "12", 12);
 *
 * @param[in] NODE           lyd_value variable
 * @param[in] CANNONICAL_VAL expected cannonical value
 * @param[in] VALUE          expected inteager value (-128 to 127).
 */
#define CHECK_LYD_VALUE_INT8(NODE, CANNONICAL_VAL, VALUE) \
    assert_non_null((NODE).realtype->plugin->print(UTEST_LYCTX, &(NODE), LY_VALUE_CANON, NULL, NULL, NULL)); \
    assert_string_equal((NODE)._canonical, CANNONICAL_VAL); \
    assert_non_null((NODE).realtype); \
    assert_int_equal(LY_TYPE_INT8, (NODE).realtype->basetype); \
    assert_int_equal(VALUE, (NODE).int8);

/**
 * @brief Internal macro. Assert that lyd_value structure members are correct. Lyd value is type INT16
 *        Example CHECK_LYD_VALUE(node->value, INT8, "12", 12);
 *
 * @param[in] NODE           lyd_value variable
 * @param[in] CANNONICAL_VAL expected cannonical value
 * @param[in] VALUE          expected inteager value.
 */
#define CHECK_LYD_VALUE_INT16(NODE, CANNONICAL_VAL, VALUE) \
    assert_non_null((NODE).realtype->plugin->print(UTEST_LYCTX, &(NODE), LY_VALUE_CANON, NULL, NULL, NULL)); \
    assert_string_equal((NODE)._canonical, CANNONICAL_VAL); \
    assert_non_null((NODE).realtype); \
    assert_int_equal(LY_TYPE_INT16, (NODE).realtype->basetype); \
    assert_int_equal(VALUE, (NODE).int16);

/**
 * @brief Internal macro. Assert that lyd_value structure members are correct. Lyd value is type UINT8.
 *        Example CHECK_LYD_VALUE(node->value, UINT8, "12", 12);
 *
 * @param[in] NODE           lyd_value variable
 * @param[in] CANNONICAL_VAL expected cannonical value
 * @param[in] VALUE          expected inteager (0 to 255).
 */
#define CHECK_LYD_VALUE_UINT8(NODE, CANNONICAL_VAL, VALUE) \
    assert_non_null((NODE).realtype->plugin->print(UTEST_LYCTX, &(NODE), LY_VALUE_CANON, NULL, NULL, NULL)); \
    assert_string_equal((NODE)._canonical, CANNONICAL_VAL); \
    assert_non_null((NODE).realtype); \
    assert_int_equal(LY_TYPE_UINT8, (NODE).realtype->basetype); \
    assert_int_equal(VALUE, (NODE).uint8);

/**
 * @brief Internal macro. Assert that lyd_value structure members are correct. Lyd value is type UINT32.
 *        Example CHECK_LYD_VALUE(node->value, UINT32, "12", 12);
 *
 * @param[in] NODE           lyd_value variable
 * @param[in] CANNONICAL_VAL expected cannonical value
 * @param[in] VALUE          expected inteager (0 to MAX_UINT32).
 */
#define CHECK_LYD_VALUE_UINT32(NODE, CANNONICAL_VAL, VALUE) \
    assert_non_null((NODE).realtype->plugin->print(UTEST_LYCTX, &(NODE), LY_VALUE_CANON, NULL, NULL, NULL)); \
    assert_string_equal((NODE)._canonical, CANNONICAL_VAL); \
    assert_non_null((NODE).realtype); \
    assert_int_equal(LY_TYPE_UINT32, (NODE).realtype->basetype); \
    assert_int_equal(VALUE, (NODE).uint32);

/**
 * @brief Internal macro. Assert that lyd_value structure members are correct. Lyd value is type STRING.
 *        Example CHECK_LYD_VALUE(node->value, STRING, "text");
 *
 * @param[in] NODE           lyd_value variable
 * @param[in] CANNONICAL_VAL expected cannonical value
 */
#define CHECK_LYD_VALUE_STRING(NODE, CANNONICAL_VAL) \
    assert_non_null((NODE).realtype->plugin->print(UTEST_LYCTX, &(NODE), LY_VALUE_CANON, NULL, NULL, NULL)); \
    assert_string_equal((NODE)._canonical, CANNONICAL_VAL); \
    assert_non_null((NODE).realtype); \
    assert_int_equal(LY_TYPE_STRING, (NODE).realtype->basetype);

/**
 * @brief Internal macro. Assert that lyd_value structure members are correct. Lyd value is type LEAFREF
 *        Example CHECK_LYD_VALUE(node->value, LEAFREF, "");
 *
 * @param[in] NODE           lyd_value variable
 * @param[in] CANNONICAL_VAL expected cannonical value
 */
#define CHECK_LYD_VALUE_LEAFREF(NODE, CANNONICAL_VAL) \
    assert_non_null((NODE).realtype->plugin->print(UTEST_LYCTX, &(NODE), LY_VALUE_CANON, NULL, NULL, NULL)); \
    assert_string_equal((NODE)._canonical, CANNONICAL_VAL); \
    assert_non_null((NODE).realtype); \
    assert_int_equal(LY_TYPE_LEAFREF, (NODE).realtype->basetype); \
    assert_non_null((NODE).ptr)

/**
 * @brief Internal macro. Assert that lyd_value structure members are correct. Lyd value is type DEC64
 *        Example CHECK_LYD_VALUE(node->value, DEC64, "125", 125);
 *
 * @param[in] NODE           lyd_value variable
 * @param[in] CANNONICAL_VAL expected cannonical value
 * @param[in] VALUE          expected value 64bit inteager
*/
#define CHECK_LYD_VALUE_DEC64(NODE, CANNONICAL_VAL, VALUE) \
    assert_non_null((NODE).realtype->plugin->print(UTEST_LYCTX, &(NODE), LY_VALUE_CANON, NULL, NULL, NULL)); \
    assert_string_equal((NODE)._canonical, CANNONICAL_VAL); \
    assert_non_null((NODE).realtype); \
    assert_int_equal(LY_TYPE_DEC64, (NODE).realtype->basetype); \
    assert_int_equal(VALUE, (NODE).dec64);

/**
 * @brief Internal macro. Assert that lyd_value structure members are correct. Lyd value is type BINARY.
 *        Example CHECK_LYD_VALUE(node->value, BINARY, "aGVs\nbG8=");
 *
 * @param[in] NODE           lyd_value variable
 * @param[in] CANNONICAL_VAL expected cannonical value
 * @param[in] VALUE          expected value data
 * @param[in] SIZE           expected value data size
*/
#define CHECK_LYD_VALUE_BINARY(NODE, CANNONICAL_VAL, VALUE, SIZE) \
    { \
        struct lyd_value_binary *_val; \
        LYD_VALUE_GET(&(NODE), _val); \
        assert_int_equal(_val->size, SIZE); \
        assert_int_equal(0, memcmp(_val->data, VALUE, SIZE)); \
        assert_non_null((NODE).realtype->plugin->print(UTEST_LYCTX, &(NODE), LY_VALUE_CANON, NULL, NULL, NULL)); \
        assert_string_equal((NODE)._canonical, CANNONICAL_VAL); \
        assert_non_null((NODE).realtype); \
        assert_int_equal(LY_TYPE_BINARY, (NODE).realtype->basetype); \
    }

/**
 * @brief Internal macro. Assert that lyd_value structure members are correct. Lyd value is type BOOL.
 *        Example CHECK_LYD_VALUE(node->value, BOOL, "true", 1);
 *
 * @param[in] NODE           lyd_value variable
 * @param[in] CANNONICAL_VAL expected cannonical value
 * @param[in] VALUE          expected boolean value 0,1
*/
#define CHECK_LYD_VALUE_BOOL(NODE, CANNONICAL_VAL, VALUE) \
    assert_non_null((NODE).realtype->plugin->print(UTEST_LYCTX, &(NODE), LY_VALUE_CANON, NULL, NULL, NULL)); \
    assert_string_equal((NODE)._canonical, CANNONICAL_VAL); \
    assert_non_null((NODE).realtype); \
    assert_int_equal(LY_TYPE_BOOL, (NODE).realtype->basetype); \
    assert_int_equal(VALUE, (NODE).boolean);

/**
 * @brief Internal macro. Assert that lyd_value structure members are correct. Lyd value is type IDENT.
 *        Example CHECK_LYD_VALUE(node->value, IDENT, "types:gigabit-ethernet", "gigabit-ethernet");
 *
 * @param[in] NODE           lyd_value variable
 * @param[in] CANNONICAL_VAL expected cannonical value
 * @param[in] VALUE          expected ident name
*/
#define CHECK_LYD_VALUE_IDENT(NODE, CANNONICAL_VAL, VALUE) \
    assert_non_null((NODE).realtype->plugin->print(UTEST_LYCTX, &(NODE), LY_VALUE_CANON, NULL, NULL, NULL)); \
    assert_string_equal((NODE)._canonical, CANNONICAL_VAL); \
    assert_non_null((NODE).realtype); \
    assert_int_equal(LY_TYPE_IDENT, (NODE).realtype->basetype); \
    assert_string_equal(VALUE, (NODE).ident->name);

/**
 * @brief Macro testing parser when parsing incorrect module;
 *
 * @param[in] DATA     String storing the schema module representation.
 * @param[in] FORMAT   Schema format of the @p DATA
 * @param[in] FEATURES Array of module's features to enable
 * @param[in] RET_VAL  ly_in_new_memory return error value
 */
#define UTEST_INVALID_MODULE(DATA, FORMAT, FEATURES, RET_VAL) \
    { \
        struct lys_module *mod; \
        assert_int_equal(LY_SUCCESS, ly_in_new_memory(DATA, &_UC->in)); \
        assert_int_equal(RET_VAL, lys_parse(_UC->ctx, _UC->in, FORMAT, FEATURES, &mod)); \
        assert_null(mod); \
    } \
    ly_in_free(_UC->in, 0); \
    _UC->in = NULL; \

/**
 * @brief Add module (from a string) into the used libyang context.
 *
 * @param[in] DATA     String storing the schema module representation.
 * @param[in] FORMAT   Schema format of the @p DATA
 * @param[in] FEATURES Array of module's features to enable
 * @param[out] MOD     Optional parameter as a pointer to variable to store the resulting module.
 */
#define UTEST_ADD_MODULE(DATA, FORMAT, FEATURES, MOD) \
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(DATA, &_UC->in)); \
    { \
        LY_ERR __r = lys_parse(_UC->ctx, _UC->in, FORMAT, FEATURES, MOD); \
        if (__r != LY_SUCCESS) { \
            print_message("[  MSG     ] Module parsing failed:\n"); \
            for (struct ly_err_item *e = ly_err_first(_UC->ctx); e; e = e->next) { \
                print_message("[  MSG     ] \t%s Path %s\n", e->msg, e->path); \
            } \
            fail(); \
        } \
    } \
    ly_in_free(_UC->in, 0); \
    _UC->in = NULL

/**
 * @brief Check expected last error message.
 *
 * @param[in] MSG Expected error message.
 */
#define CHECK_LOG_LASTMSG(MSG) \
    CHECK_STRING(ly_last_errmsg(), MSG)

/**
 * @brief Check expected last error in libyang context, which is then cleared. Can be called repeatedly to check
 * several errors. If NULL is provided as MSG, no error info record (NULL) is expected.
 *
 * @param[in] MSG Expected error message.
 * @param[in] PATH Expected error path.
 */
#define CHECK_LOG_CTX(MSG, PATH) \
    { \
        struct ly_err_item *_e = ly_err_last(_UC->ctx); \
        if (!MSG) { \
            assert_null(_e); \
        } else { \
            assert_non_null(_e); \
            CHECK_STRING(_e->msg, MSG); \
            CHECK_STRING(_e->path, PATH); \
        } \
        ly_err_clean(_UC->ctx, _e); \
    }

/**
 * @brief Check expected error in libyang context including error-app-tag.
 *
 * @param[in] MSG Expected error message.
 * @param[in] PATH Expected error path.
 * @param[in] APPTAG Expected error-app-tag.
 */
#define CHECK_LOG_CTX_APPTAG(MSG, PATH, APPTAG) \
    { \
        struct ly_err_item *_e = ly_err_last(_UC->ctx); \
        if (!MSG) { \
            assert_null(_e); \
        } else { \
            assert_non_null(_e); \
            CHECK_STRING(_e->msg, MSG); \
            CHECK_STRING(_e->path, PATH); \
            CHECK_STRING(_e->apptag, APPTAG); \
        } \
        ly_err_clean(_UC->ctx, _e); \
    }

/**
 * @brief Clear all errors stored in the libyang context.
 */
#define UTEST_LOG_CTX_CLEAN \
    ly_err_clean(_UC->ctx, NULL)

/**
 * @brief Clean up the logging callback's storage.
 */
#define UTEST_LOG_CLEAN \
    free(_UC->err_msg); \
    free(_UC->err_path); \
    _UC->err_msg = NULL; \
    _UC->err_path = NULL;

/**
 * @brief Check expected error directly logged via logging callback.
 * Useful mainly for messages logged by functions without access to libyang context.
 * @param[in] MSG Expected error message.
 * @param[in] PATH Expected error path.
 */
#define CHECK_LOG(MSG, PATH) \
    CHECK_STRING(_UC->err_msg, MSG); \
    CHECK_STRING(_UC->err_path, PATH); \
    UTEST_LOG_CLEAN

#ifdef _UTEST_MAIN_
/*
 * Functions inlined into each C source file including this header with _UTEST_MAIN_ defined
 */

/**
 * @brief Global variable holding the tests context to simplify access to it.
 */
struct utest_context *current_utest_context;

/* set to 0 to printing error messages to stderr instead of checking them in code */
#define ENABLE_LOGGER_CHECKING 1

/**
 * @brief Logging callback for libyang.
 */
static void
_utest_logger(LY_LOG_LEVEL level, const char *msg, const char *path)
{
    (void) level; /* unused */

    if (ENABLE_LOGGER_CHECKING == 0) {
        printf("\tERROR:\n\t\tMESSAGE: %s\n\t\tPATH: %s\n\t\tLEVEL: %i\n", msg, path, level);
    } else {
        free(current_utest_context->err_msg);
        current_utest_context->err_msg = msg ? strdup(msg) : NULL;
        free(current_utest_context->err_path);
        current_utest_context->err_path = path ? strdup(path) : NULL;
    }
}

/**
 * @brief Generic utest's setup
 */
static int
utest_setup(void **state)
{
    char *cur_tz;

    /* setup the logger */
    ly_set_log_clb(_utest_logger, 1);
    ly_log_options(LY_LOLOG | LY_LOSTORE);

    current_utest_context = calloc(1, sizeof *current_utest_context);
    assert_non_null(current_utest_context);
    *state = current_utest_context;

    /* libyang context */
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &current_utest_context->ctx));

    /* clean all errors from the setup - usually warnings regarding the plugins directories */
    UTEST_LOG_CLEAN;

    /* backup timezone, if any */
    cur_tz = getenv("TZ");
    if (cur_tz) {
        current_utest_context->orig_tz = strdup(cur_tz);
    }

    /* set CET */
    setenv("TZ", "CET+02:00", 1);

    return 0;
}

/**
 * @brief macro to include generic utest's setup into the test-specific setup.
 *
 * Place at the beginning of the test-specific setup
 */
#define UTEST_SETUP \
    assert_int_equal(0, utest_setup(state))

/**
 * @brief Generic utest's teardown
 */
static int
utest_teardown(void **state)
{
    *state = NULL;

    /* libyang context, no leftover messages */
    assert_null(ly_err_last(current_utest_context->ctx));
    ly_ctx_destroy(current_utest_context->ctx);

    if (current_utest_context->orig_tz) {
        /* restore TZ */
        setenv("TZ", current_utest_context->orig_tz, 1);
    }

    /* utest context */
    ly_in_free(current_utest_context->in, 0);
    free(current_utest_context->err_msg);
    free(current_utest_context->err_path);
    free(current_utest_context->orig_tz);
    free(current_utest_context);
    current_utest_context = NULL;

    return 0;
}

/**
 * @brief macro to include generic utest's teardown into the test-specific teardown.
 *
 * Place at the end of the test-specific teardown
 */
#define UTEST_TEARDOWN \
    assert_int_equal(0, utest_teardown(state))

/**
 * @brief Internal macro for utest setup with test-specific setup and teardown
 */
#define _UTEST_SETUP_TEARDOWN(FUNC, SETUP, TEARDOWN) \
    cmocka_unit_test_setup_teardown(FUNC, SETUP, TEARDOWN)

/**
 * @brief Internal macro for utest setup with test-specific setup and generic teardown
 */
#define _UTEST_SETUP(FUNC, SETUP) \
    cmocka_unit_test_setup_teardown(FUNC, SETUP, utest_teardown)

/**
 * @brief Internal macro for utest setup with generic setup and teardown
 */
#define _UTEST(FUNC) \
    cmocka_unit_test_setup_teardown(FUNC, utest_setup, utest_teardown)

/**
 * @brief Internal helper macro to select _UTEST* macro according to the provided parameters.
 */
#define _GET_UTEST_MACRO(_1, _2, _3, NAME, ...) NAME

/**
 * @brief Macro to specify test function using utest environment. Macro has variadic parameters
 * to provide test-specific setup/teardown functions:
 *
 * UTEST(test_func) - only implicit setup and teardown functions are used
 * UTEST(test_func, setup) - implicit teardown but own setup
 * UTEST(test_func, setup, teardown) - both setup and teardown are test-specific
 */
#define UTEST(...) \
    _GET_UTEST_MACRO(__VA_ARGS__, _UTEST_SETUP_TEARDOWN, _UTEST_SETUP, _UTEST, DUMMY)(__VA_ARGS__)

#else /* _UTEST_MAIN_ */

extern struct utest_context *current_utest_context;

#endif /* _UTEST_MAIN_ */

#endif /* _UTESTS_H_ */
