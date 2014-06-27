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
  | Authors: Georg Richter <georg@php.net>                               |
  |          Andrey Hristov <andrey@php.net>                             |
  |          Ulf Wendel <uw@php.net>                                     |
  +----------------------------------------------------------------------+

  $Id$
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <signal.h>

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "php_mysqli.h"
#include "php_mysqli_structs.h"
#include "mysqli_priv.h"
#include "zend_exceptions.h"
#include "zend_interfaces.h"

ZEND_DECLARE_MODULE_GLOBALS(mysqli)
static PHP_GINIT_FUNCTION(mysqli);

#define MYSQLI_ADD_PROPERTIES(a,b) \
{ \
	int i = 0; \
	while (b[i].pname != NULL) { \
		mysqli_add_property((a), (b)[i].pname, (b)[i].pname_length, \
							(mysqli_read_t)(b)[i].r_func, (mysqli_write_t)(b)[i].w_func TSRMLS_CC); \
		i++; \
	}\
}

#define MYSQLI_ADD_PROPERTIES_INFO(a,b) \
{ \
	int i = 0; \
	while (b[i].name != NULL) { \
		zend_declare_property_null((a), (b)[i].name, (b)[i].name_length, ZEND_ACC_PUBLIC TSRMLS_CC); \
		i++; \
	}\
}



static zend_object_handlers mysqli_object_handlers;
static HashTable classes;
static HashTable mysqli_driver_properties;
static HashTable mysqli_link_properties;
static HashTable mysqli_result_properties;
static HashTable mysqli_stmt_properties;
static HashTable mysqli_warning_properties;

zend_class_entry *mysqli_link_class_entry;
zend_class_entry *mysqli_stmt_class_entry;
zend_class_entry *mysqli_result_class_entry;
zend_class_entry *mysqli_driver_class_entry;
zend_class_entry *mysqli_warning_class_entry;
zend_class_entry *mysqli_exception_class_entry;


typedef int (*mysqli_read_t)(mysqli_object *obj, zval **retval TSRMLS_DC);
typedef int (*mysqli_write_t)(mysqli_object *obj, zval *newval TSRMLS_DC);

typedef struct _mysqli_prop_handler {
	char *name;
	size_t name_len;
	mysqli_read_t read_func;
	mysqli_write_t write_func;
} mysqli_prop_handler;

static int le_pmysqli;


/* Destructor for mysqli entries in free_links/used_links */
void php_mysqli_dtor_p_elements(void *data)
{
	MYSQL *mysql = (MYSQL *) data;
	TSRMLS_FETCH();
	mysqli_close(mysql, MYSQLI_CLOSE_IMPLICIT);
}


ZEND_RSRC_DTOR_FUNC(php_mysqli_dtor)
{
	if (rsrc->ptr) {
		mysqli_plist_entry *plist = (mysqli_plist_entry *) rsrc->ptr;
		zend_ptr_stack_clean(&plist->free_links, php_mysqli_dtor_p_elements, 0);
		zend_ptr_stack_destroy(&plist->free_links);
		free(plist);
	}
}


int php_le_pmysqli(void)
{
	return le_pmysqli;
}

#ifndef MYSQLI_USE_MYSQLND
/* {{{ php_free_stmt_bind_buffer */
void php_free_stmt_bind_buffer(BIND_BUFFER bbuf, int type)
{
	unsigned int i;

	if (!bbuf.var_cnt) {
		return;
	}

	for (i=0; i < bbuf.var_cnt; i++) {

		/* free temporary bind buffer */
		if (type == FETCH_RESULT && bbuf.buf[i].val) {
			efree(bbuf.buf[i].val);
		}

		if (bbuf.vars[i]) {
			zval_ptr_dtor(&bbuf.vars[i]);
		}
	}

	if (bbuf.vars) {
		efree(bbuf.vars);
	}

	/*
	  Don't free bbuf.is_null for FETCH_RESULT since we have allocated
	  is_null and buf in one block so we free only buf, which is the beginning
	  of the block. When FETCH_SIMPLE then buf wasn't allocated together with
	  buf and we have to free it.
	*/
	if (type == FETCH_RESULT) {
		efree(bbuf.buf);
	} else if (type == FETCH_SIMPLE){
		efree(bbuf.is_null);
	}

	bbuf.var_cnt = 0;
}
/* }}} */
#endif

/* {{{ php_clear_stmt_bind */
void php_clear_stmt_bind(MY_STMT *stmt TSRMLS_DC)
{
	if (stmt->stmt) {
		if (mysqli_stmt_close(stmt->stmt, TRUE)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Error occurred while closing statement");
			return;
		}
	}

	/*
	  mysqlnd keeps track of the binding and has freed its
	  structures in stmt_close() above
	*/
#ifndef MYSQLI_USE_MYSQLND
	/* Clean param bind */
	php_free_stmt_bind_buffer(stmt->param, FETCH_SIMPLE);
	/* Clean output bind */
	php_free_stmt_bind_buffer(stmt->result, FETCH_RESULT);

	if (stmt->link_handle) {
	    zend_objects_store_del_ref_by_handle(stmt->link_handle TSRMLS_CC);
	}
#endif
	if (stmt->query) {
		efree(stmt->query);
	}
	efree(stmt);
}
/* }}} */

/* {{{ php_clear_mysql */
void php_clear_mysql(MY_MYSQL *mysql) {
	if (mysql->hash_key) {
		efree(mysql->hash_key);
		mysql->hash_key = NULL;
	}
	if (mysql->li_read) {
		zval_ptr_dtor(&(mysql->li_read));
		mysql->li_read = NULL;
	}
}
/* }}} */

/* {{{ mysqli_objects_free_storage
 */
static void mysqli_objects_free_storage(void *object TSRMLS_DC)
{
	zend_object *zo = (zend_object *)object;
	mysqli_object 	*intern = (mysqli_object *)zo;
	MYSQLI_RESOURCE	*my_res = (MYSQLI_RESOURCE *)intern->ptr;

	my_efree(my_res);
	zend_object_std_dtor(&intern->zo TSRMLS_CC);
	efree(intern);
}
/* }}} */

/* mysqli_link_free_storage partly doubles the work of PHP_FUNCTION(mysqli_close) */

/* {{{ mysqli_link_free_storage
 */
static void mysqli_link_free_storage(void *object TSRMLS_DC)
{
	zend_object *zo = (zend_object *)object;
	mysqli_object 	*intern = (mysqli_object *)zo;
	MYSQLI_RESOURCE	*my_res = (MYSQLI_RESOURCE *)intern->ptr;

	if (my_res && my_res->ptr) {
		MY_MYSQL *mysql = (MY_MYSQL *)my_res->ptr;
		if (mysql->mysql) {
			php_mysqli_close(mysql, MYSQLI_CLOSE_EXPLICIT, my_res->status TSRMLS_CC);
		}
		php_clear_mysql(mysql);
		efree(mysql);
		my_res->status = MYSQLI_STATUS_UNKNOWN;
	}
	mysqli_objects_free_storage(object TSRMLS_CC);
}
/* }}} */

/* {{{ mysql_driver_free_storage */
static void mysqli_driver_free_storage(void *object TSRMLS_DC)
{
	mysqli_objects_free_storage(object TSRMLS_CC);
}
/* }}} */

/* {{{ mysqli_stmt_free_storage
 */
