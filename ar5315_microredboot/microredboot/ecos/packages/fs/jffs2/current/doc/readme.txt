This package is a port of the JFFS2 flash filing system to eCos. It has been
developed on the Compaq Ipaq, and has not been tested on any other device.

This code is subject to the original licensing terms, and additionally it 
should be noted that this code is still in an early stage of development. 

As this code will write to flash directly, caution should be exercised in
its use. It may cause areas of the flash chips essential to the operation of
the device to become corrupted.


Minor modifications are necessary to the the eCos flash drivers 

io/flash/current/src/flash.c

devs/flash/intel/strata/current/src/flash_program_buf.c

to allow byte aligned rather than word aligned writes, and to ensure overwriting an existing
word is successful (these are supplied in jffs2/current/src).


Two test files are included fileio1.c (which performs the same tests as used for eCos RamFS),
and romfileio1.c (tests as eCos RomFS).

romfileio1.c requires that a jffs2 filesystem image jffs2.img is present at the desired mount point.
This image was prepared on Linux with the tools originating with JFFS2 source from
www.infradead.org