/**
 * @file test_xpath.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Cmocka tests for XPath expression evaluation.
 *
 * Copyright (c) 2016 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <cmocka.h>

#include "tests/config.h"
#include "libyang.h"

struct state {
    struct ly_ctx *ctx;
    struct lyd_node *dt;
    struct ly_set *set;
};

static const char *data =
"<interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\" xmlns:ianaift=\"urn:ietf:params:xml:ns:yang:iana-if-type\""
    " xmlns:ip=\"urn:ietf:params:xml:ns:yang:ietf-ip\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\">"
  "<interface>"
    "<name>iface1</name>"
    "<description>iface1 dsc</description>"
    "<type>ianaift:ethernetCsmacd</type>"
    "<enabled>true</enabled>"
    "<link-up-down-trap-enable>disabled</link-up-down-trap-enable>"
    "<ip:ipv4>"
      "<ip:enabled>true</ip:enabled>"
      "<ip:forwarding>true</ip:forwarding>"
      "<ip:mtu>68</ip:mtu>"
      "<ip:address>"
        "<ip:ip>10.0.0.1</ip:ip>"
        "<ip:netmask>255.0.0.0</ip:netmask>"
      "</ip:address>"
      "<ip:address>"
        "<ip:ip>172.0.0.1</ip:ip>"
        "<ip:prefix-length>16</ip:prefix-length>"
      "</ip:address>"
      "<ip:neighbor>"
        "<ip:ip>10.0.0.2</ip:ip>"
        "<ip:link-layer-address>01:34:56:78:9a:bc:de:f0</ip:link-layer-address>"
      "</ip:neighbor>"
    "</ip:ipv4>"
    "<ip:ipv6>"
      "<ip:enabled>true</ip:enabled>"
      "<ip:forwarding>false</ip:forwarding>"
      "<ip:mtu>1280</ip:mtu>"
      "<ip:address>"
        "<ip:ip>2001:abcd:ef01:2345:6789:0:1:1</ip:ip>"
        "<ip:prefix-length>64</ip:prefix-length>"
      "</ip:address>"
      "<ip:neighbor>"
        "<ip:ip>2001:abcd:ef01:2345:6789:0:1:2</ip:ip>"
        "<ip:link-layer-address>01:34:56:78:9a:bc:de:f0</ip:link-layer-address>"
      "</ip:neighbor>"
      "<ip:dup-addr-detect-transmits>52</ip:dup-addr-detect-transmits>"
      "<ip:autoconf>"
        "<ip:create-global-addresses>true</ip:create-global-addresses>"
        "<ip:create-temporary-addresses>false</ip:create-temporary-addresses>"
        "<ip:temporary-valid-lifetime>600</ip:temporary-valid-lifetime>"
        "<ip:temporary-preferred-lifetime>300</ip:temporary-preferred-lifetime>"
      "</ip:autoconf>"
    "</ip:ipv6>"
  "</interface>"
  "<interface>"
    "<name>iface2</name>"
    "<description>iface2 dsc</description>"
    "<type>ianaift:softwareLoopback</type>"
    "<enabled>false</enabled>"
    "<link-up-down-trap-enable>disabled</link-up-down-trap-enable>"
    "<ip:ipv4>"
      "<ip:address>"
        "<ip:ip>10.0.0.5</ip:ip>"
        "<ip:netmask>255.0.0.0</ip:netmask>"
      "</ip:address>"
      "<ip:address>"
        "<ip:ip>172.0.0.5</ip:ip>"
        "<ip:prefix-length>16</ip:prefix-length>"
      "</ip:address>"
      "<ip:neighbor>"
        "<ip:ip>10.0.0.1</ip:ip>"
        "<ip:link-layer-address>01:34:56:78:9a:bc:de:fa</ip:link-layer-address>"
      "</ip:neighbor>"
    "</ip:ipv4>"
    "<ip:ipv6>"
      "<ip:address>"
        "<ip:ip>2001:abcd:ef01:2345:6789:0:1:5</ip:ip>"
        "<ip:prefix-length>64</ip:prefix-length>"
      "</ip:address>"
      "<ip:neighbor>"
        "<ip:ip>2001:abcd:ef01:2345:6789:0:1:1</ip:ip>"
        "<ip:link-layer-address>01:34:56:78:9a:bc:de:fa</ip:link-layer-address>"
      "</ip:neighbor>"
      "<ip:dup-addr-detect-transmits>100</ip:dup-addr-detect-transmits>"
      "<ip:autoconf>"
        "<ip:create-global-addresses>true</ip:create-global-addresses>"
        "<ip:create-temporary-addresses>false</ip:create-temporary-addresses>"
        "<ip:temporary-valid-lifetime>600</ip:temporary-valid-lifetime>"
        "<ip:temporary-preferred-lifetime>300</ip:temporary-preferred-lifetime>"
      "</ip:autoconf>"
    "</ip:ipv6>"
  "</interface>"
"</interfaces>"
;

static int
setup_f(void **state)
{
    struct state *st;
    const char *augschema = "ietf-ip";
    const char *typeschema = "iana-if-type";
    const char *ietfdir = TESTS_DIR"/schema/yin/ietf/";
    const struct lys_module *mod;

    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error.\n");
        return -1;
    }

    /* libyang context */
    st->ctx = ly_ctx_new(ietfdir, 0);
    if (!st->ctx) {
        fprintf(stderr, "Failed to create context.\n");
        goto error;
    }

    /* schema */
    mod = ly_ctx_load_module(st->ctx, augschema, NULL);
    if (!mod) {
        fprintf(stderr, "Failed to load data module \"%s\".\n", augschema);
        goto error;
    }
    lys_features_enable(mod, "*");

    mod = ly_ctx_get_module(st->ctx, "ietf-interfaces", NULL, 0);
    if (!mod) {
        fprintf(stderr, "Failed to get data module \"ietf-interfaces\".\n");
        goto error;
    }
    lys_features_enable(mod, "*");

    mod = ly_ctx_load_module(st->ctx, typeschema, NULL);
    if (!mod) {
        fprintf(stderr, "Failed to load data module \"%s\".\n", typeschema);
        goto error;
    }

    /* data */
    st->dt = lyd_parse_mem(st->ctx, data, LYD_XML, LYD_OPT_CONFIG);
    if (!st->dt) {
        fprintf(stderr, "Failed to build the data tree.\n");
        goto error;
    }

    return 0;

