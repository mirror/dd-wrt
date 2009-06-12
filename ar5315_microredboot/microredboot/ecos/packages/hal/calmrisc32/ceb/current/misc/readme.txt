RedBoot for the Samsung CalmRISC32 core evaluation board
May 7th 2001
README
========================================================================

This ReadMe contains instructions for running Redboot on the Samsung
calmRISC32 core evaluation board, and building RedBoot and ecosconfig.

You will need the GNUPro calmrisc32-elf toolchain which should be
installed in /usr/cygnus.

Overview
--------
This implementation of RedBoot supports the calmRISC32 core evaluation board.
The core board has no communication channel and requires an MDSChip board to
provide a serial channel for host communication. The calmRISC32 is a harvard
architecture with separate 32-bit program and data addresses. The instruction
set provides no instruction for writing to program memory. The MDSChip board
firmware (called CalmBreaker) provides a pseudo register interface so that
code running on the core has access to a serial channel and a mechanism to
write to program memory. The serial channel is fixed at 57600-8-N-1 by the
firmware. The CalmBreaker firmware also provides a serial protocol which
allows a host to download a program and to start or stop the core board.


Downloading and running RedBoot
--------------------------------
All storage on the core board is volatile, so RedBoot must be downloaded to
the board after every power cycle. Downloads require the use of a utility
program. The source file and build instructions for this utility are provided
in the RedBoot sources at:

   .../packages/hal/calmrisc32/ceb/current/support

To download the RedBoot image, first press the reset button on the MDSChip
board. The green 'Run' LED on the core board should go off. Now, use the
utility to download the RedBoot image with:

  % calmbreaker -p /dev/term/b --reset --srec-code -f redboot.elf

Note that the '-p /dev/term/b' specifies the serial port to use and will vary
from system to syetm. The download will take about two minutes. After it
finishes, start RedBoot with:

  % calmbreaker -p /dev/term/b --run

The 'Run' LED on the core board should be on. Connecting to the MDSboard with
a terminal and typing enter should result in RedBoot reprinting the command
prompt.

Rebuilding RedBoot
------------------

Assuming that the provided RedBoot source tree is located in the current 
directory, the build process is:

  % export TOPDIR=`pwd`
  % export ECOS_REPOSITORY=${TOPDIR}/packages
  % mkdir ${TOPDIR}/build
  % cd ${TOPDIR}/build
  % ecosconfig new calm32_ceb redboot
  % ecosconfig import ${ECOS_REPOSITORY}/hal/calmrisc32/current/misc/redboot_ROM.ecm
  % ecosconfig tree
  % make


Building ecosconfig
-------------------

An ecosconfig binary is supplied in the bin directory, but you may wish
to build it from source.

Detailed instructions for building the command-line tool ecosconfig
can be found in host/README. For example:

  mkdir $TEMP/redboot-build
  cd $TEMP/redboot-build
  $TOPDIR/host/configure --prefix=$TEMP/redboot-build --with-tcl=/usr
  make

