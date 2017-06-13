/*
 * admin.sql -- PostgreSQL commands for creating the RADIUS user.
 *
 *	WARNING: You should change 'localhost' and 'radpass'
 *		 to something else.  Also update raddb/sql.conf
 *		 with the new RADIUS password.
 *
 *	WARNING: This example file is untested.  Use at your own risk.
 *		 Please send any bug fixes to the mailing list.
 *
 *	$Id: 6b41aa1538b56713965e41d2b271a23c8e03bc68 $
 */

/*
 *  Create default administrator for RADIUS
 */
CREATE USER radius WITH PASSWORD 'radpass';

/* radius user needs ti clean tables in test env */
GRANT ALL ON ALL TABLES IN SCHEMA public TO radius;
GRANT SELECT, USAGE ON ALL SEQUENCES IN schema public TO radius;