error:
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return -1;
}

static int
teardown_f(void **state)
{
    struct state *st = (*state);

    lyd_free_withsiblings(st->dt);
    ly_set_free(st->set);
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_invalid(void **state)
{
    struct state *st = (*state);

    st->set = lyd_find_path(st->dt, "/:interface/name");
    assert_ptr_equal(st->set, NULL);
    assert_int_not_equal(ly_errno, 0);

    st->set = lyd_find_path(st->dt, "/interface/name[./]");
    assert_ptr_equal(st->set, NULL);
    assert_int_not_equal(ly_errno, 0);

    st->set = lyd_find_path(st->dt, "/interface/name[./.]()");
    assert_ptr_equal(st->set, NULL);
    assert_int_not_equal(ly_errno, 0);
}

static void
test_simple(void **state)
{
    struct state *st = (*state);

    st->set = lyd_find_path(st->dt, "/ietf-interfaces:interfaces");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 1);
    ly_set_free(st->set);
    st->set = NULL;

    st->set = lyd_find_path(st->dt, "/ietf-interfaces:interfaces/interface");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 2);
    ly_set_free(st->set);
    st->set = NULL;

    st->set = lyd_find_path(st->dt, "/ietf-interfaces:interfaces/interface[name='iface1']");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 1);
    ly_set_free(st->set);
    st->set = NULL;

    st->set = lyd_find_path(st->dt, "/ietf-interfaces:interfaces/interface[name='iface1']/ietf-ip:ipv4/ietf-ip:address");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 2);
    ly_set_free(st->set);
    st->set = NULL;

    st->set = lyd_find_path(st->dt, "/ietf-interfaces:interfaces/interface[name='iface1']/ietf-ip:ipv4/ietf-ip:address[ietf-ip:ip='10.0.0.1']");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 1);
    ly_set_free(st->set);
    st->set = NULL;
}

static void
test_advanced(void **state)
{
    struct state *st = (*state);

    st->set = lyd_find_path(st->dt, "/ietf-interfaces:interfaces/interface[name='iface1']/ietf-ip:ipv4/*[ietf-ip:ip]");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 3);
    ly_set_free(st->set);
    st->set = NULL;

    st->set = lyd_find_path(st->dt, "/ietf-interfaces:interfaces//*[ietf-ip:ip]");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 10);
    ly_set_free(st->set);
    st->set = NULL;

    st->set = lyd_find_path(st->dt, "/ietf-interfaces:interfaces//*[ietf-ip:ip[.='10.0.0.1']]");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 2);
    ly_set_free(st->set);
    st->set = NULL;

    st->set = lyd_find_path(st->dt, "//ietf-ip:ip[.='10.0.0.1']");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 2);
    ly_set_free(st->set);
    st->set = NULL;

    st->set = lyd_find_path(st->dt, "//*[../description]");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 14);
    ly_set_free(st->set);
    st->set = NULL;

    st->set = lyd_find_path(st->dt, "//interface[name='iface1']/ietf-ip:ipv4//*");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 12);
    ly_set_free(st->set);
    st->set = NULL;
}

