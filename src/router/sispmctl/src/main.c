/*
  main.c

  Controls the GEMBIRD Silver Shield PM USB outlet device

  (C) 2015-2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
  (C) 2011-2016, Pete Hildebrandt <send2ph@gmail.com>
  (C) 2010, Olivier Matheret, France, for the scheduling part
  (C) 2004-2011, Mondrian Nuessle, Computer Architecture Group,
      University of Mannheim, Germany
  (C) 2005, Andreas Neuper, Germany

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

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define __USE_XOPEN
#include <signal.h>
#include <syslog.h>
#include <time.h>
#include <usb.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "sispm_ctl.h"
#include "socket.h"
#include "config.h"

#ifndef MSG_NOSIGNAL
#include <signal.h>
#endif

#ifndef WEBLESS

#define BUFSIZE 256

static void read_password(void)
{
	FILE *file;
	const char filename[] = "/etc/sispmctl/password";
	char buf[BUFSIZE];
	char *pos;
	size_t len;

	file = fopen(filename, "r");
	if (!file) {
		if (errno != ENOENT) {
			perror(filename);
			exit(EXIT_FAILURE);
		}
		/* It is ok if there is no password file */
		return;
	}
	memset(buf, 0, BUFSIZE);
	len = fread(buf, 1, BUFSIZE - 1, file);
	if (!len) {
		fprintf(stderr, "Failed to read password\n");
		exit(EXIT_FAILURE);
	}
	pos = strchr(buf, '\n');
	if (pos) {
		*pos = '\0';
	}
	secret = strdup(buf);
	if (secret) {
		memset(buf, 0, 256);
	} else {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}
	fclose(file);
}

static void daemonize()
{
	/* Our process ID and Session ID */
	pid_t pid;

	/* Fork off the parent process */
	pid = fork();
	if (pid < 0)
		exit(EXIT_FAILURE);

	/* If we got a good PID, then
	   we can exit the parent process. */
	if (pid > 0)
		exit(EXIT_SUCCESS);

	/* Change the file mode mask */
	umask(0);

	/* Change the current working directory */
	if ((chdir("/var/tmp")) < 0)
		/* Log the failure */
		exit(EXIT_FAILURE);

	/*
	 * We do not expect any keyboard input anymore.
	 * STDERR is still needed for logging.
	 */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
}
#endif

static void print_disclaimer(void)
{
	fprintf(stderr, "\nSiS PM Control for Linux " PACKAGE_VERSION "\n\n"
			"(C) 2015-2020, Heinrich Schuchardt <xypron.glpk@gmx.de>\n"
			"(C) 2011-2016, Pete Hildebrandt <send2ph@gmail.com>\n"
			"(C) 2004-2011, Mondrian Nuessle\n"
			"(C) 2005-2006, Andreas Neuper\n"
			"(C) 2010, Olivier Matheret for the scheduling part\n\n"
			"This program comes with ABSOLUTELY NO WARRANTY.\n\n"
			"You may re-distribute it under the terms of the\n"
			"GNU General Public License version 2 or later.\n");
	return;
}

