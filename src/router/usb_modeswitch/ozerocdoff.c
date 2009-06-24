/*
  Option Zero-CD Disabler or simple turn off hack

  Copyright (C) 2008  Peter Henn (support@option.com)

  Note, most of the part coming from  Josua Dietze usb_modeswitch

  We use this way to disable the ZeroCD device, because we have seen some
  problems during the USB-SCSI device "unplug" and "SCSI disconnect", which
  may result into a complete aystem freezes. Although this disabling of the
  Zero-CD device is a simple SCSI rezero command, the WWAN-modem firmware
  does sometimes not correctly do a "SCSI discnnection" to the USB SCSI
  driver, before it just plug off from the SCSI bus and does require a
  USB reenumeration with the WWAN-modem USB interfaces instead of the USB
  SCSI CD-ROM device.
  Unbinding the USB SCSI driver and sending the USB rezero command simple by
  the usage of the libusb directly from user space solve this time critical
  handling between the SCSI driver and the firmware.

  History:
  0.1, 2008/04/10 P.Henn
       - Initial version with usage of code example from usb_modeswitch
  0.2, 2008/04/14 P.Henn
       - several bugfixes
       - autosetup endpoint address
       - ensure support for three firmware classes
  0.3, 2008/04/15 P.Henn
       - first optional filename compare support
  0.4, 2008/06/09 P-Henn
       - add log file functionality
       - add harder Zero-CD device search phase, if /sys/class/usb_device
         is not supported
       - add search try option

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details:

  http://www.gnu.org/licenses/gpl.txt

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <ctype.h>

#include <stdarg.h>
#include <getopt.h>
#include <usb.h>
#include <dirent.h>

#define VERSION "0.4"
#define TARGET_VENDOR 0x0af0
#define BUFFER_SIZE 65535


// some defines lend from the /usr/include/linux/usb/ch9.h
#define USB_ENDPOINT_XFERTYPE_MASK      0x03    /* in bmAttributes */
#define USB_ENDPOINT_XFER_BULK          2
#define USB_ENDPOINT_DIR_MASK           0x80
#define USB_DIR_OUT                     0       /* to device */
#define USB_DIR_IN                      0x80    /* to host */

#define UDEV_POLL_PERIOD                100     /* in ms, must be > UDEV_POLL_PERIOD_MIN, should be > 55ms and <= 100ms */
#define UDEV_POLL_PERIOD_MIN            10      /* in ms, should be > 5ms and <= 10ms */



static struct option long_options[] =
{
    {"help",	       	no_argument,       NULL, 'h'},
    {"device_id",	required_argument, NULL, 'i'},
    {"usb_filename",	optional_argument, NULL, 'f'},
    {"namelen",         optional_argument, NULL, 'n'},
    {"quiet",		no_argument,       NULL, 'q'},
    {"debug",           no_argument,       NULL, 'd'},
    {"warn_only",       no_argument,       NULL, 'w'},
    {"version",		no_argument,       NULL, 'v'},
    {"test",            no_argument,       NULL, 't'},
    {"log",             required_argument, NULL, 'l'},
    {"searchtime",      required_argument, NULL, 's'},
    {NULL, 0, NULL, 0}
};



int debug = 0;                   /* commandline switch print debug output */
int namelength = 1;              /* commandline switch of prefered name length */
struct usb_dev_handle *usb_hd;   /* global for usage in signal handler */


/* Function Protortype */
void release_usb_device(int);
struct usb_device* search_devices(int, int, const char*);
int search_message_endp(struct usb_device *);
int search_response_endp(struct usb_device *);
void print_usage(void);
const char *strrcut(const char *, int);
int strrcmp(const char *, const char *);
FILE *errlog;
FILE *outlog;


/**
 * print usage
 * prints just the help text to sreen, for all the nice options, which may be used
 */
