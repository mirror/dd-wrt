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
   | Authors: Stig S�ther Bakken <ssb@php.net>                            |
   |          Andreas Karajannis <Andreas.Karajannis@gmd.de>              |
   |	      Kevin N. Shallow <kshallow@tampabay.rr.com> (Birdstep)      |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_ODBC_INCLUDES_H
#define PHP_ODBC_INCLUDES_H

#if HAVE_UODBC

#define ODBCVER 0x0250
/*#ifndef MSVC5
#define FAR
#endif
*/

/* checking in the same order as in configure.in */

#if defined(HAVE_SOLID) || defined(HAVE_SOLID_30) || defined(HAVE_SOLID_35) /* Solid Server */

#define ODBC_TYPE "Solid"
#if defined(HAVE_SOLID)
# include <cli0core.h>
# include <cli0ext1.h>
# include <cli0env.h>
#elif defined(HAVE_SOLID_30)
# include <cli0cli.h>
# include <cli0defs.h>
# include <cli0env.h>
#elif defined(HAVE_SOLID_35)
# if !defined(PHP_WIN32)
#  include <sqlunix.h>
# endif		/* end: #if !defined(PHP_WIN32) */
# include <sqltypes.h>
# include <sqlucode.h>
# include <sqlext.h>
# include <sql.h>
#endif	/* end: #if defined(HAVE_SOLID) */
#undef HAVE_SQL_EXTENDED_FETCH
PHP_FUNCTION(solid_fetch_prev);
#define SQLSMALLINT SWORD
#define SQLUSMALLINT UWORD
#ifndef SQL_SUCCEEDED
#define SQL_SUCCEEDED(rc) (((rc)&(~1))==0)
#endif

#elif defined(HAVE_EMPRESS) /* Empress */

#define ODBC_TYPE "Empress"
#include <sql.h>
#include <sqlext.h>
#undef HAVE_SQL_EXTENDED_FETCH

#elif defined(HAVE_ADABAS) /* Adabas D */

#define ODBC_TYPE "Adabas D"
#include <WINDOWS.H>
#include <sql.h>
#include <sqlext.h>
#define HAVE_SQL_EXTENDED_FETCH 1
#define SQL_SUCCEEDED(rc) (((rc)&(~1))==0)
#define SQLINTEGER ULONG
#define SQLUSMALLINT USHORT

#elif defined(HAVE_SAPDB) /* SAP DB */

#define ODBC_TYPE "SAP DB"
#include <WINDOWS.H>
#include <sql.h>
#include <sqlext.h>
#define HAVE_SQL_EXTENDED_FETCH 1
#define SQL_SUCCEEDED(rc) (((rc)&(~1))==0)

#elif defined(HAVE_IODBC) /* iODBC library */

#ifdef CHAR
#undef CHAR
#endif

#ifdef SQLCHAR
#undef SQLCHAR
#endif

#define ODBC_TYPE "iODBC"
#include <sql.h>
#include <sqlext.h>
#include <iodbcext.h>
#define HAVE_SQL_EXTENDED_FETCH 1

#elif defined(HAVE_UNIXODBC) /* unixODBC library */

#ifdef CHAR
#undef CHAR
#endif

#ifdef SQLCHAR
#undef SQLCHAR
#endif

#define ODBC_TYPE "unixODBC"
#include <sql.h>
#include <sqlext.h>
#define HAVE_SQL_EXTENDED_FETCH 1

#elif defined(HAVE_ESOOB) /* Easysoft ODBC-ODBC Bridge library */

#define ODBC_TYPE "ESOOB"
#include <sql.h>
#include <sqlext.h>
#define HAVE_SQL_EXTENDED_FETCH 1

#elif defined(HAVE_ODBC_ROUTER) /* ODBCRouter.com */

#ifdef CHAR
#undef CHAR
#endif

#ifdef SQLCHAR
#undef SQLCHAR
#endif

#define ODBC_TYPE "ODBCRouter"
#include <odbcsdk.h>
#undef HAVE_SQL_EXTENDED_FETCH

#elif defined(HAVE_OPENLINK) /* OpenLink ODBC drivers */

#define ODBC_TYPE "Openlink"
#include <iodbc.h>
#include <isql.h>
#include <isqlext.h>
#include <udbcext.h>
#define HAVE_SQL_EXTENDED_FETCH 1
#ifndef SQLSMALLINT
#define SQLSMALLINT SWORD
#endif
#ifndef SQLUSMALLINT
#define SQLUSMALLINT UWORD
#endif

#elif defined(HAVE_BIRDSTEP) /* Raima Birdstep */

#define ODBC_TYPE "Birdstep"
#define UNIX
/*
 * Extended Fetch in the Birdstep ODBC API is incapable of returning long varchar (memo) fields.
 * So the following line has been commented-out to accommodate. - KNS
 *
 * #define HAVE_SQL_EXTENDED_FETCH 1
 */
