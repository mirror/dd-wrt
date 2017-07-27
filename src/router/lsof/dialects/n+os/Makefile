
# N+OS Makefile
#
# $Id: Makefile,v 1.14 2008/04/15 13:30:27 abe Exp $

PROG=	lsof

BIN=	${DESTDIR}

DOC=	${DESTDIR}

I=/usr/include
S=/usr/include/sys
L=/usr/include/local
P=

# Use the RC_CFLAGS environment variable to define architecture types.
# To create a "fat" executable, supporting more than one architecture,
# set RC_CFLAGS before executing make -- e.g., to build a "fat" executable
# for m68k, i486, hppa, and SPARC, using /bin/sh:
#
#	$ cd <lsof_source_directory>
#	$ ./Configure ns
#	$ RC_CFLAGS="-arch m68k -arch i486 -arch hppa -arch sparc"
#	$ export RC_CFLAGS
#	$ make

CDEF=	${RC_CFLAGS}
CDEFS=  ${CDEF} ${CFGF}
INCL=	${DINC}
CFLAGS=	${CDEFS} ${INCL} ${DEBUG}

GRP=

HDR=    lsof.h lsof_fields.h dlsof.h machine.h proto.h dproto.h

SRC=    dnode.c denode1.c dproc.c dsock.c dstore.c \
	arg.c main.c misc.c node.c print.c proc.c store.c usage.c \
	util.c

OBJ=	dnode.o dnode1.o  dproc.o dsock.o dstore.o \
	arg.o main.o misc.o node.o print.o proc.o store.o usage.o \
	util.o

MAN=	lsof.8

OTHER=	

SHELL=	/bin/sh

SOURCE=	Makefile ${OTHER} ${MAN} ${HDR} ${SRC}

all: ${PROG}

${PROG}: ${LIB} ${P} ${OBJ}
	${CC} -o $@ ${CFLAGS} ${OBJ} ${CFGL}

clean: FRC
	rm -f Makefile.bak ${PROG} a.out core errs lint.out tags *.o version.h
	rm -f machine.h.old new_machine.h
	(cd lib; ${MAKE} -f Makefile.skel clean)

install: all FRC
	@echo ''
	@echo 'Please write your own install rule.  Lsof should be installed'
	@echo 'setgid to the group that can can read /dev/kmem.  Normally'
	@echo 'that is the kmem group.  Your install rule actions might look'
	@echo 'something like this:'
	@echo ''
	@echo '    install -cs -m 2755 -g $${GRP} $${PROG} $${BIN}/$${PROG}'
	@echo '    install -c -m 444 $${MAN} $${DOC}/$${MAN}'
	@echo ''
	@echo 'You will have to complete the skeletons for the BIN, DOC, and'
	@echo 'GRP strings given at the beginning of this Makefile, e.g.,'
	@echo ''
	@echo '    BIN= $${DESTDIR}/usr/local/etc'
	@echo '    DOC= $${DESTDIR}/usr/man/man8'
	@echo '    GRP= kmem'
	@echo ''

${LIB}: FRC
	(cd lib; ${MAKE} DEBUG="${DEBUG}" CFGF="${CFGF}")

version.h:	FRC
	@echo Constructing version.h
	@rm -f version.h
	@echo '#define	LSOF_BLDCMT	"${LSOF_BLDCMT}"' > version.h;
	@echo '#define	LSOF_CC		"${CC}"' >> version.h
	@echo '#define	LSOF_CCV	"${CCV}"' >> version.h
	@echo '#define	LSOF_CCDATE	"'`date`'"' >> version.h
	@echo '#define	LSOF_CCFLAGS	"'`echo ${CFLAGS} | sed 's/\\\\(/\\(/g' | sed 's/\\\\)/\\)/g' | sed 's/"/\\\\"/g'`'"' >> version.h
	@echo '#define	LSOF_LDFLAGS	"${CFGL}"' >> version.h
	@if [ "X${LSOF_LOGNAME}" = "X" ]; then \
	  echo '#define	LSOF_LOGNAME	"${LOGNAME}"' >> version.h; \
	else \
	  if [ "${LSOF_LOGNAME}" = "none" ]; then \
	    echo '#define	LSOF_LOGNAME	""' >> version.h; \
	  else \
	    echo '#define	LSOF_LOGNAME	"${LSOF_LOGNAME}"' >> version.h; \
	  fi; \
	fi
	@if [ "X${LSOF_HOST}" = "X" ]; then \
	  echo '#define	LSOF_HOST	"'`hostname`'"' >> version.h; \
	else \
	  if [ "${LSOF_HOST}" = "none" ]; then \
	    echo '#define	LSOF_HOST	""' >> version.h; \
	  else \
	    echo '#define	LSOF_HOST	"${LSOF_HOST}"' >> version.h; \
	  fi \
	fi
	@if [ "X${LSOF_SYSINFO}" = "X" ]; then \
	    echo '#define	LSOF_SYSINFO	"'`hostinfo | head -2 | tail -1`'"' >> version.h; \
	else \
	  if [ "${LSOF_SYSINFO}" = "none" ]; then \
	    echo '#define	LSOF_SYSINFO	""' >> version.h; \
	  else \
	    echo '#define	LSOF_SYSINFO	"${LSOF_SYSINFO}"' >> version.h; \
	  fi \
	fi
	@if [ "X${LSOF_USER}" = "X" ]; then \
	  echo '#define	LSOF_USER	"${USER}"' >> version.h; \
	else \
	  if [ "${LSOF_USER}" = "none" ]; then \
	    echo '#define	LSOF_USER	""' >> version.h; \
	  else \
	    echo '#define	LSOF_USER	"${LSOF_USER}"' >> version.h; \
	  fi \
	fi
	@sed '/VN/s/.ds VN \(.*\)/#define	LSOF_VERSION	"\1"/' < version >> version.h

FRC:

# DO NOT DELETE THIS LINE - make depend DEPENDS ON IT

ddev.o:		${HDR} ddev.c

dfile.o:	${HDR} dfile.c

dmnt.o:		${HDR} dmnt.c

dnode.o:	${HDR} dnode.c

dnode1.o:	${HDR} dnode1.c

dproc.o:	${HDR} dproc.c

dsock.o:	${HDR} dsock.c

dstore.o:	${HDR} dstore.c

arg.o:		${HDR} arg.c

main.o:		${HDR} main.c

misc.o:		${HDR} misc.c

node.o:		${HDR} node.c

print.o:	${HDR} print.c

proc.o:		${HDR} proc.c

store.o:	${HDR} store.c

usage.o:	${HDR} version.h usage.c

util.o:		${HDR} util.c

# *** Do not add anything here - It will go away. ***
