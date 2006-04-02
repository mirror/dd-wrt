/* 
 * files.c -- DHCP server file manipulation *
 * Rewrite by Russ Dill <Russ.Dill@asu.edu> July 2001
 */
 
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <netdb.h>

#include "debug.h"
#include "dhcpd.h"
#include "files.h"
#include "options.h"
#include "leases.h"
#include "get_time.h"
#include <shutils.h>

/* on these functions, make sure you datatype matches */
static int read_ip(char *line, void *arg)
{
	struct in_addr *addr = arg;
	struct hostent *host;
	int retval = 1;

	if (!inet_aton(line, addr)) {
		if ((host = gethostbyname(line))) 
			addr->s_addr = *((unsigned long *) host->h_addr_list[0]);
		else retval = 0;
	}
	return retval;
}


static int read_str(char *line, void *arg)
{
	char **dest = arg;
	
	if (*dest) free(*dest);
	*dest = strdup(line);
	
	return 1;
}


static int read_u32(char *line, void *arg)
{
	u_int32_t *dest = arg;
	char *endptr;
	*dest = strtoul(line, &endptr, 0);
	return endptr[0] == '\0';
}


static int read_yn(char *line, void *arg)
{
	char *dest = arg;
	int retval = 1;

	if (!strcasecmp("yes", line))
		*dest = 1;
	else if (!strcasecmp("no", line))
		*dest = 0;
	else retval = 0;
	
	return retval;
}


/* read a dhcp option and add it to opt_list */
static int read_opt(char *line, void *arg)
{
	struct option_set **opt_list = arg;
	char *opt, *val, *endptr;
	struct dhcp_option *option = NULL;
	int retval = 0, length = 0;
	char buffer[255];
	u_int16_t result_u16;
	u_int32_t result_u32;
	int i;

	if (!(opt = strtok(line, " \t="))) return 0;
	
	for (i = 0; options[i].code; i++)
		if (!strcmp(options[i].name, opt))
			option = &(options[i]);
		
	if (!option) return 0;
	
	do {
		val = strtok(NULL, ", \t");
		if (val) {
			length = option_lengths[option->flags & TYPE_MASK];
			retval = 0;
			switch (option->flags & TYPE_MASK) {
			case OPTION_IP:
				retval = read_ip(val, buffer);
				break;
			case OPTION_IP_PAIR:
				retval = read_ip(val, buffer);
				if (!(val = strtok(NULL, ", \t/-"))) retval = 0;
				if (retval) retval = read_ip(val, buffer + 4);
				break;
			case OPTION_STRING:
				length = strlen(val);
				if (length > 0) {
					if (length > 254) length = 254;
					memcpy(buffer, val, length);
					retval = 1;
				}
				break;
			case OPTION_BOOLEAN:
				retval = read_yn(val, buffer);
				break;
			case OPTION_U8:
				buffer[0] = strtoul(val, &endptr, 0);
				retval = (endptr[0] == '\0');
				break;
			case OPTION_U16:
				result_u16 = htons(strtoul(val, &endptr, 0));
				memcpy(buffer, &result_u16, 2);
				retval = (endptr[0] == '\0');
				break;
			case OPTION_S16:
				result_u16 = htons(strtol(val, &endptr, 0));
				memcpy(buffer, &result_u16, 2);
				retval = (endptr[0] == '\0');
				break;
			case OPTION_U32:
				result_u32 = htonl(strtoul(val, &endptr, 0));
				memcpy(buffer, &result_u32, 4);
				retval = (endptr[0] == '\0');
				break;
			case OPTION_S32:
				result_u32 = htonl(strtol(val, &endptr, 0));	
				memcpy(buffer, &result_u32, 4);
				retval = (endptr[0] == '\0');
				break;
			default:
				break;
			}
			if (retval) 
				attach_option(opt_list, option, buffer, length);
		};
	} while (val && retval && option->flags & OPTION_LIST);
	return retval;
}


