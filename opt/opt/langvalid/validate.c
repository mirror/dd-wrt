#include <stdio.h>
static void StringStart(FILE *in) 
{
int b=0;
while(b!='"')
    {
    b=getc(in);
    if (b=='\\')
	b=getc(in);
    if (feof(in))return;
    }
}
 
static char *getFileString(FILE *in) 
{ 
char *buf; 
int i,b; 
buf = malloc(1024);
StringStart(in); 
for (i=0;i<1024;i++)   
    {   
    b = getc(in);   
    if (b==EOF)return NULL;   
    if (b=='\\')
    {
    buf[i] = getc(in);
    continue;
    }
    if (b==0xa)
    {
    return "error";
    }
    if (b=='"')      
	{      
	buf[i]=0;     
	buf=realloc(buf,strlen(buf)+1);
	return buf;
	}
    buf[i]=b;   
    }
return buf; 
}


void main(int argc,char *argv[])
{
FILE *in;
char *o,*t;
int linecount=1;
if (argc<2)
    {
    puts("syntax: validate [language file]");
    exit(1);
    }
in=fopen(argv[1],"rb");
while(1)
    {
    o=getFileString(in);
    if (o==NULL)break;
    if (strcmp(o,"error")==0)
	{
	printf("Error on line %d, please check translation file\n",linecount);
	break;
	}
    t=getFileString(in);
    if (t==NULL)
	{
	printf("Error on line %d, please check translation file\n",linecount);
	break;
	}
    if (strcmp(t,"error")==0)
	{
	printf("Error on line %d, please check translation file\n",linecount);
	break;
	}
printf("%s = %s\n",o,t);
    linecount++;
    }
fclose(in);
}