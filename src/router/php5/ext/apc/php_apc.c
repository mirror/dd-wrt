/*
  +----------------------------------------------------------------------+
  | APC                                                                  |
  +----------------------------------------------------------------------+
  | Copyright (c) 2006-2010 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Daniel Cowgill <dcowgill@communityconnect.com>              |
  |          Rasmus Lerdorf <rasmus@php.net>                             |
  +----------------------------------------------------------------------+

   This software was contributed to PHP by Community Connect Inc. in 2002
   and revised in 2005 by Yahoo! Inc. to add support for PHP 5.1.
   Future revisions and derivatives of this source code must acknowledge
   Community Connect Inc. as the original contributor of this module by
   leaving this note intact in the source code.

   All other licensing and usage conditions are those of the PHP Group.

 */

/* $Id: php_apc.c 303642 2010-09-21 15:42:01Z kalle $ */

#include "apc_zend.h"
#include "apc_cache.h"
#include "apc_iterator.h"
#include "apc_main.h"
#include "apc_sma.h"
#include "apc_lock.h"
#include "apc_bin.h"
#include "php_globals.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/file.h"
#include "ext/standard/flock_compat.h"
#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif
#include "SAPI.h"
#include "rfc1867.h"
#include "php_apc.h"
#include "ext/standard/md5.h"

#if HAVE_SIGACTION
#include "apc_signal.h"
#endif

/* {{{ PHP_FUNCTION declarations */
PHP_FUNCTION(apc_cache_info);
PHP_FUNCTION(apc_clear_cache);
PHP_FUNCTION(apc_sma_info);
PHP_FUNCTION(apc_store);
PHP_FUNCTION(apc_fetch);
PHP_FUNCTION(apc_delete);
PHP_FUNCTION(apc_delete_file);
PHP_FUNCTION(apc_compile_file);
PHP_FUNCTION(apc_define_constants);
PHP_FUNCTION(apc_load_constants);
PHP_FUNCTION(apc_add);
PHP_FUNCTION(apc_inc);
PHP_FUNCTION(apc_dec);
PHP_FUNCTION(apc_cas);
PHP_FUNCTION(apc_bin_dump);
PHP_FUNCTION(apc_bin_load);
PHP_FUNCTION(apc_bin_dumpfile);
PHP_FUNCTION(apc_bin_loadfile);
PHP_FUNCTION(apc_exists);
/* }}} */

/* {{{ ZEND_DECLARE_MODULE_GLOBALS(apc) */
ZEND_DECLARE_MODULE_GLOBALS(apc)

/* True globals */
apc_cache_t* apc_cache = NULL;
apc_cache_t* apc_user_cache = NULL;

static void php_apc_init_globals(zend_apc_globals* apc_globals TSRMLS_DC)
{
    apc_globals->filters = NULL;
    apc_globals->compiled_filters = NULL;
    apc_globals->initialized = 0;
    apc_globals->cache_stack = apc_stack_create(0 TSRMLS_CC);
    apc_globals->cache_by_default = 1;
    apc_globals->fpstat = 1;
    apc_globals->canonicalize = 1;
    apc_globals->stat_ctime = 0;
    apc_globals->write_lock = 1;
    apc_globals->slam_defense = 1;
    apc_globals->report_autofilter = 0;
    apc_globals->include_once = 0;
    apc_globals->apc_optimize_function = NULL;
#ifdef MULTIPART_EVENT_FORMDATA
    apc_globals->rfc1867 = 0;
    memset(&(apc_globals->rfc1867_data), 0, sizeof(apc_rfc1867_data));
#endif
    memset(&apc_globals->copied_zvals, 0, sizeof(HashTable));
    apc_globals->force_file_update = 0;
    apc_globals->coredump_unmap = 0;
    apc_globals->preload_path = NULL;
    apc_globals->use_request_time = 1;
    apc_globals->lazy_class_table = NULL;
    apc_globals->lazy_function_table = NULL;
}

static void php_apc_shutdown_globals(zend_apc_globals* apc_globals TSRMLS_DC)
{
    /* deallocate the ignore patterns */
    if (apc_globals->filters != NULL) {
        int i;
        for (i=0; apc_globals->filters[i] != NULL; i++) {
            apc_efree(apc_globals->filters[i] TSRMLS_CC);
        }
        apc_efree(apc_globals->filters TSRMLS_CC);
    }

    /* the stack should be empty */
    assert(apc_stack_size(apc_globals->cache_stack) == 0);

    /* apc cleanup */
    apc_stack_destroy(apc_globals->cache_stack TSRMLS_CC);

    /* the rest of the globals are cleaned up in apc_module_shutdown() */
}

static long apc_atol(const char *str, int str_len)
{
#if PHP_MAJOR_VERSION >= 6 || PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3
	return zend_atol(str, str_len);
#else
	/* Re-implement zend_atol() for 5.2.x */
	long retval;

	if (!str_len) {
		str_len = strlen(str);
	}

	retval = strtol(str, NULL, 0);

	if (str_len > 0) {
		switch (str[str_len - 1]) {
			case 'g':
			case 'G':
				retval *= 1024;
				/* break intentionally missing */
			case 'm':
			case 'M':
				retval *= 1024;
				/* break intentionally missing */
			case 'k':
			case 'K':
				retval *= 1024;
				break;
		}
	}

	return retval;
#endif
}

/* }}} */

/* {{{ PHP_INI */

static PHP_INI_MH(OnUpdate_filters) /* {{{ */
{
    APCG(filters) = apc_tokenize(new_value, ',' TSRMLS_CC);
    return SUCCESS;
}
/* }}} */

static PHP_INI_MH(OnUpdateShmSegments) /* {{{ */
{
#if APC_MMAP
    if(zend_atoi(new_value, new_value_length)!=1) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "apc.shm_segments setting ignored in MMAP mode");
    }
    APCG(shm_segments) = 1;
#else
    APCG(shm_segments) = zend_atoi(new_value, new_value_length);
#endif
    return SUCCESS;
}
/* }}} */

static PHP_INI_MH(OnUpdateShmSize) /* {{{ */
{
	long s = apc_atol(new_value, new_value_length);

	if(s <= 0) {
		return FAILURE;
	}

	if(s < 1048576L) {
		/* if it's less than 1Mb, they are probably using the old syntax */
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "apc.shm_size now uses M/G suffixes, please update your ini files");
		s = s * 1048576L;
	}

	APCG(shm_size) = s;

	return SUCCESS;
}
/* }}} */

#ifdef MULTIPART_EVENT_FORMDATA
static PHP_INI_MH(OnUpdateRfc1867Freq) /* {{{ */
{
    int tmp;
    tmp = zend_atoi(new_value, new_value_length);
    if(tmp < 0) {
        apc_error("rfc1867_freq must be greater than or equal to zero." TSRMLS_CC);
        return FAILURE;
    }
    if(new_value[new_value_length-1] == '%') {
        if(tmp > 100) {
            apc_error("rfc1867_freq cannot be over 100%%" TSRMLS_CC);
            return FAILURE;
        }
        APCG(rfc1867_freq) = tmp / 100.0;
    } else {
        APCG(rfc1867_freq) = tmp;
    }
    return SUCCESS;
}
/* }}} */
#endif

