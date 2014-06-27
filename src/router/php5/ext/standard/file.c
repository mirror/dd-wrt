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
   | Authors: Rasmus Lerdorf <rasmus@php.net>                             |
   |          Stig Bakken <ssb@php.net>                                   |
   |          Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   | PHP 4.0 patches by Thies C. Arntzen (thies@thieso.net)               |
   | PHP streams by Wez Furlong (wez@thebrainroom.com)                    |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

/* Synced with php 3.0 revision 1.218 1999-06-16 [ssb] */

/* {{{ includes */

#include "php.h"
#include "php_globals.h"
#include "ext/standard/flock_compat.h"
#include "ext/standard/exec.h"
#include "ext/standard/php_filestat.h"
#include "php_open_temporary_file.h"
#include "ext/standard/basic_functions.h"
#include "php_ini.h"
#include "php_smart_str.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef PHP_WIN32
# include <io.h>
# define O_RDONLY _O_RDONLY
# include "win32/param.h"
# include "win32/winutil.h"
# include "win32/fnmatch.h"
#else
# if HAVE_SYS_PARAM_H
#  include <sys/param.h>
# endif
# if HAVE_SYS_SELECT_H
#  include <sys/select.h>
# endif
# if defined(NETWARE) && defined(USE_WINSOCK)
#  include <novsock2.h>
# else
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <netdb.h>
# endif
# if HAVE_ARPA_INET_H
#  include <arpa/inet.h>
# endif
#endif

#include "ext/standard/head.h"
#include "php_string.h"
#include "file.h"

#if HAVE_PWD_H
# ifdef PHP_WIN32
#  include "win32/pwd.h"
# else
#  include <pwd.h>
# endif
#endif

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif

#include "fsock.h"
#include "fopen_wrappers.h"
#include "streamsfuncs.h"
#include "php_globals.h"

#ifdef HAVE_SYS_FILE_H
# include <sys/file.h>
#endif

#if MISSING_FCLOSE_DECL
extern int fclose(FILE *);
#endif

#ifdef HAVE_SYS_MMAN_H
# include <sys/mman.h>
#endif

#include "scanf.h"
#include "zend_API.h"

#ifdef ZTS
int file_globals_id;
#else
php_file_globals file_globals;
#endif

#if defined(HAVE_FNMATCH) && !defined(PHP_WIN32)
# ifndef _GNU_SOURCE
#  define _GNU_SOURCE
# endif
# include <fnmatch.h>
#endif

#ifdef HAVE_WCHAR_H
# include <wchar.h>
#endif

#ifndef S_ISDIR
# define S_ISDIR(mode)	(((mode)&S_IFMT) == S_IFDIR)
#endif
/* }}} */

#define PHP_STREAM_TO_ZVAL(stream, arg) \
	php_stream_from_zval_no_verify(stream, arg); \
	if (stream == NULL) {	\
		RETURN_FALSE;	\
	}

/* {{{ ZTS-stuff / Globals / Prototypes */

/* sharing globals is *evil* */
static int le_stream_context = FAILURE;

PHPAPI int php_le_stream_context(TSRMLS_D)
{
	return le_stream_context;
}
/* }}} */

/* {{{ Module-Stuff
*/
static ZEND_RSRC_DTOR_FUNC(file_context_dtor)
{
	php_stream_context *context = (php_stream_context*)rsrc->ptr;
	if (context->options) {
		zval_ptr_dtor(&context->options);
		context->options = NULL;
	}
	php_stream_context_free(context);
}

static void file_globals_ctor(php_file_globals *file_globals_p TSRMLS_DC)
{
	FG(pclose_ret) = 0;
	FG(pclose_wait) = 0;
	FG(user_stream_current_filename) = NULL;
	FG(def_chunk_size) = PHP_SOCK_CHUNK_SIZE;
	FG(wrapper_errors) = NULL;
}

static void file_globals_dtor(php_file_globals *file_globals_p TSRMLS_DC)
{
}

PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("user_agent", NULL, PHP_INI_ALL, OnUpdateString, user_agent, php_file_globals, file_globals)
	STD_PHP_INI_ENTRY("from", NULL, PHP_INI_ALL, OnUpdateString, from_address, php_file_globals, file_globals)
	STD_PHP_INI_ENTRY("default_socket_timeout", "60", PHP_INI_ALL, OnUpdateLong, default_socket_timeout, php_file_globals, file_globals)
	STD_PHP_INI_ENTRY("auto_detect_line_endings", "0", PHP_INI_ALL, OnUpdateLong, auto_detect_line_endings, php_file_globals, file_globals)
PHP_INI_END()

