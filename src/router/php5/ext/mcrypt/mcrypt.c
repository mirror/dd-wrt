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
   | Authors: Sascha Schumann <sascha@schumann.cx>                        |
   |          Derick Rethans <derick@derickrethans.nl>                    |
   +----------------------------------------------------------------------+
 */
/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"

#if HAVE_LIBMCRYPT

#if PHP_WIN32
# include "win32/winutil.h"
#endif

#include "php_mcrypt.h"
#include "fcntl.h"

#define NON_FREE
#define MCRYPT2
#include "mcrypt.h"
#include "php_ini.h"
#include "php_globals.h"
#include "ext/standard/info.h"
#include "ext/standard/php_rand.h"
#include "php_mcrypt_filter.h"

static int le_mcrypt;

typedef struct _php_mcrypt { 
	MCRYPT td;
	zend_bool init;
} php_mcrypt;

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_module_open, 0, 0, 4)
	ZEND_ARG_INFO(0, cipher)
	ZEND_ARG_INFO(0, cipher_directory)
	ZEND_ARG_INFO(0, mode)
	ZEND_ARG_INFO(0, mode_directory)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_generic_init, 0, 0, 3)
	ZEND_ARG_INFO(0, td)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, iv)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_generic, 0, 0, 2)
	ZEND_ARG_INFO(0, td)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mdecrypt_generic, 0, 0, 2)
	ZEND_ARG_INFO(0, td)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_enc_get_supported_key_sizes, 0, 0, 1)
	ZEND_ARG_INFO(0, td)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_enc_self_test, 0, 0, 1)
	ZEND_ARG_INFO(0, td)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_module_close, 0, 0, 1)
	ZEND_ARG_INFO(0, td)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_generic_deinit, 0, 0, 1)
	ZEND_ARG_INFO(0, td)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_enc_is_block_algorithm_mode, 0, 0, 1)
	ZEND_ARG_INFO(0, td)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_enc_is_block_algorithm, 0, 0, 1)
	ZEND_ARG_INFO(0, td)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_enc_is_block_mode, 0, 0, 1)
	ZEND_ARG_INFO(0, td)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_enc_get_block_size, 0, 0, 1)
	ZEND_ARG_INFO(0, td)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_enc_get_key_size, 0, 0, 1)
	ZEND_ARG_INFO(0, td)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_enc_get_iv_size, 0, 0, 1)
	ZEND_ARG_INFO(0, td)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_enc_get_algorithms_name, 0, 0, 1)
	ZEND_ARG_INFO(0, td)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_enc_get_modes_name, 0, 0, 1)
	ZEND_ARG_INFO(0, td)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_module_self_test, 0, 0, 1)
	ZEND_ARG_INFO(0, algorithm)
	ZEND_ARG_INFO(0, lib_dir)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_module_is_block_algorithm_mode, 0, 0, 1)
	ZEND_ARG_INFO(0, mode)
	ZEND_ARG_INFO(0, lib_dir)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_module_is_block_algorithm, 0, 0, 1)
	ZEND_ARG_INFO(0, algorithm)
	ZEND_ARG_INFO(0, lib_dir)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_module_is_block_mode, 0, 0, 1)
	ZEND_ARG_INFO(0, mode)
	ZEND_ARG_INFO(0, lib_dir)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_module_get_algo_block_size, 0, 0, 1)
	ZEND_ARG_INFO(0, algorithm)
	ZEND_ARG_INFO(0, lib_dir)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_module_get_algo_key_size, 0, 0, 1)
	ZEND_ARG_INFO(0, algorithm)
	ZEND_ARG_INFO(0, lib_dir)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_module_get_supported_key_sizes, 0, 0, 1)
	ZEND_ARG_INFO(0, algorithm)
	ZEND_ARG_INFO(0, lib_dir)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_list_algorithms, 0, 0, 0)
	ZEND_ARG_INFO(0, lib_dir)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_list_modes, 0, 0, 0)
	ZEND_ARG_INFO(0, lib_dir)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_get_key_size, 0, 0, 2)
	ZEND_ARG_INFO(0, cipher)
	ZEND_ARG_INFO(0, module)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_get_block_size, 0, 0, 2)
	ZEND_ARG_INFO(0, cipher)
	ZEND_ARG_INFO(0, module)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_get_iv_size, 0, 0, 2)
	ZEND_ARG_INFO(0, cipher)
	ZEND_ARG_INFO(0, module)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_get_cipher_name, 0, 0, 1)
	ZEND_ARG_INFO(0, cipher)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_encrypt, 0, 0, 5)
	ZEND_ARG_INFO(0, cipher)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, mode)
	ZEND_ARG_INFO(0, iv)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_decrypt, 0, 0, 5)
	ZEND_ARG_INFO(0, cipher)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, mode)
	ZEND_ARG_INFO(0, iv)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_ecb, 0, 0, 5)
	ZEND_ARG_INFO(0, cipher)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, mode)
	ZEND_ARG_INFO(0, iv)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_cbc, 0, 0, 5)
	ZEND_ARG_INFO(0, cipher)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, mode)
	ZEND_ARG_INFO(0, iv)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_cfb, 0, 0, 5)
	ZEND_ARG_INFO(0, cipher)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, mode)
	ZEND_ARG_INFO(0, iv)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_ofb, 0, 0, 5)
	ZEND_ARG_INFO(0, cipher)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, mode)
	ZEND_ARG_INFO(0, iv)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mcrypt_create_iv, 0, 0, 2)
	ZEND_ARG_INFO(0, size)
	ZEND_ARG_INFO(0, source)
ZEND_END_ARG_INFO()
/* }}} */

