/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2014 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt.                                 |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Piere-Alain Joye <pierre@php.net>                            |
  +----------------------------------------------------------------------+
*/

/* $Id: 05dd1ecc211075107543b0ef8cee488dd229fccf $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/file.h"
#include "ext/standard/php_string.h"
#include "ext/pcre/php_pcre.h"
#include "ext/standard/php_filestat.h"
#include "php_zip.h"
#include "lib/zip.h"
#include "lib/zipint.h"

/* zip_open is a macro for renaming libzip zipopen, so we need to use PHP_NAMED_FUNCTION */
static PHP_NAMED_FUNCTION(zif_zip_open);
static PHP_NAMED_FUNCTION(zif_zip_read);
static PHP_NAMED_FUNCTION(zif_zip_close);
static PHP_NAMED_FUNCTION(zif_zip_entry_read);
static PHP_NAMED_FUNCTION(zif_zip_entry_filesize);
static PHP_NAMED_FUNCTION(zif_zip_entry_name);
static PHP_NAMED_FUNCTION(zif_zip_entry_compressedsize);
static PHP_NAMED_FUNCTION(zif_zip_entry_compressionmethod);
static PHP_NAMED_FUNCTION(zif_zip_entry_open);
static PHP_NAMED_FUNCTION(zif_zip_entry_close);

#ifdef HAVE_GLOB
#ifndef PHP_WIN32
#include <glob.h>
#else
#include "win32/glob.h"
#endif
#endif

/* {{{ Resource le */
static int le_zip_dir;
#define le_zip_dir_name "Zip Directory"
static int le_zip_entry;
#define le_zip_entry_name "Zip Entry"
/* }}} */

/* {{{ PHP_ZIP_STAT_INDEX(za, index, flags, sb) */
#define PHP_ZIP_STAT_INDEX(za, index, flags, sb) \
	if (zip_stat_index(za, index, flags, &sb) != 0) { \
		RETURN_FALSE; \
	}
/* }}} */

/* {{{  PHP_ZIP_STAT_PATH(za, path, path_len, flags, sb) */
#define PHP_ZIP_STAT_PATH(za, path, path_len, flags, sb) \
	if (path_len < 1) { \
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Empty string as entry name"); \
		RETURN_FALSE; \
	} \
	if (zip_stat(za, path, flags, &sb) != 0) { \
		RETURN_FALSE; \
	}
/* }}} */

/* {{{ PHP_ZIP_SET_FILE_COMMENT(za, index, comment, comment_len) */
#define PHP_ZIP_SET_FILE_COMMENT(za, index, comment, comment_len) \
	if (comment_len == 0) { \
		/* Passing NULL remove the existing comment */ \
		if (zip_set_file_comment(intern, index, NULL, 0) < 0) { \
			RETURN_FALSE; \
		} \
	} else if (zip_set_file_comment(intern, index, comment, comment_len) < 0) { \
		RETURN_FALSE; \
	} \
	RETURN_TRUE;
/* }}} */

#if (PHP_MAJOR_VERSION < 6)
# define add_ascii_assoc_string add_assoc_string
# define add_ascii_assoc_long add_assoc_long
#endif

/* Flatten a path by making a relative path (to .)*/
static char * php_zip_make_relative_path(char *path, int path_len) /* {{{ */
{
	char *path_begin = path;
	size_t i;

	if (path_len < 1 || path == NULL) {
		return NULL;
	}

	if (IS_SLASH(path[0])) {
		return path + 1;
	}

	i = path_len;

	while (1) {
		while (i > 0 && !IS_SLASH(path[i])) {
			i--;
		}

		if (!i) {
			return path;
		}

		if (i >= 2 && (path[i -1] == '.' || path[i -1] == ':')) {
			/* i is the position of . or :, add 1 for / */
			path_begin = path + i + 1;
			break;
		}
		i--;
	}

	return path_begin;
}
/* }}} */

#ifdef PHP_ZIP_USE_OO
/* {{{ php_zip_extract_file */
static int php_zip_extract_file(struct zip * za, char *dest, char *file, int file_len TSRMLS_DC)
{
	php_stream_statbuf ssb;
	struct zip_file *zf;
	struct zip_stat sb;
	char b[8192];
	int n, len, ret;
	php_stream *stream;
	char *fullpath;
	char *file_dirname_fullpath;
	char file_dirname[MAXPATHLEN];
	size_t dir_len;
	char *file_basename;
	size_t file_basename_len;
	int is_dir_only = 0;
	char *path_cleaned;
	size_t path_cleaned_len;
	cwd_state new_state;

	new_state.cwd = (char*)malloc(1);
	new_state.cwd[0] = '\0';
	new_state.cwd_length = 0;

	/* Clean/normlize the path and then transform any path (absolute or relative)
		 to a path relative to cwd (../../mydir/foo.txt > mydir/foo.txt)
	 */
	virtual_file_ex(&new_state, file, NULL, CWD_EXPAND TSRMLS_CC);
	path_cleaned =  php_zip_make_relative_path(new_state.cwd, new_state.cwd_length);
	if(!path_cleaned) {
		return 0;
	}
	path_cleaned_len = strlen(path_cleaned);

	if (path_cleaned_len >= MAXPATHLEN || zip_stat(za, file, 0, &sb) != 0) {
		return 0;
	}

	/* it is a directory only, see #40228 */
	if (path_cleaned_len > 1 && IS_SLASH(path_cleaned[path_cleaned_len - 1])) {
		len = spprintf(&file_dirname_fullpath, 0, "%s/%s", dest, file);
		is_dir_only = 1;
	} else {
		memcpy(file_dirname, path_cleaned, path_cleaned_len);
		dir_len = php_dirname(file_dirname, path_cleaned_len);

		if (dir_len <= 0 || (dir_len == 1 && file_dirname[0] == '.')) {
			len = spprintf(&file_dirname_fullpath, 0, "%s", dest);
		} else {
			len = spprintf(&file_dirname_fullpath, 0, "%s/%s", dest, file_dirname);
		}

		php_basename(path_cleaned, path_cleaned_len, NULL, 0, &file_basename, (size_t *)&file_basename_len TSRMLS_CC);

		if (ZIP_OPENBASEDIR_CHECKPATH(file_dirname_fullpath)) {
			efree(file_dirname_fullpath);
			efree(file_basename);
			free(new_state.cwd);
			return 0;
		}
	}

	/* let see if the path already exists */
	if (php_stream_stat_path_ex(file_dirname_fullpath, PHP_STREAM_URL_STAT_QUIET, &ssb, NULL) < 0) {

#if defined(PHP_WIN32) && (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION == 1)
		char *e;
		e = file_dirname_fullpath;
		while (*e) {
			   if (*e == '/') {
					   *e = DEFAULT_SLASH;
			   }
			   e++;
		}
#endif

		ret = php_stream_mkdir(file_dirname_fullpath, 0777,  PHP_STREAM_MKDIR_RECURSIVE|REPORT_ERRORS, NULL);
		if (!ret) {
			efree(file_dirname_fullpath);
			if (!is_dir_only) {
				efree(file_basename);
				free(new_state.cwd);
			}
			return 0;
		}
	}

	/* it is a standalone directory, job done */
	if (is_dir_only) {
		efree(file_dirname_fullpath);
		free(new_state.cwd);
		return 1;
	}

	len = spprintf(&fullpath, 0, "%s/%s", file_dirname_fullpath, file_basename);
	if (!len) {
		efree(file_dirname_fullpath);
		efree(file_basename);
		free(new_state.cwd);
		return 0;
	} else if (len > MAXPATHLEN) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Full extraction path exceed MAXPATHLEN (%i)", MAXPATHLEN);
		efree(file_dirname_fullpath);
		efree(file_basename);
		free(new_state.cwd);
		return 0;
	}

	/* check again the full path, not sure if it
	 * is required, does a file can have a different
	 * safemode status as its parent folder?
	 */
	if (ZIP_OPENBASEDIR_CHECKPATH(fullpath)) {
		efree(fullpath);
		efree(file_dirname_fullpath);
		efree(file_basename);
		free(new_state.cwd);
		return 0;
	}

#if PHP_API_VERSION < 20100412
	stream = php_stream_open_wrapper(fullpath, "w+b", REPORT_ERRORS|ENFORCE_SAFE_MODE, NULL);
#else
	stream = php_stream_open_wrapper(fullpath, "w+b", REPORT_ERRORS, NULL);
#endif

	if (stream == NULL) {
		n = -1;
		goto done;
	}

	zf = zip_fopen(za, file, 0);
	if (zf == NULL) {
		n = -1;
		php_stream_close(stream);
		goto done;
	}

	n = 0;

	while ((n=zip_fread(zf, b, sizeof(b))) > 0) {
		php_stream_write(stream, b, n);
	}

	php_stream_close(stream);
	n = zip_fclose(zf);

done:
	efree(fullpath);
	efree(file_basename);
	efree(file_dirname_fullpath);
	free(new_state.cwd);

	if (n<0) {
		return 0;
	} else {
		return 1;
	}
}
/* }}} */

static int php_zip_add_file(struct zip *za, const char *filename, size_t filename_len,
	char *entry_name, size_t entry_name_len, long offset_start, long offset_len TSRMLS_DC) /* {{{ */
{
	struct zip_source *zs;
	int cur_idx;
	char resolved_path[MAXPATHLEN];
	zval exists_flag;


	if (ZIP_OPENBASEDIR_CHECKPATH(filename)) {
		return -1;
	}

	if (!expand_filepath(filename, resolved_path TSRMLS_CC)) {
		return -1;
	}

	php_stat(resolved_path, strlen(resolved_path), FS_EXISTS, &exists_flag TSRMLS_CC);
	if (!Z_BVAL(exists_flag)) {
		return -1;
	}

	zs = zip_source_file(za, resolved_path, offset_start, offset_len);
	if (!zs) {
		return -1;
	}

	cur_idx = zip_name_locate(za, (const char *)entry_name, 0);
	/* TODO: fix  _zip_replace */
	if (cur_idx<0) {
		/* reset the error */
		if (za->error.str) {
			_zip_error_fini(&za->error);
		}
		_zip_error_init(&za->error);
	} else {
		if (zip_delete(za, cur_idx) == -1) {
			zip_source_free(zs);
			return -1;
		}
	}

	if (zip_add(za, entry_name, zs) == -1) {
		return -1;
	} else {
		return 1;
	}
}
/* }}} */