PHP_MINIT_FUNCTION(file)
{
	le_stream_context = zend_register_list_destructors_ex(file_context_dtor, NULL, "stream-context", module_number);

#ifdef ZTS
	ts_allocate_id(&file_globals_id, sizeof(php_file_globals), (ts_allocate_ctor) file_globals_ctor, (ts_allocate_dtor) file_globals_dtor);
#else
	file_globals_ctor(&file_globals TSRMLS_CC);
#endif

	REGISTER_INI_ENTRIES();

	REGISTER_LONG_CONSTANT("SEEK_SET", SEEK_SET, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SEEK_CUR", SEEK_CUR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SEEK_END", SEEK_END, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOCK_SH", PHP_LOCK_SH, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOCK_EX", PHP_LOCK_EX, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOCK_UN", PHP_LOCK_UN, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOCK_NB", PHP_LOCK_NB, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("STREAM_NOTIFY_CONNECT",			PHP_STREAM_NOTIFY_CONNECT,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STREAM_NOTIFY_AUTH_REQUIRED",	PHP_STREAM_NOTIFY_AUTH_REQUIRED,	CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STREAM_NOTIFY_AUTH_RESULT",		PHP_STREAM_NOTIFY_AUTH_RESULT,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STREAM_NOTIFY_MIME_TYPE_IS",	PHP_STREAM_NOTIFY_MIME_TYPE_IS,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STREAM_NOTIFY_FILE_SIZE_IS",	PHP_STREAM_NOTIFY_FILE_SIZE_IS,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STREAM_NOTIFY_REDIRECTED",		PHP_STREAM_NOTIFY_REDIRECTED,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STREAM_NOTIFY_PROGRESS",		PHP_STREAM_NOTIFY_PROGRESS,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STREAM_NOTIFY_FAILURE",			PHP_STREAM_NOTIFY_FAILURE,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STREAM_NOTIFY_COMPLETED",		PHP_STREAM_NOTIFY_COMPLETED,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STREAM_NOTIFY_RESOLVE",			PHP_STREAM_NOTIFY_RESOLVE,			CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("STREAM_NOTIFY_SEVERITY_INFO",	PHP_STREAM_NOTIFY_SEVERITY_INFO,	CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STREAM_NOTIFY_SEVERITY_WARN",	PHP_STREAM_NOTIFY_SEVERITY_WARN,	CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STREAM_NOTIFY_SEVERITY_ERR",	PHP_STREAM_NOTIFY_SEVERITY_ERR,		CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("STREAM_FILTER_READ",			PHP_STREAM_FILTER_READ,				CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STREAM_FILTER_WRITE",			PHP_STREAM_FILTER_WRITE,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STREAM_FILTER_ALL",				PHP_STREAM_FILTER_ALL,				CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("STREAM_CLIENT_PERSISTENT",		PHP_STREAM_CLIENT_PERSISTENT,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STREAM_CLIENT_ASYNC_CONNECT",	PHP_STREAM_CLIENT_ASYNC_CONNECT,	CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STREAM_CLIENT_CONNECT",			PHP_STREAM_CLIENT_CONNECT,	CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("STREAM_CRYPTO_METHOD_SSLv2_CLIENT",		STREAM_CRYPTO_METHOD_SSLv2_CLIENT,	CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STREAM_CRYPTO_METHOD_SSLv3_CLIENT",		STREAM_CRYPTO_METHOD_SSLv3_CLIENT,	CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STREAM_CRYPTO_METHOD_SSLv23_CLIENT",	STREAM_CRYPTO_METHOD_SSLv23_CLIENT,	CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STREAM_CRYPTO_METHOD_TLS_CLIENT",		STREAM_CRYPTO_METHOD_TLS_CLIENT,	CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STREAM_CRYPTO_METHOD_SSLv2_SERVER",		STREAM_CRYPTO_METHOD_SSLv2_SERVER,	CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STREAM_CRYPTO_METHOD_SSLv3_SERVER",		STREAM_CRYPTO_METHOD_SSLv3_SERVER,	CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STREAM_CRYPTO_METHOD_SSLv23_SERVER",	STREAM_CRYPTO_METHOD_SSLv23_SERVER,	CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STREAM_CRYPTO_METHOD_TLS_SERVER",		STREAM_CRYPTO_METHOD_TLS_SERVER,	CONST_CS|CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("STREAM_SHUT_RD",	STREAM_SHUT_RD,		CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STREAM_SHUT_WR",	STREAM_SHUT_WR,		CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STREAM_SHUT_RDWR",	STREAM_SHUT_RDWR,	CONST_CS|CONST_PERSISTENT);

#ifdef PF_INET
	REGISTER_LONG_CONSTANT("STREAM_PF_INET", PF_INET, CONST_CS|CONST_PERSISTENT);
#elif defined(AF_INET)
	REGISTER_LONG_CONSTANT("STREAM_PF_INET", AF_INET, CONST_CS|CONST_PERSISTENT);
#endif

#if HAVE_IPV6
# ifdef PF_INET6
	REGISTER_LONG_CONSTANT("STREAM_PF_INET6", PF_INET6, CONST_CS|CONST_PERSISTENT);
# elif defined(AF_INET6)
	REGISTER_LONG_CONSTANT("STREAM_PF_INET6", AF_INET6, CONST_CS|CONST_PERSISTENT);
# endif
#endif

#ifdef PF_UNIX
	REGISTER_LONG_CONSTANT("STREAM_PF_UNIX", PF_UNIX, CONST_CS|CONST_PERSISTENT);
#elif defined(AF_UNIX)
	REGISTER_LONG_CONSTANT("STREAM_PF_UNIX", AF_UNIX, CONST_CS|CONST_PERSISTENT);
#endif

#ifdef IPPROTO_IP
	/* most people will use this one when calling socket() or socketpair() */
	REGISTER_LONG_CONSTANT("STREAM_IPPROTO_IP", IPPROTO_IP, CONST_CS|CONST_PERSISTENT);
#endif

#ifdef IPPROTO_TCP
	REGISTER_LONG_CONSTANT("STREAM_IPPROTO_TCP", IPPROTO_TCP, CONST_CS|CONST_PERSISTENT);
#endif

#ifdef IPPROTO_UDP
	REGISTER_LONG_CONSTANT("STREAM_IPPROTO_UDP", IPPROTO_UDP, CONST_CS|CONST_PERSISTENT);
#endif

#ifdef IPPROTO_ICMP
	REGISTER_LONG_CONSTANT("STREAM_IPPROTO_ICMP", IPPROTO_ICMP, CONST_CS|CONST_PERSISTENT);
#endif

#ifdef IPPROTO_RAW
	REGISTER_LONG_CONSTANT("STREAM_IPPROTO_RAW", IPPROTO_RAW, CONST_CS|CONST_PERSISTENT);
#endif

	REGISTER_LONG_CONSTANT("STREAM_SOCK_STREAM", SOCK_STREAM, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STREAM_SOCK_DGRAM", SOCK_DGRAM, CONST_CS|CONST_PERSISTENT);

#ifdef SOCK_RAW
	REGISTER_LONG_CONSTANT("STREAM_SOCK_RAW", SOCK_RAW, CONST_CS|CONST_PERSISTENT);
#endif

#ifdef SOCK_SEQPACKET
	REGISTER_LONG_CONSTANT("STREAM_SOCK_SEQPACKET", SOCK_SEQPACKET, CONST_CS|CONST_PERSISTENT);
#endif

#ifdef SOCK_RDM
	REGISTER_LONG_CONSTANT("STREAM_SOCK_RDM", SOCK_RDM, CONST_CS|CONST_PERSISTENT);
#endif

	REGISTER_LONG_CONSTANT("STREAM_PEEK", STREAM_PEEK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STREAM_OOB",  STREAM_OOB, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("STREAM_SERVER_BIND",			STREAM_XPORT_BIND,					CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STREAM_SERVER_LISTEN",			STREAM_XPORT_LISTEN,				CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("FILE_USE_INCLUDE_PATH",			PHP_FILE_USE_INCLUDE_PATH,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FILE_IGNORE_NEW_LINES",			PHP_FILE_IGNORE_NEW_LINES,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FILE_SKIP_EMPTY_LINES",			PHP_FILE_SKIP_EMPTY_LINES,			CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FILE_APPEND",					PHP_FILE_APPEND,					CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FILE_NO_DEFAULT_CONTEXT",		PHP_FILE_NO_DEFAULT_CONTEXT,		CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("FILE_TEXT",						0,									CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FILE_BINARY",					0,									CONST_CS | CONST_PERSISTENT);

#ifdef HAVE_FNMATCH
	REGISTER_LONG_CONSTANT("FNM_NOESCAPE", FNM_NOESCAPE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FNM_PATHNAME", FNM_PATHNAME, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FNM_PERIOD",   FNM_PERIOD,   CONST_CS | CONST_PERSISTENT);
# ifdef FNM_CASEFOLD /* a GNU extension */ /* TODO emulate if not available */
	REGISTER_LONG_CONSTANT("FNM_CASEFOLD", FNM_CASEFOLD, CONST_CS | CONST_PERSISTENT);
# endif
#endif

	return SUCCESS;
}
/* }}} */

PHP_MSHUTDOWN_FUNCTION(file) /* {{{ */
{
#ifndef ZTS
	file_globals_dtor(&file_globals TSRMLS_CC);
#endif
	return SUCCESS;
}
/* }}} */

static int flock_values[] = { LOCK_SH, LOCK_EX, LOCK_UN };

/* {{{ proto bool flock(resource fp, int operation [, int &wouldblock])
   Portable file locking */
PHP_FUNCTION(flock)
{
	zval *arg1, *arg3 = NULL;
	int act;
	php_stream *stream;
	long operation = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl|z", &arg1, &operation, &arg3) == FAILURE) {
		return;
	}

	PHP_STREAM_TO_ZVAL(stream, &arg1);

	act = operation & 3;
	if (act < 1 || act > 3) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Illegal operation argument");
		RETURN_FALSE;
	}

	if (arg3 && PZVAL_IS_REF(arg3)) {
		convert_to_long_ex(&arg3);
		Z_LVAL_P(arg3) = 0;
	}

	/* flock_values contains all possible actions if (operation & 4) we won't block on the lock */
	act = flock_values[act - 1] | (operation & PHP_LOCK_NB ? LOCK_NB : 0);
	if (php_stream_lock(stream, act)) {
		if (operation && errno == EWOULDBLOCK && arg3 && PZVAL_IS_REF(arg3)) {
			Z_LVAL_P(arg3) = 1;
		}
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

#define PHP_META_UNSAFE ".\\+*?[^]$() "

/* {{{ proto array get_meta_tags(string filename [, bool use_include_path])
   Extracts all meta tag content attributes from a file and returns an array */
PHP_FUNCTION(get_meta_tags)
{
	char *filename;
	int filename_len;
	zend_bool use_include_path = 0;
	int in_tag = 0, done = 0;
	int looking_for_val = 0, have_name = 0, have_content = 0;
	int saw_name = 0, saw_content = 0;
	char *name = NULL, *value = NULL, *temp = NULL;
	php_meta_tags_token tok, tok_last;
	php_meta_tags_data md;

	/* Initiailize our structure */
	memset(&md, 0, sizeof(md));

	/* Parse arguments */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "p|b", &filename, &filename_len, &use_include_path) == FAILURE) {
		return;
	}

	md.stream = php_stream_open_wrapper(filename, "rb",
			(use_include_path ? USE_PATH : 0) | REPORT_ERRORS,
			NULL);
	if (!md.stream)	{
		RETURN_FALSE;
	}

	array_init(return_value);

	tok_last = TOK_EOF;

	while (!done && (tok = php_next_meta_token(&md TSRMLS_CC)) != TOK_EOF) {
		if (tok == TOK_ID) {
			if (tok_last == TOK_OPENTAG) {
				md.in_meta = !strcasecmp("meta", md.token_data);
			} else if (tok_last == TOK_SLASH && in_tag) {
				if (strcasecmp("head", md.token_data) == 0) {
					/* We are done here! */
					done = 1;
				}
			} else if (tok_last == TOK_EQUAL && looking_for_val) {
				if (saw_name) {
					STR_FREE(name);
					/* Get the NAME attr (Single word attr, non-quoted) */
					temp = name = estrndup(md.token_data, md.token_len);

					while (temp && *temp) {
						if (strchr(PHP_META_UNSAFE, *temp)) {
							*temp = '_';
						}
						temp++;
					}

					have_name = 1;
				} else if (saw_content) {
					STR_FREE(value);
					value = estrndup(md.token_data, md.token_len);
					have_content = 1;
				}

				looking_for_val = 0;
			} else {
				if (md.in_meta) {
					if (strcasecmp("name", md.token_data) == 0) {
						saw_name = 1;
						saw_content = 0;
						looking_for_val = 1;
					} else if (strcasecmp("content", md.token_data) == 0) {
						saw_name = 0;
						saw_content = 1;
						looking_for_val = 1;
					}
				}
			}
		} else if (tok == TOK_STRING && tok_last == TOK_EQUAL && looking_for_val) {
			if (saw_name) {
				STR_FREE(name);
				/* Get the NAME attr (Quoted single/double) */
				temp = name = estrndup(md.token_data, md.token_len);

				while (temp && *temp) {
					if (strchr(PHP_META_UNSAFE, *temp)) {
						*temp = '_';
					}
					temp++;
				}

				have_name = 1;
			} else if (saw_content) {
				STR_FREE(value);
				value = estrndup(md.token_data, md.token_len);
				have_content = 1;
			}

			looking_for_val = 0;
		} else if (tok == TOK_OPENTAG) {
			if (looking_for_val) {
				looking_for_val = 0;
				have_name = saw_name = 0;
				have_content = saw_content = 0;
			}
			in_tag = 1;
		} else if (tok == TOK_CLOSETAG) {
			if (have_name) {
				/* For BC */
				php_strtolower(name, strlen(name));
				if (have_content) {
					add_assoc_string(return_value, name, value, 1);
				} else {
					add_assoc_string(return_value, name, "", 1);
				}

				efree(name);
				STR_FREE(value);
			} else if (have_content) {
				efree(value);
			}

			name = value = NULL;

			/* Reset all of our flags */
			in_tag = looking_for_val = 0;
			have_name = saw_name = 0;
			have_content = saw_content = 0;
			md.in_meta = 0;
		}

		tok_last = tok;

		if (md.token_data)
			efree(md.token_data);

		md.token_data = NULL;
	}

	STR_FREE(value);
	STR_FREE(name);
	php_stream_close(md.stream);
}
/* }}} */

/* {{{ proto string file_get_contents(string filename [, bool use_include_path [, resource context [, long offset [, long maxlen]]]])
   Read the entire file into a string */
PHP_FUNCTION(file_get_contents)
{
	char *filename;
	int filename_len;
	char *contents;
	zend_bool use_include_path = 0;
	php_stream *stream;
	int len;
	long offset = -1;
	long maxlen = PHP_STREAM_COPY_ALL;
	zval *zcontext = NULL;
	php_stream_context *context = NULL;

	/* Parse arguments */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "p|br!ll", &filename, &filename_len, &use_include_path, &zcontext, &offset, &maxlen) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() == 5 && maxlen < 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "length must be greater than or equal to zero");
		RETURN_FALSE;
	}

	context = php_stream_context_from_zval(zcontext, 0);

	stream = php_stream_open_wrapper_ex(filename, "rb",
				(use_include_path ? USE_PATH : 0) | REPORT_ERRORS,
				NULL, context);
	if (!stream) {
		RETURN_FALSE;
	}

	if (offset > 0 && php_stream_seek(stream, offset, SEEK_SET) < 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to seek to position %ld in the stream", offset);
		php_stream_close(stream);
		RETURN_FALSE;
	}

	if ((len = php_stream_copy_to_mem(stream, &contents, maxlen, 0)) > 0) {
		RETVAL_STRINGL(contents, len, 0);
	} else if (len == 0) {
		RETVAL_EMPTY_STRING();
	} else {
		RETVAL_FALSE;
	}

	php_stream_close(stream);
}
/* }}} */

/* {{{ proto int file_put_contents(string file, mixed data [, int flags [, resource context]])
   Write/Create a file with contents data and return the number of bytes written */
PHP_FUNCTION(file_put_contents)
{
	php_stream *stream;
	char *filename;
	int filename_len;
	zval *data;
	int numbytes = 0;
	long flags = 0;
	zval *zcontext = NULL;
	php_stream_context *context = NULL;
	php_stream *srcstream = NULL;
	char mode[3] = "wb";

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "pz/|lr!", &filename, &filename_len, &data, &flags, &zcontext) == FAILURE) {
		return;
	}

	if (Z_TYPE_P(data) == IS_RESOURCE) {
		php_stream_from_zval(srcstream, &data);
	}

	context = php_stream_context_from_zval(zcontext, flags & PHP_FILE_NO_DEFAULT_CONTEXT);

	if (flags & PHP_FILE_APPEND) {
		mode[0] = 'a';
	} else if (flags & LOCK_EX) {
		/* check to make sure we are dealing with a regular file */
		if (php_memnstr(filename, "://", sizeof("://") - 1, filename + filename_len)) {
			if (strncasecmp(filename, "file://", sizeof("file://") - 1)) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Exclusive locks may only be set for regular files");
				RETURN_FALSE;
			}
		}
		mode[0] = 'c';
	}
	mode[2] = '\0';

	stream = php_stream_open_wrapper_ex(filename, mode, ((flags & PHP_FILE_USE_INCLUDE_PATH) ? USE_PATH : 0) | REPORT_ERRORS, NULL, context);
	if (stream == NULL) {
		RETURN_FALSE;
	}

	if (flags & LOCK_EX && (!php_stream_supports_lock(stream) || php_stream_lock(stream, LOCK_EX))) {
		php_stream_close(stream);
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Exclusive locks are not supported for this stream");
		RETURN_FALSE;
	}

	if (mode[0] == 'c') {
		php_stream_truncate_set_size(stream, 0);
	}

	switch (Z_TYPE_P(data)) {
		case IS_RESOURCE: {
			size_t len;
			if (php_stream_copy_to_stream_ex(srcstream, stream, PHP_STREAM_COPY_ALL, &len) != SUCCESS) {
				numbytes = -1;
			} else {
				numbytes = len;
			}
			break;
		}
		case IS_NULL:
		case IS_LONG:
		case IS_DOUBLE:
		case IS_BOOL:
		case IS_CONSTANT:
			convert_to_string_ex(&data);

		case IS_STRING:
			if (Z_STRLEN_P(data)) {
				numbytes = php_stream_write(stream, Z_STRVAL_P(data), Z_STRLEN_P(data));
				if (numbytes != Z_STRLEN_P(data)) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Only %d of %d bytes written, possibly out of free disk space", numbytes, Z_STRLEN_P(data));
					numbytes = -1;
				}
			}
			break;

		case IS_ARRAY:
			if (zend_hash_num_elements(Z_ARRVAL_P(data))) {
				int bytes_written;
				zval **tmp;
				HashPosition pos;

				zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(data), &pos);
				while (zend_hash_get_current_data_ex(Z_ARRVAL_P(data), (void **) &tmp, &pos) == SUCCESS) {
					if (Z_TYPE_PP(tmp) != IS_STRING) {
						SEPARATE_ZVAL(tmp);
						convert_to_string(*tmp);
					}
					if (Z_STRLEN_PP(tmp)) {
						numbytes += Z_STRLEN_PP(tmp);
						bytes_written = php_stream_write(stream, Z_STRVAL_PP(tmp), Z_STRLEN_PP(tmp));
						if (bytes_written < 0 || bytes_written != Z_STRLEN_PP(tmp)) {
							if (bytes_written < 0) {
								php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to write %d bytes to %s", Z_STRLEN_PP(tmp), filename);
							} else {
								php_error_docref(NULL TSRMLS_CC, E_WARNING, "Only %d of %d bytes written, possibly out of free disk space", bytes_written, Z_STRLEN_PP(tmp));
							}
							numbytes = -1;
							break;
						}
					}
					zend_hash_move_forward_ex(Z_ARRVAL_P(data), &pos);
				}
			}
			break;

		case IS_OBJECT:
			if (Z_OBJ_HT_P(data) != NULL) {
				zval out;

				if (zend_std_cast_object_tostring(data, &out, IS_STRING TSRMLS_CC) == SUCCESS) {
					numbytes = php_stream_write(stream, Z_STRVAL(out), Z_STRLEN(out));
					if (numbytes != Z_STRLEN(out)) {
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Only %d of %d bytes written, possibly out of free disk space", numbytes, Z_STRLEN(out));
						numbytes = -1;
					}
					zval_dtor(&out);
					break;
				}
			}
		default:
			numbytes = -1;
			break;
	}
	php_stream_close(stream);

	if (numbytes < 0) {
		RETURN_FALSE;
	}

	RETURN_LONG(numbytes);
}
/* }}} */

