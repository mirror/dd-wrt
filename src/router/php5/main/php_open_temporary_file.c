/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2014 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Zeev Suraski <zeev@zend.com>                                 |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

#include "php.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef PHP_WIN32
#define O_RDONLY _O_RDONLY
#include "win32/param.h"
#include "win32/winutil.h"
#elif defined(NETWARE)
#ifdef USE_WINSOCK
#include <novsock2.h>
#else
#include <sys/socket.h>
#endif
#include <sys/param.h>
#else
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif

#if !defined(P_tmpdir)
#define P_tmpdir ""
#endif

/* {{{ php_do_open_temporary_file */

/* Loosely based on a tempnam() implementation by UCLA */

/*
 * Copyright (c) 1988, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

static int php_do_open_temporary_file(const char *path, const char *pfx, char **opened_path_p TSRMLS_DC)
{
	char *trailing_slash;
	char *opened_path;
	char cwd[MAXPATHLEN];
	cwd_state new_state;
	int fd = -1;
#ifndef HAVE_MKSTEMP
	int open_flags = O_CREAT | O_TRUNC | O_RDWR
#ifdef PHP_WIN32
		| _O_BINARY
#endif
		;
#endif

	if (!path || !path[0]) {
		return -1;
	}

#ifdef PHP_WIN32
	if (!php_win32_check_trailing_space(pfx, (const int)strlen(pfx))) {
		SetLastError(ERROR_INVALID_NAME);
		return -1;
	}
#endif

	if (!VCWD_GETCWD(cwd, MAXPATHLEN)) {
		cwd[0] = '\0';
	}

	new_state.cwd = strdup(cwd);
	new_state.cwd_length = strlen(cwd);

	if (virtual_file_ex(&new_state, path, NULL, CWD_REALPATH TSRMLS_CC)) {
		free(new_state.cwd);
		return -1;
	}

	if (IS_SLASH(new_state.cwd[new_state.cwd_length - 1])) {
		trailing_slash = "";
	} else {
		trailing_slash = "/";
	}

	if (spprintf(&opened_path, 0, "%s%s%sXXXXXX", new_state.cwd, trailing_slash, pfx) >= MAXPATHLEN) {
		efree(opened_path);
		free(new_state.cwd);
		return -1;
	}

#ifdef PHP_WIN32

	if (GetTempFileName(new_state.cwd, pfx, 0, opened_path)) {
		/* Some versions of windows set the temp file to be read-only,
		 * which means that opening it will fail... */
		if (VCWD_CHMOD(opened_path, 0600)) {
			efree(opened_path);
			free(new_state.cwd);
			return -1;
		}
		fd = VCWD_OPEN_MODE(opened_path, open_flags, 0600);
	}

#elif defined(HAVE_MKSTEMP)
	fd = mkstemp(opened_path);
#else
	if (mktemp(opened_path)) {
		fd = VCWD_OPEN(opened_path, open_flags);
	}
#endif

	if (fd == -1 || !opened_path_p) {
		efree(opened_path);
	} else {
		*opened_path_p = opened_path;
	}
	free(new_state.cwd);
	return fd;
}
/* }}} */

/* Cache the chosen temporary directory. */
static char* temporary_directory;

PHPAPI void php_shutdown_temporary_directory(void)
{
	if (temporary_directory) {
		free(temporary_directory);
		temporary_directory = NULL;
	}
}

/*
 *  Determine where to place temporary files.
 */
PHPAPI const char* php_get_temporary_directory(TSRMLS_D)
{
	/* Did we determine the temporary directory already? */
	if (temporary_directory) {
		return temporary_directory;
	}

	/* Is there a temporary directory "sys_temp_dir" in .ini defined? */
	{
		char *sys_temp_dir = PG(sys_temp_dir);
		if (sys_temp_dir) {
			int len = strlen(sys_temp_dir);
			if (len >= 2 && sys_temp_dir[len - 1] == DEFAULT_SLASH) {
				temporary_directory = zend_strndup(sys_temp_dir, len - 1);
				return temporary_directory;
			} else if (len >= 1 && sys_temp_dir[len - 1] != DEFAULT_SLASH) {
				temporary_directory = zend_strndup(sys_temp_dir, len);
				return temporary_directory;
			}
		}
	}

#ifdef PHP_WIN32
	/* We can't count on the environment variables TEMP or TMP,
	 * and so must make the Win32 API call to get the default
	 * directory for temporary files.  Note this call checks
	 * the environment values TMP and TEMP (in order) first.
	 */
	{
		char sTemp[MAX_PATH];
		DWORD len = GetTempPath(sizeof(sTemp),sTemp);
		assert(0 < len);  /* should *never* fail! */
		if (sTemp[len - 1] == DEFAULT_SLASH) {
			temporary_directory = zend_strndup(sTemp, len - 1);
		} else {
			temporary_directory = zend_strndup(sTemp, len);
		}
		return temporary_directory;
	}
#else
	/* On Unix use the (usual) TMPDIR environment variable. */
	{
		char* s = getenv("TMPDIR");
		if (s && *s) {
			int len = strlen(s);

			if (s[len - 1] == DEFAULT_SLASH) {
				temporary_directory = zend_strndup(s, len - 1);
			} else {
				temporary_directory = zend_strndup(s, len);
			}

			return temporary_directory;
		}
	}
#ifdef P_tmpdir
	/* Use the standard default temporary directory. */
	if (P_tmpdir) {
		temporary_directory = strdup(P_tmpdir);
		return temporary_directory;
	}
#endif
	/* Shouldn't ever(!) end up here ... last ditch default. */
	temporary_directory = strdup("/tmp");
	return temporary_directory;
#endif
}

/* {{{ php_open_temporary_file
 *
 * Unlike tempnam(), the supplied dir argument takes precedence
 * over the TMPDIR environment variable
 * This function should do its best to return a file pointer to a newly created
 * unique file, on every platform.
 */
PHPAPI int php_open_temporary_fd_ex(const char *dir, const char *pfx, char **opened_path_p, zend_bool open_basedir_check TSRMLS_DC)
{
	int fd;
	const char *temp_dir;

	if (!pfx) {
		pfx = "tmp.";
	}
	if (opened_path_p) {
		*opened_path_p = NULL;
	}

	if (!dir || *dir == '\0') {
def_tmp:
		temp_dir = php_get_temporary_directory(TSRMLS_C);

		if (temp_dir && *temp_dir != '\0' && (!open_basedir_check || !php_check_open_basedir(temp_dir TSRMLS_CC))) {
			return php_do_open_temporary_file(temp_dir, pfx, opened_path_p TSRMLS_CC);
		} else {
			return -1;
		}
	}

	/* Try the directory given as parameter. */
	fd = php_do_open_temporary_file(dir, pfx, opened_path_p TSRMLS_CC);
	if (fd == -1) {
		/* Use default temporary directory. */
		goto def_tmp;
	}
	return fd;
}

PHPAPI int php_open_temporary_fd(const char *dir, const char *pfx, char **opened_path_p TSRMLS_DC)
{
	return php_open_temporary_fd_ex(dir, pfx, opened_path_p, 0 TSRMLS_CC);
}

PHPAPI FILE *php_open_temporary_file(const char *dir, const char *pfx, char **opened_path_p TSRMLS_DC)
{
	FILE *fp;
	int fd = php_open_temporary_fd(dir, pfx, opened_path_p TSRMLS_CC);

	if (fd == -1) {
		return NULL;
	}

	fp = fdopen(fd, "r+b");
	if (fp == NULL) {
		close(fd);
	}

	return fp;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
