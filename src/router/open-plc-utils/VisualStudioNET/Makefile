#!/usr/bin/make -f
# file: VisualStudioNET/Makefile

# ====================================================================
# environment definitions;
# --------------------------------------------------------------------

include ../make.def

# ====================================================================
# 
# --------------------------------------------------------------------

CFLAGS+=
LDFLAGS+=
TRASH=*.o t t.* *.[0-9][0-9][0-9] *~ *.log *.obj *.exe *.suo *.ncb 
TOOLS=

# ====================================================================
# pseudo targets;
# --------------------------------------------------------------------

all compile: ${TOOLS}
library:
#	install -m ${DIR_PERM} -o ${OWNER} -g ${GROUP} -d ${BIN}
scripts manuals:
install: $(TOOLS) library
#	install -m ${BIN_PERM} -o ${OWNER} -g ${GROUP} ${TOOLS} ${BIN}
uninstall:
#	cd ${BIN}; rm -f ${TOOLS}
#	cd ${MAN}; rm -f ${PAGES}
fresh: clean compile
check:
clean:
	rm -fr _UpgradeReport_Files *.XML .#*
	rm -fr */[Dd]ebug */[Rr]elease */*.[0-9][0-9][0-9] 
	rm -fr Objects/*/*.?db */*.log */*.tlog */*/*.manifest
	rm -f ${TOOLS} ${TRASH} 
ignore:
#
#	Do not over-write the local .gitignore file; we maintain it
#	by hand for now;
#
#	echo ${TOOLS} | tr ' ' '\n' > .gitignore
#


