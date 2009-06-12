
FAT Filesystem README
+++++++++++++++++++++        

This file serves as documentation for the FAT filesystem until full
documentation can be generated.

Configuring FAT
===============

The FAT filesystem support consists of several packages:

CYGPKG_FS_FAT
        The FAT filesystem itself.

CYGPKG_BLOCK_LIB
        Block library. This actually implements the block cache. It
        depends on the Linux compatibility library
        (CYGPKG_LINUX_COMPAT) for support.

CYGPKG_IO_DISK
        Disk device IO support. This provides the top level disk
        driver functions. It also interprets partition tables and
        provides a separate access channel for each partition.
        
CYGPKG_DEVS_DISK_ECOSYNTH
        This is the only low level device driver currently known to
        work. This driver treats a file in the Linux filesystem as a
        disk image.

CYGPKG_DEVS_DISK_V85X_EDB_V850
        This is a device driver for an IDE compatible Compact Flash
        device on a platform that is otherwise unsupported. It has not
        been tested.        

In addition the above new packages, FAT support also relies on the
following existing packages:

CYGPKG_IO_FILEIO
        The File IO package. This provides the API by which the FAT
        filesystem is accessed.
        
CYGPKG_IO
        Device IO package. This provides all the infrastructure for
        the disk devices.
        
CYGPKG_LIBC_STRING
        Strings library. This provides the string and memory move and
        compare routines used by the filesystem.
        
CYGPKG_MEMALLOC
        The FAT filesystem currently uses malloc() to allocate
        its memory resources, so this package is required.

The doc directory that contains this file contains a number of .ecm
files that will add the FAT filesystem to most configurations. These
should be used as a basis for creating new configurations.

Device Drivers
==============

The disk device interface is divided into a generic layer in the
CYGPKG_IO_DISK package and drivers for each device.

Apart from providing the main API, the other thing that the DISK
package does is to provide support for disk partitions. It interprets
the on-disk data structures and provides an independent channel to
access each partition independently. Partitions are referenced using
an additional element to the device name. If the device is named
"/dev/disk0" then "/dev/disk0/1" refers to partition 1, "/dev/disk0/2"
to partition 2 and so on. If the disk is not partitioned, for example
a floppy disk, then "/dev/disk0/0" refers to the whole disk.

There are two device drivers currently supported:

Linux Synthetic Driver
----------------------

This driver treats a disk file on Linux as a disk image and read or
writes blocks from/to it to simulate disk IO. At present it supports
just a single disk device image. There are a number of configuration
options for this device, see the CDL file for details, and the .ecm
files for examples.

EDB V850 Compact Flash Device
-----------------------------

This uses an IDE interface to a Compact Flash device. This is actually
for a platform that is not supported in the current source tree, so
cannot be tested. However, it should be possible to adapt the code for
other devices.

Testing
=======

There is currently just one test available for the FAT filesystem,
fileio1. This test will currently only run on the Linux synthetic
target and needs a special disk image.

To run the test you first need to configure the synthetic disk driver
to access the disk image. This is most easily done by importing the
fat16.ecm file. A complete build would look like this:

$ mkdir linux.fat16
$ cd linux.fat16
$ ecosconfig new linux
$ ecosconfig import $ECOS_REPOSITORY/fs/fat/current/doc/fat16.ecm
$ ecosconfig tree
$ make tests

To make the disk image, first ensure that there is at least 10MB of
space available in your /tmp directory. Add the following two lines to
your ~/.mtoolsrc file (you may need to create it):

drive z: file="/tmp/disk0.img" partition=1
drive y: file="/tmp/disk0.img" partition=2

Now run the mkdisk0 shell script from the fs/fat/doc directory:

$ source $ECOS_REPOSITORY/fs/fat/current/doc/mkdisk0
20000+0 records in
20000+0 records out
Warning: no active (bootable) partition present
$

Ignore the warning, we are not interested in booting this disk.

It should now be possible to run the fileio1 test program.
