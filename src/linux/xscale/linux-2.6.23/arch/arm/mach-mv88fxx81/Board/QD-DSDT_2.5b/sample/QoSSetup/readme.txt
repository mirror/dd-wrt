========================================================================
		Priority Queue Setup for QoS
========================================================================

QuarterDeck Device has 4 Priority Queues to support QoS. The priority of
a frame is determined by (in priority order):
	1) The CPU's Trailer if enabled on the port.
	2) The DA address in the frame if the frame's DA address is in the address
		database with a priority defined.
	3) The IEEE 802.3ac Tag containing IEEE 802.1p priority information
		if enabled on the port.
	4) The IPv4 Type of Service (TOS)/DiffServ field or IPv6 Traffic Class 
		field if enabled on the port.
	5) The Port's default priority defined in DefPri.

This sample program will deal with the above 3) ~ 5) cases.

qos.c
	sampleQoS will enable using both IEEE 802.3ac Tag and IPv4/IPv6 Traffic 
	Class field and IEEE 802.3ac has a higher priority than IPv4/IPv6. 
	The following is the QoS mapping programmed by sampleQos:
	1) IEEE 802.3ac Tag (Priority 0 ~ 7, 3 bits)
		Priority 1~3 is using QuarterDeck Queue 0.
		Priority 0,4 is using QuarterDeck Queue 1.
		Priority 6,7 is using QuarterDeck Queue 2.
		Priority 5 is using QuarterDeck Queue 3.
	2) IPv4/IPv6 (Priority 0 ~ 63, 6 bits)
		Priority 0~7 is using QuaterDeck Queue 0.
		Priority 8~31 is using QuaterDeck Queue 1.
		Priority 32~55 is using QuaterDeck Queue 2.
		Priority 56~63 is using QuaterDeck Queue 3.
	3) Each port's default priority is set to 1.

			







