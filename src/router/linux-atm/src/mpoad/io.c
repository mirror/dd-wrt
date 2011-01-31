#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <time.h>      /* for time()     */
#include <fcntl.h>
#include <string.h>    /* for strerror() */
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/param.h> /* for OPEN_MAX   */
#if __GLIBC__ >= 2
#include <sys/poll.h>
#else /* ugly hack to make it compile on RH 4.2 - WA */
#include <syscall.h>
#include <linux/poll.h>
#define SYS_poll 168
_syscall3(int,poll,struct pollfd *,ufds,unsigned int,nfds,int,timeout);
#endif
#include <atm.h>
#include <linux/types.h>
#include <linux/atmioc.h>
#include <linux/atmmpc.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <atm.h>
#include <netinet/in.h>  /* for ntohl() */

#include "packets.h"
#include "k_interf.h"
#include "io.h"
#include "get_vars.h"

#ifdef BROKEN_POLL
#include "poll2select.h"
#endif

#ifndef OPEN_MAX  /* Fixme:  there's got to be a better way to fix this */
#define OPEN_MAX 256
#endif

#define POLL_TIMEOUT 5000    /* poll() timeout, 5 seconds */

#if 1
#define dprintf printf
#else
#define dprintf(format,args...)
#endif                                                                      

#if 0
#define ddprintf printf
#else
#define ddprintf(format,args...)
#endif                                                                      

extern struct mpc_control mpc_control; /* from main.c */

int keep_alive_sm_running = 0;

struct outgoing_shortcut {
        int      fd;
        uint32_t ipaddr;     /* in network byte order */
        int      state;      /* see io.h for states  */
};

static struct llc_snap_hdr llc_snap_mpoa_ctrl = {
        0xaa, 0xaa, 0x03,
	{0x00, 0x00, 0x5e},
	{0x00, 0x03}
};        

static time_t stay_alive;        /* Next Keep-Alive should come before we hit this time     */
static struct pollfd fds[OPEN_MAX];
static int first_empty;          /* first non-reserved slot in fds[]                        */
static int fds_used;             /* first non-occupied slot in fds[], => also # of used fds */
static short socket_type[OPEN_MAX]; /* type and state info for fds[], see "io.h" for types  */
static struct outgoing_shortcut ingress_shortcuts[OPEN_MAX]; /* array of shortcuts we made  */

static int update_ingress_entry(uint32_t *addr, int fd, int new_state);
static int msg_from_mps(int slot);
static int accept_conn(int slot);
static int add_shortcut(int slot, int type);
static int check_connection(int slot);
static int complete_connection(int slot);
static int create_shortcut(char *atm_addr,struct atm_qos qos);
static void wait_for_mps_ctrl_addr(void);
static int connect_to_MPS(void); 

