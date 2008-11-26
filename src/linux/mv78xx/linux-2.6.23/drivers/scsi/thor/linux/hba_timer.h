#if !defined(TIMER_H)

#define TIMER_H

#include "mv_include.h"

#define HBA_REQ_TIMER_AFTER_RESET 15
#define HBA_REQ_TIMER 10
#define HBA_REQ_TIMER_IOCTL (HBA_REQ_TIMER_AFTER_RESET+3)

enum _tag_hba_msg_state{
	MSG_QUEUE_IDLE=0,
	MSG_QUEUE_PROC
};

typedef struct _tag_hba_msg {
	MV_PVOID data;
	MV_U32   msg;
	MV_U32   param;
	struct  list_head msg_list;
}hba_msg;

#define MSG_QUEUE_DEPTH 32

typedef struct _tag_hba_msg_queue {
	spinlock_t lock;
	struct list_head free;
	struct list_head tasks;
	hba_msg msgs[MSG_QUEUE_DEPTH];
}hba_msg_queue;

void hba_house_keeper_init(void);
void hba_house_keeper_run(void);
void hba_house_keeper_exit(void);
void hba_msg_insert(void *data, unsigned int msg, unsigned int param);

void hba_init_timer(PMV_Request req);
void hba_remove_timer(PMV_Request req);
void hba_add_timer(PMV_Request req, int timeout,
		   MV_VOID (*function)(MV_PVOID data));

#define TIMER_INTERVAL			1000		// millisecond
#define MAX_TIMER_REQUEST		20			// same as the total number of devices
#define NO_CURRENT_TIMER		MAX_TAG_NUMBER + 1		// for each device to keep track

typedef struct _Timer_Request
{
	List_Head Queue_Pointer;
	MV_PVOID Context;
	MV_VOID (*Routine) (MV_PVOID);
	MV_BOOLEAN Valid;
	MV_U8 Reserved0[3];

	MV_U64 Time_Stamp;		// when this requested function wants to be called
} Timer_Request, *PTimer_Request;

#ifdef SUPPORT_TIMER

typedef struct _Timer_Module
{
	Timer_Request Running_Requests[MAX_TIMER_REQUEST];
	Tag_Stack Tag_Pool;

	MV_U64 Time_Stamp;		// current time
} Timer_Module, *PTimer_Module;

#else

typedef struct _Timer_Module
{
	MV_PVOID context;
	MV_VOID (*routine) (MV_PVOID);
} Timer_Module, *PTimer_Module;

#endif

/* 
 * Exposed functions 
 */
MV_U32 Timer_GetResourceQuota(MV_U16 max_io);

void Timer_Stop(PTimer_Module This);

void Timer_Initialize(
	IN OUT PTimer_Module This,
	IN MV_PU8 pool);	

MV_U8 Timer_AddRequest(	
	IN MV_PVOID extension,
	IN MV_U32 time_unit,
	IN MV_VOID (*routine) (MV_PVOID),
	IN MV_PVOID context
	);

void Timer_CheckRequest(	
	IN MV_PVOID extension
	);

void Timer_CancelRequest(
	IN MV_PVOID extension,
	IN MV_U8 request_index
	);

#endif /* TIMER_H */
