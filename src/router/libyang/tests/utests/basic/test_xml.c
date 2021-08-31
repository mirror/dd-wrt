/*
 * @file test_xml.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from xml.c
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _UTEST_MAIN_
#include "utests.h"

#define _POSIX_C_SOURCE 200809L /* strdup */

#include <string.h>

#include "context.h"
#include "in_internal.h"
#include "xml.h"

LY_ERR lyxml_ns_add(struct lyxml_ctx *xmlctx, const char *prefix, size_t prefix_len, char *uri);
LY_ERR lyxml_ns_rm(struct lyxml_ctx *xmlctx);

static void
test_element(void **state)
{
    struct lyxml_ctx *xmlctx;
    struct ly_in *in;
    const char *str;

    /* empty */
    str = "";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_SUCCESS, lyxml_ctx_new(UTEST_LYCTX, in, &xmlctx));
    assert_int_equal(LYXML_END, xmlctx->status);
    lyxml_ctx_free(xmlctx);
    ly_in_free(in, 0);

    /* end element */
    str = "</element>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_EVALID, lyxml_ctx_new(UTEST_LYCTX, in, &xmlctx));
    CHECK_LOG_CTX("Stray closing element tag (\"element\").", "Line number 1.");
    ly_in_free(in, 0);

    /* no element */
    UTEST_LOG_CLEAN;
    str = "no data present";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_EVALID, lyxml_ctx_new(UTEST_LYCTX, in, &xmlctx));
    CHECK_LOG_CTX("Invalid character sequence \"no data present\", expected element tag start ('<').", "Line number 1.");
    ly_in_free(in, 0);

    /* not supported DOCTYPE */
    str = "<!DOCTYPE greeting SYSTEM \"hello.dtd\"><greeting/>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_EVALID, lyxml_ctx_new(UTEST_LYCTX, in, &xmlctx));
    CHECK_LOG_CTX("Document Type Declaration not supported.", "Line number 1.");
    ly_in_free(in, 0);

    /* invalid XML */
    str = "<!NONSENSE/>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_EVALID, lyxml_ctx_new(UTEST_LYCTX, in, &xmlctx));
    CHECK_LOG_CTX("Unknown XML section \"<!NONSENSE/>\".", "Line number 1.");
    ly_in_free(in, 0);

    /* unqualified element */
    str = "  <  element/>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_SUCCESS, lyxml_ctx_new(UTEST_LYCTX, in, &xmlctx));
    assert_int_equal(LYXML_ELEMENT, xmlctx->status);
    assert_null(xmlctx->prefix);
    assert_true(!strncmp("element", xmlctx->name, xmlctx->name_len));
    assert_int_equal(1, xmlctx->elements.count);

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ELEM_CONTENT, xmlctx->status);
    assert_true(!strncmp("", xmlctx->value, xmlctx->value_len));

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ELEM_CLOSE, xmlctx->status);

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_END, xmlctx->status);
    lyxml_ctx_free(xmlctx);
    ly_in_free(in, 0);

    /* element with attribute */
    str = "  <  element attr=\'x\'/>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_SUCCESS, lyxml_ctx_new(UTEST_LYCTX, in, &xmlctx));
    assert_int_equal(LYXML_ELEMENT, xmlctx->status);
    assert_true(!strncmp("element", xmlctx->name, xmlctx->name_len));
    assert_null(xmlctx->prefix);
    assert_int_equal(1, xmlctx->elements.count);

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ATTRIBUTE, xmlctx->status);
    assert_true(!strncmp("attr", xmlctx->name, xmlctx->name_len));
    assert_null(xmlctx->prefix);

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ATTR_CONTENT, xmlctx->status);
    assert_int_equal(1, xmlctx->elements.count);
    assert_true(!strncmp("x", xmlctx->value, xmlctx->value_len));

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ELEM_CONTENT, xmlctx->status);

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ELEM_CLOSE, xmlctx->status);
    assert_int_equal(0, xmlctx->elements.count);

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_END, xmlctx->status);
    lyxml_ctx_free(xmlctx);
    ly_in_free(in, 0);

    /* headers and comments */
    str = "<?xml version=\"1.0\"?>  <!-- comment --> <![CDATA[<greeting>Hello, world!</greeting>]]> <?TEST xxx?> <element/>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_SUCCESS, lyxml_ctx_new(UTEST_LYCTX, in, &xmlctx));
    assert_int_equal(LYXML_ELEMENT, xmlctx->status);
    assert_true(!strncmp("element", xmlctx->name, xmlctx->name_len));
    assert_null(xmlctx->prefix);
    assert_int_equal(1, xmlctx->elements.count);

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ELEM_CONTENT, xmlctx->status);

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ELEM_CLOSE, xmlctx->status);

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_END, xmlctx->status);
    lyxml_ctx_free(xmlctx);
    ly_in_free(in, 0);

    /* separate opening and closing tags, neamespaced parsed internally */
    str = "<element xmlns=\"urn\"></element>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_SUCCESS, lyxml_ctx_new(UTEST_LYCTX, in, &xmlctx));
    assert_int_equal(LYXML_ELEMENT, xmlctx->status);
    assert_true(!strncmp("element", xmlctx->name, xmlctx->name_len));
    assert_null(xmlctx->prefix);
    assert_int_equal(1, xmlctx->elements.count);
    assert_int_equal(1, xmlctx->ns.count);

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ELEM_CONTENT, xmlctx->status);

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ELEM_CLOSE, xmlctx->status);
    assert_int_equal(0, xmlctx->elements.count);
    assert_int_equal(0, xmlctx->ns.count);

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_END, xmlctx->status);
    lyxml_ctx_free(xmlctx);
    ly_in_free(in, 0);

    /* qualified element */
    str = "  <  yin:element/>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_SUCCESS, lyxml_ctx_new(UTEST_LYCTX, in, &xmlctx));
    assert_int_equal(LYXML_ELEMENT, xmlctx->status);
    assert_true(!strncmp("element", xmlctx->name, xmlctx->name_len));
    assert_true(!strncmp("yin", xmlctx->prefix, xmlctx->prefix_len));

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ELEM_CONTENT, xmlctx->status);

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ELEM_CLOSE, xmlctx->status);

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_END, xmlctx->status);
    lyxml_ctx_free(xmlctx);
    ly_in_free(in, 0);

    /* non-matching closing tag */
    str = "<yin:element xmlns=\"urn\"></element>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_SUCCESS, lyxml_ctx_new(UTEST_LYCTX, in, &xmlctx));
    assert_int_equal(LYXML_ELEMENT, xmlctx->status);
    assert_true(!strncmp("element", xmlctx->name, xmlctx->name_len));
    assert_true(!strncmp("yin", xmlctx->prefix, xmlctx->prefix_len));
    assert_int_equal(1, xmlctx->elements.count);
    assert_int_equal(1, xmlctx->ns.count);

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ELEM_CONTENT, xmlctx->status);

    assert_int_equal(LY_EVALID, lyxml_ctx_next(xmlctx));
    CHECK_LOG_CTX("Opening (\"yin:element\") and closing (\"element\") elements tag mismatch.", "Line number 1.");
    lyxml_ctx_free(xmlctx);
    ly_in_free(in, 0);

    /* invalid closing tag */
    str = "<yin:element xmlns=\"urn\"></yin:element/>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_SUCCESS, lyxml_ctx_new(UTEST_LYCTX, in, &xmlctx));
    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LY_EVALID, lyxml_ctx_next(xmlctx));
    CHECK_LOG_CTX("Invalid character sequence \"/>\", expected element tag termination ('>').", "Line number 1.");
    lyxml_ctx_free(xmlctx);
    ly_in_free(in, 0);

    /* UTF8 characters */
    str = "<𠜎€𠜎Øn:𠜎€𠜎Øn/>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_SUCCESS, lyxml_ctx_new(UTEST_LYCTX, in, &xmlctx));
    assert_true(!strncmp("𠜎€𠜎Øn", xmlctx->name, xmlctx->name_len));
    assert_true(!strncmp("𠜎€𠜎Øn", xmlctx->prefix, xmlctx->prefix_len));

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ELEM_CONTENT, xmlctx->status);

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ELEM_CLOSE, xmlctx->status);

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_END, xmlctx->status);
    lyxml_ctx_free(xmlctx);
    ly_in_free(in, 0);

    /* invalid UTF-8 characters */
    str = "<¢:element>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_EVALID, lyxml_ctx_new(UTEST_LYCTX, in, &xmlctx));
    CHECK_LOG_CTX("Identifier \"¢:element>\" starts with an invalid character.", "Line number 1.");
    ly_in_free(in, 0);

    str = "<yin:c⁐element>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_SUCCESS, lyxml_ctx_new(UTEST_LYCTX, in, &xmlctx));
    assert_int_equal(LY_EVALID, lyxml_ctx_next(xmlctx));
    CHECK_LOG_CTX("Invalid character sequence \"⁐element>\", expected element tag end ('>' or '/>') or an attribute.", "Line number 1.");
    lyxml_ctx_free(xmlctx);
    ly_in_free(in, 0);

    /* mixed content */
    str = "<a>text <b>x</b></a>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_SUCCESS, lyxml_ctx_new(UTEST_LYCTX, in, &xmlctx));
    assert_int_equal(LYXML_ELEMENT, xmlctx->status);
    assert_true(!strncmp("a", xmlctx->name, xmlctx->name_len));
    assert_null(xmlctx->prefix);

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ELEM_CONTENT, xmlctx->status);
    assert_true(!strncmp("text ", xmlctx->value, xmlctx->value_len));

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ELEMENT, xmlctx->status);
    assert_true(!strncmp("b", xmlctx->name, xmlctx->name_len));
    assert_null(xmlctx->prefix);

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ELEM_CONTENT, xmlctx->status);
    assert_true(!strncmp("x", xmlctx->value, xmlctx->value_len));

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ELEM_CLOSE, xmlctx->status);

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ELEM_CLOSE, xmlctx->status);

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_END, xmlctx->status);
    lyxml_ctx_free(xmlctx);
    ly_in_free(in, 0);

    /* tag mismatch */
    str = "<a>text</b>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_SUCCESS, lyxml_ctx_new(UTEST_LYCTX, in, &xmlctx));
    assert_int_equal(LYXML_ELEMENT, xmlctx->status);
    assert_true(!strncmp("a", xmlctx->name, xmlctx->name_len));
    assert_null(xmlctx->prefix);

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ELEM_CONTENT, xmlctx->status);
    assert_true(!strncmp("text", xmlctx->value, xmlctx->value_len));

    assert_int_equal(LY_EVALID, lyxml_ctx_next(xmlctx));
    CHECK_LOG_CTX("Opening (\"a\") and closing (\"b\") elements tag mismatch.", "Line number 1.");
    lyxml_ctx_free(xmlctx);
    ly_in_free(in, 0);
}