const zend_function_entry mcrypt_functions[] = { /* {{{ */
	PHP_DEP_FE(mcrypt_ecb, 				arginfo_mcrypt_ecb)
	PHP_DEP_FE(mcrypt_cbc, 				arginfo_mcrypt_cbc)
	PHP_DEP_FE(mcrypt_cfb, 				arginfo_mcrypt_cfb)
	PHP_DEP_FE(mcrypt_ofb, 				arginfo_mcrypt_ofb)
	PHP_FE(mcrypt_get_key_size, 	arginfo_mcrypt_get_key_size)
	PHP_FE(mcrypt_get_block_size, 	arginfo_mcrypt_get_block_size)
	PHP_FE(mcrypt_get_cipher_name, 	arginfo_mcrypt_get_cipher_name)
	PHP_FE(mcrypt_create_iv, 		arginfo_mcrypt_create_iv)

	PHP_FE(mcrypt_list_algorithms, 	arginfo_mcrypt_list_algorithms)
	PHP_FE(mcrypt_list_modes, 		arginfo_mcrypt_list_modes)
	PHP_FE(mcrypt_get_iv_size, 		arginfo_mcrypt_get_iv_size)
	PHP_FE(mcrypt_encrypt, 			arginfo_mcrypt_encrypt)
	PHP_FE(mcrypt_decrypt, 			arginfo_mcrypt_decrypt)

	PHP_FE(mcrypt_module_open, 		arginfo_mcrypt_module_open)
	PHP_FE(mcrypt_generic_init, 	arginfo_mcrypt_generic_init)
	PHP_FE(mcrypt_generic, 			arginfo_mcrypt_generic)
	PHP_FE(mdecrypt_generic, 		arginfo_mdecrypt_generic)
	PHP_DEP_FALIAS(mcrypt_generic_end, mcrypt_generic_deinit, arginfo_mcrypt_generic_deinit)
	PHP_FE(mcrypt_generic_deinit, 	arginfo_mcrypt_generic_deinit)

	PHP_FE(mcrypt_enc_self_test, 	arginfo_mcrypt_enc_self_test)
	PHP_FE(mcrypt_enc_is_block_algorithm_mode, arginfo_mcrypt_enc_is_block_algorithm_mode)
	PHP_FE(mcrypt_enc_is_block_algorithm, 	arginfo_mcrypt_enc_is_block_algorithm)
	PHP_FE(mcrypt_enc_is_block_mode, 		arginfo_mcrypt_enc_is_block_mode)
	PHP_FE(mcrypt_enc_get_block_size, 		arginfo_mcrypt_enc_get_block_size)
	PHP_FE(mcrypt_enc_get_key_size, 		arginfo_mcrypt_enc_get_key_size)
	PHP_FE(mcrypt_enc_get_supported_key_sizes, arginfo_mcrypt_enc_get_supported_key_sizes)
	PHP_FE(mcrypt_enc_get_iv_size, 			arginfo_mcrypt_enc_get_iv_size)
	PHP_FE(mcrypt_enc_get_algorithms_name, 	arginfo_mcrypt_enc_get_algorithms_name)
	PHP_FE(mcrypt_enc_get_modes_name, 		arginfo_mcrypt_enc_get_modes_name)
	PHP_FE(mcrypt_module_self_test, 		arginfo_mcrypt_module_self_test)

	PHP_FE(mcrypt_module_is_block_algorithm_mode, 	arginfo_mcrypt_module_is_block_algorithm_mode)
	PHP_FE(mcrypt_module_is_block_algorithm, 		arginfo_mcrypt_module_is_block_algorithm)
	PHP_FE(mcrypt_module_is_block_mode, 			arginfo_mcrypt_module_is_block_mode)
	PHP_FE(mcrypt_module_get_algo_block_size, 		arginfo_mcrypt_module_get_algo_block_size)
	PHP_FE(mcrypt_module_get_algo_key_size, 		arginfo_mcrypt_module_get_algo_key_size)
	PHP_FE(mcrypt_module_get_supported_key_sizes, 	arginfo_mcrypt_module_get_supported_key_sizes)

	PHP_FE(mcrypt_module_close, 					arginfo_mcrypt_module_close)
	PHP_FE_END
};
/* }}} */

static PHP_MINFO_FUNCTION(mcrypt);
static PHP_MINIT_FUNCTION(mcrypt);
static PHP_MSHUTDOWN_FUNCTION(mcrypt);

ZEND_DECLARE_MODULE_GLOBALS(mcrypt)

zend_module_entry mcrypt_module_entry = {
	STANDARD_MODULE_HEADER,
	"mcrypt", 
	mcrypt_functions,
	PHP_MINIT(mcrypt), PHP_MSHUTDOWN(mcrypt),
	NULL, NULL,
	PHP_MINFO(mcrypt),
	NO_VERSION_YET,
	PHP_MODULE_GLOBALS(mcrypt),
	NULL,
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_MCRYPT
ZEND_GET_MODULE(mcrypt)
#endif

#define MCRYPT_ARGS2 											\
	zval **cipher, **data, **key, **mode; 						\
	int td; 													\
	char *ndata; 												\
	size_t bsize; 												\
	size_t nr; 													\
	size_t nsize

#define MCRYPT_ARGS 											\
	MCRYPT_ARGS2; 												\
	zval **iv

#define MCRYPT_SIZE 										\
	bsize = mcrypt_get_block_size(Z_LVAL_PP(cipher)); 		\
	nr = (Z_STRLEN_PP(data) + bsize - 1) / bsize; 			\
	nsize = nr * bsize

#define MCRYPT_CHECK_TD_CPY 									\
	if (td < 0) { 												\
		php_error_docref(NULL TSRMLS_CC, E_WARNING, MCRYPT_FAILED); 					\
		RETURN_FALSE; 											\
	} 															\
	ndata = ecalloc(nr, bsize); 								\
	memcpy(ndata, Z_STRVAL_PP(data), Z_STRLEN_PP(data))

#define MCRYPT_CHECK_IV 										\
	convert_to_string_ex(iv);	 								\
	if (Z_STRLEN_PP(iv) != bsize) { 							\
		php_error_docref(NULL TSRMLS_CC, E_WARNING, MCRYPT_IV_WRONG_SIZE); 			\
		RETURN_FALSE; 											\
	}

#define MCRYPT_ACTION(x) 										\
	if (Z_LVAL_PP(mode) == 0) { 								\
		mcrypt_##x(td, ndata, nsize); 							\
	} else {													\
		mdecrypt_##x(td, ndata, nsize); 						\
	}															\
	end_mcrypt_##x(td)

#define MCRYPT_IV_WRONG_SIZE "The IV parameter must be as long as the blocksize"

#define MCRYPT_ENCRYPT 0
#define MCRYPT_DECRYPT 1

#define MCRYPT_GET_INI											\
	cipher_dir_string = MCG(algorithms_dir); 					\
	module_dir_string = MCG(modes_dir);

/*
 * #warning is not ANSI C
 * #warning Invalidate resource if the param count is wrong, or other problems
 * #warning occurred during functions.
 */

#define MCRYPT_GET_CRYPT_ARGS										\
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sssZ|s", 	\
		&cipher, &cipher_len, &key, &key_len, &data, &data_len, &mode, &iv, &iv_len) == FAILURE) {	\
		return;		\
	}

#define MCRYPT_GET_TD_ARG										\
	zval *mcryptind;											\
	php_mcrypt *pm;													\
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mcryptind) == FAILURE) {			\
		return;																\
	}																						\
	ZEND_FETCH_RESOURCE (pm, php_mcrypt *, &mcryptind, -1, "MCrypt", le_mcrypt);				

