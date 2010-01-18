#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "../include/can.h"
#define MAXL 40

int main(void)
{
	int i=0, fd=0, ret=0, count=0;
	int msglen;
	char loop=0;
	char ch, transmission[MAXL+1], specialfile[MAXL+1]="/dev/can0", emptystring[MAXL+1]="", buf[MAXL+1];
	char remote[MAXL+1];
	struct canmsg_t message;

	printf("\nThis program allows you to send or receive Can messages.\n");
	printf("Please answer the following questions:\n\n");

	while ( (*transmission!='s') && (*transmission!='r') ) {
		printf("Would you like to send or receive a message?\n");
		printf("send: <s> | receive: <r> ");
		strcpy(transmission,emptystring);
		count=0;
		while ( (ch=getchar()) != '\n' )
			transmission[count++]=ch;
		transmission[count]='\0';
	}

	while ( *remote!='y' && *remote!='n' ) {
		printf("Should the message be configured for Remote Transmission Requests?\n");
		printf("yes: <y> | no: <n> ");
		strcpy(remote,emptystring);
		count=0;
		while ( (ch=getchar()) != '\n' )
			remote[count++]=ch;
		remote[count]='\0';
	}
	if (remote[0]=='y')
		message.flags = MSG_RTR;
	else
		message.flags = 0;
//	message.flags |= MSG_EXT;  hard code EXT for now

	if (transmission[0]=='s') {
			printf("From wich device file would you like to send the message?\n");
			printf(specialfile);
			*buf='\0';
			fgets(buf,MAXL,stdin);
			buf[strcspn(buf,"\n")]='\0';
			if(*buf)
				strncpy(specialfile,buf,MAXL);
			specialfile[MAXL]='\0';
			specialfile[MAXL]='\0';
		printf("Enter the Message ID ");
		scanf("%lx",&message.id);
		if(message.id>=(1<<11))
		  message.flags |= MSG_EXT;
		printf("Enter the Message Length ");
		scanf("%d",&msglen);
		message.length=msglen;
		for (i=0; i<message.length; i++) {
			int val;
			printf("Enter data byte [%d] ",i);
			scanf("%x",&val);
			message.data[i]=val;
		}
	}	
	if (*transmission=='r') {
			printf("At which device file would you like to receive the message?\n");
			printf(specialfile);
			*buf='\0';
			fgets(buf,MAXL,stdin);
			buf[strcspn(buf,"\n")]='\0';
			if(*buf)
				strncpy(specialfile,buf,MAXL);
			specialfile[MAXL]='\0';
		printf("Enter the Message ID ");
		scanf("%lx",&message.id);
		getchar();
	}

	fd=open(specialfile,O_RDWR);
	if (fd<0) {
		printf("Error opening %s\n",specialfile);
		return -1;
	}

	if (transmission[0]=='s') {
		printf("Press enter to send message\n");
		getchar();
		while (getchar() != '\n');
		ret=write(fd, &message, sizeof(struct canmsg_t));
		if (ret<0)
			printf("Error sending message from %s\n",specialfile);
		else
			printf("Message successfully sent from %s\n",specialfile);
	}

	if (*transmission=='r') {
		ioctl(fd,CONF_FILTER,message.id);
		printf("Press enter to receive message or <l>oop\n");
		loop = 'l';
		while ( loop == 'l') {
			loop = getchar(); 
			ret=read(fd, &message, sizeof(struct canmsg_t));
			if (ret<0)
				printf("Error receiving message on %s\n",
								specialfile);
			else {
				printf("Id	: %lx\n",message.id);
				printf("length	: %d\n",message.length);
				printf("flags	: 0x%02x\n", message.flags);
				#ifdef CAN_MSG_VERSION_2
				printf("time	: %lds %ldusec\n", message.timestamp.tv_sec,
					message.timestamp.tv_usec);
				#else /* CAN_MSG_VERSION_2 */
				printf("time	: %ld\n", message.timestamp);
				#endif /* CAN_MSG_VERSION_2 */
				for (i=0; i<message.length; i++)
					printf("data%d	: %02x\n",i,
							message.data[i]);
			}
		}
	}

	if (close(fd)) {
		printf("Error closing %s\n",specialfile);
		return -1;
	}

	return 0;
}	