static void
test_functions_operators(void **state)
{
    struct state *st = (*state);

    st->set = lyd_find_path(st->dt, "/ietf-interfaces:interfaces/interface/name[true() and not(false()) and not(boolean(. != 'iface1'))]");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 1);
    assert_string_equal(((struct lyd_node_leaf_list *)st->set->set.d[0])->value_str, "iface1");
    ly_set_free(st->set);
    st->set = NULL;

    st->set = lyd_find_path(st->dt, "(/ietf-interfaces:interfaces/interface/name)[round(ceiling(1.8)+0.4)+floor(0.28)]");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 1);
    assert_string_equal(((struct lyd_node_leaf_list *)st->set->set.d[0])->value_str, "iface2");
    ly_set_free(st->set);
    st->set = NULL;

    st->set = lyd_find_path(st->dt, "/ietf-interfaces:interfaces/interface/type[string-length(substring-after(\"hello12hi\", '12')) != 2 or starts-with(.,'iana') and contains(.,'back') and .=substring-before(concat(string(.),'aab', \"abb\"),'aa')]");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 1);
    assert_string_equal(((struct lyd_node_leaf_list *)st->set->set.d[0])->value_str, "iana-if-type:softwareLoopback");
    ly_set_free(st->set);
    st->set = NULL;

    st->set = lyd_find_path(st->dt, "//*[ietf-ip:neighbor/ietf-ip:link-layer-address = translate(normalize-space('\t\n01   .34    .56\t.78.9a\n\r.bc.de.f0  \t'), '. ', ':')]");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 2);
    ly_set_free(st->set);
    st->set = NULL;

    st->set = lyd_find_path(st->dt, "(//ietf-ip:ip)[position() = last()]");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 1);
    assert_string_equal(((struct lyd_node_leaf_list *)st->set->set.d[0])->value_str, "2001:abcd:ef01:2345:6789:0:1:1");
    ly_set_free(st->set);
    st->set = NULL;

    st->set = lyd_find_path(st->dt, "(//ietf-ip:ip)[count(//*[.='52'])]");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 1);
    assert_string_equal(((struct lyd_node_leaf_list *)st->set->set.d[0])->value_str, "10.0.0.1");
    ly_set_free(st->set);
    st->set = NULL;

    st->set = lyd_find_path(st->dt, "//*[local-name()='autoconf' and namespace-uri()='urn:ietf:params:xml:ns:yang:ietf-ip']");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 2);
    ly_set_free(st->set);
    st->set = NULL;

    st->set = lyd_find_path(st->dt, "//interface[name='iface2']//. | //ip | //interface[number((1 mod (20 - 15)) div 1)]//.");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 68);
    ly_set_free(st->set);
    st->set = NULL;

    st->set = lyd_find_path(st->dt, "//ietf-ip:ip[position() mod 2 = 1] | //ietf-ip:ip[position() mod 2 = 0]");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 10);
    assert_string_equal(((struct lyd_node_leaf_list *)st->set->set.d[0])->value_str, "10.0.0.1");
    assert_string_equal(((struct lyd_node_leaf_list *)st->set->set.d[1])->value_str, "172.0.0.1");
    assert_string_equal(((struct lyd_node_leaf_list *)st->set->set.d[2])->value_str, "10.0.0.2");
    assert_string_equal(((struct lyd_node_leaf_list *)st->set->set.d[7])->value_str, "10.0.0.1");
    assert_string_equal(((struct lyd_node_leaf_list *)st->set->set.d[9])->value_str, "2001:abcd:ef01:2345:6789:0:1:1");
    ly_set_free(st->set);
    st->set = NULL;

    st->set = lyd_find_path(st->dt, "(//*)[1] | (//*)[last()] | (//*)[10] | (//*)[8]//.");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 15);
    ly_set_free(st->set);
    st->set = NULL;
}

int main(void)
{
    const struct CMUnitTest tests[] = {
                    cmocka_unit_test_setup_teardown(test_invalid, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_simple, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_advanced, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_functions_operators, setup_f, teardown_f),
                    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
