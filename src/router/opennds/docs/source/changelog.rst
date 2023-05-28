What's New? - ChangeLog
#######################

opennds (9.10.0)

  * This version adds new functionality, and fixes some issues
  * Fix - unable to read client upload traffic volume on some versions of iptables-nft (generic Linux) [bluewavenet]
  * Fix - compatibility with bash shell on generic Linux [bluewavenet]
  * Fix - compiler warning, unused variable [bluewavenet]
  * Fix - silently continue if fw4 table is not found [bluewavenet]
  * Add - Start daemon earlier on boot [bluewavenet]
  * Fix - compatibility with legacy iptables packages [bluewavenet]
  * Add - call to delete nft chains [bluewavenet]
  * Fix - stop using legacy INPUT and FORWARD chains [bluewavenet]
  * Add - watchdog restart if openNDS nftables ruleset is missing [bluewavenet]
  * Add - automated rule setting/deleting for users_to_router [bluewavenet]
  * Add - Change fwhook to add users to router rule to fw4 on OpenWrt [bluewavenet]
  * Add - Set allow or passthrough mode for users_to_router rules [bluewavenet]
  * Fix - set fwhook default to disabled to prevent restart on hotplug event [bluewavenet]
  * Fix - fas-aes-https description comments [bluewavenet]
  * Fix - icon overspill on splash pages [bluewavenet]
  * Fix - missing config option in community script [bluewavenet]
  * Fix - urlencode handling of "$" character and add htmlentity encode/decode library call [bluewavenet]

 -- Rob White <dot@blue-wave.net>  Tue, 17 Jan 2023 14:33:27 +0000

opennds (9.9.1)

  * This version fixes some issues
  * Fix - minimalise deprecated legacy .conf file
  * Fix - Prevent rate limit refresh if rate limit is set to 0 [bluewavenet]
  * Fix - Mute some unneccessary debug messages [bluewavenet]
  * Fix - do not write unconfigured (null) parameters to client id file (cidfile) [bluewavenet]
  * Fix - Prevent error "Command process exited due to signal 13" when executing an external script [bluewavenet]
  * Fix - use WTERMSIG() return code for _execute_ret when execute fails [bluewavenet]
  * Fix - use correct response type for error 503 [bluewavenet]
  * Update Makefile description [bluewavenet]
  * Add - Community Local FAS install script [bluewavenet]
  * Update - Mention TCP port 80 requires AutonomousWG [afriza]

 -- Rob White <dot@blue-wave.net>  Thu, 12 Nov 2022 20:39:31 +0000

opennds (9.9.0)

  * This version adds new functionality, and fixes some issues
  * Add - Community ThemeSpec to support legacy splash.html [bluewavenet]
  * Fix - ensure nat_traversal_poll_interval defaults to 10 seconds [bluewavenet]
  * Add - process send_to_fas_deauthed and send_to_fas_custom in fas-aes-https [bluewavenet]
  * Add - support for send_to_fas_deauthed library call in binauth_log.sh [bluewavenet]
  * Add - heartbeat file containing timestamp [bluewavenet]
  * Add - send_to_fas_deauthed and send_to_fas_custom library calls [bluewavenet]
  * Add - Save authmon daemon startup arguments for libopennds [bluewavenet]
  * Fix - potential divide by zero errors [bluewavenet]
  * Add - option nat_traversal_poll_interval [bluewavenet]
  * Add - Library calls for urlencode and urldecode[bluewavenet]
  * Fix - Don't download remotes if ThemeSpec not configured [bluewavenet]
  * Add - Error report in syslog if dhcp database is not found [bluewavenet]
  * Add - library calls, deauth and daemon_deauth [bluewavenet]
  * Fix - change WTERMSIG log from WARNING to NOTICE [bluewavenet]
  * Add - Set minimum bucket size to 5 regardless of configured bucket ratio [bluewavenet]
  * Fix - safe_vasprint return value [bluewavenet]
  * Add - test if safe_calloc failed and serve error 503 [bluewavenet]
  * Add - use calloc instead of malloc[bluewavenet]
  * fix - safe functions to return error rather than exit [bluewavenet]
  * Add - b64decode custom string received by binauth script [bluewavenet]

 -- Rob White <dot@blue-wave.net>  Thu, 29 Sep 2022 16:52:46 +0000