#define MCRYPT_GET_MODE_DIR_ARGS(DIRECTORY)								\
	char *dir = NULL;                                                   \
	int   dir_len;                                                      \
	char *module;                                                       \
	int   module_len;                                                   \
	if (zend_parse_parameters (ZEND_NUM_ARGS() TSRMLS_CC,               \
		"s|s", &module, &module_len, &dir, &dir_len) == FAILURE) {      \
		return;                                                         \
	}

#define MCRYPT_OPEN_MODULE_FAILED "Module initialization failed"

#define MCRYPT_ENTRY2_2_4(a,b) REGISTER_STRING_CONSTANT("MCRYPT_" #a, b, CONST_PERSISTENT)
#define MCRYPT_ENTRY2_4(a) MCRYPT_ENTRY_NAMED(a, a)

#define PHP_MCRYPT_INIT_CHECK	\
	if (!pm->init) {	\
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Operation disallowed prior to mcrypt_generic_init().");	\
		RETURN_FALSE;	\
	}	\

PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("mcrypt.algorithms_dir", NULL, PHP_INI_ALL, OnUpdateString, algorithms_dir, zend_mcrypt_globals, mcrypt_globals)
	STD_PHP_INI_ENTRY("mcrypt.modes_dir",      NULL, PHP_INI_ALL, OnUpdateString, modes_dir, zend_mcrypt_globals, mcrypt_globals)
PHP_INI_END()

static void php_mcrypt_module_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC) /* {{{ */
{
	php_mcrypt *pm = (php_mcrypt *) rsrc->ptr;
	if (pm) {	
		mcrypt_generic_deinit(pm->td);
		mcrypt_module_close(pm->td);
		efree(pm);
		pm = NULL;
	}
}
/* }}} */
    
