/* timeout.h - Processing of signaling timeout events */
 
/* Written 1995-1997 by Werner Almesberger, EPFL-LRC */
 

#ifndef TIMEOUT_H
#define TIMEOUT_H

#define T303_TIME	 4000000 /*  4 sec */
#define T308_1_TIME	30000000 /* 30 sec */
#define T308_2_TIME	30000000 /* 30 sec */
#define T309_TIME	10000000 /* 10 sec */
#define T310_TIME	10000000 /* 10 sec */
#define T313_TIME	 4000000 /*  4 sec */

#define T360_TIME	30000000 /* 30 sec */
#define T361_TIME	20000000 /* 20 sec */
#define T399_TIME	14000000 /* 14 sec */
#define T398_TIME	 4000000 /*  4 sec */

#define START_TIMER(u,t) { assert(!u->conn_timer); \
  u->conn_timer = start_timer(t##_TIME,on_##t,u); }
#define STOP_TIMER(u) { stop_timer(u->conn_timer); u->conn_timer = NULL; }


void on_T398(void *user);
void on_T399(void *user);
void on_T303(void *user);
void on_T308_1(void *user);
void on_T308_2(void *user);
void on_T309(void *user);
void on_T310(void *user);
void on_T313(void *user);
void on_T360(void *user);
void on_T361(void *user);

#endif
