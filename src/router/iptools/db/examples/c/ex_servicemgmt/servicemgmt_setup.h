/*-
* See the file LICENSE for redistribution information.
*
* Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
*
*/

/* 
 * Head file for after-sale management system.
 *
 */

#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
extern int getopt(int, char * const *, const char *);
extern char *optarg;
#define snprintf _snprintf
#else
#include <unistd.h>
#endif

#define DEFAULT_HOMEDIR "./ServicemgmtExample"
#define CUSTOMER_FILE DEFAULT_HOMEDIR"/customer.txt"
#define ENGINEER_FILE DEFAULT_HOMEDIR"/engineer.txt"
#define SERVICE_FILE DEFAULT_HOMEDIR"/service.txt"
#define CUSTOMERDB "customerDB.db"
#define ENGINEERDB "engineerDB.db"
#define SERVICEDB "serviceDB.db"
#define CUSTNAMEDB "custnameDB.db"
#define ENGINAMEDB "enginameDB.db"
#define MAXDATABUF 1024
#define MAXFIELD 31
#define MAXLINE 150
#define PRIMARY_DB 0
#define SECONDARY_DB 1



typedef struct aftersale_dbs {
	DB_ENV *dbenv;		/* Database environment handle. */
	DB *customer_dbp;	/* Database containing customer information. */
	DB *engineer_dbp;	/* Database containing engineer information. */
	DB *service_dbp;	/* Database containing service information. */
	DB *custname_sdbp;	/* Secondary database based on the customer name index. */
	DB *enginame_sdbp;	/* Secondary database based on the engineer name index. */

	char *db_home_dir;	/* Directory containing the database files. */
	char *customer_db_name;		/* Name of the customer database. */
	char *engineer_db_name;		/* Name of the engineer database. */
	char *service_db_name;		/* Name of the service database. */
	char *custname_db_name;		/* Name of the secondary customer name database. */
	char *enginame_db_name;		/* Name of the secondary engineer name database. */

}AFTERSALE_DBS;

typedef struct customer {
	char CustomerID[MAXFIELD];		/* Customer identifier. */
	char Name[MAXFIELD];			/* Customer name. */
	char Telephone[13];			/* Customer telephone number. */
	char Address[MAXFIELD];			/* Contact address. */
	char Email[MAXFIELD];			/* Contact email. */
	char Fax[MAXFIELD];			/* Contact Fax. */
	char PurchasedAutomobile[MAXFIELD];	/* The automobile type customer purchased. */
	char PurchasedDate[MAXFIELD];		/* Purchased data. */
}CUSTOMER;

typedef struct engineer {
	char EngineerID[MAXFIELD];		/* Engineer identifier. */
	char Name[MAXFIELD];			/* Engineer name. */
	char Telephone[13];			/* Engineer telephone number. */
	char Email[MAXFIELD];			/* Contact email. */
	char Fax[MAXFIELD];			/* Contact fax. */
	char LengthofService[MAXFIELD];		/* Length of service by the engineer. */
	char Expertise[MAXFIELD];		/* Expertise of engineer. */
}ENGINEER;

typedef struct service {
	char ServiceID[MAXFIELD];		/* Service identifier. */
	char CustomerID[MAXFIELD];		/* Customer identifier. */
	char EngineerID[MAXFIELD];		/* Engineer identifier. */
	int ProblemLevel;	/* Problem level of the requested service. */
	char ProblemContent[MAXFIELD];		/* Problem content. */
	char RequestDate[MAXFIELD];		/* Request date. */
	char CloseDate[MAXFIELD];		/* Close date. */
	char Expense[MAXFIELD];			/* Expense for the service. */
	int SatiEvaluation;	/* Service evaluation from the customer. */
}SERVICE;

/* Function prototypes. */
int databases_setup(AFTERSALE_DBS *, const char *, FILE *);

int open_database(DB **, const char *, DB_ENV *, const char *, FILE *, int);

void initialize_aftersaledbs(AFTERSALE_DBS *);

void set_db_filenames(AFTERSALE_DBS *my_aftersale);

int databases_close(AFTERSALE_DBS *);

int load_customer_database(AFTERSALE_DBS, char *);
int load_engineer_database(AFTERSALE_DBS, char *);
int load_service_database(AFTERSALE_DBS, char *);

int show_customer_record(char *, DB *);
int show_all_customer_records(AFTERSALE_DBS *);
int show_custname_record(char [], DB *);
int show_engineer_record(char *, DB *);
int show_all_engineer_records(AFTERSALE_DBS *);
int show_enginame_record(char [], DB *);
int show_service_record(char *, AFTERSALE_DBS *);
int show_all_service_record(AFTERSALE_DBS *);

int insert_customer_record(DB *);
int insert_engineer_record(DB *);
int insert_service_record(DB *);

int flush_customer_database(AFTERSALE_DBS, char *);
int flush_engineer_database(AFTERSALE_DBS, char *);
int flush_service_database(AFTERSALE_DBS, char *);

int delete_customer_record(char *, DB *);
int delete_engineer_record(char *, DB *);
int delete_service_record(char *, DB *);

int update_customer_record(AFTERSALE_DBS *);
int update_engineer_record(AFTERSALE_DBS *);
int update_service_record(AFTERSALE_DBS *);

int customer_login();
int engineer_login();
int administrator_login();

int print_customer_record(CUSTOMER);
int print_engineer_record(ENGINEER);
int print_service_record(SERVICE);
