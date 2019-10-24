SiS-PM Control for Linux
========================

(c) 2015-2019, Heinrich Schuchardt &lt;xypron.glpk@gmx.de&gt;
(c) 2011-2016, Pete Hildebrandt &lt;send2ph@gmail.de&gt;
(c) 2005-2011, Mondrian Nuessle et al.

Overview
--------

This project provides a management software for the following USB controlled
powerstrips:

* Gembird SIS-PMS SilverShield
* Gembird MSIS-PM
* EnerGenie EG-PMS2
* EnerGenie EG-PMS2

Follow the instructions in INSTALL to install the application
(generic ./configure && make && make install).

When you call `sispmct` without command line parameters, it will print the
available options. `man sispmctl` shows the  man page.

Installation
------------

See the file INSTALL for generic installation instructions.
The following configure options may be of interest:

--prefix=directory-prefix
        Set installation directory-prefix (default is /usr/local)

--enable-webless
        Disable the builtin webserver. It will not be compiled into
        the binary. The commandline options for the web-interface will
        not be available.

--with-webdir=directory
        Install the web-interface file to subdirectories of the
        given directory. A symbolic link in this directory will point
        to skin1. And the path will be the default path compiled into
        the binary.
        The default without this option is /usr/local/share/doc/sispmctl/skin.

--with-bindaddr=ipaddress
        Give the IP address (in dotted decimal form, i.e. 127.0.0.1) for
        the default interface that the webserver accepts connections on.
        The default without this option is to use all available interfaces.

--with-default-port=portnumber
        Give the default port number the webserver is listening on.
        If you do not specify this option the default is 2638

Dependencies
------------

- libusb 0.1.9+ must be installed, libusb-config in $PATH


Web-Interface
------------

The web interface may be started manually (i.e. sispmctl -l)
or using the inittab. Add the following line to your /etc/inittab:

pm:2345:respawn:/usr/local/bin/sispmctl -l

There are two skins between you might select:
	src/web1/	classic sispm_http style
	src/web2/	a vertical sized frame
The skins are installed under
$(PREFIX)/share/doc/sispmctl/httpd/skin1..2

The default skin is selected by the symbolic link
$(PREFIX)/share/doc/sispmctl/skin which points to skin1 after
installation. You can easily select a different skin by
changing this symbolic link.

It is quite easy to change one or write a new one. Try it.

The web interface does not recognize if a mSIS-PM is connected, so always 4
outlets will be displayed.

The web server can be started automatically with systemd. This requires the
following steps

Create user sispmctl in group sispmctl. The command on Debian and Ubuntu is

    adduser sispmctl --system --group --disabled-login --no-create-home

Provide the necessary authorizations via udev (see below).

    cp examples/60-sispmctl.rules /lib/udev/rules.d/60-sispmctl.rules
    udevadm control --reload-rules

Adjust and copy the systemd service definition

    cp examples/sispmctl.service /lib/systemd/system

Enable and start the service with

    systemctl enable sispmctl.service
    systemctl start sispmctl.service

Usage
-----

You can display the man page with

    man sispmctl

Permissions
-----------

Per default, only root is allowed to use devices directly, therefor the SiS-PM
also only works as root.

To allow group sispmctl access create file /lib/udev/rules.d/60-sispmctl.rules
with the following content

    SUBSYSTEM=="usb", ATTR{idVendor}=="04b4", ATTR{idProduct}=="fd10", GROUP="sispmctl", MODE="660"
    SUBSYSTEM=="usb", ATTR{idVendor}=="04b4", ATTR{idProduct}=="fd11", GROUP="sispmctl", MODE="660"
    SUBSYSTEM=="usb", ATTR{idVendor}=="04b4", ATTR{idProduct}=="fd12", GROUP="sispmctl", MODE="660"
    SUBSYSTEM=="usb", ATTR{idVendor}=="04b4", ATTR{idProduct}=="fd13", GROUP="sispmctl", MODE="660"
    SUBSYSTEM=="usb", ATTR{idVendor}=="04b4", ATTR{idProduct}=="fd15", GROUP="sispmctl", MODE="660"

Then reload the udev rules with

    udevadm control --reload-rules

Solaris Support
---------------
Solaris 10 and later come with sufficient libusb support; the ugen (generic USB)
driver needs to be associated with the SIS-PM devices by issueing the following
command before plugging in the device:

    for SIS-PM:
	update_drv -a -i '"usb4b4,fd11"' ugen
    for mSIS-PM:
	update_drv -a -i '"usb4b4,fd10"' ugen
	update_drv -a -i '"usb4b4,fd12"' ugen
    for energenie:
	update_drv -a -i '"usb4b4,fd13"' ugen
    for EG-PMS2:
	update_drv -a -i '"usb4b4,fd15"' ugen
