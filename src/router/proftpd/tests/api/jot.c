/*
 * ProFTPD - FTP server testsuite
 * Copyright (c) 2020 The ProFTPD Project team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 *
 * As a special exemption, The ProFTPD Project team and other respective
 * copyright holders give permission to link this program with OpenSSL, and
 * distribute the resulting executable, without including the source code for
 * OpenSSL in the source distribution.
 */

/* Jot API tests. */

#include "tests.h"
#include "logfmt.h"
#include "json.h"
#include "jot.h"

extern pr_response_t *resp_list;

static pool *p = NULL;

static void set_up(void) {
  if (p == NULL) {
    p = permanent_pool = make_sub_pool(NULL);
  }

  if (getenv("TEST_VERBOSE") != NULL) {
    pr_trace_set_levels("jot", 1, 20);
  }

  init_fs();
}

static void tear_down(void) {
  if (session.c != NULL) {
    pr_inet_close(p, session.c);
    session.c = NULL;
  }

  if (getenv("TEST_VERBOSE") != NULL) {
    pr_trace_set_levels("jot", 0, 0);
  }

  if (p != NULL) {
    destroy_pool(p);
    p = permanent_pool = NULL;
  }
}

/* Tests */

static void assert_jot_class_filter(const char *class_name) {
  pr_jot_filters_t *filters;
  const char *rules;

  rules = class_name;

  mark_point();
  filters = pr_jot_filters_create(p, rules, PR_JOT_FILTER_TYPE_CLASSES, 0);
  ck_assert_msg(filters != NULL, "Failed to create filters from '%s': %s",
    rules, strerror(errno));
  (void) pr_jot_filters_destroy(filters);

  rules = pstrcat(p, "!", class_name, NULL);

  mark_point();
  filters = pr_jot_filters_create(p, rules, PR_JOT_FILTER_TYPE_CLASSES, 0);
  ck_assert_msg(filters != NULL, "Failed to create filters from '%s': %s",
    rules, strerror(errno));
  (void) pr_jot_filters_destroy(filters);
}

static void assert_jot_command_with_class_filter(const char *rules) {
  pr_jot_filters_t *filters;

  mark_point();
  filters = pr_jot_filters_create(p, rules,
    PR_JOT_FILTER_TYPE_COMMANDS_WITH_CLASSES, 0);
  ck_assert_msg(filters != NULL, "Failed to create filters from '%s': %s",
    rules, strerror(errno));
  (void) pr_jot_filters_destroy(filters);
}

