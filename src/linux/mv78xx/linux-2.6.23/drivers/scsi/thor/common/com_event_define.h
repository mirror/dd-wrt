#ifndef COM_EVENT_DEFINE_H
#define COM_EVENT_DEFINE_H
//=======================================
//			Perceived Severity
//=======================================

#define SEVERITY_UNKNOWN	0
#define SEVERITY_OTHER		1
#define SEVERITY_INFO		2	
#define SEVERITY_WARNING	3	// used when its appropriate to let the user decide if action is needed
#define SEVERITY_MINOR		4	// indicate action is needed, but the situation is not serious at this time
#define SEVERITY_MAJOR		5	// indicate action is needed NOW
#define SEVERITY_CRITICAL	6	// indicate action is needed NOW and the scope is broad
#define SEVERITY_FATAL		7	// indicate an error occurred, but it's too late to take remedial action

//=======================================
//				Event Classes
//=======================================

#define	EVT_CLASS_ADAPTER			0
#define	EVT_CLASS_LD				1		// Logical Drive
#define	EVT_CLASS_HD				2		// Hard Drive
#define	EVT_CLASS_PM				3		// Port Multplier
#define	EVT_CLASS_EXPANDER			4
#define	EVT_CLASS_MDD				5 
#define	EVT_CLASS_BSL				6		// Bad Sector Lock

//=============================================================
//					Event Codes 
//
//	!!!  When adding an EVT_CODE, Please put its severity level
//  !!!  and suggested mesage string as comments.  This is the 
//  !!!  only place to document how 'Params' in 'DriverEvent' 
//  !!!  structure is to be used.
//=============================================================

//
// Event code for EVT_CLASS_LD (Logical Drive)
//