#define PHP_FILE_BUF_SIZE	80

/* {{{ proto array file(string filename [, int flags[, resource context]])
   Read entire file into an array */
PHP_FUNCTION(file)
{
	char *filename;
	int filename_len;
	char *target_buf=NULL, *p, *s, *e;
	register int i = 0;
	int target_len;
	char eol_marker = '\n';
	long flags = 0;
	zend_bool use_include_path;
	zend_bool include_new_line;
	zend_bool skip_blank_lines;
	php_stream *stream;
	zval *zcontext = NULL;
	php_stream_context *context = NULL;

	/* Parse arguments */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "p|lr!", &filename, &filename_len, &flags, &zcontext) == FAILURE) {
		return;
	}
	if (flags < 0 || flags > (PHP_FILE_USE_INCLUDE_PATH | PHP_FILE_IGNORE_NEW_LINES | PHP_FILE_SKIP_EMPTY_LINES | PHP_FILE_NO_DEFAULT_CONTEXT)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "'%ld' flag is not supported", flags);
		RETURN_FALSE;
	}

	use_include_path = flags & PHP_FILE_USE_INCLUDE_PATH;
	include_new_line = !(flags & PHP_FILE_IGNORE_NEW_LINES);
	skip_blank_lines = flags & PHP_FILE_SKIP_EMPTY_LINES;

	context = php_stream_context_from_zval(zcontext, flags & PHP_FILE_NO_DEFAULT_CONTEXT);

	stream = php_stream_open_wrapper_ex(filename, "rb", (use_include_path ? USE_PATH : 0) | REPORT_ERRORS, NULL, context);
	if (!stream) {
		RETURN_FALSE;
	}

	/* Initialize return array */
	array_init(return_value);

	if ((target_len = php_stream_copy_to_mem(stream, &target_buf, PHP_STREAM_COPY_ALL, 0))) {
		s = target_buf;
		e = target_buf + target_len;

		if (!(p = php_stream_locate_eol(stream, target_buf, target_len TSRMLS_CC))) {
			p = e;
			goto parse_eol;
		}

		if (stream->flags & PHP_STREAM_FLAG_EOL_MAC) {
			eol_marker = '\r';
		}

		/* for performance reasons the code is duplicated, so that the if (include_new_line)
		 * will not need to be done for every single line in the file. */
		if (include_new_line) {
			do {
				p++;
parse_eol:
				add_index_stringl(return_value, i++, estrndup(s, p-s), p-s, 0);
				s = p;
			} while ((p = memchr(p, eol_marker, (e-p))));
		} else {
			do {
				int windows_eol = 0;
				if (p != target_buf && eol_marker == '\n' && *(p - 1) == '\r') {
					windows_eol++;
				}
				if (skip_blank_lines && !(p-s-windows_eol)) {
					s = ++p;
					continue;
				}
				add_index_stringl(return_value, i++, estrndup(s, p-s-windows_eol), p-s-windows_eol, 0);
				s = ++p;
			} while ((p = memchr(p, eol_marker, (e-p))));
		}

		/* handle any left overs of files without new lines */
		if (s != e) {
			p = e;
			goto parse_eol;
		}
	}

	if (target_buf) {
		efree(target_buf);
	}
	php_stream_close(stream);
}
/* }}} */

/* {{{ proto string tempnam(string dir, string prefix)
   Create a unique filename in a directory */
PHP_FUNCTION(tempnam)
{
	char *dir, *prefix;
	int dir_len, prefix_len;
	size_t p_len;
	char *opened_path;
	char *p;
	int fd;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ps", &dir, &dir_len, &prefix, &prefix_len) == FAILURE) {
		return;
	}

	if (php_check_open_basedir(dir TSRMLS_CC)) {
		RETURN_FALSE;
	}

	php_basename(prefix, prefix_len, NULL, 0, &p, &p_len TSRMLS_CC);
	if (p_len > 64) {
		p[63] = '\0';
	}

	RETVAL_FALSE;

	if ((fd = php_open_temporary_fd_ex(dir, p, &opened_path, 1 TSRMLS_CC)) >= 0) {
		close(fd);
		RETVAL_STRING(opened_path, 0);
	}
	efree(p);
}
/* }}} */

/* {{{ proto resource tmpfile(void)
   Create a temporary file that will be deleted automatically after use */
