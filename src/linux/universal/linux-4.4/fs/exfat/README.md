# exfat-linux

This __exFAT filesystem module for Linux kernel__ is based on `sdFAT` drivers by Samsung, which is used with their smartphone lineups.

The main objective of **exfat-linux** is to provide the best generic kernel drivers for exFAT. That means Samsung-specific modifications such as fat12/16/32 handlings, defrag and etc has been removed to make the code portable.

This project can be used for everyday Linux users by simply doing `make && make install`. Ubuntu users can simply add a PPA and start using it, without even downloading the code. This can also be directly dropped-in to an existing Linux kernel source for building the filesystem drivers inline, which should be useful for Android kernel developers.

---------------------------------------

[exfat-nofuse]'s development has been stale for more than a year with no clear maintainership. It's also been lacking critical upstream(*which is Samsung*) changes.

[exfat-nofuse]: https://github.com/dorimanx/exfat-nofuse

**exfat-linux** is:

 * Based on a totally different and newer base provided by Samsung
 * Intended to keep upstream changes constantly merged
 * Intended to fix breakage from newer kernels as-soon-as-possible

**exfat-linux** has been tested with all major LTS kernels ranging from 3.4 to 4.19 and the ones Canonical uses for Ubuntu: `3.4`, `3.10`, `3.18`, `4.1`, `4.4`, `4.9`, `4.14`, `4.19` and `4.15`, `5.0`, `5.2`, and `5.3-rc`.

It's also been tested with `x86(i386)`, `x86_64(amd64)`, `arm32(AArch32)` and `arm64(AArch64)`.

## Disclaimer

#### ● Original authorship and copyright: Samsung

#### ● Maintainer of exfat-linux: Park Ju Hyung([arter97](https://twitter.com/arter97))

## Using exfat-linux

### ● Ubuntu PPA

If you're an Ubuntu user, you can simply add a [PPA repository](https://launchpad.net/~arter97/+archive/ubuntu/exfat-linux) and start using the exFAT module.

Ubuntu will handle upgrades automatically as well.

1. Add the exfat-linux repository

   ```
   sudo add-apt-repository ppa:arter97/exfat-linux
   sudo apt update
   ```

2. Install the module

   `sudo apt install exfat-dkms`

This will use DKMS(Dynamic Kernel Module Support) and automatically build exFAT module for your current Ubuntu installation.

### ● Manually installing the module

1. Download the code

   ```
   git clone https://github.com/arter97/exfat-linux
   cd exfat-linux
   ```

2. Build

   `make`

3. Install

   `sudo make install`

This will install the module to your __currently running kernel__.

4. And finally load

   `sudo modprobe exfat`

If you upgrade the kernel, you'll have to repeat this process.

If you want to update **exfat-linux** to the latest version, you'll have to repeat this process.

### ● Merging the drivers to existing Linux kernel source

If you're using `git`, using `git subtree` or `git submodule` is highly recommended.

1. Add this repository to `fs/exfat`

2. Modify `fs/Kconfig`

```
 menu "DOS/FAT/NT Filesystems"

 source "fs/fat/Kconfig"
+source "fs/exfat/Kconfig"
 source "fs/ntfs/Kconfig"
 endmenu
```

3. Modify `fs/Makefile`

```
 obj-$(CONFIG_FAT_FS)    += fat/
+obj-$(CONFIG_EXFAT_FS)  += exfat/
 obj-$(CONFIG_BFS_FS)    += bfs/
```

And you're good to go!

## Benchmarks

For reference, existing exFAT implementations were tested and compared on a server running Ubuntu 16.04 with Linux kernel 4.14 under a contained virtual machine.

Linux 4.14 was used as higher LTS kernels don't work with [exfat-nofuse] at the time of testing.

### ● Ramdisk

#### fio sequential I/O

| Implementation   | Base   | Read         | Write        |
| ---------------  | ------ | ------------ | ------------ |
| **exfat-linux**  | 2.2.0  |    7042 MB/s |    2173 MB/s |
| [exfat-nofuse]   | 1.2.9  |    6849 MB/s |    1961 MB/s |
| [exfat-fuse]     | N/A    |    3097 MB/s |    1710 MB/s |
| ext4             | N/A    |    7352 MB/s |    3333 MB/s |

#### fio random I/O

| Implementation   | Base   | Read         | Write        |
| ---------------  | ------ | ------------ | ------------ |
| **exfat-linux**  | 2.2.0  |     760 MB/s |    2222 MB/s |
| [exfat-nofuse]   | 1.2.9  |     760 MB/s |    2160 MB/s |
| [exfat-fuse]     | N/A    |     1.7 MB/s |     1.6 MB/s |
| ext4             | N/A    |     747 MB/s |    2816 MB/s |

### ● NVMe device

#### fio sequential I/O

| Implementation   | Base   | Read         | Write        |
| ---------------  | ------ | ------------ | ------------ |
| **exfat-linux**  | 2.2.0  |    1283 MB/s |    1832 MB/s |
| [exfat-nofuse]   | 1.2.9  |    1285 MB/s |    1678 MB/s |
| [exfat-fuse]     | N/A    |     751 MB/s |    1464 MB/s |
| ext4             | N/A    |    1283 MB/s |    3356 MB/s |

#### fio random I/O

| Implementation   | Base   | Read         | Write        |
| ---------------  | ------ | ------------ | ------------ |
| **exfat-linux**  | 2.2.0  |      26 MB/s |    1885 MB/s |
| [exfat-nofuse]   | 1.2.9  |      24 MB/s |    1827 MB/s |
| [exfat-fuse]     | N/A    |     1.6 MB/s |     1.6 MB/s |
| ext4             | N/A    |      29 MB/s |    2821 MB/s |

[exfat-fuse]: https://github.com/relan/exfat

## Mount options

* uid
* gid
* umask
* dmask
* fmask
* allow_utime
* codepage
* iocharset
* quiet
* utf8
* tz

  * Please refer to the [vfat](https://github.com/torvalds/linux/blob/master/Documentation/filesystems/vfat.txt)'s documentation.

* namecase

  * Passing `namecase=1` as a mount option will make exFAT operate in a case-sensitive mode.

  * Default is insensitive mode.

* symlink

  * Allow a symlink to be created under exFAT.

* errors=continue

  * Keep going on a filesystem error.

* errors=panic

  * Panic and halt the machine if an error occurs.

* errors=remount-ro

  * Remount the filesystem read-only on an error.

* discard

  * Enable the use of discard/TRIM commands to ensure flash storage doesn't run out of free blocks. This option may introduce latency penalty on file removal operations.

* delayed_meta

  * Delay flushing metadata, hence improving performance.

  * This is enabled by default, please pass `nodelayed_meta` to disable it.

## Enjoy!
