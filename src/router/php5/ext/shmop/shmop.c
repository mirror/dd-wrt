/*
   +----------------------------------------------------------------------+
   | PHP version 5                                                        |
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
   | Authors: Slava Poliakov <hackie@prohost.org>                         |
   |          Ilia Alshanetsky <ilia@prohost.org>                         |
   +----------------------------------------------------------------------+
 */
/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "php_shmop.h"
# ifndef PHP_WIN32
# include <sys/ipc.h>
# include <sys/shm.h>
#else
#include "tsrm_win32.h"
#endif


#if HAVE_SHMOP

#include "ext/standard/info.h"

#ifdef ZTS
int shmop_globals_id;
#else
php_shmop_globals shmop_globals;
#endif

int shm_type;

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_shmop_open, 0, 0, 4)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, flags)
	ZEND_ARG_INFO(0, mode)
	ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_shmop_read, 0, 0, 3)
	ZEND_ARG_INFO(0, shmid)
	ZEND_ARG_INFO(0, start)
	ZEND_ARG_INFO(0, count)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_shmop_close, 0, 0, 1)
	ZEND_ARG_INFO(0, shmid)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_shmop_size, 0, 0, 1)
	ZEND_ARG_INFO(0, shmid)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_shmop_write, 0, 0, 3)
	ZEND_ARG_INFO(0, shmid)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_shmop_delete, 0, 0, 1)
	ZEND_ARG_INFO(0, shmid)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ shmop_functions[] 
 */
const zend_function_entry shmop_functions[] = {
	PHP_FE(shmop_open, 		arginfo_shmop_open)
	PHP_FE(shmop_read, 		arginfo_shmop_read)
	PHP_FE(shmop_close, 	arginfo_shmop_close)
	PHP_FE(shmop_size, 		arginfo_shmop_size)
	PHP_FE(shmop_write, 	arginfo_shmop_write)
	PHP_FE(shmop_delete, 	arginfo_shmop_delete)
	PHP_FE_END
};
/* }}} */

/* {{{ shmop_module_entry
 */
zend_module_entry shmop_module_entry = {
	STANDARD_MODULE_HEADER,
	"shmop",
	shmop_functions,
	PHP_MINIT(shmop),
	NULL,
	NULL,
	NULL,
	PHP_MINFO(shmop),
	NO_VERSION_YET,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_SHMOP
ZEND_GET_MODULE(shmop)
#endif

#define PHP_SHMOP_GET_RES \
	shmop = zend_list_find(shmid, &type);	\
	if (!shmop) {	\
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "no shared memory segment with an id of [%lu]", shmid);	\
		RETURN_FALSE;	\
	} else if (type != shm_type) {	\
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "not a shmop resource");	\
		RETURN_FALSE;	\
	}	\

/* {{{ rsclean
 */
static void rsclean(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	struct php_shmop *shmop = (struct php_shmop *)rsrc->ptr;

	shmdt(shmop->addr);
	efree(shmop);
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(shmop)
{
	shm_type = zend_register_list_destructors_ex(rsclean, NULL, "shmop", module_number);
	
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(shmop)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "shmop support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ proto int shmop_open (int key, string flags, int mode, int size)
   gets and attaches a shared memory segment */
PHP_FUNCTION(shmop_open)
{
	long key, mode, size;
	struct php_shmop *shmop;	
	struct shmid_ds shm;
	int rsid;
	char *flags;
	int flags_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lsll", &key, &flags, &flags_len, &mode, &size) == FAILURE) {
		return;
	}

	if (flags_len != 1) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s is not a valid flag", flags);
		RETURN_FALSE;
	}

	shmop = emalloc(sizeof(struct php_shmop));
	memset(shmop, 0, sizeof(struct php_shmop));

	shmop->key = key;
	shmop->shmflg |= mode;

	switch (flags[0]) 
	{
		case 'a':
			shmop->shmatflg |= SHM_RDONLY;
			break;
		case 'c':
			shmop->shmflg |= IPC_CREAT;
			shmop->size = size;
			break;
		case 'n':
			shmop->shmflg |= (IPC_CREAT | IPC_EXCL);
			shmop->size = size;
			break;	
		case 'w':
			/* noop 
				shm segment is being opened for read & write
				will fail if segment does not exist
			*/
			break;
		default:
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "invalid access mode");
			goto err;
	}

	if (shmop->shmflg & IPC_CREAT && shmop->size < 1) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Shared memory segment size must be greater than zero");
		goto err;
	}

	shmop->shmid = shmget(shmop->key, shmop->size, shmop->shmflg);
	if (shmop->shmid == -1) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "unable to attach or create shared memory segment");
		goto err;
	}

	if (shmctl(shmop->shmid, IPC_STAT, &shm)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "unable to get shared memory segment information");
		goto err;
	}	

	shmop->addr = shmat(shmop->shmid, 0, shmop->shmatflg);
	if (shmop->addr == (char*) -1) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "unable to attach to shared memory segment");
		goto err;
	}

	shmop->size = shm.shm_segsz;

	rsid = zend_list_insert(shmop, shm_type TSRMLS_CC);
	RETURN_LONG(rsid);
