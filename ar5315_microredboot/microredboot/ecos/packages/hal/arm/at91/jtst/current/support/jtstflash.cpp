// jtstflash.cpp Andrea Michelotti amichelotti@atmel.com
// simple flash software. Compiled with MS Visual Studio 6

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
#include "stdafx.h"
#include "windows.h"
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>

static HANDLE hComm;

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


unsigned check_ack(){
  unsigned data;
  unsigned long len;
  //usleep(80000);
  //read(fd,&data,4);
  ReadFile(hComm,&data,4,&len,NULL);
  if(len!=4){
	  printf("## (check_ack) protocol error byte read %d, expected 4\n",len);
	  exit(1);
  }
  if((data==PROTOK)){
    //    printf("## check ack :0x%x\n",data);
    return 0;
  }
  return data;
}

unsigned ask(unsigned cmd, unsigned address,unsigned len){
  unsigned long rlen;
  command_t data;

  data.len = len;
  data.cmd = cmd;
  data.add = address;
  //   printf("%% sending command\n");
  //usleep(80000);
  WriteFile(hComm,&data.cmd,sizeof(data),&rlen,NULL);

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

void initCom(char* com){
	DCB dcb;
	hComm = CreateFile (
		com,
	GENERIC_READ|GENERIC_WRITE,
	0,
	NULL,
	OPEN_EXISTING,
	0,
	0
	);

	if(hComm==INVALID_HANDLE_VALUE){
		printf("##  cannot open serial port %s\n",com);
		exit(1);
	}

	if(GetCommState(hComm,&dcb)==0){
		printf("## cannot get serial status\n");
		exit(1);
	}

	dcb.BaudRate= CBR_115200;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	dcb.fDtrControl= DTR_CONTROL_DISABLE;
	dcb.fRtsControl = RTS_CONTROL_DISABLE;
	dcb.DCBlength = sizeof(dcb);
	if(SetCommState(hComm, &dcb)==0){
		printf("## cannot set serial status\n");
		exit(1);
	}

}



int main(int argc, char* argv[])
{
 int res,i,tot=0;
  char* s_dev="COM2";

  char* s_fname=0;
  char* s_add=0;
  char* s_len=0;
  unsigned address=0x500000,len=0,tlen;
  FILE* mybin=0;
  struct stat info;
  char buffer[BUFFER_SIZE];
  unsigned ok=0xc1a0c1a0;
  unsigned long lenw;
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

	initCom(s_dev);

	{
		int fdf=fseek(mybin,0,SEEK_END );
		fdf = ftell(mybin);
		fseek(mybin,0,SEEK_SET );
		len = fdf- ftell(mybin);
	}
  printf("* binary len %d bytes flash add 0x%x..\n",len,address);
  if(ask(PRG_FLASH,address,len)!=0) {
    fclose(mybin);
    CloseHandle(hComm);
    return 0;
  }
  // check flsh id
  if((res=check_ack())!=0){
    printf("## flash id check failed 0x%x\n",res);
    CloseHandle(hComm);
    exit(1);
  }
  printf("* flash id check ok\n");

  WriteFile(hComm,&ok,4,&lenw,NULL);
  if((res=check_ack())!=0){
      printf("## flash id check failed 0x%x\n",res);
       CloseHandle(hComm);
      exit(1);
  }
  printf("* erasing space address 0x%x... please wait\n",address);
  // verify erase
  if((res=check_ack())!=0){
    printf("## verify erase failed 0x%x\n",res);
    CloseHandle(hComm);
    exit(1);
  }
  printf("* flash erase check ok\n");
  printf("* start programming %d bytes.\n",len);
  tlen=len;
  while(len>0){

    unsigned long ll=(len<BUFFER_SIZE)?len:BUFFER_SIZE;
    tot+=ll;
    fread(buffer,1,ll,mybin);
    //usleep(8000);
    //write(fd,buffer,ll);
	WriteFile(hComm,buffer,ll,&lenw,NULL);
	if(ll!=lenw){
		printf("## error writing byte write %d != byte to write %d\n",lenw,ll);
		exit(1);
	}
    printf("* downloading %.3f (rem:%d bytes)\r",(tlen-len)*100.0/tlen,tot);
    if((res=check_ack())!=0){
      printf("## programming failed base add 0x%x\n",res);
      //      close(fd);
      //      exit(1);
    }

    len-=ll;

  }
	printf("\n* 100%% end programming\n");
	return 0;
}