#include <sql.h>
#include <sqlext.h>
#define SQLINTEGER SDWORD
#define SQLSMALLINT SWORD
#define SQLUSMALLINT UWORD


#elif defined(HAVE_DBMAKER) /* DBMaker */

#define ODBC_TYPE "DBMaker"
#undef ODBCVER
#define ODBCVER 0x0300
#define HAVE_SQL_EXTENDED_FETCH 1
#include <odbc.h>


#elif defined(HAVE_CODBC) /* Custom ODBC */

#define ODBC_TYPE "Custom ODBC"
#define HAVE_SQL_EXTENDED_FETCH 1
#include <odbc.h>

#elif defined(HAVE_IBMDB2) /* DB2 CLI */

#define ODBC_TYPE "IBM DB2 CLI"
#define HAVE_SQL_EXTENDED_FETCH 1
#include <sqlcli1.h>
#ifdef DB268K
/* Need to include ASLM for 68K applications */
#include <LibraryManager.h>
#endif

#else /* MS ODBC */

#define HAVE_SQL_EXTENDED_FETCH 1
#include <WINDOWS.H>
#include <sql.h>
#include <sqlext.h>
#endif


/* Common defines */

#if defined( HAVE_IBMDB2 ) || defined( HAVE_UNIXODBC ) || defined (HAVE_IODBC)
#define ODBC_SQL_ENV_T SQLHANDLE
#define ODBC_SQL_CONN_T SQLHANDLE
#define ODBC_SQL_STMT_T SQLHANDLE
#elif defined( HAVE_SOLID_35 ) || defined( HAVE_SAPDB ) || defined ( HAVE_EMPRESS )
#define ODBC_SQL_ENV_T SQLHENV
#define ODBC_SQL_CONN_T SQLHDBC
#define ODBC_SQL_STMT_T SQLHSTMT
#else
#define ODBC_SQL_ENV_T HENV
#define ODBC_SQL_CONN_T HDBC
#define ODBC_SQL_STMT_T HSTMT
#endif

typedef struct odbc_connection {
    ODBC_SQL_ENV_T henv;
    ODBC_SQL_CONN_T hdbc;
    char laststate[6];
    char lasterrormsg[SQL_MAX_MESSAGE_LENGTH];
	int id;
	int persistent;
} odbc_connection;

typedef struct odbc_result_value {
	char name[256];
	char *value;
	SQLLEN vallen;
	SQLLEN coltype;
} odbc_result_value;

typedef struct odbc_result {
	ODBC_SQL_STMT_T stmt;
	odbc_result_value *values;
	SQLSMALLINT numcols;
	SQLSMALLINT numparams;
# if HAVE_SQL_EXTENDED_FETCH
	int fetch_abs;
# endif
	long longreadlen;
	int binmode;
	int fetched;
	odbc_connection *conn_ptr;
} odbc_result;

ZEND_BEGIN_MODULE_GLOBALS(odbc)
	char *defDB;
	char *defUser;
	char *defPW;
	long allow_persistent;
	long check_persistent;
	long max_persistent;
	long max_links;
	long num_persistent;
	long num_links;
	int defConn;
    long defaultlrl;
    long defaultbinmode;
    long default_cursortype;
    char laststate[6];
    char lasterrormsg[SQL_MAX_MESSAGE_LENGTH];
	HashTable *resource_list;
	HashTable *resource_plist;
ZEND_END_MODULE_GLOBALS(odbc)

int odbc_add_result(HashTable *list, odbc_result *result);
odbc_result *odbc_get_result(HashTable *list, int count);
void odbc_del_result(HashTable *list, int count);
int odbc_add_conn(HashTable *list, HDBC conn);
odbc_connection *odbc_get_conn(HashTable *list, int count);
void odbc_del_conn(HashTable *list, int ind);
int odbc_bindcols(odbc_result *result TSRMLS_DC);

#define ODBC_SQL_ERROR_PARAMS odbc_connection *conn_resource, ODBC_SQL_STMT_T stmt, char *func

void odbc_sql_error(ODBC_SQL_ERROR_PARAMS);

#define IS_SQL_LONG(x) (x == SQL_LONGVARBINARY || x == SQL_LONGVARCHAR)
#define IS_SQL_BINARY(x) (x == SQL_BINARY || x == SQL_VARBINARY || x == SQL_LONGVARBINARY)

#ifdef ZTS
# define ODBCG(v) TSRMG(odbc_globals_id, zend_odbc_globals *, v)
#else
# define ODBCG(v) (odbc_globals.v)
extern ZEND_API zend_odbc_globals odbc_globals;
#endif

#endif /* HAVE_UODBC */
#endif /* PHP_ODBC_INCLUDES_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
