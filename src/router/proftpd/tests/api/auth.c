/*
 * ProFTPD - FTP server testsuite
 * Copyright (c) 2014-2020 The ProFTPD Project team
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

/* Auth API tests */

#include "tests.h"

#define PR_TEST_AUTH_NAME		"testsuite_user"
#define PR_TEST_AUTH_NOBODY		"testsuite_nobody"
#define PR_TEST_AUTH_NOBODY2		"testsuite_nobody2"
#define PR_TEST_AUTH_NOGROUP		"testsuite_nogroup"
#define PR_TEST_AUTH_UID		500
#define PR_TEST_AUTH_UID_STR		"500"
#define PR_TEST_AUTH_NOUID		666
#define PR_TEST_AUTH_NOUID2		667
#define PR_TEST_AUTH_GID		500
#define PR_TEST_AUTH_GID_STR		"500"
#define PR_TEST_AUTH_NOGID		666
#define PR_TEST_AUTH_HOME		"/tmp"
#define PR_TEST_AUTH_SHELL		"/bin/bash"
#define PR_TEST_AUTH_PASSWD		"password"

static pool *p = NULL;
static server_rec *test_server = NULL;

static struct passwd test_pwd;
static struct group test_grp;

static unsigned int setpwent_count = 0;
static unsigned int endpwent_count = 0;
static unsigned int getpwent_count = 0;
static unsigned int getpwnam_count = 0;
static unsigned int getpwuid_count = 0;
static unsigned int name2uid_count = 0;
static unsigned int uid2name_count = 0;

static unsigned int setgrent_count = 0;
static unsigned int endgrent_count = 0;
static unsigned int getgrent_count = 0;
static unsigned int getgrnam_count = 0;
static unsigned int getgrgid_count = 0;
static unsigned int name2gid_count = 0;
static unsigned int gid2name_count = 0;
static unsigned int getgroups_count = 0;

static module testsuite_module = {
  NULL, NULL,

  /* Module API version */
  0x20,

  /* Module name */
  "testsuite",

  /* Module configuration directive table */
  NULL,

  /* Module command handler table */
  NULL,

  /* Module authentication handler table */
  NULL,

  /* Module initialization function */
  NULL,

  /* Session initialization function */
  NULL
};

MODRET handle_setpwent(cmd_rec *cmd) {
  setpwent_count++;
  return PR_HANDLED(cmd);
}

MODRET handle_endpwent(cmd_rec *cmd) {
  endpwent_count++;
  return PR_HANDLED(cmd);
}

MODRET handle_getpwent(cmd_rec *cmd) {
  getpwent_count++;

  if (getpwent_count == 1) {
    test_pwd.pw_uid = PR_TEST_AUTH_UID;
    test_pwd.pw_gid = PR_TEST_AUTH_GID;
    return mod_create_data(cmd, &test_pwd);
  }

  if (getpwent_count == 2) {
    test_pwd.pw_uid = (uid_t) -1;
    test_pwd.pw_gid = PR_TEST_AUTH_GID;
    return mod_create_data(cmd, &test_pwd);
  }

  if (getpwent_count == 3) {
    test_pwd.pw_uid = PR_TEST_AUTH_UID;
    test_pwd.pw_gid = (gid_t) -1;
    return mod_create_data(cmd, &test_pwd);
  }

  return PR_DECLINED(cmd);
}

MODRET handle_getpwnam(cmd_rec *cmd) {
  const char *name;

  name = cmd->argv[0];
  getpwnam_count++;

  if (strcmp(name, PR_TEST_AUTH_NAME) == 0) {
    test_pwd.pw_uid = PR_TEST_AUTH_UID;
    test_pwd.pw_gid = PR_TEST_AUTH_GID;
    return mod_create_data(cmd, &test_pwd);
  }

  if (strcmp(name, PR_TEST_AUTH_NOBODY) == 0) {
    test_pwd.pw_uid = (uid_t) -1;
    test_pwd.pw_gid = PR_TEST_AUTH_GID;
    return mod_create_data(cmd, &test_pwd);
  }

  if (strcmp(name, PR_TEST_AUTH_NOBODY2) == 0) {
    test_pwd.pw_uid = PR_TEST_AUTH_UID;
    test_pwd.pw_gid = (gid_t) -1;
    return mod_create_data(cmd, &test_pwd);
  }

  return PR_DECLINED(cmd);
}

MODRET handle_getpwuid(cmd_rec *cmd) {
  uid_t uid;

  uid = *((uid_t *) cmd->argv[0]);
  getpwuid_count++;

  if (uid == PR_TEST_AUTH_UID) {
    test_pwd.pw_uid = PR_TEST_AUTH_UID;
    test_pwd.pw_gid = PR_TEST_AUTH_GID;
    return mod_create_data(cmd, &test_pwd);
  }

  if (uid == PR_TEST_AUTH_NOUID) {
    test_pwd.pw_uid = (uid_t) -1;
    test_pwd.pw_gid = PR_TEST_AUTH_GID;
    return mod_create_data(cmd, &test_pwd);
  }

  if (uid == PR_TEST_AUTH_NOUID2) {
    test_pwd.pw_uid = PR_TEST_AUTH_UID;
    test_pwd.pw_gid = (gid_t) -1;
    return mod_create_data(cmd, &test_pwd);
  }

  return PR_DECLINED(cmd);
}

MODRET decline_name2uid(cmd_rec *cmd) {
  name2uid_count++;
  return PR_DECLINED(cmd);
}

MODRET handle_name2uid(cmd_rec *cmd) {
  const char *name;

  name = cmd->argv[0];
  name2uid_count++;

  if (strcmp(name, PR_TEST_AUTH_NAME) != 0) {
    return PR_DECLINED(cmd);
  }

  return mod_create_data(cmd, (void *) &(test_pwd.pw_uid));
}

MODRET decline_uid2name(cmd_rec *cmd) {
  uid2name_count++;
  return PR_DECLINED(cmd);
}

MODRET handle_uid2name(cmd_rec *cmd) {
  uid_t uid;

  uid = *((uid_t *) cmd->argv[0]);
  uid2name_count++;

  if (uid != PR_TEST_AUTH_UID) {
    return PR_DECLINED(cmd);
  }

  return mod_create_data(cmd, test_pwd.pw_name);
}

MODRET handle_setgrent(cmd_rec *cmd) {
  setgrent_count++;
  return PR_HANDLED(cmd);
}

MODRET handle_endgrent(cmd_rec *cmd) {
  endgrent_count++;
  return PR_HANDLED(cmd);
}

MODRET handle_getgrent(cmd_rec *cmd) {
  getgrent_count++;

  if (getgrent_count == 1) {
    test_grp.gr_gid = PR_TEST_AUTH_GID;
    return mod_create_data(cmd, &test_grp);
  }

  if (getgrent_count == 2) {
    test_grp.gr_gid = (gid_t) -1;
    return mod_create_data(cmd, &test_grp);
  }

  return PR_DECLINED(cmd);
}

MODRET handle_getgrnam(cmd_rec *cmd) {
  const char *name;

  name = cmd->argv[0];
  getgrnam_count++;

  if (strcmp(name, PR_TEST_AUTH_NAME) == 0) {
    test_grp.gr_gid = PR_TEST_AUTH_GID;
    return mod_create_data(cmd, &test_grp);
  }

  if (strcmp(name, PR_TEST_AUTH_NOGROUP) == 0) {
    test_grp.gr_gid = (gid_t) -1;
    return mod_create_data(cmd, &test_grp);
  }

  return PR_DECLINED(cmd);
}

MODRET handle_getgrgid(cmd_rec *cmd) {
  gid_t gid;

  gid = *((gid_t *) cmd->argv[0]);
  getgrgid_count++;

  if (gid == PR_TEST_AUTH_GID) {
    test_grp.gr_gid = PR_TEST_AUTH_GID;
    return mod_create_data(cmd, &test_grp);
  }

  if (gid == PR_TEST_AUTH_NOGID) {
    test_grp.gr_gid = (gid_t) -1;
    return mod_create_data(cmd, &test_grp);
  }

  return PR_DECLINED(cmd);
}

MODRET decline_name2gid(cmd_rec *cmd) {
  name2gid_count++;
  return PR_DECLINED(cmd);
}

MODRET handle_name2gid(cmd_rec *cmd) {
  const char *name;

  name = cmd->argv[0];
  name2gid_count++;

  if (strcmp(name, PR_TEST_AUTH_NAME) != 0) {
    return PR_DECLINED(cmd);
  }

  return mod_create_data(cmd, (void *) &(test_grp.gr_gid));
}

MODRET decline_gid2name(cmd_rec *cmd) {
  gid2name_count++;
  return PR_DECLINED(cmd);
}

MODRET handle_gid2name(cmd_rec *cmd) {
  gid_t gid;

  gid = *((gid_t *) cmd->argv[0]);
  gid2name_count++;

  if (gid != PR_TEST_AUTH_GID) {
    return PR_DECLINED(cmd);
  }

  return mod_create_data(cmd, test_grp.gr_name);
}

MODRET handle_getgroups(cmd_rec *cmd) {
  const char *name;
  array_header *gids = NULL, *names = NULL;

  name = (char *) cmd->argv[0];

  if (cmd->argv[1]) {
    gids = (array_header *) cmd->argv[1];
  }

  if (cmd->argv[2]) {
    names = (array_header *) cmd->argv[2];
  }

  getgroups_count++;

  if (strcmp(name, PR_TEST_AUTH_NAME) != 0) {
    return PR_DECLINED(cmd);
  }

  if (gids) { 
    *((gid_t *) push_array(gids)) = PR_TEST_AUTH_GID;
  }

  if (names) {
    *((char **) push_array(names)) = pstrdup(p, PR_TEST_AUTH_NAME);
  }

  return mod_create_data(cmd, (void *) &gids->nelts);
}

static int authn_rfc2228 = FALSE;

MODRET handle_authn(cmd_rec *cmd) {
  const char *user, *cleartext_passwd;

  user = cmd->argv[0];
  cleartext_passwd = cmd->argv[1];

  if (strcmp(user, PR_TEST_AUTH_NAME) == 0) {
    if (strcmp(cleartext_passwd, PR_TEST_AUTH_PASSWD) == 0) {
      if (authn_rfc2228) {
        authn_rfc2228 = FALSE;
        return mod_create_data(cmd, (void *) PR_AUTH_RFC2228_OK);
      }

      return PR_HANDLED(cmd);
    }

    return PR_ERROR_INT(cmd, PR_AUTH_BADPWD);
  }

  return PR_DECLINED(cmd);
}

