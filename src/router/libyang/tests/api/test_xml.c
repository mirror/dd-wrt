/*
 * @file test_xml.c
 * @author: Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief unit tests for functions from xml.h header
 *
 * Copyright (C) 2016 Deutsche Telekom AG.
 *
 * Author: Mislav Novakovic <mislav.novakovic@sartura.hr>
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#include "tests/config.h"
#include "libyang.h"

#define TMP_TEMPLATE "/tmp/libyang-XXXXXX"

struct ly_ctx *ctx = NULL;
struct lyd_node *root = NULL;

const char *a_data_xml = "\
<x xmlns=\"urn:a\">\n\
  <bubba>test</bubba>\n\
  </x>\n";

const char *a_data_xml_attr = "\
<x xmlns=\"urn:a\" bubba=\"test\">\n\
  <bubba xmlns=\"urn:a\" name=\"test\"/>\n\
</x>\n";

const char *res_xml = "<x xmlns=\"urn:a\"><bubba>test</bubba></x>";

const char *a_err_data_xml_001 = "<x xmlns=><bubba>test</bubba></x>";

const char *a_err_data_xml_002 = "<x xmlns*><bubba>test</bubba></x>";

const char *a_err_data_xml_003 = "\
<!DOCTYPE>\n\
<x xmlns=\"urn:a\">\n\
    <bubba> test </bubba>\n\
</x>\n";

/* Create a model that has different bytes.  α:two bytes  阳:three bytes  𪐕:four bytes */
const char *a_correct_data_xml_001 = "<α阳𪐕 xmlns=\"urn:a\"><bubba>test</bubba></α阳𪐕>";

const char *a_correct_data_xml_002 = "<x xmlns=\"urn:a\"><bubba>&apos; and &quot;</bubba></x>";

const char *a_correct_data_xml_003 = "\
<x xmlns=\"urn:a\">\n\
    <!-- this is comment -->\n\
    <bubba>test</bubba>\n\
</x>\n";

const char *a_correct_data_xml_004 = "\
<x xmlns=\"urn:a\">\n\
    test\n\
    <bubba>test</bubba>\n\
</x>\n";

const char *a_correct_data_xml_005 = "\
<!-- this is comment -->\n\
<x xmlns=\"urn:a\">\n\
    <bubba>test</bubba>\n\
</x>\n";

const char *a_correct_data_xml_006 = "\
<x xmlns=\"urn:a\">\n\
    <![CDATA[ you and me ]]>\n\
    <bubba>test</bubba>\n\
</x>\n";

const char *a_correct_data_xml_007 = "\
<x xmlns=\"urn:a\">\n\
    <?xml version=\" 1.0 \" ?>\n\
    <bubba>test</bubba>\n\
</x>\n";

const char *a_correct_data_xml_008 = "\
<x xmlns=\"urn:a\">\n\
    <bubba><bubba>test</bubba>test</bubba>\n\
</x>\n";

