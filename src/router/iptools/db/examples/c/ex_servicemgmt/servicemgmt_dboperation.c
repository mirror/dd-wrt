/*-
* See the file LICENSE for redistribution information.
*
* Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
*
*/

/*
 * Load the contents of customer.txt, engineer.txt and service.txt into 
 * Berkeley DB databases. Also causes the customer name and engineer name
 * secondary database to be created and loaded.
 *
 * Loads the contents of the customer.txt file into a database.Note that
 * because the custname secondary database is associated to the 
 * customer db, the custname index is automatically created when 
 * this database is loaded.
 */

#include "servicemgmt_setup.h"

int load_customer_database(AFTERSALE_DBS my_aftersale, char *customer_file)
{
	DBT key, data;
	char buf[MAXLINE];
	FILE *ifp;
	CUSTOMER my_customer;

	/* Load the customers database. */
	ifp = fopen(customer_file, "r");
	if(ifp == NULL) {
		fprintf(stderr,"Error opening file '%s'\n", customer_file);
		return (-1);
	}

	/* Iterate over the customer file. */
	while (fgets(buf, MAXLINE, ifp) != NULL) {
		/* Zero out the structure. */
		memset(&my_customer, 0, sizeof(CUSTOMER));
		/* Zero out the DBTs. */
		memset(&key, 0, sizeof(DBT));
		memset(&data, 0, sizeof(DBT));

		/* Scan the line into the structure. */
		sscanf(buf, "%30[^#]#%30[^#]#%13[^#]#%30"
		    "[^#]#%30[^#]#%30[^#]#%30[^#]#%30[^\n]", 
			my_customer.CustomerID, my_customer.Name, 
			my_customer.Telephone, my_customer.Address, 
			my_customer.Email, my_customer.Fax, 
			my_customer.PurchasedAutomobile,
			my_customer.PurchasedDate);

		/* 
		 * Now that we have our structure we can 
		 * load it into the database.
		 */

		/* Set up the database record's key. */
		key.data = my_customer.CustomerID;
		key.size = (u_int32_t)strlen(my_customer.CustomerID) + 1;

		/* Set up the database record's data. */
		data.data = &my_customer;
		data.size = sizeof(CUSTOMER);

		/* Put the data into the database. */
		my_aftersale.customer_dbp->put(my_aftersale.customer_dbp, 
		    0, &key, &data, 0);
	}

	/* Close the customer.txt file. */
	fclose(ifp);
	return (0);
}

/*
 * Loads the contents of engineer.txt file into a database. Note that
 * because the enginame secondary database is associated to the 
 * engineer db, the enginame index is automatically created when 
 * this database is loaded.
 */
int load_engineer_database(AFTERSALE_DBS my_aftersale, char *engineer_file)
{
	DBT key, data;
	char buf[MAXLINE];
	FILE *ifp;
	ENGINEER my_engineer;

	/* Load the engineers database. */
	ifp = fopen(engineer_file, "r");
	if(ifp == NULL) {
		fprintf(stderr, "Error opening file '%s' \n", engineer_file);
		return (-1);
	}

	/* Iterate over the engineer file. */
	while (fgets(buf, MAXLINE, ifp) != NULL) {
		/* Zero out the structure. */
		memset(&my_engineer, 0, sizeof(ENGINEER));
		/* Zero out the DBTs. */
		memset(&key, 0, sizeof(DBT));
		memset(&data, 0, sizeof(DBT));

		/* Scan the line into the structure. */
		sscanf(buf, "%30[^#]#%30[^#]#%13[^#]#%30"
		    "[^#]#%30[^#]#%30[^#]#%30[^\n]",
			my_engineer.EngineerID, my_engineer.Name, 
			my_engineer.Telephone, my_engineer.Email, 
			my_engineer.Fax, my_engineer.LengthofService,
			my_engineer.Expertise);

		/* 
		 * Now that we have our structure we can load it 
		 * into the database. 
		 */		

		/* Set up the database record's key. */
		key.data = my_engineer.EngineerID;
		key.size = (u_int32_t)strlen(my_engineer.EngineerID) + 1;

		/* Set up the database record's data. */
		data.data = &my_engineer;
		data.size = sizeof(ENGINEER);

		/* Put the data into the database. */
		my_aftersale.engineer_dbp->put(my_aftersale.engineer_dbp, 
		    0, &key, &data, 0);
	}

	/* Close the engineer.txt file. */
	fclose(ifp);
	return (0);
}

/*
 * Loads the contents of the service.txt file into a database.
 */