#define	EVT_CODE_LD_OFFLINE					0	// ("LD %d is offline", DeviceID)
#define	EVT_CODE_LD_ONLINE					1	// ("LD %d is online", DeviceID) 
#define	EVT_CODE_LD_CREATE					2	// ("LD %d is created", DeviceID)
#define	EVT_CODE_LD_DELETE					3	// ("LD %d has been deleted", DeviceID)
#define	EVT_CODE_LD_DEGRADE					4	// ("LD %d is degrading", DeviceID)
#define	EVT_CODE_LD_PARTIALLYOPTIMAL		5	// ("LD %d is in loose condition", DeviceID)
#define	EVT_CODE_LD_CACHE_MODE_CHANGE		6	// 
#define	EVT_CODE_LD_FIXED					7	// 
#define	EVT_CODE_LD_FOUND_ERROR				8	// 
// Note: Don't change the following 8 event code order! See raid_get_bga_event_id() for detail.
#define	EVT_CODE_LD_CHECK_START				9	// 
#define	EVT_CODE_LD_CHECK_RESTART			10	// 
#define	EVT_CODE_LD_CHECK_PAUSE				11	// 
#define	EVT_CODE_LD_CHECK_RESUME			12	// 
#define	EVT_CODE_LD_CHECK_ABORT				13	// 
#define	EVT_CODE_LD_CHECK_COMPLETE			14	// 
#define	EVT_CODE_LD_CHECK_PROGRESS			15	//  
#define	EVT_CODE_LD_CHECK_ERROR				16	// 
// Note: Don't change the following 8 event code order! See raid_get_bga_event_id() for detail. 
#define	EVT_CODE_LD_FIX_START				17	// 
#define	EVT_CODE_LD_FIX_RESTART				18	// 
#define	EVT_CODE_LD_FIX_PAUSE				19	// 
#define	EVT_CODE_LD_FIX_RESUME				20	// 
#define	EVT_CODE_LD_FIX_ABORT				21	// 
#define	EVT_CODE_LD_FIX_COMPLETE			22	// 
#define	EVT_CODE_LD_FIX_PROGRESS			23	// 
#define	EVT_CODE_LD_FIX_ERROR				24	//
// Note: Don't change the following 8 event code order!  See raid_get_bga_event_id() for detail.
#define	EVT_CODE_LD_INIT_QUICK_START		25	// ("Quick initialization of LD started", DevicdID)
#define	EVT_CODE_LD_INIT_QUICK_RESTART		26	// ("Quick initialization of LD restarted", DevicdID)
#define	EVT_CODE_LD_INIT_QUICK_PAUSE		27	// ("Quick initialization of LD paused", DevicdID)
#define	EVT_CODE_LD_INIT_QUICK_RESUME		28	// ("Quick initialization of LD resumed", DevicdID)
#define	EVT_CODE_LD_INIT_QUICK_ABORT		29	// ("Quick initialization of LD aborted", DevicdID)
#define	EVT_CODE_LD_INIT_QUICK_COMPLETE		30	// ("Quick initialization of LD completed", DevicdID)
#define	EVT_CODE_LD_INIT_QUICK_PROGRESS		31	// ("Quick initialization of LD %d is %d%% done", DevicdID, Params[0])
#define	EVT_CODE_LD_INIT_QUICK_ERROR		32	// ("Quick initialization of LD failed", DevicdID)
// Note: Don't change the following 8 event code order!  See raid_get_bga_event_id() for detail.
#define	EVT_CODE_LD_INIT_BACK_START			33	// ("Background initialization of LD started", DevicdID)
#define	EVT_CODE_LD_INIT_BACK_RESTART		34	// ("Background initialization of LD restarted", DevicdID)
#define	EVT_CODE_LD_INIT_BACK_PAUSE			35	// ("Background initialization of LD paused", DevicdID)
#define	EVT_CODE_LD_INIT_BACK_RESUME		36	// ("Background initialization of LD resumed", DevicdID)
#define	EVT_CODE_LD_INIT_BACK_ABORT			37	// ("Background initialization of LD aborted", DevicdID)
#define	EVT_CODE_LD_INIT_BACK_COMPLETE		38	// ("Background initialization of LD completed", DevicdID)
#define	EVT_CODE_LD_INIT_BACK_PROGRESS		39	// ("Background initialization of LD %d is %d%% done", DevicdID, Params[0])
#define	EVT_CODE_LD_INIT_BACK_ERROR			40	// ("Background initialization of LD failed", DevicdID)
// Note: Don't change the following 8 event code order!  See raid_get_bga_event_id() for detail.
#define	EVT_CODE_LD_INIT_FORE_START			41	// ("Foreground initialization of LD started", DevicdID)
#define	EVT_CODE_LD_INIT_FORE_RESTART		42	// ("Foreground initialization of LD restarted", DevicdID)
#define	EVT_CODE_LD_INIT_FORE_PAUSE			43	// ("Foreground initialization of LD paused", DevicdID)
#define	EVT_CODE_LD_INIT_FORE_RESUME		44	// ("Foreground initialization of LD resumed", DevicdID)
#define	EVT_CODE_LD_INIT_FORE_ABORT			45	// ("Foreground initialization of LD aborted", DevicdID)
#define	EVT_CODE_LD_INIT_FORE_COMPLETE		46	// ("Foreground initialization of LD completed", DevicdID)
#define	EVT_CODE_LD_INIT_FORE_PROGRESS		47	// ("Foreground initialization of LD %d is %d%% done", DevicdID, Params[0])
#define	EVT_CODE_LD_INIT_FORE_ERROR			48	// ("Fackground initialization of LD failed", DevicdID)
// Note: Don't change the following 8 event code order!  See raid_get_bga_event_id() for detail.
#define	EVT_CODE_LD_REBUILD_START			49	// 
#define	EVT_CODE_LD_REBUILD_RESTART			50	// 
#define	EVT_CODE_LD_REBUILD_PAUSE			51	// 
#define	EVT_CODE_LD_REBUILD_RESUME			52	// 
#define	EVT_CODE_LD_REBUILD_ABORT			53	// 
#define	EVT_CODE_LD_REBUILD_COMPLETE		54	// 
#define	EVT_CODE_LD_REBUILD_PROGRESS		55	// ("Rebuilding of LD %d is %d%% done", DevicdID, Params[0])
#define	EVT_CODE_LD_REBUILD_ERROR			56	// 
// Note: Don't change the following 8 event code order!  See raid_get_bga_event_id() for detail.
#define	EVT_CODE_LD_MIGRATION_START			57	// 
#define	EVT_CODE_LD_MIGRATION_RESTART		58	// 
#define	EVT_CODE_LD_MIGRATION_PAUSE			59	// 
#define	EVT_CODE_LD_MIGRATION_RESUME		60	// 
#define	EVT_CODE_LD_MIGRATION_ABORT			61	// 
#define	EVT_CODE_LD_MIGRATION_COMPLETE		62	// 
#define	EVT_CODE_LD_MIGRATION_PROGRESS		63	// ("Migration of LD %d is %d%% done", DevicdID, Params[0])
#define	EVT_CODE_LD_MIGRATION_ERROR			64	// 
//only used in application
#define	EVT_CODE_EVT_ERR					0xffff// 

//
// Event code for EVT_CLASS_HD (Hard Disk)
//