int
generic_init(char *yang_file, char *yang_folder)
{
    LYS_INFORMAT yang_format;
    char *schema = NULL;
    struct stat sb_schema;
    int fd = -1;

    if (!yang_file || !yang_folder) {
        goto error;
    }

    yang_format = LYS_IN_YIN;

    ctx = ly_ctx_new(yang_folder, 0);
    if (!ctx) {
        goto error;
    }

    fd = open(yang_file, O_RDONLY);
    if (fd == -1 || fstat(fd, &sb_schema) == -1 || !S_ISREG(sb_schema.st_mode)) {
        goto error;
    }

    schema = mmap(NULL, sb_schema.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    if (!lys_parse_mem(ctx, schema, yang_format)) {
        goto error;
    }

    /* cleanup */
    munmap(schema, sb_schema.st_size);

    return 0;

error:
    if (schema) {
        munmap(schema, sb_schema.st_size);
    }
    if (fd != -1) {
        close(fd);
    }

    return -1;
}

static int
setup_f(void **state)
{
    (void) state; /* unused */
    char *yang_file = TESTS_DIR"/api/files/a.yin";
    char *yang_folder = TESTS_DIR"/api/files";
    int rc;

    rc = generic_init(yang_file, yang_folder);

    if (rc) {
        return -1;
    }

    return 0;
}

static int
teardown_f(void **state)
{
    (void) state; /* unused */
    if (root)
        lyd_free(root);
    if (ctx)
        ly_ctx_destroy(ctx, NULL);

    return 0;
}

static void
test_lyxml_parse_mem(void **state)
{
    (void) state; /* unused */
    struct lyxml_elem *xml = NULL;

    xml = lyxml_parse_mem(ctx, a_data_xml, 0);

    assert_string_equal("x", xml->name);

    lyxml_free(ctx, xml);
}

static void
test_lyxml_free(void **state)
{
    (void) state; /* unused */
    struct lyxml_elem *xml = NULL;

    xml = lyxml_parse_mem(ctx, a_data_xml, 0);

    assert_string_equal("x", xml->name);

    lyxml_free(ctx, xml);
}

static void
test_lyxml_parse_path(void **state)
{
    (void) state; /* unused */
    struct lyxml_elem *xml = NULL;
    const char *path = TESTS_DIR"/api/files/a.xml";

    xml = lyxml_parse_path(ctx, path, 0);
    if (!xml) {
        fail();
    }

    assert_string_equal("x", xml->name);

    lyxml_free(ctx, xml);
}

static void
test_lyxml_print_fd(void **state)
{
    (void) state; /* unused */
    struct lyxml_elem *xml = NULL;
    const char *path = TESTS_DIR"/api/files/a.xml";

    xml = lyxml_parse_path(ctx, path, 0);
    if (!xml) {
        fail();
    }

    assert_string_equal("x", xml->name);

    (void) state; /* unused */
    char *result = NULL;
    struct stat sb;
    char file_name[20];
    int rc;
    int fd;

    memset(file_name, 0, sizeof(file_name));
    strncpy(file_name, TMP_TEMPLATE, sizeof(file_name));

    fd = mkstemp(file_name);
    if (fd < 1) {
        goto error;
    }

    rc = lyxml_print_fd(fd, xml, 0);
    if (!rc) {
        goto error;
    }

    if (fstat(fd, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        goto error;
    }

    result = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    assert_string_equal(res_xml, result);

    close(fd);
    unlink(file_name);
    lyxml_free(ctx, xml);

    return;
error:
    if (fd > 0) {
        close(fd);
        unlink(file_name);
    }
    if (!xml)
        lyxml_free(ctx, xml);
    fail();
}

static void
test_lyxml_print_file(void **state)
{
    (void) state; /* unused */
    struct lyxml_elem *xml = NULL;
    const char *path = TESTS_DIR"/api/files/a.xml";
    struct stat sb;
    char file_name[20];
    char *result;
    FILE *f = NULL;
    int rc;
    int fd;

    xml = lyxml_parse_path(ctx, path, 0);
    if (!xml) {
        fail();
    }

    assert_string_equal("x", xml->name);

    memset(file_name, 0, sizeof(file_name));
    strncpy(file_name, TMP_TEMPLATE, sizeof(file_name));

    fd = mkstemp(file_name);
    if (fd < 1) {
        goto error;
    }
    close(fd);

    f = (fopen(file_name,"r+"));
    if (f == NULL) {
        goto error;
    }

    rc = lyxml_print_file(f, xml, 0);
    if (!rc) {
        goto error;
    }

    fclose(f);

    fd = open(file_name, O_RDONLY);
    if (fd == -1 || fstat(fd, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        goto error;
    }

    result = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    assert_string_equal(res_xml, result);

    close(fd);
    unlink(file_name);
    lyxml_free(ctx, xml);

    return;
error:
    if (f)
        fclose(f);
    if (fd > 0) {
        unlink(file_name);
        close(fd);
    }
    if (xml)
        lyxml_free(ctx, xml);
    fail();
}

static void
test_lyxml_print_mem(void **state)
{
    (void) state; /* unused */
    struct lyxml_elem *xml = NULL;
    const char *path = TESTS_DIR"/api/files/a.xml";
    char *result = NULL;
    int rc;

    xml = lyxml_parse_path(ctx, path, 0);
    if (!xml) {
        fail();
    }

    assert_string_equal("x", xml->name);

    rc = lyxml_print_mem(&result, xml, 0);
    if (!rc) {
        lyxml_free(ctx, xml);
        fail();
    }

    assert_string_equal(res_xml, result);

    lyxml_free(ctx, xml);
    free(result);
}

struct buff {
	int len;
	const char *cmp;
};

ssize_t custom_lyxml_print_clb(void *arg, const void *buf, size_t count) {
    int rc;
    int len;
    struct buff *pos = arg;

    len = pos->len + count;

    const char *chunk = &pos->cmp[pos->len];

    rc = strncmp(chunk, buf, count);
    if (rc) {
	    fail();
    }

    pos->len = len;
    return count;
}

static void
test_lyxml_print_clb(void **state)
{
    (void) state; /* unused */
    struct lyxml_elem *xml = NULL;
    const char *path = TESTS_DIR"/api/files/a.xml";
    int rc;

    xml = lyxml_parse_path(ctx, path, 0);
    if (!xml) {
        fail();
    }

    assert_string_equal("x", xml->name);

    struct buff *buf = calloc(1, sizeof(struct buff));
    if (!buf) {
        fail();
        lyxml_free(ctx, xml);
    }

    buf->len = 0;
    buf->cmp = res_xml;
    void *arg = buf;

    rc = lyxml_print_clb(custom_lyxml_print_clb, arg, xml, 0);
    if (!rc) {
        fail();
        free(buf);
        lyxml_free(ctx, xml);
    }

    free(buf);
    lyxml_free(ctx, xml);
}

static void
test_lyxml_unlink(void **state)
{
    (void) state; /* unused */
    struct lyxml_elem *xml = NULL;

    xml = lyxml_parse_mem(ctx, a_data_xml, 0);

    assert_string_equal("bubba", xml->child->name);
    lyxml_free(ctx, xml->child);
    lyxml_unlink(ctx, xml->child);

    if (xml->child) {
        fail();
    }

    lyxml_free(ctx, xml);
}

static void
test_lyxml_get_attr(void **state)
{
    (void) state; /* unused */
    struct lyxml_elem *xml = NULL;
    const char *result = NULL;

    xml = lyxml_parse_mem(ctx, a_data_xml_attr, 0);

    result = lyxml_get_attr(xml, "bubba", NULL);
    if (!result) {
        lyxml_free(ctx, xml);
        fail();
    }

    assert_string_equal("test", result);

    lyxml_free(ctx, xml);
}

static void
test_lyxml_get_ns(void **state)
{
    (void) state; /* unused */
    struct lyxml_elem *xml = NULL;
    const struct lyxml_ns *ns = NULL;

    xml = lyxml_parse_mem(ctx, a_data_xml, 0);

    ns = lyxml_get_ns(xml, NULL);
    if (!ns) {
        lyxml_free(ctx, xml);
        fail();
    }

    assert_string_equal("urn:a", ns->value);

    lyxml_free(ctx, xml);
}

void
test_lyxml_dup(void **state)
{
    (void) state;
    struct lyxml_elem *first_xml = NULL;
    struct lyxml_elem *second_xml = NULL;
    const char *path = TESTS_DIR"/api/files/a.xml";

    first_xml = lyxml_parse_path(ctx, path, 0);

    if (!first_xml) {
        fail();
    }

    /* Making sure that the first and the second element aren't the same */
    if (first_xml == second_xml) {
        fail();
    }

    second_xml = lyxml_dup(ctx, first_xml);

    /* Checking whether the first element is duplicated into the second */
    if (!second_xml) {
        fail();
    }

    /* Freeing the elements */
    lyxml_free(ctx, first_xml);
    lyxml_free(ctx, second_xml);
}

void
test_lyxml_free_withsiblings(void **state)
{
    (void) state;
    struct lyxml_elem *xml = NULL;
    const char *path = TESTS_DIR"/api/files/a.xml";
    xml = lyxml_parse_path(ctx, path, 0);

    /* Making sure the element is not null */
    if (!xml) {
        fail();
    }

    /* Freeing the element with its siblings */
    lyxml_free_withsiblings(ctx, xml);
}

void
test_lyxml_xmlns_wrong_format(void **state)
{
    (void)state;
    struct lyxml_elem *xml = NULL;

    xml = lyxml_parse_mem(ctx, a_err_data_xml_001, 0);
    assert_ptr_equal(xml, NULL);
    lyxml_free(ctx, xml);

    xml = lyxml_parse_mem(ctx, a_err_data_xml_002, 0);
    assert_ptr_equal(xml, NULL);
    lyxml_free(ctx, xml);

    xml = lyxml_parse_mem(ctx, a_err_data_xml_003, 0);
    assert_ptr_equal(xml, NULL);
    lyxml_free(ctx, xml);
}

void
test_lyxml_xmlns_correct_format(void **state)
{
    (void)state;
    struct lyxml_elem *xml = NULL;

    xml = lyxml_parse_mem(ctx, a_correct_data_xml_001, 0);
    assert_string_equal("α阳𪐕", xml->name);
    lyxml_free(ctx, xml);

    xml = lyxml_parse_mem(ctx, a_correct_data_xml_002, 0);
    assert_string_equal("' and \"", xml->child->content);
    lyxml_free(ctx, xml);

    xml = lyxml_parse_mem(ctx, a_correct_data_xml_003, 0);
    assert_ptr_not_equal(xml, NULL);
    lyxml_free(ctx, xml);

    xml = lyxml_parse_mem(ctx, a_correct_data_xml_004, 0);
    assert_ptr_not_equal(xml, NULL);
    lyxml_free(ctx, xml);

    xml = lyxml_parse_mem(ctx, a_correct_data_xml_005, 0);
    assert_ptr_not_equal(xml, NULL);
    lyxml_free(ctx, xml);

    xml = lyxml_parse_mem(ctx, a_correct_data_xml_006, 0);
    assert_ptr_not_equal(xml, NULL);
    lyxml_free(ctx, xml);

    xml = lyxml_parse_mem(ctx, a_correct_data_xml_007, 0);
    assert_ptr_not_equal(xml, NULL);
    lyxml_free(ctx, xml);

    xml = lyxml_parse_mem(ctx, a_correct_data_xml_008, 0);
    assert_ptr_not_equal(xml, NULL);
    lyxml_free(ctx, xml);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_lyxml_parse_mem, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyxml_free, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyxml_parse_path, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyxml_print_file, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyxml_print_fd, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyxml_print_mem, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyxml_print_clb, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyxml_unlink, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyxml_get_attr, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyxml_get_ns, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyxml_dup, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyxml_free_withsiblings, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyxml_xmlns_wrong_format, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyxml_xmlns_correct_format, setup_f, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
