/*
  This file is part of, or distributed with, libXMLRPC - a C library for 
  xml-encoded function calls.

  Author: Dan Libby (dan@libby.com)
  Epinions.com may be contacted at feedback@epinions-inc.com
*/

/*  
  Copyright 2001 Epinions, Inc. 

  Subject to the following 3 conditions, Epinions, Inc.  permits you, free 
  of charge, to (a) use, copy, distribute, modify, perform and display this 
  software and associated documentation files (the "Software"), and (b) 
  permit others to whom the Software is furnished to do so as well.  

  1) The above copyright notice and this permission notice shall be included 
  without modification in all copies or substantial portions of the 
  Software.  

  2) THE SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY OR CONDITION OF 
  ANY KIND, EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION ANY 
  IMPLIED WARRANTIES OF ACCURACY, MERCHANTABILITY, FITNESS FOR A PARTICULAR 
  PURPOSE OR NONINFRINGEMENT.  

  3) IN NO EVENT SHALL EPINIONS, INC. BE LIABLE FOR ANY DIRECT, INDIRECT, 
  SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES OR LOST PROFITS ARISING OUT 
  OF OR IN CONNECTION WITH THE SOFTWARE (HOWEVER ARISING, INCLUDING 
  NEGLIGENCE), EVEN IF EPINIONS, INC.  IS AWARE OF THE POSSIBILITY OF SUCH 
  DAMAGES.    

*/

/* auto-generated portions of this file are also subject to the php license */

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
   | Author: Dan Libby                                                    |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

/**********************************************************************
* BUGS:                                                               *
*  - when calling a php user function, there appears to be no way to  *
*    distinguish between a return value of null, and no return value  *
*    at all.  The xml serialization layer(s) will then return a value *
*    of null, when the right thing may be no value at all. (SOAP)     *
**********************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "ext/date/php_date.h"
#include "php_ini.h"
#include "php_xmlrpc.h"
#include "xmlrpc.h"

#define PHP_EXT_VERSION "0.51"

static int le_xmlrpc_server;

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_xmlrpc_encode, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_xmlrpc_decode, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, encoding)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_xmlrpc_decode_request, 0, 0, 2)
	ZEND_ARG_INFO(0, xml)
	ZEND_ARG_INFO(1, method)
	ZEND_ARG_INFO(0, encoding)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_xmlrpc_encode_request, 0, 0, 2)
	ZEND_ARG_INFO(0, method)
	ZEND_ARG_INFO(0, params)
	ZEND_ARG_INFO(0, output_options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_xmlrpc_set_type, 0, 0, 2)
	ZEND_ARG_INFO(1, value)
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_xmlrpc_is_fault, 0, 0, 1)
	ZEND_ARG_INFO(0, arg)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_xmlrpc_server_create, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_xmlrpc_server_destroy, 0, 0, 1)
	ZEND_ARG_INFO(0, server)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_xmlrpc_server_register_method, 0, 0, 3)
	ZEND_ARG_INFO(0, server)
	ZEND_ARG_INFO(0, method_name)
	ZEND_ARG_INFO(0, function)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_xmlrpc_server_call_method, 0, 0, 3)
	ZEND_ARG_INFO(0, server)
	ZEND_ARG_INFO(0, xml)
	ZEND_ARG_INFO(0, user_data)
	ZEND_ARG_INFO(0, output_options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_xmlrpc_parse_method_descriptions, 0, 0, 1)
	ZEND_ARG_INFO(0, xml)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_xmlrpc_server_add_introspection_data, 0, 0, 2)
	ZEND_ARG_INFO(0, server)
	ZEND_ARG_INFO(0, desc)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_xmlrpc_server_register_introspection_callback, 0, 0, 2)
	ZEND_ARG_INFO(0, server)
	ZEND_ARG_INFO(0, function)
ZEND_END_ARG_INFO()
/* }}} */

const zend_function_entry xmlrpc_functions[] = {
	PHP_FE(xmlrpc_encode,									arginfo_xmlrpc_encode) 
	PHP_FE(xmlrpc_decode,									arginfo_xmlrpc_decode)
	PHP_FE(xmlrpc_decode_request,							arginfo_xmlrpc_decode_request)
	PHP_FE(xmlrpc_encode_request,							arginfo_xmlrpc_encode_request)
	PHP_FE(xmlrpc_get_type,									arginfo_xmlrpc_encode)
	PHP_FE(xmlrpc_set_type,									arginfo_xmlrpc_set_type)
	PHP_FE(xmlrpc_is_fault,									arginfo_xmlrpc_is_fault)
	PHP_FE(xmlrpc_server_create,							arginfo_xmlrpc_server_create)
	PHP_FE(xmlrpc_server_destroy,							arginfo_xmlrpc_server_destroy)
	PHP_FE(xmlrpc_server_register_method,					arginfo_xmlrpc_server_register_method)
	PHP_FE(xmlrpc_server_call_method,						arginfo_xmlrpc_server_call_method)
	PHP_FE(xmlrpc_parse_method_descriptions,				arginfo_xmlrpc_parse_method_descriptions)
	PHP_FE(xmlrpc_server_add_introspection_data,			arginfo_xmlrpc_server_add_introspection_data)
	PHP_FE(xmlrpc_server_register_introspection_callback,	arginfo_xmlrpc_server_register_introspection_callback)
	PHP_FE_END
};

