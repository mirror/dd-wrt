/*-
* See the file LICENSE for redistribution information.
*
* Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
*
*/

/*
 * Administrator login module.
 */


#include "servicemgmt_setup.h"

int administrator_login()
{
	const char *progname = "administrator_login";
	AFTERSALE_DBS my_aftersale;
	int ret;
	size_t len;
	char *customer_file, *engineer_file;
	char *service_file, *temp_buf;
	const char *service_database, *customer_database, *engineer_database;
	char buf[BUFSIZ];

	customer_database = CUSTOMERDB;
	engineer_database = ENGINEERDB;
	service_database = SERVICEDB;

	/* Discard the previous databases. */
	(void)remove(customer_database);
	(void)remove(engineer_database);
	(void)remove(service_database);


	/* Initialize the AFTERSALE_DBS struct. */
	initialize_aftersaledbs(&my_aftersale);

	/* Identify the files that hold our databases. */
	set_db_filenames(&my_aftersale);

	/* Find our input files. */
	customer_file = CUSTOMER_FILE;
	engineer_file = ENGINEER_FILE;
	service_file = SERVICE_FILE;

	/* Open all databases. */
	ret = databases_setup(&my_aftersale, progname, stderr);
	if (ret) {
		fprintf(stderr, "Error opening databases.\n");
		databases_close(&my_aftersale);
		return (ret);
	}

	/* Load the customer database. */
	ret = load_customer_database(my_aftersale, customer_file);
	if (ret) {
		fprintf(stderr, "Error loading customer database.\n");
		databases_close(&my_aftersale);
		return (ret);
	}
	printf("Done loading customer database.\n");

	/* Load the engineer database. */
	ret = load_engineer_database(my_aftersale, engineer_file);
	if (ret) {
		fprintf(stderr, "Error loading engineer database.\n");
		databases_close(&my_aftersale);
		return (ret);
	}
	printf("Done loading engineer database.\n");

	/* Load the service database. */
	ret = load_service_database(my_aftersale, service_file);
	if (ret) {
		fprintf(stderr, "Error loading service database.\n");
		databases_close(&my_aftersale);
		return (ret);
	}
	printf("Done loading service database.\n");

	/*
	 * Input the appropriate password, otherwise the system
	 * will refuse to grant further operation and display 
	 * error message.
	 */
	printf("input password:");
	fflush(stdout);
	temp_buf = fgets(buf, sizeof(buf), stdin);
	len = strlen(buf);
	if (len >=1 && buf[len-1] == '\n') {
		buf[len - 1] = '\0';
		len--;
	}
	if (strcmp(buf, "admin") != 0) {
		printf("wrong password to login.\n");
	} else {
		printf("success to login.\n");

		/* Display available operations for administrator. */
		for(;;){
			printf("input number 1 for reading customer "
			    "information;\n");
			printf("input number 2 for reading engineer "
			    "information;\n");
			printf("input number 3 for reading service "
			    "information.\n");
			printf("input number 4 for adding customer "
			    "record.\n");
			printf("input number 5 for adding engineer "
			    "record.\n");
			printf("input number 6 for adding service "
			    "record.\n");
			printf("input number 7 for deleting customer "
			    "record.\n");
			printf("input number 8 for deleting engineer "
			    "record.\n");
			printf("input number 9 for deleting service "
			    "record.\n");
			printf("input number 10 for editing service "
			    "record.\n");
			printf("select operation(input \"exit\" "
			    "or \"quit\" to end):");
			fflush(stdout);
			/* Stop inserting on EOF, or "exit" or "quit". */
			if (fgets(buf, sizeof(buf), stdin) == NULL)
				break;
			len = strlen(buf);
			if (len >=1 && buf[len-1] == '\n') {
				buf[len - 1] = '\0';
				len--;
			}

			if (strcmp(buf, "exit") == 0 || strcmp(buf, "quit") == 0)
				break;
			if (len == 0)
				continue;

			/* Diaplay all customer record information. */
			if (strcmp(buf, "1") == 0) {
				ret = show_all_customer_records(&my_aftersale);
			}

			/* Diaplay all engineer record information. */
			if (strcmp(buf, "2") == 0) {
				ret = show_all_engineer_records(&my_aftersale);
			}

			/* Diaplay all service record information. */
			if (strcmp(buf, "3") == 0) {
				ret = show_all_service_record(&my_aftersale);
			}

			/* Insert record to customer database. */
			if (strcmp(buf, "4") == 0) {
				ret = insert_customer_record(my_aftersale.customer_dbp);
			}

			/* Insert record to engineer database. */
			if (strcmp(buf, "5") == 0) {
				ret = insert_engineer_record(my_aftersale.engineer_dbp);
			}

			/* Insert record to service database. */
			if (strcmp(buf, "6") == 0) {
				ret = insert_service_record(my_aftersale.service_dbp);
			}

			/* Delete customer record by giving customer ID. */
			if (strcmp(buf, "7") == 0) {
				printf("input the CustomerID to delete:");
				fflush(stdout);
				if (fgets(buf, sizeof(buf), stdin) == NULL)
					break;
				len = strlen(buf);
				if (len >=1 && buf[len-1] == '\n') {
					buf[len - 1] = '\0';
					len--;
				}
				ret = delete_customer_record(buf, 
				    my_aftersale.customer_dbp);

				printf("customer record deleted.\n");
			}

			/* Delete engineer record by giving engineer ID. */
			if (strcmp(buf, "8") == 0) {
				printf("input the EngineerID to delete:");
				fflush(stdout);
				if (fgets(buf, sizeof(buf), stdin) == NULL)
					break;
				len = strlen(buf);
				if (len >=1 && buf[len-1] == '\n') {
					buf[len - 1] = '\0';
					len--;
				}
				ret = delete_engineer_record(buf, 
				    my_aftersale.engineer_dbp);

				printf("engineer record deleted.\n");
			}

			/* Delete service record by giving service ID. */
			if (strcmp(buf, "9") == 0) {
				printf("input the ServiceID to delete:");
				fflush(stdout);
				if (fgets(buf, sizeof(buf), stdin) == NULL)
					break;
				len = strlen(buf);
				if (len >=1 && buf[len-1] == '\n') {
					buf[len - 1] = '\0';
					len--;
				}
				ret = delete_service_record(buf, 
				    my_aftersale.service_dbp);

				printf("service record deleted.\n");
			}

			/* Edit service record. */
			if (strcmp(buf, "10") == 0) {
				printf("input the ServiceID to edit:");
				fflush(stdout);
				if (fgets(buf, sizeof(buf), stdin) == NULL)
					break;
				len = strlen(buf);
				if (len >=1 && buf[len-1] == '\n') {
					buf[len - 1] = '\0';
					len--;
				}
				ret = update_service_record(&my_aftersale);
			}
		}
	}

	/* Flush customer database content to customer.txt. */
	ret = flush_customer_database(my_aftersale, customer_file);
	if (ret) {
		fprintf(stderr, "Error flushing customer database. \n");
		databases_close(&my_aftersale);
		return (ret);
	}
	printf("Done flushing customer database.\n");

	/* Flush engineer database content to engineer.txt. */
	ret = flush_engineer_database(my_aftersale, engineer_file);
	if (ret) {
		fprintf(stderr, "Error flushing engineer database.\n");
		databases_close(&my_aftersale);
		return (ret);
	}
	printf("Done flushing engineer database.\n");

	/* Flush service database content to service.txt. */
	ret = flush_service_database(my_aftersale, service_file);
	if (ret) {
		fprintf(stderr, "Error flushing customer database.\n");
		databases_close(&my_aftersale);
		return (ret);
	}
	printf("Done flushing service database.\n");
	
	/* Close our environment and databases. */
	databases_close(&my_aftersale);

	return (ret);
}

