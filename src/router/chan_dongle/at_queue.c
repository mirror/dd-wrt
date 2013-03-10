/* 
   Copyright (C) 2009 - 2010

   Artem Makhutov <artem@makhutov.org>
   http://www.makhutov.org

   Dmitry Vagin <dmitry2004@yandex.ru>

   Copyright (C) 2010 - 2011
   bg <bg_one@mail.ru>
*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <asterisk.h>
#include <asterisk/utils.h>		/* ast_free() */

#include "at_queue.h"
#include "chan_dongle.h"		/* struct pvt */

/*!
 * \brief Free an item data
 * \param cmd - struct at_queue_cmd
 */
#/* */
static void at_queue_free_data(at_queue_cmd_t * cmd)
{
	if(cmd->data)
	{
		if((cmd->flags & ATQ_CMD_FLAG_STATIC) == 0) 
		{
			ast_free (cmd->data);
			cmd->data = NULL;
		}
		/* right because work with copy of static data */
	}
	cmd->length = 0;
}

/*!
 * \brief Free an item
 * \param e -- struct at_queue_task structure
 */
#/* */
static void at_queue_free (at_queue_task_t * task)
{
	unsigned no;
	for(no = 0; no < task->cmdsno; no++)
	{
		at_queue_free_data(&task->cmds[no]);
	}
	ast_free (task);
}


/*!
 * \brief Remove an job item from the front of the queue, and free it
 * \param pvt -- pvt structure
 */
#/* */
static void at_queue_remove (struct pvt * pvt)
{
	at_queue_task_t * task = AST_LIST_REMOVE_HEAD (&pvt->at_queue, entry);

	if (task)
	{
		PVT_STATE(pvt, at_tasks)--;
		PVT_STATE(pvt, at_cmds) -= task->cmdsno - task->cindex;
		ast_debug (4, "[%s] remove task with %u command(s) begin with '%s' expected response '%s' from queue\n", 
				PVT_ID(pvt), task->cmdsno, at_cmd2str (task->cmds[0].cmd), 
				at_res2str (task->cmds[0].res));

		at_queue_free(task);
	}
}

#/* */
static at_queue_cmd_t* at_queue_head_cmd_nc (const struct pvt * pvt)
{
	at_queue_task_t * e = AST_LIST_FIRST (&pvt->at_queue);
	if(e)
		return &e->cmds[e->cindex];
	return NULL;
}


/*!
 * \brief Add an list of commands (task) to the back of the queue
 * \param cpvt -- cpvt structure
 * \param cmds -- the commands that was sent to generate the response
 * \param cmdsno -- number of commands
 * \param prio -- priority 0 mean put at tail
 * \return task on success, NULL on error
 */
#/* */
static at_queue_task_t * at_queue_add (struct cpvt * cpvt, const at_queue_cmd_t * cmds, unsigned cmdsno, int prio)
{
	at_queue_task_t * e = NULL;
	if(cmdsno > 0)
	{
		e = ast_malloc (sizeof(*e) + cmdsno * sizeof(*cmds));
		if(e)
		{
			pvt_t * pvt = cpvt->pvt;
			at_queue_task_t * first;

			e->entry.next = 0;
			e->cmdsno = cmdsno;
			e->cindex = 0;
			e->cpvt = cpvt;

			memcpy(&e->cmds[0], cmds, cmdsno * sizeof(*cmds));


			if(prio && (first = AST_LIST_FIRST (&pvt->at_queue)))
				AST_LIST_INSERT_AFTER (&pvt->at_queue, first, e, entry);
			else
				AST_LIST_INSERT_TAIL (&pvt->at_queue, e, entry);

			PVT_STATE(pvt, at_tasks) ++;
			PVT_STATE(pvt, at_cmds) += cmdsno;

			PVT_STAT(pvt, at_tasks) ++;
			PVT_STAT(pvt, at_cmds) += cmdsno;

			ast_debug (4, "[%s] insert task with %u commands begin with '%s' expected response '%s' %s of queue\n", 
					PVT_ID(pvt), e->cmdsno, at_cmd2str (e->cmds[0].cmd), 
					at_res2str (e->cmds[0].res), prio ? "after head" : "at tail");
		}
	}
	return e;
}


