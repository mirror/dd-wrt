#! /bin/sh
# check script for GNU ddrescue - Data recovery tool
# Copyright (C) 2009-2023 Antonio Diaz Diaz.
#
# This script is free software: you have unlimited permission
# to copy, distribute, and modify it.

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
if [ ! -f "${DDRESCUELOG}" ] || [ ! -x "${DDRESCUELOG}" ] ; then
	echo "${DDRESCUELOG}: cannot execute"
	exit 1
fi

[ -e "${DDRESCUE}" ] 2> /dev/null ||
	{
	echo "$0: a POSIX shell is required to run the tests"
	echo "Try bash -c \"$0 $1 $2\""
	exit 1
	}

if [ -d tmp ] ; then rm -rf tmp ; fi
mkdir tmp
cd "${objdir}"/tmp || framework_failure

fox="${testdir}"/fox
in="${testdir}"/test.txt
in1="${testdir}"/test1.txt
in2="${testdir}"/test2.txt
in3="${testdir}"/test3.txt
in4="${testdir}"/test4.txt
in5="${testdir}"/test5.txt
blank="${testdir}"/mapfile_blank
map1="${testdir}"/mapfile1
map2="${testdir}"/mapfile2
map2i="${testdir}"/mapfile2i
map2ui="${testdir}"/mapfile2ui
map3="${testdir}"/mapfile3
map4="${testdir}"/mapfile4
map5="${testdir}"/mapfile5
map6="${testdir}"/mapfile6
map6j="${testdir}"/mapfile6j
fail=0
test_failed() { fail=1 ; printf " $1" ; [ -z "$2" ] || printf "($2)" ; }

# Description of test files for ddrescue:
# fox          : "The quick brown fox jumps over the lazy dog.\n"
# test.txt     : 4 copies of the GPLv2, alternating LF and CRLF line ends
# test[1-5].txt: partial copies of test.txt as recorded in mapfile[1-5]
# mapfile1     : Alternating finished/non-tried blocks of 0x800 bytes each
# mapfile2     : mapfile1 inverted (non-tried/finished)
# mapfile2i    : mapfile2 without the non-tried blocks
# mapfile2ui   : mapfile2 unordered and without the non-tried blocks
# mapfile3     : Alternating finished/non-tried blocks of 0x800/0x1000 bytes
# mapfile[45]  : Triphasic complement to mapfile3
# mapfile6     : mapfile1 with non-finished subsectors smaller than 512 bytes
# mapfile6j    : mapfile6 with the subsectors joined to the right blocks
# mapfile_blank: non-tried mapfile cut to the lenght of test.txt (72776)

printf "testing ddrescue-%s..." "$2"

"${DDRESCUE}" -q "${in}"
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q "${in}" out mapfile extra
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q "${in}" "${in}" mapfile
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q "${in}" out "${in}"
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q "${in}".bak out "${in}"
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q "${in}" out out
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q "${in}" out.bak out
[ $? = 1 ] || test_failed $LINENO
for i in 1ki 1K 1BB 1Bi 1Bk 1Bs 1iB 1ii 1ik 1is 1kk 1sB 1si 1sk 1ss ; do
	"${DDRESCUE}" -q -a $i "${in}" out
	[ $? = 1 ] || test_failed $LINENO $i
done
"${DDRESCUE}" -q -F- "${in}" out
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q -Fi "${in}" out mapfile
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q -F- --ask "${in}" out mapfile
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q -F- --same-file "${in}" out mapfile
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q -G --ask "${in}" out mapfile
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q -G "${in}" out
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q -F- -G "${in}" out mapfile
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q -G --command-mode "${in}" out mapfile
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q -H "${map2i}" "${in}" out mapfile
[ $? = 2 ] || test_failed $LINENO
"${DDRESCUE}" -q -K "${in}" out
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q -K, "${in}" out
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q -K0, "${in}" out
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q -K0,65535 "${in}" out
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q -K0,65536, "${in}" out
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q -i 0, "${in}" out
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q -i -1 "${in}" out
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q -m "${map1}" -m "${map2}" "${in}" out mapfile
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q -m "${map2i}" "${in}" out mapfile
[ $? = 2 ] || test_failed $LINENO
"${DDRESCUE}" -q -w "${in}" out mapfile
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q --cpass=, "${in}" out
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q --cpass=1, "${in}" out
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q --cpass=-1 "${in}" out
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q --cpass=1- "${in}" out
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q --cpass=6 "${in}" out
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q --cpass=1,6 "${in}" out
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q --cpass=1-6 "${in}" out
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q --cpass=3-2 "${in}" out
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q --log-events=- "${in}" out
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q --mapfile-interval=-2 "${in}" out
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q --mapfile-interval=30, "${in}" out
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q --mapfile-interval=,4s "${in}" out
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q --mapfile-interval=3,4s "${in}" out
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q --mapfile-interval=30,10 "${in}" out
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q --mapfile-interval=30,60, "${in}" out
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q --mapfile-interval=30,60s, "${in}" out
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUE}" -q --same-file -t "${in}" out
[ $? = 1 ] || test_failed $LINENO

