Walled Garden and Block Lists
#############################

From opennDS v10.2.0 onwards, it is possible to configure both Walled Garden (allowing specified FQDN access for Preauthenticated users) and Block Lists (blocking specified FQDN access for authenticated users). The following documentation applies to walled Garden, but configuring a Block List is very similar. (see Configuration Options in this documentation)

Manual Walled Garden
********************

Preauthenticated clients are, by default, blocked from all access to the Internet.

Access to certain web sites can be allowed. For example, clients will automatically be granted access to an external FAS server for the purpose of authentication.

Access to other web sites may be manually granted so clients can be served content such as news, information, advertising, etc. This is achieved in openNDS by allowing access to the IP address of the web sites as required.

A set of such web sites is referred to as a Walled Garden.

Note that standard unencrypted HTTP port (TCP port 80) is used for captive portal detection (CPD) and 
access to external websites should use HTTPS (TCP port 443) for security.
It is still possible to allow TCP port 80 by using Autonomous Walled Garden approach.

Autonomous Walled Garden
************************

Granting access by specifying the site IP address works well in the simplest of cases but the administrative overhead can rapidly become very difficult, for example with social media sites that load balance high volumes of traffic over many possible IP addresses.

In addition, the IP address of any web site may change.

Rather than using IP addresses, a much more efficient method of granting access would be by using the Fully Qualified Domain Name (FQDN) of each site and have a background process populate a database of allowed IP addresses.

openNDS supports Autonomous Walled Garden by means of a simple configuration option. This does have additional dependencies but these only apply if the Autonomous Walled Garden is enabled.

OpenWrt Walled Garden
*********************

The additional dependencies are the ipset (openWrt 23.03.x and earlier) and dnsmasq-full packages.
Install these by running the following commands:

.. code::

 opkg update
 opkg install ipset (not required for OpenWrt 23.05.x and onwards)
 opkg remove dnsmasq
 opkg install dnsmasq-full

Configure as follows:

.. code::

 uci add_list opennds.@opennds[0].walledgarden_fqdn_list='<fqdn1> <fqdn2> <fqdn3> [......] <fqdnN>'

where <fqdn1> to <fqdnN> are the fully qualified domain names of the URLs you want to use to populate the ipset.

eg. For Facebook use facebook.com and fbcdn.net as fqdn1 and fqdn2

In addition you can limit access to the Walled Garden to a list of IP ports:

.. code::

 uci add_list opennds.@opennds[0].walledgarden_port_list='<port1> <port2> <port3> [......] <portN>'

Restart openNDS to activate the Walled Garden.

To make these changes permanent (eg survive a reboot), run the command:

.. code::

 uci commit opennds

Generic Linux Walled Garden
***************************
On most generic Linux platforms the procedure is in principle the same as for OpenWrt.

The dnsmmasq-full package is a requirement, but this is generally the default.

You can check the compile time options of dnsmasq with the following command:

.. code::

 dnsmasq --version | grep -m1 'Compile time options:' | cut -d: -f2

If the returned string contains "no-nftset" then you will have to upgrade dnsmasq to the full version.

To enable Walled Garden, add the following to the /etc/config/opennds file

.. code::

 list walledgarden_port_list '<port1> <port2> <port3> [......] <portN>'


In addition you can specify a restricted set of ports for access to the walled garden by adding the line:

.. code::

 walledgarden_port_list <port1> <port2> <port3> [......] <portN>

Restart openNDS to activate the Walled Garden.

Warning When Port 80 is Enabled
*******************************

Port 80 is used by all devices to detect Captive Portals.

If port 80 is enabled in the Walled Garden configuration (either implicitly by adding to the ports list, or explicitly by not defining a ports list) then any Captive Portal Detection attempted by a device using a Walled Garden FQDN will fail.

For example if the FQDN "apple.com" is defined in the Walled Garden list, then Apple devices will fail to trigger the Portal Splash Page sequence (login page) if port 80 is enabled.