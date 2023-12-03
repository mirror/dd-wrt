# Configuring DAWN

## Making Configuration Changes
Edit settings under

    /etc/config/dawn

After changes, restart daemon

    /etc/init.d/dawn restart

## Resetting DAWN's Configuration
The configuration for several key sections (see per section notes below)
can be shared with other DAWN
instances via ubus. This can be used to "reset" the configuration file to
remove any items that are no longer used.  To do this:

    # Backup current config
    cd /etc/config
    mv dawn dawn.old

    # Remove current config
    vi dawn # Delete all 'config metric' and 'config times' sections
    # Save and exit vi

    # On another DAWN instance
    ubus call dawn reload_config

The configuration file on the original DAWN instance should
now have newly written sections containing only current parameter values.
You can also start two instances of DAWN with an empty configuration file
then use this method to write the default configuration for the relevant
sections.

## What Can Be Configured?
The parameters that control DAWN are listed alphabetically below.  This
section describes how they are grouped to provide various features.

### Client 'Kicking'
DAWN's primary purpose is to move (aka "kick") client devices to the best
AP.  This is enabled via the `kicking` parameter.  If it is zero you will
see no devices being moved, but can use the log messages to see what would
happen.

See the following sections for details of the two main ways that DAWN may
decide to move a client device. If both methods are enabled RSSI Comparison
is evaluated first, and if it identifies a better target AP then the Absolute
RSSI method is not used.

Once DAWN has identified a device that might benefit from moving to a
different AP it does some final checks to ensure this won't be too disruptive.

- How busy the current client-AP connection is, see `bandwidth_threshold`
- Whether sufficiently better APs for the client have been consistently
found (rather than as a one-off, for example due to walking past a wall),
see `kicking_threshold` and `min_number_to_kick`

See also "Note 1: Legacy Clients"

### Kicking Method 1: RSSI Comparison
This is enabled if the `kicking` parameter is set to 1 or 3.

DAWN calculates a "score" for the actual or potential connection between
an AP and a client device.  These values are then compared to decide if a
client should be moved.  The score includes several factors:
- The radio connection quality, see "Note 2: RSSI Scoring"
- Support for 802.11 features that improve throughput, see `ht_support`,
`no_ht_support`, `vht_support` and `no_vht_support`
- How heavily the relevant radio is being used across all clients, see
`chan_util`, `chan_util_val`, `max_chan_util` and `max_chan_util_val`
- How many clients are connected to the AP, see `use_station_count` and
`max_station_diff`

### Kicking Method 2: Absolute RSSI
This is enabled if the `kicking` parameter is set to 2 or 3.

This is a simpler method where DAWN simply looks at the current RSSI radio
signal strength for the client device.  If it is below the `rssi_center`
value then a "soft" kick is done, which asks the device to look at other APs
but does not enforce this through a disconnection. Forced disconnection would be disruptive to legacy clients that may not be able to find a suitable AP to
connect to.

### Other Features
The following are less likely to need attention:
- The way 802.11v Neighbor Reports are generated can be adjusted.
    - `disassoc_nr_length` controls the number of entries in the list.
    - `set_hostapd_nr` controls the mechanism used to build the AP NR
        - "Static" will be sufficient for a small network, and contain all
the AP entries
        - "Dynamic" will allow a large network to automatically determine
a suitable set for each AP, containing a set of nearby APs
- 802.11k has several methods for gathering BEACON reports.  The
preference for Passive, Active or Table can be set via `rrm_mode`

## Configuration Notes
### Note 1: Legacy Clients
802.11v introduced the capability for clients to be "politely" asked to
move to a different AP, and DAWN uses this capability for clients that
appear to support it.

By definition, there is no way to do this for clients that don't implement
802.11v.  For these "legacy clients" DAWN can attempt to steer them away
during the PROBE / ASSOCIATE / AUTHENTICATE process by returning status
codes that indicate errors or unwillingness to accept.  It can also force
disconnection of a connected client by "tearing down" the connection,
however this is quite brutal as the client then must start a search for a
new AP, and it may just want to come back to the same AP. If DAWN
continues to try to not accept the client it is effectively denied WiFi
access.

If you enable this legacy client behaviour via parameters indicated then
you may hit challenges as it is less tested and reliable than the 802.11v
supported steering.  Reports on its success or otherwise are welcomed so
it can be refined if necessary and possible (within the constraints of
802.11).

See: `eval_probe_req`, `eval_auth_req`, `eval_assoc_req`,
`deny_auth_reason`, `deny_assoc_reason`, `min_probe_count` and `kicking`

### Note 2: RSSI Scoring
As a part of the scoring mechanism DAWN provides two mechanisms for
evaluating the client-AP RSSI (radio signal) quality.  Although DAWN does
not prevent both mechanisms being enabled at the same time (via the
relevant increment parameters) it may be difficult to obtain desired
behaviour.