zend_module_entry xmlrpc_module_entry = {
	STANDARD_MODULE_HEADER,
	"xmlrpc",
	xmlrpc_functions,
	PHP_MINIT(xmlrpc),
	NULL,
	NULL,
	NULL,
	PHP_MINFO(xmlrpc),
	PHP_EXT_VERSION,
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_XMLRPC
ZEND_GET_MODULE(xmlrpc)
#endif

/*******************************
* local structures and defines *
*******************************/

/* per server data */
typedef struct _xmlrpc_server_data {
	zval* method_map;
	zval* introspection_map;
	XMLRPC_SERVER server_ptr;
} xmlrpc_server_data;


/* how to format output */
typedef struct _php_output_options {
	int b_php_out;
	int b_auto_version;
	STRUCT_XMLRPC_REQUEST_OUTPUT_OPTIONS xmlrpc_out;
} php_output_options;

/* data passed to C callback */
typedef struct _xmlrpc_callback_data {
	zval* xmlrpc_method;
	zval* php_function;
	zval* caller_params;
	zval* return_data;
	xmlrpc_server_data* server;
	char php_executed;
} xmlrpc_callback_data;

/* output options */
#define OUTPUT_TYPE_KEY       "output_type"
#define OUTPUT_TYPE_KEY_LEN   (sizeof(OUTPUT_TYPE_KEY) - 1)
#define OUTPUT_TYPE_VALUE_PHP "php"
#define OUTPUT_TYPE_VALUE_XML "xml"

#define VERBOSITY_KEY                  "verbosity"
#define VERBOSITY_KEY_LEN              (sizeof(VERBOSITY_KEY) - 1)
#define VERBOSITY_VALUE_NO_WHITE_SPACE "no_white_space"
#define VERBOSITY_VALUE_NEWLINES_ONLY  "newlines_only"
#define VERBOSITY_VALUE_PRETTY         "pretty"

#define ESCAPING_KEY             "escaping"
#define ESCAPING_KEY_LEN         (sizeof(ESCAPING_KEY) - 1)
#define ESCAPING_VALUE_CDATA     "cdata"
#define ESCAPING_VALUE_NON_ASCII "non-ascii"
#define ESCAPING_VALUE_NON_PRINT "non-print"
#define ESCAPING_VALUE_MARKUP    "markup"

#define VERSION_KEY          "version"
#define VERSION_KEY_LEN      (sizeof(VERSION_KEY) - 1)
#define VERSION_VALUE_SIMPLE "simple"
#define VERSION_VALUE_XMLRPC "xmlrpc"
#define VERSION_VALUE_SOAP11 "soap 1.1"
#define VERSION_VALUE_AUTO   "auto"

#define ENCODING_KEY     "encoding"
#define ENCODING_KEY_LEN (sizeof(ENCODING_KEY) - 1)
#define ENCODING_DEFAULT "iso-8859-1"

/* value types */
#define OBJECT_TYPE_ATTR  "xmlrpc_type"
#define OBJECT_VALUE_ATTR "scalar"
#define OBJECT_VALUE_TS_ATTR "timestamp"

/* faults */
#define FAULT_CODE       "faultCode"
#define FAULT_CODE_LEN   (sizeof(FAULT_CODE) - 1)
#define FAULT_STRING     "faultString"
#define FAULT_STRING_LEN (sizeof(FAULT_STRING) - 1)

/***********************
* forward declarations *
***********************/
XMLRPC_VALUE_TYPE get_zval_xmlrpc_type(zval* value, zval** newvalue);
static void php_xmlrpc_introspection_callback(XMLRPC_SERVER server, void* data);
int sset_zval_xmlrpc_type(zval* value, XMLRPC_VALUE_TYPE type);
zval* decode_request_worker(char *xml_in, int xml_in_len, char *encoding_in, zval* method_name_out);
const char* xmlrpc_type_as_str(XMLRPC_VALUE_TYPE type, XMLRPC_VECTOR_TYPE vtype);
XMLRPC_VALUE_TYPE xmlrpc_str_as_type(const char* str);
XMLRPC_VECTOR_TYPE xmlrpc_str_as_vector_type(const char* str);
int set_zval_xmlrpc_type(zval* value, XMLRPC_VALUE_TYPE type);

/*********************
* startup / shutdown *
*********************/

static void destroy_server_data(xmlrpc_server_data *server TSRMLS_DC)
{
	if (server) {
		XMLRPC_ServerDestroy(server->server_ptr);

		zval_dtor(server->method_map);
		FREE_ZVAL(server->method_map);

		zval_dtor(server->introspection_map);
		FREE_ZVAL(server->introspection_map);

		efree(server);
	}
}

/* called when server is being destructed. either when xmlrpc_server_destroy
 * is called, or when request ends.  */
static void xmlrpc_server_destructor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	if (rsrc && rsrc->ptr) {
		destroy_server_data((xmlrpc_server_data*) rsrc->ptr TSRMLS_CC);
	}
}

/* module init */
PHP_MINIT_FUNCTION(xmlrpc)
{
	le_xmlrpc_server = zend_register_list_destructors_ex(xmlrpc_server_destructor, NULL, "xmlrpc server", module_number);

	return SUCCESS;
}

/* display info in phpinfo() */
PHP_MINFO_FUNCTION(xmlrpc)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "core library version", XMLRPC_GetVersionString());
	php_info_print_table_row(2, "php extension version", PHP_EXT_VERSION);
	php_info_print_table_row(2, "author", "Dan Libby");
	php_info_print_table_row(2, "homepage", "http://xmlrpc-epi.sourceforge.net");
	php_info_print_table_row(2, "open sourced by", "Epinions.com");
	php_info_print_table_end();
}

/*******************
* random utilities *
*******************/

/* Utility functions for adding data types to arrays, with or without key (assoc, non-assoc).
 * Could easily be further generalized to work with objects.
 */
#if 0
static int add_long(zval* list, char* id, int num) {
	if(id) return add_assoc_long(list, id, num);
	else   return add_next_index_long(list, num);
}

static int add_double(zval* list, char* id, double num) {
	if(id) return add_assoc_double(list, id, num);
	else   return add_next_index_double(list, num);
}

static int add_string(zval* list, char* id, char* string, int duplicate) {
	if(id) return add_assoc_string(list, id, string, duplicate);
	else   return add_next_index_string(list, string, duplicate);
}

static int add_stringl(zval* list, char* id, char* string, uint length, int duplicate) {
	if(id) return add_assoc_stringl(list, id, string, length, duplicate);
	else   return add_next_index_stringl(list, string, length, duplicate);
}

#endif

static int add_zval(zval* list, const char* id, zval** val)
{
	if (list && val) {
		if (id) {
			int id_len = strlen(id);
			if (!(id_len > 1 && id[0] == '0') && is_numeric_string((char *)id, id_len, NULL, NULL, 0) == IS_LONG) {
				long index = strtol(id, NULL, 0);
				return zend_hash_index_update(Z_ARRVAL_P(list), index, (void *) val, sizeof(zval **), NULL);
			} else {
				return zend_hash_update(Z_ARRVAL_P(list), (char*) id, strlen(id) + 1, (void *) val, sizeof(zval **), NULL);
			}
		} else {
			return zend_hash_next_index_insert(Z_ARRVAL_P(list), (void *) val, sizeof(zval **), NULL); 
		}
	}
	/* what is the correct return on error? */
	return 0;
}

#define my_zend_hash_get_current_key(ht, my_key, num_index) zend_hash_get_current_key(ht, my_key, num_index, 0)


/*************************
* input / output options *
*************************/

/* parse an array (user input) into output options suitable for use by xmlrpc engine
 * and determine whether to return data as xml or php vars */
