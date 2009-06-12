
// jtstflash.c, Andrea Michelotti amichelotti@atmel.com
// simple flash software.
// To compile it:
// gcc -DLINUX jtstflash.c -o jtstflash (linux)
// gcc jtstflash.c -o jtstflash (cygwin)


/*
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.

    * Neither the name of [original copyright holder] nor the names of
      its contributors may be used to endorse or promote products
      derived from this software without specific prior written
      permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PRG_FLASH 0x4392014

//
#define BUFFER_SIZE 1024

#define FLASH_SIZE 1024*1024
#define PROTOK 0xc1a0c1a0
typedef struct {
  unsigned cmd;
  unsigned add;
  unsigned len;
} command_t;

unsigned totry[]={
  50,
  75,
  110,
  134,
  150,
  200,
  300,
  600,
  1200,
  2400,
  4800,
  9600,
  19200,
  38400,
  57600,
  115200
};

void check_error(int a){
  printf("\n\n\n\n\n");
  switch(a){
  case -1:
    printf("## verify error\n");
    break;
  case -2:
    printf("## flash timeout error\n");
    break;
  case -3:
    printf("## product id mismatch\n");
    break;
  case -4:
    printf("## device id mismatch\n");
    break;
  case -5:
    printf("## flash write protected\n");
    break;
  default:
    printf("## PROTOCOL ERROR check serial connection 0x%x (%d)\n",a,a);
    break;
  }

}

unsigned num2baud(unsigned baud_rate){
  unsigned BAUD;
  switch (baud_rate)
      {

      default:
	printf("%% baud rate %d not supported\n",baud_rate);
	exit(1);
      case 115200:
	BAUD = B115200;
            break;
      case 57600:
	BAUD = B57600;
            break;
      case 38400:

	   BAUD = B38400;
            break;
         case 19200:
            BAUD  = B19200;
            break;
         case 9600:
            BAUD  = B9600;
            break;
         case 4800:
            BAUD  = B4800;
            break;
         case 2400:
            BAUD  = B2400;
            break;
         case 1800:
            BAUD  = B1800;
            break;
         case 1200:
            BAUD  = B1200;
            break;
         case 600:
            BAUD  = B600;
            break;
         case 300:
            BAUD  = B300;
            break;
         case 200:
            BAUD  = B200;
            break;
         case 150:
            BAUD  = B150;
            break;
         case 134:
            BAUD  = B134;
            break;
         case 110:
            BAUD  = B110;
            break;
         case 75:
            BAUD  = B75;
            break;
         case 50:
            BAUD  = B50;
            break;
      }
  return BAUD;
}
unsigned check_ack(int fd){
  unsigned data;
  //usleep(80000);
  read(fd,&data,4);
  if((data==PROTOK)){
    //    printf("## check ack :0x%x\n",data);
    return 0;
  }
  return data;
}

unsigned ask(int fd,unsigned cmd, unsigned address,unsigned len){
  command_t data;

  data.len = len;
  data.cmd = cmd;
  data.add = address;
  //   printf("%% sending command\n");
  //usleep(80000);
  write(fd,&data.cmd,sizeof(data));

  //  sleep(1);
  /*  if((res=check_ack(fd)) ==0){
    //    printf("* command OK\n");
    return 0;
  } else {
    printf("## ACK check sending command failed 0x%x\n",res);
    }*/
  //  sleep(1);
  return 0;
}




