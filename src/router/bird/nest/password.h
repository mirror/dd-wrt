/*
 *	BIRD -- Password handling
 *
 *	(c) 1999 Pavel Machek <pavel@ucw.cz>
 *	(c) 2004 Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef PASSWORD_H
#define PASSWORD_H
#include "lib/timer.h"

#define MD5_AUTH_SIZE 16

struct password_item {
  node n;
  char *password;
  int id;
  bird_clock_t accfrom, accto, genfrom, gento;
};

extern struct password_item *last_password_item;

struct password_item *password_find(list *l, int first_fit);
void password_cpy(char *dst, char *src, int size);

#endif