err:
	efree(shmop);
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto string shmop_read (int shmid, int start, int count)
   reads from a shm segment */
PHP_FUNCTION(shmop_read)
{
	long shmid, start, count;
	struct php_shmop *shmop;
	int type;
	char *startaddr;
	int bytes;
	char *return_string;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lll", &shmid, &start, &count) == FAILURE) {
		return;
	}

	PHP_SHMOP_GET_RES

	if (start < 0 || start > shmop->size) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "start is out of range");
		RETURN_FALSE;
	}

	if (count < 0 || start > (INT_MAX - count) || start + count > shmop->size) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "count is out of range");
		RETURN_FALSE;
	}

	startaddr = shmop->addr + start;
	bytes = count ? count : shmop->size - start;

	return_string = emalloc(bytes+1);
	memcpy(return_string, startaddr, bytes);
	return_string[bytes] = 0;

	RETURN_STRINGL(return_string, bytes, 0);
}
/* }}} */

/* {{{ proto void shmop_close (int shmid)
   closes a shared memory segment */
PHP_FUNCTION(shmop_close)
{
	long shmid;
	struct php_shmop *shmop;
	int type;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &shmid) == FAILURE) {
		return;
	}

	PHP_SHMOP_GET_RES

	zend_list_delete(shmid);
}
/* }}} */

/* {{{ proto int shmop_size (int shmid)
   returns the shm size */
PHP_FUNCTION(shmop_size)
{
	long shmid;
	struct php_shmop *shmop;
	int type;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &shmid) == FAILURE) {
		return;
	}

	PHP_SHMOP_GET_RES

	RETURN_LONG(shmop->size);
}
/* }}} */

/* {{{ proto int shmop_write (int shmid, string data, int offset)
   writes to a shared memory segment */
PHP_FUNCTION(shmop_write)
{
	struct php_shmop *shmop;
	int type;
	int writesize;
	long shmid, offset;
	char *data;
	int data_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lsl", &shmid, &data, &data_len, &offset) == FAILURE) {
		return;
	}

	PHP_SHMOP_GET_RES

	if ((shmop->shmatflg & SHM_RDONLY) == SHM_RDONLY) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "trying to write to a read only segment");
		RETURN_FALSE;
	}

	if (offset < 0 || offset > shmop->size) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "offset out of range");
		RETURN_FALSE;
	}

	writesize = (data_len < shmop->size - offset) ? data_len : shmop->size - offset;
	memcpy(shmop->addr + offset, data, writesize);

	RETURN_LONG(writesize);
}
/* }}} */

/* {{{ proto bool shmop_delete (int shmid)
   mark segment for deletion */
PHP_FUNCTION(shmop_delete)
{
	long shmid;
	struct php_shmop *shmop;
	int type;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &shmid) == FAILURE) {
		return;
	}

	PHP_SHMOP_GET_RES

	if (shmctl(shmop->shmid, IPC_RMID, NULL)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "can't mark segment for deletion (are you the owner?)");
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

#endif	/* HAVE_SHMOP */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