rm -f out mapfile || framework_failure
"${DDRESCUE}" -q -t -p -J -b1024 -i15kB "${in}" out mapfile || test_failed $LINENO
"${DDRESCUE}" -q -A -y -e0 -f -n -s15k "${in}" out mapfile || test_failed $LINENO
cmp "${in}" out || test_failed $LINENO

rm -f out mapfile || framework_failure
"${DDRESCUE}" -q -R -i15000 -a 1ks -E 1Kis "${in}" out mapfile ||
	test_failed $LINENO
"${DDRESCUE}" -q -R -s15000 --cpass=5 "${in}" out mapfile || test_failed $LINENO
cmp "${in}" out || test_failed $LINENO

rm -f out || framework_failure
"${DDRESCUE}" -q -F+ -o15000 -c143 "${in}" out2 mapfile || test_failed $LINENO
"${DDRESCUE}" -q -R -S -i15000 -o0 -u -Z1Mis out2 out || test_failed $LINENO
cmp "${in}" out || test_failed $LINENO

printf "garbage" >> out || framework_failure
"${DDRESCUE}" -q -N -R -t -i15000 -o0 --cpass=1-5 --pause-on-pass=0 out2 out ||
	test_failed $LINENO
cmp "${in}" out || test_failed $LINENO
rm -f out2 || framework_failure

rm -f out || framework_failure
"${DDRESCUE}" -q -O -r1 -H - --pause-on-error=s0 "${in}" out < "${map1}" ||
	test_failed $LINENO
cmp "${in1}" out || test_failed $LINENO
"${DDRESCUE}" -q -L -K0 -c1 -H "${map2ui}" --pause-on-error=0 "${in2}" out ||
	test_failed $LINENO
cmp "${in}" out || test_failed $LINENO

rm -f out mapfile || framework_failure
"${DDRESCUE}" -q -c1 -H "${map3}" --delay-slow=0 "${in3}" out mapfile ||
	test_failed $LINENO
"${DDRESCUE}" -q -c1 -M -H "${map4}" "${in4}" out mapfile || test_failed $LINENO
"${DDRESCUE}" -q -c1 -A -M --cpass=0 -H "${map5}" "${in5}" out mapfile ||
	test_failed $LINENO
cmp -s "${in}" out && test_failed $LINENO
"${DDRESCUE}" -q -c1 -n -N -H "${map5}" "${in5}" out mapfile || test_failed $LINENO
cmp "${in}" out || test_failed $LINENO

rm -f out || framework_failure
"${DDRESCUE}" -q -X0 -m - "${in}" out < "${map1}" || test_failed $LINENO
cmp "${in1}" out || test_failed $LINENO
"${DDRESCUE}" -q -X0 -L -K0,64Ki -m "${map2i}" "${in2}" out || test_failed $LINENO
cmp "${in}" out || test_failed $LINENO

rm -f out || framework_failure
"${DDRESCUE}" -q -R -B -K,64KiB -m "${map2}" "${in}" out || test_failed $LINENO
cmp "${in2}" out || test_failed $LINENO
"${DDRESCUE}" -q -R -K65536,65536 -m "${map1}" "${in1}" out || test_failed $LINENO
cmp "${in}" out || test_failed $LINENO

rm -f out || framework_failure
"${DDRESCUE}" -q --mapfile-interval=5m -m "${map5}" "${in5}" out ||
	test_failed $LINENO
"${DDRESCUE}" -q --mapfile-interval=5,5 -m "${map4}" "${in4}" out ||
	test_failed $LINENO
"${DDRESCUE}" -q --mapfile-interval=,5s -m "${map3}" "${in3}" out ||
	test_failed $LINENO
cmp "${in}" out || test_failed $LINENO

rm -f out || framework_failure
cat "${map1}" > mapfile || framework_failure
"${DDRESCUE}" -q -I --cpass=1,2,5 "${in2}" out mapfile || test_failed $LINENO
cat "${map2}" > mapfile || framework_failure
"${DDRESCUE}" -q -I "${in}" out mapfile || test_failed $LINENO
cmp "${in}" out || test_failed $LINENO

rm -f out || framework_failure
cat "${map1}" > mapfile || framework_failure
"${DDRESCUE}" -q -R --cpass=1,2-5 "${in2}" out mapfile || test_failed $LINENO
cat "${map2}" > mapfile || framework_failure
"${DDRESCUE}" -q -R -C "${in1}" out mapfile || test_failed $LINENO
cmp "${in}" out || test_failed $LINENO

# test joining non-finished subsectors
rm -f out || framework_failure
cat "${map2}" > mapfile || framework_failure
"${DDRESCUE}" -q --cpass=1-2,5 "${in}" out mapfile || test_failed $LINENO
cmp "${in1}" out || test_failed $LINENO
cat "${map6}" > mapfile || framework_failure
"${DDRESCUE}" -q -s0x800 "${in2}" out mapfile || test_failed $LINENO
"${DDRESCUELOG}" -p "${map6j}" mapfile || test_failed $LINENO
cmp "${in1}" out || test_failed $LINENO
"${DDRESCUE}" -q "${in2}" out mapfile || test_failed $LINENO
cmp -s "${in}" out && test_failed $LINENO
"${DDRESCUELOG}" -d mapfile && test_failed $LINENO
"${DDRESCUE}" -q -r1 "${in2}" out mapfile || test_failed $LINENO
cmp "${in}" out || test_failed $LINENO
"${DDRESCUELOG}" -d mapfile || test_failed $LINENO