PHP_INI_BEGIN()
STD_PHP_INI_BOOLEAN("apc.enabled",      "1",    PHP_INI_SYSTEM, OnUpdateBool,              enabled,         zend_apc_globals, apc_globals)
STD_PHP_INI_ENTRY("apc.shm_segments",   "1",    PHP_INI_SYSTEM, OnUpdateShmSegments,       shm_segments,    zend_apc_globals, apc_globals)
STD_PHP_INI_ENTRY("apc.shm_size",       "32M",  PHP_INI_SYSTEM, OnUpdateShmSize,           shm_size,        zend_apc_globals, apc_globals)
#ifdef ZEND_ENGINE_2_4
STD_PHP_INI_ENTRY("apc.shm_strings_buffer", "4M",   PHP_INI_SYSTEM, OnUpdateLong,           shm_strings_buffer,        zend_apc_globals, apc_globals)
#endif
STD_PHP_INI_BOOLEAN("apc.include_once_override", "0", PHP_INI_SYSTEM, OnUpdateBool,     include_once,    zend_apc_globals, apc_globals)
STD_PHP_INI_ENTRY("apc.num_files_hint", "1000", PHP_INI_SYSTEM, OnUpdateLong,            num_files_hint,  zend_apc_globals, apc_globals)
STD_PHP_INI_ENTRY("apc.user_entries_hint", "4096", PHP_INI_SYSTEM, OnUpdateLong,          user_entries_hint, zend_apc_globals, apc_globals)
STD_PHP_INI_ENTRY("apc.gc_ttl",         "3600", PHP_INI_SYSTEM, OnUpdateLong,            gc_ttl,           zend_apc_globals, apc_globals)
STD_PHP_INI_ENTRY("apc.ttl",            "0",    PHP_INI_SYSTEM, OnUpdateLong,            ttl,              zend_apc_globals, apc_globals)
STD_PHP_INI_ENTRY("apc.user_ttl",       "0",    PHP_INI_SYSTEM, OnUpdateLong,            user_ttl,         zend_apc_globals, apc_globals)
#if APC_MMAP
STD_PHP_INI_ENTRY("apc.mmap_file_mask",  NULL,  PHP_INI_SYSTEM, OnUpdateString,         mmap_file_mask,   zend_apc_globals, apc_globals)
#endif
PHP_INI_ENTRY("apc.filters",        NULL,     PHP_INI_SYSTEM, OnUpdate_filters)
STD_PHP_INI_BOOLEAN("apc.cache_by_default", "1",  PHP_INI_ALL, OnUpdateBool,         cache_by_default, zend_apc_globals, apc_globals)
STD_PHP_INI_ENTRY("apc.file_update_protection", "2", PHP_INI_SYSTEM, OnUpdateLong,file_update_protection,  zend_apc_globals, apc_globals)
STD_PHP_INI_BOOLEAN("apc.enable_cli", "0",      PHP_INI_SYSTEM, OnUpdateBool,           enable_cli,       zend_apc_globals, apc_globals)
STD_PHP_INI_ENTRY("apc.max_file_size", "1M",    PHP_INI_SYSTEM, OnUpdateLong,            max_file_size,    zend_apc_globals, apc_globals)
STD_PHP_INI_BOOLEAN("apc.stat", "1",            PHP_INI_SYSTEM, OnUpdateBool,           fpstat,           zend_apc_globals, apc_globals)
STD_PHP_INI_BOOLEAN("apc.canonicalize", "1",    PHP_INI_SYSTEM, OnUpdateBool,           canonicalize,     zend_apc_globals, apc_globals)
STD_PHP_INI_BOOLEAN("apc.stat_ctime", "0",      PHP_INI_SYSTEM, OnUpdateBool,           stat_ctime,       zend_apc_globals, apc_globals)
STD_PHP_INI_BOOLEAN("apc.write_lock", "1",      PHP_INI_SYSTEM, OnUpdateBool,           write_lock,       zend_apc_globals, apc_globals)
STD_PHP_INI_BOOLEAN("apc.slam_defense", "1",    PHP_INI_SYSTEM, OnUpdateBool,           slam_defense,     zend_apc_globals, apc_globals)
STD_PHP_INI_BOOLEAN("apc.report_autofilter", "0", PHP_INI_SYSTEM, OnUpdateBool,         report_autofilter,zend_apc_globals, apc_globals)
#ifdef MULTIPART_EVENT_FORMDATA
STD_PHP_INI_BOOLEAN("apc.rfc1867", "0", PHP_INI_SYSTEM, OnUpdateBool, rfc1867, zend_apc_globals, apc_globals)
STD_PHP_INI_ENTRY("apc.rfc1867_prefix", "upload_", PHP_INI_SYSTEM, OnUpdateStringUnempty, rfc1867_prefix, zend_apc_globals, apc_globals)
STD_PHP_INI_ENTRY("apc.rfc1867_name", "APC_UPLOAD_PROGRESS", PHP_INI_SYSTEM, OnUpdateStringUnempty, rfc1867_name, zend_apc_globals, apc_globals)
STD_PHP_INI_ENTRY("apc.rfc1867_freq", "0", PHP_INI_SYSTEM, OnUpdateRfc1867Freq, rfc1867_freq, zend_apc_globals, apc_globals)
STD_PHP_INI_ENTRY("apc.rfc1867_ttl", "3600", PHP_INI_SYSTEM, OnUpdateLong, rfc1867_ttl, zend_apc_globals, apc_globals)
#endif
STD_PHP_INI_BOOLEAN("apc.coredump_unmap", "0", PHP_INI_SYSTEM, OnUpdateBool, coredump_unmap, zend_apc_globals, apc_globals)
STD_PHP_INI_ENTRY("apc.preload_path", (char*)NULL,              PHP_INI_SYSTEM, OnUpdateString,       preload_path,  zend_apc_globals, apc_globals)
STD_PHP_INI_BOOLEAN("apc.file_md5", "0", PHP_INI_SYSTEM, OnUpdateBool, file_md5,  zend_apc_globals, apc_globals)
STD_PHP_INI_BOOLEAN("apc.use_request_time", "1", PHP_INI_ALL, OnUpdateBool, use_request_time,  zend_apc_globals, apc_globals)
STD_PHP_INI_BOOLEAN("apc.lazy_functions", "0", PHP_INI_SYSTEM, OnUpdateBool, lazy_functions, zend_apc_globals, apc_globals)
STD_PHP_INI_BOOLEAN("apc.lazy_classes", "0", PHP_INI_SYSTEM, OnUpdateBool, lazy_classes, zend_apc_globals, apc_globals)
PHP_INI_END()

/* }}} */

/* {{{ PHP_MINFO_FUNCTION(apc) */
static PHP_MINFO_FUNCTION(apc)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "APC Support", APCG(enabled) ? "enabled" : "disabled");
    php_info_print_table_row(2, "Version", PHP_APC_VERSION);
#ifdef __DEBUG_APC__
    php_info_print_table_row(2, "APC Debugging", "Enabled");
#else
    php_info_print_table_row(2, "APC Debugging", "Disabled");
#endif
#if APC_MMAP
    php_info_print_table_row(2, "MMAP Support", "Enabled");
    php_info_print_table_row(2, "MMAP File Mask", APCG(mmap_file_mask));
#else
    php_info_print_table_row(2, "MMAP Support", "Disabled");
#endif
    php_info_print_table_row(2, "Locking type", APC_LOCK_TYPE);
    php_info_print_table_row(2, "Revision", "$Revision: 303642 $");
    php_info_print_table_row(2, "Build Date", __DATE__ " " __TIME__);
    php_info_print_table_end();
    DISPLAY_INI_ENTRIES();
}
/* }}} */

#ifdef MULTIPART_EVENT_FORMDATA
extern int apc_rfc1867_progress(unsigned int event, void *event_data, void **extra TSRMLS_DC);
#endif