int load_service_database(AFTERSALE_DBS my_aftersale, char *service_file)
{
	DBT key, data;
	char buf[MAXLINE];
	FILE *ifp;
	SERVICE my_service;
	char String1[MAXFIELD], String2[MAXFIELD];

	/* Load the service database. */
	ifp = fopen(service_file, "r");
	if (ifp == NULL) {
		fprintf(stderr, "Error opening file '%s'\n", service_file);
		return (-1);
	}

	/* Iterate over the engineer file. */
	while (fgets(buf, MAXLINE, ifp) != NULL) {
		/* Zero out the structure. */
		memset(&my_service, 0, sizeof(SERVICE));
		/* Zero out the DBTs. */
		memset(&key, 0, sizeof(DBT));
		memset(&data, 0, sizeof(DBT));

		/* Scan the line into the structure. */
		sscanf(buf, "%30[^#]#%30[^#]#%30[^#]#%13"
		    "[^#]#%30[^#]#%30[^#]#%30[^#]#%30[^#]#%30[^\n]", 
			my_service.ServiceID, my_service.CustomerID, 
			my_service.EngineerID, String2, 
			my_service.ProblemContent, my_service.RequestDate,
			my_service.CloseDate, my_service.Expense, String1);

		my_service.SatiEvaluation = atoi(String1);
		my_service.ProblemLevel = atoi(String2);

		/* 
		 * Now that we have our structure we can load it 
		 * into the database. 
		 */

		/* Set up the database record's key. */
		key.data = my_service.ServiceID;
		key.size = (u_int32_t)strlen(my_service.ServiceID) + 1;

		/* Set up the database record's data. */
		data.data = &my_service;
		data.size = sizeof(SERVICE);

		/* Put the data into the database. */
		my_aftersale.service_dbp->put(my_aftersale.service_dbp, 
		    0, &key, &data, 0);
	}

	/* Close the service.txt file. */
	fclose(ifp);
	return (0);
}

/*
 * Search for and display database records.
 */

int show_all_customer_records(AFTERSALE_DBS *my_aftersale)
{
	DBC *customer_cursorp;
	DBT key, data;
	int exit_value, ret;
	CUSTOMER my_customer;

	/* Initialize our DBTs. */
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));

	data.data = &my_customer;
	data.ulen = sizeof(CUSTOMER);
	data.flags = DB_DBT_USERMEM;

	/* Get a cursor to the customer db. */
	my_aftersale->customer_dbp->cursor(my_aftersale->customer_dbp, 
	    NULL, &customer_cursorp, 0);

	/*
	 * Iterate over the customer database, from the first record to 
	 * the last, display each in turn.
	 */
	exit_value = 0;
	while ((ret = customer_cursorp->get(customer_cursorp, &key, 
	    &data, DB_NEXT)) == 0) {
		ret = print_customer_record(my_customer);
	}

	/* Close the cursor. */
	customer_cursorp->close(customer_cursorp);
	return (exit_value);
}

/*
 * Search for a customer record given its CustomerID and 
 * display that record.
 */
int show_customer_record(char *CustomerID, DB *customer_dbp)
{
	DBT key, data;
	CUSTOMER my_customer;
	int ret;

	/* Initialize our DBTs. */
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));

	/* Set the search key to the customer's ID. */
	key.data = CustomerID;
	key.size = (u_int32_t)strlen(CustomerID) + 1;

	data.data = &my_customer;
	data.ulen = sizeof(CUSTOMER);
	data.flags = DB_DBT_USERMEM;

	/* Get the record. */
	ret = customer_dbp->get(customer_dbp, NULL, &key, &data, 0);
	if (ret != 0) {
		customer_dbp->err(customer_dbp, ret, "Error searching");
		return (ret);
	} else {
		ret = print_customer_record(my_customer);
	}
	return (0);
}

/*
 * Search for a customer record given its name using the 
 * custname secondary database and display that record 
 * and any duplicates that may exist.
 */
int show_custname_record(char CustomerName[], DB *custname_sdbp)
{
	DBC *custname_cursorp;
	DBT key, data;
	CUSTOMER my_customer;
	int ret;

	/* Initialize our DBTs. */
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));

	/* Set the search key to the customer's name. */
	key.data = CustomerName;
	key.size = (u_int32_t)strlen(CustomerName) + 1;

	data.data = &my_customer;
	data.ulen = sizeof(CUSTOMER);
	data.flags = DB_DBT_USERMEM;

	/* Get a cursor to the custname db. */
	custname_sdbp->cursor(custname_sdbp, NULL, &custname_cursorp, 0);

	/* Get the record. */
	ret = custname_cursorp->get(custname_cursorp, &key, &data, DB_SET);

	if (ret != 0) {
		custname_sdbp->err(custname_sdbp, ret, "Error searching");
		return (ret);
	} else {
		ret = print_customer_record(my_customer);
	}

	/* Close the cursor. */
	custname_cursorp->close(custname_cursorp);
	return (0);
}

