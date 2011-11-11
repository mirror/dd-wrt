/*
 * ProFTPD - mod_sftp miscellaneous
 * Copyright (c) 2010 TJ Saunders
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
 * As a special exemption, TJ Saunders and other respective copyright holders
 * give permission to link this program with OpenSSL, and distribute the
 * resulting executable, without including the source code for OpenSSL in the
 * source distribution.
 *
 * $Id: misc.c,v 1.2 2011/05/23 21:03:12 castaglia Exp $
 */

#include "mod_sftp.h"
#include "misc.h"

int sftp_misc_handle_chown(pr_fh_t *fh) {
  struct stat st;
  int res, xerrno;
 
  /* session.fsgid defaults to -1, so chown(2) won't chgrp unless specifically
   * requested via GroupOwner.
   */
  if (session.fsuid != (uid_t) -1) {

    PRIVS_ROOT
    res = pr_fsio_fchown(fh, session.fsuid, session.fsgid);
    xerrno = errno;
    PRIVS_RELINQUISH

    if (res < 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "chown(%s) as root failed: %s", fh->fh_path, strerror(xerrno));

    } else {
      if (session.fsgid != (gid_t) -1) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "root chown(%s) to UID %lu, GID %lu successful", fh->fh_path,
          (unsigned long) session.fsuid, (unsigned long) session.fsgid);

      } else {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "root chown(%s) to UID %lu successful", fh->fh_path,
          (unsigned long) session.fsuid);
      }

      pr_fs_clear_cache();
      pr_fsio_fstat(fh, &st);

      /* The chmod happens after the chown because chown will remove the
       * S{U,G}ID bits on some files (namely, directories); the subsequent
       * chmod is used to restore those dropped bits.  This makes it necessary
       * to use root privs when doing the chmod as well (at least in the case
       * of chown'ing the file via root privs) in order to ensure that the mode
       * can be set (a file might be being "given away", and if root privs
       * aren't used, the chmod() will fail because the old owner/session user
       * doesn't have the necessary privileges to do so).
       */
      PRIVS_ROOT
      res = pr_fsio_fchmod(fh, st.st_mode);
      xerrno = errno;
      PRIVS_RELINQUISH

      if (res < 0) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "root chmod(%s) to %04o failed: %s", fh->fh_path,
          (unsigned int) st.st_mode, strerror(xerrno));

      } else {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "root chmod(%s) to %04o successful", fh->fh_path,
          (unsigned int) st.st_mode);
      }
    }

  } else if (session.fsgid != (gid_t) -1) {
    register unsigned int i;
    int use_root_privs = TRUE;

    /* Check if session.fsgid is in session.gids.  If not, use root privs. */
    for (i = 0; i < session.gids->nelts; i++) {
      gid_t *group_ids = session.gids->elts;

      if (group_ids[i] == session.fsgid) {
        use_root_privs = FALSE;
        break;
      }
    }

    if (use_root_privs) {
      PRIVS_ROOT
    }

    res = pr_fsio_fchown(fh, (uid_t) -1, session.fsgid);
    xerrno = errno;

    if (use_root_privs) {
      PRIVS_RELINQUISH
    }

    if (res < 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "%schown(%s) failed: %s", use_root_privs ? "root " : "", fh->fh_path,
        strerror(xerrno));

    } else {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "%schown(%s) to GID %lu successful",
        use_root_privs ? "root " : "", fh->fh_path,
        (unsigned long) session.fsgid);

      pr_fs_clear_cache();
      pr_fsio_fstat(fh, &st);

      if (use_root_privs) {
        PRIVS_ROOT
      }

      res = pr_fsio_fchmod(fh, st.st_mode);
      xerrno = errno;

      if (use_root_privs) {
        PRIVS_RELINQUISH
      }

      if (res < 0) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "%schmod(%s) to %04o failed: %s", use_root_privs ? "root " : "",
          fh->fh_path, (unsigned int) st.st_mode, strerror(xerrno));
      }
    }
  }

  return 0;
}
