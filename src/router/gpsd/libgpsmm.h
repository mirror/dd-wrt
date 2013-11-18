/* $Id: libgpsmm.h 3666 2006-10-26 23:11:51Z ckuethe $ */
/*
 * Copyright (C) 2005 Alfredo Pironti
 *
 * This software is distributed under a BSD-style license. See the
 * file "COPYING" for more information.
 *
 */
#ifndef _GPSMM_H_
#define _GPSMM_H_

#include <sys/types.h>
#include "gpsd_config.h"
#include "gps.h" //the C library we are going to wrap

class gpsmm {
	public:
		gpsmm() { };
		virtual ~gpsmm();
		struct gps_data_t* open(const char *host,const char *port); //opens the connection with gpsd, MUST call this before any other method
		struct gps_data_t* open(void); //open() with default values
		struct gps_data_t* query(const char *request); //put a command to gpsd and return the updated struct
		struct gps_data_t* poll(void); //block until gpsd returns new data, then return the updated struct
		int set_callback(void (*hook)(struct gps_data_t *sentence, char *buf, size_t len, int level)); //set a callback funcition, called each time new data arrives
		int del_callback(void); //delete the callback function
		void clear_fix(void);

	private:
		struct gps_data_t *gps_data;
		struct gps_data_t *to_user;	//we return the user a copy of the internal structure. This way she can modify it without
						//integrity loss for the entire class
		struct gps_data_t* backup(void) { *to_user=*gps_data; return to_user;}; //return the backup copy
		pthread_t *handler; //needed to handle the callback registration/deletion
};
#endif // _GPSMM_H_