void main_loop(int listen_socket)
{
        int i, changed_fds;
        int kernel_ok, mps_ok, new_ctrl, new_shortcut;
        int poll_timeout = POLL_TIMEOUT;
        time_t now, previous_now;
	for (i = 0; i < OPEN_MAX; i++)
                fds[i].fd = -1;

        fds[0].fd     = mpc_control.kernel_socket;     /* mpcd <--> kernel socket   */
        fds[0].events = POLLIN;
        socket_type[0]= KERNEL;

	if(!mpc_control.mps_ctrl_addr_set)             /* Can't do much without the MPS control ATM addr */
	        wait_for_mps_ctrl_addr();
	connect_to_MPS();
	
        fds[1].fd     = mpc_control.MPS_socket;        /* we opened this to MPS      */
        fds[1].events = POLLIN;
        socket_type[1]= (OUTGOING_CTRL | CONNECTED);
        fds[2].fd     = mpc_control.MPS_listen_socket; /* for incoming control calls */
        fds[2].events = POLLIN;
        socket_type[2]= LISTENING_CTRL;
        fds[3].fd     = listen_socket;     /* for incoming shortcuts     */
        fds[3].events = POLLIN;
        socket_type[3]= LISTENING_DATA;
        fds_used = first_empty = 4;
        now = previous_now = time(NULL);

        while (1) {
                kernel_ok = mps_ok = new_ctrl = new_shortcut = 1;
                fflush(stdout);
#ifdef BROKEN_POLL
                changed_fds = poll2select(fds, fds_used, poll_timeout);
#else
                changed_fds = poll(fds, fds_used, poll_timeout);
#endif
#if 0
                printf("\nio.c: main_loop() poll returns %d\n", changed_fds);
                for (i = 0; i < OPEN_MAX; i++) {
                        if (fds[i].fd < 0 ) continue;    /* slot not in use */
                        if ( fds[i].revents == 0) {
                                printf("check1: fd %d slot %d not changed\n", fds[i].fd, i);
                        }
                        else printf("check1: fd %d slot %d     changed\n", fds[i].fd, i);
                }
#endif

                switch(changed_fds) {
                case -1:
                        printf("mpcd: io.c: main_loop: poll error: %s\n", strerror(errno));
			if(errno == EINTR) continue;
                        goto out; /* return to main() */
                        break; /* not reached */
                case 0:
			keep_alive_sm(0, -1);  /* (keepalive_liftime, seq_num) */
                        clear_expired(); /* id_list.c */
			poll_timeout = POLL_TIMEOUT;
                        previous_now = time(NULL);
                        continue;
                        break; /* not reached */
                }

                /* It was not a timeout. Adjust poll_timeout */
                now = time(NULL);
                poll_timeout -= now - previous_now;
                if (poll_timeout < 0) poll_timeout = 0;

                /* Since we are here something happened to the fds */
                if (fds[0].revents) {
                        dprintf("mpcd: io.c: main_loop() msg_from_kernel\n");
                        kernel_ok = msg_from_kernel(fds[0].fd);
                        changed_fds--;
                }
                if (fds[1].revents) {
                        ddprintf("mpcd: io.c: main_loop() msg_from_mps1\n");
                        mps_ok = msg_from_mps(1);
                        changed_fds--;
                }
                if (fds[2].revents) {
                        new_ctrl = accept_conn(2);
			changed_fds--;
			if( new_ctrl < 0 )
			        break;
			socket_type[new_ctrl] = INCOMING_CTRL | CONNECTED;
                        dprintf("mpcd: io.c main_loop() accepted INCOMING_CTRL slot %d\n", new_ctrl);
		}
                if (fds[3].revents) {
                        new_shortcut = accept_conn(3);
                        dprintf("mpcd: io.c main_loop() accepted INCOMING_SHORTCUT slot %d\n", new_shortcut);
			changed_fds--;
			if( new_shortcut < 0 )
			        break;
			socket_type[new_shortcut] = INCOMING_SHORTCUT;
                        if (add_shortcut(new_shortcut, MPC_SOCKET_EGRESS) < 0)
                                break;
		}

#if 0
                if (changed_fds == 0)  /* see if we can already go back to poll() */
                        continue;
#endif

                for (i = first_empty; i < fds_used; i++) {
                        if (fds[i].fd < 0 ) continue;    /* slot not in use */
                        if ( fds[i].revents == 0) {
                                ddprintf("fd %d slot %d not changed\n", fds[i].fd, i);
                                continue;
                        }
                        ddprintf("about to process fd %d slot %d\n", fds[i].fd, i);
                        if (socket_type[i] & INCOMING_CTRL) {
                                ddprintf("mpcd: io.c: main_loop() msg_from_mps2\n");
                                mps_ok = msg_from_mps(i);
                        }
                        else {
                                ddprintf("mpcd: io.c: main_loop() checking connection fd %d\n", fds[i].fd);
                                if (check_connection(i) < 0) {
                                        printf("mpcd: io.c: main_loop: check_connection returned < 0\n");
                                        break; /* this will cause break from while(1) too */
                                }
                        }
			if (--changed_fds == 0) break; /* no more changed fds, leave for() */
                }

                if (changed_fds != 0){
			printf("mpcd: changed_fds = %d\n", changed_fds);
                        /* break; */         /* leave while(1) */
		}
                if (kernel_ok && mps_ok >= 0 && new_ctrl >= 0 && new_shortcut >= 0)
		        continue; /* back to poll() */
                else break;       /* leave main_loop */
        }
        
 out:
        /* clean up, close the sockets */
        for (i = 0; i < fds_used; i++) {
                if (fds[i].fd < 0)
		        continue;
                close(fds[i].fd);
                socket_type[i] = NOT_USED;
        }
        printf("mpcd: io.c: exiting main_loop()\n");

        return;
}

