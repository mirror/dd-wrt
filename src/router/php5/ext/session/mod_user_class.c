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
   | Author: Arpad Ray <arpad@php.net>                                    |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

#include "php.h"
#include "php_session.h"

#define PS_SANITY_CHECK						\
	if (PS(default_mod) == NULL) {				\
		php_error_docref(NULL TSRMLS_CC, E_CORE_ERROR, "Cannot call default session handler"); \
		RETURN_FALSE;						\
	}							

#define PS_SANITY_CHECK_IS_OPEN				\
	PS_SANITY_CHECK; \
	if (!PS(mod_user_is_open)) {			\
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Parent session handler is not open");	\
		RETURN_FALSE;						\
	}							

/* {{{ proto bool SessionHandler::open(string save_path, string session_name)
   Wraps the old open handler */
PHP_METHOD(SessionHandler, open)
{
	char *save_path = NULL, *session_name = NULL;
	int save_path_len, session_name_len;

	PS_SANITY_CHECK;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &save_path, &save_path_len, &session_name, &session_name_len) == FAILURE) {
		return;
	}

	PS(mod_user_is_open) = 1;
	RETVAL_BOOL(SUCCESS == PS(default_mod)->s_open(&PS(mod_data), save_path, session_name TSRMLS_CC));
}
/* }}} */

/* {{{ proto bool SessionHandler::close()
   Wraps the old close handler */
PHP_METHOD(SessionHandler, close)
{
	PS_SANITY_CHECK_IS_OPEN;

	// don't return on failure, since not closing the default handler
	// could result in memory leaks or other nasties
	zend_parse_parameters_none();
	
	PS(mod_user_is_open) = 0;
	RETVAL_BOOL(SUCCESS == PS(default_mod)->s_close(&PS(mod_data) TSRMLS_CC));
}
/* }}} */

/* {{{ proto bool SessionHandler::read(string id)
   Wraps the old read handler */
PHP_METHOD(SessionHandler, read)
{
	char *key, *val;
	int key_len, val_len;

	PS_SANITY_CHECK_IS_OPEN;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &key_len) == FAILURE) {
		return;
	}

	if (PS(default_mod)->s_read(&PS(mod_data), key, &val, &val_len TSRMLS_CC) == FAILURE) {
		RETVAL_FALSE;
		return;
	}

	RETVAL_STRINGL(val, val_len, 1);
	efree(val);
	return;
}
/* }}} */

/* {{{ proto bool SessionHandler::write(string id, string data)
   Wraps the old write handler */
PHP_METHOD(SessionHandler, write)
{
	char *key, *val;
	int key_len, val_len;

	PS_SANITY_CHECK_IS_OPEN;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &key, &key_len, &val, &val_len) == FAILURE) {
		return;
	}

	RETVAL_BOOL(SUCCESS == PS(default_mod)->s_write(&PS(mod_data), key, val, val_len TSRMLS_CC));
}
/* }}} */

/* {{{ proto bool SessionHandler::destroy(string id)
   Wraps the old destroy handler */
PHP_METHOD(SessionHandler, destroy)
{
	char *key;
	int key_len;

	PS_SANITY_CHECK_IS_OPEN;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &key_len) == FAILURE) {
		return;
	}
	
	RETVAL_BOOL(SUCCESS == PS(default_mod)->s_destroy(&PS(mod_data), key TSRMLS_CC));
}
/* }}} */

/* {{{ proto bool SessionHandler::gc(int maxlifetime)
   Wraps the old gc handler */
PHP_METHOD(SessionHandler, gc)
{
	long maxlifetime;
	int nrdels;

	PS_SANITY_CHECK_IS_OPEN;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &maxlifetime) == FAILURE) {
		return;
	}
	
	RETVAL_BOOL(SUCCESS == PS(default_mod)->s_gc(&PS(mod_data), maxlifetime, &nrdels TSRMLS_CC));
}
/* }}} */

/* {{{ proto char SessionHandler::create_sid()
   Wraps the old create_sid handler */
PHP_METHOD(SessionHandler, create_sid)
{
	char *id;

	if (zend_parse_parameters_none() == FAILURE) {
	    return;
	}

	id = PS(default_mod)->s_create_sid(&PS(mod_data), NULL TSRMLS_CC);

	RETURN_STRING(id, 0);
}
/* }}} */
