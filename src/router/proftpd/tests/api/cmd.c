/*
 * ProFTPD - FTP server API testsuite
 * Copyright (c) 2011 The ProFTPD Project team
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

/* Command API tests
 * $Id: cmd.c,v 1.2 2011/05/23 20:50:30 castaglia Exp $
 */

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
  fail_unless(cmd == NULL, "Failed to handle null arguments");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL");

  cmd = pr_cmd_alloc(p, 0);
  fail_unless(cmd != NULL, "Failed to create cmd_rec: %s", strerror(errno));
  fail_unless(cmd->argc == 0, "Expected argc = 0, got %d", cmd->argc);

  cmd = pr_cmd_alloc(p, 1, "foo");
  fail_unless(cmd != NULL, "Failed to create cmd_rec: %s", strerror(errno));
  fail_unless(cmd->argc == 1, "Expected argc = 1, got %d", cmd->argc);
  fail_unless(cmd->argv[1] == NULL, "Failed to null-terminate argv");
}
END_TEST

START_TEST (cmd_get_id_test) {
  int res;

  res = pr_cmd_get_id(NULL);
  fail_unless(res == -1, "Failed to handle null argument");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_cmd_get_id("a");
  fail_unless(res == -1, "Failed to handle unknown argument");
  fail_unless(errno == ENOENT, "Failed to set errno to ENOENT");
  
  res = pr_cmd_get_id(C_USER);
  fail_unless(res == PR_CMD_USER_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_USER_ID, C_USER, res); 

  res = pr_cmd_get_id(C_PASS);
  fail_unless(res == PR_CMD_PASS_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_PASS_ID, C_PASS, res); 

  res = pr_cmd_get_id(C_ACCT);
  fail_unless(res == PR_CMD_ACCT_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_ACCT_ID, C_ACCT, res); 

  res = pr_cmd_get_id(C_CWD);
  fail_unless(res == PR_CMD_CWD_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_CWD_ID, C_CWD, res); 

  res = pr_cmd_get_id(C_XCWD);
  fail_unless(res == PR_CMD_XCWD_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_XCWD_ID, C_XCWD, res); 

  res = pr_cmd_get_id(C_CDUP);
  fail_unless(res == PR_CMD_CDUP_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_CDUP_ID, C_CDUP, res); 

  res = pr_cmd_get_id(C_XCUP);
  fail_unless(res == PR_CMD_XCUP_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_XCUP_ID, C_XCUP, res); 

  res = pr_cmd_get_id(C_SMNT);
  fail_unless(res == PR_CMD_SMNT_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_SMNT_ID, C_SMNT, res); 

  res = pr_cmd_get_id(C_REIN);
  fail_unless(res == PR_CMD_REIN_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_REIN_ID, C_REIN, res); 

  res = pr_cmd_get_id(C_QUIT);
  fail_unless(res == PR_CMD_QUIT_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_QUIT_ID, C_QUIT, res); 

  res = pr_cmd_get_id(C_PORT);
  fail_unless(res == PR_CMD_PORT_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_PORT_ID, C_PORT, res); 

  res = pr_cmd_get_id(C_EPRT);
  fail_unless(res == PR_CMD_EPRT_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_EPRT_ID, C_EPRT, res); 

  res = pr_cmd_get_id(C_PASV);
  fail_unless(res == PR_CMD_PASV_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_PASV_ID, C_PASV, res); 

  res = pr_cmd_get_id(C_EPSV);
  fail_unless(res == PR_CMD_EPSV_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_EPSV_ID, C_EPSV, res); 

  res = pr_cmd_get_id(C_TYPE);
  fail_unless(res == PR_CMD_TYPE_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_TYPE_ID, C_TYPE, res); 

  res = pr_cmd_get_id(C_STRU);
  fail_unless(res == PR_CMD_STRU_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_STRU_ID, C_STRU, res); 

  res = pr_cmd_get_id(C_MODE);
  fail_unless(res == PR_CMD_MODE_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_MODE_ID, C_MODE, res); 

  res = pr_cmd_get_id(C_RETR);
  fail_unless(res == PR_CMD_RETR_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_RETR_ID, C_RETR, res); 

  res = pr_cmd_get_id(C_STOR);
  fail_unless(res == PR_CMD_STOR_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_STOR_ID, C_STOR, res); 

  res = pr_cmd_get_id(C_STOU);
  fail_unless(res == PR_CMD_STOU_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_STOU_ID, C_STOU, res); 

  res = pr_cmd_get_id(C_APPE);
  fail_unless(res == PR_CMD_APPE_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_APPE_ID, C_APPE, res); 

  res = pr_cmd_get_id(C_ALLO);
  fail_unless(res == PR_CMD_ALLO_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_ALLO_ID, C_ALLO, res); 

  res = pr_cmd_get_id(C_REST);
  fail_unless(res == PR_CMD_REST_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_REST_ID, C_REST, res); 

  res = pr_cmd_get_id(C_RNFR);
  fail_unless(res == PR_CMD_RNFR_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_RNFR_ID, C_RNFR, res); 

  res = pr_cmd_get_id(C_RNTO);
  fail_unless(res == PR_CMD_RNTO_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_RNTO_ID, C_RNTO, res); 

  res = pr_cmd_get_id(C_ABOR);
  fail_unless(res == PR_CMD_ABOR_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_ABOR_ID, C_ABOR, res); 

  res = pr_cmd_get_id(C_DELE);
  fail_unless(res == PR_CMD_DELE_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_DELE_ID, C_DELE, res); 

  res = pr_cmd_get_id(C_MDTM);
  fail_unless(res == PR_CMD_MDTM_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_MDTM_ID, C_MDTM, res); 

  res = pr_cmd_get_id(C_MDTM);
  fail_unless(res == PR_CMD_MDTM_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_MDTM_ID, C_MDTM, res); 

  res = pr_cmd_get_id(C_RMD);
  fail_unless(res == PR_CMD_RMD_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_RMD_ID, C_RMD, res); 

  res = pr_cmd_get_id(C_XRMD);
  fail_unless(res == PR_CMD_XRMD_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_XRMD_ID, C_XRMD, res); 

  res = pr_cmd_get_id(C_MKD);
  fail_unless(res == PR_CMD_MKD_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_MKD_ID, C_MKD, res); 

  res = pr_cmd_get_id(C_MLSD);
  fail_unless(res == PR_CMD_MLSD_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_MLSD_ID, C_MLSD, res); 

  res = pr_cmd_get_id(C_MLST);
  fail_unless(res == PR_CMD_MLST_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_MLST_ID, C_MLST, res); 

  res = pr_cmd_get_id(C_XMKD);
  fail_unless(res == PR_CMD_XMKD_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_XMKD_ID, C_XMKD, res); 

  res = pr_cmd_get_id(C_PWD);
  fail_unless(res == PR_CMD_PWD_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_PWD_ID, C_PWD, res); 

  res = pr_cmd_get_id(C_XPWD);
  fail_unless(res == PR_CMD_XPWD_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_XPWD_ID, C_XPWD, res); 

  res = pr_cmd_get_id(C_SIZE);
  fail_unless(res == PR_CMD_SIZE_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_SIZE_ID, C_SIZE, res); 

  res = pr_cmd_get_id(C_LIST);
  fail_unless(res == PR_CMD_LIST_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_LIST_ID, C_LIST, res); 

  res = pr_cmd_get_id(C_NLST);
  fail_unless(res == PR_CMD_NLST_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_NLST_ID, C_NLST, res); 

  res = pr_cmd_get_id(C_SITE);
  fail_unless(res == PR_CMD_SITE_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_SITE_ID, C_SITE, res); 

  res = pr_cmd_get_id(C_SYST);
  fail_unless(res == PR_CMD_SYST_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_SYST_ID, C_SYST, res); 

  res = pr_cmd_get_id(C_STAT);
  fail_unless(res == PR_CMD_STAT_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_STAT_ID, C_STAT, res); 

  res = pr_cmd_get_id(C_HELP);
  fail_unless(res == PR_CMD_HELP_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_HELP_ID, C_HELP, res); 

  res = pr_cmd_get_id(C_NOOP);
  fail_unless(res == PR_CMD_NOOP_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_NOOP_ID, C_NOOP, res); 

  res = pr_cmd_get_id(C_FEAT);
  fail_unless(res == PR_CMD_FEAT_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_FEAT_ID, C_FEAT, res); 

  res = pr_cmd_get_id(C_OPTS);
  fail_unless(res == PR_CMD_OPTS_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_OPTS_ID, C_OPTS, res); 

  res = pr_cmd_get_id(C_LANG);
  fail_unless(res == PR_CMD_LANG_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_LANG_ID, C_LANG, res); 

  res = pr_cmd_get_id(C_ADAT);
  fail_unless(res == PR_CMD_ADAT_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_ADAT_ID, C_ADAT, res); 

  res = pr_cmd_get_id(C_AUTH);
  fail_unless(res == PR_CMD_AUTH_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_AUTH_ID, C_AUTH, res); 

  res = pr_cmd_get_id(C_CCC);
  fail_unless(res == PR_CMD_CCC_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_CCC_ID, C_CCC, res); 

  res = pr_cmd_get_id(C_CONF);
  fail_unless(res == PR_CMD_CONF_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_CONF_ID, C_CONF, res); 

  res = pr_cmd_get_id(C_ENC);
  fail_unless(res == PR_CMD_ENC_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_ENC_ID, C_ENC, res); 

  res = pr_cmd_get_id(C_MIC);
  fail_unless(res == PR_CMD_MIC_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_MIC_ID, C_MIC, res); 

  res = pr_cmd_get_id(C_PBSZ);
  fail_unless(res == PR_CMD_PBSZ_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_PBSZ_ID, C_PBSZ, res); 

  res = pr_cmd_get_id(C_PROT);
  fail_unless(res == PR_CMD_PROT_ID, "Expected cmd ID %d for %s, got %d",
    PR_CMD_PROT_ID, C_PROT, res); 
}
END_TEST