opennds (9.8.0)

  * This version adds new functionality, and fixes some issues
  * Fix - suppress stderr in client_params in generic linux [bluewavenet]
  * Fix - client_params on generic linux, remote logo not supported yet [bluewavenet]
  * Fix - compiler warning [bluewavenet]
  * Fix - set voucher script as executable [bluewavenet]
  * Update OpenWrt Makefile [bluewavenet]
  * Add - format footer in Themespec scripts [bluewavenet]
  * Update footer on all scripts [bluewavenet]
  * Update - Community Voucher Themespec [bluewavenet]
  * Add - Check on startup for Y2.038K bug (32 bit time) [bluewavenet]
  * Fix - Remove deprecated Debian specific files [bluewavenet]
  * Add - More css updates [bluewavenet]
  * Add - user friendly RFC8910 page511 text and remove refresh button [bluewavenet]
  * Fix - MHD becomes unresponsive serving page 511 for rfc8910 clients [bluewavenet]
  * Add - extra startup settings - ignore_sigpipe and write nds info [bluewavenet]
  * Add - set MHD connection limit to 100, set MHD listen backlog size to 128, set MHD_HTTP_HEADER_CONNECTION "close" [bluewavenet]
  * Fix - Add missing LOG_CRIT in debug [bluewavenet]
  * Add - some useful diagnostic output in authmon [bluewavenet]
  * Fix - Move testing to community [bluewavenet]
  * Fix - Community - Use tmpfs by default for vouchers.txt file [bluewavenet]
  * Add - README with use instructions and notice about flash wearout [fservida]
  * Fix - Refactor folder structure for community themespec [fservida]
  * Add - Create vouchers.txt [fservida]
  * Add - Create theme_voucher.sh [fservida]
  * Update - README.md [bluewavenet]
  * Add - image download info message [bluewavenet]
  * Add - css updates [dianariyanto]
  * Add - allow downloaded remotes refresh for all modes [bluewavenet]
  * Add - download_resources.sh to installed files [bluewavenet]
  * Add - support for download of custom images and files in the status.client page [bluewavenet]
  * Remove - Debian man page support [bluewavenet]
  * Fix - Add missing mkdir command in Makefile [dzatoah]
  * Fix - typos in src/{conf, main}.c [dzatoah]

 -- Rob White <dot@blue-wave.net>  Wed, 05 Aug 2022 15:00:13 +0000

opennds (9.7.0)

  * This version adds new functionality, and fixes some issues
  * Fix - syntax error (missing comma) in awk command in bash on generic Linux [bluewavenet]
  * Add - option to append serial number suffix to gatewayname [bluewavenet]
  * Add - block use of ip aliases on gateway interface [doctor-ox] [bluewavenet]
  * Fix - ndsctl json syntax error [bluewavenet]
  * Add - check for null variables in key value pairs in MHD callbacks [bluewavenet]
  * Fix - changed some notice messages into debug messages [bluewavenet]
  * Fix - possible return of incorrect pid [doctor-ox] [bluewavenet]
  * Fix - possible abiguities resulting in failure to parse parameters correctly [bluewavenet]
  * Fix - Remove deprecated get_client_token.sh [bluewavenet]
  * Fix - Prevent possible malformed mac address returned from dhcpcheck() [doctor-ox] [bluewavenet]

 -- Rob White <dot@blue-wave.net>  Wed, 16 Mar 2022 15:54:29 +0000

opennds (9.6.0)

  * This version adds new functionality, and fixes some issues
  * Fix - correctly display return buffer in syslog [bluewavenet]
  * Add - use heap allocation for library call return buffer [bluewavenet]
  * Fix - OpenWrt, fhook request for fw3 [bluewavenet]
  * Add - spider remote urls before downloading [bluewavenet]
  * Add - OpenWrt, revert uncommitted uci updates at startup and shutdown [bluewavenet]
  * Fix - remove unneccesary flash writes and fix hosts updates [doctor-ox] [bluewavenet]
  * Add - Updated splash images [bluewavenet]
  * Add - OpenWrt makefile for nft or ipt dependencies [bluewavenet]
  * Fix - grep by word to prevent any ambiguity [doctor-ox] [bluewavenet]
  * Fix - ensure rate limiting is disabled if rate thresholds are set to zero [bluewavenet]
  * Add - querystring support for client status page [bluewavenet]
  * Add - Advanced/standard status page checkbox [bluewavenet]
  * Add - set default session timeout to 24 hours [bluewavenet]
  * Fix - potential buffer overflow [bluewavenet]
  * Fix - Restrict max packet limit to iptables maximum [bluewavenet]
  * Fix - descriptive labels on ndsctl status output [bluewavenet]
  * Add - update of README.md [bluewavenet]
  * Fix - Added required variable to FAS return string example documentation [dorkone]
  * Add - Default checkinterval set to 15 seconds [bluewavenet]
  * Fix - incoming and outgoing counters when unlimited bursting is enabled [bluewavenet]
  * Add - maximum bucket size configuration [bluewavenet]
  * Add - calculate moving average packet size for rate limiting [bluewavenet]
  * Add - some operational default values [bluewavenet]
  * Add - initial rate limits when unrestricted bursting is disabled [bluewavenet]
  * Add - Require clients to be in the dhcp database [bluewavenet]
  * Add - dhcpcheck library call [bluewavenet]
  * Fix - Remove trailing whitespace when getting clientaddress if client not active [bluewavenet]
  * Fix - Segfault when FAS fails to Return customstring [dorkone] [bluewavenet]
  * Add - Enable/Disable unrestricted bursting [bluewavenet]
  * Add - gatewayurl to querystring and use in place of originurl in FAS [bluewavenet]
  * Fix - more accurate debug message [bluewavenet]
  * Fix - Show packet rate correctly as packets per minute [bluewavenet]
  * Add - Report Packet Rate and Bucket Size in ndsctl status and json and status client page [bluewavenet]
  * Add - rate limit refresh to client limit rules [bluewavenet]
  * Fix - code readability [bluewavenet]
  * Fix - Documentation for data sent to Authmon Daemon [bluewavenet]
  * Add - Show unrestricted burst intervals in ndsctl status [bluewavenet]
  * Add - Set default bucket ratios to 10 [bluewavenet]

 -- Rob White <dot@blue-wave.net>  Sun, 06 Feb 2022 07:44:50 +0000

