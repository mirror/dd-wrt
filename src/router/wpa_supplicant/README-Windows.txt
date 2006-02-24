wpa_supplicant for Windows
==========================

Copyright (c) 2003-2005, Jouni Malinen <jkmaline@cc.hut.fi> and
contributors
All Rights Reserved.

This program is dual-licensed under both the GPL version 2 and BSD
license. Either license may be used at your option.

This product includes software developed by the OpenSSL Project
for use in the OpenSSL Toolkit (http://www.openssl.org/)


wpa_supplicant has support for being used as a WPA/WPA2/IEEE 802.1X
Supplicant on Windows. The current port requires that WinPcap
(http://winpcap.polito.it/) is installed for accessing packets and the
driver interface. Both release versions 3.0 and 3.1 are supported.

The current port is still somewhat experimental. It has been tested
mainly on Windows XP (SP2) with limited set of NDIS drivers. In
addition, the current version has been reported to work with Windows
2000.

All security modes have been verified to work (at least complete
authentication and successfully ping a wired host):
- plaintext
- static WEP / open system authentication
- static WEP / shared key authentication
- IEEE 802.1X with dynamic WEP keys
- WPA-PSK, TKIP, CCMP, TKIP+CCMP
- WPA-EAP, TKIP, CCMP, TKIP+CCMP
- WPA2-PSK, TKIP, CCMP, TKIP+CCMP
- WPA2-EAP, TKIP, CCMP, TKIP+CCMP


Binary version
--------------

Compiled binary version of the wpa_supplicant and additional tools is
available from http://hostap.epitest.fi/wpa_supplicant/. These
binaries can be used after installing WinPcap.


Building wpa_supplicant with mingw
----------------------------------

The default build setup for wpa_supplicant is to use MinGW and
cross-compiling from Linux to MinGW/Windows. It should also be
possible to build this under Windows using the MinGW tools, but that
is not tested nor supported and is likely to require some changes to
the Makefile unless cygwin is used.


Building wpa_supplicant for cygwin
----------------------------------

wpa_supplicant can be built for cygwin by installing the needed
development packages for cygwin. This includes things like compiler,
make, openssl development package, etc. In addition, developer's pack
for WinPcap (WPdpack.zip) from
http://winpcap.polito.it/install/default.htm is needed.

.config file should enable only one driver interface,
CONFIG_DRIVER_NDIS. In addition, include directories may need to be
added to match the system. An example configuration is available in
defconfig. The library and include files for WinPcap will either need
to be installed in compiler/linker default directories or their
location will need to be adding to .config when building
wpa_supplicant.

Othen than this, the build should be more or less identical to Linux
version, i.e., just run make after having created .config file. An
additional tool, win_if_list.exe, can be built by running "make
win_if_list".


Building wpa_gui
----------------

wpa_gui uses Qt application framework from Trolltech. It can be built
with the open source version of Qt4 and MinGW. Following commands can
be used to build the binary in the Qt 4 Command Prompt:

# go to the root directory of wpa_supplicant source code
cd wpa_gui-qt4
qmake -o Makefile wpa_gui.pro
make
# the wpa_gui.exe binary is created into 'release' subdirectory


Using wpa_supplicant for Windows
--------------------------------

wpa_supplicant and wpa_cli behave more or less identically to Linux
version, so instructions in README and example wpa_supplicant.conf
should be applicable for most parts. When using access points in
"hidden SSID" mode, ap_scan=2 mode need to be used (see
wpa_supplicant.conf for more information).

Windows NDIS/WinPcap uses quite long interface names, so some care
will be needed when starting wpa_supplicant. Alternatively, the
adapter description can be used as the interface name which may be
easier since it is usually in more human-readable
format. win_if_list.exe can be used to find out the proper interface
name.

Example steps in starting up wpa_supplicant:

First, start NDIS event received, ndis_events.exe. This will be used
to collect NDIS events and send them to wpa_supplicant with a mechanism that is easier to process with the current model of one thread in an event loop.

# ./win_if_list.exe
ifname: \Device\NPF_GenericNdisWanAdapter
description: Generic NdisWan adapter

ifname: \Device\NPF_{769E012B-FD17-4935-A5E3-8090C38E25D2}
description: Atheros Wireless Network Adapter (Microsoft's Packet Scheduler)

ifname: \Device\NPF_{732546E7-E26C-48E3-9871-7537B020A211}
description: Intel 8255x-based Integrated Fast Ethernet (Microsoft's Packet Scheduler)


Since the example configuration used Atheros WLAN card, the middle one
is the correct interface in this case. The interface name for -i
command line option is the full string following "ifname:". In other
words, wpa_supplicant would be started with following command:

./wpa_supplicant.exe -i'\Device\NPF_{769E012B-FD17-4935-A5E3-8090C38E25D2}' -c wpa_supplicant.conf -d

-d optional enables some more debugging (use -dd for even more, if
needed). It can be left out if debugging information is not needed.

With the alternative mechanism for selecting the interface, this
command has identical results in this case:

./wpa_supplicant.exe -iAtheros -c wpa_supplicant.conf -d


Simple configuration example for WPA-PSK:

#ap_scan=2
ctrl_interface=/var/run/wpa_supplicant
network={
	ssid="test"
	key_mgmt=WPA-PSK
	proto=WPA
	pairwise=TKIP
	psk="secret passphrase"
}

(remove '#' from the comment out ap_scan line to enable mode in which
wpa_supplicant tries to associate with the SSID without doing
scanning; this allows APs with hidden SSIDs to be used)


wpa_cli.exe can be used to interact with the wpa_supplicant.exe
program in the same way as with Linux. Note that ctrl_interface is
using UNIX domain sockets when build for cygwin, but the native build
for Windows uses UDP sockets and the contents of the ctrl_interface
configuration item is ignore for this case. Anyway, this variable has
to be included in the configuration to enable the control interface.


Starting wpa_supplicant as a Windows service
--------------------------------------------

wpa_supplicant can be started as a Windows service by using a wrapper
program that starts wpa_supplicant. If this service is set to start
during system bootup to make the network connection available before
any user has logged in, there may be a long (half a minute or so)
delay in starting up wpa_supplicant due to WinPcap needing a driver
called "Network Monitor Driver" which is started by default on demand.

To speed up wpa_supplicant start during system bootup, "Network
Monitor Driver" can be configured to be started sooner by setting its
startup type to System instead of the default Demand. To do this, open
up Device Manager, select Show Hidden Devices, expand the "Non
Plug-and-Play devices" branch, double click "Network Monitor Driver",
go to the Driver tab, and change the Demand setting to System instead.



License information for third party sotware used in this product:

  OpenSSL License
  ---------------

/* ====================================================================
 * Copyright (c) 1998-2004 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */

 Original SSLeay License
 -----------------------

/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */
