/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 2006-2014 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Georg Richter <georg@mysql.com>                             |
  |          Andrey Hristov <andrey@mysql.com>                           |
  |          Ulf Wendel <uwendel@mysql.com>                              |
  +----------------------------------------------------------------------+
*/

/* $Id: mysqlnd_debug.c 309303 2011-03-16 12:42:59Z andrey $ */
#include "php.h"
#include "mysqlnd.h"
#include "mysqlnd_priv.h"
#include "mysqlnd_debug.h"
#include "mysqlnd_wireprotocol.h"
#include "mysqlnd_statistics.h"


static const char mysqlnd_emalloc_name[]	= "_mysqlnd_emalloc";
static const char mysqlnd_pemalloc_name[]	= "_mysqlnd_pemalloc";
static const char mysqlnd_ecalloc_name[]	= "_mysqlnd_ecalloc";
static const char mysqlnd_pecalloc_name[]	= "_mysqlnd_pecalloc";
static const char mysqlnd_erealloc_name[]	= "_mysqlnd_erealloc";
static const char mysqlnd_perealloc_name[]	= "_mysqlnd_perealloc";
static const char mysqlnd_efree_name[]		= "_mysqlnd_efree";
static const char mysqlnd_pefree_name[]		= "_mysqlnd_pefree";
static const char mysqlnd_malloc_name[]		= "_mysqlnd_malloc";
static const char mysqlnd_calloc_name[]		= "_mysqlnd_calloc";
static const char mysqlnd_realloc_name[]	= "_mysqlnd_realloc";
static const char mysqlnd_free_name[]		= "_mysqlnd_free";
static const char mysqlnd_pestrndup_name[]	= "_mysqlnd_pestrndup";
static const char mysqlnd_pestrdup_name[]	= "_mysqlnd_pestrdup";

PHPAPI const char * mysqlnd_debug_std_no_trace_funcs[] =
{
	mysqlnd_emalloc_name,
	mysqlnd_ecalloc_name,
	mysqlnd_efree_name,
	mysqlnd_erealloc_name,
	mysqlnd_pemalloc_name,
	mysqlnd_pecalloc_name,
	mysqlnd_pefree_name,
	mysqlnd_perealloc_name,
	mysqlnd_malloc_name,
	mysqlnd_calloc_name,
	mysqlnd_realloc_name,
	mysqlnd_free_name,
	mysqlnd_pestrndup_name,
	mysqlnd_read_header_name,
	mysqlnd_read_body_name,
	NULL /* must be always last */
};


#if ZEND_DEBUG
#else
#define __zend_orig_filename "/unknown/unknown"
#define __zend_orig_lineno   0
#endif

#define REAL_SIZE(s) (collect_memory_statistics? (s) + sizeof(size_t) : (s))
#define REAL_PTR(p) (collect_memory_statistics && (p)? (((char *)(p)) - sizeof(size_t)) : (p))
#define FAKE_PTR(p) (collect_memory_statistics && (p)? (((char *)(p)) + sizeof(size_t)) : (p))

/* {{{ _mysqlnd_emalloc */
void * _mysqlnd_emalloc(size_t size MYSQLND_MEM_D)
{
	void *ret;
	zend_bool collect_memory_statistics = MYSQLND_G(collect_memory_statistics);
#if PHP_DEBUG
	long * threshold = &MYSQLND_G(debug_emalloc_fail_threshold);
#endif
	TRACE_ALLOC_ENTER(mysqlnd_emalloc_name);

#if PHP_DEBUG
	{
		char * fn = strrchr(__zend_orig_filename, PHP_DIR_SEPARATOR); 
		TRACE_ALLOC_INF_FMT("file=%-15s line=%4d", fn? fn + 1:__zend_orig_filename, __zend_orig_lineno);
	}
#endif

#if PHP_DEBUG
	/* -1 is also "true" */
	if (*threshold) {
#endif
		ret = _emalloc(REAL_SIZE(size) ZEND_FILE_LINE_CC ZEND_FILE_LINE_ORIG_RELAY_CC);
#if PHP_DEBUG
		--*threshold;
	} else if (*threshold == 0) {
		ret = NULL;
	}
#endif

	TRACE_ALLOC_INF_FMT("size=%lu ptr=%p", size, ret);

	if (ret && collect_memory_statistics) {
		*(size_t *) ret = size;
		MYSQLND_INC_GLOBAL_STATISTIC_W_VALUE2(STAT_MEM_EMALLOC_COUNT, 1, STAT_MEM_EMALLOC_AMOUNT, size);
	}
	TRACE_ALLOC_RETURN(FAKE_PTR(ret));
}
/* }}} */


