
Installation of RedBoot on a Target Board using RedBoot's Flash Commands
------------------------------------------------------------------------

These are generic instructions applicable to RedBoot for many platforms.
Details of what you type have been changed to refer specifically to the
nanoEngine hardware, but you should also read the further instructions for
Bright Star Engineering's nanoEngine and SA1110 boards - known as target
"nano" in eCos configuration terms - which follow.

Here's how to install RedBoot, using the redboot images you should find in
        loaders/PLATFORM/
in your installation directory (sizes and dates are just examples):
        431497 Aug  9 15:28 redboot-ram.elf
        184802 Aug  9 15:28 redboot-ram.srec
        433104 Aug  9 15:29 redboot-rom.elf      (not used in this recipe)
        194732 Aug  9 15:29 redboot-rom.srec

Copy the two '.srec' files to /tftpboot or where-ever they have to be for
your TFTP server.

Briefly, we use whatever boot flash image you have in place already (CygMon
or an eCos stub ROM) along with GDB, to execute a RAM based version of
RedBoot.  That is used, in its command-line mode, to fetch a ROM-based boot
image of RedBoot and write it into the flash memory.  "Fetching" the image
means TFTP from a server; the image must be in S-Record format.  We then
reset the target, thus running the newly-installed boot image of RedBoot.
That in turn is used, in its command-line mode, to fetch a RAM-based boot
image of RedBoot and write it into a different area of the flash memory, in
order to make it easier to do the first part (running a RAM-based RedBoot
in order to update the boot block) again in future.

NB: the instructions below refer to a system with 8Mb of flash, at address
0x50000000 in memory.  That address is common to SA11x0 CPUs.

Other boards might have a different start address, such as 0x41000000 and
different flash blocksizes.  RedBoot's startup banner will tell you the
details, if all is functioning correctly.  If the address is different, you
must use different addresses for saving the images into the Flash Image
System (fis).  The command "fis list" will tell you the addresses to use
for the "RedBoot" and "RedBoot[backup]" images, immediately after the "fis
init" command.


Alternatively you can make a plain binary from the redboot-rom.elf and
"blow" that into the boot flash using the means of your choice, as with
previous systems.


1. Load a RedBoot, built for RAM startup, into RAM using existing GDB
   stubs.  Note: do not run it yet!

   % arm-elf-gdb -nw loaders/nano/redboot-ram.elf 

   GNU gdb 4.18-ecos-99r1-991015
   Copyright 1998 Free Software Foundation, Inc.
   GDB is free software, covered by the GNU General Public License, and you are
   welcome to change it and/or distribute copies of it under certain conditions.
   Type "show copying" to see the conditions.  This version of GDB is supported
   for customers of Cygnus Solutions.  Type "show warranty" for details.
   This GDB was configured as "--host=i686-pc-linux-gnu
   --target=arm-elf"...(no debugging symbols found)...
   (gdb) set remotebaud 38400
   (gdb) tar rem /dev/ttyS0
   Remote debugging using /dev/ttyS0
   0x41000838 in ?? ()
   (gdb) load
   Loading section .rom_vectors, size 0x44 lma 0x20000
   Loading section .text, size 0xf06c lma 0x20044
   Loading section .rodata, size 0x19a8 lma 0x2f0b0
   Loading section .data, size 0x474 lma 0x30a58
   Start address 0x20044 , load size 69324
   Transfer rate: 25208 bits/sec.
   (gdb) detach
   Ending remote debugging.
   (gdb) q

2. Execute RedBoot from RAM, and initialize the flash filing system.
       Notes: the key here is the "-o" option which keeps minicom from
       sending junk.
       The magic phrase "$c#63" is important: you must type it in exactly
       thus.  It is the packet which a "continue" command in GDB would send
       to the target.  If you get no response, try "+$c#63" instead.
       The IP and server info comes from BOOTP, which is how this RedBoot
       will start up if the flash does not contain good config info.

   % minicom -o ttyS0

   $c#63
   RedBoot(tm) debug environment - built 07:45:57, Aug  7 2000
   Copyright (C) 2000, Red Hat, Inc.

   RAM: 0x00000000-0x01000000
   FLASH: 0x50000000 - 0x50400000, 32 blocks of 0x00020000 bytes ea.
   IP: 192.168.1.25, Default server: 192.168.1.101
   RedBoot> fi init
   About to initialize [format] FLASH image system - are you sure (y/n)? y
   *** Initialize FLASH Image System
       Warning: device contents not erased, some blocks may not be usable
   ... Erase from 0x503c0000-0x50400000: .
   ... Program from 0x00fb0000-0x00fb0400 at 0x503c0000: .