opennds (9.5.1)

  * This minor version update fixes two important issues
  * Fix - ThemeSpec file downloads when mwan3 is running [bluewavenet]
  * Fix - Preemptive auth failure after previous deauth [minhng99] [bluewavenet]

 -- Rob White <dot@blue-wave.net>  Thu, 16 Dec 2021 16:22:16 +0000

opennds (9.5.0)

  * This version adds new functionality, and fixes some issues
  * Add - use average packet size instead of MTU when implementing rate limiting [bluewavenet]
  * Fix - typo in iptables command and remove a redundant command [bluewavenet]
  * Add - startdaemon() and stopdaemon() utility functions [bluewavenet]
  * Add - combined interface/ipaddress external gateway status monitoring [bluewavenet]
  * Fix - potential online/offline detection problem when mwan3 is running [bluewavenet]
  * Add - get_debug_level and syslog library calls [bluewavenet]
  * Fix - correctly reset upload and download rate rules [bluewavenet]
  * Add - extend upstream gateway checking for use with mwan3 loadbalance/failover [bluewavenet]
  * Fix - Potential NULL pointer segfault in http_microhttpd on calling authenticated() [bluewavenet]
  * Fix - Potential NULL pointer segfault in http_microhttpd on calling preauthenticated() [dddaniel]
  * Add - Calculate Bucket size based on achieved burst rate [bluewavenet]
  * Fix - prevent parameter parsing if clientip not known [bluewavenet]
  * Add - disable rate quotas by setting bucket ratio to zero [bluewavenet]
  * Fix - suppress some debug messages [bluewavenet]
  * Add - more libraries documentation [bluewavenet]
  * Add - library calls startdaemon and stopdaemon [bluewavenet]
  * Fix - Increase buffer length for longer interface names [koivunen]
  * Add - enforce minimum data rates in ndsctl auth [bluewavenet]
  * Add - Update README.md [bluewavenet]
  * Add - bucket ratio option to config file [bluewavenet]
  * Add - upload and download bucket ratio config values [bluewavenet]
  * Fix - flag initial debuglevel to externals [bluewavenet]
  * Add - limit-burst tuning to rate quotas [bluewavenet]
  * Fix - add trailing space to defaultip [bluewavenet]
  * Add - record pre-emptive authentication in local log [bluewavenet]
  * Add - Write to local log function to libopennds [bluewavenet]
  * Add - set client_type and custom string for Pre-emptive authentication [bluewavenet]
  * Fix - Remove trailing newline from library call response [bluewavenet]
  * Fix - attempt to remove cid file only if client->cid is set [bluewavenet]
  * Add - a skip option for custom downloads to speed up serving page from themespec [bluewavenet]
  * Add - put client_type into query string when type is cpd canary [bluewavenet]
  * Add - set refresh=0 before loading images [bluewavenet]
  * Fix - Truncated return status [bluewavenet]
  * Add - Acknowlegement from call to dnsconfig [bluewavenet]
  * Fix - potential buffer overflow in debug output [bluewavenet]
  * Add - processing of custom data and client type [bluewavenet]
  * Add - Client Type for RFC8908 and RFC8910 clients [bluewavenet]
  * Add - rfc8908 replies for external FAS and refactor memory management for MHD calls [bluewavenet]
  * Add - send error 403 if client is not on openNDS subnet [bluewavenet]
  * Fix - remove uneccessary safe_asprint in auth.c [bluewavenet]
  * Fix - Initialise buffer to prevent receiving spurious characters [bluewavenet]
  * Add - encoded custom data support to ndsctl json, themespec and binauth [bluewavenet]
  * Add - advert_1.htm to thankyou page of theme_click-to-continue-custom-placeholders.sh [bluewavenet]
  * Add - library call get_interface_by_ip [bluewavenet]
  * Add - function encode_custom() for encoding custom data to be sent to openNDS [bluewavenet]
  * Fix - error 511, make all html refrences absolute to enforce link to MHD [bluewavenet]
  * Add - check status_path exists and is executeable [bluewavenet]
  * Fix - regression causing error 511 to be served from default script [bluewavenet]
  * Add - venue-info-url and can-extend-session json keys [bluewavenet]
  * Add - RFC 8908 initial experimental support [bluewavenet]
  * Add - debug message when resetting client [bluewavenet]
  * Fix - Ensure the ndscids directory exists before trying to write to it. [bluewavenet]
  * Fix - use eval in do_ndsctl to allow quoting of arguments [bluewavenet]
  * Fix - ensure client hid and client cid file is reset correctly [bluewavenet]
  * Fix - Titles of example ThemeSpec Files [bluewavenet]
  * Fix - Ensure ThemeSpec Files are executable [bluewavenet]
  * Remove - deprecated Allowed and Blocked entries in ndsctl status output [bluewavenet]
  * Add - Deprecate option macmechanism, allowedmaclist and blockedmaclist [bluewavenet]

 -- Rob White <dot@blue-wave.net>  Wed, 8 Dec 2021 06:45:56 +0000

