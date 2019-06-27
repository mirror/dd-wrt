#! /bin/sh
# check script for GNU ddrescue - Data recovery tool
# Copyright (C) 2009-2014 Antonio Diaz Diaz.
#
# This script is free software: you have unlimited permission
# to copy, distribute and modify it.

LC_ALL=C
export LC_ALL
objdir=`pwd`
testdir=`cd "$1" ; pwd`
DDRESCUE="${objdir}"/ddrescue
DDRESCUELOG="${objdir}"/ddrescuelog
framework_failure() { echo "failure in testing framework" ; exit 1 ; }

if [ ! -f "${DDRESCUE}" ] || [ ! -x "${DDRESCUE}" ] ; then
	echo "${DDRESCUE}: cannot execute"
	exit 1
fi

if [ -d tmp ] ; then rm -rf tmp ; fi
mkdir tmp
cd "${objdir}"/tmp

in="${testdir}"/test.txt
in1="${testdir}"/test1.txt
in2="${testdir}"/test2.txt
blank="${testdir}"/logfile_blank
logfile1="${testdir}"/logfile1
logfile2="${testdir}"/logfile2
logfile2i="${testdir}"/logfile2i
logfile3="${testdir}"/logfile3
logfile4="${testdir}"/logfile4
logfile5="${testdir}"/logfile5
fail=0

printf "testing ddrescue-%s..." "$2"

"${DDRESCUE}" -q ${in}
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUE}" -q ${in} out logfile extra
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUE}" -q ${in} ${in} logfile
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUE}" -q ${in} out ${in}
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUE}" -q ${in} out out
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUE}" -q -F- ${in} out
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUE}" -q -F ${in} out logfile
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUE}" -q -F- --ask ${in} out logfile
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUE}" -q -G --ask ${in} out logfile
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUE}" -q -G ${in} out
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUE}" -q -F- -G ${in} out logfile
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUE}" -q -H ${logfile2i} ${in} out logfile
if [ $? = 2 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUE}" -q -K ${in} out
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUE}" -q -K, ${in} out
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUE}" -q -K0, ${in} out
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUE}" -q -K0,65535 ${in} out
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUE}" -q -i 0, ${in} out
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUE}" -q -i -1 ${in} out
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUE}" -q -m ${logfile1} -m ${logfile1} ${in} out logfile
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUE}" -q -m ${logfile2i} ${in} out logfile
if [ $? = 2 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUE}" -q -w ${in} out logfile
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUE}" -q --cpass=1, ${in} out
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUE}" -q --cpass=4 ${in} out
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi

rm -f logfile
"${DDRESCUE}" -q -t -p -i15000 ${in} out logfile || fail=1
"${DDRESCUE}" -q -D -f -n -s15000 ${in} out logfile || fail=1
cmp ${in} out || fail=1
printf .

rm -f out
rm -f logfile
"${DDRESCUE}" -q -R -i15000 ${in} out logfile || fail=1
"${DDRESCUE}" -q -R -s15000 --cpass=3 ${in} out logfile || fail=1
cmp ${in} out || fail=1
printf .

rm -f out
"${DDRESCUE}" -q -F+ -o15000 ${in} out2 logfile || fail=1
"${DDRESCUE}" -q -R -S -i15000 -o0 --unidirectional out2 out || fail=1
cmp ${in} out || fail=1
printf .

printf "garbage" >> out || framework_failure
"${DDRESCUE}" -q -R -t -i15000 -o0 out2 out || fail=1
cmp ${in} out || fail=1
printf .

rm -f out
"${DDRESCUE}" -q -O -H ${logfile1} ${in} out || fail=1
cmp ${in1} out || fail=1
printf .
"${DDRESCUE}" -q -O -L -K0 -H ${logfile2i} ${in} out || fail=1
cmp ${in} out || fail=1
printf .

rm -f out
"${DDRESCUE}" -q -X -m ${logfile1} ${in} out || fail=1
cmp ${in1} out || fail=1
printf .
"${DDRESCUE}" -q -X -L -m ${logfile2i} ${in} out || fail=1
cmp ${in} out || fail=1
printf .