static PHP_MINIT_FUNCTION(mcrypt) /* {{{ */
{
	le_mcrypt = zend_register_list_destructors_ex(php_mcrypt_module_dtor, NULL, "mcrypt", module_number);

	/* modes for mcrypt_??? routines */
	REGISTER_LONG_CONSTANT("MCRYPT_ENCRYPT", 0, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MCRYPT_DECRYPT", 1, CONST_PERSISTENT);

	/* sources for mcrypt_create_iv */
	REGISTER_LONG_CONSTANT("MCRYPT_DEV_RANDOM", 0, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MCRYPT_DEV_URANDOM", 1, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MCRYPT_RAND", 2, CONST_PERSISTENT);

	/* ciphers */
	MCRYPT_ENTRY2_2_4(3DES, "tripledes");
	MCRYPT_ENTRY2_2_4(ARCFOUR_IV, "arcfour-iv");
	MCRYPT_ENTRY2_2_4(ARCFOUR, "arcfour");
	MCRYPT_ENTRY2_2_4(BLOWFISH, "blowfish");
	MCRYPT_ENTRY2_2_4(BLOWFISH_COMPAT, "blowfish-compat");
	MCRYPT_ENTRY2_2_4(CAST_128, "cast-128");
	MCRYPT_ENTRY2_2_4(CAST_256, "cast-256");
	MCRYPT_ENTRY2_2_4(CRYPT, "crypt");
	MCRYPT_ENTRY2_2_4(DES, "des");
	MCRYPT_ENTRY2_2_4(ENIGNA, "crypt");
	MCRYPT_ENTRY2_2_4(GOST, "gost");
	MCRYPT_ENTRY2_2_4(LOKI97, "loki97");
	MCRYPT_ENTRY2_2_4(PANAMA, "panama");
	MCRYPT_ENTRY2_2_4(RC2, "rc2");
	MCRYPT_ENTRY2_2_4(RIJNDAEL_128, "rijndael-128");
	MCRYPT_ENTRY2_2_4(RIJNDAEL_192, "rijndael-192");
	MCRYPT_ENTRY2_2_4(RIJNDAEL_256, "rijndael-256");
	MCRYPT_ENTRY2_2_4(SAFER64, "safer-sk64");
	MCRYPT_ENTRY2_2_4(SAFER128, "safer-sk128");
	MCRYPT_ENTRY2_2_4(SAFERPLUS, "saferplus");
	MCRYPT_ENTRY2_2_4(SERPENT, "serpent");
	MCRYPT_ENTRY2_2_4(THREEWAY, "threeway");
	MCRYPT_ENTRY2_2_4(TRIPLEDES, "tripledes");
	MCRYPT_ENTRY2_2_4(TWOFISH, "twofish");
	MCRYPT_ENTRY2_2_4(WAKE, "wake");
	MCRYPT_ENTRY2_2_4(XTEA, "xtea");

	MCRYPT_ENTRY2_2_4(IDEA, "idea");
	MCRYPT_ENTRY2_2_4(MARS, "mars");
	MCRYPT_ENTRY2_2_4(RC6, "rc6");
	MCRYPT_ENTRY2_2_4(SKIPJACK, "skipjack");
/* modes */
	MCRYPT_ENTRY2_2_4(MODE_CBC, "cbc");
	MCRYPT_ENTRY2_2_4(MODE_CFB, "cfb");
	MCRYPT_ENTRY2_2_4(MODE_ECB, "ecb");
	MCRYPT_ENTRY2_2_4(MODE_NOFB, "nofb");
	MCRYPT_ENTRY2_2_4(MODE_OFB, "ofb");
	MCRYPT_ENTRY2_2_4(MODE_STREAM, "stream");
	REGISTER_INI_ENTRIES();

	php_stream_filter_register_factory("mcrypt.*", &php_mcrypt_filter_factory TSRMLS_CC);
	php_stream_filter_register_factory("mdecrypt.*", &php_mcrypt_filter_factory TSRMLS_CC);

	return SUCCESS;
}
/* }}} */

static PHP_MSHUTDOWN_FUNCTION(mcrypt) /* {{{ */
{
	php_stream_filter_unregister_factory("mcrypt.*" TSRMLS_CC);
	php_stream_filter_unregister_factory("mdecrypt.*" TSRMLS_CC);

	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

#include "ext/standard/php_smart_str.h"

PHP_MINFO_FUNCTION(mcrypt) /* {{{ */
{
	char **modules;
	char mcrypt_api_no[16];
	int i, count;
	smart_str tmp1 = {0};
	smart_str tmp2 = {0};

	modules = mcrypt_list_algorithms(MCG(algorithms_dir), &count);
	if (count == 0) {
		smart_str_appends(&tmp1, "none");
	}
	for (i = 0; i < count; i++) {
		smart_str_appends(&tmp1, modules[i]);
		smart_str_appendc(&tmp1, ' ');
	}
	smart_str_0(&tmp1);
	mcrypt_free_p(modules, count);

	modules = mcrypt_list_modes(MCG(modes_dir), &count);
	if (count == 0) {
		smart_str_appends(&tmp2, "none");
	}
	for (i = 0; i < count; i++) {
		smart_str_appends(&tmp2, modules[i]);
		smart_str_appendc(&tmp2, ' ');
	}
	smart_str_0 (&tmp2);
	mcrypt_free_p (modules, count);

	snprintf (mcrypt_api_no, 16, "%d", MCRYPT_API_VERSION);

	php_info_print_table_start();
	php_info_print_table_header(2, "mcrypt support", "enabled");
	php_info_print_table_header(2, "mcrypt_filter support", "enabled");
	php_info_print_table_row(2, "Version", LIBMCRYPT_VERSION);
	php_info_print_table_row(2, "Api No", mcrypt_api_no);
	php_info_print_table_row(2, "Supported ciphers", tmp1.c);
	php_info_print_table_row(2, "Supported modes", tmp2.c);
	smart_str_free(&tmp1);
	smart_str_free(&tmp2);
	php_info_print_table_end();
	
	DISPLAY_INI_ENTRIES();
}
/* }}} */

typedef enum {
	RANDOM = 0,
	URANDOM,
	RAND
} iv_source;

/* {{{ proto resource mcrypt_module_open(string cipher, string cipher_directory, string mode, string mode_directory)
   Opens the module of the algorithm and the mode to be used */
PHP_FUNCTION(mcrypt_module_open)
{
	char *cipher, *cipher_dir;
	char *mode,   *mode_dir;
	int   cipher_len, cipher_dir_len;
	int   mode_len,   mode_dir_len;
	MCRYPT td;
	php_mcrypt *pm;
   
	if (zend_parse_parameters (ZEND_NUM_ARGS() TSRMLS_CC, "ssss",
		&cipher, &cipher_len, &cipher_dir, &cipher_dir_len,
		&mode,   &mode_len,   &mode_dir,   &mode_dir_len)) {
		return;
	}
	
	td = mcrypt_module_open (
		cipher,
		cipher_dir_len > 0 ? cipher_dir : MCG(algorithms_dir),
		mode, 
		mode_dir_len > 0 ? mode_dir : MCG(modes_dir)
	);

	if (td == MCRYPT_FAILED) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not open encryption module");
		RETURN_FALSE;
	} else {
		pm = emalloc(sizeof(php_mcrypt));
		pm->td = td;
		pm->init = 0;
		ZEND_REGISTER_RESOURCE(return_value, pm, le_mcrypt);
	}
}
/* }}} */

/* {{{ proto int mcrypt_generic_init(resource td, string key, string iv)
   This function initializes all buffers for the specific module */
PHP_FUNCTION(mcrypt_generic_init)
{
	char *key, *iv;
	int key_len, iv_len;
	zval *mcryptind;
	unsigned char *key_s, *iv_s;
	int max_key_size, key_size, iv_size;
	php_mcrypt *pm;
	int result = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rss", &mcryptind, &key, &key_len, &iv, &iv_len) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(pm, php_mcrypt *, &mcryptind, -1, "MCrypt", le_mcrypt);

	max_key_size = mcrypt_enc_get_key_size(pm->td);
	iv_size = mcrypt_enc_get_iv_size(pm->td);

	if (key_len == 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Key size is 0");
	}

	key_s = emalloc(key_len);
	memset(key_s, 0, key_len);

	iv_s = emalloc(iv_size + 1);
	memset(iv_s, 0, iv_size + 1);

	if (key_len > max_key_size) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Key size too large; supplied length: %d, max: %d", key_len, max_key_size);
		key_size = max_key_size;
	} else {
		key_size = key_len;
	}
	memcpy(key_s, key, key_len);

	if (iv_len != iv_size) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Iv size incorrect; supplied length: %d, needed: %d", iv_len, iv_size);
	}
	memcpy(iv_s, iv, iv_size);

	mcrypt_generic_deinit(pm->td);
	result = mcrypt_generic_init(pm->td, key_s, key_size, iv_s);

	/* If this function fails, close the mcrypt module to prevent crashes
	 * when further functions want to access this resource */
	if (result < 0) {
		zend_list_delete(Z_LVAL_P(mcryptind));
		switch (result) {
			case -3:
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Key length incorrect");
				break;
			case -4:
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Memory allocation error");
				break;
			case -1:
			default:
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unknown error");
				break;
		}
	}
	pm->init = 1;
	RETVAL_LONG(result);

	efree(iv_s);
	efree(key_s);
}
/* }}} */

/* {{{ proto string mcrypt_generic(resource td, string data)
   This function encrypts the plaintext */
