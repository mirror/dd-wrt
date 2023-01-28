/*
 * ProFTPD - FTP server API testsuite
 * Copyright (c) 2011-2022 The ProFTPD Project team
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

/* Command API tests */

#include "tests.h"

static pool *p = NULL;

static void set_up(void) {
  if (p == NULL) {
    p = make_sub_pool(NULL);
  }
}

static void tear_down(void) {
  if (p) {
    destroy_pool(p);
    p = NULL;
  } 
}

START_TEST (cmd_alloc_test) {
  cmd_rec *cmd;

  cmd = pr_cmd_alloc(NULL, 0);
  ck_assert_msg(cmd == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  cmd = pr_cmd_alloc(p, 0);
  ck_assert_msg(cmd != NULL, "Failed to create cmd_rec: %s", strerror(errno));
  ck_assert_msg(cmd->argc == 0, "Expected argc = 0, got %d", cmd->argc);

  cmd = pr_cmd_alloc(p, 1, "foo");
  ck_assert_msg(cmd != NULL, "Failed to create cmd_rec: %s", strerror(errno));
  ck_assert_msg(cmd->argc == 1, "Expected argc = 1, got %d", cmd->argc);
  ck_assert_msg(cmd->argv[1] == NULL, "Failed to null-terminate argv");
}
END_TEST

START_TEST (cmd_get_id_test) {
  int res;

  res = pr_cmd_get_id(NULL);
  ck_assert_msg(res == -1, "Failed to handle null argument");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_cmd_get_id("a");
  ck_assert_msg(res == -1, "Failed to handle unknown argument");
  ck_assert_msg(errno == ENOENT, "Failed to set errno to ENOENT");
  
  res = pr_cmd_get_id(C_USER);
  ck_assert_msg(res == PR_CMD_USER_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_USER_ID, C_USER, res); 

  res = pr_cmd_get_id(C_PASS);
  ck_assert_msg(res == PR_CMD_PASS_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_PASS_ID, C_PASS, res); 

  res = pr_cmd_get_id(C_ACCT);
  ck_assert_msg(res == PR_CMD_ACCT_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_ACCT_ID, C_ACCT, res); 

  res = pr_cmd_get_id(C_CWD);
  ck_assert_msg(res == PR_CMD_CWD_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_CWD_ID, C_CWD, res); 

  res = pr_cmd_get_id(C_XCWD);
  ck_assert_msg(res == PR_CMD_XCWD_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_XCWD_ID, C_XCWD, res); 

  res = pr_cmd_get_id(C_CDUP);
  ck_assert_msg(res == PR_CMD_CDUP_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_CDUP_ID, C_CDUP, res); 

  res = pr_cmd_get_id(C_XCUP);
  ck_assert_msg(res == PR_CMD_XCUP_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_XCUP_ID, C_XCUP, res); 

  res = pr_cmd_get_id(C_SMNT);
  ck_assert_msg(res == PR_CMD_SMNT_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_SMNT_ID, C_SMNT, res); 

  res = pr_cmd_get_id(C_REIN);
  ck_assert_msg(res == PR_CMD_REIN_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_REIN_ID, C_REIN, res); 

  res = pr_cmd_get_id(C_QUIT);
  ck_assert_msg(res == PR_CMD_QUIT_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_QUIT_ID, C_QUIT, res); 

  res = pr_cmd_get_id(C_PORT);
  ck_assert_msg(res == PR_CMD_PORT_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_PORT_ID, C_PORT, res); 

  res = pr_cmd_get_id(C_EPRT);
  ck_assert_msg(res == PR_CMD_EPRT_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_EPRT_ID, C_EPRT, res); 

  res = pr_cmd_get_id(C_PASV);
  ck_assert_msg(res == PR_CMD_PASV_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_PASV_ID, C_PASV, res); 

  res = pr_cmd_get_id(C_EPSV);
  ck_assert_msg(res == PR_CMD_EPSV_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_EPSV_ID, C_EPSV, res); 

  res = pr_cmd_get_id(C_TYPE);
  ck_assert_msg(res == PR_CMD_TYPE_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_TYPE_ID, C_TYPE, res); 

  res = pr_cmd_get_id(C_STRU);
  ck_assert_msg(res == PR_CMD_STRU_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_STRU_ID, C_STRU, res); 

  res = pr_cmd_get_id(C_MODE);
  ck_assert_msg(res == PR_CMD_MODE_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_MODE_ID, C_MODE, res); 

  res = pr_cmd_get_id(C_RETR);
  ck_assert_msg(res == PR_CMD_RETR_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_RETR_ID, C_RETR, res); 

  res = pr_cmd_get_id(C_STOR);
  ck_assert_msg(res == PR_CMD_STOR_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_STOR_ID, C_STOR, res); 

  res = pr_cmd_get_id(C_STOU);
  ck_assert_msg(res == PR_CMD_STOU_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_STOU_ID, C_STOU, res); 

  res = pr_cmd_get_id(C_APPE);
  ck_assert_msg(res == PR_CMD_APPE_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_APPE_ID, C_APPE, res); 

  res = pr_cmd_get_id(C_ALLO);
  ck_assert_msg(res == PR_CMD_ALLO_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_ALLO_ID, C_ALLO, res); 

  res = pr_cmd_get_id(C_REST);
  ck_assert_msg(res == PR_CMD_REST_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_REST_ID, C_REST, res); 

  res = pr_cmd_get_id(C_RNFR);
  ck_assert_msg(res == PR_CMD_RNFR_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_RNFR_ID, C_RNFR, res); 

  res = pr_cmd_get_id(C_RNTO);
  ck_assert_msg(res == PR_CMD_RNTO_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_RNTO_ID, C_RNTO, res); 

  res = pr_cmd_get_id(C_ABOR);
  ck_assert_msg(res == PR_CMD_ABOR_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_ABOR_ID, C_ABOR, res); 

  res = pr_cmd_get_id(C_DELE);
  ck_assert_msg(res == PR_CMD_DELE_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_DELE_ID, C_DELE, res); 

  res = pr_cmd_get_id(C_MDTM);
  ck_assert_msg(res == PR_CMD_MDTM_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_MDTM_ID, C_MDTM, res); 

  res = pr_cmd_get_id(C_MDTM);
  ck_assert_msg(res == PR_CMD_MDTM_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_MDTM_ID, C_MDTM, res); 

  res = pr_cmd_get_id(C_RMD);
  ck_assert_msg(res == PR_CMD_RMD_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_RMD_ID, C_RMD, res); 

  res = pr_cmd_get_id(C_XRMD);
  ck_assert_msg(res == PR_CMD_XRMD_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_XRMD_ID, C_XRMD, res); 

  res = pr_cmd_get_id(C_MKD);
  ck_assert_msg(res == PR_CMD_MKD_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_MKD_ID, C_MKD, res); 

  res = pr_cmd_get_id(C_MLSD);
  ck_assert_msg(res == PR_CMD_MLSD_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_MLSD_ID, C_MLSD, res); 

  res = pr_cmd_get_id(C_MLST);
  ck_assert_msg(res == PR_CMD_MLST_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_MLST_ID, C_MLST, res); 

  res = pr_cmd_get_id(C_XMKD);
  ck_assert_msg(res == PR_CMD_XMKD_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_XMKD_ID, C_XMKD, res); 

  res = pr_cmd_get_id(C_PWD);
  ck_assert_msg(res == PR_CMD_PWD_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_PWD_ID, C_PWD, res); 

  res = pr_cmd_get_id(C_XPWD);
  ck_assert_msg(res == PR_CMD_XPWD_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_XPWD_ID, C_XPWD, res); 

  res = pr_cmd_get_id(C_SIZE);
  ck_assert_msg(res == PR_CMD_SIZE_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_SIZE_ID, C_SIZE, res); 

  res = pr_cmd_get_id(C_LIST);
  ck_assert_msg(res == PR_CMD_LIST_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_LIST_ID, C_LIST, res); 

  res = pr_cmd_get_id(C_NLST);
  ck_assert_msg(res == PR_CMD_NLST_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_NLST_ID, C_NLST, res); 

  res = pr_cmd_get_id(C_SITE);
  ck_assert_msg(res == PR_CMD_SITE_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_SITE_ID, C_SITE, res); 

  res = pr_cmd_get_id(C_SYST);
  ck_assert_msg(res == PR_CMD_SYST_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_SYST_ID, C_SYST, res); 

  res = pr_cmd_get_id(C_STAT);
  ck_assert_msg(res == PR_CMD_STAT_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_STAT_ID, C_STAT, res); 

  res = pr_cmd_get_id(C_HELP);
  ck_assert_msg(res == PR_CMD_HELP_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_HELP_ID, C_HELP, res); 

  res = pr_cmd_get_id(C_NOOP);
  ck_assert_msg(res == PR_CMD_NOOP_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_NOOP_ID, C_NOOP, res); 

  res = pr_cmd_get_id(C_FEAT);
  ck_assert_msg(res == PR_CMD_FEAT_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_FEAT_ID, C_FEAT, res); 

  res = pr_cmd_get_id(C_OPTS);
  ck_assert_msg(res == PR_CMD_OPTS_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_OPTS_ID, C_OPTS, res); 

  res = pr_cmd_get_id(C_LANG);
  ck_assert_msg(res == PR_CMD_LANG_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_LANG_ID, C_LANG, res); 

  res = pr_cmd_get_id(C_HOST);
  ck_assert_msg(res == PR_CMD_HOST_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_HOST_ID, C_HOST, res); 

  res = pr_cmd_get_id(C_CLNT);
  ck_assert_msg(res == PR_CMD_CLNT_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_CLNT_ID, C_CLNT, res); 

  res = pr_cmd_get_id(C_RANG);
  ck_assert_msg(res == PR_CMD_RANG_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_RANG_ID, C_RANG, res);

  res = pr_cmd_get_id(C_CSID);
  ck_assert_msg(res == PR_CMD_CSID_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_CSID_ID, C_CSID, res);

  /* RFC 2228 commands */
  res = pr_cmd_get_id(C_ADAT);
  ck_assert_msg(res == PR_CMD_ADAT_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_ADAT_ID, C_ADAT, res); 

  res = pr_cmd_get_id(C_AUTH);
  ck_assert_msg(res == PR_CMD_AUTH_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_AUTH_ID, C_AUTH, res); 

  res = pr_cmd_get_id(C_CCC);
  ck_assert_msg(res == PR_CMD_CCC_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_CCC_ID, C_CCC, res); 

  res = pr_cmd_get_id(C_CONF);
  ck_assert_msg(res == PR_CMD_CONF_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_CONF_ID, C_CONF, res); 

  res = pr_cmd_get_id(C_ENC);
  ck_assert_msg(res == PR_CMD_ENC_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_ENC_ID, C_ENC, res); 

  res = pr_cmd_get_id(C_MIC);
  ck_assert_msg(res == PR_CMD_MIC_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_MIC_ID, C_MIC, res); 

  res = pr_cmd_get_id(C_PBSZ);
  ck_assert_msg(res == PR_CMD_PBSZ_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_PBSZ_ID, C_PBSZ, res); 

  res = pr_cmd_get_id(C_PROT);
  ck_assert_msg(res == PR_CMD_PROT_ID, "Expected cmd ID %d for '%s', got %d",
    PR_CMD_PROT_ID, C_PROT, res); 

  /* Make sure we handle lowercased, mixed-case commands as well. */
  res = pr_cmd_get_id("user");
  ck_assert_msg(res == PR_CMD_USER_ID, "Expected cmd ID %d for 'user', got %d",
    PR_CMD_USER_ID, res);

  res = pr_cmd_get_id("RnTo");
  ck_assert_msg(res == PR_CMD_RNTO_ID, "Expected cmd ID %d for 'RnTo', got %d",
    PR_CMD_RNTO_ID, res);
}
END_TEST

START_TEST (cmd_cmp_test) {
  cmd_rec *cmd;
  int res;

  res = pr_cmd_cmp(NULL, 1);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL"); 

  cmd = pr_cmd_alloc(p, 1, "foo");
  res = pr_cmd_cmp(cmd, 0);
  ck_assert_msg(res == -1, "Failed to handle bad ID argument");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL"); 

  res = pr_cmd_cmp(cmd, 1);
  ck_assert_msg(res == 1, "Failed to handle empty cmd_rec argument");

  cmd = pr_cmd_alloc(p, 1, C_RETR);
  res = pr_cmd_cmp(cmd, PR_CMD_ACCT_ID);
  ck_assert_msg(res > 0, "Unexpected comparison result: %d", res);

  res = pr_cmd_cmp(cmd, PR_CMD_STOR_ID);
  ck_assert_msg(res < 0, "Unexpected comparison result: %d", res);

  res = pr_cmd_cmp(cmd, PR_CMD_RETR_ID);
  ck_assert_msg(res == 0, "Unexpected comparison result: %d", res);

  cmd = pr_cmd_alloc(p, 1, "ReTr");
  res = pr_cmd_cmp(cmd, PR_CMD_RETR_ID);
  ck_assert_msg(res == 0, "Unexpected comparison result: %d", res);
}
END_TEST

START_TEST (cmd_strcmp_test) {
  cmd_rec *cmd;
  int res;

  res = pr_cmd_strcmp(NULL, NULL);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  mark_point();
  cmd = pr_cmd_alloc(p, 0);
  res = pr_cmd_strcmp(cmd, "a");
  ck_assert_msg(res == 1, "Failed to handle empty cmd_rec");

  mark_point();
  cmd = pr_cmd_alloc(p, 1, C_RETR);
  res = pr_cmd_strcmp(cmd, "a");
  ck_assert_msg(res < 0, "Unexpected comparison result: %d", res);

  mark_point();
  cmd->cmd_id = 0;
  res = pr_cmd_strcmp(cmd, "S");
  ck_assert_msg(res > 0, "Unexpected comparison result: %d", res);

  mark_point();
  cmd->cmd_id = 0;
  res = pr_cmd_strcmp(cmd, C_RETR);
  ck_assert_msg(res == 0, "Unexpected comparison result: %d", res);
}
END_TEST

START_TEST (cmd_get_displayable_str_test) {
  const char *ok, *res = NULL;
  cmd_rec *cmd = NULL;
  size_t len = 0;

  res = pr_cmd_get_displayable_str(NULL, NULL);
  ck_assert_msg(res == NULL, "Failed to handle null cmd_rec");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  cmd = pr_cmd_alloc(p, 1, "foo");
  res = pr_cmd_get_displayable_str(cmd, NULL);

  ok = "foo";
  ck_assert_msg(res != NULL, "Expected string, got null");
  ck_assert_msg(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  mark_point();
  cmd->argc = 0;
  res = pr_cmd_get_displayable_str(cmd, NULL);

  ck_assert_msg(res != NULL, "Expected string, got null");

  /* Note: We still expect the PREVIOUS ok value, since
   * pr_cmd_get_displayable_str() should cache the constructed string,
   * rather than creating it anew.
   */
  ck_assert_msg(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  if (pr_cmd_clear_cache(NULL) < 0) {
    ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
      strerror(errno), errno);
  }

  mark_point();
  pr_cmd_clear_cache(cmd);
  res = pr_cmd_get_displayable_str(cmd, NULL);

  ok = "";
  ck_assert_msg(res != NULL, "Expected string, got null");
  ck_assert_msg(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, "bar");
  cmd->arg = NULL;
  res = pr_cmd_get_displayable_str(cmd, NULL);

  ok = "bar";
  ck_assert_msg(res != NULL, "Expected string, got null");
  ck_assert_msg(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, "baz");
  cmd->argv[0] = NULL;
  cmd->arg = pstrdup(p, "baz");
  res = pr_cmd_get_displayable_str(cmd, NULL);

  /* cmd->argv[0] is the command name; without that, it does not matter
   * what cmd->arg is.  Hence why if cmd->argv[0] is null, we expect the
   * empty string.
   */
  ok = "";
  ck_assert_msg(res != NULL, "Expected string, got null");
  ck_assert_msg(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  mark_point();
  cmd = pr_cmd_alloc(p, 3, "foo", "bar", "baz");
  cmd->arg = NULL;
  res = pr_cmd_get_displayable_str(cmd, NULL);
  
  /* cmd->argv[0] is the command name; without that, it does not matter
   * what cmd->arg is.  Hence why if cmd->argv[0] is null, we expect the
   * empty string.
   */
  ok = "foo bar baz";
  ck_assert_msg(res != NULL, "Expected string, got null");
  ck_assert_msg(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  /* Make sure we can handle cases where cmd_rec->argv has been tampered
   * with.
   */
  mark_point();
  cmd = pr_cmd_alloc(p, 3, "foo", "bar", "baz");
  cmd->argv[0] = NULL;
  res = pr_cmd_get_displayable_str(cmd, NULL);

  ok = " bar baz";
  ck_assert_msg(res != NULL, "Expected string, got null");
  ck_assert_msg(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  mark_point();
  cmd = pr_cmd_alloc(p, 2, C_PASS, "foo");
  res = pr_cmd_get_displayable_str(cmd, &len);
  ok = "PASS (hidden)";
  ck_assert_msg(res != NULL, "Expected displayable string, got null");
  ck_assert_msg(len == 13, "Expected len 13, got %lu", (unsigned long) len);
  ck_assert_msg(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  mark_point();
  cmd = pr_cmd_alloc(p, 2, C_ADAT, "bar baz quxx");
  res = pr_cmd_get_displayable_str(cmd, &len);
  ok = "ADAT (hidden)";
  ck_assert_msg(res != NULL, "Expected displayable string, got null");
  ck_assert_msg(len == 13, "Expected len 13, got %lu", (unsigned long) len);
  ck_assert_msg(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);
}
END_TEST

START_TEST (cmd_get_errno_test) {
  int res, *xerrno = NULL;
  cmd_rec *cmd = NULL;

  res = pr_cmd_get_errno(NULL);
  ck_assert_msg(res == -1, "Failed to handle null cmd_rec");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  cmd = pr_cmd_alloc(p, 1, "foo");
  res = pr_cmd_get_errno(cmd);
  ck_assert_msg(res == 0, "Expected errno 0, got %d", res);

  (void) pr_table_remove(cmd->notes, "errno", NULL);
  res = pr_cmd_get_errno(cmd);
  ck_assert_msg(res < 0, "Failed to handle missing 'errno' note");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  xerrno = pcalloc(cmd->pool, sizeof(int));
  (void) pr_table_add(cmd->notes, "errno", xerrno, sizeof(int));

  res = pr_cmd_set_errno(NULL, ENOENT);
  ck_assert_msg(res < 0, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  res = pr_cmd_set_errno(cmd, ENOENT);
  ck_assert_msg(res == 0, "Failed to stash errno ENOENT: %s", strerror(errno));

  res = pr_cmd_get_errno(cmd);
  ck_assert_msg(res == ENOENT, "Expected errno ENOENT, got %s (%d)",
    strerror(res), res);
}
END_TEST

START_TEST (cmd_set_name_test) {
  int res;
  cmd_rec *cmd;
  const char *name;

  res = pr_cmd_set_name(NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  cmd = pr_cmd_alloc(p, 1, "foo");
  res = pr_cmd_set_name(cmd, NULL);
  ck_assert_msg(res < 0, "Failed to handle null name");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  name = "bar";
  res = pr_cmd_set_name(cmd, name);
  ck_assert_msg(res == 0, "Failed to command name to '%s': %s", name,
    strerror(errno));
}
END_TEST

START_TEST (cmd_is_http_test) {
  int res;
  cmd_rec *cmd;

  res = pr_cmd_is_http(NULL);
  ck_assert_msg(res < 0, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, C_SYST);
  cmd->argv[0] = NULL;
  res = pr_cmd_is_http(cmd);
  ck_assert_msg(res < 0, "Failed to handle null name");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  cmd->argv[0] = C_SYST;
  res = pr_cmd_is_http(cmd);
  ck_assert_msg(res == FALSE, "Expected FALSE (%d), got %d", FALSE, res);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, "GET");
  res = pr_cmd_is_http(cmd);
  ck_assert_msg(res == TRUE, "Expected TRUE (%d), got %d", TRUE, res);
}
END_TEST

START_TEST (cmd_is_smtp_test) {
  int res;
  cmd_rec *cmd;

  res = pr_cmd_is_smtp(NULL);
  ck_assert_msg(res < 0, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, C_SYST);
  cmd->argv[0] = NULL;
  res = pr_cmd_is_smtp(cmd);
  ck_assert_msg(res < 0, "Failed to handle null name");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  cmd->argv[0] = C_SYST;
  res = pr_cmd_is_smtp(cmd);
  ck_assert_msg(res == FALSE, "Expected FALSE (%d), got %d", FALSE, res);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, "RCPT");
  res = pr_cmd_is_smtp(cmd);
  ck_assert_msg(res == TRUE, "Expected TRUE (%d), got %d", TRUE, res);
}
END_TEST

START_TEST (cmd_is_ssh2_test) {
  int res;
  cmd_rec *cmd;

  res = pr_cmd_is_ssh2(NULL);
  ck_assert_msg(res < 0, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, C_SYST);
  cmd->argv[0] = NULL;
  res = pr_cmd_is_ssh2(cmd);
  ck_assert_msg(res < 0, "Failed to handle null name");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  cmd->argv[0] = C_SYST;
  res = pr_cmd_is_ssh2(cmd);
  ck_assert_msg(res == FALSE, "Expected FALSE (%d), got %d", FALSE, res);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, "SSH-2.0-OpenSSH_5.6p1");
  res = pr_cmd_is_ssh2(cmd);
  ck_assert_msg(res == TRUE, "Expected TRUE (%d), got %d", TRUE, res);

  mark_point();
  cmd = pr_cmd_alloc(p, 1, "SSH-1.99-JSCH");
  res = pr_cmd_is_ssh2(cmd);
  ck_assert_msg(res == TRUE, "Expected TRUE (%d), got %d", TRUE, res);
}
END_TEST

Suite *tests_get_cmd_suite(void) {
  Suite *suite;
  TCase *testcase;

  suite = suite_create("cmd");

  testcase = tcase_create("base");
  tcase_add_checked_fixture(testcase, set_up, tear_down);

  tcase_add_test(testcase, cmd_alloc_test);
  tcase_add_test(testcase, cmd_get_id_test);
  tcase_add_test(testcase, cmd_cmp_test);
  tcase_add_test(testcase, cmd_strcmp_test);
  tcase_add_test(testcase, cmd_get_displayable_str_test);
  tcase_add_test(testcase, cmd_get_errno_test);
  tcase_add_test(testcase, cmd_set_name_test);
  tcase_add_test(testcase, cmd_is_http_test);
  tcase_add_test(testcase, cmd_is_smtp_test);
  tcase_add_test(testcase, cmd_is_ssh2_test);

  suite_add_tcase(suite, testcase);
  return suite;
}