START_TEST (jot_filters_create_test) {
  pr_jot_filters_t *filters;
  const char *rules;

  mark_point();
  filters = pr_jot_filters_create(NULL, NULL, 0, 0);
  ck_assert_msg(filters == NULL, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  filters = pr_jot_filters_create(p, NULL, 0, 0);
  ck_assert_msg(filters == NULL, "Failed to handle null rules");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  rules = "foo";

  mark_point();
  filters = pr_jot_filters_create(p, rules, -1, 0);
  ck_assert_msg(filters == NULL, "Failed to handle invalid rules type");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  /* Class rules */

  mark_point();
  filters = pr_jot_filters_create(p, rules, PR_JOT_FILTER_TYPE_CLASSES, 0);
  ck_assert_msg(filters == NULL, "Failed to handle invalid class name '%s'",
    rules);
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  assert_jot_class_filter("NONE");
  assert_jot_class_filter("ALL");
  assert_jot_class_filter("AUTH");
  assert_jot_class_filter("INFO");
  assert_jot_class_filter("DIRS");
  assert_jot_class_filter("READ");
  assert_jot_class_filter("WRITE");
  assert_jot_class_filter("SEC");
  assert_jot_class_filter("SECURE");
  assert_jot_class_filter("CONNECT");
  assert_jot_class_filter("EXIT");
  assert_jot_class_filter("DISCONNECT");
  assert_jot_class_filter("SSH");
  assert_jot_class_filter("SFTP");

  rules = "AUTH,!INFO";

  mark_point();
  filters = pr_jot_filters_create(p, rules, PR_JOT_FILTER_TYPE_CLASSES, 0);
  ck_assert_msg(filters != NULL, "Failed to create filters from '%s': %s",
    rules, strerror(errno));
  (void) pr_jot_filters_destroy(filters);

  rules = "!INFO|AUTH";

  mark_point();
  filters = pr_jot_filters_create(p, rules, PR_JOT_FILTER_TYPE_CLASSES, 0);
  ck_assert_msg(filters != NULL, "Failed to create filters from '%s': %s",
    rules, strerror(errno));
  (void) pr_jot_filters_destroy(filters);

  /* Command rules */

  rules = "FOO,BAR";
  mark_point();
  filters = pr_jot_filters_create(p, rules, PR_JOT_FILTER_TYPE_COMMANDS, 0);
  ck_assert_msg(filters != NULL, "Failed to create filters from '%s': %s",
    rules, strerror(errno));
  (void) pr_jot_filters_destroy(filters);

  rules = "APPE,RETR,STOR,STOU";
  mark_point();
  filters = pr_jot_filters_create(p, rules, PR_JOT_FILTER_TYPE_COMMANDS, 0);
  ck_assert_msg(filters != NULL, "Failed to create filters from '%s': %s",
    rules, strerror(errno));
  (void) pr_jot_filters_destroy(filters);

  /* Rules with commands and classes */

  rules = "CONNECT,RETR,STOR,DISCONNECT";
  assert_jot_command_with_class_filter(rules);

  rules = "RETR,STOR,AUTH";
  assert_jot_command_with_class_filter(rules);

  rules = "RETR,STOR,DIRS";
  assert_jot_command_with_class_filter(rules);

  rules = "RETR,STOR,INFO";
  assert_jot_command_with_class_filter(rules);

  rules = "RETR,STOR,MISC";
  assert_jot_command_with_class_filter(rules);

  rules = "READ,RETR,STOR";
  assert_jot_command_with_class_filter(rules);

  rules = "RETR,SEC,STOR";
  assert_jot_command_with_class_filter(rules);

  rules = "RETR,SFTP,STOR";
  assert_jot_command_with_class_filter(rules);

  rules = "RETR,SSH,STOR";
  assert_jot_command_with_class_filter(rules);

  rules = "RETR,STOR,WRITE";
  assert_jot_command_with_class_filter(rules);

  rules = "ALL";
  mark_point();
  filters = pr_jot_filters_create(p, rules,
    PR_JOT_FILTER_TYPE_COMMANDS_WITH_CLASSES, 0);
  ck_assert_msg(filters != NULL, "Failed to create filters from '%s': %s",
    rules, strerror(errno));
  (void) pr_jot_filters_destroy(filters);

  /* Flags */

  rules = "ALL";
  mark_point();
  filters = pr_jot_filters_create(p, rules,
    PR_JOT_FILTER_TYPE_COMMANDS_WITH_CLASSES, PR_JOT_FILTER_FL_ALL_INCL_ALL);
  ck_assert_msg(filters != NULL, "Failed to create filters from '%s': %s",
    rules, strerror(errno));
  (void) pr_jot_filters_destroy(filters);
}
END_TEST

START_TEST (jot_filters_destroy_test) {
  int res;
  pr_jot_filters_t *filters;

  mark_point();
  res = pr_jot_filters_destroy(NULL);
  ck_assert_msg(res < 0, "Failed to handle null filters");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  filters = pr_jot_filters_create(p, "NONE", PR_JOT_FILTER_TYPE_CLASSES, 0);

  mark_point();
  res = pr_jot_filters_destroy(filters);
  ck_assert_msg(res == 0, "Failed to destroy filters: %s", strerror(errno));
}
END_TEST

START_TEST (jot_filters_include_classes_test) {
  int res;
  pr_jot_filters_t *filters;

  mark_point();
  res = pr_jot_filters_include_classes(NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null filters");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  filters = pr_jot_filters_create(p, "NONE", PR_JOT_FILTER_TYPE_CLASSES, 0);

  res = pr_jot_filters_include_classes(filters, CL_ALL);
  ck_assert_msg(res == FALSE, "Expected FALSE, got %d", res);

  res = pr_jot_filters_include_classes(filters, CL_NONE);
  ck_assert_msg(res == TRUE, "Expected TRUE, got %d", res);

  res = pr_jot_filters_destroy(filters);
  ck_assert_msg(res == 0, "Failed to destroy filters: %s", strerror(errno));
}
END_TEST

static unsigned int parse_on_meta_count = 0;
static unsigned int parse_on_unknown_count = 0;
static unsigned int parse_on_other_count = 0;

static int parse_on_meta(pool *jot_pool, pr_jot_ctx_t *jot_ctx,
    unsigned char logfmt_id, const char *text, size_t text_len) {
  parse_on_meta_count++;
  return 0;
}

static int parse_on_unknown(pool *jot_pool, pr_jot_ctx_t *jot_ctx,
    const char *text, size_t text_len) {
  parse_on_unknown_count++;
  return 0;
}

static int parse_on_other(pool *jot_pool, pr_jot_ctx_t *jot_ctx, char ch) {
  parse_on_other_count++;
  return 0;
}

START_TEST (jot_parse_on_meta_test) {
  int res;
  pr_jot_ctx_t *jot_ctx;
  pr_jot_parsed_t *jot_parsed;

  mark_point();
  res = pr_jot_parse_on_meta(p, NULL, 0, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null jot_ctx");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  jot_ctx = pcalloc(p, sizeof(pr_jot_ctx_t));

  mark_point();
  res = pr_jot_parse_on_meta(p, jot_ctx, 0, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null jot_ctx->log");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  jot_parsed = pcalloc(p, sizeof(pr_jot_parsed_t));
  jot_ctx->log = jot_parsed;

  mark_point();
  res = pr_jot_parse_on_meta(p, jot_ctx, 0, NULL, 0);
  ck_assert_msg(res == 0, "Failed to handle parse_on_meta callback: %s",
    strerror(errno));
}
END_TEST

START_TEST (jot_parse_on_unknown_test) {
  int res;
  pr_jot_ctx_t *jot_ctx;
  pr_jot_parsed_t *jot_parsed;

  mark_point();
  res = pr_jot_parse_on_unknown(p, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null jot_ctx");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  jot_ctx = pcalloc(p, sizeof(pr_jot_ctx_t));

  mark_point();
  res = pr_jot_parse_on_unknown(p, jot_ctx, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null jot_ctx->log");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  jot_parsed = pcalloc(p, sizeof(pr_jot_parsed_t));
  jot_ctx->log = jot_parsed;

  mark_point();
  res = pr_jot_parse_on_unknown(p, jot_ctx, NULL, 0);
  ck_assert_msg(res == 0, "Failed to handle parse_on_unknown callback: %s",
    strerror(errno));
}
END_TEST

START_TEST (jot_parse_on_other_test) {
  int res;
  pr_jot_ctx_t *jot_ctx;
  pr_jot_parsed_t *jot_parsed;

  mark_point();
  res = pr_jot_parse_on_other(p, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null jot_ctx");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  jot_ctx = pcalloc(p, sizeof(pr_jot_ctx_t));

  mark_point();
  res = pr_jot_parse_on_other(p, jot_ctx, 0);
  ck_assert_msg(res < 0, "Failed to handle null jot_ctx->log");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  jot_parsed = pcalloc(p, sizeof(pr_jot_parsed_t));
  jot_ctx->log = jot_parsed;

  mark_point();
  res = pr_jot_parse_on_other(p, jot_ctx, 0);
  ck_assert_msg(res == 0, "Failed to handle parse_on_other callback: %s",
    strerror(errno));
}
END_TEST

START_TEST (jot_parse_logfmt_test) {
  int res;
  const char *text;
  size_t text_len;
  pr_jot_ctx_t *jot_ctx;

  mark_point();
  res = pr_jot_parse_logfmt(NULL, NULL, NULL, NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_jot_parse_logfmt(p, NULL, NULL, NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null text");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  text = "Hello, World!";

  mark_point();
  res = pr_jot_parse_logfmt(p, text, NULL, NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null ctx");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  jot_ctx = pcalloc(p, sizeof(pr_jot_ctx_t));

  mark_point();
  res = pr_jot_parse_logfmt(p, text, jot_ctx, NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null on_meta");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  parse_on_meta_count = parse_on_unknown_count = parse_on_other_count = 0;

  mark_point();
  res = pr_jot_parse_logfmt(p, text, jot_ctx, parse_on_meta, NULL, NULL, 0);
  ck_assert_msg(res == 0, "Failed to parse text '%s': %s", text, strerror(errno));
  ck_assert_msg(parse_on_meta_count == 0,
    "Expected on_meta count 0, got %u", parse_on_meta_count);
  ck_assert_msg(parse_on_unknown_count == 0,
    "Expected on_unknown count 0, got %u", parse_on_unknown_count);
  ck_assert_msg(parse_on_other_count == 0,
    "Expected on_other count 0, got %u", parse_on_other_count);

  parse_on_meta_count = parse_on_unknown_count = parse_on_other_count = 0;
  text_len = strlen(text);

  mark_point();
  res = pr_jot_parse_logfmt(p, text, jot_ctx, parse_on_meta, NULL,
    parse_on_other, 0);
  ck_assert_msg(res == 0, "Failed to parse text '%s': %s", text, strerror(errno));
  ck_assert_msg(parse_on_meta_count == 0,
    "Expected on_meta count 0, got %u", parse_on_meta_count);
  ck_assert_msg(parse_on_unknown_count == 0,
    "Expected on_unknown count 0, got %u", parse_on_unknown_count);
  ck_assert_msg((unsigned long) parse_on_other_count == text_len,
    "Expected on_other count %lu, got %u", (unsigned long) text_len,
    parse_on_other_count);

  parse_on_meta_count = parse_on_unknown_count = parse_on_other_count = 0;
  text = "%A %b %{epoch} %{unknown key here}, boo!";
  text_len = strlen(text);

  mark_point();
  res = pr_jot_parse_logfmt(p, text, jot_ctx, parse_on_meta, parse_on_unknown,
    parse_on_other, 0);
  ck_assert_msg(res == 0, "Failed to parse text '%s': %s", text, strerror(errno));
  ck_assert_msg(parse_on_meta_count == 3,
    "Expected on_meta count 0, got %u", parse_on_meta_count);
  ck_assert_msg(parse_on_unknown_count == 1,
    "Expected on_unknown count 0, got %u", parse_on_unknown_count);
  ck_assert_msg(parse_on_other_count == 9,
    "Expected on_other count 9, got %u", parse_on_other_count);

  parse_on_meta_count = parse_on_unknown_count = parse_on_other_count = 0;
  text = "%A %b %{epoch} %{unknown key here}, %{not closed";
  text_len = strlen(text);

  mark_point();
  res = pr_jot_parse_logfmt(p, text, jot_ctx, parse_on_meta, parse_on_unknown,
    parse_on_other, 0);
  ck_assert_msg(res == 0, "Failed to parse text '%s': %s", text, strerror(errno));
  ck_assert_msg(parse_on_meta_count == 3,
    "Expected on_meta count 0, got %u", parse_on_meta_count);
  ck_assert_msg(parse_on_unknown_count == 1,
    "Expected on_unknown count 0, got %u", parse_on_unknown_count);
  ck_assert_msg(parse_on_other_count == 17,
    "Expected on_other count 17, got %u", parse_on_other_count);
}
END_TEST

START_TEST (jot_parse_logfmt_short_vars_test) {
  register unsigned int i;
  int res;
  unsigned int text_count = 0;
  pr_jot_ctx_t *jot_ctx;
  const char *text;
  const char *texts[] = {
    "%A",
    "%D",
    "%E",
    "%F",
    "%H",
    "%I",
    "%J",
    "%L",
    "%O",
    "%R",
    "%S",
    "%T",
    "%U",
    "%V",
    "%a",
    "%b",
    "%c",
    "%d",
    "%f",
    "%g",
    "%h",
    "%l",
    "%m",
    "%p",
    "%r",
    "%s",
    "%u",
    "%v",
    "%w",
    NULL
  };

  jot_ctx = pcalloc(p, sizeof(pr_jot_ctx_t));
  text = "%X";
  parse_on_meta_count = parse_on_unknown_count = parse_on_other_count = 0;

  /* Here we expect an other count of 2, for the '%' and the 'X'.  This is
   * not a recognized/supported short variable, and definitely not a long
   * variable, and thus the entire text is treated as "other", for each
   * character.
   */

  mark_point();
  res = pr_jot_parse_logfmt(p, text, jot_ctx, parse_on_meta, NULL,
    parse_on_other, 0);
  ck_assert_msg(res == 0, "Failed to parse text '%s': %s", text, strerror(errno));
  ck_assert_msg(parse_on_meta_count == 0, "Expected on_meta count 0, got %u",
    parse_on_meta_count);
  ck_assert_msg(parse_on_other_count == 2, "Expected on_other count 2, got %u",
    parse_on_other_count);

  text = "%{0}";
  parse_on_meta_count = parse_on_unknown_count = parse_on_other_count = 0;

  mark_point();
  res = pr_jot_parse_logfmt(p, text, jot_ctx, parse_on_meta, parse_on_unknown,
    parse_on_other, 0);
  ck_assert_msg(res == 0, "Failed to parse text '%s': %s", text, strerror(errno));
  ck_assert_msg(parse_on_meta_count == 0,
    "Expected on_meta count 0, got %u", parse_on_meta_count);
  ck_assert_msg(parse_on_unknown_count == 1,
    "Expected on_unknown count 1, got %u", parse_on_unknown_count);
  ck_assert_msg(parse_on_other_count == 0,
    "Expected on_other count 0, got %u", parse_on_other_count);

  parse_on_meta_count = parse_on_unknown_count = parse_on_other_count = 0;
  text_count = 0;

  for (i = 0; texts[i]; i++) {
    text = (const char *) texts[i];
    text_count++;

    mark_point();
    res = pr_jot_parse_logfmt(p, text, jot_ctx, parse_on_meta, parse_on_unknown,
      parse_on_other, 0);
    ck_assert_msg(res == 0, "Failed to parse text '%s': %s", text,
      strerror(errno));
  }

  ck_assert_msg(parse_on_meta_count == text_count,
    "Expected on_meta count %d, got %u", text_count, parse_on_meta_count);
  ck_assert_msg(parse_on_unknown_count == 0,
    "Expected on_unknown count 0, got %u", parse_on_unknown_count);
  ck_assert_msg(parse_on_other_count == 0,
    "Expected on_other count 0, got %u", parse_on_other_count);
}
END_TEST

static int long_on_meta(pool *jot_pool, pr_jot_ctx_t *jot_ctx,
    unsigned char logfmt_id, const char *text, size_t text_len) {
  if (strncmp(text, "FOOBAR", text_len) == 0) {
    parse_on_meta_count++;
  }

  return 0;
}

START_TEST (jot_parse_logfmt_long_vars_test) {
  register unsigned int i;
  int res;
  unsigned int text_count = 0;
  pr_jot_ctx_t *jot_ctx;
  const char *text;
  const char *texts[] = {
    "%{basename}",
    "%{epoch}",
    "%{file-modified}",
    "%{file-offset}",
    "%{file-size}",
    "%{gid}",
    "%{iso8601}",
    "%{microsecs}",
    "%{millisecs}",
    "%{protocol}",
    "%{remote-port}",
    "%{transfer-failure}",
    "%{transfer-millisecs}",
    "%{transfer-port}",
    "%{transfer-status}",
    "%{transfer-type}",
    "%{uid}",
    "%{version}",
    NULL
  };

  jot_ctx = pcalloc(p, sizeof(pr_jot_ctx_t));
  text = "%{env:FOOBAR}!";
  parse_on_meta_count = parse_on_unknown_count = parse_on_other_count = 0;

  mark_point();
  res = pr_jot_parse_logfmt(p, text, jot_ctx, long_on_meta, NULL,
    parse_on_other, 0);
  ck_assert_msg(res == 0, "Failed to parse text '%s': %s", text, strerror(errno));
  ck_assert_msg(parse_on_meta_count == 1, "Expected on_meta count 1, got %u",
    parse_on_meta_count);
  ck_assert_msg(parse_on_other_count == 1, "Expected on_other count 1, got %u",
    parse_on_other_count);

  text = "%{note:FOOBAR}!";
  parse_on_meta_count = parse_on_unknown_count = parse_on_other_count = 0;

  mark_point();
  res = pr_jot_parse_logfmt(p, text, jot_ctx, long_on_meta, NULL,
    parse_on_other, 0);
  ck_assert_msg(res == 0, "Failed to parse text '%s': %s", text, strerror(errno));
  ck_assert_msg(parse_on_meta_count == 1, "Expected on_meta count 1, got %u",
    parse_on_meta_count);
  ck_assert_msg(parse_on_other_count == 1, "Expected on_other count 1, got %u",
    parse_on_other_count);

  text = "%{time:FOOBAR}!";
  parse_on_meta_count = parse_on_unknown_count = parse_on_other_count = 0;

  mark_point();
  res = pr_jot_parse_logfmt(p, text, jot_ctx, long_on_meta, NULL,
    parse_on_other, 0);
  ck_assert_msg(res == 0, "Failed to parse text '%s': %s", text, strerror(errno));
  ck_assert_msg(parse_on_meta_count == 1, "Expected on_meta count 1, got %u",
    parse_on_meta_count);
  ck_assert_msg(parse_on_other_count == 1, "Expected on_other count 1, got %u",
    parse_on_other_count);

  parse_on_meta_count = parse_on_unknown_count = parse_on_other_count = 0;
  text_count = 0;

  for (i = 0; texts[i]; i++) {
    text = (const char *) texts[i];
    text_count++;

    mark_point();
    res = pr_jot_parse_logfmt(p, text, jot_ctx, parse_on_meta, NULL,
      parse_on_other, 0);
    ck_assert_msg(res == 0, "Failed to parse text '%s': %s", text,
      strerror(errno));
  }

  ck_assert_msg(parse_on_meta_count == text_count,
    "Expected on_meta count %d, got %u", text_count, parse_on_meta_count);
  ck_assert_msg(parse_on_other_count == 0, "Expected on_other count 0, got %u",
    parse_on_other_count);

  text = "%{FOOBAR}e";
  parse_on_meta_count = parse_on_unknown_count = parse_on_other_count = 0;

  mark_point();
  res = pr_jot_parse_logfmt(p, text, jot_ctx, long_on_meta, NULL,
    parse_on_other, 0);
  ck_assert_msg(res == 0, "Failed to parse text '%s': %s", text, strerror(errno));
  ck_assert_msg(parse_on_meta_count == 1, "Expected on_meta count 1, got %u",
    parse_on_meta_count);
  ck_assert_msg(parse_on_other_count == 0, "Expected on_other count 0, got %u",
    parse_on_other_count);

  text = "%{FOOBAR}t";
  parse_on_meta_count = parse_on_unknown_count = parse_on_other_count = 0;

  mark_point();
  res = pr_jot_parse_logfmt(p, text, jot_ctx, long_on_meta, NULL,
    parse_on_other, 0);
  ck_assert_msg(res == 0, "Failed to parse text '%s': %s", text, strerror(errno));
  ck_assert_msg(parse_on_meta_count == 1, "Expected on_meta count 1, got %u",
    parse_on_meta_count);
  ck_assert_msg(parse_on_other_count == 0, "Expected on_other count 0, got %u",
    parse_on_other_count);

  text = "%{FOOBAR}T";
  parse_on_meta_count = parse_on_unknown_count = parse_on_other_count = 0;

  /* Here we should see 1 unknown for "%{FOOBAR}", and 1 other for the
   * trailing "T".
   */

  mark_point();
  res = pr_jot_parse_logfmt(p, text, jot_ctx, long_on_meta, parse_on_unknown,
    parse_on_other, 0);
  ck_assert_msg(res == 0, "Failed to parse text '%s': %s", text, strerror(errno));
  ck_assert_msg(parse_on_meta_count == 0,
    "Expected on_meta count 0, got %u", parse_on_meta_count);
  ck_assert_msg(parse_on_unknown_count == 1,
    "Expected on_unknown count 1, got %u", parse_on_unknown_count);
  ck_assert_msg(parse_on_other_count == 1,
    "Expected on_other count 1, got %u", parse_on_other_count);
}
END_TEST

START_TEST (jot_parse_logfmt_custom_vars_test) {
  int res;
  pr_jot_ctx_t *jot_ctx;
  const char *text;

  jot_ctx = pcalloc(p, sizeof(pr_jot_ctx_t));
  text = "%{0}";
  parse_on_meta_count = parse_on_unknown_count = parse_on_other_count = 0;

  mark_point();
  res = pr_jot_parse_logfmt(p, text, jot_ctx, parse_on_meta, parse_on_unknown,
    parse_on_other, 0);
  ck_assert_msg(res == 0, "Failed to parse text '%s': %s", text, strerror(errno));
  ck_assert_msg(parse_on_meta_count == 0,
    "Expected on_meta count 0, got %u", parse_on_meta_count);
  ck_assert_msg(parse_on_unknown_count == 1,
    "Expected on_unknown count 1, got %u", parse_on_unknown_count);
  ck_assert_msg(parse_on_other_count == 0,
    "Expected on_other count 0, got %u", parse_on_other_count);

  parse_on_meta_count = parse_on_unknown_count = parse_on_other_count = 0;

  mark_point();
  res = pr_jot_parse_logfmt(p, text, jot_ctx, parse_on_meta, parse_on_unknown,
    parse_on_other, PR_JOT_LOGFMT_PARSE_FL_UNKNOWN_AS_CUSTOM);
  ck_assert_msg(res == 0, "Failed to parse text '%s': %s", text, strerror(errno));
  ck_assert_msg(parse_on_meta_count == 1,
    "Expected on_meta count 1, got %u", parse_on_meta_count);
  ck_assert_msg(parse_on_unknown_count == 0,
    "Expected on_unknown count 0, got %u", parse_on_unknown_count);
  ck_assert_msg(parse_on_other_count == 0,
    "Expected on_other count 0, got %u", parse_on_other_count);
}
END_TEST

static unsigned int resolve_on_meta_count = 0;
static unsigned int resolve_on_default_count = 0;
static unsigned int resolve_on_other_count = 0;

static int resolve_id_on_meta(pool *jot_pool, pr_jot_ctx_t *jot_ctx,
    unsigned char logfmt_id, const char *jot_hint, const void *jot_val) {
  resolve_on_meta_count++;
  return 0;
}

static int resolve_id_on_default(pool *jot_pool, pr_jot_ctx_t *jot_ctx,
    unsigned char logfmt_id) {
  resolve_on_default_count++;
  return 0;
}

START_TEST (jot_resolve_logfmt_id_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt_id;

  mark_point();
  res = pr_jot_resolve_logfmt_id(NULL, NULL, NULL, 0, NULL, 0, NULL, NULL,
    NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_jot_resolve_logfmt_id(p, NULL, NULL, 0, NULL, 0, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null cmd");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));

  mark_point();
  res = pr_jot_resolve_logfmt_id(p, cmd, NULL, 0, NULL, 0, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null on_meta");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  logfmt_id = 0;

  mark_point();
  res = pr_jot_resolve_logfmt_id(p, cmd, NULL, logfmt_id, NULL, 0, NULL,
    resolve_id_on_meta, NULL);
  ck_assert_msg(res < 0, "Failed to handle invalid logfmt_id %u", logfmt_id);
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  logfmt_id = LOGFMT_META_START;

  mark_point();
  res = pr_jot_resolve_logfmt_id(p, cmd, NULL, logfmt_id, NULL, 0, NULL,
    resolve_id_on_meta, NULL);
  ck_assert_msg(res < 0, "Failed to handle invalid logfmt_id %u", logfmt_id);
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  logfmt_id = LOGFMT_META_ARG_END;

  mark_point();
  res = pr_jot_resolve_logfmt_id(p, cmd, NULL, logfmt_id, NULL, 0, NULL,
    resolve_id_on_meta, NULL);
  ck_assert_msg(res < 0, "Failed to handle invalid logfmt_id %u", logfmt_id);
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
}
END_TEST

START_TEST (jot_resolve_logfmt_id_on_default_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt_id;

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  logfmt_id = LOGFMT_META_BASENAME;
  resolve_on_meta_count = resolve_on_default_count = 0;

  mark_point();
  res = pr_jot_resolve_logfmt_id(p, cmd, NULL, logfmt_id, NULL, 0, NULL,
    resolve_id_on_meta, resolve_id_on_default);
  ck_assert_msg(res == 0, "Failed to handle logfmt_id %u: %s", logfmt_id,
    strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
}
END_TEST

START_TEST (jot_resolve_logfmt_id_filters_test) {
  int res;
  cmd_rec *cmd;
  pr_jot_filters_t *jot_filters;
  unsigned char logfmt_id;

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_CONNECT;
  logfmt_id = LOGFMT_META_CONNECT;

  /* No filters; should be implicitly jottable. */
  resolve_on_meta_count = resolve_on_default_count = 0;
  jot_filters = NULL;

  mark_point();
  res = pr_jot_resolve_logfmt_id(p, cmd, jot_filters, logfmt_id, NULL, 0, NULL,
    resolve_id_on_meta, NULL);
  ck_assert_msg(res == 0, "Failed to handle logfmt_id %u: %s", logfmt_id,
    strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);

  /* With an ALL filter, and no command class. */
  cmd->cmd_class = 0;
  logfmt_id = LOGFMT_META_COMMAND;
  resolve_on_meta_count = resolve_on_default_count = 0;
  jot_filters = pr_jot_filters_create(p, "ALL",
    PR_JOT_FILTER_TYPE_CLASSES, PR_JOT_FILTER_FL_ALL_INCL_ALL);

  mark_point();
  res = pr_jot_resolve_logfmt_id(p, cmd, jot_filters, logfmt_id, NULL, 0, NULL,
    resolve_id_on_meta, NULL);
  ck_assert_msg(res == 0, "Failed to handle logfmt_id %u: %s", logfmt_id,
    strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);

  /* With explicit filters that allow the class. */
  cmd->cmd_class = CL_CONNECT;
  logfmt_id = LOGFMT_META_CONNECT;
  resolve_on_meta_count = resolve_on_default_count = 0;
  jot_filters = pr_jot_filters_create(p, "CONNECT",
    PR_JOT_FILTER_TYPE_CLASSES, 0);

  mark_point();
  res = pr_jot_resolve_logfmt_id(p, cmd, jot_filters, logfmt_id, NULL, 0, NULL,
    resolve_id_on_meta, NULL);
  ck_assert_msg(res == 0, "Failed to handle logfmt_id %u: %s", logfmt_id,
    strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);

  /* With explicit filters that ignore the class. */
  resolve_on_meta_count = resolve_on_default_count = 0;
  jot_filters = pr_jot_filters_create(p, "!CONNECT",
    PR_JOT_FILTER_TYPE_CLASSES, 0);

  mark_point();
  res = pr_jot_resolve_logfmt_id(p, cmd, jot_filters, logfmt_id, NULL, 0, NULL,
    resolve_id_on_meta, NULL);
  ck_assert_msg(res < 0, "Failed to handle filtered logfmt_id %u", logfmt_id);
  ck_assert_msg(errno == EPERM, "Expected EPERM (%d), got %s (%d)", EPERM,
    strerror(errno), errno);
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);

  /* With explicit filters that do not match the class. */
  resolve_on_meta_count = resolve_on_default_count = 0;
  jot_filters = pr_jot_filters_create(p, "DISCONNECT",
    PR_JOT_FILTER_TYPE_CLASSES, 0);

  mark_point();
  res = pr_jot_resolve_logfmt_id(p, cmd, jot_filters, logfmt_id, NULL, 0, NULL,
    resolve_id_on_meta, NULL);
  ck_assert_msg(res < 0, "Failed to handle filtered logfmt_id %u", logfmt_id);
  ck_assert_msg(errno == EPERM, "Expected EPERM (%d), got %s (%d)", EPERM,
    strerror(errno), errno);
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);

  /* With explicit filters that allow the command. Note that this REQUIRES
   * that we use a known command, since allowed command comparisons are done
   * by ID.
   */
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "RANG"));
  cmd->cmd_class = CL_CONNECT;
  resolve_on_meta_count = resolve_on_default_count = 0;
  jot_filters = pr_jot_filters_create(p, "RANG",
    PR_JOT_FILTER_TYPE_COMMANDS, 0);

  mark_point();
  res = pr_jot_resolve_logfmt_id(p, cmd, jot_filters, logfmt_id, NULL, 0, NULL,
    resolve_id_on_meta, NULL);
  ck_assert_msg(res == 0, "Failed to handle logfmt_id %u: %s", logfmt_id,
    strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);

  /* With explicit filters that ignore the command. */
  resolve_on_meta_count = resolve_on_default_count = 0;
  jot_filters = pr_jot_filters_create(p, "!RANG",
    PR_JOT_FILTER_TYPE_COMMANDS, 0);

  mark_point();
  res = pr_jot_resolve_logfmt_id(p, cmd, jot_filters, logfmt_id, NULL, 0, NULL,
    resolve_id_on_meta, NULL);
  ck_assert_msg(res < 0, "Failed to handle filtered logfmt_id %u", logfmt_id);
  ck_assert_msg(errno == EPERM, "Expected EPERM (%d), got %s (%d)", EPERM,
    strerror(errno), errno);
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);

  /* With explicit filters that do not match the command. */
  resolve_on_meta_count = resolve_on_default_count = 0;
  jot_filters = pr_jot_filters_create(p, "FOO",
    PR_JOT_FILTER_TYPE_COMMANDS, 0);

  mark_point();
  res = pr_jot_resolve_logfmt_id(p, cmd, jot_filters, logfmt_id, NULL, 0, NULL,
    resolve_id_on_meta, NULL);
  ck_assert_msg(res < 0, "Failed to handle filtered logfmt_id %u", logfmt_id);
  ck_assert_msg(errno == EPERM, "Expected EPERM (%d), got %s (%d)", EPERM,
    strerror(errno), errno);
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
}
END_TEST

START_TEST (jot_resolve_logfmt_id_connect_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt_id;

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_CONNECT;
  logfmt_id = LOGFMT_META_CONNECT;

  resolve_on_meta_count = resolve_on_default_count = 0;

  mark_point();
  res = pr_jot_resolve_logfmt_id(p, cmd, NULL, logfmt_id, NULL, 0, NULL,
    resolve_id_on_meta, NULL);
  ck_assert_msg(res == 0, "Failed to handle logfmt_id %u: %s", logfmt_id,
    strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);

  resolve_on_meta_count = resolve_on_default_count = 0;
  cmd->cmd_class = CL_DISCONNECT;

  mark_point();
  res = pr_jot_resolve_logfmt_id(p, cmd, NULL, logfmt_id, NULL, 0, NULL,
    resolve_id_on_meta, NULL);
  ck_assert_msg(res == 0, "Failed to handle logfmt_id %u: %s", logfmt_id,
    strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
}
END_TEST

START_TEST (jot_resolve_logfmt_id_disconnect_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt_id;

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_DISCONNECT;
  logfmt_id = LOGFMT_META_DISCONNECT;

  resolve_on_meta_count = resolve_on_default_count = 0;

  mark_point();
  res = pr_jot_resolve_logfmt_id(p, cmd, NULL, logfmt_id, NULL, 0, NULL,
    resolve_id_on_meta, NULL);
  ck_assert_msg(res == 0, "Failed to handle logfmt_id %u: %s", logfmt_id,
    strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);

  resolve_on_meta_count = resolve_on_default_count = 0;
  cmd->cmd_class = CL_CONNECT;

  mark_point();
  res = pr_jot_resolve_logfmt_id(p, cmd, NULL, logfmt_id, NULL, 0, NULL,
    resolve_id_on_meta, NULL);
  ck_assert_msg(res == 0, "Failed to handle logfmt_id %u: %s", logfmt_id,
    strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
}
END_TEST

START_TEST (jot_resolve_logfmt_id_custom_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt_id;
  const char *custom_data;
  size_t custom_datalen;

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  logfmt_id = LOGFMT_META_CUSTOM;

  resolve_on_meta_count = resolve_on_default_count = 0;
  custom_data = "%{0}";
  custom_datalen = strlen(custom_data);

  mark_point();
  res = pr_jot_resolve_logfmt_id(p, cmd, NULL, logfmt_id, custom_data,
    custom_datalen, NULL, resolve_id_on_meta, NULL);
  ck_assert_msg(res == 0, "Failed to handle logfmt_id %u: %s", logfmt_id,
    strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
}
END_TEST

START_TEST (jot_resolve_logfmt_ids_test) {
  register unsigned char i;
  int res;
  cmd_rec *cmd;
  unsigned char logfmt_id;

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  resolve_on_meta_count = resolve_on_default_count = 0;

  /* Currently, the max known LogFormat meta/ID is 53 (DISCONNECT). */
  for (i = 1; i < 54; i++) {
    logfmt_id = i;

    mark_point();
    res = pr_jot_resolve_logfmt_id(p, cmd, NULL, logfmt_id, NULL, 0, NULL,
      resolve_id_on_meta, resolve_id_on_default);
    ck_assert_msg(res == 0, "Failed to handle logfmt_id %u: %s", logfmt_id,
      strerror(errno));
  }

  ck_assert_msg(resolve_on_meta_count == 20,
    "Expected on_meta count 20, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 28,
    "Expected on_default count 28, got %u", resolve_on_default_count);
}
END_TEST

static int resolve_on_meta(pool *jot_pool, pr_jot_ctx_t *jot_ctx,
    unsigned char logfmt_id, const char *jot_hint, const void *val) {
  resolve_on_meta_count++;
  return 0;
}

static int resolve_on_default(pool *jot_pool, pr_jot_ctx_t *jot_ctx,
    unsigned char logfmt_id) {
  resolve_on_default_count++;
  return 0;
}

static int resolve_on_other(pool *jot_pool, pr_jot_ctx_t *jot_ctx,
    unsigned char *text, size_t text_len) {
  resolve_on_other_count++;
  return 0;
}

START_TEST (jot_resolve_logfmt_invalid_test) {
  int res;
  cmd_rec *cmd;
  unsigned char *logfmt;

  mark_point();
  res = pr_jot_resolve_logfmt(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_jot_resolve_logfmt(p, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null cmd");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));

  mark_point();
  res = pr_jot_resolve_logfmt(p, cmd, NULL, NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null logfmt");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  logfmt = (unsigned char *) "";

  mark_point();
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null on_meta");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL, resolve_on_meta,
    NULL, NULL);
  ck_assert_msg(res == 0, "Failed to handle empty logfmt: %s", strerror(errno));
}
END_TEST

START_TEST (jot_resolve_logfmt_basename_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];

  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_BASENAME;
  logfmt[2] = 0;

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "RNTO"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "/foo/bar");
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "RETR"));
  cmd->cmd_class = CL_MISC;
  pr_table_add_dup(cmd->notes, "mod_xfer.retr-path", "foo", 0);
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "APPE"));
  cmd->cmd_class = CL_MISC;
  pr_table_add_dup(cmd->notes, "mod_xfer.store-path", "foo", 0);
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "STOR"));
  cmd->cmd_class = CL_MISC;
  pr_table_add_dup(cmd->notes, "mod_xfer.store-path", "foo", 0);
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  session.xfer.p = p;
  session.xfer.path = pstrdup(p, "/foo/bar/baz");
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  session.xfer.p = NULL;
  session.xfer.path = NULL;

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "CDUP"));
  cmd->cmd_class = CL_MISC;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "XCUP"));
  cmd->cmd_class = CL_MISC;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "PWD"));
  cmd->cmd_class = CL_MISC;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "XPWD"));
  cmd->cmd_class = CL_MISC;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "CWD"));
  cmd->cmd_class = CL_MISC;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "XCWD"));
  cmd->cmd_class = CL_MISC;
  session.chroot_path = "/foo/bar";
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  session.chroot_path = NULL;

  mark_point();
  cmd = pr_cmd_alloc(p, 4, pstrdup(p, "SITE"), pstrdup(p, "chgrp"),
    pstrdup(p, "foo"), pstrdup(p, "bar"));
  cmd->cmd_class = CL_MISC;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 4, pstrdup(p, "SITE"), pstrdup(p, "chmod"),
    pstrdup(p, "foo"), pstrdup(p, "bar"));
  cmd->cmd_class = CL_MISC;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 4, pstrdup(p, "SITE"), pstrdup(p, "UTIME"),
    pstrdup(p, "foo"), pstrdup(p, "bar"));
  cmd->cmd_class = CL_MISC;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "DELE"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "/");
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "LIST"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "/foo/bar/");
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "MDTM"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo/bar");
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "MKD"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo/bar");
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "MLSD"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo/bar");
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "MLST"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo/bar");
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "NLST"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo/bar");
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "RMD"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo/bar");
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "XMKD"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo/bar");
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "XRMD"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo/bar");
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 3, pstrdup(p, "MFMT"), pstrdup(p, "foo"),
    pstrdup(p, "BAR"));
  cmd->cmd_class = CL_MISC;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);
}
END_TEST

