name=$(basename $0 .bash)
lsof=$1
report=$2
base=$(pwd)


if [ $(${lsof} -r 1c1 -p $$ | tee -a $report | grep -e '=======' | wc -l) != 1 ]; then
    exit 1
fi

if [ $(${lsof} -r 1c5 -p $$ | tee -a $report | grep -e '=======' | wc -l) != 5 ]; then
    exit 1
fi

exit 0