#define	EVT_CODE_HD_OFFLINE					0   // ("Disk %d is unplugged", DeviceID)
#define	EVT_CODE_HD_ONLINE					1   // ("Disk %d is plugged in", DeviceID)
#define	EVT_CODE_HD_SETDOWN					2   //disk setdown
#define	EVT_CODE_HD_TIMEOUT					3
#define	EVT_CODE_HD_RW_ERROR				4
#define	EVT_CODE_HD_SMART					5
#define	EVT_CODE_HD_ERROR_FIXED				6
#define	EVT_CODE_HD_PLUG_IN					7
#define	EVT_CODE_HD_PLUG_OUT				8
#define	EVT_CODE_HD_ASSIGN_SPARE			9
#define	EVT_CODE_HD_REMOVE_SPARE			10
#define	EVT_CODE_HD_SMART_THRESHOLD_OVER	11

//
// Event code for EVT_CLASS_MDD
//

#define	EVT_CODE_MDD_ERROR					0


//=======================================
//				Event IDs
//=======================================

//
// Event Id for EVT_CLASS_LD
//

#define	EVT_ID_LD_OFFLINE					( EVT_CLASS_LD << 16 | EVT_CODE_LD_OFFLINE )
#define	EVT_ID_LD_ONLINE					( EVT_CLASS_LD << 16 | EVT_CODE_LD_ONLINE ) 
#define	EVT_ID_LD_CREATE					( EVT_CLASS_LD << 16 | EVT_CODE_LD_CREATE )
#define	EVT_ID_LD_DELETE					( EVT_CLASS_LD << 16 | EVT_CODE_LD_DELETE )
#define	EVT_ID_LD_DEGRADE					( EVT_CLASS_LD << 16 | EVT_CODE_LD_DEGRADE )
#define	EVT_ID_LD_PARTIALLYOPTIMAL 			( EVT_CLASS_LD << 16 | EVT_CODE_LD_PARTIALLYOPTIMAL )
#define	EVT_ID_LD_CACHE_MODE_CHANGE			( EVT_CLASS_LD << 16 | EVT_CODE_LD_CACHE_MODE_CHANGE )
#define	EVT_ID_LD_FIXED						( EVT_CLASS_LD << 16 | EVT_CODE_LD_FIXED )
#define	EVT_ID_LD_FOUND_ERROR				( EVT_CLASS_LD << 16 | EVT_CODE_LD_FOUND_ERROR )

#define	EVT_ID_LD_CHECK_START				( EVT_CLASS_LD << 16 | EVT_CODE_LD_CHECK_START )
#define	EVT_ID_LD_CHECK_RESTART				( EVT_CLASS_LD << 16 | EVT_CODE_LD_CHECK_RESTART )
#define	EVT_ID_LD_CHECK_PAUSE				( EVT_CLASS_LD << 16 | EVT_CODE_LD_CHECK_PAUSE )
#define	EVT_ID_LD_CHECK_RESUME				( EVT_CLASS_LD << 16 | EVT_CODE_LD_CHECK_RESUME )
#define	EVT_ID_LD_CHECK_ABORT				( EVT_CLASS_LD << 16 | EVT_CODE_LD_CHECK_ABORT )
#define	EVT_ID_LD_CHECK_COMPLETE			( EVT_CLASS_LD << 16 | EVT_CODE_LD_CHECK_COMPLETE )
#define	EVT_ID_LD_CHECK_PROGRESS			( EVT_CLASS_LD << 16 | EVT_CODE_LD_CHECK_PROGRESS )
#define	EVT_ID_LD_CHECK_ERROR				( EVT_CLASS_LD << 16 | EVT_CODE_LD_CHECK_ERROR )

#define	EVT_ID_LD_FIXED_START				( EVT_CLASS_LD << 16 | EVT_CODE_LD_FIX_START )
#define	EVT_ID_LD_FIXED_RESTART				( EVT_CLASS_LD << 16 | EVT_CODE_LD_FIX_RESTART )
#define	EVT_ID_LD_FIXED_PAUSE				( EVT_CLASS_LD << 16 | EVT_CODE_LD_FIX_PAUSE )
#define	EVT_ID_LD_FIXED_RESUME				( EVT_CLASS_LD << 16 | EVT_CODE_LD_FIX_RESUME )
#define	EVT_ID_LD_FIXED_ABORT				( EVT_CLASS_LD << 16 | EVT_CODE_LD_FIX_ABORT )
#define	EVT_ID_LD_FIXED_COMPLETE			( EVT_CLASS_LD << 16 | EVT_CODE_LD_FIX_COMPLETE )
#define	EVT_ID_LD_FIXED_PROGRESS			( EVT_CLASS_LD << 16 | EVT_CODE_LD_FIX_PROGRESS )
#define	EVT_ID_LD_FIXED_ERROR				( EVT_CLASS_LD << 16 | EVT_CODE_LD_FIX_ERROR )

