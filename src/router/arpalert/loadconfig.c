/*
 * Copyright (c) 2005-2010 Thierry FOURNIER
 * $Id: loadconfig.c 696 2008-03-31 18:44:59Z  $
 *
 */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "arpalert.h"
#include "loadconfig.h"
#include "log.h"

extern int errno;

char msg[4096];
int dump = 0;

void miseenforme(char*);
void miseenmemoire(char*);
void to_lower(char *);
char lowercase(char);
int convert_octal(char*);
int convert_int(char*);
int convert_boolean(char*);
int end_of_conf = 0;

void usage(){
	printf(
	"\n"
	"arpalert [-f config_file] [-i network_interface]\n"
	"    [-p pid_file] [-e exec_script] [-D log_level]\n"
	"    [-l leases_file] [-d] [-f] [-v] [-h] [-w]\n"
	"    [-P][-V]\n"
	"\n"
	"    -f conf_file: configuration file\n"
	"    -i devices:   comma separated list of interfaces\n"
	"    -p pid_file:  file with pid of daemon\n"
	"    -e script:    script executed whith alerts\n"
	"    -D loglevel:  loglevel (0 to 7)\n"
	"    -l leases:    file to store mac addresses\n"
	"    -m module:    module file to load\n"
	"    -d:           run as daemon\n"
	"    -F:           run in foreground\n"
	"    -v:           dump config\n"
	"    -h:           this help\n"
	"    -w:           debug option: print a dump of packets captured\n"
	"                  (loglevel 7)\n"
	"    -P:           run in promiscuous mode\n"
	"    -V:           version\n"
	"\n");
	exit(1);
}