void print_usage(void) {
        printf ("Usage: ozerocdoff [-hqwqv] -i <DeviceID> [-f <name>] [-n <len>]\n\n");
	printf (" -h, --help               this help\n");
	printf (" -i, --device_id <n>      target USB device id, required option\n");
	printf (" -f, --usb_filename <str> target USB file name\n");
	printf (" -n, --namelen <n>        target USB file name len (default %d)\n", namelength);
	printf (" -w, --warn_only          print warnings instead of errors without program abort\n");
	printf (" -q, --quiet              don't show messages\n");
	printf (" -t, --test               test only, nothing send\n");
	printf (" -l, --log                write message into log file instead of stderr\n");
	printf (" -s, --searchtime <n> ms  tries to search at least <n> ms for the Zero-CD device\n");
	printf (" -d, --debug              output some debug messages\n");
	printf (" -v, --version            show version string\n\n");
	printf ("Examples:\n");
	printf ("   ozerocdoff -i 0xc031\n");
	printf ("   ozerocdoff -i 0xc031 -f usbdev5.28 -n 2\n");
}



int main(int argc, char **argv) {
	struct usb_device *usb_dev;
	int quiet = 0;            /* commandline switch be quiet, no output */
	int warno = 0;            /* commandline switch do only warn and do no program abort */
	int test = 0;             /* commandline switch for testing, no rezero will be send */
	int count;                /* retry counter */
	int ret;                  /* general return value checker */
	int searchtime;           /* search time in ms to the Zero-CD device */
	DIR *usb_dev_dir;         /* just for testing, if directory exists */
	const char *filename="";  /* filename of this USB device */
        const char *logname=(char *)NULL;/* filename of log file */
	char buffer[BUFFER_SIZE];

	int TargetProduct=0;      /* has not to be zero */

	const char const MessageContent[]={
	  0x55, 0x53, 0x42, 0x43, 0x78, 0x56, 0x34, 0x12,
	  0x01, 0x00, 0x00, 0x00, 0x80, 0x00, 0x06, 0x01,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	
	int c, option_index = 0;
	usb_dev_dir = opendir ("/sys/class/usb_device/");
	if (usb_dev_dir != NULL) {
	      closedir (usb_dev_dir);
	      searchtime = 0;    /* no search time needed */
	} else {
	      searchtime = 800;  /* set search time to 800 ms */
	}

	while (1){
                c = getopt_long (argc, argv, "hvwdtqi:n:f:l:s:",
                        long_options, &option_index);
		if (c == -1) {
		        break;
		}
		switch (c) {
		case 'i': TargetProduct = strtol(optarg, NULL, 0); break;
		case 'n': namelength = strtol(optarg, NULL, 0); break;
		case 'f': filename = optarg; break;
		case 'l': logname = optarg; break;
		case 's': searchtime = strtol(optarg, NULL, 0); break;
	        case 'q': quiet=1; break;
		case 'd': debug=1; break;
	        case 'w': warno=1; break;
	        case 't': test=1; break;
		case 'v': printf("ozerocdoff version: %s\n", VERSION); exit(0);
		case 'h': print_usage(); exit(0);
		default:  print_usage(); exit(1);
		}
	}

	if (  (logname == (char *)NULL)
	    ||((outlog = errlog = fopen(logname, "a")) == NULL)) {
	        outlog = stdout;
	        errlog = stderr;
	} else {
	      fprintf(outlog, "\n");
	      for(count=0; count < argc; count++) {
	              fprintf(outlog, "%s ", argv[count]);
	      }
	      fprintf(outlog, "\n");
	}

	/* check argument */
	if (TargetProduct==0) {
	        fprintf(errlog, "ERROR: Device ID missing\n");
		exit(1);
	}
	if (debug) quiet=0;
	if (searchtime < 0) {
	        searchtime = 0; /* search at least one time */
	}

       	/* general USB-Lib init stuff */
	usb_init();
        /* if the searchtime is < UDEV_POLL_PERIOD_MIN, we will search only once for the device node, but
           we will wait that searchtime just before we do the first test!
           if the searchtime is > UDEV_POLL_PERIOD, we will repeat the test and do a retest after this time
           period. If the rest of the time is < UDEV_POLL_PERIOD_MIN, we will simple skip the last test,
           otherwise we do a last test after the rest wait time */
	if ((searchtime < UDEV_POLL_PERIOD_MIN) && (searchtime > 0)) {
	        usleep(searchtime*1000);
	}
	for (count=searchtime, ret=1; count>=(UDEV_POLL_PERIOD_MIN - UDEV_POLL_PERIOD); count-=UDEV_POLL_PERIOD, ret++) {
	        (void)usb_find_busses();
		(void)usb_find_devices();

		if (debug) {
		        fprintf(outlog, "%d. Search Zero-CD device\n", ret);
		}
		/* we use currently only the given device ID from the command line to detect the device */
		/* that will be bad, if we want to distingush between multiple WWAN-modems, to take care! */
		usb_dev = search_devices(TARGET_VENDOR, TargetProduct, strrcut(filename, namelength));
		if (usb_dev != NULL) {
		        break;
		} else {
		        if ((count >= UDEV_POLL_PERIOD_MIN) && (count < UDEV_POLL_PERIOD)) {
			      usleep(count*1000);
			} else if (count > UDEV_POLL_PERIOD) {
			      usleep(UDEV_POLL_PERIOD*1000);
			}
		}
	}
	if (usb_dev == NULL) {
	        if(!quiet) fprintf(errlog, "ERROR: No Zero-CD device found\n");
		      exit(2);
		}
	usb_hd = usb_open(usb_dev);
	if (usb_hd == NULL) {
		if(!quiet) fprintf(errlog, "ERROR: device access not possible\n");
		exit(4);
	}
    
	/* detach running default driver */
	signal(SIGTERM, release_usb_device);
	ret = usb_get_driver_np(usb_hd, 0, buffer, sizeof(buffer));
	if (ret == 0) {
	        if (debug) {
		        fprintf(outlog, "Found attached driver: %s\n", buffer);
		}
	        ret = usb_detach_kernel_driver_np(usb_hd, 0);
		if (ret != 0) {
		        if(!quiet) fprintf(errlog, "ERROR: unsuccessful kernel driver detaching\n");
			(void)usb_close(usb_hd);
			exit(5);
		}
	} else {
	        /* hopefully no default kernel driver yet running, send only a warning */
	        if(!quiet) fprintf(errlog, "WARNING: No kernel driver attached found\n");
	}
	ret = usb_claim_interface(usb_hd, 0);
	if (ret != 0) {
		if(!quiet) fprintf(errlog, "ERROR: could not claim interface\n");
		(void)usb_close(usb_hd);
		exit(6);
	}
	  

	/* findout the Message and Response Endpoint */	
        int MessageEndpoint=search_message_endp(usb_dev);
	int ResponseEndpoint=search_response_endp(usb_dev);
	if (MessageEndpoint == 0) {
		if(!quiet) fprintf(errlog, "ERROR: could not detremine Endpoint addresses\n");
		(void)usb_close(usb_hd);
		exit(6);
	}
	if (debug) {
	        fprintf(outlog,"MessageEndpoint:  0x%x\n", MessageEndpoint);
		fprintf(outlog,"ResponseEndpoint:  0x%x \n", ResponseEndpoint);
	}


	/* preparation to send rezero string */
       	(void)usb_clear_halt(usb_hd, MessageEndpoint); //ResponseEndpoint
        ret = usb_set_altinterface(usb_hd, 0);
	if (ret != 0) {
	        if (warno) {
		        if(!quiet) fprintf(errlog, "WARNING: unsuccessful set alternative interface\n");
		} else {
		        if(!quiet) fprintf(errlog, "ERROR: unsuccessful set alternative interface\n");
			(void)usb_release_interface(usb_hd, 0);
			(void)usb_close(usb_hd);
		        exit(7);
		}
	}


	/* usb mass storage setting time (needed by some WWAN modems) */
	sleep(1);

	if (!test) {
	        /* send rezero string */
	        ret = usb_bulk_write(usb_hd, MessageEndpoint, (char *)MessageContent, sizeof(MessageContent), 1000);
	        if (ret < 0) {
	                if (warno) {
			        if(!quiet) fprintf(errlog, "WARNING: unsuccessful write rezero string\n");
			} else {
			        if(!quiet) fprintf(errlog, "ERROR: unsuccessful write rezero string\n");
				(void)usb_release_interface(usb_hd, 0);
				(void)usb_close(usb_hd);
				exit(8);
			}
		} else {
		        if (debug) fprintf(outlog, "Have successfully send ZERO-CD disabling command\n");
		}
		/* needed by some WWAN modems) */
		if (ResponseEndpoint != 0) {
		        ret = usb_bulk_read(usb_hd, ResponseEndpoint, buffer, BUFFER_SIZE, 1000);
			if (ret < 0 ) {
			        if(!quiet) fprintf(errlog, "WARNING: unsuccessful read response message\n");
			}
		}
	}


	/* close device */
	ret = usb_close(usb_hd);
	if (ret < 0) {
	        if (warno) {
		        if(!quiet) fprintf(errlog, "WARNING: closeing failed\n");
		} else {
		        if(!quiet) fprintf(errlog, "ERROR: closeing failed\n");
			(void)usb_release_interface(usb_hd, 0);
			exit(9);
		}
	}	


	/* search again to check result. Note that still the search is needed by some WWAN modems! */
	for (count = 5; (count>0) && (usb_dev != NULL); count--) {
       	        sleep(1);
		(void)usb_find_devices();
		usb_dev = search_devices(TARGET_VENDOR, TargetProduct, "");
	}
	if (usb_dev != NULL) {
	        if (warno) {
		        if(!quiet) fprintf(errlog, "WARNING: Zero-CD device still found\n");
		} else {
		        if(!quiet) fprintf(errlog, "ERROR: Zero-CD device still found\n");
			(void)usb_release_interface(usb_hd, 0);
			exit(10);
		}
	} else {
	        if (debug) fprintf(outlog, "Checked successfully ZERO-CD disabled\n");
		else if(!quiet) fprintf(outlog, "Successfully ZERO-CD disabled\n");
	}



	/* relese device */
	(void)usb_release_interface(usb_hd, 0);
	return 0;
}


/**
 * release_usb_device
 * need for the signal handler to ensure proper usb port close and release
 *
 * input param : just a dumy paramter
 * return      : nothting
 */
void release_usb_device(int param) {
	(void)usb_release_interface(usb_hd, 0);
	(void)usb_close(usb_hd);
}


/**
 * strrcut
 * string help function, which cuts the string and returns the 
 * right boundary last 'n' characters from the given string 's' or
 * an empty string, if the given string length is shorter than 'n'
 *
 * input s  : const string
 * input n  : number of charaacters, which should be left on the 
 *            right side of the given string
 * return   : the result (shorten) strinf or empty string
 */
const char *strrcut(const char *s, int n) {
        if (strlen(s)>=n) {
	        return &(s[strlen(s)-n]);
	} else {
	        return "";
	}
}

/**
 * strrcmp
 * string help function, which is used to compare just the right
 * part of the given two strings. All other part is just cut away or
 * not be used for the comparision
 *
 * input s1 : const 1st string
 * input s2 : const 2nd string
 * return   : the strcmp return value of the shorted given strings
 */
int strrcmp(const char *s1, const char *s2) {
      if (strlen(s1)<=strlen(s2)) {
	      return strcmp(s1,strrcut(s2,strlen(s1)));
      } else {
	      return strcmp(strrcut(s1,strlen(s2)),s2);
      }
}


/** search_devices
 * search function of the vendor and device id. Optional the name can 
 * be used to compare the right part of found string of the usblib.
 *
 * input vendor  : USB vendor id
 * input product : USB product id
 * input name    : optional right part of the searched filename or just an empty string
 * return        : the found usb device structure, which matches the search
 */
struct usb_device* search_devices(int vendor, int product, const char * name) {
        struct usb_bus *bus;
    
	for (bus = usb_get_busses(); bus; bus = bus->next) {
		struct usb_device *dev;
		for (dev = bus->devices; dev; dev = dev->next) {
		        if (dev->descriptor.idVendor == vendor && dev->descriptor.idProduct == product) {
			        /* check number of Endpoints, class, subclass and protocol */
			        if (debug) {
				        fprintf(outlog, "Endpoints: %d  (2)\n", dev->config[0].interface[0].altsetting[0].bNumEndpoints);
					fprintf(outlog, "Class:     0x%x  (0x08)\n", dev->config[0].interface[0].altsetting[0].bInterfaceClass);
					fprintf(outlog, "SubClass:  0x%x  (0x06)\n", dev->config[0].interface[0].altsetting[0].bInterfaceSubClass);
					fprintf(outlog, "Protocol:  0x%x  (0x50)\n", dev->config[0].interface[0].altsetting[0].bInterfaceProtocol);
				}
				if (   (dev->config[0].interface[0].altsetting[0].bNumEndpoints == 2)
				    && (dev->config[0].interface[0].altsetting[0].bInterfaceClass == 0x08)
				    && (dev->config[0].interface[0].altsetting[0].bInterfaceSubClass == 0x06)
				    && (dev->config[0].interface[0].altsetting[0].bInterfaceProtocol == 0x50) ) {
				        if (debug) {
					        fprintf(outlog, "Found file name: '%s'\n", dev->filename);
						fprintf(outlog, "Required last chars of file name: '%s'\n", name);
					}
					if (strrcmp(name, dev->filename) == 0) {
					        if (debug) fprintf(outlog, "UMS interface found\n");
						return dev;
					} else {
					        if (debug) fprintf(outlog, "UMS interface found, but its wrong\n");
					}
				}
			}
		}
	}
	return NULL;
}

/**
 * search_message_endp
 * looks inside the USB descriptor stuff for the needed endpoint address. 
 * It searches explicit for an bulk output interface and returns the address
 * or even zero
 *
 * input dev : usb device structure
 * return    : endpoint address of 0, if it is no bulk output endpoint
 */
int search_message_endp(struct usb_device *dev) {
        if (  ((dev->config[0].interface[0].altsetting[0].endpoint[0].bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK)
	    &&((dev->config[0].interface[0].altsetting[0].endpoint[0].bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT)) {
	        return dev->config[0].interface[0].altsetting[0].endpoint[0].bEndpointAddress; 
	} else 
	        if (  ((dev->config[0].interface[0].altsetting[0].endpoint[1].bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK)
	            &&((dev->config[0].interface[0].altsetting[0].endpoint[1].bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT)) {
	                return dev->config[0].interface[0].altsetting[0].endpoint[1].bEndpointAddress;
	}
	return 0;
}

/**
 * search_response_endp
 * looks inside the USB descriptor stuff for the needed endpoint address. 
 * It searches explicit for an bulk input interface and returns the address
 * or even zero
 *
 * input dev : usb device structure
 * return    : endpoint address of 0, if it is no bulk input endpoint
 */
int search_response_endp(struct usb_device *dev) {
        if (  ((dev->config[0].interface[0].altsetting[0].endpoint[0].bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK)
	    &&((dev->config[0].interface[0].altsetting[0].endpoint[0].bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN)) {
	        return dev->config[0].interface[0].altsetting[0].endpoint[0].bEndpointAddress; 
	} else 
	        if (  ((dev->config[0].interface[0].altsetting[0].endpoint[1].bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK)
	            &&((dev->config[0].interface[0].altsetting[0].endpoint[1].bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN)) {
	                return dev->config[0].interface[0].altsetting[0].endpoint[1].bEndpointAddress;
	}
	return 0;
}


