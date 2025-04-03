#!/bin/sh
################################################################################
#
# runtests-wrapper.sh
#
# Wrapper around curl's runtests.pl that sets a couple of options
# so Privoxy is being used.
#
# Copyright (c) 2013-2021 Fabian Keil <fk@fabiankeil.de>
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

curl_setup_is_sane() {
    local curl_source_directory="${1}"
    local curl_binary="${curl_source_directory}/src/curl"
    local runtests_pl="${curl_source_directory}/tests/runtests.pl"

    if [ ! -d "${curl_source_directory}" ]; then
        echo "Missing curl source directory at ${curl_source_directory}"
        return 1
    fi
    if [ ! -f "${curl_binary}" ]; then
        echo "Missing curl binary at ${curl_binary}. Did you compile curl?"
        return 1
    fi
    if [ ! -f "${runtests_pl}" ]; then
        echo "Did not find runtests.pl at ${runtests_pl}"
        return 1
    fi

    return 0
}

runtests_wrapper() {
    local extra_args \
        a_flag proxy_args exclude_file_args testdir_args \
        privoxy_lib privoxy_ip privoxy_source_directory curl_source_directory \
        keyword directory_name test_dir

    directory_name="$(dirname "$0")"
    test_dir="$(realpath "${directory_name}")"
    privoxy_source_directory="$(realpath "${test_dir}"/../..)"
    privoxy_lib="${privoxy_source_directory}/tests/cts/privoxy-runtests.pm"
    curl_source_directory=${CURL_SOURCE_DIRECTORY:-"$(realpath "${privoxy_source_directory}"/../curl)"}

    curl_setup_is_sane "${curl_source_directory}" || exit 1

    # Defaults that can be changed through arguments
    privoxy_ip=127.0.0.1
    privoxy_port=9119
    a_flag="-a"
    proxy_args="-P http://${privoxy_ip}:${privoxy_port}/ -o HOSTIP=${privoxy_ip}"
    exclude_file_args="-E ${privoxy_source_directory}/tests/cts/curl-test-manifest-for-privoxy"
    testdir_args="-o TESTDIR=${privoxy_source_directory}/tests/cts/data"
    keyword=HTTP

    while [ -n "$1" ];
    do
        case "$1" in
            "-A")
                a_flag=""
                shift
                ;;
            "-E")
                exclude_file_args=""
                shift
                ;;
            "-k")
                shift
                keyword="$1"
                shift
                ;;
            "-i")
                shift
                privoxy_ip="$1"
                shift
                proxy_args="-P http://${privoxy_ip}:${privoxy_port}/ -o HOSTIP=${privoxy_ip}"
                ;;
            "-T")
                echo "Not setting TESTDIR"
                testdir_args=""
                shift
                ;;
            "-t")
                shift
                echo "Overwriting default TESTDIR with $1"
                testdir_args="-o TESTDIR=$1"
                shift
                ;;
            "-p")
                shift
                privoxy_port="$1"
                shift
                proxy_args="-P http://${privoxy_ip}:${privoxy_port}/ -o HOSTIP=${privoxy_ip}"
                ;;
            "-P")
                # "Obviously" -P means not setting -P
                echo "Not setting '$proxy_args'"
                proxy_args=""
                shift
                ;;
            *)
                break;;
        esac
    done

    extra_args="$*"
    
    cd "${curl_source_directory}/tests" || exit 1
    ./runtests.pl -L "${privoxy_lib}" $proxy_args $exclude_file_args $testdir_args $a_flag -n $keyword !skip $extra_args
}

main() {
    runtests_wrapper "$@"
}

main "$@"