/* {{{ PHP_MINIT_FUNCTION(apc) */
static PHP_MINIT_FUNCTION(apc)
{
    ZEND_INIT_MODULE_GLOBALS(apc, php_apc_init_globals, php_apc_shutdown_globals);

    REGISTER_INI_ENTRIES();

    /* Disable APC in cli mode unless overridden by apc.enable_cli */
    if(!APCG(enable_cli) && !strcmp(sapi_module.name, "cli")) {
        APCG(enabled) = 0;
    }

    if (APCG(enabled)) {
        if(APCG(initialized)) {
            apc_process_init(module_number TSRMLS_CC);
        } else {
            apc_module_init(module_number TSRMLS_CC);
            apc_zend_init(TSRMLS_C);
            apc_process_init(module_number TSRMLS_CC);
#ifdef MULTIPART_EVENT_FORMDATA
            /* File upload progress tracking */
            if(APCG(rfc1867)) {
                php_rfc1867_callback = apc_rfc1867_progress;
            }
#endif
            apc_iterator_init(module_number TSRMLS_CC);
        }

        zend_register_long_constant("APC_BIN_VERIFY_MD5", sizeof("APC_BIN_VERIFY_MD5"), APC_BIN_VERIFY_MD5, (CONST_CS | CONST_PERSISTENT), module_number TSRMLS_CC);
        zend_register_long_constant("APC_BIN_VERIFY_CRC32", sizeof("APC_BIN_VERIFY_CRC32"), APC_BIN_VERIFY_CRC32, (CONST_CS | CONST_PERSISTENT), module_number TSRMLS_CC);
    }

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION(apc) */
static PHP_MSHUTDOWN_FUNCTION(apc)
{
    if(APCG(enabled)) {
        apc_process_shutdown(TSRMLS_C);
        apc_zend_shutdown(TSRMLS_C);
        apc_module_shutdown(TSRMLS_C);
#ifndef ZTS
        php_apc_shutdown_globals(&apc_globals);
#endif
#if HAVE_SIGACTION
        apc_shutdown_signals(TSRMLS_C);
#endif
    }
#ifdef ZTS
    ts_free_id(apc_globals_id);
#endif
    UNREGISTER_INI_ENTRIES();
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION(apc) */
static PHP_RINIT_FUNCTION(apc)
{
    if(APCG(enabled)) {
        apc_request_init(TSRMLS_C);

#if HAVE_SIGACTION
        apc_set_signals(TSRMLS_C);
#endif
    }
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION(apc) */
static PHP_RSHUTDOWN_FUNCTION(apc)
{
    if(APCG(enabled)) {
        apc_request_shutdown(TSRMLS_C);
    }
    return SUCCESS;
}
/* }}} */

/* {{{ proto array apc_cache_info([string type [, bool limited]]) */
PHP_FUNCTION(apc_cache_info)
{
    apc_cache_info_t* info;
    apc_cache_link_t* p;
    zval* list;
    char *cache_type;
    int ct_len;
    zend_bool limited=0;
    char md5str[33];

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sb", &cache_type, &ct_len, &limited) == FAILURE) {
        return;
    }

    if(ZEND_NUM_ARGS()) {
        if(!strcasecmp(cache_type,"user")) {
            info = apc_cache_info(apc_user_cache, limited TSRMLS_CC);
        } else if(!strcasecmp(cache_type,"filehits")) {
#ifdef APC_FILEHITS
            RETVAL_ZVAL(APCG(filehits), 1, 0);
            return;
#else
            RETURN_FALSE;
#endif
        } else {
            info = apc_cache_info(apc_cache, limited TSRMLS_CC);
        }
    } else {
        info = apc_cache_info(apc_cache, limited TSRMLS_CC);
    }

    if(!info) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "No APC info available.  Perhaps APC is not enabled? Check apc.enabled in your ini file");
        RETURN_FALSE;
    }

    array_init(return_value);
    add_assoc_long(return_value, "num_slots", info->num_slots);
    add_assoc_long(return_value, "ttl", info->ttl);

    add_assoc_double(return_value, "num_hits", (double)info->num_hits);
    add_assoc_double(return_value, "num_misses", (double)info->num_misses);
    add_assoc_double(return_value, "num_inserts", (double)info->num_inserts);
    add_assoc_double(return_value, "expunges", (double)info->expunges);
    
    add_assoc_long(return_value, "start_time", info->start_time);
    add_assoc_double(return_value, "mem_size", (double)info->mem_size);
    add_assoc_long(return_value, "num_entries", info->num_entries);
#ifdef MULTIPART_EVENT_FORMDATA
    add_assoc_long(return_value, "file_upload_progress", 1);
#else
    add_assoc_long(return_value, "file_upload_progress", 0);
#endif
#if APC_MMAP
    add_assoc_stringl(return_value, "memory_type", "mmap", sizeof("mmap")-1, 1);
#else
    add_assoc_stringl(return_value, "memory_type", "IPC shared", sizeof("IPC shared")-1, 1);
#endif
#if APC_SEM_LOCKS
    add_assoc_stringl(return_value, "locking_type", "IPC semaphore", sizeof("IPC semaphore")-1, 1);
#elif APC_PTHREADMUTEX_LOCKS
    add_assoc_stringl(return_value, "locking_type", "pthread mutex", sizeof("pthread mutex")-1, 1);
#elif APC_SPIN_LOCKS
    add_assoc_stringl(return_value, "locking_type", "spin", sizeof("spin")-1, 1);
#else
    add_assoc_stringl(return_value, "locking_type", "file", sizeof("file")-1, 1);
#endif
    if(limited) {
        apc_cache_free_info(info TSRMLS_CC);
        return;
    }

    ALLOC_INIT_ZVAL(list);
    array_init(list);

    for (p = info->list; p != NULL; p = p->next) {
        zval* link;

        ALLOC_INIT_ZVAL(link);
        array_init(link);

        if(p->type == APC_CACHE_ENTRY_FILE) {
            add_assoc_string(link, "filename", p->data.file.filename, 1);
#ifdef PHP_WIN32
			{
				char buf[20];
				sprintf(buf, "%I64d",  p->data.file.device);
				add_assoc_string(link, "device", buf, 1);

				sprintf(buf, "%I64d",  p->data.file.inode);
				add_assoc_string(link, "inode", buf, 1);
			}
#else
            add_assoc_long(link, "device", p->data.file.device);
            add_assoc_long(link, "inode", p->data.file.inode);
#endif


            add_assoc_string(link, "type", "file", 1);
            if(APCG(file_md5)) {
                make_digest(md5str, p->data.file.md5);
                add_assoc_string(link, "md5", md5str, 1);
            }
        } else if(p->type == APC_CACHE_ENTRY_USER) {
            add_assoc_string(link, "info", p->data.user.info, 1);
            add_assoc_long(link, "ttl", (long)p->data.user.ttl);
            add_assoc_string(link, "type", "user", 1);
        }

        add_assoc_double(link, "num_hits", (double)p->num_hits);
        
        add_assoc_long(link, "mtime", p->mtime);
        add_assoc_long(link, "creation_time", p->creation_time);
        add_assoc_long(link, "deletion_time", p->deletion_time);
        add_assoc_long(link, "access_time", p->access_time);
        add_assoc_long(link, "ref_count", p->ref_count);
        add_assoc_long(link, "mem_size", p->mem_size);
        

        add_next_index_zval(list, link);
    }
    add_assoc_zval(return_value, "cache_list", list);

    ALLOC_INIT_ZVAL(list);
    array_init(list);

    for (p = info->deleted_list; p != NULL; p = p->next) {
        zval* link;

        ALLOC_INIT_ZVAL(link);
        array_init(link);

        if(p->type == APC_CACHE_ENTRY_FILE) {
            add_assoc_string(link, "filename", p->data.file.filename, 1);
#ifdef PHP_WIN32
			{
				char buf[20];
				sprintf(buf, "%I64d",  p->data.file.device);
				add_assoc_string(link, "device", buf, 1);

				sprintf(buf, "%I64d",  p->data.file.inode);
				add_assoc_string(link, "inode", buf, 1);
			}
#else
            add_assoc_long(link, "device", p->data.file.device);
            add_assoc_long(link, "inode", p->data.file.inode);
#endif

            add_assoc_string(link, "type", "file", 1);
            if(APCG(file_md5)) {
                make_digest(md5str, p->data.file.md5);
                add_assoc_string(link, "md5", md5str, 1);
            }
        } else if(p->type == APC_CACHE_ENTRY_USER) {
            add_assoc_string(link, "info", p->data.user.info, 1);
            add_assoc_long(link, "ttl", (long)p->data.user.ttl);
            add_assoc_string(link, "type", "user", 1);
        }
        
        add_assoc_double(link, "num_hits", (double)p->num_hits);
        
        add_assoc_long(link, "mtime", p->mtime);
        add_assoc_long(link, "creation_time", p->creation_time);
        add_assoc_long(link, "deletion_time", p->deletion_time);
        add_assoc_long(link, "access_time", p->access_time);
        add_assoc_long(link, "ref_count", p->ref_count);
        add_assoc_long(link, "mem_size", p->mem_size);
        add_next_index_zval(list, link);
    }
    add_assoc_zval(return_value, "deleted_list", list);

    apc_cache_free_info(info TSRMLS_CC);
}
/* }}} */

/* {{{ proto void apc_clear_cache([string cache]) */
PHP_FUNCTION(apc_clear_cache)
{
    char *cache_type;
    int ct_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &cache_type, &ct_len) == FAILURE) {
        return;
    }

    if(ct_len) {
        if(!strcasecmp(cache_type, "user")) {
            apc_cache_clear(apc_user_cache TSRMLS_CC);
            RETURN_TRUE;
        }
    }
    apc_cache_clear(apc_cache TSRMLS_CC);
    RETURN_TRUE;
}
/* }}} */

/* {{{ proto array apc_sma_info([bool limited]) */
PHP_FUNCTION(apc_sma_info)
{
    apc_sma_info_t* info;
    zval* block_lists;
    int i;
    zend_bool limited = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|b", &limited) == FAILURE) {
        return;
    }

    info = apc_sma_info(limited TSRMLS_CC);

    if(!info) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "No APC SMA info available.  Perhaps APC is disabled via apc.enabled?");
        RETURN_FALSE;
    }

    array_init(return_value);
    add_assoc_long(return_value, "num_seg", info->num_seg);
    add_assoc_double(return_value, "seg_size", (double)info->seg_size);
    add_assoc_double(return_value, "avail_mem", (double)apc_sma_get_avail_mem());

    if(limited) {
        apc_sma_free_info(info TSRMLS_CC);
        return;
    }

#if ALLOC_DISTRIBUTION
    {
        size_t *adist = apc_sma_get_alloc_distribution();
        zval* list;
        ALLOC_INIT_ZVAL(list);
        array_init(list);
        for(i=0; i<30; i++) {
            add_next_index_long(list, adist[i]);
        }
        add_assoc_zval(return_value, "adist", list);
    }
