#!/usr/bin/env bash

scriptdir="$(dirname "$0")"
export NOAUTODEPS=1
export SNMP_VERBOSE=1
case "$(uname -a)" in
    MSYS*|MINGW*)
	pacman --noconfirm --remove mingw-w64-x86_64-gcc-ada
	pacman --noconfirm --remove mingw-w64-x86_64-gcc-fortran
	pacman --noconfirm --remove mingw-w64-x86_64-gcc-libgfortran
	pacman --noconfirm --remove mingw-w64-x86_64-gcc-objc
	pacman --noconfirm --sync --refresh
	pacman --noconfirm --sync --needed diffutils
	pacman --noconfirm --sync --needed make
	pacman --noconfirm --sync --needed perl-ExtUtils-MakeMaker
	pacman --noconfirm --sync --needed perl-Test-Harness
	;;
esac
case "$(uname -a)" in
    MSYS*x86_64*)
	pacman --noconfirm --sync --needed openssl-devel
	pacman --noconfirm --sync --needed pkg-config
	;;
    MINGW64*)
	pacman --noconfirm --sync --needed mingw-w64-x86_64-gcc
	pacman --noconfirm --sync --needed mingw-w64-x86_64-libmariadbclient
	pacman --noconfirm --sync --needed mingw-w64-x86_64-openssl
	pacman --noconfirm --sync --needed mingw-w64-x86_64-pkgconf ||
	    pacman --noconfirm --sync --needed mingw-w64-x86_64-pkg-config
	export PATH="/mingw64/bin:$PATH"
	;;
esac
case "$MODE" in
    Android)
	NDK=$PWD/android-ndk-r27d/toolchains/llvm/prebuilt/linux-x86_64/bin
	export PATH="${NDK}:${PATH}"
	export CC=aarch64-linux-android34-clang
	;;
esac
echo "compiler path: $(type -p "${CC:-gcc}")"
branch_name=$(git rev-parse --abbrev-ref HEAD)
if ! "${scriptdir}"/net-snmp-configure "${branch_name}"; then
    echo "========================================"
    echo "Configure failed. Dumping config.log:"
    echo "========================================"
    cat config.log
    exit 1
fi
case "$MODE" in
    mini*)
	# Net-SNMP uses static dependencies, the Makefile.depend files have
	# been generated for MODE=regular, net-snmp-features.h includes
	# <net-snmp/library/features.h> in minimalist mode and that file is
	# generated dynamically and is not in Makefile.depend. Hence disable
	# parallel compilation for minimalist mode.
	nproc=1;;
    *)
	if type nproc >/dev/null 2>&1; then
	    nproc=$(nproc)
	else
	    nproc=1
	fi;;
esac
make -s -j"${nproc}" || exit $?
case "$MODE" in
    regular)
	if [ -e testing/fuzzing ]; then
	    make -C testing -s fuzz-tests || exit $?
	fi
	;;
esac