void config_load(int argc, char *argv[]){
	FILE *fp;
	char buffer[4096];
	char *buf;
	int i;

	/* loading default values */
	config[CF_MACLIST].type = 0;
	config[CF_MACLIST].attrib = "maclist file";
	config[CF_MACLIST].valeur.string = NULL;
	
	config[CF_DUMP_INTER].type = 1;
	config[CF_DUMP_INTER].attrib = "dump inter";
	config[CF_DUMP_INTER].valeur.integer = 5;
	
	config[CF_LOGFILE].type = 0;
	config[CF_LOGFILE].attrib = "log file";
	config[CF_LOGFILE].valeur.string = NULL;
	
	config[CF_ACTION].type = 0;
	config[CF_ACTION].attrib = "action on detect";
	config[CF_ACTION].valeur.string = NULL;
	
	config[CF_MOD_ALERT].type = 0;
	config[CF_MOD_ALERT].attrib = "mod on detect";
	config[CF_MOD_ALERT].valeur.string = NULL;
	
	config[CF_MOD_CONFIG].type = 0;
	config[CF_MOD_CONFIG].attrib = "mod config";
	config[CF_MOD_CONFIG].valeur.string = NULL;
	
	config[CF_LOCKFILE].type = 0;
	config[CF_LOCKFILE].attrib = "lock file";
	config[CF_LOCKFILE].valeur.string = PID_FILE;
	
	config[CF_DAEMON].type = 2;
	config[CF_DAEMON].attrib = "daemon";
	config[CF_DAEMON].valeur.integer = FALSE;
	
	config[CF_ONLY_ARP].type = 2;
	config[CF_ONLY_ARP].attrib = "catch only arp";
	config[CF_ONLY_ARP].valeur.integer = TRUE;
	
	config[CF_RELOAD].type = 1;
	config[CF_RELOAD].attrib = "reload interval";
	config[CF_RELOAD].valeur.integer = 600;
	
	config[CF_LOGLEVEL].type = 1;
	config[CF_LOGLEVEL].attrib = "log level";
	config[CF_LOGLEVEL].valeur.integer = 6;
	
	config[CF_USESYSLOG].type = 2;
	config[CF_USESYSLOG].attrib = "use syslog";
	config[CF_USESYSLOG].valeur.integer = TRUE;
	
	config[CF_TIMEOUT].type = 1;
	config[CF_TIMEOUT].attrib = "execution timeout";
	config[CF_TIMEOUT].valeur.integer = 10;

	config[CF_MAXTH].type = 1;
	config[CF_MAXTH].attrib = "max alert";
	config[CF_MAXTH].valeur.integer = 20;

	config[CF_BLACKLST].type = 0;
	config[CF_BLACKLST].attrib = "maclist alert file";
	config[CF_BLACKLST].valeur.string = NULL;
	
	config[CF_LEASES].type = 0;
	config[CF_LEASES].attrib = "maclist leases file";
	config[CF_LEASES].valeur.string = NULL;
	
	config[CF_IF].type = 0;
	config[CF_IF].attrib = "interface";
	config[CF_IF].valeur.string = NULL;

	config[CF_ABUS].type = 1;
	config[CF_ABUS].attrib = "max request";
	config[CF_ABUS].valeur.integer = 1000000;
	
	config[CF_MAXENTRY].type = 1;
	config[CF_MAXENTRY].attrib = "max entry";
	config[CF_MAXENTRY].valeur.integer = 1048576;
	
	config[CF_DMPWL].type = 2;
	config[CF_DMPWL].attrib = "dump white list";
	config[CF_DMPWL].valeur.integer = FALSE;
			
	config[CF_DMPBL].type = 2;
	config[CF_DMPBL].attrib = "dump black list";
	config[CF_DMPBL].valeur.integer = FALSE;
		
	config[CF_DMPAPP].type = 2;
	config[CF_DMPAPP].attrib = "dump new address";
	config[CF_DMPAPP].valeur.integer = TRUE;
	
	config[CF_TOOOLD].type = 1;
	config[CF_TOOOLD].attrib = "mac timeout";
	config[CF_TOOOLD].valeur.integer = 2592000; // 1 month

	config[CF_IGNORE_ME].type = 2;
	config[CF_IGNORE_ME].attrib = "ignore me";
	config[CF_IGNORE_ME].valeur.integer = TRUE;
	
	config[CF_UMASK].type = 3;
	config[CF_UMASK].attrib = "umask";
	config[CF_UMASK].valeur.integer = 0133;

	config[CF_USER].type = 0;
	config[CF_USER].attrib = "user";
	config[CF_USER].valeur.string = NULL;

	config[CF_CHROOT].type = 0;
	config[CF_CHROOT].attrib = "chroot dir";
	config[CF_CHROOT].valeur.string = NULL;

	config[CF_IGNORESELFTEST].type = 2;
	config[CF_IGNORESELFTEST].attrib = "ignore self test";
	config[CF_IGNORESELFTEST].valeur.integer = TRUE;

	config[CF_AUTHFILE].type = 0;
	config[CF_AUTHFILE].attrib = "auth request file";
	config[CF_AUTHFILE].valeur.string = NULL;

	config[CF_UNAUTH_TO_METHOD].type = 1;
	config[CF_UNAUTH_TO_METHOD].attrib = "unauth ignore time method";
	config[CF_UNAUTH_TO_METHOD].valeur.integer = 2;
	
	config[CF_IGNORE_UNKNOWN].type = 2;
	config[CF_IGNORE_UNKNOWN].attrib = "ignore unknown sender";
	config[CF_IGNORE_UNKNOWN].valeur.integer = TRUE;

	config[CF_DUMP_PAQUET].type = 2;
	config[CF_DUMP_PAQUET].attrib = "dump paquet";
	config[CF_DUMP_PAQUET].valeur.integer = FALSE;
	config[CF_DUMP_PACKET].type = 2;
	config[CF_DUMP_PACKET].attrib = "dump packet";
	config[CF_DUMP_PACKET].valeur.integer = FALSE;

	config[CF_PROMISC].type = 2;
	config[CF_PROMISC].attrib = "promiscuous";
	config[CF_PROMISC].valeur.integer = FALSE;

	// convert mac to vendor name
	config[CF_MACCONV_FILE].type = 0;
	config[CF_MACCONV_FILE].attrib = "mac vendor file";
	config[CF_MACCONV_FILE].valeur.string = NULL;
	config[CF_LOG_VENDOR].type = 2;
	config[CF_LOG_VENDOR].attrib = "log mac vendor";
	config[CF_LOG_VENDOR].valeur.integer = FALSE;
	config[CF_ALERT_VENDOR].type = 2;
	config[CF_ALERT_VENDOR].attrib = "alert mac vendor";
	config[CF_ALERT_VENDOR].valeur.integer = FALSE;
	config[CF_MOD_VENDOR].type = 2;
	config[CF_MOD_VENDOR].attrib = "mod mac vendor";
	config[CF_MOD_VENDOR].valeur.integer = FALSE;

	config[CF_ANTIFLOOD_INTER].type = 1;
	config[CF_ANTIFLOOD_INTER].attrib = "anti flood interval";
	config[CF_ANTIFLOOD_INTER].valeur.integer = 10; /* 10 secondes */
	
	config[CF_ANTIFLOOD_GLOBAL].type = 1;
	config[CF_ANTIFLOOD_GLOBAL].attrib = "anti flood global";
	config[CF_ANTIFLOOD_GLOBAL].valeur.integer = 50; /* 50 secondes */

	config[CF_LOG_ALLOW].type = 2;
	config[CF_LOG_ALLOW].attrib = "log referenced address";
	config[CF_LOG_ALLOW].valeur.integer = FALSE;
	config[CF_ALERT_ALLOW].type = 2;
	config[CF_ALERT_ALLOW].attrib = "alert on referenced address";
	config[CF_ALERT_ALLOW].valeur.integer = FALSE;
	config[CF_MOD_ALLOW].type = 2;
	config[CF_MOD_ALLOW].attrib = "mod on referenced address";
	config[CF_MOD_ALLOW].valeur.integer = FALSE;

	config[CF_LOG_DENY].type = 2;
	config[CF_LOG_DENY].attrib = "log deny address";
	config[CF_LOG_DENY].valeur.integer = TRUE;
	config[CF_ALERT_DENY].type = 2;
	config[CF_ALERT_DENY].attrib = "alert on deny address";
	config[CF_ALERT_DENY].valeur.integer = TRUE;
	config[CF_MOD_DENY].type = 2;
	config[CF_MOD_DENY].attrib = "mod on deny address";
	config[CF_MOD_DENY].valeur.integer = TRUE;

	config[CF_LOG_NEW].type = 2;
	config[CF_LOG_NEW].attrib = "log new address";
	config[CF_LOG_NEW].valeur.integer = TRUE;
	config[CF_ALERT_NEW].type = 2;
	config[CF_ALERT_NEW].attrib = "alert on new address";
	config[CF_ALERT_NEW].valeur.integer = TRUE;
	config[CF_MOD_NEW].type = 2;
	config[CF_MOD_NEW].attrib = "mod on new address";
	config[CF_MOD_NEW].valeur.integer = TRUE;

	config[CF_LOG_NEWMAC].type = 2;
	config[CF_LOG_NEWMAC].attrib = "log new mac address";
	config[CF_LOG_NEWMAC].valeur.integer = TRUE;
	config[CF_ALERT_NEWMAC].type = 2;
	config[CF_ALERT_NEWMAC].attrib = "alert on new mac address";
	config[CF_ALERT_NEWMAC].valeur.integer = TRUE;
	config[CF_MOD_NEWMAC].type = 2;
	config[CF_MOD_NEWMAC].attrib = "mod on new mac address";
	config[CF_MOD_NEWMAC].valeur.integer = TRUE;

	config[CF_LOG_IPCHG].type = 2;
	config[CF_LOG_IPCHG].attrib = "log ip change";
	config[CF_LOG_IPCHG].valeur.integer = TRUE;
	config[CF_ALERT_IPCHG].type = 2;
	config[CF_ALERT_IPCHG].attrib = "alert on ip change";
	config[CF_ALERT_IPCHG].valeur.integer = TRUE;
	config[CF_MOD_IPCHG].type = 2;
	config[CF_MOD_IPCHG].attrib = "mod on ip change";
	config[CF_MOD_IPCHG].valeur.integer = TRUE;

	config[CF_LOG_UNAUTH_RQ].type = 2;
	config[CF_LOG_UNAUTH_RQ].attrib = "log unauth request";
	config[CF_LOG_UNAUTH_RQ].valeur.integer = TRUE;
	config[CF_ALERT_UNAUTH_RQ].type = 2;
	config[CF_ALERT_UNAUTH_RQ].attrib = "alert on unauth request";
	config[CF_ALERT_UNAUTH_RQ].valeur.integer = TRUE;
	config[CF_MOD_UNAUTH_RQ].type = 2;
	config[CF_MOD_UNAUTH_RQ].attrib = "mod on unauth request";
	config[CF_MOD_UNAUTH_RQ].valeur.integer = TRUE;

	config[CF_LOG_ABUS].type = 2;
	config[CF_LOG_ABUS].attrib = "log request abus";
	config[CF_LOG_ABUS].valeur.integer = TRUE;
	config[CF_ALERT_ABUS].type = 2;
	config[CF_ALERT_ABUS].attrib = "alert on request abus";
	config[CF_ALERT_ABUS].valeur.integer = TRUE;
	config[CF_MOD_ABUS].type = 2;
	config[CF_MOD_ABUS].attrib = "mod on request abus";
	config[CF_MOD_ABUS].valeur.integer = TRUE;

	config[CF_LOG_BOGON].type = 2;
	config[CF_LOG_BOGON].attrib = "log mac error";
	config[CF_LOG_BOGON].valeur.integer = TRUE;
	config[CF_ALERT_BOGON].type = 2;
	config[CF_ALERT_BOGON].attrib = "alert on mac error";
	config[CF_ALERT_BOGON].valeur.integer = TRUE;
	config[CF_MOD_BOGON].type = 2;
	config[CF_MOD_BOGON].attrib = "mod on mac error";
	config[CF_MOD_BOGON].valeur.integer = TRUE;

	config[CF_LOG_FLOOD].type = 2;
	config[CF_LOG_FLOOD].attrib = "log flood";
	config[CF_LOG_FLOOD].valeur.integer = TRUE;
	config[CF_ALERT_FLOOD].type = 2;
	config[CF_ALERT_FLOOD].attrib = "alert on flood";
	config[CF_ALERT_FLOOD].valeur.integer = TRUE;
	config[CF_MOD_FLOOD].type = 2;
	config[CF_MOD_FLOOD].attrib = "mod on flood";
	config[CF_MOD_FLOOD].valeur.integer = TRUE;

	config[CF_LOG_MACCHG].type = 2;
	config[CF_LOG_MACCHG].attrib = "log mac change";
	config[CF_LOG_MACCHG].valeur.integer = TRUE; 
	config[CF_ALERT_MACCHG].type = 2;
	config[CF_ALERT_MACCHG].attrib = "alert on mac change";
	config[CF_ALERT_MACCHG].valeur.integer = TRUE; 
	config[CF_MOD_MACCHG].type = 2;
	config[CF_MOD_MACCHG].attrib = "mod on mac change";
	config[CF_MOD_MACCHG].valeur.integer = TRUE;

	// load command line parameters for config file
	strncpy(config_file, CONFIG_FILE, CONFIGFILE_LEN);
	for(i=1; i<argc; i++){
		if(argv[i][0]=='-' && argv[i][1]=='h'){
			usage();
		}
		if(argv[i][0]=='-' && argv[i][1]=='f'){
			if(i+1 >= argc){
				logmsg(LOG_ERR, "Option -f without argument");
				usage();
			}
			i++;
			strncpy(config_file, argv[i], CONFIGFILE_LEN);
		}
	}

	// load config file values
	buf = buffer;
	fp = fopen(config_file, "r");
	if(fp == NULL){
		logmsg(LOG_ERR, "didn't find %s, loading default config",
		        config_file);
	} else {
		while((buf = fgets(buf, 4096, fp)) != NULL){
			miseenforme(buf);
			if(buf[0] != 0){
				miseenmemoire(buf);
			}
		}
		if(fclose(fp) == EOF){
			logmsg(LOG_ERR, "[%s %d] fclose[%d]: %s",
			       __FILE__, __LINE__, errno, strerror(errno));
			exit(1);
		}
	}

	// load command line parameters
	// (this supplant config file params)
	for(i=1; i<argc; i++){
		if(argv[i][0]=='-'){
			switch(argv[i][1]){
				case 'f':
					i++;
					break;

				case 'i':
					if(i+1 >= argc){
						logmsg(LOG_ERR, "Option -i without argument");
						usage();
					}
					i++;
					if(config[CF_IF].valeur.string != NULL){
						free(config[CF_IF].valeur.string);
					}
					config[CF_IF].valeur.string = strdup(argv[i]);
					break;
	
				case 'p':
					if(i+1 >= argc){
						logmsg(LOG_ERR, "Option -p without argument");
						usage();
					}
					i++;
					if(config[CF_LOCKFILE].valeur.string != NULL){
						free(config[CF_LOCKFILE].valeur.string);
					}
					config[CF_LOCKFILE].valeur.string = strdup(argv[i]);
					break;
	
				case 'e':
					if(i+1 >= argc){
						logmsg(LOG_ERR, "Option -e without argument");
						usage();
					}
					i++;
					if(config[CF_ACTION].valeur.string != NULL){
						free(config[CF_ACTION].valeur.string);
					}
					config[CF_ACTION].valeur.string = strdup(argv[i]);
					break;
				
				case 'D':
					if(i+1 >= argc){
						logmsg(LOG_ERR, "Option -D without argument");
						usage();
					}
					i++;
					if(argv[i][0] < 48 || argv[i][0] > 55){
						logmsg(LOG_ERR, "Wrong -D parameter");
						usage();
					}
					config[CF_LOGLEVEL].valeur.integer = argv[i][0] - 48;
					break;
	
				case 'l':
					if(i+1 >= argc){
						logmsg(LOG_ERR, "Option -l without argument");
						usage();
					}
					i++;
					if(config[CF_LEASES].valeur.string != NULL){
						free(config[CF_LEASES].valeur.string);
					}
					config[CF_LEASES].valeur.string = strdup(argv[i]);
					break;
	
				case 'm':
					if(i+1 >= argc){
						logmsg(LOG_ERR, "Option -m without argument");
						usage();
					}
					i++;
					if(config[CF_MOD_ALERT].valeur.string != NULL){
						free(config[CF_MOD_ALERT].valeur.string);
					}
					config[CF_MOD_ALERT].valeur.string = strdup(argv[i]);
					break;

				case 'v':
					dump = 1;
					break;
				
				case 'V':
					printf("arpalert %s\n", PACKAGE_VERSION);
					exit(0);
					break;
				
				case 'w':
					config[CF_DUMP_PAQUET].valeur.integer = TRUE;
					break;
				
				case 'd':
					config[CF_DAEMON].valeur.integer = TRUE;
					break;
	
				case 'F':
					config[CF_DAEMON].valeur.integer = FALSE;
					break;
	
				case 'P':
					config[CF_PROMISC].valeur.integer = TRUE;
					break;
					
				case 'h':
				case '?':
				default:
					logmsg(LOG_ERR, "Wrong option: -%c", argv[i][1]);
					usage();
					exit(1);
					break;
			}
		}
	}

	if(dump==1){
		for(i=0; i<NUM_PARAMS; i++){
			switch(config[i].type){
				case 0:
					if(config[i].valeur.string == NULL){
						logmsg(LOG_NOTICE, "%s = \"\"",
						       config[i].attrib);
					}

					else {
						logmsg(LOG_NOTICE, "%s = \"%s\"",
						       config[i].attrib, config[i].valeur.string);
					}
				break;

				case 1:
					logmsg(LOG_NOTICE, "%s = %i",
					       config[i].attrib, config[i].valeur.integer);
				break;

				case 2:
					if(config[i].valeur.integer == TRUE){
						logmsg(LOG_NOTICE, "%s = true", config[i].attrib);
					} else {
						logmsg(LOG_NOTICE, "%s = false", config[i].attrib);
					}
				break;
			}
		}
	}
}