#endif
    ALLOC_INIT_ZVAL(block_lists);
    array_init(block_lists);

    for (i = 0; i < info->num_seg; i++) {
        apc_sma_link_t* p;
        zval* list;

        ALLOC_INIT_ZVAL(list);
        array_init(list);

        for (p = info->list[i]; p != NULL; p = p->next) {
            zval* link;

            ALLOC_INIT_ZVAL(link);
            array_init(link);

            add_assoc_long(link, "size", p->size);
            add_assoc_long(link, "offset", p->offset);
            add_next_index_zval(list, link);
        }
        add_next_index_zval(block_lists, list);
    }
    add_assoc_zval(return_value, "block_lists", block_lists);
    apc_sma_free_info(info TSRMLS_CC);
}
/* }}} */

/* {{{ */
int _apc_update(char *strkey, int strkey_len, apc_cache_updater_t updater, void* data TSRMLS_DC) 
{
    if(!APCG(enabled)) {
        return 0;
    }

    HANDLE_BLOCK_INTERRUPTIONS();
    APCG(current_cache) = apc_user_cache;
    
    if (!_apc_cache_user_update(apc_user_cache, strkey, strkey_len + 1, updater, data TSRMLS_CC)) {
        HANDLE_UNBLOCK_INTERRUPTIONS();
        return 0;
    }

    APCG(current_cache) = NULL;
    HANDLE_UNBLOCK_INTERRUPTIONS();

    return 1;
}
/* }}} */
    
/* {{{ _apc_store */
int _apc_store(char *strkey, int strkey_len, const zval *val, const unsigned int ttl, const int exclusive TSRMLS_DC) {
    apc_cache_entry_t *entry;
    apc_cache_key_t key;
    time_t t;
    apc_context_t ctxt={0,};
    int ret = 1;

    t = apc_time();

    if(!APCG(enabled)) return 0;

    HANDLE_BLOCK_INTERRUPTIONS();

    APCG(current_cache) = apc_user_cache;

    ctxt.pool = apc_pool_create(APC_SMALL_POOL, apc_sma_malloc, apc_sma_free, apc_sma_protect, apc_sma_unprotect TSRMLS_CC);
    if (!ctxt.pool) {
        apc_warning("Unable to allocate memory for pool." TSRMLS_CC);
        return 0;
    }
    ctxt.copy = APC_COPY_IN_USER;
    ctxt.force_update = 0;

    if(!ctxt.pool) {
        ret = 0;
        goto nocache;
    }

    if (!apc_cache_make_user_key(&key, strkey, strkey_len, t)) {
        goto freepool;
    }

    if (apc_cache_is_last_key(apc_user_cache, &key, 0, t TSRMLS_CC)) {
	    goto freepool;
    }

    if (!(entry = apc_cache_make_user_entry(strkey, strkey_len, val, &ctxt, ttl TSRMLS_CC))) {
        goto freepool;
    }

    if (!apc_cache_user_insert(apc_user_cache, key, entry, &ctxt, t, exclusive TSRMLS_CC)) {
freepool:
        apc_pool_destroy(ctxt.pool TSRMLS_CC);
        ret = 0;
    }

nocache:

    APCG(current_cache) = NULL;

    HANDLE_UNBLOCK_INTERRUPTIONS();

    return ret;
}
/* }}} */

/* {{{ apc_store_helper(INTERNAL_FUNCTION_PARAMETERS, const int exclusive)
 */
static void apc_store_helper(INTERNAL_FUNCTION_PARAMETERS, const int exclusive)
{
    zval *key = NULL;
    zval *val = NULL;
    long ttl = 0L;
    HashTable *hash;
    HashPosition hpos;
    zval **hentry;
    char *hkey=NULL;
    uint hkey_len;
    ulong hkey_idx;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|zl", &key, &val, &ttl) == FAILURE) {
        return;
    }

    if (!key) RETURN_FALSE;

    if (Z_TYPE_P(key) == IS_ARRAY) {
        hash = Z_ARRVAL_P(key);
        array_init(return_value);
        zend_hash_internal_pointer_reset_ex(hash, &hpos);
        while(zend_hash_get_current_data_ex(hash, (void**)&hentry, &hpos) == SUCCESS) {
            zend_hash_get_current_key_ex(hash, &hkey, &hkey_len, &hkey_idx, 0, &hpos);
            if (hkey) {
                if(!_apc_store(hkey, hkey_len, *hentry, (unsigned int)ttl, exclusive TSRMLS_CC)) {
                    add_assoc_long_ex(return_value, hkey, hkey_len, -1);  /* -1: insertion error */
                }
                hkey = NULL;
            } else {
                add_index_long(return_value, hkey_idx, -1);  /* -1: insertion error */
            }
            zend_hash_move_forward_ex(hash, &hpos);
        }
        return;
    } else if (Z_TYPE_P(key) == IS_STRING) {
        if (!val) RETURN_FALSE;
        if(_apc_store(Z_STRVAL_P(key), Z_STRLEN_P(key) + 1, val, (unsigned int)ttl, exclusive TSRMLS_CC))
            RETURN_TRUE;
    } else {
        apc_warning("apc_store expects key parameter to be a string or an array of key/value pairs." TSRMLS_CC);
    }

    RETURN_FALSE;
}
/* }}} */

/* {{{ proto int apc_store(mixed key, mixed var [, long ttl ])
 */
PHP_FUNCTION(apc_store) {
    apc_store_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto int apc_add(mixed key, mixed var [, long ttl ])
 */
PHP_FUNCTION(apc_add) {
    apc_store_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ inc_updater */

struct _inc_update_args {
    long step;
    long lval;
};

static int inc_updater(apc_cache_t* cache, apc_cache_entry_t* entry, void* data) {

    struct _inc_update_args *args = (struct _inc_update_args*) data;
    
    zval* val = entry->data.user.val;

    if(Z_TYPE_P(val) == IS_LONG) {
        Z_LVAL_P(val) += args->step;
        args->lval = Z_LVAL_P(val);
        return 1;
    }

    return 0;
}
/* }}} */

/* {{{ proto long apc_inc(string key [, long step [, bool& success]])
 */
PHP_FUNCTION(apc_inc) {
    char *strkey;
    int strkey_len;
    struct _inc_update_args args = {1L, -1};
    zval *success = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|lz", &strkey, &strkey_len, &(args.step), &success) == FAILURE) {
        return;
    }

    if(_apc_update(strkey, strkey_len, inc_updater, &args TSRMLS_CC)) {
        if(success) ZVAL_TRUE(success);
        RETURN_LONG(args.lval);
    }
    
    if(success) ZVAL_FALSE(success);
    
    RETURN_FALSE;
}
/* }}} */

/* {{{ proto long apc_dec(string key [, long step [, bool &success]])
 */
PHP_FUNCTION(apc_dec) {
    char *strkey;
    int strkey_len;
    struct _inc_update_args args = {1L, -1};
    zval *success = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|lz", &strkey, &strkey_len, &(args.step), &success) == FAILURE) {
        return;
    }

    args.step = args.step * -1;

    if(_apc_update(strkey, strkey_len, inc_updater, &args TSRMLS_CC)) {
        if(success) ZVAL_TRUE(success);
        RETURN_LONG(args.lval);
    }
    
    if(success) ZVAL_FALSE(success);
    
    RETURN_FALSE;
}
/* }}} */

/* {{{ cas_updater */
static int cas_updater(apc_cache_t* cache, apc_cache_entry_t* entry, void* data) {
    long* vals = ((long*)data);
    long old = vals[0];
    long new = vals[1];
    zval* val = entry->data.user.val;

    if(Z_TYPE_P(val) == IS_LONG) {
        if(Z_LVAL_P(val) == old) {
            Z_LVAL_P(val) = new;
            return 1;
        }
    }

    return 0;
}
/* }}} */

/* {{{ proto int apc_cas(string key, int old, int new)
 */
PHP_FUNCTION(apc_cas) {
    char *strkey;
    int strkey_len;
    long vals[2];

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sll", &strkey, &strkey_len, &vals[0], &vals[1]) == FAILURE) {
        return;
    }

    if(_apc_update(strkey, strkey_len, cas_updater, &vals TSRMLS_CC)) RETURN_TRUE;
    RETURN_FALSE;
}
/* }}} */

void *apc_erealloc_wrapper(void *ptr, size_t size) {
    return _erealloc(ptr, size, 0 ZEND_FILE_LINE_CC ZEND_FILE_LINE_EMPTY_CC);
}