static void
test_attribute(void **state)
{
    const char *str;
    struct lyxml_ctx *xmlctx;
    struct ly_in *in;
    struct lyxml_ns *ns;

    /* not an attribute */
    str = "<e unknown/>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_EVALID, lyxml_ctx_new(UTEST_LYCTX, in, &xmlctx));
    CHECK_LOG_CTX("Invalid character sequence \"/>\", expected '='.", "Line number 1.");
    ly_in_free(in, 0);

    str = "<e xxx=/>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_EVALID, lyxml_ctx_new(UTEST_LYCTX, in, &xmlctx));
    CHECK_LOG_CTX("Invalid character sequence \"/>\", expected either single or double quotation mark.", "Line number 1.");
    ly_in_free(in, 0);

    str = "<e xxx\n = yyy/>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_EVALID, lyxml_ctx_new(UTEST_LYCTX, in, &xmlctx));
    CHECK_LOG_CTX("Invalid character sequence \"yyy/>\", expected either single or double quotation mark.", "Line number 2.");
    ly_in_free(in, 0);

    /* valid attribute */
    str = "<e attr=\"val\"";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_SUCCESS, lyxml_ctx_new(UTEST_LYCTX, in, &xmlctx));
    assert_int_equal(LYXML_ELEMENT, xmlctx->status);
    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ATTRIBUTE, xmlctx->status);
    assert_true(!strncmp("attr", xmlctx->name, xmlctx->name_len));
    assert_null(xmlctx->prefix);

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ATTR_CONTENT, xmlctx->status);
    assert_true(!strncmp("val", xmlctx->value, xmlctx->value_len));
    assert_int_equal(xmlctx->ws_only, 0);
    assert_int_equal(xmlctx->dynamic, 0);
    lyxml_ctx_free(xmlctx);
    ly_in_free(in, 0);

    /* valid namespace with prefix */
    str = "<e xmlns:nc\n = \'urn\'/>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_SUCCESS, lyxml_ctx_new(UTEST_LYCTX, in, &xmlctx));
    assert_int_equal(LYXML_ELEMENT, xmlctx->status);
    assert_int_equal(1, xmlctx->ns.count);
    ns = (struct lyxml_ns *)xmlctx->ns.objs[0];
    assert_string_equal(ns->prefix, "nc");
    assert_string_equal(ns->uri, "urn");
    lyxml_ctx_free(xmlctx);
    ly_in_free(in, 0);
}