cat "${in1}" > out || framework_failure
cat "${map6}" > mapfile || framework_failure
"${DDRESCUE}" -q -b0x80 "${in2}" out mapfile || test_failed $LINENO
cmp -s "${in}" out && test_failed $LINENO
"${DDRESCUELOG}" -d mapfile && test_failed $LINENO
"${DDRESCUE}" -q -b0x80 -r1 "${in2}" out mapfile || test_failed $LINENO
cmp "${in}" out || test_failed $LINENO
"${DDRESCUELOG}" -d mapfile || test_failed $LINENO

cat "${in1}" > out || framework_failure
cat "${map6}" > mapfile || framework_failure
"${DDRESCUE}" -q -b4096 -s0x800 "${in2}" out mapfile || test_failed $LINENO
"${DDRESCUELOG}" -P "${map1}" mapfile || test_failed $LINENO
"${DDRESCUELOG}" -q -p "${map1}" mapfile && test_failed $LINENO
cmp "${in1}" out || test_failed $LINENO
"${DDRESCUE}" -q -b4096 "${in2}" out mapfile || test_failed $LINENO
cmp "${in}" out || test_failed $LINENO
"${DDRESCUELOG}" -d mapfile || test_failed $LINENO

rm -f out || framework_failure
for i in 0 8000 16000 24000 32000 40000 48000 56000 64000 72000 ; do
	"${DDRESCUE}" -q -i$i -s4000 -m "${map1}" "${in}" out ||
	test_failed $LINENO $i
done
cmp -s "${in}" out && test_failed $LINENO
for i in 4000 12000 20000 28000 36000 44000 52000 60000 68000 ; do
	"${DDRESCUE}" -q -i$i -s4000 -m "${map1}" "${in}" out ||
	test_failed $LINENO $i
done
cmp "${in1}" out || test_failed $LINENO
for i in 0 8000 16000 24000 32000 40000 48000 56000 64000 72000 ; do
	"${DDRESCUE}" -q -i$i -s4000 -m "${map2}" "${in2}" out ||
	test_failed $LINENO $i
done
cmp -s "${in}" out && test_failed $LINENO
for i in 4000 12000 20000 28000 36000 44000 52000 60000 68000 ; do
	"${DDRESCUE}" -q -i$i -s4000 -m "${map2}" "${in2}" out ||
	test_failed $LINENO $i
done
cmp "${in}" out || test_failed $LINENO

rm -f mapfile || framework_failure
cat "${in1}" > out || framework_failure
"${DDRESCUE}" -q -G "${in}" out mapfile || test_failed $LINENO
"${DDRESCUE}" -q "${in2}" out mapfile || test_failed $LINENO
cmp "${in}" out || test_failed $LINENO

rm -f mapfile || framework_failure
cat "${in}" > copy || framework_failure
printf "garbage" >> copy || framework_failure
cat "${in2}" > out || framework_failure
"${DDRESCUE}" -q -t -x 72776 "${in1}" copy || test_failed $LINENO
"${DDRESCUE}" -q -G "${in}" out mapfile || test_failed $LINENO
"${DDRESCUE}" -q -R -T1.5d copy out mapfile || test_failed $LINENO
cmp "${in}" out || test_failed $LINENO

"${DDRESCUE}" -q --same-file out out || test_failed $LINENO
cmp "${in}" out || test_failed $LINENO

"${DDRESCUE}" -q -t "${in}" out || test_failed $LINENO
"${DDRESCUE}" -q --same-file -o 72776 out out || test_failed $LINENO
cat "${in}" "${in}" | cmp out - || test_failed $LINENO

rm -f out || framework_failure
"${DDRESCUE}" -q "${in}" out || test_failed $LINENO
"${DDRESCUE}" -q --same-file -o 145552 -S out out || test_failed $LINENO
"${DDRESCUE}" -q --same-file -i 145552 -o 72776 out out || test_failed $LINENO
cat "${in}" "${in}" "${in}" | cmp out - || test_failed $LINENO

rm -f out || framework_failure
printf "c 0 72776\nq\n" | "${DDRESCUE}" -m "${map1}" --command-mode "${in}" out \
	> /dev/null || test_failed $LINENO
cmp "${in1}" out || test_failed $LINENO
printf "c 0 72776\nu\n" | "${DDRESCUE}" -m "${map2}" --command-mode "${in2}" out \
	> /dev/null || test_failed $LINENO
cmp "${in}" out || test_failed $LINENO

rm -f out || framework_failure
cat "${map1}" > mapfile || framework_failure
printf "c 0 72776\ns0 1\n" | "${DDRESCUE}" --command-mode "${in}" out mapfile \
	> /dev/null || test_failed $LINENO
