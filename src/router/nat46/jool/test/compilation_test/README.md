# Compilation Test

All this testing framework does is cross-compile Jool on several kernels. More than attempting to test Jool's behavior it is just a quick means to make sure we do not break our usage of the kernel API as Jool is developed, and to notice API changes when new kernels are released.

This is the rundown:

1. You clone Linux. (`git clone https://github.com/torvalds/linux`)
2. You precompile whatever versions of Linux you intend to test Jool's compilation in.  
   For example, you could make a directory containing a precompiled Linux 3.2, another one with Linux 3.3, and so on.  
   You don't need the full kernel; you only need the minimum necessary to cross-compile modules.  
   Every such precompiled Linux weights about 600 megabytes so plan ahead for room. At time of writing, we're supporting 28 kernels (3.2-4.9).  
   See `prepare-kernels.sh`.
3. You try compiling Jool on every precompiled Linux. (This is the test.)  
   See `compile-jool.sh`.

After a while of Jool development you repeat step 3 using the Linux instances you're still holding.