rm -f out
"${DDRESCUE}" -q -R -m ${logfile2} ${in} out || fail=1
cmp ${in2} out || fail=1
printf .
"${DDRESCUE}" -q -R -K,64KiB -m ${logfile1} ${in} out || fail=1
cmp ${in} out || fail=1
printf .

rm -f out
cat ${logfile1} > logfile || framework_failure
"${DDRESCUE}" -q -I ${in} out logfile || fail=1
cat ${logfile2} > logfile || framework_failure
"${DDRESCUE}" -q -I ${in} out logfile || fail=1
cmp ${in} out || fail=1
printf .

rm -f out
cat ${logfile1} > logfile || framework_failure
"${DDRESCUE}" -q -R ${in} out logfile || fail=1
cat ${logfile2} > logfile || framework_failure
"${DDRESCUE}" -q -R ${in} out logfile || fail=1
cmp ${in} out || fail=1
printf .

rm -f out
fail2=0
for i in 0 8000 16000 24000 32000 ; do
	"${DDRESCUE}" -q -i${i} -s4000 -m ${logfile1} ${in} out || fail2=1
done
cmp -s ${in} out && fail2=1
for i in 4000 12000 20000 28000 36000 ; do
	"${DDRESCUE}" -q -i${i} -s4000 -m ${logfile1} ${in} out || fail2=1
done
cmp ${in1} out || fail2=1
for i in 0 8000 16000 24000 32000 ; do
	"${DDRESCUE}" -q -i${i} -s4000 -m ${logfile2} ${in2} out || fail2=1
done
cmp -s ${in} out && fail2=1
for i in 4000 12000 20000 28000 36000 ; do
	"${DDRESCUE}" -q -i${i} -s4000 -m ${logfile2} ${in2} out || fail2=1
done
cmp ${in} out || fail2=1
if [ ${fail2} = 0 ] ; then printf . ; else printf - ; fail=1 ; fi

rm -f logfile
cat ${in1} > out || framework_failure
"${DDRESCUE}" -q -G ${in} out logfile || fail=1
"${DDRESCUE}" -q ${in2} out logfile || fail=1
cmp ${in} out || fail=1
printf .

rm -f logfile
cat ${in} > copy || framework_failure
printf "garbage" >> copy || framework_failure
cat ${in2} > out || framework_failure
"${DDRESCUE}" -q -t -x 36388 ${in1} copy || fail=1
"${DDRESCUE}" -q -G ${in} out logfile || fail=1
"${DDRESCUE}" -q -R -T1.5d copy out logfile || fail=1
cmp ${in} out || fail=1
printf .

printf "\ntesting ddrescuelog-%s..." "$2"

"${DDRESCUELOG}" -q logfile
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUELOG}" -q -d
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUELOG}" -q -t -d logfile
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUELOG}" -q -m ${logfile2i} -t logfile
if [ $? = 2 ] ; then printf . ; else printf - ; fail=1 ; fi

"${DDRESCUELOG}" -a '?,+' -i3072 ${logfile1} > logfile
"${DDRESCUELOG}" -D logfile
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUELOG}" -a '?,+' -i2048 -s1024 logfile > logfile2
"${DDRESCUELOG}" -d logfile2
if [ $? = 0 ] ; then printf . ; else printf - ; fail=1 ; fi

"${DDRESCUELOG}" -b2048 -l+ ${logfile1} > out || fail=1
"${DDRESCUELOG}" -b2048 -f -c logfile < out || fail=1
"${DDRESCUELOG}" -b2048 -l+ logfile > copy || fail=1
cmp out copy || fail=1
printf .
"${DDRESCUELOG}" -q -p ${logfile1} logfile
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUELOG}" -P ${logfile1} logfile || fail=1
printf .
"${DDRESCUELOG}" -b2048 -s36388 -f -c?+ logfile < out || fail=1
"${DDRESCUELOG}" -p ${logfile2} logfile || fail=1
printf .
"${DDRESCUELOG}" -b2048 -f -c?+ logfile < out || fail=1
"${DDRESCUELOG}" -s36388 -p ${logfile2} logfile || fail=1
printf .
"${DDRESCUELOG}" -q -s36389 -p ${logfile2} logfile
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi

