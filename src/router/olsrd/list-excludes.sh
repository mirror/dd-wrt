#!/bin/sh
#
#
# LÖist the to be excluded files
#

find -name ".cvsignore" -print | while read f
do
  d=${f%.cvsignore}
  d=${d#./}
  while read l
  do
    echo "$d$l"
  done < $f
done
echo 'CVS'
echo '*~'
echo '#*#'
echo '.#*'
echo '*.o'