START_TEST (jot_resolve_logfmt_bytes_sent_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];

  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_BYTES_SENT;
  logfmt[2] = 0;

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  session.xfer.p = p;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  session.xfer.p = NULL;

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "DELE"));
  cmd->cmd_class = CL_MISC;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);
}
END_TEST

START_TEST (jot_resolve_logfmt_filename_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];

  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_FILENAME;
  logfmt[2] = 0;

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "RNTO"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "RETR"));
  cmd->cmd_class = CL_MISC;
  pr_table_add_dup(cmd->notes, "mod_xfer.retr-path", "foobar", 0);
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "APPE"));
  cmd->cmd_class = CL_MISC;
  pr_table_add_dup(cmd->notes, "mod_xfer.store-path", "foobar", 0);
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "STOR"));
  cmd->cmd_class = CL_MISC;
  pr_table_add_dup(cmd->notes, "mod_xfer.store-path", "foobar", 0);
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  session.xfer.p = p;
  session.xfer.path = pstrdup(p, "foobar");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  session.xfer.p = NULL;
  session.xfer.path = NULL;

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "CDUP"));
  cmd->cmd_class = CL_MISC;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "PWD"));
  cmd->cmd_class = CL_MISC;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "XCUP"));
  cmd->cmd_class = CL_MISC;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "XPWD"));
  cmd->cmd_class = CL_MISC;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "CWD"));
  cmd->cmd_class = CL_MISC;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "XCWD"));
  cmd->cmd_class = CL_MISC;
  session.chroot_path = "/foo/bar/baz";
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  session.chroot_path = NULL;

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 4, pstrdup(p, "SITE"), pstrdup(p, "CHGRP"),
    pstrdup(p, "foo"), pstrdup(p, "bar"));
  cmd->cmd_class = CL_MISC;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 4, pstrdup(p, "SITE"), pstrdup(p, "CHMOD"),
    pstrdup(p, "foo"), pstrdup(p, "bar"));
  cmd->cmd_class = CL_MISC;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 4, pstrdup(p, "SITE"), pstrdup(p, "UTIME"),
    pstrdup(p, "foo"), pstrdup(p, "bar"));
  cmd->cmd_class = CL_MISC;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "DELE"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo bar");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "LIST"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo bar");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "MDTM"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo bar");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "MKD"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo bar");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "MLSD"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo bar");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "MLST"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo bar");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "NLST"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo bar");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "RMD"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo bar");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "XMKD"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo bar");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "XRMD"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo bar");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 3, pstrdup(p, "MFMT"), pstrdup(p, "foo"),
    pstrdup(p, "BAR"));
  cmd->cmd_class = CL_MISC;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);
}
END_TEST