static int php_zip_parse_options(zval *options, long *remove_all_path,
	char **remove_path, int *remove_path_len, char **add_path, int *add_path_len TSRMLS_DC) /* {{{ */
{
	zval **option;
	if (zend_hash_find(HASH_OF(options), "remove_all_path", sizeof("remove_all_path"), (void **)&option) == SUCCESS) {
		long opt;
		if (Z_TYPE_PP(option) != IS_LONG) {
			zval tmp = **option;
			zval_copy_ctor(&tmp);
			convert_to_long(&tmp);
			opt = Z_LVAL(tmp);
		} else {
			opt = Z_LVAL_PP(option);
		}
		*remove_all_path = opt;
	}

	/* If I add more options, it would make sense to create a nice static struct and loop over it. */
	if (zend_hash_find(HASH_OF(options), "remove_path", sizeof("remove_path"), (void **)&option) == SUCCESS) {
		if (Z_TYPE_PP(option) != IS_STRING) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "remove_path option expected to be a string");
			return -1;
		}

		if (Z_STRLEN_PP(option) < 1) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Empty string given as remove_path option");
			return -1;
		}

		if (Z_STRLEN_PP(option) >= MAXPATHLEN) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "remove_path string is too long (max: %i, %i given)",
						MAXPATHLEN - 1, Z_STRLEN_PP(option));
			return -1;
		}
		*remove_path_len = Z_STRLEN_PP(option);
		*remove_path = Z_STRVAL_PP(option);
	}

	if (zend_hash_find(HASH_OF(options), "add_path", sizeof("add_path"), (void **)&option) == SUCCESS) {
		if (Z_TYPE_PP(option) != IS_STRING) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "add_path option expected to be a string");
			return -1;
		}

		if (Z_STRLEN_PP(option) < 1) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Empty string given as the add_path option");
			return -1;
		}

		if (Z_STRLEN_PP(option) >= MAXPATHLEN) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "add_path string too long (max: %i, %i given)",
						MAXPATHLEN - 1, Z_STRLEN_PP(option));
			return -1;
		}
		*add_path_len = Z_STRLEN_PP(option);
		*add_path = Z_STRVAL_PP(option);
	}
	return 1;
}
/* }}} */

/* {{{ REGISTER_ZIP_CLASS_CONST_LONG */
#define REGISTER_ZIP_CLASS_CONST_LONG(const_name, value) \
	    zend_declare_class_constant_long(zip_class_entry, const_name, sizeof(const_name)-1, (long)value TSRMLS_CC);
/* }}} */

/* {{{ ZIP_FROM_OBJECT */
#define ZIP_FROM_OBJECT(intern, object) \
	{ \
		ze_zip_object *obj = (ze_zip_object*) zend_object_store_get_object(object TSRMLS_CC); \
		intern = obj->za; \
		if (!intern) { \
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid or unitialized Zip object"); \
			RETURN_FALSE; \
		} \
	}
/* }}} */

/* {{{ RETURN_SB(sb) */
#define RETURN_SB(sb) \
	{ \
		array_init(return_value); \
		add_ascii_assoc_string(return_value, "name", (char *)(sb)->name, 1); \
		add_ascii_assoc_long(return_value, "index", (long) (sb)->index); \
		add_ascii_assoc_long(return_value, "crc", (long) (sb)->crc); \
		add_ascii_assoc_long(return_value, "size", (long) (sb)->size); \
		add_ascii_assoc_long(return_value, "mtime", (long) (sb)->mtime); \
		add_ascii_assoc_long(return_value, "comp_size", (long) (sb)->comp_size); \
		add_ascii_assoc_long(return_value, "comp_method", (long) (sb)->comp_method); \
	}
/* }}} */

static int php_zip_status(struct zip *za TSRMLS_DC) /* {{{ */
{
	int zep, syp;

	zip_error_get(za, &zep, &syp);
	return zep;
}
/* }}} */

static int php_zip_status_sys(struct zip *za TSRMLS_DC) /* {{{ */
{
	int zep, syp;

	zip_error_get(za, &zep, &syp);
	return syp;
}
/* }}} */

static int php_zip_get_num_files(struct zip *za TSRMLS_DC) /* {{{ */
{
	return zip_get_num_files(za);
}
/* }}} */

static char * php_zipobj_get_filename(ze_zip_object *obj TSRMLS_DC) /* {{{ */
{

	if (!obj) {
		return NULL;
	}

	if (obj->filename) {
		return obj->filename;
	}
	return NULL;
}
/* }}} */

static char * php_zipobj_get_zip_comment(struct zip *za, int *len TSRMLS_DC) /* {{{ */
{
	if (za) {
		return (char *)zip_get_archive_comment(za, len, 0);
	}
	return NULL;
}
/* }}} */

#ifdef HAVE_GLOB /* {{{ */
#ifndef GLOB_ONLYDIR
#define GLOB_ONLYDIR (1<<30)
#define GLOB_EMULATE_ONLYDIR
#define GLOB_FLAGMASK (~GLOB_ONLYDIR)
#else
#define GLOB_FLAGMASK (~0)
#endif
#ifndef GLOB_BRACE
# define GLOB_BRACE 0
#endif
#ifndef GLOB_MARK
# define GLOB_MARK 0
#endif
#ifndef GLOB_NOSORT
# define GLOB_NOSORT 0
#endif
#ifndef GLOB_NOCHECK
# define GLOB_NOCHECK 0
#endif
#ifndef GLOB_NOESCAPE
# define GLOB_NOESCAPE 0
#endif
#ifndef GLOB_ERR
# define GLOB_ERR 0
#endif

/* This is used for checking validity of passed flags (passing invalid flags causes segfault in glob()!! */
#define GLOB_AVAILABLE_FLAGS (0 | GLOB_BRACE | GLOB_MARK | GLOB_NOSORT | GLOB_NOCHECK | GLOB_NOESCAPE | GLOB_ERR | GLOB_ONLYDIR)

#endif /* }}} */

int php_zip_glob(char *pattern, int pattern_len, long flags, zval *return_value TSRMLS_DC) /* {{{ */
{
#ifdef HAVE_GLOB
	char cwd[MAXPATHLEN];
	int cwd_skip = 0;
#ifdef ZTS
	char work_pattern[MAXPATHLEN];
	char *result;
#endif
	glob_t globbuf;
	int n;
	int ret;

	if (pattern_len >= MAXPATHLEN) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Pattern exceeds the maximum allowed length of %d characters", MAXPATHLEN);
		return -1;
	}

	if ((GLOB_AVAILABLE_FLAGS & flags) != flags) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "At least one of the passed flags is invalid or not supported on this platform");
		return -1;
	}

#ifdef ZTS
	if (!IS_ABSOLUTE_PATH(pattern, pattern_len)) {
		result = VCWD_GETCWD(cwd, MAXPATHLEN);
		if (!result) {
			cwd[0] = '\0';
		}
#ifdef PHP_WIN32
		if (IS_SLASH(*pattern)) {
			cwd[2] = '\0';
		}
#endif
		cwd_skip = strlen(cwd)+1;

		snprintf(work_pattern, MAXPATHLEN, "%s%c%s", cwd, DEFAULT_SLASH, pattern);
		pattern = work_pattern;
	}
#endif

	globbuf.gl_offs = 0;
	if (0 != (ret = glob(pattern, flags & GLOB_FLAGMASK, NULL, &globbuf))) {
#ifdef GLOB_NOMATCH
		if (GLOB_NOMATCH == ret) {
			/* Some glob implementation simply return no data if no matches
			   were found, others return the GLOB_NOMATCH error code.
			   We don't want to treat GLOB_NOMATCH as an error condition
			   so that PHP glob() behaves the same on both types of
			   implementations and so that 'foreach (glob() as ...'
			   can be used for simple glob() calls without further error
			   checking.
			*/
			array_init(return_value);
			return 0;
		}
#endif
		return 0;
	}

	/* now catch the FreeBSD style of "no matches" */
	if (!globbuf.gl_pathc || !globbuf.gl_pathv) {
		array_init(return_value);
		return 0;
	}

	/* we assume that any glob pattern will match files from one directory only
	   so checking the dirname of the first match should be sufficient */
	strncpy(cwd, globbuf.gl_pathv[0], MAXPATHLEN);
	if (ZIP_OPENBASEDIR_CHECKPATH(cwd)) {
		return -1;
	}

	array_init(return_value);
	for (n = 0; n < globbuf.gl_pathc; n++) {
		/* we need to do this everytime since GLOB_ONLYDIR does not guarantee that
		 * all directories will be filtered. GNU libc documentation states the
		 * following:
		 * If the information about the type of the file is easily available
		 * non-directories will be rejected but no extra work will be done to
		 * determine the information for each file. I.e., the caller must still be
		 * able to filter directories out.
		 */
		if (flags & GLOB_ONLYDIR) {
			struct stat s;

			if (0 != VCWD_STAT(globbuf.gl_pathv[n], &s)) {
				continue;
			}

			if (S_IFDIR != (s.st_mode & S_IFMT)) {
				continue;
			}
		}
		add_next_index_string(return_value, globbuf.gl_pathv[n]+cwd_skip, 1);
	}

	globfree(&globbuf);
	return globbuf.gl_pathc;
#else
	php_error_docref(NULL TSRMLS_CC, E_ERROR, "Glob support is not available");
	return 0;
#endif  /* HAVE_GLOB */
}
/* }}} */

int php_zip_pcre(char *regexp, int regexp_len, char *path, int path_len, zval *return_value TSRMLS_DC) /* {{{ */
{
#ifdef ZTS
	char cwd[MAXPATHLEN];
	int cwd_skip = 0;
	char work_path[MAXPATHLEN];
	char *result;
#endif
	int files_cnt;
	char **namelist;

#ifdef ZTS
	if (!IS_ABSOLUTE_PATH(path, path_len)) {
		result = VCWD_GETCWD(cwd, MAXPATHLEN);
		if (!result) {
			cwd[0] = '\0';
		}
#ifdef PHP_WIN32
		if (IS_SLASH(*path)) {
			cwd[2] = '\0';
		}
#endif
		cwd_skip = strlen(cwd)+1;

		snprintf(work_path, MAXPATHLEN, "%s%c%s", cwd, DEFAULT_SLASH, path);
		path = work_path;
	}
#endif

	if (ZIP_OPENBASEDIR_CHECKPATH(path)) {
		return -1;
	}

	files_cnt = php_stream_scandir(path, &namelist, NULL, (void *) php_stream_dirent_alphasort);

	if (files_cnt > 0) {
		pcre       *re = NULL;
		pcre_extra *pcre_extra = NULL;
		int preg_options = 0, i;

		re = pcre_get_compiled_regex(regexp, &pcre_extra, &preg_options TSRMLS_CC);
		if (!re) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid expression");
			return -1;
		}

		array_init(return_value);

		/* only the files, directories are ignored */
		for (i = 0; i < files_cnt; i++) {
			struct stat s;
			char   fullpath[MAXPATHLEN];
			int    ovector[3];
			int    matches;
			int    namelist_len = strlen(namelist[i]);


			if ((namelist_len == 1 && namelist[i][0] == '.') ||
				(namelist_len == 2 && namelist[i][0] == '.' && namelist[i][1] == '.')) {
				efree(namelist[i]);
				continue;
			}

			if ((path_len + namelist_len + 1) >= MAXPATHLEN) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "add_path string too long (max: %i, %i given)",
						MAXPATHLEN - 1, (path_len + namelist_len + 1));
				efree(namelist[i]);
				break;
			}

			snprintf(fullpath, MAXPATHLEN, "%s%c%s", path, DEFAULT_SLASH, namelist[i]);

			if (0 != VCWD_STAT(fullpath, &s)) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot read <%s>", fullpath);
				efree(namelist[i]);
				continue;
			}

			if (S_IFDIR == (s.st_mode & S_IFMT)) {
				efree(namelist[i]);
				continue;
			}

			matches = pcre_exec(re, NULL, namelist[i], strlen(namelist[i]), 0, 0, ovector, 3);
			/* 0 means that the vector is too small to hold all the captured substring offsets */
			if (matches < 0) {
				efree(namelist[i]);
				continue;
			}

			add_next_index_string(return_value, fullpath, 1);
			efree(namelist[i]);
		}
		efree(namelist);
	}
	return files_cnt;
}
/* }}} */

