/* parprouted: ProxyARP routing daemon. 
 * (C) 2004 Vladimir Ivaschenko <vi@maks.net>
 *
 * This application is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */
 
#include "parprouted.h"

char *progname;
int debug=0;
int option_arpperm=0;
static int perform_shutdown=0;

char *errstr;

pthread_t my_threads[MAX_IFACES+1];
int last_thread_idx=-1;

char * ifaces[MAX_IFACES];
int last_iface_idx=-1;

ARPTAB_ENTRY **arptab;
pthread_mutex_t arptab_mutex;

ARPTAB_ENTRY * replace_entry(struct in_addr ipaddr) 
{
    ARPTAB_ENTRY * cur_entry=*arptab;
    ARPTAB_ENTRY * prev_entry=NULL;
        
    while (cur_entry != NULL && ipaddr.s_addr != cur_entry->ipaddr_ia.s_addr) {
	prev_entry = cur_entry;
	cur_entry = cur_entry->next;
    };

    if (cur_entry == NULL) {
	if ((cur_entry = (ARPTAB_ENTRY *) malloc(sizeof(ARPTAB_ENTRY))) == NULL) {
	    errstr = strerror(errno);
	    syslog(LOG_INFO, "No memory: %s", errstr);
	} else {
	    if (prev_entry == NULL) { *arptab=cur_entry; }
	    else { prev_entry->next = cur_entry; }
	    cur_entry->next = NULL;
	    cur_entry->ifname[0] = '\0';
	    cur_entry->route_added=0;
	}
    }
    
    return cur_entry;	
}

int findentry(struct in_addr ipaddr)
{
    ARPTAB_ENTRY * cur_entry=*arptab;
    ARPTAB_ENTRY * prev_entry=NULL;
        
    while (cur_entry != NULL && ipaddr.s_addr != cur_entry->ipaddr_ia.s_addr) {
	prev_entry = cur_entry;
	cur_entry = cur_entry->next;
    };
    
    if (cur_entry == NULL)
	return 0;
    else
	return 1;
}

void processarp(int cleanup) 
{
    ARPTAB_ENTRY *cur_entry=*arptab, *prev_entry=NULL;
    char routecmd_str[ROUTE_CMD_LEN];

    while (cur_entry != NULL) {

	if (cur_entry->tstamp - time(NULL) <= ARP_TABLE_ENTRY_TIMEOUT 
	    && !cur_entry->route_added 
	    && !cur_entry->incomplete
	    && !cleanup) 
	{

	    /* added route to the kernel */
	    if (snprintf(routecmd_str, ROUTE_CMD_LEN-1, 
		     "/sbin/ip route add %s/32 metric 50 dev %s scope link",
		     inet_ntoa(cur_entry->ipaddr_ia), cur_entry->ifname) > ROUTE_CMD_LEN-1) 
	    {
		syslog(LOG_INFO, "ip route command too large to fit in buffer!");
	    } else {
		if (system(routecmd_str) != 0)
		    { syslog(LOG_INFO, "'%s' unsuccessful!", routecmd_str); }
	    }

	    cur_entry->route_added = 1;
	    cur_entry = cur_entry->next;

	} else if (!cur_entry->incomplete && (
		     cur_entry->tstamp - time(NULL) > ARP_TABLE_ENTRY_TIMEOUT 
		     || cleanup
		    )) {

	    /* remove entry from arp table and remove route from kernel */
	    if (snprintf(routecmd_str, ROUTE_CMD_LEN-1, 
		     "/sbin/ip route del %s/32 metric 50 dev %s scope link",
		     inet_ntoa(cur_entry->ipaddr_ia), cur_entry->ifname) > ROUTE_CMD_LEN-1) 
	    {
		syslog(LOG_INFO, "ip route command too large to fit in buffer!");
	    } else {
		if (system(routecmd_str) != 0)
		    syslog(LOG_INFO, "'%s' unsuccessful!", routecmd_str);
	    }

	    if (prev_entry != NULL) {
		prev_entry->next = cur_entry->next;
		free(cur_entry);
		cur_entry=prev_entry->next;
	    } else {
		*arptab = cur_entry->next;
		free(cur_entry);
		cur_entry=*arptab;
	    }
		
	} else {
	    cur_entry = cur_entry->next;
	} /* if */

    } /* while loop */

}	

