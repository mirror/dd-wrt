RedBoot for Intel XScale IXDP425, IXDPG425, GRG, and IXDP465 boards
April 25, 2005
README
========================================================================

This ReadMe contains instructions for running Redboot on the following
Intel XScale based boards:

  - IXDPG425

  - IXDP425

  - IXDP465

  - GRG

You will need the GNUPro xscale-elf toolchain which should be installed
as per the GNUPro documentation.

Overview
--------
These implementations of RedBoot support several configurations:

  * RedBoot running from the board's FLASH boot sector.

  * RedBoot booting from the board's FLASH boot sector, but copied
    into RAM for main execution.

  * RedBoot running from RAM with RedBoot in the FLASH boot sector.

In addition, these platforms support configurations for little-endian
or big-endian operation.

Installing Initial RedBoot Image
--------------------------------
Initial installations of RedBoot require the use of a flash utility or device
programmer as indicated by the board manufacturer. Please see appropriate
documentation for details on initial flash programming. A set of prebuilt files
are provided. This set corresponds to each of the supported configurations and
includes an ELF file (.elf), a binary image (.bin), and an S-record file (.srec).

* RedBoot on IXDP465

  - Running in big-endian mode from the FLASH boot sector:

  bin/ixdp465/redboot_ROM.bin
  bin/ixdp465/redboot_ROM.elf
  bin/ixdp465/redboot_ROM.srec

  - Running in big-endian mode copied from the FLASH boot sector
    to RAM:

  bin/ixdp465/redboot_ROMRAM.bin
  bin/ixdp465/redboot_ROMRAM.elf
  bin/ixdp465/redboot_ROMRAM.srec

  - Running in big-endian mode from RAM with big-endian RedBoot in the
    FLASH boot sector:

  bin/ixdp465/redboot_RAM.bin
  bin/ixdp465/redboot_RAM.elf
  bin/ixdp465/redboot_RAM.srec

  - Running in little-endian mode from the FLASH boot sector:

  bin/ixdp465/redboot_ROMLE.bin
  bin/ixdp465/redboot_ROMLE.elf
  bin/ixdp465/redboot_ROMLE.srec

  - Running in little-endian mode from RAM with big-endian RedBoot in the
    FLASH boot sector:

  bin/ixdp465/redboot_ROMRAMLE.bin
  bin/ixdp465/redboot_ROMRAMLE.elf
  bin/ixdp465/redboot_ROMRAMLE.srec

  - Running in little-endian mode from RAM with little-endian RedBoot in
    the FLASH boot sector:

  bin/ixdp465/redboot_RAMLE.bin
  bin/ixdp465/redboot_RAMLE.elf
  bin/ixdp465/redboot_RAMLE.srec

The files for the other supported platforms are arranged similarly.

Initial installations deal with the FLASH based RedBoots. Installation and
use of RAM based RedBoots is documented in the RedBoot User Manual.

After booting the initial installation of RedBoot, this warning may be
printed:

  FLASH configuration checksum error or invalid key

This is normal and indicates that the FLASH must be configured for use by
RedBoot. Even if the above message is not printed, it is a good idea to
reinitialize the FLASH anyway. Do this with the fis command followed by
the fconfig command:

  RedBoot> fis init
  About to initialize [format] FLASH image system - are you sure (y/n)? y
  *** Initialize FLASH Image System
      Warning: device contents not erased, some blocks may not be usable
  ... Unlock from 0xf1fc0000-0xf2000000: .
  ... Erase from 0xf1fc0000-0xf2000000: .
  ... Program from 0x03fbf000-0x03fff000 at 0xf1fc0000: .
  ... Lock from 0xf1fc0000-0xf2000000: .

  RedBoot> fconfig -i
  Initialize non-volatile configuration - continue (y/n)? y
  Run script at boot: false
  Use BOOTP for network configuration: true
  Console baud rate: 115200
  DNS server IP address: 
  GDB connection port: 9000
  Force console for special debug messages: false
  Network debug at boot time: false
  Update RedBoot non-volatile configuration - continue (y/n)? y
  ... Unlock from 0xf1f80000-0xf1f81000: .
  ... Erase from 0xf1f80000-0xf1f81000: .
  ... Program from 0x03fb2000-0x03fb3000 at 0xf1f80000: .
  ... Lock from 0xf1f80000-0xf1f81000: .


Endianess Issues
----------------
When starting a linux kernel which has different endianess than RedBoot,
the -x switch must be used with the exec command.