#define	EVT_ID_LD_INIT_QUICK_START			( EVT_CLASS_LD << 16 | EVT_CODE_LD_INIT_QUICK_START )
#define	EVT_ID_LD_INIT_QUICK_RESTART		( EVT_CLASS_LD << 16 | EVT_CODE_LD_INIT_QUICK_RESTART )
#define	EVT_ID_LD_INIT_QUICK_PAUSE			( EVT_CLASS_LD << 16 | EVT_CODE_LD_INIT_QUICK_PAUSE )
#define	EVT_ID_LD_INIT_QUICK_RESUME			( EVT_CLASS_LD << 16 | EVT_CODE_LD_INIT_QUICK_RESUME )
#define	EVT_ID_LD_INIT_QUICK_ABORT			( EVT_CLASS_LD << 16 | EVT_CODE_LD_INIT_QUICK_ABORT )
#define	EVT_ID_LD_INIT_QUICK_COMPLETE		( EVT_CLASS_LD << 16 | EVT_CODE_LD_INIT_QUICK_COMPLETE )
#define	EVT_ID_LD_INIT_QUICK_PROGRESS		( EVT_CLASS_LD << 16 | EVT_CODE_LD_INIT_QUICK_PROGRESS )
#define	EVT_ID_LD_INIT_QUICK_ERROR			( EVT_CLASS_LD << 16 | EVT_CODE_LD_INIT_QUICK_ERROR )

#define	EVT_ID_LD_INIT_BACK_START			( EVT_CLASS_LD << 16 | EVT_CODE_LD_INIT_BACK_START )
#define	EVT_ID_LD_INIT_BACK_RESTART			( EVT_CLASS_LD << 16 | EVT_CODE_LD_INIT_BACK_RESTART )
#define	EVT_ID_LD_INIT_BACK_PAUSE			( EVT_CLASS_LD << 16 | EVT_CODE_LD_INIT_BACK_PAUSE )
#define	EVT_ID_LD_INIT_BACK_RESUME			( EVT_CLASS_LD << 16 | EVT_CODE_LD_INIT_BACK_RESUME )
#define	EVT_ID_LD_INIT_BACK_ABORT			( EVT_CLASS_LD << 16 | EVT_CODE_LD_INIT_BACK_ABORT )
#define	EVT_ID_LD_INIT_BACK_COMPLETE		( EVT_CLASS_LD << 16 | EVT_CODE_LD_INIT_BACK_COMPLETE )
#define	EVT_ID_LD_INIT_BACK_PROGRESS		( EVT_CLASS_LD << 16 | EVT_CODE_LD_INIT_BACK_PROGRESS )
#define	EVT_ID_LD_INIT_BACK_ERROR			( EVT_CLASS_LD << 16 | EVT_CODE_LD_INIT_BACK_ERROR )

#define	EVT_ID_LD_INIT_FORE_START			( EVT_CLASS_LD << 16 | EVT_CODE_LD_INIT_FORE_START )
#define	EVT_ID_LD_INIT_FORE_RESTART			( EVT_CLASS_LD << 16 | EVT_CODE_LD_INIT_FORE_RESTART )
#define	EVT_ID_LD_INIT_FORE_PAUSE			( EVT_CLASS_LD << 16 | EVT_CODE_LD_INIT_FORE_PAUSE )
#define	EVT_ID_LD_INIT_FORE_RESUME			( EVT_CLASS_LD << 16 | EVT_CODE_LD_INIT_FORE_RESUME )
#define	EVT_ID_LD_INIT_FORE_ABORT			( EVT_CLASS_LD << 16 | EVT_CODE_LD_INIT_FORE_ABORT )
#define	EVT_ID_LD_INIT_FORE_COMPLETE		( EVT_CLASS_LD << 16 | EVT_CODE_LD_INIT_FORE_COMPLETE )
#define	EVT_ID_LD_INIT_FORE_PROGRESS		( EVT_CLASS_LD << 16 | EVT_CODE_LD_INIT_FORE_PROGRESS )
#define	EVT_ID_LD_INIT_FORE_ERROR			( EVT_CLASS_LD << 16 | EVT_CODE_LD_INIT_FORE_ERROR )