START_TEST (jot_resolve_logfmt_file_modified_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];
  char *modified;

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_FILE_MODIFIED;
  logfmt[2] = 0;

  /* First, without the "mod_xfer.file-modified" note. */
  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  /* Now, with the "mod_xfer.file-modified" note. */
  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  modified = "true";
  pr_table_add_dup(cmd->notes, "mod_xfer.file-modified", modified, 0);
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);
}
END_TEST

START_TEST (jot_resolve_logfmt_file_offset_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];
  double file_offset;

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_FILE_OFFSET;
  logfmt[2] = 0;

  /* First, without the "mod_xfer.file-offset" note. */
  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  /* Now, with the "mod_xfer.file-offset" note. */
  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  file_offset = 2.0;
  pr_table_add(cmd->notes, "mod_xfer.file-offset", &file_offset,
    sizeof(file_offset));
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);
}
END_TEST

START_TEST (jot_resolve_logfmt_file_size_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];
  off_t file_size;

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_FILE_SIZE;
  logfmt[2] = 0;

  /* First, without the "mod_xfer.file-size" note. */
  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  /* Now, with the "mod_xfer.file-size" note. */
  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  file_size = 2.0;
  pr_table_add(cmd->notes, "mod_xfer.file-size", &file_size, sizeof(file_size));
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);
}
END_TEST

