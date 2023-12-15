set -e

if [ $# = 0 ]; then
    echo "Usage: $0 DIALECT" 1>&2
    exit 1
fi

if ! [ -d lib/dialects/$1 ]; then
    echo "No such dialect: $1" 1>&2
    exit 1
fi

echo
echo RUNTIME ENVIRONMENT INFORMATION
echo =============================================================================
dialect=$1
echo "$dialect"
echo "$BASH_VERSION"
shopt
export

uname

lsof=$(pwd)/lsof
$lsof -v

export CI=1

tdir=lib/dialects/${dialect}/tests

nfailed=0
nsuccessful=0
nskipped=0
ncases=0
REPORTS=
REPORTS_SKIPPED=


run_one()
{
    local x=$1
    local d=$2
    local name
    local prefix
    local report
    local s

    chmod a+x $x
    name=$(basename $x .bash)
    if [ ${x%%/*} = "dialects" ]; then
        prefix=${dialect}-
    fi
    report=/tmp/${prefix}$name-$$.report

    printf "%s ... " $name

    set +e
    bash ./$x $lsof $report $d $dialect
    s=$?
    set -e
    ncases=$((ncases + 1))
    if [ "$s" = 0 ]; then
        s=ok
        nsuccessful=$((nsuccessful + 1))
        rm -f "$report"
    elif [ "$s" = 77 ]; then
        s=skipped
        nskipped=$((nskipped + 1))
        REPORTS_SKIPPED="${REPORTS_SKIPPED} ${report}"
    else
        s=failed
        nfailed=$((nfailed + 1))
        REPORTS="${REPORTS} ${report}"
    fi

    printf "%s\n" $s
}

echo
echo STARTING TEST '(' dialect neutral ')'
echo =============================================================================
for x in tests/case-*.bash; do
    run_one $x ./tests
done

echo
echo STARTING TEST '(' $dialect specific ')'
echo =============================================================================
for x in lib/dialects/${dialect}/tests/case-*.bash; do
    run_one $x $tdir
done

report()
{
    for r in "$@"; do
        echo
        echo '[failed]' $r
        echo -----------------------------------------------------------------------------
        cat $r
        rm $r
    done
}

report_skipped()
{
    for r in "$@"; do
        echo
        echo '[skipped]' $r
        echo -----------------------------------------------------------------------------
        cat $r
        rm $r
    done
}

echo
echo TEST SUMMARY
echo =============================================================================
printf "successful: %d\n" $nsuccessful
printf "skipped: %d\n" $nskipped
printf "failed: %d\n" $nfailed

if [ $nfailed = 0 ]; then
    printf "All %d test cases are passed successfully\n" $ncases
    if [ $nskipped = 0 ]; then
        :
    elif [ $nskipped = 1 ]; then
        printf "but 1 case is skipped\n"
        report $REPORTS
    else
        printf "but %d cases are skipped\n" $nskipped
        report $REPORTS
    fi
    exit 0
elif [ $nfailed = 1 ]; then
    printf "%d of %d case is failed\n" $nfailed $ncases
    report $REPORTS
    report_skipped $REPORTS_SKIPPED
    exit 1
else
    printf "%d of %d cases are failed\n" $nfailed $ncases
    report $REPORTS
    report_skipped $REPORTS_SKIPPED    
    exit 1
fi
