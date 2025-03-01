#!/bin/sh

if [ ! -f ./PriorityExample ]; then
    rm -r ./PriorityExample
fi
if [ ! -f ./Destination_Env ]; then
    rm -r ./Destination_Env
fi
mkdir ./PriorityExample
mkdir ./Destination_Env

echo "Starting message simulator and generating 100,000 messages."
../../../build_unix/ex_priority_msg &
echo "Starting priority sorter."
../../../build_unix/ex_priority &
echo "Starting destination site."
../../../build_unix/ex_priority_dest