opennds (9.4.0)

  * This version adds new functionality, and fixes some issues
  * Add - Error message in fas-aes-https if shared key is mismatched [bluewavenet]
  * Fix - and refactor error 511 page generation[bluewavenet]
  * Fix - and refactor dnsmasq configuration [bluewavenet]
  * Fix - Typographic error preventing RFC8910 disable [bluewavenet]
  * Add - gateway address and gatewayfqdn to ndsctl json output [bluewavenet]
  * Add - RFC8910 housekeeping on startup and shutdown [bluewavenet]
  * Add - correctly apply dhcp option 114 for generic Linux [bluewavenet]
  * Add - reading of configured ndsctlsocket in ndsctl utility[bluewavenet]
  * Add - use send_error 200 for MHD watchdog [bluewavenet]
  * Add - generation of page_511 html by library script [bluewavenet]
  * Add - extend debuglevel support to library scripts [bluewavenet]
  * Refactor - fas-aes-https to simplify and make customisation of http easier [bluewavenet]
  * Add - library script for error 511 page, allowing customisation [bluewavenet]
  * Add - make authmon report connection error details [bluewavenet]
  * Fix- remove unwanted debug message in ndsctl [bluewavenet]
  * Add - RFC8910 support by default [bluewavenet]
  * Add - display status page when accessing /login when authenticated [bluewavenet]
  * Add - MHD response to RFC8910 requests [bluewavenet]
  * Add - Dnsmasq RFC8910 configuration [bluewavenet]
  * Add - send error 511 in response to unsupported http method [bluewavenet]
  * Add - Check for ca-bundle on OpenWrt, if not installed, add syslog messages and terminate [bluewavenet]
  * Add - Make ndsctl use the configured value for socket path if set and deprecate -s option [bluewavenet]
  * Add - Warning message when Walled Garden port 80 is allowed [bluewavenet]
  * Fix - remove un-needed pthread_kill in termination_handler() [bluewavenet] [T-X]
  * Fix - debug messages from authmon.sh [bluewavenet]
  * Fix - Allow disabling gateway fqdn, facilitating access to router port 80 [bluewavenet]
  * Fix - Segfault in ndsctl when -s option is used incorrectly [bluewavenet] [T-X]
  * Fix - Typo making calculation of ul/dl rates incorrect [bluewavenet]
  * Fix - Allow port 80 to be configured in the Walled Garden [bluewavenet]

 -- Rob White <dot@blue-wave.net>  Wed, 22 Sep 2021 19:39:08 +0000

opennds (9.3.1)

  * This version fixes some issues
  * Fix - Segfault in ndsctl when -s option is used incorrectly [bluewavenet] [T-X]
  * Fix - Typo making calculation of ul/dl rates incorrect [bluewavenet]
  * Fix - Allow port 80 to be configured in the Walled Garden [bluewavenet]
  * Add - Warning message when Walled Garden port 80 is allowed [bluewavenet]

 -- Rob White <dot@blue-wave.net> Thu, 26 Aug 2021 12:09:36 +0000

opennds (9.3.0)

  * This version adds new functionality, and fixes some issues
  * Add - Add - firewall passthrough mode for authenticated users [bluewavenet]
  * Add - Add - use configured debuglevel in authmon [bluewavenet]
  * Add - automated log rotation and client_zone to binauth_log [bluewavenet]
  * Add - increased timeout interval for file downloads [bluewavenet]
  * Add - local interface to MeshZone and remove unneeded call to ip utility [bluewavenet]
  * Add - log_mountpoint and max_log_entries options [bluewavenet]
  * Add - config variables ext_interface and ext_gateway [bluewavenet]
  * Add - Start initial download of remotes only if online [bluewavenet]
  * Add - Router online/offline watchdog [bluewavenet]
  * Fix - Segfault when gatewayfqdn is disabled [bluewavenet]
  * Fix - missing clientmac when not using themespec [bluewavenet]
  * Fix - some compiler warnings [bluewavenet]
  * Fix - use configured value for webroot for remote image symlink to images folder [bluewavenet]
  * Fix - remove refrences to login.sh in documentation and comments [bluewavenet]
  * Fix - Prevent potential read overrun within the MHD page buffer [bluewavenet]
  * Remove - legacy get_ext_iface() function [bluewavenet]

 -- Rob White <dot@blue-wave.net> Sun, 8 Aug 2021 09:58:02 +0000

