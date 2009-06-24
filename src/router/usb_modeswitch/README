README for USB_ModeSwitch

For up-to-date and more detailed information (plus a friendly forum) visit
http://www.draisberghof.de/usb_modeswitch


What it is
==========

USB_ModeSwitch is (surprise!) a small mode switching tool for controlling
"flip flop" (multiple device) USB gear

Several new USB devices (especially high-speed WAN stuff, they're expensive
anyway) have their MS Windows drivers onboard; when plugged in for the first
time they act like a flash storage and start installing the driver from there.
After that (and on every consecutive plugging) this driver switches the mode
internally, the storage device vanishes (in most cases), and a new device (like
an USB modem) shows up. Some call that feature "ZeroCD".

Of course, nothing of this is documented in any form and there is hardly any
Linux driver available.
On the good side, most of the known devices work out of the box with the
available Linux modules like "usb-storage" or "usbserial". That leaves the
problem of the mode switching from storage to whatever the thing is supposed to
do.

Fortunately there are things like human intelligence, USB sniffing programs and
"libusb". It is possible to eavesdrop the communication of the MS Windows
driver, to isolate the command or action that does the switching, and to replay
the same thing with Linux.

USB_ModeSwitch eases the pain considerably by taking just the important para-
meters from a configuration file and doing all the initialization and communi-
cation stuff, with heavy help from "libusb".
Starting from version 0.9.7 it has an optional success check which spares the
manual call of "lsusb" to note any changes after execution.


How to install
==============

A x86 binary is included (compiled on Pentium M, Ubuntu 8.10).

If you want to compile it for yourself, just delete the binary and run "make".

Or type at the shell (with other options of your choice):
 > gcc -l usb -o usb_modeswitch usb_modeswitch.c
That's as easy as it gets ...

For installation run "make install" (you still have to edit the config file);

Or follow these manual steps:
Take the fresh executable "usb_modeswitch" (or the one provided with the
archive) and put it into your path (preferably "/sbin" or "/usr/sbin").

Put "usb_modeswitch.conf" into "/etc" and edit it according to your hardware.
It's heavily commented and should tell you what to do.

Alternatively you can use the command line to tell USB_ModeSwitch the things
it needs to know; try "usb_modeswitch -h" to list the parameters.
This way you can handle multiple configurations. If any command line parameters
except -W and -q are used, the default config file is NOT read.

Important: libusb stuff - like this tool - needs to be run as root!

For details how to automate the process visit the homepage (see above).


Known working hardware
======================

For details and specific setup see the homepage and the config file.


Contribute
==========

USB_ModeSwitch comes quite handy for experimenting with your own hardware if
not supported yet. You could try this approach:
Note the device's Vendor and Product ID from /proc/bus/usb/devices (or from
the output of lsusb); the assigned driver is usually "usb-storage". Then try
spying on the USB communication to the device with the same ID inside M$ Win-
doze. I recommend this tool:
"SniffUSB" (http://benoit.papillault.free.fr/usbsnoop/index.php.en).
Please post any improvements, new device information and/or bug reports to the
forum (see above) or send it to the author (see below)!


Whodunit
========

Copyright 2007, 2008, 2009 Josua Dietze (mail to the name from "usb_modeswitch.c"
 at the domain "draisberghof.de"; please do not post the complete address to the
 Net !) or write a personal message through the forum to "Josh"

Command line parsing, decent usage/config output and handling, advanced options
 and bugfixes:
 Joakim Wennergren (jokedst) (gmail.com)

TargetClass parameter implementation to support new Option devices/firmware:
 Paul Hardwick (http://www.pharscape.org)

Created with initial help from:
 "usbsnoop2libusb.pl" by Timo Lindfors (http://iki.fi/lindi/usb/usbsnoop2libusb.pl)

Config file parsing stuff borrowed from:
 Guillaume Dargaud (http://www.gdargaud.net/Hack/SourceCode.html)

Hexstr2bin function borrowed from:
 Jouni Malinen (http://hostap.epitest.fi/wpa_supplicant, from "common.c")

Code, fixes and ideas from:
 Aki Makkonen
 Denis Sutter 
 Lucas Benedicic 
 Roman Laube 
 Luigi Iotti
 Vincent Teoh
 Tommy Cheng
 Daniel Cooper
 Andrew Bird

More contributors are listed in the config file.


History
=======

Version 1.0.2, 2009/06/10
    Output bugs fixed
Version 1.0.1, 2009/06/08
    Added output of descriptor strings for further identification
Version 1.0.0, 2009/06/01
    Attention: possible incompatibilities for command line control!
    On/off flags don't require arguments anymore (-H, -S, -O, -d, -R,
    -n, new: -I), meaning "-R 0" does a reset like "-R 1" or "-R";
    long option names changed to standard format (e.g. --HuaweiMode to
    --huawei-mode); added device inquiry, for future help with device
    identification; catch error -19 as possible success; send and response
    endpoints now autoselected (consequently NeedResponse is back);
    new devices
Version 0.9.7, 2009/04/15
    Updated SonyMode, MD 400 now stable; automatic default endpoint
    detection from Andrew Bird
Version 0.9.7beta, 2009/03/15
    Major code clean up, optional success control (both suggested
    by Daniel Cooper), new devices
Version 0.9.6, 2009/01/08
    Special modes added for Sierra and Sony Ericsson, new devices
Version 0.9.5, 2008/10/27
    New options for USB tuning (jokedst), lots of new devices, clean up
Version 0.9.4, 2008/06/09
    Compat fix for libusb on FreeBSD quirks, more devices
Version 0.9.4beta2, 2008/03/19
    Successful udev device release fix
Version 0.9.4beta, 2008/03/16
    Multiple device support
Version 0.9.3, 2008/03/09
    More devices, no changes from beta version
Version 0.9.3beta, 2007/12/30
    New TargetClass parameter for recent Option firmware (Paul Hardwick), more devices
Version 0.9.2, 2007/11/02
    New Huawei mode (code from Miroslav Bobovsky, added by Denis Sutter), more devices
Version 0.9.1beta, 2007/09/04 (jokedst)
    Added command line parsing, cleaned up config stuff, doc updates
Version 0.9beta, 2007/08/15
    Name change from "icon_switch", parameter file and generalizing
Version 0.2, 2006/09/25
    Code cleaning, more messages
Version 0.1, 2006/09/24
    (as "icon_switch") Just very basic functionality ...


Legal
=====

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details:

http://www.gnu.org/licenses/gpl.txt

Or find it as the file COPYING in this folder.




Last revised: 2009-06-10, Josua Dietze
