Configuration Options
#####################

Enable openNDS
**************

Set to 0 to disable opennds

``option enabled 1``

Use deprecated generic configuration file
******************************************

Use of this setting is not recommended.

``option config '/etc/opennds/opennds.conf'``

Enable debug output (0-3)
*************************

Default: 1

Level0
------
Silent (only initial startup, LOG_ERR and LOG_EMERG messages will be seen, otherwise there will be no logging.)

Level 1
-------
LOG_ERR, LOG_EMERG, LOG_WARNING and LOG_NOTICE (this is the default level).

Level 2
-------
debuglevel 1  + LOG_INFO

Level 3
-------
debuglevel 2 + LOG_DEBUG

``option debuglevel '1'``

Firewall Restart hook
*********************

Set to 0 to disable hook that makes opennds restart when the firewall restarts.

This hook is needed as a restart of Firewall overwrites opennds iptables entries.

``option fwhook_enabled '1'``

Ndsctl Socket Access
********************

Set the socket name to use for ndsctl socket access, relative to the tmpfs mountpoint.

Any directory/folder specified must exist.
Default: ndsctl.sock (Do not add a leading "/")

Full default socket path would be /tmp/ndsctl.sock in OpenWrt
In the following example, the socket path would be /tmp/sockets/ndsctl.sock

Example:

``option ndsctlsocket 'sockets/ndsctl.sock'``

Local Log Mountpoint
********************

Default: router's volatile tmpfs storage eg on OpenWrt '/tmp'

Local logging can be directed to any storage accessible to the router eg USB drive, SSD etc

**WARNING** - you cannot use the router's built in flash storage as this would cause
excessive wear and eventual flash failure

Example:

``option log_mountpoint '/logs'``

Maximum number of Local Log Entries
***********************************

Set the maximum number of local log entries to be kept.
Default 100

Minimum value 0 (no limit)

Maximum value - limited only by free storage space on the logging mountpoint

If set to '0' there is no limit

This is the maximum number of local log entries allowed before log rotation begins

Both ThemeSpec and BinAuth log locally if they are enabled

**WARNING** - local logging is by default written to the tmpfs volatile storage

If this option were to be set too high the router could run out of tmpfs storage and/or free RAM

Non-volatile storage, such as a USB storage device may be defined using the log_mountpoint option

Example:

``option max_log_entries '1000'``

Login Option
************

Default: 1

Integer value sent to PreAuth script as login mode

openNDS comes preconfigured for three basic modes of operation

Mode 0
------
If FAS is not enabled, then this functions as mode 1

Mode 1
------
Default Dynamic Click to Continue

The pre-installed dynamic login page is enabled by setting option login_option_enabled = '1'

It generates a Click to Continue page followed by an info/advertising page.

User clicks on “Continue” are recorded in the log file /[tmpfs_dir]/ndslog/ndslog.log

Mode 2
------
Username/Emailaddress Dynamic Login

The pre-installed dynamic login page is enabled by setting option login_option_enabled = '2'

It generates a login page asking for username and email address followed by an info/advertising page.

User logins are recorded in the log file /[tmpfs_dir]/ndslog/ndslog.log

Mode 3
------
Use Theme defined in ThemeSpec path (option themespec_path)

`option login_option_enabled '1'`

Allow Preemptive Authentication
*******************************

Default: 0 - Disabled

Enable by setting to 1

This allows the ndsctl utility to preemptively authorise **connected** clients that have not entered the preauthenticated state.

This is useful for example with IoT devices that do not have CPD (captive portal detection)

or for a FAS to manage inter-captive-portal roaming by making use of a centralised database of client validations.

Example:

``option allow_preemptive_authentication '1'``

ThemeSpec Path
**************

Default: None

Required when when login_option_enabled is set to '3'

Note: /usr/lib/opennds/theme_click-to-continue.sh is used for login_option_enabled '1'

and:  /usr/lib/opennds/theme_user_email_login.sh is used for login_option_enabled '2'

Sets the ThemeSpec file path to be used when login_option_enabled '3'

The ThemeSpec script makes use of lists of custom parameters, custom variables, custom image urls and custom files and is used to generate the dynamic splash page sequence.

