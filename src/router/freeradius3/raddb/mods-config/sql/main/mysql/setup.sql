# -*- text -*-
##
## admin.sql -- MySQL commands for creating the RADIUS user.
##
##	WARNING: You should change 'localhost' and 'radpass'
##		 to something else.  Also update raddb/mods-available/sql
##		 with the new RADIUS password.
##
##	$Id: f0453e179a6721c5675f6d72ad30a97e1ccb48fa $

#
#  Create default administrator for RADIUS
#
CREATE USER 'radius'@'localhost';
SET PASSWORD FOR 'radius'@'localhost' = PASSWORD('radpass');

# The server can read any table in SQL
GRANT SELECT ON radius.* TO 'radius'@'localhost';

# The server can write to the accounting and post-auth logging table.
#
#  i.e.
GRANT ALL on radius.radacct TO 'radius'@'localhost';
GRANT ALL on radius.radpostauth TO 'radius'@'localhost';