/*!
 * \brief Write to fd
 * \param fd -- file descriptor
 * \param buf -- buffer to write
 * \param count -- number of bytes to write
 *
 * This function will write count characters from buf. It will always write
 * count chars unless it encounters an error.
 *
 * \retval number of bytes wrote
 */

#/* */
EXPORT_DEF size_t write_all (int fd, const char* buf, size_t count)
{
	ssize_t out_count;
	size_t total = 0;
	unsigned errs = 10;

	while (count > 0)
	{
		out_count = write (fd, buf, count);
		if (out_count <= 0)
		{
			if(errno == EINTR || errno == EAGAIN)
			{
				errs--;
				if(errs != 0)
					continue;
			}
			break;
		}
		errs = 10;
		count -= out_count;
		buf += out_count;
		total += out_count;
	}
	return total;
}

/*!
 * \brief Write to fd
 * \param pvt -- pvt structure
 * \param buf -- buffer to write
 * \param count -- number of bytes to write
 *
 * This function will write count characters from buf. It will always write
 * count chars unless it encounters an error.
 *
 * \retval !0 on error
 * \retval  0 success
 */

#/* */
EXPORT_DEF int at_write (struct pvt* pvt, const char* buf, size_t count)
{
	size_t wrote;

	ast_debug (5, "[%s] [%.*s]\n", PVT_ID(pvt), (int) count, buf);

	wrote = write_all(pvt->data_fd, buf, count);
	PVT_STAT(pvt, d_write_bytes) += wrote;
	if(wrote != count)
	{
		ast_debug (1, "[%s] write() error: %d\n", PVT_ID(pvt), errno);
	}

	return wrote != count;
}

/*!
 * \brief Remove an cmd item from the front of the queue
 * \param pvt -- pvt structure
 */
#/* */
EXPORT_DEF void at_queue_remove_cmd (struct pvt* pvt, at_res_t res)
{
	at_queue_task_t * task = AST_LIST_FIRST (&pvt->at_queue);

	if (task)
	{
		unsigned index = task->cindex;

		task->cindex++;
		PVT_STATE(pvt, at_cmds)--;
		ast_debug (4, "[%s] remove command '%s' expected response '%s' real '%s' cmd %u/%u flags 0x%02x from queue\n", 
				PVT_ID(pvt), at_cmd2str (task->cmds[index].cmd), 
				at_res2str (task->cmds[index].res), at_res2str (res), 
				task->cindex, task->cmdsno, task->cmds[index].flags);

		if((task->cindex >= task->cmdsno) || (task->cmds[index].res != res && (task->cmds[index].flags & ATQ_CMD_FLAG_IGNORE) == 0))
		{
			at_queue_remove(pvt);
		}
	}
}

/*!
 * \brief Try real write first command on queue
 * \param pvt -- pvt structure
 * \return 0 on success, non-0 on error
 */
