#!/usr/bin/env bash

# Copyright © 2015-2016 Collabora Ltd.
# Copyright © 2020 Ralf Habacker <ralf.habacker@freenet.de>
# SPDX-License-Identifier: MIT
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation files
# (the "Software"), to deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge,
# publish, distribute, sublicense, and/or sell copies of the Software,
# and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
# BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

set -euo pipefail
set -x

NULL=

if [ "$(uname -s || true)" = Linux ]; then
    export LANG=C.UTF-8
    export LANGUAGE=C.UTF-8
    export LC_ALL=C.UTF-8
fi

##
## initialize support to run cross compiled executables
##
# syntax: init_wine <path1> [<path2> ... [<pathn>]]
# @param  path1..n  pathes for adding to wine executable search path
#
# The function exits the shell script in case of errors
#
init_wine() {
    if ! command -v wineboot >/dev/null; then
        echo "wineboot not found"
        exit 1
    fi

    # run without X11 display to avoid that wineboot shows dialogs
    wineboot -fi

    # add local paths to wine user path
    local addpath="" d="" i
    for i in "$@"; do
        local wb=$(winepath -w "$i")
        addpath="$addpath$d$wb"
        d=";"
    done

    # create registry file from template
    local wineaddpath=$(echo "$addpath" | sed 's,\\,\\\\\\\\,g')
    sed "s,@PATH@,$wineaddpath,g" ../tools/user-path.reg.in > user-path.reg

    # add path to registry
    wine regedit /C user-path.reg

    # check if path(s) has been set and break if not
    local o=$(wine cmd /C "echo %PATH%")
    case "$o" in
        (*z:* | *Z:*)
            # OK
            ;;
        (*)
            echo "Failed to add Unix paths '$*' to path: Wine %PATH% = $o" >&2
            exit 1
            ;;
    esac
}

# ci_buildsys:
# Build system under test: meson or cmake
: "${ci_buildsys:=meson}"

# ci_compiler:
# Compiler used to build dbus: gcc or clang
: "${ci_compiler:=gcc}"

# ci_distro:
# OS distribution in which we are testing
# Typical values: auto (detect at runtime), ubuntu, debian; maybe fedora in future
: "${ci_distro:=auto}"

# ci_host:
# See ci-install.sh
: "${ci_host:=native}"

# ci_local_packages:
# prefer local packages instead of distribution
# See ci-install.sh
: "${ci_local_packages:=yes}"

# ci_parallel:
# A number of parallel jobs, passed to make -j
: "${ci_parallel:=1}"

# ci_sudo:
# If yes, assume we can get root using sudo; if no, only use current user
: "${ci_sudo:=no}"

# ci_suite:
# OS suite (release, branch) in which we are testing.
# Typical values: auto (detect at runtime), ci_distro=debian: bullseye, buster, ci_distro=fedora: 35, rawhide
: "${ci_suite:=auto}"

# ci_test:
# If yes, run tests; if no, just build
: "${ci_test:=yes}"

# ci_cmake_junit_output:
# If non-empty, emit JUnit XML output from CTest tests to that file
# Note: requires CMake 3.21 or newer.
: "${ci_cmake_junit_output:=}"

# ci_test_fatal:
# If yes, test failures break the build; if no, they are reported but ignored
: "${ci_test_fatal:=yes}"

# ci_variant:
# One of debug, reduced, legacy, production, production-no-upload-docs
: "${ci_variant:=production}"

# ci_runtime:
# One of static, shared; used for windows cross builds
: "${ci_runtime:=static}"

# print used command line
set +x; env | awk 'BEGIN { s = "" } $1 ~ /^ci_/ { s=s " " $0} END { print s " " SCRIPT }' SCRIPT=$0; set -x

# choose distribution
if [ "$ci_distro" = "auto" ]; then
    ci_distro=$(. /etc/os-release; echo ${ID} | sed 's, ,_,g')
    echo "detected ci_distro as '${ci_distro}'"
fi

# choose suite
if [ "$ci_suite" = "auto" ]; then
    ci_suite=$(. /etc/os-release; if test -v VERSION_CODENAME; then echo ${VERSION_CODENAME}; else echo ${VERSION_ID}; fi)
    echo "detected ci_suite as '${ci_suite}'"
fi

maybe_fail_tests () {
    if [ "$ci_test_fatal" = yes ]; then
        exit 1
    fi
}

srcdir="$(pwd)"

# setup default ci_builddir, if not present
if [ -z "$ci_builddir" ]; then
  ci_builddir=${srcdir}/ci-build-${ci_variant}-${ci_host}
