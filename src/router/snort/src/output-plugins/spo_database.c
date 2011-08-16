/*
** Portions Copyright (C) 2000,2001,2002 Carnegie Mellon University
** Copyright (C) 2001 Jed Pickel <jed@pickel.net>
** Portions Copyright (C) 2001 Andrew R. Baker <andrewb@farm9.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/* $Id$ */

/* Snort Database Output Plug-in
 * 
 *  Maintainer: Roman Danyliw <rdd@cert.org>, <roman@danyliw.com>
 *
 *  Originally written by Jed Pickel <jed@pickel.net> (2000-2001)
 *
 * See the doc/README.database file with this distribution 
 * documentation or the snortdb web site for configuration
 * information
 *
 * Web Site: http://www.andrew.cmu.edu/~rdanyliw/snortdb/snortdb.html
 */

/******** Configuration *************************************************/

/* 
 * If you want extra debugging information for solving database 
 * configuration problems, uncomment the following line. 
 */
/* #define DEBUG */

/* Enable DB transactions */
#define ENABLE_DB_TRANSACTIONS

/******** Headers ******************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#include "spo_database.h"
#include "event.h"
#include "decode.h"
#include "rules.h"
#include "treenodes.h"
#include "plugbase.h"
#include "spo_plugbase.h"
#include "parser.h"
#include "debug.h"
#include "util.h"
#include "snort.h"
#include "sfdaq.h"

#ifdef ENABLE_POSTGRESQL
# include <libpq-fe.h>
#endif

#ifdef ENABLE_MYSQL
# if defined(_WIN32) || defined(_WIN64)
#  include <windows.h>
# endif
# include <mysql.h>
# include <errmsg.h>
#endif

#ifdef ENABLE_ODBC
# include <sql.h>
# include <sqlext.h>
# include <sqltypes.h>
  /* The SQL Server libraries, for some reason I can't
   * understand, define their own constants for SQLRETURN
   * and SQLCHAR.  But, in SQL Server, these are numeric
   * values, not datatypes.  So we define datatypes here
   * with a non-conflicting name.
   */
typedef SQLRETURN ODBC_SQLRETURN;
typedef SQLCHAR   ODBC_SQLCHAR;
#endif

#ifdef ENABLE_ORACLE
# include <oci.h>
#endif

#ifdef ENABLE_MSSQL
# define DBNTWIN32
# include <windows.h>
# include <sqlfront.h>
# include <sqldb.h>
#endif

/******** Data Types  **************************************************/

/* enumerate the supported databases */
enum db_types_en
{
    DB_UNDEFINED  = 0,
    DB_MYSQL      = 1,
    DB_POSTGRESQL = 2,
    DB_MSSQL      = 3,
    DB_ORACLE     = 4,
    DB_ODBC       = 5
};
typedef enum db_types_en dbtype_t;

/* link-list of SQL queries */
typedef struct _SQLQuery
{
    char * val;
    struct _SQLQuery * next;
} SQLQuery;


/* the cid is unique across the dbtype, dbname, host, and sid */
/* therefore, we use these as a lookup key for the cid */
typedef struct _SharedDatabaseData
{
    dbtype_t  dbtype_id;
    char     *dbname;
    char     *host;
    int       sid;
    int       cid;
    int       reference;
} SharedDatabaseData;

typedef struct _DatabaseData
{
    SharedDatabaseData *shared;
    char  *facility;
    char  *password;
    char  *user;
    char  *port;
    char  *sensor_name;
    int    encoding;
    int    detail;
    int    ignore_bpf;
    int    tz;
    int    DBschema_version;
#ifdef ENABLE_POSTGRESQL
    PGconn * p_connection;
    PGresult * p_result;
#endif
#ifdef ENABLE_MYSQL
    MYSQL * m_sock;
    MYSQL_RES * m_result;
    MYSQL_ROW m_row;
#endif
#ifdef ENABLE_ODBC
    SQLHENV u_handle;
    SQLHDBC u_connection;
    SQLHSTMT u_statement;
    SQLINTEGER  u_col;
    SQLINTEGER  u_rows;
    dbtype_t    u_underlying_dbtype_id;
#endif
#ifdef ENABLE_ORACLE
    OCIEnv *o_environment;
    OCISvcCtx *o_servicecontext;
    OCIBind *o_bind;
    OCIError *o_error;
    OCIStmt *o_statement;
    OCIDefine *o_define;
    text o_errormsg[512];
    sb4 o_errorcode;
#endif
#ifdef ENABLE_MSSQL
    PDBPROCESS  ms_dbproc;
    PLOGINREC   ms_login;
    DBINT       ms_col;
#endif
    char *args;
} DatabaseData;

/* list for lookup of shared data information */
typedef struct _SharedDatabaseDataNode
{
    SharedDatabaseData *data;
    struct _SharedDatabaseDataNode *next;
} SharedDatabaseDataNode;


/******** Constants  ***************************************************/

#define MAX_QUERY_LENGTH 8192
#define KEYWORD_POSTGRESQL   "postgresql"
#define KEYWORD_MYSQL        "mysql"
#define KEYWORD_ODBC         "odbc"
#define KEYWORD_ORACLE       "oracle"
#define KEYWORD_MSSQL        "mssql"

#define KEYWORD_HOST         "host"
#define KEYWORD_PORT         "port"
#define KEYWORD_USER         "user"
#define KEYWORD_PASSWORD     "password"
#define KEYWORD_DBNAME       "dbname"
#define KEYWORD_SENSORNAME   "sensor_name"
#define KEYWORD_ENCODING     "encoding"
    #define KEYWORD_ENCODING_HEX      "hex"
    #define KEYWORD_ENCODING_BASE64   "base64"
    #define KEYWORD_ENCODING_ASCII    "ascii"
#define KEYWORD_DETAIL       "detail"
    #define KEYWORD_DETAIL_FULL  "full"
    #define KEYWORD_DETAIL_FAST  "fast"
#define KEYWORD_IGNOREBPF    "ignore_bpf"
#define KEYWORD_IGNOREBPF_NO   "no"
#define KEYWORD_IGNOREBPF_ZERO "0"
#define KEYWORD_IGNOREBPF_YES  "yes"
#define KEYWORD_IGNOREBPF_ONE  "1"


#define LATEST_DB_SCHEMA_VERSION 107

/******** fatals *******************************************************/
/* these strings deliberately break fatal error messages into
 * chunks with lengths < 509 to keep ISO C89 compilers happy
 */

static const char* FATAL_NO_SENSOR_1 =
    " When this plugin starts, a SELECT query is run to find the sensor id for the\n"
    " currently running sensor. If the sensor id is not found, the plugin will run\n"
    " an INSERT query to insert the proper data and generate a new sensor id. Then a\n"
    " SELECT query is run to get the newly allocated sensor id. If that fails then\n"
    " this error message is generated.\n";

static const char* FATAL_NO_SENSOR_2 =
    " Some possible causes for this error are:\n"
    "  * the user does not have proper INSERT or SELECT privileges\n"
    "  * the sensor table does not exist\n"
    "\n"
    " If you are _absolutely_ certain that you have the proper privileges set and\n"
    " that your database structure is built properly please let me know if you\n"
    " continue to get this error. You can contact me at (roman@danyliw.com).\n";

static const char* FATAL_BAD_SCHEMA_1 =
    "database: The underlying database has not been initialized correctly.  This\n"
    "          version of Snort requires version %d of the DB schema.  Your DB\n"
    "          doesn't appear to have any records in the 'schema' table.\n%s";

static const char* FATAL_BAD_SCHEMA_2 =
    "          Please re-run the appropriate DB creation script (e.g. create_mysql,\n"
    "          create_postgresql, create_oracle, create_mssql) located in the\n"
    "          contrib\\ directory.\n\n"
    "          See the database documentation for cursory details (doc/README.database).\n"
    "          and the URL to the most recent database plugin documentation.\n";

static const char* FATAL_OLD_SCHEMA_1 =
    "database: The underlying database seems to be running an older version of\n"
    "          the DB schema (current version=%d, required minimum version= %d).\n\n"
    "          If you have an existing database with events logged by a previous\n"
    "          version of snort, this database must first be upgraded to the latest\n"
    "          schema (see the snort-users mailing list archive or DB plugin\n"
    "          documention for details).\n%s\n";

static const char* FATAL_OLD_SCHEMA_2 =
    "          If migrating old data is not desired, merely create a new instance\n"
    "          of the snort database using the appropriate DB creation script\n"
    "          (e.g. create_mysql, create_postgresql, create_oracle, create_mssql)\n"
    "          located in the contrib\\ directory.\n\n"
    "          See the database documentation for cursory details (doc/README.database).\n"
    "          and the URL to the most recent database plugin documentation.\n";

static const char* FATAL_NO_SUPPORT_1 =
    "If this build of snort was obtained as a binary distribution (e.g., rpm,\n"
    "or Windows), then check for alternate builds that contains the necessary\n"
    "'%s' support.\n\n"
    "If this build of snort was compiled by you, then re-run the\n"
    "the ./configure script using the '--with-%s' switch.\n"
    "For non-standard installations of a database, the '--with-%s=DIR'\n%s";

static const char* FATAL_NO_SUPPORT_2 =
    "syntax may need to be used to specify the base directory of the DB install.\n\n"
    "See the database documentation for cursory details (doc/README.database).\n"
    "and the URL to the most recent database plugin documentation.\n";

/******** Prototypes  **************************************************/

static void          DatabaseInit(char *);
static DatabaseData *InitDatabaseData(char *args);
static void          DatabaseInitFinalize(int unused, void *arg);
static void          ParseDatabaseArgs(DatabaseData *data);
static void          Database(Packet *, char *, void *, Event *);
static char *        snort_escape_string(const char *, DatabaseData *);
static void          SpoDatabaseCleanExitFunction(int, void *);
static void          SpoDatabaseRestartFunction(int, void *);
//static void          InitDatabase(void);
static int           UpdateLastCid(DatabaseData *, int, int);
static int           GetLastCid(DatabaseData *, int);
static int           CheckDBVersion(DatabaseData *);
static void          BeginTransaction(DatabaseData * data);
static void          CommitTransaction(DatabaseData * data);
static void          RollbackTransaction(DatabaseData * data);
static int           Insert(char *, DatabaseData *);
static int           Select(char *, DatabaseData *);
static void          Connect(DatabaseData *);
static void          DatabasePrintUsage(void);
static void          FreeSharedDataList(void);

/******** Global Variables  ********************************************/

extern OptTreeNode *otn_tmp;  /* rule node */
extern ListHead *head_tmp;

static SharedDatabaseDataNode *sharedDataList = NULL;
static int instances = 0;

/******** Database Specific Extras  ************************************/

/* The following is for supporting Microsoft SQL Server */
#ifdef ENABLE_MSSQL

/* If you want extra debugging information (specific to
   Microsoft SQL Server), uncomment the following line. */
#define ENABLE_MSSQL_DEBUG

#if defined(DEBUG) || defined(ENABLE_MSSQL_DEBUG)
    /* this is for debugging purposes only */
    static char g_CurrentStatement[2048];
    #define SAVESTATEMENT(str)   strncpy(g_CurrentStatement, str, sizeof(g_CurrentStatement) - 1);
    #define CLEARSTATEMENT()     bzero((char *) g_CurrentStatement, sizeof(g_CurrentStatement));
#else
    #define SAVESTATEMENT(str)   NULL;
    #define CLEARSTATEMENT()     NULL;
#endif /* DEBUG || ENABLE_MSSQL_DEBUG*/

    /* Prototype of SQL Server callback functions. 
     * See actual declaration elsewhere for details. 
     */
    static int mssql_err_handler(PDBPROCESS dbproc, int severity, int dberr, 
                                 int oserr, LPCSTR dberrstr, LPCSTR oserrstr);
    static int mssql_msg_handler(PDBPROCESS dbproc, DBINT msgno, int msgstate, 
                                 int severity, LPCSTR msgtext, LPCSTR srvname, LPCSTR procname, 
                                 DBUSMALLINT line);
#endif /* ENABLE_MSSQL */

/*******************************************************************************
 * Function: SetupDatabase()
 *
 * Purpose: Registers the output plugin keyword and initialization 
 *          function into the output plugin list.  This is the function that
 *          gets called from InitOutputPlugins() in plugbase.c.
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ******************************************************************************/
void DatabaseSetup(void)
{
    /* link the preprocessor keyword to the init function in 
       the preproc list */
    RegisterOutputPlugin("database", OUTPUT_TYPE_FLAG__ALERT, DatabaseInit);

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "database(debug): database plugin is registered...\n"););
}

/*******************************************************************************
 * Function: DatabaseInit(char *)
 *
 * Purpose: Calls the argument parsing function, performs final setup on data
 *          structs, links the preproc function into the function list.
 *
 * Arguments: args => ptr to argument string
 *
 * Returns: void function
 *
 ******************************************************************************/
static void DatabaseInit(char *args)
{
    DatabaseData *data = NULL;

    /* parse the argument list from the rules file */
    data = InitDatabaseData(args);
    
    data->tz = GetLocalTimezone();

    ParseDatabaseArgs(data);
    
    /* Add the processor function into the function list */
    if (strncasecmp(data->facility, "log", 3) == 0)
    {
        AddFuncToOutputList(Database, OUTPUT_TYPE__LOG, data);
    }
    else
    {
        AddFuncToOutputList(Database, OUTPUT_TYPE__ALERT, data);
    }
    
    AddFuncToCleanExitList(SpoDatabaseCleanExitFunction, data);
    AddFuncToRestartList(SpoDatabaseRestartFunction, data); 
    AddFuncToPostConfigList(DatabaseInitFinalize, data);

    ++instances;
}