MODRET handle_authz(cmd_rec *cmd) {
  const char *user;

  user = cmd->argv[0];

  if (strcmp(user, PR_TEST_AUTH_NAME) == 0) {
    return PR_HANDLED(cmd);
  }

  return PR_ERROR_INT(cmd, PR_AUTH_NOPWD);
}

static int check_rfc2228 = FALSE;

MODRET handle_check(cmd_rec *cmd) {
  const char *user, *cleartext_passwd, *ciphertext_passwd;

  ciphertext_passwd = cmd->argv[0];
  user = cmd->argv[1];
  cleartext_passwd = cmd->argv[2];

  if (strcmp(user, PR_TEST_AUTH_NAME) == 0) {
    if (ciphertext_passwd != NULL &&
        strcmp(ciphertext_passwd, cleartext_passwd) == 0) {
      if (check_rfc2228) {
        check_rfc2228 = FALSE;
        return mod_create_data(cmd, (void *) PR_AUTH_RFC2228_OK);
      }

      return PR_HANDLED(cmd);
    }

    return PR_ERROR_INT(cmd, PR_AUTH_BADPWD);
  }

  return PR_DECLINED(cmd);
}

MODRET handle_requires_pass(cmd_rec *cmd) {
  const char *name;

  name = cmd->argv[0];

  if (strcmp(name, PR_TEST_AUTH_NAME) == 0) {
    return mod_create_data(cmd, (void *) PR_AUTH_RFC2228_OK);
  }

  return PR_DECLINED(cmd);
}

/* Fixtures */

static void set_up(void) {
  if (p == NULL) {
    p = permanent_pool = make_sub_pool(NULL);
  }

  init_stash();
  init_auth();
  (void) pr_auth_cache_set(TRUE, PR_AUTH_CACHE_FL_DEFAULT);

  if (getenv("TEST_VERBOSE") != NULL) {
    pr_trace_set_levels("auth", 1, 20);
  }

  test_server = pcalloc(p, sizeof(server_rec));
  tests_stubs_set_main_server(test_server);

  test_pwd.pw_name = PR_TEST_AUTH_NAME;
  test_pwd.pw_uid = PR_TEST_AUTH_UID;
  test_pwd.pw_gid = PR_TEST_AUTH_GID;
  test_pwd.pw_dir = PR_TEST_AUTH_HOME;
  test_pwd.pw_shell = PR_TEST_AUTH_SHELL;

  test_grp.gr_name = PR_TEST_AUTH_NAME;
  test_grp.gr_gid = PR_TEST_AUTH_GID;

  /* Reset counters. */
  setpwent_count = 0;
  endpwent_count = 0;
  getpwent_count = 0;
  getpwnam_count = 0;
  getpwuid_count = 0;
  name2uid_count = 0;
  uid2name_count = 0;

  setgrent_count = 0;
  endgrent_count = 0;
  getgrent_count = 0;
  getgrnam_count = 0;
  getgrgid_count = 0;
  name2gid_count = 0;
  gid2name_count = 0;
  getgroups_count = 0;

  pr_auth_cache_clear();
}

static void tear_down(void) {
  (void) pr_auth_cache_set(TRUE, PR_AUTH_CACHE_FL_DEFAULT);

  if (getenv("TEST_VERBOSE") != NULL) {
    pr_trace_set_levels("auth", 0, 0);
  }

  if (p != NULL) {
    destroy_pool(p);
    p = session.pool = permanent_pool = NULL;
  }

  test_server = NULL;
  tests_stubs_set_main_server(NULL);
}

/* Tests */

START_TEST (auth_setpwent_test) {
  int res;
  authtable authtab;
  char *sym_name = "setpwent";

  pr_auth_setpwent(p);
  ck_assert_msg(setpwent_count == 0, "Expected call count 0, got %u",
    setpwent_count);
  mark_point();
  
  /* Load the appropriate AUTH symbol, and call it. */

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = sym_name;
  authtab.handler = handle_setpwent;
  authtab.m = &testsuite_module;
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  ck_assert_msg(res == 0, "Failed to add '%s' AUTH symbol: %s", sym_name,
    strerror(errno));

  pr_auth_setpwent(p);
  ck_assert_msg(setpwent_count == 1, "Expected call count 1, got %u",
    setpwent_count);

  pr_stash_remove_symbol(PR_SYM_AUTH, sym_name, &testsuite_module);
}
END_TEST

START_TEST (auth_endpwent_test) {
  int res;
  authtable authtab;
  char *sym_name = "endpwent";

  pr_auth_endpwent(p);
  ck_assert_msg(endpwent_count == 0, "Expected call count 0, got %u",
    endpwent_count);
  mark_point();
  
  /* Load the appropriate AUTH symbol, and call it. */

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = sym_name;
  authtab.handler = handle_endpwent;
  authtab.m = &testsuite_module;
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  ck_assert_msg(res == 0, "Failed to add '%s' AUTH symbol: %s", sym_name,
    strerror(errno));

  pr_auth_endpwent(p);
  ck_assert_msg(endpwent_count == 1, "Expected call count 1, got %u",
    endpwent_count);

  pr_stash_remove_symbol(PR_SYM_AUTH, sym_name, &testsuite_module);
}
END_TEST

START_TEST (auth_getpwent_test) {
  int res;
  struct passwd *pw;
  authtable authtab;
  char *sym_name = "getpwent";

  getpwent_count = 0;

  pw = pr_auth_getpwent(NULL);
  ck_assert_msg(pw == NULL, "Found pwent unexpectedly");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL, got %d (%s)",
    errno, strerror(errno));

  pw = pr_auth_getpwent(p);
  ck_assert_msg(pw == NULL, "Found pwent unexpectedly");
  ck_assert_msg(getpwent_count == 0, "Expected call count 0, got %u",
    getpwent_count);
  mark_point();
  
  /* Load the appropriate AUTH symbol, and call it. */

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = sym_name;
  authtab.handler = handle_getpwent;
  authtab.m = &testsuite_module;
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  ck_assert_msg(res == 0, "Failed to add '%s' AUTH symbol: %s", sym_name,
    strerror(errno));

  pw = pr_auth_getpwent(p);
  ck_assert_msg(pw != NULL, "Failed to find pwent: %s", strerror(errno));
  ck_assert_msg(getpwent_count == 1, "Expected call count 1, got %u",
    getpwent_count);

  pw = pr_auth_getpwent(p);
  ck_assert_msg(pw == NULL, "Failed to avoid pwent with bad UID");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);
  ck_assert_msg(getpwent_count == 2, "Expected call count 2, got %u",
    getpwent_count);

  pw = pr_auth_getpwent(p);
  ck_assert_msg(pw == NULL, "Failed to avoid pwent with bad GID");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);
  ck_assert_msg(getpwent_count == 3, "Expected call count 3, got %u",
    getpwent_count);

  pr_auth_endpwent(p);
  pr_stash_remove_symbol(PR_SYM_AUTH, sym_name, &testsuite_module);
}
END_TEST

START_TEST (auth_getpwnam_test) {
  int res;
  struct passwd *pw;
  authtable authtab;
  char *sym_name = "getpwnam";

  pw = pr_auth_getpwnam(NULL, NULL);
  ck_assert_msg(pw == NULL, "Found pwnam unexpectedly");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL, got %d (%s)",
    errno, strerror(errno));

  pw = pr_auth_getpwnam(p, PR_TEST_AUTH_NAME);
  ck_assert_msg(pw == NULL, "Found pwnam unexpectedly");
  ck_assert_msg(getpwnam_count == 0, "Expected call count 0, got %u",
    getpwnam_count);
  mark_point();
  
  /* Load the appropriate AUTH symbol, and call it. */

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = sym_name;
  authtab.handler = handle_getpwnam;
  authtab.m = &testsuite_module;
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  ck_assert_msg(res == 0, "Failed to add '%s' AUTH symbol: %s", sym_name,
    strerror(errno));

  mark_point();

  pw = pr_auth_getpwnam(p, PR_TEST_AUTH_NOBODY);
  ck_assert_msg(pw == NULL, "Found user '%s' unexpectedly", PR_TEST_AUTH_NOBODY);
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  pw = pr_auth_getpwnam(p, PR_TEST_AUTH_NOBODY2);
  ck_assert_msg(pw == NULL, "Found user '%s' unexpectedly", PR_TEST_AUTH_NOBODY2);
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  pw = pr_auth_getpwnam(p, PR_TEST_AUTH_NAME);
  ck_assert_msg(pw != NULL, "Failed to find user '%s': %s", PR_TEST_AUTH_NAME,
    strerror(errno));
  ck_assert_msg(getpwnam_count == 3, "Expected call count 3, got %u",
    getpwnam_count);

  pw = pr_auth_getpwnam(p, PR_TEST_AUTH_NAME);
  ck_assert_msg(pw != NULL, "Failed to find user '%s': %s", PR_TEST_AUTH_NAME,
    strerror(errno));
  ck_assert_msg(getpwnam_count == 4, "Expected call count 4, got %u",
    getpwnam_count);

  mark_point();

  pw = pr_auth_getpwnam(p, "other");
  ck_assert_msg(pw == NULL, "Found pwnam for user 'other' unexpectedly");
  ck_assert_msg(errno == ENOENT, "Failed to set errno to ENOENT, got %d (%s)",
    errno, strerror(errno));
  ck_assert_msg(getpwnam_count == 5, "Expected call count 5, got %u",
    getpwnam_count);

  pr_auth_endpwent(p);
  pr_stash_remove_symbol(PR_SYM_AUTH, sym_name, &testsuite_module);
}
END_TEST

