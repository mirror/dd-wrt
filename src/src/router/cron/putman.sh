#!/bin/sh

# putman.sh - install a man page according to local custom
# vixie 27dec93 [original]
#
# $Id: putman.sh,v 1.1 2003/07/09 13:22:58 honor Exp $

PAGE=$1
DIR=$2

SECT=`expr $PAGE : '[a-z]*.\([0-9]\)'`

[ -d $DIR/man$SECT ] && {
	set -x
	cp $PAGE $DIR/man$SECT/$PAGE
	set +x
} || {
	set -x
	nroff -man $PAGE >$DIR/cat$SECT/`basename $PAGE .$SECT`.0
	set +x
}

exit 0
