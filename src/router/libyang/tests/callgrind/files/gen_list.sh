#!/bin/sh

for i in $(seq 1000)
do
  echo "    <list1><key1>vl$i</key1></list1>" >> list
done

for i in $(seq 1000)
do
  echo "    <llist1>vl$i</llist1>" >> list
done
