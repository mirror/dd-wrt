#!/bin/sh
#use this script with 1k blocksize and 8k clustersize

verbose=0
only_check=0

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color
test_file() {
    filename=$1
    hash=$2

    md5sum $filename | grep $hash 2>&1 >/dev/null
    if [ $? -eq 0 ]
    then
        echo -e "${GREEN}${filename} okay${NC}"
    else
        echo -e "${RED}${filename} fail${NC}"
        if [ $verbose -eq 1 ]; then
	    echo "Expected md5sum: $( md5sum $filename )"
            hexdump -C $filename
        fi
    fi
}

show_help() {
    echo "Usage: $0 [OPTION]... [Directory]..."
    echo ""
    echo "Create and/or check test files in one or more directories"
    echo "-c     Only check files in Directories"
    echo "-v     Verbose output (print hexdump on invalid files"
    echo "-h     Show this help"
}

OPTIND=1         # Reset in case getopts has been used previously in the shell.
while getopts "h?cv" opt; do
	case "$opt" in
		c)
			only_check=1
			;;
		v)
			verbose=1
			;;
		h|\?)
			show_help
			exit 0
			;;
	esac
done
shift $((OPTIND-1))


if [ $# -eq 0 ] ; then
    show_help
    exit 1
fi

#ntfsdir=$1
for ntfsdir in "$@"
do

    if [ $only_check -eq 0 ]; then
        rm -f $ntfsdir/seektest*

        #prepare testdata 1024(2^10) bytes with char 'y'
        va=a
	vb=b
	vc=c
	vd=d
	ve=e
	vf=f
	vg=g

        for a in $( seq 10 ); do
            va="$va$va"
            vb="$vb$vb"
            vc="$vc$vc"
            vd="$vd$vd"
            ve="$ve$ve"
            vf="$vf$vf"
            vg="$vg$vg"
        done



	# 0000..0800 |................|
	# 0800..0c00 |aaaaaaaaaaaaaaaa|
	# 0c00..2000 |................|
        echo -n "$va" | dd of=$ntfsdir/seektest1 bs=1024 seek=2 count=1 2>&1 \
		>/dev/null
        dd of=$ntfsdir/seektest1 bs=1024 seek=8 count=0 2>&1 >/dev/null

	# 0000..0400 |bbbbbbbbbbbbbbbb|
	# 0400..2000 |................|
        echo -n "$vb" | dd of=$ntfsdir/seektest2 bs=1024 seek=0 count=1 2>&1 \
		>/dev/null
        dd of=$ntfsdir/seektest2 bs=1024 seek=3 count=0 2>&1 >/dev/null
        dd of=$ntfsdir/seektest2 bs=1024 seek=8 count=0 2>&1 >/dev/null

	# 0000 .. 2000: |................|
        dd of=$ntfsdir/seektest3 bs=1024 seek=8 count=0 2>&1 >/dev/null
	# 0000 .. 0800: |................|
	# 0800 .. 0c00: |cccccccccccccccc|
	# 0c00 .. 2000: |................|
        echo -n "$vc" | dd of=$ntfsdir/seektest3 bs=1024 seek=2 count=1 \
		conv=notrunc 2>&1 >/dev/null
	# 0000 .. 0800: |................|
	# 0800 .. 0c00: |cccccccccccccccc|
	# 0c00 .. 5c00: |................|
	# 5c00 .. 6000: |cccccccccccccccc|
	echo -n "$vc" | dd of=$ntfsdir/seektest3 bs=1024 seek=23 count=1 \
		conv=notrunc 2>&1 >/dev/null

	# 0000 .. 0600 |................|
	# 0600 .. 0800 |dddddddddddddddd|
        echo -n "$vd" | dd of=$ntfsdir/seektest4 bs=512 seek=3 count=1 \
		conv=notrunc 2>&1 >/dev/null

	# 0000 .. 0100 |................|
	# 0100 .. 0180 |eeeeeeeeeeeeeeee|
	# 0180 .. 0400 |................|
        dd of=$ntfsdir/seektest5 bs=128 seek=8 count=0 2>&1 >/dev/null
        echo -n "$ve" | dd of=$ntfsdir/seektest5 bs=128 seek=2 count=1 \
		conv=notrunc 2>&1 >/dev/null

	# 0000 .. 0400: |ffffffffffffffff|
	# 0400 .. 3400: |................|
	# 3400 .. 3800: |ffffffffffffffff|
	# 3800 .. 8c00: |................|
	# 8c00 .. 9000: |ffffffffffffffff|
        echo -n "$vf" | dd of=$ntfsdir/seektest6 bs=1024 seek=0 count=1 2>&1 \
		>/dev/null
        echo -n "$vf" | dd of=$ntfsdir/seektest6 bs=1024 seek=35 count=1 2>&1 \
		>/dev/null
        echo -n "$vf" | dd of=$ntfsdir/seektest6 bs=1024 seek=13 count=1 \
		conv=notrunc 2>&1 >/dev/null

	# 0000 .. 0800: |gggggggggggggggg|
        echo -n "$vg" >> $ntfsdir/seektest7
        echo -n "$vg" >> $ntfsdir/seektest7
    fi

    test_file $ntfsdir/seektest1 712a97fd7ed026b873ba88dd179240d6
    test_file $ntfsdir/seektest2 9e06718b4e9cb5ae322e75563caaf083
    test_file $ntfsdir/seektest3 a7bab1b1fefa47be85b6144f1798e9e5
    test_file $ntfsdir/seektest4 9fd654309299dbdc036dc4df9d149bb7
    test_file $ntfsdir/seektest5 f701a839323485af8bd8877b06ac38c8
    test_file $ntfsdir/seektest6 9ef9688414af01efeabf3f455c7539d4
    test_file $ntfsdir/seektest7 0daa39e549ee743bc773f30b2dcba10a

done