#endif

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_zip_open, 0, 0, 1)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_zip_close, 0, 0, 1)
	ZEND_ARG_INFO(0, zip)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_zip_read, 0, 0, 1)
	ZEND_ARG_INFO(0, zip)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_zip_entry_open, 0, 0, 2)
	ZEND_ARG_INFO(0, zip_dp)
	ZEND_ARG_INFO(0, zip_entry)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_zip_entry_close, 0, 0, 1)
	ZEND_ARG_INFO(0, zip_ent)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_zip_entry_read, 0, 0, 1)
	ZEND_ARG_INFO(0, zip_entry)
	ZEND_ARG_INFO(0, len)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_zip_entry_name, 0, 0, 1)
	ZEND_ARG_INFO(0, zip_entry)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_zip_entry_compressedsize, 0, 0, 1)
	ZEND_ARG_INFO(0, zip_entry)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_zip_entry_filesize, 0, 0, 1)
	ZEND_ARG_INFO(0, zip_entry)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_zip_entry_compressionmethod, 0, 0, 1)
	ZEND_ARG_INFO(0, zip_entry)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ zend_function_entry */
static const zend_function_entry zip_functions[] = {
	ZEND_RAW_FENTRY("zip_open", zif_zip_open, arginfo_zip_open, 0)
	ZEND_RAW_FENTRY("zip_close", zif_zip_close, arginfo_zip_close, 0)
	ZEND_RAW_FENTRY("zip_read", zif_zip_read, arginfo_zip_read, 0)
	PHP_FE(zip_entry_open,		arginfo_zip_entry_open)
	PHP_FE(zip_entry_close,		arginfo_zip_entry_close)
	PHP_FE(zip_entry_read,		arginfo_zip_entry_read)
	PHP_FE(zip_entry_filesize,	arginfo_zip_entry_filesize)
	PHP_FE(zip_entry_name,		arginfo_zip_entry_name)
	PHP_FE(zip_entry_compressedsize,		arginfo_zip_entry_compressedsize)
	PHP_FE(zip_entry_compressionmethod,		arginfo_zip_entry_compressionmethod)
	PHP_FE_END
};
/* }}} */

/* {{{ ZE2 OO definitions */
#ifdef PHP_ZIP_USE_OO
static zend_class_entry *zip_class_entry;
static zend_object_handlers zip_object_handlers;

static HashTable zip_prop_handlers;

typedef int (*zip_read_int_t)(struct zip *za TSRMLS_DC);
typedef char *(*zip_read_const_char_t)(struct zip *za, int *len TSRMLS_DC);
typedef char *(*zip_read_const_char_from_ze_t)(ze_zip_object *obj TSRMLS_DC);

typedef struct _zip_prop_handler {
	zip_read_int_t read_int_func;
	zip_read_const_char_t read_const_char_func;
	zip_read_const_char_from_ze_t read_const_char_from_obj_func;

	int type;
} zip_prop_handler;
#endif
/* }}} */

#ifdef PHP_ZIP_USE_OO
static void php_zip_register_prop_handler(HashTable *prop_handler, char *name, zip_read_int_t read_int_func, zip_read_const_char_t read_char_func, zip_read_const_char_from_ze_t read_char_from_obj_func, int rettype TSRMLS_DC) /* {{{ */
{
	zip_prop_handler hnd;

	hnd.read_const_char_func = read_char_func;
	hnd.read_int_func = read_int_func;
	hnd.read_const_char_from_obj_func = read_char_from_obj_func;
	hnd.type = rettype;
	zend_hash_add(prop_handler, name, strlen(name)+1, &hnd, sizeof(zip_prop_handler), NULL);
}
/* }}} */

static int php_zip_property_reader(ze_zip_object *obj, zip_prop_handler *hnd, zval **retval, int newzval TSRMLS_DC) /* {{{ */
{
	const char *retchar = NULL;
	int retint = 0;
	int len = 0;

	if (obj && obj->za != NULL) {
		if (hnd->read_const_char_func) {
			retchar = hnd->read_const_char_func(obj->za, &len TSRMLS_CC);
		} else {
			if (hnd->read_int_func) {
				retint = hnd->read_int_func(obj->za TSRMLS_CC);
				if (retint == -1) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Internal zip error returned");
					return FAILURE;
				}
			} else {
				if (hnd->read_const_char_from_obj_func) {
					retchar = hnd->read_const_char_from_obj_func(obj TSRMLS_CC);
					len = strlen(retchar);
				}
			}
		}
	}

	if (newzval) {
		ALLOC_ZVAL(*retval);
	}

	switch (hnd->type) {
		case IS_STRING:
			if (retchar) {
				ZVAL_STRINGL(*retval, (char *) retchar, len, 1);
			} else {
				ZVAL_EMPTY_STRING(*retval);
			}
			break;
		case IS_BOOL:
			ZVAL_BOOL(*retval, (long)retint);
			break;
		case IS_LONG:
			ZVAL_LONG(*retval, (long)retint);
			break;
		default:
			ZVAL_NULL(*retval);
	}

	return SUCCESS;
}
/* }}} */

static zval **php_zip_get_property_ptr_ptr(zval *object, zval *member, int type, const zend_literal *key TSRMLS_DC) /* {{{ */
{
	ze_zip_object *obj;
	zval tmp_member;
	zval **retval = NULL;

	zip_prop_handler *hnd;
	zend_object_handlers *std_hnd;
	int ret;

	if (member->type != IS_STRING) {
		tmp_member = *member;
		zval_copy_ctor(&tmp_member);
		convert_to_string(&tmp_member);
		member = &tmp_member;
		key = NULL;
	}

	ret = FAILURE;
	obj = (ze_zip_object *)zend_objects_get_address(object TSRMLS_CC);

	if (obj->prop_handler != NULL) {
		if (key) {
			ret = zend_hash_quick_find(obj->prop_handler, Z_STRVAL_P(member), Z_STRLEN_P(member)+1, key->hash_value, (void **) &hnd);
		} else {
			ret = zend_hash_find(obj->prop_handler, Z_STRVAL_P(member), Z_STRLEN_P(member)+1, (void **) &hnd);
		}
	}


	if (ret == FAILURE) {
		std_hnd = zend_get_std_object_handlers();
		retval = std_hnd->get_property_ptr_ptr(object, member, type, key TSRMLS_CC);
	}

	if (member == &tmp_member) {
		zval_dtor(member);
	}
	return retval;
}
/* }}} */

static zval* php_zip_read_property(zval *object, zval *member, int type, const zend_literal *key TSRMLS_DC) /* {{{ */
{
	ze_zip_object *obj;
	zval tmp_member;
	zval *retval;
	zip_prop_handler *hnd;
	zend_object_handlers *std_hnd;
	int ret;

	if (member->type != IS_STRING) {
		tmp_member = *member;
		zval_copy_ctor(&tmp_member);
		convert_to_string(&tmp_member);
		member = &tmp_member;
		key = NULL;
	}

	ret = FAILURE;
	obj = (ze_zip_object *)zend_objects_get_address(object TSRMLS_CC);

	if (obj->prop_handler != NULL) {
		if (key) {
			ret = zend_hash_quick_find(obj->prop_handler, Z_STRVAL_P(member), Z_STRLEN_P(member)+1, key->hash_value, (void **) &hnd);
		} else {
			ret = zend_hash_find(obj->prop_handler, Z_STRVAL_P(member), Z_STRLEN_P(member)+1, (void **) &hnd);
		}
	}

	if (ret == SUCCESS) {
		ret = php_zip_property_reader(obj, hnd, &retval, 1 TSRMLS_CC);
		if (ret == SUCCESS) {
			/* ensure we're creating a temporary variable */
			Z_SET_REFCOUNT_P(retval, 0);
		} else {
			retval = EG(uninitialized_zval_ptr);
		}
	} else {
		std_hnd = zend_get_std_object_handlers();
		retval = std_hnd->read_property(object, member, type, key TSRMLS_CC);
	}

	if (member == &tmp_member) {
		zval_dtor(member);
	}
	return retval;
}
/* }}} */

static int php_zip_has_property(zval *object, zval *member, int type, const zend_literal *key TSRMLS_DC) /* {{{ */
{
	ze_zip_object *obj;
	zval tmp_member;
	zip_prop_handler *hnd;
	zend_object_handlers *std_hnd;
	int ret, retval = 0;

	if (member->type != IS_STRING) {
		tmp_member = *member;
		zval_copy_ctor(&tmp_member);
		convert_to_string(&tmp_member);
		member = &tmp_member;
		key = NULL;
	}

	ret = FAILURE;
	obj = (ze_zip_object *)zend_objects_get_address(object TSRMLS_CC);

	if (obj->prop_handler != NULL) {
		if (key) {
			ret = zend_hash_quick_find(obj->prop_handler, Z_STRVAL_P(member), Z_STRLEN_P(member)+1, key->hash_value, (void **) &hnd);
		} else {
			ret = zend_hash_find(obj->prop_handler, Z_STRVAL_P(member), Z_STRLEN_P(member)+1, (void **) &hnd);
		}
	}

	if (ret == SUCCESS) {
		zval *tmp;
		ALLOC_INIT_ZVAL(tmp);

		if (type == 2) {
			retval = 1;
		} else if (php_zip_property_reader(obj, hnd, &tmp, 0 TSRMLS_CC) == SUCCESS) {
			Z_SET_REFCOUNT_P(tmp, 1);
			Z_UNSET_ISREF_P(tmp);
			if (type == 1) {
				retval = zend_is_true(tmp);
			} else if (type == 0) {
				retval = (Z_TYPE_P(tmp) != IS_NULL);
			}
		}

		zval_ptr_dtor(&tmp);
	} else {
		std_hnd = zend_get_std_object_handlers();
		retval = std_hnd->has_property(object, member, type, key TSRMLS_CC);
	}

	if (member == &tmp_member) {
		zval_dtor(member);
	}
	return retval;
}
/* }}} */

