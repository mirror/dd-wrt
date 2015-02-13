#!/usr/bin/make -f
# file: nvm/Makefile

# ====================================================================
# environment definitions;
# --------------------------------------------------------------------

include ../make.def

# ====================================================================
# symbols;
# --------------------------------------------------------------------

CFLAGS+=
LDFLAGS+=
TRASH=*.o *.obj *.exe t t.* *~ *.[0-9][0-9][0-9] *.test *-[0-9][0-9].nvm
FILES=Makefile *.mak *.sh *.txt *.c *.h *.xml *.css
TOOLS=chknvm chknvm2 nvmsplit nvmmerge
PAGES=chknvm.1 chknvm2.1 nvmsplit.1 nvmmerge.1

# ====================================================================
# pseudo targets;
# --------------------------------------------------------------------

all compile: ${TOOLS}
compact: compile
	if [ ! -z ${STRIP} ]; then ${STRIP} ${TOOLS}; fi
	if [ -x /usr/local/bin/upx ]; then upx --best ${TOOLS}; fi
scripts:
manuals:
	test -d ${MAN} || install -m ${DIR_PERM} -o ${OWNER} -g ${GROUP} -d ${MAN}
	install -m ${MAN_PERM} -o ${OWNER} -g ${GROUP} ${PAGES} ${MAN}
library:
	install -m ${DIR_PERM} -o ${OWNER} -g ${GROUP} -d ${BIN}
install: $(TOOLS) library
	install -m ${BIN_PERM} -o ${OWNER} -g ${GROUP} ${TOOLS} ${BIN}
uninstall:
	cd ${BIN}; rm -f ${TOOLS}
check:
	${SHELL} nvm.sh
fresh: clean compile
clean:
	rm -f ${TRASH} ${TOOLS}
ignore:
	echo ${TOOLS} | tr ' ' '\n' > .gitignore

# ====================================================================
# rules;
# --------------------------------------------------------------------

%: %.o
	${CC} -o ${@} ${^} ${LDFLAGS}
%.o: %.c
	${CC} -c ${<} ${CFLAGS}
%.h: %.h

# ====================================================================
# programs; 
# --------------------------------------------------------------------

chknvm: chknvm.o checksum32.o error.o fdchecksum32.o getoptv.o fdmanifest.o manifest.o nvm.o nvmpeek.o nvmpeek1.o nvmpeek2.o putoptv.o sdrampeek.o strfbits.o version.o hexout.o
chknvm2: chknvm2.o checksum32.o error.o getoptv.o manifest.o nvm.o nvmpeek.o nvmpeek1.o nvmpeek2.o putoptv.o strfbits.o version.o hexout.o
nvmmerge: nvmmerge.o checksum32.o error.o getoptv.o putoptv.o version.o
nvmsplit: nvmsplit.o checksum32.o error.o getoptv.o putoptv.o version.o

# ====================================================================
# includes;
# --------------------------------------------------------------------

include ../tools/tools.mak
include ../ether/ether.mak
include ../key/key.mak
include ../ram/ram.mak
include ../nvm/nvm.mak
include ../plc/plc.mak

