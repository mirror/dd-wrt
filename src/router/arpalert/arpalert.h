/*
 * Copyright (c) 2005-2010 Thierry FOURNIER
 * $Id: arpalert.h 690 2008-03-31 18:36:43Z  $
 *
 */

#include <time.h>
#include <sys/time.h>

// time_t current_time;
struct timeval current_t;

// is forked
int is_forked;