3. Program RedBoot image into FLASH:
       This expects the file redboot-post.srec (see below) to exist in the
       TFTP server space on the server that answered the BOOTP request.
       It loads into the free flash memory following the boot firmware, at
       address 0x50040000.

   RedBoot> lo -v /tftpboot/redboot-post.srec -b 0x00100000
   Address offset = bf100000
   Entry point: 0x50040044, address range: 0x50040000-0x50051384
   RedBoot> fi cr RedBoot[post] -f 0x50040000 -b 0x00100000 -l 0x20000
   An image named 'RedBoot[post]' exists - are you sure (y/n)? y
   ... Erase from 0x50040000-0x50060000: .
   ... Program from 0x00100000-0x00120000 at 0x50040000: .
   ... Erase from 0x503c0000-0x50400000: .
   ... Program from 0x00fb0000-0x00ff0000 at 0x503c0000: .
   RedBoot> 

****reset the board here, leaving your terminal program connected****

   RedBoot(tm) debug environment - built 07:47:35, Aug  7 2000
   Copyright (C) 2000, Red Hat, Inc.
   
   RAM: 0x00000000-0x01000000
   FLASH: 0x50000000 - 0x50400000, 32 blocks of 0x00020000 bytes ea.
   IP: 192.168.1.25, Default server: 192.168.1.101
   RedBoot> 


4. Install RAM based RedBoot for backup/update:
       Similar considerations apply: redboot-ram.srec must be an S-record
       version of RedBoot built for RAM startup.

   RedBoot> lo -v /tftpboot/redboot-ram.srec
   Entry point: 0x00020044, address range: 0x00020000-0x00030ecc
   RedBoot> fi cr RedBoot[backup] -f 0x50060000 -b 0x20000 -r 0x20000 -l 0x20000
   An image named 'RedBoot[backup]' exists - are you sure (y/n)? y
   ... Erase from 0x50060000-0x50080000: .
   ... Program from 0x00020000-0x00040000 at 0x50060000: .
   ... Erase from 0x503c0000-0x50400000: .
   ... Program from 0x00fb0000-0x00ff0000 at 0x503c0000: .
   RedBoot> 

        You have now updated your board completely.  Phew!



5. To update RedBoot with a new version of RedBoot, it is necessary to run
   a RAM-based version of RedBoot which itself re-writes the ROM-based one,
   because you can't re-write the code that is executing at the time.

   RedBoot> fi lo RedBoot[backup]
   RedBoot> g
   +
   RedBoot(tm) debug environment - built 07:45:57, Aug  7 2000
   Copyright (C) 2000, Red Hat, Inc.
   
   RAM: 0x00000000-0x01000000
   FLASH: 0x50000000 - 0x50400000, 32 blocks of 0x00020000 bytes ea.
   IP: 192.168.1.25, Default server: 192.168.1.101
   RedBoot> 
   
     .. continue with step 3, using whatever your new boot image is called
        in the TFTP-place, in .srec format.


You probably also want to set up then environment with your own IP
addresses and so on.  Recall that this IP address is the one you use for
GDB to talk to the board, not the IP address which the eCos application
will take on (by BOOTP/DHCP or whatever means according to configury as
usual).

   RedBoot> fconfig
   Network debug at boot time: false 
   Use BOOTP for network configuration: false 
   Local IP address: 192.168.1.25 
   Default server IP address: 192.168.1.101 
   GDB connection port: 1000 
   Run script at boot: false 
   RedBoot> 


RedBoot for the nanoEngine/commEngine "nano" Target
---------------------------------------------------

Unlike other targets, the nanoEngine comes equipped with boot firmware
which you cannot modify.  See chapter 5 "nanoEngine Firmware" of the
nanoEngine Hardware Reference Manual (we refer to "July 17, 2000 Rev 0.6")
from Bright Star Engineering.

Because of this, eCos and so Redboot supports only these two startup types:
RAM and POST, rather than the more usual ROM, RAM and optionally POST.

Briefly, the POST-startup RedBoot image lives in flash following the BSE
firmware.  The BSE firmware is configured, using its standard "bootcmd"
parameter, to jump into the RedBoot image at startup.

You can perform the initial load of the POST-startup RedBoot image into
flash using the BSE firmware's "load" command.  This will load, using TFTP,
a binary file and program it into flash in one neat operation.  Because no
memory management is used in the BSE firmware, flash is mapped from address
zero upwards, so the address for the RedBoot POST image is 0x40000.  You
must use the binary version of RedBoot for this, "redboot-post.bin"

This assumes you have set up the other BSE firmware config parameters such
that it can communicate over your network, to your TFTP server.

        >
        >load /tftpboot/redboot-post.bin 40000   
        loading ... erasing blk at 00040000
        erasing blk at 00050000
        94168 bytes loaded cksum 00008579
         done
        >
        > set bootcmd "go 40000"
        > get
        myip = 10.16.19.198
        netmask = 255.255.255.0
        eth = 0
        gateway = 10.16.19.66
        serverip = 10.16.19.66
        bootcmd = go 40000
        >

