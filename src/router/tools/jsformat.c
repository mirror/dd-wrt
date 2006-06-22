#include <stdio.h>


void filter(char *name)
{
int len;
FILE *in=fopen(name,"rb");
fseek(in,0,SEEK_END);
len=ftell(in);
rewind(in);
char *buf=(char*)malloc(len);
fread(buf,len,1,in);
fclose(in);
in=fopen(name,"wb");
int i;
for (i=0;i<len;i++)
{
if (i==0 && buf[i]==0xa)i++;
if (buf[i]=='/' && buf[i+1]=='/')
    {
    while(buf[i]!=0xa)i++;
    continue;
    }
if (buf[i]==0xa && i<len-1)
    {
    while(buf[i+1]==0xa && i<len-1)i++;
    }
putc(buf[i],in);
}
fclose(in);
}

int main(int argc,char *argv[])
{
int i;
for (i=1;i<argc;i++)
    filter(argv[i]);
return 0;
}