fi
# clean up directories from possible previous builds
rm -rf "$ci_builddir"
# create build directory
mkdir -p "$ci_builddir"
# use absolute path
ci_builddir="$(realpath "$ci_builddir")"

#
# cross compile setup
#
case "$ci_host" in
    (*-w64-mingw32)
        if [ "$ci_local_packages" = yes ]; then
            dep_prefix=$(pwd)/${ci_host}-prefix
        else
            # assume the compiler was configured with a sysroot (e.g. openSUSE)
            sysroot=$("${ci_host}-gcc" --print-sysroot)
            # check if the prefix is a subdir of sysroot (e.g. openSUSE)
            if [ -d "${sysroot}/${ci_host}" ]; then
                dep_prefix="${sysroot}/${ci_host}"
            else
                # fallback: assume the dependency libraries were built with --prefix=/${ci_host}
                dep_prefix="/${ci_host}"
                export PKG_CONFIG_SYSROOT_DIR="${sysroot}"
            fi
        fi

        export PKG_CONFIG_LIBDIR="${dep_prefix}/lib/pkgconfig"
        export PKG_CONFIG_PATH=
        export PKG_CONFIG="pkg-config --define-variable=prefix=${dep_prefix}"
        unset CC
        unset CXX
        export TMPDIR=/tmp
        ;;
esac

cd "$ci_builddir"

case "$ci_host" in
    (*-w64-mingw32)
        # If we're dynamically linking libgcc, make sure Wine will find it
        if [ "$ci_test" = yes ]; then
            if [ "${ci_distro%%-*}" = opensuse ] && [ "${ci_host%%-*}" = x86_64 ]; then
                export WINEARCH=win64
            fi
            libgcc_path=
            if [ "$ci_runtime" = "shared" ]; then
                libgcc_path=$(dirname "$("${ci_host}-gcc" -print-libgcc-file-name)")
            fi
            init_wine \
                "${ci_builddir}/bin" \
                "${ci_builddir}/subprojects/expat-2.4.8" \
                "${ci_builddir}/subprojects/glib-2.72.2/gio" \
                "${ci_builddir}/subprojects/glib-2.72.2/glib" \
                "${ci_builddir}/subprojects/glib-2.72.2/gmodule" \
                "${ci_builddir}/subprojects/glib-2.72.2/gobject" \
                "${ci_builddir}/subprojects/glib-2.72.2/gthread" \
                "${dep_prefix}/bin" \
                ${libgcc_path:+"$libgcc_path"}
        fi
        ;;
esac

# Allow overriding make (e.g. on FreeBSD it has to be set to gmake)
: "${make:=make}"
export MAKE=${make}
make="${make} -j${ci_parallel} V=1 VERBOSE=1"

export UBSAN_OPTIONS=print_stacktrace=1:print_summary=1:halt_on_error=1