/*
 * If MPS control ATM address is not given as an argument this func waits until
 * kernel has found one from a TLV in le_arp and tells us what it is.
 */
static void wait_for_mps_ctrl_addr(){
	while(!mpc_control.mps_ctrl_addr_set){
#ifdef BROKEN_POLL
                if(poll2select(fds, 1, -1))
#else
	        if(poll(fds, 1, -1))
#endif
		        msg_from_kernel(fds[0].fd);
	}
	return;
}



/* 
 * Sends a packet to MPS. Adds LLC/SNAP encapsulation
 * in the beginning of the buffer.
 */
int send_to_mps(char *buff, int length)
{
        char tmp[MAX_PACKET_LENGTH + sizeof(struct llc_snap_hdr)];
	int bytes_written;
        char *pos = tmp;
	if(mpc_control.MPS_socket<0){
	        connect_to_MPS();     
		fds[1].fd = mpc_control.MPS_socket;
		fds[1].events  = POLLIN;
		fds[1].revents = 0;
		socket_type[1] = (OUTGOING_CTRL | CONNECTED);
	}
        memcpy(pos, &llc_snap_mpoa_ctrl, sizeof(struct llc_snap_hdr));
	pos += sizeof(struct llc_snap_hdr);
	memcpy(pos, buff, length);
	length += sizeof(struct llc_snap_hdr);
        bytes_written = write(mpc_control.MPS_socket, tmp, length);
	while(bytes_written != 0){
		bytes_written = write(mpc_control.MPS_socket, tmp+bytes_written, length-bytes_written);
		if( bytes_written < 0 ){
			printf("mpcd: io.c: send_to_mps() write failed\n");
			return -1;
		}
	}
	return 1;
}

/*
 * Sends a control packet over a shortcut. Used in a dataplane purge.
 */
int send_to_dataplane(char *buff, int length, int shortcut_fd)
{
        char tmp[MAX_PACKET_LENGTH + sizeof(struct llc_snap_hdr)];
	int bytes_written;
        char *pos = tmp;
	memcpy(pos, &llc_snap_mpoa_ctrl, sizeof(struct llc_snap_hdr));
	pos += sizeof(struct llc_snap_hdr);
	memcpy(pos, buff, length);
	length += sizeof(struct llc_snap_hdr); 
        bytes_written = write(shortcut_fd, tmp, length);
	while(bytes_written != 0){
		bytes_written = write(shortcut_fd, tmp+bytes_written, length-bytes_written);
		if( bytes_written < 0 ){
			printf("mpcd: io.c: write to dataplane failed\n");
			return -1;
		}
	}
	return 1;
}

/*
 * Keep alive state machine. Sequence number less than 
 * and keep_alive_lifetime equal to zero is used
 * when checking wheter the MPS is still alive.
 *
 */
void keep_alive_sm(unsigned keep_alive_lifetime, int sequence_number){
        struct k_message msg;
	static unsigned previous_sequence_number = 0;
	static int start_keep_alive_sm = 0;
	time_t now = time(NULL);
	memset(&msg,0,sizeof(struct k_message));
	if(!keep_alive_sm_running){
	        start_keep_alive_sm = 0;
		return;
	}
	if(!start_keep_alive_sm){
	        dprintf("mpcd: io.c: starting keep_alive_sm.\n");
	        stay_alive = time(NULL) + MPC_C2;
		start_keep_alive_sm = 1;
		return;
	}
	if( now > stay_alive ){
	        dprintf("mpcd: io.c: MPS death!");
	        msg.type = MPS_DEATH;
		memcpy(msg.MPS_ctrl,mpc_control.MPS_CTRL_ATM_ADDR,ATM_ESA_LEN);
		send_to_kernel(&msg);
		previous_sequence_number = 0;
		stay_alive = now + MPC_C2; 
		return;
	}
	if( sequence_number < 0 )
	        return;
	if( sequence_number < previous_sequence_number ){
	         dprintf("mpcd: io.c: MPS death!");
	         msg.type = MPS_DEATH;
		 memcpy(msg.MPS_ctrl,mpc_control.MPS_CTRL_ATM_ADDR,ATM_ESA_LEN);
		 send_to_kernel(&msg);
		 previous_sequence_number = 0;
		 stay_alive = now + MPC_C2; 
		 return;
	}
	stay_alive = now + keep_alive_lifetime;
	previous_sequence_number = sequence_number;
	return;
}  

