###############################################################################
#
# Copyright (C) 2005 Red Hat, Inc. All Rights Reserved.
# Written by David Howells (dhowells@redhat.com)
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version
# 2 of the License, or (at your option) any later version.
#
###############################################################################

echo === $OUTPUTFILE ===

endian=`file -L /proc/$$/exe`
if expr "$endian" : '.* MSB \+\(executable\|shared object\).*' >&/dev/null
then
    endian=BE
elif expr "$endian" : '.* LSB \+\(executable\|shared object\).*' >&/dev/null
then
    endian=LE
else
    echo -e "+++ \e[31;1mCan't Determine Endianness\e[0m"
    echo "+++ Can't Determine Endianness" >>$OUTPUTFILE
    exit 2
fi

maxtypelen=31
maxtype=`for ((i=0; i<$((maxtypelen)); i++)); do echo -n a; done`

PAGE_SIZE=`getconf PAGESIZE`
pagelen=$((PAGE_SIZE - 1))
fullpage=`for ((i=0; i<$((pagelen)); i++)); do echo -n a; done`
string4095=`for ((i=0; i<4095; i++)); do echo -n a; done`

if kernel_at_or_later_than 3.18
then
    maxdesc=$string4095
elif rhel6_kernel_at_or_later_than 2.6.32-589.el6
then
    maxdesc=$string4095
else
    maxdesc=$fullpage
fi

maxcall=$fullpage

maxsquota=`grep '^ *0': /proc/key-users | sed s@.*/@@`

key_gc_delay_file="/proc/sys/kernel/keys/gc_delay"
if [ -f $key_gc_delay_file ]; then
    orig_gc_delay=`cat $key_gc_delay_file`
else
    orig_gc_delay=300
fi


function marker ()
{
    echo -e "+++ \e[33m$*\e[0m"
    echo +++ $* >>$OUTPUTFILE
}

function failed()
{
    echo -e "\e[31;1mFAILED\e[0m"
    echo === FAILED === >>$OUTPUTFILE
    keyctl show >>$OUTPUTFILE
    echo ============== >>$OUTPUTFILE
    result=FAIL
}

function expect_args_error ()
{
    "$@" >>$OUTPUTFILE 2>&1
    if [ $? != 2 ]
    then
	failed
    fi

}

function toolbox_report_result()
{
    if [ $RUNNING_UNDER_RHTS = 1 ]
    then
	report_result $1 $2
    fi
    if [ $2 = FAIL ]
    then
	exit 1
    fi
}

function toolbox_skip_test()
{
    echo "++++ SKIPPING TEST" >>$OUTPUTFILE
    marker "$2"
    toolbox_report_result $1 PASS
}

###############################################################################
#
# Return true if the command is found in $PATH. If not, log that the test is
# being skipped, report the result as PASS, and exit.
#
###############################################################################
function require_command ()
{
    which "$1" >&/dev/null
    if [ $? != 0 ]
    then
	toolbox_skip_test "SKIP DUE TO MISSING COMMAND: $1"
        exit 0
    fi
}

function require_selinux ()
{
    if ! grep -q selinuxfs /proc/mounts;
    then
	toolbox_skip_test $TEST "SKIP DUE TO DISABLED SELINUX"
	exit 0
    fi
}