PHP_NAMED_FUNCTION(php_if_tmpfile)
{
	php_stream *stream;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	stream = php_stream_fopen_tmpfile();

	if (stream) {
		php_stream_to_zval(stream, return_value);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto resource fopen(string filename, string mode [, bool use_include_path [, resource context]])
   Open a file or a URL and return a file pointer */
PHP_NAMED_FUNCTION(php_if_fopen)
{
	char *filename, *mode;
	int filename_len, mode_len;
	zend_bool use_include_path = 0;
	zval *zcontext = NULL;
	php_stream *stream;
	php_stream_context *context = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ps|br", &filename, &filename_len, &mode, &mode_len, &use_include_path, &zcontext) == FAILURE) {
		RETURN_FALSE;
	}

	context = php_stream_context_from_zval(zcontext, 0);

	stream = php_stream_open_wrapper_ex(filename, mode, (use_include_path ? USE_PATH : 0) | REPORT_ERRORS, NULL, context);

	if (stream == NULL) {
		RETURN_FALSE;
	}

	php_stream_to_zval(stream, return_value);
}
/* }}} */

/* {{{ proto bool fclose(resource fp)
   Close an open file pointer */
PHPAPI PHP_FUNCTION(fclose)
{
	zval *arg1;
	php_stream *stream;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &arg1) == FAILURE) {
		RETURN_FALSE;
	}

	PHP_STREAM_TO_ZVAL(stream, &arg1);

	if ((stream->flags & PHP_STREAM_FLAG_NO_FCLOSE) != 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%d is not a valid stream resource", stream->rsrc_id);
		RETURN_FALSE;
	}

	if (!stream->is_persistent) {
		php_stream_close(stream);
	} else {
		php_stream_pclose(stream);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto resource popen(string command, string mode)
   Execute a command and open either a read or a write pipe to it */
PHP_FUNCTION(popen)
{
	char *command, *mode;
	int command_len, mode_len;
	FILE *fp;
	php_stream *stream;
	char *posix_mode;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ps", &command, &command_len, &mode, &mode_len) == FAILURE) {
		return;
	}

	posix_mode = estrndup(mode, mode_len);
#ifndef PHP_WIN32
	{
		char *z = memchr(posix_mode, 'b', mode_len);
		if (z) {
			memmove(z, z + 1, mode_len - (z - posix_mode));
		}
	}
#endif

	fp = VCWD_POPEN(command, posix_mode);
	if (!fp) {
		php_error_docref2(NULL TSRMLS_CC, command, posix_mode, E_WARNING, "%s", strerror(errno));
		efree(posix_mode);
		RETURN_FALSE;
	}

	stream = php_stream_fopen_from_pipe(fp, mode);

	if (stream == NULL)	{
		php_error_docref2(NULL TSRMLS_CC, command, mode, E_WARNING, "%s", strerror(errno));
		RETVAL_FALSE;
	} else {
		php_stream_to_zval(stream, return_value);
	}

	efree(posix_mode);
}
/* }}} */

/* {{{ proto int pclose(resource fp)
   Close a file pointer opened by popen() */
PHP_FUNCTION(pclose)
{
	zval *arg1;
	php_stream *stream;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &arg1) == FAILURE) {
		RETURN_FALSE;
	}

	PHP_STREAM_TO_ZVAL(stream, &arg1);

	FG(pclose_wait) = 1;
	zend_list_delete(stream->rsrc_id);
	FG(pclose_wait) = 0;
	RETURN_LONG(FG(pclose_ret));
}
/* }}} */

/* {{{ proto bool feof(resource fp)
   Test for end-of-file on a file pointer */
PHPAPI PHP_FUNCTION(feof)
{
	zval *arg1;
	php_stream *stream;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &arg1) == FAILURE) {
		RETURN_FALSE;
	}

	PHP_STREAM_TO_ZVAL(stream, &arg1);

	if (php_stream_eof(stream)) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto string fgets(resource fp[, int length])
   Get a line from file pointer */
PHPAPI PHP_FUNCTION(fgets)
{
	zval *arg1;
	long len = 1024;
	char *buf = NULL;
	int argc = ZEND_NUM_ARGS();
	size_t line_len = 0;
	php_stream *stream;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|l", &arg1, &len) == FAILURE) {
		RETURN_FALSE;
	}

	PHP_STREAM_TO_ZVAL(stream, &arg1);

	if (argc == 1) {
		/* ask streams to give us a buffer of an appropriate size */
		buf = php_stream_get_line(stream, NULL, 0, &line_len);
		if (buf == NULL) {
			goto exit_failed;
		}
	} else if (argc > 1) {
		if (len <= 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Length parameter must be greater than 0");
			RETURN_FALSE;
		}

		buf = ecalloc(len + 1, sizeof(char));
		if (php_stream_get_line(stream, buf, len, &line_len) == NULL) {
			goto exit_failed;
		}
	}

	ZVAL_STRINGL(return_value, buf, line_len, 0);
	/* resize buffer if it's much larger than the result.
	 * Only needed if the user requested a buffer size. */
	if (argc > 1 && Z_STRLEN_P(return_value) < len / 2) {
		Z_STRVAL_P(return_value) = erealloc(buf, line_len + 1);
	}
	return;

exit_failed:
	RETVAL_FALSE;
	if (buf) {
		efree(buf);
	}
}
/* }}} */

/* {{{ proto string fgetc(resource fp)
   Get a character from file pointer */
PHPAPI PHP_FUNCTION(fgetc)
{
	zval *arg1;
	char buf[2];
	int result;
	php_stream *stream;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &arg1) == FAILURE) {
		RETURN_FALSE;
	}

	PHP_STREAM_TO_ZVAL(stream, &arg1);

	result = php_stream_getc(stream);

	if (result == EOF) {
		RETVAL_FALSE;
	} else {
		buf[0] = result;
		buf[1] = '\0';

		RETURN_STRINGL(buf, 1, 1);
	}
}
/* }}} */

/* {{{ proto string fgetss(resource fp [, int length [, string allowable_tags]])
   Get a line from file pointer and strip HTML tags */
PHPAPI PHP_FUNCTION(fgetss)
{
	zval *fd;
	long bytes = 0;
	size_t len = 0;
	size_t actual_len, retval_len;
	char *buf = NULL, *retval;
	php_stream *stream;
	char *allowed_tags=NULL;
	int allowed_tags_len=0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|ls", &fd, &bytes, &allowed_tags, &allowed_tags_len) == FAILURE) {
		RETURN_FALSE;
	}

	PHP_STREAM_TO_ZVAL(stream, &fd);

	if (ZEND_NUM_ARGS() >= 2) {
		if (bytes <= 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Length parameter must be greater than 0");
			RETURN_FALSE;
		}

		len = (size_t) bytes;
		buf = safe_emalloc(sizeof(char), (len + 1), 0);
		/*needed because recv doesnt set null char at end*/
		memset(buf, 0, len + 1);
	}

	if ((retval = php_stream_get_line(stream, buf, len, &actual_len)) == NULL)	{
		if (buf != NULL) {
			efree(buf);
		}
		RETURN_FALSE;
	}

	retval_len = php_strip_tags(retval, actual_len, &stream->fgetss_state, allowed_tags, allowed_tags_len);

	RETURN_STRINGL(retval, retval_len, 0);
}
/* }}} */

/* {{{ proto mixed fscanf(resource stream, string format [, string ...])
   Implements a mostly ANSI compatible fscanf() */
PHP_FUNCTION(fscanf)
{
	int result, format_len, type, argc = 0;
	zval ***args = NULL;
	zval *file_handle;
	char *buf, *format;
	size_t len;
	void *what;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs*", &file_handle, &format, &format_len, &args, &argc) == FAILURE) {
		return;
	}

	what = zend_fetch_resource(&file_handle TSRMLS_CC, -1, "File-Handle", &type, 2, php_file_le_stream(), php_file_le_pstream());

	/* we can't do a ZEND_VERIFY_RESOURCE(what), otherwise we end up
	 * with a leak if we have an invalid filehandle. This needs changing
	 * if the code behind ZEND_VERIFY_RESOURCE changed. - cc */
	if (!what) {
		if (args) {
			efree(args);
		}
		RETURN_FALSE;
	}

	buf = php_stream_get_line((php_stream *) what, NULL, 0, &len);
	if (buf == NULL) {
		if (args) {
			efree(args);
		}
		RETURN_FALSE;
	}

	result = php_sscanf_internal(buf, format, argc, args, 0, &return_value TSRMLS_CC);

	if (args) {
		efree(args);
	}
	efree(buf);

	if (SCAN_ERROR_WRONG_PARAM_COUNT == result) {
		WRONG_PARAM_COUNT;
	}
}
/* }}} */

/* {{{ proto int fwrite(resource fp, string str [, int length])
   Binary-safe file write */
PHPAPI PHP_FUNCTION(fwrite)
{
	zval *arg1;
	char *arg2;
	int arg2len;
	int ret;
	int num_bytes;
	long arg3 = 0;
	char *buffer = NULL;
	php_stream *stream;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs|l", &arg1, &arg2, &arg2len, &arg3) == FAILURE) {
		RETURN_FALSE;
	}

	if (ZEND_NUM_ARGS() == 2) {
		num_bytes = arg2len;
	} else {
		num_bytes = MAX(0, MIN((int)arg3, arg2len));
	}

	if (!num_bytes) {
		RETURN_LONG(0);
	}

	PHP_STREAM_TO_ZVAL(stream, &arg1);

	ret = php_stream_write(stream, buffer ? buffer : arg2, num_bytes);
	if (buffer) {
		efree(buffer);
	}

	RETURN_LONG(ret);
}
/* }}} */

/* {{{ proto bool fflush(resource fp)
   Flushes output */
PHPAPI PHP_FUNCTION(fflush)
{
	zval *arg1;
	int ret;
	php_stream *stream;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &arg1) == FAILURE) {
		RETURN_FALSE;
	}

	PHP_STREAM_TO_ZVAL(stream, &arg1);

	ret = php_stream_flush(stream);
	if (ret) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool rewind(resource fp)
   Rewind the position of a file pointer */
