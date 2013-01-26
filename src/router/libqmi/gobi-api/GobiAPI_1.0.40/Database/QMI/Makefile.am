noinst_LTLIBRARIES = libQMIDB.la

DBFILES = \
	Entity.txt \
	EnumEntry.txt \
	Enum.txt \
	Field.txt \
	Struct.txt

QMIDB.o: $(DBFILES)
	$(LD) -r -b binary -o QMIDB.o $(DBFILES)

libQMIDB_la_SOURCES = foo.c

libQMIDB_la_LIBADD = QMIDB.o

CLEANFILES = QMIDB.o
