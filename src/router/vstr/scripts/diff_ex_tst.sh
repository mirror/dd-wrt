#! /bin/sh

if false; then
echo "Do nothing"
elif [ -r ./scripts ]; then
# In source dir
       sex=./examples
       bex=./examples
elif [ -r ../scripts ]; then
# In build dir or source dir/examples
if [ -r examples ]; then
# In build dir
       sex=../examples
       bex=./examples
else
# In source dir/examples
       sex=.
       bex=.
fi
elif [ -r ../../scripts ]; then
# In build dir/examples
       sex=../../examples
       bex=.
else
 echo "No scripts dir"
 exit 1;
fi

prefix=$1
num=$2

diff -u ${sex}/tst/ex_${prefix}_out_$num ${bex}/ex_${prefix}_tmp_$num 

