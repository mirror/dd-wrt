/*
  SisPM_ctl.c

  Controls the GEMBIRD Silver Shield PM USB outlet device

  (C) 2015-2018, Heinrich Schuchardt <xypron.glpk@gmx.de>
  (C) 2004, 2005, 2006 Mondrian Nuessle, Computer Architecture Group,
      University of Mannheim, Germany
  (C) 2005, Andreas Neuper, Germany
  (C) 2010, Olivier Matheret, France, for the scheduling part

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <wchar.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <usb.h>
#include <assert.h>
#include "sispm_ctl.h"

char serial_id[15];

int get_id(struct usb_device *dev)
{
	assert(dev != 0);
	return dev->descriptor.idProduct;
}

static int usb_control_msg_tries(usb_dev_handle *dev, int requesttype, int request, int value, int index, char *bytes, size_t size,
				 int timeout)
{
	int ret;
	char buf[64];

	if (size > sizeof(buf)) {
		return -1;
	}

	for (int i = 0; i < 5; ++i) {
		usleep(500 * i);
		memcpy(buf, bytes, size);
		ret = usb_control_msg(dev, requesttype, request, value, index, buf, size, timeout);
		if (ret == size) {
			break;
		}
	}

	memcpy(bytes, buf, size);

	return ret;
}

// for identification: reqtype=a1, request=01, b1=0x01, size=5
char *get_serial(usb_dev_handle *udev)
{
	int reqtype = 0xa1; //USB_DIR_OUT + USB_TYPE_CLASS + USB_RECIP_INTERFACE /* request type */,
	int req = 0x01;
	unsigned char buffer[6] = { 0, 0, 0, 0, 0, 0 };

	if (usb_control_msg_tries(udev, /* handle */
				  reqtype, req, (0x03 << 8) | 1, 0, /* index  */
				  (char *)buffer, /* bytes  */
				  5, /* size   */
				  5000) < 2) {
		fprintf(stderr,
			"Error performing requested action\n"
			"Libusb error string: %s\nTerminating\n",
			usb_strerror());
		usb_close(udev);
		exit(-5);
	}

	snprintf(serial_id, 15, "%02x:%02x:%02x:%02x:%02x", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);
	return serial_id;
}

int usb_command(usb_dev_handle *udev, int b1, int b2, int return_value_expected)
{
	int reqtype = 0x21; //USB_DIR_OUT + USB_TYPE_CLASS + USB_RECIP_INTERFACE /* request type */,
	int req = 0x09;
	char buffer[5];

	buffer[0] = b1;
	buffer[1] = b2;
	if (return_value_expected != 0) {
		reqtype |= USB_DIR_IN;
		req = 0x01;
	}
	if (usb_control_msg_tries(udev, /* handle */
				  reqtype, req, (0x03 << 8) | b1, 0, /* index  */
				  buffer, /* bytes  */
				  5, /* size   */
				  5000) < 2) {
		fprintf(stderr,
			"Error performing requested action\n"
			"Libusb error string: %s\nTerminating\n",
			usb_strerror());
		usb_close(udev);
		exit(-5);
	}

	return buffer[1]; //(buffer[1]!=0)?1:0;
}

usb_dev_handle *get_handle(struct usb_device *dev)
{
	usb_dev_handle *udev = NULL;
	if (!dev)
		return NULL;
	udev = usb_open(dev);

	/* prepare USB access */
	if (!udev) {
		fprintf(stderr, "Unable to open USB device %s\n", usb_strerror());
		return NULL;
	}
	if (usb_set_configuration(udev, 1)) {
		fprintf(stderr, "USB set configuration %s\n", usb_strerror());
		usb_close(udev);
		return NULL;
	}
	if (usb_claim_interface(udev, 0)) {
		fprintf(stderr, "USB claim interface %s\nMaybe device already in use?\n", usb_strerror());
		usb_close(udev);
		return NULL;
	}
	if (usb_set_altinterface(udev, 0)) {
		fprintf(stderr, "USB set alt interface %s\n", usb_strerror());
		usb_close(udev);
		return NULL;
		;
	}
	return udev;
}

