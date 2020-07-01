
## exfatprogs
As new exfat filesystem is merged into linux-5.7 kernel, exfatprogs is
created as an official userspace utilities that contain all of the standard
utilities for creating and fixing and debugging exfat filesystem in linux
system. The goal of exfatprogs is to provide high performance and quality
at the level of exfat utilities in windows. And this software is licensed
under the GNU General Public License Version 2.

## Building exfatprogs
Install prerequisite packages:
```
For Ubuntu:
    sudo apt-get install autoconf libtool pkg-config

For Fedora, RHEL:
    sudo yum install autoconf automake libtool
```

Build steps:
```
    cd into the exfatprogs directory:
    ./autogen.sh
    ./configure
    make
    make install
```

## Using exfatprogs
```
- mkfs.exfat:
    Build a exfat filesystem on a device or partition(e.g. /dev/hda1, dev/sda1).

Usage example:
    1. No option(default) : cluster size adjustment as per device size, quick format.
        mkfs.exfat /dev/sda1
    2. To change cluster size(KB or MB or Byte) user want
        mkfs.exfat -c 1048576 /dev/sda1
        mkfs.exfat -c 1024K /dev/sda1
        mkfs.exfat -c 1M /dev/sda1
    3. For full format(zero out)
        mkfs.exfat -f /dev/sda1
    4. For set volume label, use -l option with string user want.
        mkfs.exfat -L "my usb" /dev/sda1

- fsck.exfat:
    Check the consistency of your exfat filesystem and optionally repair a corrupted device formatted by exfat.

Usage example:
    1. check the consistency.
        fsck.exfat /dev/sda1
    2. repair and fix.(preparing)

- tune.exfat:
    Adjust tunable filesystem parameters on an exFAT filesystem

Usage example:
    1. print current volume label.
        tune.exfat -l /dev/sda1
    2. set new volume label.
        tune.exfat -L "new label" /dev/sda1
```

## Sending feedback
If you have any issues, please create [issues][1] or contact to [Namjae Jeon](mailto:linkinjeon@kernel.org) and
[Hyunchul Lee](mailto:hyc.lee@gmail.com).
[Contributions][2] are also welcome.

[1]: https://github.com/exfatprogs/exfatprogs/issues
[2]: https://github.com/exfatprogs/exfatprogs/pulls
