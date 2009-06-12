How to Build the RedBoot Bootrom Loader on Atheros WiSoc

The hal/mips/ar5312 directory contains code needed to support RedBoot bootrom
loader for reference designs based on any of several Wireless System-on-a-Chips 
(WiSoc) produced by Atheros Communications, Inc.

The WiSoc's supported by this code include the AR5312 and AR2313.  In addition
to the code in this directory, there are small board-dependent eCos directories 
that are also needed in order to build RedBoot.  Depending on which board you
are using, you will need:
  hal/mips/ar5312 AND ONE OF
  hal/mips/ap30 or
  hal/mips/ap43 or
  hal/mips/ap48

The ethernet driver you will need is:
  devs/eth/mips/ar531x.

Flash support comes from
  devs/flash/sst (for AP30 which uses an SST flash part)  OR
  devs/flash/amd (for AP43, AP48 which use an MX part)

It is strongly recommended that you use a Linux environment (e.g. Linux on x86)
to build "RedBoot for AP30", "RedBoot for AP43", or "RedBoot for AP48".  The
eCos web site indicates that there have been problems using Cygwin on Windows
to build eCos.

STEP 1) Establish an eCos source tree and obtain build tools.
        Follow the download instructions at the eCos web site,
            http://ecos.sourceware.org/getstart.html
        Example:
          wget --passive-ftp ftp://ecos.sourceware.org/pub/ecos/ecos-install.tcl
          sh ecos-install.tcl

        These instructions assume that your source base is eCos v2.0.  If you
        choose to use a different (more recent) version, you may have some
        additional issues to resolve.

STEP 2) Make sure your environment variables are set up properly.
        For instance
        . /opt/ecos/ecosenv.sh         OR   /* sh, bash users */
        source /opt/ecos/ecosenv.csh        /* csh, tcsh users */
        NOTE: Inspect the script to verify that it is correct.

STEP 3) Create a "build directory".
        mkdir build1

STEP 4) Obtain Atheros files:
        ecos-2.0/packages/devs/flash/sst
        ecos-2.0/packages/devs/eth/mips/ar531x
        ecos-2.0/packages/hal/mips/ap30
        ecos-2.0/packages/hal/mips/ap43
        ecos-2.0/packages/hal/mips/ap48
        ecos-2.0/packages/hal/mips/ar5312
        ecos-2.0/packages/ecos.db
        You can simply overwrite existing ecos v2.0 files with the above.

STEP 5) Apply Atheros patches:
        cd $ECOS_REPOSITORY
        for i in hal/mips/ar5312/*/DIFFS/*.diff
        do
            patch -p1 < $i
        done

STEP 6) Create a new RedBoot eCos configuration.
     6a) cd build1
     6b) ecosconfig new atheros_ap30 redboot OR
         ecosconfig new atheros_ap43 redboot OR
         ecosconfig new atheros_ap48 redboot
     6c) ecosconfig import $ECOS_REPOSITORY/hal/mips/ap30/v2_0/misc/redboot_ROM.ecm OR
         ecosconfig import $ECOS_REPOSITORY/hal/mips/ap43/v2_0/misc/redboot_ROM.ecm OR
         ecosconfig import $ECOS_REPOSITORY/hal/mips/ap48/v2_0/misc/redboot_ROM.ecm
     6d) Use "configtool ecos.ecc".
         Add "-mlong-calls" to the "Global compiler flags" under
         "Global build options".  Hit RETURN, then save and exit.
         [TBD: This should be done automatically when the import
         happens.]
         You can also change around any options that you need to.

     6e) ecosconfig tree
         NOTE: In order to create a downloadable RAM-based version of RedBoot:
         Replace ROM with RAM in step 6c, above.  The RAM version of RedBoot is
         useful for testing, and it is also used to write the ROM version of
         RedBoot into flash.  You cannot use the ROM version of RedBoot to write
         a newer ROM version into flash!

STEP 7) Build RedBoot object.
        make
        The RedBoot bootloader is in install/bin/redboot.elf

STEP 8) Convert the ROM RedBoot ELF image to a flashable image.
        cd install/bin
        mipsisa32-elf-objcopy -O binary redboot.elf redboot_rom.bin