NB: the BSE firmware runs its serial IO at 9600 Baud; RedBoot runs instead
at 38400 Baud.  You must select the right baud rate in your terminal
program to be able to set up the BSE firmware.

After a reset, the BSE firmware will print
        Boot: BSE 2000 Sep 12 2000 14:00:30
        autoboot: "go 40000" [hit ESC to abort]
and then RedBoot starts, switching to 38400 Baud.

Once you have installed a bootable RedBoot in the system in this manner, we
advice re-installing using the generic method described first in this note
in order that the Flash Image System contains an appropriate description of
the flash entries.


Rebuilding RedBoot from Source
------------------------------

To rebuild RedBoot from source, first locate the configuration export files
for your platform in the eCos source repository.  The RAM and POST startup
configuration exports can usually be found in a directory named "misc" in
the platform HAL in the eCos source repository, named either:
   2164 Nov 29 14:59 misc/redboot_RAM.cfg
   2221 Nov 29 14:59 misc/redboot_POST.cfg
or
   1432 Feb  1 13:27 misc/redboot_RAM.ecm
   1487 Feb  1 14:38 misc/redboot_POST.ecm
Having located these files, copy them say to /tmp, say, for less typing.

To make redboot for RAM startup:
  mkdir redboot.RAM
  cd redboot.RAM
  ecosconfig new nano redboot
  ecosconfig import /tmp/redboot_RAM.ecm
  ecosconfig tree
  make

To build the POST version, in a different build/config directory, just use
the config export redboot_POST.ecm (or .cfg) instead.

The resulting files will be, in each of the POST and RAM startup build
places:
   70456     ..../install/bin/redboot.bin
  433104     ..../install/bin/redboot.elf
   91407     ..../install/bin/redboot.img
  194732     ..../install/bin/redboot.srec

The .elf and .srec files have the obvious relationship to those supplied in
the "loaders/nano" directory in the install.


Cohabiting with POST in Flash
-----------------------------

The configuration export named redboot_POST.ecm configures redboot to build
for execution at address 0x50040000 (or during bootup, 0x00040000).  This
is to allow power-on self-test (POST) code or immutable firmware to live in
the lower addresses of the flash and to run before RedBoot gets control.
The assumption is that RedBoot will be entered at its base address in
physical memory, ie. 0x00040000.  Alternatively, for testing, you can call
it in an already running system by "go 0x50040040" at another RedBoot
prompt, or a branch to that address; the address is where the reset vector
points, and is reported by RedBoot's tftp load command and listed by the
fis list command, amongst other places.

Using the POST configuration enables a normal config option which causes
linking and initialization against memory layout files called "...post..."
rather than "...rom..." or "...ram..." in the include/pkgconf directory,
specifically:
   665 Feb  9 17:57 include/pkgconf/mlt_arm_sa11x0_nano_post.h
   839 Feb  9 17:57 include/pkgconf/mlt_arm_sa11x0_nano_post.ldi
   585 Feb  9 17:57 include/pkgconf/mlt_arm_sa11x0_nano_post.mlt
It is these you should edit if you wish to move that execution address from
0x50040000 in the POST configuration.  Startup type naturally remains ROM
in this configuration.

Because the nanoEngine contains immutable boot firmware at the start of
flash, RedBoot for this target is configured to reserve that area in the
Flash Image System, and to create by default an entry for the POST startup
RedBoot.

  RedBoot> fis list
  Name              FLASH addr  Mem addr    Length      Entry point
  (reserved)        0x50000000  0x50000000  0x00040000  0x00000000
  RedBoot[post]     0x50040000  0x00100000  0x00020000  0x50040040
  RedBoot[backup]   0x50060000  0x00020000  0x00020000  0x00020040
  RedBoot config    0x503E0000  0x503E0000  0x00010000  0x00000000
  FIS directory     0x503F0000  0x503F0000  0x00010000  0x00000000
  RedBoot> 

The entry "(reserved)" ensures that the FIS cannot attempt to overwrite the
BSE firmware, thus ensuring that the board remains bootable and recoverable
even after installing a broken RedBoot image.


Special Redboot Commands
------------------------

The nanoEngine/commEngine has one or two Intel i82559 Ethernet controllers
installed, but these have no associated serial EEPROM in which to record
their Ethernet Station Address (ESA, or MAC address).  The BSE firmware
records an ESA for the device it uses, but this information is not
available to RedBoot; we cannot share it.