###############################################################################
#
# extract an error message from the log file and check it
#
###############################################################################
function expect_error ()
{
    my_varname=$1

    my_errmsg="`tail -1 $OUTPUTFILE`"
    eval $my_varname="\"$my_errmsg\""

    if [ $# != 1 ]
    then
	echo "Format: expect_error <symbol>" >>$OUTPUTFILE
	failed
    fi

    case $1 in
	EPERM)		my_err="Operation not permitted";;
	EAGAIN)		my_err="Resource temporarily unavailable";;
	ENOENT)		my_err="No such file or directory";;
	EEXIST)		my_err="File exists";;
	ENOTDIR)	my_err="Not a directory";;
	EACCES)		my_err="Permission denied";;
	EINVAL)		my_err="Invalid argument";;
	ENODEV)		my_err="No such device";;
	ELOOP)		my_err="Too many levels of symbolic links";;
	EOPNOTSUPP)	my_err="Operation not supported";;
	EDEADLK)	my_err="Resource deadlock avoided";;
	EDQUOT)		my_err="Disk quota exceeded";;
	ENOKEY)
	    my_err="Required key not available"
	    old_err="Requested key not available"
	    alt_err="Unknown error 126"
	    ;;
	EKEYEXPIRED)
	    my_err="Key has expired"
	    alt_err="Unknown error 127"
	    ;;
	EKEYREVOKED)
	    my_err="Key has been revoked"
	    alt_err="Unknown error 128"
	    ;;
	EKEYREJECTED)
	    my_err="Key has been rejected"
	    alt_err="Unknown error 129"
	    ;;
	*)
	    echo "Unknown error message $1" >>$OUTPUTFILE
	    failed
	    ;;
    esac

    if expr "$my_errmsg" : ".*: $my_err" >&/dev/null
    then
	:
    elif [ "x$alt_err" != "x" ] && expr "$my_errmsg" : ".*: $alt_err" >&/dev/null
    then
	:
    elif [ "x$old_err" != "x" ] && expr "$my_errmsg" : ".*: $old_err" >&/dev/null
    then
	:
    else
	failed
    fi
}

###############################################################################
#
# wait for a key to be destroyed (get removed from /proc/keys)
#
###############################################################################
function pause_till_key_destroyed ()
{
    echo "+++ WAITING FOR KEY TO BE DESTROYED" >>$OUTPUTFILE
    hexkeyid=`printf %08x $1`

    while grep $hexkeyid /proc/keys
    do
	sleep 1
    done
}

###############################################################################
#
# wait for a key to be unlinked
#
###############################################################################
function pause_till_key_unlinked ()
{
    echo "+++ WAITING FOR KEY TO BE UNLINKED" >>$OUTPUTFILE

    while true
    do
	echo keyctl unlink $1 $2 >>$OUTPUTFILE
	keyctl unlink $1 $2 >>$OUTPUTFILE 2>&1
	if [ $? != 1 ]
	then
	    failed
	fi

	my_errmsg="`tail -1 $OUTPUTFILE`"
	if ! expr "$my_errmsg" : ".*: No such file or directory" >&/dev/null
	then
	    break
	fi
	sleep 1
    done
}

###############################################################################
#
# request a key and attach it to the new keyring
#
###############################################################################
function request_key ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl request "$@" >>$OUTPUTFILE
    keyctl request "$@" >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# request a key and attach it to the new keyring, calling out if necessary
#
###############################################################################
function request_key_callout ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl request2 "$@" >>$OUTPUTFILE
    keyctl request2 "$@" >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# request a key and attach it to the new keyring, calling out if necessary and
# passing the callout data in on stdin
#
###############################################################################
function prequest_key_callout ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    data="$1"
    shift

    echo echo -n $data \| keyctl prequest2 "$@" >>$OUTPUTFILE
    echo -n $data | keyctl prequest2 "$@" >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# create a key and attach it to the new keyring
#
###############################################################################
function create_key ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl add "$@" >>$OUTPUTFILE
    keyctl add "$@" >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# create a key and attach it to the new keyring, piping in the data
#
###############################################################################
function pcreate_key ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    data="$1"
    shift

    echo echo -n $data \| keyctl padd "$@" >>$OUTPUTFILE
    echo -n $data | keyctl padd "$@" >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# create a key and attach it to the new keyring, piping in the data
#
###############################################################################
function pcreate_key_by_size ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    data="$1"
    shift

    echo dd if=/dev/zero count=1 bs=$data \| keyctl padd "$@" >>$OUTPUTFILE
    dd if=/dev/zero count=1 bs=$data 2>/dev/null | keyctl padd "$@" >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# create a key and attach it to the new keyring
#
###############################################################################
function create_keyring ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl newring "$@" >>$OUTPUTFILE
    keyctl newring "$@" >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# extract a key ID from the log file
