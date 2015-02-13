#!/usr/bin/make -f
# file: crypt/Makefile

# ====================================================================
# environment definitions;
# --------------------------------------------------------------------

include ../make.def

# ====================================================================
# symbols;
# --------------------------------------------------------------------

CFLAGS+=
LDFLAGS+=
TRASH=*.o *.obj *.exe t t.* *~ *.[0-9][0-9][0-9] *.a
TOOLS=hpavkey hpavkeys rkey mac2pw mac2pwd
LIBS=libkeys.a
FILES=*.c *.h *.sh *.xml *.txt *.mak Makefile Makefile.*
PAGES=hpavkey.1 hpavkeys.1 rkey.1 mac2pw.1 mac2pwd.1

# ====================================================================
# pseudo targets;
# --------------------------------------------------------------------

all compile: ${TOOLS} ${LIBS}
compact: compile
	if [ ! -z ${STRIP} ]; then ${STRIP} ${TOOLS}; fi
	if [ -x /usr/local/bin/upx ]; then upx --best ${TOOLS}; fi
library:
	install -m ${DIR_PERM} -o ${OWNER} -g ${GROUP} -d ${BIN}
scripts:
manuals:
	test -d ${MAN} || install -m ${DIR_PERM} -o ${OWNER} -g ${GROUP} -d ${MAN}
	install -m ${MAN_PERM} -o ${OWNER} -g ${GROUP} ${PAGES} ${MAN}
install: $(TOOLS)
	install -m ${BIN_PERM} -o ${OWNER} -g ${GROUP} ${TOOLS} ${BIN}
uninstall:
	cd ${BIN}; rm -f ${TOOLS}
	cd ${MAN}; rm -f ${PAGES}
check:
	${SHELL} key.sh
fresh: clean compile
clean:
	rm -f ${TOOLS} ${TRASH}
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
# targets;
# --------------------------------------------------------------------

hpavkey: hpavkey.o HPAVKeyDAK.o HPAVKeyNID.o HPAVKeyNMK.o HPAVKeyOut.o HPAVKeySHA.o SHA256.o error.o getoptv.o hexout.o putoptv.o todigit.o uintspec.o version.o
hpavkeys: hpavkeys.o HPAVKeyDAK.o HPAVKeyNID.o HPAVKeyNMK.o HPAVKeyOut.o HPAVKeySHA.o SHA256.o error.o getoptv.o hexout.o putoptv.o todigit.o uintspec.o version.o
mac2pw: mac2pw.o MACPasswords.o RNDPasswords.o putpwd.o error.o getoptv.o putoptv.o todigit.o uintspec.o version.o
mac2pwd: mac2pwd.o MACPasswords.o RNDPasswords.o putpwd.o error.o getoptv.o putoptv.o todigit.o uintspec.o version.o
rkey: rkey.o HPAVKeyDAK.o HPAVKeyNID.o HPAVKeyNMK.o HPAVKeyOut.o HPAVKeySHA.o SHA256Block.o SHA256Fetch.o SHA256Reset.o SHA256Write.o error.o getoptv.o hexout.o putoptv.o strincr.o todigit.o uintspec.o version.o
libkeys.a: HPAVKeySHA.o HPAVKeyDAK.o HPAVKeyNMK.o HPAVKeyNID.o HPAVKeyOut.o SHA256.o strnpwd.o
	rm -f ${@}
	$(AR) rc ${@} ${^}
	$(RANLIB) ${@}

# ====================================================================
# 
# --------------------------------------------------------------------

include ../tools/tools.mak
include ../key/key.mak

