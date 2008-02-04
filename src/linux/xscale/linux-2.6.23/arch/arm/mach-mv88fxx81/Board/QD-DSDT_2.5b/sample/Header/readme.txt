========================================================================
		CPU Header Mode Enable or Disable
========================================================================

This sample shows how to enable/disable header mode for CPU port.
For more information about header mode, please refer to 88E6063 Spec.
section 3.5.10 and section 3.7.5.

Notes: 
When Header mode for the CPU port is enabled, Ethernet Device/Driver 
which is directly connected to the CPU port should understand Header Format.
If Ethernet Device does not know about Header mode, then user may set
the device to Promiscuous mode in order to receive packets from switch's CPU
port. After that, it is Ethernet Device Driver's responsibility to handle
Header properly.

header.c
	sampleHeaderEnable can be used to enable or disable CPU port's
	header mode







