#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "../include/can.h"
#define MAXL 40

int main(void)
{
	int i=0, fd=0, ret=0;
	unsigned long bits;
	int msglen;
	char specialfile[MAXL+1]="/dev/can0", buf[MAXL+1];
	struct canmsg_t message;

	printf("\nThis program allows you to send  a stream of Can messages.\n");
	printf("Please answer the following questions:\n\n");

//		message.flags = 0;
	message.flags |= MSG_EXT;  //hard code EXT for now

	printf("From wich device file would you like to send the message?\n");
	printf(specialfile);
	*buf='\0';
	fgets(buf,MAXL,stdin);
	buf[strcspn(buf,"\n")]='\0';
	if(*buf)
		strncpy(specialfile,buf,MAXL);
	specialfile[MAXL]='\0';
	printf("Enter the starting Message ID ");
	scanf("%lx",&message.id);
	printf("Enter the Message Length ");
	scanf("%d",&msglen);
	message.length=msglen;
	for (i=0; i<message.length; i++) {
		int val;
		printf("Enter data byte [%d] ",i);
		scanf("%x",&val);
		message.data[i]=val;
	}

	fd=open(specialfile,O_RDWR);
	if (fd<0) {
		printf("Error opening %s\n",specialfile);
		return -1;
	}

	bits=0;
	while(1) {
		message.flags = MSG_EXT;
		message.id++;
		message.length %= 8;
		message.length++;
		bits += 8*(4+message.length);
		memcpy(message.data,&bits,sizeof(bits));
		ret=write(fd, &message, sizeof(struct canmsg_t));
		if (ret<0)
			printf("Error sending message from %s\n",specialfile);
	}

	if (close(fd)) {
		printf("Error closing %s\n",specialfile);
		return -1;
	}

	return 0;
}	
