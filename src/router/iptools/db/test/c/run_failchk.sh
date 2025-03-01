#!/bin/bash

#
# test_failchk --
#	Test failchk in a simple threaded application of some numbers of readers
#	and writers competing to read and update a set of words.
#	A typical test scenario runs this programs several times concurrently,
#	with different options:
#		first with the -I option to clear out any home directory
#		one or more instances with -f to activate the failchk thread
#		one or more instance with neither -I nor -f, as minimally
#			involved workers.
#
# If no arguments are given, it selects a default mix of processes:
#	run_failchk.sh 100 2 '-r1 -w2' -w2
#
# This does 100 iterations of this failchk group of applications:
#	2 copies of test_failchk with 1 reader and 2 writer threads
#	a solo test_failchk the default # of readers (4) and 2 writers
#	a copy of the last test_failchk adding a failchk thread
#	wait a few seconds
#	randomly kill one of the non-failchk process
#
# This shell script initializes the env with the default number of readers and
# writers, then starts one "worker" process with each listed argument. The last
# worker started also uses -f, to ensure that at least one process will be
# running failchk. It is okay for -f to also be passed to one or more of the
# other processes. One of the processes  is selected at random to be killed.

if test $# -eq 0 ; then
	set -- 100 2 '-r1 -w2' -w2
	echo Running $0 $@
fi
repeat=$1
dup_procs=$2
shift; shift
nprocs=0
victim=-1

function timestamp {
perl \
	-e 'use strict;' \
	-e 'use Time::HiRes qw(time);' \
	-e 'use POSIX qw(strftime);' \
	-e 'local $| = 1;	# Line buffering on' \
	-e 'while (<>) {'		\
	-e '	# HiRes time is a float, often down to nanoseconds.' \
	-e '	my $t = time;' \
	-e '	# Display the time of day,  appending microseconds.' \
	-e '	my $date = (strftime "%H:%M:%S", localtime $t ) .' \
	-e '		   sprintf ".%06d", ($t-int($t))*1000000;' \
	-e '	printf("%s: %s", $date, $_);' \
	-e '}'
}

function dofork {
	# Keep a slight bit of history -- just the previous iteration
	test -f $home/FAILCHK.$nprocs && \
		mv -f $home/FAILCHK.$nprocs $home/FAILCHK.prev.$nprocs
	test_failchk $* $arg > $home/FAILCHK.$nprocs 2>&1 &
	pids[$nprocs]=$!
	printf "Process %d(%s): test_failchk %s\n" $nprocs ${pids[$nprocs]} "$*"
	nprocs=$((++nprocs))
}

make test_failchk

home=TESTDIR
rm $home/*
test -d $home || mkdir $home

initargs=$1
shift

function main {
    for (( i = 0; $i < $repeat; i=$((++i)) )) ; do
	    test -f stat && mv stat stat.prev
	    test -d $home && cp -pr $home $home.prev
	    nprocs=0
	    dofork $initargs
	    sleep 2
	    for arg in "$@" ; do
		     dofork $arg
	    done

	    # Duplicate the last configuration, then add a for-sure failchk'er.
	    for (( j = 0; $j < $dup_procs; j=$((++j)) )) ; do
		    dofork $arg
	    done
	    # If the failchk process does real work, it could also trip over.
	    dofork -f $arg -w1 -r0

	    # $RANDOM is not very random in the lowest bits, div by 23 to scatter a little.
	    victim=$((($RANDOM / 23) % ($nprocs - 1)))
	    delay=$((($RANDOM / 37) % 15 + 4))
	    echo "$0 #$i: Processes: ${pids[@]}; delaying $delay seconds before killing #$victim"
	    sleep $delay
	    echo "$0 #$i: Killing ${pids[$victim]}"
	    # Stop if a process has exited prematurely
	    kill -9 ${pids[$victim]} || exit 100

	    for (( j = 0; $j < $nprocs; j=$((++j)) )) ; do
		    wait ${pids[$j]}
		    stat=$?
		    echo "Waited for process #$j ${pids[$j]} returned $stat"
		    # SIGTERM exits with 2, SIGKILL 137; anything else is bad.
		    if test $stat -gt 2 -a $stat -ne 137; then 
			    signal=`expr $stat - 128`
			    test $signal -lt 0 && signal=0
			    printf \
		    "Unexpected failure for process %d: status %d signal %d\n" \
			    	${pids[$j]} $stat $signal
			    exit $stat
		    fi

	    done
	    echo ""
	    sleep 2
	    # Saving stats would be nice here; but db_stat can trip over a bad lock
	    # db_stat -NEh $home > stat || exit 50

	    # If a system might possibly be running multiple instances of this
	    # script then the follow lines needs to stay a comment. However,
	    # when running by itself you can notified of non-killed processes
	    # by enabling the pgrep.
	    # pgrep test_failchk && echo "PROCESSES REMAIN ACTIVE?!" && exit 101

    done
    echo $i iterations done
}

main $* 2>&1 | timestamp