PHP_FUNCTION(mcrypt_generic)
{
	zval *mcryptind;
	char *data;
	int data_len;
	php_mcrypt *pm;
	unsigned char* data_s;
	int block_size, data_size;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &mcryptind, &data, &data_len) == FAILURE) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(pm, php_mcrypt *, &mcryptind, -1, "MCrypt", le_mcrypt);
	PHP_MCRYPT_INIT_CHECK

	if (data_len == 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "An empty string was passed");
		RETURN_FALSE
	}

	/* Check blocksize */
	if (mcrypt_enc_is_block_mode(pm->td) == 1) { /* It's a block algorithm */
		block_size = mcrypt_enc_get_block_size(pm->td);
		data_size = (((data_len - 1) / block_size) + 1) * block_size;
		data_s = emalloc(data_size + 1);
		memset(data_s, 0, data_size);
		memcpy(data_s, data, data_len);
	} else { /* It's not a block algorithm */
		data_size = data_len;
		data_s = emalloc(data_size + 1);
		memset(data_s, 0, data_size);
		memcpy(data_s, data, data_len);
	}
	
	mcrypt_generic(pm->td, data_s, data_size);
	data_s[data_size] = '\0';

	RETVAL_STRINGL(data_s, data_size, 1);
	efree(data_s);
}
/* }}} */

/* {{{ proto string mdecrypt_generic(resource td, string data)
   This function decrypts the plaintext */
PHP_FUNCTION(mdecrypt_generic)
{
	zval *mcryptind;
	char *data;
	int data_len;
	php_mcrypt *pm;
	char* data_s;
	int block_size, data_size;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &mcryptind, &data, &data_len) == FAILURE) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(pm, php_mcrypt * , &mcryptind, -1, "MCrypt", le_mcrypt);
	PHP_MCRYPT_INIT_CHECK

	if (data_len == 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "An empty string was passed");
		RETURN_FALSE
	}

	/* Check blocksize */
	if (mcrypt_enc_is_block_mode(pm->td) == 1) { /* It's a block algorithm */
		block_size = mcrypt_enc_get_block_size(pm->td);
		data_size = (((data_len - 1) / block_size) + 1) * block_size;
		data_s = emalloc(data_size + 1);
		memset(data_s, 0, data_size);
		memcpy(data_s, data, data_len);
	} else { /* It's not a block algorithm */
		data_size = data_len;
		data_s = emalloc(data_size + 1);
		memset(data_s, 0, data_size);
		memcpy(data_s, data, data_len);
	}
	
	mdecrypt_generic(pm->td, data_s, data_size);

	RETVAL_STRINGL(data_s, data_size, 1);
	efree(data_s);
}
/* }}} */

/* {{{ proto array mcrypt_enc_get_supported_key_sizes(resource td)
   This function decrypts the crypttext */
PHP_FUNCTION(mcrypt_enc_get_supported_key_sizes)
{
	int i, count = 0;
	int *key_sizes;
	
	MCRYPT_GET_TD_ARG
	array_init(return_value);

	key_sizes = mcrypt_enc_get_supported_key_sizes(pm->td, &count);

	for (i = 0; i < count; i++) {
		add_index_long(return_value, i, key_sizes[i]);
	}

	mcrypt_free(key_sizes);
}
/* }}} */

/* {{{ proto int mcrypt_enc_self_test(resource td)
   This function runs the self test on the algorithm specified by the descriptor td */
PHP_FUNCTION(mcrypt_enc_self_test)
{
	MCRYPT_GET_TD_ARG
	RETURN_LONG(mcrypt_enc_self_test(pm->td));
}
/* }}} */

/* {{{ proto bool mcrypt_module_close(resource td)
   Free the descriptor td */
