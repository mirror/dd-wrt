#! /bin/sh

if test -z "$AWK"; then
  AWK=awk
fi
if test -z "$CC"; then
  CC=cc
fi
if test -z "$srcdir"; then
  srcdir=.
fi

LC_ALL=C
export LC_ALL

rm -f comm.h
cat << EOF > comm.h
/*
 * This file is automagically created from comm.c -- DO NOT EDIT
 */

struct comm
{
  char *name;
  int flags;
#ifdef MULTIUSER
  AclBits userbits[ACL_BITS_PER_CMD];
#endif
};

#define ARGS_MASK	(3)

#define ARGS_0	(0)
#define ARGS_1	(1)
#define ARGS_2	(2)
#define ARGS_3	(3)

#define ARGS_PLUS1	(1<<2)
#define ARGS_PLUS2	(1<<3)
#define ARGS_PLUS3	(1<<4)
#define ARGS_ORMORE	(1<<5)

#define NEED_FORE	(1<<6)	/* this command needs a fore window */
#define NEED_DISPLAY	(1<<7)	/* this command needs a display */
#define NEED_LAYER	(1<<8)	/* this command needs a layer */
#define CAN_QUERY	(1<<9)  /* this command can be queried, i.e. used with -Q to
				   get back a result to stdout */

#define ARGS_01		(ARGS_0 | ARGS_PLUS1)
#define ARGS_02		(ARGS_0 | ARGS_PLUS2)
#define ARGS_12		(ARGS_1 | ARGS_PLUS1)
#define ARGS_23		(ARGS_2 | ARGS_PLUS1)
#define ARGS_24		(ARGS_2 | ARGS_PLUS2)
#define ARGS_34		(ARGS_3 | ARGS_PLUS1)
#define ARGS_012	(ARGS_0 | ARGS_PLUS1 | ARGS_PLUS2)
#define ARGS_0123	(ARGS_0 | ARGS_PLUS1 | ARGS_PLUS2 | ARGS_PLUS3)
#define ARGS_123	(ARGS_1 | ARGS_PLUS1 | ARGS_PLUS2)
#define ARGS_124	(ARGS_1 | ARGS_PLUS1 | ARGS_PLUS3)
#define ARGS_1234	(ARGS_1 | ARGS_PLUS1 | ARGS_PLUS2 | ARGS_PLUS3)

struct action
{
  int nr;
  char **args;
  int *argl;
  int quiet;	/* Suppress (currently unused)
		   0x01 - Error message
		   0x02 - Normal message
		*/
};

#define RC_ILLEGAL -1

EOF
$AWK < ${srcdir}/comm.c >> comm.h '
/^  [{] ".*/	{   if (old > $2) {
		printf("***ERROR: %s <= %s !!!\n\n", $2, old);
		exit 1;
	    }
	old = $2;
	}
'

$CC -E -I. -I${srcdir} ${srcdir}/comm.c > comm.cpp
sed < comm.cpp \
  -n \
  -e '/^ *{ "/y/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/' \
  -e '/^ *{ "/s/^ *{ "\([^"]*\)".*/\1/p' \
| $AWK '
/.*/ {	printf "#define RC_%s %d\n",$0,i++;
     }
END  {	printf "\n#define RC_LAST %d\n",i-1;
     }
' >> comm.h
chmod a-w comm.h
rm -f comm.cpp