/*
 * Customer login module.
 */

int customer_login()
{
	DBC *custname_cursorp;
	DBT key, data;
	CUSTOMER my_customer;
	const char *progname = "customer_login";
	AFTERSALE_DBS my_aftersale;
	int ret = 0, i = 0;
	size_t len;
	char *customer_file, *temp_buf;
	const char *customer_database;
	char buf[BUFSIZ], CustomerName[BUFSIZ];

	customer_database = CUSTOMERDB;

	/* Discard the previous database. */
	(void)remove(customer_database);

	/* Initialize the AFTERSALE_DBS struct. */
	initialize_aftersaledbs(&my_aftersale);

	/* Identify the files that hold our databases. */
	set_db_filenames(&my_aftersale);

	/* Find our input files. */
	customer_file = CUSTOMER_FILE;

	/* Open all databases. */
	ret = databases_setup(&my_aftersale, progname, stderr);
	if (ret) {
		fprintf(stderr, "Error opening databases.\n");
		databases_close(&my_aftersale);
		return (ret);
	}

	/* Load the customer database. */
	ret = load_customer_database(my_aftersale, customer_file);
	if (ret) {
		fprintf(stderr, "Error loading customer database.\n");
		databases_close(&my_aftersale);
		return (ret);
	}

	/*
	 * Input the customer name, if the customer name doesn't exist,
	 * the system will refuse to grant further operation and display 
	 * error message.
	 */
	printf("input customer name to login:");
	fflush(stdout);
	temp_buf = fgets(buf, sizeof(buf), stdin);

	len = strlen(buf);
	for (i = 0; i<(int)len-1; i++) {
		CustomerName[i] = buf[i];
	}
	CustomerName[len-1] = '\0';


	if (ret = show_custname_record(CustomerName, 
	    my_aftersale.custname_sdbp)) {
		printf("error customer name to login.\n");
	} else {
		printf("success to login.\n");

		/* Initialize our DBTs. */
		memset(&key, 0, sizeof(DBT));
		memset(&data, 0, sizeof(DBT));

		/* Set the key to the customer's name. */
		key.data = CustomerName;
		key.size = (u_int32_t)strlen(CustomerName) + 1;

		data.data = &my_customer;
		data.ulen = sizeof(CUSTOMER);
		data.flags = DB_DBT_USERMEM;

		/* Get a cursor to the custname db. */
		my_aftersale.custname_sdbp->cursor(my_aftersale.custname_sdbp, 
		    NULL, &custname_cursorp, 0);
		
		ret = custname_cursorp->get(custname_cursorp, &key, 
		    &data, DB_SET);

		if (ret != 0) {
			my_aftersale.custname_sdbp->err(my_aftersale.custname_sdbp, 
			    ret, "Error searching");
			return (ret);
		}

		/* Display available operations for customer. */
		for(;;){
			printf("input number 1 for reading customer "
			    "information;\n");
			printf("input number 2 for editing customer "
			    "information.\n");
			printf("input number 3 for requesting a new "
			    "service.\n");
			printf("select operation(input \"exit\" or "
			    "\"quit\" to end):");
			fflush(stdout);
			/* Stop inserting on EOF, or "exit" or "quit". */
			if (fgets(buf, sizeof(buf), stdin) == NULL)
				break;
			len = strlen(buf);
			if (len >=1 && buf[len-1] == '\n') {
				buf[len - 1] = '\0';
				len--;
			}

			if (strcmp(buf, "exit") == 0 || strcmp(buf, "quit") == 0)
				break;
			if (len == 0)
				continue;

			/* 
			 * Show customer record information by giving 
			 * the customer name. 
			 */
			if (strcmp(buf, "1") == 0) {
				ret = show_custname_record(CustomerName, 
				    my_aftersale.custname_sdbp);
			}

			/* 
			 * Update customer record information by giving 
			 * the customer name. 
			 */
			if (strcmp(buf, "2") == 0) {
				ret = update_customer_record(&my_aftersale);
			}

			/* Request a new service by the customer. */
			if (strcmp(buf, "3") == 0) {
				printf("input the problem content:\n");
				fflush(stdout);
				if (fgets(buf, sizeof(buf), stdin) == NULL)
					break;
				len = strlen(buf);
				if (len >=1 && buf[len-1] == '\n') {
					buf[len - 1] = '\0';
					len--;
				}
				printf("customer %s requests a new service, "
				    "whose CustomerID is %s, \nthe problem "
				    "content is:%s\n", CustomerName, 
				    my_customer.CustomerID, buf);

				ret = insert_service_record(my_aftersale.service_dbp);
			}
		}
	}

	/* Flush the customer database. */
	ret = flush_customer_database(my_aftersale, customer_file);
	if (ret) {
		fprintf(stderr, "Error flushing customer database. \n");
		databases_close(&my_aftersale);
		return (ret);
	}
	printf("Done flushing customer database.\n");

	/* Close our environment and databases. */
	databases_close(&my_aftersale);

	return (ret);
}