The ThemeSpec file will normally reside in /usr/lib/opennds/ but can be anywhere accessible to openNDS.

The file must be flagged as executable and have the correct shebang for the default shell.

``option themespec_path '/usr/lib/opennds/<filename>'``


Define Custom Parameters
************************

Custom parameters are sent as fixed values to FAS

Default None

Custom Parameters listed in the form of param_name=param_value

param_name and param_value must be urlencoded if containing white space or single quotes

eg replace spaces with %20 - replace single quotes with %27

Parameters should be configured one per line to prevent possible parsing errors.

eg:

``list fas_custom_parameters_list '<param_name1=param_value1>'``

``list fas_custom_parameters_list '<param_name2=param_value2>'``

etc.

Configuration for custom parameters in the installed ThemeSpec Files
--------------------------------------------------------------------

The installed ThemeSpec files are:

theme_click-to-continue-custom-placeholders

and

theme_user-email-login-custom-placeholders

``list fas_custom_parameters_list 'logo_message=openNDS:%20Perfect%20on%20OpenWrt!'``

``list fas_custom_parameters_list 'banner1_message=BlueWave%20-%20Wireless%20Network%20Specialists'``

``list fas_custom_parameters_list 'banner2_message=HMS%20Pickle'``

``list fas_custom_parameters_list 'banner3_message=SeaWolf%20Cruiser%20Racer'``

Define Custom Variables
***********************

Custom Variables are used by FAS to dynamically collect information from clients

Default None

Custom Variables are listed in the form of var_name=var_type

"var_name" and "var_type" must be urlencoded if containing white space or single quotes

eg replace spaces with %20 - replace single quotes with %27

Variables should be configured one per line to prevent possible parsing errors.

eg:

``list fas_custom_variables_list '<var_name1=var_type1>'``

``list fas_custom_variables_list '<var_name2=var_type2>'``

etc.

FAS Generic Variables
---------------------
A custom FAS or ThemeSpec must be written to make use of FAS Generic Variables

eg:

``list fas_custom_variables_list 'membership_number=number'``

``list fas_custom_variables_list 'access_code=password'``

ThemeSpec Dynamically generated Form Fields
-------------------------------------------

ThemeSpec scripts can dynamically generate Form Field html and inject into the dynamic splash page sequence.

This is achieved using a SINGLE line containing the keyword "input", in the form: fieldname:field-description:fieldtype

Numerous fields can be defined in this single "input=" line, separated by a semicolon (;).

Configuration for custom variables in the installed ThemeSpec Files
-------------------------------------------------------------------

theme_click-to-continue-custom-placeholders

and

theme_user-email-login-custom-placeholders

This example inserts Phone Number and Home Post Code fields:

``list fas_custom_variables_list 'input=phone:Phone%20Number:text;postcode:Home%20Post%20Code:text'``

Define Custom Images
********************

Custom Images are served by a local FAS where required in dynamic portal pages

Default None

Custom images will be copied from the URL to the openNDS router

Custom Images are listed in the form of image_name_type=image_url

image_name and image_url must be urlencoded if containing white space or single quotes

The image url must begin with http:// https:// or file://

Images should be configured one per line to prevent possible parsing errors.

``list fas_custom_images_list '<image_name1_[type]=image_url1>'``

``list fas_custom_images_list '<image_name2_[type]=image_url2>'``

etc.

"type" can be any recognised image file extension eg jpg, png, ico, etc.

Configuration for custom images in the installed ThemeSpec Files
----------------------------------------------------------------

theme_click-to-continue-custom-placeholders

and

theme_user-email-login-custom-placeholders

``list fas_custom_images_list 'logo_png=https://openwrt.org/_media/logo.png'``

``list fas_custom_images_list 'banner1_jpg=https://raw.githubusercontent.com/openNDS/openNDS/v9.0.0/resources/bannerbw.jpg'``

``list fas_custom_images_list 'banner2_jpg=https://raw.githubusercontent.com/openNDS/openNDS/v9.0.0/resources/bannerpickle.jpg'``

``list fas_custom_images_list 'banner3_jpg=https://raw.githubusercontent.com/openNDS/openNDS/v9.0.0/resources/bannerseawolf.jpg'``

