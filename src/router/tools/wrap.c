#include <stdio.h>


int main(int argc,char *argv[])
{
int i;
FILE *in = fopen(argv[1],"rb");
FILE *out = fopen(argv[2],"wb");
fprintf(out,"WRAP1");
int vmsize;
int fssize;
fseek(in,0,SEEK_END);
vmsize=ftell(in);
rewind(in);
fwrite(&vmsize,1,4,out);
for (i=0;i<vmsize;i++)
    putc(getc(in),out);    
fclose(out);
fclose(in);
}