PHPAPI PHP_FUNCTION(rewind)
{
	zval *arg1;
	php_stream *stream;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &arg1) == FAILURE) {
		RETURN_FALSE;
	}

	PHP_STREAM_TO_ZVAL(stream, &arg1);

	if (-1 == php_stream_rewind(stream)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int ftell(resource fp)
   Get file pointer's read/write position */
PHPAPI PHP_FUNCTION(ftell)
{
	zval *arg1;
	long ret;
	php_stream *stream;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &arg1) == FAILURE) {
		RETURN_FALSE;
	}

	PHP_STREAM_TO_ZVAL(stream, &arg1);

	ret = php_stream_tell(stream);
	if (ret == -1)	{
		RETURN_FALSE;
	}
	RETURN_LONG(ret);
}
/* }}} */

/* {{{ proto int fseek(resource fp, int offset [, int whence])
   Seek on a file pointer */
PHPAPI PHP_FUNCTION(fseek)
{
	zval *arg1;
	long arg2, whence = SEEK_SET;
	php_stream *stream;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl|l", &arg1, &arg2, &whence) == FAILURE) {
		RETURN_FALSE;
	}

	PHP_STREAM_TO_ZVAL(stream, &arg1);

	RETURN_LONG(php_stream_seek(stream, arg2, whence));
}
/* }}} */

/* {{{ php_mkdir
*/

/* DEPRECATED APIs: Use php_stream_mkdir() instead */
PHPAPI int php_mkdir_ex(char *dir, long mode, int options TSRMLS_DC)
{
	int ret;

	if (php_check_open_basedir(dir TSRMLS_CC)) {
		return -1;
	}

	if ((ret = VCWD_MKDIR(dir, (mode_t)mode)) < 0 && (options & REPORT_ERRORS)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", strerror(errno));
	}

	return ret;
}

PHPAPI int php_mkdir(char *dir, long mode TSRMLS_DC)
{
	return php_mkdir_ex(dir, mode, REPORT_ERRORS TSRMLS_CC);
}
/* }}} */

/* {{{ proto bool mkdir(string pathname [, int mode [, bool recursive [, resource context]]])
   Create a directory */
PHP_FUNCTION(mkdir)
{
	char *dir;
	int dir_len;
	zval *zcontext = NULL;
	long mode = 0777;
	zend_bool recursive = 0;
	php_stream_context *context;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "p|lbr", &dir, &dir_len, &mode, &recursive, &zcontext) == FAILURE) {
		RETURN_FALSE;
	}

	context = php_stream_context_from_zval(zcontext, 0);

	RETURN_BOOL(php_stream_mkdir(dir, mode, (recursive ? PHP_STREAM_MKDIR_RECURSIVE : 0) | REPORT_ERRORS, context));
}
/* }}} */

/* {{{ proto bool rmdir(string dirname[, resource context])
   Remove a directory */
PHP_FUNCTION(rmdir)
{
	char *dir;
	int dir_len;
	zval *zcontext = NULL;
	php_stream_context *context;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|r", &dir, &dir_len, &zcontext) == FAILURE) {
		RETURN_FALSE;
	}

	context = php_stream_context_from_zval(zcontext, 0);

	RETURN_BOOL(php_stream_rmdir(dir, REPORT_ERRORS, context));
}
/* }}} */

/* {{{ proto int readfile(string filename [, bool use_include_path[, resource context]])
   Output a file or a URL */
PHP_FUNCTION(readfile)
{
	char *filename;
	int filename_len;
	int size = 0;
	zend_bool use_include_path = 0;
	zval *zcontext = NULL;
	php_stream *stream;
	php_stream_context *context = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "p|br!", &filename, &filename_len, &use_include_path, &zcontext) == FAILURE) {
		RETURN_FALSE;
	}

	context = php_stream_context_from_zval(zcontext, 0);

	stream = php_stream_open_wrapper_ex(filename, "rb", (use_include_path ? USE_PATH : 0) | REPORT_ERRORS, NULL, context);
	if (stream) {
		size = php_stream_passthru(stream);
		php_stream_close(stream);
		RETURN_LONG(size);
	}

	RETURN_FALSE;
}
/* }}} */

/* {{{ proto int umask([int mask])
   Return or change the umask */
PHP_FUNCTION(umask)
{
	long arg1 = 0;
	int oldumask;

	oldumask = umask(077);

	if (BG(umask) == -1) {
		BG(umask) = oldumask;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &arg1) == FAILURE) {
		RETURN_FALSE;
	}

	if (ZEND_NUM_ARGS() == 0) {
		umask(oldumask);
	} else {
		umask(arg1);
	}

	RETURN_LONG(oldumask);
}
/* }}} */

/* {{{ proto int fpassthru(resource fp)
   Output all remaining data from a file pointer */
PHPAPI PHP_FUNCTION(fpassthru)
{
	zval *arg1;
	int size;
	php_stream *stream;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &arg1) == FAILURE) {
		RETURN_FALSE;
	}

	PHP_STREAM_TO_ZVAL(stream, &arg1);

	size = php_stream_passthru(stream);
	RETURN_LONG(size);
}
/* }}} */

/* {{{ proto bool rename(string old_name, string new_name[, resource context])
   Rename a file */
PHP_FUNCTION(rename)
{
	char *old_name, *new_name;
	int old_name_len, new_name_len;
	zval *zcontext = NULL;
	php_stream_wrapper *wrapper;
	php_stream_context *context;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "pp|r", &old_name, &old_name_len, &new_name, &new_name_len, &zcontext) == FAILURE) {
		RETURN_FALSE;
	}

	wrapper = php_stream_locate_url_wrapper(old_name, NULL, 0 TSRMLS_CC);

	if (!wrapper || !wrapper->wops) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to locate stream wrapper");
		RETURN_FALSE;
	}

	if (!wrapper->wops->rename) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s wrapper does not support renaming", wrapper->wops->label ? wrapper->wops->label : "Source");
		RETURN_FALSE;
	}

	if (wrapper != php_stream_locate_url_wrapper(new_name, NULL, 0 TSRMLS_CC)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot rename a file across wrapper types");
		RETURN_FALSE;
	}

	context = php_stream_context_from_zval(zcontext, 0);

	RETURN_BOOL(wrapper->wops->rename(wrapper, old_name, new_name, 0, context TSRMLS_CC));
}
/* }}} */

/* {{{ proto bool unlink(string filename[, context context])
   Delete a file */
PHP_FUNCTION(unlink)
{
	char *filename;
	int filename_len;
	php_stream_wrapper *wrapper;
	zval *zcontext = NULL;
	php_stream_context *context = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "p|r", &filename, &filename_len, &zcontext) == FAILURE) {
		RETURN_FALSE;
	}

	context = php_stream_context_from_zval(zcontext, 0);

	wrapper = php_stream_locate_url_wrapper(filename, NULL, 0 TSRMLS_CC);

	if (!wrapper || !wrapper->wops) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to locate stream wrapper");
		RETURN_FALSE;
	}

	if (!wrapper->wops->unlink) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s does not allow unlinking", wrapper->wops->label ? wrapper->wops->label : "Wrapper");
		RETURN_FALSE;
	}
	RETURN_BOOL(wrapper->wops->unlink(wrapper, filename, REPORT_ERRORS, context TSRMLS_CC));
}
/* }}} */

/* {{{ proto bool ftruncate(resource fp, int size)
   Truncate file to 'size' length */
PHP_NAMED_FUNCTION(php_if_ftruncate)
{
	zval *fp;
	long size;
	php_stream *stream;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &fp, &size) == FAILURE) {
		RETURN_FALSE;
	}

	PHP_STREAM_TO_ZVAL(stream, &fp);

	if (!php_stream_truncate_supported(stream)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can't truncate this stream!");
		RETURN_FALSE;
	}

	RETURN_BOOL(0 == php_stream_truncate_set_size(stream, size));
}
/* }}} */

/* {{{ proto array fstat(resource fp)
   Stat() on a filehandle */
PHP_NAMED_FUNCTION(php_if_fstat)
{
	zval *fp;
	zval *stat_dev, *stat_ino, *stat_mode, *stat_nlink, *stat_uid, *stat_gid, *stat_rdev,
		 *stat_size, *stat_atime, *stat_mtime, *stat_ctime, *stat_blksize, *stat_blocks;
	php_stream *stream;
	php_stream_statbuf stat_ssb;
	char *stat_sb_names[13] = {
		"dev", "ino", "mode", "nlink", "uid", "gid", "rdev",
		"size", "atime", "mtime", "ctime", "blksize", "blocks"
	};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &fp) == FAILURE) {
		RETURN_FALSE;
	}

	PHP_STREAM_TO_ZVAL(stream, &fp);

	if (php_stream_stat(stream, &stat_ssb)) {
		RETURN_FALSE;
	}

	array_init(return_value);

	MAKE_LONG_ZVAL_INCREF(stat_dev, stat_ssb.sb.st_dev);
	MAKE_LONG_ZVAL_INCREF(stat_ino, stat_ssb.sb.st_ino);
	MAKE_LONG_ZVAL_INCREF(stat_mode, stat_ssb.sb.st_mode);
	MAKE_LONG_ZVAL_INCREF(stat_nlink, stat_ssb.sb.st_nlink);
	MAKE_LONG_ZVAL_INCREF(stat_uid, stat_ssb.sb.st_uid);
	MAKE_LONG_ZVAL_INCREF(stat_gid, stat_ssb.sb.st_gid);
#ifdef HAVE_ST_RDEV
	MAKE_LONG_ZVAL_INCREF(stat_rdev, stat_ssb.sb.st_rdev);
#else
	MAKE_LONG_ZVAL_INCREF(stat_rdev, -1);
#endif
	MAKE_LONG_ZVAL_INCREF(stat_size, stat_ssb.sb.st_size);
	MAKE_LONG_ZVAL_INCREF(stat_atime, stat_ssb.sb.st_atime);
	MAKE_LONG_ZVAL_INCREF(stat_mtime, stat_ssb.sb.st_mtime);
	MAKE_LONG_ZVAL_INCREF(stat_ctime, stat_ssb.sb.st_ctime);
