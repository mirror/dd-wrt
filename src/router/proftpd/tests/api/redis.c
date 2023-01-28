/*
 * ProFTPD - FTP server testsuite
 * Copyright (c) 2017-2021 The ProFTPD Project team
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

/* Redis API tests. */

#include "tests.h"

#ifdef PR_USE_REDIS

static pool *p = NULL;
static const char *redis_server = "127.0.0.1";
static int redis_port = 6379;

/* Fixtures */

static void set_up(void) {
  const char *key, *val;

  if (p == NULL) {
    p = permanent_pool = make_sub_pool(NULL);
  }

  redis_init();

  key = "REDIS_HOST";
  val = getenv(key);
  if (val != NULL) {
    redis_server = val;
  }

  key = "REDIS_PORT";
  val = getenv(key);
  if (val != NULL) {
    redis_port = atoi(val);
  }

  redis_set_server(redis_server, redis_port, 0UL, NULL, NULL);

  if (getenv("TEST_VERBOSE") != NULL) {
    pr_trace_set_levels("redis", 1, 20);
  }
}

static void tear_down(void) {
  if (getenv("TEST_VERBOSE") != NULL) {
    pr_trace_set_levels("redis", 0, 0);
  }

  redis_clear();

  if (p) {
    destroy_pool(p);
    p = permanent_pool = NULL;
  }
}

/* Tests */

START_TEST (redis_conn_destroy_test) {
  int res;

  mark_point();
  res = pr_redis_conn_destroy(NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
}
END_TEST

START_TEST (redis_conn_close_test) {
  int res;

  mark_point();
  res = pr_redis_conn_close(NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
}
END_TEST

START_TEST (redis_conn_new_test) {
  int res;
  pr_redis_t *redis;

  mark_point();
  redis = pr_redis_conn_new(NULL, NULL, 0);
  ck_assert_msg(redis == NULL, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));

  if (getenv("CI") == NULL &&
      getenv("CIRRUS_CLONE_DEPTH") == NULL &&
      getenv("TRAVIS") == NULL) {
    /* Now deliberately set the wrong server and port. */
    redis_set_server("127.1.2.3", redis_port, 0UL, NULL, NULL);

    mark_point();
    redis = pr_redis_conn_new(p, NULL, 0);
    ck_assert_msg(redis == NULL, "Failed to handle invalid address");
    ck_assert_msg(errno == EIO, "Expected EIO (%d), got %s (%d)", EIO,
      strerror(errno), errno);
  }

  redis_set_server(redis_server, 1020, 0UL, NULL, NULL);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis == NULL, "Failed to handle invalid port");
  ck_assert_msg(errno == EIO, "Expected EIO (%d), got %s (%d)", EIO,
    strerror(errno), errno);

  /* Restore our testing server/port. */
  redis_set_server(redis_server, redis_port, 0UL, NULL, NULL);
}
END_TEST