/*
 * Engineer login module.
 */

int engineer_login()
{
	const char *progname = "engineer_login";
	AFTERSALE_DBS my_aftersale;
	int ret, i;
	size_t len;
	char *engineer_file, *temp_buf;
	const char *engineer_database;
	char buf[BUFSIZ], EngineerName[BUFSIZ];

	engineer_database = ENGINEERDB;

	/* Discard the previous database. */
	(void)remove(engineer_database);

	/* Initialize the AFTERSALE_DBS struct. */
	initialize_aftersaledbs(&my_aftersale);

	/* Identify the files that hold our databases. */
	set_db_filenames(&my_aftersale);

	/* Find our input files. */
	engineer_file = ENGINEER_FILE;

	/* Open all databases. */
	ret = databases_setup(&my_aftersale, progname, stderr);
	if (ret) {
		fprintf(stderr, "Error opening databases.\n");
		databases_close(&my_aftersale);
		return (ret);
	}

	/* Load the engineer database. */
	ret = load_engineer_database(my_aftersale, engineer_file);
	if (ret){
		fprintf(stderr, "Error loading engineer database.\n");
		databases_close(&my_aftersale);
		return (ret);
	}

	/*
	 * Input the engineer name, if the customer name doesn't exist,
	 * the system will refuse to grant further operation and display 
	 * error message.
	 */
	printf("input engineer name:");
	fflush(stdout);
	temp_buf = fgets(buf, sizeof(buf), stdin);

	len = strlen(buf);
	for (i = 0; i<(int)len-1; i++) {
		EngineerName[i] = buf[i];
	}
	EngineerName[len-1] = '\0';

	if (ret = show_enginame_record(EngineerName, 
	    my_aftersale.enginame_sdbp)) {
		printf("error engineer name to login.\n");
	} else {
		printf("success to login.\n");
		
		/* Display available operations for engineer. */
		for(;;){
			printf("input number 1 for reading engineer "
			    "information;\n");
			printf("input number 2 for editing engineer "
			    "information.\n");
			printf("input number 3 for submitting a new "
			    "service.\n");
			printf("select operation(input \"exit\" or "
			    "\"quit\" to end):");
			fflush(stdout);
			/* Stop inserting on EOF, or "exit" or "quit". */
			if (fgets(buf, sizeof(buf), stdin) == NULL)
				break;
			len = strlen(buf);
			if (len >=1 && buf[len-1] == '\n'){
				buf[len - 1] = '\0';
				len--;
			}

			if (strcmp(buf, "exit") == 0 || strcmp(buf, "quit") == 0)
				break;
			if (len == 0)
				continue;

			/* 
			 * Show engineer record information by giving 
			 * the engineer name. 
			 */
			if (strcmp(buf, "1") == 0) {
				ret = show_enginame_record(EngineerName, 
				    my_aftersale.enginame_sdbp);
			}

			/* 
			 * Update engineer record information by giving 
			 * the engineer name. 
			 */
			if (strcmp(buf, "2") == 0) {
				ret = update_engineer_record(&my_aftersale);
			}

			/* Submit a service by the engineer. */
			if (strcmp(buf, "3") == 0) {
				printf("submitting a new service.\n");

				printf("input the ServiceID to submit:");
				fflush(stdout);
				if (fgets(buf, sizeof(buf), stdin) == NULL)
					break;
				len = strlen(buf);
				if (len >=1 && buf[len-1] == '\n') {
					buf[len - 1] = '\0';
					len--;
				}
				ret = update_service_record(&my_aftersale);
			}
		}
	}

	/* Flush the engineer database. */
	ret = flush_engineer_database(my_aftersale, engineer_file);
	if (ret) {
		fprintf(stderr, "Error flushing engineer database.\n");
		databases_close(&my_aftersale);
		return (ret);
	}
	printf("Done flushing engineer database.\n");

	/* Close our environment and databases. */
	databases_close(&my_aftersale);

	return (ret);
}
