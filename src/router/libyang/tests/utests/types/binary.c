/**
 * @file binary.c
 * @author Michal Vaško <mvasko@cesnet.cz>
 * @brief test for built-in binary type
 *
 * Copyright (c) 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

/* INCLUDE UTEST HEADER */
#define  _UTEST_MAIN_
#include "../utests.h"

/* LOCAL INCLUDE HEADERS */
#include "libyang.h"

#define MODULE_CREATE_YANG(MOD_NAME, NODES) \
    "module " MOD_NAME " {\n" \
    "  yang-version 1.1;\n" \
    "  namespace \"urn:tests:" MOD_NAME "\";\n" \
    "  prefix pref;\n" \
    NODES \
    "}\n"

static void
test_plugin_store(void **state)
{
    const char *val, *dec_val;
    char bin_val[2];
    struct ly_err_item *err = NULL;
    const struct lys_module *mod;
    struct lyd_value value = {0};
    struct lyplg_type *type = lyplg_find(LYPLG_TYPE, "", NULL, ly_data_type2str[LY_TYPE_BINARY]);
    struct lysc_type *lysc_type;
    LY_ERR ly_ret;
    const char *schema;

    /* create schema. Prepare common used variables */
    schema = MODULE_CREATE_YANG("a", "leaf l {type binary;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_type = ((struct lysc_node_leaf *)mod->compiled->data)->type;

    /* check proper type */
    assert_string_equal("libyang 2 - binary, version 1", type->id);

    /* check store XML double pad */
    val = "YWhveQ==";
    dec_val = "ahoy";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, val, strlen(val),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err));
    CHECK_LYD_VALUE(value, BINARY, val, dec_val, strlen(dec_val));
    assert_ptr_equal(value.realtype, lysc_type);
    type->free(UTEST_LYCTX, &value);

    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, dec_val, strlen(dec_val),
            0, LY_VALUE_LYB, NULL, 0, NULL, &value, NULL, &err));
    CHECK_LYD_VALUE(value, BINARY, val, dec_val, strlen(dec_val));
    assert_ptr_equal(value.realtype, lysc_type);
    type->free(UTEST_LYCTX, &value);

    /* single pad */
    val = "YWhveWo=";
    dec_val = "ahoyj";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, val, strlen(val),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err));
    CHECK_LYD_VALUE(value, BINARY, val, dec_val, strlen(dec_val));
    assert_ptr_equal(value.realtype, lysc_type);
    type->free(UTEST_LYCTX, &value);

    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, dec_val, strlen(dec_val),
            0, LY_VALUE_LYB, NULL, 0, NULL, &value, NULL, &err));
    CHECK_LYD_VALUE(value, BINARY, val, dec_val, strlen(dec_val));
    assert_ptr_equal(value.realtype, lysc_type);
    type->free(UTEST_LYCTX, &value);

    /* no pad */
    val = "YWhveWoy";
    dec_val = "ahoyj2";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, val, strlen(val),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err));
    CHECK_LYD_VALUE(value, BINARY, val, dec_val, strlen(dec_val));
    assert_ptr_equal(value.realtype, lysc_type);
    type->free(UTEST_LYCTX, &value);

    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, dec_val, strlen(dec_val),
            0, LY_VALUE_LYB, NULL, 0, NULL, &value, NULL, &err));
    CHECK_LYD_VALUE(value, BINARY, val, dec_val, strlen(dec_val));
    assert_ptr_equal(value.realtype, lysc_type);
    type->free(UTEST_LYCTX, &value);

    /* binary data */
    val = "q80=";
    bin_val[0] = 0xab;
    bin_val[1] = 0xcd;
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, val, strlen(val),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err));
    CHECK_LYD_VALUE(value, BINARY, val, bin_val, 2);
    assert_ptr_equal(value.realtype, lysc_type);
    type->free(UTEST_LYCTX, &value);

    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, bin_val, 2,
            0, LY_VALUE_LYB, NULL, 0, NULL, &value, NULL, &err));
    CHECK_LYD_VALUE(value, BINARY, val, bin_val, 2);
    assert_ptr_equal(value.realtype, lysc_type);
    type->free(UTEST_LYCTX, &value);

    /*
     * ERROR TESTS
     */
    val = "q80.";
    err = NULL;
    ly_ret = type->store(UTEST_LYCTX, lysc_type, val, strlen(val),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err);
    assert_int_equal(LY_EVALID, ly_ret);
    assert_string_equal(err->msg, "Invalid Base64 character '.'.");
    ly_err_free(err);

    val = "q80";
    err = NULL;
    ly_ret = type->store(UTEST_LYCTX, lysc_type, val, strlen(val),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err);
    assert_int_equal(LY_EVALID, ly_ret);
    assert_string_equal(err->msg, "Base64 encoded value length must be divisible by 4.");
    ly_err_free(err);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_plugin_store)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