static HashTable *php_zip_get_properties(zval *object TSRMLS_DC)/* {{{ */
{
	ze_zip_object *obj;
	zip_prop_handler *hnd;
	HashTable *props;
	zval *val;
	int ret;
	char *key;
	uint key_len;
	HashPosition pos;
	ulong num_key;

	obj = (ze_zip_object *)zend_objects_get_address(object TSRMLS_CC);
	props = zend_std_get_properties(object TSRMLS_CC);

	if (obj->prop_handler == NULL) {
		return NULL;
	}
	zend_hash_internal_pointer_reset_ex(obj->prop_handler, &pos);

	while (zend_hash_get_current_data_ex(obj->prop_handler, (void**)&hnd, &pos) == SUCCESS) {
		zend_hash_get_current_key_ex(obj->prop_handler, &key, &key_len, &num_key, 0, &pos);
		MAKE_STD_ZVAL(val);
		ret = php_zip_property_reader(obj, hnd, &val, 0 TSRMLS_CC);
		if (ret != SUCCESS) {
			val = EG(uninitialized_zval_ptr);
		}
		zend_hash_update(props, key, key_len, (void *)&val, sizeof(zval *), NULL);
		zend_hash_move_forward_ex(obj->prop_handler, &pos);
	}
	return props;
}
/* }}} */

static void php_zip_object_free_storage(void *object TSRMLS_DC) /* {{{ */
{
	ze_zip_object * intern = (ze_zip_object *) object;
	int i;

	if (!intern) {
		return;
	}
	if (intern->za) {
		if (zip_close(intern->za) != 0) {
			_zip_free(intern->za);
		}
		intern->za = NULL;
	}

	if (intern->buffers_cnt>0) {
		for (i=0; i<intern->buffers_cnt; i++) {
			efree(intern->buffers[i]);
		}
		efree(intern->buffers);
	}

	intern->za = NULL;

#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION == 1 && PHP_RELEASE_VERSION > 2) || (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 1) || (PHP_MAJOR_VERSION > 5)
	zend_object_std_dtor(&intern->zo TSRMLS_CC);
#else
	if (intern->zo.guards) {
		zend_hash_destroy(intern->zo.guards);
		FREE_HASHTABLE(intern->zo.guards);
	}

	if (intern->zo.properties) {
		zend_hash_destroy(intern->zo.properties);
		FREE_HASHTABLE(intern->zo.properties);
	}
#endif

	if (intern->filename) {
		efree(intern->filename);
	}
	efree(intern);
}
/* }}} */

static zend_object_value php_zip_object_new(zend_class_entry *class_type TSRMLS_DC) /* {{{ */
{
	ze_zip_object *intern;
	zend_object_value retval;

	intern = emalloc(sizeof(ze_zip_object));
	memset(&intern->zo, 0, sizeof(zend_object));

	intern->za = NULL;
	intern->buffers = NULL;
	intern->filename = NULL;
	intern->buffers_cnt = 0;
	intern->prop_handler = &zip_prop_handlers;

#if ((PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 1) || (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION == 1 && PHP_RELEASE_VERSION > 2))
	zend_object_std_init(&intern->zo, class_type TSRMLS_CC);
#else
	ALLOC_HASHTABLE(intern->zo.properties);
  	zend_hash_init(intern->zo.properties, 0, NULL, ZVAL_PTR_DTOR, 0);
	intern->zo.ce = class_type;
#endif

	object_properties_init(&intern->zo, class_type);

	retval.handle = zend_objects_store_put(intern,
						NULL,
						(zend_objects_free_object_storage_t) php_zip_object_free_storage,
						NULL TSRMLS_CC);

	retval.handlers = (zend_object_handlers *) & zip_object_handlers;

	return retval;
}
/* }}} */
#endif

/* {{{ Resource dtors */

/* {{{ php_zip_free_dir */
static void php_zip_free_dir(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	zip_rsrc * zip_int = (zip_rsrc *) rsrc->ptr;

	if (zip_int) {
		if (zip_int->za) {
			if (zip_close(zip_int->za) != 0) {
				_zip_free(zip_int->za);
			}
			zip_int->za = NULL;
		}

		efree(rsrc->ptr);

		rsrc->ptr = NULL;
	}
}
/* }}} */

/* {{{ php_zip_free_entry */
static void php_zip_free_entry(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	zip_read_rsrc *zr_rsrc = (zip_read_rsrc *) rsrc->ptr;

	if (zr_rsrc) {
		if (zr_rsrc->zf) {
			if (zr_rsrc->zf->za) {
				zip_fclose(zr_rsrc->zf);
			} else {
				if (zr_rsrc->zf->src)
					zip_source_free(zr_rsrc->zf->src);
				free(zr_rsrc->zf);
			}
			zr_rsrc->zf = NULL;
		}
		efree(zr_rsrc);
		rsrc->ptr = NULL;
	}
}
/* }}} */

/* }}}*/

/* reset macro */

/* {{{ function prototypes */
static PHP_MINIT_FUNCTION(zip);
static PHP_MSHUTDOWN_FUNCTION(zip);
static PHP_MINFO_FUNCTION(zip);
/* }}} */

/* {{{ zip_module_entry
 */
zend_module_entry zip_module_entry = {
	STANDARD_MODULE_HEADER,
	"zip",
	zip_functions,
	PHP_MINIT(zip),
	PHP_MSHUTDOWN(zip),
	NULL,
	NULL,
	PHP_MINFO(zip),
	PHP_ZIP_VERSION_STRING,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_ZIP
ZEND_GET_MODULE(zip)
#endif
/* set macro */

/* {{{ proto resource zip_open(string filename)
Create new zip using source uri for output */
static PHP_NAMED_FUNCTION(zif_zip_open)
{
	char     *filename;
	int       filename_len;
	char resolved_path[MAXPATHLEN + 1];
	zip_rsrc *rsrc_int;
	int err = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "p", &filename, &filename_len) == FAILURE) {
		return;
	}

	if (filename_len == 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Empty string as source");
		RETURN_FALSE;
	}

	if (ZIP_OPENBASEDIR_CHECKPATH(filename)) {
		RETURN_FALSE;
	}

	if(!expand_filepath(filename, resolved_path TSRMLS_CC)) {
		RETURN_FALSE;
	}

	rsrc_int = (zip_rsrc *)emalloc(sizeof(zip_rsrc));

	rsrc_int->za = zip_open(resolved_path, 0, &err);
	if (rsrc_int->za == NULL) {
		efree(rsrc_int);
		RETURN_LONG((long)err);
	}

	rsrc_int->index_current = 0;
	rsrc_int->num_files = zip_get_num_files(rsrc_int->za);

	ZEND_REGISTER_RESOURCE(return_value, rsrc_int, le_zip_dir);
}
/* }}} */

/* {{{ proto void zip_close(resource zip)
   Close a Zip archive */
static PHP_NAMED_FUNCTION(zif_zip_close)
{
	zval * zip;
	zip_rsrc *z_rsrc = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zip) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(z_rsrc, zip_rsrc *, &zip, -1, le_zip_dir_name, le_zip_dir);

	/* really close the zip will break BC :-D */
	zend_list_delete(Z_LVAL_P(zip));
}
/* }}} */

/* {{{ proto resource zip_read(resource zip)
   Returns the next file in the archive */
