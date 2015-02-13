#!/usr/bin/make -f
# file: nodes/Makefile

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
FILES=Makefile *.mak *.sh *.txt *.c *.h
TOOLS=

# ====================================================================
# pseudo targets;
# --------------------------------------------------------------------

all compile: ${TOOLS}
compact: compile
	if [ ! -z ${STRIP} ]; then ${STRIP} ${TOOLS}; fi
	if [ -x /usr/local/bin/upx ]; then upx --best ${TOOLS}; fi
library:
#	install -m ${DIR_PERM} -o ${OWNER} -g ${GROUP} -d ${BIN}
scripts manuals:
install: $(TOOLS) library
#	install -m ${BIN_PERM} -o ${OWNER} -g ${GROUP} ${TOOLS} ${BIN}
uninstall:
#	cd ${BIN}; rm -f ${TOOLS}
check:
	${SHELL} nodes.sh
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
$.h: %.h

# ====================================================================
#  
# --------------------------------------------------------------------

include ../nodes/nodes.mak