START_TEST (jot_resolve_logfmt_env_var_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[9];
  const char *key, *val;

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_ENV_VAR;
  logfmt[2] = 0;

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_ENV_VAR;
  logfmt[2] = LOGFMT_META_START;
  logfmt[3] = LOGFMT_META_ARG;
  logfmt[4] = 'k';
  logfmt[5] = 'e';
  logfmt[6] = 'y';
  logfmt[7] = LOGFMT_META_ARG_END;
  logfmt[8] = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  key = "key";
  val = "val";
  pr_env_set(p, key, val);
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);
  pr_env_unset(p, key);
}
END_TEST

START_TEST (jot_resolve_logfmt_note_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[9];
  const char *key, *val;

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_NOTE_VAR;
  logfmt[2] = 0;

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_NOTE_VAR;
  logfmt[2] = LOGFMT_META_START;
  logfmt[3] = LOGFMT_META_ARG;
  logfmt[4] = 'k';
  logfmt[5] = 'e';
  logfmt[6] = 'y';
  logfmt[7] = LOGFMT_META_ARG_END;
  logfmt[8] = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  key = "key";
  val = "val";
  session.notes = pr_table_alloc(p, 0);
  pr_table_add_dup(session.notes, key, val, 0);
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  pr_table_add_dup(cmd->notes, key, val, 0);
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  pr_table_empty(session.notes);
  pr_table_free(session.notes);
  session.notes = NULL;
}
END_TEST

START_TEST (jot_resolve_logfmt_remote_port_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];
  conn_t *conn;
  const pr_netaddr_t *addr;

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_REMOTE_PORT;
  logfmt[2] = 0;

  /* First, without a session remote address. */
  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  /* Now, with a session remote address. */
  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;

  mark_point();
  conn = pr_inet_create_conn(p, -1, NULL, INPORT_ANY, FALSE);
  session.c = conn;
  addr = pr_netaddr_get_addr(p, "127.0.0.1", NULL);
  ck_assert_msg(addr != NULL, "Failed to get address: %s", strerror(errno));
  pr_netaddr_set_port2((pr_netaddr_t *) addr, 2476);
  session.c->remote_addr = addr;
  pr_netaddr_set_sess_addrs();

  mark_point();
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  pr_inet_close(p, session.c);
  session.c = NULL;
}
END_TEST

START_TEST (jot_resolve_logfmt_rfc1413_ident_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];
  char *ident_user;

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_IDENT_USER;
  logfmt[2] = 0;

  /* First, without the "mod_ident.rfc1413-ident" note. */
  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  /* Now, with the "mod_ident.rfc1413-ident" note. */
  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  ident_user = "FOOBAR";
  session.notes = pr_table_alloc(p, 0);
  pr_table_add_dup(session.notes, "mod_ident.rfc1413-ident", ident_user, 0);
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  pr_table_empty(session.notes);
  pr_table_free(session.notes);
  session.notes = NULL;
}
END_TEST

START_TEST (jot_resolve_logfmt_xfer_secs_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_SECONDS;
  logfmt[2] = 0;

  /* First, without the sess.xfer.p pool. */
  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  /* Now, with the session.xfer.p pool, but no transfer times. */
  mark_point();
  session.xfer.p = p;
  session.xfer.start_time.tv_sec = session.xfer.start_time.tv_usec = 0;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  session.xfer.p = p;
  session.xfer.start_time.tv_sec = 0;
  session.xfer.start_time.tv_usec = 200000;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  session.xfer.p = p;
  session.xfer.start_time.tv_sec = 20;
  session.xfer.start_time.tv_usec = 200;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  session.xfer.p = NULL;
  session.xfer.start_time.tv_sec = session.xfer.start_time.tv_usec = 0;
}
END_TEST

START_TEST (jot_resolve_logfmt_xfer_ms_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_XFER_MS;
  logfmt[2] = 0;

  /* First, without the sess.xfer.p pool. */
  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  /* Now, with the session.xfer.p pool, but no transfer times. */
  mark_point();
  session.xfer.p = p;
  session.xfer.start_time.tv_sec = session.xfer.start_time.tv_usec = 0;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  session.xfer.p = p;
  session.xfer.start_time.tv_sec = 0;
  session.xfer.start_time.tv_usec = 200000;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  session.xfer.p = p;
  session.xfer.start_time.tv_sec = 20;
  session.xfer.start_time.tv_usec = 200;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  session.xfer.p = NULL;
  session.xfer.start_time.tv_sec = session.xfer.start_time.tv_usec = 0;
}
END_TEST

START_TEST (jot_resolve_logfmt_command_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_COMMAND;
  logfmt[2] = 0;

  mark_point();
  cmd->cmd_class = CL_CONNECT;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd->cmd_class = CL_DISCONNECT;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "PASS"));
  cmd->cmd_class = CL_AUTH;
  session.hide_password = TRUE;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "PASS"));
  cmd->cmd_class = CL_AUTH;
  session.hide_password = FALSE;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "ADAT"));
  cmd->cmd_class = CL_SEC;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);
}
END_TEST

START_TEST (jot_resolve_logfmt_local_name_test) {
  int res;
  cmd_rec *cmd;
  server_rec *server;
  unsigned char logfmt[3];

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_LOCAL_NAME;
  logfmt[2] = 0;

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  server = pcalloc(p, sizeof(server_rec));
  server->ServerName = "FOOBAR";
  cmd->server = server;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);
}
END_TEST

START_TEST (jot_resolve_logfmt_local_port_test) {
  int res;
  cmd_rec *cmd;
  server_rec *server;
  unsigned char logfmt[3];

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_LOCAL_PORT;
  logfmt[2] = 0;

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  server = pcalloc(p, sizeof(server_rec));
  server->ServerPort = 2121;
  cmd->server = server;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);
}
END_TEST

START_TEST (jot_resolve_logfmt_user_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_USER;
  logfmt[2] = 0;

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  session.user = "FOOBAR";
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  session.user = NULL;
}
END_TEST

START_TEST (jot_resolve_logfmt_uid_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_UID;
  logfmt[2] = 0;

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  session.auth_mech = "mod_auth_file.c";
  session.login_uid = 400;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  session.auth_mech = NULL;
  session.login_uid = 0;
}
END_TEST

