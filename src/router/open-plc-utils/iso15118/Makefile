#!/usr/bin/make -f
# file: iso15118/Makefile

# ====================================================================
# environment definitions;
# --------------------------------------------------------------------

include ../make.def

# ====================================================================
# symbols;
# --------------------------------------------------------------------

# CFLAGS+= -DSLAC_DEBUG
LDFLAGS+=-lm
TRASH=*.o *.obj *.exe t t.* *~ *.[0-9][0-9][0-9] 
FILES=Makefile *.mak *.sh *.c *.h 
TOOLS=evse pev
PAGES=evse.1 pev.1

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
	if [ ! -d ${MAN} ]; then install -m ${DIR_PERM} -o ${OWNER} -g ${GROUP} -d ${MAN}; fi
	install -m ${MAN_PERM} -o ${OWNER} -g ${GROUP} ${PAGES} ${MAN}
install: compile library
	install -m ${SUID_PERM} -o ${OWNER} -g ${GROUP} ${TOOLS} ${BIN}
uninstall:
	cd ${BIN}; rm -f plc ${TOOLS}
	cd ${MAN}; rm -f plc ${PAGES}
clean:
	rm -f ${TRASH} ${TOOLS}
check:
	${SHELL} iso15118.sh
ignore:
	echo ${TOOLS} | tr ' ' '\n' > .gitignore

# ====================================================================
# rules;        
# --------------------------------------------------------------------

%: %.o
	${CC} -o ${@} ${^} ${LDFLAGS}
%.o: %.c
	${CC} -c ${<} ${CFLAGS}

# ====================================================================
# programs;     
# --------------------------------------------------------------------

evse: evse.o Devices.o EthernetHeader.o QualcommHeader.o HomePlugHeader1.o UnwantedMessage.o channel.o closechannel.o config.o decdecode.o decstring.o error.o evse_cm_atten_char.o evse_cm_mnbc_sound.o evse_cm_slac_match.o evse_cm_slac_param.o evse_cm_start_atten_char.o evse_cm_set_key.o getoptv.o hexdecode.o hexdump.o hexencode.o hexout.o hexstring.o openchannel.o putoptv.o readmessage.o readpacket.o sendmessage.o sendpacket.o slac_session.o strfbits.o todigit.o uintspec.o version.o slac_debug.o
pev: pev.o Devices.o EthernetHeader.o QualcommHeader.o HomePlugHeader1.o UnwantedMessage.o channel.o closechannel.o config.o decdecode.o decstring.o error.o getoptv.o hexdecode.o hexdump.o hexencode.o hexstring.o memincr.o openchannel.o pev_cm_atten_char.o pev_cm_mnbc_sound.o pev_cm_slac_match.o pev_cm_slac_param.o pev_cm_start_atten_char.o pev_cm_set_key.o putoptv.o readmessage.o readpacket.o sendmessage.o sendpacket.o slac_connect.o slac_session.o strfbits.o todigit.o uintspec.o version.o slac_debug.o

# ====================================================================
# includes;
# --------------------------------------------------------------------

include ../tools/tools.mak
include ../ether/ether.mak
include ../mme/mme.mak
include ../plc/plc.mak
include ../key/key.mak

