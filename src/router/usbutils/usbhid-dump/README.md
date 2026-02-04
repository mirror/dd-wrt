<!---
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2010 Nikolai Kondrashov
-->

Usbhid-dump
===========

Usbhid-dump is a USB HID dumping utility based on libusb 1.0. It dumps USB HID device report descriptors and reports themselves as they are being sent, for all or specific device interfaces.

Installation
------------

Run `./configure && make` to build and `make install` to install. If building from a Git tree, run `./bootstrap` first. Usbhid-dump can also be run directly from the source directory as `src/usbhid-dump`, without installation.

Usage
-----

Here is the output of `usbhid-dump --help`:

    $ usbhid-dump --help
    Usage: usbhid-dump [OPTION]...
    Dump USB device HID report descriptor(s) and/or stream(s).
    
    Options:
      -h, --help                       output this help message and exit
      -v, --version                    output version information and exit
    
      -s, -a, --address=bus[:dev]      limit interfaces by bus number
                                       (1-255) and device address (1-255),
                                       decimal; zeroes match any
      -d, -m, --model=vid[:pid]        limit interfaces by vendor and
                                       product IDs (0001-ffff), hexadecimal;
                                       zeroes match any
      -i, --interface=NUMBER           limit interfaces by number (0-254),
                                       decimal; 255 matches any
    
      -e, --entity=STRING              what to dump: either "descriptor",
                                       "stream" or "all"; value can be
                                       abbreviated
    
      -t, --stream-timeout=NUMBER      stream interrupt transfer timeout, ms;
                                       zero means infinity
      -p, --stream-paused              start with the stream dump output
                                       paused
      -f, --stream-feedback            enable stream dumping feedback: for
                                       every transfer dumped a dot is
                                       printed to stderr
    
    Default options: --stream-timeout=60000 --entity=descriptor
    
    Signals:
      USR1/USR2                        pause/resume the stream dump output
    

**Warning:** please be careful running usbhid-dump as a superuser without limiting your device selection with options. Usbhid-dump will try to dump every device possible and If you're using a USB keyboard to control your terminal, it will be detached and you will be unable to terminate usbhid-dump and regain control.

If that happens just don't touch your keyboard for the duration of the interrupt transfer timeout (1 minute by default) and the dumping will be aborted.

Here is an example of a report descriptor and stream dump from a mouse:

    $ sudo usbhid-dump --entity=all --address=2:3
    002:003:000:DESCRIPTOR         1290272184.081322
     05 01 09 02 A1 01 09 01 A1 00 05 09 19 01 29 03
     15 00 25 01 75 01 95 03 81 02 75 05 95 01 81 01
     05 01 09 30 09 31 09 38 15 81 25 7F 75 08 95 03
     81 06 C0 C0

    Starting dumping interrupt transfer stream
    with 1 minute timeout.

    002:003:000:STREAM             1290272185.210022
     00 FF 00 00

    002:003:000:STREAM             1290272185.217988
     00 FE 00 00

    002:003:000:STREAM             1290272185.225985
     00 FC 01 00

    002:003:000:STREAM             1290272185.233995
     00 FE 01 00

    002:003:000:STREAM             1290272185.241992
     00 FF 01 00

    002:003:000:STREAM             1290272185.249995
     00 FE 02 00

    002:003:000:STREAM             1290272185.257993
     00 FF 01 00

    ^C

In the output above "002" is the bus number, "003" is the device address and "000" is the interface number. "DESCRIPTOR" indicates descriptor chunk and "STREAM" - stream chunk. The number to the right is the timestamp in seconds since epoch. The hexadecimal numbers below is the chunk dump itself. Usually every stream chunk includes a whole report, but if the report is bigger than endpoint's wMaxPacketSize, it will span several chunks.

You can use usbhid-dump along with [hidrd-convert](https://github.com/DIGImend/hidrd) to dump report descriptors in human-readable format. Like this:

    $ sudo usbhid-dump -a2:3 -i0 | grep -v : | xxd -r -p | hidrd-convert -o spec
    Usage Page (Desktop),               ; Generic desktop controls (01h)
    Usage (Mouse),                      ; Mouse (02h, application collection)
    Collection (Application),
        Usage (Pointer),                ; Pointer (01h, physical collection)
        Collection (Physical),
            Usage Page (Button),        ; Button (09h)
            Usage Minimum (01h),
            Usage Maximum (03h),
            Logical Minimum (0),
            Logical Maximum (1),
            Report Size (1),
            Report Count (3),
            Input (Variable),
            Report Size (5),
            Report Count (1),
            Input (Constant),
            Usage Page (Desktop),       ; Generic desktop controls (01h)
            Usage (X),                  ; X (30h, dynamic value)
            Usage (Y),                  ; Y (31h, dynamic value)
            Usage (Wheel),              ; Wheel (38h, dynamic value)
            Logical Minimum (-127),
            Logical Maximum (127),
            Report Size (8),
            Report Count (3),
            Input (Variable, Relative),
        End Collection,
    End Collection
