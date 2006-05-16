#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <wlutils.h>
#include <wlioctl.h>
#include <utils.h>
#include <sveasoft.h>

char* get_wdev(void);
char* get_ldev(void);
int del_ebtables(void);
int add_ebtables(void);