PHP_FUNCTION(mcrypt_module_close)
{
	MCRYPT_GET_TD_ARG
	zend_list_delete(Z_LVAL_P(mcryptind));
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool mcrypt_generic_deinit(resource td)
   This function terminates encrypt specified by the descriptor td */
PHP_FUNCTION(mcrypt_generic_deinit)
{
	MCRYPT_GET_TD_ARG

	if (mcrypt_generic_deinit(pm->td) < 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not terminate encryption specifier");
		RETURN_FALSE
	}
	pm->init = 0;
	RETURN_TRUE
}
/* }}} */

/* {{{ proto bool mcrypt_enc_is_block_algorithm_mode(resource td)
   Returns TRUE if the mode is for use with block algorithms */
PHP_FUNCTION(mcrypt_enc_is_block_algorithm_mode)
{
	MCRYPT_GET_TD_ARG

	if (mcrypt_enc_is_block_algorithm_mode(pm->td) == 1) {
		RETURN_TRUE
	} else {
		RETURN_FALSE
	}
}
/* }}} */

/* {{{ proto bool mcrypt_enc_is_block_algorithm(resource td)
   Returns TRUE if the alrogithm is a block algorithms */
PHP_FUNCTION(mcrypt_enc_is_block_algorithm)
{
	MCRYPT_GET_TD_ARG

	if (mcrypt_enc_is_block_algorithm(pm->td) == 1) {
		RETURN_TRUE 
	} else {
		RETURN_FALSE
	}
}
/* }}} */

/* {{{ proto bool mcrypt_enc_is_block_mode(resource td)
   Returns TRUE if the mode outputs blocks */
PHP_FUNCTION(mcrypt_enc_is_block_mode)
{
	MCRYPT_GET_TD_ARG

	if (mcrypt_enc_is_block_mode(pm->td) == 1) {
		RETURN_TRUE
	} else {
		RETURN_FALSE
	}
}
/* }}} */

/* {{{ proto int mcrypt_enc_get_block_size(resource td)
   Returns the block size of the cipher specified by the descriptor td */
PHP_FUNCTION(mcrypt_enc_get_block_size)
{
	MCRYPT_GET_TD_ARG
	RETURN_LONG(mcrypt_enc_get_block_size(pm->td));
}
/* }}} */

/* {{{ proto int mcrypt_enc_get_key_size(resource td)
   Returns the maximum supported key size in bytes of the algorithm specified by the descriptor td */
PHP_FUNCTION(mcrypt_enc_get_key_size)
{
	MCRYPT_GET_TD_ARG
	RETURN_LONG(mcrypt_enc_get_key_size(pm->td));
}
/* }}} */

/* {{{ proto int mcrypt_enc_get_iv_size(resource td)
   Returns the size of the IV in bytes of the algorithm specified by the descriptor td */
PHP_FUNCTION(mcrypt_enc_get_iv_size)
{
	MCRYPT_GET_TD_ARG
	RETURN_LONG(mcrypt_enc_get_iv_size(pm->td));
}
/* }}} */

/* {{{ proto string mcrypt_enc_get_algorithms_name(resource td)
   Returns the name of the algorithm specified by the descriptor td */
PHP_FUNCTION(mcrypt_enc_get_algorithms_name)
{
	char *name;
	MCRYPT_GET_TD_ARG

	name = mcrypt_enc_get_algorithms_name(pm->td);
	RETVAL_STRING(name, 1);
	mcrypt_free(name);
}
/* }}} */

/* {{{ proto string mcrypt_enc_get_modes_name(resource td)
   Returns the name of the mode specified by the descriptor td */
PHP_FUNCTION(mcrypt_enc_get_modes_name)
{
	char *name;
	MCRYPT_GET_TD_ARG

	name = mcrypt_enc_get_modes_name(pm->td);
	RETVAL_STRING(name, 1);
	mcrypt_free(name);
}
/* }}} */

/* {{{ proto bool mcrypt_module_self_test(string algorithm [, string lib_dir])
   Does a self test of the module "module" */
PHP_FUNCTION(mcrypt_module_self_test)
{
	MCRYPT_GET_MODE_DIR_ARGS(algorithms_dir);
	
	if (mcrypt_module_self_test(module, dir) == 0) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool mcrypt_module_is_block_algorithm_mode(string mode [, string lib_dir])
   Returns TRUE if the mode is for use with block algorithms */
PHP_FUNCTION(mcrypt_module_is_block_algorithm_mode)
{
	MCRYPT_GET_MODE_DIR_ARGS(modes_dir)
	
	if (mcrypt_module_is_block_algorithm_mode(module, dir) == 1) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool mcrypt_module_is_block_algorithm(string algorithm [, string lib_dir])
   Returns TRUE if the algorithm is a block algorithm */
PHP_FUNCTION(mcrypt_module_is_block_algorithm)
{
	MCRYPT_GET_MODE_DIR_ARGS(algorithms_dir)
	
	if (mcrypt_module_is_block_algorithm(module, dir) == 1) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool mcrypt_module_is_block_mode(string mode [, string lib_dir])
   Returns TRUE if the mode outputs blocks of bytes */
PHP_FUNCTION(mcrypt_module_is_block_mode)
{
	MCRYPT_GET_MODE_DIR_ARGS(modes_dir)
	
	if (mcrypt_module_is_block_mode(module, dir) == 1) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto int mcrypt_module_get_algo_block_size(string algorithm [, string lib_dir])
   Returns the block size of the algorithm */
PHP_FUNCTION(mcrypt_module_get_algo_block_size)
{
	MCRYPT_GET_MODE_DIR_ARGS(algorithms_dir)
	
	RETURN_LONG(mcrypt_module_get_algo_block_size(module, dir));
}
/* }}} */

/* {{{ proto int mcrypt_module_get_algo_key_size(string algorithm [, string lib_dir])
   Returns the maximum supported key size of the algorithm */
PHP_FUNCTION(mcrypt_module_get_algo_key_size)
{
	MCRYPT_GET_MODE_DIR_ARGS(algorithms_dir);
	
	RETURN_LONG(mcrypt_module_get_algo_key_size(module, dir));
}
/* }}} */

/* {{{ proto array mcrypt_module_get_supported_key_sizes(string algorithm [, string lib_dir])
   This function decrypts the crypttext */
PHP_FUNCTION(mcrypt_module_get_supported_key_sizes)
{
	int i, count = 0;
	int *key_sizes;
	
	MCRYPT_GET_MODE_DIR_ARGS(algorithms_dir)
	array_init(return_value);

	key_sizes = mcrypt_module_get_algo_supported_key_sizes(module, dir, &count);

	for (i = 0; i < count; i++) {
		add_index_long(return_value, i, key_sizes[i]);
	}
	mcrypt_free(key_sizes);
}
/* }}} */

/* {{{ proto array mcrypt_list_algorithms([string lib_dir])
   List all algorithms in "module_dir" */
PHP_FUNCTION(mcrypt_list_algorithms)
{
	char **modules;
	char *lib_dir = MCG(algorithms_dir);
	int   lib_dir_len;
	int   i, count;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s",
		&lib_dir, &lib_dir_len) == FAILURE) {
		return;
	}
	
	array_init(return_value);
	modules = mcrypt_list_algorithms(lib_dir, &count);

	if (count == 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "No algorithms found in module dir");
	}
	for (i = 0; i < count; i++) {
		add_index_string(return_value, i, modules[i], 1);
	}
	mcrypt_free_p(modules, count);
}
/* }}} */

/* {{{ proto array mcrypt_list_modes([string lib_dir])
   List all modes "module_dir" */
PHP_FUNCTION(mcrypt_list_modes)
{
	char **modules;
	char *lib_dir = MCG(modes_dir);
	int   lib_dir_len;
	int   i, count;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s",
		&lib_dir, &lib_dir_len) == FAILURE) {
		return;
	}

	array_init(return_value);
	modules = mcrypt_list_modes(lib_dir, &count);

	if (count == 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "No modes found in module dir");
	}
	for (i = 0; i < count; i++) {
		add_index_string(return_value, i, modules[i], 1);
	}
	mcrypt_free_p(modules, count);
}
/* }}} */

/* {{{ proto int mcrypt_get_key_size(string cipher, string module)
   Get the key size of cipher */
PHP_FUNCTION(mcrypt_get_key_size)
{
	char *cipher;
	char *module;
	int   cipher_len, module_len; 
	char *cipher_dir_string;
	char *module_dir_string;
	MCRYPT td;

	MCRYPT_GET_INI

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
		&cipher, &cipher_len, &module, &module_len) == FAILURE) {
		return;
	}
	
	td = mcrypt_module_open(cipher, cipher_dir_string, module, module_dir_string);
	if (td != MCRYPT_FAILED) {
		RETVAL_LONG(mcrypt_enc_get_key_size(td));
		mcrypt_module_close(td);
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, MCRYPT_OPEN_MODULE_FAILED);
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto int mcrypt_get_block_size(string cipher, string module)
   Get the key size of cipher */
