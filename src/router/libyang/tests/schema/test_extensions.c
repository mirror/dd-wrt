/**
 * \file test_extensions.c
 * \author Radek Krejci <rkrejci@cesnet.cz>
 * \brief libyang tests - extensions
 *
 * Copyright (c) 2016 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cmocka.h>

#include "libyang.h"
#include "tests/config.h"

#define SCHEMA_FOLDER_YIN TESTS_DIR"/schema/yin/files"
#define SCHEMA_FOLDER_YANG TESTS_DIR"/schema/yang/files"

struct state {
    struct ly_ctx *ctx;
    int fd;
    char *str1;
    char *str2;
};

static int
setup_ctx(void **state, const char *searchdir)
{
    struct state *st;

    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error");
        return -1;
    }

    /* libyang context */
    st->ctx = ly_ctx_new(searchdir, 0);
    if (!st->ctx) {
        fprintf(stderr, "Failed to create context.\n");
        goto error;
    }

    st->fd = -1;

    return 0;

error:
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return -1;
}

static int
setup_ctx_yin(void **state)
{
    return setup_ctx(state, SCHEMA_FOLDER_YIN);
}

static int
setup_ctx_yang(void **state)
{
    return setup_ctx(state, SCHEMA_FOLDER_YANG);
}

static int
teardown_ctx(void **state)
{
    struct state *st = (*state);

    ly_ctx_destroy(st->ctx, NULL);
    if (st->fd >= 0) {
        close(st->fd);
    }
    free(st->str1);
    free(st->str2);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_module_sub_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "  </yang-version>\n  <namespace uri=\"urn:ext\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "  </namespace>\n  <prefix value=\"x\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "  </prefix>\n"
                    "  <import module=\"ext-def\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "    <prefix value=\"e\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </prefix>\n    <revision-date date=\"2017-01-18\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </revision-date>\n    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "  </import>\n"
                    "  <include module=\"ext-inc\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "    <revision-date date=\"2017-01-18\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </revision-date>\n    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "  </include>\n"
                    "  <revision date=\"2017-01-20\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "  </revision>\n  <revision date=\"2017-01-18\">\n"
                    "    <e:a/>\n"
                    "  </revision>\n"
                    "  <e:a/>\n  <e:b x=\"one\"/>\n  <e:c>\n    <e:y>one</e:y>\n  </e:c>\n"
                    "</module>\n";

    mod = lys_parse_mem(st->ctx, yin, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YIN, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yin);
}

static void
test_module_sub_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yang = "module ext {\n"
                    "  yang-version 1.1 {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "  }\n  namespace \"urn:ext\" {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "  }\n  prefix x {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "  }\n\n"
                    "  import ext-def {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "    prefix e {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    revision-date 2017-01-18 {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n\n"
                    "  include ext-inc {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "    revision-date 2017-01-18 {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n\n"
                    "  revision 2017-01-20 {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n"
                    "  revision 2017-01-18 {\n"
                    "    e:a;\n"
                    "  }\n\n"
                    "  e:a;\n  e:b \"one\";\n  e:c \"one\";\n}\n";

    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YANG, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yang);
}

static void
test_container_sub_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <container name=\"c\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "    <presence value=\"test\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </presence>\n"
                    "    <config value=\"false\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </config>\n"
                    "    <status value=\"current\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </status>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n"
                    "    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "  </container>\n</module>\n";

    mod = lys_parse_mem(st->ctx, yin, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YIN, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yin);
}

static void
test_container_sub_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yang = "module ext {\n"
                    "  namespace \"urn:ext\";\n  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  container c {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "    presence \"test\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    config false {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    status current {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n}\n";

    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YANG, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yang);
}

static void
test_leaf_sub_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <typedef name=\"length\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "    <type name=\"int32\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </type>\n"
                    "    <units name=\"meter\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </units>\n"
                    "    <default value=\"10\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </default>\n"
                    "    <status value=\"current\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </status>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n"
                    "    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "  </typedef>\n"
                    "  <leaf name=\"l\">\n"
                    "    <type name=\"string\">\n"
                    "      <pattern value=\"[a-z]\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <modifier value=\"invert-match\">\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "        </modifier>\n        <error-message>\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "          <value>emsg</value>\n"
                    "        </error-message>\n        <error-app-tag value=\"eapptag\">\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "        </error-app-tag>\n        <description>\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "          <text>desc</text>\n"
                    "        </description>\n        <reference>\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "          <text>ref</text>\n"
                    "        </reference>\n"
                    "      </pattern>\n"
                    "    </type>\n"
                    "    <units name=\"petipivo\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </units>\n"
                    "    <must condition=\"true()\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <error-message>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <value>emsg</value>\n"
                    "      </error-message>\n      <error-app-tag value=\"eapptag\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </error-app-tag>\n      <description>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>desc</text>\n"
                    "      </description>\n      <reference>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>ref</text>\n"
                    "      </reference>\n"
                    "    </must>\n"
                    "    <config value=\"false\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </config>\n"
                    "    <mandatory value=\"true\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </mandatory>\n"
                    "    <status value=\"current\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </status>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n"
                    "    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "  </leaf>\n  <leaf name=\"d\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "    <type name=\"length\"/>\n"
                    "    <default value=\"1\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </default>\n"
                    "  </leaf>\n</module>\n";

    mod = lys_parse_mem(st->ctx, yin, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YIN, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yin);
}

static void
test_leaf_sub_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yang = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  typedef length {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "    type int32 {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    units \"meter\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    default \"10\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    status current {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n\n"
                    "  leaf l {\n    type string {\n"
                    "      pattern \"[a-z]\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "        modifier invert-match {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n        error-message\n          \"emsg\" {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n        error-app-tag \"eapptag\" {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n        description\n          \"desc\" {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n        reference\n          \"ref\" {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n      }\n    }\n"
                    "    units \"petipivo\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    must \"true()\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "      error-message\n        \"emsg\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      error-app-tag \"eapptag\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      description\n        \"desc\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      reference\n        \"ref\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n"
                    "    }\n    config false {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    mandatory true {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    status current {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n\n"
                    "  leaf d {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "    type int8;\n    default \"1\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n}\n";

    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YANG, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yang);
}

static void
test_leaflist_sub_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <feature name=\"f1\"/>\n  <feature name=\"f\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "    <if-feature name=\"f1\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </if-feature>\n"
                    "    <status value=\"current\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </status>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n"
                    "    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "  </feature>\n"
                    "  <leaf-list name=\"l1\">\n"
                    "    <when condition=\"true()\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <description>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>desc</text>\n"
                    "      </description>\n"
                    "      <reference>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>ref</text>\n"
                    "      </reference>\n"
                    "    </when>\n    <if-feature name=\"f\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </if-feature>\n    <type name=\"string\">\n"
                    "      <length value=\"5\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <error-message>\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "          <value>emsg</value>\n"
                    "        </error-message>\n        <error-app-tag value=\"eapptag\">\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "        </error-app-tag>\n        <description>\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "          <text>desc</text>\n"
                    "        </description>\n        <reference>\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "          <text>ref</text>\n"
                    "        </reference>\n"
                    "      </length>\n"
                    "    </type>\n"
                    "    <units name=\"petipivo\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </units>\n"
                    "    <must condition=\"true()\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <error-message>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <value>emsg</value>\n"
                    "      </error-message>\n      <error-app-tag value=\"eapptag\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </error-app-tag>\n      <description>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>desc</text>\n"
                    "      </description>\n"
                    "      <reference>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>ref</text>\n"
                    "      </reference>\n"
                    "    </must>\n"
                    "    <config value=\"true\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </config>\n"
                    "    <min-elements value=\"1\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </min-elements>\n"
                    "    <max-elements value=\"1\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </max-elements>\n"
                    "    <ordered-by value=\"user\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </ordered-by>\n"
                    "    <status value=\"current\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </status>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n"
                    "    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "  </leaf-list>\n"
                    "  <leaf-list name=\"l2\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "    <type name=\"int8\">\n"
                    "      <range value=\"1..10\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <error-message>\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "          <value>emsg</value>\n"
                    "        </error-message>\n        <error-app-tag value=\"eapptag\">\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "        </error-app-tag>\n        <description>\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "          <text>desc</text>\n"
                    "        </description>\n        <reference>\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "          <text>ref</text>\n"
                    "        </reference>\n"
                    "      </range>\n"
                    "    </type>\n"
                    "    <default value=\"1\"/>\n    <default value=\"2\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </default>\n"
                    "  </leaf-list>\n</module>\n";

    mod = lys_parse_mem(st->ctx, yin, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YIN, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yin);
}

static void
test_leaflist_sub_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yang = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  feature f1;\n\n  feature f {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "    if-feature \"f1\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    status current {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n\n"
                    "  leaf-list l1 {\n"
                    "    when \"true()\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "      description\n        \"desc\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      reference\n        \"ref\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n"
                    "    }\n    if-feature \"f\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    type string {\n      length \"5\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "        error-message\n          \"emsg\" {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n        error-app-tag \"eapptag\" {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n        description\n          \"desc\" {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n        reference\n          \"ref\" {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n      }\n    }\n"
                    "    units \"petipivo\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    must \"true()\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "      error-message\n        \"emsg\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      error-app-tag \"eapptag\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      description\n        \"desc\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      reference\n        \"ref\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n"
                    "    }\n    config true {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    min-elements 1 {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    max-elements 1 {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    ordered-by user {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    status current {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n\n"
                    "  leaf-list l2 {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "    type int8 {\n      range \"1..10\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "        error-message\n          \"emsg\" {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n        error-app-tag \"eapptag\" {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n        description\n          \"desc\" {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n        reference\n          \"ref\" {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n      }\n    }\n"
                    "    default \"1\";\n"
                    "    default \"2\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n}\n";

    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YANG, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yang);
}

static void
test_list_sub_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <list name=\"l\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "    <key value=\"id\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </key>\n"
                    "    <unique tag=\"val1\"/>\n    <unique tag=\"val2\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </unique>\n"
                    "    <config value=\"true\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </config>\n"
                    "    <min-elements value=\"1\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </min-elements>\n"
                    "    <max-elements value=\"1\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </max-elements>\n"
                    "    <ordered-by value=\"user\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </ordered-by>\n"
                    "    <status value=\"current\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </status>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n"
                    "    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "    <leaf name=\"id\">\n"
                    "      <type name=\"instance-identifier\">\n"
                    "        <e:a/>\n"
                    "        <require-instance value=\"true\">\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "        </require-instance>\n"
                    "      </type>\n"
                    "    </leaf>\n"
                    "    <leaf name=\"val1\">\n"
                    "      <type name=\"decimal64\">\n"
                    "        <e:a/>\n"
                    "        <fraction-digits value=\"2\">\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "        </fraction-digits>\n"
                    "      </type>\n"
                    "    </leaf>\n"
                    "    <leaf name=\"val2\">\n"
                    "      <type name=\"leafref\">\n"
                    "        <e:a/>\n"
                    "        <path value=\"../val1\">\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "        </path>\n"
                    "        <require-instance value=\"true\">\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "        </require-instance>\n"
                    "      </type>\n"
                    "    </leaf>\n"
                    "  </list>\n</module>\n";

    mod = lys_parse_mem(st->ctx, yin, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YIN, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yin);
}

static void
test_list_sub_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yang = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  list l {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "    key \"id\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    unique \"val1\";\n    unique \"val2\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    config true {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    min-elements 1 {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    max-elements 1 {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    ordered-by user {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    status current {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    leaf id {\n"
                    "      type instance-identifier {\n"
                    "        e:a;\n"
                    "        require-instance true {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n      }\n"
                    "    }\n\n    leaf val1 {\n"
                    "      type decimal64 {\n"
                    "        e:a;\n"
                    "        fraction-digits 2 {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n      }\n"
                    "    }\n\n    leaf val2 {\n"
                    "      type leafref {\n"
                    "        e:a;\n"
                    "        path \"../val1\" {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n        require-instance true {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n      }\n"
                    "    }\n  }\n}\n";

    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YANG, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yang);
}

static void
test_anydata_sub_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <feature name=\"f\"/>\n"
                    "  <anyxml name=\"a\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "    <when condition=\"true()\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <description>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>desc</text>\n"
                    "      </description>\n"
                    "      <reference>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>ref</text>\n"
                    "      </reference>\n"
                    "    </when>\n    <if-feature name=\"f\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </if-feature>\n"
                    "    <must condition=\"true()\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <error-message>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <value>emsg</value>\n"
                    "      </error-message>\n      <error-app-tag value=\"eapptag\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </error-app-tag>\n      <description>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>desc</text>\n"
                    "      </description>\n      <reference>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>ref</text>\n"
                    "      </reference>\n"
                    "    </must>\n"
                    "    <config value=\"true\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </config>\n"
                    "    <mandatory value=\"true\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </mandatory>\n"
                    "    <status value=\"current\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </status>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n"
                    "    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "  </anyxml>\n</module>\n";

    mod = lys_parse_mem(st->ctx, yin, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YIN, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yin);
}

