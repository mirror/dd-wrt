JAVA_DEPEND = java.dep

## silently try to include these, if it fails gnu make
## will remake these 'makefiles' with the rules given in
## this file and restart the make process again
sinclude $(JAVA_DEPEND)

compile_classpath = $(vm_classes):$(top_srcdir):$(top_srcdir)/external/w3c_dom:$(top_srcdir)/external/sax:$(top_srcdir)/external/relaxngDatatype:$(top_srcdir)/external/jsr166:.:$(PATH_TO_GLIBJ_ZIP):$(PATH_TO_ESCHER)

# source,target and classpath options
GCJ_OPTS=-fsource=1.6 -ftarget=1.6 --bootclasspath='' --classpath=$(compile_classpath)
ECJ_OPTS=-source 1.6 -target 1.6 -bootclasspath '' -classpath $(compile_classpath)
JAVAC_OPTS=$(ECJ_OPTS)

# handling source to bytecode compiler programs like gcj, jikes  and kjc
if GCJ_JAVAC
JCOMPILER = $(JAVAC) $(JAVACFLAGS) $(GCJ_OPTS) -d . @classes
else
if ECJ_JAVAC
JCOMPILER = $(JAVAC) $(JAVACFLAGS) $(ECJ_WARNINGS) $(ECJ_OPTS) $(JAVAC_MEM_OPT) -d . @classes
else
JCOMPILER = $(JAVAC) $(JAVACFLAGS) $(JAVAC_WARNINGS) $(JAVAC_OPTS) $(JAVAC_MEM_OPT) -d . @classes
endif
endif

if CREATE_COLLECTIONS
COLLECTIONS = collections.jar

if WITH_JAR
CREATE_COLLECTIONS_JAR=$(JAR) cf $@ $(COLLECTIONS_PREFIX)
else
CREATE_COLLECTIONS_JAR=$(ZIP) -r -D $@ $(COLLECTIONS_PREFIX) > /dev/null
endif

collections.jar: mkcollections.pl
	./mkcollections.pl $(top_srcdir)
	$(JCOMPILER) `$(FIND) $(COLLECTIONS_PREFIX) -name '*.java' -type f -print`
	$(CREATE_COLLECTIONS_JAR)
endif # CREATE_COLLECTIONS

if INSTALL_GLIBJ_ZIP

glibj_DATA = glibj.zip $(COLLECTIONS)

endif # INSTALL_GLIBJ_ZIP

if BUILD_CLASS_FILES
noinst_DATA = genclasses compile-classes resources
endif # BUILD_CLASS_FILES

if INSTALL_CLASS_FILES

install-data-local: genclasses compile-classes
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)
	cp -R gnu $(DESTDIR)$(pkgdatadir)
	cp -R java $(DESTDIR)$(pkgdatadir)
	cp -R javax $(DESTDIR)$(pkgdatadir)
	cp -R org $(DESTDIR)$(pkgdatadir)
	cp -R sun $(DESTDIR)$(pkgdatadir)
	cp -R META-INF $(DESTDIR)$(pkgdatadir)

# FIXME - should mimic doc/api/Makefile.am instead...
uninstall-local:
	rm -rf $(DESTDIR)$(pkgdatadir)/gnu
	rm -rf $(DESTDIR)$(pkgdatadir)/java
	rm -rf $(DESTDIR)$(pkgdatadir)/javax
	rm -rf $(DESTDIR)$(pkgdatadir)/org
	rm -rf $(DESTDIR)$(pkgdatadir)/sun
	rm -rf $(DESTDIR)$(pkgdatadir)/META-INF

endif # INSTALL_CLASS_FILES

if USE_PREBUILT_GLIBJ_ZIP

glibj.zip:
	cp $(PATH_TO_GLIBJ_ZIP) .

else

if WITH_JAR
CREATE_GLIBJ_ZIP=$(JAR) cf glibj.zip gnu java javax org sun META-INF && $(JAR) i glibj.zip
else
CREATE_GLIBJ_ZIP=$(ZIP) -r -D glibj.zip gnu java javax org sun META-INF > /dev/null
endif

glibj.zip: classes compile-classes resources
	$(CREATE_GLIBJ_ZIP)
endif # USE_PREBUILT_GLIBJ_ZIP

