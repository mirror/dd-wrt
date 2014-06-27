/*
  +----------------------------------------------------------------------+
  | phar php single-file executable PHP extension                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 2006-2014 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt.                                 |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Gregory Beaver <cellog@php.net>                             |
  |          Marcus Boerger <helly@php.net>                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

BEGIN_EXTERN_C()
int phar_wrapper_mkdir(php_stream_wrapper *wrapper, char *url_from, int mode, int options, php_stream_context *context TSRMLS_DC);
int phar_wrapper_rmdir(php_stream_wrapper *wrapper, char *url, int options, php_stream_context *context TSRMLS_DC);

#ifdef PHAR_DIRSTREAM
php_url* phar_parse_url(php_stream_wrapper *wrapper, char *filename, char *mode, int options TSRMLS_DC);

/* directory handlers */
static size_t phar_dir_write(php_stream *stream, const char *buf, size_t count TSRMLS_DC);
static size_t phar_dir_read( php_stream *stream, char *buf, size_t count TSRMLS_DC);
static int    phar_dir_close(php_stream *stream, int close_handle TSRMLS_DC);
static int    phar_dir_flush(php_stream *stream TSRMLS_DC);
static int    phar_dir_seek( php_stream *stream, off_t offset, int whence, off_t *newoffset TSRMLS_DC);
#else
php_stream* phar_wrapper_open_dir(php_stream_wrapper *wrapper, char *path, char *mode, int options, char **opened_path, php_stream_context *context STREAMS_DC TSRMLS_DC);
#endif
END_EXTERN_C()

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