/* {{{ _mysqlnd_pemalloc */
void * _mysqlnd_pemalloc(size_t size, zend_bool persistent MYSQLND_MEM_D)
{
	void *ret;
	zend_bool collect_memory_statistics = MYSQLND_G(collect_memory_statistics);
#if PHP_DEBUG
	long * threshold = persistent? &MYSQLND_G(debug_malloc_fail_threshold):&MYSQLND_G(debug_emalloc_fail_threshold);
#endif
	TRACE_ALLOC_ENTER(mysqlnd_pemalloc_name);

#if PHP_DEBUG
	{
		char * fn = strrchr(__zend_orig_filename, PHP_DIR_SEPARATOR); 
		TRACE_ALLOC_INF_FMT("file=%-15s line=%4d", fn? fn + 1:__zend_orig_filename, __zend_orig_lineno);
	}
#endif

#if PHP_DEBUG
	/* -1 is also "true" */
	if (*threshold) {
#endif
		ret = (persistent) ? __zend_malloc(REAL_SIZE(size)) : _emalloc(REAL_SIZE(size) ZEND_FILE_LINE_CC ZEND_FILE_LINE_ORIG_RELAY_CC);
#if PHP_DEBUG
		--*threshold;
	} else if (*threshold == 0) {
		ret = NULL;
	}
#endif

	TRACE_ALLOC_INF_FMT("size=%lu ptr=%p persistent=%u", size, ret, persistent);

	if (ret && collect_memory_statistics) {
		enum mysqlnd_collected_stats s1 = persistent? STAT_MEM_MALLOC_COUNT:STAT_MEM_EMALLOC_COUNT;
		enum mysqlnd_collected_stats s2 = persistent? STAT_MEM_MALLOC_AMOUNT:STAT_MEM_EMALLOC_AMOUNT;
		*(size_t *) ret = size;
		MYSQLND_INC_GLOBAL_STATISTIC_W_VALUE2(s1, 1, s2, size);
	}

	TRACE_ALLOC_RETURN(FAKE_PTR(ret));
}
/* }}} */


/* {{{ _mysqlnd_ecalloc */
void * _mysqlnd_ecalloc(unsigned int nmemb, size_t size MYSQLND_MEM_D)
{
	void *ret;
	zend_bool collect_memory_statistics = MYSQLND_G(collect_memory_statistics);
#if PHP_DEBUG
	long * threshold = &MYSQLND_G(debug_ecalloc_fail_threshold);
#endif
	TRACE_ALLOC_ENTER(mysqlnd_ecalloc_name);

#if PHP_DEBUG
	{
		char * fn = strrchr(__zend_orig_filename, PHP_DIR_SEPARATOR); 
		TRACE_ALLOC_INF_FMT("file=%-15s line=%4d", fn? fn + 1:__zend_orig_filename, __zend_orig_lineno);
	}
#endif
	TRACE_ALLOC_INF_FMT("before: %lu", zend_memory_usage(FALSE TSRMLS_CC));

#if PHP_DEBUG
	/* -1 is also "true" */
	if (*threshold) {
#endif
		ret = _ecalloc(nmemb, REAL_SIZE(size) ZEND_FILE_LINE_CC ZEND_FILE_LINE_ORIG_RELAY_CC);
#if PHP_DEBUG
		--*threshold;
	} else if (*threshold == 0) {
		ret = NULL;
	}
#endif

	TRACE_ALLOC_INF_FMT("after : %lu", zend_memory_usage(FALSE TSRMLS_CC));
	TRACE_ALLOC_INF_FMT("size=%lu ptr=%p", size, ret);
	if (ret && collect_memory_statistics) {
		*(size_t *) ret = size;
		MYSQLND_INC_GLOBAL_STATISTIC_W_VALUE2(STAT_MEM_ECALLOC_COUNT, 1, STAT_MEM_ECALLOC_AMOUNT, size);
	}
	TRACE_ALLOC_RETURN(FAKE_PTR(ret));
}
/* }}} */