Define Custom Files
*******************

Custom Files are served by a local FAS where required in dynamic portal pages

Default None

Custom files will be copied from the URL to the openNDS router

Images should be configured one per line to prevent possible parsing errors.

Custom files are listed in the form of file_name_type=file_url

file_name and file_url must be urlencoded if containing white space or single quotes

The file url must begin with http:// https:// or file://

``list fas_custom_files_list '<file_name1_[type]=file_url1>'``

``list fas_custom_files_list '<file_name2_[type]=file_url2>'``

"type" can be any recognised file extension that can be used to display web content eg txt, htm etc.

URLs using the file:// protocol must point to a valid mountpoint accessible to openNDS, for example a usb storage device.

Configuration for custom files in the installed ThemeSpec Files
----------------------------------------------------------------

theme_click-to-continue-custom-placeholders

and

theme_user-email-login-custom-placeholders

``list fas_custom_files_list 'advert1_htm=https://raw.githubusercontent.com/openNDS/openNDS/v9.0.0/resources/bannerpickle.htm'``


Set refresh interval for downloads
**********************************

Set refresh interval for downloaded remote files (in minutes)

Default 0

A setting of 0 (zero) means refresh is disabled.

This is useful for providing automated refreshing of informational or advertising content. Should the remote resources become unavailable, current versions will continue to be used.

Example, set to twelve hours (720 minutes):

``option remotes_refresh_interval '720'``

Use outdated libmicrohttpd (MHD)
********************************

Default 0 (Disabled)

Warning, enabling this *may* cause instability or in the worst case total failure - it would be better to upgrade MHD.

**Use at your own risk.**

Older versions of MHD use an older version of the MHD API and may not run correctly or fail.

Older versions of MHD convert & and + characters to spaces when present in form data. This can make a PreAuth or BinAuth impossible to use for a client if form data contains either of these characters eg. in username or password.

*There may well be other issues with older versions.*

MHD versions earlier than 0.9.71 are detected.

If this option is set to 0 (default), NDS will terminate if MHD is earlier than 0.9.71

If this option is set to 1, NDS will attempt to start and log an error.

``option use_outdated_mhd '1'``

Maximum Page Size to be served by MHD
*************************************

Default 10240 bytes

Minimum value 1024 bytes

Maximum - limited only by free RAM in the router

This sets the maximum number of bytes that will be served per page by the MHD web server.

Setting this option is useful:

	1. To reduce memory requirements on a resource constrained router
	2. To allow large pages to be served where memory usage is not a concern

Example:

``option max_page_size '4096'``

MHD Unescape callback
*********************

Default 0 (Disabled)

MHD has a built in unescape function that urldecodes incoming queries from browsers.

This advanced option allows an external unescape script to replace the built in decoder.

The script must be named unescape.sh, be present in /usr/lib/opennds/ and be executable.

A very simple standard unescape.sh script is installed by default.

Set to 1 to enable this option, 0 to disable.

Example:

``option unescape_callback_enabled '1'``

Set the MHD WebRoot
*******************

Default: /etc/opennds/htdocs

The local path where the system css file, and other static page content resides.

ie. Serve the file splash.css from this directory

Example:

``option webroot '/etc/opennds/htdocs'``

Set the GatewayInterface
************************

Default br-lan

Use this option to set the device opennds will bind to.

The value may be an interface section in /etc/config/network or a device name such as br-lan.

The selected interface must be allocated an IPv4 address.

In OpenWrt this is normally br-lan, in generic Linux it might be wlan0

``option gatewayinterface 'br-lan'``

Set the GatewayPort
*******************

Default: 2050

openNDS's own http server (MHD) uses the gateway address as its IP address.

This option sets the port it listens to.

Example:

``option gatewayport '2080'``

Set the GatewayName
*******************

Default: openNDS

gatewayname is used as an identifier for the instance of openNDS

It is displayed on the default splash page sequence for ThemeSpec and the example php scripts.

It is particularly useful in the case of a single remote FAS server that serves multiple openNDS sites, allowing the FAS to customise its response for each site.