static void mysqli_stmt_free_storage(void *object TSRMLS_DC)
{
	zend_object *zo = (zend_object *)object;
	mysqli_object 	*intern = (mysqli_object *)zo;
	MYSQLI_RESOURCE	*my_res = (MYSQLI_RESOURCE *)intern->ptr;

	if (my_res && my_res->ptr) {
		MY_STMT *stmt = (MY_STMT *)my_res->ptr;
		php_clear_stmt_bind(stmt TSRMLS_CC);
	}
	mysqli_objects_free_storage(object TSRMLS_CC);
}
/* }}} */

/* {{{ mysqli_result_free_storage
 */
static void mysqli_result_free_storage(void *object TSRMLS_DC)
{
	zend_object *zo = (zend_object *)object;
	mysqli_object 	*intern = (mysqli_object *)zo;
	MYSQLI_RESOURCE	*my_res = (MYSQLI_RESOURCE *)intern->ptr;

	if (my_res && my_res->ptr) {
		mysql_free_result(my_res->ptr);
	}
	mysqli_objects_free_storage(object TSRMLS_CC);
}
/* }}} */

/* {{{ mysqli_warning_free_storage
 */
static void mysqli_warning_free_storage(void *object TSRMLS_DC)
{
	zend_object *zo = (zend_object *)object;
	mysqli_object 	*intern = (mysqli_object *)zo;
	MYSQLI_RESOURCE	*my_res = (MYSQLI_RESOURCE *)intern->ptr;

	if (my_res && my_res->ptr) {
		php_clear_warnings((MYSQLI_WARNING *)my_res->info);
		my_res->ptr = NULL;
	}
	mysqli_objects_free_storage(object TSRMLS_CC);
}
/* }}} */

/* {{{ mysqli_read_na */
static int mysqli_read_na(mysqli_object *obj, zval **retval TSRMLS_DC)
{
	*retval = NULL;
	php_error_docref(NULL TSRMLS_CC, E_ERROR, "Cannot read property");
	return FAILURE;
}
/* }}} */

/* {{{ mysqli_write_na */
static int mysqli_write_na(mysqli_object *obj, zval *newval TSRMLS_DC)
{
	php_error_docref(NULL TSRMLS_CC, E_ERROR, "Cannot write property");
	return FAILURE;
}
/* }}} */

#ifndef Z_ADDREF_P
/* PHP 5.2, old GC */
#define Z_ADDREF_P(pz)				(++(pz)->refcount)
#define Z_REFCOUNT_P(pz)			((pz)->refcount)
#define Z_SET_REFCOUNT_P(pz, rc)	((pz)->refcount = rc)
#endif


/* {{{ mysqli_read_property */
zval *mysqli_read_property(zval *object, zval *member, int type, const zend_literal *key TSRMLS_DC)
{
	zval tmp_member;
	zval *retval;
	mysqli_object *obj;
	mysqli_prop_handler *hnd;
	int ret;

	ret = FAILURE;
	obj = (mysqli_object *)zend_objects_get_address(object TSRMLS_CC);

	if (member->type != IS_STRING) {
		tmp_member = *member;
		zval_copy_ctor(&tmp_member);
		convert_to_string(&tmp_member);
		member = &tmp_member;
	}

	if (obj->prop_handler != NULL) {
		ret = zend_hash_find(obj->prop_handler, Z_STRVAL_P(member), Z_STRLEN_P(member)+1, (void **) &hnd);
	}

	if (ret == SUCCESS) {
		ret = hnd->read_func(obj, &retval TSRMLS_CC);
		if (ret == SUCCESS) {
			/* ensure we're creating a temporary variable */
			Z_SET_REFCOUNT_P(retval, 0);
		} else {
			retval = EG(uninitialized_zval_ptr);
		}
	} else {
		zend_object_handlers * std_hnd = zend_get_std_object_handlers();
		retval = std_hnd->read_property(object, member, type, key TSRMLS_CC);
	}

	if (member == &tmp_member) {
		zval_dtor(member);
	}
	return(retval);
}
/* }}} */

/* {{{ mysqli_write_property */
void mysqli_write_property(zval *object, zval *member, zval *value, const zend_literal *key TSRMLS_DC)
{
	zval tmp_member;
	mysqli_object *obj;
	mysqli_prop_handler *hnd;
	int ret;

	if (member->type != IS_STRING) {
		tmp_member = *member;
		zval_copy_ctor(&tmp_member);
		convert_to_string(&tmp_member);
		member = &tmp_member;
	}

	ret = FAILURE;
	obj = (mysqli_object *)zend_objects_get_address(object TSRMLS_CC);

	if (obj->prop_handler != NULL) {
		ret = zend_hash_find((HashTable *)obj->prop_handler, Z_STRVAL_P(member), Z_STRLEN_P(member)+1, (void **) &hnd);
	}
	if (ret == SUCCESS) {
		hnd->write_func(obj, value TSRMLS_CC);
		if (! PZVAL_IS_REF(value) && Z_REFCOUNT_P(value) == 0) {
			Z_ADDREF_P(value);
			zval_ptr_dtor(&value);
		}
	} else {
		zend_object_handlers * std_hnd = zend_get_std_object_handlers();
		std_hnd->write_property(object, member, value, key TSRMLS_CC);
	}

	if (member == &tmp_member) {
		zval_dtor(member);
	}
}
/* }}} */

/* {{{ void mysqli_add_property(HashTable *h, char *pname, mysqli_read_t r_func, mysqli_write_t w_func TSRMLS_DC) */
void mysqli_add_property(HashTable *h, const char *pname, size_t pname_len, mysqli_read_t r_func, mysqli_write_t w_func TSRMLS_DC) {
	mysqli_prop_handler		p;

	p.name = (char*) pname;
	p.name_len = pname_len;
	p.read_func = (r_func) ? r_func : mysqli_read_na;
	p.write_func = (w_func) ? w_func : mysqli_write_na;
	zend_hash_add(h, pname, pname_len + 1, &p, sizeof(mysqli_prop_handler), NULL);
}
/* }}} */

static int mysqli_object_has_property(zval *object, zval *member, int has_set_exists, const zend_literal *key TSRMLS_DC) /* {{{ */
{
	mysqli_object *obj = (mysqli_object *)zend_objects_get_address(object TSRMLS_CC);
	mysqli_prop_handler	p;
	int ret = 0;

	if (zend_hash_find(obj->prop_handler, Z_STRVAL_P(member), Z_STRLEN_P(member) + 1, (void **)&p) == SUCCESS) {
		switch (has_set_exists) {
			case 2:
				ret = 1;
				break;
			case 1: {
				zval *value = mysqli_read_property(object, member, BP_VAR_IS, key TSRMLS_CC);
				if (value != EG(uninitialized_zval_ptr)) {
					convert_to_boolean(value);
					ret = Z_BVAL_P(value)? 1:0;
					/* refcount is 0 */
					Z_ADDREF_P(value);
					zval_ptr_dtor(&value);
				}
				break;
			}
			case 0:{
				zval *value = mysqli_read_property(object, member, BP_VAR_IS, key TSRMLS_CC);
				if (value != EG(uninitialized_zval_ptr)) {
					ret = Z_TYPE_P(value) != IS_NULL? 1:0;
					/* refcount is 0 */
					Z_ADDREF_P(value);
					zval_ptr_dtor(&value);
				}
				break;
			}
			default:
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid value for has_set_exists");
		}
	} else {
		zend_object_handlers * std_hnd = zend_get_std_object_handlers();
		ret = std_hnd->has_property(object, member, has_set_exists, key TSRMLS_CC);
	}
	return ret;
} /* }}} */