static struct config_keyword keywords[] = {
	/* keyword[14]	handler   variable address		default[20] */
	{"start",	read_ip,  &(server_config.start),	"192.168.0.20"},
	{"end",		read_ip,  &(server_config.end),		"192.168.0.254"},
	{"interface",	read_str, &(server_config.interface),	"eth0"},
	{"wan_interface",       read_str, &(server_config.wan_interface),       "eth1"},	// Added by honor
	{"option",	read_opt, &(server_config.options),	""},
	{"opt",		read_opt, &(server_config.options),	""},
	{"max_leases",	read_u32, &(server_config.max_leases),	"254"},
	{"remaining",	read_yn,  &(server_config.remaining),	"yes"},
	{"auto_time",	read_u32, &(server_config.auto_time),	"7200"},
	{"decline_time",read_u32, &(server_config.decline_time),"3600"},
	{"conflict_time",read_u32,&(server_config.conflict_time),"3600"},
	{"offer_time",	read_u32, &(server_config.offer_time),	"60"},
	{"min_lease",	read_u32, &(server_config.min_lease),	"60"},
	{"lease_file",	read_str, &(server_config.lease_file),	"/var/lib/misc/udhcpd.leases"},
	{"pidfile",	read_str, &(server_config.pidfile),	"/var/run/udhcpd.pid"},
	{"notify_file", read_str, &(server_config.notify_file),	""},
	{"siaddr",	read_ip,  &(server_config.siaddr),	"0.0.0.0"},
	{"sname",	read_str, &(server_config.sname),	""},
	{"boot_file",	read_str, &(server_config.boot_file),	""},
	/*ADDME: static lease */
	{"statics_file",read_str, &(server_config.statics_file),""},
	{"",		NULL, 	  NULL,				""}
};


int read_config(char *file)
{
	FILE *in;
	char buffer[80], orig[80], *token, *line;
	int i;

	for (i = 0; strlen(keywords[i].keyword); i++)
		if (strlen(keywords[i].def))
			keywords[i].handler(keywords[i].def, keywords[i].var);

	if (!(in = fopen(file, "r"))) {
		LOG(LOG_ERR, "unable to open config file: %s", file);
		return 0;
	}
	
	while (fgets(buffer, 80, in)) {
		if (strchr(buffer, '\n')) *(strchr(buffer, '\n')) = '\0';
		strncpy(orig, buffer, 80);
		if (strchr(buffer, '#')) *(strchr(buffer, '#')) = '\0';
		token = buffer + strspn(buffer, " \t");
		if (*token == '\0') continue;
		line = token + strcspn(token, " \t=");
		if (*line == '\0') continue;
		*line = '\0';
		line++;
		
		/* eat leading whitespace */
		line = line + strspn(line, " \t=");
		/* eat trailing whitespace */
		for (i = strlen(line); i > 0 && isspace(line[i - 1]); i--);
		line[i] = '\0';
		
		for (i = 0; strlen(keywords[i].keyword); i++)
			if (!strcasecmp(token, keywords[i].keyword))
				if (!keywords[i].handler(line, keywords[i].var)) {
					LOG(LOG_ERR, "unable to parse '%s'", orig);
					/* reset back to the default value */
					keywords[i].handler(keywords[i].def, keywords[i].var);
				}
	}
	fclose(in);
	return 1;
}


void write_leases(void)
{
	FILE *fp;
	unsigned int i;
	char buf[255];
	time_t curr = get_time(0);
	unsigned long lease_time;
	
	if (!(fp = fopen(server_config.lease_file, "w"))) {
		LOG(LOG_ERR, "Unable to open %s for writing", server_config.lease_file);
		return;
	}
	
	for (i = 0; i < server_config.max_leases; i++) {
		if (leases[i].yiaddr != 0) {
			if(leases[i].expires == EXPIRES_NEVER) {
				/* wmx: static lease */
				lease_time = EXPIRES_NEVER;
			} else {
				if (server_config.remaining) {
					if (lease_expired(&(leases[i])))
						lease_time = 0;
					else lease_time = leases[i].expires - curr;
				} else lease_time = leases[i].expires;
			}
			lease_time = htonl(lease_time);
			fwrite(leases[i].chaddr, 16, 1, fp);
			fwrite(&(leases[i].yiaddr), 4, 1, fp);
			fwrite(&lease_time, 4, 1, fp);
			fwrite(leases[i].hostname, 64, 1, fp);
		}
	}
	fclose(fp);
	
	if (server_config.notify_file) {
		sprintf(buf, "%s %s", server_config.notify_file, server_config.lease_file);
		system(buf);
	}
}