#ifdef HAVE_ST_BLKSIZE
	MAKE_LONG_ZVAL_INCREF(stat_blksize, stat_ssb.sb.st_blksize);
#else
	MAKE_LONG_ZVAL_INCREF(stat_blksize,-1);
#endif
#ifdef HAVE_ST_BLOCKS
	MAKE_LONG_ZVAL_INCREF(stat_blocks, stat_ssb.sb.st_blocks);
#else
	MAKE_LONG_ZVAL_INCREF(stat_blocks,-1);
#endif
	/* Store numeric indexes in propper order */
	zend_hash_next_index_insert(HASH_OF(return_value), (void *)&stat_dev, sizeof(zval *), NULL);
	zend_hash_next_index_insert(HASH_OF(return_value), (void *)&stat_ino, sizeof(zval *), NULL);
	zend_hash_next_index_insert(HASH_OF(return_value), (void *)&stat_mode, sizeof(zval *), NULL);
	zend_hash_next_index_insert(HASH_OF(return_value), (void *)&stat_nlink, sizeof(zval *), NULL);
	zend_hash_next_index_insert(HASH_OF(return_value), (void *)&stat_uid, sizeof(zval *), NULL);
	zend_hash_next_index_insert(HASH_OF(return_value), (void *)&stat_gid, sizeof(zval *), NULL);
	zend_hash_next_index_insert(HASH_OF(return_value), (void *)&stat_rdev, sizeof(zval *), NULL);
	zend_hash_next_index_insert(HASH_OF(return_value), (void *)&stat_size, sizeof(zval *), NULL);
	zend_hash_next_index_insert(HASH_OF(return_value), (void *)&stat_atime, sizeof(zval *), NULL);
	zend_hash_next_index_insert(HASH_OF(return_value), (void *)&stat_mtime, sizeof(zval *), NULL);
	zend_hash_next_index_insert(HASH_OF(return_value), (void *)&stat_ctime, sizeof(zval *), NULL);
	zend_hash_next_index_insert(HASH_OF(return_value), (void *)&stat_blksize, sizeof(zval *), NULL);
	zend_hash_next_index_insert(HASH_OF(return_value), (void *)&stat_blocks, sizeof(zval *), NULL);

	/* Store string indexes referencing the same zval*/
	zend_hash_update(HASH_OF(return_value), stat_sb_names[0], strlen(stat_sb_names[0])+1, (void *)&stat_dev, sizeof(zval *), NULL);
	zend_hash_update(HASH_OF(return_value), stat_sb_names[1], strlen(stat_sb_names[1])+1, (void *)&stat_ino, sizeof(zval *), NULL);
	zend_hash_update(HASH_OF(return_value), stat_sb_names[2], strlen(stat_sb_names[2])+1, (void *)&stat_mode, sizeof(zval *), NULL);
	zend_hash_update(HASH_OF(return_value), stat_sb_names[3], strlen(stat_sb_names[3])+1, (void *)&stat_nlink, sizeof(zval *), NULL);
	zend_hash_update(HASH_OF(return_value), stat_sb_names[4], strlen(stat_sb_names[4])+1, (void *)&stat_uid, sizeof(zval *), NULL);
	zend_hash_update(HASH_OF(return_value), stat_sb_names[5], strlen(stat_sb_names[5])+1, (void *)&stat_gid, sizeof(zval *), NULL);
	zend_hash_update(HASH_OF(return_value), stat_sb_names[6], strlen(stat_sb_names[6])+1, (void *)&stat_rdev, sizeof(zval *), NULL);
	zend_hash_update(HASH_OF(return_value), stat_sb_names[7], strlen(stat_sb_names[7])+1, (void *)&stat_size, sizeof(zval *), NULL);
	zend_hash_update(HASH_OF(return_value), stat_sb_names[8], strlen(stat_sb_names[8])+1, (void *)&stat_atime, sizeof(zval *), NULL);
	zend_hash_update(HASH_OF(return_value), stat_sb_names[9], strlen(stat_sb_names[9])+1, (void *)&stat_mtime, sizeof(zval *), NULL);
	zend_hash_update(HASH_OF(return_value), stat_sb_names[10], strlen(stat_sb_names[10])+1, (void *)&stat_ctime, sizeof(zval *), NULL);
	zend_hash_update(HASH_OF(return_value), stat_sb_names[11], strlen(stat_sb_names[11])+1, (void *)&stat_blksize, sizeof(zval *), NULL);
	zend_hash_update(HASH_OF(return_value), stat_sb_names[12], strlen(stat_sb_names[12])+1, (void *)&stat_blocks, sizeof(zval *), NULL);
}
/* }}} */

/* {{{ proto bool copy(string source_file, string destination_file [, resource context])
   Copy a file */
PHP_FUNCTION(copy)
{
	char *source, *target;
	int source_len, target_len;
	zval *zcontext = NULL;
	php_stream_context *context;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "pp|r", &source, &source_len, &target, &target_len, &zcontext) == FAILURE) {
		return;
	}

	if (php_check_open_basedir(source TSRMLS_CC)) {
		RETURN_FALSE;
	}

	context = php_stream_context_from_zval(zcontext, 0);

	if (php_copy_file_ctx(source, target, 0, context TSRMLS_CC) == SUCCESS) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ php_copy_file
 */
PHPAPI int php_copy_file(char *src, char *dest TSRMLS_DC)
{
	return php_copy_file_ctx(src, dest, 0, NULL TSRMLS_CC);
}
/* }}} */

/* {{{ php_copy_file_ex
 */
PHPAPI int php_copy_file_ex(char *src, char *dest, int src_flg TSRMLS_DC)
{
	return php_copy_file_ctx(src, dest, 0, NULL TSRMLS_CC);
}
/* }}} */

/* {{{ php_copy_file_ctx
 */
PHPAPI int php_copy_file_ctx(char *src, char *dest, int src_flg, php_stream_context *ctx TSRMLS_DC)
{
	php_stream *srcstream = NULL, *deststream = NULL;
	int ret = FAILURE;
	php_stream_statbuf src_s, dest_s;

	switch (php_stream_stat_path_ex(src, 0, &src_s, ctx)) {
		case -1:
			/* non-statable stream */
			goto safe_to_copy;
			break;
		case 0:
			break;
		default: /* failed to stat file, does not exist? */
			return ret;
	}
	if (S_ISDIR(src_s.sb.st_mode)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "The first argument to copy() function cannot be a directory");
		return FAILURE;
	}

	switch (php_stream_stat_path_ex(dest, PHP_STREAM_URL_STAT_QUIET | PHP_STREAM_URL_STAT_NOCACHE, &dest_s, ctx)) {
		case -1:
			/* non-statable stream */
			goto safe_to_copy;
			break;
		case 0:
			break;
		default: /* failed to stat file, does not exist? */
			return ret;
	}
	if (S_ISDIR(dest_s.sb.st_mode)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "The second argument to copy() function cannot be a directory");
		return FAILURE;
	}
	if (!src_s.sb.st_ino || !dest_s.sb.st_ino) {
		goto no_stat;
	}
	if (src_s.sb.st_ino == dest_s.sb.st_ino && src_s.sb.st_dev == dest_s.sb.st_dev) {
		return ret;
	} else {
		goto safe_to_copy;
	}
no_stat:
	{
		char *sp, *dp;
		int res;

		if ((sp = expand_filepath(src, NULL TSRMLS_CC)) == NULL) {
			return ret;
		}
		if ((dp = expand_filepath(dest, NULL TSRMLS_CC)) == NULL) {
			efree(sp);
			goto safe_to_copy;
		}

		res =
#ifndef PHP_WIN32
			!strcmp(sp, dp);
#else
			!strcasecmp(sp, dp);
#endif

		efree(sp);
		efree(dp);
		if (res) {
			return ret;
		}
	}
safe_to_copy:

	srcstream = php_stream_open_wrapper_ex(src, "rb", src_flg | REPORT_ERRORS, NULL, ctx);

	if (!srcstream) {
		return ret;
	}

	deststream = php_stream_open_wrapper_ex(dest, "wb", REPORT_ERRORS, NULL, ctx);

	if (srcstream && deststream) {
		ret = php_stream_copy_to_stream_ex(srcstream, deststream, PHP_STREAM_COPY_ALL, NULL);
	}
	if (srcstream) {
		php_stream_close(srcstream);
	}
	if (deststream) {
		php_stream_close(deststream);
	}
	return ret;
}
/* }}} */

/* {{{ proto string fread(resource fp, int length)
   Binary-safe file read */
PHPAPI PHP_FUNCTION(fread)
{
	zval *arg1;
	long len;
	php_stream *stream;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &arg1, &len) == FAILURE) {
		RETURN_FALSE;
	}

	PHP_STREAM_TO_ZVAL(stream, &arg1);

	if (len <= 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Length parameter must be greater than 0");
		RETURN_FALSE;
	}

	Z_STRVAL_P(return_value) = emalloc(len + 1);
	Z_STRLEN_P(return_value) = php_stream_read(stream, Z_STRVAL_P(return_value), len);

	/* needed because recv/read/gzread doesnt put a null at the end*/
	Z_STRVAL_P(return_value)[Z_STRLEN_P(return_value)] = 0;
	Z_TYPE_P(return_value) = IS_STRING;
}
/* }}} */

static const char *php_fgetcsv_lookup_trailing_spaces(const char *ptr, size_t len, const char delimiter TSRMLS_DC) /* {{{ */
{
	int inc_len;
	unsigned char last_chars[2] = { 0, 0 };

	while (len > 0) {
		inc_len = (*ptr == '\0' ? 1: php_mblen(ptr, len));
		switch (inc_len) {
			case -2:
			case -1:
				inc_len = 1;
				php_ignore_value(php_mblen(NULL, 0));
				break;
			case 0:
				goto quit_loop;
			case 1:
			default:
				last_chars[0] = last_chars[1];
				last_chars[1] = *ptr;
				break;
		}
		ptr += inc_len;
		len -= inc_len;
	}
quit_loop:
	switch (last_chars[1]) {
		case '\n':
			if (last_chars[0] == '\r') {
				return ptr - 2;
			}
			/* break is omitted intentionally */
		case '\r':
			return ptr - 1;
	}
	return ptr;
}
/* }}} */