START_TEST (jot_resolve_logfmt_group_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_GROUP;
  logfmt[2] = 0;

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  session.group = "FOOBAR";
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  session.group = NULL;
}
END_TEST

START_TEST (jot_resolve_logfmt_gid_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_GID;
  logfmt[2] = 0;

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  session.auth_mech = "mod_auth_file.c";
  session.login_gid = 400;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  session.auth_mech = NULL;
  session.login_gid = 0;
}
END_TEST

START_TEST (jot_resolve_logfmt_original_user_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];
  char *orig_user;

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_ORIGINAL_USER;
  logfmt[2] = 0;

  /* First, without the "mod_auth.orig-user" note. */
  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  /* Now, with the "mod_auth.orig-user" note. */
  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  orig_user = "FOOBAR";
  session.notes = pr_table_alloc(p, 0);
  pr_table_add_dup(session.notes, "mod_auth.orig-user", orig_user, 0);
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  pr_table_empty(session.notes);
  pr_table_free(session.notes);
  session.notes = NULL;
}
END_TEST

START_TEST (jot_resolve_logfmt_response_code_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_RESPONSE_CODE;
  logfmt[2] = 0;

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "QUIT"));
  cmd->cmd_class = CL_MISC;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;

  pr_response_set_pool(p);
  pr_response_add("200", "foo %s", "bar");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  pr_response_clear(&resp_list);
  pr_response_set_pool(NULL);
}
END_TEST

START_TEST (jot_resolve_logfmt_response_text_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_RESPONSE_STR;
  logfmt[2] = 0;

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;

  pr_response_set_pool(p);
  pr_response_add("200", "foo %s", "bar");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  pr_response_clear(&resp_list);
  pr_response_set_pool(NULL);
}
END_TEST

START_TEST (jot_resolve_logfmt_response_ms_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];
  uint64_t start_ms;

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_RESPONSE_MS;
  logfmt[2] = 0;

  /* First, without the "start_ms" note. */
  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  /* Now, with the "start_ms" note. */
  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  start_ms = 123456;
  pr_table_add(cmd->notes, "start_ms", &start_ms, sizeof(start_ms));
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);
}
END_TEST

START_TEST (jot_resolve_logfmt_class_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_CLASS;
  logfmt[2] = 0;

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  session.conn_class = pcalloc(p, sizeof(pr_class_t));
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  session.conn_class = NULL;
}
END_TEST

START_TEST (jot_resolve_logfmt_anon_passwd_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];
  char *anon_passwd;

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_ANON_PASS;
  logfmt[2] = 0;

  /* First, without the "mod_auth.anon-passwd" note. */
  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  /* Now, with the "mod_auth.anon-passwd" note. */
  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  anon_passwd = "FOOBAR";
  session.notes = pr_table_alloc(p, 0);
  pr_table_add_dup(session.notes, "mod_auth.anon-passwd", anon_passwd, 0);
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  pr_table_empty(session.notes);
  pr_table_free(session.notes);
  session.notes = NULL;
}
END_TEST

START_TEST (jot_resolve_logfmt_method_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_CONNECT;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_METHOD;
  logfmt[2] = 0;

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd->cmd_class = CL_MISC;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "SITE"));
  cmd->cmd_class = CL_MISC;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 2, pstrdup(p, "SITE"), pstrdup(p, "foobar"));
  cmd->cmd_class = CL_MISC;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);
}
END_TEST

START_TEST (jot_resolve_logfmt_xfer_path_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];

  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_XFER_PATH;
  logfmt[2] = 0;

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "RNTO"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  session.xfer.p = p;
  session.xfer.path = pstrdup(p, "foo/bar");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  session.xfer.p = NULL;
  session.xfer.path = NULL;

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "DELE"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo/bar");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "MKD"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo/bar");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "XMKD"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo/bar");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "RMD"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo/bar");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "XRMD"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo/bar");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);
}
END_TEST

START_TEST (jot_resolve_logfmt_xfer_port_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];

  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_XFER_PORT;
  logfmt[2] = 0;

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "PASV"));
  cmd->cmd_class = CL_MISC;
  session.data_port = 7777;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "PORT"));
  cmd->cmd_class = CL_MISC;
  session.data_port = 7777;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "EPRT"));
  cmd->cmd_class = CL_MISC;
  session.data_port = 7777;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "EPSV"));
  cmd->cmd_class = CL_MISC;
  session.data_port = 7777;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "APPE"));
  cmd->cmd_class = CL_MISC;
  session.data_port = 7777;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "LIST"));
  cmd->cmd_class = CL_MISC;
  session.data_port = 7777;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "MLSD"));
  cmd->cmd_class = CL_MISC;
  session.data_port = 7777;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "NLST"));
  cmd->cmd_class = CL_MISC;
  session.data_port = 7777;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "RETR"));
  cmd->cmd_class = CL_MISC;
  session.data_port = 7777;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "STOR"));
  cmd->cmd_class = CL_MISC;
  session.data_port = 7777;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "STOU"));
  cmd->cmd_class = CL_MISC;
  session.data_port = 7777;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  session.data_port = 0;
}
END_TEST

START_TEST (jot_resolve_logfmt_xfer_type_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];

  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_XFER_TYPE;
  logfmt[2] = 0;

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "APPE"));
  cmd->cmd_class = CL_MISC;
  session.sf_flags = SF_ASCII;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "LIST"));
  cmd->cmd_class = CL_MISC;
  session.sf_flags = SF_ASCII_OVERRIDE;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  session.sf_flags = 0;

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "MLSD"));
  cmd->cmd_class = CL_MISC;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "RETR"));
  cmd->cmd_class = CL_MISC;
  tests_stubs_set_protocol("sftp");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "STOR"));
  cmd->cmd_class = CL_MISC;
  tests_stubs_set_protocol("scp");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  tests_stubs_set_protocol(NULL);
}
END_TEST

START_TEST (jot_resolve_logfmt_xfer_status_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];

  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_XFER_STATUS;
  logfmt[2] = 0;

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "RETR"));
  cmd->cmd_class = CL_MISC;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "RETR"));
  cmd->cmd_class = CL_MISC;
  tests_stubs_set_protocol("ftps");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  tests_stubs_set_protocol(NULL);

  /* 5xx response */
  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "RETR"));
  cmd->cmd_class = CL_MISC;
  pr_response_set_pool(p);
  pr_response_add("500", "foo %s", "bar");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  pr_response_clear(&resp_list);

  /* 1xx response */
  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "RETR"));
  cmd->cmd_class = CL_MISC;
  pr_response_set_pool(p);
  pr_response_add("123", "foo %s", "bar");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  pr_response_clear(&resp_list);

  /* 2xx response */
  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "RETR"));
  cmd->cmd_class = CL_MISC;
  pr_response_set_pool(p);
  pr_response_add("234", "foo %s", "bar");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  pr_response_clear(&resp_list);

  /* ABOR */
  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "ABOR"));
  cmd->cmd_class = CL_MISC;
  pr_response_set_pool(p);
  pr_response_add("234", "foo %s", "bar");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  pr_response_clear(&resp_list);

  /* SF_ABORT */
  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "RETR"));
  cmd->cmd_class = CL_MISC;
  session.sf_flags = SF_ABORT;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  session.sf_flags = 0;
  pr_response_clear(&resp_list);
  pr_response_set_pool(NULL);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "RETR"));
  cmd->cmd_class = CL_MISC;
  tests_stubs_set_protocol("sftp");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "STOR"));
  cmd->cmd_class = CL_MISC;
  pr_table_add_dup(cmd->notes, "mod_sftp.file-status", "FOO!", 0);
  tests_stubs_set_protocol("sftp");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  tests_stubs_set_protocol(NULL);
}
END_TEST

START_TEST (jot_resolve_logfmt_xfer_failure_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];

  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_XFER_FAILURE;
  logfmt[2] = 0;

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "RETR"));
  cmd->cmd_class = CL_MISC;
  tests_stubs_set_protocol("ftps");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  tests_stubs_set_protocol(NULL);

  /* SF_ABORT */
  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "RETR"));
  cmd->cmd_class = CL_MISC;
  session.sf_flags = SF_ABORT;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  session.sf_flags = 0;

  /* 5xx response */
  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "RETR"));
  cmd->cmd_class = CL_MISC;
  pr_response_set_pool(p);
  pr_response_add("543", "foo %s", "BAR BAZ");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  pr_response_clear(&resp_list);
  pr_response_set_pool(NULL);

  /* 1xx response */
  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "RETR"));
  cmd->cmd_class = CL_MISC;
  pr_response_set_pool(p);
  pr_response_add("123", "foo %s", "BAR BAZ");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  pr_response_clear(&resp_list);
  pr_response_set_pool(NULL);

  /* 2xx response */
  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "RETR"));
  cmd->cmd_class = CL_MISC;
  pr_response_set_pool(p);
  pr_response_add("234", "foo %s", "BAR BAZ");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  pr_response_clear(&resp_list);
  pr_response_set_pool(NULL);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "RETR"));
  cmd->cmd_class = CL_MISC;
  tests_stubs_set_protocol("sftp");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  tests_stubs_set_protocol(NULL);
}
END_TEST

START_TEST (jot_resolve_logfmt_dir_name_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];

  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_DIR_NAME;
  logfmt[2] = 0;

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "CDUP"));
  cmd->cmd_class = CL_MISC;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "CWD"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "LIST"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "/foo");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "MLSD"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "/foo/");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "MKD"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo/");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "NLST"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo/");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "RMD"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo/");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "XCWD"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo/");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "XCUP"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo/");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "XMKD"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo/");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "XRMD"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "foo/");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);
}
END_TEST

START_TEST (jot_resolve_logfmt_dir_path_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];

  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_DIR_PATH;
  logfmt[2] = 0;

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "CDUP"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "/foo/bar/baz");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "LIST"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "/foo/bar/baz");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "MKD"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "/foo/bar/baz");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "MLSD"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "/foo/bar/baz");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "NLST"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "/foo/bar/baz");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "RMD"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "/foo/bar/baz");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "XCUP"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "/foo/bar/baz");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "XMKD"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "/foo/bar/baz");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "XRMD"));
  cmd->cmd_class = CL_MISC;
  cmd->arg = pstrdup(p, "/foo/bar/baz");
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "CWD"));
  cmd->cmd_class = CL_MISC;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "CWD"));
  cmd->cmd_class = CL_MISC;
  session.chroot_path = "/foo/bar/baz/";
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  session.chroot_path = NULL;
}
END_TEST