int show_all_engineer_records(AFTERSALE_DBS *my_aftersale)
{
	DBC *engineer_cursorp;
	DBT key, data;
	int exit_value, ret;
	ENGINEER my_engineer;

	/* Initialize our DBTs. */
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));

	data.data = &my_engineer;
	data.ulen = sizeof(ENGINEER);
	data.flags = DB_DBT_USERMEM;

	/* Get a cursor to the engineer db. */
	my_aftersale->engineer_dbp->cursor(my_aftersale->engineer_dbp, 
	    NULL, &engineer_cursorp, 0);

	/*
	 * Iterate over the engineer database, from the first record to 
	 * the last, display each in turn.
	 */
	exit_value = 0;
	while ((ret = engineer_cursorp->get(engineer_cursorp, &key, 
	    &data, DB_NEXT)) == 0) {
		ret = print_engineer_record(my_engineer);
	}

	/* Close the cursor. */
	engineer_cursorp->close(engineer_cursorp);
	return (exit_value);
}

/*
 * Search for an engineer record given its EngineerID and 
 * display that record.
 */
int show_engineer_record(char *EngineerID, DB *engineer_dbp)
{
	DBT key, data;
	ENGINEER my_engineer;
	int ret;

	/* Initialize our DBTs. */
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));

	/* Set the search key to the engineer's ID. */
	key.data = EngineerID;
	key.size = (u_int32_t)strlen(EngineerID) + 1;

	data.data = &my_engineer;
	data.ulen = sizeof(ENGINEER);
	data.flags = DB_DBT_USERMEM;

	/* Get the record. */
	ret = engineer_dbp->get(engineer_dbp, NULL, &key, &data, 0);
	if (ret != 0) {
		engineer_dbp->err(engineer_dbp, ret, "Error searching");
		return (ret);
	} else {
		ret = print_engineer_record(my_engineer);
	}
	return (0);
}

/*
 * Search for an engineer record given its name using the 
 * enginame secondary database and display that record 
 * and any duplicates that may exist.
 */
int show_enginame_record(char EngineerName[], DB *enginame_sdbp)
{
	DBC *enginame_cursorp;
	DBT key, data;
	ENGINEER my_engineer;
	int ret;

	/* Initialize our DBTs. */
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));

	/* Set the search key to the engineer's name. */
	key.data = EngineerName;
	key.size = (u_int32_t)strlen(EngineerName) + 1;

	data.data = &my_engineer;
	data.ulen = sizeof(ENGINEER);
	data.flags = DB_DBT_USERMEM;

	/* Get a cursor to the enginame db. */
	enginame_sdbp->cursor(enginame_sdbp, NULL, &enginame_cursorp, 0);

	ret = enginame_cursorp->get(enginame_cursorp, &key, &data, DB_SET);

	if (ret != 0){
		enginame_sdbp->err(enginame_sdbp, ret, "Error searching");
		return (ret);
	} else {
		ret = print_engineer_record(my_engineer);
	}

	/* Close the cursor. */
	enginame_cursorp->close(enginame_cursorp);
	return (0);
}

int show_all_service_record(AFTERSALE_DBS *my_aftersale)
{
	DBC *service_cursorp;
	DBT key, data;
	int exit_value, ret;
	SERVICE my_service;

	/* Initialize our DBTs. */
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));

	data.data = &my_service;
	data.ulen = sizeof(SERVICE);
	data.flags = DB_DBT_USERMEM;

	/* Get a cursor to the service db. */
	my_aftersale->service_dbp->cursor(my_aftersale->service_dbp, NULL, 
	    &service_cursorp, 0);

	/*
	 * Iterate over the service database, from the first record to 
	 * the last, display each in turn.
	 */
	exit_value = 0;
	while ((ret = service_cursorp->get(service_cursorp, &key, &data, 
	    DB_NEXT)) == 0) {
		ret = print_service_record(my_service);

		/* Print the corresponding customer information. */
		printf("\t the customer who requests the service:\n");
		ret = show_customer_record(my_service.CustomerID, 
		    my_aftersale->customer_dbp);

		/* Print the corresponding engineer information. */
		printf("\t the engineer allocated to handle the service:\n");
		ret = show_engineer_record(my_service.EngineerID, 
		    my_aftersale->engineer_dbp);
	}

	/* Close the cursor. */
	service_cursorp->close(service_cursorp);
	return (exit_value);
}

/*
 * Search for a service record given its ServiceID and 
 * display that record.
 */
