Forwarding Authentication Service (FAS)
#######################################

Overview
********
openNDS (NDS) has the ability to forward requests to a third party authentication service (FAS). This is enabled via simple configuration options.

These options are:
 1. **fasport**. This enables Forwarding Authentication Service (FAS). Redirection is changed from the default ThemeSpec to a separate FAS. The value is the IP port number of the FAS.
 2. **fasremoteip**. If set, this is the remote ip address of the FAS, if not set it will take the value of the NDS gateway address.
 3. **fasremotefqdn** If set, this is the remote fully qualified domain name (FQDN) of the FAS
 4. **faspath**. This is the path from the FAS Web Root (not the file system root) to the FAS login page.
 5. **fas_secure_enabled**. This can have four values, "0", "1", "2" or "3" providing different levels of security.
 6. **faskey** Used in combination with fas_secure_enable level 1, 2 and 3, this is a key phrase for NDS to encrypt data sent to FAS.

.. note::
 FAS (and Preauth/FAS) enables pre authentication processing. NDS authentication is the process that openNDS uses to allow a client device to access the Internet through the Firewall. In contrast, Forward Authentication is a process of "Credential Verification", after which FAS, if the verification process is successful, passes a request to NDS for access to the Internet to be granted for that client.

Using FAS
*********

**Note**:
All addresses (with the exception of fasremoteip) are relative to the *client* device, even if the FAS is located remotely.

When FAS is enabled, openNDS automatically configures firewall access to the FAS service.

The FAS service must serve a splash page of its own to replace the openNDS served output of the default ThemeSpec script. For fas_secure_enable "0", "1", and "2" this is enforced as http. For fas_secure_enable level "3", it is enforced as https.

Typically, the FAS service will be written in PHP or any other language that can provide dynamic web content.

FAS can then generate an action form for the client, typically requesting login, or self account creation for login.

The FAS can be on the same device as openNDS, on the same local area network as NDS, or on an Internet hosted web server.

Security
********

**If FAS Secure is enabled** (Levels 1 (default), 2 and 3), the client authentication token is kept secret at all times. Instead, faskey is used to generate a hashed id value (hid) and this is sent by openNDS to the FAS. The FAS must then in turn generate a new return hash id (rhid) to return to openNDS in its authentication request.

   **If set to "0"** The FAS is enforced by NDS to use **http** protocol.
   The client token is sent to the FAS in clear text in the query string of the redirect along with authaction and redir. This method is easy to bypass and useful only for the simplest systems where security does not matter.

   **If set to "1"** The FAS is enforced by NDS to use **http** protocol.
   A base64 encoded query string containing the hid is sent to the FAS, along with the clientip, clientmac, gatewayname, client_hid, gatewayaddress, authdir, originurl, clientif and custom parameters and variables.

   Should the sha256sum utility not be available, openNDS will terminate with an error message on startup.

   **If set to "2"** The FAS is enforced by NDS to use **http** protocol.

   clientip, clientmac, gatewayname, client_hid, gatewayaddress, authdir, originurl and clientif are encrypted using faskey and passed to FAS in the query string.

   The query string will also contain a randomly generated initialization vector to be used by the FAS for decryption.

   The cipher used is "AES-256-CBC".

   The "php cli" package and the "php openssl" module must both be installed for fas_secure levels 2 and 3.

   openNDS does not depend on this package and module, but will exit gracefully if this package and module are not installed when this level is set.

   The FAS must use the query string passed initialisation vector and the pre shared fas_key to decrypt the query string. An example FAS level 2 php script (fas-aes.php) is stored in the /etc/opennds directory and also supplied in the source code. This should be copied the the web root of a suitable web server for use.

   **If set to "3"** The FAS is enforced by openNDS to use **https** protocol.
   Level 3 is the same as level 2 except the use of https protocol is enforced for FAS. In addition, the "authmon" daemon is loaded. This allows the external FAS, after client verification, to effectively traverse inbound firewalls and address translation to achieve NDS authentication without generating browser security warnings or errors. An example FAS level 3 php script (fas-aes-https.php) is pre-installed in the /etc/opennds directory and also supplied in the source code. This should be copied the the web root of a suitable web server for use.

   **Option faskey has a default value.** It is recommended that this is set to some secret value in the config file and the FAS script set to use a matching value, ie faskey must be pre-shared with FAS.


