#! /bin/sh

sr="--show-reachable=yes"
# Run all exe's in the tst directory under valgrind.
# Running scripts upsets it, so do the LD thing ourselves
for i in tst/*/.libs/tst_*; do
   if [ -x $i ]; then # Skip .o
	echo $i
	LD_LIBRARY_PATH=src/.libs valgrind -q $sr --leak-check=yes $i 2>&1 | \
     egrep '(= (definitely lost|possibly lost|still reachable|ERROR SUMMARY)|assert failed)'
   fi
done

exit 0;
