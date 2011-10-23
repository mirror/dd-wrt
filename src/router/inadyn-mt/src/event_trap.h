/*
Copyright (C) 2007 Bryan Hoover (bhoover@wecs.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#define RAS_CONNECT     0
#define RAS_DISCONNECT  1
#define THREAD_RETURN   2

#define TOTAL_EVENTS    3

/*

*********client program events callback passed to constructor*********

void ras_events_handler(RAS_THREAD_DATA *p_ras_thread_data)
{

}

*/

typedef void (*EVENT_TRAP_HANDLER)(struct RAS_THREAD_DATA *p_ras_thread_data);

typedef struct RAS_THREAD_DATA {

	void               *p_context;

	HANDLE             hEvent[TOTAL_EVENTS];
	DWORD              ras_notifications_thread;

	/*
	SYNCHRONIZATION 
	*/

	HANDLE             hClientInitSignal;

	/*
	can wait for thread init complete
	*/

	HANDLE             hRASInitSignal;

	/*
	can be used for manual THREAD_RETURN trigger
	*/
	HANDLE             hRASManSignal;


	EVENT_TRAP_HANDLER p_func;

	DWORD              dwEvent;

	DWORD              dw_debug_level;
	char               szTestURL[256];

} RAS_THREAD_DATA;

/*construct, and launch/start ras events thread*/
RAS_THREAD_DATA* construct_and_launch_trap_ras_events(EVENT_TRAP_HANDLER p_func,void *p_context,const char *test_url,DWORD debug_level);

/*construct only*/
RAS_THREAD_DATA* construct_trap_ras_events(EVENT_TRAP_HANDLER p_func,void *p_context,const char *test_url,DWORD debug_level);
/*after construct only*/
int trap_ras_events(RAS_THREAD_DATA **p_ras_thread_data,BOOL init_wait);

/*"do it yourself"*/
BOOL init_thread_data(RAS_THREAD_DATA *p_ras_thread_data);
void launch_ras_events_trap(void **p_data);

void set_debug_level(RAS_THREAD_DATA *p_ras_thread_data,DWORD debug_level);
void set_online_test(RAS_THREAD_DATA *p_ras_thread_data,const char *szURL);

void destroy_trap_ras_events(RAS_THREAD_DATA **p_p_ras_thread_data);

is_online(const char *test_url,int debug_level);