static void print_usage(char *name)
{
	print_disclaimer();
	fprintf(stderr,
		"\n"
		"sispmctl -s\n"
		"sispmctl [-q] [-n] [-d 0...] [-D ...] -b <on|off>\n"
		"sispmctl [-q] [-n] [-d 0...] [-D ...] -[o|f|t|g|m] 1..4|all\n"
		"sispmctl [-q] [-n] [-d 0...] [-D ...] -[a|A] 1..4|all [--Aat '...'] "
		"[--Aafter ...] [--Ado <on|off>] ... [--Aloop ...]\n"
		"   'v'   - print version & copyright\n"
		"   'h'   - print this usage information\n"
		"   's'   - scan for supported GEMBIRD devices\n"
		"   'b'   - switch buzzer on or off\n"
		"   'o'   - switch outlet(s) on\n"
		"   'f'   - switch outlet(s) off\n"
		"   't'   - toggle outlet(s) on/off\n"
		"   'g'   - get status of outlet(s)\n"
		"   'm'   - get power supply status outlet(s) on/off\n"
		"   'd'   - apply to device 'n'\n"
		"   'D'   - apply to device with given serial number\n"
		"   'U'   - apply to device connected to USB Bus:Device\n"
		"   'n'   - show result numerically\n"
		"   'q'   - quiet mode, no explanations - but errors\n"
		"   'a'   - get schedule for outlet\n"
		"   'A'   - set schedule for outlet\n"
		"           '-A<num>'        - select outlet\n"
		"           '--Aat \"date\"'   - sets an event time as a date "
		"'%%Y-%%m-%%d %%H:%%M' in the current time zone\n"
		"           '--Aafter N'     - sets an event time as N minutes "
		"after the previous one\n"
		"           '--Ado <on|off>' - sets the current event's action\n"
		"           '--Aloop N'      - loops to 1st event's action after "
		"N minutes\n\n"
#ifndef WEBLESS
		"Web interface features:\n"
		"sispmctl [-q] [-i <ip>] [-p <#port>] [-u <path>] -l|L\n"
		"   'l'   - start port listener\n"
		"   'L'   - same as 'l', but stay in foreground\n"
		"   'i'   - bind socket on interface with given IP (dotted decimal, "
		"e.g. 192.168.1.1)\n"
		"   'p'   - port number for listener (%d)\n"
		"   'u'   - repository for web pages (default=%s)\n\n",
		listenport, homedir
#endif
	);

#ifdef WEBLESS
	fprintf(stderr, "Note: This build was compiled without "
			"web-interface features.\n\n");
#endif
}