Note: The single quote (or apostrophe) character ('), cannot be used in the gatewayname.

If it is required, use the htmlentity &#39; instead.

For example:

``option gatewayname 'Bill's WiFi'`` is invalid.

Instead use:

``option gatewayname 'Bill&#39;s WiFi'``

Example:

``option gatewayname 'OpenWrt openNDS'``

Serial Number Suffix Enable
***************************

Appends a serial number suffix to the gatewayname string.

openNDS constructs a serial number based on the router mac address and adds it to the gatewayname

Default 1 (enabled)

To disable, set to 0

Example:

``option enable_serial_number_suffix '0'``

Set GatewayFQDN
***************

Default: status.client

This is the simulated FQDN used by a client to access the Client Status Page

If not set, the Status page can be accessed at: http://gatewayaddress:gatewayport/

Warning - if set, services on port 80 of the gateway will no longer be accessible (eg Luci AdminUI)

By default, the Error511/Status page will be found at http://status.client/ by a redirection of port 80 to http://gatewayaddress:gatewayport/

Disable GatewayFQDN by setting the option to 'disable'

ie:

``option gatewayfqdn 'disable'``

Alternate Useful Example:

``option gatewayfqdn 'login.page'``

Set StatusPath
**************

Default: /usr/lib/opennds/client_params.sh

This is the script used to generate the GatewayFQDN client status page.

Example:

``option statuspath '/mycustomscripts/custom_client_params.sh'``

Set MaxClients
**************

Default 250

The maximum number of clients allowed to connect.

**This should be less than or equal to the number of allowed DHCP leases.** set for the router's dhcp server.

Example:

``option maxclients '500'``

Client timeouts in minutes
**************************

Preauthidletimeout
------------------

Default 30

This is the time in minutes after which a client is disconnected if not authenticated.

ie the client has not attempted to authenticate for this period.

Example:

``option preauthidletimeout '60'``

Authidletimeout
---------------

Default 120

This is the time in minutes after which an idle client is disconnected
ie the client has not used the network access for this period

Example:

``option authidletimeout '60'``

Session Timeout
---------------

Default 1440 minutes (24 hours).

This is the interval after which clients are forced out (a value of 0 means never).

Clients will be deauthenticated at the end of this period.

Example: Set to 20 hours (1200 minutes).

``option sessiontimeout '1200'``

Set the Checkinterval
*********************

The interval in seconds at which openNDS checks client timeouts, quota usage and runs watchdog checks.

Default 15 seconds (one quarter of a minute).

Example: Set to 30 seconds.

``option checkinterval '30'``

Set Rate Quotas
***************

Defaults 0

Integer values only.

.. note::
 Upload means *to* the Internet, download means *from* the Internet.

If the client average data rate exceeds the value set here, the client will be rate limited.

Values are in kb/s.

If set to 0, there is no limit.

Quotas and rates can also be set by FAS via Authmon Daemon, ThemeSpec scripts, BinAuth, and ndsctl auth. Values set by these methods, will override values set in the config file.

Rates:

``option uploadrate '200'``

``option downloadrate '800'``

Set Bucket Ratio
****************

Default 10

Upload and Download bucket ratios can be defined.

Allows control of upload rate limit threshold overrun per client.

Used in conjunction with MaxDownloadBucketSize and MaxUploadBucketSize.

Facilitates calculation of a dynamic "bucket size" or "queue length" (in packets) to be used for buffering upload and download traffic to achieve rate restrictions defined in this config file or by FAS for individual clients.

If a bucket becomes full, packets will overflow and be dropped to maintain the rate limit.

To minimise the number of dropped packets the bucket ratio can be increased whilst still maintaining the configured rate restriction.

***CAUTION*** Large values may consume large amounts of memory per client.

If the client's average rate does not exceed its configured value within the ratecheck window interval (See RateCheckWindow option), no memory is consumed.

If the rate is set to 0, the Bucket Ratio setting has no meaning and no memory is consumed.

Examples:

	`option upload_bucket_ratio '1'`

	`option download_bucket_ratio '5'`


MaxDownloadBucketSize
*********************

Default: 250

Allows control over download rate limiting packet loss at the expense of increased latency.

***CAUTION*** Large values may consume large amounts of memory per client.

Allowed Range 5 to 10000

Example:

`option max_download_bucket_size '100'`

MaxUploadBucketSize
*******************

Default 250

Allows control over upload rate limiting packet loss at the expense of increased latency.

***CAUTION*** Large values may consume large amounts of memory per client.

Allowed Range 5 to 10000

Example:

`option max_upload_bucket_size '100'`

DownLoadUnrestrictedBursting
****************************

Default 0

Enables / disables unrestricted bursting

Setting to 0 disables

Setting to 1 enables

If enabled, a client is allowed unrestricted bursting until its average download rate exceeds the set download rate threshold.

Unrestricted bursting minimises memory consumption at the expense of potential short term bandwidth hogging.

If disabled, a client is not allowed unrestricted bursting.

Example:

`option download_unrestricted_bursting '1'`

UpLoadUnrestrictedBursting
**************************

Default 0

Enables / disables unrestricted bursting

Setting to 0 disables

Setting to 1 enables

If enabled, a client is allowed unrestricted bursting until its average upload rate exceeds the set upload rate threshold.

Unrestricted bursting minimises memory consumption at the expense of potential short term bandwidth hogging.

If disabled, a client is not allowed unrestricted bursting.

Example:

`option upload_unrestricted_bursting '1'`

Set RateCheckWindow
*******************

Default 2

The client data rate is calculated using a moving average.

This allows clients to burst at maximum possible rate, only rate limiting if the moving average exceeds the specified upload or download rate.

The moving average window size is equal to ratecheckwindow times checkinterval (seconds).

Example: Set to 3 checkinterval periods:

``option ratecheckwindow '3'``

Disable Rate Quotas
-------------------

All rate limits can be globally disabled by setting this option to 0 (zero).

Example: Disable all rate quotas for all clients, overriding settings made in FAS via Authmon Daemon, ThemeSpec scripts, BinAuth, and ndsctl auth:

``option ratecheckwindow '0'``

Set Volume Quotas
*****************

Defaults 0

Integer values only.

Values are in kB.

If set to 0, there is no limit.

If the client data quota exceeds the value set here, the client will be deauthenticated.

The client by default may re-authenticate. It is the responsibility of the FAS (whether Themespec, other local or remote) to restrict further authentication of the client if so desired.

Example:

``option uploadquota '1000'``

``option downloadquota '10000'``


Enable BinAuth Support.
***********************

Default disabled

BinAuth enables POST AUTHENTICATION PROCESSING and and is useful in particular when a FAS is configured remotely.

If set, a BinAuth program or script is triggered by several possible methods and is called with several arguments on both authentication and deauthentication.

Possible methods
----------------

Authentication:

	"auth_client": Request for authentication received from the captive portal splash page.

	"client_auth": Acknowledgement that Client was authenticated via this script.

	"ndsctl_auth": Client was authenticated by ndsctl auth command.

Deauthentication:

	"client_deauth": Client deauthenticated by the client via captive portal splash page.

	"idle_deauth": Client was deauthenticated because of inactivity.

	"timeout_deauth": Client was deauthenticated because the session timed out.

	"ndsctl_deauth": Client was deauthenticated by ndsctl deauth command.

	"uprate_deauth": Client was deauthenticated because its average upload rate exceeded the allowed value.

	"downrate_deauth": Client was deauthenticated because its average download rate exceeded the allowed value.

	"upquota_deauth": Client was deauthenticated because its upload quota exceeded the allowed value.

	"downquota_deauth": Client was deauthenticated because its download quota exceeded the allowed value.

	"shutdown_deauth": Client was deauthenticated by openNDS terminating.

A fully functional BinAuth script is pre-installed and provides local logging of client activity.

This is enabled by the following option:

``option binauth '/usr/lib/opennds/binauth_log.sh'``

Set Fasport
***********

Default: Not set.

This is the Forwarding Authentication Service (FAS) port number.

Redirection is changed to the IP port of a FAS (provided by the system administrator).

.. note::
 If FAS is running locally (ie fasremoteip is NOT set), port 80 cannot be used.

Typical Remote Shared Hosting Example:

``option fasport '80'``

Typical Locally Hosted example (ie fasremoteip not set):

``option fasport '2090'``

Set Fasremotefqdn
*****************

Default: Not set.

If set, this is the remote fully qualified domain name (FQDN) of the FAS.

The protocol must NOT be prepended to the FQDN (ie http:// or https://).

To prevent CPD or browser security errors NDS prepends the required http:// or https:// before redirection, depending upon the fas_secure_enabled option.

If set, DNS MUST resolve fasremotefqdn to be the same ip address as fasremoteip.

Remote Shared Hosting
---------------------

Typical Remote Shared Hosting Example (replace this with your own FAS FQDN):

``option fasremotefqdn 'onboard-wifi.net'``

CDN (Content Delivery Network) hosted server
--------------------------------------------

For a CDN (Content Delivery Network) hosted server, the configuration is the same as for Remote Shared Hosting but fasremotefqdn must also be added to the Walled Garden list of FQDNs

Set the Fasremoteip
*******************

Default: GatewayAddress (the IP of NDS)

If set, this is the remote ip address of the FAS.

Typical Remote Shared Hosting Example (replace this with your own remote FAS IP):

``option fasremoteip '46.32.240.41'``

Set the Faspath
***************

Default: /

This is the path from the FAS Web Root to the FAS login page (not the file system root).

In the following examples, replace with your own values for faspath:

	Typical Remote Shared Hosting Example (if fasremotefqdn is not specified):

		``option faspath '/remote_host_fqdn/fas/fas-hid.php'``

	Typical Remote Shared Hosting Example (ie BOTH fasremoteip AND fasremotefqdn set):

		``option faspath '/fas/fas-hid.php'``

	Typical Locally Hosted Example (ie fasremoteip not set):

		``option faspath '/fas/fas-hid.php'``

Set the Faskey
**************

Default: 1234567890

A key phrase for NDS to encrypt the query string sent to FAS.

Can be any text string with no white space.

Option faskey must be pre-shared with FAS. (It is automatically pre-shared with Themespec files)

``option faskey 'mysecretopenNDSfaskey'``

Set Security Level: fas_secure_enabled
**************************************

Default: 1

Level set to "0"
----------------
	* The FAS is enforced by NDS to use http protocol.

	* The client token is sent to the FAS in clear text in the query string of the redirect along with authaction and redir.

	Note: This level is insecure and can be easily bypassed

Level set to "1"
----------------
	* The FAS is enforced by NDS to use http protocol.
	* The client token will be hashed and sent to the FAS along with other relevant information in a base 64 encoded string

	FAS must return the sha256sum of the concatenation of hid (the hashed original token), and faskey to be used by openNDS for client authentication.

Level set to "2"
----------------
	* The FAS is enforced by NDS to use http protocol.

	* The parameters clientip, clientmac, gatewayname, hid(the hashed original token), gatewayaddress, authdir, originurl and clientif

	* are encrypted using faskey and passed to FAS in the query string.

	* The query string will also contain a randomly generated initialization vector to be used by the FAS for decryption.

	* The cipher used is "AES-256-CBC".

	* The "php-cli" package and the "php-openssl" module must both be installed for fas_secure level 2 and 3. openNDS does not depend on this package and module, but will exit gracefully not installed when this level is set.

	* The FAS must use the query string passed initialisation vector and the pre shared fas_key to decrypt the query string.

An example FAS level 2 php script (fas-aes.php) is included in the /etc/opennds directory and also supplied in the source code.

Level set to "3"
----------------
	* The FAS is enforced by NDS to use https protocol.

	* Level 3 is the same as level 2 except the use of https protocol is enforced for FAS.

	* In addition, the "authmon" daemon is loaded.

	* Level 3 allows the external FAS, after client verification, to effectively traverse inbound firewalls and address translation to achieve NDS authentication without generating browser security warnings or errors.

An example FAS level 3 php script (fas-aes-https.php) is included in the /etc/opennds directory and also supplied in the source code.

Note: Option faskey must be pre shared with the FAS script in use (including any ThemeSpec local file) if fas secure is set to levels 1, 2 and 3.

Example:

``option fas_secure_enabled '3'``

Set NAT Traversal Poll Interval
*******************************

Sets the polling interval for NAT Traversal in seconds

Default 10 seconds

Allowed values between 1 and 60 seconds inclusive

Defaults to 10 seconds if set outside this range

Effective only when option fas_secure_enabled is set to 3

Example:

``option nat_traversal_poll_interval '5'``

Set PreAuth
***********

Default Not set, or automatically set by "option login_option_enabled".

PreAuth support allows FAS to call a local program or script with html served by the built in NDS web server.

If the option is set, it points to a program/script that is called by the NDS FAS handler.

All other FAS settings will be overidden.

Example:

``option preauth '/path/to/myscript/myscript.sh'``

Access Control For Authenticated Users
**************************************

Block Access For Authenticated Users (block)
--------------------------------------------

Default: None

If Block Access is specified, an allow or passthrough must be specified afterwards as any entries set here will override the access default.

Examples:

 You might want to block entire IP subnets. e.g.:

 ``list authenticated_users 'block to 123.2.3.0/24'``

 ``list authenticated_users 'block to 123.2.0.0/16'``

 ``list authenticated_users 'block to 123.0.0.0/8'``

or block access to a single IP address. e.g.:

 ``list authenticated_users 'block to 123.2.3.4'``

Do not forget to add an allow or passthrough if the default only is assumed (see Grant Access)


Grant Access For Authenticated Users (allow and passthrough)
------------------------------------------------------------

* Access can be allowed by openNDS directly, overriding the operating system firewall rules

or

* Access can be allowed by openNDS but the final decision can be passed on to the operating system firewall.

Default:

No Entry, equivalent to

 ``list authenticated_users 'passthrough all'``

Any entries set here, or above in Block Access, will override the default

Example:

Grant access overriding operating system firewall
 ``list authenticated_users 'allow all'``

Example:

Grant access to https web sites, subject to the operating system's firewall rules

 ``list authenticated_users 'passthrough tcp port 443'``

Grant access to http web sites, overriding the operating system firewall rules.

 ``list authenticated_users 'allow tcp port 80'``

Grant access to udp services at address 123.1.1.1, on port 5000, overriding the operating system firewall rules.

 ``list authenticated_users 'allow udp port 5000 to 123.1.1.1'``

Access Control For Preauthenticated Users:
******************************************

	*****IMPORTANT*****

    To support RFC8910 Captive Portal Identification

    AND to help prevent DNS tunnelling, DNS Hijacking and generally improve security,

 	*****DO NOT ALLOW ACCESS TO EXTERNAL DNS SERVICES*****

Walled Garden Access For Preauthenticated Users
***********************************************

You can allow preauthenticated users to access external services
This is commonly referred to as a Walled Garden.

A Walled Garden can be configured either:
 * Manually for known ip addresses

 * Autonomously from a list of FQDNs and ports


Manual Walled Garden configuration
----------------------------------

Manual Walled Garden configuration requires research to determine the ip addresses of the Walled Garden site(s).

This can be problematic as sites can use many dynamic ip addresses.

However, manual configuration does not require any additional dependencies (ie additional installed packages).

Note that standard unencrypted HTTP port (TCP port 80) is used for captive portal detection (CPD) and access to external websites should use HTTPS (TCP port 443) for security.

It is however, still possible to allow TCP port 80 by using the Autonomous Walled Garden approach.

Manual configuration example:

``list preauthenticated_users 'allow udp port 8020 to 112.122.123.124'``

Autonomous Walled Garden configuration
--------------------------------------

Autonomous Walled Garden configuration is activated using a list of FQDNs and Ports.

This has the advantage of discovering all ip addresses used by the Walled Garden sites.

But it does require the ipset and dnsmasq-full packages to be installed by running the following commands (on OpenWrt):

``opkg update``

``opkg install ipset``

``opkg remove dnsmasq``

``opkg install dnsmasq-full``

Configuration is then a simple matter of adding two lists as follows:
 
``list walledgarden_fqdn_list 'fqdn1 fqdn2 fqdn3 .... fqdnN'``

``list walledgarden_port_list 'port1 port2 port3 .... portN'``

Note: If walledgarden_port_list is NOT specified, then Walled Garden access is granted for all protocols (tcp, udp, icmp) on ALL ports for each fqdn specified in walledgarden_fqdn_list.

Note: If walledgarden_port_list IS specified, then:

 * Specified port numbers apply to ALL FQDN's specified in walledgarden_fqdn_list.
 * Only tcp protocol Walled Garden access is granted.


Add Facebook to the Walled Garden
---------------------------------

To add Facebook to the Walled Garden, the list entries would be:

``list walledgarden_fqdn_list 'facebook.com fbcdn.net'``

``list walledgarden_port_list '443'``


Add Paypal to the Walled Garden
-------------------------------

To add Paypal to the Walled Garden, the list entries would be:

``list walledgarden_fqdn_list 'paypal.com paypalobjects.com'``

``list walledgarden_port_list '443'``

User Access to Services On the Router
*************************************

Access is automatically granted to resources required for normal operation of the captive portal and all other access is blocked.

By default the user to router access rules are **not** passed through to the system firewall for additional processing.

Users to Router Passthrough
---------------------------

(Applies to OpenWrt only)

Default: 0 (disabled)

To enable passthrough, set to 1

``option users_to_router_passthrough '1'``

**WARNING**: Do not enable unless you know what you are doing.

*Enabling passthrough may well soft brick your router, particularly if openNDS is bound to a guest network.*

Access to the router.
---------------------

Access falls into two categories:
 * Essential
 * Optional

Essential Access
----------------

It is essential that you allow ports for DNS and DHCP (unless you have a very specific reason for doing so, **disabling these will soft brick your router!**):

``list users_to_router 'allow tcp port 53'``

``list users_to_router 'allow udp port 53'``

``list users_to_router 'allow udp port 67'``

Optional Access
---------------

You may wish to allow access to specific services on the router.

For example - Allow ports for SSH/Telnet/HTTP/HTTPS:

``list users_to_router 'allow tcp port 22'``

``list users_to_router 'allow tcp port 23'``

``list users_to_router 'allow tcp port 80'``

``list users_to_router 'allow tcp port 443'``

MAC Address Access Control List
*******************************

A list of MAC addresses can be defined that are either allowed to use the system, or are blocked.

Note: This can easily be bypassed as a client MAC address can usually be easily changed.

The mechanism used is either 'allow' or 'block' (It cannot be both).

Examples:

``option macmechanism 'allow'``

``list allowedmac '00:00:C0:01:D0:0D'``

``list allowedmac '00:00:C0:01:D0:1D'``

or

``option macmechanism 'block'``

``list blockedmac '00:00:C0:01:D0:2D'``


Trusted Clients
***************

A list of the MAC addresses of client devices that do not require authentication can be defined.

.. note::
 This can easily be be used to allow unauthorised access as a client MAC address can be changed. For a potentially more secure alternative, see "option allow_preemptive_authentication"

Example:

``list trustedmac '00:00:C0:01:D0:0D'``

``list trustedmac '00:00:C0:01:D0:1D'``

Dhcp option 114 Enable - RFC8910
********************************

Sends "default_url" (dhcp option 114) with all replies to dhcp requests

Required for RFC8910 Captive Portal Identification

Default 1 (enabled)

To disable, set to 0

Example:

``option dhcp_default_url_enable '0'``

Packet Marking Compatibility
****************************

openNDS uses specific HEXADECIMAL values to mark packets used by iptables as a bitwise mask.

This mask can conflict with the requirements of other packages.

However the defaults are fully compatible with the defaults used in mwan3 and sqm

Any values set here are interpreted as in hex format.

Option: fw_mark_authenticated
-----------------------------

Default: 30000 (0011|0000|0000|0000|0000 binary)

Option: fw_mark_trusted
-----------------------

Default: 20000 (0010|0000|0000|0000|0000 binary)

Option: fw_mark_blocked
-----------------------

Default: 10000 (0001|0000|0000|0000|0000 binary)

Examples:

``option fw_mark_authenticated '30000'``

``option fw_mark_trusted '20000'``

``option fw_mark_blocked '10000'``



