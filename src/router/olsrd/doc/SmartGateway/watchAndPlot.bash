#!/bin/bash

set -e
set -u

ifsOld="$IFS"
IFS=$'\n'
gnuplotfiles=( $(ls -1 *.gnuplot | sort) )
IFS="$ifsOld"
gnuplotfilestimes=()

# init time stamps
declare -i index=0
while [[ $index -lt ${#gnuplotfiles[*]} ]]; do
  gnuplotfilestimes[$index]=1
  index+=1
done


declare -i tim=0
while true; do
  index=0
  while [[ $index -lt ${#gnuplotfiles[*]} ]]; do
    tim=$(stat -c "%Y" "${gnuplotfiles[index]}")
    if [[ $tim -ne ${gnuplotfilestimes[index]} ]]; then
      gnuplot "${gnuplotfiles[index]}"
      gnuplotfilestimes[$index]=$tim
    fi
    index+=1
  done
  sleep 1
done
