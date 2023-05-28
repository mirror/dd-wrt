Installing openNDS
##################

Prerequisites
*************

openNDS is designed to run on a device configured as an IPv4 router and will have at least two network interfaces:

 **A WAN interface** (Wide Area Network). This interface must be connected to an Internet feed:

 * Either an ISP CPE (Internet Service Provider Customer Premises Equipment)
 * Or another router, such as the venue ADSL router.
 * It must be configured as a DHCP client, obtaining its IPv4 address and DNS server from the connected network.

 **A LAN interface** (Local Area Network). This interface MUST be configured to:

 * Provide the Default IPv4 gateway in a private IPv4 subnet that is different to any private subnets between it and the ISP CPE
 * Provide DHCP services to connected clients
 * Provide DNS services to connected clients
 * Provide Network Address Translation (NAT) for all outgoing traffic directed to the WAN interface.

If an improper routing configuration is detected, openNDS will shut down.

Installing on OpenWrt
*********************

* Have a router working with OpenWrt. At the time of writing, openNDS has been tested with OpenWrt 18.06.x, 19.7.x 21.2.x and Snapshot.

* OpenWrt version 19.07.x or less requires theoogle.com updated libmicrohttpd-no-ssl_0.9.71-1 package (or higher version) from Openwrt 21.2.x or higher.

* openNDS may or may not work on older versions of OpenWrt.

* Make sure your router is working correctly before you try to install  openNDS. In particular, make sure your DHCP daemon is serving addresses on the interface that openNDS will manage.

  The default interface is br-lan but can be changed to any LAN interface by editing the /etc/config/opennds file.

* To install openNDS on 21.3.x or higher, you may use the OpenWrt Luci web interface or alternatively, ssh to your router and run the command:

    ``opkg update``

  followed by

    ``opkg install opennds``

* To install on 19.7.x or lower, download libmicrohttpd-no-ssl_0.9.71-1 or higher to the /tmp directory on your router. Then install using the command:
    ``opkg install <packagename> --force-reinstall``

* Similarly, download openNDS v9.0.0 or higher to /tmp, again installing using the command:
    ``opkg install <packagename> --force-reinstall``

* openNDS is enabled by default and will start automatically on reboot or can be started and stopped manually.

* If the interface that you want openNDS to manage is not br-lan,
  edit /etc/config/opennds and set GatewayInterface.

* To start openNDS, run the following, or just reboot the router:

    ``service opennds start``

* To test the installation, connect a client device to the interface on your router that is managed by openNDS (for example, connect to the router's wireless lan).

 Most client device operating systems and browsers support Captive Portal Detection (CPD) and the operating system or browser on that device will attempt to contact a pre defined port 80 web page.

 CPD will trigger openNDS to serve the default splash page where you can click or tap Continue to access the Internet.

 See the Authentication section for details of setting up a proper authentication process.

 If your client device does not display the splash page it most likely does not support CPD.

 You should then manually trigger openNDS by trying to access a port 80 web site (for example, http://gnome.org is a good choice).

* To stop openNDS:

    ``service opennds stop``

* To uninstall openNDS:

    ``opkg remove opennds``

Generic Linux
*************

openNDS and libmicrohttps-ssl can be compiled in place for most distributions of Linux.

To compile libmicrohttpd and openNDS, see the chapter "How to Compile and install openNDS".