/* {{{ proto mixed apc_fetch(mixed key[, bool &success])
 */
PHP_FUNCTION(apc_fetch) {
    zval *key;
    zval *success = NULL;
    HashTable *hash;
    HashPosition hpos;
    zval **hentry;
    zval *result;
    zval *result_entry;
    char *strkey;
    int strkey_len;
    apc_cache_entry_t* entry;
    time_t t;
    apc_context_t ctxt = {0,};

    if(!APCG(enabled)) RETURN_FALSE;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z", &key, &success) == FAILURE) {
        return;
    }

    t = apc_time();

    if (success) {
        ZVAL_BOOL(success, 0);
    }

    ctxt.pool = apc_pool_create(APC_UNPOOL, apc_php_malloc, apc_php_free, NULL, NULL TSRMLS_CC);
    if (!ctxt.pool) {
        apc_warning("Unable to allocate memory for pool." TSRMLS_CC);
        RETURN_FALSE;
    }
    ctxt.copy = APC_COPY_OUT_USER;
    ctxt.force_update = 0;

    if(Z_TYPE_P(key) != IS_STRING && Z_TYPE_P(key) != IS_ARRAY) {
        convert_to_string(key);
    }

    if(Z_TYPE_P(key) == IS_STRING) {
        strkey = Z_STRVAL_P(key);
        strkey_len = Z_STRLEN_P(key);
        if(!strkey_len) RETURN_FALSE;
        entry = apc_cache_user_find(apc_user_cache, strkey, (strkey_len + 1), t TSRMLS_CC);
        if(entry) {
            /* deep-copy returned shm zval to emalloc'ed return_value */
            apc_cache_fetch_zval(return_value, entry->data.user.val, &ctxt TSRMLS_CC);
            apc_cache_release(apc_user_cache, entry TSRMLS_CC);
        } else {
            goto freepool;
        }
    } else if(Z_TYPE_P(key) == IS_ARRAY) {
        hash = Z_ARRVAL_P(key);
        MAKE_STD_ZVAL(result);
        array_init(result); 
        zend_hash_internal_pointer_reset_ex(hash, &hpos);
        while(zend_hash_get_current_data_ex(hash, (void**)&hentry, &hpos) == SUCCESS) {
            if(Z_TYPE_PP(hentry) != IS_STRING) {
                apc_warning("apc_fetch() expects a string or array of strings." TSRMLS_CC);
                goto freepool;
            }
            entry = apc_cache_user_find(apc_user_cache, Z_STRVAL_PP(hentry), (Z_STRLEN_PP(hentry) + 1), t TSRMLS_CC);
            if(entry) {
                /* deep-copy returned shm zval to emalloc'ed return_value */
                MAKE_STD_ZVAL(result_entry);
                apc_cache_fetch_zval(result_entry, entry->data.user.val, &ctxt TSRMLS_CC);
                apc_cache_release(apc_user_cache, entry TSRMLS_CC);
                zend_hash_add(Z_ARRVAL_P(result), Z_STRVAL_PP(hentry), Z_STRLEN_PP(hentry) +1, &result_entry, sizeof(zval*), NULL);
            } /* don't set values we didn't find */
            zend_hash_move_forward_ex(hash, &hpos);
        }
        RETVAL_ZVAL(result, 0, 1);
    } else {
        apc_warning("apc_fetch() expects a string or array of strings." TSRMLS_CC);
freepool:
        apc_pool_destroy(ctxt.pool TSRMLS_CC);
        RETURN_FALSE;
    }

    if (success) {
        ZVAL_BOOL(success, 1);
    }

    apc_pool_destroy(ctxt.pool TSRMLS_CC);
    return;
}
/* }}} */

/* {{{ proto mixed apc_exists(mixed key)
 */
PHP_FUNCTION(apc_exists) {
    zval *key;
    HashTable *hash;
    HashPosition hpos;
    zval **hentry;
    char *strkey;
    int strkey_len;
    apc_cache_entry_t* entry;
    zval *result;
    zval *result_entry;
    time_t t;

    if(!APCG(enabled)) RETURN_FALSE;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &key) == FAILURE) {
        return;
    }

    t = apc_time();

    if(Z_TYPE_P(key) != IS_STRING && Z_TYPE_P(key) != IS_ARRAY) {
        convert_to_string(key);
    }

    if(Z_TYPE_P(key) == IS_STRING) {
        strkey = Z_STRVAL_P(key);
        strkey_len = Z_STRLEN_P(key);
        if(!strkey_len) RETURN_FALSE;
        entry = apc_cache_user_exists(apc_user_cache, strkey, strkey_len + 1, t TSRMLS_CC);
        if(entry) {
            RETURN_TRUE;
        }
    } else if(Z_TYPE_P(key) == IS_ARRAY) {
        hash = Z_ARRVAL_P(key);
        MAKE_STD_ZVAL(result);
        array_init(result); 
        zend_hash_internal_pointer_reset_ex(hash, &hpos);
        while(zend_hash_get_current_data_ex(hash, (void**)&hentry, &hpos) == SUCCESS) {
            if(Z_TYPE_PP(hentry) != IS_STRING) {
                apc_warning("apc_exists() expects a string or array of strings." TSRMLS_CC);
                RETURN_FALSE;
            }

            entry = apc_cache_user_exists(apc_user_cache, Z_STRVAL_PP(hentry), Z_STRLEN_PP(hentry) + 1, t TSRMLS_CC);
            if(entry) {
                MAKE_STD_ZVAL(result_entry);
                ZVAL_BOOL(result_entry, 1);
                zend_hash_add(Z_ARRVAL_P(result), Z_STRVAL_PP(hentry), Z_STRLEN_PP(hentry) +1, &result_entry, sizeof(zval*), NULL);
            } /* don't set values we didn't find */
            zend_hash_move_forward_ex(hash, &hpos);
        }
        RETURN_ZVAL(result, 0, 1);
    } else {
        apc_warning("apc_exists() expects a string or array of strings." TSRMLS_CC);
    }

    RETURN_FALSE;
}
/* }}} */


/* {{{ proto mixed apc_delete(mixed keys)
 */
PHP_FUNCTION(apc_delete) {
    zval *keys;

    if(!APCG(enabled)) RETURN_FALSE;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &keys) == FAILURE) {
        return;
    }

    if (Z_TYPE_P(keys) == IS_STRING) {
        if (!Z_STRLEN_P(keys)) RETURN_FALSE;
        if(apc_cache_user_delete(apc_user_cache, Z_STRVAL_P(keys), (Z_STRLEN_P(keys) + 1) TSRMLS_CC)) {
            RETURN_TRUE;
        } else {
            RETURN_FALSE;
        }
    } else if (Z_TYPE_P(keys) == IS_ARRAY) {
        HashTable *hash = Z_ARRVAL_P(keys);
        HashPosition hpos;
        zval **hentry;
        array_init(return_value);
        zend_hash_internal_pointer_reset_ex(hash, &hpos);
        while(zend_hash_get_current_data_ex(hash, (void**)&hentry, &hpos) == SUCCESS) {
            if(Z_TYPE_PP(hentry) != IS_STRING) {
                apc_warning("apc_delete() expects a string, array of strings, or APCIterator instance." TSRMLS_CC);
                add_next_index_zval(return_value, *hentry);
                Z_ADDREF_PP(hentry);
            } else if(apc_cache_user_delete(apc_user_cache, Z_STRVAL_PP(hentry), (Z_STRLEN_PP(hentry) + 1) TSRMLS_CC) != 1) {
                add_next_index_zval(return_value, *hentry);
                Z_ADDREF_PP(hentry);
            }
            zend_hash_move_forward_ex(hash, &hpos);
        }
        return;
    } else if (Z_TYPE_P(keys) == IS_OBJECT) {
        if (apc_iterator_delete(keys TSRMLS_CC)) {
            RETURN_TRUE;
        } else {
            RETURN_FALSE;
        }
    } else {
        apc_warning("apc_delete() expects a string, array of strings, or APCIterator instance." TSRMLS_CC);
    }
}
/* }}} */

/* {{{ proto mixed apc_delete_file(mixed keys)
 *       Deletes the given files from the opcode cache.  
 *       Accepts a string, array of strings, or APCIterator object. 
 *       Returns True/False, or for an Array an Array of failed files.
 */