/* {{{ _mysqlnd_pecalloc */
void * _mysqlnd_pecalloc(unsigned int nmemb, size_t size, zend_bool persistent MYSQLND_MEM_D)
{
	void *ret;
	zend_bool collect_memory_statistics = MYSQLND_G(collect_memory_statistics);
#if PHP_DEBUG
	long * threshold = persistent? &MYSQLND_G(debug_calloc_fail_threshold):&MYSQLND_G(debug_ecalloc_fail_threshold);
#endif
	TRACE_ALLOC_ENTER(mysqlnd_pecalloc_name);
#if PHP_DEBUG
	{
		char * fn = strrchr(__zend_orig_filename, PHP_DIR_SEPARATOR); 
		TRACE_ALLOC_INF_FMT("file=%-15s line=%4d", fn? fn + 1:__zend_orig_filename, __zend_orig_lineno);
	}
#endif

#if PHP_DEBUG
	/* -1 is also "true" */
	if (*threshold) {
#endif
		ret = (persistent) ? __zend_calloc(nmemb, REAL_SIZE(size)) : _ecalloc(nmemb, REAL_SIZE(size) ZEND_FILE_LINE_CC ZEND_FILE_LINE_ORIG_RELAY_CC);
#if PHP_DEBUG
		--*threshold;
	} else if (*threshold == 0) {
		ret = NULL;
	}
#endif

	TRACE_ALLOC_INF_FMT("size=%lu ptr=%p", size, ret);

	if (ret && collect_memory_statistics) {
		enum mysqlnd_collected_stats s1 = persistent? STAT_MEM_CALLOC_COUNT:STAT_MEM_ECALLOC_COUNT;
		enum mysqlnd_collected_stats s2 = persistent? STAT_MEM_CALLOC_AMOUNT:STAT_MEM_ECALLOC_AMOUNT;
		*(size_t *) ret = size;
		MYSQLND_INC_GLOBAL_STATISTIC_W_VALUE2(s1, 1, s2, size);
	}

	TRACE_ALLOC_RETURN(FAKE_PTR(ret));
}
/* }}} */


/* {{{ _mysqlnd_erealloc */
void * _mysqlnd_erealloc(void *ptr, size_t new_size MYSQLND_MEM_D)
{
	void *ret;
	zend_bool collect_memory_statistics = MYSQLND_G(collect_memory_statistics);
	size_t old_size = collect_memory_statistics && ptr? *(size_t *) (((char*)ptr) - sizeof(size_t)) : 0;
#if PHP_DEBUG
	long * threshold = &MYSQLND_G(debug_erealloc_fail_threshold);
#endif
	TRACE_ALLOC_ENTER(mysqlnd_erealloc_name);

#if PHP_DEBUG
	{
		char * fn = strrchr(__zend_orig_filename, PHP_DIR_SEPARATOR); 
		TRACE_ALLOC_INF_FMT("file=%-15s line=%4d", fn? fn + 1:__zend_orig_filename, __zend_orig_lineno);
	}
#endif
	TRACE_ALLOC_INF_FMT("ptr=%p old_size=%lu, new_size=%lu", ptr, old_size, new_size); 

#if PHP_DEBUG
	/* -1 is also "true" */
	if (*threshold) {
#endif
		ret = _erealloc(REAL_PTR(ptr), REAL_SIZE(new_size), 0 ZEND_FILE_LINE_CC ZEND_FILE_LINE_ORIG_RELAY_CC);
#if PHP_DEBUG
		--*threshold;
	} else if (*threshold == 0) {
		ret = NULL;
	}
#endif

	TRACE_ALLOC_INF_FMT("new_ptr=%p", (char*)ret);
	if (ret && collect_memory_statistics) {
		*(size_t *) ret = new_size;
		MYSQLND_INC_GLOBAL_STATISTIC_W_VALUE2(STAT_MEM_EREALLOC_COUNT, 1, STAT_MEM_EREALLOC_AMOUNT, new_size);
	}
	TRACE_ALLOC_RETURN(FAKE_PTR(ret));
}
/* }}} */