#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3
HashTable * mysqli_object_get_debug_info(zval *object, int *is_temp TSRMLS_DC)
{
	mysqli_object *obj = (mysqli_object *)zend_objects_get_address(object TSRMLS_CC);
	HashTable *retval, *props = obj->prop_handler;
	HashPosition pos;
	mysqli_prop_handler *entry;

	ALLOC_HASHTABLE(retval);
	ZEND_INIT_SYMTABLE_EX(retval, zend_hash_num_elements(props) + 1, 0);

	zend_hash_internal_pointer_reset_ex(props, &pos);
	while (zend_hash_get_current_data_ex(props, (void **)&entry, &pos) == SUCCESS) {
		zval member;
		zval *value;
		INIT_ZVAL(member);
		ZVAL_STRINGL(&member, entry->name, entry->name_len, 0);
		value = mysqli_read_property(object, &member, BP_VAR_IS, 0 TSRMLS_CC);
		if (value != EG(uninitialized_zval_ptr)) {
			Z_ADDREF_P(value);
			zend_hash_add(retval, entry->name, entry->name_len + 1, &value, sizeof(zval *), NULL);
		}
		zend_hash_move_forward_ex(props, &pos);
	}

	*is_temp = 1;
	return retval;
}
#endif

/* {{{ mysqli_objects_new
 */
PHP_MYSQLI_EXPORT(zend_object_value) mysqli_objects_new(zend_class_entry *class_type TSRMLS_DC)
{
	zend_object_value retval;
	mysqli_object *intern;
	zend_class_entry *mysqli_base_class;
	zend_objects_free_object_storage_t free_storage;

	intern = emalloc(sizeof(mysqli_object));
	memset(intern, 0, sizeof(mysqli_object));
	intern->ptr = NULL;
	intern->prop_handler = NULL;

	mysqli_base_class = class_type;
	while (mysqli_base_class->type != ZEND_INTERNAL_CLASS &&
		   mysqli_base_class->parent != NULL) {
		mysqli_base_class = mysqli_base_class->parent;
	}
	zend_hash_find(&classes, mysqli_base_class->name, mysqli_base_class->name_length + 1,
					(void **) &intern->prop_handler);

	zend_object_std_init(&intern->zo, class_type TSRMLS_CC);
	object_properties_init(&intern->zo, class_type);

	/* link object */
	if (instanceof_function(class_type, mysqli_link_class_entry TSRMLS_CC)) {
		free_storage = mysqli_link_free_storage;
	} else if (instanceof_function(class_type, mysqli_driver_class_entry TSRMLS_CC)) { /* driver object */
		free_storage = mysqli_driver_free_storage;
	} else if (instanceof_function(class_type, mysqli_stmt_class_entry TSRMLS_CC)) { /* stmt object */
		free_storage = mysqli_stmt_free_storage;
	} else if (instanceof_function(class_type, mysqli_result_class_entry TSRMLS_CC)) { /* result object */
		free_storage = mysqli_result_free_storage;
	} else if (instanceof_function(class_type, mysqli_warning_class_entry TSRMLS_CC)) { /* warning object */
		free_storage = mysqli_warning_free_storage;
	} else {
		free_storage = mysqli_objects_free_storage;
	}

	retval.handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t) zend_objects_destroy_object, free_storage, NULL TSRMLS_CC);
	retval.handlers = &mysqli_object_handlers;

	return retval;
}
/* }}} */

#ifdef MYSQLI_USE_MYSQLND
#include "ext/mysqlnd/mysqlnd_reverse_api.h"
static MYSQLND *mysqli_convert_zv_to_mysqlnd(zval * zv TSRMLS_DC)
{
	if (Z_TYPE_P(zv) == IS_OBJECT && instanceof_function(Z_OBJCE_P(zv), mysqli_link_class_entry TSRMLS_CC)) {
		MY_MYSQL * mysql;
		MYSQLI_RESOURCE  * my_res;
		mysqli_object * intern = (mysqli_object *)zend_object_store_get_object(zv TSRMLS_CC);
		if (!(my_res = (MYSQLI_RESOURCE *)intern->ptr)) {
			/* We know that we have a mysqli object, so this failure should be emitted */
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Couldn't fetch %s", intern->zo.ce->name);
			return NULL;
		}
		mysql = (MY_MYSQL *)(my_res->ptr);
		return mysql ? mysql->mysql : NULL;
	}
	return NULL;
}

static MYSQLND_REVERSE_API mysqli_reverse_api = {
	&mysqli_module_entry,
	mysqli_convert_zv_to_mysqlnd
};
#endif

/* {{{ PHP_INI_BEGIN
*/
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY_EX("mysqli.max_links",			"-1",	PHP_INI_SYSTEM,		OnUpdateLong,		max_links,			zend_mysqli_globals,		mysqli_globals, display_link_numbers)
	STD_PHP_INI_ENTRY_EX("mysqli.max_persistent",		"-1",	PHP_INI_SYSTEM,		OnUpdateLong,		max_persistent,		zend_mysqli_globals,		mysqli_globals,	display_link_numbers)
	STD_PHP_INI_BOOLEAN("mysqli.allow_persistent",		"1",	PHP_INI_SYSTEM,		OnUpdateLong,		allow_persistent,	zend_mysqli_globals,		mysqli_globals)
	STD_PHP_INI_ENTRY("mysqli.default_host",			NULL,	PHP_INI_ALL,		OnUpdateString,		default_host,		zend_mysqli_globals,		mysqli_globals)
	STD_PHP_INI_ENTRY("mysqli.default_user",			NULL,	PHP_INI_ALL,		OnUpdateString,		default_user,		zend_mysqli_globals,		mysqli_globals)
	STD_PHP_INI_ENTRY("mysqli.default_pw",				NULL,	PHP_INI_ALL,		OnUpdateString,		default_pw,			zend_mysqli_globals,		mysqli_globals)
	STD_PHP_INI_ENTRY("mysqli.default_port",			"3306",	PHP_INI_ALL,		OnUpdateLong,		default_port,		zend_mysqli_globals,		mysqli_globals)
#ifdef PHP_MYSQL_UNIX_SOCK_ADDR
	STD_PHP_INI_ENTRY("mysqli.default_socket",			MYSQL_UNIX_ADDR,PHP_INI_ALL,OnUpdateStringUnempty,	default_socket,	zend_mysqli_globals,		mysqli_globals)
#else
	STD_PHP_INI_ENTRY("mysqli.default_socket",			NULL,	PHP_INI_ALL,		OnUpdateStringUnempty,	default_socket,	zend_mysqli_globals,		mysqli_globals)
#endif
	STD_PHP_INI_BOOLEAN("mysqli.reconnect",				"0",	PHP_INI_SYSTEM,		OnUpdateLong,		reconnect,			zend_mysqli_globals,		mysqli_globals)
	STD_PHP_INI_BOOLEAN("mysqli.allow_local_infile",	"1",	PHP_INI_SYSTEM,		OnUpdateLong,		allow_local_infile,	zend_mysqli_globals,		mysqli_globals)
PHP_INI_END()
/* }}} */


/* {{{ PHP_GINIT_FUNCTION
 */
