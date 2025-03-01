/*-
* See the file LICENSE for redistribution information.
*
* Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
*
*/

/* 
 * Database manipulation function implementation.
 *
 */

#include "servicemgmt_setup.h"

int get_customer_name(DB *, const DBT *, const DBT *, DBT *);
int get_engineer_name(DB *, const DBT *, const DBT *, DBT *);

/*
 * Used to extract the customer's name from a customer database record.
 * This function is used to create keys for secondary database records.
 */

int get_customer_name(DB *sdbp, const DBT *pkey, const DBT *pdata, DBT *skey)
{
	CUSTOMER * customer;

	customer = (CUSTOMER *)pdata->data;

	/* Set the secondary key's data to be the customer name. */
	memset(skey, 0, sizeof(DBT));
	skey->data = customer->Name;
	skey->size = (u_int32_t)strlen(customer->Name) + 1;
	return (0);
}

/* 
 * Used to extract the engineer's name from an engineer database record.
 * This function is used to create keys for secondary database records.
 */

int get_engineer_name(DB *sdbp, const DBT *pkey, const DBT *pdata, DBT *skey)
{
	ENGINEER *engineer;

	engineer = (ENGINEER *)pdata->data;

	/* Set the secondary key's data to be the engineer name. */
	memset(skey, 0, sizeof(DBT));
	skey->data = engineer->Name;
	skey->size = (u_int32_t)strlen(engineer->Name) + 1;
	return (0);
}

/* Opens a database. */
int open_database(DB **dbpp, const char * file_name,DB_ENV *dbenv, 
    const char *program_name, FILE *error_file_pointer, int is_secondary)
{
	DB *dbp;	/* Database handle. */
	u_int32_t open_flags;
	int ret;

	/* Initialize the DB handle. */
	ret=db_create(&dbp, dbenv, 0);
	if (ret != 0) {
		fprintf(error_file_pointer, "%s:%s\n", program_name, 
		    db_strerror(ret));
		return(ret);
	}
	/* Point to the memory malloc by db_create() */
	*dbpp=dbp;

	/* Set up error handling for this database. */
	dbp->set_errfile(dbp,error_file_pointer);
	dbp->set_errpfx(dbp,program_name);

	/*
	 * If this is a secondary database, then we want to allow
	 * sorted duplicate.
	 */

	if (is_secondary) {
		ret=dbp->set_flags(dbp,DB_DUPSORT);
		if(ret!=0){
			dbp->err(dbp,ret,"Attempt to set DUPSORT "
			    "flags failed.",file_name);
			return (ret);
		}
	}
	
	/* Set database pagesize. */
	if ((ret = dbp->set_pagesize(dbp,1024)) != 0) {
		dbp->err(dbp, ret, "set_pagesize");
		return (ret);
	}

	/* Set the open flags. */
	open_flags=DB_CREATE | DB_AUTO_COMMIT;

	/* Open the database using btree index method. */
	ret=dbp->open(dbp,NULL,file_name,NULL,DB_BTREE,open_flags,0);
	if(ret!=0){
		dbp->err(dbp,ret,"Database '%s' open failed.",file_name);
		return (ret);
	}
	return (0);
}

/* Initializes the AFTERSALE_DBS struct. */
void initialize_aftersaledbs(AFTERSALE_DBS *my_aftersale)
{
	my_aftersale->db_home_dir = DEFAULT_HOMEDIR;
	my_aftersale->customer_dbp = NULL;
	my_aftersale->engineer_dbp = NULL;
	my_aftersale->service_dbp = NULL;
	my_aftersale->custname_sdbp = NULL;
	my_aftersale->enginame_sdbp = NULL;
	my_aftersale->customer_db_name = NULL;
	my_aftersale->engineer_db_name = NULL;
	my_aftersale->custname_db_name = NULL;
	my_aftersale->enginame_db_name = NULL;
}

/* Identify all the files that will hold our databases. */
void set_db_filenames(AFTERSALE_DBS *my_aftersale)
{
	/* Create the Customer DB file name. */
	my_aftersale->customer_db_name = CUSTOMERDB;

	/* Create the Engineer DB file name. */
	my_aftersale->engineer_db_name = ENGINEERDB;

	/* Create the Service DB file name. */
	my_aftersale->service_db_name = SERVICEDB;

	/* Create the custname DB file name. */
	my_aftersale->custname_db_name = CUSTNAMEDB;

	/* Create the enginame DB file name. */
	my_aftersale->enginame_db_name = ENGINAMEDB;
}

