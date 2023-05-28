Frequently Asked Questions
###########################

What's the difference between versions?
***************************************


**NoDogSplash and openNDS** are derived from the same code base.

You cannot upgrade from NoDogSplash to openNDS, instead you must first uninstall NodogSplash before installing openNDS.

**NoDogSplash** is optimised for running on devices with very limited resources and supports only a single *static* templated html splash page.

**openNDS** supports dynamic html splash page generation (at default, still with minimal resource utilisation) and an API to support the coding of sophisticated Captive Portal solutions.

**openNDS v5** This was the first release of openNDS after forking from NoDogsplash. The following enhancements are included:

 * **openNDS API (FAS)**

   A forwarding authentication service. FAS supports development of "Credential Verification" running on any dynamic web serving platform, on the same device as openNDS, on another device on the local network, or on an Internet hosted web server.

 * **PreAuth**

   An implementation of FAS running on the same device as openNDS and using openNDS's own web server to generate dynamic web pages. Any scripting language or even a compiled application program can be used. This has the advantage of not requiring the resources of a separate web server.

 * **BinAuth**

   Enabling an external script to be called for doing post authentication processing such as setting session durations or writing local logs.

 * **Enforce HTTPS option**

   This option enables *https* access to a remote, Internet based FAS server, ensuring the client device does not receive any security warnings or errors. Access to the FAS server using **https** protocol is enforced.

 * **Data volume and Rate Quotas**

   This option enables built in *Data Volume* and *Data Rate* quota support. Data volume and data rate quotas can be set globally in the config file. The global values can be overridden on a client by client basis as required.

 * **Introduction of library scripts**

   Numerous library scripts are introduced to simplify development of applications.


**openNDS v6** This is the first version of openNDS to use the updated libmicrohttpd (MHD) API introduced with v0.9.71.

 * openNDS **REQUIRES** MHD v0.9.71 or higher.

**openNDS v7** This version contains several major enhancements, including:

 * **Autonomous Walled Garden Support**

   A simple openNDS configuration option enables Autonomous Walled Garden operation based on a list of target FQDNs

 * **Custom Parameter Support**

   A list of static Custom Parameters can be set as a configuration option. Once set, these parameters are fixed and will be sent to remote FAS servers.

   This functionality was added specifically to support remote configuration tools such as Opensync, but can be generally useful for passing local fixed information to a remote FAS.

   It is important that this is NOT confused with the dynamic Custom Variables that can be defined as a part of a FAS/Client dialogue.

 * **Legacy Templated splash.html Deprecated and Disabled**

   The legacy splash.html page and its templated variables is deprecated and disabled. This is replaced by an enhanced login option script capable of generating a dynamic html dialogue. The default is a simple click to continue page, similar to the old splash.html.

**openNDS v8** This version contains several major enhancements, including:

 * **Base 64 encoding of the Query string**

   The introduction of Base 64 encoding of the query string into the FAS API, enables easier customisation of FAS scripts.

 * **Global use of Hashed ID (hid)**

   Hashed ID (hid), carried in the base64 encoded query string for levels 1, 2 and 3, including the default click to continue page, provides a uniform level of security against portal bypass. The option "faskey" is now enabled at all times with a simple default value and settable in the config files and FAS scripts.

 * **Client User Status Page**

   A client user status page is now available at a configurable Gateway FQDN. This page includes a listing of the client's quota allocation and usage, as well as a "Logout" button. A client that is connected butnot logged in for any reason will be redirected to an RFC6585 Status Code 511 "Network Authentication Required" page with a "Login" button. the default url for a client to access their status page is http://status.client

**openNDS v9** This version contains several major enhancements, including:

 * **Introduction of ThemeSpec Script Files**

   Option login_option is extended to include Themed page sequences with custom parameters, variables, images and files. A ThemeSpec file has the ability to inject html form fields, text strings, images from external sources and content files also from external sources, all defined in the config file.

 * **Enhanced Data Rate Quotas**

   Data Rate Quotas are now enhanced with packet rate limiting being applied when a client average rate rises above its preset threshold. The time window over which the rate is averaged can be tuned using settings in the config file, allowing rate bursting for best user experience while still limiting the rate of larger downloads or uploads that might otherwise impact other clients.