int check_outlet_number(int id, int outlet)
{
	if (id == PRODUCT_ID_MSISPM_OLD) {
		if (outlet < 0 || outlet > 1)
			if (verbose == 1)
				fprintf(stderr, "mSIS-PM devices only feature one outlet. Number changed from %d to 0\n", outlet);
		outlet = 0;
	}
	if (id == PRODUCT_ID_MSISPM_FLASH) {
		if (outlet != 1)
			if (verbose == 1)
				fprintf(stderr, "mSIS-PM devices only feature one outlet. Number changed from %d to 1\n", outlet);
		outlet = 1;
	}
	if ((id == PRODUCT_ID_SISPM) || (id == PRODUCT_ID_SISPM_FLASH_NEW) || (id == PRODUCT_ID_SISPM_EG_PMS2)) {
		if (outlet < 1 || outlet > 4) {
			if (verbose == 1)
				fprintf(stderr, "SIS-PM devices only feature 4 outlets. Number changed from %d to 1\n", outlet);
			outlet = 1;
		}
	}
	return outlet;
}

int sispm_switch_on(usb_dev_handle *udev, int id, int outlet)
{
	outlet = check_outlet_number(id, outlet);
	return usb_command(udev, 3 * outlet, 0x03, 0);
}

int sispm_switch_off(usb_dev_handle *udev, int id, int outlet)
{
	outlet = check_outlet_number(id, outlet);
	return usb_command(udev, 3 * outlet, 0x00, 0);
}

int sispm_switch_toggle(usb_dev_handle *udev, int id, int outlet)
{
	if (!sispm_switch_getstatus(udev, id, outlet)) { //on
		sispm_switch_on(udev, id, outlet);
		return 1;
	} else {
		sispm_switch_off(udev, id, outlet);
		return 0;
	}

	return 0;
}

int sispm_switch_getstatus(usb_dev_handle *udev, int id, int outlet)
{
	int result;

	outlet = check_outlet_number(id, outlet);
	result = (usb_command(udev, 3 * outlet, 0x03, 1)); //take bit 1, which gives the relais status
	return result & 1;
}

int sispm_get_power_supply_status(usb_dev_handle *udev, int id, int outlet)
{
	int result;

	outlet = check_outlet_number(id, outlet);
	result = usb_command(udev, 3 * outlet, 0x03, 1); //take bit 0, which gives the power supply status
	return (result >> 1) & 1;
}

// displays a schedule structure in a human readable way
void plannif_display(const struct plannif *plan, int verbose, const char *progname)
{
	char datebuffer[80];
	struct tm *timeinfo;
	time_t date;
	int action;
	ulong loop = 0, lastActionTime = 0;
	char cmdline[1024] = "";

	printf("\nGet outlet %d status :\n", plan->socket);

	date = plan->timeStamp;
	timeinfo = localtime(&date);
	strftime(datebuffer, 80, "%e-%b-%Y %H:%M:%S", timeinfo);
	printf("  programmed on : %s\n", datebuffer);

	// action dates are on round minutes
	date = ((time_t)(date / 60)) * 60;

	// count loop time, as the sum of all but first events
	for (action = sizeof(plan->actions) / sizeof(struct plannifAction) - 1; action >= 0 && plan->actions[action].switchOn == -1;
	     --action)
		; // skip void entries
	if (action >= 1 && plan->actions[action].timeForNext > 0) { // we have a loop
		for (; action >= 1; --action)
			loop += plan->actions[action].timeForNext;
	}
	// compute last action time
	for (action = 0;
	     action + 1 < sizeof(plan->actions) / sizeof(struct plannifAction) && (plan->actions[action + 1].switchOn != -1);
	     ++action)
		lastActionTime += plan->actions[action].timeForNext;

	// if loop is enabled, do not display initial times, but next trigger times
	// so that at least last action is in the future
	if (loop > 0) {
		time_t now;
		ulong numOfLoops;
		time(&now);
		if (date + (lastActionTime * 60) <= now) {
			numOfLoops = 1 + (now - (date + (lastActionTime * 60))) / (loop * 60);
			date += numOfLoops * (loop * 60);
		}
	}
	// now read all filled rows, except the possibly last "stop" row
	for (action = 0; action < sizeof(plan->actions) / sizeof(struct plannifAction) && plan->actions[action].switchOn != -1;
	     ++action) {
		date += 60 * plan->actions[action].timeForNext;
		if ((action + 1 < sizeof(plan->actions) / sizeof(struct plannifAction)) &&
		    (plan->actions[action + 1].switchOn != -1)) {
			timeinfo = localtime(&date);
			strftime(datebuffer, 80, "%Y-%m-%d %H:%M", timeinfo);
			printf("  On %s ", datebuffer);
			printf("switch %s\n", (plan->actions[action + 1].switchOn ? "on" : "off"));
			if (verbose)
				sprintf(cmdline + (strlen(cmdline)), "--Aat \"%s\" --Ado %s ", datebuffer,
					(plan->actions[action + 1].switchOn ? "on" : "off"));
		} else {
			if (action > 0) {
				ulong loopdsp = loop;
				printf("  Loop every ");
				if (loopdsp >= 60 * 24 * 7) {
					printf("%li week(s) ", loopdsp / (60 * 24 * 7));
					loopdsp %= (60 * 24 * 7);
				}
				if (loopdsp >= 60 * 24) {
					printf("%li day(s) ", loopdsp / (60 * 24));
					loopdsp %= (60 * 24);
				}
				if (loopdsp >= 60) {
					printf("%lih ", loopdsp / 60);
					loopdsp %= 60;
				}
				if (loopdsp > 0)
					printf("%lumin", loopdsp);
				printf("\n");
				if (verbose)
					sprintf(cmdline + (strlen(cmdline)), "--Aloop %lu ", loop);
			} else
				printf("  No programmed event.\n");
		}
	}
	if (verbose) {
		printf("  equivalent command line : %s -A%i %s\n", progname, plan->socket, cmdline);
	}
}