Example FAS Query strings
*************************

  **Level 0** (fas_secure_enabled = 0)

   NDS sends the token and other information to FAS as clear text.

   `http://fasremoteip:fasport/faspath?authaction=http://gatewayaddress:gatewayport/opennds_auth/?clientip=[clientip]&gatewayname=[gatewayname]&tok=[token]&redir=[requested_url]`

   **Note: a knowledgeable user could bypass FAS, so running fas_secure_enabled at level 1, 2 or 3 is recommended.**


  **Level 1** (fas_secure_enabled = 1)

   NDS sends a query string containing a base64 encoded string containing required parameters and variables, plus any FAS variables generated in the client dialogue, such as username and email address. The client token is never exposed.

   Example Level 1 query string:

    `http://fasremotefqdn:fasport/faspath?fas=[b64encodedstring]&username=[client_username]&emailaddr=[client_email]`

   The b64encoded string will contain a comma space separated list of parameters.

   The decoded string received by FAS will be of the form:

   `[varname1]=[var1], [varname2]=[var2], ..... etc` (the separator being comma-space).

  For example:

   `clientip=[clientipaddress], clientmac=[clientmacaddress], gatewayname=[gatewayname], client token, gatewayaddress, authdir, originurl`

  The FAS must return the hash of the concatenated hid value and the value of faskey identified in the query string as "tok". NDS will automatically detect this.

  **Levels 2 and 3** (fas_secure_enabled = 2 and fas_secure_enabled = 3), openNDS sends encrypted information to FAS.

  `http://fasremotefqdn:fasport/faspath?fas=[aes-256-cbc data]&iv=[random initialisation vector]` (level 2)

  `https://fasremotefqdn:fasport/faspath?fas=[aes-256-cbc data]&iv=[random initialisation vector]` (level 3)

   It is the responsibility of FAS to decrypt the aes-256-cbc data it receives, using the pre shared faskey and the random initialisation vector.

  The decrypted string received by FAS will be of the form:
  [varname1]=[var1], [varname2]=[var2], ..... etc. (the separator being comma-space).

  For example:

    `clientip=[clientipaddress], clientmac=[clientmacaddress], gatewayname=[gatewayname], client token, gatewayaddress, authdir, originurl`

  It is the responsibility of FAS to parse the decrypted string for the variables it requires.

Example scripts
---------------

Full details of how to use FAS query strings can be seen in the example scripts, fas-hid.php, fas-aes.php and fas-aes-https.php

Custom Parameters
*****************

Custom Parameters are primarily intended to be used by remote configuration tools and are generally useful for passing static information to a remote FAS.

A list of Custom Parameters can be defined in the configuration file.
Once a custom parameter is defined in the configuration, its value will be fixed.

Parameters must be of the form param_name=param_value and may not contain white space or single quote characters.

Custom parameters are added to the base64 encoded query string when FAS level 1 is set or the basic login option is used. Note the basic login option is a special case of FAS level 1 running a ThemeSpec script.

Custom parameters are added to the encrypted query string when FAS levels 1, 2 and 3 are set.

The fas_custom_parameters_list option in the configuration file is used to set custom parameters. This is detailed in the default configuration file.

It is the responsibility of FAS to parse the query string for the custom parameters it requires.

Network Zones - Determining the Interface the Client is Connected To
********************************************************************

The Network coverage of a Captive Portal can take many forms, from a single SSID through to an extensive mesh network.

Using FAS, it is quite simple to dynamically adapt the Client Login page depending on the Network Zone a client is connected to.
NDS can determine the local interface or 802.11s mesh network node a client is using. A simple lookup table can then be included in a custom FAS, relating interfaces or mesh nodes to sensibly named coverage zones.

A very simple example would be a captive portal set up with a wireless network for "Staff", another for "Guests" and office machines connected via ethernet.

 * Ethernet connected office machines would gain access by simply clicking "Continue".
 * Staff mobiles connect to the Staff WiFi using a standard access code then clicking "Continue".
 * Guests connect to the open Guest Wifi and are required to enter details such as Name, email address etc.

NDS is aware of the interface or mesh node a client is using.

For a FAS using `fas_secure_enabled = 2`, an additional variable, clientif, is sent to the FAS in the encrypted query string (local or remote FAS).

For all other levels of fas_secure_enabled, PreAuth and BinAuth, the library utility "get_client_interface" is required to be used by the relevant script (local FAS only).

Working examples can be found in the included scripts:

 * fas-aes.php
 * ThemeSpec
 * demo-preauth.sh
 * demo-preauth-remote-image.sh

For details of the clientif variable and how to use get_client_interface, see the section **Library Utilities**.

After Successful Verification by FAS
************************************

If the client is successfully verified by the FAS, FAS will send the return hash id (rhid) to openNDS to finally allow the client access to the Internet.

Post FAS processing
*******************

Once the client has been authenticated by the FAS, NDS must then be informed to allow the client to have access to the Internet.

The method of achieving this depends upon the fas_secure_enabled level.