/* delete leases from memory , by honor*/
void delete_leases(int dummy)
{
	int i=0,j,count;
	struct in_addr addr;
	struct in_addr in;
	FILE *fp;

	u_int32_t table[server_config.max_leases];
	char line[80];

	dummy = 0;

	LOG(LOG_DEBUG,"Receive SIGUSR2 : delete_leases()");

	bzero(table,sizeof(table));
	
	/* read "/tmp/delete_leases" which generated from DHCPTable.asp */
	if ((fp = fopen("/tmp/.delete_leases", "r"))!=NULL) {
		while(i<(int)server_config.max_leases && fgets(line,sizeof(line),fp)!=NULL){
			LOG(LOG_DEBUG,"line %d [%s] from /tmp/.delete_leases",i,line);
			inet_aton(line, &in);
			table[i]=(u_int32_t)in.s_addr;
			i++;
		}
	   fclose(fp);
	}
	count = i;


       for (j = 0; j < (int)server_config.max_leases && count; j++) {
           if (leases[j].yiaddr != 0) {    //have value
                 addr.s_addr = leases[j].yiaddr;
                 LOG(LOG_DEBUG,"count %d [%-15s] [%lu] address[%lu] from memory", j,inet_ntoa(addr),(unsigned long)leases[j].yiaddr,(unsigned long)&leases[j]); 

                 /* search*/
	         for(i=0;i<count ;i++){
                    if(leases[j].yiaddr == table[i]) {
			 memset(&leases[j], 0, sizeof(struct dhcpOfferedAddr));   //clear memory value
                         LOG(LOG_DEBUG,"match & delete [%-15s] [%lu]", inet_ntoa(addr),(unsigned long)leases[j].yiaddr); 
			 break;
                    }
	         }//end for
           }//end if
      }
      
	read_statics(server_config.statics_file);
}

void read_leases(char *file)
{
	FILE *fp;
	unsigned int i = 0;
	struct dhcpOfferedAddr lease, *oldest;
	
	if (!(fp = fopen(file, "r"))) {
		DEBUG(LOG_ERR, "Unable to open %s for reading", file);
		return;
	}
	
	while (i < server_config.max_leases && (fread(&lease, sizeof lease, 1, fp) == 1)) {
		lease.expires = ntohl(lease.expires);
		/* Static leases have EXPIRES_NEVER, and don'rt have to be in 
		   the start .. end range. */
		if ((ntohl(lease.yiaddr) >= ntohl(server_config.start) && ntohl(lease.yiaddr) <= ntohl(server_config.end))
		    || lease.expires == EXPIRES_NEVER) { // Fix by honor
			DEBUG(LOG_DEBUG, "Reload %x", ntohl(lease.yiaddr));
			if(lease.expires != EXPIRES_NEVER && !server_config.remaining)
				lease.expires -= get_time(0);
			if (!(oldest = add_lease(lease.chaddr, lease.yiaddr, lease.expires))) {
				LOG(LOG_WARNING, "Too many leases while loading %s\n", file);
				break;
			}			
			strncpy(oldest->hostname, lease.hostname, sizeof(oldest->hostname) - 1);
			oldest->hostname[sizeof(oldest->hostname) - 1] = '\0';
			i++;
		}
		else
			DEBUG(LOG_DEBUG, "Skip %x", ntohl(lease.yiaddr));
	}
	DEBUG(LOG_INFO, "Read %d leases", i);
	fclose(fp);
}

