Stream5 High Availability (HA)
------------------------------
Michael Altizer <maltizer@sourcefire.com>

Documentation last updated 2013-11-21

##########################################
# THIS CODE IS STILL EXPERIMENTAL!
# DO NOT USE IN PRODUCTION ENVIRONMENTS.
# Please send any issues to the Snort team
##########################################

== Overview ==
Starting in Snort 2.9.5, there is limited support for sharing of session state
for the purposes of high availability.  Currently, it supports sharing the
following state from Stream5 lightweight sessions:
    * The Session Key
    * Session Flags
    * IP Protocol (requires TARGET_BASED)
    * Application Protocol (requires TARGET_BASED)
    * Direction
    * Ignore Direction

There are two types of Stream5 HA messages: updates and deletions.  Both types
use a serialized session key to describe the target session.  In the case of
a deletion message being received, there is no additional data and the target
session will deleted from the session cache if it is present.  Update messages,
on the other hand, may contain a number of data records, including the non-key
HA state enumerated above.  If the session to be updated was not present in the
cache, it will be created, otherwise it will be updated.  In either case, the
session will be placed into "HA standby" mode, indicating that the current
Snort instance is not the owner of the session.  Sessions remain in "HA standby"
mode until Snort receives a packet on them and will not time out normally,
although they will be pruned as normal when a cache is being overrun.

An HA update message will be generated when a session is marked as requiring
synchronization (the criteria for which differ per session type) and the
time is appropriate based on the configuration.  The two configurable time
constraints are Minimum Session Lifetime and Minimum Synchronization Interval.
In general, no HA update messages will be sent for a session prior to the
Minimum Session Lifetime having elapsed, and update messages on a given session
will not be sent more often than once per Minimum Synchronization Interval.
Certain critical changes to a session will result in these time constraints
being ignored.  A configured value of 0 for either time constraint will also
effectively result in it being ignored.

== Extensibility ==
There is a framework for preprocessors to register to generate and consume HA
data records built into the current implementation.  In order to register,
a preprocessor must call RegisterStreamHAFuncs() with a preprocessor ID, a
subcode unique to that preprocessor, the maximum size of its data record, and
pointers to the functions for producing and consuming its own data records.
Multiple different data records may be generated and consumed by a preprocessor
simply by registering additional functions with a different subcode.  The return
value from RegisterStreamHAFuncs() is a bit index that must be passed in to
Stream5SetHAPendingBit() to indicate that a session has changed and that an
update message should be generated.

NOTE: The current implementation is limited to a maximum of 8 such data records
being registered across all preprocessors.  (Determined by the width of
ha_pending_mask in the Stream5LWSession structure.)

== Configuration ==
In order to utilize the functionality, Snort must be configured and compiled
with --enable-ha.

In order to enable Stream5 HA, the 'enable_ha' option must be given to the
'stream5_global' directive.

There is a new preprocessor 'stream5_ha' configuration directive which accepts
the following options:
    * min_session_lifetime - The number of milliseconds a session must have lived
            before it will generate HA update messages.  It must be an integer
            between 0 and 65535.  Default: 0.
    * min_sync_interval - The number of milliseconds since the last update
            message was generated before a session will generate another. It
            must be an integer between 0 and 65535.  Default: 0.
    * startup_input_file - The name of a file for Snort to read HA messages from
            at startup to prime the session caches with saved state.
    * runtime_output_file - The name of a file to which Snort will write all HA
            messages that are generated while it is running.
    * use_side_channel - Indicates that all HA messages should also be sent to
            the side channel for processing.  No value.
            NOTE: Requires that Snort has been compiled with the Side Channel
            feature and it has been enabled.

== Example/Default Configuration ==
preprocessor stream5_global: track_udp yes, track_tcp yes, enable_ha
preprocessor stream5_ha: min_session_lifetime 1000, min_sync_interval 100,
    startup_input_file ha.in, runtime_output_file ha.out, use_side_channel