START_TEST (jot_resolve_logfmt_cmd_params_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];

  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_CMD_PARAMS;
  logfmt[2] = 0;

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_CONNECT;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd->cmd_class = CL_DISCONNECT;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd->cmd_class = CL_MISC;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 2, pstrdup(p, "FOO"), pstrdup(p, "bar"));
  cmd->arg = pstrdup(p, "baz");
  cmd->cmd_class = CL_MISC;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "ADAT"));
  cmd->cmd_class = CL_SEC;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "PASS"));
  cmd->cmd_class = CL_AUTH;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);
}
END_TEST

START_TEST (jot_resolve_logfmt_rename_from_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];
  char *rnfr_path;

  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_RENAME_FROM;
  logfmt[2] = 0;

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "RNTO"));
  cmd->cmd_class = CL_MISC;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  rnfr_path = "FOOBAR";
  session.notes = pr_table_alloc(p, 0);
  pr_table_add_dup(session.notes, "mod_core.rnfr-path", rnfr_path, 0);
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  pr_table_empty(session.notes);
  pr_table_free(session.notes);
  session.notes = NULL;
}
END_TEST

START_TEST (jot_resolve_logfmt_eos_reason_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_EOS_REASON;
  logfmt[2] = 0;

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  session.disconnect_reason = -1;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  session.disconnect_reason = PR_SESS_DISCONNECT_BANNED;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  session.disconnect_reason = 0;
}
END_TEST

START_TEST (jot_resolve_logfmt_vhost_ip_test) {
  int res;
  cmd_rec *cmd;
  server_rec *server;
  unsigned char logfmt[3];

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_VHOST_IP;
  logfmt[2] = 0;

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  mark_point();
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  server = pcalloc(p, sizeof(server_rec));
  server->ServerAddress = "FOOBAR";
  cmd->server = server;
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);
}
END_TEST

START_TEST (jot_resolve_logfmt_filters_test) {
  int res;
  cmd_rec *cmd;
  pr_jot_filters_t *jot_filters;
  unsigned char logfmt[3];

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_CONNECT;

  /* No filters; should be implicitly jottable. */
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  jot_filters = NULL;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_CONNECT;
  logfmt[2] = 0;

  mark_point();
  res = pr_jot_resolve_logfmt(p, cmd, jot_filters, logfmt, NULL,
    resolve_on_meta, NULL, NULL);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);

  /* With an ALL filter, and no command class. */
  cmd->cmd_class = 0;
  logfmt[1] = LOGFMT_META_COMMAND;
  resolve_on_meta_count = resolve_on_default_count = 0;
  jot_filters = pr_jot_filters_create(p, "ALL",
    PR_JOT_FILTER_TYPE_CLASSES, PR_JOT_FILTER_FL_ALL_INCL_ALL);

  mark_point();
  res = pr_jot_resolve_logfmt(p, cmd, jot_filters, logfmt, NULL,
    resolve_on_meta, NULL, NULL);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);

  /* With explicit filters that allow the class. */
  cmd->cmd_class = CL_CONNECT;
  logfmt[1] = LOGFMT_META_CONNECT;
  resolve_on_meta_count = resolve_on_default_count = 0;
  jot_filters = pr_jot_filters_create(p, "CONNECT",
    PR_JOT_FILTER_TYPE_CLASSES, 0);

  mark_point();
  res = pr_jot_resolve_logfmt(p, cmd, jot_filters, logfmt, NULL,
    resolve_on_meta, NULL, NULL);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);

  /* With explicit filters that ignore the class. */
  resolve_on_meta_count = resolve_on_default_count = 0;
  jot_filters = pr_jot_filters_create(p, "!CONNECT",
    PR_JOT_FILTER_TYPE_CLASSES, 0);

  mark_point();
  res = pr_jot_resolve_logfmt(p, cmd, jot_filters, logfmt, NULL,
    resolve_on_meta, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle filtered logfmt");
  ck_assert_msg(errno == EPERM, "Expected EPERM (%d), got %s (%d)", EPERM,
    strerror(errno), errno);
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);

  /* With explicit filters that do not match the class. */
  resolve_on_meta_count = resolve_on_default_count = 0;
  jot_filters = pr_jot_filters_create(p, "DISCONNECT",
    PR_JOT_FILTER_TYPE_CLASSES, 0);

  mark_point();
  res = pr_jot_resolve_logfmt(p, cmd, jot_filters, logfmt, NULL,
    resolve_on_meta, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle filtered logfmt");
  ck_assert_msg(errno == EPERM, "Expected EPERM (%d), got %s (%d)", EPERM,
    strerror(errno), errno);
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);

  /* With explicit filters that allow the command. Note that this REQUIRES
   * that we use a known command, since allowed command comparisons are done
   * by ID.
   */
  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "RANG"));
  cmd->cmd_class = CL_CONNECT;
  resolve_on_meta_count = resolve_on_default_count = 0;
  jot_filters = pr_jot_filters_create(p, "RANG",
    PR_JOT_FILTER_TYPE_COMMANDS, 0);

  mark_point();
  res = pr_jot_resolve_logfmt(p, cmd, jot_filters, logfmt, NULL,
    resolve_on_meta, NULL, NULL);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);

  /* With explicit filters that ignore the command. */
  resolve_on_meta_count = resolve_on_default_count = 0;
  jot_filters = pr_jot_filters_create(p, "!RANG",
    PR_JOT_FILTER_TYPE_COMMANDS, 0);

  mark_point();
  res = pr_jot_resolve_logfmt(p, cmd, jot_filters, logfmt, NULL,
    resolve_on_meta, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle filtered logfmt");
  ck_assert_msg(errno == EPERM, "Expected EPERM (%d), got %s (%d)", EPERM,
    strerror(errno), errno);
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);

  /* With explicit filters that do not match the command. */
  resolve_on_meta_count = resolve_on_default_count = 0;
  jot_filters = pr_jot_filters_create(p, "FOO",
    PR_JOT_FILTER_TYPE_COMMANDS, 0);

  mark_point();
  res = pr_jot_resolve_logfmt(p, cmd, jot_filters, logfmt, NULL,
    resolve_on_meta, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle filtered logfmt");
  ck_assert_msg(errno == EPERM, "Expected EPERM (%d), got %s (%d)", EPERM,
    strerror(errno), errno);
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
}
END_TEST

START_TEST (jot_resolve_logfmt_on_default_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_BASENAME;
  logfmt[2] = 0;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;

  mark_point();
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, NULL);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 1,
    "Expected on_default count 1, got %u", resolve_on_default_count);
}
END_TEST

START_TEST (jot_resolve_logfmt_on_other_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  logfmt[0] = 'A';
  logfmt[1] = '!';
  logfmt[2] = 0;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;

  mark_point();
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 1,
    "Expected on_other count 1, got %u", resolve_on_other_count);
}
END_TEST

START_TEST (jot_resolve_logfmt_connect_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_CONNECT;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_CONNECT;
  logfmt[2] = 0;

  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;

  mark_point();
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd->cmd_class = CL_DISCONNECT;

  mark_point();
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);
}
END_TEST

START_TEST (jot_resolve_logfmt_disconnect_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_DISCONNECT;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_DISCONNECT;
  logfmt[2] = 0;

  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;

  mark_point();
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);

  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;
  cmd->cmd_class = CL_CONNECT;

  mark_point();
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 0,
    "Expected on_meta count 0, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);
}
END_TEST

START_TEST (jot_resolve_logfmt_custom_test) {
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[10];

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  cmd->cmd_class = CL_MISC;
  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_CUSTOM;
  logfmt[2] = LOGFMT_META_START;
  logfmt[3] = LOGFMT_META_ARG;
  logfmt[4] = '%';
  logfmt[5] = '{';
  logfmt[6] = '0';
  logfmt[7] = '}';
  logfmt[8] = LOGFMT_META_ARG_END;
  logfmt[9] = 0;

  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;

  mark_point();
  res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
    resolve_on_meta, resolve_on_default, resolve_on_other);
  ck_assert_msg(res == 0, "Failed to handle logfmt: %s", strerror(errno));
  ck_assert_msg(resolve_on_meta_count == 1,
    "Expected on_meta count 1, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 0,
    "Expected on_default count 0, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);
}
END_TEST

START_TEST (jot_resolve_logfmts_test) {
  register unsigned char i;
  int res;
  cmd_rec *cmd;
  unsigned char logfmt[3];

  cmd = pr_cmd_alloc(p, 1, pstrdup(p, "FOO"));
  logfmt[0] = LOGFMT_META_START;
  logfmt[2] = 0;
  resolve_on_meta_count = resolve_on_default_count = resolve_on_other_count = 0;

  /* Currently, the max known LogFormat meta/ID is 53 (DISCONNECT). */
  for (i = 1; i < 54; i++) {
    logfmt[1] = i;

    mark_point();
    res = pr_jot_resolve_logfmt(p, cmd, NULL, logfmt, NULL,
      resolve_on_meta, resolve_on_default, resolve_on_other);
    ck_assert_msg(res == 0, "Failed to handle logfmt_id %u: %s", logfmt[1],
      strerror(errno));
  }

  ck_assert_msg(resolve_on_meta_count == 20,
    "Expected on_meta count 20, got %u", resolve_on_meta_count);
  ck_assert_msg(resolve_on_default_count == 28,
    "Expected on_default count 28, got %u", resolve_on_default_count);
  ck_assert_msg(resolve_on_other_count == 0,
    "Expected on_other count 0, got %u", resolve_on_other_count);
}
END_TEST

static unsigned int scan_on_meta_count = 0;

static int scan_on_meta(pool *jot_pool, pr_jot_ctx_t *jot_ctx,
    unsigned char logfmt_id, const char *logfmt_data, size_t logfmt_datalen) {
  scan_on_meta_count++;
  return 0;
}

