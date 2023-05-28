Stable release packages for openNDS on OpenWrt can be found at:

https://downloads.openwrt.org/snapshots/packages/

Here you can select the platform for the SoC/Hardware of your router and download the required openNDS package for installation.

Alternatively, you can compile openNDS yourself, or include into a custom OpenWrt image for your router using a combination of the OpenWrt development tools such as the Imagebuilder or SDK. You can also compile the entirety of OpenWrt if you wish.

To include openNDS into your OpenWRT image or to create an .ipk
package (similar to Debians .deb files), you have to build an OpenWRT image.
To build the firmware you need a Unix console to enter commands into.

Install the dependencies of the build environment:

**Debian/Ubuntu**
```
sudo apt-get install git subversion g++ libncurses5-dev gawk zlib1g-dev build-essential
```
**OpenSuse**
```
sudo zypper install -t pattern devel_basis
```

Compile everything yourself - Build Commands:
```
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
```

At this point select the appropriate "Target System" and "Target Profile"
depending on what target chipset/router you want to build for.
Now select the openNDS package in "Network ---> Captive Portals".

Now compile/build everything:

```
make
```

The images and all ipk packages are now inside the bin/ folder.
You can install the openNDS .ipk using `opkg install <ipkg-file>` on the router or just use the whole image.

For details please check the OpenWRT documentation.

### Note for developers

## Build Notes

You might want to use your own source location and not the remote repository.
To do this you need to checkout the repository yourself and commit your changes locally:

```
git clone git://github.com/opennds/opennds.git
cd opennds
... apply your changes
git commit -am "my change"
```

Now create a symbolic link in the openNDS package folder using the abolute path:

```
ln -s /my/own/project/folder/opennds/.git openwrt/package/opennds/git-src
```

Also make sure to enable

```
"Advanced configuration options" => "Enable package source tree override"
```

in the menu when you do `make menuconfig`.