static void set_output_options(php_output_options* options, zval* output_opts)
{
	if (options) {

		/* defaults */
		options->b_php_out = 0;
		options->b_auto_version = 1;
		options->xmlrpc_out.version = xmlrpc_version_1_0;
		options->xmlrpc_out.xml_elem_opts.encoding = ENCODING_DEFAULT;
		options->xmlrpc_out.xml_elem_opts.verbosity = xml_elem_pretty;
		options->xmlrpc_out.xml_elem_opts.escaping = xml_elem_markup_escaping | xml_elem_non_ascii_escaping | xml_elem_non_print_escaping;

		if (output_opts && Z_TYPE_P(output_opts) == IS_ARRAY) {
			zval** val;

			/* type of output (xml/php) */
			if (zend_hash_find(Z_ARRVAL_P(output_opts), OUTPUT_TYPE_KEY, OUTPUT_TYPE_KEY_LEN + 1, (void**) &val) == SUCCESS) {
				if (Z_TYPE_PP(val) == IS_STRING) {
					if (!strcmp(Z_STRVAL_PP(val), OUTPUT_TYPE_VALUE_PHP)) {
						options->b_php_out = 1;
					} else if (!strcmp(Z_STRVAL_PP(val), OUTPUT_TYPE_VALUE_XML)) {
						options->b_php_out = 0;
					}
				}
			}

			/* verbosity of generated xml */
			if (zend_hash_find(Z_ARRVAL_P(output_opts), VERBOSITY_KEY, VERBOSITY_KEY_LEN + 1, (void**) &val) == SUCCESS) {
				if (Z_TYPE_PP(val) == IS_STRING) {
					if (!strcmp(Z_STRVAL_PP(val), VERBOSITY_VALUE_NO_WHITE_SPACE)) {
						options->xmlrpc_out.xml_elem_opts.verbosity = xml_elem_no_white_space;
					} else if (!strcmp(Z_STRVAL_PP(val), VERBOSITY_VALUE_NEWLINES_ONLY)) {
						options->xmlrpc_out.xml_elem_opts.verbosity = xml_elem_newlines_only;
					} else if (!strcmp(Z_STRVAL_PP(val), VERBOSITY_VALUE_PRETTY)) {
						options->xmlrpc_out.xml_elem_opts.verbosity = xml_elem_pretty;
					}
				}
			}

			/* version of xml to output */
			if (zend_hash_find(Z_ARRVAL_P(output_opts), VERSION_KEY, VERSION_KEY_LEN + 1, (void**) &val) == SUCCESS) {
				if (Z_TYPE_PP(val) == IS_STRING) {
					options->b_auto_version = 0;
					if (!strcmp(Z_STRVAL_PP(val), VERSION_VALUE_XMLRPC)) {
						options->xmlrpc_out.version = xmlrpc_version_1_0;
					} else if (!strcmp(Z_STRVAL_PP(val), VERSION_VALUE_SIMPLE)) {
						options->xmlrpc_out.version = xmlrpc_version_simple;
					} else if (!strcmp((*val)->value.str.val, VERSION_VALUE_SOAP11)) {
							options->xmlrpc_out.version = xmlrpc_version_soap_1_1;
					} else { /* if(!strcmp((*val)->value.str.val, VERSION_VALUE_AUTO)) { */
							options->b_auto_version = 1;
					}
				}
			}

			/* encoding code set */
			if (zend_hash_find(Z_ARRVAL_P(output_opts), ENCODING_KEY, ENCODING_KEY_LEN + 1, (void**)&val) == SUCCESS) {
				if (Z_TYPE_PP(val) == IS_STRING) {
					options->xmlrpc_out.xml_elem_opts.encoding = estrdup(Z_STRVAL_PP(val));
				}
			}

			/* escaping options */
			if (zend_hash_find(Z_ARRVAL_P(output_opts), ESCAPING_KEY, ESCAPING_KEY_LEN + 1, (void**)&val) == SUCCESS) {
				/* multiple values allowed.  check if array */
				if (Z_TYPE_PP(val) == IS_ARRAY) {
					zval** iter_val;

					zend_hash_internal_pointer_reset(Z_ARRVAL_PP(val));
					options->xmlrpc_out.xml_elem_opts.escaping = xml_elem_no_escaping;

					while (1) {
						if (zend_hash_get_current_data(Z_ARRVAL_PP(val), (void**)&iter_val) == SUCCESS) {
							if (Z_TYPE_PP(iter_val) == IS_STRING && Z_STRVAL_PP(iter_val)) {
								if (!strcmp(Z_STRVAL_PP(iter_val), ESCAPING_VALUE_CDATA)) {
									options->xmlrpc_out.xml_elem_opts.escaping |= xml_elem_cdata_escaping;
								} else if (!strcmp(Z_STRVAL_PP(iter_val), ESCAPING_VALUE_NON_ASCII)) {
									options->xmlrpc_out.xml_elem_opts.escaping |= xml_elem_non_ascii_escaping;
								} else if (!strcmp(Z_STRVAL_PP(iter_val), ESCAPING_VALUE_NON_PRINT)) {
									options->xmlrpc_out.xml_elem_opts.escaping |= xml_elem_non_print_escaping;
								} else if (!strcmp(Z_STRVAL_PP(iter_val), ESCAPING_VALUE_MARKUP)) {
									options->xmlrpc_out.xml_elem_opts.escaping |= xml_elem_markup_escaping;
								}
							}
						} else {
							break;
						}
						zend_hash_move_forward(Z_ARRVAL_PP(val));
					}
				/* else, check for single value */
				} else if (Z_TYPE_PP(val) == IS_STRING) {
					if (!strcmp(Z_STRVAL_PP(val), ESCAPING_VALUE_CDATA)) {
						options->xmlrpc_out.xml_elem_opts.escaping = xml_elem_cdata_escaping;
					} else if (!strcmp(Z_STRVAL_PP(val), ESCAPING_VALUE_NON_ASCII)) {
						options->xmlrpc_out.xml_elem_opts.escaping = xml_elem_non_ascii_escaping;
					} else if (!strcmp(Z_STRVAL_PP(val), ESCAPING_VALUE_NON_PRINT)) {
						options->xmlrpc_out.xml_elem_opts.escaping = xml_elem_non_print_escaping;
					} else if (!strcmp(Z_STRVAL_PP(val), ESCAPING_VALUE_MARKUP)) {
						options->xmlrpc_out.xml_elem_opts.escaping = xml_elem_markup_escaping;
					}
				}
			}
		}
	}
}


/******************
* encode / decode *
******************/

/* php arrays have no distinction between array and struct types.
 * they even allow mixed.  Thus, we determine the type by iterating
 * through the entire array and figuring out each element.
 * room for some optimation here if we stop after a specific # of elements.
 */
static XMLRPC_VECTOR_TYPE determine_vector_type (HashTable *ht)
{
	int bArray = 0, bStruct = 0, bMixed = 0;
	unsigned long num_index, last_num = 0;
	char* my_key;

	zend_hash_internal_pointer_reset(ht);
	while (1) {
		int res = my_zend_hash_get_current_key(ht, &my_key, &num_index);
		
		if (res == HASH_KEY_IS_LONG) {
			if (bStruct) {
				bMixed = 1;
				break;
			} else if (last_num > 0 && last_num != num_index-1) {
				bStruct = 1;
				break;
			}
			bArray = 1;
			last_num = num_index;
		} else if (res == HASH_KEY_NON_EXISTENT) {
			break;
		} else if (res == HASH_KEY_IS_STRING) {
			if (bArray) {
				bMixed = 1;
				break;
			}
			bStruct = 1;
		}
		zend_hash_move_forward(ht);
	}
	return bMixed ? xmlrpc_vector_mixed : (bStruct ? xmlrpc_vector_struct : xmlrpc_vector_array);
}

