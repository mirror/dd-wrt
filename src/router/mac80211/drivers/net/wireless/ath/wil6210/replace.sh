for i in *.c *.h; do sed "s/$1/$2/" $i > $i.copy; cp $i.copy $i; rm $i.copy; done