static void DatabaseInitFinalize(int unused, void *arg)
{
    DatabaseData *data = (DatabaseData *)arg;
    char * select_sensor_id = NULL;
    char * select_max_sensor_id = NULL;
    char * insert_into_sensor = NULL;
    int foundEntry = 0, sensor_cid, event_cid;
    SharedDatabaseDataNode *current = NULL;
    char * escapedSensorName = NULL;
    char * escapedInterfaceName = NULL;
    char * escapedBPFFilter = NULL;
    int ret, bad_query = 0;
    const char* iface = DAQ_GetInterfaceSpec();

    if (!data)
    {
        FatalError("database:  data uninitialized\n");
    }

    /* find a unique name for sensor if one was not supplied as an option */
    if(!data->sensor_name)
    {
        data->sensor_name = GetUniqueName((char *)PRINT_INTERFACE(iface));
        if ( data->sensor_name )
        {
            if( data->sensor_name[strlen(data->sensor_name)-1] == '\n' )
            {
                data->sensor_name[strlen(data->sensor_name)-1] = '\0';
            }
        }
    }

    /* allocate memory for configuration queries */
    select_sensor_id     = (char *)SnortAlloc(MAX_QUERY_LENGTH);
    select_max_sensor_id = (char *)SnortAlloc(MAX_QUERY_LENGTH);
    insert_into_sensor   = (char *)SnortAlloc(MAX_QUERY_LENGTH);

    escapedSensorName    = snort_escape_string(data->sensor_name, data);
    escapedInterfaceName = snort_escape_string(PRINT_INTERFACE(iface), data);

    if( data->ignore_bpf == 0 )
    {
        if(snort_conf->bpf_filter == NULL)
        {
            ret = SnortSnprintf(insert_into_sensor, MAX_QUERY_LENGTH, 
                                "INSERT INTO sensor (hostname, interface, detail, encoding, last_cid) "
                                "VALUES ('%s','%s',%u,%u, 0)", 
                                escapedSensorName, escapedInterfaceName,
                                data->detail, data->encoding);

            if (ret != SNORT_SNPRINTF_SUCCESS)
                bad_query = 1;

            ret = SnortSnprintf(select_sensor_id, MAX_QUERY_LENGTH, 
                                "SELECT sid "
                                "  FROM sensor "
                                " WHERE hostname = '%s' "
                                "   AND interface = '%s' "
                                "   AND detail = %u "
                                "   AND encoding = %u "
                                "   AND filter IS NULL",
                                escapedSensorName, escapedInterfaceName,
                                data->detail, data->encoding);
            
            if (ret != SNORT_SNPRINTF_SUCCESS)
                bad_query = 1;
        }
        else
        {
            escapedBPFFilter = snort_escape_string(snort_conf->bpf_filter, data);

            ret = SnortSnprintf(insert_into_sensor, MAX_QUERY_LENGTH, 
                                "INSERT INTO sensor (hostname, interface, filter, detail, encoding, last_cid) "
                                "VALUES ('%s','%s','%s',%u,%u, 0)", 
                                escapedSensorName, escapedInterfaceName,
                                escapedBPFFilter, data->detail, data->encoding);

            if (ret != SNORT_SNPRINTF_SUCCESS)
                bad_query = 1;

            ret = SnortSnprintf(select_sensor_id, MAX_QUERY_LENGTH, 
                                "SELECT sid "
                                "  FROM sensor "
                                " WHERE hostname = '%s' "
                                "   AND interface = '%s' "
                                "   AND filter ='%s' "
                                "   AND detail = %u "
                                "   AND encoding = %u ",
                                escapedSensorName, escapedInterfaceName,
                                escapedBPFFilter, data->detail, data->encoding);

            if (ret != SNORT_SNPRINTF_SUCCESS)
                bad_query = 1;
        }
    }
    else /* ( data->ignore_bpf == 1 ) */
    {
        if(snort_conf->bpf_filter == NULL)
        {
            ret = SnortSnprintf(insert_into_sensor, MAX_QUERY_LENGTH, 
                                "INSERT INTO sensor (hostname, interface, detail, encoding, last_cid) "
                                "VALUES ('%s','%s',%u,%u, 0)", 
                                escapedSensorName, escapedInterfaceName,
                                data->detail, data->encoding);

            if (ret != SNORT_SNPRINTF_SUCCESS)
                bad_query = 1;

            ret = SnortSnprintf(select_sensor_id, MAX_QUERY_LENGTH, 
                                "SELECT sid "
                                "  FROM sensor "
                                " WHERE hostname = '%s' "
                                "   AND interface = '%s' "
                                "   AND detail = %u "
                                "   AND encoding = %u",
                                escapedSensorName, escapedInterfaceName,
                                data->detail, data->encoding);

            if (ret != SNORT_SNPRINTF_SUCCESS)
                bad_query = 1;
        }
        else
        {
            escapedBPFFilter = snort_escape_string(snort_conf->bpf_filter, data);

            ret = SnortSnprintf(insert_into_sensor, MAX_QUERY_LENGTH, 
                                "INSERT INTO sensor (hostname, interface, filter, detail, encoding, last_cid) "
                                "VALUES ('%s','%s','%s',%u,%u, 0)", 
                                escapedSensorName, escapedInterfaceName,
                                escapedBPFFilter, data->detail, data->encoding);

            if (ret != SNORT_SNPRINTF_SUCCESS)
                bad_query = 1;

            ret = SnortSnprintf(select_sensor_id, MAX_QUERY_LENGTH, 
                                "SELECT sid "
                                "  FROM sensor "
                                " WHERE hostname = '%s' "
                                "   AND interface = '%s' "
                                "   AND detail = %u "
                                "   AND encoding = %u",
                                escapedSensorName, escapedInterfaceName,
                                data->detail, data->encoding);

            if (ret != SNORT_SNPRINTF_SUCCESS)
                bad_query = 1;
        }
    }

    if (bad_query)
        FatalError("Database: Unable to construct query - output error or truncation\n");

    Connect(data);

    /* get password out of memory since we only need it for Connect */
    if (data->password != NULL)
    {
        /* it'll be null terminated */
        while (*data->password != '\0')
        {
            *data->password = '\0';
            data->password++;
        }
    }

    data->shared->sid = Select(select_sensor_id,data);
    if(data->shared->sid == 0)
    {
        Insert(insert_into_sensor,data);
        data->shared->sid = Select(select_sensor_id,data);
        if(data->shared->sid == 0)
        {
            ErrorMessage("database: Problem obtaining SENSOR ID (sid) from %s->sensor\n", 
                         data->shared->dbname);
            FatalError("%s\n%s\n", FATAL_NO_SENSOR_1, FATAL_NO_SENSOR_2);

        }
    }

    /* the cid may be shared across multiple instances of the database
     * plugin, first we check the shared data list to see if we already
     * have a value to use, if so, we replace the SharedDatabaseData struct
     * in the DatabaseData struct with the one out of the sharedDataList.
     * Sound confusing enough?  
     *   -Andrew    
     */

    /* XXX: Creating a set of list handling functions would make this cleaner */
    current = sharedDataList;
    while(current != NULL)
    {
        /* We have 4 key fields to check */
        if((current->data->sid == data->shared->sid) &&
           (current->data->dbtype_id == data->shared->dbtype_id) &&
           /* XXX: should this be a case insensitive compare? */
           (strcasecmp(current->data->dbname, data->shared->dbname) == 0) &&
           (strcasecmp(current->data->host, data->shared->host) == 0))
        {
            foundEntry = 1;
            break;
        }
        current = current->next;
    }
    
    if(foundEntry == 0)
    {
        /* Add it the the shared data list */
        SharedDatabaseDataNode *newNode = (SharedDatabaseDataNode *)SnortAlloc(sizeof(SharedDatabaseDataNode));
        newNode->data = data->shared;
        newNode->next = NULL;
        if(sharedDataList == NULL)
        {
            sharedDataList = newNode;
        }
        else
        {
            current = sharedDataList;
            while(current->next != NULL)
            {
                current = current->next;
            }
            current->next = newNode;
        }

        /* Set the cid value 
         * - get the cid value in sensor.last_cid
         * - get the MAX(cid) from event 
         * - if snort crashed without storing the latest cid, then
         *     the MAX(event.cid) > sensor.last_cid.  Update last_cid in this case
         */
        sensor_cid = GetLastCid(data, data->shared->sid);

        if (sensor_cid == -1)
            FatalError("Database: Unable to construct query - output error or truncation\n");

        ret = SnortSnprintf(select_max_sensor_id, MAX_QUERY_LENGTH,
                            "SELECT MAX(cid) "
                            "  FROM event "
                            " WHERE sid = %u",
                            data->shared->sid);
        
        if (ret != SNORT_SNPRINTF_SUCCESS)
            FatalError("Database: Unable to construct query - output error or truncation\n");

        event_cid = Select(select_max_sensor_id, data);

        if ( event_cid > sensor_cid )
        {
           ret = UpdateLastCid(data, data->shared->sid, event_cid);

           if (ret == -1)
               FatalError("Database: Unable to construct query - output error or truncation\n");

           ErrorMessage("database: inconsistent cid information for sid=%u\n", 
                        data->shared->sid);
           ErrorMessage("          Recovering by rolling forward the cid=%u\n", 
                        event_cid);
        }

        data->shared->cid = event_cid;
        ++(data->shared->cid);
    }
    else
    {
        /* Free memory associated with data->shared */
        free(data->shared);
        data->shared = current->data;
    }

    /* free memory */
    free(select_sensor_id);      select_sensor_id = NULL;
    free(select_max_sensor_id);  select_max_sensor_id = NULL;
    free(insert_into_sensor);    insert_into_sensor = NULL;
    free(escapedSensorName);     escapedSensorName = NULL;
    free(escapedInterfaceName);  escapedInterfaceName = NULL;
    if (escapedBPFFilter != NULL)
    {
        free(escapedBPFFilter);
        escapedBPFFilter = NULL;
    }

    /* Get the versioning information for the DB schema */
    data->DBschema_version = CheckDBVersion(data);

    if (data->DBschema_version == -1)
        FatalError("Database: Unable to construct query - output error or truncation\n");

    if ( data->DBschema_version == 0 )
    {
       FatalError(FATAL_BAD_SCHEMA_1, LATEST_DB_SCHEMA_VERSION, FATAL_BAD_SCHEMA_2);
    }
    if ( data->DBschema_version < LATEST_DB_SCHEMA_VERSION )
    {
       FatalError(FATAL_OLD_SCHEMA_1, data->DBschema_version, LATEST_DB_SCHEMA_VERSION, FATAL_OLD_SCHEMA_2);
    }
    /*
    else if ( data->DBschema_version < LATEST_DB_SCHEMA_VERSION )
    {                
       ErrorMessage("database: The database is using an older version of the DB schema\n");
    }
    */

    /* print out and test the capability of this plugin */
    {
        char database_support_buf[100];
        char database_in_use_buf[100];

        database_support_buf[0] = '\0';
        database_in_use_buf[0] = '\0';

        /* These strings will not overflow the buffers */
#ifdef ENABLE_MYSQL
        snprintf(database_support_buf, sizeof(database_support_buf),
                 "database: compiled support for (%s)", KEYWORD_MYSQL);
        if (data->shared->dbtype_id == DB_MYSQL)
        snprintf(database_in_use_buf, sizeof(database_in_use_buf),
                 "database: configured to use %s", KEYWORD_MYSQL);
#endif
#ifdef ENABLE_POSTGRESQL
        snprintf(database_support_buf, sizeof(database_support_buf),
                 "database: compiled support for (%s)", KEYWORD_POSTGRESQL);
        if (data->shared->dbtype_id == DB_POSTGRESQL)
        snprintf(database_in_use_buf, sizeof(database_in_use_buf),
                 "database: configured to use %s", KEYWORD_POSTGRESQL);
#endif
#ifdef ENABLE_ODBC
        snprintf(database_support_buf, sizeof(database_support_buf),
                 "database: compiled support for (%s)", KEYWORD_ODBC);
        if (data->shared->dbtype_id == DB_ODBC)
        snprintf(database_in_use_buf, sizeof(database_in_use_buf),
                 "database: configured to use %s", KEYWORD_ODBC);
#endif
#ifdef ENABLE_ORACLE
        snprintf(database_support_buf, sizeof(database_support_buf),
                 "database: compiled support for (%s)", KEYWORD_ORACLE);
        if (data->shared->dbtype_id == DB_ORACLE)
        snprintf(database_in_use_buf, sizeof(database_in_use_buf),
                 "database: configured to use %s", KEYWORD_ORACLE);
#endif
#ifdef ENABLE_MSSQL
        snprintf(database_support_buf, sizeof(database_support_buf),
                 "database: compiled support for (%s)", KEYWORD_MSSQL);
        if (data->shared->dbtype_id == DB_MSSQL)
        snprintf(database_in_use_buf, sizeof(database_in_use_buf),
                 "database: configured to use %s", KEYWORD_MSSQL);
#endif
        LogMessage("%s\n", database_support_buf);
        LogMessage("%s\n", database_in_use_buf);
    }

    LogMessage("database: schema version = %d\n", data->DBschema_version);
    if (data->shared->host != NULL)
    LogMessage("database:           host = %s\n", data->shared->host);
    if (data->port != NULL)
    LogMessage("database:           port = %s\n", data->port);
    if (data->user != NULL)
    LogMessage("database:           user = %s\n", data->user);
    if (data->shared->dbname != NULL)
    LogMessage("database:  database name = %s\n", data->shared->dbname);
    if (data->sensor_name != NULL)
    LogMessage("database:    sensor name = %s\n", data->sensor_name);
    LogMessage("database:      sensor id = %u\n", data->shared->sid);

    if (data->encoding == ENCODING_HEX)
    LogMessage("database:  data encoding = %s\n", KEYWORD_ENCODING_HEX);
    else if (data->encoding == ENCODING_BASE64)
    LogMessage("database:  data encoding = %s\n", KEYWORD_ENCODING_BASE64);
    else
    LogMessage("database:  data encoding = %s\n", KEYWORD_ENCODING_ASCII);

    if (data->detail == DETAIL_FULL)
    LogMessage("database:   detail level = %s\n", KEYWORD_DETAIL_FULL);
    else
    LogMessage("database:   detail level = %s\n", KEYWORD_DETAIL_FAST);

    if (data->ignore_bpf)
    LogMessage("database:     ignore_bpf = %s\n", KEYWORD_IGNOREBPF_YES);
    else
    LogMessage("database:     ignore_bpf = %s\n", KEYWORD_IGNOREBPF_NO);

    if(!strncasecmp(data->facility,"log",3))
    LogMessage("database: using the \"log\" facility\n");
    else
    LogMessage("database: using the \"alert\" facility\n");
}