To keep the ESAs for the two ethernet interfaces, two new items of RedBoot
configuration data are introduced.  You can list them with the RedBoot
command "fconfig -l" thus:

  RedBoot> fconfig -l
  Run script at boot: false 
  Use BOOTP for network configuration: false 
  Local IP address: 10.16.19.91 
  Default server IP address: 10.16.19.66
  Network hardware address [MAC] for eth0: 0x00:0xB5:0xE0:0xB5:0xE0:0x99
  Network hardware address [MAC] for eth1: 0x00:0xB5:0xE0:0xB5:0xE0:0x9A
  GDB connection port: 9000 
  Network debug at boot time: false 
  RedBoot> 

You should set them before running RedBoot or eCos applications with the
board connected to a network.  The fconfig command can be used as for any
configuration data item; the entire ESA is entered in one line.


Memory Maps
-----------

The first level page table is located at physical address
0xc0004000.  No second level tables are used.

NOTE
The virtual memory maps in this section use a C and B column to
indicate whether or not the region is cached (C) or buffered (B).

Physical Address Range     Description
-----------------------    ----------------------------------
0x00000000 - 0x003fffff    4Mb FLASH (nCS0)
0x18000000 - 0x18ffffff    Internal PCI bus - 2 x i82559 ethernet
0x40000000 - 0x4fffffff    External IO or PCI bus
0x80000000 - 0xbfffffff    SA-1110 Internal Registers
0xc0000000 - 0xc7ffffff    DRAM Bank 0 - 32Mb SDRAM
0xc8000000 - 0xcfffffff    DRAM Bank 1 - empty
0xe0000000 - 0xe7ffffff    Cache Clean

Virtual Address Range    C B  Description
-----------------------  - -  ----------------------------------
0x00000000 - 0x001fffff  Y Y  DRAM - 8Mb to 32Mb
0x18000000 - 0x180fffff  N N  Internal PCI bus - 2 x i82559 ethernet
0x40000000 - 0x4fffffff  N N  External IO or PCI bus
0x50000000 - 0x51ffffff  Y Y  Up to 32Mb FLASH (nCS0)
0x80000000 - 0xbfffffff  N N  SA-1110 Internal Registers
0xc0000000 - 0xc0ffffff  N Y  DRAM Bank 0: 8 or 16Mb
0xc8000000 - 0xc8ffffff  N Y  DRAM Bank 1: 8 or 16Mb or absent
0xe0000000 - 0xe7ffffff  Y Y  Cache Clean

The FLASH based RedBoot POST-startup image occupies virtual addresses
0x50040000 - 0x5005ffff.

The ethernet devices use a "PCI window" to communicate with the CPU.  This
is 1Mb of SDRAM which is shared with the ethernet devices that are on the
PCI bus.  It is neither cached nor buffered, to ensure that CPU and PCI
accesses see correct data in the correct order.  By default it is
configured to be megabyte number 30, at addresses 0x01e00000-0x01efffff.
This can be modified - and indeed must be, if less than 32Mb of SDRAM is
installed - via the memory layout tool, or by moving the section
"__pci_window" referred to by symbols CYGMEM_SECTION_pci_window* in the
linker script.

Though the nanoEngine ships with 32Mb of SDRAM all attached to DRAM bank 0,
the code can cope with any of these combinations also; "2 x " in this
context means one device in each DRAM Bank.

  1 x 8Mb = 8Mb     2 x 8Mb = 16Mb
  1 x 16Mb = 16Mb   2 x 16Mb = 32Mb

All are programmed the same in the memory controller.

Startup code detects which is fitted and programs the memory map
accordingly.  If the device(s) is 8Mb, then there are gaps in the
physical memory map, because a high order address bit is not
connected.  The gaps are the higher 2Mb out of every 4Mb.

The SA11x0 OS timer is used as a polled timer to provide timeout
support within RedBoot.


Nano Platform Port
------------------

The nano is in the set of SA11X0-based platforms.  It uses the
arm architectural HAL, the sa11x0 variant HAL, plus the nano
platform hal.  These are components
        CYGPKG_HAL_ARM                  hal/arm/arch/
        CYGPKG_HAL_ARM_SA11X0           hal/arm/sa11x0/var
        CYGPKG_HAL_ARM_SA11X0_NANO      hal/arm/sa11x0/nano
respectively.

The target name is "nano" which includes all these, plus the
ethernet driver packages, flash driver, and so on.


Ethernet Driver
---------------

The ethernet driver is in two parts:

A generic ether driver for Intel i8255x series devices, specifically the
i82559, is devs/eth/intel/i82559.  Its package name is
CYGPKG_DEVS_ETH_INTEL_I82559.

The platform-specific ether driver is devs/eth/arm/nano.  Its package is
CYGPKG_DEVS_ETH_ARM_NANO.  This tells the generic driver the address in IO
memory of the chip, for example, and other configuration details.

This driver picks up the ESA from RedBoot's configuration data - unless
configured to use a static ESA in the usual manner.

