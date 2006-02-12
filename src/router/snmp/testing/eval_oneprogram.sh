#!/bin/sh
#
# eval_oneprogram.sh [-h][-lk] <program> [<program_args>]
#
# CALLED BY: eval_suite.sh
#
#
# Runs <program> and reports how many instances of the strings SUCCESS
# or FAILED occur.
#
#
# FIX	Output from $PROGRAM on stderr is separated out and comes first.
#
#
USAGE_LONG='
#
# -h	Help.
# -k	Save the program output in "__<program_args>__<pid>o".
# -l	Long form.  (Short form by default.)
#
# <program> is the executable to run and collect the output of.
'

USAGE="Usage: `basename $0` [-h][-lk] <program>"




#------------------------------------ -o- 
# Globals.
#
AWKFILE="_`basename $0`$$.awk"
SCRIPTFILE=

dolongform=0
dokeepoutput=

TOTALFAILED=0



#------------------------------------ -o- 
# Parse & setup.
#
while [ -n "$1" ]; do
	case "$1" in
	-k)	dokeepoutput=true
		;;
	-l)	dolongform=1
		;;
	-h)	echo $USAGE
		cat <<BLIK | sed 's/^#//' | sed '1d'    1>&2
$USAGE_LONG
BLIK
		exit 0
		;;
	*)	PROGRAM="$*"
		shift `expr $# - 1`
		;;
	esac

	shift
done

[ -z "$PROGRAM" ] && echo $USAGE && exit 1


SCRIPTFILE="__`echo \`basename $PROGRAM\` | sed 's/ /_/g'`__$$o"



#------------------------------------ -o- 
# Create awk script.
#

cat <<GRONK >$AWKFILE

BEGIN {
	pass = 0
	passlist[0] = ""

	fail = 0
	faillist[0] = ""

	longform = $dolongform + 0
}

/SUCCESS/	{
	passlist[pass] = \$0
	pass += 1
}

/FAILED/	{
	faillist[fail] = \$0
	fail += 1
}

END {
	printf "$PROGRAM SUCCESS: %d\n", pass
	printf "$PROGRAM FAILED: %d\n", fail

	if (longform) {
		printf "\n"
		for (i=0; i<pass; i++)
			print passlist[i]

		for (i=0; i<fail; i++)
			print faillist[i]
	}

	exit fail
}
GRONK




#------------------------------------ -o- 
# Get and print results.
#

{ $PROGRAM $* 2>&1 ; } >$SCRIPTFILE

awk -f $AWKFILE $SCRIPTFILE
TOTALFAILED=$?

rm -f $AWKFILE
[ -z "$dokeepoutput" ] && rm -f $SCRIPTFILE




#------------------------------------ -o- 
# Exit, cleanup.
#
exit $TOTALFAILED