int show_service_record(char *ServiceID, AFTERSALE_DBS *my_aftersale)
{
	DBT key, data;
	SERVICE my_service;
	int ret;

	/* Initialize our DBTs. */
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));

	/* Set the search key to the service's ID. */
	key.data = ServiceID;
	key.size = (u_int32_t)strlen(ServiceID) + 1;

	data.data = &my_service;
	data.ulen = sizeof(SERVICE);
	data.flags = DB_DBT_USERMEM;

	/* Get the record. */
	ret = my_aftersale->service_dbp->get(my_aftersale->service_dbp, NULL, 
	    &key, &data, 0);
	if (ret != 0) {
		my_aftersale->service_dbp->err(my_aftersale->service_dbp, ret, 
		    "Error searching");
		return (ret);
	} else {
		ret = print_service_record(my_service);

		/* Print the corresponding customer information. */
		printf("\t the customer who requests the service:\n");
		ret = show_customer_record(my_service.CustomerID, 
		    my_aftersale->customer_dbp);

		/* Print the corresponding engineer information. */
		printf("\t the engineer allocated to handle the service:\n");
		ret = show_engineer_record(my_service.EngineerID, 
		    my_aftersale->engineer_dbp);
	}
	return (0);
}

int print_customer_record(CUSTOMER my_customer)
{
	printf("\tCustomer Record:\n");
	printf("\t\tCustomerID: %s\n", my_customer.CustomerID);
	printf("\t\tName: %s\n", my_customer.Name);
	printf("\t\tTelephone: %s\n", my_customer.Telephone);
	printf("\t\tAddress: %s\n", my_customer.Address);
	printf("\t\tEmail: %s\n", my_customer.Email);
	printf("\t\tFax: %s\n", my_customer.Fax);
	printf("\t\tPurchasedAutomobile: %s\n", 
	    my_customer.PurchasedAutomobile);
	printf("\t\tPurchasedDate: %s\n", my_customer.PurchasedDate);
	printf("\n");
	return (0);
}

int print_engineer_record(ENGINEER my_engineer)
{
	printf("\tEngineer Record:\n");
	printf("\t\tEngineerID: %s\n", my_engineer.EngineerID);
	printf("\t\tName: %s\n", my_engineer.Name);
	printf("\t\tTelephone: %s\n", my_engineer.Telephone);
	printf("\t\tEmail: %s\n", my_engineer.Email);
	printf("\t\tFax: %s\n", my_engineer.Fax);
	printf("\t\tLengthofService: %s\n", 
	    my_engineer.LengthofService);
	printf("\t\tExpertise: %s\n", my_engineer.Expertise);
	printf("\n");
	return (0);
}

int print_service_record(SERVICE my_service)
{
	printf("\tService Record:\n");
	printf("\t\tServiceID: %s\n", my_service.ServiceID);
	printf("\t\tCustomerID: %s\n", my_service.CustomerID);
	printf("\t\tEngineerID: %s\n", my_service.EngineerID);
	printf("\t\tProblemLevel: %d\n", my_service.ProblemLevel);
	printf("\t\tProblemContent: %s\n", my_service.ProblemContent);
	printf("\t\tRequestDate: %s\n", my_service.RequestDate);
	printf("\t\tCloseDate: %s\n", my_service.CloseDate);
	printf("\t\tExpense: %s\n", my_service.Expense);
	printf("\t\tSatiEvaluation: %d\n", my_service.SatiEvaluation);
	printf("\n");
	return (0);
}

/*
 * Write records to databases. 
 *
 * Insert records to customer database.
 */
int insert_customer_record(DB *customer_dbp)
{
	DBT key, data;
	CUSTOMER my_customer;
	int ret;
	size_t len;
	char buf[BUFSIZ];
	
	/*
	 * Insert records into the database, where 
	 * the key is the customer ID and the data is
	 * the CUSTOMER structure.
	 */
	for(;;){
		printf("input customer record(input \"exit\" or \"quit\" to "
		    "end)>\n");
		fflush(stdout);
		/* Stop inserting on EOF, or "exit" or "quit". */
		if (fgets(buf, sizeof(buf), stdin) == NULL)
			break;
		len = strlen(buf);
		if (len >= 1 && buf[len-1] == '\n') {
			buf[len - 1] = '\0';
			len--;
		}

		if (strcmp(buf, "exit") == 0 || strcmp(buf, "quit") == 0)
			break;
		if (len == 0)
			continue;

		memset(&my_customer, 0, sizeof(CUSTOMER));
		memset(&key, 0, sizeof(DBT));
		memset(&data, 0, sizeof(DBT));

		/* Scan the line into the structure. */
		sscanf(buf, "%30[^#]#%30[^#]#%13[^#]#%30"
		    "[^#]#%30[^#]#%30[^#]#%30[^#]#%30[^\n]", 
			my_customer.CustomerID, my_customer.Name, 
			my_customer.Telephone, my_customer.Address, 
			my_customer.Email, my_customer.Fax, 
			my_customer.PurchasedAutomobile, 
			my_customer.PurchasedDate);

		/* Set up the database record's key. */
		key.data = my_customer.CustomerID;
		key.size = (u_int32_t)strlen(my_customer.CustomerID) + 1;

		/* Set up the database record's data. */
		data.data = &my_customer;
		data.size = sizeof(CUSTOMER);

		/* Put the data into the database. */
		ret = customer_dbp->put(customer_dbp, NULL, &key, 
		    &data, DB_NOOVERWRITE);

		if (ret == DB_KEYEXIST) {
			customer_dbp->err(customer_dbp, ret,
			    "Put failed because key %f already exists", 
			    my_customer.CustomerID);
			break;
		} else
			printf("customer record inserted.\n");
	}
	return (0);
}

