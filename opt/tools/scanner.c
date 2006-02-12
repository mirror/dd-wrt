#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <dirent.h>
#include <stdlib.h>



int
endswith (char *str, char *cmp)
{
  int cmp_len, str_len, i;
  cmp_len = strlen (cmp);
  str_len = strlen (str);
  if (cmp_len > str_len)
    return (0);
  for (i = 0; i < cmp_len; i++)
    {
      if (str[(str_len - 1) - i] != cmp[(cmp_len - 1) - i])
	return (0);
    }
  return (1);
}



char *number(int nr)
{
char snr[10];
char *res;
res=malloc(10);
sprintf(snr,"%d",nr);
if (strlen(snr)==1)
{
sprintf(res,"00%d",nr);
}
if (strlen(snr)==2)
{
sprintf(res,"0%d",nr);
}
if (strlen(snr)==3)
{
sprintf(res,"%d",nr);
}
return res;
}
//$NAME:003:Aim$PROT:002:l7$PORT:003:0:0<&nbsp;>

char *sort[32768];

void main(int argc,char *argv[])
{
struct dirent *entry;
DIR *directory;
int idx;
char *copy;
int cnt=0;
for (idx=1;idx<argc;idx++)
{
  directory = opendir (argv[idx]);
      if (directory == NULL)
	continue;
//list all files in this directory 
      while ((entry = readdir (directory)) != NULL)
	{
	  if (endswith (entry->d_name, ".pat"))
	    {
	    copy=strdup(entry->d_name);
	    copy[strlen(copy)-4]=0;	    
	    sort[cnt++]=copy;    
	    }
	}
closedir(directory);
}
//sort entries
int i,a;
for (i=0;i<cnt;i++)
{
for (a=0;a<cnt;a++)
{
if (strcmp(sort[i],sort[a])<0)
    {
    char *b=sort[i];
    sort[i]=sort[a];
    sort[a]=b;
    }
}
}
for (i=0;i<cnt;i++)
{
	    printf("$NAME:%s:%s$PROT:002:l7$PORT:003:0:0<&nbsp;>",number(strlen(sort[i])),sort[i]);
}

}
