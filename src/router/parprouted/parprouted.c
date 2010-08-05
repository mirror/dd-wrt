/* parprouted: ProxyARP routing daemon. 
 * (C) 2008 Vladimir Ivaschenko <vi@maks.net>
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
int verbose=0;
int option_arpperm=0;
static int perform_shutdown=0;

char *errstr;

pthread_t my_threads[MAX_IFACES+1];
int last_thread_idx=-1;

char * ifaces[MAX_IFACES];
int last_iface_idx=-1;

ARPTAB_ENTRY **arptab;
pthread_mutex_t arptab_mutex;

ARPTAB_ENTRY * replace_entry(struct in_addr ipaddr, char *dev) 
{
    ARPTAB_ENTRY * cur_entry=*arptab;
    ARPTAB_ENTRY * prev_entry=NULL;

    while (cur_entry != NULL && ( ipaddr.s_addr != cur_entry->ipaddr_ia.s_addr || ( strncmp(cur_entry->ifname,dev,strlen(dev)) != 0 ) ) ) {
	prev_entry = cur_entry;
	cur_entry = cur_entry->next;
    };

    if (cur_entry == NULL) {
	if (debug) syslog(LOG_NOTICE,"Creating new arptab entry %s(%s)\n", inet_ntoa(ipaddr), dev);

	if ((cur_entry = (ARPTAB_ENTRY *) malloc(sizeof(ARPTAB_ENTRY))) == NULL) {
	    errstr = strerror(errno);
	    syslog(LOG_INFO, "No memory: %s", errstr);
	} else {
	    if (prev_entry == NULL) { *arptab=cur_entry; }
	    else { prev_entry->next = cur_entry; }
	    cur_entry->next = NULL;
	    cur_entry->ifname[0] = '\0';
	    cur_entry->route_added = 0;
	    cur_entry->want_route = 1;
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

/* Remove all entires in arptab where ipaddr is NOT on interface dev */
int remove_other_routes(struct in_addr ipaddr, const char* dev)
{
    ARPTAB_ENTRY * cur_entry;
    int removed = 0;
        
    for (cur_entry=*arptab; cur_entry != NULL; cur_entry = cur_entry->next) {
        if (ipaddr.s_addr == cur_entry->ipaddr_ia.s_addr && strcmp(dev, cur_entry->ifname) != 0)  {
            if (debug && cur_entry->want_route) syslog(LOG_NOTICE,"Marking entry %s(%s) for removal\n", inet_ntoa(ipaddr), cur_entry->ifname);
            cur_entry->want_route = 0;
            ++removed;
        }
    }
    return removed;
}


/* Remove route from kernel */
int route_remove(ARPTAB_ENTRY* cur_entry)
{
    char routecmd_str[ROUTE_CMD_LEN];
    int success = 1;
    
    if (snprintf(routecmd_str, ROUTE_CMD_LEN-1, 
	    "/usr/sbin/ip route del %s/32 metric 50 dev %s scope link",
	    inet_ntoa(cur_entry->ipaddr_ia), cur_entry->ifname) > ROUTE_CMD_LEN-1) 
    {
	syslog(LOG_INFO, "ip route command too large to fit in buffer!");
    } else {
	if (system(routecmd_str) != 0)
	{
	    syslog(LOG_INFO, "'%s' unsuccessful!", routecmd_str);
	    if (debug) syslog(LOG_NOTICE,"%s failed\n", routecmd_str);
	    success = 0;
	}
	else 
	{
	    if (debug) syslog(LOG_NOTICE,"%s success\n", routecmd_str);
	    success = 1;
	}
    }
    if (success)
	cur_entry->route_added = 0;
    
    return success;
}

/* Add route into kernel */
int route_add(ARPTAB_ENTRY* cur_entry)
{
    char routecmd_str[ROUTE_CMD_LEN];
    int success = 1;

    if (snprintf(routecmd_str, ROUTE_CMD_LEN-1, 
	    "/usr/sbin/ip route add %s/32 metric 50 dev %s scope link",
	    inet_ntoa(cur_entry->ipaddr_ia), cur_entry->ifname) > ROUTE_CMD_LEN-1) 
    {
	syslog(LOG_INFO, "ip route command too large to fit in buffer!");
    } else {
	if (system(routecmd_str) != 0)
	{ 
	    syslog(LOG_INFO, "'%s' unsuccessful, will try to remove!", routecmd_str); 
	    if (debug) syslog(LOG_NOTICE,"%s failed\n", routecmd_str);
	    route_remove(cur_entry);
	    success = 0;
	}
	else
	{
	    if (debug) syslog(LOG_NOTICE,"%s success\n", routecmd_str);
	    success = 1;
	}
    }
    if (success)
	cur_entry->route_added = 1;

    return success;
}