/*
 * Insert records to engineer database.
 */
int insert_engineer_record(DB *engineer_dbp)
{
	DBT key, data;
	ENGINEER my_engineer;
	int ret;
	size_t len;
	char buf[BUFSIZ];

	/*
	 * Insert records into the database, where 
	 * the key is the engineer ID and the data is
	 * the ENGINEER structure.
	 */
	for(;;){
		printf("input engineer record(input \"exit\" or \"quit\" "
		    "to end)>\n");
		fflush(stdout);
		/* Stop inserting on EOF, or "exit" or "quit". */
		if (fgets(buf, sizeof(buf), stdin) == NULL)
			break;
		len = strlen(buf);
		if (len >= 1 && buf[len-1] == '\n') {
			buf[len - 1] = '\0';
			len--;
		}

		if (strcmp(buf, "exit") == 0 || strcmp(buf, "quit") == 0)
			break;
		if (len == 0)
			continue;

		memset(&my_engineer, 0, sizeof(ENGINEER));
		memset(&key, 0, sizeof(DBT));
		memset(&data, 0, sizeof(DBT));

		/* Scan the line into the structure. */
		sscanf(buf, "%30[^#]#%30[^#]#%13[^#]#%30"
		    "[^#]#%30[^#]#%30[^#]#%30[^\n]",
			my_engineer.EngineerID, my_engineer.Name, 
			my_engineer.Telephone, my_engineer.Email, 
			my_engineer.Fax, my_engineer.LengthofService,
			my_engineer.Expertise);

		/* Set up the database record's key. */
		key.data = my_engineer.EngineerID;
		key.size = (u_int32_t)strlen(my_engineer.EngineerID) + 1;

		/* Set up the database record's data. */
		data.data = &my_engineer;
		data.size = sizeof(ENGINEER);

		/* Put the data into the database. */
		ret = engineer_dbp->put(engineer_dbp, NULL, &key, &data, 
		    DB_NOOVERWRITE);

		if (ret == DB_KEYEXIST) {
			engineer_dbp->err(engineer_dbp, ret,
			    "Put failed because key %f already exists", 
			    my_engineer.EngineerID);
			break;
		} else {
			printf("engineer record inserted.\n");
		}
	}
	return (0);
}

/*
 * Insert records to service database.
 */
int insert_service_record(DB *service_dbp)
{
	DBT key, data;
	SERVICE my_service;
	int ret;
	size_t len;
	char buf[BUFSIZ], String1[MAXFIELD], String2[MAXFIELD];

	/*
	 * Insert records into the database, where 
	 * the key is the service ID and the data is
	 * the SERVICE structure.
	 */
	for(;;){
		printf("input service record(input \"exit\" or \"quit\" "
		    "to end)>\n");
		fflush(stdout);
		/* Stop inserting on EOF, or "exit" or "quit". */
		if (fgets(buf, sizeof(buf), stdin) == NULL)
			break;
		len = strlen(buf);
		if (len >= 1 && buf[len-1] == '\n') {
			buf[len-1] = '\0';
			len--;
		}

		if (strcmp(buf, "exit") == 0 || strcmp(buf, "quit") == 0)
			break;
		if (len == 0)
			continue;

		memset(&my_service, 0, sizeof(SERVICE));
		memset(&key, 0, sizeof(DBT));
		memset(&data, 0, sizeof(DBT));

		/* Scan the line into the structure. */
		sscanf(buf, "%30[^#]#%30[^#]#%30[^#]#%13"
		    "[^#]#%30[^#]#%30[^#]#%30[^#]#%30[^#]#%30[^\n]", 
			my_service.ServiceID, my_service.CustomerID, 
			my_service.EngineerID, String2, 
			my_service.ProblemContent, my_service.RequestDate,
			my_service.CloseDate, my_service.Expense, String1);

		my_service.SatiEvaluation = atoi(String1);
		my_service.ProblemLevel = atoi(String2);

		/* Set up the database record's key. */
		key.data = my_service.ServiceID;
		key.size = (u_int32_t)strlen(my_service.ServiceID) + 1;

		/* Set up the database record's data. */
		data.data = &my_service;
		data.size = sizeof(SERVICE);

		/* Put the data into the database. */
		ret = service_dbp->put(service_dbp, NULL, &key, &data, 
		    DB_NOOVERWRITE);

		if (ret == DB_KEYEXIST) {
			service_dbp->err(service_dbp, ret, 
			    "Put failed because key %f already exists", 
			    my_service.ServiceID);
			break;
		} else {
			printf("service record inserted.\n");
		}
	}
	return (0);
}