void miseenmemoire(char *buf){
	char *src = NULL;
	char *m_eq = NULL;
	char *m_end = NULL;
	char m_gauche[4096];
	char m_droite[4096];
	char *gauche = NULL;
	char *droite = NULL;
	char *g = NULL;
	char *d = NULL;
	int i;
	int protection, ok;
	
	gauche = m_gauche;
	droite = m_droite;
	g = gauche;
	d = droite;
	src = buf;

	protection = 0;
	while(*src != 0){
		if(*src == '"'){
			protection ^= 0xff;
			src++;
			continue;
		}
		if(protection != 255 && *src=='=')m_eq=src;
		src++;
	}
	if(m_eq==NULL){
		logmsg(LOG_ERR, "error in config file at line: \"%s\"", buf);
		exit(1);
	}
	m_end=src;
	if(*m_eq!='='){
		logmsg(LOG_ERR, "error in config file at line: \"%s\"", buf);
		exit(1);
	}
	if(*(m_eq-1)!=' '){
		logmsg(LOG_ERR, "error in config file at line: \"%s\"", buf);
		exit(1);
	}
	if(*(m_eq+1)!=' '){
		logmsg(LOG_ERR, "error in config file at line: \"%s\"", buf);
		exit(1);
	}
	src=buf;
	while(src<(m_eq-1)){
		*gauche=lowercase(*src);
		gauche++;
		src++;
	}
	*gauche=0;
	src+=3;
	if(*src=='"'){
		src++;
		m_end--;
	}
	while(src<m_end){
		*droite=*src;
		droite++;
		src++;
	}
	*droite=0;

	i = 0;
	ok = 0;
	while(i < NUM_PARAMS){
		if(strcmp(config[i].attrib, g)==0){
			switch(config[i].type){
				case 0: config[i].valeur.string = strdup(d);
				        break;

				case 1: config[i].valeur.integer = convert_int(d);
				        break;

				case 2: config[i].valeur.integer = convert_boolean(d);
				        break;

				case 3: config[i].valeur.integer = convert_octal(d);
				        break;
			}
			ok = 1;
			break;
		}
		i++;
	}
	if(ok == 0){
		logmsg(LOG_ERR, "error in config file at "
		        "line: \"%s\": parameter inexistent",
		        buf);
		exit(1);
	}
}