/* recursively convert php values into xmlrpc values */
static XMLRPC_VALUE PHP_to_XMLRPC_worker (const char* key, zval* in_val, int depth TSRMLS_DC)
{
	XMLRPC_VALUE xReturn = NULL;

	if (in_val) {
		zval* val = NULL;
		XMLRPC_VALUE_TYPE type = get_zval_xmlrpc_type(in_val, &val);
	
		if (val) {
			switch (type) {
				case xmlrpc_base64:
					if (Z_TYPE_P(val) == IS_NULL) {
						xReturn = XMLRPC_CreateValueEmpty();
						XMLRPC_SetValueID(xReturn, key, 0);
					} else {
						xReturn = XMLRPC_CreateValueBase64(key, Z_STRVAL_P(val), Z_STRLEN_P(val));
					}
					break;
				case xmlrpc_datetime:
					convert_to_string(val);
					xReturn = XMLRPC_CreateValueDateTime_ISO8601(key, Z_STRVAL_P(val));
					break;
				case xmlrpc_boolean:
					convert_to_boolean(val);
					xReturn = XMLRPC_CreateValueBoolean(key, Z_LVAL_P(val));
					break;
				case xmlrpc_int:
					convert_to_long(val);
					xReturn = XMLRPC_CreateValueInt(key, Z_LVAL_P(val));
					break;
				case xmlrpc_double:
					convert_to_double(val);
					xReturn = XMLRPC_CreateValueDouble(key, Z_DVAL_P(val));
					break;
				case xmlrpc_string:
					convert_to_string(val);
					xReturn = XMLRPC_CreateValueString(key, Z_STRVAL_P(val), Z_STRLEN_P(val));
					break;
				case xmlrpc_vector:
					{
						unsigned long num_index;
						zval** pIter;
						char* my_key;
						HashTable *ht = NULL;
						zval *val_arr;
						XMLRPC_VECTOR_TYPE vtype;

						ht = HASH_OF(val);
						if (ht && ht->nApplyCount > 1) {
							php_error_docref(NULL TSRMLS_CC, E_ERROR, "XML-RPC doesn't support circular references");
							return NULL;
						}

						MAKE_STD_ZVAL(val_arr);
						MAKE_COPY_ZVAL(&val, val_arr);
						convert_to_array(val_arr);
						
						vtype = determine_vector_type(Z_ARRVAL_P(val_arr));
						xReturn = XMLRPC_CreateVector(key, vtype);

						zend_hash_internal_pointer_reset(Z_ARRVAL_P(val_arr));
						while(zend_hash_get_current_data(Z_ARRVAL_P(val_arr), (void**)&pIter) == SUCCESS) {
							int res = my_zend_hash_get_current_key(Z_ARRVAL_P(val_arr), &my_key, &num_index);

							switch (res) {
								case HASH_KEY_NON_EXISTENT:
									break;
								case HASH_KEY_IS_STRING:
								case HASH_KEY_IS_LONG:
									ht = HASH_OF(*pIter);
									if (ht) {
										ht->nApplyCount++;
									}
									if (res == HASH_KEY_IS_LONG) {
										char *num_str = NULL;
										
										if (vtype != xmlrpc_vector_array) {
											spprintf(&num_str, 0, "%ld", num_index);
										}
										XMLRPC_AddValueToVector(xReturn, PHP_to_XMLRPC_worker(num_str, *pIter, depth++ TSRMLS_CC));
										if (num_str) {
											efree(num_str);
										}
									} else {
										XMLRPC_AddValueToVector(xReturn, PHP_to_XMLRPC_worker(my_key, *pIter, depth++ TSRMLS_CC));
									}
									if (ht) {
										ht->nApplyCount--;
									}
									break;
							}
							zend_hash_move_forward(Z_ARRVAL_P(val_arr));
						}	
						zval_ptr_dtor(&val_arr);
					}
					break;
				default:
					break;
			}
		}
	}
	return xReturn;
}

static XMLRPC_VALUE PHP_to_XMLRPC(zval* root_val TSRMLS_DC)
{
	return PHP_to_XMLRPC_worker(NULL, root_val, 0 TSRMLS_CC);
}

/* recursively convert xmlrpc values into php values */
static zval* XMLRPC_to_PHP(XMLRPC_VALUE el)
{
	zval* elem = NULL;
	const char* pStr;

	if (el) {
		XMLRPC_VALUE_TYPE type = XMLRPC_GetValueType(el);

		MAKE_STD_ZVAL(elem); /* init. very important.  spent a frustrating day finding this out. */

		switch(type) {
			case xmlrpc_empty:
				Z_TYPE_P(elem) = IS_NULL;
				break;
			case xmlrpc_string:
				pStr = XMLRPC_GetValueString(el);
				if (pStr) {
					Z_STRLEN_P(elem) = XMLRPC_GetValueStringLen(el);
					Z_STRVAL_P(elem) = estrndup(pStr, Z_STRLEN_P(elem));
					Z_TYPE_P(elem) = IS_STRING;
				}
				break;
			case xmlrpc_int:
				Z_LVAL_P(elem) = XMLRPC_GetValueInt(el);
				Z_TYPE_P(elem) = IS_LONG;
				break;
			case xmlrpc_boolean:
				Z_LVAL_P(elem) = XMLRPC_GetValueBoolean(el);
				Z_TYPE_P(elem) = IS_BOOL;
				break;
			case xmlrpc_double:
				Z_DVAL_P(elem) = XMLRPC_GetValueDouble(el);
				Z_TYPE_P(elem) = IS_DOUBLE;
				break;
			case xmlrpc_datetime:
				Z_STRLEN_P(elem) = XMLRPC_GetValueStringLen(el);
				Z_STRVAL_P(elem) = estrndup(XMLRPC_GetValueDateTime_ISO8601(el), Z_STRLEN_P(elem));
				Z_TYPE_P(elem) = IS_STRING;
				break;
			case xmlrpc_base64:
				pStr = XMLRPC_GetValueBase64(el);
				if (pStr) {
					Z_STRLEN_P(elem) = XMLRPC_GetValueStringLen(el);
					Z_STRVAL_P(elem) = estrndup(pStr, Z_STRLEN_P(elem));
					Z_TYPE_P(elem) = IS_STRING;
				}
				break;
			case xmlrpc_vector:
				array_init(elem);
				{
					XMLRPC_VALUE xIter = XMLRPC_VectorRewind(el);

					while( xIter ) {
						zval *val = XMLRPC_to_PHP(xIter);
						if (val) {
							add_zval(elem, XMLRPC_GetValueID(xIter), &val);
						}
						xIter = XMLRPC_VectorNext(el);
					}
				}
				break;
			default:
				break;
		}
		set_zval_xmlrpc_type(elem, type);
	}
	return elem;
}

/* {{{ proto string xmlrpc_encode_request(string method, mixed params [, array output_options])
   Generates XML for a method request */
PHP_FUNCTION(xmlrpc_encode_request)
{
	XMLRPC_REQUEST xRequest = NULL;
	char *outBuf;
	zval *vals, *out_opts = NULL;
	char *method = NULL;
	int method_len;
	php_output_options out;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s!z|a", &method, &method_len, &vals, &out_opts) == FAILURE) {
		return;
	}

	set_output_options(&out, out_opts ? out_opts : 0);

	if (return_value_used) {
		xRequest = XMLRPC_RequestNew();

		if (xRequest) {
			XMLRPC_RequestSetOutputOptions(xRequest, &out.xmlrpc_out);
			if (method == NULL) {
				XMLRPC_RequestSetRequestType(xRequest, xmlrpc_request_response);
			} else {
				XMLRPC_RequestSetMethodName(xRequest, method);
				XMLRPC_RequestSetRequestType(xRequest, xmlrpc_request_call);
			}
			if (Z_TYPE_P(vals) != IS_NULL) {
				XMLRPC_RequestSetData(xRequest, PHP_to_XMLRPC(vals TSRMLS_CC));
			}

			outBuf = XMLRPC_REQUEST_ToXML(xRequest, 0);
			if (outBuf) {
				RETVAL_STRING(outBuf, 1);
				free(outBuf);
			}
			XMLRPC_RequestFree(xRequest, 1);
		}
	}
	
	if (strcmp(out.xmlrpc_out.xml_elem_opts.encoding, ENCODING_DEFAULT) != 0) {
		efree((char *)out.xmlrpc_out.xml_elem_opts.encoding);
	}
}
/* }}} */

