#!/bin/sh

mhd_listen_ip='127.0.0.1'
max_runtime_sec='1800'

if test "x${ZZUF}" = "xno" ; then
  echo "zzuf command missing" 1>&2
  exit 77
fi

if command -v "${ZZUF}" > /dev/null 2>&1 ; then : ; else
  echo "zzuf command missing" 1>&2
  exit 77
fi

run_with_socat ()
{
  echo "Trying to run the test with socat..."
  script_dir=""
  if command -v dirname > /dev/null 2>&1 ; then
    test_dir=`dirname /`
    if test "x${test_dir}" = "x/" ; then
      if dirname "$1" > /dev/null 2>&1 ; then
        script_dir=`dirname "$1"`
        if test -n "${script_dir}" ; then
          # Assume script is not in the root dir
          script_dir="${script_dir}/"
        else
          script_dir="./"
        fi
      fi
    fi
  fi
  if test -z "${script_dir}" ; then
    if echo "$1" | sed 's|[^/]*$||' > /dev/null 2>&1 ; then
      script_dir=`echo "$1" | sed 's|[^/]*$||'`
        if test -z "${script_dir}" ; then
          script_dir="./"
        fi
    fi
  fi
  if test -z "${script_dir}" ; then
    echo "Cannot determine script location, will try current directory." 1>&2
    script_dir="./"
  fi
  $SHELL "${script_dir}zzuf_socat_test_runner.sh" "$@"
  exit $?
}

# zzuf cannot pass-through the return value of checked program
# so try the direct dry-run first to get possible 77 or 99 codes
echo "## Dry-run of the $@..."
if "$@" --dry-run ; then
  echo "# Dry-run succeeded."
else
  res_code=$?
  echo "Dry-run failed with exit code $res_code." 1>&2
  if test $res_code -ne 99; then
    run_with_socat "$@"
  fi
  echo "$@ will not be run with zzuf." 1>&2
  exit $res_code
fi

# fuzz the input only for IP ${mhd_listen_ip}. libcurl uses another IP
# in this test therefore libcurl input is not fuzzed.
zzuf_all_params="--ratio=0.001:0.4 --autoinc --verbose --signal \
 --max-usertime=${max_runtime_sec} --check-exit --network \
 --allow=${mhd_listen_ip} --exclude=."

if test -n "${ZZUF_SEED}" ; then
  zzuf_all_params="${zzuf_all_params} --seed=${ZZUF_SEED}"
fi

if test -n "${ZZUF_FLAGS}" ; then
  zzuf_all_params="${zzuf_all_params} ${ZZUF_FLAGS}"
fi

# Uncomment the next line to see more data in logs
#zzuf_all_params="${zzuf_all_params} -dd"

echo "## Dry-run of the $@ with zzuf..."
if "$ZZUF" ${zzuf_all_params} "$@" --dry-run ; then
  echo "# Dry-run with zzuf succeeded."
else
  res_code=$?
  echo "$@ cannot be run with zzuf directly." 1>&2
  run_with_socat "$@"
  exit $res_code
fi

echo "## Real test of $@ with zzuf..."
"$ZZUF" ${zzuf_all_params} "$@"
exit $?
