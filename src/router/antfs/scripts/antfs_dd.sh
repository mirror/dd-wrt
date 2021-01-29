#!/bin/sh

if [ $# -eq 0 ] ; then
	echo "<INFO> Usage:  $0  <device>"
	exit 1
fi

ntfsdir=`mount | grep $1 | cut -d' ' -f3`

for a in $( seq 10 ); do
	echo 12345678901234567890 >> $ntfsdir/test &
	dd of=$ntfsdir/test seek=0 count=0 &>/dev/null &
	echo 12345678901234567890 >> $ntfsdir/test &
	dd of=$ntfsdir/test seek=0 count=0 &>/dev/null &

	for job in `jobs -p`; do
		wait $job
	done
done

echo "<INFO> Finished successfully"