/*******************************************************************************
 * Function: InitDatabaseData(char *)
 *
 * Purpose: Initialize the data structure for connecting to
 *          this database.
 *
 * Arguments: args => argument list
 *
 * Returns: Pointer to database structure
 *
 ******************************************************************************/
static DatabaseData *InitDatabaseData(char *args)
{
    DatabaseData *data;

    data = (DatabaseData *)SnortAlloc(sizeof(DatabaseData));
    data->shared = (SharedDatabaseData *)SnortAlloc(sizeof(SharedDatabaseData));

    if(args == NULL)
    {
        ErrorMessage("database: you must supply arguments for database plugin\n");
        DatabasePrintUsage();
        FatalError("\n");
    }

    data->args = SnortStrdup(args);

    return data;
}

/*******************************************************************************
 * Function: ParseDatabaseArgs(char *)
 *
 * Purpose: Process the preprocessor arguements from the rules file and 
 *          initialize the preprocessor's data struct.
 *
 * Arguments: args => argument list
 *
 * Returns: void function
 *
 ******************************************************************************/
//DatabaseData *ParseDatabaseArgs(char *args)
static void ParseDatabaseArgs(DatabaseData *data)
{
    char *dbarg;
    char *a1;
    char *type;
    char *facility;

    if(data->args == NULL)
    {
        ErrorMessage("database: you must supply arguments for database plugin\n");
        DatabasePrintUsage();
        FatalError("\n");
    }

    data->shared->dbtype_id = DB_UNDEFINED;
    data->sensor_name = NULL;
    data->facility = NULL;
    data->encoding = ENCODING_HEX;
    data->detail = DETAIL_FULL;
    data->ignore_bpf = 0;

    facility = strtok(data->args, ", ");
    if(facility != NULL)
    {
        if((!strncasecmp(facility,"log",3)) || (!strncasecmp(facility,"alert",5)))
            data->facility = facility;
        else
        {
            ErrorMessage("database: The first argument needs to be the logging facility\n");
            DatabasePrintUsage();
            FatalError("\n");
        }
    }
    else
    {
        ErrorMessage("database: Invalid format for first argment\n"); 
        DatabasePrintUsage();
        FatalError("\n");
    }

    type = strtok(NULL, ", ");

    if(type == NULL)
    {
        ErrorMessage("database: you must enter the database type in configuration "
                     "file as the second argument\n");
        DatabasePrintUsage();
        FatalError("\n");
    }

#ifdef ENABLE_MYSQL
    if(!strncasecmp(type,KEYWORD_MYSQL,strlen(KEYWORD_MYSQL)))
        data->shared->dbtype_id = DB_MYSQL; 
#endif
#ifdef ENABLE_POSTGRESQL
    if(!strncasecmp(type,KEYWORD_POSTGRESQL,strlen(KEYWORD_POSTGRESQL)))
        data->shared->dbtype_id = DB_POSTGRESQL; 
#endif
#ifdef ENABLE_ODBC
    if(!strncasecmp(type,KEYWORD_ODBC,strlen(KEYWORD_ODBC)))
        data->shared->dbtype_id = DB_ODBC; 
#endif
#ifdef ENABLE_ORACLE
    if(!strncasecmp(type,KEYWORD_ORACLE,strlen(KEYWORD_ORACLE)))
        data->shared->dbtype_id = DB_ORACLE; 
#endif
#ifdef ENABLE_MSSQL
    if(!strncasecmp(type,KEYWORD_MSSQL,strlen(KEYWORD_MSSQL)))
        data->shared->dbtype_id = DB_MSSQL; 
#endif

    if(data->shared->dbtype_id == 0)
    {
        if ( !strncasecmp(type, KEYWORD_MYSQL, strlen(KEYWORD_MYSQL)) ||
             !strncasecmp(type, KEYWORD_POSTGRESQL, strlen(KEYWORD_POSTGRESQL)) ||
             !strncasecmp(type, KEYWORD_ODBC, strlen(KEYWORD_ODBC)) ||
             !strncasecmp(type, KEYWORD_MSSQL, strlen(KEYWORD_MSSQL))  ||
             !strncasecmp(type, KEYWORD_ORACLE, strlen(KEYWORD_ORACLE)) )
        {
            ErrorMessage("database: '%s' support is not compiled into this build of snort\n\n", type);
            FatalError(FATAL_NO_SUPPORT_1, type, type, type, FATAL_NO_SUPPORT_2);
        }
        else
        {
           FatalError("database: '%s' is an unknown database type.  The supported\n"
                      "          databases include: MySQL (mysql), PostgreSQL (postgresql),\n"
                      "          ODBC (odbc), Oracle (oracle), and Microsoft SQL Server (mssql)\n",
                      type);
        }
    }

    dbarg = strtok(NULL, " =");
    while(dbarg != NULL)
    {
        a1 = NULL;
        a1 = strtok(NULL, ", ");
        if(!strncasecmp(dbarg,KEYWORD_HOST,strlen(KEYWORD_HOST)))
        {
            data->shared->host = a1;
        }
        if(!strncasecmp(dbarg,KEYWORD_PORT,strlen(KEYWORD_PORT)))
        {
            data->port = a1;
        }
        if(!strncasecmp(dbarg,KEYWORD_USER,strlen(KEYWORD_USER)))
        {
            data->user = a1;
        }
        if(!strncasecmp(dbarg,KEYWORD_PASSWORD,strlen(KEYWORD_PASSWORD)))
        {
            data->password = a1;
        }
        if(!strncasecmp(dbarg,KEYWORD_DBNAME,strlen(KEYWORD_DBNAME)))
        {
            data->shared->dbname = a1;
        }
        if(!strncasecmp(dbarg,KEYWORD_SENSORNAME,strlen(KEYWORD_SENSORNAME)))
        {
            data->sensor_name = a1;
        }
        if(!strncasecmp(dbarg,KEYWORD_ENCODING,strlen(KEYWORD_ENCODING)))
        {
            if(!strncasecmp(a1, KEYWORD_ENCODING_HEX, strlen(KEYWORD_ENCODING_HEX)))
            {
                data->encoding = ENCODING_HEX;
            }
            else if(!strncasecmp(a1, KEYWORD_ENCODING_BASE64, strlen(KEYWORD_ENCODING_BASE64)))
            {
                data->encoding = ENCODING_BASE64;
            }
            else if(!strncasecmp(a1, KEYWORD_ENCODING_ASCII, strlen(KEYWORD_ENCODING_ASCII)))
            {
                data->encoding = ENCODING_ASCII;
            }
            else
            {
                FatalError("database: unknown  (%s)", a1);
            }
        }
        if(!strncasecmp(dbarg,KEYWORD_DETAIL,strlen(KEYWORD_DETAIL)))
        {
            if(!strncasecmp(a1, KEYWORD_DETAIL_FULL, strlen(KEYWORD_DETAIL_FULL)))
            {
                data->detail = DETAIL_FULL;
            }
            else if(!strncasecmp(a1, KEYWORD_DETAIL_FAST, strlen(KEYWORD_DETAIL_FAST)))
            {
                data->detail = DETAIL_FAST;
            }
            else
            {
                FatalError("database: unknown detail level (%s)", a1);
            } 
        }
        if(!strncasecmp(dbarg,KEYWORD_IGNOREBPF,strlen(KEYWORD_IGNOREBPF)))
        {
            if(!strncasecmp(a1, KEYWORD_IGNOREBPF_NO, strlen(KEYWORD_IGNOREBPF_NO)) ||
               !strncasecmp(a1, KEYWORD_IGNOREBPF_ZERO, strlen(KEYWORD_IGNOREBPF_ZERO)))
            {
                data->ignore_bpf = 0;
            }
            else if(!strncasecmp(a1, KEYWORD_IGNOREBPF_YES, strlen(KEYWORD_IGNOREBPF_YES)) ||
                    !strncasecmp(a1, KEYWORD_IGNOREBPF_ONE, strlen(KEYWORD_IGNOREBPF_ONE)))
            {
                data->ignore_bpf = 1;
            }
            else
            {
                FatalError("database: unknown ignore_bpf argument (%s)", a1);
            }

        }
        dbarg = strtok(NULL, "=");
    } 

    if(data->shared->dbname == NULL)
    {
        ErrorMessage("database: must enter database name in configuration file\n\n");
        DatabasePrintUsage();
        FatalError("\n");
    }
    else if(data->shared->host == NULL)
    {
        ErrorMessage("database: must enter host in configuration file\n\n");
        DatabasePrintUsage();
        FatalError("\n");
    }
}

static void FreeQueryNode(SQLQuery * node)
{
    if(node)
    {
        FreeQueryNode(node->next);
        node->next = NULL;
        free(node->val);
        node->val = NULL;
        free(node);
    }
}

static SQLQuery * NewQueryNode(SQLQuery * parent, int query_size)
{
    SQLQuery * rval;

    if(query_size == 0)
    {
        query_size = MAX_QUERY_LENGTH;
    }

    if(parent)
    {
        while(parent->next)
        {
            parent = parent->next;
        }

        parent->next = (SQLQuery *)SnortAlloc(sizeof(SQLQuery));
        rval = parent->next;
    }
    else
    {
        rval = (SQLQuery *)SnortAlloc(sizeof(SQLQuery));
    }

    rval->val = (char *)SnortAlloc(query_size);
    rval->next = NULL;

    return rval;
}  

/*******************************************************************************
 * Function: Database(Packet *, char * msg, void *arg)
 *
 * Purpose: Insert data into the database
 *
 * Arguments: p   => pointer to the current packet data struct 
 *            msg => pointer to the signature message
 *
 * Returns: void function
 *
 ******************************************************************************/