cmp "${in2}" out || test_failed $LINENO
cat "${map2}" > mapfile || framework_failure
printf "c 0 72776\n" | "${DDRESCUE}" -C --command-mode "${in1}" out mapfile \
	> /dev/null || test_failed $LINENO
cmp "${in}" out || test_failed $LINENO
"${DDRESCUELOG}" -d mapfile || test_failed $LINENO

rm -f out mapfile || framework_failure
printf "c 0x1000 0x800\nc 0x2800 0x800\nc 0x4000 0x800\nc 0x5800 0x800\n\
	c 0x7000 0x800\nc 0x8800 0x800\nc 0xA000 0x800\nc 0xB800 0x800\n\
	c 0xD000 0x800\nc 0xE800 0x800\nc 0x10000 0x800\nc 0x11800 0x448\n" | \
"${DDRESCUE}" --command-mode "${in}" out mapfile > /dev/null ||
	test_failed $LINENO
cmp "${in5}" out || test_failed $LINENO
printf "c 0 0x800\nc 0x1800 0x800\nc 0x3000 0x800\nc 0x4800 0x800\n\
	c 0x6000 0x800\nc 0x7800 0x800\nc 0x9000 0x800\nc 0xA800 0x800\n\
	c 0xC000 0x800\nc 0xD800 0x800\nc 0xF000 0x800\nc 0x10800 0x800\n" | \
"${DDRESCUE}" -C --command-mode "${in3}" out mapfile > /dev/null ||
	test_failed $LINENO
printf "c 0 72776\nf\n" | "${DDRESCUE}" --command-mode "${in4}" out mapfile \
	> /dev/null || test_failed $LINENO
cmp "${in}" out || test_failed $LINENO
"${DDRESCUELOG}" -d mapfile || test_failed $LINENO

"${DDRESCUE}" -q "${in}" out --compare-before-write
cmp "${in}" out || test_failed $LINENO
cat "${in2}" > out || framework_failure
"${DDRESCUE}" -q "${in}" out --compare-before-write
cmp "${in}" out || test_failed $LINENO
rm -f out || framework_failure
"${DDRESCUE}" -q "${in}" out --compare-before-write
cmp "${in}" out || test_failed $LINENO

printf "\ntesting ddrescuelog-%s..." "$2"

"${DDRESCUELOG}" -q mapfile
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUELOG}" -q -d
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUELOG}" -q -l+l "${map1}"
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUELOG}" -q -t -d mapfile
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUELOG}" -q -m "${map2i}" -t mapfile
[ $? = 2 ] || test_failed $LINENO
"${DDRESCUELOG}" -q --shift -i20 mapfile
[ $? = 1 ] || test_failed $LINENO

"${DDRESCUELOG}" -a '?,+' -i3072 - < "${map1}" > mapfile
"${DDRESCUELOG}" -D - < mapfile
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUELOG}" -D mapfile
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUELOG}" -a '?,+' -i2048 -s1024 mapfile > mapfile2
"${DDRESCUELOG}" -d - < mapfile2 || test_failed $LINENO
"${DDRESCUELOG}" -d mapfile2 || test_failed $LINENO
[ ! -e mapfile2 ] || test_failed $LINENO

# testing --create-mapfile and --list-blocks
"${DDRESCUELOG}" -b2048 -l+ - < "${map1}" > out || test_failed $LINENO
"${DDRESCUELOG}" -b2048 -c - < out > mapfile || test_failed $LINENO
"${DDRESCUELOG}" -b2048 -l+ mapfile > copy || test_failed $LINENO
cmp out copy || test_failed $LINENO
"${DDRESCUELOG}" -q -p "${map1}" mapfile
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUELOG}" -P "${map1}" mapfile || test_failed $LINENO
"${DDRESCUELOG}" -b2048 -s72776 -f '-c?+' mapfile < out || test_failed $LINENO
"${DDRESCUELOG}" -p "${map2}" - < mapfile || test_failed $LINENO
"${DDRESCUELOG}" -b2048 -f '-c?+' mapfile < out || test_failed $LINENO
"${DDRESCUELOG}" -s72776 -p "${map2}" mapfile || test_failed $LINENO
"${DDRESCUELOG}" -q -s72777 -p "${map2}" mapfile
[ $? = 1 ] || test_failed $LINENO

"${DDRESCUELOG}" -l+ -Flist - < "${map1}" | "${DDRESCUELOG}" '-c+?' out1 ||
	test_failed $LINENO
"${DDRESCUELOG}" -l+ -Fbitmap-le - < "${map1}" | \
	"${DDRESCUELOG}" '-c+?' -Fbitmap-le out2 || test_failed $LINENO
"${DDRESCUELOG}" -p out1 out2 || test_failed $LINENO
"${DDRESCUELOG}" -l+ -Fbitmap-be - < "${map1}" | \
	"${DDRESCUELOG}" '-c+?' -Fbitmap-be - > out2 || test_failed $LINENO
