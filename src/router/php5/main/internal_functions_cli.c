/* -*- C -*-
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2007 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
 */

/* $Id: internal_functions.c.in 265279 2008-08-22 12:59:46Z helly $ */

#include "php.h"
#include "php_main.h"
#include "zend_modules.h"
#include "zend_compile.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "ext/date/php_date.h"
#include "ext/ereg/php_ereg.h"
#include "ext/pcre/php_pcre.h"
#include "ext/fileinfo/php_fileinfo.h"
#include "ext/filter/php_filter.h"
#include "ext/hash/php_hash.h"
#include "ext/json/php_json.h"
#include "ext/posix/php_posix.h"
#include "ext/reflection/php_reflection.h"
#include "ext/session/php_session.h"
#include "ext/sockets/php_sockets.h"
#include "ext/spl/php_spl.h"
#include "ext/standard/php_standard.h"


static zend_module_entry *php_builtin_extensions[] = {
	phpext_date_ptr,
	phpext_ereg_ptr,
	phpext_pcre_ptr,
	phpext_fileinfo_ptr,
	phpext_filter_ptr,
	phpext_hash_ptr,
	phpext_json_ptr,
	phpext_posix_ptr,
	phpext_reflection_ptr,
	phpext_spl_ptr,
	phpext_session_ptr,
	phpext_sockets_ptr,
	phpext_standard_ptr,

};

#define EXTCOUNT (sizeof(php_builtin_extensions)/sizeof(zend_module_entry *))

PHPAPI int php_register_internal_extensions(TSRMLS_D)
{
	return php_register_extensions(php_builtin_extensions, EXTCOUNT TSRMLS_CC);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
