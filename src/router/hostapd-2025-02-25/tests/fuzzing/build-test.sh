#!/bin/sh

error()
{
    echo "Build test failed"
    exit 1
}

JOBS=`nproc`
if [ -z "$ABC" ]; then
    JOBS=8
fi

for i in *; do
    if [ -d $i ]; then
	cd $i
	make clean
	make QUIET=1 -j$JOBS || error
	make clean
	cd ..
    fi
done

echo "Build test succeeded"