/*
 * Creates a socket, sets traffic and sap parameters
 * and binds the socket with given address.
 *
 * returns < 0 for error, socket for ok
 */
int get_socket(struct sockaddr_atmsvc *address)
{
        struct atm_qos qos;
        struct atm_sap sap;
        int socket_fd;

        socket_fd = socket(PF_ATMSVC, SOCK_DGRAM, 0);
        if( socket_fd < 0 ){
          printf("mpcd: io.c: socket creation failure: %s \n",strerror(errno));
          return -1;
        }
        memset(&qos, 0, sizeof(qos));
        memset(&sap, 0, sizeof(sap));
        qos.aal = ATM_AAL5;
        qos.txtp.traffic_class = ATM_UBR;
        qos.rxtp.traffic_class = ATM_UBR;
        qos.txtp.max_sdu = 1536;
        qos.rxtp.max_sdu = 1536;
        sap.blli[0].l2_proto = ATM_L2_ISO8802;
        if (setsockopt(socket_fd, SOL_ATM,SO_ATMQOS, &qos, sizeof(qos)) < 0){
                printf("mpcd: io.c: setsockopt SO_ATMQOS failed: %s \n",strerror(errno));
                close(socket_fd);                      
		return -1;
        }
        if (setsockopt(socket_fd,SOL_ATM,SO_ATMSAP,&sap,sizeof(sap)) < 0) {
                printf("mpcd: io.c: setsockop SO_ATMSAP failed\n");
                close (socket_fd);
                return -1;
        }
        if (address == NULL)
                return socket_fd;

        if (bind(socket_fd, (struct sockaddr *)address, sizeof(struct sockaddr_atmsvc)) < 0){
                printf("mpcd: io.c: bind  failure: %s \n",strerror(errno));
                close(socket_fd);
                return -1;
        }
        return socket_fd;
}                          	

/*
 * Creates an ATM_ANYCLASS traffic class socket, sets traffic and sap
 * parameters and binds the socket with given address.
 *
 * returns < 0 for error, socket for ok
 */
int get_listen_socket(struct sockaddr_atmsvc *address)
{

        int s;
        struct atm_qos qos;

        s = get_socket(NULL);
        if (s < 0) {
                printf("mpcd: io.c: get_listen_socket() get socket failed\n");
                return s;
        }

        memset(&qos, 0, sizeof(qos));
        qos.aal = ATM_AAL5;
        qos.txtp.traffic_class = ATM_ANYCLASS;
        qos.rxtp.traffic_class = ATM_ANYCLASS;

        if (setsockopt(s, SOL_ATM,SO_ATMQOS, &qos, sizeof(qos)) < 0){
                printf("mpcd: io.c: get_listen_socket() setsockopt SO_ATMQOS: %s\n",
                       strerror(errno));
                close(s);                      
		return -1;
        }

        if (bind(s, (struct sockaddr *)address, sizeof(struct sockaddr_atmsvc)) < 0){
                printf("mpcd: io.c: get_listen_socket() bind: %s\n", strerror(errno));
                close(s);
                return -1;
        }
        
        if (listen(s, 5) < 0) {
                printf("mpcd: io.c: get_lilsten_socket() listen: %s\n", strerror(errno));
                close(s);
                return -1;
        }

        return s;
}

/*
 * If addr != NULL searches by addr. If addr == NULL searches by fd.
 * Returns ipaddr.
 *
 */
