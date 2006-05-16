/*
 * Tiny Embedded JavaScript parser
 *
 * Copyright 2001-2003, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: ej.c,v 1.1 2005/09/26 16:58:15 seg Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

static char *lastlanguage=NULL;
typedef struct{
char *original;
char *translation;
}LANGUAGE;

#define PATTERN_BUFFER 1000

#define DEBUGLOG 1

#ifdef DEBUGLOG
FILE *log;
#define LOG(a) if (log!=NULL)fprintf(log,"%s\n",a); fflush(log);
#else
#define LOG(a)
#endif


static LANGUAGE **language=NULL;
static unsigned int langcount=0;
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

#define LANG_PREFIX "/etc/langpack/"
#define LANG_POSTFIX ".lang"


void initLanguage(char *filename)
{
int i;
char *fd;
char *original;
char *translate;
char *langstr;
LANGUAGE *desc;
FILE *in;
//if (language!=NULL)return;
langcount=0;
LOG("langstr");
langstr = filename;
if (langstr==NULL)return;
if (language!=NULL && lastlanguage!=NULL)
{
if (strcmp(langstr,lastlanguage)!=0)
    {
    lastlanguage = langstr;
    for (i=0;i<langcount;i++)
	{
	free(language[i]->original);
	free(language[i]->translation);
	}
    langcount=0;
    }
}else
if (lastlanguage==NULL)
    lastlanguage = langstr;

in = fopen(langstr,"rb");
if (in==NULL)return;
LOG("read desc");
while(1)
{
original = getFileString(in);
if (original==NULL)break;
translate = getFileString(in);
desc = (LANGUAGE*)malloc(sizeof(LANGUAGE));
LOG("entry");
LOG(original);
LOG(translate);   
desc->original = original;
desc->translation = translate;
//realloc space
language = (LANGUAGE**)realloc(language,sizeof(struct LANGUAGE**)*(langcount+2));              
language[langcount++] = desc;
language[langcount]=NULL;
}
fclose(in);

}

char *translatePage(char *buffer)
{
char *dest;
long len;
char *search;
char *replace;
int i,a,z,sl,rl;
LOG("translate");
len = strlen(buffer);
for (a=0;a<langcount;a++)
    {
    search = language[a]->original;
    replace = language[a]->translation;
    sl = strlen(search);
    rl = strlen(replace);
    for (i=0;i<len-sl;i++)
	{
	for (z=0;z<sl;z++)
	    {
	    if (search[z]!=buffer[i+z])break;
	    }
	if (z==sl)
	    {
	    LOG("replace");
	    LOG(search);
	    LOG(replace);
	    dest = (char*)malloc(len-sl+rl+1+PATTERN_BUFFER);
	    for (z=0;z<i;z++)dest[z] = buffer[z];
	    for (z=i;z<i+rl;z++) dest[z] = replace[z-i];
	    for (z=i+sl;z<len;z++) dest[z-sl+rl] = buffer[z];
	    dest[z-sl+rl]=0;
	    free(buffer);
	    buffer = dest;
	    len=strlen(buffer);
	    i-=sl;
	    i+=rl;
	    }
	}
    }
LOG("write");
return buffer;
}


int main(int argc,char *argv[])
{
char *buffer;
char *dest;
int i;
#ifdef DEBUGLOG
log=fopen("log.log","wb");
#endif

printf("translation tool\n");
printf("reading from %s\n",argv[1]);
initLanguage(argv[1]);
if (argc<3)return -1;
for (i=2;i<argc;i++)
{
printf("translating %s\n",argv[i]);
FILE *in = fopen(argv[i],"rb");
long len;
fseek(in,0,SEEK_END);
len=ftell(in);
rewind(in);
buffer = (char*)malloc(len+1);
buffer[len]=0;
fread(buffer,len,1,in);
fclose(in);
in=fopen(argv[i],"wb");
dest = translatePage(buffer);
fwrite(dest,strlen(dest),1,in);
fclose(in);
}
}