#define	EVT_ID_LD_REBUILD_START				( EVT_CLASS_LD << 16 | EVT_CODE_LD_REBUILD_START )
#define	EVT_ID_LD_REBUILD_RESTART			( EVT_CLASS_LD << 16 | EVT_CODE_LD_REBUILD_RESTART )
#define	EVT_ID_LD_REBUILD_PAUSE				( EVT_CLASS_LD << 16 | EVT_CODE_LD_REBUILD_PAUSE )
#define	EVT_ID_LD_REBUILD_RESUME			( EVT_CLASS_LD << 16 | EVT_CODE_LD_REBUILD_RESUME )
#define	EVT_ID_LD_REBUILD_ABORT				( EVT_CLASS_LD << 16 | EVT_CODE_LD_REBUILD_ABORT )
#define	EVT_ID_LD_REBUILD_COMPLETE			( EVT_CLASS_LD << 16 | EVT_CODE_LD_REBUILD_COMPLETE )
#define	EVT_ID_LD_REBUILD_PROGRESS			( EVT_CLASS_LD << 16 | EVT_CODE_LD_REBUILD_PROGRESS )
#define	EVT_ID_LD_REBUILD_ERROR				( EVT_CLASS_LD << 16 | EVT_CODE_LD_REBUILD_ERROR )

#define	EVT_ID_LD_MIGRATION_START			( EVT_CLASS_LD << 16 | EVT_CODE_LD_MIGRATION_START )
#define	EVT_ID_LD_MIGRATION_RESTART			( EVT_CLASS_LD << 16 | EVT_CODE_LD_MIGRATION_RESTART )
#define	EVT_ID_LD_MIGRATION_PAUSE			( EVT_CLASS_LD << 16 | EVT_CODE_LD_MIGRATION_PAUSE )
#define	EVT_ID_LD_MIGRATION_RESUME			( EVT_CLASS_LD << 16 | EVT_CODE_LD_MIGRATION_RESUME )
#define	EVT_ID_LD_MIGRATION_ABORT			( EVT_CLASS_LD << 16 | EVT_CODE_LD_MIGRATION_ABORT )
#define	EVT_ID_LD_MIGRATION_COMPLETE		( EVT_CLASS_LD << 16 | EVT_CODE_LD_MIGRATION_COMPLETE )
#define	EVT_ID_LD_MIGRATION_PROGRESS		( EVT_CLASS_LD << 16 | EVT_CODE_LD_MIGRATION_PROGRESS )
#define	EVT_ID_LD_MIGRATION_ERROR			( EVT_CLASS_LD << 16 | EVT_CODE_LD_MIGRATION_ERROR )

//
// Event Id for EVT_CLASS_HD
//

#define	EVT_ID_HD_OFFLINE					( EVT_CLASS_HD << 16 | EVT_CODE_HD_OFFLINE )
#define	EVT_ID_HD_ONLINE					( EVT_CLASS_HD << 16 | EVT_CODE_HD_ONLINE )
#define	EVT_ID_HD_SETDOWN					( EVT_CLASS_HD << 16 | EVT_CODE_HD_SETDOWN )
#define	EVT_ID_HD_TIMEOUT					( EVT_CLASS_HD << 16 | EVT_CODE_HD_TIMEOUT )
#define	EVT_ID_HD_RW_ERROR					( EVT_CLASS_HD << 16 | EVT_CODE_HD_RW_ERROR )
#define	EVT_ID_HD_SMART						( EVT_CLASS_HD << 16 | EVT_CODE_HD_SMART )
#define	EVT_ID_HD_ERROR_FIXED				( EVT_CLASS_HD << 16 | EVT_CODE_HD_ERROR_FIXED )
#define	EVT_ID_HD_PLUG_IN					( EVT_CLASS_HD << 16 | EVT_CODE_HD_PLUG_IN )
#define	EVT_ID_HD_PLUG_OUT					( EVT_CLASS_HD << 16 | EVT_CODE_HD_PLUG_OUT )
#define	EVT_ID_HD_ASSIGN_SPARE				( EVT_CLASS_HD << 16 | EVT_CODE_HD_ASSIGN_SPARE )
#define	EVT_ID_HD_REMOVE_SPARE				( EVT_CLASS_HD << 16 | EVT_CODE_HD_REMOVE_SPARE )
#define	EVT_ID_HD_SMART_THRESHOLD_OVER		( EVT_CLASS_HD << 16 | EVT_CODE_HD_SMART_THRESHOLD_OVER )

//
// Event Id for EVT_CLASS_MDD
//

#define	EVT_ID_MDD_ERROR					( EVT_CLASS_MDD << 16 | EVT_CODE_MDD_ERROR )


//
// Event Id for EVT_CLASS_ADAPTER
//

#define	EVT_ID_EVT_LOST					( EVT_CLASS_ADAPTER << 16 | EVT_CODE_EVT_ERR )

#endif