static void Database(Packet *p, char *msg, void *arg, Event *event)
{
    DatabaseData *data = (DatabaseData *)arg;
    SQLQuery *query = NULL,
             *root = NULL;
    char *timestamp_string = NULL,
         *insert_fields = NULL,
         *insert_values = NULL,
         *sig_name = NULL,
         *sig_class = NULL,
         *ref_system_name = NULL,
         *ref_node_id_string = NULL,
         *ref_tag = NULL,
         *packet_data = NULL,
         *packet_data_not_escaped = NULL,
         *select0 = NULL,
         *select1 = NULL,
         *insert0 = NULL;
    int i,
        insert_fields_len,
        insert_values_len,
        ok_transaction,
        ref_system_id,
        ret;
    unsigned int sig_id,
                 ref_id,
                 class_id = 0;
    ClassType *class_ptr;
    ReferenceNode *refNode;

    char sig_rev[16]="";
    char sig_sid[16]="";
    char sig_gid[16]="";

    query = NewQueryNode(NULL, 0);
    root = query;

#ifdef ENABLE_DB_TRANSACTIONS
    BeginTransaction(data);
#endif
    
    if(msg == NULL)
    {
        msg = "";
    }

    /*** Build the query for the Event Table ***/

    /* Generate a default-formatted timestamp now */
    if(p != NULL)
    {
        timestamp_string = GetTimestamp((struct timeval *) &p->pkth->ts, data->tz);
    }
    else
    {
        timestamp_string = GetCurrentTimestamp();
    }
#ifdef ENABLE_MSSQL
    if(data->shared->dbtype_id == DB_MSSQL)
    {
        /* SQL Server uses a date format which is slightly
         * different from the ISO-8601 standard generated
         * by GetTimestamp() and GetCurrentTimestamp().  We
         * need to convert from the ISO-8601 format of:
         *   "1998-01-25 23:59:59+14316557"
         * to the SQL Server format of:
         *   "1998-01-25 23:59:59.143"
         */
        if( timestamp_string!=NULL && strlen(timestamp_string)>20 )
        {
            timestamp_string[19] = '.';
        }
        if( timestamp_string!=NULL && strlen(timestamp_string)>24 )
        {
            timestamp_string[23] = '\0';
        }
    }
#endif
#ifdef ENABLE_ORACLE
    if (data->shared->dbtype_id == DB_ORACLE)
    {
        /* Oracle (everything before 9i) does not support
         * date information smaller than 1 second.
         * To go along with the TO_DATE() Oracle function
         * below, this was written to strip out all the
         * excess information. (everything beyond a second)
         * Use the Oracle format of:
         *   "1998-01-25 23:59:59"
         */
        if ( timestamp_string!=NULL && strlen(timestamp_string)>20 )
        {
            timestamp_string[19] = '\0';
        }
    }
#endif
#ifdef ENABLE_MYSQL
    if (data->shared->dbtype_id == DB_MYSQL)
    {
        /* MySql does not support date information smaller than
         * 1 second.  This was written to strip out all the
         * excess information. (everything beyond a second)
         * Use the MySql format of:
         *   "2005-12-23 22:37:16"
         */
        if ( timestamp_string!=NULL && strlen(timestamp_string)>20 )
        {
            timestamp_string[19] = '\0';
        }
    }
#endif
#ifdef ENABLE_ODBC
    if (data->shared->dbtype_id == DB_ODBC)
    {
        /* ODBC defines escape sequences for date data.
         * These escape sequences are of the format:
         *   {literal-type 'value'}
         * The Timestamp (ts) escape sequence handles
         * date/time values of the format:
         *   yyyy-mm-dd hh:mm:ss[.f...]
         * where the number of digits to the right of the
         * decimal point in a time or timestamp interval
         * literal containing a seconds component is
         * dependent on the seconds precision, as contained
         * in the SQL_DESC_PRECISION descriptor field. (For
         * more information, see function SQLSetDescField.)
         *
         * The number of decimal places within the fraction
         * of a second is database dependant.  I wasn't able
         * to easily determine the granularity of this
         * value using SQL_DESC_PRECISION, so choosing to
         * simply discard the fractional part.
         */
        if( timestamp_string!=NULL && strlen(timestamp_string)>20 )
        {
            timestamp_string[19] = '\0';
        }
    }
#endif
#ifdef ENABLE_POSTGRESQL
    if( data->shared->dbtype_id == DB_POSTGRESQL ){
        /* From Posgres Documentation
         * For timestamp with time zone, the internally stored
         * value is always in UTC (GMT). An input value that has
         * an explicit time zone specified is converted to UTC
         * using the appropriate offset for that time zone. If no
         * time zone is stated in the input string, then it is assumed
         * to be in the time zone indicated by the system's TimeZone
         * parameter, and is converted to UTC using the offset for
         * the TimeZone zone
         */
        if( timestamp_string!=NULL && strlen(timestamp_string)>24 )
        {
            timestamp_string[23] = '\0';
        }
    }
#endif

    /* Write the signature information 
     *  - Determine the ID # of the signature of this alert 
     */
    select0 = (char *) SnortAlloc(MAX_QUERY_LENGTH+1);
    sig_name = snort_escape_string(msg, data);

    if (event->sig_rev == 0)
    {
        ret = SnortSnprintf(sig_rev, sizeof(sig_rev), "IS NULL");
        
        if (ret != SNORT_SNPRINTF_SUCCESS)
            goto bad_query;
    }
    else
    {
        ret = SnortSnprintf(sig_rev, sizeof(sig_rev), "= %u", event->sig_rev);

        if (ret != SNORT_SNPRINTF_SUCCESS)
            goto bad_query;
    }

    if (event->sig_id == 0)
    {
        ret = SnortSnprintf(sig_sid, sizeof(sig_sid), "IS NULL");

        if (ret != SNORT_SNPRINTF_SUCCESS)
            goto bad_query;
    }
    else
    {
        ret = SnortSnprintf(sig_sid, sizeof(sig_sid), "= %u", event->sig_id);

        if (ret != SNORT_SNPRINTF_SUCCESS)
            goto bad_query;
    }

    if (event->sig_generator == 0)
    {
        ret = SnortSnprintf(sig_gid, sizeof(sig_gid), "IS NULL");

        if (ret != SNORT_SNPRINTF_SUCCESS)
            goto bad_query;
    }
    else
    {
        ret = SnortSnprintf(sig_gid, sizeof(sig_gid), "= %u", event->sig_generator);

        if (ret != SNORT_SNPRINTF_SUCCESS)
            goto bad_query;
    }

    ret = SnortSnprintf(select0, MAX_QUERY_LENGTH,
                        "SELECT sig_id "
                        "  FROM signature "
                        " WHERE sig_name = '%s' "
                        "   AND sig_rev %s "
                        "   AND sig_sid %s "
                        "   AND sig_gid %s ",
                        sig_name, sig_rev, sig_sid, sig_gid);

    if (ret != SNORT_SNPRINTF_SUCCESS)
        goto bad_query;

    sig_id = Select(select0, data);

    /* If this signature is detected for the first time
     *  - write the signature
     *  - write the signature's references, classification, priority, id,
     *                          revision number
     * Note: if a signature (identified with a unique text message, revision #) 
     *       initially is logged to the DB without references/classification, 
     *       but later they are added, this information will _not_ be 
     *       stored/updated unless the revision number is changed.
     *       This algorithm is used in order to prevent many DB SELECTs to
     *       verify their presence _every_ time the alert is triggered. 
     */
    if(sig_id == 0)
    {
        /* get classification and priority information  */
        if(otn_tmp)
        {
            class_ptr = otn_tmp->sigInfo.classType;

            if(class_ptr)
            {
                /* classification */
                if(class_ptr->type)
                {
                    /* Get the ID # of this classification */ 
                    select1 = (char *) SnortAlloc(MAX_QUERY_LENGTH+1);
                    sig_class = snort_escape_string(class_ptr->type, data);
            
                    ret = SnortSnprintf(select1, MAX_QUERY_LENGTH, 
                                        "SELECT sig_class_id "
                                        "  FROM sig_class "
                                        " WHERE sig_class_name = '%s'",
                                        sig_class);

                    if (ret != SNORT_SNPRINTF_SUCCESS)
                        goto bad_query;

                    class_id = Select(select1, data);

                    if ( !class_id )
                    {
                        insert0 = (char *) SnortAlloc(MAX_QUERY_LENGTH+1);
                        ret = SnortSnprintf(insert0, MAX_QUERY_LENGTH,
                                            "INSERT INTO "
                                            "sig_class (sig_class_name) "
                                            "VALUES ('%s')",
                                            sig_class);
                        if (ret != SNORT_SNPRINTF_SUCCESS)
                            goto bad_query;

                        Insert(insert0, data);

                        free(insert0);
                        insert0 = NULL;

                        class_id = Select(select1, data);
                        if ( !class_id )
                        {
                            ErrorMessage("database: unable to write classification\n");
                        }
                    }

                    free(select1);
                    select1 = NULL;

                    free(sig_class);
                    sig_class = NULL;
                }
            }
        }

        insert0 = (char *) SnortAlloc(MAX_QUERY_LENGTH+1);
        insert_fields = (char *) SnortAlloc(MAX_QUERY_LENGTH+1);
        insert_values = (char *) SnortAlloc(MAX_QUERY_LENGTH+1);
        insert_fields_len = 0;
        insert_values_len = 0;

        ret = SnortSnprintf(insert_fields, MAX_QUERY_LENGTH - insert_fields_len, "%s", "sig_name");

        if (ret != SNORT_SNPRINTF_SUCCESS)
            goto bad_query;
        
        ret = SnortSnprintf(insert_values, MAX_QUERY_LENGTH - insert_values_len, "'%s'", sig_name);

        if (ret != SNORT_SNPRINTF_SUCCESS)
            goto bad_query;
        
        insert_fields_len = strlen(insert_fields);
        insert_values_len = strlen(insert_values);

        if ( class_id > 0 )
        {
            ret = SnortSnprintf(&insert_fields[insert_fields_len], MAX_QUERY_LENGTH - insert_fields_len,
                                "%s", ",sig_class_id");

            if (ret != SNORT_SNPRINTF_SUCCESS)
                goto bad_query;
            
            ret = SnortSnprintf(&insert_values[insert_values_len], MAX_QUERY_LENGTH - insert_values_len,
                                ",%u", class_id);

            if (ret != SNORT_SNPRINTF_SUCCESS)
                goto bad_query;
            
            insert_fields_len = strlen(insert_fields);
            insert_values_len = strlen(insert_values);
        } 

        if ( event->priority > 0 )
        {
            ret = SnortSnprintf(&insert_fields[insert_fields_len], MAX_QUERY_LENGTH - insert_fields_len,
                                "%s", ",sig_priority");

            if (ret != SNORT_SNPRINTF_SUCCESS)
                goto bad_query;
            
            ret = SnortSnprintf(&insert_values[insert_values_len], MAX_QUERY_LENGTH - insert_values_len,
                                ",%u", event->priority);

            if (ret != SNORT_SNPRINTF_SUCCESS)
                goto bad_query;
            
            insert_fields_len = strlen(insert_fields);
            insert_values_len = strlen(insert_values);
        }

        if ( event->sig_rev > 0 )
        {
            ret = SnortSnprintf(&insert_fields[insert_fields_len], MAX_QUERY_LENGTH - insert_fields_len,
                                "%s", ",sig_rev");

            if (ret != SNORT_SNPRINTF_SUCCESS)
                goto bad_query;

            ret = SnortSnprintf(&insert_values[insert_values_len], MAX_QUERY_LENGTH - insert_values_len,
                                ",%u", event->sig_rev);

            if (ret != SNORT_SNPRINTF_SUCCESS)
                goto bad_query;
            
            insert_fields_len = strlen(insert_fields);
            insert_values_len = strlen(insert_values);
        }

        if ( event->sig_id > 0 )
        {
            ret = SnortSnprintf(&insert_fields[insert_fields_len], MAX_QUERY_LENGTH - insert_fields_len,
                                "%s", ",sig_sid");

            if (ret != SNORT_SNPRINTF_SUCCESS)
                goto bad_query;
            
            ret = SnortSnprintf(&insert_values[insert_values_len], MAX_QUERY_LENGTH - insert_values_len,
                                ",%u", event->sig_id);

            if (ret != SNORT_SNPRINTF_SUCCESS)
                goto bad_query;
            
            insert_fields_len = strlen(insert_fields);
            insert_values_len = strlen(insert_values);            
        }

        if ( event->sig_generator > 0 )
        {
            ret = SnortSnprintf(&insert_fields[insert_fields_len], MAX_QUERY_LENGTH - insert_fields_len,
                                "%s", ",sig_gid");

            if (ret != SNORT_SNPRINTF_SUCCESS)
                goto bad_query;
            
            ret = SnortSnprintf(&insert_values[insert_values_len], MAX_QUERY_LENGTH - insert_values_len,
                                ",%u", event->sig_generator);

            if (ret != SNORT_SNPRINTF_SUCCESS)
                goto bad_query;
            
            insert_fields_len = strlen(insert_fields);
            insert_values_len = strlen(insert_values);            
        }

        ret = SnortSnprintf(insert0, MAX_QUERY_LENGTH,
                            "INSERT INTO signature (%s) VALUES (%s)",
                            insert_fields, insert_values);

        if (ret != SNORT_SNPRINTF_SUCCESS)
            goto bad_query;
        
        Insert(insert0,data);

        sig_id = Select(select0,data);

        if(sig_id == 0)
        {
            ErrorMessage("database: Problem inserting a new signature '%s': %s\n", msg,insert0);
        }

        free(insert0);         insert0 = NULL;
        free(insert_fields);   insert_fields = NULL;
        free(insert_values);   insert_values = NULL;
        free(select0);         select0 = NULL;

        /* add the external rule references  */
        if(otn_tmp)
        {
            refNode = otn_tmp->sigInfo.refs;
            i = 1;

            while(refNode)
            {
                /* Get the ID # of the reference from the DB */
                select0 = (char *) SnortAlloc(MAX_QUERY_LENGTH+1);
                insert0 = (char *) SnortAlloc(MAX_QUERY_LENGTH+1);
                ref_system_name = snort_escape_string(refNode->system->name, data);
        
                /* Note: There is an underlying assumption that the SELECT
                 *       will do a case-insensitive comparison.
                 */
                ret = SnortSnprintf(select0, MAX_QUERY_LENGTH, 
                                    "SELECT ref_system_id "
                                    "  FROM reference_system "
                                    " WHERE ref_system_name = '%s'",
                                    ref_system_name);
                
                if (ret != SNORT_SNPRINTF_SUCCESS)
                    goto bad_query;
                
                ret = SnortSnprintf(insert0, MAX_QUERY_LENGTH,
                                    "INSERT INTO "
                                    "reference_system (ref_system_name) "
                                    "VALUES ('%s')",
                                    ref_system_name);

                if (ret != SNORT_SNPRINTF_SUCCESS)
                    goto bad_query;
                
                ref_system_id = Select(select0, data);

                if ( ref_system_id == 0 )
                {
                    Insert(insert0, data);
                    ref_system_id = Select(select0, data);
                }

                free(select0);            select0 = NULL;
                free(insert0);            insert0 = NULL;
                free(ref_system_name);    ref_system_name = NULL;

                if ( ref_system_id > 0 )
                {
                    select0 = (char *) SnortAlloc(MAX_QUERY_LENGTH+1);
                    ref_tag = snort_escape_string(refNode->id, data);

                    ret = SnortSnprintf(select0, MAX_QUERY_LENGTH,
                                        "SELECT ref_id "
                                        "  FROM reference "
                                        " WHERE ref_system_id = %d "
                                        "   AND ref_tag = '%s'",
                                        ref_system_id, ref_tag);

                    if (ret != SNORT_SNPRINTF_SUCCESS)
                        goto bad_query;
                    
                    ref_id = Select(select0, data);

                    free(ref_tag);    ref_tag = NULL;
            
                    /* If this reference is not in the database, write it */
                    if ( ref_id == 0 )
                    {
                        /* truncate the reference tag as necessary */
                        ref_node_id_string = (char *) SnortAlloc(101);

                        if ( data->DBschema_version == 103 )
                        {
                            ret = SnortSnprintf(ref_node_id_string, 20, "%s", refNode->id);

                            if (ret != SNORT_SNPRINTF_SUCCESS)
                                goto bad_query;
                        }
                        else if ( data->DBschema_version >= 104 )
                        {
                            ret = SnortSnprintf(ref_node_id_string, 100, "%s", refNode->id);

                            if (ret != SNORT_SNPRINTF_SUCCESS)
                                goto bad_query;
                        }

                        insert0 = (char *) SnortAlloc(MAX_QUERY_LENGTH+1);
                        ref_tag = snort_escape_string(ref_node_id_string, data);

                        ret = SnortSnprintf(insert0, MAX_QUERY_LENGTH,
                                            "INSERT INTO "
                                            "reference (ref_system_id, ref_tag) "
                                            "VALUES (%d, '%s')",
                                            ref_system_id, ref_tag);

                        if (ret != SNORT_SNPRINTF_SUCCESS)
                            goto bad_query;

                        Insert(insert0, data);
                        ref_id = Select(select0, data);

                        free(insert0);               insert0 = NULL;
                        free(ref_node_id_string);    ref_node_id_string = NULL;
                        free(ref_tag);               ref_tag = NULL;

                        if ( ref_id == 0 )
                        {
                            ErrorMessage("database: Unable to insert the alert reference into the DB\n");
                        }
                    }

                    free(select0);    select0 = NULL;

                    insert0 = (char *) SnortAlloc(MAX_QUERY_LENGTH+1);

                    ret = SnortSnprintf(insert0, MAX_QUERY_LENGTH,
                                        "INSERT INTO "
                                        "sig_reference (sig_id, ref_seq, ref_id) "
                                        "VALUES (%u, %d, %u)",
                                        sig_id, i, ref_id);

                    if (ret != SNORT_SNPRINTF_SUCCESS)
                        goto bad_query;
                    
                    Insert(insert0, data);

                    free(insert0);    insert0 = NULL;
                    i++;
                }
                else
                {
                    ErrorMessage("database: Unable to insert unknown reference tag ('%s') used in rule.\n", refNode->id);
                }

                refNode = refNode->next;
            }
        }
    }
    else
    {
        free(select0);    select0 = NULL;
    }

    free(sig_name);    sig_name = NULL;
    
    if ( (data->shared->dbtype_id == DB_ORACLE) &&
         (data->DBschema_version >= 105) )
    {
        ret = SnortSnprintf(query->val, MAX_QUERY_LENGTH,
                            "INSERT INTO "
                            "event (sid,cid,signature,timestamp) "
                            "VALUES (%u, %u, %u, TO_DATE('%s', 'YYYY-MM-DD HH24:MI:SS'))",
                            data->shared->sid, data->shared->cid, sig_id, timestamp_string);

        if (ret != SNORT_SNPRINTF_SUCCESS)
            goto bad_query;
    }
    else if(data->shared->dbtype_id == DB_ODBC)
    {
        ret = SnortSnprintf(query->val, MAX_QUERY_LENGTH,
                            "INSERT INTO "
                            "event (sid,cid,signature,timestamp) "
                            "VALUES (%u, %u, %u, {ts '%s'})",
                            data->shared->sid, data->shared->cid, sig_id, timestamp_string);

        if (ret != SNORT_SNPRINTF_SUCCESS)
            goto bad_query;
    }
    else
    {
        ret = SnortSnprintf(query->val, MAX_QUERY_LENGTH,
                            "INSERT INTO "
                            "event (sid,cid,signature,timestamp) "
                            "VALUES (%u, %u, %u, '%s')",
                            data->shared->sid, data->shared->cid, sig_id, timestamp_string);

        if (ret != SNORT_SNPRINTF_SUCCESS)
            goto bad_query;
    }

    free(timestamp_string);    timestamp_string = NULL;

    /* We do not log fragments! They are assumed to be handled 
       by the fragment reassembly pre-processor */

    if(p != NULL)
    {
        if((!p->frag_flag) && (IPH_IS_VALID(p))) 
        {
            /* query = NewQueryNode(query, 0); */
            if(GET_IPH_PROTO(p) == IPPROTO_ICMP && p->icmph)
            {
                query = NewQueryNode(query, 0);
                /*** Build a query for the ICMP Header ***/
                if(data->detail)
                {
                    if(p->icmph)
                    {
                        ret = SnortSnprintf(query->val, MAX_QUERY_LENGTH, 
                                            "INSERT INTO "
                                            "icmphdr (sid, cid, icmp_type, icmp_code, icmp_csum, icmp_id, icmp_seq) "
                                            "VALUES (%u,%u,%u,%u,%u,%u,%u)",
                                            data->shared->sid, data->shared->cid, p->icmph->type,
                                            p->icmph->code, ntohs(p->icmph->csum),
                                            ntohs(p->icmph->s_icmp_id), ntohs(p->icmph->s_icmp_seq));

                        if (ret != SNORT_SNPRINTF_SUCCESS)
                            goto bad_query;
                    }
                    else
                    {
                        ret = SnortSnprintf(query->val, MAX_QUERY_LENGTH, 
                                            "INSERT INTO "
                                            "icmphdr (sid, cid, icmp_type, icmp_code, icmp_csum) "
                                            "VALUES (%u,%u,%u,%u,%u)",
                                            data->shared->sid, data->shared->cid, p->icmph->type,
                                            p->icmph->code, ntohs(p->icmph->csum));

                        if (ret != SNORT_SNPRINTF_SUCCESS)
                            goto bad_query;
                    }
                }
                else
                {
                    ret = SnortSnprintf(query->val, MAX_QUERY_LENGTH, 
                                        "INSERT INTO "
                                        "icmphdr (sid, cid, icmp_type, icmp_code) "
                                        "VALUES (%u,%u,%u,%u)",
                                        data->shared->sid, data->shared->cid,
                                        p->icmph->type, p->icmph->code);

                    if (ret != SNORT_SNPRINTF_SUCCESS)
                        goto bad_query;
                }
            }
            else if(GET_IPH_PROTO(p) == IPPROTO_TCP && p->tcph)
            {
                query = NewQueryNode(query, 0);
                /*** Build a query for the TCP Header ***/
                if(data->detail)
                {
                    ret = SnortSnprintf(query->val, MAX_QUERY_LENGTH, 
                                        "INSERT INTO "
                                        "tcphdr (sid, cid, tcp_sport, tcp_dport, "
                                        "        tcp_seq, tcp_ack, tcp_off, tcp_res, "
                                        "        tcp_flags, tcp_win, tcp_csum, tcp_urp) "
                                        "VALUES (%u,%u,%u,%u,%lu,%lu,%u,%u,%u,%u,%u,%u)",
                                        data->shared->sid,
                                        data->shared->cid,
                                        ntohs(p->tcph->th_sport), 
                                        ntohs(p->tcph->th_dport),
                                        (u_long)ntohl(p->tcph->th_seq),
                                        (u_long)ntohl(p->tcph->th_ack),
                                        TCP_OFFSET(p->tcph), 
                                        TCP_X2(p->tcph),
                                        p->tcph->th_flags, 
                                        ntohs(p->tcph->th_win),
                                        ntohs(p->tcph->th_sum),
                                        ntohs(p->tcph->th_urp));

                    if (ret != SNORT_SNPRINTF_SUCCESS)
                        goto bad_query;
                }
                else
                {
                    ret = SnortSnprintf(query->val, MAX_QUERY_LENGTH, 
                                        "INSERT INTO "
                                        "tcphdr (sid,cid,tcp_sport,tcp_dport,tcp_flags) "
                                        "VALUES (%u,%u,%u,%u,%u)",
                                        data->shared->sid,
                                        data->shared->cid,
                                        ntohs(p->tcph->th_sport), 
                                        ntohs(p->tcph->th_dport),
                                        p->tcph->th_flags);

                    if (ret != SNORT_SNPRINTF_SUCCESS)
                        goto bad_query;
                }

                if(data->detail)
                {
                    /*** Build the query for TCP Options ***/
                    for(i=0; i < (int)(p->tcp_option_count); i++)
                    {
                        query = NewQueryNode(query, 0);
                        if((data->encoding == ENCODING_HEX) || (data->encoding == ENCODING_ASCII))
                        {
                            packet_data = fasthex(p->tcp_options[i].data, p->tcp_options[i].len); 
                        }
                        else
                        {
                            packet_data = base64(p->tcp_options[i].data, p->tcp_options[i].len);
                        }
                        if(data->shared->dbtype_id == DB_ORACLE)
                        {
                            /* Oracle field BLOB type case. We append unescaped
                             * opt_data data after query, which later in Insert()
                             * will be cut off and uploaded with OCIBindByPos().
                             */
                            ret = SnortSnprintf(query->val, MAX_QUERY_LENGTH, 
                                                "INSERT INTO "
                                                "opt (sid,cid,optid,opt_proto,opt_code,opt_len,opt_data) "
                                                "VALUES (%u,%u,%u,%u,%u,%u,:1)|%s",
                                                data->shared->sid,
                                                data->shared->cid,
                                                i,
                                                6,
                                                p->tcp_options[i].code,
                                                p->tcp_options[i].len,
                                                packet_data); 
                            
                            if (ret != SNORT_SNPRINTF_SUCCESS)
                                goto bad_query;

                            free(packet_data);    packet_data = NULL;
                        }
                        else
                        {
                            ret = SnortSnprintf(query->val, MAX_QUERY_LENGTH, 
                                                "INSERT INTO "
                                                "opt (sid,cid,optid,opt_proto,opt_code,opt_len,opt_data) "
                                                "VALUES (%u,%u,%u,%u,%u,%u,'%s')",
                                                data->shared->sid,
                                                data->shared->cid,
                                                i,
                                                6,
                                                p->tcp_options[i].code,
                                                p->tcp_options[i].len,
                                                packet_data); 

                            if (ret != SNORT_SNPRINTF_SUCCESS)
                                goto bad_query;

                            free(packet_data);    packet_data = NULL;
                        }
                    }
                }
            }
            else if(GET_IPH_PROTO(p) == IPPROTO_UDP && p->udph)
            {
                query = NewQueryNode(query, 0);
                /*** Build the query for the UDP Header ***/
                if(data->detail)
                {
                    ret = SnortSnprintf(query->val, MAX_QUERY_LENGTH,
                                        "INSERT INTO "
                                        "udphdr (sid, cid, udp_sport, udp_dport, udp_len, udp_csum) "
                                        "VALUES (%u, %u, %u, %u, %u, %u)",
                                        data->shared->sid,
                                        data->shared->cid,
                                        ntohs(p->udph->uh_sport), 
                                        ntohs(p->udph->uh_dport),
                                        ntohs(p->udph->uh_len),
                                        ntohs(p->udph->uh_chk));

                    if (ret != SNORT_SNPRINTF_SUCCESS)
                        goto bad_query;
                }
                else
                {
                    ret = SnortSnprintf(query->val, MAX_QUERY_LENGTH,
                                        "INSERT INTO "
                                        "udphdr (sid, cid, udp_sport, udp_dport) "
                                        "VALUES (%u, %u, %u, %u)",
                                        data->shared->sid,
                                        data->shared->cid,
                                        ntohs(p->udph->uh_sport), 
                                        ntohs(p->udph->uh_dport));

                    if (ret != SNORT_SNPRINTF_SUCCESS)
                        goto bad_query;
                }
            }
        }   

        /*** Build the query for the IP Header ***/
        if ( IPH_IS_VALID(p) && IS_IP4(p) )
        {
            query = NewQueryNode(query, 0);

            if(data->detail)
            {
                ret = SnortSnprintf(query->val, MAX_QUERY_LENGTH, 
                                    "INSERT INTO "
                                    "iphdr (sid, cid, ip_src, ip_dst, ip_ver, ip_hlen, "
                                    "       ip_tos, ip_len, ip_id, ip_flags, ip_off,"
                                    "       ip_ttl, ip_proto, ip_csum) "
                                    "VALUES (%u,%u,%lu,%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u)",
                                    data->shared->sid,
                                    data->shared->cid,
                                    (u_long)ntohl(p->iph->ip_src.s_addr), 
                                    (u_long)ntohl(p->iph->ip_dst.s_addr), 
                                    IP_VER(p->iph),
                                    IP_HLEN(p->iph), 
                                    p->iph->ip_tos,
                                    ntohs(p->iph->ip_len),
                                    ntohs(p->iph->ip_id), 
                                    p->frag_flag,
                                    ntohs(p->frag_offset),
                                    p->iph->ip_ttl, 
                                    GET_IPH_PROTO(p),
                                    ntohs(p->iph->ip_csum));

                if (ret != SNORT_SNPRINTF_SUCCESS)
                    goto bad_query;
            }
            else
            {
                ret = SnortSnprintf(query->val, MAX_QUERY_LENGTH, 
                                    "INSERT INTO "
                                    "iphdr (sid, cid, ip_src, ip_dst, ip_proto) "
                                    "VALUES (%u,%u,%lu,%lu,%u)",
                                    data->shared->sid,
                                    data->shared->cid,
                                    (u_long)ntohl(p->iph->ip_src.s_addr),
                                    (u_long)ntohl(p->iph->ip_dst.s_addr),
                                    GET_IPH_PROTO(p));

                if (ret != SNORT_SNPRINTF_SUCCESS)
                    goto bad_query;
            }

            /*** Build querys for the IP Options ***/
            if(data->detail)
            {
                for(i=0 ; i < (int)(p->ip_option_count); i++)
                {
                    if(&p->ip_options[i])
                    {
                        query = NewQueryNode(query, 0);
                        if((data->encoding == ENCODING_HEX) || (data->encoding == ENCODING_ASCII))
                        {
                            packet_data = fasthex(p->ip_options[i].data, p->ip_options[i].len); 
                        }
                        else
                        {
                            packet_data = base64(p->ip_options[i].data, p->ip_options[i].len); 
                        }

                        if(data->shared->dbtype_id == DB_ORACLE)
                        {
                            /* Oracle field BLOB type case. We append unescaped
                             * opt_data data after query, which later in Insert()
                             * will be cut off and uploaded with OCIBindByPos().
                             */
                            ret = SnortSnprintf(query->val, MAX_QUERY_LENGTH,
                                                "INSERT INTO "
                                                "opt (sid,cid,optid,opt_proto,opt_code,opt_len,opt_data) "
                                                "VALUES (%u,%u,%u,%u,%u,%u,:1)|%s",
                                                data->shared->sid,
                                                data->shared->cid,
                                                i,
                                                0,
                                                p->ip_options[i].code,
                                                p->ip_options[i].len,
                                                packet_data);

                            if (ret != SNORT_SNPRINTF_SUCCESS)
                                goto bad_query;

                            free(packet_data);    packet_data = NULL;
                        }
                        else
                        {
                            ret = SnortSnprintf(query->val, MAX_QUERY_LENGTH,
                                                "INSERT INTO "
                                                "opt (sid,cid,optid,opt_proto,opt_code,opt_len,opt_data) "
                                                "VALUES (%u,%u,%u,%u,%u,%u,'%s')",
                                                data->shared->sid,
                                                data->shared->cid,
                                                i,
                                                0,
                                                p->ip_options[i].code,
                                                p->ip_options[i].len,
                                                packet_data);

                            if (ret != SNORT_SNPRINTF_SUCCESS)
                                goto bad_query;

                            free(packet_data);    packet_data = NULL;
                        }
                    }
                }
            }
        }

        /*** Build query for the payload ***/
        if ( p->data )
        {
            if(data->detail)
            {
                if(p->dsize)
                {
                    query = NewQueryNode(query, p->dsize * 2 + MAX_QUERY_LENGTH);
                    memset(query->val, 0, p->dsize*2 + MAX_QUERY_LENGTH);
                    if(data->encoding == ENCODING_BASE64)
                    {
                        packet_data_not_escaped = base64(p->data, p->dsize);
                    }
                    else if(data->encoding == ENCODING_ASCII)
                    {
                        packet_data_not_escaped = ascii(p->data, p->dsize);
                    }
                    else
                    {
                        packet_data_not_escaped = fasthex(p->data, p->dsize);
                    }

                    packet_data = snort_escape_string(packet_data_not_escaped, data);

                    if(data->shared->dbtype_id == DB_ORACLE)
                    {
                        /* Oracle field BLOB type case. We append unescaped 
                         * packet_payload data after query, which later in Insert() 
                         * will be cut off and uploaded with OCIBindByPos().
                         */
                        ret = SnortSnprintf(query->val, (p->dsize * 2) + MAX_QUERY_LENGTH - 3,
                                            "INSERT INTO "
                                            "data (sid,cid,data_payload) "
                                            "VALUES (%u,%u,:1)|%s",
                                            data->shared->sid,
                                            data->shared->cid,
                                            packet_data_not_escaped);

                        if (ret != SNORT_SNPRINTF_SUCCESS)
                            goto bad_query;

                        free (packet_data);                packet_data = NULL;
                        free (packet_data_not_escaped);    packet_data_not_escaped = NULL;
                    }
                    else
                    {
                        ret = SnortSnprintf(query->val, (p->dsize * 2) + MAX_QUERY_LENGTH - 3,
                                            "INSERT INTO "
                                            "data (sid,cid,data_payload) "
                                            "VALUES (%u,%u,'%s')",
                                            data->shared->sid,
                                            data->shared->cid,
                                            packet_data);

                        if (ret != SNORT_SNPRINTF_SUCCESS)
                            goto bad_query;

                        free (packet_data);                packet_data = NULL;
                        free (packet_data_not_escaped);    packet_data_not_escaped = NULL;
                    }
                }
            }
        }
    }

    /* Execute the queries */
    query = root;
    ok_transaction = 1;
    while(query)
    {
        if ( Insert(query->val,data) == 0 )
        {
#ifdef ENABLE_DB_TRANSACTIONS
           RollbackTransaction(data);
#endif
           ok_transaction = 0;
           break;
        }
        else
        {
           query = query->next;
        }
    }
    FreeQueryNode(root); 
    root = NULL;

    /* Increment the cid*/
    data->shared->cid++;

#ifdef ENABLE_DB_TRANSACTIONS
    if ( ok_transaction )
    {
       CommitTransaction(data);
    }
#endif
    
    /* An ODBC bugfix */
#ifdef ENABLE_ODBC
    if(data->shared->cid == 600)
    {
        data->shared->cid = 601;
    }
#endif

    return;

bad_query:
    ErrorMessage("Database: Unable to construct query - output error or truncation\n");

    if (timestamp_string != NULL)        free(timestamp_string);
    if (insert_fields != NULL)           free(insert_fields);
    if (insert_values != NULL)           free(insert_values);
    if (sig_name != NULL)                free(sig_name);
    if (sig_class != NULL)               free(sig_class);
    if (ref_system_name != NULL)         free(ref_system_name);
    if (ref_node_id_string != NULL)      free(ref_node_id_string);
    if (ref_tag != NULL)                 free(ref_tag);
    if (packet_data != NULL)             free(packet_data);
    if (packet_data_not_escaped != NULL) free(packet_data_not_escaped);
    if (select0 != NULL)                 free(select0);
    if (select1 != NULL)                 free(select1);
    if (insert0 != NULL)                 free(insert0);

    FreeQueryNode(root);

    return;
}