Prior releases of RedBoot also used a -x switch with the load command.
This is no longer available. So, when loading a kernel with a different
endianess, the swab command to byteswap the image before executing the
image. For example, to load and launch a little-endian zImage using a
big-endian RedBoot, use something like:

  RedBoot> load -r -v -b 0x01100000 zImage-LE
  Raw file loaded 0x01100000-0x011dacd3, assumed entry at 0x01100000
  RedBoot> swab -b 0x01100000 -l 0x100000 -4
  RedBoot> exec 0x01100000 -c "console=ttyS0,115200"


NPE Ethernet Support
--------------------
This release supports two built-in NPE ethernet ports (NPE-B and NPE-C)
on the IXP42X based boards and three NPE ports (NPE-A, NPE-B, and NPE-C)
on the IXDP465. The default ethernet port is the PCI based NIC.

RedBoot allows you to set a preferred default ethernet port using the RedBoot
"fconfig" command. If the default ethernet device is not found (for instance,
the PCI card is not installed), RedBoot will try to use one of the other ports.
The default port is selected with the "fconfig net_device" command. Acceptable
devices for the "fconfig net_device" command are:

   i82559_eth0
   npe_eth0
   npe_eth1
   npe_eth2

npe_eth0 uses NPE-B, npe_eth1 uses NPE-C, and npe_eth2 uses NPE-A (only
available on IXDP465). By default, NPE-A is used for UTOPIA, not ethernet.
There is a new RedBoot flash configuration option which can be used to
control the default use for NPE-A. For example. this command will change
the default use of NPE-A to ethernet:

  RedBoot> fconfig utopia false

A reboot will be necessary for the change to take effect.

It may be necessary to set the MAC address (or Ethernet Station Address) of
the port before it is usable by RedBoot. The IXDP465 and IXDP425 boards store
the MAC addresses for the NPE ethernet ports in serial EEPROMs. The other boards
use the flash memory to hold the NPE MAC addresses. To set the MAC address
of an NPE port on the IXDP465 and IXDP425 board, use the "set_npe_mac" command.
This command accepts a "-p" argument to specify the port (0 for NPEB, 1 for NPEC,
or 2 for NPE-A) and a MAC address formatted as xx:xx:xx:xx:xx:xx or xxxxxxxxxxxx.
For instance, to set the MAC address for npe_eth0 (NPEB) use:

  RedBoot> set_npe_mac -p 0 00:01:AF:00:20:EC

Note:
  Jumpers P4 and P2 on the IXDP465 CPU card must be jumpered
  for IXP connection to I2C, not GPIO.
  Also, JP126 on IXDP465 baseboard must be jumpered to allow
  writes to EEPROM

PCI Enumeration
---------------

RedBoot will enumerate the PCI bus and assign memory and io resources
to the devices found. Because of the limited nature of the CPU window
into PCI space, it may be necessary for RedBoot to ignore certain
devices which may need special consideration. Each of the platforms
provides a hook where a decision can be made whether or not RedBoot
assigns resources to a given PCI device BAR. This function:

   int hal_plf_pci_ignore_bar(void *dev_info, int bar)

is located in:

   hal/arm/xscale/<boardname>/current/src/<boardname>_pci.c

This function should return a non-zero value if the given device
BAR should be ignored and no resources assigned to it.


Rebuilding RedBoot
------------------

The build process is nearly identical on all four supported configurations.
Assuming that the provided RedBoot source tree is located in the current 
directory and that we want to build a RedBoot that runs from the FLASH boot
sector, the build process for the IXDP465 is:

  % export TOPDIR=`pwd`
  % export ECOS_REPOSITORY=${TOPDIR}/packages
  % export VERSION=current
  % mkdir ${TOPDIR}/build
  % cd ${TOPDIR}/build
  % ecosconfig new ixdp465 redboot
  % ecosconfig import ${ECOS_REPOSITORY}/hal/arm/xscale/ixdp465/${VERSION}/misc/redboot_ROM.ecm
  % ecosconfig tree
  % make

If a different configuration is desired, simply use the above build processes but
substitute an alternate configuration file for the ecosconfig import command.

Also, if the optional NPE ethernet support is include, append "_npe" to the
target name when rebuilding. For example, in the above example, use:

  % ecosconfig new ixdp465_npe redboot

for NPE-enabled RedBoot.

Building ecosconfig
-------------------

An ecosconfig binary is supplied in the bin directory, but you may wish
to build it from source.

Detailed instructions for building the command-line tool ecosconfig
on UNIX can be found in host/README. For example:

  mkdir $TEMP/redboot-build
  cd $TEMP/redboot-build
  $TOPDIR/host/configure --prefix=$TEMP/redboot-build --with-tcl=/usr
  make 