START_TEST (auth_getpwuid_test) {
  int res;
  struct passwd *pw;
  authtable authtab;
  char *sym_name = "getpwuid";

  pw = pr_auth_getpwuid(NULL, -1);
  ck_assert_msg(pw == NULL, "Found pwuid unexpectedly");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL, got %d (%s)",
    errno, strerror(errno));

  pw = pr_auth_getpwuid(p, PR_TEST_AUTH_UID);
  ck_assert_msg(pw == NULL, "Found pwuid unexpectedly");
  ck_assert_msg(getpwuid_count == 0, "Expected call count 0, got %u",
    getpwuid_count);
  mark_point();
  
  /* Load the appropriate AUTH symbol, and call it. */

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = sym_name;
  authtab.handler = handle_getpwuid;
  authtab.m = &testsuite_module;
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  ck_assert_msg(res == 0, "Failed to add '%s' AUTH symbol: %s", sym_name,
    strerror(errno));

  mark_point();

  pw = pr_auth_getpwuid(p, PR_TEST_AUTH_UID);
  ck_assert_msg(pw != NULL, "Failed to find pwuid: %s", strerror(errno));
  ck_assert_msg(getpwuid_count == 1, "Expected call count 1, got %u",
    getpwuid_count);

  pw = pr_auth_getpwuid(p, PR_TEST_AUTH_NOUID);
  ck_assert_msg(pw == NULL, "Found pwuid for NOUID unexpectedly");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  pw = pr_auth_getpwuid(p, PR_TEST_AUTH_NOUID2);
  ck_assert_msg(pw == NULL, "Found pwuid for NOUID2 unexpectedly");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  mark_point();

  pw = pr_auth_getpwuid(p, 5);
  ck_assert_msg(pw == NULL, "Found pwuid for UID 5 unexpectedly");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  ck_assert_msg(getpwuid_count == 4, "Expected call count 4, got %u",
    getpwuid_count);

  pr_stash_remove_symbol(PR_SYM_AUTH, sym_name, &testsuite_module);
}
END_TEST

START_TEST (auth_name2uid_test) {
  int res;
  uid_t uid;
  authtable authtab;
  char *sym_name = "name2uid";

  pr_auth_cache_set(FALSE, PR_AUTH_CACHE_FL_BAD_NAME2UID);

  uid = pr_auth_name2uid(NULL, NULL);
  ck_assert_msg(uid == (uid_t) -1, "Found UID unexpectedly");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL, got %d (%s)",
    errno, strerror(errno));

  uid = pr_auth_name2uid(p, PR_TEST_AUTH_NAME);
  ck_assert_msg(uid == (uid_t) -1, "Found UID unexpectedly");
  ck_assert_msg(name2uid_count == 0, "Expected call count 0, got %u",
    name2uid_count);
  mark_point();
  
  /* Load the appropriate AUTH symbol, and call it. */

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = sym_name;
  authtab.handler = handle_name2uid;
  authtab.m = &testsuite_module;
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  ck_assert_msg(res == 0, "Failed to add '%s' AUTH symbol: %s", sym_name,
    strerror(errno));

  mark_point();

  uid = pr_auth_name2uid(p, PR_TEST_AUTH_NAME);
  ck_assert_msg(uid == PR_TEST_AUTH_UID, "Expected UID %lu, got %lu",
    (unsigned long) PR_TEST_AUTH_UID, (unsigned long) uid);
  ck_assert_msg(name2uid_count == 1, "Expected call count 1, got %u",
    name2uid_count);

  mark_point();

  /* Call again; the call counter should NOT increment due to caching. */

  uid = pr_auth_name2uid(p, PR_TEST_AUTH_NAME);
  ck_assert_msg(uid == PR_TEST_AUTH_UID, "Expected UID %lu, got %lu",
    (unsigned long) PR_TEST_AUTH_UID, (unsigned long) uid);
  ck_assert_msg(name2uid_count == 1, "Expected call count 1, got %u",
    name2uid_count);

  pr_stash_remove_symbol(PR_SYM_AUTH, sym_name, &testsuite_module);
}
END_TEST

START_TEST (auth_uid2name_test) {
  int res;
  const char *name; 
  authtable authtab;
  char *sym_name = "uid2name";

  pr_auth_cache_set(FALSE, PR_AUTH_CACHE_FL_BAD_UID2NAME);

  name = pr_auth_uid2name(NULL, -1);
  ck_assert_msg(name == NULL, "Found name unexpectedly: %s", name);
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL, got %d (%s)",
    errno, strerror(errno));
  mark_point();

  name = pr_auth_uid2name(p, PR_TEST_AUTH_UID);
  ck_assert_msg(name != NULL, "Failed to find name for UID %lu: %s",
    (unsigned long) PR_TEST_AUTH_UID, strerror(errno));
  ck_assert_msg(strcmp(name, PR_TEST_AUTH_UID_STR) == 0,
     "Expected name '%s', got '%s'", PR_TEST_AUTH_UID_STR, name);
  ck_assert_msg(uid2name_count == 0, "Expected call count 0, got %u",
    uid2name_count);
  mark_point();
  
  /* Load the appropriate AUTH symbol, and call it. */

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = sym_name;
  authtab.handler = handle_uid2name;
  authtab.m = &testsuite_module;
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  ck_assert_msg(res == 0, "Failed to add '%s' AUTH symbol: %s", sym_name,
    strerror(errno));

  mark_point();

  name = pr_auth_uid2name(p, PR_TEST_AUTH_UID);
  ck_assert_msg(name != NULL, "Expected name, got null");
  ck_assert_msg(strcmp(name, PR_TEST_AUTH_NAME) == 0,
    "Expected name '%s', got '%s'", PR_TEST_AUTH_NAME, name);
  ck_assert_msg(uid2name_count == 1, "Expected call count 1, got %u",
    uid2name_count);

  pr_stash_remove_symbol(PR_SYM_AUTH, sym_name, &testsuite_module);
}
END_TEST

START_TEST (auth_setgrent_test) {
  int res;
  authtable authtab;
  char *sym_name = "setgrent";

  pr_auth_setgrent(p);
  ck_assert_msg(setgrent_count == 0, "Expected call count 0, got %u",
    setgrent_count);
  mark_point();
  
  /* Load the appropriate AUTH symbol, and call it. */

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = sym_name;
  authtab.handler = handle_setgrent;
  authtab.m = &testsuite_module;
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  ck_assert_msg(res == 0, "Failed to add '%s' AUTH symbol: %s", sym_name,
    strerror(errno));

  pr_auth_setgrent(p);
  ck_assert_msg(setgrent_count == 1, "Expected call count 1, got %u",
    setgrent_count);

  pr_stash_remove_symbol(PR_SYM_AUTH, sym_name, &testsuite_module);
}
END_TEST

START_TEST (auth_endgrent_test) {
  int res;
  authtable authtab;
  char *sym_name = "endgrent";

  pr_auth_endgrent(p);
  ck_assert_msg(endgrent_count == 0, "Expected call count 0, got %u",
    endgrent_count);
  mark_point();
  
  /* Load the appropriate AUTH symbol, and call it. */

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = sym_name;
  authtab.handler = handle_endgrent;
  authtab.m = &testsuite_module;
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  ck_assert_msg(res == 0, "Failed to add '%s' AUTH symbol: %s", sym_name,
    strerror(errno));

  pr_auth_endgrent(p);
  ck_assert_msg(endgrent_count == 1, "Expected call count 1, got %u",
    endgrent_count);

  pr_stash_remove_symbol(PR_SYM_AUTH, sym_name, &testsuite_module);
}
END_TEST

START_TEST (auth_getgrent_test) {
  int res;
  struct group *gr;
  authtable authtab;
  char *sym_name = "getgrent";

  gr = pr_auth_getgrent(NULL);
  ck_assert_msg(gr == NULL, "Found grent unexpectedly");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL, got %d (%s)",
    errno, strerror(errno));

  gr = pr_auth_getgrent(p);
  ck_assert_msg(gr == NULL, "Found grent unexpectedly");
  ck_assert_msg(getgrent_count == 0, "Expected call count 0, got %u",
    getgrent_count);
  mark_point();
  
  /* Load the appropriate AUTH symbol, and call it. */

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = sym_name;
  authtab.handler = handle_getgrent;
  authtab.m = &testsuite_module;
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  ck_assert_msg(res == 0, "Failed to add '%s' AUTH symbol: %s", sym_name,
    strerror(errno));

  gr = pr_auth_getgrent(p);
  ck_assert_msg(gr != NULL, "Failed to find grent: %s", strerror(errno));
  ck_assert_msg(getgrent_count == 1, "Expected call count 1, got %u",
    getgrent_count);

  gr = pr_auth_getgrent(p);
  ck_assert_msg(gr == NULL, "Failed to avoid grent with bad GID");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);
  ck_assert_msg(getgrent_count == 2, "Expected call count 2, got %u",
    getgrent_count);

  pr_auth_endgrent(p);
  pr_stash_remove_symbol(PR_SYM_AUTH, sym_name, &testsuite_module);
}
END_TEST

START_TEST (auth_getgrnam_test) {
  int res;
  struct group *gr;
  authtable authtab;
  char *sym_name = "getgrnam";

  gr = pr_auth_getgrnam(NULL, NULL);
  ck_assert_msg(gr == NULL, "Found grnam unexpectedly");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL, got %d (%s)",
    errno, strerror(errno));

  gr = pr_auth_getgrnam(p, PR_TEST_AUTH_NAME);
  ck_assert_msg(gr == NULL, "Found grnam unexpectedly");
  ck_assert_msg(getgrnam_count == 0, "Expected call count 0, got %u",
    getgrnam_count);
  mark_point();
  
  /* Load the appropriate AUTH symbol, and call it. */

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = sym_name;
  authtab.handler = handle_getgrnam;
  authtab.m = &testsuite_module;
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  ck_assert_msg(res == 0, "Failed to add '%s' AUTH symbol: %s", sym_name,
    strerror(errno));

  mark_point();

  gr = pr_auth_getgrnam(p, PR_TEST_AUTH_NOGROUP);
  ck_assert_msg(gr == NULL, "Found group '%s' unexpectedly",
    PR_TEST_AUTH_NOGROUP);
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  gr = pr_auth_getgrnam(p, PR_TEST_AUTH_NAME);
  ck_assert_msg(gr != NULL, "Failed to find grnam: %s", strerror(errno));
  ck_assert_msg(getgrnam_count == 2, "Expected call count 2, got %u",
    getgrnam_count);

  mark_point();

  gr = pr_auth_getgrnam(p, "other");
  ck_assert_msg(gr == NULL, "Found grnam for user 'other' unexpectedly");
  ck_assert_msg(errno == ENOENT, "Failed to set errno to ENOENT, got %d (%s)",
    errno, strerror(errno));
  ck_assert_msg(getgrnam_count == 3, "Expected call count 3, got %u",
    getgrnam_count);

  pr_stash_remove_symbol(PR_SYM_AUTH, sym_name, &testsuite_module);
}
END_TEST

