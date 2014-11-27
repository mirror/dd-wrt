/*
 * Copyright (c) 2005-2010 Thierry FOURNIER
 * $Id: loadmodule.c 690 2008-03-31 18:36:43Z  $
 *
 */

#include "config.h"

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "loadconfig.h"
#include "log.h"
#include "loadmodule.h"

extern int errno;

// function call when an alertis send
void (* mod_alert)(int type, int nargs, void **data);
void (* mod_load)(char *config);
void (* mod_unload)(void);
void *module;

// this function load module
void module_load(void){
	struct stat entinfo;
	const char *tmp;

	module = NULL;

	// check the parameter
	if(config[CF_MOD_ALERT].valeur.string == NULL ||
	   config[CF_MOD_ALERT].valeur.string[0] == 0){
		return;
	}

	// test if is a regular file
	if(stat(config[CF_MOD_ALERT].valeur.string, &entinfo) == -1){
		logmsg(LOG_ERR, "[%s %d] stat[%d]: %s (%s)",
		       __FILE__, __LINE__, errno, strerror(errno),
		       config[CF_MOD_ALERT].valeur.string);
		abort();
	}
	if(!S_ISREG(entinfo.st_mode)){
		logmsg(LOG_ERR, "the module file \"%s\" is a regular file",
		       config[CF_MOD_ALERT].valeur.string);
	}

	// load module and resolve symbols
	module = dlopen(config[CF_MOD_ALERT].valeur.string, RTLD_NOW);
	if(!module) {
		logmsg(LOG_ERR, "[%s %d] dlopen: %s",
		       __FILE__, __LINE__, 
		       dlerror());
		exit(1);
	}

	// finding entry point mod_alert
	mod_alert = dlsym(module, "mod_alert");
	tmp = dlerror();
	if(tmp != NULL) {
		logmsg(LOG_ERR, "[%s %d] dlsym: %s",
		       __FILE__, __LINE__,
		      tmp);
		exit(1);
	}

	// finding entry point mod_load
	mod_load = dlsym(module, "mod_load");
	tmp = dlerror();
	if(tmp != NULL) {
		logmsg(LOG_NOTICE, "%s", tmp);
		mod_load = NULL;
	}
	
	// finding entry point mod_unload
	mod_unload = dlsym(module, "mod_unload");
	tmp = dlerror();
	if(tmp != NULL) {
		logmsg(LOG_NOTICE, "%s", tmp);
		mod_unload = NULL;
	}

	// module initialisation call
	if(mod_load != NULL) mod_load(config[CF_MOD_CONFIG].valeur.string);

	// can not modify config
	set_end_of_conf();
}

// unload and close module
void module_unload(void){
	const char *tmp;

	// check avalaibility of modules
	if(config[CF_MOD_ALERT].valeur.string == NULL ||
	   config[CF_MOD_ALERT].valeur.string[0] == 0){
		return;
	}

	// unload module
	if(mod_unload != NULL){
		mod_unload();
	}

	// close module
	dlclose(module);
	tmp = dlerror();
	if(tmp != NULL) {
		logmsg(LOG_ERR, "[%s %d] dlclose: %s",
		       __FILE__, __LINE__, tmp);
	}
}

// this finction execute module
// function call when an alertis send
void alerte_mod(struct ether_addr *mac_sender,
                struct in_addr ip_sender,
                int type,
                struct ether_addr *ref_mac,
                struct in_addr ref_ip,
					 char *interface,
                char *vendor){
	void *args[4];
	int parm_num = 3;

	// check the parameter
	if(config[CF_MOD_ALERT].valeur.string == NULL ||
	   config[CF_MOD_ALERT].valeur.string[0] == 0){
		return;
	}

	args[0] = (void *)interface;
	args[1] = (void *)mac_sender;
	args[2] = (void *)ip_sender.s_addr;
	switch(type){
		case 0:
		case 4:
			args[3] = &ref_ip;
			parm_num = 4;
			break;
			
		case 6:
		case 9:
			args[3] = ref_mac;
			parm_num = 4;
			break;
	}
	if(config[CF_MOD_VENDOR].valeur.integer){
		args[parm_num] = vendor;
		parm_num++;
	}
	
	mod_alert(type, parm_num, args);
	
}