static void
test_anydata_sub_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yang = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  feature f;\n\n"
                    "  anyxml l {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "    when \"true()\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "      description\n        \"desc\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      reference\n        \"ref\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n"
                    "    }\n    if-feature \"f\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    must \"true()\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "      error-message\n        \"emsg\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      error-app-tag \"eapptag\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      description\n        \"desc\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      reference\n        \"ref\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n"
                    "    }\n    config true {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    mandatory true {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    status current {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n}\n";

    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YANG, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yang);
}

static void
test_choice_sub_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <choice name=\"ch\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "    <default value=\"a\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </default>\n"
                    "    <config value=\"true\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </config>\n"
                    "    <mandatory value=\"false\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </mandatory>\n"
                    "    <status value=\"current\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </status>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n"
                    "    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "    <case name=\"a\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <status value=\"current\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </status>\n"
                    "      <description>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>desc</text>\n"
                    "      </description>\n"
                    "      <reference>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>ref</text>\n"
                    "      </reference>\n"
                    "      <leaf name=\"c\">\n"
                    "        <type name=\"bits\">\n"
                    "          <bit name=\"zero\">\n"
                    "            <e:a/>\n            <e:b x=\"one\"/>\n            <e:c>\n              <e:y>one</e:y>\n            </e:c>\n"
                    "            <position value=\"0\">\n"
                    "              <e:a/>\n              <e:b x=\"one\"/>\n              <e:c>\n                <e:y>one</e:y>\n              </e:c>\n"
                    "            </position>\n"
                    "            <status value=\"current\">\n"
                    "              <e:a/>\n              <e:b x=\"one\"/>\n              <e:c>\n                <e:y>one</e:y>\n              </e:c>\n"
                    "            </status>\n"
                    "            <description>\n"
                    "              <e:a/>\n              <e:b x=\"one\"/>\n              <e:c>\n                <e:y>one</e:y>\n              </e:c>\n"
                    "              <text>desc</text>\n"
                    "            </description>\n"
                    "            <reference>\n"
                    "              <e:a/>\n              <e:b x=\"one\"/>\n              <e:c>\n                <e:y>one</e:y>\n              </e:c>\n"
                    "              <text>ref</text>\n"
                    "            </reference>\n"
                    "          </bit>\n"
                    "          <bit name=\"one\">\n            <e:a/>\n          </bit>\n"
                    "        </type>\n"
                    "      </leaf>\n"
                    "    </case>\n"
                    "    <leaf name=\"b\">\n"
                    "      <type name=\"enumeration\">\n"
                    "        <enum name=\"one\">\n          <e:a/>\n        </enum>\n"
                    "        <enum name=\"two\">\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "          <value value=\"2\">\n"
                    "            <e:a/>\n            <e:b x=\"one\"/>\n            <e:c>\n              <e:y>one</e:y>\n            </e:c>\n"
                    "          </value>\n"
                    "          <status value=\"current\">\n"
                    "            <e:a/>\n            <e:b x=\"one\"/>\n            <e:c>\n              <e:y>one</e:y>\n            </e:c>\n"
                    "          </status>\n"
                    "          <description>\n"
                    "            <e:a/>\n            <e:b x=\"one\"/>\n            <e:c>\n              <e:y>one</e:y>\n            </e:c>\n"
                    "            <text>desc</text>\n"
                    "          </description>\n"
                    "          <reference>\n"
                    "            <e:a/>\n            <e:b x=\"one\"/>\n            <e:c>\n              <e:y>one</e:y>\n            </e:c>\n"
                    "            <text>ref</text>\n"
                    "          </reference>\n"
                    "        </enum>\n"
                    "      </type>\n"
                    "    </leaf>\n"
                    "  </choice>\n</module>\n";

    mod = lys_parse_mem(st->ctx, yin, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YIN, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yin);
}

static void
test_choice_sub_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yang = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  choice ch {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "    default \"a\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    config true {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    mandatory false {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    status current {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    case a {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "      status current {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      description\n        \"desc\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      reference\n        \"ref\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      leaf c {\n"
                    "        type bits {\n"
                    "          bit zero {\n"
                    "            e:a;\n            e:b \"one\";\n            e:c \"one\";\n"
                    "            position 0 {\n"
                    "              e:a;\n              e:b \"one\";\n              e:c \"one\";\n"
                    "            }\n"
                    "            status current {\n"
                    "              e:a;\n              e:b \"one\";\n              e:c \"one\";\n"
                    "            }\n            description\n              \"desc\" {\n"
                    "              e:a;\n              e:b \"one\";\n              e:c \"one\";\n"
                    "            }\n            reference\n              \"ref\" {\n"
                    "              e:a;\n              e:b \"one\";\n              e:c \"one\";\n"
                    "            }\n          }\n"
                    "          bit one {\n"
                    "            e:a;\n"
                    "          }\n"
                    "        }\n"
                    "      }\n\n      leaf b {\n"
                    "        type enumeration {\n"
                    "          enum \"one\" {\n"
                    "            e:a;\n"
                    "          }\n"
                    "          enum \"two\" {\n"
                    "            e:a;\n            e:b \"one\";\n            e:c \"one\";\n"
                    "            value 2 {\n"
                    "              e:a;\n              e:b \"one\";\n              e:c \"one\";\n"
                    "            }\n"
                    "            status current {\n"
                    "              e:a;\n              e:b \"one\";\n              e:c \"one\";\n"
                    "            }\n            description\n              \"desc\" {\n"
                    "              e:a;\n              e:b \"one\";\n              e:c \"one\";\n"
                    "            }\n            reference\n              \"ref\" {\n"
                    "              e:a;\n              e:b \"one\";\n              e:c \"one\";\n"
                    "            }\n          }\n        }\n      }\n    }\n  }\n}\n";

    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YANG, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yang);
}

static void
test_uses_sub_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <identity name=\"zero\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "    <status value=\"current\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </status>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n"
                    "    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "  </identity>\n"
                    "  <identity name=\"one\">\n"
                    "    <base name=\"zero\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </base>\n"
                    "  </identity>\n"
                    "  <identity name=\"two\">\n"
                    "    <base name=\"zero\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </base>\n"
                    "    <base name=\"one\">\n"
                    "      <e:a/>\n"
                    "    </base>\n"
                    "  </identity>\n"
                    "  <grouping name=\"grp\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "    <status value=\"current\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </status>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n"
                    "    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "    <container name=\"c\">\n      <e:a/>\n    </container>\n"
                    "    <leaf name=\"l\">\n"
                    "      <type name=\"identityref\">\n"
                    "        <base name=\"two\">\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "        </base>\n"
                    "      </type>\n"
                    "      <mandatory value=\"true\">\n        <e:a/>\n      </mandatory>\n"
                    "    </leaf>\n"
                    "    <leaf-list name=\"ll1\">\n"
                    "      <type name=\"int8\"/>\n"
                    "      <min-elements value=\"2\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </min-elements>\n"
                    "    </leaf-list>\n"
                    "    <leaf-list name=\"ll2\">\n"
                    "      <type name=\"int8\"/>\n"
                    "    </leaf-list>\n"
                    "  </grouping>\n"
                    "  <uses name=\"grp\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "    <status value=\"current\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </status>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n"
                    "    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "    <refine target-node=\"c\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <presence value=\"true\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </presence>\n"
                    "      <config value=\"false\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </config>\n"
                    "      <description>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>desc</text>\n"
                    "      </description>\n"
                    "      <reference>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>ref</text>\n"
                    "      </reference>\n"
                    "    </refine>\n"
                    "    <refine target-node=\"l\">\n"
                    "      <mandatory value=\"false\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </mandatory>\n"
                    "    </refine>\n"
                    "    <refine target-node=\"ll1\">\n"
                    "      <min-elements value=\"1\">\n"
                    "        <e:a/>\n"
                    "      </min-elements>\n"
                    "      <max-elements value=\"2\">\n"
                    "        <e:b x=\"one\"/>\n"
                    "      </max-elements>\n"
                    "    </refine>\n"
                    "    <refine target-node=\"ll2\">\n"
                    "      <e:a/>\n"
                    "      <default value=\"1\"/>\n"
                    "      <default value=\"2\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </default>\n"
                    "    </refine>\n"
                    "    <augment target-node=\"c\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <status value=\"current\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </status>\n"
                    "      <description>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>desc</text>\n"
                    "      </description>\n"
                    "      <reference>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>ref</text>\n"
                    "      </reference>\n"
                    "      <leaf name=\"a\">\n"
                    "        <type name=\"int8\"/>\n"
                    "      </leaf>\n"
                    "    </augment>\n"
                    "  </uses>\n"
                    "</module>\n";
    struct lys_node *uses;

    mod = lys_parse_mem(st->ctx, yin, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YIN, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yin);

    /* check applied refine's substatements' extensions */
    uses = mod->data->prev;
    assert_int_equal(uses->nodetype, LYS_USES);

    assert_int_equal(uses->child->ext_size, 15); /* number of extensions in c */
    assert_int_equal(uses->child->next->ext_size, 3); /* number of extensions in l */
    assert_int_equal(uses->child->next->next->ext_size, 2); /* number of extensions in ll1 */
    assert_int_equal(uses->child->prev->ext_size, 4); /* number of extensions in ll2 */
}

static void
test_uses_sub_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yang = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  identity zero {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "    status current {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n\n"
                    "  identity one {\n"
                    "    base zero {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n\n"
                    "  identity two {\n"
                    "    base zero {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    base one {\n"
                    "      e:a;\n"
                    "    }\n  }\n\n"
                    "  grouping grp {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "    status current {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    container c;\n\n"
                    "    leaf l {\n"
                    "      type identityref {\n"
                    "        base two {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n      }\n"
                    "      mandatory true {\n"
                    "        e:a;\n"
                    "      }\n    }\n\n"
                    "    leaf-list ll1 {\n"
                    "      type int8;\n"
                    "    }\n\n"
                    "    leaf-list ll2 {\n"
                    "      type int8;\n"
                    "    }\n  }\n\n"
                    "  uses grp {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "    status current {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    refine \"c\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "      presence \"true\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      config false {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      description\n        \"desc\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      reference\n        \"ref\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n"
                    "    }\n    refine \"l\" {\n"
                    "      mandatory false {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n"
                    "    }\n    refine \"ll1\" {\n"
                    "      min-elements 1 {\n"
                    "        e:a;\n"
                    "      }\n      max-elements 2 {\n"
                    "        e:b \"one\";\n"
                    "      }\n"
                    "    }\n    refine \"ll2\" {\n"
                    "      e:a;\n"
                    "      default \"1\";\n"
                    "      default \"2\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n"
                    "    }\n    augment \"c\" {\n"
                    "      status current {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      description\n        \"desc\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      reference\n        \"ref\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      leaf a {\n"
                    "        type int8;\n"
                    "      }\n    }\n  }\n}\n";
    struct lys_node *uses;

    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YANG, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yang);

    /* check applied refine's substatements' extensions */
    uses = mod->data->prev;
    assert_int_equal(uses->nodetype, LYS_USES);

    assert_int_equal(uses->child->ext_size, 15); /* number of extensions in c */
    assert_int_equal(uses->child->next->ext_size, 3); /* number of extensions in l */
    assert_int_equal(uses->child->next->next->ext_size, 2); /* number of extensions in ll1 */
    assert_int_equal(uses->child->prev->ext_size, 4); /* number of extensions in ll2 */
}

static void
test_extension_sub_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <extension name=\"x\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "    <argument name=\"y\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <yin-element value=\"false\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </yin-element>\n"
                    "    </argument>\n"
                    "    <status value=\"current\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </status>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n"
                    "    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "  </extension>\n"
                    "  <e:a>\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "  </e:a>\n</module>\n";

    mod = lys_parse_mem(st->ctx, yin, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YIN, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yin);
}

static void
test_extension_sub_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yang = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  extension x {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "    argument y {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "      yin-element false {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n    }\n"
                    "    status current {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n\n"
                    "  e:a {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "  }\n}\n";

    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YANG, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yang);
}

static void
test_rpc_sub_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <container name=\"c\">\n"
                    "    <action name=\"a\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <status value=\"current\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </status>\n"
                    "      <description>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>desc</text>\n"
                    "      </description>\n"
                    "      <reference>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>ref</text>\n"
                    "      </reference>\n"
                    "      <input>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <leaf name=\"in\">\n"
                    "          <type name=\"int8\"/>\n"
                    "        </leaf>\n"
                    "      </input>\n"
                    "      <output>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <leaf name=\"in\">\n"
                    "          <type name=\"int8\"/>\n"
                    "        </leaf>\n"
                    "      </output>\n"
                    "    </action>\n"
                    "  </container>\n"
                    "  <rpc name=\"r\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "    <status value=\"current\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </status>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n"
                    "    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "    <input>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <leaf name=\"in\">\n"
                    "        <type name=\"int8\"/>\n"
                    "      </leaf>\n"
                    "    </input>\n"
                    "    <output>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <leaf name=\"in\">\n"
                    "        <type name=\"int8\"/>\n"
                    "      </leaf>\n"
                    "    </output>\n"
                    "  </rpc>\n</module>\n";

    mod = lys_parse_mem(st->ctx, yin, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YIN, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yin);
}

