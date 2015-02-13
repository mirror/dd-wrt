#!/usr/bin/make -f
# file: pib/Makefile

# ====================================================================
# environment definitions;
# --------------------------------------------------------------------

include ../make.def

# ====================================================================
# symbols;
# --------------------------------------------------------------------

CFLAGS+=
LDFLAGS+=-lm
TRASH=*.o *.obj *.exe t t.* *~ *.[0-9][0-9][0-9] *.test 
FILES=Makefile *.mak *.sh *.txt *.c *.h *.xml *.css
TOOLS=chkpib chkpib2 setpib getpib modpib pib2xml pibcomp pibdump pibruin xml2pib psin psout pskey
TOOLS+=psgraph psnotch pibrump 
PAGES=chkpib.1 chkpib2.1 setpib.1 getpib.1 modpib.1 pib2xml.1 pibcomp.1 pibdump.1 xml2pib.1 psin.1 psout.1 pskey.1
# PAGES+=pibrump.1 pibruin.1 psgraph.1 psnotch.1

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
	${SHELL} pib.sh
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

fetchpib: fetchpib.o getoptv.o putoptv.o version.o checksum32.o error.o 
chkpib: chkpib.o getoptv.o putoptv.o version.o pibpeek1.o pibpeek2.o checksum32.o fdchecksum32.o  keys.o hexstring.o error.o hexdecode.o SHA256.o HPAVKeyNID.o fdmanifest.o manifest.o strfbits.o
chkpib2: chkpib2.o getoptv.o putoptv.o version.o pibpeek1.o pibpeek2.o checksum32.o fdchecksum32.o keys.o hexstring.o error.o hexdecode.o SHA256.o HPAVKeyNID.o manifest.o strfbits.o
modpib: modpib.o getoptv.o putoptv.o version.o hexencode.o hexdecode.o todigit.o memincr.o fdchecksum32.o keys.o uintspec.c error.o pibpeek1.o pibpeek2.o hexstring.o pibfile.o pibfile1.o pibfile2.o SHA256.o HPAVKeyNID.o checksum32.o
getpib: getpib.o getoptv.o putoptv.o version.o uintspec.o basespec.o todigit.o hexout.o error.o nvmseek2.o checksum32.o fdchecksum32.o
mrgpib: mrgpib.o getoptv.o putoptv.o version.o error.o fdchecksum32.o pibfile1.o
setpib: setpib.o getoptv.o putoptv.o version.o hexpeek.o hexdump.o dataspec.o basespec.o error.o todigit.o uintspec.o bytespec.o pibfile1.o checksum32.o fdchecksum32.o memencode.o nvmseek2.o
setpib1: setpib1.o getoptv.o putoptv.o version.o hexpeek.o hexdump.o dataspec.o basespec.o error.o todigit.o uintspec.o bytespec.o pibfile1.o checksum32.o fdchecksum32.o memencode.o
setpib2: setpib2.o getoptv.o putoptv.o version.o hexpeek.o hexdump.o dataspec.o basespec.o error.o todigit.o uintspec.o bytespec.o pibfile1.o checksum32.o fdchecksum32.o memencode.o
psin: psin.o getoptv.o putoptv.o version.o hexdecode.o hexstring.o todigit.o error.o pibfile1.o piblock.o pibscalers.o fdchecksum32.o 
pskey: pskey.o getoptv.o putoptv.o version.o error.o hexdecode.o hexstring.o hexout.o pibfile1.o SHA256Reset.o SHA256Write.o SHA256Block.o SHA256Fetch.o keys.o fdchecksum32.o
psout: psout.o getoptv.o putoptv.o version.o error.o pibfile.o pibfile1.o pibfile2.o pibscalers.o checksum32.o fdchecksum32.o
psnotch: psnotch.o getoptv.o putoptv.o version.o error.o todigit.o uintspec.o
psgraph: psgraph.o getoptv.o putoptv.o version.o error.o todigit.o uintspec.o pibscalers.o
pibrump: pibrump.o getoptv.o putoptv.o version.o error.o pibfile1.o fdchecksum32.o reword.o hexstring.o hexdecode.o memout.o rules.o
pibruin: pibruin.o getoptv.o putoptv.o version.o error.o pibfile1.o nvmseek2.o checksum32.o fdchecksum32.o reword.o getargv.o hexstring.o hexdecode.o memout.o lookup.o assist.o bytespec.o basespec.o ipv4spec.o ipv6spec.o todigit.o codelist.o emalloc.o rules.o ParseRule.o piblock.o ruledump.o 
pib2xml: pib2xml.o checksum32.o error.o fdchecksum32.o getoptv.o hexview.o output.o nvmseek2.o putoptv.o version.o xmlschema.o
pibdump: pibdump.o getoptv.o putoptv.o version.o hexoffset.o hexview.o emalloc.o error.o checksum32.o fdchecksum32.o nvmseek2.o 
pibcomp: pibcomp.o getoptv.o putoptv.o version.o hexoffset.o hexview.o emalloc.o error.o checksum32.o fdchecksum32.o nvmseek2.o
xml2pib: xml2pib.o HPAVKeyNID.o SHA256Block.o SHA256Fetch.o SHA256Reset.o SHA256Write.o checksum32.o emalloc.o error.o fdchecksum32.o getoptv.o hexdecode.o hexstring.o keys.o manifest.o pibpeek1.o pibpeek2.o putoptv.o strfbits.o todigit.o version.o xmledit.o xmlfree.o xmlnode.o xmlopen.o xmlscan.o xmltree.o output.o

# ====================================================================
# includes;
# --------------------------------------------------------------------

include ../tools/tools.mak
include ../nodes/nodes.mak
include ../key/key.mak
include ../pib/pib.mak
include ../plc/plc.mak