opennds (9.2.0)

  * This version adds new functionality, improves performance, adds documentation and fixes an issue
  * Add - new config options to ndsctl status [bluewavenet]
  * Add - Readthedocs / man documentation for configuration options [bluewavenet]
  * Add - Faster convergence of average rates to configured rate quotas [bluewavenet]
  * Add - BinAuth parse authenticated client database for client data [bluewavenet]
  * Add - Use heap allocation for http page buffer allowing large page sizes [bluewavenet]
  * Fix - fail to serve downloaded images on custom themespec [bluewavenet]

 -- Rob White <dot@blue-wave.net> Sun, 11 July 2021 15:05:39 +0000

opennds (9.1.1)

  * This version fixes a compiler error, some compiler warnings and mutes a debug message
  * Fix - Compiler error, missing mode in call to open() [bluewavenet]
  * Fix - Compiler warning, ignored return value from call to lockf() [bluewavenet]
  * Fix - Compiler warning, ignored return value from call to system() [bluewavenet]
  * Fix - Compiler warning, ignored return value from call to fgets() [bluewavenet]
  * Fix - Remove debug message from call to get_client_interface library [bluewavenet]

 -- Rob White <dot@blue-wave.net> Thu, 4 July 2021 21:07:21 +0000

opennds (9.1.0)

  * This version introduces new functionality, some changes and fixes
  * Add - option statuspath to enable alternate status page scripts [bluewavenet]
  * Add - ndsctl lockf() file locking [bluewavenet] [T-X]
  * Add - b64encode to ndsctl [bluewavenet]
  * Add - option max_page_size for MHD [bluewavenet]
  * Add - option remotes_refresh_interval [bluewavenet]
  * Add - Pre-download remote files in background after startup [bluewavenet]
  * Add - client id data files created by openNDS on client connect [bluewavenet]
  * Add - check routing is configured and up [bluewavenet]
  * Add - support for Preemptive Authentication for connected client devices. [bluewavenet]
  * Add - Gateway interface watchdog [bluewavenet]
  * Remove - deprecated IFB config [bluewavenet]
  * Fix - ndsctl, send return codes [bluewavenet]
  * Fix - MHD Watchdog Use uclient-fetch in OpenWrt [bluewavenet]
  * Fix - Improve MHD watchdog [bluewavenet]
  * Fix - update legacy code in ndsctl_thread [bluewavenet]
  * Fix - edge case where MHD returns (null) as host value [bluewavenet]

 -- Rob White <dot@blue-wave.net> Thu, 24 June 2021 15:06:30 +0000