PHP_FUNCTION(mcrypt_get_block_size)
{
	char *cipher;
	char *module;
	int   cipher_len, module_len; 
	char *cipher_dir_string;
	char *module_dir_string;
	MCRYPT td;

	MCRYPT_GET_INI

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
		&cipher, &cipher_len, &module, &module_len) == FAILURE) {
		return;
	}
	
	td = mcrypt_module_open(cipher, cipher_dir_string, module, module_dir_string);
	if (td != MCRYPT_FAILED) {
		RETVAL_LONG(mcrypt_enc_get_block_size(td));
		mcrypt_module_close(td);
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, MCRYPT_OPEN_MODULE_FAILED);
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto int mcrypt_get_iv_size(string cipher, string module)
   Get the IV size of cipher (Usually the same as the blocksize) */
PHP_FUNCTION(mcrypt_get_iv_size)
{
	char *cipher;
	char *module;
	int   cipher_len, module_len; 
	char *cipher_dir_string;
	char *module_dir_string;
	MCRYPT td;

	MCRYPT_GET_INI

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
		&cipher, &cipher_len, &module, &module_len) == FAILURE) {
		return;
	}
	
	td = mcrypt_module_open(cipher, cipher_dir_string, module, module_dir_string);
	if (td != MCRYPT_FAILED) {
		RETVAL_LONG(mcrypt_enc_get_iv_size(td));
		mcrypt_module_close(td);
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, MCRYPT_OPEN_MODULE_FAILED);
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto string mcrypt_get_cipher_name(string cipher)
   Get the key size of cipher */
PHP_FUNCTION(mcrypt_get_cipher_name)
{
	char *cipher_dir_string;
	char *module_dir_string;
	char *cipher_name;
	char *cipher;
	int   cipher_len;
	MCRYPT td;

	MCRYPT_GET_INI

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
		&cipher, &cipher_len) == FAILURE) {
		return;
	}

	/* The code below is actually not very nice, but I didn't see a better
	 * method */
	td = mcrypt_module_open(cipher, cipher_dir_string, "ecb", module_dir_string);
	if (td != MCRYPT_FAILED) {
		cipher_name = mcrypt_enc_get_algorithms_name(td);
		mcrypt_module_close(td);
		RETVAL_STRING(cipher_name,1);
		mcrypt_free(cipher_name);
	} else {
		td = mcrypt_module_open(cipher, cipher_dir_string, "stream", module_dir_string);
		if (td != MCRYPT_FAILED) {
			cipher_name = mcrypt_enc_get_algorithms_name(td);
			mcrypt_module_close(td);
			RETVAL_STRING(cipher_name,1);
			mcrypt_free(cipher_name);
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, MCRYPT_OPEN_MODULE_FAILED);
			RETURN_FALSE;
		}
	}
}
/* }}} */

static void php_mcrypt_do_crypt(char* cipher, const char *key, int key_len, const char *data, int data_len, char *mode, const char *iv, int iv_len, int argc, int dencrypt, zval* return_value TSRMLS_DC) /* {{{ */
{
	char *cipher_dir_string;
	char *module_dir_string;
	int block_size, max_key_length, use_key_length, i, count, iv_size;
	unsigned long int data_size;
	int *key_length_sizes;
	char *key_s = NULL, *iv_s;
	char *data_s;
	MCRYPT td;

	MCRYPT_GET_INI

	td = mcrypt_module_open(cipher, cipher_dir_string, mode, module_dir_string);
	if (td == MCRYPT_FAILED) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, MCRYPT_OPEN_MODULE_FAILED);
		RETURN_FALSE;
	}
	/* Checking for key-length */
	max_key_length = mcrypt_enc_get_key_size(td);
	if (key_len > max_key_length) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Size of key is too large for this algorithm");
	}
	key_length_sizes = mcrypt_enc_get_supported_key_sizes(td, &count);
	if (count == 0 && key_length_sizes == NULL) { /* all lengths 1 - k_l_s = OK */
		use_key_length = key_len;
		key_s = emalloc(use_key_length);
		memset(key_s, 0, use_key_length);
		memcpy(key_s, key, use_key_length);
	} else if (count == 1) {  /* only m_k_l = OK */
		key_s = emalloc(key_length_sizes[0]);
		memset(key_s, 0, key_length_sizes[0]);
		memcpy(key_s, key, MIN(key_len, key_length_sizes[0]));
		use_key_length = key_length_sizes[0];
	} else { /* dertermine smallest supported key > length of requested key */
		use_key_length = max_key_length; /* start with max key length */
		for (i = 0; i < count; i++) {
			if (key_length_sizes[i] >= key_len && 
				key_length_sizes[i] < use_key_length)
			{
				use_key_length = key_length_sizes[i];
			}
		}
		key_s = emalloc(use_key_length);
		memset(key_s, 0, use_key_length);
		memcpy(key_s, key, MIN(key_len, use_key_length));
	}
	mcrypt_free (key_length_sizes);
	
	/* Check IV */
	iv_s = NULL;
	iv_size = mcrypt_enc_get_iv_size (td);
	
	/* IV is required */
	if (mcrypt_enc_mode_has_iv(td) == 1) {
		if (argc == 5) {
			if (iv_size != iv_len) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, MCRYPT_IV_WRONG_SIZE);
			} else {
				iv_s = emalloc(iv_size + 1);
				memcpy(iv_s, iv, iv_size);
			}
		} else if (argc == 4) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Attempt to use an empty IV, which is NOT recommend");
			iv_s = emalloc(iv_size + 1);
			memset(iv_s, 0, iv_size + 1);
		}
	}

	/* Check blocksize */
	if (mcrypt_enc_is_block_mode(td) == 1) { /* It's a block algorithm */
		block_size = mcrypt_enc_get_block_size(td);
		data_size = (((data_len - 1) / block_size) + 1) * block_size;
		data_s = emalloc(data_size);
		memset(data_s, 0, data_size);
		memcpy(data_s, data, data_len);
	} else { /* It's not a block algorithm */
		data_size = data_len;
		data_s = emalloc(data_size);
		memset(data_s, 0, data_size);
		memcpy(data_s, data, data_len);
	}

	if (mcrypt_generic_init(td, key_s, use_key_length, iv_s) < 0) {
		php_error_docref(NULL TSRMLS_CC, E_RECOVERABLE_ERROR, "Mcrypt initialisation failed");
		RETURN_FALSE;
	}
	if (dencrypt == MCRYPT_ENCRYPT) {
		mcrypt_generic(td, data_s, data_size);
	} else {
		mdecrypt_generic(td, data_s, data_size);
	}
	
	RETVAL_STRINGL(data_s, data_size, 1);

	/* freeing vars */
	mcrypt_generic_end(td);
	if (key_s != NULL) {
		efree (key_s);
	}
	if (iv_s != NULL) {
		efree (iv_s);
	}
	efree (data_s);
}
/* }}} */