static int update_ingress_entry(uint32_t *addr, int fd, int new_state)
{
        int i;

        if (addr != NULL) {
                dprintf("mpcd: io.c update_ingress_entry() updating ip 0x%x\n", *addr);
                for (i = 0; i < OPEN_MAX; i++)
                        if (ingress_shortcuts[i].ipaddr == *addr)
                                break;
        }
        else {
                dprintf("mpcd: io.c update_ingress_entry() updating fd %d\n", fd);
                for (i = 0; i < OPEN_MAX; i++)
                        if (ingress_shortcuts[i].fd == fd)
                                break;
        }
        if (i == OPEN_MAX) {
                printf("mpcd: io.c: update_ingress_entry: entry not found\n");
                        return 0;
	}
        ingress_shortcuts[i].fd    = fd;
        ingress_shortcuts[i].state = new_state;
	if (new_state == INGRESS_NOT_USED)
                memset(&ingress_shortcuts[i], 0 , sizeof(ingress_shortcuts[i]));

        return ingress_shortcuts[i].ipaddr;
}

/*
 * returns < 0  for error
 *
 */
static int msg_from_mps(int slot)
{
        int bytes_read, fd;
        char buff[MAX_PACKET_LENGTH];

        fd = fds[slot].fd;
        bytes_read = read(fd, buff, sizeof(buff));
        if (bytes_read < 0) {
                printf("mpcd: io.c: read failed from MPS: %s\n", strerror(errno));
                close(fd);
                fds[slot].fd = -1;
                socket_type[slot] = NOT_USED;
                return -1;
        }
        if (bytes_read == 0) {
                dprintf("mpcd: io.c: EOF from MPS\n");
                close(fd);
                fds[slot].fd = -1;
                if (slot == 1)
		        mpc_control.MPS_socket = -1;
                socket_type[slot] = NOT_USED;
                return 1; /* See spec 4.6. Might be normal */
        }
  
        if ( memcmp(buff, &llc_snap_mpoa_ctrl, sizeof(llc_snap_mpoa_ctrl)) != 0 ) {
                printf("mpcd: io.c: msg_from_mps: MPS is pushing us garbage\n");
                return -1;
        }

        (void)recognize_packet(buff + sizeof(struct llc_snap_hdr));

	return 0;
}

/*
 * returns < 0 for error, slot of the new socket for ok
 *
 */
static int accept_conn(int slot)
{
        struct sockaddr_atmsvc sa;
        int i, new_fd;
        socklen_t sa_len;

        sa_len = sizeof(sa);
        new_fd = accept(fds[slot].fd, (struct sockaddr *)&sa, &sa_len);
        if (new_fd < 0) {
                printf("mpcd: io.c: accept_conn: %s\n", strerror(errno));
                return -1;
        }

        for (i = first_empty; i < OPEN_MAX; i++) {
                if (fds[i].fd >= 0)                /* slot in use ? */
                        continue;
                fds[i].fd      = new_fd;
                fds[i].events  = POLLIN;
                fds[i].revents = 0;
                break;
        }
        if (i == OPEN_MAX) {
                printf("mpcd: io.c: accept_conn: no room for new connection\n");
                return -1;
        }

        if (i >= fds_used)
                fds_used = i + 1;

        return i;
}

/*
 * returns < 0 for error, slot of the new socket for ok
 *
 */
static int add_shortcut(int slot, int type)
{
        struct atmmpc_ioc ioc_data;
        int ipaddr = 0;

        if (type == MPC_SOCKET_INGRESS)
                ipaddr = update_ingress_entry(NULL, fds[slot].fd, INGRESS_CONNECTED);

        ioc_data.dev_num = mpc_control.INTERFACE_NUMBER;
        ioc_data.ipaddr  = ipaddr;
        ioc_data.type    = type;
        if (ioctl(fds[slot].fd, ATMMPC_DATA, &ioc_data) < 0) {
                printf("mpcd: io.c: add_shortcut: %s\n", strerror(errno));
                close(fds[slot].fd);
                fds[slot].fd = -1;
                socket_type[slot] = NOT_USED;
		return -1;
	}
	
        return slot;
}

/*
 * ECONNRESET == RST in TCP world. Check what equivalent
 * events can happen in ATM world.
 *
 * Returns < 0 for error
 */