static void
test_rpc_sub_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yang = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  container c {\n"
                    "    action a {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "      status current {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      description\n        \"desc\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      reference\n        \"ref\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      input {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "        leaf in {\n"
                    "          type int8;\n"
                    "        }\n      }\n\n"
                    "      output {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "        leaf out {\n"
                    "          type int8;\n"
                    "        }\n      }\n    }\n  }\n\n"
                    "  rpc r {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "    status current {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    input {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "      leaf in {\n"
                    "        type int8;\n"
                    "      }\n    }\n\n"
                    "    output {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "      leaf out {\n"
                    "        type int8;\n"
                    "      }\n    }\n  }\n}\n";

    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YANG, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yang);
}

static void
test_notif_sub_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <container name=\"c\">\n"
                    "    <notification name=\"n-in-c\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <status value=\"current\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </status>\n"
                    "      <description>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>desc</text>\n"
                    "      </description>\n"
                    "      <reference>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>ref</text>\n"
                    "      </reference>\n"
                    "      <leaf name=\"info\">\n"
                    "        <type name=\"int8\"/>\n"
                    "      </leaf>\n"
                    "    </notification>\n"
                    "  </container>\n"
                    "  <notification name=\"n\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "    <status value=\"current\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </status>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n"
                    "    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "    <leaf name=\"info\">\n"
                    "      <type name=\"int8\"/>\n"
                    "    </leaf>\n"
                    "  </notification>\n</module>\n";

    mod = lys_parse_mem(st->ctx, yin, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YIN, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yin);
}

static void
test_notif_sub_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yang = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  container c {\n"
                    "    notification n-in-c {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "      status current {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      description\n        \"desc\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      reference\n        \"ref\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      leaf out {\n"
                    "        type int8;\n"
                    "      }\n    }\n  }\n\n"
                    "  notification n {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "    status current {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    leaf in {\n"
                    "      type int8;\n"
                    "    }\n  }\n}\n";

    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YANG, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yang);
}

static void
test_deviation_sub_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod, *dev;
    const char *yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <deviation target-node=\"/e:l1\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n"
                    "    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "    <deviate value=\"not-supported\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </deviate>\n"
                    "  </deviation>\n"
                    "  <deviation target-node=\"/e:ll1\">\n"
                    "    <deviate value=\"add\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <units name=\"meter\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </units>\n"
                    "      <default value=\"1\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </default>\n"
                    "      <default value=\"2\">\n"
                    "        <e:a/>\n"
                    "      </default>\n"
                    "      <config value=\"false\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </config>\n"
                    "    </deviate>\n"
                    "  </deviation>\n"
                    "  <deviation target-node=\"/e:lst1\">\n"
                    "    <deviate value=\"add\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n"
                    "      <unique tag=\"val1\"/>\n      <unique tag=\"val2\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </unique>\n"
                    "      <min-elements value=\"1\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </min-elements>\n"
                    "      <max-elements value=\"1\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </max-elements>\n"
                    "    </deviate>\n"
                    "  </deviation>\n"
                    "  <deviation target-node=\"/e:l2\">\n"
                    "    <deviate value=\"replace\">\n"
                    "      <e:b x=\"ten\"/>\n"
                    "      <mandatory value=\"false\">\n"
                    "        <e:a/>\n"
                    "      </mandatory>\n"
                    "    </deviate>\n"
                    "  </deviation>\n"
                    "  <deviation target-node=\"/e:lst1/e:val2\">\n"
                    "    <deviate value=\"delete\">\n"
                    "      <e:a/>\n"
                    "      <units name=\"meter\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </units>\n"
                    "      <default value=\"1\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </default>\n"
                    "    </deviate>\n"
                    "  </deviation>\n"
                    "  <deviation target-node=\"/e:lst2\">\n"
                    "    <deviate value=\"delete\">\n"
                    "      <e:a/>\n      <e:b x=\"two\"/>\n"
                    "      <unique tag=\"val1\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </unique>\n"
                    "    </deviate>\n"
                    "  </deviation>\n"
                    "</module>\n";
    struct lys_node *node;

    mod = ly_ctx_load_module(st->ctx, "ext-def", NULL);
    assert_ptr_not_equal(mod, NULL);

    dev = lys_parse_mem(st->ctx, yin, LYS_IN_YIN);
    assert_ptr_not_equal(dev, NULL);

    lys_print_mem(&st->str1, dev, LYS_OUT_YIN, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yin);

    /* check extensions in the deviated data */
    /* l1 is removed */
    assert_string_not_equal(mod->data->name, "l1");
    assert_string_not_equal(mod->data->prev->name, "l1");
    assert_string_not_equal(mod->data->next->name, "l1");

    /* l2 is now first and instead of 5 extensions it now has only 3 */
    node = mod->data;
    assert_string_equal(node->name, "l2");
    assert_int_equal(node->flags & LYS_MAND_MASK, LYS_MAND_FALSE);
    assert_int_equal(node->ext_size, 3);

    /* ll1 has 13 extensions (10 from substatements) */
    node = mod->data->next;
    assert_string_equal(node->name, "ll1");
    assert_int_equal(node->ext_size, 13);

    /* lst1 has 12 extensions (2 added, 9 added from substatements) */
    node = mod->data->next->next;
    assert_string_equal(node->name, "lst1");
    assert_int_equal(node->ext_size, 12);

    /* lst2 has 1 ext, since the deviation removes unique with all its extensions and 3 of the 4 node's extensions */
    node = mod->data->prev;
    assert_string_equal(node->name, "lst2");
    assert_int_equal(node->ext_size, 1);

    /* val2 has no extension, all were deleted */
    node = mod->data->next->next->child->prev;
    assert_string_equal(node->name, "val2");
    assert_string_equal(node->parent->name, "lst1");
    assert_int_equal(node->ext_size, 0);

    /* revert deviations */
    ly_ctx_remove_module(dev, NULL);

    /* l1 is reconnected at the end of data nodes */
    assert_string_equal(mod->data->prev->name, "l1");

    /* l2 is back true and contains again the 5 extensions */
    node = mod->data;
    assert_string_equal(node->name, "l2");
    assert_int_equal(node->flags & LYS_MAND_MASK, LYS_MAND_TRUE);
    assert_int_equal(node->ext_size, 5);

    /* ll1 has no extension again */
    node = mod->data->next;
    assert_string_equal(node->name, "ll1");
    assert_int_equal(node->ext_size, 0);

    /* lst1 has back 1 extension */
    node = mod->data->next->next;
    assert_string_equal(node->name, "lst1");
    assert_int_equal(node->ext_size, 1);

    /* lst2 has back all the 5 original extensions */
    node = mod->data->prev->prev; /* lst2 is not last, there is added l1 */
    assert_string_equal(node->name, "lst2");
    assert_int_equal(node->ext_size, 5);

    /* val2 has back all its 2 extensions */
    node = mod->data->next->next->child->prev;
    assert_string_equal(node->name, "val2");
    assert_string_equal(node->parent->name, "lst1");
    assert_int_equal(node->ext_size, 2);
}

static void
test_deviation_sub_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod, *dev;
    const char *yang = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  deviation \"/e:l1\" {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    deviate not-supported {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n\n"
                    "  deviation \"/e:ll1\" {\n"
                    "    deviate add {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "      units \"meter\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      default \"1\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      default \"2\" {\n"
                    "        e:a;\n"
                    "      }\n      config false {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n    }\n  }\n\n"
                    "  deviation \"/e:lst1\" {\n"
                    "    deviate add {\n"
                    "      e:a;\n      e:b \"one\";\n"
                    "      unique \"val1\";\n"
                    "      unique \"val2\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      min-elements 1 {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      max-elements 2 {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n    }\n  }\n\n"
                    "  deviation \"/e:l2\" {\n"
                    "    deviate replace {\n"
                    "      e:b \"ten\";\n"
                    "      mandatory false {\n"
                    "        e:a;\n"
                    "      }\n    }\n  }\n\n"
                    "  deviation \"/e:lst1/e:val2\" {\n"
                    "    deviate delete {\n"
                    "      e:a;\n"
                    "      units \"meter\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      default \"1\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n    }\n  }\n\n"
                    "  deviation \"/e:lst2\" {\n"
                    "    deviate delete {\n"
                    "      e:a;\n      e:b \"two\";\n"
                    "      unique \"val1\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n    }\n  }\n}\n";
    struct lys_node *node;

    mod = ly_ctx_load_module(st->ctx, "ext-def", NULL);
    assert_ptr_not_equal(mod, NULL);

    dev = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(dev, NULL);

    lys_print_mem(&st->str1, dev, LYS_OUT_YANG, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yang);

    /* check extensions in the deviated data */
    /* l1 is removed */
    assert_string_not_equal(mod->data->name, "l1");
    assert_string_not_equal(mod->data->prev->name, "l1");
    assert_string_not_equal(mod->data->next->name, "l1");

    /* l2 is now first and instead of 5 extensions it now has only 3 */
    node = mod->data;
    assert_string_equal(node->name, "l2");
    assert_int_equal(node->flags & LYS_MAND_MASK, LYS_MAND_FALSE);
    assert_int_equal(node->ext_size, 3);

    /* ll1 has 13 extensions (10 from substatements) */
    node = mod->data->next;
    assert_string_equal(node->name, "ll1");
    assert_int_equal(node->ext_size, 13);

    /* lst1 has 12 extensions (2 added, 9 added from substatements) */
    node = mod->data->next->next;
    assert_string_equal(node->name, "lst1");
    assert_int_equal(node->ext_size, 12);

    /* lst2 has 1 ext, since the deviation removes unique with all its extensions and 3 of the 4 node's extensions */
    node = mod->data->prev;
    assert_string_equal(node->name, "lst2");
    assert_int_equal(node->ext_size, 1);

    /* val2 has no extension, all were deleted */
    node = mod->data->next->next->child->prev;
    assert_string_equal(node->name, "val2");
    assert_string_equal(node->parent->name, "lst1");
    assert_int_equal(node->ext_size, 0);

    /* revert deviations */
    ly_ctx_remove_module(dev, NULL);

    /* l1 is reconnected at the end of data nodes */
    assert_string_equal(mod->data->prev->name, "l1");

    /* l2 is back true and contains again the 5 extensions */
    node = mod->data;
    assert_string_equal(node->name, "l2");
    assert_int_equal(node->flags & LYS_MAND_MASK, LYS_MAND_TRUE);
    assert_int_equal(node->ext_size, 5);

    /* ll1 has no extension again */
    node = mod->data->next;
    assert_string_equal(node->name, "ll1");
    assert_int_equal(node->ext_size, 0);

    /* lst1 has back 1 extension */
    node = mod->data->next->next;
    assert_string_equal(node->name, "lst1");
    assert_int_equal(node->ext_size, 1);

    /* lst2 has back all the 5 original extensions */
    node = mod->data->prev->prev; /* lst2 is not last, there is added l1 */
    assert_string_equal(node->name, "lst2");
    assert_int_equal(node->ext_size, 5);

    /* val2 has back all its 2 extensions */
    node = mod->data->next->next->child->prev;
    assert_string_equal(node->name, "val2");
    assert_string_equal(node->parent->name, "lst1");
    assert_int_equal(node->ext_size, 2);
}

static void
test_complex_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex>\n"
                    "    <argument name=\"a\"/>\n"
                    "    <base name=\"b\"/>\n"
                    "    <belongs-to module=\"test\">\n      <prefix value=\"t\"/>\n    </belongs-to>\n"
                    "    <contact>\n      <text>contact</text>\n    </contact>\n"
                    "    <default value=\"c\"/>\n"
                    "    <description>\n      <text>description</text>\n    </description>\n"
                    "    <error-app-tag value=\"d\"/>\n"
                    "    <error-message>\n      <value>errmsg</value>\n    </error-message>\n"
                    "    <key value=\"e\"/>\n"
                    "    <namespace uri=\"urn\"/>\n"
                    "    <organization>\n      <text>org</text>\n    </organization>\n"
                    "    <path value=\"f\"/>\n"
                    "    <prefix value=\"g\"/>\n"
                    "    <presence value=\"h\"/>\n"
                    "    <reference>\n      <text>reference</text>\n    </reference>\n"
                    "    <revision-date date=\"i\"/>\n"
                    "    <units name=\"j\"/>\n"
                    "    <modifier value=\"invert-match\"/>\n"
                    "    <require-instance value=\"true\"/>\n"
                    "    <config value=\"true\"/>\n"
                    "    <mandatory value=\"true\"/>\n"
                    "    <ordered-by value=\"user\"/>\n"
                    "    <status value=\"obsolete\"/>\n"
                    "    <fraction-digits value=\"5\"/>\n"
                    "    <max-elements value=\"2\"/>\n"
                    "    <min-elements value=\"4\"/>\n"
                    "    <position value=\"11\"/>\n"
                    "    <value value=\"355\"/>\n"
                    "    <unique tag=\"e\"/>\n"
                    "    <module name=\"inmod\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:i=\"urn:inmod\">\n"
                    "      <namespace uri=\"urn:inmod\"/>\n"
                    "      <prefix value=\"i\"/>\n"
                    "    </module>\n"
                    "    <action name=\"action\"/>\n"
                    "    <anydata name=\"anydata\"/>\n"
                    "    <anyxml name=\"anyxml\"/>\n"
                    "    <case name=\"case\"/>\n"
                    "    <choice name=\"choice\">\n"
                    "      <anydata name=\"a\"/>\n"
                    "      <anydata name=\"b\"/>\n"
                    "    </choice>\n"
                    "    <container name=\"container\">\n"
                    "      <presence value=\"presence\"/>\n"
                    "    </container>\n"
                    "    <grouping name=\"grp\">\n"
                    "      <anydata name=\"c\"/>\n"
                    "    </grouping>\n"
                    "    <input>\n"
                    "      <anydata name=\"d\"/>\n"
                    "    </input>\n"
                    "    <leaf name=\"e\">\n"
                    "      <type name=\"string\"/>\n"
                    "    </leaf>\n"
                    "    <leaf-list name=\"f\">\n"
                    "      <type name=\"string\"/>\n"
                    "    </leaf-list>\n"
                    "    <list name=\"g\">\n"
                    "      <key value=\"k\"/>\n"
                    "      <leaf name=\"k\">\n"
                    "        <type name=\"string\"/>\n"
                    "      </leaf>\n"
                    "    </list>\n"
                    "    <notification name=\"h\">\n"
                    "      <anydata name=\"i\"/>\n"
                    "    </notification>\n"
                    "    <output>\n"
                    "      <anydata name=\"i\"/>\n"
                    "    </output>\n"
                    "    <uses name=\"grp\"/>\n"
                    "    <typedef name=\"mytype\">\n      <type name=\"string\"/>\n    </typedef>\n"
                    "    <type name=\"string\"/>\n"
                    "    <if-feature name=\"f\"/>\n"
                    "    <length value=\"10..20\"/>\n"
                    "    <must condition=\"1\"/>\n"
                    "    <pattern value=\"[a-z]*\"/>\n"
                    "    <range value=\"0..10\"/>\n"
                    "    <when condition=\"1\"/>\n"
                    "    <revision date=\"2016-02-16\"/>\n"
                    "  </e:complex>\n"
                    "  <feature name=\"f\"/>\n"
                    "</module>\n";

    mod = lys_parse_mem(st->ctx, yin, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YIN, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yin);
}

static void
test_complex_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yang = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex {\n"
                    "    argument a;\n"
                    "    base b;\n"
                    "    belongs-to test {\n      prefix t;\n    }\n"
                    "    contact\n      \"contact\";\n"
                    "    default \"c\";\n"
                    "    description\n      \"description\";\n"
                    "    error-app-tag \"d\";\n"
                    "    error-message\n      \"errmsg\";\n"
                    "    key \"e\";\n"
                    "    namespace \"urn\";\n"
                    "    organization\n      \"org\";\n"
                    "    path \"f\";\n"
                    "    prefix g;\n"
                    "    presence \"h\";\n"
                    "    reference\n      \"reference\";\n"
                    "    revision-date 2017-02-21;\n"
                    "    units \"j\";\n"
                    "    modifier invert-match;\n"
                    "    require-instance true;\n"
                    "    config true;\n"
                    "    mandatory true;\n"
                    "    ordered-by user;\n"
                    "    status obsolete;\n"
                    "    fraction-digits 5;\n"
                    "    max-elements 2;\n"
                    "    min-elements 4;\n"
                    "    position 11;\n"
                    "    value 355;\n"
                    "    unique \"e\";\n"
                    "    module inmod {\n"
                    "      namespace \"urn:inmod\";\n"
                    "      prefix i;\n    }\n"
                    "    action action;\n\n"
                    "    anydata anydata;\n\n"
                    "    anyxml anyxml;\n\n"
                    "    case case;\n\n"
                    "    choice choice {\n      anydata a;\n\n      anydata b;\n    }\n\n"
                    "    container container {\n      presence \"presence\";\n    }\n\n"
                    "    grouping grp {\n      anydata c;\n    }\n\n"
                    "    input {\n      anydata d;\n    }\n\n"
                    "    leaf e {\n      type string;\n    }\n\n"
                    "    leaf-list f {\n      type string;\n    }\n\n"
                    "    list g {\n      key \"k\";\n"
                    "      leaf k {\n        type string;\n      }\n    }\n\n"
                    "    notification h {\n      anydata i;\n    }\n\n"
                    "    output {\n      anydata i;\n    }\n\n"
                    "    uses grp;\n"
                    "    typedef mytype {\n      type string;\n    }\n"
                    "    type string;\n"
                    "    if-feature \"f\";\n"
                    "    length \"10..20\";\n"
                    "    must \"1\";\n"
                    "    pattern \"[a-z]*\";\n"
                    "    range \"0..10\";\n"
                    "    when \"1\";\n"
                    "    revision 2016-02-16;\n  }\n\n"
                    "  feature f;\n}\n";

    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YANG, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yang);
}

static void
test_complex_arrays_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex-arrays>\n"
                    "    <argument name=\"a\">\n"
                    "      <yin-element value=\"true\"/>\n"
                    "    </argument>\n"
                    "    <argument name=\"b\"/>\n"
                    "    <base name=\"a\"/>\n"
                    "    <base name=\"b\"/>\n"
                    "    <belongs-to module=\"test1\">\n      <prefix value=\"t1\"/>\n    </belongs-to>\n"
                    "    <belongs-to module=\"test2\">\n      <prefix value=\"t2\"/>\n    </belongs-to>\n"
                    "    <contact>\n      <text>contact1</text>\n    </contact>\n"
                    "    <contact>\n      <text>contact2</text>\n    </contact>\n"
                    "    <default value=\"a\"/>\n"
                    "    <default value=\"b\"/>\n"
                    "    <description>\n      <text>description1</text>\n    </description>\n"
                    "    <description>\n      <text>description2</text>\n    </description>\n"
                    "    <error-app-tag value=\"a\"/>\n"
                    "    <error-app-tag value=\"b\"/>\n"
                    "    <error-message>\n      <value>errmsg1</value>\n    </error-message>\n"
                    "    <error-message>\n      <value>errmsg2</value>\n    </error-message>\n"
                    "    <key value=\"a\"/>\n"
                    "    <key value=\"b\"/>\n"
                    "    <namespace uri=\"urn1\"/>\n"
                    "    <namespace uri=\"urn2\"/>\n"
                    "    <organization>\n      <text>org1</text>\n    </organization>\n"
                    "    <organization>\n      <text>org2</text>\n    </organization>\n"
                    "    <path value=\"a\"/>\n"
                    "    <path value=\"b\"/>\n"
                    "    <prefix value=\"a\"/>\n"
                    "    <prefix value=\"b\"/>\n"
                    "    <presence value=\"a\"/>\n"
                    "    <presence value=\"b\"/>\n"
                    "    <reference>\n      <text>reference1</text>\n    </reference>\n"
                    "    <reference>\n      <text>reference2</text>\n    </reference>\n"
                    "    <units name=\"a\"/>\n"
                    "    <units name=\"b\"/>\n"
                    "    <fraction-digits value=\"5\"/>\n"
                    "    <fraction-digits value=\"10\"/>\n"
                    "    <max-elements value=\"2\"/>\n"
                    "    <max-elements value=\"3\"/>\n"
                    "    <min-elements value=\"4\"/>\n"
                    "    <min-elements value=\"5\"/>\n"
                    "    <position value=\"11\"/>\n"
                    "    <position value=\"12\"/>\n"
                    "    <value value=\"42\"/>\n"
                    "    <value value=\"-55\"/>\n"
                    "    <unique tag=\"l1 l2\"/>\n"
                    "    <unique tag=\"l2\"/>\n"
                    "    <module name=\"inmod1\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:i=\"urn:inmod1\">\n"
                    "      <namespace uri=\"urn:inmod1\"/>\n"
                    "      <prefix value=\"i\"/>\n"
                    "    </module>\n"
                    "    <module name=\"inmod2\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:i=\"urn:inmod2\">\n"
                    "      <namespace uri=\"urn:inmod2\"/>\n"
                    "      <prefix value=\"i\"/>\n"
                    "    </module>\n"
                    "    <anydata name=\"anydata1\"/>\n"
                    "    <anydata name=\"anydata2\"/>\n"
                    "    <anyxml name=\"anyxml1\"/>\n"
                    "    <anyxml name=\"anyxml2\"/>\n"
                    "    <leaf name=\"l1\">\n      <type name=\"string\"/>\n    </leaf>\n"
                    "    <leaf name=\"l2\">\n      <type name=\"string\"/>\n    </leaf>\n"
                    "    <typedef name=\"mytype1\">\n      <type name=\"string\"/>\n    </typedef>\n"
                    "    <typedef name=\"mytype2\">\n      <type name=\"string\"/>\n    </typedef>\n"
                    "    <type name=\"string\"/>\n"
                    "    <type name=\"uint8\"/>\n"
                    "    <if-feature name=\"f1\"/>\n"
                    "    <if-feature name=\"f2\"/>\n"
                    "    <length value=\"10\"/>\n"
                    "    <length value=\"20\"/>\n"
                    "    <must condition=\"1\"/>\n"
                    "    <must condition=\"2\"/>\n"
                    "    <pattern value=\"[a-z]*\"/>\n"
                    "    <pattern value=\"[A-Z]*\"/>\n"
                    "    <range value=\"0..10\"/>\n"
                    "    <range value=\"100..110\"/>\n"
                    "    <when condition=\"1\"/>\n"
                    "    <when condition=\"2\"/>\n"
                    "    <revision date=\"2016-02-16\"/>\n"
                    "    <revision date=\"2016-02-17\"/>\n"
                    "  </e:complex-arrays>\n"
                    "  <feature name=\"f1\"/>\n"
                    "  <feature name=\"f2\"/>\n"
                    "</module>\n";

    mod = lys_parse_mem(st->ctx, yin, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YIN, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yin);
}

static void
test_complex_arrays_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yang = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex-arrays {\n"
                    "    argument b;\n"
                    "    argument a {\n      yin-element true;\n    }\n"
                    "    base a;\n"
                    "    base b;\n"
                    "    belongs-to test1 {\n      prefix t1;\n    }\n"
                    "    belongs-to test2 {\n      prefix t2;\n    }\n"
                    "    contact\n      \"contact1\";\n"
                    "    contact\n      \"contact2\";\n"
                    "    default \"a\";\n"
                    "    default \"b\";\n"
                    "    description\n      \"description1\";\n"
                    "    description\n      \"description2\";\n"
                    "    error-app-tag \"a\";\n"
                    "    error-app-tag \"b\";\n"
                    "    error-message\n      \"errmsg1\";\n"
                    "    error-message\n      \"errmsg2\";\n"
                    "    key \"a\";\n"
                    "    key \"b\";\n"
                    "    namespace \"urn1\";\n"
                    "    namespace \"urn2\";\n"
                    "    organization\n      \"org1\";\n"
                    "    organization\n      \"org2\";\n"
                    "    path \"a\";\n"
                    "    path \"b\";\n"
                    "    prefix a;\n"
                    "    prefix b;\n"
                    "    presence \"a\";\n"
                    "    presence \"b\";\n"
                    "    reference\n      \"reference1\";\n"
                    "    reference\n      \"reference2\";\n"
                    "    units \"a\";\n"
                    "    units \"b\";\n"
                    "    fraction-digits 5;\n"
                    "    fraction-digits 10;\n"
                    "    max-elements 2;\n"
                    "    max-elements 3;\n"
                    "    min-elements 4;\n"
                    "    min-elements 5;\n"
                    "    position 11;\n"
                    "    position 12;\n"
                    "    value 42;\n"
                    "    value -55;\n"
                    "    unique \"l1 l2\";\n"
                    "    unique \"l2\";\n"
                    "    module inmod1 {\n"
                    "      namespace \"urn:inmod1\";\n"
                    "      prefix i;\n    }\n"
                    "    module inmod2 {\n"
                    "      namespace \"urn:inmod2\";\n"
                    "      prefix i;\n    }\n"
                    "    anydata anydata1;\n\n"
                    "    anydata anydata2;\n\n"
                    "    anyxml anyxml1;\n\n"
                    "    anyxml anyxml2;\n\n"
                    "    leaf l1 {\n      type string;\n    }\n\n"
                    "    leaf l2 {\n      type string;\n    }\n"
                    "    typedef mytype1 {\n      type string;\n    }\n"
                    "    typedef mytype2 {\n      type string;\n    }\n"
                    "    type string;\n"
                    "    type uint8;\n"
                    "    if-feature \"f1\";\n"
                    "    if-feature \"f2\";\n"
                    "    length \"10\";\n"
                    "    length \"20\";\n"
                    "    must \"1\";\n"
                    "    must \"2\";\n"
                    "    pattern \"[a-z]*\";\n"
                    "    pattern \"[A-Z]*\";\n"
                    "    range \"0..10\";\n"
                    "    range \"100..110\";\n"
                    "    when \"1\";\n"
                    "    when \"2\";\n"
                    "    revision 2016-02-16;\n"
                    "    revision 2016-02-17;\n  }\n\n"
                    "  feature f1;\n\n"
                    "  feature f2;\n}\n";

    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YANG, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yang);
}