/* {{{ _mysqlnd_perealloc */
void * _mysqlnd_perealloc(void *ptr, size_t new_size, zend_bool persistent MYSQLND_MEM_D)
{
	void *ret;
	zend_bool collect_memory_statistics = MYSQLND_G(collect_memory_statistics);
	size_t old_size = collect_memory_statistics && ptr? *(size_t *) (((char*)ptr) - sizeof(size_t)) : 0;
#if PHP_DEBUG
	long * threshold = persistent? &MYSQLND_G(debug_realloc_fail_threshold):&MYSQLND_G(debug_erealloc_fail_threshold);
#endif
	TRACE_ALLOC_ENTER(mysqlnd_perealloc_name);

#if PHP_DEBUG
	{
		char * fn = strrchr(__zend_orig_filename, PHP_DIR_SEPARATOR); 
		TRACE_ALLOC_INF_FMT("file=%-15s line=%4d", fn? fn + 1:__zend_orig_filename, __zend_orig_lineno);
	}
#endif
	TRACE_ALLOC_INF_FMT("ptr=%p old_size=%lu new_size=%lu   persistent=%u", ptr, old_size, new_size, persistent); 

#if PHP_DEBUG
	/* -1 is also "true" */
	if (*threshold) {
#endif
		ret = perealloc(REAL_PTR(ptr), REAL_SIZE(new_size), persistent);
#if PHP_DEBUG
		--*threshold;
	} else if (*threshold == 0) {
		ret = NULL;
	}
#endif

	TRACE_ALLOC_INF_FMT("new_ptr=%p", (char*)ret);

	if (ret && collect_memory_statistics) {
		enum mysqlnd_collected_stats s1 = persistent? STAT_MEM_REALLOC_COUNT:STAT_MEM_EREALLOC_COUNT;
		enum mysqlnd_collected_stats s2 = persistent? STAT_MEM_REALLOC_AMOUNT:STAT_MEM_EREALLOC_AMOUNT;
		*(size_t *) ret = new_size;
		MYSQLND_INC_GLOBAL_STATISTIC_W_VALUE2(s1, 1, s2, new_size);
	}
	TRACE_ALLOC_RETURN(FAKE_PTR(ret));
}
/* }}} */


/* {{{ _mysqlnd_efree */
void _mysqlnd_efree(void *ptr MYSQLND_MEM_D)
{
	size_t free_amount = 0;
	zend_bool collect_memory_statistics = MYSQLND_G(collect_memory_statistics);
	TRACE_ALLOC_ENTER(mysqlnd_efree_name);

#if PHP_DEBUG
	{
		char * fn = strrchr(__zend_orig_filename, PHP_DIR_SEPARATOR); 
		TRACE_ALLOC_INF_FMT("file=%-15s line=%4d", fn? fn + 1:__zend_orig_filename, __zend_orig_lineno);
	}
#endif
	TRACE_ALLOC_INF_FMT("ptr=%p", ptr); 

	if (ptr) {
		if (collect_memory_statistics) {
			free_amount = *(size_t *)(((char*)ptr) - sizeof(size_t));
			TRACE_ALLOC_INF_FMT("ptr=%p size=%u", ((char*)ptr) - sizeof(size_t), (unsigned int) free_amount);
		}
		_efree(REAL_PTR(ptr) ZEND_FILE_LINE_CC ZEND_FILE_LINE_ORIG_RELAY_CC);
	}

	if (collect_memory_statistics) {
		MYSQLND_INC_GLOBAL_STATISTIC_W_VALUE2(STAT_MEM_EFREE_COUNT, 1, STAT_MEM_EFREE_AMOUNT, free_amount);
	}
	TRACE_ALLOC_VOID_RETURN;
}
/* }}} */


