/*
   Copyright (C) 2010 bg <bg_one@mail.ru>
*/
#ifndef CHAN_DONGLE_AT_CMD_QUEUE_H_INCLUDED
#define CHAN_DONGLE_AT_CMD_QUEUE_H_INCLUDED

#include <sys/time.h>			/* struct timeval */

#include <asterisk.h>
#include <asterisk/linkedlists.h>	/* AST_LIST_ENTRY */

#include "at_command.h"			/* at_cmd_t */
#include "at_response.h"		/* at_res_t */
#include "export.h"			/* EXPORT_DECL EXPORT_DEF */


typedef struct at_queue_cmd
{
	at_cmd_t		cmd;			/*!< command code */
	at_res_t		res;			/*!< expected responce code, can be RES_OK, RES_CMGR, RES_SMS_PROMPT */

	unsigned		flags;			/*!< flags */
#define ATQ_CMD_FLAG_DEFAULT	0x00				/*!< empty flags */
#define ATQ_CMD_FLAG_STATIC	0x01				/*!< data is static no try deallocate */
#define ATQ_CMD_FLAG_IGNORE	0x02				/*!< ignore response non match condition */

	struct timeval		timeout;		/*!< timeout value, started at time when command actually written on device */
#define ATQ_CMD_TIMEOUT_1S	1				/*!< timeout value  1 sec */
#define ATQ_CMD_TIMEOUT_2S	2				/*!< timeout value  2 sec */
#define ATQ_CMD_TIMEOUT_5S	5				/*!< timeout value  5 sec */
#define ATQ_CMD_TIMEOUT_10S	10				/*!< timeout value 10 sec */
#define ATQ_CMD_TIMEOUT_15S	15				/*!< timeout value 15 ses */
#define ATQ_CMD_TIMEOUT_40S	40				/*!< timeout value 40 ses */

	char*			data;			/*!< command and data to send in device */
	unsigned		length;			/*!< data length */
} at_queue_cmd_t;

/* initializers */
#define ATQ_CMD_INIT_STF(e,icmd,iflags,idata)	do {	\
	(e).cmd = (icmd);				\
	(e).res = RES_OK;				\
	(e).flags = iflags | ATQ_CMD_FLAG_STATIC;	\
	(e).timeout.tv_sec = ATQ_CMD_TIMEOUT_2S;	\
	(e).timeout.tv_usec = 0;			\
	(e).data = (char*)(idata);			\
	(e).length = STRLEN(idata);			\
	} while(0)
#define ATQ_CMD_INIT_ST(e,icmd,idata)		ATQ_CMD_INIT_STF(e, icmd, ATQ_CMD_FLAG_DEFAULT, idata)

#define ATQ_CMD_INIT_DYNF(e,icmd,iflags)	do {	\
	(e).cmd = (icmd);				\
	(e).res = RES_OK;				\
	(e).flags = iflags & ~ATQ_CMD_FLAG_STATIC;	\
	(e).timeout.tv_sec = ATQ_CMD_TIMEOUT_2S;	\
	(e).timeout.tv_usec = 0;			\
	} while(0)
#define ATQ_CMD_INIT_DYN(e,icmd)		ATQ_CMD_INIT_DYNF(e, icmd, ATQ_CMD_FLAG_DEFAULT)
#define ATQ_CMD_INIT_DYNI(e,icmd)		ATQ_CMD_INIT_DYNF(e, icmd, ATQ_CMD_FLAG_IGNORE)