PHP_FUNCTION(apc_delete_file) {
    zval *keys;

    if(!APCG(enabled)) RETURN_FALSE;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &keys) == FAILURE) {
        return;
    }

    if (Z_TYPE_P(keys) == IS_STRING) {
        if (!Z_STRLEN_P(keys)) RETURN_FALSE;
        if(apc_cache_delete(apc_cache, Z_STRVAL_P(keys), Z_STRLEN_P(keys) + 1 TSRMLS_CC) != 1) {
            RETURN_FALSE;
        } else {
            RETURN_TRUE;
        }
    } else if (Z_TYPE_P(keys) == IS_ARRAY) {
        HashTable *hash = Z_ARRVAL_P(keys);
        HashPosition hpos;
        zval **hentry;
        array_init(return_value);
        zend_hash_internal_pointer_reset_ex(hash, &hpos);
        while(zend_hash_get_current_data_ex(hash, (void**)&hentry, &hpos) == SUCCESS) {
            if(Z_TYPE_PP(hentry) != IS_STRING) {
                apc_warning("apc_delete_file() expects a string, array of strings, or APCIterator instance." TSRMLS_CC);
                add_next_index_zval(return_value, *hentry);
                Z_ADDREF_PP(hentry);
            } else if(apc_cache_delete(apc_cache, Z_STRVAL_PP(hentry), Z_STRLEN_PP(hentry) + 1 TSRMLS_CC) != 1) {
                add_next_index_zval(return_value, *hentry);
                Z_ADDREF_PP(hentry);
            }
            zend_hash_move_forward_ex(hash, &hpos);
        }
        return;
    } else if (Z_TYPE_P(keys) == IS_OBJECT) {
        if (apc_iterator_delete(keys TSRMLS_CC)) {
            RETURN_TRUE;
        } else {
            RETURN_FALSE;
        }
    } else {
        apc_warning("apc_delete_file() expects a string, array of strings, or APCIterator instance." TSRMLS_CC);
    }
}
/* }}} */

static void _apc_define_constants(zval *constants, zend_bool case_sensitive TSRMLS_DC) {
    char *const_key;
    unsigned int const_key_len;
    zval **entry;
    HashPosition pos;

    zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(constants), &pos);
    while (zend_hash_get_current_data_ex(Z_ARRVAL_P(constants), (void**)&entry, &pos) == SUCCESS) {
        zend_constant c;
        int key_type;
        ulong num_key;

        key_type = zend_hash_get_current_key_ex(Z_ARRVAL_P(constants), &const_key, &const_key_len, &num_key, 0, &pos);
        if(key_type != HASH_KEY_IS_STRING) {
            zend_hash_move_forward_ex(Z_ARRVAL_P(constants), &pos);
            continue;
        }
        switch(Z_TYPE_PP(entry)) {
            case IS_LONG:
            case IS_DOUBLE:
            case IS_STRING:
            case IS_BOOL:
            case IS_RESOURCE:
            case IS_NULL:
                break;
            default:
                zend_hash_move_forward_ex(Z_ARRVAL_P(constants), &pos);
                continue;
        }
        c.value = **entry;
        zval_copy_ctor(&c.value);
        c.flags = case_sensitive;
        c.name = zend_strndup(const_key, const_key_len);
        c.name_len = const_key_len;
        c.module_number = PHP_USER_CONSTANT;
        zend_register_constant(&c TSRMLS_CC);

        zend_hash_move_forward_ex(Z_ARRVAL_P(constants), &pos);
    }
}

/* {{{ proto mixed apc_define_constants(string key, array constants [, bool case_sensitive])
 */
PHP_FUNCTION(apc_define_constants) {
    char *strkey;
    int strkey_len;
    zval *constants = NULL;
    zend_bool case_sensitive = 1;
    int argc = ZEND_NUM_ARGS();

    if (zend_parse_parameters(argc TSRMLS_CC, "sa|b", &strkey, &strkey_len, &constants, &case_sensitive) == FAILURE) {
        return;
    }

    if(!strkey_len) RETURN_FALSE;

    _apc_define_constants(constants, case_sensitive TSRMLS_CC);
    if(_apc_store(strkey, strkey_len + 1, constants, 0, 0 TSRMLS_CC)) RETURN_TRUE;
    RETURN_FALSE;
} /* }}} */

/* {{{ proto mixed apc_load_constants(string key [, bool case_sensitive])
 */
PHP_FUNCTION(apc_load_constants) {
    char *strkey;
    int strkey_len;
    apc_cache_entry_t* entry;
    time_t t;
    zend_bool case_sensitive = 1;

    if(!APCG(enabled)) RETURN_FALSE;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &strkey, &strkey_len, &case_sensitive) == FAILURE) {
        return;
    }

    if(!strkey_len) RETURN_FALSE;

    t = apc_time();

    entry = apc_cache_user_find(apc_user_cache, strkey, (strkey_len + 1), t TSRMLS_CC);

    if(entry) {
        _apc_define_constants(entry->data.user.val, case_sensitive TSRMLS_CC);
        apc_cache_release(apc_user_cache, entry TSRMLS_CC);
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}
/* }}} */

/* {{{ proto mixed apc_compile_file(mixed filenames [, bool atomic])
 */