void parseproc()
{
    FILE *arpf;
    int firstline;
    ARPTAB_ENTRY *entry;
    char line[ARP_LINE_LEN];
    char *item;
    struct in_addr ipaddr;
    int incomplete=0;
    int i;
    
    /* Parse /proc/net/arp table */
        
    if ((arpf = fopen(PROC_ARP, "r")) == NULL) {
	errstr = strerror(errno);
	syslog(LOG_INFO, "Error during ARP table open: %s", errstr);
    }

    firstline=1;

    while (!feof(arpf)) {
	
	if (fgets(line, ARP_LINE_LEN, arpf) == NULL) {
	    if (!ferror(arpf))
		break;
	    else {
    		errstr = strerror(errno);
		syslog(LOG_INFO, "Error during ARP table open: %s", errstr);
	    }
	} else {
	    if (firstline) { firstline=0; continue; }

	    incomplete=0;
	    	    
	    /* Incomplete ARP entries with MAC 00:00:00:00:00:00 */
	    if (strstr(line, "00:00:00:00:00:00") != NULL) 
		incomplete=1;
		
	    /* Incomplete entries having flag 0x0 */
	    if (strstr(line, "0x0") != NULL)
		incomplete=1;
	    
	    item=strtok(line, " ");

	    if ((inet_aton(item, &ipaddr)) == -1)
		    syslog(LOG_INFO, "Error parsing IP address %s", item);
	    
	    /* if IP address is marked as undiscovered and does not exist in arptab,
	       send ARP request to all ifaces */

	    if (incomplete &! findentry(ipaddr) ) {
		for (i=0; i <= last_iface_idx; i++)
		    arp_req(ifaces[i], ipaddr);
	    }

	    entry=replace_entry(ipaddr);
	    entry->incomplete = incomplete;
	    
	    entry->ipaddr_ia.s_addr = ipaddr.s_addr;
	    
	    /* Hardware type */
	    item=strtok(NULL, " "); 
	    
	    /* flags */
	    item=strtok(NULL, " "); 

	    /* MAC address */	    
	    item=strtok(NULL, " ");

	    if (strlen(item) < ARP_TABLE_ENTRY_LEN)
		strncpy(entry->hwaddr, item, ARP_TABLE_ENTRY_LEN);
	    else 
		syslog(LOG_INFO, "Error during ARP table parsing");

	    /* Mask */
	    item=strtok(NULL, " "); 

	    /* Device */
	    item=strtok(NULL, " ");
	    if (item[strlen(item)-1] == '\n') { item[strlen(item)-1] = '\0'; }
	    if (strlen(item) < ARP_TABLE_ENTRY_LEN) {
		if (entry->route_added && !strcmp(item, entry->ifname))
		    /* Remove route from kernel if it already exists through
		       a different interface */
		    entry->tstamp=0;
		else 
		    strncpy(entry->ifname, item, ARP_TABLE_ENTRY_LEN);
	    } else {
		    syslog(LOG_INFO, "Error during ARP table parsing");
	    }

	    time(&entry->tstamp);
	    
	    if (debug &! entry->route_added && !incomplete) {
	        printf("add entry: IPAddr: '%s' HWAddr: '%s' Dev: '%s'\n", 
		    inet_ntoa(entry->ipaddr_ia), entry->hwaddr, entry->ifname);
	    }

	}
    }

    if (fclose(arpf)) {
	errstr = strerror(errno);
	syslog(LOG_INFO, "Error during ARP table open: %s", errstr);
    }
}

void cleanup() 
{
    /* FIXME: I think this is a wrong way to do it ... */
    
    syslog(LOG_INFO, "Received signal; cleaning up.");
    /*
    for (i=0; i <= last_thread_idx; i++) {
	pthread_cancel(my_threads[i]);
    }
    */
    pthread_mutex_trylock(&arptab_mutex);
    processarp(1);
    syslog(LOG_INFO, "Terminating.");
    exit(1);
}