/* Opens all databases in one environment. */
int databases_setup(AFTERSALE_DBS *my_aftersale, const char *program_name, 
    FILE *error_file_pointer)
{
	int ret;
	int EnvFlags = DB_CREATE | DB_INIT_LOCK | DB_INIT_LOG | 
	    DB_INIT_MPOOL | DB_INIT_TXN;

	/* Initialize a new environment. */
	if ((ret = db_env_create(&(my_aftersale->dbenv), 0)) !=0) {
		fprintf(error_file_pointer, "%s: %s\n", 
		    program_name,db_strerror(ret));
		return (1);
	}
	
	/* Set up error handling for this database. */
	my_aftersale->dbenv->set_errfile(my_aftersale->dbenv, 
	    error_file_pointer);
	my_aftersale->dbenv->set_errpfx(my_aftersale->dbenv, 
	    program_name);

	/* Set environment cachesize. */
	if ((ret = my_aftersale->dbenv->set_cachesize(my_aftersale->dbenv, 
	    0, 64*1024, 0)) !=0) {
		my_aftersale->dbenv->err(my_aftersale->dbenv, ret, 
		    "set_cachesize");
		my_aftersale->dbenv->close(my_aftersale->dbenv, 0);
		return (1);
	}
	
	/* Open environment for further operation. */
	if ((ret = my_aftersale->dbenv->open(my_aftersale->dbenv, 
	    my_aftersale->db_home_dir,  EnvFlags, 0644)) != 0) {
		my_aftersale->dbenv->err(my_aftersale->dbenv, ret, 
		    "environment open: %s",my_aftersale->db_home_dir);
		my_aftersale->dbenv->close(my_aftersale->dbenv, 0);
		return (1);
	}

	/* Open the customer database. */
	ret = open_database(&(my_aftersale->customer_dbp), 
	    my_aftersale->customer_db_name, my_aftersale->dbenv, 
	    program_name, error_file_pointer, PRIMARY_DB);
	if (ret!= 0)
		return (ret);

	/* Open the engineer database. */
	ret = open_database(&(my_aftersale->engineer_dbp), 
	    my_aftersale->engineer_db_name, my_aftersale->dbenv, 
	    program_name, error_file_pointer, PRIMARY_DB);
	if (ret != 0)
		return (ret);

	/* Open the service database. */
	ret = open_database(&(my_aftersale->service_dbp), 
	    my_aftersale->service_db_name, my_aftersale->dbenv, 
	    program_name, error_file_pointer, PRIMARY_DB);
	if (ret != 0)
		return (ret);

	/* 
	 * Open the customer name secondary database. This is used
	 * to index the customer name in the customer database. 
	 */
	ret = open_database(&(my_aftersale->custname_sdbp), 
	    my_aftersale->custname_db_name, my_aftersale->dbenv, 
	    program_name, error_file_pointer, SECONDARY_DB);
	if (ret != 0)
		return (ret);

	/* 
	 * Open the engineer name secondary database. This is used 
	 * to index the engineer name in the engineer database. 
	 */
	ret = open_database(&(my_aftersale->enginame_sdbp), 
	    my_aftersale->enginame_db_name, my_aftersale->dbenv, 
	    program_name, error_file_pointer, SECONDARY_DB);
	if (ret != 0)
		return (ret);

	/* Associate the custname db with its primary db(customer db). */
	my_aftersale->customer_dbp->associate(my_aftersale->customer_dbp, 
	    NULL, my_aftersale->custname_sdbp, get_customer_name, 0);

	/* Associate the enginame db with its primary db(engineer db). */
	my_aftersale->engineer_dbp->associate(my_aftersale->engineer_dbp, 
	    NULL, my_aftersale->enginame_sdbp, get_engineer_name, 0);

	printf("databases opened successfully\n");
	return (0);
}

/* Closes all the databases and secondary databases. */
int databases_close(AFTERSALE_DBS *my_aftersale)
{
	int t_ret, ret;
	ret = t_ret = 0;

	/*
	 * Note that closing a database automatically flushes its cached data
	 * to disk, so no sync is required here.
	 */

	/* Close customer name secondary database. */
	if (my_aftersale->custname_sdbp != NULL) {
		t_ret = my_aftersale->custname_sdbp->close(my_aftersale->custname_sdbp, 0);
		if (t_ret != 0){
			fprintf(stderr, "Customer secondary database close "
			    "failed: %s\n", db_strerror(t_ret));
			if (ret == 0)
				ret = t_ret;
		}
	}

	/* Close engineer name secondary database. */
	if (my_aftersale->enginame_sdbp != NULL) {
		t_ret = my_aftersale->enginame_sdbp->close(my_aftersale->enginame_sdbp, 0);
		if (t_ret != 0){
			fprintf(stderr, "Engineer secondary database close "
			    "failed: %s\n", db_strerror(t_ret));
			if (ret == 0)
				ret = t_ret;
		}
	}

	/* Close customer database. */
	if (my_aftersale->customer_dbp != NULL) {
		t_ret=my_aftersale->customer_dbp->close(my_aftersale->customer_dbp,0);
		if (t_ret != 0) {
			fprintf(stderr,"Customer database close failed:%s\n", 
			    db_strerror(t_ret));
			if (ret == 0)
				ret = t_ret;
		}
	}

	/* Close engineer database. */
	if (my_aftersale->engineer_dbp != NULL) {
		t_ret = my_aftersale->engineer_dbp->close(my_aftersale->engineer_dbp, 0);
		if (t_ret != 0) {
			fprintf(stderr, "Engineer database close failed:%s\n", 
			    db_strerror(t_ret));
			if (ret == 0)
				ret = t_ret;
		}
	}

	/* Close service database. */
	if (my_aftersale->service_dbp != NULL) {
		t_ret = my_aftersale->service_dbp->close(my_aftersale->service_dbp, 0);
		if (t_ret != 0) {
			fprintf(stderr, "Service database close failed:%s\n", 
			    db_strerror(t_ret));
			if (ret == 0)
				ret = t_ret;
		}
	}

	/* Lastly, close environment that holds all the databases. */
	if (my_aftersale->dbenv != NULL) {
		t_ret = my_aftersale->dbenv->close(my_aftersale->dbenv, 0);
		if (t_ret != 0) {
			fprintf(stderr,"Environment close failed:%s\n", 
			    db_strerror(t_ret));
			if (ret == 0)
				ret = t_ret;
		}
	}

	if (ret == 0)
		printf("Databases and environment closed.\n");
	return ret;
}
