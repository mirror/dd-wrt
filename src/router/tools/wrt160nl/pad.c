#include <stdio.h>



void main(int argc,char *argv[])
{
FILE *in,*out;
in=fopen(argv[1],"rb");
out=fopen(argv[2],"wb");
fseek(in,0,SEEK_END);
long len = ftell(in);
rewind(in);
long i;
for (i=0;i<len;i++)
    putc(getc(in),out);
fclose(in);
long oldlen=len;
len+=8192;
len/=4096;
len*=4096;
len-=oldlen;
len-=0x3c;
for (i=0;i<len;i++)
    putc(0,out);
fclose(out);

}