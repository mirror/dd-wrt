#!/bin/sh

# Make this script exit with error if any command fails
set -e

# Make sure an argument is provided to this script
if [ $# -ne 1 ]
then
	echo "Usage: $0 build-distro-name"
	echo "'build-distro-name' is the Linux distribution name your are building radvd on."
	echo "Currently supported distributions are:"
	echo "  alpine"
	echo "  debian"
	echo "  fedora"
	echo "  opensuse"
	echo "  ubuntu"
fi

# Install the right dependencies according to build host
echo "Installing required dependencies for distribution '${1}'..."
case $1 in
	# watch out, the naming of libbsd vs libbsd0 is not consistent between distros!
	alpine)
		apk update
		apk add alpine-sdk autoconf automake bison check-dev clang flex gettext libtool linux-headers sudo xz libbsd libbsd-dev
		;;
	debian)
		apt update
		apt install -y autoconf automake bison build-essential check clang flex gettext libtool pkg-config sudo libbsd-dev libbsd0
		;;
	fedora)
		sudo dnf install -y autoconf automake bison check-devel clang flex gettext libtool make pkgconfig xz libbsd-devel libbsd
		;;
	opensuse)
		zypper refresh
		zypper --non-interactive install -t pattern devel_C_C++
		zypper --non-interactive install check-devel clang sudo libbsd0 libbsd-devel
		;;
	ubuntu)
		sudo apt update
		sudo apt install -y autoconf automake bison build-essential check clang flex gettext libtool pkg-config libbsd-dev libbsd0
		;;
	*)
		echo "Unsupported distribution, build will be tried without installing any dependencies."
		;;
esac

# Build radvd
./autogen.sh
./configure --with-check
make -j
make check
make dist-xz
sudo make install