/* {{{ _mysqlnd_pefree */
void _mysqlnd_pefree(void *ptr, zend_bool persistent MYSQLND_MEM_D)
{
	size_t free_amount = 0;
	zend_bool collect_memory_statistics = MYSQLND_G(collect_memory_statistics);
	TRACE_ALLOC_ENTER(mysqlnd_pefree_name);

#if PHP_DEBUG
	{
		char * fn = strrchr(__zend_orig_filename, PHP_DIR_SEPARATOR); 
		TRACE_ALLOC_INF_FMT("file=%-15s line=%4d", fn? fn + 1:__zend_orig_filename, __zend_orig_lineno);
	}
#endif
	TRACE_ALLOC_INF_FMT("ptr=%p persistent=%u", ptr, persistent); 

	if (ptr) {
		if (collect_memory_statistics) {
			free_amount = *(size_t *)(((char*)ptr) - sizeof(size_t));
			TRACE_ALLOC_INF_FMT("ptr=%p size=%u", ((char*)ptr) - sizeof(size_t), (unsigned int) free_amount);
		}
		(persistent) ? free(REAL_PTR(ptr)) : _efree(REAL_PTR(ptr) ZEND_FILE_LINE_CC ZEND_FILE_LINE_ORIG_RELAY_CC);
	}

	if (collect_memory_statistics) {
		MYSQLND_INC_GLOBAL_STATISTIC_W_VALUE2(persistent? STAT_MEM_FREE_COUNT:STAT_MEM_EFREE_COUNT, 1,
											  persistent? STAT_MEM_FREE_AMOUNT:STAT_MEM_EFREE_AMOUNT, free_amount);
	}
	TRACE_ALLOC_VOID_RETURN;
}
/* }}} */


/* {{{ _mysqlnd_malloc */
void * _mysqlnd_malloc(size_t size MYSQLND_MEM_D)
{
	void *ret;
	zend_bool collect_memory_statistics = MYSQLND_G(collect_memory_statistics);
#if PHP_DEBUG
	long * threshold = &MYSQLND_G(debug_malloc_fail_threshold);
#endif
	TRACE_ALLOC_ENTER(mysqlnd_malloc_name);

#if PHP_DEBUG
	{
		char * fn = strrchr(__zend_orig_filename, PHP_DIR_SEPARATOR); 
		TRACE_ALLOC_INF_FMT("file=%-15s line=%4d", fn? fn + 1:__zend_orig_filename, __zend_orig_lineno);
	}
#endif

#if PHP_DEBUG
	/* -1 is also "true" */
	if (*threshold) {
#endif
		ret = malloc(REAL_SIZE(size));
#if PHP_DEBUG
		--*threshold;
	} else if (*threshold == 0) {
		ret = NULL;
	}
#endif

	TRACE_ALLOC_INF_FMT("size=%lu ptr=%p", size, ret);
	if (ret && collect_memory_statistics) {
		*(size_t *) ret = size;
		MYSQLND_INC_GLOBAL_STATISTIC_W_VALUE2(STAT_MEM_MALLOC_COUNT, 1, STAT_MEM_MALLOC_AMOUNT, size);
	}
	TRACE_ALLOC_RETURN(FAKE_PTR(ret));
}
/* }}} */


/* {{{ _mysqlnd_calloc */
void * _mysqlnd_calloc(unsigned int nmemb, size_t size MYSQLND_MEM_D)
{
	void *ret;
	zend_bool collect_memory_statistics = MYSQLND_G(collect_memory_statistics);
#if PHP_DEBUG
	long * threshold = &MYSQLND_G(debug_calloc_fail_threshold);
#endif
	TRACE_ALLOC_ENTER(mysqlnd_calloc_name);

#if PHP_DEBUG
	{
		char * fn = strrchr(__zend_orig_filename, PHP_DIR_SEPARATOR); 
		TRACE_ALLOC_INF_FMT("file=%-15s line=%4d", fn? fn + 1:__zend_orig_filename, __zend_orig_lineno);
	}
#endif

#if PHP_DEBUG
	/* -1 is also "true" */
	if (*threshold) {
#endif
		ret = calloc(nmemb, REAL_SIZE(size));
#if PHP_DEBUG
		--*threshold;
	} else if (*threshold == 0) {
		ret = NULL;
	}
#endif

	TRACE_ALLOC_INF_FMT("size=%lu ptr=%p", size, ret);
	if (ret && collect_memory_statistics) {
		*(size_t *) ret = size;
		MYSQLND_INC_GLOBAL_STATISTIC_W_VALUE2(STAT_MEM_CALLOC_COUNT, 1, STAT_MEM_CALLOC_AMOUNT, size);
	}
	TRACE_ALLOC_RETURN(FAKE_PTR(ret));
}
/* }}} */


