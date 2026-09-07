#!/bin/sh
################################################################################
#
# run-privoxy-tests.sh
#
# Runs the Privoxy tests based on curl's runtests.pl framework.
#
# Copyright (c) 2021-2026 Fabian Keil <fk@fabiankeil.de>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
################################################################################

UPSTREAM_TEST_SCENARIO=upstream-tests

# Delaying the test run by a whole second is annoying on fast systems
# and may not be long enough on slow systems. There must be a more
# elegant method to verify that Privoxy is running or failed to start.
SECONDS_TO_WAIT_FOR_PRIVOXY_TO_START=1

USE_VALGRIND=false

start_privoxy() {
    local test_dir test_scenario
    local privoxy_config privoxy_binary pid_file log_file valgrind_command valgrind_log_file
    test_dir="${1}"
    test_scenario="${2}"

    privoxy_config_dir="${test_dir}/${test_scenario}"
    privoxy_binary="$(realpath "${test_dir}/../../privoxy")"
    pid_file="${test_dir}/${test_scenario}/../privoxy.pid"
    log_file="${test_dir}/logs/${test_scenario}.log"

    if $USE_VALGRIND; then
        valgrind_log_file="${test_dir}/logs/${test_scenario}-valgrind.log"
        valgrind_command="valgrind --tool=memcheck --leak-check=full --log-file=$valgrind_log_file
           --trace-children=yes --track-origins=yes"
        SECONDS_TO_WAIT_FOR_PRIVOXY_TO_START=3
    fi

     (
        cd "${privoxy_config_dir}" || exit 1
        $valgrind_command "${privoxy_binary}" --no-daemon \
                            --pidfile "${pid_file}" \
                            privoxy.conf > "${log_file}" 2>&1 || exit 1 &
    )

    sleep "${SECONDS_TO_WAIT_FOR_PRIVOXY_TO_START}"

    if [ ! -f "${pid_file}" ]; then
        echo "Privoxy failed to start or did not start in time"
        if [ -f "${log_file}" ]; then
            tail -n 1 "${log_file}"
        fi
        # Kill the whole process group including Privoxy
        # if it started after the PID file was checked.
        kill -- -$$
    fi
}

stop_privoxy() {
    local test_dir test_scenario
    local pid_file
    test_dir="${1}"
    test_scenario="${2}"
    pid_file="${test_dir}/${test_scenario}/../privoxy.pid"
    if [ -f "${pid_file}" ]; then
        kill "$(cat "${pid_file}")"
    fi
}

run_privoxy_tests() {
    local start_privoxy="${1}"; shift
    local test_scenario="${1}"; shift
    local directory_name="$(dirname "$0")"
    local test_dir="$(realpath "${directory_name}")"
    local ret

    echo "Test scenario: ${test_scenario}"
    $start_privoxy && start_privoxy "${test_dir}" "${test_scenario}"

    "${test_dir}/runtests-wrapper.sh" -E -t "${test_dir}/${test_scenario}/data" HTTP HTTPS "${@}"
    ret=$?

    $start_privoxy && stop_privoxy "${test_dir}" "${test_scenario}"

    return $ret
}

run_upstream_tests() {
    local start_privoxy="${1}"; shift
    local directory_name="$(dirname "$0")"
    local test_dir="$(realpath "${directory_name}")"
    local ret

    echo "Test scenario: ${UPSTREAM_TEST_SCENARIO}"
    $start_privoxy && start_privoxy "${test_dir}" "${UPSTREAM_TEST_SCENARIO}"

    "${test_dir}/runtests-wrapper.sh" -T HTTP "${@}"
    ret=$?

    $start_privoxy && stop_privoxy "${test_dir}" "${UPSTREAM_TEST_SCENARIO}"

    return $ret
}

get_test_scenarios() {
    local directory_name="$(dirname "$0")"
    local test_dir="$(realpath "${directory_name}")"
    local test_scenario
    local privoxy_config

    for privoxy_config in "${test_dir}"/*/privoxy.conf; do
        test_scenario="${privoxy_config%%/privoxy.conf}"
        test_scenario="${test_scenario##$test_dir/}"
        echo "${test_scenario}"
    done
}

main() {
    local test_scenario=""
    local test_scenarios=""
    local start_privoxy=true
    local ignore_errors=false
    local scenarios_with_errors=""

    while [ -n "$1" ];
    do
        case "$1" in
            "-c")
                echo "Continuing in case of test failures."
                ignore_errors=true
                shift
                ;;
            "-r")
                echo "Not starting privoxy."
                start_privoxy=false
                shift
                ;;
            "-t")
                shift
                test_scenarios="$1"
                shift
                ;;
            "-v")
                echo "Running Privoxy through valgrind."
                USE_VALGRIND=true
                shift
                ;;
            "--")
                shift
                echo "Passing remaining arguments to runtests-wrapper.sh: ${@}"
                break
                ;;
            *)
                echo "Invalid argument: $1"
                echo "Supported arguments: -c -r -t test-scenario -- Arguments for runtests-wrapper.sh"
                exit 1
                ;;
        esac
    done

    if [ -z "${test_scenarios}" ]; then
        test_scenarios="$(get_test_scenarios)"
    fi

    for test_scenario in ${test_scenarios}; do
        if [ "${test_scenario}" = "${UPSTREAM_TEST_SCENARIO}" ]; then
            run_upstream_tests ${start_privoxy} "$@"
        else
            run_privoxy_tests ${start_privoxy} "${test_scenario}" "$@"
        fi
        if [ $? != 0 ]; then
            scenarios_with_errors="${scenarios_with_errors} ${test_scenario}"
            ${ignore_errors} || exit 1
        fi
    done

    if [ -n "${scenarios_with_errors}" ]; then
       echo "The following test scenarios had at least one error:${scenarios_with_errors}"
       exit 1
    fi

    echo "All test scenarios finished without errors."

    exit 0
}

main "$@"