START_TEST (redis_conn_get_test) {
  int res;
  pr_redis_t *redis, *redis2;

  mark_point();
  redis = pr_redis_conn_get(NULL, 0UL);
  ck_assert_msg(redis == NULL, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_get(p, 0UL);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));

  mark_point();
  redis = pr_redis_conn_get(p, 0UL);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  redis2 = pr_redis_conn_get(p, 0UL);
  ck_assert_msg(redis2 != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));
  ck_assert_msg(redis == redis2, "Expected %p, got %p", redis, redis2);

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == FALSE, "Expected FALSE, got TRUE");

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_conn_set_namespace_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *prefix;
  size_t prefixsz;

  mark_point();
  res = pr_redis_conn_set_namespace(NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_conn_set_namespace(redis, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_conn_set_namespace(redis, &m, NULL, 0);
  ck_assert_msg(res == 0, "Failed to set null namespace prefix: %s",
    strerror(errno));

  prefix = "test.";
  prefixsz = strlen(prefix);

  mark_point();
  res = pr_redis_conn_set_namespace(redis, &m, prefix, 0);
  ck_assert_msg(res < 0, "Failed to handle empty namespace prefix");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_conn_set_namespace(redis, &m, prefix, prefixsz);
  ck_assert_msg(res == 0, "Failed to set namespace prefix '%s': %s", prefix,
    strerror(errno));

  mark_point();
  res = pr_redis_conn_set_namespace(redis, &m, NULL, 0);
  ck_assert_msg(res == 0, "Failed to set null namespace prefix: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_conn_get_version_test) {
  int res;
  pr_redis_t *redis;
  unsigned int major = 0, minor = 0, patch = 0;

  mark_point();
  res = pr_redis_conn_get_version(NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_conn_get_version(redis, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null version arguments");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_conn_get_version(redis, &major, &minor, &patch);
  ck_assert_msg(res == 0, "Failed to get Redis version: %s", strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_conn_auth_test) {
  int res;
  pr_redis_t *redis;
  const char *text;
  array_header *args;
  unsigned int major_version = 0;

  mark_point();
  res = pr_redis_auth(NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_auth(redis, NULL);
  ck_assert_msg(res < 0, "Failed to handle null password");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  /* What happens if we try to AUTH to a non-password-protected Redis?
   * Answer: Redis returns an error indicating that no password is required.
   *
   * Note that this behavior changed with Redis 6.x.  In particular, any
   * "AUTH default ..." command automatically succeeds with Redis 6.x,
   * regardless of the actual password given.  Sigh.
   */

  mark_point();
  res = pr_redis_conn_get_version(redis, &major_version, NULL, NULL);
  ck_assert_msg(res == 0, "Failed to get Redis version: %s", strerror(errno));

  mark_point();
  text = "password";
  res = pr_redis_auth(redis, text);

  if (major_version < 6) {
    ck_assert_msg(res < 0, "Failed to handle lack of need for authentication");
    ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
      strerror(errno), errno);

    /* Use CONFIG SET to require a password. */
    args = make_array(p, 0, sizeof(char *));
    *((char **) push_array(args)) = pstrdup(p, "CONFIG");
    *((char **) push_array(args)) = pstrdup(p, "SET");
    *((char **) push_array(args)) = pstrdup(p, "requirepass");
    *((char **) push_array(args)) = pstrdup(p, text);

    mark_point();
    res = pr_redis_command(redis, args, PR_REDIS_REPLY_TYPE_STATUS);
    ck_assert_msg(res == 0, "Failed to enable authentication: %s",
      strerror(errno));

    args = make_array(p, 0, sizeof(char *));
    *((char **) push_array(args)) = pstrdup(p, "TIME");

    mark_point();
    res = pr_redis_command(redis, args, PR_REDIS_REPLY_TYPE_ARRAY);
    ck_assert_msg(res < 0, "Failed to handle required authentication");
    ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
      strerror(errno), errno);

    mark_point();
    res = pr_redis_auth(redis, text);
    ck_assert_msg(res == 0, "Failed to authenticate client: %s", strerror(errno));

    /* Don't forget to remove the password. */
    args = make_array(p, 0, sizeof(char *));
    *((char **) push_array(args)) = pstrdup(p, "CONFIG");
    *((char **) push_array(args)) = pstrdup(p, "SET");
    *((char **) push_array(args)) = pstrdup(p, "requirepass");
    *((char **) push_array(args)) = pstrdup(p, "");

    mark_point();
    res = pr_redis_command(redis, args, PR_REDIS_REPLY_TYPE_STATUS);
    ck_assert_msg(res == 0, "Failed to remove password authentication: %s",
      strerror(errno));

  } else {
    ck_assert_msg(res == 0, "Failed to handle AUTH command: %s",
      strerror(errno));
  }

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_conn_auth2_test) {
  int res;
  pr_redis_t *redis;
  const char *username, *password;
  array_header *args;
  unsigned int major_version = 0;

  mark_point();
  res = pr_redis_auth2(NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_auth2(redis, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null username");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  /* Note: Do NOT use "default" as the initial username; that name has
   * specific semantics for Redis 6.x and later.
   */
  username = "foobar";

  mark_point();
  res = pr_redis_auth2(redis, username, NULL);
  ck_assert_msg(res < 0, "Failed to handle null password");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  /* What happens if we try to AUTH to a non-password-protected Redis?
   * Answer: Redis returns an error indicating that no password is required.
   *
   * Note that this behavior changed with Redis 6.x.  In particular, any
   * "AUTH default ..." command automatically succeeds with Redis 6.x,
   * regardless of the actual password given.  Sigh.
   */

  mark_point();
  res = pr_redis_conn_get_version(redis, &major_version, NULL, NULL);
  ck_assert_msg(res == 0, "Failed to get Redis version: %s", strerror(errno));

  mark_point();
  password = "password";
  res = pr_redis_auth2(redis, username, password);
  ck_assert_msg(res < 0, "Failed to handle lack of need for authentication");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  if (major_version < 6) {
    /* Use CONFIG SET to require a password. */
    args = make_array(p, 0, sizeof(char *));
    *((char **) push_array(args)) = pstrdup(p, "CONFIG");
    *((char **) push_array(args)) = pstrdup(p, "SET");
    *((char **) push_array(args)) = pstrdup(p, "requirepass");
    *((char **) push_array(args)) = pstrdup(p, password);

    mark_point();
    res = pr_redis_command(redis, args, PR_REDIS_REPLY_TYPE_STATUS);
    ck_assert_msg(res == 0, "Failed to enable authentication: %s",
      strerror(errno));

    args = make_array(p, 0, sizeof(char *));
    *((char **) push_array(args)) = pstrdup(p, "TIME");

    mark_point();
    res = pr_redis_command(redis, args, PR_REDIS_REPLY_TYPE_ARRAY);
    ck_assert_msg(res < 0, "Failed to handle required authentication");
    ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
      strerror(errno), errno);

    mark_point();
    res = pr_redis_auth2(redis, username, password);
    ck_assert_msg(res == 0, "Failed to authenticate client: %s", strerror(errno));

    /* Don't forget to remove the password. */
    args = make_array(p, 0, sizeof(char *));
    *((char **) push_array(args)) = pstrdup(p, "CONFIG");
    *((char **) push_array(args)) = pstrdup(p, "SET");
    *((char **) push_array(args)) = pstrdup(p, "requirepass");
    *((char **) push_array(args)) = pstrdup(p, "");

    mark_point();
    res = pr_redis_command(redis, args, PR_REDIS_REPLY_TYPE_STATUS);
    ck_assert_msg(res == 0, "Failed to remove password authentication: %s",
      strerror(errno));
  }

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_conn_select_test) {
  int res;
  pr_redis_t *redis;
  const char *text;

  mark_point();
  res = pr_redis_select(NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_select(redis, NULL);
  ck_assert_msg(res < 0, "Failed to handle null db_idx");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  text = "-1";
  res = pr_redis_select(redis, text);
  ck_assert_msg(res < 0, "Failed to handle invalid index %s", text);
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  text = "100";
  res = pr_redis_select(redis, text);
  ck_assert_msg(res < 0, "Failed to handle invalid index %s", text);
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  text = "someotherlabel";
  res = pr_redis_select(redis, text);
  ck_assert_msg(res < 0, "Failed to handle invalid index %s", text);
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  text = "0";
  res = pr_redis_select(redis, text);
  ck_assert_msg(res == 0, "Failed to select database %s: %s", text,
    strerror(errno));

  mark_point();
  text = "1";
  res = pr_redis_select(redis, text);
  ck_assert_msg(res == 0, "Failed to select database %s: %s", text,
    strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_conn_reconnect_test) {
  int res;
  pr_redis_t *redis;
  array_header *args;

  /* Note: This test is intended to be run manually, locally. */

  if (getenv("REDIS_RECONNECT") == NULL) {
    return;
  }

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  /* Now we PAUSE, and elsewhere, stop/start the Redis server, breaking the
   * connection.
   */
  pr_trace_msg("redis", 1, "PAUSING test while admin restarts Redis server");
  pr_timer_sleep(15);
  pr_trace_msg("redis", 1, "RESUMING test");

  args = make_array(p, 0, sizeof(char *));
  *((char **) push_array(args)) = pstrdup(p, "INFO");

  /* This first one should fail, due to the reconnect. */
  mark_point();
  res = pr_redis_command(redis, args, PR_REDIS_REPLY_TYPE_STRING);
  ck_assert_msg(res < 0, "Failed to handle reconnect");
  ck_assert_msg(errno == EIO, "Expected EIO (%d), got %s (%d)", EIO,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_command(redis, args, PR_REDIS_REPLY_TYPE_STRING);
  ck_assert_msg(res == 0, "Failed to handle valid command with array: %s",
    strerror(errno));


  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_command_test) {
  int res;
  pr_redis_t *redis;
  array_header *args;

  mark_point();
  res = pr_redis_command(NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_command(redis, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null args");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  args = make_array(p, 0, sizeof(char *));

  mark_point();
  res = pr_redis_command(redis, args, 0);
  ck_assert_msg(res < 0, "Failed to handle empty args");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  *((char **) push_array(args)) = pstrdup(p, "FOO");

  mark_point();
  res = pr_redis_command(redis, args, -1);
  ck_assert_msg(res < 0, "Failed to handle invalid reply type");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_command(redis, args, PR_REDIS_REPLY_TYPE_ERROR);
  ck_assert_msg(res == 0, "Failed to handle invalid command with error: %s",
    strerror(errno));

  args = make_array(p, 0, sizeof(char *));
  *((char **) push_array(args)) = pstrdup(p, "COMMAND");
  *((char **) push_array(args)) = pstrdup(p, "COUNT");

  mark_point();
  res = pr_redis_command(redis, args, PR_REDIS_REPLY_TYPE_INTEGER);
  ck_assert_msg(res == 0, "Failed to handle valid command with integer: %s",
    strerror(errno));

  args = make_array(p, 0, sizeof(char *));
  *((char **) push_array(args)) = pstrdup(p, "INFO");

  mark_point();
  res = pr_redis_command(redis, args, PR_REDIS_REPLY_TYPE_STRING);
  ck_assert_msg(res == 0, "Failed to handle valid command with array: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_sentinel_get_master_addr_test) {
  int res;
  pr_redis_t *redis;
  pr_netaddr_t *addr = NULL;
  const char *name;

  mark_point();
  res = pr_redis_sentinel_get_master_addr(NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_sentinel_get_master_addr(p, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_sentinel_get_master_addr(p, redis, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null name");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  name = "foobar";
  res = pr_redis_sentinel_get_master_addr(p, redis, name, NULL);
  ck_assert_msg(res < 0, "Failed to handle null addr");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  name = "foobar";
  res = pr_redis_sentinel_get_master_addr(p, redis, name, &addr);
  ck_assert_msg(res < 0, "Failed to handle invalid sentinel");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_sentinel_get_masters_test) {
  int res;
  pr_redis_t *redis;
  array_header *masters = NULL;

  mark_point();
  res = pr_redis_sentinel_get_masters(NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_sentinel_get_masters(p, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_sentinel_get_masters(p, redis, NULL);
  ck_assert_msg(res < 0, "Failed to handle null masters");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_sentinel_get_masters(p, redis, &masters);
  ck_assert_msg(res < 0, "Failed to handle invalid sentinel");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_sentinel_conn_new_test) {
  int res;
  pr_redis_t *redis;
  array_header *sentinels = NULL;
  const char *master = NULL;
  const pr_netaddr_t *addr;

  /* Deliberately set the wrong server and port; we want to discover the
   * correct host/port via the Sentinels.
   */
  redis_set_server(NULL, -2, 0UL, NULL, NULL);

  sentinels = make_array(p, 0, sizeof(pr_netaddr_t *));

  if (getenv("CI") != NULL ||
      getenv("CIRRUS_CLONE_DEPTH") != NULL ||
      getenv("TRAVIS") != NULL) {
    /* Treat the local Redis server as a Sentinel. */
    addr = pr_netaddr_get_addr(p, "127.0.0.1", NULL);
    pr_netaddr_set_port2((pr_netaddr_t *) addr, 6379);
    *((pr_netaddr_t **) push_array(sentinels)) = (pr_netaddr_t *) addr;

    mark_point();
    res = redis_set_sentinels(sentinels, NULL);
    ck_assert_msg(res == 0, "Failed to set sentinel list: %s", strerror(errno));

    mark_point();
    redis = pr_redis_conn_new(p, NULL, 0);
    ck_assert_msg(redis == NULL, "Failed to handle invald sentinels");
    ck_assert_msg(errno == EPERM, "Expected EPERM (%d), got %s (%d)", EPERM,
      strerror(errno), errno);

    /* Restore our testing server/port. */
    redis_set_server(redis_server, redis_port, 0UL, NULL, NULL);
    redis_set_sentinels(NULL, NULL);

    return;
  }

  mark_point();
  res = redis_set_sentinels(sentinels, NULL);
  ck_assert_msg(res < 0, "Failed to handle empty sentinel list");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  /* Set a list of bad sentinels */
  addr = pr_netaddr_get_addr(p, "127.1.2.3", NULL);
  pr_netaddr_set_port2((pr_netaddr_t *) addr, 26379);
  *((pr_netaddr_t **) push_array(sentinels)) = (pr_netaddr_t *) addr;

  addr = pr_netaddr_get_addr(p, "127.0.0.1", NULL);
  pr_netaddr_set_port2((pr_netaddr_t *) addr, 16379);
  *((pr_netaddr_t **) push_array(sentinels)) = (pr_netaddr_t *) addr;

  mark_point();
  res = redis_set_sentinels(sentinels, NULL);
  ck_assert_msg(res == 0, "Failed to set sentinels: %s", strerror(errno));

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis == NULL, "Failed to handle invalid sentinels");
  ck_assert_msg(errno == EPERM, "Expected EPERM (%d), got %s (%d)", EPERM,
    strerror(errno), errno);

  /* Set a list of one bad, one good sentinel -- use "bad" master" */
  sentinels = make_array(p, 0, sizeof(pr_netaddr_t *));
  master = "foobar";

  addr = pr_netaddr_get_addr(p, "127.0.0.1", NULL);
  pr_netaddr_set_port2((pr_netaddr_t *) addr, 16379);
  *((pr_netaddr_t **) push_array(sentinels)) = (pr_netaddr_t *) addr;

  addr = pr_netaddr_get_addr(p, "127.0.0.1", NULL);
  pr_netaddr_set_port2((pr_netaddr_t *) addr, 26379);
  *((pr_netaddr_t **) push_array(sentinels)) = (pr_netaddr_t *) addr;

  mark_point();
  res = redis_set_sentinels(sentinels, master);
  ck_assert_msg(res == 0, "Failed to set sentinels: %s", strerror(errno));

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis == NULL, "Failed to handle invalid master");
  ck_assert_msg(errno == EPERM, "Expected EPERM (%d), got %s (%d)", EPERM,
    strerror(errno), errno);

  /* Set a list of one bad, one good sentinel -- use "good" master */
  sentinels = make_array(p, 0, sizeof(pr_netaddr_t *));
  master = "proftpd";

  addr = pr_netaddr_get_addr(p, "127.0.0.1", NULL);
  pr_netaddr_set_port2((pr_netaddr_t *) addr, 16379);
  *((pr_netaddr_t **) push_array(sentinels)) = (pr_netaddr_t *) addr;

  addr = pr_netaddr_get_addr(p, "127.0.0.1", NULL);
  pr_netaddr_set_port2((pr_netaddr_t *) addr, 26379);
  *((pr_netaddr_t **) push_array(sentinels)) = (pr_netaddr_t *) addr;

  mark_point();
  res = redis_set_sentinels(sentinels, master);
  ck_assert_msg(res == 0, "Failed to set sentinels: %s", strerror(errno));

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to discover valid master: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));

  /* Set a list of one bad, one good sentinel -- use no master */
  sentinels = make_array(p, 0, sizeof(pr_netaddr_t *));
  master = NULL;

  addr = pr_netaddr_get_addr(p, "127.0.0.1", NULL);
  pr_netaddr_set_port2((pr_netaddr_t *) addr, 16379);
  *((pr_netaddr_t **) push_array(sentinels)) = (pr_netaddr_t *) addr;

  addr = pr_netaddr_get_addr(p, "127.0.0.1", NULL);
  pr_netaddr_set_port2((pr_netaddr_t *) addr, 26379);
  *((pr_netaddr_t **) push_array(sentinels)) = (pr_netaddr_t *) addr;

  mark_point();
  res = redis_set_sentinels(sentinels, master);
  ck_assert_msg(res == 0, "Failed to set sentinels: %s", strerror(errno));

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to discover valid master: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));

  /* Restore our testing server/port. */
  redis_set_server(redis_server, redis_port, 0UL, NULL, NULL);
  redis_set_sentinels(NULL, NULL);
}
END_TEST

START_TEST (redis_remove_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;

  mark_point();
  res = pr_redis_remove(NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_remove(redis, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_remove(redis, &m, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testkey";

  mark_point();
  res = pr_redis_remove(redis, &m, key);
  ck_assert_msg(res < 0, "Unexpectedly removed key '%s'", key);
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_add_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *val;
  size_t valsz;
  time_t expires;

  mark_point();
  res = pr_redis_add(NULL, NULL, NULL, NULL, 0, 0);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_add(redis, NULL, NULL, NULL, 0, 0);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_add(redis, &m, NULL, NULL, 0, 0);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testkey";

  mark_point();
  res = pr_redis_add(redis, &m, key, NULL, 0, 0);
  ck_assert_msg(res < 0, "Failed to handle null value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  val = "testval";
  valsz = strlen(val);
  expires = 0;

  mark_point();
  res = pr_redis_add(redis, &m, key, val, valsz, expires);
  ck_assert_msg(res == 0, "Failed to add key '%s', val '%s': %s", key, val,
    strerror(errno));

  mark_point();
  res = pr_redis_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove key '%s': %s", key, strerror(errno));

  expires = 3;

  mark_point();
  res = pr_redis_add(redis, &m, key, val, valsz, expires);
  ck_assert_msg(res == 0, "Failed to add key '%s', val '%s': %s", key, val,
    strerror(errno));

  mark_point();
  res = pr_redis_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove key '%s': %s", key, strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_add_with_namespace_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *prefix, *key;
  char *val;
  size_t prefixsz, valsz;
  time_t expires;

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  prefix = "test.";
  prefixsz = strlen(prefix);

  mark_point();
  res = pr_redis_conn_set_namespace(redis, &m, prefix, prefixsz);
  ck_assert_msg(res == 0, "Failed to set namespace prefix '%s': %s", prefix,
    strerror(errno));

  key = "key";
  val = "val";
  valsz = strlen(val);
  expires = 0;

  mark_point();
  res = pr_redis_add(redis, &m, key, val, valsz, expires);
  ck_assert_msg(res == 0, "Failed to add key '%s', val '%s': %s", key, val,
    strerror(errno));

  mark_point();
  res = pr_redis_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove key '%s': %s", key, strerror(errno));

  mark_point();
  res = pr_redis_conn_set_namespace(redis, &m, NULL, 0);
  ck_assert_msg(res == 0, "Failed to set null namespace prefix: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_get_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *val;
  size_t valsz;
  time_t expires;
  void *data;

  mark_point();
  data = pr_redis_get(NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(data == NULL, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  data = pr_redis_get(p, NULL, NULL, NULL, NULL);
  ck_assert_msg(data == NULL, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  data = pr_redis_get(p, redis, NULL, NULL, NULL);
  ck_assert_msg(data == NULL, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  data = pr_redis_get(p, redis, &m, NULL, NULL);
  ck_assert_msg(data == NULL, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  data = pr_redis_get(p, redis, &m, key, NULL);
  ck_assert_msg(data == NULL, "Failed to handle null valuesz");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  data = pr_redis_get(p, redis, &m, key, &valsz);
  ck_assert_msg(data == NULL, "Failed to handle nonexistent key");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  val = "Hello, World!";
  valsz = strlen(val);
  expires = 0;

  mark_point();
  res = pr_redis_set(redis, &m, key, val, valsz, expires);
  ck_assert_msg(res == 0, "Failed to set key '%s', val '%s': %s", key, val,
    strerror(errno));

  valsz = 0;

  mark_point();
  data = pr_redis_get(p, redis, &m, key, &valsz);
  ck_assert_msg(data != NULL, "Failed to get data for key '%s': %s", key,
    strerror(errno));
  ck_assert_msg(valsz == strlen(val), "Expected %lu, got %lu",
    (unsigned long) strlen(val), (unsigned long) valsz);

  mark_point();
  res = pr_redis_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove key '%s': %s", key, strerror(errno));

  mark_point();
  data = pr_redis_get(p, redis, &m, key, &valsz);
  ck_assert_msg(data == NULL, "Failed to handle nonexistent key");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_get_with_namespace_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *prefix, *key;
  char *val;
  size_t prefixsz, valsz;
  time_t expires;
  void *data;

  /* set a value, set the namespace, get it. */

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  prefix = "prefix.";
  prefixsz = strlen(prefix);

  key = "prefix.testkey";
  (void) pr_redis_remove(redis, &m, key);

  val = "Hello, World!";
  valsz = strlen(val);
  expires = 0;

  mark_point();
  res = pr_redis_set(redis, &m, key, val, valsz, expires);
  ck_assert_msg(res == 0, "Failed to set key '%s', val '%s': %s", key, val,
    strerror(errno));

  mark_point();
  res = pr_redis_conn_set_namespace(redis, &m, prefix, prefixsz);
  ck_assert_msg(res == 0, "Failed to set namespace prefix '%s': %s", prefix,
    strerror(errno));

  key = "testkey";
  valsz = 0;

  mark_point();
  data = pr_redis_get(p, redis, &m, key, &valsz);
  ck_assert_msg(data != NULL, "Failed to get data for key '%s': %s", key,
    strerror(errno));
  ck_assert_msg(valsz == strlen(val), "Expected %lu, got %lu",
    (unsigned long) strlen(val), (unsigned long) valsz);
  ck_assert_msg(memcmp(data, val, valsz) == 0, "Expected '%s', got '%.*s'",
    val, (int) valsz, data);

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_get_str_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  size_t valsz;
  time_t expires;
  char *val, *str;

  mark_point();
  str = pr_redis_get_str(NULL, NULL, NULL, NULL);
  ck_assert_msg(str == NULL, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  str = pr_redis_get_str(p, NULL, NULL, NULL);
  ck_assert_msg(str == NULL, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  str = pr_redis_get_str(p, redis, NULL, NULL);
  ck_assert_msg(str == NULL, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  str = pr_redis_get_str(p, redis, &m, NULL);
  ck_assert_msg(str == NULL, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "test_string";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  str = pr_redis_get_str(p, redis, &m, key);
  ck_assert_msg(str == NULL, "Failed to handle nonexistent key");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  val = "Hello, World!";
  valsz = strlen(val);
  expires = 0;

  mark_point();
  res = pr_redis_set(redis, &m, key, val, valsz, expires);
  ck_assert_msg(res == 0, "Failed to set key '%s', val '%s': %s", key, val,
    strerror(errno));

  mark_point();
  str = pr_redis_get_str(p, redis, &m, key);
  ck_assert_msg(str != NULL, "Failed to get string for key '%s': %s", key,
    strerror(errno));
  ck_assert_msg(strlen(str) == strlen(val), "Expected %lu, got %lu",
    (unsigned long) strlen(val), (unsigned long) strlen(str));

  mark_point();
  res = pr_redis_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove key '%s': %s", key, strerror(errno));

  mark_point();
  str = pr_redis_get_str(p, redis, &m, key);
  ck_assert_msg(str == NULL, "Failed to handle nonexistent key");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_incr_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *value;
  uint32_t incr;
  uint64_t val = 0;
  size_t valsz;
  time_t expires;

  mark_point();
  res = pr_redis_incr(NULL, NULL, NULL, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_incr(redis, NULL, NULL, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_incr(redis, &m, NULL, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testval";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_incr(redis, &m, key, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle zero incr");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  incr = 2;

  mark_point();
  res = pr_redis_incr(redis, &m, key, incr, NULL);
  ck_assert_msg(res < 0, "Failed to handle nonexistent key");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  /* Note: Yes, Redis wants a string, NOT the actual bytes.  Makes sense,
   * I guess, given its text-based protocol.
   */
  value = "31";
  valsz = strlen(value);
  expires = 0;

  mark_point();
  res = pr_redis_set(redis, &m, key, value, valsz, expires);
  ck_assert_msg(res == 0, "Failed to set key '%s', val '%s': %s", key, val,
    strerror(errno));

  mark_point();
  res = pr_redis_incr(redis, &m, key, incr, NULL);
  ck_assert_msg(res == 0, "Failed to increment key '%s' by %lu: %s", key,
    (unsigned long) incr, strerror(errno));

  val = 0;

  mark_point();
  res = pr_redis_incr(redis, &m, key, incr, &val);
  ck_assert_msg(res == 0, "Failed to increment key '%s' by %lu: %s", key,
    (unsigned long) incr, strerror(errno));
  ck_assert_msg(val == 35, "Expected %lu, got %lu", 35, (unsigned long) val);

  mark_point();
  res = pr_redis_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove key '%s': %s", key, strerror(errno));

  /* Now, let's try incrementing a non-numeric value. */
  value = "Hello, World!";
  valsz = strlen(value);
  expires = 0;

  mark_point();
  res = pr_redis_set(redis, &m, key, value, valsz, expires);
  ck_assert_msg(res == 0, "Failed to set key '%s', val '%s': %s", key, val,
    strerror(errno));

  mark_point();
  res = pr_redis_incr(redis, &m, key, incr, &val);
  ck_assert_msg(res < 0, "Failed to handle non-numeric key value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_decr_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *value;
  uint32_t decr;
  uint64_t val = 0;
  size_t valsz;
  time_t expires;

  mark_point();
  res = pr_redis_decr(NULL, NULL, NULL, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_decr(redis, NULL, NULL, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_decr(redis, &m, NULL, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testval";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_decr(redis, &m, key, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle zero decr");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  decr = 5;

  mark_point();
  res = pr_redis_decr(redis, &m, key, decr, NULL);
  ck_assert_msg(res < 0, "Failed to handle nonexistent key");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  /* Note: Yes, Redis wants a string, NOT the actual bytes.  Makes sense,
   * I guess, given its text-based protocol.
   */
  value = "31";
  valsz = strlen(value);
  expires = 0;

  mark_point();
  res = pr_redis_set(redis, &m, key, value, valsz, expires);
  ck_assert_msg(res == 0, "Failed to set key '%s', val '%s': %s", key, val,
    strerror(errno));

  mark_point();
  res = pr_redis_decr(redis, &m, key, decr, NULL);
  ck_assert_msg(res == 0, "Failed to decrement key '%s' by %lu: %s", key,
    (unsigned long) decr, strerror(errno));

  val = 0;

  mark_point();
  res = pr_redis_decr(redis, &m, key, decr, &val);
  ck_assert_msg(res == 0, "Failed to decrement key '%s' by %lu: %s", key,
    (unsigned long) decr, strerror(errno));
  ck_assert_msg(val == 21, "Expected %lu, got %lu", 21, (unsigned long) val);

  mark_point();
  res = pr_redis_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove key '%s': %s", key, strerror(errno));

  /* Now, let's try decrementing a non-numeric value. */
  value = "Hello, World!";
  valsz = strlen(value);
  expires = 0;

  mark_point();
  res = pr_redis_set(redis, &m, key, value, valsz, expires);
  ck_assert_msg(res == 0, "Failed to set key '%s', val '%s': %s", key, val,
    strerror(errno));

  mark_point();
  res = pr_redis_decr(redis, &m, key, decr, &val);
  ck_assert_msg(res < 0, "Failed to handle non-numeric key value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_rename_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *from, *to;
  char *val;
  size_t valsz;
  time_t expires;

  mark_point();
  res = pr_redis_rename(NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_rename(redis, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_rename(redis, &m, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null from");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  from = "fromkey";

  mark_point();
  res = pr_redis_rename(redis, &m, from, NULL);
  ck_assert_msg(res < 0, "Failed to handle null to");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  to = "tokey";

  mark_point();
  res = pr_redis_rename(redis, &m, from, to);
  ck_assert_msg(res < 0, "Failed to handle nonexistent from key");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  val = "testval";
  valsz = strlen(val);
  expires = 0;

  mark_point();
  res = pr_redis_set(redis, &m, from, val, valsz, expires);
  ck_assert_msg(res == 0, "Failed to set key '%s', val '%s': %s", from, val,
    strerror(errno));

  mark_point();
  res = pr_redis_rename(redis, &m, from, to);
  ck_assert_msg(res == 0, "Failed to rename '%s' to '%s': %s", from, to,
    strerror(errno));

  mark_point();
  res = pr_redis_remove(redis, &m, to);
  ck_assert_msg(res == 0, "Failed to remove key '%s': %s", to, strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_set_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *val;
  size_t valsz;
  time_t expires;

  mark_point();
  res = pr_redis_set(NULL, NULL, NULL, NULL, 0, 0);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_set(redis, NULL, NULL, NULL, 0, 0);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_set(redis, &m, NULL, NULL, 0, 0);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testkey";

  mark_point();
  res = pr_redis_set(redis, &m, key, NULL, 0, 0);
  ck_assert_msg(res < 0, "Failed to handle null value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  val = "testval";
  valsz = strlen(val);
  expires = 0;

  mark_point();
  res = pr_redis_set(redis, &m, key, val, valsz, expires);
  ck_assert_msg(res == 0, "Failed to set key '%s', val '%s': %s", key, val,
    strerror(errno));

  mark_point();
  res = pr_redis_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove key '%s': %s", key, strerror(errno));

  expires = 3;

  mark_point();
  res = pr_redis_set(redis, &m, key, val, valsz, expires);
  ck_assert_msg(res == 0, "Failed to set key '%s', val '%s': %s", key, val,
    strerror(errno));

  mark_point();
  res = pr_redis_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove key '%s': %s", key, strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_hash_remove_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;

  mark_point();
  res = pr_redis_hash_remove(NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_hash_remove(redis, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_hash_remove(redis, &m, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testkey";

  mark_point();
  res = pr_redis_hash_remove(redis, &m, key);
  ck_assert_msg(res < 0, "Unexpectedly removed key '%s'", key);
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_hash_get_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key, *field;
  char *val;
  size_t valsz;

  mark_point();
  res = pr_redis_hash_get(NULL, NULL, NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_hash_get(p, NULL, NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_hash_get(p, redis, NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_hash_get(p, redis, &m, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testhashkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_hash_get(p, redis, &m, key, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null field");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  field = "hashfield";

  mark_point();
  res = pr_redis_hash_get(p, redis, &m, key, field, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_hash_get(p, redis, &m, key, field, (void **) &val, &valsz);
  ck_assert_msg(res < 0, "Failed to handle nonexistent item");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_hash_set_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key, *field;
  char *val;
  size_t valsz;

  mark_point();
  res = pr_redis_hash_set(NULL, NULL, NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_hash_set(redis, NULL, NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_hash_set(redis, &m, NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testhashkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_hash_set(redis, &m, key, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null field");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  field = "hashfield";

  mark_point();
  res = pr_redis_hash_set(redis, &m, key, field, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  val = "hashval";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_hash_set(redis, &m, key, field, val, 0);
  ck_assert_msg(res < 0, "Failed to handle zero valuesz");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_hash_set(redis, &m, key, field, val, valsz);
  ck_assert_msg(res == 0, "Failed to set item: %s", strerror(errno));

  val = NULL;
  valsz = 0;

  mark_point();
  res = pr_redis_hash_get(p, redis, &m, key, field, (void **) &val, &valsz);
  ck_assert_msg(res == 0, "Failed to get item: %s", strerror(errno));
  ck_assert_msg(valsz == 7, "Expected item length 7, got %lu",
    (unsigned long) valsz);
  ck_assert_msg(val != NULL, "Failed to get value from hash");
  ck_assert_msg(memcmp(val, "hashval", valsz) == 0,
    "Expected 'hashval', got '%.*s'", (int) valsz, val);

  mark_point();
  res = pr_redis_hash_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove hash: %s", strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_hash_delete_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key, *field;
  char *val;
  size_t valsz;

  mark_point();
  res = pr_redis_hash_delete(NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_hash_delete(redis, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_hash_delete(redis, &m, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testhashkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_hash_delete(redis, &m, key, NULL);
  ck_assert_msg(res < 0, "Failed to handle null field");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  field = "hashfield";

  mark_point();
  res = pr_redis_hash_delete(redis, &m, key, field);
  ck_assert_msg(res < 0, "Failed to handle nonexistent field");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  val = "hashval";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_hash_set(redis, &m, key, field, val, valsz);
  ck_assert_msg(res == 0, "Failed to set item: %s", strerror(errno));

  mark_point();
  res = pr_redis_hash_delete(redis, &m, key, field);
  ck_assert_msg(res == 0, "Failed to delete field: %s", strerror(errno));

  /* Note that we add this item back, just so that the hash is NOT empty when
   * we go to remove it entirely.
   */

  mark_point();
  res = pr_redis_hash_set(redis, &m, key, field, val, valsz);
  ck_assert_msg(res == 0, "Failed to set item: %s", strerror(errno));

  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_hash_count_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key, *field;
  uint64_t count = 0;
  char *val;
  size_t valsz;

  mark_point();
  res = pr_redis_hash_count(NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_hash_count(redis, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_hash_count(redis, &m, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testhashkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_hash_count(redis, &m, key, NULL);
  ck_assert_msg(res < 0, "Failed to handle null count");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_hash_count(redis, &m, key, &count);
  ck_assert_msg(res == 0, "Failed to get count using key '%s': %s", key,
    strerror(errno));

  field = "hashfield";
  val = "hashval";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_hash_set(redis, &m, key, field, val, valsz);
  ck_assert_msg(res == 0, "Failed to set item: %s", strerror(errno));

  mark_point();
  res = pr_redis_hash_count(redis, &m, key, &count);
  ck_assert_msg(res == 0, "Failed to get count: %s", strerror(errno));
  ck_assert_msg(count == 1, "Expected 1, got %lu", (unsigned long) count);

  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_hash_exists_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key, *field;
  char *val;
  size_t valsz;

  mark_point();
  res = pr_redis_hash_exists(NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_hash_exists(redis, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_hash_exists(redis, &m, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testhashkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_hash_exists(redis, &m, key, NULL);
  ck_assert_msg(res < 0, "Failed to handle null field");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  field = "hashfield";

  mark_point();
  res = pr_redis_hash_exists(redis, &m, key, field);
  ck_assert_msg(res == FALSE, "Failed to handle nonexistent field");

  val = "hashval";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_hash_set(redis, &m, key, field, val, valsz);
  ck_assert_msg(res == 0, "Failed to set item: %s", strerror(errno));

  mark_point();
  res = pr_redis_hash_exists(redis, &m, key, field);
  ck_assert_msg(res == TRUE, "Failed to handle existing field");

  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_hash_incr_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key, *field;
  int64_t num;
  char *val;
  size_t valsz;

  mark_point();
  res = pr_redis_hash_incr(NULL, NULL, NULL, NULL, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_hash_incr(redis, NULL, NULL, NULL, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_hash_incr(redis, &m, NULL, NULL, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testhashkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_hash_incr(redis, &m, key, NULL, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null field");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  field = "hashfield";

  mark_point();
  res = pr_redis_hash_incr(redis, &m, key, field, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle nonexistent field");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  val = "1";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_hash_set(redis, &m, key, field, val, valsz);
  ck_assert_msg(res == 0, "Failed to set item: %s", strerror(errno));

  mark_point();
  res = pr_redis_hash_incr(redis, &m, key, field, 0, NULL);
  ck_assert_msg(res == 0, "Failed to handle existing field: %s", strerror(errno));

  mark_point();
  res = pr_redis_hash_incr(redis, &m, key, field, 1, &num);
  ck_assert_msg(res == 0, "Failed to handle existing field: %s", strerror(errno));
  ck_assert_msg(num == 2, "Expected 2, got %lu", (unsigned long) num);

  mark_point();
  res = pr_redis_hash_incr(redis, &m, key, field, -3, &num);
  ck_assert_msg(res == 0, "Failed to handle existing field: %s", strerror(errno));
  ck_assert_msg(num == -1, "Expected -1, got %lu", (unsigned long) num);

  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_hash_keys_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key, *field;
  char *val;
  size_t valsz;
  array_header *fields = NULL;

  mark_point();
  res = pr_redis_hash_keys(NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_hash_keys(p, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_hash_keys(p, redis, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_hash_keys(p, redis, &m, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testhashkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_hash_keys(p, redis, &m, key, NULL);
  ck_assert_msg(res < 0, "Failed to handle null fields");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_hash_keys(p, redis, &m, key, &fields);
  ck_assert_msg(res < 0, "Failed to handle nonexistent fields");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  /* Add some fields */

  field = "foo";
  val = "1";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_hash_set(redis, &m, key, field, val, valsz);
  ck_assert_msg(res == 0, "Failed to set item: %s", strerror(errno));

  field = "bar";
  val = "baz quxx";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_hash_set(redis, &m, key, field, val, valsz);
  ck_assert_msg(res == 0, "Failed to set item: %s", strerror(errno));

  fields = NULL;

  mark_point();
  res = pr_redis_hash_keys(p, redis, &m, key, &fields);
  ck_assert_msg(res == 0, "Failed to handle existing fields: %s", strerror(errno));
  ck_assert_msg(fields != NULL, "Failed to get hash fields");
  ck_assert_msg(fields->nelts == 2, "Expected 2, got %u", fields->nelts);

  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_hash_values_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key, *field;
  char *val;
  size_t valsz;
  array_header *values = NULL;

  mark_point();
  res = pr_redis_hash_values(NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_hash_values(p, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_hash_values(p, redis, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_hash_values(p, redis, &m, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testhashkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_hash_values(p, redis, &m, key, NULL);
  ck_assert_msg(res < 0, "Failed to handle null values");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_hash_values(p, redis, &m, key, &values);
  ck_assert_msg(res < 0, "Failed to handle nonexistent values");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  /* Add some fields */

  field = "foo";
  val = "1";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_hash_set(redis, &m, key, field, val, valsz);
  ck_assert_msg(res == 0, "Failed to set item: %s", strerror(errno));

  field = "bar";
  val = "baz quxx";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_hash_set(redis, &m, key, field, val, valsz);
  ck_assert_msg(res == 0, "Failed to set item: %s", strerror(errno));

  values = NULL;

  mark_point();
  res = pr_redis_hash_values(p, redis, &m, key, &values);
  ck_assert_msg(res == 0, "Failed to handle existing values: %s", strerror(errno));
  ck_assert_msg(values != NULL, "Failed to get hash values");
  ck_assert_msg(values->nelts == 2, "Expected 2, got %u", values->nelts);

  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_hash_getall_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key, *field;
  char *val;
  size_t valsz;
  pr_table_t *hash = NULL;

  mark_point();
  res = pr_redis_hash_getall(NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_hash_getall(p, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_hash_getall(p, redis, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_hash_getall(p, redis, &m, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testhashkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_hash_getall(p, redis, &m, key, NULL);
  ck_assert_msg(res < 0, "Failed to handle null hash");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_hash_getall(p, redis, &m, key, &hash);
  ck_assert_msg(res < 0, "Failed to handle nonexistent hash");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  /* Add some fields */

  field = "foo";
  val = "1";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_hash_set(redis, &m, key, field, val, valsz);
  ck_assert_msg(res == 0, "Failed to set item: %s", strerror(errno));

  field = "bar";
  val = "baz quxx";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_hash_set(redis, &m, key, field, val, valsz);
  ck_assert_msg(res == 0, "Failed to set item: %s", strerror(errno));

  hash = NULL;

  mark_point();
  res = pr_redis_hash_getall(p, redis, &m, key, &hash);
  ck_assert_msg(res == 0, "Failed to handle existing fields: %s", strerror(errno));
  ck_assert_msg(hash != NULL, "Failed to get hash");
  res = pr_table_count(hash);
  ck_assert_msg(res == 2, "Expected 2, got %d", res);

  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_hash_setall_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key, *field;
  char *val;
  size_t valsz;
  pr_table_t *hash = NULL;
  uint64_t count = 0;

  mark_point();
  res = pr_redis_hash_setall(NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_hash_setall(redis, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_hash_setall(redis, &m, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testhashkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_hash_setall(redis, &m, key, NULL);
  ck_assert_msg(res < 0, "Failed to handle null hash");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  hash = pr_table_alloc(p, 0);

  mark_point();
  res = pr_redis_hash_setall(redis, &m, key, hash);
  ck_assert_msg(res < 0, "Failed to handle empty hash");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  /* Add some fields */
  field = "foo";
  val = "1";
  valsz = strlen(val);
  (void) pr_table_add_dup(hash, pstrdup(p, field), val, valsz);

  field = "bar";
  val = "baz quxx";
  valsz = strlen(val);
  (void) pr_table_add_dup(hash, pstrdup(p, field), val, valsz);

  mark_point();
  res = pr_redis_hash_setall(redis, &m, key, hash);
  ck_assert_msg(res == 0, "Failed to set hash: %s", strerror(errno));

  mark_point();
  res = pr_redis_hash_count(redis, &m, key, &count);
  ck_assert_msg(res == 0, "Failed to count hash: %s", strerror(errno));
  ck_assert_msg(count == 2, "Expected 2, got %lu", (unsigned long) count);

  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_list_remove_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;

  mark_point();
  res = pr_redis_list_remove(NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_list_remove(redis, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_list_remove(redis, &m, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testlistkey";

  mark_point();
  res = pr_redis_list_remove(redis, &m, key);
  ck_assert_msg(res < 0, "Failed to handle nonexistent list");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_list_append_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *val;
  size_t valsz;

  mark_point();
  res = pr_redis_list_append(NULL, NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_list_append(redis, NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_list_append(redis, &m, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testlistkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_list_append(redis, &m, key, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  val = "Some JSON here";

  mark_point();
  res = pr_redis_list_append(redis, &m, key, val, 0);
  ck_assert_msg(res < 0, "Failed to handle empty value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  valsz = strlen(val);

  mark_point();
  (void) pr_redis_list_remove(redis, &m, key);

  mark_point();
  res = pr_redis_list_append(redis, &m, key, val, valsz);
  ck_assert_msg(res == 0, "Failed to append to list '%s': %s", key,
    strerror(errno));

  mark_point();
  res = pr_redis_list_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove list '%s': %s", key, strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_list_count_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  uint64_t count = 0;
  char *val;
  size_t valsz;

  mark_point();
  res = pr_redis_list_count(NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_list_count(redis, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_list_count(redis, &m, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testlistkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_list_count(redis, &m, key, NULL);
  ck_assert_msg(res < 0, "Failed to handle null count");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_list_count(redis, &m, key, &count);
  ck_assert_msg(res == 0, "Failed to get list count: %s", strerror(errno));
  ck_assert_msg(count == 0, "Expected 0, got %lu", (unsigned long) count);

  val = "Some JSON here";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_list_append(redis, &m, key, val, valsz);
  ck_assert_msg(res == 0, "Failed to append to list '%s': %s", key,
    strerror(errno));

  mark_point();
  res = pr_redis_list_count(redis, &m, key, &count);
  ck_assert_msg(res == 0, "Failed to get list count: %s", strerror(errno));
  ck_assert_msg(count == 1, "Expected 1, got %lu", (unsigned long) count);

  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_list_delete_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *val;
  size_t valsz;

  mark_point();
  res = pr_redis_list_delete(NULL, NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_list_delete(redis, NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_list_delete(redis, &m, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testlistkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_list_delete(redis, &m, key, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  val = "Some JSON here";

  mark_point();
  res = pr_redis_list_delete(redis, &m, key, val, 0);
  ck_assert_msg(res < 0, "Failed to handle empty value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  valsz = strlen(val);

  mark_point();
  (void) pr_redis_list_remove(redis, &m, key);

  mark_point();
  res = pr_redis_list_delete(redis, &m, key, val, valsz);
  ck_assert_msg(res < 0, "Failed to handle nonexistent items");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_list_append(redis, &m, key, val, valsz);
  ck_assert_msg(res == 0, "Failed to append to list '%s': %s", key,
    strerror(errno));

  mark_point();
  res = pr_redis_list_delete(redis, &m, key, val, valsz);
  ck_assert_msg(res == 0, "Failed to handle existing items");

  /* Note that we add this item back, just so that the list is NOT empty when
   * we go to remove it entirely.
   */

  mark_point();
  res = pr_redis_list_append(redis, &m, key, val, valsz);
  ck_assert_msg(res == 0, "Failed to append to list '%s': %s", key,
    strerror(errno));

  mark_point();
  res = pr_redis_list_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove list '%s': %s", key, strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_list_exists_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *val;
  size_t valsz;

  mark_point();
  res = pr_redis_list_exists(NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_list_exists(redis, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_list_exists(redis, &m, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_list_exists(redis, &m, key, 0);
  ck_assert_msg(res == FALSE, "Failed to handle nonexistent item");

  val = "testval";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_list_append(redis, &m, key, val, valsz);
  ck_assert_msg(res == 0, "Failed to add item to key '%s': %s", key,
    strerror(errno));

  mark_point();
  res = pr_redis_list_exists(redis, &m, key, 0);
  ck_assert_msg(res == TRUE, "Failed to handle existing item");

  mark_point();
  res = pr_redis_list_exists(redis, &m, key, 3);
  ck_assert_msg(res < 0, "Failed to handle invalid index");
  ck_assert_msg(errno == ERANGE, "Expected ERANGE (%d), got %s (%d)", ERANGE,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove key '%s': %s", key, strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_list_get_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *val;
  size_t valsz;

  mark_point();
  res = pr_redis_list_get(NULL, NULL, NULL, NULL, 0, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_list_get(p, NULL, NULL, NULL, 0, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_list_get(p, redis, NULL, NULL, 0, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_list_get(p, redis, &m, NULL, 0, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_list_get(p, redis, &m, key, 0, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_list_get(p, redis, &m, key, 0, (void **) &val, NULL);
  ck_assert_msg(res < 0, "Failed to handle null valuesz");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  val = "foo";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_list_append(redis, &m, key, val, valsz);
  ck_assert_msg(res == 0, "Failed to append item to key '%s': %s", key,
    strerror(errno));

  val = NULL;
  valsz = 0;

  mark_point();
  res = pr_redis_list_get(p, redis, &m, key, 3, (void **) &val, &valsz);
  ck_assert_msg(res < 0, "Failed to handle invalid index");
  ck_assert_msg(errno == ERANGE, "Expected ERANGE (%d), got %s (%d)", ERANGE,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_list_get(p, redis, &m, key, 0, (void **) &val, &valsz);
  ck_assert_msg(res == 0, "Failed to get item in list: %s", strerror(errno));
  ck_assert_msg(val != NULL, "Expected value, got null");
  ck_assert_msg(valsz == 3, "Expected 3, got %lu", (unsigned long) valsz);
  ck_assert_msg(memcmp(val, "foo", 3) == 0, "Expected 'foo', got '%.*s'",
    (int) valsz, val);

  mark_point();
  res = pr_redis_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove key '%s': %s", key, strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_list_getall_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *val;
  size_t valsz;
  array_header *values = NULL, *valueszs = NULL;

  mark_point();
  res = pr_redis_list_getall(NULL, NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_list_getall(p, NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_list_getall(p, redis, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_list_getall(p, redis, &m, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_list_getall(p, redis, &m, key, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null values");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_list_getall(p, redis, &m, key, &values, NULL);
  ck_assert_msg(res < 0, "Failed to handle null valueszs");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  val = "foo";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_list_append(redis, &m, key, val, valsz);
  ck_assert_msg(res == 0, "Failed to append item to key '%s': %s", key,
    strerror(errno));

  val = "bar";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_list_append(redis, &m, key, val, valsz);
  ck_assert_msg(res == 0, "Failed to append item to key '%s': %s", key,
    strerror(errno));

  val = "baz";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_list_append(redis, &m, key, val, valsz);
  ck_assert_msg(res == 0, "Failed to append item to key '%s': %s", key,
    strerror(errno));

  mark_point();
  res = pr_redis_list_getall(p, redis, &m, key, &values, &valueszs);
  ck_assert_msg(res == 0, "Failed to get items in list: %s", strerror(errno));
  ck_assert_msg(values != NULL, "Expected values, got null");
  ck_assert_msg(valueszs != NULL, "Expected valueszs, got null");
  ck_assert_msg(values->nelts == 3, "Expected 3, got %u", values->nelts);
  ck_assert_msg(valueszs->nelts == 3, "Expected 3, got %u", valueszs->nelts);

  mark_point();
  res = pr_redis_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove key '%s': %s", key, strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_list_pop_params_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *val;
  size_t valsz;

  mark_point();
  res = pr_redis_list_pop(NULL, NULL, NULL, NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_list_pop(p, NULL, NULL, NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_list_pop(p, redis, NULL, NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_list_pop(p, redis, &m, NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testkey";

  mark_point();
  res = pr_redis_list_pop(p, redis, &m, key, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_list_pop(p, redis, &m, key, (void **) &val, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null valuesz");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_list_pop(p, redis, &m, key, (void **) &val, &valsz, 0);
  ck_assert_msg(res < 0, "Failed to handle invalid flags");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_list_pop_left_test) {
  int res, flags = PR_REDIS_LIST_FL_LEFT;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *val;
  size_t valsz;

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  key = "testkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_list_pop(p, redis, &m, key, (void **) &val, &valsz, flags);
  ck_assert_msg(res < 0, "Failed to handle nonexistent list");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  val = "foo";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_list_append(redis, &m, key, val, valsz);
  ck_assert_msg(res == 0, "Failed to append item to key '%s': %s", key,
    strerror(errno));

  val = NULL;
  valsz = 0;

  mark_point();
  res = pr_redis_list_pop(p, redis, &m, key, (void **) &val, &valsz, flags);
  ck_assert_msg(res == 0, "Failed to get item in list: %s", strerror(errno));
  ck_assert_msg(val != NULL, "Expected value, got null");
  ck_assert_msg(valsz == 3, "Expected 3, got %lu", (unsigned long) valsz);
  ck_assert_msg(memcmp(val, "foo", 3) == 0, "Expected 'foo', got '%.*s'",
    (int) valsz, val);

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_list_pop_right_test) {
  int res, flags = PR_REDIS_LIST_FL_RIGHT;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *val;
  size_t valsz;

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  key = "testkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_list_pop(p, redis, &m, key, (void **) &val, &valsz, flags);
  ck_assert_msg(res < 0, "Failed to handle nonexistent list");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  val = "foo";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_list_append(redis, &m, key, val, valsz);
  ck_assert_msg(res == 0, "Failed to append item to key '%s': %s", key,
    strerror(errno));

  val = NULL;
  valsz = 0;

  mark_point();
  res = pr_redis_list_pop(p, redis, &m, key, (void **) &val, &valsz, flags);
  ck_assert_msg(res == 0, "Failed to get item in list: %s", strerror(errno));
  ck_assert_msg(val != NULL, "Expected value, got null");
  ck_assert_msg(valsz == 3, "Expected 3, got %lu", (unsigned long) valsz);
  ck_assert_msg(memcmp(val, "foo", 3) == 0, "Expected 'foo', got '%.*s'",
    (int) valsz, val);

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_list_push_params_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *val;
  size_t valsz;

  mark_point();
  res = pr_redis_list_push(NULL, NULL, NULL, NULL, 0, 0);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_list_push(redis, NULL, NULL, NULL, 0, 0);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_list_push(redis, &m, NULL, NULL, 0, 0);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testkey";

  mark_point();
  res = pr_redis_list_push(redis, &m, key, NULL, 0, 0);
  ck_assert_msg(res < 0, "Failed to handle null value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  val = "testval";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_list_push(redis, &m, key, val, 0, 0);
  ck_assert_msg(res < 0, "Failed to handle empty valuesz");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_list_push(redis, &m, key, val, valsz, 0);
  ck_assert_msg(res < 0, "Failed to handle invalid flags");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_list_push_left_test) {
  int res, flags = PR_REDIS_LIST_FL_LEFT;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *val;
  size_t valsz;

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  key = "testlistkey";
  (void) pr_redis_remove(redis, &m, key);

  val = "Some JSON here";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_list_push(redis, &m, key, val, valsz, flags);
  ck_assert_msg(res == 0, "Failed to append to list '%s': %s", key,
    strerror(errno));

  mark_point();
  res = pr_redis_list_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove list '%s': %s", key, strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_list_push_right_test) {
  int res, flags = PR_REDIS_LIST_FL_RIGHT;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *val;
  size_t valsz;

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  key = "testlistkey";
  (void) pr_redis_remove(redis, &m, key);

  val = "Some JSON here";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_list_push(redis, &m, key, val, valsz, flags);
  ck_assert_msg(res == 0, "Failed to append to list '%s': %s", key,
    strerror(errno));

  mark_point();
  res = pr_redis_list_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove list '%s': %s", key, strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_list_rotate_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *val = NULL;
  size_t valsz = 0;

  mark_point();
  res = pr_redis_list_rotate(NULL, NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_list_rotate(p, NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_list_rotate(p, redis, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_list_rotate(p, redis, &m, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testlistkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_list_rotate(p, redis, &m, key, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_list_rotate(p, redis, &m, key, (void **) &val, NULL);
  ck_assert_msg(res < 0, "Failed to handle null value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_list_rotate(p, redis, &m, key, (void **) &val, &valsz);
  ck_assert_msg(res < 0, "Failed to handle nonexistent key");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  val = "foo";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_list_append(redis, &m, key, val, valsz);
  ck_assert_msg(res == 0, "Failed to append item using key '%s': %s", key,
    strerror(errno));

  val = "bar";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_list_append(redis, &m, key, val, valsz);
  ck_assert_msg(res == 0, "Failed to append item using key '%s': %s", key,
    strerror(errno));

  val = NULL;
  valsz = 0;

  mark_point();
  res = pr_redis_list_rotate(p, redis, &m, key, (void **) &val, &valsz);
  ck_assert_msg(res == 0, "Failed to rotate list '%s': %s", key, strerror(errno));
  ck_assert_msg(val != NULL, "Expected value, got NULL");
  ck_assert_msg(valsz == 3, "Expected 3, got %lu", (unsigned long) valsz);
  ck_assert_msg(memcmp(val, "bar", valsz) == 0, "Expected 'bar', got '%.*s'",
    (int) valsz, val);

  val = NULL;
  valsz = 0;

  mark_point();
  res = pr_redis_list_rotate(p, redis, &m, key, (void **) &val, &valsz);
  ck_assert_msg(res == 0, "Failed to rotate list '%s': %s", key, strerror(errno));
  ck_assert_msg(val != NULL, "Expected value, got NULL");
  ck_assert_msg(valsz == 3, "Expected 3, got %lu", (unsigned long) valsz);
  ck_assert_msg(memcmp(val, "foo", valsz) == 0, "Expected 'foo', got '%.*s'",
    (int) valsz, val);

  val = NULL;
  valsz = 0;

  mark_point();
  res = pr_redis_list_rotate(p, redis, &m, key, (void **) &val, &valsz);
  ck_assert_msg(res == 0, "Failed to rotate list '%s': %s", key, strerror(errno));
  ck_assert_msg(val != NULL, "Expected value, got NULL");
  ck_assert_msg(valsz == 3, "Expected 3, got %lu", (unsigned long) valsz);
  ck_assert_msg(memcmp(val, "bar", valsz) == 0, "Expected 'bar', got '%.*s'",
    (int) valsz, val);

  mark_point();
  res = pr_redis_list_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove list '%s': %s", key, strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_list_set_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *val;
  size_t valsz;

  mark_point();
  res = pr_redis_list_set(NULL, NULL, NULL, 0, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_list_set(redis, NULL, NULL, 0, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_list_set(redis, &m, NULL, 0, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testlistkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_list_set(redis, &m, key, 0, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  val = "Some JSON here";

  mark_point();
  res = pr_redis_list_set(redis, &m, key, 0, val, 0);
  ck_assert_msg(res < 0, "Failed to handle empty value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  valsz = strlen(val);

  mark_point();
  (void) pr_redis_list_remove(redis, &m, key);

  mark_point();
  res = pr_redis_list_set(redis, &m, key, 3, val, valsz);
  ck_assert_msg(res < 0, "Failed to handle invalid index");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_list_set(redis, &m, key, 0, val, valsz);
  ck_assert_msg(res < 0, "Failed to handle invalid index");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  /* Append the item first, then set it. */

  mark_point();
  res = pr_redis_list_append(redis, &m, key, val, valsz);
  ck_assert_msg(res == 0, "Failed to append item using key '%s': %s", key,
    strerror(errno));

  val = "listval2";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_list_set(redis, &m, key, 0, val, valsz);
  ck_assert_msg(res == 0, "Failed to set item at index 0 using key '%s': %s",
    key, strerror(errno));

  mark_point();
  res = pr_redis_list_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove list '%s': %s", key, strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_list_setall_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  array_header *vals, *valszs;

  mark_point();
  res = pr_redis_list_setall(NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_list_setall(redis, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_list_setall(redis, &m, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testlistkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_list_setall(redis, &m, key, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null values");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  vals = make_array(p, 0, sizeof(char *));

  mark_point();
  res = pr_redis_list_setall(redis, &m, key, vals, NULL);
  ck_assert_msg(res < 0, "Failed to handle empty values");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  *((char **) push_array(vals)) = pstrdup(p, "Some JSON here");

  mark_point();
  res = pr_redis_list_setall(redis, &m, key, vals, NULL);
  ck_assert_msg(res < 0, "Failed to handle null valueszs");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  valszs = make_array(p, 0, sizeof(char *));

  mark_point();
  res = pr_redis_list_setall(redis, &m, key, vals, valszs);
  ck_assert_msg(res < 0, "Failed to handle empty valueszs");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  *((size_t *) push_array(valszs)) = strlen("Some JSON here");
  *((char **) push_array(vals)) = pstrdup(p, "bar");

  mark_point();
  res = pr_redis_list_setall(redis, &m, key, vals, valszs);
  ck_assert_msg(res < 0, "Failed to handle mismatched values/valueszs");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  *((size_t *) push_array(valszs)) = strlen("bar");

  mark_point();
  (void) pr_redis_list_remove(redis, &m, key);

  mark_point();
  res = pr_redis_list_setall(redis, &m, key, vals, valszs);
  ck_assert_msg(res == 0, "Failed to set items using key '%s': %s",
    key, strerror(errno));

  mark_point();
  res = pr_redis_list_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove list '%s': %s", key, strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_set_remove_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;

  mark_point();
  res = pr_redis_set_remove(NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_set_remove(redis, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_set_remove(redis, &m, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testkey";

  mark_point();
  res = pr_redis_set_remove(redis, &m, key);
  ck_assert_msg(res < 0, "Unexpectedly removed key '%s'", key);
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_set_exists_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *val;
  size_t valsz;

  mark_point();
  res = pr_redis_set_exists(NULL, NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_set_exists(redis, NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_set_exists(redis, &m, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_set_exists(redis, &m, key, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  val = "testval";
  valsz = 0;

  mark_point();
  res = pr_redis_set_exists(redis, &m, key, val, valsz);
  ck_assert_msg(res < 0, "Failed to handle zero valuesz");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  valsz = strlen(val);

  mark_point();
  res = pr_redis_set_exists(redis, &m, key, val, valsz);
  ck_assert_msg(res == FALSE, "Failed to handle nonexistent item");

  mark_point();
  res = pr_redis_set_add(redis, &m, key, val, valsz);
  ck_assert_msg(res == 0, "Failed to add item to key '%s': %s", key,
    strerror(errno));

  mark_point();
  res = pr_redis_set_exists(redis, &m, key, val, valsz);
  ck_assert_msg(res == TRUE, "Failed to handle existing item");

  mark_point();
  res = pr_redis_set_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove key '%s': %s", key, strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_set_add_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *val;
  size_t valsz;

  mark_point();
  res = pr_redis_set_add(NULL, NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_set_add(redis, NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_set_add(redis, &m, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testkey";
  (void) pr_redis_set_remove(redis, &m, key);

  mark_point();
  res = pr_redis_set_add(redis, &m, key, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  val = "testval";
  valsz = 0;

  mark_point();
  res = pr_redis_set_add(redis, &m, key, val, 0);
  ck_assert_msg(res < 0, "Failed to handle zero valuesz");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  valsz = strlen(val);

  mark_point();
  res = pr_redis_set_add(redis, &m, key, val, valsz);
  ck_assert_msg(res == 0, "Failed to add key '%s', val '%s': %s", key, val,
    strerror(errno));

  mark_point();
  res = pr_redis_set_add(redis, &m, key, val, valsz);
  ck_assert_msg(res < 0, "Failed to handle duplicates");
  ck_assert_msg(errno == EEXIST, "Expected EEXIST (%d), got %s (%d)", EEXIST,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove key '%s': %s", key, strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_set_count_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  uint64_t count;
  void *val;
  size_t valsz;

  mark_point();
  res = pr_redis_set_count(NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_set_count(redis, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_set_count(redis, &m, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();

  key = "testkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_set_count(redis, &m, key, NULL);
  ck_assert_msg(res < 0, "Failed to handle null count");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_set_count(redis, &m, key, &count);
  ck_assert_msg(res == 0, "Failed to handle get set count: %s", strerror(errno));
  ck_assert_msg(count == 0, "Expected 0, got %lu", (unsigned long) count);

  val = "testval";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_set_add(redis, &m, key, val, valsz);
  ck_assert_msg(res == 0, "Failed to add item to key '%s': %s", key,
    strerror(errno));

  mark_point();
  res = pr_redis_set_count(redis, &m, key, &count);
  ck_assert_msg(res == 0, "Failed to handle get set count: %s", strerror(errno));
  ck_assert_msg(count == 1, "Expected 1, got %lu", (unsigned long) count);

  mark_point();
  res = pr_redis_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove key '%s': %s", key, strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_set_delete_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *val;
  size_t valsz;

  mark_point();
  res = pr_redis_set_delete(NULL, NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_set_delete(redis, NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_set_delete(redis, &m, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_set_delete(redis, &m, key, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  val = "testval";
  valsz = 0;

  mark_point();
  res = pr_redis_set_delete(redis, &m, key, val, valsz);
  ck_assert_msg(res < 0, "Failed to handle zero valuesz");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  valsz = strlen(val);

  mark_point();
  res = pr_redis_set_delete(redis, &m, key, val, valsz);
  ck_assert_msg(res < 0, "Failed to handle nonexistent item");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_set_add(redis, &m, key, val, valsz);
  ck_assert_msg(res == 0, "Failed to add item to key '%s': %s", key,
    strerror(errno));

  mark_point();
  res = pr_redis_set_delete(redis, &m, key, val, valsz);
  ck_assert_msg(res == 0, "Failed to delete item from set: %s", strerror(errno));

  /* Note that we add this item back, just so that the set is NOT empty when
   * we go to remove it entirely.
   */

  mark_point();
  res = pr_redis_set_add(redis, &m, key, val, valsz);
  ck_assert_msg(res == 0, "Failed to add item to key '%s': %s", key,
    strerror(errno));

  mark_point();
  res = pr_redis_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove key '%s': %s", key, strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_set_getall_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *val;
  size_t valsz;
  array_header *values = NULL, *valueszs = NULL;

  mark_point();
  res = pr_redis_set_getall(NULL, NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_set_getall(p, NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_set_getall(p, redis, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_set_getall(p, redis, &m, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_set_getall(p, redis, &m, key, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null values");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_set_getall(p, redis, &m, key, &values, NULL);
  ck_assert_msg(res < 0, "Failed to handle null valueszs");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  val = "foo";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_set_add(redis, &m, key, val, valsz);
  ck_assert_msg(res == 0, "Failed to add item to key '%s': %s", key,
    strerror(errno));

  val = "bar";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_set_add(redis, &m, key, val, valsz);
  ck_assert_msg(res == 0, "Failed to add item to key '%s': %s", key,
    strerror(errno));

  val = "baz";
  valsz = strlen(val);

  mark_point();
  res = pr_redis_set_add(redis, &m, key, val, valsz);
  ck_assert_msg(res == 0, "Failed to add item to key '%s': %s", key,
    strerror(errno));

  mark_point();
  res = pr_redis_set_getall(p, redis, &m, key, &values, &valueszs);
  ck_assert_msg(res == 0, "Failed to get items in set: %s", strerror(errno));
  ck_assert_msg(values != NULL, "Expected values, got null");
  ck_assert_msg(valueszs != NULL, "Expected valueszs, got null");
  ck_assert_msg(values->nelts == 3, "Expected 3, got %u", values->nelts);
  ck_assert_msg(valueszs->nelts == 3, "Expected 3, got %u", valueszs->nelts);

  mark_point();
  res = pr_redis_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove key '%s': %s", key, strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_set_setall_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  array_header *vals, *valszs;

  mark_point();
  res = pr_redis_set_setall(NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_set_setall(redis, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_set_setall(redis, &m, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testsetkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_set_setall(redis, &m, key, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null values");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  vals = make_array(p, 0, sizeof(char *));

  mark_point();
  res = pr_redis_set_setall(redis, &m, key, vals, NULL);
  ck_assert_msg(res < 0, "Failed to handle empty values");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  *((char **) push_array(vals)) = pstrdup(p, "Some JSON here");

  mark_point();
  res = pr_redis_set_setall(redis, &m, key, vals, NULL);
  ck_assert_msg(res < 0, "Failed to handle null valueszs");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  valszs = make_array(p, 0, sizeof(char *));

  mark_point();
  res = pr_redis_set_setall(redis, &m, key, vals, valszs);
  ck_assert_msg(res < 0, "Failed to handle empty valueszs");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  *((size_t *) push_array(valszs)) = strlen("Some JSON here");
  *((char **) push_array(vals)) = pstrdup(p, "bar");

  mark_point();
  res = pr_redis_set_setall(redis, &m, key, vals, valszs);
  ck_assert_msg(res < 0, "Failed to handle mismatched values/valueszs");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  *((size_t *) push_array(valszs)) = strlen("bar");

  mark_point();
  (void) pr_redis_set_remove(redis, &m, key);

  mark_point();
  res = pr_redis_set_setall(redis, &m, key, vals, valszs);
  ck_assert_msg(res == 0, "Failed to set items using key '%s': %s",
    key, strerror(errno));

  mark_point();
  res = pr_redis_set_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove set '%s': %s", key, strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_sorted_set_remove_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;

  mark_point();
  res = pr_redis_sorted_set_remove(NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_sorted_set_remove(redis, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_sorted_set_remove(redis, &m, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testkey";

  mark_point();
  res = pr_redis_sorted_set_remove(redis, &m, key);
  ck_assert_msg(res < 0, "Unexpectedly removed key '%s'", key);
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_sorted_set_exists_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *val;
  size_t valsz;

  mark_point();
  res = pr_redis_sorted_set_exists(NULL, NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_sorted_set_exists(redis, NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_sorted_set_exists(redis, &m, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_sorted_set_exists(redis, &m, key, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  val = "testval";
  valsz = 0;

  mark_point();
  res = pr_redis_sorted_set_exists(redis, &m, key, val, valsz);
  ck_assert_msg(res < 0, "Failed to handle zero valuesz");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  valsz = strlen(val);

  mark_point();
  res = pr_redis_sorted_set_exists(redis, &m, key, val, valsz);
  ck_assert_msg(res == FALSE, "Failed to handle nonexistent item");

  mark_point();
  res = pr_redis_sorted_set_add(redis, &m, key, val, valsz, 1.0);
  ck_assert_msg(res == 0, "Failed to add item to key '%s': %s", key,
    strerror(errno));

  mark_point();
  res = pr_redis_sorted_set_exists(redis, &m, key, val, valsz);
  ck_assert_msg(res == TRUE, "Failed to handle existing item");

  mark_point();
  res = pr_redis_sorted_set_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove key '%s': %s", key, strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_sorted_set_add_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *val;
  size_t valsz;
  float score;

  mark_point();
  res = pr_redis_sorted_set_add(NULL, NULL, NULL, NULL, 0, 0.0);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_sorted_set_add(redis, NULL, NULL, NULL, 0, 0.0);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_sorted_set_add(redis, &m, NULL, NULL, 0, 0.0);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_sorted_set_add(redis, &m, key, NULL, 0, 0.0);
  ck_assert_msg(res < 0, "Failed to handle null value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  val = "testval";
  valsz = 0;

  mark_point();
  res = pr_redis_sorted_set_add(redis, &m, key, val, 0, 0.0);
  ck_assert_msg(res < 0, "Failed to handle zero valuesz");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  valsz = strlen(val);
  score = 75.32;

  mark_point();
  res = pr_redis_sorted_set_add(redis, &m, key, val, valsz, score);
  ck_assert_msg(res == 0, "Failed to add key '%s', val '%s': %s", key, val,
    strerror(errno));

  mark_point();
  res = pr_redis_sorted_set_add(redis, &m, key, val, valsz, score);
  ck_assert_msg(res < 0, "Failed to handle duplicates");
  ck_assert_msg(errno == EEXIST, "Expected EEXIST (%d), got %s (%d)", EEXIST,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove key '%s': %s", key, strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_sorted_set_count_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  uint64_t count;
  void *val;
  size_t valsz;
  float score;

  mark_point();
  res = pr_redis_sorted_set_count(NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_sorted_set_count(redis, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_sorted_set_count(redis, &m, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();

  key = "testkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_sorted_set_count(redis, &m, key, NULL);
  ck_assert_msg(res < 0, "Failed to handle null count");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_sorted_set_count(redis, &m, key, &count);
  ck_assert_msg(res == 0, "Failed to handle get sorted set count: %s",
    strerror(errno));
  ck_assert_msg(count == 0, "Expected 0, got %lu", (unsigned long) count);

  val = "testval";
  valsz = strlen(val);
  score = 23.45;

  mark_point();
  res = pr_redis_sorted_set_add(redis, &m, key, val, valsz, score);
  ck_assert_msg(res == 0, "Failed to add item to key '%s': %s", key,
    strerror(errno));

  mark_point();
  res = pr_redis_sorted_set_count(redis, &m, key, &count);
  ck_assert_msg(res == 0, "Failed to handle get set count: %s", strerror(errno));
  ck_assert_msg(count == 1, "Expected 1, got %lu", (unsigned long) count);

  mark_point();
  res = pr_redis_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove key '%s': %s", key, strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_sorted_set_delete_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *val;
  size_t valsz;
  float score;

  mark_point();
  res = pr_redis_sorted_set_delete(NULL, NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_sorted_set_delete(redis, NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_sorted_set_delete(redis, &m, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_sorted_set_delete(redis, &m, key, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  val = "testval";
  valsz = 0;

  mark_point();
  res = pr_redis_sorted_set_delete(redis, &m, key, val, valsz);
  ck_assert_msg(res < 0, "Failed to handle zero valuesz");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  valsz = strlen(val);

  mark_point();
  res = pr_redis_sorted_set_delete(redis, &m, key, val, valsz);
  ck_assert_msg(res < 0, "Failed to handle nonexistent item");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  score = 1.23;

  mark_point();
  res = pr_redis_sorted_set_add(redis, &m, key, val, valsz, score);
  ck_assert_msg(res == 0, "Failed to add item to key '%s': %s", key,
    strerror(errno));

  mark_point();
  res = pr_redis_sorted_set_delete(redis, &m, key, val, valsz);
  ck_assert_msg(res == 0, "Failed to delete item from set: %s", strerror(errno));

  /* Note that we add this item back, just so that the set is NOT empty when
   * we go to remove it entirely.
   */

  mark_point();
  res = pr_redis_sorted_set_add(redis, &m, key, val, valsz, score);
  ck_assert_msg(res == 0, "Failed to add item to key '%s': %s", key,
    strerror(errno));

  mark_point();
  res = pr_redis_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove key '%s': %s", key, strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_sorted_set_getn_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *val;
  size_t valsz;
  float score;
  array_header *values = NULL, *valueszs = NULL;

  mark_point();
  res = pr_redis_sorted_set_getn(NULL, NULL, NULL, NULL, 0, 0, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_sorted_set_getn(p, NULL, NULL, NULL, 0, 0, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_sorted_set_getn(p, redis, NULL, NULL, 0, 0, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_sorted_set_getn(p, redis, &m, NULL, 0, 0, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_sorted_set_getn(p, redis, &m, key, 0, 0, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null values");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_sorted_set_getn(p, redis, &m, key, 0, 0, &values, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null valueszs");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_sorted_set_getn(p, redis, &m, key, 0, 0, &values, &valueszs,
    0);
  ck_assert_msg(res < 0, "Failed to handle invalid flags value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  val = "foo";
  valsz = strlen(val);
  score = 0.123;

  mark_point();
  res = pr_redis_sorted_set_add(redis, &m, key, val, valsz, score);
  ck_assert_msg(res == 0, "Failed to add item to key '%s': %s", key,
    strerror(errno));

  val = "bar";
  valsz = strlen(val);
  score = -1.56;

  mark_point();
  res = pr_redis_sorted_set_add(redis, &m, key, val, valsz, score);
  ck_assert_msg(res == 0, "Failed to add item to key '%s': %s", key,
    strerror(errno));

  val = "baz";
  valsz = strlen(val);
  score = 234235.1;

  mark_point();
  res = pr_redis_sorted_set_add(redis, &m, key, val, valsz, score);
  ck_assert_msg(res == 0, "Failed to add item to key '%s': %s", key,
    strerror(errno));

  mark_point();
  res = pr_redis_sorted_set_getn(p, redis, &m, key, 0, 3, &values, &valueszs,
    PR_REDIS_SORTED_SET_FL_DESC);
  ck_assert_msg(res == 0, "Failed to get items in sorted set: %s",
    strerror(errno));
  ck_assert_msg(values != NULL, "Expected values, got null");
  ck_assert_msg(valueszs != NULL, "Expected valueszs, got null");
  ck_assert_msg(values->nelts == 3, "Expected 3, got %u", values->nelts);
  ck_assert_msg(valueszs->nelts == 3, "Expected 3, got %u", valueszs->nelts);

  mark_point();
  res = pr_redis_sorted_set_getn(p, redis, &m, key, 1, 2, &values, &valueszs,
    PR_REDIS_SORTED_SET_FL_ASC);
  ck_assert_msg(res == 0, "Failed to get items in sorted set: %s",
    strerror(errno));
  ck_assert_msg(values != NULL, "Expected values, got null");
  ck_assert_msg(valueszs != NULL, "Expected valueszs, got null");
  ck_assert_msg(values->nelts == 2, "Expected 2, got %u", values->nelts);
  ck_assert_msg(valueszs->nelts == 2, "Expected 2, got %u", valueszs->nelts);

  mark_point();
  res = pr_redis_sorted_set_getn(p, redis, &m, key, 1, 10, &values, &valueszs,
    PR_REDIS_SORTED_SET_FL_ASC);
  ck_assert_msg(res == 0, "Failed to get items in sorted set: %s",
    strerror(errno));
  ck_assert_msg(values != NULL, "Expected values, got null");
  ck_assert_msg(valueszs != NULL, "Expected valueszs, got null");
  ck_assert_msg(values->nelts == 2, "Expected 2, got %u", values->nelts);
  ck_assert_msg(valueszs->nelts == 2, "Expected 2, got %u", valueszs->nelts);

  mark_point();
  res = pr_redis_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove key '%s': %s", key, strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_sorted_set_incr_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *val;
  size_t valsz;
  float incr, curr;

  mark_point();
  res = pr_redis_sorted_set_incr(NULL, NULL, NULL, NULL, 0, 0.0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_sorted_set_incr(redis, NULL, NULL, NULL, 0, 0.0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_sorted_set_incr(redis, &m, NULL, NULL, 0, 0.0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testval";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_sorted_set_incr(redis, &m, key, NULL, 0, 0.0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  val = "foo";

  mark_point();
  res = pr_redis_sorted_set_incr(redis, &m, key, val, 0, 0.0, NULL);
  ck_assert_msg(res < 0, "Failed to handle empty value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  valsz = strlen(val);
  incr = 2.0;

  mark_point();
  res = pr_redis_sorted_set_incr(redis, &m, key, val, valsz, incr, NULL);
  ck_assert_msg(res < 0, "Failed to handle null current value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_sorted_set_incr(redis, &m, key, val, valsz, incr, &curr);
  ck_assert_msg(res < 0, "Failed to handle nonexistent key");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_sorted_set_add(redis, &m, key, val, valsz, incr);
  ck_assert_msg(res == 0, "Failed to set key '%s', val '%s': %s", key, val,
    strerror(errno));

  mark_point();
  res = pr_redis_sorted_set_incr(redis, &m, key, val, valsz, -incr, &curr);
  ck_assert_msg(res == 0, "Failed to increment key '%s', val '%s': %s", key, val,
    strerror(errno));

  mark_point();
  res = pr_redis_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove key '%s': %s", key, strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_sorted_set_score_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *val;
  size_t valsz;
  float score;

  mark_point();
  res = pr_redis_sorted_set_score(NULL, NULL, NULL, NULL, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_sorted_set_score(redis, NULL, NULL, NULL, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_sorted_set_score(redis, &m, NULL, NULL, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testval";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_sorted_set_score(redis, &m, key, NULL, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  val = "foo";

  mark_point();
  res = pr_redis_sorted_set_score(redis, &m, key, val, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle empty value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  valsz = strlen(val);

  mark_point();
  res = pr_redis_sorted_set_score(redis, &m, key, val, valsz, NULL);
  ck_assert_msg(res < 0, "Failed to handle null score");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_sorted_set_score(redis, &m, key, val, valsz, &score);
  ck_assert_msg(res < 0, "Failed to handle nonexistent key");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_sorted_set_add(redis, &m, key, val, valsz, 1.0);
  ck_assert_msg(res == 0, "Failed to set key '%s', val '%s': %s", key, val,
    strerror(errno));

  mark_point();
  res = pr_redis_sorted_set_score(redis, &m, key, val, valsz, &score);
  ck_assert_msg(res == 0, "Failed to score key '%s', val '%s': %s", key, val,
    strerror(errno));
  ck_assert_msg(score > 0.0, "Expected > 0.0, got %0.3f", score);

  mark_point();
  res = pr_redis_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove key '%s': %s", key, strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_sorted_set_set_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  char *val;
  size_t valsz;
  float score;

  mark_point();
  res = pr_redis_sorted_set_set(NULL, NULL, NULL, NULL, 0, 0.0);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_sorted_set_set(redis, NULL, NULL, NULL, 0, 0.0);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_sorted_set_set(redis, &m, NULL, NULL, 0, 0.0);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_sorted_set_set(redis, &m, key, NULL, 0, 0.0);
  ck_assert_msg(res < 0, "Failed to handle null value");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  val = "testval";
  valsz = 0;

  mark_point();
  res = pr_redis_sorted_set_set(redis, &m, key, val, 0, 0.0);
  ck_assert_msg(res < 0, "Failed to handle zero valuesz");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  valsz = strlen(val);
  score = 75.32;

  mark_point();
  res = pr_redis_sorted_set_set(redis, &m, key, val, valsz, score);
  ck_assert_msg(res < 0, "Failed to handle nonexistent key");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_sorted_set_add(redis, &m, key, val, valsz, score);
  ck_assert_msg(res == 0, "Failed to add key '%s', val '%s': %s", key, val,
    strerror(errno));

  score = 23.11;

  mark_point();
  res = pr_redis_sorted_set_set(redis, &m, key, val, valsz, score);
  ck_assert_msg(res == 0, "Failed to set key '%s', val '%s': %s", key, val,
    strerror(errno));

  mark_point();
  res = pr_redis_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove key '%s': %s", key, strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

START_TEST (redis_sorted_set_setall_test) {
  int res;
  pr_redis_t *redis;
  module m;
  const char *key;
  array_header *vals, *valszs, *scores;

  mark_point();
  res = pr_redis_sorted_set_setall(NULL, NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null redis");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  redis = pr_redis_conn_new(p, NULL, 0);
  ck_assert_msg(redis != NULL, "Failed to open connection to Redis: %s",
    strerror(errno));

  mark_point();
  res = pr_redis_sorted_set_setall(redis, NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null module");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_redis_sorted_set_setall(redis, &m, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "testsetkey";
  (void) pr_redis_remove(redis, &m, key);

  mark_point();
  res = pr_redis_sorted_set_setall(redis, &m, key, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null values");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  vals = make_array(p, 0, sizeof(char *));

  mark_point();
  res = pr_redis_sorted_set_setall(redis, &m, key, vals, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle empty values");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  *((char **) push_array(vals)) = pstrdup(p, "Some JSON here");

  mark_point();
  res = pr_redis_sorted_set_setall(redis, &m, key, vals, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null valueszs");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  valszs = make_array(p, 0, sizeof(char *));

  mark_point();
  res = pr_redis_sorted_set_setall(redis, &m, key, vals, valszs, NULL);
  ck_assert_msg(res < 0, "Failed to handle empty valueszs");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  *((size_t *) push_array(valszs)) = strlen("Some JSON here");
  *((char **) push_array(vals)) = pstrdup(p, "bar");

  mark_point();
  res = pr_redis_sorted_set_setall(redis, &m, key, vals, valszs, NULL);
  ck_assert_msg(res < 0, "Failed to handle mismatched values/valueszs");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  *((size_t *) push_array(valszs)) = strlen("bar");

  mark_point();
  res = pr_redis_sorted_set_setall(redis, &m, key, vals, valszs, NULL);
  ck_assert_msg(res < 0, "Failed to handle null scores");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  scores = make_array(p, 0, sizeof(char *));

  mark_point();
  res = pr_redis_sorted_set_setall(redis, &m, key, vals, valszs, scores);
  ck_assert_msg(res < 0, "Failed to handle empty scores");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  *((float *) push_array(scores)) = 1.0;

  mark_point();
  res = pr_redis_sorted_set_setall(redis, &m, key, vals, valszs, scores);
  ck_assert_msg(res < 0, "Failed to handle mismatched values/scores");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  *((float *) push_array(scores)) = 2.0;

  mark_point();
  (void) pr_redis_set_remove(redis, &m, key);

  mark_point();
  res = pr_redis_sorted_set_setall(redis, &m, key, vals, valszs, scores);
  ck_assert_msg(res == 0, "Failed to set items using key '%s': %s",
    key, strerror(errno));

  mark_point();
  res = pr_redis_set_remove(redis, &m, key);
  ck_assert_msg(res == 0, "Failed to remove set '%s': %s", key, strerror(errno));

  mark_point();
  res = pr_redis_conn_destroy(redis);
  ck_assert_msg(res == TRUE, "Failed to close redis: %s", strerror(errno));
}
END_TEST

#endif /* PR_USE_REDIS */

Suite *tests_get_redis_suite(void) {
  Suite *suite;
  TCase *testcase;

  suite = suite_create("redis");
  testcase = tcase_create("base");

#ifdef PR_USE_REDIS
  tcase_add_checked_fixture(testcase, set_up, tear_down);

  tcase_add_test(testcase, redis_conn_destroy_test);
  tcase_add_test(testcase, redis_conn_close_test);
  tcase_add_test(testcase, redis_conn_new_test);
  tcase_add_test(testcase, redis_conn_get_test);
  tcase_add_test(testcase, redis_conn_set_namespace_test);
  tcase_add_test(testcase, redis_conn_get_version_test);
  tcase_add_test(testcase, redis_conn_auth_test);
  tcase_add_test(testcase, redis_conn_auth2_test);
  tcase_add_test(testcase, redis_conn_select_test);
  tcase_add_test(testcase, redis_conn_reconnect_test);
  tcase_add_test(testcase, redis_command_test);

  tcase_add_test(testcase, redis_sentinel_get_master_addr_test);
  tcase_add_test(testcase, redis_sentinel_get_masters_test);
  tcase_add_test(testcase, redis_sentinel_conn_new_test);

  tcase_add_test(testcase, redis_remove_test);
  tcase_add_test(testcase, redis_add_test);
  tcase_add_test(testcase, redis_add_with_namespace_test);
  tcase_add_test(testcase, redis_get_test);
  tcase_add_test(testcase, redis_get_with_namespace_test);
  tcase_add_test(testcase, redis_get_str_test);
  tcase_add_test(testcase, redis_incr_test);
  tcase_add_test(testcase, redis_decr_test);
  tcase_add_test(testcase, redis_rename_test);
  tcase_add_test(testcase, redis_set_test);

  tcase_add_test(testcase, redis_hash_remove_test);
  tcase_add_test(testcase, redis_hash_get_test);
  tcase_add_test(testcase, redis_hash_set_test);
  tcase_add_test(testcase, redis_hash_delete_test);
  tcase_add_test(testcase, redis_hash_count_test);
  tcase_add_test(testcase, redis_hash_exists_test);
  tcase_add_test(testcase, redis_hash_incr_test);
  tcase_add_test(testcase, redis_hash_keys_test);
  tcase_add_test(testcase, redis_hash_values_test);
  tcase_add_test(testcase, redis_hash_getall_test);
  tcase_add_test(testcase, redis_hash_setall_test);

  tcase_add_test(testcase, redis_list_remove_test);
  tcase_add_test(testcase, redis_list_append_test);
  tcase_add_test(testcase, redis_list_count_test);
  tcase_add_test(testcase, redis_list_delete_test);
  tcase_add_test(testcase, redis_list_exists_test);
  tcase_add_test(testcase, redis_list_get_test);
  tcase_add_test(testcase, redis_list_getall_test);
  tcase_add_test(testcase, redis_list_pop_params_test);
  tcase_add_test(testcase, redis_list_pop_left_test);
  tcase_add_test(testcase, redis_list_pop_right_test);
  tcase_add_test(testcase, redis_list_push_params_test);
  tcase_add_test(testcase, redis_list_push_left_test);
  tcase_add_test(testcase, redis_list_push_right_test);
  tcase_add_test(testcase, redis_list_rotate_test);
  tcase_add_test(testcase, redis_list_set_test);
  tcase_add_test(testcase, redis_list_setall_test);

  tcase_add_test(testcase, redis_set_remove_test);
  tcase_add_test(testcase, redis_set_exists_test);
  tcase_add_test(testcase, redis_set_add_test);
  tcase_add_test(testcase, redis_set_count_test);
  tcase_add_test(testcase, redis_set_delete_test);
  tcase_add_test(testcase, redis_set_getall_test);
  tcase_add_test(testcase, redis_set_setall_test);

  tcase_add_test(testcase, redis_sorted_set_remove_test);
  tcase_add_test(testcase, redis_sorted_set_exists_test);
  tcase_add_test(testcase, redis_sorted_set_add_test);
  tcase_add_test(testcase, redis_sorted_set_count_test);
  tcase_add_test(testcase, redis_sorted_set_delete_test);
  tcase_add_test(testcase, redis_sorted_set_getn_test);
  tcase_add_test(testcase, redis_sorted_set_incr_test);
  tcase_add_test(testcase, redis_sorted_set_score_test);
  tcase_add_test(testcase, redis_sorted_set_set_test);
  tcase_add_test(testcase, redis_sorted_set_setall_test);

  /* Some of the Redis tests may take a little longer. */
  tcase_set_timeout(testcase, 30);
#endif /* PR_USE_REDIS */

  suite_add_tcase(suite, testcase);
  return suite;
}
