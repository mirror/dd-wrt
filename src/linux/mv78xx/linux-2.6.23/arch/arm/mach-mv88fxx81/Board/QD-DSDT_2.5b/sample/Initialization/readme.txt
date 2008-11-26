========================================================================
		QuaterDeck Initialization called by BSP
========================================================================

Initialization Sample Program will show how to initialize the QuaterDeck 
Driver/Device.

This Sample includes the following files:
	- msApiInit.c
	- qdSim.c
	- qdSimRegs.h
	- ev96122mii.c
	- osSem.c

msApiInit.c
	qdStart is the main function of this Sample and does the followings:
	1) register the required functions.(gtRegister API)
		1.1) readMii - BSP specific MII read function 
						(provided by BSP and required by all QuarterDeck API)
		1.2) writeMii - BSP specific MII write function 
						(provided by BSP and required by all QuarterDeck API)
		1.3) semCreate - OS specific semaphore create function.
						(provided by BSP and recommanded by QuarterDeck MAC 
						address database API)
		1.4) semDelete - OS specific semaphore delete function.
						(provided by BSP and recommanded by QuarterDeck MAC 
						address database API)
		1.5) semTake - OS specific semaphore take function.
						(provided by BSP and recommanded by QuarterDeck MAC 
						address database API)
		1.6) semGive - OS specific semaphore give function.
						(provided by BSP and recommanded by QuarterDeck MAC 
						address database API)
		Notes) The given example will use EV96122 BSP and QuarterDeck Simulator 
		as an example.

	2) Initialize BSP provided routine (if required).
		Notes) QuarterDeck Simulator needs to be initialized.(qdSimInit)

	3) Calls sysConfig routine.
		1.1) Input (GT_SYS_CONFIG) - CPU Port Number (Board Specific, 
		either port 5 or port 6) and Port state (either 1 for Forwarding mode 
		or 0 for Blocked mode)
		1.2) Output (GT_SYS_INFO) - Device ID, Base MII Address (either 0 or 
		0x10), Number of Ports, and CPU port number.

	4) Calls sysEnable (for future use.)

qdSim.c (QuaterDeck Simulator)
    Simulates QuaterDeck Device(88E6052)'s register map. When QuareterDeck API 
	try to read/write a bit or bits into QuaterDeck, the simulator will 
	redirect to its own memory place and performing the function very close to
	QuaterDeck. For example, 
	1) user can set/reset a certain bit of QuarterDeck registers
		(Phy,Port,and General registers).
	2) user can access ATU (flush, load, purge, etc. with max MAC addresses 
		of 32)
	3) user can manually generate an Interrupt and test the Interrupt routine.
	4) when user read a register, it will clear a certain register if it's a 
		Self Clear register.
	5) when user write a register, it will return ERROR if it's read only 
		register.
	Notes) Simulator can be used when user has no QuarterDeck device connected Board.

	Exported routines are :
		qdSimRead 	for reading MII registers,
		qdSimWrite 	for writing to MII registers, and
		qdSimInit 	for initializing Simulator.

ev96122mii.c
	Provides EV-96122 Board specific MII access functions.

	Exported routines are :
		gtBspReadMii 	for reading MII registers,
		gtBspWriteMii	for writing to MII registers, and
		gtBspMiiInit 	for initializing EV-96122 and QuarterDeck connection.

osSem.c
	Provides OS specific Semapore Functions.

	Exported routines are :
		osSemCreate 	for semaphore creation
		osSemDelete 	for semaphore deletion
		osSemWait 		for taking semaphore
		osSemSignal 	for releasing semaphore
