# After editing this file, do something like:
#   make clean; aclocal; autoconf; automake -a

# Program name
bin_PROGRAMS = frottle

# Check for required, non-standard libraries
frottle_LDFLAGS = -L/usr/lib
frottle_LDADD = -lpthread -lipq

# List of source files
frottle_SOURCES = main.c master.c client.c main.h wireless.h frottle.8 frottle.conf.sample

# Set the manual to be installed
man_MANS = frottle.8

# Set sysconfig dir to my favourite default and put sample .conf in it
sysconfdir = /etc
sysconf_DATA = frottle.conf.sample

