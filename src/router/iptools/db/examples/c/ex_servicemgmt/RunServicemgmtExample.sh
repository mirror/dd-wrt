#!/bin/sh

if [ ! -f ../../../build_unix/ServicemgmtExample ]; then
    rm -rf ../../../build_unix/ServicemgmtExample
fi


mkdir ../../../build_unix/ServicemgmtExample

cp customer.txt engineer.txt service.txt ../../../build_unix/ServicemgmtExample

cd ../../../build_unix

./ex_servicemgmt