main(int argc,char**argv){

  struct termios uparam,uparam_old;
  unsigned res;
  command_t data;
  int i,fd;
  unsigned baud_rate=115200;
  long BAUD;
  unsigned STOPBITS = CSTOPB;
  unsigned PARITY = 0;
  unsigned DATABITS = CS8;
  unsigned PARITYON = 0;
  int tot=0;
#ifdef LINUX
  char* s_dev="/dev/ttyS1";
#else
#warning "USING WIN32 SETTINGS"
  char* s_dev="/dev/com2";
#endif
  char* s_fname=0;
  char* s_add=0;
  char* s_len=0;
  unsigned address=0x500000,len=0,tlen;
  FILE* mybin=0;
  struct stat info;
  char buffer[BUFFER_SIZE];
  unsigned ok=0xc1a0c1a0;
  for(i=1;i<argc;i++){
    if(!strcmp(argv[i],"-d")){
      s_dev = argv[++i];
      continue;
    }


    if(!strcmp(argv[i],"-a")){
      s_add = argv[++i];
      continue;
    }

    /*    if(!strcmp(argv[i],"-reset")){
      operation = CMD_RESET;
      continue;
      }*/

    if(!strcmp(argv[i],"-h")){
      printf("usage is %s <bin> [-a <flash address>]\n",argv[0]);
      exit(0);
    }
    s_fname = argv[i];
    // printf("name  -> %s",s_fname);
  }

  if(s_fname){
    mybin=fopen(s_fname,"rb");
    if(mybin==0){
      printf("## you must specify a valid filename\n");
      exit(1);
    }
  } else {
     printf("## you must specify a valid filename\n");
     exit(1);
  }
  if(s_add){
    address=strtoul(s_add,0,0);
  }
  BAUD=num2baud(baud_rate);
  fd=open(s_dev,O_RDWR |O_NOCTTY| O_NDELAY);
  if(fd<0){
    printf("## cannot open serial %s\n",s_dev);
    exit(1);
  }
 if(tcgetattr(fd,&uparam)<0){
    perror("## cannot get serial paramenters \n");
    exit(1);
  }
 //CS8 : 8 n 1
 // hw flow ctrl
  //bzero(&uparam, sizeof(uparam));

  uparam.c_cflag = BAUD | CS8 |CLOCAL|CREAD;

  uparam.c_iflag = 0; //raw device
  uparam.c_oflag = 0; // raw device
  uparam.c_lflag = 0;//ICANON;
  //uparam.c_cc[VMIN]=1;
  //uparam.c_cc[VEOF]=4;/*ctrld*/
  //uparam.c_cc[VTIME]=0;
  fcntl(fd, F_SETFL, 0);

  if(tcflush(fd, TCIFLUSH)<0){
    printf("## cannot flush serial\n");
    exit(1);
  }
  if(tcsetattr(fd,TCSANOW,&uparam)<0){
    printf("## cannot set serial paramenters\n");
    exit(1);
  } else {
    printf("* using serial %s, %d, no parity, 8b, 1stop\n",s_dev,baud_rate);
  }
/*
  if(tcflush(fd, TCIFLUSH)<0){
    printf("## cannot flush serial paramenters\n");
    exit(1);
  }*/
  /*
  printf("* press return\n");
  {
	  char pp[80];
	  gets(pp);
	}*/
  lstat(s_fname,&info);
  len = info.st_size;

  printf("* binary len %d bytes..\n",len);
  if(ask(fd,PRG_FLASH,address,len)!=0) {
    fclose(mybin);
    close(fd);
    return 0;
  }
  // check flsh id
  if((res=check_ack(fd))!=0){
    printf("## flash id check failed 0x%x\n",res);
    close(fd);
    exit(1);
  }
  printf("* flash id check ok\n");
  write(fd,&ok,4);
  if((res=check_ack(fd))!=0){
      printf("## flash id check failed 0x%x\n",res);
      close(fd);
      exit(1);
  }
  printf("* erasing space address 0x%x... please wait\n",address);
  // verify erase
  if((res=check_ack(fd))!=0){
    printf("## verify erase failed 0x%x\n",res);
    close(fd);
    exit(1);
  }
  printf("* flash erase check ok\n");
  printf("* start programming %d bytes.\n",len);
  tlen=len;
  while(len>0){
    int ll=(len<BUFFER_SIZE)?len:BUFFER_SIZE;
    tot+=ll;
    fread(buffer,1,ll,mybin);
    usleep(8000);
    write(fd,buffer,ll);
    printf("* downloading %.3f (rem:%d bytes)\r",(tlen-len)*100.0/tlen,tot);
    if((res=check_ack(fd))!=0){
      printf("## programming failed base add 0x%x\n",res);
      //      close(fd);
      //      exit(1);
    }

    len-=ll;

  }
  printf("\n* 100%% end programming\n");
}


