How to Compile openNDS
######################

Linux/Unix - Compile in Place on Target Hardware
************************************************

Make sure the development suite for your Linux distribution is installed.

The libmicrohttpd library (MHD) is a dependency of openNDS so compiling and installing this is a prerequisite.

The nftables and iptables-nft packages are also dependencies, so should also be installed as prerequisites.

The "php cli" and "php openssl" packages are required for fas_secure levels 2 and 3 operation. These may be separate packages or one large package, depending on the Linux distribution. Not required for level 1 or Themespec operation.

**First**, create a working directory and "cd" into it.

**Next, Download and un-tar the libmicrohttpd source files.**

You can find a version number for MHD at https://ftp.gnu.org/gnu/libmicrohttpd/

The version number for MHD must not exceed 0.9.70 for versions of openNDS less than 6.0.0

.. code::

 wget https://ftp.gnu.org/gnu/libmicrohttpd/libmicrohttpd-0.9.71.tar.gz
 tar  -xf libmicrohttpd-0.9.71.tar.gz
 cd libmicrohttpd-0.9.71

where "0.9.71" is the MHD version number we are using in this example (use at least this version).

**Now configure and compile:**

.. code::

 ./configure --disable-https
 make
 sudo rm /usr/local/lib/libmicrohttpd*
 sudo make install
 sudo rm /etc/ld.so.cache
 sudo ldconfig -v
 cd ..


**Then proceed to download the opennds source files.**

You can find a release version number for openNDS at https://github.com/openNDS/openNDS/releases

.. code::

 wget https://codeload.github.com/opennds/opennds/tar.gz/v9.8.0
 tar -xf v9.8.0
 cd openNDS-9.8.0
 make
 sudo make install
 sudo systemctl enable opennds

Where "9.8.0" is the openNDS version we are using in this example.

openNDS should now start automatically at boot time.

It can be manually started, restarted, stopped or disabled with the following commands:

.. code::

 sudo systemctl start opennds

 sudo systemctl restart opennds

 sudo systemctl stop opennds

 sudo systemctl disable opennds

The status of openNDS can be checked with the following command:

.. code::

 sudo ndsctl status

On most Linux distributions you can read the last few entries for openNDS in the system message log with the command:

.. code::

 sudo systemctl status opennds

If openNDS fails to start, check for error messages with the command:

.. code::

 sudo journalctl -e

OpenWrt Package
***************
The OpenWrt package feed supports cross-compiled openNDS packages for all OpenWrt targets. See the "Installing openNDS" section of this documentation. The latest release of openNDS will be found in OpenWrt Snapshots, but will nevertheless be a stable release.

To include openNDS into your OpenWRT image or to create an .ipk
package (similar to Debian's .deb files), you can build an OpenWRT image.

You need a Unix console to enter commands into.

Install the dependencies of the build environment (eg on Debian/Ubuntu):

.. code::

 sudo apt-get install git subversion g++ libncurses5-dev gawk zlib1g-dev build-essential

Build Commands:

.. code::

 git clone https://git.openwrt.org/openwrt/openwrt.git
 cd openwrt

 ./scripts/feeds update -a
 ./scripts/feeds install -a
 ./scripts/feeds uninstall opennds

 git clone git://github.com/opennds/opennds.git
 cp -rf opennds/openwrt/opennds package/
 rm -rf opennds/

 make defconfig
 make menuconfig

At this point select the appropriate "Target System" and "Target Profile"
depending on what target chipset/router you want to build for.
Now select the openNDS package in "Network ---> Captive Portals".

Now compile/build everything:

.. code::

 make


The images and all ipk packages are now inside the bin/ folder.
You can install the openNDS .ipk using `opkg install <ipkg-file>` on the router or just use the whole image.

For details please check the OpenWRT documentation.

### Note for developers

## Build Notes

You might want to use your own source location and not the remote repository.
To do this you need to checkout the repository yourself and commit your changes locally:

.. code::

 git clone git://github.com/opennds/opennds.git
 cd opennds

... apply your changes

.. code::

 git commit -am "my change"

Now create a symbolic link in the openNDS package folder using the abolute path:


.. code::

 ln -s /my/own/project/folder/opennds/.git openwrt/package/opennds/git-src

Also make sure to enable

.. code::

 "Advanced configuration options" => "Enable package source tree override"

in the menu when you do `make menuconfig`.