int convert_octal(char *buf){
	int res = 0;
	char *b;
	
	int i;

	b = buf;
	while(*buf != 0){
		if(*buf<'0' || *buf>'7'){
			logmsg(LOG_ERR, "error in config file in "
			        "string \"%s\": octal value expected",
			        b);
			exit(1);
		}
		i = res;
		res *= 8;
		res += *buf - 48;
		buf++;
	}
	return res;
}

int convert_int(char *buf){
	int res = 0;
	char *b;

	b = buf;
	while(*buf != 0){
		if(*buf<'0' || *buf>'9'){
			logmsg(LOG_ERR, "error in config file in "
			        "string \"%s\": integer value expected",
			        b);
			exit(1);
		}
		res *= 10;
		res += *buf - 48;
		buf++;
	}	
	return res;
}

int convert_boolean(char *buf){
	to_lower(buf);

	if(strcmp("oui",   buf) == 0) return(TRUE);
	if(strcmp("yes",   buf) == 0) return(TRUE);
	if(strcmp("true",  buf) == 0) return(TRUE);
	if(strcmp("1",     buf) == 0) return(TRUE);
	
	if(strcmp("non",   buf) == 0) return(FALSE);
	if(strcmp("no",    buf) == 0) return(FALSE);
	if(strcmp("false", buf) == 0) return(FALSE);
	if(strcmp("0",     buf) == 0) return(FALSE);

	logmsg(LOG_ERR, "error in config file: boolean value expected [%s]",
		buf);
	exit(1);	 
}

