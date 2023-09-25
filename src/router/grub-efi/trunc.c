#include <stdio.h>


int main(int argc, char *argv[])
{
FILE *in,*out;
in=fopen(argv[1],"rb");
out=fopen(argv[2],"wb");
fseek(in,0,SEEK_END);
size_t len = ftell(in);
rewind(in);
char *mem=malloc(len);
fread(mem,len,1,in);
size_t i;
size_t rlen;
for (i=0;i<len;i++) {
    if (mem[i]!=0)
	rlen = i;
}
rlen = 688640*512;
//rlen +=511;
//rlen /=512;
//rlen *=512;
fwrite(mem,rlen,1,out);
fclose(in);
fclose(out);

}