static void
test_text(void **state)
{
    const char *str;
    struct lyxml_ctx *xmlctx;
    struct ly_in *in;

    /* empty attribute value */
    str = "<e a=\"\"";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_SUCCESS, lyxml_ctx_new(UTEST_LYCTX, in, &xmlctx));
    assert_int_equal(LYXML_ELEMENT, xmlctx->status);
    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ATTRIBUTE, xmlctx->status);

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ATTR_CONTENT, xmlctx->status);
    assert_true(!strncmp("", xmlctx->value, xmlctx->value_len));
    assert_int_equal(xmlctx->ws_only, 1);
    assert_int_equal(xmlctx->dynamic, 0);
    ly_in_free(in, 0);

    /* empty value but in single quotes */
    assert_int_equal(LY_SUCCESS, ly_in_new_memory("=\'\'", &in));
    xmlctx->in = in;
    LOG_LOCINIT(NULL, NULL, NULL, in);
    xmlctx->status = LYXML_ATTRIBUTE;
    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ATTR_CONTENT, xmlctx->status);
    assert_true(!strncmp("", xmlctx->value, xmlctx->value_len));
    assert_int_equal(xmlctx->ws_only, 1);
    assert_int_equal(xmlctx->dynamic, 0);
    ly_in_free(in, 0);

    /* empty element content - only formating before defining child */
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(">\n  <y>", &in));
    xmlctx->in = in;
    LOG_LOCINIT(NULL, NULL, NULL, in);
    xmlctx->status = LYXML_ELEMENT;
    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ELEM_CONTENT, xmlctx->status);
    assert_true(!strncmp("\n  ", xmlctx->value, xmlctx->value_len));
    assert_int_equal(xmlctx->ws_only, 1);
    assert_int_equal(xmlctx->dynamic, 0);
    ly_in_free(in, 0);

    /* empty element content is invalid - missing content terminating character < */
    assert_int_equal(LY_SUCCESS, ly_in_new_memory("", &in));
    xmlctx->in = in;
    LOG_LOCINIT(NULL, NULL, NULL, in);
    xmlctx->status = LYXML_ELEM_CONTENT;
    assert_int_equal(LY_EVALID, lyxml_ctx_next(xmlctx));
    CHECK_LOG_CTX("Unexpected end-of-input.", "Line number 1.");
    ly_in_free(in, 0);

    assert_int_equal(LY_SUCCESS, ly_in_new_memory("xxx", &in));
    xmlctx->in = in;
    LOG_LOCINIT(NULL, NULL, NULL, in);
    xmlctx->status = LYXML_ELEM_CONTENT;
    assert_int_equal(LY_EVALID, lyxml_ctx_next(xmlctx));
    CHECK_LOG_CTX("Invalid character sequence \"xxx\", expected element tag start ('<').", "Line number 1.");
    ly_in_free(in, 0);

    lyxml_ctx_free(xmlctx);

    /* valid strings */
    str = "<a>€𠜎Øn \n&lt;&amp;&quot;&apos;&gt; &#82;&#x4f;&#x4B;</a>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_SUCCESS, lyxml_ctx_new(UTEST_LYCTX, in, &xmlctx));
    assert_int_equal(LYXML_ELEMENT, xmlctx->status);

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ELEM_CONTENT, xmlctx->status);
    assert_true(!strncmp("€𠜎Øn \n<&\"\'> ROK", xmlctx->value, xmlctx->value_len));
    assert_int_equal(xmlctx->ws_only, 0);
    assert_int_equal(xmlctx->dynamic, 1);
    free((char *)xmlctx->value);
    ly_in_free(in, 0);

    /* test using n-bytes UTF8 hexadecimal code points */
    assert_int_equal(LY_SUCCESS, ly_in_new_memory("=\'&#x0024;&#x00A2;&#x20ac;&#x10348;\'", &in));
    xmlctx->in = in;
    LOG_LOCINIT(NULL, NULL, NULL, in);
    xmlctx->status = LYXML_ATTRIBUTE;
    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ATTR_CONTENT, xmlctx->status);
    assert_true(!strncmp("$¢€𐍈", xmlctx->value, xmlctx->value_len));
    assert_int_equal(xmlctx->ws_only, 0);
    assert_int_equal(xmlctx->dynamic, 1);
    free((char *)xmlctx->value);
    ly_in_free(in, 0);

    /* invalid characters in string */
    assert_int_equal(LY_SUCCESS, ly_in_new_memory("=\'&#x52\'", &in));
    xmlctx->in = in;
    LOG_LOCINIT(NULL, NULL, NULL, in);
    xmlctx->status = LYXML_ATTRIBUTE;
    assert_int_equal(LY_EVALID, lyxml_ctx_next(xmlctx));
    CHECK_LOG_CTX("Invalid character sequence \"'\", expected ;.", "Line number 1.");
    ly_in_free(in, 0);

    assert_int_equal(LY_SUCCESS, ly_in_new_memory("=\"&#82\"", &in));
    xmlctx->in = in;
    LOG_LOCINIT(NULL, NULL, NULL, in);
    xmlctx->status = LYXML_ATTRIBUTE;
    assert_int_equal(LY_EVALID, lyxml_ctx_next(xmlctx));
    CHECK_LOG_CTX("Invalid character sequence \"\"\", expected ;.", "Line number 1.");
    ly_in_free(in, 0);

    assert_int_equal(LY_SUCCESS, ly_in_new_memory("=\"&nonsense;\"", &in));
    xmlctx->in = in;
    LOG_LOCINIT(NULL, NULL, NULL, in);
    xmlctx->status = LYXML_ATTRIBUTE;
    assert_int_equal(LY_EVALID, lyxml_ctx_next(xmlctx));
    CHECK_LOG_CTX("Entity reference \"&nonsense;\" not supported, only predefined references allowed.", "Line number 1.");
    ly_in_free(in, 0);

    assert_int_equal(LY_SUCCESS, ly_in_new_memory(">&#o122;", &in));
    xmlctx->in = in;
    LOG_LOCINIT(NULL, NULL, NULL, in);
    xmlctx->status = LYXML_ELEMENT;
    assert_int_equal(LY_EVALID, lyxml_ctx_next(xmlctx));
    CHECK_LOG_CTX("Invalid character reference \"&#o122;\".", "Line number 1.");
    ly_in_free(in, 0);

    assert_int_equal(LY_SUCCESS, ly_in_new_memory("=\'&#x06;\'", &in));
    xmlctx->in = in;
    LOG_LOCINIT(NULL, NULL, NULL, in);
    xmlctx->status = LYXML_ATTRIBUTE;
    assert_int_equal(LY_EVALID, lyxml_ctx_next(xmlctx));
    CHECK_LOG_CTX("Invalid character reference \"&#x06;\'\" (0x00000006).", "Line number 1.");
    ly_in_free(in, 0);

    assert_int_equal(LY_SUCCESS, ly_in_new_memory("=\'&#xfdd0;\'", &in));
    xmlctx->in = in;
    LOG_LOCINIT(NULL, NULL, NULL, in);
    xmlctx->status = LYXML_ATTRIBUTE;
    assert_int_equal(LY_EVALID, lyxml_ctx_next(xmlctx));
    CHECK_LOG_CTX("Invalid character reference \"&#xfdd0;\'\" (0x0000fdd0).", "Line number 1.");
    ly_in_free(in, 0);

    assert_int_equal(LY_SUCCESS, ly_in_new_memory("=\'&#xffff;\'", &in));
    xmlctx->in = in;
    LOG_LOCINIT(NULL, NULL, NULL, in);
    xmlctx->status = LYXML_ATTRIBUTE;
    assert_int_equal(LY_EVALID, lyxml_ctx_next(xmlctx));
    CHECK_LOG_CTX("Invalid character reference \"&#xffff;\'\" (0x0000ffff).", "Line number 1.");
    ly_in_free(in, 0);

    lyxml_ctx_free(xmlctx);
}