#define FPUTCSV_FLD_CHK(c) memchr(Z_STRVAL(field), c, Z_STRLEN(field))

/* {{{ proto int fputcsv(resource fp, array fields [, string delimiter [, string enclosure [, string escape_char]]])
   Format line as CSV and write to file pointer */
PHP_FUNCTION(fputcsv)
{
	char delimiter = ',';	 /* allow this to be set as parameter */
	char enclosure = '"';	 /* allow this to be set as parameter */
	char escape_char = '\\'; /* allow this to be set as parameter */
	php_stream *stream;
	zval *fp = NULL, *fields = NULL;
	int ret;
	char *delimiter_str = NULL, *enclosure_str = NULL, *escape_str = NULL;
	int delimiter_str_len = 0, enclosure_str_len = 0, escape_str_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ra|sss",
			&fp, &fields, &delimiter_str, &delimiter_str_len,
			&enclosure_str, &enclosure_str_len,
			&escape_str, &escape_str_len) == FAILURE) {
		return;
	}

	if (delimiter_str != NULL) {
		/* Make sure that there is at least one character in string */
		if (delimiter_str_len < 1) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "delimiter must be a character");
			RETURN_FALSE;
		} else if (delimiter_str_len > 1) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "delimiter must be a single character");
		}

		/* use first character from string */
		delimiter = *delimiter_str;
	}

	if (enclosure_str != NULL) {
		if (enclosure_str_len < 1) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "enclosure must be a character");
			RETURN_FALSE;
		} else if (enclosure_str_len > 1) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "enclosure must be a single character");
		}
		/* use first character from string */
		enclosure = *enclosure_str;
	}

	if (escape_str != NULL) {
		if (escape_str_len < 1) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "escape must be a character");
			RETURN_FALSE;
		} else if (escape_str_len > 1) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "escape must be a single character");
		}
		/* use first character from string */
		escape_char = *escape_str;
	}

	PHP_STREAM_TO_ZVAL(stream, &fp);

	ret = php_fputcsv(stream, fields, delimiter, enclosure, escape_char TSRMLS_CC);
	RETURN_LONG(ret);
}
/* }}} */

/* {{{ PHPAPI int php_fputcsv(php_stream *stream, zval *fields, char delimiter, char enclosure, char escape_char TSRMLS_DC) */
PHPAPI int php_fputcsv(php_stream *stream, zval *fields, char delimiter, char enclosure, char escape_char TSRMLS_DC)
{
	int count, i = 0, ret;
	zval **field_tmp = NULL, field;
	smart_str csvline = {0};
	HashPosition pos;

	count = zend_hash_num_elements(Z_ARRVAL_P(fields));
	zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(fields), &pos);
	while (zend_hash_get_current_data_ex(Z_ARRVAL_P(fields), (void **) &field_tmp, &pos) == SUCCESS) {
		field = **field_tmp;

		if (Z_TYPE_PP(field_tmp) != IS_STRING) {
			zval_copy_ctor(&field);
			convert_to_string(&field);
		}

		/* enclose a field that contains a delimiter, an enclosure character, or a newline */
		if (FPUTCSV_FLD_CHK(delimiter) ||
			FPUTCSV_FLD_CHK(enclosure) ||
			FPUTCSV_FLD_CHK(escape_char) ||
			FPUTCSV_FLD_CHK('\n') ||
			FPUTCSV_FLD_CHK('\r') ||
			FPUTCSV_FLD_CHK('\t') ||
			FPUTCSV_FLD_CHK(' ')
		) {
			char *ch = Z_STRVAL(field);
			char *end = ch + Z_STRLEN(field);
			int escaped = 0;

			smart_str_appendc(&csvline, enclosure);
			while (ch < end) {
				if (*ch == escape_char) {
					escaped = 1;
				} else if (!escaped && *ch == enclosure) {
					smart_str_appendc(&csvline, enclosure);
				} else {
					escaped = 0;
				}
				smart_str_appendc(&csvline, *ch);
				ch++;
			}
			smart_str_appendc(&csvline, enclosure);
		} else {
			smart_str_appendl(&csvline, Z_STRVAL(field), Z_STRLEN(field));
		}

		if (++i != count) {
			smart_str_appendl(&csvline, &delimiter, 1);
		}
		zend_hash_move_forward_ex(Z_ARRVAL_P(fields), &pos);

		if (Z_TYPE_PP(field_tmp) != IS_STRING) {
			zval_dtor(&field);
		}
	}

	smart_str_appendc(&csvline, '\n');
	smart_str_0(&csvline);

	ret = php_stream_write(stream, csvline.c, csvline.len);

	smart_str_free(&csvline);

	return ret;
}
/* }}} */

/* {{{ proto array fgetcsv(resource fp [,int length [, string delimiter [, string enclosure [, string escape]]]])
   Get line from file pointer and parse for CSV fields */
PHP_FUNCTION(fgetcsv)
{
	char delimiter = ',';	/* allow this to be set as parameter */
	char enclosure = '"';	/* allow this to be set as parameter */
	char escape = '\\';

	/* first section exactly as php_fgetss */

	long len = 0;
	size_t buf_len;
	char *buf;
	php_stream *stream;

	{
		zval *fd, **len_zv = NULL;
		char *delimiter_str = NULL;
		int delimiter_str_len = 0;
		char *enclosure_str = NULL;
		int enclosure_str_len = 0;
		char *escape_str = NULL;
		int escape_str_len = 0;

		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|Zsss",
			&fd, &len_zv, &delimiter_str, &delimiter_str_len,
			&enclosure_str, &enclosure_str_len,
			&escape_str, &escape_str_len) == FAILURE
		) {
			return;
		}

		if (delimiter_str != NULL) {
			/* Make sure that there is at least one character in string */
			if (delimiter_str_len < 1) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "delimiter must be a character");
				RETURN_FALSE;
			} else if (delimiter_str_len > 1) {
				php_error_docref(NULL TSRMLS_CC, E_NOTICE, "delimiter must be a single character");
			}

			/* use first character from string */
			delimiter = delimiter_str[0];
		}

		if (enclosure_str != NULL) {
			if (enclosure_str_len < 1) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "enclosure must be a character");
				RETURN_FALSE;
			} else if (enclosure_str_len > 1) {
				php_error_docref(NULL TSRMLS_CC, E_NOTICE, "enclosure must be a single character");
			}

			/* use first character from string */
			enclosure = enclosure_str[0];
		}

		if (escape_str != NULL) {
			if (escape_str_len < 1) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "escape must be character");
				RETURN_FALSE;
			} else if (escape_str_len > 1) {
				php_error_docref(NULL TSRMLS_CC, E_NOTICE, "escape must be a single character");
			}

			escape = escape_str[0];
		}

		if (len_zv != NULL && Z_TYPE_PP(len_zv) != IS_NULL) {
			convert_to_long_ex(len_zv);
			len = Z_LVAL_PP(len_zv);
			if (len < 0) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Length parameter may not be negative");
				RETURN_FALSE;
			} else if (len == 0) {
				len = -1;
			}
		} else {
			len = -1;
		}

		PHP_STREAM_TO_ZVAL(stream, &fd);
	}

	if (len < 0) {
		if ((buf = php_stream_get_line(stream, NULL, 0, &buf_len)) == NULL) {
			RETURN_FALSE;
		}
	} else {
		buf = emalloc(len + 1);
		if (php_stream_get_line(stream, buf, len + 1, &buf_len) == NULL) {
			efree(buf);
			RETURN_FALSE;
		}
	}

	php_fgetcsv(stream, delimiter, enclosure, escape, buf_len, buf, return_value TSRMLS_CC);
}
/* }}} */