// private : scans the buffer, and fills the schedule structure accordingly
void plannif_scanf(struct plannif *plan, const unsigned char *buffer)
{
	int bufindex = 0;
	ulong nextWord;
	int actionNo = 1;

	READNEXTBYTE;
	plan->socket = (nextWord - 1) / 3;
	READNEXTDOUBLEWORD;
	plan->timeStamp = nextWord;

	// first time to wait is at the end of the buffer, but may be extended to the plannif rows space
	READWORD(0x25);
	plan->actions[0].timeForNext = nextWord;
	if (plan->actions[0].timeForNext == 0xFD21) { // max value : means we may have extensions, which would have the flag 0x4000
		do {
			READNEXTWORD;
			if ((nextWord & 0x4000) == 0x4000) {
				plan->actions[0].timeForNext += nextWord & ~0x4000;
			} else {
				REVERTNEXTWORD;
			}
		} while (nextWord == 0x7FFF);
	}
	plan->actions[0].switchOn = 1; // whatever, it is useless for the initial waiting phase

	// now we can read all schedule rows
	while (bufindex < 0x25) {
		READNEXTWORD;
		if (nextWord != 0x3FFF) { // 3FFF means "empty"
			// on/off is the MSB of the 1st word, whether it is extended or not
			plan->actions[actionNo].switchOn = nextWord >> 15;
			plan->actions[actionNo].timeForNext = nextWord & 0x7FFF;
			if (plan->actions[actionNo].timeForNext ==
			    0x3FFE) { // again, 3FFE is the max value : it might be extended to next words, if they have the flag 0x4000
				do {
					READNEXTWORD;
					if ((nextWord & 0x4000) == 0x4000) {
						plan->actions[actionNo].timeForNext += nextWord & ~0x4000;
					} else {
						REVERTNEXTWORD;
					}
				} while (nextWord == 0x7FFF);
			}
		}
		actionNo++;
	}
}

// queries the device, and fills the schedule structure
void usb_command_getplannif(usb_dev_handle *udev, int socket, struct plannif *plan)
{
	int reqtype = 0x21 | USB_DIR_IN; /* request type */
	int req = 0x01;
	unsigned char buffer[0x28];
	unsigned int id;

	if (usb_control_msg_tries(udev, /* handle */
				  reqtype, req, ((0x03 << 8) | (3 * socket)) + 1, 0, /* index  */
				  (char *)buffer, /* bytes  */
				  0x28, /* size   */
				  5000) < 0x27) {
		fprintf(stderr,
			"Error performing requested action\n"
			"Libusb error string: %s\nTerminating\n",
			usb_strerror());
		usb_close(udev);
		exit(-5);
	}

	/* // debug
	   int n;
	   for(n = 0 ; n < 0x27 ; n++)
	   printf("%02x ", (unsigned char)buffer[n]);
	   printf("\n");
	   // */

	id = get_id(usb_device(udev));
	if (id == PRODUCT_ID_SISPM_EG_PMS2)
		pms2_buffer_to_schedule(buffer, plan);
	else
		plannif_scanf(plan, buffer);
}

