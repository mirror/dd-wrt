/********************************************************************************************
 *
 * CHECK_NRPE.C - NRPE Plugin For Nagios
 * Copyright (c) 1999-2008 Ethan Galstad (nagios@nagios.org)
 * License: GPL
 *
 * Last Modified: 03-10-2008
 *
 * Command line: CHECK_NRPE -H <host_address> [-p port] [-c command] [-to to_sec]
 *
 * Description:
 *
 * This plugin will attempt to connect to the NRPE daemon on the specified server and port.
 * The daemon will attempt to run the command defined as [command].  Program output and
 * return code are sent back from the daemon and displayed as this plugin's own output and
 * return code.
 *
 ********************************************************************************************/

#include "../include/common.h"
#include "../include/config.h"
#include "../include/utils.h"


#define DEFAULT_NRPE_COMMAND	"_NRPE_CHECK"  /* check version of NRPE daemon */

int server_port=DEFAULT_SERVER_PORT;
char *server_name=NULL;
char *command_name=NULL;
int socket_timeout=DEFAULT_SOCKET_TIMEOUT;
int timeout_return_code=STATE_CRITICAL;
int sd;

char query[MAX_INPUT_BUFFER]="";

int show_help=FALSE;
int show_license=FALSE;
int show_version=FALSE;

#ifdef HAVE_SSL
SSL_METHOD *meth;
SSL_CTX *ctx;
SSL *ssl;
int use_ssl=TRUE;
#else
int use_ssl=FALSE;
#endif


int process_arguments(int,char **);
void alarm_handler(int);
int graceful_close(int,int);




