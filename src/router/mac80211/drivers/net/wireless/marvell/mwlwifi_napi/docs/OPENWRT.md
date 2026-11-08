# Openwrt
## Instructions for compiling the module:

* download sdk https://archive.openwrt.org/releases/22.03.2/targets/mvebu/cortexa9/openwrt-sdk-22.03.2-mvebu-cortexa9_gcc-11.2._musl_eabi.Linux-x86_64.tar.xz

/!\ Add to env if you compile as root user
```
export FORCE_UNSAFE_CONFIGURE="1"
```

update and install feeds
```
./scripts/feeds update -a
./scripts/feeds install -a
make menuconfig
```

#### remove:
* Select all target specific packages by default
* Select all kernel module packages by default
* Select all userspace packages by default
* Advanced configuration options (for developers)
1. Automatic rebuild of packages
2. Automatic removal of build directories

## Target a commit (sha1)
1. edit Makefile
```
nano ./feeds/base/package/kernel/mwlwifi/Makefile
```
2. drop patch
```
rm ./feeds/base/package/kernel/mwlwifi/patches -Rf
```
3. compile
```
make -j$(nproc) package/mwlwifi/compile
```
4. Find ipk here
```
ls -al bin/targets/mvebu/cortexa9/packages/kmod-mwlwifi_*
```