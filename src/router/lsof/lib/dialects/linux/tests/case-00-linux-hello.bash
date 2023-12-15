report=$2
tdir=$3

{
    # This make invocaiton is needed to
    # run test cases locally, not CI environment.
    make -C $tdir || exit 1
} > $report

exit 0
