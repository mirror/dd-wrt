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

#define TEST_SUCCESS_LYB(MOD_NAME, NODE_NAME, DATA) \
    { \
        struct lyd_node *tree_1; \
        struct lyd_node *tree_2; \
        char *xml_out, *data; \
        data = "<" NODE_NAME " xmlns=\"urn:tests:" MOD_NAME "\">" DATA "</" NODE_NAME ">"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, LYD_PARSE_ONLY | LYD_PARSE_STRICT, 0, LY_SUCCESS, tree_1); \
        assert_int_equal(lyd_print_mem(&xml_out, tree_1, LYD_LYB, LYD_PRINT_WITHSIBLINGS), 0); \
        assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, xml_out, LYD_LYB, LYD_PARSE_ONLY | LYD_PARSE_STRICT, 0, &tree_2)); \
        assert_non_null(tree_2); \
        CHECK_LYD(tree_1, tree_2); \
        free(xml_out); \
        lyd_free_all(tree_1); \
        lyd_free_all(tree_2); \
    }

static void
test_plugin_store(void **state)
{
    const char *val, *dec_val;
    unsigned char bin_val[2];
    struct ly_err_item *err = NULL;
    struct lys_module *mod;
    struct lyd_value value = {0};
    struct lyplg_type *type = lyplg_type_plugin_find(NULL, "", NULL, ly_data_type2str[LY_TYPE_BINARY]);
    struct lysc_type *lysc_type, *lysc_type2;
    LY_ERR ly_ret;
    const char *schema;

    /* create schema. Prepare common used variables */
    schema = MODULE_CREATE_YANG("a", "leaf l {type binary;} leaf k {type binary {length 4..8;}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    lysc_type2 = ((struct lysc_node_leaf *)mod->compiled->data->next)->type;

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

    /* newlines after every 64 chars */
    val = "MIIEAzCCAuugAwIBAgIURc4sipHvJSlNrQIhRhZilBvV4RowDQYJKoZIhvcNAQEL\n"
            "BQAwgZAxCzAJBgNVBAYTAkNaMRYwFAYDVQQIDA1Tb3V0aCBNb3JhdmlhMQ0wCwYD\n"
            "VQQHDARCcm5vMRgwFgYDVQQKDA9DRVNORVQgei5zLnAuby4xDDAKBgNVBAsMA1RN\n"
            "QzETMBEGA1UEAwwKZXhhbXBsZSBDQTEdMBsGCSqGSIb3DQEJARYOY2FAZXhhbXBs\n"
            "ZS5vcmcwHhcNMjEwOTAzMTAyMTAxWhcNMzEwOTAxMTAyMTAxWjCBkDELMAkGA1UE\n"
            "BhMCQ1oxFjAUBgNVBAgMDVNvdXRoIE1vcmF2aWExDTALBgNVBAcMBEJybm8xGDAW\n"
            "BgNVBAoMD0NFU05FVCB6LnMucC5vLjEMMAoGA1UECwwDVE1DMRMwEQYDVQQDDApl\n"
            "eGFtcGxlIENBMR0wGwYJKoZIhvcNAQkBFg5jYUBleGFtcGxlLm9yZzCCASIwDQYJ\n"
            "KoZIhvcNAQEBBQADggEPADCCAQoCggEBAN4Ld3JDDocyy9KXNJhEUPeZpQW3UdUN\n"
            "Xloeh5n/bxasgThkBuQ7oF/nKyVUe517U1CJA993ZIc0jhIWThAnqXkz70DX5EZ7\n"
            "ancPd01MidA6T8k1RYYJWr+vyIRYYBYzK7LSnU6wMWqPTgzZB+KMWwb065ooLEB5\n"
            "XwqAeTIMPLRqM1Galewl4ZSuRJnrXxRjfF3AWNyC9dZw6wIg8cppvoLdBGQiFJQf\n"
            "9SgiVy+HyedAytFEixqKAAIgQUJwhCgbEd6jGFbeaL8HT4MFp1VmaaUBQMkZj/Gn\n"
            "KBwCk5BEMu76EN1pzHc4Dd6DabQNGCnsqOPe31yhQGmNFy9R6zNnWZMCAwEAAaNT\n"
            "MFEwHQYDVR0OBBYEFM7w/pO8vk5oowvWPoCKo0RW/JcnMB8GA1UdIwQYMBaAFM7w\n"
            "/pO8vk5oowvWPoCKo0RW/JcnMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEL\n"
            "BQADggEBAG/xfYuRKnCyiwYC/K7kAjHmCNnLCr1mx8P1ECsSJPme3OThDTNeGf8i\n"
            "N2952tGmMFDa+DaAwPc6Gt3cWTb/NYMTLWlt2yj5rJAcLXxIU0SMafBf+F7E/R8A\n"
            "b/HDDjs0pQaJ0EJhQJVkMdfj3Wq9l0dJT5iEBUrUQflDufiMdEJEIGKZh86MgzEL\n"
            "bcn1QX8dlLc91M2OifWStqLzXPicG+jjuoPUceC0flMQDb2qx03sxvJKfYfS5ArA\n"
            "CqvdWyXLoP7DI9THJrMI/vBHJKpl4Wtmsh2OLn9VHauFMzPSGke5GwjXCpbXGepj\n"
            "9qWN8Gd/FWgSDH2OBvZ6aHdB1pPjN9k=";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, val, strlen(val),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err));
    assert_ptr_equal(value.realtype, lysc_type);
    type->free(UTEST_LYCTX, &value);

    /* empty value */
    val = "";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, val, strlen(val),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err));
    CHECK_LYD_VALUE(value, BINARY, "", "", 0);
    assert_ptr_equal(value.realtype, lysc_type);
    type->free(UTEST_LYCTX, &value);

    /* short value */
    val = "YQ==";
    dec_val = "a";
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

    /* length check */
    val = "Zm91cg==";
    dec_val = "four";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type2, val, strlen(val),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err));
    CHECK_LYD_VALUE(value, BINARY, val, dec_val, strlen(dec_val));
    assert_ptr_equal(value.realtype, lysc_type2);
    type->free(UTEST_LYCTX, &value);

    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type2, dec_val, strlen(dec_val),
            0, LY_VALUE_LYB, NULL, 0, NULL, &value, NULL, &err));
    CHECK_LYD_VALUE(value, BINARY, val, dec_val, strlen(dec_val));
    assert_ptr_equal(value.realtype, lysc_type2);
    type->free(UTEST_LYCTX, &value);

    val = "ZWlnaHQwMTI=";
    dec_val = "eight012";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type2, val, strlen(val),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err));
    CHECK_LYD_VALUE(value, BINARY, val, dec_val, strlen(dec_val));
    assert_ptr_equal(value.realtype, lysc_type2);
    type->free(UTEST_LYCTX, &value);

    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type2, dec_val, strlen(dec_val),
            0, LY_VALUE_LYB, NULL, 0, NULL, &value, NULL, &err));
    CHECK_LYD_VALUE(value, BINARY, val, dec_val, strlen(dec_val));
    assert_ptr_equal(value.realtype, lysc_type2);
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

    val = "MTIz";
    err = NULL;
    ly_ret = type->store(UTEST_LYCTX, lysc_type2, val, strlen(val),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err);
    assert_int_equal(LY_EVALID, ly_ret);
    assert_string_equal(err->msg, "Unsatisfied length - string \"MTIz\" length is not allowed.");
    ly_err_free(err);

    /* LYPLG_TYPE_STORE_ONLY test */
    val = "MTIz";
    err = NULL;
    ly_ret = type->store(UTEST_LYCTX, lysc_type2, val, strlen(val),
            LYPLG_TYPE_STORE_ONLY, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err);
    assert_int_equal(LY_SUCCESS, ly_ret);
    type->free(UTEST_LYCTX, &value);
    ly_err_free(err);
}