/* {{{ proto string mcrypt_encrypt(string cipher, string key, string data, string mode, string iv)
   OFB crypt/decrypt data using key key with cipher cipher starting with iv */
PHP_FUNCTION(mcrypt_encrypt)
{
	zval **mode;
	char *cipher, *key, *data, *iv = NULL;
	int cipher_len, key_len, data_len, iv_len = 0;
	
	MCRYPT_GET_CRYPT_ARGS
	
	convert_to_string_ex(mode);

	php_mcrypt_do_crypt(cipher, key, key_len, data, data_len, Z_STRVAL_PP(mode), iv, iv_len, ZEND_NUM_ARGS(), MCRYPT_ENCRYPT, return_value TSRMLS_CC);
}
/* }}} */

/* {{{ proto string mcrypt_decrypt(string cipher, string key, string data, string mode, string iv)
   OFB crypt/decrypt data using key key with cipher cipher starting with iv */
PHP_FUNCTION(mcrypt_decrypt)
{
	zval **mode;
	char *cipher, *key, *data, *iv = NULL;
	int cipher_len, key_len, data_len, iv_len = 0;

	MCRYPT_GET_CRYPT_ARGS
	
	convert_to_string_ex(mode);

	php_mcrypt_do_crypt(cipher, key, key_len, data, data_len, Z_STRVAL_PP(mode), iv, iv_len, ZEND_NUM_ARGS(), MCRYPT_DECRYPT, return_value TSRMLS_CC);
}
/* }}} */

/* {{{ proto string mcrypt_ecb(int cipher, string key, string data, int mode, string iv)
   ECB crypt/decrypt data using key key with cipher cipher starting with iv */
PHP_FUNCTION(mcrypt_ecb)
{
	zval **mode;
	char *cipher, *key, *data, *iv = NULL;
	int cipher_len, key_len, data_len, iv_len = 0;
	
	MCRYPT_GET_CRYPT_ARGS

	convert_to_long_ex(mode);

	php_mcrypt_do_crypt(cipher, key, key_len, data, data_len, "ecb", iv, iv_len, ZEND_NUM_ARGS(), Z_LVAL_PP(mode), return_value TSRMLS_CC);
}
/* }}} */

/* {{{ proto string mcrypt_cbc(int cipher, string key, string data, int mode, string iv)
   CBC crypt/decrypt data using key key with cipher cipher starting with iv */
PHP_FUNCTION(mcrypt_cbc)
{
	zval **mode;
	char *cipher, *key, *data, *iv = NULL;
	int cipher_len, key_len, data_len, iv_len = 0;

	MCRYPT_GET_CRYPT_ARGS

	convert_to_long_ex(mode);

	php_mcrypt_do_crypt(cipher, key, key_len, data, data_len, "cbc", iv, iv_len, ZEND_NUM_ARGS(), Z_LVAL_PP(mode), return_value TSRMLS_CC);
}
/* }}} */

/* {{{ proto string mcrypt_cfb(int cipher, string key, string data, int mode, string iv)
   CFB crypt/decrypt data using key key with cipher cipher starting with iv */
PHP_FUNCTION(mcrypt_cfb)
{
	zval **mode;
	char *cipher, *key, *data, *iv = NULL;
	int cipher_len, key_len, data_len, iv_len = 0;
	
	MCRYPT_GET_CRYPT_ARGS

	convert_to_long_ex(mode);

	php_mcrypt_do_crypt(cipher, key, key_len, data, data_len, "cfb", iv, iv_len, ZEND_NUM_ARGS(), Z_LVAL_PP(mode), return_value TSRMLS_CC);
}
/* }}} */

/* {{{ proto string mcrypt_ofb(int cipher, string key, string data, int mode, string iv)
   OFB crypt/decrypt data using key key with cipher cipher starting with iv */
PHP_FUNCTION(mcrypt_ofb)
{
	zval **mode;
	char *cipher, *key, *data, *iv = NULL;
	int cipher_len, key_len, data_len, iv_len = 0;
	
	MCRYPT_GET_CRYPT_ARGS

	convert_to_long_ex(mode);

	php_mcrypt_do_crypt(cipher, key, key_len, data, data_len, "ofb", iv, iv_len, ZEND_NUM_ARGS(), Z_LVAL_PP(mode), return_value TSRMLS_CC);
}
/* }}} */

/* {{{ proto string mcrypt_create_iv(int size, int source)
   Create an initialization vector (IV) */
PHP_FUNCTION(mcrypt_create_iv)
{
	char *iv;
	long source = RANDOM;
	long size;
	int n = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|l", &size, &source) == FAILURE) {
		return;
	}

	if (size <= 0 || size >= INT_MAX) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot create an IV with a size of less than 1 or greater than %d", INT_MAX);
		RETURN_FALSE;
	}
	
	iv = ecalloc(size + 1, 1);
	
	if (source == RANDOM || source == URANDOM) {
#if PHP_WIN32
		/* random/urandom equivalent on Windows */
		BYTE *iv_b = (BYTE *) iv;
		if (php_win32_get_random_bytes(iv_b, (size_t) size) == FAILURE){
			efree(iv);
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not gather sufficient random data");
			RETURN_FALSE;
		}
		n = size;
#else
		int    fd;
		size_t read_bytes = 0;

		fd = open(source == RANDOM ? "/dev/random" : "/dev/urandom", O_RDONLY);
		if (fd < 0) {
			efree(iv);
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot open source device");
			RETURN_FALSE;
		}
		while (read_bytes < size) {
			n = read(fd, iv + read_bytes, size - read_bytes);
			if (n < 0) {
				break;
			}
			read_bytes += n;
		}
		n = read_bytes;
		close(fd);
		if (n < size) {
			efree(iv);
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not gather sufficient random data");
			RETURN_FALSE;
		}
#endif
	} else {
		n = size;
		while (size) {
			iv[--size] = (char) (255.0 * php_rand(TSRMLS_C) / RAND_MAX);
		}
	}
	RETURN_STRINGL(iv, n, 0);
}
/* }}} */

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
