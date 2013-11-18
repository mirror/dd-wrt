/* $Id: gpsmm_test.cpp 3543 2006-09-28 06:00:14Z ckuethe $ */
/*
 * Copyright (C) 2005 Alfredo Pironti
 *
 * This software is distributed under a BSD-style license. See the
 * file "COPYING" for more information.
 *
 */

/* This simple program shows the basic functionality of the C++ wrapper class */
#include <iostream>
#include <string>
#include <unistd.h>

#include "libgpsmm.h"

using namespace std;

static void callback(struct gps_data_t* p, char* buf, size_t len, int level);

int main(void) {
	gpsmm gps_rec;
	struct gps_data_t *resp;

	resp=gps_rec.open();
	if (resp==NULL) {
		cout << "Error opening gpsd\n";
		return (1);
	}

	cout << "Going to set the callback...\n";
	if (gps_rec.set_callback(callback)!=0 ) {
		cout << "Error setting callback.\n";
		return (1);
	}

	cout << "Callback setted, sleeping...\n";
	sleep(10);
	cout << "Exited from sleep...\n";
	
	if (gps_rec.del_callback()!=0) {
		cout << "Error deleting callback\n";
		return (1);
	}
	cout << "Sleeping again, to make sure the callback is disabled\n";
	sleep(4);

	cout << "Exiting\n";
	return 0;
}

static void callback(struct gps_data_t* p, char* buf, size_t len, int level) {
	
	if (p==NULL) {
		cout << "Error polling gpsd\n";
		return;
	}
	cout << "Online:\t" << p->online << "\n";
	cout << "Status:\t" << p->status << "\n";
	cout << "Mode:\t" << p->fix.mode << "\n";
	if (p->fix.mode>=MODE_2D) {
		if (p->set & LATLON_SET) {
			cout << "LatLon changed\n";
		}
		else {
			cout << "LatLon unchanged\n";
		}
		cout << "Longitude:\t" << p->fix.longitude <<"\n";
		cout << "Latitude:\t" << p->fix.latitude <<"\n";
	}
}
