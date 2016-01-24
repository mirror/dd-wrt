#! /bin/zsh

for i in tst/**/.libs/tst_*; do

 nm -u "$i" | egrep "^  *U vstr_" | sed -e 's/$/()/;' | awk '{ print $2 }'

done | sort | uniq