"${DDRESCUELOG}" -p out1 out2 || test_failed $LINENO
"${DDRESCUELOG}" -l+ -Fbitmap -o1s - < "${map1}" | \
	"${DDRESCUELOG}" '-c+?' -Fbitmap -i1s -o0 - > out2 || test_failed $LINENO
"${DDRESCUELOG}" -P out1 out2 || test_failed $LINENO
"${DDRESCUELOG}" -C out2 > out3 || test_failed $LINENO
"${DDRESCUELOG}" -p out1 out3 || test_failed $LINENO
"${DDRESCUELOG}" -l+ -Fbitmap -b8 - < "${map1}" | \
	"${DDRESCUELOG}" '-c+?' -Fbitmap -b8 - > out2 || test_failed $LINENO
"${DDRESCUELOG}" -P "${map1}" out2 || test_failed $LINENO
rm -f out1 out2 out3 || framework_failure

printf "10\n12\n14\n16\n" | "${DDRESCUELOG}" -b2048 -f '-c+?' mapfile ||
	test_failed $LINENO
"${DDRESCUELOG}" -q -p mapfile "${map1}"
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUELOG}" -q -i0x5000 -p mapfile "${map1}"
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUELOG}" -i0x5000 -s0x3800 -p mapfile "${map1}" || test_failed $LINENO

"${DDRESCUELOG}" -C "${map2i}" > mapfile || test_failed $LINENO
"${DDRESCUELOG}" -p "${map2}" mapfile || test_failed $LINENO
"${DDRESCUELOG}" -C "${map2ui}" > mapfile || test_failed $LINENO
"${DDRESCUELOG}" -p "${map2}" mapfile || test_failed $LINENO

cat "${map1}" > mapfile || framework_failure
"${DDRESCUELOG}" -i1024 -s2048 -d mapfile
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUELOG}" -i1024 -s1024 -d mapfile || test_failed $LINENO
"${DDRESCUELOG}" -q -i1024 -s1024 -d mapfile
[ $? = 1 ] || test_failed $LINENO

cat "${map2}" > mapfile || framework_failure
"${DDRESCUELOG}" -m "${map1}" -D mapfile
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUELOG}" -L -m - -D mapfile < "${map2i}" || test_failed $LINENO
"${DDRESCUELOG}" -i1024 -s2048 -d mapfile
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUELOG}" -i2048 -s2048 -d mapfile || test_failed $LINENO

"${DDRESCUELOG}" -b2048 -l+ "${map1}" > out || test_failed $LINENO
printf "0\n2\n4\n6\n8\n10\n12\n14\n16\n18\n20\n22\n24\n26\n28\n30\n32\n34\n" > copy || framework_failure
cmp out copy || test_failed $LINENO
"${DDRESCUELOG}" -b2048 '-l?-' "${map1}" > out || test_failed $LINENO
printf "1\n3\n5\n7\n9\n11\n13\n15\n17\n19\n21\n23\n25\n27\n29\n31\n33\n35\n" > copy || framework_failure
cmp out copy || test_failed $LINENO
"${DDRESCUELOG}" -b2048 -l+ -i0x1800 -o0 -s0x4000 "${map1}" > out ||
	test_failed $LINENO
printf "1\n3\n5\n7\n" > copy || framework_failure
cmp out copy || test_failed $LINENO

"${DDRESCUELOG}" -n "${map2}" > mapfile || framework_failure
"${DDRESCUELOG}" -b2048 -l+ mapfile > out || test_failed $LINENO
printf "0\n2\n4\n6\n8\n10\n12\n14\n16\n18\n20\n22\n24\n26\n28\n30\n32\n34\n" > copy || framework_failure
cmp out copy || test_failed $LINENO
"${DDRESCUELOG}" -b2048 '-l?-' mapfile > out || test_failed $LINENO
printf "1\n3\n5\n7\n9\n11\n13\n15\n17\n19\n21\n23\n25\n27\n29\n31\n33\n35\n" > copy || framework_failure
cmp out copy || test_failed $LINENO
"${DDRESCUELOG}" -b2048 -l+ -i2048 -o0 -s0x4000 mapfile > out ||
	test_failed $LINENO
printf "1\n3\n5\n7\n" > copy || framework_failure
cmp out copy || test_failed $LINENO

"${DDRESCUELOG}" -q -P "${map2i}" - < "${map2}"
[ $? = 2 ] || test_failed $LINENO
"${DDRESCUELOG}" -L -P "${map2i}" "${map2}" || test_failed $LINENO
"${DDRESCUELOG}" -L -P "${map2ui}" "${map2}" || test_failed $LINENO

# test XOR
for i in "${map1}" "${map2}" "${map3}" "${map4}" "${map5}" ; do
	for j in "${map1}" "${map2}" "${map3}" "${map4}" "${map5}" ; do
		"${DDRESCUELOG}" -x "$j" "$i" > out || test_failed $LINENO "$i $j"
		"${DDRESCUELOG}" -x "$i" "$j" > copy || test_failed $LINENO "$i $j"
		"${DDRESCUELOG}" -P out copy || test_failed $LINENO "$i $j"
		"${DDRESCUELOG}" -x "$j" out > copy || test_failed $LINENO "$i $j"
		"${DDRESCUELOG}" -P "$i" copy || test_failed $LINENO "$i $j"
	done