/* Some of the code in this function is from the 
   mysql_real_escape_string() function distributed with mysql.

   Those portions of this function remain
   Copyright (C) 2000 MySQL AB & MySQL Finland AB & TCX DataKonsult AB

   We needed a more general case that was not MySQL specific so there
   were small modifications made to the mysql_real_escape_string() 
   function. */

static char * snort_escape_string(const char * from, DatabaseData * data)
{
    char * to;
    char * to_start;
    const char* end; 
    int from_length;

    from_length = (int)strlen(from);

    to = (char *)SnortAlloc(strlen(from) * 2 + 1);
    to_start = to;
#ifdef ENABLE_ORACLE
    if (data->shared->dbtype_id == DB_ORACLE)
    {
      for (end=from+from_length; from != end; from++)
      {
        switch(*from)
        {
          case '\'':           /*  '  -->  '' */
            *to++= '\'';
            *to++= '\'';
            break;
          case '\032':         /* Ctrl-Z (Win32 EOF)  -->  \\Z */
            *to++= '\\';       /* This gives problems on Win32 */
            *to++= 'Z';
            break;
          default:             /* copy character directly */
            *to++= *from;
        }
      }
    }
    else
#endif
#ifdef ENABLE_MSSQL
    if (data->shared->dbtype_id == DB_MSSQL)
    {
      for (end=from+from_length; from != end; from++)
      {
        switch(*from)
        {
          case '\'':           /*  '  -->  '' */      
            *to++= '\'';
            *to++= '\'';
            break;
          default:             /* copy character directly */
            *to++= *from;
        }
      }
    }
    else
#endif
/* Historically these were together in a common "else".
 * Keeping it that way until somebody complains...
 */
#if defined(ENABLE_MYSQL) || defined(ENABLE_POSTGRESQL)
    if (data->shared->dbtype_id == DB_MYSQL ||
        data->shared->dbtype_id == DB_POSTGRESQL)
    {
      for(end=from+from_length; from != end; from++)
      {
        switch(*from)
        {
          /*
           * Only need to escape '%' and '_' characters
           * when querying a SELECT...LIKE, which never
           * occurs in Snort.  Excluding these checks
           * for that reason.
          case '%':            ** %  -->  \% **
            *to++= '\\';
            *to++= '%';
            break;
          case '_':            ** _  -->  \_ **
            *to++= '\\';
            *to++= '_';
            break;
           */

          case 0:              /* NULL  -->  \\0  (probably never encountered due to strlen() above) */
            *to++= '\\';       /* Must be escaped for 'mysql' */
            *to++= '0';
            break;
          case '\n':           /* \n  -->  \\n */
            *to++= '\\';       /* Must be escaped for logs */
            *to++= 'n';
            break;
          case '\r':           /* \r  -->  \\r */
            *to++= '\\';
            *to++= 'r';
            break;
          case '\t':           /* \t  -->  \\t */
            *to++= '\\';
            *to++= 't';
            break;
          case '\\':           /* \  -->  \\ */
            *to++= '\\';
            *to++= '\\';
            break;
          case '\'':           /* '  -->  \' */
            *to++= '\\';
            *to++= '\'';
            break;
          case '"':            /* "  -->  \" */
            *to++= '\\';       /* Better safe than sorry */
            *to++= '"';
            break;
          case '\032':         /* Ctrl-Z (Win32 EOF)  -->  \\Z */
            if (data->shared->dbtype_id == DB_MYSQL)
            {
              *to++= '\\';       /* This gives problems on Win32 */
              *to++= 'Z';
            }
            else
            {
              *to++= *from; 
            }
            break;
          default:             /* copy character directly */
            *to++= *from; 
        }
      }
    }
    else
#endif
    {
      for (end=from+from_length; from != end; from++)
      {
        switch(*from)
        {
          case '\'':           /*  '  -->  '' */      
            *to++= '\'';
            *to++= '\'';
            break;
          default:             /* copy character directly */
            *to++= *from;
        }
      }
    }
    *to=0;
    return(char *)to_start;
}

