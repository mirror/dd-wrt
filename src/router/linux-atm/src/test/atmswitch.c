/*
 ** Copyright (C) 2004 Init-Sys (http://www.init-sys.com)
 ** Written by Eric Leblond <eleblond@alphalink.fr>
 **
 ** This program is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, version 2 of the License.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program; if not, write to the Free Software
 ** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */



#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <atm.h>

#define BUF_SIZE 10000

static fd_set pvc_set;
static int fds;
static int s[2];

static void usage(const char *name)
{
	fprintf(stderr,"usage: %s [-b] [-q QOS] [itf.]vpi.vci [itf.]vpi.vci\n",name);
	exit(1);
}

int main(int argc,char **argv)
{
	const char *name;
	const char *options_list = "bq:Vh";
	struct sockaddr_atmpvc addr;
	struct atm_qos qos[2];
	int background=0;
	int set_qos=0;
	int pvc;
	int option;

	name = argv[0];

	memset(qos,0,2*sizeof(*qos));

	/*parse options */
	while((option = getopt ( argc, argv, options_list)) != -1 ){
		switch (option){
			case 'b' :
				background=1;
				break;
			case 'q' :
				/* reading qos */
				if (text2qos(optarg,qos,0)<0){
					fprintf (stderr, "Invalid QOS\n");
					return 0;
				}
				if (text2qos(optarg,qos+1,0)<0){
					fprintf (stderr, "Strange !\n");
				}
				/* inert qos on receiver */
				qos[1].rxtp=qos[0].txtp;
				qos[1].txtp=qos[0].rxtp;
				set_qos=1;
				break;
			case 'V' :
				fprintf (stdout, "atmswitch (version 0.2)\n");
				return 1;
				break;
			case 'h' :
				usage(name);
				return 1;
				break;
		}
	}

	if ((argc - optind) != 2) {
		fprintf(stderr,"Invalid number of arguments\n");
		usage(name);
	}
	
	if (background) {
		pid_t pid;

		pid=fork();
		if (pid < 0) {
			fprintf(stderr,"Error detaching\n");
			exit(2);
		} else if (pid) 
			exit(0); // This is the parent

	}

	/* init pvc_set */
	FD_ZERO(&pvc_set);
	fds=0;
	/* init pvc socket */
	for (pvc=0;pvc<=1;pvc++){
		if ((s[pvc] = socket(PF_ATMPVC,SOCK_DGRAM,0)) < 0) {
			perror("socket");
			return 1;
		}
		memset(&addr,0,sizeof(addr));
		if ( set_qos == 0) {
			qos[pvc].aal = ATM_AAL5;
			qos[pvc].rxtp.traffic_class = ATM_UBR;
			qos[pvc].txtp.traffic_class = ATM_UBR;
			qos[pvc].txtp.pcr                = ATM_MAX_PCR;
			qos[pvc].rxtp.pcr                = ATM_MAX_PCR;
			qos[pvc].txtp.max_sdu            = 1524;
			qos[pvc].rxtp.max_sdu            = 1524;
		}

		if (text2atm(argv[optind+pvc],(struct sockaddr *) &addr,sizeof(addr),
					T2A_PVC | T2A_UNSPEC | T2A_WILDCARD) < 0) usage(name);
					if (setsockopt(s[pvc],SOL_ATM,SO_ATMQOS,qos+pvc,sizeof(qos[0])) < 0) {
						perror("setsockopt SO_ATMQOS");
						return 1;
					}
					if (connect(s[pvc],(struct sockaddr *) &addr,sizeof(addr)) < 0) {
						perror("connect");
						return 1;
					}
					/* add PVC socket fo fd_set */
					FD_SET(s[pvc],&pvc_set);
					if (s[pvc] >= fds) fds = s[pvc]+1;
	}
	/* infinite loop */
	while (1) {
		int ret;
		fd_set set;

		set=pvc_set;
		ret = select(fds,&set,NULL,NULL,NULL);
		if (ret < 0) { 
			if (errno != EINTR) perror("select");
			continue;
		}
		for (pvc=0;pvc<=1;pvc++){
			if (FD_ISSET(s[pvc],&set)){
				char buf[BUF_SIZE];
				int bufsiz=0;
				/* read s[pvc] */
				bufsiz=read(s[pvc],buf,BUF_SIZE);
				if (bufsiz > 0){
					/* write s[(pvc+1)%2] */
					write(s[(pvc+1)%2],buf,bufsiz);
				} else {
					perror("read");
				}
			}
		}
	}
}