/* {{{ proto string xmlrpc_encode(mixed value)
   Generates XML for a PHP value */
PHP_FUNCTION(xmlrpc_encode)
{
	XMLRPC_VALUE xOut = NULL;
	zval **arg1;
	char *outBuf;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Z", &arg1) == FAILURE) {
		return;
	}

	if (return_value_used) {
		/* convert native php type to xmlrpc type */
		xOut = PHP_to_XMLRPC(*arg1 TSRMLS_CC);

		/* generate raw xml from xmlrpc data */
		outBuf = XMLRPC_VALUE_ToXML(xOut, 0);

		if (xOut) {
			if (outBuf) {
				RETVAL_STRING(outBuf, 1);
				free(outBuf);
			}
			/* cleanup */
			XMLRPC_CleanupValue(xOut);
		}
	}
}
/* }}} */

zval* decode_request_worker(char *xml_in, int xml_in_len, char *encoding_in, zval* method_name_out) /* {{{ */
{
	zval* retval = NULL;
	XMLRPC_REQUEST response;
	STRUCT_XMLRPC_REQUEST_INPUT_OPTIONS opts = {{0}};
	const char *method_name;
	opts.xml_elem_opts.encoding = encoding_in ? utf8_get_encoding_id_from_string(encoding_in) : ENCODING_DEFAULT;

	/* generate XMLRPC_REQUEST from raw xml */
	response = XMLRPC_REQUEST_FromXML(xml_in, xml_in_len, &opts);
	if (response) {
		/* convert xmlrpc data to native php types */
		retval = XMLRPC_to_PHP(XMLRPC_RequestGetData(response));

		if (XMLRPC_RequestGetRequestType(response) == xmlrpc_request_call) {
			if (method_name_out) {
				method_name = XMLRPC_RequestGetMethodName(response);
				if (method_name) {
					zval_dtor(method_name_out);
					Z_TYPE_P(method_name_out) = IS_STRING;
					Z_STRVAL_P(method_name_out) = estrdup(method_name);
					Z_STRLEN_P(method_name_out) = strlen(Z_STRVAL_P(method_name_out));
				} else if (retval) {
					zval_ptr_dtor(&retval);
					retval = NULL;
				}
			}
		}

		/* dust, sweep, and mop */
		XMLRPC_RequestFree(response, 1);
	}
	return retval;
}
/* }}} */

/* {{{ proto array xmlrpc_decode_request(string xml, string& method [, string encoding])
   Decodes XML into native PHP types */
PHP_FUNCTION(xmlrpc_decode_request)
{
	char *xml, *encoding = NULL;
	zval **method;
	int xml_len, encoding_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sZ|s", &xml, &xml_len, &method, &encoding, &encoding_len) == FAILURE) {
		return;
	}


	if (return_value_used) {
		zval* retval = decode_request_worker(xml, xml_len, encoding_len ? encoding : NULL, *method);
		if (retval) {
			*return_value = *retval;
			FREE_ZVAL(retval);
		}
	}
}
/* }}} */

/* {{{ proto array xmlrpc_decode(string xml [, string encoding])
   Decodes XML into native PHP types */
PHP_FUNCTION(xmlrpc_decode)
{
	char *arg1, *arg2 = NULL;
	int arg1_len, arg2_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &arg1, &arg1_len, &arg2, &arg2_len) == FAILURE) {
		return;
	}

	if (return_value_used) {
		zval* retval = decode_request_worker(arg1, arg1_len, arg2_len ? arg2 : NULL, NULL);
		if (retval) {
			*return_value = *retval;
			FREE_ZVAL(retval);
		}
	}
}
/* }}} */

/*************************
* server related methods *
*************************/

/* {{{ proto resource xmlrpc_server_create(void)
   Creates an xmlrpc server */
PHP_FUNCTION(xmlrpc_server_create)
{
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	if (return_value_used) {
		zval *method_map, *introspection_map;
		xmlrpc_server_data *server = emalloc(sizeof(xmlrpc_server_data));
		MAKE_STD_ZVAL(method_map);
		MAKE_STD_ZVAL(introspection_map);
		
		array_init(method_map);
		array_init(introspection_map);
		
		/* allocate server data.  free'd in destroy_server_data() */
		server->method_map = method_map;
		server->introspection_map = introspection_map;
		server->server_ptr = XMLRPC_ServerCreate();

		XMLRPC_ServerRegisterIntrospectionCallback(server->server_ptr, php_xmlrpc_introspection_callback);

		/* store for later use */
		ZEND_REGISTER_RESOURCE(return_value,server, le_xmlrpc_server);
	}
}
/* }}} */

/* {{{ proto int xmlrpc_server_destroy(resource server)
   Destroys server resources */
PHP_FUNCTION(xmlrpc_server_destroy)
{
	zval *arg1;
	int bSuccess = FAILURE, type;
	xmlrpc_server_data *server;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &arg1) == FAILURE) {
		return;
	}

	server = zend_list_find(Z_LVAL_P(arg1), &type);

	if (server && type == le_xmlrpc_server) {
		bSuccess = zend_list_delete(Z_LVAL_P(arg1));

		/* called by hashtable destructor
		 * destroy_server_data(server);
		 */
	}
	RETVAL_LONG(bSuccess == SUCCESS);
}
/* }}} */
           
/* called by xmlrpc C engine as method handler for all registered methods.
 * it then calls the corresponding PHP function to handle the method.
 */
static XMLRPC_VALUE php_xmlrpc_callback(XMLRPC_SERVER server, XMLRPC_REQUEST xRequest, void* data) /* {{{ */
{
	xmlrpc_callback_data* pData = (xmlrpc_callback_data*)data;
	zval** php_function;
	zval* xmlrpc_params;
	zval* callback_params[3];
	TSRMLS_FETCH();

	zval_dtor(pData->xmlrpc_method);
	zval_dtor(pData->return_data);

	/* convert xmlrpc to native php types */
	ZVAL_STRING(pData->xmlrpc_method, XMLRPC_RequestGetMethodName(xRequest), 1);
	xmlrpc_params = XMLRPC_to_PHP(XMLRPC_RequestGetData(xRequest));
	
	/* check if the called method has been previous registered */
	if(zend_hash_find(Z_ARRVAL_P(pData->server->method_map),
                      Z_STRVAL_P(pData->xmlrpc_method), 
                      Z_STRLEN_P(pData->xmlrpc_method) + 1, 
                      (void**)&php_function) == SUCCESS) {

		pData->php_function = *php_function;
	}

	/* setup data hoojum */
	callback_params[0] = pData->xmlrpc_method;
	callback_params[1] = xmlrpc_params;
	callback_params[2] = pData->caller_params;

	/* Use same C function for all methods */

	/* php func prototype: function user_func($method_name, $xmlrpc_params, $user_params) */
	call_user_function(CG(function_table), NULL, pData->php_function, pData->return_data, 3, callback_params TSRMLS_CC);

	pData->php_executed = 1;

	zval_ptr_dtor(&xmlrpc_params);

	return PHP_to_XMLRPC(pData->return_data TSRMLS_CC);
}
/* }}} */