/* static initializers */
#define ATQ_CMD_DECLARE_STFT(cmd,res,data,flags,s,u)	{ (cmd), (res), ATQ_CMD_FLAG_STATIC|flags, {(s), (u)}, (char*)(data), STRLEN(data) }
#define ATQ_CMD_DECLARE_STF(cmd,res,data,flags)		ATQ_CMD_DECLARE_STFT(cmd,res,data,flags,ATQ_CMD_TIMEOUT_2S,0)
//#define ATQ_CMD_DECLARE_STF(cmd,res,data,flags)	{ (cmd), (res), ATQ_CMD_FLAG_STATIC|flags, {ATQ_CMD_TIMEOUT_2S, 0}, (char*)(data), STRLEN(data) }
#define ATQ_CMD_DECLARE_ST(cmd,data)		ATQ_CMD_DECLARE_STF(cmd, RES_OK, data, ATQ_CMD_FLAG_DEFAULT)
#define ATQ_CMD_DECLARE_STI(cmd,data)		ATQ_CMD_DECLARE_STF(cmd, RES_OK, data, ATQ_CMD_FLAG_IGNORE)
#define ATQ_CMD_DECLARE_STIT(cmd,data,s,u)	ATQ_CMD_DECLARE_STFT(cmd, RES_OK, data, ATQ_CMD_FLAG_IGNORE,s,u)

#define ATQ_CMD_DECLARE_DYNFT(cmd,res,flags,s,u) { (cmd), (res),  flags & ~ATQ_CMD_FLAG_STATIC, {(s), (u)}, 0,      0 }
#define ATQ_CMD_DECLARE_DYNF(cmd,res,flags)	ATQ_CMD_DECLARE_DYNFT(cmd,res,flags,ATQ_CMD_TIMEOUT_2S,0)
//#define ATQ_CMD_DECLARE_DYNF(cmd,res,flags)	{ (cmd), (res),  flags & ~ATQ_CMD_FLAG_STATIC, {ATQ_CMD_TIMEOUT_2S, 0}, 0,      0 }
#define ATQ_CMD_DECLARE_DYN(cmd)		ATQ_CMD_DECLARE_DYNF(cmd, RES_OK, ATQ_CMD_FLAG_DEFAULT)
#define ATQ_CMD_DECLARE_DYNI(cmd)		ATQ_CMD_DECLARE_DYNF(cmd, RES_OK, ATQ_CMD_FLAG_IGNORE)
#define ATQ_CMD_DECLARE_DYNIT(cmd,s,u)		ATQ_CMD_DECLARE_DYNFT(cmd, RES_OK, ATQ_CMD_FLAG_IGNORE,s,u)
#define ATQ_CMD_DECLARE_DYN2(cmd,res)		ATQ_CMD_DECLARE_DYNF(cmd, res, ATQ_CMD_FLAG_DEFAULT)

typedef struct at_queue_task
{
	AST_LIST_ENTRY (at_queue_task) entry;

	unsigned	cmdsno;
	unsigned	cindex;
	struct cpvt*	cpvt;

	at_queue_cmd_t	cmds[0];
} at_queue_task_t;


EXPORT_DECL int at_queue_insert_const (struct cpvt * cpvt, const at_queue_cmd_t * cmds, unsigned cmdsno, int athead);
EXPORT_DECL int at_queue_insert (struct cpvt * cpvt, at_queue_cmd_t * cmds, unsigned cmdsno, int athead);
EXPORT_DECL int at_queue_insert_task (struct cpvt * cpvt, at_queue_cmd_t * cmds, unsigned cmdsno, int athead, at_queue_task_t ** task);
EXPORT_DECL void at_queue_handle_result (struct pvt * pvt, at_res_t res);
EXPORT_DECL void at_queue_flush (struct pvt * pvt);
EXPORT_DECL const at_queue_task_t * at_queue_head_task (const struct pvt * pvt);
EXPORT_DECL const at_queue_cmd_t * at_queue_head_cmd(const struct pvt * pvt);
EXPORT_DECL int at_queue_timeout(const struct pvt * pvt);
EXPORT_DECL int at_queue_run (struct pvt * pvt);

static inline const at_queue_cmd_t * at_queue_task_cmd (const at_queue_task_t * task)
{
	return task ? &task->cmds[task->cindex] : NULL;
}

/* direct device write, dangerouse */
/* TODO: move */
EXPORT_DECL int at_write (struct pvt * pvt, const char * buf, size_t count);
EXPORT_DECL size_t write_all (int fd, const char * buf, size_t count);
#endif /* CHAN_DONGLE_AT_CMD_QUEUE_H_INCLUDED */