done

"${DDRESCUELOG}" -x "${map1}" - < "${map2}" > out || test_failed $LINENO
"${DDRESCUELOG}" -x "${map2}" "${map1}" > copy || test_failed $LINENO
"${DDRESCUELOG}" -p out copy || test_failed $LINENO
"${DDRESCUELOG}" -d out || test_failed $LINENO
"${DDRESCUELOG}" -d copy || test_failed $LINENO
"${DDRESCUELOG}" -x "${map1}" "${blank}" > out || test_failed $LINENO
"${DDRESCUELOG}" -x "${blank}" "${map1}" > copy || test_failed $LINENO
"${DDRESCUELOG}" -p out copy || test_failed $LINENO
"${DDRESCUELOG}" -p out "${map1}" || test_failed $LINENO
"${DDRESCUELOG}" -p "${map1}" copy || test_failed $LINENO
"${DDRESCUELOG}" -x "${map2}" "${map2}" > mapfile || test_failed $LINENO
"${DDRESCUELOG}" -P "${blank}" mapfile || test_failed $LINENO
"${DDRESCUELOG}" -x "${map1}" "${map1}" > mapfile || test_failed $LINENO
"${DDRESCUELOG}" -P "${blank}" mapfile || test_failed $LINENO
"${DDRESCUELOG}" -b2048 -l+ "${map1}" > out || test_failed $LINENO
"${DDRESCUELOG}" -b2048 -l- mapfile > copy || test_failed $LINENO
cmp out copy || test_failed $LINENO
"${DDRESCUELOG}" -b2048 -i0x2000 -s0x2800 -l+ "${map1}" > out ||
	test_failed $LINENO
"${DDRESCUELOG}" -i0x1800 -s0x3800 -x "${map1}" "${map1}" > mapfile ||
	test_failed $LINENO
"${DDRESCUELOG}" -b2048 -l- mapfile > copy || test_failed $LINENO
cmp out copy || test_failed $LINENO

"${DDRESCUELOG}" -x "${map3}" "${map4}" > out || test_failed $LINENO
"${DDRESCUELOG}" -x "${map4}" "${map3}" > copy || test_failed $LINENO
"${DDRESCUELOG}" -p out copy || test_failed $LINENO
"${DDRESCUELOG}" -x "${map3}" "${map5}" > out || test_failed $LINENO
"${DDRESCUELOG}" -x "${map5}" "${map3}" > copy || test_failed $LINENO
"${DDRESCUELOG}" -p out copy || test_failed $LINENO
"${DDRESCUELOG}" -x "${map4}" "${map5}" > out || test_failed $LINENO
"${DDRESCUELOG}" -x "${map5}" "${map4}" > copy || test_failed $LINENO
"${DDRESCUELOG}" -p out copy || test_failed $LINENO

"${DDRESCUELOG}" -x "${map3}" "${map4}" > out || test_failed $LINENO
"${DDRESCUELOG}" -D out && test_failed $LINENO
"${DDRESCUELOG}" -x out "${map5}" > mapfile || test_failed $LINENO
"${DDRESCUELOG}" -d mapfile || test_failed $LINENO

"${DDRESCUELOG}" -x "${map3}" "${map5}" > out || test_failed $LINENO
"${DDRESCUELOG}" -D out && test_failed $LINENO
"${DDRESCUELOG}" -x out "${map4}" > mapfile || test_failed $LINENO
"${DDRESCUELOG}" -d mapfile || test_failed $LINENO

"${DDRESCUELOG}" -x "${map4}" "${map3}" > out || test_failed $LINENO
"${DDRESCUELOG}" -D out && test_failed $LINENO
"${DDRESCUELOG}" -x out "${map5}" > mapfile || test_failed $LINENO
"${DDRESCUELOG}" -d mapfile || test_failed $LINENO

"${DDRESCUELOG}" -x "${map4}" "${map5}" > out || test_failed $LINENO
"${DDRESCUELOG}" -D out && test_failed $LINENO
"${DDRESCUELOG}" -x out "${map3}" > mapfile || test_failed $LINENO
"${DDRESCUELOG}" -d mapfile || test_failed $LINENO

"${DDRESCUELOG}" -x "${map5}" "${map3}" > out || test_failed $LINENO
"${DDRESCUELOG}" -D out && test_failed $LINENO
"${DDRESCUELOG}" -x out "${map4}" > mapfile || test_failed $LINENO
"${DDRESCUELOG}" -d mapfile || test_failed $LINENO

"${DDRESCUELOG}" -x "${map5}" "${map4}" > out || test_failed $LINENO
"${DDRESCUELOG}" -D out && test_failed $LINENO
"${DDRESCUELOG}" -x out "${map3}" > mapfile || test_failed $LINENO
"${DDRESCUELOG}" -d mapfile || test_failed $LINENO

