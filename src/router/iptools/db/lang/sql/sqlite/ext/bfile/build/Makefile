#!/usr/make
#
# Makefile for DBSQL BFile
#
# This makefile is suppose to be build for linux and windriver-ovia.
# But if that does not work for you, you can configure the makefile 
# manually.  Just set the parameters below to values that
# work well for your system.
#

# The toplevel directory of the source tree.
#
TOP = ../
# When manipulating BFILE, if you want to use c-functions set it to 1;
# Otherwise, set it to zero
ENABLE_BFILE_CAPI = 0

# The ovia options
#
ifeq ($(BUILD_OVIA), 1)
CROSS_COMPILE = arm-wrs-linux-gnueabi-arm_novfp-glibc_small-
CFLAGS += -msoft-float
CFLAGS += -I$(WIND_BASE)/wrlinux/target-libs/glibc_small/arm-wrs-linux-gnueabi-arm_novfp/include
CFLAGS += -L$(WIND_BASE)/wrlinux/target-libs/glibc_small/arm-wrs-linux-gnueabi-arm_novfp/lib
#TLIBS += -ldl
endif

# C Compile and options for use in building executables that 
# will run on the target platform.
#
TCC = $(CROSS_COMPILE)gcc
TCC += $(CFLAGS) -fPIC -O3 -I${TOP}/include -I$(DBSQL_DIR)/include -I../../../../generated -I../../../src
ifeq ($(ENABLE_BFILE_CAPI), 1)
TCC += -DBFILE_USE_CAPIS
endif

# Any target libraries which libsqlite must be linked against
# 
TLIBS += -lpthread -ldl

# The directory into which to store package information for

# Some standard variables and programs
#
prefix = $(DBSQL_DIR)
exec_prefix = ${prefix}
libdir = ${exec_prefix}/lib
includedir = ${prefix}/include
INSTALL = /usr/bin/install -c

# You should not have to change anything below this line
###############################################################################


# This is the default Makefile target.  The objects listed here
# are what get build when you type just "make" with no arguments.
#
ifndef DBSQL_DIR
all:
	echo "Please set the correct path for DBSQL_DIR"
else
all:	libbfile_ext.so bfile_test bfile_example
endif

libbfile_ext.so:	$(TOP)/src/bfile.c $(TOP)/include/bfile.h
	$(TCC) $(TOP)/src/bfile.c -o libbfile_ext.so -shared

ifeq ($(ENABLE_BFILE_CAPI), 0)
bfile_test:	$(TOP)/test/bfile_test_sql.c $(TOP)/include/bfile.h libbfile_ext.so
	$(TCC) $(TOP)/test/bfile_test_sql.c -o bfile_test_sql $(TLIBS) -L. -lbfile_ext -L$(DBSQL_DIR)/lib -ldb_sql -ldl
bfile_example:	$(TOP)/examples/bfile_example_sql.c $(TOP)/include/bfile.h libbfile_ext.so
	$(TCC) $(TOP)/examples/bfile_example_sql.c -o bfile_example_sql $(TLIBS) -L. -lbfile_ext -L$(DBSQL_DIR)/lib -ldb_sql -ldl
else
bfile_test:	$(TOP)/test/bfile_test_capi.c $(TOP)/include/bfile.h libbfile_ext.so
	$(TCC) $(TOP)/test/bfile_test_capi.c -o bfile_test_capi $(TLIBS) -L. -lbfile_ext -L$(DBSQL_DIR)/lib -ldb_sql -ldl
bfile_example:	$(TOP)/examples/bfile_example_capi.c $(TOP)/include/bfile.h libbfile_ext.so
	$(TCC) $(TOP)/examples/bfile_example_capi.c -o bfile_example_capi $(TLIBS) -L. -lbfile_ext -L$(DBSQL_DIR)/lib -ldb_sql -ldl
endif
install:	libbfile_ext.so $(TOP)/include/bfile.h
	$(INSTALL) -m 0755 libbfile_ext.so $(DESTDIR)$(libdir)
	$(INSTALL) -m 0644 $(TOP)/include/bfile.h $(DESTDIR)$(includedir)

clean:	
	rm -rf *.jpg *.db* libbfile_ext.so bfile_test bfile_example

# Makefile can not test for ovia. It sould be copy to ovia platform.
# The follow target only suit for linux.
#
ifeq ($(BUILD_OVIA), 0)
test:	bfile_test
	env LD_LIBRARY_PATH=.:$(DBSQL_DIR)/lib:$(LD_LIBRARY_PATH) ./bfile_test
endif

ifdef TESTFIXTURE
ifdef TEST_DIR
tcl_test:	$(TOP)/test/bfile.test
	cp $(TOP)/test/bfile.test $(TEST_DIR)/
	env LD_LIBRARY_PATH=.:$(DBSQL_DIR)/lib:$(LD_LIBRARY_PATH) $(TESTFIXTURE) $(TEST_DIR)/bfile.test
else
tcl_test:
	-echo "Please set the correct fullpath for TEST_DIR"
endif
else
tcl_test:
	-echo "Please set the correct fullpath for TESTFIXTURE"
endif

run:	bfile_example
	env LD_LIBRARY_PATH=.:$(DBSQL_DIR)/lib:$(LD_LIBRARY_PATH) ./bfile_example