/* {{{ _mysqlnd_realloc */
void * _mysqlnd_realloc(void *ptr, size_t new_size MYSQLND_MEM_D)
{
	void *ret;
	zend_bool collect_memory_statistics = MYSQLND_G(collect_memory_statistics);
#if PHP_DEBUG
	long * threshold = &MYSQLND_G(debug_realloc_fail_threshold);
#endif
	TRACE_ALLOC_ENTER(mysqlnd_realloc_name);

#if PHP_DEBUG
	{
		char * fn = strrchr(__zend_orig_filename, PHP_DIR_SEPARATOR); 
		TRACE_ALLOC_INF_FMT("file=%-15s line=%4d", fn? fn + 1:__zend_orig_filename, __zend_orig_lineno);
	}
#endif
	TRACE_ALLOC_INF_FMT("ptr=%p new_size=%lu ", new_size, ptr); 
	TRACE_ALLOC_INF_FMT("before: %lu", zend_memory_usage(TRUE TSRMLS_CC));

#if PHP_DEBUG
	/* -1 is also "true" */
	if (*threshold) {
#endif
		ret = realloc(REAL_PTR(ptr), REAL_SIZE(new_size));
#if PHP_DEBUG
		--*threshold;
	} else if (*threshold == 0) {
		ret = NULL;
	}
#endif

	TRACE_ALLOC_INF_FMT("new_ptr=%p", (char*)ret);

	if (ret && collect_memory_statistics) {
		*(size_t *) ret = new_size;
		MYSQLND_INC_GLOBAL_STATISTIC_W_VALUE2(STAT_MEM_REALLOC_COUNT, 1, STAT_MEM_REALLOC_AMOUNT, new_size);
	}
	TRACE_ALLOC_RETURN(FAKE_PTR(ret));
}
/* }}} */


/* {{{ _mysqlnd_free */
void _mysqlnd_free(void *ptr MYSQLND_MEM_D)
{
	size_t free_amount = 0;
	zend_bool collect_memory_statistics = MYSQLND_G(collect_memory_statistics);
	TRACE_ALLOC_ENTER(mysqlnd_free_name);

#if PHP_DEBUG
	{
		char * fn = strrchr(__zend_orig_filename, PHP_DIR_SEPARATOR); 
		TRACE_ALLOC_INF_FMT("file=%-15s line=%4d", fn? fn + 1:__zend_orig_filename, __zend_orig_lineno);
	}
#endif
	TRACE_ALLOC_INF_FMT("ptr=%p", ptr); 

	if (ptr) {
		if (collect_memory_statistics) {
			free_amount = *(size_t *)(((char*)ptr) - sizeof(size_t));
			TRACE_ALLOC_INF_FMT("ptr=%p size=%u", ((char*)ptr) - sizeof(size_t), (unsigned int) free_amount);
		}
		free(REAL_PTR(ptr));
	}

	if (collect_memory_statistics) {
		MYSQLND_INC_GLOBAL_STATISTIC_W_VALUE2(STAT_MEM_FREE_COUNT, 1, STAT_MEM_FREE_AMOUNT, free_amount);
	}
	TRACE_ALLOC_VOID_RETURN;
}
/* }}} */

#define SMART_STR_START_SIZE 2048
#define SMART_STR_PREALLOC 512
#include "ext/standard/php_smart_str.h"


