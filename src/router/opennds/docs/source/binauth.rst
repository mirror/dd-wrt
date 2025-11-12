BinAuth Option
=================

Overview
********

**BinAuth provides a method of running a post authentication script** or extension program. BinAuth is ALWAYS local to NDS and as such will have access to all the resources of the local system.

**BinAuth cannot be disabled.**

  By default, the script /usr/lib/opennds/binauth_log.sh is used

  This script manages the authenticated client database.

  In turn this database is used by openNDS to re-authenticate clients if a restart occurs.
  This is automatically achieved by a call to the **auth_restore** library function.
  After openNDS has restarted, clients that have remaining session time are re-authenticated.

Custom Binauth Script
*********************

  The custom binauth script **/usr/lib/opennds/custombinauth.sh** is called by binauth.

  This custom script can be edited to provide additional user defined functionality.

  **WARNING:** The default binauth script can be replaced using ``option binauth``, but this will disable the **auth_restore** and other significant functionality.

  ``option binauth '/usr/lib/opennds/my_binauth_script.sh'``

The Custom Binauth Variable
***************************

**A custom variable is forwarded to BinAuth**
  This can contain an embedded payload of custom data defined by the FAS. As FAS is typically remote from the NDS router, this provides a link to the local system.

Binauth Functionality
*********************

**1. BinAuth provides a means to override session timeout, data rate and data volume quotas** on a client by client basis.

**2. BinAuth is called by openNDS at the following times:**

 * After the client CPD browser makes an authentication request to openNDS
 * After the client device is granted Internet access by openNDS
 * After the client is deauthenticated by request
 * After the client idle timeout interval has expired
 * After the client session timeout interval has expired
 * After a data upload or download quota has been exceeded
 * After the client is authenticated by ndsctl command
 * After the client is deauthenticated by ndsctl command
 * After NDS has received a shutdown command

**3. Binauth provides the openNDS logging mechanism** for both local logs and remote FAS logs.

BinAuth Command Line Arguments
******************************

When OpenNDS calls the configured BinAuth script, it sends a set of command line arguments depending on the reason for the call.

BinAuth Command Methods
-----------------------

The first argument, arg[1], is always the "method".

The method will be set to one of the following values:
 * "**auth_client**" This is a request for authentication by the client.
 * "**client_auth**" This is an acknowledgement of successful authentication by NDS.
 * "**client_deauth**" This is an acknowledgement that the client has been deauthenticated by NDS.
 * "**idle_deauth**" - NDS has deauthenticated the client because the idle timeout duration has been exceeded.
 * "**timeout_deauth**" - NDS has deauthenticated the client because the session length duration has been exceeded.
 * "**downquota_deauth**" - NDS has deauthenticated the client because the client's download quota has been exceeded
 * "**upquota_deauth**" - NDS has deauthenticated the client because the client's upload quota has been exceeded
 * "**ndsctl_auth**" - NDS has authorised the client because of an ndsctl command (for example, sent by the NDS AuthMon daemon).
 * "**ndsctl_deauth**" - NDS has deauthenticated the client because of an ndsctl command.
 * "**shutdown_deauth**" - NDS has deauthenticated the client because it received a shutdown command.

Additional arguments depend on the method type:

Method auth_client
------------------
The first argument is auth_client and the following arguments are set to:

 * arg[2] = client_mac
 * arg[3] = username (deprecated)
 * arg[4] = password (deprecated)
 * arg[5] = url-escaped redir variable (the URL originally requested by the client.
 * arg[6] = url-escaped client user agent string
 * arg[7] = client_ip
 * arg[8] = client_token
 * arg[9] = url-escaped custom variable string

Method ndsctl_auth
------------------
The first argument is ndsctl_auth and the following arguments are set to:

 * arg[2] = client_mac
 * arg[3] = bytes_incoming (set to 0, reserved for future use)
 * arg[4] = bytes_outgoing (set to 0, reserved for future use)
 * arg[5] = session_start - the session start time
 * arg[6] = session_end - the session end time
 * arg[7] = client_token
 * arg[8] = url-escaped custom variable string

All Other Methods
-----------------
When the first argument is other than auth_client or ndsctl_auth, the following arguments are set to:

 * arg[2] = client_mac
 * arg[3] = bytes_incoming (total incoming bytes for client)
 * arg[4] = bytes_outgoing (total incoming bytes for client)
 * arg[5] = session_start - the session start time
 * arg[6] = session_end - the session end time
 * arg[7] = client_token

Using the Custom Variable string
--------------------------------
Method auth_client - arg[9] and ndsctl_auth - arg[8], contain the url-escaped custom variable string. openNDS extracts this variable from the query string of the http auth_client call from a FAS or ThemeSpec page.

It is provided for general unspecified use and is url-escaped.
A typical example of its use is for a level 0, 1, or 2 FAS to communicate special values for individual clients, or groups of clients.