int compare_leases(uint32_t requested_ip)
{
	FILE *fp;
        unsigned int i = 0;
        struct dhcpOfferedAddr lease, *oldest;
	int match = 0;

	/* Write leases table to file */
	system("killall -USR1 udhcpd");
	
	if (!(fp = fopen("/tmp/udhcpd.leases", "r"))) {
		DEBUG(LOG_ERR, "Unable to open /tmp/udhcpd.leases for reading");
		return;
	}
	
	while (i < 254 && (fread(&lease, sizeof lease, 1, fp) == 1)) {
		if(lease.yiaddr == requested_ip) {
			match = 1;
			break;
		}
		i++;
	}
	fclose(fp);
	return match;
}	
		
/* convert hex digits in hex to binary, ignore all non-hex digits like ':',
   return length of binary data 
*/
int unhex(u_int8_t *data, const char *hex, int maxlen) {
	int dst = 0, odd = 0;
	u_int8_t tmp = 0;
	while(*hex && dst < maxlen) {
		char ch = *hex;
		/* Half octet? */
		if(ch >= '0' && ch <='9') {
			tmp = (tmp<<4) | (ch - '0');
			odd++;
		} else if(ch >= 'a' && ch <='f') {
			tmp = (tmp<<4) | (ch - 'a' + 10);
			odd++;
		} else if(ch >= 'A' && ch <='F') {
			tmp = (tmp<<4) | (ch - 'A' + 10);
			odd++;
		}
		/* Full octet */
		if(odd == 2) {
			data[dst++] = tmp;
			odd = 0;
		}
		hex++;
	}
	return dst;
}

void read_statics(char *statics_file) {
	int x;
	FILE *fp;
	char line[80];
	struct in_addr in;
	int i=0;
      	/* read the static leases file
           <IP> <HW ADDR> <Hostname>
      	*/
	if ((fp = fopen(statics_file, "r"))!=NULL) {
 		/* remove all current static leases */
 		for (x = 0; x < (int)server_config.max_leases; x++) {
 			if(leases[x].expires == EXPIRES_NEVER) {
 				memset(&leases[x], 0, sizeof(struct dhcpOfferedAddr));//clear memory value
 			}
      		}
 		/* process the file */
		while(fgets(line,sizeof(line),fp)!=NULL){
			int col1,col2 = 0;
			u_int8_t chaddr[16];
			struct dhcpOfferedAddr *lease;
			
			/* Remove \n */
			for(x=0; line[x] && line[x]!='\n'; x++) ;
			line[x] = 0;
			
			/* Split into three columns */
			for(x=0; line[x] && !isspace(line[x]); x++) ;
			for(; line[x] && isspace(line[x]); x++) line[x] = 0;
			col1 = x;
			if(line[col1]) {
				for(x=col1; line[x] && !isspace(line[x]); x++) ;
				for(; line[x] && isspace(line[x]); x++) line[x] = 0;
				col2 = x;
			}
			if(line[col1] == 0 || line[col2] == 0) {
				LOG(LOG_DEBUG,"line %d [%s] from static leases file is invalid",i,line);
				i++;
				continue;
			}
			inet_aton(line, &in);
			
			/* line		IP 
			   col1		hwaddr
			   col2		hostname
			*/
			memset(chaddr, 0, sizeof(chaddr));
			unhex(chaddr, &line[col1], sizeof(chaddr));
			/* removes all dynamic leases on those IPs/MACs as well */
			lease = add_lease(chaddr, (u_int32_t)in.s_addr, EXPIRES_NEVER);
			if(lease) {
				LOG(LOG_DEBUG,"line %d from static leases file: added %s %s %s",i,line,&line[col1],&line[col2]);
				/* Finally, copy hostname */
				strncpy(lease->hostname, &line[col2], sizeof(lease->hostname));
				lease->hostname[sizeof(lease->hostname)-1] = '\0';
			}
			i++;
		}
	   	fclose(fp);
	}
}