/*******************************************************************************
 * Function: UpdateLastCid(DatabaseData * data, int sid, int cid)
 *
 * Purpose: Sets the last cid used for a given a sensor ID (sid), 
 *
 * Arguments: data  : database information
 *            sid   : sensor ID
 *            cid   : event ID 
 *
 * Returns: status of the update
 *
 ******************************************************************************/
static int UpdateLastCid(DatabaseData *data, int sid, int cid)
{
    char *insert0;
    int ret;

    insert0 = (char *) SnortAlloc(MAX_QUERY_LENGTH+1);
    ret = SnortSnprintf(insert0, MAX_QUERY_LENGTH,
                        "UPDATE sensor "
                        "   SET last_cid = %u "
                        " WHERE sid = %u",
                        cid, sid);

    if (ret != SNORT_SNPRINTF_SUCCESS)
    {
        free(insert0);
        return -1;
    }

    ret = Insert(insert0, data);
    free(insert0);    insert0 = NULL;
    return ret;
}

/*******************************************************************************
 * Function: GetLastCid(DatabaseData * data, int sid)
 *
 * Purpose: Returns the last cid used for a given a sensor ID (sid), 
 *
 * Arguments: data  : database information
 *            sid   : sensor ID
 *
 * Returns: last cid for a given sensor ID (sid)
 *
 ******************************************************************************/
static int GetLastCid(DatabaseData *data, int sid)
{
    char *select0;
    int tmp_cid, ret;

    select0 = (char *) SnortAlloc(MAX_QUERY_LENGTH+1);
    ret = SnortSnprintf(select0, MAX_QUERY_LENGTH,
                        "SELECT last_cid "
                        "  FROM sensor "
                        " WHERE sid = %u", sid);

    if (ret != SNORT_SNPRINTF_SUCCESS)
    {
        free(select0);
        return -1;
    }

    tmp_cid = Select(select0,data);
    free(select0);    select0 = NULL;
   
    return tmp_cid;
}

/*******************************************************************************
 * Function: CheckDBVersion(DatabaseData * data)
 *
 * Purpose: To determine the version number of the underlying DB schema
 *
 * Arguments: database information
 *
 * Returns: version number of the schema
 *
 ******************************************************************************/
static int CheckDBVersion(DatabaseData * data)
{
   char *select0;
   int schema_version;
   int ret;

   select0 = (char *) SnortAlloc(MAX_QUERY_LENGTH+1);

#if defined(ENABLE_MSSQL) || defined(ENABLE_ODBC)
   if ( data->shared->dbtype_id == DB_MSSQL ||
        (data->shared->dbtype_id==DB_ODBC && data->u_underlying_dbtype_id==DB_MSSQL) )
   {
      /* "schema" is a keyword in SQL Server, so use square brackets
       *  to indicate that we are referring to the table
       */
      ret = SnortSnprintf(select0, MAX_QUERY_LENGTH,
                          "SELECT vseq FROM [schema]");

      if (ret != SNORT_SNPRINTF_SUCCESS)
      {
          free(select0);
          return -1;
      }
   }
   else
#endif
   {
#if defined(ENABLE_MYSQL)
      if (data->shared->dbtype_id == DB_MYSQL)
      {
         /* "schema" is a keyword in MYSQL, so use `schema`
          *  to indicate that we are referring to the table
          */
         ret = SnortSnprintf(select0, MAX_QUERY_LENGTH,
                             "SELECT vseq FROM `schema`");

         if (ret != SNORT_SNPRINTF_SUCCESS)
         {
             free(select0);
             return -1;
         }
      }
      else
#endif
      {
         ret = SnortSnprintf(select0, MAX_QUERY_LENGTH,
                             "SELECT vseq FROM schema");

         if (ret != SNORT_SNPRINTF_SUCCESS)
         {
             free(select0);
             return -1;
         }
      }
   }

   schema_version = Select(select0,data);
   free(select0);    select0 = NULL;

   return schema_version;
}

/*******************************************************************************
 * Function: BeginTransaction(DatabaseData * data)
 *
 * Purpose: Database independent SQL to start a transaction
 * 
 ******************************************************************************/
static void BeginTransaction(DatabaseData * data)
{
#ifdef ENABLE_ODBC
    if ( data->shared->dbtype_id == DB_ODBC )
    {
        /* Do nothing.  ODBC will implicitly create a transaction. */
    }
    else
#endif
#ifdef ENABLE_MSSQL
    if ( data->shared->dbtype_id == DB_MSSQL )
    {
        Insert("BEGIN TRANSACTION", data);
    }
    else
#endif
#ifdef ENABLE_ORACLE
    if ( data->shared->dbtype_id == DB_ORACLE )
    {
        /* Do nothing.  Oracle will implicitly create a transaction. */
    }
    else
#endif
    {
       Insert("BEGIN", data);
    }
}

/*******************************************************************************
 * Function: CommitTransaction(DatabaseData * data)
 *
 * Purpose: Database independent SQL to commit a transaction
 * 
 ******************************************************************************/
static void CommitTransaction(DatabaseData * data)
{
#ifdef ENABLE_ODBC
    if ( data->shared->dbtype_id == DB_ODBC )
    {
        if( SQLEndTran(SQL_HANDLE_DBC, data->u_connection, SQL_COMMIT) != SQL_SUCCESS )
        {
            ODBC_SQLRETURN  ret;
            ODBC_SQLCHAR    sqlState[6];
            ODBC_SQLCHAR    msg[SQL_MAX_MESSAGE_LENGTH];
            SQLINTEGER      nativeError;
            SQLSMALLINT     errorIndex = 1;
            SQLSMALLINT     msgLen;

            while ((ret = SQLGetDiagRec( SQL_HANDLE_DBC
                                       , data->u_connection
                                       , errorIndex
                                       , sqlState
                                       , &nativeError
                                       , msg
                                       , SQL_MAX_MESSAGE_LENGTH
                                       , &msgLen)) != SQL_NO_DATA)
            {
                DEBUG_WRAP(LogMessage("database commit: %s\n", msg););
                errorIndex++;
            }
        }
    }
    else
#endif
#ifdef ENABLE_MSSQL
    if ( data->shared->dbtype_id == DB_MSSQL )
    {
       Insert("COMMIT TRANSACTION", data);
    }
    else
#endif
#ifdef ENABLE_ORACLE
    if ( data->shared->dbtype_id == DB_ORACLE )
    {
       Insert("COMMIT WORK", data);
    }
    else
#endif
    {
       Insert("COMMIT", data);
    }
}

/*******************************************************************************
 * Function: RollbackTransaction(DatabaseData * data)
 *
 * Purpose: Database independent SQL to rollback a transaction
 * 
 ******************************************************************************/
static void RollbackTransaction(DatabaseData * data)
{
#ifdef ENABLE_ODBC
    if ( data->shared->dbtype_id == DB_ODBC )
    {
        if( SQLEndTran(SQL_HANDLE_DBC, data->u_connection, SQL_ROLLBACK) != SQL_SUCCESS )
        {
            ODBC_SQLRETURN  ret;
            ODBC_SQLCHAR    sqlState[6];
            ODBC_SQLCHAR    msg[SQL_MAX_MESSAGE_LENGTH];
            SQLINTEGER      nativeError;
            SQLSMALLINT     errorIndex = 1;
            SQLSMALLINT     msgLen;

            while ((ret = SQLGetDiagRec( SQL_HANDLE_DBC
                                       , data->u_connection
                                       , errorIndex
                                       , sqlState
                                       , &nativeError
                                       , msg
                                       , SQL_MAX_MESSAGE_LENGTH
                                       , &msgLen)) != SQL_NO_DATA)
            {
                DEBUG_WRAP(LogMessage("database rollback: %s\n", msg););
                errorIndex++;
            }
        }
    }
    else
#endif
#ifdef ENABLE_MSSQL
    if ( data->shared->dbtype_id == DB_MSSQL )
    {
       Insert("ROLLBACK TRANSACTION", data);
    }
    else
#endif
#ifdef ENABLE_ORACLE
    if ( data->shared->dbtype_id == DB_ORACLE )
    {
       Insert("ROLLBACK WORK", data);
    }
    else
#endif
    {
       Insert("ROLLBACK", data);
    }
}