static void parse_command_line(int argc, char *argv[], int count, struct usb_device *dev[], char *usbdevsn[])
{
	int numeric = 0;
	int c;
	int i, j;
	int result;
	int from = 1, upto = 4;
	int status;
	int devnum = 0;
	usb_dev_handle *udev = NULL;
	usb_dev_handle *sudev = NULL; //scan device
	unsigned int id = 0; //product id of current device
	char *onoff[] = { "off", "on", "0", "1" };
#ifndef WEBLESS
	char *bindaddr = 0;
#endif
	unsigned int outlet;
	struct plannif plan;

	plannif_reset(&plan);

#ifdef BINDADDR
	if (strlen(BINDADDR) != 0)
		bindaddr = BINDADDR;
#endif

	while ((c = getopt(argc, argv, "i:o:f:t:a:A:b:g:m:lLqvh?nsd:D:u:p:U:")) != -1) {
		if (count == 0) {
			switch (c) {
			case '?':
			case 'h':
			case 'v':
				break;
			default:
				fprintf(stderr, "No GEMBIRD SiS-PM found. Check USB connections, please!\n");
				if (udev != NULL) {
					usb_close(udev);
					udev = NULL;
				}
				exit(1);
			}
		}
		if (strchr("ofgtaAm", c)) {
			if (!strncmp(optarg, "all", strlen("all"))) {
				//use all outlets
				from = 1;
				upto = 4;
			} else {
				from = upto = atoi(optarg);
			}
			if (from < 1 || upto > 4) {
				fprintf(stderr,
					"Invalid outlet number given: %s\n"
					"Expected: 1, 2, 3, 4, or all.\nTerminating.\n",
					optarg);
				print_disclaimer();
				exit(-6);
			}
		} else {
			from = upto = 0;
		}
		if (strchr("ofgbtaAm", c)) { //we need a device handle for these commands
			/* get device-handle/-id if it wasn't done already */
			if (udev == NULL) {
				udev = get_handle(dev[devnum]);
				if (udev == NULL) {
					fprintf(stderr, "No access to Gembird #%d USB device %s\n", devnum, dev[devnum]->filename);
					exit(1);
				} else if (verbose) {
					printf("Accessing Gembird #%d USB device %s\n", devnum, dev[devnum]->filename);
				}
				id = get_id(dev[devnum]);
			}
		}
#ifdef WEBLESS
		if (strchr("lLipu", c)) {
			fprintf(stderr, "Application was compiled without web-interface. "
					"Feature not available.\n");
			exit(-100);
		}
#endif

		// using first available device
		switch (c) {
		case '?':
		case 'h':
		case 'v':
			break;
		default:
			id = get_id(dev[devnum]);
			if (((id == PRODUCT_ID_MSISPM_OLD) || (id == PRODUCT_ID_MSISPM_FLASH)) && (from != upto))
				from = upto = 1;
		}
		for (i = from; i <= upto; ++i) {
			switch (c) {
			case 's':
				for (status = 0; status < count; ++status) {
					if (numeric == 0)
						printf("Gembird #%d\nUSB information:  bus %s, device %s\n", status,
						       dev[status]->bus->dirname, dev[status]->filename);
					else
						printf("%d %s %s\n", status, dev[status]->bus->dirname, dev[status]->filename);
					id = get_id(dev[status]);
					if ((id == PRODUCT_ID_SISPM) || (id == PRODUCT_ID_SISPM_FLASH_NEW) ||
					    (id == PRODUCT_ID_SISPM_EG_PMS2))
						if (numeric == 0)
							printf("device type:      4-socket SiS-PM\n");
						else
							printf("4\n");
					else if (numeric == 0)
						printf("device type:      1-socket mSiS-PM.\n");
					else
						printf("1\n");
					sudev = get_handle(dev[status]);
					id = get_id(dev[status]);
					if (sudev == NULL) {
						fprintf(stderr, "No access to Gembird #%d USB device %s\n", status,
							dev[status]->filename);
						exit(1);
					}
					if (numeric == 0)
						printf("serial number:    %s\n", get_serial(sudev));
					else
						printf("%s\n", get_serial(sudev));
					usb_close(sudev);
					sudev = NULL;
					printf("\n");
				}
				break;
				// select device...
				// replace previous (first is default) device by selected one
			case 'd': // by id
				if (udev != NULL) {
					usb_close(udev);
					udev = NULL;
				}
				devnum = atoi(optarg);
				if ((devnum < 0) || (devnum >= count)) {
					fprintf(stderr, "Invalid number or given device not found.\n"
							"Terminating\n");
					if (udev != NULL) {
						usb_close(udev);
						udev = NULL;
					}
					exit(-8);
				}
				break;
			case 'D': // by serial number
				for (j = 0; j < count; ++j) {
					if (debug)
						fprintf(stderr, "now comparing %s and %s\n", usbdevsn[j], optarg);
					if (strcasecmp(usbdevsn[j], optarg) == 0) {
						if (udev != NULL) {
							usb_close(udev);
							udev = NULL;
						}
						devnum = j;
						break;
					}
				}
				if (devnum != j) {
					fprintf(stderr,
						"No device with serial number %s found.\n"
						"Terminating\n",
						optarg);
					if (udev != NULL) {
						usb_close(udev);
						udev = NULL;
					}
					exit(-8);
				}
				break;
			case 'U': // by USB Bus:Device
				for (j = 0; j < count; ++j) {
					char tmp[8194];
					sprintf(tmp, "%s:%s", dev[j]->bus->dirname, dev[j]->filename);

					if (debug)
						fprintf(stderr, "now comparing %s and %s\n", tmp, optarg);
					if (strcasecmp(tmp, optarg) == 0) {
						if (udev != NULL) {
							usb_close(udev);
							udev = NULL;
						}
						devnum = j;
						break;
					}
				}
				if (devnum != j) {
					fprintf(stderr,
						"No device at USB Bus:Device %s found.\n"
						"Terminating\n",
						optarg);
					if (udev != NULL) {
						usb_close(udev);
						udev = NULL;
					}
					exit(-8);
				}
				break;
			case 'o':
				outlet = check_outlet_number(id, i);
				sispm_switch_on(udev, id, outlet);
				if (verbose)
					printf("Switched outlet %d %s\n", i, onoff[1 + numeric]);
				break;
			case 'f':
				outlet = check_outlet_number(id, i);
				sispm_switch_off(udev, id, outlet);
				if (verbose)
					printf("Switched outlet %d %s\n", i, onoff[0 + numeric]);
				break;
			case 't':
				outlet = check_outlet_number(id, i);
				result = sispm_switch_toggle(udev, id, outlet);
				if (verbose)
					printf("Toggled outlet %d %s\n", i, onoff[result]);
				break;
			case 'A': {
				time_t date, lastEventTime;
				struct tm *timeStamp_tm;
				struct plannif plan;
				int opt, lastAction = 0;
				ulong loop = 0;
				int optindsave = optind;
				int actionNo = 0;

				outlet = check_outlet_number(id, i);

				time(&date);
				timeStamp_tm = localtime(&date);
				lastEventTime = ((ulong)(date / 60)) * 60; // round to previous minute
				plannif_reset(&plan);
				plan.socket = outlet;
				plan.timeStamp = date;
				plan.actions[0].switchOn = 0;

				const struct option opts[] = { { "Ado", 1, NULL, 'd' },
							       { "Aafter", 1, NULL, 'a' },
							       { "Aat", 1, NULL, '@' },
							       { "Aloop", 1, NULL, 'l' },
							       { NULL, 0, 0, 0 } };

				// scan long options and store in plan+loop variables
				while ((opt = getopt_long(argc, argv, "", opts, NULL)) != EOF) {
					if (opt == 'l') {
						loop = atol(optarg);
						continue;
					}
					if (actionNo + 1 >= sizeof(plan.actions) / sizeof(struct plannifAction)) {
						// last event is reserved for loop or stop
						fprintf(stderr, "Too many scheduled events\nTerminating\n");
						exit(-7);
					}
					switch (opt) {
					case 'd':
						plan.actions[actionNo + 1].switchOn = !strcmp(optarg, "on");
						break;
					case 'a':
						plan.actions[actionNo].timeForNext = atol(optarg);
						break;
					case '@': {
						struct tm tm;
						time_t time4next;
						bzero(&tm, sizeof(tm));
						tm.tm_isdst = timeStamp_tm->tm_isdst;
						strptime(optarg, "%Y-%m-%d %H:%M", &tm);
						time4next = mktime(&tm);
						if (time4next > lastEventTime)
							plan.actions[actionNo].timeForNext = (time4next - lastEventTime) / 60;
						else
							plan.actions[actionNo].timeForNext = 0;
						break;
					}
					default:
						fprintf(stderr, "Unknown Option: %s\nTerminating\n", argv[optind - 1]);
						exit(-7);
						break;
					}
					if (plan.actions[actionNo].timeForNext == 0) {
						fprintf(stderr, "Incorrect Date: %s\nTerminating\n", optarg);
						exit(-7);
					}

					if (plan.actions[actionNo].timeForNext != -1 && plan.actions[actionNo + 1].switchOn != -1) {
						lastEventTime += 60 * plan.actions[actionNo].timeForNext;
						++actionNo;
					}
				}

				// compute the value to set in the last row, according to loop
				while (plan.actions[lastAction].timeForNext != -1) {
					if (loop && (lastAction > 0)) {
						// we ignore the first time for the loop calculation
						if (loop <= plan.actions[lastAction].timeForNext) {
							printf("error : the loop period is too short\n");
							exit(1);
						}
						loop -= plan.actions[lastAction].timeForNext;
					}
					++lastAction;
				}
				if (lastAction >= 1)
					plan.actions[lastAction].timeForNext = loop;

				// let's go, and check
				usb_command_setplannif(udev, &plan);
				if (verbose) {
					plannif_reset(&plan);
					usb_command_getplannif(udev, outlet, &plan);
					plannif_display(&plan, 0, NULL);
				}

				if (i < upto)
					optind = optindsave; // reset for next device if needed

				break;
			}
			case 'a':
				outlet = check_outlet_number(id, i);
				struct plannif plan;
				plannif_reset(&plan);
				usb_command_getplannif(udev, outlet, &plan);
				plannif_display(&plan, verbose, argv[0]);
				break;
			case 'g':
				outlet = check_outlet_number(id, i);
				result = sispm_switch_getstatus(udev, id, outlet);
				if (verbose)
					printf("Status of outlet %d:\t", i);
				printf("%s\n", onoff[result + numeric]);
				break;
			case 'm':
				outlet = check_outlet_number(id, i);
				result = sispm_get_power_supply_status(udev, id, outlet);
				if (verbose)
					printf("Power supply status is:\t");
				//take bit 1, which gives the relais status
				printf("%s\n", onoff[result + numeric]);
				break;
#ifndef WEBLESS
			case 'p':
				listenport = atoi(optarg);
				if (verbose)
					printf("Server will listen on port %d.\n", listenport);
				break;
			case 'u':
				homedir = strdup(optarg);
				if (homedir[0] != '/') {
					fprintf(stderr, "'%s' is not an absolute path\n", homedir);
					exit(EXIT_FAILURE);
				}
				if (verbose)
					printf("Web pages come from \"%s\".\n", homedir);
				break;
			case 'i':
				bindaddr = optarg;
				if (verbose)
					printf("Web server will bind on interface with IP %s\n", bindaddr);
				break;
			case 'l':
			case 'L': {
				int *s;

				openlog("sispmctl", LOG_PID, LOG_INFO);
				read_password();
				if (verbose)
					printf("Server goes to listen mode now.\n");
				if ((s = socket_init(bindaddr)) != NULL) {
					if (c == 'l')
						daemonize();
					while (1)
						l_listen(s, dev[devnum], devnum);
				} else
					exit(EXIT_FAILURE);
				break;
			}
#endif
			case 'q':
				verbose = 1 - verbose;
				break;
			case 'n':
				numeric = 2 - numeric;
				break;
			case 'b':
				if (!strncmp(optarg, "on", strlen("on"))) {
					sispm_buzzer_on(udev);
					if (verbose)
						printf("Turned buzzer %s\n", onoff[1 + numeric]);
				} else if (!strncmp(optarg, "off", strlen("off"))) {
					sispm_buzzer_off(udev);
					if (verbose)
						printf("Turned buzzer %s\n", onoff[numeric]);
				} else {
					fprintf(stderr, "Unknown option: -b %s\nTerminating\n", optarg);
					exit(-7);
				}
				break;
			case 'v':
				print_disclaimer();
				break;
			case '?':
			case 'h':
				print_usage(argv[0]);
				exit(1);
			default:
				fprintf(stderr, "Unknown option: %c(%x)\nTerminating\n", c, c);
				exit(-7);
			}
		} // loop through devices
	} // loop through options

	if (udev) {
		usb_close(udev);
		udev = NULL;
	}
	return;
}