openNDS (9.0.0)

  * This version introduces major new functionality, some changes and fixes
  * Add - post-request - add upstream payload [bluewavenet]
  * Add - post-request - base64 encode payload [bluewavenet]
  * Add - authmon add more status checking and default to view mode for upstream processing [bluewavenet]
  * Add - authmon add housekeeping call, limit concurrent authentications, send auth-ack [bluewavenet]
  * Add - fas-aes-https add housekeeping call, add auth-ack support, add "try again" button [bluewavenet]
  * Add - "$" character added to htmlentityencode [bluewavenet]
  * Add - Theme support - theme_click-to-continue [bluewavenet]
  * Add - Themespec, custom variables and custom images options to OpenWrt config [bluewavenet]
  * Add - Support for ThemeSpecPath, FasCustomParametersList, FasCustomVariablesList, FasCustomImagesList [bluewavenet]
  * Add - Example theme - click-to-continue-custom-placeholders [bluewavenet]
  * Add - Increase Buffer sizes to support custom parameters [bluewavenet]
  * Add - themespec_path argument [bluewavenet]
  * Add - Increase buffers for custom vars and images [bluewavenet]
  * Add - Increase command buffer for custom vars and images [bluewavenet]
  * Add - Increase HTMLMAXSIZE [bluewavenet]
  * Add - Use MAX_BUF for fasparam, fasvar and fasimage [bluewavenet]
  * Add - support for ThemeSpec files and placeholders [bluewavenet]
  * Add - Theme Click to Continue with Custom Placeholders [bluewavenet]
  * Add - make custom field a required entry [bluewavenet]
  * Add - bash/ash check and simplify image download config [bluewavenet]
  * Add - example custom images and text placeholders to click-to-continue-custom [bluewavenet]
  * Add - theme_user-email-login-custom-placeholders [bluewavenet]
  * Add - Status page for login failure [bluewavenet]
  * Add - fas_custom_files_list and update Makefiles [bluewavenet]
  * Add - Autoconfiguration of ndsctl socket file to use tmpfs mountpoint [bluewavenet]
  * Add - example custom images and custom html [bluewavenet]
  * Add - Set default gateway interface br-lan [bluewavenet]
  * Add - libopennds, set wget timeout [bluewavenet]
  * Add - allow disabling of gatewayfqdn [bluewavenet]
  * Add - packet rate limiting for upload/download rate quotas [bluewavenet]
  * Add - get custom resources from Github branch [bluewavenet]
  * Add - functions start_mhd() and stop_mhd() [bluewavenet]
  * Add - MHD Watchdog - restart MHD if required [bluewavenet]
  * Add - Pause and retry popen on failure [bluewavenet]
  * Add - function get_key_from_config() [bluewavenet]
  * Remove - deprecated traffic control code [bluewavenet]
  * Remove - deprecated binauth scripts [bluewavenet]
  * Remove - deprecated legacy splash page support [bluewavenet]
  * Remove - deprecated ndsctl clients [bluewavenet]
  * Remove - outdated PreAuth scripts [bluewavenet]
  * Refactor - Move hid to head of query string [bluewavenet]
  * Refactor - Move libopennds to libs
  * Fix - ndsctl auth crashed opennds if session duration argument was null [bluewavenet]
  * Fix - fas-aes-https - correctly set path for authlist for most server types [bluewavenet]
  * Fix - suppress BinAuth syslog notice message [bluewavenet]
  * Fix - setting gw_fqdn in hosts file if gw_ip is changed [bluewavenet]
  * Fix - add missing comma before trusted list in ndsctl json [bluewavenet] [gueux]
  * Fix - Improve Shell detection [bluewavenet]
  * Fix - Improve b64decode performance [bluewavenet]
  * Fix - ndsctl -s option [bluewavenet] [gueux]
  * Fix - Adjust config defaults to good real world values [bluewavenet]
  * Fix - don't override ndsparamlist in ThemeSpec files [bluewavenet]
  * Fix - Check ndsctl lock to prevent calling from Binauth [bluewavenet]
  * Fix - Clean up syslog messages at info level (2) [bluewavenet]
  * Fix - Debian changelog format to allow package building [bluewavenet]
  * Fix - numerous compiler errors and BASH compatibility issues [bluewavenet]
  * Fix - ndsctl auth, ensure if session timeout = 0 then use global value [bluewavenet]
  * Fix - setting of gatewayport, caused by typo in conf.c [bluewavenet] [Ethan-Yami]
  * Fix - remove unused credential info from log [bluewavenet]
  * Deprecate - the legacy opennds.conf file [bluewavenet]

 -- Rob White <dot@blue-wave.net> Thu, 2 May 2021 17:32:43 +0000

openNDS (8.1.1)

  * Fix - remove legacy code where option preauthenticated_users containing the keyword "block" would cause openNDS to fail to start [bluewavenet]

 -- Rob White <dot@blue-wave.net> Thu, 21 Feb 2021 16:33:34 +0000

openNDS (8.1.0)

  * This version introduces some new functionality and some fixes/enhancements
  * Fix - Add default values for gatewayfqdn. If not set in config could result in crash on conection of first client [bluewavenet]
  * Add - Authenticated users are now granted access to the router by entry in "list authenticated_users" [bluewavenet]
  * Fix - option preauth was being ignored [bluewavenet]
  * Add - query string validity check and entity encode "$" character. Generate error 511 if query string is corrupted [bluewavenet]
  * Add - a "Try Again" button to the login.sh script, to be displayed if the client token has expired before login. [bluewavenet]

 -- Rob White <dot@blue-wave.net> Thu, 18 Feb 2021 17:03:23 +0000