/*******************************************************************************
 * Function: Insert(char * query, DatabaseData * data)
 *
 * Purpose: Database independent function for SQL inserts
 * 
 * Arguments: query (An SQL insert)
 *
 * Returns: 1 if successful, 0 if fail
 *
 ******************************************************************************/
static int Insert(char * query, DatabaseData * data)
{
    int result = 0;

#ifdef ENABLE_POSTGRESQL
    if( data->shared->dbtype_id == DB_POSTGRESQL )
    {
        data->p_result = PQexec(data->p_connection,query);
        if(!(PQresultStatus(data->p_result) != PGRES_COMMAND_OK))
        {
            result = 1;
        }
        else
        {
            if(PQerrorMessage(data->p_connection)[0] != '\0')
            {
                ErrorMessage("database: postgresql_error: %s\n",
                             PQerrorMessage(data->p_connection));
            }
        } 
        PQclear(data->p_result);
    }
#endif

#ifdef ENABLE_MYSQL
    if(data->shared->dbtype_id == DB_MYSQL)
    {
        result = 1;

        if(mysql_query(data->m_sock,query) != 0)
        {
            /* Try again is case of reconnect */
            if(mysql_query(data->m_sock,query) != 0)
            {
                if(mysql_errno(data->m_sock))
                {
                    ErrorMessage("database: mysql_error: %s\nSQL=%s\n", 
                                 mysql_error(data->m_sock), query);
                }

                result = 0;
            }
        }
    }
#endif

#ifdef ENABLE_ODBC
    if(data->shared->dbtype_id == DB_ODBC)
    {
        if(SQLAllocStmt(data->u_connection, &data->u_statement) == SQL_SUCCESS)
        {
            if(SQLPrepare(data->u_statement, (ODBC_SQLCHAR *)query, SQL_NTS) == SQL_SUCCESS)
            {
                if(SQLExecute(data->u_statement) == SQL_SUCCESS)
                {
                    result = 1;
                }
                else
                {
                    ODBC_SQLRETURN  ret;
                    ODBC_SQLCHAR    sqlState[6];
                    ODBC_SQLCHAR    msg[SQL_MAX_MESSAGE_LENGTH];
                    SQLINTEGER      nativeError;
                    SQLSMALLINT     errorIndex = 1;
                    SQLSMALLINT     msgLen;

                    /* assume no error unless nativeError tells us otherwise */
                    while ((ret = SQLGetDiagRec( SQL_HANDLE_STMT
                                               , data->u_statement
                                               , errorIndex
                                               , sqlState
                                               , &nativeError
                                               , msg
                                               , SQL_MAX_MESSAGE_LENGTH
                                               , &msgLen)) != SQL_NO_DATA)
                    {
                        DEBUG_WRAP(LogMessage("database: %s\n", msg););
                        errorIndex++;
                    }
                }
            }
            SQLFreeStmt(data->u_statement, SQL_DROP);
        }
    }
#endif

#ifdef ENABLE_ORACLE
    if(data->shared->dbtype_id == DB_ORACLE)
    {
        char *blob = NULL;

        /* If BLOB type - split query to actual SQL and blob to BLOB data */  
        if(strncasecmp(query,"INSERT INTO data",16)==0 || strncasecmp(query,"INSERT INTO opt",15)==0)
        {
            if((blob=strchr(query,'|')) != NULL)
            {
                *blob='\0'; blob++;
            }
        }

        if(OCI_SUCCESS == OCIStmtPrepare(data->o_statement
                                       , data->o_error
                                       , query
                                       , strlen(query)
                                       , OCI_NTV_SYNTAX
                                       , OCI_DEFAULT))
        {
            if( blob != NULL )
            {
                OCIBindByPos(data->o_statement
                           , &data->o_bind
                           , data->o_error
                           , 1
                           , (dvoid *)blob
                           , strlen(blob)
                           , SQLT_BIN
                           , 0
                           , 0
                           , 0
                           , 0
                           , 0
                           , OCI_DEFAULT);
            }

            if(OCI_SUCCESS == OCIStmtExecute(data->o_servicecontext
                                           , data->o_statement
                                           , data->o_error
                                           , 1
                                           , 0
                                           , NULL
                                           , NULL
                                           , OCI_COMMIT_ON_SUCCESS))
            {
                result = 1;
            }
        }

        if( result != 1 )
        {
            OCIErrorGet(data->o_error
                      , 1
                      , NULL
                      , &data->o_errorcode
                      , data->o_errormsg
                      , sizeof(data->o_errormsg)
                      , OCI_HTYPE_ERROR);
            ErrorMessage("database: oracle_error: %s\n", data->o_errormsg);
            ErrorMessage("        : query: %s\n", query);
        } 
    }
#endif

#ifdef ENABLE_MSSQL
    if(data->shared->dbtype_id == DB_MSSQL)
    {
        SAVESTATEMENT(query);
        dbfreebuf(data->ms_dbproc);
        if( dbcmd(data->ms_dbproc, query) == SUCCEED )
            if( dbsqlexec(data->ms_dbproc) == SUCCEED )
                if( dbresults(data->ms_dbproc) == SUCCEED )
                {
                    while (dbnextrow(data->ms_dbproc) != NO_MORE_ROWS)
                    {
                        result = (int)data->ms_col;
                    }
                    result = 1;
                }
        CLEARSTATEMENT();
    }
#endif

#ifdef DEBUG
    if(result)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_LOG,"database(debug): (%s) executed\n", query););
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_LOG,"database(debug): (%s) failed\n", query););
    }
#endif

    return result;
}

/*******************************************************************************
 * Function: Select(char * query, DatabaeData * data)
 *
 * Purpose: Database independent function for SQL selects that 
 *          return a non zero int
 * 
 * Arguments: query (An SQL insert)
 *
 * Returns: result of query if successful, 0 if fail
 *
 ******************************************************************************/
static int Select(char * query, DatabaseData * data)
{
    int result = 0;

#ifdef ENABLE_POSTGRESQL
    if( data->shared->dbtype_id == DB_POSTGRESQL )
    {
        data->p_result = PQexec(data->p_connection,query);
        if((PQresultStatus(data->p_result) == PGRES_TUPLES_OK))
        {
            if(PQntuples(data->p_result))
            {
                if((PQntuples(data->p_result)) > 1)
                {
                    ErrorMessage("database: warning (%s) returned more than one result\n",
                                 query);
                    result = 0;
                }
                else
                {
                    result = atoi(PQgetvalue(data->p_result,0,0));
                } 
            }
        }
        if(!result)
        {
            if(PQerrorMessage(data->p_connection)[0] != '\0')
            {
                ErrorMessage("database: postgresql_error: %s\n",
                             PQerrorMessage(data->p_connection));
            }
        }
        PQclear(data->p_result);
    }
#endif

#ifdef ENABLE_MYSQL
    if(data->shared->dbtype_id == DB_MYSQL)
    {
        result = 1;

        if(mysql_query(data->m_sock,query) != 0)
        {
            /* Try again in case of reconnect */
            if(mysql_query(data->m_sock,query) != 0)
                result = 0;
        }

        if (result)
        {
            data->m_result = mysql_use_result(data->m_sock);
            if (!data->m_result)
            {
                result = 0;
            }
            else
            {
                data->m_row = mysql_fetch_row(data->m_result);
                if (data->m_row)
                {
                    if(data->m_row[0] != NULL)
                    {
                        result = atoi(data->m_row[0]);
                    }
                    else
                    {
                        result = 0;
                    }
                }
                else
                {
                    result = 0;
                }
            }

            mysql_free_result(data->m_result);
        }

        if(!result)
        {
            if(mysql_errno(data->m_sock))
            {
                ErrorMessage("database: mysql_error: %s\n", mysql_error(data->m_sock));
            }
        }
    }
#endif

#ifdef ENABLE_ODBC
    if(data->shared->dbtype_id == DB_ODBC)
    {
        if(SQLAllocStmt(data->u_connection, &data->u_statement) == SQL_SUCCESS)
            if(SQLPrepare(data->u_statement, (ODBC_SQLCHAR *)query, SQL_NTS) == SQL_SUCCESS)
                if(SQLExecute(data->u_statement) == SQL_SUCCESS)
                    if(SQLRowCount(data->u_statement, &data->u_rows) == SQL_SUCCESS)
                        if(data->u_rows)
                        {
                            if(data->u_rows > 1)
                            {
                                ErrorMessage("database: warning (%s) returned more than one result\n", query);
                                result = 0;
                            }
                            else
                            {
                                if(SQLFetch(data->u_statement) == SQL_SUCCESS)
                                    if(SQLGetData(data->u_statement,1,SQL_INTEGER,&data->u_col,
                                                  sizeof(data->u_col), NULL) == SQL_SUCCESS)
                                        result = (int)data->u_col;
                            }
                        }
    }
#endif

#ifdef ENABLE_ORACLE
    if(data->shared->dbtype_id == DB_ORACLE)
    {
        int success = 0;  /* assume it will fail */
        if(OCI_SUCCESS == OCIStmtPrepare(data->o_statement
                                       , data->o_error
                                       , query
                                       , strlen(query)
                                       , OCI_NTV_SYNTAX
                                       , OCI_DEFAULT))
        {
            if(OCI_SUCCESS == OCIDefineByPos(data->o_statement
                                           , &data->o_define
                                           , data->o_error
                                           , 1
                                           , &result
                                           , sizeof(result)
                                           , SQLT_INT
                                           , 0
                                           , 0
                                           , 0
                                           , OCI_DEFAULT))
            {
                sword status;
                status = OCIStmtExecute(data->o_servicecontext
                                               , data->o_statement
                                               , data->o_error
                                               , 1  /*0*/
                                               , 0
                                               , NULL
                                               , NULL
                                               , OCI_DEFAULT);
                if( status==OCI_SUCCESS || status==OCI_NO_DATA )
                {
                    success = 1;
                }
            }
        }
        if( ! success )
        {
            OCIErrorGet(data->o_error
                      , 1
                      , NULL
                      , &data->o_errorcode
                      , data->o_errormsg
                      , sizeof(data->o_errormsg)
                      , OCI_HTYPE_ERROR);
            ErrorMessage("database: oracle_error: %s\n", data->o_errormsg);
            ErrorMessage("        : query: %s\n", query);
        }
    }
#endif

#ifdef ENABLE_MSSQL
    if(data->shared->dbtype_id == DB_MSSQL)
    {
        SAVESTATEMENT(query);
        dbfreebuf(data->ms_dbproc);
        if( dbcmd(data->ms_dbproc, query) == SUCCEED )
            if( dbsqlexec(data->ms_dbproc) == SUCCEED )
                if( dbresults(data->ms_dbproc) == SUCCEED )
                    if( dbbind(data->ms_dbproc, 1, INTBIND, (DBINT) 0, (BYTE *) &data->ms_col) == SUCCEED )
                        while (dbnextrow(data->ms_dbproc) != NO_MORE_ROWS)
                        {
                            result = (int)data->ms_col;
                        }
        CLEARSTATEMENT();
    }
#endif

#ifdef DEBUG
    if(result)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_LOG,"database(debug): (%s) returned %u\n", query, result););
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_LOG,"database(debug): (%s) failed\n", query););
    }
#endif

    return result;
}


/*******************************************************************************
 * Function: Connect(DatabaseData * data)
 *
 * Purpose: Database independent function to initiate a database 
 *          connection
 *
 ******************************************************************************/
