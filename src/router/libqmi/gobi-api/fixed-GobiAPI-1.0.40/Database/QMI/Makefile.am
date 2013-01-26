noinst_LTLIBRARIES = libQMIDB.la

DBFILES = \
	$(srcdir)/Entity.txt \
	$(srcdir)/EnumEntry.txt \
	$(srcdir)/Enum.txt \
	$(srcdir)/Field.txt \
	$(srcdir)/Struct.txt

QMIDB.o: $(DBFILES)
	$(LD) -r -b binary -o QMIDB.o $(DBFILES)

libQMIDB_la_SOURCES = foo.c

libQMIDB_la_LIBADD = QMIDB.o

CLEANFILES = QMIDB.o

EXTRA_DIST = $(DBFILES)