openNDS (8.0.0)

  * This version introduces major new functionality and some major changes
  * Rationalisation of support for multiple Linux distributions [bluewavenet]
  * Refactor login.sh script introducing base64 encoding and hashed token (hid) support [bluewavenet]
  * Refactor fas-hid script introducing base64 encoding and simplifying customisation of the script [bluewavenet]
  * Refactor binauth_log.sh and log BinAuth custom data as url encoded [bluewavenet]
  * Refactor fas-aes, simplifying customisation of the script [bluewavenet]
  * Refactor fas-aes-https, simplifying customisation of the script [bluewavenet]
  * Change - Use hid instead of tok when fas_secure_enabled >= 1 [bluewavenet]
  * Add - base64 encoding to fas_secure_enabled level 1 [bluewavenet]
  * Add - gatewyname, clientif, session_start, session_end and last_active to ndsctl json [bluewavenet]
  * Add - support for RFC6585 Status Code 511 - Network Authentication Required [bluewavenet]
  * Add - Client Status Page UI with Logout [bluewavenet]
  * Add - GatewayFQDN option [bluewavenet]
  * Add - client interface to status page query string [bluewavenet]
  * Add - support using base 64 encoded custom string for BinAuth and replace tok with hid [bluewavenet]
  * Add - base 64 decode option to ndsctl [bluewavenet]
  * Add - b64 encoding of querystring for level 1 [bluewavenet]
  * Add - Improved performance/user-experience on congested/slow systems using php FAS scripts [bluewavenet]
  * Add - support for ndsctl auth by hid in client_list [bluewavenet]
  * Add - Ensure faskey is set to default value (always enabled) [bluewavenet]
  * Add - Display error page on login failure in login.sh [bluewavenet]
  * Add - splash.html, add deprecation notice [bluewavenet]
  * Add - authmon, improved lock checking and introduce smaller loopinterval [bluewavenet]
  * Add - client_params, wait for ndsctl if it is busy [bluewavenet]
  * Add - fas-aes-https, allow progressive output to improve user experience on slow links [bluewavenet]
  * Fix - Block access to /opennds_preauth/ if PreAuth not enabled [bluewavenet]
  * Fix - On startup, call iptables_fw_destroy before doing any other setup [bluewavenet]
  * Fix - missing final redirect to originurl in fas-hid [bluewavenet]
  * Fix - ensure gatewayname is always urlencoded [bluewavenet]
  * Fix - client session end not set by binauth [bluewavenet]
  * Fix - Session timeout, if client setting is 0, default to global value [bluewavenet]
  * Fix - missing trailing separator on query and fix some compiler errors [bluewavenet]
  * Fix - ensure authmon daemon is killed if left running from previous crash [bluewavenet]
  * Fix - add missing query separator for custom FAS parameters [bluewavenet]
  * Fix - ndsctl auth, do not set quotas if client is already authenticated [bluewavenet]
  * Fix - client_params, show "Unlimited" when "null" is received from ndsctl json [bluewavenet]
  * Update configuration files [bluewavenet]
  * update documentation [bluewavenet]

 -- Rob White <dot@blue-wave.net> Sat, 2 Jan 2021 16:38:14 +0000

openNDS (7.0.1)

  * This version contains fixes and some minor updates
  * Fix - Failure of Default Dynamic Splash page on some operating systems [bluewavenet]
  * Fix - A compiler warning - some compiler configurations were aborting compilation [bluewavenet]
  * Update - Added helpful comments in configuration files [bluewavenet]
  * Remove - references to deprecated RedirectURL in opennde.conf [bluewavenet]
  * Update - Documentation updates and corrections [bluewavenet]

 -- Rob White <dot@blue-wave.net> Wed, 7 Nov 2020 12:40:33 +0000

openNDS (7.0.0)

  * This version introduces major new enhancements and the disabling or removal of deprecated functionality
  * Fix - get_iface_ip in case of interface is vif or multihomed [bluewavenet]
  * Fix - Add missing client identifier argument in ndsctl help text [bluewavenet]
  * Deprecate - ndsctl clients option [bluewavenet]
  * Add - global quotas to output of ndsctl status [bluewavenet]
  * Fix - fix missing delimiter in fas-hid [bluewavenet]
  * Add - Report Rate Check Window in ndsctl status and show client quotas [bluewavenet]
  * Add - Quota and rate reporting to ndsctl json. Format output and fix json syntax errors [bluewavenet]
  * Fix - get_client_interface for case of iw utility not available [bluewavenet]
  * Fix - php notice for pedantic php servers in post-request [bluewavenet]
  * Add - built in autonomous Walled Garden operation [bluewavenet]
  * Remove - support for deprecated RedirectURL [bluewavenet]
  * Add - gatewaymac to the encrypted query string [bluewavenet]
  * Deprecate - legacy splash.html and disable it [bluewavenet]
  * Add - support for login mode in PreAuth  [bluewavenet]
  * Add - Support for Custom Parameters [bluewavenet]

 -- Rob White <dot@blue-wave.net> Wed, 5 Nov 2020 18:22:32 +0000

openNDS (6.0.0)

  * This version - for Openwrt after 19.07 - for compatibility with new MHD API
  * Set - minimum version of MHD to 0.9.71 for new MHD API [bluewavenet]
  * Set - use_outdated_mhd to 0 (disabled) as default [bluewavenet]
  * Add - Multifield PreAuth login script with css update [bluewavenet]
  * Add - Documentation and config option descriptions for configuring Walled Garden IP Sets

 -- Rob White <dot@blue-wave.net> Wed, 21 Aug 2020 15:43:47 +0000

openNDS (5.2.0)

  * This version - for backport to Openwrt 19.07 - for compatibility with old MHD API
  * Fix - Failure of MHD with some operating systems eg Debian [bluewavenet]
  * Fix - potential buffer truncation in ndsctl
  * Set - use_outdated_mhd to 1 (enabled) as default [bluewavenet]
  * Set - maximum permissible version of MHD to 0.9.70 to ensure old MHD API is used [bluewavenet]

 -- Rob White <dot@blue-wave.net> Wed, 12 Aug 2020 17:43:57 +0000