static void
test_complex_mand_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin1 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex-mand>\n  </e:complex-mand>\n"
                    "</module>\n";
    const char *yin2 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex-mand>\n"
                    "    <description>\n      <text>description1</text>\n    </description>\n"
                    "  </e:complex-mand>\n"
                    "</module>\n";
    const char *yin3 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex-mand>\n"
                    "    <description>\n      <text>description1</text>\n    </description>\n"
                    "    <default value=\"b\"/>\n"
                    "  </e:complex-mand>\n"
                    "</module>\n";
    const char *yin4 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex-mand>\n"
                    "    <description>\n      <text>description1</text>\n    </description>\n"
                    "    <default value=\"b\"/>\n    <config value=\"true\"/>\n"
                    "  </e:complex-mand>\n"
                    "</module>\n";
    const char *yin5 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex-mand>\n"
                    "    <description>\n      <text>description1</text>\n    </description>\n"
                    "    <default value=\"b\"/>\n    <config value=\"true\"/>\n    <mandatory value=\"true\"/>\n"
                    "  </e:complex-mand>\n"
                    "</module>\n";
    const char *yin6 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex-mand>\n"
                    "    <description>\n      <text>description1</text>\n    </description>\n"
                    "    <default value=\"b\"/>\n    <config value=\"true\"/>\n    <mandatory value=\"true\"/>\n"
                    "    <status value=\"obsolete\"/>\n"
                    "  </e:complex-mand>\n"
                    "</module>\n";
    const char *yin7 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex-mand>\n"
                    "    <description>\n      <text>description1</text>\n    </description>\n"
                    "    <default value=\"b\"/>\n    <config value=\"true\"/>\n    <mandatory value=\"true\"/>\n"
                    "    <status value=\"obsolete\"/>\n    <fraction-digits value=\"5\"/>\n"
                    "  </e:complex-mand>\n"
                    "</module>\n";
    const char *yin8 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex-mand>\n"
                    "    <description>\n      <text>description1</text>\n    </description>\n"
                    "    <default value=\"b\"/>\n    <config value=\"true\"/>\n    <mandatory value=\"true\"/>\n"
                    "    <status value=\"obsolete\"/>\n    <fraction-digits value=\"5\"/>\n"
                    "    <min-elements value=\"4\"/>\n"
                    "  </e:complex-mand>\n"
                    "</module>\n";
    const char *yin9 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex-mand>\n"
                    "    <description>\n      <text>description1</text>\n    </description>\n"
                    "    <default value=\"b\"/>\n    <config value=\"true\"/>\n    <mandatory value=\"true\"/>\n"
                    "    <status value=\"obsolete\"/>\n    <fraction-digits value=\"5\"/>\n"
                    "    <min-elements value=\"4\"/>\n"
                    "    <leaf name=\"l1\">\n      <type name=\"string\"/>\n    </leaf>\n"
                    "  </e:complex-mand>\n"
                    "</module>\n";
    const char *yin_correct = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex-mand>\n"
                    "    <description>\n      <text>description1</text>\n    </description>\n"
                    "    <default value=\"b\"/>\n"
                    "    <config value=\"true\"/>\n"
                    "    <mandatory value=\"true\"/>\n"
                    "    <status value=\"obsolete\"/>\n"
                    "    <fraction-digits value=\"5\"/>\n"
                    "    <min-elements value=\"4\"/>\n"
                    "    <leaf name=\"l1\">\n      <type name=\"string\"/>\n    </leaf>\n"
                    "    <must condition=\"1\"/>\n"
                    "  </e:complex-mand>\n"
                    "</module>\n";

    mod = lys_parse_mem(st->ctx, yin1, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    mod = lys_parse_mem(st->ctx, yin2, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    mod = lys_parse_mem(st->ctx, yin3, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    mod = lys_parse_mem(st->ctx, yin4, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    mod = lys_parse_mem(st->ctx, yin5, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    mod = lys_parse_mem(st->ctx, yin6, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    mod = lys_parse_mem(st->ctx, yin7, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    mod = lys_parse_mem(st->ctx, yin8, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    mod = lys_parse_mem(st->ctx, yin9, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);

    mod = lys_parse_mem(st->ctx, yin_correct, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YIN, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yin_correct);
}

static void
test_complex_mand_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yang1 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex-mand;\n}\n";
    const char *yang2 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex-mand {\n"
                    "    description\n    \"description1\";\n"
                    "  }\n}\n";
    const char *yang3 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex-mand {\n"
                    "    description\n      \"description1\";\n"
                    "    default \"b\";\n"
                    "  }\n}\n";
    const char *yang4 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex-mand {\n"
                    "    description\n      \"description1\";\n"
                    "    default \"b\";\n    config true;\n"
                    "  }\n}\n";
    const char *yang5 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex-mand {\n"
                    "    description\n      \"description1\";\n"
                    "    default \"b\";\n    config true;\n    mandatory true;\n"
                    "  }\n}\n";
    const char *yang6 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex-mand {\n"
                    "    description\n      \"description1\";\n"
                    "    default \"b\";\n    config true;\n    mandatory true;\n"
                    "    status obsolete;\n"
                    "  }\n}\n";
    const char *yang7 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex-mand {\n"
                    "    description\n      \"description1\";\n"
                    "    default \"b\";\n    config true;\n    mandatory true;\n"
                    "    status obsolete;\n    fraction-digits 5;\n"
                    "  }\n}\n";
    const char *yang8 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex-mand {\n"
                    "    description\n      \"description1\";\n"
                    "    default \"b\";\n    config true;\n    mandatory true;\n"
                    "    status obsolete;\n    fraction-digits 5;\n    min-elements 4;\n"
                    "  }\n}\n";
    const char *yang9 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex-mand {\n"
                    "    description\n      \"description1\";\n"
                    "    default \"b\";\n    config true;\n    mandatory true;\n"
                    "    status obsolete;\n    fraction-digits 5;\n    min-elements 4;\n"
                    "    leaf l1 {\n      type string;\n    }\n"
                    "  }\n}\n";
    const char *yang_correct = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex-mand {\n"
                    "    description\n      \"description1\";\n"
                    "    default \"b\";\n    config true;\n    mandatory true;\n"
                    "    status obsolete;\n    fraction-digits 5;\n    min-elements 4;\n"
                    "    leaf l1 {\n      type string;\n    }\n"
                    "    must \"1\";\n"
                    "  }\n}\n";

    mod = lys_parse_mem(st->ctx, yang1, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    mod = lys_parse_mem(st->ctx, yang2, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    mod = lys_parse_mem(st->ctx, yang3, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    mod = lys_parse_mem(st->ctx, yang4, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    mod = lys_parse_mem(st->ctx, yang5, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    mod = lys_parse_mem(st->ctx, yang6, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    mod = lys_parse_mem(st->ctx, yang7, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    mod = lys_parse_mem(st->ctx, yang8, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    mod = lys_parse_mem(st->ctx, yang9, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);

    mod = lys_parse_mem(st->ctx, yang_correct, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YANG, NULL, 0, 0);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yang_correct);
}

static void
test_complex_many_instace_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin1 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex>\n"
                    "    <description>\n      <text>description1</text>\n    </description>\n"
                    "    <description>\n      <text>description2</text>\n    </description>\n"
                    "  </e:complex>\n"
                    "</module>\n";
    const char *yin2 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex>\n"
                    "    <reference>\n      <text>reference1</text>\n    </reference>\n"
                    "    <reference>\n      <text>reference1</text>\n    </reference>\n"
                    "  </e:complex>\n"
                    "</module>\n";
    const char *yin3 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex>\n"
                    "    <argument name=\"b\">\n      <yin-element value=\"true\"/>    </argument>\n"
                    "    <argument name=\"a\">\n      <yin-element value=\"false\"/>    </argument>\n"
                    "  </e:complex>\n"
                    "</module>\n";
    const char *yin4 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex>\n"
                    "    <belongs-to module=\"test1\">\n      <prefix value=\"t1\"/>\n    </belongs-to>\n"
                    "    <belongs-to module=\"test2\">\n      <prefix value=\"t2\"/>\n    </belongs-to>\n"
                    "  </e:complex>\n"
                    "</module>\n";
    const char *yin5 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex>\n"
                    "    <contact>\n      <text>contact1</text>\n    </contact>\n"
                    "    <contact>\n      <text>contact2</text>\n    </contact>\n"
                    "  </e:complex>\n"
                    "</module>\n";
    const char *yin6 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex>\n"
                    "  <default value=\"b\"/>\n"
                    "  <default value=\"a\"/>\n"
                    "  </e:complex>\n"
                    "</module>\n";
    const char *yin7 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex>\n"
                    "    <error-message>\n      <value>error-message1</value>\n    </error-message>\n"
                    "    <error-message>\n      <value>error-message2</value>\n    </error-message>\n"
                    "  </e:complex>\n"
                    "</module>\n";
    const char *yin8 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex>\n"
                    "    <error-app-tag value=\"a\"/>\n"
                    "    <error-app-tag value=\"b\"/>\n"
                    "  </e:complex>\n"
                    "</module>\n";
    const char *yin9 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex>\n"
                    "    <namespace uri=\"urn:namespace1\"/>\n"
                    "    <namespace uri=\"urn:namespace2\"/>\n"
                    "  </e:complex>\n"
                    "</module>\n";
    const char *yin10 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex>\n"
                    "    <organization>\n      <text>organization1</text>\n    </organization>\n"
                    "    <organization>\n      <text>organization1</text>\n    </organization>\n"
                    "  </e:complex>\n"
                    "</module>\n";
    const char *yin11 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex>\n"
                    "    <path value=\"leaf\"/>\n"
                    "    <path value=\"leaf1\"/>\n"
                    "  </e:complex>\n"
                    "</module>\n";
    const char *yin12 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex>\n"
                    "    <presence value=\"presence1\"/>\n"
                    "    <presence value=\"presence2\"/>\n"
                    "  </e:complex>\n"
                    "</module>\n";
    const char *yin13 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex>\n"
                    "    <prefix value=\"a\"/>\n"
                    "    <prefix value=\"b\"/>\n"
                    "  </e:complex>\n"
                    "</module>\n";
    const char *yin14 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex>\n"
                    "    <revision-date date=\"2017-02-22\"/>"
                    "    <revision-date date=\"2015-02-22\"/>"
                    "  </e:complex>\n"
                    "</module>\n";
    const char *yin15 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex>\n"
                    "    <units name=\"meter\"/>\n"
                    "    <units name=\"kilogram\"/>\n"
                    "  </e:complex>\n"
                    "</module>\n";
    const char *yin16 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex>\n"
                    "    <modifier value=\"invert-match\"/>\n"
                    "    <modifier value=\"invert-match\"/>\n"
                    "  </e:complex>\n"
                    "</module>\n";
    const char *yin17 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex>\n"
                    "    <value value=\"142\"/>"
                    "    <value value=\"1456\"/>"
                    "  </e:complex>\n"
                    "</module>\n";
    const char *yin18 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex>\n"
                    "    <require-instance value=\"true\"/>"
                    "    <require-instance value=\"true\"/>"
                    "  </e:complex>\n"
                    "</module>\n";
    const char *yin19 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex>\n"
                    "    <length value=\"5..20\"/>"
                    "    <length value=\"5..10\"/>"
                    "  </e:complex>\n"
                    "</module>\n";
    const char *yin20 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex>\n"
                    "    <pattern value=\"A-Za-z]+\"/>"
                    "    <pattern value=\"[A-Za-z]+[0-9]*\"/>"
                    "  </e:complex>\n"
                    "</module>\n";
    const char *yin21 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex>\n"
                    "    <must condition=\"leaf = true\"/>"
                    "    <must condition=\"leaf = false\"/>"
                    "    <leaf name=\"leaf\">\n      <type name=\"boolean\"/>\n    </leaf>\n"
                    "  </e:complex>\n"
                    "</module>\n";
    const char *yin22 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex>\n"
                    "    <range value=\"1..25\"/>"
                    "    <range value=\"25 | 40 .. 100\"/>"
                    "  </e:complex>\n"
                    "</module>\n";
    const char *yin23 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex>\n"
                    "    <when condition=\"leaf = true\"/>"
                    "    <when condition=\"leaf = false\"/>"
                    "    <leaf name=\"leaf\">\n      <type name=\"boolean\"/>\n    </leaf>\n"
                    "  </e:complex>\n"
                    "</module>\n";
    const char *yin24 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <e:complex>\n"
                    "    <revision date=\"2016-02-16\"/>\n"
                    "    <revision date=\"2016-02-17\"/>\n"
                    "  </e:complex>\n"
                    "</module>\n";

    mod = lys_parse_mem(st->ctx, yin1, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yin2, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yin3, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yin4, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yin5, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yin6, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yin7, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yin8, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yin9, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yin9, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yin10, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yin11, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yin12, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yin13, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yin14, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yin15, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yin16, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yin17, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yin18, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yin19, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yin20, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yin21, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yin22, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yin23, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yin24, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
}

static void
test_complex_many_instace_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yang1 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex {\n"
                    "    description\n    \"description1\";\n"
                    "    description\n    \"description2\";\n"
                    "  }\n}\n";
    const char *yang2 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex {\n"
                    "    reference\n    \"reference1\";\n"
                    "    reference\n    \"reference2\";\n"
                    "  }\n}\n";
    const char *yang3 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex {\n"
                    "    argument a {\n      yin-element true;\n    }\n"
                    "    argument b {\n      yin-element true;\n    }\n"
                    "  }\n}\n";
    const char *yang4 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex {\n"
                    "    belongs-to test1 {\n      prefix t1;\n    }\n"
                    "    belongs-to test2 {\n      prefix t2;\n    }\n"
                    "  }\n}\n";
    const char *yang5 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex {\n"
                    "    contact\n    \"contact1\";\n"
                    "    contact\n    \"contact2\";\n"
                    "  }\n}\n";
    const char *yang6 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex {\n"
                    "    default \"default1\";\n"
                    "    default \"default2\";\n"
                    "  }\n}\n";
    const char *yang7 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex {\n"
                    "    error-message \"err-msg1\";\n"
                    "    error-message \"err-msg2\";\n"
                    "  }\n}\n";
    const char *yang8 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex {\n"
                    "    key \"key1\";\n"
                    "    key \"key2\";\n"
                    "  }\n}\n";
    const char *yang9 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex {\n"
                    "    namespace \"namespace1\";\n"
                    "    namespace \"namespace2\";\n"
                    "  }\n}\n";
    const char *yang10 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex {\n"
                    "    organization \"organization1\";\n"
                    "    organization \"organization2\";\n"
                    "  }\n}\n";
    const char *yang11 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex {\n"
                    "    path \"leaf\";\n"
                    "    path \"leaf\";\n"
                    "    leaf leaf {\n      type string;\n     }\n"
                    "  }\n}\n";
    const char *yang12 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex {\n"
                    "    presence \"presence1\";\n"
                    "    presence \"presence2\";\n"
                    "  }\n}\n";
    const char *yang13 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex {\n"
                    "    prefix \"prefix1\";\n"
                    "    prefix \"prefix2\";\n"
                    "  }\n}\n";
    const char *yang14 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex {\n"
                    "    revision-date 2017-02-22;\n"
                    "    revision-date 2017-02-25;\n"
                    "  }\n}\n";
    const char *yang15 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex {\n"
                    "    units \"units1\";\n"
                    "    units \"units2\";\n"
                    "  }\n}\n";
    const char *yang16 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex {\n"
                    "    modifier invert-match;\n"
                    "    modifier invert-match;\n"
                    "  }\n}\n";
    const char *yang17 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex {\n"
                    "    value 142;\n"
                    "    value 1456;\n"
                    "  }\n}\n";
    const char *yang18 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex {\n"
                    "    require-instance \"true\";\n"
                    "    require-instance \"true\";\n"
                    "  }\n}\n";
    const char *yang19 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex {\n"
                    "    length \"5..20\";\n"
                    "    length \"5..10\";\n"
                    "  }\n}\n";
    const char *yang20 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex {\n"
                    "    pattern \"[A-Za-z]+\";\n"
                    "    pattern \"[A-Za-z]+[0-9]*\";\n"
                    "  }\n}\n";
    const char *yang21 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex {\n"
                    "    must \"leaf = true\";\n"
                    "    must \"leaf = false\";\n"
                    "    leaf leaf {\n      type string;\n     }\n"
                    "  }\n}\n";
    const char *yang22 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex {\n"
                    "    range \"1..25\";\n"
                    "    range \"25 | 40 .. 100\";\n"
                    "  }\n}\n";
    const char *yang23 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex {\n"
                    "    when \"leaf = true\";\n"
                    "    when \"leaf = false\";\n"
                    "    leaf leaf {\n      type string;\n     }\n"
                    "  }\n}\n";
    const char *yang24 = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  e:complex {\n"
                    "    revision 2017-03-30;\n"
                    "    revision 2016-03-30;\n"
                    "  }\n}\n";

    mod = lys_parse_mem(st->ctx, yang1, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yang2, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yang3, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yang4, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yang5, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yang6, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yang7, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yang8, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yang9, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yang9, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yang10, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yang11, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yang12, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yang13, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yang14, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yang15, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yang16, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yang17, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yang18, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yang19, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yang20, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yang21, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yang22, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yang23, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
    mod = lys_parse_mem(st->ctx, yang24, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
}

static void
test_complex_arrays_str_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin[17];
    int i;

    yin[0] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
             "<module name=\"ext1\"\n"
             "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
             "        xmlns:x=\"urn:ext1\"\n"
             "        xmlns:e=\"urn:ext-def\">\n"
             "  <yang-version value=\"1.1\"/>\n"
             "  <namespace uri=\"urn:ext1\"/>\n"
             "  <prefix value=\"x\"/>\n"
             "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
             "  <extension name=\"a\">\n    <argument name=\"y\"/>\n  </extension>\n"
             "  <e:complex-arrays>\n"
             "    <description>\n      <text>description1</text>\n    </description>\n"
             "    <description>\n"
             "      <x:a y=\"ok\"/>\n"
             "      <text>description1</text>\n"
             "    </description>\n"
             "    <description>\n      <text>description3</text>\n    </description>\n"
             "  </e:complex-arrays>\n"
             "</module>\n";
    yin[1] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
             "<module name=\"ext2\"\n"
             "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
             "        xmlns:x=\"urn:ext2\"\n"
             "        xmlns:e=\"urn:ext-def\">\n"
             "  <yang-version value=\"1.1\"/>\n"
             "  <namespace uri=\"urn:ext2\"/>\n"
             "  <prefix value=\"x\"/>\n"
             "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
             "  <extension name=\"a\">\n    <argument name=\"y\"/>\n  </extension>\n"
             "  <e:complex-arrays>\n"
             "    <reference>\n      <text>reference1</text>\n    </reference>\n"
             "    <reference>\n"
             "      <x:a y=\"ok\"/>\n"
             "      <text>reference1</text>\n"
             "    </reference>\n"
             "    <reference>\n      <text>reference3</text>\n    </reference>\n"
             "  </e:complex-arrays>\n"
             "</module>\n";
    yin[2] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
             "<module name=\"ext3\"\n"
             "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
             "        xmlns:x=\"urn:ext3\"\n"
             "        xmlns:e=\"urn:ext-def\">\n"
             "  <yang-version value=\"1.1\"/>\n"
             "  <namespace uri=\"urn:ext3\"/>\n"
             "  <prefix value=\"x\"/>\n"
             "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
             "  <extension name=\"a\">\n    <argument name=\"y\"/>\n  </extension>\n"
             "  <e:complex-arrays>\n"
             "    <contact>\n      <text>contact1</text>\n    </contact>\n"
             "    <contact>\n"
             "      <x:a y=\"ok\"/>\n"
             "      <text>contact1</text>\n"
             "    </contact>\n"
             "    <contact>\n      <text>contact3</text>\n    </contact>\n"
             "  </e:complex-arrays>\n"
             "</module>\n";
    yin[3] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
             "<module name=\"ext4\"\n"
             "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
             "        xmlns:x=\"urn:ext4\"\n"
             "        xmlns:e=\"urn:ext-def\">\n"
             "  <yang-version value=\"1.1\"/>\n"
             "  <namespace uri=\"urn:ext4\"/>\n"
             "  <prefix value=\"x\"/>\n"
             "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
             "  <extension name=\"a\">\n    <argument name=\"y\"/>\n  </extension>\n"
             "  <e:complex-arrays>\n"
             "    <default value=\"default1\"/>\n"
             "    <default value=\"default2\">\n"
             "      <x:a y=\"ok\"/>\n"
             "    </default>\n"
             "    <default value=\"default3\"/>\n"
             "  </e:complex-arrays>\n"
             "</module>\n";
    yin[4] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
             "<module name=\"ext5\"\n"
             "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
             "        xmlns:x=\"urn:ext5\"\n"
             "        xmlns:e=\"urn:ext-def\">\n"
             "  <yang-version value=\"1.1\"/>\n"
             "  <namespace uri=\"urn:ext5\"/>\n"
             "  <prefix value=\"x\"/>\n"
             "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
             "  <extension name=\"a\">\n    <argument name=\"y\"/>\n  </extension>\n"
             "  <e:complex-arrays>\n"
             "    <error-message>\n      <value>error-message1</value>\n    </error-message>\n"
             "    <error-message>\n"
             "      <x:a y=\"ok\"/>\n"
             "      <value>error-message1</value>\n"
             "    </error-message>\n"
             "    <error-message>\n      <value>error-message3</value>\n    </error-message>\n"
             "  </e:complex-arrays>\n"
             "</module>\n";
    yin[5] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
             "<module name=\"ext6\"\n"
             "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
             "        xmlns:x=\"urn:ext6\"\n"
             "        xmlns:e=\"urn:ext-def\">\n"
             "  <yang-version value=\"1.1\"/>\n"
             "  <namespace uri=\"urn:ext6\"/>\n"
             "  <prefix value=\"x\"/>\n"
             "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
             "  <extension name=\"a\">\n    <argument name=\"y\"/>\n  </extension>\n"
             "  <e:complex-arrays>\n"
             "    <error-app-tag value=\"error-app-tag1\"/>\n"
             "    <error-app-tag value=\"error-app-tag2\">\n"
             "      <x:a y=\"ok\"/>\n"
             "    </error-app-tag>\n"
             "    <error-app-tag value=\"error-app-tag3\"/>\n"
             "  </e:complex-arrays>\n"
             "</module>\n";
    yin[6] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
             "<module name=\"ext7\"\n"
             "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
             "        xmlns:x=\"urn:ext7\"\n"
             "        xmlns:e=\"urn:ext-def\">\n"
             "  <yang-version value=\"1.1\"/>\n"
             "  <namespace uri=\"urn:ext7\"/>\n"
             "  <prefix value=\"x\"/>\n"
             "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
             "  <extension name=\"a\">\n    <argument name=\"y\"/>\n  </extension>\n"
             "  <e:complex-arrays>\n"
             "    <namespace uri=\"urn:namespace1\"/>\n"
             "    <namespace uri=\"urn:namespace2\">\n"
             "      <x:a y=\"ok\"/>\n"
             "    </namespace>\n"
             "    <namespace uri=\"urn:namespace3\"/>\n"
             "  </e:complex-arrays>\n"
             "</module>\n";
    yin[7] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
             "<module name=\"ext8\"\n"
             "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
             "        xmlns:x=\"urn:ext8\"\n"
             "        xmlns:e=\"urn:ext-def\">\n"
             "  <yang-version value=\"1.1\"/>\n"
             "  <namespace uri=\"urn:ext8\"/>\n"
             "  <prefix value=\"x\"/>\n"
             "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
             "  <extension name=\"a\">\n    <argument name=\"y\"/>\n  </extension>\n"
             "  <e:complex-arrays>\n"
             "    <organization>\n      <text>organization1</text>\n    </organization>\n"
             "    <organization>\n"
             "      <x:a y=\"ok\"/>\n"
             "      <text>organization1</text>\n"
             "    </organization>\n"
             "    <organization>\n      <text>organization3</text>\n    </organization>\n"
             "  </e:complex-arrays>\n"
             "</module>\n";
    yin[8] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
             "<module name=\"ext9\"\n"
             "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
             "        xmlns:x=\"urn:ext9\"\n"
             "        xmlns:e=\"urn:ext-def\">\n"
             "  <yang-version value=\"1.1\"/>\n"
             "  <namespace uri=\"urn:ext9\"/>\n"
             "  <prefix value=\"x\"/>\n"
             "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
             "  <extension name=\"a\">\n    <argument name=\"y\"/>\n  </extension>\n"
             "  <e:complex-arrays>\n"
             "    <path value=\"path1\"/>\n"
             "    <path value=\"path2\">\n"
             "      <x:a y=\"ok\"/>\n"
             "    </path>\n"
             "    <path value=\"path3\"/>\n"
             "  </e:complex-arrays>\n"
             "</module>\n";
    yin[9] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
             "<module name=\"ext10\"\n"
             "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
             "        xmlns:x=\"urn:ext10\"\n"
             "        xmlns:e=\"urn:ext-def\">\n"
             "  <yang-version value=\"1.1\"/>\n"
             "  <namespace uri=\"urn:ext10\"/>\n"
             "  <prefix value=\"x\"/>\n"
             "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
             "  <extension name=\"a\">\n    <argument name=\"y\"/>\n  </extension>\n"
             "  <e:complex-arrays>\n"
             "    <prefix value=\"prefix1\"/>\n"
             "    <prefix value=\"prefix2\">\n"
             "      <x:a y=\"ok\"/>\n"
             "    </prefix>\n"
             "    <prefix value=\"prefix3\"/>\n"
             "  </e:complex-arrays>\n"
             "</module>\n";
    yin[10] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
             "<module name=\"ext11\"\n"
             "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
             "        xmlns:x=\"urn:ext11\"\n"
             "        xmlns:e=\"urn:ext-def\">\n"
             "  <yang-version value=\"1.1\"/>\n"
             "  <namespace uri=\"urn:ext11\"/>\n"
             "  <prefix value=\"x\"/>\n"
             "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
             "  <extension name=\"a\">\n    <argument name=\"y\"/>\n  </extension>\n"
             "  <e:complex-arrays>\n"
             "    <presence value=\"presence1\"/>\n"
             "    <presence value=\"presence2\">\n"
             "      <x:a y=\"ok\"/>\n"
             "    </presence>\n"
             "    <presence value=\"presence3\"/>\n"
             "  </e:complex-arrays>\n"
             "</module>\n";
    yin[11] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
             "<module name=\"ext12\"\n"
             "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
             "        xmlns:x=\"urn:ext12\"\n"
             "        xmlns:e=\"urn:ext-def\">\n"
             "  <yang-version value=\"1.1\"/>\n"
             "  <namespace uri=\"urn:ext12\"/>\n"
             "  <prefix value=\"x\"/>\n"
             "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
             "  <extension name=\"a\">\n    <argument name=\"y\"/>\n  </extension>\n"
             "  <e:complex-arrays>\n"
             "    <units name=\"units1\"/>\n"
             "    <units name=\"units2\">\n"
             "      <x:a y=\"ok\"/>\n"
             "    </units>\n"
             "    <units name=\"units3\"/>\n"
             "  </e:complex-arrays>\n"
             "</module>\n";
    yin[12] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
             "<module name=\"ext13\"\n"
             "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
             "        xmlns:x=\"urn:ext13\"\n"
             "        xmlns:e=\"urn:ext-def\">\n"
             "  <yang-version value=\"1.1\"/>\n"
             "  <namespace uri=\"urn:ext13\"/>\n"
             "  <prefix value=\"x\"/>\n"
             "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
             "  <extension name=\"a\">\n    <argument name=\"y\"/>\n  </extension>\n"
             "  <e:complex-arrays>\n"
             "    <revision-date date=\"2017-02-20\"/>\n"
             "    <revision-date date=\"2017-02-25\">\n"
             "      <x:a y=\"ok\"/>\n"
             "    </revision-date>\n"
             "    <revision-date date=\"2017-02-28\"/>\n"
             "  </e:complex-arrays>\n"
             "</module>\n";
    yin[13] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
             "<module name=\"ext14\"\n"
             "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
             "        xmlns:x=\"urn:ext14\"\n"
             "        xmlns:e=\"urn:ext-def\">\n"
             "  <yang-version value=\"1.1\"/>\n"
             "  <namespace uri=\"urn:ext14\"/>\n"
             "  <prefix value=\"x\"/>\n"
             "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
             "  <extension name=\"a\">\n    <argument name=\"y\"/>\n  </extension>\n"
             "  <e:complex-arrays>\n"
             "    <base name=\"base1\"/>\n"
             "    <base name=\"base2\">\n"
             "      <x:a y=\"ok\"/>\n"
             "    </base>\n"
             "    <base name=\"base3\"/>\n"
             "  </e:complex-arrays>\n"
             "</module>\n";
    yin[14] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
             "<module name=\"ext15\"\n"
             "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
             "        xmlns:x=\"urn:ext15\"\n"
             "        xmlns:e=\"urn:ext-def\">\n"
             "  <yang-version value=\"1.1\"/>\n"
             "  <namespace uri=\"urn:ext15\"/>\n"
             "  <prefix value=\"x\"/>\n"
             "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
             "  <extension name=\"a\">\n    <argument name=\"y\"/>\n  </extension>\n"
             "  <extension name=\"b\"/>\n"
             "  <e:complex-arrays>\n"
             "    <argument name=\"argument1\"/>\n"
             "    <argument name=\"argument2\">\n"
             "      <x:a y=\"ok\"/>\n"
             "    </argument>\n"
             "    <argument name=\"argument3\">\n"
             "      <yin-element value=\"true\">\n"
             "        <x:b/>\n"
             "      </yin-element>\n"
             "    </argument>\n"
             "    <argument name=\"argument4\">\n"
             "      <yin-element value=\"false\">\n"
             "        <x:a y=\"value\"/>\n"
             "      </yin-element>\n"
             "    </argument>\n"
             "  </e:complex-arrays>\n"
             "</module>\n";
    yin[15] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
             "<module name=\"ext16\"\n"
             "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
             "        xmlns:x=\"urn:ext16\"\n"
             "        xmlns:e=\"urn:ext-def\">\n"
             "  <yang-version value=\"1.1\"/>\n"
             "  <namespace uri=\"urn:ext16\"/>\n"
             "  <prefix value=\"x\"/>\n"
             "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
             "  <extension name=\"a\">\n    <argument name=\"y\"/>\n  </extension>\n"
             "  <e:complex-arrays>\n"
             "    <belongs-to module=\"mod1\">\n      <prefix value=\"prefix1\"/>\n    </belongs-to>\n"
             "    <belongs-to module=\"mod2\">\n"
             "      <prefix value=\"prefix2\">\n        <x:a y=\"value\"/>\n      </prefix>\n"
             "    </belongs-to>\n"
             "    <belongs-to module=\"mod3\">\n"
             "      <x:a y=\"ok\"/>\n"
             "      <prefix value=\"prefix3\"/>\n"
             "    </belongs-to>\n"
             "  </e:complex-arrays>\n"
             "</module>\n";
    yin[16] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
             "<module name=\"ext17\"\n"
             "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
             "        xmlns:x=\"urn:ext17\"\n"
             "        xmlns:e=\"urn:ext-def\">\n"
             "  <yang-version value=\"1.1\"/>\n"
             "  <namespace uri=\"urn:ext17\"/>\n"
             "  <prefix value=\"x\"/>\n"
             "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
             "  <extension name=\"a\">\n    <argument name=\"y\"/>\n  </extension>\n"
             "  <e:complex-arrays>\n"
             "    <key value=\"key1\"/>\n"
             "    <key value=\"key2\">\n"
             "      <x:a y=\"ok\"/>\n"
             "    </key>\n"
             "    <key value=\"key3\"/>\n"
             "  </e:complex-arrays>\n"
             "</module>\n";

    for (i = 0; i < 17; ++i) {
        printf("module ext%d ... ", i + 1);
        mod = lys_parse_mem(st->ctx, yin[i], LYS_IN_YIN);
        assert_ptr_not_equal(mod, NULL);

        lys_print_mem(&st->str1, mod, LYS_OUT_YIN, NULL, 0, 0);
        assert_ptr_not_equal(st->str1, NULL);
        assert_string_equal(st->str1, yin[i]);
        free(st->str1);
        st->str1 = NULL;
        printf("OK\n");
    }
}

static void
test_complex_arrays_str_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yang[17];
    int i;

    yang[0] = "module ext1 {\n"
              "  yang-version 1.1;\n"
              "  namespace \"urn:ext1\";\n"
              "  prefix x;\n\n"
              "  import ext-def {\n    prefix e;\n  }\n\n"
              "  extension a {\n"
              "    e:complex-arrays {\n"
              "      description\n        \"description1\";\n"
              "      description\n        \"description2\" {\n        x:a \"ok\";\n      }\n"
              "      description\n        \"description3\";\n"
              "    }\n    argument value;\n"
              "  }\n}\n";
    yang[1] = "module ext2 {\n"
              "  yang-version 1.1;\n"
              "  namespace \"urn:ext2\";\n"
              "  prefix x;\n\n"
              "  import ext-def {\n    prefix e;\n  }\n\n"
              "  extension a {\n"
              "    e:complex-arrays {\n"
              "      reference\n        \"reference1\";\n"
              "      reference\n        \"reference2\" {\n        x:a \"ok\";\n      }\n"
              "      reference\n        \"reference3\";\n"
              "    }\n    argument value;\n"
              "  }\n}\n";
    yang[2] =  "module ext3 {\n"
              "  yang-version 1.1;\n"
              "  namespace \"urn:ext3\";\n"
              "  prefix x;\n\n"
              "  import ext-def {\n    prefix e;\n  }\n\n"
              "  extension a {\n"
              "    e:complex-arrays {\n"
              "      contact\n        \"contact1\";\n"
              "      contact\n        \"contact2\" {\n        x:a \"ok\";\n      }\n"
              "      contact\n        \"contact3\";\n"
              "    }\n    argument value;\n"
              "  }\n}\n";
    yang[3] = "module ext4 {\n"
              "  yang-version 1.1;\n"
              "  namespace \"urn:ext4\";\n"
              "  prefix x;\n\n"
              "  import ext-def {\n    prefix e;\n  }\n\n"
              "  extension a {\n"
              "    e:complex-arrays {\n"
              "      default \"default1\";\n"
              "      default \"default2\" {\n        x:a \"ok\";\n      }\n"
              "      default \"default3\";\n"
              "    }\n    argument value;\n"
              "  }\n}\n";
    yang[4] =  "module ext5 {\n"
              "  yang-version 1.1;\n"
              "  namespace \"urn:ext5\";\n"
              "  prefix x;\n\n"
              "  import ext-def {\n    prefix e;\n  }\n\n"
              "  extension a {\n"
              "    e:complex-arrays {\n"
              "      error-message\n        \"error-message1\";\n"
              "      error-message\n        \"error-message2\" {\n        x:a \"ok\";\n      }\n"
              "      error-message\n        \"error-message3\";\n"
              "    }\n    argument value;\n"
              "  }\n}\n";
    yang[5] = "module ext6 {\n"
              "  yang-version 1.1;\n"
              "  namespace \"urn:ext6\";\n"
              "  prefix x;\n\n"
              "  import ext-def {\n    prefix e;\n  }\n\n"
              "  extension a {\n"
              "    e:complex-arrays {\n"
              "      error-app-tag \"error-app-tag1\";\n"
              "      error-app-tag \"error-app-tag2\" {\n        x:a \"ok\";\n      }\n"
              "      error-app-tag \"error-app-tag3\";\n"
              "    }\n    argument value;\n"
              "  }\n}\n";
    yang[6] = "module ext7 {\n"
              "  yang-version 1.1;\n"
              "  namespace \"urn:ext7\";\n"
              "  prefix x;\n\n"
              "  import ext-def {\n    prefix e;\n  }\n\n"
              "  extension a {\n"
              "    e:complex-arrays {\n"
              "      namespace \"urn:namespace1\";\n"
              "      namespace \"urn:namespace2\" {\n        x:a \"ok\";\n      }\n"
              "      namespace \"urn:namespace3\";\n"
              "    }\n    argument value;\n"
              "  }\n}\n";
    yang[7] = "module ext8 {\n"
              "  yang-version 1.1;\n"
              "  namespace \"urn:ext8\";\n"
              "  prefix x;\n\n"
              "  import ext-def {\n    prefix e;\n  }\n\n"
              "  extension a {\n"
              "    e:complex-arrays {\n"
              "      organization\n        \"organization1\";\n"
              "      organization\n        \"organization2\" {\n        x:a \"ok\";\n      }\n"
              "      organization\n        \"organization3\";\n"
              "    }\n    argument value;\n"
              "  }\n}\n";
    yang[8] = "module ext9 {\n"
              "  yang-version 1.1;\n"
              "  namespace \"urn:ext9\";\n"
              "  prefix x;\n\n"
              "  import ext-def {\n    prefix e;\n  }\n\n"
              "  extension a {\n"
              "    e:complex-arrays {\n"
              "      path \"path1\";\n"
              "      path \"path2\" {\n        x:a \"ok\";\n      }\n"
              "      path \"path3\";\n"
              "    }\n    argument value;\n"
              "  }\n}\n";
    yang[9] = "module ext10 {\n"
              "  yang-version 1.1;\n"
              "  namespace \"urn:ext10\";\n"
              "  prefix x;\n\n"
              "  import ext-def {\n    prefix e;\n  }\n\n"
              "  extension a {\n"
              "    e:complex-arrays {\n"
              "      prefix prefix1;\n"
              "      prefix prefix2 {\n        x:a \"ok\";\n      }\n"
              "      prefix prefix3;\n"
              "    }\n    argument value;\n"
              "  }\n}\n";
    yang[10] = "module ext11 {\n"
              "  yang-version 1.1;\n"
              "  namespace \"urn:ext11\";\n"
              "  prefix x;\n\n"
              "  import ext-def {\n    prefix e;\n  }\n\n"
              "  extension a {\n"
              "    e:complex-arrays {\n"
              "      presence \"presence1\";\n"
              "      presence \"presence2\" {\n        x:a \"ok\";\n      }\n"
              "      presence \"presence3\";\n"
              "    }\n    argument value;\n"
              "  }\n}\n";
    yang[11] = "module ext12 {\n"
              "  yang-version 1.1;\n"
              "  namespace \"urn:ext12\";\n"
              "  prefix x;\n\n"
              "  import ext-def {\n    prefix e;\n  }\n\n"
              "  extension a {\n"
              "    e:complex-arrays {\n"
              "      units \"units1\";\n"
              "      units \"units2\" {\n        x:a \"ok\";\n      }\n"
              "      units \"units3\";\n"
              "    }\n    argument value;\n"
              "  }\n}\n";
    yang[12] = "module ext13 {\n"
              "  yang-version 1.1;\n"
              "  namespace \"urn:ext13\";\n"
              "  prefix x;\n\n"
              "  import ext-def {\n    prefix e;\n  }\n\n"
              "  extension a {\n"
              "    e:complex-arrays {\n"
              "      revision-date 2017-02-20;\n"
              "      revision-date 2017-02-25 {\n        x:a \"ok\";\n      }\n"
              "      revision-date 2017-02-28;\n"
              "    }\n    argument value;\n"
              "  }\n}\n";
    yang[13] = "module ext14 {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext14\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  extension a {\n"
                    "    e:complex-arrays {\n"
                    "      base base1;\n"
                    "      base base2 {\n        x:a \"ok\";\n      }\n"
                    "      base base3;\n"
                    "    }\n    argument value;\n"
                    "  }\n}\n";
    yang[14] = "module ext15 {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext15\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  extension b;\n\n"
                    "  extension a {\n"
                    "    e:complex-arrays {\n"
                    "      argument argument1;\n"
                    "      argument argument2 {\n        x:a \"ok\";\n      }\n"
                    "      argument argument3 {\n"
                    "        yin-element true {\n          x:b;\n        }\n      }\n"
                    "      argument argument4 {\n"
                    "        yin-element false {\n          x:a \"value\";\n        }\n      }\n"
                    "    }\n    argument value;\n"
                    "  }\n}\n";
    yang[15] = "module ext16 {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext16\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  extension b;\n\n"
                    "  extension a {\n"
                    "    e:complex-arrays {\n"
                    "      belongs-to mod1 {\n"
                    "        prefix prefix1;\n      }\n"
                    "      belongs-to mod2 {\n"
                    "        prefix prefix2 {\n          x:a \"ok\";\n        }\n      }\n"
                    "      belongs-to mod3 {\n        x:b;\n"
                    "        prefix prefix3;\n      }\n"
                    "    }\n    argument value;\n"
                    "  }\n}\n";
    yang[16] = "module ext17 {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext17\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  extension a {\n"
                    "    e:complex-arrays {\n"
                    "      key \"key1\";\n"
                    "      key \"key2\" {\n        x:a \"ok\";\n      }\n"
                    "      key \"key3\";\n"
                    "    }\n    argument value;\n"
                    "  }\n}\n";

    for (i = 0; i < 17; ++i) {
        printf("module ext%d ... ", i + 1);
        mod = lys_parse_mem(st->ctx, yang[i], LYS_IN_YANG);
        assert_ptr_not_equal(mod, NULL);

        lys_print_mem(&st->str1, mod, LYS_OUT_YANG, NULL, 0, 0);
        assert_ptr_not_equal(st->str1, NULL);
        assert_string_equal(st->str1, yang[i]);
        free(st->str1);
        st->str1 = NULL;
        printf("OK\n");
    }
}

void
test_extension_yang_data_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin[8];
    int i;

    yin[0] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"ext1\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:rc=\"urn:ietf:params:xml:ns:yang:ietf-restconf\">\n"
            "  <yang-version value=\"1.1\"/>\n"
            "  <namespace uri=\"urn:ext1\"/>\n"
            "  <prefix value=\"x\"/>\n"
            "  <import module=\"ietf-restconf\">\n    <prefix value=\"rc\"/>\n  </import>\n"
            "  <rc:yang-data>\n"
            "    <rc:name>gg</rc:name>\n"
            "    <choice name=\"hh\"/>\n"
            "  </rc:yang-data>\n"
            "</module>\n";
    yin[1] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"ext2\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:rc=\"urn:ietf:params:xml:ns:yang:ietf-restconf\">\n"
            "  <yang-version value=\"1.1\"/>\n"
            "  <namespace uri=\"urn:ext2\"/>\n"
            "  <prefix value=\"x\"/>\n"
            "  <import module=\"ietf-restconf\">\n    <prefix value=\"rc\"/>\n  </import>\n"
            "  <rc:yang-data>\n"
            "    <rc:name>gg</rc:name>\n"
            "    <choice name=\"hh\">\n"
            "      <leaf name=\"str\">"
            "        <type name=\"string\"/>\n"
            "      </leaf>\n"
            "    </choice>\n"
            "  </rc:yang-data>\n"
            "</module>\n";
    yin[2] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"ext3\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:rc=\"urn:ietf:params:xml:ns:yang:ietf-restconf\">\n"
            "  <yang-version value=\"1.1\"/>\n"
            "  <namespace uri=\"urn:ext3\"/>\n"
            "  <prefix value=\"x\"/>\n"
            "  <import module=\"ietf-restconf\">\n    <prefix value=\"rc\"/>\n  </import>\n"
            "  <rc:yang-data>\n"
            "    <rc:name>gg</rc:name>\n"
            "    <leaf name=\"str\">"
            "      <type name=\"string\"/>\n"
            "    </leaf>\n"
            "  </rc:yang-data>\n"
            "</module>\n";
    yin[3] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"ext4\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:rc=\"urn:ietf:params:xml:ns:yang:ietf-restconf\">\n"
            "  <yang-version value=\"1.1\"/>\n"
            "  <namespace uri=\"urn:ext4\"/>\n"
            "  <prefix value=\"x\"/>\n"
            "  <import module=\"ietf-restconf\">\n    <prefix value=\"rc\"/>\n  </import>\n"
            "  <rc:yang-data>\n"
            "    <rc:name>gg</rc:name>\n"
            "    <container name=\"abc\"/>"
            "    <container name=\"str\">"
            "      <presence value=\"enable\"/>\n"
            "    </container>\n"
            "  </rc:yang-data>\n"
            "</module>\n";
    yin[4] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"ext5\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:rc=\"urn:ietf:params:xml:ns:yang:ietf-restconf\">\n"
            "  <yang-version value=\"1.1\"/>\n"
            "  <namespace uri=\"urn:ext5\"/>\n"
            "  <prefix value=\"x\"/>\n"
            "  <import module=\"ietf-restconf\">\n    <prefix value=\"rc\"/>\n  </import>\n"
            "  <container name=\"abc\">\n"
            "    <rc:yang-data>\n"
            "      <rc:name>gg</rc:name>\n"
            "      <container name=\"str\">"
            "        <presence value=\"enable\"/>\n"
            "      </container>\n"
            "    </rc:yang-data>\n"
            "  </container>\n"
            "</module>\n";
    yin[5] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"ext6\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:rc=\"urn:ietf:params:xml:ns:yang:ietf-restconf\">\n"
            "  <yang-version value=\"1.1\"/>\n"
            "  <namespace uri=\"urn:ext6\"/>\n"
            "  <prefix value=\"x\"/>\n"
            "  <import module=\"ietf-restconf\">\n    <prefix value=\"rc\"/>\n  </import>\n"
            "  <rc:yang-data>\n"
            "    <rc:name>gg</rc:name>\n"
            "    <container name=\"str\">"
            "      <presence value=\"enable\"/>\n"
            "      <leaf name=\"str\">"
            "        <type name=\"string\"/>\n"
            "      </leaf>\n"
            "    </container>\n"
            "  </rc:yang-data>\n"
            "</module>\n";
    yin[6] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"ext7\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:rc=\"urn:ietf:params:xml:ns:yang:ietf-restconf\">\n"
            "  <yang-version value=\"1.1\"/>\n"
            "  <namespace uri=\"urn:ext7\"/>\n"
            "  <prefix value=\"x\"/>\n"
            "  <import module=\"ietf-restconf\">\n    <prefix value=\"rc\"/>\n  </import>\n"
            "  <rc:yang-data>\n"
            "    <rc:name>gg</rc:name>\n"
            "    <uses name=\"rc:restconf\"/>"
            "  </rc:yang-data>\n"
            "</module>\n";
    yin[7] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"ext8\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:rc=\"urn:ietf:params:xml:ns:yang:ietf-restconf\">\n"
            "  <yang-version value=\"1.1\"/>\n"
            "  <namespace uri=\"urn:ext8\"/>\n"
            "  <prefix value=\"x\"/>\n"
            "  <import module=\"ietf-restconf\">\n    <prefix value=\"rc\"/>\n  </import>\n"
            "  <rc:yang-data>\n"
            "    <rc:name>gg</rc:name>\n"
            "    <choice name=\"hh\">\n"
            "      <case name=\"cs\">"
            "        <uses name=\"rc:restconf\"/>"
            "      </case>"
            "      <container name=\"enableSSH\">"
            "        <presence value=\"enable ssh\"/>\n"
            "      </container>\n"
            "    </choice>\n"
            "  </rc:yang-data>\n"
            "</module>\n";

    for(i = 0; i < 5; ++i) {
        printf("module ext%d ... ", i + 1);
        mod = lys_parse_mem(st->ctx, yin[i], LYS_IN_YIN);
        assert_ptr_equal(mod, NULL);
        printf("OK\n");
    }

    for(i = 5; i < 8; ++i) {
        printf("module ext%d ... ", i + 1);
        mod = lys_parse_mem(st->ctx, yin[i], LYS_IN_YIN);
        assert_ptr_not_equal(mod, NULL);
        printf("OK\n");
    }
}

void
test_extension_yang_data_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yang[8];
    int i;

    yang[0] = "module ext1 {\n"
            "  prefix f;\n"
            "  namespace \"urn:cesnet\";\n"
            "  import ietf-restconf {\n"
            "    prefix rc;\n"
            "  }"
            "  rc:yang-data data {\n"
            "    choice hh {\n"
            "    }\n"
            "  }\n"
            "}";
    yang[1] = "module ext2 {\n"
            "  prefix f;\n"
            "  namespace \"urn:cesnet\";\n"
            "  import ietf-restconf {\n"
            "    prefix rc;\n"
            "  }"
            "  rc:yang-data data {\n"
            "    choice hh {\n"
            "      leaf str {\n"
            "        type string;\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "}";
    yang[2] = "module ext3 {\n"
            "  prefix f;\n"
            "  namespace \"urn:cesnet\";\n"
            "  import ietf-restconf {\n"
            "    prefix rc;\n"
            "  }"
            "  rc:yang-data data {\n"
            "    leaf str {\n"
            "      type string;\n"
            "    }\n"
            "  }\n"
            "}";
    yang[3] = "module ext4 {\n"
            "  prefix f;\n"
            "  namespace \"urn:cesnet\";\n"
            "  import ietf-restconf {\n"
            "    prefix rc;\n"
            "  }"
            "  rc:yang-data data {\n"
            "    container abc;\n"
            "    container str  {\n"
            "      presence \"enable\";\n"
            "    }\n"
            "  }\n"
            "}";
    yang[4] = "module ext5 {\n"
            "  prefix f;\n"
            "  namespace \"urn:cesnet\";\n"
            "  import ietf-restconf {\n"
            "    prefix rc;\n"
            "  }"
            "  container abc {\n"
            "    rc:yang-data data {\n"
            "      container str  {\n"
            "        presence \"enable\";\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "}";
    yang[5] = "module ext6 {\n"
            "  prefix f6;\n"
            "  namespace \"urn:cesnet:ext5\";\n"
            "  import ietf-restconf {\n"
            "    prefix rc;\n"
            "  }"
            "  rc:yang-data data {\n"
            "    container str  {\n"
            "      presence \"enable\";\n"
            "      leaf str {\n"
            "        type string;\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "}";
    yang[6] = "module ext7 {\n"
            "  prefix f7;\n"
            "  namespace \"urn:cesnet:ext6\";\n"
            "  import ietf-restconf {\n"
            "    prefix rc;\n"
            "  }"
            "  rc:yang-data data {\n"
            "    uses rc:restconf;\n"
            "  }\n"
            "}";
    yang[7] = "module ext8 {\n"
            "  prefix f8;\n"
            "  namespace \"urn:cesnet:ext8\";\n"
            "  import ietf-restconf {\n"
            "    prefix rc;\n"
            "  }"
            "  rc:yang-data data {\n"
            "    choice hh {\n"
            "      case cs {\n"
            "        uses rc:errors;\n"
            "      }\n"
            "      container enableSSH {\n"
            "        presence \"enable ssh\";"
            "      }\n"
            "    }\n"
            "  }\n"
            "}";

    for(i = 0; i < 5; ++i) {
        printf("module ext%d ... ", i + 1);
        mod = lys_parse_mem(st->ctx, yang[i], LYS_IN_YANG);
        assert_ptr_equal(mod, NULL);
        printf("OK\n");
    }

    for(; i < 8; ++i) {
        printf("module ext%d ... ", i + 1);
        mod = lys_parse_mem(st->ctx, yang[i], LYS_IN_YANG);
        assert_ptr_not_equal(mod, NULL);
        printf("OK\n");
    }
}

int
main(void)
{
    const struct CMUnitTest cmut[] = {
        cmocka_unit_test_setup_teardown(test_module_sub_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_container_sub_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_leaf_sub_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_leaflist_sub_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_list_sub_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_anydata_sub_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_choice_sub_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_uses_sub_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_extension_sub_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_rpc_sub_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_notif_sub_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_deviation_sub_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_complex_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_complex_arrays_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_complex_mand_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_complex_many_instace_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_complex_arrays_str_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_extension_yang_data_yin, setup_ctx_yin, teardown_ctx),

        cmocka_unit_test_setup_teardown(test_module_sub_yang, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_container_sub_yang, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_leaf_sub_yang, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_leaflist_sub_yang, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_list_sub_yang, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_anydata_sub_yang, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_choice_sub_yang, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_uses_sub_yang, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_extension_sub_yang, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_rpc_sub_yang, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_notif_sub_yang, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_deviation_sub_yang, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_complex_yang, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_complex_arrays_yang, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_complex_mand_yang, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_complex_many_instace_yang, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_complex_arrays_str_yang, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_extension_yang_data_yang, setup_ctx_yang, teardown_ctx)
    };

    return cmocka_run_group_tests(cmut, NULL, NULL);
}
