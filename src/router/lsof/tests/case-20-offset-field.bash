name=$(basename $0 .bash)
lsof=$1
report=$2
base=$(pwd)

t=/tmp/lsof-test-reg-file-$$
p=/tmp/lsof-test-reg-fifo-$$

mkfifo $p
{
    printf "%d" 1
    read < $p &
} | cat > $t &

r=1
if [ "$($lsof -Fo $t | grep '^o')" = o0t1 ]; then
    echo > $p
    r=0
fi

rm /tmp/lsof-test-reg-file-$$
rm /tmp/lsof-test-reg-fifo-$$

exit $r