// private : prints the buffer according to the schedule structure
void plannif_printf(const struct plannif *plan, unsigned char *buffer)
{
	int bufindex = 0;
	ulong nextWord, time4next;
	int actionNo;
	int i;

	nextWord = 3 * plan->socket + 1;
	WRITENEXTBYTE;
	nextWord = plan->timeStamp;
	WRITENEXTDOUBLEWORD;

	for (i = bufindex; i < 0x27; i += 2) {
		buffer[i] = 0xFF;
		buffer[i + 1] = 0x3F;
	}

	if (plan->actions[0].timeForNext == -1) {
		//delete all
		nextWord = 1;
		WRITEWORD(0x25);
	} else {
		time4next = plan->actions[0].timeForNext;
		// first time to wait is at the end of the buffer, but may be extended to the plannif rows space
		if (time4next > 0xFD21) {
			// max value is 0xFD21 : means we can set extensions, with the flag 0x4000
			time4next -= 0xFD21;
			while (time4next > 0x3FFF) { // each extension can handle 3FFF bytes
				nextWord = 0x3FFF | 0x4000;
				WRITENEXTWORD;
				time4next -= 0x3FFF;
			}
			nextWord = time4next | 0x4000;
			WRITENEXTWORD;
			nextWord = 0xFD21;
		} else
			nextWord = time4next;
		WRITEWORD(0x25);
	}

	// now we can write all schedule rows, if non empty
	for (actionNo = 1;
	     (actionNo < sizeof(plan->actions) / sizeof(struct plannifAction)) && (plan->actions[actionNo].switchOn != -1);
	     actionNo++) {
		time4next = plan->actions[actionNo].timeForNext;
		if (time4next > 0x3FFE) {
			// max value is 0x3FFE : means we can set extensions, with the flag 0x4000
			nextWord = 0x3FFE | (plan->actions[actionNo].switchOn << 15);
			WRITENEXTWORD;
			time4next -= 0x3FFE;
			while (time4next > 0x3FFF) { // each extension can handle 3FFF bytes
				nextWord = 0x3FFF | 0x4000;
				WRITENEXTWORD;
				time4next -= 0x3FFF;
			}
			nextWord = time4next | 0x4000;
		} else
			nextWord = time4next | (plan->actions[actionNo].switchOn << 15);
		WRITENEXTWORD;
	}
}

// prepares the buffer according to plannif and sends it to the device
void usb_command_setplannif(usb_dev_handle *udev, struct plannif *plan)
{
	int reqtype = 0x21; //USB_DIR_OUT + USB_TYPE_CLASS + USB_RECIP_INTERFACE /*request type*/,
	int req = 0x09;
	unsigned char buffer_size = 0x27;
	unsigned char buffer[0x28];
	unsigned int id;

	id = get_id(usb_device(udev));
	if (id == PRODUCT_ID_SISPM_EG_PMS2) {
		if (pms2_schedule_to_buffer(plan, buffer))
			exit(-2);
	} else {
		buffer_size = 0x27;
		plannif_printf(plan, buffer);
	}

	/*// debug
	   int n;
	   for(n = 0 ; n < 0x27 ; n++)
	   printf("%02x ", (unsigned char)buffer[n]);
	   printf("\n");
	   plannif_reset(plan);
	   plannif_scanf(plan, &buffer[0]);
	   plannif_display(plan, 0, NULL);
	   exit(0);
	   // */
	if (usb_control_msg_tries(udev, /* handle */
				  reqtype, req, ((0x03 << 8) | (3 * plan->socket)) + 1, 0, /* index */
				  (char *)buffer, /* bytes */
				  buffer_size, /* size  */
				  5000) < buffer_size) {
		fprintf(stderr,
			"Error performing requested action\n"
			"Libusb error string: %s\nTerminating\n",
			usb_strerror());
		usb_close(udev);
		exit(-5);
	}
}

// prepares the plannif structure with initial values : compulsory before structure use !
void plannif_reset(struct plannif *plan)
{
	int i;

	plan->socket = 0;
	plan->timeStamp = 0;
	for (i = 0; i < sizeof(plan->actions) / sizeof(struct plannifAction); ++i) {
		plan->actions[i].switchOn = -1;
		plan->actions[i].timeForNext = -1;
	}
}
