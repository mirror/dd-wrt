# DAWN Developer Guide
This is an introduction to DAWN for developers that want to understand how
it works so that they can modify, fix, etc its behaviour.

## Component Overview
DAWN collects and shares information about the APs and clients in the
network by using several standard components.  The below assumes it is
running on an OpenWrt device.

![OpenWrtInANuthshell](https://raw.githubusercontent.com/PolynomialDivision/upload_stuff/master/dawn_pictures/openwrt_in_a_nutshell_dawn.png)

Information about capability of the AP and current clients is discovered
via the hostapd and iwinfo.  The task to do this is scheduled by a timer.

hostapd also shares information about potential clients that are trying to
connect, as received in 802.11 PROBE, AUTH and ASSOC requests, plus BEACON
frames for 802.11k capable devices.

Communication with hostapd is via UBUS messages and callbacks.

This data is then exchanged with other DAWN instances that have been
discovered via uMDNS. This means that every DAWN instance has a list of
all client devices and APs in the network, plus which APs have seen a
client device (even if it has not joined the network).  The most tested
and stable method of doing this is via TCP.  There are options for
multicast and broadcast, but these are less tested and may fail.

Internally to DAWN these sets of data are stored as linked lists, which
allow scalability while ensuring that memory usage is reduced in a small
network.  Additional timers are used to remove "expired" data from these
lists, so only a reasonably recent set is retained.

Another timer triggers periodically to ask the AP to look at each locally
connected client and see if a better AP might be available.  This is
achieved by calculating a score for every AP that has a recent BEACON or
PROBE entry for the client based on signal strength, band, etc.  The
scoring mechanism is  described in the Configuration page.

If one or more other APs have a sufficiently better score then the DAWN
instance on the current AP of the device uses hostapd (via UBUS) to send
an 802.11v Disassociation Imminent frame encouraging it to select one of
the better APs listed in the frame's Neighbor Report.  This is followed by
Disassociation of the device (for 802.11v capable and legacy devices).

Additionally, for an 802.11k capable AP DAWN can use the BEACON reports
from other APs to create a Neighbor Report for the AP.  For instance, if
clients CL-A, CL-B and CL-C of AP-X report that they have reasonable
signal strength to AP-M, AP-Q and AP-T then the NR for AP-X will be AP-M +
AP-Q + AP-T.  Other APs in the network that those clients can't see will
not be in the NR.  For simpler networks DAWN can use a simple list of all
APs for the NR.

# Creating a DAWN Development Environment

The code base for the main DAWN application is C.  It makes heavy use of
pointers and pointer related structures such as linked lists.  Although it
is a single executable there are multiple code entry points that are used
by callback functions, etc. Familiarity with C memory management, simple
event-based programming and resource management in a multi-threaded
application is a baseline for working effectively with the code.

## Compiling DAWN

This section describes how to add the currently published version of DAWN
to a local OpenWrt build environment, so that you can add DAWN to locally
built firmware images or the Image Builder target.  It assumes you have
some familiarity with building OpenWrt, so is more of an "aide memoir"
than specific steps. In other words, just copying and pasting the below
will not work!

Clone OpenWrt

    https://git.openwrt.org/openwrt/openwrt.git

Update Feeds

    ./scripts/feeds update -a

Install DAWN

    ./scripts/feeds install dawn

Add DAWN to the build (under `Network -> dawn`)

    make menuconfig

Compile

    make package/dawn/compile

## Creating a local DAWN development instance
This section describes how to configure the OpenWrt build environment to
use an alternate source code version, so you can work on that code.  Do
the above Compiling Dawn steps first.

Enable local source code (under `Advanced Configuration -> Enable package
source-tree override`)

    make menuconfig

Now you need to clone DAWN, e.g. into your home directory

    git clone https://github.com/berlin-open-wireless-lab/DAWN.git ~/DAWN

You must then add a symlink. In the OpenWrt branch do something like

    ln -s ~/DAWN/.git/ ~/openwrt/feeds/packages/net/dawn/git-src

Now compile dawn

    make package/dawn/{clean,compile}

You should then be able to copy the dawn binary executable file to your
device to test your changes.

## DAWN Quality Assurance

### Runtime Messages
Helper macros and functions named dawnlog_... are provided to assist with appropriate
management of messages.  The general convention is to issue messages one
of the following priorities:
- ERROR (0): A significant runtime failure, such as memory exhaustion or
unexpected ability to write to a network socket.
- WARNING (0): An algorithmically unexpected event, such as no AP seeming
to be the host for a client device.
- ALWAYS (0): An infrequent information message that a user or
administrator will want to see, such as a device being kicked from one AP
to another.
- INFO (1): Reasonably frequent messages that indicate normal operation,
such as the arrival of hostapd messages.  The log should still be "user
friendly" at this level.
- TRACE (2): Messages that help indicate why a decision or code path was
taken.
- DEBUG (3): Verbose tracing of code paths, etc.

The minimum logging level (as specified in /etc/config/dawn) at which the
priority appears is indicated, eg (0).

### Memory handling
There are dawn_... wrappers for the standard C memory management functions
malloc(), free(), etc.  These help ensure that memory allocation is
managed correctly by building a list of resources that have been allocated
but not released.

While DAWN is running sending a HUP signal to the process will generate a
list of currently allocated blocks in the system log, visible with logread.

In a small network only a few tens of blocks are needed.  A longer list
may suggest a memory leak, and inspection of the list to find multiple
items allocated at the same line of code with steadily increasing sequence
numbers may indicate where it is arising.

### MUTEX Handling
DAWN has a set of data lists that are accessed by code which is triggered
by multiple event types (timer, network message, ubus message, etc).
Because of this there is a chance that data could become corrupted by
competing blocks of code, leading to invalid memory access, process
faults, etc.  Access to these resources is therefore managed by POSIX
Threads mutex objects.

These potential problems will not be seen on a platform that is highly
non-pre-emptive, so helper functions are available to help track down
mutex handling errors such as accessing a resource without an appropriate
lock.  This helps ensure that when the same code is run on platforms which
use pre-emptive scheduling it is less likely to hit threading bugs.

dawn_mutex_require() can be used to indicate that a segment of code is
accessing a resource that should be locked.  Often the lock should be
taken by a "higher up" block of code, and if it hasn't been a runtime
warning will appear in the system log.  Further messages in the logs from
dawn_mutex_lock() and ..._unlock() can be used to help review threading
issues, such as trying to take a lock twice, not at all, etc.

### Test Harness
In addition to the main dawn binary another named test_storage is built by
CMAKE.  This can be used to exercise the core data storage and scoring
algorithms to ensure that expected behaviour occurs.  At the time of
writing a number of *.script files exist that can be passed to
test_storage, but these haven't necessarily been maintained as the code
has evolved from when they were originally written.