int main(int argc, char *argv[])
{
	struct usb_bus *bus;
	struct usb_device *dev, *usbdev[MAXGEMBIRD], *usbdevtemp;
	char *usbdevsn[MAXGEMBIRD];
	int count = 0, found = 0, i = 1;

#ifndef MSG_NOSIGNAL
	signal(SIGPIPE, SIG_IGN);
#endif

	memset(usbdev, 0, sizeof(usbdev));

	usb_init();
	usb_find_busses();
	usb_find_devices();

	// initialize by setting device pointers to zero
	for (count = 0; count < MAXGEMBIRD; ++count)
		usbdev[count] = NULL;
	count = 0;

	//first search for GEMBIRD (m)SiS-PM devices
	for (bus = usb_busses; bus; bus = bus->next) {
		for (dev = bus->devices; dev; dev = dev->next) {
			if ((dev->descriptor.idVendor == VENDOR_ID) && ((dev->descriptor.idProduct == PRODUCT_ID_SISPM) ||
									(dev->descriptor.idProduct == PRODUCT_ID_MSISPM_OLD) ||
									(dev->descriptor.idProduct == PRODUCT_ID_MSISPM_FLASH) ||
									(dev->descriptor.idProduct == PRODUCT_ID_SISPM_FLASH_NEW) ||
									(dev->descriptor.idProduct == PRODUCT_ID_SISPM_EG_PMS2))) {
				usbdev[count] = dev;
				++count;
			}
			if (count == MAXGEMBIRD) {
				fprintf(stderr,
					"%d devices found. Please recompile if you need to "
					"support more devices!\n",
					count);
				break;
			}
		}
	}

	/* bubble sort them first, thnx Ingo Flaschenberger */
	if (count > 1) {
		do {
			found = 0;
			for (i = 1; i < count; ++i) {
				if (usbdev[i]->devnum < usbdev[i - 1]->devnum) {
					usbdevtemp = usbdev[i];
					usbdev[i] = usbdev[i - 1];
					usbdev[i - 1] = usbdevtemp;
					found = 1;
				}
			}
		} while (found != 0);
	}

	/* get serial number of each device */
	for (i = 0; i < count; ++i) {
		usb_dev_handle *sudev = NULL;

		sudev = get_handle(usbdev[i]);
		if (sudev == NULL) {
			fprintf(stderr, "No access to Gembird #%d USB device %s\n", i, usbdev[i]->filename);
			usbdevsn[i] = malloc(5);
			usbdevsn[i][0] = '#';
			usbdevsn[i][1] = '0' + i;
			usbdevsn[i][2] = '\0';
		} else {
			usbdevsn[i] = strdup(get_serial(sudev));
			usb_close(sudev);
			sudev = NULL;
		}
	}

	/* do the real work here */
	if (argc <= 1)
		print_usage(argv[0]);
	else
		parse_command_line(argc, argv, count, usbdev, usbdevsn);

	return 0;
}