PHP_FUNCTION(apc_compile_file) {
    zval *file;
    zend_file_handle file_handle;
    zend_op_array *op_array;
    char** filters = NULL;
    zend_bool cache_by_default = 1;
    HashTable cg_function_table, cg_class_table;
    HashTable *cg_orig_function_table, *cg_orig_class_table, *eg_orig_function_table, *eg_orig_class_table;
    apc_cache_entry_t** cache_entries;
    apc_cache_key_t* keys;
    zend_op_array **op_arrays;
    time_t t;
    zval **hentry;
    HashPosition hpos;
    int i=0, c=0;
    int *rval=NULL;
    int count=0;
    zend_bool atomic=1;
    apc_context_t ctxt = {0,};
    zend_execute_data *orig_current_execute_data;
    int atomic_fail;

    if(!APCG(enabled)) RETURN_FALSE;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|b", &file, &atomic) == FAILURE) {
        return;
    }

    if (Z_TYPE_P(file) != IS_ARRAY && Z_TYPE_P(file) != IS_STRING) {
        apc_warning("apc_compile_file argument must be a string or an array of strings" TSRMLS_CC);
        RETURN_FALSE;
    }

    HANDLE_BLOCK_INTERRUPTIONS();
    APCG(current_cache) = apc_cache;

    /* reset filters and cache_by_default */
    filters = APCG(filters);
    APCG(filters) = NULL;

    cache_by_default = APCG(cache_by_default);
    APCG(cache_by_default) = 1;

    /* Replace function/class tables to avoid namespace conflicts */
    zend_hash_init_ex(&cg_function_table, 100, NULL, ZEND_FUNCTION_DTOR, 1, 0);
    cg_orig_function_table = CG(function_table);
    CG(function_table) = &cg_function_table;
    zend_hash_init_ex(&cg_class_table, 10, NULL, ZEND_CLASS_DTOR, 1, 0);
    cg_orig_class_table = CG(class_table);
    CG(class_table) = &cg_class_table;
    eg_orig_function_table = EG(function_table);
    EG(function_table) = CG(function_table);
    eg_orig_class_table = EG(class_table);
    EG(class_table) = CG(class_table);
    APCG(force_file_update) = 1;

    /* Compile the file(s), loading it into the cache */
    if (Z_TYPE_P(file) == IS_STRING) {
        file_handle.type = ZEND_HANDLE_FILENAME;
        file_handle.filename = Z_STRVAL_P(file);
        file_handle.free_filename = 0;
        file_handle.opened_path = NULL;

        orig_current_execute_data = EG(current_execute_data);
        zend_try {
            op_array = zend_compile_file(&file_handle, ZEND_INCLUDE TSRMLS_CC);
        } zend_catch {
            EG(current_execute_data) = orig_current_execute_data;
            EG(in_execution) = 1;
            CG(unclean_shutdown) = 0;
            apc_warning("Error compiling %s in apc_compile_file." TSRMLS_CC, file_handle.filename);
            op_array = NULL;
        } zend_end_try();
        if(op_array != NULL) {
            /* Free up everything */
            destroy_op_array(op_array TSRMLS_CC);
            efree(op_array);
            RETVAL_TRUE;
        } else {
            RETVAL_FALSE;
        }
        zend_destroy_file_handle(&file_handle TSRMLS_CC);

    } else { /* IS_ARRAY */

        array_init(return_value);

        t = apc_time();

        op_arrays = ecalloc(Z_ARRVAL_P(file)->nNumOfElements, sizeof(zend_op_array*));
        cache_entries = ecalloc(Z_ARRVAL_P(file)->nNumOfElements, sizeof(apc_cache_entry_t*));
        keys = ecalloc(Z_ARRVAL_P(file)->nNumOfElements, sizeof(apc_cache_key_t));
        zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(file), &hpos);
        while(zend_hash_get_current_data_ex(Z_ARRVAL_P(file), (void**)&hentry, &hpos) == SUCCESS) {
            if (Z_TYPE_PP(hentry) != IS_STRING) {
                apc_warning("apc_compile_file array values must be strings, aborting." TSRMLS_CC);
                break;
            }
            file_handle.type = ZEND_HANDLE_FILENAME;
            file_handle.filename = Z_STRVAL_PP(hentry);
            file_handle.free_filename = 0;
            file_handle.opened_path = NULL;

            if (!apc_cache_make_file_key(&(keys[i]), file_handle.filename, PG(include_path), t TSRMLS_CC)) {
                add_assoc_long(return_value, Z_STRVAL_PP(hentry), -1);  /* -1: compilation error */
                apc_warning("Error compiling %s in apc_compile_file." TSRMLS_CC, file_handle.filename);
                break;
            }

            if (keys[i].type == APC_CACHE_KEY_FPFILE) {
                keys[i].data.fpfile.fullpath = estrndup(keys[i].data.fpfile.fullpath, keys[i].data.fpfile.fullpath_len);
            } else if (keys[i].type == APC_CACHE_KEY_USER) {
                keys[i].data.user.identifier = estrndup(keys[i].data.user.identifier, keys[i].data.user.identifier_len);
            }

            orig_current_execute_data = EG(current_execute_data);
            zend_try {
                if (apc_compile_cache_entry(keys[i], &file_handle, ZEND_INCLUDE, t, &op_arrays[i], &cache_entries[i] TSRMLS_CC) != SUCCESS) {
                    op_arrays[i] = NULL;
                    cache_entries[i] = NULL;
                    add_assoc_long(return_value, Z_STRVAL_PP(hentry), -2);  /* -2: input or cache insertion error */
                    apc_warning("Error compiling %s in apc_compile_file." TSRMLS_CC, file_handle.filename);
                }
            } zend_catch {
                EG(current_execute_data) = orig_current_execute_data;
                EG(in_execution) = 1;
                CG(unclean_shutdown) = 0;
                op_arrays[i] = NULL;
                cache_entries[i] = NULL;
                add_assoc_long(return_value, Z_STRVAL_PP(hentry), -1);  /* -1: compilation error */
                apc_warning("Error compiling %s in apc_compile_file." TSRMLS_CC, file_handle.filename);
            } zend_end_try();

            zend_destroy_file_handle(&file_handle TSRMLS_CC);
            if(op_arrays[i] != NULL) {
                count++;
            }

            /* clean out the function/class tables */
            zend_hash_clean(&cg_function_table);
            zend_hash_clean(&cg_class_table);

            zend_hash_move_forward_ex(Z_ARRVAL_P(file), &hpos);
            i++;
        }

        /* atomically update the cache if no errors or not atomic */
        ctxt.copy = APC_COPY_IN_OPCODE;
        ctxt.force_update = 1;
        if (count == i || !atomic) {
            rval = apc_cache_insert_mult(apc_cache, keys, cache_entries, &ctxt, t, i TSRMLS_CC);
            atomic_fail = 0;
        } else {
            atomic_fail = 1;
        }

        zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(file), &hpos);
        for(c=0; c < i; c++) {
            zend_hash_get_current_data_ex(Z_ARRVAL_P(file), (void**)&hentry, &hpos);
            if (rval && rval[c] != 1) {
                add_assoc_long(return_value, Z_STRVAL_PP(hentry), -2);  /* -2: input or cache insertion error */
                if (cache_entries[c]) {
                    apc_pool_destroy(cache_entries[c]->pool TSRMLS_CC);
                }
            }
            if (op_arrays[c]) {
                destroy_op_array(op_arrays[c] TSRMLS_CC);
                efree(op_arrays[c]);
            }
            if (atomic_fail && cache_entries[c]) {
                apc_pool_destroy(cache_entries[c]->pool TSRMLS_CC);
            }
            if (keys[c].type == APC_CACHE_KEY_FPFILE) {
                efree((void*)keys[c].data.fpfile.fullpath);
            } else if (keys[c].type == APC_CACHE_KEY_USER) {
                efree((void*)keys[c].data.user.identifier);
            }
            zend_hash_move_forward_ex(Z_ARRVAL_P(file), &hpos);
        }
        efree(op_arrays);
        efree(keys);
        efree(cache_entries);
        if (rval) {
            efree(rval);
        }

    }

    /* Return class/function tables to previous states, destroy temp tables */
    APCG(force_file_update) = 0;
    CG(function_table) = cg_orig_function_table;
    zend_hash_destroy(&cg_function_table);
    CG(class_table) = cg_orig_class_table;
    zend_hash_destroy(&cg_class_table);
    EG(function_table) = eg_orig_function_table;
    EG(class_table) = eg_orig_class_table;

    /* Restore global settings */
    APCG(filters) = filters;
    APCG(cache_by_default) = cache_by_default;

    APCG(current_cache) = NULL;
    HANDLE_UNBLOCK_INTERRUPTIONS();

}
/* }}} */

/* {{{ proto mixed apc_bin_dump([array files [, array user_vars]])
    Returns a binary dump of the given files and user variables from the APC cache.
    A NULL for files or user_vars signals a dump of every entry, while array() will dump nothing.
 */
PHP_FUNCTION(apc_bin_dump) {

    zval *z_files = NULL, *z_user_vars = NULL;
    HashTable *h_files, *h_user_vars;
    apc_bd_t *bd;

    if(!APCG(enabled)) {
        apc_warning("APC is not enabled, apc_bin_dump not available." TSRMLS_CC);
        RETURN_FALSE;
    }

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|a!a!", &z_files, &z_user_vars) == FAILURE) {
        return;
    }

    h_files = z_files ? Z_ARRVAL_P(z_files) : NULL;
    h_user_vars = z_user_vars ? Z_ARRVAL_P(z_user_vars) : NULL;
    bd = apc_bin_dump(h_files, h_user_vars TSRMLS_CC);
    if(bd) {
        RETVAL_STRINGL((char*)bd, bd->size-1, 0);
    } else {
        apc_error("Unknown error encountered during apc_bin_dump." TSRMLS_CC);
        RETVAL_NULL();
    }

    return;
}

/* {{{ proto mixed apc_bin_dumpfile(array files, array user_vars, string filename, [int flags [, resource context]])
    Output a binary dump of the given files and user variables from the APC cache to the named file.
 */
PHP_FUNCTION(apc_bin_dumpfile) {

    zval *z_files = NULL, *z_user_vars = NULL;
    HashTable *h_files, *h_user_vars;
    char *filename = NULL;
    int filename_len;
    long flags=0;
    zval *zcontext = NULL;
    php_stream_context *context = NULL;
    php_stream *stream;
    int numbytes = 0;
    apc_bd_t *bd;

    if(!APCG(enabled)) {
        apc_warning("APC is not enabled, apc_bin_dumpfile not available." TSRMLS_CC);
        RETURN_FALSE;
    }


    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a!a!s|lr!", &z_files, &z_user_vars, &filename, &filename_len, &flags, &zcontext) == FAILURE) {
        return;
    }

    if(!filename_len) {
        apc_error("apc_bin_dumpfile filename argument must be a valid filename." TSRMLS_CC);
        RETURN_FALSE;
    }

    h_files = z_files ? Z_ARRVAL_P(z_files) : NULL;
    h_user_vars = z_user_vars ? Z_ARRVAL_P(z_user_vars) : NULL;
    bd = apc_bin_dump(h_files, h_user_vars TSRMLS_CC);
    if(!bd) {
        apc_error("Unknown error encountered during apc_bin_dumpfile." TSRMLS_CC);
        RETURN_FALSE;
    }


    /* Most of the following has been taken from the file_get/put_contents functions */

    context = php_stream_context_from_zval(zcontext, flags & PHP_FILE_NO_DEFAULT_CONTEXT);
    stream = php_stream_open_wrapper_ex(filename, (flags & PHP_FILE_APPEND) ? "ab" : "wb",
                                            ENFORCE_SAFE_MODE | REPORT_ERRORS, NULL, context);
    if (stream == NULL) {
        efree(bd);
        apc_error("Unable to write to file in apc_bin_dumpfile." TSRMLS_CC);
        RETURN_FALSE;
    }

    if (flags & LOCK_EX && php_stream_lock(stream, LOCK_EX)) {
        php_stream_close(stream);
        efree(bd);
        apc_error("Unable to get a lock on file in apc_bin_dumpfile." TSRMLS_CC);
        RETURN_FALSE;
    }

    numbytes = php_stream_write(stream, (char*)bd, bd->size);
    if(numbytes != bd->size) {
        numbytes = -1;
    }

    php_stream_close(stream);
    efree(bd);

    if(numbytes < 0) {
        apc_error("Only %d of %d bytes written, possibly out of free disk space" TSRMLS_CC, numbytes, bd->size);
        RETURN_FALSE;
    }

    RETURN_LONG(numbytes);
}