openNDS (5.1.0)

  * Add - Generic Linux - install opennds.service [bluewavenet]
  * Add - Documentation updates [bluewavenet]
  * Add - config file updates [bluewavenet]
  * Add - Install sitewide username/password splash support files [bluewavenet]
  * Add - quotas to binauth_sitewide [bluewavenet]
  * Add - Splash page updates [bluewavenet]
  * Add - Implement Rate Quotas [bluewavenet]
  * Fix - check if idle preauthenticated [bluewavenet]
  * Add - support for rate quotas [bluewavenet]
  * Fix - Correctly compare client counters and clean up debuglevel messages [bluewavenet]
  * Add - Implement upload/download quotas Update fas-aes-https to support quotas [bluewavenet]
  * Add - Rename demo-preauth scripts and install all scripts [bluewavenet]
  * Add - fas-aes-https layout update [bluewavenet]
  * Add - Set some defaults in fas-aes-https [bluewavenet]
  * Add - custom data string to ndsctl auth [bluewavenet]
  * Add - custom data string to fas-hid.php [bluewavenet]
  * Add - Send custom data field to BinAuth via auth_client method [bluewavenet]
  * Fix - missing token value in auth_client [bluewavenet]
  * Add - upload/download quota and rate configuration values [bluewavenet]
  * Add - Send client token to binauth [bluewavenet]
  * Add - Rename upload_limit and download_limit to upload_rate and download_rate [bluewavenet]
  * Fix - Pass correct session end time to binauth [bluewavenet]
  * Add - some debuglevel 3 messages [bluewavenet]
  * Add - description of the favicon and page footer images [bluewavenet]
  * Add - Authmon collect authentication parameters from fas-aes-https [bluewavenet]
  * Add - sessionlength to ndsctl auth [bluewavenet]
  * Fix - Page fault when ndsctl auth is called and client not found [bluewavenet]
  * Add - Enable BinAuth / fas_secure_enabled level 3 compatibility [bluewavenet]
  * Fix - Correctly set BinAuth session_end [bluewavenet]
  * Add - Updates to Templated Splash pages [bluewavenet]
  * Add - Community Testing files [bluewavenet]
  * Fix - BinAuth error passing client session times [bluewavenet]
  * Fix - PHP notice - undefined constant [bluewavenet]
  * Fix - OpenWrt CONFLICTS variable in Makefile [bluewavenet]

 -- Rob White <dot@blue-wave.net> Wed, 24 Jun 2020 20:55:18 +0000

openNDS (5.0.1)

  * Fix - Path Traversal Attack vulnerability allowed by libmicrohttpd's built in unescape functionality [bluewavenet] [lynxis]

 -- Rob White <dot@blue-wave.net> Wed, 06 May 2020 19:56:27 +0000

openNDS (5.0.0)

  * Import - from NoDogSplash 4.5.0 allowing development without compromising NoDogSplash optimisation for minimum resource utilisation [bluewavenet]
  * Rename - from NoDogSplash to openNDS [bluewavenet]
  * Create - openNDS avatar and splash image [bluewavenet]
  * Move - wait_for_interface to opennds C code ensuring consistent start at boot time for all hardware, OpenWrt and Debian [bluewavenet]
  * Add - Enable https protocol for remote FAS [bluewavenet]
  * Add - trusted devices list to ndsctl json output [bluewavenet]
  * Add - option unescape_callback_enabled [bluewavenet]
  * Add - get_client_token library utility [bluewavenet]
  * Add - utf-8 to PreAuth header [bluewavenet]
  * Add - PreAuth Support for hashed id (hid) if sent by NDS [bluewavenet]
  * Add - library script shebang warning for systems not running Busybox [bluewavenet]
  * Add - htmlentityencode function, encode gatewayname in templated splash page [bluewavenet]
  * Add - htmlentity encode gatewayname on login page (PreAuth) [bluewavenet]
  * Add - Simple customisation of log file location for PreAuth and BinAuth [bluewavenet]
  * Add - option use_outdated_mhd [bluewavenet]
  * Add - url-encode and htmlentity-encode gatewayname on startup [bluewavenet]
  * Add - Allow special characters in username (PreAuth) [bluewavenet]
  * Add - Documentation updates [bluewavenet]
  * Add - Various style and cosmetic updates  [bluewavenet]
  * Fix - Change library script shebang to bash in Debian [bluewavenet]
  * Fix - Remove unnecessary characters causing script execution failure in Debian [bluewavenet]
  * Fix - Add missing NULL parameter in MHD_OPTION_UNESCAPE_CALLBACK [skra72] [bluewavenet]
  * Fix - Script failures running on Openwrt 19.07.0 [bluewavenet]
  * Fix - Preauth, status=authenticated [bluewavenet]
  * Fix - Prevent ndsctl from running if called from a Binauth script. [bluewavenet]
  * Fix - Minor changes in Library scripts for better portability [bluewavenet]
  * Fix - Prevent php notices on pedantic php servers [bluewavenet]
  * Fix - broken remote image retrieval (PreAuth) [bluewavenet]
  * Fix - Allow use of "#" in gatewayname [bluewavenet]

 -- Rob White <dot@blue-wave.net> Sat, 03 Apr 2020 13:23:36 +0000