START_TEST (auth_getgrgid_test) {
  int res;
  struct group *gr;
  authtable authtab;
  char *sym_name = "getgrgid";

  gr = pr_auth_getgrgid(NULL, -1);
  ck_assert_msg(gr == NULL, "Found grgid unexpectedly");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL, got %d (%s)",
    errno, strerror(errno));

  gr = pr_auth_getgrgid(p, PR_TEST_AUTH_GID);
  ck_assert_msg(gr == NULL, "Found grgid unexpectedly");
  ck_assert_msg(getgrgid_count == 0, "Expected call count 0, got %u",
    getgrgid_count);
  mark_point();
  
  /* Load the appropriate AUTH symbol, and call it. */

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = sym_name;
  authtab.handler = handle_getgrgid;
  authtab.m = &testsuite_module;
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  ck_assert_msg(res == 0, "Failed to add '%s' AUTH symbol: %s", sym_name,
    strerror(errno));

  mark_point();

  gr = pr_auth_getgrgid(p, PR_TEST_AUTH_GID);
  ck_assert_msg(gr != NULL, "Failed to find grgid: %s", strerror(errno));
  ck_assert_msg(getgrgid_count == 1, "Expected call count 1, got %u",
    getgrgid_count);

  gr = pr_auth_getgrgid(p, PR_TEST_AUTH_NOGID);
  ck_assert_msg(gr == NULL, "Found grgid for NOGID unexpectedly");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  mark_point();

  gr = pr_auth_getgrgid(p, 5);
  ck_assert_msg(gr == NULL, "Found grgid for GID 5 unexpectedly");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  ck_assert_msg(getgrgid_count == 3, "Expected call count 3, got %u",
    getgrgid_count);

  pr_stash_remove_symbol(PR_SYM_AUTH, sym_name, &testsuite_module);
}
END_TEST

START_TEST (auth_name2gid_test) {
  int res;
  gid_t gid;
  authtable authtab;
  char *sym_name = "name2gid";

  pr_auth_cache_set(FALSE, PR_AUTH_CACHE_FL_BAD_NAME2GID);

  gid = pr_auth_name2gid(NULL, NULL);
  ck_assert_msg(gid == (gid_t) -1, "Found GID unexpectedly");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL, got %d (%s)",
    errno, strerror(errno));

  gid = pr_auth_name2gid(p, PR_TEST_AUTH_NAME);
  ck_assert_msg(gid == (gid_t) -1, "Found GID unexpectedly");
  ck_assert_msg(name2gid_count == 0, "Expected call count 0, got %u",
    name2gid_count);
  mark_point();
  
  /* Load the appropriate AUTH symbol, and call it. */

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = sym_name;
  authtab.handler = handle_name2gid;
  authtab.m = &testsuite_module;
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  ck_assert_msg(res == 0, "Failed to add '%s' AUTH symbol: %s", sym_name,
    strerror(errno));

  mark_point();

  gid = pr_auth_name2gid(p, PR_TEST_AUTH_NAME);
  ck_assert_msg(gid == PR_TEST_AUTH_GID, "Expected GID %lu, got %lu",
    (unsigned long) PR_TEST_AUTH_GID, (unsigned long) gid);
  ck_assert_msg(name2gid_count == 1, "Expected call count 1, got %u",
    name2gid_count);

  mark_point();

  /* Call again; the call counter should NOT increment due to caching. */

  gid = pr_auth_name2gid(p, PR_TEST_AUTH_NAME);
  ck_assert_msg(gid == PR_TEST_AUTH_GID, "Expected GID %lu, got %lu",
    (unsigned long) PR_TEST_AUTH_GID, (unsigned long) gid);
  ck_assert_msg(name2gid_count == 1, "Expected call count 1, got %u",
    name2gid_count);

  pr_stash_remove_symbol(PR_SYM_AUTH, sym_name, &testsuite_module);
}
END_TEST

START_TEST (auth_gid2name_test) {
  int res;
  const char *name; 
  authtable authtab;
  char *sym_name = "gid2name";

  pr_auth_cache_set(FALSE, PR_AUTH_CACHE_FL_BAD_GID2NAME);

  name = pr_auth_gid2name(NULL, -1);
  ck_assert_msg(name == NULL, "Found name unexpectedly: %s", name);
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL, got %d (%s)",
    errno, strerror(errno));
  mark_point();

  name = pr_auth_gid2name(p, PR_TEST_AUTH_GID);
  ck_assert_msg(name != NULL, "Failed to find name for GID %lu: %s",
    (unsigned long) PR_TEST_AUTH_GID, strerror(errno));
  ck_assert_msg(strcmp(name, PR_TEST_AUTH_GID_STR) == 0,
     "Expected name '%s', got '%s'", PR_TEST_AUTH_GID_STR, name);
  ck_assert_msg(gid2name_count == 0, "Expected call count 0, got %u",
    gid2name_count);
  mark_point();
  
  /* Load the appropriate AUTH symbol, and call it. */

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = sym_name;
  authtab.handler = handle_gid2name;
  authtab.m = &testsuite_module;
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  ck_assert_msg(res == 0, "Failed to add '%s' AUTH symbol: %s", sym_name,
    strerror(errno));

  mark_point();

  name = pr_auth_gid2name(p, PR_TEST_AUTH_GID);
  ck_assert_msg(name != NULL, "Expected name, got null");
  ck_assert_msg(strcmp(name, PR_TEST_AUTH_NAME) == 0,
    "Expected name '%s', got '%s'", PR_TEST_AUTH_NAME, name);
  ck_assert_msg(gid2name_count == 1, "Expected call count 1, got %u",
    gid2name_count);

  pr_stash_remove_symbol(PR_SYM_AUTH, sym_name, &testsuite_module);
}
END_TEST

START_TEST (auth_getgroups_test) {
  int res;
  array_header *gids = NULL, *names = NULL;
  authtable authtab;
  char *sym_name = "getgroups";

  res = pr_auth_getgroups(NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL, got %d (%s)",
    errno, strerror(errno));

  res = pr_auth_getgroups(p, PR_TEST_AUTH_NAME, &gids, NULL);
  ck_assert_msg(res < 0, "Found groups for '%s' unexpectedly", PR_TEST_AUTH_NAME);
  ck_assert_msg(getgroups_count == 0, "Expected call count 0, got %u",
    getgroups_count);
  mark_point();
  
  /* Load the appropriate AUTH symbol, and call it. */

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = sym_name;
  authtab.handler = handle_getgroups;
  authtab.m = &testsuite_module;
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  ck_assert_msg(res == 0, "Failed to add '%s' AUTH symbol: %s", sym_name,
    strerror(errno));

  mark_point();

  res = pr_auth_getgroups(p, PR_TEST_AUTH_NAME, &gids, &names);
  ck_assert_msg(res > 0, "Expected group count 1 for '%s', got %d: %s",
    PR_TEST_AUTH_NAME, res, strerror(errno));
  ck_assert_msg(getgroups_count == 1, "Expected call count 1, got %u",
    getgroups_count);

  res = pr_auth_getgroups(p, "other", &gids, &names);
  ck_assert_msg(res < 0, "Found groups for 'other' unexpectedly");
  ck_assert_msg(getgroups_count == 2, "Expected call count 2, got %u",
    getgroups_count);

  pr_stash_remove_symbol(PR_SYM_AUTH, sym_name, &testsuite_module);
}
END_TEST

START_TEST (auth_cache_uid2name_test) {
  int res;
  const char *name; 
  authtable authtab;
  char *sym_name = "uid2name";

  /* Load the appropriate AUTH symbol, and call it. */

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = sym_name;
  authtab.handler = handle_uid2name;
  authtab.m = &testsuite_module;
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  ck_assert_msg(res == 0, "Failed to add '%s' AUTH symbol: %s", sym_name,
    strerror(errno));

  mark_point();

  name = pr_auth_uid2name(p, PR_TEST_AUTH_UID);
  ck_assert_msg(name != NULL, "Expected name, got null");
  ck_assert_msg(strcmp(name, PR_TEST_AUTH_NAME) == 0,
    "Expected name '%s', got '%s'", PR_TEST_AUTH_NAME, name);
  ck_assert_msg(uid2name_count == 1, "Expected call count 1, got %u",
    uid2name_count);

  /* Call again; the call counter should NOT increment due to caching. */

  name = pr_auth_uid2name(p, PR_TEST_AUTH_UID);
  ck_assert_msg(name != NULL, "Expected name, got null");
  ck_assert_msg(strcmp(name, PR_TEST_AUTH_NAME) == 0,
    "Expected name '%s', got '%s'", PR_TEST_AUTH_NAME, name);
  ck_assert_msg(uid2name_count == 1, "Expected call count 1, got %u",
    uid2name_count);

  pr_stash_remove_symbol(PR_SYM_AUTH, sym_name, &testsuite_module);
}
END_TEST

START_TEST (auth_cache_gid2name_test) {
  int res;
  const char *name; 
  authtable authtab;
  char *sym_name = "gid2name";

  /* Load the appropriate AUTH symbol, and call it. */

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = sym_name;
  authtab.handler = handle_gid2name;
  authtab.m = &testsuite_module;
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  ck_assert_msg(res == 0, "Failed to add '%s' AUTH symbol: %s", sym_name,
    strerror(errno));

  mark_point();

  name = pr_auth_gid2name(p, PR_TEST_AUTH_GID);
  ck_assert_msg(name != NULL, "Expected name, got null");
  ck_assert_msg(strcmp(name, PR_TEST_AUTH_NAME) == 0,
    "Expected name '%s', got '%s'", PR_TEST_AUTH_NAME, name);
  ck_assert_msg(gid2name_count == 1, "Expected call count 1, got %u",
    gid2name_count);

  /* Call again; the call counter should NOT increment due to caching. */

  name = pr_auth_gid2name(p, PR_TEST_AUTH_GID);
  ck_assert_msg(name != NULL, "Expected name, got null");
  ck_assert_msg(strcmp(name, PR_TEST_AUTH_NAME) == 0,
    "Expected name '%s', got '%s'", PR_TEST_AUTH_NAME, name);
  ck_assert_msg(gid2name_count == 1, "Expected call count 1, got %u",
    gid2name_count);

  pr_stash_remove_symbol(PR_SYM_AUTH, sym_name, &testsuite_module);
}
END_TEST

