#! /bin/sh

if false; then
echo "Do nothing"
elif [ -r ./Documentation/functions.txt ]; then
      doc=./Documentation
elif [ -r ../Documentation/functions.txt ]; then
      doc=../Documentation
else
 echo "No Documentation dir"
 exit 1;
fi

egrep "^Function" $doc/functions.txt | awk '{ print $2 }' | \
  egrep -v "^VSTR_FLAGXX"