/* called by the C server when it first receives an introspection request.  We pass this on to
 * our PHP listeners, if any
 */
static void php_xmlrpc_introspection_callback(XMLRPC_SERVER server, void* data) /* {{{ */
{
	zval retval, **php_function;
	zval *callback_params[1];
	char *php_function_name;
	xmlrpc_callback_data* pData = (xmlrpc_callback_data*)data;
	TSRMLS_FETCH();

	/* setup data hoojum */
	callback_params[0] = pData->caller_params;

	/* loop through and call all registered callbacks */
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(pData->server->introspection_map));
	while (1) {
		if (zend_hash_get_current_data(Z_ARRVAL_P(pData->server->introspection_map), (void**)&php_function) == SUCCESS) {
			if (zend_is_callable(*php_function, 0, &php_function_name TSRMLS_CC)) {
				/* php func prototype: function string user_func($user_params) */
				if (call_user_function(CG(function_table), NULL, *php_function, &retval, 1, callback_params TSRMLS_CC) == SUCCESS) {
					XMLRPC_VALUE xData;
					STRUCT_XMLRPC_ERROR err = {0};

					/* return value should be a string */
					convert_to_string(&retval);

					xData = XMLRPC_IntrospectionCreateDescription(Z_STRVAL(retval), &err);

					if (xData) {
						if (!XMLRPC_ServerAddIntrospectionData(server, xData)) {
							php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to add introspection data returned from %s(), improper element structure", php_function_name);
						}
						XMLRPC_CleanupValue(xData);
					} else {
						/* could not create description */
						if (err.xml_elem_error.parser_code) {
							php_error_docref(NULL TSRMLS_CC, E_WARNING, "xml parse error: [line %ld, column %ld, message: %s] Unable to add introspection data returned from %s()", 
								err.xml_elem_error.column, err.xml_elem_error.line, err.xml_elem_error.parser_error, php_function_name);
						} else {
							php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to add introspection data returned from %s()", php_function_name);
						}
					}
					zval_dtor(&retval);
				} else {
					/* user func failed */
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Error calling user introspection callback: %s()", php_function_name);
				}
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid callback '%s' passed", php_function_name);
			}
			efree(php_function_name);
		} else {
			break;
		}
		zend_hash_move_forward(Z_ARRVAL_P(pData->server->introspection_map));
	}
	
	/* so we don't call the same callbacks ever again */
	zend_hash_clean(Z_ARRVAL_P(pData->server->introspection_map));
}
/* }}} */

/* {{{ proto bool xmlrpc_server_register_method(resource server, string method_name, string function)
   Register a PHP function to handle method matching method_name */
PHP_FUNCTION(xmlrpc_server_register_method)
{
	char *method_key;
	int method_key_len;
	zval *handle, *method_name_save, **method_name;
	int type;
	xmlrpc_server_data* server;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsZ", &handle, &method_key, &method_key_len, &method_name) == FAILURE) {
		return;
	}

	server = zend_list_find(Z_LVAL_P(handle), &type);

	if (type == le_xmlrpc_server) {
		/* register with C engine. every method just calls our standard callback, 
		 * and it then dispatches to php as necessary
		 */
		if (XMLRPC_ServerRegisterMethod(server->server_ptr, method_key, php_xmlrpc_callback)) {
			/* save for later use */
			ALLOC_ZVAL(method_name_save);
			MAKE_COPY_ZVAL(method_name, method_name_save);

			/* register our php method */
			add_zval(server->method_map, method_key, &method_name_save);

			RETURN_BOOL(1);
		}
	}
	RETURN_BOOL(0);
}
/* }}} */

/* {{{ proto bool xmlrpc_server_register_introspection_callback(resource server, string function)
   Register a PHP function to generate documentation */
PHP_FUNCTION(xmlrpc_server_register_introspection_callback)
{
	zval **method_name, *handle, *method_name_save;
	int type;
	xmlrpc_server_data* server;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rZ", &handle, &method_name) == FAILURE) {
		return;
	}

	server = zend_list_find(Z_LVAL_P(handle), &type);

	if (type == le_xmlrpc_server) {
		/* save for later use */
		ALLOC_ZVAL(method_name_save);
		MAKE_COPY_ZVAL(method_name, method_name_save);

		/* register our php method */
		add_zval(server->introspection_map, NULL, &method_name_save);

		RETURN_BOOL(1);
	}
	RETURN_BOOL(0);
}
/* }}} */

/* this function is itchin for a re-write */

/* {{{ proto mixed xmlrpc_server_call_method(resource server, string xml, mixed user_data [, array output_options])
   Parses XML requests and call methods */