/* {{{ _mysqlnd_pestrndup */
char * _mysqlnd_pestrndup(const char * const ptr, size_t length, zend_bool persistent MYSQLND_MEM_D)
{
	char * ret;
	zend_bool collect_memory_statistics = MYSQLND_G(collect_memory_statistics);
	TRACE_ALLOC_ENTER(mysqlnd_pestrndup_name);

#if PHP_DEBUG
	{
		char * fn = strrchr(__zend_orig_filename, PHP_DIR_SEPARATOR); 
		TRACE_ALLOC_INF_FMT("file=%-15s line=%4d", fn? fn + 1:__zend_orig_filename, __zend_orig_lineno);
	}
#endif
	TRACE_ALLOC_INF_FMT("ptr=%p", ptr); 

	ret = (persistent) ? __zend_malloc(REAL_SIZE(length + 1)) : _emalloc(REAL_SIZE(length + 1) ZEND_FILE_LINE_CC ZEND_FILE_LINE_ORIG_RELAY_CC);
	{
		size_t l = length;
		char * p = (char *) ptr;
		char * dest = (char *) FAKE_PTR(ret);
		while (*p && l--) {
			*dest++ = *p++;
		}
		*dest = '\0';
	}

	if (collect_memory_statistics) {
		*(size_t *) ret = length;
		MYSQLND_INC_GLOBAL_STATISTIC(persistent? STAT_MEM_STRNDUP_COUNT : STAT_MEM_ESTRNDUP_COUNT);
	}

	TRACE_ALLOC_RETURN(FAKE_PTR(ret));
}
/* }}} */


/* {{{ _mysqlnd_pestrdup */
char * _mysqlnd_pestrdup(const char * const ptr, zend_bool persistent MYSQLND_MEM_D)
{
	char * ret;
	smart_str tmp_str = {0, 0, 0};
	const char * p = ptr;
	zend_bool collect_memory_statistics = MYSQLND_G(collect_memory_statistics);
	TRACE_ALLOC_ENTER(mysqlnd_pestrdup_name);
#if PHP_DEBUG
	{
		char * fn = strrchr(__zend_orig_filename, PHP_DIR_SEPARATOR); 
		TRACE_ALLOC_INF_FMT("file=%-15s line=%4d", fn? fn + 1:__zend_orig_filename, __zend_orig_lineno);
	}
#endif
	TRACE_ALLOC_INF_FMT("ptr=%p", ptr);
	do {
		smart_str_appendc(&tmp_str, *p);
	} while (*p++);

	ret = (persistent) ? __zend_malloc(tmp_str.len + sizeof(size_t)) : _emalloc(REAL_SIZE(tmp_str.len + sizeof(size_t)) ZEND_FILE_LINE_CC ZEND_FILE_LINE_ORIG_RELAY_CC);
	memcpy(FAKE_PTR(ret), tmp_str.c, tmp_str.len);

	if (ret && collect_memory_statistics) {
		*(size_t *) ret = tmp_str.len;
		MYSQLND_INC_GLOBAL_STATISTIC(persistent? STAT_MEM_STRDUP_COUNT : STAT_MEM_ESTRDUP_COUNT);
	}
	smart_str_free(&tmp_str);

	TRACE_ALLOC_RETURN(FAKE_PTR(ret));
}
/* }}} */


/* {{{ _mysqlnd_sprintf */
PHPAPI int _mysqlnd_sprintf(char ** pbuf, size_t max_len, const char *format, ...)
{
	int len;
	va_list ap;
	va_start(ap, format);
	len = vspprintf(pbuf, max_len, format, ap);
	va_end(ap);
	return len;
}
/* }}} */


/* {{{ _mysqlnd_sprintf_free */
PHPAPI void _mysqlnd_sprintf_free(char * p)
{
	efree(p);
}
/* }}} */

/* {{{ _mysqlnd_vsprintf */
PHPAPI int _mysqlnd_vsprintf(char ** pbuf, size_t max_len, const char * format, va_list ap)
{
	return vspprintf(pbuf, max_len, format, ap);
}
/* }}} */


#define MYSQLND_DEBUG_MEMORY 1

#if MYSQLND_DEBUG_MEMORY == 0

/* {{{ mysqlnd_zend_mm_emalloc */
static void * mysqlnd_zend_mm_emalloc(size_t size MYSQLND_MEM_D)
{
	return emalloc(size);
}
/* }}} */


/* {{{ mysqlnd_zend_mm_pemalloc */
static void * mysqlnd_zend_mm_pemalloc(size_t size, zend_bool persistent MYSQLND_MEM_D)
{
	return pemalloc(size, persistent);
}
/* }}} */


/* {{{ mysqlnd_zend_mm_ecalloc */
static void * mysqlnd_zend_mm_ecalloc(unsigned int nmemb, size_t size MYSQLND_MEM_D)
{
	return ecalloc(nmemb, size);
}
/* }}} */