#
###############################################################################
function expect_keyid ()
{
    my_varname=$1

    my_keyid="`tail -1 $OUTPUTFILE`"
    if expr "$my_keyid" : '[1-9][0-9]*' >&/dev/null
    then
	eval $my_varname=$my_keyid

	if [ $# = 2 -a "x$my_keyid" != "x$2" ]
	then
	    failed
	fi
    else
	eval $my_varname=no
	result=FAIL
    fi
}

###############################################################################
#
# prettily list a keyring
#
###############################################################################
function pretty_list_keyring ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl list $1 >>$OUTPUTFILE
    keyctl list $1 >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# list a keyring
#
###############################################################################
function list_keyring ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl rlist $1 >>$OUTPUTFILE
    keyctl rlist $1 >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# extract a keyring listing from the log file and see if a key ID is contained
# therein
#
###############################################################################
function expect_keyring_rlist ()
{
    my_varname=$1

    my_rlist="`tail -1 $OUTPUTFILE`"
    eval $my_varname="\"$my_rlist\""

    if [ $# = 2 -o $# = 3 ]
    then
	if [ "$2" = "empty" ]
	then
	    if [ "x$my_rlist" != "x" ]
	    then
		failed
	    fi
	else
	    my_keyid=$2
	    my_found=0
	    my_expected=1
	    if [ $# = 3 -a "x$3" = "x--absent" ]; then my_expected=0; fi

	    for k in $my_rlist
	    do
		if [ $k = $my_keyid ]
		then
		    my_found=1
		    break;
		fi
	    done

	    if [ $my_found != $my_expected ]
	    then
		failed
	    fi
	fi
    fi
}

###############################################################################
#
# prettily describe a key
#
###############################################################################
function pretty_describe_key ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl describe $1 >>$OUTPUTFILE
    keyctl describe $1 >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# describe a key
#
###############################################################################
function describe_key ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl rdescribe $1 "@" >>$OUTPUTFILE
    keyctl rdescribe $1 "@" >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# extract a raw key description from the log file and check it
#
###############################################################################
function expect_key_rdesc ()
{
    my_varname=$1

    my_rdesc="`tail -1 $OUTPUTFILE`"
    eval $my_varname="\"$my_rdesc\""

    if ! expr "$my_rdesc" : "$2" >&/dev/null
    then
	failed
    fi
}

###############################################################################
#
# read a key's payload as a hex dump
#
###############################################################################
function read_key ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl read $1 >>$OUTPUTFILE
    keyctl read $1 >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# read a key's payload as a printable string
#
###############################################################################
function print_key ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl print $1 >>$OUTPUTFILE
    keyctl print $1 >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# pipe a key's raw payload to stdout
#
###############################################################################
function pipe_key ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl pipe $1 >>$OUTPUTFILE
    keyctl pipe $1 >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# pipe a key's raw payload through md5sum
#
###############################################################################
function md5sum_key ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl pipe $1 \| md5sum \| cut -c1-32 >>$OUTPUTFILE
    keyctl pipe $1 | md5sum | cut -c1-32 >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# extract a printed payload from the log file
#
###############################################################################
function expect_payload ()
{
    my_varname=$1

    my_payload="`tail -1 $OUTPUTFILE`"
    eval $my_varname="\"$my_payload\""

    if [ $# == 2 -a "x$my_payload" != "x$2" ]
    then
	failed
    fi
}

###############################################################################
#
# extract multiline output from the log file
#
###############################################################################
function expect_multiline ()
{
    my_varname=$1
    my_linecount="`echo \"$2\" | wc -l`"

    my_payload=$(tail -$my_linecount $OUTPUTFILE)
    eval $my_varname="\"$my_payload\""

    if [ $# != 2 -o "x$my_payload" != "x$2" ]
    then
	failed
    fi
}

###############################################################################
#
# revoke a key
#
###############################################################################
function revoke_key ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl revoke $1 >>$OUTPUTFILE
    keyctl revoke $1 >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# unlink a key from a keyring
#
###############################################################################
function unlink_key ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    my_wait=0
    if [ "x$1" = "x--wait" ]
    then
	my_wait=1
	shift
    fi

    echo keyctl unlink $1 $2 >>$OUTPUTFILE
    keyctl unlink $1 $2 >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi

    # keys are destroyed lazily
    if [ $my_wait = 1 ]
    then
	pause_till_key_unlinked $1 $2
    fi
}

###############################################################################
#
# extract a message about the number of keys unlinked
#
###############################################################################
function expect_unlink_count ()
{
    my_varname=$1

    my_nunlinks="`tail -1 $OUTPUTFILE`"

    if ! expr "$my_nunlinks" : '^[0-9][0-9]* links removed$'
    then
	failed
    fi

    my_nunlinks=`echo $my_nunlinks | awk '{printf $1}'`
    eval $my_varname="\"$my_nunlinks\""

    if [ $# == 2 -a $my_nunlinks != $2 ]
    then
	failed
    fi
}

###############################################################################
#
# update a key from a keyring
#
###############################################################################
function update_key ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl update $1 $2 >>$OUTPUTFILE
    keyctl update $1 $2 >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# update a key from a keyring, piping the data in over stdin
#
###############################################################################
function pupdate_key ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo echo -n $2 \| keyctl pupdate $1 >>$OUTPUTFILE
    echo -n $2 | keyctl pupdate $1 >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# clear a keyring
#
###############################################################################
function clear_keyring ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl clear $1 >>$OUTPUTFILE
    keyctl clear $1 >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# restrict a keyring
#
###############################################################################
function restrict_keyring ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl restrict_keyring $1 $2 $3 >>$OUTPUTFILE
    keyctl restrict_keyring $1 $2 $3 >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# link a key to a keyring
#
###############################################################################
function link_key ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl link $1 $2 >>$OUTPUTFILE
    keyctl link $1 $2 >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# search for a key in a keyring
#
###############################################################################
function search_for_key ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl search "$@" >>$OUTPUTFILE
    keyctl search "$@" >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# set the permissions mask on a key
#
###############################################################################
function set_key_perm ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl setperm "$@" >>$OUTPUTFILE
    keyctl setperm "$@" >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# set the ownership of a key
#
###############################################################################
function chown_key ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl chown "$@" >>$OUTPUTFILE
    keyctl chown "$@" >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# set the group ownership of a key
#
###############################################################################
function chgrp_key ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl chgrp "$@" >>$OUTPUTFILE
    keyctl chgrp "$@" >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# run as a new session
#
###############################################################################
function new_session ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl session "$@" >>$OUTPUTFILE
    keyctl session "$@" >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# instantiate a key
#
###############################################################################
function instantiate_key ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl instantiate "$@" >>$OUTPUTFILE
    keyctl instantiate "$@" >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# instantiate a key, piping the data in over stdin
#
###############################################################################
function pinstantiate_key ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    data="$1"
    shift

    echo echo -n $data \| keyctl pinstantiate "$@" >>$OUTPUTFILE
    echo -n $data | keyctl pinstantiate "$@" >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# negate a key
#
###############################################################################
function negate_key ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl negate "$@" >>$OUTPUTFILE
    keyctl negate "$@" >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# set a key's expiry time
#
###############################################################################
function timeout_key ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl timeout $1 $2 >>$OUTPUTFILE
    keyctl timeout $1 $2 >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# Invalidate a key
#
###############################################################################
function invalidate_key ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl invalidate $1 >>$OUTPUTFILE
    keyctl invalidate $1 >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# Do a DH computation
#
###############################################################################
function dh_compute ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl dh_compute $@ >>$OUTPUTFILE
    keyctl dh_compute $@ >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# Do a DH computation post-processed by a KDF
#
###############################################################################
function dh_compute_kdf ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl dh_compute_kdf $@ >>$OUTPUTFILE
    keyctl dh_compute_kdf $@ >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# Do a DH computation post-processed by a KDF with other information
#
###############################################################################
function dh_compute_kdf_oi ()
{
    my_exitval=0
    if [ "x$1" = "x--fail" ]
    then
	my_exitval=1
	shift
    fi

    echo keyctl dh_compute_kdf_oi $@ >>$OUTPUTFILE
    keyctl dh_compute_kdf_oi $@ >>$OUTPUTFILE 2>&1
    if [ $? != $my_exitval ]
    then
	failed
    fi
}

###############################################################################
#
# Make sure we sleep at least N seconds
#
###############################################################################
function sleep_at_least ()
{
    my_now=`date +%s`
    my_done_at=$(($my_now+$1+1))
    sleep $1
    while [ `date +%s` -lt $my_done_at ]
    do
	# Sleep in 1/50th of a second bursts till the time catches up
	sleep .02
    done
}

###############################################################################
#
# set gc delay time, return original value
#
###############################################################################
function set_gc_delay()
{
    delay=$1
    if [ -f $key_gc_delay_file ]; then
        echo $delay > $key_gc_delay_file
        echo "Set $key_gc_delay_file to $delay, orig: $orig_gc_delay"
    fi
}