START_TEST (cmd_cmp_test) {
  cmd_rec *cmd;
  int res;

  res = pr_cmd_cmp(NULL, 1);
  fail_unless(res == -1, "Failed to handle null arguments");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL"); 

  cmd = pr_cmd_alloc(p, 1, "foo");
  res = pr_cmd_cmp(cmd, 0);
  fail_unless(res == -1, "Failed to handle bad ID argument");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL"); 

  res = pr_cmd_cmp(cmd, 1);
  fail_unless(res == 1, "Failed to handle empty cmd_rec argument");

  cmd = pr_cmd_alloc(p, 1, C_RETR);
  res = pr_cmd_cmp(cmd, PR_CMD_ACCT_ID);
  fail_unless(res > 0, "Unexpected comparison result: %d", res);

  res = pr_cmd_cmp(cmd, PR_CMD_STOR_ID);
  fail_unless(res < 0, "Unexpected comparison result: %d", res);

  res = pr_cmd_cmp(cmd, PR_CMD_RETR_ID);
  fail_unless(res == 0, "Unexpected comparison result: %d", res);
}
END_TEST

START_TEST (cmd_strcmp_test) {
  cmd_rec *cmd;
  int res;

  res = pr_cmd_strcmp(NULL, NULL);
  fail_unless(res == -1, "Failed to handle null arguments");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL");

  mark_point();
  cmd = pr_cmd_alloc(p, 0);
  res = pr_cmd_strcmp(cmd, "a");
  fail_unless(res == 1, "Failed to handle empty cmd_rec");

  mark_point();
  cmd = pr_cmd_alloc(p, 1, C_RETR);
  res = pr_cmd_strcmp(cmd, "a");
  fail_unless(res > 0, "Unexpected comparison result: %d", res);

  mark_point();
  cmd->cmd_id = 0;
  res = pr_cmd_strcmp(cmd, "S");
  fail_unless(res > 0, "Unexpected comparison result: %d", res);

  mark_point();
  cmd->cmd_id = 0;
  res = pr_cmd_strcmp(cmd, C_RETR);
  fail_unless(res == 0, "Unexpected comparison result: %d", res);
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

  suite_add_tcase(suite, testcase);

  return suite;
}