Can I upgrade from NoDogSplash to openNDS?
******************************************

No.

* You must first uninstall NoDogSplash before installing openNDS.

Can I upgrade from v5 to v6
***************************

Yes.

* But you must upgrade libmicrohttpd to version v0.9.71 or higher.

Can I upgrade from v6 to v7?
****************************

You can, if:

* You don't use RedirectURL (this has been deprecated for some time as it mostly did not work with client CPD implementations. It has now been removed. A reliable replacement is a FAS Welcome Page.
* You don't use the Templated html splash page (splash.html). Templated splash is now deprecated and disabled. It can be re-enabled by setting the allow_legacy_splash option to allow time for migration. Support will be removed entirely in a later version.

Can I upgrade from v7 to v8?
****************************

You can, if:

 * You modify your FAS scripts to use the openNDS v8 API. The FAS query string is now either base64 encoded, or encrypted.
 * In addition Hashed ID (hid) is used for authentication, removing the need for a FAS script to somehow obtain the client Token.

Can I upgrade from v8 to v9
***************************

You can, if:

 * You modify your FAS scripts to use the openNDS v9 API
 * You move to ThemeSpec scripts or FAS **from Legacy Splash**. Legacy Splash Pages are no longer supported. The default ThemeSpec (option login_option 1) is equivalent to the old splash.html click to continue page.

How can I add custom parameters, such as site specific information?
*******************************************************************

Custom parameters were introduced in openNDS version 7 and are defined simply in the config file. These parameters are passed to the FAS in the query string. Version 8 embeds any custom parameters in the encoded/encrypted query string, macing it much simpler to parse for them in the FAS script.

How can I add custom fields on the login page, such as phone number, car licence plate number etc.?
***************************************************************************************************

A simple configuration option allows fields to be added automatically to the pages of ThemeSpec login sequences.

Is it possible to display custom info or advertising on the login pages?
************************************************************************

Yes! Simple config options specify the URLs of images and html content. These will be automatically downloaded and injected into the dynamic pages created by suitable Themespec scripts.

How do I manage client data usage?
**********************************

openNDS (NDS) has built in *Data Volume* and *Data Rate* quota support.

 * Data volume and data rate quotas can be set globally in the config file.
 * The global values can be overridden on a client by client basis as required, either by FAS or BinAuth.
 * If a client exceeds their volume quota they will be deauthenticated.
 * If a client exceeds their rate quota, they will be packet rate limited to ensure their average rate stays below the rate quota value. This allows clients to burst at a higher rate for short intervals, improving performance, but prevents them from hogging bandwidth. 

Can I use Traffic Shaping with openNDS?
***************************************

SQM Scripts (Smart Queue Management), is fully compatible with openNDS and if configured to operate on the openNDS interface (br-lan by default) will provide efficient IP connection based traffic control to ensure fair usage of available bandwidth.

This can be installed as a package on OpenWrt.
For other distributions of Linux it is available at:
https://github.com/tohojo/sqm-scripts

Is an *https splash page* supported?
************************************
**Yes**. FAS Secure Level 3 enforces https protocol for the splash login page on an external FAS server.

Is *https capture* supported?
*****************************
**No**.

* If it was supported, all connections would have a **critical certificate failure**.

* HTTPS web sites are now more or less a standard and to maintain security and user confidence it is essential that captive portals **DO NOT** attempt to capture port 443.

* All modern client devices have the built in, industry standard, *Captive Portal Detection (CPD) service*. This is responsible for triggering the captive portal splash/login page and is **specifically intended to make https capture unnecessary**.

What is CPD / Captive Portal Detection?
***************************************
CPD (Captive Portal Detection) has evolved as an enhancement to the network manager component included with major Operating Systems (Linux, Android, iOS/MacOS, Windows).

 Using a pre-defined port 80 web page (the one that gets used depends on the vendor) the network manager will detect the presence of a captive portal hotspot and notify the user. In addition, most major browsers now support CPD.