void processarp(int in_cleanup) 
{
    ARPTAB_ENTRY *cur_entry=*arptab, *prev_entry=NULL;

    /* First loop to remove unwanted routes */
    while (cur_entry != NULL) {
	if (debug && verbose) syslog(LOG_NOTICE,"Working on route %s(%s) tstamp %u want_route %d\n", inet_ntoa(cur_entry->ipaddr_ia), cur_entry->ifname, (int) cur_entry->tstamp, cur_entry->want_route);

	if ( !cur_entry->want_route
	    || time(NULL) - cur_entry->tstamp > ARP_TABLE_ENTRY_TIMEOUT 
	    || in_cleanup)  {
	    
	    if (cur_entry->route_added)
		route_remove(cur_entry);

	    /* remove from arp list */
	    if (debug) syslog(LOG_NOTICE,"Delete arp %s(%s)\n", inet_ntoa(cur_entry->ipaddr_ia), cur_entry->ifname);
		
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
    	    prev_entry = cur_entry;
	    cur_entry = cur_entry->next;
	}	
    } /* while loop */

    /* Now loop to add new routes */
    cur_entry=*arptab;
    while (cur_entry != NULL) {
	if (time(NULL) - cur_entry->tstamp <= ARP_TABLE_ENTRY_TIMEOUT 
		&& cur_entry->want_route
		&& !cur_entry->route_added 
		&& !in_cleanup) 
	{
	    /* add route to the kernel */
	    route_add(cur_entry);
	}
	cur_entry = cur_entry->next;
    } /* while loop */

}	

void parseproc()
{
    FILE *arpf;
    int firstline;
    ARPTAB_ENTRY *entry;
    char line[ARP_LINE_LEN];
    struct in_addr ipaddr;
    int incomplete=0;
    int i;
    char *ip, *mac, *dev, *hw, *flags, *mask;
    
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
	    if (debug && verbose) syslog(LOG_NOTICE,"read ARP line %s", line);

	    incomplete=0;
	    	    
	    /* Incomplete ARP entries with MAC 00:00:00:00:00:00 */
	    if (strstr(line, "00:00:00:00:00:00") != NULL) 
		incomplete=1;
		
	    /* Incomplete entries having flag 0x0 */
	    if (strstr(line, "0x0") != NULL)
		incomplete=1;
	    
	    ip=strtok(line, " ");

	    if ((inet_aton(ip, &ipaddr)) == -1)
		    syslog(LOG_INFO, "Error parsing IP address %s", ip);
			
	    /* if IP address is marked as undiscovered and does not exist in arptab,
	       send ARP request to all ifaces */

	    if (incomplete &! findentry(ipaddr) ) {
	    	if (debug)  syslog(LOG_NOTICE,"incomplete entry %s found, request on all interfaces\n", inet_ntoa(ipaddr));
		for (i=0; i <= last_iface_idx; i++)
		    arp_req(ifaces[i], ipaddr, 0);
	    }

	    /* Hardware type */
	    hw=strtok(NULL, " "); 
	    
	    /* flags */
	    flags=strtok(NULL, " "); 

	    /* MAC address */	    
	    mac=strtok(NULL, " ");

	    /* Mask */
	    mask=strtok(NULL, " "); 

	    /* Device */
	    dev=strtok(NULL, " ");

	    if (dev[strlen(dev)-1] == '\n') { dev[strlen(dev)-1] = '\0'; }

	    entry=replace_entry(ipaddr, dev);
	    
	    if (entry->incomplete != incomplete && debug)
	    	syslog(LOG_NOTICE,"change entry %s(%s) to incomplete=%d\n", ip, dev, incomplete);

	    entry->ipaddr_ia.s_addr = ipaddr.s_addr;
	    entry->incomplete       = incomplete;
	    
	    if (strlen(mac) < ARP_TABLE_ENTRY_LEN)
		strncpy(entry->hwaddr, mac, ARP_TABLE_ENTRY_LEN);
	    else 
		syslog(LOG_INFO, "Error during ARP table parsing");
	    
	    if (strlen(dev) < ARP_TABLE_ENTRY_LEN) 
		strncpy(entry->ifname, dev, ARP_TABLE_ENTRY_LEN);
	    else
		syslog(LOG_INFO, "Error during ARP table parsing");

	    /* do not add routes for incomplete entries */
	    if (debug && entry->want_route != !incomplete) 
		syslog(LOG_NOTICE,"%s(%s): set want_route %d\n", inet_ntoa(entry->ipaddr_ia), entry->ifname, !incomplete);
	    entry->want_route   = !incomplete; 

            /* Remove route from kernel if it already exists through
               a different interface */
            if (entry->want_route)
            {
                if (remove_other_routes(entry->ipaddr_ia, entry->ifname) > 0)
                    if (debug) syslog(LOG_NOTICE,"Found ARP entry %s(%s), removed entries via other interfaces\n", inet_ntoa(entry->ipaddr_ia), entry->ifname);
            }

	    time(&entry->tstamp);
	    
	    if (debug && !entry->route_added && entry->want_route) {
	        syslog(LOG_NOTICE,"arptab entry: '%s' HWAddr: '%s' Dev: '%s' route_added:%d want_route:%d\n", 
		    inet_ntoa(entry->ipaddr_ia), entry->hwaddr, entry->ifname, entry->route_added, entry->want_route);
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
	    pthread_mutex_lock(&arptab_mutex);
	    refresharp(*arptab);
	    pthread_mutex_unlock(&arptab_mutex);
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
    	    printf("(C) 2007 Vladimir Ivaschenko <vi@maks.net>, GPL2 license.\n");
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
	if (debug) syslog(LOG_NOTICE,"Created ARP thread for %s.\n",ifaces[i]);
    }
        
    if (pthread_join(my_threads[0], NULL)) {
	syslog(LOG_ERR, "Error joining thread.");
	abort();
    }

    while (waitpid(-1, NULL, WNOHANG)) { }
    exit(1);     
}
