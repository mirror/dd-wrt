The Client Status/CPI (Captive Portal Information) Page
#######################################################

If the client is redirected by the CPI (RFC 8910) process, this page is displayed.

This page is also accessible by any connected client at the default url:

http://status.client

**If the client is not authenticated**, then this page is also known as the Error511 page, as it serves to the client the "511 Network Authentication Required" html status code and a button is displayed to allow the client to log in.

**If the client is authenticated**, a page is served displaying the Gatewayname and the Network Zone the client device is currently using.

Default "Quick Status" and optional "Advanced Status" options can be selected.

A list of allowed quotas and current usage is displayed along with "Refresh" and "Logout" buttons.

The "Continue" button allows the client to immediately attempt to login without waiting for the client CPD to trigger.

The URL used to access this page can be changed by setting the config option gatewayfqdn.

For best results it is recommended that gatewayfqdn is set to two words separated by a single period eg in OpenWrt:

	``option gatewayfqdn 'my.status'``

 ***Disable GatewayFQDN*** by setting the option to 'disable'
 ie:

 ``option gatewayfqdn 'disable'``

 **Warning** - if enabled, services on port 80 of the gateway will no longer be accessible (eg the OpenWrt Luci AdminUI)


Custom Files and Images
***********************
Custom files and images can be included in the Status Page.
These can be used to display useful information, advertisements, logos etc..
By default the standard openNDS logo is displayed, but a venue or company logo can be displayed by simply configuring in the openNDS config.

For example, to display the OpenWrt logo, add the following line to the openNDS config:

``list fas_custom_images_list 'logo_png=https://openwrt.org/_media/logo.png'``

Custom downloaded images are stored in ``/etc/opennds/htdocs/ndsremote/``

and custom files are stored in ``/etc/opennds/htdocs/ndsdata/``

Both image files and data files are refreshed according to the openNDS remotes_refresh_interval option. For example to refresh every 10 minutes, include the config:

``option remotes_refresh_interval '10'``

This means that the remote source can be set up to provide a different file at every refresh interval, particularly useful for providing dynamic news or advertising in a custom status page.

Custom Status Page
******************
The default  client status page is generated dynamically by the script /usr/lib/opennds/client_params.sh

An alternate Status page script can be used by setting the configuration option "statuspath" in the config file. Ensure the alternate script file is flagged as executable.