/*
 * Delete customer database record by giving its customer ID.
 */
int delete_customer_record(char *CustomerID, DB *customer_dbp)
{
	DBT key;

	/* Zero out the DBTs before using them. */
	memset(&key, 0, sizeof(DBT));

	/* Set the delete key to the customer ID. */
	key.data = CustomerID;
	key.size = (u_int32_t)strlen(CustomerID) + 1;

	/* Delete customer record. */
	customer_dbp->del(customer_dbp, NULL, &key, 0);

	return (0);
}

/*
 * Delete engineer database record by giving its engineer ID.
 */
int delete_engineer_record(char *EngineerID, DB *engineer_dbp)
{
	DBT key;

	/* Zero out the DBTs before using them. */
	memset(&key, 0, sizeof(DBT));

	/* Set the delete key to the engineer ID. */
	key.data = EngineerID;
	key.size = (u_int32_t)strlen(EngineerID) + 1;

	/* Delete engineer record. */
	engineer_dbp->del(engineer_dbp, NULL, &key, 0);

	return (0);
}

/*
 * Delete service database record by giving its service ID.
 */
int delete_service_record(char *ServiceID, DB *service_dbp)
{
	DBT key;

	/* Zero out the DBTs before using them. */
	memset(&key, 0, sizeof(DBT));

	/* Set the delete key to the service ID. */
	key.data = ServiceID;
	key.size = (u_int32_t)strlen(ServiceID) + 1;

	/* Delete service record. */
	service_dbp->del(service_dbp, NULL, &key, 0);

	return (0);
}

/*
 * Flushes the contents of customer database into service.txt file.
 */
int flush_customer_database(AFTERSALE_DBS my_aftersale, char * customer_file)
{
	DBC *customer_cursorp;
	DBT key, data;
	char buf[MAXLINE];
	FILE *ifp;
	CUSTOMER my_customer;
	int ret,n;

	/* Flush the customer database. */
	ifp = fopen(customer_file, "w+");
	if (ifp == NULL) {
		fprintf(stderr, "Error opening file '%s' \n", customer_file);
		return (-1);
	}

	/* Zero out the DBTs. */
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));
	/* Zero out the structure. */
	memset(&my_customer, 0, sizeof(CUSTOMER));

	data.data = &my_customer;
	data.ulen = sizeof(CUSTOMER);
	data.flags = DB_DBT_USERMEM;

	/* Get a cursor to the customer db. */
	my_aftersale.customer_dbp->cursor(my_aftersale.customer_dbp, NULL, 
	    &customer_cursorp, 0);

	/* 
	 * Iterate over the customer database, from the first 
	 * record to the last, flushing each in turn. 
	 */
	while ((ret = customer_cursorp->get(customer_cursorp, &key, &data, 
	    DB_NEXT)) == 0) {
		n = sprintf(buf, "%s#%s#%s#%s#%s#%s#%s#%s", 
			my_customer.CustomerID, my_customer.Name, 
			my_customer.Telephone, my_customer.Address, 
			my_customer.Email, my_customer.Fax, 
			my_customer.PurchasedAutomobile, 
			my_customer.PurchasedDate);
		fputs(buf, ifp);
		fputs("\n", ifp);
	}

	/* Close the customer.txt file. */
	fclose(ifp);
	return (0);
}

/*
 * Flushes the contents of engineer database into engineer.txt file.
 */