int main(int argc, char **argv){
        u_int32_t packet_crc32;
        u_int32_t calculated_crc32;
	int16_t result;
	int rc;
	packet send_packet;
	packet receive_packet;
	int bytes_to_send;
	int bytes_to_recv;

	result=process_arguments(argc,argv);

        if(result!=OK || show_help==TRUE || show_license==TRUE || show_version==TRUE){

		if(result!=OK)
			printf("Incorrect command line arguments supplied\n");
                printf("\n");
		printf("NRPE Plugin for Nagios\n");
		printf("Copyright (c) 1999-2008 Ethan Galstad (nagios@nagios.org)\n");
		printf("Version: %s\n",PROGRAM_VERSION);
		printf("Last Modified: %s\n",MODIFICATION_DATE);
		printf("License: GPL v2 with exemptions (-l for more info)\n");
#ifdef HAVE_SSL
		printf("SSL/TLS Available: Anonymous DH Mode, OpenSSL 0.9.6 or higher required\n");
#endif
		printf("\n");
	        }

	if(result!=OK || show_help==TRUE){

		printf("Usage: check_nrpe -H <host> [-n] [-u] [-p <port>] [-t <timeout>] [-c <command>] [-a <arglist...>]\n");
		printf("\n");
		printf("Options:\n");
		printf(" -n         = Do no use SSL\n");
		printf(" -u         = Make socket timeouts return an UNKNOWN state instead of CRITICAL\n");
		printf(" <host>     = The address of the host running the NRPE daemon\n");
		printf(" [port]     = The port on which the daemon is running (default=%d)\n",DEFAULT_SERVER_PORT);
		printf(" [timeout]  = Number of seconds before connection times out (default=%d)\n",DEFAULT_SOCKET_TIMEOUT);
		printf(" [command]  = The name of the command that the remote daemon should run\n");
		printf(" [arglist]  = Optional arguments that should be passed to the command.  Multiple\n");
		printf("              arguments should be separated by a space.  If provided, this must be\n");
		printf("              the last option supplied on the command line.\n");
		printf("\n");
		printf("Note:\n");
		printf("This plugin requires that you have the NRPE daemon running on the remote host.\n");
		printf("You must also have configured the daemon to associate a specific plugin command\n");
		printf("with the [command] option you are specifying here.  Upon receipt of the\n");
		printf("[command] argument, the NRPE daemon will run the appropriate plugin command and\n");
		printf("send the plugin output and return code back to *this* plugin.  This allows you\n");
		printf("to execute plugins on remote hosts and 'fake' the results to make Nagios think\n");
		printf("the plugin is being run locally.\n");
		printf("\n");
	        }

	if(show_license==TRUE)
		display_license();

        if(result!=OK || show_help==TRUE || show_license==TRUE || show_version==TRUE)
		exit(STATE_UNKNOWN);


        /* generate the CRC 32 table */
        generate_crc32_table();

#ifdef HAVE_SSL
	/* initialize SSL */
	if(use_ssl==TRUE){
		SSL_library_init();
		SSLeay_add_ssl_algorithms();
		meth=SSLv23_client_method();
		SSL_load_error_strings();
		if((ctx=SSL_CTX_new(meth))==NULL){
			printf("CHECK_NRPE: Error - could not create SSL context.\n");
			exit(STATE_CRITICAL);
		        }

		/* ADDED 01/19/2004 */
		/* use only TLSv1 protocol */
		SSL_CTX_set_options(ctx,SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
                }
#endif

	/* initialize alarm signal handling */
	signal(SIGALRM,alarm_handler);

	/* set socket timeout */
	alarm(socket_timeout);

	/* try to connect to the host at the given port number */
	result=my_tcp_connect(server_name,server_port,&sd);

#ifdef HAVE_SSL
	/* do SSL handshake */
	if(result==STATE_OK && use_ssl==TRUE){
		if((ssl=SSL_new(ctx))!=NULL){
			SSL_CTX_set_cipher_list(ctx,"ADH");
			SSL_set_fd(ssl,sd);
			if((rc=SSL_connect(ssl))!=1){
				printf("CHECK_NRPE: Error - Could not complete SSL handshake.\n");
#ifdef DEBUG
				printf("SSL_connect=%d\n",rc);
				/*
				rc=SSL_get_error(ssl,rc);
				printf("SSL_get_error=%d\n",rc);
				printf("ERR_get_error=%lu\n",ERR_get_error());
				printf("%s\n",ERR_error_string(rc,NULL));
				*/
				ERR_print_errors_fp(stdout);
#endif
				result=STATE_CRITICAL;
			        }
		        }
		else{
			printf("CHECK_NRPE: Error - Could not create SSL connection structure.\n");
			result=STATE_CRITICAL;
		        }

		/* bail if we had errors */
		if(result!=STATE_OK){
			SSL_CTX_free(ctx);
			close(sd);
			exit(result);
		        }
	        }
#endif

	/* we're connected and ready to go */
	if(result==STATE_OK){

		/* clear the packet buffer */
		bzero(&send_packet,sizeof(send_packet));

		/* fill the packet with semi-random data */
		randomize_buffer((char *)&send_packet,sizeof(send_packet));

		/* initialize packet data */
		send_packet.packet_version=(int16_t)htons(NRPE_PACKET_VERSION_2);
		send_packet.packet_type=(int16_t)htons(QUERY_PACKET);
		strncpy(&send_packet.buffer[0],query,MAX_PACKETBUFFER_LENGTH);
		send_packet.buffer[MAX_PACKETBUFFER_LENGTH-1]='\x0';

		/* calculate the crc 32 value of the packet */
		send_packet.crc32_value=(u_int32_t)0L;
		calculated_crc32=calculate_crc32((char *)&send_packet,sizeof(send_packet));
		send_packet.crc32_value=(u_int32_t)htonl(calculated_crc32);


		/***** ENCRYPT REQUEST *****/


		/* send the packet */
		bytes_to_send=sizeof(send_packet);
		if(use_ssl==FALSE)
			rc=sendall(sd,(char *)&send_packet,&bytes_to_send);
#ifdef HAVE_SSL
		else{
			rc=SSL_write(ssl,&send_packet,bytes_to_send);
			if(rc<0)
				rc=-1;
		        }
#endif
		if(rc==-1){
			printf("CHECK_NRPE: Error sending query to host.\n");
			close(sd);
			return STATE_UNKNOWN;
		        }

		/* wait for the response packet */
		bytes_to_recv=sizeof(receive_packet);
		if(use_ssl==FALSE)
			rc=recvall(sd,(char *)&receive_packet,&bytes_to_recv,socket_timeout);
#ifdef HAVE_SSL
		else
			rc=SSL_read(ssl,&receive_packet,bytes_to_recv);
#endif

		/* reset timeout */
		alarm(0);

		/* close the connection */
#ifdef HAVE_SSL
		if(use_ssl==TRUE){
			SSL_shutdown(ssl);
			SSL_free(ssl);
			SSL_CTX_free(ctx);
	                }
#endif
		graceful_close(sd,1000);

		/* recv() error */
		if(rc<0){
			printf("CHECK_NRPE: Error receiving data from daemon.\n");
			return STATE_UNKNOWN;
		        }

		/* server disconnected */
		else if(rc==0){
			printf("CHECK_NRPE: Received 0 bytes from daemon.  Check the remote server logs for error messages.\n");
			return STATE_UNKNOWN;
		        }

		/* receive underflow */
		else if(bytes_to_recv<sizeof(receive_packet)){
			printf("CHECK_NRPE: Receive underflow - only %d bytes received (%d expected).\n",bytes_to_recv,sizeof(receive_packet));
			return STATE_UNKNOWN;
		        }

		
		/***** DECRYPT RESPONSE *****/


		/* check the crc 32 value */
		packet_crc32=ntohl(receive_packet.crc32_value);
		receive_packet.crc32_value=0L;
		calculated_crc32=calculate_crc32((char *)&receive_packet,sizeof(receive_packet));
		if(packet_crc32!=calculated_crc32){
			printf("CHECK_NRPE: Response packet had invalid CRC32.\n");
			close(sd);
			return STATE_UNKNOWN;
                        }
	
		/* check packet version */
		if(ntohs(receive_packet.packet_version)!=NRPE_PACKET_VERSION_2){
			printf("CHECK_NRPE: Invalid packet version received from server.\n");
			close(sd);
			return STATE_UNKNOWN;
			}

		/* check packet type */
		if(ntohs(receive_packet.packet_type)!=RESPONSE_PACKET){
			printf("CHECK_NRPE: Invalid packet type received from server.\n");
			close(sd);
			return STATE_UNKNOWN;
			}

		/* get the return code from the remote plugin */
		result=(int16_t)ntohs(receive_packet.result_code);

		/* print the output returned by the daemon */
		receive_packet.buffer[MAX_PACKETBUFFER_LENGTH-1]='\x0';
		if(!strcmp(receive_packet.buffer,""))
			printf("CHECK_NRPE: No output returned from daemon.\n");
		else
			printf("%s\n",receive_packet.buffer);
	        }

	/* reset the alarm */
	else
		alarm(0);

	return result;
        }



/* process command line arguments */
int process_arguments(int argc, char **argv){
	char optchars[MAX_INPUT_BUFFER];
	int argindex=0;
	int c=1;
	int i=1;

#ifdef HAVE_GETOPT_LONG
	int option_index=0;
	static struct option long_options[]={
		{"host", required_argument, 0, 'H'},
		{"command", required_argument, 0, 'c'},
		{"args", required_argument, 0, 'a'},
		{"no-ssl", no_argument, 0, 'n'},
		{"unknown-timeout", no_argument, 0, 'u'},
		{"timeout", required_argument, 0, 't'},
		{"port", required_argument, 0, 'p'},
		{"help", no_argument, 0, 'h'},
		{"license", no_argument, 0, 'l'},
		{0, 0, 0, 0}
                };
#endif

	/* no options were supplied */
	if(argc<2)
		return ERROR;

	snprintf(optchars,MAX_INPUT_BUFFER,"H:c:a:t:p:nuhl");

	while(1){
#ifdef HAVE_GETOPT_LONG
		c=getopt_long(argc,argv,optchars,long_options,&option_index);
#else
		c=getopt(argc,argv,optchars);
#endif
		if(c==-1 || c==EOF)
			break;

		/* process all arguments */
		switch(c){

		case '?':
		case 'h':
			show_help=TRUE;
			break;
		case 'V':
			show_version=TRUE;
			break;
		case 'l':
			show_license=TRUE;
			break;
		case 't':
			socket_timeout=atoi(optarg);
			if(socket_timeout<=0)
				return ERROR;
			break;
		case 'p':
			server_port=atoi(optarg);
			if(server_port<=0)
				return ERROR;
			break;
		case 'H':
			server_name=strdup(optarg);
			break;
		case 'c':
			command_name=strdup(optarg);
			break;
		case 'a':
			argindex=optind;
			break;
		case 'n':
			use_ssl=FALSE;
			break;
		case 'u':
			timeout_return_code=STATE_UNKNOWN;
			break;
		default:
			return ERROR;
			break;
		        }
	        }

	/* determine (base) command query */
	snprintf(query,sizeof(query),"%s",(command_name==NULL)?DEFAULT_NRPE_COMMAND:command_name);
	query[sizeof(query)-1]='\x0';

	/* get the command args */
	if(argindex>0){

		for(c=argindex-1;c<argc;c++){

			i=sizeof(query)-strlen(query)-2;
			if(i<=0)
				break;

			strcat(query,"!");
			strncat(query,argv[c],i);
			query[sizeof(query)-1]='\x0';
		        }
	        }

	/* make sure required args were supplied */
	if(server_name==NULL && show_help==FALSE && show_version==FALSE  && show_license==FALSE)
		return ERROR;


	return OK;
        }



void alarm_handler(int sig){

	printf("CHECK_NRPE: Socket timeout after %d seconds.\n",socket_timeout);

	exit(timeout_return_code);
        }


/* submitted by Mark Plaksin 08/31/2006 */
int graceful_close(int sd, int timeout){
        fd_set in;
        struct timeval tv;
        char buf[1000];

	/* send FIN packet */
        shutdown(sd,SHUT_WR);  
        for(;;){

                FD_ZERO(&in);
                FD_SET(sd,&in);
                tv.tv_sec=timeout/1000;
                tv.tv_usec=(timeout % 1000)*1000;

		/* timeout or error */
                if(1!=select(sd+1,&in,NULL,NULL,&tv))
			break;   

		/* no more data (FIN or RST) */
                if(0>=recv(sd,buf,sizeof(buf),0))
			break;
		}

#ifdef HAVE_CLOSESOCKET
        closesocket(sd);
#else
	close(sd);
#endif

	return OK;
	}