static void
test_plugin_print(void **state)
{
    const char *schema, *val;
    struct lyd_value value = {0};
    struct lys_module *mod;
    struct lysc_type *lysc_type;
    struct lyplg_type *type = lyplg_type_plugin_find(NULL, "", NULL, ly_data_type2str[LY_TYPE_BINARY]);
    struct ly_err_item *err = NULL;

    /* create schema. Prepare common used variables */
    schema = MODULE_CREATE_YANG("a", "leaf l {type binary;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_type = ((struct lysc_node_leaf *)mod->compiled->data)->type;

    /* Testing empty value. */
    val = "";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, val, strlen(val),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err));
    assert_string_equal("", value.realtype->plugin->print(UTEST_LYCTX, &(value), LY_VALUE_CANON, NULL, NULL, NULL));
    type->free(UTEST_LYCTX, &value);
}

static void
test_plugin_duplicate(void **state)
{
    const char *schema, *val;
    struct lyd_value value = {0}, dup;
    struct lys_module *mod;
    struct lysc_type *lysc_type;
    struct lyplg_type *type = lyplg_type_plugin_find(NULL, "", NULL, ly_data_type2str[LY_TYPE_BINARY]);
    struct ly_err_item *err = NULL;

    /* create schema. Prepare common used variables */
    schema = MODULE_CREATE_YANG("a", "leaf l {type binary;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_type = ((struct lysc_node_leaf *)mod->compiled->data)->type;

    /* Testing empty value. */
    val = "";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, val, strlen(val),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err));
    assert_int_equal(LY_SUCCESS, type->duplicate(UTEST_LYCTX, &value, &dup));
    CHECK_LYD_VALUE(dup, BINARY, "", "", 0);
    type->free(UTEST_LYCTX, &value);
    type->free(UTEST_LYCTX, &dup);
}

