Customising openNDS
########################

After initial installation, openNDS (NDS) should be working in its most basic mode and client Captive Portal Detection (CPD) should pop up the default Click to Continue page.

Before attempting to customise NDS you should ensure it is working in this basic mode.

NDS reads its configuration file when it starts up but the location of this file varies depending on the operating system.

As NDS is a package that requires hardware configured as an IP router, perhaps the most common installation is using OpenWrt. However NDS can be compiled to run on most Linux distributions, the most common being Debian or one of its popular variants (eg Raspbian).

If NDS is working in the default, post installation mode, then you will have met the NDS dependencies and can now move on to your own customisation.

Rules for Customised Splash Pages
*********************************

It should be noted when designing a custom splash page that for security reasons many client device CPD implementations:

 * Immediately close the browser when the client has authenticated.

 * Prohibit the use of href links.

 * Prohibit downloading of external files (including .css and .js, even if they are allowed in NDS firewall settings).

 * Prohibit the execution of javascript.

The Configuration File
**********************

In OpenWrt, or operating systems supporting UCI (such as LEDE) the configuration is kept in the file:

  ``/etc/config/opennds``


In other operating systems the configuration is kept in the file:

  ``/etc/opennds/opennds.conf``

Both of these files contain a full list of options and can be edited directly. A restart of NDS is required for any changes to take effect.

In the case of OpenWrt though, once you are confident in your configuration requirements you can use UCI to read and set any of the configuration options using simple commands, making this very convenient if making changes from scripts, such as those you may write to use with Binauth and FAS.

For example, to list the full configuration, at the command line type:

.. code-block:: sh

  uci show opennds

To display the Gateway Name, type:

.. code-block:: sh

  uci get opennds.@opennds[0].gatewayname

To set the Gateway Name to a new value, type:

.. code-block:: sh

  uci set opennds.@opennds[0].gatewayname='my new gateway'

To add a new firewall rule allowing access to another service running on port 8888 on the router, type:

.. code-block:: sh

 uci add_list opennds.@opennds[0].users_to_router='allow
 tcp port 8888'

Finally you must tell UCI to commit your changes to the configuration file:

.. code-block:: sh

  uci commit opennds

API Informational files
***********************

A number of API informational files are created on startup and contain useful system information to be used by customisation scripts.

These files are located on the logfile mountpoint in the ndscids folder.
Typically the logfile mountpoint is /tmp or /run depending on the Linux distribution.

The "ndsinfo" file
==================

This file contains the runtime values of gatewayname, gatewayaddress, gatewayfqdn and the openNDS version.

The "authmonargs" File
======================

This file contains the runtime values of the url of the remote FAS server, the gateway hash of the current instance of openNDS, and the phpcli command name on the openNDS system.

The "heartbeat" file
====================

This file contains the timestamp of the last openNDS heartbeat. The file is re-written at the start of every new checkinterval cycle.

The Legacy Click and Go Splash Page
************************************

*The legacy Click to Continue html splash page was deprecated and disabled at v8.0.0.*

**From v 9.0.0 it has been removed entirely.**

Dynamic Splash Pages
********************

Default Dynamic Click to Continue
=================================

The pre-installed dynamic click to continue page sequence is enabled by default using the ThemeSpec "theme_click-to-continue".
The configuration default is equivalent to setting:

``option login_option_enabled '1'``

It generates a Click to Continue page followed by Thankyou and Landing pages.

User clicks on "Continue" are recorded in the log file /[tmpfs_dir]/ndslog/ndslog.log

Where [tmpfs_dir] is the operating system "temporary" tmpfs mount point.
This will be  /tmp /run or /var and is automatically detected.

Details of how the script works are contained in comments in the script theme_click-to-continue.sh


Pre-Installed dynamic User/email Login page sequence
====================================================

The pre-installed dynamic login page is enabled by setting option:

``option login_option_enabled '2'``

It generates a login page asking for username and email address.
User logins are recorded in the log file /[tmpfs_dir]/ndslog/ndslog.log

Where [tmpfs_dir] is the operating system "temporary" tmpfs mount point.
This will be  /tmp /run or /var and is automatically detected.

Details of how the script works are contained in comments in the script theme_user-email-login.sh


Custom Dynamic ThemeSpec Pages
==============================
Custom ThemeSpec page sequences can be added by setting option:

``option login_option_enabled '3'``

and option

``option themespecpath '/path/to/themespec_script'``

Two additional ThemeSpec files are included as examples:

/usr/lib/opennds/theme_click-to-continue-custom-placeholders.sh

and

/usr/lib/opennds/theme_user-email-login-custom-placeholders.sh

Both these also require custom parameter, variable, image and file lists:

``list fas_custom_parameters_list 'logo_message=openNDS:%20Perfect%20on%20OpenWrt!'``

``list fas_custom_parameters_list 'banner1_message=BlueWave%20-%20Wireless%20Network%20Specialists'``

``list fas_custom_parameters_list 'banner2_message=HMS%20Pickle'``

``list fas_custom_parameters_list 'banner3_message=SeaWolf%20Cruiser%20Racer'``

``list fas_custom_variables_list 'input=phone:Phone%20Number:text;postcode:Home%20Post%20Code:text'``

``list fas_custom_images_list 'logo_png=https://openwrt.org/_media/logo.png'``

``list fas_custom_images_list 'banner1_jpg=https://raw.githubusercontent.com/openNDS/openNDS/9.0.0/resources/bannerbw.jpg'``

``list fas_custom_images_list 'banner2_jpg=https://raw.githubusercontent.com/openNDS/openNDS/9.0.0/resources/bannerpickle.jpg'``

``list fas_custom_images_list 'banner3_jpg=https://raw.githubusercontent.com/openNDS/openNDS/9.0.0/resources/bannerseawolf.jpg'``

``list fas_custom_files_list 'advert1_htm=https://raw.githubusercontent.com/openNDS/openNDS/9.0.0/resources/bannerpickle.htm'``

Once configured these two example ThemeSpec scripts will download custom image files, a custom html file and inject custom user input forms for phone number and home postcode.

Other Custom Designs
====================
Custom designed dynamically generated ThemeSpec pages are supported using FAS and PreAuth. For details see the FAS and PreAuth chapters.