# test AND
for i in "${map1}" "${map2}" "${map3}" "${map4}" "${map5}" ; do
	for j in "${map1}" "${map2}" "${map3}" "${map4}" "${map5}" ; do
		"${DDRESCUELOG}" -y "$j" "$i" > out || test_failed $LINENO "$i $j"
		"${DDRESCUELOG}" -y "$i" "$j" > copy || test_failed $LINENO "$i $j"
		"${DDRESCUELOG}" -P out copy || test_failed $LINENO "$i $j"
	done
done

"${DDRESCUELOG}" -b2048 -l+ "${map1}" > out || test_failed $LINENO
"${DDRESCUELOG}" -y "${map1}" - < "${map2}" > mapfile || test_failed $LINENO
"${DDRESCUELOG}" -P "${blank}" mapfile || test_failed $LINENO
"${DDRESCUELOG}" -b2048 '-l?' mapfile > copy || test_failed $LINENO
cmp out copy || test_failed $LINENO
"${DDRESCUELOG}" -y "${map2}" "${map1}" > mapfile || test_failed $LINENO
"${DDRESCUELOG}" -P "${blank}" mapfile || test_failed $LINENO
"${DDRESCUELOG}" -b2048 -l- mapfile > copy || test_failed $LINENO
cmp out copy || test_failed $LINENO
"${DDRESCUELOG}" -b2048 -i0x2000 -s0x2800 -l+ "${map1}" > out ||
	test_failed $LINENO
"${DDRESCUELOG}" -i0x1800 -s0x3800 -y "${map2}" "${map1}" > mapfile ||
	test_failed $LINENO
"${DDRESCUELOG}" -b2048 -l- mapfile > copy || test_failed $LINENO
cmp out copy || test_failed $LINENO

"${DDRESCUELOG}" -y "${map3}" "${map4}" > out || test_failed $LINENO
"${DDRESCUELOG}" -P "${blank}" out || test_failed $LINENO
"${DDRESCUELOG}" -y "${map3}" "${map5}" > out || test_failed $LINENO
"${DDRESCUELOG}" -P "${blank}" out || test_failed $LINENO
"${DDRESCUELOG}" -y "${map4}" "${map5}" > out || test_failed $LINENO
"${DDRESCUELOG}" -P "${blank}" out || test_failed $LINENO

"${DDRESCUELOG}" -i0x2000 -s0x2800 -z "${map2}" "${map1}" > mapfile ||
	test_failed $LINENO
"${DDRESCUELOG}" -D mapfile
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUELOG}" -i0x1C00 -s0x2C00 -D mapfile
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUELOG}" -i0x2000 -s0x2C00 -D mapfile
[ $? = 1 ] || test_failed $LINENO
"${DDRESCUELOG}" -i0x2000 -s0x2800 -d mapfile || test_failed $LINENO

# test OR
for i in "${map1}" "${map2}" "${map3}" "${map4}" "${map5}" ; do
	for j in "${map1}" "${map2}" "${map3}" "${map4}" "${map5}" ; do
		"${DDRESCUELOG}" -z "$j" "$i" > out || test_failed $LINENO "$i $j"
		"${DDRESCUELOG}" -z "$i" "$j" > copy || test_failed $LINENO "$i $j"
		"${DDRESCUELOG}" -P out copy || test_failed $LINENO "$i $j"
	done
done

"${DDRESCUELOG}" -z "${map1}" - < "${map2}" > out || test_failed $LINENO
"${DDRESCUELOG}" -z "${map2}" "${map1}" > copy || test_failed $LINENO
"${DDRESCUELOG}" -p out copy || test_failed $LINENO
"${DDRESCUELOG}" -d out || test_failed $LINENO
"${DDRESCUELOG}" -d copy || test_failed $LINENO
"${DDRESCUELOG}" -z "${map1}" "${blank}" > out || test_failed $LINENO
"${DDRESCUELOG}" -z "${blank}" "${map1}" > copy || test_failed $LINENO
"${DDRESCUELOG}" -p out copy || test_failed $LINENO
"${DDRESCUELOG}" -p out "${map1}" || test_failed $LINENO
"${DDRESCUELOG}" -p "${map1}" copy || test_failed $LINENO
"${DDRESCUELOG}" -z "${map3}" "${map4}" > out || test_failed $LINENO
"${DDRESCUELOG}" -z "${map4}" "${map3}" > copy || test_failed $LINENO
"${DDRESCUELOG}" -p out copy || test_failed $LINENO
"${DDRESCUELOG}" -z "${map3}" "${map5}" > out || test_failed $LINENO
"${DDRESCUELOG}" -z "${map5}" "${map3}" > copy || test_failed $LINENO
"${DDRESCUELOG}" -p out copy || test_failed $LINENO
"${DDRESCUELOG}" -z "${map4}" "${map5}" > out || test_failed $LINENO
"${DDRESCUELOG}" -z "${map5}" "${map4}" > copy || test_failed $LINENO
"${DDRESCUELOG}" -p out copy || test_failed $LINENO

"${DDRESCUELOG}" -z "${map3}" "${map4}" > out || test_failed $LINENO
"${DDRESCUELOG}" -D out && test_failed $LINENO
"${DDRESCUELOG}" -z out "${map5}" > mapfile || test_failed $LINENO
"${DDRESCUELOG}" -d mapfile || test_failed $LINENO

