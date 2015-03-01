AUTOMAKE_OPTIONS=foreign no-dependencies

noinst_LIBRARIES = libtarget_based.a

INCLUDES = @INCLUDES@

if HAVE_TARGET_BASED
#BUILT_SOURCES = \
#sf_attribute_table_parser.c \
#sf_attribute_table.h \
#sf_attribute_table.c

nodist_libtarget_based_a_SOURCES = \
sf_attribute_table_parser.c \
sf_attribute_table.h \
sf_attribute_table.c

libtarget_based_a_SOURCES = \
sftarget_reader.c \
sftarget_reader.h \
sftarget_hostentry.c \
sftarget_hostentry.h \
sftarget_protocol_reference.c \
sftarget_protocol_reference.h \
sf_attribute_table_parser.l \
sf_attribute_table.y

else
libtarget_based_a_SOURCES = sftarget_reader.c
endif

.y.c:
	$(YACC) -d -psfat_ -o$@ $?

#### Ugly to get the header file built.
#### any other suggestions?
sf_attribute_table.h: sf_attribute_table.y
	$(YACC) -d -psfat_ $?
	mv y.tab.h $@

.l.c:
	$(LEX) -i -o$@ $?

sf_attribute_table_parser.c: sf_attribute_table_parser.l sf_attribute_table.h
	$(LEX) -i -Psfat -o$@ $<

clean-local:
	rm -f \
	sf_attribute_table_parser.c \
	sf_attribute_table.h \
	sf_attribute_table.c
