# Testing
[NB: The current content is aspirational.  Not fully implemented yet.]

## Overview

The core purpose of DAWN is the processing of information relating to the
set of access points (AP) and clients (STA - meaning stations in 802.11
vernacular) that form the wi-fi network under DAWN's management.

The required data storage and processing capabilites are generally held in
the 'storage' part of DAWN's source code tree.  The remaining parts are
mainly for interfacing with resources of the AP environment rhat provide
updated information and allow DAWN to indicate to stations what they
should do next following processing of the latest data.  Specifically,
this means components such as iwinfo, ubus and uci that are commonly found
on an AP running OpenWRT variant of Linux.

## Testing Approach
The principal focus of DAWN's test harness is on these storage and
processing components.  This is achieved by having a build target named
test_storage which builds these parts without the environment interaction
dependencies.  This "test harness" can then be executed on the build host
(e.g. a Linux desktop development environment) or on the target AP.

To configure the imaginary AP (SUT, or "system under test" in testing
parlance) a script that mimics many of the messages from ubus and other
sources is created to represent the simulated network by describing APs,
connected STAs and DAWN's own configuration parameters.  The data
evaluation algorithms are then executed, and resultant outputs to stations
are simulated as text output.

For example consider a simple network of two AP with 2 stations.  The
following (simplifed) script is used to configure what one AP will be
aware of:

    CONFIG RSSI=10db
    SELF 01:01:01:01:01:01
    AP 02:02:02:02:02:02
    CONNECT 99:99:99:99:99:99 01:01:01:01:01:01 -78dB
    HEARING 99:99:99:99:99:99 02:02:02:02:02:02 -65dB
    CONNECT 88:88:88:88:88:88 01:01:01:01:01:01 -65dB
    HEARING 88:88:88:88:88:88 02:02:02:02:02:02 -65dB
    KICK CONSUME
    KICK

This means our test AP has the BSSID 01:..., and there is another AP in
the network with BSSID 02:....  Two stations with MAC 99:... and 88:...
are in the network, both connected to AP01:... but also able to see
AP02:....  The dB values indicate RSSI levels, and will be evalutated to
determine if stations are connected to an appropriate AP.  We'ed also
configured AP01:... to have an RSSI transition threshold of 10dB.  When
"kicking evaluation" is performed STA99:... can improve its RSSI by over
10dB by switching to AP02:..., so will be instructed to do so, resulting
in the test actions:

    REMOVE  99:99:99:99:99:99 01:01:01:01:01:01
    CONNECT 99:99:99:99:99:99 02:02:02:02:02:02 -65dB

Note that this is also valid input to the test_harness, and the parameter
CONSUME on the KICK action will cause it to be reinjested for
consideration when the second KICK action is evaluated.
	
## Types of Testing
Three main areas of testing are performed by the supplied test scripts:
* Data management: Ensuring data is stored correctly and soes not cause
buffer overruns, etc
* Algorithm functionality: Review the outcomes of evaluation for somple
and complex network data
* Scalability: Evaluate the ability of DAWN to sacle linearly to hundreds
and thousands of AP and STA

A number of scenarios are defined for each type of testing, along with
scripts to execute them.

### Data Management Scenarios
Data management scenarios excercise DAWN's internal data structures by
filling and emptying them to ensure no overflow conditions occur, and
where appropriate that sorting is applied correctly.  They are independent
of any functional testing, so each data structure is excercised alone in a
way that would never occur in real usage.

#### Test DM001: AP list
Fill, print, empty and print

#### Test DM002: Client list
Fill, print, empty and print

#### Test DM001: MAC list
Fill, print, empty and print

#### Test DM001: Hearing list
Fill, print, empty and print

### Algorithm Scenarios
Algorithm scenarios are used to ensure that DAWN creates the intended
outputs to manage which stations connect to APs.  They require more
intricate crafting of the scripted synthetic data to represent the input
to DAWN's evaluation algortithms to blend situation data with decision
metrics.

#### Test AL001: 2+2 Stable
Two AP with two stations connected to AP1, and will remain there.

#### Test AL002: 2+2 Cross
Two AP with two stations connected to AP1, both switching to AP2.

#### Test AL003: 2+1+1 Cross
Two AP with one station connected each, and will cross to the other.

#### Test AL003: Load balance
Three AP with 40 stations connected to AP1, all with same metrics.  DAWN
should balance load to other APs.


### Scalability Scenarios
Scalability scenarios explore how well DAWN can function in environments
with many APs and clients.  Using commodity equipment this might mean tens
of APs with a total of a thousand or so connected stations.  Subject to
use of appropriate hardware (eg small PC rather than SoC platform) there
is an aspiration for DAWN to be stable while managing hundreds of APs each
with tens of connected stations, allowing networks to support 10,000 or
more concurrent stations.

Scalability requires a number of design aspects to function together:
* Data storage scalability: To allow efficient use of memory the amount
required should be approximately linear, so simple networks on small
devices require small amounts of memory while more complex environments
can function on moderately scaled hardware.
* Algorithm scalability: Similar to memory usage, ensuring that
calculation times are more linear than exponential as numbers of APs and
stations grow.
* Algorithm efficiency: As well as being scaleable the processing of data
must fit within absolute limits to ensure that APs can keep up with the
rate of data arriving from the network.