"${DDRESCUELOG}" -z "${map3}" "${map5}" > out || test_failed $LINENO
"${DDRESCUELOG}" -D out && test_failed $LINENO
"${DDRESCUELOG}" -z out "${map4}" > mapfile || test_failed $LINENO
"${DDRESCUELOG}" -d mapfile || test_failed $LINENO

"${DDRESCUELOG}" -z "${map4}" "${map3}" > out || test_failed $LINENO
"${DDRESCUELOG}" -D out && test_failed $LINENO
"${DDRESCUELOG}" -z out "${map5}" > mapfile || test_failed $LINENO
"${DDRESCUELOG}" -d mapfile || test_failed $LINENO

"${DDRESCUELOG}" -z "${map4}" "${map5}" > out || test_failed $LINENO
"${DDRESCUELOG}" -D out && test_failed $LINENO
"${DDRESCUELOG}" -z out "${map3}" > mapfile || test_failed $LINENO
"${DDRESCUELOG}" -d mapfile || test_failed $LINENO

"${DDRESCUELOG}" -z "${map5}" "${map3}" > out || test_failed $LINENO
"${DDRESCUELOG}" -D out && test_failed $LINENO
"${DDRESCUELOG}" -z out "${map4}" > mapfile || test_failed $LINENO
"${DDRESCUELOG}" -d mapfile || test_failed $LINENO

"${DDRESCUELOG}" -z "${map5}" "${map4}" > out || test_failed $LINENO
"${DDRESCUELOG}" -D out && test_failed $LINENO
"${DDRESCUELOG}" -z out "${map3}" > mapfile || test_failed $LINENO
"${DDRESCUELOG}" -d mapfile || test_failed $LINENO

# test ( a && b ) == !( !a || !b )
for i in "${map1}" "${map2}" "${map3}" "${map4}" "${map5}" ; do
	for j in "${map1}" "${map2}" "${map3}" "${map4}" "${map5}" ; do
		"${DDRESCUELOG}" -n "$i" > na || test_failed $LINENO "$i $j"
		"${DDRESCUELOG}" -n "$j" > nb || test_failed $LINENO "$i $j"
		"${DDRESCUELOG}" -z nb na > out || test_failed $LINENO "$i $j"
		"${DDRESCUELOG}" -n out > copy || test_failed $LINENO "$i $j"
		"${DDRESCUELOG}" -y "$j" "$i" > out || test_failed $LINENO "$i $j"
		"${DDRESCUELOG}" -P out copy || test_failed $LINENO "$i $j"
	done
done

"${DDRESCUELOG}" --shift -o0x800 -s0x11448 "${map1}" > out || test_failed $LINENO
"${DDRESCUELOG}" -p "${map2}" out || test_failed $LINENO

"${DDRESCUELOG}" --shift -o0x400 "${map1}" > out || test_failed $LINENO
"${DDRESCUELOG}" --shift -o0x400 -s0x11848 out > copy || test_failed $LINENO
"${DDRESCUELOG}" -p "${map2}" copy || test_failed $LINENO

"${DDRESCUELOG}" --shift -o0x400 "${map2}" > out || test_failed $LINENO
"${DDRESCUELOG}" --shift -i0x400 -o0 out > copy || test_failed $LINENO
"${DDRESCUELOG}" -p "${map2}" copy || test_failed $LINENO

"${DDRESCUELOG}" --shift -o0x900 "${map1}" > copy || test_failed $LINENO
"${DDRESCUELOG}" --shift -i0x300 -o0 copy > out || test_failed $LINENO
"${DDRESCUELOG}" --shift -i0x600 -o0 out > copy || test_failed $LINENO
"${DDRESCUELOG}" -p "${map1}" copy || test_failed $LINENO

"${DDRESCUELOG}" --shift -i0x800 -o0 "${map2}" > copy || test_failed $LINENO
"${DDRESCUELOG}" --shift -o0x488 copy > out || test_failed $LINENO
"${DDRESCUELOG}" --shift -o0x378 out > copy || test_failed $LINENO
"${DDRESCUELOG}" -p "${map2}" copy || test_failed $LINENO

printf "\ntesting combined rescue..."

rm -f mapfile || framework_failure
"${DDRESCUE}" -q "${in}" out mapfile			# rescue partition
"${DDRESCUELOG}" --shift -o45 mapfile > shifted_mapfile
"${DDRESCUE}" -q --same-file -o45 -s72776 --reverse out out
"${DDRESCUE}" -q -C "${fox}" out shifted_mapfile	# rescue rest of drive
cat "${fox}" "${in}" | cmp out - || test_failed $LINENO
"${DDRESCUELOG}" -d mapfile || test_failed $LINENO
"${DDRESCUELOG}" -d shifted_mapfile || test_failed $LINENO

echo
if [ ${fail} = 0 ] ; then
	echo "tests completed successfully."
	cd "${objdir}" && rm -r tmp
else
	echo "tests failed."
fi
exit ${fail}
