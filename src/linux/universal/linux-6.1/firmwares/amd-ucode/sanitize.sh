#!/bin/sh

echo "str" > list.txt
for i in *.bin
do
sed "\$s/\$/amd-ucode\/$i\ /" list.txt > _list.txt && mv -- _list.txt list.txt
#echo "#include \"$i\"" >> list.txt
done

#ex -sc '$s/$/$i/' list.txt