PHP_FUNCTION(xmlrpc_server_call_method)
{
	xmlrpc_callback_data data = {0};
	XMLRPC_REQUEST xRequest;
	STRUCT_XMLRPC_REQUEST_INPUT_OPTIONS input_opts;
	xmlrpc_server_data* server;
	zval **caller_params, *handle, *output_opts = NULL;
	char *rawxml;
	int rawxml_len, type;
	php_output_options out;
	int argc =ZEND_NUM_ARGS();
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsZ|a", &handle, &rawxml, &rawxml_len, &caller_params, &output_opts) != SUCCESS) {
		return;
	}
	/* user output options */
	if (argc == 3) {
		set_output_options(&out, NULL);
	} else {
		set_output_options(&out, output_opts);
	}

	server = zend_list_find(Z_LVAL_P(handle), &type);

	if (type == le_xmlrpc_server) {
		/* HACK: use output encoding for now */
		input_opts.xml_elem_opts.encoding = utf8_get_encoding_id_from_string(out.xmlrpc_out.xml_elem_opts.encoding);

		/* generate an XMLRPC_REQUEST from the raw xml input */
		xRequest = XMLRPC_REQUEST_FromXML(rawxml, rawxml_len, &input_opts);

		if (xRequest) {
			const char* methodname = XMLRPC_RequestGetMethodName(xRequest);
			XMLRPC_VALUE xAnswer = NULL;
			MAKE_STD_ZVAL(data.xmlrpc_method); /* init. very important.  spent a frustrating day finding this out. */
			MAKE_STD_ZVAL(data.return_data);
			Z_TYPE_P(data.return_data) = IS_NULL;  /* in case value is never init'd, we don't dtor to think it is a string or something */
			Z_TYPE_P(data.xmlrpc_method) = IS_NULL;

			/* setup some data to pass to the callback function */
			data.caller_params = *caller_params;
			data.php_executed = 0;
			data.server = server;

			/* We could just call the php method directly ourselves at this point, but we do this 
			 * with a C callback in case the xmlrpc library ever implements some cool usage stats,
			 * or somesuch.
			 */
			xAnswer = XMLRPC_ServerCallMethod(server->server_ptr, xRequest, &data);
			if (xAnswer && out.b_php_out) {
				zval_dtor(data.return_data);
				FREE_ZVAL(data.return_data);
				data.return_data = XMLRPC_to_PHP(xAnswer);
			} else if (data.php_executed && !out.b_php_out && !xAnswer) {
				xAnswer = PHP_to_XMLRPC(data.return_data TSRMLS_CC);
			}

			/* should we return data as xml? */
			if (!out.b_php_out) {
				XMLRPC_REQUEST xResponse = XMLRPC_RequestNew();
				if (xResponse) {
					char *outBuf = 0;
					int buf_len = 0;

					/* automagically determine output serialization type from request type */
					if (out.b_auto_version) { 
						XMLRPC_REQUEST_OUTPUT_OPTIONS opts = XMLRPC_RequestGetOutputOptions(xRequest);
						if (opts) {
							out.xmlrpc_out.version = opts->version;
						}
					}
					/* set some required request hoojum */
					XMLRPC_RequestSetOutputOptions(xResponse, &out.xmlrpc_out);
					XMLRPC_RequestSetRequestType(xResponse, xmlrpc_request_response);
					XMLRPC_RequestSetData(xResponse, xAnswer);
					XMLRPC_RequestSetMethodName(xResponse, methodname);

					/* generate xml */
					outBuf = XMLRPC_REQUEST_ToXML(xResponse, &buf_len);
					if (outBuf) {
						RETVAL_STRINGL(outBuf, buf_len, 1);
						free(outBuf);
					}
					/* cleanup after ourselves.  what a sty! */
					XMLRPC_RequestFree(xResponse, 0);
				}
			} else { /* or as native php types? */
				*return_value = *data.return_data;
				zval_copy_ctor(return_value);
			}

			/* cleanup after ourselves.  what a sty! */
			zval_ptr_dtor(&data.xmlrpc_method);

			zval_dtor(data.return_data);
			FREE_ZVAL(data.return_data);

			if (xAnswer) {
				XMLRPC_CleanupValue(xAnswer);
			}

			XMLRPC_RequestFree(xRequest, 1);
		}
	}
}
/* }}} */

/* {{{ proto int xmlrpc_server_add_introspection_data(resource server, array desc)
   Adds introspection documentation  */
PHP_FUNCTION(xmlrpc_server_add_introspection_data)
{
	zval *handle, *desc;
	int type;
	xmlrpc_server_data* server;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ra", &handle, &desc) == FAILURE) {
		return;
	}

	server = zend_list_find(Z_LVAL_P(handle), &type);

	if (type == le_xmlrpc_server) {
		XMLRPC_VALUE xDesc = PHP_to_XMLRPC(desc TSRMLS_CC);
		if (xDesc) {
			int retval = XMLRPC_ServerAddIntrospectionData(server->server_ptr, xDesc);
			XMLRPC_CleanupValue(xDesc);
			RETURN_LONG(retval);
		}
	}
	RETURN_LONG(0);
}
/* }}} */

/* {{{ proto array xmlrpc_parse_method_descriptions(string xml)
   Decodes XML into a list of method descriptions */
PHP_FUNCTION(xmlrpc_parse_method_descriptions)
{
	zval *retval;
	char *arg1;
	int arg1_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg1, &arg1_len) == FAILURE) {
		return;
	}

	if (return_value_used) {
		STRUCT_XMLRPC_ERROR err = {0};
		XMLRPC_VALUE xVal = XMLRPC_IntrospectionCreateDescription(arg1, &err);
		if (xVal) {
			retval = XMLRPC_to_PHP(xVal);

			if (retval) {
				RETVAL_ZVAL(retval, 1, 1);
			}
			/* dust, sweep, and mop */
			XMLRPC_CleanupValue(xVal);
		} else {
			/* could not create description */
			if (err.xml_elem_error.parser_code) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "xml parse error: [line %ld, column %ld, message: %s] Unable to create introspection data", 
						err.xml_elem_error.column, err.xml_elem_error.line, err.xml_elem_error.parser_error);
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid xml structure. Unable to create introspection data");
			}

			php_error_docref(NULL TSRMLS_CC, E_WARNING, "xml parse error. no method description created");
		}
	}
}
/* }}} */

/************
* type data *
************/

#define XMLRPC_TYPE_COUNT 9
#define XMLRPC_VECTOR_TYPE_COUNT 4
#define TYPE_STR_MAP_SIZE (XMLRPC_TYPE_COUNT + XMLRPC_VECTOR_TYPE_COUNT)

/* return a string matching a given xmlrpc type */
static const char** get_type_str_mapping(void) /* {{{ */
{
	static const char* str_mapping[TYPE_STR_MAP_SIZE];
	static int first = 1;
	if (first) {
		/* warning. do not add/delete without changing size define */
		str_mapping[xmlrpc_none]     = "none";
		str_mapping[xmlrpc_empty]    = "empty";
		str_mapping[xmlrpc_base64]   = "base64";
		str_mapping[xmlrpc_boolean]  = "boolean";
		str_mapping[xmlrpc_datetime] = "datetime";
		str_mapping[xmlrpc_double]   = "double";
		str_mapping[xmlrpc_int]      = "int";
		str_mapping[xmlrpc_string]   = "string";
		str_mapping[xmlrpc_vector]   = "vector";
		str_mapping[XMLRPC_TYPE_COUNT + xmlrpc_vector_none]   = "none";
		str_mapping[XMLRPC_TYPE_COUNT + xmlrpc_vector_array]  = "array";
		str_mapping[XMLRPC_TYPE_COUNT + xmlrpc_vector_mixed]  = "mixed";
		str_mapping[XMLRPC_TYPE_COUNT + xmlrpc_vector_struct] = "struct";
		first = 0;
	}
	return (const char**)str_mapping;
}
/* }}} */

/* map an xmlrpc type to a string */
const char* xmlrpc_type_as_str(XMLRPC_VALUE_TYPE type, XMLRPC_VECTOR_TYPE vtype) /* {{{ */
{
	const char** str_mapping = get_type_str_mapping();

	if (vtype == xmlrpc_vector_none) {
		return str_mapping[type];
	} else {
		return str_mapping[XMLRPC_TYPE_COUNT + vtype];
	}
}
/* }}} */

/* map a string to an xmlrpc type */
XMLRPC_VALUE_TYPE xmlrpc_str_as_type(const char* str) /* {{{ */
{
	const char** str_mapping = get_type_str_mapping();
	int i;

	if (str) {
		for (i = 0; i < XMLRPC_TYPE_COUNT; i++) {
			if (!strcmp(str_mapping[i], str)) {
				return (XMLRPC_VALUE_TYPE) i;
			}
		}
	}
	return xmlrpc_none;
}
/* }}} */

/* map a string to an xmlrpc vector type */
XMLRPC_VECTOR_TYPE xmlrpc_str_as_vector_type(const char* str) /* {{{ */
{
	const char** str_mapping = get_type_str_mapping();
	int i;

	if (str) {
		for (i = XMLRPC_TYPE_COUNT; i < TYPE_STR_MAP_SIZE; i++) {
			if (!strcmp(str_mapping[i], str)) {
				return (XMLRPC_VECTOR_TYPE) (i - XMLRPC_TYPE_COUNT);
			}
		}
	}
	return xmlrpc_none;
}
/* }}} */