void sighandler()
{
    /* FIXME: I think this is a wrong way to do it ... */
    perform_shutdown=1;
}

void *main_thread()
{
    time_t last_refresh;

    signal(SIGINT, sighandler);
    signal(SIGTERM, sighandler);
    signal(SIGHUP, sighandler);
    
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    pthread_cleanup_push(cleanup, NULL);
    while (1) {
	if (perform_shutdown) {
	    pthread_exit(0);
	}
	pthread_testcancel();
        pthread_mutex_lock(&arptab_mutex);
        parseproc();
        processarp(0);
	pthread_mutex_unlock(&arptab_mutex);
	usleep(SLEEPTIME);
	if (!option_arpperm && time(NULL)-last_refresh > REFRESHTIME) {
	    refresharp(*arptab);
	    time(&last_refresh);
	}
    }
    /* required since pthread_cleanup_* are implemented as macros */
    pthread_cleanup_pop(0);
}
    
int main (int argc, char **argv)
{
    pid_t child_pid;
    int i, help=1;
    
    progname = (char *) basename(argv[0]);
    
    for (i = 1; i < argc; i++) {
	if (!strcmp(argv[i],"-d")) { 
	    debug=1;
	    help=0;
	}
	else if (!strcmp(argv[i],"-p")) { 
	    option_arpperm=1;
	    help=0;
	}
	else if (!strcmp(argv[i],"-h") || !strcmp(argv[i],"--help")) {
	    break;
	}
	else {
	    last_iface_idx++;
	    ifaces[last_iface_idx]=argv[i];
	    help=0;
	}
    }

    if (help || last_iface_idx <= -1) {
	    printf("parprouted: proxy ARP routing daemon, version %s.\n", VERSION);
    	    printf("(C) 2002 Vladimir Ivaschenko <vi@maks.net>, GPL2 license.\n");
	    printf("Usage: parprouted [-d] [-p] interface [interface]\n");
	    exit(1);
    }

    if (!debug) {
        /* fork to go into the background */
        if ((child_pid = fork()) < 0) {
            fprintf(stderr, "could not fork(): %s", strerror(errno));
            exit(1);
        } else if (child_pid > 0) {
            /* fork was ok, wait for child to exit */
            if (waitpid(child_pid, NULL, 0) != child_pid) {
                perror(progname);
                exit(1);
            }
            /* and exit myself */
            exit(0);
        }
        /* and fork again to make sure we inherit all rights from init */
        if ((child_pid = fork()) < 0) {
            perror(progname);
            exit(1);
        } else if (child_pid > 0)
            exit(0);

        /* create our own session */
        setsid();

        /* close stdin/stdout/stderr */
        close(0);
        close(1);
        close(2);

    }

    openlog(progname, LOG_PID | LOG_CONS | LOG_PERROR, LOG_DAEMON);
    syslog(LOG_INFO, "Starting.");

    signal(SIGINT, sighandler);
    signal(SIGTERM, sighandler);
    signal(SIGHUP, sighandler);

    if ((arptab = (ARPTAB_ENTRY **) malloc(sizeof(ARPTAB_ENTRY **))) == NULL) {
	    errstr = strerror(errno);
	    syslog(LOG_INFO, "No memory: %s", errstr);
    }
    
    *arptab = NULL;

    pthread_mutex_init(&arptab_mutex, NULL);
    pthread_mutex_init(&req_queue_mutex, NULL);
    
    if (pthread_create(&my_threads[++last_thread_idx], NULL, main_thread, NULL)) {
	syslog(LOG_ERR, "Error creating main thread.");
	abort();
    }

    for (i=0; i <= last_iface_idx; i++) {
	if (pthread_create(&my_threads[++last_thread_idx], NULL, (void *) arp, (void *) ifaces[i])) {
	    syslog(LOG_ERR, "Error creating ARP thread for %s.",ifaces[i]);
	    abort();
	}
	if (debug) printf("Created ARP thread for %s.\n",ifaces[i]);
    }
        
    if (pthread_join(my_threads[0], NULL)) {
	syslog(LOG_ERR, "Error joining thread.");
	abort();
    }

    while (waitpid(-1, NULL, WNOHANG)) { }
    exit(1);     
}