static PHP_NAMED_FUNCTION(zif_zip_read)
{
	zval *zip_dp;
	zip_read_rsrc *zr_rsrc;
	int ret;
	zip_rsrc *rsrc_int;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zip_dp) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(rsrc_int, zip_rsrc *, &zip_dp, -1, le_zip_dir_name, le_zip_dir);

	if (rsrc_int && rsrc_int->za) {
		if (rsrc_int->index_current >= rsrc_int->num_files) {
			RETURN_FALSE;
		}

		zr_rsrc = emalloc(sizeof(zip_read_rsrc));

		ret = zip_stat_index(rsrc_int->za, rsrc_int->index_current, 0, &zr_rsrc->sb);

		if (ret != 0) {
			efree(zr_rsrc);
			RETURN_FALSE;
		}

		zr_rsrc->zf = zip_fopen_index(rsrc_int->za, rsrc_int->index_current, 0);
		if (zr_rsrc->zf) {
			rsrc_int->index_current++;
			ZEND_REGISTER_RESOURCE(return_value, zr_rsrc, le_zip_entry);
		} else {
			efree(zr_rsrc);
			RETURN_FALSE;
		}

	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool zip_entry_open(resource zip_dp, resource zip_entry [, string mode])
   Open a Zip File, pointed by the resource entry */
/* Dummy function to follow the old API */
static PHP_NAMED_FUNCTION(zif_zip_entry_open)
{
	zval * zip;
	zval * zip_entry;
	char *mode = NULL;
	int mode_len = 0;
	zip_read_rsrc * zr_rsrc;
	zip_rsrc *z_rsrc;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr|s", &zip, &zip_entry, &mode, &mode_len) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(zr_rsrc, zip_read_rsrc *, &zip_entry, -1, le_zip_entry_name, le_zip_entry);
	ZEND_FETCH_RESOURCE(z_rsrc, zip_rsrc *, &zip, -1, le_zip_dir_name, le_zip_dir);

	if (zr_rsrc->zf != NULL) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool zip_entry_close(resource zip_ent)
   Close a zip entry */
static PHP_NAMED_FUNCTION(zif_zip_entry_close)
{
	zval * zip_entry;
	zip_read_rsrc * zr_rsrc;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zip_entry) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(zr_rsrc, zip_read_rsrc *, &zip_entry, -1, le_zip_entry_name, le_zip_entry);

	RETURN_BOOL(SUCCESS == zend_list_delete(Z_LVAL_P(zip_entry)));
}
/* }}} */

/* {{{ proto mixed zip_entry_read(resource zip_entry [, int len])
   Read from an open directory entry */
static PHP_NAMED_FUNCTION(zif_zip_entry_read)
{
	zval * zip_entry;
	long len = 0;
	zip_read_rsrc * zr_rsrc;
	char *buffer;
	int n = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|l", &zip_entry, &len) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(zr_rsrc, zip_read_rsrc *, &zip_entry, -1, le_zip_entry_name, le_zip_entry);

	if (len <= 0) {
		len = 1024;
	}

	if (zr_rsrc->zf) {
		buffer = safe_emalloc(len, 1, 1);
		n = zip_fread(zr_rsrc->zf, buffer, len);
		if (n > 0) {
			buffer[n] = 0;
			RETURN_STRINGL(buffer, n, 0);
		} else {
			efree(buffer);
			RETURN_EMPTY_STRING()
		}
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

static void php_zip_entry_get_info(INTERNAL_FUNCTION_PARAMETERS, int opt) /* {{{ */
{
	zval * zip_entry;
	zip_read_rsrc * zr_rsrc;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zip_entry) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(zr_rsrc, zip_read_rsrc *, &zip_entry, -1, le_zip_entry_name, le_zip_entry);

	if (!zr_rsrc->zf) {
		RETURN_FALSE;
	}

	switch (opt) {
		case 0:
			RETURN_STRING((char *)zr_rsrc->sb.name, 1);
			break;
		case 1:
			RETURN_LONG((long) (zr_rsrc->sb.comp_size));
			break;
		case 2:
			RETURN_LONG((long) (zr_rsrc->sb.size));
			break;
		case 3:
			switch (zr_rsrc->sb.comp_method) {
				case 0:
					RETURN_STRING("stored", 1);
					break;
				case 1:
					RETURN_STRING("shrunk", 1);
					break;
				case 2:
				case 3:
				case 4:
				case 5:
					RETURN_STRING("reduced", 1);
					break;
				case 6:
					RETURN_STRING("imploded", 1);
					break;
				case 7:
					RETURN_STRING("tokenized", 1);
					break;
				case 8:
					RETURN_STRING("deflated", 1);
					break;
				case 9:
					RETURN_STRING("deflatedX", 1);
					break;
				case 10:
					RETURN_STRING("implodedX", 1);
					break;
				default:
					RETURN_FALSE;
			}
			RETURN_LONG((long) (zr_rsrc->sb.comp_method));
			break;
	}

}
/* }}} */

/* {{{ proto string zip_entry_name(resource zip_entry)
   Return the name given a ZZip entry */
static PHP_NAMED_FUNCTION(zif_zip_entry_name)
{
	php_zip_entry_get_info(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto int zip_entry_compressedsize(resource zip_entry)
   Return the compressed size of a ZZip entry */
static PHP_NAMED_FUNCTION(zif_zip_entry_compressedsize)
{
	php_zip_entry_get_info(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto int zip_entry_filesize(resource zip_entry)
   Return the actual filesize of a ZZip entry */
static PHP_NAMED_FUNCTION(zif_zip_entry_filesize)
{
	php_zip_entry_get_info(INTERNAL_FUNCTION_PARAM_PASSTHRU, 2);
}
/* }}} */

/* {{{ proto string zip_entry_compressionmethod(resource zip_entry)
   Return a string containing the compression method used on a particular entry */
static PHP_NAMED_FUNCTION(zif_zip_entry_compressionmethod)
{
	php_zip_entry_get_info(INTERNAL_FUNCTION_PARAM_PASSTHRU, 3);
}
/* }}} */

#ifdef PHP_ZIP_USE_OO
/* {{{ proto mixed ZipArchive::open(string source [, int flags])
Create new zip using source uri for output, return TRUE on success or the error code */
static ZIPARCHIVE_METHOD(open)
{
	struct zip *intern;
	char *filename;
	int filename_len;
	int err = 0;
	long flags = 0;
	char resolved_path[MAXPATHLEN];

	zval *this = getThis();
	ze_zip_object *ze_obj = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "p|l", &filename, &filename_len, &flags) == FAILURE) {
		return;
	}

	if (this) {
		/* We do not use ZIP_FROM_OBJECT, zip init function here */
		ze_obj = (ze_zip_object*) zend_object_store_get_object(this TSRMLS_CC);
	}

	if (filename_len == 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Empty string as source");
		RETURN_FALSE;
	}

	if (ZIP_OPENBASEDIR_CHECKPATH(filename)) {
		RETURN_FALSE;
	}

	if (!expand_filepath(filename, resolved_path TSRMLS_CC)) {
		RETURN_FALSE;
	}

	if (ze_obj->za) {
		/* we already have an opened zip, free it */
		if (zip_close(ze_obj->za) != 0) {
			_zip_free(ze_obj->za);
		}
		ze_obj->za = NULL;
	}
	if (ze_obj->filename) {
		efree(ze_obj->filename);
		ze_obj->filename = NULL;
	}

	intern = zip_open(resolved_path, flags, &err);
	if (!intern || err) {
		RETURN_LONG((long)err);
	}
	ze_obj->filename = estrdup(resolved_path);
	ze_obj->filename_len = strlen(resolved_path);
	ze_obj->za = intern;
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool ZipArchive::close()
close the zip archive */
static ZIPARCHIVE_METHOD(close)
{
	struct zip *intern;
	zval *this = getThis();
	ze_zip_object *ze_obj;

	if (!this) {
			RETURN_FALSE;
	}

	ZIP_FROM_OBJECT(intern, this);

	ze_obj = (ze_zip_object*) zend_object_store_get_object(this TSRMLS_CC);

	if (zip_close(intern)) {
		RETURN_FALSE;
	}

	efree(ze_obj->filename);
	ze_obj->filename = NULL;
	ze_obj->filename_len = 0;
	ze_obj->za = NULL;

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto string ZipArchive::getStatusString()
 * Returns the status error message, system and/or zip messages */
static ZIPARCHIVE_METHOD(getStatusString)
{
	struct zip *intern;
	zval *this = getThis();
	int zep, syp, len;
	char error_string[128];

	if (!this) {
		RETURN_FALSE;
	}

	ZIP_FROM_OBJECT(intern, this);

	zip_error_get(intern, &zep, &syp);

	len = zip_error_to_str(error_string, 128, zep, syp);
	RETVAL_STRINGL(error_string, len, 1);
}
/* }}} */

/* {{{ proto bool ZipArchive::createEmptyDir(string dirname)
Returns the index of the entry named filename in the archive */
static ZIPARCHIVE_METHOD(addEmptyDir)
{
	struct zip *intern;
	zval *this = getThis();
	char *dirname;
	int   dirname_len;
	int idx;
	struct zip_stat sb;
	char *s;

	if (!this) {
		RETURN_FALSE;
	}

	ZIP_FROM_OBJECT(intern, this);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
				&dirname, &dirname_len) == FAILURE) {
		return;
	}

	if (dirname_len<1) {
		RETURN_FALSE;
	}

	if (dirname[dirname_len-1] != '/') {
		s=(char *)emalloc(dirname_len+2);
		strcpy(s, dirname);
		s[dirname_len] = '/';
		s[dirname_len+1] = '\0';
	} else {
		s = dirname;
	}

	idx = zip_stat(intern, s, 0, &sb);
	if (idx >= 0) {
		RETVAL_FALSE;
	} else {
		if (zip_add_dir(intern, (const char *)s) == -1) {
			RETVAL_FALSE;
		}
		RETVAL_TRUE;
	}

	if (s != dirname) {
		efree(s);
	}
}
/* }}} */

static void php_zip_add_from_pattern(INTERNAL_FUNCTION_PARAMETERS, int type) /* {{{ */
{
	struct zip *intern;
	zval *this = getThis();
	char *pattern;
	char *path = NULL;
	char *remove_path = NULL;
	char *add_path = NULL;
	int pattern_len, add_path_len = 0, remove_path_len = 0, path_len = 0;
	long remove_all_path = 0;
	long flags = 0;
	zval *options = NULL;
	int found;

	if (!this) {
		RETURN_FALSE;
	}

	ZIP_FROM_OBJECT(intern, this);
	/* 1 == glob, 2==pcre */
	if (type == 1) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "p|la",
					&pattern, &pattern_len, &flags, &options) == FAILURE) {
			return;
		}
	} else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "p|sa",
					&pattern, &pattern_len, &path, &path_len, &options) == FAILURE) {
			return;
		}
	}

	if (pattern_len == 0) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Empty string as pattern");
		RETURN_FALSE;
	}
	if (options && (php_zip_parse_options(options, &remove_all_path, &remove_path, &remove_path_len,
			&add_path, &add_path_len TSRMLS_CC) < 0)) {
		RETURN_FALSE;
	}

	if (remove_path && remove_path_len > 1 && (remove_path[strlen(remove_path) - 1] == '/' ||
		remove_path[strlen(remove_path) - 1] == '\\')) {
		remove_path[strlen(remove_path) - 1] = '\0';
	}

	if (type == 1) {
		found = php_zip_glob(pattern, pattern_len, flags, return_value TSRMLS_CC);
	} else {
		found = php_zip_pcre(pattern, pattern_len, path, path_len, return_value TSRMLS_CC);
	}

	if (found > 0) {
		int i;
		zval **zval_file = NULL;

		for (i = 0; i < found; i++) {
			char *file, *file_stripped, *entry_name;
			size_t entry_name_len, file_stripped_len;
			char entry_name_buf[MAXPATHLEN];
			char *basename = NULL;

			if (zend_hash_index_find(Z_ARRVAL_P(return_value), i, (void **) &zval_file) == SUCCESS) {
				file = Z_STRVAL_PP(zval_file);
				if (remove_all_path) {
					php_basename(Z_STRVAL_PP(zval_file), Z_STRLEN_PP(zval_file), NULL, 0,
									&basename, (size_t *)&file_stripped_len TSRMLS_CC);
					file_stripped = basename;
				} else if (remove_path && strstr(Z_STRVAL_PP(zval_file), remove_path) != NULL) {
					file_stripped = Z_STRVAL_PP(zval_file) + remove_path_len + 1;
					file_stripped_len = Z_STRLEN_PP(zval_file) - remove_path_len - 1;
				} else {
					file_stripped = Z_STRVAL_PP(zval_file);
					file_stripped_len = Z_STRLEN_PP(zval_file);
				}

				if (add_path) {
					if ((add_path_len + file_stripped_len) > MAXPATHLEN) {
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Entry name too long (max: %d, %ld given)",
						MAXPATHLEN - 1, (add_path_len + file_stripped_len));
						zval_dtor(return_value);
						RETURN_FALSE;
					}

					snprintf(entry_name_buf, MAXPATHLEN, "%s%s", add_path, file_stripped);
					entry_name = entry_name_buf;
					entry_name_len = strlen(entry_name);
				} else {
					entry_name = Z_STRVAL_PP(zval_file);
					entry_name_len = Z_STRLEN_PP(zval_file);
				}
				if (basename) {
					efree(basename);
					basename = NULL;
				}
				if (php_zip_add_file(intern, Z_STRVAL_PP(zval_file), Z_STRLEN_PP(zval_file),
					entry_name, entry_name_len, 0, 0 TSRMLS_CC) < 0) {
					zval_dtor(return_value);
					RETURN_FALSE;
				}
			}
		}
	}
}
/* }}} */

/* {{{ proto bool ZipArchive::addGlob(string pattern[,int flags [, array options]])
Add files matching the glob pattern. See php's glob for the pattern syntax. */
static ZIPARCHIVE_METHOD(addGlob)
{
	php_zip_add_from_pattern(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto bool ZipArchive::addPattern(string pattern[, string path [, array options]])
Add files matching the pcre pattern. See php's pcre for the pattern syntax. */
static ZIPARCHIVE_METHOD(addPattern)
{
	php_zip_add_from_pattern(INTERNAL_FUNCTION_PARAM_PASSTHRU, 2);
}
/* }}} */

/* {{{ proto bool ZipArchive::addFile(string filepath[, string entryname[, int start [, int length]]])
Add a file in a Zip archive using its path and the name to use. */
static ZIPARCHIVE_METHOD(addFile)
{
	struct zip *intern;
	zval *this = getThis();
	char *filename;
	int filename_len;
	char *entry_name = NULL;
	int entry_name_len = 0;
	long offset_start = 0, offset_len = 0;

	if (!this) {
		RETURN_FALSE;
	}

	ZIP_FROM_OBJECT(intern, this);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "p|sll",
			&filename, &filename_len, &entry_name, &entry_name_len, &offset_start, &offset_len) == FAILURE) {
		return;
	}

	if (filename_len == 0) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Empty string as filename");
		RETURN_FALSE;
	}

	if (entry_name_len == 0) {
		entry_name = filename;
		entry_name_len = filename_len;
	}

	if (php_zip_add_file(intern, filename, filename_len,
		entry_name, entry_name_len, 0, 0 TSRMLS_CC) < 0) {
		RETURN_FALSE;
	} else {
		RETURN_TRUE;
	}
}
/* }}} */

