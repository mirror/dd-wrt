#!/bin/sh
# Copyright (C) The openNDS Contributors 2004-2022
# Copyright (C) BlueWave Projects and Services 2015-2023
# This software is released under the GNU GPL license.

###############################################################
# This script configures and installs a working local fas-aes,
# using the OpenWrt default uhttpd web server
###############################################################

###############################################################
# uhttpd config:
###############################################################

# Disable uhttpd from listening on port 80 (both ipv4 and ipv6)
uci del_list uhttpd.main.listen_http='0.0.0.0:80'
uci del_list uhttpd.main.listen_http='[::]:80'

# Enable uhttpd listening on port 2080
uci add_list uhttpd.main.listen_http='0.0.0.0:2080'

# Add support for php-cgi
uci add_list uhttpd.main.interpreter='.php=/usr/bin/php-cgi'

# Commit the changes
uci commit uhttpd

###############################################################
# opennds config:
###############################################################

# Disable themespec
uci set opennds.@opennds[0].login_option_enabled='0'

# Enable FAS level
uci set opennds.@opennds[0].fas_secure_enabled='2'

# Enable fas on port 2080
uci set opennds.@opennds[0].fasport='2080'

# Path to the fas script
uci set opennds.@opennds[0].faspath='/fas/fas-aes.php'

# If you have rate limiting enabled on a router that is short of ram, you can save a lot of memory by enabling unrestricted bursting
uci set opennds.@opennds[0].upload_unrestricted_bursting='1'
uci set opennds.@opennds[0].download_unrestricted_bursting='1'

# Commit the changes
uci commit opennds

# Copy the fas script to the uhttpd webroot
# In this example we are using the default fas-aes.php script.
# If you have a custom script file, copy that instead of fas-aes.php
mkdir -p /www/fas
cp /etc/opennds/fas-aes.php /www/fas/

# Install dependencies
opkg update
opkg install php8-cgi
opkg install php8-cli
opkg install php8-mod-openssl


# Restart uhttpd
service uhttpd restart

# Restart openNDS
service opennds restart