static void Connect(DatabaseData * data)
{
#ifdef ENABLE_POSTGRESQL
    if( data->shared->dbtype_id == DB_POSTGRESQL )
    {
        data->p_connection =
            PQsetdbLogin(data->shared->host,data->port, NULL, NULL,
                         data->shared->dbname, data->user, data->password);

        if(PQstatus(data->p_connection) == CONNECTION_BAD)
        {
            PQfinish(data->p_connection);
            FatalError("database: Connection to database '%s' failed\n", data->shared->dbname);
        }
    }
#endif

#ifdef ENABLE_MYSQL
    if(data->shared->dbtype_id == DB_MYSQL)
    {
#ifdef MYSQL_HAS_OPT_RECONNECT
        my_bool reconnect = 1;
#endif  /* MYSQL_HAS_OPT_RECONNECT */

        data->m_sock = mysql_init(NULL);
        if(data->m_sock == NULL)
        {
            FatalError("database: Connection to database '%s' failed\n", data->shared->dbname);
        }

#if defined(MYSQL_HAS_OPT_RECONNECT) && !defined(MYSQL_HAS_OPT_RECONNECT_BUG)
        /* This is necessary for MySQL versions 5.0.3 and greater where the default
         * behavior now is not to reconnect to the server.  Try to set the reconnect
         * option which is only available as of MySQL 5.0.13. */
        if (mysql_options(data->m_sock, MYSQL_OPT_RECONNECT, &reconnect) != 0)
            FatalError("database: Failed to set reconnect option: %s\n", mysql_error(data->m_sock));
#endif  /* !MYSQL_HAS_OPT_RECONNECT_BUG */

        if(mysql_real_connect(data->m_sock, data->shared->host, data->user,
                              data->password, data->shared->dbname,
                              data->port == NULL ? 0 : atoi(data->port), NULL, 0) == NULL)
        {
            if(mysql_errno(data->m_sock))
                FatalError("database: mysql_error: %s\n", mysql_error(data->m_sock));

            FatalError("database: Failed to logon to database '%s'\n", data->shared->dbname);
        }

#if defined(MYSQL_HAS_OPT_RECONNECT) && defined(MYSQL_HAS_OPT_RECONNECT_BUG)
        /* Versions 5.0.13 to 5.0.18 have a bug in that this needs to be set
         * after the call to mysql_real_connect() */
        if (mysql_options(data->m_sock, MYSQL_OPT_RECONNECT, &reconnect) != 0)
            FatalError("database: Failed to set reconnect option: %s\n", mysql_error(data->m_sock));
#endif  /* MYSQL_HAS_OPT_RECONNECT_BUG */
    }
#endif  /* ENABLE_MYSQL */

#ifdef ENABLE_ODBC
    if(data->shared->dbtype_id == DB_ODBC)
    {
        ODBC_SQLRETURN ret;

        data->u_underlying_dbtype_id = DB_UNDEFINED;

        if(!(SQLAllocEnv(&data->u_handle) == SQL_SUCCESS))
        {
            FatalError("database: unable to allocate ODBC environment\n");
        }
        if(!(SQLAllocConnect(data->u_handle, &data->u_connection) == SQL_SUCCESS))
        {
            FatalError("database: unable to allocate ODBC connection handle\n");
        }

        /* The SQL Server ODBC driver always returns SQL_SUCCESS_WITH_INFO
         * on a successful SQLConnect, SQLDriverConnect, or SQLBrowseConnect.
         * When an ODBC application calls SQLGetDiagRec after getting
         * SQL_SUCCESS_WITH_INFO, it can receive the following messages:
         * 5701 - Indicates that SQL Server put the user's context into the
         *        default database defined in the data source, or into the
         *        default database defined for the login ID used in the
         *        connection if the data source did not have a default database.
         * 5703 - Indicates the language being used on the server.
         * You can ignore messages 5701 and 5703; they are only informational.
         */
        ret = SQLConnect( data->u_connection
                        , (ODBC_SQLCHAR *)data->shared->dbname
                        , SQL_NTS
                        , (ODBC_SQLCHAR *)data->user
                        , SQL_NTS
                        , (ODBC_SQLCHAR *)data->password
                        , SQL_NTS);
        if( ret != SQL_SUCCESS )
        {
            int  encounteredFailure = 1;  /* assume there is an error */
            char odbcError[2000];
            odbcError[0] = '\0';

            if( ret == SQL_SUCCESS_WITH_INFO )
            {
                ODBC_SQLCHAR   sqlState[6];
                ODBC_SQLCHAR   msg[SQL_MAX_MESSAGE_LENGTH];
                SQLINTEGER     nativeError;
                SQLSMALLINT    errorIndex = 1;
                SQLSMALLINT    msgLen;

                /* assume no error unless nativeError tells us otherwise */
                encounteredFailure = 0;

                while ((ret = SQLGetDiagRec( SQL_HANDLE_DBC
                                           , data->u_connection
                                           , errorIndex
                                           , sqlState
                                           , &nativeError
                                           , msg
                                           , SQL_MAX_MESSAGE_LENGTH
                                           , &msgLen)) != SQL_NO_DATA)
                {
                    if( strstr((const char *)msg, "SQL Server") != NULL )
                    {
                        data->u_underlying_dbtype_id = DB_MSSQL;
                    }

                    if( nativeError!=5701 && nativeError!=5703 )
                    {
                        encounteredFailure = 1;
                        strncat(odbcError, (const char *)msg, sizeof(odbcError));
                    }
                    errorIndex++;
                }
            }
            if( encounteredFailure )
            {
                FatalError("database: ODBC unable to connect.  %s\n", odbcError);
            }
        }
    }
#endif

#ifdef ENABLE_ORACLE

    #define PRINT_ORACLE_ERR(func_name) \
     { \
         OCIErrorGet(data->o_error, 1, NULL, &data->o_errorcode, \
                     data->o_errormsg, sizeof(data->o_errormsg), OCI_HTYPE_ERROR); \
         ErrorMessage("database: Oracle_error: %s\n", data->o_errormsg); \
         FatalError("database: %s : Connection to database '%s' failed\n", \
                    func_name, data->shared->dbname); \
     }

    if(data->shared->dbtype_id == DB_ORACLE)
    {
      if (!getenv("ORACLE_HOME")) 
      {
         ErrorMessage("database : ORACLE_HOME environment variable not set\n");
      }
 
      if (!data->user || !data->password || !data->shared->dbname) 
      { 
         ErrorMessage("database: user, password and dbname required for Oracle\n");
         ErrorMessage("database: dbname must also be in tnsnames.ora\n");
      }

      if (data->shared->host) 
      {
         ErrorMessage("database: hostname not required for Oracle, use dbname\n");
         ErrorMessage("database: dbname must be in tnsnames.ora\n");
      }

      if (OCIInitialize(OCI_DEFAULT, NULL, NULL, NULL, NULL)) 
         PRINT_ORACLE_ERR("OCIInitialize");
 
      if (OCIEnvInit(&data->o_environment, OCI_DEFAULT, 0, NULL)) 
         PRINT_ORACLE_ERR("OCIEnvInit");
 
      if (OCIEnvInit(&data->o_environment, OCI_DEFAULT, 0, NULL)) 
         PRINT_ORACLE_ERR("OCIEnvInit (2)");
 
      if (OCIHandleAlloc(data->o_environment, (dvoid **)&data->o_error, OCI_HTYPE_ERROR, (size_t) 0, NULL))
         PRINT_ORACLE_ERR("OCIHandleAlloc");

      if (OCILogon(data->o_environment, data->o_error, &data->o_servicecontext,
                   data->user, strlen(data->user), data->password, strlen(data->password), 
                   data->shared->dbname, strlen(data->shared->dbname))) 
      {   
         OCIErrorGet(data->o_error, 1, NULL, &data->o_errorcode, data->o_errormsg, sizeof(data->o_errormsg), OCI_HTYPE_ERROR);
         ErrorMessage("database: oracle_error: %s\n", data->o_errormsg);
         ErrorMessage("database: Checklist: check database is listed in tnsnames.ora\n");
         ErrorMessage("database:            check tnsnames.ora readable\n");
         ErrorMessage("database:            check database accessible with sqlplus\n");
         FatalError("database: OCILogon : Connection to database '%s' failed\n", data->shared->dbname);
      }
 
      if (OCIHandleAlloc(data->o_environment, (dvoid **)&data->o_statement, OCI_HTYPE_STMT, 0, NULL))
         PRINT_ORACLE_ERR("OCIHandleAlloc (2)");
    }
#endif

#ifdef ENABLE_MSSQL
    if(data->shared->dbtype_id == DB_MSSQL)
    {
        CLEARSTATEMENT();
        dberrhandle(mssql_err_handler);
        dbmsghandle(mssql_msg_handler);

        if( dbinit() != NULL )
        {
            data->ms_login = dblogin();
            if( data->ms_login == NULL )
            {
                FatalError("database: Failed to allocate login structure\n");
            }
            /* Set up some informational values which are stored with the connection */
            DBSETLUSER (data->ms_login, data->user);
            DBSETLPWD  (data->ms_login, data->password);
            DBSETLAPP  (data->ms_login, "snort");
  
            data->ms_dbproc = dbopen(data->ms_login, data->shared->host);
            if( data->ms_dbproc == NULL )
            {
                FatalError("database: Failed to logon to host '%s'\n", data->shared->host);
            }
            else
            {
                if( dbuse( data->ms_dbproc, data->shared->dbname ) != SUCCEED )
                {
                    FatalError("database: Unable to change context to database '%s'\n", data->shared->dbname);
                }
            }
        }
        else
        {
            FatalError("database: Connection to database '%s' failed\n", data->shared->dbname);
        }
        CLEARSTATEMENT();
    }
#endif
}

/*******************************************************************************
 * Function: Disconnect(DatabaseData * data)
 *
 * Purpose: Database independent function to close a connection
 *
 ******************************************************************************/
static void Disconnect(DatabaseData * data)
{
    LogMessage("database: Closing connection to database \"%s\"\n", 
               data->shared->dbname);

    if(data)
    {
#ifdef ENABLE_POSTGRESQL
        if(data->shared->dbtype_id == DB_POSTGRESQL)
        {
            if(data->p_connection)
            {
                PQfinish(data->p_connection);
            }
        }
#endif

#ifdef ENABLE_MYSQL
        if(data->shared->dbtype_id == DB_MYSQL)
        {
            if(data->m_sock)
            {
                mysql_close(data->m_sock);
            }
        }
#endif

#ifdef ENABLE_ODBC
        if(data->shared->dbtype_id == DB_ODBC)
        {
            if(data->u_handle)
            {
                SQLDisconnect(data->u_connection); 
                SQLFreeHandle(SQL_HANDLE_ENV, data->u_handle); 
            }
        }
#endif

#ifdef ENABLE_ORACLE
        if(data->shared->dbtype_id == DB_ORACLE)
        {
            if(data->o_servicecontext)
            {
                OCILogoff(data->o_servicecontext, data->o_error);
                if(data->o_error)
                {
                    OCIHandleFree((dvoid *)data->o_error, OCI_HTYPE_ERROR);
                }
                if(data->o_statement)
                {
                    OCIHandleFree((dvoid *)data->o_statement, OCI_HTYPE_STMT);
                }
            }
        }
#endif

#ifdef ENABLE_MSSQL
        if(data->shared->dbtype_id == DB_MSSQL)
        {
            CLEARSTATEMENT();
            if( data->ms_dbproc != NULL )
            {
                dbfreelogin(data->ms_login);
                data->ms_login = NULL;
                dbclose(data->ms_dbproc);
                data->ms_dbproc = NULL;
            }
        }
#endif
    }
}

static void DatabasePrintUsage(void)
{
    puts("\nUSAGE: database plugin\n");

    puts(" output database: [log | alert], [type of database], [parameter list]\n");
    puts(" [log | alert] selects whether the plugin will use the alert or");
    puts(" log facility.\n");

    puts(" For the first argument, you must supply the type of database.");
    puts(" The possible values are mysql, postgresql, odbc, oracle and");
    puts(" mssql ");

    puts(" The parameter list consists of key value pairs. The proper");
    puts(" format is a list of key=value pairs each separated a space.\n");

    puts(" The only parameter that is absolutely necessary is \"dbname\"."); 
    puts(" All other parameters are optional but may be necessary");
    puts(" depending on how you have configured your RDBMS.\n");

    puts(" dbname - the name of the database you are connecting to\n"); 

    puts(" host - the host the RDBMS is on\n");

    puts(" port - the port number the RDBMS is listening on\n"); 

    puts(" user - connect to the database as this user\n");

    puts(" password - the password for given user\n");

    puts(" sensor_name - specify your own name for this snort sensor. If you");
    puts("        do not specify a name one will be generated automatically\n");

    puts(" encoding - specify a data encoding type (hex, base64, or ascii)\n");

    puts(" detail - specify a detail level (full or fast)\n");

    puts(" ignore_bpf - specify if you want to ignore the BPF part for a sensor\n");
    puts("              definition (yes or no, no is default)\n");

    puts(" FOR EXAMPLE:");
    puts(" The configuration I am currently using is MySQL with the database");
    puts(" name of \"snort\". The user \"snortusr@localhost\" has INSERT and SELECT");
    puts(" privileges on the \"snort\" database and does not require a password.");
    puts(" The following line enables snort to log to this database.\n");

    puts(" output database: log, mysql, dbname=snort user=snortusr host=localhost\n");
}

static void SpoDatabaseCleanExitFunction(int signal, void *arg)
{
    DatabaseData *data = (DatabaseData *)arg;

    DEBUG_WRAP(DebugMessage(DEBUG_LOG,"database(debug): entered SpoDatabaseCleanExitFunction\n"););

    if(data != NULL) 
    {
       UpdateLastCid(data, data->shared->sid, data->shared->cid-1);
       Disconnect(data); 
       free(data->args);
       free(data);
       data = NULL;
    }

    if(--instances == 0)
    {
       FreeSharedDataList();
    }
}

static void SpoDatabaseRestartFunction(int signal, void *arg)
{
    DatabaseData *data = (DatabaseData *)arg;

    DEBUG_WRAP(DebugMessage(DEBUG_LOG,"database(debug): entered SpoDatabaseRestartFunction\n"););

    if(data != NULL) 
    {
       UpdateLastCid(data, data->shared->sid, data->shared->cid-1);
       Disconnect(data);
       free(data->args);
       free(data);
       data = NULL;
    }

    if(--instances == 0)
    {
       FreeSharedDataList();
    }
}

static void FreeSharedDataList(void)
{
   SharedDatabaseDataNode *current;

   while(sharedDataList != NULL)
   { 
       current = sharedDataList;
       free(current->data);
       current->data = NULL;
       sharedDataList = current->next;
       free(current);
       current = NULL;
   }
}

#ifdef ENABLE_MSSQL
/*
 * The functions mssql_err_handler() and mssql_msg_handler() are callbacks that are registered
 * when we connect to SQL Server.  They get called whenever SQL Server issues errors or messages.
 * This should only occur whenever an error has occurred, or when the connection switches to
 * a different database within the server.
 */
static int mssql_err_handler(PDBPROCESS dbproc, int severity, int dberr, int oserr, 
                             LPCSTR dberrstr, LPCSTR oserrstr)
{
    int retval;
    ErrorMessage("database: DB-Library error:\n\t%s\n", dberrstr);

    if ( severity == EXCOMM && (oserr != DBNOERR || oserrstr) )
        ErrorMessage("database: Net-Lib error %d:  %s\n", oserr, oserrstr);
    if ( oserr != DBNOERR )
        ErrorMessage("database: Operating-system error:\n\t%s\n", oserrstr);
#ifdef ENABLE_MSSQL_DEBUG
    if( strlen(g_CurrentStatement) > 0 )
        ErrorMessage("database:  The above error was caused by the following statement:\n%s\n", g_CurrentStatement);
#endif
    if ( (dbproc == NULL) || DBDEAD(dbproc) )
        retval = INT_EXIT;
    else
        retval = INT_CANCEL;
    return(retval);
}


static int mssql_msg_handler(PDBPROCESS dbproc, DBINT msgno, int msgstate, int severity, 
                             LPCSTR msgtext, LPCSTR srvname, LPCSTR procname, DBUSMALLINT line)
{
    ErrorMessage("database: SQL Server message %ld, state %d, severity %d: \n\t%s\n",
                 msgno, msgstate, severity, msgtext);
    if ( (srvname!=NULL) && strlen(srvname)!=0 )
        ErrorMessage("Server '%s', ", srvname);
    if ( (procname!=NULL) && strlen(procname)!=0 )
        ErrorMessage("Procedure '%s', ", procname);
    if (line !=0) 
        ErrorMessage("Line %d", line);
    ErrorMessage("\n");
#ifdef ENABLE_MSSQL_DEBUG
    if( strlen(g_CurrentStatement) > 0 )
        ErrorMessage("database:  The above error was caused by the following statement:\n%s\n", g_CurrentStatement);
#endif

    return(0);
}
#endif
