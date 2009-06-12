#ifndef _HYPERSTONE_CURRENT_H
#define _HYPERSTONE_CURRENT_H

extern struct task_struct *_current_task;
#define get_current()   _current_task
#define current 	_current_task


#endif /* !(_HYPERSTONE_CURRENT_H) */