/* {{{ proto bool ZipArchive::addFromString(string name, string content)
Add a file using content and the entry name */
static ZIPARCHIVE_METHOD(addFromString)
{
	struct zip *intern;
	zval *this = getThis();
	char *buffer, *name;
	int buffer_len, name_len;
	ze_zip_object *ze_obj;
	struct zip_source *zs;
	int pos = 0;
	int cur_idx;

	if (!this) {
		RETURN_FALSE;
	}

	ZIP_FROM_OBJECT(intern, this);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
			&name, &name_len, &buffer, &buffer_len) == FAILURE) {
		return;
	}

	ze_obj = (ze_zip_object*) zend_object_store_get_object(this TSRMLS_CC);
	if (ze_obj->buffers_cnt) {
		ze_obj->buffers = (char **)erealloc(ze_obj->buffers, sizeof(char *) * (ze_obj->buffers_cnt+1));
		pos = ze_obj->buffers_cnt++;
	} else {
		ze_obj->buffers = (char **)emalloc(sizeof(char *));
		ze_obj->buffers_cnt++;
		pos = 0;
	}
	ze_obj->buffers[pos] = (char *)emalloc(buffer_len + 1);
	memcpy(ze_obj->buffers[pos], buffer, buffer_len + 1);

	zs = zip_source_buffer(intern, ze_obj->buffers[pos], buffer_len, 0);

	if (zs == NULL) {
		RETURN_FALSE;
	}

	cur_idx = zip_name_locate(intern, (const char *)name, 0);
	/* TODO: fix  _zip_replace */
	if (cur_idx >= 0) {
		if (zip_delete(intern, cur_idx) == -1) {
			goto fail;
		}
	}

	if (zip_add(intern, name, zs) != -1) {
		RETURN_TRUE;
	}
fail:
	zip_source_free(zs);
	RETURN_FALSE;	
}
/* }}} */

/* {{{ proto array ZipArchive::statName(string filename[, int flags])
Returns the information about a the zip entry filename */
static ZIPARCHIVE_METHOD(statName)
{
	struct zip *intern;
	zval *this = getThis();
	char *name;
	int name_len;
	long flags = 0;
	struct zip_stat sb;

	if (!this) {
		RETURN_FALSE;
	}

	ZIP_FROM_OBJECT(intern, this);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "p|l",
			&name, &name_len, &flags) == FAILURE) {
		return;
	}

	PHP_ZIP_STAT_PATH(intern, name, name_len, flags, sb);

	RETURN_SB(&sb);
}
/* }}} */

/* {{{ proto resource ZipArchive::statIndex(int index[, int flags])
Returns the zip entry informations using its index */
static ZIPARCHIVE_METHOD(statIndex)
{
	struct zip *intern;
	zval *this = getThis();
	long index, flags = 0;

	struct zip_stat sb;

	if (!this) {
		RETURN_FALSE;
	}

	ZIP_FROM_OBJECT(intern, this);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|l",
			&index, &flags) == FAILURE) {
		return;
	}

	if (zip_stat_index(intern, index, flags, &sb) != 0) {
		RETURN_FALSE;
	}
	RETURN_SB(&sb);
}
/* }}} */

/* {{{ proto int ZipArchive::locateName(string filename[, int flags])
Returns the index of the entry named filename in the archive */
static ZIPARCHIVE_METHOD(locateName)
{
	struct zip *intern;
	zval *this = getThis();
	char *name;
	int name_len;
	long flags = 0;
	long idx = -1;

	if (!this) {
		RETURN_FALSE;
	}

	ZIP_FROM_OBJECT(intern, this);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "p|l",
			&name, &name_len, &flags) == FAILURE) {
		return;
	}
	if (name_len<1) {
		RETURN_FALSE;
	}

	idx = (long)zip_name_locate(intern, (const char *)name, flags);

	if (idx >= 0) {
		RETURN_LONG(idx);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto string ZipArchive::getNameIndex(int index [, int flags])
Returns the name of the file at position index */
static ZIPARCHIVE_METHOD(getNameIndex)
{
	struct zip *intern;
	zval *this = getThis();
	const char *name;
	long flags = 0, index = 0;

	if (!this) {
		RETURN_FALSE;
	}

	ZIP_FROM_OBJECT(intern, this);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|l",
			&index, &flags) == FAILURE) {
		return;
	}

	name = zip_get_name(intern, (int) index, flags);

	if (name) {
		RETVAL_STRING((char *)name, 1);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool ZipArchive::setArchiveComment(string comment)
Set or remove (NULL/'') the comment of the archive */
static ZIPARCHIVE_METHOD(setArchiveComment)
{
	struct zip *intern;
	zval *this = getThis();
	int comment_len;
	char * comment;

	if (!this) {
		RETURN_FALSE;
	}

	ZIP_FROM_OBJECT(intern, this);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &comment, &comment_len) == FAILURE) {
		return;
	}
	if (zip_set_archive_comment(intern, (const char *)comment, (int)comment_len)) {
		RETURN_FALSE;
	} else {
		RETURN_TRUE;
	}
}
/* }}} */

/* {{{ proto string ZipArchive::getArchiveComment([int flags])
Returns the comment of an entry using its index */
static ZIPARCHIVE_METHOD(getArchiveComment)
{
	struct zip *intern;
	zval *this = getThis();
	long flags = 0;
	const char * comment;
	int comment_len = 0;

	if (!this) {
		RETURN_FALSE;
	}

	ZIP_FROM_OBJECT(intern, this);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &flags) == FAILURE) {
		return;
	}

	comment = zip_get_archive_comment(intern, &comment_len, (int)flags);
	if(comment==NULL) {
		RETURN_FALSE;
	}
	RETURN_STRINGL((char *)comment, (long)comment_len, 1);
}
/* }}} */

/* {{{ proto bool ZipArchive::setCommentName(string name, string comment)
Set or remove (NULL/'') the comment of an entry using its Name */
static ZIPARCHIVE_METHOD(setCommentName)
{
	struct zip *intern;
	zval *this = getThis();
	int comment_len, name_len;
	char * comment, *name;
	int idx;

	if (!this) {
		RETURN_FALSE;
	}

	ZIP_FROM_OBJECT(intern, this);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
			&name, &name_len, &comment, &comment_len) == FAILURE) {
		return;
	}

	if (name_len < 1) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Empty string as entry name");
	}

	idx = zip_name_locate(intern, name, 0);
	if (idx < 0) {
		RETURN_FALSE;
	}
	PHP_ZIP_SET_FILE_COMMENT(intern, idx, comment, comment_len);
}
/* }}} */

/* {{{ proto bool ZipArchive::setCommentIndex(int index, string comment)
Set or remove (NULL/'') the comment of an entry using its index */
static ZIPARCHIVE_METHOD(setCommentIndex)
{
	struct zip *intern;
	zval *this = getThis();
	long index;
	int comment_len;
	char * comment;
	struct zip_stat sb;

	if (!this) {
		RETURN_FALSE;
	}

	ZIP_FROM_OBJECT(intern, this);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls",
			&index, &comment, &comment_len) == FAILURE) {
		return;
	}

	PHP_ZIP_STAT_INDEX(intern, index, 0, sb);
	PHP_ZIP_SET_FILE_COMMENT(intern, index, comment, comment_len);
}
/* }}} */

/* {{{ proto string ZipArchive::getCommentName(string name[, int flags])
Returns the comment of an entry using its name */
static ZIPARCHIVE_METHOD(getCommentName)
{
	struct zip *intern;
	zval *this = getThis();
	int name_len, idx;
	long flags = 0;
	int comment_len = 0;
	const char * comment;
	char *name;

	if (!this) {
		RETURN_FALSE;
	}

	ZIP_FROM_OBJECT(intern, this);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l",
			&name, &name_len, &flags) == FAILURE) {
		return;
	}
	if (name_len < 1) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Empty string as entry name");
		RETURN_FALSE;
	}

	idx = zip_name_locate(intern, name, 0);
	if (idx < 0) {
		RETURN_FALSE;
	}

	comment = zip_get_file_comment(intern, idx, &comment_len, (int)flags);
	RETURN_STRINGL((char *)comment, (long)comment_len, 1);
}
/* }}} */

/* {{{ proto string ZipArchive::getCommentIndex(int index[, int flags])
Returns the comment of an entry using its index */
static ZIPARCHIVE_METHOD(getCommentIndex)
{
	struct zip *intern;
	zval *this = getThis();
	long index, flags = 0;
	const char * comment;
	int comment_len = 0;
	struct zip_stat sb;

	if (!this) {
		RETURN_FALSE;
	}

	ZIP_FROM_OBJECT(intern, this);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|l",
				&index, &flags) == FAILURE) {
		return;
	}

	PHP_ZIP_STAT_INDEX(intern, index, 0, sb);
	comment = zip_get_file_comment(intern, index, &comment_len, (int)flags);
	RETURN_STRINGL((char *)comment, (long)comment_len, 1);
}
/* }}} */

/* {{{ proto bool ZipArchive::deleteIndex(int index)
Delete a file using its index */
static ZIPARCHIVE_METHOD(deleteIndex)
{
	struct zip *intern;
	zval *this = getThis();
	long index;

	if (!this) {
		RETURN_FALSE;
	}

	ZIP_FROM_OBJECT(intern, this);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index) == FAILURE) {
		return;
	}

	if (index < 0) {
		RETURN_FALSE;
	}

	if (zip_delete(intern, index) < 0) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool ZipArchive::deleteName(string name)
Delete a file using its index */
static ZIPARCHIVE_METHOD(deleteName)
{
	struct zip *intern;
	zval *this = getThis();
	int name_len;
	char *name;
	struct zip_stat sb;

	if (!this) {
		RETURN_FALSE;
	}

	ZIP_FROM_OBJECT(intern, this);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len) == FAILURE) {
		return;
	}
	if (name_len < 1) {
		RETURN_FALSE;
	}

	PHP_ZIP_STAT_PATH(intern, name, name_len, 0, sb);
	if (zip_delete(intern, sb.index)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool ZipArchive::renameIndex(int index, string new_name)
Rename an entry selected by its index to new_name */
static ZIPARCHIVE_METHOD(renameIndex)
{
	struct zip *intern;
	zval *this = getThis();

	char *new_name;
	int new_name_len;
	long index;

	if (!this) {
		RETURN_FALSE;
	}

	ZIP_FROM_OBJECT(intern, this);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls", &index, &new_name, &new_name_len) == FAILURE) {
		return;
	}

	if (index < 0) {
		RETURN_FALSE;
	}

	if (new_name_len < 1) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Empty string as new entry name");
		RETURN_FALSE;
	}
	if (zip_rename(intern, index, (const char *)new_name) != 0) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool ZipArchive::renameName(string name, string new_name)