printf "10\n12\n14\n16\n" | "${DDRESCUELOG}" -b2048 -f -c+? logfile || fail=1
"${DDRESCUELOG}" -q -p logfile ${logfile1}
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUELOG}" -q -i0x5000 -p logfile ${logfile1}
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUELOG}" -i0x5000 -s0x3800 -p logfile ${logfile1} || fail=1
printf .

"${DDRESCUELOG}" -C ${logfile2i} > logfile || fail=1
"${DDRESCUELOG}" -p ${logfile2} logfile || fail=1
printf .

cat ${logfile1} > logfile || framework_failure
"${DDRESCUELOG}" -i1024 -s2048 -d logfile
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUELOG}" -i1024 -s1024 -d logfile || fail=1
printf .
"${DDRESCUELOG}" -q -i1024 -s1024 -d logfile
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi

cat ${logfile2} > logfile || framework_failure
"${DDRESCUELOG}" -m ${logfile1} -D logfile
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUELOG}" -L -m ${logfile2i} -D logfile || fail=1
printf .
"${DDRESCUELOG}" -i1024 -s2048 -d logfile
if [ $? = 1 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUELOG}" -i2048 -s2048 -d logfile || fail=1
printf .

"${DDRESCUELOG}" -b2048 -l+ ${logfile1} > out || fail=1
printf "0\n2\n4\n6\n8\n10\n12\n14\n16\n" > copy || framework_failure
cmp out copy || fail=1
printf .
"${DDRESCUELOG}" -b2048 -l?- ${logfile1} > out || fail=1
printf "1\n3\n5\n7\n9\n11\n13\n15\n17\n" > copy || framework_failure
cmp out copy || fail=1
printf .
"${DDRESCUELOG}" -b2048 -l+ -i0x1800 -o0 -s0x4000 ${logfile1} > out || fail=1
printf "1\n3\n5\n7\n" > copy || framework_failure
cmp out copy || fail=1
printf .

"${DDRESCUELOG}" -n ${logfile2} > logfile || framework_failure
"${DDRESCUELOG}" -b2048 -l+ logfile > out || fail=1
printf "0\n2\n4\n6\n8\n10\n12\n14\n16\n" > copy || framework_failure
cmp out copy || fail=1
printf .
"${DDRESCUELOG}" -b2048 -l?- logfile > out || fail=1
printf "1\n3\n5\n7\n9\n11\n13\n15\n17\n" > copy || framework_failure
cmp out copy || fail=1
printf .
"${DDRESCUELOG}" -b2048 -l+ -i2048 -o0 -s0x4000 logfile > out || fail=1
printf "1\n3\n5\n7\n" > copy || framework_failure
cmp out copy || fail=1
printf .

"${DDRESCUELOG}" -q -P ${logfile2i} ${logfile2}
if [ $? = 2 ] ; then printf . ; else printf - ; fail=1 ; fi
"${DDRESCUELOG}" -L -P ${logfile2i} ${logfile2} || fail=1
printf .

fail2=0			# test XOR
for i in ${logfile1} ${logfile2} ${logfile3} ${logfile4} ${logfile5} ; do
	for j in ${logfile1} ${logfile2} ${logfile3} ${logfile4} ${logfile5} ; do
		"${DDRESCUELOG}" -x ${j} ${i} > out || fail2=1
		"${DDRESCUELOG}" -x ${i} ${j} > copy || fail2=1
		"${DDRESCUELOG}" -P out copy || fail2=1
		"${DDRESCUELOG}" -x ${j} out > copy || fail2=1
		"${DDRESCUELOG}" -P ${i} copy || fail2=1
	done
done
if [ ${fail2} = 0 ] ; then printf . ; else printf - ; fail=1 ; fi

