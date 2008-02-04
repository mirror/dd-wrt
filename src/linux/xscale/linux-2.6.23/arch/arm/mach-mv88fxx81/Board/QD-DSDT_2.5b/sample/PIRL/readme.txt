==================================================================
		PIRL (Port based Ingress Rate Limit) Setup 
==================================================================

88E6065 device family (such as 88E6055 and 88E6035) and recent Marvell SOHO Switch Devices
support 'Best-in-Class' per port TCP/IP ingress rate limiting (based on some kind of a bucket scheme to keep track of the bandwidth) along with independent Storm prevention. 

This sample shows how to initialize each bucket and what kind of parameters are required.

pirl.c
	samplePIRLSetup is the main function for the PIRL setup.
	