Rename an entry selected by its name to new_name */
static ZIPARCHIVE_METHOD(renameName)
{
	struct zip *intern;
	zval *this = getThis();
	struct zip_stat sb;
	char *name, *new_name;
	int name_len, new_name_len;

	if (!this) {
		RETURN_FALSE;
	}

	ZIP_FROM_OBJECT(intern, this);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &name, &name_len, &new_name, &new_name_len) == FAILURE) {
		return;
	}

	if (new_name_len < 1) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Empty string as new entry name");
		RETURN_FALSE;
	}

	PHP_ZIP_STAT_PATH(intern, name, name_len, 0, sb);

	if (zip_rename(intern, sb.index, (const char *)new_name)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool ZipArchive::unchangeIndex(int index)
Changes to the file at position index are reverted */
static ZIPARCHIVE_METHOD(unchangeIndex)
{
	struct zip *intern;
	zval *this = getThis();
	long index;

	if (!this) {
		RETURN_FALSE;
	}

	ZIP_FROM_OBJECT(intern, this);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index) == FAILURE) {
		return;
	}

	if (index < 0) {
		RETURN_FALSE;
	}

	if (zip_unchange(intern, index) != 0) {
		RETURN_FALSE;
	} else {
		RETURN_TRUE;
	}
}
/* }}} */

/* {{{ proto bool ZipArchive::unchangeName(string name)
Changes to the file named 'name' are reverted */
static ZIPARCHIVE_METHOD(unchangeName)
{
	struct zip *intern;
	zval *this = getThis();
	struct zip_stat sb;
	char *name;
	int name_len;

	if (!this) {
		RETURN_FALSE;
	}

	ZIP_FROM_OBJECT(intern, this);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len) == FAILURE) {
		return;
	}

	if (name_len < 1) {
		RETURN_FALSE;
	}

	PHP_ZIP_STAT_PATH(intern, name, name_len, 0, sb);

	if (zip_unchange(intern, sb.index) != 0) {
		RETURN_FALSE;
	} else {
		RETURN_TRUE;
	}
}
/* }}} */

/* {{{ proto bool ZipArchive::unchangeAll()
All changes to files and global information in archive are reverted */
static ZIPARCHIVE_METHOD(unchangeAll)
{
	struct zip *intern;
	zval *this = getThis();

	if (!this) {
		RETURN_FALSE;
	}

	ZIP_FROM_OBJECT(intern, this);

	if (zip_unchange_all(intern) != 0) {
		RETURN_FALSE;
	} else {
		RETURN_TRUE;
	}
}
/* }}} */

/* {{{ proto bool ZipArchive::unchangeArchive()
Revert all global changes to the archive archive.  For now, this only reverts archive comment changes. */
static ZIPARCHIVE_METHOD(unchangeArchive)
{
	struct zip *intern;
	zval *this = getThis();

	if (!this) {
		RETURN_FALSE;
	}

	ZIP_FROM_OBJECT(intern, this);

	if (zip_unchange_archive(intern) != 0) {
		RETURN_FALSE;
	} else {
		RETURN_TRUE;
	}
}
/* }}} */

/* {{{ proto bool ZipArchive::extractTo(string pathto[, mixed files])
Extract one or more file from a zip archive */
/* TODO:
 * - allow index or array of indeces
 * - replace path
 * - patterns
 */
static ZIPARCHIVE_METHOD(extractTo)
{
	struct zip *intern;

	zval *this = getThis();
	zval *zval_files = NULL;
	zval **zval_file = NULL;
	php_stream_statbuf ssb;
	char *pathto;
	int pathto_len;
	int ret, i;

	int nelems;

	if (!this) {
		RETURN_FALSE;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &pathto, &pathto_len, &zval_files) == FAILURE) {
		return;
	}

	if (pathto_len < 1) {
		RETURN_FALSE;
	}

	if (php_stream_stat_path_ex(pathto, PHP_STREAM_URL_STAT_QUIET, &ssb, NULL) < 0) {
			ret = php_stream_mkdir(pathto, 0777,  PHP_STREAM_MKDIR_RECURSIVE, NULL);
			if (!ret) {
					RETURN_FALSE;
			}
	}

	ZIP_FROM_OBJECT(intern, this);
	if (zval_files && (Z_TYPE_P(zval_files) != IS_NULL)) {
		switch (Z_TYPE_P(zval_files)) {
			case IS_STRING:
				if (!php_zip_extract_file(intern, pathto, Z_STRVAL_P(zval_files), Z_STRLEN_P(zval_files) TSRMLS_CC)) {
					RETURN_FALSE;
				}
				break;
			case IS_ARRAY:
				nelems = zend_hash_num_elements(Z_ARRVAL_P(zval_files));
				if (nelems == 0 ) {
					RETURN_FALSE;
				}
				for (i = 0; i < nelems; i++) {
					if (zend_hash_index_find(Z_ARRVAL_P(zval_files), i, (void **) &zval_file) == SUCCESS) {
						switch (Z_TYPE_PP(zval_file)) {
							case IS_LONG:
								break;
							case IS_STRING:
								if (!php_zip_extract_file(intern, pathto, Z_STRVAL_PP(zval_file), Z_STRLEN_PP(zval_file) TSRMLS_CC)) {
									RETURN_FALSE;
								}
								break;
						}
					}
				}
				break;
			case IS_LONG:
			default:
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid argument, expect string or array of strings");
				break;
		}
	} else {
		/* Extract all files */
		int filecount = zip_get_num_files(intern);

		if (filecount == -1) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Illegal archive");
				RETURN_FALSE;
		}

		for (i = 0; i < filecount; i++) {
			char *file = (char*)zip_get_name(intern, i, ZIP_FL_UNCHANGED);
			if (!php_zip_extract_file(intern, pathto, file, strlen(file) TSRMLS_CC)) {
					RETURN_FALSE;
			}
		}
	}
	RETURN_TRUE;
}
/* }}} */

static void php_zip_get_from(INTERNAL_FUNCTION_PARAMETERS, int type) /* {{{ */
{
	struct zip *intern;
	zval *this = getThis();

	struct zip_stat sb;
	struct zip_file *zf;

	char *filename;
	int	filename_len;
	long index = -1;
	long flags = 0;
	long len = 0;

	char *buffer;
	int n = 0;

	if (!this) {
		RETURN_FALSE;
	}

	ZIP_FROM_OBJECT(intern, this);

	if (type == 1) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "p|ll", &filename, &filename_len, &len, &flags) == FAILURE) {
			return;
		}
		PHP_ZIP_STAT_PATH(intern, filename, filename_len, flags, sb);
	} else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|ll", &index, &len, &flags) == FAILURE) {
			return;
		}
		PHP_ZIP_STAT_INDEX(intern, index, 0, sb);
	}

	if (sb.size < 1) {
		RETURN_EMPTY_STRING();
	}

	if (len < 1) {
		len = sb.size;
	}
	if (index >= 0) {
		zf = zip_fopen_index(intern, index, flags);
	} else {
		zf = zip_fopen(intern, filename, flags);
	}

	if (zf == NULL) {
		RETURN_FALSE;
	}

	buffer = safe_emalloc(len, 1, 2);
	n = zip_fread(zf, buffer, len);
	if (n < 1) {
		efree(buffer);
		RETURN_EMPTY_STRING();
	}

	zip_fclose(zf);
	buffer[n] = 0;
	RETURN_STRINGL(buffer, n, 0);
}
/* }}} */

/* {{{ proto string ZipArchive::getFromName(string entryname[, int len [, int flags]])
get the contents of an entry using its name */
static ZIPARCHIVE_METHOD(getFromName)
{
	php_zip_get_from(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto string ZipArchive::getFromIndex(int index[, int len [, int flags]])
get the contents of an entry using its index */
static ZIPARCHIVE_METHOD(getFromIndex)
{
	php_zip_get_from(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto resource ZipArchive::getStream(string entryname)
get a stream for an entry using its name */
static ZIPARCHIVE_METHOD(getStream)
{
	struct zip *intern;
	zval *this = getThis();
	struct zip_stat sb;
	char *filename;
	int	filename_len;
	char *mode = "rb";
	php_stream *stream;
	ze_zip_object *obj;

	if (!this) {
		RETURN_FALSE;
	}

	ZIP_FROM_OBJECT(intern, this);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "p", &filename, &filename_len) == FAILURE) {
		return;
	}

	if (zip_stat(intern, filename, 0, &sb) != 0) {
		RETURN_FALSE;
	}

	obj = (ze_zip_object*) zend_object_store_get_object(this TSRMLS_CC);

	stream = php_stream_zip_open(obj->filename, filename, mode STREAMS_CC TSRMLS_CC);
	if (stream) {
		php_stream_to_zval(stream, return_value);
	}
}
/* }}} */

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_ziparchive_open, 0, 0, 1)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ziparchive__void, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ziparchive_addemptydir, 0, 0, 1)
	ZEND_ARG_INFO(0, dirname)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ziparchive_addglob, 0, 0, 1)
	ZEND_ARG_INFO(0, pattern)
	ZEND_ARG_INFO(0, flags)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ziparchive_addpattern, 0, 0, 1)
	ZEND_ARG_INFO(0, pattern)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ziparchive_addfile, 0, 0, 1)
	ZEND_ARG_INFO(0, filepath)
	ZEND_ARG_INFO(0, entryname)
	ZEND_ARG_INFO(0, start)
	ZEND_ARG_INFO(0, length)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ziparchive_addfromstring, 0, 0, 2)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, content)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ziparchive_statname, 0, 0, 1)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ziparchive_statindex, 0, 0, 1)
	ZEND_ARG_INFO(0, index)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ziparchive_setarchivecomment, 0, 0, 1)
	ZEND_ARG_INFO(0, comment)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ziparchive_setcommentindex, 0, 0, 2)
	ZEND_ARG_INFO(0, index)
	ZEND_ARG_INFO(0, comment)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ziparchive_getcommentname, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ziparchive_getcommentindex, 0, 0, 1)
	ZEND_ARG_INFO(0, index)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ziparchive_renameindex, 0, 0, 2)
	ZEND_ARG_INFO(0, index)
	ZEND_ARG_INFO(0, new_name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ziparchive_renamename, 0, 0, 2)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, new_name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ziparchive_unchangeindex, 0, 0, 1)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ziparchive_unchangename, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ziparchive_extractto, 0, 0, 1)
	ZEND_ARG_INFO(0, pathto)
	ZEND_ARG_INFO(0, files)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ziparchive_getfromname, 0, 0, 1)
	ZEND_ARG_INFO(0, entryname)
	ZEND_ARG_INFO(0, len)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ziparchive_getfromindex, 0, 0, 1)
	ZEND_ARG_INFO(0, index)
	ZEND_ARG_INFO(0, len)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ziparchive_getarchivecomment, 0, 0, 0)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ziparchive_setcommentname, 0, 0, 2)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, comment)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ziparchive_getstream, 0, 0, 1)
	ZEND_ARG_INFO(0, entryname)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ ze_zip_object_class_functions */
