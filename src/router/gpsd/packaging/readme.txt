= Recommendations for distribution integrators =

The X11 subdirectory contains icons and a project logo for use
in desktop packaging.

Usable deb and RPM specifications have their own subdirectories here.
Our package files want to set up a hotplug script to notify gpsd
when a potential GPS device goes active and should be polled.  The
goal is zero configuration; users should never have to tell gpsd how
to configure itself.

Bluetooth has a requirement to be able to write to the gpsd control
socket from a userland device manager.  Accordingly, you probably 
want to set up a gpsd privilege group and make sure the Bluetooth
device manager is in it.

If you see problems with gpsd being unable to get write access to
the USB or RS232 devices on your machine, it may be because after
startup, when it drops privileges, it moves itself from root to the
wrong group. gpsd normally figures out which group it should move
to by looking at the ownership of a prototypical tty (look in gpsd.c
for this code) but the owning user and group can be compiled in with
build-system options.