resources: copy-vmresources.sh
	@list=`cd $(top_srcdir)/resource && $(FIND) gnu java javax org -name \*\.properties -print -o -name \*\.css -print`; for p in $$list; do \
          dirname=`dirname $$p`; \
          if ! test -d "$$dirname"; then @mkdir_p@ "$$dirname"; fi; \
	  cp $(top_srcdir)/resource/$$p $$p; \
	done
	@list=`cd $(top_srcdir)/resource && $(FIND) META-INF -name CVS -prune -o -name .svn -prune -o -name \*\.in -prune -o -type f -print`; for p in $$list; do \
          dirname=`dirname $$p`; \
          if ! test -d "$$dirname"; then @mkdir_p@ "$$dirname"; fi; \
	  cp $(top_srcdir)/resource/$$p $$p; \
	done
	@$(SHELL) ./copy-vmresources.sh
	@list=`cd $(top_srcdir) && $(FIND) gnu/javax/swing/plaf/gtk/icons -name *.png -type f -print`; for p in $$list; do \
          dirname=`dirname $$p`; \
          if ! test -d "$$dirname"; then @mkdir_p@ "$$dirname"; fi; \
	  cp $(top_srcdir)/$$p $$p; \
	done
	touch resources

$(top_builddir)/gnu/java/locale/LocaleData.java: $(top_srcdir)/scripts/generate-locale-list.sh 
	@mkdir_p@ $(top_builddir)/gnu/java/locale
	$(top_srcdir)/scripts/generate-locale-list.sh > $(top_builddir)/gnu/java/locale/LocaleData.java

genclasses: gen-classlist.sh standard.omit $(top_builddir)/gnu/java/locale/LocaleData.java gen-xpath-parser
	top_builddir=$(top_builddir) top_srcdir=$(top_srcdir) $(SHELL) ./gen-classlist.sh standard
	touch genclasses

# Only rebuild parsers when explicitly asked to.
if REGEN_PARSERS

gen-xpath-parser: $(top_srcdir)/gnu/xml/xpath/XPathParser.java
	touch gen-xpath-parser

$(top_srcdir)/gnu/xml/xpath/XPathParser.java: $(top_srcdir)/gnu/xml/xpath/XPathParser.y
	( cd $(top_srcdir)/gnu/xml/xpath; \
	  $(JAY) XPathParser.y < $(JAY_SKELETON) > XPathParser.java )
else

gen-xpath-parser:
	touch gen-xpath-parser

endif # REGEN_PARSER

$(JAVA_DEPEND): genclasses

#if FOUND_GCJ
## When building with gcj, we do a recursive make.  We split this rule
## out specially, rather than simply defining JAVAC, so that GNU make
## will see the recursive make invocation and still allow parallel
## builds.
#compile-classes: classes $(JAVA_SRCS) Makefile
#	$(MAKE) -f $(srcdir)/Makefile.gcj \
#	  GCJ='$(GCJ)' \
#	  compile_classpath='$(top_builddir):$(compile_classpath)' \
#	  top_srcdir=$(top_srcdir)
#   	  touch compile-classes
#else
compile-classes: genclasses $(JAVA_SRCS) Makefile
	$(JCOMPILER)
	touch compile-classes
#endif

EXTRA_DIST = standard.omit.in mkcollections.pl.in Makefile.gcj split-for-gcj.sh glibj.zip
CLEANFILES = compile-classes resources classes genclasses gen-xpath-parser \
	glibj.zip classes.1 classes.2 Makefile.deps \
	$(top_builddir)/gnu/java/locale/LocaleData.java \
	$(JAVA_DEPEND)
DISTCLEANFILES = standard.omit

clean-local:
	-rm -rf gnu
	-rm -rf java
	-rm -rf javax
	-rm -rf org
	-rm -rf com
	-rm -rf sun
	-rm -rf META-INF
	-rm -rf lists

dist-hook:
	@mkdir_p@ $(distdir)
	cp -pdfR $(top_srcdir)/gnu $(top_srcdir)/java $(top_srcdir)/javax $(top_srcdir)/org $(top_srcdir)/sun $(top_srcdir)/vm $(top_srcdir)/resource $(distdir)/..
# Delete not wanted files.
	$(FIND) $(distdir)/../gnu $(distdir)/../java $(distdir)/../javax $(distdir)/../org $(distdir)/../sun $(distdir)/../vm $(distdir)/../resource -name CVS -print | xargs rm -fr
	$(FIND) $(distdir)/../gnu $(distdir)/../java $(distdir)/../javax $(distdir)/../org $(distdir)/../sun $(distdir)/../vm $(distdir)/../resource -name .cvsignore -print | xargs rm -f
	$(FIND) $(distdir)/../gnu $(distdir)/../java $(distdir)/../javax $(distdir)/../org $(distdir)/../sun $(distdir)/../vm -name class-dependencies.conf -print | xargs rm -f
	rm -f $(distdir)/../gnu/classpath/Configuration.java
	rm -f $(distdir)/../gnu/java/security/Configuration.java
	rm -f $(distdir)/../gnu/java/locale/LocaleData.java
	rm -f $(distdir)/glibj.zip