case "$ci_buildsys" in
    (cmake)
        cmdwrapper=
        cmake=cmake
        case "$ci_host" in
            (*-w64-mingw32)
                # CFLAGS and CXXFLAGS does do work, checked with cmake 3.15
                export LDFLAGS="-${ci_runtime}-libgcc"
                if [ "${ci_distro%%-*}" = opensuse ]; then
                    if [ "${ci_host%%-*}" = x86_64 ]; then
                        cmake=mingw64-cmake
                    else
                        cmake=mingw32-cmake
                    fi
                    cmdwrapper="xvfb-run -a"
                fi
                set _ "$@"
                if [ "$ci_distro" != "opensuse" ]; then
                    set "$@" -D CMAKE_TOOLCHAIN_FILE="${srcdir}/cmake/${ci_host}.cmake"
                fi
                set "$@" -D CMAKE_PREFIX_PATH="${dep_prefix}"
                if [ "$ci_local_packages" = yes ]; then
                    set "$@" -D CMAKE_INCLUDE_PATH="${dep_prefix}/include"
                    set "$@" -D CMAKE_LIBRARY_PATH="${dep_prefix}/lib"
                    set "$@" -D EXPAT_LIBRARY="${dep_prefix}/lib/libexpat.dll.a"
                    set "$@" -D GLIB2_LIBRARIES="${dep_prefix}/lib/libglib-2.0.dll.a ${dep_prefix}/lib/libgobject-2.0.dll.a ${dep_prefix}/lib/libgio-2.0.dll.a"
                fi
                if [ "$ci_test" = yes ]; then
                    set "$@" -D DBUS_USE_WINE=1
                    # test-dbus-daemon needs more time on Windows
                    export DBUS_TEST_TIMEOUT_MULTIPLIER=2
                fi
                shift
                ;;
        esac

        set -- "$@" -D DBUS_BUILD_TESTS=ON

        case "$ci_variant" in
            (debug)
                set -- "$@" -D DBUS_ENABLE_INTRUSIVE_TESTS=ON
                ;;
        esac

        $cmake -DCMAKE_VERBOSE_MAKEFILE=ON -DENABLE_WERROR=ON -S "$srcdir" -B "$ci_builddir" "$@"

        ${make}
        # The test coverage for OOM-safety is too verbose to be useful on
        # travis-ci.
        export DBUS_TEST_MALLOC_FAILURES=0
        ctest_args="-VV --timeout 180"
        if [ -n "$ci_cmake_junit_output" ]; then
            ctest_args="--output-junit $ci_cmake_junit_output $ctest_args"
        fi

        [ "$ci_test" = no ] || $cmdwrapper ctest $ctest_args || maybe_fail_tests
        ${make} install DESTDIR=$(pwd)/DESTDIR
        ( cd DESTDIR && find . -ls)
        ;;

    (meson)
        # The test coverage for OOM-safety is too verbose to be useful on
        # travis-ci, and too slow when running under wine.
        export DBUS_TEST_MALLOC_FAILURES=0

        meson_setup=
        cross_files=()

        # openSUSE has convenience wrappers that run Meson with appropriate
        # cross options
        case "$ci_host" in
            (i686-w64-mingw32)
                meson_setup=mingw32-meson
                ;;
            (x86_64-w64-mingw32)
                meson_setup=mingw64-meson
                ;;
        esac

        case "$ci_host" in
            (*-w64-mingw32)
                cross_files=("${cross_files[@]}" "${srcdir}/maint/${ci_host}.txt")

                if [ "$ci_test" = yes ]; then
                    cross_files=("${cross_files[@]}" "${srcdir}/maint/wine-exe-wrapper.txt")
                fi

                # openSUSE's wrappers are designed for building predictable
                # RPM packages, so they set --auto-features=enabled -
                # but that includes some things that make no sense on
                # Windows.
                set -- -Dapparmor=disabled "$@"
                set -- -Depoll=disabled "$@"
                set -- -Dinotify=disabled "$@"
                set -- -Dkqueue=disabled "$@"
                set -- -Dlaunchd=disabled "$@"
                set -- -Dlibaudit=disabled "$@"
                set -- -Dselinux=disabled "$@"
                set -- -Dsystemd=disabled "$@"
                set -- -Dx11_autolaunch=disabled "$@"
                # We seem to have trouble finding libexpat.dll when
                # cross-building for Windows and running tests with Wine.
                set -- -Dexpat:default_library=static "$@"
                ;;
        esac

        case "$ci_distro" in
            (debian*|ubuntu*)
                # We know how to install python3-mallard-ducktype
                ;;
            (*)
                # TODO: We don't know the openSUSE equivalent of
                # python3-mallard-ducktype
                set -- -Dducktype_docs=disabled "$@"
                ;;
        esac

        set -- -Dmodular_tests=enabled "$@"
        # By default, the Meson build would install these into
        # /lib/systemd, overwriting any systemd units that might have
        # come from the container's base OS. Install into our prefix instead,
        # keeping the CI installation separate from the container's base OS
        # while still allowing systemd to see the units. (dbus#470)
        set -- -Dsystemd_system_unitdir=/usr/local/lib/systemd/system "$@"
        set -- -Dsystemd_user_unitdir=/usr/local/lib/systemd/user "$@"

        case "$ci_variant" in
            (debug)
                set -- -Dasserts=true "$@"
                set -- -Dintrusive_tests=true "$@"
                set -- -Dverbose_mode=true "$@"

                case "$ci_host" in
                    (*-w64-mingw32)
                        ;;
                    (*)
                        set -- -Db_sanitize=address,undefined "$@"

                        # https://github.com/mesonbuild/meson/issues/764
                        if [ "$ci_compiler" = "clang" ]; then
                            set -- -Db_lundef=false "$@"
                        fi

                        set -- -Db_pie=true "$@"
                        set -- -Duser_session=true "$@"
                        ;;
                esac

                shift
                ;;
          (reduced)
                # A smaller configuration than normal, with
                # various features disabled; this emulates
                # an older system or one that does not have
                # all the optional libraries.
                set _ "$@"
                # No LSMs (the production build has both)
                set "$@" -Dselinux=disabled -Dapparmor=disabled
                # No inotify (we will use dnotify)
                set "$@" -Dinotify=disabled
                # No epoll or kqueue (we will use poll)
                set "$@" -Depoll=disabled -Dkqueue=disabled
                # No special init system support
                set "$@" -Dlaunchd=disabled -Dsystemd=disabled
                # No libaudit or valgrind
                set "$@" -Dlibaudit=disabled -Dvalgrind=disabled
                # Disable optional features, some of which are on by
                # default
                set "$@" -Dstats=false
                set "$@" -Duser_session=false
                shift
                ;;

            (legacy)
                # An unrealistically cut-down configuration,
                # to check that it compiles and works.
                set _ "$@"
                # No epoll, kqueue or poll (we will fall back
                # to select, even on Unix where we would
                # usually at least have poll)
                set "$@" -Depoll=disabled -Dkqueue=disabled
                export CPPFLAGS=-DBROKEN_POLL=1
                # Enable SELinux and AppArmor but not
                # libaudit - that configuration has sometimes
                # failed
                set "$@" -Dselinux=enabled -Dapparmor=enabled
                set "$@" -Dlibaudit=disabled -Dvalgrind=disabled
                # No directory monitoring at all
                set "$@" -Dinotify=disabled
                # No special init system support
                set "$@" -Dlaunchd=disabled -Dsystemd=disabled
                # No X11 autolaunching
                set "$@" -Dx11_autolaunch=disabled
                # Leave stats, user-session, etc. at default settings
                # to check that the defaults can compile on an old OS
                shift
                ;;
        esac

        case "$ci_compiler" in
            (clang)
                export CC=clang
                ;;
            (*)
                ;;
        esac

        # Debian doesn't have similar convenience wrappers, but we can use
        # a cross-file
        if [ -z "$meson_setup" ] || ! command -v "$meson_setup" >/dev/null; then
            meson_setup="meson setup"

            for cross_file in "${cross_files[@]}"; do
                set -- --cross-file="$cross_file" "$@"
            done
        fi

        # We assume this when we set LD_LIBRARY_PATH for as-installed
        # testing, below
        set -- "$@" --libdir=lib

        # openSUSE's mingw*-meson wrappers are designed for self-contained
        # package building, so they include --wrap-mode=nodownload. Switch
        # the wrap mode back, so we can use wraps.
        set -- "$@" --wrap=default

        $meson_setup "$@" "$srcdir"
        meson compile -v

        # This is too slow and verbose to keep enabled at the moment
        export DBUS_TEST_MALLOC_FAILURES=0

        [ "$ci_test" = no ] || meson test --print-errorlogs
        DESTDIR=DESTDIR meson install
        ( cd DESTDIR && find . -ls)

        if [ "$ci_sudo" = yes ] && [ "$ci_test" = yes ] && [ "$ci_host" = native ]; then
            sudo meson install
        fi
        ;;
