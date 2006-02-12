#include <stdio.h>
#include <malloc.h>
#include "vfs.h"


//typedef struct HEADER header;
entry *vfsopen(char *name,char *mode)
{
header h;
entry *current;
int i,a;
int r=0;
FILE *in=fopen("/usr/lib/vfs.lib","rb");
if (in==NULL)return NULL;
r=fread(&h,10,1,in);
h.entries=(entry*)malloc(sizeof(entry)*h.noe);
//read entrytable
for (i=0;i<h.noe;i++)
    {
    h.entries[i].namelen=getc(in);
    h.entries[i].name=(char*)malloc(h.entries[i].namelen+1);
    fread(h.entries[i].name,h.entries[i].namelen,1,in);
    h.entries[i].name[h.entries[i].namelen]=0;
    if (strcmp(name,h.entries[i].name)==0)
	{
	current=&h.entries[i];
	current->curpos=0;
	current->offset=getw(in);
	current->filelen=getw(in);
	current->content=malloc(current->filelen);
	fseek(in,current->offset,SEEK_SET);
	fread(current->content,current->filelen,1,in);
	fclose(in);
	for (a=0;a<i;a++)
	    {
	    free(h.entries[a].name);
	    free(h.entries);
	    }
	return current;
	}
    }
for (a=0;a<i;a++)
	    {
	    free(h.entries[a].name);
	    free(h.entries);
	    }
return NULL;    
//if (r<10)return NULL;
}


int vfsseek(entry *in,int pos,int use)
{
switch(use)
{
case SEEK_SET:
in->curpos=pos;
break;
case SEEK_END:
in->curpos=(in->filelen-1)-pos;
if (in->curpos<0){
    in->curpos=0;
    return -1;
    }
break;
case SEEK_CUR:
in->curpos+=pos;
if (in->curpos>=in->filelen)
    {
    in->curpos=0;
    return -1;
    }
break;
}
return in->curpos;
}

int vfsgetc(entry *in)
{
if (in->curpos==in->filelen)return EOF;
unsigned char b=in->content[in->curpos];
in->curpos++;
return b;
}

int vfsread(void *fill,int blocks,int blocksize,entry *in)
{
int b;
int i;
int cnt=0;
unsigned char *p=(unsigned char*)fill;
for (i=0;i<blocks*blocksize;i++)
    {
    b =vfsgetc(in);
    if (b==EOF)return cnt;
    p[cnt++]=b;
    }
return cnt;
}

int vfsrewind(entry *in)
{
in->curpos=0;
return 0;
}

int vfsclose(entry *in)
{
if (in==NULL)return -1;
if (in->name!=NULL)
    free(in->name);
if (in->content!=NULL)
    free(in->content);
free(in);
return 0;
}




