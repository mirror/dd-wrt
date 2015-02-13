#!/usr/bin/make -f
# file: ether/Makefile

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
FILES=Makefile *.mak *.sh *.c *.h 
TOOLS=efbu efeu efru efsu edru edsu nics 
PAGES=efbu.1 efeu.1 efru.1 efsu.1 edru.1 edsu.1 pcapdevs.1 nics.1

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
	install -m ${SUID_PERM} -o ${OWNER} -g ${GROUP} ${TOOLS} ${BIN}
uninstall:
	cd ${BIN}; rm -f ${TOOLS}
check:
	${SHELL} ether.sh
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
# Build C version;
# --------------------------------------------------------------------

pcapdevs: pcapdevs.o getoptv.o putoptv.o version.o error.o hexdecode.o gethwaddr.o
efbu: efbu.o getoptv.o putoptv.o version.o error.o todigit.o hexencode.o uintspec.o hexdump.o channel.o openchannel.o sendpacket.o readpacket.o closechannel.o todigit.o channel.o basespec.o
efru: efru.o getoptv.o putoptv.o version.o basespec.o uintspec.o todigit.o hexdump.o error.o channel.o openchannel.o closechannel.o sendpacket.o readpacket.o 
efeu: efeu.o getoptv.o putoptv.o version.o basespec.o uintspec.o todigit.o hexdump.o error.o channel.o openchannel.o closechannel.o sendpacket.o readpacket.o memswap.o
efsu: efsu.o getoptv.o putoptv.o version.o error.o todigit.o hexencode.o uintspec.o hexload.o hexdump.o channel.o openchannel.o sendpacket.o readpacket.o closechannel.o todigit.o basespec.o channel.o synonym.o
edru: edru.o getoptv.o putoptv.o version.o error.o hexdump.o hexdecode.o hexstring.o channel.o openchannel.o closechannel.o sendpacket.o readpacket.o uintspec.o todigit.o basespec.o
edsu: edsu.o getoptv.o putoptv.o version.o efreopen.o uintspec.o todigit.o hexdump.o hexencode.o error.o channel.o openchannel.o closechannel.o readpacket.o sendpacket.o basespec.o
nics: nics.o getoptv.o putoptv.o version.o error.o hexdecode.o decdecode.o hexstring.o decstring.o hostnics.o 
client: client.o getoptv.o putoptv.o version.o error.o 
server: server.o getoptv.o putoptv.o version.o error.o 

# ====================================================================
# includes;
# --------------------------------------------------------------------

include ../tools/tools.mak
include ../ether/ether.mak
include ../plc/plc.mak