esac

case "$ci_buildsys" in
    (meson*)
        if [ "$ci_sudo" = yes ] && [ "$ci_test" = yes ] && [ "$ci_host" = native ]; then
            sudo env LD_LIBRARY_PATH=/usr/local/lib \
                /usr/local/bin/dbus-uuidgen --ensure

            # Run "as-installed" tests with gnome-desktop-testing.
            # Also, one test needs a finite fd limit to be useful, so we
            # can set that here.
            env LD_LIBRARY_PATH=/usr/local/lib \
            bash -c 'ulimit -S -n 1024; ulimit -H -n 4096; exec "$@"' bash \
            gnome-desktop-testing-runner -d /usr/local/share dbus/ || \
                maybe_fail_tests

            # Some tests benefit from being re-run as non-root, if we were
            # not already...
            if [ "$(id -u)" = 0 ] && [ "$ci_in_docker" = yes ]; then
                sudo -u user \
                env LD_LIBRARY_PATH=/usr/local/lib \
                gnome-desktop-testing-runner -d /usr/local/share \
                    dbus/test-dbus-daemon_with_config.test \
                    || maybe_fail_tests
            fi

            # ... while other tests benefit from being re-run as root, if
            # we were not already
            if [ "$(id -u)" != 0 ]; then
                sudo env LD_LIBRARY_PATH=/usr/local/lib \
                bash -c 'ulimit -S -n 1024; ulimit -H -n 4096; exec "$@"' bash \
                    gnome-desktop-testing-runner -d /usr/local/share \
                    dbus/test-dbus-daemon_with_config.test \
                    dbus/test-uid-permissions_with_config.test || \
                    maybe_fail_tests
            fi
        fi
        ;;
esac

# vim:set sw=4 sts=4 et:
