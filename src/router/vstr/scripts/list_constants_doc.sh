#! /bin/sh

if false; then
echo "Do nothing"
elif [ -r ./Documentation/constants.txt ]; then
      doc=./Documentation
elif [ -r ../Documentation/constants.txt ]; then
      doc=../Documentation
else
 echo "No Documentation dir"
 exit 1;
fi

egrep "^Constant" $doc/constants.txt | awk '{ print $2 }'
