
# AIX Makefile
#
# $Id: Makefile,v 1.14 2008/04/15 13:28:12 abe Exp $

PROG=	lsof

BIN=	${DESTDIR}

DOC=	${DESTDIR}

I=/usr/include
S=/usr/include/sys
L=/usr/include/local
P=

CDEF=
CDEFS=  ${CDEF} ${CFGF}
INCL=	${DINC}
CFLAGS=	${CDEFS} ${INCL} ${DEBUG}

GRP=

HDR=    lsof.h lsof_fields.h dlsof.h machine.h proto.h dproto.h

SRC=    ddev.c dfile.c dmnt.c dnode.c dnode1.c dnode2.c dproc.c dsock.c \
	dstore.c arg.c main.c misc.c node.c print.c proc.c store.c usage.c \
	util.o

OBJ=	ddev.o dfile.o dmnt.o dnode.o dnode1.o dnode2.o dproc.o dsock.o \
	dstore.o arg.o main.o misc.o node.o print.o proc.o store.o usage.o \
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
	@echo 'Please write your own install rule.  Lsof Needs to be able to'
	@echo 'read /dev/kmem and /dev/mem'.  Installing it segid to the group'
	@echo 'that can read those devices is one way to allow it to read them.'
	@echo 'normally that group is the system group and your install rule'
	@echo 'might look something like this:'
	@echo ''
	@echo '    install -f $${BIN} -S -M 2755 -G $${GRP} $${PROG}'
	@echo '    install -f $${DOC} -M 444 $${MAN}'
	@echo ''
	@echo 'You will have to complete the skeletons for the BIN, DOC, and'
	@echo 'GRP strings given at the beginning of this Makefile, e.g.,'
	@echo ''
	@echo '    BIN= $${DESTDIR}/usr/local/etc'
	@echo '    DOC= $${DESTDIR}/usr/man/man8'
	@echo '    GRP= system'
	@echo ''
	@echo 'You might also consider giving lsof permission to read /dev/kmem'
	@echo 'and /dev/mem via ACLs.  First, establish a new group to which'
	@echo 'you will setgid lsof -- e.g., group kmem.  Next, change the ACLs'
	@echo 'for /dev/kmem and /dev/mem to permit group kmem members to read'
	@echo 'them.  Finally, install lsof setgid the kmem group with:
	@echo ''
	@echo '    GRP= kmem'

${LIB}:	FRC
	(cd lib; ${MAKE} DEBUG="${DEBUG}" CFGF="${CFGF}")

version.h:	FRC
	@echo Constructing version.h
	@rm -f version.h
	@echo '#define	LSOF_BLDCMT	"${LSOF_BLDCMT}"' > version.h;
	@echo '#define	LSOF_CC		"${CC}"' >> version.h
	@echo '#define	LSOF_CCV	"${CCV}"' >> version.h
	@echo '#define	LSOF_CCDATE	"'`date`'"' >> version.h
	@echo '#define	LSOF_CCFLAGS	"'`echo ${CFLAGS} | sed 's/\\\\(/\\(/g' | sed 's/\\\\)/\\)/g' | sed 's/"/\\\\"/g'`'"' >> version.h
	@echo '#define	LSOF_CINFO	"${CINFO}"' >> version.h
	@if [ "X${LSOF_HOST}" = "X" ]; then \
	  echo '#define	LSOF_HOST	"'`uname -n`'"' >> version.h; \
	else \
	  if [ "${LSOF_HOST}" = "none" ]; then \
	    echo '#define	LSOF_HOST	""' >> version.h; \
	  else \
	    echo '#define	LSOF_HOST	"${LSOF_HOST}"' >> version.h; \
	  fi \
	fi
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
	@if [ "X${LSOF_SYSINFO}" = "X" ]; then \
	    echo '#define	LSOF_SYSINFO	"'`uname -a`'"' >> version.h; \
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

dnode2.o:	${HDR} dnode2.c

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