static PHP_GINIT_FUNCTION(mysqli)
{
	mysqli_globals->num_links = 0;
	mysqli_globals->num_active_persistent = 0;
	mysqli_globals->num_inactive_persistent = 0;
	mysqli_globals->max_links = -1;
	mysqli_globals->max_persistent = -1;
	mysqli_globals->allow_persistent = 1;
	mysqli_globals->default_port = 0;
	mysqli_globals->default_host = NULL;
	mysqli_globals->default_user = NULL;
	mysqli_globals->default_pw = NULL;
	mysqli_globals->default_socket = NULL;
	mysqli_globals->reconnect = 0;
	mysqli_globals->report_mode = 0;
	mysqli_globals->report_ht = 0;
	mysqli_globals->allow_local_infile = 1;
#ifdef HAVE_EMBEDDED_MYSQLI
	mysqli_globals->embedded = 1;
#else
	mysqli_globals->embedded = 0;
#endif
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(mysqli)
{
	zend_class_entry *ce,cex;
	zend_object_handlers *std_hnd = zend_get_std_object_handlers();

	REGISTER_INI_ENTRIES();
#ifndef MYSQLI_USE_MYSQLND
#if MYSQL_VERSION_ID >= 40000
	if (mysql_server_init(0, NULL, NULL)) {
		return FAILURE;
	}
#endif
#endif

	memcpy(&mysqli_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	mysqli_object_handlers.clone_obj = NULL;
	mysqli_object_handlers.read_property = mysqli_read_property;
	mysqli_object_handlers.write_property = mysqli_write_property;
	mysqli_object_handlers.get_property_ptr_ptr = std_hnd->get_property_ptr_ptr;
	mysqli_object_handlers.has_property = mysqli_object_has_property;
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3
	mysqli_object_handlers.get_debug_info = mysqli_object_get_debug_info;
#endif

	zend_hash_init(&classes, 0, NULL, NULL, 1);

	/* persistent connections */
	le_pmysqli = zend_register_list_destructors_ex(NULL, php_mysqli_dtor,
		"MySqli persistent connection", module_number);

	INIT_CLASS_ENTRY(cex, "mysqli_sql_exception", mysqli_exception_methods);
#ifdef HAVE_SPL
	mysqli_exception_class_entry = zend_register_internal_class_ex(&cex, spl_ce_RuntimeException, NULL TSRMLS_CC);
#else
	mysqli_exception_class_entry = zend_register_internal_class_ex(&cex, zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);
#endif
	mysqli_exception_class_entry->ce_flags |= ZEND_ACC_FINAL;
	zend_declare_property_long(mysqli_exception_class_entry, "code", sizeof("code")-1, 0, ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_string(mysqli_exception_class_entry, "sqlstate", sizeof("sqlstate")-1, "00000", ZEND_ACC_PROTECTED TSRMLS_CC);

	REGISTER_MYSQLI_CLASS_ENTRY("mysqli_driver", mysqli_driver_class_entry, mysqli_driver_methods);
	ce = mysqli_driver_class_entry;
	zend_hash_init(&mysqli_driver_properties, 0, NULL, NULL, 1);
	MYSQLI_ADD_PROPERTIES(&mysqli_driver_properties, mysqli_driver_property_entries);
	MYSQLI_ADD_PROPERTIES_INFO(ce, mysqli_driver_property_info_entries);
	zend_hash_add(&classes, ce->name, ce->name_length+1, &mysqli_driver_properties, sizeof(mysqli_driver_properties), NULL);
	ce->ce_flags |= ZEND_ACC_FINAL_CLASS;

	REGISTER_MYSQLI_CLASS_ENTRY("mysqli", mysqli_link_class_entry, mysqli_link_methods);
	ce = mysqli_link_class_entry;
	zend_hash_init(&mysqli_link_properties, 0, NULL, NULL, 1);
	MYSQLI_ADD_PROPERTIES(&mysqli_link_properties, mysqli_link_property_entries);
	MYSQLI_ADD_PROPERTIES_INFO(ce, mysqli_link_property_info_entries);
	zend_hash_add(&classes, ce->name, ce->name_length+1, &mysqli_link_properties, sizeof(mysqli_link_properties), NULL);

	REGISTER_MYSQLI_CLASS_ENTRY("mysqli_warning", mysqli_warning_class_entry, mysqli_warning_methods);
	ce = mysqli_warning_class_entry;
	ce->ce_flags |= ZEND_ACC_FINAL_CLASS | ZEND_ACC_PROTECTED;
	zend_hash_init(&mysqli_warning_properties, 0, NULL, NULL, 1);
	MYSQLI_ADD_PROPERTIES(&mysqli_warning_properties, mysqli_warning_property_entries);
	MYSQLI_ADD_PROPERTIES_INFO(ce, mysqli_warning_property_info_entries);
	zend_hash_add(&classes, ce->name, ce->name_length+1, &mysqli_warning_properties, sizeof(mysqli_warning_properties), NULL);

	REGISTER_MYSQLI_CLASS_ENTRY("mysqli_result", mysqli_result_class_entry, mysqli_result_methods);
	ce = mysqli_result_class_entry;
	zend_hash_init(&mysqli_result_properties, 0, NULL, NULL, 1);
	MYSQLI_ADD_PROPERTIES(&mysqli_result_properties, mysqli_result_property_entries);
	MYSQLI_ADD_PROPERTIES_INFO(ce, mysqli_result_property_info_entries);
	mysqli_result_class_entry->get_iterator = php_mysqli_result_get_iterator;
	mysqli_result_class_entry->iterator_funcs.funcs = &php_mysqli_result_iterator_funcs;
	zend_class_implements(mysqli_result_class_entry TSRMLS_CC, 1, zend_ce_traversable);
	zend_hash_add(&classes, ce->name, ce->name_length+1, &mysqli_result_properties, sizeof(mysqli_result_properties), NULL);

	REGISTER_MYSQLI_CLASS_ENTRY("mysqli_stmt", mysqli_stmt_class_entry, mysqli_stmt_methods);
	ce = mysqli_stmt_class_entry;
	zend_hash_init(&mysqli_stmt_properties, 0, NULL, NULL, 1);
	MYSQLI_ADD_PROPERTIES(&mysqli_stmt_properties, mysqli_stmt_property_entries);
	MYSQLI_ADD_PROPERTIES_INFO(ce, mysqli_stmt_property_info_entries);
	zend_hash_add(&classes, ce->name, ce->name_length+1, &mysqli_stmt_properties, sizeof(mysqli_stmt_properties), NULL);

	/* mysqli_options */
	REGISTER_LONG_CONSTANT("MYSQLI_READ_DEFAULT_GROUP", MYSQL_READ_DEFAULT_GROUP, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_READ_DEFAULT_FILE", MYSQL_READ_DEFAULT_FILE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_OPT_CONNECT_TIMEOUT", MYSQL_OPT_CONNECT_TIMEOUT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_OPT_LOCAL_INFILE", MYSQL_OPT_LOCAL_INFILE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_INIT_COMMAND", MYSQL_INIT_COMMAND, CONST_CS | CONST_PERSISTENT);
#if defined(MYSQLI_USE_MYSQLND)
	REGISTER_LONG_CONSTANT("MYSQLI_OPT_NET_CMD_BUFFER_SIZE", MYSQLND_OPT_NET_CMD_BUFFER_SIZE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_OPT_NET_READ_BUFFER_SIZE", MYSQLND_OPT_NET_READ_BUFFER_SIZE, CONST_CS | CONST_PERSISTENT);
#endif
#ifdef MYSQLND_STRING_TO_INT_CONVERSION
	REGISTER_LONG_CONSTANT("MYSQLI_OPT_INT_AND_FLOAT_NATIVE", MYSQLND_OPT_INT_AND_FLOAT_NATIVE, CONST_CS | CONST_PERSISTENT);
#endif
#if MYSQL_VERSION_ID > 50110 || defined(MYSQLI_USE_MYSQLND)
	REGISTER_LONG_CONSTANT("MYSQLI_OPT_SSL_VERIFY_SERVER_CERT", MYSQL_OPT_SSL_VERIFY_SERVER_CERT, CONST_CS | CONST_PERSISTENT);
#endif

#if MYSQL_VERSION_ID > 50605 || defined(MYSQLI_USE_MYSQLND)
	REGISTER_LONG_CONSTANT("MYSQLI_SERVER_PUBLIC_KEY", MYSQL_SERVER_PUBLIC_KEY, CONST_CS | CONST_PERSISTENT);
#endif

	/* mysqli_real_connect flags */
	REGISTER_LONG_CONSTANT("MYSQLI_CLIENT_SSL", CLIENT_SSL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_CLIENT_COMPRESS",CLIENT_COMPRESS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_CLIENT_INTERACTIVE", CLIENT_INTERACTIVE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_CLIENT_IGNORE_SPACE", CLIENT_IGNORE_SPACE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_CLIENT_NO_SCHEMA", CLIENT_NO_SCHEMA, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_CLIENT_FOUND_ROWS", CLIENT_FOUND_ROWS, CONST_CS | CONST_PERSISTENT);

	/* for mysqli_query */
	REGISTER_LONG_CONSTANT("MYSQLI_STORE_RESULT", MYSQLI_STORE_RESULT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_USE_RESULT", MYSQLI_USE_RESULT, CONST_CS | CONST_PERSISTENT);
#if defined (MYSQLI_USE_MYSQLND)
	REGISTER_LONG_CONSTANT("MYSQLI_ASYNC", MYSQLI_ASYNC, CONST_CS | CONST_PERSISTENT);
#endif

	/* for mysqli_fetch_assoc */
	REGISTER_LONG_CONSTANT("MYSQLI_ASSOC", MYSQLI_ASSOC, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_NUM", MYSQLI_NUM, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_BOTH", MYSQLI_BOTH, CONST_CS | CONST_PERSISTENT);

	/* for mysqli_stmt_set_attr */
	REGISTER_LONG_CONSTANT("MYSQLI_STMT_ATTR_UPDATE_MAX_LENGTH", STMT_ATTR_UPDATE_MAX_LENGTH, CONST_CS | CONST_PERSISTENT);

#if MYSQL_VERSION_ID > 50003 || defined(MYSQLI_USE_MYSQLND)
	REGISTER_LONG_CONSTANT("MYSQLI_STMT_ATTR_CURSOR_TYPE", STMT_ATTR_CURSOR_TYPE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_CURSOR_TYPE_NO_CURSOR", CURSOR_TYPE_NO_CURSOR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_CURSOR_TYPE_READ_ONLY", CURSOR_TYPE_READ_ONLY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_CURSOR_TYPE_FOR_UPDATE", CURSOR_TYPE_FOR_UPDATE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_CURSOR_TYPE_SCROLLABLE", CURSOR_TYPE_SCROLLABLE, CONST_CS | CONST_PERSISTENT);
#endif

#if MYSQL_VERSION_ID > 50007 || defined(MYSQLI_USE_MYSQLND)
	REGISTER_LONG_CONSTANT("MYSQLI_STMT_ATTR_PREFETCH_ROWS", STMT_ATTR_PREFETCH_ROWS, CONST_CS | CONST_PERSISTENT);
#endif

	/* column information */
	REGISTER_LONG_CONSTANT("MYSQLI_NOT_NULL_FLAG", NOT_NULL_FLAG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_PRI_KEY_FLAG", PRI_KEY_FLAG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_UNIQUE_KEY_FLAG", UNIQUE_KEY_FLAG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_MULTIPLE_KEY_FLAG", MULTIPLE_KEY_FLAG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_BLOB_FLAG", BLOB_FLAG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_UNSIGNED_FLAG", UNSIGNED_FLAG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_ZEROFILL_FLAG", ZEROFILL_FLAG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_AUTO_INCREMENT_FLAG", AUTO_INCREMENT_FLAG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TIMESTAMP_FLAG", TIMESTAMP_FLAG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_SET_FLAG", SET_FLAG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_NUM_FLAG", NUM_FLAG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_PART_KEY_FLAG", PART_KEY_FLAG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_GROUP_FLAG", GROUP_FLAG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_ENUM_FLAG", ENUM_FLAG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_BINARY_FLAG", BINARY_FLAG, CONST_CS | CONST_PERSISTENT);
#if MYSQL_VERSION_ID > 50001 || defined(MYSQLI_USE_MYSQLND)
	REGISTER_LONG_CONSTANT("MYSQLI_NO_DEFAULT_VALUE_FLAG", NO_DEFAULT_VALUE_FLAG, CONST_CS | CONST_PERSISTENT);
#endif

#if (MYSQL_VERSION_ID > 51122 && MYSQL_VERSION_ID < 60000) || (MYSQL_VERSION_ID > 60003) || defined(MYSQLI_USE_MYSQLND)
	REGISTER_LONG_CONSTANT("MYSQLI_ON_UPDATE_NOW_FLAG", ON_UPDATE_NOW_FLAG, CONST_CS | CONST_PERSISTENT);
#endif

	REGISTER_LONG_CONSTANT("MYSQLI_TYPE_DECIMAL", FIELD_TYPE_DECIMAL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TYPE_TINY", FIELD_TYPE_TINY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TYPE_SHORT", FIELD_TYPE_SHORT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TYPE_LONG", FIELD_TYPE_LONG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TYPE_FLOAT", FIELD_TYPE_FLOAT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TYPE_DOUBLE", FIELD_TYPE_DOUBLE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TYPE_NULL", FIELD_TYPE_NULL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TYPE_TIMESTAMP", FIELD_TYPE_TIMESTAMP, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TYPE_LONGLONG", FIELD_TYPE_LONGLONG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TYPE_INT24", FIELD_TYPE_INT24, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TYPE_DATE", FIELD_TYPE_DATE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TYPE_TIME", FIELD_TYPE_TIME, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TYPE_DATETIME", FIELD_TYPE_DATETIME	, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TYPE_YEAR", FIELD_TYPE_YEAR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TYPE_NEWDATE", FIELD_TYPE_NEWDATE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TYPE_ENUM", FIELD_TYPE_ENUM, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TYPE_SET", FIELD_TYPE_SET, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TYPE_TINY_BLOB", FIELD_TYPE_TINY_BLOB, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TYPE_MEDIUM_BLOB", FIELD_TYPE_MEDIUM_BLOB, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TYPE_LONG_BLOB", FIELD_TYPE_LONG_BLOB, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TYPE_BLOB", FIELD_TYPE_BLOB, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TYPE_VAR_STRING", FIELD_TYPE_VAR_STRING, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TYPE_STRING", FIELD_TYPE_STRING, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TYPE_CHAR", FIELD_TYPE_CHAR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TYPE_INTERVAL", FIELD_TYPE_INTERVAL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TYPE_GEOMETRY", FIELD_TYPE_GEOMETRY, CONST_CS | CONST_PERSISTENT);

#if MYSQL_VERSION_ID > 50002 || defined(MYSQLI_USE_MYSQLND)
	REGISTER_LONG_CONSTANT("MYSQLI_TYPE_NEWDECIMAL", FIELD_TYPE_NEWDECIMAL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TYPE_BIT", FIELD_TYPE_BIT, CONST_CS | CONST_PERSISTENT);
#endif

	REGISTER_LONG_CONSTANT("MYSQLI_SET_CHARSET_NAME", MYSQL_SET_CHARSET_NAME, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_SET_CHARSET_DIR", MYSQL_SET_CHARSET_DIR, CONST_CS | CONST_PERSISTENT);

	/* bind support */
	REGISTER_LONG_CONSTANT("MYSQLI_NO_DATA", MYSQL_NO_DATA, CONST_CS | CONST_PERSISTENT);
#ifdef MYSQL_DATA_TRUNCATED
	REGISTER_LONG_CONSTANT("MYSQLI_DATA_TRUNCATED", MYSQL_DATA_TRUNCATED, CONST_CS | CONST_PERSISTENT);
#endif

	/* reporting */
	REGISTER_LONG_CONSTANT("MYSQLI_REPORT_INDEX", MYSQLI_REPORT_INDEX, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_REPORT_ERROR", MYSQLI_REPORT_ERROR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_REPORT_STRICT", MYSQLI_REPORT_STRICT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_REPORT_ALL", MYSQLI_REPORT_ALL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_REPORT_OFF", 0, CONST_CS | CONST_PERSISTENT);

	/* We use non-nested macros with expansion, as VC has problems */
#ifdef MYSQLI_USE_MYSQLND
	REGISTER_LONG_CONSTANT("MYSQLI_DEBUG_TRACE_ENABLED", MYSQLND_DBG_ENABLED, CONST_CS | CONST_PERSISTENT);
#else
#ifdef DBUG_ON
	REGISTER_LONG_CONSTANT("MYSQLI_DEBUG_TRACE_ENABLED", 1, CONST_CS | CONST_PERSISTENT);
#else
	REGISTER_LONG_CONSTANT("MYSQLI_DEBUG_TRACE_ENABLED", 0, CONST_CS | CONST_PERSISTENT);
#endif
#endif

	REGISTER_LONG_CONSTANT("MYSQLI_SERVER_QUERY_NO_GOOD_INDEX_USED", SERVER_QUERY_NO_GOOD_INDEX_USED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_SERVER_QUERY_NO_INDEX_USED", SERVER_QUERY_NO_INDEX_USED, CONST_CS | CONST_PERSISTENT);
#ifdef SERVER_QUERY_WAS_SLOW
	REGISTER_LONG_CONSTANT("MYSQLI_SERVER_QUERY_WAS_SLOW", SERVER_QUERY_WAS_SLOW, CONST_CS | CONST_PERSISTENT);
#endif
#ifdef SERVER_PS_OUT_PARAMS
	REGISTER_LONG_CONSTANT("MYSQLI_SERVER_PS_OUT_PARAMS", SERVER_PS_OUT_PARAMS, CONST_CS | CONST_PERSISTENT);
#endif

	REGISTER_LONG_CONSTANT("MYSQLI_REFRESH_GRANT",      REFRESH_GRANT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_REFRESH_LOG",        REFRESH_LOG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_REFRESH_TABLES",     REFRESH_TABLES, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_REFRESH_HOSTS",      REFRESH_HOSTS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_REFRESH_STATUS",     REFRESH_STATUS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_REFRESH_THREADS",    REFRESH_THREADS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_REFRESH_SLAVE",      REFRESH_SLAVE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_REFRESH_MASTER",     REFRESH_MASTER, CONST_CS | CONST_PERSISTENT);
#ifdef REFRESH_BACKUP_LOG
	REGISTER_LONG_CONSTANT("MYSQLI_REFRESH_BACKUP_LOG", REFRESH_BACKUP_LOG, CONST_CS | CONST_PERSISTENT);
#endif

#if (MYSQL_VERSION_ID >= 50611 && defined(CLIENT_CAN_HANDLE_EXPIRED_PASSWORDS)) || defined(MYSQLI_USE_MYSQLND)
	REGISTER_LONG_CONSTANT("MYSQLI_OPT_CAN_HANDLE_EXPIRED_PASSWORDS", MYSQL_OPT_CAN_HANDLE_EXPIRED_PASSWORDS, CONST_CS | CONST_PERSISTENT);
#endif

	REGISTER_LONG_CONSTANT("MYSQLI_TRANS_START_WITH_CONSISTENT_SNAPSHOT", TRANS_START_WITH_CONSISTENT_SNAPSHOT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TRANS_START_READ_WRITE", TRANS_START_READ_WRITE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TRANS_START_READ_ONLY", TRANS_START_READ_ONLY, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("MYSQLI_TRANS_COR_AND_CHAIN", TRANS_COR_AND_CHAIN, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TRANS_COR_AND_NO_CHAIN", TRANS_COR_AND_NO_CHAIN, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TRANS_COR_RELEASE", TRANS_COR_RELEASE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MYSQLI_TRANS_COR_NO_RELEASE", TRANS_COR_NO_RELEASE, CONST_CS | CONST_PERSISTENT);


#ifdef MYSQLI_USE_MYSQLND
	mysqlnd_reverse_api_register_api(&mysqli_reverse_api TSRMLS_CC);
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(mysqli)
{
#ifndef MYSQLI_USE_MYSQLND
#if MYSQL_VERSION_ID >= 40000
#ifdef PHP_WIN32
	unsigned long client_ver = mysql_get_client_version();
	/*
	  Can't call mysql_server_end() multiple times prior to 5.0.46 on Windows.
	  PHP bug#41350 MySQL bug#25621
	*/
	if ((client_ver >= 50046 && client_ver < 50100) || client_ver > 50122) {
		mysql_server_end();
	}
#else
	mysql_server_end();
#endif
#endif
#endif

	zend_hash_destroy(&mysqli_driver_properties);
	zend_hash_destroy(&mysqli_result_properties);
	zend_hash_destroy(&mysqli_stmt_properties);
	zend_hash_destroy(&mysqli_warning_properties);
	zend_hash_destroy(&mysqli_link_properties);
	zend_hash_destroy(&classes);

	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(mysqli)
{
#if !defined(MYSQLI_USE_MYSQLND) && defined(ZTS) && MYSQL_VERSION_ID >= 40000
	if (mysql_thread_init()) {
		return FAILURE;
	}
#endif
	MyG(error_msg) = NULL;
	MyG(error_no) = 0;
	MyG(report_mode) = 0;

	return SUCCESS;
}
/* }}} */

#if defined(A0) && defined(MYSQLI_USE_MYSQLND)
static void php_mysqli_persistent_helper_for_every(void *p)
{
	TSRMLS_FETCH();
	mysqlnd_end_psession((MYSQLND *) p);
} /* }}} */


static int php_mysqli_persistent_helper_once(zend_rsrc_list_entry *le TSRMLS_DC)
{
	if (le->type == php_le_pmysqli()) {
		mysqli_plist_entry *plist = (mysqli_plist_entry *) le->ptr;
		zend_ptr_stack_apply(&plist->free_links, php_mysqli_persistent_helper_for_every);
	}
	return ZEND_HASH_APPLY_KEEP;
} /* }}} */
#endif


/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(mysqli)
{
	/* check persistent connections, move used to free */

#if !defined(MYSQLI_USE_MYSQLND) && defined(ZTS) && MYSQL_VERSION_ID >= 40000
	mysql_thread_end();
#endif
	if (MyG(error_msg)) {
		efree(MyG(error_msg));
	}
#if defined(A0) && defined(MYSQLI_USE_MYSQLND)
	/* psession is being called when the connection is freed - explicitly or implicitly */
	zend_hash_apply(&EG(persistent_list), (apply_func_t) php_mysqli_persistent_helper_once TSRMLS_CC);
#endif
	return SUCCESS;
}
/* }}} */


/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(mysqli)
{
	char buf[32];

	php_info_print_table_start();
	php_info_print_table_header(2, "MysqlI Support", "enabled");
	php_info_print_table_row(2, "Client API library version", mysql_get_client_info());
	snprintf(buf, sizeof(buf), "%ld", MyG(num_active_persistent));
	php_info_print_table_row(2, "Active Persistent Links", buf);
	snprintf(buf, sizeof(buf), "%ld", MyG(num_inactive_persistent));
	php_info_print_table_row(2, "Inactive Persistent Links", buf);
	snprintf(buf, sizeof(buf), "%ld", MyG(num_links));
	php_info_print_table_row(2, "Active Links", buf);
#if !defined(MYSQLI_USE_MYSQLND)
	php_info_print_table_row(2, "Client API header version", MYSQL_SERVER_VERSION);
	php_info_print_table_row(2, "MYSQLI_SOCKET", MYSQL_UNIX_ADDR);
#endif
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */


/* Dependancies */
static const  zend_module_dep mysqli_deps[] = {
#if defined(HAVE_SPL) && ((PHP_MAJOR_VERSION > 5) || (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 1))
	ZEND_MOD_REQUIRED("spl")
#endif
#if defined(MYSQLI_USE_MYSQLND)
	ZEND_MOD_REQUIRED("mysqlnd")
#endif
	ZEND_MOD_END
};

/* {{{ mysqli_module_entry
 */
zend_module_entry mysqli_module_entry = {
#if ZEND_MODULE_API_NO >= 20050922
	STANDARD_MODULE_HEADER_EX, NULL,
	mysqli_deps,
#elif ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"mysqli",
	mysqli_functions,
	PHP_MINIT(mysqli),
	PHP_MSHUTDOWN(mysqli),
	PHP_RINIT(mysqli),
	PHP_RSHUTDOWN(mysqli),
	PHP_MINFO(mysqli),
	"0.1", /* Replace with version number for your extension */
	PHP_MODULE_GLOBALS(mysqli),
	PHP_GINIT(mysqli),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_MYSQLI
ZEND_GET_MODULE(mysqli)
#endif


/* {{{ mixed mysqli_stmt_construct()
constructor for statement object.
Parameters:
  object -> mysqli_stmt_init
  object, query -> mysqli_prepare
*/
PHP_FUNCTION(mysqli_stmt_construct)
{
	MY_MYSQL			*mysql;
	zval				*mysql_link;
	MY_STMT				*stmt;
	MYSQLI_RESOURCE		*mysqli_resource;
	char				*statement;
	int					statement_len;

	switch (ZEND_NUM_ARGS())
	{
		case 1:  /* mysql_stmt_init */
			if (zend_parse_parameters(1 TSRMLS_CC, "O", &mysql_link, mysqli_link_class_entry)==FAILURE) {
				return;
			}
			MYSQLI_FETCH_RESOURCE_CONN(mysql, &mysql_link, MYSQLI_STATUS_VALID);

			stmt = (MY_STMT *)ecalloc(1,sizeof(MY_STMT));

			stmt->stmt = mysql_stmt_init(mysql->mysql);
		break;
		case 2:
			if (zend_parse_parameters(2 TSRMLS_CC, "Os", &mysql_link, mysqli_link_class_entry, &statement, &statement_len)==FAILURE) {
				return;
			}
			MYSQLI_FETCH_RESOURCE_CONN(mysql, &mysql_link, MYSQLI_STATUS_VALID);

			stmt = (MY_STMT *)ecalloc(1,sizeof(MY_STMT));

			if ((stmt->stmt = mysql_stmt_init(mysql->mysql))) {
				mysql_stmt_prepare(stmt->stmt, (char *)statement, statement_len);
			}
		break;
		default:
			WRONG_PARAM_COUNT;
		break;
	}

	if (!stmt->stmt) {
		efree(stmt);
		RETURN_FALSE;
	}
#ifndef MYSQLI_USE_MYSQLND
	stmt->link_handle = Z_OBJ_HANDLE(*mysql_link);
	zend_objects_store_add_ref_by_handle(stmt->link_handle TSRMLS_CC);
#endif

	mysqli_resource = (MYSQLI_RESOURCE *)ecalloc (1, sizeof(MYSQLI_RESOURCE));
	mysqli_resource->ptr = (void *)stmt;
	mysqli_resource->status = (ZEND_NUM_ARGS() == 1) ? MYSQLI_STATUS_INITIALIZED : MYSQLI_STATUS_VALID;

	((mysqli_object *) zend_object_store_get_object(getThis() TSRMLS_CC))->ptr = mysqli_resource;
}
/* }}} */

/* {{{ mixed mysqli_result_construct()
constructor for result object.
Parameters:
  object [, mode] -> mysqli_store/use_result
*/
PHP_FUNCTION(mysqli_result_construct)
{
	MY_MYSQL			*mysql;
	MYSQL_RES			*result = NULL;
	zval				*mysql_link;
	MYSQLI_RESOURCE		*mysqli_resource;
	long				resmode = MYSQLI_STORE_RESULT;

	switch (ZEND_NUM_ARGS()) {
		case 1:
			if (zend_parse_parameters(1 TSRMLS_CC, "O", &mysql_link, mysqli_link_class_entry)==FAILURE) {
				return;
			}
			break;
		case 2:
			if (zend_parse_parameters(2 TSRMLS_CC, "Ol", &mysql_link, mysqli_link_class_entry, &resmode)==FAILURE) {
				return;
			}
			break;
		default:
			WRONG_PARAM_COUNT;
	}

	MYSQLI_FETCH_RESOURCE_CONN(mysql, &mysql_link, MYSQLI_STATUS_VALID);

	switch (resmode) {
		case MYSQLI_STORE_RESULT:
			result = mysql_store_result(mysql->mysql);
			break;
		case MYSQLI_USE_RESULT:
			result = mysql_use_result(mysql->mysql);
			break;
		default:
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid value for resultmode");
	}

	if (!result) {
		RETURN_FALSE;
	}

	mysqli_resource = (MYSQLI_RESOURCE *)ecalloc (1, sizeof(MYSQLI_RESOURCE));
	mysqli_resource->ptr = (void *)result;
	mysqli_resource->status = MYSQLI_STATUS_VALID;

	((mysqli_object *) zend_object_store_get_object(getThis() TSRMLS_CC))->ptr = mysqli_resource;

}
/* }}} */


/* {{{ php_mysqli_fetch_into_hash_aux
 */
void php_mysqli_fetch_into_hash_aux(zval *return_value, MYSQL_RES * result, long fetchtype TSRMLS_DC)
{
#if !defined(MYSQLI_USE_MYSQLND)
	MYSQL_ROW row;
	unsigned int	i;
	MYSQL_FIELD		*fields;
	unsigned long	*field_len;
	
	if (!(row = mysql_fetch_row(result))) {
		RETURN_NULL();
	}

	if (fetchtype & MYSQLI_ASSOC) {
		fields = mysql_fetch_fields(result);
	}

	array_init(return_value);
	field_len = mysql_fetch_lengths(result);

	for (i = 0; i < mysql_num_fields(result); i++) {
		if (row[i]) {
			zval *res;

			MAKE_STD_ZVAL(res);

#if MYSQL_VERSION_ID > 50002
			if (mysql_fetch_field_direct(result, i)->type == MYSQL_TYPE_BIT) {
				my_ulonglong llval;
				char tmp[22];
				switch (field_len[i]) {
					case 8:llval = (my_ulonglong)  bit_uint8korr(row[i]);break;
					case 7:llval = (my_ulonglong)  bit_uint7korr(row[i]);break;
					case 6:llval = (my_ulonglong)  bit_uint6korr(row[i]);break;
					case 5:llval = (my_ulonglong)  bit_uint5korr(row[i]);break;
					case 4:llval = (my_ulonglong)  bit_uint4korr(row[i]);break;
					case 3:llval = (my_ulonglong)  bit_uint3korr(row[i]);break;
					case 2:llval = (my_ulonglong)  bit_uint2korr(row[i]);break;
					case 1:llval = (my_ulonglong)  uint1korr(row[i]);break;
				}
				/* even though lval is declared as unsigned, the value
				 * may be negative. Therefor we cannot use MYSQLI_LLU_SPEC and must
				 * use MYSQLI_LL_SPEC.
				 */
				snprintf(tmp, sizeof(tmp), (mysql_fetch_field_direct(result, i)->flags & UNSIGNED_FLAG)? MYSQLI_LLU_SPEC : MYSQLI_LL_SPEC, llval);
				ZVAL_STRING(res, tmp, 1);
			} else
#endif
			{

#if PHP_API_VERSION < 20100412
				/* check if we need magic quotes */
				if (PG(magic_quotes_runtime)) {
					Z_TYPE_P(res) = IS_STRING;
					Z_STRVAL_P(res) = php_addslashes(row[i], field_len[i], &Z_STRLEN_P(res), 0 TSRMLS_CC);
				} else {
#endif
					ZVAL_STRINGL(res, row[i], field_len[i], 1);
#if PHP_API_VERSION < 20100412
				}
#endif
			}

			if (fetchtype & MYSQLI_NUM) {
				add_index_zval(return_value, i, res);
			}
			if (fetchtype & MYSQLI_ASSOC) {
				if (fetchtype & MYSQLI_NUM) {
					Z_ADDREF_P(res);
				}
				add_assoc_zval(return_value, fields[i].name, res);
			}
		} else {
			if (fetchtype & MYSQLI_NUM) {
				add_index_null(return_value, i);
			}
			if (fetchtype & MYSQLI_ASSOC) {
				add_assoc_null(return_value, fields[i].name);
			}
		}
	}
#else
	mysqlnd_fetch_into(result, ((fetchtype & MYSQLI_NUM)? MYSQLND_FETCH_NUM:0) | ((fetchtype & MYSQLI_ASSOC)? MYSQLND_FETCH_ASSOC:0), return_value, MYSQLND_MYSQLI);
#endif
}
/* }}} */


/* {{{ php_mysqli_fetch_into_hash
 */
void php_mysqli_fetch_into_hash(INTERNAL_FUNCTION_PARAMETERS, int override_flags, int into_object)
{
	MYSQL_RES		*result;
	zval			*mysql_result;
	long			fetchtype;
	zval			*ctor_params = NULL;
	zend_class_entry *ce = NULL;

	if (into_object) {
		char *class_name;
		int class_name_len;

		if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O|sz", &mysql_result, mysqli_result_class_entry, &class_name, &class_name_len, &ctor_params) == FAILURE) {
			return;
		}
		if (ZEND_NUM_ARGS() < (getThis() ? 1 : 2)) {
			ce = zend_standard_class_def;
		} else {
			ce = zend_fetch_class(class_name, class_name_len, ZEND_FETCH_CLASS_AUTO TSRMLS_CC);
		}
		if (!ce) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not find class '%s'", class_name);
			return;
		}
		fetchtype = MYSQLI_ASSOC;
	} else {
		if (override_flags) {
			if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &mysql_result, mysqli_result_class_entry) == FAILURE) {
				return;
			}
			fetchtype = override_flags;
		} else {
			fetchtype = MYSQLI_BOTH;
			if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O|l", &mysql_result, mysqli_result_class_entry, &fetchtype) == FAILURE) {
				return;
			}
		}
	}
	MYSQLI_FETCH_RESOURCE(result, MYSQL_RES *, &mysql_result, "mysqli_result", MYSQLI_STATUS_VALID);

	if (fetchtype < MYSQLI_ASSOC || fetchtype > MYSQLI_BOTH) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "The result type should be either MYSQLI_NUM, MYSQLI_ASSOC or MYSQLI_BOTH");
		RETURN_FALSE;
	}

	php_mysqli_fetch_into_hash_aux(return_value, result, fetchtype TSRMLS_CC);

	if (into_object && Z_TYPE_P(return_value) == IS_ARRAY) {
		zval dataset = *return_value;
		zend_fcall_info fci;
		zend_fcall_info_cache fcc;
		zval *retval_ptr;

		object_and_properties_init(return_value, ce, NULL);
		zend_merge_properties(return_value, Z_ARRVAL(dataset), 1 TSRMLS_CC);

		if (ce->constructor) {
			fci.size = sizeof(fci);
			fci.function_table = &ce->function_table;
			fci.function_name = NULL;
			fci.symbol_table = NULL;
			fci.object_ptr = return_value;
			fci.retval_ptr_ptr = &retval_ptr;
			if (ctor_params && Z_TYPE_P(ctor_params) != IS_NULL) {
				if (Z_TYPE_P(ctor_params) == IS_ARRAY) {
					HashTable *params_ht = Z_ARRVAL_P(ctor_params);
					Bucket *p;

					fci.param_count = 0;
					fci.params = safe_emalloc(sizeof(zval*), params_ht->nNumOfElements, 0);
					p = params_ht->pListHead;
					while (p != NULL) {
						fci.params[fci.param_count++] = (zval**)p->pData;
						p = p->pListNext;
					}
				} else {
					/* Two problems why we throw exceptions here: PHP is typeless
					 * and hence passing one argument that's not an array could be
					 * by mistake and the other way round is possible, too. The
					 * single value is an array. Also we'd have to make that one
					 * argument passed by reference.
					 */
					zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Parameter ctor_params must be an array", 0 TSRMLS_CC);
					return;
				}
			} else {
				fci.param_count = 0;
				fci.params = NULL;
			}
			fci.no_separation = 1;

			fcc.initialized = 1;
			fcc.function_handler = ce->constructor;
			fcc.calling_scope = EG(scope);
			fcc.called_scope = Z_OBJCE_P(return_value);
			fcc.object_ptr = return_value;

			if (zend_call_function(&fci, &fcc TSRMLS_CC) == FAILURE) {
				zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C), 0 TSRMLS_CC, "Could not execute %s::%s()", ce->name, ce->constructor->common.function_name);
			} else {
				if (retval_ptr) {
					zval_ptr_dtor(&retval_ptr);
				}
			}
			if (fci.params) {
				efree(fci.params);
			}
		} else if (ctor_params) {
			zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C), 0 TSRMLS_CC, "Class %s does not have a constructor hence you cannot use ctor_params", ce->name);
		}
	}
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