fail2=0
"${DDRESCUELOG}" -x ${logfile1} ${logfile2} > out || fail2=1
"${DDRESCUELOG}" -x ${logfile2} ${logfile1} > copy || fail2=1
"${DDRESCUELOG}" -p out copy || fail2=1
"${DDRESCUELOG}" -d out || fail2=1
"${DDRESCUELOG}" -d copy || fail2=1
"${DDRESCUELOG}" -x ${logfile1} ${blank} > out || fail2=1
"${DDRESCUELOG}" -x ${blank} ${logfile1} > copy || fail2=1
"${DDRESCUELOG}" -p out copy || fail2=1
"${DDRESCUELOG}" -p out ${logfile1} || fail2=1
"${DDRESCUELOG}" -p ${logfile1} copy || fail2=1
"${DDRESCUELOG}" -x ${logfile2} ${logfile2} > logfile || fail2=1
"${DDRESCUELOG}" -P ${blank} logfile || fail2=1
"${DDRESCUELOG}" -x ${logfile1} ${logfile1} > logfile || fail2=1
"${DDRESCUELOG}" -P ${blank} logfile || fail2=1
"${DDRESCUELOG}" -b2048 -l+ ${logfile1} > out || fail2=1
"${DDRESCUELOG}" -b2048 -l- logfile > copy || fail2=1
cmp out copy || fail2=1
"${DDRESCUELOG}" -b2048 -i0x2000 -s0x2800 -l+ ${logfile1} > out || fail2=1
"${DDRESCUELOG}" -i0x1800 -s0x3800 -x ${logfile1} ${logfile1} > logfile || fail2=1
"${DDRESCUELOG}" -b2048 -l- logfile > copy || fail2=1
cmp out copy || fail2=1
if [ ${fail2} = 0 ] ; then printf . ; else printf - ; fail=1 ; fi

fail2=0
"${DDRESCUELOG}" -x ${logfile3} ${logfile4} > out || fail2=1
"${DDRESCUELOG}" -x ${logfile4} ${logfile3} > copy || fail2=1
"${DDRESCUELOG}" -p out copy || fail2=1
"${DDRESCUELOG}" -x ${logfile3} ${logfile5} > out || fail2=1
"${DDRESCUELOG}" -x ${logfile5} ${logfile3} > copy || fail2=1
"${DDRESCUELOG}" -p out copy || fail2=1
"${DDRESCUELOG}" -x ${logfile4} ${logfile5} > out || fail2=1
"${DDRESCUELOG}" -x ${logfile5} ${logfile4} > copy || fail2=1
"${DDRESCUELOG}" -p out copy || fail2=1
if [ ${fail2} = 0 ] ; then printf . ; else printf - ; fail=1 ; fi

fail2=0
"${DDRESCUELOG}" -x ${logfile3} ${logfile4} > out || fail2=1
"${DDRESCUELOG}" -D out && fail2=1
"${DDRESCUELOG}" -x out ${logfile5} > logfile || fail2=1
"${DDRESCUELOG}" -d logfile || fail2=1

"${DDRESCUELOG}" -x ${logfile3} ${logfile5} > out || fail2=1
"${DDRESCUELOG}" -D out && fail2=1
"${DDRESCUELOG}" -x out ${logfile4} > logfile || fail2=1
"${DDRESCUELOG}" -d logfile || fail2=1

"${DDRESCUELOG}" -x ${logfile4} ${logfile3} > out || fail2=1
"${DDRESCUELOG}" -D out && fail2=1
"${DDRESCUELOG}" -x out ${logfile5} > logfile || fail2=1
"${DDRESCUELOG}" -d logfile || fail2=1

"${DDRESCUELOG}" -x ${logfile4} ${logfile5} > out || fail2=1
"${DDRESCUELOG}" -D out && fail2=1
"${DDRESCUELOG}" -x out ${logfile3} > logfile || fail2=1
"${DDRESCUELOG}" -d logfile || fail2=1

"${DDRESCUELOG}" -x ${logfile5} ${logfile3} > out || fail2=1
"${DDRESCUELOG}" -D out && fail2=1
"${DDRESCUELOG}" -x out ${logfile4} > logfile || fail2=1
"${DDRESCUELOG}" -d logfile || fail2=1

"${DDRESCUELOG}" -x ${logfile5} ${logfile4} > out || fail2=1
"${DDRESCUELOG}" -D out && fail2=1
"${DDRESCUELOG}" -x out ${logfile3} > logfile || fail2=1
"${DDRESCUELOG}" -d logfile || fail2=1
if [ ${fail2} = 0 ] ; then printf . ; else printf - ; fail=1 ; fi

