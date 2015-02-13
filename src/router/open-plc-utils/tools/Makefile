#!/bin/make -f
# file: tools/Makefile

# ====================================================================
# environment definitions;
# --------------------------------------------------------------------

include ../make.def

# ====================================================================
# symbols;
# --------------------------------------------------------------------

CFLAGS+=
LDFLAGS+=
TRASH=*.o *.obj *.exe t t.* *~ *.[0-9][0-9][0-9]
FILES=Makefile *.mak *.sh *.txt *.c *.h *.xml *.css
TOOLS=
PAGES=

# ====================================================================
# pseudo targets;
# --------------------------------------------------------------------

all compile: ${TOOLS}
compact: compile
	if [ ! -z ${STRIP} ]; then ${STRIP} ${TOOLS}; fi
	if [ -x /usr/local/bin/upx ]; then upx --best ${TOOLS}; fi
library:
	install -m ${DIR_PERM} -o ${OWNER} -g ${GROUP} -d ${BIN}
scripts:
manuals:
#	install -m ${MAN_PERM} -o ${OWNER} -g ${GROUP} ${PAGES} ${MAN}
install: $(TOOLS) library
#	install -m ${BIN_PERM} -o ${OWNER} -g ${GROUP} ${TOOLS} ${BIN}
uninstall:
	cd ${BIN}; rm -f ${TOOLS}
	cd ${MAN}; rm -f ${PAGES}
check: 
	${SHELL} tools.sh
fresh: clean compile
clean:
	rm -f ${TOOLS} ${TRASH}
ignore:
	echo ${TOOLS} | tr ' ' '\n' > .gitignore

# ====================================================================
#
# --------------------------------------------------------------------

%: %.o
	${CC} -o ${@} ${^} ${LDFLAGS}
%.o: %.c
	${CC} -c ${<} ${CFLAGS}
%.h: %.h

# ====================================================================
# programs;
# --------------------------------------------------------------------

grab: grab.o getoptv.o putoptv.o version.o basespec.o todigit.o error.o efreopen.o

# ====================================================================
# libraries;
# --------------------------------------------------------------------

libmtools.a: binout.o bytespec.o confirmfilename.o checksum32.o dataspec.o decdecode.o decout.o emalloc.o reverse.o error.o fdchecksum32.o filepart.o getoptv.o hexdecode.o hexdump.o hexencode.o hexin.o hexout.o hexstring.o hexview.o regview.o ipv4spec.o ipv6spec.o memdecr.o memincr.o memswap.o output.o putoptv.o strdecr.o strfbits.o strincr.o synonym.o todigit.o typename.o basespec.o uintspec.o version.o
	rm -f ${@}
	$(AR) rc ${@} ${^}
	$(RANLIB) ${@}

# ====================================================================
#
# --------------------------------------------------------------------

include ../nodes/nodes.mak