static void
test_ns(void **state)
{
    const char *str;
    struct lyxml_ctx *xmlctx;
    struct ly_in *in;
    const struct lyxml_ns *ns;

    /* opening element1 */
    str = "<element1/>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_SUCCESS, lyxml_ctx_new(UTEST_LYCTX, in, &xmlctx));

    /* processing namespace definitions */
    assert_int_equal(LY_SUCCESS, lyxml_ns_add(xmlctx, NULL, 0, strdup("urn:default")));
    assert_int_equal(LY_SUCCESS, lyxml_ns_add(xmlctx, "nc", 2, strdup("urn:nc1")));
    /* simulate adding open element2 into context */
    xmlctx->elements.count++;
    /* processing namespace definitions */
    assert_int_equal(LY_SUCCESS, lyxml_ns_add(xmlctx, "nc", 2, strdup("urn:nc2")));
    assert_int_equal(3, xmlctx->ns.count);
    assert_int_not_equal(0, xmlctx->ns.size);

    ns = lyxml_ns_get(&xmlctx->ns, NULL, 0);
    assert_non_null(ns);
    assert_null(ns->prefix);
    assert_string_equal("urn:default", ns->uri);

    ns = lyxml_ns_get(&xmlctx->ns, "nc", 2);
    assert_non_null(ns);
    assert_string_equal("nc", ns->prefix);
    assert_string_equal("urn:nc2", ns->uri);

    /* simulate closing element2 */
    xmlctx->elements.count--;
    lyxml_ns_rm(xmlctx);
    assert_int_equal(2, xmlctx->ns.count);

    ns = lyxml_ns_get(&xmlctx->ns, "nc", 2);
    assert_non_null(ns);
    assert_string_equal("nc", ns->prefix);
    assert_string_equal("urn:nc1", ns->uri);

    /* close element1 */
    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(0, xmlctx->ns.count);

    assert_null(lyxml_ns_get(&xmlctx->ns, "nc", 2));
    assert_null(lyxml_ns_get(&xmlctx->ns, NULL, 0));

    lyxml_ctx_free(xmlctx);
    ly_in_free(in, 0);
}