Mechanism 1 is "stepped".  If the RSSI value is better than the `rssi_val`
value (or worse than the `low_rssi_val` value) then the AP score has the
`rssi` (or `low_rssi`) increment values applied.  This effective creates
three "zones" of RSSI score, which may be sufficient for many cases.  To
disable this mode set both increment values to zero.

Mechanism 2 is "graduated".  For each dB that the RSSI signal differs from
the `rssi_center` value the increment `rssi_weight` is applied.  This can
provide a more refined score, but may require more effort to get the
parameters optimised.  To disable this mode set the increment value to
zero.

## Global Metric Parameters
These parameters go in the following section:

    config metric 'global'

This section is shared by `ubus call dawn reload_config`.

<!-- Use the following shell command to auto-generate the table rows from
DAWN source code:
grep 'CONFIG-G:' `find . -type f -name "*.[ch]"`| sed 's/^.*CONFIG-.: *\(.*\)$/|\1|/'|sort
-->
|Parameter|Purpose|Notes [Default is bracketed]|
|---------|-------|-----|
|bandwidth_threshold|Maximum reported AP-client bandwidth permitted when kicking.  Set to zero to disable the check.|[6] (Mbits/s)|
|chan_util_avg_period|Number of sampling periods to average channel utilization values over|[3]|
|deny_assoc_reason|802.11 code used when ASSOCIATION is denied|[17] (802.11 AP_UNABLE_TO_HANDLE_NEW_STA). See Note 1.|
|deny_auth_reason|802.11 code used when AUTHENTICATION is denied|[1] (802.11 UNSPECIFIED_FAILURE). See Note 1.|
|disassoc_nr_length|Number of entries to include in a 802.11v DISASSOCIATE Neighbor Report|[6] (Documented for use by iOS)|
|duration|802.11k BEACON request DURATION parameter|[0]|
|eval_assoc_req|Control whether ASSOCIATION frames are evaluated for rejection|[0 = No evaluation]; 1 = Evaluated. See Note 1.|
|eval_auth_req|Control whether AUTHENTICATION frames are evaluated for rejection|[0 = No evaluation]; 1 = Evaluated. See Note 1.|
|eval_probe_req|Control whether PROBE frames are evaluated for rejection|[0 = No evaluation]; 1 = Evaluated. See Note 1.|
|kicking|Method to select clients to move to better AP|0 = Disabled; 1 = RSSI Comparison; 2 = Absolute RSSI; [3 = Both]. See note 1.|
|kicking_threshold|Minimum score difference to consider kicking to alternate AP|[20]|
|max_station_diff|Number of connected stations to consider "better" for use_station_count|[1]|
|min_number_to_kick|Number of consecutive times a client should be evaluated as ready to kick before actually doing it|[3]|
|min_probe_count|Number of times a client should retry PROBE before acceptance| [3] See Note 1.|
|neighbors|Space separated list of MACS to use in "static" AP Neighbor Report| None|
|rrm_mode|Preferred order for using Passive, Active or Table 802.11k BEACON information|[PAT] String of 'P', 'A' and / or 'T'|
|set_hostapd_nr|Method used to set Neighbor Report on AP|[0 = Disabled]; 1 = "Static" based on all APs in network (plus set from configuration); 2 = "Dynamic" based on next nearest AP seen by current clients|
|use_station_count|Compare connected station counts when considering kicking|[0 = Disabled]; 1 = Enabled|

## Per Band Metric Parameters
These parameters are repeated in the following sections for each band:

    config metric '802_11a'
    config metric '802_11g'

These sections are shared by `ubus call dawn reload_config`.

<!-- Use the following shell command to auto-generate the table rows from
DAWN source code:
grep 'CONFIG-B:' `find . -type f -name "*.[ch]"`| sed 's/^.*CONFIG-.: *\(.*\)$/|\1|/'|sort
-->
|Parameter|Purpose|Notes [Default is bracketed]|
|---------|-------|-----|
|ap_weight|Per AP weighting|[0] (Deprecated)|
|chan_util|Score increment if channel utilization is below chan_util_val|[0]|
|chan_util_val|Upper threshold for good channel utilization|[140]|
|ht_support|Score increment if HT is supported|[5]|
|initial_score|Base score for AP based on operating band|[2.4GHz = 80; 5Ghz = 100]|
|low_rssi|Score addition when signal is below threshold|[-15] See note 2.|
|low_rssi_val|Threshold for bad RSSI|[-80] See note 2.|
|max_chan_util|Score increment if channel utilization is above max_chan_util_val|[-15]|
|max_chan_util_val|Lower threshold for bad channel utilization|[170]|
|no_ht_support|Score increment if HT is not supported|[0] (Deprecated)|
|no_vht_support|Score increment if VHT is not supported|[0] (Deprecated)|
|rssi_center|Midpoint for weighted RSSI evaluation|[-70] See note 2.|
|rssi|Score addition when signal exceeds threshold|[15] See note 2.|
|rssi_val|Threshold for a good RSSI|[-60] See note 2.|
|rssi_weight|Per dB increment for weighted RSSI evaluation|[0] See note 2.|
|vht_support|Score increment if VHT is supported|[5]|

