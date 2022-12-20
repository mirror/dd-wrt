
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
    5. To change boundary alignment(KB or MB or Byte) user want
        mkfs.exfat -b 16777216 /dev/sda1
        mkfs.exfat -b 16384K /dev/sda1
        mkfs.exfat -b 16M /dev/sda1

- fsck.exfat:
    Check the consistency of your exfat filesystem and optionally repair a corrupted device formatted by exfat.

Usage example:
    1. check the consistency.
        fsck.exfat /dev/sda1
    2. repair a corrupted device and create files in /LOST+FOUND, which have clusters allocated but not belonged to any files when reparing the device.
        fsck.exfat -p -s /dev/sda1
    3. repair a corrupted device in the same way above, but answering yes to all questions.
        fsck.exfat -y -s /dev/sda1

- tune.exfat:
    Adjust tunable filesystem parameters on an exFAT filesystem

Usage example:
    1. print current volume label.
        tune.exfat -l /dev/sda1
    2. set new volume label.
        tune.exfat -L "new label" /dev/sda1
    3. print current volume serial.
        tune.exfat -i /dev/sda1
    4. set new volume serial.
        tune.exfat -I 0x12345678 /dev/sda1

- exfatlabel:
    Get or Set volume label or serial

Usage example:
    1. get current volume label.
        exfatlabel /dev/sda1
    2. set new volume label.
        exfatlabel /dev/sda1 "new label"
    3. get current volume serial.
        exfatlabel -i /dev/sda1
    4. set new volume serial.
        exfatlabel -i /dev/sda1 0x12345678

- dump.exfat:
    Show on-disk information

Usage example:
    dump.exfat /dev/sda1

- exfat2img:
    Dump metadata of an exFAT filesystem

Usage example:
    exfat2img -o sda1.dump /dev/sda1

```

## Benchmarks

Some fsck implementations were tested and compared for Samsung 64GB Pro
microSDXC UHS-I Class 10 which was filled up to 35GB with 9948 directories
and 16506 files by fsstress.

The difference in the execution time for each testing is very small.


| Implementation       | version         | execution time (seconds) |
|----------------------|-----------------|--------------------------|
| **exfatprogs fsck**  | 1.0.4           | 11.561                   |
| Windows fsck         | Windows 10 1809 | 11.449                   |
| [exfat-fuse fsck]    | 1.3.0           | 68.977                   |

[exfat-fuse fsck]: https://github.com/relan/exfat

## Sending feedback
If you have any issues, please create [issues][1] or contact to [Namjae Jeon](mailto:linkinjeon@kernel.org) and
[Hyunchul Lee](mailto:hyc.lee@gmail.com).
[Contributions][2] are also welcome.

[1]: https://github.com/exfatprogs/exfatprogs/issues
[2]: https://github.com/exfatprogs/exfatprogs/pulls

## Contributor information
* Please base your pull requests on the `exfat-next` branch.
* Make sure you add 'Signed-Off' information to your commits (e.g. `git commit --signoff`).
* Please check your code contribution using kernel dev-tool script [checkpatch](https://docs.kernel.org/dev-tools/checkpatch.html).