fail2=0			# test AND
for i in ${logfile1} ${logfile2} ${logfile3} ${logfile4} ${logfile5} ; do
	for j in ${logfile1} ${logfile2} ${logfile3} ${logfile4} ${logfile5} ; do
		"${DDRESCUELOG}" -y ${j} ${i} > out || fail2=1
		"${DDRESCUELOG}" -y ${i} ${j} > copy || fail2=1
		"${DDRESCUELOG}" -P out copy || fail2=1
	done
done
if [ ${fail2} = 0 ] ; then printf . ; else printf - ; fail=1 ; fi

fail2=0
"${DDRESCUELOG}" -b2048 -l+ ${logfile1} > out || fail2=1
"${DDRESCUELOG}" -y ${logfile1} ${logfile2} > logfile || fail2=1
"${DDRESCUELOG}" -P ${blank} logfile || fail2=1
"${DDRESCUELOG}" -b2048 -l? logfile > copy || fail2=1
cmp out copy || fail2=1
"${DDRESCUELOG}" -y ${logfile2} ${logfile1} > logfile || fail2=1
"${DDRESCUELOG}" -P ${blank} logfile || fail2=1
"${DDRESCUELOG}" -b2048 -l- logfile > copy || fail2=1
cmp out copy || fail2=1
"${DDRESCUELOG}" -b2048 -i0x2000 -s0x2800 -l+ ${logfile1} > out || fail2=1
"${DDRESCUELOG}" -i0x1800 -s0x3800 -y ${logfile2} ${logfile1} > logfile || fail2=1
"${DDRESCUELOG}" -b2048 -l- logfile > copy || fail2=1
cmp out copy || fail2=1
if [ ${fail2} = 0 ] ; then printf . ; else printf - ; fail=1 ; fi

fail2=0
"${DDRESCUELOG}" -y ${logfile3} ${logfile4} > out || fail2=1
"${DDRESCUELOG}" -P ${blank} out || fail2=1
"${DDRESCUELOG}" -y ${logfile3} ${logfile5} > out || fail2=1
"${DDRESCUELOG}" -P ${blank} out || fail2=1
"${DDRESCUELOG}" -y ${logfile4} ${logfile5} > out || fail2=1
"${DDRESCUELOG}" -P ${blank} out || fail2=1
if [ ${fail2} = 0 ] ; then printf . ; else printf - ; fail=1 ; fi

fail2=0
"${DDRESCUELOG}" -i0x2000 -s0x2800 -z ${logfile2} ${logfile1} > logfile || fail2=1
"${DDRESCUELOG}" -D logfile
if [ $? != 1 ] ; then fail2=1 ; fi
"${DDRESCUELOG}" -i0x1C00 -s0x2C00 -D logfile
if [ $? != 1 ] ; then fail2=1 ; fi
"${DDRESCUELOG}" -i0x2000 -s0x2C00 -D logfile
if [ $? != 1 ] ; then fail2=1 ; fi
"${DDRESCUELOG}" -i0x2000 -s0x2800 -d logfile
if [ $? != 0 ] ; then fail2=1 ; fi
if [ ${fail2} = 0 ] ; then printf . ; else printf - ; fail=1 ; fi

fail2=0			# test OR
for i in ${logfile1} ${logfile2} ${logfile3} ${logfile4} ${logfile5} ; do
	for j in ${logfile1} ${logfile2} ${logfile3} ${logfile4} ${logfile5} ; do
		"${DDRESCUELOG}" -z ${j} ${i} > out || fail2=1
		"${DDRESCUELOG}" -z ${i} ${j} > copy || fail2=1
		"${DDRESCUELOG}" -P out copy || fail2=1
	done
done
if [ ${fail2} = 0 ] ; then printf . ; else printf - ; fail=1 ; fi