int flush_engineer_database(AFTERSALE_DBS my_aftersale, char *engineer_file)
{
	DBC *engineer_cursorp;
	DBT key, data;
	char buf[MAXLINE];
	FILE *ifp;
	ENGINEER my_engineer;
	int ret,n;

	/* Flush the engineer database. */
	ifp = fopen(engineer_file, "w+");
	if (ifp == NULL) {
		fprintf(stderr, "Error opening file '%s' \n", engineer_file);
		return (-1);
	}

	/* Zero out the DBTs. */
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));
	/* Zero out the structure. */
	memset(&my_engineer, 0, sizeof(ENGINEER));

	data.data = &my_engineer;
	data.ulen = sizeof(ENGINEER);
	data.flags = DB_DBT_USERMEM;

	/* Get a cursor to the engineer db. */
	my_aftersale.engineer_dbp->cursor(my_aftersale.engineer_dbp, NULL, 
	    &engineer_cursorp, 0);

	/* 
	 * Iterate ove the engineer database, from the first 
	 * record to the last, flushing each in turn. 
	 */
	while ((ret = engineer_cursorp->get(engineer_cursorp, &key, &data, 
	    DB_NEXT)) == 0) {
		n = sprintf(buf, "%s#%s#%s#%s#%s#%s#%s", 
			my_engineer.EngineerID, my_engineer.Name,
			my_engineer.Telephone, my_engineer.Email,
			my_engineer.Fax, my_engineer.LengthofService, 
			my_engineer.Expertise);
		fputs(buf, ifp);
		fputs("\n", ifp);
	}

	/* Close the engineer.txt file. */
	fclose(ifp);
	return (0);
}

/*
 * Flushes the contents of service database into service.txt file.
 */
int flush_service_database(AFTERSALE_DBS my_aftersale, char *service_file)
{
	DBC *service_cursorp;
	DBT key, data;
	char buf[MAXLINE];
	FILE *ifp;
	SERVICE my_service;
	int ret,n;

	/* Flush the service database. */
	ifp = fopen(service_file, "w+");
	if (ifp == NULL) {
		fprintf(stderr, "Error opening file '%s' \n", service_file);
		return (-1);
	}

	/* Zero out the DBTs. */
	memset(&key, 0, sizeof(DBT));
	memset(&key, 0, sizeof(DBT));
	/* Zero out the structure. */
	memset(&my_service, 0, sizeof(SERVICE));

	data.data = &my_service;
	data.ulen = sizeof(SERVICE);
	data.flags = DB_DBT_USERMEM;

	/* Get a cursor to the service db. */
	my_aftersale.service_dbp->cursor(my_aftersale.service_dbp, 
	    NULL, &service_cursorp, 0);

	/* 
	 * Iterate over the service database, from the first 
	 * record to the last, flushing each in turn. 
	 */
	while ((ret = service_cursorp->get(service_cursorp, &key, 
	    &data, DB_NEXT)) == 0) {
		n = sprintf(buf, "%s#%s#%s#%d#%s#%s#%s#%s#%d", 
			my_service.ServiceID, my_service.CustomerID,
			my_service.EngineerID, my_service.ProblemLevel, 
			my_service.ProblemContent, my_service.RequestDate, 
			my_service.CloseDate, my_service.Expense, 
			my_service.SatiEvaluation);
		fputs(buf, ifp);
		fputs("\n", ifp);
	}

	/* Close the service.txt file. */
	fclose(ifp);
	return (0);
}

/*
 * Updates a customer database record by giving its customer name.
 */
int update_customer_record(AFTERSALE_DBS *my_aftersale)
{
	DBC *dbc_customer;
	DBT key, data;
	CUSTOMER my_customer;
	int ret;
	char buf[BUFSIZ];
	size_t len;

	/* Initialize our DBTs. */
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));

	ret = my_aftersale->customer_dbp->cursor(my_aftersale->customer_dbp, 
	    NULL, &dbc_customer, 0);

	printf("input new customer record>\n");
	fflush(stdout);
	if (fgets(buf, sizeof(buf), stdin) == NULL)
		return (0);
	len = strlen(buf);
	if (len >= 1 && buf[len-1] == '\n') {
		buf[len - 1] = '\0';
		len--;
	}

	/* Scan the line into the structure. */
	sscanf(buf, "%30[^#]#%30[^#]#%13[^#]#%30"
	    "[^#]#%30[^#]#%30[^#]#%30[^#]#%30[^\n]", 
		my_customer.CustomerID, my_customer.Name, 
		my_customer.Telephone, my_customer.Address, 
		my_customer.Email, my_customer.Fax, 
		my_customer.PurchasedAutomobile, 
		my_customer.PurchasedDate);

	/* Set up the database record's key. */
	key.data = my_customer.CustomerID;
	key.size = (u_int32_t)strlen(my_customer.CustomerID) + 1;

	ret = dbc_customer->get (dbc_customer, &key, &data, DB_SET);
	if (ret != 0) {
		printf("error get customer record.\n");
		return (ret);
	}

	/* Set up the database record's data. */
	data.data = &my_customer;
	data.size = sizeof(CUSTOMER);

	ret = dbc_customer->put (dbc_customer, &key, &data, DB_CURRENT);
	if (ret != 0) {
		printf("error put customer record.\n");
		return (ret);
	}

	dbc_customer->close(dbc_customer);

	if (ret != 0) {
		printf("fail to close customer database cursor.\n");
	}
	
	return (0);
}