#/* */
EXPORT_DEF int at_queue_run (struct pvt * pvt)
{
	int fail = 0;
	at_queue_cmd_t * cmd = at_queue_head_cmd_nc(pvt);

	if(cmd)
	{
		if(cmd->length > 0)
		{
			ast_debug (4, "[%s] write command '%s' expected response '%s' length %u\n", 
					PVT_ID(pvt), at_cmd2str (cmd->cmd), at_res2str (cmd->res), cmd->length);

			fail = at_write(pvt, cmd->data, cmd->length);
			if(fail)
			{
				ast_log (LOG_ERROR, "[%s] Error write command '%s' expected response '%s' length %u, cancel\n", PVT_ID(pvt), at_cmd2str (cmd->cmd), at_res2str (cmd->res), cmd->length);
				at_queue_remove_cmd(pvt, cmd->res + 1);
			}
			else
			{
				/* set expire time */
				cmd->timeout = ast_tvadd (ast_tvnow(), cmd->timeout);

				/* free data and mark as written */
				at_queue_free_data(cmd);
			}
		}
#if 0
		else
		{
			/* check expiration */
			if(ast_tvcmp (ast_tvnow(), cmd->timeout) > 0)
			{
				ast_log (LOG_ERROR, "[%s] Error  command '%s' expected response '%s' expired, cancel\n", PVT_ID(pvt), at_cmd2str (cmd->cmd), at_res2str (cmd->res));
				at_queue_remove_cmd(pvt, cmd->res + 1);
				fail = -1;
			}
		}
#endif /* 0 */
	}
	/* else empty nothing todo */
	return fail;
}

/*!
 * \brief Write commands with queue
 * \param pvt -- pvt structure
 * \return 0 on success non-0 on error
 */
#/* */
EXPORT_DEF int at_queue_insert_const (struct cpvt * cpvt, const at_queue_cmd_t * cmds, unsigned cmdsno, int athead)
{
	return at_queue_add(cpvt, cmds, cmdsno, athead) == NULL || at_queue_run(cpvt->pvt);
}

#/* */
EXPORT_DEF int at_queue_insert_task (struct cpvt * cpvt, at_queue_cmd_t * cmds, unsigned cmdsno, int athead, at_queue_task_t ** task)
{
	unsigned idx;
	task[0] = at_queue_add(cpvt, cmds, cmdsno, athead);

	if(!task[0])
	{
		for(idx = 0; idx < cmdsno; idx++)
		{
			at_queue_free_data(&cmds[idx]);
		}
	}

	if(at_queue_run(cpvt->pvt))
		task[0] = NULL;

	return task[0] == NULL;
}

#/* */
EXPORT_DEF int at_queue_insert(struct cpvt * cpvt, at_queue_cmd_t * cmds, unsigned cmdsno, int athead)
{
	at_queue_task_t * task;

	return at_queue_insert_task(cpvt, cmds, cmdsno, athead, &task);
}



#/* */
EXPORT_DEF void at_queue_handle_result (struct pvt* pvt, at_res_t res)
{
	/* move queue */
	at_queue_remove_cmd(pvt, res);
}

/*!
 * \brief Remove all itmes from the queue and free them
 * \param pvt -- pvt structure
 */

#/* */
EXPORT_DEF void at_queue_flush (struct pvt* pvt)
{
	struct at_queue_task* task;

	while ((task = AST_LIST_FIRST (&pvt->at_queue)))
	{
		at_queue_remove(pvt);
	}
}

/*!
 * \brief Get the first task on queue
 * \param pvt -- pvt structure
 * \return a pointer to the first command of the given queue
 */
#/* */
EXPORT_DEF const struct at_queue_task* at_queue_head_task (const struct pvt * pvt)
{
	return AST_LIST_FIRST (&pvt->at_queue);
}


/*!
 * \brief Get the first command of a queue
 * \param pvt -- pvt structure
 * \return a pointer to the first command of the given queue
 */
#/* */
EXPORT_DEF const at_queue_cmd_t * at_queue_head_cmd(const struct pvt * pvt)
{
	return at_queue_task_cmd(at_queue_head_task(pvt));
}

#/* */
EXPORT_DEF int at_queue_timeout(const struct pvt * pvt)
{
	int ms_timeout = -1;
	const at_queue_cmd_t * cmd = at_queue_head_cmd(pvt);

	if(cmd)
	{
		if(cmd->length == 0)
		{
			ms_timeout = ast_tvdiff_ms(cmd->timeout, ast_tvnow());
		}
	}

	return ms_timeout;
}