Authentication Method for fas_secure_enabled levels 0,1 and 2
-------------------------------------------------------------

 Once FAS has verified the client credentials, authentication is achieved by accessing NDS at a special virtual URL.

 This virtual URL is of the form:

 `http://[nds_ip]:[nds_port]/[authdir]/?tok=[token]&redir=[landing_page_url]&custom=`

 This is most commonly achieved using an html form of method GET.
 The parameter redir can be the client's originally requested URL sent by NDS, or more usefully, the URL of a suitable landing page.

The "login_option/Themespec" special case
+++++++++++++++++++++++++++++++++++++++++

The default "login_option" library, libopennds.sh, is a local script so has access to ndsctl auth method of authentication without needing the authmon daemon so uses this rather than the authdir GET method detailed above. This means Themespec can directly set client quotas without requiring BinAuth.

Authentication Method for fas_secure_enabled level 3 (Authmon Daemon)
---------------------------------------------------------------------

 When fas_secure_enabled level 3 is used (https protocol), post verification authentication is achieved by the openNDS Authmon daemon.

 Authmon is started by openNDS to facilitate NAT traversal and allow a remote https FAS to communicate with the local openNDS.

 FAS will deposit client authentication variables for the Authmon daemon to use for the authentication process. These variables are as follows:

 * rhid: The return hashed ID of the client to be authenticated
 * sessionlength: length of session - minutes
 * uploadrate: maximum allowed upload data rate - kbits/sec
 * downloadrate: maximum allowed download data rate - kbits/sec
 * uploadquota: allowed upload data quota - kBytes
 * downloadquota: allowed download data quota - kBytes
 * custom: A custom data string that will be sent to BinAuth

 Details can be found in the example script fas-aes-https.php

Be aware that many client CPD processes will **automatically close** the landing page as soon as Internet access is detected.

BinAuth Post FAS Processing
***************************

As BinAuth can be enabled at the same time as FAS, a BinAuth script may be used for custom post FAS processing. (see BinAuth).

The example BinAuth script, binauth_log.sh, is designed to locally log details of each client authentication and receives client data including the token, ipaddress and macaddress. In addition it receives the custom data string sent from FAS.

In addition, if option fas_secure_enabled is set to 3, binauth_log.sh sends a deauthentication log to the remote https FAS whenever a client is deauthenticated.

Manual Access of NDS Virtual URL
********************************

If the user of an already authenticated client device manually accesses the NDS Virtual URL, they will be redirected back to FAS with the "status" query string.

 This will be of the form:

 `http://fasremoteip:fasport/faspath?clientip=[clientip]&gatewayname=[gatewayname]&status=authenticated`

FAS should then serve a suitable error page informing the client user that they are already logged in.

Running FAS on your openNDS router
**************************************

FAS has been tested using uhttpd, lighttpd, ngnix, apache and libmicrohttpd.

**Running on OpenWrt with uhttpd/PHP**:

 A FAS service may run quite well on uhttpd (the web server that serves Luci) on an OpenWrt supported device with 8MB flash and 32MB ram but shortage of ram will be an issue if more than two or three clients log in at the same time.

 For this reason a device with a **minimum** of 8MB flash and 64MB ram is recommended.

A device with 16MB flash or greater and 128MB ram or greater is recommended as a target for serious development.

 *Although port 80 is the default for uhttpd, it is reserved for Captive Portal Detection so cannot be used for FAS. uhttpd can however be configured to operate on more than one port.*

 We will use port 2080 in this example.

 Install the module php8-cgi. Further modules may be required depending on your requirements.

 To enable FAS with php in uhttpd you must add the lines:

  ``list listen_http	0.0.0.0:2080``

  ``list interpreter ".php=/usr/bin/php-cgi"``

 to the /etc/config/uhttpd file in the config uhttpd 'main' or first section.

 The two important NDS options to set will be:

 1. fasport. We will use port 2080 for uhttpd

 2. faspath. Set to, for example, /myfas/fas.php,
    your FAS files being placed in /www/myfas/

Using a Shared Hosting Server for a Remote FAS
**********************************************

 A typical Internet hosted **shared** server will be set up to serve multiple domain names.

 To access yours, it is important to configure the two options:

  fasremoteip = the **ip address** of the remote server

  **AND**

  fasremotefqdn = the **Fully Qualified Domain name** of the remote server

Using a CDN (Content Delivery Network) Hosted Server for a Remote FAS
*********************************************************************

 This is essentially the same as using a Shared Hosting Server with the additional requirement of *also* adding fasremotefqdn to the Walled Garden configuration.

 The setting for fasremoteip should be one of the valid IPv4 addresses of your CDN service.


Using the FAS Example Scripts (fas-hid, fas-aes.php and fas-aes-https.php)
**************************************************************************

These three, fully functional, example FAS scripts are included in the package install and can be found in the /etc/opennds folder. To function, they need to be copied to the web root or a folder in the web root of your FAS http/php server.

fas-hid.php
-----------
**You can run the FAS example script, fas-hid.php**, locally on the same device that is running NDS, or remotely on an Internet based FAS server.