START_TEST (auth_cache_uid2name_failed_test) {
  int res;
  const char *name; 
  authtable authtab;
  char *sym_name = "uid2name";

  /* Load the appropriate AUTH symbol, and call it. */

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = sym_name;
  authtab.handler = decline_uid2name;
  authtab.m = &testsuite_module;
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  ck_assert_msg(res == 0, "Failed to add '%s' AUTH symbol: %s", sym_name,
    strerror(errno));

  mark_point();

  name = pr_auth_uid2name(p, PR_TEST_AUTH_UID);
  ck_assert_msg(name != NULL, "Expected name, got null");
  ck_assert_msg(strcmp(name, PR_TEST_AUTH_UID_STR) == 0,
    "Expected name '%s', got '%s'", PR_TEST_AUTH_UID_STR, name);
  ck_assert_msg(uid2name_count == 1, "Expected call count 1, got %u",
    uid2name_count);

  /* Call again; the call counter should NOT increment due to caching. */

  name = pr_auth_uid2name(p, PR_TEST_AUTH_UID);
  ck_assert_msg(name != NULL, "Expected name, got null");
  ck_assert_msg(strcmp(name, PR_TEST_AUTH_UID_STR) == 0,
    "Expected name '%s', got '%s'", PR_TEST_AUTH_UID_STR, name);
  ck_assert_msg(uid2name_count == 1, "Expected call count 1, got %u",
    uid2name_count);

  pr_stash_remove_symbol(PR_SYM_AUTH, sym_name, &testsuite_module);
}
END_TEST

START_TEST (auth_cache_gid2name_failed_test) {
  int res;
  const char *name; 
  authtable authtab;
  char *sym_name = "gid2name";

  /* Load the appropriate AUTH symbol, and call it. */

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = sym_name;
  authtab.handler = decline_gid2name;
  authtab.m = &testsuite_module;
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  ck_assert_msg(res == 0, "Failed to add '%s' AUTH symbol: %s", sym_name,
    strerror(errno));

  mark_point();

  name = pr_auth_gid2name(p, PR_TEST_AUTH_GID);
  ck_assert_msg(name != NULL, "Expected name, got null");
  ck_assert_msg(strcmp(name, PR_TEST_AUTH_GID_STR) == 0,
    "Expected name '%s', got '%s'", PR_TEST_AUTH_GID_STR, name);
  ck_assert_msg(gid2name_count == 1, "Expected call count 1, got %u",
    gid2name_count);

  /* Call again; the call counter should NOT increment due to caching. */

  name = pr_auth_gid2name(p, PR_TEST_AUTH_GID);
  ck_assert_msg(name != NULL, "Expected name, got null");
  ck_assert_msg(strcmp(name, PR_TEST_AUTH_GID_STR) == 0,
    "Expected name '%s', got '%s'", PR_TEST_AUTH_GID_STR, name);
  ck_assert_msg(gid2name_count == 1, "Expected call count 1, got %u",
    gid2name_count);

  pr_stash_remove_symbol(PR_SYM_AUTH, sym_name, &testsuite_module);
}
END_TEST

START_TEST (auth_cache_name2uid_failed_test) {
  int res;
  uid_t uid;
  authtable authtab;
  char *sym_name = "name2uid";

  /* Load the appropriate AUTH symbol, and call it. */

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = sym_name;
  authtab.handler = decline_name2uid;
  authtab.m = &testsuite_module;
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  ck_assert_msg(res == 0, "Failed to add '%s' AUTH symbol: %s", sym_name,
    strerror(errno));

  mark_point();

  uid = pr_auth_name2uid(p, PR_TEST_AUTH_NAME);
  ck_assert_msg(uid == (uid_t) -1, "Expected -1, got %lu", (unsigned long) uid);
  ck_assert_msg(name2uid_count == 1, "Expected call count 1, got %u",
    name2uid_count);

  /* Call again; the call counter should NOT increment due to caching. */

  uid = pr_auth_name2uid(p, PR_TEST_AUTH_NAME);
  ck_assert_msg(uid == (uid_t) -1, "Expected -1, got %lu", (unsigned long) uid);
  ck_assert_msg(name2uid_count == 1, "Expected call count 1, got %u",
    name2uid_count);

  pr_stash_remove_symbol(PR_SYM_AUTH, sym_name, &testsuite_module);
}
END_TEST

START_TEST (auth_cache_name2gid_failed_test) {
  int res;
  gid_t gid;
  authtable authtab;
  char *sym_name = "name2gid";

  /* Load the appropriate AUTH symbol, and call it. */

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = sym_name;
  authtab.handler = decline_name2gid;
  authtab.m = &testsuite_module;
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  ck_assert_msg(res == 0, "Failed to add '%s' AUTH symbol: %s", sym_name,
    strerror(errno));

  mark_point();

  gid = pr_auth_name2gid(p, PR_TEST_AUTH_NAME);
  ck_assert_msg(gid == (gid_t) -1, "Expected -1, got %lu", (unsigned long) gid);
  ck_assert_msg(name2gid_count == 1, "Expected call count 1, got %u",
    name2gid_count);

  /* Call again; the call counter should NOT increment due to caching. */

  gid = pr_auth_name2gid(p, PR_TEST_AUTH_NAME);
  ck_assert_msg(gid == (gid_t) -1, "Expected -1, got %lu", (unsigned long) gid);
  ck_assert_msg(name2gid_count == 1, "Expected call count 1, got %u",
    name2gid_count);

  pr_stash_remove_symbol(PR_SYM_AUTH, sym_name, &testsuite_module);
}
END_TEST

START_TEST (auth_cache_clear_test) {
  int res;
  gid_t gid;
  authtable authtab;
  char *sym_name = "name2gid";

  mark_point();
  pr_auth_cache_clear();

  /* Load the appropriate AUTH symbol, and call it. */

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = sym_name;
  authtab.handler = decline_name2gid;
  authtab.m = &testsuite_module;
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  ck_assert_msg(res == 0, "Failed to add '%s' AUTH symbol: %s", sym_name,
    strerror(errno));

  mark_point();
  gid = pr_auth_name2gid(p, PR_TEST_AUTH_NAME);
  ck_assert_msg(gid == (gid_t) -1, "Expected -1, got %lu", (unsigned long) gid);
  ck_assert_msg(name2gid_count == 1, "Expected call count 1, got %u",
    name2gid_count);

  mark_point();
  pr_auth_cache_clear();
}
END_TEST

START_TEST (auth_cache_set_test) {
  int res;
  unsigned int flags = PR_AUTH_CACHE_FL_UID2NAME|PR_AUTH_CACHE_FL_GID2NAME|PR_AUTH_CACHE_FL_AUTH_MODULE|PR_AUTH_CACHE_FL_NAME2UID|PR_AUTH_CACHE_FL_NAME2GID|PR_AUTH_CACHE_FL_BAD_UID2NAME|PR_AUTH_CACHE_FL_BAD_GID2NAME|PR_AUTH_CACHE_FL_BAD_NAME2UID|PR_AUTH_CACHE_FL_BAD_NAME2GID;

  res = pr_auth_cache_set(-1, 0);
  ck_assert_msg(res < 0, "Failed to handle invalid setting");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  res = pr_auth_cache_set(TRUE, flags);
  ck_assert_msg(res == 0, "Failed to enable all auth cache settings: %s",
    strerror(errno));

  res = pr_auth_cache_set(FALSE, flags);
  ck_assert_msg(res == 0, "Failed to disable all auth cache settings: %s",
    strerror(errno));

  (void) pr_auth_cache_set(TRUE, PR_AUTH_CACHE_FL_DEFAULT);
}
END_TEST

START_TEST (auth_clear_auth_only_module_test) {
  int res;

  (void) pr_auth_cache_set(TRUE, PR_AUTH_CACHE_FL_AUTH_MODULE);

  res = pr_auth_clear_auth_only_modules();
  ck_assert_msg(res < 0, "Failed to handle no auth module list");
  ck_assert_msg(errno == EPERM, "Expected EPERM (%d), got %s (%d)", EPERM,
    strerror(errno), errno);
}
END_TEST

