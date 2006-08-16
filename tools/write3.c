#include <stdio.h>
#include <malloc.h>

int main(int argc,char *argv[])
{
int i;
char *m;
int a;

if (argc==1)
    return 0;
for (a=1;a<argc;a++)
{
printf("reading %s\n",argv[a]);
FILE *in = fopen(argv[a],"rb");
if (in==NULL)return 0;
fseek(in,0,SEEK_END);
int len=ftell(in);
//printf("malloc\n");
m=(char*)malloc(len);
fseek(in,0,SEEK_SET);
//printf("read\n");
for (i=0;i<len;i++)
     m[i]=getc(in);
     

fclose(in);
//printf("reopen\n");

in = fopen(argv[a],"wb");
//printf("trans\n");

for (i=0;i<len;i++)
    m[i]^='d';
      char b;
      for (i = 0;i<len/2;i++)
         {
	 b = m[i];
	 m[i]=m[(len-1)-i];
	 m[(len-1)-i]=b;
	 }
//printf("write\n");

fwrite(m,len,1,in);
fclose(in);
}
return 0;
}