/* {{{ proto mixed apc_bin_load(string data, [int flags])
    Load the given binary dump into the APC file/user cache.
 */
PHP_FUNCTION(apc_bin_load) {

    int data_len;
    char *data;
    long flags = 0;

    if(!APCG(enabled)) {
        apc_warning("APC is not enabled, apc_bin_load not available." TSRMLS_CC);
        RETURN_FALSE;
    }

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &data, &data_len, &flags) == FAILURE) {
        return;
    }

    if(!data_len || data_len != ((apc_bd_t*)data)->size -1) {
        apc_error("apc_bin_load string argument does not appear to be a valid APC binary dump due to size (%d vs expected %d)." TSRMLS_CC, data_len, ((apc_bd_t*)data)->size -1);
        RETURN_FALSE;
    }

    apc_bin_load((apc_bd_t*)data, (int)flags TSRMLS_CC);

    RETURN_TRUE;
}

/* {{{ proto mixed apc_bin_loadfile(string filename, [resource context, [int flags]])
    Load the given binary dump from the named file into the APC file/user cache.
 */
PHP_FUNCTION(apc_bin_loadfile) {

    char *filename;
    int filename_len;
    zval *zcontext = NULL;
    long flags;
    php_stream_context *context = NULL;
    php_stream *stream;
    char *data;
    int len;

    if(!APCG(enabled)) {
        apc_warning("APC is not enabled, apc_bin_loadfile not available." TSRMLS_CC);
        RETURN_FALSE;
    }

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|r!l", &filename, &filename_len, &zcontext, &flags) == FAILURE) {
        return;
    }

    if(!filename_len) {
        apc_error("apc_bin_loadfile filename argument must be a valid filename." TSRMLS_CC);
        RETURN_FALSE;
    }

    context = php_stream_context_from_zval(zcontext, 0);
    stream = php_stream_open_wrapper_ex(filename, "rb",
            ENFORCE_SAFE_MODE | REPORT_ERRORS, NULL, context);
    if (!stream) {
        apc_error("Unable to read from file in apc_bin_loadfile." TSRMLS_CC);
        RETURN_FALSE;
    }

    len = php_stream_copy_to_mem(stream, &data, PHP_STREAM_COPY_ALL, 0);
    if(len == 0) {
        apc_warning("File passed to apc_bin_loadfile was empty: %s." TSRMLS_CC, filename);
        RETURN_FALSE;
    } else if(len < 0) {
        apc_warning("Error reading file passed to apc_bin_loadfile: %s." TSRMLS_CC, filename);
        RETURN_FALSE;
    } else if(len != ((apc_bd_t*)data)->size) {
        apc_warning("file passed to apc_bin_loadfile does not appear to be valid due to size (%d vs expected %d)." TSRMLS_CC, len, ((apc_bd_t*)data)->size -1);
        RETURN_FALSE;
    }
    php_stream_close(stream);

    apc_bin_load((apc_bd_t*)data, (int)flags TSRMLS_CC);
    efree(data);

    RETURN_TRUE;
}
/* }}} */

/* {{{ arginfo */
#if (PHP_MAJOR_VERSION >= 6 || (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3))
# define PHP_APC_ARGINFO
#else
# define PHP_APC_ARGINFO static
#endif

PHP_APC_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_apc_store, 0, 0, 2)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, var)
    ZEND_ARG_INFO(0, ttl)
ZEND_END_ARG_INFO()

PHP_APC_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_apc_clear_cache, 0, 0, 0)
    ZEND_ARG_INFO(0, info)
ZEND_END_ARG_INFO()

PHP_APC_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_apc_sma_info, 0, 0, 0)
    ZEND_ARG_INFO(0, limited)
ZEND_END_ARG_INFO()

PHP_APC_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_apc_cache_info, 0, 0, 0)
    ZEND_ARG_INFO(0, type)
    ZEND_ARG_INFO(0, limited)
ZEND_END_ARG_INFO()

PHP_APC_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_apc_define_constants, 0, 0, 2)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, constants)
    ZEND_ARG_INFO(0, case_sensitive)
ZEND_END_ARG_INFO()

PHP_APC_ARGINFO
ZEND_BEGIN_ARG_INFO(arginfo_apc_delete_file, 0)
	ZEND_ARG_INFO(0, keys)
ZEND_END_ARG_INFO()

PHP_APC_ARGINFO
ZEND_BEGIN_ARG_INFO(arginfo_apc_delete, 0)
	ZEND_ARG_INFO(0, keys)
ZEND_END_ARG_INFO()

PHP_APC_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_apc_fetch, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(1, success)
ZEND_END_ARG_INFO()

PHP_APC_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_apc_inc, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, step)
    ZEND_ARG_INFO(1, success)
ZEND_END_ARG_INFO()

PHP_APC_ARGINFO
ZEND_BEGIN_ARG_INFO(arginfo_apc_cas, 0)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, old)
    ZEND_ARG_INFO(0, new)
ZEND_END_ARG_INFO()

PHP_APC_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_apc_load_constants, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, case_sensitive)
ZEND_END_ARG_INFO()

PHP_APC_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_apc_compile_file, 0, 0, 1)
    ZEND_ARG_INFO(0, filenames)
    ZEND_ARG_INFO(0, atomic)
ZEND_END_ARG_INFO()

PHP_APC_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_apc_bin_dump, 0, 0, 0)
    ZEND_ARG_INFO(0, files)
    ZEND_ARG_INFO(0, user_vars)
ZEND_END_ARG_INFO()

PHP_APC_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_apc_bin_dumpfile, 0, 0, 3)
    ZEND_ARG_INFO(0, files)
    ZEND_ARG_INFO(0, user_vars)
    ZEND_ARG_INFO(0, filename)
    ZEND_ARG_INFO(0, flags)
    ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()

PHP_APC_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_apc_bin_load, 0, 0, 1)
    ZEND_ARG_INFO(0, data)
    ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

PHP_APC_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_apc_bin_loadfile, 0, 0, 1)
    ZEND_ARG_INFO(0, filename)
    ZEND_ARG_INFO(0, context)
    ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

PHP_APC_ARGINFO
ZEND_BEGIN_ARG_INFO(arginfo_apc_exists, 0)
	ZEND_ARG_INFO(0, keys)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ apc_functions[] */
zend_function_entry apc_functions[] = {
    PHP_FE(apc_cache_info,          arginfo_apc_cache_info)
    PHP_FE(apc_clear_cache,         arginfo_apc_clear_cache)
    PHP_FE(apc_sma_info,            arginfo_apc_sma_info)
    PHP_FE(apc_store,               arginfo_apc_store)
    PHP_FE(apc_fetch,               arginfo_apc_fetch)
    PHP_FE(apc_delete,              arginfo_apc_delete)
    PHP_FE(apc_delete_file,         arginfo_apc_delete_file)
    PHP_FE(apc_define_constants,    arginfo_apc_define_constants)
    PHP_FE(apc_load_constants,      arginfo_apc_load_constants)
    PHP_FE(apc_compile_file,        arginfo_apc_compile_file)
    PHP_FE(apc_add,                 arginfo_apc_store)
    PHP_FE(apc_inc,                 arginfo_apc_inc)
    PHP_FE(apc_dec,                 arginfo_apc_inc)
    PHP_FE(apc_cas,                 arginfo_apc_cas)
    PHP_FE(apc_bin_dump,            arginfo_apc_bin_dump)
    PHP_FE(apc_bin_load,            arginfo_apc_bin_load)
    PHP_FE(apc_bin_dumpfile,        arginfo_apc_bin_dumpfile)
    PHP_FE(apc_bin_loadfile,        arginfo_apc_bin_loadfile)
    PHP_FE(apc_exists,              arginfo_apc_exists)
    {NULL, NULL, NULL}
};
/* }}} */

/* {{{ module definition structure */

zend_module_entry apc_module_entry = {
    STANDARD_MODULE_HEADER,
    "apc",
    apc_functions,
    PHP_MINIT(apc),
    PHP_MSHUTDOWN(apc),
    PHP_RINIT(apc),
    PHP_RSHUTDOWN(apc),
    PHP_MINFO(apc),
    PHP_APC_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_APC
ZEND_GET_MODULE(apc)
#endif
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim>600: expandtab sw=4 ts=4 sts=4 fdm=marker
 * vim<600: expandtab sw=4 ts=4 sts=4
 */