The use of http protocol is enforced. fas-hid is specifically targeted at local systems with insufficient resources to run PHP services, yet facilitate remote FAS support without exposing the client token or requiring the remote FAS to somehow access the local ndsctl.

**If run locally on the NDS device**, a minimum of 64MB of ram may be sufficient, but 128MB or more is recommended.

**If run on a remote FAS server**, a minimum of 32MB of ram on the local device may be sufficient, but 64MB or more is recommended.

fas-aes.php
-----------
**You can run the FAS example script, fas-aes.php**, locally on the same device that is running NDS (A minimum of 64MB of ram may be sufficient, but 128MB is recommended), or remotely on an Internet based FAS server. The use of http protocol is enforced.

fas-aes-https.php
-----------------
**You can run the FAS example script, fas-aes-https.php**, remotely on an Internet based https FAS server. The use of https protocol is enforced.

Library calls can be made on the openNDS router to send deauthentication information and custom strings to an https FAS, and this example script will write received data to a log file on the FAS server. The example binauth_log.sh script makes use of this functionality to inform the remote FAS of the deauthentication of a client.

On the openNDS device, a minimum of 64MB of ram may be sufficient, but 128MB is recommended.

Example Script File fas-aes.php
-------------------------------
Http protocol is enforced.

Assuming you have installed your web server of choice, configured it for port 2080 and added PHP support using the package php8-cgi, you can do the following.

 (Under other operating systems you may need to edit the opennds.conf file in /etc/opennds instead, but the process is very similar.)

 * Install the packages php8-cli and php8-mod-openssl

 * Create a folder for the FAS script eg: /[server-web-root]/nds/ on the Internet FAS server

 * Place the file fas-aes.php in /[server-web-root]/nds/

   (You can find it in the /etc/opennds directory.)

 * Edit the file /etc/config/opennds

  adding the lines:

    ``option fasport '2080'`` 

    ``option faspath '/nds/fas-aes.php'``

    ``option fas_secure_enabled '2'``

    ``option faskey '1234567890'``

 * Restart NDS using the command ``service opennds restart``

Example Script File fas-aes-https.php
-------------------------------------
Https protocol is enforced.

Assuming you have access to an Internet based https web server you can do the following.

 (Under other operating systems you may need to edit the opennds.conf file in /etc/opennds instead, but the process is very similar.)

 * Install the packages php8-cli and php8-mod-openssl on your NDS router

 * Create a folder for the FAS script eg: /[server-web-root]/nds/ on the Internet FAS server

 * Place the file fas-aes.php in /[server-web-root]/nds/

   (You can find it in the /etc/opennds directory.)

 * Edit the file /etc/config/opennds

  adding the lines:

    ``option fasport '443'`` (or the actual port in use if different)

    ``option faspath '/nds/fas-aes-https.php'``

    ``option fas_secure_enabled '3'``

    ``option faskey '1234567890'``

    ``option fasremoteip '46.32.240.41'`` (change this to the actual ip address of the remote server)

    ``option fasremotefqdn 'blue-wave.net'`` (change this to the actual FQDN of the remote server)

 * Restart NDS using the command ``service opennds restart``


Example Script File fas-hid.php
-------------------------------
Http protocol is enforced.

fas-hid.php can be configured to be run locally or remotely in the same basic way as fas-aes.

However it is targeted for use on devices with limited resources as it does not require openNDS to have locally installed php-cli modules.

It uses fas_secure_enabled level 1, but sends a digest of the client token to the remote FAS. The digest is created using faskey, so the client token is not exposed.

The fas-hid.php script then uses this digest along with the pre shared faskey to request authentication by openNDS, thus mitigating any requirement for remotely accessing ndsctl that otherwise would be required.

Assuming you have access to an Internet based http web server you can do the following.

 (Under other operating systems you may need to edit the opennds.conf file in /etc/opennds instead, but the process is very similar.)

 * Create a folder for the FAS script eg: /[server-web-root]/nds/ on the Internet FAS server

 * Place the file fas-hid.php in /[server-web-root]/nds/

   (You can find it in the /etc/opennds directory.)

 * Edit the file /etc/config/opennds

  adding the lines:

    ``option fasport '80'`` (or the actual port in use if different)

    ``option faspath '/nds/fas-hid.php'``

    ``option fas_secure_enabled '1'``

    ``option faskey '1234567890'``

    ``option fasremoteip '46.32.240.41'`` (change this to the actual ip address of the remote server)

    ``option fasremotefqdn 'blue-wave.net'`` (change this to the actual FQDN of the remote server)

 * Restart NDS using the command ``service opennds restart``


Changing faskey
***************

The value of option faskey should of course be changed, but must also be pre-shared with FAS by editing the example or your own script to match the new value.