void to_lower(char *in){
	while(*in != 0){
		if(*in > 64 && *in < 91)*in+=32;
		in++;
	}
}

char lowercase(char in){
	if(in > 64 && in < 91)in+=32;
	return in;
}

void miseenforme(char *params){
	char *src;
	char *dst;
	char *fin;
	char *mem;
	int protection;
	int debut;
	int space;

	// delete comments
	src = params;
	protection = 0;
	while(*src != 0){
		if(*src == '"'){
			protection ^= 0xff;
			src++;
			continue;
		}
		if(protection != 0xff && *src == '#'){
			*src=0;
			break;
		}
		src++;
	}

	// delete unused blank characters
	debut = 0;
	space = 0;	
	src = params;
	dst = params;
	protection = 0;
	while(*src != 0){
		/* on conserve l'interieur des guillemets sans modifs */
		if(*src == '"' || protection == 0xff){
			*dst = *src;
			if(*src == '"'){
				protection = 0x00; 
			}
			src++;
			dst++;
			continue;
		}

		// ignore end line character
		if(*src == '\n' || *src == '\r'){
			src++;
			continue;
		}

		/* si on a un caracteres non blanc on le recopie */
		if(!(*src == ' ' || *src == '\t')){
			debut = 1;
			*dst = *src;
			dst++;
			src++;
			space = 0;
			continue;
		}

		/* dans les autres cas, on considere que c'est un espace,
		 * si ce n'est pas le premier, on met un espace */
		if(space == 0 && debut == 1){
			*dst=' ';
			dst++;
		}
		space = 1;
		src++;
	}
	*dst=0;
	fin = dst;

	/* suppresion de l'eventuel dernier espace */
	if(*(fin-1)==' '){
		fin--;
		dst--;
		*fin=0;
	}

	/* mise en forme des egales colles: peuvent etre mot=mot */
	src = params;
	protection = 0;
	while(*src != 0){
		if(*src == '"'){
			protection ^= 0xff;
			src++;
			continue;
		}
		if(protection != 255 && *src == '=')break;
		src++;
	}
	if(*src == '='){
		mem = src;
		if(*(src+1)!=' ')fin++;
		if(*(src-1)!=' '){fin++; src++;}

		*(fin+1)=0;
		while(fin!=mem){
			*fin=*dst;
			fin--;
			dst--;
		}

		*(src-1) = ' ';
		*src     = '=';
		*(src+1) = ' ';
	}
}

void set_end_of_conf(void) {
	end_of_conf = 1;
}

void set_option(int opt, void *value) {
	if ( end_of_conf == 1 ) {
		logmsg(LOG_ALERT, "can not modify config");
		return;
	}

	// check for unauthorized options
	switch (opt) {
		case CF_MOD_ALERT:
		case CF_MOD_CONFIG:
		case CF_USESYSLOG:
		case CF_LOGFILE:
			logmsg(LOG_ALERT, "can not modify config option %d", opt);
			return;
	}

	switch ( config[opt].type ) {
		case 0:
			if ( value == NULL ) {
				config[opt].valeur.string = NULL;
			} else {
				config[opt].valeur.string = strdup((char *)value);
			}
			break;
		case 1:
		case 2:
			config[opt].valeur.integer = (int)value;
			break;
	}
}
