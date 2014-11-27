/*
 * Copyright (c) 2005-2010 Thierry FOURNIER
 * $Id: func_time.c 690 2008-03-31 18:36:43Z  $
 *
 */

#include <sys/time.h>

/* compare t1 to t2
 * si t1 > t2 => 1
 * si t1 = t2 => 0
 * si t1 < t2 =>-1
 */
int time_comp(struct timeval *t1, struct timeval *t2){
	if(t1->tv_sec == t2->tv_sec && t1->tv_usec == t2->tv_usec)
		return 0;
	else if((t1->tv_sec == t2->tv_sec && t1->tv_usec > t2->tv_usec) ||
	        (t1->tv_sec > t2->tv_sec))
		return 1;
	else
		return -1;
}

void time_sous(struct timeval *t1, struct timeval *t2, struct timeval *res){
	res->tv_sec = t1->tv_sec - t2->tv_sec;
	res->tv_usec = t1->tv_usec - t2->tv_usec;
	if(res->tv_usec < 0){
		res->tv_sec -= 1;
		res->tv_usec += 1000000;
	}
}
