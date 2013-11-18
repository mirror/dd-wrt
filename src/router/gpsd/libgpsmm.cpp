/* $Id: libgpsmm.cpp 3491 2006-09-21 14:36:47Z ckuethe $ */
/*
 * Copyright (C) 2005 Alfredo Pironti
 *
 * This software is distributed under a BSD-style license. See the
 * file "COPYING" for more information.
 *
 */
#include "libgpsmm.h"

struct gps_data_t* gpsmm::open(void) {
	return open("localHost",DEFAULT_GPSD_PORT);
}

struct gps_data_t* gpsmm::open(const char *host, const char *port) {
	gps_data=gps_open(host,port);
	if (gps_data==NULL) { //connection not opened
		return NULL;
	}
	else { //connection succesfully opened
		to_user= new struct gps_data_t;
		return backup(); //we return the backup of our internal structure
	}
}

struct gps_data_t* gpsmm::query(const char *request) {
	if (gps_query(gps_data,request)==-1) {
		return NULL;
	}
	else {
		return backup();
	}
}

struct gps_data_t* gpsmm::poll(void) {
	if (gps_poll(gps_data)<0) {
		// we return null if there was a read() error or connection is cloed by gpsd
		return NULL;
	}
	else {
		return backup();
	}
}

int gpsmm::set_callback(void (*hook)(struct gps_data_t *sentence, char *buf, size_t len, int level)) {
	handler = new pthread_t;
	return gps_set_callback(gps_data,hook,handler);
}

int gpsmm::del_callback(void) {
	int res;
	res=gps_del_callback(gps_data,handler);
	delete handler;
	return res;
}

void gpsmm::clear_fix(void) {
	gps_clear_fix(&(gps_data->fix));
}

gpsmm::~gpsmm() {
	if (gps_data!=NULL) {
		gps_close(gps_data);
		delete to_user;
	}
}