START_TEST (auth_add_auth_only_module_test) {
  int res;
  const char *name = "foo.bar";

  (void) pr_auth_cache_set(TRUE, PR_AUTH_CACHE_FL_AUTH_MODULE);

  res = pr_auth_add_auth_only_module(NULL);
  ck_assert_msg(res < 0, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  res = pr_auth_add_auth_only_module(name);
  ck_assert_msg(res == 0, "Failed to add auth-only module '%s': %s", name,
    strerror(errno));

  res = pr_auth_add_auth_only_module(name);
  ck_assert_msg(res < 0, "Failed to handle duplicate auth-only module");
  ck_assert_msg(errno == EEXIST, "Expected EEXIST (%d), got %s (%d)", EEXIST,
    strerror(errno), errno);

  res = pr_auth_clear_auth_only_modules();
  ck_assert_msg(res == 0, "Failed to clear auth-only modules: %s",
    strerror(errno));
}
END_TEST

START_TEST (auth_remove_auth_only_module_test) {
  int res;
  const char *name = "foo.bar";

  (void) pr_auth_cache_set(TRUE, PR_AUTH_CACHE_FL_AUTH_MODULE);

  res = pr_auth_remove_auth_only_module(NULL);
  ck_assert_msg(res < 0, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  res = pr_auth_remove_auth_only_module(name);
  ck_assert_msg(res < 0, "Failed to handle empty auth-only module list");
  ck_assert_msg(errno == EPERM, "Expected EPERM (%d), got %s (%d)", EPERM,
    strerror(errno), errno);

  res = pr_auth_add_auth_only_module(name);
  ck_assert_msg(res == 0, "Failed to add auth-only module '%s': %s", name,
    strerror(errno));

  res = pr_auth_remove_auth_only_module(name);
  ck_assert_msg(res == 0, "Failed to remove auth-only module '%s': %s", name,
    strerror(errno));

  (void) pr_auth_clear_auth_only_modules();
}
END_TEST

START_TEST (auth_authenticate_test) {
  int res;
  authtable authtab;
  char *sym_name = "auth";

  res = pr_auth_authenticate(NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  res = pr_auth_authenticate(p, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null name");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  res = pr_auth_authenticate(p, PR_TEST_AUTH_NAME, NULL);
  ck_assert_msg(res < 0, "Failed to handle null password");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  /* Load the appropriate AUTH symbol, and call it. */

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = sym_name;
  authtab.handler = handle_authn;
  authtab.m = &testsuite_module;
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  ck_assert_msg(res == 0, "Failed to add '%s' AUTH symbol: %s", sym_name,
    strerror(errno));

  res = pr_auth_authenticate(p, "other", "foobar");
  ck_assert_msg(res == PR_AUTH_NOPWD,
    "Authenticated user 'other' unexpectedly (expected %d, got %d)",
    PR_AUTH_NOPWD, res);

  res = pr_auth_authenticate(p, PR_TEST_AUTH_NAME, "foobar");
  ck_assert_msg(res == PR_AUTH_BADPWD,
    "Authenticated user '%s' unexpectedly (expected %d, got %d)",
    PR_TEST_AUTH_NAME, PR_AUTH_BADPWD, res);

  res = pr_auth_authenticate(p, PR_TEST_AUTH_NAME, PR_TEST_AUTH_PASSWD);
  ck_assert_msg(res == PR_AUTH_OK,
    "Failed to authenticate user '%s' (expected %d, got %d)",
    PR_TEST_AUTH_NAME, PR_AUTH_OK, res);

  authtab.auth_flags |= PR_AUTH_FL_REQUIRED;
  res = pr_auth_authenticate(p, PR_TEST_AUTH_NAME, PR_TEST_AUTH_PASSWD);
  ck_assert_msg(res == PR_AUTH_OK,
    "Failed to authenticate user '%s' (expected %d, got %d)",
    PR_TEST_AUTH_NAME, PR_AUTH_OK, res);
  authtab.auth_flags &= ~PR_AUTH_FL_REQUIRED;

  (void) pr_auth_cache_set(TRUE, PR_AUTH_CACHE_FL_AUTH_MODULE);

  res = pr_auth_add_auth_only_module("foo.bar");
  ck_assert_msg(res == 0, "Failed to add auth-only module: %s", strerror(errno));

  res = pr_auth_add_auth_only_module("mod_testsuite.c");
  ck_assert_msg(res == 0, "Failed to add auth-only module: %s", strerror(errno));

  res = pr_module_load(&testsuite_module);
  ck_assert_msg(res == 0, "Failed to load module: %s", strerror(errno));

  res = pr_auth_authenticate(p, "foo", "bar");
  ck_assert_msg(res == PR_AUTH_NOPWD,
    "Failed to handle unknown user 'foo' (expected %d, got %d)", PR_AUTH_NOPWD,
    res);

  res = pr_auth_authenticate(p, PR_TEST_AUTH_NAME, "bar");
  ck_assert_msg(res == PR_AUTH_BADPWD,
    "Failed to handle user '%s' with bad password (expected %d, got %d)",
    PR_TEST_AUTH_NAME, PR_AUTH_BADPWD, res);

  res = pr_auth_authenticate(p, PR_TEST_AUTH_NAME, PR_TEST_AUTH_PASSWD);
  ck_assert_msg(res == PR_AUTH_OK,
    "Failed to authenticate user '%s' (expected %d, got %d)",
    PR_TEST_AUTH_NAME, PR_AUTH_OK, res);

  authn_rfc2228 = TRUE;
  res = pr_auth_authenticate(p, PR_TEST_AUTH_NAME, PR_TEST_AUTH_PASSWD);
  ck_assert_msg(res == PR_AUTH_RFC2228_OK,
    "Failed to authenticate user '%s' (expected %d, got %d)",
    PR_TEST_AUTH_NAME, PR_AUTH_RFC2228_OK, res);

  pr_auth_clear_auth_only_modules();
  pr_module_unload(&testsuite_module);

  authn_rfc2228 = TRUE;
  res = pr_auth_authenticate(p, PR_TEST_AUTH_NAME, PR_TEST_AUTH_PASSWD);
  ck_assert_msg(res == PR_AUTH_RFC2228_OK,
    "Failed to authenticate user '%s' (expected %d, got %d)",
    PR_TEST_AUTH_NAME, PR_AUTH_RFC2228_OK, res);

  authn_rfc2228 = FALSE;
}
END_TEST

START_TEST (auth_authorize_test) {
  int res;
  authtable authtab;
  char *sym_name = "authorize";

  res = pr_auth_authorize(NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  res = pr_auth_authorize(p, NULL);
  ck_assert_msg(res < 0, "Failed to handle null name");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  res = pr_auth_authorize(p, PR_TEST_AUTH_NAME);
  ck_assert_msg(res > 0, "Failed to handle missing handler");

  /* Load the appropriate AUTH symbol, and call it. */

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = sym_name;
  authtab.handler = handle_authz;
  authtab.m = &testsuite_module;
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  ck_assert_msg(res == 0, "Failed to add '%s' AUTH symbol: %s", sym_name,
    strerror(errno));

  res = pr_auth_authorize(p, "other");
  ck_assert_msg(res == PR_AUTH_NOPWD,
    "Authorized user 'other' unexpectedly (expected %d, got %d)",
    PR_AUTH_NOPWD, res);

  res = pr_auth_authorize(p, PR_TEST_AUTH_NAME);
  ck_assert_msg(res == PR_AUTH_OK,
    "Failed to authorize user '%s' (expected %d, got %d)",
    PR_TEST_AUTH_NAME, PR_AUTH_OK, res);

  (void) pr_auth_cache_set(TRUE, PR_AUTH_CACHE_FL_AUTH_MODULE);

  res = pr_auth_add_auth_only_module("foo.bar");
  ck_assert_msg(res == 0, "Failed to add auth-only module: %s", strerror(errno));

  res = pr_auth_add_auth_only_module(testsuite_module.name);
  ck_assert_msg(res == 0, "Failed to add auth-only module: %s", strerror(errno));

  res = pr_auth_authorize(p, PR_TEST_AUTH_NAME);
  ck_assert_msg(res == PR_AUTH_OK,
    "Failed to authorize user '%s' (expected %d, got %d)",
    PR_TEST_AUTH_NAME, PR_AUTH_OK, res);

  (void) pr_auth_clear_auth_only_modules();
}
END_TEST

static int test_check_errorcode = PR_AUTH_BADPWD;

MODRET handle_check_error(cmd_rec *cmd) {
  return PR_ERROR_INT(cmd, test_check_errorcode);
}

START_TEST (auth_check_errors_test) {
  register unsigned int i;
  int res;
  const char *cleartext_passwd, *name;
  authtable authtab;
  char *sym_name = "check";
  int test_errorcodes[] = {
    PR_AUTH_ERROR,
    PR_AUTH_NOPWD,
    PR_AUTH_BADPWD,
    PR_AUTH_AGEPWD,
    PR_AUTH_DISABLEDPWD,
    PR_AUTH_CRED_INSUFFICIENT,
    PR_AUTH_CRED_UNAVAIL,
    PR_AUTH_CRED_ERROR,
    PR_AUTH_INFO_UNAVAIL,
    PR_AUTH_MAX_ATTEMPTS_EXCEEDED,
    PR_AUTH_INIT_ERROR,
    PR_AUTH_NEW_TOKEN_REQUIRED,
    0
  };

  mark_point();
  res = pr_auth_check(NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_auth_check(p, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null name");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  name = PR_TEST_AUTH_NAME;
  res = pr_auth_check(p, NULL, name, NULL);
  ck_assert_msg(res < 0, "Failed to handle null cleartext password");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  cleartext_passwd = PR_TEST_AUTH_PASSWD;
  res = pr_auth_check(p, NULL, name, cleartext_passwd);
  ck_assert_msg(res == PR_AUTH_BADPWD, "Expected %d, got %d", PR_AUTH_BADPWD,
    res);

  res = pr_module_load(&testsuite_module);
  ck_assert_msg(res == 0, "Failed to load module: %s", strerror(errno));

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = sym_name;
  authtab.handler = handle_check_error;
  authtab.m = &testsuite_module;
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  ck_assert_msg(res == 0, "Failed to add '%s' AUTH symbol: %s", sym_name,
    strerror(errno));

  (void) pr_auth_cache_set(TRUE, PR_AUTH_CACHE_FL_AUTH_MODULE);
  res = pr_auth_add_auth_only_module("mod_testsuite.c");
  ck_assert_msg(res == 0, "Failed to add auth-only module: %s", strerror(errno));

  for (i = 0; test_errorcodes[i] != 0; i++) {
    mark_point();
    test_check_errorcode = test_errorcodes[i];
    res = pr_auth_check(p, "", name, cleartext_passwd);
    ck_assert_msg(res == test_check_errorcode, "Expected %d, got %d",
      test_check_errorcode, res);
  }

  mark_point();
  test_check_errorcode = PR_AUTH_OK_NO_PASS;
  res = pr_auth_check(p, "", name, cleartext_passwd);
  ck_assert_msg(res == test_check_errorcode, "Expected %d, got %d",
    test_check_errorcode, res);

  res = pr_module_unload(&testsuite_module);
  ck_assert_msg(res == 0, "Failed to unload module: %s", strerror(errno));
  (void) pr_auth_clear_auth_only_modules();
  pr_stash_remove_symbol(PR_SYM_AUTH, sym_name, &testsuite_module);
}
END_TEST

START_TEST (auth_check_valid_test) {
  int res;
  const char *cleartext_passwd, *ciphertext_passwd, *name;
  authtable authtab;
  char *sym_name = "check";

  /* Load the appropriate AUTH symbol, and call it. */

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = sym_name;
  authtab.handler = handle_check;
  authtab.m = &testsuite_module;
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  ck_assert_msg(res == 0, "Failed to add '%s' AUTH symbol: %s", sym_name,
    strerror(errno));

  mark_point();
  cleartext_passwd = PR_TEST_AUTH_PASSWD;
  res = pr_auth_check(p, NULL, "other", cleartext_passwd);
  ck_assert_msg(res == PR_AUTH_BADPWD, "Expected %d, got %d", PR_AUTH_BADPWD,
    res);

  mark_point();
  name = PR_TEST_AUTH_NAME;
  res = pr_auth_check(p, "foo", name, cleartext_passwd);
  ck_assert_msg(res == PR_AUTH_BADPWD, "Expected %d, got %d", PR_AUTH_BADPWD,
    res);

  mark_point();
  res = pr_auth_check(p, NULL, name, cleartext_passwd);
  ck_assert_msg(res == PR_AUTH_BADPWD, "Expected %d, got %d", PR_AUTH_BADPWD,
    res);

  mark_point();
  ciphertext_passwd = PR_TEST_AUTH_PASSWD;
  res = pr_auth_check(p, ciphertext_passwd, name, cleartext_passwd);
  ck_assert_msg(res == PR_AUTH_OK, "Expected %d, got %d", PR_AUTH_OK, res);

  (void) pr_auth_cache_set(TRUE, PR_AUTH_CACHE_FL_AUTH_MODULE);

  res = pr_auth_add_auth_only_module("foo.bar");
  ck_assert_msg(res == 0, "Failed to add auth-only module: %s", strerror(errno));

  res = pr_auth_add_auth_only_module("mod_testsuite.c");
  ck_assert_msg(res == 0, "Failed to add auth-only module: %s", strerror(errno));

  mark_point();
  check_rfc2228 = TRUE;
  res = pr_auth_check(p, ciphertext_passwd, name, cleartext_passwd);
  ck_assert_msg(res == PR_AUTH_RFC2228_OK,
    "Failed to check user '%s' (expected %d, got %d)", name,
    PR_AUTH_RFC2228_OK, res);

  (void) pr_auth_clear_auth_only_modules();
  pr_stash_remove_symbol(PR_SYM_AUTH, sym_name, &testsuite_module);
}
END_TEST

START_TEST (auth_requires_pass_test) {
  int res;
  const char *name;
  authtable authtab;
  char *sym_name = "requires_pass";

  res = pr_auth_requires_pass(NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  res = pr_auth_requires_pass(p, NULL);
  ck_assert_msg(res < 0, "Failed to handle null name");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  name = "other";
  res = pr_auth_requires_pass(p, name);
  ck_assert_msg(res == TRUE, "Unknown users should require passwords (got %d)",
    res);

  /* Load the appropriate AUTH symbol, and call it. */

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = sym_name;
  authtab.handler = handle_requires_pass;
  authtab.m = &testsuite_module;
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  ck_assert_msg(res == 0, "Failed to add '%s' AUTH symbol: %s", sym_name,
    strerror(errno));

  res = pr_auth_requires_pass(p, name);
  ck_assert_msg(res == TRUE, "Unknown users should require passwords (got %d)",
    res);

  name = PR_TEST_AUTH_NAME;
  res = pr_auth_requires_pass(p, name);
  ck_assert_msg(res == FALSE, "Known users should NOT require passwords (got %d)",
    res);
}
END_TEST

START_TEST (auth_get_anon_config_test) {
  config_rec *anon_config, *c, *c2, *res;
  const char *login_user = "test";
  char *real_user = NULL, *anon_user = NULL;

  mark_point();
  login_user = "test";
  res = pr_auth_get_anon_config(NULL, NULL, NULL, NULL);
  ck_assert_msg(res == NULL, "Failed to handle null arguments");

  mark_point();
  /* UserAlias alias realname */
  c = add_config_param_set(&(test_server->conf), "UserAlias", 2, NULL, NULL);
  c->argv[0] = pstrdup(c->pool, "bar");
  c->argv[1] = pstrdup(c->pool, "foo");

  c2 = add_config_param_set(&(test_server->conf), "AuthAliasOnly", 1, NULL);
  c2->argv[0] = palloc(c2->pool, sizeof(unsigned char));
  *((unsigned char *) c2->argv[0]) = TRUE;

  login_user = "test";
  anon_user = "anon";
  res = pr_auth_get_anon_config(p, &login_user, &real_user, &anon_user);
  ck_assert_msg(res == NULL, "Failed to handle UserAlias with mismatched alias");
  ck_assert_msg(login_user == NULL, "Failed to set login_user to null");
  ck_assert_msg(anon_user == NULL, "Failed to set anon_user to null");

  mark_point();
  login_user = "test";
  c->argv[0] = pstrdup(c->pool, "*");
  res = pr_auth_get_anon_config(p, &login_user, &real_user, &anon_user);
  ck_assert_msg(res == NULL, "Failed to handle UserAlias with globbed alias");

  mark_point();
  login_user = "test";
  c->argv[0] = pstrdup(c->pool, login_user);
  res = pr_auth_get_anon_config(p, &login_user, &real_user, &anon_user);
  ck_assert_msg(res == NULL, "Failed to handle UserAlias with matching alias");

  mark_point();
  login_user = "test";
  anon_config = pcalloc(p, sizeof(config_rec));
  anon_config->config_type = CONF_ANON;
  c2 = add_config_param_set(&(anon_config->subset), "AuthAliasOnly", 1, NULL);
  c2->argv[0] = palloc(c2->pool, sizeof(unsigned char));
  *((unsigned char *) c2->argv[0]) = TRUE;
  c->parent = anon_config;
  res = pr_auth_get_anon_config(p, &login_user, &real_user, &anon_user);
  ck_assert_msg(res != NULL, "Failed to handle UserAlias with matching alias and <Anonymous> config");
  ck_assert_msg(res == anon_config, "Expected <Anonymous> config %p, got %p",
    anon_config, res);

  mark_point();
  login_user = "test";
  real_user = "foo";

  c2 = add_config_param_set(&(anon_config->subset), "UserName", 1, NULL);
  c2->argv[0] = pstrdup(c2->pool, "BAZ");
  res = pr_auth_get_anon_config(p, &login_user, &real_user, &anon_user);

  ck_assert_msg(res != NULL, "Failed to handle UserAlias with matching alias and <Anonymous> config");
  ck_assert_msg(res == anon_config, "Expected <Anonymous> config %p, got %p",
    anon_config, res);
  ck_assert_msg(real_user != NULL, "Expected real_user, got NULL");
  ck_assert_msg(strcmp(real_user, "BAZ") == 0, "Expected real_user 'BAZ', got '%s'", real_user);

  (void) remove_config(test_server->conf, "AuthAliasOnly", TRUE);
  (void) remove_config(test_server->conf, "UserAlias", FALSE);
}
END_TEST

START_TEST (auth_chroot_test) {
  int res;
  const char *path;

  mark_point();
  res = pr_auth_chroot(NULL);
  ck_assert_msg(res < 0, "Failed to handle null argument");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  path = "tmp";
  res = pr_auth_chroot(path);
  ck_assert_msg(res < 0, "Failed to chroot to '%s': %s", path, strerror(errno));
  ck_assert_msg(errno == EINVAL || errno == ENOENT,
    "Expected EINVAL (%d) or ENOENT (%d), got %s (%d)", EINVAL, ENOENT,
    strerror(errno), errno);

  /* This first time, we do not have session.pool set, so the setting of the
   * TZ environment variable will fail.
   */
  mark_point();
  path = "/tmp";
  res = pr_auth_chroot(path);
  ck_assert_msg(res < 0, "Failed to chroot to '%s': %s", path, strerror(errno));
  ck_assert_msg(errno == ENOENT || errno == EPERM || errno == EINVAL,
    "Expected ENOENT (%d), EPERM (%d) or EINVAL (%d), got %s (%d)",
    ENOENT, EPERM, EINVAL, strerror(errno), errno);

  /* We should now set the TZ environment variable, but still fail. */
  mark_point();
  session.pool = p;
  path = "/tmp";
  res = pr_auth_chroot(path);
  ck_assert_msg(res < 0, "Failed to chroot to '%s': %s", path, strerror(errno));
  ck_assert_msg(errno == ENOENT || errno == EPERM || errno == EINVAL,
    "Expected ENOENT (%d), EPERM (%d) or EINVAL (%d), got %s (%d)",
    ENOENT, EPERM, EINVAL, strerror(errno), errno);

  /* Last, the TZ environment variable should already be set. */
  mark_point();
  path = "/tmp";
  res = pr_auth_chroot(path);
  ck_assert_msg(res < 0, "Failed to chroot to '%s': %s", path, strerror(errno));
  ck_assert_msg(errno == ENOENT || errno == EPERM || errno == EINVAL,
    "Expected ENOENT (%d), EPERM (%d) or EINVAL (%d), got %s (%d)",
    ENOENT, EPERM, EINVAL, strerror(errno), errno);

  session.pool = NULL;
}
END_TEST

START_TEST (auth_banned_by_ftpusers_test) {
  const char *name;
  int res;
  config_rec *c;

  mark_point();
  res = pr_auth_banned_by_ftpusers(NULL, NULL);
  ck_assert_msg(res == FALSE, "Failed to handle null arguments");

  mark_point();
  res = pr_auth_banned_by_ftpusers(test_server->conf, NULL);
  ck_assert_msg(res == FALSE, "Failed to handle null user");

  mark_point();
  name = "testsuite";
  res = pr_auth_banned_by_ftpusers(test_server->conf, name);
  ck_assert_msg(res == FALSE, "Expected FALSE, got %d", res);

  /* UseFtpUsers off */
  mark_point();
  c = add_config_param_set(&(test_server->conf), "UseFtpUsers", 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = FALSE;

  res = pr_auth_banned_by_ftpusers(test_server->conf, name);
  ck_assert_msg(res == FALSE, "Failed to handle UseFtpUsers off (got %d)",
    res);

  (void) remove_config(test_server->conf, "UseFtpUsers", FALSE);
}
END_TEST

START_TEST (auth_is_valid_shell_test) {
  const char *shell;
  int res;
  config_rec *c;

  mark_point();
  res = pr_auth_is_valid_shell(NULL, NULL);
  ck_assert_msg(res == TRUE, "Failed to handle null arguments");

  mark_point();
  res = pr_auth_is_valid_shell(test_server->conf, NULL);
  ck_assert_msg(res == TRUE, "Failed to handle null shell");

  shell = "/foo/bar";
  res = pr_auth_is_valid_shell(test_server->conf, shell);
  ck_assert_msg(res == FALSE, "Failed to handle invalid shell '%s' (got %d)",
    shell, res);

  mark_point();
  shell = "/bin/sh";
  res = pr_auth_is_valid_shell(test_server->conf, shell);
  ck_assert_msg(res == TRUE, "Failed to handle valid shell '%s' (got %d)",
    shell, res);

  /* RequireValidShell off */
  mark_point();
  c = add_config_param_set(&(test_server->conf), "RequireValidShell", 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = FALSE;

  shell = "/foo/bar";
  res = pr_auth_is_valid_shell(test_server->conf, shell);
  ck_assert_msg(res == TRUE, "Failed to handle RequireValidShell off (got %d)",
    res);

  (void) remove_config(test_server->conf, "RequireValidShell", FALSE);
}
END_TEST

START_TEST (auth_set_groups_test) {
  int res;

  if (getuid() == PR_ROOT_UID) {
    gid_t gid;

    gid = getgid();

    mark_point();
    res = set_groups(NULL, 0, NULL);
    ck_assert_msg(res == 0, "Failed to handle zero primary gid: %s",
      strerror(errno));

    mark_point();
    res = set_groups(p, 0, NULL);
    ck_assert_msg(res == 0, "Failed to handle zero primary gid: %s",
      strerror(errno));

    mark_point();
    res = set_groups(NULL, gid, NULL);
    ck_assert_msg(res == 0, "Failed to handle current primary gid: %s",
      strerror(errno));

  } else {
    mark_point();
    res = set_groups(NULL, 0, NULL);
    ck_assert_msg(res < 0, "Failed to handle zero primary gid: %s",
      strerror(errno));
    ck_assert_msg(errno == ENOSYS, "Expected ENOSYS (%d), got %s (%d)", ENOSYS,
      strerror(errno), errno);

    mark_point();
    res = set_groups(p, 0, NULL);
    ck_assert_msg(res < 0, "Failed to handle zero primary gid: %s",
      strerror(errno));
    ck_assert_msg(errno == ENOSYS, "Expected ENOSYS (%d), got %s (%d)", ENOSYS,
      strerror(errno), errno);

    mark_point();
    res = set_groups(p, 1, NULL);
    ck_assert_msg(res < 0, "Failed to handle non-root primary gid: %s",
      strerror(errno));
    ck_assert_msg(errno == ENOSYS, "Expected ENOSYS (%d), got %s (%d)", ENOSYS,
      strerror(errno), errno);

    mark_point();
    res = set_groups(p, getgid(), NULL);
    ck_assert_msg(res < 0, "Failed to handle current primary gid: %s",
      strerror(errno));
    ck_assert_msg(errno == ENOSYS, "Expected ENOSYS (%d), got %s (%d)", ENOSYS,
      strerror(errno), errno);
  }
}
END_TEST

START_TEST (auth_get_home_test) {
  const char *home, *res;
  config_rec *c;

  res = pr_auth_get_home(NULL, NULL);
  ck_assert_msg(res == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  res = pr_auth_get_home(p, NULL);
  ck_assert_msg(res == NULL, "Failed to handle null home");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  home = "/testsuite";
  res = pr_auth_get_home(p, home);
  ck_assert_msg(res != NULL, "Failed to get home: %s", strerror(errno));
  ck_assert_msg(strcmp(home, res) == 0, "Expected '%s', got '%s'", home, res);  

  /* RewriteHome off */
  mark_point();
  c = add_config_param_set(&(test_server->conf), "RewriteHome", 1);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((int *) c->argv[0]) = FALSE;

  res = pr_auth_get_home(p, home);
  ck_assert_msg(res != NULL, "Failed to get home: %s", strerror(errno));
  ck_assert_msg(strcmp(home, res) == 0,
    "Failed to handle RewriteHome off, got '%s'", res);

  /* RewriteHome on */
  mark_point();
  *((int *) c->argv[0]) = TRUE;
  res = pr_auth_get_home(p, home);
  ck_assert_msg(res != NULL, "Failed to get home: %s", strerror(errno));
  ck_assert_msg(strcmp(home, res) == 0,
    "Failed to handle RewriteHome on, got '%s'", res);

  mark_point();
  session.notes = pr_table_alloc(p, 2);
  res = pr_auth_get_home(p, home);
  ck_assert_msg(res != NULL, "Failed to get home: %s", strerror(errno));
  ck_assert_msg(strcmp(home, res) == 0,
    "Failed to handle RewriteHome on, got '%s'", res);

  (void) pr_table_empty(session.notes);
  (void) pr_table_free(session.notes);
  session.notes = NULL;

  (void) remove_config(test_server->conf, "RewriteHome", FALSE);
}
END_TEST

START_TEST (auth_set_max_password_len_test) {
  int checked;
  size_t res;

  res = pr_auth_set_max_password_len(p, 1);
  ck_assert_msg(res == PR_TUNABLE_PASSWORD_MAX,
    "Expected %lu, got %lu", (unsigned long) PR_TUNABLE_PASSWORD_MAX,
    (unsigned long) res);

  checked = pr_auth_check(p, NULL, PR_TEST_AUTH_NAME, PR_TEST_AUTH_PASSWD);
  ck_assert_msg(checked < 0, "Failed to reject too-long password");
  ck_assert_msg(errno == EPERM, "Expected EPERM (%d), got %s (%d)", EPERM,
    strerror(errno), errno);

  res = pr_auth_set_max_password_len(p, 0);
  ck_assert_msg(res == 1, "Expected %d, got %lu", 1, (unsigned long) res);

  res = pr_auth_set_max_password_len(p, 0);
  ck_assert_msg(res == PR_TUNABLE_PASSWORD_MAX,
    "Expected %lu, got %lu", (unsigned long) PR_TUNABLE_PASSWORD_MAX,
    (unsigned long) res);
}
END_TEST

START_TEST (auth_bcrypt_test) {
  char *res;
  size_t hashed_len;

  res = pr_auth_bcrypt(NULL, NULL, NULL, NULL);
  ck_assert_msg(res == NULL, "Failed to handle null pool argument");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got '%s' (%d)", EINVAL,
    strerror(errno), errno);

  res = pr_auth_bcrypt(p, NULL, NULL, NULL);
  ck_assert_msg(res == NULL, "Failed to handle null key argument");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got '%s' (%d)", EINVAL,
    strerror(errno), errno);

  res = pr_auth_bcrypt(p, "", NULL, NULL);
  ck_assert_msg(res == NULL, "Failed to handle null salt argument");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got '%s' (%d)", EINVAL,
    strerror(errno), errno);

  res = pr_auth_bcrypt(p, "", "", NULL);
  ck_assert_msg(res == NULL, "Failed to handle null hashed_len argument");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got '%s' (%d)", EINVAL,
    strerror(errno), errno);

  res = pr_auth_bcrypt(p, "", "", &hashed_len);
  ck_assert_msg(res == NULL, "Failed to handle empty strings");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got '%s' (%d)", EINVAL,
    strerror(errno), errno);

  res = pr_auth_bcrypt(p, "foo", "$1", &hashed_len);
  ck_assert_msg(res == NULL, "Failed to handle invalid salt");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got '%s' (%d)", EINVAL,
    strerror(errno), errno);

  /* TODO: Add more tests of the invalid salt constructions: bcrypt version
   * numbers, rounds, salt too short, etc.
   */

  res = pr_auth_bcrypt(p, "password",
    "$2b$12$IoFxXvbRQUKssPqFacJFFuZl1KXl5ULppqf0aLFjwCFnLRh3NbYSG",
    &hashed_len);
  ck_assert_msg(res != NULL, "Failed to handle valid key and salt");
}
END_TEST

Suite *tests_get_auth_suite(void) {
  Suite *suite;
  TCase *testcase;

  suite = suite_create("auth");

  testcase = tcase_create("base");
  tcase_add_checked_fixture(testcase, set_up, tear_down);

  /* pwent* et al */
  tcase_add_test(testcase, auth_setpwent_test);
  tcase_add_test(testcase, auth_endpwent_test);
  tcase_add_test(testcase, auth_getpwent_test);
  tcase_add_test(testcase, auth_getpwnam_test);
  tcase_add_test(testcase, auth_getpwuid_test);
  tcase_add_test(testcase, auth_name2uid_test);
  tcase_add_test(testcase, auth_uid2name_test);

  /* grent* et al */
  tcase_add_test(testcase, auth_setgrent_test);
  tcase_add_test(testcase, auth_endgrent_test);
  tcase_add_test(testcase, auth_getgrent_test);
  tcase_add_test(testcase, auth_getgrnam_test);
  tcase_add_test(testcase, auth_getgrgid_test);
  tcase_add_test(testcase, auth_gid2name_test);
  tcase_add_test(testcase, auth_name2gid_test);
  tcase_add_test(testcase, auth_getgroups_test);

  /* Caching tests */
  tcase_add_test(testcase, auth_cache_uid2name_test);
  tcase_add_test(testcase, auth_cache_gid2name_test);
  tcase_add_test(testcase, auth_cache_uid2name_failed_test);
  tcase_add_test(testcase, auth_cache_gid2name_failed_test);
  tcase_add_test(testcase, auth_cache_name2uid_failed_test);
  tcase_add_test(testcase, auth_cache_name2gid_failed_test);
  tcase_add_test(testcase, auth_cache_clear_test);
  tcase_add_test(testcase, auth_cache_set_test);

  /* Auth modules */
  tcase_add_test(testcase, auth_clear_auth_only_module_test);
  tcase_add_test(testcase, auth_add_auth_only_module_test);
  tcase_add_test(testcase, auth_remove_auth_only_module_test);

  /* Authorization */
  tcase_add_test(testcase, auth_authenticate_test);
  tcase_add_test(testcase, auth_authorize_test);
  tcase_add_test(testcase, auth_check_errors_test);
  tcase_add_test(testcase, auth_check_valid_test);
  tcase_add_test(testcase, auth_requires_pass_test);

  /* Misc */
  tcase_add_test(testcase, auth_get_anon_config_test);
  tcase_add_test(testcase, auth_chroot_test);
  tcase_add_test(testcase, auth_banned_by_ftpusers_test);
  tcase_add_test(testcase, auth_is_valid_shell_test);
  tcase_add_test(testcase, auth_set_groups_test);
  tcase_add_test(testcase, auth_get_home_test);
  tcase_add_test(testcase, auth_set_max_password_len_test);
  tcase_add_test(testcase, auth_bcrypt_test);

  suite_add_tcase(suite, testcase);
  return suite;
}
