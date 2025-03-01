/*-
* See the file LICENSE for redistribution information.
*
* Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
*
*/

/* 
 * after-sale management system.
 *    
 *    This application provides communication interface via command line.
 *    There are mainly three entrance point for the application: administrator,
 *    customer and engineer. The user enters the system by specifing different
 *    flags such as -a, -c, -e for administrator, customer and engineer 
 *    respectively, login to carry out further operation.
 *
 */

#include "servicemgmt_setup.h"

/* Forward declarations. */
int main(int, char *[]);
int usage();

int usage(){
	(void)fprintf(stderr, "usage: after-sale management system. make "
	    "ex_servicemgmt [-a administrator_login] [-c customer_login] "
	    "[-e engineer_login]\n");
	return (EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	extern int optind;
	extern char *optarg;
	int ch, ret, aflag, cflag, eflag;
	char *temp_buf;
	char buf[BUFSIZ];
	size_t len;

	ret = aflag = cflag = eflag = 0;

	/* Parse the command line arguements. */
	while ((ch = getopt(argc, argv, "ace")) != EOF) {
		switch (ch) {
		case 'a':	/* Administrator login flag. */
			aflag = 1;
			break;
		case 'c':	/* Customer login flag. */
			cflag = 1;
			break;
		case 'e':	/* Engineer login flag. */
			eflag = 1;
			break;
		case '?':
		default:
			return (usage());
		}
	}

	argc -= optind;
	argv += optind;

	if (aflag) {
		ret = administrator_login();
		return (ret);
	}

	if (cflag) {
		ret = customer_login();
		return (ret);
	}

	if (eflag) {
		ret = engineer_login();
		return (ret);
	}

	printf("input login way(a for administrator, c for customer, e for "
	    "engineer:\n");
	fflush(stdout);
	temp_buf = fgets(buf, sizeof(buf), stdin);
	len = strlen(buf);
	if (len >=1 && buf[len-1] == '\n') {
		buf[len - 1] = '\0';
		len--;
	}
	if (!strcmp(buf, "a")) {
		ret = administrator_login();
		return (ret);
	}
	else if (!strcmp(buf, "c")) {
		ret = customer_login();
		return (ret);
	}
	else if (!strcmp(buf, "e")) {
		ret = engineer_login();
		return (ret);
	}
	else {
		printf("wrong input.\n");
	}

	return (ret);
}