## Networking Parameters
TCP networking with UMDNS and without encryption is the most tested and
stable configuration.

Encryption has been reported to be broken, so use it with caution.

Other parameters have fallen out of use but remain in the code.  A tidy up
of them is due.

These parameters go in the following section:

    config network

This section is NOT shared by `ubus call dawn reload_config`.
<!-- Use the following shell command to auto-generate the table rows from
DAWN source code:
grep 'CONFIG-N:' `find . -type f -name "*.[ch]"`|sed 's/^.*CONFIG-.: *\(.*\)$/|\1|/'|sort
-->
|Parameter|Purpose|Notes [Default is bracketed]|
|---------|-------|-----|
|bandwidth|Unused|N/A|
|broadcast_ip|IP address for broadcast and multicast|No default|
|broadcast_port|IP port for broadcast and multicast|[1025]|
|collision_domain|Unused|N/A|
|iv|Unused|N/A|
|network_option|Method of networking between DAWN instances|0 = Broadcast; 2 = Multicast; [2 = TCP with UMDNS discovery]; 3 = TCP w/out UMDNS discovery|
|server_ip|IP address when not using UMDNS|No default|
|shared_key|Unused|N/A|
|tcp_port|Port for TCP networking|[1026]|
|use_symm_enc|Enable encryption of network traffic|[0 = Disabled]; 1 = Enabled|


## Local Parameters
These parameters go in the following section:

    config local

This section is NOT shared by `ubus call dawn reload_config`.

<!-- Use the following shell command to auto-generate the table rows from
DAWN source code:
grep 'CONFIG-L:' `find . -type f -name "*.[ch]"`|sed 's/^.*CONFIG-.: *\(.*\)$/|\1|/'|sort
-->
|Parameter|Purpose|Notes [Default is bracketed]|
|---------|-------|-----|
|loglevel|Verbosity of messages in syslog|[0 = Deeper tracing to fix bugs - for debugging]; 1 = More info to help trace where algorithms may be going wrong - for debugging; 2 = Reporting on standard behaviour; 3 = Standard behaviour always worth reporting; 4 = Something appears wrong, but recoverable; 5 = Serious malfunction / unexpected behaviour|


## Timing / Scheduling Parameters
All timer values are in seconds.  They are the main mechanism for DAWN
collecting and managing much of the data that it relies on.

These parameters go in the following section:

    config times

This section is shared by `ubus call dawn reload_config`.

<!-- Use the following shell command to auto-generate the table rows from
DAWN source code:
grep 'CONFIG-T:' `find . -type f -name "*.[ch]"`|sed 's/^.*CONFIG-.: *\(.*\)$/|\1|/'|sort
-->
|Parameter|Purpose|Notes [Default is bracketed]|
|---------|-------|-----|
|con_timeout|Timespan until a connection is seen as disconnected|[60]|
|remove_ap|Timer to remove expired AP entries from core data set|[460]|
|remove_client|Timer to remove expired client entries from core data set|[15]|
|remove_probe|Timer to remove expired PROBE and BEACON entries from core data set|[30]|
|update_beacon_reports|Timer to ask all connected clients for a new BEACON REPORT|[20]|
|update_chan_util|Timer to get recent channel utilization figure for each local BSSID|[5]|
|update_client|Timer to refresh local connection information and send revised NEIGHBOR REPORT to all clients|[10]|
|update_hostapd|Timer to (re-)register for hostapd messages for each local BSSID|[10]|
|update_tcp_con|Timer to refresh / remove the TCP connections to other DAWN instances found via uMDNS|[10]|

## hostapd Parameters
These parameters go in the following section:

    config hostapd

This section is NOT shared by `ubus call dawn reload_config`.

<!-- Use the following shell command to auto-generate the table rows from
DAWN source code:
grep 'CONFIG-H:' `find . -type f -name "*.[ch]"`|sed 's/^.*CONFIG-.: *\(.*\)$/|\1|/'|sort
-->
|Parameter|Purpose|Notes [Default is bracketed]|
|---------|-------|-----|
|hostapd_dir|Path to hostapd runtime information|[/var/run/hostapd]|
