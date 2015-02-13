#!/usr/bin/make -f
# file: serial/Makefile

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
TOOLS=int6kuart int6kbaud ttysig ptsctl weeder ttysend ttyrecv ttycat int6kdetect 
PAGES=int6kuart.1 int6kbaud.1 ttysig.1 ptsctl.1 weeder.1 ttycat.1 ttysend.1 ttyrecv.1 int6kdetect.1

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
	test -d ${MAN} || install -m ${DIR_PERM} -o ${OWNER} -g ${GROUP} -d ${MAN}
	install -m ${MAN_PERM} -o ${OWNER} -g ${GROUP} ${PAGES} ${MAN}
install: $(TOOLS) library
	install -m ${BIN_PERM} -o ${OWNER} -g ${GROUP} ${TOOLS} ${BIN}
uninstall:
	cd ${BIN}; rm -f int6k ${TOOLS}
	cd ${MAN}; rm -f int6k ${PAGES}
check:
	${SHELL} serial.sh
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

int6kuart: int6kuart.o getoptv.o putoptv.o version.o uintspec.o basespec.o todigit.o error.o checksum32.o hexencode.o hexdump.o hexstring.o hexdecode.o openport.o closeport.o serial.o
int6kbaud: int6kbaud.o getoptv.o putoptv.o version.o uintspec.o basespec.o todigit.o error.o checksum32.o hexencode.o hexdump.o hexstring.o hexdecode.o synonym.o openport.o closeport.o serial.o
ttysig: ttysig.o getoptv.o putoptv.o version.o error.o uintspec.o todigit.o
ttysend: ttysend.o getoptv.o putoptv.o version.o error.o uintspec.o todigit.o baudrate.o
int6kdetect: int6kdetect.o getoptv.o putoptv.o version.o error.o baudrate.o
ttyrecv: ttyrecv.o getoptv.o putoptv.o version.o error.o uintspec.o todigit.o baudrate.o
weeder: weeder.o getoptv.o putoptv.o version.o uintspec.o synonym.o todigit.o error.o
ptsctl: ptsctl.o getoptv.o putoptv.o version.o uintspec.o synonym.o todigit.o error.o
ttycat: ttycat.o getoptv.o putoptv.o version.o efreopen.o error.o baudrate.o uintspec.o todigit.o

# ====================================================================
# includes;
# --------------------------------------------------------------------

include ../tools/tools.mak
include ../serial/serial.mak
include ../plc/plc.mak

