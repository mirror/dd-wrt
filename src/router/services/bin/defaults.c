#define STORE_DEFAULTS

#include "../sysinit/defaults.c"


int main(int argc,char *argv[])
{
FILE *out;
out=fopen("defaults.bin","wb");
int i;
int len = sizeof(srouter_defaults)/sizeof(struct nvram_tuple);
fwrite(&len,4,1,out);
for (i=0;i<sizeof(srouter_defaults)/sizeof(struct nvram_tuple);i++)
    {
    if (srouter_defaults[i].name)
	{
    putc(strlen(srouter_defaults[i].name),out);
    fwrite(srouter_defaults[i].name,strlen(srouter_defaults[i].name),1,out);
    len =  strlen(srouter_defaults[i].value);
    if (len>127)
	{
	len|=128;
	putc(len&0xff,out);
	putc(strlen(srouter_defaults[i].value)>>7,out);
	}else
	{
	putc(len,out);	
	}
    fwrite(srouter_defaults[i].value,strlen(srouter_defaults[i].value),1,out);
	}else{
	putc(0,out);
	putc(0,out);
	}
	
    }
fclose(out);
return 0;
}