PHPAPI void php_fgetcsv(php_stream *stream, char delimiter, char enclosure, char escape_char, size_t buf_len, char *buf, zval *return_value TSRMLS_DC) /* {{{ */
{
	char *temp, *tptr, *bptr, *line_end, *limit;
	size_t temp_len, line_end_len;
	int inc_len;
	zend_bool first_field = 1;

	/* initialize internal state */
	php_ignore_value(php_mblen(NULL, 0));

	/* Now into new section that parses buf for delimiter/enclosure fields */

	/* Strip trailing space from buf, saving end of line in case required for enclosure field */

	bptr = buf;
	tptr = (char *)php_fgetcsv_lookup_trailing_spaces(buf, buf_len, delimiter TSRMLS_CC);
	line_end_len = buf_len - (size_t)(tptr - buf);
	line_end = limit = tptr;

	/* reserve workspace for building each individual field */
	temp_len = buf_len;
	temp = emalloc(temp_len + line_end_len + 1);

	/* Initialize return array */
	array_init(return_value);

	/* Main loop to read CSV fields */
	/* NB this routine will return a single null entry for a blank line */

	do {
		char *comp_end, *hunk_begin;

		tptr = temp;

		inc_len = (bptr < limit ? (*bptr == '\0' ? 1: php_mblen(bptr, limit - bptr)): 0);
		if (inc_len == 1) {
			char *tmp = bptr;
			while ((*tmp != delimiter) && isspace((int)*(unsigned char *)tmp)) {
				tmp++;
  			}
			if (*tmp == enclosure) {
				bptr = tmp;
			}
  		}

		if (first_field && bptr == line_end) {
			add_next_index_null(return_value);
			break;
		}
		first_field = 0;
		/* 2. Read field, leaving bptr pointing at start of next field */
		if (inc_len != 0 && *bptr == enclosure) {
			int state = 0;

			bptr++;	/* move on to first character in field */
			hunk_begin = bptr;

			/* 2A. handle enclosure delimited field */
			for (;;) {
				switch (inc_len) {
					case 0:
						switch (state) {
							case 2:
								memcpy(tptr, hunk_begin, bptr - hunk_begin - 1);
								tptr += (bptr - hunk_begin - 1);
								hunk_begin = bptr;
								goto quit_loop_2;

							case 1:
								memcpy(tptr, hunk_begin, bptr - hunk_begin);
								tptr += (bptr - hunk_begin);
								hunk_begin = bptr;
								/* break is omitted intentionally */

							case 0: {
								char *new_buf;
								size_t new_len;
								char *new_temp;

								if (hunk_begin != line_end) {
									memcpy(tptr, hunk_begin, bptr - hunk_begin);
									tptr += (bptr - hunk_begin);
									hunk_begin = bptr;
								}

								/* add the embedded line end to the field */
								memcpy(tptr, line_end, line_end_len);
								tptr += line_end_len;

								if (stream == NULL) {
									goto quit_loop_2;
								} else if ((new_buf = php_stream_get_line(stream, NULL, 0, &new_len)) == NULL) {
									/* we've got an unterminated enclosure,
									 * assign all the data from the start of
									 * the enclosure to end of data to the
									 * last element */
									if ((size_t)temp_len > (size_t)(limit - buf)) {
										goto quit_loop_2;
									}
									zval_dtor(return_value);
									RETVAL_FALSE;
									goto out;
								}
								temp_len += new_len;
								new_temp = erealloc(temp, temp_len);
								tptr = new_temp + (size_t)(tptr - temp);
								temp = new_temp;

								efree(buf);
								buf_len = new_len;
								bptr = buf = new_buf;
								hunk_begin = buf;

								line_end = limit = (char *)php_fgetcsv_lookup_trailing_spaces(buf, buf_len, delimiter TSRMLS_CC);
								line_end_len = buf_len - (size_t)(limit - buf);

								state = 0;
							} break;
						}
						break;

					case -2:
					case -1:
						php_ignore_value(php_mblen(NULL, 0));
						/* break is omitted intentionally */
					case 1:
						/* we need to determine if the enclosure is
						 * 'real' or is it escaped */
						switch (state) {
							case 1: /* escaped */
								bptr++;
								state = 0;
								break;
							case 2: /* embedded enclosure ? let's check it */
								if (*bptr != enclosure) {
									/* real enclosure */
									memcpy(tptr, hunk_begin, bptr - hunk_begin - 1);
									tptr += (bptr - hunk_begin - 1);
									hunk_begin = bptr;
									goto quit_loop_2;
								}
								memcpy(tptr, hunk_begin, bptr - hunk_begin);
								tptr += (bptr - hunk_begin);
								bptr++;
								hunk_begin = bptr;
								state = 0;
								break;
							default:
								if (*bptr == enclosure) {
									state = 2;
								} else if (*bptr == escape_char) {
									state = 1;
								}
								bptr++;
								break;
						}
						break;

					default:
						switch (state) {
							case 2:
								/* real enclosure */
								memcpy(tptr, hunk_begin, bptr - hunk_begin - 1);
								tptr += (bptr - hunk_begin - 1);
								hunk_begin = bptr;
								goto quit_loop_2;
							case 1:
								bptr += inc_len;
								memcpy(tptr, hunk_begin, bptr - hunk_begin);
								tptr += (bptr - hunk_begin);
								hunk_begin = bptr;
								break;
							default:
								bptr += inc_len;
								break;
						}
						break;
				}
				inc_len = (bptr < limit ? (*bptr == '\0' ? 1: php_mblen(bptr, limit - bptr)): 0);
			}

		quit_loop_2:
			/* look up for a delimiter */
			for (;;) {
				switch (inc_len) {
					case 0:
						goto quit_loop_3;

					case -2:
					case -1:
						inc_len = 1;
						php_ignore_value(php_mblen(NULL, 0));
						/* break is omitted intentionally */
					case 1:
						if (*bptr == delimiter) {
							goto quit_loop_3;
						}
						break;
					default:
						break;
				}
				bptr += inc_len;
				inc_len = (bptr < limit ? (*bptr == '\0' ? 1: php_mblen(bptr, limit - bptr)): 0);
			}

		quit_loop_3:
			memcpy(tptr, hunk_begin, bptr - hunk_begin);
			tptr += (bptr - hunk_begin);
			bptr += inc_len;
			comp_end = tptr;
		} else {
			/* 2B. Handle non-enclosure field */

			hunk_begin = bptr;

			for (;;) {
				switch (inc_len) {
					case 0:
						goto quit_loop_4;
					case -2:
					case -1:
						inc_len = 1;
						php_ignore_value(php_mblen(NULL, 0));
						/* break is omitted intentionally */
					case 1:
						if (*bptr == delimiter) {
							goto quit_loop_4;
						}
						break;
					default:
						break;
				}
				bptr += inc_len;
				inc_len = (bptr < limit ? (*bptr == '\0' ? 1: php_mblen(bptr, limit - bptr)): 0);
			}
		quit_loop_4:
			memcpy(tptr, hunk_begin, bptr - hunk_begin);
			tptr += (bptr - hunk_begin);

			comp_end = (char *)php_fgetcsv_lookup_trailing_spaces(temp, tptr - temp, delimiter TSRMLS_CC);
			if (*bptr == delimiter) {
				bptr++;
			}
		}

		/* 3. Now pass our field back to php */
		*comp_end = '\0';
		add_next_index_stringl(return_value, temp, comp_end - temp, 1);
	} while (inc_len > 0);

out:
	efree(temp);
	if (stream) {
		efree(buf);
	}
}
/* }}} */

#if (!defined(__BEOS__) && !defined(NETWARE) && HAVE_REALPATH) || defined(ZTS)
/* {{{ proto string realpath(string path)
   Return the resolved path */
PHP_FUNCTION(realpath)
{
	char *filename;
	int filename_len;
	char resolved_path_buff[MAXPATHLEN];

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "p", &filename, &filename_len) == FAILURE) {
		return;
	}

	if (VCWD_REALPATH(filename, resolved_path_buff)) {
		if (php_check_open_basedir(resolved_path_buff TSRMLS_CC)) {
			RETURN_FALSE;
		}

#ifdef ZTS
		if (VCWD_ACCESS(resolved_path_buff, F_OK)) {
			RETURN_FALSE;
		}
#endif
		RETURN_STRING(resolved_path_buff, 1);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */
#endif

/* See http://www.w3.org/TR/html4/intro/sgmltut.html#h-3.2.2 */
#define PHP_META_HTML401_CHARS "-_.:"

/* {{{ php_next_meta_token
   Tokenizes an HTML file for get_meta_tags */
php_meta_tags_token php_next_meta_token(php_meta_tags_data *md TSRMLS_DC)
{
	int ch = 0, compliment;
	char buff[META_DEF_BUFSIZE + 1];

	memset((void *)buff, 0, META_DEF_BUFSIZE + 1);

	while (md->ulc || (!php_stream_eof(md->stream) && (ch = php_stream_getc(md->stream)))) {
		if (php_stream_eof(md->stream)) {
			break;
		}

		if (md->ulc) {
			ch = md->lc;
			md->ulc = 0;
		}

		switch (ch) {
			case '<':
				return TOK_OPENTAG;
				break;

			case '>':
				return TOK_CLOSETAG;
				break;

			case '=':
				return TOK_EQUAL;
				break;
			case '/':
				return TOK_SLASH;
				break;

			case '\'':
			case '"':
				compliment = ch;
				md->token_len = 0;
				while (!php_stream_eof(md->stream) && (ch = php_stream_getc(md->stream)) && ch != compliment && ch != '<' && ch != '>') {
					buff[(md->token_len)++] = ch;

					if (md->token_len == META_DEF_BUFSIZE) {
						break;
					}
				}

				if (ch == '<' || ch == '>') {
					/* Was just an apostrohpe */
					md->ulc = 1;
					md->lc = ch;
				}

				/* We don't need to alloc unless we are in a meta tag */
				if (md->in_meta) {
					md->token_data = (char *) emalloc(md->token_len + 1);
					memcpy(md->token_data, buff, md->token_len+1);
				}

				return TOK_STRING;
				break;

			case '\n':
			case '\r':
			case '\t':
				break;

			case ' ':
				return TOK_SPACE;
				break;

			default:
				if (isalnum(ch)) {
					md->token_len = 0;
					buff[(md->token_len)++] = ch;
					while (!php_stream_eof(md->stream) && (ch = php_stream_getc(md->stream)) && (isalnum(ch) || strchr(PHP_META_HTML401_CHARS, ch))) {
						buff[(md->token_len)++] = ch;

						if (md->token_len == META_DEF_BUFSIZE) {
							break;
						}
					}

					/* This is ugly, but we have to replace ungetc */
					if (!isalpha(ch) && ch != '-') {
						md->ulc = 1;
						md->lc = ch;
					}

					md->token_data = (char *) emalloc(md->token_len + 1);
					memcpy(md->token_data, buff, md->token_len+1);

					return TOK_ID;
				} else {
					return TOK_OTHER;
				}
				break;
		}
	}

	return TOK_EOF;
}
/* }}} */

#ifdef HAVE_FNMATCH
/* {{{ proto bool fnmatch(string pattern, string filename [, int flags])
   Match filename against pattern */
PHP_FUNCTION(fnmatch)
{
	char *pattern, *filename;
	int pattern_len, filename_len;
	long flags = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "pp|l", &pattern, &pattern_len, &filename, &filename_len, &flags) == FAILURE) {
		return;
	}

	if (filename_len >= MAXPATHLEN) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Filename exceeds the maximum allowed length of %d characters", MAXPATHLEN);
		RETURN_FALSE;
	}
	if (pattern_len >= MAXPATHLEN) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Pattern exceeds the maximum allowed length of %d characters", MAXPATHLEN);
		RETURN_FALSE;
	}

	RETURN_BOOL( ! fnmatch( pattern, filename, flags ));
}
/* }}} */
#endif

/* {{{ proto string sys_get_temp_dir()
   Returns directory path used for temporary files */
PHP_FUNCTION(sys_get_temp_dir)
{
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	RETURN_STRING((char *)php_get_temporary_directory(TSRMLS_C), 1);
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