START_TEST (jot_scan_logfmt_test) {
  int res;
  unsigned char logfmt[12];

  mark_point();
  res = pr_jot_scan_logfmt(NULL, NULL, 0, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_jot_scan_logfmt(p, NULL, 0, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null logfmt");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  logfmt[0] = LOGFMT_META_START;
  logfmt[1] = LOGFMT_META_CUSTOM;
  logfmt[2] = LOGFMT_META_START;
  logfmt[3] = LOGFMT_META_ARG;
  logfmt[4] = '%';
  logfmt[5] = '{';
  logfmt[6] = 'f';
  logfmt[7] = 'o';
  logfmt[8] = 'o';
  logfmt[9] = '}';
  logfmt[10] = LOGFMT_META_ARG_END;
  logfmt[11] = 0;

  mark_point();
  res = pr_jot_scan_logfmt(p, logfmt, 0, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null on_meta");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_jot_scan_logfmt(p, logfmt, 0, NULL, scan_on_meta, 0);
  ck_assert_msg(res < 0, "Failed to handle invalid LogFormat ID");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  scan_on_meta_count = 0;

  mark_point();
  res = pr_jot_scan_logfmt(p, logfmt, LOGFMT_META_ENV_VAR, NULL, scan_on_meta,
    0);
  ck_assert_msg(res == 0, "Failed to scan logmt for ENV_VAR: %s",
    strerror(errno));
  ck_assert_msg(scan_on_meta_count == 0, "Expected scan_on_meta 0, got %u",
    scan_on_meta_count);

  scan_on_meta_count = 0;

  mark_point();
  res = pr_jot_scan_logfmt(p, logfmt, LOGFMT_META_CUSTOM, NULL, scan_on_meta,
    0);
  ck_assert_msg(res == 0, "Failed to scan logmt for CUSTOM: %s",
    strerror(errno));
  ck_assert_msg(scan_on_meta_count == 1, "Expected scan_on_meta 1, got %u",
    scan_on_meta_count);
}
END_TEST

START_TEST (jot_on_json_test) {
  pr_jot_ctx_t *ctx;
  pr_json_object_t *json;
  double num;
  int res, truth;
  const char *text;

  mark_point();
  res = pr_jot_on_json(NULL, NULL, 0, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_jot_on_json(p, NULL, 0, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null ctx");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  ctx = pcalloc(p, sizeof(pr_jot_ctx_t));

  mark_point();
  res = pr_jot_on_json(p, ctx, 0, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null val");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_jot_on_json(p, ctx, 0, NULL, &num);
  ck_assert_msg(res < 0, "Failed to handle null ctx->log");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  json = pr_json_object_alloc(p);
  ctx->log = json;

  mark_point();
  res = pr_jot_on_json(p, ctx, 0, NULL, &num);
  ck_assert_msg(res < 0, "Failed to handle null ctx->user_data");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  ctx->user_data = pr_table_alloc(p, 0);

  mark_point();
  res = pr_jot_on_json(p, ctx, 0, NULL, &num);
  ck_assert_msg(res < 0, "Failed to handle null JSON info");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  ctx->user_data = pr_jot_get_logfmt2json(p);

  mark_point();
  truth = FALSE;
  res = pr_jot_on_json(p, ctx, LOGFMT_META_CONNECT, NULL, &truth);
  ck_assert_msg(res == 0, "Failed to handle LOGFMT_META_CONNECT: %s",
    strerror(errno));

  mark_point();
  num = 2476;
  res = pr_jot_on_json(p, ctx, LOGFMT_META_PID, NULL, &num);
  ck_assert_msg(res == 0, "Failed to handle LOGFMT_META_PID: %s",
    strerror(errno));

  mark_point();
  text = "lorem ipsum";
  res = pr_jot_on_json(p, ctx, LOGFMT_META_IDENT_USER, NULL, text);
  ck_assert_msg(res == 0, "Failed to handle LOGFMT_META_IDENT_USER: %s",
    strerror(errno));

  mark_point();
  text = "alef bet vet";
  res = pr_jot_on_json(p, ctx, LOGFMT_META_USER, "USER_KEY", text);
  ck_assert_msg(res == 0, "Failed to handle LOGFMT_META_USER: %s",
    strerror(errno));

  (void) pr_json_object_free(json);
}
END_TEST

START_TEST (jot_get_logfmt2json_test) {
  pr_table_t *res;

  mark_point();
  res = pr_jot_get_logfmt2json(NULL);
  ck_assert_msg(res == NULL, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_jot_get_logfmt2json(p);
  ck_assert_msg(res != NULL, "Failed to get map: %s", strerror(errno));
}
END_TEST

START_TEST (jot_get_logfmt_id_name_test) {
  register unsigned char i;
  const char *res;

  mark_point();
  res = pr_jot_get_logfmt_id_name(0);
  ck_assert_msg(res == NULL, "Failed to handle invalid logfmt_id");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  /* Currently, the max known LogFormat meta/ID is 54 (XFER_PORT). */
  for (i = 2; i < 55; i++) {
    mark_point();
    res = pr_jot_get_logfmt_id_name(i);
    ck_assert_msg(res != NULL, "Failed to get name for LogFormat ID %u: %s",
      i, strerror(errno)); 
  }

  res = pr_jot_get_logfmt_id_name(LOGFMT_META_CUSTOM);
  ck_assert_msg(res != NULL, "Failed to get name for LogFormat ID %u: %s",
    LOGFMT_META_CUSTOM, strerror(errno)); 
}
END_TEST

Suite *tests_get_jot_suite(void) {
  Suite *suite;
  TCase *testcase;

  suite = suite_create("jot");
  testcase = tcase_create("base");
  tcase_add_checked_fixture(testcase, set_up, tear_down);

  tcase_add_test(testcase, jot_filters_create_test);
  tcase_add_test(testcase, jot_filters_destroy_test);
  tcase_add_test(testcase, jot_filters_include_classes_test);

  tcase_add_test(testcase, jot_parse_on_meta_test);
  tcase_add_test(testcase, jot_parse_on_unknown_test);
  tcase_add_test(testcase, jot_parse_on_other_test);
  tcase_add_test(testcase, jot_parse_logfmt_test);
  tcase_add_test(testcase, jot_parse_logfmt_short_vars_test);
  tcase_add_test(testcase, jot_parse_logfmt_long_vars_test);
  tcase_add_test(testcase, jot_parse_logfmt_custom_vars_test);

  tcase_add_test(testcase, jot_resolve_logfmt_id_test);
  tcase_add_test(testcase, jot_resolve_logfmt_id_on_default_test);
  tcase_add_test(testcase, jot_resolve_logfmt_id_filters_test);
  tcase_add_test(testcase, jot_resolve_logfmt_id_connect_test);
  tcase_add_test(testcase, jot_resolve_logfmt_id_disconnect_test);
  tcase_add_test(testcase, jot_resolve_logfmt_id_custom_test);
  tcase_add_test(testcase, jot_resolve_logfmt_ids_test);
  tcase_add_test(testcase, jot_resolve_logfmt_invalid_test);
  tcase_add_test(testcase, jot_resolve_logfmt_basename_test);
  tcase_add_test(testcase, jot_resolve_logfmt_bytes_sent_test);
  tcase_add_test(testcase, jot_resolve_logfmt_filename_test);
  tcase_add_test(testcase, jot_resolve_logfmt_file_modified_test);
  tcase_add_test(testcase, jot_resolve_logfmt_file_offset_test);
  tcase_add_test(testcase, jot_resolve_logfmt_file_size_test);
  tcase_add_test(testcase, jot_resolve_logfmt_env_var_test);
  tcase_add_test(testcase, jot_resolve_logfmt_note_test);
  tcase_add_test(testcase, jot_resolve_logfmt_remote_port_test);
  tcase_add_test(testcase, jot_resolve_logfmt_rfc1413_ident_test);
  tcase_add_test(testcase, jot_resolve_logfmt_xfer_secs_test);
  tcase_add_test(testcase, jot_resolve_logfmt_xfer_ms_test);
  tcase_add_test(testcase, jot_resolve_logfmt_command_test);
  tcase_add_test(testcase, jot_resolve_logfmt_local_name_test);
  tcase_add_test(testcase, jot_resolve_logfmt_local_port_test);
  tcase_add_test(testcase, jot_resolve_logfmt_user_test);
  tcase_add_test(testcase, jot_resolve_logfmt_uid_test);
  tcase_add_test(testcase, jot_resolve_logfmt_group_test);
  tcase_add_test(testcase, jot_resolve_logfmt_gid_test);
  tcase_add_test(testcase, jot_resolve_logfmt_original_user_test);
  tcase_add_test(testcase, jot_resolve_logfmt_response_code_test);
  tcase_add_test(testcase, jot_resolve_logfmt_response_text_test);
  tcase_add_test(testcase, jot_resolve_logfmt_response_ms_test);
  tcase_add_test(testcase, jot_resolve_logfmt_class_test);
  tcase_add_test(testcase, jot_resolve_logfmt_anon_passwd_test);
  tcase_add_test(testcase, jot_resolve_logfmt_method_test);
  tcase_add_test(testcase, jot_resolve_logfmt_xfer_path_test);
  tcase_add_test(testcase, jot_resolve_logfmt_xfer_port_test);
  tcase_add_test(testcase, jot_resolve_logfmt_xfer_type_test);
  tcase_add_test(testcase, jot_resolve_logfmt_xfer_status_test);
  tcase_add_test(testcase, jot_resolve_logfmt_xfer_failure_test);
  tcase_add_test(testcase, jot_resolve_logfmt_dir_name_test);
  tcase_add_test(testcase, jot_resolve_logfmt_dir_path_test);
  tcase_add_test(testcase, jot_resolve_logfmt_cmd_params_test);
  tcase_add_test(testcase, jot_resolve_logfmt_rename_from_test);
  tcase_add_test(testcase, jot_resolve_logfmt_eos_reason_test);
  tcase_add_test(testcase, jot_resolve_logfmt_vhost_ip_test);
  tcase_add_test(testcase, jot_resolve_logfmt_filters_test);
  tcase_add_test(testcase, jot_resolve_logfmt_on_default_test);
  tcase_add_test(testcase, jot_resolve_logfmt_on_other_test);
  tcase_add_test(testcase, jot_resolve_logfmt_connect_test);
  tcase_add_test(testcase, jot_resolve_logfmt_disconnect_test);
  tcase_add_test(testcase, jot_resolve_logfmt_custom_test);
  tcase_add_test(testcase, jot_resolve_logfmts_test);

  tcase_add_test(testcase, jot_scan_logfmt_test);
  tcase_add_test(testcase, jot_on_json_test);
  tcase_add_test(testcase, jot_get_logfmt2json_test);
  tcase_add_test(testcase, jot_get_logfmt_id_name_test);

  suite_add_tcase(suite, testcase);
  return suite;
}