static int check_connection(int slot)
{
        char buff[MAX_PACKET_LENGTH];
        struct k_message *msg;
        struct pollfd *pfd;
        int bytes_read;

        dprintf("mpcd: io.c: check_connection() event in fd %d, type %d\n", fds[slot].fd, socket_type[slot]);
        if (socket_type[slot] & CONNECTING) { /* connect() completed (maybe) */
                complete_connection(slot);    /* ignore return value */
                return 0;
        }

        pfd = &fds[slot];
        bytes_read = read(pfd->fd, buff, sizeof(buff));
        if (bytes_read < 0) {
                if (errno == ECONNRESET || errno == EPIPE) { /* conn reset by the other end or kernel (EPIPE) */
                        if (socket_type[slot] & OUTGOING_SHORTCUT)
                                update_ingress_entry(NULL, pfd->fd, INGRESS_NOT_USED);

                        close(pfd->fd);
                        pfd->fd = -1;
                        socket_type[slot] = NOT_USED;
                        return 1;
                }
                printf("mpcd: io.c: check_connection() bytes_read=%d, errno='%s'\n", bytes_read, strerror(errno)); 
                return -1;
        }
        if (bytes_read == 0) {            /* conn closed by the other end */
                if (socket_type[slot] & OUTGOING_SHORTCUT)
                        update_ingress_entry(NULL, pfd->fd, INGRESS_NOT_USED);

                dprintf("mpcd: io.c: check_connection() fd %d type %d; connection closed'\n", pfd->fd, socket_type[slot]); 
                close(pfd->fd);
                pfd->fd = -1;
                socket_type[slot] = NOT_USED;
                return 1;
        }

        /* See if this is a MPOA control packet */
        if ( memcmp(buff, &llc_snap_mpoa_ctrl, sizeof(llc_snap_mpoa_ctrl)) == 0 )
                if ( recognize_packet(buff + sizeof(llc_snap_mpoa_ctrl)) >= 0)
		        return 1;

        dprintf("mpcd: io.c check_connection(): msg from kernel\n");
	msg = (struct k_message *)buff;
	if(msg->type == DATA_PLANE_PURGE){
	        send_purge_request(msg->content.eg_info.mps_ip,32,
				   get_own_ip_addr(mpc_control.INTERFACE_NUMBER),pfd->fd);
		return 1;
	}

	printf("mpcd: io.c check_connection(): unknown msg %d from kernel, ignoring",
	       msg->type);
	
        return 1;
}

/*
 * returns < 0 for unsuccessful connect, fd for ok
 *
 */
static int complete_connection(int slot)
{
        int retval;
        struct sockaddr_atmsvc dummy;

        dprintf("mpcd: io.c: complete_connection() completing fd %d slot %d\n", fds[slot].fd, slot);
        /* this seems to be common method in Linux-ATM
         * making sure that nonblocking connect was
         * completed successfully
         */
        retval = connect(fds[slot].fd,(struct sockaddr *)&dummy, sizeof(dummy));
        if (retval < 0) {
                printf("mpcd: io.c: complete_connection(): '%s'\n", strerror(errno));
                socket_type[slot] = NOT_USED;
                update_ingress_entry(NULL, fds[slot].fd, INGRESS_NOT_USED);
                close(fds[slot].fd);
                fds[slot].fd = -1;
                fds[slot].revents = 0;
                return 0;
        }
        socket_type[slot] &= ~CONNECTING;
        socket_type[slot] |= CONNECTED;
        
        fds[slot].events  = POLLIN; /* We left POLLOUT accidentally in. Hope you never do the same */
        fds[slot].revents = 0;
        if(socket_type[slot] & OUTGOING_SHORTCUT)
                return add_shortcut(slot, MPC_SOCKET_INGRESS);
        return fds[slot].fd;
}

/*
 * Called if kernel wants us to create a shortcut
 */