static void
test_plugin_sort(void **state)
{
    const char *v1, *v2;
    const char *schema;
    struct lys_module *mod;
    struct lyd_value val1 = {0}, val2 = {0};
    struct lyplg_type *type = lyplg_type_plugin_find(NULL, "", NULL, ly_data_type2str[LY_TYPE_BINARY]);
    struct lysc_type *lysc_type;
    struct ly_err_item *err = NULL;

    /* create schema. Prepare common used variables */
    schema = MODULE_CREATE_YANG("a", "leaf-list ll {type binary;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_type = ((struct lysc_node_leaflist *)mod->compiled->data)->type;

    /* v1 < v2, v2 > v1, v1 == v1 */
    v1 = "YWhveQ=="; /* ahoy */
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v1, strlen(v1),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &val1, NULL, &err));
    v2 = "YWhveg=="; /* ahoz */
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v2, strlen(v2),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &val2, NULL, &err));
    assert_true(0 > type->sort(UTEST_LYCTX, &val1, &val2));
    assert_int_equal(0, type->sort(UTEST_LYCTX, &val1, &val1));
    assert_true(0 < type->sort(UTEST_LYCTX, &val2, &val1));
    type->free(UTEST_LYCTX, &val1);
    type->free(UTEST_LYCTX, &val2);

    /* v2 is shorter */
    v1 = "YWhveQ=="; /* ahoj */
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v1, strlen(v1),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &val1, NULL, &err));
    v2 = "YWhv"; /* aho */
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v2, strlen(v2),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &val2, NULL, &err));
    assert_int_equal(1, type->sort(UTEST_LYCTX, &val1, &val2));
    assert_int_equal(-1, type->sort(UTEST_LYCTX, &val2, &val1));
    type->free(UTEST_LYCTX, &val1);
    type->free(UTEST_LYCTX, &val2);
}

static void
test_data_lyb(void **state)
{
    const char *schema;

    schema = MODULE_CREATE_YANG("lyb", "leaf port {type binary;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    TEST_SUCCESS_LYB("lyb", "port", "");
    TEST_SUCCESS_LYB("lyb", "port", "YWhveQ==");
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_plugin_store),
        UTEST(test_plugin_print),
        UTEST(test_plugin_duplicate),
        UTEST(test_plugin_sort),
        UTEST(test_data_lyb),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
