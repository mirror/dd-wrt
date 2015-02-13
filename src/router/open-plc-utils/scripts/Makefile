#!/bin/make -f
# file: scripts/Makefile

# ====================================================================
# environment definitions;
# --------------------------------------------------------------------

include ../make.def

# ====================================================================
#
# --------------------------------------------------------------------

TRASH=t t.* *~ *.[0-9][0-9][0-9]
FILES=*.c *.h *.mak Makefile Makefile.*
TOOLS=
PAGES=fdm.1

# ====================================================================
#
# --------------------------------------------------------------------

.PHONY: compile library scripts manuals install uninstall check fresh clean

# ====================================================================
#
# --------------------------------------------------------------------

all compile:
install:
scripts:
manuals:
	test -d ${MAN} || install -m ${DIR_PERM} -o ${OWNER} -g ${GROUP} -d ${MAN}
	install -m ${MAN_PERM} -o ${OWNER} -g ${GROUP} ${PAGES} ${MAN}
library:
uninstall:
clean:
	rm -f ${TOOLS} ${TRASH}
fresh: clean compile
ignore:
#	echo ${TOOLS} | tr ' ' '\n' > .gitignore

# ====================================================================
#
# --------------------------------------------------------------------