/* set a given value to a particular type. 
 * note: this only works on strings, and only for date and base64,
 *       which do not have native php types. black magic lies herein.
 */
int set_zval_xmlrpc_type(zval* value, XMLRPC_VALUE_TYPE newtype) /* {{{ */
{
	int bSuccess = FAILURE;
	TSRMLS_FETCH();

	/* we only really care about strings because they can represent
	 * base64 and datetime.  all other types have corresponding php types
	 */
	if (Z_TYPE_P(value) == IS_STRING) {
		if (newtype == xmlrpc_base64 || newtype == xmlrpc_datetime) {
			const char* typestr = xmlrpc_type_as_str(newtype, xmlrpc_vector_none);
			zval* type;

			MAKE_STD_ZVAL(type);

			Z_TYPE_P(type) = IS_STRING;
			Z_STRVAL_P(type) = estrdup(typestr);
			Z_STRLEN_P(type) = strlen(typestr);

			if (newtype == xmlrpc_datetime) {
				XMLRPC_VALUE v = XMLRPC_CreateValueDateTime_ISO8601(NULL, value->value.str.val);
				if (v) {
					time_t timestamp = (time_t) php_parse_date((char *)XMLRPC_GetValueDateTime_ISO8601(v), NULL);
					if (timestamp != -1) {
						zval* ztimestamp;

						MAKE_STD_ZVAL(ztimestamp);

						ztimestamp->type = IS_LONG;
						ztimestamp->value.lval = timestamp;

						convert_to_object(value);
						if (SUCCESS == zend_hash_update(Z_OBJPROP_P(value), OBJECT_TYPE_ATTR, sizeof(OBJECT_TYPE_ATTR), (void *) &type, sizeof(zval *), NULL)) {
							bSuccess = zend_hash_update(Z_OBJPROP_P(value), OBJECT_VALUE_TS_ATTR, sizeof(OBJECT_VALUE_TS_ATTR), (void *) &ztimestamp, sizeof(zval *), NULL);
						}
					} else {
						zval_ptr_dtor(&type);
					}
					XMLRPC_CleanupValue(v);
				} else {
					zval_ptr_dtor(&type);
				}
			} else {
				convert_to_object(value);
				bSuccess = zend_hash_update(Z_OBJPROP_P(value), OBJECT_TYPE_ATTR, sizeof(OBJECT_TYPE_ATTR), (void *) &type, sizeof(zval *), NULL);
			}
		}
	}

	return bSuccess;
}
/* }}} */

/* return xmlrpc type of a php value */
XMLRPC_VALUE_TYPE get_zval_xmlrpc_type(zval* value, zval** newvalue) /* {{{ */
{
	XMLRPC_VALUE_TYPE type = xmlrpc_none;
	TSRMLS_FETCH();

	if (value) {
		switch (Z_TYPE_P(value)) {
			case IS_NULL:
				type = xmlrpc_base64;
				break;
#ifndef BOOL_AS_LONG

			/* Right thing to do, but it breaks some legacy code. */
			case IS_BOOL:
				type = xmlrpc_boolean;
				break;
#else
			case IS_BOOL:
#endif
			case IS_LONG:
			case IS_RESOURCE:
				type = xmlrpc_int;
				break;
			case IS_DOUBLE:
				type = xmlrpc_double;
				break;
			case IS_CONSTANT:
				type = xmlrpc_string;
				break;
			case IS_STRING:
				type = xmlrpc_string;
				break;
			case IS_ARRAY:
			case IS_CONSTANT_ARRAY:
				type = xmlrpc_vector;
				break;
			case IS_OBJECT:
				{
					zval** attr;
					type = xmlrpc_vector;

					if (zend_hash_find(Z_OBJPROP_P(value), OBJECT_TYPE_ATTR, sizeof(OBJECT_TYPE_ATTR), (void**) &attr) == SUCCESS) {
						if (Z_TYPE_PP(attr) == IS_STRING) {
							type = xmlrpc_str_as_type(Z_STRVAL_PP(attr));
						}
					}
					break;
				}
		}

		/* if requested, return an unmolested (magic removed) copy of the value */
		if (newvalue) {
			zval** val;

			if ((type == xmlrpc_base64 && Z_TYPE_P(value) != IS_NULL) || type == xmlrpc_datetime) {
				if (zend_hash_find(Z_OBJPROP_P(value), OBJECT_VALUE_ATTR, sizeof(OBJECT_VALUE_ATTR), (void**) &val) == SUCCESS) {
					*newvalue = *val;
				}
			} else {
				*newvalue = value;
			}
		}
	}

	return type;
}
/* }}} */

/* {{{ proto bool xmlrpc_set_type(string value, string type)
   Sets xmlrpc type, base64 or datetime, for a PHP string value */
PHP_FUNCTION(xmlrpc_set_type)
{
	zval **arg;
	char *type;
	int type_len;
	XMLRPC_VALUE_TYPE vtype;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Zs", &arg, &type, &type_len) == FAILURE) {
		return;
	}

	vtype = xmlrpc_str_as_type(type);
	if (vtype != xmlrpc_none) {
		if (set_zval_xmlrpc_type(*arg, vtype) == SUCCESS) {
			RETURN_TRUE;
		}
	} else {
		zend_error(E_WARNING,"invalid type '%s' passed to xmlrpc_set_type()", type);
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto string xmlrpc_get_type(mixed value)
   Gets xmlrpc type for a PHP value. Especially useful for base64 and datetime strings */
PHP_FUNCTION(xmlrpc_get_type)
{
	zval **arg;
	XMLRPC_VALUE_TYPE type;
	XMLRPC_VECTOR_TYPE vtype = xmlrpc_vector_none;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Z", &arg) == FAILURE) {
		return;
	}

	type = get_zval_xmlrpc_type(*arg, 0);
	if (type == xmlrpc_vector) {
		vtype = determine_vector_type((Z_TYPE_PP(arg) == IS_OBJECT) ? Z_OBJPROP_PP(arg) : Z_ARRVAL_PP(arg));
	}
   
	RETURN_STRING((char*) xmlrpc_type_as_str(type, vtype), 1);
}
/* }}} */

/* {{{ proto bool xmlrpc_is_fault(array)
   Determines if an array value represents an XMLRPC fault. */
PHP_FUNCTION(xmlrpc_is_fault)
{
	zval *arg, **val;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &arg) == FAILURE) {
		return;
	}

	/* The "correct" way to do this would be to call the xmlrpc
	 * library XMLRPC_ValueIsFault() func.  However, doing that
	 * would require us to create an xmlrpc value from the php
	 * array, which is rather expensive, especially if it was
	 * a big array.  Thus, we resort to this not so clever hackery.
	 */
	if (zend_hash_find(Z_ARRVAL_P(arg), FAULT_CODE, FAULT_CODE_LEN + 1, (void**) &val) == SUCCESS && 
		zend_hash_find(Z_ARRVAL_P(arg), FAULT_STRING, FAULT_STRING_LEN + 1, (void**) &val) == SUCCESS) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */

