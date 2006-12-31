# The UNI version can be configured at run time. This is the default. Use the
# explicit version selections below only in case of problems.
#
STANDARDS=-DDYNAMIC_UNI
#
# Default is UNI 3.0, for good reasons, see below.
#
# STANDARDS=-DUNI30
#
# Note: some UNI 3.0 switches will show really strange behaviour if confronted
#	with using 3.1 signaling, so be sure to test your network *very*
#	carefully before permanently configuring machines to use UNI 3.1.
#
#STANDARDS=-DUNI31 -DALLOW_UNI30
#
# Some partial support for UNI 4.0 can be enabled by using
#
# STANDARDS=-DUNI40
#
# Note that none of -DUNI30, -DUNI31, or -DALLOW_UNI30 may be used with that
# one.
#
# If using -DUNI40, you can also enable peak cell rate modification as
# specified in Q.2963.1 with
#
# STANDARDS=-DUNI40 -DQ2963_1
#
# If you're using a Cisco LS100 or LS7010 switch, you should add the following
# line to work around a bug in their point-to-multipoint signaling (it got
# confused when receiving a CALL PROCEEDING, so we don't send it, which of
# course makes our clearing procedure slightly non-conformant):
#
# STANDARDS += -DCISCO
#
# Some versions of the Thomson Thomflex 5000 won't do any signaling before they
# get a RESTART. Uncomment the next line to enable sending of a RESTART
# whenever SAAL comes up. Note that the RESTART ACKNOWLEDGE sent in response to
# the RESTART will yield a warning, because we don't implement the full RESTART
# state machine.
#
# STANDARDS += -DTHOMFLEX
#
# Note: after changing STANDARDS, you need to rebuild at least the directories
#       ilmid/, qgen/, sigd/, and sigd.new/

ifeq ($(TOPDIR),)
TOPDIR=..
endif

CFLAGS_NOWARN=-DVERSION=\"`cat $(TOPDIR)/VERSION`\" \
  $(INCLUDES) -I$(TOPDIR)/lib
CFLAGS_NOOPT=$(CFLAGS_NOWARN) -Wall -Wshadow -Wpointer-arith \
  -Wwrite-strings -Wstrict-prototypes 
  # -Wcast-align (clashes with socketbits.h of glibc 2 on Alphas)
  # -Wmissing-prototypes (linux/byteorder is broken)
  # -Wmissing-declarations (gcc 2.6.x only)
  # -Wconversion (breaks inline)
CFLAGS_OPT=$(COPTS)
CFLAGS=$(CFLAGS_NOOPT) $(CFLAGS_OPT) $(CFLAGS_PRIVATE)
CFLAGS_LEX=$(CFLAGS_NOWARN) $(CFLAGS_OPT)
CFLAGS_YACC=$(CFLAGS_NOWARN) $(CFLAGS_OPT) -DYY_USE_CONST
LDFLAGS=
LDLIBS += -L$(TOPDIR)/lib -latm
LIBDEPS += $(TOPDIR)/lib/libatm.a
YACC=bison -y -d #-v
INSTROOT=
INSTPREFIX=$(INSTROOT)/usr
INSTBOOTBIN=$(INSTPREFIX)/sbin
INSTUSRBIN=$(INSTPREFIX)/bin
INSTSYSBIN=$(INSTPREFIX)/sbin
INSTLIB=$(INSTROOT)/usr/lib
INSTHDR=$(INSTROOT)/usr/include
INSTMAN=$(INSTPREFIX)/man
INSTMAN1=$(INSTMAN)/man1
INSTMAN4=$(INSTMAN)/man4
INSTMAN7=$(INSTMAN)/man7
INSTMAN8=$(INSTMAN)/man8
# format: "process" mode instdir files
PROCLIST=\
    process 0755 $(INSTBOOTBIN) $(BOOTPGMS); \
    process 0755 $(INSTSYSBIN) $(SYSPGMS); \
    process 0755 $(INSTUSRBIN) $(USRPGMS); \
    process 0644 $(INSTLIB) $(GENLIBS); \
    process 0644 $(INSTHDR) $(SYSHDR); \
    optprocess 0644 $(INSTHDR) $(OPTSYSHDR); \
    process 0644 $(INSTMAN1) $(MAN1); \
    process 0644 $(INSTMAN4) $(MAN4); \
    process 0644 $(INSTMAN7) $(MAN7); \
    process 0644 $(INSTMAN8) $(MAN8)

#
# Enable memory debugging if MPR is installed
#
ifeq (/usr/lib/libmpr.a,$(wildcard /usr/lib/libmpr.a))
LDLIBS += -lmpr
endif
#
# Allow Makefiles to override depend, clean, and spotless
#
ifeq ($(DEPEND),)
DEPEND = depend_default
endif

ifeq ($(CLEAN),)
CLEAN = clean_default
endif

ifeq ($(SPOTLESS),)
SPOTLESS = spotless_default
endif

LINK.c = $(CC) $(LDFLAGS)

.c:
		$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LDLIBS)

all:
		[ ! -r .checker ] || $(MAKE) clean
		$(MAKE) do_all
		for n in "" $(SUBDIRS); do [ -z "$$n" ] || \
		  make -C $$n || exit; done

# Note: bash-2 barfs on  for n in ; do ...  so we need this hack to sneak at
# least one item on the list.

do_all:		$(BOOTPGMS) $(SYSPGMS) $(USRPGMS) $(PGMS)

$(BOOTPGMS) $(SYSPGMS) $(USRPGMS) $(PGMS):	$(LIBDEPS)

checker:
		[ -r .checker ] || $(MAKE) clean
		$(MAKE) do_checker
		@for n in "" $(SUBDIRS); do [ -z "$$n" ] || \
		  make -C $$n checker || exit; done

do_checker:
		CC=checkergcc $(MAKE) -e do_all
		touch .checker

install:
		@process() { if [ ! -z "$$3" ]; then mode=$$1; dir=$$2; \
		  shift 2; echo "install -c -m $$mode $$* $$dir"; \
		  install -c -m $$mode $$* $$dir || exit 1; fi; }; \
		  optprocess() { [ -z "$$3" -o -r "/usr/include/$$3" ] || \
		  process $$*; }; \
		  $(PROCLIST)
		@for n in "" $(SUBDIRS); do [ -z "$$n" ] || \
		  make -C $$n install || exit; done

# optprocess is only defined for a single file. Right now we're using it only
# for stdint.h

instdirs:
		@process() { if [ ! -z "$$3" ]; then \
		  echo "install -d $$2"; \
		  install -d $$2 || exit 1; fi; }; \
		  optprocess() { :; }; \
		  $(PROCLIST)
		@for n in "" $(SUBDIRS); do [ -z "$$n" ] || \
		  make -C $$n instdirs || exit; done

uninstall:
		@process() { if [ ! -z "$$3" ]; then dir=$$2; shift 2; \
		  echo "cd $$dir; rm -f $$*"; \
		  cd $$dir; rm -f $$*; fi; }; \
		  optprocess() { :; }; \
		  $(PROCLIST)
		@for n in "" $(SUBDIRS); do [ -z "$$n" ] || \
		  make -C $$n uninstall || exit; done

filenames:
		@process() { if [ ! -z "$$3" ]; then dir=$$2; shift 2; \
		  for n in $$*; do echo $$dir/$$n; done; fi; }; \
		  optprocess() { process $$*; }; \
		  $(PROCLIST)
		@for n in "" $(SUBDIRS); do [ -z "$$n" ] || \
		  make -C $$n filenames || exit; done

depend:		$(DEPEND)

depend_default:
		$(CPP) -M *.c $(INCLUDES) -I$(TOPDIR)/lib >.tmpdepend
		mv .tmpdepend .depend
		for n in "" $(SUBDIRS); do [ -z "$$n" ] || \
		  make -C $$n depend || exit; done

clean:		$(CLEAN)

clean_default:
		rm -f *.o core .checker y.tab.h y.tab.c lex.yy.c $(TRASH)
		for n in "" $(SUBDIRS); do [ -z "$$n" ] || \
		  make -C $$n clean || exit; done

spotless:	$(SPOTLESS)

spotless_default:	clean
		rm -f $(BOOTPGMS) $(SYSPGMS) $(USRPGMS) $(PGMS) *.a
		rm -f .depend .checker $(TRASH_SPOTLESS)
		for n in "" $(SUBDIRS); do [ -z "$$n" ] || \
		  make -C $$n spotless || exit; done

lex.yy.o:	lex.yy.c y.tab.h
		$(CC) -c $(CFLAGS_LEX) lex.yy.c

y.tab.o:	y.tab.c
		$(CC) -c $(CFLAGS_YACC) y.tab.c

ifneq ($(NOLIBATMDEP),yes)
$(PGMS) dummy:	$(TOPDIR)/lib/libatm.a
endif

ifeq (.depend,$(wildcard .depend))
include .depend
endif
