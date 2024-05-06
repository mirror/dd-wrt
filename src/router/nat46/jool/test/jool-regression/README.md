A full regression test suite.

WARNING: Requires elevated permissions. It will modify /etc/rc.local and /etc/default/grub.

The purpose of this script is to automate unit and graybox testing on Jool, being compiled and run 
at every kernel the host machine has installed.

To run this script, make sure to modify the `config` file to suit your environment:

- Set where your git repository is, and where your Jool repository is.
- Set whichever branch you want to test
- It should automatically detect kernels installed.

This script assumes your /boot/grub/grub.cfg file contains both the regular kernel image and the "recovery mode"
for each kernel installed (it will iterate through the kernel's list in steps of two).

Run `jool-init.sh` to start the test suite.