fail2=0
"${DDRESCUELOG}" -z ${logfile1} ${logfile2} > out || fail2=1
"${DDRESCUELOG}" -z ${logfile2} ${logfile1} > copy || fail2=1
"${DDRESCUELOG}" -p out copy || fail2=1
"${DDRESCUELOG}" -d out || fail2=1
"${DDRESCUELOG}" -d copy || fail2=1
"${DDRESCUELOG}" -z ${logfile1} ${blank} > out || fail2=1
"${DDRESCUELOG}" -z ${blank} ${logfile1} > copy || fail2=1
"${DDRESCUELOG}" -p out copy || fail2=1
"${DDRESCUELOG}" -p out ${logfile1} || fail2=1
"${DDRESCUELOG}" -p ${logfile1} copy || fail2=1
"${DDRESCUELOG}" -z ${logfile3} ${logfile4} > out || fail2=1
"${DDRESCUELOG}" -z ${logfile4} ${logfile3} > copy || fail2=1
"${DDRESCUELOG}" -p out copy || fail2=1
"${DDRESCUELOG}" -z ${logfile3} ${logfile5} > out || fail2=1
"${DDRESCUELOG}" -z ${logfile5} ${logfile3} > copy || fail2=1
"${DDRESCUELOG}" -p out copy || fail2=1
"${DDRESCUELOG}" -z ${logfile4} ${logfile5} > out || fail2=1
"${DDRESCUELOG}" -z ${logfile5} ${logfile4} > copy || fail2=1
"${DDRESCUELOG}" -p out copy || fail2=1
if [ ${fail2} = 0 ] ; then printf . ; else printf - ; fail=1 ; fi

fail2=0
"${DDRESCUELOG}" -z ${logfile3} ${logfile4} > out || fail2=1
"${DDRESCUELOG}" -D out && fail2=1
"${DDRESCUELOG}" -z out ${logfile5} > logfile || fail2=1
"${DDRESCUELOG}" -d logfile || fail2=1

"${DDRESCUELOG}" -z ${logfile3} ${logfile5} > out || fail2=1
"${DDRESCUELOG}" -D out && fail2=1
"${DDRESCUELOG}" -z out ${logfile4} > logfile || fail2=1
"${DDRESCUELOG}" -d logfile || fail2=1

"${DDRESCUELOG}" -z ${logfile4} ${logfile3} > out || fail2=1
"${DDRESCUELOG}" -D out && fail2=1
"${DDRESCUELOG}" -z out ${logfile5} > logfile || fail2=1
"${DDRESCUELOG}" -d logfile || fail2=1

"${DDRESCUELOG}" -z ${logfile4} ${logfile5} > out || fail2=1
"${DDRESCUELOG}" -D out && fail2=1
"${DDRESCUELOG}" -z out ${logfile3} > logfile || fail2=1
"${DDRESCUELOG}" -d logfile || fail2=1

"${DDRESCUELOG}" -z ${logfile5} ${logfile3} > out || fail2=1
"${DDRESCUELOG}" -D out && fail2=1
"${DDRESCUELOG}" -z out ${logfile4} > logfile || fail2=1
"${DDRESCUELOG}" -d logfile || fail2=1

"${DDRESCUELOG}" -z ${logfile5} ${logfile4} > out || fail2=1
"${DDRESCUELOG}" -D out && fail2=1
"${DDRESCUELOG}" -z out ${logfile3} > logfile || fail2=1
"${DDRESCUELOG}" -d logfile || fail2=1
if [ ${fail2} = 0 ] ; then printf . ; else printf - ; fail=1 ; fi

fail2=0			# test ( a && b ) == !( !a || !b )
for i in ${logfile1} ${logfile2} ${logfile3} ${logfile4} ${logfile5} ; do
	for j in ${logfile1} ${logfile2} ${logfile3} ${logfile4} ${logfile5} ; do
		"${DDRESCUELOG}" -n ${i} > na || fail2=1
		"${DDRESCUELOG}" -n ${j} > nb || fail2=1
		"${DDRESCUELOG}" -z nb na > out || fail2=1
		"${DDRESCUELOG}" -n out > copy || fail2=1
		"${DDRESCUELOG}" -y ${j} ${i} > out || fail2=1
		"${DDRESCUELOG}" -P out copy || fail2=1
	done
done
if [ ${fail2} = 0 ] ; then printf . ; else printf - ; fail=1 ; fi

echo
if [ ${fail} = 0 ] ; then
	echo "tests completed successfully."
	cd "${objdir}" && rm -r tmp
else
	echo "tests failed."
fi
exit ${fail}