/*
 * Updates an engineer database record by giving its engineer name.
 */
int update_engineer_record(AFTERSALE_DBS *my_aftersale)
{
	DBC *dbc_engineer;
	DBT key, data;
	ENGINEER my_engineer;
	int ret;
	char buf[BUFSIZ];
	size_t len;

	/* Initialize our DBTs. */
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));

	ret = my_aftersale->engineer_dbp->cursor(my_aftersale->engineer_dbp, 
	    NULL, &dbc_engineer, 0);

	printf("input new engineer record>\n");
	fflush(stdout);
	if (fgets(buf, sizeof(buf), stdin) == NULL)
		return (0);
	len = strlen(buf);
	if (len >= 1 && buf[len-1] == '\n') {
		buf[len - 1] = '\0';
		len--;
	}

	/* Scan the line into the structure. */
	sscanf(buf, "%30[^#]#%30[^#]#%13[^#]#%30[^#]#%30[^#]#%30[^#]#%30[^\n]",
		my_engineer.EngineerID, my_engineer.Name, 
		my_engineer.Telephone, my_engineer.Email, 
		my_engineer.Fax, my_engineer.LengthofService,
		my_engineer.Expertise);

	/* Set up the database record's key. */
	key.data = my_engineer.EngineerID;
	key.size = (u_int32_t)strlen(my_engineer.EngineerID) + 1;

	ret = dbc_engineer->get(dbc_engineer, &key, &data, DB_SET);
	if (ret != 0) {
		printf("error get engineer record.\n");
		return (ret);
	}

	/* Set up the database record's data. */
	data.data = &my_engineer;
	data.size = sizeof(ENGINEER);

	ret = dbc_engineer->put(dbc_engineer, &key, &data, DB_CURRENT);
	if (ret != 0) {
		printf("error put engineer record.\n");
		return (ret);
	}

	dbc_engineer->close(dbc_engineer);

	if (ret != 0) {
		printf("fail to close engineer database cursor.\n");
	}

	return (0);
}

/*
 * Updates a service database record by giving its service ID.
 */
int update_service_record(AFTERSALE_DBS *my_aftersale)
{
	DBC *dbc_service;
	DBT key, data;
	SERVICE my_service;
	int ret;
	char buf[BUFSIZ], String1[MAXFIELD], String2[MAXFIELD];
	size_t len;

	/* Initialize our DBTs. */
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));

	ret = my_aftersale->service_dbp->cursor(my_aftersale->service_dbp, 
	    NULL, &dbc_service, 0);

	printf("input new service record>\n");
	fflush(stdout);
	if (fgets(buf, sizeof(buf), stdin) == NULL)
		return (0);
	len = strlen(buf);
	if (len >= 1 && buf[len-1] == '\n') {
		buf[len - 1] = '\0';
		len--;
	}

	/* Scan the line into the structure. */
	sscanf(buf, "%30[^#]#%30[^#]#%30[^#]#%13"
	    "[^#]#%30[^#]#%30[^#]#%30[^#]#%30[^#]#%30[^\n]", 
		my_service.ServiceID, my_service.CustomerID, 
		my_service.EngineerID, String2, 
		my_service.ProblemContent, my_service.RequestDate, 
		my_service.CloseDate, my_service.Expense, String1);

	my_service.SatiEvaluation = atoi(String1);
	my_service.ProblemLevel = atoi(String2);

	/* Set up the database record's key. */
	key.data = my_service.ServiceID;
	key.size = (u_int32_t)strlen(my_service.ServiceID) + 1;

	ret = dbc_service->get(dbc_service, &key, &data, DB_SET);
	if (ret != 0) {
		printf("error get service record.\n");
		return (ret);
	}

	/* Set up the database record's data. */
	data.data = &my_service;
	data.size = sizeof(SERVICE);


	ret = dbc_service->put(dbc_service, &key, &data, DB_CURRENT);
	if (ret != 0) {
		printf("error put service record.\n");
		return (ret);
	}

	dbc_service->close(dbc_service);

	if (ret != 0) {
		printf("fail to close service database cursor.\n");
	}

	return (0);
}
