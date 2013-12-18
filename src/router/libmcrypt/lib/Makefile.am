DEFS = @DEFS@ -DLIBDIR=\"$(libdir)/@PACKAGE@/\" 
INCLUDES = -I. -I.. $(INCLTDL) -I../modules/algorithms -I../modules/modes
EXTRA_DIST = libdefs.h bzero.h xmemory.h mcrypt_modules.h mcrypt_internal.h \
 libmcrypt.sym win32_comp.h mcrypt.h.in libmcrypt-config.in libmcrypt.m4
lib_LTLIBRARIES = libmcrypt.la 

libmcrypt_la_SOURCES = mcrypt_extra.c mcrypt.c bzero.c xmemory.c mcrypt_modules.c win32_comp.c mcrypt_threads.c \
	mcrypt_symb.c 

libmcrypt_la_LDFLAGS =  \
	  -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
	  -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
	  -export-symbols libmcrypt.sym @LIBLTDL@ 
libmcrypt_la_LIBADD = @EXTRA_OBJECTS@

m4datadir = $(datadir)/aclocal
m4data_DATA = libmcrypt.m4
bin_SCRIPTS = libmcrypt-config

mcrypt_symb.c: mcrypt_internal.h
	@echo "#include \"mcrypt_internal.h\"" > mcrypt_symb.c
	@echo "" >> mcrypt_symb.c
	@echo "/* This is automatically created. Don't touch... */" >> mcrypt_symb.c
	@echo "" >> mcrypt_symb.c
	-@for i in $(EXTRA_ALGOS); do \
		if test -f ../modules/algorithms/$$i.c; then cat ../modules/algorithms/$$i.c 2>/dev/null|grep define|grep LTX|awk '{print "extern "$$3"();";}' >> mcrypt_symb.c 2>/dev/null; fi; \
		if test -f ../modules/modes/$$i.c; then cat ../modules/modes/$$i.c 2>/dev/null|grep define|grep LTX|awk '{print "extern "$$3"();";}' >> mcrypt_symb.c 2>/dev/null; fi; \
	done
	@echo "" >> mcrypt_symb.c
	@echo "const mcrypt_preloaded mps[] = {" >> mcrypt_symb.c
	-@for i in $(EXTRA_ALGOS); do \
		if test -f ../modules/modes/$$i.c; then echo "	{\"$$i\", NULL}, " >> mcrypt_symb.c 2>/dev/null; fi; \
		if test -f ../modules/algorithms/$$i.c; then echo "	{\"$$i\", NULL}, " >> mcrypt_symb.c 2>/dev/null; fi; \
		if test -f ../modules/algorithms/$$i.c; then cat ../modules/algorithms/$$i.c 2>/dev/null|grep define|grep LTX|awk '{print "\t{\""$$3"\", "$$3"},";}' >> mcrypt_symb.c 2>/dev/null; fi; \
		if test -f ../modules/modes/$$i.c; then cat ../modules/modes/$$i.c 2>/dev/null|grep define|grep LTX|awk '{print "\t{\""$$3"\", "$$3"},";}' >> mcrypt_symb.c 2>/dev/null; fi; \
	done
	@echo "	{NULL, NULL}" >> mcrypt_symb.c
	@echo "};" >> mcrypt_symb.c

clean-local:
	-rm mcrypt_symb.c