/* {{{ mysqlnd_zend_mm_pecalloc */
static void * mysqlnd_zend_mm_pecalloc(unsigned int nmemb, size_t size, zend_bool persistent MYSQLND_MEM_D)
{
	return pecalloc(nmemb, size, persistent);
}
/* }}} */


/* {{{ mysqlnd_zend_mm_erealloc */
static void * mysqlnd_zend_mm_erealloc(void *ptr, size_t new_size MYSQLND_MEM_D)
{
	return erealloc(ptr, new_size);
}
/* }}} */


/* {{{ mysqlnd_zend_mm_perealloc */
static void * mysqlnd_zend_mm_perealloc(void *ptr, size_t new_size, zend_bool persistent MYSQLND_MEM_D)
{
	return perealloc(ptr, new_size, persistent);
}
/* }}} */


/* {{{ mysqlnd_zend_mm_efree */
static void mysqlnd_zend_mm_efree(void * ptr MYSQLND_MEM_D)
{
	efree(ptr);
}
/* }}} */


/* {{{ mysqlnd_zend_mm_pefree */
static void mysqlnd_zend_mm_pefree(void * ptr, zend_bool persistent MYSQLND_MEM_D)
{
	pefree(ptr, persistent);
}
/* }}} */


/* {{{ mysqlnd_zend_mm_malloc */
static void * mysqlnd_zend_mm_malloc(size_t size MYSQLND_MEM_D)
{
	return malloc(size);
}
/* }}} */


/* {{{ mysqlnd_zend_mm_calloc */
static void * mysqlnd_zend_mm_calloc(unsigned int nmemb, size_t size MYSQLND_MEM_D)
{
	return calloc(nmemb, size);
}
/* }}} */


/* {{{ mysqlnd_zend_mm_realloc */
static void * mysqlnd_zend_mm_realloc(void * ptr, size_t new_size MYSQLND_MEM_D)
{
	return realloc(ptr, new_size);
}
/* }}} */


/* {{{ mysqlnd_zend_mm_free */
static void mysqlnd_zend_mm_free(void * ptr MYSQLND_MEM_D)
{
	free(ptr);
}
/* }}} */


/* {{{ mysqlnd_zend_mm_pestrndup */
static char * mysqlnd_zend_mm_pestrndup(const char * const ptr, size_t length, zend_bool persistent MYSQLND_MEM_D)
{
	return pestrndup(ptr, length, persistent);
}
/* }}} */


/* {{{ mysqlnd_zend_mm_pestrdup */
static char * mysqlnd_zend_mm_pestrdup(const char * const ptr, zend_bool persistent MYSQLND_MEM_D)
{
	return pestrdup(ptr, persistent);
}
/* }}} */

#endif


PHPAPI struct st_mysqlnd_allocator_methods mysqlnd_allocator = 
{
#if MYSQLND_DEBUG_MEMORY
	_mysqlnd_emalloc,
	_mysqlnd_pemalloc,
	_mysqlnd_ecalloc,
	_mysqlnd_pecalloc,
	_mysqlnd_erealloc,
	_mysqlnd_perealloc,
	_mysqlnd_efree,
	_mysqlnd_pefree,
	_mysqlnd_malloc,
	_mysqlnd_calloc,
	_mysqlnd_realloc,
	_mysqlnd_free,
	_mysqlnd_pestrndup,
	_mysqlnd_pestrdup,
	_mysqlnd_sprintf,
	_mysqlnd_vsprintf,
	_mysqlnd_sprintf_free
#else
	mysqlnd_zend_mm_emalloc,
	mysqlnd_zend_mm_pemalloc,
	mysqlnd_zend_mm_ecalloc,
	mysqlnd_zend_mm_pecalloc,
	mysqlnd_zend_mm_erealloc,
	mysqlnd_zend_mm_perealloc,
	mysqlnd_zend_mm_efree,
	mysqlnd_zend_mm_pefree,
	mysqlnd_zend_mm_malloc,
	mysqlnd_zend_mm_calloc,
	mysqlnd_zend_mm_realloc,
	mysqlnd_zend_mm_free,
	mysqlnd_zend_mm_pestrndup,
	mysqlnd_zend_mm_pestrdup
	sprintf,
	mysqlnd_zend_mm_efree,
#endif
};


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
