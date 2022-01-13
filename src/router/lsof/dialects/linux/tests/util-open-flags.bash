lsof=$1
report=$2
tdir=$3
dialect=$4
pat=$5
tfile=$6

shift 6

TARGET=$tdir/open_with_flags
if ! [ -x $TARGET ]; then
    echo "target executable ( $TARGET ) is not found" >> $report
    exit 1
fi

$TARGET $tfile "$@" &
pid=$!

echo "expected: $pat" >> $report
echo "lsof output:" >> $report
$lsof +fg -p $pid | tee -a $report | grep -q "$pat"
result=$?

kill $pid

exit $result