static const zend_function_entry zip_class_functions[] = {
	ZIPARCHIVE_ME(open,					arginfo_ziparchive_open, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(close,				arginfo_ziparchive__void, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(getStatusString,		arginfo_ziparchive__void, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(addEmptyDir,			arginfo_ziparchive_addemptydir, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(addFromString,		arginfo_ziparchive_addfromstring, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(addFile,				arginfo_ziparchive_addfile, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(addGlob,				arginfo_ziparchive_addglob, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(addPattern,			arginfo_ziparchive_addpattern, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(renameIndex,			arginfo_ziparchive_renameindex, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(renameName,			arginfo_ziparchive_renamename, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(setArchiveComment,	arginfo_ziparchive_setarchivecomment, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(getArchiveComment,	arginfo_ziparchive_getarchivecomment, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(setCommentIndex,		arginfo_ziparchive_setcommentindex, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(setCommentName,		arginfo_ziparchive_setcommentname, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(getCommentIndex,		arginfo_ziparchive_getcommentindex, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(getCommentName,		arginfo_ziparchive_getcommentname, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(deleteIndex,			arginfo_ziparchive_unchangeindex, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(deleteName,			arginfo_ziparchive_unchangename, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(statName,				arginfo_ziparchive_statname, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(statIndex,			arginfo_ziparchive_statindex, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(locateName,			arginfo_ziparchive_statname, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(getNameIndex,			arginfo_ziparchive_statindex, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(unchangeArchive,		arginfo_ziparchive__void, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(unchangeAll,			arginfo_ziparchive__void, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(unchangeIndex,		arginfo_ziparchive_unchangeindex, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(unchangeName,			arginfo_ziparchive_unchangename, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(extractTo,			arginfo_ziparchive_extractto, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(getFromName,			arginfo_ziparchive_getfromname, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(getFromIndex,			arginfo_ziparchive_getfromindex, ZEND_ACC_PUBLIC)
	ZIPARCHIVE_ME(getStream,			arginfo_ziparchive_getstream, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};
/* }}} */
#endif

/* {{{ PHP_MINIT_FUNCTION */
static PHP_MINIT_FUNCTION(zip)
{
#ifdef PHP_ZIP_USE_OO
	zend_class_entry ce;

	memcpy(&zip_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	zip_object_handlers.clone_obj		= NULL;
	zip_object_handlers.get_property_ptr_ptr = php_zip_get_property_ptr_ptr;

	zip_object_handlers.get_properties = php_zip_get_properties;
	zip_object_handlers.read_property	= php_zip_read_property;
	zip_object_handlers.has_property	= php_zip_has_property;

	INIT_CLASS_ENTRY(ce, "ZipArchive", zip_class_functions);
	ce.create_object = php_zip_object_new;
	zip_class_entry = zend_register_internal_class(&ce TSRMLS_CC);

	zend_hash_init(&zip_prop_handlers, 0, NULL, NULL, 1);
	php_zip_register_prop_handler(&zip_prop_handlers, "status",    php_zip_status, NULL, NULL, IS_LONG TSRMLS_CC);
	php_zip_register_prop_handler(&zip_prop_handlers, "statusSys", php_zip_status_sys, NULL, NULL, IS_LONG TSRMLS_CC);
	php_zip_register_prop_handler(&zip_prop_handlers, "numFiles",  php_zip_get_num_files, NULL, NULL, IS_LONG TSRMLS_CC);
	php_zip_register_prop_handler(&zip_prop_handlers, "filename", NULL, NULL, php_zipobj_get_filename, IS_STRING TSRMLS_CC);
	php_zip_register_prop_handler(&zip_prop_handlers, "comment", NULL, php_zipobj_get_zip_comment, NULL, IS_STRING TSRMLS_CC);

	REGISTER_ZIP_CLASS_CONST_LONG("CREATE", ZIP_CREATE);
	REGISTER_ZIP_CLASS_CONST_LONG("EXCL", ZIP_EXCL);
	REGISTER_ZIP_CLASS_CONST_LONG("CHECKCONS", ZIP_CHECKCONS);
	REGISTER_ZIP_CLASS_CONST_LONG("OVERWRITE", ZIP_OVERWRITE);

	REGISTER_ZIP_CLASS_CONST_LONG("FL_NOCASE", ZIP_FL_NOCASE);
	REGISTER_ZIP_CLASS_CONST_LONG("FL_NODIR", ZIP_FL_NODIR);
	REGISTER_ZIP_CLASS_CONST_LONG("FL_COMPRESSED", ZIP_FL_COMPRESSED);
	REGISTER_ZIP_CLASS_CONST_LONG("FL_UNCHANGED", ZIP_FL_UNCHANGED);
	REGISTER_ZIP_CLASS_CONST_LONG("CM_DEFAULT", ZIP_CM_DEFAULT);
	REGISTER_ZIP_CLASS_CONST_LONG("CM_STORE", ZIP_CM_STORE);
	REGISTER_ZIP_CLASS_CONST_LONG("CM_SHRINK", ZIP_CM_SHRINK);
	REGISTER_ZIP_CLASS_CONST_LONG("CM_REDUCE_1", ZIP_CM_REDUCE_1);
	REGISTER_ZIP_CLASS_CONST_LONG("CM_REDUCE_2", ZIP_CM_REDUCE_2);
	REGISTER_ZIP_CLASS_CONST_LONG("CM_REDUCE_3", ZIP_CM_REDUCE_3);
	REGISTER_ZIP_CLASS_CONST_LONG("CM_REDUCE_4", ZIP_CM_REDUCE_4);
	REGISTER_ZIP_CLASS_CONST_LONG("CM_IMPLODE", ZIP_CM_IMPLODE);
	REGISTER_ZIP_CLASS_CONST_LONG("CM_DEFLATE", ZIP_CM_DEFLATE);
	REGISTER_ZIP_CLASS_CONST_LONG("CM_DEFLATE64", ZIP_CM_DEFLATE64);
	REGISTER_ZIP_CLASS_CONST_LONG("CM_PKWARE_IMPLODE", ZIP_CM_PKWARE_IMPLODE);
	REGISTER_ZIP_CLASS_CONST_LONG("CM_BZIP2", ZIP_CM_BZIP2);
	REGISTER_ZIP_CLASS_CONST_LONG("CM_LZMA", ZIP_CM_LZMA);
	REGISTER_ZIP_CLASS_CONST_LONG("CM_TERSE", ZIP_CM_TERSE);
	REGISTER_ZIP_CLASS_CONST_LONG("CM_LZ77", ZIP_CM_LZ77);
	REGISTER_ZIP_CLASS_CONST_LONG("CM_WAVPACK", ZIP_CM_WAVPACK);
	REGISTER_ZIP_CLASS_CONST_LONG("CM_PPMD", ZIP_CM_PPMD);

	/* Error code */
	REGISTER_ZIP_CLASS_CONST_LONG("ER_OK",			ZIP_ER_OK);			/* N No error */
	REGISTER_ZIP_CLASS_CONST_LONG("ER_MULTIDISK",	ZIP_ER_MULTIDISK);	/* N Multi-disk zip archives not supported */
	REGISTER_ZIP_CLASS_CONST_LONG("ER_RENAME",		ZIP_ER_RENAME);		/* S Renaming temporary file failed */
	REGISTER_ZIP_CLASS_CONST_LONG("ER_CLOSE",		ZIP_ER_CLOSE);		/* S Closing zip archive failed */
	REGISTER_ZIP_CLASS_CONST_LONG("ER_SEEK",		ZIP_ER_SEEK);		/* S Seek error */
	REGISTER_ZIP_CLASS_CONST_LONG("ER_READ",		ZIP_ER_READ);		/* S Read error */
	REGISTER_ZIP_CLASS_CONST_LONG("ER_WRITE",		ZIP_ER_WRITE);		/* S Write error */
	REGISTER_ZIP_CLASS_CONST_LONG("ER_CRC",			ZIP_ER_CRC);		/* N CRC error */
	REGISTER_ZIP_CLASS_CONST_LONG("ER_ZIPCLOSED",	ZIP_ER_ZIPCLOSED);	/* N Containing zip archive was closed */
	REGISTER_ZIP_CLASS_CONST_LONG("ER_NOENT",		ZIP_ER_NOENT);		/* N No such file */
	REGISTER_ZIP_CLASS_CONST_LONG("ER_EXISTS",		ZIP_ER_EXISTS);		/* N File already exists */
	REGISTER_ZIP_CLASS_CONST_LONG("ER_OPEN",		ZIP_ER_OPEN);		/* S Can't open file */
	REGISTER_ZIP_CLASS_CONST_LONG("ER_TMPOPEN",		ZIP_ER_TMPOPEN);	/* S Failure to create temporary file */
	REGISTER_ZIP_CLASS_CONST_LONG("ER_ZLIB",		ZIP_ER_ZLIB);		/* Z Zlib error */
	REGISTER_ZIP_CLASS_CONST_LONG("ER_MEMORY",		ZIP_ER_MEMORY);		/* N Malloc failure */
	REGISTER_ZIP_CLASS_CONST_LONG("ER_CHANGED",		ZIP_ER_CHANGED);	/* N Entry has been changed */
	REGISTER_ZIP_CLASS_CONST_LONG("ER_COMPNOTSUPP",	ZIP_ER_COMPNOTSUPP);/* N Compression method not supported */
	REGISTER_ZIP_CLASS_CONST_LONG("ER_EOF",			ZIP_ER_EOF);		/* N Premature EOF */
	REGISTER_ZIP_CLASS_CONST_LONG("ER_INVAL",		ZIP_ER_INVAL);		/* N Invalid argument */
	REGISTER_ZIP_CLASS_CONST_LONG("ER_NOZIP",		ZIP_ER_NOZIP);		/* N Not a zip archive */
	REGISTER_ZIP_CLASS_CONST_LONG("ER_INTERNAL",	ZIP_ER_INTERNAL);	/* N Internal error */
	REGISTER_ZIP_CLASS_CONST_LONG("ER_INCONS",		ZIP_ER_INCONS);		/* N Zip archive inconsistent */
	REGISTER_ZIP_CLASS_CONST_LONG("ER_REMOVE",		ZIP_ER_REMOVE);		/* S Can't remove file */
	REGISTER_ZIP_CLASS_CONST_LONG("ER_DELETED",  	ZIP_ER_DELETED);	/* N Entry has been deleted */

	php_register_url_stream_wrapper("zip", &php_stream_zip_wrapper TSRMLS_CC);
#endif

	le_zip_dir   = zend_register_list_destructors_ex(php_zip_free_dir,   NULL, le_zip_dir_name,   module_number);
	le_zip_entry = zend_register_list_destructors_ex(php_zip_free_entry, NULL, le_zip_entry_name, module_number);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
static PHP_MSHUTDOWN_FUNCTION(zip)
{
#ifdef PHP_ZIP_USE_OO
	zend_hash_destroy(&zip_prop_handlers);
	php_unregister_url_stream_wrapper("zip" TSRMLS_CC);
#endif
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
static PHP_MINFO_FUNCTION(zip)
{
	php_info_print_table_start();

	php_info_print_table_row(2, "Zip", "enabled");
	php_info_print_table_row(2, "Extension Version","$Id: 05dd1ecc211075107543b0ef8cee488dd229fccf $");
	php_info_print_table_row(2, "Zip version", PHP_ZIP_VERSION_STRING);
	php_info_print_table_row(2, "Libzip version", LIBZIP_VERSION);

	php_info_print_table_end();
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