static void
test_ns2(void **state)
{
    const char *str;
    struct lyxml_ctx *xmlctx;
    struct ly_in *in;

    /* opening element1 */
    str = "<element1/>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_SUCCESS, lyxml_ctx_new(UTEST_LYCTX, in, &xmlctx));

    /* default namespace defined in parent element1 */
    assert_int_equal(LY_SUCCESS, lyxml_ns_add(xmlctx, NULL, 0, strdup("urn:default")));
    assert_int_equal(1, xmlctx->ns.count);
    /* going into child element1 */
    /* simulate adding open element1 into context */
    xmlctx->elements.count++;
    /* no namespace defined, going out (first, simulate closing of so far open element) */
    xmlctx->elements.count--;
    lyxml_ns_rm(xmlctx);
    assert_int_equal(1, xmlctx->ns.count);

    /* nothing else, going out of the parent element1 */
    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(0, xmlctx->ns.count);

    lyxml_ctx_free(xmlctx);
    ly_in_free(in, 0);
}

static void
test_simple_xml(void **state)
{
    struct lyxml_ctx *xmlctx;
    struct ly_in *in;
    const char *test_input = "<elem1 attr1=\"value\"> <elem2 attr2=\"value\" /> </elem1>";

    assert_int_equal(LY_SUCCESS, ly_in_new_memory(test_input, &in));
    assert_int_equal(LY_SUCCESS, lyxml_ctx_new(UTEST_LYCTX, in, &xmlctx));
    assert_int_equal(LYXML_ELEMENT, xmlctx->status);
    assert_string_equal(xmlctx->in->current, "attr1=\"value\"> <elem2 attr2=\"value\" /> </elem1>");

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ATTRIBUTE, xmlctx->status);
    assert_string_equal(xmlctx->in->current, "=\"value\"> <elem2 attr2=\"value\" /> </elem1>");

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ATTR_CONTENT, xmlctx->status);
    assert_string_equal(xmlctx->in->current, "> <elem2 attr2=\"value\" /> </elem1>");

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ELEM_CONTENT, xmlctx->status);
    assert_string_equal(xmlctx->in->current, "<elem2 attr2=\"value\" /> </elem1>");

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ELEMENT, xmlctx->status);
    assert_string_equal(xmlctx->in->current, "attr2=\"value\" /> </elem1>");

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ATTRIBUTE, xmlctx->status);
    assert_string_equal(xmlctx->in->current, "=\"value\" /> </elem1>");

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ATTR_CONTENT, xmlctx->status);
    assert_string_equal(xmlctx->in->current, " /> </elem1>");

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ELEM_CONTENT, xmlctx->status);
    assert_string_equal(xmlctx->in->current, "/> </elem1>");

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ELEM_CLOSE, xmlctx->status);
    assert_string_equal(xmlctx->in->current, " </elem1>");

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_ELEM_CLOSE, xmlctx->status);
    assert_string_equal(xmlctx->in->current, "");

    assert_int_equal(LY_SUCCESS, lyxml_ctx_next(xmlctx));
    assert_int_equal(LYXML_END, xmlctx->status);
    assert_string_equal(xmlctx->in->current, "");

    lyxml_ctx_free(xmlctx);
    ly_in_free(in, 0);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_element),
        UTEST(test_attribute),
        UTEST(test_text),
        UTEST(test_ns),
        UTEST(test_ns2),
        UTEST(test_simple_xml),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