void create_ingress_svc(uint32_t ipaddr, char *atm_addr, struct atm_qos qos)
{
	int i, new_socket;
	
        new_socket = create_shortcut(atm_addr,qos);
        if (new_socket < 0) {
                printf("mpcd: io.c: create_ingress_svc: create_shortcut failed\n");
                return;
	}
        
        for (i = first_empty; i < OPEN_MAX; i++) {
	        if (fds[i].fd >= 0)                /* slot in use ? */
                        continue;
                fds[i].fd      = new_socket;
                fds[i].events  = POLLIN | POLLOUT;
                fds[i].revents = 0;
                break;
        }
        if (i == OPEN_MAX) {
                printf("mpcd: io.c: create_ingress_svc: create_shortcut: no room for new connection\n");
                return;
        }

        socket_type[i] = (OUTGOING_SHORTCUT | CONNECTING);
        if (i >= fds_used)
                fds_used = i + 1;

        /* Store the IP address we are creating this shortcut for */
        dprintf("mpcd: io.c: create_ingress_svc: adding ip 0x%x\n", ipaddr);
        for(i = 0; i < OPEN_MAX; i++)
                if (ingress_shortcuts[i].state == INGRESS_NOT_USED)
		        break;

        if (i == OPEN_MAX) {
                printf("mpcd: io.c: create_ingress_svc: ingress no more entries\n");
                return;
        }

        ingress_shortcuts[i].fd     = new_socket;
        ingress_shortcuts[i].ipaddr = ipaddr;
        ingress_shortcuts[i].state  = INGRESS_CONNECTING;
}

/*
 * returns < 0 for error, socket for ok
 *
 */
static int create_shortcut(char *atm_addr, struct atm_qos qos)
{
        int s, flags, retval;
        struct sockaddr_atmsvc addr;

        dprintf("mpcd: io.c: create_shortcut() addr = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
                atm_addr[0], atm_addr[1], atm_addr[2], atm_addr[3], atm_addr[4]);
        memset(&addr, 0, sizeof(addr));
        addr.sas_family = AF_ATMSVC;
        memcpy(addr.sas_addr.prv, atm_addr, ATM_ESA_LEN);
	s = get_socket(&mpc_control.data_listen_addr);
	if(qos.txtp.traffic_class > ATM_UBR || qos.rxtp.traffic_class > ATM_UBR){
	        printf("mpcd: io.c: create_shortcut() setting qos params (cbr)\n");
	        if (setsockopt(s, SOL_ATM,SO_ATMQOS, &qos, sizeof(qos)) < 0){
		        printf("mpcd: io.c: setsockopt SO_ATMQOS failed: %s \n",strerror(errno));
			close(s);
			return -1;
		}
	}
        dprintf("mpcd: create_shortcut() got fd %d \n", s);
        if ( (flags = fcntl(s, F_GETFL)) < 0) {
                printf("mpcd: io.c: fcntl(F_GETFL) failed: %s\n", strerror(errno));
                close(s);
                return -1;
        }
        if (fcntl(s, F_SETFL, flags | O_NONBLOCK) < 0) {
                printf("mpcd: io.c: fcntl(F_SETFL) failed: %s\n", strerror(errno));
                close(s);
                return -1;
        }
        retval = connect(s, (struct sockaddr *)&addr, sizeof(addr));
        if (retval < 0 && errno != EINPROGRESS) {
                printf("mpcd: io.c: create_shortcut: connect failed: %s\n", strerror(errno));
                return -1;
        }
	
        return s;
}

/*
 * Creates an active connection to MPS
 *
 * returns < 0 for error
 */
static int connect_to_MPS(){ 

        int c;
	struct sockaddr_atmsvc mps_ctrl_addr;
	struct sockaddr_atmsvc ctrl_listen_addr;
	memset(&mps_ctrl_addr,0,sizeof(struct sockaddr_atmsvc));
	memset(&ctrl_listen_addr,0,sizeof(struct sockaddr_atmsvc));
	memcpy(mps_ctrl_addr.sas_addr.prv,mpc_control.MPS_CTRL_ATM_ADDR,ATM_ESA_LEN);
	memcpy(ctrl_listen_addr.sas_addr.prv,mpc_control.OWN_ATM_ADDRESS,ATM_ESA_LEN);
	mps_ctrl_addr.sas_family = AF_ATMSVC;
	ctrl_listen_addr.sas_family = AF_ATMSVC;
        mpc_control.MPS_socket = get_socket(&ctrl_listen_addr);
        if (mpc_control.MPS_socket < 0)
                return -1;
	c = connect(mpc_control.MPS_socket, (struct sockaddr *)&(mps_ctrl_addr), 
		    sizeof(struct sockaddr_atmsvc));
        if( c < 0 ){
		printf("mpcd: io.c: connect to MPS failed: %s \n",strerror(errno));
		close(mpc_control.MPS_socket);
                return -1;  
        }
	return 0;
}
